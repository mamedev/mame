// license:BSD-3-Clause
// copyright-holders:Bryan McPhail, David Haywood
/***************************************************************************

    Dynamite Duke                       (c) 1989 Seibu Kaihatsu/Fabtek
    The Double Dynamites                (c) 1989 Seibu Kaihatsu/Fabtek


    To access test mode, reset with both start buttons held.

    Coin inputs are handled by the sound CPU, so they don't work with sound
    disabled. Just put the game in Free Play mode.

    The background layer is 5bpp and I'm not 100% sure the colours are
    correct on it, although the layer is 5bpp the palette data is 4bpp.
    My current implementation looks pretty good though I've never seen
    the real game.

    There is a country code byte in the program to select between
    Seibu Kaihatsu/Fabtek/Taito licenses.

    Emulation by Bryan McPhail, mish@tendril.co.uk


        SW#1
        --------------------------------------------------------------------
        DESCRIPTION                        1   2   3   4   5   6   7   8
        --------------------------------------------------------------------
        COIN MODE       MODE 1            OFF
                        MODE 2            ON
        --------------------------------------------------------------------
        COIN/CREDIT*
        MODE #1         1C/1P                 OFF OFF OFF OFF
                        2C/1P                 ON  OFF OFF OFF
                        3C/1P                 OFF ON  OFF OFF
                        4C/1P                 ON  ON  OFF OFF
                        FREE PLAY             ON  ON  ON  ON
        MODE #2
        COIN A          1C/1P                 OFF OFF
                        2C/1P                 ON  OFF
                        3C/1P                 OFF ON
                        5C/1P                 ON  ON
        COIN B          1C/2P                         OFF OFF
                        1C/3P                         ON  OFF
                        1C/5P                         OFF ON
                        1C/6P                         ON  ON
        --------------------------------------------------------------------
        STARTING COIN   NORMAL                                OFF
                        X2                                    ON
        --------------------------------------------------------------------
        CABINET TYPE    TABLE                                     ON
                        UPRIGHT                                   OFF
        --------------------------------------------------------------------
        VIDEO SCREEN    NORMAL                                        OFF
                        FLIP                                          ON
        --------------------------------------------------------------------
        FACTORY SETTINGS                     OFF OFF OFF OFF OFF OFF OFF OFF
        --------------------------------------------------------------------


2008-07
Dip locations and factory settings verified with dip listing
Also, implemented conditional port for Coin Mode (SW1:1)

***************************************************************************/

#include "emu.h"
#include "cpu/nec/nec.h"
#include "cpu/z80/z80.h"
#include "sound/3812intf.h"
#include "sound/okim6295.h"
#include "includes/dynduke.h"


/* Memory Maps */

static ADDRESS_MAP_START( master_map, AS_PROGRAM, 16, dynduke_state )
	AM_RANGE(0x00000, 0x06fff) AM_RAM
	AM_RANGE(0x07000, 0x07fff) AM_RAM AM_SHARE("spriteram")
	AM_RANGE(0x08000, 0x080ff) AM_RAM AM_SHARE("scroll_ram")
	AM_RANGE(0x0a000, 0x0afff) AM_RAM AM_SHARE("share1")
	AM_RANGE(0x0b000, 0x0b001) AM_READ_PORT("P1_P2")
	AM_RANGE(0x0b002, 0x0b003) AM_READ_PORT("DSW")
	AM_RANGE(0x0b004, 0x0b005) AM_WRITENOP
	AM_RANGE(0x0b006, 0x0b007) AM_WRITE(control_w)
	AM_RANGE(0x0c000, 0x0c7ff) AM_RAM_WRITE(text_w) AM_SHARE("videoram")
	AM_RANGE(0x0d000, 0x0d00d) AM_DEVREADWRITE("seibu_sound", seibu_sound_device, main_word_r, main_word_w)
	AM_RANGE(0xa0000, 0xfffff) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START( slave_map, AS_PROGRAM, 16, dynduke_state )
	AM_RANGE(0x00000, 0x05fff) AM_RAM
	AM_RANGE(0x06000, 0x067ff) AM_RAM_WRITE(background_w) AM_SHARE("back_data")
	AM_RANGE(0x06800, 0x06fff) AM_RAM_WRITE(foreground_w) AM_SHARE("fore_data")
	AM_RANGE(0x07000, 0x07fff) AM_RAM_WRITE(paletteram_w) AM_SHARE("paletteram")
	AM_RANGE(0x08000, 0x08fff) AM_RAM AM_SHARE("share1")
	AM_RANGE(0x0a000, 0x0a001) AM_WRITE(gfxbank_w)
	AM_RANGE(0x0c000, 0x0c001) AM_WRITENOP
	AM_RANGE(0xc0000, 0xfffff) AM_ROM
ADDRESS_MAP_END

/* Memory map used by DlbDyn - probably an addressing PAL is different */
static ADDRESS_MAP_START( masterj_map, AS_PROGRAM, 16, dynduke_state )
	AM_RANGE(0x00000, 0x06fff) AM_RAM
	AM_RANGE(0x07000, 0x07fff) AM_RAM AM_SHARE("spriteram")
	AM_RANGE(0x08000, 0x087ff) AM_RAM_WRITE(text_w) AM_SHARE("videoram")
	AM_RANGE(0x09000, 0x0900d) AM_DEVREADWRITE("seibu_sound", seibu_sound_device, main_word_r, main_word_w)
	AM_RANGE(0x0c000, 0x0c0ff) AM_RAM AM_SHARE("scroll_ram")
	AM_RANGE(0x0e000, 0x0efff) AM_RAM AM_SHARE("share1")
	AM_RANGE(0x0f000, 0x0f001) AM_READ_PORT("P1_P2")
	AM_RANGE(0x0f002, 0x0f003) AM_READ_PORT("DSW")
	AM_RANGE(0x0f004, 0x0f005) AM_WRITENOP
	AM_RANGE(0x0f006, 0x0f007) AM_WRITE(control_w)
	AM_RANGE(0xa0000, 0xfffff) AM_ROM
ADDRESS_MAP_END

/* Input Ports */

static INPUT_PORTS_START( dynduke )
	SEIBU_COIN_INPUTS   /* coin inputs read through sound cpu */

	PORT_START("P1_P2")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )    PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )  PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )  PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )    PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )  PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )  PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_START2 )

	PORT_START("DSW")
	PORT_DIPNAME( 0x0001, 0x0001, "Coin Mode" ) PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(      0x0001, "Mode 1" )
	PORT_DIPSETTING(      0x0000, "Mode 2" )
	PORT_DIPNAME( 0x0006, 0x0006, DEF_STR( Coin_A ) ) PORT_DIPLOCATION("SW1:2,3") PORT_CONDITION("DSW", 0x0001, EQUALS, 0x0000)
	PORT_DIPSETTING(      0x0000, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0006, DEF_STR( 1C_1C ) )
	PORT_DIPNAME( 0x0018, 0x0008, DEF_STR( Coin_B ) ) PORT_DIPLOCATION("SW1:4,5") PORT_CONDITION("DSW", 0x0001, EQUALS, 0x0000)
	PORT_DIPSETTING(      0x0018, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x001e, 0x001e, DEF_STR( Coinage ) ) PORT_DIPLOCATION("SW1:2,3,4,5") PORT_CONDITION("DSW", 0x0001, EQUALS, 0x0001)
	PORT_DIPSETTING(      0x0018, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x001a, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x001c, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x001e, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x0020, 0x0020, "Starting Coin" ) PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(      0x0020, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0000, "X 2" )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Cabinet ) ) PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(      0x0040, DEF_STR( Upright ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Flip_Screen ) ) PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_BIT( 0x0300, 0x0300, IPT_UNUSED )  /* "SW2:1,2" - Always OFF according to the manual */
	PORT_DIPNAME( 0x0c00, 0x0400, DEF_STR( Bonus_Life ) ) PORT_DIPLOCATION("SW2:3,4")
	PORT_DIPSETTING(      0x0c00, "80K 100K+" )
	PORT_DIPSETTING(      0x0800, "100K 100K+" )
	PORT_DIPSETTING(      0x0400, "120K 100K+" )
	PORT_DIPSETTING(      0x0000, "120K 120K+" )
	PORT_DIPNAME( 0x3000, 0x3000, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW2:5,6")
	PORT_DIPSETTING(      0x3000, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x4000, 0x4000, DEF_STR( Allow_Continue ) ) PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000, 0x8000, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x8000, DEF_STR( On ) )
INPUT_PORTS_END

/* Graphics Layouts */

static const gfx_layout charlayout =
{
	8,8,        /* 8*8 characters */
	1024,
	4,          /* 4 bits per pixel */
	{ 4,0,(0x10000*8)+4,0x10000*8 },
	{ 0,1,2,3,8,9,10,11 },
	{ 0*16, 1*16, 2*16, 3*16, 4*16, 5*16, 6*16, 7*16 },
	128
};

static const gfx_layout spritelayout =
{
	16,16,  /* 16*16 tiles */
	0x4000,
	4,      /* 4 bits per pixel */
	{ 12, 8, 4, 0 },
	{
	0,1,2,3, 16,17,18,19,
	512+0,512+1,512+2,512+3,
	512+8+8,512+9+8,512+10+8,512+11+8,
	},
	{
	0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32,
	8*32, 9*32, 10*32, 11*32, 12*32, 13*32, 14*32, 15*32,
	},
	1024
};

static const gfx_layout bg_layout =
{
	16,16,
	RGN_FRAC(1,3),
	6,
	{ RGN_FRAC(2,3)+4, RGN_FRAC(2,3)+0,
		RGN_FRAC(1,3)+4, RGN_FRAC(1,3)+0,
					4,               0 },
	{
		0,1,2,3,8,9,10,11,
		256+0,256+1,256+2,256+3,256+8,256+9,256+10,256+11
	},
	{
		0*16,1*16,2*16,3*16,4*16,5*16,6*16,7*16,
		8*16,9*16,10*16,11*16,12*16,13*16,14*16,15*16
	},
	512
};


static const gfx_layout fg_layout =
{
	16,16,
	0x2000,
	4,
	{ 0x80000*8+4, 0x80000*8, 4, 0 },
	{
		0,1,2,3,8,9,10,11,
		256+0,256+1,256+2,256+3,256+8,256+9,256+10,256+11
	},
	{
		0*16,1*16,2*16,3*16,4*16,5*16,6*16,7*16,
		8*16,9*16,10*16,11*16,12*16,13*16,14*16,15*16
	},
	512
};

/* Graphics Decode Information */

static GFXDECODE_START( dynduke )
	GFXDECODE_ENTRY( "gfx1", 0, charlayout,    0x500, 16 )
	GFXDECODE_ENTRY( "gfx2", 0, bg_layout,     0x000, 128 )
	GFXDECODE_ENTRY( "gfx3", 0, fg_layout,     0x200, 16 )
	GFXDECODE_ENTRY( "gfx4", 0, spritelayout,  0x300, 32 )
GFXDECODE_END

/* Interrupt Generator */

INTERRUPT_GEN_MEMBER(dynduke_state::interrupt)
{
	device.execute().set_input_line_and_vector(0, HOLD_LINE, 0xc8/4);   // VBL
}

/* Machine Driver */

static MACHINE_CONFIG_START( dynduke, dynduke_state )
	// basic machine hardware
	MCFG_CPU_ADD("maincpu", V30, 16000000/2) // NEC V30-8 CPU
	MCFG_CPU_PROGRAM_MAP(master_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", dynduke_state, interrupt)

	MCFG_CPU_ADD("slave", V30, 16000000/2) // NEC V30-8 CPU
	MCFG_CPU_PROGRAM_MAP(slave_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", dynduke_state, interrupt)

	SEIBU_SOUND_SYSTEM_CPU(14318180/4)
	SEIBU_SOUND_SYSTEM_ENCRYPTED_FULL()

	MCFG_QUANTUM_TIME(attotime::from_hz(3600))

	// video hardware
	MCFG_BUFFERED_SPRITERAM16_ADD("spriteram")

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500) /* not accurate */)
	MCFG_SCREEN_SIZE(32*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 2*8, 30*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(dynduke_state, screen_update)
	MCFG_SCREEN_VBLANK_DEVICE("spriteram", buffered_spriteram16_device, vblank_copy_rising)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", dynduke)
	MCFG_PALETTE_ADD("palette", 2048)


	// sound hardware
	SEIBU_SOUND_SYSTEM_YM3812_INTERFACE(14318180/4,1320000)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( dbldyn, dynduke )

	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(masterj_map)
MACHINE_CONFIG_END

/* ROMs */

ROM_START( dynduke )
	ROM_REGION( 0x100000, "maincpu", 0 ) /* v30 main cpu */
	ROM_LOAD16_BYTE( "1.cd8", 0x0a0000, 0x10000, CRC(a5e2a95a) SHA1(135d57073d826b9cf46fb43dc49439e1400fb021) )
	ROM_LOAD16_BYTE( "2.cd7", 0x0a0001, 0x10000, CRC(7e51af22) SHA1(b26103c0d41c469d1e2d1e4e89f591c0d9cdb67c) )
	ROM_LOAD16_BYTE( "3.e8",  0x0c0000, 0x20000, CRC(a56f8692) SHA1(00d86c660efae30c008f8220fdfd397b7d69b2cd) )
	ROM_LOAD16_BYTE( "4e.e7", 0x0c0001, 0x20000, CRC(384c0635) SHA1(4b9332d8b91426c17a2b2a58633dc6dde526284d) )

	ROM_REGION( 0x100000, "slave", 0 ) /* v30 sub cpu */
	ROM_LOAD16_BYTE( "5.p8", 0x0e0000, 0x10000, CRC(883d319c) SHA1(b0df05bfe342a5289a6368be26317fa879975463) )
	ROM_LOAD16_BYTE( "6.p7", 0x0e0001, 0x10000, CRC(d94cb4ff) SHA1(653247c420a2af037106470556e6801b29bc58e8) )

	ROM_REGION( 0x20000*2, "audiocpu", 0 ) /* sound Z80 */
	ROM_LOAD( "8.w8",        0x000000, 0x08000, CRC(3c29480b) SHA1(031a0b808df32b5ae4f722c9e9f69554d30505c1) )
	ROM_CONTINUE(            0x010000, 0x08000 )
	ROM_COPY( "audiocpu", 0, 0x018000, 0x08000 )

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "9.5k",   0x000000, 0x04000, CRC(f2bc9af4) SHA1(9092ebe9dced243c3a7f63198a1180143dd42cea) ) /* chars */
	ROM_LOAD( "10.34k", 0x010000, 0x04000, CRC(c2a9f19b) SHA1(0bcea042386109c277c6c5dbe52a020f9ea5972b) )

	ROM_REGION( 0x180000, "gfx2", 0 )
	ROM_LOAD( "dd.a2",  0x000000, 0x40000, CRC(598f343f) SHA1(eee794d9d0a92e066f00818bfb63e8ca46bda764) ) /* background */
	ROM_LOAD( "dd.b2",  0x040000, 0x40000, CRC(41a9088d) SHA1(eb0b7370dc773cb6f5066b044934ffb42bb06587) )
	ROM_LOAD( "dd.c2",  0x080000, 0x40000, CRC(cc341b42) SHA1(8c3cf09a3a0080a1cd7c1049cb8d11f03de50919) )
	ROM_LOAD( "dd.d2",  0x0c0000, 0x40000, CRC(4752b4d7) SHA1(4625b7885ff9d302e78d7324b3592ac5a3cead86) )
	ROM_LOAD( "dd.de3", 0x100000, 0x40000, CRC(44a4cb62) SHA1(70b2043d0428c90ee22ccd479d9710af24d359f6) )
	ROM_LOAD( "dd.ef3", 0x140000, 0x40000, CRC(aa8aee1a) SHA1(8b2b8dcb2287318e314b256f84c23424cfe29462) )

	ROM_REGION( 0x100000, "gfx3", 0 )
	ROM_LOAD( "dd.mn3", 0x000000, 0x40000, CRC(2ee0ca98) SHA1(2ef2c4fd337e0ee4685e4863909985ee0a4c4b91) ) /* foreground */
	ROM_LOAD( "dd.mn4", 0x040000, 0x40000, CRC(6c71e2df) SHA1(fe87277a625010c214e05b43572fadb493b3d05d) )
	ROM_LOAD( "dd.n45", 0x080000, 0x40000, CRC(85d918e1) SHA1(882cdf633288c95f2349d7c86799875b707ca347) )
	ROM_LOAD( "dd.mn5", 0x0c0000, 0x40000, CRC(e71e34df) SHA1(dce8e3de61f3869da57d476bf861856154365058) )

	ROM_REGION( 0x200000, "gfx4", 0 )
	ROM_LOAD16_BYTE( "dd.n1", 0x000000, 0x40000, CRC(cf1db927) SHA1(3fde2ca7e7e302773ae01ed89edf0bcf69fc7aa1) ) /* sprites */
	ROM_LOAD16_BYTE( "dd.n2", 0x000001, 0x40000, CRC(5328150f) SHA1(bb847a2ff7e5ac668e974d2853519d86feb81e03) )
	ROM_LOAD16_BYTE( "dd.m1", 0x080000, 0x40000, CRC(80776452) SHA1(319bfc90ccf04b9e5aaac5701767d3f7bbb71626) )
	ROM_LOAD16_BYTE( "dd.m2", 0x080001, 0x40000, CRC(ff61a573) SHA1(cfbe6c017c276d2fc1f083013b5df3686381753b) )
	ROM_LOAD16_BYTE( "dd.e1", 0x100000, 0x40000, CRC(84a0b87c) SHA1(62075128093f21ee6ea09cc2d4bc8e630b275fce) )
	ROM_LOAD16_BYTE( "dd.e2", 0x100001, 0x40000, CRC(a9585df2) SHA1(2eeac27dd018dd334447d539fdae2989c731e764) )
	ROM_LOAD16_BYTE( "dd.f1", 0x180000, 0x40000, CRC(9aed24ba) SHA1(0068b5bc0d7c817eee3bfbf7de6d19652ba78d41) )
	ROM_LOAD16_BYTE( "dd.f2", 0x180001, 0x40000, CRC(3eb5783f) SHA1(5487ceb4f3241241af1a81b1bb686bd3af10b0d1) )

	ROM_REGION( 0x40000, "oki", 0 ) /* ADPCM samples */
	ROM_LOAD( "7.x10", 0x000000, 0x10000, CRC(9cbc7b41) SHA1(107c19d3d71ee6af63d03f7278310c5e3786f91d) )
ROM_END

ROM_START( dyndukea )
	ROM_REGION( 0x100000, "maincpu", 0 ) /* v30 main cpu */
	ROM_LOAD16_BYTE( "1.cd8",   0x0a0000, 0x10000, CRC(a5e2a95a) SHA1(135d57073d826b9cf46fb43dc49439e1400fb021) )
	ROM_LOAD16_BYTE( "2.cd7",   0x0a0001, 0x10000, CRC(7e51af22) SHA1(b26103c0d41c469d1e2d1e4e89f591c0d9cdb67c) )
	ROM_LOAD16_BYTE( "dde3.e8", 0x0c0000, 0x20000, CRC(95336279) SHA1(0218640e57d0a6df03ce51f2afad9862d4b13a50) )
	ROM_LOAD16_BYTE( "dde4.e7", 0x0c0001, 0x20000, CRC(eb2d8fea) SHA1(d6bb718ece9011f7e24ca1c2f70a513e1c13a7a8) )

	ROM_REGION( 0x100000, "slave", 0 ) /* v30 sub cpu */
	ROM_LOAD16_BYTE( "5.p8", 0x0e0000, 0x10000, CRC(883d319c) SHA1(b0df05bfe342a5289a6368be26317fa879975463) )
	ROM_LOAD16_BYTE( "6.p7", 0x0e0001, 0x10000, CRC(d94cb4ff) SHA1(653247c420a2af037106470556e6801b29bc58e8) )

	ROM_REGION( 0x20000*2, "audiocpu", 0 ) /* sound Z80 */
	ROM_LOAD( "8.w8",        0x000000, 0x08000, CRC(3c29480b) SHA1(031a0b808df32b5ae4f722c9e9f69554d30505c1) )
	ROM_CONTINUE(            0x010000, 0x08000 )
	ROM_COPY( "audiocpu", 0, 0x018000, 0x08000 )

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "9.5k",   0x000000, 0x04000, CRC(f2bc9af4) SHA1(9092ebe9dced243c3a7f63198a1180143dd42cea) ) /* chars */
	ROM_LOAD( "10.34k", 0x010000, 0x04000, CRC(c2a9f19b) SHA1(0bcea042386109c277c6c5dbe52a020f9ea5972b) )

	ROM_REGION( 0x180000, "gfx2", 0 )
	ROM_LOAD( "dd.a2",  0x000000, 0x40000, CRC(598f343f) SHA1(eee794d9d0a92e066f00818bfb63e8ca46bda764) ) /* background */
	ROM_LOAD( "dd.b2",  0x040000, 0x40000, CRC(41a9088d) SHA1(eb0b7370dc773cb6f5066b044934ffb42bb06587) )
	ROM_LOAD( "dd.c2",  0x080000, 0x40000, CRC(cc341b42) SHA1(8c3cf09a3a0080a1cd7c1049cb8d11f03de50919) )
	ROM_LOAD( "dd.d2",  0x0c0000, 0x40000, CRC(4752b4d7) SHA1(4625b7885ff9d302e78d7324b3592ac5a3cead86) )
	ROM_LOAD( "dd.de3", 0x100000, 0x40000, CRC(44a4cb62) SHA1(70b2043d0428c90ee22ccd479d9710af24d359f6) )
	ROM_LOAD( "dd.ef3", 0x140000, 0x40000, CRC(aa8aee1a) SHA1(8b2b8dcb2287318e314b256f84c23424cfe29462) )

	ROM_REGION( 0x100000, "gfx3", 0 )
	ROM_LOAD( "dd.mn3", 0x000000, 0x40000, CRC(2ee0ca98) SHA1(2ef2c4fd337e0ee4685e4863909985ee0a4c4b91) ) /* foreground */
	ROM_LOAD( "dd.mn4", 0x040000, 0x40000, CRC(6c71e2df) SHA1(fe87277a625010c214e05b43572fadb493b3d05d) )
	ROM_LOAD( "dd.n45", 0x080000, 0x40000, CRC(85d918e1) SHA1(882cdf633288c95f2349d7c86799875b707ca347) )
	ROM_LOAD( "dd.mn5", 0x0c0000, 0x40000, CRC(e71e34df) SHA1(dce8e3de61f3869da57d476bf861856154365058) )

	ROM_REGION( 0x200000, "gfx4", 0 )
	ROM_LOAD16_BYTE( "dd.n1", 0x000000, 0x40000, CRC(cf1db927) SHA1(3fde2ca7e7e302773ae01ed89edf0bcf69fc7aa1) ) /* sprites */
	ROM_LOAD16_BYTE( "dd.n2", 0x000001, 0x40000, CRC(5328150f) SHA1(bb847a2ff7e5ac668e974d2853519d86feb81e03) )
	ROM_LOAD16_BYTE( "dd.m1", 0x080000, 0x40000, CRC(80776452) SHA1(319bfc90ccf04b9e5aaac5701767d3f7bbb71626) )
	ROM_LOAD16_BYTE( "dd.m2", 0x080001, 0x40000, CRC(ff61a573) SHA1(cfbe6c017c276d2fc1f083013b5df3686381753b) )
	ROM_LOAD16_BYTE( "dd.e1", 0x100000, 0x40000, CRC(84a0b87c) SHA1(62075128093f21ee6ea09cc2d4bc8e630b275fce) )
	ROM_LOAD16_BYTE( "dd.e2", 0x100001, 0x40000, CRC(a9585df2) SHA1(2eeac27dd018dd334447d539fdae2989c731e764) )
	ROM_LOAD16_BYTE( "dd.f1", 0x180000, 0x40000, CRC(9aed24ba) SHA1(0068b5bc0d7c817eee3bfbf7de6d19652ba78d41) )
	ROM_LOAD16_BYTE( "dd.f2", 0x180001, 0x40000, CRC(3eb5783f) SHA1(5487ceb4f3241241af1a81b1bb686bd3af10b0d1) )

	ROM_REGION( 0x40000, "oki", 0 ) /* ADPCM samples */
	ROM_LOAD( "7.x10", 0x000000, 0x10000, CRC(9cbc7b41) SHA1(107c19d3d71ee6af63d03f7278310c5e3786f91d) )
ROM_END

ROM_START( dyndukej )
	ROM_REGION( 0x100000, "maincpu", 0 ) /* v30 main cpu */
	ROM_LOAD16_BYTE( "1.cd8", 0x0a0000, 0x10000, CRC(a5e2a95a) SHA1(135d57073d826b9cf46fb43dc49439e1400fb021) )
	ROM_LOAD16_BYTE( "2.cd7", 0x0a0001, 0x10000, CRC(7e51af22) SHA1(b26103c0d41c469d1e2d1e4e89f591c0d9cdb67c) )
	ROM_LOAD16_BYTE( "3.e8",  0x0c0000, 0x20000, CRC(98b9d243) SHA1(db00ffafa1353425adb79f5bf6a0cf9223a0d031) ) // sldh
	ROM_LOAD16_BYTE( "4.e7",  0x0c0001, 0x20000, CRC(4f575177) SHA1(837e6bab531f16efb0d21ab5b88c529ee16b40d0) )

	ROM_REGION( 0x100000, "slave", 0 ) /* v30 sub cpu */
	ROM_LOAD16_BYTE( "5.p8", 0x0e0000, 0x10000, CRC(883d319c) SHA1(b0df05bfe342a5289a6368be26317fa879975463) )
	ROM_LOAD16_BYTE( "6.p7", 0x0e0001, 0x10000, CRC(d94cb4ff) SHA1(653247c420a2af037106470556e6801b29bc58e8) )

	ROM_REGION( 0x20000*2, "audiocpu", 0 ) /* sound Z80 */
	ROM_LOAD( "8.w8",        0x000000, 0x08000, CRC(3c29480b) SHA1(031a0b808df32b5ae4f722c9e9f69554d30505c1) )
	ROM_CONTINUE(            0x010000, 0x08000 )
	ROM_COPY( "audiocpu", 0, 0x018000, 0x08000 )

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "9.5k",   0x000000, 0x04000, CRC(f2bc9af4) SHA1(9092ebe9dced243c3a7f63198a1180143dd42cea) ) /* chars */
	ROM_LOAD( "10.34k", 0x010000, 0x04000, CRC(c2a9f19b) SHA1(0bcea042386109c277c6c5dbe52a020f9ea5972b) )

	ROM_REGION( 0x180000, "gfx2", 0 )
	ROM_LOAD( "dd.a2",  0x000000, 0x40000, CRC(598f343f) SHA1(eee794d9d0a92e066f00818bfb63e8ca46bda764) ) /* background */
	ROM_LOAD( "dd.b2",  0x040000, 0x40000, CRC(41a9088d) SHA1(eb0b7370dc773cb6f5066b044934ffb42bb06587) )
	ROM_LOAD( "dd.c2",  0x080000, 0x40000, CRC(cc341b42) SHA1(8c3cf09a3a0080a1cd7c1049cb8d11f03de50919) )
	ROM_LOAD( "dd.d2",  0x0c0000, 0x40000, CRC(4752b4d7) SHA1(4625b7885ff9d302e78d7324b3592ac5a3cead86) )
	ROM_LOAD( "dd.de3", 0x100000, 0x40000, CRC(44a4cb62) SHA1(70b2043d0428c90ee22ccd479d9710af24d359f6) )
	ROM_LOAD( "dd.ef3", 0x140000, 0x40000, CRC(aa8aee1a) SHA1(8b2b8dcb2287318e314b256f84c23424cfe29462) )

	ROM_REGION( 0x100000, "gfx3", 0 )
	ROM_LOAD( "dd.mn3", 0x000000, 0x40000, CRC(2ee0ca98) SHA1(2ef2c4fd337e0ee4685e4863909985ee0a4c4b91) ) /* foreground */
	ROM_LOAD( "dd.mn4", 0x040000, 0x40000, CRC(6c71e2df) SHA1(fe87277a625010c214e05b43572fadb493b3d05d) )
	ROM_LOAD( "dd.n45", 0x080000, 0x40000, CRC(85d918e1) SHA1(882cdf633288c95f2349d7c86799875b707ca347) )
	ROM_LOAD( "dd.mn5", 0x0c0000, 0x40000, CRC(e71e34df) SHA1(dce8e3de61f3869da57d476bf861856154365058) )

	ROM_REGION( 0x200000, "gfx4", 0 )
	ROM_LOAD16_BYTE( "dd.n1", 0x000000, 0x40000, CRC(cf1db927) SHA1(3fde2ca7e7e302773ae01ed89edf0bcf69fc7aa1) ) /* sprites */
	ROM_LOAD16_BYTE( "dd.n2", 0x000001, 0x40000, CRC(5328150f) SHA1(bb847a2ff7e5ac668e974d2853519d86feb81e03) )
	ROM_LOAD16_BYTE( "dd.m1", 0x080000, 0x40000, CRC(80776452) SHA1(319bfc90ccf04b9e5aaac5701767d3f7bbb71626) )
	ROM_LOAD16_BYTE( "dd.m2", 0x080001, 0x40000, CRC(ff61a573) SHA1(cfbe6c017c276d2fc1f083013b5df3686381753b) )
	ROM_LOAD16_BYTE( "dd.e1", 0x100000, 0x40000, CRC(84a0b87c) SHA1(62075128093f21ee6ea09cc2d4bc8e630b275fce) )
	ROM_LOAD16_BYTE( "dd.e2", 0x100001, 0x40000, CRC(a9585df2) SHA1(2eeac27dd018dd334447d539fdae2989c731e764) )
	ROM_LOAD16_BYTE( "dd.f1", 0x180000, 0x40000, CRC(9aed24ba) SHA1(0068b5bc0d7c817eee3bfbf7de6d19652ba78d41) )
	ROM_LOAD16_BYTE( "dd.f2", 0x180001, 0x40000, CRC(3eb5783f) SHA1(5487ceb4f3241241af1a81b1bb686bd3af10b0d1) )

	ROM_REGION( 0x40000, "oki", 0 ) /* ADPCM samples */
	ROM_LOAD( "7.x10", 0x000000, 0x10000, CRC(9cbc7b41) SHA1(107c19d3d71ee6af63d03f7278310c5e3786f91d) )
ROM_END

ROM_START( dyndukeu )
	ROM_REGION( 0x100000, "maincpu", 0 ) /* v30 main cpu */
	ROM_LOAD16_BYTE( "1.cd8",   0x0a0000, 0x10000, CRC(a5e2a95a) SHA1(135d57073d826b9cf46fb43dc49439e1400fb021) )
	ROM_LOAD16_BYTE( "2.cd7",   0x0a0001, 0x10000, CRC(7e51af22) SHA1(b26103c0d41c469d1e2d1e4e89f591c0d9cdb67c) )
	ROM_LOAD16_BYTE( "dd3.ef8", 0x0c0000, 0x20000, CRC(a56f8692) SHA1(00d86c660efae30c008f8220fdfd397b7d69b2cd) )
	ROM_LOAD16_BYTE( "dd4.ef7", 0x0c0001, 0x20000, CRC(ee4b87b3) SHA1(8e470543bce07cd8682f3745e15c4f1141d9549b) )

	ROM_REGION( 0x100000, "slave", 0 ) /* v30 sub cpu */
	ROM_LOAD16_BYTE( "5.p8", 0x0e0000, 0x10000, CRC(883d319c) SHA1(b0df05bfe342a5289a6368be26317fa879975463) )
	ROM_LOAD16_BYTE( "6.p7", 0x0e0001, 0x10000, CRC(d94cb4ff) SHA1(653247c420a2af037106470556e6801b29bc58e8) )

	ROM_REGION( 0x20000*2, "audiocpu", 0 ) /* sound Z80 */
	ROM_LOAD( "8.w8",        0x000000, 0x08000, CRC(3c29480b) SHA1(031a0b808df32b5ae4f722c9e9f69554d30505c1) )
	ROM_CONTINUE(            0x010000, 0x08000 )
	ROM_COPY( "audiocpu", 0, 0x018000, 0x08000 )

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "9.5k",   0x000000, 0x04000, CRC(f2bc9af4) SHA1(9092ebe9dced243c3a7f63198a1180143dd42cea) ) /* chars */
	ROM_LOAD( "10.34k", 0x010000, 0x04000, CRC(c2a9f19b) SHA1(0bcea042386109c277c6c5dbe52a020f9ea5972b) )

	ROM_REGION( 0x180000, "gfx2", 0 )
	ROM_LOAD( "dd.a2",  0x000000, 0x40000, CRC(598f343f) SHA1(eee794d9d0a92e066f00818bfb63e8ca46bda764) ) /* background */
	ROM_LOAD( "dd.b2",  0x040000, 0x40000, CRC(41a9088d) SHA1(eb0b7370dc773cb6f5066b044934ffb42bb06587) )
	ROM_LOAD( "dd.c2",  0x080000, 0x40000, CRC(cc341b42) SHA1(8c3cf09a3a0080a1cd7c1049cb8d11f03de50919) )
	ROM_LOAD( "dd.d2",  0x0c0000, 0x40000, CRC(4752b4d7) SHA1(4625b7885ff9d302e78d7324b3592ac5a3cead86) )
	ROM_LOAD( "dd.de3", 0x100000, 0x40000, CRC(44a4cb62) SHA1(70b2043d0428c90ee22ccd479d9710af24d359f6) )
	ROM_LOAD( "dd.ef3", 0x140000, 0x40000, CRC(aa8aee1a) SHA1(8b2b8dcb2287318e314b256f84c23424cfe29462) )

	ROM_REGION( 0x100000, "gfx3", 0 )
	ROM_LOAD( "dd.mn3", 0x000000, 0x40000, CRC(2ee0ca98) SHA1(2ef2c4fd337e0ee4685e4863909985ee0a4c4b91) ) /* foreground */
	ROM_LOAD( "dd.mn4", 0x040000, 0x40000, CRC(6c71e2df) SHA1(fe87277a625010c214e05b43572fadb493b3d05d) )
	ROM_LOAD( "dd.n45", 0x080000, 0x40000, CRC(85d918e1) SHA1(882cdf633288c95f2349d7c86799875b707ca347) )
	ROM_LOAD( "dd.mn5", 0x0c0000, 0x40000, CRC(e71e34df) SHA1(dce8e3de61f3869da57d476bf861856154365058) )

	ROM_REGION( 0x200000, "gfx4", 0 )
	ROM_LOAD16_BYTE( "dd.n1", 0x000000, 0x40000, CRC(cf1db927) SHA1(3fde2ca7e7e302773ae01ed89edf0bcf69fc7aa1) ) /* sprites */
	ROM_LOAD16_BYTE( "dd.n2", 0x000001, 0x40000, CRC(5328150f) SHA1(bb847a2ff7e5ac668e974d2853519d86feb81e03) )
	ROM_LOAD16_BYTE( "dd.m1", 0x080000, 0x40000, CRC(80776452) SHA1(319bfc90ccf04b9e5aaac5701767d3f7bbb71626) )
	ROM_LOAD16_BYTE( "dd.m2", 0x080001, 0x40000, CRC(ff61a573) SHA1(cfbe6c017c276d2fc1f083013b5df3686381753b) )
	ROM_LOAD16_BYTE( "dd.e1", 0x100000, 0x40000, CRC(84a0b87c) SHA1(62075128093f21ee6ea09cc2d4bc8e630b275fce) )
	ROM_LOAD16_BYTE( "dd.e2", 0x100001, 0x40000, CRC(a9585df2) SHA1(2eeac27dd018dd334447d539fdae2989c731e764) )
	ROM_LOAD16_BYTE( "dd.f1", 0x180000, 0x40000, CRC(9aed24ba) SHA1(0068b5bc0d7c817eee3bfbf7de6d19652ba78d41) )
	ROM_LOAD16_BYTE( "dd.f2", 0x180001, 0x40000, CRC(3eb5783f) SHA1(5487ceb4f3241241af1a81b1bb686bd3af10b0d1) )

	ROM_REGION( 0x40000, "oki", 0 ) /* ADPCM samples */
	ROM_LOAD( "7.x10", 0x000000, 0x10000, CRC(9cbc7b41) SHA1(107c19d3d71ee6af63d03f7278310c5e3786f91d) )
ROM_END

ROM_START( dbldynj )
	ROM_REGION( 0x100000, "maincpu", 0 ) /* v30 main cpu */
	ROM_LOAD16_BYTE( "1.cd8", 0x0a0000, 0x10000, CRC(a5e2a95a) SHA1(135d57073d826b9cf46fb43dc49439e1400fb021) )
	ROM_LOAD16_BYTE( "2.cd7", 0x0a0001, 0x10000, CRC(7e51af22) SHA1(b26103c0d41c469d1e2d1e4e89f591c0d9cdb67c) )
	ROM_LOAD16_BYTE( "3x.e8", 0x0c0000, 0x20000, CRC(633db1fe) SHA1(b8d67c3eedaf72a0d85eff878595af212f1246eb) )
	ROM_LOAD16_BYTE( "4x.e7", 0x0c0001, 0x20000, CRC(dc9ee263) SHA1(786bf36e21d9328662916181ec4b13cce8e14f24) )

	ROM_REGION( 0x100000, "slave", 0 ) /* v30 sub cpu */
	ROM_LOAD16_BYTE( "5x.p8", 0x0e0000, 0x10000, CRC(ea56d719) SHA1(6cade731316c280ef4e809aa700fdbaaabff41d0) )
	ROM_LOAD16_BYTE( "6x.p7", 0x0e0001, 0x10000, CRC(9ffa0ecd) SHA1(a22c46312ab247cd824dadf840cf1f2b0305bb29) )

	ROM_REGION( 0x20000, "audiocpu", 0 ) /* sound Z80 */
	ROM_LOAD( "8x.w8",       0x000000, 0x08000, CRC(f4066081) SHA1(0e5246f4f5513be11e6ed3ea26aada7e0a17a448) )
	ROM_CONTINUE(            0x010000, 0x08000 )
	ROM_COPY( "audiocpu", 0, 0x018000, 0x08000 )

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "9x.5k",   0x000000, 0x04000, CRC(913709e3) SHA1(a469043a09718409f7af835f7c045baedad92061) ) /* chars */
	ROM_LOAD( "10x.34k", 0x010000, 0x04000, CRC(405daacb) SHA1(2b99af73baceb44d7f78aa4a436f6a45538e0876) )

	ROM_REGION( 0x180000, "gfx2", 0 )
	ROM_LOAD( "dd.a2",  0x000000, 0x40000, CRC(598f343f) SHA1(eee794d9d0a92e066f00818bfb63e8ca46bda764) ) /* background */
	ROM_LOAD( "dd.b2",  0x040000, 0x40000, CRC(41a9088d) SHA1(eb0b7370dc773cb6f5066b044934ffb42bb06587) )
	ROM_LOAD( "dd.c2",  0x080000, 0x40000, CRC(cc341b42) SHA1(8c3cf09a3a0080a1cd7c1049cb8d11f03de50919) )
	ROM_LOAD( "dd.d2",  0x0c0000, 0x40000, CRC(4752b4d7) SHA1(4625b7885ff9d302e78d7324b3592ac5a3cead86) )
	ROM_LOAD( "dd.de3", 0x100000, 0x40000, CRC(44a4cb62) SHA1(70b2043d0428c90ee22ccd479d9710af24d359f6) )
	ROM_LOAD( "dd.ef3", 0x140000, 0x40000, CRC(aa8aee1a) SHA1(8b2b8dcb2287318e314b256f84c23424cfe29462) )

	ROM_REGION( 0x100000, "gfx3", 0 )
	ROM_LOAD( "dd.mn3", 0x000000, 0x40000, CRC(2ee0ca98) SHA1(2ef2c4fd337e0ee4685e4863909985ee0a4c4b91) ) /* foreground */
	ROM_LOAD( "dd.mn4", 0x040000, 0x40000, CRC(6c71e2df) SHA1(fe87277a625010c214e05b43572fadb493b3d05d) )
	ROM_LOAD( "dd.n45", 0x080000, 0x40000, CRC(85d918e1) SHA1(882cdf633288c95f2349d7c86799875b707ca347) )
	ROM_LOAD( "dd.mn5", 0x0c0000, 0x40000, CRC(e71e34df) SHA1(dce8e3de61f3869da57d476bf861856154365058) )

	ROM_REGION( 0x200000, "gfx4", 0 )
	ROM_LOAD16_BYTE( "dd.n1", 0x000000, 0x40000, CRC(cf1db927) SHA1(3fde2ca7e7e302773ae01ed89edf0bcf69fc7aa1) ) /* sprites */
	ROM_LOAD16_BYTE( "dd.n2", 0x000001, 0x40000, CRC(5328150f) SHA1(bb847a2ff7e5ac668e974d2853519d86feb81e03) )
	ROM_LOAD16_BYTE( "dd.m1", 0x080000, 0x40000, CRC(80776452) SHA1(319bfc90ccf04b9e5aaac5701767d3f7bbb71626) )
	ROM_LOAD16_BYTE( "dd.m2", 0x080001, 0x40000, CRC(ff61a573) SHA1(cfbe6c017c276d2fc1f083013b5df3686381753b) )
	ROM_LOAD16_BYTE( "dd.e1", 0x100000, 0x40000, CRC(84a0b87c) SHA1(62075128093f21ee6ea09cc2d4bc8e630b275fce) )
	ROM_LOAD16_BYTE( "dd.e2", 0x100001, 0x40000, CRC(a9585df2) SHA1(2eeac27dd018dd334447d539fdae2989c731e764) )
	ROM_LOAD16_BYTE( "dd.f1", 0x180000, 0x40000, CRC(9aed24ba) SHA1(0068b5bc0d7c817eee3bfbf7de6d19652ba78d41) )
	ROM_LOAD16_BYTE( "dd.f2", 0x180001, 0x40000, CRC(3eb5783f) SHA1(5487ceb4f3241241af1a81b1bb686bd3af10b0d1) )

	ROM_REGION( 0x40000, "oki", 0 ) /* ADPCM samples */
	ROM_LOAD( "7.x10", 0x000000, 0x10000, CRC(9cbc7b41) SHA1(107c19d3d71ee6af63d03f7278310c5e3786f91d) )
ROM_END

ROM_START( dbldynu )
	ROM_REGION( 0x100000, "maincpu", 0 ) /* v30 main cpu */
	ROM_LOAD16_BYTE( "1.cd8",   0x0a0000, 0x10000, CRC(a5e2a95a) SHA1(135d57073d826b9cf46fb43dc49439e1400fb021) )
	ROM_LOAD16_BYTE( "2.cd7",   0x0a0001, 0x10000, CRC(7e51af22) SHA1(b26103c0d41c469d1e2d1e4e89f591c0d9cdb67c) )
	ROM_LOAD16_BYTE( "dd3x.8e", 0x0c0000, 0x20000, CRC(9b785028) SHA1(d94c41f9f8969c0effc05d5d6c44474a396a8177) )
	ROM_LOAD16_BYTE( "dd4x.7e", 0x0c0001, 0x20000, CRC(0d0f6350) SHA1(d289bd9ac308ba1079d5b8931cc913fd326129d3) )

	ROM_REGION( 0x100000, "slave", 0 ) /* v30 sub cpu */
	ROM_LOAD16_BYTE( "5x.p8", 0x0e0000, 0x10000, CRC(ea56d719) SHA1(6cade731316c280ef4e809aa700fdbaaabff41d0) )
	ROM_LOAD16_BYTE( "6x.p7", 0x0e0001, 0x10000, CRC(9ffa0ecd) SHA1(a22c46312ab247cd824dadf840cf1f2b0305bb29) )

	ROM_REGION( 0x20000, "audiocpu", 0 ) /* sound Z80 */
	ROM_LOAD( "8x.w8",       0x000000, 0x08000, CRC(f4066081) SHA1(0e5246f4f5513be11e6ed3ea26aada7e0a17a448) )
	ROM_CONTINUE(            0x010000, 0x08000 )
	ROM_COPY( "audiocpu", 0, 0x018000, 0x08000 )

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "9x.5k",   0x000000, 0x04000, CRC(913709e3) SHA1(a469043a09718409f7af835f7c045baedad92061) ) /* chars */
	ROM_LOAD( "10x.34k", 0x010000, 0x04000, CRC(405daacb) SHA1(2b99af73baceb44d7f78aa4a436f6a45538e0876) )

	ROM_REGION( 0x180000, "gfx2", 0 )
	ROM_LOAD( "dd.a2",  0x000000, 0x40000, CRC(598f343f) SHA1(eee794d9d0a92e066f00818bfb63e8ca46bda764) ) /* background */
	ROM_LOAD( "dd.b2",  0x040000, 0x40000, CRC(41a9088d) SHA1(eb0b7370dc773cb6f5066b044934ffb42bb06587) )
	ROM_LOAD( "dd.c2",  0x080000, 0x40000, CRC(cc341b42) SHA1(8c3cf09a3a0080a1cd7c1049cb8d11f03de50919) )
	ROM_LOAD( "dd.d2",  0x0c0000, 0x40000, CRC(4752b4d7) SHA1(4625b7885ff9d302e78d7324b3592ac5a3cead86) )
	ROM_LOAD( "dd.de3", 0x100000, 0x40000, CRC(44a4cb62) SHA1(70b2043d0428c90ee22ccd479d9710af24d359f6) )
	ROM_LOAD( "dd.ef3", 0x140000, 0x40000, CRC(aa8aee1a) SHA1(8b2b8dcb2287318e314b256f84c23424cfe29462) )

	ROM_REGION( 0x100000, "gfx3", 0 )
	ROM_LOAD( "dd.mn3", 0x000000, 0x40000, CRC(2ee0ca98) SHA1(2ef2c4fd337e0ee4685e4863909985ee0a4c4b91) ) /* foreground */
	ROM_LOAD( "dd.mn4", 0x040000, 0x40000, CRC(6c71e2df) SHA1(fe87277a625010c214e05b43572fadb493b3d05d) )
	ROM_LOAD( "dd.n45", 0x080000, 0x40000, CRC(85d918e1) SHA1(882cdf633288c95f2349d7c86799875b707ca347) )
	ROM_LOAD( "dd.mn5", 0x0c0000, 0x40000, CRC(e71e34df) SHA1(dce8e3de61f3869da57d476bf861856154365058) )

	ROM_REGION( 0x200000, "gfx4", 0 )
	ROM_LOAD16_BYTE( "dd.n1", 0x000000, 0x40000, CRC(cf1db927) SHA1(3fde2ca7e7e302773ae01ed89edf0bcf69fc7aa1) ) /* sprites */
	ROM_LOAD16_BYTE( "dd.n2", 0x000001, 0x40000, CRC(5328150f) SHA1(bb847a2ff7e5ac668e974d2853519d86feb81e03) )
	ROM_LOAD16_BYTE( "dd.m1", 0x080000, 0x40000, CRC(80776452) SHA1(319bfc90ccf04b9e5aaac5701767d3f7bbb71626) )
	ROM_LOAD16_BYTE( "dd.m2", 0x080001, 0x40000, CRC(ff61a573) SHA1(cfbe6c017c276d2fc1f083013b5df3686381753b) )
	ROM_LOAD16_BYTE( "dd.e1", 0x100000, 0x40000, CRC(84a0b87c) SHA1(62075128093f21ee6ea09cc2d4bc8e630b275fce) )
	ROM_LOAD16_BYTE( "dd.e2", 0x100001, 0x40000, CRC(a9585df2) SHA1(2eeac27dd018dd334447d539fdae2989c731e764) )
	ROM_LOAD16_BYTE( "dd.f1", 0x180000, 0x40000, CRC(9aed24ba) SHA1(0068b5bc0d7c817eee3bfbf7de6d19652ba78d41) )
	ROM_LOAD16_BYTE( "dd.f2", 0x180001, 0x40000, CRC(3eb5783f) SHA1(5487ceb4f3241241af1a81b1bb686bd3af10b0d1) )

	ROM_REGION( 0x40000, "oki", 0 ) /* ADPCM samples */
	ROM_LOAD( "7.x10", 0x000000, 0x10000, CRC(9cbc7b41) SHA1(107c19d3d71ee6af63d03f7278310c5e3786f91d) )
ROM_END


/* Driver Initialization */

DRIVER_INIT_MEMBER(dynduke_state,dynduke)
{
}

/* Game Drivers */

GAME( 1989, dynduke,  0,       dynduke, dynduke, dynduke_state, dynduke, ROT0, "Seibu Kaihatsu",                  "Dynamite Duke (Europe set 1)", GAME_SUPPORTS_SAVE )
GAME( 1989, dyndukea, dynduke, dynduke, dynduke, dynduke_state, dynduke, ROT0, "Seibu Kaihatsu",                  "Dynamite Duke (Europe set 2)", GAME_SUPPORTS_SAVE )
GAME( 1989, dyndukej, dynduke, dynduke, dynduke, dynduke_state, dynduke, ROT0, "Seibu Kaihatsu",                  "Dynamite Duke (Japan)", GAME_SUPPORTS_SAVE )
GAME( 1989, dyndukeu, dynduke, dynduke, dynduke, dynduke_state, dynduke, ROT0, "Seibu Kaihatsu (Fabtek license)", "Dynamite Duke (US)", GAME_SUPPORTS_SAVE )
GAME( 1989, dbldynj,  dynduke, dbldyn,  dynduke, dynduke_state, dynduke, ROT0, "Seibu Kaihatsu",                  "The Double Dynamites (Japan)", GAME_SUPPORTS_SAVE )
GAME( 1989, dbldynu,  dynduke, dynduke, dynduke, dynduke_state, dynduke, ROT0, "Seibu Kaihatsu (Fabtek license)", "The Double Dynamites (US)", GAME_SUPPORTS_SAVE )
