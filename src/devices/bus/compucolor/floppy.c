// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Compucolor Floppy Disk Drive emulation

*********************************************************************/

#include "floppy.h"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type COMPUCOLOR_FLOPPY_PORT = &device_creator<compucolor_floppy_port_device>;
const device_type COMPUCOLOR_FLOPPY = &device_creator<compucolor_floppy_device>;


//-------------------------------------------------
//  SLOT_INTERFACE( compucolor_floppy_port_devices )
//-------------------------------------------------

SLOT_INTERFACE_START( compucolor_floppy_port_devices )
	SLOT_INTERFACE("floppy", COMPUCOLOR_FLOPPY)
SLOT_INTERFACE_END


//-------------------------------------------------
//  FLOPPY_FORMATS( floppy_formats )
//-------------------------------------------------

FLOPPY_FORMATS_MEMBER( compucolor_floppy_device::floppy_formats )
	FLOPPY_CCVF_FORMAT
FLOPPY_FORMATS_END


//-------------------------------------------------
//  SLOT_INTERFACE( compucolor_floppies )
//-------------------------------------------------

static SLOT_INTERFACE_START( compucolor_floppies )
	SLOT_INTERFACE_INTERNAL( "525sssd", FLOPPY_525_SSSD )
SLOT_INTERFACE_END


//-------------------------------------------------
//  MACHINE_DRIVER( compucolor_floppy )
//-------------------------------------------------

static MACHINE_CONFIG_FRAGMENT( compucolor_floppy )
	MCFG_FLOPPY_DRIVE_ADD("floppy", compucolor_floppies, "525sssd", compucolor_floppy_device::floppy_formats)
MACHINE_CONFIG_END


//-------------------------------------------------
//  machine_config_additions - device-specific
//  machine configurations
//-------------------------------------------------

machine_config_constructor compucolor_floppy_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( compucolor_floppy );
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  device_compucolor_floppy_port_interface - constructor
//-------------------------------------------------

device_compucolor_floppy_port_interface::device_compucolor_floppy_port_interface(const machine_config &mconfig, device_t &device)
	: device_rs232_port_interface(mconfig, device)
{
}


//-------------------------------------------------
//  compucolor_floppy_port_device - constructor
//-------------------------------------------------

compucolor_floppy_port_device::compucolor_floppy_port_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: rs232_port_device(mconfig, COMPUCOLOR_FLOPPY_PORT, "Compucolor Floppy Port", tag, owner, clock, "compclr_flp_port", __FILE__)
{
}


//-------------------------------------------------
//  compucolor_floppy_device - constructor
//-------------------------------------------------

compucolor_floppy_device::compucolor_floppy_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, COMPUCOLOR_FLOPPY, "Compucolor floppy", tag, owner, clock, "compclr_flp", __FILE__),
		device_compucolor_floppy_port_interface(mconfig, *this),
		m_floppy(*this, "floppy:525sssd"),
		m_rw(1),
		m_stp(0),
		m_sel(1),
		m_period(attotime::from_hz(9600*8))
{
	m_owner = dynamic_cast<compucolor_floppy_port_device *>(this->owner());
}


//-------------------------------------------------
//  device_config_complete -
//-------------------------------------------------

void compucolor_floppy_port_device::device_config_complete()
{
	rs232_port_device::device_config_complete();

	m_dev = dynamic_cast<device_compucolor_floppy_port_interface *>(get_card_device());
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void compucolor_floppy_port_device::device_start()
{
	rs232_port_device::device_start();
}


void compucolor_floppy_device::device_start()
{
	// allocate timer
	m_timer = timer_alloc();
	m_timer->adjust(attotime::from_hz(9600*8), 0, attotime::from_hz(9600*8));

	// state saving
	save_item(NAME(m_rw));
	save_item(NAME(m_stp));
	save_item(NAME(m_sel));
}


//-------------------------------------------------
//  device_timer - handle timer events
//-------------------------------------------------

void compucolor_floppy_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	if (!m_sel && !m_rw)
	{
		output_rxd(read_bit());
	}
}


//-------------------------------------------------
//  tx_w -
//-------------------------------------------------

void compucolor_floppy_device::tx(UINT8 state)
{
	if (!m_sel && m_rw)
	{
		write_bit(state);
	}
}


//-------------------------------------------------
//  rw_w -
//-------------------------------------------------

void compucolor_floppy_device::rw_w(int state)
{
	if (!m_rw && state)
	{
		output_rxd(1);
	}

	m_rw = state;
}


//-------------------------------------------------
//  stepper_w -
//-------------------------------------------------

void compucolor_floppy_device::stepper_w(UINT8 data)
{
	if (!m_sel)
	{
		if ((m_stp == 1 && data == 4) || (m_stp == 2 && data == 1) || (m_stp == 4 && data == 2))
		{
			// step in
			m_floppy->dir_w(1);
			m_floppy->stp_w(0);
			m_floppy->stp_w(1);
		}
		else if ((m_stp == 1 && data == 2) || (m_stp == 2 && data == 4) || (m_stp == 4 && data == 1))
		{
			// step out
			m_floppy->dir_w(0);
			m_floppy->stp_w(0);
			m_floppy->stp_w(1);
		}
	}

	m_stp = data;
}


//-------------------------------------------------
//  select_w -
//-------------------------------------------------

void compucolor_floppy_device::select_w(int state)
{
	m_floppy->mon_w(state);

	if (!m_sel && state)
	{
		output_rxd(1);
	}

	m_sel = state;
}


//-------------------------------------------------
//  read_bit -
//-------------------------------------------------

bool compucolor_floppy_device::read_bit()
{
	attotime when = machine().time();
	attotime edge = m_floppy->get_next_transition(when);
	attotime next = when + m_period;

	return (edge.is_never() || edge >= next) ? 0 : 1;
}


//-------------------------------------------------
//  write_bit -
//-------------------------------------------------

void compucolor_floppy_device::write_bit(bool bit)
{
	// TODO
}
