// license:BSD-3-Clause
// copyright-holders:hap
// thanks-to:Sean Riddle
/*******************************************************************************

SciSys Chess Companion II family
CXG Enterprise "S" family

The chess engine is LogiChess (ported from Z80 to 6801), by Kaare Danielsen,
after founding the company LogiSoft ApS.

NOTE: It triggers an NMI when the power switch is changed from ON to MEMORY.
If this is not done, NVRAM won't save properly.

TODO:
- if/when MAME supports an exit callback, hook up power-off switch to that

================================================================================

SciSys Chess Companion II family hardware notes:

Chess Companion II:
- PCB label: YO1B-01 REV.B
- Hitachi HD6301V1P (0609V171) @ ~4MHz (LC oscillator)
- chessboard buttons, 16+5 leds, piezo

Explorer Chess:
- slightly different UI (12 buttons instead of 14, but same functionality)
- portable, peg board instead of button board
- rest is same as compan2

Concord / Concord II:
- PCB label: SCISYS ST3 REV.E
- MCU clock frequency is around twice higher than Companion II (measured ~7.32MHz
  on a Concord model 251, again no XTAL)
- rest is same as compan2, it just has the buttons/status leds at the bottom
  instead of at the right

Explorer Chess and Chess Companion II / Concord have the same MCU ROM, pin P23
is either VCC or GND to distinguish between the two.

0609V171 MCU is used in:
- SciSys Chess Companion II (2 revisions)
- SciSys Olympiade (French)
- SciSys Explorer Chess
- SciSys Concord
- SciSys Concord II
- SciSys Electronic Chess Mark 8
- Tandy 1650 Portable Sensory Chess (Tandy brand Explorer Chess)
- Tandy (Radio Shack) 1650 (Fast Response Time) Computerized Chess (Tandy brand Concord)

The Tandy clones run at a lower clock frequency, 3MHz and 6MHz respectively.

================================================================================

CXG Enterprise "S" / Star Chess is on very similar hardware, so it's emulated
in this driver too.

Hardware notes:
- Hitachi HD6301V1P (HD6301V1C42P), 7.15909MHz XTAL
- port 2 I/O is changed a bit, rest is same as compan2

HD6301V1C42P MCU is used in:
- CXG Enterprise "S" (model 208, black/brown/blue)
- CXG Star Chess (model 209, black/gray)
- CXG Computachess III (model 008)
- CXG Super Computachess (model 009)
- CXG Crown (model 228)
- CXG Sphinx Galaxy 2 (model 628, suspected)
- Fidelity Genesis (Fidelity brand Computachess III)
- Mephisto Merlin 4K (H+G brand Computachess III)
- Multitech Enterprise (Multitech brand Super Computachess)

*******************************************************************************/

#include "emu.h"

#include "cpu/m6800/m6801.h"
#include "machine/sensorboard.h"
#include "sound/dac.h"
#include "video/pwm.h"

#include "speaker.h"

// internal artwork
#include "cxg_enterprise.lh"
#include "saitek_companion2.lh"
#include "saitek_expchess.lh"


namespace {

class compan2_state : public driver_device
{
public:
	compan2_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_board(*this, "board"),
		m_display(*this, "display"),
		m_inputs(*this, "IN.%u", 0)
	{ }

	void enterp(machine_config &config);
	void expchess(machine_config &config);
	void compan2(machine_config &config);

	DECLARE_INPUT_CHANGED_MEMBER(power_off);
	DECLARE_INPUT_CHANGED_MEMBER(change_cpu_freq);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	// devices/pointers
	required_device<hd6301v1_cpu_device> m_maincpu;
	required_device<sensorboard_device> m_board;
	required_device<pwm_display_device> m_display;
	required_ioport_array<3> m_inputs;

	emu_timer *m_standbytimer;
	emu_timer *m_nmitimer;
	bool m_power = false;
	u8 m_inp_mux = 0;

	// I/O handlers
	u8 input1_r();
	u8 input2_r();
	u8 input2c_r();
	void mux_w(u8 data);
	u8 power_r();
	void led_w(u8 data);

	TIMER_CALLBACK_MEMBER(set_pin);
};

void compan2_state::machine_start()
{
	m_nmitimer = timer_alloc(FUNC(compan2_state::set_pin), this);
	m_standbytimer = timer_alloc(FUNC(compan2_state::set_pin), this);

	// register for savestates
	save_item(NAME(m_power));
	save_item(NAME(m_inp_mux));
}

INPUT_CHANGED_MEMBER(compan2_state::change_cpu_freq)
{
	// Concord MCU speed is around twice higher
	m_maincpu->set_unscaled_clock((newval & 1) ? 7'200'000 : 4'000'000);
}



/*******************************************************************************
    Power
*******************************************************************************/

void compan2_state::machine_reset()
{
	m_power = true;

	m_maincpu->set_input_line(INPUT_LINE_NMI, CLEAR_LINE);
	m_maincpu->set_input_line(M6801_STBY_LINE, CLEAR_LINE);
}

TIMER_CALLBACK_MEMBER(compan2_state::set_pin)
{
	m_maincpu->set_input_line(param, ASSERT_LINE);
}

INPUT_CHANGED_MEMBER(compan2_state::power_off)
{
	if (newval && m_power)
	{
		m_power = false;

		// when power switch is set to MEMORY, it triggers an NMI after a short delay
		attotime delay = attotime::from_msec(50);
		m_nmitimer->adjust(delay, INPUT_LINE_NMI);

		// afterwards, MCU STBY pin is asserted after a short delay
		delay += attotime::from_msec(10);
		m_standbytimer->adjust(delay, M6801_STBY_LINE);
	}
}



/*******************************************************************************
    I/O
*******************************************************************************/

// common

u8 compan2_state::input1_r()
{
	u8 data = 0;

	// P10-P17: read chessboard
	for (int i = 0; i < 8; i++)
		if (BIT(m_inp_mux, i))
			data |= m_board->read_file(i ^ 7);

	return ~data;
}

u8 compan2_state::input2_r()
{
	u8 data = 0;

	// P21,P22: read buttons
	for (int i = 0; i < 2; i++)
		if (m_inp_mux & m_inputs[i]->read())
			data |= 2 << i;

	// P23: button configuration
	data |= m_inputs[2]->read() << 3 & 8;

	return ~data;
}

void compan2_state::mux_w(u8 data)
{
	// P30-P37: input mux, led data
	m_inp_mux = ~data;
	m_display->write_mx(m_inp_mux);
}

u8 compan2_state::power_r()
{
	// P40: power switch state
	return m_power ? 0 : 1;
}

void compan2_state::led_w(u8 data)
{
	// P41-P45: direct leds
	// P46,P47: board leds
	m_display->write_my(~data >> 1 & 0x7f);
}


// enterp-specific

u8 compan2_state::input2c_r()
{
	// P20,P21: read buttons
	u8 data = input2_r() >> 1 & 3;

	// P24: power switch state
	return data | (power_r() << 4) | 0xec;
}



/*******************************************************************************
    Input Ports
*******************************************************************************/

static INPUT_PORTS_START( enterp )
	PORT_START("IN.0")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_H) PORT_NAME("Hint")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_E) PORT_NAME("Enter Position")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_1) PORT_CODE(KEYCODE_1_PAD) PORT_NAME("King")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_3) PORT_CODE(KEYCODE_3_PAD) PORT_NAME("Rook")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_4) PORT_CODE(KEYCODE_4_PAD) PORT_NAME("Bishop")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_5) PORT_CODE(KEYCODE_5_PAD) PORT_NAME("Knight")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_6) PORT_CODE(KEYCODE_6_PAD) PORT_NAME("Pawn")

	PORT_START("IN.1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_O) PORT_NAME("Move")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_M) PORT_NAME("Multi Move")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_2) PORT_CODE(KEYCODE_2_PAD) PORT_NAME("Queen")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_L) PORT_NAME("Level")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_S) PORT_CODE(KEYCODE_C) PORT_NAME("Sound/Color")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_T) PORT_NAME("Take Back")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_N) PORT_NAME("New Game")

	PORT_START("IN.2")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("POWER") // needs to be triggered for nvram to work
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_POWER_OFF) PORT_CHANGED_MEMBER(DEVICE_SELF, compan2_state, power_off, 0)
INPUT_PORTS_END

static INPUT_PORTS_START( expchess )
	PORT_INCLUDE( enterp )

	PORT_MODIFY("IN.0")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_W) PORT_NAME("White/Black")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_N) PORT_NAME("New Game / Clear Board")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_F) PORT_NAME("2nd F")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_1) PORT_CODE(KEYCODE_1_PAD) PORT_NAME("King")
	PORT_BIT(0x63, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_MODIFY("IN.1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_3) PORT_CODE(KEYCODE_3_PAD) PORT_NAME("Rook")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_4) PORT_CODE(KEYCODE_4_PAD) PORT_NAME("Bishop")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_T) PORT_NAME("Take Back")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_L) PORT_NAME("Level / Sound")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_P) PORT_NAME("Play / PVP")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_5) PORT_CODE(KEYCODE_5_PAD) PORT_NAME("Knight")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_6) PORT_CODE(KEYCODE_6_PAD) PORT_NAME("Pawn")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_2) PORT_CODE(KEYCODE_2_PAD) PORT_NAME("Queen")

	PORT_MODIFY("IN.2")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_CUSTOM) // button config
INPUT_PORTS_END

static INPUT_PORTS_START( compan2 )
	PORT_INCLUDE( enterp )

	PORT_MODIFY("IN.0")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_S) PORT_NAME("Sound")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_C) PORT_NAME("Color")

	PORT_MODIFY("IN.1")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_L) PORT_NAME("Level")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_P) PORT_NAME("Play")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_E) PORT_NAME("Enter Position")

	PORT_MODIFY("IN.2")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_CUSTOM) // button config

	PORT_START("CPU")
	PORT_CONFNAME( 0x01, 0x00, "CPU Frequency" ) PORT_CHANGED_MEMBER(DEVICE_SELF, compan2_state, change_cpu_freq, 0) // factory set
	PORT_CONFSETTING(    0x00, "4MHz (original)" )
	PORT_CONFSETTING(    0x01, "7.2MHz (Concord)" )
INPUT_PORTS_END



/*******************************************************************************
    Machine Configs
*******************************************************************************/

void compan2_state::enterp(machine_config &config)
{
	// basic machine hardware
	HD6301V1(config, m_maincpu, 7.15909_MHz_XTAL);
	m_maincpu->nvram_enable_backup(true);
	m_maincpu->standby_cb().set(m_maincpu, FUNC(hd6301v1_cpu_device::nvram_set_battery));
	m_maincpu->standby_cb().append([this](int state) { if (state) m_display->clear(); });
	m_maincpu->in_p1_cb().set(FUNC(compan2_state::input1_r));
	m_maincpu->in_p2_cb().set(FUNC(compan2_state::input2c_r));
	m_maincpu->out_p2_cb().set("dac", FUNC(dac_1bit_device::write)).bit(2);
	m_maincpu->out_p3_cb().set(FUNC(compan2_state::mux_w));
	m_maincpu->out_p4_cb().set(FUNC(compan2_state::led_w));

	SENSORBOARD(config, m_board).set_type(sensorboard_device::BUTTONS);
	m_board->init_cb().set(m_board, FUNC(sensorboard_device::preset_chess));
	m_board->set_delay(attotime::from_msec(150));
	m_board->set_nvram_enable(true);

	// video hardware
	PWM_DISPLAY(config, m_display).set_size(5+2, 8);
	config.set_default_layout(layout_cxg_enterprise);

	// sound hardware
	SPEAKER(config, "speaker").front_center();
	DAC_1BIT(config, "dac").add_route(ALL_OUTPUTS, "speaker", 0.25);
}

void compan2_state::expchess(machine_config &config)
{
	enterp(config);

	// basic machine hardware
	m_maincpu->set_clock(4'000'000); // approximation, no XTAL
	m_maincpu->in_p2_cb().set(FUNC(compan2_state::input2_r));
	m_maincpu->out_p2_cb().set("dac", FUNC(dac_1bit_device::write)).bit(0);
	m_maincpu->in_p4_cb().set(FUNC(compan2_state::power_r));

	config.set_default_layout(layout_saitek_expchess);
}

void compan2_state::compan2(machine_config &config)
{
	expchess(config);
	config.set_default_layout(layout_saitek_companion2);
}



/*******************************************************************************
    ROM Definitions
*******************************************************************************/

ROM_START( compan2 )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD("1983_te-1_scisys-w_0609v171.u1", 0x0000, 0x1000, CRC(a26632fd) SHA1(fb83dc2476500acaabd949d749e58adca01012ea) )
ROM_END

ROM_START( expchess )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD("1983_te-1_scisys-w_0609v171.u1", 0x0000, 0x1000, CRC(a26632fd) SHA1(fb83dc2476500acaabd949d749e58adca01012ea) )
ROM_END

ROM_START( enterp )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD("1984nc208_newcrest_hd6301v1c42p", 0x0000, 0x1000, CRC(b9cc7da7) SHA1(ca07ba072cc101aabeb0853f518053f48cc73a4d) )
ROM_END

} // anonymous namespace



/*******************************************************************************
    Drivers
*******************************************************************************/

//    YEAR  NAME      PARENT   COMPAT  MACHINE   INPUT     CLASS          INIT        COMPANY, FULLNAME, FLAGS
SYST( 1983, compan2,  0,       0,      compan2,  compan2,  compan2_state, empty_init, "SciSys / LogiSoft", "Chess Companion II", MACHINE_SUPPORTS_SAVE )
SYST( 1983, expchess, compan2, 0,      expchess, expchess, compan2_state, empty_init, "SciSys / LogiSoft", "Explorer Chess", MACHINE_SUPPORTS_SAVE )

SYST( 1984, enterp,   0,       0,      enterp,   enterp,   compan2_state, empty_init, "CXG Systems / Newcrest Technology / LogiSoft", "Enterprise \"S\"", MACHINE_SUPPORTS_SAVE )
