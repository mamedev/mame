// license:BSD-3-Clause
// copyright-holders: 68bit
/******************************************************************************

Motorola Evaluation Kit 6802 D3 - MEK6802D3

Keypad commands:

RS (Reset)  - Reset, wired to the CPU reset line
EX (Escape) - Typically aborts user program, or exits command.
M  - Memory display/change
  Digits 5 and 6 show the entered data, and are blank unless it differs.
  Digits 7 and 8 show the actual data at the address.
  GO - increase the address.
  M  - decreases the address.
  FS - Offset calculation. Enter address, then press 'GO'.
  EX - exits memory display.
RD - Register display/alter
  RD  - advance to next register.
  T/B - trace a single instruction
  GO  - continue user code.
  EX  - exits register display.
GO - Go to user program.
  If no address is entered then it uses the pseudo PC, it continues.
  Enter the address and press 'Go' to use that entered address.
  It firstly checks that there is RAM at the stack pointer. ???
T/B - Trace one instruction.
FS T/B - Breakpoint editor
  GO - advance to next breakpoing, up to 8, then loops.
  FS - insert a breakpoint
  FC - deactivate breakpoint
  EX - exits breakpoing editor.
P/L - Punch tape
  The 300/1200 baud switch needs to be set, and the same rate selected.
  FS - Set 300 baud mode, J-BUG compatible.
  FC - Set 1200 baud mode, has checksum.
  At the 'b' prompt enter the beginning address of the data, then 'GO'.
  At the 'E' prompt enter the last address of the data, the 'GO' to
  start writing to the tape. There is a 30 second leader of $ff.
  M - Skip to Verify tape.
FS P/L - Load from tape
  FS - Set 300 baud mode, J-BUG compatible.
  FC - Set 1200 baud mode, has checksum.
  GO - Start load.
  M - Skip to Verify tape.
FS 0 to F
  One of 16 user defined functions. Press FS then one number key 0 to F.
  A pointer to a table of 16 function addresses should be set at 0x8102.
FC - Clears the 'FS' flag.


ASCII terminal commands:
B        - Breakpoing display, up to eight.
C        - Continue user program
D        - Delete all breakpoints
E <addr> - Examine a block of memory, with ASCII chars.
  Carriage Return - exit examine memory.
G <addr> - Go to the user program at <addr>.
  Keypad Escape - breaks execution.
L        - Load from audio tape.
           The 300/1200 baud switch needs to be set, and the same rate selected.
           The 300 baud rate is J-BUG compatible. The 1200 includes a checksum.
M <addr> - Memory display at <addr>
  Linefeed - Advance to next address.
  M or ^ - Previous address.
  Space  - Redisplay data.
  Carriage Return - exit memory display.
  O <addr> - Offset calculation.
N        - Trace one instruction.
O        - Offset load from tape.
P        - Punch tape.
R        - Register display and edit.
  Space  - Advance to next register.
  Carriage Return - exit register display.
  <nn>   - Change register value.
S <addr> - Set a breakpoint at the <addr>.
T <nn>   - Trace <nn> instructions.
U <addr> - Delete the breakpoint at <addr>
V        - Verify tape.
Ctrl-E   - Clear screen.
!        - Execute user function stored in user register 2, aka $816a
"        - Execute user function stored in user register 3, aka $816c
#        - Execute user function stored in user register 4, aka $816e

$FDFD    - Remove breakpoints. Documentation suggests assigning this address
           to a user function when debugging, to clean up breakpoints.


TODO

There are references to a MEK6802EA product, probably an Editor Assembler
that might have been for this system. Perhaps it was ROMable and that might
be nice to add here.

There was a color graphics evaluation board, the MEK68VDG, using the
MC6847. It must have had it's own expansion ROM as the D3DUG2 ROM for the
MEK68R2 is specific to the R2 and has no support for the 6847. This might be
a nice addition if documentation and/or a ROM can be found.

The RAM paging might need revising.

The ROM support is still TODO.

******************************************************************************/

#include "emu.h"
#include "cpu/m6800/m6800.h"
#include "machine/input_merger.h"
#include "machine/ram.h"
#include "machine/6821pia.h"
#include "machine/6850acia.h"
#include "machine/mc14411.h"
#include "machine/mc6846.h"
#include "machine/clock.h"
#include "machine/timer.h"
#include "video/pwm.h"
#include "bus/rs232/rs232.h"
#include "imagedev/cassette.h"
#include "sound/wave.h"
#include "speaker.h"
#include "mekd3.lh"

// MEK68R2
#include "machine/terminal.h"
#include "video/mc6845.h"
#include "emupal.h"
#include "screen.h"
#include "render.h"


namespace {

#define XTAL_MEKD3 3.579545_MHz_XTAL

class mekd3_state : public driver_device
{
public:
	mekd3_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_ram(*this, RAM_TAG)
		, m_ram_bank(*this, "ram_bank")
		, m_mainirq(*this, "mainirq")
		, m_mainnmi(*this, "mainnmi")
		, m_mc6846(*this, "mc6846")
		, m_kpd_pia(*this, "kpd_pia")
		, m_display(*this, "display")
		, m_keypad_columns(*this, "COL%u", 0)
		  // MEK68IO
		, m_pia_io1(*this, "pia_io1")
		, m_pia_io2(*this, "pia_io2")
		, m_acia_io1(*this, "acia_io1")
		, m_acia_cas(*this, "acia_cas")
		, m_brg(*this, "brg")
		, m_cass(*this, "cassette")
		, m_console_enable(*this, "CONSOLE_ENABLE")
		, m_rs232_tx_baud(*this, "RS232_TX_BAUD")
		, m_rs232_rx_baud(*this, "RS232_RX_BAUD")
		, m_rs232_cts_route(*this, "RS232_CTS_ROUTE")
		, m_rs232_dcd_route(*this, "RS232_DCD_ROUTE")
		, m_cas_baud(*this, "CAS_BAUD")
		  // MEK68R2
		, m_mc6845(*this, "mc6845")
		, m_palette(*this, "palette")
		, m_screen(*this, "screen")
		, m_p_chargen(*this, "chargen")
		, m_video_ram(*this, "videoram")
		, m_r2_pia(*this, "r2_pia")
		, m_r2_mode(*this, "R2_MODE")
		, m_r2_display_nationality(*this, "R2_DISPLAY_NATIONALITY")
		, m_r2_display_format(*this, "R2_DISPLAY_FORMAT")
	{ }

	void mekd3(machine_config &config);
	void init_mekd3();

	void reset_key_w(int state);
	DECLARE_INPUT_CHANGED_MEMBER(keypad_changed);
	DECLARE_INPUT_CHANGED_MEMBER(rs232_cts_route_change);
	DECLARE_INPUT_CHANGED_MEMBER(rs232_dcd_route_change);

private:
	int keypad_cb1_r();
	uint8_t keypad_key_r();
	void led_digit_w(uint8_t data);
	void led_segment_w(uint8_t data);
	TIMER_DEVICE_CALLBACK_MEMBER(led_update);
	void page_w(uint8_t data);

	required_device<m6802_cpu_device> m_maincpu;
	required_device<ram_device> m_ram;
	required_memory_bank m_ram_bank;
	required_device<input_merger_device> m_mainirq;
	required_device<input_merger_device> m_mainnmi;
	required_device<mc6846_device> m_mc6846;
	required_device<pia6821_device> m_kpd_pia;
	required_device<pwm_display_device> m_display;
	required_ioport_array<4> m_keypad_columns;

	uint8_t m_rom_page;
	uint8_t m_ram_page;
	uint8_t m_segment;
	uint8_t m_digit;

	bool keypad_key_pressed();

	void mekd3_mem(address_map &map) ATTR_COLD;
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	// MEK68IO

	void rs232_route_cts(int state);
	void rs232_route_dcd(int state);
	int m_cts;
	int m_dcd;

	// Clocks
	void write_f1_clock(int state);
	void write_f2_clock(int state);
	void write_f4_clock(int state);
	void write_f5_clock(int state);
	void write_f7_clock(int state);
	void write_f8_clock(int state);
	void write_f9_clock(int state);
	void write_f13_clock(int state);

	TIMER_DEVICE_CALLBACK_MEMBER(kansas_r);
	void acia_cas_clock300_w(int state);
	void acia_cas_clock1200_w(int state);
	uint8_t pia_io2a_r();

	required_device<pia6821_device> m_pia_io1;
	required_device<pia6821_device> m_pia_io2;
	required_device<acia6850_device> m_acia_io1;
	required_device<acia6850_device> m_acia_cas;
	required_device<mc14411_device> m_brg;
	required_device<cassette_image_device> m_cass;
	required_ioport m_console_enable;
	required_ioport m_rs232_tx_baud;
	required_ioport m_rs232_rx_baud;
	required_ioport m_rs232_cts_route;
	required_ioport m_rs232_dcd_route;
	required_ioport m_cas_baud;
	uint8_t m_cass_rx_period, m_cass_txcount;
	bool m_cass_in, m_cass_inbit, m_cass_txbit, m_cass_last_txbit;

	// MEK68R2
	MC6845_UPDATE_ROW(update_row);
	uint8_t r2_pia_pa_r();
	uint8_t r2_pia_pb_r();
	optional_device<mc6845_device> m_mc6845;
	optional_device<palette_device> m_palette;
	optional_device<screen_device> m_screen;
	optional_region_ptr<uint8_t> m_p_chargen;
	optional_shared_ptr<uint8_t> m_video_ram;
	optional_device<pia6821_device> m_r2_pia;
	optional_ioport m_r2_mode;
	optional_ioport m_r2_display_nationality;
	optional_ioport m_r2_display_format;
	void kbd_put(uint8_t data);
	uint8_t m_term_data;
};



/***********************************************************

    Address Map

************************************************************/

void mekd3_state::mekd3_mem(address_map &map)
{
	// User RAM banks
	map(0x0000, 0x7fff).bankrw("ram_bank");

	// MEK68IO User PIA
	map(0x8000, 0x8003).rw(m_pia_io1, FUNC(pia6821_device::read), FUNC(pia6821_device::write));
	// MEK68IO D3BUG cassette PIA
	map(0x8004, 0x8007).rw(m_pia_io2, FUNC(pia6821_device::read), FUNC(pia6821_device::write));

	// MEK68IO RS232 ACIA
	map(0x8008, 0x8009).rw(m_acia_io1, FUNC(acia6850_device::read), FUNC(acia6850_device::write));
	// MEK68IO cassette ACIA.
	map(0x800a, 0x800b).rw(m_acia_cas, FUNC(acia6850_device::read), FUNC(acia6850_device::write));

	// 8040-8041 MEK68VG VDG Scroll register

	// 8042-8043 MEK68R2 CRT register.
	map(0x8042, 0x8042).w(m_mc6845, FUNC(mc6845_device::address_w));
	map(0x8043, 0x8043).rw(m_mc6845, FUNC(mc6845_device::register_r), FUNC(mc6845_device::register_w));

	// MEK68R2 PIA (Keyboard)
	map(0x8044, 0x8047).rw(m_r2_pia, FUNC(pia6821_device::read), FUNC(pia6821_device::write));
	// GPIA IEEE-488 8048-804F

	// 8050-8057 MEK68EP PROM Programmer

	// MEK6802D3 timer and I/O.
	map(0x8080, 0x8087).rw(m_mc6846, FUNC(mc6846_device::read), FUNC(mc6846_device::write));
	// MEK6802D3 display/keyboard PIA
	map(0x8088, 0x808b).rw(m_kpd_pia, FUNC(pia6821_device::read), FUNC(pia6821_device::write));

	// MEK6802D3 MCM6810 RAM
	map(0x8100, 0x81ff).ram(); // system ram

	// 8200 - 87ff  User defined RAM

	// 8800 - 8fff  MEK68VDG RAM

	// 9000-9fff MEK68R2 and MEK68VDG Video RAM
	map(0x9000, 0x9fff).ram().share(m_video_ram);

	// a000-efff ROM banks.

	// D3BUG2 ROM
	map(0xf000, 0xf7ff).rom();

	// MEK6802D3 D3BUG ROM, MC6846 ROM
	map(0xf800, 0xffff).rom();
}

/***********************************************************

    Keys

************************************************************/

static INPUT_PORTS_START(mekd3)
	// RESET is not wired to the key matrix.
	PORT_START("RESET")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("RS") PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, mekd3_state, reset_key_w)

	// PORT_CODEs are not assigned to the keypad to allow it on screen at
	// the same time as the terminal or CRT console which also receive
	// keyboard inputs. When a keyboard is available the keypad is of
	// limited use, but still useful to interrupt code or reset the
	// machine. If MAME someday allows the keyboard input focus to be
	// switched then this might be redesigned.
	PORT_START("COL0")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, mekd3_state, keypad_changed, 0) PORT_NAME("M")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, mekd3_state, keypad_changed, 0) PORT_NAME("FS")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, mekd3_state, keypad_changed, 0) PORT_NAME("7")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, mekd3_state, keypad_changed, 0) PORT_NAME("4")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, mekd3_state, keypad_changed, 0) PORT_NAME("1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, mekd3_state, keypad_changed, 0) PORT_NAME("0")

	PORT_START("COL1")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, mekd3_state, keypad_changed, 0) PORT_NAME("EX")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, mekd3_state, keypad_changed, 0) PORT_NAME("FC")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, mekd3_state, keypad_changed, 0) PORT_NAME("8")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, mekd3_state, keypad_changed, 0) PORT_NAME("5")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, mekd3_state, keypad_changed, 0) PORT_NAME("2")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, mekd3_state, keypad_changed, 0) PORT_NAME("F")

	PORT_START("COL2")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, mekd3_state, keypad_changed, 0) PORT_NAME("RD")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, mekd3_state, keypad_changed, 0) PORT_NAME("P/L")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, mekd3_state, keypad_changed, 0) PORT_NAME("9")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, mekd3_state, keypad_changed, 0) PORT_NAME("6")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, mekd3_state, keypad_changed, 0) PORT_NAME("3")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, mekd3_state, keypad_changed, 0) PORT_NAME("E")

	PORT_START("COL3")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, mekd3_state, keypad_changed, 0) PORT_NAME("GO")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, mekd3_state, keypad_changed, 0) PORT_NAME("T/B")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, mekd3_state, keypad_changed, 0) PORT_NAME("A")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, mekd3_state, keypad_changed, 0) PORT_NAME("B")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, mekd3_state, keypad_changed, 0) PORT_NAME("C")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, mekd3_state, keypad_changed, 0) PORT_NAME("D")


	// MEK68IO

	PORT_START("CONSOLE_ENABLE")
	PORT_DIPNAME(0x01, 0x00, "RS-232 console")
	PORT_DIPSETTING(0x01, DEF_STR(On))
	PORT_DIPSETTING(0x00, DEF_STR(Off))

	// RS232 baud rates available for use at socket 1.
	PORT_START("RS232_TX_BAUD")
	PORT_CONFNAME(0x3f, 1, "RS232 TX Baud Rate")
	PORT_CONFSETTING(0x80, "110")
	PORT_CONFSETTING(0x40, "300")
	PORT_CONFSETTING(0x20, "600")
	PORT_CONFSETTING(0x10, "1200")
	PORT_CONFSETTING(0x08, "2400")
	PORT_CONFSETTING(0x04, "3600")
	PORT_CONFSETTING(0x02, "4800")
	PORT_CONFSETTING(0x01, "9600")

	PORT_START("RS232_RX_BAUD")
	PORT_CONFNAME(0x3f, 1, "RS232 RX Baud Rate")
	PORT_CONFSETTING(0x80, "110")
	PORT_CONFSETTING(0x40, "300")
	PORT_CONFSETTING(0x20, "600")
	PORT_CONFSETTING(0x10, "1200")
	PORT_CONFSETTING(0x08, "2400")
	PORT_CONFSETTING(0x04, "3600")
	PORT_CONFSETTING(0x02, "4800")
	PORT_CONFSETTING(0x01, "9600")

	// RS232 CTS and DCD routing at socket 3. These need to be
	// jumpered low if not driven by the RS232 device.
	PORT_START("RS232_CTS_ROUTE")
	PORT_CONFNAME(0x1, 0, "RS232 CTS") PORT_CHANGED_MEMBER(DEVICE_SELF, mekd3_state, rs232_cts_route_change, 0)
	PORT_CONFSETTING(0, "Jumper Low")
	PORT_CONFSETTING(1, "Pass Through")
	PORT_START("RS232_DCD_ROUTE")
	PORT_CONFNAME(0x1, 0, "RS232 DCD") PORT_CHANGED_MEMBER(DEVICE_SELF, mekd3_state, rs232_dcd_route_change, 0)
	PORT_CONFSETTING(0, "Jumper Low")
	PORT_CONFSETTING(1, "Pass Through")

	// A mechanical switch at the edge of the board set the cassette baud
	// rate. The software could not control or read this rate setting and
	// since D3BUG used a different format for 1200 baud it had to promote
	// for the rate too.
	PORT_START("CAS_BAUD")
	PORT_CONFNAME(0x01, 0, "Cassette Baud Rate")
	PORT_CONFSETTING(0x1, "1200")
	PORT_CONFSETTING(0x0, "300")


	// MEK68R2

	PORT_START("R2_MODE")
	PORT_DIPNAME(0x1, 0, "R2 Mode")
	PORT_DIPSETTING(0, "Normal")
	PORT_DIPSETTING(1, "Dumb terminal")

	PORT_START("R2_DISPLAY_NATIONALITY")
	PORT_DIPNAME(0x1, 1, "Display nationality")
	PORT_DIPSETTING(0, "US")
	PORT_DIPSETTING(1, "Europe")

	PORT_START("R2_DISPLAY_FORMAT")
	PORT_DIPNAME(0x0003, 2, "Display format")
	PORT_DIPSETTING(0, "16 lines of 32 characters")
	PORT_DIPSETTING(1, "16 lines of 64 characters")
	PORT_DIPSETTING(2, "20 lines of 80 characters")
	PORT_DIPSETTING(3, "User defined")

INPUT_PORTS_END


/***********************************************************

 RAM and ROM paging

************************************************************/

void mekd3_state::page_w(uint8_t data)
{
	m_rom_page = data & 0x07;
	// TODO switch the ROM bank entry.
	m_ram_page = (data >> 3) & 0x07;
	m_ram_bank->set_entry(m_ram_page);
}

/***********************************************************

    Keypad

************************************************************/

void mekd3_state::reset_key_w(int state)
{
	m_maincpu->set_input_line(INPUT_LINE_RESET, state ? CLEAR_LINE : ASSERT_LINE);

	if (!state)
	{
		m_mc6846->reset();
		m_kpd_pia->reset();
		m_kpd_pia->write(1, 2);
		// MEK68IO
		m_pia_io1->reset();
		m_pia_io2->reset();
		m_acia_io1->reset();
		m_acia_cas->reset();
		m_cass_rx_period = 0;
		m_cass_txcount = 0;
		m_cass_in = 0;
		m_cass_inbit = 0;
		// MEK68R2
		m_mc6845->reset();
		m_r2_pia->reset();
	}
}

bool mekd3_state::keypad_key_pressed()
{
	return (m_keypad_columns[0]->read() & m_digit) ||
		(m_keypad_columns[1]->read() & m_digit) ||
		(m_keypad_columns[2]->read() & m_digit) ||
		(m_keypad_columns[3]->read() & m_digit);
}

INPUT_CHANGED_MEMBER(mekd3_state::keypad_changed)
{
	m_kpd_pia->cb1_w(mekd3_state::keypad_key_pressed());
}

int mekd3_state::keypad_cb1_r()
{
	return mekd3_state::keypad_key_pressed();
}

uint8_t mekd3_state::keypad_key_r()
{
	uint8_t mux = (m_digit & 0xc0) >> 6;
	uint8_t i = (m_keypad_columns[mux]->read() & m_digit) ? 0 : 0x80;

	return i | m_segment;
}

/***********************************************************

    Seven segment LED display

************************************************************/

// PA
void mekd3_state::led_segment_w(uint8_t data)
{
	m_segment = data & 0x7f;
	m_display->matrix(m_digit, m_segment);
}

// PB
void mekd3_state::led_digit_w(uint8_t data)
{
	m_digit = data;
	m_display->matrix(m_digit, m_segment);
	// Update the keypad pressed output which depends on m_digit.
	m_kpd_pia->cb1_w(mekd3_state::keypad_key_pressed());
}

/***********************************************************

  MEK68IO

************************************************************/

void mekd3_state::rs232_route_cts(int state)
{
	if (m_rs232_cts_route->read())
		m_acia_io1->write_cts(state);

	// Cache the state, in case the ioport setting changes.
	m_cts = state;
}

void mekd3_state::rs232_route_dcd(int state)
{
	if (m_rs232_dcd_route->read())
		m_acia_io1->write_dcd(state);

	// Cache the state, in case the ioport setting changes.
	m_dcd = state;
}

INPUT_CHANGED_MEMBER(mekd3_state::rs232_cts_route_change)
{
	if (newval)
		m_acia_io1->write_cts(m_cts);
	else
		m_acia_io1->write_cts(0);
}

INPUT_CHANGED_MEMBER(mekd3_state::rs232_dcd_route_change)
{
	if (newval)
		m_acia_io1->write_dcd(m_dcd);
	else
		m_acia_io1->write_dcd(0);
}

void mekd3_state::write_f1_clock(int state)
{
	if (BIT(m_rs232_tx_baud->read(), 0))
		m_acia_io1->write_txc(state);
	if (BIT(m_rs232_rx_baud->read(), 0))
		m_acia_io1->write_rxc(state);
}

void mekd3_state::write_f2_clock(int state)
{
	if (BIT(m_rs232_tx_baud->read(), 1))
		m_acia_io1->write_txc(state);
	if (BIT(m_rs232_rx_baud->read(), 1))
		m_acia_io1->write_rxc(state);
}

void mekd3_state::write_f4_clock(int state)
{
	if (BIT(m_rs232_tx_baud->read(), 2))
		m_acia_io1->write_txc(state);
	if (BIT(m_rs232_rx_baud->read(), 2))
		m_acia_io1->write_rxc(state);
}

void mekd3_state::write_f5_clock(int state)
{
	if (BIT(m_rs232_tx_baud->read(), 3))
		m_acia_io1->write_txc(state);
	if (BIT(m_rs232_rx_baud->read(), 3))
		m_acia_io1->write_rxc(state);
}

void mekd3_state::write_f7_clock(int state)
{
	if (BIT(m_rs232_tx_baud->read(), 4))
		m_acia_io1->write_txc(state);
	if (BIT(m_rs232_rx_baud->read(), 4))
		m_acia_io1->write_rxc(state);

	// 1200 baud also drives the cassette ACIA
	if (m_cas_baud->read() == 1)
		acia_cas_clock1200_w(state);
}

void mekd3_state::write_f8_clock(int state)
{
	if (BIT(m_rs232_tx_baud->read(), 5))
		m_acia_io1->write_txc(state);
	if (BIT(m_rs232_rx_baud->read(), 5))
		m_acia_io1->write_rxc(state);
}

void mekd3_state::write_f9_clock(int state)
{
	if (BIT(m_rs232_tx_baud->read(), 6))
		m_acia_io1->write_txc(state);
	if (BIT(m_rs232_rx_baud->read(), 6))
		m_acia_io1->write_rxc(state);

	// 300 baud also drives the cassette ACIA
	if (m_cas_baud->read() == 0)
		acia_cas_clock300_w(state);
}

void mekd3_state::write_f13_clock(int state)
{
	if (BIT(m_rs232_tx_baud->read(), 7))
		m_acia_io1->write_txc(state);
	if (BIT(m_rs232_rx_baud->read(), 7))
		m_acia_io1->write_rxc(state);
}


TIMER_DEVICE_CALLBACK_MEMBER(mekd3_state::kansas_r)
{
	m_cass_rx_period++;

	// Turn 1200/2400Hz to a bit
	uint8_t cassin = (m_cass->input() > +0.04) ? 1 : 0;
	uint8_t inbit = m_cass_inbit;

	if (cassin != m_cass_in)
	{
		// Transition, now check the period.
		inbit = (m_cass_rx_period < 12) ? 1 : 0;
		m_cass_in = cassin;
		m_cass_rx_period = 0;
	}
	else if (m_cass_rx_period > 32)
	{
		// Idle the ACIA if there is no data.
		m_cass_rx_period = 32;
		inbit = 1;
	}

	if (inbit != m_cass_inbit)
	{
		m_acia_cas->write_rxd(inbit);
		m_cass_inbit = inbit;
	}
}

void mekd3_state::acia_cas_clock300_w(int state)
{
	// The Kansas City cassette format encodes a '0' bit by four cycles of
	// a 1200 Hz sine wave, and a '1' bit as eight cycles of 2400 Hz,
	// giving a 300 baud rate.
	//
	// The clock rate to the ACIA is 16x the baud rate and is divided by 2
	// to get the 2400 Hz rate, or divided by 4 to get the 1200 Hz rate.

	// Sync the period phase on TX bit transitions.
	if (m_cass_txbit != m_cass_last_txbit)
	{
		m_cass_txcount = 0;
		m_cass_last_txbit = m_cass_txbit;
	}

	if (m_cass_txbit)
		m_cass->output(BIT(m_cass_txcount, 1) ? +1.0 : -1.0); // 2400Hz
	else
		m_cass->output(BIT(m_cass_txcount, 2) ? +1.0 : -1.0); // 1200Hz

	m_cass_txcount++;
	m_acia_cas->write_txc(state);
	m_acia_cas->write_rxc(state);
}

void mekd3_state::acia_cas_clock1200_w(int state)
{
	// For the 1200 baud rate the number of cycles in reduced to just one
	// cycle at 1200 Hz and two at 2400 Hz.

	// Sync the period phase on TX bit transitions.
	if (m_cass_txbit != m_cass_last_txbit)
	{
		m_cass_txcount = 0;
		m_cass_last_txbit = m_cass_txbit;
	}

	if (m_cass_txbit)
		m_cass->output(BIT(m_cass_txcount, 3) ? +1.0 : -1.0); // 2400Hz
	else
		m_cass->output(BIT(m_cass_txcount, 4) ? +1.0 : -1.0); // 1200Hz

	m_cass_txcount++;
	m_acia_cas->write_txc(state);
	m_acia_cas->write_rxc(state);
}

uint8_t mekd3_state::pia_io2a_r()
{
	uint32_t console_enable = m_console_enable->read();

	// bit 1 (0x02) is a jumper to set RS232 terminal mode when set.

	return console_enable ? 0x02 : 0x00;
}


/***********************************************************

  MEK68R2

  This might in future be moved to a slot device, on a bus.

************************************************************/

// Delivery of keyboard inputs to the MEK68R2 keyboard is disabled when on
// views with the RS232 terminal, assuming that this keyboard is not present.
// Also disable delivery when the 'MEK68R2 present' jumper indicates it is
// disabled, assuming that this keyboard is not present.
void mekd3_state::kbd_put(uint8_t data)
{
	uint8_t view = machine().render().first_target()->view();
	if (view == 0)
		return;

	m_term_data = data;
	// Triggers on the falling edge.
	m_r2_pia->ca1_w(ASSERT_LINE);
	m_r2_pia->ca1_w(CLEAR_LINE);
	m_r2_pia->ca1_w(ASSERT_LINE);
}

// PA0 to PA6 - Keyboard data.
// PA7 - Display nationality, 0 USA, 1 Europe.
uint8_t mekd3_state::r2_pia_pa_r()
{
	uint8_t ret = m_term_data;
	int8_t display_nationality = m_r2_display_nationality->read();
	m_term_data = 0;
	return ret | (display_nationality << 7);
}

// PB0 - Mode: 0 normal, 1 dumb terminal.
// PB1,2,3 - N/C
// PB4 - User defined
// PB5 - Light pen control.
// PB7, PB6 - Display format.
//       00 - 16 lines of 32 characters.
//       01 - 16 lines of 64 characters.
//       10 - 20 lines of 80 characters.
//       11 - User defined.
uint8_t mekd3_state::r2_pia_pb_r()
{
	int8_t display_format = m_r2_display_format->read();
	int8_t mode = m_r2_mode->read();
	return (display_format << 6) | mode;
}

MC6845_UPDATE_ROW(mekd3_state::update_row)
{
	const pen_t *pen = m_palette->pens();

	int x = 0;

	for (int column = 0; column < x_count; column++)
	{
		uint8_t code = m_video_ram[(ma + column) & 0xfff];
		int dcursor = (column == cursor_x);

		if (BIT(code, 7))
		{
			/* Lores 6 pixel character.
			     -----------
			     | D1 | D0 |
			     | D3 | D2 |
			     | D5 | D4 |
			     -----------
			     D6 - 1 Grey tone, 0 brightness.
			*/
			int pixel = ((ra & 0x0c) >> 1) + 1;
			int dout = BIT(code, pixel);
			int grey = BIT(code, 6);
			int color = ((dcursor ^ dout) && de) << (grey ^ 1);
			bitmap.pix(y, x++) = pen[color];
			bitmap.pix(y, x++) = pen[color];
			bitmap.pix(y, x++) = pen[color];
			bitmap.pix(y, x++) = pen[color];
			pixel--;
			dout = BIT(code, pixel);
			color = ((dcursor ^ dout) && de) << (grey ^ 1);
			bitmap.pix(y, x++) = pen[color];
			bitmap.pix(y, x++) = pen[color];
			bitmap.pix(y, x++) = pen[color];
			bitmap.pix(y, x++) = pen[color];
		}
		else
		{
			offs_t address = ra < 8 ? ((code & 0x7f) << 3) | (ra & 0x07) : 0;
			uint8_t data = m_p_chargen[address];

			for (int bit = 0; bit < 8; bit++)
			{
				int dout = BIT(data, 7);
				int color = ((dcursor ^ dout) && de) << 1;

				bitmap.pix(y, x++) = pen[color];

				data <<= 1;
			}
		}
	}
}

/***********************************************************

************************************************************/

void mekd3_state::init_mekd3()
{
	uint8_t* ROM = memregion("maincpu")->base();

	// Hack to the trace timer delay, which is 0x000e by default, but
	// that did not produce proper timing for tracing.
	// 15 half works sometimes??
	// 16 to 19 half works, gets to user code but not to the next instruction!
	// 20 will step a nop but not a branch.
	// 21 also steps a branch.
	// 22 can overstep!
	ROM[0xf80c] = 21;
}

void mekd3_state::machine_start()
{
	uint8_t* RAM = m_ram->pointer();
	m_ram_bank->configure_entries(0, 8, RAM, 0x8000);

	save_item(NAME(m_rom_page));
	save_item(NAME(m_ram_page));
	save_item(NAME(m_segment));
	save_item(NAME(m_digit));
	save_item(NAME(m_cts));
	save_item(NAME(m_dcd));
	save_item(NAME(m_cass_rx_period));
	save_item(NAME(m_cass_txcount));
	save_item(NAME(m_cass_in));
	save_item(NAME(m_cass_inbit));
	save_item(NAME(m_cass_txbit));
	save_item(NAME(m_cass_last_txbit));
	save_item(NAME(m_term_data));
}

void mekd3_state::machine_reset()
{
	m_rom_page = 0;
	m_ram_page = 0;
	m_ram_bank->set_entry(0);

	// Avoid triggering an early interrupt when CA1 lowered. The mc6821
	// driver resets CA1 high and to trigger on a high to low
	// transition. The mekd3 programs CA1 to trigger on a low to high
	// transition and configuring this earlier here is adequate.
	m_kpd_pia->write(1, 2);

	m_brg->rsa_w(CLEAR_LINE);
	m_brg->rsb_w(ASSERT_LINE);

	m_cass_rx_period = 0;
	m_cass_txcount = 0;
	m_cass_in = 0;
	m_cass_inbit = 0;

	// Write low here if jumpered low.
	if (!m_rs232_cts_route->read())
		m_acia_io1->write_cts(0);
	if (!m_rs232_dcd_route->read())
		m_acia_io1->write_dcd(0);

	// MEK68R2
	m_r2_pia->ca1_w(ASSERT_LINE);
	m_r2_pia->ca2_w(ASSERT_LINE);
	m_r2_pia->cb1_w(0);
	m_r2_pia->cb2_w(0);
}

/***********************************************************

    Machine

************************************************************/

static DEVICE_INPUT_DEFAULTS_START(terminal)
	DEVICE_INPUT_DEFAULTS("RS232_RXBAUD", 0xff, RS232_BAUD_9600)
	DEVICE_INPUT_DEFAULTS("RS232_TXBAUD", 0xff, RS232_BAUD_9600)
	DEVICE_INPUT_DEFAULTS("RS232_DATABITS", 0xff, RS232_DATABITS_8)
	DEVICE_INPUT_DEFAULTS("RS232_PARITY", 0xff, RS232_PARITY_NONE)
	DEVICE_INPUT_DEFAULTS("RS232_STOPBITS", 0xff, RS232_STOPBITS_1)
DEVICE_INPUT_DEFAULTS_END

void mekd3_state::mekd3(machine_config &config)
{
	M6802(config, m_maincpu, XTAL_MEKD3);        // 894.8 kHz clock
	m_maincpu->set_ram_enable(false);
	m_maincpu->set_addrmap(AS_PROGRAM, &mekd3_state::mekd3_mem);

	RAM(config, m_ram).set_default_size("256K").set_default_value(0);

	INPUT_MERGER_ANY_HIGH(config, m_mainirq).output_handler().set_inputline(m_maincpu, M6802_IRQ_LINE);
	INPUT_MERGER_ANY_HIGH(config, m_mainnmi).output_handler().set_inputline(m_maincpu, INPUT_LINE_NMI);

	// LED display
	PWM_DISPLAY(config, m_display).set_size(8, 7);
	m_display->set_segmask(0xff, 0x7f);

	config.set_default_layout(layout_mekd3);

	SPEAKER(config, "mono").front_center();

	CASSETTE(config, m_cass);
	m_cass->set_default_state(CASSETTE_STOPPED | CASSETTE_MOTOR_ENABLED | CASSETTE_SPEAKER_ENABLED);
	m_cass->add_route(ALL_OUTPUTS, "mono", 0.05);

	// Keypad and display PIA. CA2 and CB2 are NC.
	PIA6821(config, m_kpd_pia);
	m_kpd_pia->readpa_handler().set(FUNC(mekd3_state::keypad_key_r));
	m_kpd_pia->readcb1_handler().set(FUNC(mekd3_state::keypad_cb1_r));
	m_kpd_pia->writepa_handler().set(FUNC(mekd3_state::led_segment_w));
	m_kpd_pia->writepb_handler().set(FUNC(mekd3_state::led_digit_w));
	m_kpd_pia->ca1_w(0);
	m_kpd_pia->irqa_handler().set(m_mainnmi, FUNC(input_merger_device::in_w<0>));
	m_kpd_pia->irqb_handler().set(m_mainnmi, FUNC(input_merger_device::in_w<1>));

	// CP1, CP2, /CTG, /CTC are available at SK1, and not used here.
	MC6846(config, m_mc6846, XTAL_MEKD3 / 4);  // Same as the cpu clock
	m_mc6846->out_port().set(FUNC(mekd3_state::page_w));
	m_mc6846->cto().set(m_kpd_pia, FUNC(pia6821_device::ca1_w)); // trace timer
	m_mc6846->irq().set(m_mainirq, FUNC(input_merger_device::in_w<0>));

	// MEK68IO

	// A 'user' PIA, I/O available at SK6.
	PIA6821(config, m_pia_io1);
	m_pia_io1->irqa_handler().set(m_mainnmi, FUNC(input_merger_device::in_w<2>));
	m_pia_io1->irqb_handler().set(m_mainirq, FUNC(input_merger_device::in_w<1>));

	// Largely a 'user' PIA, I/O available at SK5.
	// PA0 can optionally be an audio bit input, at TP1.
	// PA1 is a jumper mode input, and low by default.
	// PA2 can optionally be an audio bit output, at TP2.
	PIA6821(config, m_pia_io2);
	m_pia_io2->readpa_handler().set(FUNC(mekd3_state::pia_io2a_r));
	m_pia_io2->irqa_handler().set(m_mainirq, FUNC(input_merger_device::in_w<2>));
	m_pia_io2->irqb_handler().set(m_mainirq, FUNC(input_merger_device::in_w<3>));

	// RS232 ACIA
	// /RTS, /CTS and /DCD are available at SK3.
	ACIA6850(config, m_acia_io1, 0);
	m_acia_io1->txd_handler().set("rs232", FUNC(rs232_port_device::write_txd));
	m_acia_io1->rts_handler().set("rs232", FUNC(rs232_port_device::write_rts));
	m_acia_io1->irq_handler().set(m_mainirq, FUNC(input_merger_device::in_w<4>));

	MC14411(config, m_brg, XTAL(1'843'200));
	m_brg->out_f<1>().set(FUNC(mekd3_state::write_f1_clock));
	m_brg->out_f<2>().set(FUNC(mekd3_state::write_f2_clock));
	m_brg->out_f<4>().set(FUNC(mekd3_state::write_f4_clock));
	m_brg->out_f<4>().set(FUNC(mekd3_state::write_f5_clock));
	m_brg->out_f<7>().set(FUNC(mekd3_state::write_f7_clock));
	m_brg->out_f<8>().set(FUNC(mekd3_state::write_f8_clock));
	m_brg->out_f<9>().set(FUNC(mekd3_state::write_f9_clock));
	m_brg->out_f<13>().set(FUNC(mekd3_state::write_f13_clock));

	rs232_port_device &rs232(RS232_PORT(config, "rs232", default_rs232_devices, "terminal"));
	rs232.rxd_handler().set(m_acia_io1, FUNC(acia6850_device::write_rxd));
	rs232.cts_handler().set(FUNC(mekd3_state::rs232_route_cts));
	rs232.dcd_handler().set(FUNC(mekd3_state::rs232_route_dcd));
	rs232.set_option_device_input_defaults("terminal", DEVICE_INPUT_DEFAULTS_NAME(terminal));

	// /RTS is available at SK2.
	// /CTS and /DCD are available at SK2, or can be jumpered low.
	ACIA6850(config, m_acia_cas, 0);
	m_acia_cas->txd_handler().set([this] (bool state) { m_cass_txbit = state; });
	m_acia_cas->irq_handler().set(m_mainirq, FUNC(input_merger_device::in_w<5>));

	TIMER(config, "kansas_r").configure_periodic(FUNC(mekd3_state::kansas_r), attotime::from_hz(40000));

	// MEK68R2

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);

	m_screen->set_refresh_hz(50);
	m_screen->set_size(80 * 8 + 80 * 10, 20 * 12 + 100);
	m_screen->set_visarea(0, 80 * 8 + 80 * 10 - 1, 0, 20 * 12 + 100 - 1);
	m_screen->set_screen_update("mc6845", FUNC(mc6845_device::screen_update));

	PALETTE(config, m_palette, palette_device::MONOCHROME_HIGHLIGHT);

	MC6845(config, m_mc6845, XTAL(14'318'181)/8);
	m_mc6845->set_screen(m_screen);
	//
	m_mc6845->set_show_border_area(false);
	m_mc6845->set_char_width(8);
	m_mc6845->set_update_row_callback(FUNC(mekd3_state::update_row));
	m_mc6845->out_hsync_callback().set(m_r2_pia, FUNC(pia6821_device::cb2_w));
	m_mc6845->out_vsync_callback().set(m_r2_pia, FUNC(pia6821_device::cb1_w));

	// PA is the keyboard data and a mode flag.
	// CA1 is keyboard strobe.
	// CA2 light pen input.
	// PB0 is mode flags and light pen control.
	// CB1 is VSYNC, and CB2 is HSYNC.
	PIA6821(config, m_r2_pia);
	m_r2_pia->readpa_handler().set(FUNC(mekd3_state::r2_pia_pa_r));
	m_r2_pia->readpb_handler().set(FUNC(mekd3_state::r2_pia_pb_r));
	m_r2_pia->irqa_handler().set(m_mainirq, FUNC(input_merger_device::in_w<6>));
	m_r2_pia->irqb_handler().set(m_mainirq, FUNC(input_merger_device::in_w<7>));

	generic_keyboard_device &keyboard(GENERIC_KEYBOARD(config, "keyboard", 0));
	keyboard.set_keyboard_callback(FUNC(mekd3_state::kbd_put));
}

/***********************************************************

    ROMS

************************************************************/

ROM_START(mekd3)
	ROM_REGION(0x10000,"maincpu",0)
	ROM_LOAD("d3bug.rom", 0xf800, 0x0800, CRC(57863614) SHA1(b31679df86367d1e48e12f01a22cd0f008e74df4))
	ROM_LOAD("d3bug2.rom", 0xf000, 0x0800, CRC(bf3640b0) SHA1(374362c4464ab3986af2f08395bf254d1ce7a52f))
	ROM_REGION(0x0400, "chargen",0)
	ROM_LOAD("mcm6674p.chr", 0x0000, 0x0400, CRC(1c22088a) SHA1(b5f0bd0cfdec0cd5c1cb764506bef3c17d6af0eb))
ROM_END

} // anonymous namespace


/***************************************************************************

  Game driver(s)

***************************************************************************/

//    YEAR  NAME    PARENT  COMPAT  MACHINE   INPUT  CLASS        INIT        COMPANY     FULLNAME      FLAGS
COMP( 1978, mekd3,  0,      0,      mekd3,    mekd3, mekd3_state, init_mekd3, "Motorola", "MEK6802D3" , MACHINE_NO_SOUND )
