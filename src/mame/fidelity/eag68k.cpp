// license:BSD-3-Clause
// copyright-holders:hap
// thanks-to:Berger, yoyo_chessboard
/*******************************************************************************

Fidelity 68000-based Elite Avant Garde driver
For 6502-based EAG, see elite.cpp

Excel 68000 I/O is very similar to EAG, so it's handled in this driver as well

TODO:
- unemulated waitstates with DTACK
- V10 CPU emulation is too slow, MAME 68040 opcode timing is same as 68030 but in
  reality it is much faster, same goes for V11 of course (see note below)
- V11 CPU should be M68EC060, not yet emulated. Now using M68EC040 in its place
- V11 beeper is too high pitched, related to wrong CPU type too?
  maybe waitstates or clock divider on I/O access.
- Do the opening book modules work? It detects the rom, but then does nothing
  with it. Maybe support is limited since the basic CB9/CB16 modules have no use
  on newer chesscomputers with a large internal opening library.
- does premiere only use half of the nvram? 0x600000-0x603fff seems unused
- premiere unknown reads/writes to 0x500000 with the Vancouver program

Currently(May 2020) when compared to the real chesscomputers, to get closer to the
actual speed, overclock V10 and V11 to 230%. This can be done by starting MAME
with the -cheat option and going to the Slider Controls menu, hold Ctrl and press
Right to overclock maincpu.

================================================================================

Excel 68000 (model 6094) overview:
- 16KB RAM(2*SRM2264C-10 @ U8/U9), 64KB ROM(2*AT27C256-15DC @ U6/U7)
- HD68HC000P12 CPU, 12MHz XTAL
- PCB label 510-1129A01
- PCB has edge connector for module, but no external slot

There's room for 2 SIMMs at U22 and U23, unpopulated in Excel 68000 and Mach III.
Mach II has 2*64KB DRAM with a MB1422A DRAM controller @ 25MHz.
Mach III has wire mods from U22/U23 to U8/U9(2*8KB + 2*32KB piggybacked).
Mach IV has 2*256KB DRAM, and a daughterboard(510.1123B01) for the 68020 + 32KB RAM.

I/O is via TTL, overall very similar to EAG.

Holding NEW GAME does a quick self-test, on Mach III and Mach IV it will also
display a ROM checksum.

fex68km4 continuously tests RAM at boot and displays "512", this is normal.
To start, hold New Game or Clear.

================================================================================

Elite Avant Garde 2265 (EAG, model 6114)
----------------------------------------

There are 5 versions of model 6114(V1 to V5):

V1: 128KB DRAM, no EEPROM
V2: 128KB DRAM
V3: 512KB DRAM
V4: 1MB DRAM
V5: 128KB+16KB DRAM, dual-CPU! (2*68K @ 16MHz)

V2/V3/V4 have the same program, V2/V3 versions can be run by decreasing the
RAM size (-ramsize option). It's not verified if V1 has the same program, but
it probably does.

V6-V11 are on model 6117. Older 1986 model 6081/6088/6089 uses a 6502 CPU.

Hardware info:
--------------
- MC68HC000P12F 16MHz CPU, 16MHz XTAL
- MB1422A DRAM Controller, 25MHz XTAL near, 4 DRAM slots
  (V2: slot 2 & 3 64KB, V3: slot 2 & 3 256KB)
- 2*27C512 64KB EPROM, 2*KM6264AL-10 8KB SRAM, 2*AT28C64X 8KB EEPROM
- OKI M82C51A-2 USART, 4.9152MHz XTAL
- other special: magnet sensors, external module slot, serial port

IRQ source is a 4,9152MHz quartz crystal (Y3) connected to a 74HC4060 (U8,
ripple counter/divider). From Q13 output (counter=8192) we obtain the IRQ signal
applied to IPL1 of 68000 (pin 24) 4,9152 MHz / 8192 = 600 Hz.

The module slot pinout is different from SCC series. The data on those appears
to be compatible with EAG though, and will load fine with an adapter.

The USART allows for a serial connection between the chess computer and another
device, for example the Fidelity Challenger Printer, or a PC. It expects a baud
rate of 600.

Fidelity released a DOS tool called EAGLINK which featured PC printer support,
complete I/O control, detailed information while the program is 'thinking', etc.
It can be enabled with POP3 H3.

The chessboard is the same old wooden Auto Sensory board from EAS / Prestige.
6502-based EAG can easily be upgraded by swapping the PCB. Fidelity also provided
support for converting EAS and Prestige, via optional diode D8 next to the USART.

To play with this EAS / Prestige configuration on MAME, the user needs to provide
external artwork, or copy internal artwork fidel_eas.lay or fidel_pc.lay.

Memory map: (of what is known)
-----------
000000-01FFFF: 128KB ROM
104000-107FFF: 16KB SRAM
200000-2FFFFF: hashtable DRAM (max. 1MB)
300000-30000F W hi d0: NE591: 7seg data
300000-30000F W lo d0: NE591: LED data
300000-30000F R lo d7: 74259: keypad rows 0-7
400000-40000F W lo d0: 74259,74145/7442: led/keypad mux, buzzer out
400000-4????? R hi: external module slot
700002-700003 R lo d7: 74251: keypad row 8
604000-607FFF: 16KB EEPROM

================================================================================

Elite Avant Garde 2325 (EAG, model 6117)
----------------------------------------

There are 6 versions of model 6117(V6 to V11). From a programmer's point of view,
the hardware is very similar to model 6114.

V6: 68020, 512KB hashtable RAM
V7: 68020, 1MB h.RAM
V8: 2*68020, 512KB+128KB h.RAM (unreleased?)
V9: 68030, 1MB h.RAM
V10: 68040, 1MB h.RAM
V11: 68060, high speed, 2MB h.RAM (half unused?)

V6/V7/V9 have the same program, V6 can be run by decreasing the RAM size. V11
supposedly has the same program as V10.

V7 Hardware info:
-----------------
- 510.1139A01 daughterboard with MC68020RC20E or MC68020RC25E @ 20MHz,
  and 32KB RAM (4*MCM6264P35)
- rest is same as 6114

V7 Memory map:
--------------
000000-01FFFF: 128KB ROM
104000-107FFF: 16KB SRAM (unused?)
200000-2FFFFF: hashtable SRAM
300000-30000x: see model 6114
400000-40000x: see model 6114
700000-70000x: see model 6114
604000-607FFF: 16KB EEPROM
800000-807FFF: 32KB SRAM

V10 Hardware info:
------------------
- 68040 CPU, 25MHz
- other: assume same or very similar to V11(see below)

The ROM dump came from the V11(see below). Built-in factory test proves
that this program is a V10. Hold TB button immediately after power-on and
press it for a sequence of tests:
1) all LEDs on
2) F40C: V10 ROM checksum 1
3) 38b9: V10 ROM checksum 2
4) xxxx: external module ROM checksum (0000 if no module present)
5) xxxx: user settings (stored in EEPROM)
6) xxxx: "
7) 1024: hashtable RAM size
8) return to game

V11 Hardware info:
------------------
- MC68EC060RC75 CPU, 36MHz XTAL(36MHz bus, 72MHz CPU), CPU cooler required
- 4*CXK5863AP-20 8KB SRAM, 4*K6X4008C1F-DF55 512KB CMOS SRAM
- 4*M27C256B 32KB EPROM, 2*AT28C64 8KB EEPROM, 5*GAL16V8D
- NEC D71051C USART, on quick glance it's same as the OKI USART
- same as 6114: NE555, SN74HC4060, module slot, chessboard, ..

This is a custom overclocked V10, manufactured by Wilfried Bucke. PCB is marked:
"CHESS HW DESIGN COPYRIGHT 22-10-2002: REVA03 510.1136A01/510.1144B01 COMPONENT SIDE"
There are two versions of this, one with a 66MHz CPU, one with a 72MHz CPU.
Maybe other differences too?

V1x Memory map:
---------------
000000-01FFFF: 128KB ROM
280000-37FFFF: hashtable SRAM
B0000x-xxxxxx: see V7, -800000

================================================================================

Elite Premiere (model 6131)
---------------------------

This 2-in-1 chesscomputer was manufactured after Fidelity became a H+G subsidiary,
it's still Fidelity branded. It has a switch where the module slot used to be,
for selecting between Vancouver (Richard Lang) and 2265 (Spracklen's) programs.
The 2265 ROM half is almost identical to EAG V2 (the elite_1.6 one), the handful
of bytes difference is due to the changed hash RAM start address.

The switch is not supposed to be flipped while power is on, hence MAME will only
check it at reset (otherwise, the CPU will crash and may overwrite NVRAM). Each
program has its own NVRAM bank, so unfortunately the user can't save the game on
one program and then analyze or continue on the other.

Hardware info:
--------------
- TMP68HC000P-16, 16MHz XTAL
- 256KB ROM (2*M27C1001), 256KB RAM (2*TC551001PL-10)
- 64KB battery-backed RAM (2*KM62256AP-10), Dallas DS1210
- OKI M82C51A-2 USART, 4.9152MHz XTAL

RAM can't be expanded, there are MB1422A and SIMM PCB markings, but unpopulated.
The I/O remains the same as EAG V2.

*******************************************************************************/

#include "emu.h"

#include "bus/generic/carts.h"
#include "bus/generic/slot.h"
#include "bus/rs232/rs232.h"
#include "cpu/m68000/m68000.h"
#include "cpu/m68000/m68020.h"
#include "cpu/m68000/m68030.h"
#include "cpu/m68000/m68040.h"
#include "machine/clock.h"
#include "machine/gen_latch.h"
#include "machine/i8251.h"
#include "machine/ram.h"
#include "machine/nvram.h"
#include "machine/sensorboard.h"
#include "sound/dac.h"
#include "video/pwm.h"

#include "softlist_dev.h"
#include "speaker.h"

// internal artwork
#include "fidel_ex_68k.lh"
#include "fidel_eag_68k.lh"


namespace {

// EAG / shared

class eag_state : public driver_device
{
public:
	eag_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_ram(*this, "ram"),
		m_usart(*this, "usart"),
		m_board(*this, "board"),
		m_display(*this, "display"),
		m_dac(*this, "dac"),
		m_inputs(*this, "IN.%u", 0)
	{ }

	// machine configs
	void eagv4(machine_config &config);
	void eagv5(machine_config &config);
	void eagv7(machine_config &config);
	void eagv9(machine_config &config);
	void eagv10(machine_config &config);
	void eagv11(machine_config &config);

	void init_eag();

	DECLARE_INPUT_CHANGED_MEMBER(in1_changed) { update_dsr(); }

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	void eag_base(machine_config &config);

	// devices/pointers
	required_device<m68000_base_device> m_maincpu;
	optional_device<ram_device> m_ram;
	optional_device<i8251_device> m_usart;
	required_device<sensorboard_device> m_board;
	required_device<pwm_display_device> m_display;
	required_device<dac_1bit_device> m_dac;
	optional_ioport_array<4> m_inputs;

	u8 m_select = 0;
	u8 m_inp_mux = 0;
	u8 m_7seg_data = 0;
	u8 m_led_data = 0;

	// address maps
	void eag_map(address_map &map) ATTR_COLD;
	void eagv7_map(address_map &map) ATTR_COLD;
	void eagv10_map(address_map &map) ATTR_COLD;

	// I/O handlers
	void update_display();
	void update_dsr();
	void mux_w(offs_t offset, u8 data);
	u8 input_r(offs_t offset);
	void leds_w(offs_t offset, u8 data);
	void digit_w(offs_t offset, u8 data);
};

void eag_state::machine_start()
{
	// register for savestates
	save_item(NAME(m_select));
	save_item(NAME(m_inp_mux));
	save_item(NAME(m_7seg_data));
	save_item(NAME(m_led_data));
}

void eag_state::machine_reset()
{
	update_dsr();
}


// EAG V5

class eagv5_state : public eag_state
{
public:
	eagv5_state(const machine_config &mconfig, device_type type, const char *tag) :
		eag_state(mconfig, type, tag),
		m_subcpu(*this, "subcpu"),
		m_mainlatch(*this, "mainlatch"),
		m_sublatch(*this, "sublatch")
	{ }

	// machine configs
	void eagv5(machine_config &config);

private:
	// devices/pointers
	required_device<cpu_device> m_subcpu;
	required_device<generic_latch_8_device> m_mainlatch;
	required_device<generic_latch_8_device> m_sublatch;

	// address maps
	void main_map(address_map &map) ATTR_COLD;
	void sub_map(address_map &map) ATTR_COLD;

	// I/O handlers
	void reset_subcpu_w(u8 data);
	u8 main_ack_r();
	u8 sub_ack_r();
};


// Elite Premiere

class premiere_state : public eag_state
{
public:
	premiere_state(const machine_config &mconfig, device_type type, const char *tag) :
		eag_state(mconfig, type, tag),
		m_nvram(*this, "nvram", 0x10000, ENDIANNESS_BIG),
		m_nvrambank(*this, "nvrambank"),
		m_rombank(*this, "rombank")
	{ }

	// machine configs
	void premiere(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	// devices/pointers
	memory_share_creator<u16> m_nvram;
	required_memory_bank m_nvrambank;
	required_memory_bank m_rombank;

	// address maps
	void main_map(address_map &map) ATTR_COLD;
};

void premiere_state::machine_start()
{
	eag_state::machine_start();

	// init banks
	m_rombank->configure_entries(0, 2, memregion("maincpu")->base(), 0x20000);
	m_nvrambank->configure_entries(0, 2, m_nvram, 0x8000);
}

void premiere_state::machine_reset()
{
	eag_state::machine_reset();

	// program switch directly selects bank
	const u8 bank = m_inputs[3]->read() & 1;
	m_rombank->set_entry(bank);
	m_nvrambank->set_entry(bank);
}


// Excel 68000

class excel68k_state : public eag_state
{
public:
	excel68k_state(const machine_config &mconfig, device_type type, const char *tag) :
		eag_state(mconfig, type, tag)
	{ }

	// machine configs
	void fex68k(machine_config &config);
	void fex68km2(machine_config &config);
	void fex68km3(machine_config &config);
	void fex68km4(machine_config &config);

private:
	// address maps
	void fex68k_map(address_map &map) ATTR_COLD;
	void fex68km2_map(address_map &map) ATTR_COLD;
	void fex68km3_map(address_map &map) ATTR_COLD;
	void fex68km4_map(address_map &map) ATTR_COLD;
};



/*******************************************************************************
    I/O
*******************************************************************************/

// TTL/generic

void eag_state::update_display()
{
	// Excel 68000: 4*7seg leds, 8*8 chessboard leds
	// EAG: 8*7seg leds(2 panels), (8+1)*8 chessboard leds
	u8 seg_data = bitswap<8>(m_7seg_data,0,1,3,2,7,5,6,4);
	u8 led_data = bitswap<8>(m_led_data,0,1,2,3,4,5,6,7);
	m_display->matrix(1 << m_inp_mux, led_data << 8 | seg_data);
}

void eag_state::update_dsr()
{
	// USART DSR: 3 more buttons (and optional diode) on EAG
	if (m_usart != nullptr)
		m_usart->write_dsr(BIT(~m_inputs[1]->read(), m_inp_mux) & (BIT(m_select, 4) | m_inputs[2]->read()));
}

void eag_state::mux_w(offs_t offset, u8 data)
{
	// a1-a3,d0: 74259
	u8 mask = 1 << offset;
	m_select = (m_select & ~mask) | ((data & 1) ? mask : 0);

	// 74259 Q0-Q3: 74145 A-D (Q5-Q7 N/C)
	// 74259 Q4: optional diode to USART DSR on EAG
	m_inp_mux = m_select & 0xf;
	update_dsr();

	// 74145 0-8: input mux, digit/led select
	// 74145 9: speaker out
	m_dac->write(BIT(1 << m_inp_mux, 9));
	update_display();
}

u8 eag_state::input_r(offs_t offset)
{
	u8 data = 0;

	// a1-a3,d7: multiplexed inputs (active low)
	// read chessboard sensors
	if (m_inp_mux < 8)
	{
		// EAG chessboard is rotated 90 degrees
		if (m_inputs[2].read_safe(0) & 1)
			data = m_board->read_rank(m_inp_mux);
		else
			data = m_board->read_file(m_inp_mux, true);
	}

	// read button panel
	else if (m_inp_mux == 8)
		data = m_inputs[0]->read();

	return ~data << offset & 0x80;
}

void eag_state::leds_w(offs_t offset, u8 data)
{
	// a1-a3,d0: led data
	m_led_data = (m_led_data & ~(1 << offset)) | ((data & 1) << offset);
	update_display();
}

void eag_state::digit_w(offs_t offset, u8 data)
{
	// a1-a3,d0(d8): digit segment data
	m_7seg_data = (m_7seg_data & ~(1 << offset)) | ((data & 1) << offset);
	update_display();
}


// EAG V5

void eagv5_state::reset_subcpu_w(u8 data)
{
	// reset subcpu, from trigger to monostable 555 (R1=47K, C1=1uF)
	m_subcpu->pulse_input_line(INPUT_LINE_RESET, attotime::from_msec(52));
}

u8 eagv5_state::main_ack_r()
{
	// d8,d9: latches ack state
	return (m_mainlatch->pending_r() << 1 ^ 2) | m_sublatch->pending_r();
}

u8 eagv5_state::sub_ack_r()
{
	// d8,d9: latches ack state
	return (m_sublatch->pending_r() << 1 ^ 2) | m_mainlatch->pending_r();
}



/*******************************************************************************
    Address Maps
*******************************************************************************/

// Excel 68000

void excel68k_state::fex68k_map(address_map &map)
{
	map(0x000000, 0x00ffff).rom();
	map(0x000000, 0x00000f).mirror(0x00fff0).w(FUNC(excel68k_state::leds_w)).umask16(0x00ff);
	map(0x000000, 0x00000f).mirror(0x00fff0).w(FUNC(excel68k_state::digit_w)).umask16(0xff00);
	map(0x044000, 0x047fff).ram();
	map(0x100000, 0x10000f).mirror(0x03fff0).r(FUNC(excel68k_state::input_r)).umask16(0x00ff);
	map(0x140000, 0x14000f).mirror(0x03fff0).w(FUNC(excel68k_state::mux_w)).umask16(0x00ff);
}

void excel68k_state::fex68km2_map(address_map &map)
{
	fex68k_map(map);
	map(0x200000, 0x21ffff).ram();
}

void excel68k_state::fex68km3_map(address_map &map)
{
	fex68k_map(map);
	map(0x200000, 0x20ffff).ram();
}

void excel68k_state::fex68km4_map(address_map &map)
{
	map(0x00000000, 0x0000ffff).rom();
	map(0x00000000, 0x0000000f).mirror(0x00fff0).w(FUNC(excel68k_state::leds_w)).umask32(0x00ff00ff);
	map(0x00000000, 0x0000000f).mirror(0x00fff0).w(FUNC(excel68k_state::digit_w)).umask32(0xff00ff00);
	map(0x00044000, 0x00047fff).ram(); // unused?
	map(0x00100000, 0x0010000f).mirror(0x03fff0).r(FUNC(excel68k_state::input_r)).umask32(0x00ff00ff);
	map(0x00140000, 0x0014000f).mirror(0x03fff0).w(FUNC(excel68k_state::mux_w)).umask32(0x00ff00ff);
	map(0x00200000, 0x0027ffff).ram();
	map(0x00400000, 0x00407fff).ram();
}


// EAG

void eag_state::init_eag()
{
	// eag_map: DRAM slots at $200000-$2fffff - V1/V2/V5: 128K, V3/V6: 512K, V4/V7/V9: 1M
	m_maincpu->space(AS_PROGRAM).install_ram(0x200000, 0x200000 + m_ram->size() - 1, m_ram->pointer());
}

void eag_state::eag_map(address_map &map)
{
	map(0x000000, 0x01ffff).rom();
	map(0x104000, 0x107fff).ram();
	map(0x300000, 0x30000f).mirror(0x000010).w(FUNC(eag_state::digit_w)).umask16(0xff00).nopr();
	map(0x300000, 0x30000f).mirror(0x000010).rw(FUNC(eag_state::input_r), FUNC(eag_state::leds_w)).umask16(0x00ff);
	map(0x400000, 0x40000f).w(FUNC(eag_state::mux_w)).umask16(0x00ff);
	map(0x400000, 0x407fff).r("cartslot", FUNC(generic_slot_device::read_rom)).umask16(0xff00);
	map(0x604000, 0x607fff).ram().share("nvram");
	map(0x700000, 0x700003).rw(m_usart, FUNC(i8251_device::read), FUNC(i8251_device::write)).umask16(0x00ff);
}

void eagv5_state::main_map(address_map &map)
{
	eag_map(map);
	map(0x500000, 0x500000).r(m_sublatch, FUNC(generic_latch_8_device::read)).w(m_mainlatch, FUNC(generic_latch_8_device::write));
	map(0x500002, 0x500002).rw(FUNC(eagv5_state::main_ack_r), FUNC(eagv5_state::reset_subcpu_w));
}

void eagv5_state::sub_map(address_map &map)
{
	map(0x000000, 0x00ffff).rom();
	map(0x000000, 0x000001).mirror(0x00fffe).w(m_sublatch, FUNC(generic_latch_8_device::write)).umask16(0x00ff);
	map(0x044000, 0x047fff).ram();
	map(0x140000, 0x140000).r(FUNC(eagv5_state::sub_ack_r));
	map(0x140001, 0x140001).r(m_mainlatch, FUNC(generic_latch_8_device::read));
}

void eag_state::eagv7_map(address_map &map)
{
	map(0x000000, 0x01ffff).rom();
	map(0x104000, 0x107fff).ram();
	map(0x300000, 0x30000f).mirror(0x000010).w(FUNC(eag_state::digit_w)).umask32(0xff00ff00).nopr();
	map(0x300000, 0x30000f).mirror(0x000010).rw(FUNC(eag_state::input_r), FUNC(eag_state::leds_w)).umask32(0x00ff00ff);
	map(0x400000, 0x40000f).w(FUNC(eag_state::mux_w)).umask32(0x00ff00ff);
	map(0x400000, 0x407fff).r("cartslot", FUNC(generic_slot_device::read_rom)).umask32(0xff00ff00);
	map(0x604000, 0x607fff).ram().share("nvram");
	map(0x700000, 0x700003).rw(m_usart, FUNC(i8251_device::read), FUNC(i8251_device::write)).umask32(0x00ff00ff);
	map(0x800000, 0x807fff).ram();
}

void eag_state::eagv10_map(address_map &map)
{
	map(0x00000000, 0x0001ffff).rom();
	map(0x00280000, 0x0037ffff).ram();
	map(0x00b00000, 0x00b0000f).mirror(0x00000010).w(FUNC(eag_state::digit_w)).umask32(0xff00ff00).nopr();
	map(0x00b00000, 0x00b0000f).mirror(0x00000010).rw(FUNC(eag_state::input_r), FUNC(eag_state::leds_w)).umask32(0x00ff00ff);
	map(0x00c00000, 0x00c0000f).w(FUNC(eag_state::mux_w)).umask32(0x00ff00ff);
	map(0x00c00000, 0x00c07fff).r("cartslot", FUNC(generic_slot_device::read_rom)).umask32(0xff00ff00);
	map(0x00e04000, 0x00e07fff).ram().share("nvram");
	map(0x00f00000, 0x00f00003).rw(m_usart, FUNC(i8251_device::read), FUNC(i8251_device::write)).umask32(0x00ff00ff);
	map(0x01018000, 0x0101ffff).ram();
}

void premiere_state::main_map(address_map &map)
{
	map(0x000000, 0x01ffff).bankr(m_rombank);
	map(0x100000, 0x13ffff).ram();
	map(0x300000, 0x30000f).mirror(0x000010).w(FUNC(premiere_state::digit_w)).umask16(0xff00).nopr();
	map(0x300000, 0x30000f).mirror(0x000010).rw(FUNC(premiere_state::input_r), FUNC(premiere_state::leds_w)).umask16(0x00ff);
	map(0x400000, 0x40000f).w(FUNC(premiere_state::mux_w)).umask16(0x00ff);
	map(0x600000, 0x607fff).mirror(0x008000).bankrw(m_nvrambank);
	map(0x700000, 0x700003).rw(m_usart, FUNC(i8251_device::read), FUNC(i8251_device::write)).umask16(0x00ff);
}



/*******************************************************************************
    Input Ports
*******************************************************************************/

static INPUT_PORTS_START( excel68k )
	PORT_START("IN.0")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_R) PORT_CODE(KEYCODE_N) PORT_NAME("New Game")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_6) PORT_CODE(KEYCODE_6_PAD) PORT_NAME("Verify / King")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_5) PORT_CODE(KEYCODE_5_PAD) PORT_NAME("Options / Queen")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_4) PORT_CODE(KEYCODE_4_PAD) PORT_NAME("Level / Rook")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_3) PORT_CODE(KEYCODE_3_PAD) PORT_NAME("Take Back / Bishop")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_2) PORT_CODE(KEYCODE_2_PAD) PORT_NAME("Hint / Knight")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_1) PORT_CODE(KEYCODE_1_PAD) PORT_NAME("Move / Pawn")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_DEL) PORT_CODE(KEYCODE_BACKSPACE) PORT_NAME("Clear")
INPUT_PORTS_END

// EAG or EAS / Prestige button panel
#define HOUSING(x) PORT_CONDITION("IN.2", 0x01, EQUALS, x)

static INPUT_PORTS_START( eag_base )
	PORT_START("IN.0")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) HOUSING(1) PORT_CODE(KEYCODE_V) PORT_NAME("RV")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) HOUSING(1) PORT_CODE(KEYCODE_O) PORT_NAME("Option")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) HOUSING(1) PORT_CODE(KEYCODE_1) PORT_CODE(KEYCODE_1_PAD) PORT_NAME("LV / Pawn")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) HOUSING(1) PORT_CODE(KEYCODE_2) PORT_CODE(KEYCODE_2_PAD) PORT_NAME("TB / Knight")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) HOUSING(1) PORT_CODE(KEYCODE_3) PORT_CODE(KEYCODE_3_PAD) PORT_NAME("ST / Bishop")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) HOUSING(1) PORT_CODE(KEYCODE_4) PORT_CODE(KEYCODE_4_PAD) PORT_NAME("TM / Rook")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) HOUSING(1) PORT_CODE(KEYCODE_5) PORT_CODE(KEYCODE_5_PAD) PORT_NAME("PV / Queen")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) HOUSING(1) PORT_CODE(KEYCODE_6) PORT_CODE(KEYCODE_6_PAD) PORT_NAME("PB / King")

	PORT_START("IN.1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) HOUSING(1) PORT_CODE(KEYCODE_DEL) PORT_CODE(KEYCODE_BACKSPACE) PORT_NAME("CL") PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(eag_state::in1_changed), 0)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) HOUSING(1) PORT_CODE(KEYCODE_M) PORT_NAME("DM") PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(eag_state::in1_changed), 0)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) HOUSING(1) PORT_CODE(KEYCODE_R) PORT_CODE(KEYCODE_N) PORT_NAME("New Game") PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(eag_state::in1_changed), 0)
INPUT_PORTS_END

static INPUT_PORTS_START( eag )
	PORT_INCLUDE( eag_base )

	PORT_MODIFY("IN.0")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) HOUSING(0) PORT_CODE(KEYCODE_R) PORT_CODE(KEYCODE_N) PORT_NAME("New Game")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) HOUSING(0) PORT_CODE(KEYCODE_O) PORT_NAME("Option")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) HOUSING(0) PORT_CODE(KEYCODE_1) PORT_CODE(KEYCODE_1_PAD) PORT_NAME("PB / King")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) HOUSING(0) PORT_CODE(KEYCODE_2) PORT_CODE(KEYCODE_2_PAD) PORT_NAME("PV / Queen")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) HOUSING(0) PORT_CODE(KEYCODE_3) PORT_CODE(KEYCODE_3_PAD) PORT_NAME("TM / Rook")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) HOUSING(0) PORT_CODE(KEYCODE_4) PORT_CODE(KEYCODE_4_PAD) PORT_NAME("ST / Bishop")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) HOUSING(0) PORT_CODE(KEYCODE_5) PORT_CODE(KEYCODE_5_PAD) PORT_NAME("TB / Knight")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) HOUSING(0) PORT_CODE(KEYCODE_6) PORT_CODE(KEYCODE_6_PAD) PORT_NAME("LV / Pawn")

	PORT_MODIFY("IN.1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) HOUSING(0) PORT_CODE(KEYCODE_M) PORT_NAME("DM") PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(eag_state::in1_changed), 0)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) HOUSING(0) PORT_CODE(KEYCODE_DEL) PORT_CODE(KEYCODE_BACKSPACE) PORT_NAME("CL") PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(eag_state::in1_changed), 0)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) HOUSING(0) PORT_CODE(KEYCODE_V) PORT_NAME("RV") PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(eag_state::in1_changed), 0)

	PORT_START("IN.2") // factory set (diode)
	PORT_CONFNAME( 0x01, 0x01, "Housing" )
	PORT_CONFSETTING(    0x00, "Elite A/S / Prestige" )
	PORT_CONFSETTING(    0x01, "Elite Avant Garde" )
INPUT_PORTS_END

static INPUT_PORTS_START( premiere )
	PORT_INCLUDE( eag_base )

	PORT_START("IN.2") // does not work on Vancouver
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_CUSTOM)

	PORT_START("IN.3")
	PORT_CONFNAME( 0x01, 0x00, "Program" )
	PORT_CONFSETTING(    0x00, "Vancouver" )
	PORT_CONFSETTING(    0x01, "2265" )
INPUT_PORTS_END



/*******************************************************************************
    Machine Configs
*******************************************************************************/

void excel68k_state::fex68k(machine_config &config)
{
	// basic machine hardware
	M68000(config, m_maincpu, 12_MHz_XTAL); // HD68HC000P12
	m_maincpu->set_addrmap(AS_PROGRAM, &excel68k_state::fex68k_map);

	auto &irq_clock(CLOCK(config, "irq_clock", 600)); // 556 timer (22nF, 91K + 20K POT @ 14.8K, 0.1K), ideal is 600Hz (measured 580Hz, 604Hz, 632Hz)
	irq_clock.set_pulse_width(attotime::from_nsec(1525)); // active for 1.525us
	irq_clock.signal_handler().set_inputline(m_maincpu, M68K_IRQ_2);

	SENSORBOARD(config, m_board).set_type(sensorboard_device::BUTTONS);
	m_board->init_cb().set(m_board, FUNC(sensorboard_device::preset_chess));
	m_board->set_delay(attotime::from_msec(200));

	// video hardware
	PWM_DISPLAY(config, m_display).set_size(8, 16);
	m_display->set_segmask(0x55, 0x7f);
	config.set_default_layout(layout_fidel_ex_68k);

	// sound hardware
	SPEAKER(config, "speaker").front_center();
	DAC_1BIT(config, m_dac).add_route(ALL_OUTPUTS, "speaker", 0.25);
}

void excel68k_state::fex68km2(machine_config &config)
{
	fex68k(config);

	// basic machine hardware
	m_maincpu->set_addrmap(AS_PROGRAM, &excel68k_state::fex68km2_map);
}

void excel68k_state::fex68km3(machine_config &config)
{
	fex68k(config);

	// basic machine hardware
	m_maincpu->set_clock(16_MHz_XTAL); // factory overclock
	m_maincpu->set_addrmap(AS_PROGRAM, &excel68k_state::fex68km3_map);
}

void excel68k_state::fex68km4(machine_config &config)
{
	fex68k(config);

	// basic machine hardware
	M68020(config.replace(), m_maincpu, 20_MHz_XTAL); // XC68020RC16 or MC68020RC20E
	m_maincpu->set_addrmap(AS_PROGRAM, &excel68k_state::fex68km4_map);

	subdevice<clock_device>("irq_clock")->set_pulse_width(attotime::from_usec(10)); // irq active for 10us
}

void eag_state::eag_base(machine_config &config)
{
	// basic machine hardware
	M68000(config, m_maincpu, 16_MHz_XTAL);
	m_maincpu->set_interrupt_mixer(false);
	m_maincpu->set_addrmap(AS_PROGRAM, &eag_state::eag_map);

	auto &irq_clock(CLOCK(config, "irq_clock", 4.9152_MHz_XTAL / 0x2000)); // 4060 Q13, 600Hz
	irq_clock.set_pulse_width(attotime::from_nsec(8250)); // active for 8.25us
	irq_clock.signal_handler().set_inputline(m_maincpu, M68K_IRQ_IPL1);

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_1);

	I8251(config, m_usart, 4.9152_MHz_XTAL);
	m_usart->txd_handler().set("rs232", FUNC(rs232_port_device::write_txd));

	auto &usart_clock(CLOCK(config, "usart_clock", 4.9152_MHz_XTAL / 128)); // 4060 Q7, 38.4kHz
	usart_clock.signal_handler().set(m_usart, FUNC(i8251_device::write_txc));

	auto &rs232(RS232_PORT(config, "rs232", default_rs232_devices, nullptr));
	rs232.rxd_handler().set(m_usart, FUNC(i8251_device::write_rxd));
	rs232.cts_handler().set(m_usart, FUNC(i8251_device::write_cts));

	SENSORBOARD(config, m_board).set_type(sensorboard_device::MAGNETS);
	m_board->init_cb().set(m_board, FUNC(sensorboard_device::preset_chess));
	m_board->set_delay(attotime::from_msec(150));

	// video hardware
	PWM_DISPLAY(config, m_display).set_size(9, 16);
	m_display->set_segmask(0x1ef, 0x7f);
	config.set_default_layout(layout_fidel_eag_68k);

	// sound hardware
	SPEAKER(config, "speaker").front_center();
	DAC_1BIT(config, m_dac).add_route(ALL_OUTPUTS, "speaker", 0.25);

	// cartridge
	GENERIC_CARTSLOT(config, "cartslot", generic_plain_slot, "fidel_scc");
	SOFTWARE_LIST(config, "cart_list").set_original("fidel_scc");
}

void eag_state::eagv4(machine_config &config)
{
	eag_base(config);

	// basic machine hardware
	RAM(config, m_ram).set_extra_options("128K, 512K, 1M");
	m_ram->set_default_size("1M");
	m_ram->set_default_value(0);
}

void eagv5_state::eagv5(machine_config &config)
{
	eagv4(config);

	// basic machine hardware
	m_maincpu->set_addrmap(AS_PROGRAM, &eagv5_state::main_map);

	M68000(config, m_subcpu, 16_MHz_XTAL);
	m_subcpu->set_addrmap(AS_PROGRAM, &eagv5_state::sub_map);
	m_subcpu->set_periodic_int(FUNC(eagv5_state::irq2_line_hold), attotime::from_hz(16_MHz_XTAL/0x4000)); // 4060 Q14, ~1kHz

	GENERIC_LATCH_8(config, m_mainlatch);
	GENERIC_LATCH_8(config, m_sublatch);
	m_sublatch->data_pending_callback().set_inputline(m_maincpu, M68K_IRQ_IPL0);

	// gen_latch syncs on write, but this is still needed with tight cpu comms
	// (not that it locks up or anything, but it will calculate moves much slower if timing is off)
	config.set_maximum_quantum(attotime::from_hz(m_maincpu->clock() / 4));

	m_ram->set_default_size("128K");
}

void eag_state::eagv7(machine_config &config)
{
	eagv4(config);

	// basic machine hardware
	M68020(config.replace(), m_maincpu, 20_MHz_XTAL); // MC68020RC20E
	m_maincpu->set_interrupt_mixer(false);
	m_maincpu->set_addrmap(AS_PROGRAM, &eag_state::eagv7_map);

	m_ram->set_extra_options("512K, 1M");
}

void eag_state::eagv9(machine_config &config)
{
	eagv7(config);

	// basic machine hardware
	M68030(config.replace(), m_maincpu, 32_MHz_XTAL); // also seen with 40MHz XTAL
	m_maincpu->set_interrupt_mixer(false);
	m_maincpu->set_addrmap(AS_PROGRAM, &eag_state::eagv7_map);

	m_ram->set_extra_options("1M");
}

void eag_state::eagv10(machine_config &config)
{
	eag_base(config);

	// basic machine hardware
	M68040(config.replace(), m_maincpu, 25_MHz_XTAL);
	m_maincpu->set_interrupt_mixer(false);
	m_maincpu->set_addrmap(AS_PROGRAM, &eag_state::eagv10_map);

	// IRQ pulse should be shorter, guessed here
	subdevice<clock_device>("irq_clock")->set_pulse_width(attotime::from_nsec(1000));
}

void eag_state::eagv11(machine_config &config)
{
	eagv10(config);

	// basic machine hardware
	M68EC040(config.replace(), m_maincpu, 36_MHz_XTAL*2); // wrong! should be M68EC060
	m_maincpu->set_interrupt_mixer(false);
	m_maincpu->set_addrmap(AS_PROGRAM, &eag_state::eagv10_map);
}

void premiere_state::premiere(machine_config &config)
{
	eag_base(config);

	// basic machine hardware
	m_maincpu->set_addrmap(AS_PROGRAM, &premiere_state::main_map);

	// no cartridge
	config.device_remove("cartslot");
	config.device_remove("cart_list");
}



/*******************************************************************************
    ROM Definitions
*******************************************************************************/

ROM_START( fex68k ) // model 6094, PCB label 510.1120B01
	ROM_REGION16_BE( 0x10000, "maincpu", 0 )
	ROM_LOAD16_BYTE("e3_yellow.u6", 0x00000, 0x08000, CRC(a8a27714) SHA1(bc42a561eb39dd389c7831f1a25ad260510085d8) ) // AT27C256-15
	ROM_LOAD16_BYTE("o4_red.u7",    0x00001, 0x08000, CRC(560a14b7) SHA1(11f2375255bfa229314697f103e891ba1cf0c715) ) // "
ROM_END

ROM_START( fex68ka )
	ROM_REGION16_BE( 0x10000, "maincpu", 0 )
	ROM_LOAD16_BYTE("e3_yellow.u6", 0x00000, 0x08000, CRC(7dc60d05) SHA1(e47b4d4e64c4cac6c5a94a900c9f2dd017f849ce) )
	ROM_LOAD16_BYTE("o4_red.u7",    0x00001, 0x08000, CRC(4b738583) SHA1(ff506296ea460c7ed852339d2ab24aaae01730d8) )
ROM_END

ROM_START( fex68kb )
	ROM_REGION16_BE( 0x10000, "maincpu", 0 )
	ROM_LOAD16_BYTE("e3_yellow.u6", 0x00000, 0x08000, CRC(d9f252f5) SHA1(205cdbadb58a4cdd486d4e40d2fe6a5209d2f8a4) )
	ROM_LOAD16_BYTE("o4_red.u7",    0x00001, 0x08000, CRC(3bf8b3d7) SHA1(6ce419c63159501d2349abfd1e142e38e5466fbc) )
ROM_END

ROM_START( fex68km2 ) // model 6097, PCB label 510.1120B01
	ROM_REGION16_BE( 0x10000, "maincpu", 0 )
	ROM_LOAD16_BYTE("e6_yellow.u6", 0x00000, 0x08000, CRC(2e65e7ad) SHA1(4f3aec12041c9014d5d700909bac66bae1f9eadf) ) // 27c256
	ROM_LOAD16_BYTE("o7_red.u7",    0x00001, 0x08000, CRC(4c20334a) SHA1(2e575b88c41505cc89599d2fc13e1e84fe474469) ) // "
ROM_END

ROM_START( fex68km2a ) // model 6097, PCB label 510.1120B01
	ROM_REGION16_BE( 0x10000, "maincpu", 0 )
	ROM_LOAD16_BYTE("e7_yellow.u6", 0x00000, 0x08000, CRC(7c88f53b) SHA1(80fdeed90f1388053110c3bc385bc50c2884b11a) ) // 27c256
	ROM_LOAD16_BYTE("o8_red.u7",    0x00001, 0x08000, CRC(26da3424) SHA1(e0cec467bf1249ce89a27c45f5013859e6581ecd) ) // "
ROM_END

ROM_START( fex68km3 ) // model 6098, PCB label 510.1120B01 - checksum DB53
	ROM_REGION16_BE( 0x10000, "maincpu", 0 )
	ROM_LOAD16_BYTE("me_white.u6",  0x00000, 0x08000, CRC(4b14cd9f) SHA1(4d41196900a71bf0699dae50f4726acc0ed3dced) ) // 27c256
	ROM_LOAD16_BYTE("mo_yellow.u7", 0x00001, 0x08000, CRC(b96b0b5f) SHA1(281145be802efb38ed764aecb26b511dcd71cb87) ) // "
ROM_END

ROM_START( fex68km3a ) // model 6098, PCB label 510.1120B01 - checksum F806
	ROM_REGION16_BE( 0x10000, "maincpu", 0 )
	ROM_LOAD16_BYTE("me_white.u6",  0x00000, 0x08000, CRC(434b68ec) SHA1(eaf6d2b1db3c0145adfd9d2c5d5d5ec65524b5de) ) // 27c256
	ROM_LOAD16_BYTE("mo_yellow.u7", 0x00001, 0x08000, CRC(5e78ee99) SHA1(70e5e47b6daa35dc4d5675d4d1c6a4093fb47ea6) ) // "
ROM_END

ROM_START( fex68km3b ) // model 6098, PCB label 510.1120B01 - checksum E2DA
	ROM_REGION16_BE( 0x10000, "maincpu", 0 )
	ROM_LOAD16_BYTE("me_white.u6",  0x00000, 0x08000, CRC(39e17293) SHA1(61d557f96ae2bb164108f53d7d48239f8f2a7f6b) ) // 27c256
	ROM_LOAD16_BYTE("mo_yellow.u7", 0x00001, 0x08000, CRC(572b208a) SHA1(cd50982d8fa5bde9ac83472fddd14d56fa6ead3b) ) // "
ROM_END

ROM_START( fex68km4 ) // model 6110, PCB label 510.1120B01 - checksum FD96
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD16_BYTE("68020_even_master_2325.u6", 0x00000, 0x08000, CRC(13ea816c) SHA1(98d00fc382ddcbccb0a47c3f8d7fc73f30a15fbd) )
	ROM_LOAD16_BYTE("68020_odd_master_2325.u7",  0x00001, 0x08000, CRC(d24c7b54) SHA1(3204fd600786792a618965715990c44890cc7119) )
ROM_END


ROM_START( feagv4 ) // dumped from a V3 - checksum F66D 4B3E
	ROM_REGION16_BE( 0x20000, "maincpu", 0 )
	ROM_LOAD16_BYTE("elite_1.6_e.u22", 0x00000, 0x10000, CRC(c8b89ccc) SHA1(d62e0a72f54b793ab8853468a81255b62f874658) )
	ROM_LOAD16_BYTE("elite_1.6_o.u19", 0x00001, 0x10000, CRC(904c7061) SHA1(742110576cf673321440bc81a4dae4c949b49e38) )
ROM_END

ROM_START( feagv4a ) // dumped from a V2 - checksum FD5C 49AC
	ROM_REGION16_BE( 0x20000, "maincpu", 0 )
	ROM_LOAD16_BYTE("6114_e5_yellow.u22", 0x00000, 0x10000, CRC(f9c7bada) SHA1(60e545f829121b9a4f1100d9e85ac83797715e80) ) // 27c512
	ROM_LOAD16_BYTE("6114_o5_green.u19",  0x00001, 0x10000, CRC(04f97b22) SHA1(8b2845dd115498f7b385e8948eca6a5893c223d1) ) // "
ROM_END


ROM_START( feagv5 )
	ROM_REGION16_BE( 0x20000, "maincpu", 0 ) // PCB label 510.1136A01 - checksum 0140 9CF2
	ROM_LOAD16_BYTE("master_e", 0x00000, 0x10000, CRC(e424bddc) SHA1(ff03656addfe5c47f06df2efb4602f43a9e19d96) )
	ROM_LOAD16_BYTE("master_o", 0x00001, 0x10000, CRC(33a00894) SHA1(849460332b1ac10d452ca3631eb99f5597511b73) )

	ROM_REGION16_BE( 0x10000, "subcpu", 0 ) // PCB label 510.1138B01
	ROM_LOAD16_BYTE("slave_e", 0x00000, 0x08000, CRC(eea4de52) SHA1(a64ca8a44b431e2fa7f00e44cab7e6aa2d4a9403) )
	ROM_LOAD16_BYTE("slave_o", 0x00001, 0x08000, CRC(35fe2fdf) SHA1(731da12ee290bad9bc03cffe281c8cc48e555dfb) )
ROM_END


ROM_START( feagv7 ) // also seen on a V9 - checksum F702 6863
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD16_BYTE("6117_e3_yellow.u22", 0x00000, 0x10000, CRC(60523199) SHA1(a308eb6b782732af1ab2fd0ed8b046de7a8dd24b) )
	ROM_LOAD16_BYTE("6117_o3_red.u19",    0x00001, 0x10000, CRC(44fbb3b0) SHA1(8bf5c7ac5801f5a656ae710c1a61b693f5314b8c) )
ROM_END

ROM_START( feagv7a ) // dumped from a repro pcb - checksum FCA0 6969
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD16_BYTE("eag-v7b", 0x00000, 0x10000, CRC(f2f68b63) SHA1(621e5073e9c5083ac9a9b467f3ef8aa29beac5ac) )
	ROM_LOAD16_BYTE("eag-v7a", 0x00001, 0x10000, CRC(506b688f) SHA1(0a091c35d0f01166b57f964b111cde51c5720d58) )
ROM_END

ROM_START( feagv7b ) // PCB label 510.1136A01, dumped from a V6 - checksum 005D 6AB7
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD16_BYTE("e1_yellow.u22", 0x00000, 0x10000, CRC(2fa692a9) SHA1(357fd47e97f823462e372c7b4d0730c1fa35c364) )
	ROM_LOAD16_BYTE("o1_red.u19",    0x00001, 0x10000, CRC(bceb99f0) SHA1(601869be5fb9724fe75f14d4dac58471eed6e0f4) )
ROM_END

ROM_START( feagv7c ) // checksum 00D5 6939
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD16_BYTE("e1_yellow.u22", 0x00000, 0x10000, CRC(44baefbf) SHA1(dbc24340d7e3013cc8f111ebb2a59169c5dcb8e8) )
	ROM_LOAD16_BYTE("o1_red.u19",    0x00001, 0x10000, CRC(951a7857) SHA1(dad21b049fd4f411a79d4faefb922c1277569c0e) )
ROM_END

// feagv9 has same ROMs as feagv7
#define rom_feagv9 rom_feagv7
#define rom_feagv9a rom_feagv7a
#define rom_feagv9b rom_feagv7b
#define rom_feagv9c rom_feagv7c


ROM_START( feagv10 ) // checksum F40C 38B9
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD32_BYTE("16", 0x00000, 0x08000, CRC(8375d61f) SHA1(e042f6f01480c59ee09a458cf34f135664479824) ) // 27c256
	ROM_LOAD32_BYTE("17", 0x00001, 0x08000, CRC(bfd14916) SHA1(115af6dfd29ddd8ad6d2ce390f8ecc4d60de6fce) ) // "
	ROM_LOAD32_BYTE("18", 0x00002, 0x08000, CRC(9341dcaf) SHA1(686bd4799e89ffaf11a813d4cf5a2aedd4c2d97a) ) // "
	ROM_LOAD32_BYTE("19", 0x00003, 0x08000, CRC(a70c5468) SHA1(7f6b4f46577d5cfdaa84d387c7ce35d941e5bbc7) ) // "
ROM_END

ROM_START( feagv11 ) // checksum F40C 38B9
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD32_BYTE("16", 0x00000, 0x08000, CRC(8375d61f) SHA1(e042f6f01480c59ee09a458cf34f135664479824) ) // 27c256
	ROM_LOAD32_BYTE("17", 0x00001, 0x08000, CRC(bfd14916) SHA1(115af6dfd29ddd8ad6d2ce390f8ecc4d60de6fce) ) // "
	ROM_LOAD32_BYTE("18", 0x00002, 0x08000, CRC(9341dcaf) SHA1(686bd4799e89ffaf11a813d4cf5a2aedd4c2d97a) ) // "
	ROM_LOAD32_BYTE("19", 0x00003, 0x08000, CRC(a70c5468) SHA1(7f6b4f46577d5cfdaa84d387c7ce35d941e5bbc7) ) // "
ROM_END


ROM_START( premiere ) // model 6131, PCB label 510.1157A01 - checksum (2265 only) F667 4B06
	ROM_REGION16_BE( 0x40000, "maincpu", 0 )
	ROM_LOAD16_BYTE("101.1103a01_1meg_even.u22", 0x00000, 0x20000, CRC(0df2d4d8) SHA1(2c6cd8d83768d14aeb9860be76ed2ec0f64f118b) ) // M27C1001
	ROM_LOAD16_BYTE("101.1104a01_1meg_odd.u19",  0x00001, 0x20000, CRC(afae9d5e) SHA1(7ab5fb8b8a2fa30f2fd444a050eae2432c9236d0) ) // "
ROM_END

} // anonymous namespace



/*******************************************************************************
    Drivers
*******************************************************************************/

//    YEAR  NAME       PARENT   COMPAT  MACHINE   INPUT     CLASS           INIT        COMPANY, FULLNAME, FLAGS
SYST( 1987, fex68k,    0,       0,      fex68k,   excel68k, excel68k_state, empty_init, "Fidelity International", "Excel 68000 (set 1)", MACHINE_SUPPORTS_SAVE )
SYST( 1987, fex68ka,   fex68k,  0,      fex68k,   excel68k, excel68k_state, empty_init, "Fidelity International", "Excel 68000 (set 2)", MACHINE_SUPPORTS_SAVE )
SYST( 1987, fex68kb,   fex68k,  0,      fex68k,   excel68k, excel68k_state, empty_init, "Fidelity International", "Excel 68000 (set 3)", MACHINE_SUPPORTS_SAVE )
SYST( 1988, fex68km2,  fex68k,  0,      fex68km2, excel68k, excel68k_state, empty_init, "Fidelity International", "Excel 68000 Mach II (rev. C+, set 1)", MACHINE_SUPPORTS_SAVE )
SYST( 1988, fex68km2a, fex68k,  0,      fex68km2, excel68k, excel68k_state, empty_init, "Fidelity International", "Excel 68000 Mach II (rev. C+, set 2)", MACHINE_SUPPORTS_SAVE )
SYST( 1988, fex68km3,  fex68k,  0,      fex68km3, excel68k, excel68k_state, empty_init, "Fidelity International", "Excel 68000 Mach III Master 2265 (set 1)", MACHINE_SUPPORTS_SAVE )
SYST( 1988, fex68km3a, fex68k,  0,      fex68km3, excel68k, excel68k_state, empty_init, "Fidelity International", "Excel 68000 Mach III Master 2265 (set 2)", MACHINE_SUPPORTS_SAVE )
SYST( 1988, fex68km3b, fex68k,  0,      fex68km3, excel68k, excel68k_state, empty_init, "Fidelity International", "Excel 68000 Mach III Master 2265 (set 3)", MACHINE_SUPPORTS_SAVE )
SYST( 1989, fex68km4,  fex68k,  0,      fex68km4, excel68k, excel68k_state, empty_init, "Fidelity International", "Excel 68000 Mach IV 68020 Master 2325", MACHINE_SUPPORTS_SAVE )

SYST( 1989, feagv4,    0,       0,      eagv4,    eag,      eag_state,      init_eag,   "Fidelity International", "Elite Avant Garde 2265 (model 6114-2/3/4, set 1)", MACHINE_SUPPORTS_SAVE )
SYST( 1989, feagv4a,   feagv4,  0,      eagv4,    eag,      eag_state,      init_eag,   "Fidelity International", "Elite Avant Garde 2265 (model 6114-2/3/4, set 2)", MACHINE_SUPPORTS_SAVE )
SYST( 1989, feagv5,    feagv4,  0,      eagv5,    eag,      eagv5_state,    init_eag,   "Fidelity International", "Elite Avant Garde 2265 (model 6114-5)", MACHINE_SUPPORTS_SAVE )

SYST( 1989, feagv7,    feagv4,  0,      eagv7,    eag,      eag_state,      init_eag,   "Fidelity International", "Elite Avant Garde 2325 (model 6117-6/7, set 1)", MACHINE_SUPPORTS_SAVE )
SYST( 1989, feagv7a,   feagv4,  0,      eagv7,    eag,      eag_state,      init_eag,   "Fidelity International", "Elite Avant Garde 2325 (model 6117-6/7, set 2)", MACHINE_SUPPORTS_SAVE )
SYST( 1989, feagv7b,   feagv4,  0,      eagv7,    eag,      eag_state,      init_eag,   "Fidelity International", "Elite Avant Garde 2325 (model 6117-6/7, set 3)", MACHINE_SUPPORTS_SAVE )
SYST( 1989, feagv7c,   feagv4,  0,      eagv7,    eag,      eag_state,      init_eag,   "Fidelity International", "Elite Avant Garde 2325 (model 6117-6/7, set 4)", MACHINE_SUPPORTS_SAVE )

SYST( 1989, feagv9,    feagv4,  0,      eagv9,    eag,      eag_state,      init_eag,   "Fidelity International", "Elite Avant Garde 2325 (model 6117-9, set 1)", MACHINE_SUPPORTS_SAVE )
SYST( 1989, feagv9a,   feagv4,  0,      eagv9,    eag,      eag_state,      init_eag,   "Fidelity International", "Elite Avant Garde 2325 (model 6117-9, set 2)", MACHINE_SUPPORTS_SAVE )
SYST( 1989, feagv9b,   feagv4,  0,      eagv9,    eag,      eag_state,      init_eag,   "Fidelity International", "Elite Avant Garde 2325 (model 6117-9, set 3)", MACHINE_SUPPORTS_SAVE )
SYST( 1989, feagv9c,   feagv4,  0,      eagv9,    eag,      eag_state,      init_eag,   "Fidelity International", "Elite Avant Garde 2325 (model 6117-9, set 4)", MACHINE_SUPPORTS_SAVE )

SYST( 1990, feagv10,   feagv4,  0,      eagv10,   eag,      eag_state,      empty_init, "Fidelity International", "Elite Avant Garde 2325 (model 6117-10)", MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_TIMING )
SYST( 2001, feagv11,   feagv4,  0,      eagv11,   eag,      eag_state,      empty_init, "hack (Wilfried Bucke)", "Elite Avant Garde 2325 (model 6117-11)", MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_TIMING )

SYST( 1992, premiere,  0,       0,      premiere, premiere, premiere_state, empty_init, "Fidelity Electronics International", "Elite Premiere", MACHINE_SUPPORTS_SAVE )
