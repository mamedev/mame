// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/***************************************************************************

    MR8000 Master RAM Cartridge emulation

***************************************************************************/

#include "emu.h"
#include "mr8000.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(BBC_MR8000, bbc_mr8000_device, "bbc_mr8000", "MR8000 Master RAM Cartridge")


//-------------------------------------------------
//  INPUT_PORTS( mr8000 )
//-------------------------------------------------

INPUT_PORTS_START(mr8000)
	PORT_START("SWITCH")
	PORT_CONFNAME(0x03, 0x00, "ROM Choice")
	PORT_CONFSETTING(0x00, "A (lower banks)")
	PORT_CONFSETTING(0x01, "B (upper banks)")
	PORT_CONFSETTING(0x02, "Off (disabled)")

	PORT_CONFNAME(0x04, 0x00, "Write Protect")
	PORT_CONFSETTING(0x00, "On (read only")
	PORT_CONFSETTING(0x04, "Off (read and write)")
INPUT_PORTS_END


//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor bbc_mr8000_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(mr8000);
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  bbc_mr8000_device - constructor
//-------------------------------------------------

bbc_mr8000_device::bbc_mr8000_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, BBC_MR8000, tag, owner, clock)
	, device_bbc_cart_interface(mconfig, *this)
	, m_switch(*this, "SWITCH")
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void bbc_mr8000_device::device_start()
{
}


//-------------------------------------------------
//  read - cartridge data read
//-------------------------------------------------

uint8_t bbc_mr8000_device::read(offs_t offset, int infc, int infd, int romqa, int oe, int oe2)
{
	uint8_t data = 0x00;
	int bank = BIT(m_switch->read(), 0);

	if (oe && !BIT(m_switch->read(), 1))
	{
		data = m_nvram[offset | (bank << 15) | (romqa << 14)];
	}

	return data;
}

//-------------------------------------------------
//  write - cartridge data write
//-------------------------------------------------

void bbc_mr8000_device::write(offs_t offset, uint8_t data, int infc, int infd, int romqa, int oe, int oe2)
{
	int bank = BIT(m_switch->read(), 0);

	if (oe && !BIT(m_switch->read(), 1))
	{
		if (BIT(m_switch->read(), 2))
		{
			m_nvram[offset | (bank << 15) | (romqa << 14)] = data;
		}
	}
}
