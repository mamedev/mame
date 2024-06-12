// license:BSD-3-Clause
// copyright-holders:hap
// thanks-to:Sean Riddle
/*******************************************************************************

Saitek Kasparov GK 2000

The chess engine is by Frans Morsch. According to schematics, GK 2100 is on the
same hardware.

Hardware notes:
- Hitachi H8/323 MCU, 20MHz XTAL
- LCD with 5 7segs and custom segments
- piezo, 16 LEDs, button sensors chessboard

A13 MCU is used in:
- Saitek GK 2000 (86071220X12)
- Saitek Travel Champion 2080 (86071220X12)
- Saitek Barracuda (suspected)
- Saitek Mephisto Champion (suspected)
- Saitek Mephisto Mythos (86142221X34)
- Tandy (Radio Shack) Mega 2050X (86071221X12)
- Tandy (Radio Shack) Master 2200X (suspected)
- Tandy (Radio Shack) Chess Master (suspected)

TODO:
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
#include "saitek_gk2000.lh"


namespace {

class gk2000_state : public driver_device
{
public:
	gk2000_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_board(*this, "board"),
		m_led_pwm(*this, "led_pwm"),
		m_lcd_pwm(*this, "lcd_pwm"),
		m_dac(*this, "dac"),
		m_inputs(*this, "IN.%u", 0),
		m_out_lcd(*this, "s%u.%u", 0U, 0U)
	{ }

	void gk2000(machine_config &config);

	DECLARE_INPUT_CHANGED_MEMBER(go_button);
	DECLARE_INPUT_CHANGED_MEMBER(change_cpu_freq);

protected:
	virtual void machine_start() override;

private:
	// devices/pointers
	required_device<h8323_device> m_maincpu;
	required_device<sensorboard_device> m_board;
	required_device<pwm_display_device> m_led_pwm;
	required_device<pwm_display_device> m_lcd_pwm;
	required_device<dac_1bit_device> m_dac;
	required_ioport_array<4> m_inputs;
	output_finder<2, 24> m_out_lcd;

	u16 m_inp_mux = 0;
	u32 m_lcd_segs = 0;
	u8 m_lcd_com = 0;

	// I/O handlers
	void lcd_pwm_w(offs_t offset, u8 data);
	void update_lcd();
	template <int N> void lcd_segs_w(u8 data);
	void lcd_com_w(u8 data);

	void standby(int state);

	void p2_w(u8 data);
	u8 p4_r();
	void p5_w(u8 data);
};

void gk2000_state::machine_start()
{
	m_out_lcd.resolve();

	// register for savestates
	save_item(NAME(m_inp_mux));
	save_item(NAME(m_lcd_segs));
	save_item(NAME(m_lcd_com));
}

INPUT_CHANGED_MEMBER(gk2000_state::change_cpu_freq)
{
	// only 20MHz and 14MHz versions are known to exist, but the software supports others
	static const int xm[9] = { 8, 20, 24, 28, 32, -1, -1, -1, 14 }; // XTAL in MHz (-1 is invalid)
	int mhz = xm[(count_leading_zeros_32(bitswap<8>(newval,0,1,2,3,4,5,6,7)) - 24) % 9];

	if (mhz > 0)
		m_maincpu->set_unscaled_clock(mhz * 1'000'000);
}



/*******************************************************************************
    I/O
*******************************************************************************/

// power

void gk2000_state::standby(int state)
{
	// clear display
	if (state)
	{
		m_lcd_pwm->clear();
		m_led_pwm->clear();
	}
}

INPUT_CHANGED_MEMBER(gk2000_state::go_button)
{
	m_maincpu->set_input_line(INPUT_LINE_IRQ0, newval ? ASSERT_LINE : CLEAR_LINE);
}


// LCD

void gk2000_state::lcd_pwm_w(offs_t offset, u8 data)
{
	m_out_lcd[offset & 0x3f][offset >> 6] = data;
}

void gk2000_state::update_lcd()
{
	for (int i = 0; i < 2; i++)
	{
		// LCD common is analog (voltage level)
		const u8 com = population_count_32(m_lcd_com >> (i * 2) & 3);
		const u32 data = (com == 0) ? m_lcd_segs : (com == 2) ? ~m_lcd_segs : 0;
		m_lcd_pwm->write_row(i, data);
	}
}

template <int N>
void gk2000_state::lcd_segs_w(u8 data)
{
	// P1x, P3x, P7x: LCD segments
	const u8 shift = 8 * N;
	m_lcd_segs = (m_lcd_segs & ~(0xff << shift)) | (data << shift);
	update_lcd();
}

void gk2000_state::lcd_com_w(u8 data)
{
	// P60-P63: LCD common
	m_lcd_com = data & 0xf;
	update_lcd();
}


// misc

void gk2000_state::p2_w(u8 data)
{
	// P20-P27: input mux (chessboard), led data
	m_inp_mux = (m_inp_mux & 0x700) | (data ^ 0xff);
	m_led_pwm->write_mx(~data);
}

u8 gk2000_state::p4_r()
{
	// P40-P47: multiplexed inputs
	u8 data = 0;

	// read buttons
	for (int i = 0; i < 3; i++)
		if (BIT(m_inp_mux, i + 8))
			data |= m_inputs[i]->read();

	// read chessboard
	for (int i = 0; i < 8; i++)
		if (BIT(m_inp_mux, i))
			data |= m_board->read_rank(i);

	return ~data;
}

void gk2000_state::p5_w(u8 data)
{
	// P50: speaker out
	m_dac->write(data & 1);

	// P51,P52: led select
	m_led_pwm->write_my(~data >> 1 & 3);

	// P53-P55: input mux (buttons)
	m_inp_mux = (m_inp_mux & 0xff) | (~data << 5 & 0x700);
}



/*******************************************************************************
    Input Ports
*******************************************************************************/

static INPUT_PORTS_START( gk2000 )
	PORT_START("IN.0")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_N) PORT_NAME("New Game")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_P) PORT_NAME("Position")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_L) PORT_NAME("Level")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_O) PORT_NAME("Option")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_I) PORT_NAME("Info")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_T) PORT_NAME("Take Back")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_DEL) PORT_CODE(KEYCODE_BACKSPACE) PORT_NAME("Clear")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_ENTER) PORT_CODE(KEYCODE_ENTER_PAD) PORT_NAME("Enter")

	PORT_START("IN.1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_6) PORT_CODE(KEYCODE_6_PAD) PORT_NAME("Pawn")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_5) PORT_CODE(KEYCODE_5_PAD) PORT_NAME("Knight")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_4) PORT_CODE(KEYCODE_4_PAD) PORT_NAME("Bishop")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_3) PORT_CODE(KEYCODE_3_PAD) PORT_NAME("Rook")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_2) PORT_CODE(KEYCODE_2_PAD) PORT_NAME("Queen")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_1) PORT_CODE(KEYCODE_1_PAD) PORT_NAME("King")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_W) PORT_CODE(KEYCODE_LEFT) PORT_NAME("White / Left")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_B) PORT_CODE(KEYCODE_RIGHT) PORT_NAME("Black / Right")

	PORT_START("IN.2")
	PORT_CONFNAME( 0xff, 0x02, "CPU Frequency" ) PORT_CHANGED_MEMBER(DEVICE_SELF, gk2000_state, change_cpu_freq, 0) // factory set
	PORT_CONFSETTING(    0x00, "14MHz (Travel Champion 2080)" )
	PORT_CONFSETTING(    0x02, "20MHz (GK 2000)" )

	PORT_START("IN.3")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_G) PORT_CHANGED_MEMBER(DEVICE_SELF, gk2000_state, go_button, 0) PORT_NAME("Go / Stop")
	PORT_BIT(0xef, IP_ACTIVE_HIGH, IPT_UNUSED)
INPUT_PORTS_END



/*******************************************************************************
    Machine Configs
*******************************************************************************/

void gk2000_state::gk2000(machine_config &config)
{
	// basic machine hardware
	H8323(config, m_maincpu, 20_MHz_XTAL);
	m_maincpu->nvram_enable_backup(true);
	m_maincpu->standby_cb().set(m_maincpu, FUNC(h8323_device::nvram_set_battery));
	m_maincpu->standby_cb().append(FUNC(gk2000_state::standby));
	m_maincpu->write_port1().set(FUNC(gk2000_state::lcd_segs_w<0>));
	m_maincpu->write_port2().set(FUNC(gk2000_state::p2_w));
	m_maincpu->write_port3().set(FUNC(gk2000_state::lcd_segs_w<1>));
	m_maincpu->read_port4().set(FUNC(gk2000_state::p4_r));
	m_maincpu->read_port5().set_constant(0xff);
	m_maincpu->write_port5().set(FUNC(gk2000_state::p5_w));
	m_maincpu->read_port6().set_ioport("IN.3").invert();
	m_maincpu->write_port6().set(FUNC(gk2000_state::lcd_com_w));
	m_maincpu->write_port7().set(FUNC(gk2000_state::lcd_segs_w<2>));

	SENSORBOARD(config, m_board).set_type(sensorboard_device::BUTTONS);
	m_board->init_cb().set(m_board, FUNC(sensorboard_device::preset_chess));
	m_board->set_delay(attotime::from_msec(150));
	//m_board->set_nvram_enable(true);

	// video hardware
	PWM_DISPLAY(config, m_lcd_pwm).set_size(2, 24);
	m_lcd_pwm->output_x().set(FUNC(gk2000_state::lcd_pwm_w));

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_SVG));
	screen.set_refresh_hz(60);
	screen.set_size(1920/5, 804/5);
	screen.set_visarea_full();

	PWM_DISPLAY(config, m_led_pwm).set_size(2, 8);
	config.set_default_layout(layout_saitek_gk2000);

	// sound hardware
	SPEAKER(config, "speaker").front_center();
	DAC_1BIT(config, m_dac).add_route(ALL_OUTPUTS, "speaker", 0.25);
}



/*******************************************************************************
    ROM Definitions
*******************************************************************************/

ROM_START( gk2000 )
	ROM_REGION( 0x4000, "maincpu", 0 )
	ROM_LOAD("92_saitek_86071220x12_3238a13p.u1", 0x0000, 0x4000, CRC(2059399c) SHA1(d99d5f86b80565e6017b19ef3f330112ac1ce685) )

	ROM_REGION( 68501, "screen", 0 )
	ROM_LOAD("gk2000.svg", 0, 68501, CRC(80554c49) SHA1(88f06ec8f403eaaf7cbce4cc84807b5742ce7108) )
ROM_END

} // anonymous namespace



/*******************************************************************************
    Drivers
*******************************************************************************/

//    YEAR  NAME    PARENT  COMPAT  MACHINE  INPUT   CLASS         INIT        COMPANY, FULLNAME, FLAGS
SYST( 1992, gk2000, 0,      0,      gk2000,  gk2000, gk2000_state, empty_init, "Saitek", "Kasparov GK 2000", MACHINE_SUPPORTS_SAVE )
