// license:BSD-3-Clause
// copyright-holders:R. Belmont
/***************************************************************************

  Data East Pinball BSMT2000 sound board

  used for System 3 and Whitestar pinball games and Tattoo Assassins video

***************************************************************************/


#include "emu.h"
#include "decobsmt.h"
#include "speaker.h"

void decobsmt_device::decobsmt_map(address_map &map)
{
	map(0x0000, 0x1fff).ram();
	map(0x2000, 0xffff).rom().region("soundcpu", 0x2000);
	map(0x2000, 0x2001).w(FUNC(decobsmt_device::bsmt_reset_w));
	map(0x2002, 0x2003).r(FUNC(decobsmt_device::bsmt_comms_r));
	map(0x2006, 0x2007).r(FUNC(decobsmt_device::bsmt_status_r));
	map(0x6000, 0x6000).w(FUNC(decobsmt_device::bsmt0_w));
	map(0xa000, 0xa0ff).w(FUNC(decobsmt_device::bsmt1_w));
}

void decobsmt_device::bsmt_map(address_map &map)
{
	map(0x000000, 0xffffff).rom().region("bsmt", 0);
}

void decobsmt_device::bsmt_ready_callback()
{
	m_ourcpu->set_input_line(M6809_IRQ_LINE, ASSERT_LINE); // BSMT is ready
}

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE(DECOBSMT, decobsmt_device, "decobsmt", "Data East/Sega/Stern BSMT2000 Sound Board")


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void decobsmt_device::device_add_mconfig(machine_config &config)
{
	MC6809E(config, m_ourcpu, XTAL(24'000'000) / 12); // 68B09E U6 (E & Q = 2 MHz according to manual)
	m_ourcpu->set_addrmap(AS_PROGRAM, &decobsmt_device::decobsmt_map);
	m_ourcpu->set_periodic_int(FUNC(decobsmt_device::decobsmt_firq_interrupt), attotime::from_hz(489)); // Fixed FIRQ of 489Hz as measured on real (pinball) machine

	BSMT2000(config, m_bsmt, XTAL(24'000'000));
	m_bsmt->set_addrmap(0, &decobsmt_device::bsmt_map);
	m_bsmt->set_ready_callback(FUNC(decobsmt_device::bsmt_ready_callback));
	m_bsmt->add_route(0, *this, 1.0, 0);
	m_bsmt->add_route(1, *this, 1.0, 1);
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  decobsmt_device - constructor
//-------------------------------------------------

decobsmt_device::decobsmt_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, DECOBSMT, tag, owner, clock)
	, device_mixer_interface(mconfig, *this)
	, m_ourcpu(*this, "soundcpu")
	, m_bsmt(*this, "bsmt")
	, m_bsmt_latch(0)
	, m_bsmt_reset(0)
	, m_bsmt_comms(0)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void decobsmt_device::device_start()
{
	save_item(NAME(m_bsmt_latch));
	save_item(NAME(m_bsmt_reset));
	save_item(NAME(m_bsmt_comms));
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

void decobsmt_device::bsmt_reset_w(u8 data)
{
	uint8_t const diff = data ^ m_bsmt_reset;
	m_bsmt_reset = data;
	if ((diff & 0x80) && !(data & 0x80))
		m_bsmt->reset();
}

void decobsmt_device::bsmt0_w(u8 data)
{
	m_bsmt_latch = data;
}

void decobsmt_device::bsmt1_w(offs_t offset, u8 data)
{
	m_bsmt->write_reg(offset ^ 0xff);
	m_bsmt->write_data((m_bsmt_latch << 8) | data);
	m_ourcpu->set_input_line(M6809_IRQ_LINE, CLEAR_LINE); // BSMT is not ready
}

u8 decobsmt_device::bsmt_status_r()
{
	return m_bsmt->read_status() << 7;
}

u8 decobsmt_device::bsmt_comms_r()
{
	return m_bsmt_comms;
}

void decobsmt_device::bsmt_comms_w(u8 data)
{
	m_bsmt_comms = data;
}

void decobsmt_device::bsmt_reset_line(int state)
{
	m_ourcpu->set_input_line(INPUT_LINE_RESET, state);
}

INTERRUPT_GEN_MEMBER(decobsmt_device::decobsmt_firq_interrupt)
{
	device.execute().set_input_line(M6809_FIRQ_LINE, HOLD_LINE);
}
