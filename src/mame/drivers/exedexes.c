// license:???
// copyright-holders:Richard Davies
/***************************************************************************

    Exed Exes

    Notes:
    - Flip screen is not supported, but doesn't seem to be used (no flip screen
      dip switch and no cocktail mode)
    - Some writes to unknown memory locations (always 0?)

***************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "sound/ay8910.h"
#include "sound/sn76496.h"
#include "includes/exedexes.h"


TIMER_DEVICE_CALLBACK_MEMBER(exedexes_state::exedexes_scanline)
{
	int scanline = param;

	if(scanline == 240) // vblank-out irq
		m_maincpu->set_input_line_and_vector(0, HOLD_LINE, 0xd7);   /* RST 10h - vblank */

	if(scanline == 0) // unknown irq event
		m_maincpu->set_input_line_and_vector(0, HOLD_LINE, 0xcf);   /* RST 08h */
}


static ADDRESS_MAP_START( exedexes_map, AS_PROGRAM, 8, exedexes_state )
	AM_RANGE(0x0000, 0xbfff) AM_ROM
	AM_RANGE(0xc000, 0xc000) AM_READ_PORT("SYSTEM")
	AM_RANGE(0xc001, 0xc001) AM_READ_PORT("P1")
	AM_RANGE(0xc002, 0xc002) AM_READ_PORT("P2")
	AM_RANGE(0xc003, 0xc003) AM_READ_PORT("DSW0")
	AM_RANGE(0xc004, 0xc004) AM_READ_PORT("DSW1")
	AM_RANGE(0xc800, 0xc800) AM_WRITE(soundlatch_byte_w)
	AM_RANGE(0xc804, 0xc804) AM_WRITE(exedexes_c804_w)                              /* coin counters + text layer enable */
	AM_RANGE(0xc806, 0xc806) AM_WRITENOP                                            /* Watchdog ?? */
	AM_RANGE(0xd000, 0xd3ff) AM_RAM_WRITE(exedexes_videoram_w) AM_SHARE("videoram") /* Video RAM */
	AM_RANGE(0xd400, 0xd7ff) AM_RAM_WRITE(exedexes_colorram_w) AM_SHARE("colorram") /* Color RAM */
	AM_RANGE(0xd800, 0xd801) AM_WRITEONLY AM_SHARE("nbg_yscroll")
	AM_RANGE(0xd802, 0xd803) AM_WRITEONLY AM_SHARE("nbg_xscroll")
	AM_RANGE(0xd804, 0xd805) AM_WRITEONLY AM_SHARE("bg_scroll")
	AM_RANGE(0xd807, 0xd807) AM_WRITE(exedexes_gfxctrl_w)                           /* layer enables */
	AM_RANGE(0xe000, 0xefff) AM_RAM                                                 /* Work RAM */
	AM_RANGE(0xf000, 0xffff) AM_RAM AM_SHARE("spriteram")   /* Sprite RAM */
ADDRESS_MAP_END



static ADDRESS_MAP_START( sound_map, AS_PROGRAM, 8, exedexes_state )
	AM_RANGE(0x0000, 0x3fff) AM_ROM
	AM_RANGE(0x4000, 0x47ff) AM_RAM
	AM_RANGE(0x6000, 0x6000) AM_READ(soundlatch_byte_r)
	AM_RANGE(0x8000, 0x8001) AM_DEVWRITE("aysnd", ay8910_device, address_data_w)
	AM_RANGE(0x8002, 0x8002) AM_DEVWRITE("sn1", sn76489_device, write)
	AM_RANGE(0x8003, 0x8003) AM_DEVWRITE("sn2", sn76489_device, write)
ADDRESS_MAP_END



static INPUT_PORTS_START( exedexes )
	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )    /* probably unused */
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )    /* probably unused */
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )    /* probably unused */
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SERVICE1 ) PORT_IMPULSE(8)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN2 )

	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )    /* probably unused */
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )    /* probably unused */

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )    /* probably unused */
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )    /* probably unused */

	PORT_START("DSW0")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x03, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x08, "1" )
	PORT_DIPSETTING(    0x04, "2" )
	PORT_DIPSETTING(    0x0c, "3" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPNAME( 0x10, 0x10, "2 Players Game" )
	PORT_DIPSETTING(    0x00, "1 Credit" )
	PORT_DIPSETTING(    0x10, "2 Credits" )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Language ) )
	PORT_DIPSETTING(    0x00, DEF_STR( English ))
	PORT_DIPSETTING(    0x20, DEF_STR( Japanese ))
	PORT_DIPNAME( 0x40, 0x40, "Freeze" )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ))
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_SERVICE( 0x80, IP_ACTIVE_LOW )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x07, 0x07, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 4C_1C ))
	PORT_DIPSETTING(    0x01, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_5C ) )
	PORT_DIPNAME( 0x38, 0x38, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 4C_1C ))
	PORT_DIPSETTING(    0x08, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x38, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x28, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x18, DEF_STR( 1C_5C ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Allow_Continue ) )
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )
INPUT_PORTS_END




static const gfx_layout charlayout =
{
	8,8,    /* 8*8 characters */
	512,    /* 512 characters */
	2,  /* 2 bits per pixel */
	{ 4, 0 },
	{ 0, 1, 2, 3, 8+0, 8+1, 8+2, 8+3 },
	{ 0*16, 1*16, 2*16, 3*16, 4*16, 5*16, 6*16, 7*16 },
	16*8    /* every char takes 16 consecutive bytes */
};

static const gfx_layout spritelayout =
{
	16,16,  /* 16*16 sprites */
		256,    /* 256 sprites */
		4,      /* 4 bits per pixel */
		{ 0x4000*8+4, 0x4000*8+0, 4, 0 },
	{ 0, 1, 2, 3, 8+0, 8+1, 8+2, 8+3,
			32*8+0, 32*8+1, 32*8+2, 32*8+3, 33*8+0, 33*8+1, 33*8+2, 33*8+3 },
	{ 0*16, 1*16, 2*16, 3*16, 4*16, 5*16, 6*16, 7*16,
			8*16, 9*16, 10*16, 11*16, 12*16, 13*16, 14*16, 15*16 },
	64*8    /* every sprite takes 64 consecutive bytes */
};

static const gfx_layout tilelayout =
{
	32,32,  /* 32*32 tiles */
	64,    /* 64 tiles */
	2,      /* 2 bits per pixel */
	{ 4, 0 },
	{ 0, 1, 2, 3, 8+0, 8+1, 8+2, 8+3,
			64*8+0, 64*8+1, 64*8+2, 64*8+3, 65*8+0, 65*8+1, 65*8+2, 65*8+3,
			128*8+0, 128*8+1, 128*8+2, 128*8+3, 129*8+0, 129*8+1, 129*8+2, 129*8+3,
			192*8+0, 192*8+1, 192*8+2, 192*8+3, 193*8+0, 193*8+1, 193*8+2, 193*8+3 },
	{ 0*16, 1*16, 2*16, 3*16, 4*16, 5*16, 6*16, 7*16,
			8*16, 9*16, 10*16, 11*16, 12*16, 13*16, 14*16, 15*16,
			16*16, 17*16, 18*16, 19*16, 20*16, 21*16, 22*16, 23*16,
			24*16, 25*16, 26*16, 27*16, 28*16, 29*16, 30*16, 31*16 },
	256*8   /* every tile takes 256 consecutive bytes */
};



static GFXDECODE_START( exedexes )
	GFXDECODE_ENTRY( "gfx1", 0, charlayout,              0, 64 )
	GFXDECODE_ENTRY( "gfx2", 0, tilelayout,           64*4, 64 ) /* 32x32 Tiles */
	GFXDECODE_ENTRY( "gfx3", 0, spritelayout,       2*64*4, 16 ) /* 16x16 Tiles */
	GFXDECODE_ENTRY( "gfx4", 0, spritelayout, 2*64*4+16*16, 16 ) /* Sprites */
GFXDECODE_END


void exedexes_state::machine_start()
{
	save_item(NAME(m_chon));
	save_item(NAME(m_objon));
	save_item(NAME(m_sc1on));
	save_item(NAME(m_sc2on));
}

void exedexes_state::machine_reset()
{
	m_chon = 0;
	m_objon = 0;
	m_sc1on = 0;
	m_sc2on = 0;
}

static MACHINE_CONFIG_START( exedexes, exedexes_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, 4000000)   /* 4 MHz (?) */
	MCFG_CPU_PROGRAM_MAP(exedexes_map)
	MCFG_TIMER_DRIVER_ADD_SCANLINE("scantimer", exedexes_state, exedexes_scanline, "screen", 0, 1)

	MCFG_CPU_ADD("audiocpu", Z80, 3000000)  /* 3 MHz ??? */
	MCFG_CPU_PROGRAM_MAP(sound_map)
	MCFG_CPU_PERIODIC_INT_DRIVER(exedexes_state, irq0_line_hold, 4*60)


	/* video hardware */
	MCFG_BUFFERED_SPRITERAM8_ADD("spriteram")

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(32*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 2*8, 30*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(exedexes_state, screen_update_exedexes)
	MCFG_SCREEN_VBLANK_DEVICE("spriteram", buffered_spriteram8_device, vblank_copy_rising)
	MCFG_SCREEN_PALETTE("palette")
	MCFG_SCREEN_ORIENTATION(ROT270)

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", exedexes)

	MCFG_PALETTE_ADD("palette", 64*4+64*4+16*16+16*16)
	MCFG_PALETTE_INDIRECT_ENTRIES(256)
	MCFG_PALETTE_INIT_OWNER(exedexes_state, exedexes)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("aysnd", AY8910, 1500000)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.10)

	MCFG_SOUND_ADD("sn1", SN76489, 3000000)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.36)

	MCFG_SOUND_ADD("sn2", SN76489, 3000000)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.36)
MACHINE_CONFIG_END



ROM_START( exedexes )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "11m_ee04.bin", 0x0000, 0x4000, CRC(44140dbd) SHA1(7b56f614f7cd7655ffa3e1f4adba5a20fa25822d) )
	ROM_LOAD( "10m_ee03.bin", 0x4000, 0x4000, CRC(bf72cfba) SHA1(9f0b9472890db95e16a71f26da954780d5ec7c16) )
	ROM_LOAD( "09m_ee02.bin", 0x8000, 0x4000, CRC(7ad95e2f) SHA1(53fd8d6985d08106bab45e83827a509486d640b7) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "11e_ee01.bin", 0x00000, 0x4000, CRC(73cdf3b2) SHA1(c9f2c91011bdeecec8fa76a42d95f3a5ec77cec9) )

	ROM_REGION( 0x02000, "gfx1", 0 )
	ROM_LOAD( "05c_ee00.bin", 0x00000, 0x2000, CRC(cadb75bd) SHA1(2086be5e295e5d870bcb35f116cc925f811b7583) ) /* Characters */

	ROM_REGION( 0x04000, "gfx2", 0 )
	ROM_LOAD( "h01_ee08.bin", 0x00000, 0x4000, CRC(96a65c1d) SHA1(3b49c64b32f01ec72cf2d943bfe3aa575d62a765) ) /* 32x32 tiles planes 0-1 */

	ROM_REGION( 0x08000, "gfx3", 0 )
	ROM_LOAD( "a03_ee06.bin", 0x00000, 0x4000, CRC(6039bdd1) SHA1(01156e02ed59e6c1e55204729e515cd4419568fb) ) /* 16x16 tiles planes 0-1 */
	ROM_LOAD( "a02_ee05.bin", 0x04000, 0x4000, CRC(b32d8252) SHA1(738225146ba38f2a9216fda278838e7ebb29a0bb) ) /* 16x16 tiles planes 2-3 */

	ROM_REGION( 0x08000, "gfx4", 0 )
	ROM_LOAD( "j11_ee10.bin", 0x00000, 0x4000, CRC(bc83e265) SHA1(ac9b4cce9e539c560414abf2fc239910f2bfbb2d) ) /* Sprites planes 0-1 */
	ROM_LOAD( "j12_ee11.bin", 0x04000, 0x4000, CRC(0e0f300d) SHA1(2f973748e459b16673115abf7de8615219e39fa4) ) /* Sprites planes 2-3 */

	ROM_REGION( 0x6000, "gfx5", 0 ) /* background tilemaps */
	ROM_LOAD( "c01_ee07.bin", 0x0000, 0x4000, CRC(3625a68d) SHA1(83010ca356385b713bafe03a502c566f6a9a8365) )    /* Front Tile Map */
	ROM_LOAD( "h04_ee09.bin", 0x4000, 0x2000, CRC(6057c907) SHA1(886790641b84b8cd659d2eb5fd1adbabdd7dad3d) )    /* Back Tile map */

	ROM_REGION( 0x0b20, "proms", 0 )
	ROM_LOAD( "02d_e-02.bin", 0x0000, 0x0100, CRC(8d0d5935) SHA1(a0ab827ff3b641965ef851893c399e3988fde55e) )    /* red component */
	ROM_LOAD( "03d_e-03.bin", 0x0100, 0x0100, CRC(d3c17efc) SHA1(af88340287bd732c91bc5c75970f9de0431b4304) )    /* green component */
	ROM_LOAD( "04d_e-04.bin", 0x0200, 0x0100, CRC(58ba964c) SHA1(1f98f8e484a0462f1a9fadef9e57612a32652599) )    /* blue component */
	ROM_LOAD( "06f_e-05.bin", 0x0300, 0x0100, CRC(35a03579) SHA1(1f1b8c777622a1f5564409c5f3ce69cc68199dae) )    /* char lookup table */
	ROM_LOAD( "l04_e-10.bin", 0x0400, 0x0100, CRC(1dfad87a) SHA1(684844c24e630f46525df97ed67e2e63f7e66d0f) )    /* 32x32 tile lookup table */
	ROM_LOAD( "c04_e-07.bin", 0x0500, 0x0100, CRC(850064e0) SHA1(3884485e91bd82539d0d33f46b7abac60f4c3b1c) )    /* 16x16 tile lookup table */
	ROM_LOAD( "l09_e-11.bin", 0x0600, 0x0100, CRC(2bb68710) SHA1(cfb375316245cb8751e765f163e6acf071dda9ca) )    /* sprite lookup table */
	ROM_LOAD( "l10_e-12.bin", 0x0700, 0x0100, CRC(173184ef) SHA1(f91ecbdc67af1eed6757f660cac8a0e6866c1822) )    /* sprite palette bank */
	ROM_LOAD( "06l_e-06.bin", 0x0800, 0x0100, CRC(712ac508) SHA1(5349d722ab6733afdda65f6e0a98322f0d515e86) )    /* interrupt timing (not used) */
	ROM_LOAD( "k06_e-08.bin", 0x0900, 0x0100, CRC(0eaf5158) SHA1(bafd4108708f66cd7b280e47152b108f3e254fc9) )    /* video timing (not used) */
	ROM_LOAD( "l03_e-09.bin", 0x0a00, 0x0100, CRC(0d968558) SHA1(b376885ac8452b6cbf9ced81b1080bfd570d9b91) )    /* unknown (all 0) */
	ROM_LOAD( "03e_e-01.bin", 0x0b00, 0x0020, CRC(1acee376) SHA1(367094d924f8e0ec36d8310fada4d8143358f697) )    /* unknown (priority?) */
ROM_END

ROM_START( savgbees )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ee04e.11m",    0x0000, 0x4000, CRC(c0caf442) SHA1(f6e137c1707db620db4f79a1e038101bb3acf812) )
	ROM_LOAD( "ee03e.10m",    0x4000, 0x4000, CRC(9cd70ae1) SHA1(ad2c5de469cdc04a8e877e334a93d68d722cec9a) )
	ROM_LOAD( "ee02e.9m",     0x8000, 0x4000, CRC(a04e6368) SHA1(ed350fb490f8f84dcd9e4a9f5fb3b23079d6b996) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "ee01e.11e",    0x00000, 0x4000, CRC(93d3f952) SHA1(5c86d1ddf03083ac2787efb7a29c09b2f46ec3fa) )

	ROM_REGION( 0x02000, "gfx1", 0 )
	ROM_LOAD( "ee00e.5c",     0x00000, 0x2000, CRC(5972f95f) SHA1(7b90ceca37dba773f72a80da6272b00061526348) ) /* Characters */

	ROM_REGION( 0x04000, "gfx2", 0 )
	ROM_LOAD( "h01_ee08.bin", 0x00000, 0x4000, CRC(96a65c1d) SHA1(3b49c64b32f01ec72cf2d943bfe3aa575d62a765) ) /* 32x32 tiles planes 0-1 */

	ROM_REGION( 0x08000, "gfx3", 0 )
	ROM_LOAD( "a03_ee06.bin", 0x00000, 0x4000, CRC(6039bdd1) SHA1(01156e02ed59e6c1e55204729e515cd4419568fb) ) /* 16x16 tiles planes 0-1 */
	ROM_LOAD( "a02_ee05.bin", 0x04000, 0x4000, CRC(b32d8252) SHA1(738225146ba38f2a9216fda278838e7ebb29a0bb) ) /* 16x16 tiles planes 2-3 */

	ROM_REGION( 0x08000, "gfx4", 0 )
	ROM_LOAD( "j11_ee10.bin", 0x00000, 0x4000, CRC(bc83e265) SHA1(ac9b4cce9e539c560414abf2fc239910f2bfbb2d) ) /* Sprites planes 0-1 */
	ROM_LOAD( "j12_ee11.bin", 0x04000, 0x4000, CRC(0e0f300d) SHA1(2f973748e459b16673115abf7de8615219e39fa4) ) /* Sprites planes 2-3 */

	ROM_REGION( 0x6000, "gfx5", 0 ) /* background tilemaps */
	ROM_LOAD( "c01_ee07.bin", 0x0000, 0x4000, CRC(3625a68d) SHA1(83010ca356385b713bafe03a502c566f6a9a8365) )    /* Front Tile Map */
	ROM_LOAD( "h04_ee09.bin", 0x4000, 0x2000, CRC(6057c907) SHA1(886790641b84b8cd659d2eb5fd1adbabdd7dad3d) )    /* Back Tile map */

	ROM_REGION( 0x0b20, "proms", 0 )
	ROM_LOAD( "02d_e-02.bin", 0x0000, 0x0100, CRC(8d0d5935) SHA1(a0ab827ff3b641965ef851893c399e3988fde55e) )    /* red component */
	ROM_LOAD( "03d_e-03.bin", 0x0100, 0x0100, CRC(d3c17efc) SHA1(af88340287bd732c91bc5c75970f9de0431b4304) )    /* green component */
	ROM_LOAD( "04d_e-04.bin", 0x0200, 0x0100, CRC(58ba964c) SHA1(1f98f8e484a0462f1a9fadef9e57612a32652599) )    /* blue component */
	ROM_LOAD( "06f_e-05.bin", 0x0300, 0x0100, CRC(35a03579) SHA1(1f1b8c777622a1f5564409c5f3ce69cc68199dae) )    /* char lookup table */
	ROM_LOAD( "l04_e-10.bin", 0x0400, 0x0100, CRC(1dfad87a) SHA1(684844c24e630f46525df97ed67e2e63f7e66d0f) )    /* 32x32 tile lookup table */
	ROM_LOAD( "c04_e-07.bin", 0x0500, 0x0100, CRC(850064e0) SHA1(3884485e91bd82539d0d33f46b7abac60f4c3b1c) )    /* 16x16 tile lookup table */
	ROM_LOAD( "l09_e-11.bin", 0x0600, 0x0100, CRC(2bb68710) SHA1(cfb375316245cb8751e765f163e6acf071dda9ca) )    /* sprite lookup table */
	ROM_LOAD( "l10_e-12.bin", 0x0700, 0x0100, CRC(173184ef) SHA1(f91ecbdc67af1eed6757f660cac8a0e6866c1822) )    /* sprite palette bank */
	ROM_LOAD( "06l_e-06.bin", 0x0800, 0x0100, CRC(712ac508) SHA1(5349d722ab6733afdda65f6e0a98322f0d515e86) )    /* interrupt timing (not used) */
	ROM_LOAD( "k06_e-08.bin", 0x0900, 0x0100, CRC(0eaf5158) SHA1(bafd4108708f66cd7b280e47152b108f3e254fc9) )    /* video timing (not used) */
	ROM_LOAD( "l03_e-09.bin", 0x0a00, 0x0100, CRC(0d968558) SHA1(b376885ac8452b6cbf9ced81b1080bfd570d9b91) )    /* unknown (all 0) */
	ROM_LOAD( "03e_e-01.bin", 0x0b00, 0x0020, CRC(1acee376) SHA1(367094d924f8e0ec36d8310fada4d8143358f697) )    /* unknown (priority?) */
ROM_END



GAME( 1985, exedexes, 0,        exedexes, exedexes, driver_device, 0, ROT270, "Capcom", "Exed Exes", MACHINE_SUPPORTS_SAVE )
GAME( 1985, savgbees, exedexes, exedexes, exedexes, driver_device, 0, ROT270, "Capcom (Memetron license)", "Savage Bees", MACHINE_SUPPORTS_SAVE )
