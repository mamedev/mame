// license:BSD-3-Clause
// copyright-holders:hap
// thanks-to:Berger
/******************************************************************************

SciSys Superstar / Turbostar
Starting from Turbostar 432, SciSys started adding the "Kasparov" prefix.

Hardware notes (Superstar 28K):
- PCB label: YO1C-PE-017 REV2
- R6502AP @ ~2MHz (no XTAL, clock from RC circuit?)
- 4KB RAM (2*HM6116P-4)
- 24KB ROM (3*M5L2764K)
- TTL, buzzer, 28 LEDs, 8*8 chessboard buttons

TODO:
- verify CPU speed
- add sstar36k, and Turbostar versions (assuming it's similar hardware)
- add KSO module, sstar28k doesn't support it?

******************************************************************************/

#include "emu.h"
#include "cpu/m6502/m6502.h"
#include "machine/sensorboard.h"
#include "sound/dac.h"
#include "sound/volt_reg.h"
#include "video/pwm.h"
#include "speaker.h"

// internal artwork
#include "saitek_sstar28k.lh" // clickable


namespace {

class star_state : public driver_device
{
public:
	star_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_board(*this, "board"),
		m_display(*this, "display"),
		m_dac(*this, "dac"),
		m_inputs(*this, "IN.%u", 0)
	{ }

	// machine drivers
	void sstar28k(machine_config &config);

protected:
	virtual void machine_start() override;

private:
	// devices/pointers
	required_device<cpu_device> m_maincpu;
	required_device<sensorboard_device> m_board;
	required_device<pwm_display_device> m_display;
	required_device<dac_bit_interface> m_dac;
	required_ioport_array<2> m_inputs;

	// address maps
	void main_map(address_map &map);

	// I/O handlers
	DECLARE_WRITE8_MEMBER(control_w);
	DECLARE_READ8_MEMBER(input_r);

	u8 m_inp_mux;
};

void star_state::machine_start()
{
	m_inp_mux = 0;
	save_item(NAME(m_inp_mux));
}



/******************************************************************************
    I/O
******************************************************************************/

WRITE8_MEMBER(star_state::control_w)
{
	// d0-d3: input mux, led select
	m_inp_mux = data & 0xf;

	// d4-d6: led data
	m_display->matrix(1 << m_inp_mux, ~data >> 4 & 7);

	// d7: speaker out
	m_dac->write(BIT(data, 7));
}

READ8_MEMBER(star_state::input_r)
{
	u8 data = 0;

	// d0-d7: multiplexed inputs
	// read chessboard sensors
	if (m_inp_mux < 8)
		data = m_board->read_file(m_inp_mux);

	// read other buttons
	else if (m_inp_mux < 10)
		data = m_inputs[m_inp_mux - 8]->read();

	return data;
}



/******************************************************************************
    Address Maps
******************************************************************************/

void star_state::main_map(address_map &map)
{
	map(0x0000, 0x07ff).mirror(0x1800).ram();
	map(0x2000, 0x27ff).mirror(0x1800).ram();
	map(0x4000, 0x4000).w(FUNC(star_state::control_w));
	map(0x6000, 0x6000).r(FUNC(star_state::input_r));
	map(0xa000, 0xffff).rom();
}



/******************************************************************************
    Input Ports
******************************************************************************/

static INPUT_PORTS_START( sstar28k )
	PORT_START("IN.0")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_2) PORT_CODE(KEYCODE_2_PAD) PORT_NAME("Queen")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_4) PORT_CODE(KEYCODE_4_PAD) PORT_NAME("Bishop")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_6) PORT_CODE(KEYCODE_6_PAD) PORT_NAME("Pawn")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_L) PORT_NAME("Level")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_O) PORT_NAME("Sound")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_N) PORT_NAME("New Game")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_D) PORT_NAME("Display Move")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_T) PORT_NAME("Take Back")

	PORT_START("IN.1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_1) PORT_CODE(KEYCODE_1_PAD) PORT_NAME("King")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_3) PORT_CODE(KEYCODE_3_PAD) PORT_NAME("Rook")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_5) PORT_CODE(KEYCODE_5_PAD) PORT_NAME("Knight")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_S) PORT_NAME("Set Up")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_U) PORT_NAME("Multi Move")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_C) PORT_NAME("Color")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_R) PORT_NAME("Replay")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_M) PORT_NAME("Move")
INPUT_PORTS_END



/******************************************************************************
    Machine Drivers
******************************************************************************/

void star_state::sstar28k(machine_config &config)
{
	/* basic machine hardware */
	M6502(config, m_maincpu, 2000000); // no XTAL
	m_maincpu->set_addrmap(AS_PROGRAM, &star_state::main_map);

	const attotime irq_period = attotime::from_hz(2000000 / 0x2000); // 4020 Q13
	m_maincpu->set_periodic_int(FUNC(star_state::nmi_line_pulse), irq_period);

	SENSORBOARD(config, m_board).set_type(sensorboard_device::BUTTONS);
	m_board->init_cb().set(m_board, FUNC(sensorboard_device::preset_chess));
	m_board->set_delay(attotime::from_msec(150));

	/* video hardware */
	PWM_DISPLAY(config, m_display).set_size(10, 3);
	config.set_default_layout(layout_saitek_sstar28k);

	/* sound hardware */
	SPEAKER(config, "speaker").front_center();
	DAC_1BIT(config, m_dac).add_route(ALL_OUTPUTS, "speaker", 0.25);
	VOLTAGE_REGULATOR(config, "vref").add_route(0, "dac", 1.0, DAC_VREF_POS_INPUT);
}



/******************************************************************************
    ROM Definitions
******************************************************************************/

ROM_START( sstar28k )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("yo1c-v25_a0.u6", 0xa000, 0x2000, CRC(3c4bef09) SHA1(50349820d131db0138bd5dc4b62f38cc3aa1d7db) ) // M5L2764K
	ROM_LOAD("yo1c-v21_c0.u4", 0xc000, 0x2000, CRC(aae43b1b) SHA1(9acef9593f19ec3a6e9a671e82196d7bd054960e) ) // "
	ROM_LOAD("yo1c-v25_e0.u5", 0xe000, 0x2000, CRC(371b81fe) SHA1(c08dd0de8eebd7c1ed2d2281bf0241a83ee0f391) ) // "
ROM_END

} // anonymous namespace



/******************************************************************************
    Drivers
******************************************************************************/

//    YEAR  NAME      PARENT CMP MACHINE   INPUT     STATE       INIT        COMPANY, FULLNAME, FLAGS
CONS( 1983, sstar28k, 0,      0, sstar28k, sstar28k, star_state, empty_init, "SciSys", "Superstar 28K", MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK )
