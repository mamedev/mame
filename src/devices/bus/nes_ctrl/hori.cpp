// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/**********************************************************************

    Nintendo Family Computer Hori Twin (and 4P?) adapters

    Emulation of the 4Players adapter is quite pointless: if 2P mode
    (default mode) it behaves like a Hori Twin adapter, in 4P mode
    it has P1 and P2 inputs overwriting the inputs coming from the
    main controllers (possibly creating a bit of confusion, since
    you get 6 sets of inputs with only 4 acknowledged by the running
    system).
    For the moment we keep it available for documentation purposes.

    TODO: find out confirmation whether in 2P mode, inputs from joypads
    connected to the 4players adapter are really seen as P3 and P4 inputs.
    it seems the most reasonable setup (so that users with only 2
    external pads can use the adapter in 4P games), but one never knows...

**********************************************************************/

#include "hori.h"
#include "joypad.h"

//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type NES_HORITWIN = &device_creator<nes_horitwin_device>;
const device_type NES_HORI4P = &device_creator<nes_hori4p_device>;


static INPUT_PORTS_START( nes_hori4p )
	PORT_START("CONFIG")
	PORT_CONFNAME( 0x01, 0x00, "4 Players / 2 Players")
	PORT_CONFSETTING(  0x00, "2 Players" )
	PORT_CONFSETTING(  0x01, "4 Players" )
INPUT_PORTS_END


//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor nes_hori4p_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( nes_hori4p );
}


static SLOT_INTERFACE_START( hori_adapter )
	SLOT_INTERFACE("joypad", NES_JOYPAD)
SLOT_INTERFACE_END

static MACHINE_CONFIG_FRAGMENT( horitwin )
	MCFG_FC_EXPANSION_PORT_ADD("port1", hori_adapter, "joypad")
	MCFG_FC_EXPANSION_PORT_ADD("port2", hori_adapter, "joypad")
MACHINE_CONFIG_END

static MACHINE_CONFIG_FRAGMENT( hori4p )
	MCFG_FC_EXPANSION_PORT_ADD("port1", hori_adapter, "joypad")
	MCFG_FC_EXPANSION_PORT_ADD("port2", hori_adapter, "joypad")
	MCFG_FC_EXPANSION_PORT_ADD("port3", hori_adapter, "joypad")
	MCFG_FC_EXPANSION_PORT_ADD("port4", hori_adapter, "joypad")
MACHINE_CONFIG_END


//-------------------------------------------------
//  machine_config_additions - device-specific
//  machine configurations
//-------------------------------------------------

machine_config_constructor nes_horitwin_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( horitwin );
}

machine_config_constructor nes_hori4p_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( hori4p );
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  nes_horitwin_device - constructor
//-------------------------------------------------

nes_horitwin_device::nes_horitwin_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
					device_t(mconfig, NES_HORITWIN, "Hori Twin Adapter", tag, owner, clock, "nes_horitwin", __FILE__),
					device_nes_control_port_interface(mconfig, *this),
					m_port1(*this, "port1"),
					m_port2(*this, "port2")
{
}

nes_hori4p_device::nes_hori4p_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
					device_t(mconfig, NES_HORI4P, "Hori 4P Adapter", tag, owner, clock, "nes_hori4p", __FILE__),
					device_nes_control_port_interface(mconfig, *this),
					m_port1(*this, "port1"),
					m_port2(*this, "port2"),
					m_port3(*this, "port3"),
					m_port4(*this, "port4"),
					m_cfg(*this, "CONFIG")
{
}


//-------------------------------------------------
//  read
//-------------------------------------------------

UINT8 nes_horitwin_device::read_exp(offs_t offset)
{
	UINT8 ret = 0;
	if (offset == 0)    //$4016
		ret |= (m_port1->read_bit0() << 1);
	else    //$4017
		ret |= (m_port2->read_bit0() << 1);
	return ret;
}

UINT8 nes_hori4p_device::read_exp(offs_t offset)
{
	UINT8 ret = 0;
	if (m_cfg->read() == 0) // 2P
	{
		if (offset == 0)    //$4016
			ret |= (m_port1->read_bit0() << 1);
		else    //$4017
			ret |= (m_port2->read_bit0() << 1);
	}
	else    // 4P
	{
		if (offset == 0)    //$4016
		{
			ret |= (m_port1->read_bit0() << 0);
			ret |= (m_port3->read_bit0() << 1);
		}
		else    //$4017
		{
			ret |= (m_port2->read_bit0() << 0);
			ret |= (m_port4->read_bit0() << 1);
		}
	}
	return ret;
}

//-------------------------------------------------
//  write
//-------------------------------------------------

void nes_horitwin_device::write(UINT8 data)
{
	m_port1->write(data);
	m_port2->write(data);
}

void nes_hori4p_device::write(UINT8 data)
{
	m_port1->write(data);
	m_port2->write(data);
	m_port3->write(data);
	m_port4->write(data);
}
