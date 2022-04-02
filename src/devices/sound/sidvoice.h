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
		uint16_t len = 0;
#if defined(DIRECT_FIXPOINT)
		uint32_t stp = 0;
#else
		uint32_t pnt = 0;
		int16_t stp = 0;
#endif
	};

	SID6581_t *sid = nullptr;
	uint8_t reg[7]{ 0 };
	uint32_t SIDfreq = 0;
	uint16_t SIDpulseWidth = 0;
	uint8_t SIDctrl = 0;
	uint8_t SIDAD = 0, SIDSR = 0;

	sidOperator *carrier = nullptr;
	sidOperator *modulator = nullptr;
	int sync = 0;

	uint16_t pulseIndex = 0, newPulseIndex = 0;
	uint16_t curSIDfreq = 0;
	uint16_t curNoiseFreq = 0;

	uint8_t output = 0/*, outputMask = 0*/;

	char filtVoiceMask = 0;
	int filtEnabled = 0;
	float filtLow = 0, filtRef = 0;
	int8_t filtIO = 0;

	int32_t cycleLenCount = 0;
#if defined(DIRECT_FIXPOINT)
	PAIR cycleLen, cycleAddLen;
#else
	uint32_t cycleAddLenPnt = 0;
	uint16_t cycleLen, cycleLenPnt = 0;
#endif

	int8_t (*outProc)(sidOperator *) = nullptr;
	void (*waveProc)(sidOperator *) = nullptr;

#if defined(DIRECT_FIXPOINT)
	PAIR waveStep, waveStepAdd;
#else
	uint16_t waveStep = 0, waveStepAdd = 0;
	uint32_t waveStepPnt = 0, waveStepAddPnt = 0;
#endif
	uint16_t waveStepOld = 0;
	sw_storage wavePre[2];

#if defined(DIRECT_FIXPOINT)
	PAIR noiseReg;
#else
	uint32_t noiseReg = 0;
#endif
	uint32_t noiseStep = 0, noiseStepAdd = 0;
	uint8_t noiseOutput = 0;
	int noiseIsLocked = 0;

	uint8_t ADSRctrl = 0;
//  int gateOnCtrl = 0, gateOffCtrl = 0;
	uint16_t (*ADSRproc)(sidOperator *) = nullptr;

#ifdef SID_FPUENVE
	float fenveStep = 0.0, fenveStepAdd = 0.0;
	uint32_t enveStep = 0;
#elif defined(DIRECT_FIXPOINT)
	PAIR enveStep, enveStepAdd;
#else
	uint16_t enveStep = 0, enveStepAdd = 0;
	uint32_t enveStepPnt = 0, enveStepAddPnt = 0;
#endif
	uint8_t enveVol = 0, enveSusVol = 0;
	uint16_t enveShortAttackCount = 0;

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
