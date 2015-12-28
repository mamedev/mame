// license:LGPL-2.1+
// copyright-holders:Dirk Verwiebe, Robbbert, Cowering
/***************************************************************************
Mephisto Glasgow 3 S chess computer
Dirk V.
sp_rinter@gmx.de

68000 CPU
64 KB ROM
16 KB RAM
4 Digit LC Display

3* 74LS138  Decoder/Multiplexer
1*74LS74    Dual positive edge triggered D Flip Flop
1*74LS139 1of4 Demultiplexer
1*74LS05    HexInverter
1*NE555     R=100K C=10uF
2*74LS04  Hex Inverter
1*74LS164   8 Bit Shift register
1*74121 Monostable Multivibrator with Schmitt Trigger Inputs
1*74LS20 Dual 4 Input NAND GAte
1*74LS367 3 State Hex Buffers


Made playable by Robbbert in Nov 2009.

How to play (quick guide)
1. You are the white player.
2. Click on the piece to move (LED starts flashing), then click where it goes
3. Computer plays black, it will work out its move and beep.
4. Read the move in the display, or look for the flashing LEDs.
5. Move the computer's piece in the same way you moved yours.
6. If a piece is being taken, firstly click on the piece then click the board
    edge. This causes the piece to disappear. After that, move the piece that
    took the other piece.
7. You'll need to read the official user manual for advanced features, or if
    you get messages such as "Err1".

Note about clickable artwork: You need to be running in windowed mode;
    and you need to use newui.

R.Schaefer Oct 2010

1. everything concerning chessboard moved to machine mboard
2. Border pieces added. This allow setting up and repair chess positons
3. chessboard added for Amsterdam, Dallas 16 Bit, Dallas 32 Bit, Roma 32 Bit
4. Save states added.

***************************************************************************/

#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "glasgow.lh"
#include "sound/beep.h"
#include "includes/mboard.h"


class glasgow_state : public mboard_state
{
public:
	glasgow_state(const machine_config &mconfig, device_type type, const char *tag)
		: mboard_state(mconfig, type, tag),
	m_maincpu(*this, "maincpu"),
	m_beep(*this, "beeper"),
	m_line0(*this, "LINE0"),
	m_line1(*this, "LINE1")
	{ }

	required_device<cpu_device> m_maincpu;
	required_device<beep_device> m_beep;
	DECLARE_WRITE16_MEMBER(glasgow_lcd_w);
	DECLARE_WRITE16_MEMBER(glasgow_lcd_flag_w);
	DECLARE_READ16_MEMBER(glasgow_keys_r);
	DECLARE_WRITE16_MEMBER(glasgow_keys_w);
	DECLARE_WRITE16_MEMBER(write_lcd);
	DECLARE_WRITE16_MEMBER(write_lcd_flag);
	DECLARE_WRITE16_MEMBER(write_irq_flag);
	DECLARE_READ16_MEMBER(read_newkeys16);
	DECLARE_WRITE32_MEMBER(write_lcd32);
	DECLARE_WRITE32_MEMBER(write_lcd_flag32);
	DECLARE_READ32_MEMBER(read_newkeys32);
	DECLARE_WRITE32_MEMBER(write_beeper32);
	UINT8 m_lcd_shift_counter;
	UINT8 m_led7;
	UINT8 m_irq_flag;
	UINT16 m_beeper;
	virtual void machine_start() override;
	virtual void machine_reset() override;
	DECLARE_MACHINE_START(dallas32);
	TIMER_DEVICE_CALLBACK_MEMBER(update_nmi);
	TIMER_DEVICE_CALLBACK_MEMBER(update_nmi32);
	required_ioport m_line0;
	required_ioport m_line1;
};



WRITE16_MEMBER( glasgow_state::glasgow_lcd_w )
{
	UINT8 lcd_data = data >> 8;

	if (m_led7 == 0)
		output_set_digit_value(m_lcd_shift_counter, lcd_data);

	m_lcd_shift_counter--;
	m_lcd_shift_counter &= 3;
}

WRITE16_MEMBER( glasgow_state::glasgow_lcd_flag_w )
{
	UINT16 lcd_flag = data & 0x8100;

	m_beep->set_state(BIT(lcd_flag, 8));

	if (lcd_flag)
		m_led7 = 255;
	else
	{
		m_led7 = 0;
		mboard_key_selector = 1;
	}
}

READ16_MEMBER( glasgow_state::glasgow_keys_r )
{
	UINT8 data = 0xff;

	/* See if any keys pressed */
	data = 3;

	if (mboard_key_select == m_line0->read())
		data &= 1;

	if (mboard_key_select == m_line1->read())
		data &= 2;

	return data << 8;
}

WRITE16_MEMBER( glasgow_state::glasgow_keys_w )
{
	mboard_key_select = data >> 8;
}

WRITE16_MEMBER( glasgow_state::write_lcd )
{
	UINT8 lcd_data = data >> 8;

	output_set_digit_value(m_lcd_shift_counter, mboard_lcd_invert & 1 ? lcd_data^0xff : lcd_data);
	m_lcd_shift_counter--;
	m_lcd_shift_counter &= 3;
	logerror("LCD Offset = %d Data low = %x \n", offset, lcd_data);
}

WRITE16_MEMBER( glasgow_state::write_lcd_flag )
{
//  UINT8 lcd_flag;
	mboard_lcd_invert = 0;
//  lcd_flag=data >> 8;
	//m_beep->set_state((data >> 8) & 1 ? 1 : 0);
	if ((data >> 8) == 0)
	{
		mboard_key_selector = 1;
		m_led7 = 0;
	}
	else
		m_led7 = 0xff;


//  The key function in the rom expects after writing to
//  the  a value from the second key row;
//  if (lcd_flag != 0)
//      m_led7 = 255;
//  else
//      m_led7 = 0;
//
	logerror("LCD Flag 16 = %x \n", data);
}

WRITE16_MEMBER( glasgow_state::write_irq_flag )
{
	m_beep->set_state(data & 0x100);
	logerror("Write 0x800004 = %x \n", data);
	m_irq_flag = 1;
	m_beeper = data;
}

READ16_MEMBER( glasgow_state::read_newkeys16 )  //Amsterdam, Roma
{
	UINT16 data;

	if (mboard_key_selector == 0)
		data = m_line0->read();
	else
		data = m_line1->read();

	logerror("read Keyboard Offset = %x Data = %x Select = %x \n", offset, data, mboard_key_selector);
	data <<= 8;
	return data ;
}


#ifdef UNUSED_FUNCTION
READ16_MEMBER(glasgow_state::read_test)
{
	logerror("read test Offset = %x Data = %x\n  ",offset,data);
	return 0xffff;    // Mephisto need it for working
}
#endif

/*

    *****           32 Bit Read and write Handler   ***********

*/

WRITE32_MEMBER( glasgow_state::write_lcd32 )
{
	UINT8 lcd_data = data >> 8;

	output_set_digit_value(m_lcd_shift_counter, mboard_lcd_invert & 1 ? lcd_data^0xff : lcd_data);
	m_lcd_shift_counter--;
	m_lcd_shift_counter &= 3;
	//logerror("LCD Offset = %d Data   = %x \n  ", offset, lcd_data);
}

WRITE32_MEMBER( glasgow_state::write_lcd_flag32 )
{
//  UINT8 lcd_flag = data >> 24;

	mboard_lcd_invert = 0;

	if ((data >> 24) == 0)
	{
		mboard_key_selector = 1;
		m_led7 = 0;
	}
	else
		m_led7 = 0xff;


	//logerror("LCD Flag 32 = %x \n", data >> 24);
	//m_beep->set_state((data >> 24) & 1 ? 1 : 0);

//  if (lcd_flag != 0)
//      m_led7 = 255;
//  else
//      m_led7 = 0;
}

READ32_MEMBER( glasgow_state::read_newkeys32 ) // Dallas 32, Roma 32
{
	UINT32 data;

	if (mboard_key_selector == 0)
		data = m_line0->read();
	else
		data = m_line1->read();
	//if (mboard_key_selector == 1) data = m_line0->read(); else data = 0;
	if(data)
		logerror("read Keyboard Offset = %x Data = %x\n", offset, data);
	data <<= 24;
	return data ;
}

#ifdef UNUSED_FUNCTION
READ16_MEMBER(glasgow_state::read_board_amsterd)
{
	logerror("read board amsterdam Offset = %x \n  ", offset);
	return 0xffff;
}
#endif

WRITE32_MEMBER( glasgow_state::write_beeper32 )
{
	m_beep->set_state(data & 0x01000000);
	logerror("Write 0x8000004 = %x \n", data);
	m_irq_flag = 1;
	m_beeper = data;
}

TIMER_DEVICE_CALLBACK_MEMBER(glasgow_state::update_nmi)
{
	m_maincpu->set_input_line(7, HOLD_LINE);
}

TIMER_DEVICE_CALLBACK_MEMBER(glasgow_state::update_nmi32)
{
	m_maincpu->set_input_line(6, HOLD_LINE);
}

void glasgow_state::machine_start()
{
	mboard_key_selector = 0;
	m_irq_flag = 0;
	m_lcd_shift_counter = 3;
	m_beep->set_frequency(44);

	mboard_savestate_register();
}


MACHINE_START_MEMBER(glasgow_state,dallas32)
{
	m_lcd_shift_counter = 3;
	m_beep->set_frequency(44);

	mboard_savestate_register();
}


void glasgow_state::machine_reset()
{
	m_lcd_shift_counter = 3;

	mboard_set_border_pieces();
	mboard_set_board();
}

static ADDRESS_MAP_START(glasgow_mem, AS_PROGRAM, 16, glasgow_state)
	ADDRESS_MAP_GLOBAL_MASK(0x1FFFF)
	AM_RANGE(0x00000000, 0x0000ffff) AM_ROM
	AM_RANGE(0x00010000, 0x00010001) AM_WRITE(glasgow_lcd_w)
	AM_RANGE(0x00010002, 0x00010003) AM_READWRITE(glasgow_keys_r,glasgow_keys_w)
	AM_RANGE(0x00010004, 0x00010005) AM_WRITE(glasgow_lcd_flag_w)
	AM_RANGE(0x00010006, 0x00010007) AM_READWRITE(mboard_read_board_16,mboard_write_LED_16)
	AM_RANGE(0x00010008, 0x00010009) AM_WRITE(mboard_write_board_16)
	AM_RANGE(0x0001c000, 0x0001ffff) AM_RAM     // 16KB
ADDRESS_MAP_END


static ADDRESS_MAP_START(amsterd_mem, AS_PROGRAM, 16, glasgow_state)
	// ADDRESS_MAP_GLOBAL_MASK(0x7FFFF)
	AM_RANGE(0x00000000, 0x0000ffff) AM_ROM
	AM_RANGE(0x00800002, 0x00800003) AM_WRITE(write_lcd)
	AM_RANGE(0x00800008, 0x00800009) AM_WRITE(write_lcd_flag)
	AM_RANGE(0x00800004, 0x00800005) AM_WRITE(write_irq_flag)
	AM_RANGE(0x00800010, 0x00800011) AM_WRITE(mboard_write_board_16)
	AM_RANGE(0x00800020, 0x00800021) AM_READ(mboard_read_board_16)
	AM_RANGE(0x00800040, 0x00800041) AM_READ(read_newkeys16)
	AM_RANGE(0x00800088, 0x00800089) AM_WRITE(mboard_write_LED_16)
	AM_RANGE(0x00ffc000, 0x00ffffff) AM_RAM     // 16KB
ADDRESS_MAP_END


static ADDRESS_MAP_START(dallas32_mem, AS_PROGRAM, 32, glasgow_state)
	// ADDRESS_MAP_GLOBAL_MASK(0x1FFFF)
	AM_RANGE(0x00000000, 0x0000ffff) AM_ROM
	AM_RANGE(0x00010000, 0x0001ffff) AM_RAM // 64KB
	AM_RANGE(0x00800000, 0x00800003) AM_WRITE(write_lcd32)
	AM_RANGE(0x00800004, 0x00800007) AM_WRITE(write_beeper32)
	AM_RANGE(0x00800008, 0x0080000B) AM_WRITE(write_lcd_flag32)
	AM_RANGE(0x00800010, 0x00800013) AM_WRITE(mboard_write_board_32)
	AM_RANGE(0x00800020, 0x00800023) AM_READ(mboard_read_board_32)
	AM_RANGE(0x00800040, 0x00800043) AM_READ(read_newkeys32)
	AM_RANGE(0x00800088, 0x0080008b) AM_WRITE(mboard_write_LED_32)
ADDRESS_MAP_END

static INPUT_PORTS_START( new_keyboard ) //Amsterdam, Dallas 32, Roma, Roma 32
	PORT_START("LINE0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("A 1") PORT_CODE(KEYCODE_A) PORT_CODE(KEYCODE_1)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("B 2") PORT_CODE(KEYCODE_B) PORT_CODE(KEYCODE_2)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("C 3") PORT_CODE(KEYCODE_C) PORT_CODE(KEYCODE_3)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("D 4") PORT_CODE(KEYCODE_D) PORT_CODE(KEYCODE_4)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("E 5") PORT_CODE(KEYCODE_E) PORT_CODE(KEYCODE_5)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("F 6") PORT_CODE(KEYCODE_F) PORT_CODE(KEYCODE_6)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("0") PORT_CODE(KEYCODE_9)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("9") PORT_CODE(KEYCODE_0)

	PORT_START("LINE1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("INF") PORT_CODE(KEYCODE_F1)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("POS") PORT_CODE(KEYCODE_F2)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("LEV") PORT_CODE(KEYCODE_F3)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("MEM") PORT_CODE(KEYCODE_F4)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("CLR") PORT_CODE(KEYCODE_F5)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("ENT") PORT_CODE(KEYCODE_ENTER)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("G 7") PORT_CODE(KEYCODE_G) PORT_CODE(KEYCODE_7)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("H 8") PORT_CODE(KEYCODE_H) PORT_CODE(KEYCODE_8)
INPUT_PORTS_END

static INPUT_PORTS_START( old_keyboard )   //Glasgow,Dallas
	PORT_START("LINE0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("9") PORT_CODE(KEYCODE_9)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("CL") PORT_CODE(KEYCODE_F5)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("C 3") PORT_CODE(KEYCODE_C) PORT_CODE(KEYCODE_3)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("ENT") PORT_CODE(KEYCODE_ENTER)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("D 4") PORT_CODE(KEYCODE_D) PORT_CODE(KEYCODE_4)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("A 1") PORT_CODE(KEYCODE_A) PORT_CODE(KEYCODE_1)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F 6") PORT_CODE(KEYCODE_F) PORT_CODE(KEYCODE_6)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("B 2") PORT_CODE(KEYCODE_B) PORT_CODE(KEYCODE_2)

	PORT_START("LINE1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("E 5") PORT_CODE(KEYCODE_E) PORT_CODE(KEYCODE_5)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("INF") PORT_CODE(KEYCODE_F1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("0") PORT_CODE(KEYCODE_0)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("POS") PORT_CODE(KEYCODE_F2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("H 8") PORT_CODE(KEYCODE_H) PORT_CODE(KEYCODE_8)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("LEV") PORT_CODE(KEYCODE_F3)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("G 7") PORT_CODE(KEYCODE_G) PORT_CODE(KEYCODE_7)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("MEM") PORT_CODE(KEYCODE_F4)
INPUT_PORTS_END


INPUT_PORTS_EXTERN( chessboard);

static INPUT_PORTS_START( oldkeys )
	PORT_INCLUDE( old_keyboard )
	PORT_INCLUDE( chessboard )
INPUT_PORTS_END

static INPUT_PORTS_START( newkeys )
	PORT_INCLUDE( new_keyboard )
	PORT_INCLUDE( chessboard )
INPUT_PORTS_END

static MACHINE_CONFIG_START( glasgow, glasgow_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68000, 12000000)
	MCFG_CPU_PROGRAM_MAP(glasgow_mem)
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("beeper", BEEP, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)

	MCFG_TIMER_DRIVER_ADD_PERIODIC("nmi_timer", glasgow_state, update_nmi, attotime::from_hz(50))
	MCFG_TIMER_DRIVER_ADD_PERIODIC("artwork_timer", glasgow_state, mboard_update_artwork, attotime::from_hz(100))
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( amsterd, glasgow )
	/* basic machine hardware */
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(amsterd_mem)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( dallas32, glasgow )
	/* basic machine hardware */
	MCFG_CPU_REPLACE("maincpu", M68020, 14000000)
	MCFG_CPU_PROGRAM_MAP(dallas32_mem)
	MCFG_MACHINE_START_OVERRIDE(glasgow_state, dallas32 )

	MCFG_DEVICE_REMOVE("nmi_timer")
	MCFG_TIMER_DRIVER_ADD_PERIODIC("nmi_timer", glasgow_state, update_nmi32, attotime::from_hz(50))

MACHINE_CONFIG_END

/***************************************************************************
  ROM definitions
***************************************************************************/

ROM_START( glasgow )
	ROM_REGION( 0x10000, "maincpu", 0 )
	//ROM_LOAD("glasgow.rom", 0x000000, 0x10000, CRC(3e73eff3) )
	ROM_LOAD16_BYTE("me3_3_1u.410",0x00000, 0x04000,CRC(bc8053ba) SHA1(57ea2d5652bfdd77b17d52ab1914de974bd6be12))
	ROM_LOAD16_BYTE("me3_1_1l.410",0x00001, 0x04000,CRC(d5263c39) SHA1(1bef1cf3fd96221eb19faecb6ec921e26ac10ac4))
	ROM_LOAD16_BYTE("me3_4_2u.410",0x08000, 0x04000,CRC(8dba504a) SHA1(6bfab03af835cdb6c98773164d32c76520937efe))
	ROM_LOAD16_BYTE("me3_2_2l.410",0x08001, 0x04000,CRC(b3f27827) SHA1(864ba897d24024592d08c4ae090aa70a2cc5f213))
ROM_END

ROM_START( amsterd )
	ROM_REGION16_BE( 0x1000000, "maincpu", 0 )
	//ROM_LOAD16_BYTE("output.bin", 0x000000, 0x10000, CRC(3e73eff3) )
	ROM_LOAD16_BYTE("amsterda-u.bin",0x00000, 0x05a00,CRC(16cefe29) SHA1(9f8c2896e92fbfd47159a59cb5e87706092c86f4))
	ROM_LOAD16_BYTE("amsterda-l.bin",0x00001, 0x05a00,CRC(c859dfde) SHA1(b0bca6a8e698c322a8c597608db6735129d6cdf0))
ROM_END


ROM_START( dallas )
	ROM_REGION16_BE( 0x1000000, "maincpu", 0 )
	ROM_LOAD16_BYTE("dal_g_pr.dat",0x00000, 0x04000,CRC(66deade9) SHA1(07ec6b923f2f053172737f1fc94aec84f3ea8da1))
	ROM_LOAD16_BYTE("dal_g_pl.dat",0x00001, 0x04000,CRC(c5b6171c) SHA1(663167a3839ed7508ecb44fd5a1b2d3d8e466763))
	ROM_LOAD16_BYTE("dal_g_br.dat",0x08000, 0x04000,CRC(e24d7ec7) SHA1(a936f6fcbe9bfa49bf455f2d8a8243d1395768c1))
	ROM_LOAD16_BYTE("dal_g_bl.dat",0x08001, 0x04000,CRC(144a15e2) SHA1(c4fcc23d55fa5262f5e01dbd000644a7feb78f32))
ROM_END

ROM_START( dallas16 )
	ROM_REGION16_BE( 0x1000000, "maincpu", 0 )
	ROM_LOAD16_BYTE("dallas-u.bin",0x00000, 0x06f00,CRC(8c1462b4) SHA1(8b5f5a774a835446d08dceacac42357b9e74cfe8))
	ROM_LOAD16_BYTE("dallas-l.bin",0x00001, 0x06f00,CRC(f0d5bc03) SHA1(4b1b9a71663d5321820b4cf7da205e5fe5d3d001))
ROM_END

ROM_START( roma )
	ROM_REGION16_BE( 0x1000000, "maincpu", 0 )
	ROM_LOAD("roma32.bin", 0x000000, 0x10000, CRC(587d03bf) SHA1(504e9ff958084700076d633f9c306fc7baf64ffd))
ROM_END

ROM_START( dallas32 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("dallas32.epr", 0x000000, 0x10000, CRC(83b9ff3f) SHA1(97bf4cb3c61f8ec328735b3c98281bba44b30a28) )
ROM_END

ROM_START( roma32 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("roma32.bin", 0x000000, 0x10000, CRC(587d03bf) SHA1(504e9ff958084700076d633f9c306fc7baf64ffd) )
ROM_END


/***************************************************************************
  Game drivers
***************************************************************************/

/*     YEAR, NAME,     PARENT,   COMPAT, MACHINE,     INPUT,          INIT, COMPANY,                      FULLNAME,                 FLAGS */
CONS(  1984, glasgow,  0,        0,      glasgow,     oldkeys, driver_device,        0, "Hegener & Glaser Muenchen", "Mephisto III S Glasgow", MACHINE_SUPPORTS_SAVE)
CONS(  1984, amsterd,  0,        0,      amsterd,     newkeys, driver_device,        0, "Hegener & Glaser Muenchen", "Mephisto Amsterdam",     MACHINE_SUPPORTS_SAVE)
CONS(  1984, dallas,   glasgow,  0,      glasgow,     oldkeys, driver_device,        0, "Hegener & Glaser Muenchen", "Mephisto Dallas",        MACHINE_SUPPORTS_SAVE)
CONS(  1984, roma,     amsterd,  0,      glasgow,     newkeys, driver_device,        0, "Hegener & Glaser Muenchen", "Mephisto Roma",          MACHINE_NOT_WORKING)
CONS(  1984, dallas32, amsterd,  0,      dallas32,    newkeys, driver_device,        0, "Hegener & Glaser Muenchen", "Mephisto Dallas 32 Bit", MACHINE_SUPPORTS_SAVE)
CONS(  1984, roma32,   amsterd,  0,      dallas32,    newkeys, driver_device,        0, "Hegener & Glaser Muenchen", "Mephisto Roma 32 Bit",   MACHINE_SUPPORTS_SAVE)
CONS(  1984, dallas16, amsterd,  0,      amsterd,     newkeys, driver_device,        0, "Hegener & Glaser Muenchen", "Mephisto Dallas 16 Bit", MACHINE_SUPPORTS_SAVE)
