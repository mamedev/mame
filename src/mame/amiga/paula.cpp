// license: BSD-3-Clause
// copyright-holders: Aaron Giles, Dirk Best, Angelo Salese
/******************************************************************************

    MOS Technology/Commodore 8364 "Paula"

    Multi-purpose chip that is part of the Amiga chipset. The name "Paula"
    is derived from "Ports, Audio, UART and Logic". It features 4-channel
    DMA driven audio, the floppy controller, a serial receiver/transmitter,
    analog inputs and contains the interrupt controller.

References:
- https://www.amigarealm.com/computing/knowledge/hardref/ch5.htm

TODO:
- Inherit FDC, serial and irq controller to here;
- Move Agnus "location" logic out of here, add AUDxDR / AUDxDSR pin logic;
- low-pass filter control thru Amiga Power LED where available, technically
  outside of Paula;
- Verify ADKCON modulation;
- Verify manual mode;
- amigaaga_flop:roadkill gameplay sets up incredibly high period (-> low pitch)
  samples (engine thrust, bumping into walls);

******************************************************************************/

#include "emu.h"
#include "paula.h"

#define LIVE_AUDIO_VIEW 0

//#define VERBOSE 1
#include "logmacro.h"
#include <cstring>


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(PAULA_8364, paula_device, "paula_8364", "MOS 8364 \"Paula\"")


//*************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  paula_device - constructor
//-------------------------------------------------

paula_device::paula_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, PAULA_8364, tag, owner, clock)
	, device_sound_interface(mconfig, *this)
	, m_chipmem_r(*this, 0)
	, m_int_w(*this)
	, m_stream(nullptr)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void paula_device::device_start()
{
	// initialize channels
	for (int i = 0; i < 4; i++)
	{
		m_channel[i].index = i;
		m_channel[i].irq_timer = timer_alloc(FUNC(paula_device::signal_irq), this);
	}

	// create the stream
	m_stream = stream_alloc(0, 4, clock() / CLOCK_DIVIDER);

	save_pointer(STRUCT_MEMBER(m_channel, loc), 4);
	save_pointer(STRUCT_MEMBER(m_channel, len), 4);
	save_pointer(STRUCT_MEMBER(m_channel, per), 4);
	save_pointer(STRUCT_MEMBER(m_channel, vol), 4);
	save_pointer(STRUCT_MEMBER(m_channel, curticks), 4);
	save_pointer(STRUCT_MEMBER(m_channel, manualmode), 4);
	save_pointer(STRUCT_MEMBER(m_channel, curlocation), 4);
	save_pointer(STRUCT_MEMBER(m_channel, curlength), 4);
	save_pointer(STRUCT_MEMBER(m_channel, dma_enabled), 4);
	save_pointer(STRUCT_MEMBER(m_channel, atper), 4);
	save_pointer(STRUCT_MEMBER(m_channel, atvol), 4);
}

void paula_device::device_reset()
{
	m_dma_master_enable = false;
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
		chan.atper = false;
		chan.atvol = false;
	}
}

//-------------------------------------------------
//  update - stream updater
//-------------------------------------------------

void paula_device::update()
{
	m_stream->update();
}


//*************************************************************************
//  IMPLEMENTATION
//**************************************************************************

template <u8 ch> void paula_device::audio_channel_map(address_map &map)
{
	// TODO: location addresses belongs to Agnus
	map(0x00, 0x01).w(FUNC(paula_device::audxlch_w<ch>));
	map(0x02, 0x03).w(FUNC(paula_device::audxlcl_w<ch>));
	map(0x04, 0x05).w(FUNC(paula_device::audxlen_w<ch>));
	map(0x06, 0x07).w(FUNC(paula_device::audxper_w<ch>));
	map(0x08, 0x09).w(FUNC(paula_device::audxvol_w<ch>));
	map(0x0a, 0x0b).w(FUNC(paula_device::audxdat_w<ch>));
}

// Instantiate channel maps
template void paula_device::audio_channel_map<0>(address_map &map);
template void paula_device::audio_channel_map<1>(address_map &map);
template void paula_device::audio_channel_map<2>(address_map &map);
template void paula_device::audio_channel_map<3>(address_map &map);

template <u8 ch> void paula_device::audxlch_w(u16 data)
{
	m_stream->update();
	// TODO: chipmem mask
	m_channel[ch].loc = (m_channel[ch].loc & 0x0000ffff) | ((data & 0x001f) << 16);
}

template <u8 ch> void paula_device::audxlcl_w(u16 data)
{
	m_stream->update();
	m_channel[ch].loc = (m_channel[ch].loc & 0xffff0000) | ((data & 0xfffe) <<  0);
}

template <u8 ch> void paula_device::audxlen_w(u16 data)
{
	m_stream->update();
	m_channel[ch].len = data;
}

template <u8 ch> void paula_device::audxper_w(u16 data)
{
	m_stream->update();
	m_channel[ch].per = data;
}

template <u8 ch> void paula_device::audxvol_w(u16 data)
{
	m_stream->update();
	m_channel[ch].vol = data & 0x7f;
}

template <u8 ch> void paula_device::audxdat_w(u16 data)
{
	m_stream->update();
	m_channel[ch].dat = data;
	m_channel[ch].manualmode = true;
}

void paula_device::dmacon_set(u16 data)
{
	m_stream->update();

	m_dma_master_enable = bool(BIT(data, 9));

	// update the DMA latches on each channel and reload if fresh
	// This holds true particularly for Ocean games (bchvolly, lostpatr, pang) and waylildr:
	// they sets a DMA length for a channel then enable DMA finally resets that length to 1
	// after a short delay loop.
	for (int channum = 0; channum < 4; channum++)
	{
		audio_channel *chan = &m_channel[channum];
		if (!chan->dma_enabled && ((data >> channum) & 1))
			dma_reload(chan, true);

		// https://eab.abime.net/showthread.php?t=109529
		// if channel DMA gets disabled while in progress then make sure to issue an irq at the
		// end of current sample
		// - gunbee, 6sense, amigames:fayoh*.lha will otherwise hang with stuck note.
		if (chan->dma_enabled && !((data >> channum) & 1) && chan->curlength)
			chan->manualmode = true;

		chan->dma_enabled = bool(BIT(data, channum));
	}
}

void paula_device::adkcon_set(u16 data)
{
	m_stream->update();

	for (int channum = 0; channum < 4; channum++)
	{
		audio_channel *chan = &m_channel[channum];

		chan->atper = bool(BIT(data, channum + 4));
		chan->atvol = bool(BIT(data, channum));
	}
}

//-------------------------------------------------
//  signal_irq - irq signaling
//-------------------------------------------------

TIMER_CALLBACK_MEMBER( paula_device::signal_irq )
{
	m_int_w(param, 1);
}

//-------------------------------------------------
//  dma_reload
//-------------------------------------------------

void paula_device::dma_reload(audio_channel *chan, bool startup)
{
	chan->curlocation = chan->loc;
	// TODO: Unconfirmed, assume max size if length is 0.
	// cfr. wrestlmn and videokid, where they pratically never get even close to buffer exhaustion.
	chan->curlength = chan->len == 0 ? 0x10000 : chan->len;
	// TODO: on startup=false irq should be delayed two cycles
	if (startup)
		chan->irq_timer->adjust(attotime::from_hz(15750), chan->index); // clock() / 227
	else
		signal_irq(chan->index);

	LOG("dma_reload(%d): offs=%06X len=%04X\n", chan->index, chan->curlocation, chan->curlength);
}

std::string paula_device::print_audio_state()
{
	std::ostringstream outbuffer;

	util::stream_format(outbuffer, "DMA master %d\n", m_dma_master_enable);
	for (auto &chan : m_channel)
	{
		util::stream_format(outbuffer, "%d DMA (%d) ADK (%d%d) REGS: %06x %04x %03x %02x %d LIVE: %06x %04x %d\n"
			, chan.index
			, chan.dma_enabled
			, chan.atper
			, chan.atvol
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

void paula_device::sound_stream_update(sound_stream &stream, std::vector<read_stream_view> const &inputs, std::vector<write_stream_view> &outputs)
{
	int channum, sampoffs = 0;

	// if all DMA off, disable all channels
	if (m_dma_master_enable == false)
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
			s16 volume = (nextvol == -1) ? chan->vol : nextvol;
			s16 period = (nextper == -1) ? chan->per : nextper;
			s16 sample;
			int i;

			// normalize the volume value
			volume = (volume & 0x40) ? 64 : (volume & 0x3f);
			volume *= 4;

			// are we modulating the period of the next channel?
			if (chan->atper)
			{
				nextper = chan->dat;
				nextvol = -1;
				sample = 0;
			}

			// are we modulating the volume of the next channel?
			else if (chan->atvol)
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
				outputs[channum].put_int_clamp((sampoffs + i) / CLOCK_DIVIDER, sample, 32768);

			// account for the ticks; if we hit 0, advance
			chan->curticks -= ticks;
			if (chan->curticks == 0)
			{
				// reset the clock and ensure we're above the minimum ticks
				chan->curticks = period;
				// TODO: 123 for PAL machines, derive formula from clock()
				if (chan->curticks < 124)
					chan->curticks = 124;

				// move forward one byte; if we move to an even byte, fetch new
				if (chan->dma_enabled || chan->manualmode)
					chan->curlocation++;
				if (chan->dma_enabled && !(chan->curlocation & 1))
				{
					chan->dat = m_chipmem_r(chan->curlocation);

					if (chan->curlength != 0)
						chan->curlength--;

					// if we run out of data, reload the dma and signal an IRQ,
					// gpmaster/asparmgp definitely expects this
					// (uses channel 3 as a sequencer, changing the start address on the fly)
					if (chan->curlength == 0)
					{
						dma_reload(chan, false);
						// silence the data pointer, avoid DC offset
						// - xenon2 sets up location=0x63298 length=0x20
						// for silencing channels on-the-fly without relying on irqs.
						// Without this the location will read at 0x632d8 (data=0x7a7d), causing annoying buzzing.
						// - Ocean games (bchvolly, batmancc) also rely on this
						chan->dat = 0; //m_chipmem_r(chan->curlocation);
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
