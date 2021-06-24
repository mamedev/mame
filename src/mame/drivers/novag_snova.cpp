// license:BSD-3-Clause
// copyright-holders:hap
// thanks-to:Berger, bataais
/******************************************************************************

Novag Super Nova & related chess computers. I believe the series started with
Primo. The chess engine is by David Kittinger.

TODO:
- NMI on power-off switch, it sets 0x14 bit 7 for standby power (see below)
- add nvram, MCU is missing standby power emulation
- beeps are glitchy, as if interrupted for too long
- nsnova serial port isn't working, MCU emulation problem?
- nsnova unmapped reads from 0x33/0x34
- is the 1st version of supremo(black plastic) the same ROM?

===============================================================================

Novag Super Nova (model 904)
----------------------------

Hardware notes:
- Hitachi HD63A03YP MCU @ 16MHz (4MHz internal)
- 32KB ROM(TC57256AD-12), 8KB RAM(CXK58648P-10L)
- LCD with 4 digits and custom segments, no LCD chip
- RJ-12 port for Novag Super System (like the one in sexpertc)
- buzzer, 16 LEDs, 8*8 chessboard buttons

Older versions had a bug in the opening moves, always playing B5 after D4.

===============================================================================

Novag Supremo (model 881)
-------------------------

Hardware notes:
- Hitachi HD63A03YP MCU @ 8MHz (2MHz internal)
- 32KB ROM(TC57256AD-12), 2KB RAM(TC5516APL)
- LCD with 4 digits and custom segments, no LCD chip
- buzzer, 16 LEDs, 8*8 chessboard buttons

Supremo also had a "limited edition" rerelease in 1990, plastic is fake-wood
instead of black and backpanel sticker is gold, otherwise it's the same game.
The model number is still 881, ROM is the same as the standard fake-wood version.

******************************************************************************/

#include "emu.h"

#include "bus/rs232/rs232.h"
#include "cpu/m6800/m6801.h"
#include "machine/sensorboard.h"
#include "sound/dac.h"
#include "video/pwm.h"

#include "screen.h"
#include "speaker.h"

// internal artwork
#include "novag_snova.lh" // clickable
#include "novag_supremo.lh" // clickable


namespace {

class snova_state : public driver_device
{
public:
	snova_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_board(*this, "board"),
		m_lcd_pwm(*this, "lcd_pwm"),
		m_led_pwm(*this, "led_pwm"),
		m_dac(*this, "dac"),
		m_rs232(*this, "rs232"),
		m_inputs(*this, "IN.%u", 0),
		m_out_lcd(*this, "s%u.%u", 0U, 0U)
	{ }

	// machine configs
	void snova(machine_config &config);
	void supremo(machine_config &config);

protected:
	virtual void machine_start() override;

private:
	// devices/pointers
	required_device<hd6303y_cpu_device> m_maincpu;
	required_device<sensorboard_device> m_board;
	required_device<pwm_display_device> m_lcd_pwm;
	required_device<pwm_display_device> m_led_pwm;
	required_device<dac_bit_interface> m_dac;
	optional_device<rs232_port_device> m_rs232;
	required_ioport_array<2> m_inputs;
	output_finder<4, 10> m_out_lcd;

	void snova_map(address_map &map);
	void supremo_map(address_map &map);

	void lcd_pwm_w(offs_t offset, u8 data);
	void update_leds();

	u8 p2_r();
	void p2_w(u8 data);
	void p5_w(u8 data);
	void p6_w(u8 data);

	bool m_lcd_strobe = false;
	u8 m_inp_mux = 0;
	u8 m_select = 0;
	u8 m_led_data = 0;
};

void snova_state::machine_start()
{
	m_out_lcd.resolve();

	// register for savestates
	save_item(NAME(m_lcd_strobe));
	save_item(NAME(m_inp_mux));
	save_item(NAME(m_select));
	save_item(NAME(m_led_data));
}



/******************************************************************************
    I/O
******************************************************************************/

// misc

void snova_state::lcd_pwm_w(offs_t offset, u8 data)
{
	m_out_lcd[offset & 0x3f][offset >> 6] = data;
}

void snova_state::update_leds()
{
	m_led_pwm->matrix(m_select >> 4 & 3, m_led_data);
}


// MCU ports

u8 snova_state::p2_r()
{
	// P20: 4051 Z
	u8 data = 0;

	// read chessboard buttons
	for (int i = 0; i < 8; i++)
		if (BIT(m_led_data, i))
			data |= BIT(m_board->read_file(i), m_inp_mux);

	// read keypad buttons
	for (int i = 0; i < 2; i++)
		if (BIT(m_select, i + 6))
			data |= BIT(m_inputs[i]->read(), m_inp_mux);

	// P23: serial rx
	if (m_rs232)
		data |= m_rs232->rxd_r() << 3;

	return data ^ 1;
}

void snova_state::p2_w(u8 data)
{
	// P21: 4066 in/out to LCD
	if (m_lcd_strobe && ~data & 2)
	{
		u16 lcd_data = (m_select << 2 & 0x300) | m_led_data;
		m_lcd_pwm->matrix(m_select & 0xf, lcd_data);
	}
	m_lcd_strobe = bool(data & 2);

	// P22: speaker out
	m_dac->write(BIT(data, 2));

	// P24: serial tx
	if (m_rs232)
		m_rs232->write_txd(BIT(~data, 4));

	// P25-P27: 4051 S0-S2
	// 4051 Y0-Y7: multiplexed inputs
	m_inp_mux = data >> 5 & 7;
}

void snova_state::p5_w(u8 data)
{
	// P50-P53: 4066 control to LCD
	// P54,P55: led select
	// P56,P57: keypad mux, lcd data
	m_select = data ^ 0xf0;
	update_leds();
}

void snova_state::p6_w(u8 data)
{
	// P60-P67: led data, lcd data, chessboard mux
	m_led_data = ~data;
	update_leds();
}



/******************************************************************************
    Address Maps
******************************************************************************/

void snova_state::supremo_map(address_map &map)
{
	map(0x0000, 0x0027).m(m_maincpu, FUNC(hd6303y_cpu_device::hd6301y_io));
	map(0x0040, 0x013f).ram(); // internal
	map(0x4000, 0x47ff).ram();
	map(0x8000, 0xffff).rom();
}

void snova_state::snova_map(address_map &map)
{
	supremo_map(map);
	map(0x4000, 0x5fff).ram();
}



/******************************************************************************
    Input Ports
******************************************************************************/

static INPUT_PORTS_START( snova )
	PORT_START("IN.0")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_8) PORT_NAME("Trace Back / Next Best")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_7) PORT_NAME("Trace Forward / Auto Play")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_6) PORT_NAME("Set Level / Pawn")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_5) PORT_NAME("Info / Echo / Knight")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_4) PORT_NAME("Easy / Moves / Bishop")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_3) PORT_NAME("Solve Mate / Language / Rook")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_2) PORT_NAME("Sound / Game / Queen")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_1) PORT_NAME("Referee / Board / King")

	PORT_START("IN.1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_I) PORT_NAME("Go")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_U) PORT_NAME("Restore")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_Y) PORT_NAME("Hint / Human")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_T) PORT_NAME("Clear Board / Clear")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_R) PORT_NAME("Color / Video")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_E) PORT_NAME("Set Up / Verify")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_W) PORT_NAME("Random / Auto Clock")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_Q) PORT_NAME("New Game")
INPUT_PORTS_END

static INPUT_PORTS_START( supremo )
	PORT_START("IN.0")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_1) PORT_NAME("Trace Back / Next Best")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_2) PORT_NAME("Trace Forward / Auto Play")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_3) PORT_NAME("Set Level / Pawn")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_4) PORT_NAME("Info / Knight")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_5) PORT_NAME("Easy / Print Moves / Bishop")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_6) PORT_NAME("Solve Mate / Print Eval / Rook")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_7) PORT_NAME("Sound / Print List / Queen")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_8) PORT_NAME("Referee / Print Board / King")

	PORT_START("IN.1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_Q) PORT_NAME("Go")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_W) PORT_NAME("Restore")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_E) PORT_NAME("Hint")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_R) PORT_NAME("Clear / Clear Board")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_T) PORT_NAME("Color")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_Y) PORT_NAME("Verify / Set Up")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_U) PORT_NAME("Random")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_I) PORT_NAME("New Game")
INPUT_PORTS_END



/******************************************************************************
    Machine Configs
******************************************************************************/

void snova_state::snova(machine_config &config)
{
	/* basic machine hardware */
	HD6303Y(config, m_maincpu, 16_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &snova_state::snova_map);
	m_maincpu->in_p2_cb().set(FUNC(snova_state::p2_r));
	m_maincpu->out_p2_cb().set(FUNC(snova_state::p2_w));
	m_maincpu->out_p5_cb().set(FUNC(snova_state::p5_w));
	m_maincpu->out_p6_cb().set(FUNC(snova_state::p6_w));

	SENSORBOARD(config, m_board).set_type(sensorboard_device::BUTTONS);
	m_board->init_cb().set(m_board, FUNC(sensorboard_device::preset_chess));
	m_board->set_delay(attotime::from_msec(350));

	/* video hardware */
	PWM_DISPLAY(config, m_lcd_pwm).set_size(4, 10);
	m_lcd_pwm->output_x().set(FUNC(snova_state::lcd_pwm_w));

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_SVG));
	screen.set_refresh_hz(60);
	screen.set_size(1920/4, 591/4);
	screen.set_visarea_full();

	PWM_DISPLAY(config, m_led_pwm).set_size(2, 8);
	config.set_default_layout(layout_novag_snova);

	/* sound hardware */
	SPEAKER(config, "speaker").front_center();
	DAC_1BIT(config, m_dac).add_route(ALL_OUTPUTS, "speaker", 0.25);

	/* rs232 (configure after video) */
	RS232_PORT(config, m_rs232, default_rs232_devices, nullptr);
}

void snova_state::supremo(machine_config &config)
{
	snova(config);

	/* basic machine hardware */
	m_maincpu->set_clock(8_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &snova_state::supremo_map);

	config.set_default_layout(layout_novag_supremo);

	config.device_remove("rs232");
}



/******************************************************************************
    ROM Definitions
******************************************************************************/

ROM_START( nsnova ) // ID = N1.05
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("n_530.u5", 0x8000, 0x8000, CRC(727a0ada) SHA1(129c1edc5c1d2e12ce97ebef81c6d5555464a11d) )

	ROM_REGION( 50926, "screen", 0 )
	ROM_LOAD("nsnova.svg", 0, 50926, CRC(5ffa1b53) SHA1(8b1f862bfdf0be837a4e8dc94fea592d6ffff629) )
ROM_END

ROM_START( supremo )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("sp_a10.u5", 0x8000, 0x8000, CRC(1db63786) SHA1(4f24452ed8955b31ba88f68cc95c357660930aa4) )

	ROM_REGION( 50926, "screen", 0 )
	ROM_LOAD("nsnova.svg", 0, 50926, CRC(5ffa1b53) SHA1(8b1f862bfdf0be837a4e8dc94fea592d6ffff629) )
ROM_END

} // anonymous namespace



/******************************************************************************
    Drivers
******************************************************************************/

//    YEAR  NAME     PARENT  COMPAT  MACHINE  INPUT    CLASS        INIT        COMPANY, FULLNAME, FLAGS
CONS( 1990, nsnova,  0,      0,      snova,   snova,   snova_state, empty_init, "Novag", "Super Nova (Novag, v1.05)", MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK )

CONS( 1988, supremo, 0,      0,      supremo, supremo, snova_state, empty_init, "Novag", "Supremo", MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK )
