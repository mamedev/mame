// license:BSD-3-Clause
// copyright-holders:smf

/**************************************************************************

    Apple ImageWriter Printer

    Simple printer emulation

    This allows capturing the byte stream to a file.

**************************************************************************/

#include "emu.h"
#include "printer.h"
#include "imagewriter_printer.h"
#include <bitset>

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
	m_count(*this, "74163count"),
	m_bitmap_printer(*this, "bitmap_printer"),
	m_pf_stepper(*this, "pf_stepper"),
	m_cr_stepper(*this, "cr_stepper"),

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

//	m_maincpu->set_clk_out(this, FUNC(apple_imagewriter_printer_device::testing_timerout));  //not how that works, just sets clock rate of device


	TTL74163(config, m_count, 0);
	m_maincpu->set_clk_out(m_count, FUNC(ttl74163_device::set_unscaled_clock_int));



	[[maybe_unused]] i8155_device &io_head  (I8155(config, m_8155head,   1E6));
	io_head.in_pa_callback().set(FUNC(apple_imagewriter_printer_device::head_pa_r));
	io_head.in_pb_callback().set(FUNC(apple_imagewriter_printer_device::head_pb_r));
	io_head.in_pc_callback().set(FUNC(apple_imagewriter_printer_device::head_pc_r));
	io_head.out_pa_callback().set(FUNC(apple_imagewriter_printer_device::head_pa_w));
	io_head.out_pb_callback().set(FUNC(apple_imagewriter_printer_device::head_pb_w));
	io_head.out_pc_callback().set(FUNC(apple_imagewriter_printer_device::head_pc_w));
	io_head.out_to_callback().set(FUNC(apple_imagewriter_printer_device::head_to));
	
	
	[[maybe_unused]] i8155_device &io_switch(I8155(config, m_8155switch, 0));
	io_switch.in_pa_callback().set(FUNC(apple_imagewriter_printer_device::switch_pa_r));
	io_switch.in_pb_callback().set(FUNC(apple_imagewriter_printer_device::switch_pb_r));
	io_switch.in_pc_callback().set(FUNC(apple_imagewriter_printer_device::switch_pc_r));
	io_switch.out_pa_callback().set(FUNC(apple_imagewriter_printer_device::switch_pa_w));
	io_switch.out_pb_callback().set(FUNC(apple_imagewriter_printer_device::switch_pb_w));
	io_switch.out_pc_callback().set(FUNC(apple_imagewriter_printer_device::switch_pc_w));
	io_switch.out_to_callback().set(FUNC(apple_imagewriter_printer_device::switch_to));

	I8251(config, m_uart, 0);
	m_uart->rxrdy_handler().set(FUNC(apple_imagewriter_printer_device::rxrdy_handler));

/*
	[[maybe_unused]] i8251_device &uart(I8251(config, m_uart, 0));
	uart.rxrdy_handler().set(FUNC(apple_imagewriter_printer_device::rxrdy_handler));
*/
		
	// auto rxrdy_handler() { return m_rxrdy_handler.bind(); }

	
	//[[maybe_unused]] bitmap_printer_device &bitmap_printer(BITMAP_PRINTER(config, m_bitmap_printer, PAPER_WIDTH, PAPER_HEIGHT));

	BITMAP_PRINTER(config, m_bitmap_printer, PAPER_WIDTH, PAPER_HEIGHT);
	
	
	STEPPER(config, m_pf_stepper, (uint8_t) 0xa);
	STEPPER(config, m_cr_stepper, (uint8_t) 0xa);

	m_pf_stepper->optic_handler().set(FUNC(apple_imagewriter_printer_device::optic_handler));	
	m_cr_stepper->optic_handler().set(FUNC(apple_imagewriter_printer_device::optic_handler));	

/*
	[[maybe_unused]] stepper_device& pf_step(STEPPER(config, m_pf_stepper, (uint8_t) 0xa));
	[[maybe_unused]] stepper_device& cr_step(STEPPER(config, m_cr_stepper, (uint8_t) 0xa));

	pf_step.optic_handler().set(FUNC(apple_imagewriter_printer_device::optic_handler));	
	cr_step.optic_handler().set(FUNC(apple_imagewriter_printer_device::optic_handler));
*/
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
	ROM_LOAD("m8510-apl.ic21", 0x0000, 0x4000, CRC(d5b40497) SHA1(1602786af58b788f239591edfb0eb3730188d6a3))
ROM_END

const tiny_rom_entry *apple_imagewriter_printer_device::device_rom_region() const
{
	return ROM_NAME(apple_imagewriter_printer);
}

static INPUT_PORTS_START( apple_imagewriter )
	PORT_RS232_BAUD("RS232_RXBAUD", RS232_BAUD_9600, "RX Baud", apple_imagewriter_printer_device, update_serial)
	PORT_RS232_DATABITS("RS232_DATABITS", RS232_DATABITS_8, "Data Bits", apple_imagewriter_printer_device, update_serial)
	PORT_RS232_PARITY("RS232_PARITY", RS232_PARITY_NONE, "Parity", apple_imagewriter_printer_device, update_serial)
	PORT_RS232_STOPBITS("RS232_STOPBITS", RS232_STOPBITS_1, "Stop Bits", apple_imagewriter_printer_device, update_serial)

	// Buttons on printer
	PORT_START("ONLINE")
//	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("On Line") PORT_CODE(KEYCODE_0_PAD) PORT_CHANGED_MEMBER(DEVICE_SELF, epson_lx810l_device, online_sw, 0)
	PORT_START("FORMFEED")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Form Feed") PORT_CODE(KEYCODE_7_PAD)
	PORT_START("LINEFEED")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Line Feed") PORT_CODE(KEYCODE_9_PAD)
	PORT_START("LOADEJECT")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Load/Eject") PORT_CODE(KEYCODE_1_PAD)
	PORT_START("PAPEREND")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Paper End Sensor") PORT_CODE(KEYCODE_6_PAD)

	// DIPSW1 
	PORT_START("DIPSW1")

	PORT_DIPNAME(0x01, 0x01, "Character spacing")
	PORT_DIPLOCATION("DIP:1")
	PORT_DIPSETTING(0x01, "12 cpi") /* default */
	PORT_DIPSETTING(0x00, "10 cpi")

	PORT_DIPNAME(0x02, 0x00, "Shape of zero")
	PORT_DIPLOCATION("DIP:2")
	PORT_DIPSETTING(0x02, "Slashed")
	PORT_DIPSETTING(0x00, "Not slashed") /* default */

	PORT_DIPNAME(0x0c, 0x08, "Page length")
	PORT_DIPLOCATION("DIP:3,4")
	PORT_DIPSETTING(0x00, "11 inches")
	PORT_DIPSETTING(0x04, "12 inches")
	PORT_DIPSETTING(0x08, "8.5 inches") /* default */
	PORT_DIPSETTING(0x0c, "11.7 inches")

	PORT_DIPNAME(0x10, 0x10, "Character table")
	PORT_DIPLOCATION("DIP:5")
	PORT_DIPSETTING(0x10, "Graphics") /* default */
	PORT_DIPSETTING(0x00, "Italics")

	PORT_DIPNAME(0xe0, 0xe0, "International characters and PC selection")
	PORT_DIPLOCATION("DIP:6,7,8")
	PORT_DIPSETTING(0xe0, "United States") /* default */
	PORT_DIPSETTING(0x60, "France")
	PORT_DIPSETTING(0xa0, "Germany")
	PORT_DIPSETTING(0x20, "United Kingdom")
	PORT_DIPSETTING(0xc0, "Denmark")
	PORT_DIPSETTING(0x40, "Sweden")
	PORT_DIPSETTING(0x80, "Italy")
	PORT_DIPSETTING(0x00, "Spain")

	// DIPSW2
	PORT_START("DIPSW2")

	PORT_DIPNAME(0x01, 0x01, "Short tear-off")
	PORT_DIPLOCATION("DIP:1")
	PORT_DIPSETTING(0x01, "Invalid") /* default */
	PORT_DIPSETTING(0x00, "Valid")

	PORT_DIPNAME(0x02, 0x00, "Cut-sheet feeder mode")
	PORT_DIPLOCATION("DIP:2")
	PORT_DIPSETTING(0x02, "ON")
	PORT_DIPSETTING(0x00, "OFF") /* default */

	PORT_DIPNAME(0x04, 0x00, "Skip-over-perforation")
	PORT_DIPLOCATION("DIP:3")
	PORT_DIPSETTING(0x04, "ON")
	PORT_DIPSETTING(0x00, "OFF") /* default */

	PORT_DIPNAME(0x08, 0x00, "Auto line feed")
	PORT_DIPLOCATION("DIP:4")
	PORT_DIPSETTING(0x08, "ON")
	PORT_DIPSETTING(0x00, "OFF") /* default */

INPUT_PORTS_END


//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor apple_imagewriter_printer_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( apple_imagewriter );
}


//$70 HEAD

uint8_t apple_imagewriter_printer_device::head_pa_r(offs_t offset) { return 0; }
//void apple_imagewriter_printer_device::head_pa_w(offs_t offset, uint8_t data) 
void apple_imagewriter_printer_device::head_pa_w(uint8_t data) 
{
	printf("8155 HEAD PORT_A_WRITE %x   TIME = %f  %s\n",data, machine().time().as_double(), machine().describe_context().c_str());
}
uint8_t apple_imagewriter_printer_device::head_pb_r(offs_t offset) 
{ 
	u8 data = 0;
	printf("8155 HEAD PORT_B_READ %x   TIME = %f  %s\n",data, machine().time().as_double(), machine().describe_context().c_str());
	return data;
}

void apple_imagewriter_printer_device::head_pb_w(uint8_t data) {
	printf("8155 HEAD PORT_B_WRITE %x   TIME = %f  %s\n",data, machine().time().as_double(), machine().describe_context().c_str());
	update_cr_stepper(BIT(data ^ 0xff, 1, 4));

}

uint8_t apple_imagewriter_printer_device::head_pc_r(offs_t offset) 
{
	u8 data = 0;
	printf("8155 HEAD PORT_C_READ %x   TIME = %f  %s\n",data, machine().time().as_double(), machine().describe_context().c_str());
	return 0; 
}

void apple_imagewriter_printer_device::head_pc_w(uint8_t data) 
{

	printf("8155 HEAD PORT_C_WRITE %x   TIME = %f  %s\n",data, machine().time().as_double(), machine().describe_context().c_str());
	update_pf_stepper(BIT(data ^ 0xff, 1, 4));
	if (!BIT(data,0)) 
	{
		m_ic17_flipflop = 0;
		m_maincpu->set_input_line(I8085_RST55_LINE, m_ic17_flipflop);  // ASSERT_LINE, CLEAR_LINE
	}
}

void apple_imagewriter_printer_device::head_to(uint8_t data) 
{
	// model the IC17 flip flop part
	printf("8155 HEAD_TO %x   TIME = %f\n",data, machine().time().as_double());
	if (m_head_to_last == 0 && data)
	{
		m_ic17_flipflop = 1;
	}
	m_maincpu->set_input_line(I8085_RST55_LINE, m_ic17_flipflop);  // ASSERT_LINE, CLEAR_LINE
	m_head_to_last = data;
}


// $78 SWITCHES

uint8_t apple_imagewriter_printer_device::switch_pa_r(offs_t offset) { return 0; }
void apple_imagewriter_printer_device::switch_pa_w(uint8_t data) 
{
	printf("8155 SWITCH PORT_A_WRITE %x   TIME = %f  %s\n",data, machine().time().as_double(), machine().describe_context().c_str());
}
uint8_t apple_imagewriter_printer_device::switch_pb_r(offs_t offset) { return 0; }
void apple_imagewriter_printer_device::switch_pb_w(uint8_t data) 
{
	printf("8155 SWTICH PORT_B_WRITE %x   TIME = %f  %s\n",data, machine().time().as_double(), machine().describe_context().c_str());
}
uint8_t apple_imagewriter_printer_device::switch_pc_r(offs_t offset) { return 0; }
void apple_imagewriter_printer_device::switch_pc_w(uint8_t data) 

{
	printf("8155 SWTICH PORT_C_WRITE %x   TIME = %f  %s\n",data, machine().time().as_double(), machine().describe_context().c_str());
}

void apple_imagewriter_printer_device::switch_to(uint8_t data) 
{
	printf("8155 SWITCH_TO %x   TIME = %f\n",data, machine().time().as_double());
}




//-------------------------------------------------
//    Update Printhead
//-------------------------------------------------

void apple_imagewriter_printer_device::update_printhead(uint8_t headbits)
{

}





//-------------------------------------------------
//    Update Stepper and return delta
//-------------------------------------------------


int apple_imagewriter_printer_device::update_stepper_delta(stepper_device * stepper, uint8_t pattern, const char * name)
{
	printf("UPDATE STEPPER DELTA %x %s\n",pattern, name);
//	printf("UPDATE STEPPER STEPPER= %p\n",stepper);
	int lastpos = stepper->get_absolute_position();
	stepper->update(bitswap<4>(pattern, 3, 2, 1, 0));  // drive pattern is the "standard" reel pattern when bits 1,2 swapped
//	stepper->update(pattern);
	int delta = stepper->get_absolute_position() - lastpos;
	return delta;
}

//-------------------------------------------------
//    Update Paper Feed Stepper
//-------------------------------------------------

void apple_imagewriter_printer_device::update_pf_stepper(uint8_t vstepper)
{
//	printf("UPDATE STEPPER DELTA 0 %x\n",vstepper);
	
	int delta = update_stepper_delta(m_pf_stepper, vstepper, "PF");

//	printf("UPDATE STEPPER DELTA 1 %x\n",vstepper);
	
	if (delta > 0)
	{
		m_ypos += delta; // move down

		if (newpageflag == 1)
		{
			m_ypos = 10;  // lock to the top of page until we seek horizontally
		}
		if (y_pixel_coord(m_ypos) > m_bitmap_printer->get_bitmap().height() - 50)  // i see why it's failing
			// if we are within 50 pixels of the bottom of the page we will
			// write the page to a file, then erase the top part of the page
			// so we can still see the last page printed.
		{
			// clear paper to bottom from current position
			m_bitmap_printer->bitmap_clear_band(y_pixel_coord(m_ypos) + 7, PAPER_HEIGHT - 1, rgb_t::white());

			// save a snapshot with the slot and page as part of the filename
			m_bitmap_printer->write_snapshot_to_file(
						std::string("silentype"),
						std::string("silentype_") +
//                      m_bitmap_printer->getprintername() +
						m_bitmap_printer->get_session_time_device()->getprintername() +
						"_page_" +
						m_bitmap_printer->padzeroes(std::to_string(page_count++),3) +
						".png");

			newpageflag = 1;
			// clear page down to visible area, starting from the top of page
			m_bitmap_printer->bitmap_clear_band(0, PAPER_HEIGHT - 1 - PAPER_SCREEN_HEIGHT, rgb_t::white());

			m_ypos = 10;
		}
		// clear page down to visible area
		m_bitmap_printer->bitmap_clear_band(y_pixel_coord(m_ypos) + distfrombottom, std::min(y_pixel_coord(m_ypos) + distfrombottom+30, PAPER_HEIGHT - 1), rgb_t::white());

	}
	else if (delta < 0) // we are moving up the page
	{
		m_ypos += delta;
		if (m_ypos < 0) m_ypos = 0;  // don't go backwards past top of page
	}

	m_bitmap_printer->setheadpos(x_pixel_coord(m_xpos), y_pixel_coord(m_ypos));
}

//-------------------------------------------------
//    Update Carriage Stepper
//-------------------------------------------------

void apple_imagewriter_printer_device::update_cr_stepper(uint8_t hstepper)
{
	int delta = update_stepper_delta(m_cr_stepper, hstepper, "CR");

//  printf("CR STEPPER pat = %d, delta = %d, m_xpos = %d\n",hstepper,delta,m_xpos);

	if (delta != 0)
	{
		newpageflag = 0;

		if (delta > 0)
		{
			m_xpos += delta; xdirection = 1;
		}
		else if (delta < 0)
		{
			m_xpos += delta; xdirection = -1;
			//if (m_xpos < 0) m_xpos = 0;
		}
	}

	m_bitmap_printer->setheadpos(x_pixel_coord(m_xpos), y_pixel_coord(m_ypos));
}








void apple_imagewriter_printer_device::device_start()
{
//	update_pf_stepper(0);   // STOPS THE SEGFAULT
//	update_cr_stepper(0);
//	update_printhead(0);
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







