// license:BSD-3-Clause
// copyright-holders:Sandro Ronco
/***************************************************************************

    Ice Cold Beer mechanical arcade game (c) Taito 1983

****************************************************************************/

#include "emu.h"
#include "cpu/m6809/m6809.h"
#include "machine/6821pia.h"
#include "machine/i8279.h"
#include "sound/ay8910.h"
#include "icecold.lh"

class icecold_state : public driver_device
{
public:
	icecold_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
			m_maincpu(*this, "maincpu"),
			m_ay8910_0(*this, "ay0"),
			m_ay8910_1(*this, "ay1"),
			m_pia1(*this, "pia1")
	{ }

	DECLARE_INPUT_CHANGED_MEMBER( test_switch_press );
	DECLARE_CUSTOM_INPUT_MEMBER( motors_limit_r );
	DECLARE_WRITE8_MEMBER( scanlines_w );
	DECLARE_WRITE8_MEMBER( digit_w );
	DECLARE_READ8_MEMBER( kbd_r );
	DECLARE_WRITE8_MEMBER( snd_ctrl_w );
	DECLARE_WRITE8_MEMBER( ay_w );
	DECLARE_READ8_MEMBER( ay_r );
	DECLARE_WRITE8_MEMBER( ay8910_0_b_w );
	DECLARE_WRITE8_MEMBER( ay8910_1_a_w );
	DECLARE_WRITE8_MEMBER( ay8910_1_b_w );
	DECLARE_WRITE8_MEMBER( motors_w );

	// driver_device overrides
	virtual void machine_reset();

	// devices
	required_device<cpu_device> m_maincpu;
	required_device<ay8910_device> m_ay8910_0;
	required_device<ay8910_device> m_ay8910_1;
	required_device<pia6821_device> m_pia1;

	UINT8   m_digit;            // scanlines from i8279
	UINT8   m_sound_latch;      // sound bus latch
	UINT8   m_ay_ctrl;          // ay controls line
	UINT8   m_motors_ctrl;      // motors control
	int     m_sint;             // SINT line
	int     m_motenbl;          // /MOTENBL line
	int     m_ball_gate_sw;     // ball gate switch

	// motors positions
	int     m_rmotor;           // right motor position (0-100)
	int     m_lmotor;           // left motor position (0-100)
	TIMER_DEVICE_CALLBACK_MEMBER(icecold_sint_timer);
	TIMER_DEVICE_CALLBACK_MEMBER(icecold_motors_timer);
};

static ADDRESS_MAP_START( icecold_map, AS_PROGRAM, 8, icecold_state )
	AM_RANGE(0x0000, 0x07ff) AM_RAM
	AM_RANGE(0x4010, 0x4013) AM_DEVREADWRITE("pia0", pia6821_device, read, write)
	AM_RANGE(0x4020, 0x4023) AM_DEVREADWRITE("pia1", pia6821_device, read, write)
	AM_RANGE(0x4040, 0x4043) AM_DEVREADWRITE("pia2", pia6821_device, read, write)   // not used
	AM_RANGE(0x4080, 0x4080) AM_DEVREADWRITE("i8279", i8279_device, data_r, data_w )
	AM_RANGE(0x4081, 0x4081) AM_DEVREADWRITE("i8279", i8279_device, status_r, cmd_w)
	AM_RANGE(0x4100, 0x4100) AM_WRITE(motors_w)
	AM_RANGE(0xa000, 0xffff) AM_ROM
ADDRESS_MAP_END

static INPUT_PORTS_START( icecold )
	PORT_START("DSW3")
	PORT_DIPNAME( 0x01, 0x01, "Automatic Attract Mode" )    PORT_DIPLOCATION("SW3:1")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, "Rounds to Complete to Light Star" )  PORT_DIPLOCATION("SW3:2")
	PORT_DIPSETTING(    0x00, "1 Frame" )
	PORT_DIPSETTING(    0x02, "2 Frames" )
	PORT_DIPNAME( 0x0c, 0x00, "Automatic Attract Mode Delay" )  PORT_DIPLOCATION("SW3:3,SW3:4")
	PORT_DIPSETTING(    0x00, "1 Min" )
	PORT_DIPSETTING(    0x04, "5 Min" )
	PORT_DIPSETTING(    0x08, "10 Min" )
	PORT_DIPSETTING(    0x0c, "15 Min" )
	PORT_DIPNAME( 0x30, 0x00, "Manual Attract Mode Delay" ) PORT_DIPLOCATION("SW3:5,SW3:6")
	PORT_DIPSETTING(    0x00, "0 Min" )
	PORT_DIPSETTING(    0x10, "2 Min" )
	PORT_DIPSETTING(    0x20, "5 Min" )
	PORT_DIPSETTING(    0x30, "10 Min" )
	PORT_DIPNAME( 0xc0, 0x00, "Difficulty (Prompt Time)" )  PORT_DIPLOCATION("SW3:7,SW3:8")
	PORT_DIPSETTING(    0x00, "Easy (5, 4, 2, 1)" )
	PORT_DIPSETTING(    0x40, "Factory (4, 2, 1, 1)" )
	PORT_DIPSETTING(    0x80, "Hard (2, 2, 1, 1)" )
	PORT_DIPSETTING(    0xc0, "X-Hard (1, 1, 1, 1)" )

	PORT_START("DSW4")
	PORT_DIPNAME( 0x07, 0x00, "Dispense Option" )   PORT_DIPLOCATION("SW4:1,SW4:2,SW4:3")
	PORT_DIPSETTING(    0x00, "Disabled" )
	PORT_DIPSETTING(    0x01, "2 Tickets after Hole 6, 3 Tickets after Hole 10" )
	PORT_DIPSETTING(    0x02, "1 Ticket after Holes 5 - 10" )
	PORT_DIPSETTING(    0x03, "No Tickets Dispensed" )
	PORT_DIPSETTING(    0x04, "5 Tickets after Hole 5" )
	PORT_DIPSETTING(    0x05, "No Tickets Dispensed" )
	PORT_DIPSETTING(    0x06, "5 Tickets after Hole 10" )
	PORT_DIPSETTING(    0x07, "No Tickets Dispensed" )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Coin_A ) )   PORT_DIPLOCATION("SW4:4")
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_1C ) )
	PORT_DIPNAME( 0x30, 0x00, "Score for Extra Ball" )  PORT_DIPLOCATION("SW4:5,SW4:6")
	PORT_DIPSETTING(    0x00, "No Extra Ball" )
	PORT_DIPSETTING(    0x10, "2000" )
	PORT_DIPSETTING(    0x20, "4000" )
	PORT_DIPSETTING(    0x30, "8000" )
	PORT_DIPNAME( 0xc0, 0x00, "Bonus Countdown Speed" ) PORT_DIPLOCATION("SW4:7,SW4:8")
	PORT_DIPSETTING(    0x00, "Slow" )
	PORT_DIPSETTING(    0x40, "Factory" )
	PORT_DIPSETTING(    0x80, "Fast" )
	PORT_DIPSETTING(    0xc0, "X-Fast" )

	PORT_START("TEST")  // service switch is directly hard-wired with the NMI line
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_SERVICE) PORT_CHANGED_MEMBER(DEVICE_SELF, icecold_state, test_switch_press, 1)

	PORT_START("JOY")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_JOYSTICKRIGHT_DOWN)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_DOWN)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_JOYSTICKRIGHT_UP)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_UP)
	PORT_BIT(0x55, IP_ACTIVE_LOW, IPT_SPECIAL)          PORT_CUSTOM_MEMBER(DEVICE_SELF, icecold_state, motors_limit_r, NULL)

	PORT_START("X0")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_START1)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_COIN1)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_OTHER)    PORT_NAME("Ball Gate")  PORT_CODE(KEYCODE_0)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_TILT1)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_OTHER)    PORT_NAME(DEF_STR( Free_Play )) PORT_CODE(KEYCODE_3)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_OTHER)    PORT_NAME("Hopper cycle sensor")    PORT_CODE(KEYCODE_6)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_OTHER)    PORT_NAME("Hopper empty")   PORT_CODE(KEYCODE_7)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("X1")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_OTHER)    PORT_NAME("Hole 1") PORT_CODE(KEYCODE_1_PAD)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_OTHER)    PORT_NAME("Hole 2") PORT_CODE(KEYCODE_2_PAD)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_OTHER)    PORT_NAME("Hole 3") PORT_CODE(KEYCODE_3_PAD)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_OTHER)    PORT_NAME("Hole 4") PORT_CODE(KEYCODE_4_PAD)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_OTHER)    PORT_NAME("Hole 5") PORT_CODE(KEYCODE_5_PAD)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_OTHER)    PORT_NAME("Hole 6") PORT_CODE(KEYCODE_6_PAD)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_OTHER)    PORT_NAME("Hole 7") PORT_CODE(KEYCODE_7_PAD)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("X2")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_OTHER)    PORT_NAME("Hole 8") PORT_CODE(KEYCODE_8_PAD)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_OTHER)    PORT_NAME("Hole 9") PORT_CODE(KEYCODE_9_PAD)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_OTHER)    PORT_NAME("Hole 10") PORT_CODE(KEYCODE_0_PAD)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_OTHER)    PORT_NAME("Ticket feed")    PORT_CODE(KEYCODE_8)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_OTHER)    PORT_NAME("Errant Ball")    PORT_CODE(KEYCODE_9)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNUSED)
INPUT_PORTS_END

void icecold_state::machine_reset()
{
	// CH-C is used for generate a 30hz clock
	m_ay8910_0->set_volume(2, 0);

	m_rmotor = m_lmotor = 10;
	m_sint = 0;
	m_motenbl = 0;
	m_ball_gate_sw = 1;
}

CUSTOM_INPUT_MEMBER( icecold_state::motors_limit_r )
{
	UINT8 data = 0;

	if (m_rmotor <= 1)      data |= 0x01;   // right down limit
	if (m_lmotor <= 1)      data |= 0x04;   // left down limit
	if (m_rmotor >= 99)     data |= 0x10;   // right up limit
	if (m_lmotor >= 99)     data |= 0x40;   // left up limit

	return data;
}

INPUT_CHANGED_MEMBER( icecold_state::test_switch_press )
{
	m_maincpu->set_input_line(INPUT_LINE_NMI, newval ? ASSERT_LINE : CLEAR_LINE);
}

WRITE8_MEMBER( icecold_state::motors_w )
{
	m_motors_ctrl = data;
}

WRITE8_MEMBER( icecold_state::scanlines_w )
{
	m_digit = data;
}

WRITE8_MEMBER( icecold_state::digit_w )
{
	output_set_digit_value(m_digit, data & 0x7f);
}

READ8_MEMBER( icecold_state::kbd_r )
{
	switch(m_digit)
	{
		case 0:
			// override the ball gate switch
			return ioport("X0")->read() & ~(m_ball_gate_sw<<2);
		case 1:
			return ioport("X1")->read();
		case 2:
			return ioport("X2")->read();
		default:
			return 0xff;
	}
}


WRITE8_MEMBER( icecold_state::snd_ctrl_w )
{
	if (m_ay_ctrl & ~data & 0x04)
		m_ay8910_0->data_address_w(space, m_ay_ctrl & 0x01, m_sound_latch);
	if (m_ay_ctrl & ~data & 0x20)
		m_ay8910_1->data_address_w(space, (m_ay_ctrl>>3) & 0x01, m_sound_latch);

	m_ay_ctrl = data;
}

WRITE8_MEMBER( icecold_state::ay_w )
{
	m_sound_latch = data;
}

READ8_MEMBER( icecold_state::ay_r )
{
	if (m_ay_ctrl & 0x02)
		return m_ay8910_0->data_r(space, 0);
	if (m_ay_ctrl & 0x10)
		return m_ay8910_1->data_r(space, 0);

	return 0;
}

WRITE8_MEMBER( icecold_state::ay8910_0_b_w )
{
	output_set_lamp_value(1, BIT(data, 0));
	output_set_lamp_value(2, BIT(data, 1));
	output_set_lamp_value(3, BIT(data, 2));
	output_set_lamp_value(4, BIT(data, 3));
	output_set_lamp_value(5, BIT(data, 4));
	output_set_value("in_play", BIT(data, 5));
	output_set_value("good_game", BIT(data, 6));
	m_motenbl = BIT(data, 7);
}

WRITE8_MEMBER( icecold_state::ay8910_1_a_w )
{
	output_set_lamp_value(6, BIT(data, 0));
	output_set_lamp_value(7, BIT(data, 1));
	output_set_lamp_value(8, BIT(data, 2));
	output_set_lamp_value(9, BIT(data, 3));
	output_set_lamp_value(10, BIT(data, 4));
	output_set_value("game_over", BIT(data, 5));
	output_set_value("tilt", BIT(data, 6));
	// BIT 7 watchdog reset
}

WRITE8_MEMBER( icecold_state::ay8910_1_b_w )
{
	if (m_motenbl == 0)
	{
		output_set_value("start", BIT(data, 0));
		coin_counter_w(machine(), 1, BIT(data, 1));     // hopper counter
		coin_counter_w(machine(), 2, BIT(data, 2));     // good game counter
		coin_lockout_w(machine(), 0, BIT(data, 3));     // not used ??
		coin_counter_w(machine(), 0, BIT(data, 4));     // coin counter
		// BIT 5 errant ball solenoid
		// BIT 7 hopper motor
	}
}

TIMER_DEVICE_CALLBACK_MEMBER(icecold_state::icecold_sint_timer)
{
	m_sint = !m_sint;
	m_pia1->ca1_w(m_sint);
}

TIMER_DEVICE_CALLBACK_MEMBER(icecold_state::icecold_motors_timer)
{
	// /MOTENBL is set high during reset for disable the motors control
	if (m_motenbl == 0)
	{
		int lmotor_dir = ((m_motors_ctrl & 0x0f) == 0x06) ? -1 : ((m_motors_ctrl & 0x0f) == 0x09) ? +1 : 0;
		int rmotor_dir = ((m_motors_ctrl & 0xf0) == 0x60) ? -1 : ((m_motors_ctrl & 0xf0) == 0x90) ? +1 : 0;

		// update motors position
		m_lmotor += lmotor_dir;
		m_rmotor += rmotor_dir;

		// if one motor is at the top of the playfield, closes the ball gate switch, to simulate ball movement
		if (m_lmotor >= 99 || m_rmotor >= 99 )
			m_ball_gate_sw = 1;
		// if the motors are at the bottom of the playfield, opens the ball gate switch for start the game
		else if (m_lmotor <= 1 && m_rmotor <= 1 )
			m_ball_gate_sw = 0;

		// motors are keep in range 0-100
		m_lmotor = MIN(m_lmotor, 100);
		m_lmotor = MAX(m_lmotor, 0);
		m_rmotor = MIN(m_rmotor, 100);
		m_rmotor = MAX(m_rmotor, 0);

		if (lmotor_dir != 0 || rmotor_dir != 0)
		{
			output_set_value("lmotor", m_lmotor);
			output_set_value("rmotor", m_rmotor);

			popmessage("Left Motor   Right Motor\n%-4s         %-4s\n%02d\\100       %02d\\100",
						(lmotor_dir > 0) ? " up" : ((lmotor_dir < 0) ? "down" : "off"),
						(rmotor_dir > 0) ? " up" : ((rmotor_dir < 0) ? "down" : "off"), m_lmotor, m_rmotor);
		}
	}
}

static MACHINE_CONFIG_START( icecold, icecold_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M6809, XTAL_6MHz/4)
	MCFG_CPU_PROGRAM_MAP(icecold_map)

	MCFG_DEVICE_ADD( "pia0", PIA6821, 0)
	MCFG_PIA_READPA_HANDLER(IOPORT("JOY"))
	MCFG_PIA_READPB_HANDLER(IOPORT("DSW3"))
	MCFG_PIA_IRQA_HANDLER(DEVWRITELINE("maincpu", m6809_device, irq_line))
	MCFG_PIA_IRQB_HANDLER(DEVWRITELINE("maincpu", m6809_device, irq_line))

	MCFG_DEVICE_ADD( "pia1", PIA6821, 0)
	MCFG_PIA_READPA_HANDLER(READ8(icecold_state, ay_r))
	MCFG_PIA_WRITEPA_HANDLER(WRITE8(icecold_state, ay_w))
	MCFG_PIA_WRITEPB_HANDLER(WRITE8(icecold_state, snd_ctrl_w))
	MCFG_PIA_IRQA_HANDLER(DEVWRITELINE("maincpu", m6809_device, firq_line))
	MCFG_PIA_IRQB_HANDLER(DEVWRITELINE("maincpu", m6809_device, firq_line))

	MCFG_DEVICE_ADD( "pia2", PIA6821, 0)
	MCFG_PIA_IRQA_HANDLER(DEVWRITELINE("maincpu", m6809_device, irq_line))
	MCFG_PIA_IRQB_HANDLER(DEVWRITELINE("maincpu", m6809_device, irq_line))

	MCFG_DEVICE_ADD("i8279", I8279, XTAL_6MHz/4)
	MCFG_I8279_OUT_IRQ_CB(DEVWRITELINE("pia0", pia6821_device, cb1_w)) // irq
	MCFG_I8279_OUT_SL_CB(WRITE8(icecold_state, scanlines_w))        // scan SL lines
	MCFG_I8279_OUT_DISP_CB(WRITE8(icecold_state, digit_w))         // display A&B
	MCFG_I8279_IN_RL_CB(READ8(icecold_state, kbd_r))                // kbd RL lines

	// 30Hz signal from CH-C of ay0
	MCFG_TIMER_DRIVER_ADD_PERIODIC("sint_timer", icecold_state, icecold_sint_timer, attotime::from_hz(30))

	// for update motors position
	MCFG_TIMER_DRIVER_ADD_PERIODIC("motors_timer", icecold_state, icecold_motors_timer, attotime::from_msec(50))

	// video hardware
	MCFG_DEFAULT_LAYOUT(layout_icecold)

	// sound hardware
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("ay0", AY8910, XTAL_6MHz/4)
	MCFG_AY8910_PORT_A_READ_CB(IOPORT("DSW4"))
	MCFG_AY8910_PORT_B_WRITE_CB(WRITE8(icecold_state, ay8910_0_b_w))
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)

	MCFG_SOUND_ADD("ay1", AY8910, XTAL_6MHz/4)
	MCFG_AY8910_PORT_A_WRITE_CB(WRITE8(icecold_state, ay8910_1_a_w))
	MCFG_AY8910_PORT_B_WRITE_CB(WRITE8(icecold_state, ay8910_1_b_w))
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
MACHINE_CONFIG_END

/*-------------------------------------------------------------------
/ Ice Cold Beer
/-------------------------------------------------------------------*/
ROM_START(icecold)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("icb23b.bin", 0xe000, 0x2000, CRC(b5b69d0a) SHA1(86f5444700adebb7b2d9da702b6d5425c8d682e3))
	ROM_LOAD("icb24.bin",  0xc000, 0x2000, CRC(2d1e7282) SHA1(6f170e24f71d1504195face5f67176b55c933eef))
ROM_END

/*-------------------------------------------------------------------
/ Zeke's Peak
/-------------------------------------------------------------------*/
ROM_START(zekepeak)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("zp23.bin", 0xe000, 0x2000, CRC(ef959586) SHA1(7f8a4787b340bfa34180164806b181b5fb4e5cfa))
	ROM_LOAD("zp24.bin", 0xc000, 0x2000, CRC(ee90c8f5) SHA1(27a513000e90536e485ccdf43786b415b3c95bd7))
ROM_END


GAME(1983,  icecold,   0,        icecold,  icecold, driver_device,  0,  ROT0,  "Taito",    "Ice Cold Beer",      MACHINE_NOT_WORKING | MACHINE_MECHANICAL)
GAME(1983,  zekepeak,  icecold,  icecold,  icecold, driver_device,  0,  ROT0,  "Taito",    "Zeke's Peak",        MACHINE_NOT_WORKING | MACHINE_MECHANICAL)
