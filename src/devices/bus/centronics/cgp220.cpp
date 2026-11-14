// license:BSD-3-Clause
// copyright-holders:tim lindner
/**********************************************************************

    Radio Shack Color Graphics Printer CGP-220, parallel interface

**********************************************************************/

#include "emu.h"
#include "cgp220.h"


DEFINE_DEVICE_TYPE(CENTRONICS_CGP220, centronics_cgp220_device, "ctronics_cgp220", "Radio Shack Color Graphics Printer CGP-220 (parallel)")


centronics_cgp220_device::centronics_cgp220_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	device_t(mconfig, CENTRONICS_CGP220, tag, owner, clock),
	device_centronics_peripheral_interface(mconfig, *this),
	m_printer(*this, "cgp220")
{
}


void centronics_cgp220_device::device_add_mconfig(machine_config &config)
{
	CGP220(config, m_printer);
	m_printer->set_serial_interface(false);
	m_printer->busy_handler().set([this] (int state) { output_busy(state); });
	m_printer->ack_handler().set([this] (int state) { output_ack(state); });
	m_printer->perror_handler().set([this] (int state) { output_perror(state); });
	m_printer->fault_handler().set([this] (int state) { output_fault(state); });
	m_printer->select_handler().set([this] (int state) { output_select(state); });
}


void centronics_cgp220_device::device_start()
{
}
