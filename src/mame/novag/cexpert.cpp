// license:BSD-3-Clause
// copyright-holders:hap
// thanks-to:Berger
/*******************************************************************************

Novag Constellation Expert (model 853)

The u3 ROM contains the following message (it's David Kittinger's company):
Copyright (c) 1985, Intelligent Heuristic Programming, Inc

Hardware notes:
- R65C02P4 @ 5MHz (10MHz XTAL)
- 2*2KB RAM(NEC D449C-3), 2*32KB ROM
- 64+8 leds, magnet sensors chessboard
- ports for optional printer and chess clock

I/O is again similar to supercon

The first version was on a modified Super Constellation PCB. 4MHz CPU speed,
and 4 TMM27128 with identical ROM contents as the newer version.

*******************************************************************************/

#include "emu.h"

#include "cpu/m6502/r65c02.h"
#include "machine/clock.h"
#include "machine/nvram.h"
#include "machine/sensorboard.h"
#include "sound/beep.h"
#include "video/pwm.h"

#include "speaker.h"

// internal artwork
#include "novag_cexpert.lh"


namespace {

class cexpert_state : public driver_device
{
public:
	cexpert_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_display(*this, "display"),
		m_board(*this, "board"),
		m_beeper(*this, "beeper"),
		m_inputs(*this, "IN.%u", 0)
	{ }

	void cexpert(machine_config &config);

	DECLARE_INPUT_CHANGED_MEMBER(change_cpu_freq);

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	// devices/pointers
	required_device<cpu_device> m_maincpu;
	required_device<pwm_display_device> m_display;
	required_device<sensorboard_device> m_board;
	required_device<beep_device> m_beeper;
	required_ioport_array<8> m_inputs;

	u8 m_inp_mux = 0;

	void main_map(address_map &map) ATTR_COLD;

	// I/O handlers
	void mux_w(u8 data);
	void control_w(u8 data);
	u8 input1_r();
	u8 input2_r();
};

void cexpert_state::machine_start()
{
	save_item(NAME(m_inp_mux));
}

INPUT_CHANGED_MEMBER(cexpert_state::change_cpu_freq)
{
	// old version had a 4MHz CPU
	m_maincpu->set_unscaled_clock((newval & 1) ? (10_MHz_XTAL/2) : (8_MHz_XTAL/2));
}



/*******************************************************************************
    I/O
*******************************************************************************/

void cexpert_state::mux_w(u8 data)
{
	// d0-d7: input mux, led data
	m_inp_mux = data;
	m_display->write_mx(data);
}

void cexpert_state::control_w(u8 data)
{
	// d0-d2: clock/printer?

	// d3: enable beeper
	m_beeper->set_state(BIT(data, 3));

	// d4-d7: 74145 to led select
	u8 sel = data >> 4 & 0xf;
	m_display->write_my(1 << sel);
}

u8 cexpert_state::input1_r()
{
	u8 data = 0;

	// d0-d7: multiplexed inputs (chessboard squares)
	for (int i = 0; i < 8; i++)
		if (BIT(m_inp_mux, i))
			data |= m_board->read_rank(i ^ 7, true);

	return ~data;
}

u8 cexpert_state::input2_r()
{
	u8 data = 0;

	// d6,d7: multiplexed inputs (side panel)
	for (int i = 0; i < 8; i++)
		if (BIT(m_inp_mux, i))
			data |= m_inputs[i]->read() << 6;

	// other: ?
	return ~data;
}



/*******************************************************************************
    Address Maps
*******************************************************************************/

void cexpert_state::main_map(address_map &map)
{
	map(0x0000, 0x0fff).ram().share("nvram");
	map(0x1000, 0x1000).nopw(); // accessory?
	map(0x1100, 0x1100).nopw(); // "
	map(0x1200, 0x1200).rw(FUNC(cexpert_state::input2_r), FUNC(cexpert_state::mux_w));
	map(0x1300, 0x1300).rw(FUNC(cexpert_state::input1_r), FUNC(cexpert_state::control_w));
	map(0x1800, 0xffff).rom();
}



/*******************************************************************************
    Input Ports
*******************************************************************************/

static INPUT_PORTS_START( cexpert )
	PORT_START("IN.0")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_I) PORT_NAME("New Game")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_8) PORT_NAME("Player/Player / Gambit Book / King")

	PORT_START("IN.1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_U) PORT_NAME("Verify / Set Up")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_7) PORT_NAME("Best Move/Random / Training Level / Queen")

	PORT_START("IN.2")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_Y) PORT_NAME("Change Color")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_6) PORT_NAME("Sound / Depth Search / Bishop")

	PORT_START("IN.3")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_T) PORT_NAME("Clear Board / Tournament Book")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_5) PORT_NAME("Solve Mate / Infinite / Knight")

	PORT_START("IN.4")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_R) PORT_NAME("Print Moves / Print Evaluations")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_4) PORT_NAME("Print Board / Interface / Rook")

	PORT_START("IN.5")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_E) PORT_NAME("Trace Forward / Auto Play")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_3) PORT_NAME("Print List / Acc. Time / Pawn")

	PORT_START("IN.6")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_W) PORT_NAME("Hint / Next Best")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_2) PORT_NAME("Set Level")

	PORT_START("IN.7")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_Q) PORT_NAME("Go")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_1) PORT_NAME("Take Back / Restore")

	PORT_START("CPU")
	PORT_CONFNAME( 0x01, 0x01, "CPU Frequency" ) PORT_CHANGED_MEMBER(DEVICE_SELF, cexpert_state, change_cpu_freq, 0) // factory set
	PORT_CONFSETTING(    0x00, "4MHz" )
	PORT_CONFSETTING(    0x01, "5MHz" )
INPUT_PORTS_END



/*******************************************************************************
    Machine Configs
*******************************************************************************/

void cexpert_state::cexpert(machine_config &config)
{
	// basic machine hardware
	R65C02(config, m_maincpu, 10_MHz_XTAL/2);
	m_maincpu->set_addrmap(AS_PROGRAM, &cexpert_state::main_map);

	auto &irq_clock(CLOCK(config, "irq_clock", 15440 / 32)); // 555 timer (measured), to 4020
	irq_clock.set_pulse_width(attotime::from_nsec(15200)); // active for 15.2us
	irq_clock.signal_handler().set_inputline(m_maincpu, M6502_IRQ_LINE);

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_1);

	SENSORBOARD(config, m_board).set_type(sensorboard_device::MAGNETS);
	m_board->init_cb().set(m_board, FUNC(sensorboard_device::preset_chess));
	m_board->set_delay(attotime::from_msec(200));
	m_board->set_nvram_enable(true);

	// video hardware
	PWM_DISPLAY(config, m_display).set_size(9, 8);
	config.set_default_layout(layout_novag_cexpert);

	// sound hardware
	SPEAKER(config, "mono").front_center();
	BEEP(config, m_beeper, 15440 / 16); // 965Hz
	m_beeper->add_route(ALL_OUTPUTS, "mono", 0.25);
}



/*******************************************************************************
    ROM Definitions
*******************************************************************************/

ROM_START( cexpert )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("novag_8533-503.u3", 0x0000, 0x8000, CRC(e384de2f) SHA1(13b56e2870e3755073e7a7cc63bae995cd468562) )
	ROM_LOAD("novag_8532-502.u2", 0x8000, 0x8000, CRC(c2c367b5) SHA1(e000871c93a5e0fc2f8b29f3d2fec0607a91559b) )
ROM_END

} // anonymous namespace



/*******************************************************************************
    Drivers
*******************************************************************************/

//    YEAR  NAME     PARENT  COMPAT  MACHINE  INPUT    CLASS          INIT        COMPANY, FULLNAME, FLAGS
SYST( 1985, cexpert, 0,      0,      cexpert, cexpert, cexpert_state, empty_init, "Novag Industries / Intelligent Heuristic Programming", "Constellation Expert", MACHINE_SUPPORTS_SAVE )
