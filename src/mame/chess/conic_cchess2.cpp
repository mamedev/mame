// license:BSD-3-Clause
// copyright-holders:hap
// thanks-to:bataais
/*******************************************************************************

Conic Computer Chess (model 07012), also sold under the same name by
Hanimex (model HMG 1200) and Westrak (model CC 1). It's also known as
Tracer Chess, this title is from an advertisement flyer, the actual chess
computer was simply named Computer Chess.

The chessboard sensors aren't buttons or magnets, but 64 jacks. There is
a metal plug under each chesspiece. From MAME's perspective, it works the
same as a magnet sensorboard.

After the player's move, the user needs to press Enter. This does not apply
to the computer's move.

Hardware notes:
- Synertek 6504 @ ~1MHz
- 2*Motorola MC6821P PIA
- 4KB ROM(AMI), 1KB RAM(2*Synertek 1024x4)
- beeper, 8*8 leds, plug board

BTANB:
- Learn button still works when in modify-mode

*******************************************************************************/

#include "emu.h"

#include "cpu/m6502/m6504.h"
#include "machine/6821pia.h"
#include "machine/sensorboard.h"
#include "sound/dac.h"
#include "video/pwm.h"

#include "speaker.h"

// internal artwork
#include "conic_cchess2.lh"


namespace {

class cchess2_state : public driver_device
{
public:
	cchess2_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_pia(*this, "pia%u", 0),
		m_board(*this, "board"),
		m_display(*this, "display"),
		m_dac(*this, "dac"),
		m_inputs(*this, "IN.%u", 0)
	{ }

	// machine configs
	void cncchess2(machine_config &config);

	// assume that RESET button is tied to 6504 reset pin
	DECLARE_INPUT_CHANGED_MEMBER(reset_button) { m_maincpu->set_input_line(INPUT_LINE_RESET, newval ? ASSERT_LINE : CLEAR_LINE); }

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	// devices/pointers
	required_device<cpu_device> m_maincpu;
	required_device_array<pia6821_device, 2> m_pia;
	required_device<sensorboard_device> m_board;
	required_device<pwm_display_device> m_display;
	required_device<dac_1bit_device> m_dac;
	required_ioport_array<8> m_inputs;

	u8 m_inp_mux = 0;
	u8 m_led_data = 0;
	u8 m_dac_on = 0;

	// address maps
	void main_map(address_map &map) ATTR_COLD;

	// I/O handlers
	void update_display();
	void update_dac();
	void pia0_pa_w(u8 data);
	void pia0_pb_w(u8 data);
	u8 pia1_pa_r();
	u8 pia1_pb_r();
	void pia1_pb_w(u8 data);
};

void cchess2_state::machine_start()
{
	save_item(NAME(m_inp_mux));
	save_item(NAME(m_led_data));
	save_item(NAME(m_dac_on));
}



/*******************************************************************************
    I/O
*******************************************************************************/

void cchess2_state::update_display()
{
	m_display->matrix(m_inp_mux, m_led_data);
}

void cchess2_state::update_dac()
{
	m_dac->write(m_dac_on & BIT(m_inp_mux, 1));
}

void cchess2_state::pia0_pa_w(u8 data)
{
	// d0-d7: input mux/led select
	// d1: dac data
	m_inp_mux = data;
	update_display();
	update_dac();
}

void cchess2_state::pia0_pb_w(u8 data)
{
	// d0-d7: led data
	m_led_data = data;
	update_display();
}

u8 cchess2_state::pia1_pa_r()
{
	u8 data = 0;

	// d0-d7: chessboard sensors
	for (int i = 0; i < 8; i++)
		if (BIT(m_inp_mux, i))
			data |= m_board->read_rank(i);

	return ~data;
}

u8 cchess2_state::pia1_pb_r()
{
	u8 data = 0;

	// d0-d3: multiplexed inputs
	for (int i = 0; i < 8; i++)
		if (BIT(m_inp_mux, i))
			data |= m_inputs[i]->read();

	return data;
}

void cchess2_state::pia1_pb_w(u8 data)
{
	// d7: dac on
	m_dac_on = BIT(data, 7);
	update_dac();
}



/*******************************************************************************
    Address Maps
*******************************************************************************/

void cchess2_state::main_map(address_map &map)
{
	map.global_mask(0x1fff);
	map(0x0000, 0x03ff).ram();
	map(0x0a00, 0x0a03).mirror(0x00fc).rw(m_pia[0], FUNC(pia6821_device::read), FUNC(pia6821_device::write));
	map(0x0c00, 0x0c03).mirror(0x00fc).rw(m_pia[1], FUNC(pia6821_device::read), FUNC(pia6821_device::write));
	map(0x1000, 0x1fff).rom();
}



/*******************************************************************************
    Input Ports
*******************************************************************************/

static INPUT_PORTS_START( cncchess2 )
	PORT_START("IN.0")
	PORT_BIT(0x03, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_CONFNAME( 0x0c, 0x08, "Mode" )
	PORT_CONFSETTING(    0x04, "Modify" )
	PORT_CONFSETTING(    0x08, "Play" )

	PORT_START("IN.1")
	PORT_CONFNAME( 0x03, 0x02, "Color" )
	PORT_CONFSETTING(    0x01, "Black" ) // from the bottom
	PORT_CONFSETTING(    0x02, "White" )
	PORT_BIT(0x0c, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("IN.2")
	PORT_BIT(0x0f, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("IN.3")
	PORT_BIT(0x0f, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("IN.4")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_ENTER) PORT_CODE(KEYCODE_ENTER_PAD) PORT_NAME("Enter")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_N) PORT_NAME("Learn")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_L) PORT_NAME("Level")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_6) PORT_NAME("6 / White King")

	PORT_START("IN.5")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_5) PORT_NAME("5 / White Queen")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_4) PORT_NAME("4 / White Rook")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_3) PORT_NAME("3 / White Bishop")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_2) PORT_NAME("2 / White Knight")

	PORT_START("IN.6")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_1) PORT_NAME("1 / White Pawn")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_DEL) PORT_CODE(KEYCODE_BACKSPACE) PORT_NAME("Clear")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_7) PORT_NAME("7 / Black Pawn")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_8) PORT_NAME("8 / Black Knight")

	PORT_START("IN.7")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_9) PORT_NAME("9 / Black Bishop")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_0) PORT_NAME("10 / Black Rook")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_MINUS) PORT_NAME("11 / Black Queen")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_EQUALS) PORT_NAME("12 / Black King")

	PORT_START("RESET")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_R) PORT_CHANGED_MEMBER(DEVICE_SELF, cchess2_state, reset_button, 0) PORT_NAME("Reset")
INPUT_PORTS_END



/*******************************************************************************
    Machine Configs
*******************************************************************************/

void cchess2_state::cncchess2(machine_config &config)
{
	// basic machine hardware
	M6504(config, m_maincpu, 1'000'000); // approximation, no XTAL
	m_maincpu->set_addrmap(AS_PROGRAM, &cchess2_state::main_map);

	PIA6821(config, m_pia[0]);
	m_pia[0]->writepa_handler().set(FUNC(cchess2_state::pia0_pa_w));
	m_pia[0]->writepb_handler().set(FUNC(cchess2_state::pia0_pb_w));

	PIA6821(config, m_pia[1]);
	m_pia[1]->readpa_handler().set(FUNC(cchess2_state::pia1_pa_r));
	m_pia[1]->readpb_handler().set(FUNC(cchess2_state::pia1_pb_r));
	m_pia[1]->writepb_handler().set(FUNC(cchess2_state::pia1_pb_w));

	SENSORBOARD(config, m_board).set_type(sensorboard_device::MAGNETS);
	m_board->init_cb().set(m_board, FUNC(sensorboard_device::preset_chess));
	m_board->set_delay(attotime::from_msec(150));

	// video hardware
	PWM_DISPLAY(config, m_display).set_size(8, 8);
	config.set_default_layout(layout_conic_cchess2);

	// sound hardware
	SPEAKER(config, "speaker").front_center();
	DAC_1BIT(config, m_dac).add_route(ALL_OUTPUTS, "speaker", 0.25);
}



/*******************************************************************************
    ROM Definitions
*******************************************************************************/

ROM_START( cncchess2 )
	ROM_REGION( 0x2000, "maincpu", 0 )
	ROM_LOAD("c11485.u2", 0x1000, 0x1000, CRC(b179d536) SHA1(0b1f9c247a4a3e2ccbf8d3ae5efa62b8938f572f) ) // AMI 2332
ROM_END

} // anonymous namespace



/*******************************************************************************
    Drivers
*******************************************************************************/

//    YEAR  NAME       PARENT  COMPAT  MACHINE    INPUT      CLASS          INIT        COMPANY, FULLNAME, FLAGS
SYST( 1980, cncchess2, 0,      0,      cncchess2, cncchess2, cchess2_state, empty_init, "Conic", "Computer Chess (Conic, model 7012)", MACHINE_SUPPORTS_SAVE )
