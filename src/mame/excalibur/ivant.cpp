// license:BSD-3-Clause
// copyright-holders:hap
// thanks-to:Sean Riddle
/*******************************************************************************

Excalibur Ivan The Terrible (model 701E, H8/3216 version)
Excalibur Igor (model 711E)

This is the newer version of Ivan, see ivanto.cpp for the first version. It's
on very different hardware. Manufacturing was moved to Ewig. Ivan's model number
is the same as the first version, but it's easy to distinguish between them.

The chess engine is by Ron Nelson, similar to the one in Excalibur Mirage. It
has speech, and also sound effects that are reminiscent of Battle Chess.

Hardware notes:
- PCB label: EXCALIBUR ELECTRONICS, INC. 4/18/97, IVANT2, 00-33352-000
- Hitachi H8/3216 MCU (only 32KB out of 48KB internal ROM used), 12MHz XTAL
- small daughterboard (27C080 pinout) with an 1MB ROM under epoxy, contents is
  identical to the 1st version of Ivan after unscrambling
- 8-bit DAC (Yageo 10L503G resistor array), KA8602 amplifier
- LCD with 5 7segs and custom segments
- no LEDs, button sensors chessboard

Igor is on the same base hardware. The PCB (label: EXCALIBUR ELECTRONICS, INC.
510-1005A01, 5/28/97 IGOR) looks a bit different, but the connections are the
same as Ivan. It has a H8/3214 MCU and the sound ROM is 128KB instead of 1MB.

There's also a newer version of Igor from 2000 (model 711E-2) on much weaker
hardware, it has a Samsung KS57C2308 MCU instead.

TODO:
- it does a cold boot at every reset, so nvram won't work properly unless MAME
  adds some kind of auxillary autosave state feature at power-off

BTANB:
- speech sound is a bit scratchy, it has background noise like a tape recorder

*******************************************************************************/

#include "emu.h"

#include "cpu/h8/h83217.h"
#include "machine/sensorboard.h"
#include "sound/dac.h"
#include "video/pwm.h"

#include "screen.h"
#include "speaker.h"

// internal artwork
#include "excal_igor.lh"
#include "excal_ivant.lh"


namespace {

class ivant_state : public driver_device
{
public:
	ivant_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_board(*this, "board"),
		m_soundrom(*this, "soundrom"),
		m_lcd_pwm(*this, "lcd_pwm"),
		m_dac(*this, "dac"),
		m_inputs(*this, "IN.%u", 0),
		m_out_lcd(*this, "s%u.%u", 0U, 0U)
	{ }

	void init_ivant();

	template <typename T> void cpu_config(T &maincpu);
	void shared(machine_config &config);
	void ivant(machine_config &config);
	void igor(machine_config &config);

protected:
	virtual void machine_start() override;

private:
	// devices/pointers
	required_device<h8_device> m_maincpu;
	required_device<sensorboard_device> m_board;
	required_region_ptr<u8> m_soundrom;
	required_device<pwm_display_device> m_lcd_pwm;
	required_device<dac_8bit_r2r_device> m_dac;
	required_ioport_array<2> m_inputs;
	output_finder<2, 24> m_out_lcd;

	u8 m_lcd_com = 0;
	u8 m_dac_data = 0;
	emu_timer *m_irqtimer;

	u8 m_port1 = 0xff;
	u8 m_port2 = 0xff;
	u8 m_port5 = 0xff;
	u8 m_port7 = 0xff;

	// I/O handlers
	void lcd_pwm_w(offs_t offset, u8 data);
	void update_lcd();
	void update_dac();
	u8 read_inputs();

	TIMER_CALLBACK_MEMBER(update_irq) { read_inputs(); }

	void p1_w(u8 data);
	void p2_w(u8 data);
	u8 p3_r();
	void p4_w(u8 data);
	u8 p5_r();
	void p5_w(offs_t offset, u8 data, u8 mem_mask);
	u8 p6_r();
	void p7_w(u8 data);
};



/*******************************************************************************
    Initialization
*******************************************************************************/

void ivant_state::init_ivant()
{
	u8 *rom = memregion("soundrom")->base();
	const u32 len = memregion("soundrom")->bytes();

	// descramble data lines
	for (int i = 0; i < len; i++)
		rom[i] = bitswap<8>(rom[i], 2,3,1,4,0,5,6,7);

	// descramble address lines
	std::vector<u8> buf(len);
	memcpy(&buf[0], rom, len);
	for (int i = 0; i < len; i++)
		rom[i] = buf[bitswap<20>(i,19,18,17,16, 15,14,12,13,6,3,8,10, 11,9,7,5,4,2,1,0)];
}

void ivant_state::machine_start()
{
	m_out_lcd.resolve();

	// periodically check for interrupts
	m_irqtimer = timer_alloc(FUNC(ivant_state::update_irq), this);
	attotime period = attotime::from_msec(1);
	m_irqtimer->adjust(period, 0, period);

	// register for savestates
	save_item(NAME(m_lcd_com));
	save_item(NAME(m_dac_data));
	save_item(NAME(m_port1));
	save_item(NAME(m_port2));
	save_item(NAME(m_port5));
	save_item(NAME(m_port7));
}



/*******************************************************************************
    I/O
*******************************************************************************/

void ivant_state::lcd_pwm_w(offs_t offset, u8 data)
{
	m_out_lcd[offset & 0x3f][offset >> 6] = data;
}

void ivant_state::update_lcd()
{
	u32 lcd_segs = bitswap<8>(m_port1,0,1,2,3,4,5,6,7) << 16 | bitswap<8>(m_port2,0,1,2,3,4,5,6,7) << 8 | m_port7;

	for (int i = 0; i < 2; i++)
	{
		// LCD common is 0/1/Hi-Z
		const u32 data = BIT(m_lcd_com, i + 2) ? (BIT(m_lcd_com, i) ? ~lcd_segs : lcd_segs) : 0;
		m_lcd_pwm->write_row(i, data);
	}
}

void ivant_state::update_dac()
{
	m_dac->write((m_port5 & 4) ? 0x80 : m_dac_data);
}

u8 ivant_state::read_inputs()
{
	u8 data = 0;

	// get input mux from P2/P7/P5
	u16 inp_mux = BIT(~m_port5, 5) << 9 | bitswap<8>(~m_port7,0,1,7,6,5,4,3,2) << 1 | BIT(~m_port2, 6);

	// read buttons
	for (int i = 0; i < 2; i++)
		if (BIT(inp_mux, i + 8))
			data |= m_inputs[i]->read();

	// read chessboard
	for (int i = 0; i < 8; i++)
		if (BIT(inp_mux, i))
			data |= m_board->read_file(i, true);

	// P64-P66 are also IRQ pins (the ON button is IRQ0)
	if (!machine().side_effects_disabled())
		for (int i = 0; i < 3; i++)
			m_maincpu->set_input_line(INPUT_LINE_IRQ0 + i, BIT(data, i + 4) ? ASSERT_LINE : CLEAR_LINE);

	return ~data;
}

void ivant_state::p1_w(u8 data)
{
	// P10-P17: sound ROM address + LCD segs
	m_port1 = data;
}

void ivant_state::p2_w(u8 data)
{
	// P20-P27: sound ROM address + LCD segs
	// P26: input mux low bit
	m_port2 = data;
	read_inputs();
}

u8 ivant_state::p3_r()
{
	// P30-P37: read sound ROM
	u32 address = bitswap<4>(m_port7,4,7,6,5) << 16 | m_port2 << 8 | m_port1;
	return (m_port5 & 4) ? 0xff : m_soundrom[address & (m_soundrom.bytes() - 1)];
}

void ivant_state::p4_w(u8 data)
{
	// P40-P47 (not P46): DAC data
	m_dac_data = (m_dac_data & 0x40) | (data & 0xbf);
	update_dac();
}

u8 ivant_state::p5_r()
{
	// P50: multiplexed inputs high bit
	return read_inputs() >> 7 | 0xfe;
}

void ivant_state::p5_w(offs_t offset, u8 data, u8 mem_mask)
{
	// P51: DAC bit 6
	m_dac_data = (m_dac_data & 0xbf) | BIT(data, 1) << 6;

	// P52: sound ROM CS, KA8602 mute
	// P55: input mux high bit
	m_port5 = data;
	update_dac();
	read_inputs();

	// P53,P54: LCD common
	u8 lcd_com = (mem_mask >> 1 & 0xc) | (data >> 3 & 3);
	if (lcd_com != m_lcd_com)
	{
		m_lcd_com = lcd_com;
		update_lcd();
	}
}

u8 ivant_state::p6_r()
{
	// P60-P66: multiplexed inputs part
	return read_inputs() | 0x80;
}

void ivant_state::p7_w(u8 data)
{
	// P70-P77: input mux part + LCD segs
	// P74-P77: sound ROM address
	m_port7 = data;
	read_inputs();
}



/*******************************************************************************
    Input Ports
*******************************************************************************/

static INPUT_PORTS_START( ivant )
	PORT_START("IN.0")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_U) PORT_CODE(KEYCODE_LEFT) PORT_NAME("No / Left")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_R) PORT_NAME("Repeat")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_V) PORT_CODE(KEYCODE_2) PORT_CODE(KEYCODE_2_PAD) PORT_NAME("Verify / Queen")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_O) PORT_NAME("Option")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_N) PORT_NAME("New Game")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_B) PORT_NAME("Black / White")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_T) PORT_NAME("Take Back")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_Y) PORT_CODE(KEYCODE_RIGHT) PORT_NAME("Yes / Right")

	PORT_START("IN.1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_E) PORT_NAME("Mode")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_S) PORT_CODE(KEYCODE_1) PORT_CODE(KEYCODE_1_PAD) PORT_NAME("Set Up / King")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_I) PORT_CODE(KEYCODE_4) PORT_CODE(KEYCODE_4_PAD) PORT_NAME("Multi-Move / Bishop")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_F2) PORT_NAME("Off / Save")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_F1) PORT_CODE(KEYCODE_C) PORT_NAME("On / Clear")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_M) PORT_CODE(KEYCODE_6) PORT_CODE(KEYCODE_6_PAD) PORT_NAME("Move / Pawn")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_L) PORT_CODE(KEYCODE_3) PORT_CODE(KEYCODE_3_PAD) PORT_NAME("Level / Rook")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_H) PORT_CODE(KEYCODE_5) PORT_CODE(KEYCODE_5_PAD) PORT_NAME("Hint / Knight")

	PORT_START("BATT")
	PORT_CONFNAME( 0x40, 0x00, "Battery Status" )
	PORT_CONFSETTING(    0x40, "Low" )
	PORT_CONFSETTING(    0x00, DEF_STR( Normal ) )
	PORT_BIT(0xbf, IP_ACTIVE_HIGH, IPT_UNUSED)
INPUT_PORTS_END

static INPUT_PORTS_START( igor ) // same buttons, different layout
	PORT_INCLUDE( ivant )

	PORT_MODIFY("IN.0")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_T) PORT_NAME("Takeback")

	PORT_MODIFY("IN.1")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_I) PORT_CODE(KEYCODE_3) PORT_CODE(KEYCODE_3_PAD) PORT_NAME("Multi-Move / Bishop")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_L) PORT_CODE(KEYCODE_5) PORT_CODE(KEYCODE_5_PAD) PORT_NAME("Level / Rook")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_H) PORT_CODE(KEYCODE_4) PORT_CODE(KEYCODE_4_PAD) PORT_NAME("Hint / Knight")

	PORT_MODIFY("BATT") // read, but discarded
	PORT_BIT(0xff, IP_ACTIVE_HIGH, IPT_UNUSED)
INPUT_PORTS_END



/*******************************************************************************
    Machine Configs
*******************************************************************************/

template <typename T>
void ivant_state::cpu_config(T &maincpu)
{
	maincpu.nvram_enable_backup(true);
	maincpu.standby_cb().set(maincpu, FUNC(T::nvram_set_battery));
	maincpu.standby_cb().append([this](int state) { if (state) m_lcd_pwm->clear(); });
	maincpu.write_port1().set(FUNC(ivant_state::p1_w));
	maincpu.write_port2().set(FUNC(ivant_state::p2_w));
	maincpu.read_port3().set(FUNC(ivant_state::p3_r));
	maincpu.read_port4().set_ioport("BATT").invert();
	maincpu.write_port4().set(FUNC(ivant_state::p4_w));
	maincpu.read_port5().set(FUNC(ivant_state::p5_r));
	maincpu.write_port5().set(FUNC(ivant_state::p5_w));
	maincpu.read_port6().set(FUNC(ivant_state::p6_r));
	maincpu.write_port7().set(FUNC(ivant_state::p7_w));
}

void ivant_state::shared(machine_config &config)
{
	// basic machine hardware
	SENSORBOARD(config, m_board).set_type(sensorboard_device::BUTTONS);
	m_board->init_cb().set(m_board, FUNC(sensorboard_device::preset_chess));
	m_board->set_delay(attotime::from_msec(150));
	//m_board->set_nvram_enable(true);

	// video hardware
	PWM_DISPLAY(config, m_lcd_pwm).set_size(2, 24);
	m_lcd_pwm->output_x().set(FUNC(ivant_state::lcd_pwm_w));

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_SVG));
	screen.set_refresh_hz(60);
	screen.set_size(1920/6, 723/6);
	screen.set_visarea_full();

	// sound hardware
	SPEAKER(config, "speaker").front_center();
	DAC_8BIT_R2R(config, m_dac).add_route(ALL_OUTPUTS, "speaker", 0.5);
}

void ivant_state::ivant(machine_config &config)
{
	H83216(config, m_maincpu, 12_MHz_XTAL);
	cpu_config<h83216_device>(downcast<h83216_device &>(*m_maincpu));

	shared(config);

	config.set_default_layout(layout_excal_ivant);
}

void ivant_state::igor(machine_config &config)
{
	H83214(config, m_maincpu, 12_MHz_XTAL);
	cpu_config<h83214_device>(downcast<h83214_device &>(*m_maincpu));

	shared(config);

	config.set_default_layout(layout_excal_igor);
}



/*******************************************************************************
    ROM Definitions
*******************************************************************************/

ROM_START( ivant )
	ROM_REGION16_BE( 0xc000, "maincpu", 0 )
	ROM_LOAD("1997_rcn_1003a_excal_hd6433216l01p.ic1", 0x0000, 0xc000, CRC(6fcc34b1) SHA1(13bcb3d6766e6f3acb7d0f669337e8e40d5ed449) )

	ROM_REGION( 0x100000, "soundrom", 0 )
	ROM_LOAD("sound.ic2", 0x000000, 0x100000, CRC(7f9a78c9) SHA1(b80d33955496698c9288047e0854b335882bcdc7) ) // no label

	ROM_REGION( 89047, "screen", 0 )
	ROM_LOAD("ivant.svg", 0, 89047, CRC(fe514f65) SHA1(da5a56882bd241d01a6c49cb1cb066b88473c445) )
ROM_END

ROM_START( igor )
	ROM_REGION16_BE( 0x8000, "maincpu", 0 )
	ROM_LOAD("1997_rcn_1002a_excal_hd6433214l02p.ic1", 0x0000, 0x8000, CRC(adbc7e07) SHA1(0d297ad2fd0d18312966195cfad4658da4bc4442) )

	ROM_REGION( 0x20000, "soundrom", 0 )
	ROM_LOAD("sound.ic2", 0x00000, 0x20000, CRC(bc540da3) SHA1(68647ce1c7e87eba90d9d1912921213af03e3c5d) ) // no label

	ROM_REGION( 89047, "screen", 0 )
	ROM_LOAD("ivant.svg", 0, 89047, CRC(fe514f65) SHA1(da5a56882bd241d01a6c49cb1cb066b88473c445) )
ROM_END

} // anonymous namespace



/*******************************************************************************
    Drivers
*******************************************************************************/

//    YEAR  NAME   PARENT  COMPAT  MACHINE  INPUT  CLASS        INIT        COMPANY, FULLNAME, FLAGS
SYST( 1997, ivant, 0,      0,      ivant,   ivant, ivant_state, init_ivant, "Excalibur Electronics", "Ivan The Terrible (H8/3216 version)", MACHINE_SUPPORTS_SAVE )

SYST( 1997, igor,  0,      0,      igor,    igor,  ivant_state, init_ivant, "Excalibur Electronics", "Igor (Excalibur)", MACHINE_SUPPORTS_SAVE )
