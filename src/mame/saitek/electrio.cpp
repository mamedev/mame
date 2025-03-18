// license:BSD-3-Clause
// copyright-holders:hap
// thanks-to:Sean Riddle
/*******************************************************************************

SciSys Electronic Trio / Kasparov Pocket Chess family

Developed by Heuristic Software (Kaplan's company). Electronic Trio and Pocket
Chess have the same MCU/ROM, the former adds an option to select 2 other games
(Checkers and Tic-Tac-Toe). Use the New Game button to select a different game.
In MAME, make sure to change to set the applicable view under Video Options.

Note that for the Checkers games, captures are not done automatically. Manually
remove the captured pieces while holding Ctrl.

Hardware notes:

Electronic Trio:
- PCB label: SH2-PE-013 REV 3
- Hitachi HD44868 @ ~600kHz (62K resistor)
- piezo, 16 LEDs, button sensors chessboard

PCB labels for others (base hardware is the same):
- Kasparov Pocket Chess: SHC-PE-005
- Kasparov Pocket Plus: SHD-PE-005
- Pocket Checkers: CH1-PE-013 Rev.0 (also seen with Pocket Plus PCB)
- Kasparov Travel Mate II: TDII-PE-006
- Electronic Chess Mk 10: ST1-PE-002 REV 2
- Kasparov Mk 12: ST2-PE-001 REV 3

================================================================================

44868A12 MCU is used in:
- SciSys Kasparov Pocket Chess (model 114)
- SciSys Electronic Trio (model 124)
- SciSys Kasparov Travel Mate II (model 125)
- SciSys Courier V (red version of Travel Mate II, French)
- SciSys Electronic Chess Mk 10 (later sold as Kasparov Mk 10) (model 162)
- Scisys Electronic Chess (red version of Mk 10, French)

Button configuration (New Game to switch between games) is determined by pin R43.
VCC = Electronic Trio, GND = dedicated chess computer.

44868A14 MCU is used in:
- SciSys Kasparov Pocket Plus (model 115)
- SciSys Kasparov Plus (model 128)
- SciSys Kasparov Plus (Computer Plus Coach) (model 129)
- SciSys Kasparov Mk 12 (model 164)
- Tandy (Radio Shack) Pocket Chess Computer 1450 (model 60-2251), Tandy brand
  Pocket Plus

Mk 12 and Plus Coach have 15 buttons, but functionality is exactly the same.
As with Electronic Trio, button configuration is determined by R42/R43 pins.
SciSys did something similar with Chess Companion II and Explorer chess.

Saitek Kasparov Mk 12 Trainer (and maybe Pocket Plus Trainer as well) are on
different hardware, with an ST8108 MCU.

44868A16 MCU is used in:
- Saitek Pocket Checkers (model 630)
- Saitek Electronic Checkers (model 640)
- Tandy (Radio Shack) Sensory Electronic Checkers (model 60-2203), Tandy brand
  Pocket Checkers

Pocket Checkers is nearly the same as Electronic Trio, it's hardcoded to Checkers.
The ROM is only 5 bytes different. The A5 LED still lights up when holding New Game,
this is normal. It's not possible to select Chess or Tic-Tac-Toe.

*******************************************************************************/

#include "emu.h"

#include "cpu/hmcs40/hmcs40.h"
#include "machine/sensorboard.h"
#include "sound/dac.h"
#include "video/pwm.h"

#include "speaker.h"

// internal artwork
#include "saitek_electrio.lh"
#include "saitek_mk12.lh"
#include "saitek_pcheckers.lh"
#include "saitek_pchess.lh"
#include "saitek_pplus.lh"


namespace {

class electrio_state : public driver_device
{
public:
	electrio_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_board(*this, "board%u", 0),
		m_display(*this, "display"),
		m_dac(*this, "dac"),
		m_inputs(*this, "IN.%u", 0),
		m_out_piece(*this, "piece%u_%c%u", 1U, unsigned('a'), 1U),
		m_out_pui(*this, "piece%u_ui%u", 1U, 0U),
		m_out_count(*this, "count%u_ui%u", 1U, 0U)
	{ }

	void electrio(machine_config &config);
	void pchess(machine_config &config);
	void pcheckers(machine_config &config);
	void mk12(machine_config &config);
	void pplus(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	// devices/pointers
	required_device<hmcs40_cpu_device> m_maincpu;
	optional_device_array<sensorboard_device, 3> m_board;
	required_device<pwm_display_device> m_display;
	required_device<dac_1bit_device> m_dac;
	required_ioport_array<3> m_inputs;
	output_finder<2, 0x10, 0x10> m_out_piece;
	output_finder<2, 0x20+1> m_out_pui;
	output_finder<2, 2> m_out_count;

	u8 m_inp_mux = 0;

	template<int N> void init_checkers(u8 data);
	template<int N> void output_board(offs_t offset, u16 data);

	// I/O handlers
	template<int N> void input_w(u8 data);
	template<int N> u8 input1_r();
	void leds_w(u16 data);
	u8 input2_r();
};

void electrio_state::machine_start()
{
	// resolve outputs (electrio)
	m_out_piece.resolve();
	m_out_pui.resolve();
	m_out_count.resolve();

	save_item(NAME(m_inp_mux));
}



/*******************************************************************************
    Sensorboard
*******************************************************************************/

template<int N>
void electrio_state::init_checkers(u8 data)
{
	for (int i = 0; i < 12; i++)
	{
		m_board[N]->write_piece((i % 4) * 2 + ((i / 4) & 1), i / 4, 1); // white
		m_board[N]->write_piece((i % 4) * 2 + (~(i / 4) & 1), i / 4 + 5, 3); // black
	}
}

template<int N>
void electrio_state::output_board(offs_t offset, u16 data)
{
	// forward outputs for electrio extra boards
	const u8 sel = (offset >> 8) % 3;
	offset &= 0xff;

	switch (sel)
	{
		case 0:
		{
			const u8 x = offset & 0xf;
			const u8 y = offset >> 4 & 0xf;
			m_out_piece[N][x][y] = data;
			break;
		}

		case 1:
			m_out_pui[N][offset] = data;
			break;

		case 2:
			m_out_count[N][offset] = data;
			break;
	}
}



/*******************************************************************************
    I/O
*******************************************************************************/

template<int N>
void electrio_state::input_w(u8 data)
{
	// R0x,R1x: input mux
	const u8 shift = N * 4;
	m_inp_mux = (m_inp_mux & ~(0xf << shift)) | ((data ^ 0xf) << shift);
}

template<int N>
u8 electrio_state::input1_r()
{
	// R2x,R3x: read chessboard
	u8 data = 0;

	// more than one emulated board for electrio
	for (auto & board : m_board)
	{
		for (int i = 0; i < 8; i++)
			if (board && BIT(m_inp_mux, i))
				data |= board->read_file(i, true);
	}

	return ~data >> (N * 4) & 0xf;
}

void electrio_state::leds_w(u16 data)
{
	// D0-D15: LEDs (direct)
	m_display->write_row(0, ~data);
}

u8 electrio_state::input2_r()
{
	u8 data = 0;

	// R40,R41: read buttons
	for (int i = 0; i < 2; i++)
		if (m_inp_mux & m_inputs[i]->read())
			data |= 1 << i;

	// R42,R43: button configuration
	data |= m_inputs[2]->read() << 2 & 0xc;
	return data ^ 3;
}



/*******************************************************************************
    Input Ports
*******************************************************************************/

static INPUT_PORTS_START( electrio )
	PORT_START("IN.0")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_N) PORT_NAME("New Game")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_L) PORT_NAME("Level")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_P) PORT_NAME("Play")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_T) PORT_NAME("Take Back")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_M) PORT_NAME("Multi-Move")
	PORT_BIT(0xe0, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("IN.1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_6) PORT_CODE(KEYCODE_6_PAD) PORT_NAME("Pawn")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_5) PORT_CODE(KEYCODE_5_PAD) PORT_NAME("Knight")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_4) PORT_CODE(KEYCODE_4_PAD) PORT_NAME("Bishop")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_3) PORT_CODE(KEYCODE_3_PAD) PORT_NAME("Rook")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_2) PORT_CODE(KEYCODE_2_PAD) PORT_NAME("Queen")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_1) PORT_CODE(KEYCODE_1_PAD) PORT_NAME("King")
	PORT_BIT(0xc0, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("IN.2") // button config
	PORT_BIT(0x03, 0x02, IPT_CUSTOM)
INPUT_PORTS_END

static INPUT_PORTS_START( pchess )
	PORT_INCLUDE( electrio )

	PORT_MODIFY("IN.0")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_A) PORT_NAME("Non Auto")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_S) PORT_NAME("Sound")

	PORT_MODIFY("IN.1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_1) PORT_CODE(KEYCODE_1_PAD) PORT_NAME("Pawn")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_2) PORT_CODE(KEYCODE_2_PAD) PORT_NAME("Knight")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_3) PORT_CODE(KEYCODE_3_PAD) PORT_NAME("Bishop")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_4) PORT_CODE(KEYCODE_4_PAD) PORT_NAME("Rook")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_5) PORT_CODE(KEYCODE_5_PAD) PORT_NAME("Queen")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_6) PORT_CODE(KEYCODE_6_PAD) PORT_NAME("King")

	PORT_MODIFY("IN.2")
	PORT_BIT(0x03, 0x00, IPT_CUSTOM)
INPUT_PORTS_END

static INPUT_PORTS_START( pcheckers )
	PORT_INCLUDE( pchess )

	PORT_MODIFY("IN.1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_M) PORT_NAME("Man")
	PORT_BIT(0x1e, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_K) PORT_NAME("King")

	PORT_MODIFY("IN.2")
	PORT_BIT(0x03, 0x02, IPT_CUSTOM)
INPUT_PORTS_END

static INPUT_PORTS_START( mk12 )
	PORT_INCLUDE( electrio )

	PORT_MODIFY("IN.0")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_A) PORT_NAME("Non Auto")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_H) PORT_NAME("Help")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_D) PORT_NAME("Display Move")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_S) PORT_NAME("Studies")

	PORT_MODIFY("IN.1")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_E) PORT_NAME("Evaluate")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_MODIFY("IN.2")
	PORT_BIT(0x03, 0x03, IPT_CUSTOM)
INPUT_PORTS_END

static INPUT_PORTS_START( pplus )
	PORT_INCLUDE( pchess )

	PORT_MODIFY("IN.0")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_N) PORT_CODE(KEYCODE_U) PORT_NAME("New Game / Studies")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_S) PORT_CODE(KEYCODE_C) PORT_NAME("Sound / Coach Level")

	PORT_MODIFY("IN.1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_1) PORT_CODE(KEYCODE_1_PAD) PORT_CODE(KEYCODE_D) PORT_NAME("Pawn / Display Move")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_6) PORT_CODE(KEYCODE_6_PAD) PORT_CODE(KEYCODE_E) PORT_NAME("King / Evaluate")

	PORT_MODIFY("IN.2")
	PORT_BIT(0x03, 0x02, IPT_CUSTOM)
INPUT_PORTS_END



/*******************************************************************************
    Machine Configs
*******************************************************************************/

void electrio_state::pchess(machine_config &config)
{
	// basic machine hardware
	HD44868(config, m_maincpu, 600'000); // approximation
	m_maincpu->write_r<0>().set(FUNC(electrio_state::input_w<0>));
	m_maincpu->write_r<1>().set(FUNC(electrio_state::input_w<1>));
	m_maincpu->read_r<2>().set(FUNC(electrio_state::input1_r<0>));
	m_maincpu->read_r<3>().set(FUNC(electrio_state::input1_r<1>));
	m_maincpu->read_r<4>().set(FUNC(electrio_state::input2_r));
	m_maincpu->write_r<5>().set("dac", FUNC(dac_1bit_device::write)).rshift(3);
	m_maincpu->write_d().set(FUNC(electrio_state::leds_w));

	SENSORBOARD(config, m_board[0]).set_type(sensorboard_device::BUTTONS);
	m_board[0]->init_cb().set(m_board[0], FUNC(sensorboard_device::preset_chess));
	m_board[0]->set_delay(attotime::from_msec(150));

	// video hardware
	PWM_DISPLAY(config, m_display).set_size(1, 16);
	config.set_default_layout(layout_saitek_pchess);

	// sound hardware
	SPEAKER(config, "speaker").front_center();
	DAC_1BIT(config, m_dac).add_route(ALL_OUTPUTS, "speaker", 0.25);
}

void electrio_state::mk12(machine_config &config)
{
	pchess(config);
	config.set_default_layout(layout_saitek_mk12);
}

void electrio_state::pplus(machine_config &config)
{
	pchess(config);
	config.set_default_layout(layout_saitek_pplus);
}

void electrio_state::pcheckers(machine_config &config)
{
	pchess(config);
	config.set_default_layout(layout_saitek_pcheckers);

	m_board[0]->init_cb().set(FUNC(electrio_state::init_checkers<0>));
	m_board[0]->set_spawnpoints(4);
}

void electrio_state::electrio(machine_config &config)
{
	pchess(config);
	config.set_default_layout(layout_saitek_electrio);

	// add boards for checkers and tic-tac-toe
	SENSORBOARD(config, m_board[1]).set_type(sensorboard_device::BUTTONS);
	m_board[1]->init_cb().set(FUNC(electrio_state::init_checkers<1>));
	m_board[1]->output_cb().set(FUNC(electrio_state::output_board<0>));
	m_board[1]->set_spawnpoints(4);
	m_board[1]->set_delay(attotime::from_msec(150));

	SENSORBOARD(config, m_board[2]).set_type(sensorboard_device::BUTTONS);
	m_board[2]->output_cb().set(FUNC(electrio_state::output_board<1>));
	m_board[2]->set_spawnpoints(2);
	m_board[2]->set_delay(attotime::from_msec(150));
}



/*******************************************************************************
    ROM Definitions
*******************************************************************************/

ROM_START( electrio )
	ROM_REGION( 0x2000, "maincpu", 0 )
	ROM_LOAD("1985_sx2b_scisys_44868a12.u1", 0x0000, 0x2000, CRC(3d0cbb25) SHA1(719616f5140789dc9e3d970839b205b92c8e1a41) )
	ROM_IGNORE( 0x2000 ) // ignore factory test banks
ROM_END

ROM_START( pcheckers )
	ROM_REGION( 0x2000, "maincpu", 0 )
	ROM_LOAD("1988_cx1_saitek_44868a16.u1", 0x0000, 0x2000, CRC(0b9fd694) SHA1(8f3c13f65786c1d1f414665b459724a674226d06) )
	ROM_IGNORE( 0x2000 ) // ignore factory test banks
ROM_END

ROM_START( mk12 )
	ROM_REGION( 0x2000, "maincpu", 0 )
	ROM_LOAD("1986_sx1_scisys_44868a14.u1", 0x0000, 0x2000, CRC(41743fbc) SHA1(fb6f97ef9eddaf781eb4348a7fa956a874914086) )
	ROM_IGNORE( 0x2000 ) // ignore factory test banks
ROM_END

#define rom_kpchess rom_electrio
#define rom_kpplus rom_mk12

} // anonymous namespace



/*******************************************************************************
    Drivers
*******************************************************************************/

//    YEAR  NAME       PARENT    COMPAT  MACHINE    INPUT      CLASS           INIT        COMPANY, FULLNAME, FLAGS
SYST( 1985, electrio,  0,        0,      electrio,  electrio,  electrio_state, empty_init, "SciSys / Heuristic Software", "Electronic Trio", MACHINE_SUPPORTS_SAVE )
SYST( 1985, kpchess,   electrio, 0,      pchess,    pchess,    electrio_state, empty_init, "SciSys / Heuristic Software", "Kasparov Pocket Chess", MACHINE_SUPPORTS_SAVE )
SYST( 1988, pcheckers, electrio, 0,      pcheckers, pcheckers, electrio_state, empty_init, "Saitek / Heuristic Software", "Pocket Checkers", MACHINE_SUPPORTS_SAVE )

SYST( 1986, mk12,      0,        0,      mk12,      mk12,      electrio_state, empty_init, "SciSys / Heuristic Software", "Kasparov Mk 12", MACHINE_SUPPORTS_SAVE )
SYST( 1986, kpplus,    mk12,     0,      pplus,     pplus,     electrio_state, empty_init, "SciSys / Heuristic Software", "Kasparov Pocket Plus", MACHINE_SUPPORTS_SAVE )
