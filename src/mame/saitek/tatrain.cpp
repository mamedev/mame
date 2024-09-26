// license:BSD-3-Clause
// copyright-holders:hap
// thanks-to:Berger
/*******************************************************************************

Saitek Kasparov Turbo Advanced Trainer

The chess engine is by Frans Morsch, it is the same as the one in GK 2000.

Hardware notes:

H8/323 version (1992):
- PCB label: ST14B-PE-003, PN/N 512090-00311, REV.1
- Hitachi H8/323 MCU, 20MHz XTAL
- piezo, 24 LEDs, button sensors chessboard

H8/3212 version (1997):
- PCB label: ST14B-PE 003, PN/N 512090-00312, REV.2
- Hitachi H8/3212 MCU, 10MHz XTAL
- rest is same as 1992 version

H8/323 A14 MCU is used in:
- Saitek Turbo Advanced Trainer (1992 version)
- Saitek Champion Advanced Trainer
- Saitek Virtuoso
- Saitek Mephisto Champion (Mephisto brand Champion Advanced Trainer)
- Hegener + Glaser Schach-Trainer (H+G brand Turbo Advanced Trainer)

Champion Advanced Trainer and Mephisto Champion are 14MHz instead of 20MHz.

H8/3212 V02 MCU is used in:
- Saitek Turbo Advanced Trainer (1997 version)
- Saitek Capella

Turbo Advanced Trainer looks similar to Saitek Team-Mate. Virtuoso and Capella
are in the same housing as SciSys Astral, they lack the coach LED and button.
Note that the H8/323 has an internal /2 clock divider, so even though the newer
version has a lower frequency XTAL, it runs at the same speed.

TODO:
- it does a cold boot at every reset, so nvram won't work properly unless MAME
  adds some kind of auxillary autosave state feature at power-off

*******************************************************************************/

#include "emu.h"

#include "cpu/h8/h83217.h"
#include "cpu/h8/h8325.h"
#include "machine/sensorboard.h"
#include "sound/dac.h"
#include "video/pwm.h"

#include "speaker.h"

// internal artwork
#include "saitek_tatrain.lh"


namespace {

class tatrain_state : public driver_device
{
public:
	tatrain_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_board(*this, "board"),
		m_display(*this, "led_pwm"),
		m_dac(*this, "dac"),
		m_inputs(*this, "IN.%u", 0)
	{ }

	template <typename T> void cpu_config(T &maincpu);
	void shared(machine_config &config);
	void tatrain(machine_config &config);
	void tatraina(machine_config &config);

	DECLARE_INPUT_CHANGED_MEMBER(go_button);
	DECLARE_INPUT_CHANGED_MEMBER(tatraina_change_cpu_freq);

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	// devices/pointers
	required_device<h8_device> m_maincpu;
	required_device<sensorboard_device> m_board;
	required_device<pwm_display_device> m_display;
	required_device<dac_1bit_device> m_dac;
	required_ioport_array<3> m_inputs;

	u16 m_inp_mux = 0;

	// I/O handlers
	template<int N> void leds_w(u8 data);
	u8 p4_r();
	void p5_w(u8 data);
	void p6_w(u8 data);
	void p7_w(u8 data);
};

void tatrain_state::machine_start()
{
	save_item(NAME(m_inp_mux));
}

INPUT_CHANGED_MEMBER(tatrain_state::tatraina_change_cpu_freq)
{
	// H8/323 16MHz and 24MHz versions don't exist, but the software supports it
	static const XTAL freq[4] = { 20_MHz_XTAL, 16_MHz_XTAL, 14_MHz_XTAL, 24_MHz_XTAL };
	m_maincpu->set_unscaled_clock(freq[newval & 3]);
}



/*******************************************************************************
    I/O
*******************************************************************************/

INPUT_CHANGED_MEMBER(tatrain_state::go_button)
{
	m_maincpu->set_input_line(INPUT_LINE_IRQ0, newval ? ASSERT_LINE : CLEAR_LINE);
}

template <int N>
void tatrain_state::leds_w(u8 data)
{
	// P1x, P2x, P3x: leds (direct)
	m_display->write_row(N, ~data);
}

u8 tatrain_state::p4_r()
{
	// P40-P47: multiplexed inputs
	u8 data = 0;

	// read buttons
	for (int i = 0; i < 2; i++)
		if (BIT(m_inp_mux, i + 8))
			data |= m_inputs[i]->read();

	// read chessboard
	for (int i = 0; i < 8; i++)
		if (BIT(m_inp_mux, i))
			data |= m_board->read_rank(i ^ 7);

	return ~data;
}

void tatrain_state::p5_w(u8 data)
{
	// P52,P53: input mux (buttons)
	m_inp_mux = (m_inp_mux & 0xff) | (~data << 6 & 0x300);
}

void tatrain_state::p6_w(u8 data)
{
	// P60: speaker out
	m_dac->write(data & 1);

	// P61-P63: N/C (appears to be compatible with Turbo 16K LCDs)
}

void tatrain_state::p7_w(u8 data)
{
	// P70-P77: input mux (chessboard)
	m_inp_mux = (m_inp_mux & 0x300) | (data ^ 0xff);
}



/*******************************************************************************
    Input Ports
*******************************************************************************/

static INPUT_PORTS_START( tatrain )
	PORT_START("IN.0")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_A) PORT_NAME("Non Auto")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_P) PORT_NAME("Play")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_1) PORT_CODE(KEYCODE_1_PAD) PORT_NAME("King")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_6) PORT_CODE(KEYCODE_6_PAD) PORT_NAME("Pawn")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_4) PORT_CODE(KEYCODE_4_PAD) PORT_NAME("Bishop")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_U) PORT_NAME("Set Up")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_S) PORT_NAME("Stop")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_I) PORT_NAME("Info")

	PORT_START("IN.1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_H) PORT_NAME("Coach")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_T) PORT_NAME("Take Back")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_2) PORT_CODE(KEYCODE_2_PAD) PORT_NAME("Queen")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_5) PORT_CODE(KEYCODE_5_PAD) PORT_NAME("Knight")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_3) PORT_CODE(KEYCODE_3_PAD) PORT_NAME("Rook")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_N) PORT_NAME("New Game")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_L) PORT_NAME("Level")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_C) PORT_NAME("Color")

	PORT_START("IN.2")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_G) PORT_CHANGED_MEMBER(DEVICE_SELF, tatrain_state, go_button, 0) PORT_NAME("Go / Stop")
	PORT_BIT(0xef, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("FREQ")
	PORT_BIT(0xff, IP_ACTIVE_HIGH, IPT_UNUSED)
INPUT_PORTS_END

static INPUT_PORTS_START( tatraina )
	PORT_INCLUDE( tatrain )

	PORT_MODIFY("FREQ")
	PORT_CONFNAME( 0x03, 0x00, "CPU Frequency" ) PORT_CHANGED_MEMBER(DEVICE_SELF, tatrain_state, tatraina_change_cpu_freq, 0) // factory set
	PORT_CONFSETTING(    0x02, "14Mhz (Champion Advanced Trainer)" )
	PORT_CONFSETTING(    0x01, "16MHz (unofficial)" )
	PORT_CONFSETTING(    0x00, "20Mhz (Turbo Advanced Trainer, Virtuoso)" )
	PORT_CONFSETTING(    0x03, "24MHz (unofficial)" )
INPUT_PORTS_END



/*******************************************************************************
    Machine Configs
*******************************************************************************/

template <typename T>
void tatrain_state::cpu_config(T &maincpu)
{
	maincpu.nvram_enable_backup(true);
	maincpu.standby_cb().set(maincpu, FUNC(T::nvram_set_battery));
	maincpu.standby_cb().append([this](int state) { if (state) m_display->clear(); });
	maincpu.write_port1().set(FUNC(tatrain_state::leds_w<0>));
	maincpu.write_port2().set(FUNC(tatrain_state::leds_w<1>));
	maincpu.write_port3().set(FUNC(tatrain_state::leds_w<2>));
	maincpu.read_port4().set(FUNC(tatrain_state::p4_r));
	maincpu.read_port5().set_ioport("FREQ").invert();
	maincpu.write_port5().set(FUNC(tatrain_state::p5_w));
	maincpu.read_port6().set_ioport("IN.2").invert();
	maincpu.write_port6().set(FUNC(tatrain_state::p6_w));
	maincpu.write_port7().set(FUNC(tatrain_state::p7_w));
}

void tatrain_state::shared(machine_config &config)
{
	// basic machine hardware
	SENSORBOARD(config, m_board).set_type(sensorboard_device::BUTTONS);
	m_board->init_cb().set(m_board, FUNC(sensorboard_device::preset_chess));
	m_board->set_delay(attotime::from_msec(150));
	//m_board->set_nvram_enable(true);

	// video hardware
	PWM_DISPLAY(config, m_display).set_size(3, 8);
	config.set_default_layout(layout_saitek_tatrain);

	// sound hardware
	SPEAKER(config, "speaker").front_center();
	DAC_1BIT(config, m_dac).add_route(ALL_OUTPUTS, "speaker", 0.25);
}

void tatrain_state::tatrain(machine_config &config)
{
	H83212(config, m_maincpu, 10_MHz_XTAL);
	cpu_config<h83212_device>(downcast<h83212_device &>(*m_maincpu));

	shared(config);
}

void tatrain_state::tatraina(machine_config &config)
{
	H8323(config, m_maincpu, 20_MHz_XTAL);
	cpu_config<h8323_device>(downcast<h8323_device &>(*m_maincpu));

	shared(config);
}



/*******************************************************************************
    ROM Definitions
*******************************************************************************/

ROM_START( tatrain )
	ROM_REGION( 0x4000, "maincpu", 0 )
	ROM_LOAD("97_saitek_86158430421_hd6433212v02p.u1", 0x0000, 0x4000, CRC(73f9abb6) SHA1(3a4c3a8ad668327fe9f61c4b054e31ec6af9c48d) )
ROM_END

ROM_START( tatraina )
	ROM_REGION( 0x4000, "maincpu", 0 )
	ROM_LOAD("92_saitek_86069221x14_3238a14p.u1", 0x0000, 0x4000, CRC(ae2d536c) SHA1(6397c38d21a4291a992bf317e695aadbd4510260) )
ROM_END

} // anonymous namespace



/*******************************************************************************
    Drivers
*******************************************************************************/

//    YEAR  NAME      PARENT   COMPAT  MACHINE   INPUT     CLASS          INIT        COMPANY, FULLNAME, FLAGS
SYST( 1997, tatrain,  0,       0,      tatrain,  tatrain,  tatrain_state, empty_init, "Saitek", "Kasparov Turbo Advanced Trainer (H8/3212 version)", MACHINE_SUPPORTS_SAVE )
SYST( 1992, tatraina, tatrain, 0,      tatraina, tatraina, tatrain_state, empty_init, "Saitek", "Kasparov Turbo Advanced Trainer (H8/323 version)", MACHINE_SUPPORTS_SAVE )
