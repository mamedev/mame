// license:BSD-3-Clause
// copyright-holders:Sandro Ronco, hap
// thanks-to:yoyo_chessboard
/*******************************************************************************

Mephisto Mondial II

Hardware notes:
- G65SC02 @ 2MHz (unsure about rating)
- 2KB RAM, 32KB ROM
- expansion slot at underside (not used)
- 8*8 chessboard buttons, 24 leds, piezo

*******************************************************************************/

#include "emu.h"

#include "cpu/m6502/m65sc02.h"
#include "machine/sensorboard.h"
#include "sound/dac.h"
#include "video/pwm.h"

#include "speaker.h"

// internal artwork
#include "mephisto_mondial2.lh"


namespace {

class mondial2_state : public driver_device
{
public:
	mondial2_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_board(*this, "board"),
		m_led_pwm(*this, "led_pwm"),
		m_dac(*this, "dac"),
		m_keys(*this, "KEY.%u", 0)
	{ }

	void mondial2(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;
	required_device<sensorboard_device> m_board;
	required_device<pwm_display_device> m_led_pwm;
	required_device<dac_1bit_device> m_dac;
	required_ioport_array<4> m_keys;

	u8 m_keypad_mux = 0;
	u8 m_board_mux = 0;
	u8 m_led_data = 0;

	void mondial2_mem(address_map &map) ATTR_COLD;

	void update_leds();
	void control_w(u8 data);
	void board_w(u8 data);
	u8 input_r(offs_t offset);
};

void mondial2_state::machine_start()
{
	save_item(NAME(m_keypad_mux));
	save_item(NAME(m_board_mux));
	save_item(NAME(m_led_data));
}



/*******************************************************************************
    I/O
*******************************************************************************/

void mondial2_state::update_leds()
{
	m_led_pwm->matrix(m_board_mux, m_led_data);
}

void mondial2_state::control_w(u8 data)
{
	// d0-d3: keypad mux
	m_keypad_mux = ~data & 0xf;

	// d4-d7: led data
	m_led_data = data >> 4 & 7;
	update_leds();

	// d7: speaker out
	m_dac->write(BIT(data, 7));
}

void mondial2_state::board_w(u8 data)
{
	// d0-d7: chessboard mux, led select
	m_board_mux = ~data;
	update_leds();
}

u8 mondial2_state::input_r(offs_t offset)
{
	u8 data = 0;

	// read chessboard sensors
	for (int i = 0; i < 8; i++)
		if (BIT(m_board_mux, i))
			data |= BIT(m_board->read_rank(i), offset);

	// read keypad
	for (int i = 0; i < 4; i++)
		if (BIT(m_keypad_mux, i))
			data |= BIT(m_keys[i]->read(), offset & 3);

	return ~(data << 7);
}



/*******************************************************************************
    Address Maps
*******************************************************************************/

void mondial2_state::mondial2_mem(address_map &map)
{
	map(0x0000, 0x07ff).ram();
	map(0x2000, 0x2000).w(FUNC(mondial2_state::control_w));
	map(0x2800, 0x2800).w(FUNC(mondial2_state::board_w));
	map(0x3000, 0x3007).r(FUNC(mondial2_state::input_r));
	map(0x8000, 0xffff).rom();
}



/*******************************************************************************
    Input Ports
*******************************************************************************/

static INPUT_PORTS_START( mondial2 )
	PORT_START("KEY.0")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD)    PORT_NAME("Pawn / 1")   PORT_CODE(KEYCODE_1) PORT_CODE(KEYCODE_1_PAD)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD)    PORT_NAME("Knight / 2") PORT_CODE(KEYCODE_2) PORT_CODE(KEYCODE_2_PAD)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD)    PORT_NAME("Bishop / 3") PORT_CODE(KEYCODE_3) PORT_CODE(KEYCODE_3_PAD)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD)    PORT_NAME("Rook / 4")   PORT_CODE(KEYCODE_4) PORT_CODE(KEYCODE_4_PAD)

	PORT_START("KEY.1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD)    PORT_NAME("Queen / 5")  PORT_CODE(KEYCODE_5) PORT_CODE(KEYCODE_5_PAD)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD)    PORT_NAME("King / 6")   PORT_CODE(KEYCODE_6) PORT_CODE(KEYCODE_6_PAD)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD)    PORT_NAME("Black / 7")  PORT_CODE(KEYCODE_7) PORT_CODE(KEYCODE_7_PAD)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD)    PORT_NAME("White / 8")  PORT_CODE(KEYCODE_8) PORT_CODE(KEYCODE_8_PAD)

	PORT_START("KEY.2")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD)    PORT_NAME("PLAY")       PORT_CODE(KEYCODE_Y)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD)    PORT_NAME("POS")        PORT_CODE(KEYCODE_P)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD)    PORT_NAME("MEM")        PORT_CODE(KEYCODE_M)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD)    PORT_NAME("INFO")       PORT_CODE(KEYCODE_I)

	PORT_START("KEY.3")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD)    PORT_NAME("CL")         PORT_CODE(KEYCODE_DEL) PORT_CODE(KEYCODE_BACKSPACE)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD)    PORT_NAME("LEV")        PORT_CODE(KEYCODE_L)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD)    PORT_NAME("ENT")        PORT_CODE(KEYCODE_ENTER) PORT_CODE(KEYCODE_ENTER_PAD)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD)    PORT_NAME("RES")        PORT_CODE(KEYCODE_F1)
INPUT_PORTS_END



/*******************************************************************************
    Machine Configs
*******************************************************************************/

void mondial2_state::mondial2(machine_config &config)
{
	// basic machine hardware
	M65SC02(config, m_maincpu, 2_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &mondial2_state::mondial2_mem);

	const attotime nmi_period = attotime::from_hz(2_MHz_XTAL / 0x1000);
	m_maincpu->set_periodic_int(FUNC(mondial2_state::nmi_line_pulse), nmi_period);

	SENSORBOARD(config, m_board).set_type(sensorboard_device::BUTTONS);
	m_board->init_cb().set(m_board, FUNC(sensorboard_device::preset_chess));
	m_board->set_delay(attotime::from_msec(250));

	// video hardware
	PWM_DISPLAY(config, m_led_pwm).set_size(8, 3);
	config.set_default_layout(layout_mephisto_mondial2);

	// sound hardware
	SPEAKER(config, "speaker").front_center();
	DAC_1BIT(config, m_dac).add_route(ALL_OUTPUTS, "speaker", 0.25);
}



/*******************************************************************************
    ROM Definitions
*******************************************************************************/

ROM_START( mondial2 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("mondial_ii_01.08.87", 0x8000, 0x8000, CRC(e5945ce6) SHA1(e75bbf9d54087271d9d46fb1de7634eb957f8db0) )
ROM_END

} // anonymous namespace



/*******************************************************************************
    Drivers
*******************************************************************************/

//    YEAR  NAME      PARENT  COMPAT  MACHINE   INPUT     CLASS           INIT        COMPANY, FULLNAME, FLAGS
SYST( 1987, mondial2, 0,      0,      mondial2, mondial2, mondial2_state, empty_init, "Hegener + Glaser", "Mephisto Mondial II", MACHINE_SUPPORTS_SAVE )
