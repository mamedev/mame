// license:GPL-2.0+
// copyright-holders:Dirk Best
/***************************************************************************

    SV-807 64k memory expansion for SVI-318/328

    TODO:
    - Switch S6 (but needs to be off for the SVI anyway)

***************************************************************************/

#include "sv807.h"


//**************************************************************************
//  CONSTANTS / MACROS
//**************************************************************************

#define BK21_ACTIVE ((m_bk21 == 0) && (m_switch->read() & 0x01))
#define BK22_ACTIVE ((m_bk22 == 0) && (m_switch->read() & 0x02))
#define BK31_ACTIVE ((m_bk31 == 0) && (m_switch->read() & 0x04))
#define BK32_ACTIVE ((m_bk32 == 0) && (m_switch->read() & 0x08))
#define BK02_ACTIVE                   (m_switch->read() & 0x10)


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type SV807 = &device_creator<sv807_device>;

//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

static INPUT_PORTS_START( sv807_switches )
	PORT_START("S")
	PORT_DIPNAME(0x01, 0x00, "Bank/Page 21")
	PORT_DIPLOCATION("S:1")
	PORT_DIPSETTING(0x00, "Off")
	PORT_DIPSETTING(0x01, "On")
	PORT_DIPNAME(0x02, 0x02, "Bank/Page 22")
	PORT_DIPLOCATION("S:2")
	PORT_DIPSETTING(0x00, "Off")
	PORT_DIPSETTING(0x02, "On")
	PORT_DIPNAME(0x04, 0x04, "Bank/Page 31")
	PORT_DIPLOCATION("S:3")
	PORT_DIPSETTING(0x00, "Off")
	PORT_DIPSETTING(0x04, "On")
	PORT_DIPNAME(0x08, 0x00, "Bank/Page 32")
	PORT_DIPLOCATION("S:4")
	PORT_DIPSETTING(0x00, "Off")
	PORT_DIPSETTING(0x08, "On")
	PORT_DIPNAME(0x10, 0x00, "Bank/Page 02")
	PORT_DIPLOCATION("S:5")
	PORT_DIPSETTING(0x00, "Off")
	PORT_DIPSETTING(0x10, "On")
	PORT_DIPNAME(0x20, 0x00, "48k/32k")
	PORT_DIPLOCATION("S:6")
	PORT_DIPSETTING(0x00, "Off")
	PORT_DIPSETTING(0x20, "On")
INPUT_PORTS_END

ioport_constructor sv807_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( sv807_switches );
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  sv807_device - constructor
//-------------------------------------------------

sv807_device::sv807_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, SV803, "SV-807 64k RAM Cartridge", tag, owner, clock, "sv807", __FILE__),
	device_svi_slot_interface(mconfig, *this),
	m_switch(*this, "S"),
	m_bk21(1), m_bk22(1), m_bk31(1), m_bk32(1)
{
	m_ram_bank1 = std::make_unique<UINT8[]>(0x8000);
	m_ram_bank2 = std::make_unique<UINT8[]>(0x8000);
	memset(m_ram_bank1.get(), 0xff, 0x8000);
	memset(m_ram_bank2.get(), 0xff, 0x8000);
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void sv807_device::device_start()
{
	// register for savestates
	save_item(NAME(m_bk21));
	save_item(NAME(m_bk22));
	save_item(NAME(m_bk31));
	save_item(NAME(m_bk32));
	save_pointer(NAME(m_ram_bank1.get()), 0x8000);
	save_pointer(NAME(m_ram_bank2.get()), 0x8000);
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void sv807_device::device_reset()
{
}


//**************************************************************************
//  IMPLEMENTATION
//**************************************************************************

// test setup: S2 = enabled (22), S3 = enabled (31)

READ8_MEMBER( sv807_device::mreq_r )
{
	if ((BK21_ACTIVE || BK31_ACTIVE) && offset < 0x8000)
	{
		m_bus->romdis_w(0);
		return m_ram_bank1[offset];
	}

	if ((BK22_ACTIVE || BK32_ACTIVE || BK02_ACTIVE) && offset >= 0x8000)
	{
		m_bus->ramdis_w(0);
		return m_ram_bank2[offset - 0x8000];
	}

	return 0xff;
}

WRITE8_MEMBER( sv807_device::mreq_w )
{
	if ((BK21_ACTIVE || BK31_ACTIVE) && offset < 0x8000)
	{
		m_bus->romdis_w(0);
		m_ram_bank1[offset] = data;
	}

	if ((BK22_ACTIVE || BK32_ACTIVE || BK02_ACTIVE) && offset >= 0x8000)
	{
		m_bus->ramdis_w(0);
		m_ram_bank2[offset - 0x8000] = data;
	}
}

void sv807_device::bk21_w(int state)
{
	m_bk21 = state;
}

void sv807_device::bk22_w(int state)
{
	m_bk22 = state;
}

void sv807_device::bk31_w(int state)
{
	m_bk31 = state;
}

void sv807_device::bk32_w(int state)
{
	m_bk32 = state;
}
