// license:BSD-3-Clause
// copyright-holders:hap
// thanks-to:Sean Riddle
/*******************************************************************************

Novag Chinese Chess (象棋, model 866)

Novag's first Xiangqi computer, mainly sold in Hong Kong. Model 8710 (棋王, Chess
King) was distributed by Yorter Electronics for the local market. It's the same
hardware as model 866. The newer model 9300 is also presumed to be the same.

Hardware notes:
- PCB label: 100054
- Hitachi HD6305Y0P @ ~8MHz (LC oscillator)
- 9*10 chessboard buttons, 19+4 leds, piezo

Strangely enough, it doesn't use the HD6305 internal timer. The only peripherals
it makes use of are the I/O ports.

BTANB:
- it uses 馬 and 車 for red horse and chariot instead of 傌 and 俥, newer Novag
  Xiangqi computers have this too, so it's a design choice?

*******************************************************************************/

#include "emu.h"

#include "cpu/m6805/hd6305.h"
#include "machine/sensorboard.h"
#include "sound/dac.h"
#include "video/pwm.h"

#include "speaker.h"

// internal artwork
#include "novag_cnchess.lh"


namespace {

class cnchess_state : public driver_device
{
public:
	cnchess_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_board(*this, "board"),
		m_display(*this, "display"),
		m_dac(*this, "dac"),
		m_inputs(*this, "IN.0")
	{ }

	void cnchess(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	// devices/pointers
	required_device<hd6305y0_device> m_maincpu;
	required_device<sensorboard_device> m_board;
	required_device<pwm_display_device> m_display;
	required_device<dac_1bit_device> m_dac;
	required_ioport m_inputs;

	u16 m_inp_mux = 0;

	void init_board(u8 data);

	// I/O handlers
	u16 input_r();
	u8 input1_r();
	u8 input2_r();
	template<int N> void input_w(u8 data);
	void control_w(u8 data);
};



/*******************************************************************************
    Initialization
*******************************************************************************/

void cnchess_state::machine_start()
{
	save_item(NAME(m_inp_mux));
}

void cnchess_state::init_board(u8 data)
{
	// 1st row
	for (int i = 0; i < 5; i++)
	{
		m_board->write_piece(0, i, 3 + i);
		m_board->write_piece(0, 8 - i, 3 + i);
	}

	// cannons
	m_board->write_piece(2, 1, 2);
	m_board->write_piece(2, 7, 2);

	// soldiers
	for (int i = 0; i < 5; i++)
		m_board->write_piece(3, i * 2, 1);

	// mirrored for black pieces
	for (int y = 0; y < 4; y++)
		for (int x = 0; x < 9; x++)
		{
			u8 piece = m_board->read_piece(y, x);
			if (piece != 0)
				m_board->write_piece(9 - y, x, piece + 7);
		}
}



/*******************************************************************************
    I/O
*******************************************************************************/

u16 cnchess_state::input_r()
{
	u16 data = 0;
	const u16 inp_mux = bitswap<10>(m_inp_mux,9,8,7,4,3,5,6,2,1,0);

	// read chessboard
	for (int i = 0; i < 10; i++)
		if (BIT(inp_mux, i))
			data |= m_board->read_file(i);

	// read buttons
	if (inp_mux & m_inputs->read())
		data |= 0x200;

	return data;
}

u8 cnchess_state::input1_r()
{
	// A0-A7: read inputs low
	return bitswap<8>(~input_r(),0,1,2,5,6,4,3,7);
}

u8 cnchess_state::input2_r()
{
	// B6,B7: read inputs high
	return bitswap<2>(~input_r(),8,9) << 6 | 0x3f;
}

template<int N>
void cnchess_state::input_w(u8 data)
{
	// Ex,F0,F1: input mux, led data
	const u8 shift = N * 8;
	const u16 mask = 0xff << shift;

	m_inp_mux = ((m_inp_mux & ~mask) | (~data << shift & mask)) & 0x3ff;
	m_display->write_mx(m_inp_mux);
}

void cnchess_state::control_w(u8 data)
{
	// G4: speaker out
	m_dac->write(BIT(~data, 4));

	// G5-G7: led select
	m_display->write_my(~data >> 5 & 7);
}



/*******************************************************************************
    Input Ports
*******************************************************************************/

static INPUT_PORTS_START( cnchess )
	PORT_START("IN.0")
	PORT_BIT(0x00001, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_G) PORT_NAME("Go")
	PORT_BIT(0x00002, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_C) PORT_NAME("Change Color")
	PORT_BIT(0x00004, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_U) PORT_CODE(KEYCODE_1) PORT_CODE(KEYCODE_1_PAD) PORT_NAME("Set Up / Pawn")
	PORT_BIT(0x00008, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_V) PORT_CODE(KEYCODE_2) PORT_CODE(KEYCODE_2_PAD) PORT_NAME("Verify / Cannon")
	PORT_BIT(0x00010, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_3) PORT_CODE(KEYCODE_3_PAD) PORT_NAME("Rook")
	PORT_BIT(0x00020, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_T) PORT_CODE(KEYCODE_4) PORT_CODE(KEYCODE_4_PAD) PORT_NAME("Take Back / Knight")
	PORT_BIT(0x00040, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_H) PORT_CODE(KEYCODE_5) PORT_CODE(KEYCODE_5_PAD) PORT_NAME("Hint / Bishop")
	PORT_BIT(0x00080, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_S) PORT_CODE(KEYCODE_6) PORT_CODE(KEYCODE_6_PAD) PORT_NAME("Sound / Minister")
	PORT_BIT(0x00100, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_L) PORT_CODE(KEYCODE_7) PORT_CODE(KEYCODE_7_PAD) PORT_NAME("Level / General")
	PORT_BIT(0x00200, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_N) PORT_NAME("New Game / Clear Board")
INPUT_PORTS_END



/*******************************************************************************
    Machine Configs
*******************************************************************************/

void cnchess_state::cnchess(machine_config &config)
{
	// basic machine hardware
	HD6305Y0(config, m_maincpu, 8'000'000); // approximation, no XTAL
	m_maincpu->read_porta().set(FUNC(cnchess_state::input1_r));
	m_maincpu->read_portb().set(FUNC(cnchess_state::input2_r));
	m_maincpu->write_porte().set(FUNC(cnchess_state::input_w<0>));
	m_maincpu->write_portf().set(FUNC(cnchess_state::input_w<1>));
	m_maincpu->write_portg().set(FUNC(cnchess_state::control_w));

	SENSORBOARD(config, m_board).set_type(sensorboard_device::BUTTONS);
	m_board->init_cb().set(FUNC(cnchess_state::init_board));
	m_board->set_size(10, 9); // rotated by 90 degrees
	m_board->set_spawnpoints(14);
	m_board->set_delay(attotime::from_msec(150));

	// video hardware
	PWM_DISPLAY(config, m_display).set_size(3, 10);
	config.set_default_layout(layout_novag_cnchess);

	// sound hardware
	SPEAKER(config, "speaker").front_center();
	DAC_1BIT(config, m_dac).add_route(ALL_OUTPUTS, "speaker", 0.5);
}



/*******************************************************************************
    ROM Definitions
*******************************************************************************/

ROM_START( cnchess )
	ROM_REGION( 0x1ec0, "maincpu", 0 )
	ROM_LOAD("novag_866_35y0b12p", 0x0000, 0x1ec0, CRC(234ef959) SHA1(9ab7310275017dd4b6b152f205d6cd65014da5a6) )
ROM_END

} // anonymous namespace



/*******************************************************************************
    Drivers
*******************************************************************************/

//    YEAR  NAME     PARENT  COMPAT  MACHINE  INPUT    CLASS          INIT        COMPANY, FULLNAME, FLAGS
SYST( 1986, cnchess, 0,      0,      cnchess, cnchess, cnchess_state, empty_init, "Novag Industries", "Chinese Chess", MACHINE_SUPPORTS_SAVE )
