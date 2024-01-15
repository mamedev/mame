// license:BSD-3-Clause
// copyright-holders:hap
// thanks-to:Sean Riddle
/*******************************************************************************

Saitek Electronic Champion Backgammon

NOTE: Before exiting MAME, change the power switch from GO to STOP. Otherwise,
NVRAM won't save properly.

Hardware notes:
- PCB label: GT4-PE-009
- Hitachi HD6301Y0P @ ~4MHz (LC osc, no XTAL)
- LCD with custom segments
- 24 LEDs, 13*2 buttons sensor board, piezo

TODO:
- if/when MAME supports an exit callback, hook up power-off switch to that
- finish sensorboard handling, internal artwork

*******************************************************************************/

#include "emu.h"

#include "cpu/m6800/m6801.h"
#include "machine/sensorboard.h"
#include "sound/dac.h"
#include "video/pwm.h"

#include "screen.h"
#include "speaker.h"

//#include "saitek_ecbackg.lh"


namespace {

class ecbackg_state : public driver_device
{
public:
	ecbackg_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_board(*this, "board"),
		m_lcd_pwm(*this, "lcd_pwm"),
		m_led_pwm(*this, "led_pwm"),
		m_dac(*this, "dac"),
		m_inputs(*this, "IN.%u", 0),
		m_out_lcd(*this, "s%u.%u", 0U, 0U)
	{ }

	void ecbackg(machine_config &config);

	DECLARE_INPUT_CHANGED_MEMBER(power_off) { if (newval) m_power = false; }

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override { m_power = true; }

private:
	// devices/pointers
	required_device<hd6301y0_cpu_device> m_maincpu;
	required_device<sensorboard_device> m_board;
	required_device<pwm_display_device> m_lcd_pwm;
	required_device<pwm_display_device> m_led_pwm;
	required_device<dac_bit_interface> m_dac;
	required_ioport_array<5> m_inputs;
	output_finder<2, 24> m_out_lcd;

	bool m_power = false;
	u8 m_inp_mux = 0;
	u32 m_lcd_segs = 0;
	u8 m_lcd_com = 0;

	// I/O handlers
	void init_board(int state);
	u8 board_r(u8 row);

	void standby(int state);

	void lcd_pwm_w(offs_t offset, u8 data);
	void update_lcd();
	template <int N> void lcd_segs_w(u8 data);
	void lcd_com_w(u8 data);

	u8 p2_r();
	void p2_w(u8 data);
	u8 p5_r();
	void p6_w(u8 data);
};

void ecbackg_state::machine_start()
{
	m_out_lcd.resolve();

	// register for savestates
	save_item(NAME(m_power));
	save_item(NAME(m_inp_mux));
	save_item(NAME(m_lcd_segs));
	save_item(NAME(m_lcd_com));
}



/*******************************************************************************
    Sensorboard
*******************************************************************************/

void ecbackg_state::init_board(int state)
{
}

u8 ecbackg_state::board_r(u8 row)
{
	u8 data = 0;

	// inputs to sensorboard translation table (0xff is invalid)
	static const u8 lut_board[4*7] =
	{
		 0xff, 0x1b, 0x19, 0x17, 0x07, 0x09, 0x0b,
		 0x0c, 0x1a, 0x18, 0x16, 0x06, 0x08, 0x0a,
		 0x1c, 0x15, 0x13, 0x11, 0x01, 0x03, 0x05,
		 0xff, 0x14, 0x12, 0x10, 0x00, 0x02, 0x04
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

void ecbackg_state::lcd_pwm_w(offs_t offset, u8 data)
{
	m_out_lcd[offset & 0x3f][offset >> 6] = data;
}

void ecbackg_state::update_lcd()
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
void ecbackg_state::lcd_segs_w(u8 data)
{
	// P1x, P3x, P4x: LCD segments
	const u8 shift = 8 * N;
	const u32 mask = 0xff << shift;
	m_lcd_segs = (m_lcd_segs & ~mask) | (data << shift);
	update_lcd();
}

void ecbackg_state::lcd_com_w(u8 data)
{
	// P70-P73: LCD common
	m_lcd_com = data & 0xf;
	update_lcd();
}


// misc

void ecbackg_state::standby(int state)
{
	// clear display
	if (state)
	{
		m_lcd_pwm->clear();
		m_led_pwm->clear();
	}
}

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
	// P20-P22: led select
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
		if (BIT(bitswap<5>(m_inp_mux,6,5,4,3,0), i))
			data |= m_inputs[i]->read();

	// read board
	for (int i = 0; i < 4; i++)
		if (BIT(m_inp_mux, i))
			data |= board_r(i);

	return ~data;
}

void ecbackg_state::p6_w(u8 data)
{
	// P60-P67: input mux, led data
	m_inp_mux = ~data;
	m_led_pwm->write_mx(~data);
}



/*******************************************************************************
    Input Ports
*******************************************************************************/

static INPUT_PORTS_START( ecbackg )
	PORT_START("IN.0")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_S) PORT_NAME("Statistics")

	PORT_START("IN.1")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_B) PORT_NAME("Bear Off")

	PORT_START("IN.2")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_D) PORT_NAME("Double / Accept")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_J) PORT_NAME("Reject")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_Y) PORT_NAME("Play")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_G) PORT_NAME("Game Option")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_L) PORT_NAME("Level")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_T) PORT_NAME("Take Back")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_V) PORT_NAME("Verify")

	PORT_START("IN.3")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_R) PORT_NAME("Roll Dice")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_1) PORT_CODE(KEYCODE_1_PAD) PORT_NAME("Dice 1")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_2) PORT_CODE(KEYCODE_2_PAD) PORT_NAME("Dice 2")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_3) PORT_CODE(KEYCODE_3_PAD) PORT_NAME("Dice 3")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_4) PORT_CODE(KEYCODE_4_PAD) PORT_NAME("Dice 4")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_5) PORT_CODE(KEYCODE_5_PAD) PORT_NAME("Dice 5")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_6) PORT_CODE(KEYCODE_6_PAD) PORT_NAME("Dice 6")

	PORT_START("IN.4")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_N) PORT_NAME("New Game")

	PORT_START("POWER") // needs to be triggered for nvram to work
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_OTHER) PORT_CODE(KEYCODE_F1) PORT_CHANGED_MEMBER(DEVICE_SELF, ecbackg_state, power_off, 0) PORT_NAME("Stop")
INPUT_PORTS_END



/*******************************************************************************
    Machine Configs
*******************************************************************************/

void ecbackg_state::ecbackg(machine_config &config)
{
	// basic machine hardware
	HD6301Y0(config, m_maincpu, 4'000'000); // approximation, no XTAL
	m_maincpu->nvram_enable_backup(true);
	m_maincpu->standby_cb().set(m_maincpu, FUNC(hd6301y0_cpu_device::nvram_set_battery));
	m_maincpu->standby_cb().append(FUNC(ecbackg_state::standby));
	m_maincpu->out_p1_cb().set(FUNC(ecbackg_state::lcd_segs_w<0>));
	m_maincpu->in_p2_cb().set(FUNC(ecbackg_state::p2_r));
	m_maincpu->out_p2_cb().set(FUNC(ecbackg_state::p2_w));
	m_maincpu->out_p3_cb().set(FUNC(ecbackg_state::lcd_segs_w<1>));
	m_maincpu->out_p4_cb().set(FUNC(ecbackg_state::lcd_segs_w<2>));
	m_maincpu->in_p5_cb().set(FUNC(ecbackg_state::p5_r));
	m_maincpu->out_p6_cb().set(FUNC(ecbackg_state::p6_w));
	m_maincpu->out_p7_cb().set(FUNC(ecbackg_state::lcd_com_w));

	SENSORBOARD(config, m_board).set_type(sensorboard_device::BUTTONS);
	m_board->init_cb().set(FUNC(ecbackg_state::init_board));
	m_board->set_size(13, 10);
	m_board->set_spawnpoints(2);
	m_board->set_delay(attotime::from_msec(150));
	m_board->set_nvram_enable(true);

	// video hardware
	PWM_DISPLAY(config, m_lcd_pwm).set_size(2, 24);
	m_lcd_pwm->output_x().set(FUNC(ecbackg_state::lcd_pwm_w));

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_SVG));
	screen.set_refresh_hz(60);
	screen.set_size(1920/3, 518/3);
	screen.set_visarea_full();

	PWM_DISPLAY(config, m_led_pwm).set_size(3, 8);
	//config.set_default_layout(layout_saitek_ecbackg);

	// sound hardware
	SPEAKER(config, "speaker").front_center();
	DAC_1BIT(config, m_dac).add_route(ALL_OUTPUTS, "speaker", 0.25);
}



/*******************************************************************************
    ROM Definitions
*******************************************************************************/

ROM_START( ecbackg )
	ROM_REGION( 0x4000, "maincpu", 0 )
	ROM_LOAD("saitek_89_gx4b_c12_31y0rm79p.u1", 0x0000, 0x4000, CRC(d7278545) SHA1(9ece31cdb237067aeec480c066e0917752697a4d) )

	ROM_REGION( 109665, "screen", 0 )
	ROM_LOAD("ecbackg.svg", 0, 109665, CRC(6592acfe) SHA1(8425d7ce519a921cdbe4298ae9c6789f90d4d0b5) )
ROM_END

} // anonymous namespace



/*******************************************************************************
    Drivers
*******************************************************************************/

//    YEAR  NAME     PARENT  COMPAT  MACHINE  INPUT    CLASS          INIT        COMPANY, FULLNAME, FLAGS
SYST( 1989, ecbackg, 0,      0,      ecbackg, ecbackg, ecbackg_state, empty_init, "Saitek", "Electronic Champion Backgammon", MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK | MACHINE_NOT_WORKING )
