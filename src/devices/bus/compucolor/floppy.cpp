// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Compucolor Floppy Disk Drive emulation

*********************************************************************/

#include "emu.h"
#include "floppy.h"

#include "formats/ccvf_dsk.h"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(COMPUCOLOR_FLOPPY_PORT, compucolor_floppy_port_device, "compclr_flp_port", "Compucolor Floppy Port")
DEFINE_DEVICE_TYPE(COMPUCOLOR_FLOPPY,      compucolor_floppy_device,      "compclr_flp",      "Compucolor floppy")
DEFINE_DEVICE_TYPE(COMPUCOLOR_FLOPPY_4PH,  compucolor_floppy_4ph_device,  "compclr_flp_4ph",  "Compucolor floppy (4-phase stepper)")


//-------------------------------------------------
//  SLOT_INTERFACE( compucolor_floppy_port_devices )
//-------------------------------------------------

void compucolor_floppy_port_devices(device_slot_interface &device)
{
	device.option_add("floppy", COMPUCOLOR_FLOPPY);
	device.option_add("floppy_4ph", COMPUCOLOR_FLOPPY_4PH);
}


//-------------------------------------------------
//  FLOPPY_FORMATS( floppy_formats )
//-------------------------------------------------

void compucolor_floppy_device::floppy_formats(format_registration &fr)
{
	fr.add_fm_containers();
	fr.add(FLOPPY_CCVF_FORMAT);
}


//-------------------------------------------------
//  SLOT_INTERFACE( compucolor_floppies )
//-------------------------------------------------

static void compucolor_floppies(device_slot_interface &device)
{
	device.option_add_internal("525sssd", FLOPPY_525_COMPUCOLOR);
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
	: compucolor_floppy_device(mconfig, COMPUCOLOR_FLOPPY, tag, owner, clock, false)
{
}

compucolor_floppy_device::compucolor_floppy_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, bool four_phase)
	: device_t(mconfig, type, tag, owner, clock)
	, device_compucolor_floppy_port_interface(mconfig, *this)
	, m_floppy(*this, "floppy")
	, m_four_phase(four_phase)
	, m_rw(1)
	, m_stp(1)
	, m_step(0)
	, m_sel(1)
	, m_txd(true)
	, m_rxd(true)
	, m_writing(false)
	, m_period(attotime::from_hz(9600*8))
{
	m_owner = dynamic_cast<compucolor_floppy_port_device *>(this->owner());
}

compucolor_floppy_4ph_device::compucolor_floppy_4ph_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: compucolor_floppy_device(mconfig, COMPUCOLOR_FLOPPY_4PH, tag, owner, clock, true)
{
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
	m_timer = timer_alloc(FUNC(compucolor_floppy_device::rxd_tick), this);
	m_motor_timer = timer_alloc(FUNC(compucolor_floppy_device::motor_off), this);

	// state saving
	save_item(NAME(m_rw));
	save_item(NAME(m_stp));
	save_item(NAME(m_step));
	save_item(NAME(m_sel));
	save_item(NAME(m_txd));
	save_item(NAME(m_rxd));
	save_item(NAME(m_writing));
	save_item(NAME(m_last_edge));
	save_item(NAME(m_pos));
}


//-------------------------------------------------
//  rxd_tick -
//-------------------------------------------------

TIMER_CALLBACK_MEMBER(compucolor_floppy_device::rxd_tick)
{
	read_sync();
}


//-------------------------------------------------
//  motor_off -
//-------------------------------------------------

TIMER_CALLBACK_MEMBER(compucolor_floppy_device::motor_off)
{
	floppy_image_device *floppy = m_floppy->get_device();
	if (floppy != nullptr)
		floppy->mon_w(1);
}


//-------------------------------------------------
//  read_sync -
//-------------------------------------------------

void compucolor_floppy_device::read_sync()
{
	floppy_image_device *floppy = m_floppy->get_device();

	if (m_sel || m_rw || floppy == nullptr)
	{
		m_timer->adjust(attotime::never);
		return;
	}

	attotime const now = machine().time();
	attotime const idle = m_period * 10;
	attotime edge;

	// the reproduce amplifier latches the polarity of each flux reversal; a gap longer than any
	// run of spaces in an 8N1 frame can only be a mark, which recovers the polarity flux timing lacks
	for (;;)
	{
		edge = floppy->get_next_transition(m_pos);

		if (edge.is_never() || edge > now)
			break;

		if (edge - m_last_edge >= idle)
			m_rxd = true;

		m_rxd = !m_rxd;
		m_last_edge = edge;
		m_pos = edge;
	}

	m_pos = now;

	if (now - m_last_edge >= idle)
		m_rxd = true;
	else if (edge.is_never() || edge > m_last_edge + idle)
		edge = m_last_edge + idle;

	output_rxd(m_rxd);

	m_timer->adjust(edge.is_never() ? attotime::never : edge - now);
}


//-------------------------------------------------
//  input_txd -
//-------------------------------------------------

void compucolor_floppy_device::input_txd(int state)
{
	floppy_image_device *floppy = m_floppy->get_device();

	if (m_writing && floppy != nullptr && bool(state) != m_txd)
		floppy->write_flux_change(machine().time());

	m_txd = state;
}


//-------------------------------------------------
//  update_gates -
//-------------------------------------------------

void compucolor_floppy_device::update_gates()
{
	floppy_image_device *floppy = m_floppy->get_device();
	bool const writing = !m_sel && m_rw;

	if (writing != m_writing && floppy != nullptr)
	{
		if (writing)
		{
			floppy->write_start(machine().time());

			if (m_txd != m_rxd)
				floppy->write_flux_change(machine().time());
		}
		else
		{
			floppy->write_end(machine().time());
		}
	}

	m_writing = writing;

	m_pos = machine().time();
	read_sync();
}


//-------------------------------------------------
//  rw_w -
//-------------------------------------------------

void compucolor_floppy_device::rw_w(int state)
{
	read_sync();

	if (!m_rw && state)
	{
		output_rxd(1);
	}

	m_rw = state;

	update_gates();
}


//-------------------------------------------------
//  stepper_w -
//-------------------------------------------------

void compucolor_floppy_device::stepper_w(uint8_t data)
{
	if (m_sel)
		return;

	floppy_image_device *floppy = m_floppy->get_device();
	if (floppy == nullptr)
		return;

	// a pattern that energises no single winding leaves the rotor where it is
	uint8_t const phase = (data == 1 || data == 2 || data == 4 || (m_four_phase && data == 0)) ? data : m_stp;
	uint8_t const next = (m_stp == 4) ? (m_four_phase ? 0 : 1) : (m_stp == 0) ? 1 : (m_stp << 1);
	uint8_t const prev = (m_stp == 1) ? (m_four_phase ? 0 : 4) : (m_stp == 0) ? 4 : (m_stp >> 1);
	int const steps_per_track = m_four_phase ? 2 : 1;
	int const step = m_step;

	if (phase != m_stp && phase == next)
		m_step++;
	else if (phase != m_stp && phase == prev && m_step > 0)
		m_step--;

	m_stp = phase;

	int const cyl = m_step / steps_per_track;

	if (floppy->get_cyl() != cyl)
	{
		floppy->dir_w((floppy->get_cyl() > cyl) ? 1 : 0);
		floppy->stp_w(0);
		floppy->stp_w(1);

		if (floppy->get_cyl() != cyl)
			m_step = step;
	}

	read_sync();
}


//-------------------------------------------------
//  select_w -
//-------------------------------------------------

void compucolor_floppy_device::select_w(int state)
{
	floppy_image_device *floppy = m_floppy->get_device();

	if (!state)
	{
		m_motor_timer->adjust(attotime::never);

		if (floppy != nullptr)
			floppy->mon_w(0);
	}
	else if (!m_sel)
	{
		// the motor runs on for the 10uF/100K hold time after the drive is deselected
		m_motor_timer->adjust(attotime::from_msec(680));
	}

	if (!m_sel && state)
	{
		output_rxd(1);
	}

	m_sel = state;

	update_gates();
}
