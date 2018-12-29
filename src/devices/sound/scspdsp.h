// license:BSD-3-Clause
// copyright-holders:ElSemi, R. Belmont
#ifndef MAME_SOUND_SCSPDSP_H
#define MAME_SOUND_SCSPDSP_H

#pragma once

//the DSP Context
struct SCSPDSP
{
//Config
	address_space *space;
	uint32_t RBP; //Ring buf pointer
	uint32_t RBL; //Delay ram (Ring buffer) size in words

//context

	int16_t COEF[64];     //16 bit signed
	uint16_t MADRS[32];   //offsets (in words), 16 bit
	uint16_t MPRO[128*4]; //128 steps 64 bit
	int32_t TEMP[128];    //TEMP regs,24 bit signed
	int32_t MEMS[32]; //MEMS regs,24 bit signed
	uint32_t DEC;

//input
	int32_t MIXS[16]; //MIXS, 24 bit signed
	int16_t EXTS[2];  //External inputs (CDDA)    16 bit signed

//output
	int16_t EFREG[16];    //EFREG, 16 bit signed

	bool Stopped;
	int LastStep;

	void Init();
	void SetSample(int32_t sample, int32_t SEL, int32_t MXL);
	void Step();
	void Start();
};

#endif // MAME_SOUND_SCSPDSP_H
