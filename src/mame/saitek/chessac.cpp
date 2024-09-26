// license:BSD-3-Clause
// copyright-holders:hap
// thanks-to:Sean Riddle, Berger
/*******************************************************************************

Saitek Kasparov Chess Academy / Mephisto Schachakademie (both were later rebranded
to Mephisto Talking Chess Academy)

The chess engine is by Frans Morsch, similar to the one in GK 2000. Other features,
such as the speech and tutorial lessons, were supposedly added by Craig Barnes.

Hardware notes:
- PCB label: SCH RT33-PE-041 Rev 3.0
- Hitachi H8/3214 MCU, 16MHz XTAL
- same LCD as GK 2000
- OKI MSM6588 ADPCM Recorder @ 4MHz, small daughterboard with 4MB ROM under epoxy
- 8*8 LEDs, button sensors chessboard

The German version has 2 epoxy blobs (4MB and 2MB) on the daughterboard.

TODO:
- it does a cold boot at every reset, so nvram won't work properly unless MAME
  adds some kind of auxillary autosave state feature at power-off
- does a French speech version exist?

*******************************************************************************/

#include "emu.h"

#include "cpu/h8/h83217.h"
#include "machine/sensorboard.h"
#include "sound/okim6588.h"
#include "video/pwm.h"

#include "screen.h"
#include "speaker.h"

// internal artwork
#include "mephisto_schachak.lh"
#include "saitek_chessac.lh"


namespace {

class chessac_state : public driver_device
{
public:
	chessac_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_board(*this, "board"),
		m_adpcm_rom(*this, "adpcm"),
		m_led_pwm(*this, "led_pwm"),
		m_lcd_pwm(*this, "lcd_pwm"),
		m_okim6588(*this, "okim6588"),
		m_inputs(*this, "IN.%u", 0),
		m_out_lcd(*this, "s%u.%u", 0U, 0U)
	{ }

	void chessac(machine_config &config);
	void schachak(machine_config &config);

	DECLARE_INPUT_CHANGED_MEMBER(go_button);

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	// devices/pointers
	required_device<h83214_device> m_maincpu;
	required_device<sensorboard_device> m_board;
	required_region_ptr<u8> m_adpcm_rom;
	required_device<pwm_display_device> m_led_pwm;
	required_device<pwm_display_device> m_lcd_pwm;
	required_device<okim6588_device> m_okim6588;
	required_ioport_array<3> m_inputs;
	output_finder<2, 24> m_out_lcd;

	u16 m_inp_mux = 0;
	u32 m_lcd_segs = 0;
	u8 m_lcd_com = 0;
	u8 m_led_select = 0;
	u8 m_led_data = 0;
	u32 m_adpcm_address = 0;

	u8 m_port3 = 0xff;
	u8 m_port5 = 0xff;
	u8 m_port7 = 0xff;

	// I/O handlers
	void lcd_pwm_w(offs_t offset, u8 data);
	void update_lcd();
	template <int N> void lcd_segs_w(u8 data);

	void standby(int state);

	void update_leds();
	void update_adpcm_address();

	void p3_w(u8 data);
	u8 p4_r();
	void p5_w(u8 data);
	void p6_w(u8 data);
	u8 p7_r();
	void p7_w(u8 data);
};

void chessac_state::machine_start()
{
	m_out_lcd.resolve();

	// register for savestates
	save_item(NAME(m_inp_mux));
	save_item(NAME(m_lcd_segs));
	save_item(NAME(m_lcd_com));
	save_item(NAME(m_led_select));
	save_item(NAME(m_led_data));
	save_item(NAME(m_adpcm_address));
	save_item(NAME(m_port3));
	save_item(NAME(m_port5));
	save_item(NAME(m_port7));
}



/*******************************************************************************
    I/O
*******************************************************************************/

// power

void chessac_state::standby(int state)
{
	// clear display
	if (state)
	{
		m_lcd_pwm->clear();
		m_led_pwm->clear();
	}
}

INPUT_CHANGED_MEMBER(chessac_state::go_button)
{
	m_maincpu->set_input_line(INPUT_LINE_IRQ0, newval ? ASSERT_LINE : CLEAR_LINE);
}


// LCD

void chessac_state::lcd_pwm_w(offs_t offset, u8 data)
{
	m_out_lcd[offset & 0x3f][offset >> 6] = data;
}

void chessac_state::update_lcd()
{
	// LCD latch from P1x
	if (m_port5 & 1)
		m_lcd_segs = (m_lcd_segs << 8 & 0xff0000) | (m_lcd_segs & 0xffff);

	u32 lcd_segs = bitswap<24>(m_lcd_segs,11,18,19,20,21,12,13,14,15,22,23,0,1,2,3,4,5,6,7,16,17,8,9,10);

	for (int i = 0; i < 2; i++)
	{
		// LCD common is analog (voltage level)
		const u8 com = population_count_32(m_lcd_com >> (i * 2) & 3);
		const u32 data = (com == 0) ? lcd_segs : (com == 2) ? ~lcd_segs : 0;
		m_lcd_pwm->write_row(i, data);
	}
}

template <int N>
void chessac_state::lcd_segs_w(u8 data)
{
	// P1x, P2x: LCD segments
	const u8 shift = 8 * N;
	m_lcd_segs = (m_lcd_segs & ~(0xff << shift)) | (data << shift);
	update_lcd();
}


// misc

void chessac_state::update_leds()
{
	if (m_port5 & 0x10)
		m_led_select = ~m_port7;
	if (m_port5 & 0x20)
		m_led_data = m_port7;

	m_led_pwm->matrix(m_led_select, m_led_data);
}

void chessac_state::update_adpcm_address()
{
	for (int i = 0; i < 3; i++)
		if (BIT(m_port3, i))
		{
			const u8 shift = 8 * i;
			m_adpcm_address = (m_adpcm_address & ~(0xff << shift)) | (m_port7 << shift);
		}
}

void chessac_state::p3_w(u8 data)
{
	// P37: MSM6588 RESET
	if (m_port3 & 0x80 && ~data & 0x80)
		m_okim6588->reset();

	// P34: MSM6588 RD
	// P35: MSM6588 WR
	// P36: MSM6588 CE
	if ((data & 0xc0) == 0x80 && m_port3 & 0x20 && ~data & 0x20)
		m_okim6588->data_w(m_port7);

	// P33: ADPCM ROM CE
	// P30-P32: enable ADPCM ROM address latches
	m_port3 = data;
	update_adpcm_address();
}

u8 chessac_state::p4_r()
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
			data |= m_board->read_file(i, true);

	return ~data;
}

void chessac_state::p5_w(u8 data)
{
	// P50: enable LCD latch
	m_port5 = data;
	update_lcd();

	// P51: ext power (no need to emulate it)
	// P52,P53: N/C

	// P54,P55: enable LED latches
	update_leds();
}

void chessac_state::p6_w(u8 data)
{
	// P60-P63: LCD common
	m_lcd_com = data & 0xf;
	update_lcd();

	// P65,P66: input mux (buttons)
	m_inp_mux = (m_inp_mux & 0xff) | (~data << 3 & 0x300);
}

u8 chessac_state::p7_r()
{
	u8 data = 0xff;

	// P70-P77: read ADPCM ROM
	if (~m_port3 & 8)
		data &= m_adpcm_rom[m_adpcm_address & (m_adpcm_rom.bytes() - 1)];

	// P70-P73: read MSM6588 status
	if ((m_port3 & 0xf0) == 0xa0)
		data &= (m_okim6588->data_r() | 0xf0);

	return data;
}

void chessac_state::p7_w(u8 data)
{
	// P70-P77: input mux (chessboard)
	m_inp_mux = (m_inp_mux & 0x300) | (data ^ 0xff);

	// also data for LED latches, ADPCM address latches, and MSM6588
	m_port7 = data;
	update_leds();
	update_adpcm_address();
}



/*******************************************************************************
    Input Ports
*******************************************************************************/

static INPUT_PORTS_START( chessac )
	PORT_START("IN.0")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_Y) PORT_CODE(KEYCODE_F1) PORT_NAME("Yes") // combine for NEW GAME
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_N) PORT_CODE(KEYCODE_F1) PORT_NAME("No")  // "
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_P) PORT_NAME("Position")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_H) PORT_NAME("Hint / Info")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_RIGHT) PORT_CODE(KEYCODE_B) PORT_NAME("Fwd / Black")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_6) PORT_CODE(KEYCODE_6_PAD) PORT_NAME("Pawn")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_3) PORT_CODE(KEYCODE_3_PAD) PORT_NAME("Rook")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_1) PORT_CODE(KEYCODE_1_PAD) PORT_NAME("King")

	PORT_START("IN.1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_O) PORT_NAME("Option")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_L) PORT_NAME("Level")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_T) PORT_NAME("Tutorial")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_S) PORT_NAME("Say Again")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_LEFT) PORT_CODE(KEYCODE_W) PORT_NAME("Back / White")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_5) PORT_CODE(KEYCODE_5_PAD) PORT_NAME("Knight")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_4) PORT_CODE(KEYCODE_4_PAD) PORT_NAME("Bishop")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_2) PORT_CODE(KEYCODE_2_PAD) PORT_NAME("Queen")

	PORT_START("IN.2")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_G) PORT_CHANGED_MEMBER(DEVICE_SELF, chessac_state, go_button, 0) PORT_NAME("Go / Stop")
	PORT_BIT(0xef, IP_ACTIVE_HIGH, IPT_UNUSED)
INPUT_PORTS_END



/*******************************************************************************
    Machine Configs
*******************************************************************************/

void chessac_state::chessac(machine_config &config)
{
	// basic machine hardware
	H83214(config, m_maincpu, 16_MHz_XTAL);
	m_maincpu->nvram_enable_backup(true);
	m_maincpu->standby_cb().set(m_maincpu, FUNC(h83214_device::nvram_set_battery));
	m_maincpu->standby_cb().append(FUNC(chessac_state::standby));
	m_maincpu->write_port1().set(FUNC(chessac_state::lcd_segs_w<1>));
	m_maincpu->write_port2().set(FUNC(chessac_state::lcd_segs_w<0>));
	m_maincpu->write_port3().set(FUNC(chessac_state::p3_w));
	m_maincpu->read_port4().set(FUNC(chessac_state::p4_r));
	m_maincpu->write_port5().set(FUNC(chessac_state::p5_w));
	m_maincpu->read_port6().set_ioport("IN.2").invert();
	m_maincpu->write_port6().set(FUNC(chessac_state::p6_w));
	m_maincpu->read_port7().set(FUNC(chessac_state::p7_r));
	m_maincpu->write_port7().set(FUNC(chessac_state::p7_w));

	SENSORBOARD(config, m_board).set_type(sensorboard_device::BUTTONS);
	m_board->init_cb().set(m_board, FUNC(sensorboard_device::preset_chess));
	m_board->set_delay(attotime::from_msec(150));
	//m_board->set_nvram_enable(true);

	// video hardware
	PWM_DISPLAY(config, m_lcd_pwm).set_size(2, 24);
	m_lcd_pwm->output_x().set(FUNC(chessac_state::lcd_pwm_w));

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_SVG));
	screen.set_refresh_hz(60);
	screen.set_size(1920/5, 804/5);
	screen.set_visarea_full();

	PWM_DISPLAY(config, m_led_pwm).set_size(8, 8);
	config.set_default_layout(layout_saitek_chessac);

	// sound hardware
	SPEAKER(config, "speaker").front_center();

	OKIM6588(config, m_okim6588, 4_MHz_XTAL).add_route(ALL_OUTPUTS, "speaker", 0.5);
	m_okim6588->write_mon().set_inputline(m_maincpu, INPUT_LINE_NMI).invert();
	m_okim6588->set_mcum_pin(1);
}

void chessac_state::schachak(machine_config &config)
{
	chessac(config);
	config.set_default_layout(layout_mephisto_schachak);
}



/*******************************************************************************
    ROM Definitions
*******************************************************************************/

ROM_START( chessac )
	ROM_REGION16_BE( 0x8000, "maincpu", 0 )
	ROM_LOAD("97_saitek_86165400831_hd6433214a08f.u1", 0x0000, 0x8000, CRC(29d06d6a) SHA1(08b6f4093b240b0a34d9da67c9acffc576ba1d2d) )

	ROM_REGION( 0x400000, "adpcm", 0 )
	ROM_LOAD("adpcm.u10", 0x000000, 0x400000, CRC(73d9650c) SHA1(ecf3bd72fc954528fa72f64eac91e225d11150c6) ) // no label

	ROM_REGION( 68501, "screen", 0 )
	ROM_LOAD("gk2000.svg", 0, 68501, CRC(80554c49) SHA1(88f06ec8f403eaaf7cbce4cc84807b5742ce7108) )
ROM_END

ROM_START( schachak )
	ROM_REGION16_BE( 0x8000, "maincpu", 0 )
	ROM_LOAD("97_saitek_86165400831_hd6433214a08f.u1", 0x0000, 0x8000, CRC(29d06d6a) SHA1(08b6f4093b240b0a34d9da67c9acffc576ba1d2d) )

	ROM_REGION( 0x800000, "adpcm", 0 )
	ROM_LOAD("adpcm.u2", 0x000000, 0x400000, CRC(21608a97) SHA1(42f16cab961f1c53a649a7d630bb96304208e850) ) // no label
	ROM_LOAD("adpcm.u1", 0x400000, 0x200000, CRC(03ed8eaf) SHA1(44c6de8414b943044dcb8c20e43b40701d1ebc85) ) // "
	ROM_RELOAD(          0x600000, 0x200000 )

	ROM_REGION( 68501, "screen", 0 )
	ROM_LOAD("gk2000.svg", 0, 68501, CRC(80554c49) SHA1(88f06ec8f403eaaf7cbce4cc84807b5742ce7108) )
ROM_END

} // anonymous namespace



/*******************************************************************************
    Drivers
*******************************************************************************/

//    YEAR  NAME      PARENT   COMPAT  MACHINE   INPUT    CLASS          INIT        COMPANY, FULLNAME, FLAGS
SYST( 1997, chessac,  0,       0,      chessac,  chessac, chessac_state, empty_init, "Saitek", "Kasparov Chess Academy", MACHINE_SUPPORTS_SAVE )
SYST( 1997, schachak, chessac, 0,      schachak, chessac, chessac_state, empty_init, "Saitek", "Mephisto Schachakademie", MACHINE_SUPPORTS_SAVE )
