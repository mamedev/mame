// license:BSD-3-Clause
// copyright-holders:Robbbert
/****************************************************************************************

PINBALL
Williams System 7

Differences to system 6:
- Extra PIA at 0x2100 to handle sound and more solenoids.
- Diag digit
- Leading zero suppression
- Commas
- 7 digits for each player

Diagnostic actions:
- You must be in game over mode. All buttons are in the number-pad. When you are
  finished, you must reboot.

- Setup: NUM-2 must be in auto/up position. Press NUM-1 to enter setup mode, press
   NUM-2 to change direction.

- Tests: NUM-2 must be in manual/down position. Press NUM-1 twice and tests will
   begin. Press NUM-1 and NUM-2 together to get from test 1 to test 2. Press NUM-2
   to switch between auto/manual stepping.

- Auto Diag Test: Set Dips to SW6. Press NUM-0. Press NUM-ENTER. Press NUM-1. Tests
   will begin.

- Other: see the manual


When first started, it shows the game number and stops. Press F3 to initialise the
nvram. In theory you can then press the diagnostic buttons; or you then enter coins
and start playing.

Most games are multiball - here are the key codes:

Game                              NUM  Start game                End ball
-----------------------------------------------------------------------------------------------
Black Knight                      500  ASD hit 1                 Hit X, hold A, hit X, hold S, hit X, hit D
Cosmic Gunfight (Dragonfly)       502  AS hit 1                  AS
Jungle Lord                       503  AS hit 1                  Hit X, hold A, hit X, hit S
Pharaoh                           504  AS hit 1                  AS
Cyclone (unreleased)              505
Black Knight Limited Edition      506  (600 produced)
Solar Fire                        507  ASD hit 1                 ASD
Thunderball                       508  1                         unknown
HyperBall                         509  1                         unknown
Barracora                         510  ASD hit 1                 ASD
Varkon                            512  AX hit 1                  AX
Spellbinder (unreleased)          513
Reflex (unreleased)               514
Time Fantasy                      515  1                         X
Warlok                            516  1                         X
Defender                          517  hold up,left,right,hit 1  Hit X, hold Left, hit X, hold Up, Hit X, hit Right.
Joust                             519  ABCD hit 1                unknown
Laser Cue                         520  1                         X
Firepower II                      521  AS hit 1                  AS
Wild Texas                      *(521) AS hit 1                  AS
Guardian (unreleased)             523
Star Fighter (unreleased)         524
Light Speed (unreleased)          528
Starlight                         530  AS hit 1                  AS

*Wild Texas is a clone/bootleg of Firepower II, and shows the same game number.

Status:
- All machines are playable
- Thunderball: turn Speech DIP off, or you get corrupt sound.

ToDo:
- Some games have an additional alphanumeric display, or different display arrangements
- Mechanical sounds vary per machine
- Hyperball, status display is different

*****************************************************************************************/

#include "emu.h"
#include "genpin.h"

#include "williamssound.h"

#include "cpu/m6800/m6800.h"
#include "machine/6821pia.h"
#include "speaker.h"

#include "s7.lh"


namespace {

class s7_state : public genpin_class
{
public:
	s7_state(const machine_config &mconfig, device_type type, const char *tag)
		: genpin_class(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_s6sound(*this, "s6sound")
		, m_pia21(*this, "pia21")
		, m_pia22(*this, "pia22")
		, m_pia24(*this, "pia24")
		, m_pia28(*this, "pia28")
		, m_pia30(*this, "pia30")
		, m_io_keyboard(*this, "X%d", 0U)
		, m_dips(*this, "DS%d", 1U)
		, m_digits(*this, "digit%d", 0U)
		, m_io_outputs(*this, "out%d", 0U)
	{ }

	void s7(machine_config &config);
	void init_1() { m_game = 1U; }

	DECLARE_INPUT_CHANGED_MEMBER(main_nmi);
	DECLARE_INPUT_CHANGED_MEMBER(diag_coin);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	TIMER_CALLBACK_MEMBER(irq_timer);

private:
	void dig0_w(u8 data);
	void dig1_w(u8 data);
	void lamp0_w(u8 data);
	void lamp1_w(u8 data);
	void sol0_w(u8 data);
	void sol1_w(u8 data);
	void sol2_w(u8 data);
	void sound_w(u8 data);
	u8 dips_r();
	u8 switch_r();
	void switch_w(u8 data);
	u8 nvram_r(offs_t offset);
	void nvram_w(offs_t offset, u8 data);
	void pia21_ca2_w(int state) { }
	void pia21_cb2_w(int state) { } // enable solenoids
	void pia22_ca2_w(int state) { m_io_outputs[20] = state; } //ST5
	void pia22_cb2_w(int state) { } //ST-solenoids enable
	void pia24_ca2_w(int state) { m_io_outputs[17] = state; } //ST2
	void pia24_cb2_w(int state) { m_io_outputs[16] = state; } //ST1
	void pia28_ca2_w(int state) { } //diag leds enable
	void pia28_cb2_w(int state) { m_io_outputs[21] = state; } //ST6
	void pia30_ca2_w(int state) { m_io_outputs[19] = state; } //ST4
	void pia30_cb2_w(int state) { m_io_outputs[18] = state; } //ST3
	void pia_irq(int state);
	void main_map(address_map &map) ATTR_COLD;

	u8 m_strobe = 0U;
	u8 m_row = 0U;
	u8 m_comma = 0U;
	u8 m_nvram[0x100]{};
	bool m_data_ok = false;
	u8 m_lamp_data = 0U;
	bool m_memprotect = false;
	u8 m_game = 0U;
	emu_timer* m_irq_timer = nullptr;

	required_device<cpu_device> m_maincpu;
	required_device<williams_s6_sound_device> m_s6sound;
	required_device<pia6821_device> m_pia21;
	required_device<pia6821_device> m_pia22;
	required_device<pia6821_device> m_pia24;
	required_device<pia6821_device> m_pia28;
	required_device<pia6821_device> m_pia30;
	required_ioport_array<8> m_io_keyboard;
	required_ioport_array<2> m_dips;
	output_finder<61> m_digits;
	output_finder<86> m_io_outputs; // 22 solenoids + 64 lamps
};

void s7_state::main_map(address_map &map)
{
	map.global_mask(0x7fff);
	map(0x0000, 0x00ff).ram().mirror(0x1000);
	map(0x0100, 0x01ff).rw(FUNC(s7_state::nvram_r), FUNC(s7_state::nvram_w));
	map(0x0200, 0x03ff).ram().mirror(0x1000);
	map(0x1100, 0x11ff).ram();
	map(0x2100, 0x2103).rw(m_pia21, FUNC(pia6821_device::read), FUNC(pia6821_device::write)); // sound+solenoids
	map(0x2200, 0x2203).rw(m_pia22, FUNC(pia6821_device::read), FUNC(pia6821_device::write)); // solenoids
	map(0x2400, 0x2403).rw(m_pia24, FUNC(pia6821_device::read), FUNC(pia6821_device::write)); // lamps
	map(0x2800, 0x2803).rw(m_pia28, FUNC(pia6821_device::read), FUNC(pia6821_device::write)); // display
	map(0x3000, 0x3003).rw(m_pia30, FUNC(pia6821_device::read), FUNC(pia6821_device::write)); // inputs
	map(0x4000, 0x4003).nopw();  // splbn writes here
	map(0x5000, 0x7fff).rom().region("maincpu", 0 );
}

static INPUT_PORTS_START( s7 )
	PORT_START("X0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_8) PORT_NAME("Plumb Tilt") // 3 touches before it tilts
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_9) PORT_NAME("Tilt")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_START )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_COIN3 )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_0) PORT_NAME("Slam Tilt")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_4_PAD) PORT_NAME("High Score Reset")

	PORT_START("X1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_X) PORT_NAME("Outhole")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_A) PORT_NAME("INP10")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_B) PORT_NAME("INP11")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_C) PORT_NAME("INP12")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_D) PORT_NAME("INP13")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_E) PORT_NAME("INP14")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_F) PORT_NAME("INP15")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_G) PORT_NAME("INP16")

	PORT_START("X2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_H) PORT_NAME("INP17")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_I) PORT_NAME("INP18")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_J) PORT_NAME("INP19")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_K) PORT_NAME("INP20")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_L) PORT_NAME("INP21")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_M) PORT_NAME("INP22")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_N) PORT_NAME("INP23")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_O) PORT_NAME("INP24")

	PORT_START("X3")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_P) PORT_NAME("INP25")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_Q) PORT_NAME("INP26")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_R) PORT_NAME("INP27")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_S) PORT_NAME("INP28")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_T) PORT_NAME("INP29")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_U) PORT_NAME("INP30")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_V) PORT_NAME("INP31")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_W) PORT_NAME("INP32")

	PORT_START("X4")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_Y) PORT_NAME("INP33")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_Z) PORT_NAME("INP34")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_COMMA) PORT_NAME("INP35")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_STOP) PORT_NAME("INP36")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_SLASH) PORT_NAME("INP37")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_COLON) PORT_NAME("INP38")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_QUOTE) PORT_NAME("INP39")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_ENTER) PORT_NAME("INP40")

	PORT_START("X5")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_OPENBRACE) PORT_NAME("INP41")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_CLOSEBRACE) PORT_NAME("INP42")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_BACKSLASH) PORT_NAME("INP43")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_MINUS) PORT_NAME("INP44")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_EQUALS) PORT_NAME("INP45")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_BACKSPACE) PORT_NAME("INP46")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_UP) PORT_NAME("INP47")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_LEFT) PORT_NAME("INP48")

	PORT_START("X6")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_RIGHT) PORT_NAME("INP49")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_DOWN) PORT_NAME("INP50")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_PGUP) PORT_NAME("INP51")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_PGDN) PORT_NAME("INP52")

	PORT_START("X7")

	PORT_START("DIAGS")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Main Diag") PORT_CODE(KEYCODE_0_PAD) PORT_CHANGED_MEMBER(DEVICE_SELF, s7_state, main_nmi, 1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Advance") PORT_CODE(KEYCODE_1_PAD)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Manual/Auto") PORT_CODE(KEYCODE_2_PAD) PORT_TOGGLE
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Enter") PORT_CODE(KEYCODE_ENTER_PAD)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Coin Door") PORT_CODE(KEYCODE_F1) PORT_CHANGED_MEMBER(DEVICE_SELF, s7_state, diag_coin, 1) PORT_TOGGLE

	PORT_START("DS1") // DS1 switches exist but do nothing
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("DS2") // DS2 switches exist but do nothing
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END

static INPUT_PORTS_START( bk )
	PORT_INCLUDE(s7)
	PORT_MODIFY("X1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_H) PORT_NAME("INP09")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_I) PORT_NAME("INP10")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_J) PORT_NAME("INP13")
	PORT_MODIFY("X2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_A) PORT_NAME("INP17")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_S) PORT_NAME("INP18")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_D) PORT_NAME("INP19")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_X) PORT_NAME("Outhole")
	PORT_MODIFY("X3")
	PORT_BIT( 0x88, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_MODIFY("X4")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_MODIFY("X5")
	PORT_BIT( 0xe0, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_MODIFY("X6")
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END

static INPUT_PORTS_START( csmic )
	PORT_INCLUDE(s7)
	PORT_MODIFY("X1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_H) PORT_NAME("INP09")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_R) PORT_NAME("INP10")
	PORT_MODIFY("X2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_MODIFY("X3")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_A) PORT_NAME("INP27")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_X) PORT_NAME("Outhole")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END

static INPUT_PORTS_START( jngld )
	PORT_INCLUDE(s7)
	PORT_MODIFY("X1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_S) PORT_NAME("INP09")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_MODIFY("X3")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_OPENBRACE) PORT_NAME("INP28")
	PORT_MODIFY("X5")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_X) PORT_NAME("Outhole")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_MODIFY("X6")
	PORT_BIT( 0xfc, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END

static INPUT_PORTS_START( pharo )
	PORT_INCLUDE(s7)
	PORT_MODIFY("X1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("Left Magnet")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("Right Magnet")
	PORT_BIT( 0x0c, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_MODIFY("X3")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_B) PORT_NAME("INP28")
	PORT_MODIFY("X4")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_X) PORT_NAME("Outhole")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_A) PORT_NAME("INP38")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_S) PORT_NAME("INP39")
	PORT_MODIFY("X5")
	PORT_BIT( 0xf2, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_MODIFY("X6")
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END

static INPUT_PORTS_START( solar )
	PORT_INCLUDE(s7)
	PORT_MODIFY("X1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("Left Magnet")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("Right Magnet")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_M) PORT_NAME("INP13")
	PORT_MODIFY("X2")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_X) PORT_NAME("Outhole")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_A) PORT_NAME("INP21")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_S) PORT_NAME("INP22")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_D) PORT_NAME("INP23")
	PORT_MODIFY("X3")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_L) PORT_NAME("INP28")
	PORT_MODIFY("X5")
	PORT_BIT( 0xfc, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_MODIFY("X6")
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END

static INPUT_PORTS_START( thund )
	PORT_INCLUDE(s7)
	//PORT_MODIFY("X4")
	//PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END

static INPUT_PORTS_START( hypbl )
	PORT_INCLUDE(s7)
	PORT_MODIFY("X0")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_MODIFY("X4")
	PORT_BIT( 0xfc, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_MODIFY("X5")
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_MODIFY("X6")
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END

static INPUT_PORTS_START( barra )
	PORT_INCLUDE(s7)
	PORT_MODIFY("X1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_C) PORT_NAME("INP09")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_E) PORT_NAME("INP10")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_B) PORT_NAME("INP11")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_X) PORT_NAME("Outhole")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_D) PORT_NAME("INP13")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_A) PORT_NAME("INP14")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_S) PORT_NAME("INP15")
	PORT_MODIFY("X3")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_F) PORT_NAME("INP28")
	PORT_MODIFY("X4")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_MODIFY("X6")
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END

static INPUT_PORTS_START( vrkon )
	PORT_INCLUDE(s7)
	PORT_MODIFY("X0")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_MODIFY("X1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_X) PORT_NAME("Lower Outhole")
	PORT_BIT( 0x0c, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_MODIFY("X4")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_A) PORT_NAME("Upper Outhole")
	PORT_MODIFY("X5")
	PORT_BIT( 0xfe, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_MODIFY("X6")
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END

static INPUT_PORTS_START( tmfnt )
	PORT_INCLUDE(s7)
	PORT_MODIFY("X1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_QUOTE) PORT_NAME("INP09")
	PORT_MODIFY("X4")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_X) PORT_NAME("Outhole")
	PORT_MODIFY("X5")
	PORT_BIT( 0xfd, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_MODIFY("X6")
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END

static INPUT_PORTS_START( wrlok )
	PORT_INCLUDE(s7)
	PORT_MODIFY("X4")
	PORT_BIT( 0xc0, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_MODIFY("X5")
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_MODIFY("X6")
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END

static INPUT_PORTS_START( dfndr )
	PORT_INCLUDE(s7)
	PORT_MODIFY("X1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_BACKSPACE) PORT_NAME("INP09")
	PORT_MODIFY("X5")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_X) PORT_NAME("Outhole")
	PORT_MODIFY("X6")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_HOME) PORT_NAME("INP53")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_END) PORT_NAME("INP54")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_DEL) PORT_NAME("INP55")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_SPACE) PORT_NAME("INP56")
	PORT_MODIFY("X7")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_RCONTROL) PORT_NAME("INP57")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_RSHIFT) PORT_NAME("INP58")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_RALT) PORT_NAME("INP59")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("Smart Bomb")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("Reverse")
INPUT_PORTS_END

static INPUT_PORTS_START( jst )
	PORT_INCLUDE(s7)
	PORT_MODIFY("X0")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_MODIFY("X1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_X) PORT_NAME("P1 Outhole")
	PORT_MODIFY("X3")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_MODIFY("X4")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_Y) PORT_NAME("P2 Outhole")
	PORT_MODIFY("X6")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_HOME) PORT_NAME("INP53")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_END) PORT_NAME("INP54")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_DEL) PORT_NAME("INP55")
	PORT_MODIFY("X7")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("P1 Left Flap")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("P1 RightFlap")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_NAME("P2 Left Flap")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_NAME("P2 Right Flap")
INPUT_PORTS_END

static INPUT_PORTS_START( lsrcu )
	PORT_INCLUDE(s7)
	PORT_MODIFY("X5")
	PORT_BIT( 0xf8, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_MODIFY("X6")
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END

static INPUT_PORTS_START( fpwr2 )
	PORT_INCLUDE(s7)
	PORT_MODIFY("X1")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_S) PORT_NAME("INP11")
	PORT_MODIFY("X2")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_MODIFY("X3")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_B) PORT_NAME("INP28")
	PORT_MODIFY("X6")
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END

static INPUT_PORTS_START( strlt )
	PORT_INCLUDE(s7)
	PORT_MODIFY("X1")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_S) PORT_NAME("INP11")
	PORT_MODIFY("X3")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_B) PORT_NAME("INP28")
	PORT_MODIFY("X6")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_END) PORT_NAME("INP54")
INPUT_PORTS_END

INPUT_CHANGED_MEMBER( s7_state::main_nmi )
{
	// Diagnostic button sends a pulse to NMI pin
	if (newval==CLEAR_LINE)
		m_maincpu->pulse_input_line(INPUT_LINE_NMI, attotime::zero);
}

INPUT_CHANGED_MEMBER( s7_state::diag_coin )
{
	m_memprotect = newval;
}

void s7_state::sol0_w(u8 data)
{
	//if (BIT(data, 0))
		//m_samples->start(5, 5); // outhole

	for (u8 i = 0; i < 8; i++)
		m_io_outputs[i] = BIT(data, i);
}

void s7_state::sol1_w(u8 data)
{
	//if (BIT(data, 6))
		//m_samples->start(0, 6); // knocker

	for (u8 i = 0; i < 8; i++)
		m_io_outputs[8U+i] = BIT(data, i);
}

void s7_state::sol2_w(u8 data)
{
	m_comma = data & 0xc0;
	m_pia21->ca1_w(BIT(data, 5));
}

void s7_state::sound_w(u8 data)
{
	u8 t = m_game ? 0x3f : 0x1f;
	u8 t3 = (data & t) | ~t;
	m_s6sound->write(t3);
}

void s7_state::lamp0_w(u8 data)
{
	m_lamp_data = data ^ 0xff;
}

void s7_state::lamp1_w(u8 data)
{
	// find out which row is active
	for (u8 i = 0; i < 8; i++)
		if (BIT(data, i))
			for (u8 j = 0; j < 8; j++)
				m_io_outputs[22U+i*8U+j] = BIT(m_lamp_data, j);
}

void s7_state::dig0_w(u8 data)
{
	static const u8 patterns[16] = { 0x3f, 0x06, 0x5b, 0x4f, 0x66, 0x6d, 0x7c, 0x07, 0x7f, 0x67, 0x58, 0x4c, 0x62, 0x69, 0x78, 0 }; // 7447
	m_strobe = data & 15;
	data ^= 0xf0; // inverted by ic33
	m_data_ok = true;
	m_digits[60] = patterns[data>>4]; // diag digit
}

void s7_state::dig1_w(u8 data)
{
	static const u8 patterns[16] = { 0x3f,0x06,0xdb,0xcf,0xe6,0xed,0xfd,0x07,0xff,0xef,0,0,0,0,0,0 }; // MC14543
	if (m_data_ok)
	{
		m_digits[m_strobe+16] = patterns[data & 15] | (BIT(m_comma, 6) ? 0xc000 : 0);
		m_digits[m_strobe] = patterns[data >> 4] | (BIT(m_comma, 7) ? 0xc000 : 0);
	}
	m_data_ok = false;
}

u8 s7_state::dips_r()
{
	if (BIT(ioport("DIAGS")->read(), 4))
		return m_dips[BIT(~m_strobe, 1)]->read() << (BIT(m_strobe, 0) ? 4 : 0);

	return 0xff;
}

u8 s7_state::switch_r()
{
	u8 data = 0;
	for (u8 i = 0; i < 8; i++)
		if (BIT(m_row, i))
			data |= m_io_keyboard[i]->read();

	return data;
}

void s7_state::switch_w(u8 data)
{
	m_row = data;
}

u8 s7_state::nvram_r(offs_t offset)
{
	return m_nvram[offset] | 0xf0;
}

void s7_state::nvram_w(offs_t offset, u8 data)
{
	if (!(m_memprotect && (offset > 0x7f)))
		m_nvram[offset] = data;
}

void s7_state::pia_irq(int state)
{
	if(state == CLEAR_LINE)
	{
		// restart IRQ timer
		m_irq_timer->adjust(attotime::from_ticks(980,3580000/4),1);
	}
	else
	{
		// disable IRQ timer while other IRQs are being handled
		// (counter is reset every 32 cycles while a PIA IRQ is handled)
		m_irq_timer->adjust(attotime::zero);
	}
}

TIMER_CALLBACK_MEMBER(s7_state::irq_timer)
{
	if(param == 1)
	{
		m_maincpu->set_input_line(M6808_IRQ_LINE, ASSERT_LINE);
		m_irq_timer->adjust(attotime::from_ticks(32,3580000/4),0);
		m_pia28->ca1_w(BIT(ioport("DIAGS")->read(), 2));  // Advance
		m_pia28->cb1_w(BIT(ioport("DIAGS")->read(), 3));  // Up/Down
	}
	else
	{
		m_maincpu->set_input_line(M6808_IRQ_LINE, CLEAR_LINE);
		m_irq_timer->adjust(attotime::from_ticks(980,3580000/4),1);
		m_pia28->ca1_w(1);
		m_pia28->cb1_w(1);
	}
}

void s7_state::machine_start()
{
	genpin_class::machine_start();
	m_io_outputs.resolve();
	m_digits.resolve();

	save_item(NAME(m_strobe));
	save_item(NAME(m_row));
	save_item(NAME(m_data_ok));
	save_item(NAME(m_lamp_data));
	save_item(NAME(m_memprotect));
	save_item(NAME(m_nvram));
	save_item(NAME(m_game));
	save_item(NAME(m_comma));

	m_irq_timer = timer_alloc(FUNC(s7_state::irq_timer), this);
	m_irq_timer->adjust(attotime::from_ticks(980,3580000/4),1);
	subdevice<nvram_device>("nvram")->set_base(m_nvram, sizeof(m_nvram));
}

void s7_state::machine_reset()
{
	genpin_class::machine_reset();
	for (u8 i = 0; i < m_io_outputs.size(); i++)
		m_io_outputs[i] = 0;

	m_memprotect = 0;
}

void s7_state::s7(machine_config &config)
{
	/* basic machine hardware */
	M6808(config, m_maincpu, 3580000);
	m_maincpu->set_addrmap(AS_PROGRAM, &s7_state::main_map);

	/* Video */
	config.set_default_layout(layout_s7);

	/* Sound */
	genpin_audio(config);

	/* Devices */
	PIA6821(config, m_pia21);
	m_pia21->readpa_handler().set_constant(0xff); // PA7 unknown input
	m_pia21->readpb_handler().set_constant(0x3f); // PB0-4 unknown inputs
	m_pia21->writepa_handler().set(FUNC(s7_state::sound_w));
	m_pia21->writepb_handler().set(FUNC(s7_state::sol2_w));
	m_pia21->ca2_handler().set(FUNC(s7_state::pia21_ca2_w));
	m_pia21->cb2_handler().set(FUNC(s7_state::pia21_cb2_w));
	m_pia21->irqa_handler().set(FUNC(s7_state::pia_irq));
	m_pia21->irqb_handler().set(FUNC(s7_state::pia_irq));

	PIA6821(config, m_pia22);
	m_pia22->writepa_handler().set(FUNC(s7_state::sol0_w));
	m_pia22->writepb_handler().set(FUNC(s7_state::sol1_w));
	m_pia22->ca2_handler().set(FUNC(s7_state::pia22_ca2_w));
	m_pia22->cb2_handler().set(FUNC(s7_state::pia22_cb2_w));
	m_pia22->irqa_handler().set(FUNC(s7_state::pia_irq));
	m_pia22->irqb_handler().set(FUNC(s7_state::pia_irq));

	PIA6821(config, m_pia24);
	m_pia24->writepa_handler().set(FUNC(s7_state::lamp0_w));
	m_pia24->writepb_handler().set(FUNC(s7_state::lamp1_w));
	m_pia24->ca2_handler().set(FUNC(s7_state::pia24_ca2_w));
	m_pia24->cb2_handler().set(FUNC(s7_state::pia24_cb2_w));
	m_pia24->irqa_handler().set(FUNC(s7_state::pia_irq));
	m_pia24->irqb_handler().set(FUNC(s7_state::pia_irq));

	PIA6821(config, m_pia28);
	m_pia28->readpa_handler().set(FUNC(s7_state::dips_r));
	m_pia28->set_port_a_input_overrides_output_mask(0xff);
	m_pia28->writepa_handler().set(FUNC(s7_state::dig0_w));
	m_pia28->writepb_handler().set(FUNC(s7_state::dig1_w));
	m_pia28->ca2_handler().set(FUNC(s7_state::pia28_ca2_w));
	m_pia28->cb2_handler().set(FUNC(s7_state::pia28_cb2_w));
	m_pia28->irqa_handler().set(FUNC(s7_state::pia_irq));
	m_pia28->irqb_handler().set(FUNC(s7_state::pia_irq));

	PIA6821(config, m_pia30);
	m_pia30->readpa_handler().set(FUNC(s7_state::switch_r));
	m_pia30->set_port_a_input_overrides_output_mask(0xff);
	m_pia30->writepb_handler().set(FUNC(s7_state::switch_w));
	m_pia30->ca2_handler().set(FUNC(s7_state::pia30_ca2_w));
	m_pia30->cb2_handler().set(FUNC(s7_state::pia30_cb2_w));
	m_pia30->irqa_handler().set(FUNC(s7_state::pia_irq));
	m_pia30->irqb_handler().set(FUNC(s7_state::pia_irq));

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	/* Add the soundcard */
	SPEAKER(config, "mono").front_center();
	WILLIAMS_S6_SOUND(config, m_s6sound, 0).add_route(ALL_OUTPUTS, "mono", 1.0);
}



/*----------------------------
/ Black Knight (#500)
/----------------------------*/
ROM_START(bk_l4)
	ROM_REGION(0x3000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD("ic26.716",     0x0800, 0x0800, CRC(104b78da) SHA1(c3af2563b3b380fe0e154b737799f6beacf8998c) )
	ROM_LOAD("ic14.716",     0x1000, 0x0800, CRC(fcbe3d44) SHA1(92ec4d41beea205ba29530624b68dd1139053535) )
	ROM_LOAD("ic20.716",     0x1800, 0x0800, CRC(dfb4b75a) SHA1(bcf017b01236f755cee419e398bbd8955ae3576a) )
	ROM_LOAD("ic17.532",     0x2000, 0x1000, CRC(bb571a17) SHA1(fb0b7f247673dae0744d4188e1a03749a2237165) )

	ROM_REGION(0x5000, "s6sound:audiocpu", ROMREGION_ERASEFF)
	ROM_LOAD("speech7.532",  0x0000, 0x1000, CRC(c7e229bf) SHA1(3b2ab41031f507963af828639f1690dc350737af))
	ROM_LOAD("speech5.532",  0x1000, 0x1000, CRC(411bc92f) SHA1(6c8d26fd13ed5eeba5cc40886d39c65a64beb377))
	ROM_LOAD("speech6.532",  0x2000, 0x1000, CRC(fc985005) SHA1(9df4ad12cf98a5a92b8f933e6b6788a292c8776b))
	ROM_LOAD("speech4.532",  0x3000, 0x1000, CRC(f36f12e5) SHA1(24fb192ad029cd35c08f4899b76d527776a4895b))
	ROM_LOAD("sound12.716",  0x4800, 0x0800, CRC(6d454c0e) SHA1(21640b9ed3bdbae8bf27629891f355304e467c64))
ROM_END

ROM_START(bk_f4)
	ROM_REGION(0x3000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD("ic26.716",     0x0800, 0x0800, CRC(104b78da) SHA1(c3af2563b3b380fe0e154b737799f6beacf8998c) )
	ROM_LOAD("ic14.716",     0x1000, 0x0800, CRC(fcbe3d44) SHA1(92ec4d41beea205ba29530624b68dd1139053535) )
	ROM_LOAD("ic20.716",     0x1800, 0x0800, CRC(dfb4b75a) SHA1(bcf017b01236f755cee419e398bbd8955ae3576a) )
	ROM_LOAD("ic17.532",     0x2000, 0x1000, CRC(bb571a17) SHA1(fb0b7f247673dae0744d4188e1a03749a2237165) )

	ROM_REGION(0x5000, "s6sound:audiocpu", ROMREGION_ERASEFF)
	ROM_LOAD("speech7f.532", 0x0000, 0x1000, CRC(01debff6) SHA1(dc02199b63ae3309fdac819985f7a40010831634))
	ROM_LOAD("speech5f.532", 0x1000, 0x1000, CRC(2d310dce) SHA1(ad2ad3844659787ee9be4db50b17b8af6f5d0d42))
	ROM_LOAD("speech6f.532", 0x2000, 0x1000, CRC(96bb719b) SHA1(d602129ce1af1902e46ca26645a9a51324a788d0))
	ROM_LOAD("speech4f.532", 0x3000, 0x1000, CRC(8ee8fc3c) SHA1(ba7c00f16bdbd7413cec025c28f8b7e7bbcb12bb))
	ROM_LOAD("sound12.716",  0x4800, 0x0800, CRC(6d454c0e) SHA1(21640b9ed3bdbae8bf27629891f355304e467c64))
ROM_END

ROM_START(bk_l3)
	ROM_REGION(0x3000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD("bkl3_26.bin",  0x0800, 0x0800, CRC(6acc34a0) SHA1(3adad61d27e6416630f96554687bb66d3016166a) )
	ROM_LOAD("bkl3_14.bin",  0x1000, 0x0800, CRC(74c37e4f) SHA1(8946b110901d0660676fba0c204aa2bc78223508) )
	ROM_LOAD("ic20.716",     0x1800, 0x0800, CRC(dfb4b75a) SHA1(bcf017b01236f755cee419e398bbd8955ae3576a) )
	ROM_LOAD("ic17.532",     0x2000, 0x1000, CRC(bb571a17) SHA1(fb0b7f247673dae0744d4188e1a03749a2237165) )

	ROM_REGION(0x5000, "s6sound:audiocpu", ROMREGION_ERASEFF)
	ROM_LOAD("speech7.532",  0x0000, 0x1000, CRC(c7e229bf) SHA1(3b2ab41031f507963af828639f1690dc350737af))
	ROM_LOAD("speech5.532",  0x1000, 0x1000, CRC(411bc92f) SHA1(6c8d26fd13ed5eeba5cc40886d39c65a64beb377))
	ROM_LOAD("speech6.532",  0x2000, 0x1000, CRC(fc985005) SHA1(9df4ad12cf98a5a92b8f933e6b6788a292c8776b))
	ROM_LOAD("speech4.532",  0x3000, 0x1000, CRC(f36f12e5) SHA1(24fb192ad029cd35c08f4899b76d527776a4895b))
	ROM_LOAD("sound12.716",  0x4800, 0x0800, CRC(6d454c0e) SHA1(21640b9ed3bdbae8bf27629891f355304e467c64))
ROM_END

ROM_START(bk_l2)
	ROM_REGION(0x3000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD("bk_rev2.ic26", 0x0800, 0x0800, CRC(703b61e1) SHA1(32013d72d70ed0bfca5eb10769471966c07dba09) )
	ROM_LOAD("bk_rev2.ic14", 0x1000, 0x0800, CRC(30d87653) SHA1(2b1b927dfbd7c9ddcea2732e0d4e82c236499338) )
	ROM_LOAD("ic20.716",     0x1800, 0x0800, CRC(dfb4b75a) SHA1(bcf017b01236f755cee419e398bbd8955ae3576a) )
	ROM_LOAD("ic17.532",     0x2000, 0x1000, CRC(bb571a17) SHA1(fb0b7f247673dae0744d4188e1a03749a2237165) )

	ROM_REGION(0x5000, "s6sound:audiocpu", ROMREGION_ERASEFF)
	ROM_LOAD("speech7.532",  0x0000, 0x1000, CRC(c7e229bf) SHA1(3b2ab41031f507963af828639f1690dc350737af))
	ROM_LOAD("speech5.532",  0x1000, 0x1000, CRC(411bc92f) SHA1(6c8d26fd13ed5eeba5cc40886d39c65a64beb377))
	ROM_LOAD("speech6.532",  0x2000, 0x1000, CRC(fc985005) SHA1(9df4ad12cf98a5a92b8f933e6b6788a292c8776b))
	ROM_LOAD("speech4.532",  0x3000, 0x1000, CRC(f36f12e5) SHA1(24fb192ad029cd35c08f4899b76d527776a4895b))
	ROM_LOAD("sound12.716",  0x4800, 0x0800, CRC(6d454c0e) SHA1(21640b9ed3bdbae8bf27629891f355304e467c64))
ROM_END

/*-----------------------------------
/ Cosmic Gunfight (#502)
/-----------------------------------*/
ROM_START(csmic_l1)
	ROM_REGION(0x3000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD("ic26.716",     0x0800, 0x0800, CRC(a259eba0) SHA1(0c5acae3beacb8abb0160dd8a580d3514ca557fe) )
	ROM_LOAD("ic14.716",     0x1000, 0x0800, CRC(ac66c0dc) SHA1(9e2ac0e956008c2d56ffd564c983e127bc4af7ae) )
	ROM_LOAD("ic20.716",     0x1800, 0x0800, CRC(dfb4b75a) SHA1(bcf017b01236f755cee419e398bbd8955ae3576a) )
	ROM_LOAD("ic17.532",     0x2000, 0x1000, CRC(bb571a17) SHA1(fb0b7f247673dae0744d4188e1a03749a2237165) )

	ROM_REGION(0x5000, "s6sound:audiocpu", ROMREGION_ERASEFF)
	ROM_LOAD("sound12.716",  0x4800, 0x0800, CRC(af41737b) SHA1(8be4e7cebe5a821e859550c0350f0fc9cc00b2a9))
ROM_END

/*--------------------------------
/ Jungle Lord (#503)
/--------------------------------*/
ROM_START(jngld_l2)
	ROM_REGION(0x3000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD("ic26.716",     0x0800, 0x0800, CRC(4714b1f1) SHA1(01f8593a926df69fb8ae79260f11c5f6b868cd51) )
	ROM_LOAD("ic14.716",     0x1000, 0x0800, CRC(6e5a6374) SHA1(738ecef807de9fee6fd1e832b35511c11173914c) )
	ROM_LOAD("ic20.716",     0x1800, 0x0800, CRC(dfb4b75a) SHA1(bcf017b01236f755cee419e398bbd8955ae3576a) )
	ROM_LOAD("ic17.532",     0x2000, 0x1000, CRC(bb571a17) SHA1(fb0b7f247673dae0744d4188e1a03749a2237165) )

	ROM_REGION(0x5000, "s6sound:audiocpu", ROMREGION_ERASEFF)
	ROM_LOAD("speech7.532",  0x0000, 0x1000, CRC(83ffb695) SHA1(f9151bdfdefd5c178ca7eb5122f62b700d64f41a))
	ROM_LOAD("speech5.532",  0x1000, 0x1000, CRC(754bd474) SHA1(c05f48bb07085683de469603880eafd28dffd9f5))
	ROM_LOAD("speech6.532",  0x2000, 0x1000, CRC(f2ac6a52) SHA1(5b3e743eac382d571fd049f92ea9955342b9ffa0))
	ROM_LOAD("sound3.716",   0x4800, 0x0800, CRC(55a10d13) SHA1(521d4cdfb0ed8178b3594cedceae93b772a951a4))
ROM_END

ROM_START(jngld_l1)
	ROM_REGION(0x3000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD("ic26-l1.716",  0x0800, 0x0800, CRC(df37bb45) SHA1(60a0670e73f2370d6269ef241b581f5b0ade6ea0) )
	ROM_LOAD("ic14-l1.716",  0x1000, 0x0800, CRC(0144af0d) SHA1(2e5b6e35613decbac10f9b99c7a8cbe7f63b6b07) )
	ROM_LOAD("ic20.716",     0x1800, 0x0800, CRC(dfb4b75a) SHA1(bcf017b01236f755cee419e398bbd8955ae3576a) )
	ROM_LOAD("ic17.532",     0x2000, 0x1000, CRC(bb571a17) SHA1(fb0b7f247673dae0744d4188e1a03749a2237165) )

	ROM_REGION(0x5000, "s6sound:audiocpu", ROMREGION_ERASEFF)
	ROM_LOAD("speech7.532",  0x0000, 0x1000, CRC(83ffb695) SHA1(f9151bdfdefd5c178ca7eb5122f62b700d64f41a))
	ROM_LOAD("speech5.532",  0x1000, 0x1000, CRC(754bd474) SHA1(c05f48bb07085683de469603880eafd28dffd9f5))
	ROM_LOAD("speech6.532",  0x2000, 0x1000, CRC(f2ac6a52) SHA1(5b3e743eac382d571fd049f92ea9955342b9ffa0))
	ROM_LOAD("sound3.716",   0x4800, 0x0800, CRC(55a10d13) SHA1(521d4cdfb0ed8178b3594cedceae93b772a951a4))
ROM_END

/*--------------------------------
/ Pharaoh (#504)
/--------------------------------*/
ROM_START(pharo_l2)
	ROM_REGION(0x3000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD("ic26.716",     0x0800, 0x0800, CRC(2afbcd1f) SHA1(98bb3a74548b7d9c5d7b8432369658ed32e8be07) )
	ROM_LOAD("ic14.716",     0x1000, 0x0800, CRC(cef00088) SHA1(e0c6b776eddc060c42a483de6cc96a1c9f2afcf7) )
	ROM_LOAD("ic20.716",     0x1800, 0x0800, CRC(dfb4b75a) SHA1(bcf017b01236f755cee419e398bbd8955ae3576a) )
	ROM_LOAD("ic17.532",     0x2000, 0x1000, CRC(bb571a17) SHA1(fb0b7f247673dae0744d4188e1a03749a2237165) )

	ROM_REGION(0x5000, "s6sound:audiocpu", ROMREGION_ERASEFF)
	ROM_LOAD("speech7.532",  0x0000, 0x1000, CRC(e087f8a1) SHA1(49c2ad60d82d02f0529329f7cb4b57339d6546c6))
	ROM_LOAD("speech5.532",  0x1000, 0x1000, CRC(d72863dc) SHA1(e24ad970ed202165230fab999be42bea0f861fdd))
	ROM_LOAD("speech6.532",  0x2000, 0x1000, CRC(d29830bd) SHA1(88f6c508f2a7000bbf6c9c26e1029cf9a241d5ca))
	ROM_LOAD("speech4.532",  0x3000, 0x1000, CRC(9ecc23fd) SHA1(bf5947d186141504fd182065533d4efbfd27441d))
	ROM_LOAD("sound12.716",  0x4800, 0x0800, CRC(b0e3a04b) SHA1(eac54376fe77acf46e485ab561a01220910c1fd6))
ROM_END

/*-----------------------------------
/ Solar Fire (#507)
/-----------------------------------*/
ROM_START(solar_l2)
	ROM_REGION(0x3000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD("ic26.716",     0x0800, 0x0800, CRC(b667ee32) SHA1(bb4b5270d9cd36207b68e8c6883538d08aae1778) )
	ROM_LOAD("ic14.716",     0x1000, 0x0800, CRC(cec19a55) SHA1(a1c0f7cc36e5fc7be4e8bcc80896f77eb4c23b1a) )
	ROM_LOAD("ic20.716",     0x1800, 0x0800, CRC(dfb4b75a) SHA1(bcf017b01236f755cee419e398bbd8955ae3576a) )
	ROM_LOAD("ic17.532",     0x2000, 0x1000, CRC(bb571a17) SHA1(fb0b7f247673dae0744d4188e1a03749a2237165) )

	ROM_REGION(0x5000, "s6sound:audiocpu", ROMREGION_ERASEFF)
	ROM_LOAD("sound12.716",  0x4800, 0x0800, CRC(05a2230c) SHA1(c57cd7628310aa8f68ca24217aad1ead066a1a82))
ROM_END

/*-----------------------------------
/ Thunderball (#508) - Prototype
/-----------------------------------*/
ROM_START(thund_p1)  // dated 1982-06-22
	ROM_REGION(0x3000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD("ic20.532",     0x0000, 0x1000, CRC(aa3f07dc) SHA1(f31662972046f9a874380a8dcd1bc9259de5f6ba))
	ROM_LOAD("ic14.532",     0x1000, 0x1000, CRC(1cd34f1f) SHA1(3f5b5a319570c26a3d34d640fef2ac6c04b83b70))
	ROM_LOAD("ic17.532",     0x2000, 0x1000, CRC(bb571a17) SHA1(fb0b7f247673dae0744d4188e1a03749a2237165))

	ROM_REGION(0x5000, "s6sound:audiocpu", ROMREGION_ERASEFF)
	ROM_LOAD("speech7.532",  0x0000, 0x1000, CRC(33e1b041) SHA1(f50c0311bde69fa6e8071e297a81cc3ef3dcf44f))
	ROM_LOAD("speech5.532",  0x1000, 0x1000, CRC(11780c80) SHA1(bcc5efcd69b4f776feef32484a872863847d64cd))
	ROM_LOAD("speech6.532",  0x2000, 0x1000, CRC(ab688698) SHA1(e0cbac44a6fe30a49da478c32500a0b43903cc2b))
	ROM_LOAD("speech4.532",  0x3000, 0x1000, CRC(2a4d6f4b) SHA1(e6f8a1a6e6abc81f980a4938d98abb250e8e1e3b))
	ROM_LOAD("sound12.532",  0x4000, 0x1000, CRC(cc70af52) SHA1(d9c2840acdcd69aab39fc647dd4819eccc06af33))
ROM_END

ROM_START(thund_p2)  // dated 1982-08-31
	ROM_REGION(0x3000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD("ic20_831.532", 0x0000, 0x1000, CRC(91ed089b) SHA1(0e47f5a87cb227a6ee8645931bfa807219b388ef))
	ROM_LOAD("ic14_831.532", 0x1000, 0x1000, CRC(873ccf24) SHA1(2723aa7d059a111374d8145391fbef0c81043e4b))
	ROM_LOAD("ic17.532",     0x2000, 0x1000, CRC(bb571a17) SHA1(fb0b7f247673dae0744d4188e1a03749a2237165))

	ROM_REGION(0x5000, "s6sound:audiocpu", ROMREGION_ERASEFF)
	ROM_LOAD("speech7.532",  0x0000, 0x1000, CRC(33e1b041) SHA1(f50c0311bde69fa6e8071e297a81cc3ef3dcf44f))
	ROM_LOAD("speech5.532",  0x1000, 0x1000, CRC(11780c80) SHA1(bcc5efcd69b4f776feef32484a872863847d64cd))
	ROM_LOAD("speech6.532",  0x2000, 0x1000, CRC(ab688698) SHA1(e0cbac44a6fe30a49da478c32500a0b43903cc2b))
	ROM_LOAD("speech4.532",  0x3000, 0x1000, CRC(2a4d6f4b) SHA1(e6f8a1a6e6abc81f980a4938d98abb250e8e1e3b))
	ROM_LOAD("sound12.532",  0x4000, 0x1000, CRC(cc70af52) SHA1(d9c2840acdcd69aab39fc647dd4819eccc06af33))
ROM_END

ROM_START(thund_p3) // dated 1982-09-08
	ROM_REGION(0x3000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD("ic20_908.532", 0x0000, 0x1000, CRC(21f87917) SHA1(6cfdd5aadafb2d137f2e959fa047ffbe5c09ac2c))
	ROM_LOAD("ic14_908.532", 0x1000, 0x1000, CRC(099e798e) SHA1(38d79622b4d68c69308ee109f47509e0733828ba))
	ROM_LOAD("ic17.532",     0x2000, 0x1000, CRC(bb571a17) SHA1(fb0b7f247673dae0744d4188e1a03749a2237165))

	ROM_REGION(0x5000, "s6sound:audiocpu", ROMREGION_ERASEFF)
	ROM_LOAD("speech7.532",  0x0000, 0x1000, CRC(33e1b041) SHA1(f50c0311bde69fa6e8071e297a81cc3ef3dcf44f))
	ROM_LOAD("speech5.532",  0x1000, 0x1000, CRC(11780c80) SHA1(bcc5efcd69b4f776feef32484a872863847d64cd))
	ROM_LOAD("speech6.532",  0x2000, 0x1000, CRC(ab688698) SHA1(e0cbac44a6fe30a49da478c32500a0b43903cc2b))
	ROM_LOAD("speech4.532",  0x3000, 0x1000, CRC(2a4d6f4b) SHA1(e6f8a1a6e6abc81f980a4938d98abb250e8e1e3b))
	ROM_LOAD("sound12.532",  0x4000, 0x1000, CRC(cc70af52) SHA1(d9c2840acdcd69aab39fc647dd4819eccc06af33))
ROM_END


/*-------------------------------
/ Hyperball (#509)
/-------------------------------*/
ROM_START(hypbl_l4)
	ROM_REGION(0x3000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD("ic20.532",     0x0000, 0x1000, CRC(d13962e8) SHA1(e23310be100060c9803682680066b965aa5efb16))
	ROM_LOAD("ic14.532",     0x1000, 0x1000, CRC(8090fe71) SHA1(0f1f40c0ee8da5b2fd51efeb8be7c20d6465239e))
	ROM_LOAD("ic17.532",     0x2000, 0x1000, CRC(6f4c0c4c) SHA1(1036067e2c85da867983e6e51ee2a7b5135000df))

	ROM_REGION(0x5000, "s6sound:audiocpu", ROMREGION_ERASEFF)
	ROM_LOAD("sound12.532",  0x4000, 0x1000, CRC(06051e5e) SHA1(f0ab4be812ceaf771829dd549f2a612156102a93))
ROM_END

ROM_START(hypbl_l3)
	ROM_REGION(0x3000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD("ic20-l3.532",  0x0000, 0x1000, CRC(4a37d6e8) SHA1(8c26dd5652ace431a6ff0faf0bb9db37489c4fec))
	ROM_LOAD("ic14-l3.532",  0x1000, 0x1000, CRC(e233bbed) SHA1(bb29acc3e48d6b40b3df2e7702f8a8ff4357c15c))
	ROM_LOAD("ic17.532",     0x2000, 0x1000, CRC(6f4c0c4c) SHA1(1036067e2c85da867983e6e51ee2a7b5135000df))

	ROM_REGION(0x5000, "s6sound:audiocpu", ROMREGION_ERASEFF)
	ROM_LOAD("sound12.532",  0x4000, 0x1000, CRC(06051e5e) SHA1(f0ab4be812ceaf771829dd549f2a612156102a93))
ROM_END

ROM_START(hypbl_l2)
	ROM_REGION(0x3000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD("ic20-l2.532",  0x0000, 0x1000, CRC(f5f66cf1) SHA1(885b4961b6ec712b7445001d448d881245be1234))
	ROM_LOAD("ic14-l2.532",  0x1000, 0x1000, CRC(8eb82df4) SHA1(854b3f1fa2112fbdba19f4c843f67989c0572d8c))
	ROM_LOAD("ic17.532",     0x2000, 0x1000, CRC(6f4c0c4c) SHA1(1036067e2c85da867983e6e51ee2a7b5135000df))

	ROM_REGION(0x5000, "s6sound:audiocpu", ROMREGION_ERASEFF)
	ROM_LOAD("sound12.532",  0x4000, 0x1000, CRC(06051e5e) SHA1(f0ab4be812ceaf771829dd549f2a612156102a93))
ROM_END


/*----------------------------
/ Barracora (#510)
/----------------------------*/
ROM_START(barra_l1)
	ROM_REGION(0x3000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD("ic26.716",     0x0800, 0x0800, CRC(2a0e0171) SHA1(f1f2d4c1baed698d3b7cf2e88a2c28056e859920) )
	ROM_LOAD("ic14.716",     0x1000, 0x0800, CRC(522e944e) SHA1(0fa17b7912f8129e40de5fed8c3ccccc0a2a9366) )
	ROM_LOAD("ic20.716",     0x1800, 0x0800, CRC(dfb4b75a) SHA1(bcf017b01236f755cee419e398bbd8955ae3576a) )
	ROM_LOAD("ic17.532",     0x2000, 0x1000, CRC(bb571a17) SHA1(fb0b7f247673dae0744d4188e1a03749a2237165) )

	ROM_REGION(0x5000, "s6sound:audiocpu", ROMREGION_ERASEFF)
	ROM_LOAD("sound4.716",   0x4800, 0x0800, CRC(67ea12e7) SHA1(f81e97183442736d5766a7e5e074bc6539e8ced0))
ROM_END

/*----------------------------
/ Varkon (#512)
/----------------------------*/
ROM_START(vrkon_l1)
	ROM_REGION(0x3000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD("ic26.716",     0x0800, 0x0800, CRC(df20330c) SHA1(22157c6480ad38b9c53c390f5e7bfa63a8abd0e8) )
	ROM_LOAD("ic14.716",     0x1000, 0x0800, CRC(3baba324) SHA1(522654e0d81458d8b31150dcb0cb53c29b334358) )
	ROM_LOAD("ic20.716",     0x1800, 0x0800, CRC(dfb4b75a) SHA1(bcf017b01236f755cee419e398bbd8955ae3576a) )
	ROM_LOAD("ic17.532",     0x2000, 0x1000, CRC(bb571a17) SHA1(fb0b7f247673dae0744d4188e1a03749a2237165) )

	ROM_REGION(0x5000, "s6sound:audiocpu", ROMREGION_ERASEFF)
	ROM_LOAD("sound12.716",  0x4800, 0x0800, CRC(d13db2bb) SHA1(862546bbdd1476906948f7324b7434c29df79baa) )
ROM_END

/*-----------------------------
/ Time Fantasy (#515)
/-----------------------------*/
ROM_START(tmfnt_l5)
	ROM_REGION(0x3000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD("ic26.716",     0x0800, 0x0800, CRC(0f86947c) SHA1(e775f44b4ca5dae5ec2626fa84fae83c4f0c5c33) )
	ROM_LOAD("ic14.716",     0x1000, 0x0800, CRC(56b8e5ad) SHA1(84d6ab59032282cdccb3bdce0365c1fc766d0e5b) )
	ROM_LOAD("ic20.716",     0x1800, 0x0800, CRC(dfb4b75a) SHA1(bcf017b01236f755cee419e398bbd8955ae3576a) )
	ROM_LOAD("ic17.532",     0x2000, 0x1000, CRC(bb571a17) SHA1(fb0b7f247673dae0744d4188e1a03749a2237165) )

	ROM_REGION(0x5000, "s6sound:audiocpu", ROMREGION_ERASEFF)
	ROM_LOAD("sound3.716",   0x4800, 0x0800, CRC(55a10d13) SHA1(521d4cdfb0ed8178b3594cedceae93b772a951a4))
ROM_END

/*----------------------------
/ Warlok (#516)
/----------------------------*/
ROM_START(wrlok_l3)
	ROM_REGION(0x3000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD("ic26.716",     0x0800, 0x0800, CRC(44f8b507) SHA1(cdd8455c1e34584e8f1b75d430b8b37d4dd7dff0) )
	ROM_LOAD("ic14.716",     0x1000, 0x0800, CRC(291be241) SHA1(77fffa878f760583ef152a7939867621a61d58dc) )
	ROM_LOAD("ic20.716",     0x1800, 0x0800, CRC(dfb4b75a) SHA1(bcf017b01236f755cee419e398bbd8955ae3576a) )
	ROM_LOAD("ic17.532",     0x2000, 0x1000, CRC(bb571a17) SHA1(fb0b7f247673dae0744d4188e1a03749a2237165) )

	ROM_REGION(0x5000, "s6sound:audiocpu", ROMREGION_ERASEFF)
	ROM_LOAD("sound12.716",  0x4800, 0x0800, CRC(5d8e46d6) SHA1(68f8760ad85b8ada81f6ed00eadb9daf37191c53))
ROM_END

/*----------------------------
/ Defender (#517)
/----------------------------*/
// Multiplex solenoid requires custom solenoid handler.
ROM_START(dfndr_l4)
	ROM_REGION(0x3000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD("ic20.532",     0x0000, 0x1000, CRC(e99e64a2) SHA1(a6cde9cb771063778cae706c740b73ce9bce9aa5))
	ROM_LOAD("ic14.532",     0x1000, 0x1000, CRC(959ec419) SHA1(f400d3a1feba0e149d24f4e1a8d240fe900b3f0b))
	ROM_LOAD("ic17.532",     0x2000, 0x1000, CRC(bb571a17) SHA1(fb0b7f247673dae0744d4188e1a03749a2237165))

	ROM_REGION(0x5000, "s6sound:audiocpu", ROMREGION_ERASEFF)
	ROM_LOAD("sound12.716",  0x4800, 0x0800, CRC(cabaec58) SHA1(9605a1c299ed109a4ebcfa7ed6985ecc815c9e0c))
ROM_END

/*---------------------------
/ Joust (#519)
/--------------------------*/
ROM_START(jst_l2)
	ROM_REGION(0x3000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD("ic26.716",     0x0800, 0x0800, CRC(63eea5d8) SHA1(55c26ee94809f087bd886575a5e47efc93160190) )
	ROM_LOAD("ic14.716",     0x1000, 0x0800, CRC(c4cae4bf) SHA1(ff6e48364561402b16e40a41fa1b89e7723dd38a) )
	ROM_LOAD("ic20.716",     0x1800, 0x0800, CRC(dfb4b75a) SHA1(bcf017b01236f755cee419e398bbd8955ae3576a) )
	ROM_LOAD("ic17.532",     0x2000, 0x1000, CRC(bb571a17) SHA1(fb0b7f247673dae0744d4188e1a03749a2237165) )

	ROM_REGION(0x5000, "s6sound:audiocpu", ROMREGION_ERASEFF)
	ROM_LOAD("sound12.532",  0x4000, 0x1000, CRC(3bbc90bf) SHA1(82154e719ceca5c72d1ab034bc4ff5e3ebb36832))
ROM_END

ROM_START(jst_l1)
	ROM_REGION(0x3000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD("ic26-l1.716",  0x0800, 0x0800, CRC(123d8ffc) SHA1(c227a53653525269ea77203d4d1b14132058c073) )
	ROM_LOAD("ic14-l1.716",  0x1000, 0x0800, CRC(9871ebb2) SHA1(75c639a26d3bf7e05de7b5be063742f7448284ac) )
	ROM_LOAD("ic20.716",     0x1800, 0x0800, CRC(dfb4b75a) SHA1(bcf017b01236f755cee419e398bbd8955ae3576a) )
	ROM_LOAD("ic17.532",     0x2000, 0x1000, CRC(bb571a17) SHA1(fb0b7f247673dae0744d4188e1a03749a2237165) )

	ROM_REGION(0x5000, "s6sound:audiocpu", ROMREGION_ERASEFF)
	ROM_LOAD("sound12.532",  0x4000, 0x1000, CRC(3bbc90bf) SHA1(82154e719ceca5c72d1ab034bc4ff5e3ebb36832))
ROM_END

/*---------------------------
/ Laser Cue (#520)
/--------------------------*/
ROM_START(lsrcu_l2)
	ROM_REGION(0x3000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD("ic26.716",     0x0800, 0x0800, CRC(db4a09e7) SHA1(5ea454c852303e12cc606c2c1e403b72e0a99f25) )
	ROM_LOAD("ic14.716",     0x1000, 0x0800, CRC(39fc350d) SHA1(46e95f4016907c21c69472e6ef4a68a9adc3be77) )
	ROM_LOAD("ic20.716",     0x1800, 0x0800, CRC(dfb4b75a) SHA1(bcf017b01236f755cee419e398bbd8955ae3576a) )
	ROM_LOAD("ic17.532",     0x2000, 0x1000, CRC(bb571a17) SHA1(fb0b7f247673dae0744d4188e1a03749a2237165) )

	ROM_REGION(0x5000, "s6sound:audiocpu", ROMREGION_ERASEFF)
	ROM_LOAD("sound12.716",  0x4800, 0x0800, CRC(1888c635) SHA1(5dcdaee437a69c6027c24310f0cd2cae4e89fa05))
ROM_END

/*--------------------------------
/ Firepower II (#521)
/-------------------------------*/
ROM_START(fpwr2_l2)
	ROM_REGION(0x3000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD("ic26.716",     0x0800, 0x0800, CRC(1068939d) SHA1(f15c3a149bafee6d74e359399de88fd122b93441) )
	ROM_LOAD("ic14.716",     0x1000, 0x0800, CRC(a29688dd) SHA1(83815154bbaf51dd789112664d772a876efee3da) )
	ROM_LOAD("ic20.716",     0x1800, 0x0800, CRC(dfb4b75a) SHA1(bcf017b01236f755cee419e398bbd8955ae3576a) )
	ROM_LOAD("ic17.532",     0x2000, 0x1000, CRC(bb571a17) SHA1(fb0b7f247673dae0744d4188e1a03749a2237165) )

	ROM_REGION(0x5000, "s6sound:audiocpu", ROMREGION_ERASEFF)
	ROM_LOAD("sound3.716",   0x4800, 0x0800, CRC(55a10d13) SHA1(521d4cdfb0ed8178b3594cedceae93b772a951a4))
ROM_END

/*-------------------------------
/ Wild Texas (displays as #521L1)
/-------------------------------*/
// Conversion of Firepower II
// http://www.ipdb.org/machine.cgi?id=5500

ROM_START(wldtexas)
	ROM_REGION(0x3000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD("wldtexas.prg", 0x0000, 0x1000, CRC(243e7116) SHA1(c13c261632b3e8693a500d922f151296102e0169))
	ROM_CONTINUE(0x0000, 0x3000)

	ROM_REGION(0x5000, "s6sound:audiocpu", ROMREGION_ERASEFF)
	ROM_LOAD("sound3.716",   0x4800, 0x0800, CRC(55a10d13) SHA1(521d4cdfb0ed8178b3594cedceae93b772a951a4))
ROM_END

/*-----------------------------
/ Star Light (#530)
/-----------------------------*/
ROM_START(strlt_l1)
	ROM_REGION(0x3000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD("ic20.532",     0x0000, 0x1000, CRC(66876b56) SHA1(6fab43fbb67c7b602ca595c20a41fc1553afdb65))
	ROM_LOAD("ic14.532",     0x1000, 0x1000, CRC(292f1c4a) SHA1(0b5d50331364655672be16236d38d72b28f6dec2))
	ROM_LOAD("ic17.532",     0x2000, 0x1000, CRC(a43d8518) SHA1(fb2289bb7380838d0d817e78c39e5bcb2709373f))

	ROM_REGION(0x5000, "s6sound:audiocpu", ROMREGION_ERASEFF)
	ROM_LOAD("sound3.716",   0x4800, 0x0800, CRC(55a10d13) SHA1(521d4cdfb0ed8178b3594cedceae93b772a951a4))
ROM_END

} // Anonymous namespace


GAME( 1980, bk_l4,    0,        s7, bk,    s7_state, empty_init, ROT0, "Williams",  "Black Knight (L-4)",                MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME( 1980, bk_f4,    bk_l4,    s7, bk,    s7_state, empty_init, ROT0, "Williams",  "Black Knight (L-4, French speech)", MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME( 1980, bk_l3,    bk_l4,    s7, bk,    s7_state, empty_init, ROT0, "Williams",  "Black Knight (L-3)",                MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME( 1980, bk_l2,    bk_l4,    s7, bk,    s7_state, empty_init, ROT0, "Williams",  "Black Knight (L-2)",                MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME( 1980, csmic_l1, 0,        s7, csmic, s7_state, empty_init, ROT0, "Williams",  "Cosmic Gunfight (L-1)",             MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME( 1981, jngld_l2, 0,        s7, jngld, s7_state, empty_init, ROT0, "Williams",  "Jungle Lord (L-2)",                 MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME( 1981, jngld_l1, jngld_l2, s7, jngld, s7_state, empty_init, ROT0, "Williams",  "Jungle Lord (L-1)",                 MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME( 1981, pharo_l2, 0,        s7, pharo, s7_state, empty_init, ROT0, "Williams",  "Pharaoh (L-2)",                     MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME( 1981, solar_l2, 0,        s7, solar, s7_state, empty_init, ROT0, "Williams",  "Solar Fire (L-2)",                  MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME( 1982, thund_p1, 0,        s7, thund, s7_state, init_1,     ROT0, "Williams",  "Thunderball (P-1)",                 MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME( 1982, thund_p2, thund_p1, s7, thund, s7_state, init_1,     ROT0, "Williams",  "Thunderball (P-2)",                 MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME( 1982, thund_p3, thund_p1, s7, thund, s7_state, init_1,     ROT0, "Williams",  "Thunderball (P-3)",                 MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME( 1981, hypbl_l4, 0,        s7, hypbl, s7_state, empty_init, ROT0, "Williams",  "HyperBall (L-4)",                   MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME( 1981, hypbl_l3, hypbl_l4, s7, hypbl, s7_state, empty_init, ROT0, "Williams",  "HyperBall (L-3)",                   MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME( 1981, hypbl_l2, hypbl_l4, s7, hypbl, s7_state, empty_init, ROT0, "Williams",  "HyperBall (L-2)",                   MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME( 1981, barra_l1, 0,        s7, barra, s7_state, empty_init, ROT0, "Williams",  "Barracora (L-1)",                   MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME( 1982, vrkon_l1, 0,        s7, vrkon, s7_state, empty_init, ROT0, "Williams",  "Varkon (L-1)",                      MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME( 1982, tmfnt_l5, 0,        s7, tmfnt, s7_state, empty_init, ROT0, "Williams",  "Time Fantasy (L-5)",                MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME( 1982, wrlok_l3, 0,        s7, wrlok, s7_state, empty_init, ROT0, "Williams",  "Warlok (L-3)",                      MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME( 1982, dfndr_l4, 0,        s7, dfndr, s7_state, empty_init, ROT0, "Williams",  "Defender (L-4)",                    MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME( 1983, jst_l2,   0,        s7, jst,   s7_state, empty_init, ROT0, "Williams",  "Joust (L-2)",                       MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME( 1983, jst_l1,   jst_l2,   s7, jst,   s7_state, empty_init, ROT0, "Williams",  "Joust (L-1)",                       MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME( 1983, lsrcu_l2, 0,        s7, lsrcu, s7_state, empty_init, ROT0, "Williams",  "Laser Cue (L-2)",                   MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME( 1983, fpwr2_l2, 0,        s7, fpwr2, s7_state, empty_init, ROT0, "Williams",  "Firepower II (L-2)",                MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME( 1984, strlt_l1, 0,        s7, strlt, s7_state, empty_init, ROT0, "Williams",  "Star Light (L-1)",                  MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
// same hardware, unknown manufacturer, clone of fpwr2
GAME( 1983, wldtexas, fpwr2_l2, s7, fpwr2, s7_state, empty_init, ROT0, "<unknown>", "Wild Texas",                        MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
