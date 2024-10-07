// license:BSD-3-Clause
// copyright-holders:hap
// thanks-to:Sean Riddle
/*******************************************************************************

Fidelity Bridge Bidder (BB / 7004)

It's related to the Bridge Challenger series, but has no card reader, nor does
it play Bridge. It's more like a (cheat) tool for helping with bidding.

The 101-64109 ROM is identical to the one in Advanced Bridge Challenger.

Hardware notes:
- PCB label: main PCB: 510-1012, display PCB: 510-1013
- Zilog Z80-CPU-PS @ ~1MHz (R/C clock, no XTAL)
- Zilog Z80 PIO (custom label)
- 16KB ROM (2*NEC 2364C), 1KB RAM (2*TC5514P)
- 3*Litronix DL1414, 4 more leds, piezo

*******************************************************************************/

#include "emu.h"

#include "cpu/z80/z80.h"
#include "machine/z80pio.h"
#include "sound/dac.h"
#include "video/dl1416.h"
#include "video/pwm.h"

#include "speaker.h"

// internal artwork
#include "fidel_bridgeb.lh"


namespace {

class bridgeb_state : public driver_device
{
public:
	bridgeb_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_z80pio(*this, "z80pio"),
		m_dl1414(*this, "dl1414_%u", 0),
		m_led_pwm(*this, "led_pwm"),
		m_dac(*this, "dac"),
		m_inputs(*this, "IN.%u", 0),
		m_digits(*this, "digit%u", 0U)
	{ }

	void bridgeb(machine_config &config);

	DECLARE_INPUT_CHANGED_MEMBER(reset_button);
	DECLARE_INPUT_CHANGED_MEMBER(input_changed) { update_pa(); }

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	required_device<z80_device> m_maincpu;
	required_device<z80pio_device> m_z80pio;
	required_device_array<dl1414_device, 3> m_dl1414;
	required_device<pwm_display_device> m_led_pwm;
	required_device<dac_1bit_device> m_dac;
	required_ioport_array<8> m_inputs;
	output_finder<12> m_digits;

	u8 m_inp_mux = 0;

	void main_map(address_map &map) ATTR_COLD;
	void main_io(address_map &map) ATTR_COLD;

	template <int N> void update_digits(offs_t offset, u16 data);

	void digit_w(offs_t offset, u8 data);
	void control_w(u8 data);
	u8 input_r();
	void input_w(u8 data);
	void update_pa();
};

void bridgeb_state::machine_start()
{
	m_digits.resolve();

	// register for savestates
	save_item(NAME(m_inp_mux));
}

INPUT_CHANGED_MEMBER(bridgeb_state::reset_button)
{
	// reset button is directly wired to Z80 RESET pin
	m_maincpu->set_input_line(INPUT_LINE_RESET, newval ? ASSERT_LINE : CLEAR_LINE);
}



/*******************************************************************************
    I/O
*******************************************************************************/

// DL1414

template <int N>
void bridgeb_state::update_digits(offs_t offset, u16 data)
{
	m_digits[N * 4 + offset] = data;
}

void bridgeb_state::digit_w(offs_t offset, u8 data)
{
	// write to one DL1414 digit
	m_dl1414[(offset >> 13) % 3]->bus_w(offset >> 11 & 3, data);
}


// Z80 PIO

void bridgeb_state::control_w(u8 data)
{
	// A4,A5: led data
	// A6: N/C
	m_led_pwm->matrix(1, 1 << (data >> 4 & 3));

	// A7: speaker out
	m_dac->write(BIT(data, 7));
}

u8 bridgeb_state::input_r()
{
	u8 data = 0;

	// A0-A3: multiplexed inputs
	for (int i = 0; i < 8; i++)
		if (BIT(m_inp_mux, i))
			data |= m_inputs[i]->read() & 0xf;

	return ~data;
}

void bridgeb_state::update_pa()
{
	// push inputs to port A (can trigger IRQ)
	m_z80pio->port_a_write(input_r());
}

void bridgeb_state::input_w(u8 data)
{
	// B0-B7: input mux
	m_inp_mux = ~data;
	update_pa();
}



/*******************************************************************************
    Address Maps
*******************************************************************************/

void bridgeb_state::main_map(address_map &map)
{
	map(0x0000, 0x3fff).rom();
	map(0x6000, 0x63ff).mirror(0x1c00).ram();
	map(0xa000, 0xffff).w(FUNC(bridgeb_state::digit_w));
}

void bridgeb_state::main_io(address_map &map)
{
	map.global_mask(0x03);
	map(0x00, 0x03).rw(m_z80pio, FUNC(z80pio_device::read), FUNC(z80pio_device::write));
}



/*******************************************************************************
    Input Ports
*******************************************************************************/

#define PORT_CHANGED_CB(x) \
	PORT_CHANGED_MEMBER(DEVICE_SELF, bridgeb_state, x, 0)

static INPUT_PORTS_START( bridgeb )
	PORT_START("IN.0")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CHANGED_CB(input_changed) PORT_CODE(KEYCODE_I) PORT_CODE(KEYCODE_PLUS_PAD) PORT_NAME("A")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CHANGED_CB(input_changed) PORT_CODE(KEYCODE_0) PORT_CODE(KEYCODE_0_PAD) PORT_NAME("10")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CHANGED_CB(input_changed) PORT_CODE(KEYCODE_6) PORT_CODE(KEYCODE_6_PAD) PORT_NAME("6")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CHANGED_CB(input_changed) PORT_CODE(KEYCODE_2) PORT_CODE(KEYCODE_2_PAD) PORT_NAME("2")

	PORT_START("IN.1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CHANGED_CB(input_changed) PORT_CODE(KEYCODE_U) PORT_CODE(KEYCODE_MINUS_PAD) PORT_NAME("K")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CHANGED_CB(input_changed) PORT_CODE(KEYCODE_9) PORT_CODE(KEYCODE_9_PAD) PORT_NAME("9")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CHANGED_CB(input_changed) PORT_CODE(KEYCODE_5) PORT_CODE(KEYCODE_5_PAD) PORT_NAME("5")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CHANGED_CB(input_changed) PORT_CODE(KEYCODE_1) PORT_CODE(KEYCODE_1_PAD) PORT_NAME("1")

	PORT_START("IN.2")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CHANGED_CB(input_changed) PORT_CODE(KEYCODE_Y) PORT_CODE(KEYCODE_ASTERISK) PORT_NAME("Q")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CHANGED_CB(input_changed) PORT_CODE(KEYCODE_8) PORT_CODE(KEYCODE_8_PAD) PORT_NAME("8")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CHANGED_CB(input_changed) PORT_CODE(KEYCODE_4) PORT_CODE(KEYCODE_4_PAD) PORT_NAME("4")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CHANGED_CB(input_changed) PORT_CODE(KEYCODE_SLASH) PORT_NAME("P")

	PORT_START("IN.3")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CHANGED_CB(input_changed) PORT_CODE(KEYCODE_T) PORT_CODE(KEYCODE_SLASH_PAD) PORT_NAME("J")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CHANGED_CB(input_changed) PORT_CODE(KEYCODE_7) PORT_CODE(KEYCODE_7_PAD) PORT_NAME("7")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CHANGED_CB(input_changed) PORT_CODE(KEYCODE_3) PORT_CODE(KEYCODE_3_PAD) PORT_NAME("3")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CHANGED_CB(input_changed) PORT_CODE(KEYCODE_STOP) PORT_NAME("NT")

	PORT_START("IN.4")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CHANGED_CB(input_changed) PORT_CODE(KEYCODE_R) PORT_CODE(KEYCODE_ENTER) PORT_CODE(KEYCODE_ENTER_PAD) PORT_NAME("EN")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CHANGED_CB(input_changed) PORT_CODE(KEYCODE_F) PORT_NAME("HD")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CHANGED_CB(input_changed) PORT_CODE(KEYCODE_V) PORT_NAME("IN")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CHANGED_CB(input_changed) PORT_CODE(KEYCODE_COMMA) PORT_NAME("Spades")

	PORT_START("IN.5")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CHANGED_CB(input_changed) PORT_CODE(KEYCODE_E) PORT_CODE(KEYCODE_DEL) PORT_CODE(KEYCODE_BACKSPACE) PORT_NAME("CL")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CHANGED_CB(input_changed) PORT_CODE(KEYCODE_D) PORT_NAME("DB")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CHANGED_CB(input_changed) PORT_CODE(KEYCODE_C) PORT_NAME("VL")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CHANGED_CB(input_changed) PORT_CODE(KEYCODE_M) PORT_NAME("Hearts")

	PORT_START("IN.6")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CHANGED_CB(input_changed) PORT_CODE(KEYCODE_W) PORT_CODE(KEYCODE_SPACE) PORT_NAME("Speaker")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CHANGED_CB(input_changed) PORT_CODE(KEYCODE_S) PORT_NAME("PR")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CHANGED_CB(input_changed) PORT_CODE(KEYCODE_X) PORT_NAME("CV")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CHANGED_CB(input_changed) PORT_CODE(KEYCODE_N) PORT_NAME("Diamonds")

	PORT_START("IN.7")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CHANGED_CB(input_changed) PORT_CODE(KEYCODE_A) PORT_NAME("BR")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CHANGED_CB(input_changed) PORT_CODE(KEYCODE_Z) PORT_NAME("DL")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CHANGED_CB(input_changed) PORT_CODE(KEYCODE_B) PORT_NAME("Clubs")

	PORT_START("RESET") // is not on matrix IN.7 d0
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CHANGED_CB(reset_button) PORT_CODE(KEYCODE_Q) PORT_NAME("RE")
INPUT_PORTS_END



/*******************************************************************************
    Machine Configs
*******************************************************************************/

static const z80_daisy_config daisy_chain[] =
{
	{ "z80pio" },
	{ nullptr }
};

void bridgeb_state::bridgeb(machine_config &config)
{
	// basic machine hardware
	Z80(config, m_maincpu, 1'000'000); // R/C clock, appromixation
	m_maincpu->set_addrmap(AS_PROGRAM, &bridgeb_state::main_map);
	m_maincpu->set_addrmap(AS_IO, &bridgeb_state::main_io);
	m_maincpu->set_daisy_config(daisy_chain);

	Z80PIO(config, m_z80pio, 1'000'000);
	m_z80pio->out_int_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);
	m_z80pio->out_pa_callback().set(FUNC(bridgeb_state::control_w));
	m_z80pio->in_pa_callback().set(FUNC(bridgeb_state::input_r));
	m_z80pio->out_pb_callback().set(FUNC(bridgeb_state::input_w));

	// video hardware
	DL1414T(config, m_dl1414[0], 0U).update().set(FUNC(bridgeb_state::update_digits<0>));
	DL1414T(config, m_dl1414[1], 0U).update().set(FUNC(bridgeb_state::update_digits<1>));
	DL1414T(config, m_dl1414[2], 0U).update().set(FUNC(bridgeb_state::update_digits<2>));

	PWM_DISPLAY(config, m_led_pwm).set_size(1, 4);
	config.set_default_layout(layout_fidel_bridgeb);

	// sound hardware
	SPEAKER(config, "speaker").front_center();
	DAC_1BIT(config, m_dac).add_route(ALL_OUTPUTS, "speaker", 0.25);
}



/*******************************************************************************
    ROM Definitions
*******************************************************************************/

ROM_START( bridgeb )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("101-64111", 0x0000, 0x2000, CRC(903d0c3c) SHA1(8360bc21acfd0115730242fa1aa62356f289a88d) ) // NEC 2364C 218
	ROM_LOAD("101-64109", 0x2000, 0x2000, CRC(320afa0f) SHA1(90edfe0ac19b108d232cda376b03a3a24befad4c) ) // NEC 2364C 211
ROM_END

} // anonymous namespace



/*******************************************************************************
    Drivers
*******************************************************************************/

//    YEAR  NAME     PARENT  COMPAT  MACHINE  INPUT    CLASS          INIT        COMPANY, FULLNAME, FLAGS
SYST( 1980, bridgeb, 0,      0,      bridgeb, bridgeb, bridgeb_state, empty_init, "Fidelity Electronics", "Bridge Bidder", MACHINE_SUPPORTS_SAVE )
