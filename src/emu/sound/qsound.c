/***************************************************************************

  Capcom System QSound(tm)
  ========================

  Driver by Paul Leaman (paul@vortexcomputing.demon.co.uk)
        and Miguel Angel Horna (mahorna@teleline.es)

  A 16 channel stereo sample player.

  QSpace position is simulated by panning the sound in the stereo space.

  Register
  0  xxbb   xx = unknown bb = start high address
  1  ssss   ssss = sample start address
  2  pitch
  3  unknown (always 0x8000)
  4  loop offset from end address
  5  end
  6  master channel volume
  7  not used
  8  Balance (left=0x0110  centre=0x0120 right=0x0130)
  9  unknown (most fixed samples use 0 for this register)

  Many thanks to CAB (the author of Amuse), without whom this probably would
  never have been finished.

  If anybody has some information about this hardware, please send it to me
  to mahorna@teleline.es or 432937@cepsz.unizar.es.
  http://teleline.terra.es/personal/mahorna

***************************************************************************/

#include <math.h>
#include "sndintrf.h"
#include "streams.h"
#include "qsound.h"

/*
Debug defines
*/
#define LOG_WAVE	0
#define LOG_QSOUND  0

/* 8 bit source ROM samples */
typedef INT8 QSOUND_SRC_SAMPLE;


#define QSOUND_CLOCKDIV 166			 /* Clock divider */
#define QSOUND_CHANNELS 16
typedef stream_sample_t QSOUND_SAMPLE;

struct QSOUND_CHANNEL
{
	INT32 bank;	   /* bank (x16)    */
	INT32 address;	/* start address */
	INT32 pitch;	  /* pitch */
	INT32 reg3;	   /* unknown (always 0x8000) */
	INT32 loop;	   /* loop address */
	INT32 end;		/* end address */
	INT32 vol;		/* master volume */
	INT32 pan;		/* Pan value */
	INT32 reg9;	   /* unknown */

	/* Work variables */
	INT32 key;		/* Key on / key off */

	INT32 lvol;	   /* left volume */
	INT32 rvol;	   /* right volume */
	INT32 lastdt;	 /* last sample value */
	INT32 offset;	 /* current offset counter */
};

struct qsound_info
{
	/* Private variables */
	const struct QSound_interface *intf;	/* Interface  */
	sound_stream * stream;				/* Audio stream */
	struct QSOUND_CHANNEL channel[QSOUND_CHANNELS];
	int data;				  /* register latch data */
	QSOUND_SRC_SAMPLE *sample_rom;	/* Q sound sample ROM */
	UINT32 sample_rom_length;

	int pan_table[33];		 /* Pan volume table */
	float frq_ratio;		   /* Frequency ratio */

	#if LOG_WAVE
	FILE *fpRawDataL;
	FILE *fpRawDataR;
	#endif
};

/* Function prototypes */
void qsound_update( void *param, stream_sample_t **inputs, stream_sample_t **outputs, int length );
void qsound_set_command(struct qsound_info *chip, int data, int value);

static void *qsound_start(int sndindex, int clock, const void *config)
{
	struct qsound_info *chip;
	int i;

	chip = auto_malloc(sizeof(*chip));
	memset(chip, 0, sizeof(chip));

	chip->intf = config;

	chip->sample_rom = (QSOUND_SRC_SAMPLE *)memory_region(chip->intf->region);
	chip->sample_rom_length = memory_region_length(chip->intf->region);

	memset(chip->channel, 0, sizeof(chip->channel));

	chip->frq_ratio = 16.0;

	/* Create pan table */
	for (i=0; i<33; i++)
	{
		chip->pan_table[i]=(int)((256/sqrt(32)) * sqrt(i));
	}

#if LOG_QSOUND
	logerror("Pan table\n");
	for (i=0; i<33; i++)
		logerror("%02x ", chip->pan_table[i]);
#endif
	{
		/* Allocate stream */
		chip->stream = stream_create(
			0, 2,
			clock / QSOUND_CLOCKDIV,
			chip,
			qsound_update );
	}

#if LOG_WAVE
	chip->fpRawDataR=fopen("qsoundr.raw", "w+b");
	chip->fpRawDataL=fopen("qsoundl.raw", "w+b");
	if (!chip->fpRawDataR || !chip->fpRawDataL)
	{
		return NULL;
	}
#endif

	/* state save */
	for (i=0; i<QSOUND_CHANNELS; i++)
	{
		state_save_register_item("QSound", sndindex*QSOUND_CHANNELS+i, chip->channel[i].bank);
		state_save_register_item("QSound", sndindex*QSOUND_CHANNELS+i, chip->channel[i].address);
		state_save_register_item("QSound", sndindex*QSOUND_CHANNELS+i, chip->channel[i].pitch);
		state_save_register_item("QSound", sndindex*QSOUND_CHANNELS+i, chip->channel[i].loop);
		state_save_register_item("QSound", sndindex*QSOUND_CHANNELS+i, chip->channel[i].end);
		state_save_register_item("QSound", sndindex*QSOUND_CHANNELS+i, chip->channel[i].vol);
		state_save_register_item("QSound", sndindex*QSOUND_CHANNELS+i, chip->channel[i].pan);
		state_save_register_item("QSound", sndindex*QSOUND_CHANNELS+i, chip->channel[i].key);
		state_save_register_item("QSound", sndindex*QSOUND_CHANNELS+i, chip->channel[i].lvol);
		state_save_register_item("QSound", sndindex*QSOUND_CHANNELS+i, chip->channel[i].rvol);
		state_save_register_item("QSound", sndindex*QSOUND_CHANNELS+i, chip->channel[i].lastdt);
		state_save_register_item("QSound", sndindex*QSOUND_CHANNELS+i, chip->channel[i].offset);
	}

	return chip;
}

static void qsound_stop (void *_chip)
{
#if LOG_WAVE
	if (chip->fpRawDataR)
	{
		fclose(chip->fpRawDataR);
	}
	if (chip->fpRawDataL)
	{
		fclose(chip->fpRawDataL);
	}
#endif
}

WRITE8_HANDLER( qsound_data_h_w )
{
	struct qsound_info *chip = sndti_token(SOUND_QSOUND, 0);
	chip->data=(chip->data&0xff)|(data<<8);
}

WRITE8_HANDLER( qsound_data_l_w )
{
	struct qsound_info *chip = sndti_token(SOUND_QSOUND, 0);
	chip->data=(chip->data&0xff00)|data;
}

WRITE8_HANDLER( qsound_cmd_w )
{
	struct qsound_info *chip = sndti_token(SOUND_QSOUND, 0);
	qsound_set_command(chip, data, chip->data);
}

READ8_HANDLER( qsound_status_r )
{
	/* Port ready bit (0x80 if ready) */
	return 0x80;
}

void qsound_set_command(struct qsound_info *chip, int data, int value)
{
	int ch=0,reg=0;
	if (data < 0x80)
	{
		ch=data>>3;
		reg=data & 0x07;
	}
	else
	{
		if (data < 0x90)
		{
			ch=data-0x80;
			reg=8;
		}
		else
		{
			if (data >= 0xba && data < 0xca)
			{
				ch=data-0xba;
				reg=9;
			}
			else
			{
				/* Unknown registers */
				ch=99;
				reg=99;
			}
		}
	}

	switch (reg)
	{
		case 0: /* Bank */
			ch=(ch+1)&0x0f;	/* strange ... */
			chip->channel[ch].bank=(value&0x7f)<<16;
#ifdef MAME_DEBUG
			if (!value & 0x8000)
				popmessage("Register3=%04x",value);
#endif

			break;
		case 1: /* start */
			chip->channel[ch].address=value;
			break;
		case 2: /* pitch */
			chip->channel[ch].pitch=value * 16;
			if (!value)
			{
				/* Key off */
				chip->channel[ch].key=0;
			}
			break;
		case 3: /* unknown */
			chip->channel[ch].reg3=value;
#ifdef MAME_DEBUG
			if (value != 0x8000)
				popmessage("Register3=%04x",value);
#endif
			break;
		case 4: /* loop offset */
			chip->channel[ch].loop=value;
			break;
		case 5: /* end */
			chip->channel[ch].end=value;
			break;
		case 6: /* master volume */
			if (value==0)
			{
				/* Key off */
				chip->channel[ch].key=0;
			}
			else if (chip->channel[ch].key==0)
			{
				/* Key on */
				chip->channel[ch].key=1;
				chip->channel[ch].offset=0;
				chip->channel[ch].lastdt=0;
			}
			chip->channel[ch].vol=value;
			break;

		case 7:  /* unused */
#ifdef MAME_DEBUG
				popmessage("UNUSED QSOUND REG 7=%04x",value);
#endif

			break;
		case 8:
			{
			   int pandata=(value-0x10)&0x3f;
			   if (pandata > 32)
			   {
					pandata=32;
			   }
			   chip->channel[ch].rvol=chip->pan_table[pandata];
			   chip->channel[ch].lvol=chip->pan_table[32-pandata];
			   chip->channel[ch].pan = value;
			}
			break;
		 case 9:
			chip->channel[ch].reg9=value;
/*
#ifdef MAME_DEBUG
            popmessage("QSOUND REG 9=%04x",value);
#endif
*/
			break;
	}
#if LOG_QSOUND
	logerror("QSOUND WRITE %02x CH%02d-R%02d =%04x\n", data, ch, reg, value);
#endif
}


void qsound_update( void *param, stream_sample_t **inputs, stream_sample_t **buffer, int length )
{
	struct qsound_info *chip = param;
	int i,j;
	int rvol, lvol, count;
	struct QSOUND_CHANNEL *pC=&chip->channel[0];
	stream_sample_t  *datap[2];

	datap[0] = buffer[0];
	datap[1] = buffer[1];
	memset( datap[0], 0x00, length * sizeof(*datap[0]) );
	memset( datap[1], 0x00, length * sizeof(*datap[1]) );

	for (i=0; i<QSOUND_CHANNELS; i++)
	{
		if (pC->key)
		{
			QSOUND_SAMPLE *pOutL=datap[0];
			QSOUND_SAMPLE *pOutR=datap[1];
			rvol=(pC->rvol*pC->vol)>>8;
			lvol=(pC->lvol*pC->vol)>>8;

			for (j=length-1; j>=0; j--)
			{
				count=(pC->offset)>>16;
				pC->offset &= 0xffff;
				if (count)
				{
					pC->address += count;
					if (pC->address >= pC->end)
					{
						if (!pC->loop)
						{
							/* Reached the end of a non-looped sample */
							pC->key=0;
							break;
						}
						/* Reached the end, restart the loop */
						pC->address = (pC->end - pC->loop) & 0xffff;
					}
					pC->lastdt=chip->sample_rom[(pC->bank+pC->address)%(chip->sample_rom_length)];
				}

				(*pOutL) += ((pC->lastdt * lvol) >> 6);
				(*pOutR) += ((pC->lastdt * rvol) >> 6);
				pOutL++;
				pOutR++;
				pC->offset += pC->pitch;
			}
		}
		pC++;
	}

#if LOG_WAVE
	fwrite(datap[0], length*sizeof(QSOUND_SAMPLE), 1, chip->fpRawDataL);
	fwrite(datap[1], length*sizeof(QSOUND_SAMPLE), 1, chip->fpRawDataR);
#endif
}



/**************************************************************************
 * Generic get_info
 **************************************************************************/

static void qsound_set_info(void *token, UINT32 state, sndinfo *info)
{
	switch (state)
	{
		/* no parameters to set */
	}
}


void qsound_get_info(void *token, UINT32 state, sndinfo *info)
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case SNDINFO_PTR_SET_INFO:						info->set_info = qsound_set_info;		break;
		case SNDINFO_PTR_START:							info->start = qsound_start;				break;
		case SNDINFO_PTR_STOP:							info->stop = qsound_stop;				break;
		case SNDINFO_PTR_RESET:							/* Nothing */							break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case SNDINFO_STR_NAME:							info->s = "Q-Sound";					break;
		case SNDINFO_STR_CORE_FAMILY:					info->s = "Capcom custom";				break;
		case SNDINFO_STR_CORE_VERSION:					info->s = "1.0";						break;
		case SNDINFO_STR_CORE_FILE:						info->s = __FILE__;						break;
		case SNDINFO_STR_CORE_CREDITS:					info->s = "Copyright (c) 2004, The MAME Team"; break;
	}
}

/**************** end of file ****************/
