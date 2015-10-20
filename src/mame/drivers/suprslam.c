// license:BSD-3-Clause
// copyright-holders:David Haywood
/*** DRIVER INFORMATION & NOTES ***********************************************

Super Slams - Driver by David Haywood
   Sound Information from R. Belmont
   DSWs corrected by Stephh

TODO :

sprite offset control?
priorities (see hi-score table during attract mode)
unknown reads / writes (also KONAMI chip ..?)

WORKING NOTES :

68k interrupts
lev 1 : 0x64 : 0000 0406 - vblank?
lev 2 : 0x68 : 0000 06ae - x
lev 3 : 0x6c : 0000 06b4 - x
lev 4 : 0x70 : 0000 06ba - x
lev 5 : 0x74 : 0000 06c0 - x
lev 6 : 0x78 : 0000 06c6 - x
lev 7 : 0x7c : 0000 06cc - x

******************************************************************************/


/*** README INFORMATION *******************************************************

Super Slam
(C) 1995 Banpresto

(C) 1995 Banpresto / Toei Animation

PCB: VSEP-26
CPU: TMP68HC000P16 (68000, 64 pin DIP)
SND: Z8400B (Z80B, 40 pin DIP), YM2610, YM3016
OSC: 14.318180 MHz (Near VS920D & 053936), 24.000000 MHz, 32.000 MHz (Both near VS920F & EB26IC66)
Other Chips:
            Fujitsu CG10103 145 9520 Z14    (160 Pin PQFP)
            VS9210 4L06F1056 JAPAN 9525EAI  (176 Pin PQFP)
            VS920F 4L01F1435 JAPAN 9524EAI  (100 Pin PQFP)
            VS920E 4L06F1057 JAPAN 9533EAI  (176 pin PQFP)
            VS9209 4L01F1429 JAPAN 9523EAI  (64 pin PQFP)
            VS920D 4L04F1689 JAPAN 9524EAI  (160 pin PQFP)
            KONAMI KS10011-PF 053936 PSAC2 9522 Z02 (80 pin PQFP)

RAM:
            LGS GM76C28K-10 x 1 (Connected/Near Z80B)
            LGS GM76C28K-10 x 2 (Connected/Near 053936)
            SEC KM6264BLS-7 x 2 (Connected/Near VS920D)
            SEC KM6264BLS-7 x 2 (Connected/Near VS9210)
            UM61256FK-15 x 2 (Connected/Near VS9210)
            CY7C195-25PC x 1     -\
            UM61256FK-15 x 4       > (Connected/Near Fujitsu CG10103 145 9520 Z14)
            SEC KM6264BLS-7 x 4  -/
            LGS GM76C28K-10 x 2 (Connected/Near VS920F)
            LGS GM76C28K-10 x 2 (Connected/Near VS920E)
            UM61256FK-15 x 2 (Connected/Near 68000 & VS9209)

PALs: (4 total, not dumped, 2 located near 68000, 1 near Z80B, 1 near VS9210)

DIPs: 8 position x 3 (ALL DIPs linked to VS9209)

Info taken from sheet supplied with PCB, no info for SW3.

ROMs: (on ALL ROMs is written only "EB26")

EB26_100.BIN    16M Mask    \
EB26_101.BIN    16M Mask    |
EB26IC09.BIN    16M Mask    |  GFX (near VS9210, 053936 & VS920D)
EB26IC10.BIN    16M Mask    |
EB26IC12.BIN    16M Mask    /
EB26IC36.BIN    16M Mask
EB26IC43.BIN    16M Mask       GFX (Near VS920F & VS920E)
EB26IC59.BIN    8M Mask        Sound (Near YM2610)
EB26IC66.BIN    16M Mask       Sound (Near YM2610)
EB26IC38.BIN    27C1001            Sound Program (Near Z80B)
EB26IC47.BIN    27C240      \
EB26IC73.BIN    27C240      /  Main Program

******************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "cpu/m68000/m68000.h"

#include "sound/2610intf.h"
#include "video/vsystem_spr.h"
#include "includes/suprslam.h"


/*** SOUND *******************************************************************/

WRITE16_MEMBER(suprslam_state::sound_command_w)
{
	if (ACCESSING_BITS_0_7)
	{
		m_pending_command = 1;
		soundlatch_byte_w(space, offset, data & 0xff);
		m_audiocpu->set_input_line(INPUT_LINE_NMI, PULSE_LINE);
	}
}

#if 0
READ16_MEMBER(suprslam_state::pending_command_r)
{
	return pending_command;
}
#endif

WRITE8_MEMBER(suprslam_state::pending_command_clear_w)
{
	m_pending_command = 0;
}

WRITE8_MEMBER(suprslam_state::suprslam_sh_bankswitch_w)
{
	membank("bank1")->set_entry(data & 0x03);
}

/*** MEMORY MAPS *************************************************************/

static ADDRESS_MAP_START( suprslam_map, AS_PROGRAM, 16, suprslam_state )
	AM_RANGE(0x000000, 0x0fffff) AM_ROM
	AM_RANGE(0xfb0000, 0xfb1fff) AM_RAM AM_SHARE("spriteram")
	AM_RANGE(0xfc0000, 0xfcffff) AM_RAM AM_SHARE("sp_videoram")
	AM_RANGE(0xfd0000, 0xfdffff) AM_RAM
	AM_RANGE(0xfe0000, 0xfe0fff) AM_RAM_WRITE(suprslam_screen_videoram_w) AM_SHARE("screen_videoram")
	AM_RANGE(0xff0000, 0xff1fff) AM_RAM_WRITE(suprslam_bg_videoram_w) AM_SHARE("bg_videoram")
	AM_RANGE(0xff2000, 0xff203f) AM_RAM AM_SHARE("screen_vregs")
//  AM_RANGE(0xff3000, 0xff3001) AM_WRITENOP // sprite buffer trigger?
	AM_RANGE(0xff8000, 0xff8fff) AM_DEVREADWRITE("k053936", k053936_device, linectrl_r, linectrl_w)
	AM_RANGE(0xff9000, 0xff9001) AM_WRITE(sound_command_w)
	AM_RANGE(0xffa000, 0xffafff) AM_RAM_DEVWRITE("palette", palette_device, write) AM_SHARE("palette")
	AM_RANGE(0xffd000, 0xffd01f) AM_DEVWRITE("k053936", k053936_device, ctrl_w)
	AM_RANGE(0xffe000, 0xffe001) AM_WRITE(suprslam_bank_w)
	AM_RANGE(0xfff000, 0xfff001) AM_READ_PORT("P1")
	AM_RANGE(0xfff002, 0xfff003) AM_READ_PORT("P2")
	AM_RANGE(0xfff004, 0xfff005) AM_READ_PORT("SYSTEM")
	AM_RANGE(0xfff006, 0xfff007) AM_READ_PORT("DSW1")
	AM_RANGE(0xfff008, 0xfff009) AM_READ_PORT("DSW2")
	AM_RANGE(0xfff00c, 0xfff00d) AM_WRITEONLY AM_SHARE("spr_ctrl")
ADDRESS_MAP_END

static ADDRESS_MAP_START( sound_map, AS_PROGRAM, 8, suprslam_state )
	AM_RANGE(0x0000, 0x77ff) AM_ROM
	AM_RANGE(0x7800, 0x7fff) AM_RAM
	AM_RANGE(0x8000, 0xffff) AM_ROMBANK("bank1")
ADDRESS_MAP_END

static ADDRESS_MAP_START( sound_io_map, AS_IO, 8, suprslam_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x00) AM_WRITE(suprslam_sh_bankswitch_w)
	AM_RANGE(0x04, 0x04) AM_READ(soundlatch_byte_r) AM_WRITE(pending_command_clear_w)
	AM_RANGE(0x08, 0x0b) AM_DEVREADWRITE("ymsnd", ym2610_device, read, write)
ADDRESS_MAP_END

/*** INPUT PORTS *************************************************************/

static INPUT_PORTS_START( suprslam )
	PORT_START("P1")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(1)

	PORT_START("P2")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(2)

	PORT_START("SYSTEM")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_COIN3 )                // Only in "test mode"
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_SERVICE2 )             // "Test"
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_TILT )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_SERVICE1 )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x0001, 0x0001, "Coin Slots" )
	PORT_DIPSETTING(      0x0001, "Common" )
	PORT_DIPSETTING(      0x0000, "Separate" )
	PORT_DIPNAME( 0x000e, 0x000e, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(      0x000a, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x000c, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x000e, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0006, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 1C_6C ) )
	PORT_DIPNAME( 0x0070, 0x0070, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(      0x0050, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0060, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0070, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0030, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 1C_6C ) )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Free_Play ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x0003, 0x0003, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x0003, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x000c, 0x000c, "Play Time" )
	PORT_DIPSETTING(      0x0008, "2:00" )
	PORT_DIPSETTING(      0x000c, "3:00" )
	PORT_DIPSETTING(      0x0004, "4:00" )
	PORT_DIPSETTING(      0x0000, "5:00" )
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( On ) )
	PORT_SERVICE( 0x0040, IP_ACTIVE_LOW )
	PORT_DIPNAME( 0x0080, 0x0000, "Country" )
	PORT_DIPSETTING(      0x0080, DEF_STR( Japan ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( World ) )
INPUT_PORTS_END

/*** GFX DECODE **************************************************************/

static const gfx_layout suprslam_8x8x4_layout =
{
	8,8,
	RGN_FRAC(1,1),
	4,
	{ 0,1,2,3 },
	{ 4, 0, 12, 8, 20, 16, 28, 24 },
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32 },
	8*32
};

static const gfx_layout suprslam_16x16x4_layout =
{
	16,16,
	RGN_FRAC(1,1),
	4,
	{ 0,1,2,3 },
	{ 8, 12, 0, 4, 24, 28, 16, 20,
	32+8, 32+12, 32+0, 32+4, 32+24,32+28,32+16,32+20},
	{ 0*64, 1*64, 2*64, 3*64, 4*64, 5*64, 6*64, 7*64,
		8*64, 9*64, 10*64, 11*64, 12*64, 13*64, 14*64, 15*64
	},
	16*64
};

static GFXDECODE_START( suprslam )
	GFXDECODE_ENTRY( "gfx1", 0, suprslam_8x8x4_layout,   0x000, 16 )
	GFXDECODE_ENTRY( "gfx2", 0, suprslam_16x16x4_layout, 0x200, 16 )
	GFXDECODE_ENTRY( "gfx3", 0, suprslam_16x16x4_layout, 0x100, 16 )
GFXDECODE_END

/*** MORE SOUND **************************************************************/

WRITE_LINE_MEMBER(suprslam_state::irqhandler)
{
	m_audiocpu->set_input_line(0, state ? ASSERT_LINE : CLEAR_LINE);
}

/*** MACHINE DRIVER **********************************************************/

void suprslam_state::machine_start()
{
	save_item(NAME(m_screen_bank));
	save_item(NAME(m_bg_bank));
	save_item(NAME(m_pending_command));

	membank("bank1")->configure_entries(0, 4, memregion("audiocpu")->base() + 0x10000, 0x8000);
}

void suprslam_state::machine_reset()
{
	m_screen_bank = 0;
	m_bg_bank = 0;
	m_pending_command = 0;
}

static MACHINE_CONFIG_START( suprslam, suprslam_state )

	MCFG_CPU_ADD("maincpu", M68000, 16000000)
	MCFG_CPU_PROGRAM_MAP(suprslam_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", suprslam_state,  irq1_line_hold)

	MCFG_CPU_ADD("audiocpu", Z80,8000000/2) /* 4 MHz ??? */
	MCFG_CPU_PROGRAM_MAP(sound_map)
	MCFG_CPU_IO_MAP(sound_io_map)


	MCFG_GFXDECODE_ADD("gfxdecode", "palette", suprslam)

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_VIDEO_ATTRIBUTES(VIDEO_UPDATE_AFTER_VBLANK)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2300) /* hand-tuned */)
	MCFG_SCREEN_SIZE(64*8, 64*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 40*8-1, 0*8, 28*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(suprslam_state, screen_update_suprslam)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_PALETTE_ADD("palette", 0x800)
	MCFG_PALETTE_FORMAT(xGGGGGBBBBBRRRRR)

	MCFG_DEVICE_ADD("vsystem_spr", VSYSTEM_SPR, 0)
	MCFG_VSYSTEM_SPR_SET_TILE_INDIRECT( suprslam_state, suprslam_tile_callback )
	MCFG_VSYSTEM_SPR_SET_GFXREGION(1)
	MCFG_VSYSTEM_SPR_GFXDECODE("gfxdecode")
	MCFG_VSYSTEM_SPR_PALETTE("palette")

	MCFG_DEVICE_ADD("k053936", K053936, 0)
	MCFG_K053936_WRAP(1)
	MCFG_K053936_OFFSETS(-45, -21)

	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MCFG_SOUND_ADD("ymsnd", YM2610, 8000000)
	MCFG_YM2610_IRQ_HANDLER(WRITELINE(suprslam_state, irqhandler))
	MCFG_SOUND_ROUTE(0, "lspeaker",  0.25)
	MCFG_SOUND_ROUTE(0, "rspeaker", 0.25)
	MCFG_SOUND_ROUTE(1, "lspeaker",  1.0)
	MCFG_SOUND_ROUTE(2, "rspeaker", 1.0)
MACHINE_CONFIG_END

/*** ROM LOADING *************************************************************/

ROM_START( suprslam )
	ROM_REGION( 0x100000, "maincpu", 0 ) /* 68000 Code */
	ROM_LOAD16_WORD_SWAP( "eb26ic47.bin", 0x000000, 0x080000, CRC(8d051fd8) SHA1(1820209306116e5b09cc10a8b3661d232c688b24) )
	ROM_LOAD16_WORD_SWAP( "eb26ic73.bin", 0x080000, 0x080000, CRC(ca4ad383) SHA1(143ee475761fa54d5b3a9f4e3fb3acc8408972fd) )

	ROM_REGION( 0x030000, "audiocpu", 0 ) /* Z80 Code */
	ROM_LOAD( "eb26ic38.bin", 0x000000, 0x020000, CRC(153f2c50) SHA1(b70f248cfb18239fcd26e36fb36159f219debf2c) )
	ROM_RELOAD(               0x010000, 0x020000 )

	ROM_REGION( 0x200000, "ymsnd", 0 ) /* Samples */
	ROM_LOAD( "eb26ic66.bin", 0x000000, 0x200000, CRC(8cb33682) SHA1(0e6189ef0673227d35b9a154e333cc6cf9b65df6) )

	ROM_REGION( 0x100000, "ymsnd.deltat", 0 ) /* Samples */
	ROM_LOAD( "eb26ic59.bin", 0x000000, 0x100000, CRC(4ae4095b) SHA1(62b0600b18febb6cecb6370b03a2d6b7756840a2) )

	ROM_REGION( 0x200000, "gfx1", 0 ) /* 8x8x4 'Screen' Layer GFX */
	ROM_LOAD( "eb26ic43.bin", 0x000000, 0x200000, CRC(9dfb0959) SHA1(ba479192a422a55efcf8aa7ff995c914525b4a56) )

	ROM_REGION( 0x800000, "gfx2", 0 ) /* 16x16x4 Sprites GFX */
	ROM_LOAD( "eb26ic09.bin", 0x000000, 0x200000, CRC(5a415365) SHA1(a59a4ab231980b0540e9a8356a02530217779dbd) )
	ROM_LOAD( "eb26ic10.bin", 0x200000, 0x200000, CRC(a04f3140) SHA1(621ff823d93fecdde801912064ac951727b71677) )
	ROM_LOAD( "eb26_100.bin", 0x400000, 0x200000, CRC(c2ee5eb6) SHA1(4b61e77a0d0f38b542d5e32fa25799a4c85bf651) )
	ROM_LOAD( "eb26_101.bin", 0x600000, 0x200000, CRC(7df654b7) SHA1(3a5ed6ee7cc31566e908b835a065e9bce60389fb) )

	ROM_REGION( 0x400000, "gfx3", 0 ) /* 16x16x4 BG GFX */
	ROM_LOAD( "eb26ic12.bin", 0x000000, 0x200000, CRC(14561bd7) SHA1(5f69f68a305aba9acb21b844c8aa5b1de60f89ff) )
	ROM_LOAD( "eb26ic36.bin", 0x200000, 0x200000, CRC(92019d89) SHA1(dbf6f8384341707996e4b9e07a3d4f536cf4905b) )
ROM_END

/*** GAME DRIVERS ************************************************************/

GAME( 1995, suprslam, 0, suprslam, suprslam, driver_device, 0, ROT0, "Banpresto / Toei Animation", "From TV Animation Slam Dunk - Super Slams", MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )
