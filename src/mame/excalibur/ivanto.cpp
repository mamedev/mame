// license:BSD-3-Clause
// copyright-holders:hap
// thanks-to:Sean Riddle
/*******************************************************************************

Excalibur Ivan The Terrible (model 701E, H8/3256 version)

This is the first version, see ivant.cpp for the newer version. It was produced
in a factory owned by Eric White's company (ex-CXG), hence it's not that strange
that the LCD is the same as the one in CXG Sphinx Legend and Krypton Challenge.

Hardware notes:
- PCB label: EXCALIBUR ELECTRONICS, INC. 6/28/96, IVANT
- Hitachi H8/3256 MCU (mode 1), 20MHz XTAL
- 8-bit DAC (8L503 resistor array), KA8602 amplifier, 1MB ROM (custom label)
- LCD with 5 7segs and custom segments
- no LEDs, button sensors chessboard

The MCU used here is a HD6433256A33P from Excalibur Mirage, the internal ROM
was disabled. It runs at a higher frequency than the H8/3216 version, but
is actually a bit slower due to the H8/325 /2 clock divider.

TODO:
- it does a cold boot at every reset, so nvram won't work properly unless MAME
  adds some kind of auxillary autosave state feature at power-off

BTANB:
- speech sound is scratchy (worse than ivant), it has spikes here and there,
  verified with a digital capture, final (analog) output on the real thing
  sounds a bit better than MAME though

*******************************************************************************/

#include "emu.h"

#include "cpu/h8/h8325.h"
#include "machine/sensorboard.h"
#include "sound/dac.h"
#include "video/pwm.h"

#include "screen.h"
#include "speaker.h"

// internal artwork
#include "excal_ivanto.lh"


namespace {

class ivanto_state : public driver_device
{
public:
	ivanto_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_rombank(*this, "rombank"),
		m_board(*this, "board"),
		m_lcd_pwm(*this, "lcd_pwm"),
		m_dac(*this, "dac"),
		m_inputs(*this, "IN.%u", 0),
		m_out_lcd(*this, "s%u.%u", 0U, 0U)
	{ }

	void ivanto(machine_config &config);

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override { m_dac->write(0x80); }

private:
	// devices/pointers
	required_device<h83256_device> m_maincpu;
	required_memory_bank m_rombank;
	required_device<sensorboard_device> m_board;
	required_device<pwm_display_device> m_lcd_pwm;
	required_device<dac_8bit_r2r_device> m_dac;
	required_ioport_array<2> m_inputs;
	output_finder<2, 24> m_out_lcd;

	u16 m_inp_mux = 0;
	u32 m_latch = 0;
	u32 m_lcd_segs = 0;
	u8 m_lcd_com = 0;
	emu_timer *m_irqtimer;

	void main_map(address_map &map);

	// I/O handlers
	void lcd_pwm_w(offs_t offset, u8 data);
	void update_lcd();
	void lcd_com_w(offs_t offset, u8 data, u8 mem_mask);

	void latch_w(offs_t offset, u8 data);
	u8 read_inputs();

	TIMER_CALLBACK_MEMBER(update_irq) { read_inputs(); }

	u8 p4_r();
	void p4_w(u8 data);
	void p5_w(u8 data);
	u8 p6_r();
	void p6_w(u8 data);
};

void ivanto_state::machine_start()
{
	m_out_lcd.resolve();
	m_rombank->configure_entries(0, 64, memregion("rombank")->base(), 0x4000);

	// periodically check for interrupts
	m_irqtimer = timer_alloc(FUNC(ivanto_state::update_irq), this);
	attotime period = attotime::from_msec(1);
	m_irqtimer->adjust(period, 0, period);

	// register for savestates
	save_item(NAME(m_inp_mux));
	save_item(NAME(m_latch));
	save_item(NAME(m_lcd_segs));
	save_item(NAME(m_lcd_com));
}



/*******************************************************************************
    I/O
*******************************************************************************/

// LCD

void ivanto_state::lcd_pwm_w(offs_t offset, u8 data)
{
	m_out_lcd[offset & 0x3f][offset >> 6] = data;
}

void ivanto_state::update_lcd()
{
	for (int i = 0; i < 2; i++)
	{
		// LCD common is 0/1/Hi-Z
		const u32 data = BIT(m_lcd_com, i + 2) ? (BIT(m_lcd_com, i) ? ~m_lcd_segs : m_lcd_segs) : 0;
		m_lcd_pwm->write_row(i, data);
	}
}

void ivanto_state::lcd_com_w(offs_t offset, u8 data, u8 mem_mask)
{
	// P70,P71: LCD common
	m_lcd_com = (mem_mask << 2 & 0xc) | (data & 3);
	update_lcd();
}


// misc

void ivanto_state::latch_w(offs_t offset, u8 data)
{
	// a0-a2,d0-d3: 4*74259
	u32 mask = 1 << offset;
	for (int i = 0; i < 4; i++)
	{
		m_latch = (m_latch & ~mask) | (BIT(data, i) ? mask : 0);
		mask <<= 8;
	}

	// 74259(all) Q0,Q1: DAC data
	m_dac->write(bitswap<8>(m_latch,0,8,16,24,1,9,17,25));

	// 74259(all) Q2-Q7: LCD segments
	m_lcd_segs = 0;
	for (int i = 0; i < 4; i++)
		m_lcd_segs |= (m_latch >> (i * 8 + 2) & 0x3f) << (i * 6);

	update_lcd();

	// 74259(2) Q4-Q7, 74259(3) Q2-Q6: input mux low
	m_inp_mux = (m_inp_mux & 0x200) | (~m_latch >> 22 & 0x1f0) | (~m_latch >> 20 & 0xf);
	read_inputs();
}

u8 ivanto_state::read_inputs()
{
	u8 data = 0;

	// read buttons
	for (int i = 0; i < 2; i++)
		if (BIT(m_inp_mux, i + 8))
			data |= m_inputs[i]->read();

	// read chessboard
	for (int i = 0; i < 8; i++)
		if (BIT(m_inp_mux, i))
			data |= m_board->read_file(i, true);

	// P64-P66 are also IRQ pins (the ON button is IRQ0)
	if (!machine().side_effects_disabled())
		for (int i = 0; i < 3; i++)
			m_maincpu->set_input_line(INPUT_LINE_IRQ0 + i, BIT(data, i + 4) ? ASSERT_LINE : CLEAR_LINE);

	return ~data;
}

u8 ivanto_state::p4_r()
{
	// P47: multiplexed inputs high bit
	return (read_inputs() & 0x80) | 0x7f;
}

void ivanto_state::p4_w(u8 data)
{
	// P40-P45: ROM bank
	m_rombank->set_entry(~data & 0x3f);
}

void ivanto_state::p5_w(u8 data)
{
	// P54: KA8602 mute
	m_dac->set_output_gain(0, (data & 0x10) ? 0.0 : 1.0);

	// P55: input mux high bit
	m_inp_mux = (m_inp_mux & 0x1ff) | BIT(~data, 5) << 9;
	read_inputs();
}

u8 ivanto_state::p6_r()
{
	// P60-P66: multiplexed inputs part
	return read_inputs() | 0x80;
}



/*******************************************************************************
    Address Maps
*******************************************************************************/

void ivanto_state::main_map(address_map &map)
{
	map(0x0000, 0x0007).mirror(0xfff8).w(FUNC(ivanto_state::latch_w));
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0xbfff).bankr(m_rombank);
}



/*******************************************************************************
    Input Ports
*******************************************************************************/

static INPUT_PORTS_START( ivanto )
	PORT_START("IN.0")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_U) PORT_CODE(KEYCODE_LEFT) PORT_NAME("No / Left")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_R) PORT_NAME("Repeat")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_E) PORT_NAME("Mode")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_O) PORT_NAME("Option")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_N) PORT_NAME("New Game")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_B) PORT_NAME("Black / White")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_T) PORT_NAME("Take Back")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_Y) PORT_CODE(KEYCODE_RIGHT) PORT_NAME("Yes / Right")

	PORT_START("IN.1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_S) PORT_CODE(KEYCODE_1) PORT_CODE(KEYCODE_1_PAD) PORT_NAME("Set Up / King")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_V) PORT_CODE(KEYCODE_2) PORT_CODE(KEYCODE_2_PAD) PORT_NAME("Verify / Queen")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_I) PORT_CODE(KEYCODE_4) PORT_CODE(KEYCODE_4_PAD) PORT_NAME("Multi-Move / Bishop")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_F2) PORT_NAME("Off / Save")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_F1) PORT_CODE(KEYCODE_C) PORT_NAME("On / Clear")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_M) PORT_CODE(KEYCODE_6) PORT_CODE(KEYCODE_6_PAD) PORT_NAME("Move / Pawn")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_H) PORT_CODE(KEYCODE_5) PORT_CODE(KEYCODE_5_PAD) PORT_NAME("Hint / Knight")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_L) PORT_CODE(KEYCODE_3) PORT_CODE(KEYCODE_3_PAD) PORT_NAME("Level / Rook")

	PORT_START("BATT")
	PORT_CONFNAME( 0x08, 0x00, "Battery Status" )
	PORT_CONFSETTING(    0x08, "Low" )
	PORT_CONFSETTING(    0x00, DEF_STR( Normal ) )
	PORT_BIT(0xf7, IP_ACTIVE_HIGH, IPT_UNUSED)
INPUT_PORTS_END



/*******************************************************************************
    Machine Configs
*******************************************************************************/

void ivanto_state::ivanto(machine_config &config)
{
	// basic machine hardware
	H83256(config, m_maincpu, 20_MHz_XTAL);
	m_maincpu->set_mode(1);
	m_maincpu->set_addrmap(AS_PROGRAM, &ivanto_state::main_map);
	m_maincpu->nvram_enable_backup(true);
	m_maincpu->standby_cb().set(m_maincpu, FUNC(h83256_device::nvram_set_battery));
	m_maincpu->standby_cb().append([this](int state) { if (state) m_lcd_pwm->clear(); });
	m_maincpu->read_port4().set(FUNC(ivanto_state::p4_r));
	m_maincpu->write_port4().set(FUNC(ivanto_state::p4_w));
	m_maincpu->write_port5().set(FUNC(ivanto_state::p5_w));
	m_maincpu->read_port6().set(FUNC(ivanto_state::p6_r));
	m_maincpu->read_port7().set_ioport("BATT").invert();
	m_maincpu->write_port7().set(FUNC(ivanto_state::lcd_com_w));

	SENSORBOARD(config, m_board).set_type(sensorboard_device::BUTTONS);
	m_board->init_cb().set(m_board, FUNC(sensorboard_device::preset_chess));
	m_board->set_delay(attotime::from_msec(150));
	//m_board->set_nvram_enable(true);

	// video hardware
	PWM_DISPLAY(config, m_lcd_pwm).set_size(2, 24);
	m_lcd_pwm->output_x().set(FUNC(ivanto_state::lcd_pwm_w));

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_SVG));
	screen.set_refresh_hz(60);
	screen.set_size(1920/5, 697/5);
	screen.set_visarea_full();

	config.set_default_layout(layout_excal_ivanto);

	// sound hardware
	SPEAKER(config, "speaker").front_center();
	DAC_8BIT_R2R(config, m_dac).add_route(ALL_OUTPUTS, "speaker", 0.5);
}



/*******************************************************************************
    ROM Definitions
*******************************************************************************/

ROM_START( ivanto )
	ROM_REGION16_BE( 0x8000, "maincpu", 0 )
	ROM_LOAD("at27c256r.ic11", 0x0000, 0x8000, CRC(51a97959) SHA1(ea595001fd5eaa07ade96307a8d89d5fb44682ab) ) // no label

	ROM_REGION16_BE( 0x100000, "rombank", 0 )
	ROM_LOAD("1996_701e_excalibur_ivan_t.ic2", 0x000000, 0x100000, CRC(f7c386a1) SHA1(357ca0b0c0b1409d33876b6fa00c5ad74b2643fc) )

	ROM_REGION( 109652, "screen", 0 )
	ROM_LOAD("regency.svg", 0, 109652, CRC(6840c49e) SHA1(a9c91143c5bea5ab41fe323e719da4a46ab9d631) )
ROM_END

} // anonymous namespace



/*******************************************************************************
    Drivers
*******************************************************************************/

//    YEAR  NAME    PARENT  COMPAT  MACHINE  INPUT   CLASS         INIT        COMPANY, FULLNAME, FLAGS
SYST( 1996, ivanto, ivant,  0,      ivanto,  ivanto, ivanto_state, empty_init, "Excalibur Electronics", "Ivan The Terrible (H8/3256 version)", MACHINE_SUPPORTS_SAVE )
