// license:BSD-3-Clause
// copyright-holders:R. Belmont, Olivier Galibert
/***************************************************************************

  Sega Z80 Digital Sound Board

  used for Model 1/2/3

***************************************************************************/


#include "emu.h"
#include "dsbz80.h"
#include "machine/clock.h"

#include <algorithm>

void dsbz80_device::dsbz80_map(address_map &map)
{
	map(0x0000, 0x7fff).rom().region("mpegcpu", 0);
	map(0x8000, 0xffff).ram();
}

void dsbz80_device::dsbz80io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0xe0, 0xe0).w(FUNC(dsbz80_device::mpeg_trigger_w));
	map(0xe2, 0xe4).rw(FUNC(dsbz80_device::mpeg_pos_r), FUNC(dsbz80_device::mpeg_start_w));
	map(0xe5, 0xe7).w(FUNC(dsbz80_device::mpeg_end_w));
	map(0xe8, 0xe8).w(FUNC(dsbz80_device::mpeg_volume_w));
	map(0xe9, 0xe9).w(FUNC(dsbz80_device::mpeg_stereo_w));
	map(0xf0, 0xf1).rw(m_uart, FUNC(i8251_device::read), FUNC(i8251_device::write));
}


//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE(DSBZ80, dsbz80_device, "dsbz80_device", "Sega Z80-based Digital Sound Board")


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void dsbz80_device::device_add_mconfig(machine_config &config)
{
	Z80(config, m_ourcpu, 4000000);     // unknown clock, but probably pretty slow considering the z80 does like nothing
	m_ourcpu->set_addrmap(AS_PROGRAM, &dsbz80_device::dsbz80_map);
	m_ourcpu->set_addrmap(AS_IO, &dsbz80_device::dsbz80io_map);

	I8251(config, m_uart, 4000000);
	m_uart->rxrdy_handler().set_inputline(m_ourcpu, INPUT_LINE_IRQ0);
	m_uart->txd_handler().set(FUNC(dsbz80_device::output_txd));

	clock_device &uart_clock(CLOCK(config, "uart_clock", 500000)); // 16 times 31.25MHz (standard Sega/MIDI sound data rate)
	uart_clock.signal_handler().set("uart", FUNC(i8251_device::write_rxc));
	uart_clock.signal_handler().append("uart", FUNC(i8251_device::write_txc));
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  dsbz80_device - constructor
//-------------------------------------------------

dsbz80_device::dsbz80_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, DSBZ80, tag, owner, clock),
	device_sound_interface(mconfig, *this),
	m_ourcpu(*this, "mpegcpu"),
	m_uart(*this, "uart"),
	m_mpeg_rom(*this, "mpeg"),
	m_rxd_handler(*this),
	m_mp_start(0),
	m_mp_end(0),
	m_mp_vol(0x7f),
	m_mp_pan(0),
	m_mp_state(0),
	m_lp_start(0),
	m_lp_end(0),
	m_start(0),
	m_end(0),
	m_mp_pos(0),
	m_audio_pos(0),
	m_audio_avail(0)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void dsbz80_device::device_start()
{
	m_decoder.reset(new mpeg_audio(&m_mpeg_rom[0], mpeg_audio::L2, false, 0));
	stream_alloc(0, 2, 32000);

	save_item(NAME(m_mp_start));
	save_item(NAME(m_mp_end));
	save_item(NAME(m_mp_vol));
	save_item(NAME(m_mp_pan));
	save_item(NAME(m_mp_state));
	save_item(NAME(m_lp_start));
	save_item(NAME(m_lp_end));
	save_item(NAME(m_start));
	save_item(NAME(m_end));
	save_item(NAME(m_mp_pos));
	save_item(NAME(m_audio_pos));
	save_item(NAME(m_audio_avail));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void dsbz80_device::device_reset()
{
	m_start = m_end = 0;
	m_audio_pos = m_audio_avail = 0;
	std::fill(std::begin(m_audio_buf), std::end(m_audio_buf), 0);
	m_mp_vol = 0x7f;
	m_mp_state = 0;

	m_uart->write_cts(0);
}

//-------------------------------------------------
//  device_stop - device-specific cleanup
//-------------------------------------------------

void dsbz80_device::device_stop()
{
	m_decoder.reset();
}

void dsbz80_device::write_txd(int state)
{
	m_uart->write_rxd(state);
}

void dsbz80_device::output_txd(int state)
{
	// not used by swa
	m_rxd_handler(state);
}

void dsbz80_device::mpeg_trigger_w(uint8_t data)
{
	m_mp_state = data;

	if (data == 0)  // stop
	{
		m_mp_state = 0;
		m_audio_pos = m_audio_avail = 0;
	}
	else if (data == 1) // play without loop
	{
		m_mp_pos = m_mp_start * 8;
	}
	else if (data == 2) // play with loop
	{
		m_mp_pos = m_mp_start * 8;
	}
}

uint8_t dsbz80_device::mpeg_pos_r(offs_t offset)
{
	uint32_t const mp_prg = m_mp_pos >> 3;

	switch (offset)
	{
		case 0:
			return (mp_prg >> 16) & 0xff;
		case 1:
			return (mp_prg >> 8) & 0xff;
		case 2:
			return mp_prg & 0xff;
	}

	return 0;
}

/* NOTE: writes to the start and end while playback is already in progress
   get latched.  When the current stream ends, the MPEG hardware starts playing
   immediately from the latched start and end position.  In this way, the Z80
   enforces looping where appropriate and multi-part songs in other cases
   (song #16 is a good example)
*/

void dsbz80_device::mpeg_start_w(offs_t offset, uint8_t data)
{
	switch (offset)
	{
		case 0:
			m_start &= 0x00ffff;
			m_start |= (uint32_t)data << 16;
			break;
		case 1:
			m_start &= 0xff00ff;
			m_start |= (uint32_t)data << 8;
			break;
		case 2:
			m_start &= 0xffff00;
			m_start |= data;

			if (m_mp_state == 0)
			{
				m_mp_start = m_start;
			}
			else
			{
				m_lp_start = m_start;
				// SWA: if loop end is zero, it means "keep previous end marker"
				if (m_lp_end == 0)
				{
//                  MPEG_Set_Loop(ROM + m_lp_start, m_mp_end-m_lp_start);
				}
				else
				{
//                  MPEG_Set_Loop(ROM + m_lp_start, m_lp_end-m_lp_start);
				}
			}
			break;
	}
}

void dsbz80_device::mpeg_end_w(offs_t offset, uint8_t data)
{
	switch (offset)
	{
		case 0:
			m_end &= 0x00ffff;
			m_end |= (uint32_t)data << 16;
			break;
		case 1:
			m_end &= 0xff00ff;
			m_end |= (uint32_t)data << 8;
			break;
		case 2:
			m_end &= 0xffff00;
			m_end |= data;

			if (m_mp_state == 0)
			{
				m_mp_end = m_end;
			}
			else
			{
				m_lp_end = m_end;
//              MPEG_Set_Loop(ROM + m_lp_start, m_lp_end-m_lp_start);
			}
			break;
	}
}

void dsbz80_device::mpeg_volume_w(uint8_t data)
{
	// TODO: MSB used but unknown purpose
	m_mp_vol = ~data & 0x7f;
}

void dsbz80_device::mpeg_stereo_w(uint8_t data)
{
	m_mp_pan = data & 3;  // 0 = stereo, 1 = left on both channels, 2 = right on both channels
}

void dsbz80_device::sound_stream_update(sound_stream &stream)
{
	int samples = stream.samples();
	int sampindex = 0;
	for (;;)
	{
		while (samples && (m_audio_pos < m_audio_avail))
		{
			switch (m_mp_pan)
			{
				case 0: // stereo
					stream.put_int(0, sampindex, m_audio_buf[m_audio_pos*2] * m_mp_vol, 32768 * 128);
					stream.put_int(1, sampindex, m_audio_buf[m_audio_pos*2+1] * m_mp_vol, 32768 * 128);
					sampindex++;
					break;

				case 1: // left only
					stream.put_int(0, sampindex, m_audio_buf[m_audio_pos*2] * m_mp_vol, 32768 * 128);
					stream.put_int(1, sampindex, m_audio_buf[m_audio_pos*2] * m_mp_vol, 32768 * 128);
					sampindex++;
					break;

				case 2: // right only
					stream.put_int(0, sampindex, m_audio_buf[m_audio_pos*2+1] * m_mp_vol, 32768 * 128);
					stream.put_int(1, sampindex, m_audio_buf[m_audio_pos*2+1] * m_mp_vol, 32768 * 128);
					sampindex++;
					break;
			}
			m_audio_pos++;
			samples--;
		}

		if (!samples)
		{
			break;
		}

		if (m_mp_state == 0)
		{
			break;

		}
		else
		{
			int sample_rate, channel_count;
			bool const ok = m_decoder->decode_buffer(m_mp_pos, m_mp_end*8, m_audio_buf, m_audio_avail, sample_rate, channel_count);

			if (ok)
			{
				m_audio_pos = 0;
			}
			else
			{
				if (m_mp_state == 2)
				{
					if (m_mp_pos == m_lp_start * 8)
					{
						// We're looping on un-decodable crap, abort abort abort
						m_mp_state = 0;
					}
					m_mp_pos = m_lp_start * 8;

					if (m_lp_end)
					{
						m_mp_end = m_lp_end;
					}
				}
				else
				{
					m_mp_state = 0;
				}
			}
		}
	}
}
