// license:BSD-3-Clause
// copyright-holders:Zsolt Vasvari
/***************************************************************************

Super Basketball memory map (preliminary)
(Hold down Start 1 & Start 2 keys to enter test mode on start up;
 use Start 1 to change modes)

driver by Zsolt Vasvari

MAIN BOARD:
2000-2fff RAM
3000-33ff Color RAM
3400-37ff Video RAM
3800-39ff Sprite RAM
6000-ffff ROM


Konami designated Super Basketball with the label of GX405.

Super Basketball is composed of two boards.  A Sound board with the
label PWB(B)3000288A and a CPU/Video board with the label
PWB(A)2000177C silked screened onto them.  All Konami custom chips have
had their labels scratched off.

Sound Board Parts:

    VLM5030 @ 11e (According to the schematics, part number scratched off)
    Z80 @ 6a (According to the schematics, part number scratched off)
    14.31818MHz @ x1
    3.579545MHz @ x2
    SN76489AN @ 8d (According to the schematics, part number scratched off)
    4118 or 6116 @ 10a (According to the schematics pins match these two types of sram, part number scratched off)


CPU/Video Board Parts:

    18.432000MHz @ 1f
    M2BC200 (CR2032) Battery @ 19j

***************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "cpu/m6809/m6809.h"
#include "sound/dac.h"
#include "machine/konami1.h"
#include "includes/konamipt.h"
#include "audio/trackfld.h"
#include "includes/sbasketb.h"


WRITE8_MEMBER(sbasketb_state::sbasketb_sh_irqtrigger_w)
{
	m_audiocpu->set_input_line_and_vector(0, HOLD_LINE, 0xff);
}

WRITE8_MEMBER(sbasketb_state::sbasketb_coin_counter_w)
{
	coin_counter_w(machine(), offset, data);
}

WRITE8_MEMBER(sbasketb_state::irq_mask_w)
{
	m_irq_mask = data & 1;
}

static ADDRESS_MAP_START( sbasketb_map, AS_PROGRAM, 8, sbasketb_state )
	AM_RANGE(0x2000, 0x2fff) AM_RAM
	AM_RANGE(0x3000, 0x33ff) AM_RAM_WRITE(sbasketb_colorram_w) AM_SHARE("colorram")
	AM_RANGE(0x3400, 0x37ff) AM_RAM_WRITE(sbasketb_videoram_w) AM_SHARE("videoram")
	AM_RANGE(0x3800, 0x39ff) AM_RAM AM_SHARE("spriteram")
	AM_RANGE(0x3a00, 0x3bff) AM_RAM           /* Probably unused, but initialized */
	AM_RANGE(0x3c00, 0x3c00) AM_WRITE(watchdog_reset_w)
	AM_RANGE(0x3c10, 0x3c10) AM_READNOP    /* ???? */
	AM_RANGE(0x3c20, 0x3c20) AM_WRITEONLY AM_SHARE("palettebank")
	AM_RANGE(0x3c80, 0x3c80) AM_WRITE(sbasketb_flipscreen_w)
	AM_RANGE(0x3c81, 0x3c81) AM_WRITE(irq_mask_w)
	AM_RANGE(0x3c83, 0x3c84) AM_WRITE(sbasketb_coin_counter_w)
	AM_RANGE(0x3c85, 0x3c85) AM_WRITEONLY AM_SHARE("spriteramsel")
	AM_RANGE(0x3d00, 0x3d00) AM_WRITE(soundlatch_byte_w)
	AM_RANGE(0x3d80, 0x3d80) AM_WRITE(sbasketb_sh_irqtrigger_w)
	AM_RANGE(0x3e00, 0x3e00) AM_READ_PORT("SYSTEM")
	AM_RANGE(0x3e01, 0x3e01) AM_READ_PORT("P1")
	AM_RANGE(0x3e02, 0x3e02) AM_READ_PORT("P2")
	AM_RANGE(0x3e03, 0x3e03) AM_READNOP
	AM_RANGE(0x3e80, 0x3e80) AM_READ_PORT("DSW2")
	AM_RANGE(0x3f00, 0x3f00) AM_READ_PORT("DSW1")
	AM_RANGE(0x3f80, 0x3f80) AM_WRITEONLY AM_SHARE("scroll")
	AM_RANGE(0x6000, 0xffff) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START( sbasketb_sound_map, AS_PROGRAM, 8, sbasketb_state )
	AM_RANGE(0x0000, 0x1fff) AM_ROM
	AM_RANGE(0x4000, 0x43ff) AM_RAM
	AM_RANGE(0x6000, 0x6000) AM_READ(soundlatch_byte_r)
	AM_RANGE(0x8000, 0x8000) AM_DEVREAD("trackfld_audio", trackfld_audio_device, hyperspt_sh_timer_r)
	AM_RANGE(0xa000, 0xa000) AM_DEVWRITE("vlm", vlm5030_device, data_w) /* speech data */
	AM_RANGE(0xc000, 0xdfff) AM_DEVWRITE("trackfld_audio", trackfld_audio_device, hyperspt_sound_w)     /* speech and output controll */
	AM_RANGE(0xe000, 0xe000) AM_DEVWRITE("dac", dac_device, write_unsigned8)
	AM_RANGE(0xe001, 0xe001) AM_WRITE(konami_SN76496_latch_w)  /* Loads the snd command into the snd latch */
	AM_RANGE(0xe002, 0xe002) AM_WRITE(konami_SN76496_w)      /* This address triggers the SN chip to read the data port. */
ADDRESS_MAP_END


static INPUT_PORTS_START( sbasketb )
	PORT_START("SYSTEM")
	KONAMI8_SYSTEM_UNK

	PORT_START("P1")
	KONAMI8_MONO_B123_UNK

	PORT_START("P2")
	KONAMI8_COCKTAIL_B123_UNK

	PORT_START("DSW1")
	KONAMI_COINAGE_LOC(DEF_STR( Free_Play ), DEF_STR( Free_Play ), SW1)

	PORT_START("DSW2")
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Game_Time ) )   PORT_DIPLOCATION( "SW2:1,2" )
	PORT_DIPSETTING(    0x03, "30" )
	PORT_DIPSETTING(    0x01, "40" )
	PORT_DIPSETTING(    0x02, "50" )
	PORT_DIPSETTING(    0x00, "60" )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Cabinet ) )     PORT_DIPLOCATION( "SW2:3" )
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x08, 0x08, "Starting Score" )       PORT_DIPLOCATION( "SW2:4" )
	PORT_DIPSETTING(    0x08, "70-78" )
	PORT_DIPSETTING(    0x00, "100-115" )
	PORT_DIPNAME( 0x10, 0x00, "Ranking" )              PORT_DIPLOCATION( "SW2:5" )
	PORT_DIPSETTING(    0x00, "Data Remaining" )
	PORT_DIPSETTING(    0x10, "Data Initialized" )
	PORT_DIPNAME( 0x60, 0x60, DEF_STR( Difficulty ) )  PORT_DIPLOCATION( "SW2:6,7" )
	PORT_DIPSETTING(    0x60, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Medium ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION( "SW2:8" )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END




static const gfx_layout charlayout =
{
	8,8,    /* 8*8 characters */
	512,    /* 512 characters */
	4,      /* 4 bits per pixel */
	{ 0, 1, 2, 3 }, /* the bitplanes are packed */
	{ 0*4, 1*4, 2*4, 3*4, 4*4, 5*4, 6*4, 7*4 },
	{ 0*4*8, 1*4*8, 2*4*8, 3*4*8, 4*4*8, 5*4*8, 6*4*8, 7*4*8 },
	8*4*8     /* every char takes 32 consecutive bytes */
};

static const gfx_layout spritelayout =
{
	16,16,  /* 16*16 sprites */
	128 * 3,/* 384 sprites */
	4,      /* 4 bits per pixel */
	{ 0, 1, 2, 3 },        /* the bitplanes are packed */
	{ 0*4, 1*4,  2*4,  3*4,  4*4,  5*4,  6*4,  7*4,
			8*4, 9*4, 10*4, 11*4, 12*4, 13*4, 14*4, 15*4 },
	{ 0*4*16, 1*4*16,  2*4*16,  3*4*16,  4*4*16,  5*4*16,  6*4*16,  7*4*16,
			8*4*16, 9*4*16, 10*4*16, 11*4*16, 12*4*16, 13*4*16, 14*4*16, 15*4*16 },
	32*4*8    /* every sprite takes 128 consecutive bytes */
};



static GFXDECODE_START( sbasketb )
	GFXDECODE_ENTRY( "gfx1", 0, charlayout,       0, 16 )
	GFXDECODE_ENTRY( "gfx2", 0, spritelayout, 16*16, 16*16 )
GFXDECODE_END

INTERRUPT_GEN_MEMBER(sbasketb_state::vblank_irq)
{
	if(m_irq_mask)
		device.execute().set_input_line(0, HOLD_LINE);
}

static MACHINE_CONFIG_START( sbasketb, sbasketb_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", KONAMI1, 1400000)        /* 1.400 MHz ??? */
	MCFG_CPU_PROGRAM_MAP(sbasketb_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", sbasketb_state,  vblank_irq)

	MCFG_CPU_ADD("audiocpu", Z80, XTAL_14_31818MHz / 4) /* 3.5795 MHz */
	MCFG_CPU_PROGRAM_MAP(sbasketb_sound_map)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(32*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 2*8, 30*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(sbasketb_state, screen_update_sbasketb)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", sbasketb)
	MCFG_PALETTE_ADD("palette", 16*16+16*16*16)
	MCFG_PALETTE_INDIRECT_ENTRIES(256)
	MCFG_PALETTE_INIT_OWNER(sbasketb_state, sbasketb)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("trackfld_audio", TRACKFLD_AUDIO, 0)

	MCFG_DAC_ADD("dac")
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.80)

	MCFG_SOUND_ADD("snsnd", SN76489, XTAL_14_31818MHz / 8)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)

	MCFG_SOUND_ADD("vlm", VLM5030, XTAL_3_579545MHz) /* Schematics say 3.58MHz, but board uses 3.579545MHz xtal */
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_CONFIG_END

MACHINE_CONFIG_DERIVED(sbasketbu, sbasketb)
	MCFG_DEVICE_REMOVE("maincpu")
	MCFG_CPU_ADD("maincpu", M6809, 1400000)        /* 1.400 MHz ??? */
	MCFG_CPU_PROGRAM_MAP(sbasketb_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", sbasketb_state,  vblank_irq)
MACHINE_CONFIG_END



/***************************************************************************

  Game driver(s)

***************************************************************************/


/*
    Super Basketball (version I, encrypted)
*/

ROM_START( sbasketb )
	ROM_REGION( 0x10000, "maincpu", 0 ) /* roms located on the CPU/Video board */
	ROM_LOAD( "405g05.14j", 0x6000, 0x2000, CRC(336dc0ab) SHA1(0fe47fdbf183683c569785fc6b980337a9cfde95) )
	ROM_LOAD( "405i03.11j", 0x8000, 0x4000, CRC(d33b82dd) SHA1(9f0a1e2b0a43a2ec5029dd50dbd315291838fa39) )
	ROM_LOAD( "405i01.9j",  0xc000, 0x4000, CRC(1c09cc3f) SHA1(881c0a9313f7f1ca17e1fa956a7b13e77d71957c) )

	ROM_REGION( 0x10000, "audiocpu", 0 ) /* roms located on Sound Board  */
	ROM_LOAD( "405e13.7a",  0x0000, 0x2000, CRC(1ec7458b) SHA1(a015b982bff5f9e7ece33f2e69ff8c6c2174e710) )

	ROM_REGION( 0x04000, "gfx1", 0 ) /* roms located on the CPU/Video board */
	ROM_LOAD( "405e12.22f", 0x0000, 0x4000, CRC(e02c54da) SHA1(2fa19f3bce894ef05820f95e0b88428e4f946a35) )

	ROM_REGION( 0x0c000, "gfx2", 0 ) /* roms located on the CPU/Video board */
	ROM_LOAD( "405h06.14g", 0x0000, 0x4000, CRC(cfbbff07) SHA1(39b19866b21372524933b5eef511bb5b7ad92556) )
	ROM_LOAD( "405h08.17g", 0x4000, 0x4000, CRC(c75901b6) SHA1(4ff87123228da068f0c0ffffa4a3f03765eccd8d) )
	ROM_LOAD( "405h10.20g", 0x8000, 0x4000, CRC(95bc5942) SHA1(55bf35283385d0ae768210706720a3b289ebd9a2) )

	ROM_REGION( 0x0500, "proms", 0 ) /* roms located on the CPU/Video board */
	ROM_LOAD( "405e17.5a",  0x0000, 0x0100, CRC(b4c36d57) SHA1(c4a63f57edce2b9588e2394ff54a28f91213d550) ) /* palette red component */
	ROM_LOAD( "405e16.4a",  0x0100, 0x0100, CRC(0b7b03b8) SHA1(81297cb2b0b28b0fc0939a37ff30844d69fb65ac) ) /* palette green component */
	ROM_LOAD( "405e18.6a",  0x0200, 0x0100, CRC(9e533bad) SHA1(611e7af6813caaf2bc36c311ae48a5efd30e6f0c) ) /* palette blue component */
	ROM_LOAD( "405e20.19d", 0x0300, 0x0100, CRC(8ca6de2f) SHA1(67d29708d1a07d17c5dc5793a3293e7ace3a4e19) ) /* character lookup table */
	ROM_LOAD( "405e19.16d", 0x0400, 0x0100, CRC(e0bc782f) SHA1(9f71e696d11a60f771535f6837ecad6132047b0a) ) /* sprite lookup table */

	ROM_REGION( 0x10000, "vlm", 0 ) /* 64k for speech rom, located on Sound Board */
	ROM_LOAD( "405e15.11f", 0x0000, 0x2000, CRC(01bb5ce9) SHA1(f48477b4011befba13c8bcd83e0c9f7deb14a1e1) )
ROM_END

/*
    Super Basketball (version H, unprotected)

    Jumper Settings for CPU/Video Board

        JP6 set to 27128 with a null resistor
        JP5 set to A with a null resistor
        JP4 set to 27128 with a null resistor
        JP3 connected with a solder blob
        JP2 connected

    Jumper Settings for Sound Board

        J1 connected with a null resistor
*/

ROM_START( sbasketh )
	ROM_REGION( 0x10000, "maincpu", 0 ) /* roms located on the CPU/Video board */
	ROM_LOAD( "405h05.14j", 0x6000, 0x2000, CRC(263ec36b) SHA1(b445b600726ba4935623311e1a178aeb4a356b0a) )
	ROM_LOAD( "405h03.11j", 0x8000, 0x4000, CRC(0a4d7a82) SHA1(2e0153b41e23284427881258a44bd55be3570eb2) )
	ROM_LOAD( "405h01.9j",  0xc000, 0x4000, CRC(4f9dd9a0) SHA1(97f4c208509d50a7ce4c1ebe8a3f643ad75e833b) )

	ROM_REGION( 0x10000, "audiocpu", 0 ) /* roms located on Sound Board  */
	ROM_LOAD( "405e13.7a",  0x0000, 0x2000, CRC(1ec7458b) SHA1(a015b982bff5f9e7ece33f2e69ff8c6c2174e710) )

	ROM_REGION( 0x04000, "gfx1", 0 ) /* roms located on the CPU/Video board */
	ROM_LOAD( "405e12.22f", 0x0000, 0x4000, CRC(e02c54da) SHA1(2fa19f3bce894ef05820f95e0b88428e4f946a35) )

	ROM_REGION( 0x0c000, "gfx2", 0 ) /* roms located on the CPU/Video board */
	ROM_LOAD( "405h06.14g", 0x0000, 0x4000, CRC(cfbbff07) SHA1(39b19866b21372524933b5eef511bb5b7ad92556) )
	ROM_LOAD( "405h08.17g", 0x4000, 0x4000, CRC(c75901b6) SHA1(4ff87123228da068f0c0ffffa4a3f03765eccd8d) )
	ROM_LOAD( "405h10.20g", 0x8000, 0x4000, CRC(95bc5942) SHA1(55bf35283385d0ae768210706720a3b289ebd9a2) )

	ROM_REGION( 0x0500, "proms", 0 ) /* roms located on the CPU/Video board */
	ROM_LOAD( "405e17.5a",  0x0000, 0x0100, CRC(b4c36d57) SHA1(c4a63f57edce2b9588e2394ff54a28f91213d550) ) /* palette red component */
	ROM_LOAD( "405e16.4a",  0x0100, 0x0100, CRC(0b7b03b8) SHA1(81297cb2b0b28b0fc0939a37ff30844d69fb65ac) ) /* palette green component */
	ROM_LOAD( "405e18.6a",  0x0200, 0x0100, CRC(9e533bad) SHA1(611e7af6813caaf2bc36c311ae48a5efd30e6f0c) ) /* palette blue component */
	ROM_LOAD( "405e20.19d", 0x0300, 0x0100, CRC(8ca6de2f) SHA1(67d29708d1a07d17c5dc5793a3293e7ace3a4e19) ) /* character lookup table */
	ROM_LOAD( "405e19.16d", 0x0400, 0x0100, CRC(e0bc782f) SHA1(9f71e696d11a60f771535f6837ecad6132047b0a) ) /* sprite lookup table */

	ROM_REGION( 0x10000, "vlm", 0 ) /* 64k for speech rom, located on Sound Board */
	ROM_LOAD( "405e15.11f", 0x0000, 0x2000, CRC(01bb5ce9) SHA1(f48477b4011befba13c8bcd83e0c9f7deb14a1e1) )
ROM_END


/*
    Super Basketball (version G, encrypted)

    Jumper Settings for CPU/Video Board

        JP6 set to 2764 with a null resistor
        JP5 set to A with a null resistor
        JP4 set to 2764 with a null resistor
        JP3 not connected
        JP2 connected

    Jumper Settings for Sound Board

        J1 connected with a null resistor
*/

ROM_START( sbasketg )
	ROM_REGION( 0x10000, "maincpu", 0 ) /* roms located on the CPU/Video board */
	ROM_LOAD( "405g05.14j", 0x6000, 0x2000, CRC(336dc0ab) SHA1(0fe47fdbf183683c569785fc6b980337a9cfde95) )
	ROM_LOAD( "405g04.13j", 0x8000, 0x2000, CRC(f064a9bc) SHA1(4f1b94a880385c6ba74cc0883b24f6fec934e35d) )
	ROM_LOAD( "405g03.11j", 0xa000, 0x2000, CRC(b9de7d53) SHA1(5a4e5491ff3511992d949367fd7b5d383c2727db) )
	ROM_LOAD( "405g02.10j", 0xc000, 0x2000, CRC(e98470a0) SHA1(79af25af941fe357a8c9f0a2f11e5558670b8027) )
	ROM_LOAD( "405g01.9j",  0xe000, 0x2000, CRC(1bd0cd2e) SHA1(d162f9b989f718d9882a02a8c64743adf3d8e239) )

	ROM_REGION( 0x10000, "audiocpu", 0 ) /* roms located on Sound Board  */
	ROM_LOAD( "405e13.7a",  0x0000, 0x2000, CRC(1ec7458b) SHA1(a015b982bff5f9e7ece33f2e69ff8c6c2174e710) )

	ROM_REGION( 0x04000, "gfx1", 0 ) /* roms located on the CPU/Video board */
	ROM_LOAD( "405e12.22f", 0x0000, 0x4000, CRC(e02c54da) SHA1(2fa19f3bce894ef05820f95e0b88428e4f946a35) )

	ROM_REGION( 0x0c000, "gfx2", 0 ) /* roms located on the CPU/Video board */
	ROM_LOAD( "405e06.14g", 0x0000, 0x2000, CRC(7e2f5bb2) SHA1(e22008c0ef7ae000dcca7f43a386d43064aaea62) )
	ROM_LOAD( "405e07.16g", 0x2000, 0x2000, CRC(963a44f9) SHA1(03cd7699668b010f27af025ba6bd44509526ec7b) )
	ROM_LOAD( "405e08.17g", 0x4000, 0x2000, CRC(63901deb) SHA1(c65d896298846ed8b70a4d38b32820746214fa5c) )
	ROM_LOAD( "405e09.19g", 0x6000, 0x2000, CRC(e1873677) SHA1(19788e43cc1a6cf5ab375cbc2c745bb6cc8c163d) )
	ROM_LOAD( "405e10.20g", 0x8000, 0x2000, CRC(824815e8) SHA1(470e9d74fa2c397605a74e0bf173a6d9db4cc721) )
	ROM_LOAD( "405e11.22g", 0xa000, 0x2000, CRC(dca9b447) SHA1(12d7e85dc2fc6bd4ea7ad9035ae0b7487e4bc4bc) )

	ROM_REGION( 0x0500, "proms", 0 ) /* roms located on the CPU/Video board */
	ROM_LOAD( "405e17.5a",  0x0000, 0x0100, CRC(b4c36d57) SHA1(c4a63f57edce2b9588e2394ff54a28f91213d550) ) /* palette red component */
	ROM_LOAD( "405e16.4a",  0x0100, 0x0100, CRC(0b7b03b8) SHA1(81297cb2b0b28b0fc0939a37ff30844d69fb65ac) ) /* palette green component */
	ROM_LOAD( "405e18.6a",  0x0200, 0x0100, CRC(9e533bad) SHA1(611e7af6813caaf2bc36c311ae48a5efd30e6f0c) ) /* palette blue component */
	ROM_LOAD( "405e20.19d", 0x0300, 0x0100, CRC(8ca6de2f) SHA1(67d29708d1a07d17c5dc5793a3293e7ace3a4e19) ) /* character lookup table */
	ROM_LOAD( "405e19.16d", 0x0400, 0x0100, CRC(e0bc782f) SHA1(9f71e696d11a60f771535f6837ecad6132047b0a) ) /* sprite lookup table */

	ROM_REGION( 0x10000, "vlm", 0 ) /* 64k for speech rom, located on Sound Board */
	ROM_LOAD( "405e15.11f", 0x0000, 0x2000, CRC(01bb5ce9) SHA1(f48477b4011befba13c8bcd83e0c9f7deb14a1e1) )
ROM_END


/*
    Super Basketball (version E, encrypted)
*/

ROM_START( sbaskete )
	ROM_REGION( 0x10000, "maincpu", 0 ) /* roms located on the CPU/Video board */
	ROM_LOAD( "405e05.14j", 0x6000, 0x2000, CRC(32ea5b71) SHA1(d917c31d2c9a7229396e4a930e8d27394329533a) )
	ROM_LOAD( "405e04.13j", 0x8000, 0x2000, CRC(7abf3087) SHA1(fbaaaaae0b8bed1bc6ad7f2da267c2ef8bd75b15) )
	ROM_LOAD( "405e03.11j", 0xa000, 0x2000, CRC(9c6fcdcd) SHA1(a644ec98f49f84311829149c181aba25e7681793) )
	ROM_LOAD( "405e02.10j", 0xc000, 0x2000, CRC(0f145648) SHA1(2e238eb0663295887bf6b4905f1fd386db16d82a) )
	ROM_LOAD( "405e01.9j",  0xe000, 0x2000, CRC(6a27f1b1) SHA1(38c0be98fb122a7a6ed833af011bda5663a06510) )

	ROM_REGION( 0x10000, "audiocpu", 0 ) /* roms located on Sound Board  */
	ROM_LOAD( "405e13.7a",  0x0000, 0x2000, CRC(1ec7458b) SHA1(a015b982bff5f9e7ece33f2e69ff8c6c2174e710) )

	ROM_REGION( 0x04000, "gfx1", 0 ) /* roms located on the CPU/Video board */
	ROM_LOAD( "405e12.22f", 0x0000, 0x4000, CRC(e02c54da) SHA1(2fa19f3bce894ef05820f95e0b88428e4f946a35) )

	ROM_REGION( 0x0c000, "gfx2", 0 ) /* roms located on the CPU/Video board */
	ROM_LOAD( "405e06.14g", 0x0000, 0x2000, CRC(7e2f5bb2) SHA1(e22008c0ef7ae000dcca7f43a386d43064aaea62) )
	ROM_LOAD( "405e07.16g", 0x2000, 0x2000, CRC(963a44f9) SHA1(03cd7699668b010f27af025ba6bd44509526ec7b) )
	ROM_LOAD( "405e08.17g", 0x4000, 0x2000, CRC(63901deb) SHA1(c65d896298846ed8b70a4d38b32820746214fa5c) )
	ROM_LOAD( "405e09.19g", 0x6000, 0x2000, CRC(e1873677) SHA1(19788e43cc1a6cf5ab375cbc2c745bb6cc8c163d) )
	ROM_LOAD( "405e10.20g", 0x8000, 0x2000, CRC(824815e8) SHA1(470e9d74fa2c397605a74e0bf173a6d9db4cc721) )
	ROM_LOAD( "405e11.22g", 0xa000, 0x2000, CRC(dca9b447) SHA1(12d7e85dc2fc6bd4ea7ad9035ae0b7487e4bc4bc) )

	ROM_REGION( 0x0500, "proms", 0 ) /* roms located on the CPU/Video board */
	ROM_LOAD( "405e17.5a",  0x0000, 0x0100, CRC(b4c36d57) SHA1(c4a63f57edce2b9588e2394ff54a28f91213d550) ) /* palette red component */
	ROM_LOAD( "405e16.4a",  0x0100, 0x0100, CRC(0b7b03b8) SHA1(81297cb2b0b28b0fc0939a37ff30844d69fb65ac) ) /* palette green component */
	ROM_LOAD( "405e18.6a",  0x0200, 0x0100, CRC(9e533bad) SHA1(611e7af6813caaf2bc36c311ae48a5efd30e6f0c) ) /* palette blue component */
	ROM_LOAD( "405e20.19d", 0x0300, 0x0100, CRC(8ca6de2f) SHA1(67d29708d1a07d17c5dc5793a3293e7ace3a4e19) ) /* character lookup table */
	ROM_LOAD( "405e19.16d", 0x0400, 0x0100, CRC(e0bc782f) SHA1(9f71e696d11a60f771535f6837ecad6132047b0a) ) /* sprite lookup table */

	ROM_REGION( 0x10000, "vlm", 0 ) /* 64k for speech rom, located on Sound Board */
	ROM_LOAD( "405e15.11f", 0x0000, 0x2000, CRC(01bb5ce9) SHA1(f48477b4011befba13c8bcd83e0c9f7deb14a1e1) )
ROM_END


DRIVER_INIT_MEMBER(sbasketb_state,sbasketb)
{
}

GAME( 1984, sbasketb, 0,        sbasketb,  sbasketb, sbasketb_state, sbasketb, ROT90, "Konami", "Super Basketball (version I, encrypted)", MACHINE_SUPPORTS_SAVE )
GAME( 1984, sbasketh, sbasketb, sbasketbu, sbasketb, driver_device, 0,        ROT90, "Konami", "Super Basketball (version H, unprotected)", MACHINE_SUPPORTS_SAVE )
GAME( 1984, sbasketg, sbasketb, sbasketb,  sbasketb, sbasketb_state, sbasketb, ROT90, "Konami", "Super Basketball (version G, encrypted)", MACHINE_SUPPORTS_SAVE )
GAME( 1984, sbaskete, sbasketb, sbasketb,  sbasketb, sbasketb_state, sbasketb, ROT90, "Konami", "Super Basketball (version E, encrypted)", MACHINE_SUPPORTS_SAVE )
