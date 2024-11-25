// license:BSD-3-Clause
// copyright-holders:hap
// thanks-to:bataais
/*******************************************************************************

SciSys Chess Partner 2000, also sold by Novag with the same name.
It's probably the last SciSys / Novag collaboration.

Entering moves is not as friendly as newer sensory games. The player is expected
to press ENTER after their own move, but if they (accidentally) press it after
doing the computer's move, the computer takes your turn.

Capturing pieces is also unintuitive, having to press the destination square twice.

Hardware notes:
- 3850PK CPU at ~2.77MHz(averaged), 3853PK memory interface
- 4KB ROM, 256 bytes RAM(2*2111N)
- 4-digit 7seg panel, sensory chessboard

3850 is officially rated 2MHz, and even the CP2000 manual says it runs at 2MHz,
but tests show that it runs at a much higher speed. Three individual CP2000 were
measured, by timing move calculation, and one recording to verify beeper pitch and
display blinking rate. Real CP2000 CPU frequency is in the 2.63MHz-2.91MHz range.

The 'sequels' CP3000-CP6000 are on HMCS40 (see minichess.cpp and trsensor.cpp),
Chess Partner 1000 does not exist.

*******************************************************************************/

#include "emu.h"

#include "cpu/f8/f8.h"
#include "machine/f3853.h"
#include "machine/sensorboard.h"
#include "sound/dac.h"
#include "video/pwm.h"

#include "speaker.h"

// internal artwork
#include "saitek_cp2000.lh"


namespace {

class cp2000_state : public driver_device
{
public:
	cp2000_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_display(*this, "display"),
		m_board(*this, "board"),
		m_dac(*this, "dac"),
		m_inputs(*this, "IN.%u", 0)
	{ }

	void cp2000(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	// devices/pointers
	required_device<cpu_device> m_maincpu;
	required_device<pwm_display_device> m_display;
	required_device<sensorboard_device> m_board;
	required_device<dac_1bit_device> m_dac;
	required_ioport_array<4> m_inputs;

	u8 m_select = 0;
	u16 m_inp_mux = 0;

	// address maps
	void main_map(address_map &map) ATTR_COLD;
	void main_io(address_map &map) ATTR_COLD;

	// I/O handlers
	void control_w(u8 data);
	void digit_w(u8 data);
	u8 input_r();
};

void cp2000_state::machine_start()
{
	// register for savestates
	save_item(NAME(m_select));
	save_item(NAME(m_inp_mux));
}



/*******************************************************************************
    I/O
*******************************************************************************/

void cp2000_state::control_w(u8 data)
{
	// d0-d3: digit select
	// d4: keypad/chessboard select
	m_select = ~data;
	m_display->write_my(m_select);

	// d5: speaker out
	m_dac->write(BIT(~data, 5));
}

u8 cp2000_state::input_r()
{
	u8 data = m_inp_mux;

	// read chessboard buttons
	if (m_select & 0x10)
	{
		u8 cb = m_board->read_rank((m_inp_mux >> 1 & 7) ^ 3);
		if (m_inp_mux & 1)
			cb <<= 4;
		data |= cb & 0xf0;
	}

	// read keypad buttons
	else
	{
		// d0-d3: multiplexed inputs from d4-d7
		for (int i = 0; i < 4; i++)
			if (BIT(m_inp_mux, i + 4))
				data |= m_inputs[i]->read();

		// d4-d7: multiplexed inputs from d0-d3
		for (int i = 0; i < 4; i++)
			if (m_inp_mux & m_inputs[i]->read())
				data |= 0x10 << i;
	}

	return data;
}

void cp2000_state::digit_w(u8 data)
{
	// d0-d3: chessboard input mux (demux)
	// d0-d7: keypad input mux (direct)
	m_inp_mux = data;

	// also digit segment data
	m_display->write_mx(bitswap<8>(data,0,2,1,3,4,5,6,7));
}



/*******************************************************************************
    Address Maps
*******************************************************************************/

void cp2000_state::main_map(address_map &map)
{
	map(0x0000, 0x0fff).rom();
	map(0x1000, 0x10ff).ram();
}

void cp2000_state::main_io(address_map &map)
{
	map(0x00, 0x00).rw(FUNC(cp2000_state::input_r), FUNC(cp2000_state::digit_w));
	map(0x01, 0x01).w(FUNC(cp2000_state::control_w));
	map(0x0c, 0x0f).rw("f3853", FUNC(f3853_device::read), FUNC(f3853_device::write));
}



/*******************************************************************************
    Input Ports
*******************************************************************************/

static INPUT_PORTS_START( cp2000 )
	PORT_START("IN.0")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_4) PORT_CODE(KEYCODE_4_PAD) PORT_NAME("4 / Rook")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_8) PORT_CODE(KEYCODE_8_PAD) PORT_NAME("8 / Black")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_F) PORT_NAME("Find Position")

	PORT_START("IN.1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_3) PORT_CODE(KEYCODE_3_PAD) PORT_NAME("3 / Bishop")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_7) PORT_CODE(KEYCODE_7_PAD) PORT_NAME("7 / White")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_ENTER) PORT_CODE(KEYCODE_ENTER_PAD) PORT_NAME("Enter")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_E) PORT_NAME("Enter Position")

	PORT_START("IN.2")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_2) PORT_CODE(KEYCODE_2_PAD) PORT_NAME("2 / Knight")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_6) PORT_CODE(KEYCODE_6_PAD) PORT_NAME("6 / King")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_C) PORT_NAME("Cancel EP")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_L) PORT_CODE(KEYCODE_0) PORT_CODE(KEYCODE_0_PAD) PORT_NAME("0 / Clear Square / Level")

	PORT_START("IN.3")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_1) PORT_CODE(KEYCODE_1_PAD) PORT_NAME("1 / Pawn")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_5) PORT_CODE(KEYCODE_5_PAD) PORT_NAME("5 / Queen")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_M) PORT_NAME("Multi Move")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_N) PORT_NAME("Clear Board / New Game")
INPUT_PORTS_END



/*******************************************************************************
    Machine Configs
*******************************************************************************/

void cp2000_state::cp2000(machine_config &config)
{
	// basic machine hardware
	F8(config, m_maincpu, 2'750'000); // see driver notes
	m_maincpu->set_addrmap(AS_PROGRAM, &cp2000_state::main_map);
	m_maincpu->set_addrmap(AS_IO, &cp2000_state::main_io);
	m_maincpu->set_irq_acknowledge_callback("f3853", FUNC(f3853_device::int_acknowledge));

	f3853_device &f3853(F3853(config, "f3853", 2'750'000));
	f3853.int_req_callback().set_inputline("maincpu", F8_INPUT_LINE_INT_REQ);

	SENSORBOARD(config, m_board).set_type(sensorboard_device::BUTTONS);
	m_board->init_cb().set(m_board, FUNC(sensorboard_device::preset_chess));
	m_board->set_delay(attotime::from_msec(100));

	// video hardware
	PWM_DISPLAY(config, m_display).set_size(4, 7);
	m_display->set_segmask(0xf, 0x7f);
	config.set_default_layout(layout_saitek_cp2000);

	// sound hardware
	SPEAKER(config, "speaker").front_center();
	DAC_1BIT(config, m_dac).add_route(ALL_OUTPUTS, "speaker", 0.25);
}



/*******************************************************************************
    ROM Definitions
*******************************************************************************/

ROM_START( cp2000 )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD("c55126_y01-rom.u3", 0x0000, 0x1000, CRC(aa7b8536) SHA1(62fb2c5631541e9058e51eb6bdc6e69569baeef3) )
ROM_END

} // anonymous namespace



/*******************************************************************************
    Drivers
*******************************************************************************/

//    YEAR  NAME    PARENT  COMPAT  MACHINE  INPUT   CLASS         INIT        COMPANY, FULLNAME, FLAGS
SYST( 1980, cp2000, 0,      0,      cp2000,  cp2000, cp2000_state, empty_init, "SciSys / Novag Industries / Philidor Software", "Chess Partner 2000", MACHINE_SUPPORTS_SAVE )
