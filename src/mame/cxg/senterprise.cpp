// license:BSD-3-Clause
// copyright-holders:hap
// thanks-to:Sean Riddle, Berger
/*******************************************************************************

CXG Super Enterprise

The chess engine is Kaare Danielsen's Enterprise program. It's the 16KB 'sequel'
to LogiChess used in Enterprise "S" (emulated in saitek/companion2.cpp).

NOTE: It triggers an NMI when the power switch is changed from ON to SAVE.
If this is not done, NVRAM won't save properly.

TODO:
- if/when MAME supports an exit callback, hook up power-off switch to that

Hardware notes:

Super Crown:
- PCB label: CXG 218-600-001
- Hitachi HD6301Y0P (mode 2), 8MHz XTAL
- 2KB battery-backed RAM (HM6116LP-3)
- chessboard buttons, 24 LEDs, piezo

Super Enterprise (model 210.C):
- PCB label: 210C 600-002
- Sanyo LC7580, same LCDs as Sphinx Galaxy
- rest is same as above

210 MCU is used in:
- CXG Super Enterprise (model 210, black/brown/blue)
- CXG Advanced Star Chess (model 211)
- CXG Super Crown (model 218, black/brown)
- Mephisto Merlin 16K (H+G brand Super Crown)

210C MCU is used in:
- CXG Super Enterprise (model 210.C)
- CXG Sphinx Titan (model 270, suspected)

*******************************************************************************/

#include "emu.h"

#include "cpu/m6800/m6801.h"
#include "machine/nvram.h"
#include "machine/sensorboard.h"
#include "sound/dac.h"
#include "video/lc7580.h"
#include "video/pwm.h"

#include "speaker.h"

// internal artwork
#include "cxg_senterprise.lh"
#include "cxg_senterprisec.lh"


namespace {

// model 210 / shared

class senterp_state : public driver_device
{
public:
	senterp_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_board(*this, "board"),
		m_display(*this, "display"),
		m_dac(*this, "dac"),
		m_inputs(*this, "IN.%u", 0)
	{ }

	DECLARE_INPUT_CHANGED_MEMBER(power_off);

	void senterp(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	// devices/pointers
	required_device<hd6301y0_cpu_device> m_maincpu;
	required_device<sensorboard_device> m_board;
	required_device<pwm_display_device> m_display;
	required_device<dac_1bit_device> m_dac;
	required_ioport_array<3> m_inputs;

	emu_timer *m_standbytimer;
	u8 m_inp_mux = 0;

	void main_map(address_map &map) ATTR_COLD;

	// I/O handlers
	u8 input1_r();
	u8 input2_r();
	void leds_w(u8 data);
	void mux_w(u8 data);

	TIMER_CALLBACK_MEMBER(set_standby);
};

void senterp_state::machine_start()
{
	m_standbytimer = timer_alloc(FUNC(senterp_state::set_standby), this);

	// register for savestates
	save_item(NAME(m_inp_mux));
}


// model 210.C

class senterpc_state : public senterp_state
{
public:
	senterpc_state(const machine_config &mconfig, device_type type, const char *tag) :
		senterp_state(mconfig, type, tag),
		m_lcd(*this, "lcd"),
		m_out_digit(*this, "digit%u", 0U),
		m_out_lcd(*this, "s%u.%u", 0U, 0U)
	{ }

	void senterpc(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	required_device<lc7580_device> m_lcd;
	output_finder<8> m_out_digit;
	output_finder<2, 52> m_out_lcd;

	void lcd_output_w(offs_t offset, u64 data);
	void lcd_w(u8 data);
};

void senterpc_state::machine_start()
{
	senterp_state::machine_start();

	// resolve handlers
	m_out_digit.resolve();
	m_out_lcd.resolve();
}



/*******************************************************************************
    Power
*******************************************************************************/

void senterp_state::machine_reset()
{
	m_maincpu->set_input_line(INPUT_LINE_NMI, CLEAR_LINE);
	m_maincpu->set_input_line(M6801_STBY_LINE, CLEAR_LINE);
}

TIMER_CALLBACK_MEMBER(senterp_state::set_standby)
{
	m_maincpu->set_input_line(M6801_STBY_LINE, ASSERT_LINE);
}

INPUT_CHANGED_MEMBER(senterp_state::power_off)
{
	if (newval && !m_maincpu->standby())
	{
		// NMI when power switch is set to SAVE, followed by STBY (internal or STBY pin)
		m_maincpu->set_input_line(INPUT_LINE_NMI, ASSERT_LINE);
		m_standbytimer->adjust(attotime::from_msec(10));
	}
}



/*******************************************************************************
    I/O
*******************************************************************************/

// common

u8 senterp_state::input1_r()
{
	u8 data = 0;

	// P20,P21: read buttons
	for (int i = 0; i < 2; i++)
		if (m_inp_mux & m_inputs[i]->read())
			data |= 1 << i;

	// P26,P27: freq sel (senterp)
	data |= m_inputs[2]->read();
	return ~data;
}

u8 senterp_state::input2_r()
{
	u8 data = 0;

	// P50-P57: read chessboard
	for (int i = 0; i < 8; i++)
		if (BIT(m_inp_mux, i))
			data |= m_board->read_rank(i);

	return ~data;
}

void senterp_state::leds_w(u8 data)
{
	// P23-P25: led select
	m_display->write_my(~data >> 3 & 7);
}

void senterp_state::mux_w(u8 data)
{
	// P60-P67: input mux, led data
	m_inp_mux = ~data;
	m_display->write_mx(m_inp_mux);
}


// LCD (senterpc)

void senterpc_state::lcd_output_w(offs_t offset, u64 data)
{
	// output individual segments
	for (int i = 0; i < 52; i++)
		m_out_lcd[offset][i] = BIT(data, i);

	// unscramble digit 7segs
	static const u8 seg2digit[4*7] =
	{
		0x03, 0x04, 0x00, 0x40, 0x41, 0x02, 0x42,
		0x05, 0x06, 0x07, 0x48, 0x44, 0x45, 0x46,
		0x0c, 0x0d, 0x0b, 0x0a, 0x4a, 0x4c, 0x4b,
		0x0e, 0x0f, 0x10, 0x50, 0x4d, 0x4e, 0x4f
	};

	for (int i = 0; i < 8; i++)
	{
		u8 digit = 0;
		for (int seg = 0; seg < 7; seg++)
		{
			u8 bit = seg2digit[7 * (i & 3) + seg] + 26 * (i >> 2);
			digit |= m_out_lcd[BIT(bit, 6)][bit & 0x3f] << seg;
		}
		m_out_digit[i] = digit;
	}
}

void senterpc_state::lcd_w(u8 data)
{
	// P22: LC7580 DATA
	// P26: LC7580 CLK
	// P27: LC7580 CE
	m_lcd->data_w(BIT(data, 2));
	m_lcd->clk_w(BIT(data, 6));
	m_lcd->ce_w(BIT(data, 7));

	// P22+P27: piezo
	m_dac->write(BIT(data, 2) & BIT(~data, 7));
}



/*******************************************************************************
    Address Maps
*******************************************************************************/

void senterp_state::main_map(address_map &map)
{
	map(0x4000, 0x47ff).mirror(0x3800).ram().share("nvram");
}



/*******************************************************************************
    Input Ports
*******************************************************************************/

static INPUT_PORTS_START( senterp )
	PORT_START("IN.0")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_O) PORT_NAME("Move")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_T) PORT_NAME("Take Back")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_6) PORT_CODE(KEYCODE_6_PAD) PORT_NAME("Pawn")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_5) PORT_CODE(KEYCODE_5_PAD) PORT_NAME("Knight")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_4) PORT_CODE(KEYCODE_4_PAD) PORT_NAME("Bishop")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_3) PORT_CODE(KEYCODE_3_PAD) PORT_NAME("Rook")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_2) PORT_CODE(KEYCODE_2_PAD) PORT_NAME("Queen")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_1) PORT_CODE(KEYCODE_1_PAD) PORT_NAME("King")

	PORT_START("IN.1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_M) PORT_NAME("Multi Move")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_L) PORT_NAME("Level")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_H) PORT_NAME("Hint")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_Y) PORT_NAME("Time")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_I) PORT_NAME("Library/Clearboard")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_S) PORT_CODE(KEYCODE_C) PORT_NAME("Sound/Color")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_E) PORT_NAME("Enter Position")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_N) PORT_NAME("New Game")

	PORT_START("IN.2")
	PORT_BIT(0x3f, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_CUSTOM) // freq sel
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_CUSTOM) // "

	PORT_START("POWER") // needs to be triggered for nvram to work
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_POWER_OFF) PORT_CHANGED_MEMBER(DEVICE_SELF, senterp_state, power_off, 0)
INPUT_PORTS_END

static INPUT_PORTS_START( senterpc )
	PORT_INCLUDE( senterp )

	PORT_MODIFY("IN.1")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_R) PORT_NAME("Replay")

	PORT_MODIFY("IN.2")
	PORT_BIT(0xc0, IP_ACTIVE_HIGH, IPT_UNUSED)
INPUT_PORTS_END



/*******************************************************************************
    Machine Configs
*******************************************************************************/

void senterp_state::senterp(machine_config &config)
{
	// basic machine hardware
	HD6301Y0(config, m_maincpu, 8_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &senterp_state::main_map);
	m_maincpu->nvram_enable_backup(true);
	m_maincpu->standby_cb().set(m_maincpu, FUNC(hd6301y0_cpu_device::nvram_set_battery));
	m_maincpu->standby_cb().append([this](int state) { if (state) m_display->clear(); });
	m_maincpu->in_p2_cb().set(FUNC(senterp_state::input1_r));
	m_maincpu->out_p2_cb().set(FUNC(senterp_state::leds_w));
	m_maincpu->out_p2_cb().append(m_dac, FUNC(dac_1bit_device::write)).bit(2);
	m_maincpu->in_p5_cb().set(FUNC(senterp_state::input2_r));
	m_maincpu->out_p6_cb().set(FUNC(senterp_state::mux_w));

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	SENSORBOARD(config, m_board).set_type(sensorboard_device::BUTTONS);
	m_board->init_cb().set(m_board, FUNC(sensorboard_device::preset_chess));
	m_board->set_delay(attotime::from_msec(150));
	m_board->set_nvram_enable(true);

	// video hardware
	PWM_DISPLAY(config, m_display).set_size(3, 8);
	config.set_default_layout(layout_cxg_senterprise);

	// sound hardware
	SPEAKER(config, "speaker").front_center();
	DAC_1BIT(config, m_dac).add_route(ALL_OUTPUTS, "speaker", 0.25);
}

void senterpc_state::senterpc(machine_config &config)
{
	senterp(config);

	// basic machine hardware
	m_maincpu->standby_cb().append(m_lcd, FUNC(lc7580_device::inh_w));
	m_maincpu->out_p2_cb().set(FUNC(senterpc_state::leds_w));
	m_maincpu->out_p2_cb().append(FUNC(senterpc_state::lcd_w));

	// video hardware
	LC7580(config, m_lcd, 0);
	m_lcd->write_segs().set(FUNC(senterpc_state::lcd_output_w));

	config.set_default_layout(layout_cxg_senterprisec);
}



/*******************************************************************************
    ROM Definitions
*******************************************************************************/

ROM_START( senterp )
	ROM_REGION( 0x4000, "maincpu", 0 )
	ROM_LOAD("1985_210_newcrest_hd6301y0a14p", 0x0000, 0x4000, CRC(871719c8) SHA1(8c0f5bef2573b9cbebe87be3a899fec6308603be) )
ROM_END

ROM_START( senterpc )
	ROM_REGION( 0x4000, "maincpu", 0 )
	ROM_LOAD("1986_210c_cxg_systems_hd6301y0b27p", 0x0000, 0x4000, CRC(5bb67dd6) SHA1(753c33643a5c45e899d0f4743d3ccf7a0728bd48) )
ROM_END

} // anonymous namespace



/*******************************************************************************
    Drivers
*******************************************************************************/

//    YEAR  NAME      PARENT   COMPAT  MACHINE   INPUT     CLASS           INIT        COMPANY, FULLNAME, FLAGS
SYST( 1986, senterp,  0,       0,      senterp,  senterp,  senterp_state,  empty_init, "CXG Systems / Newcrest Technology / LogiSoft", "Super Enterprise (model 210)", MACHINE_SUPPORTS_SAVE )
SYST( 1986, senterpc, senterp, 0,      senterpc, senterpc, senterpc_state, empty_init, "CXG Systems / Newcrest Technology / LogiSoft", "Super Enterprise (model 210.C)", MACHINE_SUPPORTS_SAVE )
