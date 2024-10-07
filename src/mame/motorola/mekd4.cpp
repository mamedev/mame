// license:BSD-3-Clause
// copyright-holders: 68bit
/******************************************************************************

Motorola Evaluation Kit 6809 D4 - MEK6809D4

Memory map

Range    Short  Description

0000-0fff RAM   User RAM, 4K, default location. It can be remapped to any 1K
                address boundary and on any RAM page, or disabled.

0000-7fff RAM   Off board RAM typically maps in this range, on any RAM page.

9000-9fff RAM   MEK68R2 Screen RAM

e000-e07f I/O   I/O1 Select Area
e080-e0ff I/O   I/O2 Select Area, includes system I/O below.

e0f1-e0ff SYSIO Onboard system I/O is always mapped to these addresses
                irrespective of the RAM or ROM pages.
e0f1-e0f1 Cfg   Read: 4-bit Config jumper; Write: RAM/ROM page latch
e0f2-e0f3 ACIA  Console RS-232
e0f4-e0f7 PIA   Stop on address comparator PIA. A0 and A1 are reversed.
e0f8-e0fb PIA   Keypad and LED display
e0fc-e0ff PIA   User PIA on Keypad-display board

e400-e7ff RAM   Onboad 1K stack RAM. Always mapped to these addresses
                irrespective of the RAM or ROM page.

e800-efff ROM   ROMG U61, normal location.
f000-ffff ROM   ROMH U62, normal location.


The onboard 4K user RAM can be placed at any 1K address boundary on any RAM
page and on multiple RAM pages. The default is 0x0000 to 0x0fff on all RAM
pages, but jumpers may be added at connector J2 to modify this selection. If a
jumper is present across pins 1-2, or 3-4, or 5-6, or 7-8, then address lines
A15 to A12 respectively must be high (rather than low) for the user RAM to be
selected. By default, with no jumpers installed, the RAM page address lines
RAP0 to RAP2 are 'do not care', but J2 pin pairs can be jumpered to select the
user RAM only when these are high or low - pins 9-10 select RAP0 being high,
pins 11-12 select RAP0 being low, pins 13-14 select RAP1 being high, etc. The
user RAM can also be disabled by jumpering any of pins 10, 12, 14, 16, 18, 20
low which can be done by jumpering any of these to any of pins 2, 4, 6, or 8.
The user RAM data bus is buffered and so long as it is decoded then the data
lines are driven so removing this RAM does not free the decoded address range,
and the onboard user RAM takes precedence over off board addressing. It would
cause a data bus conflict if the user RAM were decoded in the same address
range as the onboard stack RAM, ROM, or the system I/O. This user RAM can also
be write protected by removing the jumper between pins 11-12 at J1.

TODO implement the 4K user RAM mapping.

There are eight onboard ROM sockets, labeled ROM/A to ROM/H. ROM/A to ROM/D
have addressing for A0 to A12 so support 8K devices, however collectively
their A11 and/or A12 lines can be jumpered low or high to support smaller
devices. The ROM/E and ROM/F have addressing for A0 to A11 so support 4K
devices, but can be collectively jumpered for 1K, 2K or 4K operation. ROM/G
and ROM/H are used for the monitor ROMS and support 2K or 4K devices
individually, and the standard ROM sizes are 2K and 4K respectively. The
onboard ROM data bus is buffered and so long as it is decoded then the data
lines are driven so removing a ROM does not free the decoded address range,
and the onboard ROM takes precedence over off board addressing. It would cause
a data bus conflict if this ROM were decoded in the same address range as
the onboard stack RAM, the user RAM, or the system I/O. The data bus buffer
for the ROMs is not bi-directional, it would appear to not be possible to
place RAM or I/O in these sockets, and it appears that it would cause a bus
conflict if there were a valid write to these ROMs, and the W/R line is wired
to the ROM decoder to support that - not sure what was intended here?

The onboard ROM selection is handled by a 1K mapping ROM in U31. The outputs
D0 to D7 selecting ROM/A to ROM/H respectively when low. It was up to the
programming of this ROM to avoid address conflicts, to select only one ROM at
a time. The ROM address inputs A9 to A0 are respectively: A15, A14, A13, A12,
A11, A10, ROP2, ROP1, ROP0, R/W. This allows mapping ROMS on 1K address map
boundaries. The default mapping ROM was label "D4MAP 00", a MCM68A316, and did
not select any ROM 0x4000.

The mapping ROM pins 18 and 20 and wired to 0V, pin 21 is wired to +5V, pin
19 can be jumpered to 0V or 5V, defaulting to 0V. So could a single supply
EPROM could be substituted. Pin 19 is A10 on a 2716 EPROM the lower half of a
2K EPROM could be used too.

TODO implement the onboard ROMs and their mapping.

There was a resident editor assembler product available for this system, the
"MEK6809EAC Editor/Assembler V1.0 2/80". It was supplied on a tape and side
one had a version to load into RAM at 0100 to 2fff, and side two had a version
that was ROMable at a000 to cfff. It supported the MEK68R2 CRT display or a
terminal and object code could be placed in memory or saved on tape.

TODO This editor/assembler ROM might be a nice addition if it can be found.

The CPU clock is either generated from the onboard 3.579545 MHz XTAL or
externally as set via J3.

TODO could support 1MHz and 2MHz CPU operation.

The board includes a hardware 'stop' address comparator which compares the
A0-A15 address lines to the output of the PIA at e0f4-e0f7, 'stop_pia' here,
and asserts that PIA's CA1 input when they match. The monitor software uses
this to trigger an interrupt to implement code tracing. The PIA port B outputs
B0 to B7 are compared to the address lines A0 to A7 respectively, and the port
A outputs A0 to A7 are compared to the address lines A8 to A15
respectively. Notably the comparitor does not compare the RAM or ROM bank
lines, so this might be frustrating when used with code making use of
banking. It appears that the address comparator does not distinguish between
code versus data, so it would appear to be able to trigger on a data access
too, and although the monitor documenation does not mention such a use this
use is possible with the monitor. The trigger is also available at TP1 and
this could be informative in some hardware development, with the address set
and the interrupt disabled. The CA2 input can also be toggled low to generate
an interrupt as a manually 'abort'.


J1
1-2  RS-232 console
3-4  R2 board present
7-8  Keypad present
5-6  300/1200 cassette baud rate
11-12 Write protect 4K user ram.


ASCII terminal commands:
<hex addr><space> - Memory change
  <hex byte>      - enter data, write to memory.
  <space>         - increase address, same line
  <linefeed>      - increase address, new line
  '-' or '^'      - decrease address, new line
  ';'<hex addr>   - calculate branch offset to addr, 8 bit or 16 bit offset.
  <carriage return> - return from memory change
'R'               - Register editor
  'P'             - Program counter
  'A'             - A-Accumulator
  'B'             - B-Accumulator
  'X'             - X-Index
  'Y'             - Y-Index
  'C'             - Condition Codes
  'H'             - RAM/ROM pages select latch, high nibble is ROM page, low RAM page.
  'D'             - Direct Page
  'U'             - U-Stack
  'S'             - S-Stack
  '1','2','3','4' - Definable 16-bit registers (memory locations)
  <cr>            - exit register editor
  <linefeed>      - display update
  <space>         - next register
  'T'             - trace one instruction
  'L'             - trace one line (subroutine), using single stepping
  'R'             - trace one line (subroutine), using hardware (real time) approach.
'Q'               - Enter breakpoint editor
  'I'<addr>       - insert breakpoint at <addr>, to stop after one times.
  'I'<addr>;<n>   - insert breakpoint at <addr>, to stop after <n> times.
  'R'<addr>       - remove breakpoing at <addr>
  'S'<addr>       - set stop address, to stop after one times.
  'S'<addr>;<n>   - set stop address, to stop after <n> times.
  'K'             - clear all breakpoints and deactivate the stop address.
  <cr>            - exit breakpoint editor.
'G'               - Continue at the pseudo program counter.
<addr>'G'         - Go to user program at <addr>
'M'               - Memory dump
'P'               - Punch. Store information from memory to cassette.
<addr>'P'         - Punch with offset, as if starting from <addr>.
'L'               - Load. Read information to memory from cassette.
<addr>'L'         - Load with offset.
'V'               - Verify cassette data against memory.
<addr>'V'         - Verify with offset.
<hex>=            - Convert hex to decimal.
#<decimal>$       - Convert decimal to hex.
'U'               - Switch R2 to User screen page.
'S'               - Switch R2 to System screen page.
'X'               - Enter special functions.
  'M'             - Move memory.
  'F'             - Fill memory.
  'S'             - Search memory. Escape to pause.
  'A'             - ASCII Entry.
    '@'           - End of message.
    <escape>      - Exit ASCII entry.
    <delete>      - Delete last character.
    <backspace>   - When using R2D, back up.


MEK68KPD commands:

RS (Reset)  Reset, wired to the CPU reset line
EX (Escape) Typically aborts user program. Switch from CRT to KPD.
M (Memory display/change)
  Digits 5 and 6 show the entered data, and are blank unless it differs.
  Digits 7 and 8 show the actual data at the address.
  G  - increase the address.
  M  - decreases the address.
  FS - Offset calculation. Enter address, then press 'GO'.
    Last digit is S for a short offset and L for a long 16 bit offset.
    FS - stores the offset and returns to memory display and increased the address.
    FC - return to memory display at the same address.
  EX - exits memory display.
RD (Register display/alter)
  The 'HP' register is the hardware page register, upper nibble the ROM page,
  lower the RAM page, 8 pages each.
  The 'SA' register is the hardware stop address.
    FS - at register 'SA', to change the 'number of times'.
    FC - at register 'SA', to change the stop address.
  G   - advance to next register.
  M   - previous register.
  T/B - trace a single instruction
  P/L - trace user line, an entire subroutine if next, slow software version.
  RD  - trace user line, using the stop address hardware.
  EX  - exits register display.
GO to user program.
  If no address if entered then it uses the pseudo PC, it continues.
  Enter the address and press 'Go' to use that entered address.
  It firstly checks that there is RAM at the stack pointer.
FS GO - enter the address, then FS, then GO and it runs as a sub of the LED
  display code allowing use of the LED display. The sub should be quick
  relative to the 1ms display update period.
FS T/B - Breakpoint editor
  GO - advance to next breakpoing, up to 8, then loops.
  FC - deactivate breakpoint
  FS - edit the 'number of times before stopping' for the current entry.
  FS - enter an address then press FS to enter that as a breakpoint address
       and then edit the 'number of times before stopping'.
  EX - exits breakpoing editor.
P/L (Punch tape)
  Enter an address before P/L to set the apparent beginning address.
  At the 'b' prompt enter the beginning address of the data, then 'GO'.
  At the 'E' prompt enter the last address of the data.
  Start the tape and press GO. There is a 30 second leader of $ff.
FS P/L (Load from tape)
  Enter an address before FS for an offset load or verify??
FS 0 to F
  One of 16 user defined functions. Press FS then one number key 0 to F.
  Numeric data may be entered before pressing FS.
  A pointer to a table of 16 function address should be set at 0xe72e.
  There is a reservation at 0xe730-0xe74f for this table.
  The function address is stored in 0xe700 and called by the LED display
  update function PUT every 1msec.

******************************************************************************/

#include "emu.h"

#include "bus/rs232/rs232.h"
#include "cpu/m6809/m6809.h"
#include "imagedev/cassette.h"
#include "imagedev/snapquik.h"
#include "machine/6821pia.h"
#include "machine/6850acia.h"
#include "machine/bankdev.h"
#include "machine/clock.h"
#include "machine/input_merger.h"
#include "machine/mc14411.h"
#include "machine/timer.h"
#include "sound/wave.h"
#include "video/pwm.h"

// MEK68R2
#include "machine/terminal.h"
#include "video/mc6845.h"

#include "emupal.h"
#include "render.h"
#include "screen.h"
#include "speaker.h"

#include "mekd4.lh"


namespace {

class mekd4_state : public driver_device
{
public:
	mekd4_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_bankdev(*this, "bankdev")
		, m_stop_pia(*this, "stop_pia")
		, m_kpd_pia(*this, "kpd_pia")
		, m_display(*this, "display")
		, m_user_pia(*this, "user_pia")
		, m_brg(*this, "brg")
		, m_rs232_tx_baud(*this, "RS232_TX_BAUD")
		, m_rs232_rx_baud(*this, "RS232_RX_BAUD")
		, m_rs232_cts_route(*this, "RS232_CTS_ROUTE")
		, m_rs232_dcd_route(*this, "RS232_DCD_ROUTE")
		, m_acia(*this, "acia")
		, m_cass(*this, "cassette")
		, m_jumper1(*this, "JUMPER1")
		, m_keypad_columns(*this, "COL%u", 0)
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

	void mekd4(machine_config &config);
	void init_mekd4();

	void reset_key_w(int state);
	DECLARE_INPUT_CHANGED_MEMBER(keypad_changed);
	DECLARE_INPUT_CHANGED_MEMBER(rs232_cts_route_change);
	DECLARE_INPUT_CHANGED_MEMBER(rs232_dcd_route_change);

private:
	uint8_t main_r(offs_t offset);
	void main_w(offs_t offset, uint8_t data);
	uint8_t config_r();
	void page_w(uint8_t data);
	uint8_t stop_pia_r(offs_t offset);
	void stop_pia_w(offs_t offset, uint8_t data);
	void stop_pia_pa_w(uint8_t data);
	void stop_pia_pb_w(uint8_t data);
	uint16_t m_stop_address;

	void rs232_route_cts(int state);
	void rs232_route_dcd(int state);

	// Clocks
	void write_f1_clock(int state);
	void write_f3_clock(int state);
	void write_f7_clock(int state);
	void write_f8_clock(int state);
	void write_f9_clock(int state);
	void write_f13_clock(int state);

	int keypad_cb1_r();
	uint8_t keypad_key_r();
	void led_digit_w(uint8_t data);
	void led_segment_w(uint8_t data);

	int stop_pia_cb1_r();
	void stop_pia_cb2_w(int state);

	void mekd4_stop_mem(address_map &map) ATTR_COLD;
	void mekd4_mem(address_map &map) ATTR_COLD;

	address_space *m_banked_space;

	bool keypad_key_pressed();

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	uint8_t m_rom_page; // aka ROP0, ROP1, ROP2
	uint8_t m_ram_page; // aka RAP0, RAP1, RAP2
	uint8_t m_segment;
	uint8_t m_digit;
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	required_device<cpu_device> m_maincpu;
	required_device<address_map_bank_device> m_bankdev;
	required_device<pia6821_device> m_stop_pia;
	required_device<pia6821_device> m_kpd_pia;
	required_device<pwm_display_device> m_display;
	required_device<pia6821_device> m_user_pia;
	required_device<mc14411_device> m_brg;
	required_ioport m_rs232_tx_baud;
	required_ioport m_rs232_rx_baud;
	required_ioport m_rs232_cts_route;
	required_ioport m_rs232_dcd_route;
	required_device<acia6850_device> m_acia;
	required_device<cassette_image_device> m_cass;
	required_ioport m_jumper1;
	required_ioport_array<4> m_keypad_columns;
	int m_cts;
	int m_dcd;

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

void mekd4_state::mekd4_stop_mem(address_map &map)
{
	map(0x0000, 0xffff).rw(FUNC(mekd4_state::main_r), FUNC(mekd4_state::main_w));
}

void mekd4_state::mekd4_mem(address_map &map)
{
	map(0x0000, 0x7fff).ram();

	/* MEK68R2 Video RAM 9000-9fff */
	map(0x9000, 0x9fff).ram().share(m_video_ram);

	/* MEK68VG VDG Scroll register f040-f041 */

	/* MEK68R2 CRT register f042-f043 */
	map(0xe042, 0xe042).w(m_mc6845, FUNC(mc6845_device::address_w));
	map(0xe043, 0xe043).rw(m_mc6845, FUNC(mc6845_device::register_r), FUNC(mc6845_device::register_w));

	/* MEK68R2 PIA (Keyboard) */
	map(0xe044, 0xe047).rw(m_r2_pia, FUNC(pia6821_device::read), FUNC(pia6821_device::write));

	map(0xe0f1, 0xe0f1).rw(FUNC(mekd4_state::config_r), FUNC(mekd4_state::page_w));
	map(0xe0f2, 0xe0f3).rw(m_acia, FUNC(acia6850_device::read), FUNC(acia6850_device::write));
	map(0xe0f4, 0xe0f7).rw(FUNC(mekd4_state::stop_pia_r), FUNC(mekd4_state::stop_pia_w));
	map(0xe0f8, 0xe0fb).rw(m_kpd_pia, FUNC(pia6821_device::read), FUNC(pia6821_device::write));
	map(0xe0fc, 0xe0ff).rw(m_user_pia, FUNC(pia6821_device::read), FUNC(pia6821_device::write));

	map(0xe400, 0xe7ff).ram();

	map(0xe800, 0xefff).rom();
	map(0xf000, 0xffff).rom();
}

/***********************************************************

    Keys

************************************************************/

static INPUT_PORTS_START(mekd4)

	PORT_START("JUMPER1")
	PORT_DIPNAME(0x01, 0x00, "RS-232 console (D4B)")
	PORT_DIPSETTING(0x00, DEF_STR(On))
	PORT_DIPSETTING(0x01, DEF_STR(Off))
	PORT_DIPNAME(0x02, 0x00, "MEK68R2 present (D4C)")
	PORT_DIPSETTING(0x00, DEF_STR(On))
	PORT_DIPSETTING(0x02, DEF_STR(Off))
	PORT_DIPNAME(0x04, 0x00, "Cassette baud rate")
	PORT_DIPSETTING(0x00, "1200")
	PORT_DIPSETTING(0x04, "300")
	PORT_DIPNAME(0x08, 0x00, "Keypad and display present (D4A)")
	PORT_DIPSETTING(0x00, DEF_STR(On))
	PORT_DIPSETTING(0x08, DEF_STR(Off))

	PORT_START("RS232_TX_BAUD")
	PORT_CONFNAME(0x3f, 1, "RS232 TX Baud Rate")
	PORT_CONFSETTING(0x20, "110")
	PORT_CONFSETTING(0x10, "300")
	PORT_CONFSETTING(0x08, "600")
	PORT_CONFSETTING(0x04, "1200")
	PORT_CONFSETTING(0x02, "4800")
	PORT_CONFSETTING(0x01, "9600")

	PORT_START("RS232_RX_BAUD")
	PORT_CONFNAME(0x3f, 1, "RS232 RX Baud Rate")
	PORT_CONFSETTING(0x20, "110")
	PORT_CONFSETTING(0x10, "300")
	PORT_CONFSETTING(0x08, "600")
	PORT_CONFSETTING(0x04, "1200")
	PORT_CONFSETTING(0x02, "4800")
	PORT_CONFSETTING(0x01, "9600")

	// RS232 CTS and DCD routing at the RS232 Conn. These need to be
	// jumpered to logical low if not driven by the RS232 device. There is
	// +12 and -12V available at this connector for this purpose.
	PORT_START("RS232_CTS_ROUTE")
	PORT_CONFNAME(0x1, 0, "RS232 CTS") PORT_CHANGED_MEMBER(DEVICE_SELF, mekd4_state, rs232_cts_route_change, 0)
	PORT_CONFSETTING(0, "Jumper low")
	PORT_CONFSETTING(1, "Pass through")
	PORT_START("RS232_DCD_ROUTE")
	PORT_CONFNAME(0x1, 0, "RS232 DCD") PORT_CHANGED_MEMBER(DEVICE_SELF, mekd4_state, rs232_dcd_route_change, 0)
	PORT_CONFSETTING(0, "Jumper low")
	PORT_CONFSETTING(1, "Pass through")

	// RESET is not wired to the key matrix.
	PORT_START("RESET")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("RS") PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, mekd4_state, reset_key_w)

	// PORT_CODEs are not assigned to the keypad to allow it on screen at
	// the same time as the terminal or CRT console which also receive
	// keyboard inputs. When a keyboard is available the keypad is of
	// limited use, but still useful to interrupt code or reset the
	// machine. If MAME someday allows the keyboard input focus to be
	// switched then this might be redesigned.
	PORT_START("COL0")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, mekd4_state, keypad_changed, 0) PORT_NAME("M")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, mekd4_state, keypad_changed, 0) PORT_NAME("FS")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, mekd4_state, keypad_changed, 0) PORT_NAME("7")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, mekd4_state, keypad_changed, 0) PORT_NAME("4")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, mekd4_state, keypad_changed, 0) PORT_NAME("1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, mekd4_state, keypad_changed, 0) PORT_NAME("0")

	PORT_START("COL1")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, mekd4_state, keypad_changed, 0) PORT_NAME("EX")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, mekd4_state, keypad_changed, 0) PORT_NAME("FC")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, mekd4_state, keypad_changed, 0) PORT_NAME("8")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, mekd4_state, keypad_changed, 0) PORT_NAME("5")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, mekd4_state, keypad_changed, 0) PORT_NAME("2")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, mekd4_state, keypad_changed, 0) PORT_NAME("F")

	PORT_START("COL2")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, mekd4_state, keypad_changed, 0) PORT_NAME("RD")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, mekd4_state, keypad_changed, 0) PORT_NAME("P/L")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, mekd4_state, keypad_changed, 0) PORT_NAME("9")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, mekd4_state, keypad_changed, 0) PORT_NAME("6")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, mekd4_state, keypad_changed, 0) PORT_NAME("3")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, mekd4_state, keypad_changed, 0) PORT_NAME("E")

	PORT_START("COL3")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, mekd4_state, keypad_changed, 0) PORT_NAME("GO")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, mekd4_state, keypad_changed, 0) PORT_NAME("T/B")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, mekd4_state, keypad_changed, 0) PORT_NAME("A")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, mekd4_state, keypad_changed, 0) PORT_NAME("B")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, mekd4_state, keypad_changed, 0) PORT_NAME("C")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, mekd4_state, keypad_changed, 0) PORT_NAME("D")

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

 Stop comparitor.

************************************************************/

uint8_t mekd4_state::main_r(offs_t offset)
{
	if (offset == m_stop_address && !machine().side_effects_disabled())
	{
		m_stop_pia->ca1_w(CLEAR_LINE);
		m_stop_pia->ca1_w(ASSERT_LINE);
		m_stop_pia->ca1_w(CLEAR_LINE);
	}
	return m_banked_space->read_byte(offset);
}

void mekd4_state::main_w(offs_t offset, uint8_t data)
{
	if (offset == m_stop_address && !machine().side_effects_disabled())
	{
		m_stop_pia->ca1_w(CLEAR_LINE);
		m_stop_pia->ca1_w(ASSERT_LINE);
		m_stop_pia->ca1_w(CLEAR_LINE);
	}
	m_banked_space->write_byte(offset, data);
}

uint8_t mekd4_state::config_r()
{
	return 0xf0 | m_jumper1->read();
}

// The design reversed the A0 and A1 lines so that
// a 16 bit write could write both data addresses.
uint8_t mekd4_state::stop_pia_r(offs_t offset)
{
	// Reverse the A0 and A1 address lines;
	int8_t reversed = BIT(offset, 0) << 1 | BIT(offset, 1);
	return m_stop_pia->read(reversed);
}

void mekd4_state::stop_pia_w(offs_t offset, uint8_t data)
{
	// Reverse the A0 and A1 address lines;
	int8_t reversed = BIT(offset, 0) << 1 | BIT(offset, 1);
	m_stop_pia->write(reversed, data);
}

void mekd4_state::stop_pia_pa_w(uint8_t data)
{
	m_stop_address = (m_stop_address & 0x00ff) | (data << 8);
}

void mekd4_state::stop_pia_pb_w(uint8_t data)
{
	m_stop_address = (m_stop_address & 0xff00) | data;
}

/***********************************************************

 RAM and ROM paging

************************************************************/

void mekd4_state::page_w(uint8_t data)
{
	m_rom_page = data & 0x07;
	m_ram_page = (data >> 4) & 0x07;
}

/***********************************************************

    Keypad

************************************************************/

void mekd4_state::reset_key_w(int state)
{
	m_maincpu->set_input_line(INPUT_LINE_RESET, state ? CLEAR_LINE : ASSERT_LINE);

	// TODO reset other devices.
}

bool mekd4_state::keypad_key_pressed()
{
	return (m_keypad_columns[0]->read() & m_digit) ||
		(m_keypad_columns[1]->read() & m_digit) ||
		(m_keypad_columns[2]->read() & m_digit) ||
		(m_keypad_columns[3]->read() & m_digit);
}

INPUT_CHANGED_MEMBER(mekd4_state::keypad_changed)
{
	m_kpd_pia->cb1_w(mekd4_state::keypad_key_pressed());
}

int mekd4_state::keypad_cb1_r()
{
	return mekd4_state::keypad_key_pressed();
}

uint8_t mekd4_state::keypad_key_r()
{
	uint8_t mux = (m_digit & 0xc0) >> 6;
	uint8_t i = (m_keypad_columns[mux]->read() & m_digit) ? 0 : 0x80;

	return i | m_segment;
}

/***********************************************************

    Seven segment LED display

************************************************************/

// PA
void mekd4_state::led_segment_w(uint8_t data)
{
	m_segment = data & 0x7f;
	m_display->matrix(m_digit, ~m_segment);
}

// PB
void mekd4_state::led_digit_w(uint8_t data)
{
	m_digit = data;
	m_display->matrix(m_digit, ~m_segment);
	// Update the keypad pressed output which depends on m_digit.
	m_kpd_pia->cb1_w(mekd4_state::keypad_key_pressed());
}


/***********************************************************

  Cassette

************************************************************/

int mekd4_state::stop_pia_cb1_r()
{
	uint8_t state = m_cass->input() > +0.0;
	return state;
}

void mekd4_state::stop_pia_cb2_w(int state)
{
	m_cass->output(state ? -1.0 : +1.0);
}

/***********************************************************

  ACIA

************************************************************/

void mekd4_state::rs232_route_cts(int state)
{
	if (m_rs232_cts_route->read())
		m_acia->write_cts(state);

	// Cache the state, in case the ioport setting changes.
	m_cts = state;
}

void mekd4_state::rs232_route_dcd(int state)
{
	if (m_rs232_dcd_route->read())
		m_acia->write_dcd(state);

	// Cache the state, in case the ioport setting changes.
	m_dcd = state;
}

INPUT_CHANGED_MEMBER(mekd4_state::rs232_cts_route_change)
{
	if (newval)
		m_acia->write_cts(m_cts);
	else
		m_acia->write_cts(0);
}

INPUT_CHANGED_MEMBER(mekd4_state::rs232_dcd_route_change)
{
	if (newval)
		m_acia->write_dcd(m_dcd);
	else
		m_acia->write_dcd(0);
}

void mekd4_state::write_f1_clock(int state)
{
	if (BIT(m_rs232_tx_baud->read(), 0))
		m_acia->write_txc(state);
	if (BIT(m_rs232_rx_baud->read(), 0))
		m_acia->write_rxc(state);
}

void mekd4_state::write_f3_clock(int state)
{
	if (BIT(m_rs232_tx_baud->read(), 1))
		m_acia->write_txc(state);
	if (BIT(m_rs232_rx_baud->read(), 1))
		m_acia->write_rxc(state);
}

void mekd4_state::write_f7_clock(int state)
{
	if (BIT(m_rs232_tx_baud->read(), 2))
		m_acia->write_txc(state);
	if (BIT(m_rs232_rx_baud->read(), 2))
		m_acia->write_rxc(state);
}

void mekd4_state::write_f8_clock(int state)
{
	if (BIT(m_rs232_tx_baud->read(), 3))
		m_acia->write_txc(state);
	if (BIT(m_rs232_rx_baud->read(), 3))
		m_acia->write_rxc(state);
}

void mekd4_state::write_f9_clock(int state)
{
	if (BIT(m_rs232_tx_baud->read(), 4))
		m_acia->write_txc(state);
	if (BIT(m_rs232_rx_baud->read(), 4))
		m_acia->write_rxc(state);
}

void mekd4_state::write_f13_clock(int state)
{
	if (BIT(m_rs232_tx_baud->read(), 5))
		m_acia->write_txc(state);
	if (BIT(m_rs232_rx_baud->read(), 5))
		m_acia->write_rxc(state);
}

/***********************************************************

  MEK68R2

  This might in future be moved to a slot device, on a bus.

************************************************************/

// Delivery of keyboard inputs to the MEK68R2 keyboard is disabled when on
// views with the RS232 terminal, assuming that this keyboard is not present.
// Also disable delivery when the 'MEK68R2 present' jumper indicates it is
// disabled, assuming that this keyboard is not present.
void mekd4_state::kbd_put(uint8_t data)
{
	uint8_t view = machine().render().first_target()->view();
	if (view == 0)
		return;

	if (BIT(m_jumper1->read(), 1))
		return;

	m_term_data = data;
	// Triggers on the falling edge.
	m_r2_pia->ca1_w(ASSERT_LINE);
	m_r2_pia->ca1_w(CLEAR_LINE);
	m_r2_pia->ca1_w(ASSERT_LINE);
}

// PA0 to PA6 - Keyboard data.
// PA7 - Display nationality, 0 USA, 1 Europe.
uint8_t mekd4_state::r2_pia_pa_r()
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
uint8_t mekd4_state::r2_pia_pb_r()
{
	int8_t display_format = m_r2_display_format->read();
	int8_t mode = m_r2_mode->read();
	return (display_format << 6) | mode;
}

MC6845_UPDATE_ROW(mekd4_state::update_row)
{
	const pen_t *pen = m_palette->pens();

	int x = 0;

	for (int column = 0; column < x_count; column++)
	{
		uint8_t code = m_video_ram[(ma + column) & 0xfff];
		int dcursor = (column == cursor_x);

		if (BIT(code, 7)) {
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
		} else {
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

void mekd4_state::init_mekd4()
{
}

void mekd4_state::machine_start()
{
	m_banked_space = &subdevice<address_map_bank_device>("bankdev")->space(AS_PROGRAM);

	save_item(NAME(m_stop_address));
	save_item(NAME(m_rom_page));
	save_item(NAME(m_ram_page));
	save_item(NAME(m_segment));
	save_item(NAME(m_digit));
	save_item(NAME(m_cts));
	save_item(NAME(m_dcd));
	save_item(NAME(m_term_data));
}

void mekd4_state::machine_reset()
{
	m_rom_page = 0;
	m_ram_page = 0;
	m_stop_address = 0x0000;

	// Avoid triggering an early interrupt when CB1 lowered. The mc6821
	// driver resets CB1 high and to trigger on a high to low
	// transition. The mekd4 programs CB1 to trigger on a low to high
	// transition and configuring this earlier here is adequate.
	m_kpd_pia->write(1, 2);
	m_kpd_pia->cb2_w(ASSERT_LINE);  // Pulled high.

	m_brg->rsa_w(CLEAR_LINE);
	m_brg->rsb_w(ASSERT_LINE);

	// Write low here if jumpered low.
	if (!m_rs232_cts_route->read())
		m_acia->write_cts(0);
	if (!m_rs232_dcd_route->read())
		m_acia->write_dcd(0);

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

void mekd4_state::mekd4(machine_config &config)
{
	MC6809(config, m_maincpu, 3.579545_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &mekd4_state::mekd4_stop_mem);

	ADDRESS_MAP_BANK(config, m_bankdev, 0);
	m_bankdev->set_endianness(ENDIANNESS_BIG);
	m_bankdev->set_data_width(8);
	m_bankdev->set_addr_width(20);
	m_bankdev->set_addrmap(AS_PROGRAM, &mekd4_state::mekd4_mem);

	INPUT_MERGER_ANY_HIGH(config, "mainirq").output_handler().set_inputline(m_maincpu, M6809_IRQ_LINE);
	INPUT_MERGER_ANY_HIGH(config, "mainnmi").output_handler().set_inputline(m_maincpu, INPUT_LINE_NMI);

	/* LED display */
	PWM_DISPLAY(config, m_display).set_size(8, 7);
	m_display->set_segmask(0xff, 0x7f);

	config.set_default_layout(layout_mekd4);

	SPEAKER(config, "mono").front_center();

	CASSETTE(config, m_cass);
	m_cass->set_default_state(CASSETTE_STOPPED | CASSETTE_MOTOR_ENABLED | CASSETTE_SPEAKER_ENABLED);
	m_cass->add_route(ALL_OUTPUTS, "mono", 0.05);

	// IRQ is not connected. RTS, CTS, and DCD are available.
	ACIA6850(config, m_acia, 0);
	m_acia->txd_handler().set("rs232", FUNC(rs232_port_device::write_txd));
	m_acia->rts_handler().set("rs232", FUNC(rs232_port_device::write_rts));

	MC14411(config, m_brg, XTAL(1'843'200));
	m_brg->out_f<1>().set(FUNC(mekd4_state::write_f1_clock));
	m_brg->out_f<3>().set(FUNC(mekd4_state::write_f3_clock));
	m_brg->out_f<7>().set(FUNC(mekd4_state::write_f7_clock));
	m_brg->out_f<8>().set(FUNC(mekd4_state::write_f8_clock));
	m_brg->out_f<9>().set(FUNC(mekd4_state::write_f9_clock));
	m_brg->out_f<13>().set(FUNC(mekd4_state::write_f13_clock));

	rs232_port_device &rs232(RS232_PORT(config, "rs232", default_rs232_devices, "terminal"));
	rs232.rxd_handler().set(m_acia, FUNC(acia6850_device::write_rxd));
	rs232.cts_handler().set(FUNC(mekd4_state::rs232_route_cts));
	rs232.dcd_handler().set(FUNC(mekd4_state::rs232_route_dcd));
	rs232.set_option_device_input_defaults("terminal", DEVICE_INPUT_DEFAULTS_NAME(terminal));

	// Stop PIA. IRQB is NC.
	PIA6821(config, m_stop_pia);
	m_stop_pia->writepa_handler().set(FUNC(mekd4_state::stop_pia_pa_w));
	m_stop_pia->writepb_handler().set(FUNC(mekd4_state::stop_pia_pb_w));
	m_stop_pia->ca2_w(1); // Connected to 'abort' TP2. Can be toggled low to and abort user code.
	m_stop_pia->readcb1_handler().set(FUNC(mekd4_state::stop_pia_cb1_r));
	m_stop_pia->cb2_handler().set(FUNC(mekd4_state::stop_pia_cb2_w));
	m_stop_pia->irqa_handler().set("mainnmi", FUNC(input_merger_device::in_w<0>));

	// Keypad and display PIA. CA1, CA2, IRQA are NC. CB2 is pulled high.
	PIA6821(config, m_kpd_pia);
	m_kpd_pia->readpa_handler().set(FUNC(mekd4_state::keypad_key_r));
	m_kpd_pia->readcb1_handler().set(FUNC(mekd4_state::keypad_cb1_r));
	m_kpd_pia->writepa_handler().set(FUNC(mekd4_state::led_segment_w));
	m_kpd_pia->writepb_handler().set(FUNC(mekd4_state::led_digit_w));
	m_kpd_pia->irqb_handler().set("mainnmi", FUNC(input_merger_device::in_w<1>));

	// Keypad and display board User PIA.
	PIA6821(config, m_user_pia);
	m_user_pia->irqa_handler().set("mainirq", FUNC(input_merger_device::in_w<0>));
	m_user_pia->irqb_handler().set("mainirq", FUNC(input_merger_device::in_w<1>));

	// MEK68R2

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);

	m_screen->set_refresh_hz(50);
	m_screen->set_size(80 * 8 + 80 * 10, 20 * 12 + 100);
	m_screen->set_visarea(0, 80 * 8 + 80 * 10 - 1, 0, 20 * 12 + 100 - 1);
	m_screen->set_screen_update("mc6845", FUNC(mc6845_device::screen_update));

	PALETTE(config, m_palette, palette_device::MONOCHROME_HIGHLIGHT);

	MC6845(config, m_mc6845, XTAL(14'318'181)/8);
	m_mc6845->set_screen(m_screen);
	m_mc6845->set_show_border_area(false);
	m_mc6845->set_char_width(8);
	m_mc6845->set_update_row_callback(FUNC(mekd4_state::update_row));
	m_mc6845->out_hsync_callback().set(m_r2_pia, FUNC(pia6821_device::cb2_w));
	m_mc6845->out_vsync_callback().set(m_r2_pia, FUNC(pia6821_device::cb1_w));

	// PA is the keyboard data and a mode flag.
	// CA1 is keyboard strobe.
	// CA2 light pen input.
	// PB0 is mode flags and light pen control.
	// CB1 is VSYNC, and CB2 is HSYNC.
	PIA6821(config, m_r2_pia);
	m_r2_pia->readpa_handler().set(FUNC(mekd4_state::r2_pia_pa_r));
	m_r2_pia->readpb_handler().set(FUNC(mekd4_state::r2_pia_pb_r));
	m_r2_pia->irqa_handler().set("mainirq", FUNC(input_merger_device::in_w<2>));
	m_r2_pia->irqb_handler().set("mainirq", FUNC(input_merger_device::in_w<3>));

	generic_keyboard_device &keyboard(GENERIC_KEYBOARD(config, "keyboard", 0));
	keyboard.set_keyboard_callback(FUNC(mekd4_state::kbd_put));
}

/***********************************************************

    ROMS

************************************************************/

ROM_START(mekd4)
	ROM_REGION(0x10000,"bankdev",0)
	ROM_LOAD("d4bugr2.rom", 0xe800, 0x0800, CRC(0b80a67d) SHA1(20d980767a7a667fe0f8e377bb2c29e297e6c635))
	ROM_LOAD("d4bugkpd.rom", 0xf000, 0x1000, CRC(1fdf414a) SHA1(3c8883a6ee0ae89398d9be5a5843db4c3b20f7fd))
	ROM_REGION(0x0400, "chargen",0)
	ROM_LOAD("mcm6674p.chr", 0x0000, 0x0400, CRC(1c22088a) SHA1(b5f0bd0cfdec0cd5c1cb764506bef3c17d6af0eb))
	ROM_REGION(0x0400, "rommap",0)
	ROM_LOAD("d4map00.rom", 0x0000, 0x0400, CRC(7e676444) SHA1(4f8a7443da509561be958786f9bd72eac3969a89))
ROM_END

} // anonymous namespace


/***************************************************************************

  Game driver(s)

***************************************************************************/

//    YEAR  NAME    PARENT  COMPAT  MACHINE   INPUT  CLASS        INIT        COMPANY     FULLNAME      FLAGS
COMP(1980, mekd4,  0,      0,      mekd4,    mekd4, mekd4_state, init_mekd4, "Motorola", "MEK6802D4" , MACHINE_NO_SOUND)
