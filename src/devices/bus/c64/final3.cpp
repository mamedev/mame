// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Final Cartridge III emulation

**********************************************************************/

#include "emu.h"
#include "final3.h"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(C64_FINAL3, c64_final3_cartridge_device, "c64_final3", "C64 Final Cartridge III")


//-------------------------------------------------
//  INPUT_CHANGED_MEMBER( freeze )
//-------------------------------------------------

INPUT_CHANGED_MEMBER( c64_final3_cartridge_device::freeze )
{
	if (newval)
	{
		m_game = 0;
		m_hidden = 0;
	}

	m_slot->nmi_w(newval);
}


//-------------------------------------------------
//  INPUT_PORTS( c64_final3 )
//-------------------------------------------------

static INPUT_PORTS_START( c64_final3 )
	PORT_START("SW")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Reset") PORT_CODE(KEYCODE_F11) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF_OWNER, FUNC(c64_expansion_slot_device::reset_w))
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Freeze") PORT_CODE(KEYCODE_F12) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(c64_final3_cartridge_device::freeze), 0)
INPUT_PORTS_END


//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor c64_final3_cartridge_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( c64_final3 );
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  c64_final3_cartridge_device - constructor
//-------------------------------------------------

c64_final3_cartridge_device::c64_final3_cartridge_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, C64_FINAL3, tag, owner, clock),
	device_c64_expansion_card_interface(mconfig, *this), m_bank(0), m_hidden(0)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void c64_final3_cartridge_device::device_start()
{
	// state saving
	save_item(NAME(m_bank));
	save_item(NAME(m_hidden));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void c64_final3_cartridge_device::device_reset()
{
	m_bank = 0;

	m_exrom = 0;
	m_game = 0;

	m_slot->nmi_w(CLEAR_LINE);

	m_hidden = 0;
}


//-------------------------------------------------
//  c64_cd_r - cartridge data read
//-------------------------------------------------

uint8_t c64_final3_cartridge_device::c64_cd_r(offs_t offset, uint8_t data, int sphi2, int ba, int roml, int romh, int io1, int io2)
{
	if (!roml || !romh || !io1 || !io2)
	{
		offs_t addr = (m_bank << 14) | (offset & 0x3fff);
		data = m_roml[addr];
	}

	return data;
}


//-------------------------------------------------
//  c64_cd_w - cartridge data write
//-------------------------------------------------

void c64_final3_cartridge_device::c64_cd_w(offs_t offset, uint8_t data, int sphi2, int ba, int roml, int romh, int io1, int io2)
{
	if (!m_hidden && !io2 && ((offset & 0xff) == 0xff))
	{
		/*

		    bit     description

		    0       A14
		    1       A15
		    2
		    3
		    4       EXROM
		    5       GAME
		    6       NMI
		    7       hide register

		*/

		m_bank = data & 0x03;

		m_exrom = BIT(data, 4);
		m_game = BIT(data, 5);

		m_slot->nmi_w(BIT(data, 6) ? CLEAR_LINE : ASSERT_LINE);

		m_hidden = BIT(data, 7);
	}
}
