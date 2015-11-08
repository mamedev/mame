// license:BSD-3-Clause
// copyright-holders:Luca Elia
/***************************************************************************

                            -= Clash Road =-

                    driver by   Luca Elia (l.elia@tin.it)

Main  CPU   :   Z80A

Video Chips :   ?

Sound CPU   :   Z80A

Sound Chips :   Custom (NAMCO)

XTAL        :   18.432 MHz


***************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "audio/wiping.h"
#include "includes/clshroad.h"

void clshroad_state::machine_reset()
{
	flip_screen_set(0);
}


READ8_MEMBER(clshroad_state::input_r)
{
	return  ((~ioport("P1")->read() & (1 << offset)) ? 1 : 0) |
			((~ioport("P2")->read() & (1 << offset)) ? 2 : 0) |
			((~ioport("DSW1")->read() & (1 << offset)) ? 4 : 0) |
			((~ioport("DSW2")->read() & (1 << offset)) ? 8 : 0) ;
}


static ADDRESS_MAP_START( clshroad_map, AS_PROGRAM, 8, clshroad_state )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0x8000, 0x95ff) AM_RAM
	AM_RANGE(0x9600, 0x97ff) AM_RAM AM_SHARE("share1")
	AM_RANGE(0x9800, 0x9dff) AM_RAM
	AM_RANGE(0x9e00, 0x9fff) AM_RAM AM_SHARE("spriteram")
	AM_RANGE(0xa001, 0xa001) AM_WRITENOP    // ? Interrupt related
	AM_RANGE(0xa004, 0xa004) AM_WRITE(flipscreen_w)
	AM_RANGE(0xa100, 0xa107) AM_READ(input_r)
	AM_RANGE(0xa800, 0xafff) AM_RAM_WRITE(vram_1_w) AM_SHARE("vram_1") // Layer 1
	AM_RANGE(0xb000, 0xb003) AM_WRITEONLY AM_SHARE("vregs") // Scroll
	AM_RANGE(0xc000, 0xc7ff) AM_RAM_WRITE(vram_0_w) AM_SHARE("vram_0") // Layer 0
ADDRESS_MAP_END

static ADDRESS_MAP_START( clshroad_sound_map, AS_PROGRAM, 8, clshroad_state )
	AM_RANGE(0x0000, 0x1fff) AM_ROM
	AM_RANGE(0x4000, 0x7fff) AM_DEVWRITE("custom", wiping_sound_device, sound_w)
	AM_RANGE(0x9600, 0x97ff) AM_RAM AM_SHARE("share1")
	AM_RANGE(0xa003, 0xa003) AM_WRITENOP    // ? Interrupt related
ADDRESS_MAP_END



static INPUT_PORTS_START( clshroad )
	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN  )

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START2 )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Coinage ) )      PORT_DIPLOCATION("SW1:1,2")
	PORT_DIPSETTING(    0x00, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Cabinet ) )      PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x18, 0x18, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SW1:4,5") // Damage when falling
	PORT_DIPSETTING(    0x18, DEF_STR( Normal )  )  // 8
	PORT_DIPSETTING(    0x10, DEF_STR( Hard )    )  // A
	PORT_DIPSETTING(    0x08, DEF_STR( Harder )  )  // C
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )  // E
	PORT_DIPNAME( 0x20, 0x20, "Invulnerability (Cheat)")    PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPUNUSED_DIPLOC( 0x40, 0x40, "SW1:7" )        /* Listed as "Unused" */
	PORT_DIPUNUSED_DIPLOC( 0x80, 0x80, "SW1:8" )        /* Listed as "Unused" */

	/* DSW2 is listed as "Unused" */
	PORT_START("DSW2")
/*
first bit OFF is:   0           0   <- value
                    1           1
                    2           2
                    3           3
                    4           4
                    5           5
                    6           6
                    else        FF

But the values seems unused then.
*/
	PORT_DIPUNUSED_DIPLOC( 0x01, 0x01, "SW2:1" )        /* Listed as "Unused" */
	PORT_DIPUNUSED_DIPLOC( 0x02, 0x02, "SW2:2" )        /* Listed as "Unused" */
	PORT_DIPUNUSED_DIPLOC( 0x04, 0x04, "SW2:3" )        /* Listed as "Unused" */
	PORT_DIPUNUSED_DIPLOC( 0x08, 0x08, "SW2:4" )        /* Listed as "Unused" */
	PORT_DIPUNUSED_DIPLOC( 0x10, 0x10, "SW2:5" )        /* Listed as "Unused" */
	PORT_DIPUNUSED_DIPLOC( 0x20, 0x20, "SW2:6" )        /* Listed as "Unused" */
	PORT_DIPUNUSED_DIPLOC( 0x40, 0x40, "SW2:7" )        /* Listed as "Unused" */
	PORT_DIPUNUSED_DIPLOC( 0x80, 0x80, "SW2:8" )        /* Listed as "Unused" */
INPUT_PORTS_END

static INPUT_PORTS_START( firebatl )
	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN  )

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START2 )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x7f, 0x03, DEF_STR( Lives ) )        PORT_DIPLOCATION("SW1:1,2,3,4,5,6,7")
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x01, "2" )
	PORT_DIPSETTING(    0x03, "3" )
	PORT_DIPSETTING(    0x07, "4" )
	PORT_DIPSETTING(    0x0f, "5" )
	PORT_DIPSETTING(    0x1f, "6" )
	PORT_DIPSETTING(    0x3f, "7" )
	PORT_DIPSETTING(    0x7f, "Infinite Lives" )
	PORT_DIPUNKNOWN_DIPLOC( 0x80, 0x80, "SW1:8" )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Coinage ) )      PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(    0x00, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Cabinet ) )      PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Bonus_Life ) )   PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(    0x10, "10K 30K+" )
	PORT_DIPSETTING(    0x00, "20K 30K+" )
	PORT_DIPUNKNOWN_DIPLOC( 0x20, 0x20, "SW2:6" )
	PORT_DIPUNKNOWN_DIPLOC( 0x40, 0x40, "SW2:7" )
	PORT_DIPUNKNOWN_DIPLOC( 0x80, 0x80, "SW2:8" )
INPUT_PORTS_END


static const gfx_layout layout_8x8x2 =
{
	8,8,
	RGN_FRAC(1,1),
	2,
	{ 0, 4 },
	{ STEP4(0,1), STEP4(8,1) },
	{ STEP8(0,8*2) },
	8*8*2
};

static const gfx_layout layout_8x8x4 =
{
	8,8,
	RGN_FRAC(1,2),
	4,
	{ RGN_FRAC(1,2) + 0, RGN_FRAC(1,2) + 4, 0, 4 },
	{ STEP4(0,1), STEP4(8,1) },
	{ STEP8(0,8*2) },
	8*8*2
};

static const gfx_layout layout_16x16x4 =
{
	16,16,
	RGN_FRAC(1,2),
	4,
	{ RGN_FRAC(1,2) + 0, RGN_FRAC(1,2) + 4, 0, 4 },
	{ STEP4(0,1), STEP4(8,1), STEP4(8*8*2+0,1), STEP4(8*8*2+8,1) },
	{ STEP8(0,8*2), STEP8(8*8*2*2,8*2) },
	16*16*2
};

static GFXDECODE_START( firebatl )
	GFXDECODE_ENTRY( "gfx1", 0, layout_16x16x4,   0, 16 ) // [0] Sprites
	GFXDECODE_ENTRY( "gfx2", 0, layout_16x16x4,  16,  1 ) // [1] Layer 0
	GFXDECODE_ENTRY( "gfx3", 0, layout_8x8x2,   512, 64 ) // [2] Layer 1
GFXDECODE_END

static GFXDECODE_START( clshroad )
	GFXDECODE_ENTRY( "gfx1", 0, layout_16x16x4,    0, 16 ) // [0] Sprites
	GFXDECODE_ENTRY( "gfx2", 0, layout_16x16x4, 0x90,  1 ) // [1] Layer 0
	GFXDECODE_ENTRY( "gfx3", 0, layout_8x8x4,      0, 16 ) // [2] Layer 1
GFXDECODE_END



static MACHINE_CONFIG_START( firebatl, clshroad_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, 3000000)   /* ? */
	MCFG_CPU_PROGRAM_MAP(clshroad_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", clshroad_state,  irq0_line_hold)   /* IRQ, no NMI */

	MCFG_CPU_ADD("audiocpu", Z80, 3000000)  /* ? */
	MCFG_CPU_PROGRAM_MAP(clshroad_sound_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", clshroad_state,  irq0_line_hold)   /* IRQ, no NMI */


	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(0x120, 0x100)
	MCFG_SCREEN_VISIBLE_AREA(0, 0x120-1, 0x0+16, 0x100-16-1)
	MCFG_SCREEN_UPDATE_DRIVER(clshroad_state, screen_update)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", firebatl)
	MCFG_PALETTE_ADD("palette", 512+64*4)
	MCFG_PALETTE_INDIRECT_ENTRIES(256)
	MCFG_PALETTE_INIT_OWNER(clshroad_state,firebatl)

	MCFG_VIDEO_START_OVERRIDE(clshroad_state,firebatl)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("custom", WIPING, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_CONFIG_END

static MACHINE_CONFIG_START( clshroad, clshroad_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, XTAL_18_432MHz/4)  /* ? real speed unknown. 3MHz is too low and causes problems */
	MCFG_CPU_PROGRAM_MAP(clshroad_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", clshroad_state,  irq0_line_hold)   /* IRQ, no NMI */

	MCFG_CPU_ADD("audiocpu", Z80, XTAL_18_432MHz/6) /* ? */
	MCFG_CPU_PROGRAM_MAP(clshroad_sound_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", clshroad_state,  irq0_line_hold)   /* IRQ, no NMI */


	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(0x120, 0x100)
	MCFG_SCREEN_VISIBLE_AREA(0, 0x120-1, 0x0+16, 0x100-16-1)
	MCFG_SCREEN_UPDATE_DRIVER(clshroad_state, screen_update)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", clshroad)
	MCFG_PALETTE_ADD("palette", 256)

	MCFG_PALETTE_INIT_OWNER(clshroad_state,clshroad)
	MCFG_VIDEO_START_OVERRIDE(clshroad_state,clshroad)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("custom", WIPING, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_CONFIG_END



ROM_START( firebatl )
	ROM_REGION( 0x10000, "maincpu", 0 )     /* Main Z80 Code */
	ROM_LOAD( "rom01",       0x00000, 0x2000, CRC(10e24ef6) SHA1(b6dae9824eb3cecececbdfdb416a90b1b61ff18d) )
	ROM_LOAD( "rom02",       0x02000, 0x2000, CRC(47f79bee) SHA1(23e64ff69ff5112b0413d12a283ca90cf3642389) )
	ROM_LOAD( "rom03",       0x04000, 0x2000, CRC(693459b9) SHA1(8bba526960f49c9e6c7bca40eb8fbbfc81588660) )

	ROM_REGION( 0x10000, "audiocpu", 0 )        /* Sound Z80 Code */
	ROM_LOAD( "rom04",       0x0000, 0x2000, CRC(5f232d9a) SHA1(d0b9926cb02203f1a1f7fd0d0d7b1fe8eddc6511) )

	ROM_REGION( 0x08000, "gfx1", ROMREGION_INVERT ) /* Sprites */
	ROM_LOAD( "rom14",       0x0000, 0x2000, CRC(36a508a7) SHA1(9b2dede4332d2b8e55e7c5f916d8cf370d7e77fc) )
	ROM_LOAD( "rom13",       0x2000, 0x2000, CRC(a2ec508e) SHA1(a6dd7b9729f320ed3a28e0cd8ea7b26c2a639e1a) )
	ROM_LOAD( "rom12",       0x4000, 0x2000, CRC(f80ece92) SHA1(2cc4317b2c58be48dc285bb3a667863e2ca8d5b7) )
	ROM_LOAD( "rom11",       0x6000, 0x2000, CRC(b293e701) SHA1(9dacaa9897d91dc465f2c1907804fed9bfb7207b) )

	ROM_REGION( 0x08000, "gfx2", ROMREGION_INVERT ) /* Layer 0 */
	ROM_LOAD( "rom09",       0x0000, 0x2000, CRC(77ea3e39) SHA1(c897664bd4f4b163a557d39d12374dae08a0a0c2) )
	ROM_LOAD( "rom08",       0x2000, 0x2000, CRC(1b7585dd) SHA1(e402c879c5651bf0fa21dcf1ff3c4b7bf690cbaa) )
	ROM_LOAD( "rom07",       0x4000, 0x2000, CRC(e3ec9825) SHA1(ea266683a48e8515d40ed077fd55d15a1859c942) )
	ROM_LOAD( "rom06",       0x6000, 0x2000, CRC(d29fab5f) SHA1(de5f8d57d3dd9090e6c056ff7f1ab0bb59630863) )

	ROM_REGION( 0x01000, "gfx3", 0 )    /* Layer 1 */
	ROM_LOAD( "rom15",       0x0000, 0x1000, CRC(8b5464d6) SHA1(e65acd280c0d9776cb80073241cf260b76ff0ca6) )

	ROM_REGION( 0x0a20, "proms", 0 )
	ROM_LOAD( "prom6.bpr",   0x0000, 0x0100, CRC(b117d22c) SHA1(357efed6597757907077a7e5130bfa643d5dd197) ) /* palette red? */
	ROM_LOAD( "prom7.bpr",   0x0100, 0x0100, CRC(9b6b4f56) SHA1(7fd726a20fce40b8ba4b8ef05fb51a85ad9fd282) ) /* palette green? */
	ROM_LOAD( "prom8.bpr",   0x0200, 0x0100, CRC(67cb68ae) SHA1(9b54c7e51d8db0d8699723173709f04dd2fdfa77) ) /* palette blue? */
	ROM_LOAD( "prom9.bpr",   0x0300, 0x0100, CRC(dd015b80) SHA1(ce45577204cfbbe623121c1bd99a190464ae7895) ) /* char lookup table msb? */
	ROM_LOAD( "prom10.bpr",  0x0400, 0x0100, CRC(71b768c7) SHA1(3d8c106758d279daf8e989d4c1bb72de3419d2d6) ) /* char lookup table lsb? */
	ROM_LOAD( "prom4.bpr",   0x0500, 0x0100, CRC(06523b81) SHA1(0042c364fd2fabd6b04cb2d59a71a7e6deb90ab3) ) /* unknown */
	ROM_LOAD( "prom5.bpr",   0x0600, 0x0100, CRC(75ea8f70) SHA1(1a2c478e7b87fa7f8725a3d1ff06c5c9422dd524) ) /* unknown */
	ROM_LOAD( "prom11.bpr",  0x0700, 0x0100, CRC(ba42a582) SHA1(2e8f3dab82a34078b866e9875978e83fef045f86) ) /* unknown */
	ROM_LOAD( "prom12.bpr",  0x0800, 0x0100, CRC(f2540c51) SHA1(126f698eb65e54fa16a1abfa5b40b0161cb66254) ) /* unknown */
	ROM_LOAD( "prom13.bpr",  0x0900, 0x0100, CRC(4e2a2781) SHA1(7be2e066499ea0af76f6ae926fe87e02f8c36a6f) ) /* unknown */
	ROM_LOAD( "prom1.bpr",   0x0a00, 0x0020, CRC(1afc04f0) SHA1(38207cf3e15bac7034ac06469b95708d22b57da4) ) /* timing? (on the cpu board) */

	ROM_REGION( 0x2000, "samples", 0 )  /* samples */
	ROM_LOAD( "rom05",       0x0000, 0x2000, CRC(21544cd6) SHA1(b9644ab3c4393cd2669d2b5b3c80d7a9f1c91ca6) )

	ROM_REGION( 0x0200, "soundproms", 0 )   /* 4bit->8bit sample expansion PROMs */
	ROM_LOAD( "prom3.bpr",   0x0000, 0x0100, CRC(bd2c080b) SHA1(9782bb5001e96db56bc29df398187f700bce4f8e) ) /* low 4 bits */
	ROM_LOAD( "prom2.bpr",   0x0100, 0x0100, CRC(4017a2a6) SHA1(dadef2de7a1119758c8e6d397aa42815b0218889) ) /* high 4 bits */
ROM_END

ROM_START( clshroad )
	ROM_REGION( 0x10000, "maincpu", 0 )     /* Main Z80 Code */
	ROM_LOAD( "clashr3.bin", 0x0000, 0x8000, CRC(865c32ae) SHA1(e5cdd2d624fe6dc8bd6bebf2bd1c79d287408c63) )

	ROM_REGION( 0x10000, "audiocpu", 0 )        /* Sound Z80 Code */
	ROM_LOAD( "clashr2.bin", 0x0000, 0x2000, CRC(e6389ec1) SHA1(6ec94d5e389e9104f40fc48df6f15674415851c0) )

	ROM_REGION( 0x08000, "gfx1", ROMREGION_INVERT ) /* Sprites */
	ROM_LOAD( "clashr5.bin", 0x0000, 0x4000, CRC(094858b8) SHA1(a19f79cb665bbb1e25a94e9dd09a9e99f553afe8) )
	ROM_LOAD( "clashr6.bin", 0x4000, 0x4000, CRC(daa1daf3) SHA1(cc24c97c9950adc0041f68832774e40c87d1d4b2) )

	ROM_REGION( 0x08000, "gfx2", ROMREGION_INVERT ) /* Layer 0 */
	ROM_LOAD( "clashr8.bin", 0x0000, 0x4000, CRC(cbb66719) SHA1(2497575f84a956bc2b9e4c3f2c71ae42d036355e) )
	ROM_LOAD( "clashr9.bin", 0x4000, 0x4000, CRC(c15e8eed) SHA1(3b1e7fa014d176a01d5f9214051b0c8cc5556684) )

	ROM_REGION( 0x04000, "gfx3", ROMREGION_INVERT)  /* Layer 1 */
	ROM_LOAD( "clashr4.bin", 0x0000, 0x2000, CRC(664201d9) SHA1(4eb85306f0c9683d0e0cf787f6389df8fe4a3d9d) )
	ROM_LOAD( "clashr7.bin", 0x2000, 0x2000, CRC(97973030) SHA1(cca7a9d2751add7f6dd9bac83f7f63ece8021dbc) )

	ROM_REGION( 0x0b40, "proms", 0 )
	ROM_LOAD( "82s129.6", 0x0000, 0x0100, CRC(38f443da) SHA1(a015217508b18eb3f1987cd5b53f31608b13de08) )    /* r */
	ROM_LOAD( "82s129.7", 0x0100, 0x0100, CRC(977fab0c) SHA1(78e7b4f1e9891d2d9cf1e1ec0c4f59a311cef1c5) )    /* g */
	ROM_LOAD( "82s129.8", 0x0200, 0x0100, CRC(ae7ae54d) SHA1(d7d4682e437f2f7adb7fceb813437c06f27f2711) )    /* b */
	/* all other proms that firebatl has are missing */
	ROM_LOAD( "clashrd.a2",  0x0900, 0x0100, CRC(4e2a2781) SHA1(7be2e066499ea0af76f6ae926fe87e02f8c36a6f) ) /* unknown */
	ROM_LOAD( "clashrd.g4",  0x0a00, 0x0020, CRC(1afc04f0) SHA1(38207cf3e15bac7034ac06469b95708d22b57da4) ) /* timing? */
	ROM_LOAD( "clashrd.b11", 0x0a20, 0x0020, CRC(d453f2c5) SHA1(7fdc5bf59bad9e8f00e970565ff6f6b3773541db) ) /* unknown (possibly bad dump) */
	ROM_LOAD( "clashrd.g10", 0x0a40, 0x0100, CRC(73afefd0) SHA1(d14c5490c5b174d54043bfdf5c6fb675e67492e7) ) /* unknown (possibly bad dump) */

	ROM_REGION( 0x2000, "samples", 0 )  /* samples */
	ROM_LOAD( "clashr1.bin", 0x0000, 0x2000, CRC(0d0a8068) SHA1(529878d0c5f078590e07ec0fffc27b212843c0ad) )

	ROM_REGION( 0x0200, "soundproms", 0 )   /* 4bit->8bit sample expansion PROMs */
	ROM_LOAD( "clashrd.g8",  0x0000, 0x0100, CRC(bd2c080b) SHA1(9782bb5001e96db56bc29df398187f700bce4f8e) ) /* low 4 bits */
	ROM_LOAD( "clashrd.g7",  0x0100, 0x0100, CRC(4017a2a6) SHA1(dadef2de7a1119758c8e6d397aa42815b0218889) ) /* high 4 bits */
ROM_END

ROM_START( clshroads )
	ROM_REGION( 0x10000, "maincpu", 0 )     /* Main Z80 Code */
	ROM_LOAD( "cr-3",  0x0000, 0x8000, CRC(23559df2) SHA1(41a08a4fbad3da1898226e2ca1956a9f7c8f94b0) )

	ROM_REGION( 0x10000, "audiocpu", 0 )        /* Sound Z80 Code */
	ROM_LOAD( "clashr2.bin", 0x0000, 0x2000, CRC(e6389ec1) SHA1(6ec94d5e389e9104f40fc48df6f15674415851c0) )

	ROM_REGION( 0x08000, "gfx1", ROMREGION_INVERT ) /* Sprites */
	ROM_LOAD( "cr-12", 0x0000, 0x2000, CRC(e5aa4c46) SHA1(c0ed717e263aca2d0ec8a078f0aa3377357b9e3d) )
	ROM_LOAD( "cr-11", 0x2000, 0x2000, CRC(7fc11c7c) SHA1(e798c4abe87fd701f250625ae6545ab00bcfbef5) )
	ROM_LOAD( "cr-10", 0x4000, 0x2000, CRC(6b1293b7) SHA1(3219c03e87be0f53d4556a45d82278fc712f4d0b) )
	ROM_LOAD( "cr-9",  0x6000, 0x2000, CRC(d219722c) SHA1(e68a0883113db43a878a5529fa8deb7816573e35) )

	ROM_REGION( 0x08000, "gfx2", ROMREGION_INVERT ) /* Layer 0 */
	ROM_LOAD( "cr-7",  0x0000, 0x2000, CRC(e8aa7ac3) SHA1(12f4f1041001ce6e77e9b0c691663f4a20969eaa) )
	ROM_LOAD( "cr-6",  0x2000, 0x2000, CRC(037be475) SHA1(72a8c1da210239ecac96ea22e54a5d193f32ad83) )
	ROM_LOAD( "cr-5",  0x4000, 0x2000, CRC(a4151734) SHA1(1ccf68270bbfd557cd8cca3f7f36e9e2de7e94e6) )
	ROM_LOAD( "cr-4",  0x6000, 0x2000, CRC(5ef24757) SHA1(4c6a06fdadb0b52f62148642e0416c4f60c8048b) )

	ROM_REGION( 0x04000, "gfx3", ROMREGION_INVERT)  /* Layer 1 */
	ROM_LOAD( "cr-13", 0x0000, 0x2000, CRC(012a6412) SHA1(ae9757e56f896e4158e6af7dc12eb6c2a3755f4d) )
	ROM_LOAD( "cr-8",  0x2000, 0x2000, CRC(3c2b816c) SHA1(3df3d9f49475ccfbb0792d98c3d12c8fd15034bc) )

	ROM_REGION( 0x0b40, "proms", 0 )
	ROM_LOAD( "82s129.6", 0x0000, 0x0100, CRC(38f443da) SHA1(a015217508b18eb3f1987cd5b53f31608b13de08) )    /* r */
	ROM_LOAD( "82s129.7", 0x0100, 0x0100, CRC(977fab0c) SHA1(78e7b4f1e9891d2d9cf1e1ec0c4f59a311cef1c5) )    /* g */
	ROM_LOAD( "82s129.8", 0x0200, 0x0100, CRC(ae7ae54d) SHA1(d7d4682e437f2f7adb7fceb813437c06f27f2711) )    /* b */
	/* all other proms that firebatl has are missing */
	ROM_LOAD( "clashrd.a2",  0x0900, 0x0100, CRC(4e2a2781) SHA1(7be2e066499ea0af76f6ae926fe87e02f8c36a6f) ) /* unknown */
	ROM_LOAD( "clashrd.g4",  0x0a00, 0x0020, CRC(1afc04f0) SHA1(38207cf3e15bac7034ac06469b95708d22b57da4) ) /* timing? */
	ROM_LOAD( "clashrd.b11", 0x0a20, 0x0020, CRC(d453f2c5) SHA1(7fdc5bf59bad9e8f00e970565ff6f6b3773541db) ) /* unknown (possibly bad dump) */
	ROM_LOAD( "clashrd.g10", 0x0a40, 0x0100, CRC(73afefd0) SHA1(d14c5490c5b174d54043bfdf5c6fb675e67492e7) ) /* unknown (possibly bad dump) */

	ROM_REGION( 0x2000, "samples", 0 )  /* samples */
	ROM_LOAD( "clashr1.bin", 0x0000, 0x2000, CRC(0d0a8068) SHA1(529878d0c5f078590e07ec0fffc27b212843c0ad) )

	ROM_REGION( 0x0200, "soundproms", 0 )   /* 4bit->8bit sample expansion PROMs */
	ROM_LOAD( "clashrd.g8",  0x0000, 0x0100, CRC(bd2c080b) SHA1(9782bb5001e96db56bc29df398187f700bce4f8e) ) /* low 4 bits */
	ROM_LOAD( "clashrd.g7",  0x0100, 0x0100, CRC(4017a2a6) SHA1(dadef2de7a1119758c8e6d397aa42815b0218889) ) /* high 4 bits */
ROM_END

// this set came from a bootleg board, but I believe it to be original for the following reason:
//  the ONLY difference between this and the parent set is the Wood Place string, however, in the parent
//  set the Wood Place string is padded with several 0x20 (Space) characters to fit the same number of bytes
//  in which the Data East Corporation string fits, suggesting that they always planned to put it there.
ROM_START( clshroadd )
	ROM_REGION( 0x10000, "maincpu", 0 )     /* Main Z80 Code */
	ROM_LOAD( "crdeco-3.bin",  0x0000, 0x8000, CRC(1d54195c) SHA1(4b1d7d333707b5ebd57572742eb74df5abe8a70d) )

	ROM_REGION( 0x10000, "audiocpu", 0 )        /* Sound Z80 Code */
	ROM_LOAD( "clashr2.bin", 0x0000, 0x2000, CRC(e6389ec1) SHA1(6ec94d5e389e9104f40fc48df6f15674415851c0) )

	ROM_REGION( 0x08000, "gfx1", ROMREGION_INVERT ) /* Sprites */
	ROM_LOAD( "clashr5.bin", 0x0000, 0x4000, CRC(094858b8) SHA1(a19f79cb665bbb1e25a94e9dd09a9e99f553afe8) )
	ROM_LOAD( "clashr6.bin", 0x4000, 0x4000, CRC(daa1daf3) SHA1(cc24c97c9950adc0041f68832774e40c87d1d4b2) )

	ROM_REGION( 0x08000, "gfx2", ROMREGION_INVERT ) /* Layer 0 */
	ROM_LOAD( "clashr8.bin", 0x0000, 0x4000, CRC(cbb66719) SHA1(2497575f84a956bc2b9e4c3f2c71ae42d036355e) )
	ROM_LOAD( "clashr9.bin", 0x4000, 0x4000, CRC(c15e8eed) SHA1(3b1e7fa014d176a01d5f9214051b0c8cc5556684) )

	ROM_REGION( 0x04000, "gfx3", ROMREGION_INVERT)  /* Layer 1 */
	ROM_LOAD( "clashr4.bin", 0x0000, 0x2000, CRC(664201d9) SHA1(4eb85306f0c9683d0e0cf787f6389df8fe4a3d9d) )
	ROM_LOAD( "clashr7.bin", 0x2000, 0x2000, CRC(97973030) SHA1(cca7a9d2751add7f6dd9bac83f7f63ece8021dbc) )

	ROM_REGION( 0x0b40, "proms", 0 )
	ROM_LOAD( "82s129.6", 0x0000, 0x0100, CRC(38f443da) SHA1(a015217508b18eb3f1987cd5b53f31608b13de08) )    /* r */
	ROM_LOAD( "82s129.7", 0x0100, 0x0100, CRC(977fab0c) SHA1(78e7b4f1e9891d2d9cf1e1ec0c4f59a311cef1c5) )    /* g */
	ROM_LOAD( "82s129.8", 0x0200, 0x0100, CRC(ae7ae54d) SHA1(d7d4682e437f2f7adb7fceb813437c06f27f2711) )    /* b */
	/* all other proms that firebatl has are missing */
	ROM_LOAD( "clashrd.a2",  0x0900, 0x0100, CRC(4e2a2781) SHA1(7be2e066499ea0af76f6ae926fe87e02f8c36a6f) ) /* unknown */
	ROM_LOAD( "clashrd.g4",  0x0a00, 0x0020, CRC(1afc04f0) SHA1(38207cf3e15bac7034ac06469b95708d22b57da4) ) /* timing? */
	ROM_LOAD( "clashrd.b11", 0x0a20, 0x0020, CRC(d453f2c5) SHA1(7fdc5bf59bad9e8f00e970565ff6f6b3773541db) ) /* unknown (possibly bad dump) */
	ROM_LOAD( "clashrd.g10", 0x0a40, 0x0100, CRC(73afefd0) SHA1(d14c5490c5b174d54043bfdf5c6fb675e67492e7) ) /* unknown (possibly bad dump) */

	ROM_REGION( 0x2000, "samples", 0 )  /* samples */
	ROM_LOAD( "clashr1.bin", 0x0000, 0x2000, CRC(0d0a8068) SHA1(529878d0c5f078590e07ec0fffc27b212843c0ad) )

	ROM_REGION( 0x0200, "soundproms", 0 )   /* 4bit->8bit sample expansion PROMs */
	ROM_LOAD( "clashrd.g8",  0x0000, 0x0100, CRC(bd2c080b) SHA1(9782bb5001e96db56bc29df398187f700bce4f8e) ) /* low 4 bits */
	ROM_LOAD( "clashrd.g7",  0x0100, 0x0100, CRC(4017a2a6) SHA1(dadef2de7a1119758c8e6d397aa42815b0218889) ) /* high 4 bits */
ROM_END

DRIVER_INIT_MEMBER(clshroad_state,firebatl)
{
/*
Pugsy> firebatl:0:05C6:C3:100:Fix the Game:It's a hack but seems to make it work!
Pugsy> firebatl:0:05C7:8D:600:Fix the Game (2/3)
Pugsy> firebatl:0:05C8:23:600:Fix the Game (3/3)

without this the death sequence never ends so the game is unplayable after you
die once, it would be nice to avoid the hack however

*/
	UINT8 *ROM = memregion("maincpu")->base();

	ROM[0x05C6] = 0xc3;
	ROM[0x05C7] = 0x8d;
	ROM[0x05C8] = 0x23;
}

GAME( 1984, firebatl, 0,        firebatl, firebatl, clshroad_state, firebatl, ROT90, "Taito", "Fire Battle", MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )
GAME( 1986, clshroad, 0,        clshroad, clshroad, driver_device, 0,        ROT0,  "Wood Place Inc.", "Clash-Road", MACHINE_SUPPORTS_SAVE )
GAME( 1986, clshroads,clshroad, clshroad, clshroad, driver_device, 0,        ROT0,  "Wood Place Inc. (Status Game Corp. license)", "Clash-Road (Status license)", MACHINE_SUPPORTS_SAVE )
GAME( 1986, clshroadd,clshroad, clshroad, clshroad, driver_device, 0,        ROT0,  "Wood Place Inc. (Data East license)", "Clash-Road (Data East license)", MACHINE_SUPPORTS_SAVE )
