// license:BSD-3-Clause
// copyright-holders:hap
// thanks-to:Berger
/*******************************************************************************

Novag Savant, chess computer with touchscreen. It was followed by Savant II and
Savant Royale on the same hardware, the program is the same and they just added
a bigger opening book. The chess engine is MyChess by David Kittinger.

Like the 1984 version Chess Robot Adversary, Savant Royale was marketed as 7.5MHz,
but it's not known how they sped it up,

Hardware overview:
- Zilog Z80B @ 6MHz
- 24KB ROM(3*TMM2364) + sockets for expansion (populated in Savant II)
- 4KB RAM(8*MM2114N-2L), 256x4 battery-backed RAM(MWS5101)
- Fairchild 3870 @ 4MHz (2KB internal ROM)
- HLCD0538, HLCD0539, LCD screen with 8*8 touch-sensitive overlay
- external ports for optional chess clock and printer

The display (both the LCD and the sensors) didn't last long, probably none exist
anymore in original working order.

*******************************************************************************/

#include "emu.h"

#include "cpu/z80/z80.h"
#include "cpu/f8/f8.h"
#include "machine/f3853.h"
#include "machine/nvram.h"
#include "machine/sensorboard.h"
#include "sound/dac.h"
#include "video/hlcd0538.h"
#include "video/pwm.h"

#include "screen.h"
#include "speaker.h"

// internal artwork
#include "novag_savant.lh"


namespace {

class savant_state : public driver_device
{
public:
	savant_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_mcu(*this, "mcu"),
		m_psu(*this, "psu"),
		m_lcd1(*this, "lcd1"),
		m_lcd2(*this, "lcd2"),
		m_display(*this, "display"),
		m_board(*this, "board"),
		m_dac(*this, "dac"),
		m_nvram(*this, "nvram"),
		m_inputs(*this, "IN.%u", 0)
	{ }

	// machine configs
	void savant(machine_config &config);

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;

private:
	// devices/pointers
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_mcu;
	required_device<f38t56_device> m_psu;
	required_device<hlcd0538_device> m_lcd1;
	required_device<hlcd0539_device> m_lcd2;
	required_device<pwm_display_device> m_display;
	required_device<sensorboard_device> m_board;
	required_device<dac_1bit_device> m_dac;
	required_shared_ptr<u8> m_nvram;
	required_ioport_array<3> m_inputs;

	bool m_z80_wait = false;
	u8 m_inp_mux = 0;
	u8 m_databus = 0;
	u8 m_control = 0;
	u64 m_lcd_data = 0;

	// address maps
	void main_map(address_map &map);
	void main_io(address_map &map);
	void mcu_map(address_map &map);
	void mcu_io(address_map &map);

	// I/O handlers
	void nvram_w(offs_t offset, u8 data);
	u8 nvram_r(offs_t offset);
	u8 stall_r(offs_t offset);
	void stall_w(offs_t offset, u8 data = 0);
	u8 mcustatus_r();

	void lcd1_output_w(u64 data);
	void lcd2_output_w(u64 data);

	u8 databus_r();
	void databus_w(u8 data);
	u8 control_r();
	void control_w(u8 data);
	void lcd_w(u8 data);
	u8 input_r();
};

void savant_state::machine_start()
{
	// register for savestates
	save_item(NAME(m_z80_wait));
	save_item(NAME(m_inp_mux));
	save_item(NAME(m_databus));
	save_item(NAME(m_control));
	save_item(NAME(m_lcd_data));
}

void savant_state::machine_reset()
{
	m_z80_wait = false;
}



/*******************************************************************************
    I/O
*******************************************************************************/

// Z80 side

void savant_state::nvram_w(offs_t offset, u8 data)
{
	// nvram is only d0-d3
	m_nvram[offset] = data & 0xf;
}

u8 savant_state::nvram_r(offs_t offset)
{
	return m_nvram[offset] & 0xf;
}

void savant_state::stall_w(offs_t offset, u8 data)
{
	// any access to port C0 puts the Z80 into WAIT, sets BUSRQ, and sets MCU EXT INT
	if (!m_z80_wait)
	{
		m_databus = offset >> 8;
		m_psu->ext_int_w(1);
		m_maincpu->set_input_line(Z80_INPUT_LINE_WAIT, ASSERT_LINE);
		m_maincpu->set_input_line(Z80_INPUT_LINE_BUSRQ, ASSERT_LINE);
		m_maincpu->defer_access();
	}

	m_z80_wait = !m_z80_wait;
}

u8 savant_state::stall_r(offs_t offset)
{
	if (!machine().side_effects_disabled())
		stall_w(offset);

	return m_databus;
}

u8 savant_state::mcustatus_r()
{
	// d0: MCU P1.2
	return BIT(~m_control, 2);
}


// 3870 side

void savant_state::lcd1_output_w(u64 data)
{
	// uses C1-C24
	m_lcd_data = m_lcd_data << 24 | (data >> 8 & 0xffffff);
	m_display->matrix(data & 0xff, m_lcd_data);
}

void savant_state::lcd2_output_w(u64 data)
{
	// uses C6-C32
	m_lcd_data = data >> 5 & 0x7ffffff;
}

u8 savant_state::databus_r()
{
	return ~m_databus;
}

void savant_state::databus_w(u8 data)
{
	m_databus = ~data;
}

u8 savant_state::control_r()
{
	return m_control;
}

void savant_state::control_w(u8 data)
{
	// d0: clear EXT INT, clear Z80 WAIT
	if (data & ~m_control & 1)
	{
		m_psu->ext_int_w(0);
		m_maincpu->set_input_line(Z80_INPUT_LINE_WAIT, CLEAR_LINE);
	}

	// d1: clear Z80 BUSRQ
	if (data & ~m_control & 2)
		m_maincpu->set_input_line(Z80_INPUT_LINE_BUSRQ, CLEAR_LINE);

	// d2: return data for Z80

	// d3: speaker out
	m_dac->write(BIT(data, 3));

	// d4: LCD pins
	m_lcd2->lcd_w(BIT(~data, 4));
	m_lcd1->lcd_w(BIT(~data, 4));

	// d5-d7: keypad mux
	m_control = data;
}

void savant_state::lcd_w(u8 data)
{
	// d0: HLCD0538 data
	// d4: HLCD0539 data
	m_lcd1->data_w(BIT(~data, 0));
	m_lcd2->data_w(BIT(~data, 4));

	// STROBE pin to LCD chips CLK
	m_lcd1->clk_w(1); m_lcd1->clk_w(0);
	m_lcd2->clk_w(1); m_lcd2->clk_w(0);

	// also touchscreen input mux
	m_inp_mux = bitswap<8>(data,7,3,6,2,5,1,4,0);
}

u8 savant_state::input_r()
{
	u8 data = 0;

	// read touchscreen
	for (int i = 0; i < 8; i++)
		if (BIT(m_inp_mux, i))
			data |= m_board->read_rank(i);

	// read keypad
	for (int i = 0; i < 3; i++)
		if (BIT(m_control >> 5, i))
			data |= m_inputs[i]->read();

	return bitswap<8>(data,3,2,1,0,4,5,6,7);
}



/*******************************************************************************
    Address Maps
*******************************************************************************/

void savant_state::main_map(address_map &map)
{
	map(0x0000, 0xbfff).rom();
	map(0xc000, 0xcfff).ram();
	map(0xd000, 0xd0ff).mirror(0x0300).ram().rw(FUNC(savant_state::nvram_r), FUNC(savant_state::nvram_w)).share("nvram");
}

void savant_state::main_io(address_map &map)
{
	map(0xc0, 0xc0).mirror(0x0038).select(0xff00).rw(FUNC(savant_state::stall_r), FUNC(savant_state::stall_w));
	map(0xc1, 0xc1).mirror(0xff38).nopw(); // clock
	map(0xc2, 0xc2).mirror(0xff38).nopw(); // printer
	map(0xc3, 0xc3).mirror(0xff38).nopr(); // printer
	map(0xc4, 0xc4).mirror(0xff38).r(FUNC(savant_state::mcustatus_r));
	map(0xc5, 0xc5).mirror(0xff38).nopw(); // printer
}

void savant_state::mcu_map(address_map &map)
{
	map.global_mask(0x7ff);
	map(0x0000, 0x07ff).rom();
}

void savant_state::mcu_io(address_map &map)
{
	map(0x00, 0x00).rw(FUNC(savant_state::databus_r), FUNC(savant_state::databus_w));
	map(0x01, 0x01).rw(FUNC(savant_state::control_r), FUNC(savant_state::control_w));
	map(0x04, 0x07).rw(m_psu, FUNC(f38t56_device::read), FUNC(f38t56_device::write));
}



/*******************************************************************************
    Input Ports
*******************************************************************************/

static INPUT_PORTS_START( savant )
	PORT_START("IN.0")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_C) PORT_NAME("Review")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_D) PORT_NAME("Print Board")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_E) PORT_NAME("Sound on/off / Knight")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_3) PORT_NAME("Solve Mate")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_V) PORT_NAME("Show Moves")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_F) PORT_NAME("Form Size")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_R) PORT_NAME("Best Move / Bishop")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_4) PORT_NAME("Clear Board")

	PORT_START("IN.1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_X) PORT_NAME("Trace Forward")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_S) PORT_NAME("Print List")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_W) PORT_NAME("Set Rate / Rook")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_2) PORT_NAME("Classic Game")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_B) PORT_NAME("Return")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_G) PORT_NAME("Auto Play")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_T) PORT_NAME("Mate Alert / Queen")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_5) PORT_NAME("Change Color")

	PORT_START("IN.2")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_Z) PORT_NAME("Trace Back")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_A) PORT_NAME("Print Moves")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_Q) PORT_NAME("Set Level / Pawn")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_1) PORT_NAME("New Game")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_N) PORT_NAME("Go")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_H) PORT_NAME("Hint")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_Y) PORT_NAME("Promote")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_6) PORT_NAME("Set Up")

	PORT_START("LIGHT") // user-controlled light switch (9 light bulbs behind LCD panel)
	PORT_CONFNAME( 0x01, 0x01, "LCD Light" )
	PORT_CONFSETTING(    0x00, DEF_STR( Off ) )
	PORT_CONFSETTING(    0x01, DEF_STR( On ) )
INPUT_PORTS_END



/*******************************************************************************
    Machine Configs
*******************************************************************************/

void savant_state::savant(machine_config &config)
{
	// basic machine hardware
	Z80(config, m_maincpu, 6_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &savant_state::main_map);
	m_maincpu->set_addrmap(AS_IO, &savant_state::main_io);

	F8(config, m_mcu, 4_MHz_XTAL/2);
	m_mcu->set_addrmap(AS_PROGRAM, &savant_state::mcu_map);
	m_mcu->set_addrmap(AS_IO, &savant_state::mcu_io);
	m_mcu->set_irq_acknowledge_callback("psu", FUNC(f38t56_device::int_acknowledge));

	F38T56(config, m_psu, 4_MHz_XTAL/2);
	m_psu->set_int_vector(0x0020);
	m_psu->int_req_callback().set_inputline(m_mcu, F8_INPUT_LINE_INT_REQ);
	m_psu->write_a().set(FUNC(savant_state::lcd_w));
	m_psu->read_b().set(FUNC(savant_state::input_r));

	config.set_perfect_quantum(m_mcu);

	SENSORBOARD(config, m_board).set_type(sensorboard_device::BUTTONS);
	m_board->set_delay(attotime::from_msec(200));
	m_board->set_ui_enable(false); // no chesspieces
	m_board->set_mod_enable(true);

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_1);

	// video hardware
	HLCD0538(config, m_lcd1).write_cols().set(FUNC(savant_state::lcd1_output_w));
	HLCD0539(config, m_lcd2).write_cols().set(FUNC(savant_state::lcd2_output_w));

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_SVG));
	screen.set_refresh_hz(60);
	screen.set_size(958, 1080);
	screen.set_visarea_full();

	PWM_DISPLAY(config, m_display).set_size(8, 24+27);
	config.set_default_layout(layout_novag_savant);

	// sound hardware
	SPEAKER(config, "speaker").front_center();
	DAC_1BIT(config, m_dac).add_route(ALL_OUTPUTS, "speaker", 0.25);
}



/*******************************************************************************
    ROM Definitions
*******************************************************************************/

ROM_START( savant )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD("5605_1f_orange.u13", 0x0000, 0x2000, CRC(0f24fd37) SHA1(b9426b53623d2a98aa2b3099010a7579b0f51db5) ) // TMM2364
	ROM_LOAD("5606_1g_white.u14",  0x2000, 0x2000, CRC(e8b2eddd) SHA1(5f148a3c1c2cd099bd19a48d972a01e5e26ef2ff) ) // "
	ROM_LOAD("5607_1e_blue.u15",   0x4000, 0x2000, CRC(a07f845a) SHA1(e45218fdf955777e571a71ae9d501567b760a3c0) ) // "
	// 3 more ROM sockets not populated(yellow, red, gold), manual mentions possible expansion

	ROM_REGION( 0x0800, "mcu", 0 )
	ROM_LOAD("sl90547.u29", 0x0000, 0x0800, CRC(6fbf2aa0) SHA1(18e673ba5b806b397dd3d350525b5467c25a0d94) )

	ROM_REGION( 764183, "screen", 0)
	ROM_LOAD("savant.svg", 0, 764183, CRC(c3adac84) SHA1(e534aa0a3d339b5351a44aa0507c1ae6af8e1d75) )
ROM_END

ROM_START( savant2 )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD("5605_1f_orange.u13",    0x0000, 0x2000, CRC(0f24fd37) SHA1(b9426b53623d2a98aa2b3099010a7579b0f51db5) ) // TMM2364
	ROM_LOAD("5606_1g_white.u14",     0x2000, 0x2000, CRC(e8b2eddd) SHA1(5f148a3c1c2cd099bd19a48d972a01e5e26ef2ff) ) // "
	ROM_LOAD("5607_1e_blue.u15",      0x4000, 0x2000, CRC(a07f845a) SHA1(e45218fdf955777e571a71ae9d501567b760a3c0) ) // "
	ROM_LOAD("c2m040_savant_red.u17", 0x8000, 0x2000, CRC(0025afb4) SHA1(4f1b63754ed1cc6d765165ec217556b5e7705df6) ) // "

	ROM_REGION( 0x0800, "mcu", 0 )
	ROM_LOAD("sl90547.u29", 0x0000, 0x0800, CRC(6fbf2aa0) SHA1(18e673ba5b806b397dd3d350525b5467c25a0d94) )

	ROM_REGION( 764183, "screen", 0)
	ROM_LOAD("savant.svg", 0, 764183, CRC(c3adac84) SHA1(e534aa0a3d339b5351a44aa0507c1ae6af8e1d75) )
ROM_END

} // anonymous namespace



/*******************************************************************************
    Drivers
*******************************************************************************/

//    YEAR  NAME     PARENT  COMPAT  MACHINE  INPUT   CLASS         INIT        COMPANY, FULLNAME, FLAGS
SYST( 1981, savant,  0,      0,      savant,  savant, savant_state, empty_init, "Novag Industries", "Savant", MACHINE_SUPPORTS_SAVE )
SYST( 1982, savant2, savant, 0,      savant,  savant, savant_state, empty_init, "Novag Industries", "Savant II", MACHINE_SUPPORTS_SAVE )
