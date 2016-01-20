// license:BSD-3-Clause
// copyright-holders:smf
#include "printer.h"

serial_printer_device::serial_printer_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, SERIAL_PRINTER, "Serial Printer", tag, owner, clock, "serial_printer", __FILE__),
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

static MACHINE_CONFIG_FRAGMENT(serial_printer)
	MCFG_DEVICE_ADD("printer", PRINTER, 0)
	MCFG_PRINTER_ONLINE_CB(WRITELINE(serial_printer_device, printer_online))
MACHINE_CONFIG_END

machine_config_constructor serial_printer_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME(serial_printer);
}

static INPUT_PORTS_START(serial_printer)
	MCFG_RS232_BAUD("RS232_RXBAUD", RS232_BAUD_9600, "RX Baud", serial_printer_device, update_serial)
	MCFG_RS232_STARTBITS("RS232_STARTBITS", RS232_STARTBITS_1, "Start Bits", serial_printer_device, update_serial)
	MCFG_RS232_DATABITS("RS232_DATABITS", RS232_DATABITS_8, "Data Bits", serial_printer_device, update_serial)
	MCFG_RS232_PARITY("RS232_PARITY", RS232_PARITY_NONE, "Parity", serial_printer_device, update_serial)
	MCFG_RS232_STOPBITS("RS232_STOPBITS", RS232_STOPBITS_1, "Stop Bits", serial_printer_device, update_serial)
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

void serial_printer_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	device_serial_interface::device_timer(timer, id, param, ptr);
}

void serial_printer_device::rcv_complete()
{
	receive_register_extract();
	m_printer->output(get_received_char());
}

const device_type SERIAL_PRINTER = &device_creator<serial_printer_device>;
