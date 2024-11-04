// license:BSD-3-Clause
// copyright-holders:hap
// thanks-to:Sean Riddle
/*******************************************************************************

Tryom Omar, handheld backgammon computer

Hardware notes (Omar I):
- PCB label: AP0379 REV.8, PC103, JRH
- Fairchild 3870 MCU, custom label
- 9-digit 7seg led panel (3 unused), piezo

Omar II has an LCD instead of 7segs. PCB label is AN2379 REV.6, PC101.

There are more 'sequels', but assumed to be the same game. The newer ones didn't
include a backgammon board.

LED versions: Omar I, Omar IV, Cardinal Electronic Backgammon
LCD versions: Omar II, Omar III, Omar V, Tandy Computerized Backgammon

Confirmed to be the same MCU ROM: Omar II = Omar V.

BTANB:
- omar1 piezo buzzes when you hold down a button

*******************************************************************************/

#include "emu.h"

#include "cpu/f8/f8.h"
#include "machine/f3853.h"
#include "sound/dac.h"
#include "video/pwm.h"

#include "screen.h"
#include "speaker.h"

#include "multibyte.h"

#include "omar1.lh"
#include "omar2.lh"


namespace {

// Omar I / shared

class omar_state : public driver_device
{
public:
	omar_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_psu(*this, "psu"),
		m_display(*this, "display"),
		m_dac(*this, "dac"),
		m_inputs(*this, "IN.%u", 0)
	{ }

	void omar1(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;

protected:
	// devices/pointers
	required_device<cpu_device> m_maincpu;
	required_device<f38t56_device> m_psu;
	required_device<pwm_display_device> m_display;
	required_device<dac_1bit_device> m_dac;
	required_ioport_array<5> m_inputs;

	u8 m_inp_mux = 0;

	void main_map(address_map &map) ATTR_COLD;
	void main_io(address_map &map) ATTR_COLD;

	void input1_w(u8 data);
	void input2_w(u8 data);
	void input3_w(u8 data);
	void input4_w(u8 data);
	u8 input_r();

	virtual void display1_w(u8 data);
	virtual void display2_w(u8 data);
};

void omar_state::machine_start()
{
	save_item(NAME(m_inp_mux));
}


// Omar II

class omar2_state : public omar_state
{
public:
	omar2_state(const machine_config &mconfig, device_type type, const char *tag) :
		omar_state(mconfig, type, tag)
	{ }

	void omar2(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	u8 m_lcd_com = 0;
	u8 m_lcd_data[4] = { };

	void update_lcd();

	virtual void display1_w(u8 data) override;
	virtual void display2_w(u8 data) override;
	void display3_w(u8 data);
	void display4_w(u8 data);
};

void omar2_state::machine_start()
{
	omar_state::machine_start();

	save_item(NAME(m_lcd_com));
	save_item(NAME(m_lcd_data));
}



/*******************************************************************************
    I/O
*******************************************************************************/

// common

void omar_state::input1_w(u8 data)
{
	// P00: input mux part
	m_inp_mux = (m_inp_mux & ~2) | (data << 1 & 2);

	// P07: speaker out
	m_dac->write(BIT(~data, 7));
}

void omar_state::input2_w(u8 data)
{
	// P11: input mux part
	m_inp_mux = (m_inp_mux & ~1) | (data >> 1 & 1);
}

void omar_state::input3_w(u8 data)
{
	// P40,P41: input mux part
	m_inp_mux = (m_inp_mux & ~0xc) | (data << 2 & 0xc);
}

void omar_state::input4_w(u8 data)
{
	// P55: input mux part
	m_inp_mux = (m_inp_mux & ~0x10) | (data >> 1 & 0x10);
}

u8 omar_state::input_r()
{
	u8 data = 0;

	// P42-P46: multiplexed inputs
	for (int i = 0; i < 5; i++)
		if (BIT(m_inp_mux, i))
			data |= m_inputs[i]->read();

	return data << 2;
}


// LED version

void omar_state::display1_w(u8 data)
{
	input1_w(data);

	// P01-P06: digit select
	m_display->write_my(~data >> 1 & 0x3f);
}

void omar_state::display2_w(u8 data)
{
	input2_w(data);

	// P10,P12-P17: digit data
	m_display->write_mx(bitswap<7>(~data,4,5,7,3,0,6,2));
}


// LCD version

void omar2_state::update_lcd()
{
	m_display->write_row(0, m_lcd_com ? ~get_u32le(m_lcd_data) : get_u32le(m_lcd_data));
}

void omar2_state::display1_w(u8 data)
{
	input1_w(data ^ 0x80);

	// P00-P06: LCD data
	m_lcd_data[0] = (m_lcd_data[0] & 0x80) | (data & 0x7f);
	update_lcd();
}

void omar2_state::display2_w(u8 data)
{
	input2_w(data);

	// P10: LCD common
	m_lcd_com = data & 1;

	// P11-P17: LCD data
	m_lcd_data[1] = (m_lcd_data[1] & 0x01) | (data & 0xfe);
	update_lcd();
}

void omar2_state::display3_w(u8 data)
{
	// P40,P47: LCD data
	m_lcd_data[0] = (m_lcd_data[0] & 0x7f) | (data & 0x80);
	m_lcd_data[1] = (m_lcd_data[1] & 0xfe) | (data & 0x01);

	// P41: 4015 data (4015 Q to LCD)
	m_lcd_data[3] = (m_lcd_data[3] << 1) | (BIT(data, 1));
	update_lcd();
}

void omar2_state::display4_w(u8 data)
{
	// P50-P57: LCD data
	m_lcd_data[2] = data;
	update_lcd();
}



/*******************************************************************************
    Address Maps
*******************************************************************************/

void omar_state::main_map(address_map &map)
{
	map.global_mask(0x7ff);
	map(0x0000, 0x07ff).rom();
}

void omar_state::main_io(address_map &map)
{
	map(0x00, 0x00).w(FUNC(omar_state::display1_w));
	map(0x01, 0x01).w(FUNC(omar_state::display2_w));
	map(0x04, 0x07).rw(m_psu, FUNC(f38t56_device::read), FUNC(f38t56_device::write));
}



/*******************************************************************************
    Input Ports
*******************************************************************************/

static INPUT_PORTS_START( omar )
	PORT_START("IN.0")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_O) PORT_NAME("BO")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_V) PORT_NAME("VR")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_D) PORT_NAME("DBL")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_A) PORT_NAME("ACC")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_S) PORT_NAME("ST")

	PORT_START("IN.1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_DOWN) PORT_NAME("Down")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_EQUALS) PORT_NAME("12")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_MINUS) PORT_NAME("11")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_0) PORT_CODE(KEYCODE_0_PAD) PORT_NAME("10")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_9) PORT_CODE(KEYCODE_9_PAD) PORT_NAME("9")

	PORT_START("IN.2")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_ENTER) PORT_CODE(KEYCODE_ENTER_PAD) PORT_NAME("EN")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_8) PORT_CODE(KEYCODE_8_PAD) PORT_NAME("8")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_7) PORT_CODE(KEYCODE_7_PAD) PORT_NAME("7")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_6) PORT_CODE(KEYCODE_6_PAD) PORT_NAME("6")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_5) PORT_CODE(KEYCODE_5_PAD) PORT_NAME("5")

	PORT_START("IN.3")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_UP) PORT_NAME("Up")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_4) PORT_CODE(KEYCODE_4_PAD) PORT_NAME("4")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_3) PORT_CODE(KEYCODE_3_PAD) PORT_NAME("3")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_2) PORT_CODE(KEYCODE_2_PAD) PORT_NAME("2")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_1) PORT_CODE(KEYCODE_1_PAD) PORT_NAME("1")

	PORT_START("IN.4")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_B) PORT_NAME("BAR")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_DEL) PORT_CODE(KEYCODE_BACKSPACE) PORT_NAME("CLR")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_X) PORT_NAME("Dice")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_C) PORT_NAME("CUB")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_UNUSED)
INPUT_PORTS_END



/*******************************************************************************
    Machine Configs
*******************************************************************************/

void omar_state::omar1(machine_config &config)
{
	// basic machine hardware
	F8(config, m_maincpu, 2700000/2); // approximation
	m_maincpu->set_addrmap(AS_PROGRAM, &omar_state::main_map);
	m_maincpu->set_addrmap(AS_IO, &omar_state::main_io);
	m_maincpu->set_irq_acknowledge_callback(m_psu, FUNC(f38t56_device::int_acknowledge));

	F38T56(config, m_psu, 2700000/2);
	m_psu->set_int_vector(0x20);
	m_psu->int_req_callback().set_inputline(m_maincpu, F8_INPUT_LINE_INT_REQ);
	m_psu->read_a().set(FUNC(omar_state::input_r));
	m_psu->write_a().set(FUNC(omar_state::input3_w));
	m_psu->write_b().set(FUNC(omar_state::input4_w));

	// video hardware
	PWM_DISPLAY(config, m_display).set_size(6, 7);
	m_display->set_segmask(0x3f, 0x7f);
	config.set_default_layout(layout_omar1);

	// sound hardware
	SPEAKER(config, "speaker").front_center();
	DAC_1BIT(config, m_dac).add_route(ALL_OUTPUTS, "speaker", 0.25);
}

void omar2_state::omar2(machine_config &config)
{
	omar1(config);

	// basic machine hardware
	m_psu->write_a().append(FUNC(omar2_state::display3_w));
	m_psu->write_b().append(FUNC(omar2_state::display4_w));

	// video hardware
	PWM_DISPLAY(config.replace(), m_display).set_size(1, 32);
	m_display->set_bri_levels(0.25);
	config.set_default_layout(layout_omar2);

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_SVG));
	screen.set_refresh_hz(60);
	screen.set_size(1920/2.5, 412/2.5);
	screen.set_visarea_full();
}



/*******************************************************************************
    ROM Definitions
*******************************************************************************/

ROM_START( omar1 )
	ROM_REGION( 0x0800, "maincpu", 0 )
	ROM_LOAD("lom1ar279", 0x0000, 0x0800, CRC(e1bcee50) SHA1(658d6d8a0af3c1672610c651fea7d0289e4703f7) ) // 3870X-0245
ROM_END

ROM_START( omar2 )
	ROM_REGION( 0x0800, "maincpu", 0 )
	ROM_LOAD("xom2ar1779", 0x0000, 0x0800, CRC(e0d6a119) SHA1(40e35020f483245bcc82443923763b037498a98d) ) // 3870X-0248

	ROM_REGION( 43303, "screen", 0 )
	ROM_LOAD("omar2.svg", 0, 43303, CRC(fc9e757d) SHA1(c52e2f7c01e174286b75efd0c4c137f42c46053f) )
ROM_END

} // anonymous namespace



/*******************************************************************************
    Drivers
*******************************************************************************/

//    YEAR  NAME   PARENT  COMPAT  MACHINE  INPUT  CLASS        INIT        COMPANY, FULLNAME, FLAGS
SYST( 1979, omar1, 0,      0,      omar1,   omar,  omar_state,  empty_init, "Tryom", "Omar I", MACHINE_SUPPORTS_SAVE )
SYST( 1979, omar2, 0,      0,      omar2,   omar,  omar2_state, empty_init, "Tryom", "Omar II", MACHINE_SUPPORTS_SAVE )
