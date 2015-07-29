// license:BSD-3-Clause
// copyright-holders:Yochizo
/***************************************************************************

Exzisus
-------------------------------------
driver by Yochizo

This driver is heavily dependent on the Raine source.
Very thanks to Richard Bush and the Raine team.


Supported games :
==================
   Exzisus (2 sets)    (C) 1987 Taito


System specs :
===============
   CPU       : Z80 x 4
   Sound     : YM2151
   Chips     : TC0010VCU x 2 + TC0140SYT

   There are two types of Exzisus PCB:

   * The first (K1100256A) has separate RGB outputs for the background and sprites.
   Exactly how they are combined to form the final image is unknown.
   * The second, later PCB has a single video output and is JAMMA compliant.

TODO:
- There must be a way for cpu a to stop cpu c, otherwise the RAM check in test
  mode cannot work. However, the only way I found to do that is making writes
  to F404 pulse the reset line, which isn't a common way to handle these things.

****************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "includes/taitoipt.h"
#include "audio/taitosnd.h"
#include "sound/2151intf.h"
#include "includes/exzisus.h"


/***************************************************************************

  Memory Handler(s)

***************************************************************************/

WRITE8_MEMBER(exzisus_state::cpua_bankswitch_w)
{
	membank("cpuabank")->set_entry(data & 0x0f);
	flip_screen_set(data & 0x40);
}

WRITE8_MEMBER(exzisus_state::cpub_bankswitch_w)
{
	membank("cpubbank")->set_entry(data & 0x0f);
	flip_screen_set(data & 0x40);
}

WRITE8_MEMBER(exzisus_state::coincounter_w)
{
	coin_lockout_w(machine(), 0,~data & 0x01);
	coin_lockout_w(machine(), 1,~data & 0x02);
	coin_counter_w(machine(), 0,data & 0x04);
	coin_counter_w(machine(), 1,data & 0x08);
}

// is it ok that cpub_reset refers to cpuc?
WRITE8_MEMBER(exzisus_state::cpub_reset_w)
{
	m_cpuc->set_input_line(INPUT_LINE_RESET, PULSE_LINE);
}

#if 0
// without cpub_reset_w, the following patch would be needed for
// the RAM check to work
DRIVER_INIT_MEMBER(exzisus_state,exzisus)
{
	UINT8 *RAM = memregion("cpua")->base();

	/* Fix WORK RAM error */
	RAM[0x67fd] = 0x18;

	/* Fix ROM 1 error */
	RAM[0x6829] = 0x18;
}
#endif


/**************************************************************************

  Memory Map(s)

**************************************************************************/

static ADDRESS_MAP_START( cpua_map, AS_PROGRAM, 8, exzisus_state )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0x8000, 0xbfff) AM_ROMBANK("cpuabank")
	AM_RANGE(0xc000, 0xc5ff) AM_RAM AM_SHARE("objectram1")
	AM_RANGE(0xc600, 0xdfff) AM_RAM AM_SHARE("videoram1")
	AM_RANGE(0xe000, 0xefff) AM_RAM AM_SHARE("sharedram_ac")
	AM_RANGE(0xf400, 0xf400) AM_WRITE(cpua_bankswitch_w)
	AM_RANGE(0xf404, 0xf404) AM_WRITE(cpub_reset_w) // ??
	AM_RANGE(0xf800, 0xffff) AM_RAM AM_SHARE("sharedram_ab")
ADDRESS_MAP_END

static ADDRESS_MAP_START( cpub_map, AS_PROGRAM, 8, exzisus_state )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0x8000, 0xbfff) AM_ROMBANK("cpubbank")
	AM_RANGE(0xc000, 0xc5ff) AM_RAM AM_SHARE("objectram0")
	AM_RANGE(0xc600, 0xdfff) AM_RAM AM_SHARE("videoram0")
	AM_RANGE(0xe000, 0xefff) AM_RAM
	AM_RANGE(0xf000, 0xf000) AM_READNOP AM_DEVWRITE("tc0140syt", tc0140syt_device, master_port_w)
	AM_RANGE(0xf001, 0xf001) AM_DEVREADWRITE("tc0140syt", tc0140syt_device, master_comm_r, master_comm_w)
	AM_RANGE(0xf400, 0xf400) AM_READ_PORT("P1")
	AM_RANGE(0xf400, 0xf400) AM_WRITE(cpub_bankswitch_w)
	AM_RANGE(0xf401, 0xf401) AM_READ_PORT("P2")
	AM_RANGE(0xf402, 0xf402) AM_READ_PORT("SYSTEM")
	AM_RANGE(0xf402, 0xf402) AM_WRITE(coincounter_w)
	AM_RANGE(0xf404, 0xf404) AM_READ_PORT("DSWA")
	AM_RANGE(0xf404, 0xf404) AM_WRITENOP // ??
	AM_RANGE(0xf405, 0xf405) AM_READ_PORT("DSWB")
	AM_RANGE(0xf800, 0xffff) AM_RAM AM_SHARE("sharedram_ab")
ADDRESS_MAP_END

static ADDRESS_MAP_START( cpuc_map, AS_PROGRAM, 8, exzisus_state )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0x8000, 0x85ff) AM_RAM AM_SHARE("objectram1")
	AM_RANGE(0x8600, 0x9fff) AM_RAM AM_SHARE("videoram1")
	AM_RANGE(0xa000, 0xafff) AM_RAM AM_SHARE("sharedram_ac")
	AM_RANGE(0xb000, 0xbfff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( sound_map, AS_PROGRAM, 8, exzisus_state )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0x8000, 0x8fff) AM_RAM
	AM_RANGE(0x9000, 0x9001) AM_DEVREADWRITE("ymsnd", ym2151_device, read, write)
	AM_RANGE(0xa000, 0xa000) AM_READNOP AM_DEVWRITE("tc0140syt", tc0140syt_device, slave_port_w)
	AM_RANGE(0xa001, 0xa001) AM_DEVREADWRITE("tc0140syt", tc0140syt_device, slave_comm_r, slave_comm_w)
ADDRESS_MAP_END


/***************************************************************************

  Input Port(s)

***************************************************************************/

static INPUT_PORTS_START( exzisus )
	PORT_START("P1")
	TAITO_JOY_UDRL_2_BUTTONS( 1 )

	PORT_START("P2")
	TAITO_JOY_UDRL_2_BUTTONS( 2 )

	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_LOW,  IPT_SERVICE1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW,  IPT_TILT )
	PORT_BIT( 0x04, IP_ACTIVE_LOW,  IPT_START1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW,  IPT_START2 )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0xc0, IP_ACTIVE_LOW,  IPT_UNKNOWN )

	PORT_START("DSWA")
	TAITO_MACHINE_COCKTAIL
	TAITO_COINAGE_JAPAN_OLD

	PORT_START("DSWB")
	TAITO_DIFFICULTY
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x08, "100k and every 150k" )
	PORT_DIPSETTING(    0x0c, "150k and every 200k" )
	PORT_DIPSETTING(    0x04, "150k" )
	PORT_DIPSETTING(    0x00, "200k" )
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPSETTING(    0x30, "3" )
	PORT_DIPSETTING(    0x20, "4" )
	PORT_DIPSETTING(    0x10, "5" )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unused ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Service Mode (buggy)" )      // buggy: all other switches in DSW2 must be on
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END


/***************************************************************************

  Machine Driver(s)

***************************************************************************/

void exzisus_state::machine_start()
{
	membank("cpuabank")->configure_entries(0, 16, memregion("cpua")->base(), 0x4000);
	membank("cpubbank")->configure_entries(0, 16, memregion("cpub")->base(), 0x4000);
}

static const gfx_layout charlayout =
{
	8, 8,
	RGN_FRAC(1,2),
	4,
	{ RGN_FRAC(1,2), RGN_FRAC(1,2)+4, 0, 4 },
	{ 3, 2, 1, 0, 8+3, 8+2, 8+1, 8+0 },
	{ 0*16, 1*16, 2*16, 3*16, 4*16, 5*16, 6*16, 7*16 },
	16*8
};

static GFXDECODE_START( exzisus )
	GFXDECODE_ENTRY( "bg0", 0, charlayout,   0, 256 )
	GFXDECODE_ENTRY( "bg1", 0, charlayout, 256, 256 )
GFXDECODE_END



/* All clocks are unconfirmed */
static MACHINE_CONFIG_START( exzisus, exzisus_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("cpua", Z80, 6000000)
	MCFG_CPU_PROGRAM_MAP(cpua_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", exzisus_state,  irq0_line_hold)

	MCFG_CPU_ADD("cpub", Z80, 6000000)
	MCFG_CPU_PROGRAM_MAP(cpub_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", exzisus_state,  irq0_line_hold)

	MCFG_CPU_ADD("cpuc", Z80, 6000000)
	MCFG_CPU_PROGRAM_MAP(cpuc_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", exzisus_state,  irq0_line_hold)

	MCFG_CPU_ADD("audiocpu", Z80, 4000000)
	MCFG_CPU_PROGRAM_MAP(sound_map)

	MCFG_QUANTUM_TIME(attotime::from_hz(600))   /* 10 CPU slices per frame - enough for the sound CPU to read all commands */

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(32*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 2*8, 30*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(exzisus_state, screen_update)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", exzisus)
	MCFG_PALETTE_ADD_RRRRGGGGBBBB_PROMS("palette", 1024)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_YM2151_ADD("ymsnd", 4000000)
	MCFG_YM2151_IRQ_HANDLER(INPUTLINE("audiocpu", 0))
	MCFG_SOUND_ROUTE(0, "mono", 0.50)
	MCFG_SOUND_ROUTE(1, "mono", 0.50)

	MCFG_DEVICE_ADD("tc0140syt", TC0140SYT, 0)
	MCFG_TC0140SYT_MASTER_CPU("cpub")
	MCFG_TC0140SYT_SLAVE_CPU("audiocpu")
MACHINE_CONFIG_END


/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( exzisus )
	ROM_REGION( 0x40000, "cpua", 0 )
	ROM_LOAD( "b12-09.7d",  0x00000, 0x10000, CRC(e80f49a9) SHA1(3995d52195cdadfa82ff992ec0456fce09e75132) )
	ROM_LOAD( "b12-11.9d",  0x10000, 0x10000, CRC(11fcda2c) SHA1(4f8d1dff339d96ffadde2cc7eec23cfeb42481f2) )

	ROM_REGION( 0x40000, "cpub", 0 )
	ROM_LOAD( "b12-10.7f",  0x00000, 0x10000, CRC(a60227f1) SHA1(1e0d09f6b77794095092316fe8bf823d4c7775bb) )
	ROM_LOAD( "b12-12.8f",  0x10000, 0x10000, CRC(a662be67) SHA1(0643480d56d8ac020288db800a705dd5d0d3ad9f) )
	ROM_LOAD( "b12-13.10f", 0x20000, 0x10000, CRC(04a29633) SHA1(39476365241718f01f9630c12467cb24791a67e1) )

	ROM_REGION( 0x10000, "cpuc", 0 )
	ROM_LOAD( "b12-14.12c", 0x00000, 0x08000, CRC(b5ce5e75) SHA1(6d5ec788684e1be4c727ac02b9fa313a42985b40) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "b12-21.19f", 0x00000, 0x08000, CRC(b7e0f00e) SHA1(f79ef0dee6bd29c09b8e5c586514200e3fbaa87e) )

	ROM_REGION( 0x80000, "bg0", ROMREGION_INVERT )
	ROM_LOAD( "b12-16.17d", 0x00000, 0x10000, CRC(6fec6acb) SHA1(2289c116d3f6093988a088d011f192dd4a99aa77) )
	ROM_LOAD( "b12-18.19d", 0x10000, 0x10000, CRC(64e358aa) SHA1(cd1a23458b1a2f9c8c8aea8086dc04e0f6cc6908) )
	ROM_LOAD( "b12-20.20d", 0x20000, 0x10000, CRC(87f52e89) SHA1(3f8530aca087fa2a32dc6dfbcfe2f86604ee3ca1) )
	ROM_LOAD( "b12-15.17c", 0x40000, 0x10000, CRC(d81107c8) SHA1(c024c9b7956de493687e1373318d4cd74b3555b2) )
	ROM_LOAD( "b12-17.19c", 0x50000, 0x10000, CRC(db1d5a6c) SHA1(c2e1b8d92c2b3b2ce775ed50ca4a37e84ed35a93) )
	ROM_LOAD( "b12-19.20c", 0x60000, 0x10000, CRC(772b2641) SHA1(35cc6d5a725f1817791e710afde992e64d14104f) )

	ROM_REGION( 0x80000, "bg1", ROMREGION_INVERT )
	ROM_LOAD( "b12-05.1c",  0x00000, 0x10000, CRC(be5c5cc1) SHA1(af50c1ee0ce134871ea636c0e939f1a007a1cc13) )
	ROM_LOAD( "b12-07.3c",  0x10000, 0x10000, CRC(9353e39f) SHA1(576620818eb7a50a86aac0376a58ac22a29bb16d) )
	ROM_LOAD( "b12-06.1d",  0x40000, 0x10000, CRC(8571e6ed) SHA1(0a3228408f4d2afe3744172d24ae41d6400c30b6) )
	ROM_LOAD( "b12-08.3d",  0x50000, 0x10000, CRC(55ea5cca) SHA1(5717652ca028ff7a55c40573d52755985ed77ef7) )

	ROM_REGION( 0x00c00, "proms", 0 )
	ROM_LOAD( "b12-27.13l", 0x00000, 0x00100, CRC(524c9a01) SHA1(1894de29ba15a26043706ca4c5ca33aa8373447a) )
	ROM_LOAD( "b12-24.6m",  0x00100, 0x00100, CRC(1aa5bde9) SHA1(1bb6d5614183ff98600c5555ec8f5c545648e55c) )
	ROM_LOAD( "b12-26.12l", 0x00400, 0x00100, CRC(65f42c61) SHA1(7dc493d918f16661e3524c4189e785edfd345dbb) )
	ROM_LOAD( "b12-23.4m",  0x00500, 0x00100, CRC(fad4db5f) SHA1(bb1169ed6147fb9ac413f0d63e428dd190e7641d) )
	ROM_LOAD( "b12-25.11l", 0x00800, 0x00100, CRC(3e30f45b) SHA1(aa1f5278975c101f03feb16cac0bd074a6a41d2c) )
	ROM_LOAD( "b12-22.2m",  0x00900, 0x00100, CRC(936855d2) SHA1(57bb7cb40462f37e49c4ff0d9c833bbd2fb78428) )
ROM_END

ROM_START( exzisusa )
	ROM_REGION( 0x40000, "cpua", 0 )
	ROM_LOAD( "b23-10.7d",  0x00000, 0x10000, CRC(c80216fc) SHA1(7b952779c420be08573768f09bd65d0a188df024) )
	ROM_LOAD( "b23-12.9d",  0x10000, 0x10000, CRC(13637f54) SHA1(c175bc60120e32eec6ccca822fa497a42dd59823) )

	ROM_REGION( 0x40000, "cpub", 0 )
	ROM_LOAD( "b23-11.7f",  0x00000, 0x10000, CRC(d6a79cef) SHA1(e2b56aa38c017b24b50f304b9fe49ee14006f9a4) )
	ROM_LOAD( "b12-12.8f",  0x10000, 0x10000, CRC(a662be67) SHA1(0643480d56d8ac020288db800a705dd5d0d3ad9f) )
	ROM_LOAD( "b12-13.10f", 0x20000, 0x10000, CRC(04a29633) SHA1(39476365241718f01f9630c12467cb24791a67e1) )

	ROM_REGION( 0x10000, "cpuc", 0 )
	ROM_LOAD( "b23-13.12c", 0x00000, 0x08000, CRC(51110aa1) SHA1(34c2701625eb1987affad1efd19ff8c9971456ae) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "b23-14.19f", 0x00000, 0x08000, CRC(f7ca7df2) SHA1(6048d9341f0303546e447a76439e1927d14cdd57) )

	ROM_REGION( 0x80000, "bg0", ROMREGION_INVERT )
	ROM_LOAD( "b12-16.17d", 0x00000, 0x10000, CRC(6fec6acb) SHA1(2289c116d3f6093988a088d011f192dd4a99aa77) )
	ROM_LOAD( "b12-18.19d", 0x10000, 0x10000, CRC(64e358aa) SHA1(cd1a23458b1a2f9c8c8aea8086dc04e0f6cc6908) )
	ROM_LOAD( "b12-20.20d", 0x20000, 0x10000, CRC(87f52e89) SHA1(3f8530aca087fa2a32dc6dfbcfe2f86604ee3ca1) )
	ROM_LOAD( "b12-15.17c", 0x40000, 0x10000, CRC(d81107c8) SHA1(c024c9b7956de493687e1373318d4cd74b3555b2) )
	ROM_LOAD( "b12-17.19c", 0x50000, 0x10000, CRC(db1d5a6c) SHA1(c2e1b8d92c2b3b2ce775ed50ca4a37e84ed35a93) )
	ROM_LOAD( "b12-19.20c", 0x60000, 0x10000, CRC(772b2641) SHA1(35cc6d5a725f1817791e710afde992e64d14104f) )

	ROM_REGION( 0x80000, "bg1", ROMREGION_INVERT )
	ROM_LOAD( "b23-06.1c",  0x00000, 0x10000, CRC(44f8f661) SHA1(d77160a89e45556cd9ce211d89c398e1086d8d92) )
	ROM_LOAD( "b23-08.3c",  0x10000, 0x10000, CRC(1ce498c1) SHA1(a9ce3de997089bd40c99bd89919b459c9f215fc8) )
	ROM_LOAD( "b23-07.1d",  0x40000, 0x10000, CRC(d7f6ec89) SHA1(e8da207ddaf46ceff870b45ecec0e89c499291b4) )
	ROM_LOAD( "b23-09.3d",  0x50000, 0x10000, CRC(6651617f) SHA1(6351a0b01589cb181b896285ade70e9dfcd799ec) )

	ROM_REGION( 0x00c00, "proms", 0 )
	/* These appear to be twice the correct size */
	ROM_LOAD( "b23-04.15l", 0x00000, 0x00400, CRC(5042cffa) SHA1(c969748866a12681cf2dbf25a46da2c4e4f92313) )
	ROM_LOAD( "b23-03.14l", 0x00400, 0x00400, BAD_DUMP CRC(9458fd45) SHA1(7f7cdacf37bb6f15de1109fa73ba3c5fc88893d0) ) /* D0 is fixed */
	ROM_LOAD( "b23-05.16l", 0x00800, 0x00400, CRC(87f0f69a) SHA1(37df6fd56245fab9beaabfd86fd8f95d7c42c2a5) )
ROM_END

ROM_START( exzisust )
	ROM_REGION( 0x40000, "cpua", 0 )
	ROM_LOAD( "b23-10.7d",  0x00000, 0x10000, CRC(c80216fc) SHA1(7b952779c420be08573768f09bd65d0a188df024) )
	ROM_LOAD( "b23-12.9d",  0x10000, 0x10000, CRC(13637f54) SHA1(c175bc60120e32eec6ccca822fa497a42dd59823) )

	ROM_REGION( 0x40000, "cpub", 0 )
	ROM_LOAD( "b23-15.7f",  0x00000, 0x10000, CRC(2f8b3752) SHA1(acfbb8aa20e6b031b9543e1e56268f3f5c7f7f07) )
	ROM_LOAD( "b12-12.8f",  0x10000, 0x10000, CRC(a662be67) SHA1(0643480d56d8ac020288db800a705dd5d0d3ad9f) )
	ROM_LOAD( "b12-13.10f", 0x20000, 0x10000, CRC(04a29633) SHA1(39476365241718f01f9630c12467cb24791a67e1) )

	ROM_REGION( 0x10000, "cpuc", 0 )
	ROM_LOAD( "b23-13.12c", 0x00000, 0x08000, CRC(51110aa1) SHA1(34c2701625eb1987affad1efd19ff8c9971456ae) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "b23-14.19f", 0x00000, 0x08000, CRC(f7ca7df2) SHA1(6048d9341f0303546e447a76439e1927d14cdd57) )

	ROM_REGION( 0x80000, "bg0", ROMREGION_INVERT )
	ROM_LOAD( "b12-16.17d", 0x00000, 0x10000, CRC(6fec6acb) SHA1(2289c116d3f6093988a088d011f192dd4a99aa77) )
	ROM_LOAD( "b12-18.19d", 0x10000, 0x10000, CRC(64e358aa) SHA1(cd1a23458b1a2f9c8c8aea8086dc04e0f6cc6908) )
	ROM_LOAD( "b12-20.20d", 0x20000, 0x10000, CRC(87f52e89) SHA1(3f8530aca087fa2a32dc6dfbcfe2f86604ee3ca1) )
	ROM_LOAD( "b12-15.17c", 0x40000, 0x10000, CRC(d81107c8) SHA1(c024c9b7956de493687e1373318d4cd74b3555b2) )
	ROM_LOAD( "b12-17.19c", 0x50000, 0x10000, CRC(db1d5a6c) SHA1(c2e1b8d92c2b3b2ce775ed50ca4a37e84ed35a93) )
	ROM_LOAD( "b12-19.20c", 0x60000, 0x10000, CRC(772b2641) SHA1(35cc6d5a725f1817791e710afde992e64d14104f) )

	ROM_REGION( 0x80000, "bg1", ROMREGION_INVERT )
	ROM_LOAD( "b23-06.1c",  0x00000, 0x10000, CRC(44f8f661) SHA1(d77160a89e45556cd9ce211d89c398e1086d8d92) )
	ROM_LOAD( "b23-08.3c",  0x10000, 0x10000, CRC(1ce498c1) SHA1(a9ce3de997089bd40c99bd89919b459c9f215fc8) )
	ROM_LOAD( "b23-07.1d",  0x40000, 0x10000, CRC(d7f6ec89) SHA1(e8da207ddaf46ceff870b45ecec0e89c499291b4) )
	ROM_LOAD( "b23-09.3d",  0x50000, 0x10000, CRC(6651617f) SHA1(6351a0b01589cb181b896285ade70e9dfcd799ec) )

	ROM_REGION( 0x00c00, "proms", 0 )
	/* These appear to be twice the correct size */
	ROM_LOAD( "b23-04.15l", 0x00000, 0x00400, CRC(5042cffa) SHA1(c969748866a12681cf2dbf25a46da2c4e4f92313) )
	ROM_LOAD( "b23-03.14l", 0x00400, 0x00400, BAD_DUMP CRC(9458fd45) SHA1(7f7cdacf37bb6f15de1109fa73ba3c5fc88893d0) ) /* D0 is fixed */
	ROM_LOAD( "b23-05.16l", 0x00800, 0x00400, CRC(87f0f69a) SHA1(37df6fd56245fab9beaabfd86fd8f95d7c42c2a5) )
ROM_END

GAME( 1987, exzisus,  0,       exzisus, exzisus, driver_device, 0, ROT0, "Taito Corporation", "Exzisus (Japan, dedicated)",  MACHINE_SUPPORTS_SAVE )
GAME( 1987, exzisusa, exzisus, exzisus, exzisus, driver_device, 0, ROT0, "Taito Corporation", "Exzisus (Japan, conversion)", MACHINE_SUPPORTS_SAVE )
GAME( 1987, exzisust, exzisus, exzisus, exzisus, driver_device, 0, ROT0, "Taito Corporation (TAD license)", "Exzisus (TAD license)", MACHINE_SUPPORTS_SAVE )
