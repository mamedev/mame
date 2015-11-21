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

#include "epson_lx810l.h"
extern const char layout_lx800[]; /* use layout from lx800 */

//#define LX810LDEBUG
#ifdef LX810LDEBUG
#define LX810LLOG(...) fprintf(stderr, __VA_ARGS__)
#else
#define LX810LLOG(...)
#endif

//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type EPSON_LX810L = &device_creator<epson_lx810l_t>;
const device_type EPSON_AP2000 = &device_creator<epson_ap2000_t>;


//-------------------------------------------------
//  ROM( lx810l )
//-------------------------------------------------

ROM_START( lx810l )
	ROM_REGION(0x8000, "maincpu", 0)
	ROM_LOAD("lx810l.ic3c", 0x0000, 0x8000, CRC(a66454e1) SHA1(8e6f2f98abcbd8af6e34b9ba746edf0d18aef843) )
	ROM_REGION(0x20, "eeprom", 0)
	ROM_LOAD( "at93c06", 0x00, 0x20, NO_DUMP )
ROM_END


//-------------------------------------------------
//  ROM( ap2000 )
//-------------------------------------------------

ROM_START( ap2000 )
	ROM_REGION(0x8000, "maincpu", 0)
	ROM_LOAD("ap2k.ic3c", 0x0000, 0x8000, CRC(ee7294b7) SHA1(219ffa6ff661ce95d5772c9fc1967093718f04e9) )
	ROM_REGION(0x20, "eeprom", 0)
	ROM_LOAD( "at93c06", 0x00, 0x20, NO_DUMP )
ROM_END


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const rom_entry *epson_lx810l_t::device_rom_region() const
{
	return ROM_NAME( lx810l );
}


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const rom_entry *epson_ap2000_t::device_rom_region() const
{
	return ROM_NAME( ap2000 );
}


//-------------------------------------------------
//  ADDRESS_MAP( lx810l_mem )
//-------------------------------------------------

static ADDRESS_MAP_START( lx810l_mem, AS_PROGRAM, 8, epson_lx810l_t )
	AM_RANGE(0x0000, 0x7fff) AM_ROM /* 32k firmware */
	AM_RANGE(0x8000, 0x9fff) AM_RAM /* 8k external RAM */
	AM_RANGE(0xa000, 0xbfff) AM_READWRITE(fakemem_r, fakemem_w) /* fake memory, write one, set all */
	AM_RANGE(0xc000, 0xdfff) AM_MIRROR(0x1ff0) AM_DEVREADWRITE("e05a30", e05a30_device, read, write)
	AM_RANGE(0xe000, 0xfeff) AM_NOP /* not used */
	AM_RANGE(0xff00, 0xffff) AM_RAM /* internal CPU RAM */
ADDRESS_MAP_END


//-------------------------------------------------
//  ADDRESS_MAP( lx810l_io )
//-------------------------------------------------

static ADDRESS_MAP_START( lx810l_io, AS_IO, 8, epson_lx810l_t )
	AM_RANGE(UPD7810_PORTA, UPD7810_PORTA) AM_READWRITE(porta_r, porta_w)
	AM_RANGE(UPD7810_PORTB, UPD7810_PORTB) AM_READWRITE(portb_r, portb_w)
	AM_RANGE(UPD7810_PORTC, UPD7810_PORTC) AM_READWRITE(portc_r, portc_w)
ADDRESS_MAP_END


//-------------------------------------------------
//  MACHINE_DRIVER( epson_lx810l )
//-------------------------------------------------

static MACHINE_CONFIG_FRAGMENT( epson_lx810l )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", UPD7810, XTAL_14_7456MHz)
	MCFG_CPU_PROGRAM_MAP(lx810l_mem)
	MCFG_CPU_IO_MAP(lx810l_io)
	MCFG_UPD7810_AN0(READ8(epson_lx810l_t, an0_r))
	MCFG_UPD7810_AN1(READ8(epson_lx810l_t, an1_r))
	MCFG_UPD7810_AN2(READ8(epson_lx810l_t, an2_r))
	MCFG_UPD7810_AN3(READ8(epson_lx810l_t, an3_r))
	MCFG_UPD7810_AN4(READ8(epson_lx810l_t, an4_r))
	MCFG_UPD7810_AN5(READ8(epson_lx810l_t, an5_r))
	MCFG_UPD7810_AN6(READ8(epson_lx810l_t, an6_r))
	MCFG_UPD7810_AN7(READ8(epson_lx810l_t, an7_r))
	MCFG_UPD7810_CO0(WRITELINE(epson_lx810l_t, co0_w))
	MCFG_UPD7810_CO1(WRITELINE(epson_lx810l_t, co1_w))

	MCFG_DEFAULT_LAYOUT(layout_lx800)

	/* video hardware (simulates paper) */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(PAPER_WIDTH, PAPER_HEIGHT)
	MCFG_SCREEN_VISIBLE_AREA(0, PAPER_WIDTH-1, 0, PAPER_HEIGHT-1)
	MCFG_SCREEN_UPDATE_DRIVER(epson_lx810l_t, screen_update_lx810l)

	/* audio hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_DAC_ADD("dac")
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)

	/* gate array */
	MCFG_DEVICE_ADD("e05a30", E05A30, 0)
	MCFG_E05A30_PRINTHEAD_CALLBACK(WRITE16(epson_lx810l_t, printhead))
	MCFG_E05A30_PF_STEPPER_CALLBACK(WRITE8(epson_lx810l_t, pf_stepper))
	MCFG_E05A30_CR_STEPPER_CALLBACK(WRITE8(epson_lx810l_t, cr_stepper))
	MCFG_E05A30_READY_CALLBACK(WRITELINE(epson_lx810l_t, e05a30_ready))
	MCFG_E05A30_CENTRONICS_ACK_CALLBACK(WRITELINE(epson_lx810l_t, e05a30_centronics_ack))
	MCFG_E05A30_CENTRONICS_BUSY_CALLBACK(WRITELINE(epson_lx810l_t, e05a30_centronics_busy))
	MCFG_E05A30_CENTRONICS_PERROR_CALLBACK(WRITELINE(epson_lx810l_t, e05a30_centronics_perror))
	MCFG_E05A30_CENTRONICS_FAULT_CALLBACK(WRITELINE(epson_lx810l_t, e05a30_centronics_fault))
	MCFG_E05A30_CENTRONICS_SELECT_CALLBACK(WRITELINE(epson_lx810l_t, e05a30_centronics_select))

	/* 256-bit eeprom */
	MCFG_EEPROM_SERIAL_93C06_ADD("eeprom")

	/* steppers */
	MCFG_STEPPER_ADD("pf_stepper")
	MCFG_STEPPER_REEL_TYPE(NOT_A_REEL)
	MCFG_STEPPER_INIT_PHASE(4)

	MCFG_STEPPER_ADD("cr_stepper")
	MCFG_STEPPER_REEL_TYPE(NOT_A_REEL)
	MCFG_STEPPER_INIT_PHASE(2)

MACHINE_CONFIG_END

//-------------------------------------------------
//  machine_config_additions - device-specific
//  machine configurations
//-------------------------------------------------

machine_config_constructor epson_lx810l_t::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( epson_lx810l );
}


/***************************************************************************
    INPUT PORTS
***************************************************************************/

static INPUT_PORTS_START( epson_lx810l )

	/* Buttons on printer */
	PORT_START("ONLINE")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("On Line") PORT_CODE(KEYCODE_O) PORT_CHANGED_MEMBER(DEVICE_SELF, epson_lx810l_t, online_sw, NULL)
	PORT_START("FORMFEED")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Form Feed") PORT_CODE(KEYCODE_F) PORT_TOGGLE
	PORT_START("LINEFEED")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Line Feed") PORT_CODE(KEYCODE_L) PORT_TOGGLE
	PORT_START("LOADEJECT")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Load/Eject") PORT_CODE(KEYCODE_E)

	/* DIPSW1 */
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

	/* DIPSW2 */
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

ioport_constructor epson_lx810l_t::device_input_ports() const
{
	return INPUT_PORTS_NAME( epson_lx810l );
}

INPUT_CHANGED_MEMBER(epson_lx810l_t::online_sw)
{
	m_maincpu->set_input_line(UPD7810_INTF2, newval ? CLEAR_LINE : ASSERT_LINE);
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  epson_lx810l_t - constructor
//-------------------------------------------------

epson_lx810l_t::epson_lx810l_t(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, EPSON_LX810L, "Epson LX-810L", tag, owner, clock, "lx810l", __FILE__),
	device_centronics_peripheral_interface(mconfig, *this),
	m_maincpu(*this, "maincpu"),
	m_pf_stepper(*this, "pf_stepper"),
	m_cr_stepper(*this, "cr_stepper"),
	m_eeprom(*this, "eeprom"),
	m_dac(*this, "dac"),
	m_e05a30(*this, "e05a30"),
	m_screen(*this, "screen"),
	m_93c06_clk(0),
	m_93c06_cs(0),
	m_printhead(0),
	m_pf_pos_abs(1),
	m_cr_pos_abs(1),
	m_real_cr_pos(1),
	m_real_cr_steps(0),
	m_real_cr_dir(0), m_fakemem(0)
{
}

epson_lx810l_t::epson_lx810l_t(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source) :
	device_t(mconfig, type, name, tag, owner, clock, shortname, __FILE__),
	device_centronics_peripheral_interface(mconfig, *this),
	m_maincpu(*this, "maincpu"),
	m_pf_stepper(*this, "pf_stepper"),
	m_cr_stepper(*this, "cr_stepper"),
	m_eeprom(*this, "eeprom"),
	m_dac(*this, "dac"),
	m_e05a30(*this, "e05a30"),
	m_screen(*this, "screen"),
	m_93c06_clk(0),
	m_93c06_cs(0),
	m_printhead(0),
	m_pf_pos_abs(1),
	m_cr_pos_abs(1),
	m_real_cr_pos(1),
	m_real_cr_steps(0),
	m_real_cr_dir(0), m_fakemem(0)
{
}

epson_ap2000_t::epson_ap2000_t(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: epson_lx810l_t(mconfig, EPSON_AP2000, "Epson ActionPrinter 2000", tag, owner, clock, "ap2000", __FILE__)
{ }


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------




void epson_lx810l_t::device_start()
{
	machine().first_screen()->register_screen_bitmap(m_bitmap);
	m_bitmap.fill(0xffffff); /* Start with a clean white piece of paper */
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void epson_lx810l_t::device_reset()
{
	m_dac->write_unsigned8(0);
}


//-------------------------------------------------
//  device_timer - device-specific timer
//-------------------------------------------------

void epson_lx810l_t::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	switch (id) {
	case TIMER_CR:
		/* The firmware issues two half-steps in sequence, one immediately
		 * after the other. At full speed, the motor does two half-steps at
		 * each 833 microseconds. A timer fires the printhead twice, with
		 * the same period as each half-step (417 microseconds), but with
		 * a 356 microseconds delay relative to the motor steps.
		 */
		m_real_cr_pos += param;
		m_real_cr_steps--;
		if (m_real_cr_steps)
			timer_set(attotime::from_usec(400), TIMER_CR, m_real_cr_dir);
		break;
	}
}


/***************************************************************************
    FAKEMEM READ/WRITE
***************************************************************************/

READ8_MEMBER(epson_lx810l_t::fakemem_r)
{
	return m_fakemem;
}

WRITE8_MEMBER(epson_lx810l_t::fakemem_w)
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
READ8_MEMBER( epson_lx810l_t::porta_r )
{
	UINT8 result = 0;
	UINT8 hp_sensor = m_cr_pos_abs <= 0 ? 0 : 1;
	UINT8 pe_sensor = m_pf_pos_abs <= 0 ? 1 : 0;

	result |= hp_sensor; /* home position */
	result |= pe_sensor << 1; /* paper end */
	result |= ioport("LINEFEED")->read() << 6;
	result |= ioport("FORMFEED")->read() << 7;

	LX810LLOG("%s: lx810l_PA_r(%02x): result %02x\n", machine().describe_context(), offset, result);

	return result;
}

WRITE8_MEMBER( epson_lx810l_t::porta_w )
{
	LX810LLOG("%s: lx810l_PA_w(%02x): %02x: stepper vref %d\n", machine().describe_context(), offset, data, BIT(data, 3) | (BIT(data, 4)<<1) | (BIT(data, 5)<<2));
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
READ8_MEMBER( epson_lx810l_t::portb_r )
{
	UINT8 result = ~ioport("DIPSW1")->read();

	/* if 93C06 is selected */
	if (m_93c06_cs) {
		UINT8 do_r = m_eeprom->do_read();
		result &= 0xfe;
		result |= do_r;
	}

	LX810LLOG("%s: lx810l_PB_r(%02x): result %02x\n", machine().describe_context(), offset, result);

	return result;
}

WRITE8_MEMBER( epson_lx810l_t::portb_w )
{
	UINT8 data_in = BIT(data, 1);

	/* if 93C06 is selected */
	if (m_93c06_cs)
		m_eeprom->di_write(data_in);

	LX810LLOG("%s: lx810l_PB_w(%02x): %02x: 93c06 data %d\n", machine().describe_context(), offset, data, data_in);
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
READ8_MEMBER( epson_lx810l_t::portc_r )
{
	UINT8 result = 0;

	/* result |= ioport("serial")->read() << 1; */
	result |= !ioport("ONLINE")->read() << 3;
	result |= m_93c06_clk << 4;
	result |= m_93c06_cs  << 5;

	LX810LLOG("%s: lx810l_PC_r(%02x): %02x\n", machine().describe_context(), offset, result);

	return result;
}

WRITE8_MEMBER( epson_lx810l_t::portc_w )
{
	/* ioport("serial")->write(BIT(data, 0)); */

	m_93c06_clk =  BIT(data, 4);
	m_93c06_cs  = !BIT(data, 5);

	LX810LLOG("%s: PC_w(%02x): %02x 93c06 clk: %d cs: %d\n", machine().describe_context(), offset, data, m_93c06_clk, m_93c06_cs);

	m_eeprom->clk_write(m_93c06_clk ? ASSERT_LINE : CLEAR_LINE);
	m_eeprom->cs_write (m_93c06_cs  ? ASSERT_LINE : CLEAR_LINE);

	output_set_value("online_led", !BIT(data, 2));
}


/***************************************************************************
    GATE ARRAY
***************************************************************************/

WRITE16_MEMBER( epson_lx810l_t::printhead )
{
	m_printhead = data;
}

WRITE8_MEMBER( epson_lx810l_t::pf_stepper )
{
	int changed = m_pf_stepper->update(data);
	m_pf_pos_abs = -m_pf_stepper->get_absolute_position();

	/* clear last line of paper */
	if (changed > 0) {
		void *line = m_bitmap.raw_pixptr(bitmap_line(9), 0);
		memset(line, 0xff, m_bitmap.width() * 4);
	}

	LX810LLOG("%s: %s(%02x); abs %d\n", machine().describe_context(), __func__, data, m_pf_pos_abs);
}

WRITE8_MEMBER( epson_lx810l_t::cr_stepper )
{
	int m_cr_pos_abs_prev = m_cr_pos_abs;

	m_cr_stepper->update(data);
	m_cr_pos_abs = -m_cr_stepper->get_absolute_position();

	if (m_cr_pos_abs > m_cr_pos_abs_prev) {
		/* going right */
		m_real_cr_dir =  1;
	} else {
		/* going left */
		m_real_cr_dir = -1;
	}

	if (!m_real_cr_steps)
		timer_set(attotime::from_usec(400), TIMER_CR, m_real_cr_dir);
	m_real_cr_steps++;

	LX810LLOG("%s: %s(%02x); abs %d\n", machine().describe_context(), __func__, data, m_cr_pos_abs);
}

WRITE_LINE_MEMBER( epson_lx810l_t::e05a30_ready )
{
	m_maincpu->set_input_line(INPUT_LINE_NMI, PULSE_LINE);
}


/***************************************************************************
    Video hardware (simulates paper)
***************************************************************************/

UINT32 epson_lx810l_t::screen_update_lx810l(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	int scrolly = -bitmap_line(9);
	copyscrollbitmap(bitmap, m_bitmap, 0, NULL, 1, &scrolly, cliprect);

	/* draw "printhead" */
	bitmap.plot_box(m_real_cr_pos + CR_OFFSET - 10, PAPER_HEIGHT - 36, 20, 36, 0x888888);

	return 0;
}


/***************************************************************************
    Extended Timer Output
***************************************************************************/

WRITE_LINE_MEMBER( epson_lx810l_t::co0_w )
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
		if (m_real_cr_pos < m_bitmap.width()) {
			for (int i = 0; i < 9; i++) {
				unsigned int y = bitmap_line(i);
				if ((m_printhead & (1<<(8-i))) != 0)
					m_bitmap.pix32(y, m_real_cr_pos + CR_OFFSET) = 0x000000;
			}
		}
	}
}

WRITE_LINE_MEMBER( epson_lx810l_t::co1_w )
{
	m_dac->write_unsigned8(0 - !state);
}


/***************************************************************************
    ADC
***************************************************************************/

READ8_MEMBER(epson_lx810l_t::an0_r)
{
	UINT8 res = !!(ioport("DIPSW2")->read() & 0x01);
	return res - 1; /* DIPSW2.1 */
}

READ8_MEMBER(epson_lx810l_t::an1_r)
{
	UINT8 res = !!(ioport("DIPSW2")->read() & 0x02);
	return res - 1; /* DIPSW2.2 */
}

READ8_MEMBER(epson_lx810l_t::an2_r)
{
	UINT8 res = !!(ioport("DIPSW2")->read() & 0x04);
	return res - 1; /* DIPSW2.3 */
}

READ8_MEMBER(epson_lx810l_t::an3_r)
{
	UINT8 res = !!(ioport("DIPSW2")->read() & 0x08);
	return res - 1; /* DIPSW2.4 */
}

READ8_MEMBER(epson_lx810l_t::an4_r)
{
	return 0xff;
}

READ8_MEMBER(epson_lx810l_t::an5_r)
{
	return 0xCB; /* motor voltage, 0xcb = 24V */
}

READ8_MEMBER(epson_lx810l_t::an6_r)
{
	UINT8 res = !ioport("LOADEJECT")->read();
	return res - 1;
}

READ8_MEMBER(epson_lx810l_t::an7_r)
{
	return 0xff;
}
