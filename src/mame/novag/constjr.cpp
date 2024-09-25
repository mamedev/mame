// license:BSD-3-Clause
// copyright-holders:hap
// thanks-to:Mychess
/*******************************************************************************

Novag Constellation Junior (aka Constellation Jr.)

NOTE: It triggers an NMI at power-off (or power-failure). If this isn't done,
NVRAM won't work properly.

The interface and look are very similar to Novag Presto (Micro II program).
The chess engine is by David Kittinger.

Constellation Junior model number is 852, but the MCU label says 846, and the MCU
datestamp is from 1984. There is no known Novag model 846, so perhaps they intended
to sell it in 1984, but changed their mind and delayed it until early 1985.

Hardware notes:
- PCB label: 100040/100039
- Hitachi HD6301V1P @ ~4MHz (LC oscillator)
- 8*8 chessboard buttons, 16+4 leds, piezo

TODO:
- if/when MAME supports an exit callback, hook up power-off switch to that

BTANB:
- slower chessboard button response when a piece moves 1 square vertically between
  the 4th and the 5th ranks, hence the 350ms set_delay

*******************************************************************************/

#include "emu.h"

#include "cpu/m6800/m6801.h"
#include "machine/sensorboard.h"
#include "sound/dac.h"
#include "video/pwm.h"

#include "speaker.h"

// internal artwork
#include "novag_constjr.lh"


namespace {

class constjr_state : public driver_device
{
public:
	constjr_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_board(*this, "board"),
		m_display(*this, "display"),
		m_dac(*this, "dac"),
		m_inputs(*this, "IN.0")
	{ }

	void constjr(machine_config &config);

	DECLARE_INPUT_CHANGED_MEMBER(power_off);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	// devices/pointers
	required_device<hd6301v1_cpu_device> m_maincpu;
	required_device<sensorboard_device> m_board;
	required_device<pwm_display_device> m_display;
	required_device<dac_1bit_device> m_dac;
	required_ioport m_inputs;

	emu_timer *m_standbytimer;
	u16 m_inp_mux = 0;

	// I/O handlers
	void board_w(u8 data);
	void control_w(u8 data);
	void ledsel_w(u8 data);
	u8 input_r();

	TIMER_CALLBACK_MEMBER(set_standby);
};

void constjr_state::machine_start()
{
	m_standbytimer = timer_alloc(FUNC(constjr_state::set_standby), this);

	// register for savestates
	save_item(NAME(m_inp_mux));
}



/*******************************************************************************
    Power
*******************************************************************************/

void constjr_state::machine_reset()
{
	m_maincpu->set_input_line(INPUT_LINE_NMI, CLEAR_LINE);
	m_maincpu->set_input_line(M6801_STBY_LINE, CLEAR_LINE);
}

TIMER_CALLBACK_MEMBER(constjr_state::set_standby)
{
	m_maincpu->set_input_line(M6801_STBY_LINE, ASSERT_LINE);
}

INPUT_CHANGED_MEMBER(constjr_state::power_off)
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

void constjr_state::board_w(u8 data)
{
	// P10-P17: input mux (chessboard), led data
	m_inp_mux = (m_inp_mux & 0x100) | (data ^ 0xff);
	m_display->write_mx(~data);
}

void constjr_state::control_w(u8 data)
{
	// P22: speaker out
	m_dac->write(BIT(~data, 2));

	// P24: input mux (buttons)
	m_inp_mux = (m_inp_mux & 0xff) | (~data << 4 & 0x100);
}

void constjr_state::ledsel_w(u8 data)
{
	// P35-P37: led select
	m_display->write_my(~data >> 5 & 7);
}

u8 constjr_state::input_r()
{
	// P40-P47: multiplexed inputs
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



/*******************************************************************************
    Input Ports
*******************************************************************************/

static INPUT_PORTS_START( constjr )
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
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_POWER_OFF) PORT_CHANGED_MEMBER(DEVICE_SELF, constjr_state, power_off, 0)
INPUT_PORTS_END



/*******************************************************************************
    Machine Configs
*******************************************************************************/

void constjr_state::constjr(machine_config &config)
{
	// basic machine hardware
	HD6301V1(config, m_maincpu, 4'000'000); // approximation, no XTAL
	m_maincpu->nvram_enable_backup(true);
	m_maincpu->standby_cb().set(m_maincpu, FUNC(hd6301v1_cpu_device::nvram_set_battery));
	m_maincpu->standby_cb().append([this](int state) { if (state) m_display->clear(); });
	m_maincpu->out_p1_cb().set(FUNC(constjr_state::board_w));
	m_maincpu->out_p2_cb().set(FUNC(constjr_state::control_w));
	m_maincpu->out_p3_cb().set(FUNC(constjr_state::ledsel_w));
	m_maincpu->in_p4_cb().set(FUNC(constjr_state::input_r));

	SENSORBOARD(config, m_board).set_type(sensorboard_device::BUTTONS);
	m_board->init_cb().set(m_board, FUNC(sensorboard_device::preset_chess));
	m_board->set_delay(attotime::from_msec(350));
	m_board->set_nvram_enable(true);

	// video hardware
	PWM_DISPLAY(config, m_display).set_size(3, 8);
	config.set_default_layout(layout_novag_constjr);

	// sound hardware
	SPEAKER(config, "speaker").front_center();
	DAC_1BIT(config, m_dac).add_route(ALL_OUTPUTS, "speaker", 0.25);
}



/*******************************************************************************
    ROM Definitions
*******************************************************************************/

ROM_START( constjr )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD("hd6301v1p_d24_ucc-846", 0x0000, 0x1000, CRC(e73703e4) SHA1(03eb889a9981a6bdfca129ea84f984fbf7858d47) )
ROM_END

} // anonymous namespace



/*******************************************************************************
    Drivers
*******************************************************************************/

//    YEAR  NAME     PARENT  COMPAT  MACHINE  INPUT    CLASS          INIT        COMPANY, FULLNAME, FLAGS
SYST( 1985, constjr, 0,      0,      constjr, constjr, constjr_state, empty_init, "Novag Industries / Intelligent Heuristic Programming", "Constellation Junior", MACHINE_SUPPORTS_SAVE )
