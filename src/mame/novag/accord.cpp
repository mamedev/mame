// license:BSD-3-Clause
// copyright-holders:hap
// thanks-to:Berger
/*******************************************************************************

Novag Accord (model 875)

NOTE: It triggers an NMI at power-off (or power-failure). If this isn't done,
NVRAM won't work properly.

Hardware notes (Alto):
- PCB label: 100088/100089
- Hitachi HD6301X0P @ ~8MHz (LC oscillator)
- 8*8 chessboard buttons, 16+4 leds, piezo

The I/O is almost the same as on Constellation Junior, just using other pins.
But since the MCU is different, it's awkward to put in the same driver (can't
use the standard cpu_device here).

Accord and Alto have the same MCU ROM.

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
#include "novag_accord.lh"


namespace {

class accord_state : public driver_device
{
public:
	accord_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_board(*this, "board"),
		m_display(*this, "display"),
		m_dac(*this, "dac"),
		m_inputs(*this, "IN.0")
	{ }

	void accord(machine_config &config);

	DECLARE_INPUT_CHANGED_MEMBER(power_off);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	// devices/pointers
	required_device<hd6301x0_cpu_device> m_maincpu;
	required_device<sensorboard_device> m_board;
	required_device<pwm_display_device> m_display;
	required_device<dac_1bit_device> m_dac;
	required_ioport m_inputs;

	emu_timer *m_standbytimer;
	u16 m_inp_mux = 0;

	// I/O handlers
	void control_w(u8 data);
	u8 input_r();
	void board_w(u8 data);
	void ledsel_w(u8 data);

	TIMER_CALLBACK_MEMBER(set_standby);
};

void accord_state::machine_start()
{
	m_standbytimer = timer_alloc(FUNC(accord_state::set_standby), this);

	// register for savestates
	save_item(NAME(m_inp_mux));
}



/*******************************************************************************
    Power
*******************************************************************************/

void accord_state::machine_reset()
{
	m_maincpu->set_input_line(INPUT_LINE_NMI, CLEAR_LINE);
	m_maincpu->set_input_line(M6801_STBY_LINE, CLEAR_LINE);
}

TIMER_CALLBACK_MEMBER(accord_state::set_standby)
{
	m_maincpu->set_input_line(M6801_STBY_LINE, ASSERT_LINE);
}

INPUT_CHANGED_MEMBER(accord_state::power_off)
{
	if (newval && !m_maincpu->standby())
	{
		// NMI when power goes off, followed by STBY after a short delay
		m_maincpu->set_input_line(INPUT_LINE_NMI, ASSERT_LINE);
		m_standbytimer->adjust(attotime::from_msec(10));
	}
}



/*******************************************************************************
    I/O
*******************************************************************************/

void accord_state::control_w(u8 data)
{
	// P26: speaker out
	m_dac->write(BIT(~data, 6));

	// P27: input mux (buttons)
	m_inp_mux = (m_inp_mux & 0xff) | (~data << 1 & 0x100);
}

u8 accord_state::input_r()
{
	// P30-P37: multiplexed inputs
	u8 data = 0;

	// read buttons
	if (m_inp_mux & 0x100)
		data |= m_inputs->read();

	// read chessboard
	for (int i = 0; i < 8; i++)
		if (BIT(m_inp_mux, i))
			data |= m_board->read_rank(i, true);

	return ~data;
}

void accord_state::board_w(u8 data)
{
	// P60-P67: input mux (chessboard), led data
	m_inp_mux = (m_inp_mux & 0x100) | (data ^ 0xff);
	m_display->write_mx(~data);
}

void accord_state::ledsel_w(u8 data)
{
	// P70-P72: led select
	m_display->write_my(~data & 7);
}



/*******************************************************************************
    Input Ports
*******************************************************************************/

static INPUT_PORTS_START( accord )
	PORT_START("IN.0")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_8) PORT_CODE(KEYCODE_8_PAD) PORT_CODE(KEYCODE_G) PORT_NAME("Go")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_7) PORT_CODE(KEYCODE_7_PAD) PORT_CODE(KEYCODE_T) PORT_NAME("Take Back / King")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_6) PORT_CODE(KEYCODE_6_PAD) PORT_CODE(KEYCODE_S) PORT_NAME("Sound / Queen")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_5) PORT_CODE(KEYCODE_5_PAD) PORT_CODE(KEYCODE_L) PORT_NAME("Set Level / Bishop")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_4) PORT_CODE(KEYCODE_4_PAD) PORT_CODE(KEYCODE_N) PORT_NAME("New Game / Knight")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_3) PORT_CODE(KEYCODE_3_PAD) PORT_CODE(KEYCODE_U) PORT_NAME("Set Up / Rook")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_2) PORT_CODE(KEYCODE_2_PAD) PORT_CODE(KEYCODE_V) PORT_NAME("Verify / Pawn")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_1) PORT_CODE(KEYCODE_1_PAD) PORT_CODE(KEYCODE_B) PORT_NAME("Black/White")

	PORT_START("POWER") // needs to be triggered for nvram to work
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_POWER_OFF) PORT_CHANGED_MEMBER(DEVICE_SELF, accord_state, power_off, 0)
INPUT_PORTS_END



/*******************************************************************************
    Machine Configs
*******************************************************************************/

void accord_state::accord(machine_config &config)
{
	// basic machine hardware
	HD6301X0(config, m_maincpu, 8'000'000);
	m_maincpu->nvram_enable_backup(true);
	m_maincpu->standby_cb().set(m_maincpu, FUNC(hd6301v1_cpu_device::nvram_set_battery));
	m_maincpu->standby_cb().append([this](int state) { if (state) m_display->clear(); });
	m_maincpu->out_p2_cb().set(FUNC(accord_state::control_w));
	m_maincpu->in_p3_cb().set(FUNC(accord_state::input_r));
	m_maincpu->out_p6_cb().set(FUNC(accord_state::board_w));
	m_maincpu->out_p7_cb().set(FUNC(accord_state::ledsel_w));

	SENSORBOARD(config, m_board).set_type(sensorboard_device::BUTTONS);
	m_board->init_cb().set(m_board, FUNC(sensorboard_device::preset_chess));
	m_board->set_delay(attotime::from_msec(150));
	m_board->set_nvram_enable(true);

	// video hardware
	PWM_DISPLAY(config, m_display).set_size(3, 8);
	config.set_default_layout(layout_novag_accord);

	// sound hardware
	SPEAKER(config, "speaker").front_center();
	DAC_1BIT(config, m_dac).add_route(ALL_OUTPUTS, "speaker", 0.25);
}



/*******************************************************************************
    ROM Definitions
*******************************************************************************/

ROM_START( accord )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD("novag_875_31x0b87p", 0x0000, 0x1000, CRC(4d77b7db) SHA1(21b38448fcf08aed8acc1b6e3bee2164ee638c9b) )
ROM_END

} // anonymous namespace



/*******************************************************************************
    Drivers
*******************************************************************************/

//    YEAR  NAME    PARENT  COMPAT  MACHINE  INPUT   CLASS         INIT        COMPANY, FULLNAME, FLAGS
SYST( 1987, accord, 0,      0,      accord,  accord, accord_state, empty_init, "Novag Industries / Intelligent Heuristic Programming", "Accord", MACHINE_SUPPORTS_SAVE )
