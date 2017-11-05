// license:BSD-3-Clause
// copyright-holders:ElSemi, Deunan Knute, R. Belmont
#ifndef MAME_SOUND_AICADSP_H
#define MAME_SOUND_AICADSP_H

#pragma once

//the DSP Context
struct AICADSP
{
	void init();
	void setsample(int32_t sample, int32_t SEL, int32_t MXL);
	void step();
	void start();

//Config
	uint16_t *AICARAM;
	uint32_t AICARAM_LENGTH;
	uint32_t RBP; //Ring buf pointer
	uint32_t RBL; //Delay ram (Ring buffer) size in words

//context

	int16_t COEF[128*2];      //16 bit signed
	uint16_t MADRS[64*2]; //offsets (in words), 16 bit
	uint16_t MPRO[128*4*2*2]; //128 steps 64 bit
	int32_t TEMP[128];    //TEMP regs,24 bit signed
	int32_t MEMS[32]; //MEMS regs,24 bit signed
	uint32_t DEC;

//input
	int32_t MIXS[16]; //MIXS, 24 bit signed
	int16_t EXTS[2];  //External inputs (CDDA)    16 bit signed

//output
	int16_t EFREG[16];    //EFREG, 16 bit signed

	int Stopped;
	int LastStep;
};

#endif // MAME_SOUND_AICADSP_H
