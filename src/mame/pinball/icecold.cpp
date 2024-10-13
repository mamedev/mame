// license:BSD-3-Clause
// copyright-holders:Sandro Ronco
/***************************************************************************

Ice Cold Beer mechanical arcade game (c) Taito 1983

How to play
1. Insert coin (it says Cr 1)
2. Press Start
3. Wait for Play light
4. Press num-1 which corresponds to the first light on the top row.
5. Continue steps 3 and 4, but pressing num-2, etc as the light advances.
   For the last light, press num-0.
6. To miss, press the wrong button, it says OOPS.
7. After 3 misses, the game ends.

****************************************************************************/

#include "emu.h"
#include "cpu/m6809/m6809.h"
#include "machine/6821pia.h"
#include "machine/i8279.h"
#include "machine/timer.h"
#include "sound/ay8910.h"
#include "speaker.h"

#include "icecold.lh"


namespace {

class icecold_state : public driver_device
{
public:
	icecold_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_ay8910(*this, "ay%u", 0U),
		m_pia1(*this, "pia1"),
		m_digit_outputs(*this, "digit%u", 0U),
		m_lamp_outputs(*this, "lamp%u", 1U),
		m_lmotor_output(*this, "lmotor"),
		m_rmotor_output(*this, "rmotor"),
		m_in_play(*this, "in_play"),
		m_good_game(*this, "good_game"),
		m_game_over(*this, "game_over"),
		m_tilt_output(*this, "tilt"),
		m_start_output(*this, "start")
	{ }

	void icecold(machine_config &config);

	DECLARE_INPUT_CHANGED_MEMBER( test_switch_press );
	ioport_value motors_limit_r();

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	void scanlines_w(uint8_t data);
	void digit_w(uint8_t data);
	uint8_t kbd_r();
	void snd_ctrl_w(uint8_t data);
	void ay_w(uint8_t data);
	uint8_t ay_r();
	void ay8910_0_b_w(uint8_t data);
	void ay8910_1_a_w(uint8_t data);
	void ay8910_1_b_w(uint8_t data);
	void motors_w(uint8_t data);

	// devices
	required_device<cpu_device> m_maincpu;
	required_device_array<ay8910_device, 2> m_ay8910;
	required_device<pia6821_device> m_pia1;

	// outputs
	output_finder<8> m_digit_outputs;
	output_finder<10> m_lamp_outputs;
	output_finder<> m_lmotor_output;
	output_finder<> m_rmotor_output;
	output_finder<> m_in_play;
	output_finder<> m_good_game;
	output_finder<> m_game_over;
	output_finder<> m_tilt_output;
	output_finder<> m_start_output;

	uint8_t m_digit = 0;            // scanlines from i8279
	uint8_t m_sound_latch = 0;      // sound bus latch
	uint8_t m_ay_ctrl = 0;          // ay controls line
	uint8_t m_motors_ctrl = 0;      // motors control
	int     m_sint = 0;             // SINT line
	int     m_motenbl = 0;          // /MOTENBL line
	int     m_ball_gate_sw = 0;     // ball gate switch

	// motors positions
	int     m_rmotor = 0;           // right motor position (0-100)
	int     m_lmotor = 0;           // left motor position (0-100)
	TIMER_DEVICE_CALLBACK_MEMBER(icecold_sint_timer);
	TIMER_DEVICE_CALLBACK_MEMBER(icecold_motors_timer);
	void icecold_map(address_map &map) ATTR_COLD;
};

void icecold_state::icecold_map(address_map &map)
{
	map(0x0000, 0x07ff).ram();
	map(0x4010, 0x4013).rw("pia0", FUNC(pia6821_device::read), FUNC(pia6821_device::write));
	map(0x4020, 0x4023).rw(m_pia1, FUNC(pia6821_device::read), FUNC(pia6821_device::write));
	map(0x4040, 0x4043).rw("pia2", FUNC(pia6821_device::read), FUNC(pia6821_device::write));   // not used
	map(0x4080, 0x4081).rw("i8279", FUNC(i8279_device::read), FUNC(i8279_device::write));
	map(0x4100, 0x4100).w(FUNC(icecold_state::motors_w));
	map(0xa000, 0xffff).rom();
}

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
	PORT_BIT(0x55, IP_ACTIVE_LOW, IPT_CUSTOM)          PORT_CUSTOM_MEMBER(icecold_state, motors_limit_r)

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

void icecold_state::machine_start()
{
	m_digit_outputs.resolve();
	m_lamp_outputs.resolve();
	m_lmotor_output.resolve();
	m_rmotor_output.resolve();
	m_in_play.resolve();
	m_good_game.resolve();
	m_game_over.resolve();
	m_tilt_output.resolve();
	m_start_output.resolve();
}

void icecold_state::machine_reset()
{
	// CH-C is used for generate a 30hz clock
	m_ay8910[0]->set_volume(2, 0);

	m_rmotor = m_lmotor = 10;
	m_sint = 0;
	m_motenbl = 0;
	m_ball_gate_sw = 1;
}

ioport_value icecold_state::motors_limit_r()
{
	uint8_t data = 0;

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

void icecold_state::motors_w(uint8_t data)
{
	m_motors_ctrl = data;
}

void icecold_state::scanlines_w(uint8_t data)
{
	m_digit = data & 7;
}

void icecold_state::digit_w(uint8_t data)
{
	m_digit_outputs[m_digit] = data & 0x7f;
}

uint8_t icecold_state::kbd_r()
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


void icecold_state::snd_ctrl_w(uint8_t data)
{
	if (m_ay_ctrl & ~data & 0x04)
		m_ay8910[0]->data_address_w(m_ay_ctrl & 0x01, m_sound_latch);
	if (m_ay_ctrl & ~data & 0x20)
		m_ay8910[1]->data_address_w((m_ay_ctrl>>3) & 0x01, m_sound_latch);

	m_ay_ctrl = data;
}

void icecold_state::ay_w(uint8_t data)
{
	m_sound_latch = data;
}

uint8_t icecold_state::ay_r()
{
	if (m_ay_ctrl & 0x02)
		return m_ay8910[0]->data_r();
	if (m_ay_ctrl & 0x10)
		return m_ay8910[1]->data_r();

	return 0;
}

void icecold_state::ay8910_0_b_w(uint8_t data)
{
	for (int n = 0; n < 5; n++)
		m_lamp_outputs[n] = BIT(data, n);
	m_in_play = BIT(data, 5);
	m_good_game = BIT(data, 6);
	m_motenbl = BIT(data, 7);
}

void icecold_state::ay8910_1_a_w(uint8_t data)
{
	for (int n = 0; n < 5; n++)
		m_lamp_outputs[n + 5] = BIT(data, n);
	m_game_over = BIT(data, 5);
	m_tilt_output = BIT(data, 6);
	// BIT 7 watchdog reset
}

void icecold_state::ay8910_1_b_w(uint8_t data)
{
	if (m_motenbl == 0)
	{
		m_start_output = BIT(data, 0);
		machine().bookkeeping().coin_counter_w(1, BIT(data, 1));     // hopper counter
		machine().bookkeeping().coin_counter_w(2, BIT(data, 2));     // good game counter
		machine().bookkeeping().coin_lockout_w(0, BIT(data, 3));     // not used ??
		machine().bookkeeping().coin_counter_w(0, BIT(data, 4));     // coin counter
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
		m_lmotor = std::min(m_lmotor, 100);
		m_lmotor = std::max(m_lmotor, 0);
		m_rmotor = std::min(m_rmotor, 100);
		m_rmotor = std::max(m_rmotor, 0);

		if (lmotor_dir != 0 || rmotor_dir != 0)
		{
			m_lmotor_output = m_lmotor;
			m_rmotor_output = m_rmotor;

			popmessage("Left Motor   Right Motor\n%-4s         %-4s\n%02d\\100       %02d\\100",
						(lmotor_dir > 0) ? " up" : ((lmotor_dir < 0) ? "down" : "off"),
						(rmotor_dir > 0) ? " up" : ((rmotor_dir < 0) ? "down" : "off"), m_lmotor, m_rmotor);
		}
	}
}

void icecold_state::icecold(machine_config &config)
{
	/* basic machine hardware */
	MC6809E(config, m_maincpu, XTAL(6'000'000)/4); // 68A09E
	m_maincpu->set_addrmap(AS_PROGRAM, &icecold_state::icecold_map);

	pia6821_device &pia0(PIA6821(config, "pia0"));
	pia0.readpa_handler().set_ioport("JOY");
	pia0.readpb_handler().set_ioport("DSW3");
	pia0.irqa_handler().set_inputline("maincpu", M6809_IRQ_LINE);
	pia0.irqb_handler().set_inputline("maincpu", M6809_IRQ_LINE);

	PIA6821(config, m_pia1);
	m_pia1->readpa_handler().set(FUNC(icecold_state::ay_r));
	m_pia1->writepa_handler().set(FUNC(icecold_state::ay_w));
	m_pia1->writepb_handler().set(FUNC(icecold_state::snd_ctrl_w));
	m_pia1->irqa_handler().set_inputline("maincpu", M6809_FIRQ_LINE);
	m_pia1->irqb_handler().set_inputline("maincpu", M6809_FIRQ_LINE);

	pia6821_device &pia2(PIA6821(config, "pia2"));
	pia2.irqa_handler().set_inputline("maincpu", M6809_IRQ_LINE);
	pia2.irqb_handler().set_inputline("maincpu", M6809_IRQ_LINE);

	i8279_device &kbdc(I8279(config, "i8279", XTAL(6'000'000)/4));
	kbdc.out_irq_callback().set("pia0", FUNC(pia6821_device::cb1_w));   // irq
	kbdc.out_sl_callback().set(FUNC(icecold_state::scanlines_w));       // scan SL lines
	kbdc.out_disp_callback().set(FUNC(icecold_state::digit_w));         // display A&B
	kbdc.in_rl_callback().set(FUNC(icecold_state::kbd_r));              // kbd RL lines

	// 30Hz signal from CH-C of ay0
	TIMER(config, "sint_timer", 0).configure_periodic(FUNC(icecold_state::icecold_sint_timer), attotime::from_hz(30));

	// for update motors position
	TIMER(config, "motors_timer", 0).configure_periodic(FUNC(icecold_state::icecold_motors_timer), attotime::from_msec(50));

	// video hardware
	config.set_default_layout(layout_icecold);

	// sound hardware
	SPEAKER(config, "mono").front_center();
	AY8910(config, m_ay8910[0], XTAL(6'000'000)/4);
	m_ay8910[0]->port_a_read_callback().set_ioport("DSW4");
	m_ay8910[0]->port_b_write_callback().set(FUNC(icecold_state::ay8910_0_b_w));
	m_ay8910[0]->add_route(ALL_OUTPUTS, "mono", 0.25);

	AY8910(config, m_ay8910[1], XTAL(6'000'000)/4);
	m_ay8910[1]->port_a_write_callback().set(FUNC(icecold_state::ay8910_1_a_w));
	m_ay8910[1]->port_b_write_callback().set(FUNC(icecold_state::ay8910_1_b_w));
	m_ay8910[1]->add_route(ALL_OUTPUTS, "mono", 0.25);
}

/*-------------------------------------------------------------------
/ Ice Cold Beer
/-------------------------------------------------------------------*/
ROM_START(icecold)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("icb23b.bin", 0xe000, 0x2000, CRC(b5b69d0a) SHA1(86f5444700adebb7b2d9da702b6d5425c8d682e3))
	ROM_LOAD("icb24.bin",  0xc000, 0x2000, CRC(2d1e7282) SHA1(6f170e24f71d1504195face5f67176b55c933eef))
ROM_END

ROM_START(icecoldf)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("icb23b_f.bin", 0xe000, 0x2000, CRC(6fe73c9d) SHA1(24b60da1fc791844601bd9a7628fde195e9e9644) )
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

} // anonymous namespace


GAME( 1983, icecold,  0,       icecold, icecold, icecold_state, empty_init, ROT0, "Taito", "Ice Cold Beer (set 1)", MACHINE_NOT_WORKING | MACHINE_MECHANICAL)
GAME( 1983, icecoldf, icecold, icecold, icecold, icecold_state, empty_init, ROT0, "Taito", "Ice Cold Beer (set 2)", MACHINE_NOT_WORKING | MACHINE_MECHANICAL)
GAME( 1983, zekepeak, icecold, icecold, icecold, icecold_state, empty_init, ROT0, "Taito", "Zeke's Peak",   MACHINE_NOT_WORKING | MACHINE_MECHANICAL)
