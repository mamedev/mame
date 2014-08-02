/********************************************************************************************

    PINBALL
    Bally MPU AS-2518-17

    These are some very early and well known SS machines, such as 'Eight Ball'.

    They have an orange digital display, and a chime unit.


ToDo:
- Display to fix
- Bow & Arrow fails the PIA test and doesn't boot
- Dips, Inputs, Solenoids vary per game
- Mechanical

*********************************************************************************************/


#include "machine/genpin.h"
#include "cpu/m6800/m6800.h"
#include "machine/6821pia.h"
#include "by17.lh"


class by17_state : public genpin_class
{
public:
	by17_state(const machine_config &mconfig, device_type type, const char *tag)
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

	DECLARE_DRIVER_INIT(by17);
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
	TIMER_DEVICE_CALLBACK_MEMBER(u10_timer);
	TIMER_DEVICE_CALLBACK_MEMBER(u11_timer);
private:
	UINT8 m_u10;
	UINT8 m_u10_a;
	UINT8 m_u10_b;
	UINT8 m_u11_a;
	UINT8 m_u11_b;
	bool m_u10_ca2;
	bool m_u10_cb2;
	bool m_u10_timer;
	bool m_u11_timer;
	UINT8 m_digit;
	UINT8 m_segment;
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


static ADDRESS_MAP_START( by17_map, AS_PROGRAM, 8, by17_state )
	ADDRESS_MAP_GLOBAL_MASK(0x1fff)
	AM_RANGE(0x0000, 0x007f) AM_RAM // internal to the cpu
	AM_RANGE(0x0088, 0x008b) AM_DEVREADWRITE("pia_u10", pia6821_device, read, write)
	AM_RANGE(0x0090, 0x0093) AM_DEVREADWRITE("pia_u11", pia6821_device, read, write)
	AM_RANGE(0x0200, 0x02ff) AM_RAM AM_SHARE("nvram")
	AM_RANGE(0x1000, 0x1fff) AM_ROM AM_REGION("roms", 0 )
ADDRESS_MAP_END

static INPUT_PORTS_START( by17 )
	PORT_START("TEST")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SERVICE1 ) PORT_NAME("Self Test") PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, by17_state, self_test, 0)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_SERVICE2 ) PORT_NAME("Activity") PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, by17_state, activity_test, 0)

	PORT_START("DSW0")
	PORT_DIPNAME( 0x01, 0x00, "S01") // S1-5: 32 combinations of coins/credits of a coin slot. S9-13 other slot.
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x01, DEF_STR( On ))
	PORT_DIPNAME( 0x02, 0x00, "S02")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x02, DEF_STR( On ))
	PORT_DIPNAME( 0x04, 0x00, "S03")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x04, DEF_STR( On ))
	PORT_DIPNAME( 0x08, 0x00, "S04")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x08, DEF_STR( On ))
	PORT_DIPNAME( 0x10, 0x00, "S05")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x10, DEF_STR( On ))
	PORT_DIPNAME( 0x20, 0x20, "S06")
	PORT_DIPSETTING(    0x00, DEF_STR( No ))
	PORT_DIPSETTING(    0x20, DEF_STR( Yes ))
	PORT_DIPNAME( 0x40, 0x40, "S07")
	PORT_DIPSETTING(    0x00, DEF_STR( No ))
	PORT_DIPSETTING(    0x40, DEF_STR( Yes ))
	PORT_DIPNAME( 0x80, 0x80, "S08")
	PORT_DIPSETTING(    0x00, DEF_STR( No ))
	PORT_DIPSETTING(    0x80, DEF_STR( Yes ))

	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x00, "S09")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x01, DEF_STR( On ))
	PORT_DIPNAME( 0x02, 0x00, "S10")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x02, DEF_STR( On ))
	PORT_DIPNAME( 0x04, 0x00, "S11")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x04, DEF_STR( On ))
	PORT_DIPNAME( 0x08, 0x00, "S12")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x08, DEF_STR( On ))
	PORT_DIPNAME( 0x10, 0x00, "S13")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x10, DEF_STR( On ))
	PORT_DIPNAME( 0x20, 0x00, "S14")
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ))
	PORT_DIPSETTING(    0x20, DEF_STR( No ))
	PORT_DIPNAME( 0x40, 0x40, "S15")
	PORT_DIPSETTING(    0x00, DEF_STR( No ))
	PORT_DIPSETTING(    0x40, DEF_STR( Yes ))
	PORT_DIPNAME( 0x80, 0x00, "S16")
	PORT_DIPSETTING(    0x00, DEF_STR( No ))
	PORT_DIPSETTING(    0x80, DEF_STR( Yes ))

	PORT_START("DSW2")
	PORT_DIPNAME( 0x01, 0x00, "S17")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x01, DEF_STR( On ))
	PORT_DIPNAME( 0x02, 0x00, "S18")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x02, DEF_STR( On ))
	PORT_DIPNAME( 0x04, 0x00, "S19")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x04, DEF_STR( On ))
	PORT_DIPNAME( 0x08, 0x00, "S20")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x08, DEF_STR( On ))
	PORT_DIPNAME( 0x10, 0x00, "S21")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x10, DEF_STR( On ))
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
	PORT_DIPNAME( 0x03, 0x03, "Maximum Credits")
	PORT_DIPSETTING(    0x00, "10")
	PORT_DIPSETTING(    0x01, "15")
	PORT_DIPSETTING(    0x02, "25")
	PORT_DIPSETTING(    0x03, "40")
	PORT_DIPNAME( 0x04, 0x04, "Credits displayed")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x04, DEF_STR( On ))
	PORT_DIPNAME( 0x08, 0x08, "Match")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x08, DEF_STR( On ))
	PORT_DIPNAME( 0x10, 0x00, "Keep all replays")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x10, DEF_STR( On ))
	PORT_DIPNAME( 0x20, 0x00, "Voice" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x20, DEF_STR( On ))
	PORT_DIPNAME( 0xC0, 0x40, "Balls")
	PORT_DIPSETTING(    0xC0, "2")
	PORT_DIPSETTING(    0x00, "3")
	PORT_DIPSETTING(    0x80, "4")
	PORT_DIPSETTING(    0x40, "5")

	PORT_START("X0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER )
	PORT_BIT( 0x0a, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OTHER )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_TILT )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Outhole") PORT_CODE(KEYCODE_X)

	PORT_START("X1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_COIN3 )
	PORT_BIT( 0x38, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_OTHER )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_TILT1 ) PORT_NAME("Slam Tilt")

	// from here, vary per game
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

INPUT_CHANGED_MEMBER( by17_state::activity_test )
{
	if(newval)
		m_maincpu->set_input_line(INPUT_LINE_NMI, PULSE_LINE);
}

INPUT_CHANGED_MEMBER( by17_state::self_test )
{
	m_pia_u10->ca1_w(newval);
}

WRITE_LINE_MEMBER( by17_state::u10_ca2_w )
{
	m_u10_ca2 = state;
}
		
WRITE_LINE_MEMBER( by17_state::u10_cb2_w )
{
}

WRITE_LINE_MEMBER( by17_state::u11_ca2_w )
{
	output_set_value("led0", !state);
}

WRITE_LINE_MEMBER( by17_state::u11_cb2_w )
{
}

READ8_MEMBER( by17_state::u10_a_r )
{
	return m_u10_a;
}

WRITE8_MEMBER( by17_state::u10_a_w )
{
	static const UINT8 patterns[16] = { 0x3f,0x06,0x5b,0x4f,0x66,0x6d,0x7d,0x07,0x7f,0x6f,0,0,0,0,0,0 }; // MC14543
	m_segment = data >> 4;
	m_u10_a = data;
	m_u10 = (data & 15) | (BIT(m_u11_a, 0) << 4);

	if (!m_u10_ca2)
	{
		switch (m_u10)
		{
		case 0x10:
			output_set_digit_value(m_digit, patterns[m_segment]);
			break;
		case 0x1d:
			output_set_digit_value(8+m_digit, patterns[m_segment]);
			break;
		case 0x1b:
			output_set_digit_value(16+m_digit, patterns[m_segment]);
			break;
		case 0x07:
			output_set_digit_value(24+m_digit, patterns[m_segment]);
			break;
		case 0x0f:
			output_set_digit_value(32+m_digit, patterns[m_segment]);
			break;
		default:
			break;
		}
	}
}

READ8_MEMBER( by17_state::u10_b_r )
{
	UINT8 data = 0;

	if (BIT(m_u10_a, 0))
		data |= m_io_x0->read();

	if (BIT(m_u10_a, 1))
		data |= m_io_x1->read();

	if (BIT(m_u10_a, 2))
		data |= m_io_x2->read();

	if (BIT(m_u10_a, 3))
		data |= m_io_x3->read();

	if (BIT(m_u10_a, 4))
		data |= m_io_x4->read();

	if (BIT(m_u10_a, 5))
		data |= m_io_dsw0->read();

	if (BIT(m_u10_a, 6))
		data |= m_io_dsw1->read();

	if (BIT(m_u10_a, 7))
		data |= m_io_dsw2->read();

	if (m_u10_cb2)
		data |= m_io_dsw3->read();

	return data;
}

WRITE8_MEMBER( by17_state::u10_b_w )
{
	m_u10_b = data;
}

READ8_MEMBER( by17_state::u11_a_r )
{
	return m_u11_a;
}

WRITE8_MEMBER( by17_state::u11_a_w )
{
	m_u11_a = data;

	m_digit = 0xff;
	if BIT(data, 2)
		m_digit = 4;
	else
	if BIT(data, 3)
		m_digit = 3;
	else
	if BIT(data, 4)
		m_digit = 2;
	else
	if BIT(data, 5)
		m_digit = 1;
	else
	if BIT(data, 6)
		m_digit = 0;
	else
	if BIT(data, 7)
		m_digit = 5;
}

WRITE8_MEMBER( by17_state::u11_b_w )
{
	m_u11_b = data;
	switch (data & 15)
	{
		case 0x0: //
			//m_samples->start(0, 3);
			break;
		case 0x1: // chime 10
			m_samples->start(1, 1);
			break;
		case 0x2: // chime 100
			m_samples->start(2, 2);
			break;
		case 0x3: // chime 1000
			m_samples->start(3, 3);
			break;
		case 0x4: // chime 10000
			m_samples->start(0, 4);
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

void by17_state::machine_reset()
{
	m_u10_a = 0;
	m_u10_b = 0;
	m_u10_cb2 = 0;
	m_u11_a = 0;
	m_u11_b = 0;
}

DRIVER_INIT_MEMBER(by17_state,by17)
{
}

// zero-cross detection
TIMER_DEVICE_CALLBACK_MEMBER( by17_state::u10_timer )
{
	m_u10_timer ^= 1;
	m_pia_u10->cb1_w(m_u10_timer);
}

// 555 timer for display refresh
TIMER_DEVICE_CALLBACK_MEMBER( by17_state::u11_timer )
{
	m_u11_timer ^= 1;
	m_pia_u11->ca1_w(m_u11_timer);
}

static MACHINE_CONFIG_START( by17, by17_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M6800, 1000000) // no xtal, just 2 chips forming a random oscillator
	MCFG_CPU_PROGRAM_MAP(by17_map)

	MCFG_NVRAM_ADD_0FILL("nvram")

	/* Video */
	MCFG_DEFAULT_LAYOUT(layout_by17)

	/* Sound */
	MCFG_FRAGMENT_ADD( genpin_audio )

	/* Devices */
	MCFG_DEVICE_ADD("pia_u10", PIA6821, 0)
	MCFG_PIA_READPA_HANDLER(READ8(by17_state, u10_a_r))
	MCFG_PIA_WRITEPA_HANDLER(WRITE8(by17_state, u10_a_w))
	MCFG_PIA_READPB_HANDLER(READ8(by17_state, u10_b_r))
	MCFG_PIA_WRITEPB_HANDLER(WRITE8(by17_state, u10_b_w))
	MCFG_PIA_CA2_HANDLER(WRITELINE(by17_state, u10_ca2_w))
	MCFG_PIA_CB2_HANDLER(WRITELINE(by17_state, u10_cb2_w))
	MCFG_PIA_IRQA_HANDLER(DEVWRITELINE("maincpu", m6800_cpu_device, irq_line))
	MCFG_PIA_IRQB_HANDLER(DEVWRITELINE("maincpu", m6800_cpu_device, irq_line))
	MCFG_TIMER_DRIVER_ADD_PERIODIC("timer_z", by17_state, u10_timer, attotime::from_hz(120)) // mains freq*2

	MCFG_DEVICE_ADD("pia_u11", PIA6821, 0)
	MCFG_PIA_READPA_HANDLER(READ8(by17_state, u11_a_r))
	MCFG_PIA_WRITEPA_HANDLER(WRITE8(by17_state, u11_a_w))
	MCFG_PIA_WRITEPB_HANDLER(WRITE8(by17_state, u11_b_w))
	MCFG_PIA_CA2_HANDLER(WRITELINE(by17_state, u11_ca2_w))
	MCFG_PIA_CB2_HANDLER(WRITELINE(by17_state, u11_cb2_w))
	MCFG_PIA_IRQA_HANDLER(DEVWRITELINE("maincpu", m6800_cpu_device, irq_line))
	MCFG_PIA_IRQB_HANDLER(DEVWRITELINE("maincpu", m6800_cpu_device, irq_line))
	MCFG_TIMER_DRIVER_ADD_PERIODIC("timer_d", by17_state, u11_timer, attotime::from_hz(634)) // 555 timer*2
MACHINE_CONFIG_END

/*--------------------------------
/ Black Jack
/-------------------------------*/
ROM_START(blackjck)
	ROM_REGION(0x1000, "roms", 0)
	ROM_LOAD( "728-32_2.716", 0x0000, 0x0800, CRC(1333c9d1) SHA1(1fbb60d84db47ffaf7f291575b2705783a110678))
	ROM_LOAD( "720-20_6.716", 0x0800, 0x0800, CRC(0c17aa4d) SHA1(729e61a29691857112579efcdb96a35e8e5b1279))
ROM_END

/*--------------------------------
/ Bow and Arrow
/-------------------------------*/
ROM_START(bowarrow)
	ROM_REGION(0x1000, "roms", 0)
	ROM_LOAD("b14.bin", 0x0400, 0x0200, CRC(d4d0f92a) SHA1(b996cbe9762fafd64115dc78e24626cf08f8abf7))
	ROM_LOAD("b16.bin", 0x0600, 0x0200, CRC(ad2102e7) SHA1(86887beea5e03e80f60c947d6d71431e5eab3d1b))
	ROM_LOAD("b18.bin", 0x0800, 0x0200, CRC(5d84656b) SHA1(d17350f5a0cc0cd00b60df4903034489dce7ade5))
	ROM_LOAD("b1a.bin", 0x0a00, 0x0200, CRC(6f083ce6) SHA1(624b00e72e223c6b9fbf38b831200c9a7aa0d8f7))
	ROM_LOAD("b1c.bin", 0x0c00, 0x0200, CRC(6ed4d39e) SHA1(1f6c57c7274c76246dd2f0b70ec459857a5cf1eb))
	ROM_LOAD("b1e.bin", 0x0e00, 0x0200, CRC(ff2f97de) SHA1(28a8fdeccb1382d3a1153c97466426459c9fa075))
ROM_END

/*--------------------------------
/ Eight Ball
/-------------------------------*/
ROM_START(eightbll)
	ROM_REGION(0x1000, "roms", 0)
	ROM_LOAD( "723-20_2.716", 0x0000, 0x0800, CRC(33559e7b) SHA1(49008db95c8f012e7e3b613e6eee811512207fa9))
	ROM_LOAD( "720-20_6.716", 0x0800, 0x0800, CRC(0c17aa4d) SHA1(729e61a29691857112579efcdb96a35e8e5b1279))
ROM_END

/*--------------------------------
/ Evel Knievel
/-------------------------------*/
ROM_START(evelknie)
	ROM_REGION(0x1000, "roms", 0)
	ROM_LOAD( "722-17_2.716", 0x0000, 0x0800, CRC(b6d9a3fa) SHA1(1939e13f73a324e3d2fd269a54446f48cf530f50))
	ROM_LOAD( "720-20_6.716", 0x0800, 0x0800, CRC(0c17aa4d) SHA1(729e61a29691857112579efcdb96a35e8e5b1279))
ROM_END

/*--------------------------------
/ Freedom
/-------------------------------*/
ROM_START(freedom)
	ROM_REGION(0x1000, "roms", 0)
	ROM_LOAD( "720-08_1.474", 0x0400, 0x0200, CRC(b78bceeb) SHA1(acf6f1a497ada344211f12dbf4be619bee559950))
	ROM_LOAD( "720-10_2.474", 0x0600, 0x0200, CRC(ca90c8a7) SHA1(d9b5e95247e846e50a2a43c85ad5eb1fc761ab67))
	ROM_LOAD( "720-07_6.716", 0x0800, 0x0800, CRC(0f4e8b83) SHA1(faa05dde24eb60be0cdc4456ae2e660a15ed85ac))
ROM_END

/*--------------------------------
/ Mata Hari
/-------------------------------*/
ROM_START(matahari)
	ROM_REGION(0x1000, "roms", 0)
	ROM_LOAD( "725-21_2.716", 0x0000, 0x0800, CRC(63acd9b0) SHA1(2347342f1281c097ea39c79236d85b00a1dfc7b2))
	ROM_LOAD( "720-20_6.716", 0x0800, 0x0800, CRC(0c17aa4d) SHA1(729e61a29691857112579efcdb96a35e8e5b1279))
ROM_END

/*--------------------------------
/ Night Rider
/-------------------------------*/
ROM_START(nightrdr)
	ROM_REGION(0x1000, "roms", 0)
	ROM_LOAD( "721-21_1.716", 0x0000, 0x0800, CRC(237c4060) SHA1(4ce3dba9189fe7666fc76a2c8ee7fff9b12d4c00))
	ROM_LOAD( "720-20_6.716", 0x0800, 0x0800, CRC(f394e357) SHA1(73444f848825a398515153d18de027792b57bcc7))
ROM_END

ROM_START(nightr20)
	ROM_REGION(0x1000, "roms", 0)
	ROM_LOAD( "721-21_1.716", 0x0000, 0x0800, CRC(237c4060) SHA1(4ce3dba9189fe7666fc76a2c8ee7fff9b12d4c00))
	ROM_LOAD( "720-20_6(__rev20).716", 0x0800, 0x0800, CRC(0c17aa4d) SHA1(729e61a29691857112579efcdb96a35e8e5b1279))
ROM_END

/*--------------------------------
/ Power Play
/-------------------------------*/
ROM_START(pwerplay)
	ROM_REGION(0x1000, "roms", 0)
	ROM_LOAD( "724-25_2.716", 0x0000, 0x0800, CRC(43012f35) SHA1(f90d582e3394d949a637a09882ffdad7664c44c0))
	ROM_LOAD( "720-20_6.716", 0x0800, 0x0800, CRC(0c17aa4d) SHA1(729e61a29691857112579efcdb96a35e8e5b1279))
ROM_END

/*--------------------------------
/ Stellar Airship / Geiger-Automatenbau GMBH, of Germany (1981)
/-------------------------------*/

/*--------------------------------
/ Strikes and Spares
/-------------------------------*/
ROM_START(stk_sprs)
	ROM_REGION(0x1000, "roms", 0)
	ROM_LOAD( "740-16_2.716", 0x0000, 0x0800, CRC(2be27024) SHA1(266dee3a5c4c115acc20543df2eb172f1e85dacb))
	ROM_LOAD( "720-20_6.716", 0x0800, 0x0800, CRC(0c17aa4d) SHA1(729e61a29691857112579efcdb96a35e8e5b1279))
ROM_END


GAME( 1978, blackjck, 0,        by17, by17, by17_state, by17, ROT0, "Bally","Black Jack (Pinball)", GAME_IS_SKELETON_MECHANICAL)
GAME( 1976, bowarrow, 0,        by17, by17, by17_state, by17, ROT0, "Bally","Bow & Arrow (Prototype)", GAME_IS_SKELETON_MECHANICAL)
GAME( 1977, eightbll, 0,        by17, by17, by17_state, by17, ROT0, "Bally","Eight Ball", GAME_IS_SKELETON_MECHANICAL)
GAME( 1977, evelknie, 0,        by17, by17, by17_state, by17, ROT0, "Bally","Evel Knievel", GAME_IS_SKELETON_MECHANICAL)
GAME( 1977, freedom,  0,        by17, by17, by17_state, by17, ROT0, "Bally","Freedom", GAME_IS_SKELETON_MECHANICAL)
GAME( 1978, matahari, 0,        by17, by17, by17_state, by17, ROT0, "Bally","Mata Hari", GAME_IS_SKELETON_MECHANICAL)
GAME( 1977, nightrdr, 0,        by17, by17, by17_state, by17, ROT0, "Bally","Night Rider (rev. 21)", GAME_IS_SKELETON_MECHANICAL)
GAME( 1977, nightr20, nightrdr, by17, by17, by17_state, by17, ROT0, "Bally","Night Rider (rev. 20)", GAME_IS_SKELETON_MECHANICAL)
GAME( 1978, pwerplay, 0,        by17, by17, by17_state, by17, ROT0, "Bally","Power Play (Pinball)", GAME_IS_SKELETON_MECHANICAL)
GAME( 1978, stk_sprs, 0,        by17, by17, by17_state, by17, ROT0, "Bally","Strikes and Spares", GAME_IS_SKELETON_MECHANICAL)
