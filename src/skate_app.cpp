#include <algorithm>
#include <chrono>
#include <cstdint>
#include <memory>
#include <mutex>

#include <Windows.h>
#include <rex/cvar.h>
#include <rex/logging.h>
#include <rex/rex_app.h>

#include "kernel_audit.h"

rex::PPCImageInfo SkateGuestImage();

REXCVAR_DEFINE_BOOL(skate_frame_timing, false, "Skate", "Log guest swap timing every five seconds");
REXCVAR_DEFINE_BOOL(skate_gpu_wait_backoff, false, "Skate", "Back off sustained guest graphics polling");

namespace {

class GuestSwapTiming {
public:
    void Record() {
        std::unique_lock lock(mutex_);
        const auto now = Clock::now();
        if (++submissions_ == 1) {
            window_start_ = last_swap_ = now;
            return;
        }

        longest_gap_ = std::max(longest_gap_, now - last_swap_);
        last_swap_ = now;
        ++intervals_;
        const auto elapsed = now - window_start_;
        if (elapsed < std::chrono::seconds(5)) {
            return;
        }

        const double mean_ms = Milliseconds(elapsed).count() / intervals_;
        const double max_ms = Milliseconds(longest_gap_).count();
        const auto intervals = intervals_;
        const rex::ui::FrameStats stats{mean_ms, 1000.0 / mean_ms, submissions_};
        stats_ = stats;
        window_start_ = now;
        longest_gap_ = {};
        intervals_ = 0;
        lock.unlock();

        if (REXCVAR_GET(skate_frame_timing)) {
            REXLOG_INFO("Guest swaps: {:.1f}/s, mean {:.2f} ms, longest gap {:.2f} ms ({} intervals)",
                        stats.fps, stats.frame_time_ms, max_ms, intervals);
        }
    }

    rex::ui::FrameStats Read() {
        std::scoped_lock lock(mutex_);
        return stats_;
    }

private:
    using Clock = std::chrono::steady_clock;
    using Milliseconds = std::chrono::duration<double, std::milli>;

    std::mutex mutex_;
    Clock::time_point window_start_{};
    Clock::time_point last_swap_{};
    Clock::duration longest_gap_{};
    uint64_t submissions_ = 0;
    uint64_t intervals_ = 0;
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
