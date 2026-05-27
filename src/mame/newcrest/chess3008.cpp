// license:BSD-3-Clause
// copyright-holders:hap
// thanks-to:Berger
/*******************************************************************************

Newcrest CXG Chess 3008

The hardware and chess engine are very similar to Super Enterprise (model C).

NOTE: It triggers an NMI when the power switch is changed from ON to SAVE.
If this is not done, NVRAM won't save properly.

TODO:
- if/when MAME supports an exit callback, hook up power-off switch to that

Hardware notes:
- PCB label: CXG 3008 600 002
- Hitachi HD6301Y0P (mode 2), 8MHz XTAL
- 2KB battery-backed RAM (KM6816AL-15)
- Sanyo LC7580, 2 LCD panels (each 4-digit, some unused segments)
- chessboard magnet sensors, 64+8 LEDs, piezo

*******************************************************************************/

#include "emu.h"

#include "chess3008_lcd.h"

#include "cpu/m6800/m6801.h"
#include "machine/nvram.h"
#include "machine/sensorboard.h"
#include "sound/dac.h"
#include "video/pwm.h"

#include "speaker.h"

// internal artwork
#include "cxg_chess3008.lh"


namespace {

class chess3008_state : public driver_device
{
public:
	chess3008_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_board(*this, "board"),
		m_lcd(*this, "lcd"),
		m_display(*this, "display"),
		m_dac(*this, "dac"),
		m_inputs(*this, "IN.%u", 0)
	{ }

	DECLARE_INPUT_CHANGED_MEMBER(power_off);

	void chess3008(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	// devices/pointers
	required_device<hd6301y0_cpu_device> m_maincpu;
	required_device<sensorboard_device> m_board;
	optional_device<chess3008_lcd_device> m_lcd;
	required_device<pwm_display_device> m_display;
	required_device<dac_1bit_device> m_dac;
	required_ioport_array<2> m_inputs;

	emu_timer *m_standbytimer;
	u8 m_inp_mux = 0;

	void main_map(address_map &map) ATTR_COLD;

	// I/O handlers
	void control_w(u8 data);
	u8 input_r();
	void leds_w(u8 data);

	TIMER_CALLBACK_MEMBER(set_standby);
};

void chess3008_state::machine_start()
{
	m_standbytimer = timer_alloc(FUNC(chess3008_state::set_standby), this);

	// register for savestates
	save_item(NAME(m_inp_mux));
}



/*******************************************************************************
    Power
*******************************************************************************/

void chess3008_state::machine_reset()
{
	m_maincpu->set_input_line(INPUT_LINE_NMI, CLEAR_LINE);
	m_maincpu->set_input_line(M6801_STBY_LINE, CLEAR_LINE);
}

TIMER_CALLBACK_MEMBER(chess3008_state::set_standby)
{
	m_maincpu->set_input_line(M6801_STBY_LINE, ASSERT_LINE);
}

INPUT_CHANGED_MEMBER(chess3008_state::power_off)
{
	if (newval && !m_maincpu->standby())
	{
		// NMI when power switch is set to SAVE, followed by STBY (internal or STBY pin)
		m_maincpu->set_input_line(INPUT_LINE_NMI, ASSERT_LINE);
		m_standbytimer->adjust(attotime::from_msec(10));
	}
}



/*******************************************************************************
    I/O
*******************************************************************************/

void chess3008_state::control_w(u8 data)
{
	// P20-P23: led select, input mux
	m_inp_mux = data & 0xf;
	m_display->write_my(1 << m_inp_mux);

	// P24: speaker out
	m_dac->write(BIT(data, 4));

	// P25-P27: LC7580 pins
	m_lcd->lcd_w(data >> 5);
}

u8 chess3008_state::input_r()
{
	// P50-P57: multiplexed inputs
	u8 data = 0;

	// read chessboard sensors
	if (m_inp_mux < 8)
		data = m_board->read_rank(m_inp_mux);

	// read other buttons
	else if (m_inp_mux < 10)
		data = m_inputs[m_inp_mux - 8]->read();

	return ~data;
}

void chess3008_state::leds_w(u8 data)
{
	// P60-P67: led data
	m_display->write_mx(~data);
}



/*******************************************************************************
    Address Maps
*******************************************************************************/

void chess3008_state::main_map(address_map &map)
{
	map(0x4000, 0x47ff).mirror(0x3800).ram().share("nvram");
}



/*******************************************************************************
    Input Ports
*******************************************************************************/

static INPUT_PORTS_START( chess3008 )
	PORT_START("IN.0")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_O) PORT_NAME("Move")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_T) PORT_NAME("Take Back")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_6) PORT_CODE(KEYCODE_6_PAD) PORT_NAME("Pawn")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_5) PORT_CODE(KEYCODE_5_PAD) PORT_NAME("Knight")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_4) PORT_CODE(KEYCODE_4_PAD) PORT_NAME("Bishop")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_3) PORT_CODE(KEYCODE_3_PAD) PORT_NAME("Rook")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_2) PORT_CODE(KEYCODE_2_PAD) PORT_NAME("Queen")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_1) PORT_CODE(KEYCODE_1_PAD) PORT_NAME("King")

	PORT_START("IN.1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_M) PORT_NAME("Multi-Move")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_L) PORT_NAME("Level")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_H) PORT_NAME("Hint")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_R) PORT_NAME("Replay")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_I) PORT_NAME("Library")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_S) PORT_CODE(KEYCODE_C) PORT_NAME("Sound/Color")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_E) PORT_NAME("Enter Position")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_N) PORT_NAME("New Game")

	PORT_START("POWER") // needs to be triggered for nvram to work
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_POWER_OFF) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(chess3008_state::power_off), 0)
INPUT_PORTS_END



/*******************************************************************************
    Machine Configs
*******************************************************************************/

void chess3008_state::chess3008(machine_config &config)
{
	// basic machine hardware
	HD6301Y0(config, m_maincpu, 8_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &chess3008_state::main_map);
	m_maincpu->nvram_enable_backup(true);
	m_maincpu->standby_cb().set(m_maincpu, FUNC(hd6301y0_cpu_device::nvram_set_battery));
	m_maincpu->standby_cb().append([this](int state) { if (state) m_display->clear(); });
	m_maincpu->standby_cb().append(m_lcd, FUNC(chess3008_lcd_device::inh_w));
	m_maincpu->out_p2_cb().set(FUNC(chess3008_state::control_w));
	m_maincpu->in_p5_cb().set(FUNC(chess3008_state::input_r));
	m_maincpu->out_p6_cb().set(FUNC(chess3008_state::leds_w));

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	SENSORBOARD(config, m_board).set_type(sensorboard_device::MAGNETS);
	m_board->init_cb().set(m_board, FUNC(sensorboard_device::preset_chess));
	m_board->set_delay(attotime::from_msec(150));
	m_board->set_nvram_enable(true);

	// video hardware
	CHESS3008_LCD(config, m_lcd);

	PWM_DISPLAY(config, m_display).set_size(9, 8);
	config.set_default_layout(layout_cxg_chess3008);

	// sound hardware
	SPEAKER(config, "speaker").front_center();
	DAC_1BIT(config, m_dac).add_route(ALL_OUTPUTS, "speaker", 0.25);
}



/*******************************************************************************
    ROM Definitions
*******************************************************************************/

ROM_START( ch3008 )
	ROM_REGION( 0x4000, "maincpu", 0 )
	ROM_LOAD("1987_3001_cxg_systems_hd6301y0c22p", 0x0000, 0x4000, CRC(63c97c21) SHA1(5ed7d00a7eb335038ad40b6c19b5cc13e274f6d4) )
ROM_END

} // anonymous namespace



/*******************************************************************************
    Drivers
*******************************************************************************/

//    YEAR  NAME    PARENT  COMPAT  MACHINE    INPUT      CLASS            INIT        COMPANY, FULLNAME, FLAGS
SYST( 1987, ch3008, 0,      0,      chess3008, chess3008, chess3008_state, empty_init, "Newcrest Technology / CXG Systems / LogiSoft", "Chess 3008", MACHINE_SUPPORTS_SAVE )
