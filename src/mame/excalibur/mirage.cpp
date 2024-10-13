// license:BSD-3-Clause
// copyright-holders:hap
/*******************************************************************************

Excalibur Mirage (model 702E)

It's Excalibur's first chess computer, and also Ron Nelson's official return to
chess programming. The x/y motorized magnet is similar to the one used in
Fidelity Phantom (and Milton Bradley Grand·Master before that).

Before moving a piece, wait until the computer is done with its own move. After
capturing a piece, select the captured piece from the MAME sensorboard spawn
block and place it anywhere on a free spot at the designated box at the edge
of the chessboard.

Hardware notes:
- PCB label: EXCALIBUR ELECTRONICS, INC. 6/5/96, MIRAGE, 00-55052-000
- Hitachi H8/3256 MCU (only 32KB out of 48KB internal ROM used), either Mask ROM
  or PROM, 20MHz XTAL
- 2*L293DNE motor drivers, 2 DC motors (like a plotter), electromagnet under the
  chessboard for automatically moving the pieces
- LCD with 5 7segs and custom segments
- piezo, button sensors chessboard

There's also a version with a fake wood housing instead of black plastic, it's
most likely the same hardware.

TODO:
- like fphantom, sensorboard undo buffer fills up pretty quickly
- dump/add PROM version, maybe they improved the motor drift issue?
- it does a cold boot at every reset, so nvram won't work properly unless MAME
  adds some kind of auxillary autosave state feature at power-off

BTANB:
- Motors gradually drift, causing it to place/pick up pieces off-center. It
  recalibrates itself once in a while but it's not enough. MAME's sensorboard
  device can't deal with it, so there's a workaround (see realign_magnet_pos).
  Ron Nelson blamed it on the hardware engineer, but it's a software fault.

*******************************************************************************/

#include "emu.h"

#include "cpu/h8/h8325.h"
#include "machine/sensorboard.h"
#include "sound/dac.h"
#include "video/pwm.h"

#include "screen.h"
#include "speaker.h"

// internal artwork
#include "excal_mirage.lh"


namespace {

class mirage_state : public driver_device
{
public:
	mirage_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_board(*this, "board"),
		m_lcd_pwm(*this, "lcd_pwm"),
		m_dac(*this, "dac"),
		m_inputs(*this, "IN.%u", 0),
		m_out_lcd(*this, "s%u.%u", 0U, 0U),
		m_piece_hand(*this, "cpu_hand"),
		m_out_motor(*this, "motor%u", 0U),
		m_out_pos(*this, "pos_%c", unsigned('x'))
	{ }

	void mirage(machine_config &config);

	DECLARE_INPUT_CHANGED_MEMBER(on_button);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	// devices/pointers
	required_device<h83256_device> m_maincpu;
	required_device<sensorboard_device> m_board;
	required_device<pwm_display_device> m_lcd_pwm;
	required_device<dac_1bit_device> m_dac;
	required_ioport_array<3> m_inputs;
	output_finder<2, 24> m_out_lcd;
	output_finder<> m_piece_hand;
	output_finder<5> m_out_motor;
	output_finder<2> m_out_pos;

	u16 m_inp_mux = 0;
	u32 m_lcd_segs = 0;
	u8 m_lcd_com = 0;

	u8 m_magnet = 0;
	u8 m_pieces_map[0x40][0x40] = { };

	u8 m_motor_control[2] = { };
	u32 m_motor_max[2] = { };
	u32 m_motor_pos[2] = { };
	s32 m_motor_drift[2] = { };
	u8 m_motor_quad[2] = { };

	attotime m_motor_period;
	attotime m_motor_remain[2];
	emu_timer *m_motor_timer[2];

	void init_board(u8 data);
	void clear_board(u8 data);
	void init_motors();

	void get_scaled_pos(double *x, double *y);
	void output_magnet_pos();
	void realign_magnet_pos();
	void update_piece(u8 magnet);

	TIMER_CALLBACK_MEMBER(motor_count);
	void motor_control(int m, u8 control);

	// I/O handlers
	void lcd_pwm_w(offs_t offset, u8 data);
	void update_lcd();
	template <int N> void lcd_segs_w(u8 data);

	u8 p4_r();
	u8 p5_r();
	void p5_w(u8 data);
	u8 p6_r();
	void p6_w(u8 data);
	u8 p7_r();
	void p7_w(offs_t offset, u8 data, u8 mem_mask);
};



/*******************************************************************************
    Initialization
*******************************************************************************/

void mirage_state::machine_start()
{
	init_motors();

	// resolve handlers
	m_out_lcd.resolve();
	m_piece_hand.resolve();
	m_out_motor.resolve();
	m_out_pos.resolve();

	// register for savestates
	save_item(NAME(m_inp_mux));
	save_item(NAME(m_lcd_segs));
	save_item(NAME(m_lcd_com));

	save_item(NAME(m_magnet));
	save_item(NAME(m_pieces_map));
	save_item(NAME(m_motor_control));
	save_item(NAME(m_motor_pos));
	save_item(NAME(m_motor_drift));
	save_item(NAME(m_motor_quad));
	save_item(NAME(m_motor_remain));
}

void mirage_state::machine_reset()
{
	memset(m_motor_drift, 0, sizeof(m_motor_drift));
	output_magnet_pos();
}

void mirage_state::init_board(u8 data)
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

void mirage_state::clear_board(u8 data)
{
	memset(m_pieces_map, 0, sizeof(m_pieces_map));
	m_piece_hand = 0;
	m_board->clear_board(data);
}



/*******************************************************************************
    Motor Sim
*******************************************************************************/

void mirage_state::init_motors()
{
	m_motor_period = attotime::from_usec(800);

	for (int i = 0; i < 2; i++)
	{
		m_motor_timer[i] = timer_alloc(FUNC(mirage_state::motor_count), this);
		m_motor_remain[i] = m_motor_period / 2;
	}

	m_motor_max[1] = 1500 - 1;
	m_motor_pos[1] = 1480;

	m_motor_max[0] = 2020 - 1;
	m_motor_pos[0] = 24;
}

void mirage_state::get_scaled_pos(double *x, double *y)
{
	// 176 counts per square
	*x = std::clamp(double(int(m_motor_pos[0]) - 20) / (176.0 / 4.0) + 2.0, 0.0, 48.0);
	*y = std::clamp(double(int(m_motor_pos[1]) - 160) / (176.0 / 4.0) + 2.0, 0.0, 32.0);
}

void mirage_state::output_magnet_pos()
{
	double x, y;
	get_scaled_pos(&x, &y);

	// put active state on x bit 11
	const int active = m_magnet ? 0x800 : 0;
	m_out_pos[0] = int(x * 25.0 + 0.5) | active;
	m_out_pos[1] = int(y * 25.0 + 0.5);
}

void mirage_state::realign_magnet_pos()
{
	// compensate for gradual drift, see BTANB
	for (int i = 0; i < 2; )
	{
		double pos[2];
		get_scaled_pos(&pos[0], &pos[1]);

		const double limit = 1.0 / 11.0; // 4 counts
		int inc = 0;

		if ((round(pos[i]) - pos[i]) > limit && m_motor_pos[i] < m_motor_max[i] - 4)
			inc = 1;
		else if ((round(pos[i]) - pos[i]) < -limit && m_motor_pos[i] > 4)
			inc = -1;
		else
			i++;

		if (inc != 0)
		{
			m_motor_pos[i] += inc * 4;
			m_motor_drift[i] -= inc;

			logerror("motor %c drift error (%d total)\n", 'X' + i, m_motor_drift[i]);
		}
	}
}

void mirage_state::update_piece(u8 magnet)
{
	if (magnet == m_magnet)
		return;

	realign_magnet_pos();

	m_magnet = magnet;
	output_magnet_pos();

	double dx, dy;
	get_scaled_pos(&dx, &dy);

	int mx = dx + 0.5;
	int my = dy + 0.5;

	// convert motors position into board coordinates
	int x = mx / 4 - 2;
	int y = 7 - (my / 4);

	if (x < 0)
		x += 12;

	const bool valid_pos = (mx & 3) == 2 && (my & 3) == 2;

	// sensorboard handling is almost the same as fidelity/phantom.cpp
	if (magnet)
	{
		if (valid_pos)
		{
			// pick up piece, unless it was picked up by the user
			const int pos = (y << 4 & 0xf0) | (x & 0x0f);
			if (pos != m_board->get_handpos())
			{
				m_piece_hand = m_board->read_piece(x, y);

				if (m_piece_hand != 0)
				{
					m_board->write_piece(x, y, 0);
					m_board->refresh();
				}
			}
		}
		else
		{
			// pick up piece from internal pieces map
			m_piece_hand = m_pieces_map[my][mx];
			m_pieces_map[my][mx] = 0;
		}
	}
	else if (m_piece_hand != 0)
	{
		if (valid_pos)
		{
			// collision with piece on board (user interference)
			if (m_board->read_piece(x, y) != 0)
				popmessage("Collision at %c%d!", x + 'A', y + 1);
			else
			{
				m_board->write_piece(x, y, m_piece_hand);
				m_board->refresh();
			}
		}
		else
		{
			// collision with internal pieces map (shouldn't happen)
			if (m_pieces_map[my][mx] != 0)
				popmessage("Internal collision!");
			else
				m_pieces_map[my][mx] = m_piece_hand;
		}

		m_piece_hand = 0;
	}
}

TIMER_CALLBACK_MEMBER(mirage_state::motor_count)
{
	const int m = param ? 1 : 0;
	assert(m_motor_control[m] & 2);

	// 1 quarter rotation per period
	int inc = 0;
	if (m_motor_control[m] & 1)
	{
		if (m_motor_pos[m] < m_motor_max[m])
			inc = 1;
	}
	else if (m_motor_pos[m] > 0)
		inc = -1;

	m_motor_remain[m] = m_motor_period;

	if (inc == 0)
		return;

	m_motor_pos[m] += inc;
	m_motor_timer[m]->adjust(m_motor_period, m);

	output_magnet_pos();

	// update quadrature encoder
	static const u8 lut_quad[4] = { 0,1,3,2 };
	m_motor_quad[m] = lut_quad[m_motor_pos[m] & 3];

	m_maincpu->set_input_line(INPUT_LINE_IRQ1 + m, (m_motor_quad[m] & 1) ? ASSERT_LINE : CLEAR_LINE);
}

void mirage_state::motor_control(int m, u8 control)
{
	if (control == m_motor_control[m])
		return;

	// remember remaining time
	if (m_motor_control[m] & 2 && !m_motor_timer[m]->remaining().is_never())
	{
		m_motor_remain[m] = m_motor_timer[m]->remaining();
		m_motor_timer[m]->adjust(attotime::never);
	}

	// invert remaining time if direction flipped
	if ((m_motor_control[m] ^ control) & 1)
		m_motor_remain[m] = m_motor_period - m_motor_remain[m];

	m_motor_control[m] = control;

	// (re)start the timer
	if (control & 2)
		m_motor_timer[m]->adjust(m_motor_remain[m], m);
}



/*******************************************************************************
    I/O
*******************************************************************************/

// LCD

void mirage_state::lcd_pwm_w(offs_t offset, u8 data)
{
	m_out_lcd[offset & 0x3f][offset >> 6] = data;
}

void mirage_state::update_lcd()
{
	u32 lcd_segs = (m_lcd_segs & 0xfff000) | bitswap<12>(m_lcd_segs,7,6,5,4,3,2,1,0,8,9,10,11);

	for (int i = 0; i < 2; i++)
	{
		// LCD common is 0/1/Hi-Z
		const u32 data = BIT(m_lcd_com, i + 2) ? (BIT(m_lcd_com, i) ? ~lcd_segs : lcd_segs) : 0;
		m_lcd_pwm->write_row(i, data);
	}
}

template <int N>
void mirage_state::lcd_segs_w(u8 data)
{
	// P1x, P2x, P3x: LCD segments
	const u8 shift = 8 * N;
	m_lcd_segs = (m_lcd_segs & ~(0xff << shift)) | (data << shift);
	update_lcd();

	// P1x,P20-P23: input mux (chessboard)
	m_inp_mux = (m_inp_mux & 0x3000) | bitswap<12>(~m_lcd_segs,18,19,8,9,10,11,12,13,14,15,16,17);
}


// misc

INPUT_CHANGED_MEMBER(mirage_state::on_button)
{
	m_maincpu->set_input_line(INPUT_LINE_IRQ0, newval ? ASSERT_LINE : CLEAR_LINE);
}

u8 mirage_state::p4_r()
{
	// P40-P47: multiplexed inputs
	u8 data = 0;

	// read buttons
	for (int i = 0; i < 2; i++)
		if (BIT(m_inp_mux, i + 12))
			data |= m_inputs[i]->read();

	// read chessboard
	for (int i = 0; i < 12; i++)
		if (BIT(m_inp_mux, i))
			data |= m_board->read_file(i);

	return bitswap<8>(~data,7,6,5,4,0,1,2,3);
}

u8 mirage_state::p5_r()
{
	// P55: motor Y quadrature B
	return ~(BIT(m_motor_quad[1], 1) << 5);
}

void mirage_state::p5_w(u8 data)
{
	// P52: speaker out
	m_dac->write(BIT(~data, 2));

	// P53: motor Y direction
	// P54: motor Y on
	motor_control(1, data >> 3 & 3);

	for (int i = 0; i < 2; i++)
		m_out_motor[i] = BIT(data, i + 3);
}

u8 mirage_state::p6_r()
{
	// P63: battery status
	// P64: on/off button (IRQ0)
	u8 data = m_inputs[2]->read() << 3 & 0x18;

	// P65: motor X quadrature A (IRQ1)
	// P66: motor Y quadrature A (IRQ2)
	data |= m_motor_quad[0] << 5 & 0x20;
	data |= m_motor_quad[1] << 6 & 0x40;

	return ~data;
}

void mirage_state::p6_w(u8 data)
{
	// P60,P61: input mux (buttons)
	m_inp_mux = (m_inp_mux & 0xfff) | (~data << 12 & 0x3000);
}

u8 mirage_state::p7_r()
{
	// P73: motor X quadrature B
	return ~(BIT(m_motor_quad[0], 1) << 3);
}

void mirage_state::p7_w(offs_t offset, u8 data, u8 mem_mask)
{
	// P70,P71: LCD common
	m_lcd_com = (mem_mask << 2 & 0xc) | (data & 3);
	update_lcd();

	// P74: motor X direction
	// P75: motor X on
	motor_control(0, data >> 4 & 3);

	// P76: electromagnet
	update_piece(BIT(data, 6));

	for (int i = 0; i < 3; i++)
		m_out_motor[i + 2] = BIT(data, i + 4);

	// P77: motor board power?
}



/*******************************************************************************
    Input Ports
*******************************************************************************/

static INPUT_PORTS_START( mirage )
	PORT_START("IN.0")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_H) PORT_CODE(KEYCODE_4) PORT_CODE(KEYCODE_4_PAD) PORT_NAME("Hint / Bishop")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_T) PORT_CODE(KEYCODE_5) PORT_CODE(KEYCODE_5_PAD) PORT_NAME("Takeback / Knight")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_M) PORT_CODE(KEYCODE_6) PORT_CODE(KEYCODE_6_PAD) PORT_NAME("Move / Pawn")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_C) PORT_NAME("Clear")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_L) PORT_CODE(KEYCODE_3) PORT_CODE(KEYCODE_3_PAD) PORT_NAME("Level / Rook")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_V) PORT_CODE(KEYCODE_2) PORT_CODE(KEYCODE_2_PAD) PORT_NAME("Verify / Queen")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_S) PORT_CODE(KEYCODE_1) PORT_CODE(KEYCODE_1_PAD) PORT_NAME("Setup / King")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_N) PORT_NAME("New Game")

	PORT_START("IN.1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_O) PORT_NAME("Option")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_R) PORT_NAME("Replay")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_A) PORT_NAME("Auto / Stop")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_U) PORT_CODE(KEYCODE_RIGHT) PORT_NAME("Multi-Move / Right")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_B) PORT_CODE(KEYCODE_LEFT) PORT_NAME("Black / White / Left")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_E) PORT_NAME("Mode")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("IN.2")
	PORT_CONFNAME( 0x01, 0x00, "Battery Status" )
	PORT_CONFSETTING(    0x01, "Low" )
	PORT_CONFSETTING(    0x00, DEF_STR( Normal ) )
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_F1) PORT_CHANGED_MEMBER(DEVICE_SELF, mirage_state, on_button, 0) PORT_NAME("On / Off")
INPUT_PORTS_END



/*******************************************************************************
    Machine Configs
*******************************************************************************/

void mirage_state::mirage(machine_config &config)
{
	// basic machine hardware
	H83256(config, m_maincpu, 20_MHz_XTAL);
	m_maincpu->nvram_enable_backup(true);
	m_maincpu->standby_cb().set(m_maincpu, FUNC(h83256_device::nvram_set_battery));
	m_maincpu->standby_cb().append([this](int state) { if (state) m_lcd_pwm->clear(); });
	m_maincpu->write_port1().set(FUNC(mirage_state::lcd_segs_w<1>));
	m_maincpu->write_port2().set(FUNC(mirage_state::lcd_segs_w<2>));
	m_maincpu->write_port3().set(FUNC(mirage_state::lcd_segs_w<0>));
	m_maincpu->read_port4().set(FUNC(mirage_state::p4_r));
	m_maincpu->read_port5().set(FUNC(mirage_state::p5_r));
	m_maincpu->write_port5().set(FUNC(mirage_state::p5_w));
	m_maincpu->read_port6().set(FUNC(mirage_state::p6_r));
	m_maincpu->write_port6().set(FUNC(mirage_state::p6_w));
	m_maincpu->read_port7().set(FUNC(mirage_state::p7_r));
	m_maincpu->write_port7().set(FUNC(mirage_state::p7_w));

	SENSORBOARD(config, m_board).set_type(sensorboard_device::BUTTONS);
	m_board->set_size(8+4, 8);
	m_board->clear_cb().set(FUNC(mirage_state::clear_board));
	m_board->init_cb().set(FUNC(mirage_state::init_board));
	m_board->set_delay(attotime::from_msec(150));
	//m_board->set_nvram_enable(true);

	// video hardware
	PWM_DISPLAY(config, m_lcd_pwm).set_size(2, 24);
	m_lcd_pwm->output_x().set(FUNC(mirage_state::lcd_pwm_w));

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_SVG));
	screen.set_refresh_hz(60);
	screen.set_size(1920/5, 697/5);
	screen.set_visarea_full();

	config.set_default_layout(layout_excal_mirage);

	// sound hardware
	SPEAKER(config, "speaker").front_center();
	DAC_1BIT(config, m_dac).add_route(ALL_OUTPUTS, "speaker", 0.25);
}



/*******************************************************************************
    ROM Definitions
*******************************************************************************/

ROM_START( emirage )
	ROM_REGION( 0xc000, "maincpu", 0 )
	ROM_LOAD("1996_7012_excalibur_hd6433256a33p.ic1", 0x0000, 0xc000, CRC(41eed8ea) SHA1(8b5370814d2bfc2d5fcb4ee86c30d676517bcd3a) )

	ROM_REGION( 109652, "screen", 0 )
	ROM_LOAD("regency.svg", 0, 109652, CRC(6840c49e) SHA1(a9c91143c5bea5ab41fe323e719da4a46ab9d631) )
ROM_END

} // anonymous namespace



/*******************************************************************************
    Drivers
*******************************************************************************/

//    YEAR  NAME     PARENT  COMPAT  MACHINE  INPUT   CLASS         INIT        COMPANY, FULLNAME, FLAGS
SYST( 1996, emirage, 0,      0,      mirage,  mirage, mirage_state, empty_init, "Excalibur Electronics", "Mirage (Excalibur)", MACHINE_SUPPORTS_SAVE | MACHINE_MECHANICAL | MACHINE_IMPERFECT_CONTROLS )
