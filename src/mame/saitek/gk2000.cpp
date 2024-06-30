// license:BSD-3-Clause
// copyright-holders:hap
// thanks-to:Sean Riddle, Berger
/*******************************************************************************

Saitek GK 2000 / Centurion

These chess computers all have the same I/O and fit in the same driver. The chess
engine is by Frans Morsch.

TODO:
- is the H8/3212 V03 version on the same hardware?
- versions with the A20 ROM that don't officially support the extra options on
  the 2nd row, can still access them when turning the computer on by simultaneously
  pressing the Go/Stop button with the Option button. This doesn't work on MAME,
  something with MCU standby stabilisation maybe?
- it does a cold boot at every reset, so nvram won't work properly unless MAME
  adds some kind of auxillary autosave state feature at power-off

================================================================================

Saitek GK 2000 family

Hardware notes:

GK 2000 (H8/323 version):
- PCB label: ST12-PE-009 REV1
- Hitachi H8/323 MCU, 20MHz XTAL
- LCD with 5 7segs and custom segments
- piezo, 16 LEDs, button sensors chessboard

Saitek GK 2100 is on the same hardware, but has a H8/325 instead of H8/323.
Travel Champion 2100 has the same MCU.

H8/323 A13 MCU is used in:
- Saitek GK 2000 (86071220X12)
- Saitek Travel Champion 2080 (86071220X12)
- Saitek Mephisto Mythos (86142221X34)
- Tandy (Radio Shack) Mega 2050X (86071221X12)
- Tandy (Radio Shack) Master 2200X (suspected)

Travel Champion 2080 and Tandy Mega 2050X are 14MHz instead of 20MHz.

================================================================================

Saitek Centurion family

This is the program with the infamous H8 bug, not named after the MCU, but after
the H8 square. The piece on H8 is moved immediately, regardless of playing level,
often resulting in blunders.

Hardware notes:

Centurion / Cougar:
- PCB label: ST42C/39A LOGIC PCB, SAITEK LTD, PN 51A125-01113 VER 3.0,
  SCH ST42C-PE-000 REV 1.0, SCH ST39A-PE-002 VER 1.0
- Hitachi H8/3214 MCU, configuration diodes for XTAL a.o.
- same LCD and board hardware as GK 2000

Mephisto Explorer Pro:
- PCB label: 51CT12-01002, REV1.0 (smaller PCB)
- same MCU as Cougar, 16MHz XTAL
- LCD layout is slightly different, symbols are on the right side

For test mode, hold Enter after cold boot during the LCD test. It will say "TST",
press Enter again to see the diode configuration setting.

S = Studies button (if absent, this input functions as new game)
t = Teach mode (press a Symbol key after pressing Level)
L = LEDs enabled
n = 0/2/4/6 10+n MHz XTAL
b = Playing mode options on the 2nd row

All known chess computers with the A20 ROM have LEDs, and the S/t/b options are
either configured as either St, or b. So, none of them that have the Studies
button and teach mode officially support the extra options and vice versa.

H8/3214 A20 MCU is used in:
- Saitek Centurion (config: StL0_)
- Saitek Cosmos (config: __L0b)
- Saitek Cougar (config: __L6b)
- Saitek Mephisto Chess Challenger (config: StL0_)
- Saitek Mephisto Explorer Pro (config: StL6_)
- Saitek Mephisto Expert Travel Chess (config: __L0b)
- Saitek Mephisto Mystery (config: __L2b)

*******************************************************************************/

#include "emu.h"

#include "cpu/h8/h83217.h"
#include "cpu/h8/h8325.h"
#include "machine/sensorboard.h"
#include "sound/dac.h"
#include "video/pwm.h"

#include "screen.h"
#include "speaker.h"

// internal artwork
#include "saitek_centurion.lh"
#include "saitek_cougar.lh"
#include "saitek_gk2000.lh"
#include "saitek_gk2100.lh"


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

	template <typename T> void cpu_config(T &maincpu);
	void shared(machine_config &config);
	void gk2000(machine_config &config);
	void gk2100(machine_config &config);
	void centurion(machine_config &config);
	void cougar(machine_config &config);

	DECLARE_INPUT_CHANGED_MEMBER(go_button);
	DECLARE_INPUT_CHANGED_MEMBER(gk2000_change_cpu_freq);
	DECLARE_INPUT_CHANGED_MEMBER(centurion_change_cpu_freq);

protected:
	virtual void machine_start() override;

private:
	// devices/pointers
	required_device<h8_device> m_maincpu;
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

INPUT_CHANGED_MEMBER(gk2000_state::gk2000_change_cpu_freq)
{
	// only 20MHz and 14MHz versions are known to exist, but the software supports others
	static const int xm[9] = { 8, 20, 24, 28, 32, -1, -1, -1, 14 }; // XTAL in MHz (-1 is invalid)
	int mhz = xm[(count_leading_zeros_32(bitswap<8>(newval,0,1,2,3,4,5,6,7)) - 24) % 9];

	if (mhz > 0)
		m_maincpu->set_unscaled_clock(mhz * 1'000'000);
}

INPUT_CHANGED_MEMBER(gk2000_state::centurion_change_cpu_freq)
{
	// 14MHz version doesn't exist, but the software supports it
	static const XTAL freq[4] = { 12_MHz_XTAL, 16_MHz_XTAL, 14_MHz_XTAL, 10_MHz_XTAL };
	m_maincpu->set_unscaled_clock(freq[newval & 3]);
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
	PORT_CONFNAME( 0xff, 0x02, "CPU Frequency" ) PORT_CHANGED_MEMBER(DEVICE_SELF, gk2000_state, gk2000_change_cpu_freq, 0) // factory set
	PORT_CONFSETTING(    0x01, "8MHz (unofficial)" )
	PORT_CONFSETTING(    0x00, "14MHz (Travel Champion 2080)" )
	PORT_CONFSETTING(    0x02, "20MHz (GK 2000, GK 2100)" )
	PORT_CONFSETTING(    0x04, "24MHz (unofficial)" )
	PORT_CONFSETTING(    0x08, "28MHz (unofficial)" )
	PORT_CONFSETTING(    0x10, "32MHz (unofficial)" )

	PORT_START("IN.3")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_G) PORT_CHANGED_MEMBER(DEVICE_SELF, gk2000_state, go_button, 0) PORT_NAME("Go / Stop")
	PORT_BIT(0xef, IP_ACTIVE_HIGH, IPT_UNUSED)
INPUT_PORTS_END

static INPUT_PORTS_START( gk2100 )
	PORT_INCLUDE( gk2000 )

	PORT_MODIFY("IN.1")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_W) PORT_CODE(KEYCODE_MINUS) PORT_CODE(KEYCODE_MINUS_PAD) PORT_NAME("White / -")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_B) PORT_CODE(KEYCODE_EQUALS) PORT_CODE(KEYCODE_PLUS_PAD) PORT_NAME("Black / +")
INPUT_PORTS_END

static INPUT_PORTS_START( cougar )
	PORT_INCLUDE( gk2000 )

	PORT_MODIFY("IN.2") // configuration diodes
	PORT_BIT(0x27, 0x00, IPT_CUSTOM) // __Lnb
	PORT_CONFNAME( 0x18, 0x08, "CPU Frequency" ) PORT_CHANGED_MEMBER(DEVICE_SELF, gk2000_state, centurion_change_cpu_freq, 0)
	PORT_CONFSETTING(    0x18, "10MHz (Centurion, Cosmos)" )
	PORT_CONFSETTING(    0x00, "12MHz (Mephisto Mystery)" )
	PORT_CONFSETTING(    0x10, "14MHz (unofficial)" )
	PORT_CONFSETTING(    0x08, "16MHz (Cougar, Mephisto Explorer Pro)" )
	PORT_BIT(0xc0, IP_ACTIVE_HIGH, IPT_UNUSED)
INPUT_PORTS_END

static INPUT_PORTS_START( centurion )
	PORT_INCLUDE( cougar )

	PORT_MODIFY("IN.0")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_S) PORT_NAME("Studies")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_DEL) PORT_CODE(KEYCODE_BACKSPACE) PORT_CODE(KEYCODE_F1) PORT_NAME("Clear")   // combine for NEW GAME
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_ENTER) PORT_CODE(KEYCODE_ENTER_PAD) PORT_CODE(KEYCODE_F1) PORT_NAME("Enter") // "

	PORT_MODIFY("IN.2") // change defaults
	PORT_BIT(0x27, 0x23, IPT_CUSTOM) // StLn_
	PORT_CONFNAME( 0x18, 0x18, "CPU Frequency" ) PORT_CHANGED_MEMBER(DEVICE_SELF, gk2000_state, centurion_change_cpu_freq, 0)
	PORT_CONFSETTING(    0x18, "10MHz (Centurion, Cosmos)" )
	PORT_CONFSETTING(    0x00, "12MHz (Mephisto Mystery)" )
	PORT_CONFSETTING(    0x10, "14MHz (unofficial)" )
	PORT_CONFSETTING(    0x08, "16MHz (Cougar, Mephisto Explorer Pro)" )
INPUT_PORTS_END



/*******************************************************************************
    Machine Configs
*******************************************************************************/

template <typename T>
void gk2000_state::cpu_config(T &maincpu)
{
	maincpu.nvram_enable_backup(true);
	maincpu.standby_cb().set(maincpu, FUNC(T::nvram_set_battery));
	maincpu.standby_cb().append(FUNC(gk2000_state::standby));
	maincpu.write_port1().set(FUNC(gk2000_state::lcd_segs_w<0>));
	maincpu.write_port2().set(FUNC(gk2000_state::p2_w));
	maincpu.write_port3().set(FUNC(gk2000_state::lcd_segs_w<1>));
	maincpu.read_port4().set(FUNC(gk2000_state::p4_r));
	maincpu.read_port5().set_constant(0xff);
	maincpu.write_port5().set(FUNC(gk2000_state::p5_w));
	maincpu.read_port6().set_ioport("IN.3").invert();
	maincpu.write_port6().set(FUNC(gk2000_state::lcd_com_w));
	maincpu.write_port7().set(FUNC(gk2000_state::lcd_segs_w<2>));
}

void gk2000_state::shared(machine_config &config)
{
	// basic machine hardware
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

void gk2000_state::gk2000(machine_config &config)
{
	H8323(config, m_maincpu, 20_MHz_XTAL);
	cpu_config<h8323_device>(downcast<h8323_device &>(*m_maincpu));

	shared(config);
}

void gk2000_state::gk2100(machine_config &config)
{
	H8325(config, m_maincpu, 20_MHz_XTAL);
	cpu_config<h8325_device>(downcast<h8325_device &>(*m_maincpu));

	shared(config);

	config.set_default_layout(layout_saitek_gk2100);
}

void gk2000_state::centurion(machine_config &config)
{
	H83214(config, m_maincpu, 10_MHz_XTAL);
	cpu_config<h83214_device>(downcast<h83214_device &>(*m_maincpu));

	shared(config);

	config.set_default_layout(layout_saitek_centurion);
}

void gk2000_state::cougar(machine_config &config)
{
	centurion(config);
	m_maincpu->set_clock(16_MHz_XTAL);

	config.set_default_layout(layout_saitek_cougar);
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

ROM_START( gk2100 )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD("94_saitek_86100150110_3258b40p.u1", 0x0000, 0x8000, CRC(33823df6) SHA1(df528bbbf5eed985d05ced07fcb8f1cfb91a9f1b) )

	ROM_REGION( 68501, "screen", 0 )
	ROM_LOAD("gk2000.svg", 0, 68501, CRC(80554c49) SHA1(88f06ec8f403eaaf7cbce4cc84807b5742ce7108) )
ROM_END

ROM_START( centurion )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD("98_saitek_86171400305_hd6433214a20p.u1", 0x0000, 0x8000, CRC(31e35d22) SHA1(92cc3d90fc4e33f9634c0229fdb339dd0d8c5133) )

	ROM_REGION( 68501, "screen", 0 )
	ROM_LOAD("gk2000.svg", 0, 68501, CRC(80554c49) SHA1(88f06ec8f403eaaf7cbce4cc84807b5742ce7108) )
ROM_END

#define rom_cougar rom_centurion

} // anonymous namespace



/*******************************************************************************
    Drivers
*******************************************************************************/

//    YEAR  NAME       PARENT     COMPAT  MACHINE     INPUT      CLASS         INIT        COMPANY, FULLNAME, FLAGS
SYST( 1992, gk2000,    0,         0,      gk2000,     gk2000,    gk2000_state, empty_init, "Saitek", "Kasparov GK 2000", MACHINE_SUPPORTS_SAVE )

SYST( 1994, gk2100,    0,         0,      gk2100,     gk2100,    gk2000_state, empty_init, "Saitek", "Kasparov GK 2100", MACHINE_SUPPORTS_SAVE )

SYST( 1998, centurion, 0,         0,      centurion,  centurion, gk2000_state, empty_init, "Saitek", "Kasparov Centurion", MACHINE_SUPPORTS_SAVE )
SYST( 1998, cougar,    centurion, 0,      cougar,     cougar,    gk2000_state, empty_init, "Saitek", "Kasparov Cougar", MACHINE_SUPPORTS_SAVE )
