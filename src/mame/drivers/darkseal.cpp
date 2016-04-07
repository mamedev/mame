// license:BSD-3-Clause
// copyright-holders:Bryan McPhail
/***************************************************************************

    Dark Seal (Rev 3)    (c) 1990 Data East Corporation (World version)
    Dark Seal (Rev 1)    (c) 1990 Data East Corporation (World version)
    Dark Seal (Rev 4)    (c) 1990 Data East Corporation (Japanese version)
    Gate Of Doom (Rev 4) (c) 1990 Data East Corporation (USA version)
    Gate of Doom (Rev 1) (c) 1990 Data East Corporation (USA version)

    Board:  DE-0332-2

    Emulation by Bryan McPhail, mish@tendril.co.uk

    2008-08
    Dip locations verified with the manual.

***************************************************************************/

#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "cpu/h6280/h6280.h"
#include "sound/2203intf.h"
#include "sound/2151intf.h"
#include "sound/okim6295.h"
#include "includes/darkseal.h"

/******************************************************************************/

WRITE16_MEMBER(darkseal_state::control_w)
{
	switch (offset<<1) {
	case 6: /* DMA flag */
		m_spriteram->copy();
		return;
	case 8: /* Sound CPU write */
		soundlatch_byte_w(space, 0, data & 0xff);
		m_audiocpu->set_input_line(0, HOLD_LINE);
		return;
	case 0xa: /* IRQ Ack (VBL) */
		return;
	}
}

READ16_MEMBER(darkseal_state::control_r)
{
	switch (offset<<1)
	{
		case 0:
			return ioport("DSW")->read();

		case 2:
			return ioport("P1_P2")->read();

		case 4:
			return ioport("SYSTEM")->read();
	}

	return ~0;
}

/******************************************************************************/

static ADDRESS_MAP_START( darkseal_map, AS_PROGRAM, 16, darkseal_state )
	AM_RANGE(0x000000, 0x07ffff) AM_ROM
	AM_RANGE(0x100000, 0x103fff) AM_RAM AM_SHARE("ram")
	AM_RANGE(0x120000, 0x1207ff) AM_RAM AM_SHARE("spriteram")
	AM_RANGE(0x140000, 0x140fff) AM_RAM_WRITE(palette_24bit_rg_w) AM_SHARE("paletteram")
	AM_RANGE(0x141000, 0x141fff) AM_RAM_WRITE(palette_24bit_b_w) AM_SHARE("paletteram2")
	AM_RANGE(0x180000, 0x18000f) AM_READWRITE(control_r, control_w)

	AM_RANGE(0x200000, 0x201fff) AM_DEVREADWRITE("tilegen2", deco16ic_device, pf1_data_r, pf1_data_w)
	AM_RANGE(0x202000, 0x203fff) AM_DEVREADWRITE("tilegen2", deco16ic_device, pf2_data_r, pf2_data_w)
	AM_RANGE(0x240000, 0x24000f) AM_DEVWRITE("tilegen2", deco16ic_device, pf_control_w)

	AM_RANGE(0x220000, 0x220fff) AM_RAM AM_SHARE("pf1_rowscroll")
	// pf2 & 4 rowscrolls are where? (maybe don't exist?)
	AM_RANGE(0x222000, 0x222fff) AM_RAM AM_SHARE("pf3_rowscroll")

	AM_RANGE(0x260000, 0x261fff) AM_DEVREADWRITE("tilegen1", deco16ic_device, pf1_data_r, pf1_data_w)
	AM_RANGE(0x262000, 0x263fff) AM_DEVREADWRITE("tilegen1", deco16ic_device, pf2_data_r, pf2_data_w)
	AM_RANGE(0x2a0000, 0x2a000f) AM_DEVWRITE("tilegen1", deco16ic_device, pf_control_w)
ADDRESS_MAP_END

/******************************************************************************/

static ADDRESS_MAP_START( sound_map, AS_PROGRAM, 8, darkseal_state )
	AM_RANGE(0x000000, 0x00ffff) AM_ROM
	AM_RANGE(0x100000, 0x100001) AM_DEVREADWRITE("ym1", ym2203_device, read, write)
	AM_RANGE(0x110000, 0x110001) AM_DEVREADWRITE("ym2", ym2151_device, read, write)
	AM_RANGE(0x120000, 0x120001) AM_DEVREADWRITE("oki1", okim6295_device, read, write)
	AM_RANGE(0x130000, 0x130001) AM_DEVREADWRITE("oki2", okim6295_device, read, write)
	AM_RANGE(0x140000, 0x140001) AM_READ(soundlatch_byte_r)
	AM_RANGE(0x1f0000, 0x1f1fff) AM_RAMBANK("bank8")
	AM_RANGE(0x1fec00, 0x1fec01) AM_DEVWRITE("audiocpu", h6280_device, timer_w)
	AM_RANGE(0x1ff400, 0x1ff403) AM_DEVWRITE("audiocpu", h6280_device, irq_status_w)
ADDRESS_MAP_END

/******************************************************************************/

static INPUT_PORTS_START( darkseal )
	PORT_START("P1_P2")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNUSED )   /* button 3 - unused */
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_START1 )

	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNUSED )   /* button 3 - unused */
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_START2 )

	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_VBLANK("screen")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW")
	PORT_DIPNAME( 0x0007, 0x0007, DEF_STR( Coin_A ) ) PORT_DIPLOCATION("SW1:1,2,3")
	PORT_DIPSETTING(      0x0000, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0007, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0006, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0005, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x0003, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( 1C_6C ) )
	PORT_DIPNAME( 0x0038, 0x0038, DEF_STR( Coin_B ) ) PORT_DIPLOCATION("SW1:4,5,6")
	PORT_DIPSETTING(      0x0000, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0038, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0030, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0028, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x0018, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( 1C_6C ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Flip_Screen ) ) PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPUNUSED_DIPLOC( 0x0080, 0x0080, "SW1:8" )    /* Manual says 'Always OFF' */

	PORT_DIPNAME( 0x0300, 0x0300, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(      0x0000, "1" )
	PORT_DIPSETTING(      0x0100, "2" )
	PORT_DIPSETTING(      0x0300, "3" )
	PORT_DIPSETTING(      0x0200, "4" )
	PORT_DIPNAME( 0x0c00, 0x0c00, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW2:3,4")
	PORT_DIPSETTING(      0x0800, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x0c00, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x3000, 0x3000, "Energy" ) PORT_DIPLOCATION("SW2:5,6")
	PORT_DIPSETTING(      0x0000, "2" )
	PORT_DIPSETTING(      0x1000, "2.5" )
	PORT_DIPSETTING(      0x3000, "3" )
	PORT_DIPSETTING(      0x2000, "4" )
	PORT_DIPNAME( 0x4000, 0x4000, DEF_STR( Allow_Continue ) ) PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(      0x0000, DEF_STR( No ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x8000, 0x0000, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(      0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
INPUT_PORTS_END

/******************************************************************************/

static const gfx_layout charlayout =
{
	8,8,    /* 8*8 chars */
	4096,
	4,      /* 4 bits per pixel  */
	{ 0x00000*8, 0x10000*8, 0x8000*8, 0x18000*8 },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8 /* every char takes 8 consecutive bytes */
};

static const gfx_layout seallayout =
{
	16,16,
	4096,
	4,
	{ 8, 0,  0x40000*8+8, 0x40000*8 },
	{ 32*8+0, 32*8+1, 32*8+2, 32*8+3, 32*8+4, 32*8+5, 32*8+6, 32*8+7,
		0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*16, 1*16, 2*16, 3*16, 4*16, 5*16, 6*16, 7*16,
			8*16, 9*16, 10*16, 11*16, 12*16, 13*16, 14*16, 15*16 },
	64*8
};

static const gfx_layout seallayout2 =
{
	16,16,
	4096*2,
	4,
	{ 8, 0, 0x80000*8+8, 0x80000*8 },
	{ 32*8+0, 32*8+1, 32*8+2, 32*8+3, 32*8+4, 32*8+5, 32*8+6, 32*8+7,
		0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*16, 1*16, 2*16, 3*16, 4*16, 5*16, 6*16, 7*16,
			8*16, 9*16, 10*16, 11*16, 12*16, 13*16, 14*16, 15*16 },
	64*8
};

static GFXDECODE_START( darkseal )
	GFXDECODE_ENTRY( "gfx1", 0, charlayout,    0, 16 )  /* Characters 8x8 */
	GFXDECODE_ENTRY( "gfx2", 0, seallayout,  768, 16 )  /* Tiles 16x16 */
	GFXDECODE_ENTRY( "gfx1", 0, charlayout,    0, 16 )  /* Characters 8x8 */
	GFXDECODE_ENTRY( "gfx3", 0, seallayout, 1024, 16 )  /* Tiles 16x16 */
	GFXDECODE_ENTRY( "gfx4", 0, seallayout2, 256, 32 )  /* Sprites 16x16 */
GFXDECODE_END

/******************************************************************************/

static MACHINE_CONFIG_START( darkseal, darkseal_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68000, XTAL_24MHz/2) /* Custom chip 59 */
	MCFG_CPU_PROGRAM_MAP(darkseal_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", darkseal_state,  irq6_line_hold)/* VBL */

	MCFG_CPU_ADD("audiocpu", H6280, XTAL_32_22MHz/4) /* Custom chip 45, Audio section crystal is 32.220 MHz */
	MCFG_CPU_PROGRAM_MAP(sound_map)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(58)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(529))
	MCFG_SCREEN_SIZE(32*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 1*8, 31*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(darkseal_state, screen_update)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", darkseal)
	MCFG_PALETTE_ADD("palette", 2048)

	MCFG_BUFFERED_SPRITERAM16_ADD("spriteram")

	MCFG_DEVICE_ADD("tilegen1", DECO16IC, 0)
	MCFG_DECO16IC_SPLIT(0)
	MCFG_DECO16IC_WIDTH12(3)     // both these tilemaps need to be twice the y size of usual!
	MCFG_DECO16IC_PF1_TRANS_MASK(0x0f)
	MCFG_DECO16IC_PF2_TRANS_MASK(0x0f)
	MCFG_DECO16IC_PF1_COL_BANK(0x00)
	MCFG_DECO16IC_PF2_COL_BANK(0x00)
	MCFG_DECO16IC_PF1_COL_MASK(0x0f)
	MCFG_DECO16IC_PF2_COL_MASK(0x0f)
	MCFG_DECO16IC_PF12_8X8_BANK(0)
	MCFG_DECO16IC_PF12_16X16_BANK(1)
	MCFG_DECO16IC_GFXDECODE("gfxdecode")

	MCFG_DEVICE_ADD("tilegen2", DECO16IC, 0)
	MCFG_DECO16IC_SPLIT(0)
	MCFG_DECO16IC_WIDTH12(1)
	MCFG_DECO16IC_PF1_TRANS_MASK(0x0f)
	MCFG_DECO16IC_PF2_TRANS_MASK(0x0f)
	MCFG_DECO16IC_PF1_COL_BANK(0x00)
	MCFG_DECO16IC_PF2_COL_BANK(0x00)
	MCFG_DECO16IC_PF1_COL_MASK(0x0f)
	MCFG_DECO16IC_PF2_COL_MASK(0x0f)
	MCFG_DECO16IC_PF12_8X8_BANK(2)
	MCFG_DECO16IC_PF12_16X16_BANK(3)
	MCFG_DECO16IC_GFXDECODE("gfxdecode")

	MCFG_DEVICE_ADD("spritegen", DECO_SPRITE, 0)
	MCFG_DECO_SPRITE_GFX_REGION(4)
	MCFG_DECO_SPRITE_GFXDECODE("gfxdecode")


	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("ym1", YM2203, XTAL_32_22MHz/8)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.45)

	MCFG_YM2151_ADD("ym2", XTAL_32_22MHz/9)
	MCFG_YM2151_IRQ_HANDLER(INPUTLINE("audiocpu", 1)) // IRQ2
	MCFG_SOUND_ROUTE(0, "mono", 0.55)
	MCFG_SOUND_ROUTE(1, "mono", 0.55)

	MCFG_OKIM6295_ADD("oki1", XTAL_32_22MHz/32, OKIM6295_PIN7_HIGH)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)

	MCFG_OKIM6295_ADD("oki2", XTAL_32_22MHz/16, OKIM6295_PIN7_HIGH)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.60)
MACHINE_CONFIG_END

/******************************************************************************/

ROM_START( darkseal )
	ROM_REGION( 0x80000, "maincpu", 0 ) /* 68000 code */
	ROM_LOAD16_BYTE( "ga_04-3.j12", 0x00000, 0x20000, CRC(bafad556) SHA1(5bd8a787d41a33919701ced29212bc11301e31d9) )
	ROM_LOAD16_BYTE( "ga_01-3.h14", 0x00001, 0x20000, CRC(f409050e) SHA1(4653094aeb5dd7ba1e490c04897a23ba8990df3c) )
	ROM_LOAD16_BYTE( "ga_00.h12",   0x40000, 0x20000, CRC(fbf3ac63) SHA1(51af581ee951eedeb4aa413ecbebe8bf4d30613b) )
	ROM_LOAD16_BYTE( "ga_05.j14",   0x40001, 0x20000, CRC(d5e3ae3f) SHA1(12f6e92af115422c6ab6ef1d33675d1e1cd58e10) )

	ROM_REGION( 0x10000, "audiocpu", 0 )    /* Sound CPU */
	ROM_LOAD( "fz_06-1.j15", 0x00000, 0x10000, CRC(c4828a6d) SHA1(fbfd0c85730bbe18401879cd68c19aaec9d482d8) )

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "fz_02.j1", 0x000000, 0x10000, CRC(3c9c3012) SHA1(086c2123725d4aa32838c0b6c82317d9c789c465) ) /* chars */
	ROM_LOAD( "fz_03.j2", 0x010000, 0x10000, CRC(264b90ed) SHA1(0bb1557673107c2d732a9374d5601a6eaf229473) )

	ROM_REGION( 0x080000, "gfx2", 0 )
	ROM_LOAD( "mac-03.h3", 0x000000, 0x80000, CRC(9996f3dc) SHA1(fffd9ecfe142a0c7c3c9c521778ff9c55ea8b225) ) /* tiles 1 */

	ROM_REGION( 0x080000, "gfx3", 0 )
	ROM_LOAD( "mac-02.e20", 0x000000, 0x80000, CRC(49504e89) SHA1(6da4733a650b9040abb2a81a49476368b514b5ab) ) /* tiles 2 */

	ROM_REGION( 0x100000, "gfx4", 0 )
	ROM_LOAD( "mac-00.b1", 0x000000, 0x80000, CRC(52acf1d6) SHA1(a7b68782417baafc86371b106fd31c5317f5b3d8) ) /* sprites */
	ROM_LOAD( "mac-01.b3", 0x080000, 0x80000, CRC(b28f7584) SHA1(e02ddd45130a7b50f80b6dd049059dba8071d768) )

	ROM_REGION( 0x40000, "oki1", 0 )    /* ADPCM samples */
	ROM_LOAD( "fz_08.l17", 0x00000, 0x20000, CRC(c9bf68e1) SHA1(c81e2534a814fe44c8787946a9fbe18f1743c3b4) )

	ROM_REGION( 0x40000, "oki2", 0 )    /* ADPCM samples */
	ROM_LOAD( "fz_07.k14", 0x00000, 0x20000, CRC(588dd3cb) SHA1(16c4e7670a4967768ddbfd52939d4e6e42268441) )
ROM_END

ROM_START( darkseal1 )
	ROM_REGION( 0x80000, "maincpu", 0 ) /* 68000 code */
	ROM_LOAD16_BYTE( "FZ_04-4.J12", 0x00000, 0x20000, CRC(a1a985a9) SHA1(eac3f43ff4016dcc21fe34b6bfed36e0d4b86959) )
	ROM_LOAD16_BYTE( "FZ_01-1.H14", 0x00001, 0x20000, CRC(98bd2940) SHA1(88ac727c3797e646834266320a71aa159e2b2541) )
	ROM_LOAD16_BYTE( "FZ_00-2.H12", 0x40000, 0x20000, CRC(fbf3ac63) SHA1(51af581ee951eedeb4aa413ecbebe8bf4d30613b) )
	ROM_LOAD16_BYTE( "FZ_05-2.J14", 0x40001, 0x20000, CRC(d5e3ae3f) SHA1(12f6e92af115422c6ab6ef1d33675d1e1cd58e10) )

	ROM_REGION( 0x10000, "audiocpu", 0 )    /* Sound CPU */
	ROM_LOAD( "FZ_06-1.J15", 0x00000, 0x10000, CRC(c4828a6d) SHA1(fbfd0c85730bbe18401879cd68c19aaec9d482d8) )

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "FZ_02-1.J1", 0x000000, 0x10000, CRC(3c9c3012) SHA1(086c2123725d4aa32838c0b6c82317d9c789c465) ) /* chars */
	ROM_LOAD( "FZ_03-1.J2", 0x010000, 0x10000, CRC(264b90ed) SHA1(0bb1557673107c2d732a9374d5601a6eaf229473) )

	ROM_REGION( 0x080000, "gfx2", 0 )
	ROM_LOAD( "mac-03.h3", 0x000000, 0x80000, CRC(9996f3dc) SHA1(fffd9ecfe142a0c7c3c9c521778ff9c55ea8b225) ) /* tiles 1 */

	ROM_REGION( 0x080000, "gfx3", 0 )
	ROM_LOAD( "mac-02.e20", 0x000000, 0x80000, CRC(49504e89) SHA1(6da4733a650b9040abb2a81a49476368b514b5ab) ) /* tiles 2 */

	ROM_REGION( 0x100000, "gfx4", 0 )
	ROM_LOAD( "mac-00.b1", 0x000000, 0x80000, CRC(52acf1d6) SHA1(a7b68782417baafc86371b106fd31c5317f5b3d8) ) /* sprites */
	ROM_LOAD( "mac-01.b3", 0x080000, 0x80000, CRC(b28f7584) SHA1(e02ddd45130a7b50f80b6dd049059dba8071d768) )

	ROM_REGION( 0x40000, "oki1", 0 )    /* ADPCM samples */
	ROM_LOAD( "FZ_08-1.K17", 0x00000, 0x20000, CRC(c9bf68e1) SHA1(c81e2534a814fe44c8787946a9fbe18f1743c3b4) )

	ROM_REGION( 0x40000, "oki2", 0 )    /* ADPCM samples */
	ROM_LOAD( "FZ_07-.K14", 0x00000, 0x20000, CRC(588dd3cb) SHA1(16c4e7670a4967768ddbfd52939d4e6e42268441) )
ROM_END

ROM_START( darksealj )
	ROM_REGION( 0x80000, "maincpu", 0 ) /* 68000 code */
	ROM_LOAD16_BYTE( "fz_04-4.j12", 0x00000, 0x20000, CRC(817faa2c) SHA1(8a79703f0e3aeb2ceeb098466561ab604baef301) )
	ROM_LOAD16_BYTE( "fz_01-4.h14", 0x00001, 0x20000, CRC(373caeee) SHA1(5cfa0c7672c439e9d011d9ec93da32c2377dce19) )
	ROM_LOAD16_BYTE( "fz_00-2.h12", 0x40000, 0x20000, CRC(1ab99aa7) SHA1(1da51f3ee0d15094911d4090264b945090d51242) )
	ROM_LOAD16_BYTE( "fz_05-2.j14", 0x40001, 0x20000, CRC(3374ef8c) SHA1(4144e71e452e281078bcd9b9a996db9f5dccc346) )

	ROM_REGION( 0x10000, "audiocpu", 0 )    /* Sound CPU */
	ROM_LOAD( "fz_06-1.j15", 0x00000, 0x10000, CRC(c4828a6d) SHA1(fbfd0c85730bbe18401879cd68c19aaec9d482d8) )

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "fz_02.j1", 0x000000, 0x10000, CRC(3c9c3012) SHA1(086c2123725d4aa32838c0b6c82317d9c789c465) ) /* chars */
	ROM_LOAD( "fz_03.j2", 0x010000, 0x10000, CRC(264b90ed) SHA1(0bb1557673107c2d732a9374d5601a6eaf229473) )

	ROM_REGION( 0x080000, "gfx2", 0 )
	ROM_LOAD( "mac-03.h3", 0x000000, 0x80000, CRC(9996f3dc) SHA1(fffd9ecfe142a0c7c3c9c521778ff9c55ea8b225) ) /* tiles 1 */

	ROM_REGION( 0x080000, "gfx3", 0 )
	ROM_LOAD( "mac-02.e20", 0x000000, 0x80000, CRC(49504e89) SHA1(6da4733a650b9040abb2a81a49476368b514b5ab) ) /* tiles 2 */

	ROM_REGION( 0x100000, "gfx4", 0 )
	ROM_LOAD( "mac-00.b1", 0x000000, 0x80000, CRC(52acf1d6) SHA1(a7b68782417baafc86371b106fd31c5317f5b3d8) ) /* sprites */
	ROM_LOAD( "mac-01.b3", 0x080000, 0x80000, CRC(b28f7584) SHA1(e02ddd45130a7b50f80b6dd049059dba8071d768) )

	ROM_REGION( 0x40000, "oki1", 0 )    /* ADPCM samples */
	ROM_LOAD( "fz_08.l17", 0x00000, 0x20000, CRC(c9bf68e1) SHA1(c81e2534a814fe44c8787946a9fbe18f1743c3b4) )

	ROM_REGION( 0x40000, "oki2", 0 )    /* ADPCM samples */
	ROM_LOAD( "fz_07.k14", 0x00000, 0x20000, CRC(588dd3cb) SHA1(16c4e7670a4967768ddbfd52939d4e6e42268441) )
ROM_END

ROM_START( gatedoom )
	ROM_REGION( 0x80000, "maincpu", 0 ) /* 68000 code */
	ROM_LOAD16_BYTE( "gb_04-4.j12", 0x00000, 0x20000, CRC(8e3a0bfd) SHA1(1d20bd678a630e2006c7f50f4d13656136db3721) )
	ROM_LOAD16_BYTE( "gb_01-4.h14", 0x00001, 0x20000, CRC(8d0fd383) SHA1(797e3cf43c9315b4195232eb1787a2292af4901b) )
	ROM_LOAD16_BYTE( "gb_00.h12",   0x40000, 0x20000, CRC(a88c16a1) SHA1(e02d5470692f23afa658b9bda933bb20be64602f) )
	ROM_LOAD16_BYTE( "gb_05.j14",   0x40001, 0x20000, CRC(252d7e14) SHA1(b2f27cd9686dfc697f3faca74d20b298a59efab2) )

	ROM_REGION( 0x10000, "audiocpu", 0 )    /* Sound CPU */
	ROM_LOAD( "fz_06-1.j15", 0x00000, 0x10000, CRC(c4828a6d) SHA1(fbfd0c85730bbe18401879cd68c19aaec9d482d8) )

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "fz_02.j1", 0x000000, 0x10000, CRC(3c9c3012) SHA1(086c2123725d4aa32838c0b6c82317d9c789c465) ) /* chars */
	ROM_LOAD( "fz_03.j2", 0x010000, 0x10000, CRC(264b90ed) SHA1(0bb1557673107c2d732a9374d5601a6eaf229473) )

	ROM_REGION( 0x080000, "gfx2", 0 )
	ROM_LOAD( "mac-03.h3", 0x000000, 0x80000, CRC(9996f3dc) SHA1(fffd9ecfe142a0c7c3c9c521778ff9c55ea8b225) ) /* tiles 1 */

	ROM_REGION( 0x080000, "gfx3", 0 )
	ROM_LOAD( "mac-02.e20", 0x000000, 0x80000, CRC(49504e89) SHA1(6da4733a650b9040abb2a81a49476368b514b5ab) ) /* tiles 2 */

	ROM_REGION( 0x100000, "gfx4", 0 )
	ROM_LOAD( "mac-00.b1", 0x000000, 0x80000, CRC(52acf1d6) SHA1(a7b68782417baafc86371b106fd31c5317f5b3d8) ) /* sprites */
	ROM_LOAD( "mac-01.b3", 0x080000, 0x80000, CRC(b28f7584) SHA1(e02ddd45130a7b50f80b6dd049059dba8071d768) )

	ROM_REGION( 0x40000, "oki1", 0 )    /* ADPCM samples */
	ROM_LOAD( "fz_08.l17", 0x00000, 0x20000, CRC(c9bf68e1) SHA1(c81e2534a814fe44c8787946a9fbe18f1743c3b4) )

	ROM_REGION( 0x40000, "oki2", 0 )    /* ADPCM samples */
	ROM_LOAD( "fz_07.k14", 0x00000, 0x20000, CRC(588dd3cb) SHA1(16c4e7670a4967768ddbfd52939d4e6e42268441) )
ROM_END

ROM_START( gatedoom1 )
	ROM_REGION( 0x80000, "maincpu", 0 ) /* 68000 code */
	ROM_LOAD16_BYTE( "gb_04.j12", 0x00000, 0x20000, CRC(4c3bbd2b) SHA1(e74a532edd01a559d0c388b37a2ead98c19fe5de) )
	ROM_LOAD16_BYTE( "gb_01.h14", 0x00001, 0x20000, CRC(59e367f4) SHA1(f88fa23b8e91f312103eb8a1d9a91d8171ec3ad4) )
	ROM_LOAD16_BYTE( "gb_00.h12", 0x40000, 0x20000, CRC(a88c16a1) SHA1(e02d5470692f23afa658b9bda933bb20be64602f) )
	ROM_LOAD16_BYTE( "gb_05.j14", 0x40001, 0x20000, CRC(252d7e14) SHA1(b2f27cd9686dfc697f3faca74d20b298a59efab2) )

	ROM_REGION( 0x10000, "audiocpu", 0 )    /* Sound CPU */
	ROM_LOAD( "fz_06-1.j15", 0x00000, 0x10000, CRC(c4828a6d) SHA1(fbfd0c85730bbe18401879cd68c19aaec9d482d8) )

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "fz_02.j1", 0x000000, 0x10000, CRC(3c9c3012) SHA1(086c2123725d4aa32838c0b6c82317d9c789c465) ) /* chars */
	ROM_LOAD( "fz_03.j2", 0x010000, 0x10000, CRC(264b90ed) SHA1(0bb1557673107c2d732a9374d5601a6eaf229473) )

	ROM_REGION( 0x080000, "gfx2", 0 )
	ROM_LOAD( "mac-03.h3", 0x000000, 0x80000, CRC(9996f3dc) SHA1(fffd9ecfe142a0c7c3c9c521778ff9c55ea8b225) ) /* tiles 1 */

	ROM_REGION( 0x080000, "gfx3", 0 )
	ROM_LOAD( "mac-02.e20", 0x000000, 0x80000, CRC(49504e89) SHA1(6da4733a650b9040abb2a81a49476368b514b5ab) ) /* tiles 2 */

	ROM_REGION( 0x100000, "gfx4", 0 )
	ROM_LOAD( "mac-00.b1", 0x000000, 0x80000, CRC(52acf1d6) SHA1(a7b68782417baafc86371b106fd31c5317f5b3d8) ) /* sprites */
	ROM_LOAD( "mac-01.b3", 0x080000, 0x80000, CRC(b28f7584) SHA1(e02ddd45130a7b50f80b6dd049059dba8071d768) )

	ROM_REGION( 0x40000, "oki1", 0 )    /* ADPCM samples */
	ROM_LOAD( "fz_08.l17", 0x00000, 0x20000, CRC(c9bf68e1) SHA1(c81e2534a814fe44c8787946a9fbe18f1743c3b4) )

	ROM_REGION( 0x40000, "oki2", 0 )    /* ADPCM samples */
	ROM_LOAD( "fz_07.k14", 0x00000, 0x20000, CRC(588dd3cb) SHA1(16c4e7670a4967768ddbfd52939d4e6e42268441) )
ROM_END

/******************************************************************************/

DRIVER_INIT_MEMBER(darkseal_state,darkseal)
{
	UINT8 *RAM = memregion("maincpu")->base();
	int i;

	for (i=0x00000; i<0x80000; i++)
		RAM[i]=(RAM[i] & 0xbd) | ((RAM[i] & 0x02) << 5) | ((RAM[i] & 0x40) >> 5);

}

/******************************************************************************/

GAME( 1990, darkseal,  0,        darkseal, darkseal, darkseal_state, darkseal, ROT0, "Data East Corporation", "Dark Seal (World revision 3)", MACHINE_SUPPORTS_SAVE )
GAME( 1990, darkseal1, darkseal, darkseal, darkseal, darkseal_state, darkseal, ROT0, "Data East Corporation", "Dark Seal (World revision 1)", MACHINE_SUPPORTS_SAVE )
GAME( 1990, darksealj, darkseal, darkseal, darkseal, darkseal_state, darkseal, ROT0, "Data East Corporation", "Dark Seal (Japan revision 4)", MACHINE_SUPPORTS_SAVE )
GAME( 1990, gatedoom,  darkseal, darkseal, darkseal, darkseal_state, darkseal, ROT0, "Data East Corporation", "Gate of Doom (US revision 4)", MACHINE_SUPPORTS_SAVE )
GAME( 1990, gatedoom1, darkseal, darkseal, darkseal, darkseal_state, darkseal, ROT0, "Data East Corporation", "Gate of Doom (US revision 1)", MACHINE_SUPPORTS_SAVE )
