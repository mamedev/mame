// license:BSD-3-Clause
// copyright-holders:Robbbert
/********************************************************************************************

    PINBALL
    Bally MPU AS-2518-35

There are many sound boards used, however there are only a few major types.
* AS-2888 has discrete circuitry.
* AS-3022 has a 6802/6808/6810 + PIA + AY-3-8910 + 1x4kROM.
* A084-91495-A371 same as above
* AS-3107 (Squawk & Talk) has a 6802/6808/6810 + 2xPIA + AY-3-8912 + AD558 + TMS5200 + TMS6100 + 4x1kROM.
* Cheap Squeak has 6803 + ZN429 + 2x2kROM
* Xenon (#1196) has a sound board (similar to AS-3022) and a vocalizer board, with 32k of roms.


Sound Board     Machine
---------------------------------------------
AS-2888-1       1106,1116,1119,1138,1147,1148
AS-2888-3       1152
AS-2888-4       1161,1162,1167
AS-3022-1       1173
AS-3022-2       1154
AS-3022-3       1157
AS-3022-5       1187
AS-3022-6       1178
AS-3022-7       1192
AS-3022-8       1198
AS-3022-9       1199
AS-3022-11      1210
AS-3022-12      1217
AS-3022-14      1273
AS-3022-15      1276
AS-3022-18      1311
AS-3059/3060    1196
AS-3107-1       1215
AS-3107-2       1220
AS-3107-3       1219
AS-3107-4       1222
AS-3107-5       1233
AS-3107-6       1245
AS-3107-7       1239
AS-3107-8       1248
AS-3107-9       1247
AS-3107-10      1262
AS-3107-11      1282
AS-3107-12      1283
A084-91495-A371 1371
Cheap Squeak    1390,1391,0A17,0A40,0A44,0B42
(unknown)       1370 (Centaur II - Manual has the correct cover but insides are for Centaur #1239)


ToDo:
- The Nuova Bell games don't boot
- The Bell games have major problems
- Sound
- Dips, Inputs, Solenoids vary per game
- Mechanical

*********************************************************************************************/


#include "machine/genpin.h"
#include "cpu/m6800/m6800.h"
#include "machine/6821pia.h"
#include "by35.lh"


class by35_state : public genpin_class
{
public:
	by35_state(const machine_config &mconfig, device_type type, const char *tag)
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
	{ }

	DECLARE_DRIVER_INIT(by35_6);
	DECLARE_DRIVER_INIT(by35_7);
	DECLARE_READ8_MEMBER(u10_a_r);
	DECLARE_WRITE8_MEMBER(u10_a_w);
	DECLARE_READ8_MEMBER(u10_b_r);
	DECLARE_WRITE8_MEMBER(u10_b_w);
	DECLARE_READ8_MEMBER(u11_a_r);
	DECLARE_WRITE8_MEMBER(u11_a_w);
	DECLARE_WRITE8_MEMBER(u11_b_w);
	DECLARE_WRITE_LINE_MEMBER(u10_ca2_w);
	DECLARE_WRITE_LINE_MEMBER(u10_cb2_w);
	DECLARE_WRITE_LINE_MEMBER(u11_ca2_w);
	DECLARE_WRITE_LINE_MEMBER(u11_cb2_w);
	DECLARE_INPUT_CHANGED_MEMBER(activity_test);
	DECLARE_INPUT_CHANGED_MEMBER(self_test);
	TIMER_DEVICE_CALLBACK_MEMBER(timer_x);
	TIMER_DEVICE_CALLBACK_MEMBER(u11_timer);
private:
	UINT8 m_u10a;
	UINT8 m_u10b;
	UINT8 m_u11a;
	UINT8 m_u11b;
	bool m_u10_ca2;
	bool m_u10_cb2;
	bool m_u11_cb2;
	bool m_timer_x;
	bool m_u11_timer;
	bool m_7d;
	UINT8 m_digit;
	UINT8 m_counter;
	UINT8 m_segment[5];
	virtual void machine_reset();
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
};


static ADDRESS_MAP_START( by35_map, AS_PROGRAM, 8, by35_state )
	//ADDRESS_MAP_GLOBAL_MASK(0x7fff)
	AM_RANGE(0x0000, 0x007f) AM_RAM // internal to the cpu
	AM_RANGE(0x0088, 0x008b) AM_DEVREADWRITE("pia_u10", pia6821_device, read, write)
	AM_RANGE(0x0090, 0x0093) AM_DEVREADWRITE("pia_u11", pia6821_device, read, write)
	AM_RANGE(0x0200, 0x02ff) AM_RAM AM_SHARE("nvram")
	AM_RANGE(0x1000, 0xffff) AM_ROM //AM_REGION("roms", 0 )
ADDRESS_MAP_END

static INPUT_PORTS_START( by35 )
	PORT_START("TEST")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SERVICE1 ) PORT_NAME("Self Test") PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, by35_state, self_test, 0)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_SERVICE2 ) PORT_NAME("Activity") PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, by35_state, activity_test, 0)

	PORT_START("DSW0")
	PORT_DIPNAME( 0x1f, 0x02, "Coin Slot 1")
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
	PORT_DIPNAME( 0x60, 0x40, "Award for beating high score")
	PORT_DIPSETTING(    0x00, "Nothing")
	PORT_DIPSETTING(    0x20, "1 free game")
	PORT_DIPSETTING(    0x40, "2 free games")
	PORT_DIPSETTING(    0x60, "3 free games")
	PORT_DIPNAME( 0x80, 0x00, "Melody option 1")
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
	PORT_DIPNAME( 0x60, 0x60, "Award")
	PORT_DIPSETTING(    0x00, "Nothing")
	PORT_DIPSETTING(    0x40, "Extra Ball")
	PORT_DIPSETTING(    0x60, "Free Game")
	PORT_DIPNAME( 0x80, 0x00, "Balls")
	PORT_DIPSETTING(    0x00, "3")
	PORT_DIPSETTING(    0x80, "5")

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
	PORT_DIPNAME( 0x20, 0x00, "S22 (game specific)")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x20, DEF_STR( On ))
	PORT_DIPNAME( 0x40, 0x00, "S23 (game specific)")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x40, DEF_STR( On ))
	PORT_DIPNAME( 0x80, 0x00, "S24 (game specific)")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x80, DEF_STR( On ))

	PORT_START("DSW3")
	PORT_DIPNAME( 0x0f, 0x00, "Coin Slot 2")
	PORT_DIPSETTING(    0x00, "Same as slot 1")
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_1C ))
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ))
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_3C ))
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_4C ))
	PORT_DIPSETTING(    0x05, DEF_STR( 1C_5C ))
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_6C ))
	PORT_DIPSETTING(    0x07, DEF_STR( 1C_7C ))
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_8C ))
	PORT_DIPSETTING(    0x09, DEF_STR( 1C_9C ))
	PORT_DIPSETTING(    0x0a, "1 coin 10 credits")
	PORT_DIPSETTING(    0x0b, "1 coin 11 credits")
	PORT_DIPSETTING(    0x0c, "1 coin 12 credits")
	PORT_DIPSETTING(    0x0d, "1 coin 13 credits")
	PORT_DIPSETTING(    0x0e, "1 coin 14 credits")
	PORT_DIPSETTING(    0x0f, "1 coin 15 credits")
	PORT_DIPNAME( 0x10, 0x00, "S29")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x10, DEF_STR( On ))
	PORT_DIPNAME( 0x20, 0x00, "S30")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x20, DEF_STR( On ))
	PORT_DIPNAME( 0x40, 0x00, "S31 (game specific)")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x40, DEF_STR( On ))
	PORT_DIPNAME( 0x80, 0x00, "Melody option 2")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x80, DEF_STR( On ))

	PORT_START("X0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_SLASH)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_COLON)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_QUOTE)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_BACKSLASH)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_BACKSPACE)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_TILT )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Outhole") PORT_CODE(KEYCODE_X)

	PORT_START("X1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN3 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_L)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_OPENBRACE)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_CLOSEBRACE)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_ENTER)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_TILT1 ) PORT_NAME("Slam Tilt")

	PORT_START("X2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_A)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_S)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_D)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_F)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_G)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_H)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_J)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_K)

	PORT_START("X3")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_Q)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_W)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_E)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_R)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_Y)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_U)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_I)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_O)

	PORT_START("X4")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_Z)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_C)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_V)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_B)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_N)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_M)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_COMMA)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_STOP)
INPUT_PORTS_END

INPUT_CHANGED_MEMBER( by35_state::activity_test )
{
	if(newval)
		m_maincpu->set_input_line(INPUT_LINE_NMI, PULSE_LINE);
}

INPUT_CHANGED_MEMBER( by35_state::self_test )
{
	m_pia_u10->ca1_w(newval);
}

WRITE_LINE_MEMBER( by35_state::u10_ca2_w )
{
	m_u10_ca2 = state;
	if (!state)
		m_counter = 0;
}

WRITE_LINE_MEMBER( by35_state::u10_cb2_w )
{
}

WRITE_LINE_MEMBER( by35_state::u11_ca2_w )
{
	output_set_value("led0", !state);
}

WRITE_LINE_MEMBER( by35_state::u11_cb2_w )
{
	m_u11_cb2 = state;
}

READ8_MEMBER( by35_state::u10_a_r )
{
	return m_u10a;
}

WRITE8_MEMBER( by35_state::u10_a_w )
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

READ8_MEMBER( by35_state::u10_b_r )
{
	UINT8 data = 0;

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

WRITE8_MEMBER( by35_state::u10_b_w )
{
	m_u10b = data;
}

READ8_MEMBER( by35_state::u11_a_r )
{
	return m_u11a;
}

WRITE8_MEMBER( by35_state::u11_a_w )
{
	m_u11a = data;

	if (!m_u10_ca2)
	{
		if (m_7d & BIT(data, 1))
			m_digit = 6;
		else
		if BIT(data, 2)
			m_digit = 5;
		else
		if BIT(data, 3)
			m_digit = 4;
		else
		if BIT(data, 4)
			m_digit = 3;
		else
		if BIT(data, 5)
			m_digit = 2;
		else
		if BIT(data, 6)
			m_digit = 1;
		else
		if BIT(data, 7)
			m_digit = 0;

		if (BIT(data, 0) && (m_counter > 8))
		{
			static const UINT8 patterns[16] = { 0x3f,0x06,0x5b,0x4f,0x66,0x6d,0x7d,0x07,0x7f,0x6f,0,0,0,0,0,0 }; // MC14543
			output_set_digit_value(m_digit, patterns[m_segment[0]]);
			output_set_digit_value(10+m_digit, patterns[m_segment[1]]);
			output_set_digit_value(20+m_digit, patterns[m_segment[2]]);
			output_set_digit_value(30+m_digit, patterns[m_segment[3]]);
			output_set_digit_value(40+m_digit, patterns[m_segment[4]]);
		}
	}
}

WRITE8_MEMBER( by35_state::u11_b_w )
{
	m_u11b = data;
	if (!m_u11_cb2)
	{
		switch (data & 15)
		{
			// these vary per game
			case 0x0: //
				//m_samples->start(0, 3);
				break;
			case 0x1: // chime 10
				//m_samples->start(1, 1);
				break;
			case 0x2: // chime 100
				//m_samples->start(2, 2);
				break;
			case 0x3: // chime 1000
				//m_samples->start(3, 3);
				break;
			case 0x4: // chime 10000
				//m_samples->start(0, 4);
				break;
			// these 2 are standard
			case 0x5: // knocker
				m_samples->start(0, 6);
				break;
			case 0x6: // outhole
				m_samples->start(0, 5);
				break;
			// these vary per game
			case 0x7:
			case 0x8:
			case 0x9:
				//m_samples->start(0, 5);
				break;
			case 0xa:
				//m_samples->start(0, 5);
				break;
			case 0xb:
				//m_samples->start(0, 0);
				break;
			case 0xc:
				//m_samples->start(0, 5);
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
	}
}

void by35_state::machine_reset()
{
	m_u10a = 0;
	m_u10b = 0;
	m_u10_cb2 = 0;
	m_u11a = 0;
	m_u11b = 0;
}

DRIVER_INIT_MEMBER( by35_state, by35_6 )
{
	m_7d = 0;
}

DRIVER_INIT_MEMBER( by35_state, by35_7 )
{
	m_7d = 1;
}

// zero-cross detection
TIMER_DEVICE_CALLBACK_MEMBER( by35_state::timer_x )
{
	m_timer_x ^= 1;
	m_pia_u10->cb1_w(m_timer_x);
}

// 555 timer for display refresh
TIMER_DEVICE_CALLBACK_MEMBER( by35_state::u11_timer )
{
	m_u11_timer ^= 1;
	m_pia_u11->ca1_w(m_u11_timer);
}

static MACHINE_CONFIG_START( by35, by35_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M6800, 1000000) // no xtal, just 2 chips forming a random oscillator
	MCFG_CPU_PROGRAM_MAP(by35_map)

	MCFG_NVRAM_ADD_0FILL("nvram")

	/* Video */
	MCFG_DEFAULT_LAYOUT(layout_by35)

	/* Sound */
	MCFG_FRAGMENT_ADD( genpin_audio )

	/* Devices */
	MCFG_DEVICE_ADD("pia_u10", PIA6821, 0)
	MCFG_PIA_READPA_HANDLER(READ8(by35_state, u10_a_r))
	MCFG_PIA_WRITEPA_HANDLER(WRITE8(by35_state, u10_a_w))
	MCFG_PIA_READPB_HANDLER(READ8(by35_state, u10_b_r))
	MCFG_PIA_WRITEPB_HANDLER(WRITE8(by35_state, u10_b_w))
	MCFG_PIA_CA2_HANDLER(WRITELINE(by35_state, u10_ca2_w))
	MCFG_PIA_CB2_HANDLER(WRITELINE(by35_state, u10_cb2_w))
	MCFG_PIA_IRQA_HANDLER(DEVWRITELINE("maincpu", m6800_cpu_device, irq_line))
	MCFG_PIA_IRQB_HANDLER(DEVWRITELINE("maincpu", m6800_cpu_device, irq_line))
	MCFG_TIMER_DRIVER_ADD_PERIODIC("timer_x", by35_state, timer_x, attotime::from_hz(120)) // mains freq*2

	MCFG_DEVICE_ADD("pia_u11", PIA6821, 0)
	MCFG_PIA_READPA_HANDLER(READ8(by35_state, u11_a_r))
	MCFG_PIA_WRITEPA_HANDLER(WRITE8(by35_state, u11_a_w))
	MCFG_PIA_WRITEPB_HANDLER(WRITE8(by35_state, u11_b_w))
	MCFG_PIA_CA2_HANDLER(WRITELINE(by35_state, u11_ca2_w))
	MCFG_PIA_CB2_HANDLER(WRITELINE(by35_state, u11_cb2_w))
	MCFG_PIA_IRQA_HANDLER(DEVWRITELINE("maincpu", m6800_cpu_device, irq_line))
	MCFG_PIA_IRQB_HANDLER(DEVWRITELINE("maincpu", m6800_cpu_device, irq_line))
	MCFG_TIMER_DRIVER_ADD_PERIODIC("timer_d", by35_state, u11_timer, attotime::from_hz(634)) // 555 timer*2
MACHINE_CONFIG_END


/*--------------------------------
/ Supersonic #1106
/-------------------------------*/
ROM_START(sst)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD( "741-10_1.716", 0x1000, 0x0800, CRC(5e4cd81a) SHA1(d2a4a3599ad7271cd0ddc376c31c9b2e8defa379))
	ROM_LOAD( "741-08_2.716", 0x5000, 0x0800, CRC(2789cbe6) SHA1(8230657cb5ee793354a5d4a80a9348639ec9af8f))
	ROM_LOAD( "720-30_6.716", 0x5800, 0x0800, CRC(4be8aab0) SHA1(b6ae0c4f27b7dd7fb13c0632617a2559f86f29ae))
	ROM_RELOAD( 0xf800, 0x0800)
	ROM_REGION(0x0020, "sound1", 0)
	ROM_LOAD( "729-18_3.123", 0x0000, 0x0020, CRC(7b6b7d45) SHA1(22f791bac0baab71754b2f6c00c217a342c92df5))
ROM_END

/*--------------------------------
/ Playboy #1116
/-------------------------------*/
ROM_START(playboy)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD( "743-14_1.716", 0x1000, 0x0800, CRC(5c40984a) SHA1(dea104242fcb6d604faa0f01f087bc58bd43cd9d))
	ROM_LOAD( "743-12_2.716", 0x5000, 0x0800, CRC(6fa66664) SHA1(4943220942ce74d4620eb5fbbab8f8a763f65a2e))
	ROM_LOAD( "720-30_6.716", 0x5800, 0x0800, CRC(4be8aab0) SHA1(b6ae0c4f27b7dd7fb13c0632617a2559f86f29ae))
	ROM_RELOAD( 0xf800, 0x0800)
	ROM_REGION(0x0020, "sound1", 0)
	ROM_LOAD( "729-18_3.123", 0x0000, 0x0020, CRC(7b6b7d45) SHA1(22f791bac0baab71754b2f6c00c217a342c92df5))
ROM_END

/*--------------------------------
/ Lost World #1119
/-------------------------------*/
ROM_START(lostwrlp)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD( "729-33_1.716", 0x1000, 0x0800, CRC(4ca40b95) SHA1(4b4a3fbffb0aa99dab6330e24f93605eee35ac54))
	ROM_LOAD( "729-48_2.716", 0x5000, 0x0800, CRC(963bffd8) SHA1(5144092d019132946b396fd7134866a878b3ca62))
	ROM_LOAD( "720-28_6.716", 0x5800, 0x0800, CRC(f24cce3e) SHA1(0dfeaeb5b1cf4c950ff530ee56966ac0f2257111))
	ROM_RELOAD( 0xf800, 0x0800)
	ROM_REGION(0x0020, "sound1", 0)
	ROM_LOAD( "729-18_3.123", 0x0000, 0x0020, CRC(7b6b7d45) SHA1(22f791bac0baab71754b2f6c00c217a342c92df5))
ROM_END

/*--------------------------------
/ Six Million Dollar Man #1138
/-------------------------------*/
ROM_START(smman)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD( "742-20_1.716", 0x1000, 0x0800, CRC(33e55a75) SHA1(98fbec07c9d03557654e5b67e29738c66156ec62))
	ROM_LOAD( "742-18_2.716", 0x5000, 0x0800, CRC(5365d36c) SHA1(1db651d31e28cf3fda00bef5289bb14d3b37b3c1))
	ROM_LOAD( "720-30_6.716", 0x5800, 0x0800, CRC(4be8aab0) SHA1(b6ae0c4f27b7dd7fb13c0632617a2559f86f29ae))
	ROM_RELOAD( 0xf800, 0x0800)
	ROM_REGION(0x0020, "sound1", 0)
	ROM_LOAD( "729-18_3.123", 0x0000, 0x0020, CRC(7b6b7d45) SHA1(22f791bac0baab71754b2f6c00c217a342c92df5))
ROM_END

/*----------------------------------
/ Voltan Escapes Cosmic Doom #1147
/-----------------------------------*/
ROM_START(voltan)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD( "744-03_1.716", 0x1000, 0x0800, CRC(ad2467ae) SHA1(58c4de1ea696372bce9146a4c48a296ebcb2c431))
	ROM_LOAD( "744-04_2.716", 0x5000, 0x0800, CRC(dbf58b83) SHA1(2d5e1c42fb8987eec81d89a4fe758ff0b88a1889))
	ROM_LOAD( "720-30_6.716", 0x5800, 0x0800, CRC(4be8aab0) SHA1(b6ae0c4f27b7dd7fb13c0632617a2559f86f29ae))
	ROM_RELOAD( 0xf800, 0x0800)
	ROM_REGION(0x0020, "sound1", 0)
	ROM_LOAD( "729-18_3.123", 0x0000, 0x0020, CRC(7b6b7d45) SHA1(22f791bac0baab71754b2f6c00c217a342c92df5))
ROM_END

/*--------------------------------
/ Star Trek #1148
/-------------------------------*/
ROM_START(startrep)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD( "745-11_1.716", 0x1000, 0x0800, CRC(a077efca) SHA1(6f78d9a43db0b99c3818a73a04d15aa300194a6d))
	ROM_LOAD( "745-12_2.716", 0x5000, 0x0800, CRC(f683210a) SHA1(6120909d97269d9abfcc34eef2c79b56a9cf53bc))
	ROM_LOAD( "720-30_6.716", 0x5800, 0x0800, CRC(4be8aab0) SHA1(b6ae0c4f27b7dd7fb13c0632617a2559f86f29ae))
	ROM_RELOAD( 0xf800, 0x0800)
	ROM_REGION(0x0020, "sound1", 0)
	ROM_LOAD( "729-18_3.123", 0x0000, 0x0020, CRC(7b6b7d45) SHA1(22f791bac0baab71754b2f6c00c217a342c92df5))
ROM_END

/*--------------------------------
/ Kiss #1152
/-------------------------------*/
ROM_START(kiss)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD( "746-11_1.716", 0x1000, 0x0800, CRC(78ec7fad) SHA1(b7e47ed14be08571b620de71cd5006faaddc88d5))
	ROM_LOAD( "746-14_2.716", 0x5000, 0x0800, CRC(0fc8922d) SHA1(dc6bd4d2d744df69b33ec69896cf71ac10c14a35))
	ROM_LOAD( "720-30_6.716", 0x5800, 0x0800, CRC(4be8aab0) SHA1(b6ae0c4f27b7dd7fb13c0632617a2559f86f29ae))
	ROM_RELOAD( 0xf800, 0x0800)
	ROM_REGION(0x0020, "sound1", 0)
	ROM_LOAD( "729-18_3.123", 0x0000, 0x0020, CRC(7b6b7d45) SHA1(22f791bac0baab71754b2f6c00c217a342c92df5))
ROM_END

/*--------------------------------
/ Nitro Ground Shaker #1154
/-------------------------------*/
ROM_START(ngndshkr)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD( "776-17_1.716", 0x1000, 0x0800, CRC(f2d44235) SHA1(282106767b5ec5180fa8e7eb2eb5b4766849c920))
	ROM_LOAD( "776-11_2.716", 0x5000, 0x0800, CRC(b0396b55) SHA1(2d10c4af7ecfa23b64ffb640111b582f44256fd5))
	ROM_LOAD( "720-35_6.716", 0x5800, 0x0800, CRC(78d6d289) SHA1(47c3005790119294309f12ea68b7e573f360b9ef))
	ROM_RELOAD( 0xf800, 0x0800)
	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("776-15_4.716", 0xf000, 0x0800, CRC(63c80c52) SHA1(3350919fce237b308b8f960948f70d01d312e9c0))
	ROM_RELOAD(0xf800, 0x0800)
	ROM_RELOAD(0x1000, 0x0800)
	ROM_RELOAD(0x1800, 0x0800)
ROM_END

/*--------------------------------
/ Silverball Mania #1157
/-------------------------------*/
ROM_START(slbmania)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD( "786-16_1.716", 0x1000, 0x0800, CRC(c054733f) SHA1(2699cf940ce40012e2d7554b0b130adcb2bec6d1))
	ROM_LOAD( "786-17_2.716", 0x5000, 0x0800, CRC(94af0298) SHA1(579eb0290283194d92b172f787d8a9ff54f16a07))
	ROM_LOAD( "720-35_6.716", 0x5800, 0x0800, CRC(78d6d289) SHA1(47c3005790119294309f12ea68b7e573f360b9ef))
	ROM_RELOAD( 0xf800, 0x0800)
	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("786-11_4.716", 0xf000, 0x0800, CRC(2a3641e6) SHA1(64693d424277e2aaf5fd4af33b2d348a8a455448))
	ROM_RELOAD(0xf800, 0x0800)
	ROM_RELOAD(0x1000, 0x0800)
	ROM_RELOAD(0x1800, 0x0800)
ROM_END

/*-----------------------------------
/ Harlem Globetrotters On Tour #1161
/------------------------------------*/
ROM_START(hglbtrtr)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD( "750-07_1.716", 0x1000, 0x0800, CRC(da594719) SHA1(0aaa50e7d62da64f88d82b00cf0747945be88818))
	ROM_LOAD( "750-08_2.716", 0x5000, 0x0800, CRC(3c783931) SHA1(ee260511063aff1b72e18b3bc5a5be81aecf10c9))
	ROM_LOAD( "720-35_6.716", 0x5800, 0x0800, CRC(78d6d289) SHA1(47c3005790119294309f12ea68b7e573f360b9ef))
	ROM_RELOAD( 0xf800, 0x0800)
	ROM_REGION(0x0020, "sound1", 0)
	ROM_LOAD( "729-51_3.123", 0x0000, 0x0020, CRC(6e7d3e8b) SHA1(7a93d82a05213ffa6eacfa318051414f872a701d))
ROM_END

/*--------------------------------
/ Dolly Parton #1162
/-------------------------------*/
ROM_START(dollyptn)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD( "777-10_1.716", 0x1000, 0x0800, CRC(ca88cb9a) SHA1(0deac1c02b2121635af4bd76a6695d8abc09d694))
	ROM_LOAD( "777-13_2.716", 0x5000, 0x0800, CRC(7fc93ea3) SHA1(534ac5ed34397fe622dcf7cc90eaf38a311fa871))
	ROM_LOAD( "720-35_6.716", 0x5800, 0x0800, CRC(78d6d289) SHA1(47c3005790119294309f12ea68b7e573f360b9ef))
	ROM_RELOAD( 0xf800, 0x0800)
	ROM_REGION(0x0020, "sound1", 0)
	ROM_LOAD( "729-51_3.123", 0x0000, 0x0020, CRC(6e7d3e8b) SHA1(7a93d82a05213ffa6eacfa318051414f872a701d))
ROM_END

/*--------------------------------
/ Paragon #1167
/-------------------------------*/
ROM_START(paragon)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD( "748-17_1.716", 0x1000, 0x0800, CRC(08dbdf32) SHA1(43d1380d809683e74d67b6cf57c6eb0ad248a813))
	ROM_LOAD( "748-15_2.716", 0x5000, 0x0800, CRC(26cc05c1) SHA1(6e11a0f2327dbf15f6c149ddd873d9af96597d9d))
	ROM_LOAD( "720-30_6.716", 0x5800, 0x0800, CRC(4be8aab0) SHA1(b6ae0c4f27b7dd7fb13c0632617a2559f86f29ae))
	ROM_RELOAD( 0xf800, 0x0800)
	ROM_REGION(0x0020, "sound1", 0)
	ROM_LOAD( "729-51_3.123", 0x0000, 0x0020, CRC(6e7d3e8b) SHA1(7a93d82a05213ffa6eacfa318051414f872a701d))
ROM_END

/*--------------------------------
/ Future Spa #1173
/-------------------------------*/
ROM_START(futurspa)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD( "781-07_1.716", 0x1000, 0x0800, CRC(4c716a6a) SHA1(a19ff17079b7ef0b9e6933ffc718dee0236bae10))
	ROM_LOAD( "781-09_2.716", 0x5000, 0x0800, CRC(316617ed) SHA1(749d63cefe9541885b51db89302ad8a23e8f5b0a))
	ROM_LOAD( "720-35_6.716", 0x5800, 0x0800, CRC(78d6d289) SHA1(47c3005790119294309f12ea68b7e573f360b9ef))
	ROM_RELOAD( 0xf800, 0x0800)
	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("781-02_4.716", 0xf000, 0x0800, CRC(364f7c9a) SHA1(e6a3d425317eaeba4109712c6949f11c50b82892))
	ROM_RELOAD(0xf800, 0x0800)
	ROM_RELOAD(0x1000, 0x0800)
	ROM_RELOAD(0x1800, 0x0800)
ROM_END

/*--------------------------------
/ Space Invaders #1178
/-------------------------------*/
ROM_START(spaceinv)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD( "792-10_1.716", 0x1000, 0x0800, CRC(075eba5a) SHA1(7147c2dfb6af1c39bbfb9e98f409baae10d09628))
	ROM_LOAD( "792-13_2.716", 0x5000, 0x0800, CRC(b87b9e6b) SHA1(eab787ea81409ba88e30a342564944e1fade8124))
	ROM_LOAD( "720-37_6.716", 0x5800, 0x0800, CRC(ceff6993) SHA1(bc91e7afdfc441ff47a37031f2d6caeb9ab64143))
	ROM_RELOAD( 0xf800, 0x0800)
	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("792-07_4.716", 0xf000, 0x0800, CRC(787ffd5e) SHA1(4dadad7095de27622c2120311a84555dacdc3364))
	ROM_RELOAD(0xf800, 0x0800)
	ROM_RELOAD(0x1000, 0x0800)
	ROM_RELOAD(0x1800, 0x0800)
ROM_END

/*--------------------------------
/ Rolling Stones #1187
/-------------------------------*/
ROM_START(rollston)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD( "796-17_1.716", 0x1000, 0x0800, CRC(51a826d7) SHA1(6811149c8948066b85b4018802afd409dbe8c2e1))
	ROM_LOAD( "796-18_2.716", 0x5000, 0x0800, CRC(08c75b1a) SHA1(792a535514fe4d9476914f7f61c696a7a1bdb549))
	ROM_LOAD( "720-35_6.716", 0x5800, 0x0800, CRC(78d6d289) SHA1(47c3005790119294309f12ea68b7e573f360b9ef))
	ROM_RELOAD( 0xf800, 0x0800)
	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("796-19_4.716", 0xf000, 0x0800, CRC(b740d047) SHA1(710edb6bbba0a03e4f516b501f019493a3a4033e))
	ROM_RELOAD(0xf800, 0x0800)
	ROM_RELOAD(0x1000, 0x0800)
	ROM_RELOAD(0x1800, 0x0800)
ROM_END

/*--------------------------------
/ Mystic #1192
/-------------------------------*/
ROM_START(mystic)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD( "798-03_1.716", 0x1000, 0x0800, CRC(f9c91e3b) SHA1(a3e6600b7b809cdd51a2d61b679f4f45ecf16e99))
	ROM_LOAD( "798-04_2.716", 0x5000, 0x0800, CRC(f54e5785) SHA1(425304512b70ef0f17ca9854af96cbb63c5ee33e))
	ROM_LOAD( "720-35_6.716", 0x5800, 0x0800, CRC(78d6d289) SHA1(47c3005790119294309f12ea68b7e573f360b9ef))
	ROM_RELOAD( 0xf800, 0x0800)
	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("798-05_4.716", 0xf000, 0x0800, CRC(e759e093) SHA1(e635dac4aa925804ec658e856f7830290bfbc7b8))
	ROM_RELOAD(0xf800, 0x0800)
	ROM_RELOAD(0x1000, 0x0800)
	ROM_RELOAD(0x1800, 0x0800)
ROM_END

/*--------------------------------
/ Xenon #1196
/-------------------------------*/
ROM_START(xenon)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD( "811-40_1.716", 0x1000, 0x0800, CRC(0fba871b) SHA1(52bc0ef65507f0f7422c319d0dc2059e12deab6d))
	ROM_LOAD( "811-41_2.716", 0x5000, 0x0800, CRC(1ea0d891) SHA1(98cd8cfed5c0f437d2b9423b31205f1e8b7436f9))
	ROM_LOAD( "720-40_6.732", 0x1800, 0x0800, CRC(d7aaaa03) SHA1(4e0b901645e509bcb59bf81a6ffc1612b4fb16ee))
	ROM_CONTINUE( 0x5800, 0x0800 )
	ROM_RELOAD( 0xf000, 0x1000 )
	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("811-35_4.532", 0xf000, 0x1000, CRC(e9caccbb) SHA1(e2e09ac738c48342212bf38687299876b40cecbb))
	ROM_LOAD("811-22_1.532", 0x8000, 0x1000, CRC(c49a968e) SHA1(86680e8cbb82e69c232313e5fdd7a0058b7eef13))
	ROM_LOAD("811-23_2.532", 0x9000, 0x1000, CRC(41043996) SHA1(78fa3782ee9f32d14cf41a96a60f708087e97bb9))
	ROM_LOAD("811-24_3.532", 0xa000, 0x1000, CRC(53d65542) SHA1(edb63b6d36524ae17ec40cfc02d5cf9985f0477f))
	ROM_LOAD("811-25_4.532", 0xb000, 0x1000, CRC(2c678631) SHA1(a1f9a732fdb498a71caf61ec8cf3d105cf7e114e))
	ROM_LOAD("811-26_5.532", 0xc000, 0x1000, CRC(b8e7febc) SHA1(e557b1bbbc68a6884edebe779df4529116031e00))
	ROM_LOAD("811-27_6.532", 0xd000, 0x1000, CRC(1e2a2afa) SHA1(3f4d4a562e46c162b80660eec8d9af6efe165dd6))
	ROM_LOAD("811-28_7.532", 0xe000, 0x1000, CRC(cebb4cd8) SHA1(2678ffb5e8e2fcff07f029f14a9e0bf1fb95f7bc))
ROM_END

ROM_START(xenonf)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD( "811-40_1.716", 0x1000, 0x0800, CRC(0fba871b) SHA1(52bc0ef65507f0f7422c319d0dc2059e12deab6d))
	ROM_LOAD( "811-41_2.716", 0x5000, 0x0800, CRC(1ea0d891) SHA1(98cd8cfed5c0f437d2b9423b31205f1e8b7436f9))
	ROM_LOAD( "720-40_6.732", 0x1800, 0x0800, CRC(d7aaaa03) SHA1(4e0b901645e509bcb59bf81a6ffc1612b4fb16ee))
	ROM_CONTINUE( 0x5800, 0x0800 )
	ROM_RELOAD( 0xf000, 0x1000 )
	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("811-36_4.532", 0xf000, 0x1000, CRC(73156c6e) SHA1(b0b3ecb44428c01849189adf6c86be3e95a99012))
	ROM_LOAD("811-22_1.532", 0x8000, 0x1000, CRC(c49a968e) SHA1(86680e8cbb82e69c232313e5fdd7a0058b7eef13))
	ROM_LOAD("811-23_2.532", 0x9000, 0x1000, CRC(41043996) SHA1(78fa3782ee9f32d14cf41a96a60f708087e97bb9))
	ROM_LOAD("811-24_3.532", 0xa000, 0x1000, CRC(53d65542) SHA1(edb63b6d36524ae17ec40cfc02d5cf9985f0477f))
	ROM_LOAD("811-29_4.532", 0xb000, 0x1000, CRC(e586ec31) SHA1(080d43e9a5895e95533ae73cffa4948f747ce510))
	ROM_LOAD("811-30_5.532", 0xc000, 0x1000, CRC(e48d98e3) SHA1(bb32ab96501dcd21525540a61bd5e478a35b1fef))
	ROM_LOAD("811-31_6.532", 0xd000, 0x1000, CRC(0a2336e5) SHA1(28eeb00b03b8d9eb0e6966be00dfbf3a1e13e04c))
	ROM_LOAD("811-32_7.532", 0xe000, 0x1000, CRC(987e6118) SHA1(4cded4ff715494f762d043dbcb0298111f327311))
ROM_END

/*--------------------------------
/ Viking #1198
/-------------------------------*/
ROM_START(viking)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD( "802-05_1.716", 0x1000, 0x0800, CRC(a5db0574) SHA1(d9836679ed797b649f2c1e22bc24e8a9fe1c3000))
	ROM_LOAD( "802-06_2.716", 0x5000, 0x0800, CRC(40410760) SHA1(b0b87d8600a03de7090e42f6ebdeeb5feccf87f6))
	ROM_LOAD( "720-35_6.716", 0x5800, 0x0800, CRC(78d6d289) SHA1(47c3005790119294309f12ea68b7e573f360b9ef))
	ROM_RELOAD( 0xf800, 0x0800)
	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("802-07-4.716", 0xf000, 0x0800, CRC(62bc5030) SHA1(5a696f784a415d5b16ee23cd72a905264a2bbeac))
	ROM_RELOAD(0xf800, 0x0800)
	ROM_RELOAD(0x1000, 0x0800)
	ROM_RELOAD(0x1800, 0x0800)
ROM_END

/*--------------------------------
/ Hot Doggin' #1199
/-------------------------------*/
ROM_START(hotdoggn)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD( "809-05_1.716", 0x1000, 0x0800, CRC(2744abcb) SHA1(b45bd58c365785d12f9bec381574058e29f33fd2))
	ROM_LOAD( "809-06_2.716", 0x5000, 0x0800, CRC(03db3d4d) SHA1(b8eed2d22474d2b0a1667eef2fdd4ecfa5fd35f3))
	ROM_LOAD( "720-35_6.716", 0x5800, 0x0800, CRC(78d6d289) SHA1(47c3005790119294309f12ea68b7e573f360b9ef))
	ROM_RELOAD( 0xf800, 0x0800)
	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("809-07_4.716", 0xf000, 0x0800, CRC(43f28d7f) SHA1(01fca0ee0137a0715421eaa3582ff8d324340ecf))
	ROM_RELOAD(0xf800, 0x0800)
	ROM_RELOAD(0x1000, 0x0800)
	ROM_RELOAD(0x1800, 0x0800)
ROM_END

#ifdef MISSING_GAME
ROM_START(hotdoggb) // check to see if this is the same as above but with a different split
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD( "hotd2732.u2", 0x1000, 0x0800, CRC(709305ee) SHA1(37d5e681a1a2b8b2782dae3007db3e5036003e00))
	ROM_CONTINUE( 0x5000, 0x0800)
	ROM_LOAD( "720-3532.u6b", 0x1800, 0x0800, CRC(b5e6a3d5) SHA1(fa1593eeed449dbac87965e613b501108a015eb2) )
	ROM_CONTINUE( 0x5800, 0x0800)
	ROM_RELOAD( 0xf000, 0x1000)
	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("809-07_4.716", 0xf000, 0x0800, CRC(43f28d7f) SHA1(01fca0ee0137a0715421eaa3582ff8d324340ecf))
	ROM_RELOAD(0xf800, 0x0800)
	ROM_RELOAD(0x1000, 0x0800)
	ROM_RELOAD(0x1800, 0x0800)
ROM_END
#endif

/*--------------------------------
/ Skateball #1210
/-------------------------------*/
ROM_START(skatebll)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD( "823-24_1.716", 0x1000, 0x0800, CRC(46e797d1) SHA1(7ddbf6047b8d95af8727c32b056bee1c4aa228e4))
	ROM_LOAD( "823-25_2.716", 0x5000, 0x0800, CRC(960cb8c3) SHA1(3a4499cab85d3563961b0a01c78fa1f3ba2188fe))
	ROM_LOAD( "720-40_6.732", 0x1800, 0x0800, CRC(d7aaaa03) SHA1(4e0b901645e509bcb59bf81a6ffc1612b4fb16ee))
	ROM_CONTINUE( 0x5800, 0x0800 )
	ROM_RELOAD( 0xf000, 0x1000 )
	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("823-02_4.716", 0xf000, 0x0800, CRC(d1037b20) SHA1(8784728540573be5e8ebb940ec0046b778f9413b))
	ROM_RELOAD(0xf800, 0x0800)
	ROM_RELOAD(0x1000, 0x0800)
	ROM_RELOAD(0x1800, 0x0800)
ROM_END

/*--------------------------------
/ Flash Gordon #1215
/-------------------------------*/
ROM_START(flashgdn)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD( "834-23_2.732", 0x1000, 0x0800, CRC(0c7a0d91) SHA1(1f79be15817975acbc35cb08591e2289e2eca938))
	ROM_CONTINUE( 0x5000, 0x0800)
	ROM_LOAD( "720-52_6.732", 0x1800, 0x0800, CRC(2a43d9fb) SHA1(9ff903c32b80780383578a9abaa3ef9d3bcecbc7))
	ROM_CONTINUE( 0x5800, 0x0800)
	ROM_RELOAD( 0xf000, 0x1000)
	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("834-20_2.532", 0xc000, 0x1000, CRC(2f8ced3e) SHA1(ecdeb07c31c22ec313b55774f4358a9923c5e9e7))
	ROM_LOAD("834-18_5.532", 0xf000, 0x1000, CRC(8799e80e) SHA1(f255b4e7964967c82cfc2de20ebe4b8d501e3cb0))
ROM_END

ROM_START(flashgdnf)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD( "834-23_2.732", 0x1000, 0x0800, CRC(0c7a0d91) SHA1(1f79be15817975acbc35cb08591e2289e2eca938))
	ROM_CONTINUE( 0x5000, 0x0800)
	ROM_LOAD( "720-52_6.732", 0x1800, 0x0800, CRC(2a43d9fb) SHA1(9ff903c32b80780383578a9abaa3ef9d3bcecbc7))
	ROM_CONTINUE( 0x5800, 0x0800)
	ROM_RELOAD( 0xf000, 0x1000)
	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("834-35_2.532", 0xc000, 0x1000, CRC(dff3f711) SHA1(254a5670775ecb6c347f33af8ba7c350e4cfa550))
	ROM_LOAD("834-36_5.532", 0xf000, 0x1000, CRC(18691897) SHA1(3b445e0756c07d80f14c01af5a7f87744474ae15))
ROM_END

ROM_START(flashgdnv)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD( "834-23_2.732", 0x1000, 0x0800, CRC(0c7a0d91) SHA1(1f79be15817975acbc35cb08591e2289e2eca938))
	ROM_CONTINUE( 0x5000, 0x0800)
	ROM_LOAD( "720-52_6.732", 0x1800, 0x0800, CRC(2a43d9fb) SHA1(9ff903c32b80780383578a9abaa3ef9d3bcecbc7) )
	ROM_CONTINUE( 0x5800, 0x0800)
	ROM_RELOAD( 0xf000, 0x1000)
	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("834-02_4.532", 0xf000, 0x1000, CRC(f1eb0a12) SHA1(a58567665547aacf9a1b2c39295d963527ef8696))
	ROM_LOAD("834-03_1.532", 0x8000, 0x1000, CRC(88bef6f4) SHA1(561e0bde04661b700552e4fbb6141c39f2789c99))
	ROM_LOAD("834-04_2.532", 0x9000, 0x1000, CRC(bce91475) SHA1(482b424977d73b36e2014617e3bd3deb51091c28))
	ROM_LOAD("834-05_3.532", 0xa000, 0x1000, CRC(1a4dbd99) SHA1(fa9ae0bde118a40ba9a0e9a085b30298cac0ea93))
	ROM_LOAD("834-06_4.532", 0xb000, 0x1000, CRC(983c9e9d) SHA1(aae323a39b0ec987e6b9b98e5d9b2c58b1eea1a4))
	ROM_LOAD("834-07_5.532", 0xc000, 0x1000, CRC(697f5333) SHA1(39bbff8790e394a20ef5ba3239fb1d9359be0fe5))
	ROM_LOAD("834-08_6.532", 0xd000, 0x1000, CRC(75dd195f) SHA1(fdb6f7a15cd42e1326bf6baf8fa69f6266653cef))
	ROM_LOAD("834-09_7.532", 0xe000, 0x1000, CRC(19ceabd1) SHA1(37e7780f2ba3e06462e775547278dcba1b6d2ac8))
ROM_END

/*--------------------------------
/ Frontier #1217
/-------------------------------*/
ROM_START(frontier)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD( "819-08_1.716", 0x1000, 0x0800, CRC(e2f8ce9d) SHA1(03b38486e12f1677dcabcd0f14d194c59b3bd214))
	ROM_LOAD( "819-07_2.716", 0x5000, 0x0800, CRC(af023a85) SHA1(95df232ba654293066beccbad158146259a764b7))
	ROM_LOAD( "720-40_6.732", 0x1800, 0x0800, CRC(d7aaaa03) SHA1(4e0b901645e509bcb59bf81a6ffc1612b4fb16ee))
	ROM_CONTINUE( 0x5800, 0x0800 )
	ROM_RELOAD( 0xf000, 0x1000 )
	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("819-09_4.716", 0xf000, 0x0800, CRC(a62059ca) SHA1(75e139ea2573a8c3b666c9a1024d9308da9875c7))
	ROM_RELOAD(0xf800, 0x0800)
	ROM_RELOAD(0x1000, 0x0800)
	ROM_RELOAD(0x1800, 0x0800)
ROM_END

/*--------------------------------
/ Fireball II #1219
/-------------------------------*/
ROM_START(fball_ii)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD( "839-12_2.732", 0x1000, 0x0800, CRC(45e768ad) SHA1(b706cb5f3dcfa2db54d8d15de180fcbf36b3768f))
	ROM_CONTINUE( 0x5000, 0x0800)
	ROM_LOAD( "720-52_6.732", 0x1800, 0x0800, CRC(2a43d9fb) SHA1(9ff903c32b80780383578a9abaa3ef9d3bcecbc7) )
	ROM_CONTINUE( 0x5800, 0x0800)
	ROM_RELOAD( 0xf000, 0x1000)
	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("839-01_2.532", 0xc000, 0x1000, CRC(4aa473bd) SHA1(eaa12ded76f9999d33ce0fe6198df1708e007e12))
	ROM_LOAD("839-02_5.532", 0xf000, 0x1000, CRC(8bf904ff) SHA1(de78d08bddd546abac65c2f95f1d52797e716362))
ROM_END

/*--------------------------------
/ Eight Ball Deluxe #1220
/-------------------------------*/
ROM_START(eballdlx)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD( "838-15_2.732", 0x1000, 0x0800, CRC(68d92acc) SHA1(f37b16d2953677cd779073bc3eac4b586d62fad8))
	ROM_CONTINUE( 0x5000, 0x0800)
	ROM_LOAD( "720-52_6.732", 0x1800, 0x0800, CRC(2a43d9fb) SHA1(9ff903c32b80780383578a9abaa3ef9d3bcecbc7) )
	ROM_CONTINUE( 0x5800, 0x0800)
	ROM_RELOAD( 0xf000, 0x1000)
	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("838-08_3.532", 0xd000, 0x1000, CRC(c39478d7) SHA1(8148aca7c4113921ab882da32d6d88e66abb22cc))
	ROM_LOAD("838-09_4.716", 0xe000, 0x0800, CRC(518ea89e) SHA1(a387274ef530bb57f31819733b35615a39260126))
	ROM_RELOAD(0xe800, 0x0800)
	ROM_LOAD("838-10_5.532", 0xf000, 0x1000, CRC(9c63925d) SHA1(abd1fa6308d3569e16ee10bfabce269a124d8f26))
ROM_END

ROM_START(eballd14)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD( "838-14_2.732", 0x1000, 0x0800, CRC(27eeabde) SHA1(a8f81dbb70202bdad1c9734d629e8a5c27f2a835))
	ROM_CONTINUE( 0x5000, 0x0800)
	ROM_LOAD( "720-52_6.732", 0x1800, 0x0800, CRC(2a43d9fb) SHA1(9ff903c32b80780383578a9abaa3ef9d3bcecbc7) )
	ROM_CONTINUE( 0x5800, 0x0800)
	ROM_RELOAD( 0xf000, 0x1000)
	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("838-08_3.532", 0xd000, 0x1000, CRC(c39478d7) SHA1(8148aca7c4113921ab882da32d6d88e66abb22cc))
	ROM_LOAD("838-09_4.716", 0xe000, 0x0800, CRC(518ea89e) SHA1(a387274ef530bb57f31819733b35615a39260126))
	ROM_RELOAD(0xe800, 0x0800)
	ROM_LOAD("838-10_5.532", 0xf000, 0x1000, CRC(9c63925d) SHA1(abd1fa6308d3569e16ee10bfabce269a124d8f26))
ROM_END

/*--------------------------------
/ Embryon #1222
/-------------------------------*/
ROM_START(embryon)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD( "841-06_2.732", 0x1000, 0x0800, CRC(80ab18e7) SHA1(52e5b1709e6f21919fc9efed67f51934d883dbb7))
	ROM_CONTINUE( 0x5000, 0x0800)
	ROM_LOAD( "720-52_6.732", 0x1800, 0x0800, CRC(2a43d9fb) SHA1(9ff903c32b80780383578a9abaa3ef9d3bcecbc7) )
	ROM_CONTINUE( 0x5800, 0x0800)
	ROM_RELOAD( 0xf000, 0x1000)
	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("841-01_4.716", 0xe000, 0x0800, CRC(e8b234e3) SHA1(584e553748b1c6571491150e346d815005948b68))
	ROM_RELOAD(0xe800, 0x0800)
	ROM_LOAD("841-02_5.532", 0xf000, 0x1000, CRC(9cd8c04e) SHA1(7d74d8f33a98c9832fda1054187eb7300dbf5f5e))
ROM_END

/*--------------------------------
/ Fathom #1233
/-------------------------------*/
ROM_START(fathom)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD( "842-08_2.732", 0x1000, 0x0800, CRC(1180f284) SHA1(78be1fa54faba5c5b14f580e41546be685846391))
	ROM_CONTINUE( 0x5000, 0x0800)
	ROM_LOAD( "720-53_6.732", 0x1800, 0x0800, CRC(c2e92f80) SHA1(61de956a4b6e9fb9ef2b25c01bff1fb5972284ad) )
	ROM_CONTINUE( 0x5800, 0x0800)
	ROM_RELOAD( 0xf000, 0x1000)
	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("842-01_4.532", 0xe000, 0x1000, CRC(2ac02093) SHA1(a89c1d24f4f3e1f58ca4e476f408835efb368a90))
	ROM_LOAD("842-02_5.532", 0xf000, 0x1000, CRC(736800bc) SHA1(2679d4d76e7258ad18ffe05cf333f21c35adfe0e))
ROM_END

/*--------------------------------
/ Centaur #1239
/-------------------------------*/
ROM_START(centaur)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD( "848-08_2.732", 0x1000, 0x0800, CRC(8bdcd32b) SHA1(39f64393d3a39a8172b3d80d196253aac1342f40))
	ROM_CONTINUE( 0x5000, 0x0800)
	ROM_LOAD( "720-53_6.732", 0x1800, 0x0800, CRC(c2e92f80) SHA1(61de956a4b6e9fb9ef2b25c01bff1fb5972284ad) )
	ROM_CONTINUE( 0x5800, 0x0800)
	ROM_RELOAD( 0xf000, 0x1000)
	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("848-01_3.532", 0xd000, 0x1000, CRC(88322c8a) SHA1(424fd2b107f5fbc3ab8b58e3fa8c285170b1f09a))
	ROM_LOAD("848-02_4.532", 0xe000, 0x1000, CRC(d6dbd0e4) SHA1(62e4c8c1a747c5f6a3a4bf4d0bc80b06a1f70d13))
	ROM_LOAD("848-05_5.716", 0xf000, 0x0800, CRC(cbd765ba) SHA1(bdfae28af46c805f253f02d449dd81575aa9305b))
	ROM_RELOAD(0xf800, 0x0800)
ROM_END

/*--------------------------------
/ Medusa #1245
/-------------------------------*/
ROM_START(medusa)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD( "845-16_2.732", 0x1000, 0x0800, CRC(b0fbd1ac) SHA1(e876eced0c02a2b4b3c308494e8c453074d0e561))
	ROM_CONTINUE( 0x5000, 0x0800)
	ROM_LOAD( "720-53_6.732", 0x1800, 0x0800, CRC(c2e92f80) SHA1(61de956a4b6e9fb9ef2b25c01bff1fb5972284ad) )
	ROM_CONTINUE( 0x5800, 0x0800)
	ROM_RELOAD( 0xf000, 0x1000)
	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("845-01_3.532", 0xd000, 0x1000, CRC(32200e02) SHA1(e75356a20f81a68e6b27d2fa04b8cc9b17f3976a))
	ROM_LOAD("845-02_4.532", 0xe000, 0x1000, CRC(ab95885a) SHA1(fa91cef2a244d25d408585d1e14e1ed8fdc8c845))
	ROM_LOAD("845-05_5.716", 0xf000, 0x0800, CRC(3792a812) SHA1(5c7cc43e57d8e8ded1cc109aa65c4f08052899b9))
	ROM_RELOAD(0xf800, 0x0800)
ROM_END

/*--------------------------------
/ Vector #1247
/-------------------------------*/
ROM_START(vector)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD( "858-11_2.732", 0x1000, 0x0800, CRC(323e286b) SHA1(998387900363fd46d392a931c1f092c886a23c69))
	ROM_CONTINUE( 0x5000, 0x0800)
	ROM_LOAD( "720-53_6.732", 0x1800, 0x0800, CRC(c2e92f80) SHA1(61de956a4b6e9fb9ef2b25c01bff1fb5972284ad) )
	ROM_CONTINUE( 0x5800, 0x0800)
	ROM_RELOAD( 0xf000, 0x1000)
	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("858-01_2.532", 0xc000, 0x1000, CRC(bd2edef9) SHA1(8f129016440bad5e78d4b073268e76e542b61684))
	ROM_LOAD("858-02_3.532", 0xd000, 0x1000, CRC(c592fb35) SHA1(5201824f129812c907e7d8a4600de23d95fd1eb0))
	ROM_LOAD("858-03_4.532", 0xe000, 0x1000, CRC(8661d312) SHA1(36d04d875382ff5387991d660d031c662b414698))
	ROM_LOAD("858-06_5.532", 0xf000, 0x1000, CRC(3050edf6) SHA1(e028192d9a8c17123b07566c6d73302cec07b440))
ROM_END

/*--------------------------------
/ Elektra #1248
/-------------------------------*/
ROM_START(elektra)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD( "857-04_2.732", 0x1000, 0x0800, CRC(d2476720) SHA1(372c210c4f19302ffe25722bba6bcaaa85c4b90d))
	ROM_CONTINUE( 0x5000, 0x0800)
	ROM_LOAD( "720-53_6.732", 0x1800, 0x0800, CRC(c2e92f80) SHA1(61de956a4b6e9fb9ef2b25c01bff1fb5972284ad) )
	ROM_CONTINUE( 0x5800, 0x0800)
	ROM_RELOAD( 0xf000, 0x1000)
	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("857-01_3.532", 0xd000, 0x1000, CRC(031548cc) SHA1(1f0204afd32dc07a301f404b4b064e34a83bd783))
	ROM_LOAD("857-02_4.532", 0xe000, 0x1000, CRC(efc870d9) SHA1(45132c123b3191d616e2e9372948ab66ff221228))
	ROM_LOAD("857-03_5.716", 0xf000, 0x0800, CRC(eae2c6a6) SHA1(ee3a9b01fa07e2df4eb6d2ab26da5f7f0e12475b))
	ROM_RELOAD(0xf800, 0x0800)
ROM_END

/*--------------------------------
/ Spectrum #1262
/-------------------------------*/
ROM_START(spectrm)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD( "868-00_2.732", 0x1000, 0x0800, NO_DUMP)
	ROM_CONTINUE( 0x5000, 0x0800)
	ROM_LOAD( "720-53_6.732", 0x1800, 0x0800, CRC(c2e92f80) SHA1(61de956a4b6e9fb9ef2b25c01bff1fb5972284ad) )
	ROM_CONTINUE( 0x5800, 0x0800)
	ROM_RELOAD( 0xf000, 0x1000)
	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("868-01_3.532", 0xd000, 0x1000, CRC(c3a16c66) SHA1(8c0a8b50fac0e218515b471621e80000ae475296))
	ROM_LOAD("868-02_4.532", 0xe000, 0x1000, CRC(6b441399) SHA1(aae9e805f76cd6bc264bf69dd2d57629ee58bfc2))
	ROM_LOAD("868-03_5.716", 0xf000, 0x0800, CRC(4a5ac3b8) SHA1(288feba40efd65f4eec5c0b2fcf013904e3dc24e))
	ROM_RELOAD(0xf800, 0x0800)
ROM_END

ROM_START(spectrm4)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD( "868-04_2.732", 0x1000, 0x0800, CRC(b377f5f1) SHA1(adc40204da90ef1a4470a478520b949c6ded07b5))
	ROM_CONTINUE( 0x5000, 0x0800)
	ROM_LOAD( "720-53_6.732", 0x1800, 0x0800, CRC(c2e92f80) SHA1(61de956a4b6e9fb9ef2b25c01bff1fb5972284ad) )
	ROM_CONTINUE( 0x5800, 0x0800)
	ROM_RELOAD( 0xf000, 0x1000)
	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("868-01_3.532", 0xd000, 0x1000, CRC(c3a16c66) SHA1(8c0a8b50fac0e218515b471621e80000ae475296))
	ROM_LOAD("868-02_4.532", 0xe000, 0x1000, CRC(6b441399) SHA1(aae9e805f76cd6bc264bf69dd2d57629ee58bfc2))
	ROM_LOAD("868-03_5.716", 0xf000, 0x0800, CRC(4a5ac3b8) SHA1(288feba40efd65f4eec5c0b2fcf013904e3dc24e))
	ROM_RELOAD(0xf800, 0x0800)
ROM_END

/*--------------------------------------------------
/ Speakeasy #1273
/--------------------------------------------------*/
ROM_START(speakesy)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD( "877-03_2.732", 0x1000, 0x0800, CRC(34b28bbc) SHA1(c649a04664e694cfbd6b4d496bf76f5e802d492a))
	ROM_CONTINUE( 0x5000, 0x0800)
	ROM_LOAD( "720-53_6.732", 0x1800, 0x0800, CRC(c2e92f80) SHA1(61de956a4b6e9fb9ef2b25c01bff1fb5972284ad) )
	ROM_CONTINUE( 0x5800, 0x0800)
	ROM_RELOAD( 0xf000, 0x1000)
	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("877-01_4.716", 0xf000, 0x0800, CRC(6534e826) SHA1(580653636f8d33e758e6631c9ce495f42fe3747a))
	ROM_RELOAD(0xf800, 0x0800)
	ROM_RELOAD(0x1000, 0x0800)
	ROM_RELOAD(0x1800, 0x0800)
ROM_END

ROM_START(speakesy4p)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD( "877-04_2.732", 0x1000, 0x0800, CRC(8926f2bb) SHA1(617c032ce949007d6bcb52268f17bec5a02f8651))
	ROM_CONTINUE( 0x5000, 0x0800)
	ROM_LOAD( "720-53_6.732", 0x1800, 0x0800, CRC(c2e92f80) SHA1(61de956a4b6e9fb9ef2b25c01bff1fb5972284ad) )
	ROM_CONTINUE( 0x5800, 0x0800)
	ROM_RELOAD( 0xf000, 0x1000)
	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("877-01_4.716", 0xf000, 0x0800, CRC(6534e826) SHA1(580653636f8d33e758e6631c9ce495f42fe3747a))
	ROM_RELOAD(0xf800, 0x0800)
	ROM_RELOAD(0x1000, 0x0800)
	ROM_RELOAD(0x1800, 0x0800)
ROM_END

/*---------------------------------------------------
/ BMX #1276
/----------------------------------------------------*/
ROM_START(bmx)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD( "888-03_2.732", 0x1000, 0x0800, CRC(038cf1be) SHA1(b000a3d84623db6a7644551e5e2f0d7b533acb13))
	ROM_CONTINUE( 0x5000, 0x0800)
	ROM_LOAD( "720-53_6.732", 0x1800, 0x0800, CRC(c2e92f80) SHA1(61de956a4b6e9fb9ef2b25c01bff1fb5972284ad) )
	ROM_CONTINUE( 0x5800, 0x0800)
	ROM_RELOAD( 0xf000, 0x1000)
	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("888-02_4.532", 0xf000, 0x1000, CRC(5692c679) SHA1(7eef074d16cde589cde7500c4dc76c9a902c7fe3))
	ROM_RELOAD(0x1000, 0x1000)
ROM_END

/*--------------------------------
/ Rapid Fire #1282
/-------------------------------*/
ROM_START(rapidfip)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD( "869-04_2.732", 0x1000, 0x0800, CRC(26fdf048) SHA1(15787345e7162a530334bff98d877e525d4a1295))
	ROM_CONTINUE( 0x5000, 0x0800)
	ROM_LOAD( "869-03_6.732", 0x1800, 0x0800, CRC(f6af5e8d) SHA1(3cf782d4a0ca38e3953a20d23d0eb01af87ba445) )
	ROM_CONTINUE( 0x5800, 0x0800)
	ROM_RELOAD( 0xf000, 0x1000)
	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("869-02_5.532", 0xf000, 0x1000, CRC(5a74cb86) SHA1(4fd09b0bc4257cb7b48cd8087b8b15fe768f7ddf))
ROM_END

/*--------------------------------------
/ Mr. and Mrs. Pacman #1283
/--------------------------------------*/
ROM_START(m_mpac)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD( "872-04_2.732", 0x1000, 0x0800, CRC(5e542882) SHA1(bec5f56cd5192e0a12ea1226a49a2b7d8eaaa5cf))
	ROM_CONTINUE( 0x5000, 0x0800)
	ROM_LOAD( "720-53_6.732", 0x1800, 0x0800, CRC(c2e92f80) SHA1(61de956a4b6e9fb9ef2b25c01bff1fb5972284ad) )
	ROM_CONTINUE( 0x5800, 0x0800)
	ROM_RELOAD( 0xf000, 0x1000)
	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("872-01_4.532", 0xe000, 0x1000, CRC(d21ce16d) SHA1(3ee6e2629530e7e6e4d7eac713d34c48297a1047))
	ROM_LOAD("872-03_5.532", 0xf000, 0x1000, CRC(8fcdf853) SHA1(7c6bffcd974d2684e7f2c69d926f6cabb53e2f90))
ROM_END

/*-----------------------------------------------------------
/ Grand Slam #1311
/-----------------------------------------------------------*/
ROM_START(granslam)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD( "grndslam.u2", 0x1000, 0x0800, CRC(66aea9dc) SHA1(76c017dc83a63b7f1e6035e228370219eb9c0678))
	ROM_CONTINUE( 0x5000, 0x0800)
	ROM_LOAD( "grndslam.u6", 0x1800, 0x0800, CRC(9e6ccea1) SHA1(5e158e021e0f3eed063577ae22cf5f1bc9655065) )
	ROM_CONTINUE( 0x5800, 0x0800)
	ROM_RELOAD( 0xf000, 0x1000)
	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("grndslam.u4", 0xf000, 0x1000, CRC(ac34bc38) SHA1(376ceb53cb51d250b5bc222001291b0c85e42e8a))
	ROM_RELOAD(0x1000, 0x1000)
ROM_END

ROM_START(granslam4)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD( "gr_slam.u2b", 0x1000, 0x0800, CRC(552d9423) SHA1(16b86d5b7539fd803f458f1633ecc249ef15243d))
	ROM_CONTINUE( 0x5000, 0x0800)
	ROM_LOAD( "grndslam.u6", 0x1800, 0x0800, CRC(9e6ccea1) SHA1(5e158e021e0f3eed063577ae22cf5f1bc9655065) )
	ROM_CONTINUE( 0x5800, 0x0800)
	ROM_RELOAD( 0xf000, 0x1000)
	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("grndslam.u4", 0xf000, 0x1000, CRC(ac34bc38) SHA1(376ceb53cb51d250b5bc222001291b0c85e42e8a))
	ROM_RELOAD(0x1000, 0x1000)
ROM_END


/*--------------------------------
/ Centaur II #1370
/-------------------------------*/

/*----------------------------------------------------------
/ Gold Ball #1371
/----------------------------------------------------------*/
ROM_START(goldball)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD( "gold2732.u2", 0x1000, 0x0800, CRC(3169493c) SHA1(1335fcdfb2d6970d78c636748ff419baf85ef78b))
	ROM_CONTINUE( 0x5000, 0x0800)
	ROM_LOAD( "goldball.u6", 0x1800, 0x0800, CRC(9b6e79d0) SHA1(4fcda91bbe930e6131d94964a08459e395f841af))
	ROM_CONTINUE( 0x5800, 0x0800)
	ROM_RELOAD( 0xf000, 0x1000)
	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("gb_u4.532", 0xf000, 0x1000, CRC(2dcb0315) SHA1(8cb9c9f627f0c8420d3b3d9f0d10d77a82c8be56))
	ROM_RELOAD(0x1000, 0x1000)
ROM_END

ROM_START(goldballn)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD( "u2.532", 0x1000, 0x0800, CRC(aa6eb9d6) SHA1(a73cc832450e718d9b8484e409a1f8093d91d786))
	ROM_CONTINUE( 0x5000, 0x0800)
	ROM_LOAD( "goldball.u6", 0x1800, 0x0800, CRC(9b6e79d0) SHA1(4fcda91bbe930e6131d94964a08459e395f841af) )
	ROM_CONTINUE( 0x5800, 0x0800)
	ROM_RELOAD( 0xf000, 0x1000)
	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("gb_u4.532", 0xf000, 0x1000, CRC(2dcb0315) SHA1(8cb9c9f627f0c8420d3b3d9f0d10d77a82c8be56))
	ROM_RELOAD(0x1000, 0x1000)
ROM_END

/*--------------------------------
/ Kings of Steel #1390
/-------------------------------*/
ROM_START(kosteel)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD( "kngs2732.u2", 0x1000, 0x0800, CRC(f876d8f2) SHA1(581f4b98e0a69f4ae879caeafdbf2eb979514ad1))
	ROM_CONTINUE( 0x5000, 0x0800)
	ROM_LOAD( "720-5332.u6", 0x1800, 0x0800, CRC(c2e92f80) SHA1(61de956a4b6e9fb9ef2b25c01bff1fb5972284ad) )
	ROM_CONTINUE( 0x5800, 0x0800)
	ROM_RELOAD( 0xf000, 0x1000)
	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("kngsu4.snd", 0x8000, 0x1000, CRC(f3e4d2f6) SHA1(93f4e9e1348b1225bc02db38c994e3338afb175c))
	ROM_RELOAD(0x9000, 0x1000)
	ROM_RELOAD(0xa000, 0x1000)
	ROM_RELOAD(0xb000, 0x1000)
	ROM_LOAD("kngsu3.snd", 0xc000, 0x1000, CRC(11b02dca) SHA1(464eee1aa1fd9b6e26d4ba635777fffad0222106))
	ROM_RELOAD(0xd000, 0x1000)
	ROM_RELOAD(0xe000, 0x1000)
	ROM_RELOAD(0xf000, 0x1000)
ROM_END

/*--------------------------------
/ X's & O's #1391
/-------------------------------*/
ROM_START(xsandos)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD( "x&os2732.u2", 0x1000, 0x0800, CRC(068dfe5a) SHA1(028baf79852b14cac51a7cdc8e751a8173beeccb))
	ROM_CONTINUE( 0x5000, 0x0800)
	ROM_LOAD( "720-5332.u6", 0x1800, 0x0800, CRC(c2e92f80) SHA1(61de956a4b6e9fb9ef2b25c01bff1fb5972284ad) )
	ROM_CONTINUE( 0x5800, 0x0800)
	ROM_RELOAD( 0xf000, 0x1000)
	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("720_u3.snd", 0xc000, 0x2000, CRC(5d8e2adb) SHA1(901a26f5e598386295a1298ee3a634941bd58b3e))
	ROM_RELOAD(0xe000, 0x2000)
ROM_END

/*--------------------------------
/ Spy Hunter #0A17
/-------------------------------*/
ROM_START(spyhuntr)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD( "spy-2732.u2", 0x1000, 0x0800, CRC(9e930f2d) SHA1(fb48ce0d8d8f8a695827c0eea57510b53daa7c39))
	ROM_CONTINUE( 0x5000, 0x0800)
	ROM_LOAD( "720-5332.u6", 0x1800, 0x0800, CRC(c2e92f80) SHA1(61de956a4b6e9fb9ef2b25c01bff1fb5972284ad) )
	ROM_CONTINUE( 0x5800, 0x0800)
	ROM_RELOAD( 0xf000, 0x1000)
	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("spy_u4.532", 0x8000, 0x1000, CRC(a43887d0) SHA1(6bbc55943fa9f0cd97f946767f21652e19d85265))
	ROM_RELOAD(0x9000, 0x1000)
	ROM_RELOAD(0xa000, 0x1000)
	ROM_RELOAD(0xb000, 0x1000)
	ROM_LOAD("spy_u3.532", 0xc000, 0x1000, CRC(95ffc1b8) SHA1(28f058f74abbbee120dca06f7321bcb588bef3c6))
	ROM_RELOAD(0xd000, 0x1000)
	ROM_RELOAD(0xe000, 0x1000)
	ROM_RELOAD(0xf000, 0x1000)
ROM_END

/*-------------------------------------
/ Fireball Classic #0A40
/------------------------------------*/
ROM_START(fbclass)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD( "fb-class.u2", 0x1000, 0x0800, CRC(32faac6c) SHA1(589020d09f26326dab266bc7c74ca0e10de565e6))
	ROM_CONTINUE( 0x5000, 0x0800)
	ROM_LOAD( "720-5332.u6", 0x1800, 0x0800, CRC(c2e92f80) SHA1(61de956a4b6e9fb9ef2b25c01bff1fb5972284ad) )
	ROM_CONTINUE( 0x5800, 0x0800)
	ROM_RELOAD( 0xf000, 0x1000)
	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("fbcu4.snd", 0x8000, 0x1000, CRC(697ab16f) SHA1(7beed02e6cb042f90d2048778408b1f744ffe242))
	ROM_RELOAD(0x9000, 0x1000)
	ROM_RELOAD(0xa000, 0x1000)
	ROM_RELOAD(0xb000, 0x1000)
	ROM_LOAD("fbcu3.snd", 0xc000, 0x1000, CRC(1ad71775) SHA1(ddb885730deaf315fe7f3c1803628c06eedc8350))
	ROM_RELOAD(0xd000, 0x1000)
	ROM_RELOAD(0xe000, 0x1000)
	ROM_RELOAD(0xf000, 0x1000)
ROM_END

/*--------------------------------
/ Black Pyramid #0A44
/-------------------------------*/
ROM_START(blakpyra)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD( "blkp2732.u2", 0x1000, 0x0800, CRC(600535b0) SHA1(33d080f4430ad9c33ee9de1bfbb5cfde50f0776e))
	ROM_CONTINUE( 0x5000, 0x0800)
	ROM_LOAD( "720-5332.u6", 0x1800, 0x0800, CRC(c2e92f80) SHA1(61de956a4b6e9fb9ef2b25c01bff1fb5972284ad) )
	ROM_CONTINUE( 0x5800, 0x0800)
	ROM_RELOAD( 0xf000, 0x1000)
	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("bp_u4.532", 0x8000, 0x1000, CRC(57978b4a) SHA1(4995837790d81b02325d39b548fb882a591769c5))
	ROM_RELOAD(0x9000, 0x1000)
	ROM_RELOAD(0xa000, 0x1000)
	ROM_RELOAD(0xb000, 0x1000)
	ROM_LOAD("bp_u3.532", 0xc000, 0x1000, CRC(a5005067) SHA1(bd460a20a6e8f33746880d72241d6776b85126cf))
	ROM_RELOAD(0xd000, 0x1000)
	ROM_RELOAD(0xe000, 0x1000)
	ROM_RELOAD(0xf000, 0x1000)
ROM_END

/*--------------------------------
/ Cybernaut #0B42
/-------------------------------*/
ROM_START(cybrnaut)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD( "cybe2732.u2", 0x1000, 0x0800, CRC(0610b0e0) SHA1(92f5e8a83240ad03ecc16ece4824b047b77816f7))
	ROM_CONTINUE( 0x5000, 0x0800)
	ROM_LOAD( "720-5332.u6", 0x1800, 0x0800, CRC(c2e92f80) SHA1(61de956a4b6e9fb9ef2b25c01bff1fb5972284ad) )
	ROM_CONTINUE( 0x5800, 0x0800)
	ROM_RELOAD( 0xf000, 0x1000)
	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("cybu3.snd", 0xc000, 0x2000, CRC(a3c1f6e7) SHA1(35a5e828a6f2dd9009e165328a005fa079bad6cb))
	ROM_RELOAD(0xe000, 0x2000)
ROM_END

/*--------------------------------
/ Cosmic Flash (Flash Gordon Clone)
/-------------------------------*/
ROM_START(cosflash)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD( "cf2d.532", 0x1000, 0x0800, CRC(939e941d) SHA1(889862043f351762e8c866aefb36a9ea75cbf828))
	ROM_CONTINUE( 0x5000, 0x0800)
	ROM_LOAD( "cf6d.532", 0x1800, 0x0800, CRC(7af93d2f) SHA1(2d939b14f7fe79f836e12926f44b70037630cd3f) )
	ROM_CONTINUE( 0x5800, 0x0800)
	ROM_RELOAD( 0xf000, 0x1000)
	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("834-20_2.532", 0xc000, 0x1000, CRC(2f8ced3e) SHA1(ecdeb07c31c22ec313b55774f4358a9923c5e9e7))
	ROM_LOAD("834-18_5.532", 0xf000, 0x1000, CRC(8799e80e) SHA1(f255b4e7964967c82cfc2de20ebe4b8d501e3cb0))
ROM_END

/*--------------------------------
/ Dark Rider
/-------------------------------*/

/*--------------------------------
/ Fantasy
/-------------------------------*/

/*--------------------------------
/ Fly High
/-------------------------------*/

/*--------------------------------
/ Genesis
/-------------------------------*/

/*--------------------------------
/ Miss World
/-------------------------------*/

/*--------------------------------
/ Mystic Star
/-------------------------------*/
ROM_START(myststar)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD( "rom1.bin", 0x1000, 0x0800, CRC(9a12dc91) SHA1(8961c22b2aeabac04d36d124f283409e11faee8a))
	ROM_LOAD( "rom2.bin", 0x5000, 0x0800, CRC(888ee5ae) SHA1(d99746c7c9a9a0a83b4bc15473fe9ebd3b02ffe4))
	ROM_LOAD( "rom3.bin", 0x5800, 0x0800, CRC(9e0a4619) SHA1(82065b74d39ba932704514e83d432262d360f1e1))
	ROM_RELOAD( 0xf800, 0x0800)
	ROM_REGION(0x0020, "sound1", 0)
	ROM_LOAD( "snd.123", 0x0000, 0x0020, NO_DUMP)
ROM_END

/*--------------------------------
/ New Wave (Black Pyramid Clone)
/-------------------------------*/
ROM_START(newwave)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD( "blkp2732.u2", 0x1000, 0x0800, CRC(600535b0) SHA1(33d080f4430ad9c33ee9de1bfbb5cfde50f0776e))
	ROM_CONTINUE( 0x5000, 0x0800)
	ROM_LOAD( "newwu6.532", 0x1800, 0x0800, CRC(ca72a96b) SHA1(efcd8b41bf0c19ebd7db492632e046b348619460) )
	ROM_CONTINUE( 0x5800, 0x0800)
	ROM_RELOAD( 0xf000, 0x1000)
	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("newwu4.532", 0x8000, 0x1000, CRC(6f4f2a95) SHA1(a7a375827c0429b8b3d2ee9e471f557152492993))
	ROM_RELOAD(0x9000, 0x1000)
	ROM_RELOAD(0xa000, 0x1000)
	ROM_RELOAD(0xb000, 0x1000)
	ROM_LOAD("bp_u3.532", 0xc000, 0x1000, CRC(a5005067) SHA1(bd460a20a6e8f33746880d72241d6776b85126cf))
	ROM_RELOAD(0xd000, 0x1000)
	ROM_RELOAD(0xe000, 0x1000)
	ROM_RELOAD(0xf000, 0x1000)
ROM_END

/*--------------------------------
/ Pin Ball Pool
/-------------------------------*/

/*--------------------------------
/ Pinball
/-------------------------------*/

/*--------------------------------
/ Saturn 2 (Spy Hunter Clone)
/-------------------------------*/
ROM_START(saturn2)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD( "spy-2732.u2", 0x1000, 0x0800, CRC(9e930f2d) SHA1(fb48ce0d8d8f8a695827c0eea57510b53daa7c39))
	ROM_CONTINUE( 0x5000, 0x0800)
	ROM_LOAD( "saturn2.u6", 0x1800, 0x0800, CRC(ca72a96b) SHA1(efcd8b41bf0c19ebd7db492632e046b348619460) )
	ROM_CONTINUE( 0x5800, 0x0800)
	ROM_RELOAD( 0xf000, 0x1000)
	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("spy_u4.532", 0x8000, 0x1000, CRC(a43887d0) SHA1(6bbc55943fa9f0cd97f946767f21652e19d85265))
	ROM_RELOAD(0x9000, 0x1000)
	ROM_RELOAD(0xa000, 0x1000)
	ROM_RELOAD(0xb000, 0x1000)
	ROM_LOAD("spy_u3.532", 0xc000, 0x1000, CRC(95ffc1b8) SHA1(28f058f74abbbee120dca06f7321bcb588bef3c6))
	ROM_RELOAD(0xd000, 0x1000)
	ROM_RELOAD(0xe000, 0x1000)
	ROM_RELOAD(0xf000, 0x1000)
ROM_END

/*--------------------------------
/ Sexy Girl
/-------------------------------*/

/*--------------------------------
/ Space Hawks (Cybernaut Clone)
/-------------------------------*/
ROM_START(spacehaw)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD( "cybe2732.u2g", 0x1000, 0x0800, CRC(d4a5e2f6) SHA1(841e940632993919a68c905546f533ff38a0ce31))
	ROM_CONTINUE( 0x5000, 0x0800)
	ROM_LOAD( "spacehaw.u6", 0x1800, 0x0800, CRC(b154a3a3) SHA1(d632c5eddd0582ba2ca778ab03e11ca3f6f4e1ed) )
	ROM_CONTINUE( 0x5800, 0x0800)
	ROM_RELOAD( 0xf000, 0x1000)
	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("cybu3.snd", 0xc000, 0x2000, CRC(a3c1f6e7) SHA1(35a5e828a6f2dd9009e165328a005fa079bad6cb))
	ROM_RELOAD(0xe000, 0x2000)
ROM_END

/*--------------------------------
/ Space Rider
/-------------------------------*/

/*--------------------------------
/ Super Bowl
/-------------------------------*/

/*--------------------------------
/ Tiger Rag (Kings Of Steel Clone)
/-------------------------------*/
ROM_START(tigerrag)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("tigerrag.mpu", 0xe000, 0x2000, CRC(3eb389ba) SHA1(bdfdcf00f4a2200d39d7e469fe633e0b7b8f1676))
	ROM_COPY("maincpu", 0xe000, 0x1000,0x0800)
	ROM_COPY("maincpu", 0xe800, 0x5000,0x0800)
	ROM_COPY("maincpu", 0xf000, 0x1800,0x0800)
	ROM_COPY("maincpu", 0xf800, 0x5800,0x0800)
	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("kngsu4.snd", 0x8000, 0x1000, CRC(f3e4d2f6) SHA1(93f4e9e1348b1225bc02db38c994e3338afb175c))
	ROM_RELOAD(0x9000, 0x1000)
	ROM_RELOAD(0xa000, 0x1000)
	ROM_RELOAD(0xb000, 0x1000)
	ROM_LOAD("kngsu3.snd", 0xc000, 0x1000, CRC(11b02dca) SHA1(464eee1aa1fd9b6e26d4ba635777fffad0222106))
	ROM_RELOAD(0xd000, 0x1000)
	ROM_RELOAD(0xe000, 0x1000)
	ROM_RELOAD(0xf000, 0x1000)
ROM_END

/*--------------------------------
/ Unofficial
/-------------------------------*/
/*--------------------------------
/ 301/Bulls Eye
/-------------------------------*/
ROM_START(bullseye)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("bull.u2", 0x2000, 0x0800, CRC(a2951aa2) SHA1(f9c0826c5d1d6d904286678ed90de3850a13b5f4))
	ROM_CONTINUE( 0x2800, 0x0800)
	ROM_LOAD("bull.u6", 0x3000, 0x0800, CRC(64d4b9c4) SHA1(bf4d0671372fd3a445c4c7330b9849171ca8048c))
	ROM_CONTINUE( 0x3800, 0x0800)
	ROM_RELOAD( 0xf000, 0x1000)
	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("bull.snd", 0x8000, 0x0800, CRC(c0482a2f) SHA1(a6aa698ad517cdc078129d702ee936af576260ed))
	ROM_RELOAD(0x8800, 0x0800)
	ROM_RELOAD(0xf800, 0x0800)
ROM_END

/*--------------------------------
/ World Defender
/-------------------------------*/
ROM_START(worlddef)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("worlddef.764", 0xe000, 0x2000, CRC(ad1a7ba3) SHA1(d799b6d1cd252cd6d9fb72586099c43de7c22a00))
	ROM_COPY("maincpu", 0xe000, 0x1000,0x0800)
	ROM_COPY("maincpu", 0xe800, 0x5000,0x0800)
	ROM_COPY("maincpu", 0xf000, 0x1800,0x0800)
	ROM_COPY("maincpu", 0xf800, 0x5800,0x0800)
	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("wodefsnd.764", 0xc000, 0x2000, CRC(b8d4dc20) SHA1(5aecac4a2deb7ea8e0ff0600ea459ef272dcd5f0))
	ROM_RELOAD(0xe000, 0x2000)
ROM_END

/*--------------------------------
/ Dark Shadow
/-------------------------------*/
ROM_START(darkshad)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("cpu_u7.bin", 0xe000, 0x2000, CRC(8d04c546) SHA1(951e75d9867b85a0bf9f04fe9aa647a53b6830bc))
	ROM_COPY("maincpu", 0xe000, 0x1000,0x0800)
	ROM_COPY("maincpu", 0xe800, 0x1800,0x0800)
	ROM_COPY("maincpu", 0xf000, 0x5000,0x0800)
	ROM_COPY("maincpu", 0xf800, 0x5800,0x0800)
	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("bp_u4.532", 0x8000, 0x1000, CRC(57978b4a) SHA1(4995837790d81b02325d39b548fb882a591769c5))
	ROM_RELOAD(0x9000, 0x1000)
	ROM_RELOAD(0xa000, 0x1000)
	ROM_RELOAD(0xb000, 0x1000)
	ROM_LOAD("bp_u3.532", 0xc000, 0x1000, CRC(a5005067) SHA1(bd460a20a6e8f33746880d72241d6776b85126cf))
	ROM_RELOAD(0xd000, 0x1000)
	ROM_RELOAD(0xe000, 0x1000)
	ROM_RELOAD(0xf000, 0x1000)
ROM_END

/*--------------------------------
/ Based of Nuova hardware
/-------------------------------*/

/*--------------------------------
/ Skill Flight
/-------------------------------*/
ROM_START(skflight)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("game_u7.64", 0xe000, 0x2000, CRC(fe5001eb) SHA1(f7d56d484141ba8ec82664b6aebbf3a683547d20))
	ROM_LOAD("game_u8.64", 0xc000, 0x2000, CRC(58f259fe) SHA1(505f3996f66dbb4027bd47f6b7ba9e4baaeb6e51))
	ROM_COPY("maincpu", 0xc000, 0x9000,0x1000)
	ROM_COPY("maincpu", 0xe000, 0x1000,0x1000)
	ROM_COPY("maincpu", 0xf000, 0x5000,0x1000)
	ROM_REGION(0x40000, "sound1", 0)
	ROM_LOAD("snd_u3.256", 0x0000, 0x8000, CRC(43424fb1) SHA1(428d2f7444cd71b6c49c04749b42263e3c185856))
	ROM_RELOAD(0x10000, 0x8000)
	ROM_RELOAD(0x20000, 0x8000)
	ROM_RELOAD(0x30000, 0x8000)
	ROM_LOAD("snd_u4.256", 0x8000, 0x8000, CRC(10378feb) SHA1(5da2b9c530167c80b9d411da159e4b6e95b76647))
	ROM_RELOAD(0x18000, 0x8000)
	ROM_RELOAD(0x28000, 0x8000)
	ROM_RELOAD(0x38000, 0x8000)
	ROM_REGION(0x10000, "cpu2", 0)
	ROM_COPY("sound1", 0x0000, 0x8000,0x8000)
ROM_END

/*--------------------------------
/ Cobra
/-------------------------------*/
ROM_START(cobrap)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("cpu_u7.256", 0xc000, 0x4000, CRC(c0f89577) SHA1(16d351f2bf642bf886e808b58173b3e699a44fd6))
	ROM_COPY("maincpu", 0xc000, 0x1000,0x1000)
	ROM_COPY("maincpu", 0xd000, 0x5000,0x1000)
	ROM_COPY("maincpu", 0xe000, 0x9000,0x1000)
	ROM_COPY("maincpu", 0xf000, 0xd000,0x1000)
	ROM_REGION(0x40000, "sound1", 0)
	ROM_LOAD("snd_u8.256", 0x00000,0x8000, CRC(cdf2a28d) SHA1(d4969370109b4c7f31f48a3ebd8925268caf9c44))
	ROM_RELOAD(0x20000, 0x8000)
	ROM_LOAD("snd_u9.256", 0x08000,0x8000, CRC(08bd0db9) SHA1(af851b8c993649b61645a414459000c206516bec))
	ROM_RELOAD(0x28000, 0x8000)
	ROM_LOAD("snd_u10.256",0x10000,0x8000, CRC(634bc64c) SHA1(8389fda08ee7bf0e5002153cec22e219bf786993))
	ROM_RELOAD(0x30000, 0x8000)
	ROM_LOAD("snd_u11.256",0x18000,0x8000, CRC(d4da383c) SHA1(032a4a425936d5c822fba6e46483f03a87c1a6ec))
	ROM_RELOAD(0x38000, 0x8000)
	ROM_REGION(0x10000, "cpu2", 0)
	ROM_COPY("sound1", 0x0000, 0x8000,0x8000)
ROM_END

/*--------------------------------
/ Future Queen
/-------------------------------*/
ROM_START(futrquen)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("mpu_u2.bin", 0xc000, 0x4000, CRC(bc66b636) SHA1(65f3e6461a1eca8542bbbc5b8c7cd1fca1b3011f))
	ROM_COPY("maincpu", 0xc000, 0x1000,0x0800)
	ROM_COPY("maincpu", 0xd000, 0x1800,0x0800)
	ROM_COPY("maincpu", 0xc800, 0x5000,0x0800)
	ROM_COPY("maincpu", 0xd800, 0x5800,0x0800)
	ROM_COPY("maincpu", 0xe000, 0x9000,0x0800)
	ROM_COPY("maincpu", 0xf000, 0x9800,0x0800)
	ROM_COPY("maincpu", 0xe800, 0xd000,0x0800)
	ROM_COPY("maincpu", 0xf800, 0xd800,0x0800)
	ROM_REGION(0x40000, "sound1", 0)
	ROM_LOAD("snd_u8.bin", 0x00000,0x8000, CRC(3d254d89) SHA1(2b4aa3387179e2c0fbf18684128761d3f778dcb2))
	ROM_RELOAD(0x20000, 0x8000)
	ROM_LOAD("snd_u9.bin", 0x08000,0x8000, CRC(9560f2c3) SHA1(3de6d074e2a3d3c8377fa330d4562b2d266bbfff))
	ROM_RELOAD(0x28000, 0x8000)
	ROM_LOAD("snd_u10.bin",0x10000,0x8000, CRC(70f440bc) SHA1(9fa4d33cc6174ce8f43f030487171bfbacf65537))
	ROM_RELOAD(0x30000, 0x8000)
	ROM_LOAD("snd_u11.bin",0x18000,0x8000, CRC(71d98d17) SHA1(9575b80a91a67b1644e909f70d364e0a75f73b02))
	ROM_RELOAD(0x38000, 0x8000)
	ROM_REGION(0x10000, "cpu2", 0)
	ROM_COPY("sound1", 0x0000, 0x8000,0x8000)
ROM_END
/*--------------------------------
/ F1 Grand Prix
/-------------------------------*/
ROM_START(f1gpp)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("cpu_u7", 0x8000, 0x8000, CRC(2287dea1) SHA1(5438752bf63aadaa6b6d71bbf56a72d8b67b545a))
	ROM_COPY("maincpu", 0x8000, 0x1000,0x1000)
	ROM_COPY("maincpu", 0x9000, 0x5000,0x1000)
	ROM_COPY("maincpu", 0xd000, 0x7000,0x1000)
	ROM_COPY("maincpu", 0xa000, 0x9000,0x1000)
	ROM_COPY("maincpu", 0xb000, 0xd000,0x1000)
	ROM_COPY("maincpu", 0xe000, 0xb000,0x1000)
	ROM_REGION(0x40000, "sound1", 0)
	ROM_LOAD("snd_u8a", 0x20000,0x8000, CRC(3a2af90b) SHA1(f6eeae74b3bfb1cfd9235c5214f7c029e0ad14d6))
	ROM_LOAD("snd_u8b", 0x00000,0x8000, CRC(14cddb29) SHA1(667b54174ad5dd8aa45037574916ecb4ee996a94))
	ROM_LOAD("snd_u9a", 0x28000,0x8000, CRC(681ee99c) SHA1(955cd782073a1ce0be7a427c236d47fcb9cccd20))
	ROM_LOAD("snd_u9b", 0x08000,0x8000, CRC(726920b5) SHA1(002e7a072a173836c89746cceca7e5d2ac26356d))
	ROM_LOAD("snd_u10a",0x30000,0x8000, CRC(4d3fc9bb) SHA1(d43cd134f399e128a678b86e57b1917fad70df76))
	ROM_LOAD("snd_u10b",0x10000,0x8000, CRC(9de359fb) SHA1(ce75a78dc4ed747421a386d172fa0f8a1369e860))
	ROM_LOAD("snd_u11a",0x38000,0x8000, CRC(884dc754) SHA1(b121476ea621eae7a7ba0b9a1b5e87051e1e9e3d))
	ROM_LOAD("snd_u11b",0x18000,0x8000, CRC(2394b498) SHA1(bf0884a6556a27791e7e801051be5975dd6b95c4))
	ROM_REGION(0x10000, "cpu2", 0)
	ROM_COPY("sound1", 0x0000, 0x8000,0x8000)
ROM_END

/*--------------------------------
/ Top Pin
/-------------------------------*/
ROM_START(toppin)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("cpu_256.bin", 0xc000, 0x4000, CRC(3aa32c96) SHA1(989fdc642efe6fa41319d7ccae6681ab4d76feb4))
	ROM_COPY("maincpu", 0xc000, 0x1000,0x0800)
	ROM_COPY("maincpu", 0xd000, 0x1800,0x0800)
	ROM_COPY("maincpu", 0xc800, 0x5000,0x0800)
	ROM_COPY("maincpu", 0xd800, 0x5800,0x0800)
	ROM_COPY("maincpu", 0xe000, 0x9000,0x0800)
	ROM_COPY("maincpu", 0xf000, 0x9800,0x0800)
	ROM_COPY("maincpu", 0xe800, 0xd000,0x0800)
	ROM_COPY("maincpu", 0xf800, 0xd800,0x0800)
	ROM_REGION(0x40000, "sound1", 0)
	ROM_LOAD("snd_u8.bin", 0x00000,0x8000, CRC(2cb9c931) SHA1(2537976c890ceff857b9aaf204c48ab014aad94e))
	ROM_RELOAD(0x20000, 0x8000)
	ROM_LOAD("snd_u9.bin", 0x08000,0x8000, CRC(72690344) SHA1(c2a13aa59f0c605eb616256cd288b79cceca003b))
	ROM_RELOAD(0x28000, 0x8000)
	ROM_LOAD("snd_u10.bin",0x10000,0x8000, CRC(bca9a805) SHA1(0deb3172b5c8fc91c4b02b21b1e3794ed7adef13))
	ROM_RELOAD(0x30000, 0x8000)
	ROM_LOAD("snd_u11.bin",0x18000,0x8000, CRC(1814a50d) SHA1(6fe22e774fa90725d0db9f1020bad88bae0ef85c))
	ROM_RELOAD(0x38000, 0x8000)
	ROM_REGION(0x10000, "cpu2", 0)
	ROM_COPY("sound1", 0x0000, 0x8000,0x8000)
ROM_END

/*--------------------------------
/ U-boat 65
/-------------------------------*/
ROM_START(uboat65)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("cpu_u7.256", 0x8000, 0x8000, CRC(f0fa1cbc) SHA1(4373bb37927dde01f5a4da5ef6094424909e9bc6))
	ROM_COPY("maincpu", 0x8000, 0x1000,0x1000)
	ROM_COPY("maincpu", 0x9000, 0x5000,0x1000)
	ROM_COPY("maincpu", 0xd000, 0x7000,0x1000)
	ROM_COPY("maincpu", 0xa000, 0x9000,0x1000)
	ROM_COPY("maincpu", 0xb000, 0xd000,0x1000)
	ROM_COPY("maincpu", 0xe000, 0xb000,0x1000)
	ROM_REGION(0x40000, "sound1", 0)
	ROM_LOAD("snd_ic3.256", 0x0000, 0x8000, CRC(c7811983) SHA1(7924248dcc08b05c34d3ddf2e488b778215bc7ea))
	ROM_RELOAD(0x10000, 0x8000)
	ROM_RELOAD(0x20000, 0x8000)
	ROM_RELOAD(0x30000, 0x8000)
	ROM_LOAD("snd_ic5.256", 0x8000, 0x8000, CRC(bc35e5cf) SHA1(a809b0056c576416aa76ead0437e036c2cdbd1ef))
	ROM_RELOAD(0x18000, 0x8000)
	ROM_RELOAD(0x28000, 0x8000)
	ROM_RELOAD(0x38000, 0x8000)
	ROM_REGION(0x10000, "cpu2", 0)
	ROM_COPY("sound1", 0x0000, 0x8000,0x8000)
ROM_END
/*--------------------------------
/ Big Ball Bowling (Bowler)
/-------------------------------*/
ROM_START(bbbowlin)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD( "cpu_u2.716", 0x1000, 0x0800, CRC(179e0c69) SHA1(7921839d2014a00b99ce7c44b325ea4403df9eea))
	ROM_LOAD( "cpu_u6.716", 0x1800, 0x0800, CRC(7b48e45b) SHA1(ac32292ef593bf8350e8bbc41113b6c1cb78a79e))
	ROM_RELOAD( 0xf800, 0x0800)
ROM_END

/*----------------------------
/ Stars & Strikes (Bowler)
/----------------------------*/
ROM_START(monrobwl)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD( "cpu_u1.716", 0x1000, 0x0800, CRC(42592cc9) SHA1(22452072199c4b82a413065f8dfe235a39fe3825))
	ROM_LOAD( "cpu_u5.716", 0x1800, 0x0800, CRC(78e2dcd2) SHA1(7fbe9f7adc69af5afa489d9fd953640f3466de3f))
	ROM_LOAD( "cpu_u2.716", 0x5000, 0x0800, CRC(73534680) SHA1(d5233a9d4600fa28b767ee1a251ed1a1ffbaf9c4))
	ROM_LOAD( "cpu_u6.716", 0x5800, 0x0800, CRC(ad77d719) SHA1(f8f8d0d183d639d19fea552d35a7be3aa7f07c17))
	ROM_RELOAD( 0xf800, 0x0800)
ROM_END

/*-----------------------------------------------------------------------------------------------
/ Midnight Marauders (Gun game) different hardware, not a pinball, to be moved to its own driver
/------------------------------------------------------------------------------------------------*/
ROM_START(mdntmrdr)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD( "mdru2.532", 0x1000, 0x0800, CRC(f72668bc) SHA1(25b984e1828905190c73c359ee6c9858ed1b2224))
	ROM_CONTINUE( 0x5000, 0x0800)
	ROM_LOAD( "mdru6.732", 0x1800, 0x0800, CRC(ff55fb57) SHA1(4a44fc8732c8cbce38c9605c7958b02a6bc95da1))
	ROM_CONTINUE( 0x5800, 0x0800)
	ROM_RELOAD( 0xf000, 0x1000)
	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("u3.bin", 0xd000, 0x1000, CRC(3ba474e4) SHA1(4ee5c3ad2c9dca49e9394521506e97a95e3d9a17))
	ROM_LOAD("u5.bin", 0xf000, 0x1000, CRC(3ab40e35) SHA1(63b2ee074e5993a2616e67d3383bc3d3ac51b400))
ROM_END

/*----------------------------
/ Black Beauty (Shuffle)
/----------------------------*/
ROM_START(blbeauty)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD( "cpu_u1.716", 0x1000, 0x0800, CRC(e2550957) SHA1(e445548b650fec5d593ca7da587300799ef94991))
	ROM_LOAD( "cpu_u5.716", 0x1800, 0x0800, CRC(70fcd9f7) SHA1(ca5c2ea09f45f5ba50526880c158aaac61f007d5))
	ROM_LOAD( "cpu_u2.716", 0x5000, 0x0800, CRC(3f55d17f) SHA1(e6333e53570fb05a841a7f141872c8bd14143f9c))
	ROM_LOAD( "cpu_u6.716", 0x5800, 0x0800, CRC(842cd307) SHA1(8429d84e8bc4343b437801d0236150e04de79b75))
	ROM_RELOAD( 0xf800, 0x0800)
ROM_END

/*--------------------------------
/ Super Bowl (X's & O's Clone)
/-------------------------------*/
ROM_START(suprbowl)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD( "sbowlu2.732", 0x1000, 0x0800, CRC(bc497a13) SHA1(f428373bde72f0302c45c326aebbe56e8b09c2d6))
	ROM_CONTINUE( 0x5000, 0x0800)
	ROM_LOAD( "sbowlu6.732", 0x1800, 0x0800, CRC(a9c92719) SHA1(972da0cf87863b637b88575c329f1d8162098d6f))
	ROM_CONTINUE( 0x5800, 0x0800)
	ROM_RELOAD( 0xf000, 0x1000)
	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("720_u3.snd", 0xc000, 0x2000, CRC(5d8e2adb) SHA1(901a26f5e598386295a1298ee3a634941bd58b3e))
	ROM_RELOAD(0xe000, 0x2000)
ROM_END

// AS-2888 sound
GAME( 1979, sst,        0,        by35, by35, by35_state, by35_6, ROT0, "Bally", "Supersonic", MACHINE_IS_SKELETON_MECHANICAL)
GAME( 1978, playboy,    0,        by35, by35, by35_state, by35_6, ROT0, "Bally", "Playboy", MACHINE_IS_SKELETON_MECHANICAL)
GAME( 1978, lostwrlp,   0,        by35, by35, by35_state, by35_6, ROT0, "Bally", "Lost World", MACHINE_IS_SKELETON_MECHANICAL)
GAME( 1978, smman,      0,        by35, by35, by35_state, by35_6, ROT0, "Bally", "Six Million Dollar Man", MACHINE_IS_SKELETON_MECHANICAL)
GAME( 1978, voltan,     0,        by35, by35, by35_state, by35_6, ROT0, "Bally", "Voltan Escapes Cosmic Doom", MACHINE_IS_SKELETON_MECHANICAL)
GAME( 1979, startrep,   0,        by35, by35, by35_state, by35_6, ROT0, "Bally", "Star Trek (Pinball)", MACHINE_IS_SKELETON_MECHANICAL)
GAME( 1979, kiss,       0,        by35, by35, by35_state, by35_6, ROT0, "Bally", "Kiss", MACHINE_IS_SKELETON_MECHANICAL)
GAME( 1979, hglbtrtr,   0,        by35, by35, by35_state, by35_6, ROT0, "Bally", "Harlem Globetrotters On Tour", MACHINE_IS_SKELETON_MECHANICAL)
GAME( 1979, dollyptn,   0,        by35, by35, by35_state, by35_6, ROT0, "Bally", "Dolly Parton", MACHINE_IS_SKELETON_MECHANICAL)
GAME( 1979, paragon,    0,        by35, by35, by35_state, by35_6, ROT0, "Bally", "Paragon", MACHINE_IS_SKELETON_MECHANICAL)

// AS-3022 sound
GAME( 1980, ngndshkr,   0,        by35, by35, by35_state, by35_6, ROT0, "Bally", "Nitro Ground Shaker", MACHINE_IS_SKELETON_MECHANICAL)
GAME( 1980, slbmania,   0,        by35, by35, by35_state, by35_6, ROT0, "Bally", "Silverball Mania", MACHINE_IS_SKELETON_MECHANICAL)
GAME( 1979, futurspa,   0,        by35, by35, by35_state, by35_6, ROT0, "Bally", "Future Spa", MACHINE_IS_SKELETON_MECHANICAL)
GAME( 1980, spaceinv,   0,        by35, by35, by35_state, by35_6, ROT0, "Bally", "Space Invaders", MACHINE_IS_SKELETON_MECHANICAL)
GAME( 1980, rollston,   0,        by35, by35, by35_state, by35_6, ROT0, "Bally", "Rolling Stones", MACHINE_IS_SKELETON_MECHANICAL)
GAME( 1980, mystic,     0,        by35, by35, by35_state, by35_6, ROT0, "Bally", "Mystic", MACHINE_IS_SKELETON_MECHANICAL)
GAME( 1980, xenon,      0,        by35, by35, by35_state, by35_6, ROT0, "Bally", "Xenon", MACHINE_IS_SKELETON_MECHANICAL)
GAME( 1980, xenonf,     xenon,    by35, by35, by35_state, by35_6, ROT0, "Bally", "Xenon (French)", MACHINE_IS_SKELETON_MECHANICAL)
GAME( 1980, viking,     0,        by35, by35, by35_state, by35_6, ROT0, "Bally", "Viking", MACHINE_IS_SKELETON_MECHANICAL)
GAME( 1980, hotdoggn,   0,        by35, by35, by35_state, by35_6, ROT0, "Bally", "Hotdoggin'", MACHINE_IS_SKELETON_MECHANICAL)
GAME( 1980, skatebll,   0,        by35, by35, by35_state, by35_7, ROT0, "Bally", "Skateball", MACHINE_IS_SKELETON_MECHANICAL)
GAME( 1980, frontier,   0,        by35, by35, by35_state, by35_7, ROT0, "Bally", "Frontier", MACHINE_IS_SKELETON_MECHANICAL)
GAME( 1982, speakesy,   0,        by35, by35, by35_state, by35_7, ROT0, "Bally", "Speakeasy", MACHINE_IS_SKELETON_MECHANICAL)
GAME( 1982, speakesy4p, speakesy, by35, by35, by35_state, by35_7, ROT0, "Bally", "Speakeasy 4 Player", MACHINE_IS_SKELETON_MECHANICAL)
GAME( 1983, bmx,        0,        by35, by35, by35_state, by35_7, ROT0, "Bally", "BMX", MACHINE_IS_SKELETON_MECHANICAL)
GAME( 1983, granslam,   0,        by35, by35, by35_state, by35_7, ROT0, "Bally", "Grand Slam", MACHINE_IS_SKELETON_MECHANICAL)
GAME( 1983, granslam4,  granslam, by35, by35, by35_state, by35_7, ROT0, "Bally", "Grand Slam (4 Players)", MACHINE_IS_SKELETON_MECHANICAL)
GAME( 1983, goldball,   0,        by35, by35, by35_state, by35_7, ROT0, "Bally", "Gold Ball (set 1)", MACHINE_IS_SKELETON_MECHANICAL)
GAME( 1983, goldballn,  goldball, by35, by35, by35_state, by35_7, ROT0, "Bally / Oliver", "Gold Ball (set 2)", MACHINE_IS_SKELETON_MECHANICAL)

// Squawk & Talk sound
GAME( 1981, flashgdn,   0,        by35, by35, by35_state, by35_7, ROT0, "Bally", "Flash Gordon", MACHINE_IS_SKELETON_MECHANICAL)
GAME( 1981, flashgdnf,  flashgdn, by35, by35, by35_state, by35_7, ROT0, "Bally", "Flash Gordon (French)", MACHINE_IS_SKELETON_MECHANICAL)
GAME( 1981, flashgdnv,  flashgdn, by35, by35, by35_state, by35_7, ROT0, "Bally", "Flash Gordon (Vocalizer sound)", MACHINE_IS_SKELETON_MECHANICAL)
GAME( 1981, fball_ii,   0,        by35, by35, by35_state, by35_7, ROT0, "Bally", "Fireball II", MACHINE_IS_SKELETON_MECHANICAL)
GAME( 1981, eballdlx,   0,        by35, by35, by35_state, by35_7, ROT0, "Bally", "Eight Ball Deluxe (rev. 15)", MACHINE_IS_SKELETON_MECHANICAL)
GAME( 1981, eballd14,   eballdlx, by35, by35, by35_state, by35_7, ROT0, "Bally", "Eight Ball Deluxe (rev. 14)", MACHINE_IS_SKELETON_MECHANICAL)
GAME( 1981, embryon,    0,        by35, by35, by35_state, by35_7, ROT0, "Bally", "Embryon", MACHINE_IS_SKELETON_MECHANICAL)
GAME( 1981, fathom,     0,        by35, by35, by35_state, by35_7, ROT0, "Bally", "Fathom", MACHINE_IS_SKELETON_MECHANICAL)
GAME( 1981, centaur,    0,        by35, by35, by35_state, by35_7, ROT0, "Bally", "Centaur", MACHINE_IS_SKELETON_MECHANICAL)
GAME( 1981, medusa,     0,        by35, by35, by35_state, by35_7, ROT0, "Bally", "Medusa", MACHINE_IS_SKELETON_MECHANICAL)
GAME( 1982, vector,     0,        by35, by35, by35_state, by35_7, ROT0, "Bally", "Vector", MACHINE_IS_SKELETON_MECHANICAL)
GAME( 1981, elektra,    0,        by35, by35, by35_state, by35_7, ROT0, "Bally", "Elektra", MACHINE_IS_SKELETON_MECHANICAL)
GAME( 1982, spectrm,    0,        by35, by35, by35_state, by35_7, ROT0, "Bally", "Spectrum", MACHINE_IS_SKELETON_MECHANICAL)
GAME( 1982, spectrm4,   spectrm,  by35, by35, by35_state, by35_7, ROT0, "Bally", "Spectrum (ver 4)", MACHINE_IS_SKELETON_MECHANICAL)
GAME( 1982, rapidfip,   0,        by35, by35, by35_state, by35_7, ROT0, "Bally", "Rapid Fire", MACHINE_IS_SKELETON_MECHANICAL)
GAME( 1982, m_mpac,     0,        by35, by35, by35_state, by35_7, ROT0, "Bally", "Mr. and Mrs. PacMan", MACHINE_IS_SKELETON_MECHANICAL)

// Cheap Squeak sound
GAME( 1984, kosteel,    0,        by35, by35, by35_state, by35_7, ROT0, "Bally", "Kings of Steel", MACHINE_IS_SKELETON_MECHANICAL)
GAME( 1983, xsandos,    0,        by35, by35, by35_state, by35_7, ROT0, "Bally", "X's & O's", MACHINE_IS_SKELETON_MECHANICAL)
GAME( 1984, spyhuntr,   0,        by35, by35, by35_state, by35_7, ROT0, "Bally", "Spy Hunter (Pinball)", MACHINE_IS_SKELETON_MECHANICAL)
GAME( 1984, fbclass,    0,        by35, by35, by35_state, by35_7, ROT0, "Bally", "Fireball Classic", MACHINE_IS_SKELETON_MECHANICAL)
GAME( 1984, blakpyra,   0,        by35, by35, by35_state, by35_7, ROT0, "Bally", "Black Pyramid", MACHINE_IS_SKELETON_MECHANICAL)
GAME( 1985, cybrnaut,   0,        by35, by35, by35_state, by35_7, ROT0, "Bally", "Cybernaut", MACHINE_IS_SKELETON_MECHANICAL)

// Other manufacturers
GAME( 1984, suprbowl,   xsandos,  by35, by35, by35_state, by35_7, ROT0, "Bell Games", "Super Bowl", MACHINE_IS_SKELETON_MECHANICAL)
GAME( 1984, tigerrag,   kosteel,  by35, by35, by35_state, by35_7, ROT0, "Bell Games", "Tiger Rag", MACHINE_IS_SKELETON_MECHANICAL)
GAME( 1985, cosflash,   flashgdn, by35, by35, by35_state, by35_7, ROT0, "Bell Games", "Cosmic Flash", MACHINE_IS_SKELETON_MECHANICAL)
GAME( 1985, newwave,    blakpyra, by35, by35, by35_state, by35_7, ROT0, "Bell Games", "New Wave", MACHINE_IS_SKELETON_MECHANICAL)
GAME( 1985, saturn2,    spyhuntr, by35, by35, by35_state, by35_7, ROT0, "Bell Games", "Saturn 2", MACHINE_IS_SKELETON_MECHANICAL)
GAME( 1985, worlddef,   0,        by35, by35, by35_state, by35_7, ROT0, "Bell Games", "World Defender", MACHINE_IS_SKELETON_MECHANICAL)
GAME( 1986, spacehaw,   cybrnaut, by35, by35, by35_state, by35_7, ROT0, "Nuova Bell Games", "Space Hawks", MACHINE_IS_SKELETON_MECHANICAL)
GAME( 1986, darkshad,   0,        by35, by35, by35_state, by35_7, ROT0, "Nuova Bell Games", "Dark Shadow", MACHINE_IS_SKELETON_MECHANICAL)
GAME( 1986, skflight,   0,        by35, by35, by35_state, by35_7, ROT0, "Nuova Bell Games", "Skill Flight", MACHINE_IS_SKELETON_MECHANICAL)
GAME( 1987, cobrap,     0,        by35, by35, by35_state, by35_7, ROT0, "Nuova Bell Games", "Cobra", MACHINE_IS_SKELETON_MECHANICAL)
GAME( 1987, futrquen,   0,        by35, by35, by35_state, by35_7, ROT0, "Nuova Bell Games", "Future Queen", MACHINE_IS_SKELETON_MECHANICAL)
GAME( 1987, f1gpp,      0,        by35, by35, by35_state, by35_7, ROT0, "Nuova Bell Games", "F1 Grand Prix", MACHINE_IS_SKELETON_MECHANICAL)
GAME( 1988, toppin,     0,        by35, by35, by35_state, by35_7, ROT0, "Nuova Bell Games", "Top Pin", MACHINE_IS_SKELETON_MECHANICAL)
GAME( 1988, uboat65,    0,        by35, by35, by35_state, by35_7, ROT0, "Nuova Bell Games", "U-boat 65", MACHINE_IS_SKELETON_MECHANICAL)
GAME( 1986, bullseye,   0,        by35, by35, by35_state, by35_7, ROT0, "Grand Products", "301/Bullseye", MACHINE_IS_SKELETON_MECHANICAL)
GAME( 1988, bbbowlin,   0,        by35, by35, by35_state, by35_7, ROT0, "United", "Big Ball Bowling (Bowler)", MACHINE_IS_SKELETON_MECHANICAL)
GAME( 1988, monrobwl,   0,        by35, by35, by35_state, by35_7, ROT0, "Monroe Bowling Co.", "Stars & Strikes (Bowler)", MACHINE_IS_SKELETON_MECHANICAL)
GAME( 1984, mdntmrdr,   0,        by35, by35, by35_state, by35_6, ROT0, "Bally Midway", "Midnight Marauders (Gun game)", MACHINE_IS_SKELETON_MECHANICAL)
GAME( 1988, blbeauty,   0,        by35, by35, by35_state, by35_7, ROT0, "Stern", "Black Beauty (Shuffle)", MACHINE_IS_SKELETON_MECHANICAL)
GAME( 1984, myststar,   0,        by35, by35, by35_state, by35_6, ROT0, "Zaccaria", "Mystic Star", MACHINE_IS_SKELETON_MECHANICAL)
