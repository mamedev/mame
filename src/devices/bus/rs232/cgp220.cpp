// license:BSD-3-Clause
// copyright-holders:tim lindner
/**********************************************************************

    Radio Shack Color Graphics Printer CGP-220, serial interface

    Like other Radio Shack serial printers the busy signal is returned
    on RX. 600 or 2400 baud, 8 data bits, no parity, set by a switch.

**********************************************************************/

#include "emu.h"
#include "cgp220.h"


DEFINE_DEVICE_TYPE(RS232_CGP220, rs232_cgp220_device, "rs232_cgp220", "Radio Shack Color Graphics Printer CGP-220 (serial)")


rs232_cgp220_device::rs232_cgp220_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	device_t(mconfig, RS232_CGP220, tag, owner, clock),
	device_rs232_port_interface(mconfig, *this),
	m_printer(*this, "cgp220")
{
}


void rs232_cgp220_device::device_add_mconfig(machine_config &config)
{
	CGP220(config, m_printer);
	m_printer->set_serial_interface(true);
	m_printer->busy_handler().set(FUNC(rs232_cgp220_device::busy_w));
}


void rs232_cgp220_device::device_start()
{
}


void rs232_cgp220_device::busy_w(int state)
{
	output_rxd(state);
	output_dcd(state);
	output_dsr(state);
	output_cts(state);
}
