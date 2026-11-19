#include <algorithm>
#include <array>
#include <atomic>
#include <bit>
#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <memory>
#include <mutex>
#include <string>

#include <Windows.h>
#include <rex/audio/sdl/sdl_audio_system.h>
#include <rex/cvar.h>
#include <rex/logging.h>
#include <rex/ppc.h>
#include <rex/rex_app.h>

#include "kernel_audit.h"

rex::PPCImageInfo SkateGuestImage();

REXCVAR_DEFINE_BOOL(skate_frame_timing, false, "Skate", "Log guest swap timing every five seconds");
REXCVAR_DEFINE_BOOL(skate_gpu_wait_backoff, false, "Skate", "Back off sustained guest graphics polling");

namespace {

constexpr uint32_t kOriginalInputRate = 60;
constexpr uint32_t kFlickitInputRate = 120;
constexpr uint32_t kGestureRingOffset = 24;
constexpr uint32_t kPresentationRingOffset = 28;
constexpr uint32_t kInputReaderReturnAddress = 0x824C1F0C;
std::atomic<bool> flickit_at_120_hz{false};
thread_local bool scale_input_button_timing = false;

uint16_t PackFloat16(uint32_t bits) {
    const uint32_t sign = (bits >> 16) & 0x8000;
    const uint32_t exponent = (bits >> 23) & 0xFF;
    const uint32_t mantissa = bits & 0x007FFFFF;
    if (exponent == 0xFF) {
        const uint32_t quiet_bit = mantissa == 0 ? 0 : 0x0200;
        return static_cast<uint16_t>(sign | 0x7C00 | (mantissa >> 13) | quiet_bit);
    }
    // Conversion toward zero clamps finite overflow and truncates discarded bits.
    if (exponent > 142) {
        return static_cast<uint16_t>(sign | 0x7BFF);
    }
    if (exponent < 103) {
        return static_cast<uint16_t>(sign);
    }
    if (exponent < 113) {
        return static_cast<uint16_t>(sign | ((0x00800000 | mantissa) >> (126 - exponent)));
    }
    return static_cast<uint16_t>(sign | ((exponent - 112) << 10) | (mantissa >> 13));
}

void PackFloat16x4(PPCVRegister& destination, const PPCVRegister& source, size_t half_offset) {
    const PPCVRegister input = source;
    for (size_t lane = 0; lane < 4; ++lane) {
        destination.u16[half_offset + lane] = PackFloat16(input.u32[lane]);
    }
}

uint32_t UnpackFloat16(uint16_t bits) {
    const uint32_t sign = static_cast<uint32_t>(bits & 0x8000) << 16;
    const uint32_t exponent = (bits >> 10) & 0x1F;
    const uint32_t mantissa = bits & 0x3FF;
    if (exponent == 0) {
        if (mantissa == 0) {
            return sign;
        }
        const uint32_t shift = std::countl_zero(mantissa) - 21;
        return sign | ((113 - shift) << 23) | (((mantissa << shift) & 0x3FF) << 13);
    }
    if (exponent == 0x1F) {
        // Match Xenia's F16C path by preserving payloads and quieting NaNs.
        const uint32_t quiet_bit = mantissa == 0 ? 0 : 0x00400000;
        return sign | 0x7F800000 | (mantissa << 13) | quiet_bit;
    }
    return sign | ((exponent + 112) << 23) | (mantissa << 13);
}

class SkateAudioSystem final : public rex::audio::sdl::SDLAudioSystem {
public:
    using SDLAudioSystem::SDLAudioSystem;

    rex::X_STATUS CreateDriver(size_t index, rex::thread::Semaphore* semaphore,
                               rex::audio::AudioDriver** driver) override {
        const auto status = SDLAudioSystem::CreateDriver(index, semaphore, driver);
        if (XFAILED(status)) {
            // The Guest uses the driver after a failed XAudio registration.
            REXLOG_ERROR("Could not initialize Port audio. Check the Windows playback device "
                         "and any SDL_AUDIO_DRIVER override. See the preceding SDL error.");
            rex::FlushLogging();
            std::_Exit(EXIT_FAILURE);
        }
        return status;
    }
};

class GuestSwapTiming {
public:
    void Record() {
        std::unique_lock lock(mutex_);
        const auto now = Clock::now();
        if (++submissions_ == 1) {
            window_start_ = last_swap_ = now;
            return;
        }

        const auto gap = now - last_swap_;
        longest_gap_ = std::max(longest_gap_, gap);
        last_swap_ = now;
        ++intervals_;
        const bool log_timing = REXCVAR_GET(skate_frame_timing);
        if (log_timing) {
            const auto milliseconds = std::chrono::ceil<std::chrono::milliseconds>(gap).count();
            ++interval_buckets_[std::min(static_cast<size_t>(milliseconds), kOverflowBucket)];
            ++timed_intervals_;
        }
        const auto elapsed = now - window_start_;
        if (elapsed < std::chrono::seconds(5)) {
            return;
        }

        const double mean_ms = Milliseconds(elapsed).count() / intervals_;
        const double max_ms = Milliseconds(longest_gap_).count();
        const auto intervals = intervals_;
        const auto timed_intervals = timed_intervals_;
        const auto p95_ms = log_timing ? PercentileUpperBound(95) : 0;
        const auto p99_ms = log_timing ? PercentileUpperBound(99) : 0;
        const rex::ui::FrameStats stats{mean_ms, 1000.0 / mean_ms, submissions_};
        stats_ = stats;
        window_start_ = now;
        longest_gap_ = {};
        intervals_ = 0;
        interval_buckets_.fill(0);
        timed_intervals_ = 0;
        lock.unlock();

        if (log_timing) {
            REXLOG_INFO("Guest swaps: {:.1f}/s, mean {:.2f} ms, longest gap {:.2f} ms ({} intervals)",
                        stats.fps, stats.frame_time_ms, max_ms, intervals);
            if (timed_intervals) {
                REXLOG_INFO("Guest swap percentiles: p95 {}, p99 {} ({} timed intervals)",
                            FormatPercentile(p95_ms), FormatPercentile(p99_ms), timed_intervals);
            }
        }
    }

    rex::ui::FrameStats Read() {
        std::scoped_lock lock(mutex_);
        return stats_;
    }

private:
    using Clock = std::chrono::steady_clock;
    using Milliseconds = std::chrono::duration<double, std::milli>;
    static constexpr size_t kOverflowBucket = 101;

    size_t PercentileUpperBound(uint64_t percentile) const {
        if (!timed_intervals_) {
            return 0;
        }
        const auto rank = (timed_intervals_ * percentile + 99) / 100;
        uint64_t cumulative = 0;
        for (size_t bucket = 0; bucket < interval_buckets_.size(); ++bucket) {
            cumulative += interval_buckets_[bucket];
            if (cumulative >= rank) {
                return bucket;
            }
        }
        return kOverflowBucket;
    }

    static std::string FormatPercentile(size_t bucket) {
        return bucket == kOverflowBucket ? ">100 ms" : fmt::format("≤{} ms", bucket);
    }

    std::mutex mutex_;
    Clock::time_point window_start_{};
    Clock::time_point last_swap_{};
    Clock::duration longest_gap_{};
    uint64_t submissions_ = 0;
    uint64_t intervals_ = 0;
    std::array<uint64_t, kOverflowBucket + 1> interval_buckets_{};
    uint64_t timed_intervals_ = 0;
    rex::ui::FrameStats stats_{};
};

GuestSwapTiming guest_swap_timing;

class GraphicsWaitBackoff {
public:
    void Poll() {
        using namespace std::chrono_literals;
        const auto now = std::chrono::steady_clock::now();
        if (now - previous_poll_ > 500us) {
            polls_ = 0;
        }
        previous_poll_ = now;
        if (++polls_ < 256) {
            return;
        }
        polls_ = 0;

        // The Guest checks for available space again after the timer expires.
        static thread_local Timer timer;
        LARGE_INTEGER due{};
        due.QuadPart = -750; // Relative 75 microseconds in 100-nanosecond units.
        if (timer.handle && SetWaitableTimer(timer.handle, &due, 0, nullptr, nullptr, FALSE)) {
            WaitForSingleObject(timer.handle, 1);
        }
    }

private:
    struct Timer {
        HANDLE handle = CreateWaitableTimerExW(nullptr, nullptr, CREATE_WAITABLE_TIMER_HIGH_RESOLUTION,
                                               TIMER_MODIFY_STATE | SYNCHRONIZE);
        ~Timer() {
            if (handle) {
                CloseHandle(handle);
            }
        }
    };

    std::chrono::steady_clock::time_point previous_poll_{};
    unsigned polls_ = 0;
};

class SkateApp final : public rex::ReXApp {
public:
    explicit SkateApp(rex::ui::WindowedAppContext& context)
        : ReXApp(context, "skateport", SkateGuestImage()) {}

private:
    void OnPreSetup(rex::RuntimeConfig& config) override {
        config.audio_factory = [](rex::runtime::FunctionDispatcher* dispatcher) {
            return std::make_unique<SkateAudioSystem>(dispatcher);
        };
        if (config.gpu_plugin.empty()) {
            config.gpu_plugin = "xenos";
        }
    }

    void OnPostLoadXexImage() override {
        SetGuestFrameStats([] { return guest_swap_timing.Read(); });
        LogKernelImportCoverage(*runtime());
    }
};

std::unique_ptr<rex::ui::WindowedApp> CreateSkateApp(rex::ui::WindowedAppContext& context) {
    return std::make_unique<SkateApp>(context);
}

} // namespace

void SkatePackFloat16x4Low(PPCVRegister& destination, PPCVRegister& source) {
    PackFloat16x4(destination, source, 0);
}

void SkatePackFloat16x4High(PPCVRegister& destination, PPCVRegister& source) {
    PackFloat16x4(destination, source, 4);
}

void SkatePackFloat16x4InPlace(PPCVRegister& vector) {
    SkatePackFloat16x4High(vector, vector);
}

void SkatePackFloat16x2High(PPCVRegister& destination, PPCVRegister& source) {
    const uint32_t first = source.u32[3];
    const uint32_t second = source.u32[2];
    destination.u32[3] = (static_cast<uint32_t>(PackFloat16(first)) << 16) | PackFloat16(second);
}

void SkateUnpackShort2(PPCVRegister& destination, PPCVRegister& source) {
    // SHORT2 uses 3.0f-biased bits, reserving -32768 for quiet NaN.
    const std::array<int16_t, 2> components{source.s16[1], source.s16[0]};
    for (size_t lane = 0; lane < components.size(); ++lane) {
        destination.u32[3 - lane] = components[lane] == -32768
            ? 0x7FC00000U
            : static_cast<uint32_t>(0x40400000 + components[lane]);
    }
    destination.u32[1] = 0;
    destination.u32[0] = 0x3F800000U;
}

void SkateUnpackShort2InPlace(PPCVRegister& vector) {
    SkateUnpackShort2(vector, vector);
}

void SkateUnpackFloat16x2InPlace(PPCVRegister& vector) {
    const uint32_t source = vector.u32[0];
    vector.u32[3] = UnpackFloat16(static_cast<uint16_t>(source >> 16));
    vector.u32[2] = UnpackFloat16(static_cast<uint16_t>(source));
    vector.u32[1] = 0;
    vector.u32[0] = 0x3F800000;
}

void SkateUnpackFloat16x4(PPCVRegister& destination, PPCVRegister& source) {
    const PPCVRegister input = source;
    for (size_t lane = 0; lane < 4; ++lane) {
        destination.u32[lane] = UnpackFloat16(input.u16[lane]);
    }
}

void SkateUnpackFloat16x4InPlace(PPCVRegister& vector) {
    SkateUnpackFloat16x4(vector, vector);
}

void SkateSetFlickitRate(PPCRegister& rate) {
    if (rate.u32 == kOriginalInputRate) {
        rate.u64 = kFlickitInputRate;
        flickit_at_120_hz.store(true, std::memory_order_relaxed);
    }
}

bool SkateSkipPresentationInput(PPCRegister& ring_offset) {
    if (ring_offset.u32 != kPresentationRingOffset ||
        !flickit_at_120_hz.load(std::memory_order_relaxed)) {
        return false;
    }

    static thread_local bool skip_snapshot = true;
    skip_snapshot = !skip_snapshot;
    return skip_snapshot;
}

void SkateSelectInputButtonTiming(PPCRegister& return_address, PPCRegister& ring_offset) {
    scale_input_button_timing = return_address.u32 == kInputReaderReturnAddress &&
                                ring_offset.u32 == kGestureRingOffset &&
                                flickit_at_120_hz.load(std::memory_order_relaxed);
}

void SkateAdjustInputRepeat(PPCRegister& initial_delay, PPCRegister& repeat_delay) {
    if (scale_input_button_timing) {
        initial_delay.u64 *= 2;
        repeat_delay.u64 *= 2;
    }
}

void SkateAdjustInputDebounce(PPCRegister& samples, PPCCRRegister& comparison,
                              PPCXERRegister& xer) {
    if (scale_input_button_timing) {
        comparison.compare<int32_t>(samples.s32, 6, xer);
    }
}

void SkateRecordGuestSwap() {
    guest_swap_timing.Record();
}

void SkateBackOffGraphicsWait() {
    if (REXCVAR_GET(skate_gpu_wait_backoff)) {
        static thread_local GraphicsWaitBackoff backoff;
        backoff.Poll();
    }
}

REX_DEFINE_APP(skateport, CreateSkateApp)
