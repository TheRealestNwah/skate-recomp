#pragma once

#include <rex/ppc/context.h>

void SkateSetFlickitRate(PPCRegister& rate);
bool SkateSkipPresentationInput(PPCRegister& ring_offset);
void SkateSelectInputButtonTiming(PPCRegister& return_address, PPCRegister& ring_offset);
void SkateAdjustInputRepeat(PPCRegister& initial_delay, PPCRegister& repeat_delay);
void SkateAdjustInputDebounce(PPCRegister& samples, PPCCRRegister& comparison,
                              PPCXERRegister& xer);
