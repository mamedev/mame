// license:BSD-3-Clause
// copyright-holders:hap
// thanks-to:Berger
/*******************************************************************************

Yeno 416 XL

The chess engine is by Frans Morsch, similar to Mephisto Europa.

NOTE: Turn the power off (SAVE) before exiting MAME, otherwise NVRAM won't save
properly.

Hardware notes:
- PCB label: CHESS3 YENO 416XL
- Hitachi HD6301Y0P @ ~8MHz (LC oscillator)
- 8*8 chessboard buttons, 16+7 LEDs, piezo

TODO:
- if/when MAME supports an exit callback, hook up power-off switch to that

*******************************************************************************/

#include "emu.h"

#include "cpu/m6800/m6801.h"
#include "machine/sensorboard.h"
#include "sound/dac.h"
#include "video/pwm.h"

#include "speaker.h"

// internal artwork
#include "yeno_416xl.lh"


namespace {

class y416xl_state : public driver_device
{
public:
	y416xl_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_board(*this, "board"),
		m_display(*this, "display"),
		m_dac(*this, "dac"),
		m_inputs(*this, "IN.%u", 0)
	{ }

	void y416xl(machine_config &config);

	DECLARE_INPUT_CHANGED_MEMBER(power_off) { if (newval) m_power = false; }

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override { m_power = true; }

private:
	// devices/pointers
	required_device<hd6301y0_cpu_device> m_maincpu;
	required_device<sensorboard_device> m_board;
	required_device<pwm_display_device> m_display;
	required_device<dac_1bit_device> m_dac;
	required_ioport_array<2> m_inputs;

	bool m_power = false;
	u16 m_inp_mux = 0;

	// I/O handlers
	template <int N> void leds_w(u8 data);
	u8 p2_r();
	void p2_w(u8 data);
	u8 p5_r();
	void p6_w(u8 data);
};

void y416xl_state::machine_start()
{
	// register for savestates
	save_item(NAME(m_power));
	save_item(NAME(m_inp_mux));
}



/*******************************************************************************
    I/O
*******************************************************************************/

template <int N>
void y416xl_state::leds_w(u8 data)
{
	// P10-P17, P30-P37, P40-P47: leds (direct)
	m_display->write_row(N, ~data);
}

u8 y416xl_state::p2_r()
{
	// P22: power switch
	return m_power ? 0xff : 0xfb;
}

void y416xl_state::p2_w(u8 data)
{
	// P20,P21: input mux (buttons)
	m_inp_mux = (m_inp_mux & 0xff) | (~data << 8 & 0x300);

	// P23: speaker out
	m_dac->write(BIT(data, 3));
}

u8 y416xl_state::p5_r()
{
	// P50-P57: multiplexed inputs
	u8 data = 0;

	// read buttons
	for (int i = 0; i < 3; i++)
		if (BIT(m_inp_mux, i + 8))
			data |= m_inputs[i]->read();

	// read chessboard
	for (int i = 0; i < 8; i++)
		if (BIT(m_inp_mux, i))
			data |= m_board->read_file(i);

	return ~data;
}

void y416xl_state::p6_w(u8 data)
{
	// P60-P67: input mux (chessboard)
	m_inp_mux = (m_inp_mux & 0x300) | (data ^ 0xff);
}



/*******************************************************************************
    Input Ports
*******************************************************************************/

static INPUT_PORTS_START( y416xl )
	PORT_START("IN.0")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_6) PORT_CODE(KEYCODE_6_PAD) PORT_NAME("Pawn")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_5) PORT_CODE(KEYCODE_5_PAD) PORT_NAME("Knight")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_4) PORT_CODE(KEYCODE_4_PAD) PORT_NAME("Bishop")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_3) PORT_CODE(KEYCODE_3_PAD) PORT_NAME("Rook")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_2) PORT_CODE(KEYCODE_2_PAD) PORT_NAME("Queen")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_1) PORT_CODE(KEYCODE_1_PAD) PORT_NAME("King")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_P) PORT_NAME("Play")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_T) PORT_NAME("Take Back")

	PORT_START("IN.1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_N) PORT_NAME("New Game")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_L) PORT_NAME("Level")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_S) PORT_NAME("Set Up")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_U) PORT_NAME("Multimove")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_C) PORT_NAME("Change Color")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_O) PORT_NAME("Sound / Change Direction")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_E) PORT_NAME("Learning")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_M) PORT_NAME("Show Move")

	PORT_START("POWER") // needs to be triggered for nvram to work
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_POWER_OFF) PORT_CHANGED_MEMBER(DEVICE_SELF, y416xl_state, power_off, 0)
INPUT_PORTS_END



/*******************************************************************************
    Machine Configs
*******************************************************************************/

void y416xl_state::y416xl(machine_config &config)
{
	// basic machine hardware
	HD6301Y0(config, m_maincpu, 8'000'000); // approximation, no XTAL
	m_maincpu->nvram_enable_backup(true);
	m_maincpu->standby_cb().set(m_maincpu, FUNC(hd6301y0_cpu_device::nvram_set_battery));
	m_maincpu->standby_cb().append([this](int state) { if (state) m_display->clear(); });
	m_maincpu->out_p1_cb().set(FUNC(y416xl_state::leds_w<0>));
	m_maincpu->in_p2_cb().set(FUNC(y416xl_state::p2_r));
	m_maincpu->out_p2_cb().set(FUNC(y416xl_state::p2_w));
	m_maincpu->out_p3_cb().set(FUNC(y416xl_state::leds_w<1>));
	m_maincpu->out_p4_cb().set(FUNC(y416xl_state::leds_w<2>));
	m_maincpu->in_p5_cb().set(FUNC(y416xl_state::p5_r));
	m_maincpu->out_p6_cb().set(FUNC(y416xl_state::p6_w));

	SENSORBOARD(config, m_board).set_type(sensorboard_device::BUTTONS);
	m_board->init_cb().set(m_board, FUNC(sensorboard_device::preset_chess));
	m_board->set_delay(attotime::from_msec(150));
	m_board->set_nvram_enable(true);

	// video hardware
	PWM_DISPLAY(config, m_display).set_size(3, 8);
	config.set_default_layout(layout_yeno_416xl);

	// sound hardware
	SPEAKER(config, "speaker").front_center();
	DAC_1BIT(config, m_dac).add_route(ALL_OUTPUTS, "speaker", 0.25);
}



/*******************************************************************************
    ROM Definitions
*******************************************************************************/

ROM_START( y416xl )
	ROM_REGION( 0x4000, "maincpu", 0 )
	ROM_LOAD("yeno_ic-890529_hd6301y0k79p.u1", 0x0000, 0x4000, CRC(85b4368d) SHA1(9dc12a6538141efe1ebcf8162ac7348ddc2b33ae) )
ROM_END

} // anonymous namespace



/*******************************************************************************
    Drivers
*******************************************************************************/

//    YEAR  NAME    PARENT  COMPAT  MACHINE  INPUT   CLASS         INIT        COMPANY, FULLNAME, FLAGS
SYST( 1989, y416xl, 0,      0,      y416xl,  y416xl, y416xl_state, empty_init, "Yeno", "416 XL (Yeno)", MACHINE_SUPPORTS_SAVE )
