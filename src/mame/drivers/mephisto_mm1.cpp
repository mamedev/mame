// license:BSD-3-Clause
// copyright-holders:hap
// thanks-to:Berger
/******************************************************************************

Mephisto MM I, the first H+G slide-in chesscomputer module

The module was included with either the Modular or Modular Exclusive chessboards.
Initially, the module itself didn't have a name. It was only later in retrospect,
after the release of Modul MM II that it became known as the MM I.

Hardware notes:
- PCB label: HGS 10 121 01
- CDP1806 @ 8MHz, 6.5V (IRQ from internal timer)
- 32KB ROM (2*D27128, or HN613256P)
- 4KB RAM (2*HM6116LP-3)
- CDP1853CE, CD4011BE, 3*40373BP, 4556BE
- modular slot, 18-button keypad, beeper

It supports the HG 170 opening book module.
LCD module is assumed to be same as MM II and others.

TODO:
- doesn't work, MAME doesn't emulate 1806 CPU

******************************************************************************/

#include "emu.h"
#include "cpu/cosmac/cosmac.h"
#include "machine/sensorboard.h"
#include "sound/dac.h"
#include "sound/volt_reg.h"
#include "video/pwm.h"
#include "speaker.h"

// internal artwork
//#include "mephisto_mm1.lh" // clickable


namespace {

class mm1_state : public driver_device
{
public:
	mm1_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_board(*this, "board"),
		m_display(*this, "display"),
		m_dac(*this, "dac"),
		m_inputs(*this, "IN.%u", 0)
	{ }

	// machine drivers
	void mm1(machine_config &config);

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;

private:
	// devices/pointers
	required_device<cdp1802_device> m_maincpu;
	required_device<sensorboard_device> m_board;
	required_device<pwm_display_device> m_display;
	required_device<dac_bit_interface> m_dac;
	required_ioport_array<8> m_inputs;

	// address maps
	void mm1_map(address_map &map);
	void mm1_io(address_map &map);

	// I/O handlers
	INTERRUPT_GEN_MEMBER(interrupt);
	void update_display();
	DECLARE_READ_LINE_MEMBER(clear_r);
	DECLARE_WRITE_LINE_MEMBER(q_w);
	DECLARE_WRITE8_MEMBER(sound_w);
	DECLARE_WRITE8_MEMBER(lcd_w);
	DECLARE_WRITE8_MEMBER(board_w);
	DECLARE_WRITE8_MEMBER(led_w);
	DECLARE_WRITE8_MEMBER(keypad_w);
	DECLARE_READ8_MEMBER(board_r);
	template<int P> DECLARE_READ_LINE_MEMBER(keypad_r);

	bool m_reset;
	u8 m_lcd_mask;
	u8 m_digit_idx;
	u8 m_digit_data[4];

	u8 m_kp_mux;
	u8 m_cb_mux;
	u8 m_led_data;
};

void mm1_state::machine_start()
{
	// zerofill
	m_reset = false;
	m_lcd_mask = 0;
	m_digit_idx = 0;
	memset(m_digit_data, 0, sizeof(m_digit_data));

	m_kp_mux = 0;
	m_cb_mux = 0;
	m_led_data = 0;

	// register for savestates
	save_item(NAME(m_reset));
	save_item(NAME(m_lcd_mask));
	save_item(NAME(m_digit_idx));
	save_item(NAME(m_digit_data));

	save_item(NAME(m_kp_mux));
	save_item(NAME(m_cb_mux));
	save_item(NAME(m_led_data));
}

void mm1_state::machine_reset()
{
	m_reset = true;
	m_digit_idx = 0;
}



/******************************************************************************
    I/O
******************************************************************************/

INTERRUPT_GEN_MEMBER(mm1_state::interrupt)
{
	m_maincpu->set_input_line(COSMAC_INPUT_LINE_INT, HOLD_LINE);
}

READ_LINE_MEMBER(mm1_state::clear_r)
{
	// CLEAR low + RESET high resets cpu
	int ret = (m_reset) ? 0 : 1;
	m_reset = false;
	return ret;
}

WRITE_LINE_MEMBER(mm1_state::q_w)
{
	// Q: LCD digit data mask
	// also assume LCD update on rising edge
	if (state && !m_lcd_mask)
	{
		for (int i = 0; i < 4; i++)
			m_display->write_row(i, m_digit_data[i]);
		m_display->update();
	}

	m_lcd_mask = state ? 0xff : 0;
}

WRITE8_MEMBER(mm1_state::lcd_w)
{
	// d0-d7: write/shift LCD digit (4*CD4015)
	// note: last digit "dp" is the colon in the middle
	m_digit_data[m_digit_idx] = data ^ m_lcd_mask;
	m_digit_idx = (m_digit_idx + 1) & 3;
}

WRITE8_MEMBER(mm1_state::sound_w)
{
	// d0: speaker out
	m_dac->write(~data & 1);
}

void mm1_state::update_display()
{
	// 64 chessboard leds
	m_display->matrix_partial(4, 8, m_cb_mux, m_led_data);
}

WRITE8_MEMBER(mm1_state::led_w)
{
	// d0-d7: chessboard led data
	m_led_data = data;
	update_display();
}

WRITE8_MEMBER(mm1_state::board_w)
{
	// d0-d7: chessboard input/led mux
	m_cb_mux = ~data;
	update_display();
}

READ8_MEMBER(mm1_state::board_r)
{
	u8 data = 0;

	// d0-d7: multiplexed inputs (chessboard)
	for (int i = 0; i < 8; i++)
		if (BIT(m_cb_mux, i))
			data |= m_board->read_rank(i);

	return ~data;
}

WRITE8_MEMBER(mm1_state::keypad_w)
{
	// d0-d7: keypad input mux
	m_kp_mux = ~data;
}

template<int P>
READ_LINE_MEMBER(mm1_state::keypad_r)
{
	u8 data = 0;

	// EF3,EF4: multiplexed inputs (keypad)
	for (int i = 0; i < 8; i++)
		if (BIT(m_kp_mux, i))
			data |= m_inputs[i]->read();

	return data >> P & 1;
}



/******************************************************************************
    Address Maps
******************************************************************************/

void mm1_state::mm1_map(address_map &map)
{
	map(0x0000, 0x7fff).rom().nopw();
	map(0xd000, 0xdfff).mirror(0x2000).ram();
}

void mm1_state::mm1_io(address_map &map)
{
	map(0x01, 0x01).w(FUNC(mm1_state::sound_w));
	map(0x02, 0x02).w(FUNC(mm1_state::keypad_w));
	map(0x03, 0x03).r(FUNC(mm1_state::board_r));
	map(0x04, 0x04).w(FUNC(mm1_state::board_w));
	map(0x05, 0x05).w(FUNC(mm1_state::led_w));
	map(0x06, 0x06).w(FUNC(mm1_state::lcd_w));
}



/******************************************************************************
    Input Ports
******************************************************************************/

static INPUT_PORTS_START( mm1 )
	PORT_START("IN.0")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_1) // e5
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_2) // 9?

	PORT_START("IN.1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_3) // info?
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_4) // cl?

	PORT_START("IN.2")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_5) // 0?
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_6) // c3

	PORT_START("IN.3")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_7) // pos?
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_8) // ent?

	PORT_START("IN.4")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_Q) // h8
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_W) // d4

	PORT_START("IN.5")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_E) // lev?
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_R) // a1

	PORT_START("IN.6")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_T) // g7
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_Y) // f6

	PORT_START("IN.7")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_U) // memo?
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_I) // b2
INPUT_PORTS_END



/******************************************************************************
    Machine Drivers
******************************************************************************/

void mm1_state::mm1(machine_config &config)
{
	/* basic machine hardware */
	CDP1802(config, m_maincpu, 8_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &mm1_state::mm1_map);
	m_maincpu->set_addrmap(AS_IO, &mm1_state::mm1_io);
	m_maincpu->clear_cb().set(FUNC(mm1_state::clear_r));
	m_maincpu->q_cb().set(FUNC(mm1_state::q_w));
	m_maincpu->ef3_cb().set(FUNC(mm1_state::keypad_r<0>));
	m_maincpu->ef4_cb().set(FUNC(mm1_state::keypad_r<1>));

	m_maincpu->set_periodic_int(FUNC(mm1_state::interrupt), attotime::from_hz(150)); // fake

	SENSORBOARD(config, m_board).set_type(sensorboard_device::MAGNETS);
	m_board->init_cb().set(m_board, FUNC(sensorboard_device::preset_chess));
	m_board->set_delay(attotime::from_msec(200));

	/* video hardware */
	PWM_DISPLAY(config, m_display).set_size(4+8, 8);
	m_display->set_segmask(0xf, 0x7f);
	//config.set_default_layout(layout_mephisto_mm1);

	/* sound hardware */
	SPEAKER(config, "speaker").front_center();
	DAC_1BIT(config, m_dac).add_route(ALL_OUTPUTS, "speaker", 0.25);
	VOLTAGE_REGULATOR(config, "vref").add_route(0, "dac", 1.0, DAC_VREF_POS_INPUT);
}



/******************************************************************************
    ROM Definitions
******************************************************************************/

ROM_START( mm1 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("114", 0x0000, 0x4000, CRC(208b4c43) SHA1(48f891d614fa643f47d099f94aff15a44c2efc07) ) // D27128
	ROM_LOAD("214", 0x4000, 0x4000, CRC(93734e49) SHA1(9ad6c191074c4122300f059e2ef9cfeff7b81463) ) // "
ROM_END

} // anonymous namespace



/******************************************************************************
    Drivers
******************************************************************************/

//    YEAR  NAME  PARENT CMP MACHINE INPUT STATE      INIT        COMPANY, FULLNAME, FLAGS
CONS( 1983, mm1,  0,      0, mm1,    mm1,  mm1_state, empty_init, "Hegener + Glaser", "Mephisto MM I", MACHINE_SUPPORTS_SAVE | MACHINE_NOT_WORKING )
