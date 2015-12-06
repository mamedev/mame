// license:BSD-3-Clause
// copyright-holders:Robbbert
/********************************************************************************************

    PINBALL
    Stern MP-200 MPU
    (almost identical to Bally MPU-35)


ToDo:
- Sound - All machines have a B605/C605 sound card containing a 6840 and many other chips
- Sound - Games 126,128-151,165 have a A720 voice synthesizer with a 'CRC' CPU and many other chips
- Dips, Inputs, Solenoids vary per game
- Mechanical

*********************************************************************************************/


#include "machine/genpin.h"
#include "cpu/m6800/m6800.h"
#include "machine/6821pia.h"
#include "sound/s14001a.h"
#include "st_mp200.lh"

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
	{ }

	DECLARE_DRIVER_INIT(st_mp200);
	DECLARE_DRIVER_INIT(st_mp201);
	DECLARE_DRIVER_INIT(st_mp202);
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
	bool m_su; // speech unit fitted yes/no
	bool m_7d; // 7-digit display yes/no
	UINT8 m_digit;
	UINT8 m_counter;
	UINT8 m_segment[5];
	virtual void machine_reset() override;
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
};


static ADDRESS_MAP_START( st_mp200_map, AS_PROGRAM, 8, st_mp200_state )
	//ADDRESS_MAP_GLOBAL_MASK(0x7fff)
	AM_RANGE(0x0000, 0x007f) AM_RAM // internal to the cpu
	AM_RANGE(0x0088, 0x008b) AM_DEVREADWRITE("pia_u10", pia6821_device, read, write)
	AM_RANGE(0x0090, 0x0093) AM_DEVREADWRITE("pia_u11", pia6821_device, read, write)
	AM_RANGE(0x00a0, 0x00a7) AM_WRITENOP // to sound board
	AM_RANGE(0x00c0, 0x00c7) // to sound board
	AM_RANGE(0x0200, 0x02ff) AM_RAM AM_SHARE("nvram")
	AM_RANGE(0x1000, 0xffff) AM_ROM //AM_REGION("roms", 0 )
ADDRESS_MAP_END

static INPUT_PORTS_START( mp200 )
	PORT_START("TEST")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SERVICE1 ) PORT_NAME("Self Test") PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, st_mp200_state, self_test, 0)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_SERVICE2 ) PORT_NAME("Activity") PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, st_mp200_state, activity_test, 0)

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
	PORT_DIPNAME( 0x80, 0x00, "Play melody always")
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
	PORT_DIPSETTING(    0x00, "20")
	PORT_DIPSETTING(    0x00, "25")
	PORT_DIPSETTING(    0x00, "30")
	PORT_DIPSETTING(    0x00, "35")
	PORT_DIPSETTING(    0x00, "40")
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
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_BACKSLASH)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_BACKSPACE)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_TILT )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_TILT1 ) PORT_NAME("Slam Tilt")

	PORT_START("X1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_SLASH)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_COLON)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_QUOTE)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_L)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_OPENBRACE)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_CLOSEBRACE)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_ENTER)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_Z)

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
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Outhole") PORT_CODE(KEYCODE_X)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_C)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_V)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_B)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_N)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_M)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_COMMA)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_STOP)
INPUT_PORTS_END

INPUT_CHANGED_MEMBER( st_mp200_state::activity_test )
{
	if(newval)
		m_maincpu->set_input_line(INPUT_LINE_NMI, PULSE_LINE);
}

INPUT_CHANGED_MEMBER( st_mp200_state::self_test )
{
	m_pia_u10->ca1_w(newval);
}

WRITE_LINE_MEMBER( st_mp200_state::u10_ca2_w )
{
	m_u10_ca2 = state;
	if (!state)
		m_counter = 0;
}

WRITE_LINE_MEMBER( st_mp200_state::u10_cb2_w )
{
	if (m_su)
	{
		if (m_s14001a->bsy_r())
			m_pia_u11->cb1_w(0);
		else
			m_pia_u11->cb1_w(state);
	}
}

WRITE_LINE_MEMBER( st_mp200_state::u11_ca2_w )
{
	output_set_value("led0", !state);

	if ((m_su) & (state))
	{
		if BIT(m_u10a, 7)
		{
			m_s14001a->reg_w(m_u10a & 0x3f);
			m_s14001a->rst_w(1);
			m_s14001a->rst_w(0);
		}
		else
		if BIT(m_u10a, 6)
		{
			m_s14001a->set_volume(((m_u10a & 0x38) >> 3) + 1);

			UINT8 clock_divisor = 16 - (m_u10a & 0x07);

			m_s14001a->set_clock(S14001_CLOCK / clock_divisor / 8);
		}
	}
}

WRITE_LINE_MEMBER( st_mp200_state::u11_cb2_w )
{
	m_u11_cb2 = state;
}

READ8_MEMBER( st_mp200_state::u10_a_r )
{
	return m_u10a;
}

WRITE8_MEMBER( st_mp200_state::u10_a_w )
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

READ8_MEMBER( st_mp200_state::u10_b_r )
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

WRITE8_MEMBER( st_mp200_state::u10_b_w )
{
	m_u10b = data;
}

READ8_MEMBER( st_mp200_state::u11_a_r )
{
	return m_u11a;
}

WRITE8_MEMBER( st_mp200_state::u11_a_w )
{
	m_u11a = data;

	if (!m_u10_ca2)
	{
		if (m_7d && BIT(data, 1))
		{
			m_digit = 6;
		}
		else if BIT(data, 2)
		{
			m_digit = 5;
		}
		else if BIT(data, 3)
		{
			m_digit = 4;
		}
		else if BIT(data, 4)
		{
			m_digit = 3;
		}
		else if BIT(data, 5)
		{
			m_digit = 2;
		}
		else if BIT(data, 6)
		{
			m_digit = 1;
		}
		else if BIT(data, 7)
		{
			m_digit = 0;
		}

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

WRITE8_MEMBER( st_mp200_state::u11_b_w )
{
	m_u11b = data;
	if (!m_u11_cb2)
	{
		switch (data & 15)
		{
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

void st_mp200_state::machine_reset()
{
	m_u10a = 0;
	m_u10b = 0;
	m_u10_cb2 = 0;
	m_u11a = 0;
	m_u11b = 0;
}

DRIVER_INIT_MEMBER( st_mp200_state, st_mp200 )
{
	m_7d = 1;
	m_su = 0;
}

DRIVER_INIT_MEMBER( st_mp200_state, st_mp201 )
{
	m_7d = 1;
	m_su = 1;
}

DRIVER_INIT_MEMBER( st_mp200_state, st_mp202 )
{
	m_7d = 0;
	m_su = 0;
}

// zero-cross detection
TIMER_DEVICE_CALLBACK_MEMBER( st_mp200_state::timer_x )
{
	m_timer_x ^= 1;
	m_pia_u10->cb1_w(m_timer_x);
}

// 555 timer for display refresh
TIMER_DEVICE_CALLBACK_MEMBER( st_mp200_state::u11_timer )
{
	m_u11_timer ^= 1;
	m_pia_u11->ca1_w(m_u11_timer);
}

static MACHINE_CONFIG_START( st_mp200, st_mp200_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M6800, 1000000) // no xtal, just 2 chips forming a random oscillator
	MCFG_CPU_PROGRAM_MAP(st_mp200_map)

	MCFG_NVRAM_ADD_0FILL("nvram")

	/* Video */
	MCFG_DEFAULT_LAYOUT(layout_st_mp200)

	/* Sound */
	MCFG_FRAGMENT_ADD( genpin_audio )

	/* Devices */
	MCFG_DEVICE_ADD("pia_u10", PIA6821, 0)
	MCFG_PIA_READPA_HANDLER(READ8(st_mp200_state, u10_a_r))
	MCFG_PIA_WRITEPA_HANDLER(WRITE8(st_mp200_state, u10_a_w))
	MCFG_PIA_READPB_HANDLER(READ8(st_mp200_state, u10_b_r))
	MCFG_PIA_WRITEPB_HANDLER(WRITE8(st_mp200_state, u10_b_w))
	MCFG_PIA_CA2_HANDLER(WRITELINE(st_mp200_state, u10_ca2_w))
	MCFG_PIA_CB2_HANDLER(WRITELINE(st_mp200_state, u10_cb2_w))
	MCFG_PIA_IRQA_HANDLER(DEVWRITELINE("maincpu", m6800_cpu_device, irq_line))
	MCFG_PIA_IRQB_HANDLER(DEVWRITELINE("maincpu", m6800_cpu_device, irq_line))
	MCFG_TIMER_DRIVER_ADD_PERIODIC("timer_x", st_mp200_state, timer_x, attotime::from_hz(120)) // mains freq*2

	MCFG_DEVICE_ADD("pia_u11", PIA6821, 0)
	MCFG_PIA_READPA_HANDLER(READ8(st_mp200_state, u11_a_r))
	MCFG_PIA_WRITEPA_HANDLER(WRITE8(st_mp200_state, u11_a_w))
	MCFG_PIA_WRITEPB_HANDLER(WRITE8(st_mp200_state, u11_b_w))
	MCFG_PIA_CA2_HANDLER(WRITELINE(st_mp200_state, u11_ca2_w))
	MCFG_PIA_CB2_HANDLER(WRITELINE(st_mp200_state, u11_cb2_w))
	MCFG_PIA_IRQA_HANDLER(DEVWRITELINE("maincpu", m6800_cpu_device, irq_line))
	MCFG_PIA_IRQB_HANDLER(DEVWRITELINE("maincpu", m6800_cpu_device, irq_line))
	MCFG_TIMER_DRIVER_ADD_PERIODIC("timer_d", st_mp200_state, u11_timer, attotime::from_hz(634)) // 555 timer*2
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( st_mp201, st_mp200 )
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("speech", S14001A, S14001_CLOCK)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)
MACHINE_CONFIG_END


/*--------------------------------
/ Meteor #113
/-------------------------------*/
ROM_START(meteorp)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD( "cpu_u1.716", 0x1000, 0x0800, CRC(e0fd8452) SHA1(a13215378a678e26a565742d81fdadd2e161ba7a))
	ROM_LOAD( "cpu_u5.716", 0x1800, 0x0800, CRC(43a46997) SHA1(2c74ca10cf9091db10542960f499f39f3da277ee))
	ROM_LOAD( "cpu_u2.716", 0x5000, 0x0800, CRC(fd396792) SHA1(b5d051a7ce7e7c2f9c4a0d900cef4f9ef2089476))
	ROM_LOAD( "cpu_u6.716", 0x5800, 0x0800, CRC(03fa346c) SHA1(51c04123cb433e90920c241e2d1f89db4643427b))
	ROM_RELOAD( 0xf800, 0x0800)
ROM_END

/*--------------------------------
/ Galaxy #114
/-------------------------------*/
ROM_START(galaxypi)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD( "cpu_u1.716", 0x1000, 0x0800, CRC(35656b67) SHA1(e1ad9456c561d19220f8607576cb505588512179))
	ROM_LOAD( "cpu_u5.716", 0x1800, 0x0800, CRC(12be0601) SHA1(d651b834348c071dda660f37b4e359bf01cbd8d3))
	ROM_LOAD( "cpu_u2.716", 0x5000, 0x0800, CRC(08bdb285) SHA1(7984835ac151e5dac05628f3d5146d20e3623c38))
	ROM_LOAD( "cpu_u6.716", 0x5800, 0x0800, CRC(ad846a42) SHA1(303c9cb933ca60d35e12793a4ac0cf7ef11bc92e))
	ROM_RELOAD( 0xf800, 0x0800)
ROM_END

/*--------------------------------
/ Cheetah #116
/-------------------------------*/
ROM_START(cheetah)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD( "cpu_u1.716", 0x1000, 0x0800, CRC(6a845d94) SHA1(c272d5895edf2270f5f06fc33345bb4911abbee4))
	ROM_LOAD( "cpu_u5.716", 0x1800, 0x0800, CRC(e7bdbe6c) SHA1(8b213c2271dbd5157e0d34a33672130b935d76be))
	ROM_LOAD( "cpu_u2.716", 0x5000, 0x0800, CRC(a827a1a1) SHA1(723ebf193b5ce7b19df70e83caa9bb80f2e3fa66))
	ROM_LOAD( "cpu_u6.716", 0x5800, 0x0800, CRC(ed33c227) SHA1(a96ba2814cef7663728bb5fdea2dc6ecfa219038))
	ROM_RELOAD( 0xf800, 0x0800)
ROM_END

/*--------------------------------
/ Quicksilver #117
/-------------------------------*/
ROM_START(quicksil)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD( "cpu_u1.716", 0x1000, 0x0800, CRC(fc1bd20a) SHA1(e3c547f996dfc5d1567223d234443cf31d648ef6))
	ROM_LOAD( "cpu_u5.716", 0x1800, 0x0800, CRC(0bcaceb4) SHA1(461d2fe5772a5ac84d31a4a186b9f639c683ca8a))
	ROM_LOAD( "cpu_u2.716", 0x5000, 0x0800, CRC(8cb01165) SHA1(b42e2ccce2c20ad570cdcdb63c9d12e414f9b255))
	ROM_LOAD( "cpu_u6.716", 0x5800, 0x0800, CRC(8c0e336a) SHA1(8d3a5b7c07d03c7e2945ea60c72f9181d3ee2a14))
	ROM_RELOAD( 0xf800, 0x0800)
ROM_END

/*--------------------------------
/ Ali #119
/-------------------------------*/
ROM_START(ali)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD( "cpu_u1.716", 0x1000, 0x0800, CRC(92e75b40) SHA1(bace68db0ea12d50a546157d11084f3b00949136))
	ROM_LOAD( "cpu_u5.716", 0x1800, 0x0800, CRC(119a4300) SHA1(e913d9bd399b90502efe110c8bf7f23ae07df276))
	ROM_LOAD( "cpu_u2.716", 0x5000, 0x0800, CRC(9c91d08f) SHA1(a3e8c8e8c2c8b03d86b36eea8c84e5c0a27b8444))
	ROM_LOAD( "cpu_u6.716", 0x5800, 0x0800, CRC(7629db56) SHA1(f922d31ec4dd1755da0a24bec4e3fa3a7a9b22fc))
	ROM_RELOAD( 0xf800, 0x0800)
ROM_END


/*--------------------------------
/ Big Game #121
/-------------------------------*/
ROM_START(biggame)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD( "cpu_u1.716", 0x1000, 0x0800, CRC(f59c7514) SHA1(49ab034a21e70956f63327aec4cbae115cd66a66))
	ROM_LOAD( "cpu_u5.716", 0x1800, 0x0800, CRC(57df1dc5) SHA1(283f45879b76d56ba0db0fb3d9d9771f91a70d02))
	ROM_LOAD( "cpu_u2.716", 0x5000, 0x0800, CRC(0251039b) SHA1(0a0e662788cf012dfb773d200c542a2a363748a8))
	ROM_LOAD( "cpu_u6.716", 0x5800, 0x0800, CRC(801e9a66) SHA1(8634d6bd4af3e5ec3b736679393462961b76ede1))
	ROM_RELOAD( 0xf800, 0x0800)
ROM_END

/*--------------------------------
/ Seawitch #123
/-------------------------------*/
ROM_START(seawitch)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD( "cpu_u1.716", 0x1000, 0x0800, CRC(c214140b) SHA1(4d68ddd3b0f051c5f601ea5b9d5d5195d6017304))
	ROM_LOAD( "cpu_u5.716", 0x1800, 0x0800, CRC(ab2eab3a) SHA1(80a8c1ccd554be279720a26466bd6c59e1e56df0))
	ROM_LOAD( "cpu_u2.716", 0x5000, 0x0800, CRC(b8844174) SHA1(6e01321196fd6fce7b5526efc402044c87fe96a6))
	ROM_LOAD( "cpu_u6.716", 0x5800, 0x0800, CRC(6c296d8f) SHA1(8cdb77f382ef1214ef45579213cf8f19141366ad))
	ROM_RELOAD( 0xf800, 0x0800)
ROM_END

/*--------------------------------
/ Nine Ball #125
/-------------------------------*/
ROM_START(nineball)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD( "cpu_u1.716", 0x1000, 0x0800, CRC(fcb58f97) SHA1(6510a6d0b466bd27ade50992260cea716d79fda2))
	ROM_LOAD( "cpu_u5.716", 0x1800, 0x0800, CRC(c7c62161) SHA1(624eab2fdf7bafbf4af012df521bd09f9b2da8d8))
	ROM_LOAD( "cpu_u2.716", 0x5000, 0x0800, CRC(bdd7f258) SHA1(2a38de09827100cbbd4e79be50aad03a3f2b63b4))
	ROM_LOAD( "cpu_u6.716", 0x5800, 0x0800, CRC(7e831499) SHA1(8d3c148b91c21938b1b5fca85ecd8f6d7f1e76b0))
	ROM_RELOAD( 0xf800, 0x0800)
ROM_END

/*--------------------------------
/ Lightning #126
/-------------------------------*/
ROM_START(lightnin)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD( "cpu_u1.716", 0x1000, 0x0800, CRC(d3469d0a) SHA1(18565f5c85694da8eaf850146d3d9a90a17b7816))
	ROM_LOAD( "cpu_u5.716", 0x1800, 0x0800, CRC(cd52262d) SHA1(099aeda2183822046cce907b265b42319007ac32))
	ROM_LOAD( "cpu_u2.716", 0x5000, 0x0800, CRC(e0933419) SHA1(1f7cad915496f34473dffde7e320d51838acd0fd))
	ROM_LOAD( "cpu_u6.716", 0x5800, 0x0800, CRC(df221c6b) SHA1(5935020d3a24d829fbeaa8cf764daff48a151a81))
	ROM_RELOAD( 0xf800, 0x0800)

	ROM_REGION(0x1000, "speech", 0)
	ROM_LOAD("snd_u9.716", 0x0000, 0x0800, CRC(00ffa77c) SHA1(242efd800731a7f84369c6ce54298d0a227dd8ba))
	ROM_LOAD("snd_u10.716",0x0800, 0x0800, CRC(80fe9158) SHA1(20fcdb4c09b25e494f02bbfb20c07ff2870d5798))
ROM_END

/*--------------------------------
/ Stargazer #127
/-------------------------------*/
ROM_START(stargzr)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD( "cpu_u1.716", 0x1000, 0x0800, CRC(83606fd4) SHA1(7f6448bc0dabe50de40fd47a7242c1be4a93e84d))
	ROM_LOAD( "cpu_u5.716", 0x1800, 0x0800, CRC(c54ae389) SHA1(062e64e8ced723adb7f4040539ba6400fc4a9c9a))
	ROM_LOAD( "cpu_u2.716", 0x5000, 0x0800, CRC(1a4c7dcb) SHA1(54888a8867b8d60f215b7e683ae4966f14ddca15))
	ROM_LOAD( "cpu_u6.716", 0x5800, 0x0800, CRC(4e1f4dc6) SHA1(1f63a0b71af84fb6e1168ff77cbcbabcaa1323f3))
	ROM_RELOAD( 0xf800, 0x0800)
ROM_END

/*--------------------------------
/ Flight 2000 #128
/-------------------------------*/
ROM_START(flight2k)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD( "cpu_u1.716", 0x1000, 0x0800, CRC(df9efed9) SHA1(47727664e745e77ca1c221a32bd56d936f5b31bc))
	ROM_LOAD( "cpu_u5.716", 0x1800, 0x0800, CRC(38c13649) SHA1(bcdbd17b48edd41ec7d38261595ac06eb8fc6a4d))
	ROM_LOAD( "cpu_u2.716", 0x5000, 0x0800, CRC(425fae6a) SHA1(fde8d23e6ebb176ba72f763d66c2e17e51237fa1))
	ROM_LOAD( "cpu_u6.716", 0x5800, 0x0800, CRC(dc243186) SHA1(046ce51b8a8218214088c4264548c753bd880e19))
	ROM_RELOAD( 0xf800, 0x0800)

	ROM_REGION(0x1000, "speech", 0)
	ROM_LOAD("snd_u9.716", 0x0000, 0x0800, CRC(d816573c) SHA1(75134a017c34abbb149159ca001d35464a3f5128))
ROM_END

/*--------------------------------
/ Freefall #134
/-------------------------------*/
ROM_START(freefall)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD( "cpu_u1.716", 0x1000, 0x0800, CRC(d13891ad) SHA1(afb40c51f2d5695c74ce9979c0a818845f95edd4))
	ROM_LOAD( "cpu_u5.716", 0x1800, 0x0800, CRC(77bc7759) SHA1(3f739757180b3dcce5426935a51e4b615f157199))
	ROM_LOAD( "cpu_u2.716", 0x5000, 0x0800, CRC(82bda054) SHA1(32772e878d2a4bba8f67e419a68a81fec2a5f6d7))
	ROM_LOAD( "cpu_u6.716", 0x5800, 0x0800, CRC(68168b97) SHA1(defa4bba465182db22debddb4070c40c048c95e2))
	ROM_RELOAD( 0xf800, 0x0800)

	ROM_REGION(0x1000, "speech", 0)
	ROM_LOAD("snd_u9.716", 0x0000, 0x0800, CRC(ea8cf062) SHA1(55c840a9bea363fd436c00a115cb61d15a9f8c47))
	ROM_LOAD("snd_u10.716",0x0800, 0x0800, CRC(dd681a79) SHA1(d954cae375fb0145e10536e43d1cb03902de2ea3))
ROM_END

/*--------------------------------
/ Split Second #144
/-------------------------------*/
ROM_START(spltsecp)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD( "cpu_u1.716", 0x1000, 0x0800, CRC(c6ff9aa9) SHA1(39f80faca16c869ac14df7c5fc3dfa80b47dad95))
	ROM_LOAD( "cpu_u5.716", 0x1800, 0x0800, CRC(fda74efc) SHA1(31becc243ada23e2f4d17927985772c9fcf8a3c3))
	ROM_LOAD( "cpu_u2.716", 0x5000, 0x0800, CRC(81b9f784) SHA1(43cf71b51eda70a3c126340ea658c03c438e4f18))
	ROM_LOAD( "cpu_u6.716", 0x5800, 0x0800, CRC(ecbedb0a) SHA1(8cc7281dd2bd300ab95a08761c12733d98599ebd))
	ROM_RELOAD( 0xf800, 0x0800)

	ROM_REGION(0x1000, "speech", 0)
	ROM_LOAD("snd_u9.716", 0x0000, 0x0800, CRC(e6ed5f48) SHA1(ea2bbc607acb2b816667cd54f3d07605110c252e))
	ROM_LOAD("snd_u10.716",0x0800, 0x0800, CRC(36e6ee70) SHA1(61bd89d69627bea89b7f31af63ff90ace6db3c85))
ROM_END

/*--------------------------------
/ Catacomb #147
/-------------------------------*/
ROM_START(catacomp)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD( "cpu_u1.716", 0x1000, 0x0800, CRC(d445dd40) SHA1(9ff5896977d7e2a0cf788c77dcfd7c010e17d2fb))
	ROM_LOAD( "cpu_u5.716", 0x1800, 0x0800, CRC(d717a545) SHA1(a183f3b1f766c3a82ae52defc38d84328fb7b31a))
	ROM_LOAD( "cpu_u2.716", 0x5000, 0x0800, CRC(bc504409) SHA1(cd3e948d34a8db71fc841261e683988c9df31ef8))
	ROM_LOAD( "cpu_u6.716", 0x5800, 0x0800, CRC(da61b5a2) SHA1(ec4a914cd57b37921578699bc427f12a3670c7eb))
	ROM_RELOAD( 0xf800, 0x0800)

	ROM_REGION(0x1000, "speech", 0)
	ROM_LOAD("snd_u9.716", 0x0000, 0x0800, CRC(a13cb591) SHA1(b64a2dc3429803095dc05cdd1718db2404b13eb8))
	ROM_LOAD("snd_u10.716",0x0800, 0x0800, CRC(2b31f8be) SHA1(05b394bd8b6c04e34fe2bab19cbd0f06d9e4b90d))
ROM_END

/*--------------------------------
/ Viper #148
/-------------------------------*/
ROM_START(viperp)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD( "cpu_u1.716", 0x1000, 0x0800, CRC(d0ea0aeb) SHA1(28f4df9f45807abd1528aa6e5a80933156e6d692))
	ROM_LOAD( "cpu_u5.716", 0x1800, 0x0800, CRC(d26c7273) SHA1(303c18861941463932fdf47e9606159936b28dc1))
	ROM_LOAD( "cpu_u2.716", 0x5000, 0x0800, CRC(d03f1612) SHA1(d390ec1e953148ac26bf218701117855c941fc65))
	ROM_LOAD( "cpu_u6.716", 0x5800, 0x0800, CRC(96ff5f60) SHA1(a9df887ca338db208a684540f6c9fc07722c3aa5))
	ROM_RELOAD( 0xf800, 0x0800)
ROM_END

/*--------------------------------
/ Iron Maiden #151
/-------------------------------*/
ROM_START(ironmaid)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD( "cpu_u1.716", 0x1000, 0x0800, CRC(e15371a4) SHA1(fe441ed8abd325190d8eee6d907e17c7fc02be64))
	ROM_LOAD( "cpu_u5.716", 0x1800, 0x0800, CRC(84a29c01) SHA1(0e0ff8821c7028ce690328cd08a77bb51c0993c9))
	ROM_LOAD( "cpu_u2.716", 0x5000, 0x0800, CRC(981ac0dd) SHA1(c585907b74695812f333867cf359a01a5ea6ed81))
	ROM_LOAD( "cpu_u6.716", 0x5800, 0x0800, CRC(4e6f9c25) SHA1(9053e1d335a29f7acade7752adffe69f42032959))
	ROM_RELOAD( 0xf800, 0x0800)
ROM_END

/*--------------------------------
/ Dragonfist #153
/-------------------------------*/
ROM_START(dragfist)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD( "cpu_u1.716", 0x1000, 0x0800, CRC(4cbd1a38) SHA1(73b7291f38cd0a3300107605db26d474ecfc3101))
	ROM_LOAD( "cpu_u5.716", 0x1800, 0x0800, CRC(1783269a) SHA1(75151b79844d26d9e8ecf00dec96643ee2fedc5b))
	ROM_LOAD( "cpu_u2.716", 0x5000, 0x0800, CRC(9ac8292b) SHA1(99ad3ad6e1d1b19695ce1b5b76f6bd85c9c6530d))
	ROM_LOAD( "cpu_u6.716", 0x5800, 0x0800, CRC(a374c8f9) SHA1(481116025a52353f298f3d93dfe33b3ad9f86d18))
	ROM_RELOAD( 0xf800, 0x0800)
ROM_END

/*--------------------------------
/ Orbitor 1 #165
/-------------------------------*/
ROM_START(orbitor1)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD( "cpu_u1.716", 0x1000, 0x0800, CRC(575520e3) SHA1(9d52b065a14d4f95cebd48f60f628f2c246385fa))
	ROM_LOAD( "cpu_u5.716", 0x1800, 0x0800, CRC(d31f27a8) SHA1(0442260db42192a95f6292e6b57000c127871d28))
	ROM_LOAD( "cpu_u2.716", 0x5000, 0x0800, CRC(4421d827) SHA1(9b617215f2d92ef2c69104eb4e63a924704665aa))
	ROM_LOAD( "cpu_u6.716", 0x5800, 0x0800, CRC(8861155a) SHA1(81a1b3434d4f80dee5704454f8359200faea173d))
	ROM_RELOAD( 0xf800, 0x0800)

	ROM_REGION(0x1000, "speech", 0)
	ROM_LOAD("snd_u9.716", 0x0000, 0x0800, CRC(2ba24569) SHA1(da2f4a4eeed9ae7ff8a342f4d630e12dcb2decf5))
	ROM_LOAD("snd_u10.716",0x0800, 0x0800, CRC(8e5b4a38) SHA1(de3f59363553f5f0d6098401734436930e64fbbd))
ROM_END

/*--------------------------------
/ Cue (Proto - Never released)
/-------------------------------*/
#ifdef MISSING_GAME // everything is NO_DUMP
ROM_START(cue)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD( "cpu_u1.716", 0x1000, 0x0800, NO_DUMP)
	ROM_LOAD( "cpu_u5.716", 0x1800, 0x0800, NO_DUMP)
	ROM_LOAD( "cpu_u2.716", 0x5000, 0x0800, NO_DUMP)
	ROM_LOAD( "cpu_u6.716", 0x5800, 0x0800, NO_DUMP)
	ROM_RELOAD( 0xf800, 0x0800)
ROM_END
#endif

/*--------------------------------
/ Hypnox
/-------------------------------*/

/*----------------------------------------
/ Lazer Lord (Proto - Never released)
/---------------------------------------*/
ROM_START(lazrlord)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD( "cpu_u1.716", 0x1000, 0x0800, CRC(32a6f341) SHA1(75922c6831463d240fe057a0f72280d417899fa4))
	ROM_LOAD( "cpu_u5.716", 0x1800, 0x0800, CRC(17583ba4) SHA1(4807e3ab18c2e40a292b499fe038975bb4b9fc17))
	ROM_LOAD( "cpu_u2.716", 0x5000, 0x0800, CRC(669f3a8e) SHA1(4beb0e4c75f4e3c1788808b57081612d4774d130))
	ROM_LOAD( "cpu_u6.716", 0x5800, 0x0800, CRC(395327a3) SHA1(e2a3a8ea696bcc4b5e11b08b6c7a6d9a991aa4af))
	ROM_RELOAD( 0xf800, 0x0800)
ROM_END

/*--------------------------------
/ Gamatron (Pinstar game, 1985)
/-------------------------------*/
ROM_START(gamatron)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD( "gamatron.764", 0x1000, 0x0800, CRC(fa9f7676) SHA1(8c56868eb6af7bb8ad73523ab6583100fcadc3c1))
	ROM_CONTINUE( 0x5000, 0x0800)
	ROM_CONTINUE( 0x1800, 0x0800)
	ROM_CONTINUE( 0x5800, 0x0800)
	ROM_RELOAD( 0xe000, 0x2000)
ROM_END

/*----------------------------------
/ Black Sheep Squadron (Astro game)
/---------------------------------*/
ROM_START(blkshpsq)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD( "cpu_u2.716", 0x1000, 0x0800, CRC(23d6cd54) SHA1(301ba10f3f333109630dd8abd13a6b4063f805a9))
	ROM_RELOAD( 0x5000, 0x0800)
	ROM_LOAD( "cpu_u6.716", 0x1800, 0x0800, CRC(ea68b9f7) SHA1(ebb69f4faadf457454939e47d8ae6e79eb0e1a11))
	ROM_RELOAD( 0x5800, 0x0800)
	ROM_RELOAD( 0xf800, 0x0800)
ROM_END

/*----------------------------------
/ Unknown game and manufacturer
/---------------------------------*/
ROM_START(st_game)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD( "cpu_u2.716", 0x1000, 0x0800, CRC(b9ac5204) SHA1(1ac4e336eb62c091e61e9b6b21a858e70ac9ab38))
	ROM_RELOAD( 0x5000, 0x0800)
	ROM_LOAD( "cpu_u6.716", 0x1800, 0x0800, CRC(e16fbde1) SHA1(f7fe2f2ef9251792af1227f82dcc95239dd8baa1))
	ROM_RELOAD( 0x5800, 0x0800)
	ROM_RELOAD( 0xf800, 0x0800)
ROM_END

// 6-digit
GAME(1979,  meteorp,    0,          st_mp200,   mp200, st_mp200_state,   st_mp202,   ROT0, "Stern", "Meteor (Stern)", MACHINE_IS_SKELETON_MECHANICAL)
GAME(1980,  galaxypi,   0,          st_mp200,   mp200, st_mp200_state,   st_mp202,   ROT0, "Stern", "Galaxy", MACHINE_IS_SKELETON_MECHANICAL)
GAME(1980,  ali,        0,          st_mp200,   mp200, st_mp200_state,   st_mp202,   ROT0, "Stern", "Ali", MACHINE_IS_SKELETON_MECHANICAL)

// 7-digit
GAME(1980,  biggame,    0,          st_mp200,   mp200, st_mp200_state,   st_mp200,   ROT0, "Stern", "Big Game", MACHINE_IS_SKELETON_MECHANICAL)
GAME(1980,  cheetah,    0,          st_mp200,   mp200, st_mp200_state,   st_mp200,   ROT0, "Stern", "Cheetah", MACHINE_IS_SKELETON_MECHANICAL)
GAME(1980,  quicksil,   0,          st_mp200,   mp200, st_mp200_state,   st_mp200,   ROT0, "Stern", "Quicksilver", MACHINE_IS_SKELETON_MECHANICAL)
GAME(1980,  seawitch,   0,          st_mp200,   mp200, st_mp200_state,   st_mp200,   ROT0, "Stern", "Seawitch", MACHINE_IS_SKELETON_MECHANICAL)
GAME(1980,  nineball,   0,          st_mp200,   mp200, st_mp200_state,   st_mp200,   ROT0, "Stern", "Nine Ball", MACHINE_IS_SKELETON_MECHANICAL)
GAME(1981,  lightnin,   0,          st_mp201,   mp200, st_mp200_state,   st_mp201,   ROT0, "Stern", "Lightning", MACHINE_IS_SKELETON_MECHANICAL)
GAME(1980,  stargzr,    0,          st_mp200,   mp200, st_mp200_state,   st_mp200,   ROT0, "Stern", "Stargazer", MACHINE_IS_SKELETON_MECHANICAL)
GAME(1981,  spltsecp,   0,          st_mp201,   mp200, st_mp200_state,   st_mp201,   ROT0, "Stern", "Split Second (Pinball)", MACHINE_IS_SKELETON_MECHANICAL)
GAME(1981,  catacomp,   0,          st_mp201,   mp200, st_mp200_state,   st_mp201,   ROT0, "Stern", "Catacomb (Pinball)", MACHINE_IS_SKELETON_MECHANICAL)
GAME(1982,  dragfist,   0,          st_mp200,   mp200, st_mp200_state,   st_mp200,   ROT0, "Stern", "Dragonfist", MACHINE_IS_SKELETON_MECHANICAL)
GAME(1984,  lazrlord,   0,          st_mp200,   mp200, st_mp200_state,   st_mp200,   ROT0, "Stern", "Lazer Lord", MACHINE_IS_SKELETON_MECHANICAL)

// hang after boot
GAME(1980,  flight2k,   0,          st_mp201,   mp200, st_mp200_state,   st_mp201,   ROT0, "Stern", "Flight 2000", MACHINE_IS_SKELETON_MECHANICAL)
GAME(1981,  freefall,   0,          st_mp201,   mp200, st_mp200_state,   st_mp201,   ROT0, "Stern", "Freefall", MACHINE_IS_SKELETON_MECHANICAL)
GAME(1981,  viperp,     0,          st_mp200,   mp200, st_mp200_state,   st_mp200,   ROT0, "Stern", "Viper (Pinball)", MACHINE_IS_SKELETON_MECHANICAL)
GAME(1981,  ironmaid,   0,          st_mp200,   mp200, st_mp200_state,   st_mp200,   ROT0, "Stern", "Iron Maiden", MACHINE_IS_SKELETON_MECHANICAL)
GAME(1982,  orbitor1,   0,          st_mp201,   mp200, st_mp200_state,   st_mp201,   ROT0, "Stern", "Orbitor 1", MACHINE_IS_SKELETON_MECHANICAL)

// other manufacturer
GAME(1985,  gamatron,   flight2k,   st_mp200,   mp200, st_mp200_state,   st_mp200,   ROT0, "Pinstar", "Gamatron", MACHINE_IS_SKELETON_MECHANICAL)
GAME(1978,  blkshpsq,   0,          st_mp200,   mp200, st_mp200_state,   st_mp202,   ROT0, "Astro", "Black Sheep Squadron", MACHINE_IS_SKELETON_MECHANICAL)
GAME(198?,  st_game,    0,          st_mp200,   mp200, st_mp200_state,   st_mp200,   ROT0, "<unknown>", "unknown pinball game", MACHINE_IS_SKELETON_MECHANICAL)
