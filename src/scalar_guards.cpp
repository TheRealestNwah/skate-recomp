#include "scalar_guards.h"

namespace {

template<unsigned width>
bool GuardShift(PPCRegister& destination, const PPCRegister& source,
                 const PPCRegister& count, PPCXERRegister& xer) {
    const auto distance = count.u64 & (2 * width - 1);
    if (distance < width) return false;
    // Read both operands before writing a destination that may alias either.
    const bool negative = ((source.u64 >> (width - 1)) & 1) != 0;
    destination.u64 = negative ? UINT64_MAX : 0;
    xer.ca = negative;
    return true;
}

} // namespace

bool SkateGuardInt64Conversion(PPCRegister& destination, PPCRegister& source,
                               PPCFPSCRRegister& fpscr) {
    // double(INT64_MAX) rounds to this value; the SDK's strict comparison misses equality.
    if (source.u64 != 0x43E0000000000000ULL) return false;
    fpscr.disableFlushMode();
    destination.u64 = 0x7FFFFFFFFFFFFFFFULL;
    return true;
}

bool SkateGuardInt64ConversionInPlace(PPCRegister& value, PPCFPSCRRegister& fpscr) {
    return SkateGuardInt64Conversion(value, value, fpscr);
}

bool SkateGuardWordShift(PPCRegister& destination, PPCRegister& source,
                         PPCRegister& count, PPCXERRegister& xer) {
    return GuardShift<32>(destination, source, count, xer);
}

bool SkateGuardWordShiftSource(PPCRegister& value, PPCRegister& count, PPCXERRegister& xer) {
    return SkateGuardWordShift(value, value, count, xer);
}

bool SkateGuardWordShiftCount(PPCRegister& count, PPCRegister& source, PPCXERRegister& xer) {
    return SkateGuardWordShift(count, source, count, xer);
}

bool SkateGuardWordShiftRecord(PPCRegister& destination, PPCRegister& source,
                               PPCRegister& count, PPCXERRegister& xer, PPCCRRegister& condition) {
    if (!SkateGuardWordShift(destination, source, count, xer)) return false;
    condition.compare<int32_t>(destination.s32, 0, xer);
    return true;
}

bool SkateGuardWordShiftSourceRecord(PPCRegister& value, PPCRegister& count,
                                     PPCXERRegister& xer, PPCCRRegister& condition) {
    return SkateGuardWordShiftRecord(value, value, count, xer, condition);
}

bool SkateGuardWordShiftCountRecord(PPCRegister& count, PPCRegister& source,
                                    PPCXERRegister& xer, PPCCRRegister& condition) {
    return SkateGuardWordShiftRecord(count, source, count, xer, condition);
}

bool SkateGuardDoubleShift(PPCRegister& destination, PPCRegister& source,
                           PPCRegister& count, PPCXERRegister& xer) {
    return GuardShift<64>(destination, source, count, xer);
}

bool SkateGuardDoubleShiftSource(PPCRegister& value, PPCRegister& count, PPCXERRegister& xer) {
    return SkateGuardDoubleShift(value, value, count, xer);
}

bool SkateGuardDoubleShiftCount(PPCRegister& count, PPCRegister& source, PPCXERRegister& xer) {
    return SkateGuardDoubleShift(count, source, count, xer);
}
