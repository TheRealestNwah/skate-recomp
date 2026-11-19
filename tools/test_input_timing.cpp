#include "input_timing.h"

#include <array>
#include <atomic>
#include <barrier>
#include <cstdio>
#include <thread>

namespace {

bool Expect(bool condition, const char* message) {
    if (!condition) std::fprintf(stderr, "Input timing failure: %s\n", message);
    return condition;
}

bool CheckButtonTiming(bool scaled) {
    PPCRegister initial{}, repeat{};
    initial.u64 = 24;
    repeat.u64 = 12;
    SkateAdjustInputRepeat(initial, repeat);
    if (!Expect(initial.u64 == (scaled ? 48 : 24) && repeat.u64 == (scaled ? 24 : 12),
                "repeat counts must preserve their durations at the selected sample rate")) {
        return false;
    }
    constexpr std::array<int32_t, 11> values{
        INT32_MIN, -1, 0, 2, 3, 4, 5, 6, 7, 8, INT32_MAX,
    };
    for (int32_t value : values) {
        for (uint8_t summary : {0, 1}) {
            PPCRegister samples{};
            samples.s64 = value;
            PPCXERRegister xer{summary, 1, 1};
            PPCCRRegister comparison{};
            // Seed the original Guest comparison against three samples.
            comparison.set_raw((value < 3 ? 8 : 0) | (value > 3 ? 4 : 0) |
                               (value == 3 ? 2 : 0) | summary);
            SkateAdjustInputDebounce(samples, comparison, xer);
            const int32_t threshold = scaled ? 6 : 3;
            const uint32_t expected = (value < threshold ? 8 : 0) |
                                      (value > threshold ? 4 : 0) |
                                      (value == threshold ? 2 : 0) | summary;
            if (!Expect(comparison.raw() == expected && samples.s64 == value &&
                        xer.so == summary && xer.ov == 1 && xer.ca == 1,
                        "debounce result, sample register, or XER flags changed incorrectly")) {
                return false;
            }
        }
    }
    return true;
}

bool CheckSingleThread() {
    PPCRegister rate{}, offset{}, caller{};
    caller.u64 = 0x824C1F0C;
    offset.u64 = 24;
    SkateSelectInputButtonTiming(caller, offset);
    if (!CheckButtonTiming(false)) return false;
    for (uint32_t value : {0U, 1U, 30U, 59U, 61U, 120U, UINT32_MAX}) {
        rate.u64 = value;
        SkateSetFlickitRate(rate);
        offset.u64 = 28;
        if (!Expect(rate.u64 == value && !SkateSkipPresentationInput(offset),
                    "a rate other than 60 must not enable 120 Hz sampling")) return false;
    }
    rate.u64 = 60;
    SkateSetFlickitRate(rate);
    if (!Expect(rate.u64 == 120, "60 Hz must become 120 Hz")) return false;
    for (unsigned sample = 0; sample < 240; ++sample) {
        offset.u64 = 24;
        if (!Expect(!SkateSkipPresentationInput(offset), "gesture samples must not be skipped")) {
            return false;
        }
        offset.u64 = 0;
        if (!Expect(!SkateSkipPresentationInput(offset), "other rings must not be skipped")) {
            return false;
        }
        offset.u64 = 28;
        if (!Expect(SkateSkipPresentationInput(offset) == (sample % 2 == 1),
                    "presentation samples must alternate without interference from other rings")) {
            return false;
        }
    }
    for (uint32_t address : {0x824C1F0CU, 0x824C1F08U, 0x824C1F10U, 0U}) {
        for (uint32_t ring : {24U, 28U, 0U}) {
            caller.u64 = address;
            offset.u64 = ring;
            SkateSelectInputButtonTiming(caller, offset);
            if (!CheckButtonTiming(address == 0x824C1F0C && ring == 24)) return false;
        }
    }
    return true;
}

bool CheckThreads() {
    std::atomic<bool> failed{false};
    std::barrier rendezvous(2);
    const auto worker = [&](bool scaled) {
        PPCRegister caller{}, offset{};
        caller.u64 = scaled ? 0x824C1F0C : 0;
        for (unsigned sample = 0; sample < 1024; ++sample) {
            offset.u64 = 24;
            SkateSelectInputButtonTiming(caller, offset);
            // Both selectors finish before either thread consumes its own timing state.
            rendezvous.arrive_and_wait();
            if (!CheckButtonTiming(scaled)) failed.store(true, std::memory_order_relaxed);
            offset.u64 = 28;
            if (SkateSkipPresentationInput(offset) != (sample % 2 == 1)) {
                failed.store(true, std::memory_order_relaxed);
            }
            rendezvous.arrive_and_wait();
        }
    };
    std::thread first(worker, true), second(worker, false);
    first.join();
    second.join();
    return Expect(!failed.load(std::memory_order_relaxed),
                  "timing selection and presentation phase must remain independent per thread");
}

} // namespace

int main() {
    if (!CheckSingleThread() || !CheckThreads()) return 1;
    std::puts("Passed input rate gates, cadence, debounce/repeat timing, XER flags, and thread isolation");
    return 0;
}
