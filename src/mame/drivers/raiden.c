// license:???
// copyright-holders:Oliver Bergmann, Bryan McPhail, Randy Mongenel
/***************************************************************************

    Seibu Raiden hardware

    Raiden                          (c) 1990 Seibu Kaihatsu
    Raiden (Alternate Hardware)     (c) 1990 Seibu Kaihatsu
    Raiden (Korean license)         (c) 1990 Seibu Kaihatsu
    Raiden (Taiwanese license)      (c) 1990 Seibu Kaihatsu

    driver by Oliver Bergmann, Bryan McPhail, Randy Mongenel

    The alternate hardware version is probably newer than the main sets.
    It looks closer to the newer 68000 games, while the main set looks
    closer to Dynamite Duke (1989 game) in terms of graphics registers used, etc.

    As well as different graphics registers the alternate set has a
    different memory map, and different fix char layer memory layout!

    To access test mode, reset with both start buttons held.

    The country byte is stored at 0xffffd in the main cpu region,
    (that's 0x1fffe in program rom 4).

        0x80  = World/Japan version? (Seibu Kaihatsu) (distributed by Tecmo?)
        0x81  = USA version (Fabtek license)
        0x82  = Taiwan version (Liang HWA Electronics license)
        0x83  = Hong Kong version (Wah Yan Electronics license)
        0x84  = Korean version (IBL Corporation license)

        There are also strings for Spanish, Greece, Mexico, Middle &
        South America though it's not clear if they are used.

    Common set is main PCB and an OBJ1 daughterboard.
    XTALs: 20MHz, 14.31818MHz, 12MHz
    CPUs:  2 x Sony CXQ70116P-10 (NEC V30 @ 10MHz), Z80A
    Sound: YM3812, OKI M6295
    Custom ICs:
    - SEI0160 QFP60 (2 on main PCB, 3 on OBJ1 PCB)
    - S1S6091 QFP80 (4 on main PCB, 4 on OBJ1 PCB)
    - Altera EP910PC-40 EPLD, one next to each V30
    - SEI0050BU DIP40
    - SEI80BU DIP42
    - SEI0100BU "YM3931"
    - many CMOS Gate Arrays

    One of the sets is SEI8904 with SEI9008 subboard.

***************************************************************************/

#include "emu.h"
#include "cpu/nec/nec.h"
#include "cpu/z80/z80.h"
#include "sound/3812intf.h"
#include "sound/okim6295.h"
#include "includes/raiden.h"
#include "video/seibu_crtc.h"


/******************************************************************************/

static ADDRESS_MAP_START( main_map, AS_PROGRAM, 16, raiden_state )
	AM_RANGE(0x00000, 0x06fff) AM_RAM
	AM_RANGE(0x07000, 0x07fff) AM_RAM AM_SHARE("spriteram")
	AM_RANGE(0x08000, 0x08fff) AM_RAM AM_SHARE("shared_ram")
	AM_RANGE(0x0a000, 0x0a00d) AM_DEVREADWRITE("seibu_sound", seibu_sound_device, main_word_r, main_word_w)
	AM_RANGE(0x0c000, 0x0c7ff) AM_WRITE(raiden_text_w) AM_SHARE("videoram")
	AM_RANGE(0x0e000, 0x0e001) AM_READ_PORT("P1_P2")
	AM_RANGE(0x0e002, 0x0e003) AM_READ_PORT("DSW")
	AM_RANGE(0x0e004, 0x0e005) AM_WRITENOP // watchdog?
	AM_RANGE(0x0e006, 0x0e007) AM_WRITE8(raiden_control_w, 0x00ff)
	AM_RANGE(0x0f000, 0x0f03f) AM_WRITEONLY AM_SHARE("scroll_ram")
	AM_RANGE(0xa0000, 0xfffff) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START( sub_map, AS_PROGRAM, 16, raiden_state )
	AM_RANGE(0x00000, 0x01fff) AM_RAM
	AM_RANGE(0x02000, 0x027ff) AM_RAM_WRITE(raiden_background_w) AM_SHARE("back_data")
	AM_RANGE(0x02800, 0x02fff) AM_RAM_WRITE(raiden_foreground_w) AM_SHARE("fore_data")
	AM_RANGE(0x03000, 0x03fff) AM_RAM_DEVWRITE("palette", palette_device, write) AM_SHARE("palette")
	AM_RANGE(0x04000, 0x04fff) AM_RAM AM_SHARE("shared_ram")
	AM_RANGE(0x07ffe, 0x07fff) AM_WRITENOP // ?
	AM_RANGE(0x08000, 0x08001) AM_WRITENOP // watchdog?
	AM_RANGE(0x0a000, 0x0a001) AM_WRITENOP // ?
	AM_RANGE(0xc0000, 0xfffff) AM_ROM
ADDRESS_MAP_END


/******************************************************************************/

static ADDRESS_MAP_START( raidenu_main_map, AS_PROGRAM, 16, raiden_state )
	AM_RANGE(0x00000, 0x06fff) AM_RAM
	AM_RANGE(0x07000, 0x07fff) AM_RAM AM_SHARE("spriteram")
	AM_RANGE(0x08000, 0x0803f) AM_WRITEONLY AM_SHARE("scroll_ram")
	AM_RANGE(0x0a000, 0x0afff) AM_RAM AM_SHARE("shared_ram")
	AM_RANGE(0x0b000, 0x0b001) AM_READ_PORT("P1_P2")
	AM_RANGE(0x0b002, 0x0b003) AM_READ_PORT("DSW")
	AM_RANGE(0x0b004, 0x0b005) AM_WRITENOP // watchdog?
	AM_RANGE(0x0b006, 0x0b007) AM_WRITE8(raiden_control_w, 0x00ff)
	AM_RANGE(0x0c000, 0x0c7ff) AM_WRITE(raiden_text_w) AM_SHARE("videoram")
	AM_RANGE(0x0d000, 0x0d00d) AM_DEVREADWRITE("seibu_sound", seibu_sound_device, main_word_r, main_word_w)
	AM_RANGE(0xa0000, 0xfffff) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START( raidenu_sub_map, AS_PROGRAM, 16, raiden_state )
	AM_RANGE(0x00000, 0x05fff) AM_RAM
	AM_RANGE(0x06000, 0x067ff) AM_RAM_WRITE(raiden_background_w) AM_SHARE("back_data")
	AM_RANGE(0x06800, 0x06fff) AM_RAM_WRITE(raiden_foreground_w) AM_SHARE("fore_data")
	AM_RANGE(0x07000, 0x07fff) AM_RAM_DEVWRITE("palette", palette_device, write) AM_SHARE("palette")
	AM_RANGE(0x08000, 0x08fff) AM_RAM AM_SHARE("shared_ram")
	AM_RANGE(0x0a000, 0x0a001) AM_WRITENOP // ?
	AM_RANGE(0x0c000, 0x0c001) AM_WRITENOP // watchdog?
	AM_RANGE(0xc0000, 0xfffff) AM_ROM
ADDRESS_MAP_END


/******************************************************************************/

static ADDRESS_MAP_START( raidenb_main_map, AS_PROGRAM, 16, raiden_state )
	AM_RANGE(0x00000, 0x06fff) AM_RAM
	AM_RANGE(0x07000, 0x07fff) AM_RAM AM_SHARE("spriteram")
	AM_RANGE(0x0a000, 0x0afff) AM_RAM AM_SHARE("shared_ram")
	AM_RANGE(0x0b000, 0x0b001) AM_READ_PORT("P1_P2")
	AM_RANGE(0x0b002, 0x0b003) AM_READ_PORT("DSW")
	AM_RANGE(0x0b004, 0x0b005) AM_WRITENOP // watchdog?
	AM_RANGE(0x0b006, 0x0b007) AM_WRITE8(raidenb_control_w, 0x00ff)
	AM_RANGE(0x0c000, 0x0c7ff) AM_WRITE(raiden_text_w) AM_SHARE("videoram")
	AM_RANGE(0x0d000, 0x0d00d) AM_DEVREADWRITE("seibu_sound", seibu_sound_device, main_word_r, main_word_w)
	AM_RANGE(0x0d040, 0x0d08f) AM_DEVREADWRITE("crtc", seibu_crtc_device, read, write)
	AM_RANGE(0xa0000, 0xfffff) AM_ROM
ADDRESS_MAP_END


/*****************************************************************************/

static INPUT_PORTS_START( raiden )
	SEIBU_COIN_INPUTS /* coin inputs read through sound cpu */

	PORT_START("P1_P2")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(2)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_START2 )

	PORT_START("DSW")
	PORT_DIPNAME( 0x0001, 0x0001, "Coin Mode" ) PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(      0x0001, "A" )
	PORT_DIPSETTING(      0x0000, "B" )
	/* Coin Mode A */
	PORT_DIPNAME( 0x001e, 0x001e, DEF_STR( Coinage ) ) PORT_CONDITION("DSW", 0x0001, EQUALS, 0x0001) PORT_DIPLOCATION("SW1:2,3,4,5")
	PORT_DIPSETTING(      0x0014, DEF_STR( 6C_1C ) )
	PORT_DIPSETTING(      0x0016, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(      0x0018, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x001a, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( 8C_3C ) )
	PORT_DIPSETTING(      0x001c, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( 5C_3C ) )
	PORT_DIPSETTING(      0x0006, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(      0x001e, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(      0x0012, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x000e, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x000c, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(      0x000a, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Free_Play ) )
	/* Coin Mode B */
	PORT_DIPNAME( 0x0006, 0x0006, DEF_STR( Coin_A ) ) PORT_CONDITION("DSW", 0x0001, NOTEQUALS, 0x0001) PORT_DIPLOCATION("SW1:2,3")
	PORT_DIPSETTING(      0x0000, "5C/1C or Free if Coin B too" )
	PORT_DIPSETTING(      0x0002, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0006, DEF_STR( 1C_1C ) )
	PORT_DIPNAME( 0x0018, 0x0018, DEF_STR( Coin_B ) ) PORT_CONDITION("DSW", 0x0001, NOTEQUALS, 0x0001) PORT_DIPLOCATION("SW1:4,5")
	PORT_DIPSETTING(      0x0018, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(      0x0000, "1C/6C or Free if Coin A too" )

	PORT_DIPNAME( 0x0020, 0x0020, "Credits to Start" ) PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(      0x0020, "1" )
	PORT_DIPSETTING(      0x0000, "2" )
	PORT_DIPUNUSED_DIPLOC( 0x0040, 0x0040, "SW1:7")
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Flip_Screen ) ) PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0300, 0x0300, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(      0x0200, "1" )
	PORT_DIPSETTING(      0x0100, "2" )
	PORT_DIPSETTING(      0x0300, "3" )
	PORT_DIPSETTING(      0x0000, "5" )
	PORT_DIPNAME( 0x0c00, 0x0c00, DEF_STR( Bonus_Life ) ) PORT_DIPLOCATION("SW2:3,4")
	PORT_DIPSETTING(      0x0800, "80000 300000" )
	PORT_DIPSETTING(      0x0c00, "150000 400000" )
	PORT_DIPSETTING(      0x0400, "300000 1000000" )
	PORT_DIPSETTING(      0x0000, "1000000 5000000" )
	PORT_DIPNAME( 0x3000, 0x3000, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW2:5,6")
	PORT_DIPSETTING(      0x2000, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x3000, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Very_Hard ) )
	PORT_DIPNAME( 0x4000, 0x4000, DEF_STR( Allow_Continue ) ) PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(      0x0000, DEF_STR( No ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x8000, 0x8000, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x8000, DEF_STR( On ) )
INPUT_PORTS_END


/******************************************************************************/

static const gfx_layout raiden_charlayout =
{
	8,8,        /* 8*8 characters */
	2048,       /* 512 characters */
	4,          /* 4 bits per pixel */
	{ 4,0,(0x08000*8)+4,0x08000*8  },
	{ 0,1,2,3,8,9,10,11 },
	{ 0*16, 1*16, 2*16, 3*16, 4*16, 5*16, 6*16, 7*16 },
	128
};

static const gfx_layout raiden_spritelayout =
{
	16,16,      /* 16*16 tiles */
	4096,       /* 2048*4 tiles */
	4,          /* 4 bits per pixel */
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

static GFXDECODE_START( raiden )
	GFXDECODE_ENTRY( "gfx1", 0, raiden_charlayout,   768, 16 )
	GFXDECODE_ENTRY( "gfx2", 0, raiden_spritelayout,   0, 16 )
	GFXDECODE_ENTRY( "gfx3", 0, raiden_spritelayout, 256, 16 )
	GFXDECODE_ENTRY( "gfx4", 0, raiden_spritelayout, 512, 16 )
GFXDECODE_END


/******************************************************************************/

INTERRUPT_GEN_MEMBER(raiden_state::raiden_interrupt)
{
	device.execute().set_input_line_and_vector(0, HOLD_LINE, 0xc8/4); /* VBL */
}

static MACHINE_CONFIG_START( raiden, raiden_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", V30,XTAL_20MHz/2) /* NEC V30 CPU, 20MHz verified on pcb */
	MCFG_CPU_PROGRAM_MAP(main_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", raiden_state, raiden_interrupt)

	MCFG_CPU_ADD("sub", V30,XTAL_20MHz/2) /* NEC V30 CPU, 20MHz verified on pcb */
	MCFG_CPU_PROGRAM_MAP(sub_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", raiden_state, raiden_interrupt)

	SEIBU_SOUND_SYSTEM_CPU(XTAL_14_31818MHz/4) /* verified on pcb */

	MCFG_QUANTUM_TIME(attotime::from_hz(12000))

	/* video hardware */
	MCFG_BUFFERED_SPRITERAM16_ADD("spriteram")

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(59.60) /* verified on pcb */
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500) /* not accurate */)
	MCFG_SCREEN_SIZE(32*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 2*8, 30*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(raiden_state, screen_update_raiden)
	MCFG_SCREEN_VBLANK_DEVICE("spriteram", buffered_spriteram16_device, vblank_copy_rising)
	MCFG_SCREEN_PALETTE("palette")
	MCFG_SCREEN_ORIENTATION(ROT270)

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", raiden)
	MCFG_PALETTE_ADD("palette", 2048)
	MCFG_PALETTE_FORMAT(xxxxBBBBGGGGRRRR)

	/* sound hardware */
	SEIBU_SOUND_SYSTEM_YM3812_RAIDEN_INTERFACE(XTAL_14_31818MHz/4,XTAL_12MHz/12) // frequency and pin 7 verified (pin set in audio\seibu.h)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( raidene, raiden )
	SEIBU_SOUND_SYSTEM_ENCRYPTED_FULL()
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( raidenu, raidene )

	/* basic machine hardware */
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(raidenu_main_map)

	MCFG_CPU_MODIFY("sub")
	MCFG_CPU_PROGRAM_MAP(raidenu_sub_map)
MACHINE_CONFIG_END

WRITE16_MEMBER( raiden_state::raidenb_layer_scroll_w )
{
	COMBINE_DATA(&m_raidenb_scroll_ram[offset]);
}

static MACHINE_CONFIG_DERIVED( raidenb, raiden )

	/* basic machine hardware */
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(raidenb_main_map)

	/* video hardware */
	MCFG_VIDEO_START_OVERRIDE(raiden_state,raidenb)

	MCFG_DEVICE_ADD("crtc", SEIBU_CRTC, 0)
	MCFG_SEIBU_CRTC_LAYER_EN_CB(WRITE16(raiden_state, raidenb_layer_enable_w))
	MCFG_SEIBU_CRTC_LAYER_SCROLL_CB(WRITE16(raiden_state, raidenb_layer_scroll_w))

	MCFG_SCREEN_MODIFY("screen")
	MCFG_SCREEN_UPDATE_DRIVER(raiden_state, screen_update_raidenb)
MACHINE_CONFIG_END


/***************************************************************************/

/*

Note: Seibu labeled the roms simply as 1 through 10 and didn't generally
      change the labels at all between versions even though the data was
      different between them.
*/

/* These versions use the same board and make use of the region byte at 0x1fffe (0x1fffd also may differ and is used for unknown purpose) */

ROM_START( raiden ) /* from a board with 2 daughter cards, no official board #s? */
	ROM_REGION( 0x100000, "maincpu", 0 ) /* v30 main cpu */
	ROM_LOAD16_BYTE( "1.u0253", 0x0a0000, 0x10000, CRC(a4b12785) SHA1(446314e82ce01315cb3e3d1f323eaa2ad6fb48dd) )
	ROM_LOAD16_BYTE( "2.u0252", 0x0a0001, 0x10000, CRC(17640bd5) SHA1(5bbc99900426b1a072b52537ae9a50220c378a0d) )
	ROM_LOAD16_BYTE( "3.u022",  0x0c0000, 0x20000, CRC(f6af09d0) SHA1(ecd49f3351359ea2d5cbd140c9962d45c5544ecd) ) /* both 3 & 4 had a red "dot" on label, 4 also had printed "J" */
	ROM_LOAD16_BYTE( "4j.u023", 0x0c0001, 0x20000, CRC(505c4c5d) SHA1(07f61fd1ff24f482a1ae2f86c4c0f32850cbd539) ) /* 0x1fffd == 0x00, 0x1fffe == 0x04 */

	ROM_REGION( 0x100000, "sub", 0 ) /* v30 sub cpu */
	ROM_LOAD16_BYTE( "5.u042", 0x0c0000, 0x20000, CRC(ed03562e) SHA1(bf6b44fb53fa2321cd52c00fcb43b8ceb6ceffff) )
	ROM_LOAD16_BYTE( "6.u043", 0x0c0001, 0x20000, CRC(a19d5b5d) SHA1(aa5e5be60b737913e5677f88ebc218302245e5af) )

	ROM_REGION( 0x20000, "audiocpu", 0 ) /* 64k code for sound Z80 */
	ROM_LOAD( "8.u212",      0x000000, 0x08000, CRC(cbe055c7) SHA1(34a06a541d059c621d87fdf41546c9d052a61963) )
	ROM_CONTINUE(            0x010000, 0x08000 )
	ROM_COPY( "audiocpu", 0, 0x018000, 0x08000 )

	ROM_REGION( 0x010000, "gfx1", 0 ) /* Chars */
	ROM_LOAD( "9",  0x00000, 0x08000, CRC(1922b25e) SHA1(da27122dd1c43770e7385ad602ef397c64d2f754) ) /* On some PCBs there is no explicit */
	ROM_LOAD( "10", 0x08000, 0x08000, CRC(5f90786a) SHA1(4f63b07c6afbcf5196a433f3356bef984fe303ef) ) /* U location for these two roms     */

	ROM_REGION( 0x080000, "gfx2", 0 ) /* tiles */
	ROM_LOAD( "sei420", 0x00000, 0x80000, CRC(da151f0b) SHA1(02682497caf5f058331f18c652471829fa08d54f) ) /* tiles @ U105 on this PCB */

	ROM_REGION( 0x080000, "gfx3", 0 ) /* tiles */
	ROM_LOAD( "sei430", 0x00000, 0x80000, CRC(ac1f57ac) SHA1(1de926a0db73b99904ef119ac816c53d1551156a) ) /* tiles @ U115 on this PCB */

	ROM_REGION( 0x090000, "gfx4", 0 ) /* sprites */
	ROM_LOAD( "sei440",  0x00000, 0x80000, CRC(946d7bde) SHA1(30e8755c2b1ca8bff6278710b8422b51f75eec10) )

	ROM_REGION( 0x40000, "oki", 0 )  /* ADPCM samples */
	ROM_LOAD( "7.u203", 0x00000, 0x10000, CRC(8f927822) SHA1(592f2719f2c448c3b4b239eeaec078b411e12dbb) )
ROM_END

ROM_START( raidena )
	ROM_REGION( 0x100000, "maincpu", 0 ) /* v30 main cpu */
	ROM_LOAD16_BYTE( "1.u0253", 0x0a0000, 0x10000, CRC(a4b12785) SHA1(446314e82ce01315cb3e3d1f323eaa2ad6fb48dd) )
	ROM_LOAD16_BYTE( "2.u0252", 0x0a0001, 0x10000, CRC(17640bd5) SHA1(5bbc99900426b1a072b52537ae9a50220c378a0d) )
	ROM_LOAD16_BYTE( "3.u022",  0x0c0000, 0x20000, CRC(f6af09d0) SHA1(ecd49f3351359ea2d5cbd140c9962d45c5544ecd) )
	ROM_LOAD16_BYTE( "4.u023",  0x0c0001, 0x20000, CRC(6bdfd416) SHA1(7c3692d0c46c0fd360b9b2b5a8dc55d9217be357) ) /* 0x1fffd == 0x00, 0x1fffe == 0x84 */

	ROM_REGION( 0x100000, "sub", 0 ) /* v30 sub cpu */
	ROM_LOAD16_BYTE( "5.u042", 0x0c0000, 0x20000, CRC(ed03562e) SHA1(bf6b44fb53fa2321cd52c00fcb43b8ceb6ceffff) )
	ROM_LOAD16_BYTE( "6.u043", 0x0c0001, 0x20000, CRC(a19d5b5d) SHA1(aa5e5be60b737913e5677f88ebc218302245e5af) )

	ROM_REGION( 0x20000, "audiocpu", 0 ) /* 64k code for sound Z80 */
	ROM_LOAD( "8.u212",      0x000000, 0x08000, CRC(cbe055c7) SHA1(34a06a541d059c621d87fdf41546c9d052a61963) )
	ROM_CONTINUE(            0x010000, 0x08000 )
	ROM_COPY( "audiocpu", 0, 0x018000, 0x08000 )

	ROM_REGION( 0x010000, "gfx1", 0 ) /* Chars */
	ROM_LOAD( "9",  0x00000, 0x08000, CRC(1922b25e) SHA1(da27122dd1c43770e7385ad602ef397c64d2f754) ) /* On some PCBs there is no explicit */
	ROM_LOAD( "10", 0x08000, 0x08000, CRC(5f90786a) SHA1(4f63b07c6afbcf5196a433f3356bef984fe303ef) ) /* U location for these two roms     */

	ROM_REGION( 0x080000, "gfx2", 0 ) /* tiles */
	ROM_LOAD( "sei420", 0x00000, 0x80000, CRC(da151f0b) SHA1(02682497caf5f058331f18c652471829fa08d54f) ) /* tiles @ U105 on this PCB */

	ROM_REGION( 0x080000, "gfx3", 0 ) /* tiles */
	ROM_LOAD( "sei430", 0x00000, 0x80000, CRC(ac1f57ac) SHA1(1de926a0db73b99904ef119ac816c53d1551156a) ) /* tiles @ U115 on this PCB */

	ROM_REGION( 0x090000, "gfx4", 0 ) /* sprites */
	ROM_LOAD( "sei440",  0x00000, 0x80000, CRC(946d7bde) SHA1(30e8755c2b1ca8bff6278710b8422b51f75eec10) )

	ROM_REGION( 0x40000, "oki", 0 )  /* ADPCM samples */
	ROM_LOAD( "7.u203", 0x00000, 0x10000, CRC(8f927822) SHA1(592f2719f2c448c3b4b239eeaec078b411e12dbb) )
ROM_END

ROM_START( raident )
	ROM_REGION( 0x100000, "maincpu", 0 ) /* v30 main cpu */
	ROM_LOAD16_BYTE( "1.u0253", 0x0a0000, 0x10000, CRC(a4b12785) SHA1(446314e82ce01315cb3e3d1f323eaa2ad6fb48dd) )
	ROM_LOAD16_BYTE( "2.u0252", 0x0a0001, 0x10000, CRC(17640bd5) SHA1(5bbc99900426b1a072b52537ae9a50220c378a0d) )
	ROM_LOAD16_BYTE( "3.u022",  0x0c0000, 0x20000, CRC(f6af09d0) SHA1(ecd49f3351359ea2d5cbd140c9962d45c5544ecd) )
	ROM_LOAD16_BYTE( "4t.u023", 0x0c0001, 0x20000, CRC(61eefab1) SHA1(a886ce1eb1c6451b1cf9eb8dbdc2d484d9881ced) ) /* 0x1fffd == 0x02, 0x1fffe == 0x06 */

	ROM_REGION( 0x100000, "sub", 0 ) /* v30 sub cpu */
	ROM_LOAD16_BYTE( "5.u042", 0x0c0000, 0x20000, CRC(ed03562e) SHA1(bf6b44fb53fa2321cd52c00fcb43b8ceb6ceffff) )
	ROM_LOAD16_BYTE( "6.u043", 0x0c0001, 0x20000, CRC(a19d5b5d) SHA1(aa5e5be60b737913e5677f88ebc218302245e5af) )

	ROM_REGION( 0x20000, "audiocpu", 0 ) /* 64k code for sound Z80 */
	ROM_LOAD( "8.u212",      0x000000, 0x08000, CRC(cbe055c7) SHA1(34a06a541d059c621d87fdf41546c9d052a61963) )
	ROM_CONTINUE(            0x010000, 0x08000 )
	ROM_COPY( "audiocpu", 0, 0x018000, 0x08000 )

	ROM_REGION( 0x010000, "gfx1", 0 ) /* Chars */
	ROM_LOAD( "9",  0x00000, 0x08000, CRC(1922b25e) SHA1(da27122dd1c43770e7385ad602ef397c64d2f754) ) /* On some PCBs there is no explicit */
	ROM_LOAD( "10", 0x08000, 0x08000, CRC(5f90786a) SHA1(4f63b07c6afbcf5196a433f3356bef984fe303ef) ) /* U location for these two roms     */

	ROM_REGION( 0x080000, "gfx2", 0 ) /* tiles */
	ROM_LOAD( "sei420", 0x00000, 0x80000, CRC(da151f0b) SHA1(02682497caf5f058331f18c652471829fa08d54f) ) /* tiles @ U105 on this PCB */

	ROM_REGION( 0x080000, "gfx3", 0 ) /* tiles */
	ROM_LOAD( "sei430", 0x00000, 0x80000, CRC(ac1f57ac) SHA1(1de926a0db73b99904ef119ac816c53d1551156a) ) /* tiles @ U115 on this PCB */

	ROM_REGION( 0x090000, "gfx4", 0 ) /* sprites */
	ROM_LOAD( "sei440",  0x00000, 0x80000, CRC(946d7bde) SHA1(30e8755c2b1ca8bff6278710b8422b51f75eec10) )

	ROM_REGION( 0x40000, "oki", 0 )  /* ADPCM samples */
	ROM_LOAD( "7.u203", 0x00000, 0x10000, CRC(8f927822) SHA1(592f2719f2c448c3b4b239eeaec078b411e12dbb) )
ROM_END

ROM_START( raidenu )
	ROM_REGION( 0x100000, "maincpu", 0 ) /* v30 main cpu */
	ROM_LOAD16_BYTE( "1.u0253", 0x0a0000, 0x10000, CRC(a4b12785) SHA1(446314e82ce01315cb3e3d1f323eaa2ad6fb48dd) )
	ROM_LOAD16_BYTE( "2.u0252", 0x0a0001, 0x10000, CRC(17640bd5) SHA1(5bbc99900426b1a072b52537ae9a50220c378a0d) )
	ROM_LOAD16_BYTE( "3a.u022", 0x0c0000, 0x20000, CRC(a8fadbdd) SHA1(a23729a51c45c1dba4e625503a37d111ae72ced0) ) /* Both 3A & 4A different for the US version */
	ROM_LOAD16_BYTE( "4a.u023", 0x0c0001, 0x20000, CRC(bafb268d) SHA1(132d3ebf9d9d5fffa3040338106fad428c54dbaa) ) /* 0x1fffd == 0x01, 0x1fffe == 0x85 */

	ROM_REGION( 0x100000, "sub", 0 ) /* v30 sub cpu */
	ROM_LOAD16_BYTE( "5.u042", 0x0c0000, 0x20000, CRC(ed03562e) SHA1(bf6b44fb53fa2321cd52c00fcb43b8ceb6ceffff) )
	ROM_LOAD16_BYTE( "6.u043", 0x0c0001, 0x20000, CRC(a19d5b5d) SHA1(aa5e5be60b737913e5677f88ebc218302245e5af) )

	ROM_REGION( 0x20000, "audiocpu", 0 ) /* 64k code for sound Z80 */
	ROM_LOAD( "8.u212",      0x000000, 0x08000, CRC(cbe055c7) SHA1(34a06a541d059c621d87fdf41546c9d052a61963) )
	ROM_CONTINUE(            0x010000, 0x08000 )
	ROM_COPY( "audiocpu", 0, 0x018000, 0x08000 )

	ROM_REGION( 0x010000, "gfx1", 0 ) /* Chars */
	ROM_LOAD( "9",  0x00000, 0x08000, CRC(1922b25e) SHA1(da27122dd1c43770e7385ad602ef397c64d2f754) ) /* On some PCBs there is no explicit */
	ROM_LOAD( "10", 0x08000, 0x08000, CRC(5f90786a) SHA1(4f63b07c6afbcf5196a433f3356bef984fe303ef) ) /* U location for these two roms     */

	ROM_REGION( 0x080000, "gfx2", 0 ) /* tiles */
	ROM_LOAD( "sei420", 0x00000, 0x80000, CRC(da151f0b) SHA1(02682497caf5f058331f18c652471829fa08d54f) ) /* tiles @ U105 on this PCB */

	ROM_REGION( 0x080000, "gfx3", 0 ) /* tiles */
	ROM_LOAD( "sei430", 0x00000, 0x80000, CRC(ac1f57ac) SHA1(1de926a0db73b99904ef119ac816c53d1551156a) ) /* tiles @ U115 on this PCB */

	ROM_REGION( 0x090000, "gfx4", 0 ) /* sprites */
	ROM_LOAD( "sei440",  0x00000, 0x80000, CRC(946d7bde) SHA1(30e8755c2b1ca8bff6278710b8422b51f75eec10) )

	ROM_REGION( 0x40000, "oki", 0 )  /* ADPCM samples */
	ROM_LOAD( "7.u203", 0x00000, 0x10000, CRC(8f927822) SHA1(592f2719f2c448c3b4b239eeaec078b411e12dbb) )
ROM_END

ROM_START( raidenk ) /* Same board as above. Not sure why the sound CPU would be decrypted */
	ROM_REGION( 0x100000, "maincpu", 0 ) /* v30 main cpu */
	ROM_LOAD16_BYTE( "1.u0253", 0x0a0000, 0x10000, CRC(a4b12785) SHA1(446314e82ce01315cb3e3d1f323eaa2ad6fb48dd) )
	ROM_LOAD16_BYTE( "2.u0252", 0x0a0001, 0x10000, CRC(17640bd5) SHA1(5bbc99900426b1a072b52537ae9a50220c378a0d) )
	ROM_LOAD16_BYTE( "3.u022",  0x0c0000, 0x20000, CRC(f6af09d0) SHA1(ecd49f3351359ea2d5cbd140c9962d45c5544ecd) )
	ROM_LOAD16_BYTE( "4k.u023", 0x0c0001, 0x20000, CRC(fddf24da) SHA1(ececed0b0b96d070d85bfb6174029142bc96d5f0) ) /* 0x1fffd == 0x02, 0x1fffe == 0xA4 */

	ROM_REGION( 0x100000, "sub", 0 ) /* v30 sub cpu */
	ROM_LOAD16_BYTE( "5.u042", 0x0c0000, 0x20000, CRC(ed03562e) SHA1(bf6b44fb53fa2321cd52c00fcb43b8ceb6ceffff) )
	ROM_LOAD16_BYTE( "6.u043", 0x0c0001, 0x20000, CRC(a19d5b5d) SHA1(aa5e5be60b737913e5677f88ebc218302245e5af) )

	ROM_REGION( 0x20000, "audiocpu", 0 ) /* 64k code for sound Z80 */
	ROM_LOAD( "8b.u212",     0x000000, 0x08000, CRC(99ee7505) SHA1(b97c8ee5e26e8554b5de506fba3b32cc2fde53c9) ) /* Not encrypted */
	ROM_CONTINUE(            0x010000, 0x08000 )
	ROM_COPY( "audiocpu", 0, 0x018000, 0x08000 )

	ROM_REGION( 0x010000, "gfx1", 0 ) /* Chars */
	ROM_LOAD( "9",  0x00000, 0x08000, CRC(1922b25e) SHA1(da27122dd1c43770e7385ad602ef397c64d2f754) ) /* On some PCBs there is no explicit */
	ROM_LOAD( "10", 0x08000, 0x08000, CRC(5f90786a) SHA1(4f63b07c6afbcf5196a433f3356bef984fe303ef) ) /* U location for these two roms     */

	ROM_REGION( 0x080000, "gfx2", 0 ) /* tiles */
	ROM_LOAD( "sei420", 0x00000, 0x80000, CRC(da151f0b) SHA1(02682497caf5f058331f18c652471829fa08d54f) ) /* tiles @ U105 on this PCB */

	ROM_REGION( 0x080000, "gfx3", 0 ) /* tiles */
	ROM_LOAD( "sei430", 0x00000, 0x80000, CRC(ac1f57ac) SHA1(1de926a0db73b99904ef119ac816c53d1551156a) ) /* tiles @ U115 on this PCB */

	ROM_REGION( 0x090000, "gfx4", 0 ) /* sprites */
	ROM_LOAD( "sei440",  0x00000, 0x80000, CRC(946d7bde) SHA1(30e8755c2b1ca8bff6278710b8422b51f75eec10) )

	ROM_REGION( 0x40000, "oki", 0 )  /* ADPCM samples */
	ROM_LOAD( "7.u203", 0x00000, 0x10000, CRC(8f927822) SHA1(592f2719f2c448c3b4b239eeaec078b411e12dbb) )
ROM_END

ROM_START( raidenb )/* Different hardware, Main & Sub CPU code not encrypted. */
	ROM_REGION( 0x100000, "maincpu", 0 ) /* v30 main cpu */
	ROM_LOAD16_BYTE( "1.u0253", 0x0a0000, 0x10000, CRC(a4b12785) SHA1(446314e82ce01315cb3e3d1f323eaa2ad6fb48dd) )
	ROM_LOAD16_BYTE( "2.u0252", 0x0a0001, 0x10000, CRC(17640bd5) SHA1(5bbc99900426b1a072b52537ae9a50220c378a0d) )
	ROM_LOAD16_BYTE( "3__(raidenb).u022", 0x0c0000, 0x20000, CRC(9d735bf5) SHA1(531981eac2ef0c0635f067a649899f98738d5c67) ) /* Simply labeled as 3 */
	ROM_LOAD16_BYTE( "4__(raidenb).u023", 0x0c0001, 0x20000, CRC(8d184b99) SHA1(71cd4179aa2341d2ceecbb6a9c26f5919d46ca4c) ) /* Simply labeled as 4 */

	ROM_REGION( 0x100000, "sub", 0 ) /* v30 sub cpu */
	ROM_LOAD16_BYTE( "5__(raidenb).u042", 0x0c0000, 0x20000, CRC(7aca6d61) SHA1(4d80ec87e54d7495b9bdf819b9985b1c8183c80d) ) /* Simply labeled as 5 */
	ROM_LOAD16_BYTE( "6__(raidenb).u043", 0x0c0001, 0x20000, CRC(e3d35cc2) SHA1(4329865985aaf3fb524618e2e958563c8fa6ead5) ) /* Simply labeled as 6 */

	ROM_REGION( 0x20000, "audiocpu", 0 ) /* 64k code for sound Z80 */
	ROM_LOAD( "rai6.u212",   0x000000, 0x08000, CRC(723a483b) SHA1(50e67945e83ea1748fb748de3287d26446d4e0a0) ) /* Should be labeled "8" ??? */
	ROM_CONTINUE(            0x010000, 0x08000 )
	ROM_COPY( "audiocpu", 0, 0x018000, 0x08000 )

	ROM_REGION( 0x010000, "gfx1", 0 ) /* Chars */
	ROM_LOAD( "9",  0x00000, 0x08000, CRC(1922b25e) SHA1(da27122dd1c43770e7385ad602ef397c64d2f754) ) /* On some PCBs there is no explicit */
	ROM_LOAD( "10", 0x08000, 0x08000, CRC(5f90786a) SHA1(4f63b07c6afbcf5196a433f3356bef984fe303ef) ) /* U location for these two roms     */

	ROM_REGION( 0x080000, "gfx2", 0 ) /* tiles */
	ROM_LOAD( "sei420", 0x00000, 0x80000, CRC(da151f0b) SHA1(02682497caf5f058331f18c652471829fa08d54f) ) /* U919 on this PCB */

	ROM_REGION( 0x080000, "gfx3", 0 ) /* tiles */
	ROM_LOAD( "sei430", 0x00000, 0x80000, CRC(ac1f57ac) SHA1(1de926a0db73b99904ef119ac816c53d1551156a) ) /* U920 on this PCB */

	ROM_REGION( 0x090000, "gfx4", 0 ) /* Sprites */
	ROM_LOAD( "sei440", 0x00000, 0x80000, CRC(946d7bde) SHA1(30e8755c2b1ca8bff6278710b8422b51f75eec10) ) /* U165 on this PCB */

	ROM_REGION( 0x40000, "oki", 0 )  /* ADPCM samples */
	ROM_LOAD( "7.u203", 0x00000, 0x10000, CRC(8f927822) SHA1(592f2719f2c448c3b4b239eeaec078b411e12dbb) )

	ROM_REGION( 0x2000, "plds", 0 )
	ROM_LOAD( "ep910pc-1.bin", 0x0000, 0x0884, NO_DUMP ) /* PAL is read protected */
	ROM_LOAD( "ep910pc-2.bin", 0x1000, 0x0884, NO_DUMP ) /* PAL is read protected */
ROM_END

ROM_START( raidenua )/* Different hardware, Main, Sub & sound CPU code not encrypted. */
	ROM_REGION( 0x100000, "maincpu", 0 ) /* v30 main cpu */
	ROM_LOAD16_BYTE( "1.c8",   0x0a0000, 0x10000, CRC(a4b12785) SHA1(446314e82ce01315cb3e3d1f323eaa2ad6fb48dd) )
	ROM_LOAD16_BYTE( "2.c7",   0x0a0001, 0x10000, CRC(17640bd5) SHA1(5bbc99900426b1a072b52537ae9a50220c378a0d) )
	ROM_LOAD16_BYTE( "3dd.e8", 0x0c0000, 0x20000, CRC(b6f3bad2) SHA1(214474ab9fa65e2716155b77d7825951cc98148a) )
	ROM_LOAD16_BYTE( "4dd.e7", 0x0c0001, 0x20000, CRC(d294dfc1) SHA1(03606ddfa35d5cb34c447fa370495e1fbb0cad0e) )

	ROM_REGION( 0x100000, "sub", 0 ) /* v30 sub cpu */
	ROM_LOAD16_BYTE( "5.p8", 0x0c0000, 0x20000, CRC(15c1cf45) SHA1(daac732a1d3e8f36fa665f984e05651cbca74fef) )
	ROM_LOAD16_BYTE( "6.p7", 0x0c0001, 0x20000, CRC(261c381b) SHA1(64a9e0ea9abcba6287829cf4abb806362b62c806) )

	ROM_REGION( 0x20000, "audiocpu", 0 ) /* 64k code for sound Z80 */
	ROM_LOAD( "8.w8",        0x00000, 0x08000, CRC(105b9c11) SHA1(eb142806f8410d584d914b91207361a15ab18e6f) )
	ROM_CONTINUE(            0x10000, 0x08000 )
	ROM_COPY( "audiocpu", 0, 0x18000, 0x08000 )

	ROM_REGION( 0x010000, "gfx1", 0 ) /* Chars */
	ROM_LOAD( "9",  0x00000, 0x08000, CRC(1922b25e) SHA1(da27122dd1c43770e7385ad602ef397c64d2f754) ) /* U016 on this PCB */
	ROM_LOAD( "10", 0x08000, 0x08000, CRC(5f90786a) SHA1(4f63b07c6afbcf5196a433f3356bef984fe303ef) ) /* U017 on this PCB */

	ROM_REGION( 0x080000, "gfx2", 0 ) /* tiles */
	ROM_LOAD( "sei420", 0x00000, 0x80000, CRC(da151f0b) SHA1(02682497caf5f058331f18c652471829fa08d54f) ) /* U011 on this PCB */

	ROM_REGION( 0x080000, "gfx3", 0 ) /* tiles */
	ROM_LOAD( "sei430", 0x00000, 0x80000, CRC(ac1f57ac) SHA1(1de926a0db73b99904ef119ac816c53d1551156a) ) /* U013 on this PCB */

	ROM_REGION( 0x090000, "gfx4", 0 ) /* Sprites */
	ROM_LOAD( "sei440",  0x00000, 0x80000, CRC(946d7bde) SHA1(30e8755c2b1ca8bff6278710b8422b51f75eec10) ) /* U012 on this PCB */

	ROM_REGION( 0x40000, "oki", 0 )  /* ADPCM samples */
	ROM_LOAD( "7.x10", 0x00000, 0x10000, CRC(2051263e) SHA1(dff96caa11adf619360d88704e3af8427ddfe524) )
ROM_END


/***************************************************************************/

/* This is based on code by Niclas Karlsson Mate, who figured out the
encryption method! The technique is a combination of a XOR table plus
bit-swapping */
void raiden_state::common_decrypt()
{
	UINT16 *RAM = (UINT16 *)memregion("maincpu")->base();
	int i;

	for (i = 0; i < 0x20000; i++)
	{
		static const UINT16 xor_table[] = { 0x200e,0x0006,0x000a,0x0002,0x240e,0x000e,0x04c2,0x00c2,0x008c,0x0004,0x0088,0x0000,0x048c,0x000c,0x04c0,0x00c0 };
		UINT16 data = RAM[0xc0000/2 + i];
		data ^= xor_table[i & 0x0f];
		data = BITSWAP16(data, 15,14,10,12,11,13,9,8,3,2,5,4,7,1,6,0);
		RAM[0xc0000/2 + i] = data;
	}

	RAM = (UINT16 *)memregion("sub")->base();

	for (i = 0; i < 0x20000; i++)
	{
		static const UINT16 xor_table[] = { 0x0080,0x0080,0x0244,0x0288,0x0288,0x0288,0x1041,0x1009 };
		UINT16 data = RAM[0xc0000/2 + i];
		data ^= xor_table[i & 0x07];
		data = BITSWAP16(data, 15,14,13,9,11,10,12,8,2,0,5,4,7,3,1,6);
		RAM[0xc0000/2 + i] = data;
	}
}


DRIVER_INIT_MEMBER(raiden_state,raiden)
{
	common_decrypt();
}

DRIVER_INIT_MEMBER(raiden_state,raidenk)
{
	common_decrypt();
}

DRIVER_INIT_MEMBER(raiden_state,raidenu)
{
}


/***************************************************************************/

/* Same PCB, differ by region byte(s) */
GAME( 1990, raiden,   0,      raidene, raiden, raiden_state,  raiden,  ROT270, "Seibu Kaihatsu", "Raiden (set 1)", MACHINE_SUPPORTS_SAVE )
GAME( 1990, raidena,  raiden, raidene, raiden, raiden_state,  raiden,  ROT270, "Seibu Kaihatsu", "Raiden (set 2)", MACHINE_SUPPORTS_SAVE )
GAME( 1990, raidenu,  raiden, raidene, raiden, raiden_state,  raiden,  ROT270, "Seibu Kaihatsu (Fabtek license)", "Raiden (US set 1)", MACHINE_SUPPORTS_SAVE )
GAME( 1990, raident,  raiden, raidene, raiden, raiden_state,  raiden,  ROT270, "Seibu Kaihatsu (Liang HWA Electronics license)", "Raiden (Taiwan)", MACHINE_SUPPORTS_SAVE )

/* Same as above, but the sound CPU code is not encrypted */
GAME( 1990, raidenk,  raiden, raiden,  raiden, raiden_state,  raidenk, ROT270, "Seibu Kaihatsu (IBL Corporation license)", "Raiden (Korea)", MACHINE_SUPPORTS_SAVE )

/* Alternate hardware; SEI8904 + SEI9008 PCBs. Main & Sub CPU code not encrypted */
GAME( 1990, raidenua, raiden, raidenu, raiden, raiden_state,  raidenu, ROT270, "Seibu Kaihatsu (Fabtek license)", "Raiden (US set 2)", MACHINE_SUPPORTS_SAVE )

/* Alternate hardware. Main, Sub & Sound CPU code not encrypted - could possibly be a bootleg?? It also sports Seibu custom CRTC. */
GAME( 1990, raidenb,  raiden, raidenb, raiden, driver_device, 0,       ROT270, "Seibu Kaihatsu", "Raiden (set 3)", MACHINE_SUPPORTS_SAVE )
