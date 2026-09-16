// license:BSD-3-Clause
// copyright-holders:hap
// thanks-to:Sean Riddle
/*******************************************************************************

Tryom Gammon Master series, backgammon computer

Gammon Master (published by Cardinal) (not dumped yet)
Gammonmaster II (standard model and Doubler model)

According to the manual, the two GM II have the same housing. GM II has labels
intended for The Doubler, but they don't apply to this model.

Hardware notes:
- PCB label: AN0178 REV I, JRH
- Motorola MC6800P @ ~0.85MHz (no XTAL, main clock and NMI are from a 74124)
- 512 bytes RAM (4*NEC D2111AL-4), 4KB+2KB ROM
- DL-4507 4-digit 7seg panel + other leds for dice and status
- no sound, non-electronic backgammon board is attached

To start a game, press ST twice, then EN. Read the manual for more information.
For test mode, press BO after boot, followed by a number.

TODO:
- what is IN.5 0x04 for? maybe just for led strobe timing (reads it at PC=E1BB,
  sets a counter in RAM offset E3)
- it locks up when holding the 1 key at boot (happens on real machine too),
  what's the purpose of checking this key at power on?

*******************************************************************************/

#include "emu.h"

#include "cpu/m6800/m6800.h"
#include "video/pwm.h"

#include "gammonm2.lh"


namespace {

class gammonm_state : public driver_device
{
public:
	gammonm_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_display(*this, "display"),
		m_inputs(*this, "IN.%u", 0)
	{ }

	void gammonm2(machine_config &config);

private:
	// devices/pointers
	required_device<cpu_device> m_maincpu;
	required_device<pwm_display_device> m_display;
	required_ioport_array<6> m_inputs;

	void main_map(address_map &map) ATTR_COLD;

	void led_w(offs_t offset, u8 data);
	void digit_w(offs_t offset, u8 data);
	u8 input_r(offs_t offset);
};



/*******************************************************************************
    I/O
*******************************************************************************/

void gammonm_state::led_w(offs_t offset, u8 data)
{
	// a0-a3 + data: misc leds
	m_display->matrix_partial(4, 4, offset, ~data);
}

void gammonm_state::digit_w(offs_t offset, u8 data)
{
	// a0-a3 + data: 7seg leds
	m_display->matrix_partial(0, 4, offset, bitswap<8>(~data,4,6,3,5,2,7,1,0));
}

u8 gammonm_state::input_r(offs_t offset)
{
	u8 data = 0x0f;

	// read multiplexed inputs
	for (int i = 0; i < 5; i++)
		if (!BIT(offset, i))
			data &= m_inputs[i]->read();

	return data | m_inputs[5]->read() << 4;
}



/*******************************************************************************
    Address Maps
*******************************************************************************/

void gammonm_state::main_map(address_map &map)
{
	map(0x0000, 0x01ff).ram();
	map(0x4000, 0x400f).mirror(0x1ff0).w(FUNC(gammonm_state::led_w));
	map(0x6000, 0x601f).mirror(0x1fe0).r(FUNC(gammonm_state::input_r));
	map(0x8000, 0x800f).mirror(0x1ff0).w(FUNC(gammonm_state::digit_w));
	map(0xe000, 0xe7ff).rom();
	map(0xf000, 0xffff).rom();
}



/*******************************************************************************
    Input Ports
*******************************************************************************/

static INPUT_PORTS_START( gammonm2 )
	PORT_START("IN.0")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_S) PORT_NAME("ST")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_DEL) PORT_CODE(KEYCODE_BACKSPACE) PORT_NAME("CL")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_ENTER) PORT_CODE(KEYCODE_ENTER_PAD) PORT_NAME("EN")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_UP) PORT_NAME("Up")

	PORT_START("IN.1")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_B) PORT_NAME("BA")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_O) PORT_NAME("BO")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_V) PORT_NAME("VR")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_DOWN) PORT_NAME("Down")

	PORT_START("IN.2")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_9) PORT_CODE(KEYCODE_9_PAD) PORT_NAME("9")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_0) PORT_CODE(KEYCODE_0_PAD) PORT_NAME("10")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_MINUS) PORT_NAME("11")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_EQUALS) PORT_NAME("12")

	PORT_START("IN.3")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_5) PORT_CODE(KEYCODE_5_PAD) PORT_NAME("5")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_6) PORT_CODE(KEYCODE_6_PAD) PORT_NAME("6")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_7) PORT_CODE(KEYCODE_7_PAD) PORT_NAME("7")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_8) PORT_CODE(KEYCODE_8_PAD) PORT_NAME("8")

	PORT_START("IN.4")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_1) PORT_CODE(KEYCODE_1_PAD) PORT_NAME("1")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_2) PORT_CODE(KEYCODE_2_PAD) PORT_NAME("2")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_3) PORT_CODE(KEYCODE_3_PAD) PORT_NAME("3")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_4) PORT_CODE(KEYCODE_4_PAD) PORT_NAME("4")

	PORT_START("IN.5")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_UNKNOWN)
	PORT_CONFNAME( 0x08, 0x00, "Version" ) // factory-set
	PORT_CONFSETTING(    0x08, "Standard" )
	PORT_CONFSETTING(    0x00, "The Doubler" )
INPUT_PORTS_END



/*******************************************************************************
    Machine Configs
*******************************************************************************/

void gammonm_state::gammonm2(machine_config &config)
{
	// basic machine hardware
	M6800(config, m_maincpu, 850000); // measured
	m_maincpu->set_addrmap(AS_PROGRAM, &gammonm_state::main_map);

	const attotime nmi_period = attotime::from_hz(262); // measured
	m_maincpu->set_periodic_int(FUNC(gammonm_state::nmi_line_pulse), nmi_period);

	// video hardware
	PWM_DISPLAY(config, m_display).set_size(8, 8);
	m_display->set_segmask(0xf, 0x7f);
	config.set_default_layout(layout_gammonm2);
}



/*******************************************************************************
    ROM Definitions
*******************************************************************************/

ROM_START( gammonm2 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("c38083_bc3.0-s3", 0xe000, 0x0800, CRC(5a82df02) SHA1(dbad870b0fc69c1af892441aa5a851366c7b04a6) )
	ROM_LOAD("d2332c_027",      0xf000, 0x1000, CRC(407e015e) SHA1(09900a672e8cc6f280253544511e6e08a1c78a02) )
ROM_END

} // anonymous namespace



/*******************************************************************************
    Drivers
*******************************************************************************/

//    YEAR  NAME      PARENT  COMPAT  MACHINE   INPUT     CLASS          INIT        COMPANY, FULLNAME, FLAGS
SYST( 1978, gammonm2, 0,      0,      gammonm2, gammonm2, gammonm_state, empty_init, "Tryom", "Gammonmaster II", MACHINE_SUPPORTS_SAVE | MACHINE_NO_SOUND_HW )
