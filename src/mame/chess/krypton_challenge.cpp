// license:BSD-3-Clause
// copyright-holders:hap
// thanks-to:Berger
/*******************************************************************************

Krypton Challenge (model 5T-938)

It was manufactured by Timorite, Ltd. (Eric White's company), the chess engine is
by Gyula Horváth, similar to the one in CXG Sphinx Legend.

To start a new game, keep holding the NEW GAME button until the display says HELLO.

TODO:
- CXG Sphinx Legend may be on the same hardware? if so, move driver to cxg folder
- is Krypton a product brand, or a company alias for the Chinese factory behind it?
- it does a cold boot at every reset, so nvram won't work properly unless MAME
  adds some kind of auxillary autosave state feature at power-off

Hardware notes:

Systema Challenge:
- PCB label: LCD CHESS 938, JAN.1994, REV.1, EB-093801-01
- Hitachi H8/325 or H8/3256 MCU, 20MHz XTAL
- LCD with 5 7segs and custom segments
- piezo, 16 LEDs, button sensors chessboard

Krypton Regency (1995 version):
- no main PCB label, LED PCB: TW933-E2, 996
- Hitachi H8/325 MCU, 20MHz XTAL
- rest is same as Challenge

Krypton Regency (1997 version):
- PCB label: COPYRIGHT 1997 TIMORITE LTD., EB-93301-01 REV. 2.0
- Hitachi H8/3256 MCU, 20MHz XTAL
- no LEDs (cost reduction)
- rest is same as Challenge

The H8/3256 MCU has Krypton Regency's model number (933) on the label, though
it was also used in Challenge.

H8/325 A95 MCU is used in:
- Krypton (or Systema) Challenge (black or gray, 1994 version)
- Krypton Comet (suspected)
- Krypton Regency (1995 version, with LEDs)
- Excalibur Legend II (Excalibur brand Challenge)

H8/3256 A26 MCU is used in:
- Krypton (or Systema) Challenge (1996 version)
- Krypton Regency (1997 version, without LEDs)
- Excalibur Avenger (suspected, Excalibur brand Comet, with newer MCU)
- Excalibur Legend III (suspected, Excalibur brand Regency)

*******************************************************************************/

#include "emu.h"

#include "cpu/h8/h8325.h"
#include "machine/sensorboard.h"
#include "sound/dac.h"
#include "video/pwm.h"

#include "screen.h"
#include "speaker.h"

// internal artwork
#include "krypton_challenge.lh"


namespace {

class kchal_state : public driver_device
{
public:
	kchal_state(const machine_config &mconfig, device_type type, const char *tag) :
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
	void kchal(machine_config &config);
	void kchala(machine_config &config);

	DECLARE_INPUT_CHANGED_MEMBER(in1_changed) { update_irq2(); }

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	// devices/pointers
	required_device<h8_device> m_maincpu;
	required_device<sensorboard_device> m_board;
	required_device<pwm_display_device> m_led_pwm;
	required_device<pwm_display_device> m_lcd_pwm;
	required_device<dac_1bit_device> m_dac;
	required_ioport_array<3> m_inputs;
	output_finder<2, 24> m_out_lcd;

	u16 m_inp_mux = 0;
	u8 m_inp_mux2 = 0;
	u32 m_lcd_segs = 0;
	u8 m_lcd_com = 0;

	// I/O handlers
	void lcd_pwm_w(offs_t offset, u8 data);
	void update_lcd();
	template <int N> void lcd_segs_w(u8 data);

	void standby(int state);
	int update_irq2();

	void p2_w(u8 data);
	void p5_w(offs_t offset, u8 data, u8 mem_mask);
	u8 p6_r();
	void p6_w(offs_t offset, u8 data, u8 mem_mask);
	u8 p7_r();
	void p7_w(u8 data);
};

void kchal_state::machine_start()
{
	m_out_lcd.resolve();

	// register for savestates
	save_item(NAME(m_inp_mux));
	save_item(NAME(m_inp_mux2));
	save_item(NAME(m_lcd_segs));
	save_item(NAME(m_lcd_com));
}



/*******************************************************************************
    I/O
*******************************************************************************/

// power

void kchal_state::standby(int state)
{
	// clear display
	if (state)
	{
		m_lcd_pwm->clear();
		m_led_pwm->clear();
	}
}

int kchal_state::update_irq2()
{
	// 2nd button row is tied to IRQ2 (used for on/off button)
	int state = (m_inp_mux2 & m_inputs[1]->read()) ? ASSERT_LINE : CLEAR_LINE;
	m_maincpu->set_input_line(INPUT_LINE_IRQ2, state);

	return state;
}


// LCD

void kchal_state::lcd_pwm_w(offs_t offset, u8 data)
{
	m_out_lcd[offset & 0x3f][offset >> 6] = data;
}

void kchal_state::update_lcd()
{
	u32 lcd_segs = bitswap<24>(m_lcd_segs,1,0, 15,14,13,12,11,10,9,8, 16,17,23,22,21,20,19,18, 25,26,27,28,29,31);

	for (int i = 0; i < 2; i++)
	{
		// LCD common is 0/1/Hi-Z
		const u32 data = BIT(m_lcd_com, i + 2) ? (BIT(m_lcd_com, i) ? ~lcd_segs : lcd_segs) : 0;
		m_lcd_pwm->write_row(i, data);
	}
}

template <int N>
void kchal_state::lcd_segs_w(u8 data)
{
	// P1x, P3x, P4x, P6x: LCD segments
	const u8 shift = 8 * N;
	m_lcd_segs = (m_lcd_segs & ~(0xff << shift)) | (data << shift);
	update_lcd();
}


// misc

void kchal_state::p2_w(u8 data)
{
	// P20-P27: input mux (chessboard), LED data
	m_inp_mux = (m_inp_mux & 0x300) | (data ^ 0xff);
	m_led_pwm->write_mx(~data);
}

void kchal_state::p5_w(offs_t offset, u8 data, u8 mem_mask)
{
	// P50: LCD common 2
	m_lcd_com = (m_lcd_com & 5) | (data << 1 & 2) | (mem_mask << 3 & 8);
	update_lcd();

	// P51: speaker out
	m_dac->write(BIT(~data, 1));

	// P52: input mux part
	m_inp_mux = (m_inp_mux & 0x2ff) | (BIT(~data, 2) << 8);

	// P54,P55: LED select
	m_led_pwm->write_my(~data >> 4 & 3);
}

u8 kchal_state::p6_r()
{
	// P65: battery status
	u8 data = m_inputs[2]->read() << 5 & 0x20;

	// P66: IRQ2 pin
	if (!machine().side_effects_disabled() && update_irq2())
		data |= 0x40;

	return ~data;
}

void kchal_state::p6_w(offs_t offset, u8 data, u8 mem_mask)
{
	// P60,P61: LCD segs
	lcd_segs_w<0>(data & 3);

	// P62: LCD common 1
	m_lcd_com = (m_lcd_com & 0xa) | (BIT(data, 2)) | (mem_mask & 4);
	update_lcd();

	// P66: input mux part
	m_inp_mux = (m_inp_mux & 0x1ff) | (BIT(~data, 6) << 9);
}

u8 kchal_state::p7_r()
{
	// P70-P77: multiplexed inputs
	u8 data = 0;

	// read buttons
	for (int i = 0; i < 2; i++)
		if (BIT(m_inp_mux, i + 8))
			data |= m_inputs[i]->read();

	// read chessboard
	for (int i = 0; i < 8; i++)
		if (BIT(m_inp_mux, i))
			data |= m_board->read_file(i ^ 7);

	return ~data;
}

void kchal_state::p7_w(u8 data)
{
	// P70-P77: input mux (other way around)
	m_inp_mux2 = ~data;
	update_irq2();
}



/*******************************************************************************
    Input Ports
*******************************************************************************/

#define PORT_CHANGED_IN1() \
	PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(kchal_state::in1_changed), 0)

static INPUT_PORTS_START( kchal )
	PORT_START("IN.0")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_5) PORT_CODE(KEYCODE_5_PAD) PORT_NAME("Knight")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_2) PORT_CODE(KEYCODE_2_PAD) PORT_NAME("Queen")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_S) PORT_NAME("Set-Up / Features")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_L) PORT_NAME("Level")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_F) PORT_NAME("Step Forward")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_U) PORT_NAME("Multi-Move / Analysis")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_3) PORT_CODE(KEYCODE_3_PAD) PORT_NAME("Rook")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_6) PORT_CODE(KEYCODE_6_PAD) PORT_NAME("Pawn")

	PORT_START("IN.1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CHANGED_IN1() PORT_CODE(KEYCODE_F1) PORT_NAME("On / Off")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CHANGED_IN1() PORT_CODE(KEYCODE_1) PORT_CODE(KEYCODE_1_PAD) PORT_NAME("King")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CHANGED_IN1() PORT_CODE(KEYCODE_M) PORT_NAME("Move / Swap Board")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CHANGED_IN1() PORT_CODE(KEYCODE_O) PORT_NAME("Sound / Style")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CHANGED_IN1() PORT_CODE(KEYCODE_T) PORT_NAME("Take Back")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CHANGED_IN1() PORT_CODE(KEYCODE_H) PORT_NAME("Hint / Info")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CHANGED_IN1() PORT_CODE(KEYCODE_4) PORT_CODE(KEYCODE_4_PAD) PORT_NAME("Bishop")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CHANGED_IN1() PORT_CODE(KEYCODE_N) PORT_NAME("New Game / Clear Board")

	PORT_START("IN.2")
	PORT_CONFNAME( 0x01, 0x01, "Battery Status" )
	PORT_CONFSETTING(    0x00, "Low" )
	PORT_CONFSETTING(    0x01, DEF_STR( Normal ) )
INPUT_PORTS_END



/*******************************************************************************
    Machine Configs
*******************************************************************************/

template <typename T>
void kchal_state::cpu_config(T &maincpu)
{
	maincpu.nvram_enable_backup(true);
	maincpu.standby_cb().set(maincpu, FUNC(T::nvram_set_battery));
	maincpu.standby_cb().append(FUNC(kchal_state::standby));
	maincpu.write_port1().set(FUNC(kchal_state::lcd_segs_w<2>));
	maincpu.read_port2().set_constant(0xef); // hardware config?
	maincpu.write_port2().set(FUNC(kchal_state::p2_w));
	maincpu.write_port3().set(FUNC(kchal_state::lcd_segs_w<1>));
	maincpu.write_port4().set(FUNC(kchal_state::lcd_segs_w<3>));
	maincpu.read_port5().set_constant(0xff);
	maincpu.write_port5().set(FUNC(kchal_state::p5_w));
	maincpu.read_port6().set(FUNC(kchal_state::p6_r));
	maincpu.write_port6().set(FUNC(kchal_state::p6_w));
	maincpu.read_port7().set(FUNC(kchal_state::p7_r));
	maincpu.write_port7().set(FUNC(kchal_state::p7_w));
}

void kchal_state::shared(machine_config &config)
{
	SENSORBOARD(config, m_board).set_type(sensorboard_device::BUTTONS);
	m_board->init_cb().set(m_board, FUNC(sensorboard_device::preset_chess));
	m_board->set_delay(attotime::from_msec(150));
	//m_board->set_nvram_enable(true);

	// video hardware
	PWM_DISPLAY(config, m_lcd_pwm).set_size(2, 24);
	m_lcd_pwm->output_x().set(FUNC(kchal_state::lcd_pwm_w));

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_SVG));
	screen.set_refresh_hz(60);
	screen.set_size(1920/5, 697/5);
	screen.set_visarea_full();

	PWM_DISPLAY(config, m_led_pwm).set_size(2, 8);
	config.set_default_layout(layout_krypton_challenge);

	// sound hardware
	SPEAKER(config, "speaker").front_center();
	DAC_1BIT(config, m_dac).add_route(ALL_OUTPUTS, "speaker", 0.25);
}

void kchal_state::kchal(machine_config &config)
{
	H83256(config, m_maincpu, 20_MHz_XTAL);
	cpu_config<h83256_device>(downcast<h83256_device &>(*m_maincpu));

	shared(config);
}

void kchal_state::kchala(machine_config &config)
{
	H8325(config, m_maincpu, 20_MHz_XTAL);
	cpu_config<h8325_device>(downcast<h8325_device &>(*m_maincpu));

	shared(config);
}



/*******************************************************************************
    ROM Definitions
*******************************************************************************/

ROM_START( kchal )
	ROM_REGION16_BE( 0xc000, "maincpu", 0 )
	ROM_LOAD("1996_933_timorite_hd6433256a26p.ic1", 0x0000, 0xc000, CRC(72eb3f2b) SHA1(30e4166e351210475cf9709b0feb717d9d3ac747) )

	ROM_REGION( 109652, "screen", 0 )
	ROM_LOAD("kchal.svg", 0, 109652, CRC(6840c49e) SHA1(a9c91143c5bea5ab41fe323e719da4a46ab9d631) )
ROM_END

ROM_START( kchala )
	ROM_REGION16_BE( 0x8000, "maincpu", 0 )
	ROM_LOAD("1993_vil_v938_hd6433258a95p.ic1", 0x0000, 0x8000, CRC(9277d7d4) SHA1(0ba5129846c11bb7bf02dade1b934e21c45316c8) )

	ROM_REGION( 109652, "screen", 0 )
	ROM_LOAD("kchal.svg", 0, 109652, CRC(6840c49e) SHA1(a9c91143c5bea5ab41fe323e719da4a46ab9d631) )
ROM_END

} // anonymous namespace



/*******************************************************************************
    Drivers
*******************************************************************************/

//    YEAR  NAME    PARENT  COMPAT  MACHINE  INPUT  CLASS        INIT        COMPANY, FULLNAME, FLAGS
SYST( 1996, kchal,  0,      0,      kchal,   kchal, kchal_state, empty_init, "Krypton / Timorite", "Challenge (1996 version)", MACHINE_SUPPORTS_SAVE )
SYST( 1994, kchala, kchal,  0,      kchala,  kchal, kchal_state, empty_init, "Krypton / Timorite", "Challenge (1994 version)", MACHINE_SUPPORTS_SAVE )
