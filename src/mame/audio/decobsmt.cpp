// license:BSD-3-Clause
// copyright-holders:R. Belmont
/***************************************************************************

  Data East Pinball BSMT2000 sound board

  used for System 3 and Whitestar pinball games and Tattoo Assassins video

***************************************************************************/


#include "emu.h"
#include "audio/decobsmt.h"
#include "speaker.h"

#define M6809_TAG   "soundcpu"
#define BSMT_TAG    "bsmt"

ADDRESS_MAP_START(decobsmt_device::decobsmt_map)
	AM_RANGE(0x0000, 0x1fff) AM_RAM
	AM_RANGE(0x2000, 0xffff) AM_ROM AM_REGION(":soundcpu", 0x2000)
	AM_RANGE(0x2000, 0x2001) AM_WRITE(bsmt_reset_w)
	AM_RANGE(0x2002, 0x2003) AM_READ(bsmt_comms_r)
	AM_RANGE(0x2006, 0x2007) AM_READ(bsmt_status_r)
	AM_RANGE(0x6000, 0x6000) AM_WRITE(bsmt0_w)
	AM_RANGE(0xa000, 0xa0ff) AM_WRITE(bsmt1_w)
ADDRESS_MAP_END

ADDRESS_MAP_START(decobsmt_device::bsmt_map)
	AM_RANGE(0x000000, 0xffffff) AM_ROM AM_REGION(":bsmt", 0)
ADDRESS_MAP_END

void decobsmt_device::bsmt_ready_callback()
{
	m_ourcpu->set_input_line(M6809_IRQ_LINE, ASSERT_LINE); /* BSMT is ready */
}

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE(DECOBSMT, decobsmt_device, "decobsmt", "Data East/Sega/Stern BSMT2000 Sound Board")


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

MACHINE_CONFIG_START(decobsmt_device::device_add_mconfig)
	MCFG_CPU_ADD(M6809_TAG, MC6809E, XTAL(24'000'000) / 12) // 68B09E U6 (E & Q = 2 MHz according to manual)
	MCFG_CPU_PROGRAM_MAP(decobsmt_map)
	MCFG_CPU_PERIODIC_INT_DRIVER(decobsmt_device, decobsmt_firq_interrupt, 489) /* Fixed FIRQ of 489Hz as measured on real (pinball) machine */

	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")
	MCFG_BSMT2000_ADD(BSMT_TAG, XTAL(24'000'000))
	MCFG_DEVICE_ADDRESS_MAP(0, bsmt_map)
	MCFG_BSMT2000_READY_CALLBACK(decobsmt_device, bsmt_ready_callback)
	MCFG_SOUND_ROUTE(0, "lspeaker", 2.0)
	MCFG_SOUND_ROUTE(1, "rspeaker", 2.0)
MACHINE_CONFIG_END

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  decobsmt_device - constructor
//-------------------------------------------------

decobsmt_device::decobsmt_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, DECOBSMT, tag, owner, clock)
	, m_ourcpu(*this, M6809_TAG)
	, m_bsmt(*this, BSMT_TAG)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void decobsmt_device::device_start()
{
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void decobsmt_device::device_reset()
{
	m_bsmt_latch = 0;
	m_bsmt_reset = 0;
	m_bsmt_comms = 0;
}

WRITE8_MEMBER(decobsmt_device::bsmt_reset_w)
{
	uint8_t diff = data ^ m_bsmt_reset;
	m_bsmt_reset = data;
	if ((diff & 0x80) && !(data & 0x80))
		m_bsmt->reset();
}

WRITE8_MEMBER(decobsmt_device::bsmt0_w)
{
	m_bsmt_latch = data;
}

WRITE8_MEMBER(decobsmt_device::bsmt1_w)
{
	m_bsmt->write_reg(offset ^ 0xff);
	m_bsmt->write_data((m_bsmt_latch << 8) | data);
	m_ourcpu->set_input_line(M6809_IRQ_LINE, CLEAR_LINE); /* BSMT is not ready */
}

READ8_MEMBER(decobsmt_device::bsmt_status_r)
{
	return m_bsmt->read_status() << 7;
}

READ8_MEMBER(decobsmt_device::bsmt_comms_r)
{
	return m_bsmt_comms;
}

WRITE8_MEMBER(decobsmt_device::bsmt_comms_w)
{
	m_bsmt_comms = data;
}

WRITE_LINE_MEMBER(decobsmt_device::bsmt_reset_line)
{
	m_ourcpu->set_input_line(INPUT_LINE_RESET, state);
}

INTERRUPT_GEN_MEMBER(decobsmt_device::decobsmt_firq_interrupt)
{
	device.execute().set_input_line(M6809_FIRQ_LINE, HOLD_LINE);
}
