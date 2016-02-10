// license:BSD-3-Clause
// copyright-holders:Bryan McPhail
/***************************************************************************

  Mad Motor                             (c) 1989 Mitchell Corporation

  But it's really a Data East game..  Bad Dudes era graphics hardware with
  Dark Seal era sound hardware.  Maybe a license for a specific territory?

  "This game is developed by Mitchell, but they entrusted PCB design and some
  routines to Data East."

  Emulation by Bryan McPhail, mish@tendril.co.uk

***************************************************************************/

#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "cpu/h6280/h6280.h"
#include "sound/2203intf.h"
#include "sound/2151intf.h"
#include "sound/okim6295.h"
#include "includes/madmotor.h"


/******************************************************************************/

WRITE16_MEMBER(madmotor_state::madmotor_sound_w)
{
	if (ACCESSING_BITS_0_7)
	{
		soundlatch_byte_w(space, 0, data & 0xff);
		m_audiocpu->set_input_line(0, HOLD_LINE);
	}
}


/******************************************************************************/


static ADDRESS_MAP_START( madmotor_map, AS_PROGRAM, 16, madmotor_state )
	AM_RANGE(0x000000, 0x07ffff) AM_ROM
	AM_RANGE(0x180000, 0x180007) AM_DEVWRITE("tilegen1", deco_bac06_device, pf_control_0_w)                          /* text layer */
	AM_RANGE(0x180010, 0x180017) AM_DEVWRITE("tilegen1", deco_bac06_device, pf_control_1_w)
	AM_RANGE(0x184000, 0x18407f) AM_DEVREADWRITE("tilegen1", deco_bac06_device, pf_colscroll_r, pf_colscroll_w)
	AM_RANGE(0x184080, 0x1843ff) AM_RAM
	AM_RANGE(0x184400, 0x1847ff) AM_DEVREADWRITE("tilegen1", deco_bac06_device, pf_rowscroll_r, pf_rowscroll_w)
	AM_RANGE(0x188000, 0x189fff) AM_DEVREADWRITE("tilegen1", deco_bac06_device, pf_data_r, pf_data_w)
	AM_RANGE(0x18c000, 0x18c001) AM_NOP
	AM_RANGE(0x190000, 0x190007) AM_DEVWRITE("tilegen2", deco_bac06_device, pf_control_0_w)                          /* text layer */
	AM_RANGE(0x190010, 0x190017) AM_DEVWRITE("tilegen2", deco_bac06_device, pf_control_1_w)
	AM_RANGE(0x198000, 0x1987ff) AM_DEVREADWRITE("tilegen2", deco_bac06_device, pf_data_r, pf_data_w)
	AM_RANGE(0x19c000, 0x19c001) AM_READNOP
	AM_RANGE(0x1a0000, 0x1a0007) AM_DEVWRITE("tilegen3", deco_bac06_device, pf_control_0_w)                          /* text layer */
	AM_RANGE(0x1a0010, 0x1a0017) AM_DEVWRITE("tilegen3", deco_bac06_device, pf_control_1_w)
	AM_RANGE(0x1a4000, 0x1a4fff) AM_DEVREADWRITE("tilegen3", deco_bac06_device, pf_data_r, pf_data_w)
	AM_RANGE(0x3e0000, 0x3e3fff) AM_RAM
	AM_RANGE(0x3e8000, 0x3e87ff) AM_RAM AM_SHARE("spriteram")
	AM_RANGE(0x3f0000, 0x3f07ff) AM_RAM_DEVWRITE("palette", palette_device, write) AM_SHARE("palette")
	AM_RANGE(0x3f8002, 0x3f8003) AM_READ_PORT("P1_P2")
	AM_RANGE(0x3f8004, 0x3f8005) AM_READ_PORT("DSW")
	AM_RANGE(0x3f8006, 0x3f8007) AM_READ_PORT("SYSTEM")
	AM_RANGE(0x3fc004, 0x3fc005) AM_WRITE(madmotor_sound_w)
ADDRESS_MAP_END

/******************************************************************************/

/* Physical memory map (21 bits) */
static ADDRESS_MAP_START( sound_map, AS_PROGRAM, 8, madmotor_state )
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

static INPUT_PORTS_START( madmotor )
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

	PORT_START("DSW")
	PORT_DIPNAME( 0x0007, 0x0007, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0007, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0006, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0005, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x0003, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( 1C_6C ) )
	PORT_DIPNAME( 0x0038, 0x0038, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0038, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0030, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0028, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x0018, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( 1C_6C ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0300, 0x0300, DEF_STR( Lives ) )
	PORT_DIPSETTING(      0x0000, "2" )
	PORT_DIPSETTING(      0x0300, "3" )
	PORT_DIPSETTING(      0x0200, "4" )
	PORT_DIPSETTING(      0x0100, "5" )
	PORT_DIPNAME( 0x0c00, 0x0c00, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x0c00, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x1000, 0x0000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x0000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000, 0x4000, DEF_STR( Allow_Continue ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( No ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x8000, 0x0000, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(      0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )

	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_VBLANK("screen")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END

/******************************************************************************/

static const gfx_layout charlayout =
{
	8,8,    /* 8*8 chars */
	4096,
	4,      /* 4 bits per pixel  */
	{ 0x18000*8, 0x8000*8, 0x10000*8, 0x00000*8 },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8 /* every char takes 8 consecutive bytes */
};

static const gfx_layout tilelayout =
{
	16,16,
	2048,
	4,
	{ 0x30000*8, 0x10000*8, 0x20000*8, 0x00000*8 },
	{ 16*8+0, 16*8+1, 16*8+2, 16*8+3, 16*8+4, 16*8+5, 16*8+6, 16*8+7,
			0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8,
			8*8, 9*8, 10*8, 11*8, 12*8, 13*8, 14*8, 15*8 },
	16*16
};

static const gfx_layout tilelayout2 =
{
	16,16,
	4096,
	4,
	{ 0x60000*8, 0x20000*8, 0x40000*8, 0x00000*8 },
	{ 16*8+0, 16*8+1, 16*8+2, 16*8+3, 16*8+4, 16*8+5, 16*8+6, 16*8+7,
			0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8,
			8*8, 9*8, 10*8, 11*8, 12*8, 13*8, 14*8, 15*8 },
	16*16
};

static const gfx_layout spritelayout =
{
	16,16,
	4096*2,
	4,
	{ 0xc0000*8, 0x80000*8, 0x40000*8, 0x00000*8 },
	{ 16*8+0, 16*8+1, 16*8+2, 16*8+3, 16*8+4, 16*8+5, 16*8+6, 16*8+7,
			0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8,
			8*8, 9*8, 10*8, 11*8, 12*8, 13*8, 14*8, 15*8 },
	16*16
};

static GFXDECODE_START( madmotor )
	GFXDECODE_ENTRY( "gfx1", 0, charlayout,     0, 16 ) /* Characters 8x8 */
	GFXDECODE_ENTRY( "gfx2", 0, tilelayout,   512, 16 ) /* Tiles 16x16 */
	GFXDECODE_ENTRY( "gfx3", 0, tilelayout2,  768, 16 ) /* Tiles 16x16 */
	GFXDECODE_ENTRY( "gfx4", 0, spritelayout, 256, 16 ) /* Sprites 16x16 */
GFXDECODE_END

/******************************************************************************/

void madmotor_state::machine_start()
{
	save_item(NAME(m_flipscreen));
}

void madmotor_state::machine_reset()
{
	m_flipscreen = 0;
}

static MACHINE_CONFIG_START( madmotor, madmotor_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68000, 12000000) /* Custom chip 59, 24 MHz crystal */
	MCFG_CPU_PROGRAM_MAP(madmotor_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", madmotor_state,  irq6_line_hold)/* VBL */

	MCFG_CPU_ADD("audiocpu", H6280, 8053000/2) /* Custom chip 45, Crystal near CPU is 8.053 MHz */
	MCFG_CPU_PROGRAM_MAP(sound_map)


	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_VIDEO_ATTRIBUTES(VIDEO_UPDATE_BEFORE_VBLANK)
	MCFG_SCREEN_REFRESH_RATE(58)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500) /* not accurate */ /* frames per second, vblank duration taken from Burger Time */)
	MCFG_SCREEN_SIZE(32*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 1*8, 31*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(madmotor_state, screen_update_madmotor)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", madmotor)
	MCFG_PALETTE_ADD("palette", 1024)
	MCFG_PALETTE_FORMAT(xxxxBBBBGGGGRRRR)

	MCFG_DEVICE_ADD("tilegen1", DECO_BAC06, 0)
	deco_bac06_device::set_gfx_region_wide(*device,0,0,0);
	MCFG_DECO_BAC06_GFXDECODE("gfxdecode")
	MCFG_DEVICE_ADD("tilegen2", DECO_BAC06, 0)
	deco_bac06_device::set_gfx_region_wide(*device,0,1,0);
	MCFG_DECO_BAC06_GFXDECODE("gfxdecode")
	MCFG_DEVICE_ADD("tilegen3", DECO_BAC06, 0)
	deco_bac06_device::set_gfx_region_wide(*device,0,2,1);
	MCFG_DECO_BAC06_GFXDECODE("gfxdecode")

	MCFG_DEVICE_ADD("spritegen", DECO_MXC06, 0)
	deco_mxc06_device::set_gfx_region(*device, 3);
	MCFG_DECO_MXC06_GFXDECODE("gfxdecode")
	MCFG_DECO_MXC06_PALETTE("palette")



	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("ym1", YM2203, 21470000/6)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.40)

	MCFG_YM2151_ADD("ym2", 21470000/6)
	MCFG_YM2151_IRQ_HANDLER(INPUTLINE("audiocpu", 1)) /* IRQ 2 */
	MCFG_SOUND_ROUTE(0, "mono", 0.45)
	MCFG_SOUND_ROUTE(1, "mono", 0.45)

	MCFG_OKIM6295_ADD("oki1", 1023924, OKIM6295_PIN7_HIGH) // clock frequency & pin 7 not verified
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)

	MCFG_OKIM6295_ADD("oki2", 2047848, OKIM6295_PIN7_HIGH) // clock frequency & pin 7 not verified
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
MACHINE_CONFIG_END

/******************************************************************************/

/* ROM names all corrected as per exact checksum matching PCB - system11 */
ROM_START( madmotor )
	ROM_REGION( 0x80000, "maincpu", 0 ) /* 68000 code */
	ROM_LOAD16_BYTE( "02-2.B4", 0x00000, 0x20000, CRC(50b554e0) SHA1(e33d0ab5464ab5ff394dd630536ac83baf0aa2c9) )
	ROM_LOAD16_BYTE( "00-2.B1", 0x00001, 0x20000, CRC(2d6a1b3f) SHA1(fa7058bf907becac56ed9938c5643aaefdf7a2c0) )
	ROM_LOAD16_BYTE( "03-2.B6", 0x40000, 0x20000, CRC(442a0a52) SHA1(86bb5470d5653d125481250f778c632371dddad8) )
	ROM_LOAD16_BYTE( "01-2.B3", 0x40001, 0x20000, CRC(e246876e) SHA1(648dca8bab001cfb42618081bbc1efa14118743e) )

	ROM_REGION( 0x10000, "audiocpu", 0 )    /* Sound CPU */
	ROM_LOAD( "14.L7",    0x00000, 0x10000, CRC(1c28a7e5) SHA1(ed30d0a5a8a079677bd34b6d98ab1b15b934b30f) )

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "04.A9",    0x000000, 0x10000, CRC(833ca3ab) SHA1(7a3e7ebecc1596d2e487595369ad9ba54ced5bfb) )    /* chars */
	ROM_LOAD( "05.A11",    0x010000, 0x10000, CRC(a691fbfe) SHA1(c726a4c15d599feb6883d9b643453e7028fa16d6) )

	ROM_REGION( 0x040000, "gfx2", 0 )
	ROM_LOAD( "10.A19",    0x000000, 0x20000, CRC(9dbf482b) SHA1(086e9170d577e502604c180f174fbce53a1e20e5) )    /* tiles */
	ROM_LOAD( "11.A21",    0x020000, 0x20000, CRC(593c48a9) SHA1(1158888f6b836253b8ae9db9b8e352f289b2e815) )

	ROM_REGION( 0x080000, "gfx3", 0 )
	ROM_LOAD( "06.A13",    0x000000, 0x20000, CRC(448850e5) SHA1(6a44a42738cf6a55b4bec807e0a3939a42b36793) )    /* tiles */
	ROM_LOAD( "07.A14",    0x020000, 0x20000, CRC(ede4d141) SHA1(7b847372bac043aa397aa5c274f90b9193de9176) )
	ROM_LOAD( "08.A16",    0x040000, 0x20000, CRC(c380e5e5) SHA1(ec87a94e7948b84c96b1577f5a8caebc56e38a94) )
	ROM_LOAD( "09.A18",    0x060000, 0x20000, CRC(1ee3326a) SHA1(bd03e5c4a2e7689260e6cc67288e71ef13f05a4b) )

	ROM_REGION( 0x100000, "gfx4", 0 )
	ROM_LOAD( "15.H11",    0x000000, 0x20000, CRC(90ae9f74) SHA1(806f96fd08fca1beeeaefe3c0fac1991410aa9c4) )    /* sprites */
	ROM_LOAD( "16.H13",    0x020000, 0x20000, CRC(e96ac815) SHA1(a2b22a29ad0a4f144bb09299c454dc7a842a5318) )
	ROM_LOAD( "17.H14",    0x040000, 0x20000, CRC(abad9a1b) SHA1(3cec6b4ef925205efe4a8fb28e08eb58e3ba4019) )
	ROM_LOAD( "18.H16",    0x060000, 0x20000, CRC(96d8d64b) SHA1(54ce87fe2b14b574176d2a1d2b86057b9cd10883) )
	ROM_LOAD( "19.J13",    0x080000, 0x20000, CRC(cbd8c9b8) SHA1(5e86c0298b3eea06920121eecb70e5bee705addf) )
	ROM_LOAD( "20.J14",    0x0a0000, 0x20000, CRC(47f706a8) SHA1(bd4fe499710f8905eb4b8d1ca990f2908feb95e1) )
	ROM_LOAD( "21.J16",    0x0c0000, 0x20000, CRC(9c72d364) SHA1(9290e463273fa1f921279f1bab808d91d3aa9648) )
	ROM_LOAD( "22.J18",    0x0e0000, 0x20000, CRC(1e78aa60) SHA1(f5f58ee6f5efe56e72623e57ce27884551e09bd9) )

	ROM_REGION( 0x40000, "oki1", 0 )    /* ADPCM samples */
	ROM_LOAD( "12.H1",    0x00000, 0x20000, CRC(c202d200) SHA1(8470654923a0e8780dad678f5745f8e3e3be08b2) )

	ROM_REGION( 0x40000, "oki2", 0 )    /* ADPCM samples */
	ROM_LOAD( "13.H3",    0x00000, 0x20000, CRC(cc4d65e9) SHA1(b9bcaa52c570f94d2f2e5dd84c94773cc4115442) )
ROM_END

/******************************************************************************/

DRIVER_INIT_MEMBER(madmotor_state,madmotor)
{
	UINT8 *rom = memregion("maincpu")->base();
	int i;

	for (i = 0x00000;i < 0x80000;i++)
	{
		rom[i] = (rom[i] & 0xdb) | ((rom[i] & 0x04) << 3) | ((rom[i] & 0x20) >> 3);
		rom[i] = (rom[i] & 0x7e) | ((rom[i] & 0x01) << 7) | ((rom[i] & 0x80) >> 7);
	}
}


	/* The title screen is undated, but it's (c) 1989 Data East at 0xefa0 */
GAME( 1989, madmotor, 0, madmotor, madmotor, madmotor_state, madmotor, ROT0, "Mitchell", "Mad Motor (prototype)", MACHINE_SUPPORTS_SAVE )
