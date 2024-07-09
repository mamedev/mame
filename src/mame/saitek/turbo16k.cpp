// license:BSD-3-Clause
// copyright-holders:hap
// thanks-to:Sean Riddle
/*******************************************************************************

SciSys Turbo 16K family

These chesscomputers are all on similar hardware. The chess engine is by Julio
Kaplan and Craig Barnes.

NOTE: Before exiting MAME, press the STOP button to turn the power off. Otherwise,
NVRAM won't save properly.

TODO:
- dump/add other MCU revisions, SX8 for tmate/conquist is known to exist
- what is t1850's official title? "1850 Deluxe Table Chess" is from the back of
  the computer. The manual can't make up its mind and says "1850 Chess Computer",
  "1850 Chess: 16 Level Program", or "1850 Sensory Chess Game". The box disagrees
  and says "Sixteen Level Computerized Chess"

Hardware notes:
- Hitachi HD6301Y0P/F MCU, 8MHz or 12MHz (LC osc, no XTAL)
- 16 board LEDs (can be tri-color), 7 or 8 status LEDs
- buttons sensor board, piezo

As seen on PCB and MCU labels, the Tandy (Radio Shack) versions are programmed and
manufactured by SciSys, presumably under contract by Tandy.

Turbo 16K/S-24K and Conquistador have tri-color LEDs, and two LCD chess clocks.
As hinted by the strange clock start value of 1:00 instead of 0:00, the clocks
are actually digital watch components. The IC under epoxy is an OKI MSM5001N.

I/O for LEDs and buttons is scrambled a bit for Team-Mate and Conquistador, the
base hardware remains the same.

SX4(A) program is used in:
- Tandy (Radio Shack) 1850 60-2199 (8MHz, ST4A-PE-015 PCB)
- no known SciSys chesscomputers, to distinguish: this program has 16 playing
  levels and SX5A has 17

SX5(A) program is used in:
- SciSys Companion III (8MHz, ST4B-PE-007 PCB)
- SciSys Express 16K (8MHz, SH5-PE-009 PCB)
- SciSys Astral (12MHz, SW4-PE-010 PCB)
- SciSys Turbo 16K (12MHz, ST5-PE-023 PCB)
- Tandy (Radio Shack) 1850 60-2201A (8MHz, ST5A-PE-002 PCB)
- Mephisto Monaco (H+G brand Express 16K)

SX5A 6301Y0A97F (QFP) has the same ROM contents as 6301Y0A96P.

SX8(A) program is used in:
- Saitek Team-Mate aka Team-Mate Advanced Trainer (8MHz, ST8B-PE-017 PCB)
- Saitek Cavalier aka Portable Advanced Trainer (suspected, 8MHz, ? PCB)
- Saitek Conquistador (12MHz, ST8-PE-021 PCB)

*******************************************************************************/

#include "emu.h"

#include "cpu/m6800/m6801.h"
#include "machine/msm5001n.h"
#include "machine/sensorboard.h"
#include "sound/spkrdev.h"
#include "video/pwm.h"

#include "speaker.h"

// internal artwork
#include "saitek_companion3.lh"
#include "saitek_conquistador.lh"
#include "saitek_teammate.lh"
#include "saitek_turbo16k.lh"
#include "t1850.lh"


namespace {

// Turbo 16K / shared

class turbo16k_state : public driver_device
{
public:
	turbo16k_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_board(*this, "board"),
		m_lcd_clock(*this, "clock%u", 0),
		m_display(*this, "display"),
		m_dac(*this, "dac"),
		m_inputs(*this, "IN.%u", 0),
		m_lcd_digits(*this, "ldigit%u.%u", 0U, 0U),
		m_lcd_colon(*this, "lc%u", 0U)
	{ }

	void compan3(machine_config &config);
	void turbo16k(machine_config &config);
	void t1850(machine_config &config);

	DECLARE_INPUT_CHANGED_MEMBER(go_button);
	DECLARE_INPUT_CHANGED_MEMBER(change_cpu_freq);

protected:
	virtual void machine_start() override;

	// devices/pointers
	required_device<hd6301y0_cpu_device> m_maincpu;
	required_device<sensorboard_device> m_board;
	optional_device_array<msm5001n_device, 2> m_lcd_clock;
	required_device<pwm_display_device> m_display;
	required_device<speaker_sound_device> m_dac;
	required_ioport_array<2> m_inputs;
	output_finder<2, 4> m_lcd_digits;
	output_finder<2> m_lcd_colon;

	u8 m_inp_mux = 0;
	u8 m_led_select = 0;
	u8 m_led_data[2] = { };
	u16 m_lcd_data[4] = { };

	// I/O handlers
	template<int N> void lcd_output_w(offs_t offset, u16 data);
	void lcd_enable(u8 enable);
	void update_display();

	template <int N> void leds1_w(u8 data);
	template <int N> void leds2_w(u8 data);
	void p2l_w(u8 data);
	virtual void p3_w(u8 data);
	virtual u8 p5_r();
	virtual u8 p6_r();
	void p6_w(u8 data);
};

void turbo16k_state::machine_start()
{
	m_lcd_digits.resolve();
	m_lcd_colon.resolve();

	// register for savestates
	save_item(NAME(m_inp_mux));
	save_item(NAME(m_led_select));
	save_item(NAME(m_led_data));
	save_item(NAME(m_lcd_data));
}

INPUT_CHANGED_MEMBER(turbo16k_state::change_cpu_freq)
{
	// 4MHz and 16MHz versions don't exist, but the software supports it
	static const u32 freq[4] = { 4'000'000, 8'000'000, 12'000'000, 16'000'000 };
	m_maincpu->set_unscaled_clock(freq[bitswap<2>(newval,4,0)]);
}


// Conquistador

class conquist_state : public turbo16k_state
{
public:
	conquist_state(const machine_config &mconfig, device_type type, const char *tag) :
		turbo16k_state(mconfig, type, tag)
	{ }

	void conquist(machine_config &config);
	void tmate(machine_config &config);

protected:
	virtual void p3_w(u8 data) override;
	virtual u8 p5_r() override;
	virtual u8 p6_r() override;
};



/*******************************************************************************
    I/O
*******************************************************************************/

// common

INPUT_CHANGED_MEMBER(turbo16k_state::go_button)
{
	// standby check actually comes from P70 high-impedance state
	if (newval && m_maincpu->standby())
		m_maincpu->pulse_input_line(INPUT_LINE_RESET, attotime::zero);
}

void turbo16k_state::update_display()
{
	m_display->matrix_partial(0, 3, m_led_select << 1 | 1, m_led_data[1] << 8 | m_led_data[0]);
}

template <int N>
void turbo16k_state::leds1_w(u8 data)
{
	// P10-P17, P40-P47: board leds
	m_led_data[N] = ~data;
	update_display();
}

template <int N>
void turbo16k_state::leds2_w(u8 data)
{
	// P2x, P6x, P7x: status leds (direct)
	m_display->write_row(N + 3, ~data);
}

void turbo16k_state::p3_w(u8 data)
{
	// P30-P37: input mux
	m_inp_mux = ~data;
}

u8 turbo16k_state::p5_r()
{
	u8 data = 0;

	// P50-P57: read chessboard
	for (int i = 0; i < 8; i++)
		if (BIT(m_inp_mux, i))
			data |= m_board->read_file(i, true);

	return ~data;
}

u8 turbo16k_state::p6_r()
{
	u8 data = 0;

	// P60,P61: read buttons
	for (int i = 0; i < 2; i++)
		if (m_inp_mux & m_inputs[i]->read())
			data |= 1 << i;

	return ~data;
}

void turbo16k_state::p6_w(u8 data)
{
	// P63,P64: status leds
	leds2_w<1>(data);

	// P65,P66: red/green led select
	m_led_select = ~data >> 5 & 3;
	update_display();

	// P67: speaker out
	m_dac->level_w(BIT(~data, 7));
}


// LCD

template<int N>
void turbo16k_state::lcd_output_w(offs_t offset, u16 data)
{
	m_lcd_data[N << 1 | (offset & 1)] = data;
	u32 segs = m_lcd_data[N << 1 | 1] << 11 | m_lcd_data[N << 1];

	// unscramble segments
	m_lcd_digits[N][0] = bitswap<7>(segs,12,2,13,17,11,0,1);
	m_lcd_digits[N][1] = bitswap<7>(segs,15,5,16,4,14,3,4);
	m_lcd_digits[N][2] = bitswap<7>(segs,19,9,20,21,18,7,8);
	m_lcd_digits[N][3] = BIT(segs, 10) ? 6 : 0;
	m_lcd_colon[N] = BIT(segs, 6);
}

void turbo16k_state::lcd_enable(u8 enable)
{
	// LCD XTAL can be disabled by software
	for (int i = 0; i < 2; i++)
		m_lcd_clock[i]->set_clock_scale(BIT(enable, i) ? 1.0 : 0.0);
}

void turbo16k_state::p2l_w(u8 data)
{
	// P24,P25: status leds
	leds2_w<0>(data);

	// P20,P21: LCD clocks enabled
	lcd_enable(~data & 3);

	// P26: LCD clocks power (both)
	m_lcd_clock[0]->power_w(BIT(data, 6));
	m_lcd_clock[1]->power_w(BIT(data, 6));
}


// conquist-specific

void conquist_state::p3_w(u8 data)
{
	// P30-P37: input mux (scrambled)
	m_inp_mux = bitswap<8>(~data,0,1,2,7,6,3,5,4);
}

u8 conquist_state::p5_r()
{
	// P50,P51: read buttons
	u8 data = turbo16k_state::p6_r() & 3;

	// P52-P57: read chessboard part
	data |= turbo16k_state::p5_r() << 2;
	return bitswap<8>(data,7,4,3,5,2,6,1,0);
}

u8 conquist_state::p6_r()
{
	// P60,P61: read chessboard part
	return bitswap<2>(turbo16k_state::p5_r(),6,7) | 0xfc;
}



/*******************************************************************************
    Input Ports
*******************************************************************************/

static INPUT_PORTS_START( turbo16k )
	PORT_START("IN.0")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_6) PORT_CODE(KEYCODE_6_PAD) PORT_NAME("Pawn")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_5) PORT_CODE(KEYCODE_5_PAD) PORT_NAME("Knight")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_4) PORT_CODE(KEYCODE_4_PAD) PORT_NAME("Bishop")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_3) PORT_CODE(KEYCODE_3_PAD) PORT_NAME("Rook")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_1) PORT_CODE(KEYCODE_1_PAD) PORT_NAME("King")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_C) PORT_NAME("Color")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_D) PORT_NAME("Sound")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_S) PORT_NAME("Stop")

	PORT_START("IN.1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_N) PORT_NAME("New Game")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_T) PORT_NAME("Take Back")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_U) PORT_NAME("Set Up")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_P) PORT_NAME("Play")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_2) PORT_CODE(KEYCODE_2_PAD) PORT_NAME("Queen")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_M) PORT_NAME("Multi Move")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_L) PORT_NAME("Level")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_I) PORT_NAME("Display Move")

	PORT_START("RESET")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_G) PORT_CHANGED_MEMBER(DEVICE_SELF, turbo16k_state, go_button, 0) PORT_NAME("Go")

	PORT_START("FREQ")
	PORT_CONFNAME( 0x88, 0x80, "CPU Frequency" ) PORT_CHANGED_MEMBER(DEVICE_SELF, turbo16k_state, change_cpu_freq, 0) // factory set
	PORT_CONFSETTING(    0x00, "4MHz (unofficial)" )
	PORT_CONFSETTING(    0x08, "8MHz (Companion III, Express 16K)" )
	PORT_CONFSETTING(    0x80, "12MHz (Turbo 16K, Astral)" )
	PORT_CONFSETTING(    0x88, "16MHz (unofficial)" )
	PORT_BIT(0x77, IP_ACTIVE_LOW, IPT_UNUSED)
INPUT_PORTS_END

static INPUT_PORTS_START( compan3 )
	PORT_INCLUDE( turbo16k )

	PORT_MODIFY("FREQ") // default to 8MHz
	PORT_CONFNAME( 0x88, 0x08, "CPU Frequency" ) PORT_CHANGED_MEMBER(DEVICE_SELF, turbo16k_state, change_cpu_freq, 0) // factory set
	PORT_CONFSETTING(    0x00, "4MHz (unofficial)" )
	PORT_CONFSETTING(    0x08, "8MHz (Companion III, Express 16K)" )
	PORT_CONFSETTING(    0x80, "12MHz (Turbo 16K, Astral)" )
	PORT_CONFSETTING(    0x88, "16MHz (unofficial)" )
INPUT_PORTS_END

static INPUT_PORTS_START( conquist )
	PORT_INCLUDE( turbo16k )

	PORT_MODIFY("IN.0")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_H) PORT_NAME("Coach")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_T) PORT_NAME("Take Back")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_2) PORT_CODE(KEYCODE_2_PAD) PORT_NAME("Queen")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_5) PORT_CODE(KEYCODE_5_PAD) PORT_NAME("Knight")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_3) PORT_CODE(KEYCODE_3_PAD) PORT_NAME("Rook")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_N) PORT_NAME("New Game")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_L) PORT_NAME("Level")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_C) PORT_NAME("Color")

	PORT_MODIFY("IN.1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_A) PORT_NAME("Non Auto")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_P) PORT_NAME("Play")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_1) PORT_CODE(KEYCODE_1_PAD) PORT_NAME("King")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_6) PORT_CODE(KEYCODE_6_PAD) PORT_NAME("Pawn")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_4) PORT_CODE(KEYCODE_4_PAD) PORT_NAME("Bishop")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_U) PORT_NAME("Set Up")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_S) PORT_NAME("Stop")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_I) PORT_NAME("Info")

	PORT_MODIFY("FREQ")
	PORT_CONFNAME( 0x88, 0x80, "CPU Frequency" ) PORT_CHANGED_MEMBER(DEVICE_SELF, turbo16k_state, change_cpu_freq, 0) // factory set
	PORT_CONFSETTING(    0x08, "8MHz (Team-Mate)" )
	PORT_CONFSETTING(    0x80, "12MHz (Conquistador)" )
INPUT_PORTS_END

static INPUT_PORTS_START( tmate )
	PORT_INCLUDE( conquist )

	PORT_MODIFY("FREQ") // default to 8MHz
	PORT_CONFNAME( 0x88, 0x08, "CPU Frequency" ) PORT_CHANGED_MEMBER(DEVICE_SELF, turbo16k_state, change_cpu_freq, 0) // factory set
	PORT_CONFSETTING(    0x08, "8MHz (Team-Mate)" )
	PORT_CONFSETTING(    0x80, "12MHz (Conquistador)" )
INPUT_PORTS_END

static INPUT_PORTS_START( t1850 )
	PORT_INCLUDE( compan3 )

	PORT_MODIFY("IN.0")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_S) PORT_NAME("Sound")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_R) PORT_NAME("Memory")

	PORT_MODIFY("IN.1")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_D) PORT_NAME("Display Move")

	PORT_MODIFY("RESET")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_O) PORT_CHANGED_MEMBER(DEVICE_SELF, turbo16k_state, go_button, 0) PORT_NAME("Power On")
INPUT_PORTS_END



/*******************************************************************************
    Machine Configs
*******************************************************************************/

void turbo16k_state::compan3(machine_config &config)
{
	// basic machine hardware
	HD6301Y0(config, m_maincpu, 8'000'000); // approximation, no XTAL
	m_maincpu->nvram_enable_backup(true);
	m_maincpu->standby_cb().set(m_maincpu, FUNC(hd6301y0_cpu_device::nvram_set_battery));
	m_maincpu->standby_cb().append([this](int state) { if (state) m_display->clear(); });
	m_maincpu->out_p1_cb().set(FUNC(turbo16k_state::leds1_w<0>));
	m_maincpu->in_p2_cb().set_ioport("FREQ");
	m_maincpu->in_p2_override_mask(0x88); // SX4A and SX5A rely on this
	m_maincpu->out_p2_cb().set(FUNC(turbo16k_state::leds2_w<0>));
	m_maincpu->out_p3_cb().set(FUNC(turbo16k_state::p3_w));
	m_maincpu->out_p4_cb().set(FUNC(turbo16k_state::leds1_w<1>));
	m_maincpu->in_p5_cb().set(FUNC(turbo16k_state::p5_r));
	m_maincpu->in_p6_cb().set(FUNC(turbo16k_state::p6_r));
	m_maincpu->out_p6_cb().set(FUNC(turbo16k_state::p6_w));
	m_maincpu->out_p7_cb().set(FUNC(turbo16k_state::leds2_w<2>));

	SENSORBOARD(config, m_board).set_type(sensorboard_device::BUTTONS);
	m_board->init_cb().set(m_board, FUNC(sensorboard_device::preset_chess));
	m_board->set_delay(attotime::from_msec(150));
	m_board->set_nvram_enable(true);

	// video hardware
	PWM_DISPLAY(config, m_display).set_size(3+3, 16);
	config.set_default_layout(layout_saitek_companion3);

	// sound hardware
	SPEAKER(config, "speaker").front_center();
	SPEAKER_SOUND(config, m_dac).add_route(ALL_OUTPUTS, "speaker", 0.25);
}

void turbo16k_state::turbo16k(machine_config &config)
{
	compan3(config);

	// basic machine hardware
	m_maincpu->set_clock(12'000'000);
	m_maincpu->standby_cb().append([this](int state) { if (state) lcd_enable(0); });
	m_maincpu->out_p2_cb().set(FUNC(turbo16k_state::p2l_w));

	MSM5001N(config, m_lcd_clock[0], 32.768_kHz_XTAL).write_segs().set(FUNC(turbo16k_state::lcd_output_w<0>));
	MSM5001N(config, m_lcd_clock[1], 32.768_kHz_XTAL).write_segs().set(FUNC(turbo16k_state::lcd_output_w<1>));

	config.set_default_layout(layout_saitek_turbo16k);
}

void conquist_state::conquist(machine_config &config)
{
	turbo16k(config);
	config.set_default_layout(layout_saitek_conquistador);
}

void conquist_state::tmate(machine_config &config)
{
	compan3(config);
	config.set_default_layout(layout_saitek_teammate);
}

void turbo16k_state::t1850(machine_config &config)
{
	compan3(config);
	config.set_default_layout(layout_t1850);
}



/*******************************************************************************
    ROM Definitions
*******************************************************************************/

ROM_START( turbo16k )
	ROM_REGION( 0x4000, "maincpu", 0 )
	ROM_LOAD("1986_sx5a_scisys_6301y0a96p.u1", 0x0000, 0x4000, CRC(65dd0626) SHA1(aa82242cb05f6c063430297a07a702d31c99a3ed) )
ROM_END

ROM_START( conquist )
	ROM_REGION( 0x4000, "maincpu", 0 )
	ROM_LOAD("1988_sx8a_saitek_31y0g84p.u1", 0x0000, 0x4000, CRC(083e0346) SHA1(32bd8609d16c885afbd5566c64330f84f3c46099) )
ROM_END

ROM_START( t1850 )
	ROM_REGION( 0x4000, "maincpu", 0 )
	ROM_LOAD("1985_sx4a_scisys_6301y0a13p.u1", 0x0000, 0x4000, CRC(6ba27399) SHA1(9bccd77eed416e92bcaa722e9ff3a4d6ba9946a1) )
ROM_END

#define rom_compan3 rom_turbo16k
#define rom_tmate rom_conquist

} // anonymous namespace



/*******************************************************************************
    Drivers
*******************************************************************************/

//    YEAR  NAME      PARENT    COMPAT  MACHINE   INPUT     CLASS           INIT        COMPANY, FULLNAME, FLAGS
SYST( 1986, turbo16k, 0,        0,      turbo16k, turbo16k, turbo16k_state, empty_init, "SciSys / Heuristic Software", "Turbo 16K", MACHINE_SUPPORTS_SAVE )
SYST( 1986, compan3,  turbo16k, 0,      compan3,  compan3,  turbo16k_state, empty_init, "SciSys / Heuristic Software", "Companion III", MACHINE_SUPPORTS_SAVE )

SYST( 1988, conquist, 0,        0,      conquist, conquist, conquist_state, empty_init, "Saitek / Heuristic Software", "Kasparov Conquistador", MACHINE_SUPPORTS_SAVE )
SYST( 1988, tmate,    conquist, 0,      tmate,    tmate,    conquist_state, empty_init, "Saitek / Heuristic Software", "Kasparov Team-Mate", MACHINE_SUPPORTS_SAVE )

SYST( 1986, t1850,    0,        0,      t1850,    t1850,    turbo16k_state, empty_init, "Tandy Corporation / SciSys / Heuristic Software", "1850 Deluxe Table Chess (model 60-2199)", MACHINE_SUPPORTS_SAVE )
