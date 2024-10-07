// license:BSD-3-Clause
// copyright-holders:Robbbert
/*******************************************************************************

PINBALL
LTD (Brazil)

Not much info available for these machines. There's a homebrew partial schematic
available with some System 3 pages, and some System 4 pages. There's also a
foreign-language manual, but no schematic.

Used PinMAME as a reference.

The manual mentions these machines:
Arizona, Atlantis, Galaxia, Hustler, Martian Queen.

PinMAME has a large list of games, these are:
1977: O Gaucho, Samba
1978: Grand Prix
1981: Al Capone, Amazon, Arizona, Atlantis, Black Hole, Carnaval no Rio,
      Cowboy Eight Ball, Disco Dancing, Force, Galaxia, Haunted Hotel,
      Hustler, King Kong, Kung Fu, Mr. & Mrs. Pec-Men, Martian Queen,
      Space Poker, Time Machine, Zephy
1982: Alien Warrior, Columbia, Cowboy 2, Trick Shooter
(unknown year): Viking King

Status:
- All games can accept coins and start up
- System 3 2-players games are playable.
- hhouse, press 2 to start

System 3:
- Games are 2 players, except zephy and cowboy which have 3.
- All 2-player games are playable. 3-player games inputs not responding.
- May need to hit X when the first ball starts, to enable scoring.

ToDo:
- No mechanical sounds (no info available)
- System 3, get the correct frequencies for the beep sound
- System 3, "force","bhol_ltd","spcpoker" have no sound
- System 3, slam tilt to connect to reset line
- System 3, how to enter setup menu?
- System 3, 3-player games, no playfield inputs (could be an issue of missing balls)
- System 4, no playfield inputs (could be an issue of missing balls)

********************************************************************************/

#include "emu.h"
#include "genpin.h"

#include "cpu/m6800/m6801.h"
#include "machine/74123.h"
#include "machine/clock.h"
#include "machine/input_merger.h"
#include "machine/rescap.h"
#include "sound/ay8910.h"
#include "sound/spkrdev.h"
#include "speaker.h"

#include "ltd.lh"

namespace {

class ltd_state : public genpin_class
{
public:
	ltd_state(const machine_config &mconfig, device_type type, const char *tag)
		: genpin_class(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_io_keyboard(*this, "X%d", 0U)
		, m_digits(*this, "digit%d", 0U)
		, m_io_outputs(*this, "out%d", 0U)
	{ }

	DECLARE_INPUT_CHANGED_MEMBER(ficha);

protected:

	u8 m_digit = 0U;
	void mr_common();
	void ms_common();
	required_device<cpu_device> m_maincpu;
	required_ioport_array<8> m_io_keyboard;
	output_finder<60> m_digits;
	output_finder<72> m_io_outputs;   // 56 lamps + 8 solenoids + 8 mystery
};

class ltd3_state : public ltd_state
{
public:
	ltd3_state(const machine_config &mconfig, device_type type, const char *tag)
		: ltd_state(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_irq_pulse(*this, "irq_pulse")
		, m_snd_pulse(*this, "snd_pulse")
		, m_monotone(*this, "monotone")
	{ }

	void ltd3(machine_config &config);

	void init_0() { m_game = 0; }
	void init_1() { m_game = 1; }
	void init_2() { m_game = 2; }
	void init_3() { m_game = 3; }

private:
	virtual void machine_reset() override ATTR_COLD;
	virtual void machine_start() override ATTR_COLD;
	u8 ram_r(offs_t);
	void ram_w(offs_t, u8);
	u8 sw_r(offs_t offset);
	void irq_w(int state);
	void ltd3_map(address_map &map) ATTR_COLD;
	u8 m_game = 0;
	u8 m_ram[0x80]{};
	u8 m_segment = 0;
	required_device<m6802_cpu_device> m_maincpu;
	optional_device<ttl74123_device> m_irq_pulse;
	optional_device<ttl74123_device> m_snd_pulse;
	optional_device<clock_device> m_monotone;
};

class ltd4_state : public ltd_state
{
public:
	ltd4_state(const machine_config &mconfig, device_type type, const char *tag)
		: ltd_state(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
	{ }

	void ltd4(machine_config &config);

private:
	virtual void machine_reset() override ATTR_COLD;
	virtual void machine_start() override ATTR_COLD;
	void ltd4_map(address_map &map) ATTR_COLD;
	u8 port1_r();
	void port1_w(u8 data);
	u8 port2_r();
	void port2_w(u8 data);
	void ay0a_w(u8);
	u8 m_port2 = 0;
	u8 m_counter = 0;
	required_device<m6803_cpu_device> m_maincpu;
};

void ltd3_state::ltd3_map(address_map &map)
{
	map(0x0000, 0x007f).rw(FUNC(ltd3_state::ram_r),FUNC(ltd3_state::ram_w)); // internal to the cpu
	map(0x0080, 0x0087).mirror(0x78).r(FUNC(ltd3_state::sw_r));
	map(0x0800, 0x0bff).w("ay0", FUNC(ay8910_device::data_w));
	map(0x0c00, 0x0fff).w("ay0", FUNC(ay8910_device::address_w));
	map(0x1800, 0x1bff).w("ay1", FUNC(ay8910_device::data_w));
	map(0x1c00, 0x1fff).w("ay1", FUNC(ay8910_device::address_w));
	map(0x2800, 0x2bff).w("ay2", FUNC(ay8910_device::data_w));
	map(0x2c00, 0x2fff).w("ay2", FUNC(ay8910_device::address_w));
	map(0xb000, 0xb000).w("ay2", FUNC(ay8910_device::reset_w)); // this might reset all 3
	map(0xc000, 0xcfff).rom().mirror(0x3000).region("roms", 0);
}

void ltd4_state::ltd4_map(address_map &map)
{
	map(0x0100, 0x01ff).ram().share("nvram");
	map(0x0800, 0x0bff).w("ay0", FUNC(ay8910_device::address_w));
	map(0x0c00, 0x0fff).w("ay0", FUNC(ay8910_device::reset_w));
	map(0x2800, 0x2bff).w("ay0", FUNC(ay8910_device::data_w));
	map(0x1000, 0x13ff).w("ay1", FUNC(ay8910_device::address_w));
	map(0x1400, 0x17ff).w("ay1", FUNC(ay8910_device::reset_w));
	map(0x3000, 0x33ff).w("ay1", FUNC(ay8910_device::data_w));
	map(0x1800, 0x1bff).w("ay2", FUNC(ay8910_device::address_w));
	map(0x1c00, 0x1fff).w("ay2", FUNC(ay8910_device::reset_w));
	map(0x3800, 0x3bff).w("ay2", FUNC(ay8910_device::data_w));
	map(0xc000, 0xdfff).rom().mirror(0x2000).region("roms", 0);
}

// bits 6,7 not connected to data bus
static INPUT_PORTS_START( ltd3 )
	PORT_START("FICHA")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, ltd_state, ficha, 0)

	PORT_START("X0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_9) PORT_NAME("Tilt")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_A) PORT_NAME("INP02")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_B) PORT_NAME("INP03")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_C) PORT_NAME("INP04")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_D) PORT_NAME("INP05")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_E) PORT_NAME("INP06")
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("X1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_F) PORT_NAME("INP07")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_G) PORT_NAME("INP08")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_H) PORT_NAME("INP09")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_I) PORT_NAME("INP10")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_J) PORT_NAME("INP11")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_K) PORT_NAME("INP12")
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("X2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_L) PORT_NAME("INP13")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_M) PORT_NAME("INP14")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_N) PORT_NAME("INP15")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_O) PORT_NAME("INP16")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_P) PORT_NAME("INP17")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_Q) PORT_NAME("INP18")
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("X3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_R) PORT_NAME("INP19")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_S) PORT_NAME("INP20")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_T) PORT_NAME("INP21")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_U) PORT_NAME("INP22")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_V) PORT_NAME("INP23")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START )
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("X4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_W) PORT_NAME("INP25")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_Y) PORT_NAME("INP26")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_Z) PORT_NAME("INP27")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_COMMA) PORT_NAME("INP28")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_STOP) PORT_NAME("INP29")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_SLASH) PORT_NAME("INP30")
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("X5")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_COLON) PORT_NAME("INP31")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_QUOTE) PORT_NAME("INP32")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_ENTER) PORT_NAME("INP33")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_SPACE) PORT_NAME("INP34")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_LSHIFT) PORT_NAME("INP35")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_RSHIFT) PORT_NAME("INP36")
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("X6")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_OPENBRACE) PORT_NAME("INP37")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_CLOSEBRACE) PORT_NAME("INP38")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_BACKSLASH) PORT_NAME("INP39")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_MINUS) PORT_NAME("INP40")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_EQUALS) PORT_NAME("INP41")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_BACKSPACE) PORT_NAME("INP42")
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("X7")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_X) PORT_NAME("Outhole")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_DOWN) PORT_NAME("INP44")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_LEFT) PORT_NAME("INP45")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_RIGHT) PORT_NAME("INP46")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_UP) PORT_NAME("INP47")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_END) PORT_NAME("INP48")
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END

// this needs to be redone once inputs start to work
static INPUT_PORTS_START( ltd4 )
	PORT_START("X0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_A) PORT_NAME("INP01")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_B) PORT_NAME("INP02")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_C) PORT_NAME("INP03")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_D) PORT_NAME("INP04")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_E) PORT_NAME("INP05")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_9) PORT_NAME("Tilt")
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("X1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_0_PAD) PORT_NAME("Setup")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_1_PAD) PORT_NAME("Advance")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_F) PORT_NAME("INP09")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_G) PORT_NAME("INP10")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START2 ) // Haunted House only
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("X2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_H) PORT_NAME("INP14")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_I) PORT_NAME("INP15")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_J) PORT_NAME("INP16")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_K) PORT_NAME("INP17")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_L) PORT_NAME("INP18")
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("X3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_M) PORT_NAME("INP19")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_N) PORT_NAME("INP20")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_O) PORT_NAME("INP21")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_P) PORT_NAME("INP22")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_Q) PORT_NAME("INP23")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_R) PORT_NAME("INP24")
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("X4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_S) PORT_NAME("INP25")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_T) PORT_NAME("INP26")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_U) PORT_NAME("INP27")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_V) PORT_NAME("INP28")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_W) PORT_NAME("INP29")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_Y) PORT_NAME("INP30")
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("X5")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_Z) PORT_NAME("INP31")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_COMMA) PORT_NAME("INP32")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_STOP) PORT_NAME("INP33")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_SLASH) PORT_NAME("INP34")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_COLON) PORT_NAME("INP35")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_QUOTE) PORT_NAME("INP36")
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("X6")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_ENTER) PORT_NAME("INP37")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_OPENBRACE) PORT_NAME("INP38")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_CLOSEBRACE) PORT_NAME("INP39")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_BACKSLASH) PORT_NAME("INP40")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_MINUS) PORT_NAME("INP41")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_EQUALS) PORT_NAME("INP42")
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("X7")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_X) PORT_NAME("Outhole") // not confirmed
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_BACKSPACE) PORT_NAME("INP44")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_LEFT) PORT_NAME("INP45")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_RIGHT) PORT_NAME("INP46")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_UP) PORT_NAME("INP47")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_DOWN) PORT_NAME("INP48")
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END

INPUT_CHANGED_MEMBER( ltd_state::ficha )
{
	if(newval)
		m_maincpu->pulse_input_line(INPUT_LINE_NMI, attotime::zero);
}

u8 ltd3_state::ram_r(offs_t offset)
{
	return m_ram[offset];
}

void ltd3_state::ram_w(offs_t offset, u8 data)
{
	static const u8 patterns[16] = { 0x3f, 0x06, 0x5b, 0x4f, 0x66, 0x6d, 0x7c, 0x07, 0x7f, 0x67, 0x58, 0x4c, 0x62, 0x69, 0x78, 0 }; // 7447
	u8 laydigits[4][32]{
		{ 40, 48,  1, 11,  2, 12,  3, 13,  4, 14, 48, 48, 48, 48, 48, 48, 48, 48, 41, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48 },   // older than force
		{ 40, 48,  1, 11,  2, 12,  3, 13,  4, 14,  0, 10, 48, 48, 48, 48, 48, 48, 41, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48 },   // force
		{ 40, 48,  0, 10,  1, 11,  2, 12,  3, 13,  4, 14,  5, 15, 48, 48, 48, 48, 41, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48 },   // black hole
		{ 13, 41,  0, 14,  1, 15,  2, 20,  3, 21,  4, 22,  5, 23, 10, 24, 11, 25, 12, 40, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48 }};  // zephy
		// 48 = do not display

	m_ram[offset] = data;
	if (offset >= 0x70)
	{
		offset &= 15;
		switch (offset)
		{
			// Lamps
			case 0:
			case 1:
			case 2:
			case 3:
			case 4:
			case 5:
			case 6:
			// Solenoids
			case 7:
				{
					for (u8 i = 0; i < 8; i++)
						m_io_outputs[offset*8+i] = BIT(data, i);
				}
				break;
			case 8:
				// Display segment
				m_segment = data;
				break;
			case 9:
				// Beep Sound
				m_snd_pulse->b_w(BIT(data, 4));
				if (BIT(data, 4))
				{
					u16 freq = 100 + ((data>>5) * 300);  // guess
					m_monotone->set_unscaled_clock(freq);
				}
				// Display digits
				{
					data &= 15;
					u8 dispdig = laydigits[m_game][data*2];
					m_digits[dispdig] = patterns[BIT(m_segment, 0, 4)];
					dispdig = laydigits[m_game][data*2+1];
					m_digits[dispdig] = patterns[BIT(m_segment, 4, 4)];
				}
				break;
			default:
				break;
		}
	}
}

// switches
u8 ltd3_state::sw_r(offs_t offset)
{
	return m_io_keyboard[offset]->read();
}

u8 ltd4_state:: port1_r()
{
	u8 row = m_digit >> 4;
	if (row < 8 && !BIT(m_port2, 4))
		return m_io_keyboard[row]->read();
	else
		return 0xff;
}

void ltd4_state::port1_w(u8 data)
{
	if (m_port2 & 0x10)
	{
		u8 row = m_digit & 15;
		u8 segment;
		if (row > 13)
		{
			//if (m_counter == 1) printf("1%d=%02X ",row,data);
			//if (m_counter == 2) printf("2%d=%02X ",row,data);
		}

		switch (m_counter)
		{
			case 11:
				m_digit = data;
				break;
			case 1:
				segment = bitswap<8>(data, 7, 0, 1, 2, 3, 4, 5, 6);
				if (row>7)
					m_digits[row+2] = segment; // P2
				else
					m_digits[row] = segment; // P1
				break;
			case 2:
				segment = bitswap<8>(data, 7, 0, 1, 2, 3, 4, 5, 6);
				if (row>13)
					m_digits[row+26] = segment; // credits / ball
				else
				if (row>7)
					m_digits[row+22] = segment; // P4
				else
					m_digits[row+20] = segment; // P3
				break;
			// Lamps and Solenoids
			case 3:
			case 4:
			case 5:
			case 6:
			case 7:
			case 8:
			case 9:
			case 10:
				{
					for (u8 i = 0; i < 8; i++)
						m_io_outputs[(m_counter-3)*8+i] = BIT(data, i);
				}
				break;
		}
	}
}

u8 ltd4_state:: port2_r()
{
	return m_port2;
}

void ltd4_state::port2_w(u8 data)
{
	if (((m_port2 & 0x10) == 0) && ((data & 0x10) == 0x10))
		m_counter++;

	if (data == 14)
		m_counter = 0;

	m_port2 = data;
}

// Unknown outputs used by pecmen, alcapone, cowboy2
// Added to lamp outputs for now
void ltd4_state::ay0a_w(u8 data)
{
	for (u8 i = 0; i < 8; i++)
		m_io_outputs[64+i] = BIT(data, i);
}

void ltd_state::ms_common()
{
	genpin_class::machine_start();

	m_digits.resolve();
	m_io_outputs.resolve();
	save_item(NAME(m_digit));
}

void ltd3_state::machine_start()
{
	ms_common();
	save_item(NAME(m_game));
	save_item(NAME(m_ram));
	save_item(NAME(m_segment));
}

void ltd4_state::machine_start()
{
	ms_common();
	save_item(NAME(m_counter));
	save_item(NAME(m_port2));
}

void ltd_state::mr_common()
{
	genpin_class::machine_reset();
	for (u8 i = 0; i < m_io_outputs.size(); i++)
		m_io_outputs[i] = 0;
}

void ltd3_state::machine_reset()
{
	mr_common();
	if (m_game == 0)
	{
		m_digits[0] = 0x3f;
		m_digits[10] = 0x3f;
	}
}

void ltd4_state::machine_reset()
{
	mr_common();
}

void ltd3_state::irq_w(int state)
{
	// Using /Q output
	m_maincpu->set_input_line(M6802_IRQ_LINE, !state ? CLEAR_LINE : ASSERT_LINE);
}

void ltd3_state::ltd3(machine_config &config)
{
	/* basic machine hardware */
	m6802_cpu_device &maincpu(M6802(config, m_maincpu, XTAL(3'579'545)));
	maincpu.set_ram_enable(false);
	maincpu.set_addrmap(AS_PROGRAM, &ltd3_state::ltd3_map);

	/* Video */
	config.set_default_layout(layout_ltd);

	// homebrew schematic says 1uF, but the pulse doesn't finish before
	// the next clock input pulse arrives. 1nF works nicely.
	TTL74123(config, m_irq_pulse, RES_K(30), CAP_N(1));  // U116
	m_irq_pulse->set_connection_type(TTL74123_GROUNDED);
	m_irq_pulse->set_a_pin_value(0);
	m_irq_pulse->set_b_pin_value(1);  // nc
	m_irq_pulse->set_clear_pin_value(1);  // nc
	m_irq_pulse->out_cb().set(FUNC(ltd3_state::irq_w));

	/* Sound */
	genpin_audio(config);
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, "speaker").add_route(ALL_OUTPUTS, "mono", 0.50);
	CLOCK(config, m_monotone, 0);
	m_monotone->signal_handler().set("snd", FUNC(input_merger_device::in_w<0>));
	AY8910(config, "ay0", XTAL(3'579'545)/2).add_route(ALL_OUTPUTS, "mono", 0.75); /* guess */
	AY8910(config, "ay1", XTAL(3'579'545)/2).add_route(ALL_OUTPUTS, "mono", 0.75); /* guess */
	AY8910(config, "ay2", XTAL(3'579'545)/2).add_route(ALL_OUTPUTS, "mono", 0.75); /* guess */

	TTL74123(config, m_snd_pulse, RES_K(22), CAP_U(10));
	m_snd_pulse->set_connection_type(TTL74123_GROUNDED);
	m_snd_pulse->set_a_pin_value(0);
	m_snd_pulse->set_b_pin_value(1);
	m_snd_pulse->set_clear_pin_value(1);  // nc
	m_snd_pulse->out_cb().set("snd", FUNC(input_merger_device::in_w<1>));

	clock_device &irq_clock(CLOCK(config, "irq_clock", 1765));  // 680 ohm, 1uF, U117
	irq_clock.signal_handler().set(m_irq_pulse, FUNC(ttl74123_device::a_w));

	INPUT_MERGER_ALL_HIGH(config, "snd").output_handler().set("speaker", FUNC(speaker_sound_device::level_w));
}

void ltd4_state::ltd4(machine_config &config)
{
	/* basic machine hardware */
	m6803_cpu_device &maincpu(M6803(config, m_maincpu, XTAL(3'579'545))); // guess, no details available
	maincpu.set_addrmap(AS_PROGRAM, &ltd4_state::ltd4_map);
	maincpu.in_p1_cb().set(FUNC(ltd4_state::port1_r));
	maincpu.out_p1_cb().set(FUNC(ltd4_state::port1_w));
	maincpu.in_p2_cb().set(FUNC(ltd4_state::port2_r));
	maincpu.out_p2_cb().set(FUNC(ltd4_state::port2_w));

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	/* Video */
	config.set_default_layout(layout_ltd);

	/* Sound */
	genpin_audio(config);

	SPEAKER(config, "mono").front_center();
	ay8910_device &ay0(AY8910(config, "ay0", XTAL(3'579'545)/2));
	ay0.add_route(ALL_OUTPUTS, "mono", 0.75); /* guess */
	ay0.port_a_write_callback().set(FUNC(ltd4_state::ay0a_w));
	AY8910(config, "ay1", XTAL(3'579'545)/2).add_route(ALL_OUTPUTS, "mono", 0.75); /* guess */
	AY8910(config, "ay2", XTAL(3'579'545)/2).add_route(ALL_OUTPUTS, "mono", 0.75); /* guess */
}

/*-------------------------------------------------------------------
/ Arizona (backglass has the word "Territory" under "Arizona". Is it
/          part of the title?)
/-------------------------------------------------------------------*/
ROM_START(arizona)
	ROM_REGION(0x1000, "roms", 0)
	ROM_LOAD("arizltd.bin", 0x0000, 0x0400, CRC(908f00d8) SHA1(98f28f1aedbad43e0e096959fdef45e038405473))
	ROM_RELOAD(0x0400, 0x0400)
	ROM_RELOAD(0x0800, 0x0400)
	ROM_RELOAD(0x0c00, 0x0400)
ROM_END

/*-------------------------------------------------------------------
/ Atlantis
/-------------------------------------------------------------------*/
ROM_START(atla_ltd)
	ROM_REGION(0x1000, "roms", 0)
	ROM_LOAD("atlantis.bin", 0x0000, 0x0800, CRC(c61be043) SHA1(e6c4463f59a5743fa34aa55beeb6f536ad9f1b56))
	ROM_RELOAD(0x0800, 0x0800)
ROM_END

/*-------------------------------------------------------------------
/ Disco Dancing
/-------------------------------------------------------------------*/
ROM_START(discodan)
	ROM_REGION(0x1000, "roms", 0)
	ROM_LOAD("disco.bin", 0x0000, 0x0800, CRC(83c79157) SHA1(286fd0c984870639fcd7d7b8f6a5a5ddabcddcf5))
	ROM_RELOAD(0x0800, 0x0800)
ROM_END

/*-------------------------------------------------------------------
/ Hustler
/-------------------------------------------------------------------*/
ROM_START(hustlerp)
	ROM_REGION(0x1000, "roms", 0)
	ROM_LOAD("hustler_1.bin", 0x0000, 0x0800, CRC(43f323f5) SHA1(086b81699bea08b10b4231e398f4f689395355b0))
	ROM_RELOAD(0x0800, 0x0800)
ROM_END

/*-------------------------------------------------------------------
/ Martian Queen
/-------------------------------------------------------------------*/

ROM_START(marqueen)
	ROM_REGION(0x1000, "roms", 0) // the bad dump was hacked to make it work
	ROM_LOAD( "mqueen.bin",   0x0000, 0x0800, BAD_DUMP CRC(cb664001) SHA1(00152f89e58bc11567a8de32ccaaa47146dace0d) )
	ROM_RELOAD(0x0800, 0x0800)
ROM_END

/*-------------------------------------------------------------------
/ King Kong
/-------------------------------------------------------------------*/

ROM_START(kkongltd)
	ROM_REGION(0x1000, "roms", 0)
	ROM_LOAD("kong.bin", 0x0000, 0x0800, CRC(5b2a3123) SHA1(eee417d17d3272ee63c728915af84da33f1f73a2))
	ROM_RELOAD(0x0800, 0x0800)
ROM_END

/*-------------------------------------------------------------------
/ Viking King
/-------------------------------------------------------------------*/

ROM_START(vikngkng)
	ROM_REGION(0x1000, "roms", 0)
	ROM_LOAD("vikking.bin", 0x0000, 0x0800, CRC(aa32d158) SHA1(b24294ae4ecb2ab3119ad7fe79ef567b19ac792a))
	ROM_RELOAD(0x0800, 0x0800)
ROM_END

/*-------------------------------------------------------------------
/ Force
/-------------------------------------------------------------------*/

ROM_START(force)
	ROM_REGION(0x1000, "roms", 0)
	ROM_LOAD("forceltd.bin", 0x0000, 0x0800, CRC(48f9ebbe) SHA1(8aaab352fb21263b1b93ffefd9b5169284083beb))
	ROM_RELOAD(0x0800, 0x0800)
ROM_END

/*-------------------------------------------------------------------
/ Space Poker
/-------------------------------------------------------------------*/

ROM_START(spcpoker)
	ROM_REGION(0x1000, "roms", 0)
	ROM_LOAD( "space_poker_16_jp.bin", 0x0000, 0x0800, CRC(8fc2bdf6) SHA1(48eeae7ef21adbb4801e339565e55a03db6de179) )
	ROM_RELOAD(0x0800, 0x0800)
ROM_END

/*-------------------------------------------------------------------
/ Black Hole
/-------------------------------------------------------------------*/
ROM_START(bhol_ltd)
	ROM_REGION(0x1000, "roms", 0)
	ROM_LOAD("blackhol.bin", 0x0000, 0x0800, CRC(9f6ae35e) SHA1(c17bf08a41c6cf93550671b0724c58e8ac302c33))
	ROM_RELOAD(0x0800, 0x0800)
ROM_END

/*-------------------------------------------------------------------
/ Cowboy Eight Ball
/-------------------------------------------------------------------*/

ROM_START(cowboy)
	ROM_REGION(0x1000, "roms", 0)
	ROM_LOAD("cowboy3p.bin", 0x0000, 0x1000, CRC(5afa29af) SHA1(a5ccf5cd17c63d4292222b792535187b1bcfa786))
ROM_END

ROM_START(cowboya)
	ROM_REGION(0x1000, "roms", 0)
	ROM_LOAD("cowboy3a.bin", 0x0000, 0x1000, CRC(48278d77) SHA1(4102a2be10b48d0edb2b636e10696cbb2cd3a4c4))
ROM_END

/*-------------------------------------------------------------------
/ Zephy
/-------------------------------------------------------------------*/
ROM_START(zephy)
	ROM_REGION(0x1000, "roms", 0)
	ROM_LOAD("zephy.l2", 0x0000, 0x1000, CRC(8dd11287) SHA1(8133d0c797eb0fdb56d83fc55da91bfc3cddc9e3))
ROM_END

ROM_START(zephya)
	ROM_REGION(0x1000, "roms", 0)
	ROM_LOAD("zephy1.bin", 0x0000, 0x1000, CRC(ae189c8a) SHA1(c309b436ef94cd5c266c88fe5f222261e083e4eb))
ROM_END

/*-------------------------------------------------------------------
/ Cowboy Eight Ball 2
/-------------------------------------------------------------------*/
ROM_START(cowboy2)
	ROM_REGION(0x2000, "roms", 0)
	ROM_LOAD("cowboy_l.bin", 0x0000, 0x1000, CRC(87befe2a) SHA1(93fdf40b10e53d7d95e5dc72923b6be887411fc0))
	ROM_LOAD("cowboy_h.bin", 0x1000, 0x1000, CRC(105e5d7b) SHA1(75edeab8c8ba19f334479133802acbc25f405763))
ROM_END

/*-------------------------------------------------------------------
/ Haunted Hotel
/-------------------------------------------------------------------*/
ROM_START(hhotel)
	ROM_REGION(0x2000, "roms", 0)
	ROM_LOAD("hh1.bin", 0x0000, 0x1000, CRC(a107a683) SHA1(5bb79d9a0a6b33f067cdd54942784c67ab557909))
	ROM_LOAD("hh2.bin", 0x1000, 0x1000, CRC(e0c2ebc1) SHA1(131240589162c7b3f44a2bb951945c7d64f89c8d))
ROM_END

/*-------------------------------------------------------------------
/ Mr. & Mrs. Pec-Men
/-------------------------------------------------------------------*/
ROM_START(pecmen)
	ROM_REGION(0x2000, "roms", 0)
	ROM_LOAD("pecmen_l.bin", 0x0000, 0x1000, CRC(f86c724e) SHA1(635ec94a1c6e77800ef9774102cc639be86c4261))
	ROM_LOAD("pecmen_h.bin", 0x1000, 0x1000, CRC(013abca0) SHA1(269376af92368d214c3d09ec6d3eb653841666f3))
ROM_END

/*-------------------------------------------------------------------
/ Al Capone
/-------------------------------------------------------------------*/
ROM_START(alcapone)
	ROM_REGION(0x2000, "roms", 0)
	ROM_LOAD("alcapo_l.bin", 0x0000, 0x1000, CRC(c4270ba8) SHA1(f3d80af9900c94df2d43f2755341a346a0b64c87))
	ROM_LOAD("alcapo_h.bin", 0x1000, 0x1000, CRC(279f766d) SHA1(453c58e44c4ef8f1f9eb752b6163c61ebed70b27))
ROM_END

/*-------------------------------------------------------------------
/ Alien Warrior
/-------------------------------------------------------------------*/

// No good dump available

/*-------------------------------------------------------------------
/ Columbia
/-------------------------------------------------------------------*/
ROM_START(columbia)
	ROM_REGION(0x2000, "roms", 0)
	ROM_LOAD("columb-l.bin", 0x0000, 0x1000, CRC(ac345dee) SHA1(14f03fa8059de5cd69cc83638aa6533fbcead37e))
	ROM_LOAD("columb-h.bin", 0x1000, 0x1000, CRC(acd2a85b) SHA1(30889ee4230ce05f6060f926b2137bbf5939db2d))
ROM_END

/*-------------------------------------------------------------------
/ Time Machine
/-------------------------------------------------------------------*/
ROM_START(tmacltd4)
	ROM_REGION(0x2000, "roms", 0)
	ROM_LOAD("tm4-l.bin", 0x0000, 0x1000, CRC(69691662) SHA1(3d86314967075e3f5b168c8d7bf6b26bbbb957bd))
	ROM_LOAD("tm4-h.bin", 0x1000, 0x1000, CRC(f5f97992) SHA1(ba31f71a600e7061b500e0750f50643503e52a80))
ROM_END

ROM_START(tmacltd2)
	ROM_REGION(0x2000, "roms", 0)
	ROM_LOAD("tm4-l.bin", 0x0000, 0x1000, NO_DUMP)
	ROM_LOAD("tm4-h.bin", 0x1000, 0x1000, CRC(f717c9db) SHA1(9ca5819b707fa20edfc289734e1aa189ae242aa3))
ROM_END

/*-------------------------------------------------------------------
/ Trick Shooter
/-------------------------------------------------------------------*/
ROM_START(tricksht)
	ROM_REGION(0x2000, "roms", 0)
	ROM_LOAD("tricks-l.bin", 0x0000, 0x1000, CRC(951413ff) SHA1(f4a28f7b41cb077377433dc7bfb6647e5d392481))
	ROM_LOAD("tricks-h.bin", 0x1000, 0x1000, CRC(2e4efb51) SHA1(3dd20addecf4b47bd68b05d557c378d1dbbbd892))
ROM_END

} // Anonymous namespace

// system 3, 2-player, with beep sounds, playable
GAME(1981, arizona,  0,        ltd3, ltd3, ltd3_state, init_0,   ROT0, "LTD", "Arizona",                           MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME(1981, atla_ltd, 0,        ltd3, ltd3, ltd3_state, init_0,   ROT0, "LTD", "Atlantis (LTD)",                    MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME(1981, discodan, 0,        ltd3, ltd3, ltd3_state, init_0,   ROT0, "LTD", "Disco Dancing",                     MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME(1981, hustlerp, 0,        ltd3, ltd3, ltd3_state, init_0,   ROT0, "LTD", "Hustler",                           MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME(1981, marqueen, 0,        ltd3, ltd3, ltd3_state, init_0,   ROT0, "LTD", "Martian Queen",                     MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME(1981, kkongltd, 0,        ltd3, ltd3, ltd3_state, init_0,   ROT0, "LTD", "King Kong",                         MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME(198?, vikngkng, 0,        ltd3, ltd3, ltd3_state, init_0,   ROT0, "LTD", "Viking King",                       MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )

// system 3, 2-player, unknown sound system, playable
GAME(1981, force,    0,        ltd3, ltd3, ltd3_state, init_1,   ROT0, "LTD", "Force",                             MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME(1981, bhol_ltd, 0,        ltd3, ltd3, ltd3_state, init_2,   ROT0, "LTD", "Black Hole (LTD)",                  MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME(1981, spcpoker, 0,        ltd3, ltd3, ltd3_state, init_2,   ROT0, "LTD", "Space Poker",                       MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )

// system 3, 3-player, ay sounds, unplayable
GAME(1981, cowboy,   0,        ltd3, ltd3, ltd3_state, init_3,   ROT0, "LTD", "Cowboy Eight Ball (set 1)",         MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME(1981, cowboya,  cowboy,   ltd3, ltd3, ltd3_state, init_3,   ROT0, "LTD", "Cowboy Eight Ball (set 2)",         MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME(1981, zephy,    0,        ltd3, ltd3, ltd3_state, init_3,   ROT0, "LTD", "Zephy (set 1)",                     MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME(1981, zephya,   zephy,    ltd3, ltd3, ltd3_state, init_3,   ROT0, "LTD", "Zephy (set 2)",                     MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )

// system 4, mostly 4-player, ay sounds, unplayable
GAME(1982, cowboy2,  0,        ltd4, ltd4, ltd4_state, empty_init, ROT0, "LTD", "Cowboy Eight Ball 2",               MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME(1981, hhotel,   0,        ltd4, ltd4, ltd4_state, empty_init, ROT0, "LTD", "Haunted Hotel",                     MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME(1981, pecmen,   0,        ltd4, ltd4, ltd4_state, empty_init, ROT0, "LTD", "Mr. & Mrs. Pec-Men",                MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME(1981, alcapone, 0,        ltd4, ltd4, ltd4_state, empty_init, ROT0, "LTD", "Al Capone",                         MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME(1982, columbia, 0,        ltd4, ltd4, ltd4_state, empty_init, ROT0, "LTD", "Columbia",                          MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME(1981, tmacltd4, 0,        ltd4, ltd4, ltd4_state, empty_init, ROT0, "LTD", "Time Machine (LTD, 4 players)",     MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME(1981, tmacltd2, tmacltd4, ltd4, ltd4, ltd4_state, empty_init, ROT0, "LTD", "Time Machine (LTD, 2 players)",     MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME(1982, tricksht, 0,        ltd4, ltd4, ltd4_state, empty_init, ROT0, "LTD", "Trick Shooter",                     MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
