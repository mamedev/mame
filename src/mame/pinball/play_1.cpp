// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic, Robbbert
/*********************************************************************************

PINBALL
Playmatic MPU 1

ToDo:
- Add remaining mechanical sounds
- Lamps, solenoids (spcgambl has a "cone" lighted by the COx outputs)


Status:
- All games are working
- When starting a game, hold X until you hear the outhole sound.
- All game except Chance: When starting a ball, hit Z to increment the ball number.
   Skipping this step can lead to weird bugs happening.

Todo:
- Outputs

**********************************************************************************/

#include "emu.h"
#include "genpin.h"

#include "cpu/cosmac/cosmac.h"
#include "machine/clock.h"
#include "sound/spkrdev.h"
#include "speaker.h"

#include "play_1.lh"


namespace {

class play_1_state : public genpin_class
{
public:
	play_1_state(const machine_config &mconfig, device_type type, const char *tag)
		: genpin_class(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_io_keyboard(*this, "X%d", 0U)
		, m_monotone(*this, "monotone")
		, m_digits(*this, "digit%d", 0U)
		, m_leds(*this, "led%d", 1U)
		, m_player_lamps(*this, "text%d", 1U)
		, m_io_outputs(*this, "out%d", 0U)
	{ }

	void chance(machine_config &config);
	void play_1(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	u8 port07_r();
	void port01_w(u8 data);
	void port02_w(u8 data);
	void port03_w(u8 data);
	void port04_w(u8 data);
	void port05_w(u8 data);
	void port06_w(u8 data);
	int clear_r();
	int wait_r();
	int ef2_r();
	int ef3_r();
	int ef4_r();
	void clock_w(int state);

	void chance_map(address_map &map) ATTR_COLD;
	void io_map(address_map &map) ATTR_COLD;
	void mem_map(address_map &map) ATTR_COLD;

	u8 m_resetcnt = 0U;
	u16 m_clockcnt = 0U;
	u16 m_waitcnt = 0U;
	u8 m_segment = 0U;
	u8 m_match = 0U;
	u8 m_ball = 0U;
	required_device<cosmac_device> m_maincpu;
	required_ioport_array<10> m_io_keyboard;
	required_device<clock_device> m_monotone;
	output_finder<46> m_digits;
	output_finder<5> m_leds;
	output_finder<4> m_player_lamps;
	output_finder<56> m_io_outputs;   // 8 solenoids + 48 lamps
};

void play_1_state::mem_map(address_map &map)
{
	map.global_mask(0xfff);
	map(0x0000, 0x07ff).rom().region("roms", 0);
	map(0x0800, 0x081f).ram().share("nvram"); // capacitor acting as a 2-month "battery"
	map(0x0c00, 0x0c1f).ram();
}

void play_1_state::chance_map(address_map &map)
{
	map.global_mask(0xfff);
	map(0x0000, 0x0bff).rom().region("roms", 0);
	map(0x0c00, 0x0c1f).ram();
	map(0x0e00, 0x0e1f).ram().share("nvram"); // capacitor acting as a 2-month "battery"
}

void play_1_state::io_map(address_map &map)
{
	map(0x01, 0x01).portr("X3").w(FUNC(play_1_state::port01_w)); //segments
	map(0x02, 0x02).portr("X4").w(FUNC(play_1_state::port02_w)); // N1-8
	map(0x03, 0x03).portr("X5").w(FUNC(play_1_state::port03_w)); // D1-4
	map(0x04, 0x04).portr("X6").w(FUNC(play_1_state::port04_w)); // U1-8
	map(0x05, 0x05).portr("X7").w(FUNC(play_1_state::port05_w)); // V1-8
	map(0x06, 0x06).portr("X8").w(FUNC(play_1_state::port06_w)); // W1-8
	map(0x07, 0x07).r(FUNC(play_1_state::port07_r));
}

static INPUT_PORTS_START( chance )
	PORT_START("X0")
	PORT_DIPNAME(0x01, 0x01, "Unknown" ) // Shows in schematic, not mentioned in the manuals, appears to have no effect
	PORT_DIPSETTING (  0x00, "3 games" )
	PORT_DIPSETTING (  0x01, "1 game" )
	PORT_DIPNAME(0x02, 0x00, "Balls")
	PORT_DIPSETTING (  0x00, "3" )
	PORT_DIPSETTING (  0x02, "5" )
	PORT_DIPNAME(0x04, 0x00, "Special award")
	PORT_DIPSETTING (  0x00, "Free game" )
	PORT_DIPSETTING (  0x04, "Extra ball" )

	PORT_START("X1")
	PORT_DIPNAME(0xff, 0x08, "Coinage for slot 2" )
	PORT_DIPSETTING (  0x01, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING (  0x02, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING (  0x04, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING (  0x08, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING (  0x10, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING (  0x20, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING (  0x40, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING (  0x80, DEF_STR( 1C_6C ) )

	PORT_START("X2")
	PORT_DIPNAME(0xff, 0x01, "Coinage for slot 3" )
	PORT_DIPSETTING (  0x01, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING (  0x02, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING (  0x04, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING (  0x08, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING (  0x10, DEF_STR( 1C_7C ) )
	PORT_DIPSETTING (  0x20, DEF_STR( 1C_8C ) )
	PORT_DIPSETTING (  0x40, DEF_STR( 1C_9C ) )
	PORT_DIPSETTING (  0x80, "1 coin 10 credits" )

	PORT_START("X3") // 11-18
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_X) PORT_NAME("Outhole") // outhole
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_9) PORT_NAME("Tilt") // Tilt
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_START )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("X4") // 21-28
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_A) PORT_NAME("INP21")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_B) PORT_NAME("INP22")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_C) PORT_NAME("INP23")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_D) PORT_NAME("INP26")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_E) PORT_NAME("INP27")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_F) PORT_NAME("INP28")

	PORT_START("X5") // 31-38
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_G) PORT_NAME("INP33")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_H) PORT_NAME("INP34")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_I) PORT_NAME("INP35")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_J) PORT_NAME("INP36")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_K) PORT_NAME("INP37")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_L) PORT_NAME("INP38")

	PORT_START("X6") // 41-48
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_M) PORT_NAME("INP43")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_N) PORT_NAME("INP44")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_O) PORT_NAME("INP45")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_P) PORT_NAME("INP46")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_Q) PORT_NAME("INP47")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_R) PORT_NAME("INP48")

	PORT_START("X7") // 51-58
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_S) PORT_NAME("INP51")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_T) PORT_NAME("INP52")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_U) PORT_NAME("INP53")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_V) PORT_NAME("INP54")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_W) PORT_NAME("INP55")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_Y) PORT_NAME("INP57")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_Z) PORT_NAME("INP58")

	PORT_START("X8") // 61-68
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_DEL_PAD) PORT_NAME("Show total replays") // Show total free replays
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_1_PAD) PORT_NAME("Show total pays") // Show total games paid
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_4_PAD) PORT_NAME("Show 1st chute") // Show 1st chute
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_5_PAD) PORT_NAME("Show 2nd chute") // Show 2nd shute
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_6_PAD) PORT_NAME("Show 3rd chute") // Show 3rd chute
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN3 )

	PORT_START("X9") // 71-78
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_0_PAD) PORT_NAME("Test") // Test
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_2_PAD) PORT_NAME("Reset Units 1-5") // Reset Unit 1,2,3,4,5
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_3_PAD) PORT_NAME("Show high score") // Show high score to date
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_7_PAD) PORT_NAME("Set 3rd replay") // Set 3rd replay score
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_8_PAD) PORT_NAME("Set 2nd replay") // Set 2nd replay score
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_9_PAD) PORT_NAME("Set 1st replay") // Set 1st replay score
INPUT_PORTS_END

static INPUT_PORTS_START( play_1 )
	PORT_INCLUDE( chance )
	PORT_MODIFY("X3") // 48-41
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_A) PORT_NAME("INP48")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_START ) // start
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_X) PORT_NAME("Outhole")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_B) PORT_NAME("INP45")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_9) PORT_NAME("Tilt")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_Z) PORT_NAME("Ball Enable") // trough
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_MODIFY("X4") // 38-31
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_C) PORT_NAME("INP38")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_D) PORT_NAME("INP37")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_E) PORT_NAME("INP36")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_F) PORT_NAME("INP35")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_G) PORT_NAME("INP34")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_H) PORT_NAME("INP33")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_I) PORT_NAME("INP32")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_J) PORT_NAME("INP31")

	PORT_MODIFY("X5") // 28-21
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_K) PORT_NAME("INP27")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_L) PORT_NAME("INP26")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_M) PORT_NAME("INP25")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_N) PORT_NAME("INP24")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_O) PORT_NAME("INP23")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_P) PORT_NAME("INP22")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_Q) PORT_NAME("INP21")

	PORT_MODIFY("X6") // 18-11
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_R) PORT_NAME("INP18")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_S) PORT_NAME("INP17")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_T) PORT_NAME("INP16")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_U) PORT_NAME("INP15")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_V) PORT_NAME("INP14")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_W) PORT_NAME("INP13")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_Y) PORT_NAME("INP12")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_COMMA) PORT_NAME("INP11")

	PORT_MODIFY("X7") // not exist
	PORT_BIT( 0xFF, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END

static INPUT_PORTS_START( spcgambl )
	PORT_INCLUDE( play_1 )
	PORT_MODIFY("X3") // 58-51
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_0) PORT_NAME("Slam Tilt") // Anti-cheat
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_START ) // start
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_A) PORT_NAME("INP56")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_B) PORT_NAME("INP55")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_9) PORT_NAME("Tilt")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_MODIFY("X4") // 48-41
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_X) PORT_NAME("Outhole")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_Z) PORT_NAME("Ball Enable") // trough
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_MODIFY("X5") // 38-31
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_C) PORT_NAME("INP38")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_D) PORT_NAME("INP37")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_E) PORT_NAME("INP36")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_F) PORT_NAME("INP35")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_G) PORT_NAME("INP34")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_H) PORT_NAME("INP33")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_I) PORT_NAME("INP32")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_J) PORT_NAME("INP31")

	PORT_MODIFY("X6") // 28-21
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_K) PORT_NAME("INP28")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_L) PORT_NAME("INP27")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_M) PORT_NAME("INP26")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_N) PORT_NAME("INP25")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_O) PORT_NAME("INP24")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_P) PORT_NAME("INP23")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_Q) PORT_NAME("INP22")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_R) PORT_NAME("INP21")

	PORT_MODIFY("X7") // 18-11
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_S) PORT_NAME("INP18")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_T) PORT_NAME("INP17")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_U) PORT_NAME("INP16")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_V) PORT_NAME("INP15")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_W) PORT_NAME("INP14")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_Y) PORT_NAME("INP13")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_COMMA) PORT_NAME("INP12")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END


void play_1_state::machine_start()
{
	genpin_class::machine_start();

	m_digits.resolve();
	m_leds.resolve();
	m_player_lamps.resolve();
	m_io_outputs.resolve();

	save_item(NAME(m_resetcnt));
	save_item(NAME(m_clockcnt));
	save_item(NAME(m_waitcnt));
	save_item(NAME(m_segment));
	save_item(NAME(m_match));
	save_item(NAME(m_ball));
}

void play_1_state::machine_reset()
{
	genpin_class::machine_reset();
	for (u8 i = 0; i < m_io_outputs.size(); i++)
		m_io_outputs[i] = 0;

	m_waitcnt = 0xffff;
	m_resetcnt = 0;
	m_clockcnt = 0;
	m_segment = 0;
	m_match = 0;
	m_ball = 0;
}

u8 play_1_state::port07_r()
{
	u8 data = m_io_keyboard[9]->read() & 0x3f;
	data |= (m_segment & m_io_keyboard[1]->read()) ? 0x40 : 0;
	data |= (m_segment & m_io_keyboard[2]->read()) ? 0x80 : 0;
	return data;
}

void play_1_state::port01_w(u8 data)
{
	static const u8 patterns[16] = { 0x3f, 0x06, 0x5b, 0x4f, 0x66, 0x6d, 0x7c, 0x07, 0x7f, 0x67, 0, 0, 0, 0, 0, 0 }; // 4511
	// d0-1 via 4013 to match-game board
	// d4-7 via 4511 to match-game board
	if (BIT(data, 0))
		m_digits[40] = patterns[1];
	else
		m_digits[40] = 0;

	if (BIT(data, 1))
	{
		m_digits[44] = patterns[0];
		m_digits[45] = patterns[0];
	}
	else
	{
		m_digits[44] = 0;
		m_digits[45] = 0;
	}

	m_match = patterns[data>>4];
	m_waitcnt = 0;
}

void play_1_state::port02_w(u8 data)
{
	// N1-8, segments and other
	m_segment = data;
	m_waitcnt = 0;
}

void play_1_state::port03_w(u8 data)
{
	static const u8 patterns[16] = { 0x3f, 0x06, 0x5b, 0x4f, 0x66, 0x6d, 0x7c, 0x07, 0x7f, 0x67, 0, 0, 0, 0, 0, 0 }; // 4511
	// D1-4, digit select
	switch (~data & 15)
	{
		case 0:
			// M1 thru M8: bit 4 lights up the bumpers, bit 6 outhole, bit 7 knocker
			if (BIT(m_segment, 6)) // outhole
				m_samples->start(0, 5);
			if (BIT(m_segment, 7)) // knocker
				m_samples->start(0, 6);
			for (u8 i = 0; i < 8; i++)
				m_io_outputs[i] = BIT(m_segment, i);
			break;
		case 1:
			// a combination of bits could set higher frequencies, but that isn't documented or used
			if (BIT(m_segment, 0))
				m_monotone->set_unscaled_clock(523);
			else
			if (BIT(m_segment, 1))
				m_monotone->set_unscaled_clock(659);
			else
			if (BIT(m_segment, 2))
				m_monotone->set_unscaled_clock(784);
			else
			if (BIT(m_segment, 3))
				m_monotone->set_unscaled_clock(988);
			else
			if ((m_segment & 0x0F)==0)
				m_monotone->set_unscaled_clock(0);
			// Bit 4 when low makes the sound fade out, not emulated yet

			// display player number
			{
				u8 player = m_segment >> 5;
				for (u8 i = 1; i < 5; i++)
					m_player_lamps[i - 1] = (player == i) ? 0 : 1;
			}
			break;
		case 2:
			m_digits[0] = patterns[m_segment>>4];
			m_digits[1] = patterns[m_segment&15];
			break;
		case 3:
			m_digits[2] = patterns[m_segment>>4];
			m_digits[3] = patterns[m_segment&15];
			break;
		case 4:
			m_digits[4] = patterns[m_segment>>4];
			m_digits[5] = patterns[m_segment&15];
			m_digits[14] = patterns[m_segment>>4];
			m_digits[15] = patterns[m_segment&15];
			m_digits[24] = patterns[m_segment>>4];
			m_digits[25] = patterns[m_segment&15];
			m_digits[34] = patterns[m_segment>>4];
			m_digits[35] = patterns[m_segment&15];
			break;
		case 5:
			m_digits[10] = patterns[m_segment>>4];
			m_digits[11] = patterns[m_segment&15];
			break;
		case 6:
			m_digits[12] = patterns[m_segment>>4];
			m_digits[13] = patterns[m_segment&15];
			break;
		case 7:
			m_digits[20] = patterns[m_segment>>4];
			m_digits[21] = patterns[m_segment&15];
			break;
		case 8:
			m_digits[22] = patterns[m_segment>>4];
			m_digits[23] = patterns[m_segment&15];
			break;
		case 9:
			m_digits[30] = patterns[m_segment>>4];
			m_digits[31] = patterns[m_segment&15];
			break;
		case 10:
		case 11:
			m_digits[32] = patterns[m_segment>>4];
			m_digits[33] = patterns[m_segment&15];
			break;
		default:
			break;
	}
	m_waitcnt = 0;
}

void play_1_state::port04_w(u8 data)
{
	// U1-8
	m_ball = data;
	m_waitcnt = 0;
	u8 t = BIT(m_clockcnt, 1) ? 8 : 16;
	for (u8 i = 0; i < 8; i++)
		m_io_outputs[t+i] = BIT(m_segment, i);
}

void play_1_state::port05_w(u8 data)
{
	// V1-8
	m_waitcnt = 0;
	u8 t = BIT(m_clockcnt, 1) ? 24 : 32;
	for (u8 i = 0; i < 8; i++)
		m_io_outputs[t+i] = BIT(m_segment, i);
}

void play_1_state::port06_w(u8 data)
{
	// W1-8
	m_waitcnt = 0;
	u8 t = BIT(m_clockcnt, 1) ? 40 : 48;
	for (u8 i = 0; i < 8; i++)
		m_io_outputs[t+i] = BIT(m_segment, i);
}

int play_1_state::clear_r()
{
	// A hack to make the machine reset itself on boot
	if (m_resetcnt < 0xff)
		m_resetcnt++;
	return (m_resetcnt < 0xf0) ? 0 : 1;
}

int play_1_state::wait_r()
{
	// Any OUT instruction forces a 60-100msec wait
	if (m_waitcnt < 0x180)
	{
		m_waitcnt++;
		return 0;
	}
	else
		return 1;
}

int play_1_state::ef2_r()
{
	return !BIT(m_io_keyboard[0]->read(), 0); // 1 or 3 games dip (1=1 game) inverted
}

int play_1_state::ef3_r()
{
	return !BIT(m_io_keyboard[0]->read(), 1); // 3 or 5 balls dip (1=5 balls) inverted
}

int play_1_state::ef4_r()
{
	return !BIT(m_io_keyboard[0]->read(), 2); // extra ball or game dip (1=extra ball) inverted
}

void play_1_state::clock_w(int state)
{
	if (state)
	{
		m_clockcnt++;
		m_maincpu->int_w(BIT(m_clockcnt, 0)); // inverted
		m_maincpu->ef1_w(BIT(m_clockcnt, 1)); // inverted
		if (BIT(m_clockcnt, 1))
			m_digits[41] = m_match;
		else
		{
			m_digits[43] = m_match;
			m_leds[0] = !BIT(m_ball, 1);
			m_leds[1] = !BIT(m_ball, 2);
			m_leds[2] = !BIT(m_ball, 3);
			m_leds[3] = !BIT(m_ball, 4);
			m_leds[4] = !BIT(m_ball, 5);
		}
	}
}

void play_1_state::play_1(machine_config &config)
{
	/* basic machine hardware */
	CDP1802(config, m_maincpu, 400000); // 2 gates, 1 cap, 1 resistor oscillating somewhere between 350 to 450 kHz
	m_maincpu->set_addrmap(AS_PROGRAM, &play_1_state::mem_map);
	m_maincpu->set_addrmap(AS_IO, &play_1_state::io_map);
	m_maincpu->wait_cb().set(FUNC(play_1_state::wait_r));
	m_maincpu->clear_cb().set(FUNC(play_1_state::clear_r));
	m_maincpu->ef2_cb().set(FUNC(play_1_state::ef2_r));
	m_maincpu->ef3_cb().set(FUNC(play_1_state::ef3_r));
	m_maincpu->ef4_cb().set(FUNC(play_1_state::ef4_r));

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	/* Video */
	config.set_default_layout(layout_play_1);

	clock_device &xpoint(CLOCK(config, "xpoint", 100)); // crossing-point detector
	xpoint.signal_handler().set(FUNC(play_1_state::clock_w));

	/* Sound */
	genpin_audio(config);
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, "speaker").add_route(ALL_OUTPUTS, "mono", 0.50);
	CLOCK(config, m_monotone, 0); // sound device
	m_monotone->signal_handler().set("speaker", FUNC(speaker_sound_device::level_w));
}

void play_1_state::chance(machine_config &config)
{
	play_1(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &play_1_state::chance_map);
}

/* PLAYMATIC MPU-1 ALTERNATE ROMS =======================================================================

This is a list of known alternate roms. Nothing has been tested.

Big Town
Probably overdumps
ROM_LOAD( "bgtowna2708.bin", 0x0000, 0x0800, CRC(cb3b9987) SHA1(a1337dd190c401034df2ebf16bb15c5eb6a26945) )
ROM_LOAD( "bgtownb2708.bin", 0x0000, 0x0800, CRC(e972b2bd) SHA1(80c13d7e3fb2b879c9fa44284ec4e44d58a2e29d) )

*/
/*-------------------------------------------------------------------
/ Space Gambler (03/78)
/-------------------------------------------------------------------*/
ROM_START(spcgambl)
	ROM_REGION(0x800, "roms", 0)
	ROM_LOAD("spcgamba.bin", 0x0000, 0x0400, CRC(3b6e5287) SHA1(4d2fae779bb4117a99a9311b96ab79799f40067b))
	ROM_LOAD("spcgambb.bin", 0x0400, 0x0400, CRC(5c61f25c) SHA1(44b2d74926bf5678146b6d2347b4147e8a29a660))
ROM_END

/*-------------------------------------------------------------------
/ Big Town  (04/78)
/-------------------------------------------------------------------*/
ROM_START(bigtown)
	ROM_REGION(0x800, "roms", 0)
	ROM_LOAD("bigtowna.bin", 0x0000, 0x0400, CRC(253f1b93) SHA1(7ff5267d0dfe6ae19ec6b0412902f4ce83f23ed1))
	ROM_LOAD("bigtownb.bin", 0x0400, 0x0400, CRC(5e2ba9c0) SHA1(abd285aa5702c7fb84257b4341f64ff83c1fc0ce))
ROM_END

/*-------------------------------------------------------------------
/ Last Lap (09/78)
/-------------------------------------------------------------------*/
ROM_START(lastlap)
	ROM_REGION(0x800, "roms", 0)
	ROM_LOAD("lastlapa.bin", 0x0000, 0x0400, CRC(253f1b93) SHA1(7ff5267d0dfe6ae19ec6b0412902f4ce83f23ed1))
	ROM_LOAD("lastlapb.bin", 0x0400, 0x0400, CRC(5e2ba9c0) SHA1(abd285aa5702c7fb84257b4341f64ff83c1fc0ce))
ROM_END

/*-------------------------------------------------------------------
/ Third World (??/78)
/-------------------------------------------------------------------*/
ROM_START(thrdwrld)
	ROM_REGION(0x800, "roms", 0)
	ROM_LOAD("3rdwrlda.bin", 0x0000, 0x0400, CRC(253f1b93) SHA1(7ff5267d0dfe6ae19ec6b0412902f4ce83f23ed1))
	ROM_LOAD("3rdwrldb.bin", 0x0400, 0x0400, CRC(5e2ba9c0) SHA1(abd285aa5702c7fb84257b4341f64ff83c1fc0ce))
ROM_END

/*-------------------------------------------------------------------
/ Chance (09/78)
/-------------------------------------------------------------------*/
ROM_START(chance)
	ROM_REGION(0xc00, "roms", 0)
	ROM_LOAD("chance_a.bin", 0x0000, 0x0400, CRC(3cd9d5a6) SHA1(c1d9488495a67198f7f60f70a889a9a3062c71d7))
	ROM_LOAD("chance_b.bin", 0x0400, 0x0400, CRC(a281b0f1) SHA1(1d2d26ce5f50294d5a95f688c82c3bdcec75de95))
	ROM_LOAD("chance_c.bin", 0x0800, 0x0200, CRC(369afee3) SHA1(7fa46c7f255a5ef21b0d1cc018056bc4889d68b8))
ROM_END

/*-------------------------------------------------------------------
/ Party  (05/79)
/-------------------------------------------------------------------*/
ROM_START(party)
	ROM_REGION(0x800, "roms", 0)
	ROM_LOAD("party_a.bin", 0x0000, 0x0400, CRC(253f1b93) SHA1(7ff5267d0dfe6ae19ec6b0412902f4ce83f23ed1))
	ROM_LOAD("party_b.bin", 0x0400, 0x0400, CRC(5e2ba9c0) SHA1(abd285aa5702c7fb84257b4341f64ff83c1fc0ce))
ROM_END

/*-------------------------------------------------------------------
/ Night Fever (??/79)
/-------------------------------------------------------------------*/
ROM_START(ngtfever)
	ROM_REGION(0x800, "roms", 0)
	ROM_LOAD("nfevera.bin", 0x0000, 0x0400, CRC(253f1b93) SHA1(7ff5267d0dfe6ae19ec6b0412902f4ce83f23ed1))
	ROM_LOAD("nfeverb.bin", 0x0400, 0x0400, CRC(5e2ba9c0) SHA1(abd285aa5702c7fb84257b4341f64ff83c1fc0ce))
ROM_END

} // anonymous namespace


/* Big Town, Last Lap, Night Fever, Party and Third World all reportedly share the same roms with different playfield/machine artworks */
GAME(1978, bigtown,  0,       play_1, play_1,   play_1_state, empty_init, ROT0, "Playmatic", "Big Town",      MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME(1978, lastlap,  bigtown, play_1, play_1,   play_1_state, empty_init, ROT0, "Playmatic", "Last Lap",      MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME(1979, party,    bigtown, play_1, play_1,   play_1_state, empty_init, ROT0, "Playmatic", "Party",         MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME(1979, ngtfever, bigtown, play_1, play_1,   play_1_state, empty_init, ROT0, "Sonic",     "Night Fever",   MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME(1978, thrdwrld, bigtown, play_1, play_1,   play_1_state, empty_init, ROT0, "Sonic",     "Third World",   MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME(1978, spcgambl, 0,       play_1, spcgambl, play_1_state, empty_init, ROT0, "Playmatic", "Space Gambler", MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME(1978, chance,   0,       chance, chance,   play_1_state, empty_init, ROT0, "Playmatic", "Chance",        MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
