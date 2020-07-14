// license: GPL-2.0+
// copyright-holders: Dirk Best
/***************************************************************************

    1 Mb Interface for SAM Coupe

***************************************************************************/

#include "emu.h"
#include "onemeg.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(SAM_ONEMEG, sam_onemeg_device, "onemeg", "1 Mb Interface")

//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

static INPUT_PORTS_START( onemeg )
	PORT_START("dip")
	PORT_DIPNAME(0x03, 0x03, "Board ID")
	PORT_DIPSETTING(0x00, "1")
	PORT_DIPSETTING(0x01, "2")
	PORT_DIPSETTING(0x02, "3")
	PORT_DIPSETTING(0x03, "4")
INPUT_PORTS_END

ioport_constructor sam_onemeg_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( onemeg );
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  sam_onemeg_device - constructor
//-------------------------------------------------

sam_onemeg_device::sam_onemeg_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, SAM_ONEMEG, tag, owner, clock),
	device_samcoupe_expansion_interface(mconfig, *this),
	m_dip(*this, "dip"),
	m_xmem(0),
	m_expage{ 0x00, 0x00 }
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void sam_onemeg_device::device_start()
{
	// init ram
	m_ram = std::make_unique<uint8_t[]>(0x100000);
	memset(m_ram.get(), 0x55, 0x100000);

	// register for savestates
	save_item(NAME(m_xmem));
	save_item(NAME(m_expage[0]));
	save_item(NAME(m_expage[1]));
	save_pointer(NAME(m_ram), 0x100000);
}


//**************************************************************************
//  IMPLEMENTATION
//**************************************************************************

void sam_onemeg_device::xmem_w(int state)
{
	m_xmem = state;
}

uint8_t sam_onemeg_device::mreq_r(offs_t offset)
{
	uint8_t data = 0xff;

	// read if external memory is enabled and the id matches
	if (m_xmem && (m_expage[BIT(offset, 14)] >> 6 == m_dip->read()))
		data = m_ram[((m_expage[BIT(offset, 14)] & 0x3f) << 14) | (offset & 0x3fff)];

	return data;
}

void sam_onemeg_device::mreq_w(offs_t offset, uint8_t data)
{
	// write if external memory is enabled and the id matches
	if (m_xmem && (m_expage[BIT(offset, 14)] >> 6 == m_dip->read()))
		m_ram[((m_expage[BIT(offset, 14)] & 0x3f) << 14) | (offset & 0x3fff)] = data;
}

void sam_onemeg_device::iorq_w(offs_t offset, uint8_t data)
{
	// 0x80: EXPAGE-C
	// 0x81: EXPAGE-D

	// 76------  selected memory expansion
	// --543210  memory bank

	if ((offset & 0xfe) == 0x80)
		m_expage[BIT(offset, 0)] = data;
}
