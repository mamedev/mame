// license:BSD-3-Clause
// copyright-holders:hap
// thanks-to:Sean Riddle
/*******************************************************************************

Excalibur Grandmaster

Hardware notes:
- PCB label: EXCAL IBUR ELECTRONICS 1997, KARPOV1, 4/16/97, 00-82652-001
- Hitachi H8/3214 MCU, 12MHz XTAL
- 2 LCDs with 4 7segs and custom segments
- piezo, no LEDs, magnet sensors chessboard

TODO:
- WIP (mainly the LCDs)
- it does a cold boot at every reset, so nvram won't work properly unless MAME
  adds some kind of auxillary autosave state feature at power-off

*******************************************************************************/

#include "emu.h"

#include "cpu/h8/h83217.h"
#include "machine/sensorboard.h"
#include "sound/dac.h"
//#include "video/pwm.h"

//#include "screen_svg.h"
#include "speaker.h"

// internal artwork
//#include "excal_grandmaster.lh"


namespace {

class grandmas_state : public driver_device
{
public:
	grandmas_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_board(*this, "board"),
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
	required_device<dac_1bit_device> m_dac;
	required_ioport_array<3> m_inputs;

	u8 m_port1 = 0xff;
	u8 m_port3 = 0xff;
	u8 m_port4 = 0xff;
	u8 m_port5 = 0xff;
	emu_timer *m_irqtimer;

	// I/O handlers
	u8 read_inputs();

	TIMER_CALLBACK_MEMBER(update_irq) { read_inputs(); }

	void p1_w(u8 data) { m_port1 = data; }
	void p3_w(u8 data) { m_port3 = data; }
	void p4_w(u8 data);
	u8 p5_r();
	void p5_w(u8 data);
	u8 p6_r();
};

void grandmas_state::machine_start()
{
	// periodically check for interrupts
	m_irqtimer = timer_alloc(FUNC(grandmas_state::update_irq), this);
	attotime period = attotime::from_msec(1);
	m_irqtimer->adjust(period, 0, period);

	// register for savestates
	save_item(NAME(m_port1));
	save_item(NAME(m_port3));
	save_item(NAME(m_port4));
	save_item(NAME(m_port5));
}



/*******************************************************************************
    I/O
*******************************************************************************/

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
	u8 p2_mux = m_port1 << 1 | (m_port3 & 1);
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
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_1)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_2)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_3)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_4)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_5)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_6) // on
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_7) // off
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_8)

	PORT_START("IN.1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_Q) // new game
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_W)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_E)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_R)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_T)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_Y) // move
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_U)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_I)

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
	m_maincpu->write_port1().set(FUNC(grandmas_state::p1_w));
	m_maincpu->write_port3().set(FUNC(grandmas_state::p3_w));
	m_maincpu->write_port4().set(FUNC(grandmas_state::p4_w));
	m_maincpu->read_port5().set(FUNC(grandmas_state::p5_r));
	m_maincpu->write_port5().set(FUNC(grandmas_state::p5_w));
	m_maincpu->read_port6().set(FUNC(grandmas_state::p6_r));

	SENSORBOARD(config, m_board).set_type(sensorboard_device::MAGNETS);
	m_board->init_cb().set(m_board, FUNC(sensorboard_device::preset_chess));
	m_board->set_delay(attotime::from_msec(150));
	//m_board->set_nvram_enable(true);

	// video hardware
	//config.set_default_layout(layout_excal_grandmaster);

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
ROM_END

} // anonymous namespace



/*******************************************************************************
    Drivers
*******************************************************************************/

//    YEAR  NAME       PARENT  COMPAT  MACHINE   INPUT     CLASS           INIT        COMPANY, FULLNAME, FLAGS
SYST( 1997, egrandmas, 0,      0,      grandmas, grandmas, grandmas_state, empty_init, "Excalibur Electronics", "Grandmaster (Excalibur)", MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE )
