// license:BSD-3-Clause
// copyright-holders:hap
// thanks-to:Sean Riddle
/*******************************************************************************

Saitek Electronic Dames

Electronic draughts (checkers) game, international rules apply (10*10 board,
flying kings, backward captures allowed).

NOTE: Before exiting MAME, press the STOP button to turn the power off. Otherwise,
NVRAM won't save properly.

MAME sensorboard handling is similar to Fidelity Dame Sensory Challenger, see
fidelity/dames.cpp. It will give an error beep if the user removes a captured
piece from the board, but it doesn't matter.

Two versions were sold, each should have the same MCU and ROM: a tabletop model
(Electronic Dames, 8MHz or 12MHz), and a portable model (Compact Dames Computer,
8MHz). As with SciSys/Saitek chess computers, they were also licensed to Tandy.
The program engine is DIOS by Eric van Riet Paap.

According to the second hand market, the tabletop French version is much more
common than the English one. The manual and a LED label incorrectly call crowned
men queens instead of kings, perhaps due to a translation from French (dame).

Hardware notes (Compact Dames Computer):
- PCB label: DH1-PE-009 REV.1
- Hitachi HD6301Y0P MCU, 8MHz or 12MHz (LC osc, no XTAL)
- 20+8 LEDs, 5*10 buttons sensor board, piezo

*******************************************************************************/

#include "emu.h"

#include "cpu/m6800/m6801.h"
#include "machine/sensorboard.h"
#include "sound/dac.h"
#include "video/pwm.h"

#include "speaker.h"

// internal artwork
#include "saitek_edames.lh"


namespace {

class edames_state : public driver_device
{
public:
	edames_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_board(*this, "board"),
		m_display(*this, "display"),
		m_dac(*this, "dac"),
		m_inputs(*this, "IN.%u", 0)
	{ }

	void edames(machine_config &config);

	DECLARE_INPUT_CHANGED_MEMBER(go_button);
	DECLARE_INPUT_CHANGED_MEMBER(change_cpu_freq);

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	// devices/pointers
	required_device<hd6301y0_cpu_device> m_maincpu;
	required_device<sensorboard_device> m_board;
	required_device<pwm_display_device> m_display;
	required_device<dac_1bit_device> m_dac;
	required_ioport_array<4> m_inputs;

	u16 m_inp_mux = 0;
	u8 m_led_select = 0;
	u16 m_led_data[2] = { };
	bool m_enable_reset = false;

	void init_board(u8 data);

	// I/O handlers
	void update_display();

	void p1_w(u8 data);
	void p3_w(u8 data);
	void p4_w(u8 data);
	u8 p5_r();
	void p6_w(u8 data);
	void p7_w(u8 data);
};



/*******************************************************************************
    Initialization
*******************************************************************************/

void edames_state::machine_start()
{
	// register for savestates
	save_item(NAME(m_inp_mux));
	save_item(NAME(m_led_select));
	save_item(NAME(m_led_data));
	save_item(NAME(m_enable_reset));
}

void edames_state::init_board(u8 data)
{
	for (int i = 0; i < 20; i++)
	{
		m_board->write_piece(i % 5, i / 5, 1); // white
		m_board->write_piece(i % 5, i / 5 + 6, 3); // black
	}
}

INPUT_CHANGED_MEMBER(edames_state::change_cpu_freq)
{
	// 6MHz and 10MHz versions don't exist, but the software supports it
	static const u32 freq[4] = { 6'000'000, 8'000'000, 10'000'000, 12'000'000 };
	m_maincpu->set_unscaled_clock(freq[~newval & 3]);
}



/*******************************************************************************
    I/O
*******************************************************************************/

INPUT_CHANGED_MEMBER(edames_state::go_button)
{
	if (newval && m_enable_reset)
		m_maincpu->pulse_input_line(INPUT_LINE_RESET, attotime::zero);
}

void edames_state::update_display()
{
	m_display->matrix_partial(0, 2, m_led_select, m_led_data[0]);
	m_display->write_row(2, m_led_data[1]);
}

void edames_state::p1_w(u8 data)
{
	// P10-P17: board leds data part
	m_led_data[0] = (m_led_data[0] & 0x300) | (data ^ 0xff);
	update_display();
}

void edames_state::p3_w(u8 data)
{
	// P30-P36: status leds part
	m_led_data[1] = (m_led_data[1] & 0x80) | (~data & 0x7f);
	update_display();

	// P37: enable reset button
	m_enable_reset = bool(BIT(data, 7));
}

void edames_state::p4_w(u8 data)
{
	// P40-P45: input mux part
	m_inp_mux = (m_inp_mux & 0x7f) | (~data << 7);

	// P47: status leds part
	m_led_data[1] = (m_led_data[1] & 0x7f) | (~data & 0x80);
	update_display();
}

u8 edames_state::p5_r()
{
	// P50-P54: multiplexed inputs
	u8 data = 0;

	// read checkerboard
	for (int i = 0; i < 10; i++)
		if (BIT(m_inp_mux, i))
			data |= m_board->read_rank(i, true);

	// read buttons
	for (int i = 0; i < 3; i++)
		if (BIT(m_inp_mux, i + 10))
			data |= m_inputs[i]->read();

	// P55-P57: freq sel and one more button
	return (~data & 0x1f) | (m_inputs[3]->read() << 5);
}

void edames_state::p6_w(u8 data)
{
	// P60-P67: input mux part
	m_inp_mux = (m_inp_mux & ~0x7f) | (~data & 0x7f);
}

void edames_state::p7_w(u8 data)
{
	// P70,P71: board leds select
	m_led_select = ~data & 3;

	// P72-P73: board leds data part
	m_led_data[0] = (m_led_data[0] & 0xff) | (~data << 6 & 0x300);
	update_display();

	// P74: speaker out
	m_dac->write(BIT(~data, 4));
}



/*******************************************************************************
    Input Ports
*******************************************************************************/

static INPUT_PORTS_START( edames ) // see comments for French version labels
	PORT_START("IN.0")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_Y) PORT_NAME("Set Up")     // Position
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_H) PORT_NAME("Play")       // Joue
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_5) PORT_CODE(KEYCODE_N) PORT_NAME("New Game") // Nouvelle Partie
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_3) PORT_CODE(KEYCODE_L) PORT_NAME("Level")    // Niveau
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_4) PORT_NAME("Take Back")  // Recule 1 Coup

	PORT_START("IN.1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_A) PORT_NAME("Move")       // Coup
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_F) PORT_NAME("Non Auto")   // Non Auto
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_G) PORT_NAME("Sound")      // Son
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_R) PORT_NAME("Black Man")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_W) PORT_NAME("White Man")

	PORT_START("IN.2")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_S) PORT_NAME("Evaluation") // Calcul
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_D) PORT_NAME("Depth")      // Note
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_Q) PORT_NAME("White King")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_E) PORT_NAME("Black King")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_2) PORT_NAME("Stop")       // Stop

	PORT_START("IN.3")
	PORT_CONFNAME( 0x03, 0x02, "CPU Frequency" ) PORT_CHANGED_MEMBER(DEVICE_SELF, edames_state, change_cpu_freq, 0) // factory set
	PORT_CONFSETTING(    0x03, "6MHz (unofficial)" )
	PORT_CONFSETTING(    0x02, "8MHz (original version)" )
	PORT_CONFSETTING(    0x01, "10MHz (unofficial)" )
	PORT_CONFSETTING(    0x00, "12MHz (newer version)" )
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_T) PORT_NAME("Swap Side")   // Tourne Damier

	PORT_START("RESET")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_1) PORT_CHANGED_MEMBER(DEVICE_SELF, edames_state, go_button, 0) PORT_NAME("Go") // Go
INPUT_PORTS_END



/*******************************************************************************
    Machine Configs
*******************************************************************************/

void edames_state::edames(machine_config &config)
{
	// basic machine hardware
	HD6301Y0(config, m_maincpu, 8'000'000); // approximation, no XTAL
	m_maincpu->nvram_enable_backup(true);
	m_maincpu->standby_cb().set(m_maincpu, FUNC(hd6301y0_cpu_device::nvram_set_battery));
	m_maincpu->standby_cb().append([this](int state) { if (state) m_display->clear(); });
	m_maincpu->out_p1_cb().set(FUNC(edames_state::p1_w));
	m_maincpu->out_p3_cb().set(FUNC(edames_state::p3_w));
	m_maincpu->out_p4_cb().set(FUNC(edames_state::p4_w));
	m_maincpu->in_p5_cb().set(FUNC(edames_state::p5_r));
	m_maincpu->out_p6_cb().set(FUNC(edames_state::p6_w));
	m_maincpu->out_p7_cb().set(FUNC(edames_state::p7_w));

	SENSORBOARD(config, m_board).set_type(sensorboard_device::BUTTONS);
	m_board->init_cb().set(FUNC(edames_state::init_board));
	m_board->set_size(5, 10); // 2 columns per x (eg. square 1 & 6 are same x)
	m_board->set_spawnpoints(4);
	m_board->set_delay(attotime::from_msec(150));
	m_board->set_nvram_enable(true);

	// video hardware
	PWM_DISPLAY(config, m_display).set_size(2+1, 10);
	config.set_default_layout(layout_saitek_edames);

	// sound hardware
	SPEAKER(config, "speaker").front_center();
	DAC_1BIT(config, m_dac).add_route(ALL_OUTPUTS, "speaker", 0.25);
}



/*******************************************************************************
    ROM Definitions
*******************************************************************************/

ROM_START( edames )
	ROM_REGION( 0x4000, "maincpu", 0 )
	ROM_LOAD("1988_d1_saitek_6301y0h04p.u1", 0x0000, 0x4000, CRC(a23e2114) SHA1(85b7685ec612221d2ffdd2df4550ffad22acef81) )
ROM_END

} // anonymous namespace



/*******************************************************************************
    Drivers
*******************************************************************************/

//    YEAR  NAME    PARENT    COMPAT  MACHINE  INPUT   CLASS         INIT        COMPANY, FULLNAME, FLAGS
SYST( 1988, edames, 0,        0,      edames,  edames, edames_state, empty_init, "Saitek", "Electronic Dames", MACHINE_SUPPORTS_SAVE )
