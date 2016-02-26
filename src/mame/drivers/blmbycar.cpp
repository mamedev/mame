// license:BSD-3-Clause
// copyright-holders:Luca Elia
/***************************************************************************

                              -= Blomby Car =-

                    driver by   Luca Elia (l.elia@tin.it)


Main  CPU    :  68000
Video Chips  :  TI TPC1020AFN-084 (= Actel A1020A PL84C 9548)
Sound Chips  :  K-665 9546 (= M6295)

To Do:

- Flip screen unused ?
- Better driving wheel(s) support

Blomby Car is said to be a bootleg of Gaelco's World Rally and uses many
of the same fonts

Waterball

Check game speed, it depends on a bit we toggle..

***************************************************************************/

#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "sound/okim6295.h"
#include "includes/blmbycar.h"


/***************************************************************************


                                Sound Banking


***************************************************************************/

/* The top 64k of samples are banked (16 banks total) */

WRITE16_MEMBER(blmbycar_state::okibank_w)
{
	if (ACCESSING_BITS_0_7)
	{
		membank("okibank")->set_entry(data & 0x0f);
	}
}

/***************************************************************************


                                Input Handling


***************************************************************************/

/* Preliminary potentiometric wheel support */

WRITE16_MEMBER(blmbycar_state::blmbycar_pot_wheel_reset_w)
{
	if (ACCESSING_BITS_0_7)
		m_pot_wheel = ~ioport("WHEEL")->read() & 0xff;
}

WRITE16_MEMBER(blmbycar_state::blmbycar_pot_wheel_shift_w)
{
	if (ACCESSING_BITS_0_7)
	{
		if ( ((m_old_val & 0xff) == 0xff) && ((data & 0xff) == 0) )
			m_pot_wheel <<= 1;
		m_old_val = data;
	}
}

READ16_MEMBER(blmbycar_state::blmbycar_pot_wheel_r)
{
	return ((m_pot_wheel & 0x80) ? 0x04 : 0) | (machine().rand() & 0x08);
}


/* Preliminary optical wheel support */

READ16_MEMBER(blmbycar_state::blmbycar_opt_wheel_r)
{
	return (~ioport("WHEEL")->read() & 0xff) << 8;
}


/***************************************************************************


                                Memory Maps


***************************************************************************/

static ADDRESS_MAP_START( common_map, AS_PROGRAM, 16, blmbycar_state )
	AM_RANGE(0x000000, 0x0fffff) AM_ROM
	AM_RANGE(0xfec000, 0xfeffff) AM_RAM
	AM_RANGE(0x100000, 0x103fff) AM_WRITEONLY                                               // ???
	AM_RANGE(0x104000, 0x105fff) AM_RAM_WRITE(vram_1_w) AM_SHARE("vram_1") // Layer 1
	AM_RANGE(0x106000, 0x107fff) AM_RAM_WRITE(vram_0_w) AM_SHARE("vram_0") // Layer 0
	AM_RANGE(0x108000, 0x10bfff) AM_WRITEONLY                                               // ???
	AM_RANGE(0x10c000, 0x10c003) AM_WRITEONLY AM_SHARE("scroll_1")              // Scroll 1
	AM_RANGE(0x10c004, 0x10c007) AM_WRITEONLY AM_SHARE("scroll_0")              // Scroll 0
	AM_RANGE(0x200000, 0x2005ff) AM_RAM_DEVWRITE("palette", palette_device, write) AM_SHARE("palette") AM_MIRROR(0x4000) // Palette
	AM_RANGE(0x200600, 0x203fff) AM_RAM AM_MIRROR(0x4000)
	AM_RANGE(0x440000, 0x441fff) AM_RAM
	AM_RANGE(0x444000, 0x445fff) AM_WRITEONLY AM_SHARE("spriteram")// Sprites (size?)
	AM_RANGE(0x700000, 0x700001) AM_READ_PORT("DSW")
	AM_RANGE(0x700002, 0x700003) AM_READ_PORT("P1_P2")
	AM_RANGE(0x70000c, 0x70000d) AM_WRITE(okibank_w)                               // Sound
	AM_RANGE(0x70000e, 0x70000f) AM_DEVREADWRITE8("oki", okim6295_device, read, write, 0x00ff)  // Sound
ADDRESS_MAP_END

static ADDRESS_MAP_START( blmbycar_map, AS_PROGRAM, 16, blmbycar_state )
	AM_IMPORT_FROM(common_map)

	AM_RANGE(0x700004, 0x700005) AM_READ(blmbycar_opt_wheel_r)                              // Wheel (optical)
	AM_RANGE(0x700006, 0x700007) AM_READ_PORT("UNK")
	AM_RANGE(0x700008, 0x700009) AM_READ(blmbycar_pot_wheel_r)                              // Wheel (potentiometer)
	AM_RANGE(0x70000a, 0x70000b) AM_WRITENOP                                                // ? Wheel
	AM_RANGE(0x70006a, 0x70006b) AM_WRITE(blmbycar_pot_wheel_reset_w)                       // Wheel (potentiometer)
	AM_RANGE(0x70007a, 0x70007b) AM_WRITE(blmbycar_pot_wheel_shift_w)                       //
ADDRESS_MAP_END

READ16_MEMBER(blmbycar_state::waterball_unk_r)
{
	m_retvalue ^= 0x0008; // must toggle.. but not vblank?
	return m_retvalue;
}

static ADDRESS_MAP_START( watrball_map, AS_PROGRAM, 16, blmbycar_state )
	AM_IMPORT_FROM(common_map)

	AM_RANGE(0x700006, 0x700007) AM_READNOP                                                 // read
	AM_RANGE(0x700008, 0x700009) AM_READ(waterball_unk_r)                                   // 0x0008 must toggle
	AM_RANGE(0x70000a, 0x70000b) AM_WRITEONLY                                               // ?? busy
ADDRESS_MAP_END

static ADDRESS_MAP_START( blmbycar_oki_map, AS_0, 8, blmbycar_state )
	AM_RANGE(0x00000, 0x2ffff) AM_ROM
	AM_RANGE(0x30000, 0x3ffff) AM_ROMBANK("okibank")
ADDRESS_MAP_END

/***************************************************************************


                                Input Ports


***************************************************************************/

static INPUT_PORTS_START( blmbycar )

	PORT_START("DSW")       /* $700000.w */
	PORT_DIPNAME( 0x0003, 0x0003, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW1:8,7")
	PORT_DIPSETTING(      0x0002, DEF_STR( Easy )    )
	PORT_DIPSETTING(      0x0003, DEF_STR( Normal )  )
	PORT_DIPSETTING(      0x0001, DEF_STR( Hard )    )
	PORT_DIPSETTING(      0x0000, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x0004, 0x0004, "Joysticks" ) PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(      0x0000, "1" )
	PORT_DIPSETTING(      0x0004, "2" )
	PORT_DIPNAME( 0x0018, 0x0018, DEF_STR( Controls ) ) PORT_DIPLOCATION("SW1:5,4")
	PORT_DIPSETTING(      0x0018, DEF_STR( Joystick ) )
//  PORT_DIPSETTING(      0x0010, "Pot Wheel" ) // Preliminary
//  PORT_DIPSETTING(      0x0008, "Opt Wheel" ) // Preliminary
//  PORT_DIPSETTING(      0x0000, DEF_STR( Unused ) )   // Time goes to 0 rally fast!
	PORT_DIPNAME( 0x0020, 0x0000, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, "Unknown 1-2" ) PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_SERVICE_DIPLOC(  0x0080, IP_ACTIVE_LOW, "SW1:1" )

	PORT_DIPNAME( 0x0700, 0x0700, DEF_STR( Coin_A ) ) PORT_DIPLOCATION("SW2:8,7,6")
	PORT_DIPSETTING(      0x0700, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 3C_4C ) )
	PORT_DIPSETTING(      0x0100, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(      0x0600, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0500, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x0300, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( 1C_6C ) )
	PORT_DIPNAME( 0x3800, 0x3800, DEF_STR( Coin_B ) ) PORT_DIPLOCATION("SW2:5,4,3")
	PORT_DIPSETTING(      0x1000, DEF_STR( 6C_1C ) )
	PORT_DIPSETTING(      0x1800, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x2800, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x3000, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 4C_3C ) )
	PORT_DIPSETTING(      0x3800, DEF_STR( 1C_1C ) )
	PORT_DIPNAME( 0x4000, 0x4000, "Credits To Start" ) PORT_DIPLOCATION("SW2:2")
	PORT_DIPSETTING(      0x4000, "1" )
	PORT_DIPSETTING(      0x0000, "2" )
	PORT_DIPNAME( 0x8000, 0x8000, DEF_STR( Free_Play ) ) PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(      0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )

	PORT_START("P1_P2") /* $700002.w */
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN   ) PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_COIN1  )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_COIN2  )

	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(2)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN   ) PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_START1  )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_START2  )

	PORT_START("WHEEL") /* $700004.w */
	PORT_BIT ( 0x00ff, 0x0080, IPT_AD_STICK_X ) PORT_SENSITIVITY(30) PORT_KEYDELTA(1)

	PORT_START("UNK")       /* $700006.w */
	PORT_BIT( 0xffff, IP_ACTIVE_LOW, IPT_UNKNOWN )

INPUT_PORTS_END


static INPUT_PORTS_START( watrball )
	PORT_START("DSW")
	PORT_DIPNAME( 0x0003, 0x0003, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW1:8,7") /* Affects timer */
	PORT_DIPSETTING(      0x0002, DEF_STR( Easy )    )    /* 180 Seconds */
	PORT_DIPSETTING(      0x0003, DEF_STR( Normal )  )    /* 150 Seconds */
	PORT_DIPSETTING(      0x0001, DEF_STR( Hard )    )    /* 120 Seconds */
	PORT_DIPSETTING(      0x0000, DEF_STR( Hardest ) )    /* 100 Seconds */
	PORT_DIPUNUSED_DIPLOC( 0x0004, 0x0004, "SW1:6" )
	PORT_DIPUNUSED_DIPLOC( 0x0008, 0x0008, "SW1:5" )
	PORT_DIPUNUSED_DIPLOC( 0x0010, 0x0010, "SW1:4" )
	PORT_DIPNAME( 0x0020, 0x0000, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, "Unknown 1-2" ) PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_SERVICE_DIPLOC(  0x0080, IP_ACTIVE_LOW, "SW1:1" )

	PORT_DIPNAME( 0x0700, 0x0700, DEF_STR( Coin_A ) ) PORT_DIPLOCATION("SW2:8,7,6")
	PORT_DIPSETTING(      0x0700, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 3C_4C ) )
	PORT_DIPSETTING(      0x0100, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(      0x0600, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0500, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x0300, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( 1C_6C ) )
	PORT_DIPNAME( 0x3800, 0x3800, DEF_STR( Coin_B ) ) PORT_DIPLOCATION("SW2:5,4,3")
	PORT_DIPSETTING(      0x1000, DEF_STR( 6C_1C ) )
	PORT_DIPSETTING(      0x1800, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x2800, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x3000, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 4C_3C ) )
	PORT_DIPSETTING(      0x3800, DEF_STR( 1C_1C ) )
	PORT_DIPUNUSED_DIPLOC( 0x4000, 0x4000, "SW2:2" )
	PORT_DIPUNUSED_DIPLOC( 0x8000, 0x8000, "SW2:1" )

	PORT_START("P1_P2")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN   ) PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_COIN1  )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_COIN2  )

	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(2)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN   ) PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_START1  )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_START2  )
INPUT_PORTS_END

/***************************************************************************


                                Graphics Layouts


***************************************************************************/

/* 16x16x4 tiles (made of four 8x8 tiles) */
static const gfx_layout layout_16x16x4 =
{
	16,16,
	RGN_FRAC(1,4),
	4,
	{ RGN_FRAC(3,4),RGN_FRAC(2,4),RGN_FRAC(1,4),RGN_FRAC(0,4) },
	{ STEP8(0,1), STEP8(8*8*2,1) },
	{ STEP8(0,8), STEP8(8*8*1,8) },
	16*16
};

/* Layers both use the first $20 color codes. Sprites the next $10 */
static GFXDECODE_START( blmbycar )
	GFXDECODE_ENTRY( "sprites", 0, layout_16x16x4, 0x0, 0x30 ) // [0] Layers + Sprites
GFXDECODE_END



/***************************************************************************


                                Machine Drivers


***************************************************************************/

MACHINE_START_MEMBER(blmbycar_state,blmbycar)
{
	save_item(NAME(m_pot_wheel));
	save_item(NAME(m_old_val));

	membank("okibank")->configure_entries(0, 16, memregion("oki")->base(), 0x10000);
}

MACHINE_RESET_MEMBER(blmbycar_state,blmbycar)
{
	m_pot_wheel = 0;
	m_old_val = 0;
}


static MACHINE_CONFIG_START( blmbycar, blmbycar_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68000, XTAL_24MHz/2)   /* 12MHz */
	MCFG_CPU_PROGRAM_MAP(blmbycar_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", blmbycar_state,  irq1_line_hold)

	MCFG_MACHINE_START_OVERRIDE(blmbycar_state,blmbycar)
	MCFG_MACHINE_RESET_OVERRIDE(blmbycar_state,blmbycar)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(0x180, 0x100)
	MCFG_SCREEN_VISIBLE_AREA(0, 0x180-1, 0, 0x100-1)
	MCFG_SCREEN_UPDATE_DRIVER(blmbycar_state, screen_update)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", blmbycar)

	MCFG_PALETTE_ADD("palette", 0x300)
	MCFG_PALETTE_FORMAT(xxxxBBBBRRRRGGGG)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MCFG_OKIM6295_ADD("oki", XTAL_1MHz, OKIM6295_PIN7_HIGH) // clock frequency & pin 7 not verified
	MCFG_DEVICE_ADDRESS_MAP(AS_0, blmbycar_oki_map)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 1.0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 1.0)
MACHINE_CONFIG_END


MACHINE_START_MEMBER(blmbycar_state,watrball)
{
	save_item(NAME(m_retvalue));

	membank("okibank")->configure_entries(0, 16, memregion("oki")->base(), 0x10000);
}

MACHINE_RESET_MEMBER(blmbycar_state,watrball)
{
	m_retvalue = 0;
}

static MACHINE_CONFIG_DERIVED( watrball, blmbycar )

	/* basic machine hardware */
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(watrball_map)

	MCFG_MACHINE_START_OVERRIDE(blmbycar_state,watrball)
	MCFG_MACHINE_RESET_OVERRIDE(blmbycar_state,watrball)

	/* video hardware */
	MCFG_SCREEN_MODIFY("screen")
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500) /* not accurate */)
	MCFG_SCREEN_VISIBLE_AREA(0, 0x180-1, 16, 0x100-1)
MACHINE_CONFIG_END


/***************************************************************************


                                ROMs Loading


***************************************************************************/

/***************************************************************************

                                Blomby Car
Abm & Gecas, 1990.

CPU : MC68000P12
SND : Oki M6295 (samples only)
OSC : 30.000MHz, 24.000MHz & 1.00MHz resonator
DSW : 2 x 8
GFX : TI TPC1020AFN-084

***************************************************************************/

ROM_START( blmbycar )
	ROM_REGION( 0x100000, "maincpu", 0 )        /* 68000 Code */
	ROM_LOAD16_BYTE( "bcrom4.bin", 0x000000, 0x080000, CRC(06d490ba) SHA1(6d113561b474bf613c6b91c9525a52025ae65ab7) )
	ROM_LOAD16_BYTE( "bcrom6.bin", 0x000001, 0x080000, CRC(33aca664) SHA1(04fff492654d3edac62e9d35808e5946bcc78cbb) )

	ROM_REGION( 0x200000, "sprites", 0 )    /* Sprites */
	ROM_LOAD( "bc_rom7",   0x000000, 0x080000, CRC(e55ca79b) SHA1(4453a6ae0518832f437ab701c28cb2f32920f8ba) )
	ROM_LOAD( "bc_rom8",   0x080000, 0x080000, CRC(cdf38c96) SHA1(3273c29b6a01a7296d06fc653120f8c615195d2c) )
	ROM_LOAD( "bc_rom9",   0x100000, 0x080000, CRC(0337ab3d) SHA1(18c72cd640c7b599390dffaeb670f5832202bf06) )
	ROM_LOAD( "bc_rom10",  0x180000, 0x080000, CRC(5458917e) SHA1(c8dd5a391cc20a573e27a140b185893a8c04859e) )

	ROM_REGION( 0x100000, "oki", 0 )    /* 8 bit adpcm (banked) */
	ROM_LOAD( "bc_rom1",     0x000000, 0x080000, CRC(ac6f8ba1) SHA1(69d2d47cdd331bde5a8973d29659b3f8520452e7) )
	ROM_LOAD( "bc_rom2",     0x080000, 0x080000, CRC(a4bc31bf) SHA1(f3d60141a91449a73f6cec9f4bc5d95ca7911e19) )
ROM_END

ROM_START( blmbycaru )
	ROM_REGION( 0x100000, "maincpu", 0 )        /* 68000 Code */
	ROM_LOAD16_BYTE( "bc_rom4", 0x000000, 0x080000, CRC(76f054a2) SHA1(198efd152b13033e5249119ca48b9e0f6351b0b9) )
	ROM_LOAD16_BYTE( "bc_rom6", 0x000001, 0x080000, CRC(2570b4c5) SHA1(706465950023a6ef7c85ceb9c76246d7556b3859) )

	ROM_REGION( 0x200000, "sprites", 0 )    /* Sprites */
	ROM_LOAD( "bc_rom7",   0x000000, 0x080000, CRC(e55ca79b) SHA1(4453a6ae0518832f437ab701c28cb2f32920f8ba) )
	ROM_LOAD( "bc_rom8",   0x080000, 0x080000, CRC(cdf38c96) SHA1(3273c29b6a01a7296d06fc653120f8c615195d2c) )
	ROM_LOAD( "bc_rom9",   0x100000, 0x080000, CRC(0337ab3d) SHA1(18c72cd640c7b599390dffaeb670f5832202bf06) )
	ROM_LOAD( "bc_rom10",  0x180000, 0x080000, CRC(5458917e) SHA1(c8dd5a391cc20a573e27a140b185893a8c04859e) )

	ROM_REGION( 0x100000, "oki", 0 )    /* 8 bit adpcm (banked) */
	ROM_LOAD( "bc_rom1",     0x000000, 0x080000, CRC(ac6f8ba1) SHA1(69d2d47cdd331bde5a8973d29659b3f8520452e7) )
	ROM_LOAD( "bc_rom2",     0x080000, 0x080000, CRC(a4bc31bf) SHA1(f3d60141a91449a73f6cec9f4bc5d95ca7911e19) )
ROM_END

/*

Waterball by ABM (sticker on the pcb 12-3-96)
The pcb has some empty sockets, maybe it was used for other games since it has no markings.

The game has fonts identical to World Rally and obviously Blomby Car ;)

1x 68k
1x oki 6295
1x OSC 30mhz
1x OSC 24mhz
1x FPGA
1x dispswitch

*/

ROM_START( watrball )
	ROM_REGION( 0x100000, "maincpu", 0 )        /* 68000 Code */
	ROM_LOAD16_BYTE( "rom4.bin", 0x000000, 0x020000, CRC(bfbfa720) SHA1(d6d14c0ba545eb7adee7190da2d3db1c6dd00d75) )
	ROM_LOAD16_BYTE( "rom6.bin", 0x000001, 0x020000, CRC(acff9b01) SHA1(b85671bcc4f03fdf05eb1c9b5d4143e33ec1d7db) )

	ROM_REGION( 0x200000, "sprites", 0 )    /* Sprites */
	ROM_LOAD( "rom7.bin",   0x000000, 0x080000, CRC(e7e5c311) SHA1(5af1a666bf23c5505d120d81fb942f5c49341861) )
	ROM_LOAD( "rom8.bin",   0x080000, 0x080000, CRC(fd27ce6e) SHA1(a472a8cc25818427d2870518649780146e51835b) )
	ROM_LOAD( "rom9.bin",   0x100000, 0x080000, CRC(122cc0ad) SHA1(27cdb19fa082089e47c5cdb44742cfd93aa23a00) )
	ROM_LOAD( "rom10.bin",  0x180000, 0x080000, CRC(22a2a706) SHA1(c7350a94a857e0007d7fc0076b44a3d62693cb6c) )

	ROM_REGION( 0x100000, "oki", 0 )    /* 8 bit adpcm (banked) */
	ROM_LOAD( "rom1.bin",     0x000000, 0x080000, CRC(7f88dee7) SHA1(d493b961fa4631185a33faee7f61786430707209))
//  ROM_LOAD( "rom2.bin",     0x080000, 0x080000, /* not populated for this game */ )
ROM_END


DRIVER_INIT_MEMBER(blmbycar_state,blmbycar)
{
	UINT16 *RAM  = (UINT16 *) memregion("maincpu")->base();
	size_t size = memregion("maincpu")->bytes() / 2;
	int i;

	for (i = 0; i < size; i++)
	{
		UINT16 x = RAM[i];
		x = (x & ~0x0606) | ((x & 0x0202) << 1) | ((x & 0x0404) >> 1);
		RAM[i] = x;
	}
}

/***************************************************************************


                                Game Drivers


***************************************************************************/

GAME( 1994, blmbycar,  0,        blmbycar, blmbycar, blmbycar_state, blmbycar, ROT0, "ABM & Gecas", "Blomby Car", MACHINE_SUPPORTS_SAVE )
GAME( 1994, blmbycaru, blmbycar, blmbycar, blmbycar, driver_device,  0,        ROT0, "ABM & Gecas", "Blomby Car (not encrypted)", MACHINE_SUPPORTS_SAVE )
GAME( 1996, watrball,  0,        watrball, watrball, driver_device,  0,        ROT0, "ABM",         "Water Balls", MACHINE_SUPPORTS_SAVE )
