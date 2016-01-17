// license:GPL-2.0+
// copyright-holders:Dirk Best
/***************************************************************************

    VTech Laser/VZ Printer Interface

    VTech PI 20
    Dick Smith Electronics X-7320

***************************************************************************/

#include "printer.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type PRINTER_INTERFACE = &device_creator<printer_interface_device>;

//-------------------------------------------------
//  machine_config_additions - device-specific
//  machine configurations
//-------------------------------------------------

static MACHINE_CONFIG_FRAGMENT( printer_interface )
	MCFG_CENTRONICS_ADD("centronics", centronics_devices, "printer")
	MCFG_CENTRONICS_BUSY_HANDLER(WRITELINE(printer_interface_device, busy_w))
	MCFG_CENTRONICS_OUTPUT_LATCH_ADD("latch", "centronics")
MACHINE_CONFIG_END

machine_config_constructor printer_interface_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( printer_interface );
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  printer_interface_device - constructor
//-------------------------------------------------

printer_interface_device::printer_interface_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, PRINTER_INTERFACE, "Laser/VZ Printer Interface", tag, owner, clock, "printer", __FILE__),
	device_ioexp_interface(mconfig, *this),
	m_centronics(*this, "centronics"),
	m_latch(*this, "latch"),
	m_centronics_busy(0)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void printer_interface_device::device_start()
{
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void printer_interface_device::device_reset()
{
	m_slot->m_io->install_read_handler(0x00, 0x00, read8_delegate(FUNC(printer_interface_device::busy_r), this));
	m_slot->m_io->install_write_handler(0x0d, 0x0d, write8_delegate(FUNC(printer_interface_device::strobe_w), this));
	m_slot->m_io->install_write_handler(0x0e, 0x0e, write8_delegate(FUNC(output_latch_device::write), m_latch.target()));
}


//**************************************************************************
//  IMPLEMENTATION
//**************************************************************************

WRITE_LINE_MEMBER( printer_interface_device::busy_w )
{
	m_centronics_busy = state;
}

READ8_MEMBER( printer_interface_device::busy_r )
{
	return 0xfe | m_centronics_busy;
}

WRITE8_MEMBER( printer_interface_device::strobe_w )
{
	m_centronics->write_strobe(1);
	m_centronics->write_strobe(0);
}
