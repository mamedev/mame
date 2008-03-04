/*******************************************************************************************

Yumefuda (c) 198? Alba

driver by Angelo Salese

Notes:
-I think the name of this hardware is "Alba ZG board",an older(?) revision of the
 "Alba ZC board" used by Hanaroku (hanaroku.c driver).
 My guess is that this game is dated 1986/1987.

TODO:
-Correct Bankswitch emulation.Obviously I'm not happy with the current implementation...

-Finish controls.

-Custom RAM emulation.

-Fix the colors.

============================================================================================
Code disassembling
(anything that I don't know the meaning is in brackets):
[0458]-> (Writes to ports 00 & 01)
[04bd]-> Enables prot lock
[04d9]-> Palette RAM init
[0c55]-> Video Ram Init
[04c3]-> Disables prot lock
[2603]-> Custom Ram Math
[008b]-> Custom Ram Check 1 [without prot lock]
[009A]-> Custom Ram Check 2 [must be NZ]
[010e]-> Video Ram Math 1 [$c000]
[00A0]-> Video Ram Check 1 [must be Z]
[013b]-> Video Ram Math 2 [$d000]
[00A6]-> Video Ram Check 2 [must be Z]
[0197]-> Eeprom Check 1
    [2344] -> Eeprom sub check 1
    [2318] -> Eeprom sub check 2
    ...
[0222]-> Back Up Check
[047c]-> (Writes to ports 83 & 80)
[0488]-> Multiple writes to sound ports 40 & 41
[04b4]-> Disables prot lock
[0810]->[08b5]->write & read to sound port 40
[08dd]->
    [079d]->(write to port c0)
    [0985]->Nvram lock enable/disable
    ...
[0b44]-> Nvram Check
[0b1a]-> Nvram Check
[090a]-> (?)
[0312]-> Eeprom init msg
*******************************************************************************************/

#include "driver.h"
#include "machine/eeprom.h"
#include "sound/ay8910.h"

static tilemap *bg_tilemap;

static TILE_GET_INFO( y_get_bg_tile_info )
{
	int code = videoram[tile_index];
	int color = colorram[tile_index];

	SET_TILE_INFO(
			0,
			code + ((color & 0xf8) << 3),
			color & 0x7,
			0);
}


static VIDEO_START( yumefuda )
{
	bg_tilemap = tilemap_create(y_get_bg_tile_info,tilemap_scan_rows,8,8,32,32);
}

static VIDEO_UPDATE( yumefuda )
{
	tilemap_draw(bitmap,cliprect,bg_tilemap,0,0);
	return 0;
}

/***************************************************************************************/



static const gfx_layout charlayout =
{
	8,8,
	RGN_FRAC(1,4),
	4,
	{ RGN_FRAC(0,4), RGN_FRAC(1,4), RGN_FRAC(2,4), RGN_FRAC(3,4) },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8
};

static GFXDECODE_START( yumefuda )
	GFXDECODE_ENTRY( REGION_GFX1, 0x0000, charlayout,   0, 0x10 )
GFXDECODE_END


static WRITE8_HANDLER( yumefuda_vram_w )
{
	videoram[offset] = data;
	tilemap_mark_tile_dirty(bg_tilemap,offset);
}

static WRITE8_HANDLER( yumefuda_cram_w )
{
	colorram[offset] = data;
	tilemap_mark_tile_dirty(bg_tilemap,offset);
}

static UINT8 *cus_ram;
static UINT8 prot_lock;

/*Custom RAM (Protection)*/
static READ8_HANDLER( custom_ram_r )
{
//  logerror("Custom RAM read at %02x PC = %x\n",offset+0xaf80,activecpu_get_pc());
	return cus_ram[offset];// ^ 0x55;
}

static WRITE8_HANDLER( custom_ram_w )
{
//  logerror("Custom RAM write at %02x : %02x PC = %x\n",offset+0xaf80,data,activecpu_get_pc());
	if(prot_lock)	{ cus_ram[offset] = data; }
}

/*this might be used as NVRAM commands btw*/
static WRITE8_HANDLER( prot_lock_w )
{
//  logerror("PC %04x Prot lock value written %02x\n",activecpu_get_pc(),data);
	prot_lock = data;
}

static WRITE8_HANDLER( eeprom_w )
{
	EEPROM_write_bit(data & 0x04);
	EEPROM_set_cs_line((data & 0x02) ? CLEAR_LINE : ASSERT_LINE);
	EEPROM_set_clock_line((data & 0x08) ? ASSERT_LINE : CLEAR_LINE);
}

static WRITE8_HANDLER( port_c0_w )
{
//  logerror("PC %04x (Port $c0) value written %02x\n",activecpu_get_pc(),data);
}


static READ8_HANDLER( eeprom_r )
{
	return ((~EEPROM_read_bit()&0x01)<<6) | (0xff&~0x40);
}

static UINT8 mux_data;

static READ8_HANDLER( mux_r )
{
	switch(mux_data)
	{
		case 0x00: return readinputport(0);
		case 0x01: return readinputport(1);
		case 0x02: return readinputport(2);
		case 0x04: return readinputport(3);
		case 0x08: return readinputport(4);
		/* FIXME: Was this a quick hack? */
		case 0x10: return 0xff; //return readinputport(5);
		case 0x20: return 0xff; //return readinputport(6);
	}
	return 0xff;
}

static WRITE8_HANDLER( mux_w )
{
	static int bank=-1;

	int new_bank = (data&0xc0)>>6;

	//0x10000 service mode
	//0x12000 gameplay
	//0x14000 bonus game
	//0x16000 ?
	if(bank!=new_bank) {
		UINT8 *ROM = memory_region(REGION_CPU1);
		UINT32 bankaddress;

		bank = new_bank;
		bankaddress = 0x10000 + 0x2000 * bank;
		memory_set_bankptr(1, &ROM[bankaddress]);
	}

	mux_data = data & ~0xc0;
}


/***************************************************************************************/

static ADDRESS_MAP_START( main_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0x8000, 0x9fff) AM_READWRITE(MRA8_BANK1, MWA8_ROM)
	AM_RANGE(0xa7fc, 0xa7fc) AM_WRITE(prot_lock_w)
	AM_RANGE(0xa7ff, 0xa7ff) AM_WRITE(eeprom_w)
	AM_RANGE(0xaf80, 0xafff) AM_READWRITE(custom_ram_r, custom_ram_w) AM_BASE(&cus_ram) /*260d - 2626*/
	AM_RANGE(0xb000, 0xb0ff) AM_READWRITE(MRA8_RAM, paletteram_RRRGGGBB_w) AM_BASE(&paletteram) /*Wrong format*/
	AM_RANGE(0xc000, 0xc3ff) AM_READWRITE(MRA8_RAM, yumefuda_vram_w) AM_BASE(&videoram)
	AM_RANGE(0xd000, 0xd3ff) AM_READWRITE(MRA8_RAM, yumefuda_cram_w) AM_BASE(&colorram)
	AM_RANGE(0xe000, 0xffff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( port_map, ADDRESS_SPACE_IO, 8 )
	ADDRESS_MAP_FLAGS( AMEF_ABITS(8) )
	AM_RANGE(0x40, 0x40) AM_READWRITE(AY8910_read_port_0_r, AY8910_control_port_0_w)
	AM_RANGE(0x41, 0x41) AM_WRITE(AY8910_write_port_0_w)
	AM_RANGE(0x80, 0x80) AM_WRITE(mux_w)
	AM_RANGE(0x81, 0x81) AM_READ(eeprom_r)
	AM_RANGE(0x82, 0x82) AM_READ(mux_r)
	AM_RANGE(0xc0, 0xc0) AM_WRITE(port_c0_w) /*watchdog write?*/
ADDRESS_MAP_END

static MACHINE_DRIVER_START( yumefuda )

	/* basic machine hardware */
	MDRV_CPU_ADD(Z80 , 6000000) /*???*/
	MDRV_CPU_PROGRAM_MAP(main_map,0)
	MDRV_CPU_IO_MAP(port_map,0)
	MDRV_CPU_VBLANK_INT("main", irq0_line_hold)

	MDRV_NVRAM_HANDLER(93C46)

	/* video hardware */
	MDRV_SCREEN_ADD("main", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(32*8, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(0, 32*8-1, 0, 32*8-1)

	MDRV_GFXDECODE( yumefuda )
	MDRV_PALETTE_LENGTH(0x100)

	MDRV_VIDEO_START( yumefuda )
	MDRV_VIDEO_UPDATE( yumefuda )

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_SOUND_ADD(AY8910, 1500000)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)
MACHINE_DRIVER_END

/***************************************************************************************/

static INPUT_PORTS_START( yumefuda )
	PORT_START
	PORT_DIPNAME( 0x01, 0x01, "Port 0" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_IMPULSE(2)
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unused ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(2)
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unused ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unused ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unused ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unused ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START
	PORT_DIPNAME( 0x01, 0x01, "Port 1" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unused ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unused ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unused ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unused ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P1 BET Button") PORT_CODE(KEYCODE_RALT)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START1 )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unused ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START
	PORT_DIPNAME( 0x01, 0x01, "Port 2" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unused ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unused ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unused ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON3 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON4 )

	PORT_START
	PORT_DIPNAME( 0x01, 0x01, "Port 3" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unused ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unused ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unused ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unused ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_NAME("P1 Button 6 / Yes") PORT_CODE(KEYCODE_RCONTROL)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON5 )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unused ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START
	PORT_DIPNAME( 0x01, 0x01, "Port 4" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unused ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unused ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unused ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unused ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unused ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unused ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unused ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
/*
    PORT_START
    PORT_DIPNAME( 0x01, 0x01, "Port 5" )
    PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
    PORT_DIPSETTING(    0x00, DEF_STR( On ) )
    PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unused ) )
    PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
    PORT_DIPSETTING(    0x00, DEF_STR( On ) )
    PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unused ) )
    PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
    PORT_DIPSETTING(    0x00, DEF_STR( On ) )
    PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unused ) )
    PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
    PORT_DIPSETTING(    0x00, DEF_STR( On ) )
    PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unused ) )
    PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
    PORT_DIPSETTING(    0x00, DEF_STR( On ) )
    PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unused ) )
    PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
    PORT_DIPSETTING(    0x00, DEF_STR( On ) )
    PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unused ) )
    PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
    PORT_DIPSETTING(    0x00, DEF_STR( On ) )
    PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unused ) )
    PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
    PORT_DIPSETTING(    0x00, DEF_STR( On ) )

    PORT_START
    PORT_DIPNAME( 0x01, 0x01, "Port 6" )
    PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
    PORT_DIPSETTING(    0x00, DEF_STR( On ) )
    PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unused ) )
    PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
    PORT_DIPSETTING(    0x00, DEF_STR( On ) )
    PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unused ) )
    PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
    PORT_DIPSETTING(    0x00, DEF_STR( On ) )
    PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unused ) )
    PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
    PORT_DIPSETTING(    0x00, DEF_STR( On ) )
    PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unused ) )
    PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
    PORT_DIPSETTING(    0x00, DEF_STR( On ) )
    PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unused ) )
    PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
    PORT_DIPSETTING(    0x00, DEF_STR( On ) )
    PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unused ) )
    PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
    PORT_DIPSETTING(    0x00, DEF_STR( On ) )
    PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unused ) )
    PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
    PORT_DIPSETTING(    0x00, DEF_STR( On ) )*/
INPUT_PORTS_END

/***************************************************************************************/


ROM_START( yumefuda )
	ROM_REGION( 0x18000, REGION_CPU1, 0 ) /* code */
	ROM_LOAD("zg004y02.u43", 0x00000, 0x8000, CRC(974c543c) SHA1(56aeb318cb00445f133246dfddc8c24bb0c23f2d))
	ROM_LOAD("zg004y01.u42", 0x10000, 0x8000, CRC(ae99126b) SHA1(4ae2c1c804bbc505a013f5e3d98c0bfbb51b747a))

	ROM_REGION( 0x10000, REGION_GFX1, 0 )
	ROM_LOAD("zg001006.u6", 0x0000, 0x4000, CRC(a5df443c) SHA1(a6c088a463c05e43a7b559c5d0afceddc88ef476))
	ROM_LOAD("zg001005.u5", 0x4000, 0x4000, CRC(158b6cde) SHA1(3e335b7dc1bbae2edb02722025180f32ab91f69f))
	ROM_LOAD("zg001004.u4", 0x8000, 0x4000, CRC(d8676435) SHA1(9b6df5378948f492717e1a4d9c833ddc5a9e8225))
	ROM_LOAD("zg001003.u3", 0xc000, 0x4000, CRC(5822ff27) SHA1(d40fa0790de3c912f770ef8f610bd8c42bc3500f))
ROM_END

GAME( 198?, yumefuda, 0, yumefuda, yumefuda, 0, ROT0, "Alba", "(Medal) Yumefuda [BET]", GAME_NOT_WORKING | GAME_WRONG_COLORS )
