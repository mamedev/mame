// license:BSD-3-Clause
// copyright-holders:hap
// thanks-to:bataais
/******************************************************************************

* scisys_cp2000.cpp, subdriver of machine/chessbase.cpp

SciSys Chess Partner 2000, also sold by Novag with the same name.

- 3850PK CPU at ~2MHz, 3853PK memory interface
- 4KB ROM, 256 bytes RAM(2*2111N)
- 4-digit 7seg panel, sensory chessboard

******************************************************************************/

#include "emu.h"
#include "includes/chessbase.h"

#include "cpu/f8/f8.h"
#include "machine/f3853.h"
#include "sound/dac.h"
#include "sound/volt_reg.h"
#include "speaker.h"

// internal artwork
#include "scisys_cp2000.lh" // clickable


namespace {

class cp2000_state : public chessbase_state
{
public:
	cp2000_state(const machine_config &mconfig, device_type type, const char *tag) :
		chessbase_state(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_dac(*this, "dac")
	{ }

	// machine drivers
	void cp2000(machine_config &config);

private:
	// devices/pointers
	required_device<cpu_device> m_maincpu;
	required_device<dac_bit_interface> m_dac;

	// address maps
	void main_map(address_map &map);
	void main_io(address_map &map);

	// I/O handlers
	DECLARE_WRITE8_MEMBER(control_w);
	DECLARE_WRITE8_MEMBER(digit_w);
	DECLARE_READ8_MEMBER(input_r);
};


/******************************************************************************
    Devices, I/O
******************************************************************************/

// CPU I/O ports

WRITE8_MEMBER(cp2000_state::control_w)
{
	// d0-d3: digit select
	m_led_select = data;

	// d4: keypad/chessboard select

	// d5: speaker out
	m_dac->write(BIT(~data, 5));
}

READ8_MEMBER(cp2000_state::input_r)
{
	u8 data = m_7seg_data;

	// read chessboard buttons
	u8 cb = (~m_led_select & 0x10) ? read_inputs(8) : 0;
	data |= (m_inp_mux & 0xff00) ? (cb & 0xf0) : (cb << 4);

	// read keypad buttons
	if (m_led_select & 0x10)
	{
		// d0-d3: multiplexed inputs from d4-d7
		for (int i = 0; i < 4; i++)
			if (BIT(m_7seg_data, i+4))
				data |= m_inp_matrix[i+8]->read();

		// d4-d7: multiplexed inputs from d0-d3
		for (int i = 0; i < 4; i++)
			if (m_7seg_data & m_inp_matrix[i+8]->read())
				data |= 1 << (i+4);
	}

	return data;
}

WRITE8_MEMBER(cp2000_state::digit_w)
{
	// d0-d3: chessboard input mux (demux)
	m_inp_mux = 1 << (data & 0xf);
	m_inp_mux |= m_inp_mux >> 8;

	// d0-d7: keypad input mux (direct)

	// also digit segment data, update display here
	set_display_segmask(0xf, 0x7f);
	u8 digit = bitswap<8>(data,0,2,1,3,4,5,6,7);
	display_matrix(7, 4, digit, ~m_led_select);
	m_7seg_data = data;
}



/******************************************************************************
    Address Maps
******************************************************************************/

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



/******************************************************************************
    Input Ports
******************************************************************************/

static INPUT_PORTS_START( cp2000 )
	PORT_INCLUDE( generic_cb_buttons )

	PORT_START("IN.8")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_4) PORT_CODE(KEYCODE_4_PAD) PORT_NAME("4 / Rook")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_8) PORT_CODE(KEYCODE_8_PAD) PORT_NAME("8 / Black")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_F) PORT_NAME("Find Position")

	PORT_START("IN.9")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_3) PORT_CODE(KEYCODE_3_PAD) PORT_NAME("3 / Bishop")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_7) PORT_CODE(KEYCODE_7_PAD) PORT_NAME("7 / White")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_ENTER) PORT_CODE(KEYCODE_ENTER_PAD) PORT_NAME("Enter")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_E) PORT_NAME("Enter Position")

	PORT_START("IN.10")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_2) PORT_CODE(KEYCODE_2_PAD) PORT_NAME("2 / Knight")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_6) PORT_CODE(KEYCODE_6_PAD) PORT_NAME("6 / King")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_C) PORT_NAME("Cancel EP")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_L) PORT_CODE(KEYCODE_0) PORT_CODE(KEYCODE_0_PAD) PORT_NAME("0 / Clear Square / Level")

	PORT_START("IN.11")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_1) PORT_CODE(KEYCODE_1_PAD) PORT_NAME("1 / Pawn")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_5) PORT_CODE(KEYCODE_5_PAD) PORT_NAME("5 / Queen")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_M) PORT_NAME("Multi Move")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_N) PORT_NAME("Clear Board / New Game")
INPUT_PORTS_END



/******************************************************************************
    Machine Drivers
******************************************************************************/

void cp2000_state::cp2000(machine_config &config)
{
	/* basic machine hardware */
	F8(config, m_maincpu, 2000000);
	m_maincpu->set_addrmap(AS_PROGRAM, &cp2000_state::main_map);
	m_maincpu->set_addrmap(AS_IO, &cp2000_state::main_io);
	m_maincpu->set_irq_acknowledge_callback("f3853", FUNC(f3853_device::int_acknowledge));

	f3853_device &f3853(F3853(config, "f3853", 2000000));
	f3853.int_req_callback().set_inputline("maincpu", F8_INPUT_LINE_INT_REQ);

	TIMER(config, "display_decay").configure_periodic(FUNC(cp2000_state::display_decay_tick), attotime::from_msec(1));
	config.set_default_layout(layout_scisys_cp2000);

	/* sound hardware */
	SPEAKER(config, "speaker").front_center();
	DAC_1BIT(config, m_dac).add_route(ALL_OUTPUTS, "speaker", 0.25);
	VOLTAGE_REGULATOR(config, "vref").add_route(0, "dac", 1.0, DAC_VREF_POS_INPUT);
}



/******************************************************************************
    ROM Definitions
******************************************************************************/

ROM_START( cp2000 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("c55126_y01-rom.u3", 0x0000, 0x1000, CRC(aa7b8536) SHA1(62fb2c5631541e9058e51eb6bdc6e69569baeef3) )
ROM_END

} // anonymous namespace



/******************************************************************************
    Drivers
******************************************************************************/

//    YEAR  NAME    PARENT CMP MACHINE  INPUT   CLASS         INIT        COMPANY, FULLNAME, FLAGS
CONS( 1980, cp2000, 0,      0, cp2000,  cp2000, cp2000_state, empty_init, "SciSys", "Chess Partner 2000", MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK | MACHINE_IMPERFECT_CONTROLS )
