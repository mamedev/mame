// license:BSD-3-Clause
// copyright-holders:hap
// thanks-to:Berger
/******************************************************************************

CXG Sphinx 40 / 50

This is a modular chesscomputer, similar to Mephisto's 3-drawers one.
Chesscomputer on the right, LCD in the middle, and future expansion(?) on the left.
The only difference between 40 and 50 is the board size (40cm vs 50cm).

The chess engine is Cyrus 68K, by Mark Taylor, with advice from David Levy.
It's not related to the Z80 version of Cyrus, only by name.

This chessboard was also used on the Sphinx 40 / 50 Plus, which is another
incarnation of Frans Morsch's Dominator program.

TODO:
- unmapped read from 0x200000, looks like expansion ROM
- verify XTAL and irq source/frequency
- identify buttons
- lcd

Hardware notes:

Distribution board:
- PCB label: C 1987 CXG SYSTEMS S.A 68K 600 203
- Hitachi HD46821P (6821 PIA)
- piezo, connector to chessboard (magnet sensors, 8*8 leds)

Program/CPU module:
- PCB label: (C) 1987 NEWCREST TECHNOLOGY LTD, CXG-68K-600-001
- daughterboard for buttons, label CXG SYSTEMS, 68K-600-101
- Signetics SCN68000C8N64 @ 8MHz
- 64KB ROM (2*GI 27256-20)
- 128KB RAM (4*KM41464AP-12 64kx4 DRAM)
- 2KB battery-backed RAM (NEC D449C)

LCD module
- PCB label: CXG-68K-600-302
- Hitachi HD61603 LCD Driver
- 2 displays (4 digits each)

******************************************************************************/

#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "machine/6821pia.h"
#include "machine/bankdev.h"
#include "machine/nvram.h"
#include "machine/sensorboard.h"
#include "sound/dac.h"
#include "sound/volt_reg.h"
#include "video/pwm.h"
#include "speaker.h"

// internal artwork
#include "cxg_sphinx40.lh" // clickable


namespace {

class sphinx40_state : public driver_device
{
public:
	sphinx40_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_pia(*this, "pia"),
		m_display(*this, "display"),
		m_board(*this, "board"),
		m_dac(*this, "dac"),
		m_inputs(*this, "IN.%u", 0)
	{ }

	// machine drivers
	void sphinx40(machine_config &config);

protected:
	virtual void machine_start() override;

private:
	// devices/pointers
	required_device<cpu_device> m_maincpu;
	required_device<pia6821_device> m_pia;
	required_device<pwm_display_device> m_display;
	required_device<sensorboard_device> m_board;
	required_device<dac_bit_interface> m_dac;
	required_ioport_array<4> m_inputs;

	// address maps
	void main_map(address_map &map);
	void nvram_map(address_map &map);

	// I/O handlers
	DECLARE_READ8_MEMBER(input_r);
	DECLARE_WRITE8_MEMBER(input_w);
	DECLARE_WRITE8_MEMBER(lcd_w);

	void update_display();
	DECLARE_WRITE8_MEMBER(cb_mux_w);
	DECLARE_WRITE8_MEMBER(cb_leds_w);
	DECLARE_READ8_MEMBER(cb_r);

	u8 m_cb_mux = 0;
	u8 m_led_data = 0;
	u8 m_inp_mux = 0;
};

void sphinx40_state::machine_start()
{
	// register for savestates
	save_item(NAME(m_cb_mux));
	save_item(NAME(m_led_data));
	save_item(NAME(m_inp_mux));
}



/******************************************************************************
    I/O
******************************************************************************/

// 6821 PIA

void sphinx40_state::update_display()
{
	m_display->matrix(m_cb_mux, m_led_data);
}

WRITE8_MEMBER(sphinx40_state::cb_mux_w)
{
	// PA0-PA7: chessboard input/led mux
	m_cb_mux = ~data;
	update_display();
}

WRITE8_MEMBER(sphinx40_state::cb_leds_w)
{
	// PB0-PB7: chessboard leds
	m_led_data = ~data;
	update_display();
}


// TTL

READ8_MEMBER(sphinx40_state::cb_r)
{
	u8 data = 0;

	// d0-d7: multiplexed inputs (chessboard)
	for (int i = 0; i < 8; i++)
		if (BIT(m_cb_mux, i))
			data |= m_board->read_rank(i, true);

	return data;
}

WRITE8_MEMBER(sphinx40_state::input_w)
{
	// d0-d3: input mux (buttons)
	m_inp_mux = ~data & 0xf;
}

READ8_MEMBER(sphinx40_state::input_r)
{
	u8 data = 0;

	// d0-d7: multiplexed inputs (buttons)
	for (int i = 0; i < 4; i++)
		if (BIT(m_inp_mux, i))
			data |= m_inputs[i]->read();

	return ~data;
}

WRITE8_MEMBER(sphinx40_state::lcd_w)
{
	// d0-d3: HD61603 data
	//printf("%X ",data);
}



/******************************************************************************
    Address Maps
******************************************************************************/

void sphinx40_state::main_map(address_map &map)
{
	map(0x000000, 0x00ffff).rom();
	map(0x100000, 0x11ffff).ram();
	map(0x400000, 0x400fff).m("nvram_map", FUNC(address_map_bank_device::amap8)).umask16(0x00ff);
	map(0x70fcf1, 0x70fcf1).r(FUNC(sphinx40_state::cb_r));
	map(0x70fd71, 0x70fd71).r(FUNC(sphinx40_state::input_r));
	map(0x70fdb1, 0x70fdb1).w(FUNC(sphinx40_state::input_w));
	map(0x70fde0, 0x70fde0).w(FUNC(sphinx40_state::lcd_w));
	map(0x70fff0, 0x70fff7).rw(m_pia, FUNC(pia6821_device::read), FUNC(pia6821_device::write)).umask16(0xff00);
}

void sphinx40_state::nvram_map(address_map &map)
{
	// nvram is 8-bit (2KB)
	map(0x000, 0x7ff).ram().share("nvram");
}



/******************************************************************************
    Input Ports
******************************************************************************/

static INPUT_PORTS_START( sphinx40 )
	PORT_START("IN.0")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_1)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_2)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_3)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_4)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_5)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_6)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_7)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_8)

	PORT_START("IN.1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_Q) // rook
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_W)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_E)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_R)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_T) // bishop
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_Y)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_U)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_I)

	PORT_START("IN.2")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_A) // knight
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_S) // sound?
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_D)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_F)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_G) // pawn
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_H)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_J)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_K)

	PORT_START("IN.3")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_Z) // king
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_X)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_C) // pawn?
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_V) // new game?
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_B) // queen
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_N)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_M)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_COMMA)
INPUT_PORTS_END



/******************************************************************************
    Machine Drivers
******************************************************************************/

void sphinx40_state::sphinx40(machine_config &config)
{
	/* basic machine hardware */
	M68000(config, m_maincpu, 8000000);
	m_maincpu->set_addrmap(AS_PROGRAM, &sphinx40_state::main_map);

	const attotime irq_period = attotime::from_hz(8000000 / 0x1000);
	m_maincpu->set_periodic_int(FUNC(sphinx40_state::irq4_line_hold), irq_period);

	PIA6821(config, m_pia, 0);
	m_pia->writepa_handler().set(FUNC(sphinx40_state::cb_mux_w));
	m_pia->writepb_handler().set(FUNC(sphinx40_state::cb_leds_w));
	m_pia->cb2_handler().set(m_dac, FUNC(dac_bit_interface::write));

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);
	ADDRESS_MAP_BANK(config, "nvram_map").set_map(&sphinx40_state::nvram_map).set_options(ENDIANNESS_BIG, 8, 11);

	SENSORBOARD(config, m_board).set_type(sensorboard_device::MAGNETS);
	m_board->init_cb().set(m_board, FUNC(sensorboard_device::preset_chess));
	m_board->set_delay(attotime::from_msec(150));

	/* video hardware */

	PWM_DISPLAY(config, m_display).set_size(8, 8);
	config.set_default_layout(layout_cxg_sphinx40);

	/* sound hardware */
	SPEAKER(config, "speaker").front_center();
	DAC_1BIT(config, m_dac).add_route(ALL_OUTPUTS, "speaker", 0.25);
	VOLTAGE_REGULATOR(config, "vref").add_route(0, "dac", 1.0, DAC_VREF_POS_INPUT);
}



/******************************************************************************
    ROM Definitions
******************************************************************************/

ROM_START( sphinx40 )
	ROM_REGION16_BE( 0x10000, "maincpu", 0 )
	ROM_LOAD16_BYTE("gold.u3", 0x0000, 0x8000, CRC(e7cccd12) SHA1(4542f62963ab78796626c0c938e39e715d1c19f8) )
	ROM_LOAD16_BYTE("orange.u2", 0x0001, 0x8000, CRC(9e0bbd15) SHA1(5867f35489d15c1e395f6b2aa91a76d74ad6f2f4) )
ROM_END

} // anonymous namespace



/******************************************************************************
    Drivers
******************************************************************************/

/*    YEAR  NAME      PARENT  COMPAT  MACHINE   INPUT     CLASS           INIT        COMPANY, FULLNAME, FLAGS */
CONS( 1987, sphinx40, 0,      0,      sphinx40, sphinx40, sphinx40_state, empty_init, "CXG Systems / Newcrest Technology", "Sphinx 40", MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK | MACHINE_NOT_WORKING )
