// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay

#ifndef MAME_CPU_ALPHA_COMMON_H
#define MAME_CPU_ALPHA_COMMON_H

#pragma once

// instruction field extraction
#define Ra(x)     ((x >> 21) & 31)         // 'a' register field
#define Rb(x)     ((x >> 16) & 31)         // 'b' register field
#define Rc(x)     (x & 31)                 // 'c' register field
#define Im(x)     (u64(u8(x >> 13)))       // literal immediate field

#define Disp_M(x) (s64(s16(x)))            // memory instruction 16-bit signed offset
#define Disp_P(x) (s64(util::sext(x, 12))) // hardware load/store 12-bit signed offset
#define Disp_B(x) (s64(util::sext(x << 2, 23))) // branch instruction offset

#endif // MAME_CPU_ALPHA_COMMON_H
