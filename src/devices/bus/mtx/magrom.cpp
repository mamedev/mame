// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Martin Allcorn's Games ROM

**********************************************************************/


#include "emu.h"
#include "magrom.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(MTX_MAGROM, mtx_magrom_device, "mtx_magrom", "MTX MAGROM")


//-------------------------------------------------
//  INPUT_PORTS( magrom )
//-------------------------------------------------

INPUT_PORTS_START(magrom)
	PORT_START("JUMPERS")
	PORT_CONFNAME(0x03, 0x01, "System")
	PORT_CONFSETTING(0x00, "MTX500")
	PORT_CONFSETTING(0x01, "MTX512")
	PORT_CONFSETTING(0x03, "RS128")
	PORT_CONFNAME(0x04, 0x00, "ROM ID")
	PORT_CONFSETTING(0x00, "GROM")
	PORT_CONFSETTING(0x04, "ROM 6")
INPUT_PORTS_END


//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor mtx_magrom_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(magrom);
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  mtx_magrom_device - constructor
//-------------------------------------------------

mtx_magrom_device::mtx_magrom_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, MTX_MAGROM, tag, owner, clock)
	, device_mtx_exp_interface(mconfig, *this)
	, m_bank(*this, "bank")
	, m_jumpers(*this, "JUMPERS")
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void mtx_magrom_device::device_start()
{
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void mtx_magrom_device::device_reset()
{
	m_bank->configure_entries(0, 32, get_rom_base(), 0x4000);

	int rom_id = (m_jumpers->read() & 0x04) ? 6 : 7;
	machine().root_device().membank("rommap_bank1")->configure_entry(rom_id, get_rom_base() + 0x2000);

	io_space().install_write_handler(0xfb, 0xfb, write8smo_delegate(*this, FUNC(mtx_magrom_device::page_w)));
}


//**************************************************************************
//  IMPLEMENTATION
//**************************************************************************

void mtx_magrom_device::bankswitch(uint8_t data)
{
	int page = m_jumpers->read() & 0x03;
	if (!BIT(data, 7) && BIT(data, 0, 4) == page)
		program_space().install_read_bank(0x4000, 0x7fff, m_bank);
}


void mtx_magrom_device::page_w(uint8_t data)
{
	m_bank->set_entry(data & 0x1f);
}
