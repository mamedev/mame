// license:BSD-3-Clause
// copyright-holders:Ramiro Polla, Felipe Sanches
/*
 * Epson LX-810L dot matrix printer emulation
 *
 * IC list:
 *   uPD7810HG (cpu)
 *   E05A30 (gate array)
 *   2064C (8k RAM)
 *   ER59256 (EEP-ROM - serial nvram)
 *   SLA7020M (step motor driver)
 *   uPC494C (pulse width modulation control)
 *
 * Devices boot and enter main input loop. Data is received through the
 * centronics bus and printed as expected in a separate screen.
 *
 * It is possible to run the printers' self test with this procedure:
 * - Turn on device;
 * - Toggle Line Feed button (press 'L');
 * - Reset device;
 * - Toggle Line Feed button again;
 * - Press Online button (press 'O');
 *
 * The printer's carriage will seek home and it will start printing
 * some test data. The Online LED will blink at each line.
 */

#include "emu.h"
#include "epson_lx810l.h"
#include "speaker.h"

//#define VERBOSE 1
#include "logmacro.h"

//extern const char layout_lx800[]; /* use layout from lx800 */


/* The printer starts printing at x offset 44 and stops printing at x
 * offset 1009, giving a total of 965 printable pixels. Supposedly, the
 * border at the far right would be at x offset 1053. I've chosen the
 * width for the paper as 1024, since it's a nicer number than 1053, so
 * an offset must be used to centralize the pixels.
 */
#define CR_OFFSET    (-14)

#define PAPER_WIDTH  1024    // 120 dpi * 8.5333 inches
#define PAPER_HEIGHT (11*72) // 72 dpi * 11 inches



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(EPSON_LX810L, epson_lx810l_device, "lx810l", "Epson LX-810L")
DEFINE_DEVICE_TYPE(EPSON_AP2000, epson_ap2000_device, "ap2000", "Epson ActionPrinter 2000")


//-------------------------------------------------
//  ROM( lx810l )
//-------------------------------------------------

ROM_START( lx810l )
	ROM_REGION(0x8000, "maincpu", 0)
	ROM_LOAD("lx810l.ic3c", 0x0000, 0x8000, CRC(a66454e1) SHA1(8e6f2f98abcbd8af6e34b9ba746edf0d18aef843) )
	ROM_REGION16_BE(0x20, "eeprom", 0)
	ROM_LOAD( "at93c06", 0x00, 0x20, NO_DUMP )
ROM_END


//-------------------------------------------------
//  ROM( ap2000 )
//-------------------------------------------------

ROM_START( ap2000 )
	ROM_REGION(0x8000, "maincpu", 0)
	ROM_LOAD("ap2k.ic3c", 0x0000, 0x8000, CRC(ee7294b7) SHA1(219ffa6ff661ce95d5772c9fc1967093718f04e9) )
	ROM_REGION16_BE(0x20, "eeprom", 0)
	ROM_LOAD( "at93c06", 0x00, 0x20, NO_DUMP )
ROM_END


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *epson_lx810l_device::device_rom_region() const
{
	return ROM_NAME( lx810l );
}


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *epson_ap2000_device::device_rom_region() const
{
	return ROM_NAME( ap2000 );
}


//-------------------------------------------------
//  ADDRESS_MAP( lx810l_mem )
//-------------------------------------------------

void epson_lx810l_device::lx810l_mem(address_map &map)
{
	map(0x0000, 0x7fff).rom(); /* 32k firmware */
	map(0x8000, 0x9fff).ram(); /* 8k external RAM */
	map(0xa000, 0xbfff).rw(FUNC(epson_lx810l_device::fakemem_r), FUNC(epson_lx810l_device::fakemem_w)); /* fake memory, write one, set all */
	map(0xc000, 0xc00f).mirror(0x1ff0).rw("e05a30", FUNC(e05a30_device::read), FUNC(e05a30_device::write));
	map(0xe000, 0xfeff).noprw(); /* not used */
}


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void epson_lx810l_device::device_add_mconfig(machine_config &config)
{
	/* basic machine hardware */
	upd7810_device &upd(UPD7810(config, m_maincpu, 14.7456_MHz_XTAL));
	upd.set_addrmap(AS_PROGRAM, &epson_lx810l_device::lx810l_mem);
	upd.pa_in_cb().set(FUNC(epson_lx810l_device::porta_r));
	upd.pa_out_cb().set(FUNC(epson_lx810l_device::porta_w));
	upd.pb_in_cb().set(FUNC(epson_lx810l_device::portb_r));
	upd.pb_out_cb().set(FUNC(epson_lx810l_device::portb_w));
	upd.pc_in_cb().set(FUNC(epson_lx810l_device::portc_r));
	upd.pc_out_cb().set(FUNC(epson_lx810l_device::portc_w));
	upd.an0_func().set(FUNC(epson_lx810l_device::an0_r));
	upd.an1_func().set(FUNC(epson_lx810l_device::an1_r));
	upd.an2_func().set(FUNC(epson_lx810l_device::an2_r));
	upd.an3_func().set(FUNC(epson_lx810l_device::an3_r));
	upd.an4_func().set(FUNC(epson_lx810l_device::an4_r));
	upd.an5_func().set(FUNC(epson_lx810l_device::an5_r));
	upd.an6_func().set(FUNC(epson_lx810l_device::an6_r));
	upd.an7_func().set(FUNC(epson_lx810l_device::an7_r));
	upd.co0_func().set(FUNC(epson_lx810l_device::co0_w));
	upd.co1_func().set("dac", FUNC(dac_bit_interface::write));

//  config.set_default_layout(layout_lx800);

	/* audio hardware */
	SPEAKER(config, "speaker").front_center();
	DAC_1BIT(config, "dac", 0).add_route(ALL_OUTPUTS, "speaker", 0.25);

	/* gate array */
	e05a30_device &e05a30(E05A30(config, m_e05a30, 0));
	e05a30.printhead().set(FUNC(epson_lx810l_device::printhead));
	e05a30.pf_stepper().set(FUNC(epson_lx810l_device::pf_stepper));
	e05a30.cr_stepper().set(FUNC(epson_lx810l_device::cr_stepper));
	e05a30.ready().set(FUNC(epson_lx810l_device::e05a30_ready));
	e05a30.centronics_ack().set(FUNC(epson_lx810l_device::e05a30_centronics_ack));
	e05a30.centronics_busy().set(FUNC(epson_lx810l_device::e05a30_centronics_busy));
	e05a30.centronics_perror().set(FUNC(epson_lx810l_device::e05a30_centronics_perror));
	e05a30.centronics_fault().set(FUNC(epson_lx810l_device::e05a30_centronics_fault));
	e05a30.centronics_select().set(FUNC(epson_lx810l_device::e05a30_centronics_select));
	e05a30.cpu_reset().set(FUNC(epson_lx810l_device::e05a30_cpu_reset));
	e05a30.ready_led().set(FUNC(epson_lx810l_device::e05a30_ready_led));

	/* 256-bit eeprom */
	EEPROM_93C06_16BIT(config, "eeprom");

	BITMAP_PRINTER(config, m_bitmap_printer, PAPER_WIDTH, PAPER_HEIGHT, 120, 72);  // do 72 dpi
	m_bitmap_printer->set_pf_stepper_ratio(1,6);  // pf stepper moves at 216 dpi so at 72dpi half steps
	m_bitmap_printer->set_cr_stepper_ratio(1,1);
}

/***************************************************************************
    INPUT PORTS
***************************************************************************/


static INPUT_PORTS_START( epson_lx810 )
	/* Buttons on printer */
	PORT_START("ONLINE")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("On Line") PORT_CODE(KEYCODE_0_PAD) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(epson_lx810l_device::online_sw), 0)
	PORT_START("FORMFEED")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Form Feed") PORT_CODE(KEYCODE_7_PAD)
	PORT_START("LINEFEED")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Line Feed") PORT_CODE(KEYCODE_9_PAD)
	PORT_START("LOADEJECT")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Load/Eject") PORT_CODE(KEYCODE_1_PAD)
	PORT_START("PAPEREND")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Paper End Sensor") PORT_CODE(KEYCODE_6_PAD)
	PORT_START("RESET")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Reset Printer") PORT_CODE(KEYCODE_2_PAD) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(epson_lx810l_device::reset_printer), 0)

	PORT_START("DIPSW1")
	PORT_DIPNAME(0x01, 0x00, "Character spacing")             PORT_DIPLOCATION("SW 1:!1")
	PORT_DIPSETTING(   0x00, "10 cpi")
	PORT_DIPSETTING(   0x01, "12 cpi")
	PORT_DIPNAME(0x02, 0x00, "Shape of zero")                 PORT_DIPLOCATION("SW 1:!2")
	PORT_DIPSETTING(   0x00, "Not slashed")
	PORT_DIPSETTING(   0x02, "Slashed")
	PORT_DIPNAME(0x04, 0x00, "Character table")               PORT_DIPLOCATION("SW 1:!3")
	PORT_DIPSETTING(   0x00, "Italics")
	PORT_DIPSETTING(   0x04, "Graphics")
	PORT_DIPNAME(0x08, 0x08, "Short tear-off")                PORT_DIPLOCATION("SW 1:!4")
	PORT_DIPSETTING(   0x08, DEF_STR(Off))
	PORT_DIPSETTING(   0x00, DEF_STR(On))
	PORT_DIPNAME(0x10, 0x00, "Draft printing speed")          PORT_DIPLOCATION("SW 1:!6")
	PORT_DIPSETTING(   0x10, DEF_STR(Normal))
	PORT_DIPSETTING(   0x00, "High speed")
	PORT_DIPNAME(0xe0, 0xe0, "International character set")   PORT_DIPLOCATION("SW 1:!6,!7,!8")
	PORT_DIPSETTING(   0xe0, "USA (PC 437)")
	PORT_DIPSETTING(   0x60, "France (PC 850)")
	PORT_DIPSETTING(   0xa0, "Germany (PC 860)")
	PORT_DIPSETTING(   0x20, "UK (PC 863)")
	PORT_DIPSETTING(   0xc0, "Denmark (PC 865)")
	PORT_DIPSETTING(   0x40, "Sweden (PC 437)")
	PORT_DIPSETTING(   0x80, "Italy (PC 437)")
	PORT_DIPSETTING(   0x00, "Spain (PC 437)")

	PORT_START("DIPSW2")
	PORT_DIPNAME(0x01, 0x00, "Page length")                   PORT_DIPLOCATION("SW 2:!1")
	PORT_DIPSETTING(   0x00, "11\"")
	PORT_DIPSETTING(   0x01, "12\"")
	PORT_DIPNAME(0x02, 0x00, "Cut-sheet feeder mode")         PORT_DIPLOCATION("SW 2:!2")
	PORT_DIPSETTING(   0x00, DEF_STR(Off))
	PORT_DIPSETTING(   0x02, DEF_STR(On))
	PORT_DIPNAME(0x04, 0x00, "Skip over perforation")         PORT_DIPLOCATION("SW 2:!3")
	PORT_DIPSETTING(   0x00, DEF_STR(Off))
	PORT_DIPSETTING(   0x04, "1\"")
	PORT_DIPNAME(0x08, 0x00, "Auto line feed")                PORT_DIPLOCATION("SW 2:!4")
	PORT_DIPSETTING(   0x00, DEF_STR(Off))
	PORT_DIPSETTING(   0x08, DEF_STR(On))
INPUT_PORTS_END

static INPUT_PORTS_START( epson_lx810l )
	PORT_INCLUDE(epson_lx810)

	PORT_MODIFY("DIPSW1")
	PORT_DIPNAME(0x0c, 0x00, "Page length")                   PORT_DIPLOCATION("SW 1:!3,!4")
	PORT_DIPSETTING(   0x08, "8.5\"")
	PORT_DIPSETTING(   0x00, "11\"")
	PORT_DIPSETTING(   0x0c, "11.7\" (A4)")
	PORT_DIPSETTING(   0x04, "12\"")
	PORT_DIPNAME(0x10, 0x00, "Character table")               PORT_DIPLOCATION("SW 1:!5")
	PORT_DIPSETTING(   0x00, "Italics")
	PORT_DIPSETTING(   0x10, "Graphics")

	PORT_MODIFY("DIPSW2")
	PORT_DIPNAME(0x01, 0x01, "Short tear-off")                PORT_DIPLOCATION("SW 2:!1")
	PORT_DIPSETTING(   0x01, "Invalid")
	PORT_DIPSETTING(   0x00, "Valid")
INPUT_PORTS_END



//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor epson_lx810l_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(epson_lx810l);
}

INPUT_CHANGED_MEMBER(epson_lx810l_device::online_sw)
{
	m_maincpu->set_input_line(UPD7810_INTF2, newval ? CLEAR_LINE : ASSERT_LINE);
}

INPUT_CHANGED_MEMBER(epson_lx810l_device::reset_printer)
{
	if (newval)
	{
		m_maincpu->pulse_input_line(INPUT_LINE_RESET, attotime::zero);  // reset cpu
		m_e05a30->reset();  // this will generate an NMI interrupt when the e05a30 is ready (minimum 0.9 seconds after reset)
	}
}




//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  epson_lx810l_device - constructor
//-------------------------------------------------

epson_lx810l_device::epson_lx810l_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	epson_lx810l_device(mconfig, EPSON_LX810L, tag, owner, clock)
{
}

epson_lx810l_device::epson_lx810l_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, type, tag, owner, clock),
	device_centronics_peripheral_interface(mconfig, *this),
	m_maincpu(*this, "maincpu"),
	m_bitmap_printer(*this, "bitmap_printer"),
	m_eeprom(*this, "eeprom"),
	m_e05a30(*this, "e05a30"),
	m_online_led(*this, "online_led"),
	m_ready_led(*this, "ready_led"),
	m_online_ioport(*this, "ONLINE"),
	m_formfeed_ioport(*this, "FORMFEED"),
	m_linefeed_ioport(*this, "LINEFEED"),
	m_loadeject_ioport(*this, "LOADEJECT"),
	m_paperend_ioport(*this, "PAPEREND"),
	m_dipsw1_ioport(*this, "DIPSW1"),
	m_dipsw2_ioport(*this, "DIPSW2"),
	m_93c06_clk(0),
	m_93c06_cs(0),
	m_printhead(0),
	m_real_cr_steps(0),
	m_fakemem(0),
	m_in_between_offset(0),
	m_rightward_offset(-3)
{
}

epson_ap2000_device::epson_ap2000_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: epson_lx810l_device(mconfig, EPSON_AP2000, tag, owner, clock)
{ }


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void epson_lx810l_device::device_start()
{
	m_online_led.resolve();
	m_ready_led.resolve();

	m_cr_timer = timer_alloc(FUNC(epson_lx810l_device::cr_tick), this);
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void epson_lx810l_device::device_reset()
{
	m_in_between_offset = 0;
}


//-------------------------------------------------
//  cr_tick - handle a carriage return
//-------------------------------------------------

TIMER_CALLBACK_MEMBER(epson_lx810l_device::cr_tick)
{
	/* The firmware issues two half-steps in sequence, one immediately
	 * after the other. At full speed, the motor does two half-steps at
	 * each 833 microseconds. A timer fires the printhead twice, with
	 * the same period as each half-step (417 microseconds), but with
	 * a 356 microseconds delay relative to the motor steps.
	 */
	m_in_between_offset += param;

	m_real_cr_steps--;
	if (m_real_cr_steps)
		m_cr_timer->adjust(attotime::from_usec(400), m_bitmap_printer->m_cr_direction);
}


/***************************************************************************
    FAKEMEM READ/WRITE
***************************************************************************/

uint8_t epson_lx810l_device::fakemem_r()
{
	return m_fakemem;
}

void epson_lx810l_device::fakemem_w(uint8_t data)
{
	m_fakemem = data;
}


/***************************************************************************
    I/O PORTS
***************************************************************************/

/*
 * PA0  R   CN7 sensor (Home Position, HP, active low)
 * PA1  R   CN6 sensor (Paper-End, PE, active low)
 * PA2  R   CN4 sensor (Release, low = tractor)
 * PA3   W  Stepper motor voltage reference (these 3 pins make up one voltage)
 * PA4   W  Stepper motor voltage reference (these 3 pins make up one voltage)
 * PA5   W  Stepper motor voltage reference (these 3 pins make up one voltage)
 * PA6  R   Line Feed SWITCH
 * PA7  R   Form Feed SWITCH
 */
uint8_t epson_lx810l_device::porta_r(offs_t offset)
{
	uint8_t result = 0;
	uint8_t hp_sensor = (m_bitmap_printer->m_xpos <= 0) ? 0 : 1;

	//uint8_t pe_sensor = m_pf_pos_abs <= 0 ? 1 : 0;

	result |= hp_sensor; /* home position */
	//result |= pe_sensor << 1; /* paper end */
	result |= m_paperend_ioport->read() << 1;  // simulate a paper out error
	result |= m_linefeed_ioport->read() << 6;
	result |= m_formfeed_ioport->read() << 7;

	LOG("%s: lx810l_PA_r(%02x): result %02x\n", machine().describe_context(), offset, result);

	m_bitmap_printer->set_led_state(bitmap_printer_device::LED_ERROR, !m_paperend_ioport->read());

	return result;
}

void epson_lx810l_device::porta_w(offs_t offset, uint8_t data)
{
	LOG("%s: lx810l_PA_w(%02x): %02x: stepper vref %d\n", machine().describe_context(), offset, data, BIT(data, 3) | (BIT(data, 4)<<1) | (BIT(data, 5)<<2));
}

/*
 * PB0  R   DIP1.0 & 93C06.DO
 * PB1  RW  DIP1.1 & 93C06.DI
 * PB2  R   DIP1.2
 * PB3  R   DIP1.3
 * PB4  R   DIP1.4
 * PB5  R   DIP1.5
 * PB6  R   DIP1.6
 * PB7  R   DIP1.7
 */
uint8_t epson_lx810l_device::portb_r(offs_t offset)
{
	uint8_t result = ~m_dipsw1_ioport->read();

	/* if 93C06 is selected */
	if (m_93c06_cs) {
		uint8_t do_r = m_eeprom->do_read();
		result &= 0xfe;
		result |= do_r;
	}

	LOG("%s: lx810l_PB_r(%02x): result %02x\n", machine().describe_context(), offset, result);

	return result;
}

void epson_lx810l_device::portb_w(offs_t offset, uint8_t data)
{
	uint8_t data_in = BIT(data, 1);

	/* if 93C06 is selected */
	if (m_93c06_cs)
		m_eeprom->di_write(data_in);

	LOG("%s: lx810l_PB_w(%02x): %02x: 93c06 data %d\n", machine().describe_context(), offset, data, data_in);
}

/*
 * PC0   W  TXD        serial i/o txd, also TAMA.25
 * PC1  R   RXD        serial i/o rxd, also E05A30.28
 * PC2   W  ONLINE LP  online led
 * PC3  R   ONLINE SW  online switch
 * PC4   W  93C06.SK
 * PC5   W  93C06.CS
 * PC6   W  FIRE       drive pulse width signal, also E05A30.57
 * PC7   W  BUZZER     buzzer signal
 */
uint8_t epson_lx810l_device::portc_r(offs_t offset)
{
	uint8_t result = 0;

	/* result |= ioport("serial")->read() << 1; */
	result |= !m_online_ioport->read() << 3;
	result |= m_93c06_clk << 4;
	result |= m_93c06_cs  << 5;

	LOG("%s: lx810l_PC_r(%02x): %02x\n", machine().describe_context(), offset, result);

	return result;
}

void epson_lx810l_device::portc_w(offs_t offset, uint8_t data)
{
	/* ioport("serial")->write(BIT(data, 0)); */

	m_93c06_clk =  BIT(data, 4);
	m_93c06_cs  = !BIT(data, 5);

	LOG("%s: PC_w(%02x): %02x 93c06 clk: %d cs: %d\n", machine().describe_context(), offset, data, m_93c06_clk, m_93c06_cs);

	m_eeprom->clk_write(m_93c06_clk ? ASSERT_LINE : CLEAR_LINE);
	m_eeprom->cs_write (m_93c06_cs  ? ASSERT_LINE : CLEAR_LINE);

	m_online_led = !BIT(data, 2);
	m_bitmap_printer->set_led_state(bitmap_printer_device::LED_ONLINE, m_online_led);
}


/***************************************************************************
    GATE ARRAY
***************************************************************************/

void epson_lx810l_device::printhead(uint16_t data)
{
	m_printhead = data;
}

void epson_lx810l_device::pf_stepper(uint8_t data)
{
	m_bitmap_printer->update_pf_stepper(data);
}

void epson_lx810l_device::cr_stepper(uint8_t data)
{
	m_bitmap_printer->update_cr_stepper(bitswap<4>(data, 0, 1, 2, 3));  // reverse bits

	m_in_between_offset = 0;

	if (!m_real_cr_steps)
	{
		m_cr_timer->adjust(attotime::from_usec(400), m_bitmap_printer->m_cr_direction);
		m_real_cr_steps++;
	}
}

void epson_lx810l_device::e05a30_ready(int state)
{
	// must be longer than attotime::zero - 0.09 is minimum to initialize properly
	m_maincpu->pulse_input_line(INPUT_LINE_NMI, attotime::from_double(0.09));
}


/***************************************************************************
    Extended Timer Output
***************************************************************************/

void epson_lx810l_device::co0_w(int state)
{
	/* Printhead is being fired on !state. */
	if (!state) {
		/* The firmware expects a 300 microseconds delay between the fire
		 * signal and the impact of the printhead on the paper. This can be
		 * verified by the timings of the steps and fire signals for the
		 * same positions with different directions (left to right or right
		 * to left). We don't simulate this delay since it is smaller than
		 * the time it takes the printhead to travel one pixel (which would
		 * be 417 microseconds), so it makes no difference to us.
		 * It is interesting to note that the vertical alignment between
		 * lines which are being printed in different directions is
		 * noticeably off in the 20+ years old printer used for testing =).
		 */
		for (int i = 0; i < 9; i++)
		{
			if ((m_printhead & (1<<(8-i))) != 0)
				m_bitmap_printer->pix(m_bitmap_printer->m_ypos + i * 1, // * 1 for no interleave at 72 vdpi
				m_bitmap_printer->m_xpos + CR_OFFSET + m_in_between_offset +
				(m_bitmap_printer->m_cr_direction > 0 ? m_rightward_offset : 0)) = 0x000000;
		}
	}
}


/***************************************************************************
    ADC
***************************************************************************/

uint8_t epson_lx810l_device::an0_r()
{
	uint8_t res = !!(m_dipsw2_ioport->read() & 0x01);
	return res - 1; /* DIPSW2.1 */
}

uint8_t epson_lx810l_device::an1_r()
{
	uint8_t res = !!(m_dipsw2_ioport->read() & 0x02);
	return res - 1; /* DIPSW2.2 */
}

uint8_t epson_lx810l_device::an2_r()
{
	uint8_t res = !!(m_dipsw2_ioport->read() & 0x04);
	return res - 1; /* DIPSW2.3 */
}

uint8_t epson_lx810l_device::an3_r()
{
	uint8_t res = !!(m_dipsw2_ioport->read() & 0x08);
	return res - 1; /* DIPSW2.4 */
}

uint8_t epson_lx810l_device::an4_r()
{
	return 0xff;
}

uint8_t epson_lx810l_device::an5_r()
{
	return 0xCB; /* motor voltage, 0xcb = 24V */
}

uint8_t epson_lx810l_device::an6_r()
{
	uint8_t res = !m_loadeject_ioport->read();
	return res - 1;
}

uint8_t epson_lx810l_device::an7_r()
{
	return 0xff;
}
