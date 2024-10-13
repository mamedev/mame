// license:BSD-3-Clause
// copyright-holders:hap
// thanks-to:Michael Kamprath
/*******************************************************************************

Logix Teammate Game Computer
Stylized as T.E.A.M.M.A.T.E.: Total Electronic Advanced Microprocessing
Maneuvers And Tactics Equipment

It's a tabletop with built-in mini games, led display overlays were included.
Two versions are known, one with black hexadecimal keys, and one with blue keys.

Hardware notes:
- Mostek 3870 @ ~3.6MHz
- 2*7seg led + 16 leds, 1-bit sound

*******************************************************************************/

#include "emu.h"

#include "cpu/f8/f8.h"
#include "machine/f3853.h"
#include "sound/dac.h"
#include "video/pwm.h"

#include "speaker.h"

// internal artwork
#include "teammate.lh"


namespace {

class teammate_state : public driver_device
{
public:
	teammate_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_display(*this, "display"),
		m_dac(*this, "dac"),
		m_inputs(*this, "IN.%u", 0)
	{ }

	void teammate(machine_config &config);

	// P3 button is tied to MCU RESET pin
	DECLARE_INPUT_CHANGED_MEMBER(reset_button) { m_maincpu->set_input_line(INPUT_LINE_RESET, newval ? ASSERT_LINE : CLEAR_LINE); }

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;
	required_device<pwm_display_device> m_display;
	required_device<dac_bit_interface> m_dac;
	required_ioport_array<3> m_inputs;

	u8 m_inp_sel = 0;
	u8 m_input = 0;
	u8 m_sound = 0;
	u8 m_select = 0;
	u8 m_led_data = 0;

	void main_map(address_map &map) ATTR_COLD;
	void main_io(address_map &map) ATTR_COLD;

	void select_w(u8 data);
	u8 select_r();
	void led_w(u8 data);
	u8 led_r();
	void input_w(u8 data);
	u8 input_r();
	void sound_w(u8 data);
	u8 sound_r();
};

void teammate_state::machine_start()
{
	save_item(NAME(m_inp_sel));
	save_item(NAME(m_input));
	save_item(NAME(m_sound));
	save_item(NAME(m_select));
	save_item(NAME(m_led_data));
}



/*******************************************************************************
    I/O
*******************************************************************************/

void teammate_state::select_w(u8 data)
{
	// P00,P01,P06,P07: led select
	m_display->write_my((data >> 4 & 0xc) | (data & 3));

	// P02,P03,P05: input select
	// P04: N/C
	m_inp_sel = (data >> 3 & 4) | (data >> 2 & 3);
	m_select = data;
}

u8 teammate_state::select_r()
{
	return m_select;
}

void teammate_state::led_w(u8 data)
{
	// P10-P17: DS8817N to leds
	m_display->write_mx(bitswap<8>(~data,0,1,2,3,4,5,6,7));
	m_led_data = data;
}

u8 teammate_state::led_r()
{
	return m_led_data;
}

void teammate_state::input_w(u8 data)
{
	m_input = data;
}

u8 teammate_state::input_r()
{
	u8 data = 0;

	// P40-P47: read buttons
	for (int i = 0; i < 3; i++)
		if (BIT(m_inp_sel, i))
			data |= m_inputs[i]->read();

	return data | m_input;
}

void teammate_state::sound_w(u8 data)
{
	// P50: speaker out
	m_dac->write(data & 1);
	m_sound = data;
}

u8 teammate_state::sound_r()
{
	return m_sound;
}



/*******************************************************************************
    Address Maps
*******************************************************************************/

void teammate_state::main_map(address_map &map)
{
	map.global_mask(0x7ff);
	map(0x0000, 0x07ff).rom();
}

void teammate_state::main_io(address_map &map)
{
	map(0x00, 0x00).rw(FUNC(teammate_state::select_r), FUNC(teammate_state::select_w));
	map(0x01, 0x01).rw(FUNC(teammate_state::led_r), FUNC(teammate_state::led_w));
	map(0x04, 0x07).rw("psu", FUNC(f38t56_device::read), FUNC(f38t56_device::write));
}



/*******************************************************************************
    Input Ports
*******************************************************************************/

static INPUT_PORTS_START( teammate )
	PORT_START("IN.0")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_D) PORT_CHAR('D')
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_8) PORT_CODE(KEYCODE_8_PAD) PORT_CHAR('8')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_5) PORT_CODE(KEYCODE_5_PAD) PORT_CHAR('5')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_2) PORT_CODE(KEYCODE_2_PAD) PORT_CHAR('2')
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_1) PORT_CODE(KEYCODE_1_PAD) PORT_CHAR('1')
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_4) PORT_CODE(KEYCODE_4_PAD) PORT_CHAR('4')
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_7) PORT_CODE(KEYCODE_7_PAD) PORT_CHAR('7')
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_0) PORT_CODE(KEYCODE_0_PAD) PORT_CHAR('0')

	PORT_START("IN.1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F) PORT_CHAR('F')
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_C) PORT_CHAR('C')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_B) PORT_CHAR('B')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_A) PORT_CHAR('A')
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_3) PORT_CODE(KEYCODE_3_PAD) PORT_CHAR('3')
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_6) PORT_CODE(KEYCODE_6_PAD) PORT_CHAR('6')
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_9) PORT_CODE(KEYCODE_9_PAD) PORT_CHAR('9')
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_E) PORT_CHAR('E')

	PORT_START("IN.2")
	PORT_BIT(0x0f, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F1) PORT_NAME("P1")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F4) PORT_NAME("P4")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F2) PORT_NAME("P2")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("RESET")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F3) PORT_CHANGED_MEMBER(DEVICE_SELF, teammate_state, reset_button, 0) PORT_NAME("P3")
INPUT_PORTS_END



/*******************************************************************************
    Machine Configs
*******************************************************************************/

void teammate_state::teammate(machine_config &config)
{
	// basic machine hardware
	F8(config, m_maincpu, 3'600'000/2); // R/C osc, approximation
	m_maincpu->set_addrmap(AS_PROGRAM, &teammate_state::main_map);
	m_maincpu->set_addrmap(AS_IO, &teammate_state::main_io);
	m_maincpu->set_irq_acknowledge_callback("psu", FUNC(f38t56_device::int_acknowledge));

	f38t56_device &psu(F38T56(config, "psu", 3'600'000/2));
	psu.set_int_vector(0x20);
	psu.int_req_callback().set_inputline("maincpu", F8_INPUT_LINE_INT_REQ);
	psu.read_a().set(FUNC(teammate_state::input_r));
	psu.write_a().set(FUNC(teammate_state::input_w));
	psu.read_b().set(FUNC(teammate_state::sound_r));
	psu.write_b().set(FUNC(teammate_state::sound_w));

	// video hardware
	PWM_DISPLAY(config, m_display).set_size(4, 8);
	m_display->set_segmask(0xc, 0x7f);

	config.set_default_layout(layout_teammate);

	// sound hardware
	SPEAKER(config, "speaker").front_center();
	DAC_1BIT(config, m_dac).add_route(ALL_OUTPUTS, "speaker", 0.25);
}



/*******************************************************************************
    ROM Definitions
*******************************************************************************/

ROM_START( teammate )
	ROM_REGION( 0x0800, "maincpu", 0 )
	ROM_LOAD("mk14194n", 0x0000, 0x0800, CRC(15615d03) SHA1(4448a64888c68d6bf3555dcce736ad0126515843) )
ROM_END

} // anonymous namespace



/******************************************************************************
    Drivers
******************************************************************************/

//    YEAR  NAME      PARENT  COMPAT  MACHINE   INPUT     CLASS           INIT        COMPANY, FULLNAME, FLAGS
SYST( 1978, teammate, 0,      0,      teammate, teammate, teammate_state, empty_init, "Logix", "Teammate Game Computer", MACHINE_SUPPORTS_SAVE )
