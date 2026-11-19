#include "vector_minmax.h"

#include <array>
#include <cstdio>
#include <cstring>

#include <rex/ppc.h>

namespace {

uint32_t ReferenceLane(uint32_t left, uint32_t right, bool maximum) {
    if ((left & 0x7FFFFFFF) > 0x7F800000) return left | 0x00400000;
    if ((right & 0x7FFFFFFF) > 0x7F800000) return right | 0x00400000;
    if ((left & 0x7F800000) == 0) left &= 0x80000000;
    if ((right & 0x7F800000) == 0) right &= 0x80000000;
    if (((left | right) & 0x7FFFFFFF) == 0) {
        return maximum ? left & right : left | right;
    }
    // Integer keys order non-NaN IEEE floats independently of the Host's FP mode.
    const uint32_t left_key = left & 0x80000000 ? ~left : left ^ 0x80000000;
    const uint32_t right_key = right & 0x80000000 ? ~right : right ^ 0x80000000;
    return (maximum ? left_key >= right_key : left_key <= right_key) ? left : right;
}

bool Check(const PPCVRegister& a, const PPCVRegister& b, unsigned rounding,
           bool initial_flush, uint64_t& checked) {
    for (bool maximum : {false, true}) {
        PPCVRegister expected{};
        for (size_t lane = 0; lane < 4; ++lane) {
            expected.u32[lane] = ReferenceLane(a.u32[lane], b.u32[lane], maximum);
        }
        for (unsigned variant = 0; variant < 3; ++variant) {
            _mm_setcsr(0x1F80 | (rounding << 13) | (initial_flush ? 0x8040 : 0));
            PPCFPSCRRegister fpscr{};
            fpscr.InitHost();
            PPCVRegister left = a, right = b, result{};
            if (maximum) {
                switch (variant) {
                    case 0: SkateVectorMax(result, left, right, fpscr); break;
                    case 1: SkateVectorMaxLeft(left, right, fpscr); result = left; break;
                    case 2: SkateVectorMaxRight(right, left, fpscr); result = right; break;
                }
            } else {
                switch (variant) {
                    case 0: SkateVectorMin(result, left, right, fpscr); break;
                    case 1: SkateVectorMinLeft(left, right, fpscr); result = left; break;
                    case 2: SkateVectorMinRight(right, left, fpscr); result = right; break;
                }
            }
            const bool left_preserved = variant == 1 || std::memcmp(&left, &a, sizeof(left)) == 0;
            const bool right_preserved = variant == 2 || std::memcmp(&right, &b, sizeof(right)) == 0;
            if (std::memcmp(&result, &expected, sizeof(result)) != 0 ||
                !left_preserved || !right_preserved ||
                (_mm_getcsr() & 0x8040) != 0x8040 ||
                (_mm_getcsr() & 0x6000) != (rounding << 13)) {
                std::fprintf(stderr, "%s failure at case %llu, adapter %u, rounding %u, flush %u\n",
                             maximum ? "Max" : "Min", static_cast<unsigned long long>(checked),
                             variant, rounding, initial_flush);
                for (size_t lane = 0; lane < 4; ++lane) {
                    std::fprintf(stderr, "lane %zu: left=%08X right=%08X result=%08X expected=%08X\n",
                                 lane, a.u32[lane], b.u32[lane],
                                 result.u32[lane], expected.u32[lane]);
                }
                return false;
            }
            ++checked;
        }
    }
    return true;
}

bool Run(uint64_t& checked) {
    constexpr std::array<uint32_t, 24> values{
        0, 0x80000000, 1, 0x80000001, 0x007FFFFF, 0x807FFFFF,
        0x00800000, 0x80800000, 0x3F800000, 0xBF800000, 0x3F800001, 0xBF800001,
        0x7F7FFFFF, 0xFF7FFFFF, 0x7F800000, 0xFF800000,
        0x7FC00000, 0xFFC00001, 0x7F800001, 0xFFBFFFFF,
        0x7FC12345, 0x7F812345, 0xFFC54321, 0xFF854321,
    };
    uint32_t state = 0x812B5ACE;
    const auto random = [&state] {
        state ^= state << 13;
        state ^= state >> 17;
        state ^= state << 5;
        return state;
    };
    for (unsigned rounding = 0; rounding < 4; ++rounding) {
        for (bool flush : {false, true}) {
            for (uint32_t left : values) {
                for (uint32_t right : values) {
                    PPCVRegister a{}, b{};
                    for (size_t lane = 0; lane < 4; ++lane) {
                        a.u32[lane] = left;
                        b.u32[lane] = right;
                    }
                    if (!Check(a, b, rounding, flush, checked)) return false;
                }
            }
            for (unsigned sample = 0; sample < 4096; ++sample) {
                PPCVRegister a{}, b{};
                for (size_t lane = 0; lane < 4; ++lane) {
                    a.u32[lane] = random();
                    b.u32[lane] = random();
                }
                if (!Check(a, b, rounding, flush, checked)) return false;
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
    std::printf("Passed %llu Host min/max cases\n", static_cast<unsigned long long>(checked));
    return 0;
}
