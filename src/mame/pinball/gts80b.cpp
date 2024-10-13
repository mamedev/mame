// license:BSD-3-Clause
// copyright-holders:Robbbert
/****************************************************************************************************************

PINBALL
Gottlieb System 80B

Same as system 80, except that the displays are 20-digit alphanumeric driven by Rockwell 10939/10941 chips.

The test rom says U4 is faulty. Using MOS6532 fixes this error, but the games ramdomly slam-tilt instead.

PinMAME used for the display character generator.

When asked to enter your initials, use the Advance buttons to select a letter, and the Start button to enter it.

Here are the key codes to enable play: (may need to hit X to start a ball)

Game                 NUM  Start game                                       End ball
--------------------------------------------------------------------------------------------------
Amazon Hunt II       684C 1                                                X
Amazon Hunt III      684D 1                                                X
Bounty Hunter        694  1                                                X
Chicago Cubs         696  1                                                X
Rock                 697  1                                                X
Tag Team Wrestling   698  1 then End and F1                                X
Raven                702  1                                                X
Hollywood Heat       703  1 then jiggle F4 and Del until sound is heard    X (wait for score to count up) then Del
Rock Encore          704  1                                                X
Genesis              705  1 then jiggle F1 and num-                        X then num-
Spring Break         706  1 then F2 and num- and End and Del               X then Del
Gold Wings           707  1 then quote and del and pad+                    Del and quote and num+
Monte Carlo          708  1 then jiggle [ and End                          End
Arena                709  1 then jiggle M and Del                          Del
Victory              710  1 then Del (wait for sound) then num+            Del (wait for sound) then num+
Diamond Lady         711  1 then Del and Quote                             Del then Quote
TX-Sector            712  1 then Del and num+                              Del and num+
Big House            713  1 then jiggle Del and minus                      Del
Robo-War             714  1 then Del and num+                              Del
Excalibur            715  1 then hold F4 and hit Del                       Del
Bad Girls            717  1 then hold F2 and X, hit End, get a message     End
Hot Shots            718  1 then hold F4 and hit Del                       Del
Bone Busters Inc     719  1 then num- and num+                             num+
Night Moves         C101  1 then jiggle quote del num+ [                   num+ then jiggle same keys as before
Master               ---  1 then jiggle F1 and num-                        X then num-
Top Sound            ---  1                                                X

ManilaMatic Games:
- Defender (not dumped)
- Master (working)
- Out Law (not dumped)
- Pinball Shop (not dumped, unknown hw)
- Star King (not dumped, gts80a)
- Top Sound (working)

Status:
- All games are playable
- Various sounds are missing in some games, usually because the cpu concerned runs into the weeds.

ToDo:
- Missing sounds because of program crashes
- bighouse: after game ends, the display freezes. Game keeps running though.

*****************************************************************************************************************/

#include "emu.h"
#include "genpin.h"

#include "gottlieb_a.h"

#include "machine/input_merger.h"

#include "speaker.h"

#include "gts80b.lh"


namespace {

class gts80b_state : public genpin_class
{
public:
	gts80b_state(const machine_config &mconfig, device_type type, const char *tag)
		: genpin_class(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_riot1(*this, "riot1")
		, m_riot2(*this, "riot2")
		, m_riot3(*this, "riot3")
		, m_io_dips(*this, "DSW%d", 0U)
		, m_io_keyboard(*this, "X%d", 0U)
		, m_p3_sound(*this, "p3sound")
		, m_p4_sound(*this, "p4sound")
		, m_p5_sound(*this, "p5sound")
		, m_p6_sound(*this, "p6sound")
		, m_r2_sound(*this, "r2sound")
		, m_digits(*this, "digit%d", 0U)
		, m_io_outputs(*this, "out%d", 0U)
	{ }

	void p0(machine_config &config);  // base config
	void p3(machine_config &config);  // no schematic available
	void p4(machine_config &config);  // same as r2 but bigger roms, no speech
	void p5(machine_config &config);  // same as p4 + YM2151
	void p6(machine_config &config);  // bonebusters
	void r2(machine_config &config);  // r2 (2x ay, spo250, dac)
	void master(machine_config &config);
	DECLARE_INPUT_CHANGED_MEMBER(slam_w);
	void init_s80c() { m_slam_low = true; }

protected:
	virtual void machine_reset() override ATTR_COLD;
	virtual void machine_start() override ATTR_COLD;

private:
	u8 port1a_r();
	u8 port2a_r();
	void port1b_w(u8 data);
	void port2a_w(u8 data);
	void port2b_w(u8 data);
	void port3a_w(u8 data);
	void port3b_w(u8 data);
	void gts80b_map(address_map &map) ATTR_COLD;
	void master_map(address_map &map) ATTR_COLD;

	u8 m_segment = 0U;
	u8 m_lamprow = 0U;
	u8 m_swrow = 0U;
	u8 m_soundex = 0U;
	u8 m_sol_state[9][2]{};
	u8 m_dispcmd = 0U;
	bool m_in_cmd_mode[2]{};
	u8 m_digit[2]{};
	bool m_slam_low = false;

	required_device<m6502_device> m_maincpu;
	required_device<mos6532_device> m_riot1;
	required_device<mos6532_device> m_riot2;
	required_device<mos6532_device> m_riot3;
	required_ioport_array<4> m_io_dips;
	required_ioport_array<9> m_io_keyboard;
	optional_device<gottlieb_sound_p3_device> m_p3_sound;
	optional_device<gottlieb_sound_p4_device> m_p4_sound;
	optional_device<gottlieb_sound_p5_device> m_p5_sound;
	optional_device<gottlieb_sound_p6_device> m_p6_sound;
	optional_device<gottlieb_sound_r2_device> m_r2_sound;
	output_finder<40> m_digits;
	output_finder<57> m_io_outputs;   // 8 solenoids, 1 outhole, 48 lamps
};

void gts80b_state::gts80b_map(address_map &map)
{
	map.global_mask(0xbfff);
	map(0x0000, 0x007f).mirror(0x8000).m(m_riot1, FUNC(mos6532_device::ram_map));
	map(0x0080, 0x00ff).mirror(0x8000).m(m_riot2, FUNC(mos6532_device::ram_map));
	map(0x0100, 0x017f).mirror(0x8000).m(m_riot3, FUNC(mos6532_device::ram_map));
	map(0x01cb, 0x01cb).lr8(NAME([] () { return 0xff; }));  // continual read
	map(0x0200, 0x021f).mirror(0x8060).m(m_riot1, FUNC(mos6532_device::io_map));
	map(0x0280, 0x029f).mirror(0x8060).m(m_riot2, FUNC(mos6532_device::io_map));
	map(0x0300, 0x031f).mirror(0x8060).m(m_riot3, FUNC(mos6532_device::io_map));
	map(0x1000, 0x17ff).rom();
	map(0x1800, 0x18ff).mirror(0x8000).ram().share("nvram"); // 5101L-1 256x4
	map(0x2000, 0x2fff).rom();
	map(0x3000, 0x3fff).mirror(0x8000).rom();
	map(0x9000, 0x97ff).rom();
}

// for mmmaster, a guess, seems to work
void gts80b_state::master_map(address_map &map)
{
	map(0x0000, 0x7fff).mirror(0x8000).rom();
	map(0x0000, 0x007f).mirror(0x8000).m(m_riot1, FUNC(mos6532_device::ram_map));
	map(0x0080, 0x00ff).mirror(0x8000).m(m_riot2, FUNC(mos6532_device::ram_map));
	map(0x0100, 0x017f).mirror(0x8000).m(m_riot3, FUNC(mos6532_device::ram_map));
	map(0x01cb, 0x01cb).lr8(NAME([] () { return 0xff; }));  // continual read
	map(0x0200, 0x021f).mirror(0x8060).m(m_riot1, FUNC(mos6532_device::io_map));
	map(0x0280, 0x029f).mirror(0x8060).m(m_riot2, FUNC(mos6532_device::io_map));
	map(0x0300, 0x031f).mirror(0x8060).m(m_riot3, FUNC(mos6532_device::io_map));
	map(0x1800, 0x18ff).mirror(0x8000).ram().share("nvram"); // 5101L-1 256x4
}


static INPUT_PORTS_START( gts80b )
	PORT_START("DSW0")
	PORT_DIPNAME( 0x80, 0x80, "SW 01")
	PORT_DIPSETTING(    0x00, DEF_STR(On))
	PORT_DIPSETTING(    0x80, DEF_STR(Off))
	PORT_DIPNAME( 0x40, 0x40, "SW 02")
	PORT_DIPSETTING(    0x00, DEF_STR(On))
	PORT_DIPSETTING(    0x40, DEF_STR(Off))
	PORT_DIPNAME( 0x20, 0x20, "SW 03")
	PORT_DIPSETTING(    0x00, DEF_STR(On))
	PORT_DIPSETTING(    0x20, DEF_STR(Off))
	PORT_DIPNAME( 0x10, 0x10, "SW 04")
	PORT_DIPSETTING(    0x00, DEF_STR(On))
	PORT_DIPSETTING(    0x10, DEF_STR(Off))
	PORT_DIPNAME( 0x08, 0x08, "SW 05")
	PORT_DIPSETTING(    0x00, DEF_STR(On))
	PORT_DIPSETTING(    0x08, DEF_STR(Off))
	PORT_DIPNAME( 0x04, 0x04, "SW 06")
	PORT_DIPSETTING(    0x00, DEF_STR(On))
	PORT_DIPSETTING(    0x04, DEF_STR(Off))
	PORT_DIPNAME( 0x02, 0x02, "SW 07")
	PORT_DIPSETTING(    0x00, DEF_STR(On))
	PORT_DIPSETTING(    0x02, DEF_STR(Off))
	PORT_DIPNAME( 0x01, 0x01, "SW 08")
	PORT_DIPSETTING(    0x00, DEF_STR(On))
	PORT_DIPSETTING(    0x01, DEF_STR(Off))

	PORT_START("DSW1")
	PORT_DIPNAME( 0x80, 0x80, "SW 09")
	PORT_DIPSETTING(    0x00, DEF_STR(On))
	PORT_DIPSETTING(    0x80, DEF_STR(Off))
	PORT_DIPNAME( 0x40, 0x40, "SW 10")
	PORT_DIPSETTING(    0x00, DEF_STR(On))
	PORT_DIPSETTING(    0x40, DEF_STR(Off))
	PORT_DIPNAME( 0x20, 0x20, "SW 11")
	PORT_DIPSETTING(    0x00, DEF_STR(On))
	PORT_DIPSETTING(    0x20, DEF_STR(Off))
	PORT_DIPNAME( 0x10, 0x10, "SW 12")
	PORT_DIPSETTING(    0x00, DEF_STR(On))
	PORT_DIPSETTING(    0x10, DEF_STR(Off))
	PORT_DIPNAME( 0x08, 0x08, "SW 13")
	PORT_DIPSETTING(    0x00, DEF_STR(On))
	PORT_DIPSETTING(    0x08, DEF_STR(Off))
	PORT_DIPNAME( 0x04, 0x00, "SW 14")
	PORT_DIPSETTING(    0x00, DEF_STR(On))
	PORT_DIPSETTING(    0x04, DEF_STR(Off))
	PORT_DIPNAME( 0x02, 0x00, "SW 15")
	PORT_DIPSETTING(    0x00, DEF_STR(On))
	PORT_DIPSETTING(    0x02, DEF_STR(Off))
	PORT_DIPNAME( 0x01, 0x00, "SW 16")
	PORT_DIPSETTING(    0x00, DEF_STR(On))
	PORT_DIPSETTING(    0x01, DEF_STR(Off))

	PORT_START("DSW2")
	PORT_DIPNAME( 0x80, 0x80, "SW 17")
	PORT_DIPSETTING(    0x00, DEF_STR(On))
	PORT_DIPSETTING(    0x80, DEF_STR(Off))
	PORT_DIPNAME( 0x40, 0x40, "SW 18")
	PORT_DIPSETTING(    0x00, DEF_STR(On))
	PORT_DIPSETTING(    0x40, DEF_STR(Off))
	PORT_DIPNAME( 0x20, 0x20, "SW 19")
	PORT_DIPSETTING(    0x00, DEF_STR(On))
	PORT_DIPSETTING(    0x20, DEF_STR(Off))
	PORT_DIPNAME( 0x10, 0x10, "SW 20")
	PORT_DIPSETTING(    0x00, DEF_STR(On))
	PORT_DIPSETTING(    0x10, DEF_STR(Off))
	PORT_DIPNAME( 0x08, 0x08, "SW 21")
	PORT_DIPSETTING(    0x00, DEF_STR(On))
	PORT_DIPSETTING(    0x08, DEF_STR(Off))
	PORT_DIPNAME( 0x04, 0x04, "SW 22")
	PORT_DIPSETTING(    0x00, DEF_STR(On))
	PORT_DIPSETTING(    0x04, DEF_STR(Off))
	PORT_DIPNAME( 0x02, 0x00, "SW 23")
	PORT_DIPSETTING(    0x00, DEF_STR(On))
	PORT_DIPSETTING(    0x02, DEF_STR(Off))
	PORT_DIPNAME( 0x01, 0x00, "SW 24")
	PORT_DIPSETTING(    0x00, DEF_STR(On))
	PORT_DIPSETTING(    0x01, DEF_STR(Off))

	PORT_START("DSW3")
	PORT_DIPNAME( 0x80, 0x00, "SW 25")
	PORT_DIPSETTING(    0x00, DEF_STR(On))
	PORT_DIPSETTING(    0x80, DEF_STR(Off))
	PORT_DIPNAME( 0x40, 0x00, "SW 26")
	PORT_DIPSETTING(    0x00, DEF_STR(On))
	PORT_DIPSETTING(    0x40, DEF_STR(Off))
	PORT_DIPNAME( 0x20, 0x20, "SW 27")
	PORT_DIPSETTING(    0x00, DEF_STR(On))
	PORT_DIPSETTING(    0x20, DEF_STR(Off))
	PORT_DIPNAME( 0x10, 0x10, "SW 28")
	PORT_DIPSETTING(    0x00, DEF_STR(On))
	PORT_DIPSETTING(    0x10, DEF_STR(Off))
	PORT_DIPNAME( 0x08, 0x08, "SW 29")
	PORT_DIPSETTING(    0x00, DEF_STR(On))
	PORT_DIPSETTING(    0x08, DEF_STR(Off))
	PORT_DIPNAME( 0x04, 0x04, "SW 30")
	PORT_DIPSETTING(    0x00, DEF_STR(On))
	PORT_DIPSETTING(    0x04, DEF_STR(Off))
	PORT_DIPNAME( 0x02, 0x02, "SW 31")
	PORT_DIPSETTING(    0x00, DEF_STR(On))
	PORT_DIPSETTING(    0x02, DEF_STR(Off))
	PORT_DIPNAME( 0x01, 0x01, "SW 32")
	PORT_DIPSETTING(    0x00, DEF_STR(On))
	PORT_DIPSETTING(    0x01, DEF_STR(Off))

	PORT_START("X0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_A) PORT_NAME("INP00")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_B) PORT_NAME("INP01")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_C) PORT_NAME("INP02")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_D) PORT_NAME("INP03")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_E) PORT_NAME("INP04")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_F) PORT_NAME("INP05")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_1_PAD) PORT_NAME("Left Advance Button")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_0_PAD) PORT_NAME("Play/Test")

	PORT_START("X1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_G) PORT_NAME("INP10")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_H) PORT_NAME("INP11")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_I) PORT_NAME("INP12")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_J) PORT_NAME("INP13")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_K) PORT_NAME("INP14")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_L) PORT_NAME("INP15")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_2_PAD) PORT_NAME("Right Advance Button")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN1 )

	PORT_START("X2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_M) PORT_NAME("INP20")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_N) PORT_NAME("INP21")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_O) PORT_NAME("INP22")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_P) PORT_NAME("INP23")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_Q) PORT_NAME("INP24")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_R) PORT_NAME("INP25")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_S) PORT_NAME("INP26")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN2 )

	PORT_START("X3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_T) PORT_NAME("INP30")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_U) PORT_NAME("INP31")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_V) PORT_NAME("INP32")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_W) PORT_NAME("INP33")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_Y) PORT_NAME("INP34")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_Z) PORT_NAME("INP35")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_MINUS) PORT_NAME("INP36")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN3 )

	PORT_START("X4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_EQUALS) PORT_NAME("INP40")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_BACKSPACE) PORT_NAME("INP41")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_OPENBRACE) PORT_NAME("INP42")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_CLOSEBRACE) PORT_NAME("INP43")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_BACKSLASH) PORT_NAME("INP44")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_COLON) PORT_NAME("INP45")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_QUOTE) PORT_NAME("INP46")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START )

	PORT_START("X5")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_ENTER) PORT_NAME("INP50")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_STOP) PORT_NAME("INP51")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_COMMA) PORT_NAME("INP52")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_SLASH) PORT_NAME("INP53")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_ASTERISK) PORT_NAME("INP54")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_MINUS_PAD) PORT_NAME("INP55")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_PLUS_PAD) PORT_NAME("INP56")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_9) PORT_NAME("Tilt") // won't boot if closed

	PORT_START("X6")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_SLASH_PAD) PORT_NAME("INP60")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_ENTER_PAD) PORT_NAME("INP61")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_PGUP) PORT_NAME("INP62")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_PGDN) PORT_NAME("INP63")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_HOME) PORT_NAME("INP64")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_END) PORT_NAME("INP65")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_DEL) PORT_NAME("INP66")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_X) PORT_NAME("Outhole")

	PORT_START("X7")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_UP) PORT_NAME("INP70")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_LEFT) PORT_NAME("INP71")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_RIGHT) PORT_NAME("INP72")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_DOWN) PORT_NAME("INP73")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_F1) PORT_NAME("INP74")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_F2) PORT_NAME("INP75")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_F4) PORT_NAME("INP76")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("X8")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_0) PORT_NAME("Slam") PORT_CHANGED_MEMBER(DEVICE_SELF, gts80b_state, slam_w, 0)
INPUT_PORTS_END

INPUT_CHANGED_MEMBER( gts80b_state::slam_w )
{
	u8 val = m_slam_low ? 0 : 1;
	m_riot2->pa_bit_w<7>(newval ? val : val^1);
}

static const uint16_t patterns[] = {
	/* 0x00-0x07 */ 0x0000, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff,
	/* 0x08-0x0f */ 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff,
	/* 0x10-0x17 */ 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff,
	/* 0x18-0x1f */ 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff,
	/* 0x20-0x27 */ 0x0000, 0x0309, 0x0220, 0x2A4E, 0x2A6D, 0x6E65, 0x135D, 0x0400,
	/* 0x28-0x2f */ 0x1400, 0x4100, 0x7F40, 0x2A40, 0x0000, 0x0840, 0x0000, 0x4400,
	/* 0x30-0x37 */ 0x003f, 0x2200, 0x085B, 0x084f, 0x0866, 0x086D, 0x087D, 0x0007,
	/* 0x38-0x3f */ 0x087F, 0x086F, 0x0009, 0x4001, 0x4408, 0x0848, 0x1108, 0x2803,
	/* 0x40-0x47 */ 0x205F, 0x0877, 0x2A0F, 0x0039, 0x220F, 0x0079, 0x0071, 0x083D,
	/* 0x48-0x4f */ 0x0876, 0x2209, 0x001E, 0x1470, 0x0038, 0x0536, 0x1136, 0x003f,
	/* 0x50-0x57 */ 0x0873, 0x103F, 0x1873, 0x086D, 0x2201, 0x003E, 0x4430, 0x5036,
	/* 0x58-0x5f */ 0x5500, 0x2500, 0x4409, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff,
	/* 0x60-0x67 */ 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff,
	/* 0x68-0x6f */ 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff,
	/* 0x70-0x77 */ 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff,
	/* 0x78-0x7f */ 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff,
};

u8 gts80b_state::port1a_r()
{
	u8 data = 0xff;
	if ((m_lamprow < 4) && BIT(m_segment, 7))
		data = m_io_dips[m_lamprow]->read();

	for (u8 i = 0; i < 8; i++)
		if (!BIT(m_swrow, i))
			data &= m_io_keyboard[i]->read();

	return data ^ 0xff;  // inverted by Z14 (7400)
}

u8 gts80b_state::port2a_r()
{
	u8 val = m_slam_low ? 0x80 : 0;
	return m_io_keyboard[8]->read() ^ val; // slam tilt
}

// sw strobes
void gts80b_state::port1b_w(u8 data)
{
	m_swrow = data ^ 0xff;  // inverted by Z11 (7404)
}

void gts80b_state::port2a_w(u8 data)
{
	if (BIT(data, 4))
		m_dispcmd = (m_dispcmd & 0xf0) | BIT(m_segment, 0, 4);
	if (BIT(data, 5))
		m_dispcmd = (m_dispcmd & 0x0f) | (m_segment << 4);
}

//d0-3 data; d4-5 = which display enabled; d6 = display reset; d7 = dipsw enable
void gts80b_state::port2b_w(u8 data)
{
	m_segment = data;
	uint16_t segment;

	// crude approximation of the Rockwell display chips
	for (u8 i = 0; i < 2; i++) // 2 chips
	{
		if (!BIT(data, i+4)) // are we addressing the chip?
		{
			if (m_in_cmd_mode[i]) // in command mode?
			{
				if ((m_dispcmd >= 0xc0) && (m_dispcmd < 0xd4)) // we only support one command
					m_digit[i] = data & 0x1f;
				m_in_cmd_mode[i] = false;
			}
			else
			if (m_dispcmd == 1) // 01 = enter command mode
			{
				m_in_cmd_mode[i] = true;
			}
			else
			{ // display a character
				segment = patterns[m_dispcmd & 0x7f]; // ignore blank/inverse bit
				segment = bitswap<16>(segment, 12, 10, 8, 14, 13, 9, 11, 6, 5, 4, 3, 3, 2, 1, 0, 0);
				m_digits[m_digit[i]+i*20] = segment;
				m_digit[i]++; // auto-increment pointer
				if (m_digit[i] > 19) m_digit[i] = 0; // check for overflow
			}
		}
	}
}

// solenoids and sound
void gts80b_state::port3a_w(u8 data)
{
	u8 i;
	data ^= 0x1f;   // Z27 inverter
	// Sound
	u8 sndcmd = data & 15;
	if (!BIT(data, 4))  // Z31
		sndcmd = 0;

	sndcmd ^= 15;  // inverted again by Z13 on the A3 board
	if (sndcmd == 15)
		m_soundex = 0xf0;
	if (m_r2_sound)
		m_r2_sound->write(sndcmd | m_soundex);
	else
	if (m_p3_sound)
		m_p3_sound->write(sndcmd);
	else
	if (m_p4_sound)
		m_p4_sound->write(sndcmd | m_soundex);
	else
	if (m_p5_sound)
		m_p5_sound->write(sndcmd | m_soundex);
	else
	if (m_p6_sound)
		m_p6_sound->write(sndcmd | m_soundex);

	// Solenoids group 1
	if (!BIT(data, 5))
		for (i = 0; i < 4;i++)
			m_sol_state[i][0] = (BIT(data, 0, 2) == i) ? 1 : 0;
	else
		for (i = 0; i < 4;i++)
			m_sol_state[i][0] = 0;

	// Solenoids group 2
	if (!BIT(data, 6))
		for (i = 0; i < 4;i++)
			m_sol_state[i+4][0] = (BIT(data, 2, 2) == i) ? 1 : 0;
	else
		for (i = 4; i < 8;i++)
			m_sol_state[i][0] = 0;

	// Outhole
	m_sol_state[8][0] = BIT(data, 7) ^ 1;

	// Smooth solenoids
	// Some solenoids get continuously pulsed, which is absorbed by the real thing, but
	// causes issues for us. So we need to use only the first occurrence of a particular sound.
	for (i = 0; i < 9; i++)
	{
		switch (m_sol_state[i][1])
		{
			case 0:   // was off
				if (m_sol_state[i][0] == 1) // was off, coming on
				{
					m_io_outputs[i] = 1;
					m_sol_state[i][1] = 1;  // remember
				}
				break;
			case 1:   // was on
				if (m_sol_state[i][0] == 0) // was on, going off
				{
					m_io_outputs[i] = 0;
					m_sol_state[i][1] = 0;  // remember
				}
				else
					m_io_outputs[i] = 0;  // still on from before
				break;
			default:
				m_sol_state[i][1] = 0;
				break;
		}
	}
	// Activate solenoids
	for (i = 0; i < 9; i++)
	{
		bool state = m_io_outputs[i] ? 1 : 0;
		switch (i)
		{
			case 2:
				machine().bookkeeping().coin_counter_w(0, state);
				break;
			case 3:
				machine().bookkeeping().coin_counter_w(1, state);
				break;
			case 6:
				machine().bookkeeping().coin_counter_w(2, state);
				break;
			case 7:
				if (state)
					m_samples->start(0, 6);  // knocker
				break;
			case 8:
				if (state)
					m_samples->start(0, 9);  // outhole
				break;
			default:
				break;
		}
	}
}

// Lamps
void gts80b_state::port3b_w(u8 data)
{
	m_lamprow = BIT(data, 4, 4);
	if (m_lamprow && (m_lamprow < 13))
		for (u8 i = 0; i < 4; i++)
			m_io_outputs[m_lamprow*4+i+5] = BIT(data, i);
	// confirmed with Hollywood Heat - ball must start with siren
	m_soundex = m_io_outputs[13] ? 0xe0 : 0xf0;  // r2 sound16
}

void gts80b_state::machine_start()
{
	m_digits.resolve();
	m_io_outputs.resolve();

	save_item(NAME(m_lamprow));
	save_item(NAME(m_swrow));
	save_item(NAME(m_segment));
	save_item(NAME(m_soundex));
	save_item(NAME(m_sol_state));
	save_item(NAME(m_dispcmd));
	save_item(NAME(m_in_cmd_mode));
	save_item(NAME(m_digit));
}

void gts80b_state::machine_reset()
{
	for (u8 i = 0; i < m_io_outputs.size(); i++)
		m_io_outputs[i] = 0;

	m_lamprow = 0;
	m_swrow = 0;
	m_segment = 0;
	m_soundex = 0;
	m_in_cmd_mode[0] = false;
	m_in_cmd_mode[1] = false;
	m_dispcmd = 0;
	m_digit[0] = 0;
	m_digit[1] = 0;
}

/* with Sound Board */
void gts80b_state::p0(machine_config &config)
{
	/* basic machine hardware */
	M6502(config, m_maincpu, XTAL(3'579'545)/4);
	m_maincpu->set_addrmap(AS_PROGRAM, &gts80b_state::gts80b_map);

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	/* Video */
	config.set_default_layout(layout_gts80b);

	/* Devices */
	MOS6532(config, m_riot1, XTAL(3'579'545)/4);
	m_riot1->pa_rd_callback().set(FUNC(gts80b_state::port1a_r)); // sw_r
	m_riot1->pb_wr_callback().set(FUNC(gts80b_state::port1b_w)); // sw_w
	m_riot1->irq_wr_callback().set("irq", FUNC(input_merger_device::in_w<0>));

	MOS6532(config, m_riot2, XTAL(3'579'545)/4);
	m_riot2->pa_rd_callback().set(FUNC(gts80b_state::port2a_r)); // pa7 - slam tilt
	m_riot2->pa_wr_callback().set(FUNC(gts80b_state::port2a_w)); // digit select
	m_riot2->pb_wr_callback().set(FUNC(gts80b_state::port2b_w)); // seg
	m_riot2->irq_wr_callback().set("irq", FUNC(input_merger_device::in_w<1>));

	MOS6532(config, m_riot3, XTAL(3'579'545)/4);
	m_riot3->pa_wr_callback().set(FUNC(gts80b_state::port3a_w)); // sol, snd
	m_riot3->pb_wr_callback().set(FUNC(gts80b_state::port3b_w)); // lamps
	m_riot3->irq_wr_callback().set("irq", FUNC(input_merger_device::in_w<2>));

	INPUT_MERGER_ANY_HIGH(config, "irq").output_handler().set_inputline("maincpu", m6502_device::IRQ_LINE);

	/* Sound */
	genpin_audio(config);
	SPEAKER(config, "mono").front_center();
}

void gts80b_state::p3(machine_config &config)
{
	p0(config);
	GOTTLIEB_SOUND_PIN3(config, m_p3_sound).add_route(ALL_OUTPUTS, "mono", 1.00);
}

void gts80b_state::p4(machine_config &config)
{
	p0(config);
	GOTTLIEB_SOUND_PIN4(config, m_p4_sound).add_route(ALL_OUTPUTS, "mono", 1.00);
}

void gts80b_state::p5(machine_config &config)
{
	p0(config);
	GOTTLIEB_SOUND_PIN5(config, m_p5_sound).add_route(ALL_OUTPUTS, "mono", 1.00);
}

void gts80b_state::p6(machine_config &config)
{
	p0(config);
	GOTTLIEB_SOUND_PIN6(config, m_p6_sound).add_route(ALL_OUTPUTS, "mono", 1.00);
}

void gts80b_state::r2(machine_config &config)
{
	p0(config);
	GOTTLIEB_SOUND_REV2(config, m_r2_sound).add_route(ALL_OUTPUTS, "mono", 1.00);
}

void gts80b_state::master(machine_config &config)
{
	r2(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &gts80b_state::master_map);
}


/* SYSTEM-80B ALTERNATE ROMS =======================================================================

This is a list of known alternate roms. Nothing has been tested. Comments are derived from the roms.
Loading address is not shown. In some cases, suffix X = French, suffix Y = German, no suffix = English.
The majority of these are freeplay roms from flipprojets.

Amazon Hunt II
ROM_LOAD( "prom1_684c.bin", 0x000000, 0x002000, CRC(fe08af7d) SHA1(54ea8205f649c9ab8a62354b023acf823c24fc1f) )

Amazon Hunt III
ROM_LOAD( "prom1_684d.bin", 0x000000, 0x002000, CRC(7db1a9a9) SHA1(41927730ae4571d0a0488baee4d10b756524440e) )
ROM_LOAD( "684d-1-cpu_fp.rom", 0x000000, 0x002000, CRC(6110c1f0) SHA1(e478510b02fa7d26c0c619f2bfdadd1503a1f6d1) )

Arena
ROM_LOAD( "prom1_709.bin", 0x000000, 0x002000, CRC(33ade406) SHA1(daea04b9ccd95b2e4ee3d45ad8ea6e1851cf8cc6) )
ROM_LOAD( "prom2_709.bin", 0x000000, 0x000800, CRC(4783b689) SHA1(d10d4cbf8d00c9d0db57cdac32ef96498275eea6) )
ROM_LOAD( "prom1_709x.bin", 0x000000, 0x002000, CRC(7389f4dc) SHA1(3dc72f011ebb2debbc005761da4b9720a279db2a) )
ROM_LOAD( "prom2_709x.bin", 0x000000, 0x000800, CRC(49b127d8) SHA1(0436f83e969b4bfc7edaf881bf7556a868c88cdc) )
ROM_LOAD( "prom1_709y.bin", 0x000000, 0x002000, CRC(d41ce2b1) SHA1(044fd0bcabb317d0fa84ff17036f6dba90201cbf) )
ROM_LOAD( "prom2_709y.bin", 0x000000, 0x000800, CRC(e170d1cd) SHA1(bd7919eb9e480309f794ac25a371c7b818dcd01b) )
ROM_LOAD( "prom1_709a.bin", 0x000000, 0x002000, CRC(21f500cb) SHA1(ce76a8a5eb71e57aca70880a2e40979f083173ef) )
ROM_LOAD( "prom2_709a.bin", 0x000000, 0x000800, CRC(13c8813b) SHA1(756e3583fd55b72e0bfb15e9b4a60740b389ca2e) )

Bad Girls
ROM_LOAD( "prom1_717.bin", 0x000000, 0x002000, CRC(05e8259b) SHA1(d1e4e50e44e215dcfa510e4d45d6c39e136452b1) )
ROM_LOAD( "prom2_717.bin", 0x000000, 0x001000, CRC(583933ec) SHA1(89da6750d779d68db578715b058f9321695b79b0) )
ROM_LOAD( "prom1_717x.bin", 0x000000, 0x002000, CRC(32f42091) SHA1(0709f251b5633a68a93066721d105141fb79d74a) )
ROM_LOAD( "prom2_717x.bin", 0x000000, 0x001000, CRC(58c35099) SHA1(ff76bd28175ea0f5d0437c16c5ae6886339edfe2) )
ROM_LOAD( "prom1_717y.bin", 0x000000, 0x002000, CRC(34a93b4b) SHA1(14522c5c1c476d5507100d3554db6c2236d48df3) )
ROM_LOAD( "prom2_717y.bin", 0x000000, 0x001000, CRC(55aa30ac) SHA1(9544485ccf52a2ad51a00cce0c12871db099699f) )

Big House
ROM_LOAD( "prom1_713.bin", 0x000000, 0x002000, CRC(8a8510a2) SHA1(729a7254d00fee7c4aa29684e944df4eab113565) )
ROM_LOAD( "prom2_713.bin", 0x000000, 0x001000, CRC(047c8ef5) SHA1(3afa2a0011b724836b69b2ef386597e0953dfadf) )
ROM_LOAD( "prom1_713x.bin", 0x000000, 0x002000, CRC(51012001) SHA1(5ad45694273234b2d13028b90c2a58245394095e) )
ROM_LOAD( "prom2_713x.bin", 0x000000, 0x001000, CRC(767efc44) SHA1(6b8f9a580e6a6ad92c9efe9f4345496d5063b7a8) )
ROM_LOAD( "prom1_713y.bin", 0x000000, 0x002000, CRC(8f51d4c3) SHA1(972582aedfdbccd7a14d841b4ec156ab73e8c88f) )
ROM_LOAD( "prom2_713y.bin", 0x000000, 0x001000, CRC(214f0afb) SHA1(9874773e4ffa2472e78d42dfa9e21a621bf7b49e) )

Bone Busters
ROM_LOAD( "prom1_719.bin", 0x000000, 0x002000, CRC(c5914b1f) SHA1(599a6da358d294304e07425fdde3f1ece0f4f57a) )
ROM_LOAD( "prom2_719.bin", 0x000000, 0x001000, CRC(681643df) SHA1(76af6951e4403b4951298d35a9058bcebfa6bc43) )
ROM_LOAD( "prom1_719x.bin", 0x000000, 0x002000, CRC(5aa1bb17) SHA1(6499608e83261c1fd152e9bf982ce1470b6edf93) )
ROM_LOAD( "prom2_719x.bin", 0x000000, 0x001000, CRC(73b6486e) SHA1(1baf17f31b16d564ed5e3bdf9f74b21f83ed76fa) )
ROM_LOAD( "prom1_719y.bin", 0x000000, 0x002000, CRC(0063411b) SHA1(8bf7350acff3ac7d76ed2dee42aceee1de486497) )
ROM_LOAD( "prom2_719y.bin", 0x000000, 0x001000, CRC(3b85c8bd) SHA1(5c99349dc3ae05b82932d6ec9d2d1a29c2a7e36d) )

Bounty Hunter
ROM_LOAD( "prom1_694.bin", 0x000000, 0x002000, CRC(9e68b714) SHA1(bb33b1e8fb50776731c450e2c05c49dcd5535f41) )
ROM_LOAD( "prom1_694y.bin", 0x000000, 0x002000, CRC(07b9333f) SHA1(1355201be26ac8f7bca96275443e33c9a01eedf3) )

Chicago Cubs Triple Play
ROM_LOAD( "prom1_f1.cpu", 0x000000, 0x002000, CRC(4b58be44) SHA1(db7734692b3ff158cbd229b2d3ca723cfe963c7b) )
ROM_LOAD( "prom1_fp.cpu", 0x000000, 0x002000, CRC(521946d4) SHA1(527ed3f221e0ca5fe1778e3095c9b8a414911206) )
ROM_LOAD( "prom1_696.bin", 0x000000, 0x002000, CRC(4bc2371f) SHA1(fea37984e7b7833ea1fac3f3a82758c87b6d629c) )
ROM_LOAD( "prom1_696y.bin", 0x000000, 0x002000, CRC(7b6e6819) SHA1(ca2a739301a8be1ff7dd139171cd28f29d5aad59) )

Diamond Lady
ROM_LOAD( "prom1_711.bin", 0x000000, 0x002000, CRC(eef4da86) SHA1(74b274adfddc29fed91d00af52fc4e477b571fe8) )
ROM_LOAD( "prom2_711.bin", 0x000000, 0x000800, CRC(862951dc) SHA1(b15899ecf7ec869e3722cef3f5c16b0dadd2514e) )
ROM_LOAD( "prom2_711x.bin", 0x000000, 0x000800, CRC(943019a8) SHA1(558c3696339bb6e150b4ddb499bc60897d5954ec) )
ROM_LOAD( "prom1_711x.bin", 0x000000, 0x002000, CRC(1d7feafd) SHA1(70f02157fcd94ff7b66750054b542642a3a051b2) )
ROM_LOAD( "prom1_711y.bin", 0x000000, 0x002000, CRC(4a070002) SHA1(80f1b2bd36c7133d92a35fb995cf268ff4259e86) )
ROM_LOAD( "prom2_711y.bin", 0x000000, 0x000800, CRC(f0ef69f6) SHA1(1f48bb656bb20073e2ff261199cb94919f0bb2ab) )

Excalibur
ROM_LOAD( "prom1_715.bin", 0x000000, 0x002000, CRC(86cf464b) SHA1(c857187e6f3dd1f5b5013c95d3ded8a9a5a2e485) )
ROM_LOAD( "prom2_715.bin", 0x000000, 0x001000, CRC(082d64ab) SHA1(0eae3b549839fc281d2487d483d0b4e723ebdc48) )
ROM_LOAD( "prom1_715x.bin", 0x000000, 0x002000, CRC(65601620) SHA1(d8a0f13618f5af4954e0079890ad1ce6ae490d57) )
ROM_LOAD( "prom2_715x.bin", 0x000000, 0x001000, CRC(499e2e41) SHA1(1e3fcba18882bd7df30a43843916aa5d7968eecc) )
ROM_LOAD( "prom1_715y.bin", 0x000000, 0x002000, CRC(76d20188) SHA1(fd822702a6ad880b88f88886b752d7a1087095fd) )
ROM_LOAD( "prom2_715y.bin", 0x000000, 0x001000, CRC(49079396) SHA1(92361a87464e39afeb74fe531b7d4356323405b8) )

Genesis
ROM_LOAD( "prom1_705.bin", 0x000000, 0x002000, CRC(662722b1) SHA1(1a7cf2f6cf92b5e6a288272ff785f215b241842f) )
ROM_LOAD( "prom2_705.bin", 0x000000, 0x000800, CRC(ac9f3a0f) SHA1(0e44888dc046121794e824d128628f991245c1cb) )
ROM_LOAD( "prom1_705x.bin", 0x000000, 0x002000, CRC(4d94f012) SHA1(8da4793345365330d13873edee9ffded173ed935) )
ROM_LOAD( "prom2_705x.bin", 0x000000, 0x000800, CRC(ea7f824f) SHA1(45f619153e0584cffd33e6e09e6f5a97ab9522b2) )
ROM_LOAD( "prom1_705y.bin", 0x000000, 0x002000, CRC(24af8cef) SHA1(4b54f5ed32afc11bf3dc8b16e046add6ddbf93ab) )
ROM_LOAD( "prom2_705y.bin", 0x000000, 0x000800, CRC(e8fc30af) SHA1(2401bff3cf566cae4e6de6167fa004c5fe232928) )

Gold Wings
ROM_LOAD( "prom1_707.bin", 0x000000, 0x002000, CRC(58f19602) SHA1(b17c7aadcb314e6639446ed08de7666f5ea3dd66) )
ROM_LOAD( "prom2_707.bin", 0x000000, 0x000800, CRC(a5318c20) SHA1(8b4dcf45b13657ff753237a2e7d0352fda7755ef) )
ROM_LOAD( "prom1_707x.bin", 0x000000, 0x002000, CRC(90dd07b7) SHA1(0058812c0ba94e4bb62579e84bc3f61918d2e6ab) )
ROM_LOAD( "prom2_707x.bin", 0x000000, 0x000800, CRC(50337adf) SHA1(dc286d52e6872edd68af442cbd0442babc174b93) )
ROM_LOAD( "prom1_707y.bin", 0x000000, 0x002000, CRC(912c5086) SHA1(ecd8d42ebc0840098b9ee3a6b9fe8fde4cb1467f) )
ROM_LOAD( "prom2_707y.bin", 0x000000, 0x000800, CRC(f69c963c) SHA1(9e39344ecfcca1115e12c559c66eaa21716c0ce2) )

Hollywood Heat
ROM_LOAD( "prom1_703.bin", 0x000000, 0x002000, CRC(63bc8395) SHA1(b4007b5a6b78e162c9b3e0243f01fa30501323ae) )
ROM_LOAD( "prom2_703.bin", 0x000000, 0x000800, CRC(a465e5f3) SHA1(56afa2f67aebcd17345bba76ecb814653719ee7b) )
ROM_LOAD( "prom1_703x.bin", 0x000000, 0x002000, CRC(9b7df518) SHA1(02c649370c3424929813dcf8321bcc5f8cc85c88) )
ROM_LOAD( "prom2_703x.bin", 0x000000, 0x000800, CRC(969ca81f) SHA1(2606a0f63434056c5d2b509a885c9919a7a5d70f) )
ROM_LOAD( "prom1_703y.bin", 0x000000, 0x002000, CRC(11fa2432) SHA1(c08a7481d5d2f74ead9de1b0c8816d3dbf321f0f) )
ROM_LOAD( "prom2_703y.bin", 0x000000, 0x000800, CRC(bf60b631) SHA1(944089895d4253dd094a8f6b7168f9e62a75568a) )

Hot Shots
ROM_LOAD( "prom1_718.bin", 0x000000, 0x002000, CRC(74c0f7d7) SHA1(7af87d03fb604e7192ff4ee7581c034e8b2556da) )
ROM_LOAD( "prom2_718.bin", 0x000000, 0x001000, CRC(7695c7db) SHA1(90188ff83b888262ba849e5af9d99145c5bc1c30) )
ROM_LOAD( "prom2_718x.bin", 0x000000, 0x001000, CRC(476e260c) SHA1(2b88920c77462d190f9b98aebf8fcb5c9e853ecd) )
ROM_LOAD( "prom1_718x.bin", 0x000000, 0x002000, CRC(dedba56b) SHA1(b9b435173f1325e57532c7001777dec862213d97) )
ROM_LOAD( "prom1_718y.bin", 0x000000, 0x002000, CRC(dca0b300) SHA1(d7473ea1398dff2bd861d4b49b0cee2764599b34) )
ROM_LOAD( "prom2_718y.bin", 0x000000, 0x001000, CRC(7e2f0d59) SHA1(b8a7b9be3e4d705631e017da87b27be53ed23f30) )

Monte Carlo
ROM_LOAD( "prom1_708.bin", 0x000000, 0x002000, CRC(5b703cf8) SHA1(dfb3eb886675989a1bbae7a2581e522869d81392) )
ROM_LOAD( "prom2_708.bin", 0x000000, 0x000800, CRC(6860e315) SHA1(cecb1815334506dfebf29efe3e4e2a838010e8db) )
ROM_LOAD( "prom1_708x.bin", 0x000000, 0x002000, CRC(dbe0f749) SHA1(2a1fc7606dbc99ac534901ed91943d6dd49bd4e2) )
ROM_LOAD( "prom2_708x.bin", 0x000000, 0x000800, CRC(f6842631) SHA1(7447994d2055c7fa12aaf35e93436ee829f5b7ae) )
ROM_LOAD( "prom1_708y.bin", 0x000000, 0x002000, CRC(bcf93933) SHA1(846c7d7c1da7516dbe0d19b4fc87eecfb69b13c1) )
ROM_LOAD( "prom2_708y.bin", 0x000000, 0x000800, CRC(2a5e0c4f) SHA1(b386168bd911b9977104c47da962d0248f22614b) )
ROM_LOAD( "prom1_708a.bin", 0x000000, 0x002000, CRC(25787b75) SHA1(f8ad7a22018b5414bf1ea412004ee63cb55c2036) )
ROM_LOAD( "prom2_708a.bin", 0x000000, 0x000800, CRC(5dd75c06) SHA1(911f7e56b7602c9bc9b51dde7719d3e0562f0702) )
ROM_LOAD( "prom1_708a.bin", 0x000000, 0x002000, CRC(d47c24ae) SHA1(034ae4515e3ab94b054e85aef96f4376b43a1157) )
ROM_LOAD( "prom2_708a.bin", 0x000000, 0x000800, CRC(8e72a68f) SHA1(8320c44020f7d5f9e887b17556252f1c617235ac) )

Night Moves
ROM_LOAD( "prom1_c101.bin", 0x000000, 0x002000, CRC(1cd28cac) SHA1(304139bcd4d496f913399d9945a46aadf32078f9) )
ROM_LOAD( "prom2_c101.bin", 0x000000, 0x001000, CRC(a2bc00e4) SHA1(5c3e9033f5c72b87058b2f70a0ff0811cc6770fa) )

Raven
ROM_LOAD( "prom1r.cpu",   0x000000, 0x002000, CRC(51629598) SHA1(a5408fad2baec43633f407665f006fae74f3d9aa) )  // Rambo hack
ROM_LOAD( "prom2a.cpu",   0x000000, 0x000800, CRC(a693785e) SHA1(7c8878f1c3c5205b3ae46a78c881bbd2b722838d) )  // Rambo hack
ROM_LOAD( "prom1_702.bin", 0x000000, 0x002000, CRC(d6e5120b) SHA1(1d00bce8170b5ad4185e6517ba1a0f46c8ae7444) )
ROM_LOAD( "prom2_702.bin", 0x000000, 0x000800, CRC(481f3fb8) SHA1(22ffa55ed362219ebedbc40edcf866ff152a01b9) )
ROM_LOAD( "prom2_702y.bin", 0x000000, 0x000800, CRC(4ca540a5) SHA1(50bb240465d80b7763574e1261f8d0ddda5ad587) )
ROM_LOAD( "prom1_702y.bin", 0x000000, 0x002000, CRC(ab3bbef5) SHA1(199ebb3359a1617148264b307b8b294c037f27a4) )
ROM_LOAD( "prom1_702a.bin", 0x000000, 0x002000, CRC(d6e5120b) SHA1(1d00bce8170b5ad4185e6517ba1a0f46c8ae7444) )
ROM_LOAD( "prom2_702a.bin", 0x000000, 0x000800, CRC(a693785e) SHA1(7c8878f1c3c5205b3ae46a78c881bbd2b722838d) )

Robo-War
ROM_LOAD( "prom1_714.bin", 0x000000, 0x002000, CRC(96da77eb) SHA1(402208f5ac52d8f1e7193bf7c86faa106afe3492) )
ROM_LOAD( "prom2_714.bin", 0x000000, 0x000800, CRC(893177ed) SHA1(791540a64d498979e5b0c8baf4ceb2fd5ff7f047) )
ROM_LOAD( "prom1_714x.bin", 0x000000, 0x002000, CRC(51cd5108) SHA1(9ba30ba0eaaabc8e60c79dc2322aeec51e4de09a) )
ROM_LOAD( "prom2_714x.bin", 0x000000, 0x000800, CRC(1afa0e69) SHA1(178813494b877ac9ca36863661596b4df04df1bb) )

Rock
ROM_LOAD( "prom1_697.bin", 0x000000, 0x002000, CRC(77736ac3) SHA1(30c6322dcda033cbd1cbda5f1bfe97c2067c37f5) )
ROM_LOAD( "prom1_697y.bin", 0x000000, 0x002000, CRC(792b4be4) SHA1(4ee2755024dcc31aaa4cb3b4266fa48291e49d23) )

Rock Encore
ROM_LOAD( "prom1c.cpu",   0x000000, 0x002000, CRC(410d02f6) SHA1(87968576bf5dcca886bcadd4ab379fff080e6eeb) )  // Clash hack
ROM_LOAD( "prom1_704.bin", 0x000000, 0x002000, CRC(77736ac3) SHA1(30c6322dcda033cbd1cbda5f1bfe97c2067c37f5) )
ROM_LOAD( "prom1_704y.bin", 0x000000, 0x002000, CRC(792b4be4) SHA1(4ee2755024dcc31aaa4cb3b4266fa48291e49d23) )

Spring Break
ROM_LOAD( "prom1_706.bin", 0x000000, 0x002000, CRC(2424a214) SHA1(d4d48082652e99731833fbb57a1c04fea6b564b0) )
ROM_LOAD( "prom2_706.bin", 0x000000, 0x000800, CRC(47171062) SHA1(0d2e7777f695ab22170be861019c05ddeade5f85) )
ROM_LOAD( "prom1_706s.bin", 0x000000, 0x002000, CRC(dc956db7) SHA1(8bdb357c0a4c78967b4bb053f9d807897a28ad88) )
ROM_LOAD( "prom2_706s.bin", 0x000000, 0x000800, CRC(911cd14f) SHA1(2bc3ff6a3889da69b97f8ec318f93208e3d42cfe) )
ROM_LOAD( "prom1_706x.bin", 0x000000, 0x002000, CRC(8866bddf) SHA1(e8e54dbd5887241d96f21cb878024436e35f4e40) )
ROM_LOAD( "prom2_706x.bin", 0x000000, 0x000800, CRC(c0ee0555) SHA1(3d2aef5a8a6452f9f87b4ec2040643dda5843ebd) )
ROM_LOAD( "prom1_706y.bin", 0x000000, 0x002000, CRC(11ae0ad4) SHA1(b187c31a0fc2aa7f53655820be26e370e379004c) )
ROM_LOAD( "prom2_706y.bin", 0x000000, 0x000800, CRC(fa4b750d) SHA1(89f797f65fc18473419080810bca4590f77e2502) )
ROM_LOAD( "prom1_706a.bin", 0x000000, 0x002000, CRC(3638cb30) SHA1(6c19ca94255a3dbceb8dd33b2e56287836b1ecba) )
ROM_LOAD( "prom2_706a.bin", 0x000000, 0x000800, CRC(d9d841b4) SHA1(8b9773e5ae9917d27089deca3b8311cb74e7f88e) )

Tag Team
ROM_LOAD( "prom1_698.bin", 0x000000, 0x002000, CRC(3f052c44) SHA1(176fbe35a4ad5832b1ba61889a858b8585dc86be) )
ROM_LOAD( "prom2_698.bin", 0x000000, 0x000800, CRC(fd1615ce) SHA1(3a6c3525552286b86e5340af2bf196f12adc9b35) )
ROM_LOAD( "prom1_698y.bin", 0x000000, 0x002000, CRC(ae1ed7a2) SHA1(e1f640bd350c8c9edc8742de897d92bb58950c3c) )
ROM_LOAD( "prom2_698y.bin", 0x000000, 0x000800, CRC(5e6d2da7) SHA1(9b23d1ac34163edeaceffe806a2a559f3d408b41) )
ROM_LOAD( "prom1_698a.bin", 0x000000, 0x002000, CRC(9c2c0058) SHA1(92e28a0e5fb454b046d1cd365e39ebdd6fa6baf1) )
ROM_LOAD( "prom2_698a.bin", 0x000000, 0x000800, CRC(6d56b636) SHA1(8f50f2742be727835e7343307787b4b5daa1623a) )

TX-Sector
ROM_LOAD( "prom1_712.bin", 0x000000, 0x002000, CRC(13283b01) SHA1(e4e0602ead0ec4d4f54a39df3d08b1aaeb92f1ca) )
ROM_LOAD( "prom2_712.bin", 0x000000, 0x000800, CRC(f12514e6) SHA1(80bca17c33df99ed1a7acc21f7f70ea90e7c0463) )
ROM_LOAD( "prom1_712x.bin", 0x000000, 0x002000, CRC(45e47931) SHA1(a92d323c4892cd1aa429cd884a8f1d33f0379667) )
ROM_LOAD( "prom2_712x.bin", 0x000000, 0x000800, CRC(1bd08247) SHA1(968cc30e5e5c783e73cb3278a58189c4f8b8186f) )
ROM_LOAD( "prom1_712y.bin", 0x000000, 0x002000, CRC(0c374395) SHA1(52a7035598ba83aaf149550e7d08190f9773c25a) )
ROM_LOAD( "prom2_712y.bin", 0x000000, 0x000800, CRC(2b17261f) SHA1(a3195190c0d5116b60e487a7b7f3a28c1f110e89) )

Victory
ROM_LOAD( "prom1_710.bin", 0x000000, 0x002000, CRC(3eba52ec) SHA1(e3cbdd803373e614c5b9bb5e61c9e2dfcf25df6c) )
ROM_LOAD( "prom2_710.bin", 0x000000, 0x000800, CRC(6a42eaf4) SHA1(3e28b01473266db463986a4283e1be85f2410fb1) )
ROM_LOAD( "prom1_710x.bin", 0x000000, 0x002000, CRC(a626da77) SHA1(300674ffb48deed503aae62a3b53b9941058605b) )
ROM_LOAD( "prom2_710x.bin", 0x000000, 0x000800, CRC(dffcfa77) SHA1(3efaca85295ca55268b8d7c7cfe8f09f159d5fbd) )
ROM_LOAD( "prom1_710y.bin", 0x000000, 0x002000, CRC(6daebe71) SHA1(bc49c074210f3f3cc5314282e32cebb7ce67a81d) )
ROM_LOAD( "prom2_710y.bin", 0x000000, 0x000800, CRC(b191a87a) SHA1(f205ffb41c5ba34e3cefc96ca870a5d08bee8854) )

Unknown - these are for this hardware, but it's not known which specific machine they work on
ROM_LOAD( "prom1_bb.cpu", 0x000000, 0x002000, CRC(a035eb2d) SHA1(0f467b506bd514129e4175af3e35a666e09ec41b) )  // Beach Bums hack
ROM_LOAD( "prom1_b.cpu",  0x000000, 0x002000, CRC(6556d711) SHA1(9d0ccaf05d0aa5a68e5514a2ade7773959868bbb) )  // Bubba the Redneck Werewolf hack

*/

/*-------------------------------------------------------------------
/ Ace High (#700) 1985 (Prototype)
/-------------------------------------------------------------------*/

/*-------------------------------------------------------------------
/ Amazon Hunt II (#684C)
/-------------------------------------------------------------------*/

ROM_START(amazonh2)
	ROM_REGION(0x10000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD("684c-cpu.rom", 0x2000, 0x2000, CRC(0b5040c3) SHA1(104e5a63b4097ea72a5b31df1a7d5198342be5c4))
	ROM_RELOAD(0x6000, 0x2000)
	ROM_RELOAD(0xa000, 0x2000)
	ROM_RELOAD(0xe000, 0x2000)

	ROM_REGION(0x10000, "r2sound:audiocpu", ROMREGION_ERASEFF)
	ROM_LOAD("684c-snd.rom",0xc000,0x2000, CRC(182d64e1) SHA1(c0aaa646a3d53cf00aa23e0b8d46bbb70ce46e5c))

	ROM_REGION(0x10000, "r2sound:speechcpu", ROMREGION_ERASEFF)
ROM_END

/*-------------------------------------------------------------------
/ Amazon Hunt III (#684D)
/-------------------------------------------------------------------*/

ROM_START(amazonh3)
	ROM_REGION(0x10000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD("684d-cpu.rom", 0x2000, 0x2000, CRC(2ec8bd4c) SHA1(46a08ddccba952fa69b79739802b676567f6386f))
	ROM_RELOAD(0x6000, 0x2000)
	ROM_RELOAD(0xa000, 0x2000)
	ROM_RELOAD(0xe000, 0x2000)

	ROM_REGION(0x10000, "p4sound:audiocpu", ROMREGION_ERASEFF)
	ROM_LOAD("684d-snd.rom",0x8000,0x8000, CRC(a660f233) SHA1(3b80629696a2fd5aa4a86ed472e60c95d3cfa906))

	ROM_REGION(0x10000, "p4sound:speechcpu", ROMREGION_ERASEFF)
ROM_END

ROM_START(amazonh3a)
	ROM_REGION(0x10000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD("684d-1-cpu.rom", 0x2000, 0x2000, CRC(bf4674e1) SHA1(30974f89f9e4cbb61f8f620499ee6a64c9b7b31c))
	ROM_RELOAD(0x6000, 0x2000)
	ROM_RELOAD(0xa000, 0x2000)
	ROM_RELOAD(0xe000, 0x2000)

	ROM_REGION(0x10000, "p4sound:audiocpu", ROMREGION_ERASEFF)
	ROM_LOAD("684d-snd.rom",0x8000,0x8000, CRC(a660f233) SHA1(3b80629696a2fd5aa4a86ed472e60c95d3cfa906))

	ROM_REGION(0x10000, "p4sound:speechcpu", ROMREGION_ERASEFF)
ROM_END

/*-------------------------------------------------------------------
/ Arena (#709)
/-------------------------------------------------------------------*/
ROM_START(arena)
	ROM_REGION(0x10000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD("prom2.cpu", 0x1000, 0x0800, CRC(4783b689) SHA1(d10d4cbf8d00c9d0db57cdac32ef96498275eea6))
	ROM_RELOAD(0x5000, 0x0800)
	ROM_RELOAD(0x9000, 0x0800)
	ROM_RELOAD(0xd000, 0x0800)
	ROM_LOAD("prom1.cpu", 0x2000, 0x2000, CRC(8c9f8ee9) SHA1(840505d08e387c3f7de105305e183f8ed3a6d5c6))
	ROM_RELOAD(0x6000, 0x2000)
	ROM_RELOAD(0xa000, 0x2000)
	ROM_RELOAD(0xe000, 0x2000)

	ROM_REGION(0x10000, "r2sound:audiocpu", ROMREGION_ERASEFF)
	ROM_LOAD("drom1.snd",0xc000,0x2000, CRC(78e6cbf1) SHA1(7b66a0cb211a93cf475172aa0465a952009e1a59))

	ROM_REGION(0x10000, "r2sound:speechcpu", ROMREGION_ERASEFF)
	ROM_LOAD("yrom1.snd",0xe000,0x2000, CRC(f7a951c2) SHA1(12d7a6119d9033ae02c6312c9af888bfc7c63ad1))
	ROM_LOAD("yrom2.snd",0xc000,0x2000, CRC(cc2aef4e) SHA1(a6e243de99f6a76eb527e879f4441c036dd379b6))
ROM_END

ROM_START(arenaa)
	ROM_REGION(0x10000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD("prom2a.cpu", 0x1000, 0x0800, CRC(13c8813b) SHA1(756e3583fd55b72e0bfb15e9b4a60740b389ca2e))
	ROM_RELOAD(0x5000, 0x0800)
	ROM_RELOAD(0x9000, 0x0800)
	ROM_RELOAD(0xd000, 0x0800)
	ROM_LOAD("prom1a.cpu", 0x2000, 0x2000, CRC(253eceb1) SHA1(b46ccec4b3e8fc57fb3295b675b4f27dafc0322e))
	ROM_RELOAD(0x6000, 0x2000)
	ROM_RELOAD(0xa000, 0x2000)
	ROM_RELOAD(0xe000, 0x2000)

	ROM_REGION(0x10000, "r2sound:audiocpu", ROMREGION_ERASEFF)
	ROM_LOAD("drom1.snd",0xc000,0x2000, CRC(78e6cbf1) SHA1(7b66a0cb211a93cf475172aa0465a952009e1a59))

	ROM_REGION(0x10000, "r2sound:speechcpu", ROMREGION_ERASEFF)
	ROM_LOAD("yrom1.snd",0xe000,0x2000, CRC(f7a951c2) SHA1(12d7a6119d9033ae02c6312c9af888bfc7c63ad1))
	ROM_LOAD("yrom2.snd",0xc000,0x2000, CRC(cc2aef4e) SHA1(a6e243de99f6a76eb527e879f4441c036dd379b6))
ROM_END

ROM_START(arenaf)
	ROM_REGION(0x10000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD("prom2f.cpu", 0x1000, 0x0800, CRC(49b127d8) SHA1(0436f83e969b4bfc7edaf881bf7556a868c88cdc))
	ROM_RELOAD(0x5000, 0x0800)
	ROM_RELOAD(0x9000, 0x0800)
	ROM_RELOAD(0xd000, 0x0800)
	ROM_LOAD("prom1f.cpu", 0x2000, 0x2000, CRC(391fb7de) SHA1(ec47a6e057d18a0043afccb694c23d0fa0d42aa0))
	ROM_RELOAD(0x6000, 0x2000)
	ROM_RELOAD(0xa000, 0x2000)
	ROM_RELOAD(0xe000, 0x2000)

	ROM_REGION(0x10000, "r2sound:audiocpu", ROMREGION_ERASEFF)
	ROM_LOAD("drom1.snd",0xc000,0x2000, CRC(78e6cbf1) SHA1(7b66a0cb211a93cf475172aa0465a952009e1a59))

	ROM_REGION(0x10000, "r2sound:speechcpu", ROMREGION_ERASEFF)
	ROM_LOAD("yrom1.snd",0xe000,0x2000, CRC(f7a951c2) SHA1(12d7a6119d9033ae02c6312c9af888bfc7c63ad1))
	ROM_LOAD("yrom2.snd",0xc000,0x2000, CRC(cc2aef4e) SHA1(a6e243de99f6a76eb527e879f4441c036dd379b6))
ROM_END

ROM_START(arenag)
	ROM_REGION(0x10000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD("prom2g.cpu", 0x1000, 0x0800, CRC(e170d1cd) SHA1(bd7919eb9e480309f794ac25a371c7b818dcd01b))
	ROM_RELOAD(0x5000, 0x0800)
	ROM_RELOAD(0x9000, 0x0800)
	ROM_RELOAD(0xd000, 0x0800)
	ROM_LOAD("prom1g.cpu", 0x2000, 0x2000, CRC(71fd6e48) SHA1(5c87ba79968085d386fd1357c9d8b2b7a745682a))
	ROM_RELOAD(0x6000, 0x2000)
	ROM_RELOAD(0xa000, 0x2000)
	ROM_RELOAD(0xe000, 0x2000)

	ROM_REGION(0x10000, "r2sound:audiocpu", ROMREGION_ERASEFF)
	ROM_LOAD("drom1.snd",0xc000,0x2000, CRC(78e6cbf1) SHA1(7b66a0cb211a93cf475172aa0465a952009e1a59))

	ROM_REGION(0x10000, "r2sound:speechcpu", ROMREGION_ERASEFF)
	ROM_LOAD("yrom1.snd",0xe000,0x2000, CRC(f7a951c2) SHA1(12d7a6119d9033ae02c6312c9af888bfc7c63ad1))
	ROM_LOAD("yrom2.snd",0xc000,0x2000, CRC(cc2aef4e) SHA1(a6e243de99f6a76eb527e879f4441c036dd379b6))
ROM_END

/*-------------------------------------------------------------------
/ Bad Girls (#717)
/-------------------------------------------------------------------*/
ROM_START(badgirls)
	ROM_REGION(0x10000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD("prom2.cpu", 0x1000, 0x0800, CRC(583933ec) SHA1(89da6750d779d68db578715b058f9321695b79b0))
	ROM_CONTINUE(0x9000, 0x0800)
	ROM_RELOAD(0x5000, 0x0800)
	ROM_CONTINUE(0xd000, 0x0800)
	ROM_LOAD("prom1.cpu", 0x2000, 0x2000, CRC(956aeae0) SHA1(24d9d514fc83aba1ab310bfe4ed80605df399417))
	ROM_RELOAD(0x6000, 0x2000)
	ROM_RELOAD(0xa000, 0x2000)
	ROM_RELOAD(0xe000, 0x2000)

	ROM_REGION(0x10000, "p5sound:audiocpu", ROMREGION_ERASEFF)
	ROM_LOAD("drom1.snd", 0x8000, 0x8000, CRC(452dec20) SHA1(a9c41dfb2d83c5671ab96e946f13df774b567976))

	ROM_REGION(0x10000, "p5sound:speechcpu", ROMREGION_ERASEFF)
	ROM_LOAD("yrom1.snd", 0x8000, 0x8000, CRC(ab3b8e2d) SHA1(b57a0b804b42b923bb102d295e3b8a69b1033d27))
ROM_END

ROM_START(badgirlsa)
	ROM_REGION(0x10000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD("prom2a.cpu", 0x1000, 0x0800, CRC(53e05ca7) SHA1(a45a37e180f10fcbc3fe89be28b3d5c7e56561c2))
	ROM_CONTINUE(0x9000, 0x0800)
	ROM_RELOAD(0x5000, 0x0800)
	ROM_CONTINUE(0xd000, 0x0800)
	ROM_LOAD("prom1a.cpu", 0x2000, 0x2000, CRC(07082568) SHA1(ea89dede1543fe34f8f0e95a33120a727c3ff274))
	ROM_RELOAD(0x6000, 0x2000)
	ROM_RELOAD(0xa000, 0x2000)
	ROM_RELOAD(0xe000, 0x2000)

	ROM_REGION(0x10000, "p5sound:audiocpu", ROMREGION_ERASEFF)
	ROM_LOAD("drom1.snd", 0x8000, 0x8000, CRC(452dec20) SHA1(a9c41dfb2d83c5671ab96e946f13df774b567976))

	ROM_REGION(0x10000, "p5sound:speechcpu", ROMREGION_ERASEFF)
	ROM_LOAD("yrom1.snd", 0x8000, 0x8000, CRC(ab3b8e2d) SHA1(b57a0b804b42b923bb102d295e3b8a69b1033d27))
ROM_END

ROM_START(badgirlsf)
	ROM_REGION(0x10000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD("prom2f.cpu", 0x1000, 0x0800, CRC(58c35099) SHA1(ff76bd28175ea0f5d0437c16c5ae6886339edfe2))
	ROM_CONTINUE(0x9000, 0x0800)
	ROM_RELOAD(0x5000, 0x0800)
	ROM_CONTINUE(0xd000, 0x0800)
	ROM_LOAD("prom1f.cpu", 0x2000, 0x2000, CRC(9861147a) SHA1(e9d31cd1130bc1785db26c23f52944842fdd4ca0))
	ROM_RELOAD(0x6000, 0x2000)
	ROM_RELOAD(0xa000, 0x2000)
	ROM_RELOAD(0xe000, 0x2000)

	ROM_REGION(0x10000, "p5sound:audiocpu", ROMREGION_ERASEFF)
	ROM_LOAD("drom1.snd", 0x8000, 0x8000, CRC(452dec20) SHA1(a9c41dfb2d83c5671ab96e946f13df774b567976))

	ROM_REGION(0x10000, "p5sound:speechcpu", ROMREGION_ERASEFF)
	ROM_LOAD("yrom1.snd", 0x8000, 0x8000, CRC(ab3b8e2d) SHA1(b57a0b804b42b923bb102d295e3b8a69b1033d27))
ROM_END

ROM_START(badgirlsg)
	ROM_REGION(0x10000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD("prom2g.cpu", 0x1000, 0x0800, CRC(55aa30ac) SHA1(9544485ccf52a2ad51a00cce0c12871db099699f))
	ROM_CONTINUE(0x9000, 0x0800)
	ROM_RELOAD(0x5000, 0x0800)
	ROM_CONTINUE(0xd000, 0x0800)
	ROM_LOAD("prom1g.cpu", 0x2000, 0x2000, CRC(f2923255) SHA1(645b62d015e3a4feaf485c600eb345824f551b9e))
	ROM_RELOAD(0x6000, 0x2000)
	ROM_RELOAD(0xa000, 0x2000)
	ROM_RELOAD(0xe000, 0x2000)

	ROM_REGION(0x10000, "p5sound:audiocpu", ROMREGION_ERASEFF)
	ROM_LOAD("drom1.snd", 0x8000, 0x8000, CRC(452dec20) SHA1(a9c41dfb2d83c5671ab96e946f13df774b567976))

	ROM_REGION(0x10000, "p5sound:speechcpu", ROMREGION_ERASEFF)
	ROM_LOAD("yrom1.snd", 0x8000, 0x8000, CRC(ab3b8e2d) SHA1(b57a0b804b42b923bb102d295e3b8a69b1033d27))
ROM_END

/*-------------------------------------------------------------------
/ Big House (#713)
/-------------------------------------------------------------------*/
ROM_START(bighouse)
	ROM_REGION(0x10000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD("prom2.cpu", 0x1000, 0x0800, CRC(047c8ef5) SHA1(3afa2a0011b724836b69b2ef386597e0953dfadf))
	ROM_CONTINUE(0x9000, 0x0800)
	ROM_RELOAD(0x5000, 0x0800)
	ROM_CONTINUE(0xd000, 0x0800)
	ROM_LOAD("prom1.cpu", 0x2000, 0x2000, CRC(0ecef900) SHA1(78e4ed6e40fdb45dde2d0f2cf60d4c8a7ea2e39e))
	ROM_RELOAD(0x6000, 0x2000)
	ROM_RELOAD(0xa000, 0x2000)
	ROM_RELOAD(0xe000, 0x2000)

	ROM_REGION(0x10000, "p5sound:audiocpu", ROMREGION_ERASEFF)
	ROM_LOAD("drom1.snd", 0x8000, 0x8000, CRC(f330fd04) SHA1(1288c47f636d9d5b826a2b870b81788a630e489e))

	ROM_REGION(0x10000, "p5sound:speechcpu", ROMREGION_ERASEFF)
	ROM_LOAD("yrom1.snd", 0x8000, 0x8000, CRC(0b1ba1cb) SHA1(26327689992018837b1c9957c515ab67248623eb))
ROM_END

ROM_START(bighousef)
	ROM_REGION(0x10000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD("prom2f.cpu", 0x1000, 0x0800, CRC(767efc44) SHA1(6b8f9a580e6a6ad92c9efe9f4345496d5063b7a8))
	ROM_CONTINUE(0x9000, 0x0800)
	ROM_RELOAD(0x5000, 0x0800)
	ROM_CONTINUE(0xd000, 0x0800)
	ROM_LOAD("prom1f.cpu", 0x2000, 0x2000, CRC(b87150bc) SHA1(2ebdf27ede3445ac99068c8cec712c06e57c7ffc))
	ROM_RELOAD(0x6000, 0x2000)
	ROM_RELOAD(0xa000, 0x2000)
	ROM_RELOAD(0xe000, 0x2000)

	ROM_REGION(0x10000, "p5sound:audiocpu", ROMREGION_ERASEFF)
	ROM_LOAD("drom1.snd", 0x8000, 0x8000, CRC(f330fd04) SHA1(1288c47f636d9d5b826a2b870b81788a630e489e))

	ROM_REGION(0x10000, "p5sound:speechcpu", ROMREGION_ERASEFF)
	ROM_LOAD("yrom1.snd", 0x8000, 0x8000, CRC(0b1ba1cb) SHA1(26327689992018837b1c9957c515ab67248623eb))
ROM_END

ROM_START(bighouseg)
	ROM_REGION(0x10000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD("prom2g.cpu", 0x1000, 0x0800, CRC(214f0afb) SHA1(9874773e4ffa2472e78d42dfa9e21a621bf7b49e))
	ROM_CONTINUE(0x9000, 0x0800)
	ROM_RELOAD(0x5000, 0x0800)
	ROM_CONTINUE(0xd000, 0x0800)
	ROM_LOAD("prom1g.cpu", 0x2000, 0x2000, CRC(374f3593) SHA1(e90d867fff28ee86f017b1b638bc26f1bcde6b81))
	ROM_RELOAD(0x6000, 0x2000)
	ROM_RELOAD(0xa000, 0x2000)
	ROM_RELOAD(0xe000, 0x2000)

	ROM_REGION(0x10000, "p5sound:audiocpu", ROMREGION_ERASEFF)
	ROM_LOAD("drom1.snd", 0x8000, 0x8000, CRC(f330fd04) SHA1(1288c47f636d9d5b826a2b870b81788a630e489e))

	ROM_REGION(0x10000, "p5sound:speechcpu", ROMREGION_ERASEFF)
	ROM_LOAD("yrom1.snd", 0x8000, 0x8000, CRC(0b1ba1cb) SHA1(26327689992018837b1c9957c515ab67248623eb))
ROM_END

/*-------------------------------------------------------------------
/ Bone Busters Inc. (#719)
/-------------------------------------------------------------------*/
ROM_START(bonebstr)
	ROM_REGION(0x10000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD("prom2.cpu", 0x1000, 0x0800, CRC(681643df) SHA1(76af6951e4403b4951298d35a9058bcebfa6bc43))
	ROM_CONTINUE(0x9000, 0x0800)
	ROM_RELOAD(0x5000, 0x0800)
	ROM_CONTINUE(0xd000, 0x0800)
	ROM_LOAD("prom1.cpu", 0x2000, 0x2000, CRC(052f97be) SHA1(0ee108e79c4196dffedc64d7f7a576e0394427c1))
	ROM_RELOAD(0x6000, 0x2000)
	ROM_RELOAD(0xa000, 0x2000)
	ROM_RELOAD(0xe000, 0x2000)

	ROM_REGION(0x10000, "p6sound:dcpu2", ROMREGION_ERASEFF)
	ROM_LOAD("drom2.snd", 0x8000, 0x8000, CRC(d147d78d) SHA1(f8f6d6a1921685b883b224a9ea85ead52a32a4c3))

	ROM_REGION(0x10000, "p6sound:audiocpu", ROMREGION_ERASEFF)
	ROM_LOAD("drom1.snd", 0x8000, 0x8000, CRC(ec43f4e9) SHA1(77b0988700be7a597dca7e5f06ac5d3c6834ce21))

	ROM_REGION(0x10000, "p6sound:speechcpu", ROMREGION_ERASEFF)
	ROM_LOAD("yrom1.snd", 0x8000, 0x8000, CRC(a95eedfc) SHA1(5ced2d6869a9895f8ff26d830b21d3c9364b32e7))
ROM_END

ROM_START(bonebstrf)
	ROM_REGION(0x10000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD("prom2f.cpu", 0x1000, 0x0800, CRC(73b6486e) SHA1(1baf17f31b16d564ed5e3bdf9f74b21f83ed76fa))
	ROM_CONTINUE(0x9000, 0x0800)
	ROM_RELOAD(0x5000, 0x0800)
	ROM_CONTINUE(0xd000, 0x0800)
	ROM_LOAD("prom1f.cpu", 0x2000, 0x2000, CRC(3d334065) SHA1(6d44819cf84bee375a9f62351b00375404f6d3e3))
	ROM_RELOAD(0x6000, 0x2000)
	ROM_RELOAD(0xa000, 0x2000)
	ROM_RELOAD(0xe000, 0x2000)

	ROM_REGION(0x10000, "p6sound:dcpu2", ROMREGION_ERASEFF)
	ROM_LOAD("drom2.snd", 0x8000, 0x8000, CRC(d147d78d) SHA1(f8f6d6a1921685b883b224a9ea85ead52a32a4c3))

	ROM_REGION(0x10000, "p6sound:audiocpu", ROMREGION_ERASEFF)
	ROM_LOAD("drom1.snd", 0x8000, 0x8000, CRC(ec43f4e9) SHA1(77b0988700be7a597dca7e5f06ac5d3c6834ce21))

	ROM_REGION(0x10000, "p6sound:speechcpu", ROMREGION_ERASEFF)
	ROM_LOAD("yrom1.snd", 0x8000, 0x8000, CRC(a95eedfc) SHA1(5ced2d6869a9895f8ff26d830b21d3c9364b32e7))
ROM_END

ROM_START(bonebstrg)
	ROM_REGION(0x10000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD("prom2g.cpu", 0x1000, 0x0800, CRC(3b85c8bd) SHA1(5c99349dc3ae05b82932d6ec9d2d1a29c2a7e36d))
	ROM_CONTINUE(0x9000, 0x0800)
	ROM_RELOAD(0x5000, 0x0800)
	ROM_CONTINUE(0xd000, 0x0800)
	ROM_LOAD("prom1g.cpu", 0x2000, 0x2000, CRC(a0aab93e) SHA1(b7fa3d6eeb1977e4d91644aab1ac03aeee6934d0))
	ROM_RELOAD(0x6000, 0x2000)
	ROM_RELOAD(0xa000, 0x2000)
	ROM_RELOAD(0xe000, 0x2000)

	ROM_REGION(0x10000, "p6sound:dcpu2", ROMREGION_ERASEFF)
	ROM_LOAD("drom2.snd", 0x8000, 0x8000, CRC(d147d78d) SHA1(f8f6d6a1921685b883b224a9ea85ead52a32a4c3))

	ROM_REGION(0x10000, "p6sound:audiocpu", ROMREGION_ERASEFF)
	ROM_LOAD("drom1.snd", 0x8000, 0x8000, CRC(ec43f4e9) SHA1(77b0988700be7a597dca7e5f06ac5d3c6834ce21))

	ROM_REGION(0x10000, "p6sound:speechcpu", ROMREGION_ERASEFF)
	ROM_LOAD("yrom1.snd", 0x8000, 0x8000, CRC(a95eedfc) SHA1(5ced2d6869a9895f8ff26d830b21d3c9364b32e7))
ROM_END

/*-------------------------------------------------------------------
/ Bounty Hunter (#694)
/-------------------------------------------------------------------*/
ROM_START(bountyh)
	ROM_REGION(0x10000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD("prom1.cpu", 0x2000, 0x2000, CRC(e8190df7) SHA1(5304918d35e379da17ab19d8879a7ace5c864326))
	ROM_RELOAD(0x6000, 0x2000)
	ROM_RELOAD(0xa000, 0x2000)
	ROM_RELOAD(0xe000, 0x2000)

	ROM_REGION(0x1000, "p3sound:audiocpu", ROMREGION_ERASEFF)
	ROM_LOAD("694-s.snd", 0x0800, 0x0800, CRC(a0383e41) SHA1(156514d2b52fcd89b608b85991c5066780949979))
ROM_END

ROM_START(bountyhg)
	ROM_REGION(0x10000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD("prom1g.cpu", 0x2000, 0x2000, CRC(ea4b7e2d) SHA1(9141c950b33e32ae8ad76fd0dd06d1a13d38be9d))
	ROM_RELOAD(0x6000, 0x2000)
	ROM_RELOAD(0xa000, 0x2000)
	ROM_RELOAD(0xe000, 0x2000)

	ROM_REGION(0x1000, "p3sound:audiocpu", ROMREGION_ERASEFF)
	ROM_LOAD("694-s.snd", 0x0800, 0x0800, CRC(a0383e41) SHA1(156514d2b52fcd89b608b85991c5066780949979))
ROM_END

/*-------------------------------------------------------------------
/ Chicago Cubs' Triple Play (#696)
/-------------------------------------------------------------------*/
ROM_START(triplay)
	ROM_REGION(0x10000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD("prom1.cpu", 0x2000, 0x2000, CRC(42b29b01) SHA1(58145ce10939d00faff49972ada669005a223792))
	ROM_RELOAD(0x6000, 0x2000)
	ROM_RELOAD(0xa000, 0x2000)
	ROM_RELOAD(0xe000, 0x2000)

	ROM_REGION(0x1000, "p3sound:audiocpu", ROMREGION_ERASEFF)
	ROM_LOAD("696-s.snd", 0x0800, 0x0800, CRC(deedea61) SHA1(6aec221397f250d5dd99faefa313e8028c8818f7))
ROM_END

ROM_START(triplaya)
	ROM_REGION(0x10000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD("prom1a.cpu", 0x2000, 0x2000, CRC(fc2145cb) SHA1(f7b9648c533997e9f777a8b40dad9852f26abd9a))
	ROM_RELOAD(0x6000, 0x2000)
	ROM_RELOAD(0xa000, 0x2000)
	ROM_RELOAD(0xe000, 0x2000)

	ROM_REGION(0x1000, "p3sound:audiocpu", ROMREGION_ERASEFF)
	ROM_LOAD("696-s.snd", 0x0800, 0x0800, CRC(deedea61) SHA1(6aec221397f250d5dd99faefa313e8028c8818f7))
ROM_END

ROM_START(triplayg)
	ROM_REGION(0x10000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD("prom1g.cpu", 0x2000, 0x2000, CRC(5e2bf7a9) SHA1(fdbec615b22416bb4b2e712d47c54c945d849252))
	ROM_RELOAD(0x6000, 0x2000)
	ROM_RELOAD(0xa000, 0x2000)
	ROM_RELOAD(0xe000, 0x2000)

	ROM_REGION(0x1000, "p3sound:audiocpu", ROMREGION_ERASEFF)
	ROM_LOAD("696-s.snd", 0x0800, 0x0800, CRC(deedea61) SHA1(6aec221397f250d5dd99faefa313e8028c8818f7))
ROM_END

/*-------------------------------------------------------------------
/ Diamond Lady (#711)
/-------------------------------------------------------------------*/
ROM_START(diamondp)
	ROM_REGION(0x10000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD("prom2.cpu", 0x1000, 0x0800, CRC(862951dc) SHA1(b15899ecf7ec869e3722cef3f5c16b0dadd2514e))
	ROM_RELOAD(0x5000, 0x0800)
	ROM_RELOAD(0x9000, 0x0800)
	ROM_RELOAD(0xd000, 0x0800)
	ROM_LOAD("prom1.cpu", 0x2000, 0x2000, CRC(7a011757) SHA1(cc49ec7451feae035670ea9d70cc8f6b32747c90))
	ROM_RELOAD(0x6000, 0x2000)
	ROM_RELOAD(0xa000, 0x2000)
	ROM_RELOAD(0xe000, 0x2000)

	ROM_REGION(0x10000, "p4sound:audiocpu", ROMREGION_ERASEFF)
	ROM_LOAD("drom1.snd", 0x8000, 0x8000, CRC(c216d1e4) SHA1(aa38db5ad36d1d1d35e727ab27c1f1c05a9627cd))

	ROM_REGION(0x10000, "p4sound:speechcpu", ROMREGION_ERASEFF)
	ROM_LOAD("yrom1.snd", 0x8000, 0x8000, CRC(0a18d626) SHA1(6b367668be55ca04c69c4c4c5a4a524ae8f790f8))
ROM_END

ROM_START(diamondpf)
	ROM_REGION(0x10000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD("prom2f.cpu", 0x1000, 0x0800, CRC(943019a8) SHA1(558c3696339bb6e150b4ddb499bc60897d5954ec))
	ROM_RELOAD(0x5000, 0x0800)
	ROM_RELOAD(0x9000, 0x0800)
	ROM_RELOAD(0xd000, 0x0800)
	ROM_LOAD("prom1f.cpu", 0x2000, 0x2000, CRC(479b0267) SHA1(a9586c5b2cc3561ba3409123eca5a73ebabfd823))
	ROM_RELOAD(0x6000, 0x2000)
	ROM_RELOAD(0xa000, 0x2000)
	ROM_RELOAD(0xe000, 0x2000)

	ROM_REGION(0x10000, "p4sound:audiocpu", ROMREGION_ERASEFF)
	ROM_LOAD("drom1.snd", 0x8000, 0x8000, CRC(c216d1e4) SHA1(aa38db5ad36d1d1d35e727ab27c1f1c05a9627cd))

	ROM_REGION(0x10000, "p4sound:speechcpu", ROMREGION_ERASEFF)
	ROM_LOAD("yrom1.snd", 0x8000, 0x8000, CRC(0a18d626) SHA1(6b367668be55ca04c69c4c4c5a4a524ae8f790f8))
ROM_END

ROM_START(diamondpg)
	ROM_REGION(0x10000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD("prom2g.cpu", 0x1000, 0x0800, CRC(f0ef69f6) SHA1(1f48bb656bb20073e2ff261199cb94919f0bb2ab))
	ROM_RELOAD(0x5000, 0x0800)
	ROM_RELOAD(0x9000, 0x0800)
	ROM_RELOAD(0xd000, 0x0800)
	ROM_LOAD("prom1g.cpu", 0x2000, 0x2000, CRC(961cfdf9) SHA1(97135f77705969736f704acdeda6157bb765c73e))
	ROM_RELOAD(0x6000, 0x2000)
	ROM_RELOAD(0xa000, 0x2000)
	ROM_RELOAD(0xe000, 0x2000)

	ROM_REGION(0x10000, "p4sound:audiocpu", ROMREGION_ERASEFF)
	ROM_LOAD("drom1.snd", 0x8000, 0x8000, CRC(c216d1e4) SHA1(aa38db5ad36d1d1d35e727ab27c1f1c05a9627cd))

	ROM_REGION(0x10000, "p4sound:speechcpu", ROMREGION_ERASEFF)
	ROM_LOAD("yrom1.snd", 0x8000, 0x8000, CRC(0a18d626) SHA1(6b367668be55ca04c69c4c4c5a4a524ae8f790f8))
ROM_END

/*-------------------------------------------------------------------
/ Excalibur (#715)
/-------------------------------------------------------------------*/
ROM_START(excalibr)
	ROM_REGION(0x10000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD("prom2.cpu", 0x1000, 0x0800, CRC(082d64ab) SHA1(0eae3b549839fc281d2487d483d0b4e723ebdc48))
	ROM_CONTINUE(0x9000, 0x0800)
	ROM_RELOAD(0x5000, 0x0800)
	ROM_CONTINUE(0xd000, 0x0800)
	ROM_LOAD("prom1.cpu", 0x2000, 0x2000, CRC(e8902c16) SHA1(c3e4ece6be7027a4deef052ba4be752070e9b542))
	ROM_RELOAD(0x6000, 0x2000)
	ROM_RELOAD(0xa000, 0x2000)
	ROM_RELOAD(0xe000, 0x2000)

	ROM_REGION(0x10000, "p4sound:audiocpu", ROMREGION_ERASEFF)
	ROM_LOAD("drom1.snd", 0x8000, 0x8000, CRC(a4368cd0) SHA1(c48513e56899938dc83a3545d8ee9def3dc1491f))

	ROM_REGION(0x10000, "p4sound:speechcpu", ROMREGION_ERASEFF)
	ROM_LOAD("yrom1.snd", 0x8000, 0x8000, CRC(9f194744) SHA1(dbd73b546071c3d4f0dcfe21e3e646da716c5b71))
ROM_END

ROM_START(excalibrf)
	ROM_REGION(0x10000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD("prom2f.cpu", 0x1000, 0x0800, CRC(499e2e41) SHA1(1e3fcba18882bd7df30a43843916aa5d7968eecc))
	ROM_CONTINUE(0x9000, 0x0800)
	ROM_RELOAD(0x5000, 0x0800)
	ROM_CONTINUE(0xd000, 0x0800)
	ROM_LOAD("prom1f.cpu", 0x2000, 0x2000, CRC(ed1083d7) SHA1(3ff829ecfaba7d20c75268d3ee5224cb3cac3507))
	ROM_RELOAD(0x6000, 0x2000)
	ROM_RELOAD(0xa000, 0x2000)
	ROM_RELOAD(0xe000, 0x2000)

	ROM_REGION(0x10000, "p4sound:audiocpu", ROMREGION_ERASEFF)
	ROM_LOAD("drom1.snd", 0x8000, 0x8000, CRC(a4368cd0) SHA1(c48513e56899938dc83a3545d8ee9def3dc1491f))

	ROM_REGION(0x10000, "p4sound:speechcpu", ROMREGION_ERASEFF)
	ROM_LOAD("yrom1.snd", 0x8000, 0x8000, CRC(9f194744) SHA1(dbd73b546071c3d4f0dcfe21e3e646da716c5b71))
ROM_END

ROM_START(excalibrg)
	ROM_REGION(0x10000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD("prom2g.cpu", 0x1000, 0x0800, CRC(49079396) SHA1(92361a87464e39afeb74fe531b7d4356323405b8))
	ROM_CONTINUE(0x9000, 0x0800)
	ROM_RELOAD(0x5000, 0x0800)
	ROM_CONTINUE(0xd000, 0x0800)
	ROM_LOAD("prom1g.cpu", 0x2000, 0x2000, CRC(504fad7a) SHA1(6648778d537161e9bdcf2955209e1525e90a3617))
	ROM_RELOAD(0x6000, 0x2000)
	ROM_RELOAD(0xa000, 0x2000)
	ROM_RELOAD(0xe000, 0x2000)

	ROM_REGION(0x10000, "p4sound:audiocpu", ROMREGION_ERASEFF)
	ROM_LOAD("drom1.snd", 0x8000, 0x8000, CRC(a4368cd0) SHA1(c48513e56899938dc83a3545d8ee9def3dc1491f))

	ROM_REGION(0x10000, "p4sound:speechcpu", ROMREGION_ERASEFF)
	ROM_LOAD("yrom1.snd", 0x8000, 0x8000, CRC(9f194744) SHA1(dbd73b546071c3d4f0dcfe21e3e646da716c5b71))
ROM_END

/*-------------------------------------------------------------------
/ Genesis (#705)
/-------------------------------------------------------------------*/
ROM_START(genesisp)
	ROM_REGION(0x10000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD("prom2.cpu", 0x1000, 0x0800, CRC(ac9f3a0f) SHA1(0e44888dc046121794e824d128628f991245c1cb))
	ROM_RELOAD(0x5000, 0x0800)
	ROM_RELOAD(0x9000, 0x0800)
	ROM_RELOAD(0xd000, 0x0800)
	ROM_LOAD("prom1.cpu", 0x2000, 0x2000, CRC(4a2f185c) SHA1(b45982b1ce9777292731ad523516c76cde4ddfa4))
	ROM_RELOAD(0x6000, 0x2000)
	ROM_RELOAD(0xa000, 0x2000)
	ROM_RELOAD(0xe000, 0x2000)

	ROM_REGION(0x10000, "r2sound:audiocpu", ROMREGION_ERASEFF)
	ROM_LOAD("drom1.snd",0xc000,0x2000, CRC(758e1743) SHA1(6df3011c044796afcd88e52d1ca69692cb489ff4))

	ROM_REGION(0x10000, "r2sound:speechcpu", ROMREGION_ERASEFF)
	ROM_LOAD("yrom1.snd",0xe000,0x2000, CRC(4869b0ec) SHA1(b8a56753257205af56e06105515b8a700bb1935b))
	ROM_LOAD("yrom2.snd",0xc000,0x2000, CRC(0528c024) SHA1(d24ff7e088b08c1f35b54be3c806f8a8757d96c7))
ROM_END

ROM_START(genesispf)
	ROM_REGION(0x10000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD("prom2f.cpu", 0x1000, 0x0800, CRC(ea7f824f) SHA1(45f619153e0584cffd33e6e09e6f5a97ab9522b2))
	ROM_RELOAD(0x5000, 0x0800)
	ROM_RELOAD(0x9000, 0x0800)
	ROM_RELOAD(0xd000, 0x0800)
	ROM_LOAD("prom1f.cpu", 0x2000, 0x2000, CRC(e7ef875b) SHA1(37ac83d9a75ce604c5a4173ce918beb64f75cd3e))
	ROM_RELOAD(0x6000, 0x2000)
	ROM_RELOAD(0xa000, 0x2000)
	ROM_RELOAD(0xe000, 0x2000)

	ROM_REGION(0x10000, "r2sound:audiocpu", ROMREGION_ERASEFF)
	ROM_LOAD("drom1.snd",0xc000,0x2000, CRC(758e1743) SHA1(6df3011c044796afcd88e52d1ca69692cb489ff4))

	ROM_REGION(0x10000, "r2sound:speechcpu", ROMREGION_ERASEFF)
	ROM_LOAD("yrom1.snd",0xe000,0x2000, CRC(4869b0ec) SHA1(b8a56753257205af56e06105515b8a700bb1935b))
	ROM_LOAD("yrom2.snd",0xc000,0x2000, CRC(0528c024) SHA1(d24ff7e088b08c1f35b54be3c806f8a8757d96c7))
ROM_END

ROM_START(genesispg)
	ROM_REGION(0x10000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD("prom2g.cpu", 0x1000, 0x0800, CRC(e8fc30af) SHA1(2401bff3cf566cae4e6de6167fa004c5fe232928))
	ROM_RELOAD(0x5000, 0x0800)
	ROM_RELOAD(0x9000, 0x0800)
	ROM_RELOAD(0xd000, 0x0800)
	ROM_LOAD("prom1g.cpu", 0x2000, 0x2000, CRC(68a27ec1) SHA1(b14a933e6c7e2972faef8dfecebabe3da4021367))
	ROM_RELOAD(0x6000, 0x2000)
	ROM_RELOAD(0xa000, 0x2000)
	ROM_RELOAD(0xe000, 0x2000)

	ROM_REGION(0x10000, "r2sound:audiocpu", ROMREGION_ERASEFF)
	ROM_LOAD("drom1.snd",0xc000,0x2000, CRC(758e1743) SHA1(6df3011c044796afcd88e52d1ca69692cb489ff4))

	ROM_REGION(0x10000, "r2sound:speechcpu", ROMREGION_ERASEFF)
	ROM_LOAD("yrom1.snd",0xe000,0x2000, CRC(4869b0ec) SHA1(b8a56753257205af56e06105515b8a700bb1935b))
	ROM_LOAD("yrom2.snd",0xc000,0x2000, CRC(0528c024) SHA1(d24ff7e088b08c1f35b54be3c806f8a8757d96c7))
ROM_END

/*-------------------------------------------------------------------
/ Gold Wings (#707)
/-------------------------------------------------------------------*/
ROM_START(goldwing)
	ROM_REGION(0x10000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD("prom2.cpu", 0x1000, 0x0800, CRC(a5318c20) SHA1(8b4dcf45b13657ff753237a2e7d0352fda7755ef))
	ROM_RELOAD(0x5000, 0x0800)
	ROM_RELOAD(0x9000, 0x0800)
	ROM_RELOAD(0xd000, 0x0800)
	ROM_LOAD("prom1.cpu", 0x2000, 0x2000, CRC(bf242185) SHA1(0bf231050aa29f8bba5cb478a815b3d83bad93b3))
	ROM_RELOAD(0x6000, 0x2000)
	ROM_RELOAD(0xa000, 0x2000)
	ROM_RELOAD(0xe000, 0x2000)

	ROM_REGION(0x10000, "r2sound:audiocpu", ROMREGION_ERASEFF)
	ROM_LOAD("drom1.snd",0xc000,0x2000, CRC(892dbb21) SHA1(e24611544693e95dd2b9c0f2532c4d1f0b8ac10c))

	ROM_REGION(0x10000, "r2sound:speechcpu", ROMREGION_ERASEFF)
	ROM_LOAD("yrom1.snd",0xe000,0x2000, CRC(e17e9b1f) SHA1(ada9a6139a13ef31173801d560ec732d5a285140))
	ROM_LOAD("yrom2.snd",0xc000,0x2000, CRC(4e482023) SHA1(62e97d229eb28ff67f0ebc4ee04c1b4918a4affe))
ROM_END

ROM_START(goldwingf)
	ROM_REGION(0x10000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD("prom2f.cpu", 0x1000, 0x0800, CRC(50337adf) SHA1(dc286d52e6872edd68af442cbd0442babc174b93))
	ROM_RELOAD(0x5000, 0x0800)
	ROM_RELOAD(0x9000, 0x0800)
	ROM_RELOAD(0xd000, 0x0800)
	ROM_LOAD("prom1f.cpu", 0x2000, 0x2000, CRC(ec046fc0) SHA1(856f09f420e0f37488b0a896a37fffad62f18d6d))
	ROM_RELOAD(0x6000, 0x2000)
	ROM_RELOAD(0xa000, 0x2000)
	ROM_RELOAD(0xe000, 0x2000)

	ROM_REGION(0x10000, "r2sound:audiocpu", ROMREGION_ERASEFF)
	ROM_LOAD("drom1.snd",0xc000,0x2000, CRC(892dbb21) SHA1(e24611544693e95dd2b9c0f2532c4d1f0b8ac10c))

	ROM_REGION(0x10000, "r2sound:speechcpu", ROMREGION_ERASEFF)
	ROM_LOAD("yrom1.snd",0xe000,0x2000, CRC(e17e9b1f) SHA1(ada9a6139a13ef31173801d560ec732d5a285140))
	ROM_LOAD("yrom2.snd",0xc000,0x2000, CRC(4e482023) SHA1(62e97d229eb28ff67f0ebc4ee04c1b4918a4affe))
ROM_END

ROM_START(goldwingg)
	ROM_REGION(0x10000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD("prom2g.cpu", 0x1000, 0x0800, CRC(f69c963c) SHA1(9e39344ecfcca1115e12c559c66eaa21716c0ce2))
	ROM_RELOAD(0x5000, 0x0800)
	ROM_RELOAD(0x9000, 0x0800)
	ROM_RELOAD(0xd000, 0x0800)
	ROM_LOAD("prom1g.cpu", 0x2000, 0x2000, CRC(a9349b2f) SHA1(836c86d8db8be5ac29013bbe4daec8d96d15fba0))
	ROM_RELOAD(0x6000, 0x2000)
	ROM_RELOAD(0xa000, 0x2000)
	ROM_RELOAD(0xe000, 0x2000)

	ROM_REGION(0x10000, "r2sound:audiocpu", ROMREGION_ERASEFF)
	ROM_LOAD("drom1.snd",0xc000,0x2000, CRC(892dbb21) SHA1(e24611544693e95dd2b9c0f2532c4d1f0b8ac10c))

	ROM_REGION(0x10000, "r2sound:speechcpu", ROMREGION_ERASEFF)
	ROM_LOAD("yrom1.snd",0xe000,0x2000, CRC(e17e9b1f) SHA1(ada9a6139a13ef31173801d560ec732d5a285140))
	ROM_LOAD("yrom2.snd",0xc000,0x2000, CRC(4e482023) SHA1(62e97d229eb28ff67f0ebc4ee04c1b4918a4affe))
ROM_END

/*-------------------------------------------------------------------
/ Hollywood Heat (#703)
/-------------------------------------------------------------------*/
ROM_START(hlywoodh)
	ROM_REGION(0x10000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD("prom2.cpu", 0x1000, 0x0800, CRC(a465e5f3) SHA1(56afa2f67aebcd17345bba76ecb814653719ee7b))
	ROM_RELOAD(0x5000, 0x0800)
	ROM_RELOAD(0x9000, 0x0800)
	ROM_RELOAD(0xd000, 0x0800)
	ROM_LOAD("prom1.cpu", 0x2000, 0x2000, CRC(0493e27a) SHA1(72c603cda3cc43ed0f841a9fcc6f40d020475e74))
	ROM_RELOAD(0x6000, 0x2000)
	ROM_RELOAD(0xa000, 0x2000)
	ROM_RELOAD(0xe000, 0x2000)

	ROM_REGION(0x10000, "r2sound:audiocpu", ROMREGION_ERASEFF)
	ROM_LOAD("drom1.snd",0xc000,0x2000, CRC(a698ec33) SHA1(e7c1d28279ec4f12095c3a106c6cefcc2a84b31e))

	ROM_REGION(0x10000, "r2sound:speechcpu", ROMREGION_ERASEFF)
	ROM_LOAD("yrom1.snd",0xe000,0x2000, CRC(9232591e) SHA1(72883e0c542c572226c6c654bea14749cc9e351f))
	ROM_LOAD("yrom2.snd",0xc000,0x2000, CRC(51709c2f) SHA1(5834d7b72bd36e30c87377dc7c3ad0cf26ff303a))
ROM_END

ROM_START(hlywoodhf)
	ROM_REGION(0x10000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD("prom2f.cpu", 0x1000, 0x0800, CRC(969ca81f) SHA1(2606a0f63434056c5d2b509a885c9919a7a5d70f))
	ROM_RELOAD(0x5000, 0x0800)
	ROM_RELOAD(0x9000, 0x0800)
	ROM_RELOAD(0xd000, 0x0800)
	ROM_LOAD("prom1f.cpu", 0x2000, 0x2000, CRC(ddc45d2d) SHA1(8bd50f3e0049fe322f7bc626d39f9787cfea1940))
	ROM_RELOAD(0x6000, 0x2000)
	ROM_RELOAD(0xa000, 0x2000)
	ROM_RELOAD(0xe000, 0x2000)

	ROM_REGION(0x10000, "r2sound:audiocpu", ROMREGION_ERASEFF)
	ROM_LOAD("drom1.snd",0xc000,0x2000, CRC(a698ec33) SHA1(e7c1d28279ec4f12095c3a106c6cefcc2a84b31e))

	ROM_REGION(0x10000, "r2sound:speechcpu", ROMREGION_ERASEFF)
	ROM_LOAD("yrom1.snd",0xe000,0x2000, CRC(9232591e) SHA1(72883e0c542c572226c6c654bea14749cc9e351f))
	ROM_LOAD("yrom2.snd",0xc000,0x2000, CRC(51709c2f) SHA1(5834d7b72bd36e30c87377dc7c3ad0cf26ff303a))
ROM_END

ROM_START(hlywoodhg)
	ROM_REGION(0x10000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD("prom2g.cpu", 0x1000, 0x0800, CRC(bf60b631) SHA1(944089895d4253dd094a8f6b7168f9e62a75568a))
	ROM_RELOAD(0x5000, 0x0800)
	ROM_RELOAD(0x9000, 0x0800)
	ROM_RELOAD(0xd000, 0x0800)
	ROM_LOAD("prom1g.cpu", 0x2000, 0x2000, CRC(0f212d15) SHA1(b671b8fbc50f5528f0de061c7695932035266a0e))
	ROM_RELOAD(0x6000, 0x2000)
	ROM_RELOAD(0xa000, 0x2000)
	ROM_RELOAD(0xe000, 0x2000)

	ROM_REGION(0x10000, "r2sound:audiocpu", ROMREGION_ERASEFF)
	ROM_LOAD("drom1.snd",0xc000,0x2000, CRC(a698ec33) SHA1(e7c1d28279ec4f12095c3a106c6cefcc2a84b31e))

	ROM_REGION(0x10000, "r2sound:speechcpu", ROMREGION_ERASEFF)
	ROM_LOAD("yrom1.snd",0xe000,0x2000, CRC(9232591e) SHA1(72883e0c542c572226c6c654bea14749cc9e351f))
	ROM_LOAD("yrom2.snd",0xc000,0x2000, CRC(51709c2f) SHA1(5834d7b72bd36e30c87377dc7c3ad0cf26ff303a))
ROM_END

/*-------------------------------------------------------------------
/ Hot Shots (#718)
/-------------------------------------------------------------------*/
ROM_START(hotshots)
	ROM_REGION(0x10000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD("prom2.cpu", 0x1000, 0x0800, CRC(7695c7db) SHA1(90188ff83b888262ba849e5af9d99145c5bc1c30))
	ROM_CONTINUE(0x9000, 0x0800)
	ROM_RELOAD(0x5000, 0x0800)
	ROM_CONTINUE(0xd000, 0x0800)
	ROM_LOAD("prom1.cpu", 0x2000, 0x2000, CRC(122ff4a8) SHA1(195392b9f2050b52392a123831bb7a9428087c1b))
	ROM_RELOAD(0x6000, 0x2000)
	ROM_RELOAD(0xa000, 0x2000)
	ROM_RELOAD(0xe000, 0x2000)

	ROM_REGION(0x10000, "p5sound:audiocpu", ROMREGION_ERASEFF)
	ROM_LOAD("drom1.snd", 0x8000, 0x8000, CRC(42c3cc3d) SHA1(26ca7f3a71b83df18ac6be1d1eb28da20120285e))

	ROM_REGION(0x10000, "p5sound:speechcpu", ROMREGION_ERASEFF)
	ROM_LOAD("yrom1.snd", 0x8000, 0x8000, CRC(2933a80e) SHA1(5982b9ed361d90f8ea47047fc29770ef142acbec))
ROM_END

ROM_START(hotshotsf)
	ROM_REGION(0x10000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD("prom2f.cpu", 0x1000, 0x0800, CRC(476e260c) SHA1(2b88920c77462d190f9b98aebf8fcb5c9e853ecd))
	ROM_CONTINUE(0x9000, 0x0800)
	ROM_RELOAD(0x5000, 0x0800)
	ROM_CONTINUE(0xd000, 0x0800)
	ROM_LOAD("prom1f.cpu", 0x2000, 0x2000, CRC(8d74aca7) SHA1(c25b015ad8a6fa142c7cb46e2ac0229eb00289cf))
	ROM_RELOAD(0x6000, 0x2000)
	ROM_RELOAD(0xa000, 0x2000)
	ROM_RELOAD(0xe000, 0x2000)

	ROM_REGION(0x10000, "p5sound:audiocpu", ROMREGION_ERASEFF)
	ROM_LOAD("drom1.snd", 0x8000, 0x8000, CRC(42c3cc3d) SHA1(26ca7f3a71b83df18ac6be1d1eb28da20120285e))

	ROM_REGION(0x10000, "p5sound:speechcpu", ROMREGION_ERASEFF)
	ROM_LOAD("yrom1.snd", 0x8000, 0x8000, CRC(2933a80e) SHA1(5982b9ed361d90f8ea47047fc29770ef142acbec))
ROM_END

ROM_START(hotshotsg)
	ROM_REGION(0x10000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD("prom2g.cpu", 0x1000, 0x0800, CRC(7e2f0d59) SHA1(b8a7b9be3e4d705631e017da87b27be53ed23f30))
	ROM_CONTINUE(0x9000, 0x0800)
	ROM_RELOAD(0x5000, 0x0800)
	ROM_CONTINUE(0xd000, 0x0800)
	ROM_LOAD("prom1g.cpu", 0x2000, 0x2000, CRC(e07b46ad) SHA1(c7b48dcfb074f3d0f38a6d49028ba172946467fc))
	ROM_RELOAD(0x6000, 0x2000)
	ROM_RELOAD(0xa000, 0x2000)
	ROM_RELOAD(0xe000, 0x2000)

	ROM_REGION(0x10000, "p5sound:audiocpu", ROMREGION_ERASEFF)
	ROM_LOAD("drom1.snd", 0x8000, 0x8000, CRC(42c3cc3d) SHA1(26ca7f3a71b83df18ac6be1d1eb28da20120285e))

	ROM_REGION(0x10000, "p5sound:speechcpu", ROMREGION_ERASEFF)
	ROM_LOAD("yrom1.snd", 0x8000, 0x8000, CRC(2933a80e) SHA1(5982b9ed361d90f8ea47047fc29770ef142acbec))
ROM_END

/*-------------------------------------------------------------------
/ Monte Carlo (#708)
/-------------------------------------------------------------------*/
ROM_START(mntecrlo)
	ROM_REGION(0x10000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD("prom2.cpu", 0x1000, 0x0800, CRC(6860e315) SHA1(cecb1815334506dfebf29efe3e4e2a838010e8db))
	ROM_RELOAD(0x5000, 0x0800)
	ROM_RELOAD(0x9000, 0x0800)
	ROM_RELOAD(0xd000, 0x0800)
	ROM_LOAD("prom1.cpu", 0x2000, 0x2000, CRC(0fbf15a3) SHA1(0155b39c2c38224301857313ab784c1d39f1183b))
	ROM_RELOAD(0x6000, 0x2000)
	ROM_RELOAD(0xa000, 0x2000)
	ROM_RELOAD(0xe000, 0x2000)

	ROM_REGION(0x10000, "r2sound:audiocpu", ROMREGION_ERASEFF)
	ROM_LOAD("drom1.snd",0xc000,0x2000, CRC(1a53ac15) SHA1(f2751664a09431e908873580ddf4f44df9b4eda7))

	ROM_REGION(0x10000, "r2sound:speechcpu", ROMREGION_ERASEFF)
	ROM_LOAD("yrom1.snd",0xe000,0x2000, CRC(6e234c49) SHA1(fdb4126ecdaac378d144e9dd3c29b4e79290da2a))
	ROM_LOAD("yrom2.snd",0xc000,0x2000, CRC(a95d1a6b) SHA1(91946ef7af0e4dd96db6d2d6f4f2e9a3a7279b81))
ROM_END

ROM_START(mntecrloa)
	ROM_REGION(0x10000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD("prom2a.cpu", 0x1000, 0x0800, CRC(5dd75c06) SHA1(911f7e56b7602c9bc9b51dde7719d3e0562f0702))
	ROM_RELOAD(0x5000, 0x0800)
	ROM_RELOAD(0x9000, 0x0800)
	ROM_RELOAD(0xd000, 0x0800)
	ROM_LOAD("prom1a.cpu", 0x2000, 0x2000, CRC(de980755) SHA1(0df99526a432e26fb73288b529dc0f4f49623e81))
	ROM_RELOAD(0x6000, 0x2000)
	ROM_RELOAD(0xa000, 0x2000)
	ROM_RELOAD(0xe000, 0x2000)

	ROM_REGION(0x10000, "r2sound:audiocpu", ROMREGION_ERASEFF)
	ROM_LOAD("drom1.snd",0xc000,0x2000, CRC(1a53ac15) SHA1(f2751664a09431e908873580ddf4f44df9b4eda7))

	ROM_REGION(0x10000, "r2sound:speechcpu", ROMREGION_ERASEFF)
	ROM_LOAD("yrom1.snd",0xe000,0x2000, CRC(6e234c49) SHA1(fdb4126ecdaac378d144e9dd3c29b4e79290da2a))
	ROM_LOAD("yrom2.snd",0xc000,0x2000, CRC(a95d1a6b) SHA1(91946ef7af0e4dd96db6d2d6f4f2e9a3a7279b81))
ROM_END

ROM_START(mntecrlof)
	ROM_REGION(0x10000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD("prom2f.cpu", 0x1000, 0x0800, CRC(f6842631) SHA1(7447994d2055c7fa12aaf35e93436ee829f5b7ae))
	ROM_RELOAD(0x5000, 0x0800)
	ROM_RELOAD(0x9000, 0x0800)
	ROM_RELOAD(0xd000, 0x0800)
	ROM_LOAD("prom1f.cpu", 0x2000, 0x2000, CRC(33a8dbc9) SHA1(5ef586e2b1ba7f245723584bc14c60c2860d19fc))
	ROM_RELOAD(0x6000, 0x2000)
	ROM_RELOAD(0xa000, 0x2000)
	ROM_RELOAD(0xe000, 0x2000)

	ROM_REGION(0x10000, "r2sound:audiocpu", ROMREGION_ERASEFF)
	ROM_LOAD("drom1.snd",0xc000,0x2000, CRC(1a53ac15) SHA1(f2751664a09431e908873580ddf4f44df9b4eda7))

	ROM_REGION(0x10000, "r2sound:speechcpu", ROMREGION_ERASEFF)
	ROM_LOAD("yrom1.snd",0xe000,0x2000, CRC(6e234c49) SHA1(fdb4126ecdaac378d144e9dd3c29b4e79290da2a))
	ROM_LOAD("yrom2.snd",0xc000,0x2000, CRC(a95d1a6b) SHA1(91946ef7af0e4dd96db6d2d6f4f2e9a3a7279b81))
ROM_END

ROM_START(mntecrlog)
	ROM_REGION(0x10000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD("prom2g.cpu", 0x1000, 0x0800, CRC(2a5e0c4f) SHA1(b386168bd911b9977104c47da962d0248f22614b))
	ROM_RELOAD(0x5000, 0x0800)
	ROM_RELOAD(0x9000, 0x0800)
	ROM_RELOAD(0xd000, 0x0800)
	ROM_LOAD("prom1g.cpu", 0x2000, 0x2000, CRC(25e015f1) SHA1(4b1467438def657eac3b8a858d7b17c102e14f0d))
	ROM_RELOAD(0x6000, 0x2000)
	ROM_RELOAD(0xa000, 0x2000)
	ROM_RELOAD(0xe000, 0x2000)

	ROM_REGION(0x10000, "r2sound:audiocpu", ROMREGION_ERASEFF)
	ROM_LOAD("drom1.snd",0xc000,0x2000, CRC(1a53ac15) SHA1(f2751664a09431e908873580ddf4f44df9b4eda7))

	ROM_REGION(0x10000, "r2sound:speechcpu", ROMREGION_ERASEFF)
	ROM_LOAD("yrom1.snd",0xe000,0x2000, CRC(6e234c49) SHA1(fdb4126ecdaac378d144e9dd3c29b4e79290da2a))
	ROM_LOAD("yrom2.snd",0xc000,0x2000, CRC(a95d1a6b) SHA1(91946ef7af0e4dd96db6d2d6f4f2e9a3a7279b81))
ROM_END

ROM_START(mntecrlo2)
	ROM_REGION(0x10000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD("prom2_2.cpu", 0x1000, 0x0800, CRC(8e72a68f) SHA1(8320c44020f7d5f9e887b17556252f1c617235ac))
	ROM_RELOAD(0x5000, 0x0800)
	ROM_RELOAD(0x9000, 0x0800)
	ROM_RELOAD(0xd000, 0x0800)
	ROM_LOAD("prom1_2.cpu", 0x2000, 0x2000, CRC(9bd6a010) SHA1(680ce076452ab3fd911fa58fc48c07ea2ec793da))
	ROM_RELOAD(0x6000, 0x2000)
	ROM_RELOAD(0xa000, 0x2000)
	ROM_RELOAD(0xe000, 0x2000)

	ROM_REGION(0x10000, "r2sound:audiocpu", ROMREGION_ERASEFF)
	ROM_LOAD("drom1.snd",0xc000,0x2000, CRC(1a53ac15) SHA1(f2751664a09431e908873580ddf4f44df9b4eda7))

	ROM_REGION(0x10000, "r2sound:speechcpu", ROMREGION_ERASEFF)
	ROM_LOAD("yrom1.snd",0xe000,0x2000, CRC(6e234c49) SHA1(fdb4126ecdaac378d144e9dd3c29b4e79290da2a))
	ROM_LOAD("yrom2.snd",0xc000,0x2000, CRC(a95d1a6b) SHA1(91946ef7af0e4dd96db6d2d6f4f2e9a3a7279b81))
ROM_END

/*-------------------------------------------------------------------
/ Night Moves (#C101)
/ One-sided cocktail designed and built by Gottlieb for International Concepts
/-------------------------------------------------------------------*/
ROM_START(nmoves)
	ROM_REGION(0x10000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD("nmovsp2.732", 0x1000, 0x0800, CRC(a2bc00e4) SHA1(5c3e9033f5c72b87058b2f70a0ff0811cc6770fa))
	ROM_CONTINUE(0x9000, 0x0800)
	ROM_RELOAD(0x5000, 0x0800)
	ROM_CONTINUE(0xd000, 0x0800)
	ROM_LOAD("nmovsp1.764", 0x2000, 0x2000, CRC(36837146) SHA1(88312ae1d1fe76defc4aa2d0a0570c5bb56253e9))
	ROM_RELOAD(0x6000, 0x2000)
	ROM_RELOAD(0xa000, 0x2000)
	ROM_RELOAD(0xe000, 0x2000)

	ROM_REGION(0x10000, "p5sound:audiocpu", ROMREGION_ERASEFF)
	ROM_LOAD("nmovdrom.256", 0x8000, 0x8000, CRC(90929841) SHA1(e203ccd3552c9843c91fc49a437f60ae2dd49142))

	ROM_REGION(0x10000, "p5sound:speechcpu", ROMREGION_ERASEFF)
	ROM_LOAD("nmovyrom.256", 0x8000, 0x8000, CRC(cb74a687) SHA1(af8275807491eb35643cdeb6c898025fde47ceac))
ROM_END

/*-------------------------------------------------------------------
/ Raven (#702)
/-------------------------------------------------------------------*/
ROM_START(raven)
	ROM_REGION(0x10000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD("prom2.cpu", 0x1000, 0x0800, CRC(481f3fb8) SHA1(22ffa55ed362219ebedbc40edcf866ff152a01b9))
	ROM_RELOAD(0x5000, 0x0800)
	ROM_RELOAD(0x9000, 0x0800)
	ROM_RELOAD(0xd000, 0x0800)
	ROM_LOAD("prom1.cpu", 0x2000, 0x2000, CRC(edc88561) SHA1(101878527307c6f04d141dd74e04102c4ea53105))
	ROM_RELOAD(0x6000, 0x2000)
	ROM_RELOAD(0xa000, 0x2000)
	ROM_RELOAD(0xe000, 0x2000)

	ROM_REGION(0x10000, "r2sound:audiocpu", ROMREGION_ERASEFF)
	ROM_LOAD("drom1.snd",0xc000,0x2000, CRC(a04bf7d0) SHA1(5be5d445b199e7dc9d42e7ee5e9b31c18dec3881))

	ROM_REGION(0x10000, "r2sound:speechcpu", ROMREGION_ERASEFF)
	ROM_LOAD("yrom1.snd",0xe000,0x2000, CRC(ee5f868b) SHA1(23ef4112b94109ad4d4a6b9bb5215acec20e5e55))
ROM_END

ROM_START(raveng)
	ROM_REGION(0x10000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD("prom2g.cpu", 0x1000, 0x0800, CRC(4ca540a5) SHA1(50bb240465d80b7763574e1261f8d0ddda5ad587))
	ROM_RELOAD(0x5000, 0x0800)
	ROM_RELOAD(0x9000, 0x0800)
	ROM_RELOAD(0xd000, 0x0800)
	ROM_LOAD("prom1g.cpu", 0x2000, 0x2000, CRC(3441aeda) SHA1(12dd2faac64170bad5cf5b9247283f64df9e5337))
	ROM_RELOAD(0x6000, 0x2000)
	ROM_RELOAD(0xa000, 0x2000)
	ROM_RELOAD(0xe000, 0x2000)

	ROM_REGION(0x10000, "r2sound:audiocpu", ROMREGION_ERASEFF)
	ROM_LOAD("drom1.snd",0xc000,0x2000, CRC(a04bf7d0) SHA1(5be5d445b199e7dc9d42e7ee5e9b31c18dec3881))

	ROM_REGION(0x10000, "r2sound:speechcpu", ROMREGION_ERASEFF)
	ROM_LOAD("yrom1.snd",0xe000,0x2000, CRC(ee5f868b) SHA1(23ef4112b94109ad4d4a6b9bb5215acec20e5e55))
ROM_END

ROM_START(ravena)
	ROM_REGION(0x10000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD("prom2a.cpu", 0x1000, 0x0800, CRC(a693785e) SHA1(7c8878f1c3c5205b3ae46a78c881bbd2b722838d))
	ROM_RELOAD(0x5000, 0x0800)
	ROM_RELOAD(0x9000, 0x0800)
	ROM_RELOAD(0xd000, 0x0800)
	ROM_LOAD("prom1.cpu", 0x2000, 0x2000, CRC(edc88561) SHA1(101878527307c6f04d141dd74e04102c4ea53105))
	ROM_RELOAD(0x6000, 0x2000)
	ROM_RELOAD(0xa000, 0x2000)
	ROM_RELOAD(0xe000, 0x2000)

	ROM_REGION(0x10000, "r2sound:audiocpu", ROMREGION_ERASEFF)
	ROM_LOAD("drom1.snd",0xc000,0x2000, CRC(a04bf7d0) SHA1(5be5d445b199e7dc9d42e7ee5e9b31c18dec3881))

	ROM_REGION(0x10000, "r2sound:speechcpu", ROMREGION_ERASEFF)
	ROM_LOAD("yrom1.snd",0xe000,0x2000, CRC(ee5f868b) SHA1(23ef4112b94109ad4d4a6b9bb5215acec20e5e55))
ROM_END

/*-------------------------------------------------------------------
/ Robo-War (#714)
/-------------------------------------------------------------------*/
ROM_START(robowars)
	ROM_REGION(0x10000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD("prom2.cpu", 0x1000, 0x0800, CRC(893177ed) SHA1(791540a64d498979e5b0c8baf4ceb2fd5ff7f047))
	ROM_RELOAD(0x5000, 0x0800)
	ROM_RELOAD(0x9000, 0x0800)
	ROM_RELOAD(0xd000, 0x0800)
	ROM_LOAD("prom1.cpu", 0x2000, 0x2000, CRC(cd1587d8) SHA1(77e8e02dc03d052e9e4ce19c9431439e4211a29f))
	ROM_RELOAD(0x6000, 0x2000)
	ROM_RELOAD(0xa000, 0x2000)
	ROM_RELOAD(0xe000, 0x2000)

	ROM_REGION(0x10000, "p4sound:audiocpu", ROMREGION_ERASEFF)
	ROM_LOAD("drom1.snd", 0x8000, 0x8000, CRC(ea59b6a1) SHA1(6a4cdd37ba85f94f703afd1c5d3f102f51fedf46))

	ROM_REGION(0x10000, "p4sound:speechcpu", ROMREGION_ERASEFF)
	ROM_LOAD("yrom1.snd", 0x8000, 0x8000, CRC(7ecd8b67) SHA1(c5167b0acc64e535d389ba70be92a65672e119f6))
ROM_END

ROM_START(robowarsf)
	ROM_REGION(0x10000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD("prom2f.cpu", 0x1000, 0x0800, CRC(1afa0e69) SHA1(178813494b877ac9ca36863661596b4df04df1bb))
	ROM_RELOAD(0x5000, 0x0800)
	ROM_RELOAD(0x9000, 0x0800)
	ROM_RELOAD(0xd000, 0x0800)
	ROM_LOAD("prom1f.cpu", 0x2000, 0x2000, CRC(263cb8f9) SHA1(ba27ca0618b9ed68c258a654bdd00a24f8413239))
	ROM_RELOAD(0x6000, 0x2000)
	ROM_RELOAD(0xa000, 0x2000)
	ROM_RELOAD(0xe000, 0x2000)

	ROM_REGION(0x10000, "p4sound:audiocpu", ROMREGION_ERASEFF)
	ROM_LOAD("drom1.snd", 0x8000, 0x8000, CRC(ea59b6a1) SHA1(6a4cdd37ba85f94f703afd1c5d3f102f51fedf46))

	ROM_REGION(0x10000, "p4sound:speechcpu", ROMREGION_ERASEFF)
	ROM_LOAD("yrom1.snd", 0x8000, 0x8000, CRC(7ecd8b67) SHA1(c5167b0acc64e535d389ba70be92a65672e119f6))
ROM_END

/*-------------------------------------------------------------------
/ Rock (#697)
/-------------------------------------------------------------------*/
ROM_START(rock)
	ROM_REGION(0x10000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD("prom1.cpu", 0x2000, 0x2000, CRC(1146c1d3) SHA1(1e838756017cdc51239c082f8d491cd2824d273d))
	ROM_RELOAD(0x6000, 0x2000)
	ROM_RELOAD(0xa000, 0x2000)
	ROM_RELOAD(0xe000, 0x2000)

	ROM_REGION(0x10000, "r2sound:audiocpu", ROMREGION_ERASEFF)
	ROM_LOAD("drom1.snd",0xc000,0x2000, CRC(03830e81) SHA1(786f85eba5a8f5e9cc659305623e1d178b5410f6))

	ROM_REGION(0x10000, "r2sound:speechcpu", ROMREGION_ERASEFF)
	ROM_LOAD("yrom1.snd",0xe000,0x2000, CRC(effba2ad) SHA1(2288a4f655376e0aa18f8ecd9a3818ed4d6c6891))
ROM_END

ROM_START(rockg)
	ROM_REGION(0x10000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD("prom1g.cpu", 0x2000, 0x2000, CRC(2de3f1e5) SHA1(ceb964292703080bb742dbc073a14dbf745ad38e))
	ROM_RELOAD(0x6000, 0x2000)
	ROM_RELOAD(0xa000, 0x2000)
	ROM_RELOAD(0xe000, 0x2000)

	ROM_REGION(0x10000, "r2sound:audiocpu", ROMREGION_ERASEFF)
	ROM_LOAD("drom1.snd",0xc000,0x2000, CRC(03830e81) SHA1(786f85eba5a8f5e9cc659305623e1d178b5410f6))

	ROM_REGION(0x10000, "r2sound:speechcpu", ROMREGION_ERASEFF)
	ROM_LOAD("yrom1.snd",0xe000,0x2000, CRC(effba2ad) SHA1(2288a4f655376e0aa18f8ecd9a3818ed4d6c6891))
ROM_END

/*-------------------------------------------------------------------
/ Rock Encore (#704)
/-------------------------------------------------------------------*/
ROM_START(rock_enc)
	ROM_REGION(0x10000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD("prom1.cpu", 0x2000, 0x2000, CRC(1146c1d3) SHA1(1e838756017cdc51239c082f8d491cd2824d273d))
	ROM_RELOAD(0x6000, 0x2000)
	ROM_RELOAD(0xa000, 0x2000)
	ROM_RELOAD(0xe000, 0x2000)

	ROM_REGION(0x10000, "r2sound:audiocpu", ROMREGION_ERASEFF)
	ROM_LOAD("drom1a.snd",0xc000,0x2000, CRC(b8aa8912) SHA1(abff690256c0030807b2d4dfa0516496516384e8))

	ROM_REGION(0x10000, "r2sound:speechcpu", ROMREGION_ERASEFF)
	ROM_LOAD("yrom1a.snd",0xe000,0x2000, CRC(a62e3b94) SHA1(59636c2ac7ebbd116a0eb39479c97299ba391906))
	ROM_LOAD("yrom2a.snd",0xc000,0x2000, CRC(66645a3f) SHA1(f06261af81e6b1829d639933297d2461a8c993fc))
ROM_END

ROM_START(rock_encg)
	ROM_REGION(0x10000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD("prom1g.cpu", 0x2000, 0x2000, CRC(2de3f1e5) SHA1(ceb964292703080bb742dbc073a14dbf745ad38e))
	ROM_RELOAD(0x6000, 0x2000)
	ROM_RELOAD(0xa000, 0x2000)
	ROM_RELOAD(0xe000, 0x2000)

	ROM_REGION(0x10000, "r2sound:audiocpu", ROMREGION_ERASEFF)
	ROM_LOAD("drom1a.snd",0xc000,0x2000, CRC(b8aa8912) SHA1(abff690256c0030807b2d4dfa0516496516384e8))

	ROM_REGION(0x10000, "r2sound:speechcpu", ROMREGION_ERASEFF)
	ROM_LOAD("yrom1a.snd",0xe000,0x2000, CRC(a62e3b94) SHA1(59636c2ac7ebbd116a0eb39479c97299ba391906))
	ROM_LOAD("yrom2a.snd",0xc000,0x2000, CRC(66645a3f) SHA1(f06261af81e6b1829d639933297d2461a8c993fc))
ROM_END

/*-------------------------------------------------------------------
/ Spring Break (#706)
/-------------------------------------------------------------------*/
ROM_START(sprbreak)
	ROM_REGION(0x10000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD("prom2.cpu", 0x1000, 0x0800, CRC(47171062) SHA1(0d2e7777f695ab22170be861019c05ddeade5f85))
	ROM_RELOAD(0x5000, 0x0800)
	ROM_RELOAD(0x9000, 0x0800)
	ROM_RELOAD(0xd000, 0x0800)
	ROM_LOAD("prom1.cpu", 0x2000, 0x2000, CRC(53ed608b) SHA1(555a6c02d637ea03e8265bb2b0fba95f2e2584b3))
	ROM_RELOAD(0x6000, 0x2000)
	ROM_RELOAD(0xa000, 0x2000)
	ROM_RELOAD(0xe000, 0x2000)

	ROM_REGION(0x10000, "r2sound:audiocpu", ROMREGION_ERASEFF)
	ROM_LOAD("drom1.snd",0xc000,0x2000, CRC(97d3f9ba) SHA1(1b34c7e51373c26d29d757c57a2b0333fe38d19e))

	ROM_REGION(0x10000, "r2sound:speechcpu", ROMREGION_ERASEFF)
	ROM_LOAD("yrom1.snd",0xe000,0x2000, CRC(5ea89df9) SHA1(98ce7661a4d862fd02c77e69b0f6e9372c3ade2b))
	ROM_LOAD("yrom2.snd",0xc000,0x2000, CRC(0fb0128e) SHA1(3bdc5ed11b8e062f71f2a78b955830bd985e80a3))
ROM_END

ROM_START(sprbreaka)
	ROM_REGION(0x10000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD("prom2a.cpu", 0x1000, 0x0800, CRC(d9d841b4) SHA1(8b9773e5ae9917d27089deca3b8311cb74e7f88e))
	ROM_RELOAD(0x5000, 0x0800)
	ROM_RELOAD(0x9000, 0x0800)
	ROM_RELOAD(0xd000, 0x0800)
	ROM_LOAD("prom1a.cpu", 0x2000, 0x2000, CRC(93db71e9) SHA1(59f75c4ef2c36b4f1f94dd365f2df82e7bcf53f8))
	ROM_RELOAD(0x6000, 0x2000)
	ROM_RELOAD(0xa000, 0x2000)
	ROM_RELOAD(0xe000, 0x2000)

	ROM_REGION(0x10000, "r2sound:audiocpu", ROMREGION_ERASEFF)
	ROM_LOAD("drom1.snd",0xc000,0x2000, CRC(97d3f9ba) SHA1(1b34c7e51373c26d29d757c57a2b0333fe38d19e))

	ROM_REGION(0x10000, "r2sound:speechcpu", ROMREGION_ERASEFF)
	ROM_LOAD("yrom1.snd",0xe000,0x2000, CRC(5ea89df9) SHA1(98ce7661a4d862fd02c77e69b0f6e9372c3ade2b))
	ROM_LOAD("yrom2.snd",0xc000,0x2000, CRC(0fb0128e) SHA1(3bdc5ed11b8e062f71f2a78b955830bd985e80a3))
ROM_END

ROM_START(sprbreakf)
	ROM_REGION(0x10000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD("prom2f.cpu", 0x1000, 0x0800, CRC(c0ee0555) SHA1(3d2aef5a8a6452f9f87b4ec2040643dda5843ebd))
	ROM_RELOAD(0x5000, 0x0800)
	ROM_RELOAD(0x9000, 0x0800)
	ROM_RELOAD(0xd000, 0x0800)
	ROM_LOAD("prom1f.cpu", 0x2000, 0x2000, CRC(608cf4d5) SHA1(41193eb036da7c7d05f313d1a68723504a7a90f4))
	ROM_RELOAD(0x6000, 0x2000)
	ROM_RELOAD(0xa000, 0x2000)
	ROM_RELOAD(0xe000, 0x2000)

	ROM_REGION(0x10000, "r2sound:audiocpu", ROMREGION_ERASEFF)
	ROM_LOAD("drom1.snd",0xc000,0x2000, CRC(97d3f9ba) SHA1(1b34c7e51373c26d29d757c57a2b0333fe38d19e))

	ROM_REGION(0x10000, "r2sound:speechcpu", ROMREGION_ERASEFF)
	ROM_LOAD("yrom1.snd",0xe000,0x2000, CRC(5ea89df9) SHA1(98ce7661a4d862fd02c77e69b0f6e9372c3ade2b))
	ROM_LOAD("yrom2.snd",0xc000,0x2000, CRC(0fb0128e) SHA1(3bdc5ed11b8e062f71f2a78b955830bd985e80a3))
ROM_END

ROM_START(sprbreakg)
	ROM_REGION(0x10000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD("prom2g.cpu", 0x1000, 0x0800, CRC(fa4b750d) SHA1(89f797f65fc18473419080810bca4590f77e2502))
	ROM_RELOAD(0x5000, 0x0800)
	ROM_RELOAD(0x9000, 0x0800)
	ROM_RELOAD(0xd000, 0x0800)
	ROM_LOAD("prom1g.cpu", 0x2000, 0x2000, CRC(2d9c4640) SHA1(3671a962334f5c84ae2635891ee90c62be69da5c))
	ROM_RELOAD(0x6000, 0x2000)
	ROM_RELOAD(0xa000, 0x2000)
	ROM_RELOAD(0xe000, 0x2000)

	ROM_REGION(0x10000, "r2sound:audiocpu", ROMREGION_ERASEFF)
	ROM_LOAD("drom1.snd",0xc000,0x2000, CRC(97d3f9ba) SHA1(1b34c7e51373c26d29d757c57a2b0333fe38d19e))

	ROM_REGION(0x10000, "r2sound:speechcpu", ROMREGION_ERASEFF)
	ROM_LOAD("yrom1.snd",0xe000,0x2000, CRC(5ea89df9) SHA1(98ce7661a4d862fd02c77e69b0f6e9372c3ade2b))
	ROM_LOAD("yrom2.snd",0xc000,0x2000, CRC(0fb0128e) SHA1(3bdc5ed11b8e062f71f2a78b955830bd985e80a3))
ROM_END

ROM_START(sprbreaks)
	ROM_REGION(0x10000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD("prom2.rv2", 0x1000, 0x0800, CRC(911cd14f) SHA1(2bc3ff6a3889da69b97f8ec318f93208e3d42cfe))
	ROM_RELOAD(0x5000, 0x0800)
	ROM_RELOAD(0x9000, 0x0800)
	ROM_RELOAD(0xd000, 0x0800)
	ROM_LOAD("prom1.rv2", 0x2000, 0x2000, CRC(d67d9d2f) SHA1(ebb82f0a1b7d6a2ec2607d4000e58fb6bfa73fe7))
	ROM_RELOAD(0x6000, 0x2000)
	ROM_RELOAD(0xa000, 0x2000)
	ROM_RELOAD(0xe000, 0x2000)

	ROM_REGION(0x10000, "r2sound:audiocpu", ROMREGION_ERASEFF)
	ROM_LOAD("drom1.snd",0xc000,0x2000, CRC(97d3f9ba) SHA1(1b34c7e51373c26d29d757c57a2b0333fe38d19e))

	ROM_REGION(0x10000, "r2sound:speechcpu", ROMREGION_ERASEFF)
	ROM_LOAD("yrom1.snd",0xe000,0x2000, CRC(5ea89df9) SHA1(98ce7661a4d862fd02c77e69b0f6e9372c3ade2b))
	ROM_LOAD("yrom2.snd",0xc000,0x2000, CRC(0fb0128e) SHA1(3bdc5ed11b8e062f71f2a78b955830bd985e80a3))
ROM_END

/*-------------------------------------------------------------------
/ Tag-Team Wrestling (#698)
/-------------------------------------------------------------------*/
ROM_START(tagteamp)
	ROM_REGION(0x10000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD("prom2.cpu", 0x1000, 0x0800, CRC(fd1615ce) SHA1(3a6c3525552286b86e5340af2bf196f12adc9b35))
	ROM_RELOAD(0x5000, 0x0800)
	ROM_RELOAD(0x9000, 0x0800)
	ROM_RELOAD(0xd000, 0x0800)
	ROM_LOAD("prom1.cpu", 0x2000, 0x2000, CRC(65931038) SHA1(6d2f1a9fb1b3ce4610074fd3f2ac37ad6af70a44))
	ROM_RELOAD(0x6000, 0x2000)
	ROM_RELOAD(0xa000, 0x2000)
	ROM_RELOAD(0xe000, 0x2000)

	ROM_REGION(0x1000, "p3sound:audiocpu", ROMREGION_ERASEFF)
	ROM_LOAD("698-s.snd", 0x0800, 0x0800, CRC(9c8191b7) SHA1(12b017692f078dcdc8e4bbf1ffcea1c5d0293d06))
ROM_END

ROM_START(tagteampg)
	ROM_REGION(0x10000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD("prom2g.cpu", 0x1000, 0x0800, CRC(5e6d2da7) SHA1(9b23d1ac34163edeaceffe806a2a559f3d408b41))
	ROM_RELOAD(0x5000, 0x0800)
	ROM_RELOAD(0x9000, 0x0800)
	ROM_RELOAD(0xd000, 0x0800)
	ROM_LOAD("prom1g.cpu", 0x2000, 0x2000, CRC(e206c519) SHA1(0d5b3237807b6f11633ab9be2b0e5b000369a0e8))
	ROM_RELOAD(0x6000, 0x2000)
	ROM_RELOAD(0xa000, 0x2000)
	ROM_RELOAD(0xe000, 0x2000)

	ROM_REGION(0x1000, "p3sound:audiocpu", ROMREGION_ERASEFF)
	ROM_LOAD("698-s.snd", 0x0800, 0x0800, CRC(9c8191b7) SHA1(12b017692f078dcdc8e4bbf1ffcea1c5d0293d06))
ROM_END

ROM_START(tagteamp2)
	ROM_REGION(0x10000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD("prom2a.cpu", 0x1000, 0x0800, CRC(6d56b636) SHA1(8f50f2742be727835e7343307787b4b5daa1623a))
	ROM_RELOAD(0x5000, 0x0800)
	ROM_RELOAD(0x9000, 0x0800)
	ROM_RELOAD(0xd000, 0x0800)
	ROM_LOAD("prom1a.cpu", 0x2000, 0x2000, CRC(92766607) SHA1(29744dd3c447cc51fb123750ae1456329122e986))
	ROM_RELOAD(0x6000, 0x2000)
	ROM_RELOAD(0xa000, 0x2000)
	ROM_RELOAD(0xe000, 0x2000)

	ROM_REGION(0x1000, "p3sound:audiocpu", ROMREGION_ERASEFF)
	ROM_LOAD("698-s.snd", 0x0800, 0x0800, CRC(9c8191b7) SHA1(12b017692f078dcdc8e4bbf1ffcea1c5d0293d06))
ROM_END

/*-------------------------------------------------------------------
/ TX-Sector (#712)
/-------------------------------------------------------------------*/
ROM_START(txsector)
	ROM_REGION(0x10000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD("prom2.cpu", 0x1000, 0x0800, CRC(f12514e6) SHA1(80bca17c33df99ed1a7acc21f7f70ea90e7c0463))
	ROM_RELOAD(0x5000, 0x0800)
	ROM_RELOAD(0x9000, 0x0800)
	ROM_RELOAD(0xd000, 0x0800)
	ROM_LOAD("prom1.cpu", 0x2000, 0x2000, CRC(e51d39da) SHA1(b6e4d573b62cc441a153cc4d8b647ee46b4dd2a7))
	ROM_RELOAD(0x6000, 0x2000)
	ROM_RELOAD(0xa000, 0x2000)
	ROM_RELOAD(0xe000, 0x2000)

	ROM_REGION(0x10000, "p4sound:audiocpu", ROMREGION_ERASEFF)
	ROM_LOAD("drom1.snd", 0x8000, 0x8000, CRC(61d66ca1) SHA1(59b1705b13d46b29f45257c566274f3cdce15ec2))

	ROM_REGION(0x10000, "p4sound:speechcpu", ROMREGION_ERASEFF)
	ROM_LOAD("yrom1.snd", 0x8000, 0x8000, CRC(469ef444) SHA1(faa16f34357a53c3fc61b59251fabdc44c605000))
ROM_END

ROM_START(txsectorf)
	ROM_REGION(0x10000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD("prom2f.cpu", 0x1000, 0x0800, CRC(1bd08247) SHA1(968cc30e5e5c783e73cb3278a58189c4f8b8186f))
	ROM_RELOAD(0x5000, 0x0800)
	ROM_RELOAD(0x9000, 0x0800)
	ROM_RELOAD(0xd000, 0x0800)
	ROM_LOAD("prom1f.cpu", 0x2000, 0x2000, CRC(8df27155) SHA1(67aeeab0d50e43674082e1dd99a849db64ba00b2))
	ROM_RELOAD(0x6000, 0x2000)
	ROM_RELOAD(0xa000, 0x2000)
	ROM_RELOAD(0xe000, 0x2000)

	ROM_REGION(0x10000, "p4sound:audiocpu", ROMREGION_ERASEFF)
	ROM_LOAD("drom1.snd", 0x8000, 0x8000, CRC(61d66ca1) SHA1(59b1705b13d46b29f45257c566274f3cdce15ec2))

	ROM_REGION(0x10000, "p4sound:speechcpu", ROMREGION_ERASEFF)
	ROM_LOAD("yrom1.snd", 0x8000, 0x8000, CRC(469ef444) SHA1(faa16f34357a53c3fc61b59251fabdc44c605000))
ROM_END

ROM_START(txsectorg)
	ROM_REGION(0x10000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD("prom2g.cpu", 0x1000, 0x0800, CRC(2b17261f) SHA1(a3195190c0d5116b60e487a7b7f3a28c1f110e89))
	ROM_RELOAD(0x5000, 0x0800)
	ROM_RELOAD(0x9000, 0x0800)
	ROM_RELOAD(0xd000, 0x0800)
	ROM_LOAD("prom1g.cpu", 0x2000, 0x2000, CRC(83ea2f11) SHA1(ac3570597512c71c099aa15f0750a12a3e206b83))
	ROM_RELOAD(0x6000, 0x2000)
	ROM_RELOAD(0xa000, 0x2000)
	ROM_RELOAD(0xe000, 0x2000)

	ROM_REGION(0x10000, "p4sound:audiocpu", ROMREGION_ERASEFF)
	ROM_LOAD("drom1.snd", 0x8000, 0x8000, CRC(61d66ca1) SHA1(59b1705b13d46b29f45257c566274f3cdce15ec2))

	ROM_REGION(0x10000, "p4sound:speechcpu", ROMREGION_ERASEFF)
	ROM_LOAD("yrom1.snd", 0x8000, 0x8000, CRC(469ef444) SHA1(faa16f34357a53c3fc61b59251fabdc44c605000))
ROM_END

/*-------------------------------------------------------------------
/ Victory (#710)
/-------------------------------------------------------------------*/
ROM_START(victoryp)
	ROM_REGION(0x10000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD("prom2.cpu", 0x1000, 0x0800, CRC(6a42eaf4) SHA1(3e28b01473266db463986a4283e1be85f2410fb1))
	ROM_RELOAD(0x5000, 0x0800)
	ROM_RELOAD(0x9000, 0x0800)
	ROM_RELOAD(0xd000, 0x0800)
	ROM_LOAD("prom1.cpu", 0x2000, 0x2000, CRC(e724db90) SHA1(10e760e129ce89f11372c6dd3616216d45f2c926))
	ROM_RELOAD(0x6000, 0x2000)
	ROM_RELOAD(0xa000, 0x2000)
	ROM_RELOAD(0xe000, 0x2000)

	ROM_REGION(0x10000, "p4sound:audiocpu", ROMREGION_ERASEFF)
	ROM_LOAD("drom1.snd", 0x8000, 0x8000, CRC(4ab6dab7) SHA1(7e21e69029e60052112ddd5c7481582ea6684dc1))

	ROM_REGION(0x10000, "p4sound:speechcpu", ROMREGION_ERASEFF)
	ROM_LOAD("yrom1.snd", 0x8000, 0x8000, CRC(921a100e) SHA1(0c3c7eae4ceeb5a1a8150bac52203d3f1e8f917e))
ROM_END

ROM_START(victorypf)
	ROM_REGION(0x10000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD("prom2f.cpu", 0x1000, 0x0800, CRC(dffcfa77) SHA1(3efaca85295ca55268b8d7c7cfe8f09f159d5fbd))
	ROM_RELOAD(0x5000, 0x0800)
	ROM_RELOAD(0x9000, 0x0800)
	ROM_RELOAD(0xd000, 0x0800)
	ROM_LOAD("prom1f.cpu", 0x2000, 0x2000, CRC(d3a9df20) SHA1(7e0a97a4c1b488af89959cbaa693e23302479d0a))
	ROM_RELOAD(0x6000, 0x2000)
	ROM_RELOAD(0xa000, 0x2000)
	ROM_RELOAD(0xe000, 0x2000)

	ROM_REGION(0x10000, "p4sound:audiocpu", ROMREGION_ERASEFF)
	ROM_LOAD("drom1.snd", 0x8000, 0x8000, CRC(4ab6dab7) SHA1(7e21e69029e60052112ddd5c7481582ea6684dc1))

	ROM_REGION(0x10000, "p4sound:speechcpu", ROMREGION_ERASEFF)
	ROM_LOAD("yrom1.snd", 0x8000, 0x8000, CRC(921a100e) SHA1(0c3c7eae4ceeb5a1a8150bac52203d3f1e8f917e))
ROM_END

ROM_START(victorypg)
	ROM_REGION(0x10000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD("prom2g.cpu", 0x1000, 0x0800, CRC(b191a87a) SHA1(f205ffb41c5ba34e3cefc96ca870a5d08bee8854))
	ROM_RELOAD(0x5000, 0x0800)
	ROM_RELOAD(0x9000, 0x0800)
	ROM_RELOAD(0xd000, 0x0800)
	ROM_LOAD("prom1g.cpu", 0x2000, 0x2000, CRC(097b9062) SHA1(e7f05084b36f84b9948702ba297700473386ae6d))
	ROM_RELOAD(0x6000, 0x2000)
	ROM_RELOAD(0xa000, 0x2000)
	ROM_RELOAD(0xe000, 0x2000)

	ROM_REGION(0x10000, "p4sound:audiocpu", ROMREGION_ERASEFF)
	ROM_LOAD("drom1.snd", 0x8000, 0x8000, CRC(4ab6dab7) SHA1(7e21e69029e60052112ddd5c7481582ea6684dc1))

	ROM_REGION(0x10000, "p4sound:speechcpu", ROMREGION_ERASEFF)
	ROM_LOAD("yrom1.snd", 0x8000, 0x8000, CRC(921a100e) SHA1(0c3c7eae4ceeb5a1a8150bac52203d3f1e8f917e))
ROM_END

/*-------------------------------------------------------------------
/ System 80B Test Fixture
/-------------------------------------------------------------------*/
ROM_START(s80btest)
	ROM_REGION(0x10000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD("test2.cpu", 0x1000, 0x0800, CRC(6199c002) SHA1(d997e7a2f10b1780532aea689ee00e0c60e1cc64))
	ROM_RELOAD(0x5000, 0x0800)
	ROM_RELOAD(0x9000, 0x0800)
	ROM_RELOAD(0xd000, 0x0800)
	ROM_LOAD("test1.cpu", 0x2000, 0x2000, CRC(032ccbff) SHA1(e6703bd061d7c8c7e8917371d253647cf1320356))
	ROM_RELOAD(0x6000, 0x2000)
	ROM_RELOAD(0xa000, 0x2000)
	ROM_RELOAD(0xe000, 0x2000)

	ROM_REGION(0x10000, "p4sound:audiocpu", ROMREGION_ERASEFF)
	ROM_LOAD("testd.snd", 0xe000, 0x2000, CRC(5d04a6d9) SHA1(f83bd8692146af7d234c1a32d0b688e76d1b2b85))

	ROM_REGION(0x10000, "p4sound:speechcpu", ROMREGION_ERASEFF)
	ROM_LOAD("testy.snd", 0xe000, 0x2000, CRC(bd998860) SHA1(8a23376cc646c9854af204e32034bf40ebe23656) )
ROM_END

/*-------------------------------------------------------------------
/ Master (ManilaMatic)
/
/ Notes from one of the PinMAME devs:
/ It's a Gottlieb System 80B clone of "Genesis" more or less;
/ they only swapped in Italian texts and maybe changed some game rules.
/ The main CPU board is using a 6502 CPU with all 16 address lines
/ (System 80B only used 14), 2K of static RAM, and a 27256 EPROM.
/
/ Obviously they forgot to adjust the ROM checksums of the game
/ because it reports an error when running the memory test.
/ The game works just fine however, and when comparing the game code
/ to the Genesis one, it's identical for the most part.
/
/ TODO: implement different memory map
/-------------------------------------------------------------------*/

ROM_START(mmmaster)
	ROM_REGION(0x10000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD("gprom.cpu", 0x0000, 0x8000, CRC(0ffacb1d) SHA1(c609f49e0933ceb3d7eb1725a3ba0f1486978bd6))

	ROM_REGION(0x10000, "r2sound:audiocpu", ROMREGION_ERASEFF)
	ROM_LOAD("drom1.snd",0xc000,0x2000, CRC(758e1743) SHA1(6df3011c044796afcd88e52d1ca69692cb489ff4))

	ROM_REGION(0x10000, "r2sound:speechcpu", ROMREGION_ERASEFF)
	ROM_LOAD("yrom1.snd",0xe000,0x2000, CRC(4869b0ec) SHA1(b8a56753257205af56e06105515b8a700bb1935b))
	ROM_LOAD("yrom2.snd",0xc000,0x2000, CRC(0528c024) SHA1(d24ff7e088b08c1f35b54be3c806f8a8757d96c7))
ROM_END

/*-------------------------------------------------------------------
/ Top Sound (ManilaMatic)
/-------------------------------------------------------------------*/

ROM_START(topsound)
	ROM_REGION(0x10000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD("mm_ts_1.cpu", 0x6000, 0x2000, CRC(8ade048f) SHA1(f8527d99461b61a865023e0576ac5a9d33e4f0b0))
	ROM_LOAD("mm_ts_2.cpu", 0x2000, 0x2000, CRC(a525aac8) SHA1(9389688e053beb7db45278524c4d62cf067f817d))

	ROM_REGION(0x10000, "r2sound:audiocpu", ROMREGION_ERASEFF)
	ROM_LOAD("drom1a.snd",0xc000,0x2000, CRC(b8aa8912) SHA1(abff690256c0030807b2d4dfa0516496516384e8))

	ROM_REGION(0x10000, "r2sound:speechcpu", ROMREGION_ERASEFF)
	ROM_LOAD("yrom1a.snd",0xe000,0x2000, CRC(a62e3b94) SHA1(59636c2ac7ebbd116a0eb39479c97299ba391906))
	ROM_LOAD("yrom2a.snd",0xc000,0x2000, CRC(66645a3f) SHA1(f06261af81e6b1829d639933297d2461a8c993fc))
ROM_END

} // Anonymous namespace


GAME(1985, bountyh,   0,        p3, gts80b, gts80b_state, empty_init, ROT0, "Gottlieb",               "Bounty Hunter",                             MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME(1985, bountyhg,  bountyh,  p3, gts80b, gts80b_state, empty_init, ROT0, "Gottlieb",               "Bounty Hunter (German)",                    MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME(1985, triplay,   0,        p3, gts80b, gts80b_state, empty_init, ROT0, "Gottlieb",               "Chicago Cubs' Triple Play",                 MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME(1985, triplaya,  triplay,  p3, gts80b, gts80b_state, empty_init, ROT0, "Gottlieb",               "Chicago Cubs' Triple Play (alternate set)", MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME(1985, triplayg,  triplay,  p3, gts80b, gts80b_state, empty_init, ROT0, "Gottlieb",               "Chicago Cubs' Triple Play (German)",        MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME(1985, rock,      0,        r2, gts80b, gts80b_state, empty_init, ROT0, "Gottlieb",               "Rock",                                      MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME(1985, rockg,     rock,     r2, gts80b, gts80b_state, empty_init, ROT0, "Gottlieb",               "Rock (German)",                             MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME(1985, tagteamp,  0,        p3, gts80b, gts80b_state, empty_init, ROT0, "Gottlieb",               "Tag-Team Wrestling",                        MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME(1985, tagteampg, tagteamp, p3, gts80b, gts80b_state, empty_init, ROT0, "Gottlieb",               "Tag-Team Wrestling (German)",               MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME(1985, tagteamp2, tagteamp, p3, gts80b, gts80b_state, empty_init, ROT0, "Gottlieb",               "Tag-Team Wrestling (rev.2)",                MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME(1986, raven,     0,        r2, gts80b, gts80b_state, empty_init, ROT0, "Gottlieb",               "Raven",                                     MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME(1986, ravena,    raven,    r2, gts80b, gts80b_state, empty_init, ROT0, "Gottlieb",               "Raven (alternate set)",                     MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME(1986, raveng,    raven,    r2, gts80b, gts80b_state, empty_init, ROT0, "Gottlieb",               "Raven (German)",                            MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME(1986, hlywoodh,  0,        r2, gts80b, gts80b_state, empty_init, ROT0, "Gottlieb",               "Hollywood Heat",                            MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME(1986, hlywoodhf, hlywoodh, r2, gts80b, gts80b_state, empty_init, ROT0, "Gottlieb",               "Hollywood Heat (French)",                   MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME(1986, hlywoodhg, hlywoodh, r2, gts80b, gts80b_state, empty_init, ROT0, "Gottlieb",               "Hollywood Heat (German)",                   MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME(1986, rock_enc,  rock,     r2, gts80b, gts80b_state, empty_init, ROT0, "Gottlieb",               "Rock Encore",                               MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME(1986, rock_encg, rock,     r2, gts80b, gts80b_state, empty_init, ROT0, "Gottlieb",               "Rock Encore (German)",                      MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME(1986, genesisp,  0,        r2, gts80b, gts80b_state, empty_init, ROT0, "Gottlieb",               "Genesis",                                   MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME(1986, genesispf, genesisp, r2, gts80b, gts80b_state, empty_init, ROT0, "Gottlieb",               "Genesis (French)",                          MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME(1986, genesispg, genesisp, r2, gts80b, gts80b_state, empty_init, ROT0, "Gottlieb",               "Genesis (German)",                          MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME(1987, amazonh2,  0,        r2, gts80b, gts80b_state, empty_init, ROT0, "Gottlieb",               "Amazon Hunt II (French)",                   MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME(1987, sprbreak,  0,        r2, gts80b, gts80b_state, empty_init, ROT0, "Gottlieb",               "Spring Break",                              MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME(1987, sprbreaka, sprbreak, r2, gts80b, gts80b_state, empty_init, ROT0, "Gottlieb",               "Spring Break (alternate set)",              MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME(1987, sprbreakf, sprbreak, r2, gts80b, gts80b_state, empty_init, ROT0, "Gottlieb",               "Spring Break (French)",                     MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME(1987, sprbreakg, sprbreak, r2, gts80b, gts80b_state, empty_init, ROT0, "Gottlieb",               "Spring Break (German)",                     MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME(1987, sprbreaks, sprbreak, r2, gts80b, gts80b_state, empty_init, ROT0, "Gottlieb",               "Spring Break (single ball game)",           MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME(1986, goldwing,  0,        r2, gts80b, gts80b_state, empty_init, ROT0, "Gottlieb",               "Gold Wings",                                MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME(1986, goldwingf, goldwing, r2, gts80b, gts80b_state, empty_init, ROT0, "Gottlieb",               "Gold Wings (French)",                       MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME(1986, goldwingg, goldwing, r2, gts80b, gts80b_state, empty_init, ROT0, "Gottlieb",               "Gold Wings (German)",                       MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME(1987, mntecrlo,  0,        r2, gts80b, gts80b_state, empty_init, ROT0, "Gottlieb",               "Monte Carlo (Pinball)",                     MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME(1987, mntecrloa, mntecrlo, r2, gts80b, gts80b_state, empty_init, ROT0, "Gottlieb",               "Monte Carlo (Pinball, alternate set)",      MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME(1987, mntecrlof, mntecrlo, r2, gts80b, gts80b_state, empty_init, ROT0, "Gottlieb",               "Monte Carlo (Pinball, French)",             MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME(1987, mntecrlog, mntecrlo, r2, gts80b, gts80b_state, empty_init, ROT0, "Gottlieb",               "Monte Carlo (Pinball, German)",             MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME(1987, mntecrlo2, mntecrlo, r2, gts80b, gts80b_state, empty_init, ROT0, "Gottlieb",               "Monte Carlo (Pinball, rev. 2)",             MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME(1987, arena,     0,        r2, gts80b, gts80b_state, empty_init, ROT0, "Gottlieb",               "Arena",                                     MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME(1987, arenaa,    arena,    r2, gts80b, gts80b_state, empty_init, ROT0, "Gottlieb",               "Arena (alternate set)",                     MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME(1987, arenaf,    arena,    r2, gts80b, gts80b_state, empty_init, ROT0, "Gottlieb",               "Arena (French)",                            MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME(1987, arenag,    arena,    r2, gts80b, gts80b_state, empty_init, ROT0, "Gottlieb",               "Arena (German)",                            MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME(1987, victoryp,  0,        p4, gts80b, gts80b_state, empty_init, ROT0, "Gottlieb",               "Victory (Pinball)",                         MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME(1987, victorypf, victoryp, p4, gts80b, gts80b_state, empty_init, ROT0, "Gottlieb",               "Victory (Pinball, French)",                 MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME(1987, victorypg, victoryp, p4, gts80b, gts80b_state, empty_init, ROT0, "Gottlieb",               "Victory (Pinball, German)",                 MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME(1988, diamondp,  0,        p4, gts80b, gts80b_state, empty_init, ROT0, "Gottlieb",               "Diamond Lady",                              MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME(1988, diamondpf, diamondp, p4, gts80b, gts80b_state, empty_init, ROT0, "Gottlieb",               "Diamond Lady (French)",                     MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME(1988, diamondpg, diamondp, p4, gts80b, gts80b_state, empty_init, ROT0, "Gottlieb",               "Diamond Lady (German)",                     MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME(1988, txsector,  0,        p4, gts80b, gts80b_state, empty_init, ROT0, "Gottlieb",               "TX-Sector",                                 MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME(1988, txsectorf, txsector, p4, gts80b, gts80b_state, empty_init, ROT0, "Gottlieb",               "TX-Sector (French)",                        MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME(1988, txsectorg, txsector, p4, gts80b, gts80b_state, empty_init, ROT0, "Gottlieb",               "TX-Sector (German)",                        MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME(1989, bighouse,  0,        p5, gts80b, gts80b_state, init_s80c,  ROT0, "Gottlieb",               "Big House",                                 MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME(1989, bighousef, bighouse, p5, gts80b, gts80b_state, init_s80c,  ROT0, "Gottlieb",               "Big House (French)",                        MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME(1989, bighouseg, bighouse, p5, gts80b, gts80b_state, init_s80c,  ROT0, "Gottlieb",               "Big House (German)",                        MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME(1988, robowars,  0,        p4, gts80b, gts80b_state, empty_init, ROT0, "Gottlieb",               "Robo-War",                                  MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME(1988, robowarsf, robowars, p4, gts80b, gts80b_state, empty_init, ROT0, "Gottlieb",               "Robo-War (French)",                         MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME(1988, excalibr,  0,        p4, gts80b, gts80b_state, empty_init, ROT0, "Gottlieb",               "Excalibur",                                 MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME(1988, excalibrf, excalibr, p4, gts80b, gts80b_state, empty_init, ROT0, "Gottlieb",               "Excalibur (French)",                        MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME(1988, excalibrg, excalibr, p4, gts80b, gts80b_state, empty_init, ROT0, "Gottlieb",               "Excalibur (German)",                        MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME(1988, badgirls,  0,        p5, gts80b, gts80b_state, init_s80c,  ROT0, "Gottlieb",               "Bad Girls",                                 MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME(1988, badgirlsa, badgirls, p5, gts80b, gts80b_state, init_s80c,  ROT0, "Gottlieb",               "Bad Girls (alternate set)",                 MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME(1988, badgirlsf, badgirls, p5, gts80b, gts80b_state, init_s80c,  ROT0, "Gottlieb",               "Bad Girls (French)",                        MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME(1988, badgirlsg, badgirls, p5, gts80b, gts80b_state, init_s80c,  ROT0, "Gottlieb",               "Bad Girls (German)",                        MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME(1989, hotshots,  0,        p5, gts80b, gts80b_state, init_s80c,  ROT0, "Gottlieb",               "Hot Shots",                                 MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME(1989, hotshotsf, hotshots, p5, gts80b, gts80b_state, init_s80c,  ROT0, "Gottlieb",               "Hot Shots (French)",                        MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME(1989, hotshotsg, hotshots, p5, gts80b, gts80b_state, init_s80c,  ROT0, "Gottlieb",               "Hot Shots (German)",                        MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME(1989, bonebstr,  0,        p6, gts80b, gts80b_state, init_s80c,  ROT0, "Gottlieb",               "Bone Busters Inc.",                         MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME(1989, bonebstrf, bonebstr, p6, gts80b, gts80b_state, init_s80c,  ROT0, "Gottlieb",               "Bone Busters Inc. (French)",                MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME(1989, bonebstrg, bonebstr, p6, gts80b, gts80b_state, init_s80c,  ROT0, "Gottlieb",               "Bone Busters Inc. (German)",                MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME(1989, nmoves,    0,        p5, gts80b, gts80b_state, init_s80c,  ROT0, "International Concepts", "Night Moves",                               MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME(1987, amazonh3,  0,        p4, gts80b, gts80b_state, init_s80c,  ROT0, "Gottlieb",               "Amazon Hunt III (French)",                  MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME(1987, amazonh3a, amazonh3, p4, gts80b, gts80b_state, init_s80c,  ROT0, "Gottlieb",               "Amazon Hunt III (rev. 1, French)",          MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME(198?, s80btest,  0,        p0, gts80b, gts80b_state, empty_init, ROT0, "Gottlieb",               "System 80B Test",                           MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME(1988, mmmaster,  0,    master, gts80b, gts80b_state, empty_init, ROT0, "ManilaMatic",            "Master",                                    MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME(1988, topsound,  0,    master, gts80b, gts80b_state, empty_init, ROT0, "ManilaMatic",            "Top Sound (French)",                        MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
