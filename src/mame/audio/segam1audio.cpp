// license:BSD-3-Clause
// copyright-holders:R. Belmont
/***************************************************************************

  Sega Model 1 sound board (68000 + 2x 315-5560 "MultiPCM")

  used for Model 1 and early Model 2 games

***************************************************************************/

#include "audio/segam1audio.h"

#define M68000_TAG      "m1sndcpu"
#define MULTIPCM_1_TAG  "m1pcm1"
#define MULTIPCM_2_TAG  "m1pcm2"
#define YM3438_TAG      "m1ym"

static ADDRESS_MAP_START( segam1audio_map, AS_PROGRAM, 16, segam1audio_device )
		AM_RANGE(0x000000, 0x03ffff) AM_ROM AM_REGION(":m1sndcpu", 0)
		AM_RANGE(0x080000, 0x09ffff) AM_ROM AM_REGION(":m1sndcpu", 0x20000) // mirror of upper ROM socket
	AM_RANGE(0xc20000, 0xc20001) AM_READWRITE(m1_snd_68k_latch_r, m1_snd_68k_latch1_w )
	AM_RANGE(0xc20002, 0xc20003) AM_READWRITE(m1_snd_v60_ready_r, m1_snd_68k_latch2_w )
	AM_RANGE(0xc40000, 0xc40007) AM_DEVREADWRITE8(MULTIPCM_1_TAG, multipcm_device, read, write, 0x00ff )
	AM_RANGE(0xc40012, 0xc40013) AM_WRITENOP
	AM_RANGE(0xc50000, 0xc50001) AM_WRITE(m1_snd_mpcm_bnk1_w )
	AM_RANGE(0xc60000, 0xc60007) AM_DEVREADWRITE8(MULTIPCM_2_TAG, multipcm_device, read, write, 0x00ff )
	AM_RANGE(0xc70000, 0xc70001) AM_WRITE(m1_snd_mpcm_bnk2_w )
	AM_RANGE(0xd00000, 0xd00007) AM_DEVREADWRITE8(YM3438_TAG, ym3438_device, read, write, 0x00ff )
	AM_RANGE(0xf00000, 0xf0ffff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( mpcm1_map, AS_0, 8, segam1audio_device )
	AM_RANGE(0x000000, 0x3fffff) AM_ROM AM_REGION(":m1pcm1", 0)
ADDRESS_MAP_END

static ADDRESS_MAP_START( mpcm2_map, AS_0, 8, segam1audio_device )
	AM_RANGE(0x000000, 0x3fffff) AM_ROM AM_REGION(":m1pcm2", 0)
ADDRESS_MAP_END

MACHINE_CONFIG_FRAGMENT( segam1audio )
	MCFG_CPU_ADD(M68000_TAG, M68000, 10000000)  // verified on real h/w
	MCFG_CPU_PROGRAM_MAP(segam1audio_map)

	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MCFG_SOUND_ADD(YM3438_TAG, YM3438, 8000000)
	MCFG_SOUND_ROUTE(0, "lspeaker", 0.60)
	MCFG_SOUND_ROUTE(1, "rspeaker", 0.60)

	MCFG_SOUND_ADD(MULTIPCM_1_TAG, MULTIPCM, 8000000)
	MCFG_DEVICE_ADDRESS_MAP(AS_0, mpcm1_map)
	MCFG_SOUND_ROUTE(0, "lspeaker", 1.0)
	MCFG_SOUND_ROUTE(1, "rspeaker", 1.0)

	MCFG_SOUND_ADD(MULTIPCM_2_TAG, MULTIPCM, 8000000)
	MCFG_DEVICE_ADDRESS_MAP(AS_0, mpcm2_map)
	MCFG_SOUND_ROUTE(0, "lspeaker", 1.0)
	MCFG_SOUND_ROUTE(1, "rspeaker", 1.0)
MACHINE_CONFIG_END

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

const device_type SEGAM1AUDIO = &device_creator<segam1audio_device>;

//-------------------------------------------------
//  machine_config_additions - device-specific
//  machine configurations
//-------------------------------------------------

machine_config_constructor segam1audio_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( segam1audio );
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  segam1audio_device - constructor
//-------------------------------------------------

segam1audio_device::segam1audio_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, SEGAM1AUDIO, "Sega Model 1 Sound Board", tag, owner, clock, "segam1audio", __FILE__),
	m_audiocpu(*this, M68000_TAG),
	m_multipcm_1(*this, MULTIPCM_1_TAG),
	m_multipcm_2(*this, MULTIPCM_2_TAG),
	m_ym(*this, YM3438_TAG),
	m_main_irq_cb(*this)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void segam1audio_device::device_start()
{
	m_main_irq_cb.resolve_safe();
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void segam1audio_device::device_reset()
{
	// init the sound FIFO
	m_fifo_rptr = m_fifo_wptr = 0;
	memset(m_to_68k, 0, sizeof(m_to_68k));
}

READ16_MEMBER(segam1audio_device::m1_snd_68k_latch_r)
{
	UINT16 retval;

	retval = m_to_68k[m_fifo_rptr];

	m_fifo_rptr++;
	if (m_fifo_rptr >= ARRAY_LENGTH(m_to_68k)) m_fifo_rptr = 0;

	return retval;
}

READ16_MEMBER(segam1audio_device::m1_snd_v60_ready_r)
{
	return 1;
}

WRITE16_MEMBER(segam1audio_device::m1_snd_mpcm_bnk1_w)
{
	m_multipcm_1->set_bank(0x100000 * (data & 3), 0x100000 * (data & 3));
}

WRITE16_MEMBER(segam1audio_device::m1_snd_mpcm_bnk2_w)
{
	m_multipcm_2->set_bank(0x100000 * (data & 3), 0x100000 * (data & 3));
}

WRITE16_MEMBER(segam1audio_device::m1_snd_68k_latch1_w)
{
}

WRITE16_MEMBER(segam1audio_device::m1_snd_68k_latch2_w)
{
}

READ16_MEMBER(segam1audio_device::ready_r)
{
	int sr = m_audiocpu->state_int(M68K_SR);

	if ((sr & 0x0700) > 0x0100)
	{
		return 0;
	}

	return 0xff;
}

void segam1audio_device::check_fifo_irq()
{
	// if the FIFO has something in it, signal the 68k
	if (m_fifo_rptr != m_fifo_wptr)
	{
		m_audiocpu->set_input_line(2, HOLD_LINE);
	}
}

void segam1audio_device::write_fifo(UINT8 data)
{
	m_to_68k[m_fifo_wptr] = data;
	m_fifo_wptr++;
	if (m_fifo_wptr >= ARRAY_LENGTH(m_to_68k)) m_fifo_wptr = 0;

	// signal the 68000 that there's data waiting
	m_audiocpu->set_input_line(2, HOLD_LINE);
}
