// license:BSD-3-Clause
// copyright-holders:hap, Joakim Larsson Edstrom
/*******************************************************************************

Applied Concepts Destiny Prodigy

The chess engine is Morphy, which in turn is based on Sargon 2.5.

Not counting The Mate, a chess game with chessboard peripheral for the Apple II,
this is the only known Destiny series chesscomputer. ACI also announced Destiny
Laser Chess, but it was never released.

********************************************************************************

PCB notes:

  +-----------------------------------------------------------------------------------+
  |LEDS------------------------------------------+        +-----------------+         |
  | | |             o o o o o o o o COLCN        |        |                 |         |
  | V |                                          |        | 4 char BCD LED  |         |
  | O | 8                                        |        +-----------------+         |
  |   |                                    ROWCN |        |||||||||||||||||||  ____   |
  | O | 7                                       o|+----------------------+    /    \  |
  |   |                                         o||  VIA                 |   ( beep ) |
  | O | 6                                       o|| R6522                |    \____/  |
  |   |                                         o|+----------------------+            |
  | O | 5                                       o|+----------------------+            |
  |   |                                         o||  CPU                 |            |
  | O | 4        8 x 8 button matrix            o|| R6502-13             |   +--+  __ |
  |   |                                         o|+----------------------+   |74| PWR||
  | O | 3                                       o|   o    o  +-------------+ |LS| SW >|
  |   |                                          | +=======+ |  ROM        | |14|  __||
  | O | 2                                        | | 2MHz  | | R2912       | +--+     |
  |   |                                          | |  XTAL | +-------------+     +--+ |
  | O | 1                                        |++-------+ +-------------+     |74| |
  |   |                                          || 74145N | |  RAM        |     |LS| |
  |   |   A    B    C    D    E    F    G    H   |+--------+ | M58725P     |     |00| |
  |   +------------------------------------------+           +-------------+     +--+ |
  |LEDS-> O    O    O    O    O    O    O    O                OOOOOOOOOOOO KPDCN      |
  +-----------------------------------------------------------------------------------+

  Tracing the image shows that VIA Port A is used on the ROWCN and Port B on COLCN

  The VIA pins CB1 and CB2 together with PB0 and PB1 via the 74145 drives the BCD display.
  The BCD display is of common anode type and each anode a0-a3 selects which digit that
  will be lit selected by PB0 and PB1. The segments to be lit is selected by the 8 bits
  shifted into the 74164 by clock from VIA CB1 and data from CB2.

Behind the BCD display we find the following supporting circuit:

                                                                 4x7 segment BCD display
                                        +---+       +-----+          +---+---+---+---+
  +-----------------+            CB1    |74 |==/4/=>|2x   |==/8/====>| 0   1   2   3 |
  |                 |       VIA  CB2    |164|==/4/=>|75491| segments |               |
  | 4 char BCD LED  |  ===> 6522        +---+       +-----+          +---+---+---+---+
  +-----------------+            PB1--->|74 |                          |   |   |   |
  |||||||||||||||||||            PB2--->|145|=/4/=/R/=>b(4x    )c=/4/==============>
                                        +---+           (PN2907)e=+     anodes
                                                                  |+5v

The keypad is connected to the 12 pin KPDCN connector left to right KP1:

  Pin #: KP1 KP2 KP3 KP4 KP5 KP6 KP7 KP8 KP9 KP10 KP11 K12
  VIA  :     PB4 PB5 PA0 PA1 PA2 PA3 PA4 PA5 PA6       PA7
  74145:  Q8                                       Q9      - used to decode/ground one half of the KPAD at a time

*******************************************************************************/

#include "emu.h"

#include "cpu/m6502/m6502.h"
#include "machine/6522via.h"
#include "machine/sensorboard.h"
#include "video/pwm.h"
#include "sound/dac.h"

#include "speaker.h"

// internal artwork
#include "aci_prodigy.lh"


namespace {

class prodigy_state : public driver_device
{
public:
	prodigy_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_via(*this, "via"),
		m_board(*this, "board"),
		m_display(*this, "display"),
		m_dac(*this, "dac"),
		m_inputs(*this, "IN.%u", 0)
	{ }

	void prodigy(machine_config &config);

protected:
	virtual void machine_start() override;

private:
	// devices/pointers
	required_device<cpu_device> m_maincpu;
	required_device<via6522_device> m_via;
	required_device<sensorboard_device> m_board;
	required_device<pwm_display_device> m_display;
	required_device<dac_bit_interface> m_dac;
	required_ioport_array<2> m_inputs;

	u8 m_select = 0;
	u8 m_led_data = 0;
	u8 m_shift_data = 0;
	u8 m_shift_clock = 0;

	void main_map(address_map &map);

	// I/O handlers
	void update_display();
	u8 input1_r();
	u8 input2_r();
	void control_w(u8 data);

	void shift_clock_w(int state);
	void shift_data_w(int state);
};

void prodigy_state::machine_start()
{
	// register for savestates
	save_item(NAME(m_select));
	save_item(NAME(m_led_data));
	save_item(NAME(m_shift_data));
	save_item(NAME(m_shift_clock));
}



/*******************************************************************************
    I/O
*******************************************************************************/

// 6522 ports

void prodigy_state::update_display()
{
	// 4 7seg leds via 74145 Q0-Q3 + leds via Q4,Q5
	m_display->matrix(1 << m_select, m_led_data);
}

void prodigy_state::shift_clock_w(int state)
{
	// shift 8-bit led/digit data on rising edge
	if (state && !m_shift_clock)
	{
		m_led_data = m_led_data << 1 | (m_shift_data & 1);
		update_display();
	}

	m_shift_clock = state;
}

void prodigy_state::shift_data_w(int state)
{
	m_shift_data = state;
}

u8 prodigy_state::input1_r()
{
	u8 data = 0;

	// PA0-PA7: multiplexed inputs
	// read chessboard sensors via 74145 Q0-Q7
	if (m_select < 8)
		data = m_board->read_file(m_select);

	// read keypad(high) via 74145 Q8,Q9
	else if (m_select == 8 || m_select == 9)
		data = m_inputs[m_select - 8]->read() >> 2 & 0xff;

	return ~data;
}

u8 prodigy_state::input2_r()
{
	u8 data = 0;

	// PB4,PB5: keypad(low) via 74145 Q8,Q9
	if (m_select == 8 || m_select == 9)
		data = (m_inputs[m_select - 8]->read() & 3) << 4;

	// PB6: ?

	return ~data;
}

void prodigy_state::control_w(u8 data)
{
	// PB0-PB3: 74145
	m_select = data & 0xf;
	update_display();

	// PB7: speaker out
	m_dac->write(BIT(data, 7));
}



/*******************************************************************************
    Address Maps
*******************************************************************************/

void prodigy_state::main_map(address_map &map)
{
	map.global_mask(0x7fff);
	map(0x0000, 0x07ff).ram();
	map(0x2000, 0x200f).m(m_via, FUNC(via6522_device::map));
	map(0x6000, 0x7fff).rom();
}



/*******************************************************************************
    Input Ports
*******************************************************************************/

static INPUT_PORTS_START( prodigy )
	PORT_START("IN.0")
	PORT_BIT(0x001, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_O) PORT_NAME("Go")
	PORT_BIT(0x002, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_A) PORT_CODE(KEYCODE_1) PORT_CODE(KEYCODE_1_PAD) PORT_NAME("A 1 / Pawn")
	PORT_BIT(0x004, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_D) PORT_CODE(KEYCODE_4) PORT_CODE(KEYCODE_4_PAD) PORT_NAME("D 4 / Rook")
	PORT_BIT(0x008, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_G) PORT_CODE(KEYCODE_7) PORT_CODE(KEYCODE_7_PAD) PORT_NAME("G 7")
	PORT_BIT(0x010, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_R) PORT_NAME("Restore")
	PORT_BIT(0x020, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_DEL) PORT_CODE(KEYCODE_BACKSPACE) PORT_NAME("CE") // clear entry
	PORT_BIT(0x040, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_L) PORT_NAME("Level")
	PORT_BIT(0x080, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_X) PORT_NAME("Change Board")
	PORT_BIT(0x100, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_V) PORT_NAME("Verify")
	PORT_BIT(0x200, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_ENTER) PORT_CODE(KEYCODE_ENTER_PAD) PORT_NAME("Enter")

	PORT_START("IN.1")
	PORT_BIT(0x001, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_Q) PORT_NAME("Black")
	PORT_BIT(0x002, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_B) PORT_CODE(KEYCODE_2) PORT_CODE(KEYCODE_2_PAD) PORT_NAME("B 2 / Knight")
	PORT_BIT(0x004, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_E) PORT_CODE(KEYCODE_5) PORT_CODE(KEYCODE_5_PAD) PORT_NAME("E 5 / Queen")
	PORT_BIT(0x008, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_H) PORT_CODE(KEYCODE_8) PORT_CODE(KEYCODE_8_PAD) PORT_NAME("H 8")
	PORT_BIT(0x010, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_I) PORT_NAME("Halt / Hint")
	PORT_BIT(0x020, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_U) PORT_NAME("Audio")
	PORT_BIT(0x040, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_T) PORT_NAME("Time / Number")
	PORT_BIT(0x080, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_F) PORT_CODE(KEYCODE_6) PORT_CODE(KEYCODE_6_PAD) PORT_NAME("F 6 / King")
	PORT_BIT(0x100, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_C) PORT_CODE(KEYCODE_3) PORT_CODE(KEYCODE_3_PAD) PORT_NAME("C 3 / Bishop")
	PORT_BIT(0x200, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_W) PORT_NAME("White")
INPUT_PORTS_END



/*******************************************************************************
    Machine Configs
*******************************************************************************/

void prodigy_state::prodigy(machine_config &config)
{
	// basic machine hardware
	M6502(config, m_maincpu, 2_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &prodigy_state::main_map);

	MOS6522(config, m_via, 2_MHz_XTAL); // DDRA = 0x00, DDRB = 0x8f
	m_via->readpa_handler().set(FUNC(prodigy_state::input1_r));
	m_via->readpb_handler().set(FUNC(prodigy_state::input2_r));
	m_via->writepb_handler().set(FUNC(prodigy_state::control_w));
	m_via->cb1_handler().set(FUNC(prodigy_state::shift_clock_w));
	m_via->cb2_handler().set(FUNC(prodigy_state::shift_data_w));
	m_via->irq_handler().set_inputline(m_maincpu, M6502_IRQ_LINE);

	SENSORBOARD(config, m_board).set_type(sensorboard_device::BUTTONS);
	m_board->init_cb().set(m_board, FUNC(sensorboard_device::preset_chess));
	m_board->set_delay(attotime::from_msec(150));

	// video hardware
	PWM_DISPLAY(config, m_display).set_size(6, 8);
	m_display->set_segmask(0xf, 0xff);
	config.set_default_layout(layout_aci_prodigy);

	// sound hardware
	SPEAKER(config, "speaker").front_center();
	DAC_1BIT(config, m_dac).add_route(ALL_OUTPUTS, "speaker", 0.25);
}



/*******************************************************************************
    ROM Definitions
*******************************************************************************/

ROM_START( prodigy )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("007-707101", 0x6000, 0x2000, CRC(8d60345a) SHA1(fff18ff12e1b1be91f8eac1178605a682564eff2) )
ROM_END

} // anonymous namespace



/*******************************************************************************
    Drivers
*******************************************************************************/

//    YEAR  NAME     PARENT  COMPAT  MACHINE  INPUT    CLASS          INIT        COMPANY, FULLNAME, FLAGS
SYST( 1982, prodigy, 0,      0,      prodigy, prodigy, prodigy_state, empty_init, "Applied Concepts", "Destiny Prodigy", MACHINE_SUPPORTS_SAVE )
