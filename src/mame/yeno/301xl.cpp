// license:BSD-3-Clause
// copyright-holders:hap
// thanks-to:Berger, Sean Riddle
/*******************************************************************************

Yeno 301 XL

NOTE: It triggers an IRQ when the power switch is changed from ON to SAVE.
If this is not done, NVRAM won't save properly.

It's by the same programmer as Chess King Triomphe / Counter Gambit, also using
the TRAP interrupt for the beeper routine. The ROM data contains (C)1988Bray,
it appears that Intelligent Chess Software went defunct around that time.

Hardware notes:
- PCB label: WSE 8108A
- Hitachi HD63B01X0P, 8MHz XTAL
- 8*8 chessboard buttons, 16+1 LEDs, piezo

Yeno 309 XT is on the same PCB, and has the same MCU ROM.

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
#include "yeno_301xl.lh"


namespace {

class y301xl_state : public driver_device
{
public:
	y301xl_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_board(*this, "board"),
		m_display(*this, "display"),
		m_dac(*this, "dac"),
		m_inputs(*this, "IN.%u", 0)
	{ }

	DECLARE_INPUT_CHANGED_MEMBER(power_off);

	void y301xl(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	// devices/pointers
	required_device<hd6301x0_cpu_device> m_maincpu;
	required_device<sensorboard_device> m_board;
	required_device<pwm_display_device> m_display;
	required_device<dac_1bit_device> m_dac;
	required_ioport_array<2> m_inputs;

	emu_timer *m_standbytimer;
	u16 m_inp_mux = 0;

	// I/O handlers
	u8 input_r();
	void control_w(u8 data);
	void board_w(u8 data);

	TIMER_CALLBACK_MEMBER(set_standby);
};

void y301xl_state::machine_start()
{
	m_standbytimer = timer_alloc(FUNC(y301xl_state::set_standby), this);

	// register for savestates
	save_item(NAME(m_inp_mux));
}



/*******************************************************************************
    Power
*******************************************************************************/

void y301xl_state::machine_reset()
{
	m_maincpu->set_input_line(HD6301_IRQ1_LINE, CLEAR_LINE);
	m_maincpu->set_input_line(M6801_STBY_LINE, CLEAR_LINE);
}

TIMER_CALLBACK_MEMBER(y301xl_state::set_standby)
{
	m_maincpu->set_input_line(M6801_STBY_LINE, ASSERT_LINE);
}

INPUT_CHANGED_MEMBER(y301xl_state::power_off)
{
	if (newval && !m_maincpu->standby())
	{
		// IRQ1 when power switch is set to SAVE, followed by STBY after a short delay
		m_maincpu->set_input_line(HD6301_IRQ1_LINE, ASSERT_LINE);
		m_standbytimer->adjust(attotime::from_msec(50));
	}
}



/*******************************************************************************
    I/O
*******************************************************************************/

u8 y301xl_state::input_r()
{
	// P20-P27: multiplexed inputs
	u8 data = 0;

	// read buttons
	for (int i = 0; i < 2; i++)
		if (BIT(m_inp_mux, i + 8))
			data |= m_inputs[i]->read();

	// read chessboard
	for (int i = 0; i < 8; i++)
		if (BIT(m_inp_mux, i))
			data |= m_board->read_file(i);

	return ~data;
}

void y301xl_state::control_w(u8 data)
{
	// P40,P41: input mux (buttons)
	m_inp_mux = (m_inp_mux & 0xff) | (~data << 8 & 0x300);

	// P42: green led (direct)
	// P44,P45: led select
	m_display->write_my(bitswap<3>(~data,2,5,4));

	// P47: speaker out
	m_dac->write(BIT(~data, 7));
}

void y301xl_state::board_w(u8 data)
{
	// P60-P67: input mux (chessboard), led data
	m_inp_mux = (m_inp_mux & 0x300) | (data ^ 0xff);
	m_display->write_mx(~data);
}



/*******************************************************************************
    Input Ports
*******************************************************************************/

static INPUT_PORTS_START( y301xl )
	PORT_START("IN.0")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_1) PORT_CODE(KEYCODE_1_PAD) PORT_NAME("Pawn")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_2) PORT_CODE(KEYCODE_2_PAD) PORT_NAME("Knight")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_3) PORT_CODE(KEYCODE_3_PAD) PORT_NAME("Bishop")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_4) PORT_CODE(KEYCODE_4_PAD) PORT_NAME("Rook")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_5) PORT_CODE(KEYCODE_5_PAD) PORT_NAME("Queen")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_6) PORT_CODE(KEYCODE_6_PAD) PORT_NAME("King")
	PORT_BIT(0xc0, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("IN.1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_S) PORT_NAME("Sound")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_P) PORT_NAME("Play")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_T) PORT_NAME("Take Back")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_L) PORT_NAME("Level")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_N) PORT_NAME("New")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_U) PORT_NAME("Set Up")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_B) PORT_NAME("Black")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_W) PORT_NAME("White")

	PORT_START("POWER") // needs to be triggered for nvram to work
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_POWER_OFF) PORT_CHANGED_MEMBER(DEVICE_SELF, y301xl_state, power_off, 0)
INPUT_PORTS_END



/*******************************************************************************
    Machine Configs
*******************************************************************************/

void y301xl_state::y301xl(machine_config &config)
{
	// basic machine hardware
	HD6301X0(config, m_maincpu, 8_MHz_XTAL);
	m_maincpu->nvram_enable_backup(true);
	m_maincpu->standby_cb().set(m_maincpu, FUNC(hd6301x0_cpu_device::nvram_set_battery));
	m_maincpu->standby_cb().append([this](int state) { if (state) m_display->clear(); });
	m_maincpu->in_p2_cb().set(FUNC(y301xl_state::input_r));
	m_maincpu->out_p4_cb().set(FUNC(y301xl_state::control_w));
	m_maincpu->out_p6_cb().set(FUNC(y301xl_state::board_w));

	SENSORBOARD(config, m_board).set_type(sensorboard_device::BUTTONS);
	m_board->init_cb().set(m_board, FUNC(sensorboard_device::preset_chess));
	m_board->set_delay(attotime::from_msec(150));
	m_board->set_nvram_enable(true);

	// video hardware
	PWM_DISPLAY(config, m_display).set_size(2+1, 8);
	config.set_default_layout(layout_yeno_301xl);

	// sound hardware
	SPEAKER(config, "speaker").front_center();
	DAC_1BIT(config, m_dac).add_route(ALL_OUTPUTS, "speaker", 0.25);
}



/*******************************************************************************
    ROM Definitions
*******************************************************************************/

ROM_START( y301xl )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD("yeno_301_xl_hd63b01x0c70p", 0x0000, 0x1000, CRC(6da5ee23) SHA1(f37298c56c5130783c86fb03fab78d86c20467e5) )
ROM_END

} // anonymous namespace



/*******************************************************************************
    Drivers
*******************************************************************************/

//    YEAR  NAME    PARENT  COMPAT  MACHINE  INPUT   CLASS         INIT        COMPANY, FULLNAME, FLAGS
SYST( 1988, y301xl, 0,      0,      y301xl,  y301xl, y301xl_state, empty_init, "Yeno / Bray Research", "301 XL (Yeno)", MACHINE_SUPPORTS_SAVE )
