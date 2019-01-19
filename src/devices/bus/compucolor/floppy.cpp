// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Compucolor Floppy Disk Drive emulation

*********************************************************************/

#include "emu.h"
#include "floppy.h"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(COMPUCOLOR_FLOPPY_PORT, compucolor_floppy_port_device, "compclr_flp_port", "Compucolor Floppy Port")
DEFINE_DEVICE_TYPE(COMPUCOLOR_FLOPPY,      compucolor_floppy_device,      "compclr_flp",      "Compucolor floppy")


//-------------------------------------------------
//  SLOT_INTERFACE( compucolor_floppy_port_devices )
//-------------------------------------------------

void compucolor_floppy_port_devices(device_slot_interface &device)
{
	device.option_add("floppy", COMPUCOLOR_FLOPPY);
}


//-------------------------------------------------
//  FLOPPY_FORMATS( floppy_formats )
//-------------------------------------------------

FLOPPY_FORMATS_MEMBER( compucolor_floppy_device::floppy_formats )
	FLOPPY_CCVF_FORMAT
FLOPPY_FORMATS_END


//-------------------------------------------------
//  SLOT_INTERFACE( compucolor_floppies )
//-------------------------------------------------

static void compucolor_floppies(device_slot_interface &device)
{
	device.option_add_internal("525sssd", FLOPPY_525_SSSD);
}


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void compucolor_floppy_device::device_add_mconfig(machine_config &config)
{
	FLOPPY_CONNECTOR(config, m_floppy, compucolor_floppies, "525sssd", compucolor_floppy_device::floppy_formats);
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

compucolor_floppy_port_device::compucolor_floppy_port_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: rs232_port_device(mconfig, COMPUCOLOR_FLOPPY_PORT, tag, owner, clock), m_dev(nullptr)
{
}


//-------------------------------------------------
//  compucolor_floppy_device - constructor
//-------------------------------------------------

compucolor_floppy_device::compucolor_floppy_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, COMPUCOLOR_FLOPPY, tag, owner, clock)
	, device_compucolor_floppy_port_interface(mconfig, *this)
	, m_floppy(*this, "floppy")
	, m_rw(1)
	, m_stp(0)
	, m_sel(1)
	, m_period(attotime::from_hz(9600*8))
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

void compucolor_floppy_device::tx(uint8_t state)
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

void compucolor_floppy_device::stepper_w(uint8_t data)
{
	if (!m_sel)
	{
		floppy_image_device *floppy = m_floppy->get_device();
		if (floppy == nullptr)
			return;

		if ((m_stp == 1 && data == 4) || (m_stp == 2 && data == 1) || (m_stp == 4 && data == 2))
		{
			// step in
			floppy->dir_w(1);
			floppy->stp_w(0);
			floppy->stp_w(1);
		}
		else if ((m_stp == 1 && data == 2) || (m_stp == 2 && data == 4) || (m_stp == 4 && data == 1))
		{
			// step out
			floppy->dir_w(0);
			floppy->stp_w(0);
			floppy->stp_w(1);
		}
	}

	m_stp = data;
}


//-------------------------------------------------
//  select_w -
//-------------------------------------------------

void compucolor_floppy_device::select_w(int state)
{
	floppy_image_device *floppy = m_floppy->get_device();
	if (floppy != nullptr)
		floppy->mon_w(state);

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
	floppy_image_device *floppy = m_floppy->get_device();

	attotime when = machine().time();
	attotime edge = (floppy == nullptr) ? attotime::never : floppy->get_next_transition(when);
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
