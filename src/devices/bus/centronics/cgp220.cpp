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
	m_printer->busy_handler().set(FUNC(centronics_cgp220_device::output_busy));
	m_printer->ack_handler().set(FUNC(centronics_cgp220_device::output_ack));
	m_printer->perror_handler().set(FUNC(centronics_cgp220_device::output_perror));
	m_printer->fault_handler().set(FUNC(centronics_cgp220_device::output_fault));
	m_printer->select_handler().set(FUNC(centronics_cgp220_device::output_select));
}


void centronics_cgp220_device::device_start()
{
}


void centronics_cgp220_device::input_strobe(int state) { m_printer->strobe_w(state); }
void centronics_cgp220_device::input_data0(int state) { m_printer->data_bit_w<0>(state); }
void centronics_cgp220_device::input_data1(int state) { m_printer->data_bit_w<1>(state); }
void centronics_cgp220_device::input_data2(int state) { m_printer->data_bit_w<2>(state); }
void centronics_cgp220_device::input_data3(int state) { m_printer->data_bit_w<3>(state); }
void centronics_cgp220_device::input_data4(int state) { m_printer->data_bit_w<4>(state); }
void centronics_cgp220_device::input_data5(int state) { m_printer->data_bit_w<5>(state); }
void centronics_cgp220_device::input_data6(int state) { m_printer->data_bit_w<6>(state); }
void centronics_cgp220_device::input_data7(int state) { m_printer->data_bit_w<7>(state); }
