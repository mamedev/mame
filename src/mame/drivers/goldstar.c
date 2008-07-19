/***************************************************************************

Golden Star

driver by Mirko Buffoni

Is this a Konami board?
***************************************************************************/
#include "driver.h"
#include "sound/ay8910.h"
#include "sound/okim6295.h"

static int dataoffset=0;

extern UINT8 *goldstar_video1, *goldstar_video2, *goldstar_video3;
extern size_t goldstar_video_size;
extern UINT8 *goldstar_scroll1, *goldstar_scroll2, *goldstar_scroll3;

WRITE8_HANDLER( goldstar_fa00_w );
VIDEO_START( goldstar );
VIDEO_UPDATE( goldstar );


static UINT8 *nvram;
static size_t nvram_size;

static NVRAM_HANDLER( goldstar )
{
	if (read_or_write)
                mame_fwrite(file,nvram,nvram_size);
	else
	{
		if (file)
                        mame_fread(file,nvram,nvram_size);
		else
			memset(nvram,0xff,nvram_size);
	}
}



static WRITE8_HANDLER( protection_w )
{
	if (data == 0x2a)
		dataoffset = 0;
}

static READ8_HANDLER( protection_r )
{
	static const int data[4] = { 0x47, 0x4f, 0x4c, 0x44 };

	dataoffset %= 4;
	return data[dataoffset++];
}

static ADDRESS_MAP_START( map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0xb7ff) AM_ROM
	AM_RANGE(0xb800, 0xbfff) AM_RAM AM_BASE(&nvram) AM_SIZE(&nvram_size)
	AM_RANGE(0xc000, 0xc7ff) AM_ROM
	AM_RANGE(0xc800, 0xcfff) AM_RAM AM_BASE(&videoram) AM_SIZE(&videoram_size)
	AM_RANGE(0xd000, 0xd7ff) AM_RAM AM_BASE(&colorram)
	AM_RANGE(0xd800, 0xd9ff) AM_RAM AM_BASE(&goldstar_video1) AM_SIZE(&goldstar_video_size)
	AM_RANGE(0xe000, 0xe1ff) AM_RAM AM_BASE(&goldstar_video2)
	AM_RANGE(0xe800, 0xe9ff) AM_WRITE(SMH_RAM) AM_BASE(&goldstar_video3)
	AM_RANGE(0xf040, 0xf07f) AM_WRITE(SMH_RAM) AM_BASE(&goldstar_scroll1)
	AM_RANGE(0xf800, 0xf800) AM_READ(input_port_0_r)
	AM_RANGE(0xf801, 0xf801) AM_READ(input_port_1_r)	/* Test Mode */
	AM_RANGE(0xf802, 0xf802) AM_READ(input_port_2_r)	/* DSW 1 */
//  AM_RANGE(0xf803, 0xf803)
//  AM_RANGE(0xf804, 0xf804)
	AM_RANGE(0xf805, 0xf805) AM_READ(input_port_7_r)	/* DSW 4 (also appears in 8910 port) */
	AM_RANGE(0xf806, 0xf806) AM_READ(input_port_9_r)	/* (don't know to which one of the */
										/* service mode dip switches it should map) */
	AM_RANGE(0xf080, 0xf0bf) AM_WRITE(SMH_RAM) AM_BASE(&goldstar_scroll2)
	AM_RANGE(0xf0c0, 0xf0ff) AM_WRITE(SMH_RAM) AM_BASE(&goldstar_scroll3)
	AM_RANGE(0xf810, 0xf810) AM_READ(input_port_3_r)
	AM_RANGE(0xf811, 0xf811) AM_READ(input_port_4_r)
	AM_RANGE(0xf820, 0xf820) AM_READ(input_port_5_r)	/* DSW 2 */
	AM_RANGE(0xf830, 0xf830) AM_READWRITE(AY8910_read_port_0_r,AY8910_write_port_0_w)
	AM_RANGE(0xf840, 0xf840) AM_WRITE(AY8910_control_port_0_w)
	AM_RANGE(0xfa00, 0xfa00) AM_WRITE(goldstar_fa00_w)
	AM_RANGE(0xfb00, 0xfb00) AM_READWRITE(OKIM6295_status_0_r,OKIM6295_data_0_w)
	AM_RANGE(0xfd00, 0xfdff) AM_READWRITE(SMH_RAM,paletteram_BBGGGRRR_w) AM_BASE(&paletteram)
	AM_RANGE(0xfe00, 0xfe00) AM_READWRITE(protection_r,protection_w)
ADDRESS_MAP_END


static ADDRESS_MAP_START( readport, ADDRESS_SPACE_IO, 8 )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x10, 0x10) AM_READ(input_port_8_r)
ADDRESS_MAP_END

static INPUT_PORTS_START( goldstar )
	PORT_START_TAG("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_CODE(KEYCODE_V) PORT_NAME("Bet Red/2")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_CODE(KEYCODE_C) PORT_NAME("Stop 3/Small/1/Info")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_CODE(KEYCODE_B) PORT_NAME("Bet Blue/Double/3")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_CODE(KEYCODE_Z) PORT_NAME("Stop 1/Take")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_CODE(KEYCODE_X) PORT_NAME("Stop 2/Big/Ticket")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_CODE(KEYCODE_N) PORT_NAME("Start/Stop All/4")

	PORT_START_TAG("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )	/* this is not a coin, not sure what it is */
												/* maybe it's used to buy tickets. Will check soon. */
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_SERVICE_NO_TOGGLE( 0x40, IP_ACTIVE_LOW )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_F1) PORT_NAME("Statistics")

	PORT_START_TAG("DSW1")
	PORT_DIPNAME( 0x01, 0x00, "Game Style" )
	PORT_DIPSETTING(    0x01, "Gettoni" )
	PORT_DIPSETTING(    0x00, "Ticket" )
	PORT_DIPNAME( 0x02, 0x02, "Hopper Out" )
	PORT_DIPSETTING(    0x02, "Active Low" )
	PORT_DIPSETTING(    0x00, "Active High" )
	PORT_DIPNAME( 0x04, 0x04, "Payout Automatic?" )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, "W-Up '7'" )
	PORT_DIPSETTING(    0x08, "Loss" )
	PORT_DIPSETTING(    0x00, "Even" )
	PORT_DIPNAME( 0x10, 0x10, "W-Up Pay Rate" )
	PORT_DIPSETTING(    0x10, "60%" )
	PORT_DIPSETTING(    0x00, "70%" )
	PORT_DIPNAME( 0x20, 0x20, "W-Up Game" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0xc0, 0x00, "Bet Max" )
	PORT_DIPSETTING(    0xc0, "8 Bet" )
	PORT_DIPSETTING(    0x80, "16 Bet" )
	PORT_DIPSETTING(    0x40, "32 Bet" )
	PORT_DIPSETTING(    0x00, "50 Bet" )

	PORT_START_TAG("IN3")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START_TAG("IN4")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START_TAG("DSW2")
	PORT_DIPNAME( 0x07, 0x00, "Main Game Pay Rate" )
	PORT_DIPSETTING(    0x00, "75 %" )
	PORT_DIPSETTING(    0x01, "70 %" )
	PORT_DIPSETTING(    0x02, "65 %" )
	PORT_DIPSETTING(    0x03, "60 %" )
	PORT_DIPSETTING(    0x04, "55 %" )
	PORT_DIPSETTING(    0x05, "50 %" )
	PORT_DIPSETTING(    0x06, "45 %" )
	PORT_DIPSETTING(    0x07, "40 %" )
	PORT_DIPNAME( 0x18, 0x00, "Hopper Limit" )
	PORT_DIPSETTING(    0x18, "300" )
	PORT_DIPSETTING(    0x10, "500" )
	PORT_DIPSETTING(    0x08, "1000" )
	PORT_DIPSETTING(    0x00, "Unlimited" )
	PORT_DIPNAME( 0x20, 0x00, "100 Odds Sound" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, "Key-In Type" )
	PORT_DIPSETTING(    0x40, "B-Type" )
	PORT_DIPSETTING(    0x00, "A-Type" )
	PORT_DIPNAME( 0x80, 0x00, "Center Super 7 Bet Limit" )
	PORT_DIPSETTING(    0x80, "Unlimited" )
	PORT_DIPSETTING(    0x00, "Limited" )

	PORT_START_TAG("DSW3")
	PORT_DIPNAME( 0x0c, 0x00, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(    0x00, "1 Coin/10 Credits" )
	PORT_DIPSETTING(    0x04, "1 Coin/20 Credits" )
	PORT_DIPSETTING(    0x08, "1 Coin/50 Credits" )
	PORT_DIPSETTING(    0x0c, "1 Coin/100 Credits" )
	PORT_DIPNAME( 0xc0, 0x40, "Coin C" )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0xc0, "1 Coin/10 Credits" )

	PORT_START_TAG("DSW4")
	PORT_DIPNAME( 0x07, 0x06, "Credit Limited" )
	PORT_DIPSETTING(    0x07, "5000" )
	PORT_DIPSETTING(    0x06, "10000" )
	PORT_DIPSETTING(    0x05, "20000" )
	PORT_DIPSETTING(    0x04, "30000" )
	PORT_DIPSETTING(    0x03, "40000" )
	PORT_DIPSETTING(    0x02, "50000" )
	PORT_DIPSETTING(    0x01, "100000" )
	PORT_DIPSETTING(    0x00, "Unlimited" )
	PORT_DIPNAME( 0x08, 0x00, "Display Credit Limit" )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "Type of Coin D" )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, "Bonus Play Min Bet" )
	PORT_DIPSETTING(    0x20, "16 Bet" )
	PORT_DIPSETTING(    0x00, "8 Bet" )
	PORT_DIPNAME( 0x40, 0x00, "Reel Speed" )
	PORT_DIPSETTING(    0x40, DEF_STR( Low ) )
	PORT_DIPSETTING(    0x00, DEF_STR( High ) )
	PORT_DIPNAME( 0x80, 0x00, "Ticket Payment" )
	PORT_DIPSETTING(    0x80, "1 Ticket/100" )
	PORT_DIPSETTING(    0x00, "Pay All" )

	PORT_START_TAG("DSW6")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START_TAG("DSW7")	/* ??? */
	PORT_BIT( 0xdf, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_DIPNAME( 0x20, 0x00, "Show Woman" )
	PORT_DIPSETTING(    0x20, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
INPUT_PORTS_END


static const gfx_layout charlayout =
{
	8,8,    /* 8*8 characters */
	4096,    /* 4096 characters */
	3,      /* 3 bits per pixel */
	{ 2, 4, 6 }, /* the bitplanes are packed in one byte */
	{ 0*8+0, 0*8+1, 1*8+0, 1*8+1, 2*8+0, 2*8+1, 3*8+0, 3*8+1 },
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32 },
	32*8   /* every char takes 32 consecutive bytes */
};

static const gfx_layout tilelayout =
{
	8,32,    /* 8*32 characters */
	256,    /* 256 tiles */
	4,      /* 4 bits per pixel */
	{ 0, 2, 4, 6 },
	{ 0, 1, 1*8+0, 1*8+1, 2*8+0, 2*8+1, 3*8+0, 3*8+1 },
	{ 0*8, 4*8, 8*8, 12*8, 16*8, 20*8, 24*8, 28*8,
			32*8, 36*8, 40*8, 44*8, 48*8, 52*8, 56*8, 60*8,
			64*8, 68*8, 72*8, 76*8, 80*8, 84*8, 88*8, 92*8,
			96*8, 100*8, 104*8, 108*8, 112*8, 116*8, 120*8, 124*8 },
	128*8   /* every char takes 128 consecutive bytes */
};


static const gfx_layout tilelayoutbl =
{
	8,32,    /* 8*32 characters */
	256,    /* 256 tiles */
	4,      /* 4 bits per pixel */
	{ 0, 2, 4, 6 },
	{ 0, 1, 2*8+0, 2*8+1, 1*8+0, 1*8+1, 3*8+0, 3*8+1 },
	{ 0*8, 4*8, 8*8, 12*8, 16*8, 20*8, 24*8, 28*8,
			32*8, 36*8, 40*8, 44*8, 48*8, 52*8, 56*8, 60*8,
			64*8, 68*8, 72*8, 76*8, 80*8, 84*8, 88*8, 92*8,
			96*8, 100*8, 104*8, 108*8, 112*8, 116*8, 120*8, 124*8 },
	128*8   /* every char takes 128 consecutive bytes */
};


static GFXDECODE_START( goldstar )
	GFXDECODE_ENTRY( REGION_GFX1, 0, charlayout,   0, 16 )
	GFXDECODE_ENTRY( REGION_GFX2, 0, tilelayout, 128,  8 )
GFXDECODE_END
static GFXDECODE_START( bl )
	GFXDECODE_ENTRY( REGION_GFX1, 0, charlayout,   0, 16 )
	GFXDECODE_ENTRY( REGION_GFX2, 0, tilelayoutbl, 128,  8 )
GFXDECODE_END
static GFXDECODE_START( ml )
	GFXDECODE_ENTRY( REGION_GFX1, 0, charlayout,   0, 16 )
	GFXDECODE_ENTRY( REGION_GFX2, 0x18000, tilelayout, 128,  8 )
GFXDECODE_END



static const struct AY8910interface ay8910_interface =
{
	AY8910_LEGACY_OUTPUT,
	AY8910_DEFAULT_LOADS,
	input_port_7_r,	/* DSW 4 */
	input_port_6_r,	/* DSW 3 */
	NULL,
	NULL
};

static MACHINE_DRIVER_START( goldstar )

	/* basic machine hardware */
	MDRV_CPU_ADD("main", Z80, 3579545)//(4000000?)
	MDRV_CPU_PROGRAM_MAP(map,0)
	MDRV_CPU_IO_MAP(readport,0)
	MDRV_CPU_VBLANK_INT("main", irq0_line_hold)

	/* video hardware */
	MDRV_SCREEN_ADD("main", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(64*8, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(0*8, 64*8-1, 2*8, 30*8-1)

	MDRV_GFXDECODE(goldstar)
	MDRV_PALETTE_LENGTH(256)
	MDRV_NVRAM_HANDLER(goldstar)

	MDRV_VIDEO_START(goldstar)
	MDRV_VIDEO_UPDATE(goldstar)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")//set up a standard mono speaker called 'mono'
	MDRV_SOUND_ADD("ay", AY8910,1500000)//1 AY8910, at clock 150000Hz
	MDRV_SOUND_CONFIG(ay8910_interface)//read extra data from interface
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)//all sound goes to the 'mono' speaker, at 0.50 X maximum

	MDRV_SOUND_ADD("oki", OKIM6295, 1056000)//clock
	MDRV_SOUND_CONFIG(okim6295_interface_region_1_pin7high) // clock frequency & pin 7 not verified //REGION_SOUND1
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)//all sound goes to the 'mono' speaker, at 1.0 X maximum

MACHINE_DRIVER_END


static MACHINE_DRIVER_START( goldstbl )

	/* basic machine hardware */
	MDRV_CPU_ADD("main", Z80, 3579545)//(4000000?)
	MDRV_CPU_PROGRAM_MAP(map,0)
	MDRV_CPU_IO_MAP(readport,0)
	MDRV_CPU_VBLANK_INT("main", irq0_line_hold)

	/* video hardware */
	MDRV_SCREEN_ADD("main", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
//  MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(64*8, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(0*8, 64*8-1, 2*8, 30*8-1)

	MDRV_GFXDECODE(bl)
	MDRV_PALETTE_LENGTH(256)
	MDRV_NVRAM_HANDLER(goldstar)

	MDRV_VIDEO_START(goldstar)
	MDRV_VIDEO_UPDATE(goldstar)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")//set up a standard mono speaker called 'mono'
	MDRV_SOUND_ADD("ay", AY8910,1500000)//1 AY8910, at clock 150000Hz
	MDRV_SOUND_CONFIG(ay8910_interface)//read extra data from interface
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)//all sound goes to the 'mono' speaker, at 0.50 X maximum

	MDRV_SOUND_ADD("oki", OKIM6295, 1056000)//clock
	MDRV_SOUND_CONFIG(okim6295_interface_region_1_pin7high) // clock frequency & pin 7 not verified //REGION_SOUND1
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)//all sound goes to the 'mono' speaker, at 1.0 X maximum
MACHINE_DRIVER_END


static MACHINE_DRIVER_START( moonlght )

	/* basic machine hardware */
	MDRV_CPU_ADD("main", Z80, 3579545)//(4000000?)
	MDRV_CPU_PROGRAM_MAP(map,0)
	MDRV_CPU_IO_MAP(readport,0)
	MDRV_CPU_VBLANK_INT("main", irq0_line_hold)

	/* video hardware */
	MDRV_SCREEN_ADD("main", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
//  MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(64*8, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(0*8, 64*8-1, 2*8, 30*8-1)

	MDRV_GFXDECODE(ml)
	MDRV_PALETTE_LENGTH(256)
	MDRV_NVRAM_HANDLER(goldstar)

	MDRV_VIDEO_START(goldstar)
	MDRV_VIDEO_UPDATE(goldstar)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")//set up a standard mono speaker called 'mono'
	MDRV_SOUND_ADD("ay", AY8910,1500000)//1 AY8910, at clock 150000Hz
	MDRV_SOUND_CONFIG(ay8910_interface)//read extra data from interface
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)//all sound goes to the 'mono' speaker, at 0.50 X maximum

	MDRV_SOUND_ADD("oki", OKIM6295, 1056000)//clock
	MDRV_SOUND_CONFIG(okim6295_interface_region_1_pin7high)// clock frequency & pin 7 not verified //REGION_SOUND1
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)//all sound goes to the 'mono' speaker, at 1.0 X maximum
MACHINE_DRIVER_END



/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( goldstar )
	ROM_REGION( 0x10000, REGION_CPU1, 0 )
	ROM_LOAD( "gs4-cpu.bin",  0x0000, 0x10000, CRC(73e47d4d) SHA1(df2d8233572dc12e8a4b56e5d4f6c566e4ababc9) )

	ROM_REGION( 0x20000, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD( "gs2.bin",      0x00000, 0x20000, CRC(a2d5b898) SHA1(84cca22c91628cfefb67013652b151f034a06159) )

	ROM_REGION( 0x08000, REGION_GFX2, ROMREGION_DISPOSE )
	ROM_LOAD( "gs3.bin",      0x00000, 0x08000, CRC(8454ce3c) SHA1(74686ebb91f191db8cbc3d0417a5e8112c5b67b1) )

	ROM_REGION( 0x20000, REGION_SOUND1, 0 )	/* Audio ADPCM */
	ROM_LOAD( "gs1-snd.bin",  0x0000, 0x20000, CRC(9d58960f) SHA1(c68edf95743e146398aabf6b9617d18e1f9bf25b) )
ROM_END


ROM_START( goldstbl )
	ROM_REGION( 0x10000, REGION_CPU1, 0 )
	ROM_LOAD( "gsb-cpu.bin",  0x0000, 0x10000, CRC(82b238c3) SHA1(1306e700e213f423bdd79b182aa11335796f7f38) )

	ROM_REGION( 0x20000, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD( "gs2.bin",      0x00000, 0x20000, CRC(a2d5b898) SHA1(84cca22c91628cfefb67013652b151f034a06159) )

	ROM_REGION( 0x08000, REGION_GFX2, ROMREGION_DISPOSE )
	ROM_LOAD( "gsb-spr.bin",  0x00000, 0x08000, CRC(52ecd4c7) SHA1(7ef013020521a0c19ecd67db1c00047e78a3c736) )

	ROM_REGION( 0x20000, REGION_SOUND1, 0 )	/* Audio ADPCM */
	ROM_LOAD( "gs1-snd.bin",  0x0000, 0x20000, CRC(9d58960f) SHA1(c68edf95743e146398aabf6b9617d18e1f9bf25b) )
ROM_END


ROM_START( moonlght )
	ROM_REGION( 0x20000, REGION_CPU1, 0 )
	ROM_LOAD( "4.bin",  	  0x0000, 0x20000, CRC(ecb06cfb) SHA1(e32613cac5583a0fecf04fca98796b91698e530c) )

	ROM_REGION( 0x20000, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD( "28.bin",      0x00000, 0x20000, CRC(76915c0f) SHA1(3f6d1c0dd3d9bf29538181a0e930291b822dad8c) )

	ROM_REGION( 0x20000, REGION_GFX2, ROMREGION_DISPOSE )
	ROM_LOAD( "29.bin",      0x00000, 0x20000, CRC(8a5f274d) SHA1(0f2ad61b00e220fc509c01c11c1a8f4e47b54f2a) )

	ROM_REGION( 0x20000, REGION_SOUND1, 0 )	/* Audio ADPCM */
	ROM_LOAD( "gs1-snd.bin",  0x0000, 0x20000, CRC(9d58960f) SHA1(c68edf95743e146398aabf6b9617d18e1f9bf25b) )
ROM_END


static DRIVER_INIT(goldstar)
{
	int A;
	UINT8 *RAM = memory_region(machine, REGION_CPU1);


	for (A = 0;A < 0x10000;A++)
	{
		if ((A & 0x30) == 0)
			RAM[A] ^= 0x82;
		else
			RAM[A] ^= 0xcc;
	}
}



GAME( 199?, goldstar, 0,        goldstar, goldstar, goldstar, ROT0, "IGS", "Golden Star", 0 )
GAME( 199?, goldstbl, goldstar, goldstbl, goldstar, 0,        ROT0, "IGS", "Golden Star (Blue version)", 0 )
GAME( 199?, moonlght, goldstar, moonlght, goldstar, 0,        ROT0, "unknown", "Moon Light", 0 )
