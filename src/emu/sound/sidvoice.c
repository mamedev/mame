#include <math.h>

#include "sidvoice.h"
#include "sid.h"
#include "sidenvel.h"
#include "sidw6581.h"
#include "sidw8580.h"

static UINT8 triangleTable[4096];
static UINT8 sawtoothTable[4096];
static UINT8 squareTable[2*4096];
static UINT8* waveform30;
static UINT8* waveform50;
static UINT8* waveform60;
static UINT8* waveform70;
#if defined(LARGE_NOISE_TABLE)
  static UINT8 noiseTableMSB[1<<8];
  static UINT8 noiseTableLSB[1L<<16];
#else
  static UINT8 noiseTableMSB[1<<8];
  static UINT8 noiseTableMID[1<<8];
  static UINT8 noiseTableLSB[1<<8];
#endif

static INT8* ampMod1x8;

static const UINT32 noiseSeed = 0x7ffff8;

void sidInitMixerEngine(void)
{
	UINT16 uk;
	INT32 si, sj    ;

	/* 8-bit volume modulation tables. */
	float filterAmpl = 1.0;

	filterAmpl = 0.7;

	ampMod1x8=(INT8*) auto_malloc(256*256);

	uk = 0;
	for ( si = 0; si < 256; si++ )
	{
		for ( sj = -128; sj < 128; sj++, uk++ )
		{
			ampMod1x8[uk] = (INT8)(((si*sj)/255)*filterAmpl);
		}
	}

}

INLINE void waveAdvance(sidOperator* pVoice)
{
#if defined(DIRECT_FIXPOINT)
	pVoice->waveStep.l += pVoice->waveStepAdd.l;
	pVoice->waveStep.w[HI] &= 4095;
#else
	pVoice->waveStepPnt += pVoice->waveStepAddPnt;
	pVoice->waveStep += pVoice->waveStepAdd;
	if (pVoice->waveStepPnt > 65535 ) pVoice->waveStep++;
	pVoice->waveStepPnt &= 0xFFFF;
	pVoice->waveStep &= 4095;
#endif
}

INLINE void noiseAdvance(sidOperator* pVoice)
{
	pVoice->noiseStep += pVoice->noiseStepAdd;
	if (pVoice->noiseStep >= (1L<<20))
	{
		pVoice->noiseStep -= (1L<<20);
#if defined(DIRECT_FIXPOINT)
		pVoice->noiseReg.l = (pVoice->noiseReg.l << 1) |
			(((pVoice->noiseReg.l >> 22) ^ (pVoice->noiseReg.l >> 17)) & 1);
#else
		pVoice->noiseReg = (pVoice->noiseReg << 1) |
			(((pVoice->noiseReg >> 22) ^ (pVoice->noiseReg >> 17)) & 1);
#endif
#if defined(DIRECT_FIXPOINT) && defined(LARGE_NOISE_TABLE)
		pVoice->noiseOutput = (noiseTableLSB[pVoice->noiseReg.w[LO]]
							   |noiseTableMSB[pVoice->noiseReg.w[HI]&0xff]);
#elif defined(DIRECT_FIXPOINT)
		pVoice->noiseOutput = (noiseTableLSB[pVoice->noiseReg.b[LOLO]]
							   |noiseTableMID[pVoice->noiseReg.b[LOHI]]
							   |noiseTableMSB[pVoice->noiseReg.b[HILO]]);
#else
		pVoice->noiseOutput = (noiseTableLSB[pVoice->noiseReg&0xff]
							   |noiseTableMID[pVoice->noiseReg>>8&0xff]
							   |noiseTableMSB[pVoice->noiseReg>>16&0xff]);
#endif
	}
}

INLINE void noiseAdvanceHp(sidOperator* pVoice)
{
	UINT32 tmp = pVoice->noiseStepAdd;
	while (tmp >= (1L<<20))
	{
		tmp -= (1L<<20);
#if defined(DIRECT_FIXPOINT)
		pVoice->noiseReg.l = (pVoice->noiseReg.l << 1) |
			(((pVoice->noiseReg.l >> 22) ^ (pVoice->noiseReg.l >> 17)) & 1);
#else
		pVoice->noiseReg = (pVoice->noiseReg << 1) |
			(((pVoice->noiseReg >> 22) ^ (pVoice->noiseReg >> 17)) & 1);
#endif
	}
	pVoice->noiseStep += tmp;
	if (pVoice->noiseStep >= (1L<<20))
	{
		pVoice->noiseStep -= (1L<<20);
#if defined(DIRECT_FIXPOINT)
		pVoice->noiseReg.l = (pVoice->noiseReg.l << 1) |
			(((pVoice->noiseReg.l >> 22) ^ (pVoice->noiseReg.l >> 17)) & 1);
#else
		pVoice->noiseReg = (pVoice->noiseReg << 1) |
			(((pVoice->noiseReg >> 22) ^ (pVoice->noiseReg >> 17)) & 1);
#endif
	}
#if defined(DIRECT_FIXPOINT) && defined(LARGE_NOISE_TABLE)
	pVoice->noiseOutput = (noiseTableLSB[pVoice->noiseReg.w[LO]]
						   |noiseTableMSB[pVoice->noiseReg.w[HI]&0xff]);
#elif defined(DIRECT_FIXPOINT)
	pVoice->noiseOutput = (noiseTableLSB[pVoice->noiseReg.b[LOLO]]
						   |noiseTableMID[pVoice->noiseReg.b[LOHI]]
						   |noiseTableMSB[pVoice->noiseReg.b[HILO]]);
#else
	pVoice->noiseOutput = (noiseTableLSB[pVoice->noiseReg&0xff]
						   |noiseTableMID[pVoice->noiseReg>>8&0xff]
						   |noiseTableMSB[pVoice->noiseReg>>16&0xff]);
#endif
}


#if defined(DIRECT_FIXPOINT)
  #define triangle triangleTable[pVoice->waveStep.w[HI]]
  #define sawtooth sawtoothTable[pVoice->waveStep.w[HI]]
  #define square squareTable[pVoice->waveStep.w[HI] + pVoice->pulseIndex]
  #define triSaw waveform30[pVoice->waveStep.w[HI]]
  #define triSquare waveform50[pVoice->waveStep.w[HI] + pVoice->SIDpulseWidth]
  #define sawSquare waveform60[pVoice->waveStep.w[HI] + pVoice->SIDpulseWidth]
  #define triSawSquare waveform70[pVoice->waveStep.w[HI] + pVoice->SIDpulseWidth]
#else
  #define triangle triangleTable[pVoice->waveStep]
  #define sawtooth sawtoothTable[pVoice->waveStep]
  #define square squareTable[pVoice->waveStep + pVoice->pulseIndex]
  #define triSaw waveform30[pVoice->waveStep]
  #define triSquare waveform50[pVoice->waveStep + pVoice->SIDpulseWidth]
  #define sawSquare waveform60[pVoice->waveStep + pVoice->SIDpulseWidth]
  #define triSawSquare waveform70[pVoice->waveStep + pVoice->SIDpulseWidth]
#endif


static void sidMode00(sidOperator* pVoice)  {
	pVoice->output = (pVoice->filtIO-0x80);
	waveAdvance(pVoice);
}

#if 0
/* not used */
static void sidModeReal00(sidOperator* pVoice)  {
	pVoice->output = 0;
	waveAdvance(pVoice);
}
#endif

static void sidMode10(sidOperator* pVoice)  {
  pVoice->output = triangle;
  waveAdvance(pVoice);
}

static void sidMode20(sidOperator* pVoice)  {
  pVoice->output = sawtooth;
  waveAdvance(pVoice);
}

static void sidMode30(sidOperator* pVoice)  {
  pVoice->output = triSaw;
  waveAdvance(pVoice);
}

static void sidMode40(sidOperator* pVoice)  {
  pVoice->output = square;
  waveAdvance(pVoice);
}

static void sidMode50(sidOperator* pVoice)  {
  pVoice->output = triSquare;
  waveAdvance(pVoice);
}

static void sidMode60(sidOperator* pVoice)  {
  pVoice->output = sawSquare;
  waveAdvance(pVoice);
}

static void sidMode70(sidOperator* pVoice)  {
  pVoice->output = triSawSquare;
  waveAdvance(pVoice);
}

static void sidMode80(sidOperator* pVoice)  {
  pVoice->output = pVoice->noiseOutput;
  waveAdvance(pVoice);
  noiseAdvance(pVoice);
}

static void sidMode80hp(sidOperator* pVoice)  {
  pVoice->output = pVoice->noiseOutput;
  waveAdvance(pVoice);
  noiseAdvanceHp(pVoice);
}

static void sidModeLock(sidOperator* pVoice)
{
	pVoice->noiseIsLocked = TRUE;
	pVoice->output = (pVoice->filtIO-0x80);
	waveAdvance(pVoice);
}

/* */
/* */
/* */

static void sidMode14(sidOperator* pVoice)
{
#if defined(DIRECT_FIXPOINT)
  if ( pVoice->modulator->waveStep.w[HI] < 2048 )
#else
  if ( pVoice->modulator->waveStep < 2048 )
#endif
	pVoice->output = triangle;
  else
	pVoice->output = 0xFF ^ triangle;
  waveAdvance(pVoice);
}

static void sidMode34(sidOperator* pVoice)  {
#if defined(DIRECT_FIXPOINT)
  if ( pVoice->modulator->waveStep.w[HI] < 2048 )
#else
  if ( pVoice->modulator->waveStep < 2048 )
#endif
	pVoice->output = triSaw;
  else
	pVoice->output = 0xFF ^ triSaw;
  waveAdvance(pVoice);
}

static void sidMode54(sidOperator* pVoice)  {
#if defined(DIRECT_FIXPOINT)
  if ( pVoice->modulator->waveStep.w[HI] < 2048 )
#else
  if ( pVoice->modulator->waveStep < 2048 )
#endif
	pVoice->output = triSquare;
  else
    pVoice->output = 0xFF ^ triSquare;
  waveAdvance(pVoice);
}

static void sidMode74(sidOperator* pVoice)  {
#if defined(DIRECT_FIXPOINT)
  if ( pVoice->modulator->waveStep.w[HI] < 2048 )
#else
  if ( pVoice->modulator->waveStep < 2048 )
#endif
	pVoice->output = triSawSquare;
  else
    pVoice->output = 0xFF ^ triSawSquare;
  waveAdvance(pVoice);
}

/* */
/* */
/* */

INLINE void waveCalcCycleLen(sidOperator* pVoice)
{
#if defined(DIRECT_FIXPOINT)
	pVoice->cycleAddLen.w[HI] = 0;
	pVoice->cycleAddLen.l += pVoice->cycleLen.l;
	pVoice->cycleLenCount = pVoice->cycleAddLen.w[HI];
#else
	pVoice->cycleAddLenPnt += pVoice->cycleLenPnt;
	pVoice->cycleLenCount = pVoice->cycleLen;
	if ( pVoice->cycleAddLenPnt > 65535 ) pVoice->cycleLenCount++;
	pVoice->cycleAddLenPnt &= 0xFFFF;
#endif
	/* If we keep the value cycleLen between 1 <= x <= 65535, */
	/* the following check is not required. */
/*  if ( pVoice->cycleLenCount == 0 ) */
/*  { */
/*#if defined(DIRECT_FIXPOINT) */
/*      pVoice->waveStep.l = 0; */
/*#else */
/*      pVoice->waveStep = (pVoice->waveStepPnt = 0); */
/*#endif */
/*      pVoice->cycleLenCount = 0; */
/*  } */
/*  else */
	{
#if defined(DIRECT_FIXPOINT)
		register UINT16 diff = pVoice->cycleLenCount - pVoice->cycleLen.w[HI];
#else
		register UINT16 diff = pVoice->cycleLenCount - pVoice->cycleLen;
#endif
		if ( pVoice->wavePre[diff].len != pVoice->cycleLenCount )
		{
			pVoice->wavePre[diff].len = pVoice->cycleLenCount;
#if defined(DIRECT_FIXPOINT)
			pVoice->wavePre[diff].stp = (pVoice->waveStepAdd.l = (4096UL*65536UL) / pVoice->cycleLenCount);
#else
			pVoice->wavePre[diff].stp = (pVoice->waveStepAdd = 4096UL / pVoice->cycleLenCount);
			pVoice->wavePre[diff].pnt = (pVoice->waveStepAddPnt = ((4096UL % pVoice->cycleLenCount) * 65536UL) / pVoice->cycleLenCount);
#endif
		}
		else
		{
#if defined(DIRECT_FIXPOINT)
			pVoice->waveStepAdd.l = pVoice->wavePre[diff].stp;
#else
			pVoice->waveStepAdd = pVoice->wavePre[diff].stp;
			pVoice->waveStepAddPnt = pVoice->wavePre[diff].pnt;
#endif
		}
	}  /* see above (opening bracket) */
}

INLINE void waveCalcFilter(sidOperator* pVoice)
{
	if ( pVoice->filtEnabled )
	{
		if ( pVoice->sid->filter.Type != 0 )
		{
			if ( pVoice->sid->filter.Type == 0x20 )
			{
				float tmp;
				pVoice->filtLow += ( pVoice->filtRef * pVoice->sid->filter.Dy );
				tmp = (float)pVoice->filtIO - pVoice->filtLow;
				tmp -= pVoice->filtRef * pVoice->sid->filter.ResDy;
				pVoice->filtRef += ( tmp * (pVoice->sid->filter.Dy) );
				pVoice->filtIO = (INT8)(pVoice->filtRef-pVoice->filtLow/4);
			}
			else if (pVoice->sid->filter.Type == 0x40)
			{
				float tmp, tmp2;
				pVoice->filtLow += ( pVoice->filtRef * pVoice->sid->filter.Dy * 0.1 );
				tmp = (float)pVoice->filtIO - pVoice->filtLow;
				tmp -= pVoice->filtRef * pVoice->sid->filter.ResDy;
				pVoice->filtRef += ( tmp * (pVoice->sid->filter.Dy) );
				tmp2 = pVoice->filtRef - pVoice->filtIO/8;
				if (tmp2 < -128)
					tmp2 = -128;
				if (tmp2 > 127)
					tmp2 = 127;
				pVoice->filtIO = (INT8)tmp2;
			}
			else
			{
				float sample, sample2;
				int tmp;
				pVoice->filtLow += ( pVoice->filtRef * pVoice->sid->filter.Dy );
				sample = pVoice->filtIO;
				sample2 = sample - pVoice->filtLow;
				tmp = (int)sample2;
				sample2 -= pVoice->filtRef * pVoice->sid->filter.ResDy;
				pVoice->filtRef += ( sample2 * pVoice->sid->filter.Dy );

				if ( pVoice->sid->filter.Type == 0x10 )
				{
					pVoice->filtIO = (INT8)pVoice->filtLow;
				}
				else if ( pVoice->sid->filter.Type == 0x30 )
				{
					pVoice->filtIO = (INT8)pVoice->filtLow;
				}
				else if ( pVoice->sid->filter.Type == 0x50 )
				{
					pVoice->filtIO = (INT8)(sample - (tmp >> 1));
				}
				else if ( pVoice->sid->filter.Type == 0x60 )
				{
					pVoice->filtIO = (INT8)tmp;
				}
				else if ( pVoice->sid->filter.Type == 0x70 )
				{
					pVoice->filtIO = (INT8)(sample - (tmp >> 1));
				}
			}
		}
		else /* pVoice->sid->filter.Type == 0x00 */
		{
			pVoice->filtIO = 0;
		}
	}
}

static INT8 waveCalcMute(sidOperator* pVoice)
{
	(*pVoice->ADSRproc)(pVoice);  /* just process envelope */
	return pVoice->filtIO;//&pVoice->outputMask;
}


INT8 sidWaveCalcNormal(sidOperator* pVoice)
{
	if ( pVoice->cycleLenCount <= 0 )
	{
		waveCalcCycleLen(pVoice);
		if (( pVoice->SIDctrl & 0x40 ) == 0x40 )
		{
			pVoice->pulseIndex = pVoice->newPulseIndex;
			if ( pVoice->pulseIndex > 2048 )
			{
#if defined(DIRECT_FIXPOINT)
				pVoice->waveStep.w[HI] = 0;
#else
				pVoice->waveStep = 0;
#endif
			}
		}
	}
	(*pVoice->waveProc)(pVoice);
	pVoice->filtIO = ampMod1x8[(*pVoice->ADSRproc)(pVoice)|pVoice->output];
//  pVoice->filtIO = pVoice->sid->masterVolume; // test for digi sound
	waveCalcFilter(pVoice);
	return pVoice->filtIO;//&pVoice->outputMask;
}


static INT8 waveCalcRangeCheck(sidOperator* pVoice)
{
#if defined(DIRECT_FIXPOINT)
	pVoice->waveStepOld = pVoice->waveStep.w[HI];
	(*pVoice->waveProc)(pVoice);
	if (pVoice->waveStep.w[HI] < pVoice->waveStepOld)
#else
	pVoice->waveStepOld = pVoice->waveStep;
	(*pVoice->waveProc)(pVoice);
	if (pVoice->waveStep < pVoice->waveStepOld)
#endif
	{
		/* Next step switch back to normal calculation. */
		pVoice->cycleLenCount = 0;
		pVoice->outProc = &sidWaveCalcNormal;
#if defined(DIRECT_FIXPOINT)
				pVoice->waveStep.w[HI] = 4095;
#else
				pVoice->waveStep = 4095;
#endif
	}
	pVoice->filtIO = ampMod1x8[(*pVoice->ADSRproc)(pVoice)|pVoice->output];
	waveCalcFilter(pVoice);
	return pVoice->filtIO;//&pVoice->outputMask;
}

/* -------------------------------------------------- Operator frame set-up 1 */

void sidEmuSet(sidOperator* pVoice)
{
	UINT8 enveTemp, newWave, oldWave;
	UINT8 ADtemp;
	UINT8 SRtemp;
	UINT8 tmpSusVol;

	pVoice->SIDfreq = pVoice->reg[0]|(pVoice->reg[1]<<8);

	pVoice->SIDpulseWidth = (pVoice->reg[2]|(pVoice->reg[3]<<8)) & 0x0FFF;
	pVoice->newPulseIndex = 4096 - pVoice->SIDpulseWidth;
#if defined(DIRECT_FIXPOINT)
	if ( ((pVoice->waveStep.w[HI] + pVoice->pulseIndex) >= 0x1000)
		&& ((pVoice->waveStep.w[HI] + pVoice->newPulseIndex) >= 0x1000) )
	{
		pVoice->pulseIndex = pVoice->newPulseIndex;
	}
	else if ( ((pVoice->waveStep.w[HI] + pVoice->pulseIndex) < 0x1000)
			&& ((pVoice->waveStep.w[HI] + pVoice->newPulseIndex) < 0x1000) )
	{
		pVoice->pulseIndex = pVoice->newPulseIndex;
	}
#else
	if ( ((pVoice->waveStep + pVoice->pulseIndex) >= 0x1000)
		&& ((pVoice->waveStep + pVoice->newPulseIndex) >= 0x1000) )
	{
		pVoice->pulseIndex = pVoice->newPulseIndex;
	}
	else if ( ((pVoice->waveStep + pVoice->pulseIndex) < 0x1000)
			&& ((pVoice->waveStep + pVoice->newPulseIndex) < 0x1000) )
	{
		pVoice->pulseIndex = pVoice->newPulseIndex;
	}
#endif


    oldWave = pVoice->SIDctrl;
    enveTemp = pVoice->ADSRctrl;
    pVoice->SIDctrl = (newWave = pVoice->reg[4]|(pVoice->reg[5]<<8));

    if (( newWave & 1 ) ==0 )
    {
		if (( oldWave & 1 ) !=0 )
		  enveTemp = ENVE_STARTRELEASE;
/*      else if ( pVoice->gateOnCtrl ) */
/*      { */
/*          enveTemp = ENVE_STARTSHORTATTACK; */
/*      } */
    }
	else if ( /*pVoice->gateOffCtrl || */((oldWave&1)==0) )
	{
		enveTemp = ENVE_STARTATTACK;
	}

	if ((( oldWave ^ newWave ) & 0xF0 ) != 0 )
	{
		pVoice->cycleLenCount = 0;
	}

	ADtemp = pVoice->reg[5];
	SRtemp = pVoice->reg[6];
	if ( pVoice->SIDAD != ADtemp )
	{
		enveTemp |= ENVE_ALTER;
	}
	else if ( pVoice->SIDSR != SRtemp )
	{
		enveTemp |= ENVE_ALTER;
	}
	pVoice->SIDAD = ADtemp;
	pVoice->SIDSR = SRtemp;
	tmpSusVol = masterVolumeLevels[SRtemp >> 4];
	if (pVoice->ADSRctrl != ENVE_SUSTAIN)  /* !!! */
	{
		pVoice->enveSusVol = tmpSusVol;
	}
	else
	{
		if ( pVoice->enveSusVol > pVoice->enveVol )
			pVoice->enveSusVol = 0;
		else
			pVoice->enveSusVol = tmpSusVol;
	}

	pVoice->ADSRproc = enveModeTable[enveTemp>>1];  /* shifting out the KEY-bit */
	pVoice->ADSRctrl = enveTemp & (255-ENVE_ALTER-1);

	pVoice->filtEnabled = pVoice->sid->filter.Enabled &&
		((pVoice->sid->reg[0x17] & pVoice->filtVoiceMask)!=0);
}

/* -------------------------------------------------- Operator frame set-up 2 */

/* MOS-8580, MOS-6581 (no 70) */
static ptr2sidVoidFunc sidModeNormalTable[16] =
{
  sidMode00, sidMode10, sidMode20, sidMode30, sidMode40, sidMode50, sidMode60, sidMode70,
  sidMode80, sidModeLock, sidModeLock, sidModeLock, sidModeLock, sidModeLock, sidModeLock, sidModeLock
};

/* MOS-8580, MOS-6581 (no 74) */
static ptr2sidVoidFunc sidModeRingTable[16] =
{
  sidMode00, sidMode14, sidMode00, sidMode34, sidMode00, sidMode54, sidMode00, sidMode74,
  sidModeLock, sidModeLock, sidModeLock, sidModeLock, sidModeLock, sidModeLock, sidModeLock, sidModeLock
};

void sidClearOperator( sidOperator* pVoice )
{
	pVoice->SIDfreq = 0;
	pVoice->SIDctrl = 0;
	pVoice->SIDAD = 0;
	pVoice->SIDSR = 0;

	pVoice->sync = FALSE;

	pVoice->pulseIndex = (pVoice->newPulseIndex = (pVoice->SIDpulseWidth = 0));
	pVoice->curSIDfreq = (pVoice->curNoiseFreq = 0);

	pVoice->output = (pVoice->noiseOutput = 0);
	pVoice->filtIO = 0;

	pVoice->filtEnabled = FALSE;
	pVoice->filtLow = (pVoice->filtRef = 0);

	pVoice->cycleLenCount = 0;
#if defined(DIRECT_FIXPOINT)
	pVoice->cycleLen.l = (pVoice->cycleAddLen.l = 0);
#else
	pVoice->cycleLen = (pVoice->cycleLenPnt = 0);
	pVoice->cycleAddLenPnt = 0;
#endif

	pVoice->outProc = waveCalcMute;

#if defined(DIRECT_FIXPOINT)
	pVoice->waveStepAdd.l = (pVoice->waveStep.l = 0);
	pVoice->wavePre[0].len = (pVoice->wavePre[0].stp = 0);
	pVoice->wavePre[1].len = (pVoice->wavePre[1].stp = 0);
#else
	pVoice->waveStepAdd = (pVoice->waveStepAddPnt = 0);
	pVoice->waveStep = (pVoice->waveStepPnt = 0);
	pVoice->wavePre[0].len = 0;
	pVoice->wavePre[0].stp = (pVoice->wavePre[0].pnt = 0);
	pVoice->wavePre[1].len = 0;
	pVoice->wavePre[1].stp = (pVoice->wavePre[1].pnt = 0);
#endif
	pVoice->waveStepOld = 0;

#if defined(DIRECT_FIXPOINT)
	pVoice->noiseReg.l = noiseSeed;
#else
	pVoice->noiseReg = noiseSeed;
#endif
	pVoice->noiseStepAdd = (pVoice->noiseStep = 0);
	pVoice->noiseIsLocked = FALSE;
}

void sidEmuSet2(sidOperator* pVoice)
{
    pVoice->outProc = &sidWaveCalcNormal;
    pVoice->sync = FALSE;

    if ( (pVoice->SIDfreq < 16) || ((pVoice->SIDctrl & 8) != 0) )
//    if ( /*(pVoice->SIDfreq < 16) || */((pVoice->SIDctrl & 8) != 0) )
    {
	pVoice->outProc = waveCalcMute;
	if (pVoice->SIDfreq == 0)
	{
#if defined(DIRECT_FIXPOINT)
	    pVoice->cycleLen.l = (pVoice->cycleAddLen.l = 0);
	    pVoice->waveStep.l = 0;
#else
	    pVoice->cycleLen = (pVoice->cycleLenPnt = 0);
	    pVoice->cycleAddLenPnt = 0;
	    pVoice->waveStep = 0;
	    pVoice->waveStepPnt = 0;
#endif
	    pVoice->curSIDfreq = (pVoice->curNoiseFreq = 0);
	    pVoice->noiseStepAdd = 0;
	    pVoice->cycleLenCount = 0;
	}
	if ((pVoice->SIDctrl & 8) != 0)
	{
	    if (pVoice->noiseIsLocked)
	    {
		pVoice->noiseIsLocked = FALSE;
#if defined(DIRECT_FIXPOINT)
		pVoice->noiseReg.l = noiseSeed;
#else
		pVoice->noiseReg = noiseSeed;
#endif
	    }
	}
    }
    else
    {
	if ( pVoice->curSIDfreq != pVoice->SIDfreq )
	{
	    pVoice->curSIDfreq = pVoice->SIDfreq;
	    /* We keep the value cycleLen between 1 <= x <= 65535. */
	    /* This makes a range-check in waveCalcCycleLen() unrequired. */
#if defined(DIRECT_FIXPOINT)
	    pVoice->cycleLen.l = ((pVoice->sid->PCMsid << 12) / pVoice->SIDfreq) << 4;
	    if (pVoice->cycleLenCount > 0)
	    {
		waveCalcCycleLen(pVoice);
		pVoice->outProc = &waveCalcRangeCheck;
	    }
#else
	    pVoice->cycleLen = pVoice->sid->PCMsid / pVoice->SIDfreq;
	    pVoice->cycleLenPnt = (( pVoice->sid->PCMsid % pVoice->SIDfreq ) * 65536UL ) / pVoice->SIDfreq;
	    if (pVoice->cycleLenCount > 0)
	    {
		waveCalcCycleLen(pVoice);
		pVoice->outProc = &waveCalcRangeCheck;
	    }
#endif
	}

	if ((( pVoice->SIDctrl & 0x80 ) == 0x80 ) && ( pVoice->curNoiseFreq != pVoice->SIDfreq ))
	{
	    pVoice->curNoiseFreq = pVoice->SIDfreq;
	    pVoice->noiseStepAdd = (pVoice->sid->PCMsidNoise * pVoice->SIDfreq) >> 8;
	    if (pVoice->noiseStepAdd >= (1L<<21))
		sidModeNormalTable[8] = sidMode80hp;
	    else
		sidModeNormalTable[8] = sidMode80;
	}

	if (( pVoice->SIDctrl & 2 ) != 0 )
	{
	    if ( ( pVoice->modulator->SIDfreq == 0 ) || (( pVoice->modulator->SIDctrl & 8 ) != 0 ) )
	    {
		;
	    }
	    else if ( (( pVoice->carrier->SIDctrl & 2 ) != 0 ) &&
		      ( pVoice->modulator->SIDfreq >= ( pVoice->SIDfreq << 1 )) )
	    {
		;
	    }
	    else
	    {
		pVoice->sync = TRUE;
	    }
	}

	if ((( pVoice->SIDctrl & 0x14 ) == 0x14 ) && ( pVoice->modulator->SIDfreq != 0 ))
	    pVoice->waveProc = sidModeRingTable[pVoice->SIDctrl >> 4];
	else
	    pVoice->waveProc = sidModeNormalTable[pVoice->SIDctrl >> 4];
    }
}

void sidInitWaveformTables(SIDTYPE type)
{
	int i,j;
	UINT16 k;

	k = 0;
	for ( i = 0; i < 256; i++ )
		for ( j = 0; j < 8; j++ )
			triangleTable[k++] = i;
	for ( i = 255; i >= 0; i-- )
		for ( j = 0; j < 8; j++ )
			triangleTable[k++] = i;

	k = 0;
	for ( i = 0; i < 256; i++ )
		for ( j = 0; j < 16; j++ )
			sawtoothTable[k++] = i;

	k = 0;
	for ( i = 0; i < 4096; i++ )
	    squareTable[k++] = 255; //0; my estimation; especial for digi sound
	for ( i = 0; i < 4096; i++ )
	    squareTable[k++] = 0; //255;

	if ( type==MOS8580 )
	{
        waveform30 = waveform30_8580;
        waveform50 = waveform50_8580;
        waveform60 = waveform60_8580;
        waveform70 = waveform70_8580;
	}
	else
	{
        waveform30 = waveform30_6581;
        waveform50 = waveform50_6581;
        waveform60 = waveform60_6581;
        waveform70 = waveform70_6581;  /* really audible? */
	}

	for ( i = 4096; i < 8192; i++ )
	{
		waveform50[i] = 0;
		waveform60[i] = 0;
		waveform70[i] = 0;
	}

	if ( type==MOS8580 )
	{
		sidModeNormalTable[3] = sidMode30;
		sidModeNormalTable[6] = sidMode60;
		sidModeNormalTable[7] = sidMode70;
		sidModeRingTable[7] = sidMode74;
	}
	else
	{
		sidModeNormalTable[3] = sidMode30;
		sidModeNormalTable[6] = sidMode60;
		sidModeNormalTable[7] = sidMode00;  /* really audible? */
		sidModeRingTable[7] = sidMode00;    /* */
	}

	{
#if defined(LARGE_NOISE_TABLE)
	UINT32 ni;
	for (ni = 0; ni < sizeof(noiseTableLSB); ni++)
	{
		noiseTableLSB[ni] = (UINT8)
			(((ni >> (13-4)) & 0x10) |
			 ((ni >> (11-3)) & 0x08) |
			 ((ni >> (7-2)) & 0x04) |
			 ((ni >> (4-1)) & 0x02) |
			 ((ni >> (2-0)) & 0x01));
	}
	for (ni = 0; ni < sizeof(noiseTableMSB); ni++)
	{
		noiseTableMSB[ni] = (UINT8)
			(((ni << (7-(22-16))) & 0x80) |
			 ((ni << (6-(20-16))) & 0x40) |
			 ((ni << (5-(16-16))) & 0x20));
	}
#else
	UINT32 ni;
	for (ni = 0; ni < sizeof(noiseTableLSB); ni++)
	{
		noiseTableLSB[ni] = (UINT8)
			(((ni >> (7-2)) & 0x04) |
			 ((ni >> (4-1)) & 0x02) |
			 ((ni >> (2-0)) & 0x01));
	}
	for (ni = 0; ni < sizeof(noiseTableMID); ni++)
	{
		noiseTableMID[ni] = (UINT8)
			(((ni >> (13-8-4)) & 0x10) |
			 ((ni << (3-(11-8))) & 0x08));
	}
	for (ni = 0; ni < sizeof(noiseTableMSB); ni++)
	{
		noiseTableMSB[ni] = (UINT8)
			(((ni << (7-(22-16))) & 0x80) |
			 ((ni << (6-(20-16))) & 0x40) |
			 ((ni << (5-(16-16))) & 0x20));
	}
#endif
	}
}

