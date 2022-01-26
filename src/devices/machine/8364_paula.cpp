// license: BSD-3-Clause
// copyright-holders: Aaron Giles, Dirk Best
/***************************************************************************

    MOS Technology 8364 "Paula"

    TODO:
    - Inherit FDC, serial and irq controller to here;
    - Move Agnus "location" logic from here;
    - low-pass filter;
    - convert volume values to non-linear dB scale (cfr. )
    - Verify ADKCON modulation;
    - Verify manual mode;
    - When a DMA stop occurs, is the correlated channel playback stopped
      at the end of the current cycle or as soon as possible like current
      implementation?

***************************************************************************/

#include "emu.h"
#include "8364_paula.h"

#define LIVE_AUDIO_VIEW 0

//#define VERBOSE 1
#include "logmacro.h"
#include <cstring>


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(PAULA_8364, paula_8364_device, "paula_8364", "MOS 8364 \"Paula\"")


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
		m_channel[i].irq_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(paula_8364_device::signal_irq), this));
	}

	// create the stream
	m_stream = stream_alloc(0, 4, clock() / CLOCK_DIVIDER);
}

void paula_8364_device::device_reset()
{
	m_dmacon = 0;
	m_adkcon = 0;
	for (auto &chan : m_channel)
	{
		chan.loc = 0;
		chan.len = 0;
		chan.per = 0;
		chan.vol = 0;
		chan.curticks = 0;
		chan.manualmode = false;
		chan.curlocation = 0;
		chan.curlength = 0;
		chan.dma_enabled = false;
	}
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
		// update the DMA latches on each channel and reload if fresh
		// This holds true particularly for Ocean games (bchvolly, lostpatr, pang) and waylildr:
		// they sets a DMA length for a channel then enable DMA then resets that length to 1
		// after a short delay loop.
		for (int channum = 0; channum < 4; channum++)
		{
			audio_channel *chan = &m_channel[channum];
			if (!chan->dma_enabled && ((m_dmacon >> channum) & 1))
				dma_reload(chan, true);

			chan->dma_enabled = BIT(m_dmacon, channum);
		}
		break;

	case REG_ADKCON:
		m_stream->update();
		m_adkcon = (data & 0x8000) ? (m_adkcon | (data & 0x7fff)) : (m_adkcon & ~(data & 0x7fff));
		break;

	// FIXME: location belongs to Agnus
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
	case REG_AUD0VOL: m_channel[CHAN_0].vol = data & 0x7f; break;
	case REG_AUD0DAT: m_channel[CHAN_0].dat = data; m_channel[CHAN_0].manualmode = true; break;
	case REG_AUD1LEN: m_channel[CHAN_1].len = data; break;
	case REG_AUD1PER: m_channel[CHAN_1].per = data; break;
	case REG_AUD1VOL: m_channel[CHAN_1].vol = data & 0x7f; break;
	case REG_AUD1DAT: m_channel[CHAN_1].dat = data; m_channel[CHAN_1].manualmode = true; break;
	case REG_AUD2LEN: m_channel[CHAN_2].len = data; break;
	case REG_AUD2PER: m_channel[CHAN_2].per = data; break;
	case REG_AUD2VOL: m_channel[CHAN_2].vol = data & 0x7f; break;
	case REG_AUD2DAT: m_channel[CHAN_2].dat = data; m_channel[CHAN_2].manualmode = true; break;
	case REG_AUD3LEN: m_channel[CHAN_3].len = data; break;
	case REG_AUD3PER: m_channel[CHAN_3].per = data; break;
	case REG_AUD3VOL: m_channel[CHAN_3].vol = data & 0x7f; break;
	case REG_AUD3DAT: m_channel[CHAN_3].dat = data; m_channel[CHAN_3].manualmode = true; break;
	}
}

//-------------------------------------------------
//  signal_irq - irq signaling
//-------------------------------------------------

TIMER_CALLBACK_MEMBER( paula_8364_device::signal_irq )
{
	m_int_w(param, 1);
}

//-------------------------------------------------
//  dma_reload
//-------------------------------------------------

void paula_8364_device::dma_reload(audio_channel *chan, bool startup)
{
	chan->curlocation = chan->loc;
	// TODO: how to treat length == 0?
	chan->curlength = chan->len;
	// TODO: on startup=false irq should be delayed two cycles
	if (startup)
		chan->irq_timer->adjust(attotime::from_hz(15750), chan->index); // clock() / 227
	else
		signal_irq(chan->index);

	LOG("dma_reload(%d): offs=%06X len=%04X\n", chan->index, chan->curlocation, chan->curlength);
}

std::string paula_8364_device::print_audio_state()
{
	std::ostringstream outbuffer;

	util::stream_format(outbuffer, "DMACON: %04x (%d) ADKCON %04x\n", m_dmacon, BIT(m_dmacon, 9), m_adkcon);
	for (auto &chan : m_channel)
	{
		util::stream_format(outbuffer, "%d (%d) (%d%d) REGS: %06x %04x %03x %02x %d LIVE: %06x %04x %d\n"
			, chan.index
			, BIT(m_dmacon, chan.index)
			, BIT(m_adkcon, chan.index+4)
			, BIT(m_adkcon, chan.index)
			, chan.loc
			, chan.len
			, chan.per
			, chan.vol
			, chan.manualmode
			, chan.curlocation
			, chan.curlength
			, chan.dma_enabled
		);
	}

	return outbuffer.str();
}

//-------------------------------------------------
//  sound_stream_update - handle a stream update
//-------------------------------------------------

void paula_8364_device::sound_stream_update(sound_stream &stream, std::vector<read_stream_view> const &inputs, std::vector<write_stream_view> &outputs)
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
			outputs[channum].fill(0);
		return;
	}

	int samples = outputs[0].samples() * CLOCK_DIVIDER;

	if (LIVE_AUDIO_VIEW)
		popmessage(print_audio_state());

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
			s32 sample;
			int i;

			// normalize the volume value
			// FIXME: definitely not linear
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
				outputs[channum].put_int((sampoffs + i) / CLOCK_DIVIDER, sample, 32768);

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

					// if we run out of data, reload the dma and signal an IRQ,
					// gpmaster/asparmgp definitely expects this
					// (uses channel 3 as a sequencer, changing the start address on the fly)
					if (chan->curlength == 0)
					{
						dma_reload(chan, false);
						// reload the data pointer, otherwise aliasing / buzzing outside the given buffer will be heard
						// For example: Xenon 2 sets up location=0x63298 length=0x20
						// for silencing channels on-the-fly without relying on irqs.
						// Without this the location will read at 0x632d8 (data=0x7a7d), causing annoying buzzing.
						chan->dat = m_mem_r(chan->curlocation);
					}
				}

				// latch the next byte of the sample
				if (!(chan->curlocation & 1))
					chan->latched = chan->dat >> 8;
				else
					chan->latched = chan->dat >> 0;

				// if we're in manual mode, signal an interrupt once we latch the low byte
				if (!chan->dma_enabled && chan->manualmode && (chan->curlocation & 1))
				{
					signal_irq(channum);
					chan->manualmode = false;
				}
			}
		}

		// bump ourselves forward by the number of ticks
		sampoffs += ticks;
		samples -= ticks;
	}
}
