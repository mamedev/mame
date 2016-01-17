// license:GPL-2.0+
// copyright-holders:Dirk Best
/***************************************************************************

    EACA Colour Genie Printer Interface EG2012

***************************************************************************/

#include "printer.h"


//**************************************************************************
//  CONSTANTS/MACROS
//**************************************************************************

#define VERBOSE 0


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type CGENIE_PRINTER = &device_creator<cgenie_printer_device>;

//-------------------------------------------------
//  machine_config_additions - device-specific
//  machine configurations
//-------------------------------------------------

static MACHINE_CONFIG_FRAGMENT( cgenie_printer )
	MCFG_CENTRONICS_ADD("centronics", centronics_devices, "printer")
	MCFG_CENTRONICS_BUSY_HANDLER(WRITELINE(cgenie_printer_device, busy_w))
	MCFG_CENTRONICS_PERROR_HANDLER(WRITELINE(cgenie_printer_device, perror_w))
	MCFG_CENTRONICS_SELECT_HANDLER(WRITELINE(cgenie_printer_device, select_w))
	MCFG_CENTRONICS_FAULT_HANDLER(WRITELINE(cgenie_printer_device, fault_w))
	MCFG_CENTRONICS_OUTPUT_LATCH_ADD("latch", "centronics")
MACHINE_CONFIG_END

machine_config_constructor cgenie_printer_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( cgenie_printer );
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  cgenie_printer_device - constructor
//-------------------------------------------------

cgenie_printer_device::cgenie_printer_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, CGENIE_PRINTER, "Printer Interface EG2012", tag, owner, clock, "cgenie_printer", __FILE__),
	device_parallel_interface(mconfig, *this),
	m_centronics(*this, "centronics"),
	m_latch(*this, "latch"),
	m_centronics_busy(0),
	m_centronics_out_of_paper(0),
	m_centronics_unit_sel(1),
	m_centronics_ready(1)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void cgenie_printer_device::device_start()
{
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void cgenie_printer_device::device_reset()
{
}


//**************************************************************************
//  IMPLEMENTATION
//**************************************************************************

WRITE_LINE_MEMBER( cgenie_printer_device::busy_w )
{
	m_centronics_busy = state;
}

WRITE_LINE_MEMBER( cgenie_printer_device::perror_w )
{
	m_centronics_out_of_paper = state;
}

WRITE_LINE_MEMBER( cgenie_printer_device::select_w )
{
	m_centronics_unit_sel = state;
}

WRITE_LINE_MEMBER( cgenie_printer_device::fault_w )
{
	m_centronics_ready = state;
}

void cgenie_printer_device::pa_w(UINT8 data)
{
	if (VERBOSE)
		logerror("%s: pa_w %02x\n", tag().c_str(), data);

	m_latch->write(data);
}

UINT8 cgenie_printer_device::pb_r()
{
	UINT8 data = 0x0f;

	data |= m_centronics_ready << 4;
	data |= m_centronics_unit_sel << 5;
	data |= m_centronics_out_of_paper << 6;
	data |= m_centronics_busy << 7;

	return data;
}

void cgenie_printer_device::pb_w(UINT8 data)
{
	if (VERBOSE)
		logerror("%s: pa_w %02x\n", tag().c_str(), data);

	m_centronics->write_strobe(BIT(data, 0));
}
