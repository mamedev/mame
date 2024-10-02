// license:BSD-3-Clause
// copyright-holders:hap
// thanks-to:Berger
/*******************************************************************************

Novag Micro II (model 821)

This program was used in several Novag chesscomputers:
- Novag Micro II (1st use)
- Novag Micro III
- Novag Presto
- Novag Octo
- Novag Allegro (not Allegro 4)
- Novag Piccolo

The chess engine is by Julio Kaplan, not David Kittinger.

Hardware notes:

Micro II, Micro III(same pcb):
- Mitsubishi M5L8049-079P-6, 6MHz XTAL
- buzzer, 20 leds, 8*8 chessboard buttons

Presto:
- PCB label: 100023, 100024
- NEC D80C49C MCU(serial 186), OSC from LC circuit measured ~6MHz

Octo (high-speed):
- same PCB as Presto
- NEC D80C49HC MCU(serial 111), 12MHz OSC from LC circuit, this was advertised
  as 15MHz on the box, but measured ~12MHz (older Octo version is probably ~6MHz?)
- speaker circuit is a bit different, not sure why

Piccolo (high-speed):
- PCB label: NOVAG 100041
- same MCU serial as Octo, LC OSC is around 14MHz

Note that even though the MCUs are different, internal ROM contents was confirmed
to be identical for Micro II/III, Presto, Octo, Piccolo.

BTANB:
- controls are very sensitive (board sensors too): 6MHz: valid (single) button
  press registered between 307ms and 436ms, 15MHz: between 123ms and 174ms

*******************************************************************************/

#include "emu.h"

#include "cpu/mcs48/mcs48.h"
#include "machine/sensorboard.h"
#include "sound/dac.h"
#include "video/pwm.h"

#include "speaker.h"

// internal artwork
#include "novag_micro2.lh"


namespace {

class micro2_state : public driver_device
{
public:
	micro2_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_display(*this, "display"),
		m_board(*this, "board"),
		m_dac(*this, "dac"),
		m_inputs(*this, "IN.0")
	{ }

	// machine configs
	void micro2(machine_config &config);

	DECLARE_INPUT_CHANGED_MEMBER(change_cpu_freq) { set_cpu_freq(); }

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override { set_cpu_freq(); }

private:
	// devices/pointers
	required_device<mcs48_cpu_device> m_maincpu;
	required_device<pwm_display_device> m_display;
	required_device<sensorboard_device> m_board;
	required_device<dac_1bit_device> m_dac;
	required_ioport m_inputs;

	bool m_kp_select = false;
	u8 m_inp_mux = 0;

	// I/O handlers
	void mux_w(u8 data);
	void control_w(u8 data);
	u8 input_r();

	void set_cpu_freq();
};



/*******************************************************************************
    Initialization
*******************************************************************************/

void micro2_state::machine_start()
{
	// register for savestates
	save_item(NAME(m_kp_select));
	save_item(NAME(m_inp_mux));
}

void micro2_state::set_cpu_freq()
{
	// known CPU speeds: 6MHz(XTAL), 6MHz(LC), ~15MHz(may vary, LC)
	u32 freq = (ioport("CPU")->read() & 1) ? 15'000'000 : 6'000'000;
	m_maincpu->set_unscaled_clock(freq);

	m_board->set_delay(attotime::from_ticks(2'000'000, freq)); // see BTANB
}



/*******************************************************************************
    I/O
*******************************************************************************/

void micro2_state::mux_w(u8 data)
{
	// D0-D7: input mux, led data
	m_inp_mux = ~data;
	m_display->write_mx(m_inp_mux);
}

void micro2_state::control_w(u8 data)
{
	// P21: keypad select
	m_kp_select = bool(~data & 2);

	// P22,P23: speaker lead 1,2
	m_dac->write(BIT(data, 2) & BIT(~data, 3));

	// P24-P26: led select
	m_display->write_my(~data >> 4 & 7);
}

u8 micro2_state::input_r()
{
	// P10-P17: multiplexed inputs
	u8 data = 0;

	// read chessboard buttons
	for (int i = 0; i < 8; i++)
		if (BIT(m_inp_mux, i))
			data |= m_board->read_rank(i);

	// read sidepanel keypad
	if (m_kp_select)
		data |= m_inputs->read();

	return ~data;
}



/*******************************************************************************
    Input Ports
*******************************************************************************/

static INPUT_PORTS_START( micro2 )
	PORT_START("IN.0")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_1) PORT_CODE(KEYCODE_1_PAD) PORT_CODE(KEYCODE_B) PORT_NAME("B/W") // aka "Black/White" or "Change Color"
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_2) PORT_CODE(KEYCODE_2_PAD) PORT_CODE(KEYCODE_V) PORT_NAME("Verify / Pawn")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_3) PORT_CODE(KEYCODE_3_PAD) PORT_CODE(KEYCODE_U) PORT_NAME("Set Up / Rook")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_4) PORT_CODE(KEYCODE_4_PAD) PORT_NAME("Knight")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_5) PORT_CODE(KEYCODE_5_PAD) PORT_CODE(KEYCODE_L) PORT_NAME("Level / Bishop")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_6) PORT_CODE(KEYCODE_6_PAD) PORT_NAME("Queen")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_7) PORT_CODE(KEYCODE_7_PAD) PORT_CODE(KEYCODE_T) PORT_NAME("Take Back / King")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_8) PORT_CODE(KEYCODE_8_PAD) PORT_CODE(KEYCODE_G) PORT_NAME("Go")

	PORT_START("CPU")
	PORT_CONFNAME( 0x01, 0x00, "CPU Frequency" ) PORT_CHANGED_MEMBER(DEVICE_SELF, micro2_state, change_cpu_freq, 0) // factory set
	PORT_CONFSETTING(    0x00, "6MHz (original)" )
	PORT_CONFSETTING(    0x01, "15MHz (newer)" )
INPUT_PORTS_END



/*******************************************************************************
    Machine Configs
*******************************************************************************/

void micro2_state::micro2(machine_config &config)
{
	// basic machine hardware
	I8049(config, m_maincpu, 6_MHz_XTAL); // see set_cpu_freq
	m_maincpu->p1_in_cb().set(FUNC(micro2_state::input_r));
	m_maincpu->p2_out_cb().set(FUNC(micro2_state::control_w));
	m_maincpu->bus_out_cb().set(FUNC(micro2_state::mux_w));

	SENSORBOARD(config, m_board).set_type(sensorboard_device::BUTTONS);
	m_board->init_cb().set(m_board, FUNC(sensorboard_device::preset_chess));

	// video hardware
	PWM_DISPLAY(config, m_display).set_size(3, 8);
	config.set_default_layout(layout_novag_micro2);

	// sound hardware
	SPEAKER(config, "speaker").front_center();
	DAC_1BIT(config, m_dac).add_route(ALL_OUTPUTS, "speaker", 0.25);
}



/*******************************************************************************
    ROM Definitions
*******************************************************************************/

ROM_START( nmicro2 )
	ROM_REGION( 0x0800, "maincpu", 0 )
	ROM_LOAD("8049_8210.u1", 0x0000, 0x0800, CRC(29a0eb4c) SHA1(e058d6018e53ddcaa3b5ec25b33b8bff091b04db) )
ROM_END

} // anonymous namespace



/*******************************************************************************
    Drivers
*******************************************************************************/

//    YEAR  NAME     PARENT   COMPAT  MACHINE  INPUT   CLASS         INIT        COMPANY, FULLNAME, FLAGS
SYST( 1982, nmicro2, 0,       0,      micro2,  micro2, micro2_state, empty_init, "Novag Industries / Heuristic Software", "Micro II (Novag)", MACHINE_SUPPORTS_SAVE )
