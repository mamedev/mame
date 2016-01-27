// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

Time Pilot 84  (c) 1984 Konami

---- Master 6809 ------

Write
 2000-27ff MAFR Watch dog ?
 2800      COL0 a register that index the colors Proms
 3000      reset IRQ
 3001      OUT2  Coin Counter 2
 3002      OUT1  Coin Counter 1
 3003      MUT
 3004      HREV  Flip Screen X
 3005      VREV  Flip Screen Y
 3006      -
 3007      GMED
 3800      SON   Sound on
 3A00      SDA   Sound data
 3C00      SHF0 SHF1 J2 J3 J4 J5 J6 J7  background Y position
 3E00      L0 - L7                      background X position

Read:
 2800      in0  Buttons 1
 2820      in1  Buttons 2
 2840      in2  Buttons 3
 2860      in3  Dip switches 1
 3000      in4  Dip switches 2
 3800      in5  Dip switches 3 (not used)

Read/Write
 4000-47ff Char ram, 2 pages
 4800-4fff Background character ram, 2 pages
 5000-57ff Ram (Common for the Master and Slave 6809)  0x5000-0x517f sprites data
 6000-ffff Rom (only from $8000 to $ffff is used in this game)


------ Slave 6809 --------
 0000-1fff SAFR Watch dog ?
 2000      beam position
 4000      enable or reset IRQ
 6000-67ff DRA
 8000-87ff Ram (Common for the Master and Slave 6809)
 E000-ffff Rom


------ Sound CPU (Z80) -----
There are 3 or 4 76489AN chips driven by the Z80

0000-1fff Rom program (A6)
2000-3fff Rom Program (A4) (not used or missing?)
4000-43ff Ram
6000-7fff Sound data in
8000-9fff Timer
A000-Bfff Filters
C000      Store Data that will go to one of the 76489AN
C001      76489 #1 trigger
C002      76489 #2 (optional) trigger
C003      76489 #3 trigger
C004      76489 #4 trigger

***************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "cpu/m6809/m6809.h"
#include "sound/sn76496.h"
#include "sound/flt_rc.h"
#include "includes/konamipt.h"
#include "includes/tp84.h"



void tp84_state::machine_start()
{
}


READ8_MEMBER(tp84_state::tp84_sh_timer_r)
{
	/* main xtal 14.318MHz, divided by 4 to get the CPU clock, further */
	/* divided by 2048 to get this timer */
	/* (divide by (2048/2), and not 1024, because the CPU cycle counter is */
	/* incremented every other state change of the clock) */
	return (m_audiocpu->total_cycles() / (2048/2)) & 0x0f;
}


WRITE8_MEMBER(tp84_state::tp84_filter_w)
{
	int C;

	/* 76489 #0 */
	C = 0;
	if (offset & 0x008) C +=  47000;    /*  47000pF = 0.047uF */
	if (offset & 0x010) C += 470000;    /* 470000pF = 0.47uF */
	dynamic_cast<filter_rc_device*>(machine().device("filter1"))->filter_rc_set_RC(FLT_RC_LOWPASS,1000,2200,1000,CAP_P(C));

	/* 76489 #1 (optional) */
	C = 0;
	if (offset & 0x020) C +=  47000;    /*  47000pF = 0.047uF */
	if (offset & 0x040) C += 470000;    /* 470000pF = 0.47uF */
		//  dynamic_cast<filter_rc_device*>(machine().device("filter2"))->filter_rc_set_RC(,1000,2200,1000,C);

	/* 76489 #2 */
	C = 0;
	if (offset & 0x080) C += 470000;    /* 470000pF = 0.47uF */
	dynamic_cast<filter_rc_device*>(machine().device("filter2"))->filter_rc_set_RC(FLT_RC_LOWPASS,1000,2200,1000,CAP_P(C));

	/* 76489 #3 */
	C = 0;
	if (offset & 0x100) C += 470000;    /* 470000pF = 0.47uF */
	dynamic_cast<filter_rc_device*>(machine().device("filter3"))->filter_rc_set_RC(FLT_RC_LOWPASS,1000,2200,1000,CAP_P(C));
}

WRITE8_MEMBER(tp84_state::tp84_sh_irqtrigger_w)
{
	m_audiocpu->set_input_line_and_vector(0,HOLD_LINE,0xff);
}



static ADDRESS_MAP_START( tp84_cpu1_map, AS_PROGRAM, 8, tp84_state )
	AM_RANGE(0x2000, 0x2000) AM_WRITE(watchdog_reset_w)
	AM_RANGE(0x2800, 0x2800) AM_READ_PORT("SYSTEM") AM_WRITEONLY AM_SHARE("palette_bank")
	AM_RANGE(0x2820, 0x2820) AM_READ_PORT("P1")
	AM_RANGE(0x2840, 0x2840) AM_READ_PORT("P2")
	AM_RANGE(0x2860, 0x2860) AM_READ_PORT("DSW1")
	AM_RANGE(0x3000, 0x3000) AM_READ_PORT("DSW2") AM_WRITEONLY
	AM_RANGE(0x3004, 0x3004) AM_WRITEONLY AM_SHARE("flipscreen_x")
	AM_RANGE(0x3005, 0x3005) AM_WRITEONLY AM_SHARE("flipscreen_y")
	AM_RANGE(0x3800, 0x3800) AM_WRITE(tp84_sh_irqtrigger_w)
	AM_RANGE(0x3a00, 0x3a00) AM_WRITE(soundlatch_byte_w)
	AM_RANGE(0x3c00, 0x3c00) AM_WRITEONLY AM_SHARE("scroll_x")
	AM_RANGE(0x3e00, 0x3e00) AM_WRITEONLY AM_SHARE("scroll_y")
	AM_RANGE(0x4000, 0x43ff) AM_RAM AM_SHARE("bg_videoram")
	AM_RANGE(0x4400, 0x47ff) AM_RAM AM_SHARE("fg_videoram")
	AM_RANGE(0x4800, 0x4bff) AM_RAM AM_SHARE("bg_colorram")
	AM_RANGE(0x4c00, 0x4fff) AM_RAM AM_SHARE("fg_colorram")
	AM_RANGE(0x5000, 0x57ff) AM_RAM AM_SHARE("share1")
	AM_RANGE(0x8000, 0xffff) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START( tp84b_cpu1_map, AS_PROGRAM, 8, tp84_state )
	AM_RANGE(0x0000, 0x03ff) AM_RAM AM_SHARE("bg_videoram")
	AM_RANGE(0x0400, 0x07ff) AM_RAM AM_SHARE("fg_videoram")
	AM_RANGE(0x0800, 0x0bff) AM_RAM AM_SHARE("bg_colorram")
	AM_RANGE(0x0c00, 0x0fff) AM_RAM AM_SHARE("fg_colorram")
	AM_RANGE(0x1000, 0x17ff) AM_RAM AM_SHARE("share1")
	AM_RANGE(0x1800, 0x1800) AM_WRITE(watchdog_reset_w)
	AM_RANGE(0x1a00, 0x1a00) AM_READ_PORT("SYSTEM") AM_WRITEONLY AM_SHARE("palette_bank")
	AM_RANGE(0x1a20, 0x1a20) AM_READ_PORT("P1")
	AM_RANGE(0x1a40, 0x1a40) AM_READ_PORT("P2")
	AM_RANGE(0x1a60, 0x1a60) AM_READ_PORT("DSW1")
	AM_RANGE(0x1c00, 0x1c00) AM_READ_PORT("DSW2") AM_WRITENOP
	AM_RANGE(0x1c04, 0x1c04) AM_WRITEONLY AM_SHARE("flipscreen_x")
	AM_RANGE(0x1c05, 0x1c05) AM_WRITEONLY AM_SHARE("flipscreen_y")
	AM_RANGE(0x1e00, 0x1e00) AM_WRITE(tp84_sh_irqtrigger_w)
	AM_RANGE(0x1e80, 0x1e80) AM_WRITE(soundlatch_byte_w)
	AM_RANGE(0x1f00, 0x1f00) AM_WRITEONLY AM_SHARE("scroll_x")
	AM_RANGE(0x1f80, 0x1f80) AM_WRITEONLY AM_SHARE("scroll_y")
	AM_RANGE(0x8000, 0xffff) AM_ROM
ADDRESS_MAP_END


WRITE8_MEMBER(tp84_state::sub_irq_mask_w)
{
	m_sub_irq_mask = data & 1;
}


static ADDRESS_MAP_START( cpu2_map, AS_PROGRAM, 8, tp84_state )
//  AM_RANGE(0x0000, 0x0000) AM_RAM /* Watch dog ?*/
	AM_RANGE(0x2000, 0x2000) AM_READ(tp84_scanline_r) /* beam position */
	AM_RANGE(0x4000, 0x4000) AM_WRITE(sub_irq_mask_w)
	AM_RANGE(0x6000, 0x679f) AM_RAM
	AM_RANGE(0x67a0, 0x67ff) AM_RAM_WRITE(tp84_spriteram_w) AM_SHARE("spriteram")
	AM_RANGE(0x8000, 0x87ff) AM_RAM AM_SHARE("share1")
	AM_RANGE(0xe000, 0xffff) AM_ROM
ADDRESS_MAP_END


static ADDRESS_MAP_START( audio_map, AS_PROGRAM, 8, tp84_state )
	AM_RANGE(0x0000, 0x3fff) AM_ROM
	AM_RANGE(0x4000, 0x43ff) AM_RAM
	AM_RANGE(0x6000, 0x6000) AM_READ(soundlatch_byte_r)
	AM_RANGE(0x8000, 0x8000) AM_READ(tp84_sh_timer_r)
	AM_RANGE(0xa000, 0xa1ff) AM_WRITE(tp84_filter_w)
	AM_RANGE(0xc000, 0xc000) AM_WRITENOP
	AM_RANGE(0xc001, 0xc001) AM_DEVWRITE("y2404_1", y2404_device, write)
	AM_RANGE(0xc003, 0xc003) AM_DEVWRITE("y2404_2", y2404_device, write)
	AM_RANGE(0xc004, 0xc004) AM_DEVWRITE("y2404_3", y2404_device, write)
ADDRESS_MAP_END



static INPUT_PORTS_START( tp84 )
	PORT_START("SYSTEM")
	KONAMI8_SYSTEM_UNK

	PORT_START("P1")
	KONAMI8_MONO_B12_UNK

	PORT_START("P2")
	KONAMI8_COCKTAIL_B12_UNK

	PORT_START("DSW1")
	KONAMI_COINAGE_LOC(DEF_STR( Free_Play ), "Invalid", SW1)
	/* "Invalid" = both coin slots disabled */

	PORT_START("DSW2")
	PORT_DIPNAME( 0x03, 0x02, DEF_STR( Lives ) )        PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(    0x03, "2" )
	PORT_DIPSETTING(    0x02, "3" )
	PORT_DIPSETTING(    0x01, "5" )
	PORT_DIPSETTING(    0x00, "7" )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Cabinet ) )      PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x18, 0x10, DEF_STR( Bonus_Life ) )   PORT_DIPLOCATION("SW2:4,5")
	PORT_DIPSETTING(    0x18, "10000 and every 50000" )
	PORT_DIPSETTING(    0x10, "20000 and every 60000" )
	PORT_DIPSETTING(    0x08, "30000 and every 70000" )
	PORT_DIPSETTING(    0x00, "40000 and every 80000" )
	PORT_DIPNAME( 0x60, 0x20, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SW2:6,7")
	PORT_DIPSETTING(    0x60, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Normal ) ) // JP default
	PORT_DIPSETTING(    0x20, DEF_STR( Hard ) )   // US default
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( tp84a )
	PORT_INCLUDE( tp84 )

	PORT_MODIFY("DSW2")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Lives ) )        PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(    0x03, "3" )
	PORT_DIPSETTING(    0x02, "4" )
	PORT_DIPSETTING(    0x01, "5" )
	PORT_DIPSETTING(    0x00, "7" )
INPUT_PORTS_END


static const gfx_layout charlayout =
{
	8,8,
	RGN_FRAC(1,1),
	2,
	{ 4, 0 },
	{  0, 1, 2, 3, 8*8+0, 8*8+1, 8*8+2, 8*8+3 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	16*8
};

static const gfx_layout spritelayout =
{
	16,16,
	RGN_FRAC(1,2),
	4,
	{ RGN_FRAC(1,2)+4, RGN_FRAC(1,2)+0, 4 ,0 },
	{ 0, 1, 2, 3, 8*8+0, 8*8+1, 8*8+2, 8*8+3,
			16*8+0, 16*8+1, 16*8+2, 16*8+3, 24*8+0, 24*8+1, 24*8+2, 24*8+3 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8,
			32*8, 33*8, 34*8, 35*8, 36*8, 37*8, 38*8, 39*8 },
	64*8
};

static GFXDECODE_START( tp84 )
	GFXDECODE_ENTRY( "gfx1", 0, charlayout,        0, 64*8 )
	GFXDECODE_ENTRY( "gfx2", 0, spritelayout, 64*4*8, 16*8 )
GFXDECODE_END

INTERRUPT_GEN_MEMBER(tp84_state::sub_vblank_irq)
{
	if(m_sub_irq_mask)
		device.execute().set_input_line(0, HOLD_LINE);
}


static MACHINE_CONFIG_START( tp84, tp84_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("cpu1",M6809, XTAL_18_432MHz/12) /* verified on pcb */
	MCFG_CPU_PROGRAM_MAP(tp84_cpu1_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", tp84_state,  irq0_line_hold)

	MCFG_CPU_ADD("sub", M6809, XTAL_18_432MHz/12)   /* verified on pcb */
	MCFG_CPU_PROGRAM_MAP(cpu2_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", tp84_state,  sub_vblank_irq)

	MCFG_CPU_ADD("audiocpu", Z80,XTAL_14_31818MHz/4) /* verified on pcb */
	MCFG_CPU_PROGRAM_MAP(audio_map)

	MCFG_QUANTUM_TIME(attotime::from_hz(6000))  /* 100 CPU slices per frame - an high value to ensure proper */
							/* synchronization of the CPUs */


	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(32*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 2*8, 30*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(tp84_state, screen_update_tp84)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", tp84)
	MCFG_PALETTE_ADD("palette", 4096)
	MCFG_PALETTE_INDIRECT_ENTRIES(256)
	MCFG_PALETTE_INIT_OWNER(tp84_state, tp84)

	/* audio hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("y2404_1", Y2404, XTAL_14_31818MHz/8) /* verified on pcb */
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "filter1", 0.75)

	MCFG_SOUND_ADD("y2404_2", Y2404, XTAL_14_31818MHz/8) /* verified on pcb */
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "filter2", 0.75)

	MCFG_SOUND_ADD("y2404_3", Y2404, XTAL_14_31818MHz/8) /* verified on pcb */
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "filter3", 0.75)

	MCFG_FILTER_RC_ADD("filter1", 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
	MCFG_FILTER_RC_ADD("filter2", 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
	MCFG_FILTER_RC_ADD("filter3", 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( tp84b, tp84 )
	MCFG_CPU_MODIFY("cpu1")
	MCFG_CPU_PROGRAM_MAP(tp84b_cpu1_map)
MACHINE_CONFIG_END

/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( tp84 )
	ROM_REGION( 0x10000, "cpu1", 0 )
	ROM_LOAD( "388_f04.7j",  0x8000, 0x2000, CRC(605f61c7) SHA1(6848ef35ec7f92cccefb0fb2de42c4b0e9ec476f) )
	ROM_LOAD( "388_05.8j",   0xa000, 0x2000, CRC(4b4629a4) SHA1(f3bb1ee66c9e47d050370ac9ca74f3020cb9cfa3) )
	ROM_LOAD( "388_f06.9j",  0xc000, 0x2000, CRC(dbd5333b) SHA1(65dee1fd4c940a5423d57cb55a7f2ad89c59c5c6) )
	ROM_LOAD( "388_07.10j",  0xe000, 0x2000, CRC(a45237c4) SHA1(896e31c59aedf1c7e73e6f30fbe78cc020b457ab) )

	ROM_REGION( 0x10000, "sub", 0 ) /* 64k for the second CPU */
	ROM_LOAD( "388_f08.10d", 0xe000, 0x2000, CRC(36462ff1) SHA1(118a1b46ee01a583e6cf39af59b073321c76dbff) ) /* E08? */

	ROM_REGION( 0x10000, "audiocpu", 0 )    /* 64k for code of sound cpu Z80 */
	ROM_LOAD( "388j13.6a",   0x0000, 0x2000, CRC(c44414da) SHA1(981289f5bdf7dc1348f4ca547ac933ef503b6588) )

	ROM_REGION( 0x4000, "gfx1", 0 )
	ROM_LOAD( "388_h02.2j",  0x0000, 0x2000, CRC(05c7508f) SHA1(1a3c7cd47ad34e37a7b0f3014e10c055cbb2b559) ) /* chars */
	ROM_LOAD( "388_d01.1j",  0x2000, 0x2000, CRC(498d90b7) SHA1(6975f3a1603b14132aab58329195a4845a6e28bb) )

	ROM_REGION( 0x8000, "gfx2", 0 )
	ROM_LOAD( "388_e09.12a", 0x0000, 0x2000, CRC(cd682f30) SHA1(6f48d3efc53d63171ec655e64b225412de1374e4) ) /* sprites */
	ROM_LOAD( "388_e10.13a", 0x2000, 0x2000, CRC(888d4bd6) SHA1(7e2dde080bb614709561431a81b0490b2aaa42a9) )
	ROM_LOAD( "388_e11.14a", 0x4000, 0x2000, CRC(9a220b39) SHA1(792aaa4daedc8eb807d5a66d87da4641739b1660) )
	ROM_LOAD( "388_e12.15a", 0x6000, 0x2000, CRC(fac98397) SHA1(d90f99b19ab3cddfdfd37a273fb437be098088bc) )

	ROM_REGION( 0x0500, "proms", 0 )
	ROM_LOAD( "388d14.2c",   0x0000, 0x0100, CRC(d737eaba) SHA1(e39026f87f5b995cf4a38b5d3d3fee7561762ae6) ) /* palette red component */
	ROM_LOAD( "388d15.2d",   0x0100, 0x0100, CRC(2f6a9a2a) SHA1(f09d8b92c7f9bf046cdd815c5282d0510e61b6e0) ) /* palette green component */
	ROM_LOAD( "388d16.1e",   0x0200, 0x0100, CRC(2e21329b) SHA1(9ba8af294dbd6f3a5d039c74a56e0605a913c037) ) /* palette blue component */
	ROM_LOAD( "388d18.1f",   0x0300, 0x0100, CRC(61d2d398) SHA1(3f74ad733b07b6a31cf9d4956d171eb9253dd6bf) ) /* char lookup table */
	ROM_LOAD( "388j17.16c", 0x0400, 0x0100, CRC(13c4e198) SHA1(42ab23206be99e840bd9c52cefa175c12fac8e5b) ) /* sprite lookup table */
ROM_END

ROM_START( tp84a )
	ROM_REGION( 0x10000, "cpu1", 0 )
	ROM_LOAD( "388_f04.7j",  0x8000, 0x2000, CRC(605f61c7) SHA1(6848ef35ec7f92cccefb0fb2de42c4b0e9ec476f) )
	ROM_LOAD( "388_f05.8j",  0xa000, 0x2000, CRC(e97d5093) SHA1(c76c119574d19d2ac10e6987150744542803ef5b) )
	ROM_LOAD( "388_f06.9j",  0xc000, 0x2000, CRC(dbd5333b) SHA1(65dee1fd4c940a5423d57cb55a7f2ad89c59c5c6) )
	ROM_LOAD( "388_f07.10j", 0xe000, 0x2000, CRC(8fbdb4ef) SHA1(e615c4d9964ab00f6776147c54925b4b6100b360) )

	ROM_REGION( 0x10000, "sub", 0 ) /* 64k for the second CPU */
	ROM_LOAD( "388_f08.10d", 0xe000, 0x2000, CRC(36462ff1) SHA1(118a1b46ee01a583e6cf39af59b073321c76dbff) ) /* E08? */

	ROM_REGION( 0x10000, "audiocpu", 0 )    /* 64k for code of sound cpu Z80 */
	ROM_LOAD( "388j13.6a",   0x0000, 0x2000, CRC(c44414da) SHA1(981289f5bdf7dc1348f4ca547ac933ef503b6588) )

	ROM_REGION( 0x4000, "gfx1", 0 )
	ROM_LOAD( "388_h02.2j",  0x0000, 0x2000, CRC(05c7508f) SHA1(1a3c7cd47ad34e37a7b0f3014e10c055cbb2b559) ) /* chars */
	ROM_LOAD( "388_d01.1j",  0x2000, 0x2000, CRC(498d90b7) SHA1(6975f3a1603b14132aab58329195a4845a6e28bb) )

	ROM_REGION( 0x8000, "gfx2", 0 )
	ROM_LOAD( "388_e09.12a", 0x0000, 0x2000, CRC(cd682f30) SHA1(6f48d3efc53d63171ec655e64b225412de1374e4) ) /* sprites */
	ROM_LOAD( "388_e10.13a", 0x2000, 0x2000, CRC(888d4bd6) SHA1(7e2dde080bb614709561431a81b0490b2aaa42a9) )
	ROM_LOAD( "388_e11.14a", 0x4000, 0x2000, CRC(9a220b39) SHA1(792aaa4daedc8eb807d5a66d87da4641739b1660) )
	ROM_LOAD( "388_e12.15a", 0x6000, 0x2000, CRC(fac98397) SHA1(d90f99b19ab3cddfdfd37a273fb437be098088bc) )

	ROM_REGION( 0x0500, "proms", 0 )
	ROM_LOAD( "388d14.2c",   0x0000, 0x0100, CRC(d737eaba) SHA1(e39026f87f5b995cf4a38b5d3d3fee7561762ae6) ) /* palette red component */
	ROM_LOAD( "388d15.2d",   0x0100, 0x0100, CRC(2f6a9a2a) SHA1(f09d8b92c7f9bf046cdd815c5282d0510e61b6e0) ) /* palette green component */
	ROM_LOAD( "388d16.1e",   0x0200, 0x0100, CRC(2e21329b) SHA1(9ba8af294dbd6f3a5d039c74a56e0605a913c037) ) /* palette blue component */
	ROM_LOAD( "388d18.1f",   0x0300, 0x0100, CRC(61d2d398) SHA1(3f74ad733b07b6a31cf9d4956d171eb9253dd6bf) ) /* char lookup table */
	ROM_LOAD( "388d17.16c",  0x0400, 0x0100, CRC(af8f839c) SHA1(b469785a4153a221403fcf72c65c9c35ae75df5d) ) /* sprite lookup table (dump miss?)*/
ROM_END

ROM_START( tp84b )
	ROM_REGION( 0x10000, "cpu1", 0 )
	/* 0x6000 - 0x7fff space for diagnostic rom */
	ROM_LOAD( "388j05.8j",   0x8000, 0x4000, CRC(a59e2fda) SHA1(7d776d5d3fcfbe81d42580cfe93614dc4618a440) )
	ROM_LOAD( "388j07.10j",  0xc000, 0x4000, CRC(d25d18e6) SHA1(043f515cc66f6af004be81d6a6b5a92b553107ff) )

	ROM_REGION( 0x10000, "sub", 0 ) /* 64k for the second CPU */
	ROM_LOAD( "388j08.10d", 0xe000, 0x2000, CRC(2aea6b42) SHA1(58c3b4852f22a766f440b98904b73c00a31eae01) )

	ROM_REGION( 0x10000, "audiocpu", 0 )    /* 64k for code of sound cpu Z80 */
	ROM_LOAD( "388j13.6a", 0x0000, 0x2000, CRC(c44414da) SHA1(981289f5bdf7dc1348f4ca547ac933ef503b6588) )

	ROM_REGION( 0x4000, "gfx1", 0 )
	ROM_LOAD( "388j02.2j",  0x0000, 0x4000, CRC(e1225f53) SHA1(59d07dc4faafc82999e9716f0bba1cb7350c03e3) ) /* chars */

	ROM_REGION( 0x8000, "gfx2", 0 )
	ROM_LOAD( "388j09.12a", 0x0000, 0x4000, CRC(aec90936) SHA1(3420c24bbedb140cb20fdaf51acbe9493830b64a) ) /* sprites */
	ROM_LOAD( "388j11.14a", 0x4000, 0x4000, CRC(29257f03) SHA1(ebbb980bd226e8ada7e517e92487a32bfbc82f91) )

	ROM_REGION( 0x0500, "proms", 0 )
	ROM_LOAD( "388j14.2c",  0x0000, 0x0100, CRC(d737eaba) SHA1(e39026f87f5b995cf4a38b5d3d3fee7561762ae6) ) /* palette red component */
	ROM_LOAD( "388j15.2d",  0x0100, 0x0100, CRC(2f6a9a2a) SHA1(f09d8b92c7f9bf046cdd815c5282d0510e61b6e0) ) /* palette green component */
	ROM_LOAD( "388j16.1e",  0x0200, 0x0100, CRC(2e21329b) SHA1(9ba8af294dbd6f3a5d039c74a56e0605a913c037) ) /* palette blue component */
	ROM_LOAD( "388j18.1f",  0x0300, 0x0100, CRC(61d2d398) SHA1(3f74ad733b07b6a31cf9d4956d171eb9253dd6bf) ) /* char lookup table */
	ROM_LOAD( "388j17.16c", 0x0400, 0x0100, CRC(13c4e198) SHA1(42ab23206be99e840bd9c52cefa175c12fac8e5b) ) /* sprite lookup table */
ROM_END


GAME( 1984, tp84,  0,    tp84,  tp84, driver_device, 0, ROT90, "Konami", "Time Pilot '84 (set 1)", MACHINE_SUPPORTS_SAVE )
GAME( 1984, tp84a, tp84, tp84,  tp84a, driver_device,0, ROT90, "Konami", "Time Pilot '84 (set 2)", MACHINE_SUPPORTS_SAVE )
GAME( 1984, tp84b, tp84, tp84b, tp84, driver_device, 0, ROT90, "Konami", "Time Pilot '84 (set 3)", MACHINE_SUPPORTS_SAVE )
