// license:BSD-3-Clause
// copyright-holders:ElSemi, Deunan Knute, R. Belmont
#pragma once

#ifndef __AICADSP_H__
#define __AICADSP_H__

//the DSP Context
struct AICADSP
{
//Config
	UINT16 *AICARAM;
	UINT32 AICARAM_LENGTH;
	UINT32 RBP; //Ring buf pointer
	UINT32 RBL; //Delay ram (Ring buffer) size in words

//context

	INT16 COEF[128*2];      //16 bit signed
	UINT16 MADRS[64*2]; //offsets (in words), 16 bit
	UINT16 MPRO[128*4*2*2]; //128 steps 64 bit
	INT32 TEMP[128];    //TEMP regs,24 bit signed
	INT32 MEMS[32]; //MEMS regs,24 bit signed
	UINT32 DEC;

//input
	INT32 MIXS[16]; //MIXS, 24 bit signed
	INT16 EXTS[2];  //External inputs (CDDA)    16 bit signed

//output
	INT16 EFREG[16];    //EFREG, 16 bit signed

	int Stopped;
	int LastStep;
};

void aica_dsp_init(AICADSP *DSP);
void aica_dsp_setsample(AICADSP *DSP, INT32 sample, INT32 SEL, INT32 MXL);
void aica_dsp_step(AICADSP *DSP);
void aica_dsp_start(AICADSP *DSP);

#endif /* __AICADSP_H__ */
