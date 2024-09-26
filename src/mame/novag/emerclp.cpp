// license:BSD-3-Clause
// copyright-holders:hap
// thanks-to:Berger
/*******************************************************************************

Novag Emerald Classic Plus (model 38710)

Hardware notes:
- PCB label: 100215 A, EMERALD CLASSIC II
- Hitachi H8/325 MCU, 26.601712MHz XTAL
- LCD with 6 7segs and custom segments (same as Sapphire II)
- piezo, 16 LEDs, 8*8 chessboard buttons

H8/325 C88 MCU is used in:
- Novag Emerald Classic Plus
- Novag Amber (suspected)
- Novag Turquoise (suspected)
- Excalibur Karpov 2294 (Excalibur brand Emerald Classic Plus)

TODO:
- are Emerald and Emerald Classic on similar hardware? and Novag Obsidian?
- it does a cold boot at every reset, so nvram won't work properly unless MAME
  adds some kind of auxillary autosave state feature at power-off

*******************************************************************************/

#include "emu.h"

#include "cpu/h8/h8325.h"
#include "machine/sensorboard.h"
#include "sound/dac.h"
#include "video/pwm.h"

#include "screen.h"
#include "speaker.h"

// internal artwork
#include "novag_emerclp.lh"


namespace {

class emerclp_state : public driver_device
{
public:
	emerclp_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_board(*this, "board"),
		m_led_pwm(*this, "led_pwm"),
		m_lcd_pwm(*this, "lcd_pwm"),
		m_dac(*this, "dac"),
		m_inputs(*this, "IN.%u", 0),
		m_out_lcd(*this, "s%u.%u", 0U, 0U)
	{ }

	void emerclp(machine_config &config);

	DECLARE_INPUT_CHANGED_MEMBER(power_switch);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override { set_power(true); }

private:
	// devices/pointers
	required_device<h8325_device> m_maincpu;
	required_device<sensorboard_device> m_board;
	required_device<pwm_display_device> m_led_pwm;
	required_device<pwm_display_device> m_lcd_pwm;
	required_device<dac_2bit_ones_complement_device> m_dac;
	required_ioport_array<2> m_inputs;
	output_finder<4, 16> m_out_lcd;

	bool m_power = false;
	u8 m_inp_mux = 0;
	u8 m_lcd_com = 0;
	u32 m_lcd_segs = 0;

	// I/O handlers
	void standby(int state);
	void set_power(bool power);
	u8 power_r();

	void lcd_pwm_w(offs_t offset, u8 data);
	void update_lcd();
	template <int N> void lcd_segs_w(u8 data);
	void lcd_com_w(u8 data);

	void p1_w(u8 data);
	u8 p2_r();
	u8 p4_r();
	void p4_w(u8 data);
};

void emerclp_state::machine_start()
{
	m_out_lcd.resolve();

	// register for savestates
	save_item(NAME(m_power));
	save_item(NAME(m_inp_mux));
	save_item(NAME(m_lcd_com));
	save_item(NAME(m_lcd_segs));
}



/*******************************************************************************
    I/O
*******************************************************************************/

// power

void emerclp_state::standby(int state)
{
	// clear display
	if (state)
	{
		m_lcd_pwm->clear();
		m_led_pwm->clear();
	}
}

void emerclp_state::set_power(bool power)
{
	// power switch is tied to IRQ0
	m_maincpu->set_input_line(INPUT_LINE_IRQ0, power ? ASSERT_LINE : CLEAR_LINE);
	m_power = power;
}

INPUT_CHANGED_MEMBER(emerclp_state::power_switch)
{
	if (newval)
		set_power(bool(param));
}

u8 emerclp_state::power_r()
{
	// P64: power switch (IRQ0)
	return m_power ? 0xef : 0xff;
}


// LCD

void emerclp_state::lcd_pwm_w(offs_t offset, u8 data)
{
	m_out_lcd[offset & 0x3f][offset >> 6] = data;
}

void emerclp_state::update_lcd()
{
	u16 lcd_segs = bitswap<16>(m_lcd_segs, 23,22,21,20,19,18,17,16, 13,12,11,10, 3,2,1,0);

	for (int i = 0; i < 4; i++)
	{
		// LCD common is analog (voltage level)
		const u8 com = population_count_32(m_lcd_com >> (i * 2) & 3);
		const u16 data = (com == 0) ? lcd_segs : (com == 2) ? ~lcd_segs : 0;
		m_lcd_pwm->write_row(i, data);
	}
}

template <int N>
void emerclp_state::lcd_segs_w(u8 data)
{
	// P4x, P5x, P7x: LCD segments
	const u8 shift = 8 * N;
	m_lcd_segs = (m_lcd_segs & ~(0xff << shift)) | (data << shift);
	update_lcd();
}

void emerclp_state::lcd_com_w(u8 data)
{
	// P30-P37: LCD common
	m_lcd_com = data;
	update_lcd();
}


// misc

void emerclp_state::p1_w(u8 data)
{
	// P10-P17: input mux, LED data
	m_inp_mux = ~data;
	m_led_pwm->write_mx(~data);
}

u8 emerclp_state::p2_r()
{
	u8 data = 0;

	// P20-P27: read chessboard
	for (int i = 0; i < 8; i++)
		if (BIT(m_inp_mux, i))
			data |= m_board->read_rank(i, true);

	return ~data;
}

u8 emerclp_state::p4_r()
{
	u8 data = 0;

	// P46,P47: read buttons
	for (int i = 0; i < 2; i++)
		if (m_inp_mux & m_inputs[i]->read())
			data |= 0x40 << i;

	return ~data;
}

void emerclp_state::p4_w(u8 data)
{
	// P44,P45: LED select
	m_led_pwm->write_my(~data >> 4 & 3);
}



/*******************************************************************************
    Input Ports
*******************************************************************************/

static INPUT_PORTS_START( emerclp )
	PORT_START("IN.0")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_1) PORT_NAME("Change Color")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_2) PORT_NAME("Take Back / Next Best")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_3) PORT_NAME("King / Easy")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_4) PORT_NAME("Queen / Random")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_5) PORT_NAME("Rook / Restore")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_6) PORT_NAME("Bishop / Info")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_7) PORT_NAME("Knight / Sound")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_8) PORT_NAME("Pawn / Referee")

	PORT_START("IN.1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_Q) PORT_NAME("Go")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_W) PORT_NAME("Trace Forward / Autoplay")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_E) PORT_NAME("Verify / Set Up")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_R) PORT_NAME("Training")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_T) PORT_NAME("Hint")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_Y) PORT_NAME("Set Level")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_U) PORT_NAME("Clear")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_I) PORT_NAME("New Game")

	PORT_START("POWER")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_POWER_ON) PORT_CHANGED_MEMBER(DEVICE_SELF, emerclp_state, power_switch, 1)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_POWER_OFF) PORT_CHANGED_MEMBER(DEVICE_SELF, emerclp_state, power_switch, 0)
INPUT_PORTS_END



/*******************************************************************************
    Machine Configs
*******************************************************************************/

void emerclp_state::emerclp(machine_config &config)
{
	// basic machine hardware
	H8325(config, m_maincpu, 26.601712_MHz_XTAL);
	m_maincpu->nvram_enable_backup(true);
	m_maincpu->standby_cb().set(m_maincpu, FUNC(h8325_device::nvram_set_battery));
	m_maincpu->standby_cb().append(FUNC(emerclp_state::standby));
	m_maincpu->write_port1().set(FUNC(emerclp_state::p1_w));
	m_maincpu->read_port2().set(FUNC(emerclp_state::p2_r));
	m_maincpu->write_port3().set(FUNC(emerclp_state::lcd_com_w));
	m_maincpu->read_port4().set(FUNC(emerclp_state::p4_r));
	m_maincpu->write_port4().set(FUNC(emerclp_state::p4_w));
	m_maincpu->write_port4().append(FUNC(emerclp_state::lcd_segs_w<0>));
	m_maincpu->write_port5().set(FUNC(emerclp_state::lcd_segs_w<1>));
	m_maincpu->read_port6().set(FUNC(emerclp_state::power_r));
	m_maincpu->write_port6().set(m_dac, FUNC(dac_2bit_ones_complement_device::write)).mask(3);
	m_maincpu->write_port7().set(FUNC(emerclp_state::lcd_segs_w<2>));

	SENSORBOARD(config, m_board).set_type(sensorboard_device::BUTTONS);
	m_board->init_cb().set(m_board, FUNC(sensorboard_device::preset_chess));
	m_board->set_delay(attotime::from_msec(150));
	//m_board->set_nvram_enable(true);

	// video hardware
	PWM_DISPLAY(config, m_lcd_pwm).set_size(4, 16);
	m_lcd_pwm->output_x().set(FUNC(emerclp_state::lcd_pwm_w));

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_SVG));
	screen.set_refresh_hz(60);
	screen.set_size(1920/5, 671/5);
	screen.set_visarea_full();

	PWM_DISPLAY(config, m_led_pwm).set_size(2, 8);
	config.set_default_layout(layout_novag_emerclp);

	// sound hardware
	SPEAKER(config, "speaker").front_center();
	DAC_2BIT_ONES_COMPLEMENT(config, m_dac).add_route(ALL_OUTPUTS, "speaker", 0.125);
}



/*******************************************************************************
    ROM Definitions
*******************************************************************************/

ROM_START( emerclp )
	ROM_REGION16_BE( 0x8000, "maincpu", 0 )
	ROM_LOAD("ihp_1700106_6433258c88f.u1", 0x0000, 0x8000, CRC(8638661f) SHA1(de8f434fb5b079ec3137b4cf993822a5a5aafde8) )

	ROM_REGION( 72533, "screen", 0 )
	ROM_LOAD("sapphire2.svg", 0, 72533, CRC(34944b61) SHA1(4a0536ac07790cced9f9bf15522b17ebc375ff8a) )
ROM_END

} // anonymous namespace



/*******************************************************************************
    Drivers
*******************************************************************************/

//    YEAR  NAME     PARENT  COMPAT  MACHINE  INPUT     CLASS          INIT        COMPANY, FULLNAME, FLAGS
SYST( 1997, emerclp, 0,      0,      emerclp,  emerclp, emerclp_state, empty_init, "Novag Industries / Intelligent Heuristic Programming", "Emerald Classic Plus", MACHINE_SUPPORTS_SAVE )
