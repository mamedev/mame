// license:???
// copyright-holders:insideoutboy
/*
Super Cross II (JPN Ver.)
(c)1986 GM Shoji

C2-00172-D
CPU  :Z80B
Sound:SN76489 x3

SCS-24.4E
SCS-25.4C
SCS-26.4B
SCS-27.5K
SCS-28.5J
SCS-29.5H
SCS-30.5F

SC-62.3A
SC-63.3B
SC-64.6A

C2-00171-D
CPU  :Z80B
OSC  :10.000MHz

SCM-00.10L
SCM-01.10K
SCM-02.10J
SCM-03.10G
SCM-20.5K
SCM-21.5G
SCM-22.5E
SCM-23.5B

SC-60.4K
SC-61.5A

Notes:

- sprites pop in at the wrong place sometimes before entering the screen

- correct drawing/animation of bg is very sensitive to cpu speed/interrupts/
  interleave, current settings aren't correct but don't think there's any
  visible problems

- engine rev sound may not be completely correct

- bg not using second half of prom, of interest is this half is identical to
  the second half of a bankp/appoooh prom, hardware is similar to bankp/appoooh
  in a few ways, there's also an unused SEGA logo in the bg graphics

- fg not using odd colours, shouldn't matter as the colours are duplicated

- sprite priorities are wrong when bikes are jumping as they are ordered on
  vertical position only, assume this is original behaviour
*/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "sound/sn76496.h"
#include "includes/sprcros2.h"




WRITE8_MEMBER(sprcros2_state::m_port7_w)
{
	//76543210
	//x------- unused
	//-x------ bankswitch halves of scm-01.10k into c000-dfff
	//--xx---- unused
	//----x--- irq enable
	//-----x-- ?? off with title flash and screen clears, possibly layer/sprite enable
	//------x- flip screen
	//-------x nmi enable

	if((m_port7^data)&0x40)
		membank("masterbank")->set_entry((data&0x40)>>6);

	machine().tilemap().set_flip_all(data&0x02?(TILEMAP_FLIPX|TILEMAP_FLIPY):0 );

	m_port7 = data;
}

WRITE8_MEMBER(sprcros2_state::s_port3_w)
{
	//76543210
	//xxxx---- unused
	//----x--- bankswitch halves of scs-27.5k into c000-dfff
	//-----xx- unused
	//-------x nmi enable

	if((m_s_port3^data)&0x08)
		membank("slavebank")->set_entry((data&0x08)>>3);

	m_s_port3 = data;
}

static ADDRESS_MAP_START( sprcros2_master_map, AS_PROGRAM, 8, sprcros2_state )
	AM_RANGE(0x0000, 0xbfff) AM_ROM
	AM_RANGE(0xc000, 0xdfff) AM_ROMBANK("masterbank")
	AM_RANGE(0xe000, 0xe7ff) AM_RAM_WRITE(fgvideoram_w) AM_SHARE("fgvideoram")
	AM_RANGE(0xe800, 0xe817) AM_RAM                     //always zero
	AM_RANGE(0xe818, 0xe83f) AM_RAM AM_SHARE("spriteram")
	AM_RANGE(0xe840, 0xefff) AM_RAM                     //always zero
	AM_RANGE(0xf000, 0xf7ff) AM_RAM
	AM_RANGE(0xf800, 0xffff) AM_RAM AM_SHARE("share1")          //shared with slave cpu
ADDRESS_MAP_END

static ADDRESS_MAP_START( sprcros2_master_io_map, AS_IO, 8, sprcros2_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x00) AM_READ_PORT("P1") AM_DEVWRITE("sn1", sn76489_device, write)
	AM_RANGE(0x01, 0x01) AM_READ_PORT("P2") AM_DEVWRITE("sn2", sn76489_device, write)
	AM_RANGE(0x02, 0x02) AM_READ_PORT("EXTRA") AM_DEVWRITE("sn3", sn76489_device, write)
	AM_RANGE(0x04, 0x04) AM_READ_PORT("DSW1")
	AM_RANGE(0x05, 0x05) AM_READ_PORT("DSW2")
	AM_RANGE(0x07, 0x07) AM_WRITE(m_port7_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( sprcros2_slave_map, AS_PROGRAM, 8, sprcros2_state )
	AM_RANGE(0x0000, 0xbfff) AM_ROM
	AM_RANGE(0xc000, 0xdfff) AM_ROMBANK("slavebank")
	AM_RANGE(0xe000, 0xe7ff) AM_RAM_WRITE(bgvideoram_w) AM_SHARE("bgvideoram")
	AM_RANGE(0xe800, 0xefff) AM_RAM                     //always zero
	AM_RANGE(0xf000, 0xf7ff) AM_RAM
	AM_RANGE(0xf800, 0xffff) AM_RAM AM_SHARE("share1")
ADDRESS_MAP_END

static ADDRESS_MAP_START( sprcros2_slave_io_map, AS_IO, 8, sprcros2_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x00) AM_WRITE(bgscrollx_w)
	AM_RANGE(0x01, 0x01) AM_WRITE(bgscrolly_w)
	AM_RANGE(0x03, 0x03) AM_WRITE(s_port3_w)
ADDRESS_MAP_END

static INPUT_PORTS_START( sprcros2 )
	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("EXTRA")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_COIN3 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON2 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_COCKTAIL
	PORT_BIT( 0xc0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x07, 0x00, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 1C_6C ) )
	PORT_BIT( 0xf8, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ) )
	PORT_BIT( 0x0e, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x70, IP_ACTIVE_HIGH, IPT_UNUSED )            //unused coinage bits
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )
INPUT_PORTS_END

static const gfx_layout sprcros2_bglayout =
{
	8,8,
	RGN_FRAC(1,3),
	3,
	{ RGN_FRAC(0,3), RGN_FRAC(1,3), RGN_FRAC(2,3) },
	{ STEP8(0,1) },
	{ STEP8(0,8) },
	8*8
};

static const gfx_layout sprcros2_spritelayout =
{
	32,32,
	RGN_FRAC(1,3),
	3,
	{ RGN_FRAC(0,3), RGN_FRAC(1,3), RGN_FRAC(2,3) },
	{ STEP8(0,1), STEP8(256,1), STEP8(512,1), STEP8(768,1) },
	{ STEP16(0,8), STEP16(128,8) },
	32*32
};

static const gfx_layout sprcros2_fglayout =
{
	8,8,
	RGN_FRAC(1,1),
	2,
	{ 0, 4 },
	{ STEP4(64,1), STEP4(0,1) },
	{ STEP8(0,8) },
	8*8*2
};

static GFXDECODE_START( sprcros2 )
	GFXDECODE_ENTRY( "gfx1", 0, sprcros2_bglayout,     0,   16 )
	GFXDECODE_ENTRY( "gfx2", 0, sprcros2_spritelayout, 256, 6  )
	GFXDECODE_ENTRY( "gfx3", 0, sprcros2_fglayout,     512, 64 )
GFXDECODE_END

TIMER_DEVICE_CALLBACK_MEMBER(sprcros2_state::m_interrupt)
{
	int scanline = param;

	if (scanline == 240)
	{
		if(m_port7&0x01)
			m_master->set_input_line(INPUT_LINE_NMI, PULSE_LINE);
	}
	else if(scanline == 0)
	{
		if(m_port7&0x08)
			m_master->set_input_line(0, HOLD_LINE);
	}
}

INTERRUPT_GEN_MEMBER(sprcros2_state::s_interrupt)
{
	if(m_s_port3&0x01)
		device.execute().set_input_line(INPUT_LINE_NMI, PULSE_LINE);
}

void sprcros2_state::machine_start()
{
	membank("masterbank")->configure_entries(0, 2, memregion("master")->base() + 0x10000, 0x2000);
	membank("slavebank")->configure_entries(0, 2, memregion("slave")->base() + 0x10000, 0x2000);

	save_item(NAME(m_port7));
	save_item(NAME(m_s_port3));
}

static MACHINE_CONFIG_START( sprcros2, sprcros2_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("master", Z80,10000000/2)
	MCFG_CPU_PROGRAM_MAP(sprcros2_master_map)
	MCFG_CPU_IO_MAP(sprcros2_master_io_map)
	MCFG_TIMER_DRIVER_ADD_SCANLINE("scantimer", sprcros2_state, m_interrupt, "screen", 0, 1)

	MCFG_CPU_ADD("slave", Z80,10000000/2)
	MCFG_CPU_PROGRAM_MAP(sprcros2_slave_map)
	MCFG_CPU_IO_MAP(sprcros2_slave_io_map)
	MCFG_CPU_PERIODIC_INT_DRIVER(sprcros2_state, s_interrupt, 2*60)    //2 nmis


	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(32*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(1*8, 31*8-1, 2*8, 30*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(sprcros2_state, screen_update)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", sprcros2)
	MCFG_PALETTE_ADD("palette", 768)
	MCFG_PALETTE_INDIRECT_ENTRIES(32)
	MCFG_PALETTE_INIT_OWNER(sprcros2_state, sprcros2)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("sn1", SN76489, 10000000/4)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)

	MCFG_SOUND_ADD("sn2", SN76489, 10000000/4)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)

	MCFG_SOUND_ADD("sn3", SN76489, 10000000/4)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)
MACHINE_CONFIG_END

ROM_START( sprcros2 )
	ROM_REGION( 0x14000, "master", 0 )
	ROM_LOAD( "scm-03.10g", 0x00000, 0x4000, CRC(b9757908) SHA1(d59cb2aac1b6268fc766306850f5711d4a12d897) )
	ROM_LOAD( "scm-02.10j", 0x04000, 0x4000, CRC(849c5c87) SHA1(0e02c4990e371d6a290efa53301818e769648945) )
	ROM_LOAD( "scm-01.10k", 0x08000, 0x4000, CRC(385a62de) SHA1(847bf9d97ab3fa8949d9198e4e509948a940d6aa) )

	ROM_LOAD( "scm-00.10l", 0x10000, 0x4000, CRC(13fa3684) SHA1(611b7a237e394f285dcc5beb027dacdbdd58a7a0) ) //banked into c000-dfff

	ROM_REGION( 0x14000, "slave", 0 )
	ROM_LOAD( "scs-30.5f",  0x00000, 0x4000, CRC(c0a40e41) SHA1(e74131b353855749258dffa45091c825ccdbf05a) )
	ROM_LOAD( "scs-29.5h",  0x04000, 0x4000, CRC(83d49fa5) SHA1(7112110df2f382bbc0e651adcec975054a485b9b) )
	ROM_LOAD( "scs-28.5j",  0x08000, 0x4000, CRC(480d351f) SHA1(d1b86f441ae0e58b30e0f089ab25de219d5f30e3) )

	ROM_LOAD( "scs-27.5k",  0x10000, 0x4000, CRC(2cf720cb) SHA1(a95c5b8c88371cf597bb7d80afeca6a48c7b74e6) ) //banked into c000-dfff

	ROM_REGION( 0xc000, "gfx1", 0 ) //bg
	ROM_LOAD( "scs-26.4b",   0x0000, 0x4000, CRC(f958b56d) SHA1(a1973179d336d2ba57294155550515f2b8a33a09) )
	ROM_LOAD( "scs-25.4c",   0x4000, 0x4000, CRC(d6fd7ba5) SHA1(1c26c4c1655b2be9cb6103e75386cc2f0cf27fc5) )
	ROM_LOAD( "scs-24.4e",   0x8000, 0x4000, CRC(87783c36) SHA1(7102be795afcddd76b4d41823e95c65fe1ffbca0) )

	ROM_REGION( 0xc000, "gfx2", 0 )
	ROM_LOAD( "scm-23.5b",   0x0000, 0x4000, CRC(ab42f8e3) SHA1(8c2213b7c47a48e223fc3f7d323d16c0e4cd0457) ) //sprites
	ROM_LOAD( "scm-22.5e",   0x4000, 0x4000, CRC(0cad254c) SHA1(36e30e30b652b3a388a3c4a82251196f79368f59) )
	ROM_LOAD( "scm-21.5g",   0x8000, 0x4000, CRC(b6b68998) SHA1(cc3c6d996beeedcc7e5199f10d65c5b1d3c6e666) )

	ROM_REGION( 0x4000, "gfx3", 0 ) //fg
	ROM_LOAD( "scm-20.5k",   0x0000, 0x4000, CRC(67a099a6) SHA1(43981abdcaa0ff36183027a3c691ce2df7f06ec7) )

	ROM_REGION( 0x0420, "proms", 0 )
	ROM_LOAD( "sc-64.6a",    0x0000, 0x0020, CRC(336dd1c0) SHA1(f0a0d2c13617fd84ee55c0cb96643761a8735147) ) //palette
	ROM_LOAD( "sc-63.3b",    0x0020, 0x0100, CRC(9034a059) SHA1(1801965b4f0f3e04ca4b3faf0ba3a27dbb008474) ) //bg clut lo nibble
	ROM_LOAD( "sc-62.3a",    0x0120, 0x0100, CRC(3c78a14f) SHA1(8f9c196a3e18bdce2d4855bc285bd5bde534bf09) ) //bg clut hi nibble
	ROM_LOAD( "sc-61.5a",    0x0220, 0x0100, CRC(2f71185d) SHA1(974fbb52285f01f4353e9acb1992dcd6fdefedcb) ) //sprite clut
	ROM_LOAD( "sc-60.4k",    0x0320, 0x0100, CRC(d7a4e57d) SHA1(6db02ec6aa55b05422cb505e63c71e36b4b11b4a) ) //fg clut
ROM_END

/* this is probably an old revision */
ROM_START( sprcros2a )
	ROM_REGION( 0x14000, "master", 0 )
	ROM_LOAD( "15.bin",     0x00000, 0x4000, CRC(b9d02558) SHA1(775404c6c7648d9dab02b496541739ea700cd481) )
	ROM_LOAD( "scm-02.10j", 0x04000, 0x4000, CRC(849c5c87) SHA1(0e02c4990e371d6a290efa53301818e769648945) )
	ROM_LOAD( "scm-01.10k", 0x08000, 0x4000, CRC(385a62de) SHA1(847bf9d97ab3fa8949d9198e4e509948a940d6aa) )

	ROM_LOAD( "scm-00.10l", 0x10000, 0x4000, CRC(13fa3684) SHA1(611b7a237e394f285dcc5beb027dacdbdd58a7a0) ) //banked into c000-dfff

	ROM_REGION( 0x14000, "slave", 0 )
	ROM_LOAD( "scs-30.5f",  0x00000, 0x4000, CRC(c0a40e41) SHA1(e74131b353855749258dffa45091c825ccdbf05a) )
	ROM_LOAD( "scs-29.5h",  0x04000, 0x4000, CRC(83d49fa5) SHA1(7112110df2f382bbc0e651adcec975054a485b9b) )
	ROM_LOAD( "scs-28.5j",  0x08000, 0x4000, CRC(480d351f) SHA1(d1b86f441ae0e58b30e0f089ab25de219d5f30e3) )

	ROM_LOAD( "scs-27.5k",  0x10000, 0x4000, CRC(2cf720cb) SHA1(a95c5b8c88371cf597bb7d80afeca6a48c7b74e6) ) //banked into c000-dfff

	ROM_REGION( 0xc000, "gfx1", 0 ) //bg
	ROM_LOAD( "scs-26.4b",   0x0000, 0x4000, CRC(f958b56d) SHA1(a1973179d336d2ba57294155550515f2b8a33a09) )
	ROM_LOAD( "scs-25.4c",   0x4000, 0x4000, CRC(d6fd7ba5) SHA1(1c26c4c1655b2be9cb6103e75386cc2f0cf27fc5) )
	ROM_LOAD( "scs-24.4e",   0x8000, 0x4000, CRC(87783c36) SHA1(7102be795afcddd76b4d41823e95c65fe1ffbca0) )

	ROM_REGION( 0xc000, "gfx2", 0 )
	ROM_LOAD( "scm-23.5b",   0x0000, 0x4000, CRC(ab42f8e3) SHA1(8c2213b7c47a48e223fc3f7d323d16c0e4cd0457) ) //sprites
	ROM_LOAD( "scm-22.5e",   0x4000, 0x4000, CRC(0cad254c) SHA1(36e30e30b652b3a388a3c4a82251196f79368f59) )
	ROM_LOAD( "scm-21.5g",   0x8000, 0x4000, CRC(b6b68998) SHA1(cc3c6d996beeedcc7e5199f10d65c5b1d3c6e666) )

	ROM_REGION( 0x4000, "gfx3", 0 ) //fg
	ROM_LOAD( "scm-20.5k",   0x0000, 0x4000, CRC(67a099a6) SHA1(43981abdcaa0ff36183027a3c691ce2df7f06ec7) )

	ROM_REGION( 0x0420, "proms", 0 )
	ROM_LOAD( "sc-64.6a",    0x0000, 0x0020, CRC(336dd1c0) SHA1(f0a0d2c13617fd84ee55c0cb96643761a8735147) ) //palette
	ROM_LOAD( "sc-63.3b",    0x0020, 0x0100, CRC(9034a059) SHA1(1801965b4f0f3e04ca4b3faf0ba3a27dbb008474) ) //bg clut lo nibble
	ROM_LOAD( "sc-62.3a",    0x0120, 0x0100, CRC(3c78a14f) SHA1(8f9c196a3e18bdce2d4855bc285bd5bde534bf09) ) //bg clut hi nibble
	ROM_LOAD( "sc-61.5a",    0x0220, 0x0100, CRC(2f71185d) SHA1(974fbb52285f01f4353e9acb1992dcd6fdefedcb) ) //sprite clut
	ROM_LOAD( "sc-60.4k",    0x0320, 0x0100, CRC(d7a4e57d) SHA1(6db02ec6aa55b05422cb505e63c71e36b4b11b4a) ) //fg clut
ROM_END

GAME( 1986, sprcros2, 0,        sprcros2, sprcros2, driver_device, 0, ROT0, "GM Shoji", "Super Cross II (Japan, set 1)", GAME_SUPPORTS_SAVE )
GAME( 1986, sprcros2a,sprcros2, sprcros2, sprcros2, driver_device, 0, ROT0, "GM Shoji", "Super Cross II (Japan, set 2)", GAME_SUPPORTS_SAVE )
