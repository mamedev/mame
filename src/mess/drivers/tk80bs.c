/***************************************************************************



NEC TK80 / MIKROLAB KR580IK80
*****************************
TK80 driver by Robbbert
Mikrolab driver by Micko
Merged by Robbbert.

TK80 (Training Kit 80) considered to be Japan's first home computer.
It consisted of 25 keys and 8 LED digits, and was programmed in hex.
The Mikrolab is a Russian clone which appears to be almost completely identical.

TK85 seems to be the same as TK80, except is has a larger ROM. No
schematics etc are available. Thanks to 'Nama' who dumped the rom.
It has 25 keys, so a few aren't defined yet.

When booted, the system begins at 0000 which is ROM. You need to change the
address to 8000 before entering a program. Here is a test to paste in:
8000-11^22^33^44^55^66^77^88^99^8000-
Press the right-arrow to confirm data has been entered.

Operation:
4 digits at left is the address; 2 digits at right is the data.
As you increment addresses, the middle 2 digits show the previous byte.
You can enter 4 digits, and pressing 'ADRS SET' will transfer this info
to the left, thus setting the address to this value. Press 'WRITE INCR' to
store new data and increment the address. Press 'READ INCR' and 'READ DECR'
to scan through data without updating it. Other keys unknown/not implemented.

ToDo:
- Add storage


NEC TK80BS
**********
TK-80BS (c) 1980 NEC

Preliminary driver by Angelo Salese

The TK80BS (Basic Station) has a plugin keyboard, BASIC in rom,
and connected to a tv.

TODO:
    - (try to) dump proper roms, the whole driver is based off fake roms;
    - BASIC doesn't seem to work properly; (It does if you type NEW first)


****************************************************************************/

#include "emu.h"
#include "cpu/i8085/i8085.h"
#include "machine/i8255.h"
#include "machine/keyboard.h"
#include "tk80.lh"


class tk80bs_state : public driver_device
{
public:
	tk80bs_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
	m_maincpu(*this, "maincpu"),
	m_ppi8255_2(*this, "ppi8255_2"),
	m_p_videoram(*this, "videoram"){ }

	DECLARE_READ8_MEMBER(ppi_custom_r);
	DECLARE_WRITE8_MEMBER(ppi_custom_w);
	DECLARE_READ8_MEMBER(key_matrix_r);
	DECLARE_READ8_MEMBER(serial_r);
	DECLARE_WRITE8_MEMBER(serial_w);
	DECLARE_WRITE8_MEMBER(mikrolab_serial_w);
	DECLARE_READ8_MEMBER(display_r);
	DECLARE_WRITE8_MEMBER(display_w);
	DECLARE_WRITE8_MEMBER(kbd_put);
	DECLARE_READ8_MEMBER(port_a_r);
	DECLARE_READ8_MEMBER(port_b_r);
	UINT8 m_term_data;
	UINT8 m_keyb_press;
	UINT8 m_keyb_press_flag;
	UINT8 m_shift_press_flag;
	UINT8 m_ppi_portc;
	DECLARE_MACHINE_RESET(tk80);
	DECLARE_MACHINE_START(tk80bs);
	DECLARE_MACHINE_RESET(tk80bs);
	DECLARE_VIDEO_START(tk80bs);
	UINT32 screen_update_tk80bs(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	required_device<cpu_device> m_maincpu;
	optional_device<i8255_device> m_ppi8255_2;
	optional_shared_ptr<UINT8> m_p_videoram;
};

/************************************************* TK80 ******************************************/

READ8_MEMBER( tk80bs_state::display_r )
{
	return output_get_digit_value(offset);
}

WRITE8_MEMBER( tk80bs_state::display_w )
{
	output_set_digit_value(offset, data);
}

static ADDRESS_MAP_START(tk80_mem, AS_PROGRAM, 8, tk80bs_state)
	ADDRESS_MAP_UNMAP_HIGH
	ADDRESS_MAP_GLOBAL_MASK(0x83ff) // A10-14 not connected
	AM_RANGE(0x0000, 0x02ff) AM_ROM
	AM_RANGE(0x0300, 0x03ff) AM_RAM // EEPROM
	AM_RANGE(0x8000, 0x83f7) AM_RAM // RAM
	AM_RANGE(0x83f8, 0x83ff) AM_RAM AM_READWRITE(display_r,display_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START(tk85_mem, AS_PROGRAM, 8, tk80bs_state)
	ADDRESS_MAP_UNMAP_HIGH
	ADDRESS_MAP_GLOBAL_MASK(0x87ff) // A10-14 not connected
	AM_RANGE(0x0000, 0x07ff) AM_ROM
	AM_RANGE(0x8000, 0x83f7) AM_RAM
	AM_RANGE(0x83f8, 0x83ff) AM_RAM AM_READWRITE(display_r,display_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START(tk80_io, AS_IO, 8, tk80bs_state)
	ADDRESS_MAP_UNMAP_HIGH
	ADDRESS_MAP_GLOBAL_MASK(0xff) // possibly should be 3
	AM_RANGE(0xf8, 0xfb) AM_DEVREADWRITE("ppi8255_0", i8255_device, read, write)
ADDRESS_MAP_END

static ADDRESS_MAP_START(mikrolab_io, AS_IO, 8, tk80bs_state)
	ADDRESS_MAP_UNMAP_HIGH
	ADDRESS_MAP_GLOBAL_MASK(0xff) // possibly should be 3
	AM_RANGE(0xf8, 0xfb) AM_DEVREADWRITE("ppi8255_1", i8255_device, read, write)
ADDRESS_MAP_END

/* Input ports */
static INPUT_PORTS_START( tk80 )
	PORT_START("X0") /* KEY ROW 0 */
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("0") PORT_CODE(KEYCODE_0) PORT_CHAR('0')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("1") PORT_CODE(KEYCODE_1) PORT_CHAR('1')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("2") PORT_CODE(KEYCODE_2) PORT_CHAR('2')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("3") PORT_CODE(KEYCODE_3) PORT_CHAR('3')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("4") PORT_CODE(KEYCODE_4) PORT_CHAR('4')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("5") PORT_CODE(KEYCODE_5) PORT_CHAR('5')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("6") PORT_CODE(KEYCODE_6) PORT_CHAR('6')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("7") PORT_CODE(KEYCODE_7) PORT_CHAR('7')

	PORT_START("X1") /* KEY ROW 1 */
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("8") PORT_CODE(KEYCODE_8) PORT_CHAR('8')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("9") PORT_CODE(KEYCODE_9) PORT_CHAR('9')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("A") PORT_CODE(KEYCODE_A) PORT_CHAR('A')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("B") PORT_CODE(KEYCODE_B) PORT_CHAR('B')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("C") PORT_CODE(KEYCODE_C) PORT_CHAR('C')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("D") PORT_CODE(KEYCODE_D) PORT_CHAR('D')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("E") PORT_CODE(KEYCODE_E) PORT_CHAR('E')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F") PORT_CODE(KEYCODE_F) PORT_CHAR('F')

	PORT_START("X2") /* KEY ROW 2 */
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("RUN") PORT_CODE(KEYCODE_X) PORT_CHAR('X')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("RET") PORT_CODE(KEYCODE_W)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("ADRS SET") PORT_CODE(KEYCODE_MINUS) PORT_CHAR('-')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("READ DECR") PORT_CODE(KEYCODE_LEFT)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("READ INCR") PORT_CODE(KEYCODE_RIGHT)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("WRITE INCR") PORT_CODE(KEYCODE_UP) PORT_CHAR('^')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("STORE DATA") PORT_CODE(KEYCODE_I)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("LOAD DATA") PORT_CODE(KEYCODE_O)
INPUT_PORTS_END

INPUT_PORTS_START( mikrolab )
	PORT_INCLUDE( tk80 )
	PORT_MODIFY("X2")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("RUN") PORT_CODE(KEYCODE_X) PORT_CHAR('X')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("RESUME") PORT_CODE(KEYCODE_W)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("ADDRESS") PORT_CODE(KEYCODE_MINUS) PORT_CHAR('-')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("-") PORT_CODE(KEYCODE_LEFT)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("+") PORT_CODE(KEYCODE_RIGHT)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("SAVE") PORT_CODE(KEYCODE_UP) PORT_CHAR('^')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("END") PORT_CODE(KEYCODE_I)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("ENTER") PORT_CODE(KEYCODE_O)
INPUT_PORTS_END

MACHINE_RESET_MEMBER(tk80bs_state,tk80)
{
}

READ8_MEMBER( tk80bs_state::key_matrix_r )
{
// PA0-7 keyscan in

	UINT8 data = 0xff;
	if (BIT(m_ppi_portc, 4))
		data &= ioport("X0")->read();
	if (BIT(m_ppi_portc, 5))
		data &= ioport("X1")->read();
	if (BIT(m_ppi_portc, 6))
		data &= ioport("X2")->read();

	return data;
}

READ8_MEMBER( tk80bs_state::serial_r )
{
// PB0 - serial in
	//printf("B R\n");

	return 0;
}

WRITE8_MEMBER( tk80bs_state::serial_w )
{
// PC0 - serial out
// PC4-6 keyscan out
// PC7 - display on/off
	m_ppi_portc = data ^ 0x70;
}

WRITE8_MEMBER( tk80bs_state::mikrolab_serial_w )
{
// PC0 - serial out
// PC4-6 keyscan out
// PC7 - display on/off
	m_ppi_portc = data;
}

static I8255_INTERFACE( ppi8255_intf_0 )
{
	DEVCB_DRIVER_MEMBER(tk80bs_state, key_matrix_r),        /* Port A read */
	DEVCB_NULL,                         /* Port A write */
	DEVCB_DRIVER_MEMBER(tk80bs_state, serial_r),            /* Port B read */
	DEVCB_NULL,                         /* Port B write */
	DEVCB_NULL,                         /* Port C read */
	DEVCB_DRIVER_MEMBER(tk80bs_state, serial_w)         /* Port C write */
};

static I8255_INTERFACE( ppi8255_intf_1 )
{
	DEVCB_DRIVER_MEMBER(tk80bs_state, key_matrix_r),        /* Port A read */
	DEVCB_NULL,                         /* Port A write */
	DEVCB_DRIVER_MEMBER(tk80bs_state, serial_r),            /* Port B read */
	DEVCB_NULL,                         /* Port B write */
	DEVCB_NULL,                         /* Port C read */
	DEVCB_DRIVER_MEMBER(tk80bs_state, mikrolab_serial_w)        /* Port C write */
};


static MACHINE_CONFIG_START( tk80, tk80bs_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu",I8080, XTAL_1MHz) // 18.432 / 9
	MCFG_CPU_PROGRAM_MAP(tk80_mem)
	MCFG_CPU_IO_MAP(tk80_io)

	MCFG_MACHINE_RESET_OVERRIDE(tk80bs_state,tk80)

	/* video hardware */
	MCFG_DEFAULT_LAYOUT(layout_tk80)

	/* Devices */
	MCFG_I8255_ADD( "ppi8255_0", ppi8255_intf_0 )
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( mikrolab, tk80 )
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(tk85_mem)
	MCFG_CPU_IO_MAP(mikrolab_io)
	/* Devices */
	MCFG_DEVICE_REMOVE("ppi8255_0")
	MCFG_I8255_ADD( "ppi8255_1", ppi8255_intf_1 )
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( tk85, tk80 )
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(tk85_mem)
MACHINE_CONFIG_END

/************************************************* TK80BS ****************************************/

VIDEO_START_MEMBER(tk80bs_state,tk80bs)
{
}

UINT32 tk80bs_state::screen_update_tk80bs(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int x,y;
	int count;

	count = 0;

	for(y=0;y<16;y++)
	{
		for(x=0;x<32;x++)
		{
			int tile = m_p_videoram[count++];

			drawgfx_opaque(bitmap, cliprect, machine().gfx[0], tile, 0, 0, 0, x*8, y*8);
		}
	}

	return 0;
}

/* A0 and A1 are swapped at the 8255 chip */
READ8_MEMBER( tk80bs_state::ppi_custom_r )
{
	switch(offset)
	{
		case 1:
			return m_ppi8255_2->read(space, 2);
		case 2:
			return m_ppi8255_2->read(space, 1);
		default:
			return m_ppi8255_2->read(space, offset);
	}
}

WRITE8_MEMBER( tk80bs_state::ppi_custom_w )
{
	switch(offset)
	{
		case 1:
			m_ppi8255_2->write(space, 2, data);
			break;
		case 2:
			m_ppi8255_2->write(space, 1, data);
			break;
		default:
			m_ppi8255_2->write(space, offset, data);
	}
}

static ADDRESS_MAP_START(tk80bs_mem, AS_PROGRAM, 8, tk80bs_state)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0x07ff) AM_ROM
//  AM_RANGE(0x0c00, 0x7bff) AM_ROM // ext
	AM_RANGE(0x7df8, 0x7df9) AM_NOP // i8251 sio
	AM_RANGE(0x7dfc, 0x7dff) AM_READWRITE(ppi_custom_r,ppi_custom_w)
	AM_RANGE(0x7e00, 0x7fff) AM_RAM AM_SHARE("videoram") // video ram
	AM_RANGE(0x8000, 0xcfff) AM_RAM // RAM
	AM_RANGE(0xd000, 0xefff) AM_ROM // BASIC
	AM_RANGE(0xf000, 0xffff) AM_ROM // BSMON
ADDRESS_MAP_END


/* Input ports */
static INPUT_PORTS_START( tk80bs )
INPUT_PORTS_END

MACHINE_START_MEMBER(tk80bs_state,tk80bs)
{
}

MACHINE_RESET_MEMBER(tk80bs_state,tk80bs)
{
}

READ8_MEMBER( tk80bs_state::port_a_r )
{
	UINT8 ret = m_term_data;
	m_term_data = 0;
	return ret;
}


READ8_MEMBER( tk80bs_state::port_b_r )
{
	if (m_term_data)
	{
		m_ppi8255_2->pc4_w(0); // send a strobe pulse
		return 0x20;
	}
	else
		return 0;
}

static I8255_INTERFACE( ppi8255_intf_2 )
{
	DEVCB_DRIVER_MEMBER(tk80bs_state, port_a_r),            /* Port A read */
	DEVCB_NULL,                         /* Port A write */
	DEVCB_DRIVER_MEMBER(tk80bs_state, port_b_r),            /* Port B read */
	DEVCB_NULL,                         /* Port B write */
	DEVCB_NULL,                         /* Port C read */
	DEVCB_NULL                          /* Port C write */
};

WRITE8_MEMBER( tk80bs_state::kbd_put )
{
	data &= 0x7f;
	if (data > 0x5f) data-=0x20;
	m_term_data = data;
}

static ASCII_KEYBOARD_INTERFACE( keyboard_intf )
{
	DEVCB_DRIVER_MEMBER(tk80bs_state, kbd_put)
};


/* F4 Character Displayer */
static const gfx_layout tk80bs_charlayout =
{
	8, 8,
	512,
	1,
	{ 0 },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8
};

static GFXDECODE_START( tk80bs )
	GFXDECODE_ENTRY( "chargen", 0x0000, tk80bs_charlayout, 0, 1 )
GFXDECODE_END


static MACHINE_CONFIG_START( tk80bs, tk80bs_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu",I8080, XTAL_1MHz) //unknown clock
	MCFG_CPU_PROGRAM_MAP(tk80bs_mem)

	MCFG_MACHINE_START_OVERRIDE(tk80bs_state,tk80bs)
	MCFG_MACHINE_RESET_OVERRIDE(tk80bs_state,tk80bs)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(50)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500)) /* not accurate */
	MCFG_SCREEN_SIZE(256, 128)
	MCFG_SCREEN_VISIBLE_AREA(0, 256-1, 0, 128-1)
	MCFG_VIDEO_START_OVERRIDE(tk80bs_state,tk80bs)
	MCFG_SCREEN_UPDATE_DRIVER(tk80bs_state, screen_update_tk80bs)
	MCFG_PALETTE_LENGTH(2)
	MCFG_PALETTE_INIT(black_and_white)
	MCFG_GFXDECODE(tk80bs)

	/* Devices */
	MCFG_I8255_ADD( "ppi8255_2", ppi8255_intf_2 )
	MCFG_ASCII_KEYBOARD_ADD(KEYBOARD_TAG, keyboard_intf)
MACHINE_CONFIG_END

/* ROM definition */
ROM_START( tk80 )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "tk80-1.bin", 0x0000, 0x0100, CRC(897295e4) SHA1(50fb42b07252fc48044830e2f228e218fc59481c))
	ROM_LOAD( "tk80-2.bin", 0x0100, 0x0100, CRC(d54480c3) SHA1(354962aca1710ac75b40c8c23a6c303938f9d596))
	ROM_LOAD( "tk80-3.bin", 0x0200, 0x0100, CRC(8d4b02ef) SHA1(2b5a1ee8f97db23ffec48b96f12986461024c995))
ROM_END

ROM_START( mikrolab )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )
	/* these dumps are taken from PDF so need check with real device */
	ROM_LOAD( "rom-1.bin", 0x0000, 0x0200, BAD_DUMP CRC(eed5f23b) SHA1(c82f7a16ce44c4fcbcb333245555feae1fcdf058))
	ROM_LOAD( "rom-2.bin", 0x0200, 0x0200, BAD_DUMP CRC(726a224f) SHA1(7ed8d2c6dd4fb7836475e207e1972e33a6a91d2f))
ROM_END

ROM_START( nectk85 )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "tk85.bin",  0x0000, 0x0800, CRC(8a0b6d7e) SHA1(6acc8c04990692b08929043ccf638761b7301def))
ROM_END

ROM_START( tk80bs )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )
	/* all of these aren't taken from an original machine*/
	ROM_SYSTEM_BIOS(0, "psedo", "Pseudo LEVEL 1")
	ROMX_LOAD( "tk80.dummy", 0x0000, 0x0800, BAD_DUMP CRC(553b25ca) SHA1(939350d7fa56ce567ddf393c9f4b9db6ebc18a2c), ROM_BIOS(1))
	ROMX_LOAD( "ext.l1",     0x0c00, 0x6e46, BAD_DUMP CRC(d05ed3ff) SHA1(8544aa2cb58df9edf221f5be2cdafa248dd33828), ROM_BIOS(1))
	ROMX_LOAD( "lv1basic.l1",0xe000, 0x09a2, BAD_DUMP CRC(3ff67a71) SHA1(528c9331740637e853c099e1739ecdea6dd200bc), ROM_BIOS(1))
	ROMX_LOAD( "bsmon.l1",   0xf000, 0x0db0, BAD_DUMP CRC(5daa599b) SHA1(7e6ec5bfb3eea114f7ee9ef589a89246b8533b2f), ROM_BIOS(1))

	ROM_SYSTEM_BIOS(1, "psedo10", "Pseudo LEVEL 2 1.0")
	ROMX_LOAD( "tk80.dummy", 0x0000, 0x0800, BAD_DUMP CRC(553b25ca) SHA1(939350d7fa56ce567ddf393c9f4b9db6ebc18a2c), ROM_BIOS(2))
	ROMX_LOAD( "ext.10",     0x0c00, 0x3dc2, BAD_DUMP CRC(3c64d488) SHA1(919180d5b34b981ab3dd8b2885d3c0933203f355), ROM_BIOS(2))
	ROMX_LOAD( "lv2basic.10",0xd000, 0x2000, BAD_DUMP CRC(594fe70e) SHA1(5854c1be5fa78c1bfee365379495f14bc23e15e7), ROM_BIOS(2))
	ROMX_LOAD( "bsmon.10",   0xf000, 0x0daf, BAD_DUMP CRC(d0047983) SHA1(79e2b5dc47b574b55375cbafffff144744093ec1), ROM_BIOS(2))

	ROM_SYSTEM_BIOS(2, "psedo11", "Pseudo LEVEL 2 1.1")
	ROMX_LOAD( "tk80.dummy", 0x0000, 0x0800, BAD_DUMP CRC(553b25ca) SHA1(939350d7fa56ce567ddf393c9f4b9db6ebc18a2c), ROM_BIOS(3))
	ROMX_LOAD( "ext.11",     0x0c00, 0x3dd4, BAD_DUMP CRC(bd5c5169) SHA1(2ad70828348372328b76bac0fa93d3f6f17ade34), ROM_BIOS(3))
	ROMX_LOAD( "lv2basic.11",0xd000, 0x2000, BAD_DUMP CRC(3df9a3bd) SHA1(9539409c876bce27d630fe47d07a4316d2ce09cb), ROM_BIOS(3))
	ROMX_LOAD( "bsmon.11",   0xf000, 0x0ff6, BAD_DUMP CRC(fca7a609) SHA1(7c7eb5e5e4cf1e0021383bdfc192b88262aba6f5), ROM_BIOS(3))

	ROM_REGION( 0x1000, "chargen", ROMREGION_ERASEFF )
	ROM_LOAD( "font.rom",    0x0000, 0x1000, BAD_DUMP CRC(94d95199) SHA1(9fe741eab866b0c520d4108bccc6277172fa190c))
ROM_END

/* Driver */

/*    YEAR  NAME      PARENT  COMPAT   MACHINE    INPUT     INIT     COMPANY                        FULLNAME       FLAGS */
COMP( 1976, tk80,     0,      0,       tk80,      tk80,     driver_device, 0, "Nippon Electronic Company", "TK-80", GAME_NO_SOUND_HW)
COMP( 1980, tk80bs,   tk80,   0,       tk80bs,    tk80bs,   driver_device, 0, "Nippon Electronic Company", "TK-80BS", GAME_NOT_WORKING | GAME_NO_SOUND_HW)
COMP( 19??, nectk85,  tk80,   0,       tk85,      tk80,     driver_device, 0, "Nippon Electronic Company", "TK-85", GAME_NO_SOUND_HW)
COMP( 19??, mikrolab, tk80,   0,       mikrolab,  mikrolab, driver_device, 0, "<unknown>", "Mikrolab KR580IK80", GAME_NO_SOUND_HW)
