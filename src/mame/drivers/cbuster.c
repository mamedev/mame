// license:BSD-3-Clause
// copyright-holders:Bryan McPhail
/***************************************************************************

  Crude Buster (World version FX)       (c) 1990 Data East Corporation
  Crude Buster (World version FU)       (c) 1990 Data East Corporation
  Crude Buster (Japanese version)       (c) 1990 Data East Corporation
  Two Crude (USA version)               (c) 1990 Data East USA

  The 'FX' board is filled with 'FU' roms except for the 4 program roms,
  both boards have 'export' stickers which usually indicates a World version.
  Maybe one is a UK or European version.

  DE-0333-3 PCB

  Data East 59 - 64 pin (68k)
  Data East 55 - 160 pin (x2) (tilemaps)
  Data East 45 (sticker) - 80 pin (Hu6280)
  Data East 52 - 128 pin (sprites)

  OSC: 24MHz & 32.22MHz
  YM2203C, YM2152, OKI 6295 x 2
  2 x 8-position Dipswitches

  (no obvious protection chip, probably a PAL?)

  Emulation by Bryan McPhail, mish@tendril.co.uk

  2008-07
  Dip locations and settings verified with manual (JPN version)

***************************************************************************/

#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "cpu/h6280/h6280.h"
#include "includes/cbuster.h"
#include "sound/2203intf.h"
#include "sound/2151intf.h"
#include "sound/okim6295.h"

WRITE16_MEMBER(cbuster_state::twocrude_control_w)
{
	data &= mem_mask;

	switch (offset << 1)
	{
	case 0: /* DMA flag */
		memcpy(m_spriteram16_buffer, m_spriteram16, 0x800);
		return;

	case 6: /* IRQ ack */
		return;

	case 2: /* Sound CPU write */
		soundlatch_byte_w(space, 0, data & 0xff);
		m_audiocpu->set_input_line(0, HOLD_LINE);
		return;

	case 4: /* Protection, maybe this is a PAL on the board?

            80046 is level number
            stop at stage and enter.
            see also 8216..

                9a 00 = pf4 over pf3 (normal) (level 0)
                9a f1 =  (level 1 - water), pf3 over ALL sprites + pf4
                9a 80 = pf3 over pf4 (Level 2 - copter)
                9a 40 = pf3 over ALL sprites + pf4 (snow) level 3
                9a c0 = doesn't matter?
                9a ff = pf 3 over pf4

            I can't find a priority register, I assume it's tied to the
            protection?!

        */
		if ((data & 0xffff) == 0x9a00) m_prot = 0;
		if ((data & 0xffff) == 0xaa)   m_prot = 0x74;
		if ((data & 0xffff) == 0x0200) m_prot = 0x63 << 8;
		if ((data & 0xffff) == 0x9a)   m_prot = 0xe;
		if ((data & 0xffff) == 0x55)   m_prot = 0x1e;
		if ((data & 0xffff) == 0x0e)  {m_prot = 0x0e; m_pri = 0;} /* start */
		if ((data & 0xffff) == 0x00)  {m_prot = 0x0e; m_pri = 0;} /* level 0 */
		if ((data & 0xffff) == 0xf1)  {m_prot = 0x36; m_pri = 1;} /* level 1 */
		if ((data & 0xffff) == 0x80)  {m_prot = 0x2e; m_pri = 1;} /* level 2 */
		if ((data & 0xffff) == 0x40)  {m_prot = 0x1e; m_pri = 1;} /* level 3 */
		if ((data & 0xffff) == 0xc0)  {m_prot = 0x3e; m_pri = 0;} /* level 4 */
		if ((data & 0xffff) == 0xff)  {m_prot = 0x76; m_pri = 1;} /* level 5 */

		break;
	}
	logerror("Warning %04x- %02x written to control %02x\n", space.device().safe_pc(), data, offset);
}

READ16_MEMBER(cbuster_state::twocrude_control_r)
{
	switch (offset << 1)
	{
		case 0: /* Player 1 & Player 2 joysticks & fire buttons */
			return ioport("P1_P2")->read();

		case 2: /* Dip Switches */
			return ioport("DSW")->read();

		case 4: /* Protection */
			logerror("%04x : protection control read at 30c000 %d\n", space.device().safe_pc(), offset);
			return m_prot;

		case 6: /* Credits, VBL in byte 7 */
			return ioport("COINS")->read();
	}

	return ~0;
}

/******************************************************************************/

static ADDRESS_MAP_START( twocrude_map, AS_PROGRAM, 16, cbuster_state )
	AM_RANGE(0x000000, 0x07ffff) AM_ROM
	AM_RANGE(0x080000, 0x083fff) AM_RAM AM_SHARE("ram")

	AM_RANGE(0x0a0000, 0x0a1fff) AM_DEVREADWRITE("tilegen1", deco16ic_device, pf1_data_r, pf1_data_w)
	AM_RANGE(0x0a2000, 0x0a2fff) AM_DEVREADWRITE("tilegen1", deco16ic_device, pf2_data_r, pf2_data_w)
	AM_RANGE(0x0a4000, 0x0a47ff) AM_RAM AM_SHARE("pf1_rowscroll")
	AM_RANGE(0x0a6000, 0x0a67ff) AM_RAM AM_SHARE("pf2_rowscroll")

	AM_RANGE(0x0a8000, 0x0a8fff) AM_DEVREADWRITE("tilegen2", deco16ic_device, pf1_data_r, pf1_data_w)
	AM_RANGE(0x0aa000, 0x0aafff) AM_DEVREADWRITE("tilegen2", deco16ic_device, pf2_data_r, pf2_data_w)
	AM_RANGE(0x0ac000, 0x0ac7ff) AM_RAM AM_SHARE("pf3_rowscroll")
	AM_RANGE(0x0ae000, 0x0ae7ff) AM_RAM AM_SHARE("pf4_rowscroll")

	AM_RANGE(0x0b0000, 0x0b07ff) AM_RAM AM_SHARE("spriteram16")
	AM_RANGE(0x0b4000, 0x0b4001) AM_WRITENOP
	AM_RANGE(0x0b5000, 0x0b500f) AM_DEVWRITE("tilegen1", deco16ic_device, pf_control_w)
	AM_RANGE(0x0b6000, 0x0b600f) AM_DEVWRITE("tilegen2", deco16ic_device, pf_control_w)
	AM_RANGE(0x0b8000, 0x0b8fff) AM_RAM_DEVWRITE("palette", palette_device, write) AM_SHARE("palette")
	AM_RANGE(0x0b9000, 0x0b9fff) AM_RAM_DEVWRITE("palette", palette_device, write_ext) AM_SHARE("palette_ext")
	AM_RANGE(0x0bc000, 0x0bc00f) AM_READWRITE(twocrude_control_r, twocrude_control_w)
ADDRESS_MAP_END



/******************************************************************************/

static ADDRESS_MAP_START( sound_map, AS_PROGRAM, 8, cbuster_state )
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

static INPUT_PORTS_START( twocrude )
	PORT_START("P1_P2")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON3 )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_START2 )

	PORT_START("COINS")
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
	PORT_DIPUNUSED_DIPLOC( 0x0080, 0x0080, "SW1:8" )    /* Manual says OFF "Don't Change" */

	PORT_DIPNAME( 0x0300, 0x0300, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(      0x0100, "1" )
	PORT_DIPSETTING(      0x0000, "2" )
	PORT_DIPSETTING(      0x0300, "3" )
	PORT_DIPSETTING(      0x0200, "4" )
	PORT_DIPNAME( 0x0c00, 0x0c00, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW2:3,4")
	PORT_DIPSETTING(      0x0c00, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Hardest ) )
	PORT_DIPUNUSED_DIPLOC( 0x1000, 0x1000, "SW2:5" )    /* Manual says OFF "Don't Change" */
	PORT_DIPUNUSED_DIPLOC( 0x2000, 0x2000, "SW2:6" )    /* Manual says OFF "Don't Change" */
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
	8,8,
	RGN_FRAC(1,1),
	4,
	{ 24,16,8,0 },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32 },
	8*32
};

static const gfx_layout tilelayout =
{
	16,16,
	RGN_FRAC(1,1),
	4,
	{ 24, 16, 8, 0 },
	{ 64*8+0, 64*8+1, 64*8+2, 64*8+3, 64*8+4, 64*8+5, 64*8+6, 64*8+7,
		0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32,
			8*32, 9*32, 10*32, 11*32, 12*32, 13*32, 14*32, 15*32 },
	128*8
};

static const gfx_layout spritelayout =
{
	16,16,
	(4096*2)+2048,  /* Main bank + 4 extra roms */
	4,
	{ 0xa0000*8+8, 0xa0000*8, 8, 0 },
	{ 32*8+0, 32*8+1, 32*8+2, 32*8+3, 32*8+4, 32*8+5, 32*8+6, 32*8+7,
		0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*16, 1*16, 2*16, 3*16, 4*16, 5*16, 6*16, 7*16,
			8*16, 9*16, 10*16, 11*16, 12*16, 13*16, 14*16, 15*16 },
	64*8
};

static GFXDECODE_START( cbuster )
	GFXDECODE_ENTRY( "gfx1", 0, charlayout,        0, 0x500 )   /* Characters 8x8 */
	GFXDECODE_ENTRY( "gfx1", 0, tilelayout,     0, 0x500 )  /* Tiles 16x16 */
	GFXDECODE_ENTRY( "gfx2", 0, tilelayout,     0, 0x500 )  /* Tiles 16x16 */
	GFXDECODE_ENTRY( "gfx3", 0, spritelayout, 0x100, 80 )   /* Sprites 16x16 */
GFXDECODE_END

/******************************************************************************/

DECO16IC_BANK_CB_MEMBER(cbuster_state::bank_callback)
{
	return ((bank >> 4) & 0x7) * 0x1000;
}

void cbuster_state::machine_start()
{
	save_item(NAME(m_prot));
	save_item(NAME(m_pri));
}

void cbuster_state::machine_reset()
{
	m_prot = 0;
	m_pri = 0;
}

static MACHINE_CONFIG_START( twocrude, cbuster_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68000, XTAL_24MHz/2) /* Custom chip 59 @ 12MHz Verified */
	MCFG_CPU_PROGRAM_MAP(twocrude_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", cbuster_state,  irq4_line_hold)/* VBL */

	MCFG_CPU_ADD("audiocpu", H6280, XTAL_24MHz/4) /* Custom chip 45, 6MHz Verified */
	MCFG_CPU_PROGRAM_MAP(sound_map)


	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(58)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(529))
	MCFG_SCREEN_SIZE(32*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 1*8, 31*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(cbuster_state, screen_update_twocrude)

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", cbuster)
	MCFG_PALETTE_ADD("palette", 2048)
	MCFG_PALETTE_FORMAT(XBGR)


	MCFG_DEVICE_ADD("tilegen1", DECO16IC, 0)
	MCFG_DECO16IC_SPLIT(0)
	MCFG_DECO16IC_WIDTH12(1)
	MCFG_DECO16IC_PF1_TRANS_MASK(0x0f)
	MCFG_DECO16IC_PF2_TRANS_MASK(0x0f)
	MCFG_DECO16IC_PF1_COL_BANK(0x00)
	MCFG_DECO16IC_PF2_COL_BANK(0x20)
	MCFG_DECO16IC_PF1_COL_MASK(0x0f)
	MCFG_DECO16IC_PF2_COL_MASK(0x0f)
	MCFG_DECO16IC_BANK1_CB(cbuster_state, bank_callback)
	MCFG_DECO16IC_BANK2_CB(cbuster_state, bank_callback)
	MCFG_DECO16IC_PF12_8X8_BANK(0)
	MCFG_DECO16IC_PF12_16X16_BANK(1)
	MCFG_DECO16IC_GFXDECODE("gfxdecode")
	MCFG_DECO16IC_PALETTE("palette")

	MCFG_DEVICE_ADD("tilegen2", DECO16IC, 0)
	MCFG_DECO16IC_SPLIT(0)
	MCFG_DECO16IC_WIDTH12(1)
	MCFG_DECO16IC_PF1_TRANS_MASK(0x0f)
	MCFG_DECO16IC_PF2_TRANS_MASK(0x0f)
	MCFG_DECO16IC_PF1_COL_BANK(0x30)
	MCFG_DECO16IC_PF2_COL_BANK(0x40)
	MCFG_DECO16IC_PF1_COL_MASK(0x0f)
	MCFG_DECO16IC_PF2_COL_MASK(0x0f)
	MCFG_DECO16IC_BANK1_CB(cbuster_state, bank_callback)
	MCFG_DECO16IC_BANK2_CB(cbuster_state, bank_callback)
	MCFG_DECO16IC_PF12_8X8_BANK(0)
	MCFG_DECO16IC_PF12_16X16_BANK(2)
	MCFG_DECO16IC_GFXDECODE("gfxdecode")
	MCFG_DECO16IC_PALETTE("palette")

	MCFG_DEVICE_ADD("spritegen", DECO_SPRITE, 0)
	MCFG_DECO_SPRITE_GFX_REGION(3)
	MCFG_DECO_SPRITE_GFXDECODE("gfxdecode")
	MCFG_DECO_SPRITE_PALETTE("palette")

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("ym1", YM2203, XTAL_32_22MHz/24) /* 1.3425MHz Verified */
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.60)

	MCFG_YM2151_ADD("ym2", XTAL_32_22MHz/9) /* 3.58MHz Verified */
	MCFG_YM2151_IRQ_HANDLER(INPUTLINE("audiocpu", 1)) /* IRQ2 */
	MCFG_SOUND_ROUTE(0, "mono", 0.45)
	MCFG_SOUND_ROUTE(1, "mono", 0.45)

	MCFG_OKIM6295_ADD("oki1", XTAL_32_22MHz/32, OKIM6295_PIN7_HIGH) /* 1.0068MHz Verified */
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.75)

	MCFG_OKIM6295_ADD("oki2", XTAL_32_22MHz/16, OKIM6295_PIN7_HIGH) /* 2.01375MHz Verified */
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.60)
MACHINE_CONFIG_END

/******************************************************************************/

ROM_START( cbuster )
	ROM_REGION( 0x80000, "maincpu", 0 ) /* 68000 code */
	ROM_LOAD16_BYTE( "fx01.7l", 0x00000, 0x20000, CRC(ddae6d83) SHA1(ce3fed1393b71821730fb8d87869a89c8e07c456) )
	ROM_LOAD16_BYTE( "fx00.4l", 0x00001, 0x20000, CRC(5bc2c0de) SHA1(fa9c357ae4a5c814b7113df3b2f12982077f3e6b) )
	ROM_LOAD16_BYTE( "fx03.9l", 0x40000, 0x20000, CRC(c3d65bf9) SHA1(99dd650fd4b427bca25a0776fbd6221f93504106) )
	ROM_LOAD16_BYTE( "fx02.6l", 0x40001, 0x20000, CRC(b875266b) SHA1(a76f8e061392e17394a3f975584823ad39e0097e) )

	ROM_REGION( 0x10000, "audiocpu", 0 )    /* Sound CPU */
	ROM_LOAD( "fu11-.19h", 0x00000, 0x10000, CRC(65f20f10) SHA1(cf914893edd98a0f39bbf7068a469ed7d34bd90e) )

	ROM_REGION( 0x100000, "gfx1", 0 )
	ROM_LOAD( "mab-00.4c",       0x00000, 0x80000, CRC(660eaabd) SHA1(e3d614e13fdb9af159d9758a869d9dae3dbe14e0) ) /* Tiles */
	ROM_LOAD16_BYTE( "fu05-.6c", 0x80000, 0x10000, CRC(8134d412) SHA1(9c70ff6f9f24ec89c0bb4645afdf2a5ca27e9a0c) ) /* Chars */
	ROM_LOAD16_BYTE( "fu06-.7c", 0x80001, 0x10000, CRC(2f914a45) SHA1(bb44ba4779e45ee77ef0006363df91aac1f4559a) )

	ROM_REGION( 0x80000, "gfx2", 0 )
	ROM_LOAD( "mab-01.19a", 0x00000, 0x80000, CRC(1080d619) SHA1(68f33a1580d33e4dd0858248c12a0a10ac117249) ) /* Tiles */

	ROM_REGION( 0x180000,"gfx3", 0 )
	ROM_LOAD( "mab-02.10a", 0x000000, 0x80000, CRC(58b7231d) SHA1(5b51a2fa42c67f23648be205295184a1fddc00f5) ) /* Sprites */
	ROM_LOAD( "mab-03.11a", 0x0a0000, 0x80000, CRC(76053b9d) SHA1(093cd01a13509701ec9dd1a806132600a5bd1915) )

	/* Space for extra sprites to be copied to (0x20000) */

	ROM_LOAD( "fu07-.4a", 0x140000, 0x10000, CRC(ca8d0bb3) SHA1(9262d6003cf0cb8c33d0f6c1d0ef35490b29f9b4) ) /* Extra sprites */
	ROM_LOAD( "fu08-.5a", 0x150000, 0x10000, CRC(c6afc5c8) SHA1(feddd546f09884c51e4d1802477de4e152a51082) )
	ROM_LOAD( "fu09-.7a", 0x160000, 0x10000, CRC(526809ca) SHA1(2cb9e7417211c1eb23d32e3fee71c5254d34a3ff) )
	ROM_LOAD( "fu10-.8a", 0x170000, 0x10000, CRC(6be6d50e) SHA1(b944db4b3a7c76190f6b40f71f033e16e7964f6a) )

	ROM_REGION( 0x40000, "oki1", 0 )    /* ADPCM samples */
	ROM_LOAD( "fu12-.16k", 0x00000, 0x20000, CRC(2d1d65f2) SHA1(be3d57b9976ddf7ee6d20ee9e78fe826ee411d79) )

	ROM_REGION( 0x40000, "oki2", 0 )    /* ADPCM samples */
	ROM_LOAD( "fu13-.21e", 0x00000, 0x20000, CRC(b8525622) SHA1(4a6ec5e3f64256b1383bfbab4167cbd2ec11b5c5) )

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "mb7114h.18e", 0x0000, 0x0100, CRC(3645b70f) SHA1(7d3831867362037892b43efb007e27d3bd5f6488) )   /* Priority (not used) */
ROM_END

ROM_START( cbusterw )
	ROM_REGION( 0x80000, "maincpu", 0 ) /* 68000 code */
	ROM_LOAD16_BYTE( "fu01-.7l", 0x00000, 0x20000, CRC(0203e0f8) SHA1(7709636429f2cab43caba3422122dba970dfb50b) )
	ROM_LOAD16_BYTE( "fu00-.4l", 0x00001, 0x20000, CRC(9c58626d) SHA1(6bc950929391221755972658258937a1ef96c244) )
	ROM_LOAD16_BYTE( "fu03-.9l", 0x40000, 0x20000, CRC(def46956) SHA1(e1f71a440430f8f9351ee9e1826ca2d0d5a372f8) )
	ROM_LOAD16_BYTE( "fu02-.6l", 0x40001, 0x20000, CRC(649c3338) SHA1(06373b364283706f0b00ab6d014c674e4b9818fa) )

	ROM_REGION( 0x10000, "audiocpu", 0 )    /* Sound CPU */
	ROM_LOAD( "fu11-.19h", 0x00000, 0x10000, CRC(65f20f10) SHA1(cf914893edd98a0f39bbf7068a469ed7d34bd90e) )

	ROM_REGION( 0x100000, "gfx1", 0 )
	ROM_LOAD( "mab-00.4c",       0x00000, 0x80000, CRC(660eaabd) SHA1(e3d614e13fdb9af159d9758a869d9dae3dbe14e0) ) /* Tiles */
	ROM_LOAD16_BYTE( "fu05-.6c", 0x80000, 0x10000, CRC(8134d412) SHA1(9c70ff6f9f24ec89c0bb4645afdf2a5ca27e9a0c) ) /* Chars */
	ROM_LOAD16_BYTE( "fu06-.7c", 0x80001, 0x10000, CRC(2f914a45) SHA1(bb44ba4779e45ee77ef0006363df91aac1f4559a) )

	ROM_REGION( 0x80000, "gfx2", 0 )
	ROM_LOAD( "mab-01.19a", 0x00000, 0x80000, CRC(1080d619) SHA1(68f33a1580d33e4dd0858248c12a0a10ac117249) ) /* Tiles */

	ROM_REGION( 0x180000,"gfx3", 0 )
	ROM_LOAD( "mab-02.10a", 0x000000, 0x80000, CRC(58b7231d) SHA1(5b51a2fa42c67f23648be205295184a1fddc00f5) ) /* Sprites */
	ROM_LOAD( "mab-03.11a", 0x0a0000, 0x80000, CRC(76053b9d) SHA1(093cd01a13509701ec9dd1a806132600a5bd1915) )

	/* Space for extra sprites to be copied to (0x20000) */

	ROM_LOAD( "fu07-.4a", 0x140000, 0x10000, CRC(ca8d0bb3) SHA1(9262d6003cf0cb8c33d0f6c1d0ef35490b29f9b4) ) /* Extra sprites */
	ROM_LOAD( "fu08-.5a", 0x150000, 0x10000, CRC(c6afc5c8) SHA1(feddd546f09884c51e4d1802477de4e152a51082) )
	ROM_LOAD( "fu09-.7a", 0x160000, 0x10000, CRC(526809ca) SHA1(2cb9e7417211c1eb23d32e3fee71c5254d34a3ff) )
	ROM_LOAD( "fu10-.8a", 0x170000, 0x10000, CRC(6be6d50e) SHA1(b944db4b3a7c76190f6b40f71f033e16e7964f6a) )

	ROM_REGION( 0x40000, "oki1", 0 )    /* ADPCM samples */
	ROM_LOAD( "fu12-.16k", 0x00000, 0x20000, CRC(2d1d65f2) SHA1(be3d57b9976ddf7ee6d20ee9e78fe826ee411d79) )

	ROM_REGION( 0x40000, "oki2", 0 )    /* ADPCM samples */
	ROM_LOAD( "fu13-.21e", 0x00000, 0x20000, CRC(b8525622) SHA1(4a6ec5e3f64256b1383bfbab4167cbd2ec11b5c5) )

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "mb7114h.18e",   0x0000, 0x0100, CRC(3645b70f) SHA1(7d3831867362037892b43efb007e27d3bd5f6488) )   /* Priority (not used) */
ROM_END

ROM_START( cbusterj )
	ROM_REGION( 0x80000, "maincpu", 0 ) /* 68000 code */
	ROM_LOAD16_BYTE( "fr01-1.7l", 0x00000, 0x20000, CRC(af3c014f) SHA1(a7724c48f73e52b19f3688a413e2ed013e226c6b) )
	ROM_LOAD16_BYTE( "fr00-1.4l", 0x00001, 0x20000, CRC(f666ad52) SHA1(6f7325bc3bb79fd8112df677250c4bae572dfa43) )
	ROM_LOAD16_BYTE( "fr03.9l",   0x40000, 0x20000, CRC(02c06118) SHA1(a251f936f80d8a9af033fe6d0d42e1e17ebbbf98) )
	ROM_LOAD16_BYTE( "fr02.6l",   0x40001, 0x20000, CRC(b6c34332) SHA1(c1215c72a03b368655e20f4557475a2fc4c46c9e) )

	ROM_REGION( 0x10000, "audiocpu", 0 )    /* Sound CPU */
	ROM_LOAD( "fu11-.19h", 0x00000, 0x10000, CRC(65f20f10) SHA1(cf914893edd98a0f39bbf7068a469ed7d34bd90e) )

	ROM_REGION( 0x100000, "gfx1", 0 )
	ROM_LOAD( "mab-00.4c",        0x00000, 0x80000, CRC(660eaabd) SHA1(e3d614e13fdb9af159d9758a869d9dae3dbe14e0) ) /* Tiles */
	/* Note: Revision 01 fixes some Japanese fonts: see characters 0x118a / 0x118b / 0x11f9 (16x16) */
	ROM_LOAD16_BYTE( "fr05-1.6c", 0x80000, 0x10000, CRC(b1f0d910) SHA1(a2a2ee3a99db52e77e9c108dacffb0387da131a9) ) /* Chars */
	ROM_LOAD16_BYTE( "fr06-1.7c", 0x80001, 0x10000, CRC(2f914a45) SHA1(bb44ba4779e45ee77ef0006363df91aac1f4559a) )


	ROM_REGION( 0x80000, "gfx2", 0 )
	ROM_LOAD( "mab-01.19a", 0x00000, 0x80000, CRC(1080d619) SHA1(68f33a1580d33e4dd0858248c12a0a10ac117249) ) /* Tiles */

	ROM_REGION( 0x180000,"gfx3", 0 )
	ROM_LOAD( "mab-02.10a", 0x000000, 0x80000, CRC(58b7231d) SHA1(5b51a2fa42c67f23648be205295184a1fddc00f5) ) /* Sprites */
	ROM_LOAD( "mab-03.11a", 0x0a0000, 0x80000, CRC(76053b9d) SHA1(093cd01a13509701ec9dd1a806132600a5bd1915) )

	/* Space for extra sprites to be copied to (0x20000) */

	ROM_LOAD( "fr07.4a", 0x140000, 0x10000, CRC(52c85318) SHA1(74032dac7cb7e7d3028aab4c5f5b0a4e2a7caa03) ) /* Extra sprites */
	ROM_LOAD( "fr08.5a", 0x150000, 0x10000, CRC(ea25fbac) SHA1(d00dce24e94ffc212ab3880c00fcadb7b2116f01) )
	ROM_LOAD( "fr09.7a", 0x160000, 0x10000, CRC(f8363424) SHA1(6a6b143a3474965ef89f75e9d7b15946ae26d0d4) )
	ROM_LOAD( "fr10.8a", 0x170000, 0x10000, CRC(241d5760) SHA1(cd216ecf7e88939b91a6e0f02a23c8b875ac24dc) )

	ROM_REGION( 0x40000, "oki1", 0 )    /* ADPCM samples */
	ROM_LOAD( "fu12-.16k", 0x00000, 0x20000, CRC(2d1d65f2) SHA1(be3d57b9976ddf7ee6d20ee9e78fe826ee411d79) )

	ROM_REGION( 0x40000, "oki2", 0 )    /* ADPCM samples */
	ROM_LOAD( "fu13-.21e", 0x00000, 0x20000, CRC(b8525622) SHA1(4a6ec5e3f64256b1383bfbab4167cbd2ec11b5c5) )

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "mb7114h.18e",   0x0000, 0x0100, CRC(3645b70f) SHA1(7d3831867362037892b43efb007e27d3bd5f6488) )   /* Priority (not used) */
ROM_END

ROM_START( twocrude )
	ROM_REGION( 0x80000, "maincpu", 0 ) /* 68000 code */
	ROM_LOAD16_BYTE( "ft01-1.7l", 0x00000, 0x20000, CRC(7342ffc4) SHA1(dcf611552f0d085f0b552985970f66d1bd6e51e9) )
	ROM_LOAD16_BYTE( "ft00-1.4l", 0x00001, 0x20000, CRC(3f5f535f) SHA1(f5d2b12f98bfcc3426dd2596baaaeca775835e6b) )
	ROM_LOAD16_BYTE( "ft03.9l",   0x40000, 0x20000, CRC(28002c99) SHA1(6397b05a1a237bb17657bee6c8185f61c60c6a2c) )
	ROM_LOAD16_BYTE( "ft02.6l",   0x40001, 0x20000, CRC(37ea0626) SHA1(ec1822eda83829c599cad217b6d5dd34fb970101) )

	ROM_REGION( 0x10000, "audiocpu", 0 )    /* Sound CPU */
	ROM_LOAD( "ft11-.19h", 0x00000, 0x10000, CRC(65f20f10) SHA1(cf914893edd98a0f39bbf7068a469ed7d34bd90e) )

	ROM_REGION( 0x100000, "gfx1", 0 )
	ROM_LOAD( "mab-00.4c",        0x00000, 0x80000, CRC(660eaabd) SHA1(e3d614e13fdb9af159d9758a869d9dae3dbe14e0) ) /* Tiles */
	/* Note: Revision 01 fixes some Japanese fonts: see characters 0x118a / 0x118b / 0x11f9 (16x16) */
	ROM_LOAD16_BYTE( "ft05-1.6c", 0x80000, 0x10000, CRC(b1f0d910) SHA1(a2a2ee3a99db52e77e9c108dacffb0387da131a9) ) /* Chars */
	ROM_LOAD16_BYTE( "ft06-1.7c", 0x80001, 0x10000, CRC(2f914a45) SHA1(bb44ba4779e45ee77ef0006363df91aac1f4559a) )

	ROM_REGION( 0x80000, "gfx2", 0 )
	ROM_LOAD( "mab-01.19a", 0x00000, 0x80000, CRC(1080d619) SHA1(68f33a1580d33e4dd0858248c12a0a10ac117249) ) /* Tiles */

	ROM_REGION( 0x180000,"gfx3", 0 )
	ROM_LOAD( "mab-02.10a", 0x000000, 0x80000, CRC(58b7231d) SHA1(5b51a2fa42c67f23648be205295184a1fddc00f5) ) /* Sprites */
	ROM_LOAD( "mab-03.11a", 0x0a0000, 0x80000, CRC(76053b9d) SHA1(093cd01a13509701ec9dd1a806132600a5bd1915) )

	/* Space for extra sprites to be copied to (0x20000) */

	ROM_LOAD( "ft07-.4a", 0x140000, 0x10000, CRC(e3465c25) SHA1(5369a87847e6f881efc8460e6e8efcf8ff46e87f) ) /* Extra sprites */
	ROM_LOAD( "ft08-.5a", 0x150000, 0x10000, CRC(c7f1d565) SHA1(d5dc55cf879f7feaff166a6708d60ef0bf31ddf5) )
	ROM_LOAD( "ft09-.7a", 0x160000, 0x10000, CRC(6e3657b9) SHA1(7e6a140e33f9bc18e35c255680eebe152a5d8858) )
	ROM_LOAD( "ft10-.8a", 0x170000, 0x10000, CRC(cdb83560) SHA1(8b258c4436ccea5a74edff1b6219ab7a5eac0328) )

	ROM_REGION( 0x40000, "oki1", 0 )    /* ADPCM samples */
	ROM_LOAD( "ft12-.16k", 0x00000, 0x20000, CRC(2d1d65f2) SHA1(be3d57b9976ddf7ee6d20ee9e78fe826ee411d79) )

	ROM_REGION( 0x40000, "oki2", 0 )    /* ADPCM samples */
	ROM_LOAD( "ft13-.21e", 0x00000, 0x20000, CRC(b8525622) SHA1(4a6ec5e3f64256b1383bfbab4167cbd2ec11b5c5) )

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "mb7114h.18e", 0x0000, 0x0100, CRC(3645b70f) SHA1(7d3831867362037892b43efb007e27d3bd5f6488) )   /* Priority (not used) */
ROM_END

ROM_START( twocrudea )
	ROM_REGION( 0x80000, "maincpu", 0 ) /* 68000 code */
	ROM_LOAD16_BYTE( "ft01.7l", 0x00000, 0x20000, CRC(08e96489) SHA1(1e75893cc086d6d6b428ca055851b51d0bc367aa) )
	ROM_LOAD16_BYTE( "ft00.4l", 0x00001, 0x20000, CRC(6765c445) SHA1(b2bbb86414eafe32ed66f3f8ab095a2bce3a1a4b) )
	ROM_LOAD16_BYTE( "ft03.9l", 0x40000, 0x20000, CRC(28002c99) SHA1(6397b05a1a237bb17657bee6c8185f61c60c6a2c) )
	ROM_LOAD16_BYTE( "ft02.6l", 0x40001, 0x20000, CRC(37ea0626) SHA1(ec1822eda83829c599cad217b6d5dd34fb970101) )

	ROM_REGION( 0x10000, "audiocpu", 0 )    /* Sound CPU */
	ROM_LOAD( "ft11-.19h", 0x00000, 0x10000, CRC(65f20f10) SHA1(cf914893edd98a0f39bbf7068a469ed7d34bd90e) )

	ROM_REGION( 0x100000, "gfx1", 0 )
	ROM_LOAD( "mab-00.4c",       0x00000, 0x80000, CRC(660eaabd) SHA1(e3d614e13fdb9af159d9758a869d9dae3dbe14e0) ) /* Tiles */
	ROM_LOAD16_BYTE( "ft05-.6c", 0x80000, 0x10000, CRC(8134d412) SHA1(9c70ff6f9f24ec89c0bb4645afdf2a5ca27e9a0c) ) /* Chars */
	ROM_LOAD16_BYTE( "ft06-.7c", 0x80001, 0x10000, CRC(2f914a45) SHA1(bb44ba4779e45ee77ef0006363df91aac1f4559a) )

	ROM_REGION( 0x80000, "gfx2", 0 )
	ROM_LOAD( "mab-01.19a", 0x00000, 0x80000, CRC(1080d619) SHA1(68f33a1580d33e4dd0858248c12a0a10ac117249) ) /* Tiles */

	ROM_REGION( 0x180000,"gfx3", 0 )
	ROM_LOAD( "mab-02.10a", 0x000000, 0x80000, CRC(58b7231d) SHA1(5b51a2fa42c67f23648be205295184a1fddc00f5) ) /* Sprites */
	ROM_LOAD( "mab-03.11a", 0x0a0000, 0x80000, CRC(76053b9d) SHA1(093cd01a13509701ec9dd1a806132600a5bd1915) )

	/* Space for extra sprites to be copied to (0x20000) */

	ROM_LOAD( "ft07-.4a", 0x140000, 0x10000, CRC(e3465c25) SHA1(5369a87847e6f881efc8460e6e8efcf8ff46e87f) ) /* Extra sprites */
	ROM_LOAD( "ft08-.5a", 0x150000, 0x10000, CRC(c7f1d565) SHA1(d5dc55cf879f7feaff166a6708d60ef0bf31ddf5) )
	ROM_LOAD( "ft09-.7a", 0x160000, 0x10000, CRC(6e3657b9) SHA1(7e6a140e33f9bc18e35c255680eebe152a5d8858) )
	ROM_LOAD( "ft10-.8a", 0x170000, 0x10000, CRC(cdb83560) SHA1(8b258c4436ccea5a74edff1b6219ab7a5eac0328) )

	ROM_REGION( 0x40000, "oki1", 0 )    /* ADPCM samples */
	ROM_LOAD( "ft12-.16k", 0x00000, 0x20000, CRC(2d1d65f2) SHA1(be3d57b9976ddf7ee6d20ee9e78fe826ee411d79) )

	ROM_REGION( 0x40000, "oki2", 0 )    /* ADPCM samples */
	ROM_LOAD( "ft13-.21e", 0x00000, 0x20000, CRC(b8525622) SHA1(4a6ec5e3f64256b1383bfbab4167cbd2ec11b5c5) )

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "mb7114h.18e", 0x0000, 0x0100, CRC(3645b70f) SHA1(7d3831867362037892b43efb007e27d3bd5f6488) )   /* Priority (not used) */
ROM_END

/******************************************************************************/

DRIVER_INIT_MEMBER(cbuster_state,twocrude)
{
	UINT8 *RAM = memregion("maincpu")->base();
	UINT8 *PTR;
	int i, j;

	/* Main cpu decrypt */
	for (i = 0x00000; i < 0x80000; i += 2)
	{
		int h = i + NATIVE_ENDIAN_VALUE_LE_BE(1,0), l = i + NATIVE_ENDIAN_VALUE_LE_BE(0,1);

		RAM[h] = (RAM[h] & 0xcf) | ((RAM[h] & 0x10) << 1) | ((RAM[h] & 0x20) >> 1);
		RAM[h] = (RAM[h] & 0x5f) | ((RAM[h] & 0x20) << 2) | ((RAM[h] & 0x80) >> 2);

		RAM[l] = (RAM[l] & 0xbd) | ((RAM[l] & 0x2) << 5) | ((RAM[l] & 0x40) >> 5);
		RAM[l] = (RAM[l] & 0xf5) | ((RAM[l] & 0x2) << 2) | ((RAM[l] & 0x8) >> 2);
	}

	/* Rearrange the 'extra' sprite bank to be in the same format as main sprites */
	RAM = memregion("gfx3")->base() + 0x080000;
	PTR = memregion("gfx3")->base() + 0x140000;
	for (i = 0; i < 0x20000; i += 64)
	{
		for (j = 0; j < 16; j += 1)
		{ /* Copy 16 lines down */
			RAM[i +       0 + j * 2] = PTR[i / 2 +       0 + j]; /* Pixels 0-7 for each plane */
			RAM[i +       1 + j * 2] = PTR[i / 2 + 0x10000 + j];
			RAM[i + 0xa0000 + j * 2] = PTR[i / 2 + 0x20000 + j];
			RAM[i + 0xa0001 + j * 2] = PTR[i / 2 + 0x30000 + j];
		}

		for (j = 0; j < 16; j += 1)
		{ /* Copy 16 lines down */
			RAM[i +    0x20 + j * 2] = PTR[i / 2 +    0x10 + j]; /* Pixels 8-15 for each plane */
			RAM[i +    0x21 + j * 2] = PTR[i / 2 + 0x10010 + j];
			RAM[i + 0xa0020 + j * 2] = PTR[i / 2 + 0x20010 + j];
			RAM[i + 0xa0021 + j * 2] = PTR[i / 2 + 0x30010 + j];
		}
	}
}

/******************************************************************************/

GAME( 1990, cbuster,  0,       twocrude, twocrude, cbuster_state, twocrude, ROT0, "Data East Corporation", "Crude Buster (World FX version)", MACHINE_SUPPORTS_SAVE )
GAME( 1990, cbusterw, cbuster, twocrude, twocrude, cbuster_state, twocrude, ROT0, "Data East Corporation", "Crude Buster (World FU version)", MACHINE_SUPPORTS_SAVE )
GAME( 1990, cbusterj, cbuster, twocrude, twocrude, cbuster_state, twocrude, ROT0, "Data East Corporation", "Crude Buster (Japan FR revision 1)", MACHINE_SUPPORTS_SAVE )
GAME( 1990, twocrude, cbuster, twocrude, twocrude, cbuster_state, twocrude, ROT0, "Data East USA", "Two Crude (US FT revision 1)", MACHINE_SUPPORTS_SAVE )
GAME( 1990, twocrudea,cbuster, twocrude, twocrude, cbuster_state, twocrude, ROT0, "Data East USA", "Two Crude (US FT version)", MACHINE_SUPPORTS_SAVE )
