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
	AM_RANGE(0xf800, 0xf800) AM_READ_PORT("IN0")
	AM_RANGE(0xf801, 0xf801) AM_READ_PORT("IN1")	/* Test Mode */
	AM_RANGE(0xf802, 0xf802) AM_READ_PORT("DSW1")
//  AM_RANGE(0xf803, 0xf803)
//  AM_RANGE(0xf804, 0xf804)
	AM_RANGE(0xf805, 0xf805) AM_READ_PORT("DSW4")	/* DSW 4 (also appears in 8910 port) */
	AM_RANGE(0xf806, 0xf806) AM_READ_PORT("DSW7")	/* (don't know to which one of the */
													/* service mode dip switches it should map) */
	AM_RANGE(0xf080, 0xf0bf) AM_WRITE(SMH_RAM) AM_BASE(&goldstar_scroll2)
	AM_RANGE(0xf0c0, 0xf0ff) AM_WRITE(SMH_RAM) AM_BASE(&goldstar_scroll3)
	AM_RANGE(0xf810, 0xf810) AM_READ_PORT("UNK1")
	AM_RANGE(0xf811, 0xf811) AM_READ_PORT("UNK2")
	AM_RANGE(0xf820, 0xf820) AM_READ_PORT("DSW2")
	AM_RANGE(0xf830, 0xf830) AM_READWRITE(ay8910_read_port_0_r,ay8910_write_port_0_w)
	AM_RANGE(0xf840, 0xf840) AM_WRITE(ay8910_control_port_0_w)
	AM_RANGE(0xfa00, 0xfa00) AM_WRITE(goldstar_fa00_w)
	AM_RANGE(0xfb00, 0xfb00) AM_READWRITE(okim6295_status_0_r,okim6295_data_0_w)
	AM_RANGE(0xfd00, 0xfdff) AM_READWRITE(SMH_RAM,paletteram_BBGGGRRR_w) AM_BASE(&paletteram)
	AM_RANGE(0xfe00, 0xfe00) AM_READWRITE(protection_r,protection_w)
ADDRESS_MAP_END


static ADDRESS_MAP_START( readport, ADDRESS_SPACE_IO, 8 )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x10, 0x10) AM_READ_PORT("DSW6")
ADDRESS_MAP_END

static INPUT_PORTS_START( goldstar )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_CODE(KEYCODE_V) PORT_NAME("Bet Red/2")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_CODE(KEYCODE_C) PORT_NAME("Stop 3/Small/1/Info")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_CODE(KEYCODE_B) PORT_NAME("Bet Blue/Double/3")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_CODE(KEYCODE_Z) PORT_NAME("Stop 1/Take")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_CODE(KEYCODE_X) PORT_NAME("Stop 2/Big/Ticket")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_CODE(KEYCODE_N) PORT_NAME("Start/Stop All/4")

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )	/* this is not a coin, not sure what it is */
												/* maybe it's used to buy tickets. Will check soon. */
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_SERVICE_NO_TOGGLE( 0x40, IP_ACTIVE_LOW )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_F1) PORT_NAME("Statistics")

	PORT_START("DSW1")
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

	PORT_START("UNK1")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("UNK2")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW2")
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

	PORT_START("DSW3")
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

	PORT_START("DSW4")
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

	PORT_START("DSW6")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW7")	/* ??? */
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


static const gfx_layout charlayout_chry10 =
{
	8,8,    /* 8*8 characters */
	RGN_FRAC(1,1),    /* 4096 characters */
	3,      /* 3 bits per pixel */
	{ 2, 4, 6 }, /* the bitplanes are packed in one byte */
	{ 3*8+0, 3*8+1, 2*8+0, 2*8+1, 1*8+0, 1*8+1, 0*8+0, 0*8+1 },
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

static const gfx_layout tilelayout_chry10 =
{
	8,32,    /* 8*32 characters */
	256,    /* 256 tiles */
	4,      /* 4 bits per pixel */
	{ 0, 2, 4, 6 },
	{ 3*8+0, 3*8+1, 2*8+0, 2*8+1, 1*8+0, 1*8+1, 0*8+0, 0*8+1 },
	{ 0*8, 4*8, 8*8, 12*8, 16*8, 20*8, 24*8, 28*8,
			32*8, 36*8, 40*8, 44*8, 48*8, 52*8, 56*8, 60*8,
			64*8, 68*8, 72*8, 76*8, 80*8, 84*8, 88*8, 92*8,
			96*8, 100*8, 104*8, 108*8, 112*8, 116*8, 120*8, 124*8 },
	128*8   /* every char takes 128 consecutive bytes */
};




static GFXDECODE_START( goldstar )
	GFXDECODE_ENTRY( "gfx1", 0, charlayout,   0, 16 )
	GFXDECODE_ENTRY( "gfx2", 0, tilelayout, 128,  8 )
GFXDECODE_END
static GFXDECODE_START( bl )
	GFXDECODE_ENTRY( "gfx1", 0, charlayout,   0, 16 )
	GFXDECODE_ENTRY( "gfx2", 0, tilelayoutbl, 128,  8 )
GFXDECODE_END
static GFXDECODE_START( ml )
	GFXDECODE_ENTRY( "gfx1", 0, charlayout,   0, 16 )
	GFXDECODE_ENTRY( "gfx2", 0x18000, tilelayout, 128,  8 )
GFXDECODE_END

static GFXDECODE_START( chry10 )
	GFXDECODE_ENTRY( "gfx1", 0, charlayout_chry10,   0, 16 )
	GFXDECODE_ENTRY( "gfx2", 0, tilelayout_chry10, 128,  8 )
GFXDECODE_END



static const ay8910_interface ay8910_config =
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
	MDRV_SOUND_CONFIG(ay8910_config)//read extra data from interface
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)//all sound goes to the 'mono' speaker, at 0.50 X maximum

	MDRV_SOUND_ADD("oki", OKIM6295, 1056000)//clock
	MDRV_SOUND_CONFIG(okim6295_interface_pin7high) // clock frequency & pin 7 not verified //"oki"
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
	MDRV_SOUND_CONFIG(ay8910_config)//read extra data from interface
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)//all sound goes to the 'mono' speaker, at 0.50 X maximum

	MDRV_SOUND_ADD("oki", OKIM6295, 1056000)//clock
	MDRV_SOUND_CONFIG(okim6295_interface_pin7high) // clock frequency & pin 7 not verified //"oki"
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
	MDRV_SOUND_CONFIG(ay8910_config)//read extra data from interface
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)//all sound goes to the 'mono' speaker, at 0.50 X maximum

	MDRV_SOUND_ADD("oki", OKIM6295, 1056000)//clock
	MDRV_SOUND_CONFIG(okim6295_interface_pin7high)// clock frequency & pin 7 not verified //"oki"
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)//all sound goes to the 'mono' speaker, at 1.0 X maximum
MACHINE_DRIVER_END



static MACHINE_DRIVER_START( chry10 )

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

	MDRV_GFXDECODE(chry10)
	MDRV_PALETTE_LENGTH(256)
	MDRV_NVRAM_HANDLER(goldstar)

	MDRV_VIDEO_START(goldstar)
	MDRV_VIDEO_UPDATE(goldstar)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")//set up a standard mono speaker called 'mono'
	MDRV_SOUND_ADD("ay", AY8910,1500000)//1 AY8910, at clock 150000Hz
	MDRV_SOUND_CONFIG(ay8910_config)//read extra data from interface
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)//all sound goes to the 'mono' speaker, at 0.50 X maximum

	MDRV_SOUND_ADD("oki", OKIM6295, 1056000)//clock
	MDRV_SOUND_CONFIG(okim6295_interface_pin7high) // clock frequency & pin 7 not verified //"oki"
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)//all sound goes to the 'mono' speaker, at 1.0 X maximum
MACHINE_DRIVER_END



/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( goldstar )
	ROM_REGION( 0x10000, "main", 0 )
	ROM_LOAD( "gs4-cpu.bin",  0x0000, 0x10000, CRC(73e47d4d) SHA1(df2d8233572dc12e8a4b56e5d4f6c566e4ababc9) )

	ROM_REGION( 0x20000, "gfx1", ROMREGION_DISPOSE )
	ROM_LOAD( "gs2.bin",      0x00000, 0x20000, CRC(a2d5b898) SHA1(84cca22c91628cfefb67013652b151f034a06159) )

	ROM_REGION( 0x08000, "gfx2", ROMREGION_DISPOSE )
	ROM_LOAD( "gs3.bin",      0x00000, 0x08000, CRC(8454ce3c) SHA1(74686ebb91f191db8cbc3d0417a5e8112c5b67b1) )

	ROM_REGION( 0x20000, "oki", 0 )	/* Audio ADPCM */
	ROM_LOAD( "gs1-snd.bin",  0x0000, 0x20000, CRC(9d58960f) SHA1(c68edf95743e146398aabf6b9617d18e1f9bf25b) )
ROM_END


ROM_START( goldstbl )
	ROM_REGION( 0x10000, "main", 0 )
	ROM_LOAD( "gsb-cpu.bin",  0x0000, 0x10000, CRC(82b238c3) SHA1(1306e700e213f423bdd79b182aa11335796f7f38) )

	ROM_REGION( 0x20000, "gfx1", ROMREGION_DISPOSE )
	ROM_LOAD( "gs2.bin",      0x00000, 0x20000, CRC(a2d5b898) SHA1(84cca22c91628cfefb67013652b151f034a06159) )

	ROM_REGION( 0x08000, "gfx2", ROMREGION_DISPOSE )
	ROM_LOAD( "gsb-spr.bin",  0x00000, 0x08000, CRC(52ecd4c7) SHA1(7ef013020521a0c19ecd67db1c00047e78a3c736) )

	ROM_REGION( 0x20000, "oki", 0 )	/* Audio ADPCM */
	ROM_LOAD( "gs1-snd.bin",  0x0000, 0x20000, CRC(9d58960f) SHA1(c68edf95743e146398aabf6b9617d18e1f9bf25b) )
ROM_END

/*

Cherry I Gold

Anno    1998
Produttore
N.revisione W4BON (rev.1)


CPU

1x TMPZ84C00AP-6 (u12)(main)
2x D8255AC-2 (u45,u46) (missing)
1x D71055C (u40) (missing)
1x YM2149 (u39)
1x SN76489AN (u38)
1x oscillator 12.0C45

ROMs

1x I27256 (u3)
1x I27C010 (u1)
1x PROM N82S147AN (u2)
1x M27C512 (u20)
1x GAL20V8 (pl1)(read protected)
1x PALCE20V8H (pl2)(read protected)
1x ispLSI1024-60LJ (pl3)(read protected)
3x PALCE16V8H (pl4,pl6,pl7)(read protected)
1x PEEL22CV10 (pl5)(read protected)
Note

1x 36x2 edge connector
1x 10x2 edge connector
2x trimmer (volume)
5x 8x2 switches dip (sw1-5)
1x push lever (TS)


Cherry Gold  (Cherry 10)

Anno    1997
Produttore
N.revisione W4BON (rev.1)

CPU

1x TMPZ84C00AP-6 (u12)(main)
2x D8255AC-2 (u45,u46)
1x D71055C (u40)
1x WF19054 (u39)(equivalent to AY-3-8910)
1x SN76489AN (u38)
1x PIC16F84 (on a small daughterboard)(read protected)
1x oscillator 12.000

ROMs

1x TMS27C256 (u3)
1x TMS27C010 (u1)
1x PROM N82S147AN (u2)
1x M27C512 (u20)
2x PALCE20V8H (pl1,pl2)(read protected)
1x ispLSI1024-60LJ (pl3)(read protected)
3x PALCE16V8H (pl4,pl6,pl7)(read protected)
1x GAL22V10B (pl5)(read protected)

Note

1x 36x2 edge connector
1x 10x2 edge connector
2x trimmer (volume)
5x 8x2 switches dip (sw1-5)
1x push lever (TS)

*/

ROM_START( chry10 )
	ROM_REGION( 0x10000, "main", 0 )
	ROM_LOAD( "ver.1h2.u20",  0x0000, 0x10000, CRC(85bbde06) SHA1(f44d335feb4697b195e9fc7e5aeaabf099e21ed8) )

	ROM_REGION( 0x10000, "pic", 0 )
	ROM_LOAD( "pic16f84.bad.dump",    0x00000, 0x014f4, BAD_DUMP CRC(876ff1ed) SHA1(fcd6892e2b8371030af15e4d8c9f4a351ce0551c) )

	ROM_REGION( 0x20000, "gfx1", ROMREGION_DISPOSE )
	ROM_LOAD( "27c010.u1",      0x00000, 0x20000, CRC(05515cf8) SHA1(366dd44ae93bdc4cf456f97f38edac83441cbc89) )

	ROM_REGION( 0x08000, "gfx2", ROMREGION_DISPOSE )
	ROM_LOAD( "1.u3",      0x00000, 0x08000, CRC(32b46e5c) SHA1(49e59589188324e15ec2b8157839423faea9833f) )

	ROM_REGION( 0x0200, "prom", ROMREGION_DISPOSE )
	ROM_LOAD( "82s147.u2",      0x00000, 0x0200, CRC(5c8f2b8f) SHA1(67d2121e75813dd85d83858c5fc5ec6ad9cc2a7d) )

	ROM_REGION( 0x02e5, "palgal", ROMREGION_DISPOSE )
	ROM_LOAD( "palce20v8h.pl1.bad.dump",    0x00000, 0x0157, BAD_DUMP CRC(f0c6d78c) SHA1(03ff589711179950209c405192bd41a032c6c6d6) )
	ROM_LOAD( "palce20v8h.pl2.bad.dump",    0x00000, 0x0157, BAD_DUMP CRC(f0c6d78c) SHA1(03ff589711179950209c405192bd41a032c6c6d6) )
	ROM_LOAD( "palce16v8h.pl4.bad.dump",    0x00000, 0x0117, BAD_DUMP CRC(c89d2f52) SHA1(f9d52d9c42ef95b7b85bbf6d09888ebdeac11fd3) )
	ROM_LOAD( "gal22v10b.pl5.bad.dump",     0x00000, 0x02e5, BAD_DUMP CRC(996854bc) SHA1(647d2f49b739f7ca55c0b85290b6a21256834fd8) )
	ROM_LOAD( "palce16v8h.pl6.bad.dump",    0x00000, 0x0117, BAD_DUMP CRC(7e3d99d8) SHA1(983e10eba11e4aeab5103ae644a8e6181d9b27a9) )
	ROM_LOAD( "palce16v8h.pl7.bad.dump",    0x00000, 0x0117, BAD_DUMP CRC(c89d2f52) SHA1(f9d52d9c42ef95b7b85bbf6d09888ebdeac11fd3) )

	ROM_REGION( 0x20000, "oki", ROMREGION_ERASE00 )
	/* no oki on this pcb .. */
ROM_END



ROM_START( chryigld )
	ROM_REGION( 0x10000, "main", 0 )
	ROM_LOAD( "ol-v9.u20",  0x00000, 0x10000, CRC(b61c0695) SHA1(63c44b20fd7f76bdb33331273d2610e8cfd31add) )

	ROM_REGION( 0x20000, "gfx1", ROMREGION_DISPOSE )
	ROM_LOAD( "ol-la.u1",      0x00000, 0x20000, CRC(c3c912f1) SHA1(a2131f092ae1971f79a11d6a18b031cd98529320) )

	ROM_REGION( 0x08000, "gfx2", ROMREGION_DISPOSE )
	ROM_LOAD( "1.u3",      0x00000, 0x08000, CRC(32b46e5c) SHA1(49e59589188324e15ec2b8157839423faea9833f) )

	ROM_REGION( 0x0200, "prom", ROMREGION_DISPOSE )
	ROM_LOAD( "82s147.u2",      0x00000, 0x0200, CRC(5c8f2b8f) SHA1(67d2121e75813dd85d83858c5fc5ec6ad9cc2a7d) )

	ROM_REGION( 0x02dd, "palgal", ROMREGION_DISPOSE )
	ROM_LOAD( "gal20v8.pl1.bad.dump",    0x00000, 0x0157, BAD_DUMP CRC(bf885908) SHA1(6cac1022172ee0c178fd3b9c187b1ffb4742898f) )
	ROM_LOAD( "palce20v8h.pl2.bad.dump", 0x00000, 0x0157, BAD_DUMP CRC(f0c6d78c) SHA1(03ff589711179950209c405192bd41a032c6c6d6) )
	ROM_LOAD( "palce16v8h.pl4.bad.dump", 0x00000, 0x0117, BAD_DUMP CRC(c89d2f52) SHA1(f9d52d9c42ef95b7b85bbf6d09888ebdeac11fd3) )
	ROM_LOAD( "peel22cv10a.pl5.bad.dump",0x00000, 0x02dd, BAD_DUMP CRC(8e6075d9) SHA1(f2c1b6497a4d9e873d36b89771c135a2cd91d05f) )
	ROM_LOAD( "palce16v8h.pl6.bad.dump", 0x00000, 0x0117, BAD_DUMP CRC(7e3d99d8) SHA1(983e10eba11e4aeab5103ae644a8e6181d9b27a9) )
	ROM_LOAD( "palce16v8h.pl7.bad.dump", 0x00000, 0x0117, BAD_DUMP CRC(c89d2f52) SHA1(f9d52d9c42ef95b7b85bbf6d09888ebdeac11fd3) )

	ROM_REGION( 0x20000, "oki", ROMREGION_ERASE00 )
	/* no oki on this pcb .. */
ROM_END



ROM_START( moonlght )
	ROM_REGION( 0x20000, "main", 0 )
	ROM_LOAD( "4.bin",  	  0x0000, 0x20000, CRC(ecb06cfb) SHA1(e32613cac5583a0fecf04fca98796b91698e530c) )

	ROM_REGION( 0x20000, "gfx1", ROMREGION_DISPOSE )
	ROM_LOAD( "28.bin",      0x00000, 0x20000, CRC(76915c0f) SHA1(3f6d1c0dd3d9bf29538181a0e930291b822dad8c) )

	ROM_REGION( 0x20000, "gfx2", ROMREGION_DISPOSE )
	ROM_LOAD( "29.bin",      0x00000, 0x20000, CRC(8a5f274d) SHA1(0f2ad61b00e220fc509c01c11c1a8f4e47b54f2a) )

	ROM_REGION( 0x20000, "oki", 0 )	/* Audio ADPCM */
	ROM_LOAD( "gs1-snd.bin",  0x0000, 0x20000, CRC(9d58960f) SHA1(c68edf95743e146398aabf6b9617d18e1f9bf25b) )
ROM_END


static DRIVER_INIT(goldstar)
{
	int A;
	UINT8 *ROM = memory_region(machine, "main");

	for (A = 0;A < 0x10000;A++)
	{
		if ((A & 0x30) == 0)
			ROM[A] ^= 0x82;
		else
			ROM[A] ^= 0xcc;
	}
}

static DRIVER_INIT( chry10 )
{
	int A;
	UINT8 *ROM = memory_region(machine, "main");
	UINT8 *buffer;
	buffer = malloc(0x10000);

	// there is more to this..
	for (A = 0;A < 0x10000;A++)
	{
		buffer[A^0x800] = ROM[A];
	}

	memcpy(ROM,buffer,0x10000);
	free(buffer);

	#if 0
	{
		FILE *fp;
		char filename[256];
		sprintf(filename,"decrypted_%s", machine->gamedrv->name);
		fp=fopen(filename, "w+b");
		if (fp)
		{
			fwrite(ROM, 0x10000, 1, fp);
			fclose(fp);
		}
	}
	#endif

	// these are scrambled, 0x800 should go at 0x000, data seems swapped too
}


GAME( 199?, goldstar, 0,        goldstar, goldstar, goldstar, ROT0, "IGS", "Golden Star", 0 )
GAME( 199?, goldstbl, goldstar, goldstbl, goldstar, 0,        ROT0, "IGS", "Golden Star (Blue version)", 0 )
GAME( 199?, moonlght, goldstar, moonlght, goldstar, 0,        ROT0, "bootleg", "Moon Light (bootleg of Golden Star)", 0 )
GAME( 199?, chry10,  goldstar,  chry10,  goldstar, chry10,  ROT0, "bootleg", "Cherry 10 (bootleg of Golden Star)", GAME_NOT_WORKING )
GAME( 199?, chryigld, goldstar, chry10,  goldstar, chry10,  ROT0, "bootleg", "Cherry I Gold (bootleg of Golden Star)", GAME_NOT_WORKING )
