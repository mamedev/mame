/***************************************************************************

    Amiga Computer / Arcadia Game System

    Driver by:

    Ernesto Corvi & Mariusz Wojcieszek

***************************************************************************/

#include "emu.h"
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

struct audio_channel
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


struct amiga_audio
{
	audio_channel	channel[4];
	sound_stream *	stream;
};


INLINE amiga_audio *get_safe_token( device_t *device )
{
        assert(device != NULL);
        assert(device->type() == AMIGA);

        return (amiga_audio *)downcast<amiga_sound_device *>(device)->token();
}

/*************************************
 *
 *  DMA reload/IRQ signalling
 *
 *************************************/

static TIMER_CALLBACK( signal_irq )
{
	amiga_custom_w(machine.device("maincpu")->memory().space(AS_PROGRAM), REG_INTREQ, 0x8000 | (0x80 << param), 0xffff);
}


static void dma_reload(amiga_state *state, audio_channel *chan)
{
	chan->curlocation = CUSTOM_REG_LONG(REG_AUD0LCH + chan->index * 8);
	chan->curlength = CUSTOM_REG(REG_AUD0LEN + chan->index * 8);
	chan->irq_timer->adjust(attotime::from_hz(15750), chan->index);
	LOG(("dma_reload(%d): offs=%05X len=%04X\n", chan->index, chan->curlocation, chan->curlength));
}



/*************************************
 *
 *  Manual mode data writer
 *
 *************************************/

void amiga_audio_data_w(device_t *device, int which, UINT16 data)
{
	amiga_audio *audio_state = get_safe_token(device);

	audio_state->channel[which].manualmode = TRUE;
}



/*************************************
 *
 *  Stream updater
 *
 *************************************/

void amiga_audio_update(device_t *device)
{
	amiga_audio *audio_state = get_safe_token(device);

	audio_state->stream->update();
}



static STREAM_UPDATE( amiga_stream_update )
{
	amiga_state *state = device->machine().driver_data<amiga_state>();
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
			dma_reload(state, chan);
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
					CUSTOM_REG(REG_AUD0DAT + channum * 8) = (*state->m_chip_ram_r)(state, chan->curlocation);
					if (chan->curlength != 0)
						chan->curlength--;

					/* if we run out of data, reload the dma */
					if (chan->curlength == 0)
						dma_reload(state, chan);
				}

				/* latch the next byte of the sample */
				if (!(chan->curlocation & 1))
					chan->latched = CUSTOM_REG(REG_AUD0DAT + channum * 8) >> 8;
				else
					chan->latched = CUSTOM_REG(REG_AUD0DAT + channum * 8) >> 0;

				/* if we're in manual mode, signal an interrupt once we latch the low byte */
				if (!chan->dmaenabled && chan->manualmode && (chan->curlocation & 1))
				{
					signal_irq(device->machine(), NULL, channum);
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
	amiga_audio *audio_state = get_safe_token(device);
	int i;

	for (i = 0; i < 4; i++)
	{
		audio_state->channel[i].index = i;
		audio_state->channel[i].irq_timer = device->machine().scheduler().timer_alloc(FUNC(signal_irq));
	}

	/* create the stream */
	audio_state->stream = device->machine().sound().stream_alloc(*device, 0, 4, device->clock() / CLOCK_DIVIDER, audio_state, amiga_stream_update);
}


const device_type AMIGA = &device_creator<amiga_sound_device>;

amiga_sound_device::amiga_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, AMIGA, "Amiga Paula", tag, owner, clock),
	  device_sound_interface(mconfig, *this)
{
	m_token = global_alloc_clear(amiga_audio);
}

//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void amiga_sound_device::device_config_complete()
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void amiga_sound_device::device_start()
{
	DEVICE_START_NAME( amiga_sound )(this);
}

//-------------------------------------------------
//  sound_stream_update - handle a stream update
//-------------------------------------------------

void amiga_sound_device::sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples)
{
	// should never get here
	fatalerror("sound_stream_update called; not applicable to legacy sound devices\n");
}


