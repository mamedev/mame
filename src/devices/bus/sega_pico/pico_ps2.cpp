// license:BSD-3-Clause
// copyright-holders:QUFB
/**********************************************************************

    Sega Pico PS/2 slot emulation

**********************************************************************/

#include "emu.h"

#include "pico_ps2.h"

//**************************************************************************
//  CARD INTERFACE
//**************************************************************************

device_pico_ps2_slot_interface::device_pico_ps2_slot_interface(const machine_config &mconfig, device_t &device) :
	device_interface(device, "pico_ps2")
{
}

device_pico_ps2_slot_interface::~device_pico_ps2_slot_interface()
{
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

DEFINE_DEVICE_TYPE(PICO_PS2_SLOT, pico_ps2_slot_device, "pico_ps2_slot", "Sega Pico PS/2 slot")

static INPUT_PORTS_START( pico_ps2_slot )
	PORT_START("PS2_CONFIG")
	PORT_CONFNAME( 0x80, 0x00, "PS/2 Connection" )
	PORT_CONFSETTING( 0x00, DEF_STR( On ) )
	PORT_CONFSETTING( 0x80, DEF_STR( Off ) )
INPUT_PORTS_END

ioport_constructor pico_ps2_slot_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( pico_ps2_slot );
}

pico_ps2_slot_device::pico_ps2_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, PICO_PS2_SLOT, tag, owner, clock)
	, device_single_card_slot_interface<device_pico_ps2_slot_interface>(mconfig, *this)
	, m_io_ps2_config(*this, "PS2_CONFIG")
	, m_device(nullptr)
{
}

pico_ps2_slot_device::~pico_ps2_slot_device()
{
}

void pico_ps2_slot_device::device_start()
{
	m_device = get_card_device();
}

uint8_t pico_ps2_slot_device::read(offs_t offset)
{
	uint8_t data = 0xff;
	if ((m_io_ps2_config->read() == 0) && m_device)
	{
		data = m_device->ps2_r(offset);
	}
	return data;
}

void pico_ps2_slot_device::write(offs_t offset, uint8_t data)
{
	if ((m_io_ps2_config->read() == 0) && m_device)
	{
		m_device->ps2_w(offset, data);
	}
}

bool pico_ps2_slot_device::is_readable(uint8_t offset)
{
	return m_device != nullptr;
}

bool pico_ps2_slot_device::is_writeable(uint8_t offset)
{
	return m_device != nullptr;
}
