/*          Ramtek M79 Ambush


The following chart explains the settings of the eight switches on
the DIP switch.  A plus(+) in the column means the toggle switch is
up on the plus side of the DIP switch.

                                SWITCHES
                         |  12  |  345  |  678  |
------------------------------------------------|
Length of Game (seconds) |      |       |       |
                  60     |  00  |       |       |
                  90     |  0+  |       |       |
                  90     |  +0  |       |       |
                 120     |  ++  |       |       |
-------------------------+------+-------+-------|
Points for Extended Time |      |       |       |
                1500     |      |  000  |       |
                2500     |      |  +00  |       |
                3500     |      |  0+0  |       |
                5000     |      |  ++0  |       |
    NO extended time     |      |  +++  |       |
-------------------------+------+-------+-------|
Coins Per Game           |      |       |       |
 Free Play - two players |      |       |  0++  |
 One Coin  - two players |      |       |  0+0  |
 One Coin  - each player |      |       |  000  |
 Two Coins - each player |      |       |  +00  |
-------------------------------------------------


Based on extensive tests on location, the factory settings for the
most universal combinations are:
    60 second long game
    2500 points for extended play             12345678
    One coin each player                      00+00000

Ports:
 In:
  8000 DIP SW
  8002 D0=VBlank
  8004 Game Switch Inputs
  8005 Game Switch Inputs

 Out:
  8000
  8001 Mask Sel (Manual calls it "Select All RAM")
  8002 Sound Control (According to Manual)
  8003 D0=SelfTest LED

*/

#include "driver.h"
#include "m79amb.h"
#include "cpu/i8085/i8085.h"

static UINT8 *ramtek_videoram;
static UINT8 *mask;


static WRITE8_HANDLER( ramtek_videoram_w )
{
	ramtek_videoram[offset] = data & ~*mask;
}

static VIDEO_UPDATE( ramtek )
{
	offs_t offs;

	for (offs = 0; offs < 0x2000; offs++)
	{
		int i;

		UINT8 data = ramtek_videoram[offs];
		int y = offs >> 5;
		int x = (offs & 0x1f) << 3;

		for (i = 0; i < 8; i++)
		{
			pen_t pen = (data & 0x80) ? RGB_WHITE : RGB_BLACK;
			*BITMAP_ADDR32(bitmap, y, x) = pen;

			x++;
			data <<= 1;
		}
	}

	return 0;
}


static const int ControllerTable[32] = {
    0  , 1  , 3  , 2  , 6  , 7  , 5  , 4  ,
    12 , 13 , 15 , 14 , 10 , 11 , 9  , 8  ,
    24 , 25 , 27 , 26 , 30 , 31 , 29 , 28 ,
    20 , 21 , 23 , 22 , 18 , 19 , 17 , 16
};

static READ8_HANDLER( gray5bit_controller0_r )
{
    int port_data = input_port_read(space->machine, "8004");
    return (port_data & 0xe0) | (~ControllerTable[port_data & 0x1f] & 0x1f);
}

static READ8_HANDLER( gray5bit_controller1_r )
{
    int port_data = input_port_read(space->machine, "8005");
    return (port_data & 0xe0) | (~ControllerTable[port_data & 0x1f] & 0x1f);
}

static WRITE8_HANDLER( m79amb_8002_w )
{
	/* D1 may also be watchdog reset */
	/* port goes to 0x7f to turn on explosion lamp */
	output_set_value("EXP_LAMP", data ? 1 : 0);
}

static ADDRESS_MAP_START( main_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x1fff) AM_ROM
	AM_RANGE(0x4000, 0x5fff) AM_RAM_WRITE(ramtek_videoram_w) AM_BASE(&ramtek_videoram)
	AM_RANGE(0x6000, 0x63ff) AM_RAM		/* ?? */
	AM_RANGE(0x8000, 0x8000) AM_READ_PORT("8000") AM_DEVWRITE("discrete", m79amb_8000_w)
	AM_RANGE(0x8001, 0x8001) AM_WRITE(SMH_RAM) AM_BASE(&mask)
	AM_RANGE(0x8002, 0x8002) AM_READ_PORT("8002") AM_WRITE(m79amb_8002_w)
	AM_RANGE(0x8003, 0x8003) AM_DEVWRITE("discrete", m79amb_8003_w)
	AM_RANGE(0x8004, 0x8004) AM_READ(gray5bit_controller0_r)
	AM_RANGE(0x8005, 0x8005) AM_READ(gray5bit_controller1_r)
	AM_RANGE(0xc000, 0xc07f) AM_RAM					/* ?? */
	AM_RANGE(0xc200, 0xc27f) AM_RAM					/* ?? */
ADDRESS_MAP_END



static INPUT_PORTS_START( m79amb )
	PORT_START("8000")
	PORT_DIPNAME( 0x03, 0x00, "Play Time" )
	PORT_DIPSETTING(    0x00, "60 Seconds" )
	PORT_DIPSETTING(    0x01, "90 Seconds" )
	PORT_DIPSETTING(    0x02, "90 Seconds" )
	PORT_DIPSETTING(    0x03, "120 Seconds" )
	PORT_DIPNAME( 0x1c, 0x04, "Points for Extended Time" )
	PORT_DIPSETTING(    0x00, "1500" )
	PORT_DIPSETTING(    0x04, "2500" )
	PORT_DIPSETTING(    0x08, "3500" )
	PORT_DIPSETTING(    0x0c, "5000" )
	PORT_DIPSETTING(    0x1c, "No Extended Time" )
	PORT_DIPNAME( 0xe0, 0x00, DEF_STR( Coinage ))
	PORT_DIPSETTING(    0x20, DEF_STR( 2C_1C ))
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ))
	PORT_DIPSETTING(    0x40, DEF_STR( 1C_2C ))
	PORT_DIPSETTING(    0xc0, DEF_STR( Free_Play ))

	PORT_START("8002")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_VBLANK )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_TILT )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("8004")
	PORT_BIT( 0x1f, 0x10, IPT_PADDLE ) PORT_MINMAX(0,0x1f) PORT_SENSITIVITY(25) PORT_KEYDELTA(10) PORT_CENTERDELTA(0)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("8005")
	PORT_BIT( 0x1f, 0x10, IPT_PADDLE ) PORT_MINMAX(0,0x1f) PORT_SENSITIVITY(25) PORT_KEYDELTA(10) PORT_CENTERDELTA(0) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END


static INTERRUPT_GEN( m79amb_interrupt )
{
	cpu_set_input_line_and_vector(device, 0, HOLD_LINE, 0xcf);  /* RST 08h */
}

static DRIVER_INIT( m79amb )
{
	UINT8 *rom = memory_region(machine, "maincpu");
	int i;

	/* PROM data is active low */
 	for (i = 0;i < 0x2000;i++)
		rom[i] = ~rom[i];
}

static MACHINE_DRIVER_START( m79amb )

	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu", 8080, XTAL_19_6608MHz / 10)
	MDRV_CPU_PROGRAM_MAP(main_map)
	MDRV_CPU_VBLANK_INT("screen", m79amb_interrupt)

	/* video hardware */
	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500) /* not accurate */)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_RGB32)
	MDRV_SCREEN_SIZE(32*8, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 4*8, 32*8-1)

	MDRV_VIDEO_UPDATE(ramtek)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_SOUND_ADD("discrete", DISCRETE, 0)
	MDRV_SOUND_CONFIG_DISCRETE(m79amb)

	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_DRIVER_END



ROM_START( m79amb )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "m79.10t",      0x0000, 0x0200, CRC(ccf30b1e) SHA1(c1a77f8dc81c491928f81121ca5c9b7f8753794f) )
	ROM_LOAD( "m79.9t",       0x0200, 0x0200, CRC(daf807dd) SHA1(16cd9d553bfb111c8380966cbde39dbddd5fe58c) )
	ROM_LOAD( "m79.8t",       0x0400, 0x0200, CRC(79fafa02) SHA1(440620f5be44febdd7c64014739dc71fb570cc92) )
	ROM_LOAD( "m79.7t",       0x0600, 0x0200, CRC(06f511f8) SHA1(a8dcaec0723b8ac9ad9474e3e931a23d7aa4ec23) )
	ROM_LOAD( "m79.6t",       0x0800, 0x0200, CRC(24634390) SHA1(bc5f2ae5a49904dde1bd5e6b134571bf1d912735) )
	ROM_LOAD( "m79.5t",       0x0a00, 0x0200, CRC(95252aa6) SHA1(e7ea598f864510557b511682a5d2223a512ff029) )
	ROM_LOAD( "m79.4t",       0x0c00, 0x0200, CRC(54cffb0f) SHA1(4a4ad921ef6324c927a2e4a9da624d8096b6d87b) )
	ROM_LOAD( "m79.3ta",      0x0e00, 0x0200, CRC(27db5ede) SHA1(890587181497ed6e1d45ed501790a6d4d3315f00) )
	ROM_LOAD( "m79.10u",      0x1000, 0x0200, CRC(e41d13d2) SHA1(cc2911f46a0465305e4c7bc08f55acd065f93534) )
	ROM_LOAD( "m79.9u",       0x1200, 0x0200, CRC(e35f5616) SHA1(394ad92ad7dd233ece17335cf20aef8861b41508) )
	ROM_LOAD( "m79.8u",       0x1400, 0x0200, CRC(14eafd7c) SHA1(ca2d17f6ae1c3ff461a1b2bc6f37622e70cdaae8) )
	ROM_LOAD( "m79.7u",       0x1600, 0x0200, CRC(b9864f25) SHA1(9330cf96b7bce13e0ee3ad746b00e82ef10c3989) )
	ROM_LOAD( "m79.6u",       0x1800, 0x0200, CRC(dd25197f) SHA1(13eaf40251de82e817f488a9de738aadd8f6715e) )
	ROM_LOAD( "m79.5u",       0x1a00, 0x0200, CRC(251545e2) SHA1(05a0d5e8f143ea376fb3c517cf5e9d0d3534b933) )
	ROM_LOAD( "m79.4u",       0x1c00, 0x0200, CRC(b5f55c75) SHA1(f478cde73ae961be7b58c769f035eef58fd45555) )
	ROM_LOAD( "m79.3u",       0x1e00, 0x0200, CRC(e968691a) SHA1(7024d10f2af195fc4050861706b1f3d22cb22a9c) )
ROM_END



GAME( 1977, m79amb, 0, m79amb, m79amb, m79amb, ROT0, "Ramtek", "M-79 Ambush", GAME_IMPERFECT_SOUND )
