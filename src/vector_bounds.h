#pragma once

#include <rex/ppc/context.h>

void SkateCompareBounds(PPCVRegister& destination, PPCVRegister& source,
                        PPCVRegister& bounds, PPCFPSCRRegister& fpscr);
void SkateCompareBoundsLeft(PPCVRegister& vector, PPCVRegister& bounds,
                            PPCFPSCRRegister& fpscr);
void SkateCompareBoundsRight(PPCVRegister& vector, PPCVRegister& source,
                             PPCFPSCRRegister& fpscr);
void SkateCompareBoundsLeftRecord(PPCVRegister& vector, PPCVRegister& bounds,
                                  PPCFPSCRRegister& fpscr, PPCCRRegister& condition);
void SkateCompareBoundsRightRecord(PPCVRegister& vector, PPCVRegister& source,
                                   PPCFPSCRRegister& fpscr, PPCCRRegister& condition);
