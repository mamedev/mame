// license:BSD-3-Clause
// copyright-holders:R. Belmont
/***************************************************************************

    dsp3210dau.h
    The DSP3210 data arithmetic unit's accumulator type; the datapath
    itself is in dsp3210dau.hxx.

***************************************************************************/

#ifndef MAME_CPU_DSP32_DSP3210DAU_H
#define MAME_CPU_DSP32_DSP3210DAU_H

#pragma once

#include <cstdint>

namespace dsp3210dau {

// a 40-bit accumulator: the mantissa and guard bits including the
// implicit bit, as a fixed-point integer with 31 fraction bits, and the
// biased exponent (0 = zero, whatever the mantissa says)
struct acc_t
{
	int64_t m;
	int16_t e;
};

} // namespace dsp3210dau

#endif // MAME_CPU_DSP32_DSP3210DAU_H
