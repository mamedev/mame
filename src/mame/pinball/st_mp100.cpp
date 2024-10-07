// license:BSD-3-Clause
// copyright-holders:Robbbert
/********************************************************************************************

PINBALL
Stern MP-100 MPU
(almost identical to Bally MPU-17)


Status:
- pinball,stingray,stars,memlane,blkshpsq: working
- others are working but with wrong/missing sounds

ToDo:
- Dips, Inputs, Solenoids vary per game
- Sound board - an enormous mass of discrete circuitry

*********************************************************************************************/


#include "emu.h"
#include "genpin.h"
#include "cpu/m6800/m6800.h"
#include "machine/6821pia.h"
#include "machine/clock.h"
#include "machine/input_merger.h"
#include "st_mp100.lh"


namespace {

class st_mp100_state : public genpin_class
{
public:
	st_mp100_state(const machine_config &mconfig, device_type type, const char *tag)
		: genpin_class(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
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

	void st_mp100(machine_config &config);

	DECLARE_INPUT_CHANGED_MEMBER(activity_test);
	DECLARE_INPUT_CHANGED_MEMBER(self_test);

protected:
	virtual void machine_reset() override ATTR_COLD;
	virtual void machine_start() override ATTR_COLD;

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

	u8 m_u10a = 0U;
	u8 m_u10b = 0U;
	u8 m_u11a = 0U;
	u8 m_u11b = 0U;
	bool m_u10_ca2 = false;
	bool m_u10_cb2 = false;
	bool m_u11_cb2 = false;
	u8 m_stored_lamp = 0xffU;
	u8 m_digit = 0U;
	u8 m_counter = 0U;
	u8 m_segment[5]{};
	u8 m_last_solenoid = 15U;

	required_device<m6800_cpu_device> m_maincpu;
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
	output_finder<48> m_digits;
	output_finder<1> m_io_leds;
	output_finder<80> m_io_outputs;   // 16 solenoids + 64 lamps
};


void st_mp100_state::mem_map(address_map &map)
{
	map.global_mask(0x1fff);
	map(0x0000, 0x007f).ram(); // internal to the cpu
	map(0x0088, 0x008b).rw(m_pia_u10, FUNC(pia6821_device::read), FUNC(pia6821_device::write));
	map(0x0090, 0x0093).rw(m_pia_u11, FUNC(pia6821_device::read), FUNC(pia6821_device::write));
	map(0x00a0, 0x00a7).nopw(); // to sound board
	map(0x00c0, 0x00c7).nopw(); // to sound board
	map(0x0200, 0x02ff).ram().share("nvram");
	map(0x1000, 0x1fff).rom().region("maincpu", 0);
}

static INPUT_PORTS_START( mp100 )
	PORT_START("TEST")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_0_PAD) PORT_NAME("Self Test") PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, st_mp100_state, self_test, 0)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_1_PAD) PORT_NAME("Activity") PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, st_mp100_state, activity_test, 0)

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
	PORT_DIPNAME( 0x07, 0x02, "Maximum Credits")
	PORT_DIPSETTING(    0x00, "5")
	PORT_DIPSETTING(    0x01, "10")
	PORT_DIPSETTING(    0x02, "15")
	PORT_DIPSETTING(    0x03, "20")
	PORT_DIPSETTING(    0x04, "25")
	PORT_DIPSETTING(    0x05, "30")
	PORT_DIPSETTING(    0x06, "35")
	PORT_DIPSETTING(    0x07, "40")
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
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_A) PORT_NAME("INP01")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_B) PORT_NAME("INP02")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_C) PORT_NAME("INP03")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_D) PORT_NAME("INP04")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_E) PORT_NAME("INP05")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_9) PORT_NAME("Tilt")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_X) PORT_NAME("Outhole")

	PORT_START("X1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN3 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_F) PORT_NAME("INP12")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_G) PORT_NAME("INP13")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_H) PORT_NAME("INP14")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_I) PORT_NAME("INP15")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_0) PORT_NAME("Slam Tilt")

	PORT_START("X2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_J) PORT_NAME("INP17")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_K) PORT_NAME("INP18")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_L) PORT_NAME("INP19")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_M) PORT_NAME("INP20")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_N) PORT_NAME("INP21")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_O) PORT_NAME("INP22")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_P) PORT_NAME("INP23")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_Q) PORT_NAME("INP24")

	PORT_START("X3")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_R) PORT_NAME("INP25")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_S) PORT_NAME("INP26")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_T) PORT_NAME("INP27")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_U) PORT_NAME("INP28")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_V) PORT_NAME("INP29")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_W) PORT_NAME("INP30")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_Y) PORT_NAME("INP31")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_Z) PORT_NAME("INP32")

	PORT_START("X4")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_COMMA) PORT_NAME("INP33")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_STOP) PORT_NAME("INP34")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_SLASH) PORT_NAME("INP35")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_COLON) PORT_NAME("INP36")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_QUOTE) PORT_NAME("INP37")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_ENTER) PORT_NAME("INP38")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_OPENBRACE) PORT_NAME("INP39")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_CLOSEBRACE) PORT_NAME("INP40")
INPUT_PORTS_END

static INPUT_PORTS_START( mp200 )
	PORT_START("TEST")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_0_PAD) PORT_NAME("Self Test") PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, st_mp100_state, self_test, 0)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_1_PAD) PORT_NAME("Activity") PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, st_mp100_state, activity_test, 0)

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
	PORT_DIPNAME( 0x07, 0x02, "Maximum Credits")
	PORT_DIPSETTING(    0x00, "5")
	PORT_DIPSETTING(    0x01, "10")
	PORT_DIPSETTING(    0x02, "15")
	PORT_DIPSETTING(    0x03, "20")
	PORT_DIPSETTING(    0x04, "25")
	PORT_DIPSETTING(    0x05, "30")
	PORT_DIPSETTING(    0x06, "35")
	PORT_DIPSETTING(    0x07, "40")
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

INPUT_CHANGED_MEMBER( st_mp100_state::activity_test )
{
	if(newval)
		m_maincpu->pulse_input_line(INPUT_LINE_NMI, attotime::zero);
}

INPUT_CHANGED_MEMBER( st_mp100_state::self_test )
{
	m_pia_u10->ca1_w(newval);
}

void st_mp100_state::u10_ca2_w(int state)
{
	m_u10_ca2 = state;
	if (!state)
		m_counter = 0;
}

void st_mp100_state::u10_cb2_w(int state)
{
	m_u10_cb2 = state;
	if (state)
		m_stored_lamp = m_u10a;
}

void st_mp100_state::u11_ca2_w(int state)
{
	m_io_leds[0] = state ? 0 : 1;
}

void st_mp100_state::u11_cb2_w(int state)
{
	m_u11_cb2 = state;
}

u8 st_mp100_state::u10_a_r()
{
	return m_u10a;
}

void st_mp100_state::u10_a_w(u8 data)
{
	m_u10a = data;

	// Displays
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

	// Lamps
	if ((m_stored_lamp >= 0xf0) && (m_stored_lamp < 0xff))
	{
		m_stored_lamp &= 15;  // extract lamp address
		for (u8 i = 0; i < 4; i++)
			m_io_outputs[i*16+m_stored_lamp+16] = BIT(~data, i+4);  // get strobe

		m_stored_lamp = 0xff;
	}
}

u8 st_mp100_state::u10_b_r()
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

void st_mp100_state::u10_b_w(u8 data)
{
	m_u10b = data;
}

u8 st_mp100_state::u11_a_r()
{
	return m_u11a;
}

void st_mp100_state::u11_a_w(u8 data)
{
	m_u11a = data;

	if (!m_u10_ca2)
	{
		if (BIT(data, 2))
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
}

void st_mp100_state::u11_b_w(u8 data)
{
	if (m_last_solenoid < 15)
		m_io_outputs[m_last_solenoid] = 0;

	m_u11b = data;
	if (!m_u11_cb2)
	{
		switch (data & 15)
		{
			case 0x0: // chime 10
				m_samples->start(3, 3);
				break;
			case 0x1: // chime 100
				m_samples->start(2, 2);
				break;
			case 0x2: // chime 1000
				m_samples->start(1, 1);
				break;
			case 0x3: // chime 10000
				m_samples->start(4, 4);
				break;
			case 0x4: // chime 10000
				m_samples->start(4, 4);
				break;
			case 0x5: // knocker
				m_samples->start(0, 6);
				break;
			case 0x6: // outhole
				m_samples->start(0, 5);
				break;
			// from here, vary per game
			case 0x7:
			case 0x8:
			case 0x9:
				//m_samples->start(0, 5);
				break;
			case 0xa:
				m_samples->start(0, 0);
				break;
			case 0xb:
				m_samples->start(0, 0);
				break;
			case 0xc:
				m_samples->start(0, 0);
				break;
			case 0xd:
				//m_samples->start(0, 0);
				break;
			case 0xe:
				//m_samples->start(0, 5);
				break;
			case 0xf: // not used
				break;
		}
		if ((data & 15) < 15)
			m_io_outputs[data & 15] = 1;
		m_last_solenoid = data & 15;
	}
}

void st_mp100_state::machine_start()
{
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
}

void st_mp100_state::machine_reset()
{
	m_u10a = 0;
	m_u10b = 0;
	m_u10_cb2 = 0;
	m_u11a = 0;
	m_u11b = 0;
	m_last_solenoid = 15;
	m_stored_lamp = 0xff;
	for (u8 i = 0; i < m_io_outputs.size(); i++)
		m_io_outputs[i] = 0;
	for (u8 i = 0; i < std::size(m_segment); i++)
		m_segment[i] = 0;
}

void st_mp100_state::st_mp100(machine_config &config)
{
	/* basic machine hardware */
	M6800(config, m_maincpu, 1000000); // no xtal, just 2 chips forming a random oscillator
	m_maincpu->set_addrmap(AS_PROGRAM, &st_mp100_state::mem_map);

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	/* Video */
	config.set_default_layout(layout_st_mp100);

	/* Sound */
	genpin_audio(config);

	/* Devices */
	PIA6821(config, m_pia_u10);
	m_pia_u10->readpa_handler().set(FUNC(st_mp100_state::u10_a_r));
	m_pia_u10->writepa_handler().set(FUNC(st_mp100_state::u10_a_w));
	m_pia_u10->readpb_handler().set(FUNC(st_mp100_state::u10_b_r));
	m_pia_u10->writepb_handler().set(FUNC(st_mp100_state::u10_b_w));
	m_pia_u10->ca2_handler().set(FUNC(st_mp100_state::u10_ca2_w));
	m_pia_u10->cb2_handler().set(FUNC(st_mp100_state::u10_cb2_w));
	m_pia_u10->irqa_handler().set("irq", FUNC(input_merger_device::in_w<0>));
	m_pia_u10->irqb_handler().set("irq", FUNC(input_merger_device::in_w<1>));

	clock_device &u10_clock(CLOCK(config, "u10_clock", 120)); // crosspoint detector
	u10_clock.signal_handler().set(m_pia_u10, FUNC(pia6821_device::cb1_w));

	PIA6821(config, m_pia_u11);
	m_pia_u11->readpa_handler().set(FUNC(st_mp100_state::u11_a_r));
	m_pia_u11->writepa_handler().set(FUNC(st_mp100_state::u11_a_w));
	m_pia_u11->writepb_handler().set(FUNC(st_mp100_state::u11_b_w));
	m_pia_u11->ca2_handler().set(FUNC(st_mp100_state::u11_ca2_w));
	m_pia_u11->cb2_handler().set(FUNC(st_mp100_state::u11_cb2_w));
	m_pia_u11->irqa_handler().set("irq", FUNC(input_merger_device::in_w<2>));
	m_pia_u11->irqb_handler().set("irq", FUNC(input_merger_device::in_w<3>));

	clock_device &u11_clock(CLOCK(config, "u11_clock", 634));  // NE555 astable
	u11_clock.signal_handler().set(m_pia_u11, FUNC(pia6821_device::ca1_w));

	INPUT_MERGER_ANY_HIGH(config, "irq").output_handler().set_inputline(m_maincpu, M6800_IRQ_LINE);
}

/* ========== ALTERNATE ROMS =======================================================================

This is a list of known alternate roms. Nothing has been tested. They are all modified for free play.

Cosmic Princess
ROM_LOAD( "fpcosp_2.716", 0x0000, 0x0800, CRC(7b770230) SHA1(f3aa59779a1662d3e7accb12786e14e9f344c657) )
ROM_LOAD( "fpcosp_6.716", 0x0800, 0x0800, CRC(27e45565) SHA1(55e56b12225ecccecb3c71f9c7134246436eb2d1) )

Dracula
ROM_LOAD( "fpdrac_2.716", 0x0000, 0x0800, CRC(c53b2f2b) SHA1(e722239167abf160ff1b3dcdd5971ae51d5d5d21) )
ROM_LOAD( "fpdrac_6.716", 0x0800, 0x0800, CRC(fa3c8b9a) SHA1(627f569495c86885a2c45c2f8c65bf8c433bab61) )

Hot Hand
ROM_LOAD( "fphoth_2.716", 0x0000, 0x0800, CRC(8757431b) SHA1(7ea13f2abc2b2322d08e419c66cca8cf22040e7f) )
ROM_LOAD( "fphoth_6.716", 0x0800, 0x0800, CRC(709dbde2) SHA1(c8ee72344b098e0edb16d4957627d283b1a34f50) )

Lectronamo
ROM_LOAD( "fplect_2.716", 0x0000, 0x0800, CRC(c7584530) SHA1(71d9d9081be1358af42d6ac9bfe4bdbae3d59471) )
ROM_LOAD( "fplect_6.716", 0x0800, 0x0800, CRC(11393e70) SHA1(d7a99ec82a26f8dadfef58c7d075e8e97a512d64) )

Magic
ROM_LOAD( "fpmagic2.716", 0x0000, 0x0800, CRC(7b770230) SHA1(f3aa59779a1662d3e7accb12786e14e9f344c657) )
ROM_LOAD( "fpmagic6.716", 0x0800, 0x0800, CRC(3bc9594a) SHA1(7cd3954b28bac605e7dc08b6144bfdd237d75643) )

Memory Lane
ROM_LOAD( "fpmeml_2.716", 0x0000, 0x0800, CRC(d6df6538) SHA1(b086eb84fbcb39b2e4067f9c939e526685c095d6) )
ROM_LOAD( "fpmeml_6.716", 0x0800, 0x0800, CRC(2c3bb145) SHA1(94e7888f8e1653fd4f5f01111f636944a3517ff2) )

Nugent
ROM_LOAD( "fpnugt_6.716", 0x0000, 0x0800, CRC(11393e70) SHA1(d7a99ec82a26f8dadfef58c7d075e8e97a512d64) )
ROM_LOAD( "fpnugt_2.716", 0x0800, 0x0800, CRC(c7584530) SHA1(71d9d9081be1358af42d6ac9bfe4bdbae3d59471) )

Pinball
ROM_LOAD( "fppinb_2.716", 0x0000, 0x0800, CRC(380667c1) SHA1(a2363c1795fe9b44064a9959dca2cf5aa08920df) )
ROM_LOAD( "fppinb_6.716", 0x0800, 0x0800, CRC(9fd9e2bb) SHA1(092e9b5d35dc59a7999b99998b0f240c93c7ba3c) )

Stars
ROM_LOAD( "fpstars2.716", 0x0000, 0x0800, CRC(4fe7f1aa) SHA1(9da462c0973bd9531ff38a3b6d57f69cb6a2e219) )
ROM_LOAD( "fpstars6.716", 0x0800, 0x0800, CRC(c4560fd3) SHA1(1ee8863b57ae781a6dac9ebfc92f467b60e54138) )

Stingray
ROM_LOAD( "fpstry_2.716", 0x0000, 0x0800, CRC(380667c1) SHA1(a2363c1795fe9b44064a9959dca2cf5aa08920df) )
ROM_LOAD( "fpstry_6.716", 0x0800, 0x0800, CRC(9fd9e2bb) SHA1(092e9b5d35dc59a7999b99998b0f240c93c7ba3c) )

Trident
ROM_LOAD( "fptrid_2.716", 0x0000, 0x0800, CRC(985c931b) SHA1(673a2f7aa825b2092c5b68ad1edd36e19f2ded96) )
ROM_LOAD( "fptrid_6.716", 0x0800, 0x0800, CRC(944fe5de) SHA1(8b331f03bcb761253b0e4df6a8a20e5ff33bb163) )

Wildfyre
ROM_LOAD( "fpwldf_2.716", 0x0000, 0x0800, CRC(c53b2f2b) SHA1(e722239167abf160ff1b3dcdd5971ae51d5d5d21) )
ROM_LOAD( "fpwldf_6.716", 0x0800, 0x0800, CRC(fa3c8b9a) SHA1(627f569495c86885a2c45c2f8c65bf8c433bab61) )

*/


/*--------------------------------
/ Pinball #101
/-------------------------------*/
ROM_START(pinball)
	ROM_REGION(0x1000, "maincpu", 0)
	ROM_LOAD( "cpu_u2.716", 0x0000, 0x0800, CRC(1db32a33) SHA1(2f0a3ca36968b81f29373e4f2cf7ee28a4071882))
	ROM_LOAD( "cpu_u6.716", 0x0800, 0x0800, CRC(432e9b9e) SHA1(292e509f50bc841f6e469c198fc82c2a9095f008))
ROM_END

/*------------------------------------
/ Stingray #102 - same roms as Pinball
/-------------------------------------*/
ROM_START(stingray)
	ROM_REGION(0x1000, "maincpu", 0)
	ROM_LOAD( "cpu_u2.716", 0x0000, 0x0800, CRC(1db32a33) SHA1(2f0a3ca36968b81f29373e4f2cf7ee28a4071882))
	ROM_LOAD( "cpu_u6.716", 0x0800, 0x0800, CRC(432e9b9e) SHA1(292e509f50bc841f6e469c198fc82c2a9095f008))
ROM_END

/*--------------------------------
/ Stars #103
/-------------------------------*/
ROM_START(stars)
	ROM_REGION(0x1000, "maincpu", 0)
	ROM_LOAD( "cpu_u2.716", 0x0000, 0x0800, CRC(630d05df) SHA1(2baa16265d524297332fa951d9eab3e0e8d26078))
	ROM_LOAD( "cpu_u6.716", 0x0800, 0x0800, CRC(57e63d42) SHA1(619ef955553654893c3071d8b70855fee8a5e6a7))
ROM_END

/*--------------------------------
/ Memory Lane #104
/-------------------------------*/
ROM_START(memlane)
	ROM_REGION(0x1000, "maincpu", 0)
	ROM_LOAD( "cpu_u2.716", 0x0000, 0x0800, CRC(aff1859d) SHA1(5a9801d139bf2477b6d351a2654ae07516be144a))
	ROM_LOAD( "cpu_u6.716", 0x0800, 0x0800, CRC(3e236e3c) SHA1(7f631a5fac8a1b1af3b5332ba38d52553f13531a))
ROM_END

/*--------------------------------
/ Lectronamo #105
/-------------------------------*/
ROM_START(lectrono)
	ROM_REGION(0x1000, "maincpu", 0)
	ROM_LOAD( "cpu_u2.716", 0x0000, 0x0800, CRC(79e918ff) SHA1(a728eb26d941a9c7484be593a216905237d32551))
	ROM_LOAD( "cpu_u6.716", 0x0800, 0x0800, CRC(7c6e5fb5) SHA1(3aa4e0c1f377ba024e6b34bd431a188ff02d4eaa))
ROM_END

/*--------------------------------
/ Wildfyre #106
/-------------------------------*/
ROM_START(wildfyre)
	ROM_REGION(0x1000, "maincpu", 0)
	ROM_LOAD( "cpu_u2.716", 0x0000, 0x0800, CRC(063f8b5e) SHA1(80434de549102bff829b474603d6736b839b8999))
	ROM_LOAD( "cpu_u6.716", 0x0800, 0x0800, CRC(00336fbc) SHA1(d2c360b8a80b209ecf4ec02ee19a5234c0364504))
ROM_END

/*-----------------------------------
/ Nugent #108 - same roms as lectrono
/------------------------------------*/
ROM_START(nugent)
	ROM_REGION(0x1000, "maincpu", 0)
	ROM_LOAD( "cpu_u2.716", 0x0000, 0x0800, CRC(79e918ff) SHA1(a728eb26d941a9c7484be593a216905237d32551))
	ROM_LOAD( "cpu_u6.716", 0x0800, 0x0800, CRC(7c6e5fb5) SHA1(3aa4e0c1f377ba024e6b34bd431a188ff02d4eaa))
ROM_END

/*------------------------------------
/ Dracula #109 - same roms as wildfyre
/-------------------------------------*/
ROM_START(dracula)
	ROM_REGION(0x1000, "maincpu", 0)
	ROM_LOAD( "cpu_u2.716", 0x0000, 0x0800, CRC(063f8b5e) SHA1(80434de549102bff829b474603d6736b839b8999))
	ROM_LOAD( "cpu_u6.716", 0x0800, 0x0800, CRC(00336fbc) SHA1(d2c360b8a80b209ecf4ec02ee19a5234c0364504))
ROM_END

/*--------------------------------
/ Trident #110
/-------------------------------*/
ROM_START(trident)
	ROM_REGION(0x1000, "maincpu", 0)
	ROM_LOAD( "25arom_p11a.u2",  0x0000, 0x0800, CRC(6dcd6ad3) SHA1(f748acc8628c5013b630a5c7b25a1bf72e36b16d))   // 9316A-2920
	ROM_LOAD( "25arom_p12au.u6", 0x0800, 0x0800, CRC(fb955a6f) SHA1(387080d5af318463475797fecff026d6db776a0c))   // 9316A-2921
ROM_END

ROM_START(tridento)
	ROM_REGION(0x1000, "maincpu", 0)
	ROM_LOAD( "25arom_p11.u2",  0x0000, 0x0800, CRC(934e49dd) SHA1(cbf6ca2759166f522f651825da0c75cf7248d3da))
	ROM_LOAD( "25arom_p12u.u6", 0x0800, 0x0800, CRC(540bce56) SHA1(0b21385501b83e448403e0216371487ed54026b7))
ROM_END

/*-------------------------------------
/ Cosmic Princess #111 - same ROMs as Magic
/-------------------------------------*/
ROM_START(princess)
	ROM_REGION(0x1000, "maincpu", 0)
	ROM_LOAD( "cpu_u2.716", 0x0000, 0x0800, CRC(8838091f) SHA1(d2702b5e15076793b4560c77b78eed6c1da571b6))
	ROM_LOAD( "cpu_u6.716", 0x0800, 0x0800, CRC(fb955a6f) SHA1(387080d5af318463475797fecff026d6db776a0c))
ROM_END

/*--------------------------------
/ Hot Hand #112
/-------------------------------*/
ROM_START(hothand)
	ROM_REGION(0x1000, "maincpu", 0)
	ROM_LOAD( "cpu_u2.716", 0x0000, 0x0800, CRC(5e79ea2e) SHA1(9b45c59b2076fcb3a35de1dd3ba2444ea852f149))
	ROM_LOAD( "cpu_u6.716", 0x0800, 0x0800, CRC(fb955a6f) SHA1(387080d5af318463475797fecff026d6db776a0c))
ROM_END

/*------------------------------------
/ Magic #115
/-------------------------------------*/
ROM_START(magic)
	ROM_REGION(0x1000, "maincpu", 0)
	ROM_LOAD( "cpu_u2.716", 0x0000, 0x0800, CRC(8838091f) SHA1(d2702b5e15076793b4560c77b78eed6c1da571b6))
	ROM_LOAD( "cpu_u6.716", 0x0800, 0x0800, CRC(fb955a6f) SHA1(387080d5af318463475797fecff026d6db776a0c))
ROM_END

/*----------------------------------
/ Black Sheep Squadron (Astro game #314)
/---------------------------------*/
ROM_START(blkshpsq)
	ROM_REGION(0x1000, "maincpu", 0)
	ROM_LOAD( "cpu_u2.716", 0x0000, 0x0800, CRC(23d6cd54) SHA1(301ba10f3f333109630dd8abd13a6b4063f805a9))
	ROM_LOAD( "cpu_u6.716", 0x0800, 0x0800, CRC(ea68b9f7) SHA1(ebb69f4faadf457454939e47d8ae6e79eb0e1a11))
ROM_END

} // Anonymous namespace


// chimes
GAME( 1977,  pinball,    0,      st_mp100,   mp100, st_mp100_state, empty_init, ROT0, "Stern", "Pinball",           MACHINE_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME( 1977,  stingray,   0,      st_mp100,   mp100, st_mp100_state, empty_init, ROT0, "Stern", "Stingray",          MACHINE_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME( 1978,  stars,      0,      st_mp100,   mp100, st_mp100_state, empty_init, ROT0, "Stern", "Stars",             MACHINE_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME( 1978,  memlane,    0,      st_mp100,   mp100, st_mp100_state, empty_init, ROT0, "Stern", "Memory Lane",       MACHINE_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME( 1978,  blkshpsq,   0,      st_mp100,   mp100, st_mp100_state, empty_init, ROT0, "Astro", "Black Sheep Squadron",        MACHINE_MECHANICAL | MACHINE_SUPPORTS_SAVE )

// sound unit B-521
GAME( 1978,  lectrono,   0,      st_mp100,   mp100, st_mp100_state, empty_init, ROT0, "Stern", "Lectronamo",        MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME( 1978,  wildfyre,   0,      st_mp100,   mp100, st_mp100_state, empty_init, ROT0, "Stern", "Wildfyre",          MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME( 1978,  nugent,     0,      st_mp100,   mp100, st_mp100_state, empty_init, ROT0, "Stern", "Nugent",            MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME( 1979,  dracula,    0,      st_mp100,   mp100, st_mp100_state, empty_init, ROT0, "Stern", "Dracula (Pinball)", MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )

// different inputs
GAME( 1979,  trident,    0,       st_mp100,  mp200, st_mp100_state, empty_init, ROT0, "Stern", "Trident",             MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME( 1979,  tridento,   trident, st_mp100,  mp200, st_mp100_state, empty_init, ROT0, "Stern", "Trident (Older set)", MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME( 1979,  hothand,    0,       st_mp100,  mp200, st_mp100_state, empty_init, ROT0, "Stern", "Hot Hand",            MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME( 1979,  princess,   0,       st_mp100,  mp200, st_mp100_state, empty_init, ROT0, "Stern", "Cosmic Princess",     MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME( 1979,  magic,      0,       st_mp100,  mp200, st_mp100_state, empty_init, ROT0, "Stern", "Magic",               MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
