/*
 * Sega System 32 Multi/Model 1 custom PCM chip (315-5560) emulation.
 *
 * by R. Belmont.  Info from AMUSE, Hoot, and the YMF278B (OPL4).
 * This chip is sort of a dry run for the PCM section of the YMF278B,
 * and it has many obvious similarities to that chip.
 *
 * voice registers:
 * 0: pan
 * 1: sample to play (PCM chip uses table to figure out)
 * 2: LSB of pitch
 * 3: MSB of pitch
 * 4: voice control: top bit = 1 for key on, 0 for key off
 * 5: bit 0: loop, bits 1-7 = volume attenuate (0=max, 7f=min)
 * 6: LFO (OutRunners engine, singing man in Daytona)
 * 7: LFO
 *
 * The first sample ROM contains a variable length table with 12
 * bytes per instrument/sample.  The end of the table is marked
 * by 12 bytes of 0xFF.  This is very similar to the YMF278B.
 *
 * The first 3 bytes are the offset into the file (big endian).
 * The next 2 are the loop start offset into the file (big endian)
 * The next 2 are the 2's complement of the total sample size (big endian)
 * The next byte is unknown.
 * The next 3 are envelope attack / decay / release parameters (not yet emulated)
 *
 */

#include <math.h>
#include "sndintrf.h"
#include "streams.h"
#include "multipcm.h"

#define MULTIPCM_CLOCKDIV    	(360.0)
#define MULTIPCM_ONE		(18)

static int ctbl[] =
{
	0, 1, 2, 3, 4, 5, 6 , -1,	// voice number mapping
	7, 8, 9, 10,11,12,13, -1,
	14,15,16,17,18,19,20, -1,
	21,22,23,24,25,26,27, -1,
};

static int decaytbl[16] =	// decay times
{
     0,   300,   800,  1400,
  2100,  3000,  4000,  5200,
  6600,  8200, 10000, 12000,
 14500, 17500, 21000, 25000
};


// sample info struct
typedef struct PCM_t
{
	INT32	st;
	INT32	size;
	INT32	loop;
	UINT8	env[4];
} PCMInfoT;

// voice structure
typedef struct Voice_t
{
	INT8    active;		// active flag
	INT8	loop;	        // loop flag
	INT32   end;		// length of sample
	INT32	loopst;		// loop start offset
	INT32     pan;		// panning
	INT32 	vol;		// volume
	INT8    *pSamp;		// pointer to start of sample data

	INT32	ptdelta;	// pitch step
	INT32	ptoffset;	// fixed point offset
	INT32	ptsum;		// fixed point sum
  	INT32 	relamt;		// release amount
  	INT32 	relcount;	// release counter
	INT8	relstage;	// release stage
} VoiceT;

// chip structure
typedef struct MultiPCM_t
{
	sound_stream * stream;

	UINT8 registers[28][8];	// 8 registers per voice?
	UINT32 bankL, bankR;

	VoiceT Voices[28];
	int curreg, curvoice;
	INT8 *romptr;

	long dlttbl[0x1001];		// pre-calculated pitch table
	long voltbl[128];	// pre-calculated volume table
	long pantbl[16];		// pre-calculated panning table

	PCMInfoT samples[512];

} MultiPCMT;

static void MultiPCM_postload(void *param)
{
	MultiPCMT *mpcm = param;
	int j;
	for (j = 0; j < 28; j++)
	{
		int inum = mpcm->registers[j][1] | ((mpcm->registers[j][2]&0x1)<<8);
	  	mpcm->Voices[j].pSamp = &mpcm->romptr[mpcm->samples[inum].st];
	}
}

static void MultiPCM_update(void *param, stream_sample_t **inputs, stream_sample_t **buffer, int length )
{
	MultiPCMT *mpcm = param;
	stream_sample_t  *datap[2];
	int i, j;
	signed long lvol, rvol, mlvol, mrvol;
	INT8 *pSamp;
	long	cnt, ptsum, ptoffset, ptdelta, end;
	VoiceT	*vptr;
	float decTemp;
	char relstage;
	int relcount, relamt;
	float invrelamt;

	datap[0] = buffer[0];
	datap[1] = buffer[1];

	memset(datap[0], 0, sizeof(*datap[0])*length);
	memset(datap[1], 0, sizeof(*datap[1])*length);

	for (j = 0; j < 28; j++)
	{
		vptr = &mpcm->Voices[j];

		// is voice playing?
		if ((vptr->active) || (vptr->relstage))
		{	// only calculate volume once per voice per update cycle
			rvol = mpcm->pantbl[vptr->pan];
			lvol = mpcm->pantbl[15-vptr->pan];

			mrvol = rvol = (rvol * vptr->vol)>>8;
			mlvol = lvol = (lvol * vptr->vol)>>8;

			decTemp = 1.0f;

			// copy our "working set" into locals
			ptsum = vptr->ptsum;
			ptoffset = vptr->ptoffset;
			ptdelta = vptr->ptdelta;
			end = vptr->end;
			pSamp = vptr->pSamp;
			relstage = vptr->relstage;
			relcount = vptr->relcount;
			relamt = vptr->relamt;
			invrelamt = 1.f / (float)relamt;

			for (i = 0; i < length; i++)
			{
				cnt = ptsum >> MULTIPCM_ONE;
				ptsum &= ((1<<MULTIPCM_ONE)-1);
				ptoffset += cnt;

				if (ptoffset >= end)
				{
					if (vptr->loop)
					{
						ptoffset = vptr->loopst;
					}
					else
					{
						vptr->active = 0;
						break;
					}
				}

				if (relstage)
				{
					relcount++;
					if (relcount > relamt)
					{
						relstage = 0;
						vptr->relstage = 0;
						break;
					}

					decTemp = 1.0f - (relcount * invrelamt);

					lvol = mlvol * decTemp;
					rvol = mrvol * decTemp;
				}

				ptsum += ptdelta;

				datap[0][i] += ((pSamp[ptoffset] * lvol)>>2);
				datap[1][i] += ((pSamp[ptoffset] * rvol)>>2);
			}

			// copy back the working values we need to keep
			vptr->ptsum = ptsum;
			vptr->ptoffset = ptoffset;
			vptr->relcount = relcount;
		}
	}
}

static void *multipcm_start(int sndindex, int clock, const void *config)
{
	int i;
	double unity = (double)(1<<MULTIPCM_ONE);
	UINT8* phdr;
	long nowadrs;
	long idx;
	const struct MultiPCM_interface *intf = config;
	MultiPCMT *mpcm;

	// make volume table
	double	max=255.0;
	double	db=(48.0/128);

	mpcm = auto_malloc(sizeof(*mpcm));
	memset(mpcm, 0, sizeof(*mpcm));

	for (i = 0; i < 128; i++)
	{
		mpcm->voltbl[i]=max;
		max /= pow(10.0,db/20.0);
	}
	mpcm->voltbl[127]=0;

	// make pan table
	for(i=0; i<16; i++)
	{
		mpcm->pantbl[i]=(long)( (255/sqrt(15)) * sqrt(i));
	}

	mpcm->curreg = mpcm->curvoice = 0;

	mpcm->romptr = (INT8 *)memory_region(intf->region);

	for (i = 0; i < 28; i++)
	{
		mpcm->Voices[i].active = 0;
		mpcm->Voices[i].ptsum = 0;
		mpcm->Voices[i].ptoffset = 0;
		mpcm->Voices[i].loop = 0;
		mpcm->Voices[i].loopst = 0;
		mpcm->Voices[i].end = 0;
		mpcm->Voices[i].pan = 0;
		mpcm->Voices[i].vol = 0;
		mpcm->Voices[i].relamt = 0;
		mpcm->Voices[i].relcount = 0;
		mpcm->Voices[i].relstage = 0;
	}

	mpcm->stream = stream_create(0, 2, clock / MULTIPCM_CLOCKDIV, mpcm, MultiPCM_update);

	// make pitch delta table (1 octave)
	for(i=0; i<0x1001; i++)
	{
		mpcm->dlttbl[i] = (long)(unity * (1.0 + ((double)i / 4096.0)));
	}

	// precalculate the PCM data for a small speedup
	phdr = (UINT8 *)mpcm->romptr;
	for(i = 0; i < 511; i++)
	{
		idx = i*12;
		nowadrs = (phdr[idx + 0]<<16) + (phdr[idx + 1]<<8) + (phdr[idx + 2]);

		if((nowadrs == 0)||(nowadrs==0xffffff))
		{	// invalid entry
			mpcm->samples[i].st=0;
			mpcm->samples[i].size=0;
		}
		else
		{
			mpcm->samples[i].st = nowadrs;
			mpcm->samples[i].loop = (phdr[idx + 3]<<8) + (phdr[idx + 4]);
			mpcm->samples[i].size = 0xffff - ((phdr[idx + 5]<<8) + (phdr[idx + 6]));
			mpcm->samples[i].env[0] = phdr[idx + 8];
			mpcm->samples[i].env[1] = phdr[idx + 9];
			mpcm->samples[i].env[2] = phdr[idx + 10];
		}
	}

	/* set up the save state info */
	{
		int v;
		char mname[20];

		sprintf(mname, "MultiPCM %d", sndindex);

		state_save_register_item(mname, sndindex, mpcm->bankL);
		state_save_register_item(mname, sndindex, mpcm->bankR);

		state_save_register_item_2d_array(mname, sndindex, mpcm->registers);

		for (v = 0; v < 28; v++)
		{
			char mname2[32];

			sprintf(mname2, "MultiPCM %d v %d", sndindex, v);

			state_save_register_item(mname2, sndindex, mpcm->Voices[v].active);
			state_save_register_item(mname2, sndindex, mpcm->Voices[v].loop);
			state_save_register_item(mname2, sndindex, mpcm->Voices[v].end);
			state_save_register_item(mname2, sndindex, mpcm->Voices[v].loopst);
			state_save_register_item(mname2, sndindex, mpcm->Voices[v].pan);
			state_save_register_item(mname2, sndindex, mpcm->Voices[v].vol);
			state_save_register_item(mname2, sndindex, mpcm->Voices[v].ptdelta);
			state_save_register_item(mname2, sndindex, mpcm->Voices[v].ptoffset);
			state_save_register_item(mname2, sndindex, mpcm->Voices[v].ptsum);
			state_save_register_item(mname2, sndindex, mpcm->Voices[v].relamt);
			state_save_register_item(mname2, sndindex, mpcm->Voices[v].relstage);
		}

		state_save_register_item(mname, sndindex, mpcm->curreg);
		state_save_register_item(mname, sndindex, mpcm->curvoice);
	}

	state_save_register_func_postload_ptr(MultiPCM_postload, mpcm);

	return mpcm;
}

/* write register */
static void MultiPCM_reg_w(int chip, int offset, UINT8 data)
{
	int ppp, inum;
	signed short pitch;
	long pt_abs, pt_oct, st;
	int vnum;
	MultiPCMT *cptr = sndti_token(SOUND_MULTIPCM, chip);
	VoiceT *vptr;

	stream_update(cptr->stream);

	switch (offset)
	{
		case 0:	      	// data / status
			if ((cptr->curvoice > 27) || (cptr->curvoice < 0))
			{
				//logerror("MPCM: unknown write to voice > 28\n");
				return;
			}

			vnum = cptr->curvoice;
			cptr->registers[vnum][cptr->curreg] = data;

			vptr = &cptr->Voices[vnum];

			switch (cptr->curreg)
			{
			 	case 0:	// panning
					ppp = (cptr->registers[vnum][0]>>4)&0xf;
					if (ppp >= 8)
					{
						ppp = -(16-ppp);
					}
					vptr->pan = ppp + 8;
					break;

				case 1:	// sample
					break;

				case 2:	// pitch LSB
				// MUST fall through to update pitch also!
				case 3: // pitch MSB
					// compute frequency divisor
					pitch = (cptr->registers[vnum][3]<<8) + cptr->registers[vnum][2];
					pt_abs = (double)abs(pitch);
					pt_oct = pt_abs>>12;
					if(pitch < 0)
					{
						vptr->ptdelta = cptr->dlttbl[0x1000 - (pt_abs&0xfff)];
						vptr->ptdelta >>= (pt_oct+1);
					}
					else
					{
						vptr->ptdelta = cptr->dlttbl[pt_abs&0xfff];
						vptr->ptdelta <<= pt_oct;
					}
					break;

				case 4:	// key on/off
					if (data & 0x80)
					{
						inum = cptr->registers[vnum][1];

						// calc decay amount
						vptr->relamt = decaytbl[(0x0f - cptr->samples[inum].env[2])];

						// compute start and end pointers
						st = cptr->samples[inum].st;

						// perform banking
						if (st >= 0x100000)
						{
/*                              logerror("MPCM: key on chip %d voice %d\n", chip, vnum);
                           logerror("regs %02x %02x %02x %02x %02x %02x %02x %02x\n", cptr->registers[vnum][0],
                               cptr->registers[vnum][1],cptr->registers[vnum][2],cptr->registers[vnum][3],
                               cptr->registers[vnum][4],cptr->registers[vnum][5],
                               cptr->registers[vnum][6],cptr->registers[vnum][7]);*/

							if (vptr->pan < 8)
							{
								st = (st & 0xfffff) + cptr->bankL;
							}
							else
							{
								st = (st & 0xfffff) + cptr->bankR;
							}
						}

						vptr->pSamp = &cptr->romptr[st];
						vptr->end = cptr->samples[inum].size;
						vptr->loopst = cptr->samples[inum].loop;

						vptr->ptoffset = 0;
						vptr->ptsum = 0;
						vptr->active = 1;
						vptr->relstage = 0;
					}
					else
					{
//                      logerror("MPCM: key off chip %d voice %d\n", chip, vnum);
						vptr->active = 0;
						vptr->relcount = 0;
						if ((vptr->loop) && (vptr->pSamp))
						{
							vptr->relstage = 1;
						}
					}
				 	break;

				case 5:	// volume/loop
					vptr->vol = cptr->voltbl[(cptr->registers[vnum][5]>>1)&0x7f];
					vptr->loop = (cptr->registers[vnum][5]&0x1) || !vptr->loopst;
					break;

				case 6: // ??? LFO? reverb?
				case 7:
//                  logerror("write %x to reg %d, voice %d\n", data, cptr->curreg, vnum);
					break;

				default:
//                  logerror("write %x to reg %d, voice %d\n", data, cptr->curreg, vnum);
					break;
			}
			break;

		case 1:		// voice select
			cptr->curvoice = ctbl[data&0x1f];
			break;

		case 2:		// register select
			cptr->curreg = data;
			if (cptr->curreg > 7)
				cptr->curreg = 7;
			break;
	}
}

/* read register */

static UINT8 MultiPCM_reg_r(int chip, int offset)
{
	UINT8 retval = 0;

	switch (offset)
	{
	case 0:
		retval = 0;	// always return READY
		break;

	default:
		//logerror("read from unknown MPCM register %ld\n", offset);
		break;
	}

	return retval;
}

/* MAME/M1 access functions */

READ8_HANDLER( MultiPCM_reg_0_r )
{
	return MultiPCM_reg_r(0, offset);
}

WRITE8_HANDLER( MultiPCM_reg_0_w )
{
	MultiPCM_reg_w(0, offset, data);
}

READ8_HANDLER( MultiPCM_reg_1_r )
{
	return MultiPCM_reg_r(1, offset);
}

WRITE8_HANDLER( MultiPCM_reg_1_w )
{
	MultiPCM_reg_w(1, offset, data);
}

void multipcm_set_bank(int which, UINT32 leftoffs, UINT32 rightoffs)
{
	struct MultiPCM_t *mpcm = sndti_token(SOUND_MULTIPCM, which);
	mpcm->bankL = leftoffs;
	mpcm->bankR = rightoffs;
}



/**************************************************************************
 * Generic get_info
 **************************************************************************/

static void multipcm_set_info(void *token, UINT32 state, sndinfo *info)
{
	switch (state)
	{
		/* no parameters to set */
	}
}


void multipcm_get_info(void *token, UINT32 state, sndinfo *info)
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case SNDINFO_PTR_SET_INFO:						info->set_info = multipcm_set_info;		break;
		case SNDINFO_PTR_START:							info->start = multipcm_start;			break;
		case SNDINFO_PTR_STOP:							/* Nothing */							break;
		case SNDINFO_PTR_RESET:							/* Nothing */							break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case SNDINFO_STR_NAME:							info->s = "MultiPCM";					break;
		case SNDINFO_STR_CORE_FAMILY:					info->s = "Sega custom";				break;
		case SNDINFO_STR_CORE_VERSION:					info->s = "1.0";						break;
		case SNDINFO_STR_CORE_FILE:						info->s = __FILE__;						break;
		case SNDINFO_STR_CORE_CREDITS:					info->s = "Copyright (c) 2004, The MAME Team"; break;
	}
}

