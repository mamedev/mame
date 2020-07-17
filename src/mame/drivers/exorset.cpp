// license:BSD-3-Clause
// copyright-holders:68bit
/***************************************************************************

Motorola EXORset 30

The EXORset 30 runs XDOS which is a subset or variant of MDOS. The EXORBUG
includes the 'XDOS' command to boot XDOS.

References: EXORset 30 User's Guide. MSET30(D1) 1980 Motorola.

The RS232 interface default is 2400 baud 8N1.

EXORBUG 1.2 commands:

PRNT - Prompts for an address range and prints the memory in this range.

PNCH - Prompts for an address range and header name and writes the memory in
this range to the cassette interface.

DUMP - Prompts for an address range and writes the memory in this range to the
RS232 interface using the S1 format.

LOAD - Load memory content back from the cassette interface.

VERF - Compares memory with content read back from the cassette interface.

DWLD - Download S19 records from over the RS232 interface.

LINK - Download S19 records over a parallel link at 0xed00.

XDOS - Boots the XDOS floppy disk operating system.

TMAP - Toggles the address map, between map 1 and map 2. The EXORBUG prompt
changes from '.' for map 1 (Executive) to ':' map 2 (User)

XCOM - Switches to 'on-line' mode, aka terminal mode, to act as an RS232
terminal. ESC-O received from the RS232 line returns to the normal mode.

RA - Display and edit CPU Accumulator A register
RB - Display and edit CPU Accumulator B register
RC - Display and edit the CPU Condition Codes register
RD - Display and edit CPU Accumulator D register, a combination of A and B.
RL - Display and edit the CPU Program Counter register
RP - Display and edit the CPU Direct Page register
RS - Display and edit the CPU S stack register
RU - Display and edit the CPU U stack register
RX - Display and edit the CPU X index register
RY - Display and edit the CPU Y index register
RR - Display all the CPU registers

SM - stop memory address ???

MV - move memory contents from a source address range to a target address.

IS - Insert a string at a starting address and terminated by Ctrl-D (0x04).

SP - ?

DT - ?

EV - enable or disable the stop address line?

TC - trace counter?

xx$I, $I, ;I - Fills a address range with byte xx.

$M, ;M - set up a search address range and mask

xxxx$N, $N, ;N - trace xxxx instructions

xxxx$P, ;P - continues, and traces to the $T stop address. if there is no stop
address it prints 'NO BKPT'.

$T - prompts for a trace stop address

$U, ;U - Clears all hardware breakpoints

xxxx$U, ;U - Clears the hardware breakpoint at address xxxx

xxxx$V, ;V - Set a hardware breakpoint at address xxxx. Multiple hardware
breakpoints may be set in the same 4k page - the top 4 address bits must be
the same for all the hardware breakpoints due to hardware limitations.

$V, ;V - Display all the hardware breakpoints.

$W, ;W - search and prints matches in the range set by the $M command.

$Z - ??

$G - run from ??
xxxx$G - run from the given address

xxxx/ memory examine and change
  Memory examine and change.
    xx LF   - enter new value, and advance to the next address
    LF      - advance to the next address
    ^       - previous address
    xxxx;O  - calculate a branch offset from the current to the given address.


Terminal emulation escape codes:
ESC-B toggle inverse characters
ESC-C clears from the current position to the end of the screen
ESC-E clear screen
ESC-F toggle screen size, and clears screen, graphics off
ESC-G bell
ESC-H cursor left
ESC-K kills characters to the end of the line, including the current character
ESC-L home cursor, top left
ESC-N cursor right
ESC-S alphanumeric on
ESC-T alphanumeric off
ESC-U cursor up
ESC-V cursor down
ESC-Y graphics on
ESC-Z graphics off
ESC-O switch from online to stand-alone mode, but only when coming from the host?

****************************************************************************/

#include "emu.h"
#include "cpu/m6809/m6809.h"
#include "machine/6821pia.h"
#include "machine/6840ptm.h"
#include "machine/6850acia.h"
#include "machine/input_merger.h"
#include "machine/bankdev.h"
#include "machine/mc6843.h"
#include "machine/terminal.h"
#include "bus/rs232/rs232.h"
#include "sound/spkrdev.h"
#include "sound/beep.h"
#include "video/mc6845.h"
#include "formats/mdos_dsk.h"
#include "imagedev/printer.h"
#include "imagedev/cassette.h"
#include "emupal.h"
#include "screen.h"
#include "speaker.h"

class exorset_state : public driver_device
{
public:
	exorset_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_bankdev(*this, "bankdev")
		, m_mainirq(*this, "mainirq")
		, m_mainnmi(*this, "mainnmi")
		, m_maincpu_clock(*this, "MAINCPU_CLOCK")
		, m_break_key(*this, "BREAK_KEY")
		, m_ptm(*this, "ptm")
		, m_pia2(*this, "pia2")
		, m_fnkeys_columns(*this, "FCOL%u", 0)
		, m_pia3(*this, "pia3")
		, m_sw9(*this, "SW9")
		, m_sw18(*this, "SW18")
		, m_acia(*this, "acia")
		, m_mc6845(*this, "mc6845")
		, m_palette(*this, "palette")
		, m_screen(*this, "screen")
		, m_chargen(*this, "chargen")
		, m_vram(*this, "videoram")
		, m_gram(*this, "gram")
		, m_pia1(*this, "pia1")
		, m_printer(*this, "printer")
		, m_cass(*this, "cassette")
		, m_fdc(*this, "fdc")
		, m_fdc2(*this, "fdc2")
		, m_floppy_image(*this, "floppy%u", 0U)
		, m_erom_fd8_patch(*this, "EROM_FD8_PATCH")
		, m_erom2_fd8_patch(*this, "EROM2_FD8_PATCH")
		, m_beeper(*this, "beeper")
		, m_pia_link(*this, "pia_link")
	{
	}

	enum
	{
		TIMER_TRACE,
	};

	void exorset(machine_config &config);

	DECLARE_INPUT_CHANGED_MEMBER(maincpu_clock_change);
	DECLARE_INPUT_CHANGED_MEMBER(fnkeys_changed);
	DECLARE_WRITE_LINE_MEMBER(break_key_w);

private:
	virtual void machine_reset() override;
	virtual void machine_start() override;

	void dbg_map(address_map &map);
	void mem_map(address_map &map);

	address_space *m_banked_space;
	u8 main_r(offs_t offset);
	void main_w(offs_t offset, u8 data);

	required_device<mc6809_device> m_maincpu;
	required_device<address_map_bank_device> m_bankdev;
	required_device<input_merger_device> m_mainirq;
	required_device<input_merger_device> m_mainnmi;
	required_ioport m_maincpu_clock;
	required_ioport m_break_key;
	required_device<ptm6840_device> m_ptm;
	DECLARE_WRITE_LINE_MEMBER(ptm_o3_callback);

	required_device<pia6821_device> m_pia2;
	u8 pia2_pa_r();
	void pia2_pa_w(u8 data);
	u8 m_pia2_pa;
	DECLARE_READ_LINE_MEMBER(pia2_ca1_r);
	u8 pia2_pb_r();
	u8 m_term_data;
	DECLARE_WRITE_LINE_MEMBER(pia2_ca2_w);
	DECLARE_READ_LINE_MEMBER(pia2_cb1_r);
	u8 fnkeys_r();
	required_ioport_array<4> m_fnkeys_columns;
	bool fnkeys_pressed();

	required_device<pia6821_device> m_pia3;
	void pia3_pa_w(u8 data);
	DECLARE_READ_LINE_MEMBER(pia3_ca1_r);
	u8 pia3_pb_r();
	void pia3_pb_w(u8 data);
	DECLARE_READ_LINE_MEMBER(pia3_ca2_r);
	DECLARE_WRITE_LINE_MEMBER(pia3_cb2_w);
	u8 m_stop_addr_din;
	u8 m_stop_addr_rw;
	u8 m_stop_addr_wc;
	u16 m_stop_addr_high;
	u8 m_stop_ram[0x1000];
	u8 m_stop_addr_dout;
	u8 m_stop_nmi_en;
	u8 m_trace_en;
	u8 m_trace;
	u8 m_addr_map_ctrl;
	u8 m_timing_ctrl;
	u8 m_alphanumeric_disp_en;
	u8 m_graphic_disp_en;
	u8 m_cycle_count_en;

	required_ioport m_sw9;
	required_ioport m_sw18;
	required_device<acia6850_device> m_acia;

	optional_device<mc6845_device> m_mc6845;
	optional_device<palette_device> m_palette;
	optional_device<screen_device> m_screen;
	optional_region_ptr<u8> m_chargen;
	optional_shared_ptr<u8> m_vram;
	optional_shared_ptr<u8> m_gram;
	MC6845_UPDATE_ROW(update_row);

	required_device<pia6821_device> m_pia1;
	required_device<printer_image_device> m_printer;
	required_device<cassette_image_device> m_cass;
	void pia1_pa_w(u8 data);
	int pia1_ca1_r();
	void pia1_ca2_w(int state);
	u8 pia1_pb_r();
	void pia1_pb_w(u8 data);
	u8 pia1_cb1_r();
	void pia1_cb2_w(u8 state);
	u8 m_printer_data;
	u8 m_printer_data_strobe;

	required_device<mc6843_device> m_fdc;
	DECLARE_WRITE_LINE_MEMBER(fdc_index_0_w);
	DECLARE_WRITE_LINE_MEMBER(fdc_index_1_w);
	DECLARE_WRITE_LINE_MEMBER(fdc_index_2_w);
	DECLARE_WRITE_LINE_MEMBER(fdc_index_3_w);
	void index_callback(int index, int state);
	void drive_select_w(u8 data);
	u8 m_drive;

	required_device<mc6843_device> m_fdc2;
	DECLARE_WRITE_LINE_MEMBER(fdc2_index_0_w);
	DECLARE_WRITE_LINE_MEMBER(fdc2_index_1_w);
	DECLARE_WRITE_LINE_MEMBER(fdc2_index_2_w);
	DECLARE_WRITE_LINE_MEMBER(fdc2_index_3_w);
	void index_callback2(int index, int state);
	void drive_select2_w(u8 data);
	u8 m_drive2;

	required_device_array<legacy_floppy_image_device, 8> m_floppy_image;
	required_ioport m_erom_fd8_patch;
	required_ioport m_erom2_fd8_patch;

	required_device<beep_device> m_beeper;

	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

	// Keyboard
	void kbd_put(u8 code);

	required_device<pia6821_device> m_pia_link;
	u8 pia_link_pa_r();
	void pia_link_ca2_w(u8 data);
	u8 m_link_strobe;
	u8 m_link_data;
};

void exorset_state::dbg_map(address_map &map)
{
	map(0x0000, 0xffff).rw(FUNC(exorset_state::main_r), FUNC(exorset_state::main_w));
}

void exorset_state::mem_map(address_map &map)
{
	// Map 2 - User

	// User RAM

	// One set of standard address map PROMs appears to mirror the
	// motherboard RAM in this address range in both address maps, however
	// XDOS4 goes to lengths to trampoline data between addresses in this
	// range and supports separate memory in each map which is more useful
	// and so that mappnig is implement in this emulator.
	map(0x00000, 0x07fff).ram();

	// 0x08000 to 0x0dfff map to the motherboard ROM0 to ROM11 using one
	// sets of standard address map PROMs but these were configurable and
	// are mapped to external RAM for this emulator.
	map(0x08000, 0x0dfff).ram();

	// Video RAM.
	map(0x0e000, 0x0e7ff).mirror(0x10000).ram().share(m_vram);

	// XDOS supports two floppy disk controllers, one in each address map,
	// and it uses trampolines to move data between the controller in the
	// alternate map and EXORBUG includes support for copying data between
	// maps.

	// Disk driver code.
	map(0x0e800, 0x0ebff).rom().region("fdc2", 0);

	map(0x0ec00, 0x0ec07).rw(m_fdc2, FUNC(mc6843_device::read), FUNC(mc6843_device::write));
	map(0x0ec08, 0x0ec0F).w(FUNC(exorset_state::drive_select2_w));

	// The EXORBUG LINK command loads S19 data from this parallel port
	// This is not documented as being on the motherboard, so must have
	// been an expansion option?
	map(0x0ed00, 0x0ed03).mirror(0x10000).rw(m_pia_link, FUNC(pia6821_device::read), FUNC(pia6821_device::write));

	map(0x0ef00, 0x0ef00).mirror(0x10000).w(m_mc6845, FUNC(mc6845_device::address_w));
	map(0x0ef01, 0x0ef01).mirror(0x10000).rw(m_mc6845, FUNC(mc6845_device::register_r), FUNC(mc6845_device::register_w));

	map(0x0ef20, 0x0ef27).mirror(0x10000).rw(m_ptm, FUNC(ptm6840_device::read), FUNC(ptm6840_device::write));

	map(0x0ef40, 0x0ef41).mirror(0x10000).rw(m_acia, FUNC(acia6850_device::read), FUNC(acia6850_device::write));

	// Printer
	map(0x0ef60, 0x0ef63).mirror(0x10000).rw(m_pia1, FUNC(pia6821_device::read), FUNC(pia6821_device::write));

	// Keyboards
	map(0x0ef80, 0x0ef83).mirror(0x10000).rw(m_pia2, FUNC(pia6821_device::read), FUNC(pia6821_device::write));

	// Debug and system control
	map(0x0efa0, 0x0efa3).mirror(0x10000).rw(m_pia3, FUNC(pia6821_device::read), FUNC(pia6821_device::write));

	// EXORBUG
	map(0x0f000, 0x0ffff).mirror(0x10000).rom().region("exorbug", 0);

	// Map 1 - Executive

	// RAM
	map(0x10000, 0x1dfff).ram().share(m_gram);

	// Disk driver code.
	map(0x1e800, 0x1ebff).rom().region("fdc", 0);

	// Disk driver unit
	map(0x1ec00, 0x1ec07).rw(m_fdc, FUNC(mc6843_device::read), FUNC(mc6843_device::write));
	map(0x1ec08, 0x1ec0F).w(FUNC(exorset_state::drive_select_w));
}

/* Input ports */
static INPUT_PORTS_START( exorset )

	PORT_START("SW9")
	PORT_CONFNAME(0xffff, 0x4000, "Graphics RAM base address")
	PORT_CONFSETTING(0x0000, "0x0000")
	PORT_CONFSETTING(0x4000, "0x4000")
	PORT_CONFSETTING(0x8000, "0x8000")

	PORT_START("SW18")
	PORT_CONFNAME(0x80, 0x80, "Display format")
	PORT_CONFSETTING(0x80, "80 x 22")
	PORT_CONFSETTING(0x00, "64 x 16")
	PORT_CONFNAME(0x40, 0x40, "Display refresh frequency")
	PORT_CONFSETTING(0x40, "50 Hz (USA)")
	PORT_CONFSETTING(0x00, "60 Hz (EU)")
	PORT_CONFNAME(0x20, 0x20, "Map")
	PORT_CONFSETTING(0x20, "Map 1")
	PORT_CONFSETTING(0x00, "Map 2")

	PORT_START("MAINCPU_CLOCK")
	PORT_CONFNAME(0xffffff, 4000000, "CPU clock") PORT_CHANGED_MEMBER(DEVICE_SELF, exorset_state, maincpu_clock_change, 0)
	PORT_CONFSETTING(4000000, "1.0 MHz")
	PORT_CONFSETTING(6000000, "1.5 MHz")
	PORT_CONFSETTING(8000000, "2.0 MHz")

	PORT_START("EROM_FD8_PATCH")
	PORT_CONFNAME(0x1, 0, "Patch EROM for 8 inch floppy disk drives")
	PORT_CONFSETTING(0, "No, 5.25 inch drives with 16 sectors")
	PORT_CONFSETTING(1, "Yes, 8 inch drives with 26 sectors")

	PORT_START("EROM2_FD8_PATCH")
	PORT_CONFNAME(0x1, 0, "Patch EROM2 for 8 inch floppy disk drives")
	PORT_CONFSETTING(0, "No, 5.25 inch drives with 16 sectors")
	PORT_CONFSETTING(1, "Yes, 8 inch drives with 26 sectors")

	PORT_START("BREAK_KEY")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Break") PORT_CODE(KEYCODE_PAUSE) PORT_CHAR(UCHAR_MAMEKEY(CANCEL)) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, exorset_state, break_key_w)

	// Special function keys

	PORT_START("FCOL0")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, exorset_state, fnkeys_changed, 0) PORT_CODE(KEYCODE_F1) PORT_CHAR(UCHAR_MAMEKEY(F1))
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, exorset_state, fnkeys_changed, 0) PORT_CODE(KEYCODE_F5) PORT_CHAR(UCHAR_MAMEKEY(F5))
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, exorset_state, fnkeys_changed, 0) PORT_CODE(KEYCODE_F9) PORT_CHAR(UCHAR_MAMEKEY(F9))
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, exorset_state, fnkeys_changed, 0) PORT_CODE(KEYCODE_F13) PORT_CHAR(UCHAR_MAMEKEY(F13))

	PORT_START("FCOL1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, exorset_state, fnkeys_changed, 0) PORT_CODE(KEYCODE_F2) PORT_CHAR(UCHAR_MAMEKEY(F2))
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, exorset_state, fnkeys_changed, 0) PORT_CODE(KEYCODE_F6) PORT_CHAR(UCHAR_MAMEKEY(F6))
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, exorset_state, fnkeys_changed, 0) PORT_CODE(KEYCODE_F10) PORT_CHAR(UCHAR_MAMEKEY(F10))
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, exorset_state, fnkeys_changed, 0) PORT_CODE(KEYCODE_F14) PORT_CHAR(UCHAR_MAMEKEY(F14))

	PORT_START("FCOL2")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, exorset_state, fnkeys_changed, 0) PORT_CODE(KEYCODE_F3) PORT_CHAR(UCHAR_MAMEKEY(F3))
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, exorset_state, fnkeys_changed, 0) PORT_CODE(KEYCODE_F7) PORT_CHAR(UCHAR_MAMEKEY(F7))
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, exorset_state, fnkeys_changed, 0) PORT_CODE(KEYCODE_F11) PORT_CHAR(UCHAR_MAMEKEY(F11))
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, exorset_state, fnkeys_changed, 0) PORT_CODE(KEYCODE_F15) PORT_CHAR(UCHAR_MAMEKEY(F15))

	PORT_START("FCOL3")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, exorset_state, fnkeys_changed, 0) PORT_CODE(KEYCODE_F4) PORT_CHAR(UCHAR_MAMEKEY(F4))
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, exorset_state, fnkeys_changed, 0) PORT_CODE(KEYCODE_F8) PORT_CHAR(UCHAR_MAMEKEY(F8))
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, exorset_state, fnkeys_changed, 0) PORT_CODE(KEYCODE_F12) PORT_CHAR(UCHAR_MAMEKEY(F12))
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, exorset_state, fnkeys_changed, 0) PORT_CODE(KEYCODE_F16) PORT_CHAR(UCHAR_MAMEKEY(F16))

INPUT_PORTS_END

INPUT_CHANGED_MEMBER(exorset_state::maincpu_clock_change)
{
	m_maincpu->set_clock(newval);
	m_ptm->set_ext_clock(0, newval / 4);
}

WRITE_LINE_MEMBER(exorset_state::ptm_o3_callback)
{
	m_acia->write_txc(state);
	m_acia->write_rxc(state);
}

u8 exorset_state::main_r(offs_t offset)
{
	if ((offset & 0x0f000) == m_stop_addr_high
		&& !machine().side_effects_disabled())
	{
		if (m_stop_addr_rw == 1)
		{
			if (m_stop_addr_wc)
			{
				m_pia3->ca1_w(1);
				m_stop_addr_dout = 1;
			}
			else
			{
				u8 dout = m_stop_ram[offset & 0x00fff];

				m_pia3->ca1_w(dout);
				m_stop_addr_dout = dout;
				if (dout == 0 && m_stop_nmi_en)
				{
					m_mainnmi->in_w<0>(1);
					m_mainnmi->in_w<0>(0);
				}
			}
		}
		else
		{
			m_stop_ram[offset & 0x00fff] = m_stop_addr_din;
		}
	}

	return m_banked_space->read_byte((m_addr_map_ctrl << 16) | offset);
}

void exorset_state::main_w(offs_t offset, u8 data)
{
	m_banked_space->write_byte((m_addr_map_ctrl << 16) | offset, data);
}

u8 exorset_state::pia_link_pa_r()
{
	return m_link_data;
}

void exorset_state::pia_link_ca2_w(u8 state)
{
	if (m_link_strobe == 1 && state ==0)
	{
		// TODO, might read from an image, but this can be done over
		// the RS232 so it's redundant.
		m_link_data = 0x20;
		// Triggers on a high to low transition.
		m_pia_link->ca1_w(1);
		m_pia_link->ca1_w(0);
		m_pia_link->ca1_w(1);
	}
	m_link_strobe = state;
	return;
}


void exorset_state::pia3_pa_w(u8 data)
{
	m_stop_addr_din = BIT(data, 0);
	m_stop_addr_rw = BIT(data, 1);
	m_stop_addr_wc = BIT(data, 2);
	m_trace_en = BIT(data, 3);
	m_stop_addr_high = (data & 0xf0) << 8;

	if (m_trace_en)
	{
		// The trace timer needs to scale with the CPU clock.
		u32 maincpu_clock = m_maincpu_clock->read();
		if (!maincpu_clock)
			maincpu_clock = 4000000;

		timer_set(attotime::from_ticks(16, maincpu_clock / 4), TIMER_TRACE);
	}
	else
	{
		m_pia3->ca2_w(CLEAR_LINE);
		m_trace = 0;
	}

	if (!m_stop_addr_wc)
	{
		m_mainnmi->in_w<0>(0);
		m_pia3->ca1_w(1);
		m_stop_addr_dout = 1;
	}
}

READ_LINE_MEMBER(exorset_state::pia3_ca1_r)
{
	return m_stop_addr_dout;
}

READ_LINE_MEMBER(exorset_state::pia3_ca2_r)
{
	return m_trace;
}


u8 exorset_state::pia3_pb_r()
{
	u8 res = 0;

	res |= m_sw18->read();
	return res;
}

void exorset_state::pia3_pb_w(u8 data)
{
	m_addr_map_ctrl = BIT(data, 0);
	m_timing_ctrl = BIT(data, 1);
	m_alphanumeric_disp_en = BIT(data, 2);
	m_graphic_disp_en = BIT(data, 3);
	m_cycle_count_en = BIT(data, 4);
}

WRITE_LINE_MEMBER(exorset_state::pia3_cb2_w)
{
	m_stop_nmi_en = !state;
	if (!m_stop_nmi_en)
		m_mainnmi->in_w<0>(0);

}

void exorset_state::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	switch (id)
	{
	case TIMER_TRACE:
		// Triggers an interrupt on a hight to low transition.
		m_pia3->ca2_w(ASSERT_LINE);
		m_pia3->ca2_w(CLEAR_LINE);
		m_trace = 0;
		break;
	default:
		throw emu_fatalerror("Unknown id in exorset_state::device_timer");
	}
}


void exorset_state::pia1_pa_w(u8 data)
{
	// External parallel printer data output.
	m_printer_data = data;
}

int exorset_state::pia1_ca1_r()
{
	// External parallel printer busy input.
	return 0;
}

void exorset_state::pia1_ca2_w(int state)
{
	// External parallel printer data strobe.

	// Trigger on the falling edge.
	if (m_printer_data_strobe == 1 && state == 0)
	{
		m_printer->output(m_printer_data);
		// Toggle the printer acknowledge line as the software waits
		// for a high to low transition.
		m_pia1->ca1_w(ASSERT_LINE);
		m_pia1->ca1_w(CLEAR_LINE);
		m_pia1->ca1_w(ASSERT_LINE);
	}
	m_printer_data_strobe = state;
}


u8 exorset_state::pia1_pb_r()
{
	u8 cassette_input = m_cass->input() > +0.0;
	u8 paper_error = 0;
	u8 busy = 0;
	u8 selected = m_printer->is_ready();

	return (cassette_input << 7) | (busy << 2) | (paper_error << 1) | selected;
}

void exorset_state::pia1_pb_w(u8 data)
{
	m_cass->output(BIT(data, 6) ? +1.0 : -1.0);
}

u8 exorset_state::pia1_cb1_r()
{
	// Printer 'fault' input.
	u8 fault = 0;
	return fault;
}

void exorset_state::pia1_cb2_w(u8 state)
{
	// Printer 'input prime' output.
}



// The special function keys use the PIA port A, and this port has pull up
// resistors, and pulls down when it outputs a zero. The software uses the
// lines for input and output probing different row and column combinations
// and to drive a four input NAND gate connected for each column.

// Determine the NAND gate output.
bool exorset_state::fnkeys_pressed()
{
	u8 rows = ~m_pia2_pa & 0x0f;

	// If any of the column lines are programmed to pull down then the
	// NAND gate output is high.
	if ((m_pia2_pa & 0xf0) != 0xf0)
		return 1;

	// If a row line is programmed to pull down then intersecting columns
	// with a key pressed are pulled low and the NAND gate output is high.
	if ((m_fnkeys_columns[0]->read() & rows) ||
		(m_fnkeys_columns[1]->read() & rows) ||
		(m_fnkeys_columns[2]->read() & rows) ||
		(m_fnkeys_columns[3]->read() & rows))
	{
		return 1;
	}

	// Otherwise all columns are pulled high and the NAND gate output is
	// low.
	return 0;
}

INPUT_CHANGED_MEMBER(exorset_state::fnkeys_changed)
{
	m_pia2->ca1_w(fnkeys_pressed());
}

READ_LINE_MEMBER(exorset_state::pia2_ca1_r)
{
	return fnkeys_pressed();
}

u8 exorset_state::pia2_pa_r()
{
	u8 ret = 0xff;

	// Any lines pull low read back low.
	ret &= ~m_pia2_pa;

	// If a column is pulled low then any intersecting rows with keys
	// pressed are pulled low.

	if ((m_pia2_pa & 0x10) == 0)
		ret &= ~m_fnkeys_columns[0]->read();

	if ((m_pia2_pa & 0x20) == 0)
		ret &= ~m_fnkeys_columns[1]->read();

	if ((m_pia2_pa & 0x40) == 0)
		ret &= ~m_fnkeys_columns[2]->read();

	if ((m_pia2_pa & 0x80) == 0)
		ret &= ~m_fnkeys_columns[3]->read();

	// If a row is pulled low then any interseting columns with keys
	// pressed are pulled low.

	for (int row = 0; row < 4; row++)
	{
		if (BIT(m_pia2_pa, row))
			continue;

		for (int col = 0; col < 4; col++)
		{
			if (BIT(m_fnkeys_columns[col]->read(), row))
				ret &= ~(0x10 << col);
		}
	}

	return ret;
}

void exorset_state::pia2_pa_w(u8 data)
{
	m_pia2_pa = data;
	m_pia2->ca1_w(fnkeys_pressed());
}

u8 exorset_state::pia2_pb_r()
{
	u8 ret = m_term_data;
	m_term_data = 0;
	return ret;
}

WRITE_LINE_MEMBER(exorset_state::pia2_ca2_w)
{
	m_beeper->set_state(state);
}

WRITE_LINE_MEMBER(exorset_state::break_key_w)
{
	m_pia2->cb2_w(!state);
}


MC6845_UPDATE_ROW(exorset_state::update_row)
{
	const pen_t *pen = m_palette->pens();
	int x = 0;
	u16 gbase = m_sw9->read();
	u8 half_en = m_alphanumeric_disp_en || !m_graphic_disp_en;

	for (int column = 0; column < x_count; column++)
	{
		u16 ma2 = (ma + column) & 0x7ff;
		u8 code = m_vram[ma2];
		int invert = (column == cursor_x) ^ BIT(code, 7);
		u16 caddr = ra < 8 ? ((code & 0x7f) << 3) | (ra & 0x07) : 0;
		u8 cdata = m_chargen[caddr];
		u16 gaddr = gbase | ((ma2 & 0x03c0) << 4) | ((ra & 0x0f) << 6) | (ma2 & 0x3f);
		u8 gdata = m_gram[gaddr];

		for (int bit = 0; bit < 8; bit++)
		{
			u8 cd = (BIT(cdata, 7) ^ invert) & m_alphanumeric_disp_en & de;
			u8 gd = BIT(gdata, 7) & m_graphic_disp_en & de;
			u8 half = half_en && !(m_alphanumeric_disp_en && cd);
			int color = (cd | gd) << half;

			bitmap.pix32(y, x++) = pen[color];

			cdata <<= 1;
			gdata <<= 1;
		}
	}
}

static const gfx_layout char_layout =
{
	8,8,
	RGN_FRAC(1,1),
	1,
	{ 0 },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8
};

static GFXDECODE_START(chars)
	GFXDECODE_ENTRY("chargen", 0, char_layout, 0, 1)
GFXDECODE_END



// Keyboard

void exorset_state::kbd_put(u8 code)
{
	m_term_data = code;
	// Triggers on the falling edge.
	m_pia2->cb1_w(ASSERT_LINE);
	m_pia2->cb1_w(CLEAR_LINE);
	m_pia2->cb1_w(ASSERT_LINE);
}


// Floppy controllers

void exorset_state::drive_select_w(u8 data)
{
	m_drive = data & 0x03;
	u8 side = BIT(data, 2);
	u8 enable = BIT(data, 3);

	m_fdc->set_drive(m_drive);
	m_fdc->set_side(!side);

	for (int i = 0; i < 4; i++)
	{
		u8 motor = enable && i == m_drive;
		m_floppy_image[i]->floppy_mon_w(motor);
		// ??
		m_floppy_image[i]->floppy_drive_set_ready_state(ASSERT_LINE, 0);
	}

}

void exorset_state::index_callback(int index, int state)
{
	if (index == m_drive)
		m_fdc->set_index_pulse(state);
}

WRITE_LINE_MEMBER(exorset_state::fdc_index_0_w)
{
	index_callback(0, state);
}

WRITE_LINE_MEMBER(exorset_state::fdc_index_1_w)
{
	index_callback(1, state);
}

WRITE_LINE_MEMBER(exorset_state::fdc_index_2_w)
{
	index_callback(2, state);
}

WRITE_LINE_MEMBER(exorset_state::fdc_index_3_w)
{
	index_callback(3, state);
}


void exorset_state::drive_select2_w(u8 data)
{
	m_drive2 = data & 0x03;
	u8 side = BIT(data, 2);
	u8 enable = BIT(data, 3);

	m_fdc2->set_drive(m_drive2);
	m_fdc2->set_side(!side);

	for (int i = 0; i < 4; i++)
	{
		u8 motor = enable && i == m_drive2;
		m_floppy_image[i + 4]->floppy_mon_w(motor);
		// ??
		m_floppy_image[i + 4]->floppy_drive_set_ready_state(ASSERT_LINE, 0);
	}

}

void exorset_state::index_callback2(int index, int state)
{
	if (index == m_drive2)
		m_fdc2->set_index_pulse(state);
}

WRITE_LINE_MEMBER(exorset_state::fdc2_index_0_w)
{
	index_callback2(0, state);
}

WRITE_LINE_MEMBER(exorset_state::fdc2_index_1_w)
{
	index_callback2(1, state);
}

WRITE_LINE_MEMBER(exorset_state::fdc2_index_2_w)
{
	index_callback2(2, state);
}

WRITE_LINE_MEMBER(exorset_state::fdc2_index_3_w)
{
	index_callback2(3, state);
}

static const floppy_interface exorset_floppy_interface =
{
	FLOPPY_STANDARD_5_25_SSSD,
	LEGACY_FLOPPY_OPTIONS_NAME(mdos),
	"exorset_flop"
};


void exorset_state::machine_reset()
{
	u32 maincpu_clock = m_maincpu_clock->read();
	if (maincpu_clock)
	{
		m_maincpu->set_clock(maincpu_clock);
		m_ptm->set_ext_clock(0, maincpu_clock / 4);
	}

	m_pia2_pa = 0;
	m_term_data = 0;

	m_stop_addr_din = 0;
	m_stop_addr_rw = 0;
	m_stop_addr_wc = 0;
	m_stop_addr_high = 0;
	memset(m_stop_ram, 0x00, sizeof(m_stop_ram));
	m_stop_addr_dout = 0;
	m_stop_nmi_en = 0;
	m_trace_en = 0;
	m_trace = 0;
	m_addr_map_ctrl = 0;
	m_timing_ctrl = 0;
	m_alphanumeric_disp_en = 0;
	m_graphic_disp_en = 0;
	m_cycle_count_en = 0;

	m_printer_data = 0;
	m_printer_data_strobe = 1;
	m_pia1->ca1_w(ASSERT_LINE);

	m_beeper->set_state(0);

	for (auto &img : m_floppy_image)
	{
		if (img.found())
		{
			img->floppy_drive_set_ready_state( FLOPPY_DRIVE_READY, 0 );
			img->floppy_drive_set_rpm( 300. );
			img->floppy_drive_seek( - img->floppy_drive_get_current_track() );
		}
	}
	m_drive = 0;
	m_drive2 = 0;

	m_palette->set_pen_color(0, 0x00, 0x00, 0x00); // Off
	m_palette->set_pen_color(1, 35 + 110, 200 + 55, 75 + 110); // On
	m_palette->set_pen_color(2, 35 + 55, 200, 75 + 55); // Half

	if (m_erom_fd8_patch->read())
	{
		// Patch
		u8* erom = memregion("fdc")->base();
		erom[0x013e] = 0x07; // 2000 usable sectors.
		erom[0x013f] = 0xd2;
		erom[0x014c] = 0x1a; // 26 sectors
		erom[0x0151] = 0x1a; // 26 sectors
		erom[0x01bd] = 0x1a + 1; // 26 sectors
		erom[0x03ff] = 0x43; // 'C'
	}

	if (m_erom2_fd8_patch->read())
	{
		// Patch
		u8* erom = memregion("fdc2")->base();

		erom[0x013e] = 0x07; // 2000 usable sectors.
		erom[0x013f] = 0xd2;
		erom[0x014c] = 0x1a; // 26 sectors
		erom[0x0151] = 0x1a; // 26 sectors
		erom[0x01bd] = 0x1a + 1; // 26 sectors
		erom[0x03ff] = 0x43; // 'C'
	}

	m_link_strobe = 0;
	m_link_data = 0;
}

void exorset_state::machine_start()
{
	m_banked_space = &subdevice<address_map_bank_device>("bankdev")->space(AS_PROGRAM);

	save_item(NAME(m_pia2_pa));
	save_item(NAME(m_term_data));

	save_item(NAME(m_stop_addr_din));
	save_item(NAME(m_stop_addr_rw));
	save_item(NAME(m_stop_addr_wc));
	save_item(NAME(m_stop_addr_high));
	save_item(NAME(m_stop_ram));
	save_item(NAME(m_stop_addr_dout));
	save_item(NAME(m_stop_nmi_en));
	save_item(NAME(m_trace_en));
	save_item(NAME(m_trace));
	save_item(NAME(m_addr_map_ctrl));
	save_item(NAME(m_timing_ctrl));
	save_item(NAME(m_alphanumeric_disp_en));
	save_item(NAME(m_graphic_disp_en));
	save_item(NAME(m_cycle_count_en));

	save_item(NAME(m_printer_data));
	save_item(NAME(m_printer_data_strobe));

	save_item(NAME(m_drive));
	save_item(NAME(m_drive2));

	save_item(NAME(m_link_strobe));
	save_item(NAME(m_link_data));
}

void exorset_state::exorset(machine_config &config)
{
	MC6809(config, m_maincpu, 4_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &exorset_state::dbg_map);

	ADDRESS_MAP_BANK(config, m_bankdev, 0);
	m_bankdev->set_endianness(ENDIANNESS_BIG);
	m_bankdev->set_data_width(8);
	m_bankdev->set_addr_width(17);
	m_bankdev->set_addrmap(AS_PROGRAM, &exorset_state::mem_map);

	INPUT_MERGER_ANY_HIGH(config, m_mainirq).output_handler().set_inputline(m_maincpu, M6809_IRQ_LINE);
	INPUT_MERGER_ANY_HIGH(config, m_mainnmi).output_handler().set_inputline(m_maincpu, INPUT_LINE_NMI);

	PTM6840(config, m_ptm, 1000000);
	m_ptm->set_external_clocks(1000000, 0, 0);
	m_ptm->o3_callback().set(FUNC(exorset_state::ptm_o3_callback));
	m_ptm->irq_callback().set(m_mainnmi, FUNC(input_merger_device::in_w<1>));

	ACIA6850(config, m_acia, 0);
	m_acia->txd_handler().set("rs232", FUNC(rs232_port_device::write_txd));
	m_acia->rts_handler().set("rs232", FUNC(rs232_port_device::write_rts));
	m_acia->irq_handler().set(m_mainnmi, FUNC(input_merger_device::in_w<2>));

	rs232_port_device &rs232(RS232_PORT(config, "rs232", default_rs232_devices, nullptr));
	rs232.rxd_handler().set(m_acia, FUNC(acia6850_device::write_rxd));

	PIA6821(config, m_pia3, 0);
	m_pia3->writepa_handler().set(FUNC(exorset_state::pia3_pa_w));
	m_pia3->readca1_handler().set(FUNC(exorset_state::pia3_ca1_r));
	m_pia3->readca2_handler().set(FUNC(exorset_state::pia3_ca2_r));
	m_pia3->readpb_handler().set(FUNC(exorset_state::pia3_pb_r));
	m_pia3->writepb_handler().set(FUNC(exorset_state::pia3_pb_w));
	m_pia3->cb2_handler().set(FUNC(exorset_state::pia3_cb2_w));
	m_pia3->irqa_handler().set(m_mainnmi, FUNC(input_merger_device::in_w<3>));

	PIA6821(config, m_pia1, 0);
	m_pia1->writepa_handler().set(FUNC(exorset_state::pia1_pa_w));
	m_pia1->readca1_handler().set(FUNC(exorset_state::pia1_ca1_r));
	m_pia1->ca2_handler().set(FUNC(exorset_state::pia1_ca2_w));
	m_pia1->readpb_handler().set(FUNC(exorset_state::pia1_pb_r));
	m_pia1->writepb_handler().set(FUNC(exorset_state::pia1_pb_w));
	m_pia1->readcb1_handler().set(FUNC(exorset_state::pia1_cb1_r));
	m_pia1->cb2_handler().set(FUNC(exorset_state::pia1_cb2_w));

	PRINTER(config, m_printer, 0);

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(50);
	m_screen->set_size(80 * 8, 24 * 12);
	m_screen->set_visarea(0, 80 * 8 - 1, 0, 24 * 12 - 1);
	m_screen->set_screen_update("mc6845", FUNC(mc6845_device::screen_update));

	PALETTE(config, m_palette).set_entries(3);

	MC6845(config, m_mc6845, 16000000/8);
	m_mc6845->set_screen(m_screen);
	m_mc6845->set_show_border_area(false);
	m_mc6845->set_char_width(8);
	m_mc6845->set_update_row_callback(FUNC(exorset_state::update_row));

	GFXDECODE(config, "gfxdecode", m_palette, chars);

	PIA6821(config, m_pia2);
	m_pia2->readpa_handler().set(FUNC(exorset_state::pia2_pa_r));
	m_pia2->writepa_handler().set(FUNC(exorset_state::pia2_pa_w));
	m_pia2->readca1_handler().set(FUNC(exorset_state::pia2_ca1_r));
	m_pia2->readpb_handler().set(FUNC(exorset_state::pia2_pb_r));
	m_pia2->ca2_handler().set(FUNC(exorset_state::pia2_ca2_w));
	m_pia2->irqa_handler().set(m_mainnmi, FUNC(input_merger_device::in_w<4>));
	m_pia2->irqb_handler().set(m_mainnmi, FUNC(input_merger_device::in_w<5>));

	MC6843(config, m_fdc, 0);
	m_fdc->set_floppy_drives(m_floppy_image[0], m_floppy_image[1], m_floppy_image[2], m_floppy_image[3]);
	m_fdc->irq().set(m_mainirq, FUNC(input_merger_device::in_w<0>));

	LEGACY_FLOPPY(config, m_floppy_image[0], 0, &exorset_floppy_interface);
	m_floppy_image[0]->out_idx_cb().set(FUNC(exorset_state::fdc_index_0_w));
	LEGACY_FLOPPY(config, m_floppy_image[1], 0, &exorset_floppy_interface);
	m_floppy_image[1]->out_idx_cb().set(FUNC(exorset_state::fdc_index_1_w));
	LEGACY_FLOPPY(config, m_floppy_image[2], 0, &exorset_floppy_interface);
	m_floppy_image[2]->out_idx_cb().set(FUNC(exorset_state::fdc_index_2_w));
	LEGACY_FLOPPY(config, m_floppy_image[3], 0, &exorset_floppy_interface);
	m_floppy_image[3]->out_idx_cb().set(FUNC(exorset_state::fdc_index_3_w));

	MC6843(config, m_fdc2, 0);
	m_fdc2->set_floppy_drives(m_floppy_image[4], m_floppy_image[5], m_floppy_image[6], m_floppy_image[7]);
	m_fdc2->irq().set(m_mainirq, FUNC(input_merger_device::in_w<1>));

	LEGACY_FLOPPY(config, m_floppy_image[4], 0, &exorset_floppy_interface);
	m_floppy_image[4]->out_idx_cb().set(FUNC(exorset_state::fdc2_index_0_w));
	LEGACY_FLOPPY(config, m_floppy_image[5], 0, &exorset_floppy_interface);
	m_floppy_image[5]->out_idx_cb().set(FUNC(exorset_state::fdc2_index_1_w));
	LEGACY_FLOPPY(config, m_floppy_image[6], 0, &exorset_floppy_interface);
	m_floppy_image[6]->out_idx_cb().set(FUNC(exorset_state::fdc2_index_2_w));
	LEGACY_FLOPPY(config, m_floppy_image[7], 0, &exorset_floppy_interface);
	m_floppy_image[7]->out_idx_cb().set(FUNC(exorset_state::fdc2_index_3_w));

	SPEAKER(config, "mono").front_center();
	BEEP(config, m_beeper, 2000);
	m_beeper->add_route(ALL_OUTPUTS, "mono", 0.25);

	CASSETTE(config, m_cass);
	m_cass->set_default_state(CASSETTE_STOPPED | CASSETTE_MOTOR_ENABLED | CASSETTE_SPEAKER_ENABLED);
	m_cass->add_route(ALL_OUTPUTS, "mono", 0.05);

	generic_keyboard_device &keyboard(GENERIC_KEYBOARD(config, "keyboard", 0));
	keyboard.set_keyboard_callback(FUNC(exorset_state::kbd_put));

	PIA6821(config, m_pia_link, 0);
	m_pia_link->readpa_handler().set(FUNC(exorset_state::pia_link_pa_r));
	m_pia_link->ca2_handler().set(FUNC(exorset_state::pia_link_ca2_w));
}

/* ROM definition */
ROM_START( exorset )
	ROM_REGION( 0x0400, "fdc", 0 )
	ROM_LOAD("fdcexorset30.bin", 0x0000, 0x0400, CRC(c8e61845) SHA1(a549c08124d2c8a34f43d2d2ed31d9773a25456d))

	ROM_REGION( 0x0400, "fdc2", 0 )
	ROM_LOAD("fdcexorset30.bin", 0x0000, 0x0400, CRC(c8e61845) SHA1(a549c08124d2c8a34f43d2d2ed31d9773a25456d))

	ROM_REGION( 0x1000, "exorbug", 0 )
	ROM_LOAD("exorbug12.bin", 0x0000, 0x1000, CRC(ef65bcf4) SHA1(10f75f69bb05c2c02bc056417769c99339231c03))

	ROM_REGION(0x0400, "chargen", 0)
	ROM_LOAD("mcm6674p.chr", 0x0000, 0x0400, CRC(1c22088a) SHA1(b5f0bd0cfdec0cd5c1cb764506bef3c17d6af0eb))

ROM_END

/* Driver */

//    YEAR  NAME    PARENT  COMPAT  MACHINE  INPUT  CLASS        INIT        COMPANY                                     FULLNAME      FLAGS
COMP( 1979, exorset,  0,      0,      exorset,   exorset, exorset_state, empty_init, "Motorola", "EXORset 30", MACHINE_SUPPORTS_SAVE)
