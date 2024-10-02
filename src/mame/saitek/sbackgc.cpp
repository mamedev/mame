// license:BSD-3-Clause
// copyright-holders:hap
// thanks-to:Sean Riddle
/*******************************************************************************

Saitek Sensory Backgammon Computer / Electronic Champion Backgammon

NOTE: Before exiting MAME, change the power switch from GO to STOP. Otherwise,
NVRAM won't save properly. For sbackgc, only turn it off when it's the user's
turn to move.

These were supposedly programmed by Eric van Riet Paap (not sure about Sensory,
but pretty certain about Champion). The Champion program got 3rd place in the
first Computer Olympiad in 1989 (backgammon category). It also includes several
game variations like Jacquet or Trictrac.

MAME's sensorboard interface is a bit different compared to chess. Pieces can be
stacked up to 3. Pressing the Bear Off button (Del / Backspace) will optionally
remove the currently held piece. Capturing pieces is disabled, except when hitting
a blot, in which case the piece in hand is swapped with the one on the board.

Hardware notes:

Sensory Backgammon Computer (model 680):
- PCB label: GT2-PE-018
- Hitachi HD614085S @ ~5MHz (LC osc, no XTAL)
- LCD with custom segments
- 24 LEDs, 13*2 buttons sensor board, piezo

Electronic Champion Backgammon (model 681):
- PCB label: GT4-PE-009
- Hitachi HD6301Y0P @ ~4MHz (LC osc, no XTAL)
- rest is same as Sensory Backgammon Computer

There's also model 082, it looks nearly identical to model 680, but hardware is
unknown. Handheld Champion Backgammon (model 682) has a HD6301Y0F.

TODO:
- is Sensory Backgammon Computer model 082 on a different MCU? maybe NEC uCOM-75?
- if/when MAME supports an exit callback, hook up power-off switch to that

*******************************************************************************/

#include "emu.h"

#include "cpu/hmcs400/hmcs400.h"
#include "cpu/m6800/m6801.h"
#include "machine/sensorboard.h"
#include "sound/dac.h"
#include "video/pwm.h"

#include "screen.h"
#include "speaker.h"

// internal artwork
#include "saitek_ecbackg.lh"
#include "saitek_sbackgc.lh"


namespace {

// shared base class

class sbackgc_base_state : public driver_device
{
public:
	sbackgc_base_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_board(*this, "board"),
		m_led_pwm(*this, "led_pwm"),
		m_lcd_pwm(*this, "lcd_pwm"),
		m_dac(*this, "dac"),
		m_inputs(*this, "IN.%u", 0),
		m_out_lcd(*this, "s%u.%u", 0U, 0U)
	{ }

	void shared(machine_config &config);

	DECLARE_INPUT_CHANGED_MEMBER(init_board);
	DECLARE_INPUT_CHANGED_MEMBER(bear_off);
	DECLARE_INPUT_CHANGED_MEMBER(power_off) { if (newval) m_power = false; }

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override { m_power = true; }

	// devices/pointers
	required_device<cpu_device> m_maincpu;
	required_device<sensorboard_device> m_board;
	required_device<pwm_display_device> m_led_pwm;
	required_device<pwm_display_device> m_lcd_pwm;
	required_device<dac_1bit_device> m_dac;
	required_ioport_array<6> m_inputs;
	output_finder<2, 24> m_out_lcd;

	bool m_power = false;
	u8 m_inp_mux = 0;
	u8 m_led_data = 0;
	u32 m_lcd_segs = 0;
	u8 m_lcd_com = 0;

	// sensorboard handlers
	void init_backgammon();
	void init_jacquet();
	void init_plakoto();
	void board_init_cb(u8 data);
	u8 board_spawn_cb(offs_t offset);
	u8 board_remove_cb();
	u8 board_sensor_cb(offs_t offset);
	u8 board_r(u8 row);

	// shared I/O handlers (just the LCD)
	void clear_display();
	void lcd_pwm_w(offs_t offset, u8 data);
	void update_lcd();
	template <int N> void lcd_segs_w(offs_t offset, u8 data, u8 mem_mask);
	void lcd_com_w(u8 data);
};

void sbackgc_base_state::machine_start()
{
	m_out_lcd.resolve();

	// register for savestates
	save_item(NAME(m_power));
	save_item(NAME(m_led_data));
	save_item(NAME(m_inp_mux));
	save_item(NAME(m_lcd_segs));
	save_item(NAME(m_lcd_com));
}


// Sensory Backgammon Computer

class sbackgc_state : public sbackgc_base_state
{
public:
	sbackgc_state(const machine_config &mconfig, device_type type, const char *tag) :
		sbackgc_base_state(mconfig, type, tag)
	{ }

	void sbackgc(machine_config &config);

private:
	// I/O handlers
	template <int N> void leds_w(u8 data);
	u8 buttons_r();
	u8 input1_r();
	u16 input2_r();
	void control_w(u16 data);
};


// Electronic Champion Backgammon

class ecbackg_state : public sbackgc_base_state
{
public:
	ecbackg_state(const machine_config &mconfig, device_type type, const char *tag) :
		sbackgc_base_state(mconfig, type, tag)
	{ }

	void ecbackg(machine_config &config);

private:
	// I/O handlers
	u8 p2_r();
	void p2_w(u8 data);
	u8 p5_r();
	void p6_w(u8 data);
};



/*******************************************************************************
    Sensorboard
*******************************************************************************/

void sbackgc_base_state::init_backgammon()
{
	// initial position for Backgammon (default)
	for (int i = 0; i < 5; i++)
	{
		m_board->write_piece(6, i, 1);
		m_board->write_piece(0, i, 4);
		m_board->write_piece(0, 9 - i, 1);
		m_board->write_piece(6, 9 - i, 4);

		if (i < 3)
		{
			m_board->write_piece(4, i, 1);
			m_board->write_piece(4, 9 - i, 4);
		}

		if (i < 2)
		{
			m_board->write_piece(11, 9 - i, 1);
			m_board->write_piece(11, i, 4);
		}
	}
}

void sbackgc_base_state::init_jacquet()
{
	// initial position for Jacquet / Trictrac / Moultezim
	for (int i = 0; i < 5; i++)
	{
		m_board->write_piece(11, 9 - i, 3);
		m_board->write_piece(0, i, 6);
	}
}

void sbackgc_base_state::init_plakoto()
{
	// initial position for Plakoto
	for (int i = 0; i < 5; i++)
	{
		m_board->write_piece(11, 9 - i, 3);
		m_board->write_piece(11, i, 6);
	}
}

INPUT_CHANGED_MEMBER(sbackgc_base_state::init_board)
{
	if (!newval)
		return;

	m_board->cancel_sensor();
	m_board->cancel_hand();
	m_board->clear_board();

	// 3 possible initial board positions
	switch (param)
	{
		case 0:
			init_backgammon();
			break;

		case 1:
			init_jacquet();
			break;

		case 2:
			init_plakoto();
			break;

		default:
			break;
	}

	m_board->refresh();
}

void sbackgc_base_state::board_init_cb(u8 data)
{
	if (~data & 1)
		init_backgammon();
}

u8 sbackgc_base_state::board_spawn_cb(offs_t offset)
{
	return (offset == 1) ? 1 : 4;
}

INPUT_CHANGED_MEMBER(sbackgc_base_state::bear_off)
{
	if (newval && m_inputs[5]->read() & 1)
	{
		// remove piece when Bear Off button is pressed
		board_remove_cb();
		m_board->refresh();
	}
}

u8 sbackgc_base_state::board_remove_cb()
{
	int handpos = m_board->get_handpos();

	if (handpos != -1)
	{
		u8 x = handpos & 0xf;
		u8 y = handpos >> 4 & 0xf;
		u8 piece = m_board->read_piece(x, y);

		m_board->remove_hand();

		// decrement piece
		if (piece != 1 && piece != 4)
			m_board->write_piece(x, y, piece - 1);
	}
	else
		m_board->cancel_hand();

	// block default sensorboard handling
	return 1;
}

u8 sbackgc_base_state::board_sensor_cb(offs_t offset)
{
	u8 x = offset & 0xf;
	u8 y = offset >> 4 & 0xf;
	u8 piece = m_board->read_piece(x, y);

	u8 hand = m_board->get_hand();

	// drop piece
	if (hand != 0)
	{
		// drop piece on same spot
		if (offset == m_board->get_handpos())
		{
			m_board->cancel_hand();
			return 1;
		}

		if (piece != 0)
		{
			u8 piece_color = (piece < 4) ? 1 : 4;
			u8 piece_index = piece - piece_color;

			// invalid drop (overflow, or onto a stack of the opposite color)
			if (piece_index == 2 || ((hand != piece_color) && piece_index == 1))
				return 1;

			board_remove_cb();

			// swap piece (blot)
			if ((hand != piece_color) && piece_index == 0)
			{
				m_board->write_piece(x, y, hand);
				m_board->set_hand(piece_color);
			}

			// increment piece
			else
				m_board->write_piece(x, y, piece + 1);
		}
		else
		{
			// drop piece on empty spot
			board_remove_cb();
			m_board->write_piece(x, y, hand);
		}
	}

	// pick up piece
	else if (piece != 0)
	{
		// white or black piece
		piece = (piece < 4) ? 1 : 4;

		m_board->set_hand(piece);
		m_board->set_handpos(offset);
	}

	// block default sensorboard handling
	return 3;
}

u8 sbackgc_base_state::board_r(u8 row)
{
	u8 data = 0;

	// inputs to sensorboard translation table (0xff is invalid)
	static const u8 lut_board[4*7] =
	{
		0x0c, 0x1a, 0x18, 0x16, 0x06, 0x08, 0x0a,
		0x1c, 0x15, 0x13, 0x11, 0x01, 0x03, 0x05,
		0xff, 0x14, 0x12, 0x10, 0x00, 0x02, 0x04,
		0xff, 0x1b, 0x19, 0x17, 0x07, 0x09, 0x0b
	};

	for (int i = 0; i < 7; i++)
	{
		const u8 pos = lut_board[row * 7 + i];
		const u8 x = pos & 0xf;
		const u8 y = pos >> 4;

		// 5 rows per button
		int state = 0;
		for (int j = 0; j < 5; j++)
			state |= m_board->read_sensor(x, y * 5 + j);

		data = data << 1 | state;
	}

	return data;
}



/*******************************************************************************
    I/O
*******************************************************************************/

// LCD

void sbackgc_base_state::clear_display()
{
	m_lcd_pwm->clear();
	m_led_pwm->clear();
}

void sbackgc_base_state::lcd_pwm_w(offs_t offset, u8 data)
{
	m_out_lcd[offset & 0x3f][offset >> 6] = data;
}

void sbackgc_base_state::update_lcd()
{
	for (int i = 0; i < 2; i++)
	{
		// LCD common is analog (voltage level)
		const u8 com = population_count_32(m_lcd_com >> (i * 2) & 3);
		const u32 data = (com == 0) ? m_lcd_segs : (com == 2) ? ~m_lcd_segs : 0;
		m_lcd_pwm->write_row(i, data);
	}
}

template <int N>
void sbackgc_base_state::lcd_segs_w(offs_t offset, u8 data, u8 mem_mask)
{
	if (mem_mask == 0)
		return;

	// R3x-R8x: LCD segments (P1x,P3x,P4x on ecbackg)
	const u8 width = 32 - count_leading_zeros_32(mem_mask);
	const u8 mask = (1 << width) - 1;
	const u8 shift = width * N;

	m_lcd_segs = (m_lcd_segs & ~(mask << shift)) | ((data & mask) << shift);
	update_lcd();
}

void sbackgc_base_state::lcd_com_w(u8 data)
{
	// R00-R03: LCD common (P70-P73 on ecbackg)
	m_lcd_com = data & 0xf;
	update_lcd();
}


// sbackgc-specific

template <int N>
void sbackgc_state::leds_w(u8 data)
{
	// R1x,R2x: LED data
	const u8 shift = 4 * N;
	m_led_data = (m_led_data & ~(0xf << shift)) | (data << shift);
	m_led_pwm->write_mx(m_led_data);
}

u8 sbackgc_state::buttons_r()
{
	u8 data = 0xf;

	for (int i = 0; i < 4; i++)
		if (m_inp_mux & m_inputs[i]->read())
			data ^= 1 << i;

	return data;
}

u8 sbackgc_state::input1_r()
{
	// R90-R93: multiplexed inputs
	u8 data = 0xf;

	// read board
	for (int i = 0; i < 4; i++)
		if (m_inp_mux & board_r(i))
			data ^= 1 << i;

	// read buttons (high)
	return data & (buttons_r() | 3);
}

u16 sbackgc_state::input2_r()
{
	// D11,D12: read buttons (low)
	u16 data = buttons_r() << 11 & 0x1800;

	// D13: power switch state
	data |= m_power ? 0x2000 : 0;

	// D15: freq sel (3MHz or 5MHz)
	return data | 0x800f;
}

void sbackgc_state::control_w(u16 data)
{
	// D0-D2: LED select
	m_led_pwm->write_my(~data & 7);

	// D4-D10: input mux
	m_inp_mux = ~data >> 4 & 0x7f;

	// D14: speaker out
	m_dac->write(BIT(data, 14));
}


// ecbackg-specific

u8 ecbackg_state::p2_r()
{
	// P27: power switch state
	u8 data = m_power ? 0 : 0x80;

	// P24: P57 (unused)
	data |= ~p5_r() >> 3 & 0x10;
	return ~data;
}

void ecbackg_state::p2_w(u8 data)
{
	// P20-P22: LED select
	m_led_pwm->write_my(data & 7);

	// P23: speaker out
	m_dac->write(BIT(data, 3));
}

u8 ecbackg_state::p5_r()
{
	// P50-P56: multiplexed inputs
	u8 data = 0;

	// read buttons
	for (int i = 0; i < 5; i++)
		if (BIT(bitswap<5>(m_inp_mux,6,0,3,5,4), i))
			data |= m_inputs[i]->read();

	// read board
	for (int i = 0; i < 4; i++)
		if (BIT(bitswap<4>(m_inp_mux,0,3,2,1), i))
			data |= board_r(i);

	return ~data;
}

void ecbackg_state::p6_w(u8 data)
{
	// P60-P67: input mux, LED data
	m_inp_mux = ~data;
	m_led_pwm->write_mx(~data);
}



/*******************************************************************************
    Input Ports
*******************************************************************************/

static INPUT_PORTS_START( sbackgc )
	PORT_START("IN.0")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_D) PORT_CODE(KEYCODE_A) PORT_NAME("Double / Accept")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_J) PORT_NAME("Reject")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_P) PORT_NAME("Play")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_O) PORT_NAME("Game Option")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_L) PORT_NAME("Level")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_T) PORT_NAME("Take Back")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_V) PORT_NAME("Verify")

	PORT_START("IN.1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_R) PORT_CODE(KEYCODE_ENTER) PORT_CODE(KEYCODE_ENTER_PAD) PORT_NAME("Roll Dice")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_1) PORT_CODE(KEYCODE_1_PAD) PORT_NAME("Dice 1")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_2) PORT_CODE(KEYCODE_2_PAD) PORT_NAME("Dice 2")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_3) PORT_CODE(KEYCODE_3_PAD) PORT_NAME("Dice 3")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_4) PORT_CODE(KEYCODE_4_PAD) PORT_NAME("Dice 4")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_5) PORT_CODE(KEYCODE_5_PAD) PORT_NAME("Dice 5")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_6) PORT_CODE(KEYCODE_6_PAD) PORT_NAME("Dice 6")

	PORT_START("IN.2")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_B) PORT_CODE(KEYCODE_DEL) PORT_CODE(KEYCODE_BACKSPACE) PORT_CHANGED_MEMBER(DEVICE_SELF, sbackgc_base_state, bear_off, 0) PORT_NAME("Bear Off")

	PORT_START("IN.3")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_S) PORT_NAME("Set Up")

	PORT_START("IN.4")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_N) PORT_NAME("New Game")

	PORT_START("IN.5")
	PORT_CONFNAME( 0x01, 0x01, "Remove Piece on Bear Off" )
	PORT_CONFSETTING(    0x00, DEF_STR( No ) )
	PORT_CONFSETTING(    0x01, DEF_STR( Yes ) )

	PORT_START("POWER") // needs to be triggered for nvram to work
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_POWER_OFF) PORT_CHANGED_MEMBER(DEVICE_SELF, sbackgc_base_state, power_off, 0) PORT_NAME("Stop")

	PORT_START("BOARD")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_OTHER) PORT_CHANGED_MEMBER(DEVICE_SELF, sbackgc_base_state, init_board, 0) PORT_NAME("Board Reset Backgammon")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_OTHER) PORT_CHANGED_MEMBER(DEVICE_SELF, sbackgc_base_state, init_board, 1) PORT_NAME("Board Reset Jacquet")
INPUT_PORTS_END

static INPUT_PORTS_START( ecbackg )
	PORT_INCLUDE( sbackgc )

	PORT_MODIFY("IN.3")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_S) PORT_NAME("Statistics")

	PORT_MODIFY("BOARD")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_OTHER) PORT_CHANGED_MEMBER(DEVICE_SELF, sbackgc_base_state, init_board, 2) PORT_NAME("Board Reset Plakoto")
INPUT_PORTS_END



/*******************************************************************************
    Machine Configs
*******************************************************************************/

void sbackgc_base_state::shared(machine_config &config)
{
	// basic machine hardware
	SENSORBOARD(config, m_board).set_type(sensorboard_device::BUTTONS);
	m_board->init_cb().set(FUNC(sbackgc_base_state::board_init_cb));
	m_board->spawn_cb().set(FUNC(sbackgc_base_state::board_spawn_cb));
	m_board->remove_cb().set(FUNC(sbackgc_base_state::board_remove_cb));
	m_board->sensor_cb().set(FUNC(sbackgc_base_state::board_sensor_cb));
	m_board->set_size(13, 10);
	m_board->set_spawnpoints(2);
	m_board->set_max_id(6);
	m_board->set_delay(attotime::from_msec(150));
	m_board->set_nvram_enable(true);

	// video hardware
	PWM_DISPLAY(config, m_lcd_pwm).set_size(2, 24);
	m_lcd_pwm->output_x().set(FUNC(sbackgc_base_state::lcd_pwm_w));

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_SVG));
	screen.set_refresh_hz(60);
	screen.set_size(1920/5, 518/5);
	screen.set_visarea_full();

	PWM_DISPLAY(config, m_led_pwm).set_size(3, 8);

	// sound hardware
	SPEAKER(config, "speaker").front_center();
	DAC_1BIT(config, m_dac).add_route(ALL_OUTPUTS, "speaker", 0.25);
}

void sbackgc_state::sbackgc(machine_config &config)
{
	// basic machine hardware
	hmcs400_cpu_device &maincpu(HD614085(config, m_maincpu, 5'000'000)); // approximation, no XTAL
	maincpu.nvram_enable_backup(true);
	maincpu.stop_cb().set(maincpu, FUNC(hmcs400_cpu_device::nvram_set_battery));
	maincpu.stop_cb().append([this](int state) { if (state) clear_display(); });
	maincpu.write_r<0x0>().set(FUNC(sbackgc_state::lcd_com_w));
	maincpu.write_r<0x1>().set(FUNC(sbackgc_state::leds_w<0>));
	maincpu.write_r<0x2>().set(FUNC(sbackgc_state::leds_w<1>));
	maincpu.write_r<0x3>().set(FUNC(sbackgc_state::lcd_segs_w<0>));
	maincpu.write_r<0x4>().set(FUNC(sbackgc_state::lcd_segs_w<1>));
	maincpu.write_r<0x5>().set(FUNC(sbackgc_state::lcd_segs_w<2>));
	maincpu.write_r<0x6>().set(FUNC(sbackgc_state::lcd_segs_w<3>));
	maincpu.write_r<0x7>().set(FUNC(sbackgc_state::lcd_segs_w<4>));
	maincpu.write_r<0x8>().set(FUNC(sbackgc_state::lcd_segs_w<5>));
	maincpu.read_r<0x9>().set(FUNC(sbackgc_state::input1_r));
	maincpu.read_r<0xa>().set_ioport("IN.4").exor(1);
	maincpu.write_d().set(FUNC(sbackgc_state::control_w));
	maincpu.read_d().set(FUNC(sbackgc_state::input2_r));

	shared(config);

	m_lcd_pwm->set_bri_levels(0.05);
	config.set_default_layout(layout_saitek_sbackgc);
}

void ecbackg_state::ecbackg(machine_config &config)
{
	// basic machine hardware
	hd6301y0_cpu_device &maincpu(HD6301Y0(config, m_maincpu, 4'000'000)); // approximation, no XTAL
	maincpu.nvram_enable_backup(true);
	maincpu.standby_cb().set(maincpu, FUNC(hd6301y0_cpu_device::nvram_set_battery));
	maincpu.standby_cb().append([this](int state) { if (state) clear_display(); });
	maincpu.out_p1_cb().set(FUNC(ecbackg_state::lcd_segs_w<0>));
	maincpu.in_p2_cb().set(FUNC(ecbackg_state::p2_r));
	maincpu.out_p2_cb().set(FUNC(ecbackg_state::p2_w));
	maincpu.out_p3_cb().set(FUNC(ecbackg_state::lcd_segs_w<1>));
	maincpu.out_p4_cb().set(FUNC(ecbackg_state::lcd_segs_w<2>));
	maincpu.in_p5_cb().set(FUNC(ecbackg_state::p5_r));
	maincpu.out_p6_cb().set(FUNC(ecbackg_state::p6_w));
	maincpu.out_p7_cb().set(FUNC(ecbackg_state::lcd_com_w));

	shared(config);

	config.set_default_layout(layout_saitek_ecbackg);
}



/*******************************************************************************
    ROM Definitions
*******************************************************************************/

ROM_START( sbackgc )
	ROM_REGION( 0x4000, "maincpu", 0 )
	ROM_LOAD("1988_gx2_saitek_614085sc72.u1", 0x0000, 0x4000, CRC(0e91d7a1) SHA1(fa147105fa99d01210b53389b8b70031cff6ad66) )

	ROM_REGION( 109526, "screen", 0 )
	ROM_LOAD("sbackgc.svg", 0, 109526, CRC(183b12fb) SHA1(5a3682d6ff28086c2ab890eee1403c8c3d7d8527) )
ROM_END

ROM_START( ecbackg )
	ROM_REGION( 0x4000, "maincpu", 0 )
	ROM_LOAD("saitek_89_gx4b_c12_31y0rm79p.u1", 0x0000, 0x4000, CRC(d7278545) SHA1(9ece31cdb237067aeec480c066e0917752697a4d) )

	ROM_REGION( 109526, "screen", 0 )
	ROM_LOAD("ecbackg.svg", 0, 109526, CRC(b8149d74) SHA1(0cc6f1a2c50f53f8d2be73379019d275799d0546) )
ROM_END

} // anonymous namespace



/*******************************************************************************
    Drivers
*******************************************************************************/

//    YEAR  NAME     PARENT  COMPAT  MACHINE  INPUT    CLASS          INIT        COMPANY, FULLNAME, FLAGS
SYST( 1988, sbackgc, 0,      0,      sbackgc, sbackgc, sbackgc_state, empty_init, "Saitek", "Sensory Backgammon Computer", MACHINE_SUPPORTS_SAVE )

SYST( 1989, ecbackg, 0,      0,      ecbackg, ecbackg, ecbackg_state, empty_init, "Saitek", "Electronic Champion Backgammon", MACHINE_SUPPORTS_SAVE )
