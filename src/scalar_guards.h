#pragma once

#include <rex/ppc/context.h>

bool SkateGuardInt64Conversion(PPCRegister& destination, PPCRegister& source,
                               PPCFPSCRRegister& fpscr);
bool SkateGuardInt64ConversionInPlace(PPCRegister& value, PPCFPSCRRegister& fpscr);
bool SkateGuardWordShift(PPCRegister& destination, PPCRegister& source,
                         PPCRegister& count, PPCXERRegister& xer);
bool SkateGuardWordShiftSource(PPCRegister& value, PPCRegister& count, PPCXERRegister& xer);
bool SkateGuardWordShiftCount(PPCRegister& count, PPCRegister& source, PPCXERRegister& xer);
bool SkateGuardWordShiftRecord(PPCRegister& destination, PPCRegister& source,
                               PPCRegister& count, PPCXERRegister& xer, PPCCRRegister& condition);
bool SkateGuardWordShiftSourceRecord(PPCRegister& value, PPCRegister& count,
                                     PPCXERRegister& xer, PPCCRRegister& condition);
bool SkateGuardWordShiftCountRecord(PPCRegister& count, PPCRegister& source,
                                    PPCXERRegister& xer, PPCCRRegister& condition);
bool SkateGuardDoubleShift(PPCRegister& destination, PPCRegister& source,
                           PPCRegister& count, PPCXERRegister& xer);
bool SkateGuardDoubleShiftSource(PPCRegister& value, PPCRegister& count, PPCXERRegister& xer);
bool SkateGuardDoubleShiftCount(PPCRegister& count, PPCRegister& source, PPCXERRegister& xer);
