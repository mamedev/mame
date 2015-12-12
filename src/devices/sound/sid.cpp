// license:BSD-3-Clause
// copyright-holders:Peter Trauner
/*
  copyright peter trauner

  based on michael schwend's sid play

  Noise generation algorithm is used courtesy of Asger Alstrup Nielsen.
  His original publication can be found on the SID home page.

  Noise table optimization proposed by Phillip Wooller. The output of
  each table does not differ.

  MOS-8580 R5 combined waveforms recorded by Dennis "Deadman" Lindroos.
*/

#include "emu.h"
#include "sidvoice.h"
#include "sidenvel.h"
#include "sid.h"

static float *filterTable;
static float *bandPassParam;
#define lowPassParam filterTable
static float filterResTable[16];

#define maxLogicalVoices 4

static const int mix16monoMiddleIndex = 256*maxLogicalVoices/2;
static UINT16 mix16mono[256*maxLogicalVoices];

static UINT16 zero16bit=0;  /* either signed or unsigned */
//UINT32 splitBufferLen;

static void MixerInit(int threeVoiceAmplify)
{
	long si;
	UINT16 ui;
	long ampDiv = maxLogicalVoices;

	if (threeVoiceAmplify)
	{
		ampDiv = (maxLogicalVoices-1);
	}

	/* Mixing formulas are optimized by sample input value. */

	si = (-128*maxLogicalVoices) * 256;
	for (ui = 0; ui < sizeof(mix16mono)/sizeof(UINT16); ui++ )
	{
		mix16mono[ui] = (UINT16)(si/ampDiv) + zero16bit;
		si+=256;
	}

}


static inline void syncEm(SID6581_t *This)
{
	int sync1 = (This->optr1.modulator->cycleLenCount <= 0);
	int sync2 = (This->optr2.modulator->cycleLenCount <= 0);
	int sync3 = (This->optr3.modulator->cycleLenCount <= 0);

	This->optr1.cycleLenCount--;
	This->optr2.cycleLenCount--;
	This->optr3.cycleLenCount--;

	if (This->optr1.sync && sync1)
	{
		This->optr1.cycleLenCount = 0;
		This->optr1.outProc = &sidWaveCalcNormal;
#if defined(DIRECT_FIXPOINT)
		optr1.waveStep.l = 0;
#else
		This->optr1.waveStep = (This->optr1.waveStepPnt = 0);
#endif
	}
	if (This->optr2.sync && sync2)
	{
		This->optr2.cycleLenCount = 0;
		This->optr2.outProc = &sidWaveCalcNormal;
#if defined(DIRECT_FIXPOINT)
		This->optr2.waveStep.l = 0;
#else
		This->optr2.waveStep = (This->optr2.waveStepPnt = 0);
#endif
	}
	if (This->optr3.sync && sync3)
	{
		This->optr3.cycleLenCount = 0;
		This->optr3.outProc = &sidWaveCalcNormal;
#if defined(DIRECT_FIXPOINT)
		optr3.waveStep.l = 0;
#else
		This->optr3.waveStep = (This->optr3.waveStepPnt = 0);
#endif
	}
}


void sidEmuFillBuffer(SID6581_t *This, stream_sample_t *buffer, UINT32 bufferLen )
{
//void* fill16bitMono( SID6581_t *This, void* buffer, UINT32 numberOfSamples )

	for ( ; bufferLen > 0; bufferLen-- )
	{
		*buffer++ = (INT16) mix16mono[(unsigned)(mix16monoMiddleIndex
								+(*This->optr1.outProc)(&This->optr1)
								+(*This->optr2.outProc)(&This->optr2)
								+(This->optr3.outProc(&This->optr3)&This->optr3_outputmask)
/* hack for digi sounds
   does n't seam to come from a tone operator
   ghostbusters and goldrunner everything except volume zeroed */
							+(This->masterVolume<<2)
//                        +(*sampleEmuRout)()
		)];
		syncEm(This);
	}
}

/* --------------------------------------------------------------------- Init */


/* Reset. */

int sidEmuReset(SID6581_t *This)
{
	sidClearOperator( &This->optr1 );
	enveEmuResetOperator( &This->optr1 );
	sidClearOperator( &This->optr2 );
	enveEmuResetOperator( &This->optr2 );
	sidClearOperator( &This->optr3 );
	enveEmuResetOperator( &This->optr3 );
	This->optr3_outputmask = ~0;  /* on */

//  sampleEmuReset();

	This->filter.Type = (This->filter.CurType = 0);
	This->filter.Value = 0;
	This->filter.Dy = (This->filter.ResDy = 0);

	sidEmuSet( &This->optr1 );
	sidEmuSet( &This->optr2 );
	sidEmuSet( &This->optr3 );

	sidEmuSet2( &This->optr1 );
	sidEmuSet2( &This->optr2 );
	sidEmuSet2( &This->optr3 );

	return TRUE;
}


static void filterTableInit(running_machine &machine)
{
	int sample_rate = machine.sample_rate();
	UINT16 uk;
	/* Parameter calculation has not been moved to a separate function */
	/* by purpose. */
	const float filterRefFreq = 44100.0f;

	float yMax = 1.0f;
	float yMin = 0.01f;
	float yAdd;
	float yTmp, rk, rk2;

	float resDyMax;
	float resDyMin;
	float resDy;

	filterTable = auto_alloc_array(machine, float, 0x800);
	bandPassParam = auto_alloc_array(machine, float, 0x800);

	uk = 0;
	for ( rk = 0; rk < 0x800; rk++ )
	{
		filterTable[uk] = (((expf(rk/0x800*logf(400.0f))/60.0f)+0.05f)
			*filterRefFreq) / sample_rate;
		if ( filterTable[uk] < yMin )
			filterTable[uk] = yMin;
		if ( filterTable[uk] > yMax )
			filterTable[uk] = yMax;
		uk++;
	}

	/*extern float bandPassParam[0x800]; */
	yMax = 0.22f;
	yMin = 0.05f;  /* less for some R1/R4 chips */
	yAdd = (yMax-yMin)/2048.0f;
	yTmp = yMin;
	uk = 0;
	/* Some C++ compilers still have non-local scope! */
	for ( rk2 = 0; rk2 < 0x800; rk2++ )
	{
		bandPassParam[uk] = (yTmp*filterRefFreq) / sample_rate;
		yTmp += yAdd;
		uk++;
	}

	/*extern float filterResTable[16]; */
	resDyMax = 1.0f;
	resDyMin = 2.0f;
	resDy = resDyMin;
	for ( uk = 0; uk < 16; uk++ )
	{
		filterResTable[uk] = resDy;
		resDy -= (( resDyMin - resDyMax ) / 15 );
	}
	filterResTable[0] = resDyMin;
	filterResTable[15] = resDyMax;
}

void sid6581_init (SID6581_t *This)
{
	This->optr1.sid=This;
	This->optr2.sid=This;
	This->optr3.sid=This;

	This->optr1.modulator = &This->optr3;
	This->optr3.carrier = &This->optr1;
	This->optr1.filtVoiceMask = 1;

	This->optr2.modulator = &This->optr1;
	This->optr1.carrier = &This->optr2;
	This->optr2.filtVoiceMask = 2;

	This->optr3.modulator = &This->optr2;
	This->optr2.carrier = &This->optr3;
	This->optr3.filtVoiceMask = 4;



	This->PCMsid = (UINT32)(This->PCMfreq * (16777216.0 / This->clock));
	This->PCMsidNoise = (UINT32)((This->clock*256.0)/This->PCMfreq);

	This->filter.Enabled = TRUE;

	sidInitMixerEngine(This->device->machine());
	filterTableInit(This->device->machine());

	sidInitWaveformTables(This->type);

	enveEmuInit(This->PCMfreq, TRUE);

	MixerInit(0);

	sidEmuReset(This);
}

void sid6581_port_w (SID6581_t *This, int offset, int data)
{
	offset &= 0x1f;

	switch (offset)
	{
		case 0x19: case 0x1a: case 0x1b: case 0x1c:
		case 0x1d:
		case 0x1e:
		case 0x1f:
			break;
		case 0x15: case 0x16: case 0x17:
		case 0x18:
			This->mixer_channel->update();
			This->reg[offset] = data;
			This->masterVolume = ( This->reg[0x18] & 15 );
			This->masterVolumeAmplIndex = This->masterVolume << 8;

			if ((This->reg[0x18]&0x80) &&
				((This->reg[0x17]&This->optr3.filtVoiceMask)==0))
				This->optr3_outputmask = 0;     /* off */
			else
				This->optr3_outputmask = ~0;  /* on */

			This->filter.Type = This->reg[0x18] & 0x70;
			if (This->filter.Type != This->filter.CurType)
			{
				This->filter.CurType = This->filter.Type;
				This->optr1.filtLow = (This->optr1.filtRef = 0);
				This->optr2.filtLow = (This->optr2.filtRef = 0);
				This->optr3.filtLow = (This->optr3.filtRef = 0);
			}
			if ( This->filter.Enabled )
			{
				This->filter.Value = 0x7ff & ( (This->reg[0x15]&7) | ( (UINT16)This->reg[0x16] << 3 ));
				if (This->filter.Type == 0x20)
					This->filter.Dy = bandPassParam ? bandPassParam[This->filter.Value] : 0.0f;
				else
					This->filter.Dy = lowPassParam ? lowPassParam[This->filter.Value] : 0.0f;
				This->filter.ResDy = filterResTable[This->reg[0x17] >> 4] - This->filter.Dy;
				if ( This->filter.ResDy < 1.0f )
					This->filter.ResDy = 1.0f;
			}

			sidEmuSet( &This->optr1 );
			sidEmuSet( &This->optr3 );
			sidEmuSet( &This->optr2 );

			// relies on sidEmuSet also for other channels!
			sidEmuSet2( &This->optr1 );
			sidEmuSet2( &This->optr2 );
			sidEmuSet2( &This->optr3 );
			break;

		default:
			This->mixer_channel->update();
			This->reg[offset] = data;

			if (offset<7) {
				This->optr1.reg[offset] = data;
			} else if (offset<14) {
				This->optr2.reg[offset-7] = data;
			} else if (offset<21) {
				This->optr3.reg[offset-14] = data;
			}

			sidEmuSet( &This->optr1 );
			sidEmuSet( &This->optr3 );
			sidEmuSet( &This->optr2 );

			// relies on sidEmuSet also for other channels!
			sidEmuSet2( &This->optr1 );
			sidEmuSet2( &This->optr2 );
			sidEmuSet2( &This->optr3 );
			break;
	}
}

int sid6581_port_r (running_machine &machine, SID6581_t *This, int offset)
{
	int data;
/* SIDPLAY reads last written at a sid address value */
	offset &= 0x1f;
	switch (offset)
	{
	case 0x1d:
	case 0x1e:
	case 0x1f:
	data=0xff;
	break;
	case 0x1b:
	This->mixer_channel->update();
	data = This->optr3.output;
	break;
	case 0x1c:
	This->mixer_channel->update();
	data = This->optr3.enveVol;
	break;
	default:
	data=This->reg[offset];
	}
	return data;
}
