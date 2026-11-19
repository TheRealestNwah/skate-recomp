#pragma once

#include <rex/ppc/context.h>

void SkateVectorMax(PPCVRegister& destination, PPCVRegister& left,
                    PPCVRegister& right, PPCFPSCRRegister& fpscr);
void SkateVectorMaxLeft(PPCVRegister& vector, PPCVRegister& right, PPCFPSCRRegister& fpscr);
void SkateVectorMaxRight(PPCVRegister& vector, PPCVRegister& left, PPCFPSCRRegister& fpscr);
void SkateVectorMin(PPCVRegister& destination, PPCVRegister& left,
                    PPCVRegister& right, PPCFPSCRRegister& fpscr);
void SkateVectorMinLeft(PPCVRegister& vector, PPCVRegister& right, PPCFPSCRRegister& fpscr);
void SkateVectorMinRight(PPCVRegister& vector, PPCVRegister& left, PPCFPSCRRegister& fpscr);
