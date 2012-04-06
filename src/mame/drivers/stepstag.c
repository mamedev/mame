/*  Jaleco 'Stepping Stage'

*************************************************************************
 Naibo added:

 A PC computer(Harddisk not dumped yet) + Two 68000 based board set.

 One 68000 drives 3 screens, another handles players input.
*************************************************************************

 dump is incomplete, these are leftovers from an upgrade
 music roms are missing at least

 is it a 3 screen game (1 horizontal, 2 vertical) ?

 TODO:
 - clearly derived from Tetris Plus 2 HW
 - NVRAM is unchecked

 */

#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "rendlay.h"
#include "stepstag.lh"
#include "includes/tetrisp2.h"
#include "machine/nvram.h"


class stepstag_state : public tetrisp2_state
{
public:
	stepstag_state(const machine_config &mconfig, device_type type, const char *tag)
		: tetrisp2_state(mconfig, type, tag) { }

	DECLARE_READ16_MEMBER(stepstag_ip_1_word_r);
	DECLARE_READ16_MEMBER(unknown_read_0xc00000);
	DECLARE_READ16_MEMBER(unknown_read_0xd00000);
	DECLARE_READ16_MEMBER(unknown_read_0xffff00);
};

READ16_MEMBER(stepstag_state::stepstag_ip_1_word_r)
{
	return	( input_port_read(machine(), "SYSTEM") &  0xfcff ) |
			(           machine().rand() & ~0xfcff ) |
			(      1 << (8 + (machine().rand()&1)) );
}

READ16_MEMBER(stepstag_state::unknown_read_0xc00000)
{
	return machine().rand();
}

READ16_MEMBER(stepstag_state::unknown_read_0xd00000)
{
	return machine().rand();
}

READ16_MEMBER(stepstag_state::unknown_read_0xffff00)
{
	return machine().rand();
}

static ADDRESS_MAP_START( stepstag_map, AS_PROGRAM, 16, stepstag_state )
	AM_RANGE(0x000000, 0x0fffff) AM_ROM
	AM_RANGE(0x100000, 0x103fff) AM_RAM AM_SHARE("spriteram")			// Object RAM
	AM_RANGE(0x104000, 0x107fff) AM_RAM															// Spare Object RAM
	AM_RANGE(0x108000, 0x10ffff) AM_RAM															// Work RAM
	AM_RANGE(0x200000, 0x23ffff) AM_READWRITE8(tetrisp2_priority_r, rockn_priority_w, 0x00ff)		// Priority
	AM_RANGE(0x300000, 0x31ffff) AM_RAM_WRITE(tetrisp2_palette_w) AM_SHARE("paletteram")		// Palette
	AM_RANGE(0x400000, 0x403fff) AM_RAM_WRITE(tetrisp2_vram_fg_w) AM_BASE(m_vram_fg)	// Foreground
	AM_RANGE(0x404000, 0x407fff) AM_RAM_WRITE(tetrisp2_vram_bg_w) AM_BASE(m_vram_bg)	// Background
	AM_RANGE(0x408000, 0x409fff) AM_RAM															// ???
	AM_RANGE(0x500000, 0x50ffff) AM_RAM															// Line
	AM_RANGE(0x600000, 0x60ffff) AM_RAM_WRITE(tetrisp2_vram_rot_w) AM_BASE(m_vram_rot)	// Rotation
	AM_RANGE(0x900000, 0x903fff) AM_READWRITE(tetrisp2_nvram_r, tetrisp2_nvram_w) AM_BASE(m_nvram) AM_SHARE("nvram")	// NVRAM
	AM_RANGE(0x904000, 0x907fff) AM_READWRITE(tetrisp2_nvram_r, tetrisp2_nvram_w)				// NVRAM (mirror)
//  AM_RANGE(0xa00000, 0xa7ffff) AM_READ(unknown_read_0xc00000 ) // presumably comms with slave CPU
	AM_RANGE(0xb00000, 0xb00001) AM_WRITENOP								// Coin Counter plus other things
	AM_RANGE(0xb20000, 0xb20001) AM_WRITENOP													// protection related?
	AM_RANGE(0xb40000, 0xb4000b) AM_WRITEONLY AM_BASE(m_scroll_fg)						// Foreground Scrolling
	AM_RANGE(0xb40010, 0xb4001b) AM_WRITEONLY AM_BASE(m_scroll_bg)						// Background Scrolling
	AM_RANGE(0xb4003e, 0xb4003f) AM_WRITENOP													// scr_size
	AM_RANGE(0xb60000, 0xb6002f) AM_WRITEONLY AM_BASE(m_rotregs)						// Rotation Registers
	AM_RANGE(0xba0000, 0xba001f) AM_WRITE(tetrisp2_systemregs_w)								// system param
	AM_RANGE(0xba001e, 0xba001f) AM_WRITENOP													// Lev 2 irq ack
	AM_RANGE(0xbe0002, 0xbe0003) AM_READ_PORT("PLAYERS")										// Inputs
	AM_RANGE(0xbe0004, 0xbe0005) AM_READ(stepstag_ip_1_word_r)									// Inputs & protection
	AM_RANGE(0xbe0008, 0xbe0009) AM_READ_PORT("DSW")											// Inputs
	AM_RANGE(0xbe000a, 0xbe000b) AM_READNOP //watchdog
ADDRESS_MAP_END




static ADDRESS_MAP_START( stepstag_sub_map, AS_PROGRAM, 16, stepstag_state )
	AM_RANGE(0x000000, 0x0fffff) AM_ROM
	AM_RANGE(0x200000, 0x20ffff) AM_RAM

	AM_RANGE(0x300000, 0x33ffff) AM_RAM

	AM_RANGE(0x400000, 0x43ffff) AM_RAM

	AM_RANGE(0x500000, 0x53ffff) AM_RAM

	AM_RANGE(0x700000, 0x700001) AM_WRITENOP //??
	AM_RANGE(0x700002, 0x700003) AM_WRITENOP //??
	AM_RANGE(0x700004, 0x700005) AM_WRITENOP //??
	AM_RANGE(0x700006, 0x700007) AM_WRITENOP //??

	AM_RANGE(0x800000, 0x87ffff) AM_RAM
	AM_RANGE(0x880000, 0x880001) AM_WRITENOP //??
	AM_RANGE(0x900000, 0x97ffff) AM_RAM
	AM_RANGE(0x980000, 0x980001) AM_WRITENOP //??
	AM_RANGE(0xa00000, 0xa7ffff) AM_RAM
	AM_RANGE(0xa80000, 0xa80001) AM_WRITENOP //??
	AM_RANGE(0xc00000, 0xc00001) AM_READ(unknown_read_0xc00000) AM_WRITENOP //??
	AM_RANGE(0xd00000, 0xd00001) AM_READ(unknown_read_0xd00000)
	AM_RANGE(0xf00000, 0xf00001) AM_WRITENOP //??
	AM_RANGE(0xffff00, 0xffff01) AM_READ(unknown_read_0xffff00)
ADDRESS_MAP_END

/* TODO: inputs are copied from Tetris Plus 2, they needs to be changed accordingly */
static INPUT_PORTS_START( stepstag )
	PORT_START("PLAYERS") /*$be0002.w*/
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(2)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("SYSTEM") /*$be0004.w*/
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_START2   )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_SPECIAL  )	/* ?*/
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_SPECIAL  )	/* ?*/
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW") //$be0008.w
	PORT_DIPNAME( 0x0007, 0x0007, DEF_STR( Coin_A ) ) PORT_DIPLOCATION("SW1:1,2,3")
	PORT_DIPSETTING(      0x0000, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0003, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0007, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0006, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0005, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( 1C_4C ) )
	PORT_DIPNAME( 0x0038, 0x0038, DEF_STR( Coin_B ) ) PORT_DIPLOCATION("SW1:4,5,6")
	PORT_DIPSETTING(      0x0000, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0018, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0038, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0030, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0028, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( 1C_4C ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Free_Play ) ) PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_SERVICE_DIPLOC(  0x0080, IP_ACTIVE_LOW, "SW1:8" )
	PORT_DIPNAME( 0x0300, 0x0300, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(      0x0000, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x0300, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0100, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x0400, 0x0400, "Vs Mode Rounds" ) PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(      0x0000, "1" )
	PORT_DIPSETTING(      0x0400, "3" )
	PORT_DIPNAME( 0x0800, 0x0000, DEF_STR( Language ) ) PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(      0x0800, DEF_STR( Japanese ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( English ) )
	PORT_DIPNAME( 0x1000, 0x1000, "F.B.I Logo" ) PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(      0x1000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x2000, "Voice" ) PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000, 0x4000, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000, 0x8000, DEF_STR( Flip_Screen ) ) PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(      0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
INPUT_PORTS_END

/* 8x8x8 tiles */
static const gfx_layout layout_8x8x8 =
{
	8,8,
	RGN_FRAC(1,1),
	8,
	{ STEP8(0,1) },
	{ STEP8(0,8) },
	{ STEP8(0,8*8) },
	8*8*8
};

/* sprites are contained in 256x256 "tiles" */
static GFXLAYOUT_RAW( spritelayout, 256, 256, 256*8, 256*256*8 )


static GFXDECODE_START( stepstag )
	GFXDECODE_ENTRY( "gfx1", 0, spritelayout, 0x0000, 0x10 ) // [0] Sprites
	GFXDECODE_ENTRY( "gfx2", 0, layout_8x8x8, 0x1000, 0x10 ) // [1] Background
	GFXDECODE_ENTRY( "gfx3", 0, layout_8x8x8, 0x2000, 0x10 ) // [2] Rotation
	GFXDECODE_ENTRY( "gfx4", 0, layout_8x8x8, 0x6000, 0x10 ) // [3] Foreground
	GFXDECODE_ENTRY( "gfx5", 0, layout_8x8x8, 0x0000, 0x10 ) // [0] Sprites
	GFXDECODE_ENTRY( "gfx6", 0, layout_8x8x8, 0x1000, 0x10 ) // [1] Background
	GFXDECODE_ENTRY( "gfx7", 0, layout_8x8x8, 0x2000, 0x10 ) // [2] Rotation
	GFXDECODE_ENTRY( "gfx8", 0, layout_8x8x8, 0x6000, 0x10 ) // [3] Foreground
GFXDECODE_END

static SCREEN_UPDATE_IND16( stepstag )
{
	return 0;
}

static MACHINE_CONFIG_START( stepstag, stepstag_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68000, 16000000 ) //??
	MCFG_CPU_PROGRAM_MAP(stepstag_map)
	MCFG_CPU_VBLANK_INT("screen", irq2_line_hold)

	MCFG_CPU_ADD("sub", M68000, 16000000 ) //??
	MCFG_CPU_PROGRAM_MAP(stepstag_sub_map)
	MCFG_CPU_VBLANK_INT("screen", irq4_line_hold) // 4 & 6 valid

	MCFG_NVRAM_ADD_0FILL("nvram")

	/* video hardware */
	MCFG_SCREEN_ADD("lscreen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(64*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 0*8, 32*8-1)
	MCFG_SCREEN_UPDATE_STATIC(stepstag)

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(0x140, 0xe0)
	MCFG_SCREEN_VISIBLE_AREA(0, 0x140-1, 0, 0xe0-1)
	MCFG_SCREEN_UPDATE_STATIC(tetrisp2)

	MCFG_SCREEN_ADD("rscreen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(64*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 0*8, 32*8-1)
	MCFG_SCREEN_UPDATE_STATIC(stepstag)

	MCFG_PALETTE_LENGTH(0x8000)

	MCFG_VIDEO_START(tetrisp2)
	MCFG_GFXDECODE(stepstag)

	MCFG_DEFAULT_LAYOUT(layout_stepstag)
MACHINE_CONFIG_END


ROM_START( stepstag )
	ROM_REGION( 0x100000, "maincpu", 0 ) /* 68k */
	ROM_LOAD16_BYTE( "vj98344ver11.1", 0x00001, 0x80000, CRC(aedcb225) SHA1(f167c390e79ffbf7c019c326384ae656ae8b7d13) )
	ROM_LOAD16_BYTE( "vj98344ver11.4", 0x00000, 0x80000, CRC(391ca913) SHA1(2cc329aa6419f8a0d7e0fb8a9f4c2b8ca25197b3) )

	ROM_REGION( 0x100000, "sub", 0 ) /* 68k */
	ROM_LOAD16_BYTE( "vj98348ver11.11", 0x00000, 0x80000, CRC(29b7f848) SHA1(c4d89e5c9be622b2d9038c359a5f65ce0dd461b0) )
	ROM_LOAD16_BYTE( "vj98348ver11.14", 0x00001, 0x80000, CRC(e3314c6c) SHA1(61b0e9f9d0126d9f475304866a03cfa21701d9aa) )

	ROM_REGION( 0x400000, "gfx1", 0 ) /* sprites, TRUSTED */
	ROM_LOAD( "mr99001-06", 0x00000, 0x400000, CRC(cfa27c93) SHA1(a0837877736e8e898f3acc64bc87ee0cc4d9f243) )

	ROM_REGION( 0x400000, "gfx2", 0 ) /* */
	ROM_LOAD( "mr99001-03", 0x00000, 0x400000, CRC(40fee0df) SHA1(94c3567e82f8039b3169bf4dcb1fcd9e39c6eb27) )

	ROM_REGION( 0x400000, "gfx3", 0 ) /* */
	ROM_LOAD( "mr99001-04", 0x00000, 0x400000, CRC(d6837981) SHA1(56709d73304f0b186c70844ae96f73400b541609) )

	ROM_REGION( 0x400000, "gfx4", 0 ) /* FG tiles, TRUSTED */
	ROM_LOAD( "mr99001-05", 0x00000, 0x400000, CRC(3958473b) SHA1(12279a587263290945744b22aafb80460eea77f7) )

	ROM_REGION( 0x400000, "gfx5", 0 ) /* vertical screen rom */
	ROM_LOAD( "mr99001-02", 0x00000, 0x400000, CRC(12c65d86) SHA1(7fe5853fa3ba086f8da15702b126eb13c6ea30a9) )

	ROM_REGION( 0x800000, "gfx6", 0 ) /* vertical screen roms */
	ROM_LOAD( "mr99001-01", 0x00000, 0x400000,  CRC(aa92cebf) SHA1(2ccc0d2ef9bc92c27f0a625819154bbcf9cfde0c) )
	ROM_LOAD( "s.s.s._vj-98348_3_pr99021-01", 0x400000, 0x400000, CRC(e0fbc6f1) SHA1(7ca4507702f3f81bb9de3f9b5d270d379e439633) )

	ROM_REGION( 0x400000, "gfx7", 0 ) /* */
	ROM_LOAD( "s.s.s._vj-98348_19_pr99021-02", 0x00000, 0x400000, CRC(2d98da1a) SHA1(b09375fa1b4b2e0794632d6e237459009f40310d) )

	ROM_REGION( 0x400000, "gfx8", 0 ) /* */
	ROM_LOAD( "s.s.s._vj-98348_26_pr99021-01", 0x000000, 0x400000, CRC(fefb3777) SHA1(df624e105ab1dea52317e318ad29caa02b900788) )

	DISK_REGION( "disks" )
	DISK_IMAGE("stepstag", 0, NO_DUMP)
ROM_END

ROM_START( step3 )
ROM_REGION( 0x100000, "maincpu", 0 ) /* 68k */
	ROM_LOAD16_BYTE( "vj98344ver11.1", 0x00001, 0x80000, NO_DUMP )
	ROM_LOAD16_BYTE( "vj98344ver11.4", 0x00000, 0x80000, NO_DUMP )
	// c'est la programme de stepstag (avoir besoin de modifications, numero de chansons par example)

	ROM_REGION( 0x100000, "sub", 0 ) /* 68k */
	ROM_LOAD16_BYTE( "vj98348_step3_11_v1.1", 0x00000, 0x80000, CRC(9c36aef5) SHA1(bbac48c2c7949a6f8a6ec83515e94a343c88d1b6) )
	ROM_LOAD16_BYTE( "vj98348_step3_14_v1.1", 0x00001, 0x80000, CRC(b86be557) SHA1(49dbd6ef1c50adcf3386d5423da8ae7685649c46) )

	ROM_REGION( 0x1000000, "gfx1", 0 ) /* */
	ROM_LOAD( "mr9930-01.ic2", 0x000000, 0x400000, CRC(9e3e054e) SHA1(06a4fa76cb83dbe9d565d5ccd0a5ecc5067887c9) )

	ROM_REGION( 0x1000000, "gfx2", 0 ) /* */
	ROM_LOAD( "mr9930-02.ic3", 0x000000, 0x400000, CRC(b23c29f4) SHA1(a7b10a3a9af43db319baf8633bb3728120960923) )

	ROM_REGION( 0x1000000, "gfx3", 0 ) /* */
	ROM_LOAD( "mr9930-03.ic4", 0x000000, 0x400000, CRC(9a5d070f) SHA1(b4668b4f299033140a2c56499cc2712ba111cb57) )

	ROM_REGION( 0x1000000, "gfx4", 0 ) /* screen centre */
	ROM_LOAD( "mr99030-04.ic17", 0x000000, 0x400000, CRC(3eac3591) SHA1(3b294e94af23fd92fdf51d2c9c43f60d2ebd1688) )

	ROM_REGION( 0x1000000, "gfx5", 0 ) /* */
	ROM_LOAD( "mr99030-05.ic18", 0x000000, 0x400000, CRC(dea7b8d6) SHA1(d7d98675eb3998a8057929f90aa340c1e5f6a617) )

	ROM_REGION( 0x1000000, "gfx6", 0 ) /* */
	ROM_LOAD( "mr99030-06.ic19", 0x000000, 0x400000, CRC(71489d79) SHA1(0398a354c2588e3974cb76a331e46165db6af06d) )

	ROM_REGION( 0x1000000, "gfx7", 0 ) /* */
	ROM_LOAD( "mr9930-01.ic30", 0x000000, 0x400000, CRC(9e3e054e) SHA1(06a4fa76cb83dbe9d565d5ccd0a5ecc5067887c9) )
	ROM_LOAD( "mr9930-02.ic29", 0x400000, 0x400000, CRC(b23c29f4) SHA1(a7b10a3a9af43db319baf8633bb3728120960923) )
	ROM_LOAD( "mr9930-03.ic28", 0x800000, 0x400000, CRC(9a5d070f) SHA1(b4668b4f299033140a2c56499cc2712ba111cb57) )

	ROM_REGION( 0x1000000, "gfx8", 0 ) /* */
	ROM_LOAD( "vj98348_step3_4_v1.1", 0x000000, 0x400000, CRC(dec612df) SHA1(acb86bb90c1cc61c7db3e022c69a5ff0611ffbae) )
	ROM_LOAD( "vj98348_step3_18_v1.1", 0x400000, 0x400000, CRC(bc92f0a0) SHA1(49c08de7a898a27972d4209709ddf447c5dca36a) )
	ROM_LOAD( "vj98348_step3_25_v1.1", 0x800000, 0x400000, CRC(dec612df) SHA1(acb86bb90c1cc61c7db3e022c69a5ff0611ffbae) )

	DISK_REGION( "disks" )
	DISK_IMAGE("step3", 0, NO_DUMP)
ROM_END

//GAME( 1999, stepstag, 0, stepstag, stepstag, 0, ROT0, "Jaleco", "Stepping Stage", GAME_NO_SOUND| GAME_NOT_WORKING)    // Original Game
GAME( 1999, stepstag, 0, stepstag, stepstag, 0, ROT0, "Jaleco", "Stepping Stage Special", GAME_NO_SOUND| GAME_NOT_WORKING)
//GAME( 1999, stepstag, 0, stepstag, stepstag, 0, ROT0, "Jaleco", "Stepping Stage 2 Supreme", GAME_NO_SOUND| GAME_NOT_WORKING)
GAME( 1999, step3, 0,	stepstag, stepstag, 0,	ROT0, "Jaleco", "Stepping 3 Superior", GAME_NO_SOUND| GAME_NOT_WORKING)
