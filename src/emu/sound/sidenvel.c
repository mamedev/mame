/*========================================================================= */
/* This source implements the ADSR volume envelope of the SID-chip. */
/* Two different envelope shapes are implemented, an exponential */
/* approximation and the linear shape, which can easily be determined */
/* by reading the registers of the third SID operator. */
/* */
/* Accurate volume envelope times as of November 1994 are used */
/* courtesy of George W. Taylor <aa601@cfn.cs.dal.ca>, <yurik@io.org> */
/* They are slightly modified. */
/* */
/* To use the rounded envelope times from the C64 Programmers Reference */
/* Book define SID_REFTIMES at the Makefile level. */
/* */
/* To perform realtime calculations with floating point precision define */
/* SID_FPUENVE at the Makefile level. On high-end FPUs (not Pentium !), */
/* this can result in speed improvement. Default is integer fixpoint. */
/* */
/* Global Makefile definables: */
/* */
/*   DIRECT_FIXPOINT - use a union to access integer fixpoint operands */
/*                     in memory. This makes an assumption about the */
/*                     hardware and software architecture and therefore */
/*                     is considered a hack ! */
/* */
/* Local (or Makefile) definables: */
/* */
/*   SID_REFTIMES - use rounded envelope times */
/*   SID_FPUENVE  - use floating point precision for calculations */
/*                  (will override the global DIRECT_FIXPOINT setting !) */
/* */
/*========================================================================= */

#include <math.h>

#include "sidvoice.h"
#include "sid.h"

#include "side6581.h"
#include "sidenvel.h"

const UINT8 masterVolumeLevels[16] =
{
    0,  17,  34,  51,  68,  85, 102, 119,
  136, 153, 170, 187, 204, 221, 238, 255
};

static UINT16 masterAmplModTable[16*256];

static float attackTimes[16] =
{
  /* milliseconds */
#if defined(SID_REFTIMES)
  2,8,16,24,38,56,68,80,
  100,250,500,800,1000,3000,5000,8000
#else
  2.2528606, 8.0099577, 15.7696042, 23.7795619, 37.2963655, 55.0684591,
  66.8330845, 78.3473987,
  98.1219818, 244.554021, 489.108042, 782.472742, 977.715461, 2933.64701,
  4889.07793, 7822.72493
#endif
};

static float decayReleaseTimes[16] =
{
  /* milliseconds */
#if defined(SID_REFTIMES)
  8,24,48,72,114,168,204,240,
  300,750,1500,2400,3000,9000,15000,24000
#else
  8.91777693, 24.594051, 48.4185907, 73.0116639, 114.512475, 169.078356,
  205.199432, 240.551975,
  301.266125, 750.858245, 1501.71551, 2402.43682, 3001.89298, 9007.21405,
  15010.998, 24018.2111
#endif
};

#ifdef SID_FPUENVE
  static float attackRates[16];
  static float decayReleaseRates[16];
#elif defined(DIRECT_FIXPOINT)
  static UINT32 attackRates[16];
  static UINT32 decayReleaseRates[16];
#else
  static UINT32 attackRates[16];
  static UINT32 attackRatesP[16];
  static UINT32 decayReleaseRates[16];
  static UINT32 decayReleaseRatesP[16];
#endif

const UINT32 attackTabLen = 255;
static UINT32 releaseTabLen;
static UINT32 releasePos[256];


void enveEmuInit( UINT32 updateFreq, int measuredValues )
{
	UINT32 i, j, k;

	releaseTabLen = sizeof(releaseTab);
	for ( i = 0; i < 256; i++ )
	{
		j = 0;
		while (( j < releaseTabLen ) && (releaseTab[j] > i) )
		{
			j++;
		}
		if ( j < releaseTabLen )
		{
			releasePos[i] = j;
		}
		else
		{
			releasePos[i] = releaseTabLen -1;
		}
	}

	k = 0;
	for ( i = 0; i < 16; i++ )
	{
		for ( j = 0; j < 256; j++ )
		{
			UINT16 tmpVol = j;
			if (measuredValues)
			{
				tmpVol = (UINT16) ((293.0*(1-exp(j/-130.0)))+4.0);
				if (j == 0)
					tmpVol = 0;
				if (tmpVol > 255)
					tmpVol = 255;
			}
			/* Want the modulated volume value in the high byte. */
			masterAmplModTable[k++] = ((tmpVol * masterVolumeLevels[i]) / 255) << 8;
		}
	}

	for ( i = 0; i < 16; i++ )
	{
#ifdef SID_FPUENVE
		double scaledenvelen = floor(( attackTimes[i] * updateFreq ) / 1000UL );
		if (scaledenvelen == 0)
			scaledenvelen = 1;
		attackRates[i] = attackTabLen / scaledenvelen;

		scaledenvelen = floor(( decayReleaseTimes[i] * updateFreq ) / 1000UL );
		if (scaledenvelen == 0)
			scaledenvelen = 1;
		decayReleaseRates[i] = releaseTabLen / scaledenvelen;
#elif defined(DIRECT_FIXPOINT)
		UINT32 scaledenvelen = (UINT32)floor(( attackTimes[i] * updateFreq ) / 1000UL );
		if (scaledenvelen == 0)
			scaledenvelen = 1;
		attackRates[i] = (attackTabLen << 16) / scaledenvelen;

		scaledenvelen = (UINT32)floor(( decayReleaseTimes[i] * updateFreq ) / 1000UL );
		if (scaledenvelen == 0)
			scaledenvelen = 1;
		decayReleaseRates[i] = (releaseTabLen << 16) / scaledenvelen;
#else
		UINT32 scaledenvelen = (UINT32)(/*floor*/(( attackTimes[i] * updateFreq ) / 1000UL ));

		if (scaledenvelen == 0)
			scaledenvelen = 1;
		attackRates[i] = attackTabLen / scaledenvelen;
		attackRatesP[i] = (( attackTabLen % scaledenvelen ) * 65536UL ) / scaledenvelen;

		scaledenvelen = (UINT32)(/*floor*/(( decayReleaseTimes[i] * updateFreq ) / 1000UL ));
		if (scaledenvelen == 0)
			scaledenvelen = 1;
		decayReleaseRates[i] = releaseTabLen / scaledenvelen;
		decayReleaseRatesP[i] = (( releaseTabLen % scaledenvelen ) * 65536UL ) / scaledenvelen;
#endif
  }
}

/* Reset op. */

void enveEmuResetOperator(sidOperator* pVoice)
{
	/* mute, end of R-phase */
	pVoice->ADSRctrl = ENVE_MUTE;
//  pVoice->gateOnCtrl = (pVoice->gateOffCtrl = false);

#ifdef SID_FPUENVE
	pVoice->fenveStep = (pVoice->fenveStepAdd = 0);
	pVoice->enveStep = 0;
#elif defined(DIRECT_FIXPOINT)
	pVoice->enveStep.l = (pVoice->enveStepAdd.l = 0);
#else
	pVoice->enveStep = (pVoice->enveStepPnt = 0);
	pVoice->enveStepAdd = (pVoice->enveStepAddPnt = 0);
#endif
	pVoice->enveSusVol = 0;
	pVoice->enveVol = 0;
	pVoice->enveShortAttackCount = 0;
}

INLINE UINT16 enveEmuStartAttack(sidOperator*);
INLINE UINT16 enveEmuStartDecay(sidOperator*);
INLINE UINT16 enveEmuStartRelease(sidOperator*);
INLINE UINT16 enveEmuAlterAttack(sidOperator*);
INLINE UINT16 enveEmuAlterDecay(sidOperator*);
INLINE UINT16 enveEmuAlterSustain(sidOperator*);
INLINE UINT16 enveEmuAlterSustainDecay(sidOperator*);
INLINE UINT16 enveEmuAlterRelease(sidOperator*);
INLINE UINT16 enveEmuAttack(sidOperator*);
INLINE UINT16 enveEmuDecay(sidOperator*);
INLINE UINT16 enveEmuSustain(sidOperator*);
INLINE UINT16 enveEmuSustainDecay(sidOperator*);
INLINE UINT16 enveEmuRelease(sidOperator*);
INLINE UINT16 enveEmuMute(sidOperator*);

INLINE UINT16 enveEmuStartShortAttack(sidOperator*);
INLINE UINT16 enveEmuAlterShortAttack(sidOperator*);
INLINE UINT16 enveEmuShortAttack(sidOperator*);


ptr2sidUwordFunc enveModeTable[] =
{
	/* 0 */
	&enveEmuStartAttack, &enveEmuStartRelease,
	&enveEmuAttack, &enveEmuDecay, &enveEmuSustain, &enveEmuRelease,
	&enveEmuSustainDecay, &enveEmuMute,
	/* 16 */
	&enveEmuStartShortAttack,
	&enveEmuMute, &enveEmuMute, &enveEmuMute,
	&enveEmuMute, &enveEmuMute, &enveEmuMute, &enveEmuMute,
    /* 32        */
	&enveEmuStartAttack, &enveEmuStartRelease,
	&enveEmuAlterAttack, &enveEmuAlterDecay, &enveEmuAlterSustain, &enveEmuAlterRelease,
	&enveEmuAlterSustainDecay, &enveEmuMute,
    /* 48        */
	&enveEmuStartShortAttack,
	&enveEmuMute, &enveEmuMute, &enveEmuMute,
	&enveEmuMute, &enveEmuMute, &enveEmuMute, &enveEmuMute
};

/* Real-time functions. */
/* Order is important because of inline optimizations. */
/* */
/* ADSRctrl is (index*2) to enveModeTable[], because of KEY-bit. */

INLINE void enveEmuEnveAdvance(sidOperator* pVoice)
{
#ifdef SID_FPUENVE
	pVoice->fenveStep += pVoice->fenveStepAdd;
#elif defined(DIRECT_FIXPOINT)
	pVoice->enveStep.l += pVoice->enveStepAdd.l;
#else
	pVoice->enveStepPnt += pVoice->enveStepAddPnt;
	pVoice->enveStep += pVoice->enveStepAdd + ( pVoice->enveStepPnt > 65535 );
	pVoice->enveStepPnt &= 0xFFFF;
#endif
}

/* */
/* Mute/Idle. */
/* */

/* Only used in the beginning. */
INLINE UINT16 enveEmuMute(sidOperator* pVoice)
{
	return 0;
}

/* */
/* Release */
/* */

INLINE UINT16 enveEmuRelease(sidOperator* pVoice)
{
#ifdef SID_FPUENVE
	pVoice->enveStep = (UINT16)pVoice->fenveStep;
#endif
#if defined(DIRECT_FIXPOINT) && !defined(SID_FPUENVE)
	if ( pVoice->enveStep.w[HI] >= releaseTabLen )
#else
	if ( pVoice->enveStep >= releaseTabLen )
#endif
	{
		pVoice->enveVol = releaseTab[releaseTabLen -1];
		return masterAmplModTable[ pVoice->sid->masterVolumeAmplIndex + pVoice->enveVol ];
	}
	else
	{
#if defined(DIRECT_FIXPOINT) && !defined(SID_FPUENVE)
		pVoice->enveVol = releaseTab[pVoice->enveStep.w[HI]];
#else
		pVoice->enveVol = releaseTab[pVoice->enveStep];
#endif
		enveEmuEnveAdvance(pVoice);
		return masterAmplModTable[ pVoice->sid->masterVolumeAmplIndex + pVoice->enveVol ];
	}
}

INLINE UINT16 enveEmuAlterRelease(sidOperator* pVoice)
{
	UINT8 release = pVoice->SIDSR & 0x0F;
#ifdef SID_FPUENVE
	pVoice->fenveStepAdd = decayReleaseRates[release];
#elif defined(DIRECT_FIXPOINT)
	pVoice->enveStepAdd.l = decayReleaseRates[release];
#else
	pVoice->enveStepAdd = decayReleaseRates[release];
	pVoice->enveStepAddPnt = decayReleaseRatesP[release];
#endif
	pVoice->ADSRproc = &enveEmuRelease;
	return enveEmuRelease(pVoice);
}

INLINE UINT16 enveEmuStartRelease(sidOperator* pVoice)
{
	pVoice->ADSRctrl = ENVE_RELEASE;
#ifdef SID_FPUENVE
	pVoice->fenveStep = releasePos[pVoice->enveVol];
#elif defined(DIRECT_FIXPOINT)
	pVoice->enveStep.w[HI] = releasePos[pVoice->enveVol];
	pVoice->enveStep.w[LO] = 0;
#else
	pVoice->enveStep = releasePos[pVoice->enveVol];
	pVoice->enveStepPnt = 0;
#endif
	return enveEmuAlterRelease(pVoice);
}

/* */
/* Sustain */
/* */

INLINE UINT16 enveEmuSustain(sidOperator* pVoice)
{
	return masterAmplModTable[pVoice->sid->masterVolumeAmplIndex+pVoice->enveVol];
}

INLINE UINT16 enveEmuSustainDecay(sidOperator* pVoice)
{
#ifdef SID_FPUENVE
	pVoice->enveStep = (UINT16)pVoice->fenveStep;
#endif
#if defined(DIRECT_FIXPOINT) && !defined(SID_FPUENVE)
	if ( pVoice->enveStep.w[HI] >= releaseTabLen )
#else
	if ( pVoice->enveStep >= releaseTabLen )
#endif
	{
		pVoice->enveVol = releaseTab[releaseTabLen-1];
		return enveEmuAlterSustain(pVoice);
	}
	else
	{
#if defined(DIRECT_FIXPOINT) && !defined(SID_FPUENVE)
		pVoice->enveVol = releaseTab[pVoice->enveStep.w[HI]];
#else
		pVoice->enveVol = releaseTab[pVoice->enveStep];
#endif
		/* Will be controlled from sidEmuSet2(). */
		if ( pVoice->enveVol <= pVoice->enveSusVol )
		{
			pVoice->enveVol = pVoice->enveSusVol;
			return enveEmuAlterSustain(pVoice);
		}
		else
		{
			enveEmuEnveAdvance(pVoice);
			return masterAmplModTable[ pVoice->sid->masterVolumeAmplIndex + pVoice->enveVol ];
		}
	}
}

/* This is the same as enveEmuStartSustainDecay(). */
INLINE UINT16 enveEmuAlterSustainDecay(sidOperator* pVoice)
{
	UINT8 decay = pVoice->SIDAD & 0x0F ;
#ifdef SID_FPUENVE
	pVoice->fenveStepAdd = decayReleaseRates[decay];
#elif defined(DIRECT_FIXPOINT)
	pVoice->enveStepAdd.l = decayReleaseRates[decay];
#else
	pVoice->enveStepAdd = decayReleaseRates[decay];
	pVoice->enveStepAddPnt = decayReleaseRatesP[decay];
#endif
	pVoice->ADSRproc = &enveEmuSustainDecay;
	return enveEmuSustainDecay(pVoice);
}

/* This is the same as enveEmuStartSustain(). */
INLINE UINT16 enveEmuAlterSustain(sidOperator* pVoice)
{
	if ( pVoice->enveVol > pVoice->enveSusVol )
	{
		pVoice->ADSRctrl = ENVE_SUSTAINDECAY;
		pVoice->ADSRproc = &enveEmuSustainDecay;
		return enveEmuAlterSustainDecay(pVoice);
	}
	else
	{
		pVoice->ADSRctrl = ENVE_SUSTAIN;
		pVoice->ADSRproc = &enveEmuSustain;
		return enveEmuSustain(pVoice);
	}
}

/* */
/* Decay */
/* */

INLINE UINT16 enveEmuDecay(sidOperator* pVoice)
{
#ifdef SID_FPUENVE
	pVoice->enveStep = (UINT16)pVoice->fenveStep;
#endif
#if defined(DIRECT_FIXPOINT) && !defined(SID_FPUENVE)
	if ( pVoice->enveStep.w[HI] >= releaseTabLen )
#else
	if ( pVoice->enveStep >= releaseTabLen )
#endif
	{
		pVoice->enveVol = pVoice->enveSusVol;
		return enveEmuAlterSustain(pVoice);  /* start sustain */
	}
	else
	{
#if defined(DIRECT_FIXPOINT) && !defined(SID_FPUENVE)
		pVoice->enveVol = releaseTab[pVoice->enveStep.w[HI]];
#else
		pVoice->enveVol = releaseTab[pVoice->enveStep];
#endif
		/* Will be controlled from sidEmuSet2(). */
		if ( pVoice->enveVol <= pVoice->enveSusVol )
		{
			pVoice->enveVol = pVoice->enveSusVol;
			return enveEmuAlterSustain(pVoice);  /* start sustain */
		}
		else
		{
			enveEmuEnveAdvance(pVoice);
			return masterAmplModTable[ pVoice->sid->masterVolumeAmplIndex + pVoice->enveVol ];
		}
	}
}

INLINE UINT16 enveEmuAlterDecay(sidOperator* pVoice)
{
	UINT8 decay = pVoice->SIDAD & 0x0F ;
#ifdef SID_FPUENVE
	pVoice->fenveStepAdd = decayReleaseRates[decay];
#elif defined(DIRECT_FIXPOINT)
	pVoice->enveStepAdd.l = decayReleaseRates[decay];
#else
	pVoice->enveStepAdd = decayReleaseRates[decay];
	pVoice->enveStepAddPnt = decayReleaseRatesP[decay];
#endif
	pVoice->ADSRproc = &enveEmuDecay;
	return enveEmuDecay(pVoice);
}

INLINE UINT16 enveEmuStartDecay(sidOperator* pVoice)
{
	pVoice->ADSRctrl = ENVE_DECAY;
#ifdef SID_FPUENVE
	pVoice->fenveStep = 0;
#elif defined(DIRECT_FIXPOINT)
	pVoice->enveStep.l = 0;
#else
	pVoice->enveStep = (pVoice->enveStepPnt = 0);
#endif
	return enveEmuAlterDecay(pVoice);
}

/* */
/* Attack */
/* */

INLINE UINT16 enveEmuAttack(sidOperator* pVoice)
{
#ifdef SID_FPUENVE
	pVoice->enveStep = (UINT16)pVoice->fenveStep;
#endif
#if defined(DIRECT_FIXPOINT) && !defined(SID_FPUENVE)
	if ( pVoice->enveStep.w[HI] > attackTabLen )
#else
	if ( pVoice->enveStep >= attackTabLen )
#endif
		return enveEmuStartDecay(pVoice);
	else
	{
#if defined(DIRECT_FIXPOINT) && !defined(SID_FPUENVE)
		pVoice->enveVol = pVoice->enveStep.w[HI];
#else
		pVoice->enveVol = pVoice->enveStep;
#endif
		enveEmuEnveAdvance(pVoice);
		return masterAmplModTable[ pVoice->sid->masterVolumeAmplIndex + pVoice->enveVol ];
	}
}

INLINE UINT16 enveEmuAlterAttack(sidOperator* pVoice)
{
	UINT8 attack = pVoice->SIDAD >> 4;
#ifdef SID_FPUENVE
	pVoice->fenveStepAdd = attackRates[attack];
#elif defined(DIRECT_FIXPOINT)
	pVoice->enveStepAdd.l = attackRates[attack];
#else
	pVoice->enveStepAdd = attackRates[attack];
	pVoice->enveStepAddPnt = attackRatesP[attack];
#endif
	pVoice->ADSRproc = &enveEmuAttack;
	return enveEmuAttack(pVoice);
}

INLINE UINT16 enveEmuStartAttack(sidOperator* pVoice)
{
	pVoice->ADSRctrl = ENVE_ATTACK;
#ifdef SID_FPUENVE
	pVoice->fenveStep = (float)pVoice->enveVol;
#elif defined(DIRECT_FIXPOINT)
	pVoice->enveStep.w[HI] = pVoice->enveVol;
	pVoice->enveStep.w[LO] = 0;
#else
	pVoice->enveStep = pVoice->enveVol;
	pVoice->enveStepPnt = 0;
#endif
	return enveEmuAlterAttack(pVoice);
}

/* */
/* Experimental. */
/* */

/*#include <iostream.h> */
/*#include <iomanip.h> */

INLINE UINT16 enveEmuShortAttack(sidOperator* pVoice)
{
#ifdef SID_FPUENVE
	pVoice->enveStep = (UINT16)pVoice->fenveStep;
#endif
#if defined(DIRECT_FIXPOINT) && !defined(SID_FPUENVE)
	if ((pVoice->enveStep.w[HI] > attackTabLen) ||
		(pVoice->enveShortAttackCount == 0))
#else
	if ((pVoice->enveStep >= attackTabLen) ||
		(pVoice->enveShortAttackCount == 0))
#endif
/*      return enveEmuStartRelease(pVoice); */
		return enveEmuStartDecay(pVoice);
	else
	{
#if defined(DIRECT_FIXPOINT) && !defined(SID_FPUENVE)
		pVoice->enveVol = pVoice->enveStep.w[HI];
#else
		pVoice->enveVol = pVoice->enveStep;
#endif
	    pVoice->enveShortAttackCount--;
/*      cout << hex << pVoice->enveShortAttackCount << " / " << pVoice->enveVol << endl; */
		enveEmuEnveAdvance(pVoice);
		return masterAmplModTable[ pVoice->sid->masterVolumeAmplIndex + pVoice->enveVol ];
	}
}

INLINE UINT16 enveEmuAlterShortAttack(sidOperator* pVoice)
{
	UINT8 attack = pVoice->SIDAD >> 4;
#ifdef SID_FPUENVE
	pVoice->fenveStepAdd = attackRates[attack];
#elif defined(DIRECT_FIXPOINT)
	pVoice->enveStepAdd.l = attackRates[attack];
#else
	pVoice->enveStepAdd = attackRates[attack];
	pVoice->enveStepAddPnt = attackRatesP[attack];
#endif
	pVoice->ADSRproc = &enveEmuShortAttack;
	return enveEmuShortAttack(pVoice);
}

INLINE UINT16 enveEmuStartShortAttack(sidOperator* pVoice)
{
	pVoice->ADSRctrl = ENVE_SHORTATTACK;
#ifdef SID_FPUENVE
	pVoice->fenveStep = (float)pVoice->enveVol;
#elif defined(DIRECT_FIXPOINT)
	pVoice->enveStep.w[HI] = pVoice->enveVol;
	pVoice->enveStep.w[LO] = 0;
#else
	pVoice->enveStep = pVoice->enveVol;
	pVoice->enveStepPnt = 0;
#endif
	pVoice->enveShortAttackCount = 65535;  /* unused */
	return enveEmuAlterShortAttack(pVoice);
}
