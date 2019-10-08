// license:BSD-3-Clause
// copyright-holders:Dirk Verwiebe, Cowering
/***************************************************************************
Mephisto Glasgow 3 S chess computer
Dirk V.
sp_rinter@gmx.de

68000 CPU
64 KB ROM
16 KB RAM
4 Digit LCD

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
1*SG-10   Seiko 4-pin plastic XTAL chip "50H", to IPL0+2


How to play (quick guide)
1. You are the white player.
2. Click on the piece to move (LED starts flashing), then click where it goes
3. Computer plays black, it will work out its move and beep.
4. Read the move in the display, or look for the flashing LEDs.
5. Move the computer's piece in the same way you moved yours.
6. You'll need to read the official user manual for advanced features, or if
    you get messages such as "Err1".

TODO:
- add waitstates(applies to glasgow, amsterd, others?), CPU is 12MHz but with DTACK
  waitstates for slow EPROMs, effective speed is less than 10MHz
- LCD module is 8.8.:8.8 like mephisto_brikett/mm1 (so, add ":" in the middle)

***************************************************************************/

#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "machine/mmboard.h"
#include "sound/dac.h"
#include "sound/volt_reg.h"
#include "speaker.h"

// internal artwork
#include "mephisto_amsterdam.lh"
#include "mephisto_glasgow.lh"


class glasgow_state : public driver_device
{
public:
	glasgow_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_dac(*this, "dac")
		, m_board(*this, "board")
		, m_keyboard(*this, "LINE%u", 0)
		, m_digits(*this, "digit%u", 0U)
	{ }

	void glasgow(machine_config &config);

protected:
	DECLARE_WRITE8_MEMBER(glasgow_lcd_w);
	DECLARE_WRITE8_MEMBER(glasgow_lcd_flag_w);
	DECLARE_READ8_MEMBER(glasgow_keys_r);
	DECLARE_WRITE8_MEMBER(glasgow_keys_w);

	virtual void machine_start() override;
	virtual void machine_reset() override;

	void glasgow_mem(address_map &map);

	required_device<cpu_device> m_maincpu;
	required_device<dac_bit_interface> m_dac;
	required_device<mephisto_board_device> m_board;
	required_ioport_array<2> m_keyboard;
	output_finder<4> m_digits;

	uint8_t m_lcd_shift_counter;
	uint8_t m_led7;
	uint8_t m_key_select;
};


class amsterd_state : public glasgow_state
{
public:
	using glasgow_state::glasgow_state;

	void amsterd(machine_config &config);
	void dallas32(machine_config &config);

protected:
	DECLARE_WRITE8_MEMBER(write_lcd);
	DECLARE_WRITE8_MEMBER(write_lcd_flag);
	DECLARE_WRITE8_MEMBER(write_beeper);
	DECLARE_WRITE8_MEMBER(write_board);
	DECLARE_READ8_MEMBER(read_newkeys);

	void amsterd_mem(address_map &map);
	void dallas32_mem(address_map &map);
};



WRITE8_MEMBER( glasgow_state::glasgow_lcd_w )
{
	if (m_led7 == 0)
		m_digits[m_lcd_shift_counter] = data;

	m_lcd_shift_counter--;
	m_lcd_shift_counter &= 3;
}

WRITE8_MEMBER( glasgow_state::glasgow_lcd_flag_w )
{
	uint8_t const lcd_flag = data & 0x81;

	m_dac->write(BIT(lcd_flag, 0));

	if (lcd_flag)
		m_led7 = 255;
	else
		m_led7 = 0;
}

READ8_MEMBER( glasgow_state::glasgow_keys_r )
{
	// See if any keys pressed
	uint8_t data = 3;

	if (m_key_select == m_keyboard[0]->read())
		data &= 1;

	if (m_key_select == m_keyboard[1]->read())
		data &= 2;

	return data;
}

WRITE8_MEMBER( glasgow_state::glasgow_keys_w )
{
	m_key_select = data;
}

WRITE8_MEMBER( amsterd_state::write_lcd )
{
	if (m_lcd_shift_counter & 4)
		m_digits[m_lcd_shift_counter & 3] = data;

	m_lcd_shift_counter--;
	m_lcd_shift_counter &= 7;
}

WRITE8_MEMBER( amsterd_state::write_lcd_flag )
{
	m_key_select = 1;

	// The key function in the rom expects after writing to
	// the  a value from the second key row;
	m_led7 = data ? 255 : 0;
}

WRITE8_MEMBER( amsterd_state::write_board )
{
	m_key_select = 0;
	m_board->led_w(space, offset, 0);
	m_board->mux_w(space, offset, data);
}

WRITE8_MEMBER( amsterd_state::write_beeper )
{
	m_dac->write(BIT(data, 0));
}

READ8_MEMBER( amsterd_state::read_newkeys )
{
	return m_keyboard[m_key_select & 1]->read();
}

void glasgow_state::machine_start()
{
	m_digits.resolve();

	save_item(NAME(m_lcd_shift_counter));
	save_item(NAME(m_led7));
	save_item(NAME(m_key_select));
}


void glasgow_state::machine_reset()
{
	m_lcd_shift_counter = 3;
	m_key_select = 0;
	m_led7 = 0;
}


void glasgow_state::glasgow_mem(address_map &map)
{
	map.global_mask(0x1ffff);
	map(0x000000, 0x00ffff).rom();
	map(0x010000, 0x010000).w(FUNC(glasgow_state::glasgow_lcd_w));
	map(0x010002, 0x010002).rw(FUNC(glasgow_state::glasgow_keys_r), FUNC(glasgow_state::glasgow_keys_w));
	map(0x010004, 0x010004).w(FUNC(glasgow_state::glasgow_lcd_flag_w));
	map(0x010006, 0x010006).rw("board", FUNC(mephisto_board_device::input_r), FUNC(mephisto_board_device::led_w));
	map(0x010008, 0x010008).w("board", FUNC(mephisto_board_device::mux_w));
	map(0x01c000, 0x01ffff).ram(); // 16KB
}

void amsterd_state::amsterd_mem(address_map &map)
{
	map(0x000000, 0x00ffff).rom();
	map(0x800002, 0x800002).w(FUNC(amsterd_state::write_lcd));
	map(0x800008, 0x800008).w(FUNC(amsterd_state::write_lcd_flag));
	map(0x800004, 0x800004).w(FUNC(amsterd_state::write_beeper));
	map(0x800010, 0x800010).w(FUNC(amsterd_state::write_board));
	map(0x800020, 0x800020).r("board", FUNC(mephisto_board_device::input_r));
	map(0x800040, 0x800040).r(FUNC(amsterd_state::read_newkeys));
	map(0x800088, 0x800088).w("board", FUNC(mephisto_board_device::led_w));
	map(0xffc000, 0xffffff).ram(); // 16KB
}

void amsterd_state::dallas32_mem(address_map &map)
{
	map(0x000000, 0x00ffff).rom();
	map(0x010000, 0x01ffff).ram(); // 64KB
	map(0x800002, 0x800002).w(FUNC(amsterd_state::write_lcd));
	map(0x800004, 0x800004).w(FUNC(amsterd_state::write_beeper));
	map(0x800008, 0x800008).w(FUNC(amsterd_state::write_lcd_flag));
	map(0x800010, 0x800010).w(FUNC(amsterd_state::write_board));
	map(0x800020, 0x800020).r("board", FUNC(mephisto_board_device::input_r));
	map(0x800040, 0x800040).r(FUNC(amsterd_state::read_newkeys));
	map(0x800088, 0x800088).w("board", FUNC(mephisto_board_device::led_w));
}

static INPUT_PORTS_START( new_keyboard )
	PORT_START("LINE0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("A / 1") PORT_CODE(KEYCODE_A) PORT_CODE(KEYCODE_1) PORT_CODE(KEYCODE_1_PAD)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("B / 2 / Pawn") PORT_CODE(KEYCODE_B) PORT_CODE(KEYCODE_2) PORT_CODE(KEYCODE_2_PAD)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("C / 3 / Knight") PORT_CODE(KEYCODE_C) PORT_CODE(KEYCODE_3) PORT_CODE(KEYCODE_3_PAD)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("D / 4 / Bishop") PORT_CODE(KEYCODE_D) PORT_CODE(KEYCODE_4) PORT_CODE(KEYCODE_4_PAD)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("E / 5 / Rook") PORT_CODE(KEYCODE_E) PORT_CODE(KEYCODE_5) PORT_CODE(KEYCODE_5_PAD)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("F / 6 / Queen") PORT_CODE(KEYCODE_F) PORT_CODE(KEYCODE_6) PORT_CODE(KEYCODE_6_PAD)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Left / Black / 9") PORT_CODE(KEYCODE_9) PORT_CODE(KEYCODE_9_PAD) PORT_CODE(KEYCODE_LEFT)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Right / White / 0") PORT_CODE(KEYCODE_0) PORT_CODE(KEYCODE_0_PAD) PORT_CODE(KEYCODE_RIGHT)

	PORT_START("LINE1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("INFO") PORT_CODE(KEYCODE_I)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("POS") PORT_CODE(KEYCODE_O)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("LEV") PORT_CODE(KEYCODE_L)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("MEM") PORT_CODE(KEYCODE_M)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("CL") PORT_CODE(KEYCODE_BACKSPACE) PORT_CODE(KEYCODE_DEL)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("ENT") PORT_CODE(KEYCODE_ENTER) PORT_CODE(KEYCODE_ENTER_PAD)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("G / 7 / King") PORT_CODE(KEYCODE_G) PORT_CODE(KEYCODE_7) PORT_CODE(KEYCODE_7_PAD)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("H / 8") PORT_CODE(KEYCODE_H) PORT_CODE(KEYCODE_8) PORT_CODE(KEYCODE_8_PAD)
INPUT_PORTS_END

static INPUT_PORTS_START( old_keyboard )
	PORT_START("LINE0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Left / Black / 9") PORT_CODE(KEYCODE_9) PORT_CODE(KEYCODE_9_PAD) PORT_CODE(KEYCODE_LEFT)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("CL") PORT_CODE(KEYCODE_BACKSPACE) PORT_CODE(KEYCODE_DEL)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("C / 3 / Knight") PORT_CODE(KEYCODE_C) PORT_CODE(KEYCODE_3) PORT_CODE(KEYCODE_3_PAD)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("ENT") PORT_CODE(KEYCODE_ENTER) PORT_CODE(KEYCODE_ENTER_PAD)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("D / 4 / Bishop") PORT_CODE(KEYCODE_D) PORT_CODE(KEYCODE_4) PORT_CODE(KEYCODE_4_PAD)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("A / 1") PORT_CODE(KEYCODE_A) PORT_CODE(KEYCODE_1) PORT_CODE(KEYCODE_1_PAD)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("F / 6 / Queen") PORT_CODE(KEYCODE_F) PORT_CODE(KEYCODE_6) PORT_CODE(KEYCODE_6_PAD)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("B / 2 / Pawn") PORT_CODE(KEYCODE_B) PORT_CODE(KEYCODE_2) PORT_CODE(KEYCODE_2_PAD)

	PORT_START("LINE1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("E / 5 / Rook") PORT_CODE(KEYCODE_E) PORT_CODE(KEYCODE_5) PORT_CODE(KEYCODE_5_PAD)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("INFO") PORT_CODE(KEYCODE_I)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Right / White / 0") PORT_CODE(KEYCODE_0) PORT_CODE(KEYCODE_0_PAD) PORT_CODE(KEYCODE_RIGHT)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("POS") PORT_CODE(KEYCODE_O)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("H / 8") PORT_CODE(KEYCODE_H) PORT_CODE(KEYCODE_8) PORT_CODE(KEYCODE_8_PAD)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("LEV") PORT_CODE(KEYCODE_L)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("G / 7 / King") PORT_CODE(KEYCODE_G) PORT_CODE(KEYCODE_7) PORT_CODE(KEYCODE_7_PAD)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("MEM") PORT_CODE(KEYCODE_M)
INPUT_PORTS_END


void glasgow_state::glasgow(machine_config &config)
{
	/* basic machine hardware */
	M68000(config, m_maincpu, 12_MHz_XTAL);
	m_maincpu->set_periodic_int(FUNC(glasgow_state::irq5_line_hold), attotime::from_hz(50));
	m_maincpu->set_addrmap(AS_PROGRAM, &glasgow_state::glasgow_mem);

	MEPHISTO_SENSORS_BOARD(config, m_board);
	m_board->set_delay(attotime::from_msec(200));

	/* video hardware */
	config.set_default_layout(layout_mephisto_glasgow);

	/* sound hardware */
	SPEAKER(config, "speaker").front_center();
	DAC_1BIT(config, m_dac).add_route(ALL_OUTPUTS, "speaker", 0.25);
	VOLTAGE_REGULATOR(config, "vref").add_route(0, "dac", 1.0, DAC_VREF_POS_INPUT);
}

void amsterd_state::amsterd(machine_config &config)
{
	glasgow(config);

	/* basic machine hardware */
	m_maincpu->set_addrmap(AS_PROGRAM, &amsterd_state::amsterd_mem);

	config.set_default_layout(layout_mephisto_amsterdam);
}

void amsterd_state::dallas32(machine_config &config)
{
	amsterd(config);

	/* basic machine hardware */
	M68020(config.replace(), m_maincpu, 14_MHz_XTAL);
	m_maincpu->set_periodic_int(FUNC(amsterd_state::irq5_line_hold), attotime::from_hz(50));
	m_maincpu->set_addrmap(AS_PROGRAM, &amsterd_state::dallas32_mem);
}


/***************************************************************************
  ROM definitions
***************************************************************************/

ROM_START( glasgow )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD16_BYTE("me3_3_1u.410", 0x00000, 0x04000, CRC(bc8053ba) SHA1(57ea2d5652bfdd77b17d52ab1914de974bd6be12) )
	ROM_LOAD16_BYTE("me3_1_1l.410", 0x00001, 0x04000, CRC(d5263c39) SHA1(1bef1cf3fd96221eb19faecb6ec921e26ac10ac4) )
	ROM_LOAD16_BYTE("me3_4_2u.410", 0x08000, 0x04000, CRC(8dba504a) SHA1(6bfab03af835cdb6c98773164d32c76520937efe) )
	ROM_LOAD16_BYTE("me3_2_2l.410", 0x08001, 0x04000, CRC(b3f27827) SHA1(864ba897d24024592d08c4ae090aa70a2cc5f213) )
ROM_END

ROM_START( amsterd )
	ROM_REGION16_BE( 0x1000000, "maincpu", 0 )
	ROM_LOAD16_BYTE("amsterda-u.bin", 0x00000, 0x08000, CRC(0a75514e) SHA1(27daf78b0aba4d7a293b96b3c1fa92f6ee9bcb59) )
	ROM_LOAD16_BYTE("amsterda-l.bin", 0x00001, 0x08000, CRC(6e17d8fa) SHA1(e0f9e57aaa445f6ff7cbe868658ed7bcfa7e31fb) )
ROM_END

ROM_START( amsterda )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD16_BYTE("vr_1_1", 0x00000, 0x04000, CRC(a186cc81) SHA1(903f93243536de3c2778ba3d38dcf46ae568862d) )
	ROM_LOAD16_BYTE("vl_2_1", 0x00001, 0x04000, CRC(9b326226) SHA1(1b29319643d63a43ac84c1af08e02a4fc4fc6ffa) )
	ROM_LOAD16_BYTE("br_3_1", 0x08000, 0x04000, CRC(372fd7fe) SHA1(c7c11796450fe202e9641170cd0625461cee24af) )
	ROM_LOAD16_BYTE("bl_4_1", 0x08001, 0x04000, CRC(533e584a) SHA1(0e4510977dc627125c278920492bc137793a9554) )
ROM_END

ROM_START( dallas32 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("dallas32.epr", 0x000000, 0x10000, CRC(83b9ff3f) SHA1(97bf4cb3c61f8ec328735b3c98281bba44b30a28) )
ROM_END

ROM_START( dallas16 )
	ROM_REGION16_BE( 0x1000000, "maincpu", 0 )
	ROM_LOAD16_BYTE("dallas-u.bin", 0x00000, 0x08000, CRC(70b741f7) SHA1(23d55ed0fea127b727d725c367ee1932ff5af39f) )
	ROM_LOAD16_BYTE("dallas-l.bin", 0x00001, 0x08000, CRC(69300ad3) SHA1(57ec1b955b1ddfe722011ff5da68a0cd71af9251) )
ROM_END

ROM_START( dallas16a )
	ROM_REGION16_BE( 0x1000000, "maincpu", 0 )
	ROM_LOAD16_BYTE("dal_g_pr", 0x00000, 0x04000, CRC(66deade9) SHA1(07ec6b923f2f053172737f1fc94aec84f3ea8da1) )
	ROM_LOAD16_BYTE("dal_g_pl", 0x00001, 0x04000, CRC(c5b6171c) SHA1(663167a3839ed7508ecb44fd5a1b2d3d8e466763) )
	ROM_LOAD16_BYTE("dal_g_br", 0x08000, 0x04000, CRC(e24d7ec7) SHA1(a936f6fcbe9bfa49bf455f2d8a8243d1395768c1) )
	ROM_LOAD16_BYTE("dal_g_bl", 0x08001, 0x04000, CRC(144a15e2) SHA1(c4fcc23d55fa5262f5e01dbd000644a7feb78f32) )
ROM_END

ROM_START( roma32 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("roma32.bin", 0x000000, 0x10000, CRC(587d03bf) SHA1(504e9ff958084700076d633f9c306fc7baf64ffd) )
ROM_END

ROM_START( roma16 )
	ROM_REGION16_BE( 0x1000000, "maincpu", 0 )
	ROM_LOAD16_BYTE("roma16-u.bin", 0x00000, 0x08000, CRC(111d030f) SHA1(e027f7e7018d28ab794e7730392506056809db6b) )
	ROM_LOAD16_BYTE("roma16-l.bin", 0x00001, 0x08000, CRC(8245ddd2) SHA1(ab048b60fdc4358913a5d07b6fee863b66dd6734) )
ROM_END

ROM_START( roma16a )
	ROM_REGION16_BE( 0x1000000, "maincpu", 0 )
	ROM_LOAD16_BYTE("roma_r_low",  0x00000, 0x04000, CRC(f2312170) SHA1(82a50ba59f74365aa77478adaadbbace6693dcc1) )
	ROM_LOAD16_BYTE("roma_l_low",  0x00001, 0x04000, CRC(5fbb72cc) SHA1(458473a62f9f7394c9d02a6ad0939d8e19bae78b) )
	ROM_LOAD16_BYTE("roma_r_high", 0x08000, 0x04000, CRC(a55917db) SHA1(df9a9a96cdc1c9a7ed0dc70c4ddbb4278236a15f) )
	ROM_LOAD16_BYTE("roma_l_high", 0x08001, 0x04000, CRC(0b20617b) SHA1(f0296c486ce9009a69de1e50b90b0e1b7555f468) )
ROM_END


/***************************************************************************
  Game drivers
***************************************************************************/

/*    YEAR, NAME,      PARENT    COMPAT  MACHINE   INPUT         CLASS          INIT        COMPANY             FULLNAME                  FLAGS */
CONS( 1984, glasgow,   0,        0,      glasgow,  old_keyboard, glasgow_state, empty_init, "Hegener + Glaser", "Mephisto III-S Glasgow", MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK )

CONS( 1985, amsterd,   0,        0,      amsterd,  new_keyboard, amsterd_state, empty_init, "Hegener + Glaser", "Mephisto Amsterdam",     MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK )
CONS( 1985, amsterda,  amsterd,  0,      glasgow,  old_keyboard, glasgow_state, empty_init, "Hegener + Glaser", "Mephisto Amsterdam (Glasgow hardware)", MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK )

CONS( 1986, dallas32,  0,        0,      dallas32, new_keyboard, amsterd_state, empty_init, "Hegener + Glaser", "Mephisto Dallas 68020",  MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK )
CONS( 1986, dallas16,  dallas32, 0,      amsterd,  new_keyboard, amsterd_state, empty_init, "Hegener + Glaser", "Mephisto Dallas 68000",  MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK )
CONS( 1986, dallas16a, dallas32, 0,      glasgow,  old_keyboard, glasgow_state, empty_init, "Hegener + Glaser", "Mephisto Dallas 68000 (Glasgow hardware)", MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK )

CONS( 1987, roma32,    0,        0,      dallas32, new_keyboard, amsterd_state, empty_init, "Hegener + Glaser", "Mephisto Roma 68020",    MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK )
CONS( 1987, roma16,    roma32,   0,      amsterd,  new_keyboard, amsterd_state, empty_init, "Hegener + Glaser", "Mephisto Roma 68000",    MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK )
CONS( 1987, roma16a,   roma32,   0,      glasgow,  old_keyboard, glasgow_state, empty_init, "Hegener + Glaser", "Mephisto Roma 68000 (Glasgow hardware)", MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK )
