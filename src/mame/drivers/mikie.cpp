// license:BSD-3-Clause
// copyright-holders:Allard van der Bas
/***************************************************************************

    Mikie memory map (preliminary)

    driver by Allard van der Bas


    MAIN BOARD:
    2800-288f Sprite RAM (288f, not 287f - quite unusual)
    3800-3bff Color RAM
    3c00-3fff Video RAM
    4000-5fff ROM (?)
    5ff0      Watchdog (?)
    6000-ffff ROM


Stephh's notes (based on the games M6809 code and some tests) :

  - To enter service mode, keep START1 and START2 pressed on reset.
    Then press START1 to cycle through the different tests.
  - According to code at 0x618f, you can start a game with 255 lives
    if you set DSW1 and DSW2 to the following settings :
      * "Coin A"      : "Free Play"
      * "Coin B"      : "No Coin B"
      * "Lives"       : "7"
      * "Cabinet"     : "Upright"
      * "Bonus Life"  : "20k 70k 50k+"
      * "Difficulty"  : "Medium"
      * "Demo Sounds" : "Off"
    DSW3 is not tested here, so settings can be anything.
  - I'm very surprised to notice that 'mikie' and 'mikiej' have the same
    PRG ROMS but different GFX ROMS.
    This is very rare (unique ?) for the Konami games.
  - There might exist undumped Konami/Centuri version(s) of this game :
    the manual I've found speaks about a "conversion kit".

***************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "cpu/m6809/m6809.h"
#include "sound/sn76496.h"
#include "includes/konamipt.h"
#include "includes/mikie.h"


#define MIKIE_TIMER_RATE 512

#define XTAL    14318180
#define OSC     18432000
#define CLK     XTAL/4


/*************************************
 *
 *  Memory handlers
 *
 *************************************/

READ8_MEMBER(mikie_state::mikie_sh_timer_r)
{
	int clock = m_audiocpu->total_cycles() / MIKIE_TIMER_RATE;

	return clock;
}

WRITE8_MEMBER(mikie_state::mikie_sh_irqtrigger_w)
{
	if (m_last_irq == 0 && data == 1)
	{
		// setting bit 0 low then high triggers IRQ on the sound CPU
		m_audiocpu->set_input_line_and_vector(0, HOLD_LINE, 0xff);
	}

	m_last_irq = data;
}

WRITE8_MEMBER(mikie_state::mikie_coin_counter_w)
{
	machine().bookkeeping().coin_counter_w(offset, data);
}

WRITE8_MEMBER(mikie_state::irq_mask_w)
{
	m_irq_mask = data & 1;
}

/*************************************
 *
 *  Address maps
 *
 *************************************/

static ADDRESS_MAP_START( mikie_map, AS_PROGRAM, 8, mikie_state )
	AM_RANGE(0x0000, 0x00ff) AM_RAM
	AM_RANGE(0x2000, 0x2001) AM_WRITE(mikie_coin_counter_w)
	AM_RANGE(0x2002, 0x2002) AM_WRITE(mikie_sh_irqtrigger_w)
	AM_RANGE(0x2006, 0x2006) AM_WRITE(mikie_flipscreen_w)
	AM_RANGE(0x2007, 0x2007) AM_WRITE(irq_mask_w)
	AM_RANGE(0x2100, 0x2100) AM_WRITE(watchdog_reset_w)
	AM_RANGE(0x2200, 0x2200) AM_WRITE(mikie_palettebank_w)
	AM_RANGE(0x2300, 0x2300) AM_WRITENOP    // ???
	AM_RANGE(0x2400, 0x2400) AM_READ_PORT("SYSTEM") AM_WRITE(soundlatch_byte_w)
	AM_RANGE(0x2401, 0x2401) AM_READ_PORT("P1")
	AM_RANGE(0x2402, 0x2402) AM_READ_PORT("P2")
	AM_RANGE(0x2403, 0x2403) AM_READ_PORT("DSW3")
	AM_RANGE(0x2500, 0x2500) AM_READ_PORT("DSW1")
	AM_RANGE(0x2501, 0x2501) AM_READ_PORT("DSW2")
	AM_RANGE(0x2800, 0x288f) AM_RAM AM_SHARE("spriteram")
	AM_RANGE(0x2890, 0x37ff) AM_RAM
	AM_RANGE(0x3800, 0x3bff) AM_RAM_WRITE(mikie_colorram_w) AM_SHARE("colorram")
	AM_RANGE(0x3c00, 0x3fff) AM_RAM_WRITE(mikie_videoram_w) AM_SHARE("videoram")
	AM_RANGE(0x4000, 0x5fff) AM_ROM // Machine checks for extra rom
	AM_RANGE(0x6000, 0xffff) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START( sound_map, AS_PROGRAM, 8, mikie_state )
	AM_RANGE(0x0000, 0x3fff) AM_ROM
	AM_RANGE(0x4000, 0x43ff) AM_RAM
	AM_RANGE(0x8000, 0x8000) AM_WRITENOP    // sound command latch
	AM_RANGE(0x8001, 0x8001) AM_WRITENOP    // ???
	AM_RANGE(0x8002, 0x8002) AM_DEVWRITE("sn1", sn76489a_device, write) // trigger read of latch
	AM_RANGE(0x8003, 0x8003) AM_READ(soundlatch_byte_r)
	AM_RANGE(0x8004, 0x8004) AM_DEVWRITE("sn2", sn76489a_device, write) // trigger read of latch
	AM_RANGE(0x8005, 0x8005) AM_READ(mikie_sh_timer_r)
	AM_RANGE(0x8079, 0x8079) AM_WRITENOP    // ???
	AM_RANGE(0xa003, 0xa003) AM_WRITENOP    // ???
ADDRESS_MAP_END

/*************************************
 *
 *  Input ports
 *
 *************************************/

/* verified from M6809 code */
static INPUT_PORTS_START( mikie )
	PORT_START("SYSTEM")
	KONAMI8_SYSTEM_UNK

	PORT_START("P1")
	KONAMI8_MONO_4WAY_B12_UNK

	PORT_START("P2")
	KONAMI8_COCKTAIL_4WAY_B12_UNK

	PORT_START("DSW1")
	KONAMI_COINAGE_LOC(DEF_STR( Free_Play ), "No Coin B", SW1)
	/* "No Coin B" = coins produce sound, but no effect on coin counter */

	PORT_START("DSW2")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Lives ) )            PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(    0x03, "3" )
	PORT_DIPSETTING(    0x02, "4" )
	PORT_DIPSETTING(    0x01, "5" )
	PORT_DIPSETTING(    0x00, "7" )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Cabinet ) )          PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x18, 0x18, DEF_STR( Bonus_Life ) )       PORT_DIPLOCATION("SW2:4,5")
	PORT_DIPSETTING(    0x18, "20k 70k 50k+" )
	PORT_DIPSETTING(    0x10, "30K 90k 60k+" )
	PORT_DIPSETTING(    0x08, "30k only" )
	PORT_DIPSETTING(    0x00, "40K only" )
	PORT_DIPNAME( 0x60, 0x60, DEF_STR( Difficulty ) )       PORT_DIPLOCATION("SW2:6,7")
	PORT_DIPSETTING(    0x60, DEF_STR( Easy ) )             /* 1 */
	PORT_DIPSETTING(    0x40, DEF_STR( Medium ) )           /* 2 */
	PORT_DIPSETTING(    0x20, DEF_STR( Hard ) )             /* 3 */
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )          /* 4 */
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Demo_Sounds ) )      PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	/* DSW3 is not mounted on PCB nor listed in manual */
	PORT_START("DSW3")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "Upright Controls" )
	PORT_DIPSETTING(    0x02, DEF_STR( Single ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Dual ) )
	PORT_DIPUNUSED( 0x04, 0x04 )
	PORT_DIPUNUSED( 0x08, 0x08 )
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END


/*************************************
 *
 *  Graphics definitions
 *
 *************************************/

static const gfx_layout charlayout =
{
	8,8,    /* 8*8 characters */
	512,    /* 512 characters */
	4,      /* 4 bits per pixel */
	{ 0, 1, 2, 3 }, /* the bitplanes are packed */
	{ 0*4, 1*4, 2*4, 3*4, 4*4, 5*4, 6*4, 7*4 },
	{ 0*4*8, 1*4*8, 2*4*8, 3*4*8, 4*4*8, 5*4*8, 6*4*8, 7*4*8 },
	8*4*8     /* every char takes 32 consecutive bytes */
};

static const gfx_layout spritelayout =
{
	16,16,       /* 16*16 sprites */
	256,            /* 256 sprites */
	4,             /* 4 bits per pixel */
	{ 0, 4, 256*128*8+0, 256*128*8+4 },
	{ 32*8+0, 32*8+1, 32*8+2, 32*8+3, 16*8+0, 16*8+1, 16*8+2, 16*8+3,
			0, 1, 2, 3, 48*8+0, 48*8+1, 48*8+2, 48*8+3 },
	{ 0*16, 1*16, 2*16, 3*16, 4*16, 5*16, 6*16, 7*16,
			32*16, 33*16, 34*16, 35*16, 36*16, 37*16, 38*16, 39*16 },
	128*8   /* every sprite takes 64 bytes */
};

static GFXDECODE_START( mikie )
	GFXDECODE_ENTRY( "gfx1", 0x0000, charlayout,         0, 16*8 )
	GFXDECODE_ENTRY( "gfx2", 0x0000, spritelayout, 16*8*16, 16*8 )
	GFXDECODE_ENTRY( "gfx2", 0x0001, spritelayout, 16*8*16, 16*8 )
GFXDECODE_END


/*************************************
 *
 *  Machine driver
 *
 *************************************/

void mikie_state::machine_start()
{
	save_item(NAME(m_palettebank));
	save_item(NAME(m_last_irq));
}

void mikie_state::machine_reset()
{
	m_palettebank = 0;
	m_last_irq = 0;
}

INTERRUPT_GEN_MEMBER(mikie_state::vblank_irq)
{
	if(m_irq_mask)
		device.execute().set_input_line(0, HOLD_LINE);
}

static MACHINE_CONFIG_START( mikie, mikie_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M6809, OSC/12)
	MCFG_CPU_PROGRAM_MAP(mikie_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", mikie_state,  vblank_irq)

	MCFG_CPU_ADD("audiocpu", Z80, CLK)
	MCFG_CPU_PROGRAM_MAP(sound_map)


	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60.59)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(32*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 2*8, 30*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(mikie_state, screen_update_mikie)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", mikie)
	MCFG_PALETTE_ADD("palette", 16*8*16+16*8*16)
	MCFG_PALETTE_INDIRECT_ENTRIES(256)
	MCFG_PALETTE_INIT_OWNER(mikie_state, mikie)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("sn1", SN76489A, XTAL/8)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.60)

	MCFG_SOUND_ADD("sn2", SN76489A, CLK)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.60)
MACHINE_CONFIG_END

/*************************************
 *
 *  ROM definition(s)
 *
 *************************************/

ROM_START( mikie )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "n14.11c", 0x6000, 0x2000, CRC(f698e6dd) SHA1(99220eeee4e7b88caa26f2d08502689e1f1fcdf8) )
	ROM_LOAD( "o13.12a", 0x8000, 0x4000, CRC(826e7035) SHA1(bd62783cb1ba4e7f0196f337280461bb7627f70f) )
	ROM_LOAD( "o17.12d", 0xc000, 0x4000, CRC(161c25c8) SHA1(373a92b8412676ad9870cb562c73e47db7b40bea) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "n10.6e",  0x0000, 0x2000, CRC(2cf9d670) SHA1(b324b92aff70d7878160128611dd5fdec6949659) )

	ROM_REGION( 0x4000, "gfx1", 0 )
	ROM_LOAD( "o11.8i",  0x0000, 0x4000, CRC(3c82aaf3) SHA1(c84256ac5fd5e40b197651c56e303c69aae72950) )

	ROM_REGION( 0x10000, "gfx2", 0 )
	ROM_LOAD( "001.f1",  0x0000, 0x4000, CRC(a2ba0df5) SHA1(873d49c1c2efbb222d1bf63396729d4b7d9477c3) )
	ROM_LOAD( "003.f3",  0x4000, 0x4000, CRC(9775ab32) SHA1(0271567c5f5a6bb2eaffb9d5dc2af6b8142dc8a9) )
	ROM_LOAD( "005.h1",  0x8000, 0x4000, CRC(ba44aeef) SHA1(410bfd9146242254a920092e280af87586709527) )
	ROM_LOAD( "007.h3",  0xc000, 0x4000, CRC(31afc153) SHA1(31ca33f585fddb86131ef61de73e3563fa027455) )

	ROM_REGION( 0x500, "proms", 0 )
	ROM_LOAD( "d19.1i",  0x0000, 0x0100, CRC(8b83e7cf) SHA1(4fce779947f9f318023c7c54a71a4751f6bb8eb1) ) // red component
	ROM_LOAD( "d21.3i",  0x0100, 0x0100, CRC(3556304a) SHA1(6f4fc3ef6b1b44278e7c8c1034ee4fbef90cf85a) ) // green component
	ROM_LOAD( "d20.2i",  0x0200, 0x0100, CRC(676a0669) SHA1(14236a831204d52cdf8c2ef318a565d6c5587ce0) ) // blue component
	ROM_LOAD( "d22.12h", 0x0300, 0x0100, CRC(872be05c) SHA1(1525303589d7ed909bc6e2827fbaa2c16ad4030b) ) // character lookup table
	ROM_LOAD( "d18.f9",  0x0400, 0x0100, CRC(7396b374) SHA1(fedcc421a61d6623dc9c41b0a3e164efeb50ec7c) ) // sprite lookup table
ROM_END

ROM_START( mikiej )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "n14.11c", 0x6000, 0x2000, CRC(f698e6dd) SHA1(99220eeee4e7b88caa26f2d08502689e1f1fcdf8) )
	ROM_LOAD( "o13.12a", 0x8000, 0x4000, CRC(826e7035) SHA1(bd62783cb1ba4e7f0196f337280461bb7627f70f) )
	ROM_LOAD( "o17.12d", 0xc000, 0x4000, CRC(161c25c8) SHA1(373a92b8412676ad9870cb562c73e47db7b40bea) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "n10.6e",  0x0000, 0x2000, CRC(2cf9d670) SHA1(b324b92aff70d7878160128611dd5fdec6949659) )

	ROM_REGION( 0x4000, "gfx1", 0 )
	ROM_LOAD( "q11.8i",  0x0000, 0x4000, CRC(c48b269b) SHA1(d7fcfa44fcda90f1a7df6c974210716ae82c47a3) )

	ROM_REGION( 0x10000, "gfx2", 0 )
	ROM_LOAD( "q01.f1",  0x0000, 0x4000, CRC(31551987) SHA1(b6cbdb8b511d99b27546a6c4d01f2948d5ad3a42) )
	ROM_LOAD( "q03.f3",  0x4000, 0x4000, CRC(34414df0) SHA1(0189deac6f19de386b4e49cfe6322b212e74264a) )
	ROM_LOAD( "q05.h1",  0x8000, 0x4000, CRC(f9e1ebb1) SHA1(c88c1fc22f21b3e7d558c47de2716dac01fdd621) )
	ROM_LOAD( "q07.h3",  0xc000, 0x4000, CRC(15dc093b) SHA1(0b5a5aea25283b8edb7f534fc84b13f5176e26d6) )

	ROM_REGION( 0x500, "proms", 0 )
	ROM_LOAD( "d19.1i",  0x0000, 0x0100, CRC(8b83e7cf) SHA1(4fce779947f9f318023c7c54a71a4751f6bb8eb1) ) // red component
	ROM_LOAD( "d21.3i",  0x0100, 0x0100, CRC(3556304a) SHA1(6f4fc3ef6b1b44278e7c8c1034ee4fbef90cf85a) ) // green component
	ROM_LOAD( "d20.2i",  0x0200, 0x0100, CRC(676a0669) SHA1(14236a831204d52cdf8c2ef318a565d6c5587ce0) ) // blue component
	ROM_LOAD( "d22.12h", 0x0300, 0x0100, CRC(872be05c) SHA1(1525303589d7ed909bc6e2827fbaa2c16ad4030b) ) // character lookup table
	ROM_LOAD( "d18.f9",  0x0400, 0x0100, CRC(7396b374) SHA1(fedcc421a61d6623dc9c41b0a3e164efeb50ec7c) ) // sprite lookup table
ROM_END

ROM_START( mikiehs )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "l14.11c", 0x6000, 0x2000, CRC(633f3a6d) SHA1(9255e0cb8d53773a52cade2fbd2e4c1968164313) )
	ROM_LOAD( "m13.12a", 0x8000, 0x4000, CRC(9c42d715) SHA1(533ae7c5bde6d341b9138dd439d9ff46fe0767f4) )
	ROM_LOAD( "m17.12d", 0xc000, 0x4000, CRC(cb5c03c9) SHA1(6bc2f4c5f4f97c2bbcac8ff51358369ce07948e9) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "h10.6e",  0x0000, 0x2000, CRC(4ed887d2) SHA1(953218a3b41019e2e52932dd3522741812c46c75) )

	ROM_REGION( 0x4000, "gfx1", 0 )
	ROM_LOAD( "l11.8i",  0x0000, 0x4000, CRC(5ba9d86b) SHA1(2246795dd68a62efb2c70a9177ee97a58ccb2566) )

	ROM_REGION( 0x10000, "gfx2", 0 )
	ROM_LOAD( "i01.f1",  0x0000, 0x4000, CRC(0c0cab5f) SHA1(c3eb4c3a432e86f4664329a0de5583cb5de7b6f5) )
	ROM_LOAD( "i03.f3",  0x4000, 0x4000, CRC(694da32f) SHA1(02bd83d77f42822e42e48977856dfa0e3abfcab0) )
	ROM_LOAD( "i05.h1",  0x8000, 0x4000, CRC(00e357e1) SHA1(d5b46709083d74950d0deedecb4fd631d0e74afb) )
	ROM_LOAD( "i07.h3",  0xc000, 0x4000, CRC(ceeba6ac) SHA1(ca5f715dece88540d9ed0e0146cff09f6868d09f) )

	ROM_REGION( 0x500, "proms", 0 )
	ROM_LOAD( "d19.1i",  0x0000, 0x0100, CRC(8b83e7cf) SHA1(4fce779947f9f318023c7c54a71a4751f6bb8eb1) ) // red component
	ROM_LOAD( "d21.3i",  0x0100, 0x0100, CRC(3556304a) SHA1(6f4fc3ef6b1b44278e7c8c1034ee4fbef90cf85a) ) // green component
	ROM_LOAD( "d20.2i",  0x0200, 0x0100, CRC(676a0669) SHA1(14236a831204d52cdf8c2ef318a565d6c5587ce0) ) // blue component
	ROM_LOAD( "d22.12h", 0x0300, 0x0100, CRC(872be05c) SHA1(1525303589d7ed909bc6e2827fbaa2c16ad4030b) ) // character lookup table
	ROM_LOAD( "d18.f9",  0x0400, 0x0100, CRC(7396b374) SHA1(fedcc421a61d6623dc9c41b0a3e164efeb50ec7c) ) // sprite lookup table
ROM_END

/*************************************
 *
 *  Game driver(s)
 *
 *************************************/

GAME( 1984, mikie,   0,     mikie, mikie, driver_device, 0, ROT270, "Konami", "Mikie", MACHINE_SUPPORTS_SAVE )
GAME( 1984, mikiej,  mikie, mikie, mikie, driver_device, 0, ROT270, "Konami", "Shinnyuushain Tooru-kun", MACHINE_SUPPORTS_SAVE )
GAME( 1984, mikiehs, mikie, mikie, mikie, driver_device, 0, ROT270, "Konami", "Mikie (High School Graffiti)", MACHINE_SUPPORTS_SAVE )
