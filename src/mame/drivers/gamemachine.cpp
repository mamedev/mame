// license:BSD-3-Clause
// copyright-holders:hap
// thanks-to:Sean Riddle
/******************************************************************************

Waddingtons 2001: The Game Machine

It's a tabletop electronic game machine + calculator.
It was possibly created by VTech, but they didn't distribute it by themselves
until later in 1980 as the Computer Game System. There's also a handheld version
"Mini Game Machine". VTech later made a sequel "Game Machine 2" with 5 games.

After boot, press a number to start a game:
0: 4 Function Calculator (not a game)
1: Shooting Gallery
2: Black Jack
3: Code Hunter
4: Grand Prix

Screen and keyboard overlays were provided for each game, though the default keyboard
labels already show the alternate functions.

hardware notes:
- Mostek MK3870 MCU, 2KB internal ROM
- 12 digits 7seg VFD panel
- MC1455P(555 timer) + bunch of discrete components for sound, see schematic:
  http://seanriddle.com/gamemachineaudio.JPG

TODO:
- discrete sound, currently it's emulated crudely, just enough to make it beep when supposed to
- MK3870 is not emulated in MAME, plain F8 is used here instead

******************************************************************************/

#include "emu.h"
#include "cpu/f8/f8.h"
#include "machine/timer.h"
#include "sound/beep.h"
#include "speaker.h"

#include "tgm.lh"

namespace {

class tgm_state : public driver_device
{
public:
	tgm_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_beeper(*this, "beeper"),
		m_vfd_delay(*this, "vfd_delay_%u", 0),
		m_inp_matrix(*this, "IN.%u", 0),
		m_out_digit(*this, "digit%u", 0U)
	{ }

	void tgm(machine_config &config);

protected:
	virtual void machine_start() override;

private:
	// devices/pointers
	required_device<cpu_device> m_maincpu;
	required_device<beep_device> m_beeper;
	required_device_array<timer_device, 12> m_vfd_delay;
	required_ioport_array<10> m_inp_matrix;
	output_finder<12> m_out_digit;

	void main_map(address_map &map);
	void main_io(address_map &map);

	TIMER_DEVICE_CALLBACK_MEMBER(vfd_delay_off);

	void display_update(u16 edge);
	DECLARE_WRITE8_MEMBER(mux1_w);
	DECLARE_WRITE8_MEMBER(mux2_w);
	DECLARE_WRITE8_MEMBER(_7seg_w);
	DECLARE_READ8_MEMBER(input_r);
	DECLARE_WRITE8_MEMBER(sound_w);
	DECLARE_READ8_MEMBER(timer_r);

	u16 m_inp_mux;
	u16 m_digit_select;
	u8 m_7seg_data;
};

void tgm_state::machine_start()
{
	// resolve handlers
	m_out_digit.resolve();

	// zerofill
	m_inp_mux = 0;
	m_digit_select = 0;
	m_7seg_data = 0;

	// register for savestates
	save_item(NAME(m_inp_mux));
	save_item(NAME(m_digit_select));
	save_item(NAME(m_7seg_data));
}


/******************************************************************************
    Devices, I/O
******************************************************************************/

// VFD handling

TIMER_DEVICE_CALLBACK_MEMBER(tgm_state::vfd_delay_off)
{
	// clear VFD outputs
	if (!BIT(m_digit_select, param))
		m_out_digit[param] = 0;
}

void tgm_state::display_update(u16 edge)
{
	for (int i = 0; i < 12; i++)
	{
		// output VFD digit data
		if (BIT(m_digit_select, i))
			m_out_digit[i] = m_7seg_data;

		// they're strobed, so on falling edge, delay them going off to prevent flicker
		// BTANB: some digit segments get stuck after crashing in the GP game, it's not due to the simulated delay here
		else if (BIT(edge, i))
			m_vfd_delay[i]->adjust(attotime::from_msec(20), i);
	}
}


// MK3870 ports

WRITE8_MEMBER(tgm_state::mux1_w)
{
	// P00-P06: input mux part
	m_inp_mux = (m_inp_mux & 7) | (data << 3 & 0x3f8);

	// P00-P07: digit select part
	u16 prev = m_digit_select;
	m_digit_select = (m_digit_select & 0xf) | (data << 4);
	display_update(m_digit_select ^ prev);
}

WRITE8_MEMBER(tgm_state::mux2_w)
{
	// P15-P17: input mux part
	m_inp_mux = (m_inp_mux & 0x3f8) | (data >> 5 & 7);

	// P14-P17: digit select part
	u16 prev = m_digit_select;
	m_digit_select = (m_digit_select & 0xff0) | (data >> 4 & 0xf);
	display_update(m_digit_select ^ prev);
}

WRITE8_MEMBER(tgm_state::_7seg_w)
{
	// P50-P57: digit 7seg data
	m_7seg_data = bitswap<8>(data,0,1,2,3,4,5,6,7);
	display_update(0);
}

READ8_MEMBER(tgm_state::input_r)
{
	u8 data = 0;

	// P12,P13: multiplexed inputs
	for (int i = 0; i < 10; i++)
		if (m_inp_mux >> i & 1)
			data |= m_inp_matrix[i]->read();

	return data << 2;
}

WRITE8_MEMBER(tgm_state::sound_w)
{
	// P40: 555 reset
	m_beeper->set_state(~data & 1);

	// P42-P46: through caps, then 555 trigger/treshold
	u8 pitch = 0x20 - bitswap<5>(data,6,5,2,3,4);
	m_beeper->set_clock(64 * pitch);

	// P41: polarized cap, then 555 ctrl
	// P47: resistor, then 555 ctrl
	//..
}

READ8_MEMBER(tgm_state::timer_r)
{
	// MK3870 internal timer register (used as RNG here)
	return machine().rand();
}



/******************************************************************************
    Address Maps
******************************************************************************/

void tgm_state::main_map(address_map &map)
{
	map.global_mask(0x7ff);
	map(0x0000, 0x07ff).rom();
}

void tgm_state::main_io(address_map &map)
{
	map(0x00, 0x00).w(FUNC(tgm_state::mux1_w));
	map(0x01, 0x01).rw(FUNC(tgm_state::input_r), FUNC(tgm_state::mux2_w));
	map(0x04, 0x04).w(FUNC(tgm_state::sound_w));
	map(0x05, 0x05).w(FUNC(tgm_state::_7seg_w));
	map(0x07, 0x07).r(FUNC(tgm_state::timer_r));
}



/******************************************************************************
    Input Ports
******************************************************************************/

static INPUT_PORTS_START( tgm )
	PORT_START("IN.0")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_COLON) PORT_CODE(KEYCODE_DEL) PORT_CODE(KEYCODE_BACKSPACE) PORT_NAME("CL")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_SLASH) PORT_CODE(KEYCODE_9) PORT_CODE(KEYCODE_9_PAD) PORT_NAME("9")

	PORT_START("IN.1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_L) PORT_CODE(KEYCODE_SLASH_PAD) PORT_NAME(UTF8_DIVIDE)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_STOP) PORT_CODE(KEYCODE_8) PORT_CODE(KEYCODE_8_PAD) PORT_NAME("8")

	PORT_START("IN.2")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_K) PORT_CODE(KEYCODE_ASTERISK) PORT_NAME(UTF8_MULTIPLY)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_COMMA) PORT_CODE(KEYCODE_7) PORT_CODE(KEYCODE_7_PAD) PORT_NAME("7")

	PORT_START("IN.3")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_J) PORT_CODE(KEYCODE_MINUS_PAD) PORT_NAME("-=")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_M) PORT_CODE(KEYCODE_6) PORT_CODE(KEYCODE_6_PAD) PORT_NAME("6")

	PORT_START("IN.4")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_H) PORT_CODE(KEYCODE_PLUS_PAD) PORT_NAME("+=")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_N) PORT_CODE(KEYCODE_5) PORT_CODE(KEYCODE_5_PAD) PORT_NAME("5")

	PORT_START("IN.5")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_G) PORT_CODE(KEYCODE_DEL_PAD) PORT_NAME(".")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_B) PORT_CODE(KEYCODE_4) PORT_CODE(KEYCODE_4_PAD) PORT_NAME("4")

	PORT_START("IN.6")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_F) PORT_CODE(KEYCODE_MINUS) PORT_NAME("+/-")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_V) PORT_CODE(KEYCODE_3) PORT_CODE(KEYCODE_3_PAD) PORT_NAME("3")

	PORT_START("IN.7")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_D) PORT_CODE(KEYCODE_END) PORT_NAME("MR")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_C) PORT_CODE(KEYCODE_2) PORT_CODE(KEYCODE_2_PAD) PORT_NAME("2")

	PORT_START("IN.8")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_S) PORT_CODE(KEYCODE_HOME) PORT_NAME("MS")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_X) PORT_CODE(KEYCODE_1) PORT_CODE(KEYCODE_1_PAD) PORT_NAME("1")

	PORT_START("IN.9")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_A) PORT_CODE(KEYCODE_ENTER) PORT_CODE(KEYCODE_ENTER_PAD) PORT_NAME("Return")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_Z) PORT_CODE(KEYCODE_0) PORT_CODE(KEYCODE_0_PAD) PORT_NAME("0")
INPUT_PORTS_END



/******************************************************************************
    Machine Configs
******************************************************************************/

void tgm_state::tgm(machine_config &config)
{
	/* basic machine hardware */
	F8(config, m_maincpu, 2000000); // measured around 2.1MHz
	m_maincpu->set_addrmap(AS_PROGRAM, &tgm_state::main_map);
	m_maincpu->set_addrmap(AS_IO, &tgm_state::main_io);

	/* video hardware */
	for (int i = 0; i < 12; i++)
		TIMER(config, m_vfd_delay[i]).configure_generic(FUNC(tgm_state::vfd_delay_off));

	config.set_default_layout(layout_tgm);

	/* sound hardware */
	SPEAKER(config, "speaker").front_center();
	BEEP(config, m_beeper, 0).add_route(ALL_OUTPUTS, "speaker", 0.25);
}



/******************************************************************************
    ROM Definitions
******************************************************************************/

ROM_START( 2001tgm )
	ROM_REGION( 0x0800, "maincpu", 0 )
	ROM_LOAD("mk14154n_2001", 0x0000, 0x0800, CRC(6d524c32) SHA1(73d84e59952b751c76dff8bf259b98e1f9136b41) )
ROM_END

} // anonymous namespace



/******************************************************************************
    Drivers
******************************************************************************/

//    YEAR  NAME     PARENT CMP MACHINE  INPUT  CLASS      INIT        COMPANY, FULLNAME, FLAGS
COMP( 1978, 2001tgm, 0,      0, tgm,     tgm,   tgm_state, empty_init, "Waddingtons", "2001: The Game Machine", MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_SOUND )
