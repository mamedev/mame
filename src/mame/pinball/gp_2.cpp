// license:BSD-3-Clause
// copyright-holders:Robbbert
/*****************************************************************************************

PINBALL
Game Plan MPU-2

When first turned on, you need to press num-0 to enter the setup program, then keep
pressing num-0 until 06 shows in the credits display. Press the credit button to set
the first high score at which a free credit is awarded. Then press num-0 to set the
2nd high score, then num-0 to set the 3rd high score. Keep pressing num-0 until you exit
back to normal operation. If this setup is not done, each player will get 3 free
games at the start of ball 1.

All the Z80 "maincpu" code is copied from gp_1.cpp
Any bug fixes need to be applied both here and there.

Sound boards: (each game has its own custom sounds)
-------------------------------------------------------------------------------
Old Coney Island            3x SN76477
Sharpshooter                3x SN76477
Super Nova                  4x SN76477
Andromeda/Cyclopes          6808/6802/6810 + 6821 + 1 rom + ZN428
Lady Sharpshooter           6808/6802/6810 + 2x6821 + 2xROM + 6840 + discrete
(no schematics for the others)

Status:
- All games are working without sound, except:
- gwarfare stops responding to inputs after a while
- mbossy rom missing, black screen
- andromep, andromepa, cyclopes, cyclopes1: To start, hold CX hit 1. End ball, CX


ToDo:
- Sound
- Mechanical sounds
- Inputs vary per machine

******************************************************************************************/


#include "emu.h"
#include "genpin.h"
#include "cpu/z80/z80.h"
#include "machine/z80daisy.h"
#include "machine/i8255.h"
#include "machine/clock.h"
#include "machine/z80ctc.h"
//#include "sound/sn76477.h"
//#include "speaker.h"
#include "gp_2.lh"

namespace {

class gp_2_state : public genpin_class
{
public:
	gp_2_state(const machine_config &mconfig, device_type type, const char *tag)
		: genpin_class(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_ppi(*this, "ppi")
		, m_ctc(*this, "ctc")
		, m_io_dsw0(*this, "DSW0")
		, m_io_dsw1(*this, "DSW1")
		, m_io_dsw2(*this, "DSW2")
		, m_io_dsw3(*this, "DSW3")
		, m_io_x7(*this, "X7")
		, m_io_x8(*this, "X8")
		, m_io_x9(*this, "X9")
		, m_io_xa(*this, "XA")
		, m_io_xb(*this, "XB")
		, m_digits(*this, "digit%d", 0U)
		, m_io_leds(*this, "led%d", 0U)
		, m_io_outputs(*this, "out%d", 0U)
	{ }

	void gp_2(machine_config &config);

private:
	void porta_w(u8 data);
	void portc_w(u8 data);
	u8 portb_r();
	void io_map(address_map &map) ATTR_COLD;
	void mem_map(address_map &map) ATTR_COLD;
	u8 m_u14 = 0U;
	u8 m_digit = 0U;
	u8 m_segment[16]{};
	u8 m_last_solenoid = 15U;
	virtual void machine_reset() override ATTR_COLD;
	virtual void machine_start() override ATTR_COLD;
	required_device<z80_device> m_maincpu;
	required_device<i8255_device> m_ppi;
	required_device<z80ctc_device> m_ctc;
	required_ioport m_io_dsw0;
	required_ioport m_io_dsw1;
	required_ioport m_io_dsw2;
	required_ioport m_io_dsw3;
	required_ioport m_io_x7;
	required_ioport m_io_x8;
	required_ioport m_io_x9;
	required_ioport m_io_xa;
	required_ioport m_io_xb;
	output_finder<40> m_digits;
	output_finder<1> m_io_leds;
	output_finder<64> m_io_outputs;   // 16 solenoids + 48 lamps
};


void gp_2_state::mem_map(address_map &map)
{
	map(0x0000, 0x3fff).rom();
	map(0x8c00, 0x8dff).ram().share("nvram");
}

void gp_2_state::io_map(address_map &map)
{
	map.global_mask(0x0f);
	map(0x04, 0x07).rw(m_ppi, FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0x08, 0x0b).rw(m_ctc, FUNC(z80ctc_device::read), FUNC(z80ctc_device::write));
}

static INPUT_PORTS_START( gp_common )
	PORT_START("X7")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_3_PAD) PORT_NAME("Accounting Reset") // This pushbutton on the MPU board is called "S33"
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_0) PORT_NAME("Slam Tilt")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_A) PORT_NAME("INP04")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_COIN3 )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_9) PORT_NAME("Tilt")

	PORT_START("X8")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_B) PORT_NAME("INP09")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_C) PORT_NAME("INP10")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_X) PORT_NAME("Outhole")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_D) PORT_NAME("INP12")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_E) PORT_NAME("INP13")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_F) PORT_NAME("INP14")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_G) PORT_NAME("INP15")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_H) PORT_NAME("INP16")

	PORT_START("X9")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_I) PORT_NAME("INP17")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_J) PORT_NAME("INP18")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_K) PORT_NAME("INP19")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_L) PORT_NAME("INP20")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_M) PORT_NAME("INP21")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_N) PORT_NAME("INP22")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_O) PORT_NAME("INP23")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_P) PORT_NAME("INP24")

	PORT_START("XA")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_Q) PORT_NAME("INP25")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_0_PAD) PORT_NAME("Setup")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_R) PORT_NAME("INP27")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_S) PORT_NAME("INP28")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_T) PORT_NAME("INP29")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_U) PORT_NAME("INP30")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_V) PORT_NAME("INP31")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_W) PORT_NAME("INP32")

	PORT_START("XB")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_Y) PORT_NAME("INP33")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_Z) PORT_NAME("INP34")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_COMMA) PORT_NAME("INP35")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_STOP) PORT_NAME("INP36")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_SLASH) PORT_NAME("INP37")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_COLON) PORT_NAME("INP38")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_QUOTE) PORT_NAME("INP39")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_ENTER) PORT_NAME("INP40")
INPUT_PORTS_END

static INPUT_PORTS_START( gp_1 )
	PORT_START("DSW0")
	PORT_DIPNAME( 0x1f, 0x02, "Coin Slot 1")
	PORT_DIPSETTING(    0x03, DEF_STR( 2C_1C ))
	PORT_DIPSETTING(    0x05, DEF_STR( 2C_2C ))
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_1C ))
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_3C )) // same as 01
	PORT_DIPSETTING(    0x07, DEF_STR( 2C_3C ))
	PORT_DIPSETTING(    0x09, DEF_STR( 2C_4C ))
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_2C ))
	PORT_DIPSETTING(    0x0b, DEF_STR( 2C_5C ))
	PORT_DIPSETTING(    0x0d, DEF_STR( 2C_6C ))
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_3C ))
	PORT_DIPSETTING(    0x0f, DEF_STR( 2C_7C ))
	PORT_DIPSETTING(    0x11, DEF_STR( 2C_8C ))
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_4C ))
	PORT_DIPSETTING(    0x13, "2 Coins/9 Credits")
	PORT_DIPSETTING(    0x15, "2 Coins/10 Credits")
	PORT_DIPSETTING(    0x0a, DEF_STR( 1C_5C ))
	PORT_DIPSETTING(    0x17, "2 Coins/11 Credits")
	PORT_DIPSETTING(    0x19, "2 Coins/12 Credits")
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_6C ))
	PORT_DIPSETTING(    0x1b, "2 Coins/13 Credits")
	PORT_DIPSETTING(    0x1d, "2 Coins/14 Credits")
	PORT_DIPSETTING(    0x0e, DEF_STR( 1C_7C ))
	PORT_DIPSETTING(    0x1f, "2 Coins/15 Credits")
	PORT_DIPSETTING(    0x10, DEF_STR( 1C_8C ))
	PORT_DIPSETTING(    0x12, DEF_STR( 1C_9C ))
	PORT_DIPSETTING(    0x14, "1 Coin/10 Credits")
	PORT_DIPSETTING(    0x16, "1 Coin/11 Credits")
	PORT_DIPSETTING(    0x18, "1 Coin/12 Credits")
	PORT_DIPSETTING(    0x1a, "1 Coin/13 Credits")
	PORT_DIPSETTING(    0x1c, "1 Coin/14 Credits")
	PORT_DIPSETTING(    0x1e, "1 Coin/15 Credits")
	PORT_DIPNAME( 0x20, 0x00, "S06")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x20, DEF_STR( On ))
	PORT_DIPNAME( 0x40, 0x00, "S07")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x40, DEF_STR( On ))
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Free_Play ))
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x80, DEF_STR( On ))

	PORT_START("DSW1")
	PORT_DIPNAME( 0x0f, 0x00, "Coin Slot 2") // S09-12 determine coinage for slot 2
	PORT_DIPSETTING(    0x00, "Same as Slot 1")
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_1C ))
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ))
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_3C ))
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_4C ))
	PORT_DIPSETTING(    0x05, DEF_STR( 1C_5C ))
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_6C ))
	PORT_DIPSETTING(    0x07, DEF_STR( 1C_7C ))
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_8C ))
	PORT_DIPSETTING(    0x09, DEF_STR( 1C_9C ))
	PORT_DIPSETTING(    0x0a, "1 Coin/10 Credits")
	PORT_DIPSETTING(    0x0b, "1 Coin/11 Credits")
	PORT_DIPSETTING(    0x0c, "1 Coin/12 Credits")
	PORT_DIPSETTING(    0x0d, "1 Coin/13 Credits")
	PORT_DIPSETTING(    0x0e, "1 Coin/14 Credits")
	PORT_DIPSETTING(    0x0f, "1 Coin/15 Credits")
	PORT_DIPNAME( 0x10, 0x00, "S13")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x10, DEF_STR( On ))
	PORT_DIPNAME( 0x20, 0x00, "S14")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x20, DEF_STR( On ))
	PORT_DIPNAME( 0x40, 0x00, "S15")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x40, DEF_STR( On ))
	PORT_DIPNAME( 0x80, 0x00, "Play Tunes")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x80, DEF_STR( On ))

	PORT_START("DSW2")
	PORT_DIPNAME( 0x1f, 0x02, "Coin Slot 3")
	PORT_DIPSETTING(    0x03, DEF_STR( 2C_1C ))
	PORT_DIPSETTING(    0x05, DEF_STR( 2C_2C ))
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_1C ))
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_3C )) // same as 01
	PORT_DIPSETTING(    0x07, DEF_STR( 2C_3C ))
	PORT_DIPSETTING(    0x09, DEF_STR( 2C_4C ))
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_2C ))
	PORT_DIPSETTING(    0x0b, DEF_STR( 2C_5C ))
	PORT_DIPSETTING(    0x0d, DEF_STR( 2C_6C ))
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_3C ))
	PORT_DIPSETTING(    0x0f, DEF_STR( 2C_7C ))
	PORT_DIPSETTING(    0x11, DEF_STR( 2C_8C ))
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_4C ))
	PORT_DIPSETTING(    0x13, "2 Coins/9 Credits")
	PORT_DIPSETTING(    0x15, "2 Coins/10 Credits")
	PORT_DIPSETTING(    0x0a, DEF_STR( 1C_5C ))
	PORT_DIPSETTING(    0x17, "2 Coins/11 Credits")
	PORT_DIPSETTING(    0x19, "2 Coins/12 Credits")
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_6C ))
	PORT_DIPSETTING(    0x1b, "2 Coins/13 Credits")
	PORT_DIPSETTING(    0x1d, "2 Coins/14 Credits")
	PORT_DIPSETTING(    0x0e, DEF_STR( 1C_7C ))
	PORT_DIPSETTING(    0x1f, "2 Coins/15 Credits")
	PORT_DIPSETTING(    0x10, DEF_STR( 1C_8C ))
	PORT_DIPSETTING(    0x12, DEF_STR( 1C_9C ))
	PORT_DIPSETTING(    0x14, "1 Coin/10 Credits")
	PORT_DIPSETTING(    0x16, "1 Coin/11 Credits")
	PORT_DIPSETTING(    0x18, "1 Coin/12 Credits")
	PORT_DIPSETTING(    0x1a, "1 Coin/13 Credits")
	PORT_DIPSETTING(    0x1c, "1 Coin/14 Credits")
	PORT_DIPSETTING(    0x1e, "1 Coin/15 Credits")
	PORT_DIPNAME( 0x20, 0x00, "S22")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x20, DEF_STR( On ))
	PORT_DIPNAME( 0x40, 0x00, "S23")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x40, DEF_STR( On ))
	PORT_DIPNAME( 0x80, 0x00, "S24")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x80, DEF_STR( On ))

	PORT_START("DSW3")
	PORT_DIPNAME( 0x07, 0x02, "Max number of credits")
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPSETTING(    0x01, "10")
	PORT_DIPSETTING(    0x02, "15")
	PORT_DIPSETTING(    0x03, "20")
	PORT_DIPSETTING(    0x04, "25")
	PORT_DIPSETTING(    0x05, "30")
	PORT_DIPSETTING(    0x06, "35")
	PORT_DIPSETTING(    0x07, "40")
	PORT_DIPNAME( 0x08, 0x00, "Balls")
	PORT_DIPSETTING(    0x00, "3")
	PORT_DIPSETTING(    0x08, "5")
	PORT_DIPNAME( 0x10, 0x10, "Award")
	PORT_DIPSETTING(    0x00, "Extra Ball")
	PORT_DIPSETTING(    0x10, "Replay")
	PORT_DIPNAME( 0x20, 0x20, "Match")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x20, DEF_STR( On ))
	PORT_DIPNAME( 0xC0, 0x80, "Credits for exceeding high score")
	PORT_DIPSETTING(    0x00, "0")
	PORT_DIPSETTING(    0x40, "1")
	PORT_DIPSETTING(    0x80, "2")
	PORT_DIPSETTING(    0xC0, "3")

	PORT_INCLUDE(gp_common)
INPUT_PORTS_END

static INPUT_PORTS_START( gp_2 )
	PORT_START("DSW0")
	PORT_DIPNAME( 0x1f, 0x02, "Coin Slot 1")
	PORT_DIPSETTING(    0x03, DEF_STR( 2C_1C ))
	PORT_DIPSETTING(    0x05, DEF_STR( 2C_2C ))
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_1C ))
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_3C )) // same as 01
	PORT_DIPSETTING(    0x07, DEF_STR( 2C_3C ))
	PORT_DIPSETTING(    0x09, DEF_STR( 2C_4C ))
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_2C ))
	PORT_DIPSETTING(    0x0b, DEF_STR( 2C_5C ))
	PORT_DIPSETTING(    0x0d, DEF_STR( 2C_6C ))
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_3C ))
	PORT_DIPSETTING(    0x0f, DEF_STR( 2C_7C ))
	PORT_DIPSETTING(    0x11, DEF_STR( 2C_8C ))
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_4C ))
	PORT_DIPSETTING(    0x13, "2 Coins/9 Credits")
	PORT_DIPSETTING(    0x15, "2 Coins/10 Credits")
	PORT_DIPSETTING(    0x0a, DEF_STR( 1C_5C ))
	PORT_DIPSETTING(    0x17, "2 Coins/11 Credits")
	PORT_DIPSETTING(    0x19, "2 Coins/12 Credits")
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_6C ))
	PORT_DIPSETTING(    0x1b, "2 Coins/13 Credits")
	PORT_DIPSETTING(    0x1d, "2 Coins/14 Credits")
	PORT_DIPSETTING(    0x0e, DEF_STR( 1C_7C ))
	PORT_DIPSETTING(    0x1f, "2 Coins/15 Credits")
	PORT_DIPSETTING(    0x10, DEF_STR( 1C_8C ))
	PORT_DIPSETTING(    0x12, DEF_STR( 1C_9C ))
	PORT_DIPSETTING(    0x14, "1 Coin/10 Credits")
	PORT_DIPSETTING(    0x16, "1 Coin/11 Credits")
	PORT_DIPSETTING(    0x18, "1 Coin/12 Credits")
	PORT_DIPSETTING(    0x1a, "1 Coin/13 Credits")
	PORT_DIPSETTING(    0x1c, "1 Coin/14 Credits")
	PORT_DIPSETTING(    0x1e, "1 Coin/15 Credits")
	PORT_DIPNAME( 0x60, 0x00, "Special lights at")
	PORT_DIPSETTING(    0x60, "60000")
	PORT_DIPSETTING(    0x40, "90000")
	PORT_DIPSETTING(    0x20, "120000")
	PORT_DIPSETTING(    0x00, "150000")
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Free_Play ))
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x80, DEF_STR( On ))

	PORT_START("DSW1")
	PORT_DIPNAME( 0x0f, 0x00, "Coin Slot 2") // S09-12 determine coinage for slot 2
	PORT_DIPSETTING(    0x00, "Same as Slot 1")
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_1C ))
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ))
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_3C ))
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_4C ))
	PORT_DIPSETTING(    0x05, DEF_STR( 1C_5C ))
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_6C ))
	PORT_DIPSETTING(    0x07, DEF_STR( 1C_7C ))
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_8C ))
	PORT_DIPSETTING(    0x09, DEF_STR( 1C_9C ))
	PORT_DIPSETTING(    0x0a, "1 Coin/10 Credits")
	PORT_DIPSETTING(    0x0b, "1 Coin/11 Credits")
	PORT_DIPSETTING(    0x0c, "1 Coin/12 Credits")
	PORT_DIPSETTING(    0x0d, "1 Coin/13 Credits")
	PORT_DIPSETTING(    0x0e, "1 Coin/14 Credits")
	PORT_DIPSETTING(    0x0f, "1 Coin/15 Credits")
	PORT_DIPNAME( 0x10, 0x10, "Music")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x10, DEF_STR( On ))
	PORT_DIPNAME( 0x20, 0x20, "Extra Ball")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x20, DEF_STR( On ))
	PORT_DIPNAME( 0x40, 0x40, "Remember Saucer Values")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x40, DEF_STR( On ))
	PORT_DIPNAME( 0x80, 0x80, "Extra Ball lights at")
	PORT_DIPSETTING(    0x80, "100000")
	PORT_DIPSETTING(    0x00, "150000")

	PORT_START("DSW2")
	PORT_DIPNAME( 0x1f, 0x02, "Coin Slot 3")
	PORT_DIPSETTING(    0x03, DEF_STR( 2C_1C ))
	PORT_DIPSETTING(    0x05, DEF_STR( 2C_2C ))
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_1C ))
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_3C )) // same as 01
	PORT_DIPSETTING(    0x07, DEF_STR( 2C_3C ))
	PORT_DIPSETTING(    0x09, DEF_STR( 2C_4C ))
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_2C ))
	PORT_DIPSETTING(    0x0b, DEF_STR( 2C_5C ))
	PORT_DIPSETTING(    0x0d, DEF_STR( 2C_6C ))
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_3C ))
	PORT_DIPSETTING(    0x0f, DEF_STR( 2C_7C ))
	PORT_DIPSETTING(    0x11, DEF_STR( 2C_8C ))
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_4C ))
	PORT_DIPSETTING(    0x13, "2 Coins/9 Credits")
	PORT_DIPSETTING(    0x15, "2 Coins/10 Credits")
	PORT_DIPSETTING(    0x0a, DEF_STR( 1C_5C ))
	PORT_DIPSETTING(    0x17, "2 Coins/11 Credits")
	PORT_DIPSETTING(    0x19, "2 Coins/12 Credits")
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_6C ))
	PORT_DIPSETTING(    0x1b, "2 Coins/13 Credits")
	PORT_DIPSETTING(    0x1d, "2 Coins/14 Credits")
	PORT_DIPSETTING(    0x0e, DEF_STR( 1C_7C ))
	PORT_DIPSETTING(    0x1f, "2 Coins/15 Credits")
	PORT_DIPSETTING(    0x10, DEF_STR( 1C_8C ))
	PORT_DIPSETTING(    0x12, DEF_STR( 1C_9C ))
	PORT_DIPSETTING(    0x14, "1 Coin/10 Credits")
	PORT_DIPSETTING(    0x16, "1 Coin/11 Credits")
	PORT_DIPSETTING(    0x18, "1 Coin/12 Credits")
	PORT_DIPSETTING(    0x1a, "1 Coin/13 Credits")
	PORT_DIPSETTING(    0x1c, "1 Coin/14 Credits")
	PORT_DIPSETTING(    0x1e, "1 Coin/15 Credits")
	PORT_DIPNAME( 0x20, 0x00, "Remember Bonus Multiplier")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x20, DEF_STR( On ))
	PORT_DIPNAME( 0xc0, 0x80, "Balls")
	PORT_DIPSETTING(    0x00, "1")
	PORT_DIPSETTING(    0x40, "2")
	PORT_DIPSETTING(    0x80, "3")
	PORT_DIPSETTING(    0xc0, "5")

	PORT_START("DSW3")
	PORT_DIPNAME( 0x01, 0x00, "Remember Special and Extra Ball lanes")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x01, DEF_STR( On ))
	PORT_DIPNAME( 0x06, 0x04, "Max number of credits")
	PORT_DIPSETTING(    0x00, "10")
	PORT_DIPSETTING(    0x02, "20")
	PORT_DIPSETTING(    0x04, "30")
	PORT_DIPSETTING(    0x06, "40")
	PORT_DIPNAME( 0x18, 0x18, "Award")
	PORT_DIPSETTING(    0x00, DEF_STR( None ))
	PORT_DIPSETTING(    0x08, "50000 points")
	PORT_DIPSETTING(    0x10, "Extra Ball")
	PORT_DIPSETTING(    0x18, "Replay")
	PORT_DIPNAME( 0x20, 0x20, "Match")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x20, DEF_STR( On ))
	PORT_DIPNAME( 0xC0, 0x80, "Credits for exceeding high score")
	PORT_DIPSETTING(    0x00, "0")
	PORT_DIPSETTING(    0x40, "1")
	PORT_DIPSETTING(    0x80, "2")
	PORT_DIPSETTING(    0xC0, "3")

	PORT_INCLUDE(gp_common)
INPUT_PORTS_END

u8 gp_2_state::portb_r()
{
	switch (m_u14)
	{
		case 7:
			return m_io_x7->read();
		case 8:
			return m_io_x8->read();
		case 9:
			return m_io_x9->read();
		case 10:
			return m_io_xa->read();
		case 11:
			return m_io_xb->read();
		case 12:
			return m_io_dsw0->read();
		case 13:
			return m_io_dsw1->read();
		case 14:
			return m_io_dsw2->read();
		case 15:
			return m_io_dsw3->read();
	}
	return 0;
}

void gp_2_state::porta_w(u8 data)
{
	m_u14 = data >> 4;
	if (m_last_solenoid < 15)
		m_io_outputs[m_last_solenoid] = 0;

	m_u14 = data >> 4;
	// Solenoids are different per game, no point allocating anything specific
	if ((m_u14 >= 1) && (m_u14 <= 2))
	{
		if ((data & 15) < 15)
			m_io_outputs[data & 15] = 1;
		m_last_solenoid = data & 15;
	}
	else
		m_last_solenoid = 15;

	static const u8 patterns[16] = { 0x3f,0x06,0x5b,0x4f,0x66,0x6d,0x7c,0x07,0x7f,0x67,0x58,0x4c,0x62,0x69,0x78,0 }; // 7448
	if (m_digit == 7)
		m_segment[m_u14] = data & 15;
	else
	if (m_u14 == 7)
	{
		m_digits[m_digit] = patterns[m_segment[7]];
		m_digits[m_digit+8] = patterns[m_segment[8]];
		m_digits[m_digit+16] = patterns[m_segment[9]];
		m_digits[m_digit+24] = patterns[m_segment[10]];
		m_digits[m_digit+32] = patterns[m_segment[11]];
	}

	// Lamps
	if ((m_u14 >= 3) && (m_u14 <= 5))
		for (u8 i = 0; i < 16; i++)
			m_io_outputs[(m_u14 - 3) * 16 + 16 + i] = ((data & 15) == i);
}

void gp_2_state::portc_w(u8 data)
{
	m_io_leds[0] = BIT(data, 3) ? 0 : 1;
	m_digit = data & 7;
}

void gp_2_state::machine_start()
{
	m_digits.resolve();
	m_io_leds.resolve();
	m_io_outputs.resolve();

	save_item(NAME(m_u14));
	save_item(NAME(m_digit));
	save_item(NAME(m_segment));
	save_item(NAME(m_last_solenoid));
}

void gp_2_state::machine_reset()
{
	m_u14 = 0;
	m_digit = 0xff;
	m_last_solenoid = 15;
	for (u8 i = 0; i < m_io_outputs.size(); i++)
		m_io_outputs[i] = 0;
	for (u8 i = 0; i < std::size(m_segment); i++)
		m_segment[i] = 0;
}

static const z80_daisy_config daisy_chain[] =
{
	{ "ctc" },
	{ nullptr }
};

void gp_2_state::gp_2(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, 2457600);
	m_maincpu->set_addrmap(AS_PROGRAM, &gp_2_state::mem_map);
	m_maincpu->set_addrmap(AS_IO, &gp_2_state::io_map);
	m_maincpu->set_daisy_config(daisy_chain);

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	/* Video */
	config.set_default_layout(layout_gp_2);

	/* Sound */
	genpin_audio(config);

	/* Devices */
	I8255A(config, m_ppi);
	m_ppi->out_pa_callback().set(FUNC(gp_2_state::porta_w));
	m_ppi->in_pb_callback().set(FUNC(gp_2_state::portb_r));
	m_ppi->out_pc_callback().set(FUNC(gp_2_state::portc_w));

	Z80CTC(config, m_ctc, 2457600);
	m_ctc->intr_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0); // Todo: absence of ints will cause a watchdog reset

	clock_device &cpoint_clock(CLOCK(config, "cpoint_clock", 120)); // crosspoint detector
	cpoint_clock.signal_handler().set(m_ctc, FUNC(z80ctc_device::trg2));
}

/* ========== ALTERNATE ROMS =======================================================================

This is a list of known alternate roms. Nothing has been tested.

Agents 777
ROM_LOAD( "770ab",        0x0000, 0x1000, CRC(e7f901c4) SHA1(c28c472e7890aaa10a6a47d8729bd6aebd15a20e) )

Andromeda
ROM_LOAD( "850.c",        0x1000, 0x0800, CRC(75dc73c4) SHA1(79fadec7650a1419f47b22875dee6f678114b439) ) // used by mbossy

Attila the Hun
ROM_LOAD( "260.a_b",      0x0000, 0x1000, CRC(0030ce7f) SHA1(44921f3c771c90f09cac3c927915aa8ff70bd782) )

Captain Hook
ROM_LOAD( "780a-b.13",    0x0000, 0x1000, CRC(ec757bb7) SHA1(0a10143cf7a60f2a39f36e8c40b5ce52cab04d0b) )

Lady Sharpshooter
ROM_LOAD( "830co.716",    0x0000, 0x1000, CRC(e3970bab) SHA1(f5be9a51c382a87dd304c39b27fc468a7f0a74f3) )
ROM_LOAD( "830ab.732",    0x0000, 0x1000, CRC(9ca19183) SHA1(83e73e809c2484396348990bfdfae143ac371f9a) )

Mike Bossy
ROM_LOAD( "snd-ic9.a",    0x0000, 0x1000, CRC(b0541739) SHA1(5392fa6a405ba2e09ad4d0ec52c48675bf507532) )

Pinball Lizard
ROM_LOAD( "9316b.lef",    0x0000, 0x0895, CRC(c0fb2543) SHA1(8e44f513e8f2afed2a40a6a3bb8637ab18631d2b) )  // not a rom

*/

/*-------------------------------------------------------------------
/ Agents 777 (November 1984) - Model #770
/-------------------------------------------------------------------*/
ROM_START(agent777)
	ROM_REGION(0x4000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD( "770a", 0x0000, 0x0800, CRC(fc4eebcd) SHA1(742a201e89c1357d2a1f24b0acf3b78ffec96c74))
	ROM_LOAD( "770b", 0x0800, 0x0800, CRC(ea62aece) SHA1(32be10bc76a59e03c3fd3294daefc8d28c20386a))
	ROM_LOAD( "770c", 0x1000, 0x0800, CRC(59280db7) SHA1(8f199be7bfbc01466541c07dc4c365e20055a66c))

	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD ("770snd", 0x3800, 0x0800, CRC(e4e66c9f) SHA1(f373facefb18c64377da47308a8bbd5fc80e9c2d))
	ROM_CONTINUE(0x7800, 0x0800)
	ROM_RELOAD (0xf000, 0x1000)
ROM_END

/*-------------------------------------------------------------------
/ Andromeda (August 1985) - Model #850
/-------------------------------------------------------------------*/
ROM_START(andromep)
	ROM_REGION(0x4000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD( "850.a", 0x0000, 0x1000, CRC(67ed03ee) SHA1(efe7c495766ffb73545a77ab24f02925ac0395f1))
	ROM_LOAD( "850.b", 0x1000, 0x1000, CRC(37c244e8) SHA1(5cef0a1a6f2c34f2d01bdd12ce11da40c8be4296))

	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("850.snd", 0x3800, 0x0800, CRC(18e084a6) SHA1(56efbabe60305f168ca479295577bff7f3a4dace))
	ROM_RELOAD(0x7800, 0x0800)
	ROM_RELOAD(0xf800, 0x0800)
ROM_END

ROM_START(andromepa)
	ROM_REGION(0x4000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD( "850.a", 0x0000, 0x1000, CRC(67ed03ee) SHA1(efe7c495766ffb73545a77ab24f02925ac0395f1))
	ROM_LOAD( "850b.rom", 0x1000, 0x1000, CRC(fc1829a5) SHA1(9761543d17c0a5c08b0fec45c35648ce769a3463))

	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("850.snd", 0x3800, 0x0800, CRC(18e084a6) SHA1(56efbabe60305f168ca479295577bff7f3a4dace))
	ROM_RELOAD(0x7800, 0x0800)
	ROM_RELOAD(0xf800, 0x0800)
ROM_END

/*-------------------------------------------------------------------
/ Attila the Hun (April 1984) - Model #260
/-------------------------------------------------------------------*/
ROM_START(attila)
	ROM_REGION(0x4000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD( "260.a", 0x0000, 0x0800, CRC(b31c11d8) SHA1(d3f2ad84cc28e99acb54349b232dbf8abdf15b21))
	ROM_LOAD( "260.b", 0x0800, 0x0800, CRC(e8cca86d) SHA1(ed0797175a573537be2d5119ad68b1847e49e578))
	ROM_LOAD( "260.c", 0x1000, 0x0800, CRC(206605c3) SHA1(14f61a2f43c29370bcb6db29969e8dfcfe3da1ab))

	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD ("260.snd", 0x3800, 0x0800, CRC(21e6b188) SHA1(84148942e6007d49bb4085ec3678954d48e4439e))
	ROM_CONTINUE(0x7800, 0x0800)
	ROM_RELOAD (0xf000, 0x1000)
ROM_END

/*-------------------------------------------------------------------
/ Captain Hook (April 1985) - Model #780
/-------------------------------------------------------------------*/
ROM_START(cpthook)
	ROM_REGION(0x4000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD( "780.a", 0x0000, 0x0800, CRC(6bd5a495) SHA1(8462e0c68176daee6b23dce9091f5aee99e62631))
	ROM_LOAD( "780.b", 0x0800, 0x0800, CRC(3d1c5555) SHA1(ecb0d40f5e6e37acfc8589816e24b26525273393))
	ROM_LOAD( "780.c", 0x1000, 0x0800, CRC(e54bc51f) SHA1(3480e0cdd43f9ac3fda8cd466b2f039210525e8b))

	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD ("780.snd", 0x3800, 0x0800, CRC(95af3392) SHA1(73a2b583b7fc423c2e4390667aebc90ad41f4f93))
	ROM_CONTINUE(0x7800, 0x0800)
	ROM_RELOAD (0xf000, 0x1000)
ROM_END

/*-------------------------------------------------------------------
/ Cyclopes (November 1985) - Model #800
/-------------------------------------------------------------------*/
ROM_START(cyclopes)
	ROM_REGION(0x4000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD( "800.a", 0x0000, 0x1000, CRC(3e9628e5) SHA1(4dad9e082a9f4140162bc155f2b0f0a948ba012f))
	ROM_LOAD( "800.b", 0x1000, 0x1000, CRC(3f945c46) SHA1(25eb543e0b0edcd0a0dcf8e4aa1405cda55ebe2e))
	ROM_LOAD( "800.c", 0x2000, 0x1000, CRC(7ea18e65) SHA1(e86d82e3ba659499dfbf14920b196252784724f7))

	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD ("800.snd", 0x3800, 0x0800, CRC(290db3d2) SHA1(a236594f7a89969981bd5707d6dfbb5120fb8f46))
	ROM_CONTINUE(0x7800, 0x0800)
	ROM_RELOAD (0xf000, 0x1000)
ROM_END

ROM_START(cyclopes1)
	ROM_REGION(0x4000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD( "800a.111585", 0x0000, 0x1000, CRC(13131b90) SHA1(33f6c4aaaa2511a9c78e68f8df9a6461cd92c23f))
	ROM_LOAD( "800b.111585", 0x1000, 0x1000, CRC(3d515632) SHA1(2c4a7f18760b591a85331fa0304177a730540489))
	ROM_LOAD( "800c.111585", 0x2000, 0x1000, CRC(2078bd3f) SHA1(fed719ffdbd71242393c0786ad6e763a9e25ff8e))

	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD ("800.snd", 0x3800, 0x0800, CRC(290db3d2) SHA1(a236594f7a89969981bd5707d6dfbb5120fb8f46))
	ROM_CONTINUE(0x7800, 0x0800)
	ROM_RELOAD (0xf000, 0x1000)
ROM_END

/*-------------------------------------------------------------------
/ Global Warfare (June 1981)  - Model #240
/-------------------------------------------------------------------*/
ROM_START(gwarfare)
	ROM_REGION(0x4000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD( "240a.716", 0x0000, 0x0800, CRC(30206428) SHA1(7a9029e4fd4c4c00da3256ed06464c0bd8022168))
	ROM_LOAD( "240b.716", 0x0800, 0x0800, CRC(a54eb15d) SHA1(b9235bd188c1251eb213789800b7686b5e3c557f))
	ROM_LOAD( "240c.716", 0x1000, 0x0800, CRC(60d115a8) SHA1(e970fdd7cbbb2c81ab8c8209edfb681798c683b9))

	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("gw240bot.rom", 0x3800, 0x0800, CRC(3245a206) SHA1(b321b2d276fbd74199eff2d8c0d1b8a2f5c93604))
	ROM_RELOAD(0xf800, 0x0800)
	ROM_LOAD("gw240top.rom",0x3000, 0x0800, CRC(faaf3de1) SHA1(9c984d1ac696eb16f7bf35463a69a470344314a7))
ROM_END

/*-------------------------------------------------------------------
/ Lady Sharpshooter (May 1985) - Cocktail Model #830
/-------------------------------------------------------------------*/
ROM_START(ladyshot)
	ROM_REGION(0x4000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD( "830a.716", 0x0000, 0x0800, CRC(c055b993) SHA1(a9a7156e5ec0a32db1ffe36b3c6280953a2606ff))
	ROM_LOAD( "830b.716", 0x0800, 0x0800, CRC(1e3308ea) SHA1(a5955a6a15b33c4cf35105ab524a8e7e03d748b6))
	ROM_LOAD( "830c.716", 0x1000, 0x0800, CRC(f5e1db15) SHA1(e8168ab37ba30211045fc96b23dad5f06592b38d))

	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD ("830.snd", 0x3800, 0x0800, NO_DUMP)
	ROM_CONTINUE(0x7800, 0x0800)
	ROM_RELOAD (0xf000, 0x1000)
ROM_END

ROM_START(ladyshota)
	ROM_REGION(0x4000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD( "830a2.716", 0x0000, 0x0800, CRC(2c1f1629) SHA1(9233ce4328d779ff6548cdd5d6819cd368bef313))
	ROM_LOAD( "830b2.716", 0x0800, 0x0800, CRC(2105a538) SHA1(0360d3e740d8b6f816cfe7fe1fb32ac476251b9f))
	ROM_LOAD( "830c2.716", 0x1000, 0x0800, CRC(2d96bdde) SHA1(7c03a29a91f03fba9ed5e53a93335113a7cbafb3))

	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD ("830.snd", 0x3800, 0x0800, NO_DUMP)
	ROM_CONTINUE(0x7800, 0x0800)
	ROM_RELOAD (0xf000, 0x1000)
ROM_END

/*-------------------------------------------------------------------
/ Loch Ness Monster (November 1985) - Model #??? (prototype only)
/-------------------------------------------------------------------*/
// 1 prototype exists. Company closed down just as game was about to go into production.

/*-------------------------------------------------------------------
/ Mike Bossy (January 1982) - Model #??? (prototype only)
/-------------------------------------------------------------------*/
ROM_START(mbossy)
	ROM_REGION(0x4000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD( "mb_a.716", 0x0000, 0x0800, CRC(a811f936) SHA1(f44fed7acd26a621f105925d52405e985d0b5e5d) )
	ROM_LOAD( "mb_b.716", 0x0800, 0x0800, CRC(75ec7247) SHA1(10fa1e3ac2adbd7b24744a4fb0149bcc74df6b4c) )
	ROM_LOAD( "mb_c.716", 0x1000, 0x0800, CRC(75dc73c4) SHA1(79fadec7650a1419f47b22875dee6f678114b439) )
	ROM_LOAD( "mb_d.716", 0x1800, 0x0800, NO_DUMP)

	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("mb.u9", 0x3800, 0x0800, CRC(dfa98db5) SHA1(65361630f530383e67837c428050bcdb15373c0b))
	ROM_RELOAD(0xf800, 0x0800)
	ROM_LOAD("mb.u10",0x3000, 0x0800, CRC(2d3c91f9) SHA1(7e1f067af29d9e484da234382d7dc821ca07b6c4))
ROM_END

/*-------------------------------------------------------------------
/ Old Coney Island! (December 1979) - Model #180
/-------------------------------------------------------------------*/
ROM_START(coneyis)
	ROM_REGION(0x4000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD( "130a.716", 0x0000, 0x0800, CRC(dc402b37) SHA1(90c46391a1e5f000f3b235d580463bf96b45bd3e))
	ROM_LOAD( "130b.716", 0x0800, 0x0800, CRC(19a86f5e) SHA1(bc4a87314fc9c4e74e492c3f6e44d5d6cae72939))
	ROM_LOAD( "130c.716", 0x1000, 0x0800, CRC(b956f67b) SHA1(ff64383d7f59e9bbec588553e35a21fb94c7203b))
ROM_END

/*-------------------------------------------------------------------
/ Pinball Lizard (June / July 1980) - Model #210
/-------------------------------------------------------------------*/
ROM_START(lizard)
	ROM_REGION(0x4000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD( "130a.716", 0x0000, 0x0800, CRC(dc402b37) SHA1(90c46391a1e5f000f3b235d580463bf96b45bd3e)) // u12
	ROM_LOAD( "130b.716", 0x0800, 0x0800, CRC(19a86f5e) SHA1(bc4a87314fc9c4e74e492c3f6e44d5d6cae72939)) // u13
	ROM_LOAD( "130c.716", 0x1000, 0x0800, CRC(b956f67b) SHA1(ff64383d7f59e9bbec588553e35a21fb94c7203b)) // u26

	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("lizard.u9", 0x3800, 0x0800, CRC(2d121b24) SHA1(55c16951538229571165c35a353da53e22d11f81))
	ROM_RELOAD(0xf800, 0x0800)
	ROM_LOAD("lizard.u10",0x3000, 0x0800, CRC(28b8f1f0) SHA1(db6d816366e0bca59376f6f8bf87e6a2d849aa72))
ROM_END

/*-------------------------------------------------------------------
/ Sharp Shooter II (November 1983) - Model #730
/-------------------------------------------------------------------*/
ROM_START(sshootr2)
	ROM_REGION(0x4000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD( "130a.716", 0x0000, 0x0800, CRC(dc402b37) SHA1(90c46391a1e5f000f3b235d580463bf96b45bd3e))
	ROM_LOAD( "130b.716", 0x0800, 0x0800, CRC(19a86f5e) SHA1(bc4a87314fc9c4e74e492c3f6e44d5d6cae72939))
	ROM_LOAD( "730c", 0x1000, 0x0800, CRC(d1af712b) SHA1(9dce2ec1c2d9630a29dd21f4685c09019e59b147))

	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("730u9.snd", 0x3800, 0x0800, CRC(dfa98db5) SHA1(65361630f530383e67837c428050bcdb15373c0b))
	ROM_RELOAD(0xf800, 0x0800)
	ROM_LOAD("730u10.snd",0x3000, 0x0800, CRC(6d3dcf44) SHA1(3703313d4172ebfec1dcacca949076541ee35cb7))
ROM_END

/*-------------------------------------------------------------------
/ Sharpshooter (May 1979) - Model #130
/-------------------------------------------------------------------*/
ROM_START(sshootep)
	ROM_REGION(0x4000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD( "130a.716", 0x0000, 0x0800, CRC(dc402b37) SHA1(90c46391a1e5f000f3b235d580463bf96b45bd3e))
	ROM_LOAD( "130b.716", 0x0800, 0x0800, CRC(19a86f5e) SHA1(bc4a87314fc9c4e74e492c3f6e44d5d6cae72939))
	ROM_LOAD( "130c.716", 0x1000, 0x0800, CRC(b956f67b) SHA1(ff64383d7f59e9bbec588553e35a21fb94c7203b))
ROM_END

/*-------------------------------------------------------------------
/ Super Nova (May 1982) - Model #150
/-------------------------------------------------------------------*/
ROM_START(suprnova)
	ROM_REGION(0x4000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD( "130a.716", 0x0000, 0x0800, CRC(dc402b37) SHA1(90c46391a1e5f000f3b235d580463bf96b45bd3e))
	ROM_LOAD( "150b.716", 0x0800, 0x0800, CRC(8980a8bb) SHA1(129816fe85681b760307a713c667737a750b0c04))
	ROM_LOAD( "150c.716", 0x1000, 0x0800, CRC(6fe08f96) SHA1(1309619a2400674fa1d05dc9214fdb85419fd1c3))
ROM_END

} // anonymous namespace

// GP1 dips
GAME( 1979, sshootep,  0,        gp_2, gp_1, gp_2_state, empty_init, ROT0, "Game Plan", "Sharpshooter (Game Plan)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME( 1979, coneyis,   0,        gp_2, gp_1, gp_2_state, empty_init, ROT0, "Game Plan", "Old Coney Island!",        MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME( 1980, lizard,    0,        gp_2, gp_1, gp_2_state, empty_init, ROT0, "Game Plan", "Pinball Lizard",           MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME( 1982, suprnova,  0,        gp_2, gp_1, gp_2_state, empty_init, ROT0, "Game Plan", "Super Nova (Game Plan)",   MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME( 1983, sshootr2,  0,        gp_2, gp_1, gp_2_state, empty_init, ROT0, "Game Plan", "Sharp Shooter II",         MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )

// GP2 dips
GAME( 1981, gwarfare,  0,        gp_2, gp_2, gp_2_state, empty_init, ROT0, "Game Plan", "Global Warfare", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME( 1982, mbossy,    0,        gp_2, gp_2, gp_2_state, empty_init, ROT0, "Game Plan", "Mike Bossy",     MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME( 1984, attila,    0,        gp_2, gp_2, gp_2_state, empty_init, ROT0, "Game Plan", "Attila The Hun", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )

// revolving match
GAME( 1984, agent777,  0,        gp_2, gp_2, gp_2_state, empty_init, ROT0, "Game Plan", "Agents 777",                MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME( 1985, cpthook,   0,        gp_2, gp_2, gp_2_state, empty_init, ROT0, "Game Plan", "Captain Hook",              MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME( 1985, ladyshot,  0,        gp_2, gp_2, gp_2_state, empty_init, ROT0, "Game Plan", "Lady Sharpshooter (set 1)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME( 1985, ladyshota, ladyshot, gp_2, gp_2, gp_2_state, empty_init, ROT0, "Game Plan", "Lady Sharpshooter (set 2)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME( 1985, andromep,  0,        gp_2, gp_2, gp_2_state, empty_init, ROT0, "Game Plan", "Andromeda (set 1)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME( 1985, andromepa, andromep, gp_2, gp_2, gp_2_state, empty_init, ROT0, "Game Plan", "Andromeda (set 2)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME( 1985, cyclopes,  0,        gp_2, gp_2, gp_2_state, empty_init, ROT0, "Game Plan", "Cyclopes (12/85)",  MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME( 1985, cyclopes1, cyclopes, gp_2, gp_2, gp_2_state, empty_init, ROT0, "Game Plan", "Cyclopes (11/85)",  MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
