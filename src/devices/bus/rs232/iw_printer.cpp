// license:BSD-3-Clause
// copyright-holders:smf

/**************************************************************************

    Simple printer emulation

    This allows capturing the byte stream to a file.

**************************************************************************/

#include "emu.h"
#include "printer.h"
#include "iw_printer.h"




DEFINE_DEVICE_TYPE(APPLE_IMAGEWRITER_PRINTER, apple_imagewriter_printer_device, "apple_imagewriter", "apple imagewriter printer")

apple_imagewriter_printer_device::apple_imagewriter_printer_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: apple_imagewriter_printer_device(mconfig, APPLE_IMAGEWRITER_PRINTER, tag, owner, clock)
{
}

apple_imagewriter_printer_device::apple_imagewriter_printer_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, type, tag, owner, clock),
	device_serial_interface(mconfig, *this),
	device_rs232_port_interface(mconfig, *this),
	m_initial_rx_state(1),
	m_printer(*this, "printer"),
	m_maincpu(*this, "maincpu"),
	m_uart(*this, "uart"),
	m_8155head(*this, "8155head"),
	m_8155switch(*this, "8155switch"),
	m_bitmap_printer(*this, "bitmap_printer"),

	m_rs232_rxbaud(*this, "RS232_RXBAUD"),
	m_rs232_databits(*this, "RS232_DATABITS"),
	m_rs232_parity(*this, "RS232_PARITY"),
	m_rs232_stopbits(*this, "RS232_STOPBITS")
{
}

void apple_imagewriter_printer_device::device_add_mconfig(machine_config &config)
{
	PRINTER(config, m_printer, 0);
	m_printer->online_callback().set(FUNC(apple_imagewriter_printer_device::printer_online));

	// basic machine hardware 
	i8085a_cpu_device &cpu(I8085A(config, m_maincpu, 9.8304_MHz_XTAL / 2));  // aka 4.9152_MHz_XTAL
	cpu.set_addrmap(AS_PROGRAM, &apple_imagewriter_printer_device::mem_map);
	cpu.set_addrmap(AS_IO,      &apple_imagewriter_printer_device::io_map);

	[[maybe_unused]] i8155_device &io_head  (I8155(config, m_8155head,   0));  
	[[maybe_unused]] i8155_device &io_switch(I8155(config, m_8155switch, 0));
	[[maybe_unused]] i8251_device &uart(I8251(config, m_uart,0));
	[[maybe_unused]] bitmap_printer_device &bitmap_printer(BITMAP_PRINTER(config, m_bitmap_printer,PAPER_WIDTH, PAPER_HEIGHT));    
}


void apple_imagewriter_printer_device::mem_map(address_map &map)
{
    map(0x0000, 0x3fff).rom().region("maincpu", 0).nopw();  // main rom
    map(0x7000, 0x70ff).ram().mirror(0x700);  // 256 bytes on 8155
    map(0x7800, 0x78ff).ram().mirror(0x700);  // 256 bytes on 8155
    map(0x8000, 0x87ff).ram();  // 2k 6116
    map(0x8800, 0x8fff).ram();  // 2k 6116
    map(0x9000, 0x97ff).ram();  // 2k 6116
}

void apple_imagewriter_printer_device::io_map(address_map &map)
{
    map(0x60, 0x60).rw(m_uart, FUNC(i8251_device::data_r),   FUNC(i8251_device::data_w));
    map(0x61, 0x61).rw(m_uart, FUNC(i8251_device::status_r), FUNC(i8251_device::control_w));


    map(0x70, 0x77).rw(m_8155head, FUNC(i8155_device::io_r), FUNC(i8155_device::io_w));
    map(0x78, 0x7f).rw(m_8155switch, FUNC(i8155_device::io_r), FUNC(i8155_device::io_w));
}




ROM_START(apple_imagewriter_printer)
        ROM_REGION(0x4000, "maincpu", 0)
        ROM_LOAD("8510B-APL.bin", 0x0000, 0x4000, CRC(d5b40497) SHA1(1602786af58b788f239591edfb0eb3730188d6a3))
ROM_END




const tiny_rom_entry *apple_imagewriter_printer_device::device_rom_region() const
{
        return ROM_NAME(apple_imagewriter_printer);
}

static INPUT_PORTS_START(serial_printer)
	PORT_RS232_BAUD("RS232_RXBAUD", RS232_BAUD_9600, "RX Baud", apple_imagewriter_printer_device, update_serial)
	PORT_RS232_DATABITS("RS232_DATABITS", RS232_DATABITS_8, "Data Bits", apple_imagewriter_printer_device, update_serial)
	PORT_RS232_PARITY("RS232_PARITY", RS232_PARITY_NONE, "Parity", apple_imagewriter_printer_device, update_serial)
	PORT_RS232_STOPBITS("RS232_STOPBITS", RS232_STOPBITS_1, "Stop Bits", apple_imagewriter_printer_device, update_serial)
INPUT_PORTS_END



ioport_constructor apple_imagewriter_printer_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(serial_printer);
}

void apple_imagewriter_printer_device::device_start()
{
}



WRITE_LINE_MEMBER(apple_imagewriter_printer_device::update_serial)
{
	int startbits = 1;
	int databits = convert_databits(m_rs232_databits->read());
	parity_t parity = convert_parity(m_rs232_parity->read());
	stop_bits_t stopbits = convert_stopbits(m_rs232_stopbits->read());

	set_data_frame(startbits, databits, parity, stopbits);

	int rxbaud = convert_baud(m_rs232_rxbaud->read());
	set_rcv_rate(rxbaud);

	// TODO: make this configurable
	output_rxd(m_initial_rx_state);
	output_dcd(0);
	output_dsr(0);
	output_cts(0);
}

void apple_imagewriter_printer_device::device_reset()
{
	update_serial(0);
}

WRITE_LINE_MEMBER(apple_imagewriter_printer_device::printer_online)
{
	/// TODO: ?
}

void apple_imagewriter_printer_device::rcv_complete()
{
	receive_register_extract();
	m_printer->output(get_received_char());
}

//DEFINE_DEVICE_TYPE(APPLE_IMAGEWRITER_PRINTER, apple_imagewriter_printer_device, "apple_imagewriter_printer", "Apple Imagewriter Printer")

