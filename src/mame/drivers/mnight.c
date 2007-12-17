/***************************************************************************

Mutant Night

driver by Leandro Dardini

TODO:
- must do palette marking, it is overflowing at the moment

***************************************************************************/
#include "driver.h"
#include "sound/2203intf.h"

WRITE8_HANDLER( mnight_bgvideoram_w );
WRITE8_HANDLER( mnight_fgvideoram_w );
WRITE8_HANDLER( mnight_sprite_overdraw_w );
WRITE8_HANDLER( mnight_background_enable_w );
VIDEO_START( mnight );
VIDEO_UPDATE( mnight );

extern UINT8    *mnight_scrolly_ram;
extern UINT8    *mnight_scrollx_ram;
extern UINT8    *mnight_bgenable_ram;
extern UINT8    *mnight_spoverdraw_ram;
extern UINT8    *mnight_background_videoram;
extern UINT8    *mnight_foreground_videoram;
extern size_t mnight_backgroundram_size;
extern size_t mnight_foregroundram_size;

static int mnight_bank_latch = 255, main_cpu_num;

static MACHINE_RESET( mnight )
{
	main_cpu_num = 0;
}

static INTERRUPT_GEN( mnight_interrupt )
{
	cpunum_set_input_line_and_vector(0,0,HOLD_LINE,0xd7);	/* RST 10h */
}

static READ8_HANDLER( mnight_bankselect_r )
{
	return mnight_bank_latch;
}

static WRITE8_HANDLER( mnight_bankselect_w )
{
	UINT8 *RAM = memory_region(REGION_CPU1+main_cpu_num);
	int bankaddress;

	if ( data != mnight_bank_latch )
	{
		mnight_bank_latch = data;

		bankaddress = 0x10000 + ((data & 0x7) * 0x4000);
		memory_set_bankptr(1,&RAM[bankaddress]);	 /* Select 8 banks of 16k */
	}
}


static ADDRESS_MAP_START( readmem, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x7fff) AM_READ(MRA8_ROM)
	AM_RANGE(0x8000, 0xbfff) AM_READ(MRA8_BANK1)
	AM_RANGE(0xc000, 0xf7ff) AM_READ(MRA8_RAM)
	AM_RANGE(0xf800, 0xf800) AM_READ(input_port_2_r)
	AM_RANGE(0xf801, 0xf801) AM_READ(input_port_0_r)
	AM_RANGE(0xf802, 0xf802) AM_READ(input_port_1_r)
	AM_RANGE(0xf803, 0xf803) AM_READ(input_port_3_r)
	AM_RANGE(0xf804, 0xf804) AM_READ(input_port_4_r)
	AM_RANGE(0xfa00, 0xfa00) AM_READ(MRA8_RAM)
	AM_RANGE(0xfa01, 0xfa01) AM_READ(MRA8_RAM)
	AM_RANGE(0xfa02, 0xfa02) AM_READ(mnight_bankselect_r)
	AM_RANGE(0xfa03, 0xfa03) AM_READ(MRA8_RAM)
	AM_RANGE(0xfa08, 0xfa09) AM_READ(MRA8_RAM)
	AM_RANGE(0xfa0a, 0xfa0b) AM_READ(MRA8_RAM)
	AM_RANGE(0xfa0c, 0xfa0c) AM_READ(MRA8_RAM)
ADDRESS_MAP_END


static ADDRESS_MAP_START( writemem, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0xbfff) AM_WRITE(MWA8_ROM)
	AM_RANGE(0xc000, 0xd9ff) AM_WRITE(MWA8_RAM)
	AM_RANGE(0xda00, 0xdfff) AM_WRITE(MWA8_RAM) AM_BASE(&spriteram) AM_SIZE(&spriteram_size)
	AM_RANGE(0xe000, 0xe7ff) AM_WRITE(mnight_bgvideoram_w) AM_BASE(&mnight_background_videoram) AM_SIZE(&mnight_backgroundram_size) // VFY
	AM_RANGE(0xe800, 0xefff) AM_WRITE(mnight_fgvideoram_w) AM_BASE(&mnight_foreground_videoram) AM_SIZE(&mnight_foregroundram_size) //VFY
	AM_RANGE(0xf000, 0xf5ff) AM_WRITE(paletteram_RRRRGGGGBBBBxxxx_be_w) AM_BASE(&paletteram)
	AM_RANGE(0xf600, 0xf7ff) AM_WRITE(MWA8_RAM)
	AM_RANGE(0xfa00, 0xfa00) AM_WRITE(soundlatch_w)
	AM_RANGE(0xfa01, 0xfa01) AM_WRITE(MWA8_RAM)		   // unknown but used
	AM_RANGE(0xfa02, 0xfa02) AM_WRITE(mnight_bankselect_w)
	AM_RANGE(0xfa03, 0xfa03) AM_WRITE(mnight_sprite_overdraw_w) AM_BASE(&mnight_spoverdraw_ram)
	AM_RANGE(0xfa08, 0xfa09) AM_WRITE(MWA8_RAM) AM_BASE(&mnight_scrollx_ram)
	AM_RANGE(0xfa0a, 0xfa0b) AM_WRITE(MWA8_RAM) AM_BASE(&mnight_scrolly_ram)
	AM_RANGE(0xfa0c, 0xfa0c) AM_WRITE(mnight_background_enable_w) AM_BASE(&mnight_bgenable_ram)
ADDRESS_MAP_END


static ADDRESS_MAP_START( snd_readmem, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0xbfff) AM_READ(MRA8_ROM)
	AM_RANGE(0xc000, 0xc7ff) AM_READ(MRA8_RAM)
	AM_RANGE(0xe000, 0xe000) AM_READ(soundlatch_r)
	AM_RANGE(0xefee, 0xefee) AM_READ(MRA8_NOP)
ADDRESS_MAP_END


static ADDRESS_MAP_START( snd_writemem, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0xbfff) AM_WRITE(MWA8_ROM)
	AM_RANGE(0xc000, 0xc7ff) AM_WRITE(MWA8_RAM)
	AM_RANGE(0xeff5, 0xeff6) AM_WRITE(MWA8_NOP)			   /* SAMPLE FREQUENCY ??? */
	AM_RANGE(0xefee, 0xefee) AM_WRITE(MWA8_NOP)			   /* CHIP COMMAND ?? */
ADDRESS_MAP_END

static ADDRESS_MAP_START( snd_writeport, ADDRESS_SPACE_IO, 8 )
	ADDRESS_MAP_FLAGS( AMEF_ABITS(8) )
	AM_RANGE(0x00, 0x00) AM_WRITE(YM2203_control_port_0_w)
	AM_RANGE(0x01, 0x01) AM_WRITE(YM2203_write_port_0_w)
	AM_RANGE(0x80, 0x80) AM_WRITE(YM2203_control_port_1_w)
	AM_RANGE(0x81, 0x81) AM_WRITE(YM2203_write_port_1_w)
ADDRESS_MAP_END


static INPUT_PORTS_START( mnight )
	PORT_START /* Player 1 controls */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START /* Player 2 controls */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SERVICE )	/* keep pressed during boot to enter service mode */
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN2 )

	PORT_START /* DSW0 */
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x02, "30k and every 50k" )
	PORT_DIPSETTING(    0x00, "50k and every 80k" )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x00, "Difficult" )
	PORT_DIPNAME( 0x08, 0x08, "Free Game" )
	PORT_DIPSETTING(    0x08, DEF_STR( Off )  )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Cabinet) )
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0xc0, 0xc0, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x80, "2" )
	PORT_DIPSETTING(    0xc0, "3" )
	PORT_DIPSETTING(    0x40, "4" )
	PORT_DIPSETTING(    0x00, "5" )

	PORT_START /* DSW1 */
	PORT_SERVICE( 0x01, IP_ACTIVE_LOW )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0xe0, 0xe0, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x60, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0xe0, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0xa0, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 1C_4C ) )
INPUT_PORTS_END

static INPUT_PORTS_START( arkarea )
	PORT_START /* Player 1 controls */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START /* Player 2 controls */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SERVICE )	/* keep pressed during boot to enter service mode */
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN2 )

	PORT_START /* DSW0 */
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x40, "50000 and every 50000" )
	PORT_DIPSETTING(    0x00, "100000 and every 100000" )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x80, "3" )
	PORT_DIPSETTING(    0x00, "4" )

	PORT_START /* DSW1 */
	PORT_SERVICE( 0x01, IP_ACTIVE_LOW )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END



static const gfx_layout charlayout =
{
	8,8,	 /* 8*8 characters */
	1024,	 /* 1024 characters */
	4,		 /* 4 bits per pixel */
	{0,1,2,3}, /* the bitplanes are packed in one nibble */
	{0, 4, 16384*8+0, 16384*8+4, 8, 12, 16384*8+8, 16384*8+12},
	{16*0, 16*1, 16*2, 16*3, 16*4, 16*5, 16*6, 16*7},
	8*16
};

static const gfx_layout spritelayout =
{
	16,16,	 /* 16*16 characters */
	1536,	 /* 1536 sprites */
	4,		 /* 4 bits per pixel */
	{0,1,2,3}, /* the bitplanes are packed in one nibble */
	{0,  4,  0x18000*8+0,  0x18000*8+4,  8, 12,  0x18000*8+8, 0x18000*8+12,
		16*8+0, 16*8+4, 16*8+0x18000*8+0, 16*8+0x18000*8+4, 16*8+8, 16*8+12, 16*8+0x18000*8+8, 16*8+0x18000*8+12},
	{16*0, 16*1, 16*2, 16*3, 16*4, 16*5, 16*6, 16*7,
		32*8+16*0, 32*8+16*1, 32*8+16*2, 32*8+16*3, 32*8+16*4, 32*8+16*5, 32*8+16*6, 32*8+16*7},
	8*64
};

static const gfx_layout bigspritelayout =
{
	32,32,	 /* 32*32 characters */
	384,	 /* 384 sprites */
	4,		 /* 4 bits per pixel */
	{0,1,2,3}, /* the bitplanes are packed in one nibble */
	{0,  4,  0x18000*8+0,  0x18000*8+4,  8, 12,  0x18000*8+8, 0x18000*8+12,
		16*8+0, 16*8+4, 16*8+0x18000*8+0, 16*8+0x18000*8+4, 16*8+8, 16*8+12, 16*8+0x18000*8+8, 16*8+0x18000*8+12,
	 	64*8+0, 64*8+4, 64*8+0x18000*8+0, 64*8+0x18000*8+4, 64*8+8, 64*8+12, 64*8+0x18000*8+8, 64*8+0x18000*8+12,
		64*8+16*8+0, 64*8+16*8+4, 64*8+16*8+0x18000*8+0, 64*8+16*8+0x18000*8+4,
		64*8+16*8+8, 64*8+16*8+12, 64*8+16*8+0x18000*8+8, 64*8+16*8+0x18000*8+12 },
	{16*0, 16*1, 16*2, 16*3, 16*4, 16*5, 16*6, 16*7,
		32*8+16*0, 32*8+16*1, 32*8+16*2, 32*8+16*3, 32*8+16*4, 32*8+16*5, 32*8+16*6, 32*8+16*7,
		128*8+16*0, 128*8+16*1, 128*8+16*2, 128*8+16*3,
		128*8+16*4, 128*8+16*5, 128*8+16*6, 128*8+16*7,
		128*8+32*8+16*0, 128*8+32*8+16*1, 128*8+32*8+16*2, 128*8+32*8+16*3,
		128*8+32*8+16*4, 128*8+32*8+16*5, 128*8+32*8+16*6, 128*8+32*8+16*7 },
	8*64*4
};


static GFXDECODE_START( mnight )
	GFXDECODE_ENTRY( REGION_GFX1, 0, spritelayout,     0*16, 16)
	GFXDECODE_ENTRY( REGION_GFX2, 0, spritelayout,    16*16, 16)
	GFXDECODE_ENTRY( REGION_GFX2, 0, bigspritelayout, 16*16, 16)
	GFXDECODE_ENTRY( REGION_GFX3, 0, charlayout,      32*16, 16)
GFXDECODE_END


static MACHINE_DRIVER_START( mnight )

	/* basic machine hardware */
	MDRV_CPU_ADD(Z80, 12000000/2)		/* 12000000/2 ??? */
	MDRV_CPU_PROGRAM_MAP(readmem,writemem)	/* very sensitive to these settings */
	MDRV_CPU_VBLANK_INT(mnight_interrupt,1)

	MDRV_CPU_ADD(Z80, 5000000)
	/* audio CPU */		/* 5mhz crystal ??? */
	MDRV_CPU_PROGRAM_MAP(snd_readmem,snd_writemem)
	MDRV_CPU_IO_MAP(0,snd_writeport)
	MDRV_CPU_VBLANK_INT(irq0_line_hold,2)

	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(10000))
	MDRV_INTERLEAVE(10)

	MDRV_MACHINE_RESET(mnight)

	/* video hardware */
	MDRV_VIDEO_ATTRIBUTES(VIDEO_TYPE_RASTER)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(32*8, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 4*8, 28*8-1)
	MDRV_GFXDECODE(mnight)
	MDRV_PALETTE_LENGTH(768)

	MDRV_VIDEO_START(mnight)
	MDRV_VIDEO_UPDATE(mnight)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_SOUND_ADD(YM2203, 12000000/8)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)

	MDRV_SOUND_ADD(YM2203, 12000000/8)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
MACHINE_DRIVER_END


ROM_START( mnight )
	ROM_REGION( 0x30000, REGION_CPU1, 0 )
	ROM_LOAD( "mn6-j19.bin",  0x00000, 0x8000, CRC(56678d14) SHA1(acf3a97ca29db8ab9cad69599c5567464af3ae44) )
	ROM_LOAD( "mn5-j17.bin",  0x10000, 0x8000, CRC(2a73f88e) SHA1(0a7b769174f2b976650453d808cb23668dff0260) )
	ROM_LOAD( "mn4-j16.bin",  0x18000, 0x8000, CRC(c5e42bb4) SHA1(1956e737ae393e987cb7e8eae520518f1e0f597f) )
	ROM_LOAD( "mn3-j14.bin",  0x20000, 0x8000, CRC(df6a4f7a) SHA1(ce3cb84b514220d686b12c03567289fd23da0fe1) )
	ROM_LOAD( "mn2-j12.bin",  0x28000, 0x8000, CRC(9c391d1b) SHA1(06e8c202337e3eba38c479e8b7e29a3f8ffc4ed1) )

	ROM_REGION( 0x10000, REGION_CPU2, 0 )
	ROM_LOAD( "mn1-j7.bin",   0x00000, 0x10000, CRC(a0782a31) SHA1(8abd2f0b0c2c2eb876f324f7a095a5cdc773c187) )

	ROM_REGION( 0x30000, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD( "mn11-b20.bin", 0x00000, 0x4000, CRC(4d37e0f4) SHA1(a6d9aaccd97769197622cda45474e223c2ee1d98) )   // background tiles
	ROM_CONTINUE(             0x18000, 0x4000 )
	ROM_CONTINUE(             0x04000, 0x4000 )
	ROM_CONTINUE(             0x1c000, 0x4000 )
	ROM_LOAD( "mn12-b22.bin", 0x08000, 0x4000, CRC(b22cbbd3) SHA1(70984f1051fd236730d97011bc87dacb3ca38594) )
	ROM_CONTINUE(             0x20000, 0x4000 )
	ROM_CONTINUE(             0x0c000, 0x4000 )
	ROM_CONTINUE(             0x24000, 0x4000 )
	ROM_LOAD( "mn13-b23.bin", 0x10000, 0x4000, CRC(65714070) SHA1(48f3c130c97d00e8f0535904dc2237277067c475) )
	ROM_CONTINUE(             0x28000, 0x4000 )
	ROM_CONTINUE(             0x14000, 0x4000 )
	ROM_CONTINUE(             0x2c000, 0x4000 )

	ROM_REGION( 0x30000, REGION_GFX2, ROMREGION_DISPOSE )
	ROM_LOAD( "mn7-e11.bin",  0x00000, 0x4000, CRC(4883059c) SHA1(53d4b9b0f0725c25e302ee1549a306778ec74d85) )	  // sprites tiles
	ROM_CONTINUE(             0x18000, 0x4000 )
	ROM_CONTINUE(             0x04000, 0x4000 )
	ROM_CONTINUE(             0x1c000, 0x4000 )
	ROM_LOAD( "mn8-e12.bin",  0x08000, 0x4000, CRC(02b91445) SHA1(f0cf85f9e17c40248de16bca8df6d745e359b92d) )
	ROM_CONTINUE(             0x20000, 0x4000 )
	ROM_CONTINUE(             0x0c000, 0x4000 )
	ROM_CONTINUE(             0x24000, 0x4000 )
	ROM_LOAD( "mn9-e14.bin",  0x10000, 0x4000, CRC(9f08d160) SHA1(1a0041ad138e7e6598d4d03d7cbd52a7244557ac) )
	ROM_CONTINUE(             0x28000, 0x4000 )
	ROM_CONTINUE(             0x14000, 0x4000 )
	ROM_CONTINUE(             0x2c000, 0x4000 )

	ROM_REGION( 0x08000, REGION_GFX3, ROMREGION_DISPOSE )
	ROM_LOAD( "mn10-b10.bin", 0x00000, 0x2000, CRC(37b8221f) SHA1(ac86e0ae8039fd30a028a893d08ce099f7765615) )	// foreground tiles OK
	ROM_CONTINUE(             0x04000, 0x2000 )
	ROM_CONTINUE(             0x02000, 0x2000 )
	ROM_CONTINUE(             0x06000, 0x2000 )
ROM_END

ROM_START( arkarea )
	ROM_REGION( 0x30000, REGION_CPU1, 0 )
	ROM_LOAD( "arkarea.008",  0x00000, 0x8000, CRC(1ce1b5b9) SHA1(ab7755523d58736b124deb59779dedee870fd7d2) )
	ROM_LOAD( "arkarea.009",  0x10000, 0x8000, CRC(db1c81d1) SHA1(64a2f51c218d84c4eaeb8c5a5a3f0f4cdf5d174c) )
	ROM_LOAD( "arkarea.010",  0x18000, 0x8000, CRC(5a460dae) SHA1(e64d3662bb074a528cd71061621c0dd3765928b6) )
	ROM_LOAD( "arkarea.011",  0x20000, 0x8000, CRC(63f022c9) SHA1(414f52d2b9584f08285b261d1dafcc9e6e5e0c74) )
	ROM_LOAD( "arkarea.012",  0x28000, 0x8000, CRC(3c4c65d5) SHA1(e11f840f8b1da0933a0a4342f5fa1a17f0d6d3e2) )

	ROM_REGION( 0x10000, REGION_CPU2, 0 )
	ROM_LOAD( "arkarea.013",  0x00000, 0x8000, CRC(2d409d58) SHA1(6344b43db5459691728c3f843b643c84ea71dd8e) )

	ROM_REGION( 0x30000, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD( "arkarea.003",  0x00000, 0x4000, CRC(6f45a308) SHA1(b6994fe1f50d5e9cf38d3efbd69a2c5f76f33c56) )   // background tiles
	ROM_CONTINUE(             0x18000, 0x4000 )
	ROM_CONTINUE(             0x04000, 0x4000 )
	ROM_CONTINUE(             0x1c000, 0x4000 )
	ROM_LOAD( "arkarea.002",  0x08000, 0x4000, CRC(051d3482) SHA1(3ebef1a7280f52df6d5ee34e3d4e7567aac0c165) )
	ROM_CONTINUE(             0x20000, 0x4000 )
	ROM_CONTINUE(             0x0c000, 0x4000 )
	ROM_CONTINUE(             0x24000, 0x4000 )
	ROM_LOAD( "arkarea.001",  0x10000, 0x4000, CRC(09d11ab7) SHA1(14f68e93e7173069f790493eafe9e1adc1a074cc) )
	ROM_CONTINUE(             0x28000, 0x4000 )
	ROM_CONTINUE(             0x14000, 0x4000 )
	ROM_CONTINUE(             0x2c000, 0x4000 )

	ROM_REGION( 0x30000, REGION_GFX2, ROMREGION_DISPOSE )
	ROM_LOAD( "arkarea.007",  0x00000, 0x4000, CRC(d5684a27) SHA1(4961e8a5df2510afb1ef3e937d0a5d52e91893a3) )   // sprites tiles
	ROM_CONTINUE(             0x18000, 0x4000 )
	ROM_CONTINUE(             0x04000, 0x4000 )
	ROM_CONTINUE(             0x1c000, 0x4000 )
	ROM_LOAD( "arkarea.006",  0x08000, 0x4000, CRC(2c0567d6) SHA1(f36a2a3ff487660f89470516617482331f008da0) )
	ROM_CONTINUE(             0x20000, 0x4000 )
	ROM_CONTINUE(             0x0c000, 0x4000 )
	ROM_CONTINUE(             0x24000, 0x4000 )
	ROM_LOAD( "arkarea.005",  0x10000, 0x4000, CRC(9886004d) SHA1(4050756af5c00ab1a368780fe091460fd9e2cb05) )
	ROM_CONTINUE(             0x28000, 0x4000 )
	ROM_CONTINUE(             0x14000, 0x4000 )
	ROM_CONTINUE(             0x2c000, 0x4000 )

	ROM_REGION( 0x08000, REGION_GFX3, ROMREGION_DISPOSE )
	ROM_LOAD( "arkarea.004",  0x00000, 0x2000, CRC(69e36af2) SHA1(2bccef8f396dcb5261af0140af04c95ee8ecae11) ) // foreground tiles OK
	ROM_CONTINUE(             0x04000, 0x2000 )
	ROM_CONTINUE(             0x02000, 0x2000 )
	ROM_CONTINUE(             0x06000, 0x2000 )
ROM_END



GAME( 1987, mnight,  0, mnight, mnight,  0, ROT0, "UPL (Kawakus license)", "Mutant Night", 0 )
GAME( 1988?,arkarea, 0, mnight, arkarea, 0, ROT0, "UPL", "Ark Area", 0 )
