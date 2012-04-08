/***************************************************************************

    Konami Battlantis Hardware

    Supports:
     GX765 - Rack 'em Up/The Hustler (c) 1987 Konami
     GX777 - Battlantis (c) 1987 Konami

    Preliminary driver by: Manuel Abadia <manu@teleline.es>

***************************************************************************/

#include "emu.h"
#include "cpu/hd6309/hd6309.h"
#include "cpu/z80/z80.h"
#include "sound/3812intf.h"
#include "video/konicdev.h"
#include "includes/konamipt.h"
#include "includes/battlnts.h"


/*************************************
 *
 *  Memory handlers
 *
 *************************************/

static INTERRUPT_GEN( battlnts_interrupt )
{
	battlnts_state *state = device->machine().driver_data<battlnts_state>();
	if (k007342_is_int_enabled(state->m_k007342))
		device_set_input_line(device, HD6309_IRQ_LINE, HOLD_LINE);
}

WRITE8_MEMBER(battlnts_state::battlnts_sh_irqtrigger_w)
{
	device_set_input_line_and_vector(m_audiocpu, 0, HOLD_LINE, 0xff);
}

WRITE8_MEMBER(battlnts_state::battlnts_bankswitch_w)
{
	/* bits 6 & 7 = bank number */
	memory_set_bank(machine(), "bank1", (data & 0xc0) >> 6);

	/* bits 4 & 5 = coin counters */
	coin_counter_w(machine(), 0, data & 0x10);
	coin_counter_w(machine(), 1, data & 0x20);

	/* other bits unknown */
}


/*************************************
 *
 *  Address maps
 *
 *************************************/

static ADDRESS_MAP_START( battlnts_map, AS_PROGRAM, 8, battlnts_state )
	AM_RANGE(0x0000, 0x1fff) AM_DEVREADWRITE_LEGACY("k007342", k007342_r, k007342_w)	/* Color RAM + Video RAM */
	AM_RANGE(0x2000, 0x21ff) AM_DEVREADWRITE_LEGACY("k007420", k007420_r, k007420_w)	/* Sprite RAM */
	AM_RANGE(0x2200, 0x23ff) AM_DEVREADWRITE_LEGACY("k007342", k007342_scroll_r, k007342_scroll_w)		/* Scroll RAM */
	AM_RANGE(0x2400, 0x24ff) AM_RAM_WRITE(paletteram_xBBBBBGGGGGRRRRR_be_w) AM_SHARE("paletteram")/* palette */
	AM_RANGE(0x2600, 0x2607) AM_DEVWRITE_LEGACY("k007342", k007342_vreg_w)			/* Video Registers */
	AM_RANGE(0x2e00, 0x2e00) AM_READ_PORT("DSW1")
	AM_RANGE(0x2e01, 0x2e01) AM_READ_PORT("P2")
	AM_RANGE(0x2e02, 0x2e02) AM_READ_PORT("P1")
	AM_RANGE(0x2e03, 0x2e03) AM_READ_PORT("DSW3")				/* coinsw, testsw, startsw */
	AM_RANGE(0x2e04, 0x2e04) AM_READ_PORT("DSW2")
	AM_RANGE(0x2e08, 0x2e08) AM_WRITE(battlnts_bankswitch_w)	/* bankswitch control */
	AM_RANGE(0x2e0c, 0x2e0c) AM_WRITE(battlnts_spritebank_w)	/* sprite bank select */
	AM_RANGE(0x2e10, 0x2e10) AM_WRITE(watchdog_reset_w)			/* watchdog reset */
	AM_RANGE(0x2e14, 0x2e14) AM_WRITE(soundlatch_w)				/* sound code # */
	AM_RANGE(0x2e18, 0x2e18) AM_WRITE(battlnts_sh_irqtrigger_w)	/* cause interrupt on audio CPU */
	AM_RANGE(0x4000, 0x7fff) AM_ROMBANK("bank1")						/* banked ROM */
	AM_RANGE(0x8000, 0xffff) AM_ROM								/* ROM 777e02.bin */
ADDRESS_MAP_END

static ADDRESS_MAP_START( battlnts_sound_map, AS_PROGRAM, 8, battlnts_state )
	AM_RANGE(0x0000, 0x7fff) AM_ROM							/* ROM 777c01.rom */
	AM_RANGE(0x8000, 0x87ff) AM_RAM							/* RAM */
	AM_RANGE(0xa000, 0xa001) AM_DEVREADWRITE_LEGACY("ym1", ym3812_r, ym3812_w)		/* YM3812 (chip 1) */
	AM_RANGE(0xc000, 0xc001) AM_DEVREADWRITE_LEGACY("ym2", ym3812_r, ym3812_w)		/* YM3812 (chip 2) */
	AM_RANGE(0xe000, 0xe000) AM_READ(soundlatch_r)			/* soundlatch_r */
ADDRESS_MAP_END

/*************************************
 *
 *  Input ports
 *
 *************************************/

static INPUT_PORTS_START( battlnts )
	PORT_START("DSW1")
	KONAMI_COINAGE_LOC(DEF_STR( Free_Play ), "No Coin B", SW1)
	/* "No Coin B" = coins produce sound, but no effect on coin counter */

	PORT_START("DSW2")
	PORT_DIPNAME( 0x03, 0x02, DEF_STR( Lives ) )		PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(	0x03, "2" )
	PORT_DIPSETTING(	0x02, "3" )
	PORT_DIPSETTING(	0x01, "5" )
	PORT_DIPSETTING(	0x00, "7" )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Cabinet ) )		PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(	0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(	0x04, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x18, 0x18, DEF_STR( Bonus_Life ) )	PORT_DIPLOCATION("SW2:4,5")
	PORT_DIPSETTING(	0x18, "30k And Every 70k" )
	PORT_DIPSETTING(	0x10, "40k And Every 80k" )
	PORT_DIPSETTING(	0x08, "40k" )
	PORT_DIPSETTING(	0x00, "50k" )
	PORT_DIPNAME( 0x60, 0x40, DEF_STR( Difficulty ) )	PORT_DIPLOCATION("SW2:6,7")
	PORT_DIPSETTING(	0x60, DEF_STR( Easy ) )
	PORT_DIPSETTING(	0x40, DEF_STR( Normal ) )
	PORT_DIPSETTING(	0x20, DEF_STR( Difficult ) )
	PORT_DIPSETTING(	0x00, DEF_STR( Very_Difficult ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Demo_Sounds ) )	PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(	0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(	0x00, DEF_STR( On ) )

	PORT_START("DSW3")
	KONAMI8_SYSTEM_10
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Flip_Screen ) )	PORT_DIPLOCATION("SW3:1")
	PORT_DIPSETTING(	0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(	0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "Upright Controls" )		PORT_DIPLOCATION("SW3:2")
	PORT_DIPSETTING(	0x40, DEF_STR( Single ) )
	PORT_DIPSETTING(	0x00, DEF_STR( Dual ) )
	PORT_SERVICE_DIPLOC(0x80, IP_ACTIVE_LOW, "SW3:3" )

	PORT_START("P1")
	KONAMI8_B1(1)
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Allow_Continue ) ) PORT_DIPLOCATION("SW3:4")
	PORT_DIPSETTING(	0x80, "3 Times" )
	PORT_DIPSETTING(	0x00, "5 Times" )

	PORT_START("P2")
	KONAMI8_B1_UNK(2)
INPUT_PORTS_END

static INPUT_PORTS_START( rackemup )
	PORT_INCLUDE( battlnts )

	PORT_MODIFY("DSW2")
	PORT_DIPNAME( 0x03, 0x02, "Balls" )					PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(	0x03, "2" )
	PORT_DIPSETTING(	0x02, "3" )
	PORT_DIPSETTING(	0x01, "4" )
	PORT_DIPSETTING(	0x00, "7" )
	PORT_DIPNAME( 0x18, 0x10, "Time To Aim" )			PORT_DIPLOCATION("SW2:4,5")
	PORT_DIPSETTING(	0x18, "25s (Stage 1: 30s)" )
	PORT_DIPSETTING(	0x10, "20s (Stage 1: 25s)" )
	PORT_DIPSETTING(	0x08, "17s (Stage 1: 22s)" )
	PORT_DIPSETTING(	0x00, "15s (Stage 1: 20s)" )

	PORT_MODIFY("P1")
	KONAMI8_B12(1)
	PORT_DIPUNUSED_DIPLOC( 0x80, 0x80, "SW3:4" )

	PORT_MODIFY("P2")
	KONAMI8_B12_UNK(2)
INPUT_PORTS_END

static INPUT_PORTS_START( thehustl )
	PORT_INCLUDE( rackemup )

	PORT_MODIFY("DSW3")
	PORT_DIPUNUSED_DIPLOC( 0x40, 0x40, "SW3:2" )
INPUT_PORTS_END


/*************************************
 *
 *  Graphics definitions
 *
 *************************************/

static const gfx_layout charlayout =
{
	8,8,			/* 8 x 8 characters */
	0x40000/32, 	/* 8192 characters */
	4,				/* 4bpp */
	{ 0, 1, 2, 3 }, /* the four bitplanes are packed in one nibble */
	{ 2*4, 3*4, 0*4, 1*4, 6*4, 7*4, 4*4, 5*4 },
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32 },
	32*8			/* every character takes 32 consecutive bytes */
};

static const gfx_layout spritelayout =
{
	8,8,			/* 8*8 sprites */
	0x40000/32, /* 8192 sprites */
	4,				/* 4 bpp */
	{ 0, 1, 2, 3 }, /* the four bitplanes are packed in one nibble */
	{ 0*4, 1*4, 2*4, 3*4, 4*4, 5*4, 6*4, 7*4},
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32 },
	32*8			/* every sprite takes 32 consecutive bytes */
};


static GFXDECODE_START( battlnts )
	GFXDECODE_ENTRY( "gfx1", 0, charlayout,		0, 1 ) /* colors  0-15 */
	GFXDECODE_ENTRY( "gfx2", 0, spritelayout, 4*16, 1 ) /* colors 64-79 */
GFXDECODE_END


/*************************************
 *
 *  Machine driver
 *
 *************************************/

static const k007342_interface bladestl_k007342_intf =
{
	0,	battlnts_tile_callback	/* gfx_num (for tile creation), callback */
};

static const k007420_interface bladestl_k007420_intf =
{
	0x3ff,	battlnts_sprite_callback	/* banklimit, callback */
};


static MACHINE_START( battlnts )
{
	battlnts_state *state = machine.driver_data<battlnts_state>();
	UINT8 *ROM = machine.region("maincpu")->base();

	memory_configure_bank(machine, "bank1", 0, 4, &ROM[0x10000], 0x4000);

	state->m_audiocpu = machine.device("audiocpu");
	state->m_k007342 = machine.device("k007342");
	state->m_k007420 = machine.device("k007420");

	state->save_item(NAME(state->m_spritebank));
	state->save_item(NAME(state->m_layer_colorbase));
}

static MACHINE_RESET( battlnts )
{
	battlnts_state *state = machine.driver_data<battlnts_state>();

	state->m_layer_colorbase[0] = 0;
	state->m_layer_colorbase[1] = 0;
	state->m_spritebank = 0;
}

static MACHINE_CONFIG_START( battlnts, battlnts_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", HD6309, XTAL_24MHz / 2 /* 3000000*4? */)
	MCFG_CPU_PROGRAM_MAP(battlnts_map)
	MCFG_CPU_VBLANK_INT("screen", battlnts_interrupt)

	MCFG_CPU_ADD("audiocpu", Z80, XTAL_24MHz / 6 /* 3579545? */)
	MCFG_CPU_PROGRAM_MAP(battlnts_sound_map)

	MCFG_MACHINE_START(battlnts)
	MCFG_MACHINE_RESET(battlnts)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(32*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 2*8, 30*8-1)
	MCFG_SCREEN_UPDATE_STATIC(battlnts)

	MCFG_GFXDECODE(battlnts)
	MCFG_PALETTE_LENGTH(128)

	MCFG_K007342_ADD("k007342", bladestl_k007342_intf)
	MCFG_K007420_ADD("k007420", bladestl_k007420_intf)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("ym1", YM3812, XTAL_24MHz / 8)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)

	MCFG_SOUND_ADD("ym2", YM3812, XTAL_24MHz / 8)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_CONFIG_END


/*************************************
 *
 *  ROM definition(s)
 *
 *************************************/

ROM_START( battlnts )
	ROM_REGION( 0x20000, "maincpu", 0 ) /* code + banked roms */
	ROM_LOAD( "777_g02.7e", 0x08000, 0x08000, CRC(dbd8e17e) SHA1(586a22b714011c67a915c4a350ceca19ff875635) ) /* fixed ROM */
	ROM_LOAD( "777_g03.8e", 0x10000, 0x10000, CRC(7bd44fef) SHA1(308ec5246f5537b34e368535672ac687f456750a) ) /* banked ROM */

	ROM_REGION( 0x10000, "audiocpu", 0 ) /* 64k for the sound CPU */
	ROM_LOAD( "777_c01.10a",  0x00000, 0x08000, CRC(c21206e9) SHA1(7b133e04be67dc061a186ab0481d848b69b370d7) )

	ROM_REGION( 0x40000, "gfx1", 0 )
	ROM_LOAD( "777_c04.13a",  0x00000, 0x40000, CRC(45d92347) SHA1(8537b4ccd0a80ea3260ef82fde177f1d65a49c03) ) /* tiles */

	ROM_REGION( 0x40000, "gfx2", 0 )
	ROM_LOAD( "777_c05.13e",  0x00000, 0x40000, CRC(aeee778c) SHA1(fc58ada9c97361d13439b7b0918c947d48402445) ) /* sprites */
ROM_END

ROM_START( battlntsj )
	ROM_REGION( 0x20000, "maincpu", 0 ) /* code + banked roms */
	ROM_LOAD( "777_e02.7e",  0x08000, 0x08000, CRC(d631cfcb) SHA1(7787da0dd8cd218abc27204e517e04d7a1913a3b) ) /* fixed ROM */
	ROM_LOAD( "777_e03.8e",  0x10000, 0x10000, CRC(5ef1f4ef) SHA1(e3e6e1fc5a65328d94c23e2e76eef3504b70e58b) ) /* banked ROM */

	ROM_REGION( 0x10000, "audiocpu", 0 ) /* 64k for the sound CPU */
	ROM_LOAD( "777_c01.10a",  0x00000, 0x08000, CRC(c21206e9) SHA1(7b133e04be67dc061a186ab0481d848b69b370d7) )

	ROM_REGION( 0x40000, "gfx1", 0 )
	ROM_LOAD( "777_c04.13a",  0x00000, 0x40000, CRC(45d92347) SHA1(8537b4ccd0a80ea3260ef82fde177f1d65a49c03) ) /* tiles */

	ROM_REGION( 0x40000, "gfx2", 0 )
	ROM_LOAD( "777_c05.13e",  0x00000, 0x40000, CRC(aeee778c) SHA1(fc58ada9c97361d13439b7b0918c947d48402445) ) /* sprites */
ROM_END

ROM_START( rackemup )
	ROM_REGION( 0x20000, "maincpu", 0 ) /* code + banked roms */
	ROM_LOAD( "765_l02.7e",  0x08000, 0x08000, CRC(3dfc48bd) SHA1(9ba98e9f27dd0a6efec145bea2a5ae7df8567437) ) /* fixed ROM */
	ROM_LOAD( "765_j03.8e",  0x10000, 0x10000, CRC(a13fd751) SHA1(27ec66835c85b7ac0221a813d38e9cca0d9be3b8) ) /* banked ROM */

	ROM_REGION( 0x10000, "audiocpu", 0 ) /* 64k for the sound CPU */
	ROM_LOAD( "765_j01.10a", 0x00000, 0x08000, CRC(77ae753e) SHA1(9e463a825d31bb79644b083d24b25670d96441c5) )

	ROM_REGION( 0x40000, "gfx1", 0 )
	ROM_LOAD( "765_l04.13a", 0x00000, 0x40000, CRC(acfbeee2) SHA1(c2bf750892ba33d4610fa4497170f49c101ed4c1) ) /* tiles */

	ROM_REGION( 0x40000, "gfx2", 0 )
	ROM_LOAD( "765_l05.13e", 0x00000, 0x40000, CRC(1bb6855f) SHA1(251081564dfede8fa9a422081d58465fe5ca4ed1) ) /* sprites */
ROM_END

ROM_START( thehustl )
	ROM_REGION( 0x20000, "maincpu", 0 ) /* code + banked roms */
	ROM_LOAD( "765_m02.7e",  0x08000, 0x08000, CRC(934807b9) SHA1(84e13a5c1587ee28330f369f9a1180219edbda9d) ) /* fixed ROM */
	ROM_LOAD( "765_j03.8e",  0x10000, 0x10000, CRC(a13fd751) SHA1(27ec66835c85b7ac0221a813d38e9cca0d9be3b8) ) /* banked ROM */

	ROM_REGION( 0x10000, "audiocpu", 0 ) /* 64k for the sound CPU */
	ROM_LOAD( "765_j01.10a", 0x00000, 0x08000, CRC(77ae753e) SHA1(9e463a825d31bb79644b083d24b25670d96441c5) )

	ROM_REGION( 0x40000, "gfx1", 0 )
	ROM_LOAD( "765_e04.13a", 0x00000, 0x40000, CRC(08c2b72e) SHA1(02d9c690da839d6fee75fffdf66a4d3da35a0263) ) /* tiles */

	ROM_REGION( 0x40000, "gfx2", 0 )
	ROM_LOAD( "765_e05.13e", 0x00000, 0x40000, CRC(ef044655) SHA1(c8272283eab8fc2899979da398819cb72c92a299) ) /* sprites */
ROM_END

ROM_START( thehustlj )
	ROM_REGION( 0x20000, "maincpu", 0 ) /* code + banked roms */
	ROM_LOAD( "765_j02.7e",  0x08000, 0x08000, CRC(2ac14c75) SHA1(b88f6279ab88719f4207e28486a0022554668382) ) /* fixed ROM */
	ROM_LOAD( "765_j03.8e",  0x10000, 0x10000, CRC(a13fd751) SHA1(27ec66835c85b7ac0221a813d38e9cca0d9be3b8) ) /* banked ROM */

	ROM_REGION( 0x10000, "audiocpu", 0 ) /* 64k for the sound CPU */
	ROM_LOAD( "765_j01.10a", 0x00000, 0x08000, CRC(77ae753e) SHA1(9e463a825d31bb79644b083d24b25670d96441c5) )

	ROM_REGION( 0x40000, "gfx1", 0 )
	ROM_LOAD( "765_e04.13a", 0x00000, 0x40000, CRC(08c2b72e) SHA1(02d9c690da839d6fee75fffdf66a4d3da35a0263) ) /* tiles */

	ROM_REGION( 0x40000, "gfx2", 0 )
	ROM_LOAD( "765_e05.13e", 0x00000, 0x40000, CRC(ef044655) SHA1(c8272283eab8fc2899979da398819cb72c92a299) ) /* sprites */
ROM_END


/*************************************
 *
 *  Driver initialization
 *
 *************************************/

/*
    This recursive function doesn't use additional memory
    (it could be easily converted into an iterative one).
    It's called shuffle because it mimics the shuffling of a deck of cards.
*/
static void shuffle( UINT8 *buf, int len )
{
	int i;
	UINT8 t;

	if (len == 2)
		return;

	if (len % 4)
		fatalerror("shuffle() - not modulo 4");	/* must not happen */

	len /= 2;

	for (i = 0; i < len / 2; i++)
	{
		t = buf[len / 2 + i];
		buf[len / 2 + i] = buf[len + i];
		buf[len + i] = t;
	}

	shuffle(buf, len);
	shuffle(buf + len, len);
}


static DRIVER_INIT( rackemup )
{
	/* rearrange char ROM */
	shuffle(machine.region("gfx1")->base(), machine.region("gfx1")->bytes());
}



/*************************************
 *
 *  Game driver(s)
 *
 *************************************/

GAME( 1987, battlnts,  0,        battlnts, battlnts, 0,        ROT90, "Konami", "Battlantis", GAME_SUPPORTS_SAVE )
GAME( 1987, battlntsj, battlnts, battlnts, battlnts, 0,        ROT90, "Konami", "Battlantis (Japan)", GAME_SUPPORTS_SAVE )
GAME( 1987, rackemup,  0,        battlnts, rackemup, rackemup, ROT90, "Konami", "Rack 'em Up", GAME_NO_COCKTAIL | GAME_SUPPORTS_SAVE )
GAME( 1987, thehustl,  rackemup, battlnts, thehustl, 0,        ROT90, "Konami", "The Hustler (Japan version M)", GAME_NO_COCKTAIL | GAME_SUPPORTS_SAVE )
GAME( 1987, thehustlj, rackemup, battlnts, thehustl, 0,        ROT90, "Konami", "The Hustler (Japan version J)", GAME_NO_COCKTAIL | GAME_SUPPORTS_SAVE )
