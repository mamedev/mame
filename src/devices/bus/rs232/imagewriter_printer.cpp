// license:BSD-3-Clause
// copyright-holders:smf

/**************************************************************************

    Apple ImageWriter Printer

    Simple printer emulation

    This allows capturing the byte stream to a file.


Notes:

    To get it to work, you can experiment with connecting DTR to DSR, CTS, or DSR + CTS.
    The same goes for RTS, you can connect to DSR, CTS or DSR + CTS.
    By default, DTR should connect to DSR and RTS should connect to CTS.

    Also Invert1 will invert the DTR and Invert2 will invert CTS.


    Running the self test:
    You probably won't be able to press keypad 7 quickly enough at the start of mame in order to
    activate the self test.  However, you can reset the printer with keypad 8 while holding down keypad 7
    in order to run the self test.  The self test can be exited by resetting the printer with keypad 8.

    Changes to dip switch settings don't come into effect until resetting the printer (keypad 8) since
    the dip switches are read upon startup.

    Select switch (keypad 0) doesn't seem to stop printing unless you also press the "LF button" (keypad 9)
    momentarily while the printer is printing.

    Pressing the cover switch button (keypad 3) will go immediately into select off mode.

    Activating the Paper end switch (keypad 6 toggle) will stop printing after printing approx 8 lines.

    Can run two imagewriters on the mac512k with ./mame mac512k -rs232b imagewriter -rs232a imagewriter

**************************************************************************/

// A9M0303 is a standard size printer that will accept paper from 3 inch to 10 inches.
//
// A9M0305 is a Wide Carriage Imagewriter printer that will accept paper up to 15″ wide.
//
// ImageWriter Schematics:
//   Apple_Schematics_Imagewriter/APPLE_050-0089-A-1of2.pdf
//   Apple_Schematics_Imagewriter/APPLE_050-0089-A-2of2.pdf
//   "Sams Apple Printer A9M0303.pdf"
//
// Apple Schematic shows J1 jumper that selects between 15" and 8" imagewriter connected to 8085 SID pin 5.
//  8" imagewriter connected to R42 4.7k resistor connected to 5v. (J1 jumper open)
// 15" imagewriter connected to ground. (J1 jumper closed)

#include "emu.h"
#include "printer.h"
#include "imagewriter_printer.h"
#include <bitset>
//#define VERBOSE 1
//#define LOG_OUTPUT_FUNC osd_printf_info
#include "logmacro.h"
#include <math.h>

DEFINE_DEVICE_TYPE(APPLE_IMAGEWRITER_PRINTER, apple_imagewriter_printer_device, "apple_imagewriter", "Apple ImageWriter Printer A9M0303")

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
	m_pulse1(*this, "pulse1"),
	m_pulse2(*this, "pulse2"),
	m_bitmap_printer(*this, "bitmap_printer"),
	m_pf_stepper(*this, "pf_stepper"),
	m_cr_stepper(*this, "cr_stepper"),
	m_timer_rxclock(*this, "rx_clock_8251"),

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
	i8085a_cpu_device &cpu(I8085A(config, m_maincpu, CLK2 ));  // 9.8304_MHz_XTAL / 2 aka 4.9152_MHz_XTAL
	cpu.set_addrmap(AS_PROGRAM, &apple_imagewriter_printer_device::mem_map);
	cpu.set_addrmap(AS_IO,      &apple_imagewriter_printer_device::io_map);

	m_maincpu->out_sod_func().set(FUNC(apple_imagewriter_printer_device::maincpu_out_sod_func));
	m_maincpu->in_sid_func().set(FUNC(apple_imagewriter_printer_device::maincpu_in_sid_func));

	// 74123 for the printhead pulse generation

	TTL74123(config, m_pulse1, 10000, 1000e-12);     // second stage (hooked up to 1 section of 74123)
	m_pulse1->set_connection_type(TTL74123_GROUNDED);
	m_pulse1->set_clear_pin_value(1);  // not clear
	m_pulse1->set_b_pin_value(1);
	m_pulse1->out_cb().set(FUNC(apple_imagewriter_printer_device::pulse1_out_handler));
	TTL74123(config, m_pulse2, 18000, .022E-6);  // first stage  (hooked up to 2 section of 74123)
	m_pulse2->set_connection_type(TTL74123_GROUNDED);
	m_pulse2->out_cb().set(FUNC(apple_imagewriter_printer_device::pulse2_out_handler));
	m_pulse2->set_clear_pin_value(1);

	I8155(config, m_8155head,  CLK1 );
	m_8155head->in_pa_callback() .set(FUNC(apple_imagewriter_printer_device::head_pa_r));
	m_8155head->in_pb_callback() .set(FUNC(apple_imagewriter_printer_device::head_pb_r));
	m_8155head->in_pc_callback() .set(FUNC(apple_imagewriter_printer_device::head_pc_r));
	m_8155head->out_pa_callback().set(FUNC(apple_imagewriter_printer_device::head_pa_w));
	m_8155head->out_pb_callback().set(FUNC(apple_imagewriter_printer_device::head_pb_w));
	m_8155head->out_pc_callback().set(FUNC(apple_imagewriter_printer_device::head_pc_w));
	m_8155head->out_to_callback().set(FUNC(apple_imagewriter_printer_device::head_to));

	I8155(config, m_8155switch, CLK1 / 2 );
	// input clock gets adjusted by PB6 line
	// faster the 8155switch clock is, the faster the printhead moves, either CLK1 / 2 or CLK1 / 8

	m_8155switch->in_pa_callback() .set(FUNC(apple_imagewriter_printer_device::switch_pa_r));
	m_8155switch->in_pb_callback() .set(FUNC(apple_imagewriter_printer_device::switch_pb_r));
	m_8155switch->in_pc_callback() .set(FUNC(apple_imagewriter_printer_device::switch_pc_r));
	m_8155switch->out_pa_callback().set(FUNC(apple_imagewriter_printer_device::switch_pa_w));
	m_8155switch->out_pb_callback().set(FUNC(apple_imagewriter_printer_device::switch_pb_w));
	m_8155switch->out_pc_callback().set(FUNC(apple_imagewriter_printer_device::switch_pc_w));
	m_8155switch->out_to_callback().set(FUNC(apple_imagewriter_printer_device::switch_to));

	I8251(config, m_uart, CLK1 );
	m_uart->rxrdy_handler().set(FUNC(apple_imagewriter_printer_device::rxrdy_handler));
	m_uart->dtr_handler().set(FUNC(apple_imagewriter_printer_device::dtr_handler));
	m_uart->rts_handler().set(FUNC(apple_imagewriter_printer_device::rts_handler));
	m_uart->txd_handler().set(FUNC(apple_imagewriter_printer_device::txd_handler));


	BITMAP_PRINTER(config, m_bitmap_printer, PAPER_WIDTH, PAPER_HEIGHT);

	STEPPER(config, m_pf_stepper, (uint8_t) 0xa);
	STEPPER(config, m_cr_stepper, (uint8_t) 0xa);

	TIMER(config, m_timer_rxclock, 0);

	m_timer_rxclock->configure_periodic(FUNC(apple_imagewriter_printer_device::pulse_uart_clock), attotime::from_hz( 9600 * 16 * 2));
	// output from 74393 is either 1/16 input clock of (9.8304mhz / 2 / 2 (aka CLK1))  or 1/128 input clock (CLK1)
	// multiplexed by IC5 / 74LS157
	// 9600 baud: 9.8304e6 / 2 / 2 (=CLK1) / 16 (=output from 74393) / (16=m_br_factor 8251) = 9600
	// 2400 baud: 9.8304e6 / 2 / 2 (=CLK1) / 16 (=output from 74393) / (64=m_br_factor 8251) = 2400
	// 1200 baud: 9.8304e6 / 2 / 2 (=CLK1) / 128 (=output from 74393) / (16=m_br_factor 8251) = 1200
	// 300  baud: 9.8304e6 / 2 / 2 (=CLK1) / 128 (=output from 74393) / (64=m_br_factor 8251) = 300

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

	map(0x70, 0x77).rw(m_8155head,   FUNC(i8155_device::io_r), FUNC(i8155_device::io_w));
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



	PORT_START("RESET")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Reset Printer") PORT_CODE(KEYCODE_8_PAD) PORT_CHANGED_MEMBER(DEVICE_SELF, apple_imagewriter_printer_device, reset_sw, 0)


	PORT_START("DTR")
	PORT_CONFNAME(0x3, 0x01, "Connect DTR ->")
	PORT_CONFSETTING(0x0, "No connect")
	PORT_CONFSETTING(0x1, "DSR")  // default to DSR
	PORT_CONFSETTING(0x2, "CTS")
	PORT_CONFSETTING(0x3, "DSR + CTS")

	PORT_START("RTS")
	PORT_CONFNAME(0x3, 0x02, "Connect RTS ->")
	PORT_CONFSETTING(0x0, "No connect")
	PORT_CONFSETTING(0x1, "DSR")
	PORT_CONFSETTING(0x2, "CTS")  // default to CTS
	PORT_CONFSETTING(0x3, "DSR + CTS")


	PORT_START("INVERT1")  // for testing / inverting various things without having to recompile
	PORT_CONFNAME(0x1, 0x01, "Invert1 DTR")
	PORT_CONFSETTING(0x0, "Normal")
	PORT_CONFSETTING(0x1, "Invert")

	PORT_START("INVERT2")
	PORT_CONFNAME(0x1, 0x01, "Invert2 RTS")
	PORT_CONFSETTING(0x0, "Normal")
	PORT_CONFSETTING(0x1, "Invert")

	PORT_START("DEBUGMSG")
	PORT_CONFNAME(0x1, 0x00, "Debug Messages")
	PORT_CONFSETTING(0x0, "Off")
	PORT_CONFSETTING(0x1, "On")

	PORT_START("DEBUGMSG2")
	PORT_CONFNAME(0x1, 0x00, "Debug Messages 2")
	PORT_CONFSETTING(0x0, "Off")
	PORT_CONFSETTING(0x1, "On")

	PORT_START("WIDTH")
	PORT_CONFNAME(0x1, 0x01, "Printer Width")
	PORT_CONFSETTING(0x0, "15 Inches")
	PORT_CONFSETTING(0x1, "8 Inches")

	PORT_START("DARKPIXEL")
	PORT_CONFNAME(0x7, 0x00, "Print Darkness")
	PORT_CONFSETTING(0x0, "Dark")  // default to printing dark pixels (2x2)
	PORT_CONFSETTING(0x1, "Medium Dark")
	PORT_CONFSETTING(0x2, "Medium")
	PORT_CONFSETTING(0x3, "Medium Light")
	PORT_CONFSETTING(0x4, "Light")
	PORT_CONFSETTING(0x5, "Very Light")
	PORT_CONFSETTING(0x6, "Single Dot")
	PORT_CONFSETTING(0x7, "Narrow Vertical")


	// Buttons on printer
	PORT_START("SELECT")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Select Printer") PORT_CODE(KEYCODE_0_PAD) PORT_CHANGED_MEMBER(DEVICE_SELF, apple_imagewriter_printer_device, select_sw, 0)
	PORT_START("FORMFEED")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Form Feed") PORT_CODE(KEYCODE_7_PAD)
	PORT_START("LINEFEED")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Line Feed") PORT_CODE(KEYCODE_9_PAD)
	PORT_START("PAPEREND")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Paper End Sensor") PORT_CODE(KEYCODE_6_PAD) PORT_TOGGLE
	PORT_START("COVER")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Carrier Cover") PORT_CODE(KEYCODE_3_PAD)  // Active high when cover open


	// DIPSW1
	PORT_START("DIPSW1")

	PORT_DIPNAME(0x07, 0x07, "International characters and PC selection")
	PORT_DIPLOCATION("SW1:1,2,3")
	PORT_DIPSETTING(0x07, "American") // default
	PORT_DIPSETTING(0x04, "British")
	PORT_DIPSETTING(0x03, "German")
	PORT_DIPSETTING(0x01, "French")
	PORT_DIPSETTING(0x02, "Swedish")
	PORT_DIPSETTING(0x06, "Italian")
	PORT_DIPSETTING(0x00, "Spanish")
	PORT_DIPSETTING(0x05, "American")  // duplicate american setting

	PORT_DIPNAME(0x08, 0x08, "Page length")
	PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(0x08, "66 lines (11 inches)") // default
	PORT_DIPSETTING(0x00, "72 lines (12 inches)")

	PORT_DIPNAME(0x10, 0x00, "8th Data Bit")
	PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(0x10, "Use 8th Bit")
	PORT_DIPSETTING(0x00, "Ignore 8th Bit") // default

	PORT_DIPNAME(0x060, 0x60, "Character Set")
	PORT_DIPLOCATION("SW1:6,7")
	PORT_DIPSETTING(0x60, "Pica (10 cpi)") // default
	PORT_DIPSETTING(0x40, "Elite (12 cpi)")
	PORT_DIPSETTING(0x20, "Ultracondensed (17 cpi)")
	PORT_DIPSETTING(0x00, "Elite Proportional")

	PORT_DIPNAME(0x80, 0x00, "Line Feed")
	PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(0x00, "Auto Add LF after CR")
	PORT_DIPSETTING(0x80, "No LF after CR") // default

	// DIPSW2
	PORT_START("DIPSW2")

	PORT_DIPNAME(0x03, 0x00, "Baud Rate")
	PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(0x00, "9600") // default
	PORT_DIPSETTING(0x01, "2400")
	PORT_DIPSETTING(0x02, "1200")
	PORT_DIPSETTING(0x03, "300")
//  PORT_CHANGED_MEMBER(DEVICE_SELF, apple_imagewriter_printer_device, baud_rate_changed, 0)

	PORT_DIPNAME(0x04, 0x04, "Flow Control")
	PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(0x04, "Data Terminal Ready") // default
	PORT_DIPSETTING(0x00, "XON/XOFF")

	PORT_DIPNAME(0x08, 0x08, "2-4 Unused")
	PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(0x08, "ON")
	PORT_DIPSETTING(0x00, "OFF") // default

INPUT_PORTS_END

//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor apple_imagewriter_printer_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( apple_imagewriter );
}


void apple_imagewriter_printer_device::maincpu_out_sod_func(uint8_t data)
{
	// connects to fault* on serial interface  pin 14  (secondary cts)
	//  printf("MAINCPU OUT SOD FUNCTION VALUE = %x   TIME = %f  %s\n",data, machine().time().as_double(), machine().describe_context().c_str());
}

//-------------------------------------------------
//    8155 Head/Motor Functions
//-------------------------------------------------
//
//  $70 base address for 8155 HEAD/MOTOR
//  $70 command/status
//  $71 port A
//  $72 port B
//  $73 port C
//  $74 low timer
//  $75 high timer
//
//-------------------------------------------------
//    8155 Head/Motor Port A
//-------------------------------------------------

uint8_t apple_imagewriter_printer_device::head_pa_r(offs_t offset)
{
	u8 data = 0;
	return data;
}


void apple_imagewriter_printer_device::head_pa_w(uint8_t data)
{
	// PA0..PA7 = PRINTHEAD DOTS 1-8 (active low)

	m_dotpattern &= ~(0xff);
	m_dotpattern |= (data ^ 0xff);
}

//-------------------------------------------------
//    8155 Head/Motor Port B
//-------------------------------------------------

uint8_t apple_imagewriter_printer_device::head_pb_r(offs_t offset)
{
	u8 data = 0;
	return data;
}

void apple_imagewriter_printer_device::head_pb_w(uint8_t data)
{
	// PB0 = PRINTHEAD DOT 9 (active low)
	// PB1..PB4 = CR MOTOR A-D
	// PB5      = CR MOTOR ENABLE
	// PB6 = connected to 74163 to adjust counter inputs pin 4 and pin 5 (not emulated)
	//       74163 will reload either 1110 (e) or 1000 (8) depending on PB6 (pin 3 is gnd and pin 6 is 5v)
	//       and then count to 16, the carry out from the 74163 is connected through a NOT gate
	//       to the 8155 switches pin 3 (timer in).
	//       So what PB6 essentially selects is to either do a count of 2 clocks of CLK2 or a count of 8 clocks of CLK2.
	//       This selects between (Normal or 1/4 printhead speed)
	// PB7 = PRINTHEAD FIRE

	update_cr_stepper(BIT(data ^ 0xff, 1, 4));  // motor pattern inverted
	m_dotpattern &= ~(1 << 8);
	m_dotpattern |= (!BIT(data, 0) << 8);  // dot pattern is inverted

	m_pulse2->b_w(!BIT(data, 7));  // hook up to 74123 section 2

	// depending on pb6, get a rate that's either (9.8304 / 2 / 2) divided by 2 or 8 =  1.2288 mhz or 0.3072 mhz
	if (BIT(data, 6) != BIT (m_head_pb_last, 6))
	{
		m_8155switch->set_unscaled_clock_int(  CLK2.value() / 2 / (BIT(data, 6) ? 2 : 8) );
	}

	m_head_pb_last = data;
}

//-------------------------------------------------
//    8155 Head/Motor Port C
//-------------------------------------------------

uint8_t apple_imagewriter_printer_device::head_pc_r(offs_t offset)
{
	return 0;
}

void apple_imagewriter_printer_device::head_pc_w(uint8_t data)
{
	// PC0 = connected to IC17 flipflop PRESET*
	// PC1..PC4 = PF MOTOR A-D
	// PC5      = PF MOTOR ENABLE

	update_pf_stepper(BIT(data ^ 0xff, 1, 4));
	if (!BIT(data,0))
	{
		m_ic17_flipflop_head = 1;
		m_maincpu->set_input_line(I8085_RST55_LINE, !m_ic17_flipflop_head);
	}
}

//-------------------------------------------------
//    8155 Head/Motor Timer Out
//-------------------------------------------------

void apple_imagewriter_printer_device::head_to(uint8_t data)
{
	// model the IC17 flip flop part
	// TO = connected to IC17 flipflop CLK  (clocks in a zero)

	if ((!m_head_to_last) && data)  // clock in a zero on rising clock
	{
		m_ic17_flipflop_head = 0;
		m_maincpu->set_input_line(I8085_RST55_LINE, !m_ic17_flipflop_head);
	}

	m_head_to_last = data;
}

//-------------------------------------------------
//    8155 Switches Functions
//-------------------------------------------------
//
//  $78 base address for 8155 SWITCHES
//  $78 command/status
//  $79 port A
//  $7a port B
//  $7b port C
//  $7c low timer
//  $7d high timer
//
//-------------------------------------------------
//    8155 Switches Functions Port A
//-------------------------------------------------

uint8_t apple_imagewriter_printer_device::switch_pa_r(offs_t offset)
{
	u8 data =
			(!(x_pixel_coord(m_xpos) <= m_left_edge)  << 0) | // m4 home detector
			(ioport("PAPEREND")->read()               << 1) | // simulate a paper out error
			(ioport("COVER")->read()                  << 2) | //
			(!(x_pixel_coord(m_xpos) >  m_right_edge) << 3) | // return switch
			(m_ic17_flipflop_select_status            << 4) | // select status flip flop
			(ioport("FORMFEED")->read()               << 5) | //
			(ioport("LINEFEED")->read()               << 6) | //
			(BIT(ioport("DIPSW2")->read(), 3)         << 7);  // DIP 2-4 (unused)

	m_bitmap_printer->setprintheadcolor(
			m_ic17_flipflop_select_status   ? 0x888888 : 0x00dd00,    // select led
			ioport("PAPEREND")->read()      ? 0xff0000 : 0x000000 );  // paperend led

	return data;
}

void apple_imagewriter_printer_device::switch_pa_w(uint8_t data)
{
}

//-------------------------------------------------
//    8155 Switches Functions Port B
//-------------------------------------------------

uint8_t apple_imagewriter_printer_device::switch_pb_r(offs_t offset)
{
	return  (   !BIT(m_switches_pc_last, 0) ?        // PC0 controls the multiplexer  A*/B
				(ioport("DIPSW1")->read() & 0x07) :  // dip sw1-1,2,3     A
				(ioport("DIPSW2")->read() & 0x07)    // dip sw2-1,2,3     B
			)
			|   (ioport("DIPSW1")->read() & 0xf8);   // dip sw1-4,5,6,7,8
}

void apple_imagewriter_printer_device::switch_pb_w(uint8_t data)
{
}

//-------------------------------------------------
//    8155 Switches Functions Port C
//-------------------------------------------------

uint8_t apple_imagewriter_printer_device::switch_pc_r(offs_t offset)
{
	return 0;
}

void apple_imagewriter_printer_device::switch_pc_w(uint8_t data)
{
	m_switches_pc_last = data;

	int MULTIPLEX = 0; // PC0
	int CLEARBIT  = 1; // PC1
	int PRESETBIT = 2; // PC2

	if (!BIT(m_switches_pc_last, CLEARBIT))
	{
		m_ic17_flipflop_select_status = 0;  // *CLR
	}
	if (!BIT(m_switches_pc_last, PRESETBIT))
	{
		m_ic17_flipflop_select_status = 1;  // *PRE
	}

	// base clock is (9600 hz * 16)
	// so our divisor will be 1 or 8 for either (9600 hz / 1 * 16) or (9600 hz / 8 (=1200hz) * 16)

	m_baud_clock_divisor = (BIT(m_switches_pc_last, MULTIPLEX)) ? 1 : 8;

	//m_timer_rxclock->configure_periodic(FUNC(apple_imagewriter_printer_device::pulse_uart_clock), attotime::from_hz( 9600 * 16 * 2));
	// output from 74393 is either 1/16 input clock of (9.8304mhz / 2 / 2 (aka CLK1))  or 1/128 input clock (CLK1)
	// multiplexed by IC5 / 74LS157
	// 9600 baud: 9.8304e6 / 2 / 2 (=CLK1) / 16 (=output from 74393) / (16=m_brf 8251) = 9600
	// 2400 baud: 9.8304e6 / 2 / 2 (=CLK1) / 16 (=output from 74393) / (64=m_brf 8251) = 2400
	// 1200 baud: 9.8304e6 / 2 / 2 (=CLK1) / 128 (=output from 74393) / (16=m_brf 8251) = 1200
	// 300  baud: 9.8304e6 / 2 / 2 (=CLK1) / 128 (=output from 74393) / (64=m_brf 8251) = 300
}

//-------------------------------------------------
//    8155 Switches Functions Timer Out
//-------------------------------------------------


void apple_imagewriter_printer_device::switch_to(uint8_t data)
{
	// is timerout* inverted yes
	m_maincpu->set_input_line(I8085_RST75_LINE, data);  // is it !data for TIMEROUT*
	m_pulse2->a_w(data);  // send data to pulse generator 74123 section 2

	m_switches_to_last = data;
}

//-------------------------------------------------
//    Darken Pixel
//-------------------------------------------------

void apple_imagewriter_printer_device::darken_pixel(double darkpct, unsigned int& pixel)
{
	if (darkpct > 0.0)
	{
		u8 intensity = darkpct * 15.0;

		u32 pixelval = pixel;
		u32 darkenval = intensity * 0x111111;

		pixelval &= 0xffffff;

		u32 rp = BIT(pixelval, 16, 8);
		u32 gp = BIT(pixelval, 8, 8);
		u32 bp = BIT(pixelval, 0, 8);

		u32 rd = BIT(darkenval, 16, 8);
		u32 gd = BIT(darkenval, 8, 8);
		u32 bd = BIT(darkenval, 0, 8);

		u32 r = (rp >= rd) ? rp - rd : 0;    // subtract the amount to darken
		u32 g = (gp >= gd) ? gp - gd : 0;
		u32 b = (bp >= bd) ? bp - bd : 0;

		pixelval = (r << 16) | (g << 8) | (b << 0);

		pixel = pixelval;
	}
}


//-------------------------------------------------
//    Update Printhead
//-------------------------------------------------

void apple_imagewriter_printer_device::update_printhead()
{

	LOG("PRINTHEAD %x\n",m_dotpattern);
//  printf("PRINTHEAD %x %s\n",m_dotpattern, std::bitset<9>(m_dotpattern).to_string().c_str());
	const auto numdots = 9;

	const double darkenpixelarray[][2][2] =  // array of 2x2 dot patterns
	{
		{
			{ 1.0, 1.0 }, // very dark 0
			{ 1.0, 1.0 }
		},
		{
			{ 1.0, 0.75 }, // medium dark 1
			{ 0.75, 0.50 }
		},
		{
			{ 1.0, 0.5 }, // medium 2
			{ 0.5, 0.25 }
		},
		{
			{ 0.80, 0.35 }, // medium light 3
			{ 0.35, 0.0 }
		},

		{
			{ 0.70, 0.25 }, // light 4
			{ 0.25, 0.0 }
		},

		{
			{ 0.50, 0.15 }, // very light 5
			{ 0.15, 0.0 }
		},
		{
			{ 1.0, 0.0 },  // single dot 6
			{ 0.0, 0.0 }
		},
		{
			{ 1.0, 0.0 },  // narrow vertical 7
			{ 1.0, 0.0 }
		},

	};

	for (int i = 0; i < numdots; i++)
	{
		int xpixel = x_pixel_coord(m_xpos) + ((xdirection == 1) ? right_offset : left_offset); // offset to correct alignment
		int ypixel = y_pixel_coord(m_ypos) + 2 * i; // gap of 1/72 between printhead dots so multiply by 2

		if ((xpixel >= 0) && (xpixel <= (PAPER_WIDTH - 1)))
		{
			int darklevel = (ioport("DARKPIXEL")->read() & 0x7);
			int dotsizex = 2;
			int dotsizey = 2;
			for (int xo = 0; xo < dotsizex; xo++ )
			for (int yo = 0; yo < dotsizey; yo++ )
				darken_pixel( BIT(m_dotpattern, i) ?
					darkenpixelarray[darklevel][yo][xo] : 0.0, m_bitmap_printer->pix(ypixel + yo, xpixel + xo) );
		}
	}
}


//-------------------------------------------------
//    Update Stepper and return delta
//-------------------------------------------------

int apple_imagewriter_printer_device::update_stepper_delta(stepper_device * stepper, uint8_t pattern, const char * name, int direction)
{
	int lastpos = stepper->get_absolute_position();
	stepper->update(bitswap<4>(pattern, 0, 2, 1, 3));  // drive pattern is the "standard" reel pattern when bits 1,2 swapped
	int delta = stepper->get_absolute_position() - lastpos;
	return delta * direction;
}

//-------------------------------------------------
//    Update Head Position
//-------------------------------------------------

void apple_imagewriter_printer_device::update_head_pos()
{
	m_bitmap_printer->setheadpos( std::max(0, x_pixel_coord(m_xpos)), // keep printhead visible on left of screen
									y_pixel_coord(m_ypos));
}

//-------------------------------------------------
//    Update Paper Feed Stepper
//-------------------------------------------------

void apple_imagewriter_printer_device::update_pf_stepper(uint8_t vstepper)
{
	int delta = update_stepper_delta(m_pf_stepper, vstepper, "PF", -1);

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
						std::string("imagewriter"),
						std::string("imagewriter_") +
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
	update_head_pos();
}

//-------------------------------------------------
//    Update Carriage Stepper
//-------------------------------------------------

void apple_imagewriter_printer_device::update_cr_stepper(uint8_t hstepper)
{
	int delta = update_stepper_delta(m_cr_stepper, hstepper, "CR", 1);

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
		}
	}
	update_head_pos();
}

//-------------------------------------------------
//    Device Start
//-------------------------------------------------

void apple_imagewriter_printer_device::device_start()
{
	save_item(NAME(left_offset));
	save_item(NAME(right_offset));
	save_item(NAME(m_left_edge));
	save_item(NAME(m_right_edge));
	save_item(NAME(xposratio0));
	save_item(NAME(xposratio1));
	save_item(NAME(yposratio0));
	save_item(NAME(yposratio1));
}

//-------------------------------------------------
//    Update Serial
//-------------------------------------------------

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

//-------------------------------------------------
//    Device Reset
//-------------------------------------------------

void apple_imagewriter_printer_device::device_reset()
{
	update_serial(0);
}

void apple_imagewriter_printer_device::device_reset_after_children()
{
}


WRITE_LINE_MEMBER(apple_imagewriter_printer_device::printer_online)
{
	/// TODO: ?
}

//-------------------------------------------------
//    RCV Complete
//-------------------------------------------------

void apple_imagewriter_printer_device::rcv_complete()
{
	receive_register_extract();
	m_printer->output(get_received_char());
	if (ioport("DEBUGMSG2")->read())    printf("RECEIVED CHARACTER = %x\n", get_received_char());
	if (ioport("DEBUGMSG2")->read())    printf("BUILDSTRING = %s\n",buildstring.c_str());
	buildstring = "";
	char c = get_received_char();
	if (ioport("DEBUGMSG2")->read())
	{
		if (c >=32 &&  c<=0x7f) printf("CHAR = %c\n",c); else printf("CHAR = 0x%x\n",c);
	}
}

