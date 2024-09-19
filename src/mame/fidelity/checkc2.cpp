// license:BSD-3-Clause
// copyright-holders:hap
// thanks-to:Sean Riddle
/*******************************************************************************

Fidelity Checker Challenger (CR)

Even though it has fewer levels and presumedly a weaker program, this one
is a couple of months newer than model ACR (see cc10.cpp).

Hardware notes:
- PCB label: P261A
- NEC uCOM-43 MCU, label D546C 055 (die label same)
- 256x4 RAM (NEC D2101AL-4)
- 4-digit 7seg led display + 2 other leds, no sound

TODO:
- according to the manual, the right digits should blink when the CPU
  opponent wants to make a double jump, but it doesn't blink on MAME

*******************************************************************************/

#include "emu.h"

#include "cpu/ucom4/ucom4.h"
#include "video/pwm.h"

// internal artwork
#include "fidel_cr.lh"


namespace {

class cr_state : public driver_device
{
public:
	cr_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_display(*this, "display"),
		m_inputs(*this, "IN.%u", 0)
	{ }

	void cr(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	// devices/pointers
	required_device<ucom4_cpu_device> m_maincpu;
	required_device<pwm_display_device> m_display;
	required_ioport_array<4> m_inputs;

	u8 m_ram[0x100] = { };
	u8 m_ram_address = 0;
	u8 m_ram_data = 0;
	u8 m_ram_control = 0;

	u8 m_inp_mux = 0;
	u8 m_led_select = 0;
	u8 m_7seg_data = 0;

	// I/O handlers
	void update_display();
	void segsel_w(u8 data);
	void seg0_w(u8 data);
	void seg1_w(u8 data);
	void control_w(u8 data);
	void ram_w(u8 data);
	u8 ram_r();
	void rama0_w(u8 data);
	void rama1_w(u8 data);
	u8 input_r();
};

void cr_state::machine_start()
{
	// register for savestates
	save_item(NAME(m_ram));
	save_item(NAME(m_ram_address));
	save_item(NAME(m_ram_data));
	save_item(NAME(m_ram_control));
	save_item(NAME(m_inp_mux));
	save_item(NAME(m_led_select));
	save_item(NAME(m_7seg_data));
}



/*******************************************************************************
    I/O
*******************************************************************************/

void cr_state::update_display()
{
	m_display->matrix(m_led_select, m_7seg_data);
}

void cr_state::segsel_w(u8 data)
{
	// G: 7seg select
	m_led_select = (m_led_select & 0x30) | data;
	update_display();
}

void cr_state::seg0_w(u8 data)
{
	// I: 7seg data(low)
	m_7seg_data = (m_7seg_data & 0x78) | (data & 7);
	update_display();
}

void cr_state::seg1_w(u8 data)
{
	// H: 7seg data(high)
	m_7seg_data = (m_7seg_data & 0x07) | data << 3;
	update_display();
}

void cr_state::control_w(u8 data)
{
	// F0,F1: direct leds
	m_led_select = (m_led_select & 0x0f) | (data << 4 & 0x30);
	update_display();

	// F2: RAM R/W, F3: RAM CE
	if ((data & 0xc) == 8 && ~m_ram_control & 2)
		m_ram[m_ram_address] = m_ram_data;
	m_ram_control = data >> 2;
}

void cr_state::ram_w(u8 data)
{
	// C: RAM DI
	m_ram_data = data;
}

u8 cr_state::ram_r()
{
	// B: RAM DO
	return m_ram[m_ram_address];
}

void cr_state::rama0_w(u8 data)
{
	// E: RAM address(low), input mux
	m_ram_address = (m_ram_address & 0xf0) | data;
	m_inp_mux = data;
}

void cr_state::rama1_w(u8 data)
{
	// D: RAM address(high)
	m_ram_address = (m_ram_address & 0x0f) | data << 4;
}

u8 cr_state::input_r()
{
	u8 data = 0;

	// A: multiplexed inputs
	for (int i = 0; i < 4; i++)
		if (BIT(m_inp_mux, i))
			data |= m_inputs[i]->read();

	return data;
}



/*******************************************************************************
    Input Ports
*******************************************************************************/

static INPUT_PORTS_START( cr )
	PORT_START("IN.0")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("0") PORT_CODE(KEYCODE_0) PORT_CODE(KEYCODE_0_PAD)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("PV") PORT_CODE(KEYCODE_V)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("LV") PORT_CODE(KEYCODE_L)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("RE") PORT_CODE(KEYCODE_R)

	PORT_START("IN.1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("1") PORT_CODE(KEYCODE_1) PORT_CODE(KEYCODE_1_PAD)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("4") PORT_CODE(KEYCODE_4) PORT_CODE(KEYCODE_4_PAD)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("7") PORT_CODE(KEYCODE_7) PORT_CODE(KEYCODE_7_PAD)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("CL") PORT_CODE(KEYCODE_DEL) PORT_CODE(KEYCODE_BACKSPACE)

	PORT_START("IN.2")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("2") PORT_CODE(KEYCODE_2) PORT_CODE(KEYCODE_2_PAD)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("5") PORT_CODE(KEYCODE_5) PORT_CODE(KEYCODE_5_PAD)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("8") PORT_CODE(KEYCODE_8) PORT_CODE(KEYCODE_8_PAD)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("TO") PORT_CODE(KEYCODE_T)

	PORT_START("IN.3")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("3") PORT_CODE(KEYCODE_3) PORT_CODE(KEYCODE_3_PAD)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("6") PORT_CODE(KEYCODE_6) PORT_CODE(KEYCODE_6_PAD)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("9") PORT_CODE(KEYCODE_9) PORT_CODE(KEYCODE_9_PAD)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("EN") PORT_CODE(KEYCODE_ENTER) PORT_CODE(KEYCODE_ENTER_PAD)
INPUT_PORTS_END



/*******************************************************************************
    Machine Configs
*******************************************************************************/

void cr_state::cr(machine_config &config)
{
	// basic machine hardware
	NEC_D546(config, m_maincpu, 400'000); // approximation
	m_maincpu->read_a().set(FUNC(cr_state::input_r));
	m_maincpu->read_b().set(FUNC(cr_state::ram_r));
	m_maincpu->write_c().set(FUNC(cr_state::ram_w));
	m_maincpu->write_d().set(FUNC(cr_state::rama1_w));
	m_maincpu->write_e().set(FUNC(cr_state::rama0_w));
	m_maincpu->write_f().set(FUNC(cr_state::control_w));
	m_maincpu->write_g().set(FUNC(cr_state::segsel_w));
	m_maincpu->write_h().set(FUNC(cr_state::seg1_w));
	m_maincpu->write_i().set(FUNC(cr_state::seg0_w));

	// video hardware
	PWM_DISPLAY(config, m_display).set_size(6, 7);
	m_display->set_segmask(0xf, 0x7f);
	config.set_default_layout(layout_fidel_cr);
}



/*******************************************************************************
    ROM Definitions
*******************************************************************************/

ROM_START( checkc2 )
	ROM_REGION( 0x0800, "maincpu", 0 )
	ROM_LOAD( "d546c-055", 0x0000, 0x0800, CRC(ee2de21e) SHA1(ca727093dc36dc15453bc5cca4e559fdc8242355) )
ROM_END

} // anonymous namespace



/*******************************************************************************
    Drivers
*******************************************************************************/

//    YEAR  NAME     PARENT  COMPAT  MACHINE  INPUT CLASS     INIT        COMPANY, FULLNAME, FLAGS
SYST( 1978, checkc2, 0,      0,      cr,      cr,   cr_state, empty_init, "Fidelity Electronics", "Checker Challenger (model CR, 2 levels)", MACHINE_SUPPORTS_SAVE | MACHINE_NO_SOUND_HW )
