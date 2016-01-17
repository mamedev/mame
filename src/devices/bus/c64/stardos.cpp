// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    StarPoint Software StarDOS cartridge emulation

**********************************************************************/

/*

    PCB Layout
    ----------

    |===========================|
    |=|                         |
    |=|   LS30                  |
    |=|               LS157     |
    |=|          LS00           |
    |=|   ROM                   |
    |=|          7407           |
    |=|                         |
    |=|                      SW1|
    |===========================|

    ROM     - Toshiba TMM271128D-25 16Kx8 EPROM

*/

#include "stardos.h"



//**************************************************************************
//  MACROS/CONSTANTS
//**************************************************************************

#define IO1_FULL_CHARGE     27
#define IO2_FULL_CHARGE     42



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type C64_STARDOS = &device_creator<c64_stardos_cartridge_device>;


//-------------------------------------------------
//  INPUT_PORTS( c64_stardos )
//-------------------------------------------------

static INPUT_PORTS_START( c64_stardos )
	PORT_START("RESET")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Reset") PORT_CODE(KEYCODE_F11) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF_OWNER, c64_expansion_slot_device, reset_w)
INPUT_PORTS_END


//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor c64_stardos_cartridge_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( c64_stardos );
}



//**************************************************************************
//  INLINE HELPERS
//**************************************************************************

//-------------------------------------------------
//  charge_io1_capacitor -
//-------------------------------------------------

inline void c64_stardos_cartridge_device::charge_io1_capacitor()
{
	m_io1_charge++;

	if (m_io1_charge >= IO1_FULL_CHARGE)
	{
		m_exrom = 0;
		m_io1_charge = 0;
	}
}


//-------------------------------------------------
//  charge_io2_capacitor -
//-------------------------------------------------

void c64_stardos_cartridge_device::charge_io2_capacitor()
{
	m_io2_charge++;

	if (m_io2_charge >= IO2_FULL_CHARGE)
	{
		m_exrom = 1;
		m_io2_charge = 0;
	}
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  c64_stardos_cartridge_device - constructor
//-------------------------------------------------

c64_stardos_cartridge_device::c64_stardos_cartridge_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, C64_STARDOS, "C64 StarDOS cartridge", tag, owner, clock, "c64_stardos", __FILE__),
	device_c64_expansion_card_interface(mconfig, *this),
	m_io1_charge(0),
	m_io2_charge(0)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void c64_stardos_cartridge_device::device_start()
{
	// state saving
	save_item(NAME(m_io1_charge));
	save_item(NAME(m_io2_charge));
}


//-------------------------------------------------
//  c64_cd_r - cartridge data read
//-------------------------------------------------

UINT8 c64_stardos_cartridge_device::c64_cd_r(address_space &space, offs_t offset, UINT8 data, int sphi2, int ba, int roml, int romh, int io1, int io2)
{
	if (!roml || !romh)
	{
		// TODO BITSWAP8(7,6,5,4,3,1,2,0) ?
		data = m_roml[offset & 0x3fff];
	}
	else if (!io1)
	{
		charge_io1_capacitor();
	}
	else if (!io2)
	{
		charge_io2_capacitor();
	}

	return data;
}


//-------------------------------------------------
//  c64_cd_w - cartridge data write
//-------------------------------------------------

void c64_stardos_cartridge_device::c64_cd_w(address_space &space, offs_t offset, UINT8 data, int sphi2, int ba, int roml, int romh, int io1, int io2)
{
	if (!io1)
	{
		charge_io1_capacitor();
	}
	else if (!io2)
	{
		charge_io2_capacitor();
	}
}


//-------------------------------------------------
//  c64_game_r - GAME read
//-------------------------------------------------

int c64_stardos_cartridge_device::c64_game_r(offs_t offset, int sphi2, int ba, int rw)
{
	return !(sphi2 && ba & rw & ((offset & 0xe000) == 0xe000) & m_slot->hiram());
}


//-------------------------------------------------
//  c64_exrom_r - EXROM read
//-------------------------------------------------

int c64_stardos_cartridge_device::c64_exrom_r(offs_t offset, int sphi2, int ba, int rw)
{
	return (BIT(offset, 13)) ? 1 : m_exrom;
}
