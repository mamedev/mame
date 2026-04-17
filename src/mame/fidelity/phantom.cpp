// license:BSD-3-Clause
// copyright-holders:hap, Sandro Ronco
// thanks-to:Berger
/*******************************************************************************

Fidelity Phantom (model 6100)

Fidelity licensed (or perhaps bought) the design of Milton Bradley's Grand Master
motorized chessboard and released their own version. It has a small LCD panel added,
the rest looks nearly the same from the outside. After Fidelity was taken over by H+G,
it was rereleased in 1990 as the Mephisto Phantom. This is assumed to be identical.

At boot-up, the computer will do a self-test, the user can start playing after the
motor has moved to the upper-right corner. The computer will continue positioning
pieces though, so it may be a bit distracting. Or, just fast forward MAME for a while
(hold INS key on Windows) before starting a new game.

After the user captures a piece, select the captured piece from the MAME sensorboard
spawn block and place it anywhere on a free spot at the designated box at the edge
of the chessboard.

Hardware notes:
- PCB label: 510.1128A01
- R65C02P4, XTAL marked 4.915200
- 2*32KB ROM 27C256-15, 8KB RAM MS6264L-10
- LCD driver, display panel for digits
- 2 DC motors under chessboard, electromagnet for automatically moving chess pieces
- piezo speaker, 9 LEDs, 12*8 chessboard buttons

Chesster Phantom is on the same base hardware, and adds the Chesster voice to it,
using the same ROM as the original Chesster. Model 6124 extra hardware is on a
daughterboard, the housing is the same as model 6100, except for button labels.
Model 6126 has a dedicated PCB, this version also has a motion sensor at the front
and 2 leds to mimic eyes, and the housing color theme is green instead of beige.

TODO:
- verify motor comms, fphantom has motor drift errors that don't appear to happen
  on the real thing (auto corrected by gmboard_device, so it works fine)

BTANB:
- cphantom: As the manual suggests, the computer's move should be displayed on the
  LCD while it's moving the piece just like in fphantom, but this is often too brief
  or not displayed at all. (This may seem like a minor bug in the game, but it
  actually makes it more difficult to write a MAME UCI plugin for this driver.)

*******************************************************************************/

#include "emu.h"

#include "cpu/m6502/r65c02.h"
#include "machine/7474.h"
#include "machine/clock.h"
#include "machine/gmboard.h"
#include "machine/input_merger.h"
#include "sound/dac.h"
#include "video/pwm.h"

#include "speaker.h"

// internal artwork
#include "fidel_cphantom.lh"
#include "fidel_phantom.lh"


namespace {

// Phantom 6100 / shared

class phantom_state : public driver_device
{
public:
	phantom_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_irq_ff(*this, "irq_ff%u", 0),
		m_rombank(*this, "rombank"),
		m_dac(*this, "dac"),
		m_board(*this, "board"),
		m_display(*this, "display"),
		m_inputs(*this, "IN.%u", 0)
	{ }

	void phantom(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	// devices/pointers
	required_device<cpu_device> m_maincpu;
	optional_device_array<ttl7474_device, 4> m_irq_ff;
	required_memory_bank m_rombank;
	optional_device<dac_1bit_device> m_dac;
	required_device<gmboard_device> m_board;
	required_device<pwm_display_device> m_display;
	optional_ioport_array<2> m_inputs;

	u8 m_mux = 0;
	u8 m_select = 0;
	u32 m_lcd_data = 0;
	bool m_stop[2] = { };

	// address maps
	virtual void main_map(address_map &map) ATTR_COLD;

	// I/O handlers
	void update_lcd(u8 select);
	virtual void control_w(offs_t offset, u8 data);
	void lcd_w(offs_t offset, u8 data);
	void motors_w(u8 data);
	virtual u8 input_r(offs_t offset);
	u8 motors_r(offs_t offset);
	template<int N> u8 irq_clear_r();
	template<int N> void motor_stop_w(int state);
};

void phantom_state::machine_start()
{
	int numbanks = memregion("rombank")->bytes() / 0x4000;
	m_rombank->configure_entries(0, numbanks, memregion("rombank")->base(), 0x4000);

	// register for savestates
	save_item(NAME(m_mux));
	save_item(NAME(m_select));
	save_item(NAME(m_lcd_data));
	save_item(NAME(m_stop));
}

void phantom_state::machine_reset()
{
	m_rombank->set_entry(0);

	// make sure 7474 is not in an indeterminate state
	irq_clear_r<0>();
	irq_clear_r<1>();
	irq_clear_r<2>();
}


// Chesster Phantom

class cphantom_state : public phantom_state
{
public:
	cphantom_state(const machine_config &mconfig, device_type type, const char *tag) :
		phantom_state(mconfig, type, tag),
		m_speech(*this, "speech"),
		m_eye_led(*this, "eye_led")
	{ }

	void cphantom(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	required_device<dac_8bit_r2r_device> m_speech;
	output_finder<> m_eye_led;

	virtual void main_map(address_map &map) override ATTR_COLD;

	virtual void control_w(offs_t offset, u8 data) override;
	virtual u8 input_r(offs_t offset) override;

	u8 m_select2 = 0;
};

void cphantom_state::machine_start()
{
	phantom_state::machine_start();

	m_eye_led.resolve();
	save_item(NAME(m_select2));
}

void cphantom_state::machine_reset()
{
	phantom_state::machine_reset();
	m_speech->write(0x80);
}



/*******************************************************************************
    I/O
*******************************************************************************/

void phantom_state::update_lcd(u8 select)
{
	// update lcd at any edge
	if ((select ^ m_select) & 0x80)
	{
		u8 mask = (m_select & 0x80) ? 0xff : 0;
		for (int i = 0; i < 4; i++)
			m_display->write_row(i+1, (m_lcd_data >> (8*i) & 0xff) ^ mask);
	}
}

void phantom_state::control_w(offs_t offset, u8 data)
{
	// a0-a2,d1: 74259
	u8 prev = m_select;
	u8 mask = 1 << offset;
	m_select = (m_select & ~mask) | ((data & 0x02) ? mask : 0);

	// 74259 Q0-Q3: 7442 a0-a3
	// 7442 0-8: led data, input mux
	// 74259 Q4: led select
	m_mux = m_select & 0xf;
	m_display->matrix_partial(0, 1, BIT(~m_select, 4), 1 << m_mux);

	// 74259 Q6: bookrom bank
	m_rombank->set_entry(BIT(m_select, 6));

	// 74259 Q7: lcd polarity
	update_lcd(prev);
}

void cphantom_state::control_w(offs_t offset, u8 data)
{
	// chesster version has two 74259, more I/O
	u8 lcd_prev = m_select;

	// a0-a2,d0,d1: 2*74259
	u8 mask = 1 << offset;
	m_select = (m_select & ~mask) | ((data & 1) ? mask : 0);
	m_select2 = (m_select2 & ~mask) | ((data & 2) ? mask : 0);

	// 74259(both) Q0,Q1: 7442 a0-a3
	// 7442 0-8: led data, input mux
	// 74259(1) Q2: led select
	m_mux = BIT(m_select, 0) | BIT(m_select2, 0) << 1 | BIT(m_select, 1) << 2 | BIT(m_select2, 1) << 3;
	m_display->matrix_partial(0, 1, BIT(~m_select, 2), 1 << m_mux);

	// 74259(2) Q2: eye leds
	m_eye_led = BIT(~m_select2, 2);

	// 74259(2) Q3: nmi F/F clear
	m_irq_ff[3]->clear_w(BIT(m_select2, 3));

	// 74259(1) Q4,Q5, 74259(2) Q4: speechrom bank
	m_rombank->set_entry(BIT(m_select, 4) | (BIT(m_select2, 4) << 1) | (BIT(m_select, 5) << 2));

	// 74259(1) Q7: lcd polarity
	update_lcd(lcd_prev);
}

void phantom_state::motors_w(u8 data)
{
	// d0: vertical motor down
	// d1: vertical motor up
	if (!m_stop[1])
		m_board->motor_w(1, bitswap<2>(data, 0, 1));

	// d2: horizontal motor left
	// d3: horizontal motor right
	if (!m_stop[0])
		m_board->motor_w(0, data >> 2 & 3);

	// d4: electromagnet
	m_board->magnet_w(BIT(data, 4));

	// d5: speaker (not for chesster version, though it still writes to it)
	if (m_dac != nullptr)
		m_dac->write(BIT(data, 5));
}

void phantom_state::lcd_w(offs_t offset, u8 data)
{
	// a0-a2,d0,d2,d4,d6: 4*74259 to lcd digit segments
	u32 mask = bitswap<8>(1 << offset,3,7,6,0,1,2,4,5);
	for (int i = 0; i < 4; i++)
	{
		m_lcd_data = (m_lcd_data & ~mask) | (BIT(data, i * 2) ? mask : 0);
		mask <<= 8;
	}
}

u8 phantom_state::input_r(offs_t offset)
{
	u8 data = 0xff;

	// buttons
	if (m_mux == 8)
	{
		data ^= BIT(m_inputs[0]->read(), offset * 2 + 1) << 6;
		data ^= BIT(m_inputs[0]->read(), offset * 2 + 0) << 7;
	}

	// chessboard sensors
	else if (offset < 4)
	{
		data ^= BIT(m_board->read_file(offset * 2 + 1), m_mux) << 6;
		data ^= BIT(m_board->read_file(offset * 2 + 0), m_mux) << 7;
	}

	// captured pieces
	else
	{
		data ^= BIT(m_board->read_file( 8 + (offset & 1)), m_mux) << 6; // black
		data ^= BIT(m_board->read_file(11 - (offset & 1)), m_mux) << 7; // white
	}

	return data;
}

u8 cphantom_state::input_r(offs_t offset)
{
	u8 data = phantom_state::input_r(offset) & 0xfe;

	// d0: motion sensor (simulated here with an arbitrary timer)
	int motion = ((machine().time().as_ticks(50) & 0xff) > 0) ? 1 : 0;
	return data | motion | (m_inputs[1]->read() & 1);
}

u8 phantom_state::motors_r(offs_t offset)
{
	u8 data = 0x3f;

	// IRQ F/F output
	if (offset)
	{
		data |= m_irq_ff[1]->output_r() << 6;
		data |= m_irq_ff[0]->output_r() << 7;
	}

	// motor quadrature state
	else
	{
		data |= BIT(m_board->quad_r(1), 1) << 6;
		data |= BIT(m_board->quad_r(0), 1) << 7;
	}

	return data;
}

template<int N>
void phantom_state::motor_stop_w(int state)
{
	if (state)
	{
		if (N < 2)
		{
			// cphantom expects motors to turn off
			m_stop[N] = true;
			m_board->motor_w(N, 0);
		}
		else
			m_stop[0] = m_stop[1] = false;
	}
}

template<int N>
u8 phantom_state::irq_clear_r()
{
	if (!machine().side_effects_disabled())
	{
		m_irq_ff[N]->clear_w(0);
		m_irq_ff[N]->clear_w(1);
	}

	return 0xff;
}



/*******************************************************************************
    Address Maps
*******************************************************************************/

void phantom_state::main_map(address_map &map)
{
	map(0x0000, 0x1fff).ram();
	map(0x2000, 0x2007).mirror(0x00f8).w(FUNC(phantom_state::control_w));
	map(0x2100, 0x2107).mirror(0x00f8).w(FUNC(phantom_state::lcd_w)).nopr();
	map(0x2200, 0x2200).mirror(0x00ff).w(FUNC(phantom_state::motors_w));
	map(0x2400, 0x2405).mirror(0x00f8).r(FUNC(phantom_state::input_r));
	map(0x2406, 0x2407).mirror(0x00f8).r(FUNC(phantom_state::motors_r));
	map(0x2500, 0x2500).mirror(0x00ff).r(FUNC(phantom_state::irq_clear_r<0>));
	map(0x2600, 0x2600).mirror(0x00ff).r(FUNC(phantom_state::irq_clear_r<1>));
	map(0x2700, 0x2700).mirror(0x00ff).r(FUNC(phantom_state::irq_clear_r<2>));
	map(0x4000, 0x7fff).bankr(m_rombank);
	map(0x8000, 0xffff).rom();
}

void cphantom_state::main_map(address_map &map)
{
	phantom_state::main_map(map);
	map(0x2300, 0x2300).mirror(0x00ff).w(m_speech, FUNC(dac_8bit_r2r_device::data_w));
}



/*******************************************************************************
    Input Ports
*******************************************************************************/

static INPUT_PORTS_START( phantom )
	PORT_START("IN.0")
	PORT_BIT(0x001, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_V) PORT_NAME("Verify / Problem")
	PORT_BIT(0x002, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_O) PORT_NAME("Option / Time")
	PORT_BIT(0x004, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_L) PORT_NAME("Level / New")
	PORT_BIT(0x008, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_T) PORT_NAME("Take Back / Replay")
	PORT_BIT(0x010, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_H) PORT_NAME("Hint / Info")
	PORT_BIT(0x020, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_M) PORT_NAME("Move / Alternate")
	PORT_BIT(0x040, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_A) PORT_NAME("Auto / Stop")
	PORT_BIT(0x080, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_C) PORT_NAME("Clear")
	PORT_BIT(0x100, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_S) PORT_NAME("Shift")
	PORT_BIT(0x200, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x400, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x800, IP_ACTIVE_HIGH, IPT_UNUSED)
INPUT_PORTS_END

static INPUT_PORTS_START( cphantom )
	PORT_INCLUDE( phantom )

	PORT_MODIFY("IN.0")
	PORT_BIT(0x002, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_O) PORT_NAME("Option / Replay")
	PORT_BIT(0x008, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_I) PORT_NAME("Info / Auto")
	PORT_BIT(0x010, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_T) PORT_NAME("Take Back / Repeat")
	PORT_BIT(0x020, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_H) PORT_NAME("Hint / Yes")
	PORT_BIT(0x040, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_M) PORT_NAME("Move / No/Stop")

	PORT_START("IN.1") // motion sensor is inverted here, eg. hold down key to pretend that nobody's there
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_OTHER) PORT_CODE(KEYCODE_F1) PORT_NAME("Motion Sensor")
INPUT_PORTS_END



/*******************************************************************************
    Machine Configs
*******************************************************************************/

void phantom_state::phantom(machine_config &config)
{
	// basic machine hardware
	R65C02(config, m_maincpu, 4.9152_MHz_XTAL); // R65C02P4 or RP65C02G
	m_maincpu->set_addrmap(AS_PROGRAM, &phantom_state::main_map);

	INPUT_MERGER_ANY_HIGH(config, "mainirq").output_handler().set_inputline(m_maincpu, R65C02_IRQ_LINE);

	TTL7474(config, m_irq_ff[0]).output_cb().set("mainirq", FUNC(input_merger_device::in_w<0>));
	TTL7474(config, m_irq_ff[1]).output_cb().set("mainirq", FUNC(input_merger_device::in_w<1>));
	TTL7474(config, m_irq_ff[2]).output_cb().set("mainirq", FUNC(input_merger_device::in_w<2>));

	m_irq_ff[0]->output_cb().append(FUNC(phantom_state::motor_stop_w<0>));
	m_irq_ff[1]->output_cb().append(FUNC(phantom_state::motor_stop_w<1>));
	m_irq_ff[2]->output_cb().append(FUNC(phantom_state::motor_stop_w<2>));

	auto &irq_clock(CLOCK(config, "irq_clock"));
	irq_clock.set_period(attotime::from_ticks(0x2000, 4.9152_MHz_XTAL)); // 4060, 600Hz
	irq_clock.signal_handler().set(m_irq_ff[2], FUNC(ttl7474_device::clock_w));

	MB_GMBOARD(config, m_board);
	m_board->set_size(750, 575, 64);
	m_board->set_offsets(10, 75);
	m_board->quad_cb<0>().set(m_irq_ff[0], FUNC(ttl7474_device::clock_w)).bit(0);
	m_board->quad_cb<1>().set(m_irq_ff[1], FUNC(ttl7474_device::clock_w)).bit(0);

	// video hardware
	PWM_DISPLAY(config, m_display).set_size(1+4, 9);
	m_display->set_segmask(0x1e, 0x7f);

	config.set_default_layout(layout_fidel_phantom);

	// sound hardware
	SPEAKER(config, "speaker").front_center();
	DAC_1BIT(config, m_dac).add_route(ALL_OUTPUTS, "speaker", 0.25);
}

void cphantom_state::cphantom(machine_config &config)
{
	phantom(config);

	// basic machine hardware
	TTL7474(config, m_irq_ff[3]).comp_output_cb().set_inputline(m_maincpu, R65C02_NMI_LINE).invert();

	auto &nmi_clock(CLOCK(config, "nmi_clock"));
	nmi_clock.set_period(attotime::from_ticks(0x200, 4.9152_MHz_XTAL)); // 4060, 9.6kHz
	nmi_clock.signal_handler().set(m_irq_ff[3], FUNC(ttl7474_device::clock_w));

	m_board->set_offsets(10, 70);

	config.set_default_layout(layout_fidel_cphantom);

	// sound hardware
	config.device_remove("dac");
	DAC_8BIT_R2R(config, m_speech).add_route(ALL_OUTPUTS, "speaker", 0.5);
}



/*******************************************************************************
    ROM Definitions
*******************************************************************************/

ROM_START( fphantom ) // model 6100, PCB label 510.1128A01
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("u_3c_yellow.u3", 0x8000, 0x8000, CRC(fb7c38ae) SHA1(a1aa7637705052cb4eec92644dc79aee7ba4d77c) ) // 27C256

	ROM_REGION( 0x8000, "rombank", 0 )
	ROM_LOAD("u_4_white.u4",  0x0000, 0x8000, CRC(e4181ba2) SHA1(1f77d1867c6f566be98645fc252a01108f412c96) ) // 27C256
ROM_END


ROM_START( cphantom ) // model 6126, PCB label 510.1128D01
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("pv.u3", 0x8000, 0x8000, CRC(450a9ab5) SHA1(8392c76cf18cd6f8b17c8b12fac40c5cea874941) ) // 27C256

	ROM_REGION( 0x20000, "rombank", 0 )
	ROM_LOAD("101-1091b02.u4", 0x0000, 0x20000, CRC(fa370e88) SHA1(a937c8f1ec295cf9539d12466993974e40771493) ) // AMI, 27C010 or equivalent
ROM_END

} // anonymous namespace



/*******************************************************************************
    Drivers
*******************************************************************************/

//    YEAR  NAME      PARENT  COMPAT  MACHINE   INPUT     CLASS           INIT        COMPANY, FULLNAME, FLAGS
SYST( 1988, fphantom, 0,      0,      phantom,  phantom,  phantom_state,  empty_init, "Fidelity International", "Phantom (Fidelity)", MACHINE_SUPPORTS_SAVE | MACHINE_MECHANICAL | MACHINE_IMPERFECT_CONTROLS )

SYST( 1991, cphantom, 0,      0,      cphantom, cphantom, cphantom_state, empty_init, "Fidelity Electronics International", "Chesster Phantom (model 6126)", MACHINE_SUPPORTS_SAVE | MACHINE_MECHANICAL | MACHINE_IMPERFECT_CONTROLS )
