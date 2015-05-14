// license:BSD-3-Clause
// copyright-holders:Peter Trauner
#pragma once

#ifndef __SIDVOICE_H__
#define __SIDVOICE_H__


/*
  approximation of the sid6581 chip
  this part is for 1 (of the 3) voices of a chip
*/
#include "sound/mos6581.h"

struct sw_storage
{
	UINT16 len;
#if defined(DIRECT_FIXPOINT)
	UINT32 stp;
#else
	UINT32 pnt;
	INT16 stp;
#endif
};

struct SID6581_t;

struct sidOperator
{
	SID6581_t *sid;
	UINT8 reg[7];
	UINT32 SIDfreq;
	UINT16 SIDpulseWidth;
	UINT8 SIDctrl;
	UINT8 SIDAD, SIDSR;

	sidOperator* carrier;
	sidOperator* modulator;
	int sync;

	UINT16 pulseIndex, newPulseIndex;
	UINT16 curSIDfreq;
	UINT16 curNoiseFreq;

	UINT8 output;//, outputMask;

	char filtVoiceMask;
	int filtEnabled;
	float filtLow, filtRef;
	INT8 filtIO;

	INT32 cycleLenCount;
#if defined(DIRECT_FIXPOINT)
	cpuLword cycleLen, cycleAddLen;
#else
	UINT32 cycleAddLenPnt;
	UINT16 cycleLen, cycleLenPnt;
#endif

	INT8(*outProc)(sidOperator *);
	void(*waveProc)(sidOperator *);

#if defined(DIRECT_FIXPOINT)
	cpuLword waveStep, waveStepAdd;
#else
	UINT16 waveStep, waveStepAdd;
	UINT32 waveStepPnt, waveStepAddPnt;
#endif
	UINT16 waveStepOld;
	struct sw_storage wavePre[2];

#if defined(DIRECT_FIXPOINT) && defined(LARGE_NOISE_TABLE)
	cpuLword noiseReg;
#elif defined(DIRECT_FIXPOINT)
	cpuLBword noiseReg;
#else
	UINT32 noiseReg;
#endif
	UINT32 noiseStep, noiseStepAdd;
	UINT8 noiseOutput;
	int noiseIsLocked;

	UINT8 ADSRctrl;
//  int gateOnCtrl, gateOffCtrl;
	UINT16 (*ADSRproc)(sidOperator *);

#ifdef SID_FPUENVE
	float fenveStep, fenveStepAdd;
	UINT32 enveStep;
#elif defined(DIRECT_FIXPOINT)
	cpuLword enveStep, enveStepAdd;
#else
	UINT16 enveStep, enveStepAdd;
	UINT32 enveStepPnt, enveStepAddPnt;
#endif
	UINT8 enveVol, enveSusVol;
	UINT16 enveShortAttackCount;
};

typedef INT8 (*ptr2sidFunc)(sidOperator *);
typedef UINT16 (*ptr2sidUwordFunc)(sidOperator *);
typedef void (*ptr2sidVoidFunc)(sidOperator *);

void sidClearOperator( sidOperator* pVoice );

void sidEmuSet(sidOperator* pVoice);
void sidEmuSet2(sidOperator* pVoice);
INT8 sidWaveCalcNormal(sidOperator* pVoice);

void sidInitWaveformTables(int type);
void sidInitMixerEngine(running_machine &machine);

#if 0
extern ptr2sidVoidFunc sid6581ModeNormalTable[16];
extern ptr2sidVoidFunc sid6581ModeRingTable[16];
extern ptr2sidVoidFunc sid8580ModeNormalTable[16];
extern ptr2sidVoidFunc sid8580ModeRingTable[16];
#endif

#endif /* __SIDVOICE_H__ */
