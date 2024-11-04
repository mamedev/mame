// license:BSD-3-Clause
// copyright-holders:Sandro Ronco, hap
/*******************************************************************************

Mephisto Mondial

The chess engine is Nona by Frans Morsch, the first time his program was included
in a chess computer.

Hardware notes:
- G65SC02-1 @ 2MHz
- 2KB RAM(TC5517AP), 16KB ROM
- expansion slot at underside
- 8*8 chessboard buttons, 24 leds, active buzzer

TODO:
- verify XTAL (or maybe RC or LC circuit), 2MHz is correct
- dump/add MM 1000 module

*******************************************************************************/

#include "emu.h"

#include "bus/generic/slot.h"
#include "bus/generic/carts.h"
#include "cpu/m6502/m65sc02.h"
#include "machine/sensorboard.h"
#include "sound/beep.h"
#include "video/pwm.h"

#include "speaker.h"

// internal artwork
#include "mephisto_mondial.lh"


namespace {

class mondial_state : public driver_device
{
public:
	mondial_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_board(*this, "board"),
		m_led_pwm(*this, "led_pwm"),
		m_beeper(*this, "beeper"),
		m_keys(*this, "KEY.%u", 0)
	{ }

	void mondial(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;
	required_device<sensorboard_device> m_board;
	required_device<pwm_display_device> m_led_pwm;
	required_device<beep_device> m_beeper;
	required_ioport_array<2> m_keys;

	u8 m_inp_mux = 0;

	void mondial_mem(address_map &map) ATTR_COLD;

	void control_w(u8 data);
	u8 irq_ack_r();
	u8 input_r(offs_t offset);
};

void mondial_state::machine_start()
{
	save_item(NAME(m_inp_mux));
}



/*******************************************************************************
    I/O
*******************************************************************************/

void mondial_state::control_w(u8 data)
{
	// d0-d3: input mux, led select
	// d4-d6: led data
	m_inp_mux = data & 0xf;
	m_led_pwm->matrix(1 << m_inp_mux, ~data >> 4 & 7);

	// d7: enable beeper
	m_beeper->set_state(BIT(data, 7));
}

u8 mondial_state::irq_ack_r()
{
	if (!machine().side_effects_disabled())
		m_maincpu->set_input_line(0, CLEAR_LINE);

	return 0;
}

u8 mondial_state::input_r(offs_t offset)
{
	u8 data = 0;

	// read chessboard sensors
	if (m_inp_mux < 8)
		data = m_board->read_rank(m_inp_mux);

	// read keypad
	else
		data = m_keys[m_inp_mux & 1]->read();

	return ~(BIT(data, offset) << 7);
}



/*******************************************************************************
    Address Maps
*******************************************************************************/

void mondial_state::mondial_mem(address_map &map)
{
	map(0x0000, 0x07ff).ram();
	map(0x1000, 0x1000).w(FUNC(mondial_state::control_w));
	map(0x1800, 0x1807).r(FUNC(mondial_state::input_r));
	map(0x2000, 0x2000).r(FUNC(mondial_state::irq_ack_r));
	map(0xc000, 0xffff).rom();
}



/*******************************************************************************
    Input Ports
*******************************************************************************/

static INPUT_PORTS_START( mondial )
	PORT_START("KEY.0")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD)    PORT_NAME("Pawn / 1")   PORT_CODE(KEYCODE_1) PORT_CODE(KEYCODE_1_PAD)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD)    PORT_NAME("Knight / 2") PORT_CODE(KEYCODE_2) PORT_CODE(KEYCODE_2_PAD)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD)    PORT_NAME("Bishop / 3") PORT_CODE(KEYCODE_3) PORT_CODE(KEYCODE_3_PAD)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD)    PORT_NAME("Rook / 4")   PORT_CODE(KEYCODE_4) PORT_CODE(KEYCODE_4_PAD)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD)    PORT_NAME("Queen / 5")  PORT_CODE(KEYCODE_5) PORT_CODE(KEYCODE_5_PAD)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD)    PORT_NAME("King / 6")   PORT_CODE(KEYCODE_6) PORT_CODE(KEYCODE_6_PAD)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD)    PORT_NAME("Black / 7")  PORT_CODE(KEYCODE_7) PORT_CODE(KEYCODE_7_PAD)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD)    PORT_NAME("White / 8")  PORT_CODE(KEYCODE_8) PORT_CODE(KEYCODE_8_PAD)

	PORT_START("KEY.1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD)    PORT_NAME("PLAY")       PORT_CODE(KEYCODE_Y)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD)    PORT_NAME("POS")        PORT_CODE(KEYCODE_P)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD)    PORT_NAME("MEM")        PORT_CODE(KEYCODE_M)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD)    PORT_NAME("INFO")       PORT_CODE(KEYCODE_I)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD)    PORT_NAME("CL")         PORT_CODE(KEYCODE_DEL) PORT_CODE(KEYCODE_BACKSPACE)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD)    PORT_NAME("LEV")        PORT_CODE(KEYCODE_L)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD)    PORT_NAME("ENT")        PORT_CODE(KEYCODE_ENTER) PORT_CODE(KEYCODE_ENTER_PAD)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD)    PORT_NAME("RES")        PORT_CODE(KEYCODE_F1)
INPUT_PORTS_END



/*******************************************************************************
    Machine Configs
*******************************************************************************/

void mondial_state::mondial(machine_config &config)
{
	// basic machine hardware
	M65SC02(config, m_maincpu, 2'000'000);
	m_maincpu->set_addrmap(AS_PROGRAM, &mondial_state::mondial_mem);

	const attotime irq_period = attotime::from_hz(2'000'000 / 0x1000);
	m_maincpu->set_periodic_int(FUNC(mondial_state::irq0_line_assert), irq_period);

	SENSORBOARD(config, m_board).set_type(sensorboard_device::BUTTONS);
	m_board->init_cb().set(m_board, FUNC(sensorboard_device::preset_chess));
	m_board->set_delay(attotime::from_msec(250));

	//GENERIC_CARTSLOT(config, "cartslot", generic_plain_slot, "mondial_cart");
	//SOFTWARE_LIST(config, "cart_list").set_original("mephisto_mondial");

	// video hardware
	PWM_DISPLAY(config, m_led_pwm).set_size(8, 3);
	config.set_default_layout(layout_mephisto_mondial);

	// sound hardware
	SPEAKER(config, "speaker").front_center();
	BEEP(config, m_beeper, 2150); // approximation
	m_beeper->add_route(ALL_OUTPUTS, "speaker", 0.25);
}



/*******************************************************************************
    ROM Definitions
*******************************************************************************/

ROM_START( mondial )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("cn61057n_hgs_10_470_00", 0xc000, 0x4000, CRC(5cde2e26) SHA1(337be35d5120ca12143ca17f8aa0642b313b3851) )
ROM_END

} // anonymous namespace



/*******************************************************************************
    Drivers
*******************************************************************************/

//    YEAR  NAME      PARENT  COMPAT  MACHINE  INPUT    CLASS          INIT        COMPANY, FULLNAME, FLAGS
SYST( 1985, mondial,  0,      0,      mondial, mondial, mondial_state, empty_init, "Hegener + Glaser", "Mephisto Mondial", MACHINE_SUPPORTS_SAVE )
