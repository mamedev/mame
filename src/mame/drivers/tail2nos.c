// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
/***************************************************************************

    Tail to Nose / Super Formula - (c) 1989 Video System Co.

    Driver by Nicola Salmoria


    press F1+F3 to see ROM/RAM tests and the final animation

***************************************************************************/

#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "cpu/z80/z80.h"
#include "sound/2608intf.h"
#include "includes/tail2nos.h"


WRITE16_MEMBER(tail2nos_state::sound_command_w)
{
	if (ACCESSING_BITS_0_7)
	{
		soundlatch_byte_w(space, offset, data & 0xff);
		m_audiocpu->set_input_line(INPUT_LINE_NMI, PULSE_LINE);
	}
}

WRITE8_MEMBER(tail2nos_state::sound_bankswitch_w)
{
	membank("bank3")->set_entry(data & 0x01);
}

static ADDRESS_MAP_START( main_map, AS_PROGRAM, 16, tail2nos_state )
	AM_RANGE(0x000000, 0x03ffff) AM_ROM
	AM_RANGE(0x200000, 0x27ffff) AM_ROM AM_REGION("user1", 0)    /* extra ROM */
	AM_RANGE(0x2c0000, 0x2dffff) AM_ROM AM_REGION("user2", 0)
	AM_RANGE(0x400000, 0x41ffff) AM_RAM_WRITE(tail2nos_zoomdata_w) AM_SHARE("k051316")
	AM_RANGE(0x500000, 0x500fff) AM_DEVREADWRITE8("k051316", k051316_device, read, write, 0x00ff)
	AM_RANGE(0x510000, 0x51001f) AM_DEVWRITE8("k051316", k051316_device, ctrl_w, 0x00ff)
	AM_RANGE(0xff8000, 0xffbfff) AM_RAM                             /* work RAM */
	AM_RANGE(0xffc000, 0xffc2ff) AM_RAM AM_SHARE("spriteram")
	AM_RANGE(0xffc300, 0xffcfff) AM_RAM
	AM_RANGE(0xffd000, 0xffdfff) AM_RAM_WRITE(tail2nos_txvideoram_w) AM_SHARE("txvideoram")
	AM_RANGE(0xffe000, 0xffefff) AM_RAM_DEVWRITE("palette", palette_device, write) AM_SHARE("palette")
	AM_RANGE(0xfff000, 0xfff001) AM_READ_PORT("INPUTS") AM_WRITE(tail2nos_gfxbank_w)
	AM_RANGE(0xfff004, 0xfff005) AM_READ_PORT("DSW")
	AM_RANGE(0xfff008, 0xfff009) AM_WRITE(sound_command_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( sound_map, AS_PROGRAM, 8, tail2nos_state )
	AM_RANGE(0x0000, 0x77ff) AM_ROM
	AM_RANGE(0x7800, 0x7fff) AM_RAM
	AM_RANGE(0x8000, 0xffff) AM_ROMBANK("bank3")
ADDRESS_MAP_END

static ADDRESS_MAP_START( sound_port_map, AS_IO, 8, tail2nos_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x07, 0x07) AM_READ(soundlatch_byte_r) AM_WRITENOP /* the write is a clear pending command */
	AM_RANGE(0x08, 0x0b) AM_DEVWRITE("ymsnd", ym2608_device, write)
#if 0
	AM_RANGE(0x18, 0x1b) AM_DEVREAD("ymsnd", ym2608_device, read)
#endif
ADDRESS_MAP_END


static INPUT_PORTS_START( tail2nos )
	PORT_START("INPUTS")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_2WAY
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_2WAY
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("Test Advance") PORT_CODE(KEYCODE_F1)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW")
	PORT_DIPNAME( 0x000f, 0x0000, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(      0x0009, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0007, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0006, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x000b, "6 Coins/4 Credits" )
	PORT_DIPSETTING(      0x000c, DEF_STR( 4C_3C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x000d, "5 Coins/6 Credits" )
	PORT_DIPSETTING(      0x000e, DEF_STR( 4C_5C ) )
	PORT_DIPSETTING(      0x000a, DEF_STR( 2C_3C ) )
//  PORT_DIPSETTING(      0x000f, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0003, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(      0x0005, DEF_STR( 1C_6C ) )
	PORT_DIPNAME( 0x00f0, 0x0000, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(      0x0090, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0070, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0060, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x00b0, "6 Coins/4 Credits" )
	PORT_DIPSETTING(      0x00c0, DEF_STR( 4C_3C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x00d0, "5 Coins/6 Credits" )
	PORT_DIPSETTING(      0x00e0, DEF_STR( 4C_5C ) )
	PORT_DIPSETTING(      0x00a0, DEF_STR( 2C_3C ) )
//  PORT_DIPSETTING(      0x00f0, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0030, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(      0x0050, DEF_STR( 1C_6C ) )
	PORT_DIPNAME( 0x0300, 0x0000, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(      0x0100, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0300, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x0400, 0x0000, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_SERVICE( 0x0800, IP_ACTIVE_HIGH )
	PORT_DIPNAME( 0x1000, 0x1000, "Game Mode" )
	PORT_DIPSETTING(      0x1000, DEF_STR( Single ) )
	PORT_DIPSETTING(      0x0000, "Multiple" )
	PORT_DIPNAME( 0x2000, 0x0000, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000, 0x4000, "Control Panel" )
	PORT_DIPSETTING(      0x4000, DEF_STR( Standard ) )
	PORT_DIPSETTING(      0x0000, "Original" )
	PORT_DIPNAME( 0x8000, 0x0000, "Country" )
	PORT_DIPSETTING(      0x0000, "Domestic" )
	PORT_DIPSETTING(      0x8000, "Overseas" )
INPUT_PORTS_END



static const gfx_layout tail2nos_charlayout =
{
	8,8,
	RGN_FRAC(1,1),
	4,
	{ 0, 1, 2, 3 },
	{ 1*4, 0*4, 3*4, 2*4, 5*4, 4*4, 7*4, 6*4 },
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32 },
	32*8
};

static const gfx_layout tail2nos_spritelayout =
{
	16,32,
	RGN_FRAC(1,2),
	4,
	{ 0, 1, 2, 3 },
	{ 1*4, 0*4, 3*4, 2*4, RGN_FRAC(1,2)+1*4, RGN_FRAC(1,2)+0*4, RGN_FRAC(1,2)+3*4, RGN_FRAC(1,2)+2*4,
			5*4, 4*4, 7*4, 6*4, RGN_FRAC(1,2)+5*4, RGN_FRAC(1,2)+4*4, RGN_FRAC(1,2)+7*4, RGN_FRAC(1,2)+6*4 },
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32,
			8*32, 9*32, 10*32, 11*32, 12*32, 13*32, 14*32, 15*32,
			16*32, 17*32, 18*32, 19*32, 20*32, 21*32, 22*32, 23*32,
			24*32, 25*32, 26*32, 27*32, 28*32, 29*32, 30*32, 31*32 },
	128*8
};

static GFXDECODE_START( tail2nos )
	GFXDECODE_ENTRY( "gfx1", 0, tail2nos_charlayout,   0, 128 )
	GFXDECODE_ENTRY( "gfx2", 0, tail2nos_spritelayout, 0, 128 )
GFXDECODE_END



WRITE_LINE_MEMBER(tail2nos_state::irqhandler)
{
	m_audiocpu->set_input_line(0, state ? ASSERT_LINE : CLEAR_LINE);
}

void tail2nos_state::machine_start()
{
	UINT8 *ROM = memregion("audiocpu")->base();

	membank("bank3")->configure_entries(0, 2, &ROM[0x10000], 0x8000);
	membank("bank3")->set_entry(0);

	save_item(NAME(m_txbank));
	save_item(NAME(m_txpalette));
	save_item(NAME(m_video_enable));
}

void tail2nos_state::machine_reset()
{
	m_txbank = 0;
	m_txpalette = 0;
	m_video_enable = 0;
}

static MACHINE_CONFIG_START( tail2nos, tail2nos_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68000,XTAL_20MHz/2)    /* verified on pcb */
	MCFG_CPU_PROGRAM_MAP(main_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", tail2nos_state,  irq6_line_hold)

	MCFG_CPU_ADD("audiocpu", Z80,XTAL_20MHz/4)  /* verified on pcb */
	MCFG_CPU_PROGRAM_MAP(sound_map)
	MCFG_CPU_IO_MAP(sound_port_map)
								/* IRQs are triggered by the YM2608 */

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(64*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 40*8-1, 1*8, 31*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(tail2nos_state, screen_update_tail2nos)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", tail2nos)
	MCFG_PALETTE_ADD("palette", 2048)
	MCFG_PALETTE_FORMAT(xRRRRRGGGGGBBBBB)

	MCFG_DEVICE_ADD("k051316", K051316, 0)
	MCFG_GFX_PALETTE("palette")
	MCFG_K051316_BPP(-4)
	MCFG_K051316_OFFSETS(-89, -14)
	MCFG_K051316_WRAP(1)
	MCFG_K051316_CB(tail2nos_state, zoom_callback)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MCFG_SOUND_ADD("ymsnd", YM2608, XTAL_8MHz)  /* verified on pcb */
	MCFG_YM2608_IRQ_HANDLER(WRITELINE(tail2nos_state, irqhandler))
	MCFG_AY8910_PORT_B_WRITE_CB(WRITE8(tail2nos_state, sound_bankswitch_w))
	MCFG_SOUND_ROUTE(0, "lspeaker",  0.25)
	MCFG_SOUND_ROUTE(0, "rspeaker", 0.25)
	MCFG_SOUND_ROUTE(1, "lspeaker",  1.0)
	MCFG_SOUND_ROUTE(2, "rspeaker", 1.0)
MACHINE_CONFIG_END



ROM_START( tail2nos )
	ROM_REGION( 0x40000, "maincpu", 0 ) /* 68000 code */
	ROM_LOAD16_BYTE( "v4",           0x00000, 0x10000, CRC(1d4240c2) SHA1(db8992d8e718e20acb7b3f2f0b1f358098863145) )
	ROM_LOAD16_BYTE( "v7",           0x00001, 0x10000, CRC(0fb70066) SHA1(3d38672402d5ab70599c191cc274746a192b399b) )
	ROM_LOAD16_BYTE( "v3",           0x20000, 0x10000, CRC(e2e0abad) SHA1(1a1054bada9654484fe81fe4b4b32af5ab7b53f0) )
	ROM_LOAD16_BYTE( "v6",           0x20001, 0x10000, CRC(069817a7) SHA1(cca382fe2a49c8c3c84b879a1c30dffff84ef406) )

	ROM_REGION16_BE( 0x80000, "user1", 0 )
	/* extra ROM mapped at 200000 */
	ROM_LOAD16_WORD_SWAP( "a23",     0x00000, 0x80000, CRC(d851cf04) SHA1(ac5b366b686c5a037b127d223dc6fe90985eb160) )

	ROM_REGION16_BE( 0x20000, "user2", 0 )
	/* extra ROM mapped at 2c0000 */
	ROM_LOAD16_BYTE( "v5",           0x00000, 0x10000, CRC(a9fe15a1) SHA1(d90bf40c610ea7daaa338f83f82cdffbae7da08e) )
	ROM_LOAD16_BYTE( "v8",           0x00001, 0x10000, CRC(4fb6a43e) SHA1(5cddda0029b3b141c88b0c128655d35bb12fa34d) )

	ROM_REGION( 0x20000, "audiocpu", 0 )    /* 64k for the audio CPU + banks */
	ROM_LOAD( "v2",           0x00000, 0x08000, CRC(920d8920) SHA1(b8d30903248fee6f985af7fafbe534cfc8c6e829) )
	ROM_LOAD( "v1",           0x10000, 0x10000, CRC(bf35c1a4) SHA1(a838740e023dc3344dc528324a8dbc48bb98b574) )

	ROM_REGION( 0x100000, "gfx1", 0 )
	ROM_LOAD( "a24",          0x00000, 0x80000, CRC(b1e9de43) SHA1(0144252dd9ed561fbebd4994cccf11f6c87e1825) )
	ROM_LOAD( "o1s",          0x80000, 0x40000, CRC(e27a8eb4) SHA1(4fcadabf42a1c3deeb6d74d75cdbee802cf16db5) )

	ROM_REGION( 0x080000, "gfx2", 0 )
	ROM_LOAD( "oj1",          0x000000, 0x40000, CRC(39c36b35) SHA1(a97480696bf6d81bf415737e03cc5324d439ab84) )
	ROM_LOAD( "oj2",          0x040000, 0x40000, CRC(77ccaea2) SHA1(e38175859c75c6d0f2f01752fad6e167608c4662) )

	ROM_REGION( 0x20000, "ymsnd", 0 ) /* sound samples */
	ROM_LOAD( "osb",          0x00000, 0x20000, CRC(d49ab2f5) SHA1(92f7f6c8f35ac39910879dd88d2cfb6db7c848c9) )
ROM_END

ROM_START( sformula )
	ROM_REGION( 0x40000, "maincpu", 0 ) /* 68000 code */
	ROM_LOAD16_BYTE( "ic129.4",      0x00000, 0x10000, CRC(672bf690) SHA1(b322234b47f20a36430bc03be0b52d9b7f82967b) )
	ROM_LOAD16_BYTE( "ic130.7",      0x00001, 0x10000, CRC(73f0c91c) SHA1(faf14eb1a210c7330b47b78ca6c6563ea6482b3b) )
	ROM_LOAD16_BYTE( "v3",           0x20000, 0x10000, CRC(e2e0abad) SHA1(1a1054bada9654484fe81fe4b4b32af5ab7b53f0) )
	ROM_LOAD16_BYTE( "v6",           0x20001, 0x10000, CRC(069817a7) SHA1(cca382fe2a49c8c3c84b879a1c30dffff84ef406) )

	ROM_REGION16_BE( 0x80000, "user1", 0 )
	/* extra ROM mapped at 200000 */
	ROM_LOAD16_WORD_SWAP( "a23",     0x00000, 0x80000, CRC(d851cf04) SHA1(ac5b366b686c5a037b127d223dc6fe90985eb160) )

	ROM_REGION16_BE( 0x20000, "user2", 0 )
	/* extra ROM mapped at 2c0000 */
	ROM_LOAD16_BYTE( "v5",           0x00000, 0x10000, CRC(a9fe15a1) SHA1(d90bf40c610ea7daaa338f83f82cdffbae7da08e) )
	ROM_LOAD16_BYTE( "v8",           0x00001, 0x10000, CRC(4fb6a43e) SHA1(5cddda0029b3b141c88b0c128655d35bb12fa34d) )

	ROM_REGION( 0x20000, "audiocpu", 0 )    /* 64k for the audio CPU + banks */
	ROM_LOAD( "v2",           0x00000, 0x08000, CRC(920d8920) SHA1(b8d30903248fee6f985af7fafbe534cfc8c6e829) )
	ROM_LOAD( "v1",           0x10000, 0x10000, CRC(bf35c1a4) SHA1(a838740e023dc3344dc528324a8dbc48bb98b574) )

	ROM_REGION( 0x100000, "gfx1", 0 )
	ROM_LOAD( "a24",          0x00000, 0x80000, CRC(b1e9de43) SHA1(0144252dd9ed561fbebd4994cccf11f6c87e1825) )
	ROM_LOAD( "o1s",          0x80000, 0x40000, CRC(e27a8eb4) SHA1(4fcadabf42a1c3deeb6d74d75cdbee802cf16db5) )

	ROM_REGION( 0x80000, "gfx2", 0 )
	ROM_LOAD( "oj1",          0x000000, 0x40000, CRC(39c36b35) SHA1(a97480696bf6d81bf415737e03cc5324d439ab84) )
	ROM_LOAD( "oj2",          0x040000, 0x40000, CRC(77ccaea2) SHA1(e38175859c75c6d0f2f01752fad6e167608c4662) )

	ROM_REGION( 0x20000, "ymsnd", 0 ) /* sound samples */
	ROM_LOAD( "osb",          0x00000, 0x20000, CRC(d49ab2f5) SHA1(92f7f6c8f35ac39910879dd88d2cfb6db7c848c9) )
ROM_END


ROM_START( sformulaa )
	ROM_REGION( 0x40000, "maincpu", 0 ) /* 68000 code */
	ROM_LOAD16_BYTE( "04.bin",      0x00000, 0x10000, CRC(f40e9c3c) SHA1(2ab45f46f92bce42748692cafe601c5893de127b) )
	ROM_LOAD16_BYTE( "07.bin",      0x00001, 0x10000, CRC(d1cf6dca) SHA1(18228cc98722eb5907850e2d0317d1f4bf04fb8f) )
	ROM_LOAD16_BYTE( "v3",           0x20000, 0x10000, CRC(e2e0abad) SHA1(1a1054bada9654484fe81fe4b4b32af5ab7b53f0) )
	ROM_LOAD16_BYTE( "v6",           0x20001, 0x10000, CRC(069817a7) SHA1(cca382fe2a49c8c3c84b879a1c30dffff84ef406) )

	ROM_REGION16_BE( 0x80000, "user1", 0 )
	/* extra ROM mapped at 200000 */
	ROM_LOAD16_WORD_SWAP( "a23",     0x00000, 0x80000, CRC(d851cf04) SHA1(ac5b366b686c5a037b127d223dc6fe90985eb160) )

	ROM_REGION16_BE( 0x20000, "user2", 0 )
	/* extra ROM mapped at 2c0000 */
	ROM_LOAD16_BYTE( "v5",           0x00000, 0x10000, CRC(a9fe15a1) SHA1(d90bf40c610ea7daaa338f83f82cdffbae7da08e) )
	ROM_LOAD16_BYTE( "v8",           0x00001, 0x10000, CRC(4fb6a43e) SHA1(5cddda0029b3b141c88b0c128655d35bb12fa34d) )

	ROM_REGION( 0x20000, "audiocpu", 0 )    /* 64k for the audio CPU + banks */
	ROM_LOAD( "v2",           0x00000, 0x08000, CRC(920d8920) SHA1(b8d30903248fee6f985af7fafbe534cfc8c6e829) )
	ROM_LOAD( "v1",           0x10000, 0x10000, CRC(bf35c1a4) SHA1(a838740e023dc3344dc528324a8dbc48bb98b574) )

	ROM_REGION( 0x100000, "gfx1", ROMREGION_ERASE00 )
	ROM_LOAD( "a24",          0x00000, 0x80000, CRC(b1e9de43) SHA1(0144252dd9ed561fbebd4994cccf11f6c87e1825) )
	ROM_LOAD( "o1s",          0x80000, 0x40000, CRC(e27a8eb4) SHA1(4fcadabf42a1c3deeb6d74d75cdbee802cf16db5) )
	ROM_LOAD( "9.bin",           0xc0000, 0x08000, CRC(c76edc0a) SHA1(2c6c21f8d1f3bcb0f65ba5a779fe479783271e0b) ) // present on this PCB, contains Japanese text + same font as in above roms, where does it map? is there another layer?

	ROM_REGION( 0x80000, "gfx2", 0 )
	ROM_LOAD( "oj1",          0x000000, 0x40000, CRC(39c36b35) SHA1(a97480696bf6d81bf415737e03cc5324d439ab84) )
	ROM_LOAD( "oj2",          0x040000, 0x40000, CRC(77ccaea2) SHA1(e38175859c75c6d0f2f01752fad6e167608c4662) )

	ROM_REGION( 0x20000, "ymsnd", 0 ) /* sound samples */
	ROM_LOAD( "osb",          0x00000, 0x20000, CRC(d49ab2f5) SHA1(92f7f6c8f35ac39910879dd88d2cfb6db7c848c9) )
ROM_END

GAME( 1989, tail2nos, 0,        tail2nos, tail2nos, driver_device, 0, ROT90, "V-System Co.", "Tail to Nose - Great Championship", MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE )
GAME( 1989, sformula, tail2nos, tail2nos, tail2nos, driver_device, 0, ROT90, "V-System Co.", "Super Formula (Japan, set 1)", MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE )
GAME( 1989, sformulaa,tail2nos, tail2nos, tail2nos, driver_device, 0, ROT90, "V-System Co.", "Super Formula (Japan, set 2)", MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE ) // No Japan warning, but Japanese version
