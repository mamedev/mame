/**********************************************************************

    Final Cartridge emulation

    Copyright MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

**********************************************************************/

#include "c64_final.h"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type C64_FINAL = &device_creator<c64_final_cartridge_device>;


INPUT_CHANGED_MEMBER( c64_final_cartridge_device::reset )
{
	if (!newval)
	{
		device_reset();
	}

	m_slot->reset_w(newval ? CLEAR_LINE : ASSERT_LINE);
}

INPUT_CHANGED_MEMBER( c64_final_cartridge_device::freeze )
{
	if (!newval)
	{
		m_game = 0;
	}

	m_slot->nmi_w(newval ? CLEAR_LINE : ASSERT_LINE);
}

static INPUT_PORTS_START( c64_final )
	PORT_START("SW")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Reset") PORT_CODE(KEYCODE_F11) PORT_CHANGED_MEMBER(DEVICE_SELF, c64_final_cartridge_device, reset, 0)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Freeze") PORT_CODE(KEYCODE_F12) PORT_CHANGED_MEMBER(DEVICE_SELF, c64_final_cartridge_device, freeze, 0)
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

c64_final_cartridge_device::c64_final_cartridge_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, C64_FINAL, "C64 Final cartridge", tag, owner, clock),
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

UINT8 c64_final_cartridge_device::c64_cd_r(address_space &space, offs_t offset, UINT8 data, int ba, int roml, int romh, int io1, int io2)
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

void c64_final_cartridge_device::c64_cd_w(address_space &space, offs_t offset, UINT8 data, int ba, int roml, int romh, int io1, int io2)
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
