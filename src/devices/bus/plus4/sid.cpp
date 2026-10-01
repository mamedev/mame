// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Commodore Plus/4 SID cartridge emulation

    https://plus4world.powweb.com/hardware/Solders_SID_Card

**********************************************************************/

#include "emu.h"
#include "sid.h"

#include "speaker.h"



//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define MOS8580_TAG     "mos8580"
#define CONTROL1_TAG    "joy1"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(PLUS4_SID, plus4_sid_cartridge_device, "plus4_sid", "Plus/4 SID cartridge")


//-------------------------------------------------
//  ROM( plus4_sid )
//-------------------------------------------------

ROM_START( plus4_sid )
	ROM_REGION( 0x100, "pld", 0 )
	ROM_LOAD( "gal16v8", 0x000, 0x100, NO_DUMP )
ROM_END


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *plus4_sid_cartridge_device::device_rom_region() const
{
	return ROM_NAME( plus4_sid );
}


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void plus4_sid_cartridge_device::device_add_mconfig(machine_config &config)
{
	SPEAKER(config, "speaker").front_center();
	MOS8580(config, m_sid, clock()).add_route(ALL_OUTPUTS, "speaker", 1.0);

	VCS_CONTROL_PORT(config, m_joy, vcs_control_port_devices, nullptr);
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  plus4_sid_cartridge_device - constructor
//-------------------------------------------------

plus4_sid_cartridge_device::plus4_sid_cartridge_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, PLUS4_SID, tag, owner, clock),
	device_plus4_expansion_card_interface(mconfig, *this),
	m_sid(*this, MOS8580_TAG),
	m_joy(*this, CONTROL1_TAG)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void plus4_sid_cartridge_device::device_start()
{
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void plus4_sid_cartridge_device::device_reset()
{
	m_sid->reset();

	m_slot->io().install_readwrite_handler(0x040, 0x05f, emu::rw_delegate(*m_sid, FUNC(mos6581_device::read)), emu::rw_delegate(*m_sid, FUNC(mos6581_device::write)));
	m_slot->io().install_readwrite_handler(0x180, 0x19f, emu::rw_delegate(*m_sid, FUNC(mos6581_device::read)), emu::rw_delegate(*m_sid, FUNC(mos6581_device::write)));
	m_slot->io().install_read_handler(0x080, 0x08f, emu::rw_delegate(*this, FUNC(plus4_sid_cartridge_device::joy_r)));
}


//-------------------------------------------------
//  joy_r - joystick read
//-------------------------------------------------

uint8_t plus4_sid_cartridge_device::joy_r()
{
	return m_joy->read_joy();
}
