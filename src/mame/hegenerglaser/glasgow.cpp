// license:BSD-3-Clause
// copyright-holders:Dirk Verwiebe, Cowering, hap
// thanks-to:Berger
/*******************************************************************************

Mephisto Glasgow 3 S chess computer
Dirk V.
sp_rinter@gmx.de

Hardware notes:
- R68000C10 or MC68000P12 @ 12MHz, IRQ from 50Hz Seiko SG-10 chip "50H", to IPL0+2
- 64KB ROM (4*27128), 16KB RAM (2*6264)
- LCD module same as MMx series, piezo

Other TTL:

3*74LS138 Decoder/Multiplexer
1*74LS74  Dual positive edge triggered D Flip Flop
1*74LS139 1of4 Demultiplexer
1*74LS05  Hex Inverter
1*NE555   R=100K C=10uF
2*74LS04  Hex Inverter
1*74LS164 8 Bit Shift register
1*74121   Monostable Multivibrator with Schmitt Trigger Inputs
1*74LS20  Dual 4 Input NAND GAte
1*74LS367 3 State Hex Buffers

TODO:
- add waitstates, CPU is 12MHz but with DTACK waitstates for slow EPROMs,
  effective speed is around 7.2MHz

*******************************************************************************/

#include "emu.h"

#include "mmboard.h"
#include "mmdisplay1.h"

#include "cpu/m68000/m68000.h"
#include "sound/dac.h"

#include "speaker.h"

// internal artwork
#include "mephisto_glasgow.lh"


namespace {

class glasgow_state : public driver_device
{
public:
	glasgow_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_board(*this, "board"),
		m_display(*this, "display"),
		m_dac(*this, "dac"),
		m_keys(*this, "KEY.%u", 0)
	{ }

	void glasgow(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;
	required_device<mephisto_board_device> m_board;
	required_device<mephisto_display1_device> m_display;
	required_device<dac_1bit_device> m_dac;
	required_ioport_array<2> m_keys;

	u8 m_kp_mux = 0;

	void glasgow_mem(address_map &map) ATTR_COLD;

	void control_w(u8 data);
	u8 keys_r();
	void keys_w(u8 data);
};

void glasgow_state::machine_start()
{
	save_item(NAME(m_kp_mux));
}



/*******************************************************************************
    I/O
*******************************************************************************/

void glasgow_state::control_w(u8 data)
{
	// d0: speaker out
	m_dac->write(BIT(data, 0));

	// d7: lcd strobe
	m_display->strobe_w(BIT(data, 7));
}

u8 glasgow_state::keys_r()
{
	u8 data = 0;

	// d0,d1: multiplexed inputs
	for (int i = 0; i < 2; i++)
		if (m_kp_mux & m_keys[i]->read())
			data |= 1 << i;

	// reading keypad also clears irq
	if (!machine().side_effects_disabled())
		m_maincpu->set_input_line(M68K_IRQ_5, CLEAR_LINE);

	return ~data;
}

void glasgow_state::keys_w(u8 data)
{
	m_kp_mux = ~data;
}



/*******************************************************************************
    Address Maps
*******************************************************************************/

void glasgow_state::glasgow_mem(address_map &map)
{
	map.global_mask(0x1ffff);
	map(0x000000, 0x00ffff).rom();
	map(0x010000, 0x010000).w(m_display, FUNC(mephisto_display1_device::data_w));
	map(0x010002, 0x010002).rw(FUNC(glasgow_state::keys_r), FUNC(glasgow_state::keys_w));
	map(0x010004, 0x010004).w(FUNC(glasgow_state::control_w));
	map(0x010006, 0x010006).rw(m_board, FUNC(mephisto_board_device::input_r), FUNC(mephisto_board_device::led_w));
	map(0x010008, 0x010008).w(m_board, FUNC(mephisto_board_device::mux_w));
	map(0x01c000, 0x01ffff).ram();
}



/*******************************************************************************
    Input Ports
*******************************************************************************/

static INPUT_PORTS_START( glasgow )
	PORT_START("KEY.0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("E / 5 / Rook") PORT_CODE(KEYCODE_E) PORT_CODE(KEYCODE_5) PORT_CODE(KEYCODE_5_PAD)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("INFO") PORT_CODE(KEYCODE_I)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Right / White / 0") PORT_CODE(KEYCODE_0) PORT_CODE(KEYCODE_0_PAD) PORT_CODE(KEYCODE_RIGHT)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("POS") PORT_CODE(KEYCODE_P)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("H / 8") PORT_CODE(KEYCODE_H) PORT_CODE(KEYCODE_8) PORT_CODE(KEYCODE_8_PAD)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("LEV") PORT_CODE(KEYCODE_L)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("G / 7 / King") PORT_CODE(KEYCODE_G) PORT_CODE(KEYCODE_7) PORT_CODE(KEYCODE_7_PAD)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("MEM") PORT_CODE(KEYCODE_M)

	PORT_START("KEY.1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Left / Black / 9") PORT_CODE(KEYCODE_9) PORT_CODE(KEYCODE_9_PAD) PORT_CODE(KEYCODE_LEFT)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("CL") PORT_CODE(KEYCODE_BACKSPACE) PORT_CODE(KEYCODE_DEL)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("C / 3 / Knight") PORT_CODE(KEYCODE_C) PORT_CODE(KEYCODE_3) PORT_CODE(KEYCODE_3_PAD)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("ENT") PORT_CODE(KEYCODE_ENTER) PORT_CODE(KEYCODE_ENTER_PAD)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("D / 4 / Bishop") PORT_CODE(KEYCODE_D) PORT_CODE(KEYCODE_4) PORT_CODE(KEYCODE_4_PAD)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("A / 1") PORT_CODE(KEYCODE_A) PORT_CODE(KEYCODE_1) PORT_CODE(KEYCODE_1_PAD)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("F / 6 / Queen") PORT_CODE(KEYCODE_F) PORT_CODE(KEYCODE_6) PORT_CODE(KEYCODE_6_PAD)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("B / 2 / Pawn") PORT_CODE(KEYCODE_B) PORT_CODE(KEYCODE_2) PORT_CODE(KEYCODE_2_PAD)
INPUT_PORTS_END



/*******************************************************************************
    Machine Configs
*******************************************************************************/

void glasgow_state::glasgow(machine_config &config)
{
	// basic machine hardware
	M68000(config, m_maincpu, 12_MHz_XTAL);
	m_maincpu->set_periodic_int(FUNC(glasgow_state::irq5_line_assert), attotime::from_hz(50));
	m_maincpu->set_addrmap(AS_PROGRAM, &glasgow_state::glasgow_mem);

	MEPHISTO_SENSORS_BOARD(config, m_board);
	m_board->set_delay(attotime::from_msec(200));

	// video hardware
	MEPHISTO_DISPLAY_MODULE1(config, m_display);
	config.set_default_layout(layout_mephisto_glasgow);

	// sound hardware
	SPEAKER(config, "speaker").front_center();
	DAC_1BIT(config, m_dac).add_route(ALL_OUTPUTS, "speaker", 0.25);
}



/*******************************************************************************
    ROM Definitions
*******************************************************************************/

ROM_START( glasgow )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD16_BYTE("me3_3_1u.410", 0x00000, 0x04000, CRC(bc8053ba) SHA1(57ea2d5652bfdd77b17d52ab1914de974bd6be12) )
	ROM_LOAD16_BYTE("me3_1_1l.410", 0x00001, 0x04000, CRC(d5263c39) SHA1(1bef1cf3fd96221eb19faecb6ec921e26ac10ac4) )
	ROM_LOAD16_BYTE("me3_4_2u.410", 0x08000, 0x04000, CRC(8dba504a) SHA1(6bfab03af835cdb6c98773164d32c76520937efe) )
	ROM_LOAD16_BYTE("me3_2_2l.410", 0x08001, 0x04000, CRC(b3f27827) SHA1(864ba897d24024592d08c4ae090aa70a2cc5f213) )
ROM_END


ROM_START( amsterdg )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD16_BYTE("vr_1_1", 0x00000, 0x04000, CRC(a186cc81) SHA1(903f93243536de3c2778ba3d38dcf46ae568862d) )
	ROM_LOAD16_BYTE("vl_2_1", 0x00001, 0x04000, CRC(9b326226) SHA1(1b29319643d63a43ac84c1af08e02a4fc4fc6ffa) )
	ROM_LOAD16_BYTE("br_3_1", 0x08000, 0x04000, CRC(372fd7fe) SHA1(c7c11796450fe202e9641170cd0625461cee24af) )
	ROM_LOAD16_BYTE("bl_4_1", 0x08001, 0x04000, CRC(533e584a) SHA1(0e4510977dc627125c278920492bc137793a9554) )
ROM_END

ROM_START( dallas16g )
	ROM_REGION16_BE( 0x10000, "maincpu", 0 )
	ROM_LOAD16_BYTE("dal_g_pr", 0x00000, 0x04000, CRC(66deade9) SHA1(07ec6b923f2f053172737f1fc94aec84f3ea8da1) )
	ROM_LOAD16_BYTE("dal_g_pl", 0x00001, 0x04000, CRC(c5b6171c) SHA1(663167a3839ed7508ecb44fd5a1b2d3d8e466763) )
	ROM_LOAD16_BYTE("dal_g_br", 0x08000, 0x04000, CRC(e24d7ec7) SHA1(a936f6fcbe9bfa49bf455f2d8a8243d1395768c1) )
	ROM_LOAD16_BYTE("dal_g_bl", 0x08001, 0x04000, CRC(144a15e2) SHA1(c4fcc23d55fa5262f5e01dbd000644a7feb78f32) )
ROM_END

ROM_START( roma16g )
	ROM_REGION16_BE( 0x10000, "maincpu", 0 )
	ROM_LOAD16_BYTE("roma_r_low",  0x00000, 0x04000, CRC(f2312170) SHA1(82a50ba59f74365aa77478adaadbbace6693dcc1) )
	ROM_LOAD16_BYTE("roma_l_low",  0x00001, 0x04000, CRC(5fbb72cc) SHA1(458473a62f9f7394c9d02a6ad0939d8e19bae78b) )
	ROM_LOAD16_BYTE("roma_r_high", 0x08000, 0x04000, CRC(a55917db) SHA1(df9a9a96cdc1c9a7ed0dc70c4ddbb4278236a15f) )
	ROM_LOAD16_BYTE("roma_l_high", 0x08001, 0x04000, CRC(0b20617b) SHA1(f0296c486ce9009a69de1e50b90b0e1b7555f468) )
ROM_END

} // anonymous namespace



/*******************************************************************************
    Drivers
*******************************************************************************/

//    YEAR  NAME       PARENT    COMPAT  MACHINE   INPUT    CLASS          INIT        COMPANY, FULLNAME, FLAGS
SYST( 1984, glasgow,   0,        0,      glasgow,  glasgow, glasgow_state, empty_init, "Hegener + Glaser", "Mephisto III-S Glasgow", MACHINE_SUPPORTS_SAVE )

// newer chesscomputers on 4-ROM hardware (see amsterdam.cpp for parent sets)
SYST( 1985, amsterdg,  amsterd,  0,      glasgow,  glasgow, glasgow_state, empty_init, "Hegener + Glaser", "Mephisto Amsterdam (Glasgow hardware)", MACHINE_SUPPORTS_SAVE )
SYST( 1986, dallas16g, dallas32, 0,      glasgow,  glasgow, glasgow_state, empty_init, "Hegener + Glaser", "Mephisto Dallas 68000 (Glasgow hardware)", MACHINE_SUPPORTS_SAVE )
SYST( 1987, roma16g,   roma32,   0,      glasgow,  glasgow, glasgow_state, empty_init, "Hegener + Glaser", "Mephisto Roma 68000 (Glasgow hardware)", MACHINE_SUPPORTS_SAVE )
