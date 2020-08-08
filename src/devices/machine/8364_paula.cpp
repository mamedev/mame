// license: BSD-3-Clause
// copyright-holders: Aaron Giles, Dirk Best
/***************************************************************************

    Commodore 8364 "Paula"

***************************************************************************/

#include "emu.h"
#include "8364_paula.h"

//#define VERBOSE 1
#include "logmacro.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(PAULA_8364, paula_8364_device, "paula_8364", "8364 Paula")


//*************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  paula_8364_device - constructor
//-------------------------------------------------

paula_8364_device::paula_8364_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, PAULA_8364, tag, owner, clock),
	device_sound_interface(mconfig, *this),
	m_mem_r(*this), m_int_w(*this),
	m_dmacon(0), m_adkcon(0),
	m_stream(nullptr)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void paula_8364_device::device_start()
{
	// resolve callbacks
	m_mem_r.resolve_safe(0);
	m_int_w.resolve_safe();

	// initialize channels
	for (int i = 0; i < 4; i++)
	{
		m_channel[i].index = i;
		m_channel[i].curticks = 0;
		m_channel[i].manualmode = false;
		m_channel[i].curlocation = 0;
		m_channel[i].irq_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(paula_8364_device::signal_irq), this));
	}

	// create the stream
	m_stream = machine().sound().stream_alloc(*this, 0, 4, clock() / CLOCK_DIVIDER);
}

//-------------------------------------------------
//  update - stream updater
//-------------------------------------------------

void paula_8364_device::update()
{
	m_stream->update();
}


//*************************************************************************
//  IMPLEMENTATION
//**************************************************************************

uint16_t paula_8364_device::reg_r(offs_t offset)
{
	switch (offset)
	{
	case REG_DMACONR:
		return m_dmacon;

	case REG_ADKCONR:
		return m_adkcon;
	}

	return 0xffff;
}

void paula_8364_device::reg_w(offs_t offset, uint16_t data)
{
	if (offset >= 0xa0 && offset <= 0xdf)
		m_stream->update();

	switch (offset)
	{
	case REG_DMACON:
		m_stream->update();
		m_dmacon = (data & 0x8000) ? (m_dmacon | (data & 0x021f)) : (m_dmacon & ~(data & 0x021f));  // only bits 15, 9 and 5 to 0
		break;

	case REG_ADKCON:
		m_stream->update();
		m_adkcon = (data & 0x8000) ? (m_adkcon | (data & 0x7fff)) : (m_adkcon & ~(data & 0x7fff));
		break;

	// to be moved
	case REG_AUD0LCL: m_channel[CHAN_0].loc = (m_channel[CHAN_0].loc & 0xffff0000) | ((data & 0xfffe) <<  0); break; // 15-bit
	case REG_AUD0LCH: m_channel[CHAN_0].loc = (m_channel[CHAN_0].loc & 0x0000ffff) | ((data & 0x001f) << 16); break; // 3-bit on ocs, 5-bit ecs
	case REG_AUD1LCL: m_channel[CHAN_1].loc = (m_channel[CHAN_1].loc & 0xffff0000) | ((data & 0xfffe) <<  0); break; // 15-bit
	case REG_AUD1LCH: m_channel[CHAN_1].loc = (m_channel[CHAN_1].loc & 0x0000ffff) | ((data & 0x001f) << 16); break; // 3-bit on ocs, 5-bit ecs
	case REG_AUD2LCL: m_channel[CHAN_2].loc = (m_channel[CHAN_2].loc & 0xffff0000) | ((data & 0xfffe) <<  0); break; // 15-bit
	case REG_AUD2LCH: m_channel[CHAN_2].loc = (m_channel[CHAN_2].loc & 0x0000ffff) | ((data & 0x001f) << 16); break; // 3-bit on ocs, 5-bit ecs
	case REG_AUD3LCL: m_channel[CHAN_3].loc = (m_channel[CHAN_3].loc & 0xffff0000) | ((data & 0xfffe) <<  0); break; // 15-bit
	case REG_AUD3LCH: m_channel[CHAN_3].loc = (m_channel[CHAN_3].loc & 0x0000ffff) | ((data & 0x001f) << 16); break; // 3-bit on ocs, 5-bit ecs

	// audio data
	case REG_AUD0LEN: m_channel[CHAN_0].len = data; break;
	case REG_AUD0PER: m_channel[CHAN_0].per = data; break;
	case REG_AUD0VOL: m_channel[CHAN_0].vol = data; break;
	case REG_AUD0DAT: m_channel[CHAN_0].dat = data; m_channel[CHAN_0].manualmode = true; break;
	case REG_AUD1LEN: m_channel[CHAN_1].len = data; break;
	case REG_AUD1PER: m_channel[CHAN_1].per = data; break;
	case REG_AUD1VOL: m_channel[CHAN_1].vol = data; break;
	case REG_AUD1DAT: m_channel[CHAN_1].dat = data; m_channel[CHAN_1].manualmode = true; break;
	case REG_AUD2LEN: m_channel[CHAN_2].len = data; break;
	case REG_AUD2PER: m_channel[CHAN_2].per = data; break;
	case REG_AUD2VOL: m_channel[CHAN_2].vol = data; break;
	case REG_AUD2DAT: m_channel[CHAN_2].dat = data; m_channel[CHAN_2].manualmode = true; break;
	case REG_AUD3LEN: m_channel[CHAN_3].len = data; break;
	case REG_AUD3PER: m_channel[CHAN_3].per = data; break;
	case REG_AUD3VOL: m_channel[CHAN_3].vol = data; break;
	case REG_AUD3DAT: m_channel[CHAN_3].dat = data; m_channel[CHAN_3].manualmode = true; break;
	}
}

//-------------------------------------------------
//  signal_irq - irq signaling
//-------------------------------------------------

TIMER_CALLBACK_MEMBER( paula_8364_device::signal_irq )
{
	m_int_w(param);
}

//-------------------------------------------------
//  dma_reload
//-------------------------------------------------

void paula_8364_device::dma_reload(audio_channel *chan)
{
	chan->curlocation = chan->loc;
	chan->curlength = chan->len;
	chan->irq_timer->adjust(attotime::from_hz(15750), chan->index); // clock() / 227

	LOG("dma_reload(%d): offs=%05X len=%04X\n", chan->index, chan->curlocation, chan->curlength);
}

//-------------------------------------------------
//  sound_stream_update - handle a stream update
//-------------------------------------------------

void paula_8364_device::sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples)
{
	int channum, sampoffs = 0;

	// if all DMA off, disable all channels
	if (BIT(m_dmacon, 9) == 0)
	{
		m_channel[0].dma_enabled =
		m_channel[1].dma_enabled =
		m_channel[2].dma_enabled =
		m_channel[3].dma_enabled = false;

		// clear the sample data to 0
		for (channum = 0; channum < 4; channum++)
			memset(outputs[channum], 0, sizeof(stream_sample_t) * samples);
		return;
	}

	samples *= CLOCK_DIVIDER;

	// update the DMA states on each channel and reload if fresh
	for (channum = 0; channum < 4; channum++)
	{
		audio_channel *chan = &m_channel[channum];
		if (!chan->dma_enabled && ((m_dmacon >> channum) & 1))
			dma_reload(chan);

		chan->dma_enabled = BIT(m_dmacon, channum);
	}

	// loop until done
	while (samples > 0)
	{
		int nextper, nextvol;
		int ticks = samples;

		// determine the number of ticks we can do in this chunk
		if (ticks > m_channel[0].curticks)
			ticks = m_channel[0].curticks;
		if (ticks > m_channel[1].curticks)
			ticks = m_channel[1].curticks;
		if (ticks > m_channel[2].curticks)
			ticks = m_channel[2].curticks;
		if (ticks > m_channel[3].curticks)
			ticks = m_channel[3].curticks;

		// loop over channels
		nextper = nextvol = -1;
		for (channum = 0; channum < 4; channum++)
		{
			audio_channel *chan = &m_channel[channum];
			int volume = (nextvol == -1) ? chan->vol : nextvol;
			int period = (nextper == -1) ? chan->per : nextper;
			stream_sample_t sample;
			int i;

			// normalize the volume value
			volume = (volume & 0x40) ? 64 : (volume & 0x3f);
			volume *= 4;

			// are we modulating the period of the next channel?
			if ((m_adkcon >> channum) & 0x10)
			{
				nextper = chan->dat;
				nextvol = -1;
				sample = 0;
			}

			// are we modulating the volume of the next channel?
			else if ((m_adkcon >> channum) & 0x01)
			{
				nextper = -1;
				nextvol = chan->dat;
				sample = 0;
			}

			// otherwise, we are generating data
			else
			{
				nextper = nextvol = -1;
				sample = chan->latched * volume;
			}

			// fill the buffer with the sample
			for (i = 0; i < ticks; i += CLOCK_DIVIDER)
				outputs[channum][(sampoffs + i) / CLOCK_DIVIDER] = sample;

			// account for the ticks; if we hit 0, advance
			chan->curticks -= ticks;
			if (chan->curticks == 0)
			{
				// reset the clock and ensure we're above the minimum ticks
				chan->curticks = period;
				if (chan->curticks < 124)
					chan->curticks = 124;

				// move forward one byte; if we move to an even byte, fetch new
				if (chan->dma_enabled || chan->manualmode)
					chan->curlocation++;
				if (chan->dma_enabled && !(chan->curlocation & 1))
				{
					chan->dat = m_mem_r(chan->curlocation);

					if (chan->curlength != 0)
						chan->curlength--;

					// if we run out of data, reload the dma
					if (chan->curlength == 0)
						dma_reload(chan);
				}

				// latch the next byte of the sample
				if (!(chan->curlocation & 1))
					chan->latched = chan->dat >> 8;
				else
					chan->latched = chan->dat >> 0;

				// if we're in manual mode, signal an interrupt once we latch the low byte
				if (!chan->dma_enabled && chan->manualmode && (chan->curlocation & 1))
				{
					signal_irq(nullptr, channum);
					chan->manualmode = false;
				}
			}
		}

		// bump ourselves forward by the number of ticks
		sampoffs += ticks;
		samples -= ticks;
	}
}
