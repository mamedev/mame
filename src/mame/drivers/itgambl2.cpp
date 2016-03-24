// license:BSD-3-Clause
// copyright-holders:Roberto Fresca
/************************************************************************


  Italian Gambling games based on H8/3337 MCU + NEC D7759GC (sound).

  Written by Roberto Fresca.


  All these games use MCUs with internal ROM for their programs.

  They have 60KB of internal (normally flash) ROM that can't be dumped
  easily, and thus we can't emulate them at the moment because there is
  nothing to emulate.

  This driver is just a placeholder for the graphics/sound ROM loading


*************************************************************************

  --- Hardware Notes ---

  The hardware is normally composed by:


  CPU:   1x H8/3337 (HD64F3337CP16 or HD64F3337F16).
           (60KB ROM; 2KB RAM)

  Sound: 1x NEC D7759GC.
         1x TDA2003 (audio amplifier).

  PLDs:  2x ispLSI1032E-70JL.

  Clock: 1x Xtal 16.000 MHz.
         1x Xtal 14.318180 MHz.

  ROMs:  1x (up to) 27C2000 or similar (sound).
         3x or 4x 27C4001 or similar (graphics).

  Connectors: 1x 28x2 edge connector.
              1x RS232 connector.
              1x 14 legs connector.
              1x 34 legs connector (optional).

  Other: 1x battery.
         1x red led.
         2x 8 DIP switches.
         2x trimmer.


************************************************************************/

#define MAIN_CLOCK  XTAL_16MHz
#define SND_CLOCK   XTAL_14_31818MHz

#include "emu.h"
#include "cpu/h8/h83337.h"
#include "sound/upd7759.h"


class itgambl2_state : public driver_device
{
public:
	itgambl2_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_palette(*this, "palette")  { }

	int m_test_x;
	int m_test_y;
	int m_start_offs;
	virtual void machine_reset() override;
	virtual void video_start() override;
	DECLARE_PALETTE_INIT(itgambl2);
	UINT32 screen_update_itgambl2(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	required_device<cpu_device> m_maincpu;
	required_device<palette_device> m_palette;
};


/*************************
*     Video Hardware     *
*************************/

void itgambl2_state::video_start()
{
	m_test_x = 256;
	m_test_y = 256;
	m_start_offs = 0;
}

/* (dirty) debug code for looking 8bpps blitter-based gfxs */
UINT32 itgambl2_state::screen_update_itgambl2(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	int x,y,count;
	const UINT8 *blit_ram = memregion("gfx1")->base();

	if(machine().input().code_pressed(KEYCODE_Z))
		m_test_x++;

	if(machine().input().code_pressed(KEYCODE_X))
		m_test_x--;

	if(machine().input().code_pressed(KEYCODE_A))
		m_test_y++;

	if(machine().input().code_pressed(KEYCODE_S))
		m_test_y--;

	if(machine().input().code_pressed(KEYCODE_Q))
		m_start_offs+=0x200;

	if(machine().input().code_pressed(KEYCODE_W))
		m_start_offs-=0x200;

	if(machine().input().code_pressed(KEYCODE_E))
		m_start_offs++;

	if(machine().input().code_pressed(KEYCODE_R))
		m_start_offs--;

	popmessage("%d %d %04x",m_test_x,m_test_y,m_start_offs);

	bitmap.fill(m_palette->black_pen(), cliprect);

	count = (m_start_offs);

	for(y=0;y<m_test_y;y++)
	{
		for(x=0;x<m_test_x;x++)
		{
			UINT32 color;

			color = (blit_ram[count] & 0xff)>>0;

			if(cliprect.contains(x, y))
				bitmap.pix32(y, x) = m_palette->pen(color);

			count++;
		}
	}

	return 0;
}


/*************************
* Memory map information *
*************************/

static ADDRESS_MAP_START( itgambl2_map, AS_PROGRAM, 16, itgambl2_state )
	AM_RANGE(0x000000, 0x3fff) AM_ROM
ADDRESS_MAP_END


/*************************
*      Input ports       *
*************************/

static INPUT_PORTS_START( itgambl2 )
	PORT_START("IN0")
	PORT_DIPNAME( 0x0001, 0x0001, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0002, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0004, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0008, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0100, 0x0100, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0200, 0x0200, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0400, 0x0400, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0800, 0x0800, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x1000, 0x1000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x2000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000, 0x4000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000, 0x8000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
INPUT_PORTS_END


/*************************
*    Graphics Layouts    *
*************************/

static const gfx_layout gfxlayout_8x8x8 =
{
/* this is wrong and need to be fixed */

	8,8,
	RGN_FRAC(1,1),
	8,
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	{ 0*64, 1*64, 2*64, 3*64, 4*64, 5*64, 6*64, 7*64 },
	8*64
};


/******************************
* Graphics Decode Information *
******************************/

static GFXDECODE_START( itgambl2 )
	GFXDECODE_ENTRY( "gfx1", 0, gfxlayout_8x8x8,   0, 16  )
GFXDECODE_END


/**************************
*      Machine Reset      *
**************************/

void itgambl2_state::machine_reset()
{
	/* stop the CPU, we have no code for it anyway */
	m_maincpu->set_input_line(INPUT_LINE_HALT, ASSERT_LINE);
}

/* default 444 palette for debug purpose*/
PALETTE_INIT_MEMBER(itgambl2_state, itgambl2)
{
	int x,r,g,b;

	for(x=0;x<0x100;x++)
	{
		r = (x & 0xf)*0x10;
		g = ((x & 0x3c)>>2)*0x10;
		b = ((x & 0xf0)>>4)*0x10;
		palette.set_pen_color(x,rgb_t(r,g,b));
	}
}

/**************************
*     Machine Drivers     *
**************************/

static MACHINE_CONFIG_START( itgambl2, itgambl2_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", H83337, MAIN_CLOCK)
	MCFG_CPU_PROGRAM_MAP(itgambl2_map)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(512, 256)
	MCFG_SCREEN_VISIBLE_AREA(0, 512-1, 0, 256-1)
	MCFG_SCREEN_UPDATE_DRIVER(itgambl2_state, screen_update_itgambl2)


	MCFG_GFXDECODE_ADD("gfxdecode", "palette", itgambl2)
	MCFG_PALETTE_ADD("palette", 0x200)
	MCFG_PALETTE_INIT_OWNER(itgambl2_state, itgambl2)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("upd", UPD7759, UPD7759_STANDARD_CLOCK)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)
MACHINE_CONFIG_END


/*************************
*        Rom Load        *
*************************/

/* NtCash

CPU:

1x HD64F3337CP16 (main)
2x ispLSI1032E-70JL (PLD)
1x NEC D7759GC (sound)
1x TDA2003 (audio amp)
1x oscillator 14.318180 MHz
1x oscillator 16.000 MHz

ROMs:

1x M27C1001 (0)
4x M27C4001 (1, 2, 3, 4)

Note:

1x 28x2 edge connector
1x RS232 connector
1x 14 legs isp connector
1x battery
1x red led
2x 8x2 switches dip
2x trimmer

*/

ROM_START( ntcash )
	ROM_REGION( 0x1000000, "maincpu", 0 ) /* all the program code is in here */
	ROM_LOAD( "ntcash_hd64f3337cp16.mcu", 0x00000, 0x4000, NO_DUMP )

	ROM_REGION( 0x200000, "gfx1", 0 )
	ROM_LOAD( "ntcashep1.bin", 0x000000, 0x80000, CRC(f1e8b74d) SHA1(b84e36ab101d6b5b1f60d9778bd8e5d89b3d437d) )
	ROM_LOAD( "ntcashep2.bin", 0x080000, 0x80000, CRC(b51513c8) SHA1(27b6469daecb92d8a8ed6e9ab317d20f49dd6475) )
	ROM_LOAD( "ntcashep3.bin", 0x100000, 0x80000, CRC(ba46f1b2) SHA1(61f5b2f1732bbdb2bd21835d2c6e2890c1f0fc8c) )
	ROM_LOAD( "ntcashep4.bin", 0x180000, 0x80000, CRC(1e42142d) SHA1(82444584b1d61ff0a34d7767f70cab995d26e1e1) )

	ROM_REGION( 0x20000, "upd", 0 ) /* NEC D7759GC samples */
	ROM_LOAD( "ntcashmsg0.bin", 0x00000, 0x20000, CRC(e3022f30) SHA1(859bdf0ce871c0b39224dc93b8005a5e0a5552b1) )
ROM_END


/* Wizard (Ver 1.0)

CPU:

1x HD64F3337CP16 (main)
2x ispLSI1032E-70JL (PLD)
1x NEC D7759GC (sound)
1x TDA2003 (audio amp)
1x oscillator 14.318180 MHz
1x oscillator 16.000 MHz

ROMs:

1x M27C1001 (0)
4x M27C4001 (1, 2, 3, 4)

Note:

1x 28x2 edge connector
1x RS232 connector
1x 14 legs isp connector
1x battery
1x red led
2x 8x2 DIP switches
2x trimmer

*/

ROM_START( wizard )
	ROM_REGION( 0x1000000, "maincpu", 0 ) /* all the program code is in here */
	ROM_LOAD( "wizard_ver1.2_hd64f3337cp16.mcu", 0x00000, 0x4000, NO_DUMP )

	ROM_REGION( 0x200000, "gfx1", 0 )
	ROM_LOAD( "wizardep1.bin", 0x000000, 0x80000, CRC(a99af86f) SHA1(4bf32df74e93a6b40cf8213e99ec6ef538d9802d) )
	ROM_LOAD( "wizardep2.bin", 0x080000, 0x80000, CRC(bc52566d) SHA1(ecd4f3852c3ba8981316686042dfc2c0013f139f) )
	ROM_LOAD( "wizardep3.bin", 0x100000, 0x80000, CRC(98e1905a) SHA1(805df94fef011b48d5eb2abbd294b7cd338d7124) )
	ROM_LOAD( "wizardep4.bin", 0x180000, 0x80000, CRC(f129916a) SHA1(c1c0fcb04622dde196299c2e88a807b2aa00bf5e) )

	ROM_REGION( 0x20000, "upd", 0 ) /* NEC D7759GC samples */
	ROM_LOAD( "wizardmsg0.bin", 0x00000, 0x20000, CRC(94b28a4b) SHA1(2c10462cd7c8dc79dba735a061841a9c8b423091) )
ROM_END


/* Laser 2001 (Ver 1.2)

CPU:

1x HD64F3337CP16 (main)
2x ispLSI1032E-70JL (PLD)
1x NEC D7759GC (sound)
1x TDA2003 (audio amp)
1x oscillator 14.318180 MHz
1x oscillator 16.000 MHz

ROMs:

1x M27C1001 (0)
4x M27C4001 (1, 2, 3, 4)

Note:

1x 28x2 edge connector
1x RS232 connector
1x 14 legs isp connector
1x battery
1x red led
2x 8x2 switches dip
2x trimmer

*/

ROM_START( laser2k1 )
	ROM_REGION( 0x1000000, "maincpu", 0 ) /* all the program code is in here */
	ROM_LOAD( "laser2k1_ver1.2_hd64f3337cp16.mcu", 0x00000, 0x4000, NO_DUMP )

	ROM_REGION( 0x200000, "gfx1", 0 )
	ROM_LOAD( "xlep1.bin", 0x000000, 0x80000, CRC(b45c9491) SHA1(1fa0572d3efb847dcf49bb99f429322dcb72b0d1) )
	ROM_LOAD( "xlep2.bin", 0x080000, 0x80000, CRC(75c82293) SHA1(e6d847a2259393ef8877e9237c7624bf2e36f197) )
	ROM_LOAD( "xlep3.bin", 0x100000, 0x80000, CRC(3a45d626) SHA1(c804916b6bfe04bacd7ac6f32e5041ed65e7b91e) )
	ROM_LOAD( "xlep4.bin", 0x180000, 0x80000, CRC(d0381819) SHA1(30bab7e1c68192a2e1c324ef4c8a3d3b5696eb2b) )

	ROM_REGION( 0x20000, "upd", 0 ) /* NEC D7759GC samples */
	ROM_LOAD( "xlmsg0.bin", 0x00000, 0x20000, CRC(36287068) SHA1(d964837cb5370c7b878e1e531ef6d8c3840f776c) )
ROM_END


/* Magic Drink (Ver 1.2)

CPU:

1x HD64F3337CP16 (main)
2x ispLSI1032E-70JL (PLD)
1x NEC D7759GC (sound)
1x TDA2003 (audio amp)
1x oscillator 14.318180 MHz
1x oscillator 16.000 MHz

ROMs:

1x 27C2000 (s)
3x M27C4001 (1, 2, 3)

Note:

1x 28x2 edge connector
1x RS232 connector
1x 14 legs isp connector
1x battery
1x red led
2x 8x2 switches dip
2x trimmer

*/

ROM_START( mdrink )
	ROM_REGION( 0x1000000, "maincpu", 0 ) /* all the program code is in here */
	ROM_LOAD( "mdrink_ver1.2_hd64f3337cp16.mcu", 0x00000, 0x4000, NO_DUMP )

	ROM_REGION( 0x180000, "gfx1", 0 )
	ROM_LOAD( "mdrink-1.bin", 0x000000, 0x80000, CRC(25a7cea9) SHA1(d67a7264501699c8f7a48c3f3956903a5c95898f) )
	ROM_LOAD( "mdrink-2.bin", 0x080000, 0x80000, CRC(c2a14bca) SHA1(8d0095333c34d81d103f15ee5731e2e4aa4d1fac) )
	ROM_LOAD( "mdrink-3.bin", 0x100000, 0x80000, CRC(ff593676) SHA1(b21bb85df0b7b79c07ded2c4b950c94719e08302) )

	ROM_REGION( 0x40000, "upd", 0 ) /* NEC D7759GC samples */
	ROM_LOAD( "mdrink-s.bin", 0x00000, 0x40000, CRC(d78b7823) SHA1(ca01e4aa3e25c3a40517b4fe07c31915e79af650) )
ROM_END


/* Unknown... (Ver 1.2)

CPU

1x H8/3337-HDY1A3-64F3337F16 (main)
1x NEC D7759GC-0124XY007 (sound)
2x ispLSI1032E-70LJ-C110AA02 (main)
1x oscillator 14.318180 MHz
1x oscillator 16.000 MHz

ROMs

1x M27C1001 (0)
1x 27C4000 (1)
3x M27C4001 (2, 3, 4)

Note

1x 28x2 edge connector (not JAMMA)
1x RS232 connector
1x 14 legs connector
1x 34 legs connector
2x 8 DIP switches
1x battery
1x trimmer (volume)
1x trimmer (unknown)

--------------------------

Silkscreened on PCB:
"CE Angelo Arena - Via Vighi, 26  40026 - Imola (BO)"

PCB n. 2-0276 TE04.01

Formely named "videopoker1"

*/

ROM_START( te0144 )
	ROM_REGION( 0x1000000, "maincpu", 0 ) /* all the program code is in here */
	ROM_LOAD( "te0144_ver1.2_hdy1a3-64f3337f16.mcu", 0x00000, 0x4000, NO_DUMP )

	ROM_REGION( 0x200000, "gfx1", 0 )
	ROM_LOAD( "pb1.bin", 0x000000, 0x80000, CRC(b7b4ea0f) SHA1(d11096684059e6063747f3e082d70aef1ee8d259) )
	ROM_LOAD( "pb2.bin", 0x080000, 0x80000, CRC(b02fd07e) SHA1(415a834cd47fdcb180b2a5fa267c1566b9ca0b61) )
	ROM_LOAD( "pb3.bin", 0x100000, 0x80000, CRC(1984427e) SHA1(0200360f083019235f464ed9b96bf7f78a07df37) )
	ROM_LOAD( "pb4.bin", 0x180000, 0x80000, CRC(ac513c2d) SHA1(aedc29b12157f02a014359ceae71a2a7892afa72) )

	ROM_REGION( 0x20000, "upd", 0 ) /* NEC D7759GC samples */
	ROM_LOAD( "pb0.bin", 0x00000, 0x20000, CRC(123ef964) SHA1(b36d91b58119c15211a54ff7d78c7137d638ea88) )
ROM_END


/* Carta Magica (Ver 1.8)

CPU:

1x H8/3337-HD64F3337CP16 (main)
1x NEC D7759GC (sound)
1x TDA2003 (sound)
2x ispLSI1032E-70LJ-E011J02
1x oscillator 14.318180 MHz
1x oscillator 16.000 MHz

ROMs:

1x 27C1001 or similar (0)
3x 27C4001 or similar (1, 2, 3)

Note:

1x 28x2 edge connector (not JAMMA)
1x RS232 connector
1x 14 legs connector
2x 8 DIP switches
1x battery
1x trimmer (volume)
1x trimmer (unknown)

--------------------------

Silkscreened on PCB:
"SMS distribuzione"
"Base 2 Synth Rev.1"
"APM1"

*/

ROM_START( cmagica )
	ROM_REGION( 0x1000000, "maincpu", 0 ) /* all the program code is in here */
	ROM_LOAD( "cmagica_ver1.8_hd64f3337cp16.mcu", 0x00000, 0x4000, NO_DUMP )

	ROM_REGION( 0x180000, "gfx1", 0 )
	ROM_LOAD( "1.u6", 0x000000, 0x80000, CRC(3e7e6c9f) SHA1(53a7c4422d9a7c63a21cf4d35d4d883dc2d0eac0) )
	ROM_LOAD( "2.u7", 0x080000, 0x80000, CRC(6339b62d) SHA1(160030e07600c8db365429c27a33081cfa7d3d61) )
	ROM_LOAD( "3.u4", 0x100000, 0x80000, CRC(ba636099) SHA1(3d3d9eee5d6808d7666dbf113d7c17a03b6b461e) )

	ROM_REGION( 0x20000, "upd", 0 ) /* NEC D7759GC samples */
	ROM_LOAD( "sound.bin", 0x00000, 0x20000, CRC(9dab99a6) SHA1(ce34056dd964be32359acd2e53a6101cb4d9ddff) )
ROM_END


/* Millennium Sun

CPU:

1x H8/3337-HD64F3337F16 (main)
1x maybe NEC D7759GC (sound)
1x TDA2003 (audio amp)
2x ispLSI1032E-70LJ
1x oscillator 14.318180 MHz
1x oscillator 16.000 MHz

ROMs:

1x 27C1001 or similar (msg0)
4x 27C4001 or similar (ep1, ep2, ep3)

Note:

1x 28x2 edge connector (not JAMMA)
1x RS232 connector
1x 14 legs connector
1x 34 legs connector
2x 8 DIP switches
1x battery
1x trimmer (volume)
1x trimmer (unknown)

--------------------------

Silkscreened on PCB:
"BV 2-0257/A"

*/

ROM_START( millsun )
	ROM_REGION( 0x1000000, "maincpu", 0 ) /* all the program code is in here */
	ROM_LOAD( "millsun_hd64f3337f16.mcu", 0x00000, 0x4000, NO_DUMP )

	ROM_REGION( 0x200000, "gfx1", 0 )
	ROM_LOAD( "msun_ep1.bin", 0x000000, 0x80000, CRC(06f10795) SHA1(f88a36e11f8ba38439aa066dc013427f204be3d7) )
	ROM_LOAD( "msun_ep2.bin", 0x080000, 0x80000, CRC(f85d10e6) SHA1(d33017c4a4883a4c9c76132deb5c57eb38f9fdb3) )
	ROM_LOAD( "msun_ep3.bin", 0x100000, 0x80000, CRC(329d380c) SHA1(618a7010fca8be6c368c3cc09fe129d8a4c72087) )
	ROM_LOAD( "msun_ep4.bin", 0x180000, 0x80000, CRC(071f5257) SHA1(891116086f5ce99327d9752c99465c25bd6dd69e) )

	ROM_REGION( 0x20000, "upd", 0 ) /* NEC D7759GC samples */
	ROM_LOAD( "msun_msg0.bin", 0x00000, 0x20000, CRC(b4bfbbb9) SHA1(ba2d6555f169273fa43de320614a5ea3ba2857e8) )
ROM_END


/* Super Space 2001

CPU:

1x H8/3337-HD64F3337F16 (main)
1x maybe NEC D7759GC (sound)
1x TDA2003 (audio amp)
2x ispLSI1032E-70LJ
1x oscillator 14.318180 MHz
1x oscillator 16.000 MHz

ROMs:

1x 27C1001 or similar (msg0)
4x 27C4001 or similar (ep1, ep2, ep3)

Note:

1x 28x2 edge connector (not JAMMA)
1x RS232 connector
1x 14 legs connector
1x 34 legs connector
2x 8 DIP switches
1x battery
1x trimmer (volume)
1x trimmer (unknown)

--------------------------

Silkscreened on PCB:
"BV 2-0257/A"

*/

ROM_START( sspac2k1 )
	ROM_REGION( 0x1000000, "maincpu", 0 ) /* all the program code is in here */
	ROM_LOAD( "sspac2k1_hd64f3337f16.mcu", 0x00000, 0x4000, NO_DUMP )

	ROM_REGION( 0x200000, "gfx1", 0 )
	ROM_LOAD( "sup_spaces_ep1.bin", 0x000000, 0x80000, CRC(d512ee80) SHA1(f113218899394bf1dfe81518746414c4eda9a94c) )
	ROM_LOAD( "sup_spaces_ep2.bin", 0x080000, 0x80000, CRC(775eb938) SHA1(a83851ea6d90aaf3cad064cdbcc8379eed3d90ca) )
	ROM_LOAD( "sup_spaces_ep3.bin", 0x100000, 0x80000, CRC(d1d9c06c) SHA1(64993b5572201cc2c29d8900a89f036e96221e05) )
	ROM_LOAD( "sup_spaces_ep4.bin", 0x180000, 0x80000, CRC(0c02ad49) SHA1(64b382bf6dabf08229324807c6b66e600f38039d) )

	ROM_REGION( 0x20000, "upd", 0 ) /* NEC D7759GC samples */
	ROM_LOAD( "sup_spaces_msg0.bin", 0x00000, 0x20000, CRC(93edd0ad) SHA1(f122e147c918c6cb12043008ede729d6e0a4e543) )
ROM_END


/* Elvis?

CPU:

1x H8/3337-HD64F3337CP16
1x NEC D7759GC (sound)
1x TDA2003 (audio amp)
2x ispLSI1032E-70LJ
1x oscillator 14.318180 MHz
1x oscillator 16.000 MHz

ROMs:

1x 27C1001 or similar (0)
3x 27C4001 or similar (1, 2, 3)

Note:

1x 28x2 edge connector (not JAMMA)
1x RS232 connector
1x 14 legs connector
2x 8 DIP switches
1x battery
1x trimmer (volume)
1x trimmer (unknown)

--------------------------

Silkscreened on PCB:
"2-0250"

*/

ROM_START( elvis )
	ROM_REGION( 0x1000000, "maincpu", 0 ) /* all the program code is in here */
	ROM_LOAD( "elvis_hd64f3337cp16.mcu", 0x00000, 0x4000, NO_DUMP )

	ROM_REGION( 0x180000, "gfx1", 0 )
	ROM_LOAD( "1.bin", 0x000000, 0x80000, CRC(9e15983f) SHA1(272673ac9685cf0f5cc8a9263c91e4f93c30197f) )
	ROM_LOAD( "2.bin", 0x080000, 0x80000, CRC(c420af73) SHA1(fb0e03456a4b2f18c35d5ee2efeb29e3f2f26eae) )
	ROM_LOAD( "3.bin", 0x100000, 0x80000, CRC(bc10b1b6) SHA1(ef25f974cd0b44b91a8db215ff8d2dd3f4313bd8) )

	ROM_REGION( 0x20000, "upd", 0 ) /* NEC D7759GC samples */
	ROM_LOAD( "0.bin", 0x00000, 0x20000, CRC(833c5be5) SHA1(89110cb52265ee5bfdf73c0af343b7ce2356e394) )
ROM_END

/* Triple Star 2000

CPU:
1x HD64F3337YCP16 (main)(not dumped)
2x NEC D7759GC (speech synthesizer)
1x TDA2003 (sound)
1x oscillator 16.000000
1x oscillator 14.318180

ROMs:
1x M27C1001 (0)
4x M27C4001 (1,2,3,4)


RAMs
3x TC551001

PLDs
2x ispLSI10032E-70LJ
Note    1x 28x2 edge connector
1x 14 legs connector
1x RS232 connector
1x trimmer (volume)
1x trimmer (unknown)
1x red LED
2x 8x2 switches DIP
1x battery 3.6V
*/

ROM_START( trstar2k )
	ROM_REGION( 0x1000000, "maincpu", 0 ) /* all the program code is in here */
	ROM_LOAD( "trstar2k_hd64f3337ycp16.mcu", 0x00000, 0x4000, NO_DUMP )

	ROM_REGION( 0x200000, "gfx1", 0 )
	ROM_LOAD( "ep1.bin", 0x000000, 0x80000, CRC(59394c87) SHA1(a8b5de197b474714a8e5a5c959b81cb78fc69291) )
	ROM_LOAD( "ep2.bin", 0x080000, 0x80000, CRC(80608870) SHA1(5af501e4bb9498d2b9b614cc98ec9f4c907f207d) )
	ROM_LOAD( "ep3.bin", 0x100000, 0x80000, CRC(cad4523f) SHA1(1a16f200622a8c9666beea2da2ec64bf7c9195a8) )
	ROM_LOAD( "ep4.bin", 0x180000, 0x80000, CRC(e488d31d) SHA1(e0a51abf1459a1c7205750b9cad28a63bbabed96) )

	ROM_REGION( 0x20000, "upd", 0 ) /* NEC D7759GC samples */
	ROM_LOAD( "msg0.bin", 0x00000, 0x20000, CRC(b25e1c8a) SHA1(a211412c3354a9f1a9662445b4cc379dad27813b) )
ROM_END

/* Super Star

(no readme)

*/

ROM_START( sstar )
	ROM_REGION( 0x1000000, "maincpu", 0 ) /* all the program code is in here */
	ROM_LOAD( "sstar_hd64f3337cp16.mcu", 0x00000, 0x4000, NO_DUMP )

	ROM_REGION( 0x180000, "gfx1", 0 )
	ROM_LOAD( "sstar.ep1", 0x000000, 0x80000, CRC(e798295e) SHA1(ed9a0ceeaefccfb1bde5894548ba91d631055b69) ) //contains C-based strings?
	ROM_LOAD( "sstar.ep2", 0x080000, 0x80000, CRC(5e9fa33b) SHA1(5196723db69bf9f1df497f4d8f84ac1d9768736f) )
	ROM_LOAD( "sstar.ep3", 0x100000, 0x80000, CRC(67abc2a1) SHA1(877e233b2120281779a2480e8035a73df87e7240) )

	ROM_REGION( 0x20000, "upd", 0 ) /* NEC D7759GC samples */
	ROM_LOAD( "sstar.msg0", 0x00000, 0x20000, CRC(04f44a53) SHA1(0b27d1fe1992d1769abec2078defc30896c36bcb) )
ROM_END

/* Pirati

H8/337 HD64F3337CP16
16.000 MHz
14.31818 MHz
D7759GC

*/

ROM_START( pirati )
	ROM_REGION( 0x1000000, "maincpu", 0 ) /* all the program code is in here */
	ROM_LOAD( "pirati_hd64f3337cp16.mcu", 0x00000, 0x4000, NO_DUMP )

	ROM_REGION( 0x180000, "gfx1", 0 )
	ROM_LOAD( "s.ch.ep1", 0x000000, 0x80000, CRC(735d28a6) SHA1(feaf71b64db45e9dd68bff4daf75d3bd5e6ab6c8) )
	ROM_LOAD( "s.ch.ep2", 0x080000, 0x80000, CRC(35b75de6) SHA1(95c85c505ed0f3ddcc8c5d0d9e19128515840282) )
	ROM_LOAD( "s.ch.ep3", 0x100000, 0x80000, CRC(faff2daa) SHA1(ea7376d20d5ddd50c7a4b0c86d7998bcc0fa1598) )

	ROM_REGION( 0x20000, "upd", 0 ) /* NEC D7759GC samples */
	ROM_LOAD( "m.s.g.0", 0x00000, 0x20000, CRC(01ed1dcd) SHA1(69b0b4ff2633ca4ca7b3a01830582c8b6df059e8) )
ROM_END

/*

Magic Number

CPUs
1x   H8/3337             32-bit Single-Chip Microcomputer - main (internal ROM not dumped)
1x  D7759                ADPCM Speech Synthesizer LSIs - sound
1x  TDA2003              Audio Amplifier - sound
1x  oscillator  14.31818MHz
1x  oscillator  16.000
ROMs
3x  M27C4001    1,2,3   dumped
1x  M27C1001    MSG     dumped
RAMs
3x  CXK581000BM-70LL
PLDs
2x  ispLSI1032E-70Lj        not dumped
Others

1x 28x2 edge connector
1x 7x2 legs ISP connector
1x RS232 connector
2x trimmer
2x 8x2 switches DIP
1x jumper
1x red LED
1x battery 3.6V
*/

ROM_START( mnumitg )
	ROM_REGION( 0x1000000, "maincpu", 0 ) /* all the program code is in here */
	ROM_LOAD( "mnum_hd64f3337cp16.mcu", 0x00000, 0x4000, NO_DUMP )

	ROM_REGION( 0x180000, "gfx1", 0 )
	ROM_LOAD( "mnum-ep1.bin", 0x000000, 0x80000, CRC(ee80b8d6) SHA1(49dd3323f4369759c38c168d172f7716a9132f98) )
	ROM_LOAD( "mnum-ep2.bin", 0x080000, 0x80000, CRC(685cb1cf) SHA1(7815ec3dcbf2c78f85520e533d9cbf51a119255d) )
	ROM_LOAD( "mnum-ep3.bin", 0x100000, 0x80000, CRC(ebebd71c) SHA1(98902e43c69d207aa7dbca23d10bbeb81272292f) )

	ROM_REGION( 0x20000, "upd", 0 ) /* NEC D7759GC samples */
	ROM_LOAD( "mnum-msg0.bin", 0x00000, 0x20000, CRC(b25e1c8a) SHA1(a211412c3354a9f1a9662445b4cc379dad27813b) )
ROM_END

/*

CPUs
1x   H8/3337            32-bit Single-Chip Microcomputer - main (internal ROM not dumped)
1x  D7759               ADPCM Speech Synthesizer LSIs - sound
1x  TDA2003             Audio Amplifier - sound
1x  oscillator  14.31818MHz
1x  oscillator  16.000
ROMs
3x  M27C4001    1,2,3   dumped
1x  M27C1001    SND     dumped
RAMs
3x  MX66C1024MC-70
PLDs
2x  ispLSI1032E-70Lj    not dumped
Others

1x 28x2 edge connector
1x 17x2 legs ISP connector
1x 7x2 legs ISP connector
1x 10 legs connector
1x 4 legs connector
1x RS232 connector
2x trimmer (unknown)
2x 8x2 switches DIP
1x jumper
1x battery 3.6V

*/

ROM_START( mclass )
	ROM_REGION( 0x1000000, "maincpu", 0 ) /* all the program code is in here */
	ROM_LOAD( "mclass_hd64f3337cp16.mcu", 0x00000, 0x4000, NO_DUMP )

	ROM_REGION( 0x180000, "gfx1", 0 )
	ROM_LOAD( "magicclass1.bin", 0x000000, 0x80000, CRC(12927480) SHA1(380e980cf5d869fbcba224d75c7eaee650465227) )
	ROM_LOAD( "magicclass2.bin", 0x080000, 0x80000, CRC(b472dda6) SHA1(e23202157dfa6f1f76f9dc410ef7e1f12b5031bf) )
	ROM_LOAD( "magicclass3.bin", 0x100000, 0x80000, CRC(b1bc38e4) SHA1(7dfa352535baae7d048ef4537f2d9ac72c46dedc) )

	ROM_REGION( 0x20000, "upd", 0 ) /* NEC D7759GC samples */
	ROM_LOAD( "magicclasssnd.bin", 0x00000, 0x20000, CRC(9dab99a6) SHA1(ce34056dd964be32359acd2e53a6101cb4d9ddff) )
ROM_END

/*

CPUs
1x  H8/3337             32-bit Single-Chip Microcomputer - main (internal ROM not dumped)
1x  D7759               ADPCM Speech Synthesizer LSIs - sound
1x  TDA2003             Audio Amplifier - sound
1x  oscillator  14.31818MHz
1x  oscillator  16.000
ROMs
3x  M27C4001    1,2,3   dumped
1x  M27C1001    SND     dumped
RAMs
3x  V62C5181024L-35W
PLDs
2x  ispLSI1032E-70Lj    not dumped
Others

1x 28x2 edge connector
1x 7x2 legs ISP connector
1x RS232 connector
1x trimmer (unknown)
1x trimmer (volume)
2x 8x2 switches DIP
1x jumper
1x red LED
1x battery 3.6V

*/

ROM_START( europass )
	ROM_REGION( 0x1000000, "maincpu", 0 ) /* all the program code is in here */
	ROM_LOAD( "europass_hd64f3337cp16.mcu", 0x00000, 0x4000, NO_DUMP )

	ROM_REGION( 0x180000, "gfx1", 0 )
	ROM_LOAD( "europass1.bin", 0x000000, 0x80000, CRC(93c54f02) SHA1(b0371c70363b6b2097dc478320bdae0856211d2e) )
	ROM_LOAD( "europass2.bin", 0x080000, 0x80000, CRC(62bcb3de) SHA1(fc35f534635340f5ae22ae838bc10605ae0b7a4b) )
	ROM_LOAD( "europass3.bin", 0x100000, 0x80000, CRC(0f2b2c21) SHA1(c4706585e4176e4a5f5ce40046e6e14b93952816) )

	ROM_REGION( 0x20000, "upd", 0 ) /* NEC D7759GC samples */
	ROM_LOAD( "xninesnd.bin", 0x00000, 0x20000, CRC(9dab99a6) SHA1(ce34056dd964be32359acd2e53a6101cb4d9ddff) )
ROM_END

ROM_START( thedrink )
	ROM_REGION( 0x1000000, "maincpu", 0 ) /* all the program code is in here */
	ROM_LOAD( "thedrink_hd64f3337cp16.mcu", 0x00000, 0x4000, NO_DUMP )

	ROM_REGION( 0x200000, "gfx1", 0 )
	ROM_LOAD( "the-drink-ep1.bin", 0x000000, 0x80000, CRC(9d0f61ad) SHA1(8e45afdd7e31a830b62b3dc85e21e9bc024262ff) )
	ROM_LOAD( "the-drink-ep2.bin", 0x080000, 0x80000, CRC(b21f349a) SHA1(c46b95af869a648a17e0dd8f0eb82d5e347289ab) )
	ROM_LOAD( "the-drink-ep3.bin", 0x100000, 0x80000, CRC(bb1af614) SHA1(77496efc361b6fd2b4bac0304032ea44e47d1819) )
	ROM_LOAD( "the-drink-ep4.bin", 0x180000, 0x80000, CRC(f02a6387) SHA1(59d2a20d4fa9c78ef49c9afe82939c0a882012cc) )

	ROM_REGION( 0x20000, "upd", 0 ) /* NEC D7759GC samples */
	ROM_LOAD( "the-drink-msg0.bin", 0x00000, 0x20000, CRC(5eefd405) SHA1(7b1d91181f5078c55cfa623d7e8fc5b4ebfff110) )
ROM_END

ROM_START( unkh8gam )
	ROM_REGION( 0x1000000, "maincpu", 0 ) /* all the program code is in here */
	ROM_LOAD( "unkh8gam.mcu", 0x00000, 0x4000, NO_DUMP )

	ROM_REGION( 0x180000, "gfx1", 0 )
	ROM_LOAD( "24.bin", 0x000000, 0x80000, CRC(240eb4bf) SHA1(f79a735ed290f84a44411127af3e16a514b62b6b) )
	ROM_LOAD( "25.bin", 0x080000, 0x80000, CRC(c273c0ce) SHA1(8774cb101fccf1d97d66816df56dec8fa4e24ee6) )
	ROM_LOAD( "26.bin", 0x100000, 0x80000, CRC(0293b9bb) SHA1(00794fab4d9deb2ca5ce352ac7ed7aedb59bec7b) )

	ROM_REGION( 0x20000, "upd", 0 ) /* NEC D7759GC samples */
	ROM_LOAD( "30.bin", 0x00000, 0x20000, CRC(72e56518) SHA1(7afdd6434beeea22673228c2417e4dee253a42b5) )
ROM_END


/*************************
*      Game Drivers      *
*************************/

/*    YEAR  NAME      PARENT  MACHINE   INPUT     STATE          INIT ROT    COMPANY      FULLNAME                                FLAGS  */
GAME( 1999, ntcash,   0,      itgambl2, itgambl2, driver_device, 0,   ROT0, "<unknown>", "NtCash",                                MACHINE_IS_SKELETON )
GAME( 1999, wizard,   0,      itgambl2, itgambl2, driver_device, 0,   ROT0, "A.A.",      "Wizard (Ver 1.0)",                      MACHINE_IS_SKELETON )
GAME( 200?, trstar2k, 0,      itgambl2, itgambl2, driver_device, 0,   ROT0, "A.M.",      "Triple Star 2000",                      MACHINE_IS_SKELETON )
GAME( 2001, laser2k1, 0,      itgambl2, itgambl2, driver_device, 0,   ROT0, "<unknown>", "Laser 2001 (Ver 1.2)",                  MACHINE_IS_SKELETON )
GAME( 2001, mdrink,   0,      itgambl2, itgambl2, driver_device, 0,   ROT0, "<unknown>", "Magic Drink (Ver 1.2)",                 MACHINE_IS_SKELETON )
GAME( 2001, te0144,   0,      itgambl2, itgambl2, driver_device, 0,   ROT0, "<unknown>", "Puzzle Bobble (Italian Gambling Game)", MACHINE_IS_SKELETON )
GAME( 200?, cmagica,  0,      itgambl2, itgambl2, driver_device, 0,   ROT0, "<unknown>", "Carta Magica (Ver 1.8)",                MACHINE_IS_SKELETON )
GAME( 200?, millsun,  0,      itgambl2, itgambl2, driver_device, 0,   ROT0, "<unknown>", "Millennium Sun",                        MACHINE_IS_SKELETON )
GAME( 200?, sspac2k1, 0,      itgambl2, itgambl2, driver_device, 0,   ROT0, "<unknown>", "Super Space 2001",                      MACHINE_IS_SKELETON )
GAME( 200?, elvis,    0,      itgambl2, itgambl2, driver_device, 0,   ROT0, "<unknown>", "Elvis?",                                MACHINE_IS_SKELETON )
GAME( 200?, sstar,    0,      itgambl2, itgambl2, driver_device, 0,   ROT0, "<unknown>", "Super Star",                            MACHINE_IS_SKELETON )
GAME( 2001, pirati,   0,      itgambl2, itgambl2, driver_device, 0,   ROT0, "Cin",       "Pirati",                                MACHINE_IS_SKELETON )
GAME( 200?, mnumitg,  0,      itgambl2, itgambl2, driver_device, 0,   ROT0, "<unknown>", "Magic Number (Italian Gambling Game, Ver 1.5)", MACHINE_IS_SKELETON )
GAME( 200?, mclass,   0,      itgambl2, itgambl2, driver_device, 0,   ROT0, "<unknown>", "Magic Class (Ver 2.2)",                 MACHINE_IS_SKELETON )
GAME( 200?, europass, 0,      itgambl2, itgambl2, driver_device, 0,   ROT0, "<unknown>", "Euro Pass (Ver 1.1)",                   MACHINE_IS_SKELETON )
GAME( 200?, thedrink, 0,      itgambl2, itgambl2, driver_device, 0,   ROT0, "<unknown>", "The Drink",                             MACHINE_IS_SKELETON )
GAME( 200?, unkh8gam, 0,      itgambl2, itgambl2, driver_device, 0,   ROT0, "<unknown>", "unknown H8 Italian Gambling game",      MACHINE_IS_SKELETON )
