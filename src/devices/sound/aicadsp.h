// license:BSD-3-Clause
// copyright-holders:ElSemi, Deunan Knute, R. Belmont
#ifndef MAME_SOUND_AICADSP_H
#define MAME_SOUND_AICADSP_H

#pragma once

//the DSP Context
struct AICADSP
{
	void init() noexcept;
	void setsample(s32 sample, u8 SEL, s32 MXL) noexcept;
	void step();
	void start() noexcept;

//Config
	memory_access<23, 1, 0, ENDIANNESS_LITTLE>::cache cache;
	memory_access<23, 1, 0, ENDIANNESS_LITTLE>::specific space;
	u32 RBP; //Ring buf pointer
	u32 RBL; //Delay ram (Ring buffer) size in words

//context

	s16 COEF[128 * 2];      //16 bit signed
	u16 MADRS[64 * 2]; //offsets (in words), 16 bit
	u16 MPRO[128 * 4 * 2 * 2]; //128 steps 64 bit
	s32 TEMP[128];    //TEMP regs,24 bit signed
	s32 MEMS[32]; //MEMS regs,24 bit signed
	u32 DEC;

//input
	s32 MIXS[16]; //MIXS, 24 bit signed
	s16 EXTS[2];  //External inputs (CDDA)    16 bit signed

//output
	s16 EFREG[16];    //EFREG, 16 bit signed

	bool Stopped;
	int LastStep;
};

#endif // MAME_SOUND_AICADSP_H
