#include "scalar_guards.h"

#include <array>
#include <bit>
#include <cstdio>

namespace {

bool CheckConversion(uint64_t bits, unsigned rounding, bool flush, uint64_t& checked) {
    for (bool in_place : {false, true}) {
        _mm_setcsr(0x1F80 | (rounding << 13) | (flush ? 0x8040 : 0));
        PPCFPSCRRegister fpscr{};
        fpscr.InitHost();
        const auto original_csr = fpscr.getcsr();
        PPCRegister source{}, destination{};
        source.u64 = bits;
        destination.u64 = in_place ? bits : 0xA55A12345678FFFFULL;
        const auto original_destination = destination.u64;
        const bool handled = in_place ? SkateGuardInt64ConversionInPlace(destination, fpscr)
                                      : SkateGuardInt64Conversion(destination, source, fpscr);
        const bool boundary = bits == std::bit_cast<uint64_t>(0x1p63);
        const auto expected = boundary ? uint64_t(INT64_MAX) : original_destination;
        const auto expected_csr = boundary ? original_csr & ~PPCFPSCRRegister::FlushMask
                                           : original_csr;
        if (handled != boundary || destination.u64 != expected || source.u64 != bits ||
            fpscr.getcsr() != expected_csr || fpscr.csr != expected_csr) {
            std::fprintf(stderr, "Conversion guard failure: input=%016llX alias=%u round=%u flush=%u\n",
                         static_cast<unsigned long long>(bits), in_place, rounding, flush);
            return false;
        }
        ++checked;
    }
    return true;
}

bool CheckShift(uint64_t value, uint64_t distance, unsigned flags, uint64_t& checked) {
    for (unsigned variant = 0; variant < 9; ++variant) {
        const unsigned width = variant < 6 ? 32 : 64;
        const unsigned alias = variant % 3;
        const bool record = variant >= 3 && variant < 6;
        const bool should_handle = distance % (2 * width) >= width;
        const bool negative = width == 32
            ? std::bit_cast<int32_t>(static_cast<uint32_t>(value)) < 0
            : std::bit_cast<int64_t>(value) < 0;
        PPCRegister source{}, count{}, destination{};
        source.u64 = value;
        count.u64 = distance;
        destination.u64 = 0x1234A55AFFDD4321ULL;
        PPCXERRegister xer{uint8_t(flags & 1), uint8_t((flags >> 1) & 1), uint8_t((flags >> 2) & 1)};
        const auto original_ca = xer.ca;
        PPCCRRegister condition{};
        condition.set_raw(flags & 8 ? 15 : 0);
        const auto original_cr = condition.raw();
        bool handled = false;
        switch (variant) {
            case 0: handled = SkateGuardWordShift(destination, source, count, xer); break;
            case 1: handled = SkateGuardWordShiftSource(source, count, xer); destination = source; break;
            case 2: handled = SkateGuardWordShiftCount(count, source, xer); destination = count; break;
            case 3: handled = SkateGuardWordShiftRecord(destination, source, count, xer, condition); break;
            case 4:
                handled = SkateGuardWordShiftSourceRecord(source, count, xer, condition);
                destination = source;
                break;
            case 5:
                handled = SkateGuardWordShiftCountRecord(count, source, xer, condition);
                destination = count;
                break;
            case 6: handled = SkateGuardDoubleShift(destination, source, count, xer); break;
            case 7: handled = SkateGuardDoubleShiftSource(source, count, xer); destination = source; break;
            case 8: handled = SkateGuardDoubleShiftCount(count, source, xer); destination = count; break;
        }
        const uint64_t unchanged = alias == 1 ? value : alias == 2 ? distance : 0x1234A55AFFDD4321ULL;
        const uint64_t expected = should_handle ? (negative ? UINT64_MAX : 0) : unchanged;
        const uint32_t expected_cr = should_handle && record
            ? (negative ? 8 : 2) | (flags & 1) : original_cr;
        if (handled != should_handle || destination.u64 != expected ||
            xer.ca != (should_handle ? negative : original_ca) ||
            xer.so != (flags & 1) || xer.ov != ((flags >> 1) & 1) ||
            condition.raw() != expected_cr ||
            (alias != 1 && source.u64 != value) || (alias != 2 && count.u64 != distance)) {
            std::fprintf(stderr, "Shift guard failure: value=%016llX count=%016llX adapter=%u flags=%u\n",
                         static_cast<unsigned long long>(value),
                         static_cast<unsigned long long>(distance), variant, flags);
            return false;
        }
        ++checked;
    }
    return true;
}

bool Run(uint64_t& checked) {
    constexpr std::array<uint64_t, 22> floating_bits{
        0, 0x8000000000000000ULL, 1, 0x8000000000000001ULL,
        0x3FF0000000000000ULL, 0xBFF0000000000000ULL,
        0x43DFFFFFFFFFFFFFULL, 0x43E0000000000000ULL, 0x43E0000000000001ULL,
        0xC3DFFFFFFFFFFFFFULL, 0xC3E0000000000000ULL, 0xC3E0000000000001ULL,
        0x7FEFFFFFFFFFFFFFULL, 0xFFEFFFFFFFFFFFFFULL,
        0x7FF0000000000000ULL, 0xFFF0000000000000ULL,
        0x7FF8000000000000ULL, 0xFFF8000000000001ULL,
        0x7FF0000000000001ULL, 0xFFF0000000000001ULL,
        0x3FF8000000000000ULL, 0xBFF8000000000000ULL,
    };
    for (unsigned rounding = 0; rounding < 4; ++rounding) {
        for (bool flush : {false, true}) {
            for (auto bits : floating_bits) {
                if (!CheckConversion(bits, rounding, flush, checked)) return false;
            }
        }
    }
    constexpr std::array<uint64_t, 13> integers{
        0, 1, 2, 0x7FFFFFFF, 0x80000000, 0x80000001, 0xFFFFFFFF,
        0x100000000ULL, 0xFFFFFFFF7FFFFFFFULL, 0xFFFFFFFF80000000ULL,
        0x7FFFFFFFFFFFFFFFULL, 0x8000000000000000ULL, UINT64_MAX,
    };
    for (auto value : integers) {
        for (uint64_t count = 0; count < 256; ++count) {
            for (unsigned flags = 0; flags < 16; ++flags) {
                if (!CheckShift(value, count, flags, checked) ||
                    !CheckShift(value, count | 0xD3A1000000000000ULL, flags, checked)) return false;
            }
        }
    }
    return true;
}

} // namespace

int main() {
    const unsigned saved_mxcsr = _mm_getcsr();
    uint64_t checked = 0;
    const bool passed = Run(checked);
    _mm_setcsr(saved_mxcsr);
    if (!passed) return 1;
    std::printf("Passed %llu scalar guard cases\n", static_cast<unsigned long long>(checked));
    return 0;
}
