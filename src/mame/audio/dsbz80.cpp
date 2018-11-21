// license:BSD-3-Clause
// copyright-holders:R. Belmont, Olivier Galibert
/***************************************************************************

  Sega Z80 Digital Sound Board

  used for Model 1/2/3

***************************************************************************/


#include "emu.h"
#include "audio/dsbz80.h"
#include "machine/clock.h"

#define Z80_TAG "mpegcpu"

void dsbz80_device::dsbz80_map(address_map &map)
{
	map(0x0000, 0x7fff).rom().region(":mpegcpu", 0);
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
	map(0xf0, 0xf1).rw("uart", FUNC(i8251_device::read), FUNC(i8251_device::write));
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
	m_ourcpu(*this, Z80_TAG),
	m_uart(*this, "uart"),
	m_rxd_handler(*this)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void dsbz80_device::device_start()
{
	m_rxd_handler.resolve_safe();
	uint8_t *rom_base = machine().root_device().memregion("mpeg")->base();
	decoder = new mpeg_audio(rom_base, mpeg_audio::L2, false, 0);
	machine().sound().stream_alloc(*this, 0, 2, 32000);
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void dsbz80_device::device_reset()
{
	start = end = 0;
	audio_pos = audio_avail = 0;
	memset(audio_buf, 0, sizeof(audio_buf));
	mp_state = 0;

	m_uart->write_cts(0);
}

WRITE_LINE_MEMBER(dsbz80_device::write_txd)
{
	m_uart->write_rxd(state);
}

WRITE_LINE_MEMBER(dsbz80_device::output_txd)
{
	// not used by swa
	m_rxd_handler(state);
}

WRITE8_MEMBER(dsbz80_device::mpeg_trigger_w)
{
	mp_state = data;

	if (data == 0)  // stop
	{
		mp_state = 0;
		audio_pos = audio_avail = 0;
	}
	else if (data == 1) // play without loop
	{
		mp_pos = mp_start*8;
	}
	else if (data == 2) // play with loop
	{
		mp_pos = mp_start*8;
	}
}

READ8_MEMBER(dsbz80_device::mpeg_pos_r)
{
	int mp_prg = mp_pos >> 3;

	switch (offset)
	{
		case 0:
			return (mp_prg>>16)&0xff;
		case 1:
			return (mp_prg>>8)&0xff;
		case 2:
			return mp_prg&0xff;
	}

	return 0;
}

/* NOTE: writes to the start and end while playback is already in progress
   get latched.  When the current stream ends, the MPEG hardware starts playing
   immediately from the latched start and end position.  In this way, the Z80
   enforces looping where appropriate and multi-part songs in other cases
   (song #16 is a good example)
*/

WRITE8_MEMBER(dsbz80_device::mpeg_start_w)
{
	switch (offset)
	{
		case 0:
			start &= 0x00ffff;
			start |= (int)data<<16;
			break;
		case 1:
			start &= 0xff00ff;
			start |= (int)data<<8;
			break;
		case 2:
			start &= 0xffff00;
			start |= data;

			if (mp_state == 0)
			{
				mp_start = start;
			}
			else
			{
				lp_start = start;
				// SWA: if loop end is zero, it means "keep previous end marker"
				if (lp_end == 0)
				{
//                  MPEG_Set_Loop(ROM + lp_start, mp_end-lp_start);
				}
				else
				{
//                  MPEG_Set_Loop(ROM + lp_start, lp_end-lp_start);
				}
			}
			break;
	}
}

WRITE8_MEMBER(dsbz80_device::mpeg_end_w)
{
	switch (offset)
	{
		case 0:
			end &= 0x00ffff;
			end |= (int)data<<16;
			break;
		case 1:
			end &= 0xff00ff;
			end |= (int)data<<8;
			break;
		case 2:
			end &= 0xffff00;
			end |= data;

			if (mp_state == 0)
			{
				mp_end = end;
			}
			else
			{
				lp_end = end;
//              MPEG_Set_Loop(ROM + lp_start, lp_end-lp_start);
			}
			break;
	}
}

WRITE8_MEMBER(dsbz80_device::mpeg_volume_w)
{
	mp_vol = 0x7f - data;
}

WRITE8_MEMBER(dsbz80_device::mpeg_stereo_w)
{
	mp_pan = data & 3;  // 0 = stereo, 1 = left on both channels, 2 = right on both channels
}

void dsbz80_device::sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples)
{
	stream_sample_t *out_l = outputs[0];
	stream_sample_t *out_r = outputs[1];

	for(;;)
	{
		while(samples && audio_pos < audio_avail)
		{
			switch (mp_pan)
			{
				case 0: // stereo
					*out_l++ = audio_buf[audio_pos*2];
					*out_r++ = audio_buf[audio_pos*2+1];
					break;

				case 1: // left only
					*out_l++ = audio_buf[audio_pos*2];
					*out_r++ = audio_buf[audio_pos*2];
					break;

				case 2: // right only
					*out_l++ = audio_buf[audio_pos*2+1];
					*out_r++ = audio_buf[audio_pos*2+1];
					break;
			}
			audio_pos++;
			samples--;
		}

		if(!samples)
		{
			break;
		}

		if(mp_state == 0)
		{
			for(int i=0; i != samples; i++)
			{
				*out_l++ = 0;
				*out_r++ = 0;
			}
			break;

		}
		else
		{
			int sample_rate, channel_count;
			bool ok = decoder->decode_buffer(mp_pos, mp_end*8, audio_buf, audio_avail, sample_rate, channel_count);

			if (ok)
			{
				audio_pos = 0;
			}
			else
			{
				if(mp_state == 2)
				{
					if (mp_pos == lp_start*8)
					{
						// We're looping on un-decodable crap, abort abort abort
						mp_state = 0;
					}
					mp_pos = lp_start*8;

					if (lp_end)
					{
						mp_end = lp_end;
					}
				}
				else
				{
					mp_state = 0;
				}
			}
		}
	}
}
