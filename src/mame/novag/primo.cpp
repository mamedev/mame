// license:BSD-3-Clause
// copyright-holders:hap
// thanks-to:Berger, bataais
/*******************************************************************************

Novag Primo & related chess computers. The chess engine is by David Kittinger.

NOTE: Turn the power switch off before exiting MAME, otherwise NVRAM won't save
properly.

TODO:
- if/when MAME supports an exit callback, hook up power-off switch to that
- Novag Super System peripherals don't work on nsnova due to serial clock drift,
  baud rate differs a bit between host and client, m6801 serial emulation issue
- unmapped reads from 0x3c/0x3d (primo/supremo) or 0x33/0x34 (nsnova)
- supremo unmapped writes to 0x2000/0x6000, always 0?
- is the 1st version of supremo(black plastic) the same ROM?
- is "Aquamarine / Super Nova" the same ROM as nsnova and just a redesign?

BTANB:
- primo has the same bug as nvip, where if the user presses Go right after
  entering a move during the opening, the CPU opponent will answer by playing
  a move with white

================================================================================

Novag Primo (model 871)
-----------------------

Hardware notes:
- PCB label: 100059/100060
- Hitachi HD6301Y0P (mode 2) @ 8MHz
- 2KB RAM(M5M5118P)
- LCD with 4 7segs and custom segments, no LCD chip
- buzzer, 16 LEDs, 8*8 chessboard buttons

The LCD is the same as the one in VIP / Super VIP.

================================================================================

Novag Supremo (model 881)
-------------------------

Hardware notes:
- PCB label: 100090
- Hitachi HD63A03YP @ 8MHz
- 32KB ROM(TC57256AD-12), 2KB RAM(TC5516APL)
- rest is same as Primo

Supremo also had a "limited edition" rerelease in 1990, plastic is fake-wood
instead of black and backpanel sticker is gold, otherwise it's the same game.
The model number is still 881, ROM is the same as the standard fake-wood version.

================================================================================

Novag Super Nova (model 904)
----------------------------

Hardware notes:
- Hitachi HD63A03YP (or HD6301Y0P in mode 1) @ 16MHz
- 32KB ROM(TC57256AD-12), 8KB RAM(CXK58648P-10L)
- LCD with 4 7segs and custom segments, no LCD chip
- RJ-12 port for Novag Super System (like the one in nsvip/sexpertc)
- buzzer, 16 LEDs, 8*8 chessboard buttons

Older versions had a bug in the opening moves, always playing B5 after D4.

The program is very similar to Super VIP, it could be said that Super Nova is
the Super VIP combined with the Novag Super System Touch Sensory board.

*******************************************************************************/

#include "emu.h"

#include "bus/rs232/rs232.h"
#include "cpu/m6800/m6801.h"
#include "machine/nvram.h"
#include "machine/sensorboard.h"
#include "sound/dac.h"
#include "video/pwm.h"

#include "screen.h"
#include "speaker.h"

// internal artwork
#include "novag_primo.lh"
#include "novag_snova.lh"
#include "novag_supremo.lh"


namespace {

class primo_state : public driver_device
{
public:
	primo_state(const machine_config &mconfig, device_type type, const char *tag) :
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

	void primo(machine_config &config);
	void supremo(machine_config &config);
	void snova(machine_config &config);

	DECLARE_INPUT_CHANGED_MEMBER(primo_power_off) { if (newval) m_power = false; }
	DECLARE_INPUT_CHANGED_MEMBER(snova_power_off);

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override { m_power = true; }

private:
	// devices/pointers
	required_device<hd6301y_cpu_device> m_maincpu;
	required_device<sensorboard_device> m_board;
	required_device<pwm_display_device> m_lcd_pwm;
	required_device<pwm_display_device> m_led_pwm;
	required_device<dac_1bit_device> m_dac;
	optional_device<rs232_port_device> m_rs232;
	required_ioport_array<2> m_inputs;
	output_finder<4, 10> m_out_lcd;

	bool m_power = false;
	bool m_lcd_strobe = false;
	u8 m_inp_mux = 0;
	u8 m_select = 0;
	u8 m_led_data = 0;

	void primo_map(address_map &map);
	void supremo_map(address_map &map);
	void snova_map(address_map &map);

	// I/O handlers
	void standby(int state);
	void lcd_pwm_w(offs_t offset, u8 data);
	void update_leds();

	u8 p2_r();
	void p2_w(u8 data);
	void p5_w(u8 data);
	void p6_w(u8 data);
};

void primo_state::machine_start()
{
	m_out_lcd.resolve();

	// register for savestates
	save_item(NAME(m_power));
	save_item(NAME(m_lcd_strobe));
	save_item(NAME(m_inp_mux));
	save_item(NAME(m_select));
	save_item(NAME(m_led_data));
}



/*******************************************************************************
    I/O
*******************************************************************************/

// power

void primo_state::standby(int state)
{
	// clear display
	if (state)
	{
		m_lcd_pwm->clear();
		m_led_pwm->clear();
	}
}

INPUT_CHANGED_MEMBER(primo_state::snova_power_off)
{
	// nsnova NMI at power-off, which will trigger standby mode
	if (newval && !m_maincpu->standby())
		m_maincpu->pulse_input_line(INPUT_LINE_NMI, attotime::zero);
}


// misc

void primo_state::lcd_pwm_w(offs_t offset, u8 data)
{
	m_out_lcd[offset & 0x3f][offset >> 6] = data;
}

void primo_state::update_leds()
{
	m_led_pwm->matrix(m_select >> 4 & 3, m_led_data);
}


// MCU ports

u8 primo_state::p2_r()
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

	// P23 (nprimo, supremo): power switch
	if (!m_power)
		data |= 8;

	// P23 (nsnova): serial rx
	if (m_rs232)
		data |= m_rs232->rxd_r() << 3;

	return data ^ 1;
}

void primo_state::p2_w(u8 data)
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

	// P24 (nsnova): serial tx (TTL)
	if (m_rs232)
		m_rs232->write_txd(BIT(data, 4));

	// P25-P27: 4051 S0-S2
	// 4051 Y0-Y7: multiplexed inputs
	m_inp_mux = data >> 5 & 7;
}

void primo_state::p5_w(u8 data)
{
	// P50-P53: 4066 control to LCD
	// P54,P55: led select
	// P56,P57: keypad mux, lcd data
	m_select = data ^ 0xf0;
	update_leds();
}

void primo_state::p6_w(u8 data)
{
	// P60-P67: led data, lcd data, chessboard mux
	m_led_data = ~data;
	update_leds();
}



/*******************************************************************************
    Address Maps
*******************************************************************************/

void primo_state::primo_map(address_map &map)
{
	map(0x4000, 0x47ff).mirror(0x3800).ram().share("nvram");
}

void primo_state::supremo_map(address_map &map)
{
	map(0x4000, 0x47ff).mirror(0x1800).ram().share("nvram");
	map(0x8000, 0xffff).rom();
}

void primo_state::snova_map(address_map &map)
{
	map(0x4000, 0x5fff).ram().share("nvram");
	map(0x8000, 0xffff).rom();
}



/*******************************************************************************
    Input Ports
*******************************************************************************/

static INPUT_PORTS_START( primo )
	PORT_START("IN.0")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_1) PORT_NAME("Trace Back / Next Best")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_2) PORT_NAME("Trace Forward / Auto Play")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_3) PORT_NAME("Set Level / Pawn")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_4) PORT_NAME("Info / Knight")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_5) PORT_NAME("Easy / Bishop")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_6) PORT_NAME("Solve Mate / Rook")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_7) PORT_NAME("Sound / Queen")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_8) PORT_NAME("Referee / King")

	PORT_START("IN.1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_Q) PORT_NAME("Go")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_W) PORT_NAME("Restore")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_E) PORT_NAME("Hint")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_R) PORT_NAME("Clear / Clear Board")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_T) PORT_NAME("Color")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_Y) PORT_NAME("Verify / Set Up")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_U) PORT_NAME("Random")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_I) PORT_CODE(KEYCODE_N) PORT_NAME("New Game")

	PORT_START("POWER") // needs to be triggered for nvram to work
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_OTHER) PORT_CODE(KEYCODE_F1) PORT_CHANGED_MEMBER(DEVICE_SELF, primo_state, primo_power_off, 0) PORT_NAME("Power Off")
INPUT_PORTS_END

static INPUT_PORTS_START( supremo )
	PORT_INCLUDE( primo )

	PORT_MODIFY("IN.0")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_5) PORT_NAME("Easy / Print Moves / Bishop")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_6) PORT_NAME("Solve Mate / Print Eval / Rook")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_7) PORT_NAME("Sound / Print List / Queen")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_8) PORT_NAME("Referee / Print Board / King")
INPUT_PORTS_END


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
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_Q) PORT_CODE(KEYCODE_N) PORT_NAME("New Game")

	PORT_START("POWER") // needs to be triggered for nvram to work
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_OTHER) PORT_CODE(KEYCODE_F1) PORT_CHANGED_MEMBER(DEVICE_SELF, primo_state, snova_power_off, 0) PORT_NAME("Power Off")
INPUT_PORTS_END



/*******************************************************************************
    Machine Configs
*******************************************************************************/

void primo_state::primo(machine_config &config)
{
	// basic machine hardware
	HD6301Y0(config, m_maincpu, 8_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &primo_state::primo_map);
	m_maincpu->nvram_enable_backup(true);
	m_maincpu->standby_cb().set(m_maincpu, FUNC(hd6301y0_cpu_device::nvram_set_battery));
	m_maincpu->standby_cb().append(FUNC(primo_state::standby));
	m_maincpu->in_p2_cb().set(FUNC(primo_state::p2_r));
	m_maincpu->out_p2_cb().set(FUNC(primo_state::p2_w));
	m_maincpu->out_p5_cb().set(FUNC(primo_state::p5_w));
	m_maincpu->out_p6_cb().set(FUNC(primo_state::p6_w));

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	SENSORBOARD(config, m_board).set_type(sensorboard_device::BUTTONS);
	m_board->init_cb().set(m_board, FUNC(sensorboard_device::preset_chess));
	m_board->set_delay(attotime::from_msec(350));
	m_board->set_nvram_enable(true);

	// video hardware
	PWM_DISPLAY(config, m_lcd_pwm).set_size(4, 10);
	m_lcd_pwm->output_x().set(FUNC(primo_state::lcd_pwm_w));

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_SVG));
	screen.set_refresh_hz(60);
	screen.set_size(1920/5, 606/5);
	screen.set_visarea_full();

	PWM_DISPLAY(config, m_led_pwm).set_size(2, 8);
	config.set_default_layout(layout_novag_primo);

	// sound hardware
	SPEAKER(config, "speaker").front_center();
	DAC_1BIT(config, m_dac).add_route(ALL_OUTPUTS, "speaker", 0.25);
}

void primo_state::supremo(machine_config &config)
{
	primo(config);

	// basic machine hardware
	HD6303Y(config.replace(), m_maincpu, 8_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &primo_state::supremo_map);
	m_maincpu->nvram_enable_backup(true);
	m_maincpu->standby_cb().set(m_maincpu, FUNC(hd6303y_cpu_device::nvram_set_battery));
	m_maincpu->standby_cb().append(FUNC(primo_state::standby));
	m_maincpu->in_p2_cb().set(FUNC(primo_state::p2_r));
	m_maincpu->out_p2_cb().set(FUNC(primo_state::p2_w));
	m_maincpu->out_p5_cb().set(FUNC(primo_state::p5_w));
	m_maincpu->out_p6_cb().set(FUNC(primo_state::p6_w));

	config.set_default_layout(layout_novag_supremo);
}

void primo_state::snova(machine_config &config)
{
	supremo(config);

	// basic machine hardware
	m_maincpu->set_clock(16_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &primo_state::snova_map);

	config.set_default_layout(layout_novag_snova);

	// rs232 (configure after video)
	RS232_PORT(config, m_rs232, default_rs232_devices, nullptr);
}



/*******************************************************************************
    ROM Definitions
*******************************************************************************/

ROM_START( nprimo )
	ROM_REGION( 0x4000, "maincpu", 0 )
	ROM_LOAD("novag_871_31y0c34p.u1", 0x0000, 0x4000, CRC(ad692d2e) SHA1(f41ae4e02a83ba6446a6df59a6ee2c2f87a1f4d8) )

	ROM_REGION( 36256, "screen", 0 )
	ROM_LOAD("nvip.svg", 0, 36256, CRC(3373e0d5) SHA1(25bfbf0405017388c30f4529106baccb4723bc6b) )
ROM_END


ROM_START( supremo )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("sp_a10.u5", 0x8000, 0x8000, CRC(1db63786) SHA1(4f24452ed8955b31ba88f68cc95c357660930aa4) )

	ROM_REGION( 36256, "screen", 0 )
	ROM_LOAD("nvip.svg", 0, 36256, CRC(3373e0d5) SHA1(25bfbf0405017388c30f4529106baccb4723bc6b) )
ROM_END


ROM_START( nsnova ) // ID = N1.05, serial 326xx/340xx
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("n_530.u5", 0x8000, 0x8000, CRC(727a0ada) SHA1(129c1edc5c1d2e12ce97ebef81c6d5555464a11d) )

	ROM_REGION( 36256, "screen", 0 )
	ROM_LOAD("nvip.svg", 0, 36256, CRC(3373e0d5) SHA1(25bfbf0405017388c30f4529106baccb4723bc6b) )
ROM_END

ROM_START( nsnovaa ) // ID = N1.05, serial 310xx
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("n_319.u5", 0x8000, 0x8000, CRC(7ad4cbde) SHA1(cc92a162d4a63466f2333708a8e07269646188ea) ) // 1 byte different, does not look like bitrot

	ROM_REGION( 36256, "screen", 0 )
	ROM_LOAD("nvip.svg", 0, 36256, CRC(3373e0d5) SHA1(25bfbf0405017388c30f4529106baccb4723bc6b) )
ROM_END

} // anonymous namespace



/*******************************************************************************
    Drivers
*******************************************************************************/

//    YEAR  NAME     PARENT  COMPAT  MACHINE  INPUT    CLASS        INIT        COMPANY, FULLNAME, FLAGS
SYST( 1987, nprimo,  0,      0,      primo,   primo,   primo_state, empty_init, "Novag Industries", "Primo (Novag)", MACHINE_SUPPORTS_SAVE )

SYST( 1988, supremo, 0,      0,      supremo, supremo, primo_state, empty_init, "Novag Industries", "Supremo", MACHINE_SUPPORTS_SAVE )

SYST( 1990, nsnova,  0,      0,      snova,   snova,   primo_state, empty_init, "Novag Industries", "Super Nova (Novag, v1.05 set 1)", MACHINE_SUPPORTS_SAVE )
SYST( 1990, nsnovaa, nsnova, 0,      snova,   snova,   primo_state, empty_init, "Novag Industries", "Super Nova (Novag, v1.05 set 2)", MACHINE_SUPPORTS_SAVE )
