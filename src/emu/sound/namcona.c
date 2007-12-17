/*
    Namco NA1/2 Sound Hardware

    PCM samples and sound sequencing metadata are written by the main CPU to
    shared RAM.

    The sound CPU's type is unknown, though it appears to be little endian.
    It has an internal BIOS.

    RAM[0x820]:         song select
    RAM[0x822]:         song control (fade?)
    RAM[0x824..0x89e]:
        Even addresses are used to select the sound effect.
        0x40 is written to odd addresses as a signal to play the requested sound.

    Metadata vectors:
    addr  sample
    0000: 0012   // table of addresses for each VOX sequence
    0002: 07e4   // table of wave records (5 words each record)
    0004: 0794   // unknown table (5 words each record)
    0006: 0000   // unknown (always zero?)
    0008: 0102   // unknown address table
    000a: 0384   // unknown address table
    000c: 0690   // unknown chunk
    000e: 07bc   // unknown table (5 words each record)
    0010: 7700   // unknown
    0012: addresses for each song sequence start here

    Known issues:
    - many opcodes are ignored or implemented imperfectly
    - the metadata contains several mystery tables
*/

#include <math.h>
#include "sndintrf.h"
#include "streams.h"
#include "namcona.h"

#define kTwelfthRootTwo 1.059463094
#define FIXED_POINT_SHIFT (10) /* for mixing */
#define MAX_VOICE 16
#define MAX_SEQUENCE_VOICE 32
#define MAX_SEQUENCE  0x40
#define MAX_SEQUENCE_RECURSION 4 /* ? */
#define SAMPLE_RATE_BASE (42667*2)

struct voice
{
	INT32 bActive;

	INT32 flags;
	INT32 start; /* fixed point */
	INT32 end;   /* fixed point */
	INT32 loop;  /* fixed point */
	INT32 baseFreq;
	INT32 bank;

	INT32 delta; /* fixed point */
	INT32 pos;
	INT32 note;
	INT32 preNote;
	INT32 volume;
	INT32 pan;
	INT32 leftVolume;
	INT32 rightVolume;
	INT32 dnote;
	INT32 detune;
	INT32 ignoreCount;
	INT32 delay;
	INT32 delayCount;
	INT32 freqCtrlTableNo;
	INT32 freqCtrlTable;
	INT32 freqCtrlSpeed;
	INT32 freqCtrlAmp;
	INT32 freqCtrlPointer;
	INT32 freqCtrlCounter;
	INT32 freqCtrlFreq;
	INT32 volCtrlTable;
	INT32 volCtrlCount;
	INT32 volCtrlPointer;
	INT32 volCtrlCounter;
	INT32 volCtrlSpeed;
	INT32 volCtrlVol;
	INT32 volCtrlVolEnd;
	INT32 panCtrlTable;
	INT32 panCtrlSpeed;
	INT32 panCtrlCounter;
	INT32 panCtrlEnable;
	INT32 portament;
	INT32 portamentSpeed;
	INT32 portamentNote;
};

struct sequence
{
	UINT8 volume;
	UINT8 reg2; /* master freq? */
	UINT8 tempo;
	int addr;
	int pause;
	int channel[8];
	int stackData[MAX_SEQUENCE_RECURSION]; /* used for sub-sequences */
	int stackSize;
	int count; /* used for "repeat" opcode */
	int count2;
	int count3;
};

struct namcona
{
	int mSampleRate;
	sound_stream * mStream;
	INT16 *mpMixerBuffer;
	INT32 *mpPitchTable;
	UINT16 *mpROM;
	UINT16 *mpMetaData;
	struct sequence mSequence[MAX_SEQUENCE];
	struct voice mVoice[MAX_SEQUENCE_VOICE];
};

static const UINT16 VolCtrlSpeedTable[0x80] =
{
	0x0000, 0x0001, 0x0002, 0x0003, 0x0004, 0x0005, 0x0007, 0x0009,
	0x000b, 0x000d, 0x000f, 0x0011, 0x0013, 0x0015, 0x0017, 0x0019,
	0x001b, 0x001d, 0x001f, 0x0022, 0x0024, 0x0027, 0x002a, 0x002d,
	0x0030, 0x0033, 0x0037, 0x003b, 0x0040, 0x0044, 0x0049, 0x004f,
	0x0054, 0x005a, 0x0061, 0x0068, 0x0070, 0x0078, 0x0081, 0x008a,
	0x0094, 0x009f, 0x00aa, 0x00b7, 0x00c4, 0x00d2, 0x00e1, 0x00f2,
	0x0103, 0x0116, 0x012a, 0x0140, 0x0157, 0x0170, 0x018b, 0x01ab,
	0x01c7, 0x01e8, 0x020b, 0x0231, 0x025a, 0x0285, 0x02b4, 0x02e6,
	0x031c, 0x0356, 0x0394, 0x03d6, 0x041d, 0x046a, 0x04bc, 0x0514,
	0x0572, 0x05d7, 0x0644, 0x06b8, 0x0735, 0x07ba, 0x084a, 0x08e4,
	0x0989, 0x0a3a, 0x0af8, 0x0bc3, 0x0c9e, 0x0d88, 0x0e83, 0x0f91,
	0x10b2, 0x11e8, 0x1334, 0x1498, 0x1617, 0x17b1, 0x1969, 0x1b40,
	0x1d3a, 0x1f59, 0x219f, 0x240f, 0x26ac, 0x297a, 0x2c7c, 0x2fb6,
	0x332b, 0x36e1, 0x3adc, 0x3f20, 0x43b4, 0x489d, 0x4de1, 0x5386,
	0x5995, 0x6014, 0x670b, 0x6e84, 0x7687, 0x7f1f, 0x8857, 0x923a,
	0x9cd4, 0xa833, 0xb465, 0xc17a, 0xcf81, 0xde8d, 0xeeb0, 0xffff
};


/**
 * Given a sequence, return a pointer to the associated 16 bit word in shared RAM.
 * The 16 bits are interpretted as follows:
 * xxxxxxxx-------- index into sequence table (selects song/sample)
 * --------x------- 1 if sequence is active/playing
 * ---------x------ 1 if the main CPU has written a new sound command
 * ----------xxxxxx unknown/unused
 */
static UINT16 *
GetSequenceStatusAddr( struct namcona *chip, struct sequence *pSeq )
{
	int offs = pSeq - chip->mSequence;
	return &chip->mpROM[0x820/2+offs];
}

static void
Silence( struct namcona *chip )
{
	int i;
	for( i=0; i<MAX_SEQUENCE_VOICE; i++ )
	{
		chip->mVoice[i].bActive = 0;
	}
	for( i=0; i<MAX_SEQUENCE; i++ )
	{
		UINT16 *pStatus = GetSequenceStatusAddr(chip, &chip->mSequence[i]);
		*pStatus &= 0xff7f; /* wipe "sequence-is-playing" flag */
	}
}

static UINT8
ReadMetaDataByte( struct namcona *chip, int addr )
{
	UINT16 data = chip->mpMetaData[addr/2];
	return (addr&1)?(data&0xff):(data>>8);
} /* ReadMetaDataByte */

static UINT16
ReadMetaDataWord( struct namcona *chip, int addr )
{
	return ReadMetaDataByte(chip, addr)+ReadMetaDataByte(chip, addr+1)*256;
} /* ReadMetaDataWord */

static signed char
ReadPCMSample(struct namcona *chip, int addr, int flag )
{
	UINT16 data16 = chip->mpROM[addr/2];
	int dat = (addr&1)?(data16&0xff):(data16>>8);

	if( flag&0x100 )
	{
		if( dat&0x80 )
		{
			dat = -(dat&0x7f) - 1;
		}
	}

	return dat;
} /* ReadPCMSample */

static void
RenderSamples(struct namcona *chip, stream_sample_t **buffer, INT16 *pSource, int length )
{
	int i;
	stream_sample_t * pDest1 = buffer[0];
	stream_sample_t * pDest2 = buffer[1];
	for (i = 0; i < length; i++)
	{
		INT32 dataL = /* 100 * */ (*pSource++);
		INT32 dataR = /* 100 * */ (*pSource++);
		if( dataL > 0x7fff )
		{
			dataL =  0x7fff; /* clip */
		}
		else if( dataL < -0x8000 )
		{
			dataL = -0x8000; /* clip */
		}
		if( dataR > 0x7fff )
		{
			dataR =  0x7fff; /* clip */
		}
		else if( dataR < -0x8000 )
		{
			dataR = -0x8000; /* clip */
		}
		*pDest1++ = (INT16)dataL; /* stereo left */
		*pDest2++ = (INT16)dataR; /* stereo right */
	}
} /* RenderSamples */

static void
PushSequenceAddr(struct namcona *chip, struct sequence *pSequence, int addr )
{
	if( pSequence->stackSize<MAX_SEQUENCE_RECURSION )
	{
		pSequence->stackData[pSequence->stackSize++] = addr;
	}
	else
	{
		logerror( "sound/namcona.c stack overflow!\n" );
	}
} /* PushSequenceAddr */

static void
PopSequenceAddr(struct namcona *chip, struct sequence *pSequence )
{
	if( pSequence->stackSize )
	{
		pSequence->addr = pSequence->stackData[--pSequence->stackSize];
	}
	else
	{
		UINT16 *pStatus = GetSequenceStatusAddr(chip,pSequence);
		*pStatus &= 0xff7f; /* wipe "sequence-is-playing" flag */
	}
} /* PopSequenceAddr */

static void
HandleSubroutine(struct namcona *chip, struct sequence *pSequence )
{
	int addr = ReadMetaDataWord(chip, pSequence->addr );
	PushSequenceAddr(chip, pSequence, pSequence->addr+2 );
	pSequence->addr = addr;
} /* HandleSubroutine */

static void
HandleRepeat(struct namcona *chip, struct sequence *pSequence )
{
	int count = ReadMetaDataByte( chip, pSequence->addr++ );
	int addr = ReadMetaDataWord(chip, pSequence->addr );
	if( pSequence->count < count )
	{
		pSequence->count++;
		pSequence->addr = addr;
	}
	else
	{
		pSequence->count = 0;
		pSequence->addr += 2;
	}
} /* HandleRepeat */

static void
HandleRepeatOut(struct namcona *chip, struct sequence *pSequence )
{
	int count = ReadMetaDataByte( chip, pSequence->addr++ );
	int addr = ReadMetaDataWord(chip, pSequence->addr );
	if( pSequence->count2 < count )
	{
		pSequence->count2++;
		pSequence->addr += 2;
	}
	else
	{
		pSequence->count2 = 0;
		pSequence->addr = addr;
	}
} /* HandleRepeatOut */

static void
MapArgs(struct namcona *chip, struct sequence *pSequence,
		 int bCommon,
		 void (*callback)( struct namcona *chip, struct sequence *, int chan, UINT8 data ) )
{
	UINT8 set = ReadMetaDataByte(chip,pSequence->addr++);
	UINT8 data = 0;
	int i;
	if( bCommon )
	{
		data = ReadMetaDataByte( chip, pSequence->addr++ );
	}
	for( i=0; i<8; i++ )
	{
		if( set&(1<<(7-i)) )
		{
			if( !bCommon )
			{
				data = ReadMetaDataByte( chip,pSequence->addr++ );
			}
			callback( chip, pSequence, i, data );
		}
	}
} /* MapArgs */

static void
AssignChannel(struct namcona *chip, struct sequence *pSequence, int chan, UINT8 data )
{
	if( (pSequence - chip->mSequence) > 1 )
		data += 16;		/* sound effect */

	if( data<MAX_SEQUENCE_VOICE )
	{
		struct voice *pVoice = &chip->mVoice[data];
		pSequence->channel[chan] = data;
		pVoice->bActive = 0;
		pVoice->bank = 0;
		pVoice->volume = 0x80;
		pVoice->pan = 0x80;
		pVoice->dnote = 0;
		pVoice->detune = 0;
		pVoice->freqCtrlTableNo = 0;
		pVoice->freqCtrlSpeed = 0;
		pVoice->freqCtrlAmp = 0;
		pVoice->volCtrlTable = 0;
		pVoice->volCtrlCount = 0;
		pVoice->panCtrlTable = 0;
		pVoice->ignoreCount = 0;
		pVoice->delay = 0;
		pVoice->portament = 0;
	}
} /* AssignChannel */

static void
IgnoreUnknownOp(struct namcona *chip, struct sequence *pSequence, int chan, UINT8 data )
{
} /* IgnoreUnknownOp */

static void
SelectWave(struct namcona *chip, struct sequence *pSequence, int chan, UINT8 data )
{
	struct voice *pVoice = &chip->mVoice[pSequence->channel[chan]];
	int bank = 0x20000 + pVoice->bank*0x20000;
	int addr  = ReadMetaDataWord(chip,2)+10*data;

	pVoice->flags    = ReadMetaDataWord(chip,addr+0*2);
	pVoice->start    = ReadMetaDataWord(chip,addr+1*2)*2+bank;
	pVoice->end      = ReadMetaDataWord(chip,addr+2*2)*2+bank;
	pVoice->loop     = ReadMetaDataWord(chip,addr+3*2)*2+bank;
	pVoice->baseFreq = ReadMetaDataWord(chip,addr+4*2); /* unsure what this is; not currently used */

	pVoice->start <<= FIXED_POINT_SHIFT;
	pVoice->end   <<= FIXED_POINT_SHIFT;
	pVoice->loop  <<= FIXED_POINT_SHIFT;

	pVoice->bActive = 0;
	pVoice->dnote = 0;
	pVoice->detune = 0;
} /* SelectWave */

static void
PlayNote(struct namcona *chip, struct sequence *pSequence, int chan, UINT8 data )
{
	struct voice *pVoice = &chip->mVoice[pSequence->channel[chan]];
	if( data == 0xff )
	{
		pVoice->volCtrlCounter = 1;
	}
	else
	{
		if (pVoice->ignoreCount > 0)
			pVoice->ignoreCount--;
		else
		{
			pVoice->delayCount = pVoice->delay;
			pVoice->preNote = pVoice->note;
			pVoice->note = data<<8;
			pVoice->bActive |= 4;
		}
	}
} /* PlayNote */

static void
SelectWaveAndPlayNote(struct namcona *chip, struct sequence *pSequence, int chan, UINT8 data )
{
	struct voice *pVoice = &chip->mVoice[pSequence->channel[chan]];
	int bank = 0x20000 + pVoice->bank*0x20000;
	int addr = ReadMetaDataWord(chip,4)+10*data;
	float pbase = (float)SAMPLE_RATE_BASE / (float)chip->mSampleRate;
	INT32 frequency;

	pVoice->flags    = ReadMetaDataWord(chip,addr+0*2);
	pVoice->start    = ReadMetaDataWord(chip,addr+3*2)*2+bank;
	pVoice->end      = ReadMetaDataWord(chip,addr+4*2)*2+bank;
	pVoice->loop     = pVoice->start;

	pVoice->start <<= FIXED_POINT_SHIFT;
	pVoice->end   <<= FIXED_POINT_SHIFT;
	pVoice->loop  <<= FIXED_POINT_SHIFT;

	pVoice->leftVolume  = ReadMetaDataByte(chip,addr+1*2);
	pVoice->rightVolume = ReadMetaDataByte(chip,addr+1*2+1);

	frequency = ReadMetaDataWord(chip,addr+2*2);
	pVoice->delta = (long)((float)frequency * pbase);
	pVoice->delta >>= 16-FIXED_POINT_SHIFT;

	pVoice->bActive = 2;
	pVoice->pos = pVoice->start;
} /* SelectWaveAndPlayNote */

static void
Detune(struct namcona *chip, struct sequence *pSequence, int chan, UINT8 data )
{
	struct voice *pVoice = &chip->mVoice[pSequence->channel[chan]];
	pVoice->detune = data;
} /* Detune */


static void
DNote(struct namcona *chip, struct sequence *pSequence, int chan, UINT8 data )
{
	struct voice *pVoice = &chip->mVoice[pSequence->channel[chan]];
	pVoice->dnote = data;
} /* DNote */

static void
Pan(struct namcona *chip, struct sequence *pSequence, int chan, UINT8 data )
{
	struct voice *pVoice = &chip->mVoice[pSequence->channel[chan]];
	pVoice->pan = data;
	pVoice->panCtrlSpeed = 0;
} /* Pan */

static void
Volume(struct namcona *chip, struct sequence *pSequence, int chan, UINT8 data )
{
	struct voice *pVoice = &chip->mVoice[pSequence->channel[chan]];
	pVoice->volume = pSequence->volume * data / 256;
} /* Volume */

static void
FreqCtrlTable(struct namcona *chip, struct sequence *pSequence, int chan, UINT8 data )
{
	struct voice *pVoice = &chip->mVoice[pSequence->channel[chan]];
	pVoice->freqCtrlTableNo = data;
} /* FreqCtrlTable */

static void
FreqCtrlSpeed(struct namcona *chip, struct sequence *pSequence, int chan, UINT8 data )
{
	struct voice *pVoice = &chip->mVoice[pSequence->channel[chan]];
	pVoice->freqCtrlSpeed = data;
} /* FreqCtrlSpeed */

static void
FreqCtrlAmp(struct namcona *chip, struct sequence *pSequence, int chan, UINT8 data )
{
	struct voice *pVoice = &chip->mVoice[pSequence->channel[chan]];
	pVoice->freqCtrlAmp = data;
} /* FreqCtrlAmp */

static void
VolCtrlTable(struct namcona *chip, struct sequence *pSequence, int chan, UINT8 data )
{
	struct voice *pVoice = &chip->mVoice[pSequence->channel[chan]];
	int addr = ReadMetaDataWord(chip,10);
	addr = ReadMetaDataWord(chip,addr+data*2);
	pVoice->volCtrlTable = addr;
} /* VolCtrlTable */

static void
VolCtrlCount(struct namcona *chip, struct sequence *pSequence, int chan, UINT8 data )
{
	struct voice *pVoice = &chip->mVoice[pSequence->channel[chan]];
	pVoice->volCtrlCount = data;
} /* VolCtrlCount */

static void
PanCtrlTable(struct namcona *chip, struct sequence *pSequence, int chan, UINT8 data )
{
	struct voice *pVoice = &chip->mVoice[pSequence->channel[chan]];
	int addr = ReadMetaDataWord(chip,12) + data*4;
	pVoice->panCtrlTable = addr;
	pVoice->panCtrlSpeed = ReadMetaDataByte(chip,addr+3);
	if( pVoice->panCtrlSpeed & 0x80)
		pVoice->panCtrlSpeed = -(0x100 - pVoice->panCtrlSpeed);
} /* PanCtrlTable */

static void
IgnoreCount(struct namcona *chip, struct sequence *pSequence, int chan, UINT8 data )
{
	struct voice *pVoice = &chip->mVoice[pSequence->channel[chan]];
	pVoice->ignoreCount = data;
} /* IgnoreCount */

static void
Delay(struct namcona *chip, struct sequence *pSequence, int chan, UINT8 data )
{
	struct voice *pVoice = &chip->mVoice[pSequence->channel[chan]];
	pVoice->delay = data;
} /* Delay */

static void
Portament(struct namcona *chip, struct sequence *pSequence, int chan, UINT8 data )
{
	struct voice *pVoice = &chip->mVoice[pSequence->channel[chan]];
	pVoice->portament = (0x100 - data) & 0xff;
} /* Portament */

static void
UpdateSequence(struct namcona *chip, struct sequence *pSequence )
{
	UINT16 *pStatus = GetSequenceStatusAddr(chip,pSequence);
	UINT16 data = *pStatus;

	if( data&0x0040 )
	{ /* bit 0x0040 indicates that a sound request was written by the main CPU */
		int offs = ReadMetaDataWord(chip,0)+(data>>8)*2;
		memset( pSequence, 0x00, sizeof(struct sequence) );
		if( pSequence == &chip->mSequence[0] ) Silence(chip); /* hack! */
		pSequence->addr = ReadMetaDataWord(chip,offs);
		*pStatus = (data&0xffbf)|0x0080; /* set "sequence-is-playing" flag */
	}
	if( data&0x0080 )
	{
		if( pSequence == &chip->mSequence[1] )
		{
			if( !(*(pStatus-1) & 0x0080) )
				*pStatus = data&0xff7f;
		}
	}

	while( (*pStatus)&0x0080 )
	{
		if( pSequence->pause )
		{
			pSequence->pause--;
			return;
		}
		else
		{
			int code = ReadMetaDataByte(chip,pSequence->addr++);
			if( code&0x80 )
			{
				pSequence->pause = pSequence->tempo*((code&0x7f)+1);
			}
			else
			{
				int bCommon = (code&0x40);
				switch( code&0x3f )
				{
				case 0x01: /* master volume */
					pSequence->volume = ReadMetaDataByte(chip,pSequence->addr++);
					break;

				case 0x02: /* master tempo */
					pSequence->reg2 = ReadMetaDataByte(chip,pSequence->addr++);
					break;

				case 0x03: /* tempo */
					pSequence->tempo = ReadMetaDataByte(chip,pSequence->addr++) * pSequence->reg2;
					if (pSequence->tempo == 0) pSequence->tempo = 1;
					break;

				case 0x04:
					HandleSubroutine(chip, pSequence );
					break;

				case 0x05: /* end-of-sequence */
					PopSequenceAddr(chip, pSequence );
					break;

				case 0x06: /* operand is note index */
					MapArgs(chip, pSequence, bCommon, PlayNote );
					pSequence->pause = pSequence->tempo;
					break;

				case 0x07:
					MapArgs(chip, pSequence, bCommon, SelectWave );
					break;

				case 0x08:
					MapArgs(chip, pSequence, bCommon, Volume );
					break;

				case 0x09:
					pSequence->addr = ReadMetaDataWord(chip,pSequence->addr);
					break;

				case 0x0a:
					HandleRepeat(chip, pSequence );
					break;

				case 0x0b:
					HandleRepeatOut(chip, pSequence );
					break;

				case 0x0c:
					MapArgs(chip, pSequence, bCommon, DNote );
					break;

				case 0x0d:
					MapArgs(chip, pSequence, bCommon, Detune );
					break;

				case 0x0e:
					MapArgs(chip, pSequence, bCommon, FreqCtrlTable );
					break;

				case 0x0f:
					MapArgs(chip, pSequence, bCommon, FreqCtrlSpeed );
					break;

				case 0x12:
					MapArgs(chip, pSequence, bCommon, FreqCtrlAmp );
					break;

				case 0x13:
					MapArgs(chip, pSequence, bCommon, VolCtrlTable );
					break;

				case 0x11:
					MapArgs(chip, pSequence, bCommon, VolCtrlCount );
					break;

				case 0x10:
					MapArgs(chip, pSequence, bCommon, IgnoreCount );
					break;

				case 0x14:
					MapArgs(chip, pSequence, bCommon, Delay );
					break;

				case 0x16:
					MapArgs(chip, pSequence, bCommon, Pan );
					break;

				case 0x17:
					MapArgs(chip, pSequence, bCommon, PanCtrlTable );
					break;

				case 0x19: // one loop?
					if (pSequence->count3 == 0)
					{
						pSequence->addr = ReadMetaDataWord(chip,pSequence->addr);
						pSequence->count3++;
					}
					else
					{
						pSequence->addr += 2;
						pSequence->count3 = 0;
					}
					break;

				case 0x1b:
					MapArgs(chip, pSequence, bCommon, SelectWaveAndPlayNote );
					pSequence->pause = pSequence->tempo;
					break;

				case 0x1c: // request sound effect ?
					pSequence->addr += 2;
					break;

				case 0x1e:
					{
						int no = ReadMetaDataByte(chip,pSequence->addr++);	/* Sequence No */
						int cod = ReadMetaDataByte(chip,pSequence->addr++);
						if (no < MAX_SEQUENCE)
						{
							struct sequence *pSequence2 = &chip->mSequence[no];
							UINT16 *pStatus2 = GetSequenceStatusAddr(chip,pSequence2);
							int offs = 0x12+cod*2;
							*pStatus2 = (cod<<8)|0x0080;
							memset( pSequence2, 0x00, sizeof(struct sequence) );
							pSequence2->addr = ReadMetaDataWord(chip,offs);
						}
					}
					break;

				case 0x20:
					MapArgs(chip, pSequence, bCommon, AssignChannel );
					break;

				case 0x22:
					MapArgs(chip, pSequence, bCommon, Portament );
					break;

				case 0x23:
					{
						UINT8 reg23_0 = ReadMetaDataByte(chip,pSequence->addr++); /* Channel select */
						UINT8 reg23_1 = ReadMetaDataByte(chip,pSequence->addr++); /* PCM bank select */
						/* reg23_0: 0 = Ch. 0- 3 PCM Bank select
                                    1 = Ch. 4- 7 PCM Bank select
                                    2 = Ch. 8-11 PCM Bank select
                                    3 = Ch.12-15 PCM Bank select
                        */
						if( reg23_0 == 5 ) reg23_0 = 3; /* xday2,bkrtmaq */
						if( reg23_0 < 4 )
						{
							if( (pSequence - chip->mSequence) > 1 )
								reg23_0 += 4;		/* sound effect */
							chip->mVoice[reg23_0*4 + 0].bank = reg23_1;
							chip->mVoice[reg23_0*4 + 1].bank = reg23_1;
							chip->mVoice[reg23_0*4 + 2].bank = reg23_1;
							chip->mVoice[reg23_0*4 + 3].bank = reg23_1;
						}
					}
					break;

				case 0x24: // priority ?
					MapArgs(chip, pSequence, bCommon, IgnoreUnknownOp );
					break;

				default:
					//mame_printf_debug( "? 0x%x\n", code&0x3f );
					*pStatus &= 0xff7f; /* clear "sequence-is-playing" flag */
					break;
				}
			}
		}
	}
} /* UpdateSequence */

static void
InitFreqCtrl( struct namcona *chip, struct voice *pVoice )
{
	if( pVoice->freqCtrlTableNo == 0 )
	{
		pVoice->freqCtrlPointer = 0;
	}
	else
	{
		int addr = ReadMetaDataWord(chip,8);
		addr = ReadMetaDataWord(chip,addr+pVoice->freqCtrlTableNo*2);
		pVoice->freqCtrlTable = pVoice->freqCtrlTableNo;
		pVoice->freqCtrlPointer = addr;
	}
	pVoice->freqCtrlCounter = 0;
	pVoice->freqCtrlFreq = 0;
}

static void
UpdateFreqCtrl( struct namcona *chip, struct voice *pVoice )
{
	int addr,freq;

	if( !pVoice->freqCtrlPointer ) return;

	pVoice->freqCtrlCounter += pVoice->freqCtrlSpeed;
	if( pVoice->freqCtrlCounter > 0xff )
	{
		pVoice->freqCtrlPointer += pVoice->freqCtrlCounter>>8;
		pVoice->freqCtrlCounter &= 0xff;
	}

	switch( ReadMetaDataByte(chip,pVoice->freqCtrlPointer+1) )
	{
	case 0xfd:
		pVoice->freqCtrlTable++;
	case 0xfe:
		addr = ReadMetaDataWord(chip,8);
		addr = ReadMetaDataWord(chip,addr+pVoice->freqCtrlTable*2);
		pVoice->freqCtrlPointer = addr;
		break;
	case 0xff:
		freq  = ReadMetaDataByte(chip,pVoice->freqCtrlPointer+0) * 0x100;
		freq -= 100 * 0x100;
		freq *= pVoice->freqCtrlAmp;
		freq /= 0x100 * 2;
		pVoice->freqCtrlFreq = freq;
		pVoice->freqCtrlPointer = 0;
		break;
	default:
		freq  = ReadMetaDataByte(chip,pVoice->freqCtrlPointer+0) * (0x100 - pVoice->freqCtrlCounter);
		freq += ReadMetaDataByte(chip,pVoice->freqCtrlPointer+1) *          pVoice->freqCtrlCounter;
		freq -= 100 * 0x100;
		freq *= pVoice->freqCtrlAmp;
		freq /= 0x100 * 2;
		pVoice->freqCtrlFreq = freq;
		break;
	}
}

static void
InitVolCtrl( struct namcona *chip, struct voice *pVoice )
{
	pVoice->volCtrlPointer = pVoice->volCtrlTable;
	pVoice->volCtrlCounter = pVoice->volCtrlCount;
	if( pVoice->volCtrlCounter > 0 ) pVoice->volCtrlCounter++;
	if( pVoice->volCtrlPointer )
	{
		pVoice->volCtrlSpeed  = VolCtrlSpeedTable[ReadMetaDataByte(chip,pVoice->volCtrlPointer) & 0x7f];
		pVoice->volCtrlVolEnd = ReadMetaDataByte(chip,pVoice->volCtrlPointer+1)<<8;
		pVoice->volCtrlPointer += 2;
		pVoice->volCtrlVol = 0;
	}
	else
	{
		pVoice->volCtrlVol = 0xffff;
	}
}

static void
UpdateVolCtrl( struct namcona *chip, struct voice *pVoice )
{
	if( !pVoice->volCtrlPointer ) return;

	if( pVoice->volCtrlCounter == 1 )
	{
		pVoice->volCtrlCounter = 0;

		while( pVoice->volCtrlPointer )
		{
			if( ReadMetaDataByte(chip,pVoice->volCtrlPointer) == 0x00 )
			{
				if( ReadMetaDataByte(chip,pVoice->volCtrlPointer+1) == 0xff )
				{
					pVoice->volCtrlPointer = 0;
					pVoice->bActive &= ~1;
				}
				else
				{
					pVoice->volCtrlPointer += 2;
					pVoice->volCtrlSpeed  = VolCtrlSpeedTable[ReadMetaDataByte(chip,pVoice->volCtrlPointer) & 0x7f];
					pVoice->volCtrlVolEnd = ReadMetaDataByte(chip,pVoice->volCtrlPointer+1)<<8;
					pVoice->volCtrlPointer += 2;
				}
				break;
			}
			pVoice->volCtrlPointer += 2;
		}
	}
	else
	{
		int next = 0;

		if( pVoice->volCtrlCounter > 0 )
			pVoice->volCtrlCounter--;

		if( pVoice->volCtrlVol < pVoice->volCtrlVolEnd )
		{
			pVoice->volCtrlVol += pVoice->volCtrlSpeed;
			if( pVoice->volCtrlVol >= pVoice->volCtrlVolEnd )
			{
				pVoice->volCtrlVol = pVoice->volCtrlVolEnd;
				next = 1;
			}
		}
		else
		{
			pVoice->volCtrlVol -= pVoice->volCtrlSpeed;
			if( pVoice->volCtrlVol <= pVoice->volCtrlVolEnd )
			{
				pVoice->volCtrlVol = pVoice->volCtrlVolEnd;
				next = 1;
			}
		}

		if( next )
		{
			UINT8 val1, val2;

			val1 = ReadMetaDataByte(chip,pVoice->volCtrlPointer+0);
			val2 = ReadMetaDataByte(chip,pVoice->volCtrlPointer+1);

			if( val1 == 0x00 && val2 > 0x80 )
			{
				pVoice->volCtrlPointer = 0;
				pVoice->bActive &= ~1;
			}
			else if( val1 != 0x00 || val2 != 0x80 )
			{
				if( val1 == 0x00 )
					pVoice->volCtrlPointer = pVoice->volCtrlTable + val2;
				pVoice->volCtrlSpeed  = VolCtrlSpeedTable[ReadMetaDataByte(chip,pVoice->volCtrlPointer) & 0x7f];
				pVoice->volCtrlVolEnd = ReadMetaDataByte(chip,pVoice->volCtrlPointer+1)<<8;
				pVoice->volCtrlPointer += 2;
			}
		}
	}
}


static void
InitPanCtrl( struct namcona *chip, struct voice *pVoice )
{
	if( pVoice->panCtrlTable != 0 && pVoice->panCtrlSpeed != 0)
	{
		pVoice->panCtrlEnable = 1;
		pVoice->panCtrlCounter = ReadMetaDataByte(chip,pVoice->panCtrlTable);
		if( pVoice->panCtrlCounter != 0xFE )
			pVoice->pan = ReadMetaDataByte(chip,pVoice->panCtrlTable+1);
	}
	else
	{
		pVoice->panCtrlEnable = 0;
	}
}

static void
UpdatePanCtrl( struct namcona *chip, struct voice *pVoice )
{
	if( pVoice->panCtrlEnable )
	{
		if( pVoice->panCtrlCounter == 0 )
		{
			UINT8 pan_2 = ReadMetaDataByte(chip,pVoice->panCtrlTable+2);
			UINT8 pan_3 = ReadMetaDataByte(chip,pVoice->panCtrlTable+3);
			if( !(pan_3&0x80) )
			{
				pVoice->pan += pan_3;
				if( pVoice->pan > 0xff || pVoice->pan >= pan_2)
				{
					pVoice->pan = pan_2;
					pVoice->panCtrlEnable = 0;
				}
			}
			else
			{
				pVoice->pan -= 0x100 - pan_3;
				if( pVoice->pan < 0 || pVoice->pan <= pan_2)
				{
					pVoice->pan = pan_2;
					pVoice->panCtrlEnable = 0;
				}
			}
		}
		else if( pVoice->panCtrlCounter < 0xFE )
		{
			pVoice->panCtrlCounter--;
		}
		else
		{
			if( pVoice->panCtrlSpeed >= 0 )
			{
				pVoice->pan += pVoice->panCtrlSpeed;
				if( pVoice->pan > 0xff )
				{
					pVoice->pan = 0xff;
					pVoice->panCtrlSpeed = -pVoice->panCtrlSpeed;
				}
			}
			else
			{
				pVoice->pan += pVoice->panCtrlSpeed;
				if( pVoice->pan < 0 )
				{
					pVoice->pan = 0;
					pVoice->panCtrlSpeed = -pVoice->panCtrlSpeed;
				}
			}
		}
	}
}

static void
InitPortament( struct namcona *chip, struct voice *pVoice )
{
	pVoice->portamentSpeed = pVoice->portament;
}

static void
UpdatePortament( struct namcona *chip, struct voice *pVoice )
{
	if( !pVoice->portamentSpeed )
	{
		pVoice->portamentNote = pVoice->note;
	}
	else
	{
		int note;
		note = pVoice->note - pVoice->preNote;
		if( note >= 0 )
		{
			note = (note/256+1) * pVoice->portamentSpeed / 2;
			pVoice->portamentNote += note;
			if( pVoice->portamentNote > pVoice->note )
			{
				pVoice->portamentNote = pVoice->note;
				pVoice->portamentSpeed = 0;
			}
		}
		else
		{
			note = (-note/256+1) * pVoice->portamentSpeed / 2;
			pVoice->portamentNote -= note;
			if( pVoice->portamentNote < pVoice->note )
			{
				pVoice->portamentNote = pVoice->note;
				pVoice->portamentSpeed = 0;
			}
		}
	}
}

static void
StartPlayNote( struct namcona *chip, struct voice *pVoice )
{
	if( pVoice->bActive & 4 )
	{
		if( pVoice->delayCount == 0)
		{
			pVoice->bActive &= ~4;
			pVoice->bActive |= 1;
			pVoice->pos = pVoice->start;
			InitFreqCtrl( chip, pVoice );
			InitVolCtrl( chip, pVoice );
			InitPanCtrl( chip, pVoice );
			InitPortament( chip, pVoice );
		}
		else
			pVoice->delayCount--;
	}
}

static void
UpdatePlayNote( struct namcona *chip, struct voice *pVoice )
{
	int vol;
	UINT16 Note = pVoice->portamentNote + (pVoice->dnote<<8) + pVoice->detune + pVoice->baseFreq;
	Note += pVoice->freqCtrlFreq;
	Note &= 0x7fff;
	pVoice->delta = chip->mpPitchTable[Note>>8];
	pVoice->delta += (chip->mpPitchTable[(Note>>8)+1]-pVoice->delta) * (Note&0xff) / 256;

	vol = (pVoice->volCtrlVol>>8) * pVoice->volume / 256;
	pVoice->leftVolume  = (0x100-pVoice->pan) * vol / 256;
	pVoice->rightVolume = pVoice->pan * vol / 256;
}

static void
UpdateStatus( struct namcona *chip, struct voice *pVoice )
{
	StartPlayNote( chip, pVoice );
	if( pVoice->bActive & 1 )
	{
		UpdateFreqCtrl( chip, pVoice );
		UpdateVolCtrl( chip, pVoice );
		UpdatePanCtrl( chip, pVoice );
		UpdatePortament( chip, pVoice );
		UpdatePlayNote( chip, pVoice );
	}
}

static void
UpdateSound( void *param, stream_sample_t **inputs, stream_sample_t **buffer, int length )
{
	struct namcona *chip = param;
	int i;

	for( i=0; i<MAX_SEQUENCE; i++ )
		UpdateSequence(chip, &chip->mSequence[i] );
	for( i=0; i<MAX_SEQUENCE_VOICE; i++ )
		UpdateStatus(chip, &chip->mVoice[i] );
	for( i=0; i<MAX_SEQUENCE; i++ )
		UpdateSequence(chip, &chip->mSequence[i] );
	for( i=0; i<MAX_SEQUENCE_VOICE; i++ )
		UpdateStatus(chip, &chip->mVoice[i] );

	if( length>chip->mSampleRate ) length = chip->mSampleRate;
	memset(chip->mpMixerBuffer, 0, length * sizeof(INT16) * 2);
	for( i=0;i<MAX_VOICE;i++ )
	{
		struct voice *pVoice;
		if( chip->mVoice[i+16].bActive & 3 )
			pVoice = &chip->mVoice[i+16];	//sound effect is high priority
		else
			pVoice = &chip->mVoice[i];

		if( (pVoice->bActive & 3) && pVoice->delta )
		{
			INT32 delta  = pVoice->delta;
			INT32 end    = pVoice->end;
			INT32 pos    = pVoice->pos;
			INT32 lvol   = pVoice->leftVolume;
			INT32 rvol   = pVoice->rightVolume;
			INT16 *pDest = chip->mpMixerBuffer;
			INT16 dat;
			int j;
			for( j=0; j<length; j++ )
			{
				if( pos >= end )
				{
					if( pVoice->flags&0x1000 )
					{
						pos = pos - end + pVoice->loop;
					}
					else
					{
						pVoice->bActive &= ~3;
						break;
					}
				}
				dat = ReadPCMSample(chip,pos>>FIXED_POINT_SHIFT, pVoice->flags)*32;
				*pDest++ += dat*lvol/256;
				*pDest++ += dat*rvol/256;
				pos += delta;
			}
			pVoice->pos = pos;
		}
	}
	RenderSamples(chip, buffer, chip->mpMixerBuffer, length );
} /* UpdateSound */

static void *namcona_start(int sndindex, int clock, const void *config)
{
	const struct NAMCONAinterface *intf = config;
	struct namcona *chip;
	chip = auto_malloc(sizeof(*chip));
	memset(chip, 0, sizeof(*chip));
	chip->mSampleRate = clock;
	chip->mStream = stream_create(0, 2, chip->mSampleRate, chip, UpdateSound);
	chip->mpROM = (UINT16 *)intf->memory_base;
	chip->mpMetaData = chip->mpROM+intf->metadata_offset;

	memset( chip->mVoice, 0x00, sizeof(chip->mVoice) );
	memset( chip->mSequence, 0x00, sizeof(chip->mSequence) );

	chip->mpMixerBuffer = auto_malloc( sizeof(INT16)*chip->mSampleRate*2 );
	chip->mpPitchTable = auto_malloc( sizeof(INT32)*0xff );
		{
			int i;
			for( i=0; i<0xff; i++ )
			{
				int data = i;
				double freq = freq = (1<<FIXED_POINT_SHIFT)/4;
				while( data>0x3a )
				{
					data--;
					freq *= kTwelfthRootTwo;
				}
				while( data<0x3a )
				{
					data++;
					freq /= kTwelfthRootTwo;
				}
				chip->mpPitchTable[i] = (INT32)freq;
			}
		}
	return chip;
} /* NAMCONA_sh_start */

#if 0
static void
DumpSampleTable( FILE *f, struct namcona *chip, int table, unsigned char *special )
{
	int iStart = ReadMetaDataWord(chip,table);
	int i=iStart;
	fprintf( f, "\nsample table:\n" );
	while( i<0x10000 )
	{
		fprintf( f, "%04x(%02x): %04x %04x %04x %04x %04x (len=%d)\n",
			i, (i-iStart)/10,
			ReadMetaDataWord(chip,i+0*2), // flags
			ReadMetaDataWord(chip,i+1*2), // start
			ReadMetaDataWord(chip,i+2*2), // end
			ReadMetaDataWord(chip,i+3*2), // loop
			ReadMetaDataWord(chip,i+4*2), // freq
			ReadMetaDataWord(chip,i+2*2)-ReadMetaDataWord(chip,i+1*2) );
		i+=5*2;
		if( special[i] || special[i+1] ) break;
	}
	fprintf( f, "\n" );
}

static void
DumpBytes( FILE *f, struct namcona *chip, int addr, unsigned char *special )
{
	fprintf( f, "%04x:", addr );
	while( addr<0x10000 )
	{
		fprintf( f, " %02x", ReadMetaDataByte(chip,addr) );
		addr++;
		if( (addr&0xf)==0 ) fprintf( f, "\n%04x;", addr );
		if( special[addr] ) break;
	}
	fprintf( f, "\n" );
}

static void
DumpUnkTable( FILE *f, struct namcona *chip, int table, unsigned char *special )
{
	int iStart = ReadMetaDataWord(chip,table);
	int iFinish = ReadMetaDataWord(chip,iStart);
	int i,addr;

	if( iFinish<iStart )
	{ /* hack for CosmoGang */
		iFinish = ReadMetaDataWord(chip,iStart+2);
	}

	fprintf( f, "\nunk table; %d entries:\n", (iFinish-iStart)/2 );
	fprintf( f, "%04x:", iStart );


	for( i=iStart; i<iFinish; i+=2 )
	{
		addr = ReadMetaDataWord(chip,i);
		fprintf( f, " %04x", addr );
		special[addr] = 1;
	}
	fprintf( f,"\n" );
	for( i=iStart; i<iFinish; i+=2 )
	{
		addr = ReadMetaDataWord(chip,i);
		fprintf( f, "%04x:", addr );
		for(;;)
		{
			fprintf( f, " %02x", ReadMetaDataByte(chip,addr) );
			addr++;
			if( addr>=0x10000 || special[addr] ) break;
		}
		fprintf( f, "\n" );
	}
	fprintf( f, "\n" );
}
#endif

static void
namcona_stop( void *chip )
{
	#if 0
	FILE *f = fopen("snd.txt","w");
	if( f )
	{
		unsigned char *special = malloc(0x10000);
		if( special )
		{
			int i,addr, table;
			memset( special, 0x00, 0x10000 );
			for( i=0; i<0x12; i+=2 )
			{
				addr = ReadMetaDataWord(chip,i);
				if( i ) special[addr] = 1;
				fprintf( f, "%04x: %04x\n", i, addr );
			}
			table = 0x12;
			i = table;
			while( i<0x10000 )
			{
				addr = ReadMetaDataWord(chip,i);
				special[addr] = 1;
				i+=2;
				if( special[i] ) break;
			}
			DumpSampleTable( f, chip, 0x0002, special );
			DumpSampleTable( f, chip, 0x0004, special );
			/* 0x0006 is unused */
			DumpUnkTable( f, chip, 0x0008, special );
			DumpUnkTable( f, chip, 0x000a, special );
			fprintf( f, "\nunknown chunk:\n" );
			DumpBytes(f, chip, ReadMetaDataWord(chip,0x0c), special );
			DumpSampleTable( f, chip, 0x000e, special );
			fprintf( f, "\nSong Table:\n" );
			i = 0x12;
			while( i<0x10000 )
			{
				addr = ReadMetaDataWord(chip,i);
				fprintf( f, "[%02x] ", (i-0x12)/2 );
				DumpBytes( f,chip,addr,special );
				i+=2;
				if( special[i] ) break;
			}
			free( special );
		}
		fclose( f );
	}
	#endif
} /* NAMCONA_sh_stop */




/**************************************************************************
 * Generic get_info
 **************************************************************************/

static void namcona_set_info(void *token, UINT32 state, sndinfo *info)
{
	switch (state)
	{
		/* no parameters to set */
	}
}


void namcona_get_info(void *token, UINT32 state, sndinfo *info)
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case SNDINFO_PTR_SET_INFO:						info->set_info = namcona_set_info;		break;
		case SNDINFO_PTR_START:							info->start = namcona_start;			break;
		case SNDINFO_PTR_STOP:							info->stop = namcona_stop;				break;
		case SNDINFO_PTR_RESET:							/* Nothing */							break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case SNDINFO_STR_NAME:							info->s = "Namco NA";					break;
		case SNDINFO_STR_CORE_FAMILY:					info->s = "Namco custom";				break;
		case SNDINFO_STR_CORE_VERSION:					info->s = "1.0";						break;
		case SNDINFO_STR_CORE_FILE:						info->s = __FILE__;						break;
		case SNDINFO_STR_CORE_CREDITS:					info->s = "Copyright (c) 2004, The MAME Team"; break;
	}
}

