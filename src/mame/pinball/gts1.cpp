// license:BSD-3-Clause
// copyright-holders:Robbbert
/****************************************************************************************************

PINBALL
Gottlieb System 1

Gottlieb's first foray into computerised pinball.

Typical of Gottlieb's golden period, these machines are physically well-designed and made.
However, the computer side was another story, an attempt to catch up to its competitors who
were way ahead in the technology race. Instead of each board being solidly grounded to the
chassis, the only connections were through flaky edge connectors. Voltage differences would
then cause solenoids and lights to switch on at random and destroy transistors. Further, the
CPU chips chosen were an unusual 4-bit design that was already old.

The first games had chimes. Then, this was replaced by 3 NE555 tone oscillators. The last
machines had a real sound board which had more computing power than the main cpu.

Game numbering:
Each Gottlieb game had the model number printed on the instruction card, so it was very
easy to gather information. Gottlieb either made a single-player game, or a 2-player and
a 4-player game. For example, Centigrade 37 (#407) was a single-player game, while Bronco
(4-player)(#396) was exactly the same as Mustang (2-player)(#397). Cleopatra (#409) was
originally a 4-player EM game (with Pyramid #410 being the 2-player version). Then, the SS
version was made, and it kept the same number. After that, the SS versions were suffixed
with 'SS' up to The Incredible Hulk (#433), and then the 'SS' was dropped.
Of the solid-state games, Asteroid Annie is single-player - the others are 4-player.


Game List: (the ROM letter is stamped onto the personality prom)

Game                                  NUM    ROM
------------------------------------------------
Cleopatra                             409     A
Sinbad                                412SS   B
Joker Poker                           417SS   C
Dragon                                419SS   D
Solar Ride                            421SS   E
Countdown                             422SS   F
Close Encounters of the third kind    424SS   G
Charlie's Angels                      425SS   H
Pinball Pool                          427SS   I
Totem                                 429SS   J
The Incredible Hulk                   433SS   K
Genie                                 435     L
Buck Rogers                           437     N
Torch                                 438     P
Roller Disco                          440     R
Asteroid Annie and the Aliens         442     S
Test Prom                                     T
**** Other Manufacturer ****
Hell's Queen
Sky Warrior
Tiger Woman
L'Hexagone
Sahara Love
Movie
Jungle Queen


The personality prom is programmed in PGOL (pinball-game oriented language).

Chips used:
U1   11660     Rockwell Parallel Processing System 4-bit CPU (PPS/4-2)
U2   10696EE   General Purpose I/O expander: 5101L RAM interface (device#6)
U3   10696EE   General purpose I/O expander: dipswitches, lamps, misc (device#3)
U4   A1753CX   Early RIOT-type device: Custom 2kx8 ROM, 128x4 RAM, 16x1 I/O (solenoid control)
U5   A1752CX   Early RIOT-type device: Custom 2kx8 ROM, 128x4 RAM, 16x1 I/O (switch matrix)
U6   10788     Display driver
Z22  5101L     4-bit static RAM
Z23  MM6351-IJ Personality PROM


Status:
- All games are playable

ToDo:
- Could be some more CPU bugs, if so hunt them down.
- The sound-board sounds are very weird.
- Z80-based sound board for hexagone and sahalove (no info available).

*****************************************************************************************************/


#include "emu.h"
#include "genpin.h"

#include "gottlieb_a.h"

#include "machine/ra17xx.h"
#include "machine/r10696.h"
#include "machine/r10788.h"
#include "cpu/pps4/pps4.h"
#include "sound/beep.h"

#include "speaker.h"

#include "gts1.lh"

#define VERBOSE    1
#include "logmacro.h"

namespace {

class gts1_state : public genpin_class
{
public:
	gts1_state(const machine_config &mconfig, device_type type, const char *tag)
		: genpin_class(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_pm(*this, "module")   // personality module
		, m_nvram(*this, "nvram")
		, m_dips(*this, "DSW%u", 0U)
		, m_switches(*this, "X%u", 0U)
		, m_p1_sound(*this, "beeper")
		, m_p2_sound(*this, "p2sound")
		, m_digit8(*this, "digit8_%u", 0U)
		, m_digit7(*this, "digit7_%u", 0U)
		, m_io_outputs(*this, "out%d", 0U)
	{ }

	void p0(machine_config &config); // chimes
	void p1(machine_config &config); // ne555 tones
	void p2(machine_config &config); // multi-mode sound card

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	u8 gts1_solenoid_r(offs_t offset);
	void gts1_solenoid_w(offs_t offset, u8 data);
	u8 gts1_switches_r(offs_t offset);
	void gts1_switches_w(offs_t offset, u8 data);
	void gts1_display_w(offs_t offset, u8 data);
	u16 seg8to14(u16 data);
	u8 gts1_lamp_apm_r(offs_t offset);
	void gts1_lamp_apm_w(offs_t offset, u8 data);
	u8 gts1_nvram_r(offs_t offset);
	void gts1_nvram_w(offs_t offset, u8 data);
	u8 gts1_pa_r();
	void gts1_do_w(u8 data);
	void nvram_w();

	void gts1_map(address_map &map) ATTR_COLD;
	void gts1_data(address_map &map) ATTR_COLD;
	void gts1_io(address_map &map) ATTR_COLD;

	required_device<pps4_2_device> m_maincpu;
	required_region_ptr<u8> m_pm;
	required_shared_ptr<u8> m_nvram;
	required_ioport_array<3> m_dips;
	required_ioport_array<7> m_switches;
	optional_device<beep_device> m_p1_sound;
	optional_device<gottlieb_sound_p2_device> m_p2_sound;
	output_finder<32> m_digit8; // digits 0-5,8-13,16-21,24-29
	output_finder<32> m_digit7; // digits 6,7,14,15 on repurposed digital clock display
	output_finder<44> m_io_outputs; // 8 solenoids + 36 lamps

	u8 m_strobe = 0U;             //!< switches strobe lines (5 lower bits used)
	u8 m_nvram_addr = 0xffU;      //!< NVRAM address
	u8 m_nvram_data = 0U;
	bool m_nvram_e2 = false;      //!< NVRWAM enable (E2 line)
	bool m_nvram_wr = false;      //!< NVRWAM write (W/R line)
	u16 m_6351_addr = 0U;         //!< ROM MM6351 address (12 bits)
	u8 m_z30_out = 0U;            //!< 4-to-16 decoder outputs
	u8 m_lamp_data = 0U;
};

void gts1_state::gts1_map(address_map &map)
{
	map(0x0000, 0x0fff).rom();  // ROM inside A1752XX and A1753XX chips
}

void gts1_state::gts1_data(address_map &map)
{
	map(0x0000, 0x00ff).ram();  // RAM inside A1752XX and A1753XX chips
	map(0x0100, 0x01ff).ram().share("nvram");   // Battery-backed M5101L
}

void gts1_state::gts1_io(address_map &map)
{
	map(0x20, 0x2f).rw("u5", FUNC(ra17xx_device::io_r), FUNC(ra17xx_device::io_w)); // (U5) switch matrix
	map(0x30, 0x3f).rw("u3", FUNC(r10696_device::io_r), FUNC(r10696_device::io_w)); // (U3) lamps, display, dips
	map(0x40, 0x4f).rw("u4", FUNC(ra17xx_device::io_r), FUNC(ra17xx_device::io_w)); // (U4) solenoids, nvram control
	map(0x60, 0x6f).rw("u2", FUNC(r10696_device::io_r), FUNC(r10696_device::io_w)); // (U2) NVRAM io chip
	map(0xd0, 0xdf).rw("u6", FUNC(r10788_device::io_r), FUNC(r10788_device::io_w)); // (U6) display chip
}

static INPUT_PORTS_START( gts1_dips )
	PORT_START("DSW0")
	PORT_DIPNAME( 0x0f, 0x0f, "Coinage Left Slot") PORT_DIPLOCATION("SWM1:1,2,3,4")
	PORT_DIPSETTING(    0x0f, DEF_STR(1C_1C))
	PORT_DIPSETTING(    0x0e, DEF_STR(1C_2C))
	PORT_DIPSETTING(    0x0d, DEF_STR(1C_3C))
	PORT_DIPSETTING(    0x0c, DEF_STR(1C_4C))
	PORT_DIPSETTING(    0x0b, DEF_STR(1C_5C))
	PORT_DIPSETTING(    0x0a, DEF_STR(1C_6C))
	PORT_DIPSETTING(    0x09, DEF_STR(1C_7C))
	PORT_DIPSETTING(    0x08, DEF_STR(1C_8C))
	PORT_DIPSETTING(    0x07, DEF_STR(1C_9C))
	PORT_DIPSETTING(    0x06, DEF_STR(2C_1C))
	PORT_DIPSETTING(    0x05, DEF_STR(2C_2C))
	PORT_DIPSETTING(    0x04, DEF_STR(2C_3C))
	PORT_DIPSETTING(    0x03, DEF_STR(2C_4C))
	PORT_DIPSETTING(    0x02, DEF_STR(2C_5C))
	PORT_DIPSETTING(    0x01, "1 coin 1 credit, 2 coins 3 credits")
	PORT_DIPSETTING(    0x00, DEF_STR(3C_1C))
	PORT_DIPNAME( 0xf0, 0xf0, "Coinage Right Slot") PORT_DIPLOCATION("SWM1:5,6,7,8")
	PORT_DIPSETTING(    0xf0, DEF_STR(1C_1C))
	PORT_DIPSETTING(    0xe0, DEF_STR(1C_2C))
	PORT_DIPSETTING(    0xd0, DEF_STR(1C_3C))
	PORT_DIPSETTING(    0xc0, DEF_STR(1C_4C))
	PORT_DIPSETTING(    0xb0, DEF_STR(1C_5C))
	PORT_DIPSETTING(    0xa0, DEF_STR(1C_6C))
	PORT_DIPSETTING(    0x90, DEF_STR(1C_7C))
	PORT_DIPSETTING(    0x80, DEF_STR(1C_8C))
	PORT_DIPSETTING(    0x70, DEF_STR(1C_9C))
	PORT_DIPSETTING(    0x60, DEF_STR(2C_1C))
	PORT_DIPSETTING(    0x50, DEF_STR(2C_2C))
	PORT_DIPSETTING(    0x40, DEF_STR(2C_3C))
	PORT_DIPSETTING(    0x30, DEF_STR(2C_4C))
	PORT_DIPSETTING(    0x20, DEF_STR(2C_5C))
	PORT_DIPSETTING(    0x10, "1 coin 1 credit, 2 coins 3 credits")
	PORT_DIPSETTING(    0x00, DEF_STR(3C_1C))

	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x00, "Balls") PORT_DIPLOCATION("SWM2:1")
	PORT_DIPSETTING(    0x00, "3")
	PORT_DIPSETTING(    0x01, "5")
	PORT_DIPNAME( 0x02, 0x00, "Match") PORT_DIPLOCATION("SWM2:2")
	PORT_DIPSETTING(    0x00, DEF_STR( On ))
	PORT_DIPSETTING(    0x02, DEF_STR( Off ))
	PORT_DIPNAME( 0x04, 0x00, "Preset score award") PORT_DIPLOCATION("SWM2:3")
	PORT_DIPSETTING(    0x00, "Replay")
	PORT_DIPSETTING(    0x04, "Extra Ball")
	PORT_DIPNAME( 0x08, 0x00, "Tilt Penalty") PORT_DIPLOCATION("SWM2:4")
	PORT_DIPSETTING(    0x00, "Ball")
	PORT_DIPSETTING(    0x08, "Game")
	PORT_DIPNAME( 0x10, 0x00, "Credits display") PORT_DIPLOCATION("SWM2:5")
	PORT_DIPSETTING(    0x00, DEF_STR( On ))
	PORT_DIPSETTING(    0x10, DEF_STR( Off ))
	PORT_DIPNAME( 0x20, 0x00, "Game start sound") PORT_DIPLOCATION("SWM2:6")
	PORT_DIPSETTING(    0x00, DEF_STR( On ))
	PORT_DIPSETTING(    0x20, DEF_STR( Off ))
	PORT_DIPNAME( 0x40, 0x40, "S15") PORT_DIPLOCATION("SWM2:7")
	PORT_DIPSETTING(    0x00, DEF_STR( On ))
	PORT_DIPSETTING(    0x40, DEF_STR( Off ))
	PORT_DIPNAME( 0x80, 0x80, "S16") PORT_DIPLOCATION("SWM2:8")
	PORT_DIPSETTING(    0x00, DEF_STR( On ))
	PORT_DIPSETTING(    0x80, DEF_STR( Off ))

	PORT_START("DSW2")
	PORT_DIPNAME( 0x03, 0x00, "Max credits") PORT_DIPLOCATION("SWM3:1,2")
	PORT_DIPSETTING(    0x03, "5")
	PORT_DIPSETTING(    0x02, "8")
	PORT_DIPSETTING(    0x01, "10")
	PORT_DIPSETTING(    0x00, "15")
	PORT_DIPNAME( 0x04, 0x04, "S1-4 control both slots") PORT_DIPLOCATION("SWM3:3")
	PORT_DIPSETTING(    0x00, DEF_STR( On ))
	PORT_DIPSETTING(    0x04, DEF_STR( Off ))
	PORT_DIPNAME( 0x08, 0x00, "Ingame sound") PORT_DIPLOCATION("SWM3:4")
	PORT_DIPSETTING(    0x00, DEF_STR( On ))
	PORT_DIPSETTING(    0x08, DEF_STR( Off ))
	PORT_DIPNAME( 0x10, 0x00, "High game display") PORT_DIPLOCATION("SWM3:5")
	PORT_DIPSETTING(    0x00, DEF_STR( On ))
	PORT_DIPSETTING(    0x10, DEF_STR( Off ))
	PORT_DIPNAME( 0x20, 0x00, "High game awards 3 credits") PORT_DIPLOCATION("SWM3:6")
	PORT_DIPSETTING(    0x00, DEF_STR( On ))
	PORT_DIPSETTING(    0x20, DEF_STR( Off ))
	PORT_DIPNAME( 0x40, 0x00, "Coin sound") PORT_DIPLOCATION("SWM3:7")
	PORT_DIPSETTING(    0x00, DEF_STR( On ))
	PORT_DIPSETTING(    0x40, DEF_STR( Off ))
	PORT_DIPNAME( 0x80, 0x80, "S24") PORT_DIPLOCATION("SWM3:8")
	PORT_DIPSETTING(    0x00, DEF_STR( On ))
	PORT_DIPSETTING(    0x80, DEF_STR( Off ))
INPUT_PORTS_END

static INPUT_PORTS_START( gts1_switches )
	PORT_START("X0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_0_PAD) PORT_NAME("Play/Test")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_A) PORT_NAME("INP10")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_B) PORT_NAME("INP20")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_C) PORT_NAME("INP30")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_D) PORT_NAME("INP40")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_E) PORT_NAME("INP50")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_F) PORT_NAME("INP60")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_G) PORT_NAME("INP70")

	PORT_START("X1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_H) PORT_NAME("INP11")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_I) PORT_NAME("INP21")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_J) PORT_NAME("INP31")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_K) PORT_NAME("INP41")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_L) PORT_NAME("INP51")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_M) PORT_NAME("INP61")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_N) PORT_NAME("INP71")

	PORT_START("X2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_O) PORT_NAME("INP12")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_P) PORT_NAME("INP22")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_Q) PORT_NAME("INP32")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_R) PORT_NAME("INP42")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_S) PORT_NAME("INP52")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_T) PORT_NAME("INP62")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_U) PORT_NAME("INP72")

	PORT_START("X3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_V) PORT_NAME("INP13")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_W) PORT_NAME("INP23")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_Y) PORT_NAME("INP33")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_Z) PORT_NAME("INP43")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_MINUS) PORT_NAME("INP53")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_EQUALS) PORT_NAME("INP63")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_BACKSPACE) PORT_NAME("INP73")

	PORT_START("X4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_9) PORT_NAME("Tilt")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_OPENBRACE) PORT_NAME("INP14")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_CLOSEBRACE) PORT_NAME("INP24")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_BACKSLASH) PORT_NAME("INP34")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_COMMA) PORT_NAME("INP44")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_QUOTE) PORT_NAME("INP54")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_STOP) PORT_NAME("INP64")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_SLASH) PORT_NAME("INP74")

	PORT_START("X5")
	PORT_BIT( 0xFF, IP_ACTIVE_LOW, IPT_UNUSED ) // not connected but it is tested

	PORT_START("X6")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_5_PAD) PORT_NAME("Reset")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_X) PORT_NAME("Outhole")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_0) PORT_NAME("Slam Tilt")
INPUT_PORTS_END

static INPUT_PORTS_START( gts1 )
	PORT_INCLUDE( gts1_dips )

	PORT_INCLUDE( gts1_switches )
INPUT_PORTS_END

static INPUT_PORTS_START( jokrpokr )
	PORT_INCLUDE( gts1 )

	PORT_MODIFY("X0")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_A) PORT_NAME("POP/BUMBER")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_B) PORT_NAME("EXTRA BALL TARGET")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_C) PORT_NAME("SPECIAL ROLLOVER")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_D) PORT_NAME("10 POINT CONTACTS")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_E) PORT_NAME("\"A\" DROP TARGET (red)")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_MODIFY("X1")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_H) PORT_NAME("\"A\" ROLLOVER")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_I) PORT_NAME("\"10\" DROP TARGET")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_J) PORT_NAME("\"Q\" DROP TARGET (red)")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_K) PORT_NAME("\"K\" DROP TARGET (black)")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_L) PORT_NAME("\"A\" DROP TARGET (black)")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_MODIFY("X2")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_O) PORT_NAME("\"B\" ROLLOVER")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_P) PORT_NAME("\"J\" DROP TARGET (black)")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_Q) PORT_NAME("\"O\" DROP TARGET (black)")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_R) PORT_NAME("\"K\" DROP TARGET")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_S) PORT_NAME("JOKER DROP TARGET")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_MODIFY("X3")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_V) PORT_NAME("\"C\" ROLLOVER")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_W) PORT_NAME("\"J\" DROP TARGET (red)")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_Y) PORT_NAME("\"O\" DROP TARGET (red)")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_Z) PORT_NAME("\"K\" DROP TARGET")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_MINUS) PORT_NAME("\"A\" DROP TARGET (red)")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_MODIFY("X4")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_COMMA) PORT_NAME("\"K\" DROP TARGET")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_QUOTE) PORT_NAME("\"A\" DROP TARGET (red)")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNUSED)
INPUT_PORTS_END

void gts1_state::machine_start()
{
	m_digit8.resolve();
	m_digit7.resolve();
	m_io_outputs.resolve();

	save_item(NAME(m_strobe));
	save_item(NAME(m_nvram_addr));
	save_item(NAME(m_nvram_data));
	save_item(NAME(m_nvram_e2));
	save_item(NAME(m_nvram_wr));
	save_item(NAME(m_6351_addr));
	save_item(NAME(m_z30_out));
	save_item(NAME(m_lamp_data));
}

void gts1_state::machine_reset()
{
	m_strobe = 0;
	m_nvram_addr = 0xff;
	m_nvram_data = 0;
	m_nvram_e2 = false;
	m_nvram_wr = false;
	m_6351_addr = 0x3ff;
	m_z30_out = 0U;
	m_lamp_data = 0U;
}

u8 gts1_state::gts1_solenoid_r(offs_t offset) // does nothing
{
	u8 data = 0;
	//LOG("%s: solenoid[%02x] -> %x\n", __FUNCTION__, offset, data);
	return data;
}

void gts1_state::gts1_solenoid_w(offs_t offset, u8 data)
{
	//LOG("%s: solenoid #[%02X] gets data=%X\n", __FUNCTION__, offset, data);
	switch (offset)
	{
	case  0:  // outhole
		if (data)
			m_samples->start(5, 5);
		break;
	case  1:  // knocker
		if (data)
			m_samples->start(0, 6);
		break;
	case  2:  // tens chime
		if (m_p1_sound)
		{
			m_p1_sound->set_clock(593);
			m_p1_sound->set_state(data);
		}
		else
		if (m_p2_sound)
			m_p2_sound->write(data ? 11 : 15);
		else
		if (data)
			m_samples->start(3, 3);
		break;
	case  3:  // hundreds chime
		if (m_p1_sound)
		{
			m_p1_sound->set_clock(265);
			m_p1_sound->set_state(data);
		}
		else
		if (m_p2_sound)
			m_p2_sound->write(data ? 13 : 15);
		else
		if (data)
			m_samples->start(2, 2);
		break;
	case  4:  // thousands chime
		if (m_p1_sound)
		{
			m_p1_sound->set_clock(153);
			m_p1_sound->set_state(data);
		}
		else
		if (m_p2_sound)
			m_p2_sound->write(data ? 14 : 15);
		else
		if (data)
			m_samples->start(1, 1);
		break;
	case  5:  // optional per machine
		break;
	case  6:  // optional per machine
		break;
	case  7:  // optional per machine
		break;
	case  8: case  9: case 10: case 11:  // unused
		break;
	case 12:    // spare
		break;
	case 13:    // RAM control E2
		m_nvram_e2 = data ? true : false;
		nvram_w();
		break;
	case 14:    // RAM control W/R
		m_nvram_wr = data ? false : true;
		nvram_w();
		break;
	case 15:    // spare
		break;
	}

	if (offset < 8)
		m_io_outputs[offset] = data;
}

u8 gts1_state::gts1_switches_r(offs_t offset)
{
	u8 data = 0;
	if (offset > 7)
		for (u8 i = 0; i < 6; i++)
			if (BIT(m_strobe, i))
			{
				data |= BIT(m_switches[i]->read(), offset & 7);
				//LOG("%s: switches[bit %X of %X, using offset of %X] got %x\n", __FUNCTION__, i, m_strobe, offset&7, data);
			}
	return data ? 0 : 1;
}

void gts1_state::gts1_switches_w(offs_t offset, u8 data) // WORKS
{
	// outputs O-0 to O-4 are the 5 strobe lines
	if (offset < 6)
	{
		if (data)
			m_strobe |= (1<<offset);
		else
			m_strobe &= ~(1<<offset);
		//LOG("%s: strobe is now[%x], data was %x\n", __FUNCTION__, m_strobe, data);
	}
}

u16 gts1_state::seg8to14(u16 data)
{
	// convert custom 8seg digit to MAME 14seg digit
	return bitswap<10>(data,7,7,6,6,5,4,3,2,1,0);
}

/**
 * @brief write a 8seg display value
 * @param offset digit number 0 .. 15
 * @param data 4-bit value to display
 */
void gts1_state::gts1_display_w(offs_t offset, u8 data) // WORKS
{
	/*
	 * The 7448 is modified to be disabled through RI/RBO
	 * when the input is 0001, and in this case the extra
	 * output H is generated instead.
	 */
	enum : u8
	{
		_a = 1 << 0,
		_b = 1 << 1,
		_c = 1 << 2,
		_d = 1 << 3,
		_e = 1 << 4,
		_f = 1 << 5,
		_g = 1 << 6,
		_h = 1 << 7
	};
	static constexpr u8 ttl7448_mod[16] = {
	/* 0 */  _a | _b | _c | _d | _e | _f,
	/* 1 */  _h,
	/* 2 */  _a | _b | _d | _e | _g,
	/* 3 */  _a | _b | _c | _d | _g,
	/* 4 */  _b | _c | _f | _g,
	/* 5 */  _a | _c | _d | _f | _g,
	/* 6 */  _a | _c | _d | _e | _f | _g,
	/* 7 */  _a | _b | _c,
	/* 8 */  _a | _b | _c | _d | _e | _f | _g,
	/* 9 */  _a | _b | _c | _d | _f | _g,
	/* a */  _d | _e | _g,
	/* b */  _c | _d | _g,
	/* c */  _b | _f | _g,
	/* d */  _a | _d | _f | _g,
	/* e */  _d | _e | _f | _g,
	/* f */  0
	};
	data ^= 0xff;  // It was stored in the 10788 inverted
	u8 a = ttl7448_mod[BIT(data, 0, 4)];
	u8 b = ttl7448_mod[BIT(data, 4, 4)];
	// LOG("%s: offset:%d data:%02x a:%02x b:%02x\n", __FUNCTION__, offset, data, a, b);
	if ((offset % 8) < 6)
	{
		m_digit8[offset] = seg8to14(a);
		m_digit8[offset + 16] = seg8to14(b);
	}
	else
	{
		/*
		 * For the 4 7-seg displays the segment h is turned back into
		 * segments b and c to display the 7-seg "1". */
		if (a & _h)
			a = _b | _c;

		m_digit7[offset] = a;
	}
}

/**
 * @brief read input groups A, B, C of NVRAM io chip (U2)
 * @param offset 0 ... 2 = group
 * @return 4-bit value read from the group
 */
u8 gts1_state::gts1_nvram_r(offs_t offset) // WORKS
{
	u8 data = 0x0f;
	switch (offset)
	{
		case 0: // group A
			if (m_nvram_e2)
			{
				data = m_nvram[m_nvram_addr];
				//LOG("%s: NVRAM READ @[%02x] -> %x\n", __FUNCTION__, m_nvram_addr, data);
			}
			break;
		case 1: // group B
		case 2: // group C
			// Schematics says: SPARES
			break;
	}
	return ~data & 15;
}

/**
 * @brief write output groups A, B, C of NVRAM io chip (U2)
 * @param offset 0 ... 2 = group
 * @param data 4 bit value to write
 */
 /* NVRAM locations: Each stored parameter takes 6 nybbles. The address is XY, where X = the test number in the bookkeeping functions (0 to 9),
    and Y = 0 for units, 1 for tens, etc. Additionally, A0-A5 is for the high score to date (bookkeeping function 10), and B0-B5 is the stored credits. */
void gts1_state::gts1_nvram_w(offs_t offset, u8 data) // WORKS
{
	data = ~data & 15;  // make the nvram compatible with pinmame
	switch (offset)
	{
		case 0: // group A - address lines 3:0
			m_nvram_addr = (m_nvram_addr & 0xf0) | data;
			break;
		case 1: // group B - address lines 7:4
			m_nvram_addr = (m_nvram_addr & 15) | (data << 4);
			break;
		case 2: // group C - data bits 3:0 of NVRAM
			m_nvram_data = data;
			nvram_w();
			break;
	}
}

void gts1_state::nvram_w()
{
	if (m_nvram_wr && m_nvram_e2)
	{
		//LOG("%s: NVRAM WRITE @[%02x] <- %x\n", __FUNCTION__, m_nvram_addr, m_nvram_data);
		m_nvram[m_nvram_addr] = m_nvram_data;
		//if (m_nvram_addr == 0xb1) machine().debug_break();
	}
}

/**
 * @brief read input groups A, B, C of lamp + apm I/O chip (U3)
 * @param offset 0 ... 2 = group
 * @return 4-bit value read from the group
 */
u8 gts1_state::gts1_lamp_apm_r(offs_t offset) // Think this works - dips seem to work correctly
{
	u8 data = 0x0f;
	switch (offset) {
		case 0: // group A switches S01-S04, S09-S12, S17-S20
			if (m_z30_out < 3)
				data = BIT(m_dips[m_z30_out]->read(),0,4);
			break;
		case 1: // group B switches S05-S08, S13-S16, S21-S24
			if (m_z30_out < 3)
				data = BIT(m_dips[m_z30_out]->read(),4,4);
			break;
		case 2: // 3 hardwired switches
			data = m_switches[6]->read();
			break;
	}
	//LOG("%s: offs=%d, m_z30_out=%d, data=%d\n", __FUNCTION__, offset, m_z30_out, data);
	return data;
}

/**
 * @brief write output groups A, B, C of lamp + apm I/O chip (U3)
 * @param offset 0 ... 2 = group
 * @param data 4 bit value to write
 */
void gts1_state::gts1_lamp_apm_w(offs_t offset, u8 data)
{
	u8 sndcmd = 0;
	switch (offset) {
		case 0: // LD1-LD4 on jumper J5
			m_lamp_data = data & 15;
			break;
		case 1: // Z30 1-of-16 decoder
			m_z30_out = ~data & 15;
			if (m_z30_out == 1)
			{
				if (m_p2_sound)
				{
					// Sound card has inputs from tilt and game over relays
					if (BIT(m_lamp_data, 0))
						sndcmd |= 0x40;
					if (BIT(m_lamp_data, 1))
						sndcmd |= 0x08;
					m_p2_sound->write(sndcmd);
				}
			}
			if ((m_z30_out >= 1) && (m_z30_out <= 9))
				for (u8 i = 0; i < 4; i++)
					m_io_outputs[4+m_z30_out*4+i] = BIT(m_lamp_data, i);
			break;
		case 2: // O9: PGOL PROM A8, O10: PGOL PROM A9
			m_6351_addr = (m_6351_addr & 0xff) | ((data & 3) << 8);
			// O11 and O12 are unused
			break;
	}
}

u8 gts1_state::gts1_pa_r()
{
	u16 addr = m_6351_addr ^ 0x3ff;
	// return nibble from personality module ROM
	u8 data = m_pm[addr];
	LOG("%s: PROM READ @[%03x]:%02x\n", __FUNCTION__, addr, data);
	//machine().debug_break();
	return data;
}

void gts1_state::gts1_do_w(u8 data)
{
	// write address lines (DO1-4 to A0-3, DIO1-4 to A4-7)
	m_6351_addr = (m_6351_addr & 0x300) | data;
	LOG("%s: PROM addr:%02x\n", __FUNCTION__, m_6351_addr);
}


void gts1_state::p0(machine_config & config)
{
	/* basic machine hardware */
	PPS4_2(config, m_maincpu, XTAL(3'579'545));  // divided by 18 in the CPU
	m_maincpu->set_addrmap(AS_PROGRAM, &gts1_state::gts1_map);
	m_maincpu->set_addrmap(AS_DATA, &gts1_state::gts1_data);
	m_maincpu->set_addrmap(AS_IO, &gts1_state::gts1_io);
	m_maincpu->dia_cb().set(FUNC(gts1_state::gts1_pa_r));
	m_maincpu->do_cb().set(FUNC(gts1_state::gts1_do_w));

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	/* A1753CE 2048 x 8 ROM (000-7ff), 128 x 4 RAM (00-7f) and 16 I/O lines (20 ... 2f) */
	ra17xx_device &u5(RA17XX(config, "u5", 0));
	u5.iord_cb().set(FUNC(gts1_state::gts1_switches_r));
	u5.iowr_cb().set(FUNC(gts1_state::gts1_switches_w));
	u5.set_cpu_tag(m_maincpu);

	/* A1752CF 2048 x 8 ROM (800-fff), 128 x 4 RAM (80-ff) and 16 I/O lines (40 ... 4f) */
	ra17xx_device &u4(RA17XX(config, "u4", 0));
	u4.iord_cb().set(FUNC(gts1_state::gts1_solenoid_r));
	u4.iowr_cb().set(FUNC(gts1_state::gts1_solenoid_w));
	u4.set_cpu_tag(m_maincpu);

	/* 10696 General Purpose Input/Output */
	r10696_device &u2(R10696(config, "u2", 0));
	u2.iord_cb().set(FUNC(gts1_state::gts1_nvram_r));
	u2.iowr_cb().set(FUNC(gts1_state::gts1_nvram_w));

	/* 10696 General Purpose Input/Output */
	r10696_device &u3(R10696(config, "u3", 0));
	u3.iord_cb().set(FUNC(gts1_state::gts1_lamp_apm_r));
	u3.iowr_cb().set(FUNC(gts1_state::gts1_lamp_apm_w));

	/* 10788 General Purpose Display and Keyboard */
	r10788_device &u6(R10788(config, "u6", XTAL(3'579'545) / 18 ));  // divided in the circuit
	u6.update_cb().set(FUNC(gts1_state::gts1_display_w));

	/* Video */
	config.set_default_layout(layout_gts1);

	/* Sound */
	genpin_audio(config);
}

void gts1_state::p1(machine_config &config)
{
	p0(config);
	SPEAKER(config, "mono").front_center();
	BEEP(config, m_p1_sound, 387).add_route(ALL_OUTPUTS, "mono", 0.50);
}

void gts1_state::p2(machine_config &config)
{
	p0(config);
	SPEAKER(config, "mono").front_center();
	GOTTLIEB_SOUND_PIN2(config, m_p2_sound).add_route(ALL_OUTPUTS, "mono", 1.0);
}


#define GTS1_BIOS \
	ROM_REGION( 0x1000, "maincpu", ROMREGION_ERASEFF ) \
	ROM_LOAD("a1752cf.u5", 0x0000, 0x0800, CRC(614a3bd9) SHA1(febca18fb6f96037ca82e515dd161dfcb0e4c776) ) \
	ROM_LOAD("a1753ce.u4", 0x0800, 0x0800, CRC(4cd312dd) SHA1(31245daa9972ef8652caee69986585bb8239e86e) )


ROM_START( gts1 )
	GTS1_BIOS

	ROM_REGION( 0x0400, "module", ROMREGION_ERASEFF )
ROM_END

/*-------------------------------------------------------------------
/ Asteroid Annie and the Aliens (12/1980) #442
/-------------------------------------------------------------------*/
ROM_START(astannie)
	GTS1_BIOS

	ROM_REGION( 0x0400, "module", 0 )
	ROM_LOAD("442.cpu",   0x0000, 0x0400, CRC(579521e0) SHA1(b1b19473e1ca3373955ee96104b87f586c4c311c))

	ROM_REGION( 0x1000, "p2sound:audiocpu", 0)
	ROM_LOAD("442.snd", 0x0400, 0x0400, CRC(c70195b4) SHA1(ff06197f07111d6a4b8942dcfe8d2279bda6f281))

	ROM_REGION( 0x0400, "p2sound:r6530", 0 )
	ROM_LOAD("6530sys1.bin", 0x0000, 0x0400, CRC(b7831321) SHA1(c94f4bee97854d0373653a6867016e27d3fc1340))
ROM_END

/*-------------------------------------------------------------------
/ Buck Rogers (01/1980) #437
/-------------------------------------------------------------------*/
ROM_START(buckrgrs)
	GTS1_BIOS

	ROM_REGION( 0x0400, "module", 0 )
	ROM_LOAD("437.cpu",   0x0000, 0x0400, CRC(e57d9278) SHA1(dfc4ebff1e14b9a074468671a8e5ac7948d5b352))

	ROM_REGION( 0x1000, "p2sound:audiocpu", 0)
	ROM_LOAD("437.snd", 0x0400, 0x0400, CRC(732b5a27) SHA1(7860ea54e75152246c3ac3205122d750b243b40c))

	ROM_REGION( 0x0400, "p2sound:r6530", 0 )
	ROM_LOAD("6530sys1.bin", 0x0000, 0x0400, CRC(b7831321) SHA1(c94f4bee97854d0373653a6867016e27d3fc1340))
ROM_END

/*-------------------------------------------------------------------
/ Charlie's Angels (11/1978) #425
/-------------------------------------------------------------------*/
ROM_START(charlies)
	GTS1_BIOS

	ROM_REGION( 0x0400, "module", 0 )
	ROM_LOAD("425.cpu",   0x0000, 0x0400, CRC(928b4279) SHA1(51096d45e880d6a8263eaeaa0cdab0f61ad2f58d))
ROM_END
/*-------------------------------------------------------------------
/ Cleopatra (11/1977) #409
/-------------------------------------------------------------------*/
ROM_START(cleoptra)
	GTS1_BIOS

	ROM_REGION( 0x0400, "module", 0 )
	ROM_LOAD("409.cpu",   0x0000, 0x0400, CRC(8063ff71) SHA1(205f09f067bf79544d2ce2a48d23259901f935dd))
ROM_END

/*-------------------------------------------------------------------
/ Close Encounters of the Third Kind (10/1978) #424
/-------------------------------------------------------------------*/
ROM_START(closeenc)
	GTS1_BIOS

	ROM_REGION( 0x0400, "module", 0 )
	ROM_LOAD("424.cpu",   0x0000, 0x0400, CRC(a7a5dd13) SHA1(223c67b9484baa719c91de52b363ff22813db160))
ROM_END

/*-------------------------------------------------------------------
/ Count-Down (05/1979) #422
/-------------------------------------------------------------------*/
ROM_START(countdwn)
	GTS1_BIOS

	ROM_REGION( 0x0400, "module", 0 )
	ROM_LOAD("422.cpu",   0x0000, 0x0400, CRC(51bc2df0) SHA1(d4b555d106c6b4e420b0fcd1df8871f869476c22))
ROM_END

/*-------------------------------------------------------------------
/ Dragon (10/1978) #419
/-------------------------------------------------------------------*/
ROM_START(dragon)
	GTS1_BIOS

	ROM_REGION( 0x0400, "module", 0 )
	ROM_LOAD("419.cpu",   0x0000, 0x0400, CRC(018d9b3a) SHA1(da37ef5017c71bc41bdb1f30d3fd7ac3b7e1ee7e))
ROM_END

/*-------------------------------------------------------------------
/ Genie (11/1979) #435
/-------------------------------------------------------------------*/
ROM_START(geniep)
	GTS1_BIOS

	ROM_REGION( 0x0400, "module", 0 )
	ROM_LOAD("435.cpu",   0x0000, 0x0400, CRC(7749fd92) SHA1(9cd3e799842392e3939877bf295759c27f199e58))

	ROM_REGION( 0x1000, "p2sound:audiocpu", 0)
	ROM_LOAD("435.snd", 0x0400, 0x0400, CRC(4a98ceed) SHA1(f1d7548e03107033c39953ee04b043b5301dbb47))

	ROM_REGION( 0x0400, "p2sound:r6530", 0 )
	ROM_LOAD("6530sys1.bin", 0x0000, 0x0400, CRC(b7831321) SHA1(c94f4bee97854d0373653a6867016e27d3fc1340))
ROM_END

/*-------------------------------------------------------------------
/ Joker Poker (08/1978) #417
/-------------------------------------------------------------------*/
ROM_START(jokrpokr)
	GTS1_BIOS

	ROM_REGION( 0x0400, "module", 0 )
	ROM_LOAD("417.cpu",   0x0000, 0x0400, CRC(33dade08) SHA1(23b8dbd7b6c84b806fc0d2da95478235cbf9f80a))
ROM_END

/*-------------------------------------------------------------------
/ Jungle Queen (1985)
/-------------------------------------------------------------------*/
// Conversion kit
// Rumoured to use same roms as Pinball Pool

/*-------------------------------------------------------------------
/ L'Hexagone (04/1986)
/-------------------------------------------------------------------*/
ROM_START(hexagone)
	GTS1_BIOS

	ROM_REGION( 0x0400, "module", 0 )
	ROM_LOAD("435.cpu",   0x0000, 0x0400, CRC(7749fd92) SHA1(9cd3e799842392e3939877bf295759c27f199e58))

	ROM_REGION( 0x10000, "audiocpu", 0) // Z-80 code
	ROM_LOAD("hexagone.bin", 0x0000, 0x4000, CRC(002b5464) SHA1(e2d971c4e85b4fb6580c2d3945c9946ea0cebc2e))
ROM_END
/*-------------------------------------------------------------------
/ Movie
/-------------------------------------------------------------------*/

/*-------------------------------------------------------------------
/ Pinball Pool (08/1979) #427
/-------------------------------------------------------------------*/
ROM_START(pinpool)
	GTS1_BIOS

	ROM_REGION( 0x0400, "module", 0 )
	ROM_LOAD("427.cpu",   0x0000, 0x0400, CRC(c496393d) SHA1(e91d9596aacdb4277fa200a3f8f9da099c278f32))
ROM_END

/*-------------------------------------------------------------------
/ Roller Disco (02/1980) #440
/-------------------------------------------------------------------*/
ROM_START(roldisco)
	GTS1_BIOS

	ROM_REGION( 0x0400, "module", 0 )
	ROM_LOAD("440.cpu",   0x0000, 0x0400, CRC(bc50631f) SHA1(6aa3124d09fc4e369d087a5ad6dd1737ace55e41))

	ROM_REGION( 0x1000, "p2sound:audiocpu", 0)
	ROM_LOAD("440.snd", 0x0400, 0x0400, CRC(4a0a05ae) SHA1(88f21b5638494d8e78dc0b6b7d69873b76b5f75d))

	ROM_REGION( 0x0400, "p2sound:r6530", 0 )
	ROM_LOAD("6530sys1.bin", 0x0000, 0x0400, CRC(b7831321) SHA1(c94f4bee97854d0373653a6867016e27d3fc1340))
ROM_END

/*-------------------------------------------------------------------
/ Sahara Love (1984)
/-------------------------------------------------------------------*/
ROM_START(sahalove)
	GTS1_BIOS

	ROM_REGION( 0x0400, "module", 0 )
	ROM_LOAD("412.cpu",   0x0000, 0x0400, CRC(84a86b83) SHA1(f331f2ffd7d1b279b4ffbb939aa8649e723f5fac))

	ROM_REGION( 0x4000, "audiocpu", 0) // extra Z80 for sound. TODO: emulate
	ROM_LOAD("sahalove.bin", 0x0000, 0x2000,  CRC(3512840a) SHA1(eb36bb78bbf2f8610bc1d71a6651b937db3a5c69))
ROM_END

/*-------------------------------------------------------------------
/ Sinbad (05/1978) #412
/-------------------------------------------------------------------*/
ROM_START(sinbad)
	GTS1_BIOS

	ROM_REGION( 0x0400, "module", 0 )
	ROM_LOAD("412.cpu",   0x0000, 0x0400, CRC(84a86b83) SHA1(f331f2ffd7d1b279b4ffbb939aa8649e723f5fac))
ROM_END

ROM_START(sinbadn)
	GTS1_BIOS

	ROM_REGION( 0x0400, "module", 0 )
	ROM_LOAD("412no1.cpu", 0x0000, 0x0400, CRC(f5373f5f) SHA1(027840501416ff01b2adf07188c7d667adf3ad5f))
ROM_END

/*-------------------------------------------------------------------
/ Sky Warrior (1983)
/-------------------------------------------------------------------*/
// Conversion kit

/*-------------------------------------------------------------------
/ Solar Ride (02/1979) #421
/-------------------------------------------------------------------*/
ROM_START(solaride)
	GTS1_BIOS

	ROM_REGION( 0x0400, "module", 0 )
	ROM_LOAD("421.cpu",   0x0000, 0x0400, CRC(6b5c5da6) SHA1(a09b7009473be53586f53f48b7bfed9a0c5ecd55))
ROM_END

/*-------------------------------------------------------------------
/ The Incredible Hulk (10/1979) #433
/-------------------------------------------------------------------*/
ROM_START(hulk)
	GTS1_BIOS

	ROM_REGION( 0x0400, "module", 0 )
	ROM_LOAD("433.cpu",   0x0000, 0x0400, CRC(c05d2b52) SHA1(393fe063b029246317c90ee384db95a84d61dbb7))

	ROM_REGION( 0x1000, "p2sound:audiocpu", 0)
	ROM_LOAD("433.snd", 0x0400, 0x0400, CRC(20cd1dff) SHA1(93e7c47ff7051c3c0dc9f8f95aa33ba094e7cf25))

	ROM_REGION( 0x0400, "p2sound:r6530", 0 )
	ROM_LOAD("6530sys1.bin", 0x0000, 0x0400, CRC(b7831321) SHA1(c94f4bee97854d0373653a6867016e27d3fc1340))
ROM_END

/*-------------------------------------------------------------------
/ Torch (02/1980) #438
/-------------------------------------------------------------------*/
ROM_START(torch)
	GTS1_BIOS

	ROM_REGION( 0x0400, "module", 0 )
	ROM_LOAD("438.cpu",   0x0000, 0x0400, CRC(2d396a64) SHA1(38a1862771500faa471071db08dfbadc6e8759e8))

	ROM_REGION( 0x1000, "p2sound:audiocpu", 0)
	ROM_LOAD("438.snd", 0x0400, 0x0400, CRC(a9619b48) SHA1(1906bc1b059bf31082e3b4546f5a30159479ad3c))

	ROM_REGION( 0x0400, "p2sound:r6530", 0 )
	ROM_LOAD("6530sys1.bin", 0x0000, 0x0400, CRC(b7831321) SHA1(c94f4bee97854d0373653a6867016e27d3fc1340))
ROM_END

/*-------------------------------------------------------------------
/ Totem (10/1979) #429
/-------------------------------------------------------------------*/
ROM_START(totem)
	GTS1_BIOS

	ROM_REGION( 0x0400, "module", 0 )
	ROM_LOAD("429.cpu",   0x0000, 0x0400, CRC(7885a384) SHA1(1770662af7d48ad8297097a9877c5c497119978d))

	ROM_REGION( 0x1000, "p2sound:audiocpu", 0)
	ROM_LOAD("429.snd", 0x0400, 0x0400, CRC(5d1b7ed4) SHA1(4a584f880e907fb21da78f3b3a0617f20599688f))

	ROM_REGION( 0x0400, "p2sound:r6530", 0 )
	ROM_LOAD("6530sys1.bin", 0x0000, 0x0400, CRC(b7831321) SHA1(c94f4bee97854d0373653a6867016e27d3fc1340))
ROM_END

/*-------------------------------------------------------------------
/ System 1 Test prom
/-------------------------------------------------------------------*/
ROM_START(sys1test)
	GTS1_BIOS

	ROM_REGION( 0x0400, "module", 0 )
	ROM_LOAD("test.cpu",  0x0000, 0x0400, CRC(8b0704bb) SHA1(5f0eb8d5af867b815b6012c9d078927398efe6d8))
ROM_END

} // Anonymous namespace

GAME(1977,  gts1,     0,      p0,  gts1,     gts1_state, empty_init, ROT0, "Gottlieb",         "System 1",                             MACHINE_IS_BIOS_ROOT | MACHINE_NOT_WORKING)
GAME(19??,  sys1test, gts1,   p0,  gts1,     gts1_state, empty_init, ROT0, "Gottlieb",         "System 1 Test prom",                   MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )

// chimes
GAME(1977,  cleoptra, gts1,   p0,  gts1,     gts1_state, empty_init, ROT0, "Gottlieb",         "Cleopatra",                            MACHINE_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME(1978,  sinbad,   gts1,   p0,  gts1,     gts1_state, empty_init, ROT0, "Gottlieb",         "Sinbad",                               MACHINE_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME(1978,  sinbadn,  sinbad, p0,  gts1,     gts1_state, empty_init, ROT0, "Gottlieb",         "Sinbad (Norway)",                      MACHINE_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME(1978,  jokrpokr, gts1,   p0,  jokrpokr, gts1_state, empty_init, ROT0, "Gottlieb",         "Joker Poker",                          MACHINE_MECHANICAL | MACHINE_SUPPORTS_SAVE )

// NE555 beeper
GAME(1978,  dragon,   gts1,   p1,  gts1,     gts1_state, empty_init, ROT0, "Gottlieb",         "Dragon",                               MACHINE_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME(1979,  solaride, gts1,   p1,  gts1,     gts1_state, empty_init, ROT0, "Gottlieb",         "Solar Ride",                           MACHINE_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME(1979,  countdwn, gts1,   p1,  gts1,     gts1_state, empty_init, ROT0, "Gottlieb",         "Count-Down",                           MACHINE_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME(1978,  closeenc, gts1,   p1,  gts1,     gts1_state, empty_init, ROT0, "Gottlieb",         "Close Encounters of the Third Kind",   MACHINE_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME(1978,  charlies, gts1,   p1,  gts1,     gts1_state, empty_init, ROT0, "Gottlieb",         "Charlie's Angels",                     MACHINE_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME(1979,  pinpool,  gts1,   p1,  gts1,     gts1_state, empty_init, ROT0, "Gottlieb",         "Pinball Pool",                         MACHINE_MECHANICAL | MACHINE_SUPPORTS_SAVE )

// sound card
GAME(1979,  totem,    gts1,   p2,  gts1,     gts1_state, empty_init, ROT0, "Gottlieb",         "Totem",                                MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME(1979,  hulk,     gts1,   p2,  gts1,     gts1_state, empty_init, ROT0, "Gottlieb",         "The Incredible Hulk",                  MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME(1979,  geniep,   gts1,   p2,  gts1,     gts1_state, empty_init, ROT0, "Gottlieb",         "Genie (Pinball)",                      MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME(1980,  buckrgrs, gts1,   p2,  gts1,     gts1_state, empty_init, ROT0, "Gottlieb",         "Buck Rogers",                          MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME(1980,  torch,    gts1,   p2,  gts1,     gts1_state, empty_init, ROT0, "Gottlieb",         "Torch",                                MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME(1980,  roldisco, gts1,   p2,  gts1,     gts1_state, empty_init, ROT0, "Gottlieb",         "Roller Disco",                         MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME(1980,  astannie, gts1,   p2,  gts1,     gts1_state, empty_init, ROT0, "Gottlieb",         "Asteroid Annie and the Aliens",        MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )

// other manufacturer
GAME(1984,  sahalove, sinbad, p0,  gts1,     gts1_state, empty_init, ROT0, "Christian Tabart", "Sahara Love (France)",                 MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME(1986,  hexagone, gts1,   p0,  gts1,     gts1_state, empty_init, ROT0, "Christian Tabart", "L'Hexagone (France)",                  MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
