// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    TUG 64K Dynamic RAM Board

    http://www.microtan.ukpc.net/pageProducts.html#RAM

**********************************************************************/


#include "emu.h"
#include "tug64k.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(TANBUS_TUG64K, tanbus_tug64k_device, "tanbus_tug64k", "TUG 64K Dynamic RAM Board")


//-------------------------------------------------
//  INPUT_PORTS( tug64k )
//-------------------------------------------------

INPUT_PORTS_START(tug64k)
	PORT_START("SW1")
	PORT_DIPNAME(0x01, 0x01, "Overlay Tanex RAM") PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(0x00, DEF_STR(Off))
	PORT_DIPSETTING(0x01, DEF_STR(On))
	PORT_DIPNAME(0x02, 0x02, "Recognise INHRAM Signal") PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(0x00, DEF_STR(Off))
	PORT_DIPSETTING(0x02, DEF_STR(On))
	PORT_DIPNAME(0x04, 0x04, "Recognise I/O Signal") PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(0x00, DEF_STR(Off))
	PORT_DIPSETTING(0x04, DEF_STR(On))
	PORT_DIPNAME(0x08, 0x08, "Recognise Block Enable") PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(0x00, DEF_STR(Off))
	PORT_DIPSETTING(0x08, DEF_STR(On))

	PORT_START("SW2")
	PORT_DIPNAME(0x01, 0x00, "Disable RAM &C000-&C7FF") PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(0x00, DEF_STR(Off))
	PORT_DIPSETTING(0x01, DEF_STR(On))
	PORT_DIPNAME(0x02, 0x00, "Disable RAM &C800-&CFFF") PORT_DIPLOCATION("SW2:2")
	PORT_DIPSETTING(0x00, DEF_STR(Off))
	PORT_DIPSETTING(0x02, DEF_STR(On))
	PORT_DIPNAME(0x04, 0x00, "Disable RAM &D000-&D7FF") PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(0x00, DEF_STR(Off))
	PORT_DIPSETTING(0x04, DEF_STR(On))
	PORT_DIPNAME(0x08, 0x00, "Disable RAM &D800-&DFFF") PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(0x00, DEF_STR(Off))
	PORT_DIPSETTING(0x08, DEF_STR(On))
	PORT_DIPNAME(0x10, 0x00, "Disable RAM &E000-&E7FF") PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(0x00, DEF_STR(Off))
	PORT_DIPSETTING(0x10, DEF_STR(On))
	PORT_DIPNAME(0x20, 0x00, "Disable RAM &E800-&EFFF") PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(0x00, DEF_STR(Off))
	PORT_DIPSETTING(0x20, DEF_STR(On))
	PORT_DIPNAME(0x40, 0x00, "Disable RAM &F000-&F7FF") PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(0x00, DEF_STR(Off))
	PORT_DIPSETTING(0x40, DEF_STR(On))
	PORT_DIPNAME(0x80, 0x00, "Not Used") PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(0x00, DEF_STR(Off))
	PORT_DIPSETTING(0x80, DEF_STR(On))
INPUT_PORTS_END

//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor tanbus_tug64k_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(tug64k);
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  tanbus_tug64k_device - constructor
//-------------------------------------------------

tanbus_tug64k_device::tanbus_tug64k_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, TANBUS_TUG64K, tag, owner, clock)
	, device_tanbus_interface(mconfig, *this)
	, m_dsw(*this, "SW%u", 1)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void tanbus_tug64k_device::device_start()
{
	m_ram = std::make_unique<uint8_t[]>(0x10000);

	save_pointer(NAME(m_ram), 0x10000);
}

//-------------------------------------------------
//  read - card read
//-------------------------------------------------

uint8_t tanbus_tug64k_device::read(offs_t offset, int inhrom, int inhram, int be)
{
	uint8_t data = 0xff;

	if (ram_enabled(offset, inhrom, inhram, be))
	{
		data = m_ram[offset];
	}

	return data;
}

//-------------------------------------------------
//  write - card write
//-------------------------------------------------

void tanbus_tug64k_device::write(offs_t offset, uint8_t data, int inhrom, int inhram, int be)
{
	if (ram_enabled(offset, inhrom, inhram, be))
	{
		m_ram[offset] = data;
	}
}


bool tanbus_tug64k_device::ram_enabled(offs_t offset, int inhrom, int inhram, int be)
{
	/* overlay Tanex RAM */
	if (!BIT(m_dsw[0]->read(), 0) && (offset >= 0x0400) && (offset < 0x2000))
		return false;

	/* recognise INHRAM signal */
	if (BIT(m_dsw[0]->read(), 1) && inhram)
		return false;

	/* recognise I/O signal (should be a Tanex output) */
	if (BIT(m_dsw[0]->read(), 2) && (offset & 0xfc00) == 0xbc00)
		return false;

	/* recognise Block Enable */
	if (BIT(m_dsw[0]->read(), 3) && !be)
		return false;

	/* disable RAM locations */
	switch (offset & 0xf800)
	{
	case 0xc000:
		if (BIT(m_dsw[1]->read(), 0))
			return false;
		break;
	case 0xc800:
		if (BIT(m_dsw[1]->read(), 1))
			return false;
		break;
	case 0xd000:
		if (BIT(m_dsw[1]->read(), 2))
			return false;
		break;
	case 0xd800:
		if (BIT(m_dsw[1]->read(), 3))
			return false;
		break;
	case 0xe000:
		if (BIT(m_dsw[1]->read(), 4))
			return false;
		break;
	case 0xe800:
		if (BIT(m_dsw[1]->read(), 5))
			return false;
		break;
	case 0xf000:
		if (BIT(m_dsw[1]->read(), 6))
			return false;
		break;
	case 0xf800:
		return false;
		break;
	}

	return true;
}
