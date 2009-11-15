/*****************************************************************************************

Speed Attack! (c) 1984 Seta Kikaku Corp.

driver by Pierpaolo Prazzoli & Angelo Salese, based on early work by David Haywood

TODO:
 - Video emulation requires a major conversion to the HD46505SP C.R.T. chip (MC6845 clone)
 - It's possible that there is only one coin chute and not two,needs a real board to know
   more about it.

How to play:
 - A to D selects a card.
 - Turn takes one or more cards into your hand (depends on how many cards you
   putted on the stacks).
 - Left & right puts a card on one of the two stacks.

Notes:
 - According to the text gfx rom, there are also a Taito and a KKK versions out there.

------------------------------------------------------------------------------------------
SPEED ATTACK!
(c)SETA

CPU :Z80 x 1
SOUND   :AY-3-8910 x 1
XTAL    :12MHZ

SETA CUSTOM ?
AC-002 , AC-003

CB1-1   :1C
CB0-2   :1D
CB1-3   :1F
CB0-4   :1H
CB0-5   :7C
CB0-6   :7D
CB0-7   :7E

CB1.BPR :7L TBP18S030
CB2.BPR :6K 82S129

----------------------------------------------------------

DIP SWITCH 8BIT (Default: ALL ON)

SW 1,2 : COIN CREDIT   LL:1-1 HL:1-2 LH:1-5 HH:1-10
SW 3,4 : LEVEL LL:EASY -> LH -> HL -> HH:HARD
SW 5,6 : NOT USE
SW 7   : FLIP SCREEN H:FLIP
SW 8   : TEST MODE H:TEST

   PARTS SIDE | SOLDIER SIDE
  ----------------------------
      GND   | 1|    GND
      GND   | 2|    GND
      +5V   | 3|    +5V
            | 4|
     +12V   | 5|   +12V
  SPEAKER(+)| 6|  SPEAKER(-)
     SYNC   | 7| COIN COUNTER
       B    | 8|  SERVICE
       G    | 9|  COIN SW
       R    |10|
     PD 6   |11|   PS 6 (NOT USE)
     PD 5   |12|   PS 5 (NOT USE)
     PD 4   |13|   PS 4
     PD 3   |14|   PS 3
     PD 1   |15|   PS 1
     PD 2   |16|   PS 2
            |17|
            |18|

PS / PD :  key matrix
*****************************************************************************************/

#include "driver.h"
#include "cpu/z80/z80.h"
#include "sound/ay8910.h"
#include "video/mc6845.h"

static UINT8 mux_data;
static UINT8 km_status,coin_settings;

#define MASTER_CLOCK XTAL_12MHz

extern WRITE8_HANDLER( speedatk_videoram_w );
extern WRITE8_HANDLER( speedatk_colorram_w );
extern PALETTE_INIT( speedatk );
extern VIDEO_START( speedatk );
extern VIDEO_UPDATE( speedatk );

/* This "key matrix" device maps some buttons with multiple bit activations,for        *
 * example pressing button A + button B it causes an output of button C.               *
 * This function converts the bit inputs of this game into the usual way MAME does,and *
 * it handles the multiplexer device between player one and two.                       */
static READ8_HANDLER( key_matrix_r )
{
	static UINT8 coin_impulse;

	if(coin_impulse > 0)
	{
		coin_impulse--;
		return 0x80;
	}

	if((input_port_read(space->machine,"COINS") & 1) || (input_port_read(space->machine,"COINS") & 2))
	{
		coin_impulse = coin_settings;
		coin_impulse--;
		return 0x80;
	}

	switch(mux_data)
	{
		case 0x02:
		{
			switch(input_port_read(space->machine, "P1"))
			{
				case 0x002: return 0x02;
				case 0x001: return 0x01;
				case 0x004: return 0x03;
				case 0x008: return 0x04;
				case 0x010: return 0x07;
				case 0x020: return 0x08;
				case 0x040: return 0x09;
				case 0x080: return 0x0a;
				case 0x100: return 0x10;
				case 0x200: return 0x20;
				case 0x400: return 0x40;
				default:	return 0x00;
			}
		}
		case 0x04:
		{
			switch(input_port_read(space->machine, "P2"))
			{
				case 0x002: return 0x02;
				case 0x001: return 0x01;
		 		case 0x004: return 0x03;
		 		case 0x008: return 0x04;
				case 0x010: return 0x07;
				case 0x020: return 0x08;
				case 0x040: return 0x09;
				case 0x080: return 0x0a;
				case 0x100: return 0x10;
				case 0x200: return 0x20;
				case 0x400: return 0x40;
				default:	return 0x00;
			}
		}
		default: logerror("Input reads with mux_data = %x\n",mux_data);
	}

	return 0x00;
}

static WRITE8_HANDLER( key_matrix_w )
{
	mux_data = data;
}

/*Key matrix status,used for coin settings and I don't know what else...*/
static READ8_HANDLER( key_matrix_status_r )
{
	/*bit 0: busy flag,active low*/
	return (km_status & 0xfe) | 1;
}

/*
high four bits are for command, low four are for param
My guess is that the other commands configs the key matrix, it probably needs some tests on the real thing.
1f
3f
41
61
8x coinage setting command
a1
*/
static WRITE8_HANDLER( key_matrix_status_w )
{
	km_status = data;
	if((km_status & 0xf0) == 0x80) //coinage setting command
		coin_settings = km_status & 0xf;
}

static ADDRESS_MAP_START( speedatk_mem, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0x8000, 0x8000) AM_READWRITE(key_matrix_r,key_matrix_w)
	AM_RANGE(0x8001, 0x8001) AM_READWRITE(key_matrix_status_r,key_matrix_status_w)
	AM_RANGE(0x8800, 0x8fff) AM_RAM
	AM_RANGE(0xa000, 0xa3ff) AM_RAM_WRITE(speedatk_videoram_w) AM_BASE(&videoram)
	AM_RANGE(0xb000, 0xb3ff) AM_RAM_WRITE(speedatk_colorram_w) AM_BASE(&colorram)
ADDRESS_MAP_END

static ADDRESS_MAP_START( speedatk_io, ADDRESS_SPACE_IO, 8 )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x00) AM_DEVWRITE("crtc", mc6845_address_w)
	AM_RANGE(0x01, 0x01) AM_DEVWRITE("crtc", mc6845_register_w)
	AM_RANGE(0x24, 0x24) AM_WRITE(watchdog_reset_w)  //watchdog
	AM_RANGE(0x40, 0x40) AM_DEVREAD("aysnd", ay8910_r)
	AM_RANGE(0x40, 0x41) AM_DEVWRITE("aysnd", ay8910_address_data_w)
	//what's 60-6f for? Seems used only in attract mode...
ADDRESS_MAP_END

static INPUT_PORTS_START( speedatk )
	PORT_START("DSW")
	PORT_SERVICE( 0x01, IP_ACTIVE_HIGH )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Unused ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Unused ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x30, 0x00, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x30, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0xc0, 0x00, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0xc0, "1 Coin/10 Credits" )

	PORT_START("P1")
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(1) PORT_NAME("P1 A") PORT_CODE(KEYCODE_Z)
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(1) PORT_NAME("P1 B") PORT_CODE(KEYCODE_X)
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_PLAYER(1) PORT_NAME("P1 C") PORT_CODE(KEYCODE_C)
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_PLAYER(1) PORT_NAME("P1 D") PORT_CODE(KEYCODE_V)
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_BUTTON5 ) PORT_PLAYER(1) PORT_NAME("P1 Left") PORT_CODE(KEYCODE_A)
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_BUTTON6 ) PORT_PLAYER(1) PORT_NAME("P1 Right") PORT_CODE(KEYCODE_S)
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_BUTTON7 ) PORT_PLAYER(1) PORT_NAME("P1 Turn") PORT_CODE(KEYCODE_Q)
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("P2")
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(2) PORT_NAME("P2 A")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(2) PORT_NAME("P2 B")
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_PLAYER(2) PORT_NAME("P2 C")
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_PLAYER(2) PORT_NAME("P2 D")
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_BUTTON5 ) PORT_PLAYER(2) PORT_NAME("P2 Left")
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_BUTTON6 ) PORT_PLAYER(2) PORT_NAME("P2 Right")
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_BUTTON7 ) PORT_PLAYER(2) PORT_NAME("P2 Turn")
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("COINS")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_COIN1 ) PORT_IMPULSE(1)
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_COIN2 ) PORT_IMPULSE(1)
INPUT_PORTS_END

static const gfx_layout charlayout_1bpp =
{
	8,8,
	RGN_FRAC(1,1),
	3,
	{ 0, 0, 0 },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8
};

static const gfx_layout charlayout_3bpp =
{
	8,8,
	RGN_FRAC(1,3),
	3,
	{ RGN_FRAC(2,3), RGN_FRAC(1,3), RGN_FRAC(0,3) },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8
};


static GFXDECODE_START( speedatk )
	GFXDECODE_ENTRY( "gfx1", 0, charlayout_1bpp,   0, 32 )
	GFXDECODE_ENTRY( "gfx2", 0, charlayout_3bpp,   0, 32 )
GFXDECODE_END

static const mc6845_interface mc6845_intf =
{
	"screen",	/* screen we are acting on */
	8,			/* number of pixels per video memory address */
	NULL,		/* before pixel update callback */
	NULL,		/* row update callback */
	NULL,		/* after pixel update callback */
	DEVCB_NULL,	/* callback for display state changes */
	DEVCB_NULL,	/* callback for cursor state changes */
	DEVCB_NULL,	/* HSYNC callback */
	DEVCB_NULL,	/* VSYNC callback */
	NULL		/* update address callback */
};

static WRITE8_DEVICE_HANDLER( speedatk_output_w )
{
	//flip_screen_set(device->machine, data & 0x80); //breaks the video emulation?

	//if((data & 0x7f) != 0x7f)
	//popmessage("%02x",data);
}

static const ay8910_interface ay8910_config =
{
	AY8910_LEGACY_OUTPUT,
	AY8910_DEFAULT_LOADS,
	DEVCB_NULL,
	DEVCB_INPUT_PORT("DSW"),
	DEVCB_HANDLER(speedatk_output_w),
	DEVCB_NULL
};

static MACHINE_DRIVER_START( speedatk )
	MDRV_CPU_ADD("maincpu", Z80,MASTER_CLOCK/2) //divider is unknown
	MDRV_CPU_PROGRAM_MAP(speedatk_mem)
	MDRV_CPU_IO_MAP(speedatk_io)
	MDRV_CPU_VBLANK_INT("screen", irq0_line_hold)

	MDRV_WATCHDOG_VBLANK_INIT(8) // timing is unknown

	/* video hardware */
	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(320, 256)
	MDRV_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 0*8, 32*8-1)

	MDRV_MC6845_ADD("crtc", H46505, MASTER_CLOCK/16, mc6845_intf)	/* hand tuned to get ~60 fps */

	MDRV_GFXDECODE(speedatk)
	MDRV_PALETTE_LENGTH(0x100)
	MDRV_PALETTE_INIT(speedatk)

	MDRV_VIDEO_START(speedatk)
	MDRV_VIDEO_UPDATE(speedatk)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_SOUND_ADD("aysnd", AY8910, MASTER_CLOCK/4) //divider is unknown
	MDRV_SOUND_CONFIG(ay8910_config)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.5)
MACHINE_DRIVER_END

ROM_START( speedatk )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "cb1-1",        0x0000, 0x2000, CRC(df988e05) SHA1(0ec91c5f2e1adf952a4fe7aede591e763773a75b) )
	ROM_LOAD( "cb0-2",        0x2000, 0x2000, CRC(be949154) SHA1(8a594a7ebdc8456290919163f7ea4ccb0d1f4edb) )
	ROM_LOAD( "cb1-3",        0x4000, 0x2000, CRC(741a5949) SHA1(7f7bebd4fb73fef9aa28549d100f632c442ac9b3) )
	ROM_LOAD( "cb0-4",        0x6000, 0x2000, CRC(53a9c0c8) SHA1(cd0fd94411dabf09828c1f629891158c40794127) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "cb0-7",        0x0000, 0x2000, CRC(a86007b5) SHA1(8e5cab76c37a8d53e1355000cd1a0a85ffae0e8c) )

	ROM_REGION( 0x6000, "gfx2", 0 )
	ROM_LOAD( "cb0-5",        0x0000, 0x2000, CRC(47a966e7) SHA1(fdaa0f88656afc431bae367679ce6298fa962e0f) )
	ROM_LOAD( "cb0-6",        0x2000, 0x2000, CRC(cc1da937) SHA1(1697bb008bfa5c33a282bd470ac39c324eea7509) )
	ROM_COPY( "gfx2",    	  0x0000, 0x4000, 0x1000 ) /* Fill the blank space with cards gfx */
	ROM_COPY( "gfx1",    	  0x1000, 0x5000, 0x1000 ) /* Gfx from cb0-7 */

	ROM_REGION( 0x0120, "proms", 0 )
	ROM_LOAD( "cb1.bpr",      0x0000, 0x0020, CRC(a0176c23) SHA1(133fb9eef8a6595cac2dcd7edce4789899a59e84) ) /* color PROM */
	ROM_LOAD( "cb2.bpr",      0x0020, 0x0100, CRC(a604cf96) SHA1(a4ef6e77dcd3abe4c27e8e636222a5ee711a51f5) ) /* lookup table */
ROM_END

GAME( 1984, speedatk, 0, speedatk, speedatk, 0, ROT0, "Seta Kikaku Corp.", "Speed Attack! (Japan)", GAME_NO_COCKTAIL )

