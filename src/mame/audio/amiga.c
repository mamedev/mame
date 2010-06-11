/***************************************************************************

    Amiga Computer / Arcadia Game System

    Driver by:

    Ernesto Corvi & Mariusz Wojcieszek

***************************************************************************/

#include "emu.h"
#include "streams.h"
#include "includes/amiga.h"
#include "cpu/m68000/m68000.h"


/*************************************
 *
 *  Debugging
 *
 *************************************/

#define VERBOSE 0
#define LOG(x) do { if (VERBOSE) logerror x; } while (0)



/*************************************
 *
 *  Constants
 *
 *************************************/

#define CLOCK_DIVIDER	16



/*************************************
 *
 *  Type definitions
 *
 *************************************/

typedef struct _audio_channel audio_channel;
struct _audio_channel
{
	emu_timer *	irq_timer;
	UINT32			curlocation;
	UINT16			curlength;
	UINT16			curticks;
	UINT8			index;
	UINT8			dmaenabled;
	UINT8			manualmode;
	INT8			latched;
};


typedef struct _amiga_audio amiga_audio;
struct _amiga_audio
{
	audio_channel	channel[4];
	sound_stream *	stream;
};



/*************************************
 *
 *  Globals
 *
 *************************************/

static amiga_audio *audio_state;



/*************************************
 *
 *  DMA reload/IRQ signalling
 *
 *************************************/

static TIMER_CALLBACK( signal_irq )
{
	amiga_custom_w(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), REG_INTREQ, 0x8000 | (0x80 << param), 0xffff);
}


static void dma_reload(audio_channel *chan)
{
	chan->curlocation = CUSTOM_REG_LONG(REG_AUD0LCH + chan->index * 8);
	chan->curlength = CUSTOM_REG(REG_AUD0LEN + chan->index * 8);
	timer_adjust_oneshot(chan->irq_timer, ATTOTIME_IN_HZ(15750), chan->index);
	LOG(("dma_reload(%d): offs=%05X len=%04X\n", chan->index, chan->curlocation, chan->curlength));
}



/*************************************
 *
 *  Manual mode data writer
 *
 *************************************/

void amiga_audio_data_w(int which, UINT16 data)
{
	audio_state->channel[which].manualmode = TRUE;
}



/*************************************
 *
 *  Stream updater
 *
 *************************************/

void amiga_audio_update(void)
{
	stream_update(audio_state->stream);
}



static STREAM_UPDATE( amiga_stream_update )
{
	amiga_audio *audio = (amiga_audio *)param;
	int channum, sampoffs = 0;

	/* if all DMA off, disable all channels */
	if (!(CUSTOM_REG(REG_DMACON) & 0x0200))
	{
		audio->channel[0].dmaenabled =
		audio->channel[1].dmaenabled =
		audio->channel[2].dmaenabled =
		audio->channel[3].dmaenabled = FALSE;

		/* clear the sample data to 0 */
		for (channum = 0; channum < 4; channum++)
			memset(outputs[channum], 0, sizeof(stream_sample_t) * samples);
		return;
	}

	samples *= CLOCK_DIVIDER;

	/* update the DMA states on each channel and reload if fresh */
	for (channum = 0; channum < 4; channum++)
	{
		audio_channel *chan = &audio->channel[channum];
		if (!chan->dmaenabled && ((CUSTOM_REG(REG_DMACON) >> channum) & 1))
			dma_reload(chan);
		chan->dmaenabled = (CUSTOM_REG(REG_DMACON) >> channum) & 1;
	}

	/* loop until done */
	while (samples > 0)
	{
		int nextper, nextvol;
		int ticks = samples;

		/* determine the number of ticks we can do in this chunk */
		if (ticks > audio->channel[0].curticks)
			ticks = audio->channel[0].curticks;
		if (ticks > audio->channel[1].curticks)
			ticks = audio->channel[1].curticks;
		if (ticks > audio->channel[2].curticks)
			ticks = audio->channel[2].curticks;
		if (ticks > audio->channel[3].curticks)
			ticks = audio->channel[3].curticks;

		/* loop over channels */
		nextper = nextvol = -1;
		for (channum = 0; channum < 4; channum++)
		{
			int volume = (nextvol == -1) ? CUSTOM_REG(REG_AUD0VOL + channum * 8) : nextvol;
			int period = (nextper == -1) ? CUSTOM_REG(REG_AUD0PER + channum * 8) : nextper;
			audio_channel *chan = &audio->channel[channum];
			stream_sample_t sample;
			int i;

			/* normalize the volume value */
			volume = (volume & 0x40) ? 64 : (volume & 0x3f);
			volume *= 4;

			/* are we modulating the period of the next channel? */
			if ((CUSTOM_REG(REG_ADKCON) >> channum) & 0x10)
			{
				nextper = CUSTOM_REG(REG_AUD0DAT + channum * 8);
				nextvol = -1;
				sample = 0;
			}

			/* are we modulating the volume of the next channel? */
			else if ((CUSTOM_REG(REG_ADKCON) >> channum) & 0x01)
			{
				nextper = -1;
				nextvol = CUSTOM_REG(REG_AUD0DAT + channum * 8);
				sample = 0;
			}

			/* otherwise, we are generating data */
			else
			{
				nextper = nextvol = -1;
				sample = chan->latched * volume;
			}

			/* fill the buffer with the sample */
			for (i = 0; i < ticks; i += CLOCK_DIVIDER)
				outputs[channum][(sampoffs + i) / CLOCK_DIVIDER] = sample;

			/* account for the ticks; if we hit 0, advance */
			chan->curticks -= ticks;
			if (chan->curticks == 0)
			{
				/* reset the clock and ensure we're above the minimum ticks */
				chan->curticks = period;
				if (chan->curticks < 124)
					chan->curticks = 124;

				/* move forward one byte; if we move to an even byte, fetch new */
				if (chan->dmaenabled || chan->manualmode)
					chan->curlocation++;
				if (chan->dmaenabled && !(chan->curlocation & 1))
				{
					CUSTOM_REG(REG_AUD0DAT + channum * 8) = amiga_chip_ram_r(chan->curlocation);
					if (chan->curlength != 0)
						chan->curlength--;

					/* if we run out of data, reload the dma */
					if (chan->curlength == 0)
						dma_reload(chan);
				}

				/* latch the next byte of the sample */
				if (!(chan->curlocation & 1))
					chan->latched = CUSTOM_REG(REG_AUD0DAT + channum * 8) >> 8;
				else
					chan->latched = CUSTOM_REG(REG_AUD0DAT + channum * 8) >> 0;

				/* if we're in manual mode, signal an interrupt once we latch the low byte */
				if (!chan->dmaenabled && chan->manualmode && (chan->curlocation & 1))
				{
					signal_irq(device->machine, NULL, channum);
					chan->manualmode = FALSE;
				}
			}
		}

		/* bump ourselves forward by the number of ticks */
		sampoffs += ticks;
		samples -= ticks;
	}
}



/*************************************
 *
 *  Sound system startup
 *
 *************************************/

static DEVICE_START( amiga_sound )
{
	running_machine *machine = device->machine;
	int i;

	/* allocate a new audio state */
	audio_state = (amiga_audio *)downcast<legacy_device_base *>(device)->token();
	for (i = 0; i < 4; i++)
	{
		audio_state->channel[i].index = i;
		audio_state->channel[i].irq_timer = timer_alloc(machine, signal_irq, NULL);
	}

	/* create the stream */
	audio_state->stream = stream_create(device, 0, 4, device->clock() / CLOCK_DIVIDER, audio_state, amiga_stream_update);
}


DEVICE_GET_INFO( amiga_sound )
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case DEVINFO_INT_TOKEN_BYTES:					info->i = sizeof(*audio_state);					break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case DEVINFO_FCT_START:							info->start = DEVICE_START_NAME(amiga_sound);	break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case DEVINFO_STR_NAME:							strcpy(info->s, "Amiga Paula");				break;
		case DEVINFO_STR_SOURCE_FILE:						strcpy(info->s, __FILE__);						break;
	}
}


DEFINE_LEGACY_SOUND_DEVICE(AMIGA, amiga_sound);
