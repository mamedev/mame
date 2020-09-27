// license:BSD-3-Clause
// copyright-holders:smf
#include "emu.h"
#include "rs232luaprinter.h"

serial_luaprinter_device::serial_luaprinter_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) : 
	device_t(mconfig, SERIAL_LUAPRINTER, tag, owner, clock),
	device_serial_interface(mconfig, *this),
	device_rs232_port_interface(mconfig, *this),
	device_luaprinter_interface(mconfig, *this),
	m_printer(*this, "printer"),
	m_screen(*this, "screen"),
	m_rs232_rxbaud(*this, "RS232_RXBAUD"),
	m_rs232_startbits(*this, "RS232_STARTBITS"),
	m_rs232_databits(*this, "RS232_DATABITS"),
	m_rs232_parity(*this, "RS232_PARITY"),
	m_rs232_stopbits(*this, "RS232_STOPBITS")
{
}

void serial_luaprinter_device::device_add_mconfig(machine_config &config)
{
	PRINTER(config, m_printer, 0);
	m_printer->online_callback().set(FUNC(serial_luaprinter_device::printer_online));
	/* video hardware (simulates paper) */
    screen_device &screen(SCREEN(config, m_screen, SCREEN_TYPE_RASTER));
    screen.set_refresh_hz(60);
    screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
    screen.set_size(PAPER_WIDTH, PAPER_SCREEN_HEIGHT);
    screen.set_visarea(0, PAPER_WIDTH-1, 0, PAPER_SCREEN_HEIGHT-1);
    screen.set_screen_update(FUNC(device_luaprinter_interface::lp_screen_update));
}

static INPUT_PORTS_START(serial_luaprinter)
	PORT_RS232_BAUD("RS232_RXBAUD", RS232_BAUD_9600, "RX Baud", serial_luaprinter_device, update_serial)
	PORT_RS232_STARTBITS("RS232_STARTBITS", RS232_STARTBITS_1, "Start Bits", serial_luaprinter_device, update_serial)
	PORT_RS232_DATABITS("RS232_DATABITS", RS232_DATABITS_8, "Data Bits", serial_luaprinter_device, update_serial)
	PORT_RS232_PARITY("RS232_PARITY", RS232_PARITY_NONE, "Parity", serial_luaprinter_device, update_serial)
	PORT_RS232_STOPBITS("RS232_STOPBITS", RS232_STOPBITS_1, "Stop Bits", serial_luaprinter_device, update_serial)

	PORT_START("luaprinter_baud")
    PORT_DIPNAME(0xff, 0x07, "RS232 Baud") // default to 9600 baud
    PORT_DIPLOCATION("DIP:1,2,3,4,5,6,7,8")
	PORT_DIPSETTING(0x00, "RS232_BAUD_110 (0x00)")
	PORT_DIPSETTING(0x01, "RS232_BAUD_150 (0x01)")
	PORT_DIPSETTING(0x02, "RS232_BAUD_300 (0x02)")
	PORT_DIPSETTING(0x03, "RS232_BAUD_600 (0x03)")
	PORT_DIPSETTING(0x04, "RS232_BAUD_1200 (0x04)")
	PORT_DIPSETTING(0x05, "RS232_BAUD_2400 (0x05)")
	PORT_DIPSETTING(0x06, "RS232_BAUD_4800 (0x06)")
	PORT_DIPSETTING(0x07, "RS232_BAUD_9600 (0x07)")
	PORT_DIPSETTING(0x08, "RS232_BAUD_14400 (0x08)")
	PORT_DIPSETTING(0x09, "RS232_BAUD_19200 (0x09)")
	PORT_DIPSETTING(0x0a, "RS232_BAUD_28800 (0x0a)")
	PORT_DIPSETTING(0x0b, "RS232_BAUD_38400 (0x0b)")
	PORT_DIPSETTING(0x0c, "RS232_BAUD_57600 (0x0c)")
	PORT_DIPSETTING(0x0d, "RS232_BAUD_115200 (0x0d)")
INPUT_PORTS_END

ioport_constructor serial_luaprinter_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(serial_luaprinter);
}

void serial_luaprinter_device::device_start()
{
	m_bitmap.allocate(PAPER_WIDTH, PAPER_HEIGHT);
	m_bitmap.fill(0xffffff);
	initluaprinter(m_bitmap);
}

void serial_luaprinter_device::device_stop()
{
	if (m_lp_pagedirty)
		savepage();
}


WRITE_LINE_MEMBER(serial_luaprinter_device::update_serial)
{
	int startbits = convert_startbits(m_rs232_startbits->read());
	int databits = convert_databits(m_rs232_databits->read());
	parity_t parity = convert_parity(m_rs232_parity->read());
	stop_bits_t stopbits = convert_stopbits(m_rs232_stopbits->read());
	set_data_frame(startbits, databits, parity, stopbits);

	int rxbaud = convert_baud(ioport("luaprinter_baud")->read());
	set_rcv_rate(rxbaud);

	// TODO: make this configurable
	output_rxd(1);
	output_dcd(0);
	output_dsr(0);
	output_cts(0);
}

void serial_luaprinter_device::device_reset()
{
	update_serial(0);
}

WRITE_LINE_MEMBER(serial_luaprinter_device::printer_online)
{
	/// TODO: ?
}

void serial_luaprinter_device::rcv_complete()
{
	receive_register_extract();
	u8 recchar = get_received_char();
	m_printer->output(recchar);
	putnextchar(recchar);
}


DEFINE_DEVICE_TYPE(SERIAL_LUAPRINTER, serial_luaprinter_device, "serial_luaprinter", "Serial Printer")
