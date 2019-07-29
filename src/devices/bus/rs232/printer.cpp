// license:BSD-3-Clause
// copyright-holders:smf
#include "emu.h"
#include "printer.h"

serial_printer_device::serial_printer_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, SERIAL_PRINTER, tag, owner, clock),
	device_serial_interface(mconfig, *this),
	device_rs232_port_interface(mconfig, *this),
	m_printer(*this, "printer"),
	m_rs232_rxbaud(*this, "RS232_RXBAUD"),
	m_rs232_startbits(*this, "RS232_STARTBITS"),
	m_rs232_databits(*this, "RS232_DATABITS"),
	m_rs232_parity(*this, "RS232_PARITY"),
	m_rs232_stopbits(*this, "RS232_STOPBITS")
{
}

void serial_printer_device::device_add_mconfig(machine_config &config)
{
	PRINTER(config, m_printer, 0);
	m_printer->online_callback().set(FUNC(serial_printer_device::printer_online));
}

static INPUT_PORTS_START(serial_printer)
	PORT_RS232_BAUD("RS232_RXBAUD", RS232_BAUD_9600, "RX Baud", serial_printer_device, update_serial)
	PORT_RS232_STARTBITS("RS232_STARTBITS", RS232_STARTBITS_1, "Start Bits", serial_printer_device, update_serial)
	PORT_RS232_DATABITS("RS232_DATABITS", RS232_DATABITS_8, "Data Bits", serial_printer_device, update_serial)
	PORT_RS232_PARITY("RS232_PARITY", RS232_PARITY_NONE, "Parity", serial_printer_device, update_serial)
	PORT_RS232_STOPBITS("RS232_STOPBITS", RS232_STOPBITS_1, "Stop Bits", serial_printer_device, update_serial)
INPUT_PORTS_END

ioport_constructor serial_printer_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(serial_printer);
}

void serial_printer_device::device_start()
{
}

WRITE_LINE_MEMBER(serial_printer_device::update_serial)
{
	int startbits = convert_startbits(m_rs232_startbits->read());
	int databits = convert_databits(m_rs232_databits->read());
	parity_t parity = convert_parity(m_rs232_parity->read());
	stop_bits_t stopbits = convert_stopbits(m_rs232_stopbits->read());

	set_data_frame(startbits, databits, parity, stopbits);

	int rxbaud = convert_baud(m_rs232_rxbaud->read());
	set_rcv_rate(rxbaud);

	// TODO: make this configurable
	output_rxd(0);
	output_dcd(0);
	output_dsr(0);
	output_cts(0);
}

void serial_printer_device::device_reset()
{
	update_serial(0);
}

WRITE_LINE_MEMBER(serial_printer_device::printer_online)
{
	/// TODO: ?
}

void serial_printer_device::rcv_complete()
{
	receive_register_extract();
	m_printer->output(get_received_char());
}

DEFINE_DEVICE_TYPE(SERIAL_PRINTER, serial_printer_device, "serial_printer", "Serial Printer")
