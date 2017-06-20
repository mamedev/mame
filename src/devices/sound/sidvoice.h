// license:BSD-3-Clause
// copyright-holders:Peter Trauner
#ifndef MAME_SOUND_SIDVOICE_H
#define MAME_SOUND_SIDVOICE_H

#pragma once


/*
  approximation of the sid6581 chip
  this part is for 1 (of the 3) voices of a chip
*/
struct SID6581_t;

struct sidOperator
{
	struct sw_storage
	{
		uint16_t len;
#if defined(DIRECT_FIXPOINT)
		uint32_t stp;
#else
		uint32_t pnt;
		int16_t stp;
#endif
	};

	SID6581_t *sid;
	uint8_t reg[7];
	uint32_t SIDfreq;
	uint16_t SIDpulseWidth;
	uint8_t SIDctrl;
	uint8_t SIDAD, SIDSR;

	sidOperator* carrier;
	sidOperator* modulator;
	int sync;

	uint16_t pulseIndex, newPulseIndex;
	uint16_t curSIDfreq;
	uint16_t curNoiseFreq;

	uint8_t output;//, outputMask;

	char filtVoiceMask;
	int filtEnabled;
	float filtLow, filtRef;
	int8_t filtIO;

	int32_t cycleLenCount;
#if defined(DIRECT_FIXPOINT)
	cpuLword cycleLen, cycleAddLen;
#else
	uint32_t cycleAddLenPnt;
	uint16_t cycleLen, cycleLenPnt;
#endif

	int8_t (*outProc)(sidOperator *);
	void (*waveProc)(sidOperator *);

#if defined(DIRECT_FIXPOINT)
	cpuLword waveStep, waveStepAdd;
#else
	uint16_t waveStep, waveStepAdd;
	uint32_t waveStepPnt, waveStepAddPnt;
#endif
	uint16_t waveStepOld;
	sw_storage wavePre[2];

#if defined(DIRECT_FIXPOINT) && defined(LARGE_NOISE_TABLE)
	cpuLword noiseReg;
#elif defined(DIRECT_FIXPOINT)
	cpuLBword noiseReg;
#else
	uint32_t noiseReg;
#endif
	uint32_t noiseStep, noiseStepAdd;
	uint8_t noiseOutput;
	int noiseIsLocked;

	uint8_t ADSRctrl;
//  int gateOnCtrl, gateOffCtrl;
	uint16_t (*ADSRproc)(sidOperator *);

#ifdef SID_FPUENVE
	float fenveStep, fenveStepAdd;
	uint32_t enveStep;
#elif defined(DIRECT_FIXPOINT)
	cpuLword enveStep, enveStepAdd;
#else
	uint16_t enveStep, enveStepAdd;
	uint32_t enveStepPnt, enveStepAddPnt;
#endif
	uint8_t enveVol, enveSusVol;
	uint16_t enveShortAttackCount;

	void clear();

	void set();
	void set2();
	static int8_t wave_calc_normal(sidOperator *pVoice);

private:
	void wave_calc_cycle_len();
};

typedef int8_t (*ptr2sidFunc)(sidOperator *);
typedef uint16_t (*ptr2sidUwordFunc)(sidOperator *);
typedef void (*ptr2sidVoidFunc)(sidOperator *);

void sidInitWaveformTables(int type);
void sidInitMixerEngine(running_machine &machine);

#if 0
extern ptr2sidVoidFunc sid6581ModeNormalTable[16];
extern ptr2sidVoidFunc sid6581ModeRingTable[16];
extern ptr2sidVoidFunc sid8580ModeNormalTable[16];
extern ptr2sidVoidFunc sid8580ModeRingTable[16];
#endif

#endif // MAME_SOUND_SIDVOICE_H
