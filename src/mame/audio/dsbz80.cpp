// license:BSD-3-Clause
// copyright-holders:R. Belmont, Olivier Galibert
/***************************************************************************

  Sega Z80 Digital Sound Board

  used for Model 1/2/3

***************************************************************************/


#include "emu.h"
#include "audio/dsbz80.h"

#define Z80_TAG "mpegcpu"

static ADDRESS_MAP_START( dsbz80_map, AS_PROGRAM, 8, dsbz80_device )
	AM_RANGE(0x0000, 0x7fff) AM_ROM AM_REGION(":mpegcpu", 0)
	AM_RANGE(0x8000, 0xffff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( dsbz80io_map, AS_IO, 8, dsbz80_device )
	AM_RANGE(0xe0, 0xe0) AM_MIRROR(0xff00) AM_WRITE(mpeg_trigger_w)
	AM_RANGE(0xe2, 0xe4) AM_MIRROR(0xff00) AM_READWRITE(mpeg_pos_r, mpeg_start_w)
	AM_RANGE(0xe5, 0xe7) AM_MIRROR(0xff00) AM_WRITE(mpeg_end_w)
	AM_RANGE(0xe8, 0xe8) AM_MIRROR(0xff00) AM_WRITE(mpeg_volume_w)
	AM_RANGE(0xe9, 0xe9) AM_MIRROR(0xff00) AM_WRITE(mpeg_stereo_w)
	AM_RANGE(0xf0, 0xf0) AM_MIRROR(0xff00) AM_READ(latch_r)
	AM_RANGE(0xf1, 0xf1) AM_MIRROR(0xff00) AM_READ(status_r)
ADDRESS_MAP_END


MACHINE_CONFIG_FRAGMENT( dsbz80 )
	MCFG_CPU_ADD(Z80_TAG, Z80, 4000000)     /* unknown clock, but probably pretty slow considering the z80 does like nothing */
	MCFG_CPU_PROGRAM_MAP(dsbz80_map)
	MCFG_CPU_IO_MAP(dsbz80io_map)
MACHINE_CONFIG_END

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

const device_type DSBZ80 = &device_creator<dsbz80_device>;


//-------------------------------------------------
//  machine_config_additions - device-specific
//  machine configurations
//-------------------------------------------------

machine_config_constructor dsbz80_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( dsbz80 );
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  dsbz80_device - constructor
//-------------------------------------------------

dsbz80_device::dsbz80_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, DSBZ80, "Sega Z80-based Digital Sound Board", tag, owner, clock, "dsbz80", __FILE__),
	device_sound_interface(mconfig, *this),
	m_ourcpu(*this, Z80_TAG)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void dsbz80_device::device_start()
{
	UINT8 *rom_base = machine().root_device().memregion("mpeg")->base();
	decoder = new mpeg_audio(rom_base, mpeg_audio::L2, false, 0);
	machine().sound().stream_alloc(*this, 0, 2, 32000);
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void dsbz80_device::device_reset()
{
	m_dsb_latch = 0;
	status = 1;
	start = end = 0;
	audio_pos = audio_avail = 0;
	memset(audio_buf, 0, sizeof(audio_buf));
	mp_state = 0;
}

WRITE8_MEMBER(dsbz80_device::latch_w)
{
	m_ourcpu->set_input_line(INPUT_LINE_IRQ0, ASSERT_LINE);
	m_dsb_latch = data;
	status |= 2;
//  printf("%02x to DSB latch\n", data);
}

READ8_MEMBER(dsbz80_device::latch_r)
{
	m_ourcpu->set_input_line(INPUT_LINE_IRQ0, CLEAR_LINE);
//  printf("DSB Z80 read %02x\n", m_dsb_latch);
	status &= ~2;
	return m_dsb_latch;
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

READ8_MEMBER(dsbz80_device::status_r)
{
	// bit 0 = ??? (must be 1 for most games)
	// bit 1 = command is pending (used by SWA instead of IRQ)
	// other bits = ???
	// SWA requires that status & 0x38 = 0 or else it loops endlessly...
	return status;
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
