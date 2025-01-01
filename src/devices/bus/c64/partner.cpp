// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Timeworks PARTNER 64 cartridge emulation

**********************************************************************/

/*

    PCB Layout
    ----------

    |===========================|
    |=|                         |
    |=|LS05 LS09 LS00     HC74  |
    |=|                         |
    |=|                         |
    |=|   ROM       RAM         |
    |=|       LS133             |
    |=|                   LS156 |
    |=|                         |
    |===========================|

    ROM     - General Instrument 27C128-25 16Kx8 EPROM "TIMEWORKS C-64 VER 2-16-87"
    RAM     - Sony CXK5864PN-15L 8Kx8 SRAM

*/

#include "emu.h"
#include "partner.h"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(C64_PARTNER, c64_partner_cartridge_device, "c64_partner", "C64 PARTNER 64 cartridge")


//-------------------------------------------------
//  INPUT_PORTS( c64_partner )
//-------------------------------------------------

void c64_partner_cartridge_device::nmi_w(int state)
{
	if (!state && !m_a6 && !m_nmi)
	{
		m_slot->nmi_w(ASSERT_LINE);
		m_nmi = 1;
	}
}

static INPUT_PORTS_START( c64_partner )
	PORT_START("NMI")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Menu") PORT_CODE(KEYCODE_F11) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, FUNC(c64_partner_cartridge_device::nmi_w))
INPUT_PORTS_END


//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor c64_partner_cartridge_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( c64_partner );
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  c64_partner_cartridge_device - constructor
//-------------------------------------------------

c64_partner_cartridge_device::c64_partner_cartridge_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, C64_PARTNER, tag, owner, clock),
	device_c64_expansion_card_interface(mconfig, *this),
	m_ram(*this, "ram", 0x2000, ENDIANNESS_LITTLE),
	m_a0(1),
	m_a6(1),
	m_nmi(0)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void c64_partner_cartridge_device::device_start()
{
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void c64_partner_cartridge_device::device_reset()
{
	m_a0 = 1;
	m_a6 = 1;

	if (m_nmi && m_a6)
	{
		m_slot->nmi_w(CLEAR_LINE);
		m_nmi = 0;
	}
}


//-------------------------------------------------
//  c64_cd_r - cartridge data read
//-------------------------------------------------

uint8_t c64_partner_cartridge_device::c64_cd_r(offs_t offset, uint8_t data, int sphi2, int ba, int roml, int romh, int io1, int io2)
{
	if (!io1)
	{
		data = m_romh[offset & 0x3fff];
	}

	if (m_nmi && (offset == 0xfffa || offset == 0xfffb))
	{
		m_a0 = 1;
	}

	if (m_a0 && BIT(offset, 15))
	{
		switch ((offset >> 13) & 0x03)
		{
		case 0: case 3:
			data = m_romh[offset & 0x3fff];
			break;

		case 1:
			data = m_ram[offset & 0x1fff];
			break;
		}
	}

	return data;
}


//-------------------------------------------------
//  c64_cd_w - cartridge data write
//-------------------------------------------------

void c64_partner_cartridge_device::c64_cd_w(offs_t offset, uint8_t data, int sphi2, int ba, int roml, int romh, int io1, int io2)
{
	if (!io1)
	{
		m_a0 = BIT(offset, 0);
		m_a6 = BIT(offset, 6);

		if (m_nmi && m_a6)
		{
			m_slot->nmi_w(CLEAR_LINE);
			m_nmi = 0;
		}
	}

	if (m_a0 && BIT(offset, 15))
	{
		switch ((offset >> 13) & 0x03)
		{
		case 1:
			m_ram[offset & 0x1fff] = data;
			break;
		}
	}

	if (m_nmi && (offset == 0xfffa || offset == 0xfffb))
	{
		m_a0 = 1;
	}
}


//-------------------------------------------------
//  c64_game_r - GAME read
//-------------------------------------------------

int c64_partner_cartridge_device::c64_game_r(offs_t offset, int sphi2, int ba, int rw)
{
	int game = 1;

	if (m_a0 && BIT(offset, 15))
	{
		switch ((offset >> 13) & 0x03)
		{
		case 0: case 1: case 3:
			game = 0;
			break;
		}
	}

	// TODO if I/O1=0, GAME=0

	return game;
}
