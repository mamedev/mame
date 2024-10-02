// license:BSD-3-Clause
// copyright-holders:Robbbert
/********************************************************************************************

PINBALL
Stern MP-200 MPU
(almost identical to Bally MPU-35)


Status:
- All games are playable, but see the key code list below.
- All games have missing or wrong sound
- cue uses first player display for all players, one at a time (this is by design)
- flight2k, gamatron: take a while to finish booting if balls are missing
- st_sam: This is a test unit. Internal tests are working.
- gamatron is actually a Bally conversion - it won't physically fit into a Stern machine.
- drgnfist, cue, nineball, lightnin, spltsecp, catacomb, lazrlord: need a new layout to
  handle different credit/ball display (shows 17 at boot).
- Speech mostly doesn't work.

SAM IV tester
- No schematic or manuals have been found
- Unknown how to use it
- Unknown what the PIAs connect to
- 5x 6-digit displays
- The LED flashes a few times followed by nothing

Here are the key codes to enable play:

Game              NUM  Start game                End ball
-----------------------------------------------------------------------------------------------
Nine Ball         125  X'; hit 1                 X';
Lightning         126  X./ hit 1                 X./
Flight 2000       128  X./ hit 1                 X./
Freefall          134  X./ hit 1                 X./
Split Second      144  X./ hit 1                 X./
Catacomb          147  1                         unknown
Viper             148  X./ hit 1                 X./
Iron Maiden       151  X./ hit 1                 X./
Orbitor 1         165  X. hit 1                  X. (last ball is difficult to end)
Lazer Lord        ---  1                         unknown
Gamatron          ---  X./ hit 1                 X./
Others            ---  1                         X

ToDo:
- Sound - All machines have a B605/C605 sound card containing a 6840 and many other chips
- Sound - Games 126,128-151,165 have a A720 voice synthesizer with a 'CRC' CPU and many other chips
- Dips, Inputs, Solenoids vary per game
- Mechanical sounds

*********************************************************************************************/


#include "emu.h"
#include "genpin.h"

#include "cpu/m6800/m6800.h"
#include "machine/6821pia.h"
#include "machine/clock.h"
#include "machine/input_merger.h"
#include "sound/s14001a.h"
#include "speaker.h"

#include "st_mp200.lh"


namespace{

#define S14001_CLOCK                (25e5)

class st_mp200_state : public genpin_class
{
public:
	st_mp200_state(const machine_config &mconfig, device_type type, const char *tag)
		: genpin_class(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_s14001a(*this, "speech")
		, m_pia_u10(*this, "pia_u10")
		, m_pia_u11(*this, "pia_u11")
		, m_io_test(*this, "TEST")
		, m_io_dsw0(*this, "DSW0")
		, m_io_dsw1(*this, "DSW1")
		, m_io_dsw2(*this, "DSW2")
		, m_io_dsw3(*this, "DSW3")
		, m_io_x0(*this, "X0")
		, m_io_x1(*this, "X1")
		, m_io_x2(*this, "X2")
		, m_io_x3(*this, "X3")
		, m_io_x4(*this, "X4")
		, m_digits(*this, "digit%d", 0U)
		, m_io_leds(*this, "led%d", 0U)
		, m_io_outputs(*this, "out%d", 0U)
	{ }

	void st_mp201(machine_config &config);
	void st_mp200(machine_config &config);
	void st_sam4(machine_config &config);

	void init_st_mp200();
	void init_st_mp201();
	void init_st_mp202();

	DECLARE_INPUT_CHANGED_MEMBER(activity_test);
	DECLARE_INPUT_CHANGED_MEMBER(self_test);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	u8 u10_a_r();
	void u10_a_w(u8 data);
	u8 u10_b_r();
	void u10_b_w(u8 data);
	u8 u11_a_r();
	void u11_a_w(u8 data);
	void u11_b_w(u8 data);
	void u10_ca2_w(int state);
	void u10_cb2_w(int state);
	void u11_ca2_w(int state);
	void u11_cb2_w(int state);

	void mem_map(address_map &map) ATTR_COLD;
	void sam4_map(address_map &map) ATTR_COLD;

	u8 m_u10a = 0U;
	u8 m_u10b = 0U;
	u8 m_u11a = 0U;
	u8 m_u11b = 0U;
	bool m_u10_ca2 = false;
	bool m_u10_cb2 = false;
	bool m_u11_cb2 = false;
	bool m_7d = false; // 7-digit display yes/no
	u8 m_stored_lamp = 0xffU;
	u8 m_digit = 0U;
	u8 m_counter = 0U;
	u8 m_segment[5]{};
	u8 m_last_solenoid = 31U;
	required_device<m6800_cpu_device> m_maincpu;
	optional_device<s14001a_device> m_s14001a;
	required_device<pia6821_device> m_pia_u10;
	required_device<pia6821_device> m_pia_u11;
	required_ioport m_io_test;
	required_ioport m_io_dsw0;
	required_ioport m_io_dsw1;
	required_ioport m_io_dsw2;
	required_ioport m_io_dsw3;
	required_ioport m_io_x0;
	required_ioport m_io_x1;
	required_ioport m_io_x2;
	required_ioport m_io_x3;
	required_ioport m_io_x4;
	output_finder<47> m_digits;
	output_finder<1> m_io_leds;
	output_finder<96> m_io_outputs;   // 32 solenoids + 64 lamps
};


void st_mp200_state::mem_map(address_map &map)
{
	map(0x0000, 0x007f).ram(); // internal to the cpu
	map(0x0088, 0x008b).rw(m_pia_u10, FUNC(pia6821_device::read), FUNC(pia6821_device::write));
	map(0x0090, 0x0093).rw(m_pia_u11, FUNC(pia6821_device::read), FUNC(pia6821_device::write));
	map(0x00a0, 0x00a7).nopw(); // to sound board
	map(0x00c0, 0x00c7).nopw(); // to sound board
	map(0x0200, 0x02ff).ram().share("nvram");
	map(0x1000, 0x1fff).rom();
	map(0x5000, 0x5fff).rom();
	map(0xf800, 0xffff).rom().region("maincpu", 0x5800);  // vectors
}

void st_mp200_state::sam4_map(address_map &map)
{
	map.global_mask(0x3fff);
	map(0x0000, 0x007f).ram(); // internal to the cpu
	map(0x0088, 0x008b).rw(m_pia_u10, FUNC(pia6821_device::read), FUNC(pia6821_device::write));
	map(0x0090, 0x0093).rw(m_pia_u11, FUNC(pia6821_device::read), FUNC(pia6821_device::write));
	map(0x2000, 0x207f).ram();
	map(0x2104, 0x2107).rw("sam4_pia0", FUNC(pia6821_device::read), FUNC(pia6821_device::write));
	map(0x2108, 0x210b).rw("sam4_pia1", FUNC(pia6821_device::read), FUNC(pia6821_device::write));
	map(0x2110, 0x2113).rw("sam4_pia2", FUNC(pia6821_device::read), FUNC(pia6821_device::write));
	map(0x2120, 0x2123).rw("sam4_pia3", FUNC(pia6821_device::read), FUNC(pia6821_device::write));
	map(0x2140, 0x2143).rw("sam4_pia4", FUNC(pia6821_device::read), FUNC(pia6821_device::write));
	map(0x3000, 0x3fff).rom().region("maincpu", 0);
}

static INPUT_PORTS_START( mp200 )
	PORT_START("TEST")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_0_PAD) PORT_NAME("Self Test") PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, st_mp200_state, self_test, 0)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_1_PAD) PORT_NAME("Activity") PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, st_mp200_state, activity_test, 0)

	PORT_START("DSW0")
	PORT_DIPNAME( 0x1f, 0x02, "Coin Slot 2")
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_3C )) // same as 01
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_1C ))
	PORT_DIPSETTING(    0x03, DEF_STR( 2C_1C ))
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_2C ))
	PORT_DIPSETTING(    0x05, DEF_STR( 2C_2C ))
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_3C ))
	PORT_DIPSETTING(    0x07, DEF_STR( 2C_3C ))
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_4C ))
	PORT_DIPSETTING(    0x09, DEF_STR( 2C_4C ))
	PORT_DIPSETTING(    0x0a, DEF_STR( 1C_5C ))
	PORT_DIPSETTING(    0x0b, DEF_STR( 2C_5C ))
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_6C ))
	PORT_DIPSETTING(    0x0d, DEF_STR( 2C_6C ))
	PORT_DIPSETTING(    0x0e, DEF_STR( 1C_7C ))
	PORT_DIPSETTING(    0x0f, DEF_STR( 2C_7C ))
	PORT_DIPSETTING(    0x10, DEF_STR( 1C_8C ))
	PORT_DIPSETTING(    0x11, DEF_STR( 2C_8C ))
	PORT_DIPSETTING(    0x12, DEF_STR( 1C_9C ))
	PORT_DIPSETTING(    0x13, "2 coins 9 credits")
	PORT_DIPSETTING(    0x14, "1 coin 10 credits")
	PORT_DIPSETTING(    0x15, "2 coins 10 credits")
	PORT_DIPSETTING(    0x16, "1 coin 11 credits")
	PORT_DIPSETTING(    0x17, "2 coins 11 credits")
	PORT_DIPSETTING(    0x18, "1 coin 12 credits")
	PORT_DIPSETTING(    0x19, "2 coins 12 credits")
	PORT_DIPSETTING(    0x1a, "1 coin 13 credits")
	PORT_DIPSETTING(    0x1b, "2 coins 13 credits")
	PORT_DIPSETTING(    0x1c, "1 coin 14 credits")
	PORT_DIPSETTING(    0x1d, "2 coins 14 credits")
	PORT_DIPSETTING(    0x1e, "1 coin 15 credits")
	PORT_DIPSETTING(    0x1f, "2 coins 15 credits")
	PORT_DIPNAME( 0x20, 0x20, "Award")
	PORT_DIPSETTING(    0x00, "Extra Ball")
	PORT_DIPSETTING(    0x20, "Free Game")
	PORT_DIPNAME( 0x40, 0x00, "Balls")
	PORT_DIPSETTING(    0x00, "3")
	PORT_DIPSETTING(    0x40, "5")
	PORT_DIPNAME( 0x80, 0x80, "Play melody always")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x80, DEF_STR( On ))

	PORT_START("DSW1")
	PORT_DIPNAME( 0x1f, 0x02, "Coin Slot 3")
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_3C )) // same as 01
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_1C ))
	PORT_DIPSETTING(    0x03, DEF_STR( 2C_1C ))
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_2C ))
	PORT_DIPSETTING(    0x05, DEF_STR( 2C_2C ))
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_3C ))
	PORT_DIPSETTING(    0x07, DEF_STR( 2C_3C ))
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_4C ))
	PORT_DIPSETTING(    0x09, DEF_STR( 2C_4C ))
	PORT_DIPSETTING(    0x0a, DEF_STR( 1C_5C ))
	PORT_DIPSETTING(    0x0b, DEF_STR( 2C_5C ))
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_6C ))
	PORT_DIPSETTING(    0x0d, DEF_STR( 2C_6C ))
	PORT_DIPSETTING(    0x0e, DEF_STR( 1C_7C ))
	PORT_DIPSETTING(    0x0f, DEF_STR( 2C_7C ))
	PORT_DIPSETTING(    0x10, DEF_STR( 1C_8C ))
	PORT_DIPSETTING(    0x11, DEF_STR( 2C_8C ))
	PORT_DIPSETTING(    0x12, DEF_STR( 1C_9C ))
	PORT_DIPSETTING(    0x13, "2 coins 9 credits")
	PORT_DIPSETTING(    0x14, "1 coin 10 credits")
	PORT_DIPSETTING(    0x15, "2 coins 10 credits")
	PORT_DIPSETTING(    0x16, "1 coin 11 credits")
	PORT_DIPSETTING(    0x17, "2 coins 11 credits")
	PORT_DIPSETTING(    0x18, "1 coin 12 credits")
	PORT_DIPSETTING(    0x19, "2 coins 12 credits")
	PORT_DIPSETTING(    0x1a, "1 coin 13 credits")
	PORT_DIPSETTING(    0x1b, "2 coins 13 credits")
	PORT_DIPSETTING(    0x1c, "1 coin 14 credits")
	PORT_DIPSETTING(    0x1d, "2 coins 14 credits")
	PORT_DIPSETTING(    0x1e, "1 coin 15 credits")
	PORT_DIPSETTING(    0x1f, "2 coins 15 credits")
	PORT_DIPNAME( 0x20, 0x00, "S14")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x20, DEF_STR( On ))
	PORT_DIPNAME( 0x40, 0x00, "Award for beating high score")
	PORT_DIPSETTING(    0x00, "Novelty")
	PORT_DIPSETTING(    0x40, "3 Free Games")
	PORT_DIPNAME( 0x80, 0x00, "Rollover lights")
	PORT_DIPSETTING(    0x00, "Always on")
	PORT_DIPSETTING(    0x80, "Alternate")

	PORT_START("DSW2")
	PORT_DIPNAME( 0x01, 0x01, "Talking feature")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x01, DEF_STR( On ))
	PORT_DIPNAME( 0x06, 0x02, "Maximum Credits")
	PORT_DIPSETTING(    0x00, "10")
	PORT_DIPSETTING(    0x02, "15")
	PORT_DIPSETTING(    0x04, "25")
	PORT_DIPSETTING(    0x06, "40")
	PORT_DIPNAME( 0x08, 0x08, "Credits displayed")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x08, DEF_STR( On ))
	PORT_DIPNAME( 0x10, 0x10, "Match")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x10, DEF_STR( On ))
	PORT_DIPNAME( 0x20, 0x00, "S22")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x20, DEF_STR( On ))
	PORT_DIPNAME( 0x40, 0x00, "S23")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x40, DEF_STR( On ))
	PORT_DIPNAME( 0x80, 0x00, "Award a free game for hitting all targets 2nd time")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x80, DEF_STR( On ))

	PORT_START("DSW3")
	PORT_DIPNAME( 0x01, 0x00, "S25")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x01, DEF_STR( On ))
	PORT_DIPNAME( 0x02, 0x00, "S26")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x02, DEF_STR( On ))
	PORT_DIPNAME( 0x04, 0x04, "S27")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x04, DEF_STR( On ))
	PORT_DIPNAME( 0x08, 0x00, "S28")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x08, DEF_STR( On ))
	PORT_DIPNAME( 0x10, 0x00, "S29")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x10, DEF_STR( On ))
	PORT_DIPNAME( 0x20, 0x00, "S30")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x20, DEF_STR( On ))
	PORT_DIPNAME( 0xc0, 0x80, "Award for Special")
	PORT_DIPSETTING(    0x00, "100000 points")
	PORT_DIPSETTING(    0x40, "Extra Ball")
	PORT_DIPSETTING(    0x80, "Free Game")
	PORT_DIPSETTING(    0xc0, "Extra Ball and Free Game")

	PORT_START("X0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN3 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_A) PORT_NAME("INP04")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_B) PORT_NAME("INP05")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_9) PORT_NAME("Tilt")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_0) PORT_NAME("Slam Tilt")

	PORT_START("X1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_C) PORT_NAME("INP09")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_D) PORT_NAME("INP10")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_E) PORT_NAME("INP11")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_F) PORT_NAME("INP12")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_G) PORT_NAME("INP13")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_H) PORT_NAME("INP14")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_I) PORT_NAME("INP15")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_J) PORT_NAME("INP16")

	PORT_START("X2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_K) PORT_NAME("INP17")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_L) PORT_NAME("INP18")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_M) PORT_NAME("INP19")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_N) PORT_NAME("INP20")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_O) PORT_NAME("INP21")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_P) PORT_NAME("INP22")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_Q) PORT_NAME("INP23")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_R) PORT_NAME("INP24")

	PORT_START("X3")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_S) PORT_NAME("INP25")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_T) PORT_NAME("INP26")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_U) PORT_NAME("INP27")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_V) PORT_NAME("INP28")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_W) PORT_NAME("INP29")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_Y) PORT_NAME("INP30")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_Z) PORT_NAME("INP31")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_COMMA) PORT_NAME("INP32")

	PORT_START("X4")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_X) PORT_NAME("Outhole")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_STOP) PORT_NAME("INP34")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_SLASH) PORT_NAME("INP35")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_COLON) PORT_NAME("INP36")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_QUOTE) PORT_NAME("INP37")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_ENTER) PORT_NAME("INP38")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_OPENBRACE) PORT_NAME("INP39")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_CLOSEBRACE) PORT_NAME("INP40")
INPUT_PORTS_END

INPUT_CHANGED_MEMBER( st_mp200_state::activity_test )
{
	if(newval)
		m_maincpu->pulse_input_line(INPUT_LINE_NMI, attotime::zero);
}

INPUT_CHANGED_MEMBER( st_mp200_state::self_test )
{
	m_pia_u10->ca1_w(newval);
}

void st_mp200_state::u10_ca2_w(int state)
{
	m_u10_ca2 = state;
	if (!state)
		m_counter = 0;
}

void st_mp200_state::u10_cb2_w(int state)
{
	m_u10_cb2 = state;
	if (state)
		m_stored_lamp = m_u10a;

	if (m_s14001a)
	{
		if (m_s14001a->busy_r())
			m_pia_u11->cb1_w(0);
		else
			m_pia_u11->cb1_w(state);
	}
}

void st_mp200_state::u11_ca2_w(int state)
{
	m_io_leds[0] = state ? 0 : 1;

	if (m_s14001a && state)
	{
		if (BIT(m_u10a, 7))
		{
			m_s14001a->data_w(m_u10a & 0x3f);
			m_s14001a->start_w(1);
			m_s14001a->start_w(0);
		}
		else if (BIT(m_u10a, 6))
		{
			m_s14001a->set_output_gain(0, ((m_u10a >> 3 & 0xf) + 1) / 16.0);

			u8 clock_divisor = 16 - (m_u10a & 0x07);
			m_s14001a->set_unscaled_clock(S14001_CLOCK / clock_divisor / 8);
		}
	}
}

void st_mp200_state::u11_cb2_w(int state)
{
	m_u11_cb2 = state;
}

u8 st_mp200_state::u10_a_r()
{
	return m_u10a;
}

void st_mp200_state::u10_a_w(u8 data)
{
	m_u10a = data;

	if (!m_u10_ca2)
	{
		m_counter++;

		if (m_counter==1)
			m_segment[0] = data>>4;
		else
		if (m_counter==3)
			m_segment[1] = data>>4;
		else
		if (m_counter==5)
			m_segment[2] = data>>4;
		else
		if (m_counter==7)
			m_segment[3] = data>>4;
		else
		if (m_counter==9)
			m_segment[4] = data>>4;
	}
}

u8 st_mp200_state::u10_b_r()
{
	u8 data = 0;

	if (BIT(m_u10a, 0))
		data |= m_io_x0->read();

	if (BIT(m_u10a, 1))
		data |= m_io_x1->read();

	if (BIT(m_u10a, 2))
		data |= m_io_x2->read();

	if (BIT(m_u10a, 3))
		data |= m_io_x3->read();

	if (BIT(m_u10a, 4))
		data |= m_io_x4->read();

	if (BIT(m_u10a, 5))
		data |= m_io_dsw0->read();

	if (BIT(m_u10a, 6))
		data |= m_io_dsw1->read();

	if (BIT(m_u10a, 7))
		data |= m_io_dsw2->read();

	if (m_u10_cb2)
		data |= m_io_dsw3->read();

	return data;
}

void st_mp200_state::u10_b_w(u8 data)
{
	m_u10b = data;
}

u8 st_mp200_state::u11_a_r()
{
	return m_u11a;
}

void st_mp200_state::u11_a_w(u8 data)
{
	m_u11a = data;

	// Displays
	if (!m_u10_ca2)
	{
		if (m_7d && BIT(data, 1))
			m_digit = 6;
		else if (BIT(data, 2))
			m_digit = 5;
		else if (BIT(data, 3))
			m_digit = 4;
		else if (BIT(data, 4))
			m_digit = 3;
		else if (BIT(data, 5))
			m_digit = 2;
		else if (BIT(data, 6))
			m_digit = 1;
		else if (BIT(data, 7))
			m_digit = 0;

		if (BIT(data, 0) && (m_counter > 8))
		{
			static const u8 patterns[16] = { 0x3f,0x06,0x5b,0x4f,0x66,0x6d,0x7d,0x07,0x7f,0x6f,0,0,0,0,0,0 }; // MC14543
			m_digits[m_digit] = patterns[m_segment[0]];
			m_digits[10+m_digit] = patterns[m_segment[1]];
			m_digits[20+m_digit] = patterns[m_segment[2]];
			m_digits[30+m_digit] = patterns[m_segment[3]];
			m_digits[40+m_digit] = patterns[m_segment[4]];
		}
	}

	// Lamps
	if ((m_stored_lamp >= 0xf0) && (m_stored_lamp < 0xff))
	{
		m_stored_lamp &= 15;  // extract lamp address
		for (u8 i = 0; i < 4; i++)
			m_io_outputs[i*16+m_stored_lamp+32] = BIT(~data, i+4);  // get strobe

		m_stored_lamp = 0xff;
	}
}

void st_mp200_state::u11_b_w(u8 data)
{
	// Solenoids
	m_io_outputs[m_last_solenoid] = 0;
	m_u11b = data;
	data = (data & 15) | (m_u11_cb2 ? 0x10 : 0);
	// Solenoids are different per game, no point allocating anything specific
	m_io_outputs[data] = 1;
	m_last_solenoid = data;
}

void st_mp200_state::machine_start()
{
	genpin_class::machine_start();
	m_digits.resolve();
	m_io_leds.resolve();
	m_io_outputs.resolve();

	save_item(NAME(m_u10a));
	save_item(NAME(m_u10b));
	save_item(NAME(m_u11a));
	save_item(NAME(m_u11b));
	save_item(NAME(m_u10_ca2));
	save_item(NAME(m_u10_cb2));
	save_item(NAME(m_u11_cb2));
	save_item(NAME(m_counter));
	save_item(NAME(m_digit));
	save_item(NAME(m_segment));
	save_item(NAME(m_last_solenoid));
	save_item(NAME(m_stored_lamp));
	save_item(NAME(m_7d));
}

void st_mp200_state::machine_reset()
{
	genpin_class::machine_reset();

	m_u10a = 0;
	m_u10b = 0;
	m_u10_cb2 = 0;
	m_u11a = 0;
	m_u11b = 0;
	m_last_solenoid = 31;
	m_stored_lamp = 0xff;
	for (u8 i = 0; i < m_io_outputs.size(); i++)
		m_io_outputs[i] = 0;
	for (u8 i = 0; i < std::size(m_segment); i++)
		m_segment[i] = 0;
}

void st_mp200_state::init_st_mp200()
{
	m_7d = 1;
}

void st_mp200_state::init_st_mp201()
{
	m_7d = 1;
}

void st_mp200_state::init_st_mp202()
{
	m_7d = 0;
}

void st_mp200_state::st_mp200(machine_config &config)
{
	/* basic machine hardware */
	M6800(config, m_maincpu, 1000000); // no xtal, just 2 chips forming a random oscillator
	m_maincpu->set_addrmap(AS_PROGRAM, &st_mp200_state::mem_map);

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	/* Video */
	config.set_default_layout(layout_st_mp200);

	/* Sound */
	genpin_audio(config);

	/* Devices */
	PIA6821(config, m_pia_u10);
	m_pia_u10->readpa_handler().set(FUNC(st_mp200_state::u10_a_r));
	m_pia_u10->writepa_handler().set(FUNC(st_mp200_state::u10_a_w));
	m_pia_u10->readpb_handler().set(FUNC(st_mp200_state::u10_b_r));
	m_pia_u10->writepb_handler().set(FUNC(st_mp200_state::u10_b_w));
	m_pia_u10->ca2_handler().set(FUNC(st_mp200_state::u10_ca2_w));
	m_pia_u10->cb2_handler().set(FUNC(st_mp200_state::u10_cb2_w));
	m_pia_u10->irqa_handler().set("irq", FUNC(input_merger_device::in_w<0>));
	m_pia_u10->irqb_handler().set("irq", FUNC(input_merger_device::in_w<1>));

	clock_device &u10_clock(CLOCK(config, "u10_clock", 120)); // crosspoint detector
	u10_clock.signal_handler().set(m_pia_u10, FUNC(pia6821_device::cb1_w));

	PIA6821(config, m_pia_u11);
	m_pia_u11->readpa_handler().set(FUNC(st_mp200_state::u11_a_r));
	m_pia_u11->writepa_handler().set(FUNC(st_mp200_state::u11_a_w));
	m_pia_u11->writepb_handler().set(FUNC(st_mp200_state::u11_b_w));
	m_pia_u11->ca2_handler().set(FUNC(st_mp200_state::u11_ca2_w));
	m_pia_u11->cb2_handler().set(FUNC(st_mp200_state::u11_cb2_w));
	m_pia_u11->irqa_handler().set("irq", FUNC(input_merger_device::in_w<2>));
	m_pia_u11->irqb_handler().set("irq", FUNC(input_merger_device::in_w<3>));

	clock_device &u11_clock(CLOCK(config, "u11_clock", 634));  // NE555 astable
	u11_clock.signal_handler().set(m_pia_u11, FUNC(pia6821_device::ca1_w));

	INPUT_MERGER_ANY_HIGH(config, "irq").output_handler().set_inputline(m_maincpu, M6800_IRQ_LINE);
}

void st_mp200_state::st_mp201(machine_config &config)
{
	st_mp200(config);
	SPEAKER(config, "mono").front_center();
	S14001A(config, m_s14001a, S14001_CLOCK).add_route(ALL_OUTPUTS, "mono", 1.00);
}

void st_mp200_state::st_sam4(machine_config &config)
{
	st_mp200(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &st_mp200_state::sam4_map);
	config.device_remove("nvram");
	PIA6821(config, "sam4_pia0");
	PIA6821(config, "sam4_pia1");
	PIA6821(config, "sam4_pia2");
	pia6821_device &pia3(PIA6821(config, "sam4_pia3"));
	pia3.writepa_handler().set_nop();
	pia3.writepb_handler().set_nop();
	PIA6821(config, "sam4_pia4");
}

/* ========== ALTERNATE ROMS =======================================================================

This is a list of known alternate roms. Nothing has been tested. They are modified for free play unless otherwise noted.

Ali
ROM_LOAD( "fpali_u1.716", 0x1000, 0x0800, CRC(85888cfa) SHA1(286bce3a2e1cbedc224c10d884f5db8b7ffa2a75) )
ROM_LOAD( "fpali_u5.716", 0x1800, 0x0800, CRC(52b8e39e) SHA1(267921c2a5636d1f2cc93f0913f005a3e50801a4) )

Big Game
ROM_LOAD( "fpbg_u2.716",  0x5000, 0x0800, CRC(e74dc6fe) SHA1(37d178d236f921ea4d95bed42f7683e428ca2da9) )

Catacomb
ROM_LOAD( "fpcat_u5.716", 0x1800, 0x0800, CRC(3114512f) SHA1(660286e223ad87ddeefdcc7887cdb77bd7a42cee) )
ROM_LOAD( "snd_u9.716",   0x0000, 0x0800, CRC(90877c99) SHA1(daaecbfadadf563689c49beaa15f4cf3900f079d) ) // catacomba
ROM_LOAD( "snd_u10.716",  0x0800, 0x0800, CRC(2b31f8be) SHA1(05b394bd8b6c04e34fe2bab19cbd0f06d9e4b90d) ) // catacomba
ROM_LOAD( "cpu_u6a.716",  0x0000, 0x0800, CRC(9f2ca810) SHA1(4de16d9ed1a5bef74d9450a54f8441343b0cb66f) ) // catacomba
ROM_LOAD( "cpu_u5a.716",  0x1800, 0x0800, CRC(2e6c5e8a) SHA1(0c97cae873e615569d0eddaf8273c46e3c796a36) ) // catacomba
ROM_LOAD( "cpu_u2a.716",  0x5000, 0x0800, CRC(a7e326c5) SHA1(3a8afa7c9fd5ad66d3250b456c9f3ed959d8a5e9) ) // catacomba
ROM_LOAD( "cpu_u1a.716",  0x1000, 0x0800, CRC(bf674561) SHA1(e66fdf9f8a0d5f51cb45550f46f936f4377e34da) ) // catacomba

Cheetah
ROM_LOAD( "cpu_u6.716",   0x5800, 0x0800, CRC(ed33c227) SHA1(a96ba2814cef7663728bb5fdea2dc6ecfa219038) )
ROM_LOAD( "cpu_u2.716",   0x5000, 0x0800, CRC(a827a1a1) SHA1(723ebf193b5ce7b19df70e83caa9bb80f2e3fa66) )
ROM_LOAD( "fpch_u1.716",  0x1000, 0x0800, CRC(af26e00e) SHA1(9573cf5a56bbd7a534022b43752c7c25042d707b) )
ROM_LOAD( "fpch_u5.716",  0x1800, 0x0800, CRC(f6c34e1d) SHA1(732b94196a45c0a818fe5613c106f9aad5eae53e) )
ROM_LOAD( "cpu_u6.716",   0x5800, 0x0800, CRC(ed33c227) SHA1(a96ba2814cef7663728bb5fdea2dc6ecfa219038) ) // cheetah1
ROM_LOAD( "cpu_u2.716",   0x5000, 0x0800, CRC(a827a1a1) SHA1(723ebf193b5ce7b19df70e83caa9bb80f2e3fa66) ) // cheetah1
ROM_LOAD( "cpu_u1.716",   0x1000, 0x0800, CRC(6a845d94) SHA1(c272d5895edf2270f5f06fc33345bb4911abbee4) ) // cheetah1
ROM_LOAD( "cpu_u5.716",   0x1800, 0x0800, CRC(fb7e0400) SHA1(81443e93e68d8dfecc3a33d61a1a39e6e9ea34ca) ) // cheetah1
ROM_LOAD( "cpu_u2.716",   0x5000, 0x0800, CRC(a827a1a1) SHA1(723ebf193b5ce7b19df70e83caa9bb80f2e3fa66) ) // cheetah2
ROM_LOAD( "cpu_u1.716",   0x1000, 0x0800, CRC(568db33e) SHA1(a62f48f77dc170d88a5bf2d033e92e409839e749) ) // cheetah2
ROM_LOAD( "cpu_u5.716",   0x1800, 0x0800, CRC(d4e4b50d) SHA1(c59f465ade7512d129a0e60519ec7066db2cbde9) ) // cheetah2
ROM_LOAD( "cpu_u6.716",   0x5800, 0x0800, CRC(f9e66c18) SHA1(41ba7eecf2ff9305d79cc5ae30c08d5b89f03909) ) // cheetah2

Dragonfist
ROM_LOAD( "cpu_u6.716",   0x5800, 0x0800, CRC(a374c8f9) SHA1(481116025a52353f298f3d93dfe33b3ad9f86d18) )
ROM_LOAD( "cpu_u2.716",   0x5000, 0x0800, CRC(9ac8292b) SHA1(99ad3ad6e1d1b19695ce1b5b76f6bd85c9c6530d) )
ROM_LOAD( "cpu_u1.716",   0x1000, 0x0800, CRC(4cbd1a38) SHA1(73b7291f38cd0a3300107605db26d474ecfc3101) )
ROM_LOAD( "fpdf_u5.716",  0x1800, 0x0800, CRC(90b0247b) SHA1(7126459abd160d62cbc5c41c4759464d6def640f) )
ROM_LOAD( "cpu_u5.716",   0x1800, 0x0800, CRC(1783269a) SHA1(75151b79844d26d9e8ecf00dec96643ee2fedc5b) ) // dragfisb
ROM_LOAD( "cpu_u2.716",   0x5000, 0x0800, CRC(9ac8292b) SHA1(99ad3ad6e1d1b19695ce1b5b76f6bd85c9c6530d) ) // dragfisb
ROM_LOAD( "cpu_u1.716",   0x1000, 0x0800, CRC(4cbd1a38) SHA1(73b7291f38cd0a3300107605db26d474ecfc3101) ) // dragfisb
ROM_LOAD( "cpu_u6.716",   0x5800, 0x0800, CRC(7e8db47b) SHA1(4a42636c1baf39072bbe123855c9cc5f20ca6888) ) // dragfisb
ROM_LOAD( "df-1105-u1.716", 0x1000, 0x0800, CRC(5c08ec76) SHA1(56dc76e1bf54054d9d685f80213776bae2135b78) ) // dragfis3
ROM_LOAD( "df-1105-u2.716", 0x5000, 0x0800, CRC(d7d761cd) SHA1(46f8120c9e4b392ca6bf631fa8a10c638674b04d) ) // dragfis3
ROM_LOAD( "df-1105-u5.716", 0x1800, 0x0800, CRC(3d7ddc6e) SHA1(d26176df1307f61ae3ab3ed7b5dbad5cba6b6221) ) // dragfis3
ROM_LOAD( "df-1105-u6.716", 0x5800, 0x0800, CRC(d34b4f61) SHA1(4eda993da4fc61fd247871e992ca37c1ae97cea8) ) // dragfis3

Flight 2000
ROM_LOAD( "free2ku5.716", 0x1800, 0x0800, CRC(c3854b59) SHA1(1ca80f481f98106ebd415268d1ea4538650cd115) )
ROM_LOAD( "free2ku6.716", 0x5800, 0x0800, CRC(228bd69e) SHA1(067576f9893effc1daff94568363bcc49177d4b1) )
ROM_LOAD( "cpu_u1m.716",  0x1000, 0x0800, CRC(da0850ba) SHA1(9f8d9781b67c388f21a39ca8c17f778ed99caae9) ) // flight2m
ROM_LOAD( "cpu_u2m.716",  0x5000, 0x0800, CRC(1a9b72b5) SHA1(ad814e2f9c619838a80039674edfdf0f5c6591e6) ) // flight2m
ROM_LOAD( "cpu_u5m.716",  0x1800, 0x0800, CRC(cb73782e) SHA1(28ba926647d814f88bcd02082a995ae0a2c80f6f) ) // flight2m
ROM_LOAD( "cpu_u6m.716",  0x5800, 0x0800, CRC(437bcd19) SHA1(764ab4a8cde90cdd2138883d2470275b50a5423c) ) // flight2m

Freefall
ROM_LOAD( "fpff_u2.716",  0x5000, 0x0800, CRC(b4f811ac) SHA1(0af78971a2ee32c423b8a95a486e1b8ad0ddd1d9) )

Galaxy
ROM_LOAD( "fpgal_u2.716", 0x5000, 0x0800, CRC(c41ce9e8) SHA1(15e344cb72ce80fa5baa0136f2ca461b1d7d39ac) )
ROM_LOAD( "fpgal_u6.716", 0x5800, 0x0800, CRC(a5ab7492) SHA1(b7c2635ab721d50f87e87cbf779cab7c787be222) )
ROM_LOAD( "cpu_u1b.716",  0x1000, 0x0800, CRC(53f7c0c9) SHA1(c3ee8bbdd1eca7a044c7abf4e0ba6059f523c323) ) // galaxyb
ROM_LOAD( "cpu_u2b.716",  0x5000, 0x0800, CRC(f0b4e60b) SHA1(e1628ec94585fbf4935e824721472cc9c91bbf89) ) // galaxyb
ROM_LOAD( "cpu_u5b.716",  0x1800, 0x0800, CRC(1b1cd31b) SHA1(65a6a58d2c509419fce3142a9ae88d8ea7d25f1c) ) // galaxyb
ROM_LOAD( "cpu_u6b.716",  0x5800, 0x0800, CRC(be4eacc1) SHA1(3d95e8e859312ef0a7ed52356dabe35ed0bebdef) ) // galaxyb
ROM_LOAD( "cpu_u6ps.716", 0x5800, 0x0800, CRC(de4af8c0) SHA1(ed10dc48045cd8e3d6996eeca6baf695ccd3fcfd) ) // galaxyps

Iron Maiden
ROM_LOAD( "fp_imu5.716",  0x1800, 0x0800, CRC(31c16c20) SHA1(ebc022157cf794ecc397217db3e9c5bf6d45113b) )

Lightning
ROM_LOAD( "fpltg_u5.716", 0x1800, 0x0800, CRC(696b8f87) SHA1(becb662f8c5b160eff968079706dee957f5cfb83) )
ROM_LOAD( "fpltg_u6.716", 0x5800, 0x0800, CRC(2a652b4e) SHA1(25d048afc42e31373a3b957bb7fce4c3acfe9668) )

Meteor
ROM_LOAD( "fpmet_u1.716", 0x1000, 0x0800, CRC(7a5472ce) SHA1(7e3be954b66e5f61cfdbb189fa60ca33ad40a975) )
ROM_LOAD( "fpmet_u6.716", 0x5800, 0x0800, CRC(0bd52abc) SHA1(2a40bcffbfc28b44c5badf1833ce6b1bd76a02e5) )
ROM_LOAD( "cpu_u6a.716",  0x5800, 0x0800, CRC(95e95131) SHA1(92f7563802d22007f6da3d7c5fdfa6af2ca35e2d) ) // meteora2
ROM_LOAD( "cpu_u2b.716",  0x5000, 0x0800, CRC(62cd0484) SHA1(754bb6a7c3c6024b642dba4bc148ed110ab14295) ) // meteorb
ROM_LOAD( "cpu_u5b.716",  0x1800, 0x0800, CRC(fe374449) SHA1(6ed39ae54a65a37d1d9bff52a12c5e9caee90cf1) ) // meteorb
ROM_LOAD( "cpu_u6b.716",  0x5800, 0x0800, CRC(10cb5d60) SHA1(1d3da195fbe06b49d08e4ce2ebc5d9d811126aa6) ) // meteorb
ROM_LOAD( "fp10met2.716", 0x5000, 0x0800, CRC(5a33ed97) SHA1(86c0c6cc68c33cdab603b65ec1dd30e208e0b1c1) ) // meteorc
ROM_LOAD( "fp10met6.716", 0x5800, 0x0800, CRC(2ae8e9fb) SHA1(60af7ffef90382c7bf4ec7612e079114481825e1) ) // meteorc
ROM_LOAD( "fp10met1.716", 0x1000, 0x0800, CRC(4b92583f) SHA1(44c78d856694d7fb34089f78212deac18a7c149f) ) // meteorc
ROM_LOAD( "fp10met5.716", 0x1800, 0x0800, CRC(cd795f7a) SHA1(8f6113c7415cf5d2cf09d17c7de7f8e3d1c84334) ) // meteorc
ROM_LOAD( "cpu_u1.716",   0x1000, 0x0800, CRC(e0fd8452) SHA1(a13215378a678e26a565742d81fdadd2e161ba7a) ) // meteord
ROM_LOAD( "cpu_u2.716",   0x5000, 0x0800, CRC(5a33ed97) SHA1(86c0c6cc68c33cdab603b65ec1dd30e208e0b1c1) ) // meteord
ROM_LOAD( "cpu_u5.716",   0x1800, 0x0800, CRC(a0ac4dac) SHA1(05943374cdc9d67a20a00c62213e04f8f72c772c) ) // meteord
ROM_LOAD( "cpu_u6.716",   0x5800, 0x0800, CRC(2ae8e9fb) SHA1(60af7ffef90382c7bf4ec7612e079114481825e1) ) // meteord
ROM_LOAD( "meteor64a-u1.716", 0x1000, 0x0800, CRC(1854eeef) SHA1(3d7c0eafc06e98ea6a6ba838926db77f246689b4) ) // meteore
ROM_LOAD( "meteor64a-u2.716", 0x5000, 0x0800, CRC(2036529d) SHA1(5c2fbaea21180d28d959a79a39640379dc2e9d37) ) // meteore
ROM_LOAD( "meteor64a-u5.716", 0x1800, 0x0800, CRC(3562c73a) SHA1(de6fb056190459bec9c4f51505278ab06db81958) ) // meteore
ROM_LOAD( "meteor64a-u6.716", 0x5800, 0x0800, CRC(88148373) SHA1(af443866b05916646b2615fe242c577b4ae0edb9) ) // meteore
ROM_LOAD( "meteor74a-u1.716", 0x1000, 0x0800, CRC(92e9c6a1) SHA1(4b044efd5d3136d4749bf2184f714f2af9a6d487) ) // meteore7
ROM_LOAD( "meteor74a-u2.716", 0x5000, 0x0800, CRC(3ab7e5fa) SHA1(a751b40ecea0e0d6054bffed6be09d971a3c748a) ) // meteore7
ROM_LOAD( "meteor74a-u5.716", 0x1800, 0x0800, CRC(25de4151) SHA1(530e75e2c28e5480bf13682c4c7def19bbb9ab87) ) // meteore7
ROM_LOAD( "meteor74a-u6.716", 0x5800, 0x0800, CRC(8d6e6e97) SHA1(0fde3ee4956071f1fcad7acd5311588d6e442a39) ) // meteore7

Nineball
ROM_LOAD( "fp9b_u2.716",  0x5000, 0x0800, CRC(f22c2fb6) SHA1(51b927d01d9d1e0b6a3169677a559c4a2b02c198) )
ROM_LOAD( "fp9b_u6.716",  0x5800, 0x0800, CRC(d4599ee8) SHA1(0b9cfa48e335dffb427fd02c0f63814350c52aca) )
ROM_LOAD( "cpu_u1a.716",  0x1000, 0x0800, CRC(52891fc8) SHA1(81a0d9105136d6b561321cd96974a7497c49a8b6) ) // ninebala
ROM_LOAD( "cpu_u2a.716",  0x5000, 0x0800, CRC(f89d1b8f) SHA1(d5b2bcdea335367ce31576a67a9464fc146a6fb4) ) // ninebala
ROM_LOAD( "cpu_u5a.716",  0x1800, 0x0800, CRC(0dfbc660) SHA1(ad2f3f2176624f24b655609808351c1157e14438) ) // ninebala
ROM_LOAD( "cpu_u6a.716",  0x5800, 0x0800, CRC(78aeba96) SHA1(da091b90a0be4c521d323d6fd012210e341c410e) ) // ninebala
ROM_LOAD( "nineball.256", 0x0000, 0x8000, CRC(06cb8a63) SHA1(c901bba0b41b45c5cfa6d04181f1e035beab5a08) ) // ninebalb, looks like a memory dump of the first 32k

Orbitor 1
ROM_LOAD( "fpo1_u2.716",  0x5000, 0x0800, CRC(83a6dc48) SHA1(ab6e8e5078ce94bd052ebaef9d323154b5c053bf) )
ROM_LOAD( "fpo1_u5.716",  0x1800, 0x0800, CRC(d6eadb75) SHA1(dc3b483d76f3d02a3f8dd1d298f432ff594b968d) )
ROM_LOAD( "o1v3_u1.716",  0x1000, 0x0800, CRC(31677402) SHA1(5814c4eaa8c36842b6a9c3bbafed1efff39c7b3a) ) // orbitora
ROM_LOAD( "o1v3_u5.716",  0x1800, 0x0800, CRC(e0e1c8a0) SHA1(40c030c2d80ce3d894a85b1687678e5a545dc701) ) // orbitora
ROM_LOAD( "o1v4u1.716",   0x1000, 0x0800, CRC(fcf502bd) SHA1(f1c84d5b29e20eada1aa35e811bd9fec451099f7) ) // orbitorb
ROM_LOAD( "o1v4u2.716",   0x5000, 0x0800, CRC(7b708b68) SHA1(cc649d2e6bed80f5608e53578239dca76615c746) ) // orbitorb
ROM_LOAD( "o1v4u5.716",   0x1800, 0x0800, CRC(4dafd165) SHA1(4a5e09200282c4d9d432bd3f839a16dbb3198e31) ) // orbitorb
ROM_LOAD( "mod5.716",     0x1800, 0x0800, CRC(57896ec0) SHA1(a8aefa1fd8f05f3e2e112f2c2de474e71070ae05) ) // orbitorc
ROM_LOAD( "o1v3_u1.716",  0x1000, 0x0800, CRC(31677402) SHA1(5814c4eaa8c36842b6a9c3bbafed1efff39c7b3a) ) // orbitorc

Quicksilver
ROM_LOAD( "fpqs_u6.716",  0x5800, 0x0800, CRC(0514d8c3) SHA1(4ebbc5a76ac25d38ec97a2fd443b492ea5335ae2) )
ROM_LOAD( "cpu_u6.716",   0x5800, 0x0800, CRC(8c0e336a) SHA1(8d3a5b7c07d03c7e2945ea60c72f9181d3ee2a14) ) // a8
ROM_LOAD( "cpu_u5.716",   0x1800, 0x0800, CRC(e2634491) SHA1(376d2c039a1badd843bfbdc7fa2fc57a0014ea6f) ) // a8
ROM_LOAD( "cpu_u2.716",   0x5000, 0x0800, CRC(8cb01165) SHA1(b42e2ccce2c20ad570cdcdb63c9d12e414f9b255) ) // a8
ROM_LOAD( "cpu_u1.716",   0x1000, 0x0800, CRC(0bf508d1) SHA1(57417ec6c6192cd89ef9c5b31d5603a53d5cff3a) ) // a8

Sea Witch
ROM_LOAD( "fpsw_u1.716",  0x1000, 0x0800, CRC(058aa5d4) SHA1(a6aafbcb7c720f1c02ee98a170d2985eda58076c) )
ROM_LOAD( "fpsw_u5.716",  0x1800, 0x0800, CRC(d3072929) SHA1(6dcc2fdc334d3f3f39355570346406d6eea3ca89) )

Split Second
ROM_LOAD( "fpspscu5.716", 0x1800, 0x0800, CRC(230e3be4) SHA1(4e2268e61af2105bdc92a55eeb99cbbb68d007ba) )
ROM_LOAD( "fpspscu6.716", 0x5800, 0x0800, CRC(8e8c831e) SHA1(28985ea7342284ffcc69a53b63053f15a0cd408b) )

Star Gazer
ROM_LOAD( "fpsg_u1.716",  0x1000, 0x0800, CRC(4a4e3847) SHA1(87205061f5c981a2e6be6b8d56c8bc563611a30c) )
ROM_LOAD( "fpsg_u5.716",  0x1800, 0x0800, CRC(902398ad) SHA1(6ab51ac90819167a8c8da3d5aacafdc0e9f939fb) )
ROM_LOAD( "cpu_u2b.716",  0x5000, 0x0800, CRC(360427cc) SHA1(ad76124b7fd088a5e2d24cf369c1620cdcc80309) ) // stargzrb
ROM_LOAD( "cpu_u5b.716",  0x1800, 0x0800, CRC(29682d85) SHA1(3f449270cd4098a7ed1ee9c0d801110b1b653913) ) // stargzrb
ROM_LOAD( "cpu_u6b.716",  0x5800, 0x0800, CRC(b68b11c5) SHA1(1af6aca8ecf70d2adf588a1e856f753193c05abd) ) // stargzrb

Viper
ROM_LOAD( "fpvip_u5.716", 0x1800, 0x0800, CRC(a6802658) SHA1(81d6366694491cf16f4427ee20a068802410db26) )

*/

/*--------------------------------
/ Meteor #113
/-------------------------------*/
ROM_START(meteorp)
	ROM_REGION(0x6000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD( "25arom_p21a.u1", 0x1000, 0x0800, CRC(9ee33909) SHA1(5f58e4e72af47047c8f060f98706ed9607720705))
	ROM_LOAD( "25arom_p23.u5",  0x1800, 0x0800, CRC(43a46997) SHA1(2c74ca10cf9091db10542960f499f39f3da277ee))
	ROM_LOAD( "25arom_p22.u2",  0x5000, 0x0800, CRC(fd396792) SHA1(b5d051a7ce7e7c2f9c4a0d900cef4f9ef2089476))
	ROM_LOAD( "25arom_p24.u6",  0x5800, 0x0800, CRC(03fa346c) SHA1(51c04123cb433e90920c241e2d1f89db4643427b))
ROM_END

ROM_START(meteorpo)   // Original release has the bonus countdown bug when the thread engine gets overloaded with scoring
	ROM_REGION(0x6000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD( "25arom_p21.u1",  0x1000, 0x0800, CRC(e0fd8452) SHA1(a13215378a678e26a565742d81fdadd2e161ba7a))
	ROM_LOAD( "25arom_p23.u5",  0x1800, 0x0800, CRC(43a46997) SHA1(2c74ca10cf9091db10542960f499f39f3da277ee))
	ROM_LOAD( "25arom_p22.u2",  0x5000, 0x0800, CRC(fd396792) SHA1(b5d051a7ce7e7c2f9c4a0d900cef4f9ef2089476))
	ROM_LOAD( "25arom_p24.u6",  0x5800, 0x0800, CRC(03fa346c) SHA1(51c04123cb433e90920c241e2d1f89db4643427b))
ROM_END

/*--------------------------------
/ Galaxy #114
/-------------------------------*/
ROM_START(galaxypi)
	ROM_REGION(0x6000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD( "cpu_u1.716", 0x1000, 0x0800, CRC(35656b67) SHA1(e1ad9456c561d19220f8607576cb505588512179))
	ROM_LOAD( "cpu_u5.716", 0x1800, 0x0800, CRC(12be0601) SHA1(d651b834348c071dda660f37b4e359bf01cbd8d3))
	ROM_LOAD( "cpu_u2.716", 0x5000, 0x0800, CRC(08bdb285) SHA1(7984835ac151e5dac05628f3d5146d20e3623c38))
	ROM_LOAD( "cpu_u6.716", 0x5800, 0x0800, CRC(ad846a42) SHA1(303c9cb933ca60d35e12793a4ac0cf7ef11bc92e))
ROM_END

/*--------------------------------
/ Cheetah #116
/-------------------------------*/
ROM_START(cheetah)
	ROM_REGION(0x6000, "maincpu", ROMREGION_ERASEFF)   // Black cabinet version
	ROM_LOAD( "cheetah__r_b20.u1", 0x1000, 0x0800, CRC(6a845d94) SHA1(c272d5895edf2270f5f06fc33345bb4911abbee4))   // 25A-E1B exists is there an E1A and/or E1 version?
	ROM_LOAD( "cheetah__r_b20.u5", 0x1800, 0x0800, CRC(e7bdbe6c) SHA1(8b213c2271dbd5157e0d34a33672130b935d76be))   // 25A-E5
	ROM_LOAD( "cheetah__r_b20.u2", 0x5000, 0x0800, CRC(a827a1a1) SHA1(723ebf193b5ce7b19df70e83caa9bb80f2e3fa66))   // 25A-E2
	ROM_LOAD( "cheetah__r_b20.u6", 0x5800, 0x0800, CRC(ed33c227) SHA1(a96ba2814cef7663728bb5fdea2dc6ecfa219038))   // 25A-E6
ROM_END

ROM_START(cheetahb)
	ROM_REGION(0x6000, "maincpu", ROMREGION_ERASEFF)   // Blue cabinet version - has different sound effects to the black cabinet version
	ROM_LOAD( "cheetah__x_b16.u1", 0x1000, 0x0800, CRC(2f736a0a) SHA1(e0dc14215d90145881ac1b407fbe057770696122))
	ROM_LOAD( "cheetah__x_b16.u5", 0x1800, 0x0800, CRC(168f0650) SHA1(5b3294bf64f06cc9d193bb14891b2acfbb5c06d4))
	ROM_LOAD( "cheetah__x_b16.u2", 0x5000, 0x0800, CRC(f6bd41bc) SHA1(ac94f4ba17c31dfe10ab7efab63d98aa3401e4ae))
	ROM_LOAD( "cheetah__x_b16.u6", 0x5800, 0x0800, CRC(c7eba210) SHA1(ced377e53f30b371e74c26527e5f8bebcc10ee59))
ROM_END

/*--------------------------------
/ Quicksilver #117
/-------------------------------*/
ROM_START(quicksil)
	ROM_REGION(0x6000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD( "cpu_u1.716", 0x1000, 0x0800, CRC(fc1bd20a) SHA1(e3c547f996dfc5d1567223d234443cf31d648ef6))
	ROM_LOAD( "cpu_u5.716", 0x1800, 0x0800, CRC(0bcaceb4) SHA1(461d2fe5772a5ac84d31a4a186b9f639c683ca8a))
	ROM_LOAD( "cpu_u2.716", 0x5000, 0x0800, CRC(8cb01165) SHA1(b42e2ccce2c20ad570cdcdb63c9d12e414f9b255))
	ROM_LOAD( "cpu_u6.716", 0x5800, 0x0800, CRC(8c0e336a) SHA1(8d3a5b7c07d03c7e2945ea60c72f9181d3ee2a14))
ROM_END

/*--------------------------------
/ Ali #119
/-------------------------------*/
ROM_START(ali)
	ROM_REGION(0x6000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD( "cpu_u1.716", 0x1000, 0x0800, CRC(92e75b40) SHA1(bace68db0ea12d50a546157d11084f3b00949136))
	ROM_LOAD( "cpu_u5.716", 0x1800, 0x0800, CRC(119a4300) SHA1(e913d9bd399b90502efe110c8bf7f23ae07df276))
	ROM_LOAD( "cpu_u2.716", 0x5000, 0x0800, CRC(9c91d08f) SHA1(a3e8c8e8c2c8b03d86b36eea8c84e5c0a27b8444))
	ROM_LOAD( "cpu_u6.716", 0x5800, 0x0800, CRC(7629db56) SHA1(f922d31ec4dd1755da0a24bec4e3fa3a7a9b22fc))
ROM_END


/*--------------------------------
/ Big Game #121
/-------------------------------*/
ROM_START(biggame)
	ROM_REGION(0x6000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD( "cpu_u1.716", 0x1000, 0x0800, CRC(f59c7514) SHA1(49ab034a21e70956f63327aec4cbae115cd66a66))
	ROM_LOAD( "cpu_u5.716", 0x1800, 0x0800, CRC(57df1dc5) SHA1(283f45879b76d56ba0db0fb3d9d9771f91a70d02))
	ROM_LOAD( "cpu_u2.716", 0x5000, 0x0800, CRC(0251039b) SHA1(0a0e662788cf012dfb773d200c542a2a363748a8))
	ROM_LOAD( "cpu_u6.716", 0x5800, 0x0800, CRC(801e9a66) SHA1(8634d6bd4af3e5ec3b736679393462961b76ede1))
ROM_END

/*--------------------------------
/ Seawitch #123
/-------------------------------*/
ROM_START(seawitch)
	ROM_REGION(0x6000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD( "cpu_u1.716", 0x1000, 0x0800, CRC(c214140b) SHA1(4d68ddd3b0f051c5f601ea5b9d5d5195d6017304))
	ROM_LOAD( "cpu_u5.716", 0x1800, 0x0800, CRC(ab2eab3a) SHA1(80a8c1ccd554be279720a26466bd6c59e1e56df0))
	ROM_LOAD( "cpu_u2.716", 0x5000, 0x0800, CRC(b8844174) SHA1(6e01321196fd6fce7b5526efc402044c87fe96a6))
	ROM_LOAD( "cpu_u6.716", 0x5800, 0x0800, CRC(6c296d8f) SHA1(8cdb77f382ef1214ef45579213cf8f19141366ad))
ROM_END

/*--------------------------------
/ Nine Ball #125
/-------------------------------*/
ROM_START(nineball)
	ROM_REGION(0x6000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD( "cpu_u1.716", 0x1000, 0x0800, CRC(fcb58f97) SHA1(6510a6d0b466bd27ade50992260cea716d79fda2))
	ROM_LOAD( "cpu_u5.716", 0x1800, 0x0800, CRC(c7c62161) SHA1(624eab2fdf7bafbf4af012df521bd09f9b2da8d8))
	ROM_LOAD( "cpu_u2.716", 0x5000, 0x0800, CRC(bdd7f258) SHA1(2a38de09827100cbbd4e79be50aad03a3f2b63b4))
	ROM_LOAD( "cpu_u6.716", 0x5800, 0x0800, CRC(7e831499) SHA1(8d3c148b91c21938b1b5fca85ecd8f6d7f1e76b0))
ROM_END

/*--------------------------------
/ Lightning #126
/-------------------------------*/
ROM_START(lightnin)
	ROM_REGION(0x6000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD( "cpu_u1.716", 0x1000, 0x0800, CRC(d3469d0a) SHA1(18565f5c85694da8eaf850146d3d9a90a17b7816))
	ROM_LOAD( "cpu_u5.716", 0x1800, 0x0800, CRC(cd52262d) SHA1(099aeda2183822046cce907b265b42319007ac32))
	ROM_LOAD( "cpu_u2.716", 0x5000, 0x0800, CRC(e0933419) SHA1(1f7cad915496f34473dffde7e320d51838acd0fd))
	ROM_LOAD( "cpu_u6.716", 0x5800, 0x0800, CRC(df221c6b) SHA1(5935020d3a24d829fbeaa8cf764daff48a151a81))

	ROM_REGION(0x1000, "speech", 0)
	ROM_LOAD("snd_u9.716", 0x0000, 0x0800, CRC(00ffa77c) SHA1(242efd800731a7f84369c6ce54298d0a227dd8ba))
	ROM_LOAD("snd_u10.716",0x0800, 0x0800, CRC(80fe9158) SHA1(20fcdb4c09b25e494f02bbfb20c07ff2870d5798))
ROM_END

/*--------------------------------
/ Stargazer #127
/-------------------------------*/
ROM_START(stargzr)
	ROM_REGION(0x6000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD( "cpu_u1.716", 0x1000, 0x0800, CRC(83606fd4) SHA1(7f6448bc0dabe50de40fd47a7242c1be4a93e84d))
	ROM_LOAD( "cpu_u5.716", 0x1800, 0x0800, CRC(c54ae389) SHA1(062e64e8ced723adb7f4040539ba6400fc4a9c9a))
	ROM_LOAD( "cpu_u2.716", 0x5000, 0x0800, CRC(1a4c7dcb) SHA1(54888a8867b8d60f215b7e683ae4966f14ddca15))
	ROM_LOAD( "cpu_u6.716", 0x5800, 0x0800, CRC(4e1f4dc6) SHA1(1f63a0b71af84fb6e1168ff77cbcbabcaa1323f3))
ROM_END

/*--------------------------------
/ Flight 2000 #128
/-------------------------------*/
ROM_START(flight2k)
	ROM_REGION(0x6000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD( "cpu_u1.716", 0x1000, 0x0800, CRC(df9efed9) SHA1(47727664e745e77ca1c221a32bd56d936f5b31bc))
	ROM_LOAD( "cpu_u5.716", 0x1800, 0x0800, CRC(38c13649) SHA1(bcdbd17b48edd41ec7d38261595ac06eb8fc6a4d))
	ROM_LOAD( "cpu_u2.716", 0x5000, 0x0800, CRC(425fae6a) SHA1(fde8d23e6ebb176ba72f763d66c2e17e51237fa1))
	ROM_LOAD( "cpu_u6.716", 0x5800, 0x0800, CRC(dc243186) SHA1(046ce51b8a8218214088c4264548c753bd880e19))

	ROM_REGION(0x1000, "speech", 0)
	ROM_LOAD("snd_u9.716", 0x0000, 0x0800, CRC(d816573c) SHA1(75134a017c34abbb149159ca001d35464a3f5128))
ROM_END

/*--------------------------------
/ Freefall #134
/-------------------------------*/
ROM_START(freefall)
	ROM_REGION(0x6000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD( "cpu_u1.716", 0x1000, 0x0800, CRC(d13891ad) SHA1(afb40c51f2d5695c74ce9979c0a818845f95edd4))
	ROM_LOAD( "cpu_u5.716", 0x1800, 0x0800, CRC(77bc7759) SHA1(3f739757180b3dcce5426935a51e4b615f157199))
	ROM_LOAD( "cpu_u2.716", 0x5000, 0x0800, CRC(82bda054) SHA1(32772e878d2a4bba8f67e419a68a81fec2a5f6d7))
	ROM_LOAD( "cpu_u6.716", 0x5800, 0x0800, CRC(68168b97) SHA1(defa4bba465182db22debddb4070c40c048c95e2))

	ROM_REGION(0x1000, "speech", 0)
	ROM_LOAD("snd_u9.716", 0x0000, 0x0800, CRC(ea8cf062) SHA1(55c840a9bea363fd436c00a115cb61d15a9f8c47))
	ROM_LOAD("snd_u10.716",0x0800, 0x0800, CRC(dd681a79) SHA1(d954cae375fb0145e10536e43d1cb03902de2ea3))
ROM_END

/*--------------------------------
/ Split Second #144
/-------------------------------*/
ROM_START(spltsecp)
	ROM_REGION(0x6000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD( "cpu_u1.716", 0x1000, 0x0800, CRC(c6ff9aa9) SHA1(39f80faca16c869ac14df7c5fc3dfa80b47dad95))
	ROM_LOAD( "cpu_u5.716", 0x1800, 0x0800, CRC(fda74efc) SHA1(31becc243ada23e2f4d17927985772c9fcf8a3c3))
	ROM_LOAD( "cpu_u2.716", 0x5000, 0x0800, CRC(81b9f784) SHA1(43cf71b51eda70a3c126340ea658c03c438e4f18))
	ROM_LOAD( "cpu_u6.716", 0x5800, 0x0800, CRC(ecbedb0a) SHA1(8cc7281dd2bd300ab95a08761c12733d98599ebd))

	ROM_REGION(0x1000, "speech", 0)
	ROM_LOAD("snd_u9.716", 0x0000, 0x0800, CRC(e6ed5f48) SHA1(ea2bbc607acb2b816667cd54f3d07605110c252e))
	ROM_LOAD("snd_u10.716",0x0800, 0x0800, CRC(36e6ee70) SHA1(61bd89d69627bea89b7f31af63ff90ace6db3c85))
ROM_END

/*--------------------------------
/ Catacomb #147
/-------------------------------*/
ROM_START(catacomp)
	ROM_REGION(0x6000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD( "cpu_u1.716", 0x1000, 0x0800, CRC(d445dd40) SHA1(9ff5896977d7e2a0cf788c77dcfd7c010e17d2fb))
	ROM_LOAD( "cpu_u5.716", 0x1800, 0x0800, CRC(d717a545) SHA1(a183f3b1f766c3a82ae52defc38d84328fb7b31a))
	ROM_LOAD( "cpu_u2.716", 0x5000, 0x0800, CRC(bc504409) SHA1(cd3e948d34a8db71fc841261e683988c9df31ef8))
	ROM_LOAD( "cpu_u6.716", 0x5800, 0x0800, CRC(da61b5a2) SHA1(ec4a914cd57b37921578699bc427f12a3670c7eb))

	ROM_REGION(0x1000, "speech", 0)
	ROM_LOAD("snd_u9.716", 0x0000, 0x0800, CRC(a13cb591) SHA1(b64a2dc3429803095dc05cdd1718db2404b13eb8))
	ROM_LOAD("snd_u10.716",0x0800, 0x0800, CRC(2b31f8be) SHA1(05b394bd8b6c04e34fe2bab19cbd0f06d9e4b90d))
ROM_END

/*--------------------------------
/ Viper #148
/-------------------------------*/
ROM_START(viperp)
	ROM_REGION(0x6000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD( "cpu_u1.716", 0x1000, 0x0800, CRC(d0ea0aeb) SHA1(28f4df9f45807abd1528aa6e5a80933156e6d692))
	ROM_LOAD( "cpu_u5.716", 0x1800, 0x0800, CRC(d26c7273) SHA1(303c18861941463932fdf47e9606159936b28dc1))
	ROM_LOAD( "cpu_u2.716", 0x5000, 0x0800, CRC(d03f1612) SHA1(d390ec1e953148ac26bf218701117855c941fc65))
	ROM_LOAD( "cpu_u6.716", 0x5800, 0x0800, CRC(96ff5f60) SHA1(a9df887ca338db208a684540f6c9fc07722c3aa5))
ROM_END

/*--------------------------------
/ Iron Maiden #151
/-------------------------------*/
ROM_START(ironmaid)
	ROM_REGION(0x6000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD( "cpu_u1.716", 0x1000, 0x0800, CRC(e15371a4) SHA1(fe441ed8abd325190d8eee6d907e17c7fc02be64))
	ROM_LOAD( "cpu_u5.716", 0x1800, 0x0800, CRC(84a29c01) SHA1(0e0ff8821c7028ce690328cd08a77bb51c0993c9))
	ROM_LOAD( "cpu_u2.716", 0x5000, 0x0800, CRC(981ac0dd) SHA1(c585907b74695812f333867cf359a01a5ea6ed81))
	ROM_LOAD( "cpu_u6.716", 0x5800, 0x0800, CRC(4e6f9c25) SHA1(9053e1d335a29f7acade7752adffe69f42032959))
ROM_END

/*--------------------------------
/ Hypnox #152
/-------------------------------*/
// Never produced. Possibly 2 prototypes existed.

/*--------------------------------
/ Dragonfist #153
/-------------------------------*/
ROM_START(dragfist)
	ROM_REGION(0x6000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD( "cpu_u1.716", 0x1000, 0x0800, CRC(4cbd1a38) SHA1(73b7291f38cd0a3300107605db26d474ecfc3101))
	ROM_LOAD( "cpu_u5.716", 0x1800, 0x0800, CRC(1783269a) SHA1(75151b79844d26d9e8ecf00dec96643ee2fedc5b))
	ROM_LOAD( "cpu_u2.716", 0x5000, 0x0800, CRC(9ac8292b) SHA1(99ad3ad6e1d1b19695ce1b5b76f6bd85c9c6530d))
	ROM_LOAD( "cpu_u6.716", 0x5800, 0x0800, CRC(a374c8f9) SHA1(481116025a52353f298f3d93dfe33b3ad9f86d18))
ROM_END

/*--------------------------------
/ Orbitor 1 #165
/-------------------------------*/
ROM_START(orbitor1)
	ROM_REGION(0x6000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD( "cpu_u1.716", 0x1000, 0x0800, CRC(575520e3) SHA1(9d52b065a14d4f95cebd48f60f628f2c246385fa))
	ROM_LOAD( "cpu_u5.716", 0x1800, 0x0800, CRC(d31f27a8) SHA1(0442260db42192a95f6292e6b57000c127871d28))
	ROM_LOAD( "cpu_u2.716", 0x5000, 0x0800, CRC(4421d827) SHA1(9b617215f2d92ef2c69104eb4e63a924704665aa))
	ROM_LOAD( "cpu_u6.716", 0x5800, 0x0800, CRC(8861155a) SHA1(81a1b3434d4f80dee5704454f8359200faea173d))

	ROM_REGION(0x1000, "speech", 0)
	ROM_LOAD("snd_u9.716", 0x0000, 0x0800, CRC(2ba24569) SHA1(da2f4a4eeed9ae7ff8a342f4d630e12dcb2decf5))
	ROM_LOAD("snd_u10.716",0x0800, 0x0800, CRC(8e5b4a38) SHA1(de3f59363553f5f0d6098401734436930e64fbbd))
ROM_END

/*--------------------------------
/ Cue (Proto - Never released)
/-------------------------------*/

ROM_START(cue)
	ROM_REGION(0x6000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD( "cpu_u1.716", 0x1000, 0x0800, CRC(0e1b4136) SHA1(ce69436a8cd30e2056df2ef86339f2e98e749774))
	ROM_LOAD( "cpu_u5.716", 0x1800, 0x0800, CRC(65e15866) SHA1(a5f0d156b7429e2565da762d53decf8bc1589a5e))
	ROM_LOAD( "cpu_u2.716", 0x5000, 0x0800, CRC(7a30ea8e) SHA1(5c8b1ad0add887c5986559c640d620971739e9a1))
	ROM_LOAD( "cpu_u6.716", 0x5800, 0x0800, CRC(412d4592) SHA1(2bcc8832875bd6be49e17328069c19c955f35f8d))
ROM_END


/*----------------------------------------
/ Lazer Lord (Proto - Never released)
/---------------------------------------*/
ROM_START(lazrlord)
	ROM_REGION(0x6000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD( "cpu_u1.716", 0x1000, 0x0800, CRC(32a6f341) SHA1(75922c6831463d240fe057a0f72280d417899fa4))
	ROM_LOAD( "cpu_u5.716", 0x1800, 0x0800, CRC(17583ba4) SHA1(4807e3ab18c2e40a292b499fe038975bb4b9fc17))
	ROM_LOAD( "cpu_u2.716", 0x5000, 0x0800, CRC(669f3a8e) SHA1(4beb0e4c75f4e3c1788808b57081612d4774d130))
	ROM_LOAD( "cpu_u6.716", 0x5800, 0x0800, CRC(395327a3) SHA1(e2a3a8ea696bcc4b5e11b08b6c7a6d9a991aa4af))
ROM_END

/*--------------------------------
/ Gamatron (Pinstar game, 1985)
/-------------------------------*/
ROM_START(gamatron)
	ROM_REGION(0x6000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD( "gamatron.764", 0x1000, 0x0800, CRC(fa9f7676) SHA1(8c56868eb6af7bb8ad73523ab6583100fcadc3c1))
	ROM_CONTINUE( 0x5000, 0x0800)
	ROM_CONTINUE( 0x1800, 0x0800)
	ROM_CONTINUE( 0x5800, 0x0800)
ROM_END

/*----------------------------------------
/ Stern SAM III Test Fixture
/  working - see the manual
/----------------------------------------*/
ROM_START(st_sam)
	ROM_REGION(0x6000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD( "sam_iii_rev5.u2", 0x5000, 0x0800, CRC(b9ac5204) SHA1(1ac4e336eb62c091e61e9b6b21a858e70ac9ab38))
	ROM_LOAD( "sam_iii_rev5.u6", 0x5800, 0x0800, CRC(e16fbde1) SHA1(f7fe2f2ef9251792af1227f82dcc95239dd8baa1))
ROM_END

/*----------------------------------------
/ Stern SAM IV Test Fixture
/----------------------------------------*/
ROM_START(st_sam4)
	ROM_REGION(0x1000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD( "sam_iv_r_c5.u1", 0x0c00, 0x0400, CRC(5fc44fc9) SHA1(aef3b9dbb0ba1c110b20b8e577168f4c67b6c99d) )
	ROM_LOAD( "sam_iv_r_c5.u3", 0x0800, 0x0400, CRC(121a4db0) SHA1(a6a94fb4e17ca1ebcd009b96de6a3c253c7fb510) )
	ROM_LOAD( "sam_iv_r_c5.u5", 0x0400, 0x0400, CRC(361af770) SHA1(1d9698bf261e4f34c7304569c3b5c6d31edaa16a) )
	ROM_LOAD( "sam_iv_r_c5.u7", 0x0000, 0x0400, CRC(8766c667) SHA1(d6e6d1927016487f1429d084ec6b1abf54c004c5) )
ROM_END

} // Anonymous namespace


// 6-digit
GAME(1979,  meteorp,    0,          st_mp200,   mp200, st_mp200_state, init_st_mp202, ROT0, "Stern",     "Meteor (Bug fix release)",    MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME(1979,  meteorpo,   meteorp,    st_mp200,   mp200, st_mp200_state, init_st_mp202, ROT0, "Stern",     "Meteor (First release)",      MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME(1980,  galaxypi,   0,          st_mp200,   mp200, st_mp200_state, init_st_mp202, ROT0, "Stern",     "Galaxy",                      MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME(1980,  ali,        0,          st_mp200,   mp200, st_mp200_state, init_st_mp202, ROT0, "Stern",     "Ali",                         MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )

// 7-digit
GAME(1980,  cheetah,    0,          st_mp200,   mp200, st_mp200_state, init_st_mp200, ROT0, "Stern",     "Cheetah (Black Cabinet)",     MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME(1980,  cheetahb,   cheetah,    st_mp200,   mp200, st_mp200_state, init_st_mp202, ROT0, "Stern",     "Cheetah (Blue Cabinet)",      MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME(1980,  quicksil,   0,          st_mp200,   mp200, st_mp200_state, init_st_mp200, ROT0, "Stern",     "Quicksilver",                 MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME(1980,  biggame,    0,          st_mp200,   mp200, st_mp200_state, init_st_mp200, ROT0, "Stern",     "Big Game",                    MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME(1980,  seawitch,   0,          st_mp200,   mp200, st_mp200_state, init_st_mp200, ROT0, "Stern",     "Seawitch",                    MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME(1980,  stargzr,    0,          st_mp200,   mp200, st_mp200_state, init_st_mp200, ROT0, "Stern",     "Stargazer",                   MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME(1982,  dragfist,   0,          st_mp200,   mp200, st_mp200_state, init_st_mp200, ROT0, "Stern",     "Dragonfist",                  MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME(1982,  cue,        0,          st_mp200,   mp200, st_mp200_state, init_st_mp200, ROT0, "Stern",     "Cue (Prototype)",             MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )

// multiball
GAME(1980,  nineball,   0,          st_mp200,   mp200, st_mp200_state, init_st_mp200, ROT0, "Stern",     "Nine Ball",                   MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME(1981,  lightnin,   0,          st_mp201,   mp200, st_mp200_state, init_st_mp201, ROT0, "Stern",     "Lightning",                   MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME(1980,  flight2k,   0,          st_mp201,   mp200, st_mp200_state, init_st_mp201, ROT0, "Stern",     "Flight 2000",                 MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME(1981,  freefall,   0,          st_mp201,   mp200, st_mp200_state, init_st_mp201, ROT0, "Stern",     "Freefall",                    MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME(1981,  spltsecp,   0,          st_mp201,   mp200, st_mp200_state, init_st_mp201, ROT0, "Stern",     "Split Second (Pinball)",      MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME(1981,  catacomp,   0,          st_mp201,   mp200, st_mp200_state, init_st_mp201, ROT0, "Stern",     "Catacomb (Pinball)",          MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME(1981,  viperp,     0,          st_mp200,   mp200, st_mp200_state, init_st_mp200, ROT0, "Stern",     "Viper (Pinball)",             MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME(1981,  ironmaid,   0,          st_mp200,   mp200, st_mp200_state, init_st_mp200, ROT0, "Stern",     "Iron Maiden",                 MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME(1982,  orbitor1,   0,          st_mp201,   mp200, st_mp200_state, init_st_mp201, ROT0, "Stern",     "Orbitor 1",                   MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME(1984,  lazrlord,   0,          st_mp200,   mp200, st_mp200_state, init_st_mp200, ROT0, "Stern",     "Lazer Lord",                  MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )

// other manufacturer
GAME(1985,  gamatron,   flight2k,   st_mp200,   mp200, st_mp200_state, init_st_mp200, ROT0, "Pinstar",   "Gamatron",                    MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME(198?,  st_sam,     0,          st_mp200,   mp200, st_mp200_state, init_st_mp200, ROT0, "Stern",     "SAM III Test Fixture",        MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME(198?,  st_sam4,    st_sam,     st_sam4,    mp200, st_mp200_state, init_st_mp200, ROT0, "Stern",     "SAM IV Test Fixture",         MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
