// license:BSD-3-Clause
// copyright-holders:hap
/*******************************************************************************

Novag Chess Robot Adversary, chess computer with robotic arm. The chess engine
is MyChess by David Kittinger, just like the one in Novag Savant.

Hardware notes:
- PCB label: GOODNIGHT DESIGN, PACIFIC MICROELECTRONICS GROUP, 743-279A/280A/281A
- Zilog Z8400B PS, 6 MHz XTAL
- 40KB ROM (4*2764 or equivalent, 4*MSM2716AS) + 1 socket for expansion
- 5KB RAM (8*TMM314APL-1, 2*TC5514AP-8 battery-backed)
- SN76489AN sound
- robot arm with 4 DC motors
- 12+12 leds, 8*8 magnet sensors, printer port

See patent US4398720 for a more detailed description of the hardware.

Newer versions sold in West Germany were marketed as 7.5MHz, but it's not known
if it's really an overclock, or maybe they just removed waitstates.

On the left and right of the chessboard are designated spots for captured pieces,
no magnet sensors underneath. The user is not required to place pieces there, but
the chesscomputer will give an error when it tries to take a piece from there
(eg. with trace back, review, or when it cleans up after the game has finished).

In MAME, the claw position is shown with a small dot, opaque means it's open.
After the CPU's move, wait until the claw is closed before inputting new move.

TODO:
- it becomes unresponsive when pressing certain keys right after the user lost,
  like Trace Back, possibly BTANB?
- Z80 waitstates according to patent

*******************************************************************************/

#include "emu.h"

#include "cpu/z80/z80.h"
#include "machine/clock.h"
#include "machine/nvram.h"
#include "machine/sensorboard.h"
#include "sound/sn76496.h"
#include "video/pwm.h"

#include "speaker.h"

// internal artwork
#include "novag_robotadv.lh"


namespace {

class robotadv_state : public driver_device
{
public:
	robotadv_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_display(*this, "display"),
		m_board(*this, "board"),
		m_sn(*this, "sn"),
		m_inputs(*this, "IN.%u", 0),
		m_piece_hand(*this, "cpu_hand"),
		m_out_motor(*this, "motor%u", 0U),
		m_out_pos(*this, "pos_%c", unsigned('x'))
	{ }

	void robotadv(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;
	required_device<pwm_display_device> m_display;
	required_device<sensorboard_device> m_board;
	required_device<sn76489a_device> m_sn;
	required_ioport_array<3> m_inputs;
	output_finder<> m_piece_hand;
	output_finder<6> m_out_motor;
	output_finder<2> m_out_pos;

	u8 m_control1 = 0;
	u8 m_control2 = 0;
	u8 m_latch = 0;
	u8 m_motor_on = 0;
	u8 m_motor_dir = 0;
	u8 m_limits = 0;
	s32 m_counter[4] = { };
	attotime m_pwm_accum[4];
	attotime m_pwm_last;
	emu_timer *m_refresh_timer;

	void main_map(address_map &map) ATTR_COLD;
	void io_map(address_map &map) ATTR_COLD;

	void control1_w(u8 data);
	void control2_w(u8 data);
	void latch_w(u8 data);
	u8 limits_r();
	u8 input_r();
	u8 counters_r();

	void init_board(u8 data);
	void clear_board(u8 data);
	void refresh(s32 param = 0);
	void update_counters();
	void update_limits();
	void update_clawpos(double *x, double *y);
	void update_piece(double x, double y);
};



/*******************************************************************************
    Initialization
*******************************************************************************/

void robotadv_state::machine_start()
{
	m_refresh_timer = timer_alloc(FUNC(robotadv_state::refresh), this);

	// resolve outputs
	m_piece_hand.resolve();
	m_out_motor.resolve();
	m_out_pos.resolve();

	// register for savestates
	save_item(NAME(m_control1));
	save_item(NAME(m_control2));
	save_item(NAME(m_latch));
	save_item(NAME(m_motor_on));
	save_item(NAME(m_motor_dir));
	save_item(NAME(m_limits));
	save_item(NAME(m_counter));
	save_item(NAME(m_pwm_accum));
	save_item(NAME(m_pwm_last));
}

void robotadv_state::machine_reset()
{
	refresh();
}

void robotadv_state::init_board(u8 data)
{
	m_board->preset_chess(data);

	// reposition pieces if board will be rotated
	if (data & 2)
	{
		for (int y = 0; y < 8; y++)
			for (int x = 7; x >= 0; x--)
			{
				m_board->write_piece(x + 4, y, m_board->read_piece(x, y));
				m_board->write_piece(x, y, 0);
			}
	}
}

void robotadv_state::clear_board(u8 data)
{
	m_piece_hand = 0;
	m_board->clear_board(data);
}



/*******************************************************************************
    Motor Sim
*******************************************************************************/

void robotadv_state::update_counters()
{
	attotime now = machine().time();
	attotime delta = now - m_pwm_last;
	m_pwm_last = now;

	for (int i = 0; i < 4; i++)
	{
		if (BIT(m_motor_on, i))
		{
			m_pwm_accum[i] += delta;

			// increment counters per 1us
			const u32 freq = 1'000'000;
			u64 ticks = m_pwm_accum[i].as_ticks(freq);
			m_pwm_accum[i] -= attotime::from_ticks(ticks, freq);

			if (BIT(m_motor_dir, i))
				m_counter[i] -= ticks;
			else
				m_counter[i] += ticks;
		}
	}
}

void robotadv_state::update_limits()
{
	m_limits = 0;

	// claw and forearm lever
	const u32 range[2] = { 300'000, 2'000'000 };
	for (int i = 0; i < 2; i++)
	{
		m_counter[i] %= range[i];
		if (m_counter[i] > range[i] / 2)
			m_limits |= 1 << i;
	}

	// forearm rotation
	if (m_counter[2] <= 0)
	{
		m_counter[2] = 0;
		m_limits |= 4;
	}

	// upper arm rotation
	if (m_counter[3] >= 0)
		m_limits |= 8;
}

void robotadv_state::update_clawpos(double *x, double *y)
{
	double r, d, a;

	// upper arm
	r = 5.43;
	d = m_counter[3] / 12670.0;
	a = d * (M_PI / 180.0) + M_PI;
	*x = r * cos(a);
	*y = r * sin(a);

	// elbow (home position is at a slight angle)
	r = 1.07;
	d = m_counter[2] / 14730.0 + 8.8;
	a += d * (M_PI / 180.0) + (1.5 * M_PI);
	*x += r * cos(a);
	*y += r * sin(a);

	// forearm is perpendicular to elbow
	r = 5.62;
	a += 1.5 * M_PI;
	*x += r * cos(a);
	*y += r * sin(a);
}

void robotadv_state::update_piece(double x, double y)
{
	// convert claw x/y to sensorboard x/y (1.0 = 1 square)
	int bx = -1, by = 0;

	// chessboard
	x += 4.0;
	y -= 2.1;
	if (x >= 0.0 && x < 8.0 && y >= 0.0 && y < 8.0)
	{
		bx = x;
		by = y;
	}

	// left edge
	x += 2.4;
	y += 1.0;
	if (x >= 0.0 && x < 2.0 && y >= 0.0 && y < 8.0)
	{
		bx = x + 8;
		by = y;
	}

	// right edge
	x -= 10.8;
	if (x >= 0.0 && x < 2.0 && y >= 0.0 && y < 8.0)
	{
		bx = x + 10;
		by = y;
	}

	by = 7 - by;

	if (m_limits & 1)
	{
		// open empty claw
		if (m_piece_hand == 0)
			return;

		// drop piece to invalid position (shouldn't happen)
		else if (bx == -1)
			popmessage("Invalid piece drop!");

		// collision with piece on board (user interference)
		else if (m_board->read_piece(bx, by) != 0)
			popmessage("Collision at %c%d!", bx + 'A', by + 1);
		else
		{
			m_board->write_piece(bx, by, m_piece_hand);
			m_board->refresh();
		}

		m_piece_hand = 0;
	}
	else
	{
		// close claw while forearm is raised, or at invalid position
		if (~m_limits & 2 || bx == -1)
			return;

		// pick up piece, unless it was picked up by the user
		const int pos = (by << 4 & 0xf0) | (bx & 0x0f);
		if (pos != m_board->get_handpos())
		{
			m_piece_hand = m_board->read_piece(bx, by);

			if (m_piece_hand != 0)
			{
				m_board->write_piece(bx, by, 0);
				m_board->refresh();
			}
		}
	}
}

void robotadv_state::refresh(s32 param)
{
	if (machine().side_effects_disabled())
		return;

	update_counters();

	u8 limits_prev = m_limits;
	update_limits();

	double x, y;
	update_clawpos(&x, &y);

	// claw opened or closed
	if ((m_limits ^ limits_prev) & 1)
		update_piece(x, y);

	// output claw position
	const int open = (m_limits & 1) ? 0x800 : 0; // put open state on x bit 11
	m_out_pos[0] = int((x + 15.0) * 50.0 + 0.5) | open;
	m_out_pos[1] = int((y + 15.0) * 50.0 + 0.5);

	m_refresh_timer->adjust(attotime::from_hz(60));
}



/*******************************************************************************
    I/O
*******************************************************************************/

void robotadv_state::control1_w(u8 data)
{
	// d0 falling edge: write sound
	if (~data & m_control1 & 1)
		m_sn->write(m_latch);

	// d1: ?
	// d2,d3: chess clock peripheral?
	// d5,d7: arm motor on
	refresh();
	m_motor_on = (m_motor_on & ~0xc) | (bitswap<2>(data,7,5) << 2);

	// d4,d6: arm motor direction
	m_motor_dir = (m_motor_dir & ~0xc) | (bitswap<2>(data,6,4) << 2);

	// reverse accum if direction changed
	if ((data ^ m_control1) & 0x10)
		m_pwm_accum[2] = attotime::from_usec(1) - m_pwm_accum[2];
	if ((data ^ m_control1) & 0x40)
		m_pwm_accum[3] = attotime::from_usec(1) - m_pwm_accum[3];

	for (int i = 0; i < 4; i++)
		m_out_motor[i + 2] = BIT(data, i + 4);

	m_control1 = data;
}

void robotadv_state::control2_w(u8 data)
{
	// d0,d1: claw/lever motor on
	refresh();
	m_motor_on = (m_motor_on & ~3) | (data & 3);

	for (int i = 0; i < 2; i++)
		m_out_motor[i] = BIT(data, i);

	// d2-d4: keypad mux
	// d5-d7: led select
	m_display->write_my(data >> 5);
	m_control2 = data;
}

void robotadv_state::latch_w(u8 data)
{
	// led data, sound data, chessboard mux
	m_display->write_mx(data);
	m_latch = data;
}

u8 robotadv_state::limits_r()
{
	// d0: ?
	// d1-d4: limit switches
	// d5-d7: printer
	refresh();
	return m_limits << 1;
}

u8 robotadv_state::input_r()
{
	u8 data = 0;

	// read chessboard
	if (m_latch)
	{
		refresh();
		for (int i = 0; i < 8; i++)
		{
			if (BIT(m_latch, i))
				data |= m_board->read_file(i, true);
		}
	}

	// read keypad
	for (int i = 0; i < 3; i++)
		if (BIT(m_control2 >> 2, i))
			data |= m_inputs[i]->read();

	return ~data;
}

u8 robotadv_state::counters_r()
{
	// arm motors optical sensors to cd4029 counters
	refresh();
	const int ratio = 300; // around 1 count per 300us
	u8 c2 = (m_counter[2] / ratio) & 0xf;
	u8 c3 = (m_counter[3] / ratio) & 0xf;

	return c2 << 4 | c3;
}



/*******************************************************************************
    Address Maps
*******************************************************************************/

void robotadv_state::main_map(address_map &map)
{
	map(0x0000, 0x5fff).rom();
	map(0x8000, 0xbfff).rom();
	map(0xc000, 0xcfff).ram();
	map(0xd400, 0xd7ff).ram().share("nvram");
}

void robotadv_state::io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0xc0, 0xc0).w(FUNC(robotadv_state::control1_w));
	map(0xc1, 0xc1).w(FUNC(robotadv_state::control2_w));
	map(0xc2, 0xc2).nopw(); // printer
	map(0xc3, 0xc3).r(FUNC(robotadv_state::limits_r));
	map(0xc4, 0xc4).w(FUNC(robotadv_state::latch_w));
	map(0xc5, 0xc5).nopw(); // printer
	map(0xc6, 0xc6).r(FUNC(robotadv_state::input_r));
	map(0xc7, 0xc7).r(FUNC(robotadv_state::counters_r));
}



/*******************************************************************************
    Input Ports
*******************************************************************************/

static INPUT_PORTS_START( robotadv )
	PORT_START("IN.0")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_C) PORT_NAME("Trace Forward")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_E) PORT_NAME("Verify")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_R) PORT_NAME("Change Color")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_V) PORT_NAME("Review")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_B) PORT_NAME("Return")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_T) PORT_NAME("Hint")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_Y) PORT_NAME("Level")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_N) PORT_NAME("Go")

	PORT_START("IN.1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_G) PORT_NAME("Print List")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_5) PORT_NAME("Sound")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_6) PORT_NAME("Solve Mate")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_H) PORT_NAME("Print Board")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_Z) PORT_NAME("Best Move")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_Q) PORT_NAME("Promote")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_W) PORT_NAME("Set Up")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_X) PORT_NAME("Trace Back")

	PORT_START("IN.2")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_A) PORT_NAME("Demo Program")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_1) PORT_NAME("New Game")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_2) PORT_NAME("Auto Play")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_S) PORT_NAME("Test")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_D) PORT_NAME("Form Size")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_3) PORT_NAME("Classic Game")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_4) PORT_NAME("Emotions")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_F) PORT_NAME("Print Moves")
INPUT_PORTS_END



/*******************************************************************************
    Machine Configs
*******************************************************************************/

void robotadv_state::robotadv(machine_config &config)
{
	// basic machine hardware
	Z80(config, m_maincpu, 6_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &robotadv_state::main_map);
	m_maincpu->set_addrmap(AS_IO, &robotadv_state::io_map);

	auto &irq_clock(CLOCK(config, "irq_clock", 1200)); // approximation, from 555 timer with VR
	irq_clock.set_pulse_width(attotime::from_usec(10)); // guessed
	irq_clock.signal_handler().set_inputline(m_maincpu, INPUT_LINE_IRQ0);

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_1);

	SENSORBOARD(config, m_board).set_type(sensorboard_device::MAGNETS);
	m_board->set_size(8+4, 8);
	m_board->clear_cb().set(FUNC(robotadv_state::clear_board));
	m_board->init_cb().set(FUNC(robotadv_state::init_board));
	m_board->set_delay(attotime::from_msec(150));
	m_board->set_nvram_enable(true);

	// video hardware
	PWM_DISPLAY(config, m_display).set_size(3, 8);
	config.set_default_layout(layout_novag_robotadv);

	// sound hardware
	SPEAKER(config, "speaker").front_center();
	SN76489A(config, m_sn, 6_MHz_XTAL / 2).add_route(ALL_OUTPUTS, "speaker", 1.0);
}



/*******************************************************************************
    ROM Definitions
*******************************************************************************/

ROM_START( robotadv )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("mk37029n-5_brown.u5", 0x0000, 0x2000, CRC(04f8524c) SHA1(644a35ab53aaf641af799fab2ebc870a7f4d4535) ) // MK37000-5
	ROM_LOAD("mk37030n-5_white.u6", 0x2000, 0x2000, CRC(d6db4cfb) SHA1(54ab3eee2cbc9604e793116144c129f372e4144e) ) // "
	ROM_LOAD("mk37031n-5_blue.u8",  0x4000, 0x2000, CRC(ae1ead57) SHA1(13fcbd751efb478f0c4f08611388eaae60bba8bf) ) // "
	ROM_LOAD("orange.u10",          0xa000, 0x2000, CRC(a90a2576) SHA1(9f91ca21477de3ebc668d3ec3a08842c5d19c5ec) ) // HN482764G

	ROM_LOAD("u1",         0x8000, 0x0800, CRC(db6b2598) SHA1(1315c831d3a745a171fddc7ecb7a2d23d9acf4e8) ) // MSM2716AS (u1 has no label, confirmed with several pcbs)
	ROM_LOAD("robep_2.u2", 0x8800, 0x0800, CRC(100d8a59) SHA1(b60656eee18f861b0bafedea8242afd319a8e9e3) ) // "
	ROM_LOAD("robep_3.u3", 0x9000, 0x0800, CRC(1a0067db) SHA1(73527246847e14527b7fba0ef19aaa46650d15da) ) // "
	ROM_LOAD("robep_4.u4", 0x9800, 0x0800, CRC(30dba023) SHA1(03a133ea454ee2f60890a51a77be57eadd9af9dd) ) // "
ROM_END

} // anonymous namespace



/*******************************************************************************
    Drivers
*******************************************************************************/

//    YEAR  NAME      PARENT  COMPAT  MACHINE   INPUT     CLASS           INIT        COMPANY, FULLNAME, FLAGS
SYST( 1982, robotadv, 0,      0,      robotadv, robotadv, robotadv_state, empty_init, "Novag Industries / Intelligent Heuristic Programming", "Chess Robot Adversary", MACHINE_SUPPORTS_SAVE | MACHINE_MECHANICAL | MACHINE_IMPERFECT_CONTROLS )
