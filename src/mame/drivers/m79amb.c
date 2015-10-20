// license:BSD-3-Clause
// copyright-holders:Al Kossow
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


The cabinet has backdrop artwork, a lightbulb for explosions,
and two large (paddles pretending to be) guns.

*/

#include "emu.h"
#include "includes/m79amb.h"
#include "cpu/i8085/i8085.h"

WRITE8_MEMBER(m79amb_state::ramtek_videoram_w)
{
	m_videoram[offset] = data & ~*m_mask;
}

UINT32 m79amb_state::screen_update_ramtek(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	offs_t offs;

	for (offs = 0; offs < 0x2000; offs++)
	{
		int i;

		UINT8 data = m_videoram[offs];
		int y = offs >> 5;
		int x = (offs & 0x1f) << 3;

		for (i = 0; i < 8; i++)
		{
			pen_t pen = (data & 0x80) ? rgb_t::white : rgb_t::black;
			bitmap.pix32(y, x) = pen;

			x++;
			data <<= 1;
		}
	}

	return 0;
}


READ8_MEMBER(m79amb_state::gray5bit_controller0_r)
{
	UINT8 port_data = ioport("8004")->read();
	UINT8 gun_pos = ioport("GUN1")->read();

	return (port_data & 0xe0) | m_lut_gun1[gun_pos];
}

READ8_MEMBER(m79amb_state::gray5bit_controller1_r)
{
	UINT8 port_data = ioport("8005")->read();
	UINT8 gun_pos = ioport("GUN2")->read();

	return (port_data & 0xe0) | m_lut_gun2[gun_pos];
}

WRITE8_MEMBER(m79amb_state::m79amb_8002_w)
{
	/* D1 may also be watchdog reset */
	/* port goes to 0x7f to turn on explosion lamp */
	output_set_value("EXP_LAMP", data ? 1 : 0);
}

static ADDRESS_MAP_START( main_map, AS_PROGRAM, 8, m79amb_state )
	AM_RANGE(0x0000, 0x1fff) AM_ROM
	AM_RANGE(0x4000, 0x5fff) AM_RAM_WRITE(ramtek_videoram_w) AM_SHARE("videoram")
	AM_RANGE(0x6000, 0x63ff) AM_RAM                 /* ?? */
	AM_RANGE(0x8000, 0x8000) AM_READ_PORT("8000") AM_WRITE(m79amb_8000_w)
	AM_RANGE(0x8001, 0x8001) AM_WRITEONLY AM_SHARE("mask")
	AM_RANGE(0x8002, 0x8002) AM_READ_PORT("8002") AM_WRITE(m79amb_8002_w)
	AM_RANGE(0x8003, 0x8003) AM_WRITE(m79amb_8003_w)
	AM_RANGE(0x8004, 0x8004) AM_READ(gray5bit_controller0_r)
	AM_RANGE(0x8005, 0x8005) AM_READ(gray5bit_controller1_r)
	AM_RANGE(0xc000, 0xc07f) AM_RAM                 /* ?? */
	AM_RANGE(0xc200, 0xc27f) AM_RAM                 /* ?? */
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
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_VBLANK("screen")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_TILT )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("8004")
	PORT_BIT( 0x1f, IP_ACTIVE_LOW, IPT_UNUSED ) // gun 1 here
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("8005")
	PORT_BIT( 0x1f, IP_ACTIVE_LOW, IPT_UNUSED ) // gun 2 here
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	// fake ports for the guns
	PORT_START("GUN1")
	PORT_BIT( 0xff, 0x80, IPT_PADDLE ) PORT_CROSSHAIR(X, (1.0-(19.0/256.0)), 19.0/256.0, 30.0/224.0) PORT_MINMAX(19,255) PORT_SENSITIVITY(50) PORT_KEYDELTA(5) PORT_CENTERDELTA(0) PORT_PLAYER(1)

	PORT_START("GUN2")
	PORT_BIT( 0xff, 0x80, IPT_PADDLE ) PORT_CROSSHAIR(X, (1.0-(22.0/256.0)), 0.0, 30.0/224.0) PORT_MINMAX(0,234) PORT_SENSITIVITY(50) PORT_KEYDELTA(5) PORT_CENTERDELTA(0) PORT_PLAYER(2)
INPUT_PORTS_END


INTERRUPT_GEN_MEMBER(m79amb_state::m79amb_interrupt)
{
	device.execute().set_input_line_and_vector(0, HOLD_LINE, 0xcf);  /* RST 08h */
}

static MACHINE_CONFIG_START( m79amb, m79amb_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", I8080, XTAL_19_6608MHz / 10)
	MCFG_CPU_PROGRAM_MAP(main_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", m79amb_state,  m79amb_interrupt)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500) /* not accurate */)
	MCFG_SCREEN_SIZE(32*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 4*8, 32*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(m79amb_state, screen_update_ramtek)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("discrete", DISCRETE, 0)
	MCFG_DISCRETE_INTF(m79amb)

	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_CONFIG_END



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



/* grenade trajectory per gun position is inconsistent and sloppy in the game:
     0,     1,     3,     2,     6,     7,     5,     4     - gun position
    90.00, 90.00, 90.00, 90.00, 86.42, 86.42, 86.42, 86.42  - grenade trajectory (angle, est)
     18.0,  18.0,  18.0,  18.0,  27.2,  27.2,  27.2,  31.2  - crosses with y=28 (x, est)

    12,    13,    15,    14,    10,    11,     9,     8,
    84.39, 84.39, 84.39, 80.87, 79.00, 80.87, 79.00, 79.00
     41.9,  48.9,  56.8,  75.8,  87.2,  88.8, 101.6, 107.6

    24,    25,    27,    26,    30,    31,    29,    28,
    79.00, 79.00, 75.59, 75.59, 75.59, 73.72, 73.72, 73.72
    114.1, 121.5, 138.8, 146.0, 152.7, 162.6, 167.6, 172.7

    20,    21,    23,    22,    18,    19,    17,    16
    73.72, 70.08, 70.08, 70.08, 67.97, 67.97, 64.34, 64.34
    181.6, 199.9, 205.4, 211.9, 223.5, 232.4, 254.0, 254.0
*/

static const UINT8 lut_cross[0x20] = {
		19,    20,    21,    23,    25,    27,    29,    37,
		45,    53,    66,    82,    88,    95,   105,   111,
	118,   130,   142,   149,   158,   165,   170,   177,
	191,   203,   209,   218,   228,   243,   249,   255,
};

static const UINT8 lut_pos[0x20] = {
	0x1f,  0x1e,  0x1c,  0x1d,  0x19,  0x18,  0x1a,  0x1b,
	0x13,  0x12,  0x10,  0x11,  0x15,  0x14,  0x16,  0x17,
	0x07,  0x06,  0x04,  0x05,  0x01,  0x00,  0x02,  0x03,
	0x0b,  0x0a,  0x08,  0x09,  0x0d,  0x0c,  0x0e,  0x0f
};


DRIVER_INIT_MEMBER(m79amb_state,m79amb)
{
	UINT8 *rom = memregion("maincpu")->base();
	int i, j;

	/* PROM data is active low */
	for (i = 0; i < 0x2000; i++)
		rom[i] = ~rom[i];

	/* gun positions */
	for (i = 0; i < 0x100; i++)
	{
		/* gun 1, start at left 18 */
		for (j = 0; j < 0x20; j++)
		{
			if (i <= lut_cross[j])
			{
				m_lut_gun1[i] = lut_pos[j];
				break;
			}
		}

		/* gun 2, start at right 235 */
		for (j = 0; j < 0x20; j++)
		{
			if (i >= (253 - lut_cross[j]))
			{
				m_lut_gun2[i] = lut_pos[j];
				break;
			}
		}
	}
}

GAME( 1977, m79amb, 0, m79amb, m79amb, m79amb_state, m79amb, ROT0, "Ramtek", "M-79 Ambush", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
