#include <algorithm>
#include <chrono>
#include <cstdint>
#include <memory>
#include <mutex>

#include <rex/cvar.h>
#include <rex/logging.h>
#include <rex/rex_app.h>

#include "kernel_audit.h"

rex::PPCImageInfo SkateGuestImage();

REXCVAR_DEFINE_BOOL(skate_frame_timing, false, "Skate", "Log guest swap timing every five seconds");

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

REX_DEFINE_APP(skateport, CreateSkateApp)
