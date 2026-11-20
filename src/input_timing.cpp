#include "input_timing.h"

#include <atomic>
#include <rex/cvar.h>

REXCVAR_DEFINE_BOOL(skate_experimental_flickit_120hz, false, "Skate",
                    "Run the Flickit input worker at the experimental 120 Hz rate")
    .lifecycle(rex::cvar::Lifecycle::kInitOnly);

namespace {

constexpr uint32_t kOriginalInputRate = 60;
constexpr uint32_t kFlickitInputRate = 120;
constexpr uint32_t kGestureRingOffset = 24;
constexpr uint32_t kPresentationRingOffset = 28;
constexpr uint32_t kInputReaderReturnAddress = 0x824C1F0C;
std::atomic<bool> flickit_at_120_hz{false};
thread_local bool scale_input_button_timing = false;

} // namespace

void SkateSetFlickitRate(PPCRegister& rate) {
    if (rate.u32 == kOriginalInputRate && REXCVAR_GET(skate_experimental_flickit_120hz)) {
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

