// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
/***************************************************************************

    Irem M52 hardware

****************************************************************************

    Moon Patrol memory map

    driver by Nicola Salmoria

    0000-3fff ROM
    8000-83ff Video RAM
    8400-87ff Color RAM
    e000-e7ff RAM


    read:
    8800      protection
    d000      IN0
    d001      IN1
    d002      IN2
    d003      DSW1
    d004      DSW2

    write:
    c800-c8ff sprites
    d000      sound command
    d001      flip screen

    I/O ports
    write:
    10-1f     scroll registers
    40        background #1 x position
    60        background #1 y position
    80        background #2 x position
    a0        background #2 y position
    c0        background control

*****************************************************************************

    Locations based on m58.c driver

***************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "audio/irem.h"
#include "includes/iremipt.h"
#include "includes/m52.h"


#define MASTER_CLOCK        XTAL_18_432MHz


/*************************************
 *
 *  Memory maps
 *
 *************************************/

static ADDRESS_MAP_START( main_map, AS_PROGRAM, 8, m52_state )
	AM_RANGE(0x0000, 0x3fff) AM_ROM
	AM_RANGE(0x8000, 0x83ff) AM_RAM_WRITE(m52_videoram_w) AM_SHARE("videoram")
	AM_RANGE(0x8400, 0x87ff) AM_RAM_WRITE(m52_colorram_w) AM_SHARE("colorram")
	AM_RANGE(0x8800, 0x8800) AM_MIRROR(0x07ff) AM_READ(m52_protection_r)
	AM_RANGE(0xc800, 0xcbff) AM_MIRROR(0x0400) AM_WRITEONLY AM_SHARE("spriteram")
	AM_RANGE(0xd000, 0xd000) AM_MIRROR(0x07fc) AM_DEVWRITE("irem_audio", irem_audio_device, cmd_w)
	AM_RANGE(0xd001, 0xd001) AM_MIRROR(0x07fc) AM_WRITE(m52_flipscreen_w)   /* + coin counters */
	AM_RANGE(0xd000, 0xd000) AM_MIRROR(0x07f8) AM_READ_PORT("IN0")
	AM_RANGE(0xd001, 0xd001) AM_MIRROR(0x07f8) AM_READ_PORT("IN1")
	AM_RANGE(0xd002, 0xd002) AM_MIRROR(0x07f8) AM_READ_PORT("IN2")
	AM_RANGE(0xd003, 0xd003) AM_MIRROR(0x07f8) AM_READ_PORT("DSW1")
	AM_RANGE(0xd004, 0xd004) AM_MIRROR(0x07f8) AM_READ_PORT("DSW2")
	AM_RANGE(0xe000, 0xe7ff) AM_RAM
ADDRESS_MAP_END


static ADDRESS_MAP_START( alpha1v_map, AS_PROGRAM, 8, m52_state )
	AM_RANGE(0x0000, 0x6fff) AM_ROM
	AM_RANGE(0x8000, 0x83ff) AM_RAM_WRITE(m52_videoram_w) AM_SHARE("videoram")
	AM_RANGE(0x8400, 0x87ff) AM_RAM_WRITE(m52_colorram_w) AM_SHARE("colorram")
	AM_RANGE(0xc800, 0xc9ff) AM_WRITEONLY AM_SHARE("spriteram") // bigger or mirrored?
	AM_RANGE(0xd000, 0xd000) AM_READ_PORT("IN0") AM_DEVWRITE("irem_audio", irem_audio_device, cmd_w)
	AM_RANGE(0xd001, 0xd001) AM_READ_PORT("IN1") AM_WRITE(alpha1v_flipscreen_w)
	AM_RANGE(0xd002, 0xd002) AM_READ_PORT("IN2")
	AM_RANGE(0xd003, 0xd003) AM_READ_PORT("DSW1")
	AM_RANGE(0xd004, 0xd004) AM_READ_PORT("DSW2")
	AM_RANGE(0xe000, 0xefff) AM_RAM // bigger or mirrored?
ADDRESS_MAP_END


static ADDRESS_MAP_START( main_portmap, AS_IO, 8, m52_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x00) AM_MIRROR(0x1f) AM_WRITE(m52_scroll_w)
	AM_RANGE(0x40, 0x40) AM_MIRROR(0x1f) AM_WRITE(m52_bg1xpos_w)
	AM_RANGE(0x60, 0x60) AM_MIRROR(0x1f) AM_WRITE(m52_bg1ypos_w)
	AM_RANGE(0x80, 0x80) AM_MIRROR(0x1f) AM_WRITE(m52_bg2xpos_w)
	AM_RANGE(0xa0, 0xa0) AM_MIRROR(0x1f) AM_WRITE(m52_bg2ypos_w)
	AM_RANGE(0xc0, 0xc0) AM_MIRROR(0x1f) AM_WRITE(m52_bgcontrol_w)
ADDRESS_MAP_END



/*************************************
 *
 *  Port definitions
 *
 *************************************/

/* Same as m57, m58 and m62 (IREM Z80 hardware) */
static INPUT_PORTS_START( m52 )
	PORT_START("IN0")
	/* Start 1 & 2 also restarts and freezes the game with stop mode on
	   and are used in test mode to enter and esc the various tests */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START2 )
	/* coin input must be active for 19 frames to be consistently recognized */
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE1 ) PORT_IMPULSE(19)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )  PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )  PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )    PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON1 )

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )  PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )  PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )    PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL

	/* DSW1 is so different from game to game that it isn't included here */

	PORT_START("DSW2")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Flip_Screen ) ) PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Cabinet ) ) PORT_DIPLOCATION("SW2:2")
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x04, 0x04, "Coin Mode" ) PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(    0x04, "Mode 1" )
	PORT_DIPSETTING(    0x00, "Mode 2" )
	PORT_DIPUNKNOWN_DIPLOC( 0x08, IP_ACTIVE_LOW, "SW2:4" )
	PORT_DIPUNKNOWN_DIPLOC( 0x10, IP_ACTIVE_LOW, "SW2:5" )
	PORT_DIPUNKNOWN_DIPLOC( 0x20, IP_ACTIVE_LOW, "SW2:6" )
	PORT_DIPNAME( 0x40, 0x40, "Invulnerability (Cheat)") PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_SERVICE_DIPLOC( 0x80, IP_ACTIVE_LOW, "SW2:8" )
INPUT_PORTS_END


static INPUT_PORTS_START( mpatrol )
	PORT_INCLUDE(m52)

	PORT_MODIFY("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_2WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )  PORT_2WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )             /* IPT_JOYSTICK_DOWN */
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )             /* IPT_JOYSTICK_UP */

	PORT_MODIFY("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_2WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )  PORT_2WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )             /* IPT_JOYSTICK_DOWN PORT_COCKTAIL */
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )             /* IPT_JOYSTICK_UP   PORT_COCKTAIL */

	PORT_MODIFY("DSW2")
	PORT_DIPUNUSED_DIPLOC( 0x08, IP_ACTIVE_LOW, "SW2:4" )   /* should have been "slow motion" but no conditional jump at 0x00c3 */
	/* In stop mode, press 2 to stop and 1 to restart */
	PORT_DIPNAME( 0x10, 0x10, "Stop Mode (Cheat)") PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "Sector Selection (Cheat)") PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x03, 0x02, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW1:1,2")
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x01, "2" )
	PORT_DIPSETTING(    0x02, "3" )
	PORT_DIPSETTING(    0x03, "5" )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Bonus_Life ) ) PORT_DIPLOCATION("SW1:3,4")
	PORT_DIPSETTING(    0x0c, "10000 30000 50000" )
	PORT_DIPSETTING(    0x08, "20000 40000 60000" )
	PORT_DIPSETTING(    0x04, "10000" )
	PORT_DIPSETTING(    0x00, DEF_STR( None ) )
	IREM_Z80_COINAGE_TYPE_1_LOC(SW1)
INPUT_PORTS_END


static INPUT_PORTS_START( mpatrolw )
	PORT_INCLUDE(mpatrol)

	PORT_MODIFY("DSW1")
	PORT_DIPNAME( 0x03, 0x02, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW1:1,2")
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPSETTING(    0x01, "3" )
	PORT_DIPSETTING(    0x02, "4" )
	PORT_DIPSETTING(    0x03, "5" )
INPUT_PORTS_END


static INPUT_PORTS_START( alpha1v )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )  PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )  PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )    PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON1 )

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )  PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )  PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )    PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL

	PORT_START("DSW1")
	PORT_DIPNAME( 0x03, 0x02, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x03, "2" )
	PORT_DIPSETTING(    0x02, "3" )
	PORT_DIPSETTING(    0x01, "4" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ) )
	PORT_DIPUNUSED( 0x08, IP_ACTIVE_LOW )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Allow_Continue ) )
	PORT_DIPSETTING(    0x10, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPUNUSED( 0x20, IP_ACTIVE_LOW )
	PORT_DIPUNUSED( 0x40, IP_ACTIVE_LOW )
	PORT_DIPUNUSED( 0x80, IP_ACTIVE_LOW )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x0f, 0x00, DEF_STR( Coin_A ) )           /* table at 0x5ef4 - 16 bytes (coins) + 16 bytes (credits) */
	PORT_DIPSETTING(    0x0b, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 3C_1C ) )
//  PORT_DIPSETTING(    0x08, DEF_STR( 2C_1C ) )
//  PORT_DIPSETTING(    0x09, DEF_STR( 2C_1C ) )
//  PORT_DIPSETTING(    0x0a, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x0d, DEF_STR( 2C_1C ) )
//  PORT_DIPSETTING(    0x0e, DEF_STR( 1C_1C ) )
//  PORT_DIPSETTING(    0x0f, DEF_STR( 1C_1C ) )
//  PORT_DIPSETTING(    0x07, DEF_STR( 1C_1C ) )
//  PORT_DIPSETTING(    0x06, DEF_STR( 1C_1C ) )
//  PORT_DIPSETTING(    0x05, DEF_STR( 1C_1C ) )
//  PORT_DIPSETTING(    0x04, DEF_STR( 1C_1C ) )
//  PORT_DIPSETTING(    0x03, DEF_STR( 1C_1C ) )
//  PORT_DIPSETTING(    0x02, DEF_STR( 1C_1C ) )
//  PORT_DIPSETTING(    0x01, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPNAME( 0xf0, 0x00, DEF_STR( Coin_B ) )           /* table at 0x5f14 - 16 bytes (coins) + 16 bytes (credits) */
//  PORT_DIPSETTING(    0xf0, "1 Coin/0 Credit" )
//  PORT_DIPSETTING(    0x70, DEF_STR( 1C_1C ) )
//  PORT_DIPSETTING(    0x60, DEF_STR( 1C_1C ) )
//  PORT_DIPSETTING(    0x50, DEF_STR( 1C_1C ) )
//  PORT_DIPSETTING(    0x40, DEF_STR( 1C_1C ) )
//  PORT_DIPSETTING(    0x30, DEF_STR( 1C_1C ) )
//  PORT_DIPSETTING(    0x20, DEF_STR( 1C_1C ) )
//  PORT_DIPSETTING(    0x10, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
//  PORT_DIPSETTING(    0xe0, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0xd0, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0xb0, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0xa0, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x90, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 1C_7C ) )
INPUT_PORTS_END



/*************************************
 *
 *  Graphics layouts
 *
 *************************************/

static const gfx_layout charlayout =
{
	8,8,
	RGN_FRAC(1,2),
	2,
	{ RGN_FRAC(0,2), RGN_FRAC(1,2) },
	{ STEP8(0,1) },
	{ STEP8(0,8) },
	8*8
};

static const gfx_layout spritelayout =
{
	16,16,
	RGN_FRAC(1,2),
	2,
	{ RGN_FRAC(0,2), RGN_FRAC(1,2) },
	{ STEP8(0,1), STEP8(16*8,1) },
	{ STEP16(0,8) },
	32*8
};

static const UINT32 bgcharlayout_xoffset[256] =
{
	STEP4(0x000,1), STEP4(0x008,1), STEP4(0x010,1), STEP4(0x018,1),
	STEP4(0x020,1), STEP4(0x028,1), STEP4(0x030,1), STEP4(0x038,1),
	STEP4(0x040,1), STEP4(0x048,1), STEP4(0x050,1), STEP4(0x058,1),
	STEP4(0x060,1), STEP4(0x068,1), STEP4(0x070,1), STEP4(0x078,1),
	STEP4(0x080,1), STEP4(0x088,1), STEP4(0x090,1), STEP4(0x098,1),
	STEP4(0x0a0,1), STEP4(0x0a8,1), STEP4(0x0b0,1), STEP4(0x0b8,1),
	STEP4(0x0c0,1), STEP4(0x0c8,1), STEP4(0x0d0,1), STEP4(0x0d8,1),
	STEP4(0x0e0,1), STEP4(0x0e8,1), STEP4(0x0f0,1), STEP4(0x0f8,1),
	STEP4(0x100,1), STEP4(0x108,1), STEP4(0x110,1), STEP4(0x118,1),
	STEP4(0x120,1), STEP4(0x128,1), STEP4(0x130,1), STEP4(0x138,1),
	STEP4(0x140,1), STEP4(0x148,1), STEP4(0x150,1), STEP4(0x158,1),
	STEP4(0x160,1), STEP4(0x168,1), STEP4(0x170,1), STEP4(0x178,1),
	STEP4(0x180,1), STEP4(0x188,1), STEP4(0x190,1), STEP4(0x198,1),
	STEP4(0x1a0,1), STEP4(0x1a8,1), STEP4(0x1b0,1), STEP4(0x1b8,1),
	STEP4(0x1c0,1), STEP4(0x1c8,1), STEP4(0x1d0,1), STEP4(0x1d8,1),
	STEP4(0x1e0,1), STEP4(0x1e8,1), STEP4(0x1f0,1), STEP4(0x1f8,1)
};

static const UINT32 bgcharlayout_yoffset[64] =
{
	STEP32(0x0000,0x200), STEP32(0x4000,0x200)
};

static const gfx_layout bgcharlayout =
{
	256, 64, /* 256x64 image format */
	1,       /* 1 image */
	2,       /* 2 bits per pixel */
	{ 4, 0 },       /* the two bitplanes for 4 pixels are packed into one byte */
	EXTENDED_XOFFS,
	EXTENDED_YOFFS,
	0x8000,
	bgcharlayout_xoffset,
	bgcharlayout_yoffset
};


static GFXDECODE_START( m52 )
	GFXDECODE_ENTRY( "gfx1", 0x0000, charlayout,                0, 128 )
	GFXDECODE_ENTRY( "gfx2", 0x0000, spritelayout,          128*4,  16 )
	GFXDECODE_ENTRY( "gfx3", 0x0000, bgcharlayout, 128*4+16*4+0*4,   1 )
	GFXDECODE_ENTRY( "gfx4", 0x0000, bgcharlayout, 128*4+16*4+1*4,   1 )
	GFXDECODE_ENTRY( "gfx5", 0x0000, bgcharlayout, 128*4+16*4+2*4,   1 )
GFXDECODE_END



/*************************************
 *
 *  Machine drivers
 *
 *************************************/

void m52_state::machine_reset()
{
	m_bg1xpos = 0;
	m_bg1ypos = 0;
	m_bg2xpos = 0;
	m_bg2ypos = 0;
	m_bgcontrol = 0;
}

static MACHINE_CONFIG_START( m52, m52_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, MASTER_CLOCK/6)
	MCFG_CPU_PROGRAM_MAP(main_map)
	MCFG_CPU_IO_MAP(main_portmap)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", m52_state,  irq0_line_hold)

	/* video hardware */
	MCFG_GFXDECODE_ADD("gfxdecode", "palette", m52)
	MCFG_PALETTE_ADD("palette", 128*4+16*4+3*4)
	MCFG_PALETTE_INDIRECT_ENTRIES(512+32+32)
	MCFG_PALETTE_INIT_OWNER(m52_state, m52)

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_RAW_PARAMS(MASTER_CLOCK/3, 384, 136, 376, 282, 22, 274)
	MCFG_SCREEN_UPDATE_DRIVER(m52_state, screen_update_m52)
	MCFG_SCREEN_PALETTE("palette")

	/* sound hardware */
	//MCFG_FRAGMENT_ADD(m52_sound_c_audio)
	MCFG_DEVICE_ADD("irem_audio", IREM_M52_SOUNDC_AUDIO, 0)

MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( alpha1v, m52 )

	/* basic machine hardware */
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(alpha1v_map)
MACHINE_CONFIG_END



/*************************************
 *
 *  ROM definitions
 *
 *************************************/

ROM_START( mpatrol )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "mpa-1.3m",      0x0000, 0x1000, CRC(5873a860) SHA1(8c03726d6e049c3edbc277440184e31679f78258) )
	ROM_LOAD( "mpa-2.3l",      0x1000, 0x1000, CRC(f4b85974) SHA1(dfb2efb57378a20af6f20569f4360cde95596f93) )
	ROM_LOAD( "mpa-3.3k",      0x2000, 0x1000, CRC(2e1a598c) SHA1(112c3c9678db8a8540a8df3708020c87fd10c91b) )
	ROM_LOAD( "mpa-4.3j",      0x3000, 0x1000, CRC(dd05b587) SHA1(727961b0dafa4a96b580d51013336db2a18aff1e) )

	ROM_REGION( 0x8000, "irem_audio:iremsound", 0 )
	ROM_LOAD( "mp-s1.1a",     0x7000, 0x1000, CRC(561d3108) SHA1(4998c68a9e9a8002251fa8f07aa1082444a9dc80) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "mpe-5.3e",     0x0000, 0x1000, CRC(e3ee7f75) SHA1(b03d0d56150d3e9da4a4c871338097b4f450b649) )       /* chars */
	ROM_LOAD( "mpe-4.3f",     0x1000, 0x1000, CRC(cca6d023) SHA1(fecb3059fb09897a096add9452b50aec55c07545) )

	ROM_REGION( 0x2000, "gfx2", 0 )
	ROM_LOAD( "mpb-2.3m",     0x0000, 0x1000, CRC(707ace5e) SHA1(93c682e13e74bce29ced3a87bffb29569c114c3b) )       /* sprites */
	ROM_LOAD( "mpb-1.3n",     0x1000, 0x1000, CRC(9b72133a) SHA1(1393ef92ae1ad58a4b62ca1660c0793d30a8b5e2) )

	ROM_REGION( 0x1000, "gfx3", 0 )
	ROM_LOAD( "mpe-1.3l",     0x0000, 0x1000, CRC(c46a7f72) SHA1(8bb7c9acaf6833fb6c0575b015991b873a305a84) )       /* background graphics */

	ROM_REGION( 0x1000, "gfx4", 0 )
	ROM_LOAD( "mpe-2.3k",     0x0000, 0x1000, CRC(c7aa1fb0) SHA1(14c6c76e1d0db2c0745e5d6d33ea6945fac8e9ee) )

	ROM_REGION( 0x1000, "gfx5", 0 )
	ROM_LOAD( "mpe-3.3h",     0x0000, 0x1000, CRC(a0919392) SHA1(8a090cb8d483a3d67c7360058e3fdd70e151cd62) )

	ROM_REGION( 0x0340, "proms", 0 )
	ROM_LOAD( "mpc-4.2a",     0x0000, 0x0200, CRC(07f99284) SHA1(dfc52958f2520e1ce4446dd4c84c91413bbacf76) )
	ROM_LOAD( "mpc-3.1m",     0x0200, 0x0020, CRC(6a57eff2) SHA1(2d1c12dab5915da2ccd466e39436c88be434d634) ) /* background palette */
	ROM_LOAD( "mpc-1.1f",     0x0220, 0x0020, CRC(26979b13) SHA1(8c41a8cce4f3384c392a9f7a223a50d7be0e14a5) ) /* sprite palette */
	ROM_LOAD( "mpc-2.2h",     0x0240, 0x0100, CRC(7ae4cd97) SHA1(bc0662fac82ffe65f02092d912b2c2b0c7a8ac2b) ) /* sprite lookup table */
ROM_END

ROM_START( mpatrolw )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "mpa-1w.3m",    0x0000, 0x1000, CRC(baa1a1d4) SHA1(7968a7f221e7f4c9c81ddc8de17f6568e17b9ea8) )
	ROM_LOAD( "mpa-2w.3l",    0x1000, 0x1000, CRC(52459e51) SHA1(ae685b7848baa1b87a3f2bce97356286171e16d4) )
	ROM_LOAD( "mpa-3w.3k",    0x2000, 0x1000, CRC(9b249fe5) SHA1(c01e0d572c4c163f3cf4b2aa9f4246427811b78d) )
	ROM_LOAD( "mpa-4w.3j",    0x3000, 0x1000, CRC(fee76972) SHA1(c3166b027f89f61964ead804d3c2da387454c4c2) )

	ROM_REGION( 0x8000, "irem_audio:iremsound", 0 )
	ROM_LOAD( "mp-s1.1a",     0x7000, 0x1000, CRC(561d3108) SHA1(4998c68a9e9a8002251fa8f07aa1082444a9dc80) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "mpe-5w.3e",    0x0000, 0x1000, CRC(f56e01fe) SHA1(93f582d63b9cd5c6dca207aa57b213c939cdda1d) )       /* chars */
	ROM_LOAD( "mpe-4w.3f",    0x1000, 0x1000, CRC(caaba2d9) SHA1(7016a26c2d01e3209749598e993cd8ce91f12c88) )

	ROM_REGION( 0x2000, "gfx2", 0 )
	ROM_LOAD( "mpb-2.3m",     0x0000, 0x1000, CRC(707ace5e) SHA1(93c682e13e74bce29ced3a87bffb29569c114c3b) )       /* sprites */
	ROM_LOAD( "mpb-1.3n",     0x1000, 0x1000, CRC(9b72133a) SHA1(1393ef92ae1ad58a4b62ca1660c0793d30a8b5e2) )

	ROM_REGION( 0x1000, "gfx3", 0 )
	ROM_LOAD( "mpe-1.3l",     0x0000, 0x1000, CRC(c46a7f72) SHA1(8bb7c9acaf6833fb6c0575b015991b873a305a84) )       /* background graphics */

	ROM_REGION( 0x1000, "gfx4", 0 )
	ROM_LOAD( "mpe-2.3k",     0x0000, 0x1000, CRC(c7aa1fb0) SHA1(14c6c76e1d0db2c0745e5d6d33ea6945fac8e9ee) )

	ROM_REGION( 0x1000, "gfx5", 0 )
	ROM_LOAD( "mpe-3.3h",     0x0000, 0x1000, CRC(a0919392) SHA1(8a090cb8d483a3d67c7360058e3fdd70e151cd62) )

	ROM_REGION( 0x0340, "proms", 0 )
	ROM_LOAD( "mpc-4a.2a",    0x0000, 0x0200, CRC(cb0a5ff3) SHA1(d3f88b4e0c4858abac8b52105656ecece0cf4df9) )
	ROM_LOAD( "mpc-3.1m",     0x0200, 0x0020, CRC(6a57eff2) SHA1(2d1c12dab5915da2ccd466e39436c88be434d634) ) /* background palette */
	ROM_LOAD( "mpc-1.1f",     0x0220, 0x0020, CRC(26979b13) SHA1(8c41a8cce4f3384c392a9f7a223a50d7be0e14a5) ) /* sprite palette */
	ROM_LOAD( "mpc-2.2h",     0x0240, 0x0100, CRC(7ae4cd97) SHA1(bc0662fac82ffe65f02092d912b2c2b0c7a8ac2b) ) /* sprite lookup table */
ROM_END

ROM_START( mranger )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "mpa-1.3m",      0x0000, 0x1000, CRC(5873a860) SHA1(8c03726d6e049c3edbc277440184e31679f78258) )
	ROM_LOAD( "mra-2.3l",      0x1000, 0x1000, CRC(217dd431) SHA1(7b81f854209afc1fd3df11b7375f36de6bc4a7c3) )
	ROM_LOAD( "mra-3.3k",      0x2000, 0x1000, CRC(9f0af7b2) SHA1(3daaec15b0d3bc30723ebb14b50f66f288f0d096) )
	ROM_LOAD( "mra-4.3j",      0x3000, 0x1000, CRC(7fe8e2cd) SHA1(4ffad9c7a9360999b213b790c6c76cc79c8e49d5) )

	ROM_REGION( 0x8000, "irem_audio:iremsound", 0 )
	ROM_LOAD( "mp-s1.1a",     0x7000, 0x1000, CRC(561d3108) SHA1(4998c68a9e9a8002251fa8f07aa1082444a9dc80) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "mpe-5.3e",     0x0000, 0x1000, CRC(e3ee7f75) SHA1(b03d0d56150d3e9da4a4c871338097b4f450b649) )       /* chars */
	ROM_LOAD( "mpe-4.3f",     0x1000, 0x1000, CRC(cca6d023) SHA1(fecb3059fb09897a096add9452b50aec55c07545) )

	ROM_REGION( 0x2000, "gfx2", 0 )
	ROM_LOAD( "mpb-2.3m",     0x0000, 0x1000, CRC(707ace5e) SHA1(93c682e13e74bce29ced3a87bffb29569c114c3b) )       /* sprites */
	ROM_LOAD( "mpb-1.3n",     0x1000, 0x1000, CRC(9b72133a) SHA1(1393ef92ae1ad58a4b62ca1660c0793d30a8b5e2) )

	ROM_REGION( 0x1000, "gfx3", 0 )
	ROM_LOAD( "mpe-1.3l",     0x0000, 0x1000, CRC(c46a7f72) SHA1(8bb7c9acaf6833fb6c0575b015991b873a305a84) )       /* background graphics */

	ROM_REGION( 0x1000, "gfx4", 0 )
	ROM_LOAD( "mpe-2.3k",     0x0000, 0x1000, CRC(c7aa1fb0) SHA1(14c6c76e1d0db2c0745e5d6d33ea6945fac8e9ee) )

	ROM_REGION( 0x1000, "gfx5", 0 )
	ROM_LOAD( "mpe-3.3h",     0x0000, 0x1000, CRC(a0919392) SHA1(8a090cb8d483a3d67c7360058e3fdd70e151cd62) )

	ROM_REGION( 0x0340, "proms", 0 )
	ROM_LOAD( "mpc-4.2a",     0x0000, 0x0200, CRC(07f99284) SHA1(dfc52958f2520e1ce4446dd4c84c91413bbacf76) )
	ROM_LOAD( "mpc-3.1m",     0x0200, 0x0020, CRC(6a57eff2) SHA1(2d1c12dab5915da2ccd466e39436c88be434d634) ) /* background palette */
	ROM_LOAD( "mpc-1.1f",     0x0220, 0x0020, CRC(26979b13) SHA1(8c41a8cce4f3384c392a9f7a223a50d7be0e14a5) ) /* sprite palette */
	ROM_LOAD( "mpc-2.2h",     0x0240, 0x0100, CRC(7ae4cd97) SHA1(bc0662fac82ffe65f02092d912b2c2b0c7a8ac2b) ) /* sprite lookup table */
ROM_END


ROM_START( alpha1v )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "2-m3",      0x0000, 0x1000, CRC(3a679d34) SHA1(1a54a43070c56dc91d4d258f29e29613bb309f1c) )
	ROM_LOAD( "3-l3",      0x1000, 0x1000, CRC(2f09df64) SHA1(e91602e9e41ad24dd1d7f384ed81b9bdaadd03e1) )
	ROM_LOAD( "4-k3",      0x2000, 0x1000, CRC(64fb9c8a) SHA1(735fd00cc42193a417e6cde75f12b4cf2e804942) )
	ROM_LOAD( "5-j3",      0x3000, 0x1000, CRC(d1643d18) SHA1(7c794b82e17e2ba0a6237e3fc20d8314f6c2481c) )
	ROM_LOAD( "6-h3",      0x4000, 0x1000, CRC(cf34ab51) SHA1(3696da71e2bc7edd1ee7aeaac87be5386608c09e) )
	ROM_LOAD( "7-f3",      0x5000, 0x1000, CRC(99db9781) SHA1(a56a675cc4cbc9681bfe8052f51f19336eb2a0a6) )
	ROM_LOAD( "7a e3",     0x6000, 0x1000, CRC(3b0b4b0d) SHA1(0d8eea1e2db269943611289b3490a578ee347f85) )

	ROM_REGION( 0x8000, "irem_audio:iremsound", 0 )
	ROM_LOAD( "1-a1",      0x7000, 0x1000, CRC(9e07fdd5) SHA1(ed4f462fcfe91fa8e88bfeaaba0a0c11fa0b4601) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "14-e3",     0x0000, 0x1000, CRC(cf00c737) SHA1(415e90289039cac4d04cb1d559f1378ca6a32132) )       /* chars */
	ROM_LOAD( "13-f3",     0x1000, 0x1000, CRC(4b799229) SHA1(42cbdcf787b08b041d30504d699a12c378224933) )

	ROM_REGION( 0x3000, "gfx2", 0 ) // 3bpp? (mpatrol is 2bpp..)
	ROM_LOAD( "15-n3",     0x0000, 0x1000, CRC(dc26df76) SHA1(dd1cff7935f5559f9d1b440e02d5e5aa521b0054) )
	ROM_LOAD( "16-l3",     0x1000, 0x1000, CRC(39b9863b) SHA1(da9da9a1066188f050c422dfed1bbbd3ba612ccc) )
	ROM_LOAD( "17-k3",     0x2000, 0x1000, CRC(cfd90773) SHA1(052e126888b6de636db9c521a090699c282b620b) )

	/* all the background roms just contain stars.. */
	ROM_REGION( 0x1000, "gfx3", 0 )
	ROM_LOAD( "11-k3",     0x0000, 0x1000, CRC(7659440a) SHA1(2efd27c82913513dd03e799f1ed3c10b0863677d) ) // these two match..
	ROM_LOAD( "12-jh3",    0x0000, 0x1000, CRC(7659440a) SHA1(2efd27c82913513dd03e799f1ed3c10b0863677d) )

	ROM_REGION( 0x1000, "gfx4", 0 )
	ROM_LOAD( "9-n3",     0x0000, 0x1000, CRC(0fdb7d13) SHA1(e828254a4f94df633d338b5772719276d41c6b7f) )

	ROM_REGION( 0x1000, "gfx5", 0 )
	ROM_LOAD( "10-lm3",     0x0000, 0x1000, CRC(9dde3a75) SHA1(293d093485be19bfb20685d76a08ac78e24062bf) )

	ROM_REGION( 0x0340, "proms", 0 )
	ROM_LOAD( "63s481-a2",     0x0000, 0x0200, CRC(58678ea8) SHA1(b13a78a5bca8ad5bdec1293512b53654768a7a7a) )
	ROM_LOAD( "18s030-m1",     0x0200, 0x0020, CRC(6a57eff2) SHA1(2d1c12dab5915da2ccd466e39436c88be434d634) ) /* background palette */
	ROM_LOAD( "mb7051-f1",     0x0220, 0x0020, CRC(d8bdd0df) SHA1(ca522428927911808214d319af314f601497ded4) ) /* sprite palette */
	ROM_LOAD( "mb7052-h2",     0x0240, 0x0100, CRC(ce9f0ef9) SHA1(3afb94ed033f272983bbed22a59856df7824ef8a) ) /* sprite lookup table */
ROM_END



/*************************************
 *
 *  Game drivers
 *
 *************************************/

GAME( 1982, mpatrol,  0,        m52,      mpatrol,  driver_device,  0, ROT0, "Irem", "Moon Patrol", MACHINE_SUPPORTS_SAVE )
GAME( 1982, mpatrolw, mpatrol,  m52,      mpatrolw, driver_device,  0, ROT0, "Irem (Williams license)", "Moon Patrol (Williams)", MACHINE_SUPPORTS_SAVE ) // USA
GAME( 1982, mranger,  mpatrol,  m52,      mpatrol,  driver_device,  0, ROT0, "bootleg", "Moon Ranger (bootleg of Moon Patrol)", MACHINE_SUPPORTS_SAVE ) // Italy
GAME( 1988, alpha1v,  0,        alpha1v,  alpha1v,  driver_device,  0, ROT0, "Vision Electronics", "Alpha One (Vision Electronics)", MACHINE_NOT_WORKING| MACHINE_NO_SOUND| MACHINE_SUPPORTS_SAVE )
