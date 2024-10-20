// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Final Cartridge emulation

**********************************************************************/

#include "emu.h"
#include "final.h"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(C64_FINAL, c64_final_cartridge_device, "c64_final", "C64 Final Cartridge")


//-------------------------------------------------
//  INPUT_CHANGED_MEMBER( freeze )
//-------------------------------------------------

INPUT_CHANGED_MEMBER( c64_final_cartridge_device::freeze )
{
	if (newval)
	{
		m_game = 0;
	}

	m_slot->nmi_w(newval);
}


//-------------------------------------------------
//  INPUT_PORTS( c64_final )
//-------------------------------------------------

static INPUT_PORTS_START( c64_final )
	PORT_START("SW")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Reset") PORT_CODE(KEYCODE_F11) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF_OWNER, FUNC(c64_expansion_slot_device::reset_w))
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Freeze") PORT_CODE(KEYCODE_F12) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(c64_final_cartridge_device::freeze), 0)
INPUT_PORTS_END


//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor c64_final_cartridge_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( c64_final );
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  c64_final_cartridge_device - constructor
//-------------------------------------------------

c64_final_cartridge_device::c64_final_cartridge_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, C64_FINAL, tag, owner, clock),
	device_c64_expansion_card_interface(mconfig, *this)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void c64_final_cartridge_device::device_start()
{
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void c64_final_cartridge_device::device_reset()
{
	m_exrom = 0;
	m_game = 0;
}


//-------------------------------------------------
//  c64_cd_r - cartridge data read
//-------------------------------------------------

uint8_t c64_final_cartridge_device::c64_cd_r(offs_t offset, uint8_t data, int sphi2, int ba, int roml, int romh, int io1, int io2)
{
	if (!roml || !romh || !io1 || !io2)
	{
		data = m_roml[offset & 0x3fff];
	}

	if (!io1)
	{
		m_game = 1;
		m_exrom = 1;
	}
	else if (!io2)
	{
		m_exrom = 0;
		m_game = 0;
	}

	return data;
}


//-------------------------------------------------
//  c64_cd_w - cartridge data write
//-------------------------------------------------

void c64_final_cartridge_device::c64_cd_w(offs_t offset, uint8_t data, int sphi2, int ba, int roml, int romh, int io1, int io2)
{
	if (!io1)
	{
		m_game = 1;
		m_exrom = 1;
	}
	else if (!io2)
	{
		m_exrom = 0;
		m_game = 0;
	}
}
