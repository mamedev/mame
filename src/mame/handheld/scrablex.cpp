// license:BSD-3-Clause
// copyright-holders:hap
// thanks-to:Sean Riddle
/*******************************************************************************

Selchow & Righter Scrabble Lexor
It's a tabletop scrabble game/scorekeeper.

Hardware notes:
- PCB label: PB-074
- Fujitsu MB8841 MCU
- 8-digit 14-seg LEDs, 2-bit sound

There's also a version of this game on a Matsushita MN1405 MCU (see hh_mn1400.cpp),
even though it's a different MCU, the I/O is very similar.

*******************************************************************************/

#include "emu.h"

#include "cpu/mb88xx/mb88xx.h"
#include "sound/spkrdev.h"
#include "video/pwm.h"

#include "speaker.h"

#include "scrablex.lh"


namespace {

class scrablex_state : public driver_device
{
public:
	scrablex_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_display(*this, "display"),
		m_speaker(*this, "speaker"),
		m_inputs(*this, "IN.%u", 0)
	{ }

	void scrablex(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	required_device<mb8841_cpu_device> m_maincpu;
	required_device<pwm_display_device> m_display;
	required_device<speaker_sound_device> m_speaker;
	required_ioport_array<5> m_inputs;

	u8 m_inp_mux = 0;
	u16 m_r = 0;

	void write_o(u8 data);
	void write_p(u8 data);
	template<int N> u8 read_r();
	template<int N> void write_r(u8 data);
};

void scrablex_state::machine_start()
{
	// register for savestates
	save_item(NAME(m_inp_mux));
	save_item(NAME(m_r));
}



/*******************************************************************************
    I/O
*******************************************************************************/

void scrablex_state::write_o(u8 data)
{
	// O0-O7: digit select (uses index 8-15)
	m_display->write_my((1 << (data & 0xf)) >> 8);
}

void scrablex_state::write_p(u8 data)
{
	// P0-P3: input mux part
	m_inp_mux = (m_inp_mux & ~0xf) | (~data & 0xf);
}

template<int N>
u8 scrablex_state::read_r()
{
	u16 data = 0;

	// R0-R8: multiplexed inputs
	for (int i = 0; i < 5; i++)
		if (BIT(m_inp_mux, i))
			data |= m_inputs[i]->read();

	return ~data >> (N * 4) & 0xf;
}

template<int N>
void scrablex_state::write_r(u8 data)
{
	// R0-R13: digit segments
	m_r = (m_r & ~(0xf << (N * 4))) | ((~data & 0xf) << (N * 4));
	m_display->write_mx(bitswap<14>(m_r,10,8,6,12,11,7,9,13,5,4,3,2,1,0));

	// R14,R15: speaker out
	m_speaker->level_w(m_r >> 14 & 3);

	// R15: input mux part
	m_inp_mux = (m_inp_mux & 0xf) | (m_r >> 11 & 0x10);
}



/*******************************************************************************
    Input Ports
*******************************************************************************/

static INPUT_PORTS_START( scrablex )
	PORT_START("IN.0")
	PORT_BIT( 0x001, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_A) PORT_CHAR('A')
	PORT_BIT( 0x002, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_B) PORT_CHAR('B')
	PORT_BIT( 0x004, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_C) PORT_CHAR('C')
	PORT_BIT( 0x008, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_D) PORT_CHAR('D')
	PORT_BIT( 0x010, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_E) PORT_CHAR('E')
	PORT_BIT( 0x020, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F) PORT_CHAR('F')
	PORT_BIT( 0x040, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_G) PORT_CHAR('G')
	PORT_BIT( 0x080, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_H) PORT_CHAR('H')
	PORT_BIT( 0x100, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_I) PORT_CHAR('I')

	PORT_START("IN.1")
	PORT_BIT( 0x001, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_J) PORT_CHAR('J')
	PORT_BIT( 0x002, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_K) PORT_CHAR('K')
	PORT_BIT( 0x004, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_L) PORT_CHAR('L')
	PORT_BIT( 0x008, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_M) PORT_CHAR('M')
	PORT_BIT( 0x010, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_N) PORT_CHAR('N')
	PORT_BIT( 0x020, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_O) PORT_CHAR('O')
	PORT_BIT( 0x040, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_P) PORT_CHAR('P')
	PORT_BIT( 0x080, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Q) PORT_CHAR('Q')
	PORT_BIT( 0x100, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_R) PORT_CHAR('R')

	PORT_START("IN.2")
	PORT_BIT( 0x001, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_S) PORT_CHAR('S')
	PORT_BIT( 0x002, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_T) PORT_CHAR('T')
	PORT_BIT( 0x004, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_U) PORT_CHAR('U')
	PORT_BIT( 0x008, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_V) PORT_CHAR('V')
	PORT_BIT( 0x010, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_W) PORT_CHAR('W')
	PORT_BIT( 0x020, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_X) PORT_CHAR('X')
	PORT_BIT( 0x040, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Y) PORT_CHAR('Y')
	PORT_BIT( 0x080, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Z) PORT_CHAR('Z')
	PORT_BIT( 0x100, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_SPACE) PORT_CHAR('*')

	PORT_START("IN.3")
	PORT_BIT( 0x001, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x002, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_COMMA) PORT_NAME("Double")
	PORT_BIT( 0x004, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_STOP) PORT_NAME("Triple")
	PORT_BIT( 0x008, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_SLASH) PORT_NAME("Word")
	PORT_BIT( 0x010, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_EQUALS) PORT_NAME("Bonus")
	PORT_BIT( 0x020, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_MINUS) PORT_CHAR('-') PORT_NAME("Minus")
	PORT_BIT( 0x040, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_DEL) PORT_CODE(KEYCODE_BACKSPACE) PORT_CHAR(8) PORT_NAME("Clear")
	PORT_BIT( 0x080, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_ENTER) PORT_CHAR(13) PORT_NAME("Enter")
	PORT_BIT( 0x100, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.4")
	PORT_BIT( 0x001, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F1) PORT_NAME("Flash")
	PORT_BIT( 0x002, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F2) PORT_NAME("Solo")
	PORT_BIT( 0x004, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F3) PORT_NAME("Score")
	PORT_BIT( 0x008, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_1) PORT_CHAR('1')
	PORT_BIT( 0x010, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_2) PORT_CHAR('2')
	PORT_BIT( 0x020, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_3) PORT_CHAR('3')
	PORT_BIT( 0x040, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_4) PORT_CHAR('4')
	PORT_BIT( 0x080, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_5) PORT_NAME("Timer")
	PORT_BIT( 0x100, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_6) PORT_NAME("Review")
INPUT_PORTS_END



/*******************************************************************************
    Machine Configs
*******************************************************************************/

void scrablex_state::scrablex(machine_config &config)
{
	// basic machine hardware
	MB8841(config, m_maincpu, 500000); // approximation - RC osc. R=15K, C=100pF
	m_maincpu->write_o().set(FUNC(scrablex_state::write_o));
	m_maincpu->write_p().set(FUNC(scrablex_state::write_p));
	m_maincpu->read_r<0>().set(FUNC(scrablex_state::read_r<0>));
	m_maincpu->read_r<1>().set(FUNC(scrablex_state::read_r<1>));
	m_maincpu->read_r<2>().set(FUNC(scrablex_state::read_r<2>));
	m_maincpu->read_r<3>().set(FUNC(scrablex_state::read_r<3>));
	m_maincpu->write_r<0>().set(FUNC(scrablex_state::write_r<0>));
	m_maincpu->write_r<1>().set(FUNC(scrablex_state::write_r<1>));
	m_maincpu->write_r<2>().set(FUNC(scrablex_state::write_r<2>));
	m_maincpu->write_r<3>().set(FUNC(scrablex_state::write_r<3>));

	// video hardware
	PWM_DISPLAY(config, m_display).set_size(8, 14);
	m_display->set_segmask(0xff, 0x3fff);
	config.set_default_layout(layout_scrablex);

	// sound hardware
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker);
	static const double speaker_levels[4] = { 0.0, 1.0, -1.0, 0.0 };
	m_speaker->set_levels(4, speaker_levels);
	m_speaker->add_route(ALL_OUTPUTS, "mono", 0.125);
}



/*******************************************************************************
    ROM Definitions
*******************************************************************************/

ROM_START( scrablex )
	ROM_REGION( 0x0800, "maincpu", 0 )
	ROM_LOAD( "mb8841_320m", 0x0000, 0x0800, CRC(bef4ab5a) SHA1(44e385225c5e8818ed93c3654a294aa412042e34) )
ROM_END

} // anonymous namespace



/*******************************************************************************
    Drivers
*******************************************************************************/

//    YEAR  NAME      PARENT  COMPAT  MACHINE   INPUT     CLASS           INIT        COMPANY, FULLNAME, FLAGS
SYST( 1980, scrablex, 0,      0,      scrablex, scrablex, scrablex_state, empty_init, "Selchow & Righter", "Scrabble Lexor: Computer Word Game (MB8841 version)", MACHINE_SUPPORTS_SAVE )
