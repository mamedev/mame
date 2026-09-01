// license:BSD-3-Clause
// copyright-holders:hap
// thanks-to:Sean Riddle
/*******************************************************************************

Excalibur Grandmaster

Two versions exist, 747K (black) and 747P (silver, P for Platinum?). Other than
the housing color, they're presumed to be the same.

Hardware notes:
- PCB label: EXCAL IBUR ELECTRONICS 1997, KARPOV1, 4/16/97, 00-82652-001
- Hitachi H8/3214 MCU, 12MHz XTAL
- 2 LCDs with 4 7segs and custom segments
- piezo, no LEDs, magnet sensors chessboard

TODO:
- finish the buttons
- it does a cold boot at every reset, so nvram won't work properly unless MAME
  adds some kind of auxillary autosave state feature at power-off

*******************************************************************************/

#include "emu.h"

#include "cpu/h8/h83217.h"
#include "machine/sensorboard.h"
#include "sound/dac.h"
#include "video/pwm.h"

#include "screen_svg.h"
#include "speaker.h"

// internal artwork
#include "excal_grandmas.lh"


namespace {

class grandmas_state : public driver_device
{
public:
	grandmas_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_board(*this, "board"),
		m_lcd_pwm(*this, "lcd_pwm"),
		m_dac(*this, "dac"),
		m_inputs(*this, "IN.%u", 0)
	{ }

	void grandmas(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	// devices/pointers
	required_device<h83214_device> m_maincpu;
	required_device<sensorboard_device> m_board;
	required_device<pwm_display_device> m_lcd_pwm;
	required_device<dac_1bit_device> m_dac;
	required_ioport_array<3> m_inputs;

	u8 m_port4 = 0xff;
	u8 m_port5 = 0xff;
	u32 m_lcd_segs = 0;
	u8 m_lcd_com = 0;
	emu_timer *m_irqtimer;

	// I/O handlers
	void update_lcd();
	u8 read_inputs();

	TIMER_CALLBACK_MEMBER(update_irq) { read_inputs(); }

	template <int N> void lcd_segs_w(u8 data);
	void p4_w(u8 data);
	u8 p5_r();
	void p5_w(u8 data);
	u8 p6_r();
	void lcd_com_w(offs_t offset, u8 data, u8 mem_mask);
};

void grandmas_state::machine_start()
{
	// periodically check for interrupts
	m_irqtimer = timer_alloc(FUNC(grandmas_state::update_irq), this);
	attotime period = attotime::from_msec(1);
	m_irqtimer->adjust(period, 0, period);

	// register for savestates
	save_item(NAME(m_port4));
	save_item(NAME(m_port5));
	save_item(NAME(m_lcd_segs));
	save_item(NAME(m_lcd_com));
}



/*******************************************************************************
    I/O
*******************************************************************************/

// LCD

void grandmas_state::update_lcd()
{
	for (int i = 0; i < 3; i++)
	{
		// LCD common is 0/1/Hi-Z
		const u32 data = BIT(m_lcd_com, i + 3) ? (BIT(m_lcd_com, i) ? ~m_lcd_segs : m_lcd_segs) : 0;
		m_lcd_pwm->write_row(i, data);
	}
}

template <int N>
void grandmas_state::lcd_segs_w(u8 data)
{
	// P1x, P2x, P3x: LCD segments (P10-P13 and P30 also used for p2 input mux)
	const u8 shift = 8 * N;
	m_lcd_segs = (m_lcd_segs & ~(0xff << shift)) | (data << shift);
}

void grandmas_state::lcd_com_w(offs_t offset, u8 data, u8 mem_mask)
{
	// P75-P77: LCD common
	u8 lcd_com = (mem_mask >> 2 & 0x38) | (data >> 5 & 7);
	if (lcd_com != m_lcd_com)
	{
		m_lcd_com = lcd_com;
		update_lcd();
	}
}


// misc

u8 grandmas_state::read_inputs()
{
	u8 data = 0;

	// get board mux from P4/P5
	u8 board_mux = (~m_port4 & 0xbf) | BIT(~m_port5, 5) << 6;

	// read chessboard
	for (int i = 0; i < 8; i++)
		if (BIT(board_mux, i))
			data |= m_board->read_rank(i);

	// read buttons
	for (int i = 0; i < 2; i++)
		if (BIT(~m_port5, i + 1))
			data |= m_inputs[i]->read();

	// P64-P66 are also IRQ pins (the ON button is IRQ1)
	if (!machine().side_effects_disabled())
		for (int i = 0; i < 3; i++)
			m_maincpu->set_input_line(INPUT_LINE_IRQ0 + i, BIT(data, i + 4) ? ASSERT_LINE : CLEAR_LINE);

	return bitswap<8>(~data,7,6,5,4,0,1,2,3);
}

void grandmas_state::p4_w(u8 data)
{
	// P40-P45,P47: input mux part
	m_port4 = data;
	read_inputs();

	// P46: N/C (not battery status)
}

u8 grandmas_state::p5_r()
{
	// P50: multiplexed inputs high bit
	u8 data = read_inputs() >> 7;

	// P53: player 2 buttons
	u8 p2_mux = m_lcd_segs << 1 | BIT(m_lcd_segs, 16);
	data |= (~p2_mux & m_inputs[2]->read()) ? 0 : 8;

	return data | 0xf6;
}

void grandmas_state::p5_w(u8 data)
{
	// P51,P52,P55: input mux part
	m_port5 = data;
	read_inputs();

	// P54: speaker out
	m_dac->write(BIT(~data, 4));
}

u8 grandmas_state::p6_r()
{
	// P60-P66: multiplexed inputs part
	return read_inputs() | 0x80;
}



/*******************************************************************************
    Input Ports
*******************************************************************************/

static INPUT_PORTS_START( grandmas )
	PORT_START("IN.0")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_1) // -
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_2) // score
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_3) // hint
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_4) // clock
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_5) // to
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_6) // on
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_7) // off
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_8) // from

	PORT_START("IN.1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_Q) // new game
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_W) // mode
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_E) // setup
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_R) // verify
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_T) // monitor
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_Y) // move
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_U) // tb
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_I) // level

	PORT_START("IN.2")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_A) // score
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_S) // hint
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_D) // clock
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_F) // from
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_G) // to
INPUT_PORTS_END



/*******************************************************************************
    Machine Configs
*******************************************************************************/

void grandmas_state::grandmas(machine_config &config)
{
	// basic machine hardware
	H83214(config, m_maincpu, 12_MHz_XTAL);
	m_maincpu->nvram_enable_backup(true);
	m_maincpu->standby_cb().set(m_maincpu, FUNC(h83214_device::nvram_set_battery));
	m_maincpu->standby_cb().append([this](int state) { if (state) m_lcd_pwm->clear(); });
	m_maincpu->write_port1().set(FUNC(grandmas_state::lcd_segs_w<0>));
	m_maincpu->write_port2().set(FUNC(grandmas_state::lcd_segs_w<1>));
	m_maincpu->write_port3().set(FUNC(grandmas_state::lcd_segs_w<2>));
	m_maincpu->write_port4().set(FUNC(grandmas_state::p4_w));
	m_maincpu->read_port5().set(FUNC(grandmas_state::p5_r));
	m_maincpu->write_port5().set(FUNC(grandmas_state::p5_w));
	m_maincpu->read_port6().set(FUNC(grandmas_state::p6_r));
	m_maincpu->write_port7().set(FUNC(grandmas_state::lcd_com_w));

	SENSORBOARD(config, m_board).set_type(sensorboard_device::MAGNETS);
	m_board->init_cb().set(m_board, FUNC(sensorboard_device::preset_chess));
	m_board->set_delay(attotime::from_msec(150));
	//m_board->set_nvram_enable(true);

	// video hardware
	PWM_DISPLAY(config, m_lcd_pwm).set_size(3, 24);

	screen_svg_device &screen1(SCREEN_SVG(config, "screen1"));
	screen1.set_refresh_hz(60);
	screen1.set_size(1920/6, 753/6);

	screen_svg_device &screen2(SCREEN_SVG(config, "screen2"));
	screen2.set_refresh_hz(60);
	screen2.set_size(1920/6, 753/6);

	config.set_default_layout(layout_excal_grandmas);

	// sound hardware
	SPEAKER(config, "speaker").front_center();
	DAC_1BIT(config, m_dac).add_route(ALL_OUTPUTS, "speaker", 0.25);
}



/*******************************************************************************
    ROM Definitions
*******************************************************************************/

ROM_START( egrandmas )
	ROM_REGION16_BE( 0x8000, "maincpu", 0 )
	ROM_LOAD("1997_rcn_1002a_excal_hd6433214l01p.ic1", 0x0000, 0x8000, CRC(7c3641df) SHA1(454080dcdbfe378403b51df125c1f7c872edf54a) )

	ROM_REGION( 94253, "screen1", 0)
	ROM_LOAD("egrandmas1.svg", 0, 94253, CRC(6dee6349) SHA1(44db648f2195768cd78cb4af4df23492b527ecbc) )

	ROM_REGION( 94279, "screen2", 0)
	ROM_LOAD("egrandmas2.svg", 0, 94279, CRC(1613b268) SHA1(281a6ba7dd6a88ce077ac5839c8616d0504ee12e) )
ROM_END

} // anonymous namespace



/*******************************************************************************
    Drivers
*******************************************************************************/

//    YEAR  NAME       PARENT  COMPAT  MACHINE   INPUT     CLASS           INIT        COMPANY, FULLNAME, FLAGS
SYST( 1997, egrandmas, 0,      0,      grandmas, grandmas, grandmas_state, empty_init, "Excalibur Electronics", "Grandmaster (Excalibur)", MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE )
