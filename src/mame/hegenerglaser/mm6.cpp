// license:BSD-3-Clause
// copyright-holders:hap
// thanks-to:Berger, Mr. Lars
/*******************************************************************************

Saitek Mephisto MM VI

It's a module, just like the older "MM" series. The chess engine is by Frans Morsch.
Press Left + Right during POST for QC test mode.

Hardware notes:
- PCB label: Saitek MMVI LOGIC MODUL, RM19-PE-001 REV0
- Hitachi H8/325 MCU, 20MHz XTAL
- STM 93C56 EEPROM (128x16), piezo
- compatible with magnets chessboard and 6th gen. LCD module

Due to uninitialized EEPROM, it will give an error after the 1st boot. Press any
button to continue. After having saved once, the error won't show up anymore.

It will auto-save once in a while, but even then, make sure to press SAVE before
power off (MAME exit) to make it remember the current settings and chess position,
this is also warned about in the manual.

*******************************************************************************/

#include "emu.h"

#include "mboard.h"
#include "mdisplay3.h"

#include "cpu/h8/h8325.h"
#include "machine/eepromser.h"
#include "sound/dac.h"

#include "speaker.h"

// internal artwork
#include "mephisto_mm6.lh"


namespace {

class mm6_state : public driver_device
{
public:
	mm6_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_eeprom(*this, "eeprom"),
		m_board(*this, "board"),
		m_display(*this, "display"),
		m_dac(*this, "dac"),
		m_inputs(*this, "IN.%u", 0)
	{ }

	void mm6(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	// devices/pointers
	required_device<h8325_device> m_maincpu;
	required_device<eeprom_serial_93cxx_device> m_eeprom;
	required_device<mephisto_board_device> m_board;
	required_device<mephisto_display3_device> m_display;
	required_device<dac_1bit_device> m_dac;
	required_ioport_array<2> m_inputs;

	u8 m_inp_mux = 0;

	u8 p2_r();
	void p2_w(u8 data);
	u8 p3_r();
	u8 p4_r();
	void p4_w(u8 data);
	void p5_w(u8 data);
	void p6_w(u8 data);
	void p7_w(u8 data);
};

void mm6_state::machine_start()
{
	save_item(NAME(m_inp_mux));
}



/*******************************************************************************
    I/O
*******************************************************************************/

u8 mm6_state::p2_r()
{
	// P20-P27: chessboard data
	return m_board->data_r();
}

void mm6_state::p2_w(u8 data)
{
	// P20-P27: chessboard & LCD data
	m_board->data_w(data);
	m_display->data_w(data);
}

u8 mm6_state::p3_r()
{
	u8 data = 0;

	// P30-P37: read buttons
	for (int i = 0; i < 3; i++)
		if (BIT(m_inp_mux, i))
			data |= m_inputs[i]->read();

	return ~data;
}

u8 mm6_state::p4_r()
{
	// P40: EEPROM DO
	return m_eeprom->do_read() | 0xfe;
}

void mm6_state::p4_w(u8 data)
{
	// P41-P42: EEPROM output signals
	m_eeprom->di_write(BIT(data, 1));
	m_eeprom->clk_write(BIT(data, 2));
	m_eeprom->cs_write(BIT(data, 3));
}

void mm6_state::p5_w(u8 data)
{
	// P50-P53: chessboard signals
	m_board->ldc_en_w(BIT(data, 0));
	m_board->cb_en_w(BIT(data, 1));
	m_board->row_le_w(BIT(data, 2));
	m_board->ldc_le_w(BIT(data, 3));
}

void mm6_state::p6_w(u8 data)
{
	// P60: speaker out
	m_dac->write(data & 1);

	// P62,P63: input mux
	m_inp_mux = ~data >> 2 & 3;
}

void mm6_state::p7_w(u8 data)
{
	// P71: LCD clock
	m_display->clk_w(BIT(data, 1));

	// P72-P75: LCD common
	m_display->common_w(data >> 2 & 0xf);
}



/*******************************************************************************
    Input Ports
*******************************************************************************/

static INPUT_PORTS_START( mm6 )
	PORT_START("IN.0")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_S) PORT_NAME("SAVE")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_O) PORT_NAME("OPT")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_L) PORT_NAME("LEV")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_W) PORT_CODE(KEYCODE_LEFT) PORT_NAME("White / Left")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_N) PORT_NAME("New Game")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_B) PORT_CODE(KEYCODE_RIGHT) PORT_NAME("Black / Right")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_P) PORT_NAME("POS")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_I) PORT_NAME("INFO")

	PORT_START("IN.1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_1) PORT_CODE(KEYCODE_1_PAD) PORT_NAME("Pawn")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_2) PORT_CODE(KEYCODE_2_PAD) PORT_NAME("Knight")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_3) PORT_CODE(KEYCODE_3_PAD) PORT_NAME("Bishop")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_BACKSPACE) PORT_CODE(KEYCODE_DEL) PORT_NAME("CLR")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_6) PORT_CODE(KEYCODE_6_PAD) PORT_NAME("King")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_ENTER) PORT_CODE(KEYCODE_ENTER_PAD) PORT_NAME("ENT")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_5) PORT_CODE(KEYCODE_5_PAD) PORT_NAME("Queen")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_4) PORT_CODE(KEYCODE_4_PAD) PORT_NAME("Rook")
INPUT_PORTS_END



/*******************************************************************************
    Machine Configs
*******************************************************************************/

void mm6_state::mm6(machine_config &config)
{
	// basic machine hardware
	H8325(config, m_maincpu, 20_MHz_XTAL);
	m_maincpu->read_port2().set(FUNC(mm6_state::p2_r));
	m_maincpu->write_port2().set(FUNC(mm6_state::p2_w));
	m_maincpu->read_port3().set(FUNC(mm6_state::p3_r));
	m_maincpu->read_port4().set(FUNC(mm6_state::p4_r));
	m_maincpu->write_port4().set(FUNC(mm6_state::p4_w));
	m_maincpu->write_port5().set(FUNC(mm6_state::p5_w));
	m_maincpu->write_port6().set(FUNC(mm6_state::p6_w));
	m_maincpu->write_port7().set(FUNC(mm6_state::p7_w));

	EEPROM_93C56_16BIT(config, m_eeprom).default_value(~0);

	MEPHISTO_SENSORS_BOARD(config, m_board);
	m_board->set_delay(attotime::from_msec(200));
	subdevice<sensorboard_device>("board:board")->set_nvram_enable(true);

	// video hardware
	MEPHISTO_DISPLAY_MODULE3(config, m_display);
	config.set_default_layout(layout_mephisto_mm6);

	// sound hardware
	SPEAKER(config, "speaker").front_center();
	DAC_1BIT(config, m_dac).add_route(ALL_OUTPUTS, "speaker", 0.25);
}



/*******************************************************************************
    ROM Definitions
*******************************************************************************/

ROM_START( mm6 )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD("96_saitek_86146150508_3258c68p.u1", 0x0000, 0x8000, CRC(816a82fd) SHA1(7342f46b09653ea825cb92161e502d658c72de3b) )
ROM_END

} // anonymous namespace



/*******************************************************************************
    Drivers
*******************************************************************************/

//    YEAR  NAME  PARENT  COMPAT  MACHINE  INPUT  CLASS      INIT        COMPANY, FULLNAME, FLAGS
SYST( 1996, mm6,  0,      0,      mm6,     mm6,   mm6_state, empty_init, "Saitek", "Mephisto MM VI", MACHINE_SUPPORTS_SAVE ) // when H+G was a subsidiary of Saitek
