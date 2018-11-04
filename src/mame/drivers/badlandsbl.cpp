// license:BSD-3-Clause
// copyright-holders:Aaron Giles, Angelo Salese
/*************************************************************************************** 

 Badlands - Playmark Bootleg support

 TODO:
 - sprites (never copied to where they are intended?)
 
 Year: 1989
 Producer: Playmark

 cpu: 68000
 sound cpu:  Z80
 sound ics: YM2151 + 3012

 other ics: 28c16 2kx8 eeprom.Used to store bookeeping,settings etc. like original pcb.

 Osc: 20 Mhz, 28 Mhz

 ROMs:

 blb21, blb22, blb27, blb28 main program
 blb26 sound program
 blb29 to blb40 graphics

 All eproms are 27c512

 Note

 This romset comes from a bootleg pcb produced by Playmark.This pcb was been modified to use as control standard joysticks instead of steering wheels.Game differences are: Copyright string removed.

***************************************************************************************/

#include "emu.h"
#include "includes/badlands.h"

READ16_MEMBER(badlandsbl_state::badlandsb_unk_r)
{
	return 0xffff;
}

READ8_MEMBER(badlandsbl_state::bootleg_shared_r)
{
	return m_b_sharedram[offset];
}

WRITE8_MEMBER(badlandsbl_state::bootleg_shared_w)
{
	m_b_sharedram[offset] = data;
}

ADDRESS_MAP_START(badlandsbl_state::bootleg_map)
	AM_RANGE(0x000000, 0x03ffff) AM_ROM

	// only 0-0xff accessed, assume all range is shared
	AM_RANGE(0x400000, 0x401fff) AM_READWRITE8(bootleg_shared_r,bootleg_shared_w,0xffff)

	AM_RANGE(0xfc0000, 0xfc0001) AM_READ(badlandsb_unk_r ) // sound comms?

	AM_RANGE(0xfd0000, 0xfd1fff) AM_DEVREADWRITE8("eeprom", eeprom_parallel_28xx_device, read, write, 0x00ff)
	//AM_RANGE(0xfe0000, 0xfe1fff) AM_DEVWRITE("watchdog", watchdog_timer_device, reset_w)
	AM_RANGE(0xfe2000, 0xfe3fff) AM_WRITE(video_int_ack_w)

	AM_RANGE(0xfe0000, 0xfe0001) AM_WRITENOP
	AM_RANGE(0xfe4000, 0xfe4001) AM_READ_PORT("FE4000")
	AM_RANGE(0xfe4004, 0xfe4005) AM_READ_PORT("P1")
	AM_RANGE(0xfe4006, 0xfe4007) AM_READ_PORT("P2")
	AM_RANGE(0xfe4008, 0xfe4009) AM_WRITE(badlands_pf_bank_w)
	AM_RANGE(0xfe400c, 0xfe400d) AM_DEVWRITE("eeprom", eeprom_parallel_28xx_device, unlock_write16)

	AM_RANGE(0xffc000, 0xffc3ff) AM_DEVREADWRITE8("palette", palette_device, read8, write8, 0xff00) AM_SHARE("palette")
	AM_RANGE(0xffe000, 0xffefff) AM_RAM_DEVWRITE("playfield", tilemap_device, write16) AM_SHARE("playfield")
	// TODO: actually sprites are at 0xfff600-0x7ff ?
	AM_RANGE(0xfff000, 0xfff1ff) AM_RAM AM_SHARE("mob")
	AM_RANGE(0xfff200, 0xffffff) AM_RAM
ADDRESS_MAP_END

WRITE8_MEMBER(badlandsbl_state::bootleg_main_irq_w)
{
	m_maincpu->set_input_line(2, HOLD_LINE);
}

ADDRESS_MAP_START(badlandsbl_state::bootleg_audio_map)
	AM_RANGE(0x0000, 0x1fff) AM_ROM AM_REGION("audiorom", 0)
	AM_RANGE(0x2000, 0x3fff) AM_RAM AM_SHARE("b_sharedram")
	AM_RANGE(0x4000, 0xcfff) AM_ROM AM_REGION("audiorom", 0x4000)
	AM_RANGE(0xd400, 0xd400) AM_WRITE(bootleg_main_irq_w) // correct?
	AM_RANGE(0xd800, 0xd801) AM_DEVREADWRITE("ymsnd", ym2151_device, read, write)
ADDRESS_MAP_END


static INPUT_PORTS_START( badlandsb )
	PORT_INCLUDE( badlands )

	PORT_START("P1")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_2WAY PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_2WAY PORT_PLAYER(1)
	PORT_BIT( 0xfffc, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("P2")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_2WAY PORT_PLAYER(2)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_2WAY PORT_PLAYER(2)
	PORT_BIT( 0xfffc, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_MODIFY("FE6000")
	PORT_BIT( 0xffff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_MODIFY("FE6002")
	PORT_BIT( 0xffff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_MODIFY("AUDIO") /* audio port */
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )

INPUT_PORTS_END


static const gfx_layout pflayout_bootleg =
{
	8,8,
	RGN_FRAC(1,4),
	4,
	{ RGN_FRAC(0,4), RGN_FRAC(1,4), RGN_FRAC(2,4), RGN_FRAC(3,4) },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8
};

static const gfx_layout badlands_molayout =
{
	16,8,
	RGN_FRAC(1,1),
	4,
	{ 0, 1, 2, 3 },
	{ 0, 4, 8, 12, 16, 20, 24, 28, 32, 36, 40, 44, 48, 52, 56, 60 },
	{ 0*8, 8*8, 16*8, 24*8, 32*8, 40*8, 48*8, 56*8 },
	64*8
};

static GFXDECODE_START( badlandsb )
	GFXDECODE_ENTRY( "gfx1", 0, pflayout_bootleg,    0, 8 )
	GFXDECODE_ENTRY( "gfx2", 0, badlands_molayout,  128, 8 )
GFXDECODE_END


void badlandsbl_state::machine_reset()
{
//  m_pedal_value[0] = m_pedal_value[1] = 0x80;
	atarigen_state::machine_reset();
//  scanline_timer_reset(*m_screen, 32);

//  memcpy(m_bank_base, &m_bank_source_data[0x0000], 0x1000);
}

TIMER_DEVICE_CALLBACK_MEMBER(badlandsbl_state::bootleg_sound_scanline)
{
	int scanline = param;
	//address_space &space = m_audiocpu->space(AS_PROGRAM);

	// 32V
	if ((scanline % 64) == 0 && scanline < 240)
		m_audiocpu->set_input_line(0, HOLD_LINE);
}

MACHINE_CONFIG_START(badlandsbl_state::badlandsb)

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68000, XTAL(28'000'000)/4)   /* Divisor estimated */
	MCFG_CPU_PROGRAM_MAP(bootleg_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", badlandsbl_state,  irq1_line_hold) //vblank_int)

	MCFG_CPU_ADD("audiocpu", Z80, XTAL(20'000'000)/12)    /* Divisor estimated */
	MCFG_CPU_PROGRAM_MAP(bootleg_audio_map)
	MCFG_TIMER_DRIVER_ADD_SCANLINE("scantimer", badlandsbl_state, bootleg_sound_scanline, "screen", 0, 1)

	MCFG_MACHINE_START_OVERRIDE(badlands_state,badlands)

	MCFG_EEPROM_2816_ADD("eeprom")
	MCFG_EEPROM_28XX_LOCK_AFTER_WRITE(true)

	/* video hardware */
	MCFG_GFXDECODE_ADD("gfxdecode", "palette", badlandsb)
	MCFG_PALETTE_ADD("palette", 256)
	MCFG_PALETTE_FORMAT(IRRRRRGGGGGBBBBB)
	MCFG_PALETTE_MEMBITS(8)

	MCFG_TILEMAP_ADD_STANDARD("playfield", "gfxdecode", 2, badlands_state, get_playfield_tile_info, 8,8, SCAN_ROWS, 64,32)
	MCFG_ATARI_MOTION_OBJECTS_ADD("mob", "screen", badlands_state::s_mob_config)
	MCFG_ATARI_MOTION_OBJECTS_GFXDECODE("gfxdecode")

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_VIDEO_ATTRIBUTES(VIDEO_UPDATE_BEFORE_VBLANK)
	/* note: these parameters are from published specs, not derived */
	/* the board uses an SOS-2 chip to generate video signals */
	MCFG_SCREEN_RAW_PARAMS(ATARI_CLOCK_14MHz/2, 456, 0, 336, 262, 0, 240)
	MCFG_SCREEN_UPDATE_DRIVER(badlands_state, screen_update_badlands)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_VIDEO_START_OVERRIDE(badlands_state,badlands)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_YM2151_ADD("ymsnd", XTAL(20'000'000)/8)  /* Divisor estimated */
	MCFG_SOUND_ROUTE(0, "mono", 0.30)
	MCFG_SOUND_ROUTE(1, "mono", 0.30)
MACHINE_CONFIG_END



/* bootleg by Playmark, uses Joystick controls */
ROM_START( badlandsb )
	/* bootleg 68k Program */
	ROM_REGION( 0x40000, "maincpu", 0 ) /* 4*64k for 68000 code */
	ROM_LOAD16_BYTE( "blb28.ic21",  0x00000, 0x10000, CRC(dffb025d) SHA1(f2c17607acbbeee7d5d3f3dd2e8dc768b755e991) )
	ROM_LOAD16_BYTE( "blb22.ic22",  0x00001, 0x10000, CRC(ca3015c4) SHA1(72e1451498143d920239487704f4b4a8a71410e0) )
	ROM_LOAD16_BYTE( "blb27.ic19",  0x20000, 0x10000, CRC(0e2e807f) SHA1(5b61de066dca12c44335aa68a13c821845657866) )
	ROM_LOAD16_BYTE( "blb21.ic20",  0x20001, 0x10000, CRC(99a20c2c) SHA1(9b0a5a5dafb8816e72330d302c60339b600b49a8) )

	/* Z80 on the bootleg! */
	ROM_REGION( 0x10000, "audiorom", 0 )
	ROM_LOAD( "blb26.ic27", 0x00000, 0x10000, CRC(59503ab4) SHA1(ea5686ee28f6125c1394d687cc35c6322c8f900c) )

	/* the 2nd half of 122,123,124 and 125 is identical to the first half and not used */
	ROM_REGION( 0x80000, "gfx1", 0 )
	ROM_LOAD( "13.ic123",     0x000000, 0x10000, CRC(55fac198) SHA1(055938f38cb7fc02ecaf35446b2598f7808baa1c) ) // alt set replaced bad rom
	ROM_LOAD( "blb37.ic92",   0x008000, 0x10000, CRC(9188db9f) SHA1(8f7dc2c4c0dec9a80b6214a2efaa0de0858de84c) )
	ROM_LOAD( "blb38.ic125",  0x020000, 0x10000, CRC(4839dd54) SHA1(031efbc144e5e088be0f3576aa514c7c2b775f6d) )
	ROM_LOAD( "8.ic91",       0x028000, 0x10000, CRC(4d85a509) SHA1(f8f13fe16706d01f0ee87cd1188c3115d6cdbf46) ) // alt set replaced bad rom
	ROM_LOAD( "blb30.ic122",  0x040000, 0x10000, CRC(61a1bcec) SHA1(fd38bbf8f6c8d1e0e936740db757f9fa85753503) )
	ROM_LOAD( "blb35.ic90",   0x048000, 0x10000, CRC(649c17f0) SHA1(bed3b7fc2c0516fe309bb81b65d8925ecf3065e4) )
	ROM_LOAD( "blb32.ic124",  0x060000, 0x10000, CRC(a67c61ba) SHA1(d701eb7f4520b57be54a7113d39f81d52800ee7e) )
	ROM_LOAD( "blb29.ic88",   0x068000, 0x10000, CRC(a9f280e5) SHA1(daff021d14f17da8c4469270a1e50e5a01d05d49) )

	/* the 1st half of 67 & 68 are empty and not used */
	ROM_REGION( 0x40000, "gfx2", 0 )
	ROM_LOAD16_BYTE( "blb33.ic67", 0x10001, 0x10000, CRC(aebf9938) SHA1(3778aacbde07e5a5d010e41ab62d5b0db8632ad8) )
	ROM_LOAD16_BYTE( "blb34.ic34", 0x00001, 0x10000, CRC(3eac30a5) SHA1(deefc668185bf30ad3eeba73853f97ce12b85293) )
	ROM_LOAD16_BYTE( "blb39.ic68", 0x10000, 0x10000, CRC(f398f2d7) SHA1(1eef64680101888425490eb4d5b86072e59753cf) )
	ROM_LOAD16_BYTE( "blb40.ic35", 0x00000, 0x10000, CRC(b47679ee) SHA1(0bd7d40dad214c54021c2014efbd374a7e4c7a3f) )
ROM_END


ROM_START( badlandsb2 )
	/* bootleg 68k Program */
	ROM_REGION( 0x40000, "maincpu", 0 ) /* 4*64k for 68000 code */
	ROM_LOAD16_BYTE( "5.ic21",  0x00000, 0x10000, CRC(dffb025d) SHA1(f2c17607acbbeee7d5d3f3dd2e8dc768b755e991) )
	ROM_LOAD16_BYTE( "2.ic22",  0x00001, 0x10000, CRC(ca3015c4) SHA1(72e1451498143d920239487704f4b4a8a71410e0) )
	ROM_LOAD16_BYTE( "4.ic19",  0x20000, 0x10000, CRC(0e2e807f) SHA1(5b61de066dca12c44335aa68a13c821845657866) )
	ROM_LOAD16_BYTE( "1.ic20",  0x20001, 0x10000, CRC(99a20c2c) SHA1(9b0a5a5dafb8816e72330d302c60339b600b49a8) )

	/* Z80 on the bootleg! */
	ROM_REGION( 0x10000, "audiorom", 0 )
	ROM_LOAD( "3.ic27", 0x00000, 0x10000, CRC(08850eb5) SHA1(be169e8ccee275b72bcfca66cd126cc27af7a1d6) )  // only rom that differs from badlandsb

	/* the 2nd half of 122,123,124 and 125 is identical to the first half and not used */
	ROM_REGION( 0x80000, "gfx1", 0 )
	ROM_LOAD( "13.ic123",  0x000000, 0x10000, CRC(55fac198) SHA1(055938f38cb7fc02ecaf35446b2598f7808baa1c) )
	ROM_LOAD( "14.ic92",   0x008000, 0x10000, CRC(9188db9f) SHA1(8f7dc2c4c0dec9a80b6214a2efaa0de0858de84c) )
	ROM_LOAD( "15.ic125",  0x020000, 0x10000, CRC(4839dd54) SHA1(031efbc144e5e088be0f3576aa514c7c2b775f6d) )
	ROM_LOAD( "8.ic91",    0x028000, 0x10000, CRC(4d85a509) SHA1(f8f13fe16706d01f0ee87cd1188c3115d6cdbf46) )
	ROM_LOAD( "7.ic122",   0x040000, 0x10000, CRC(61a1bcec) SHA1(fd38bbf8f6c8d1e0e936740db757f9fa85753503) )
	ROM_LOAD( "12.ic90",   0x048000, 0x10000, CRC(649c17f0) SHA1(bed3b7fc2c0516fe309bb81b65d8925ecf3065e4) )
	ROM_LOAD( "9.ic124",   0x060000, 0x10000, CRC(a67c61ba) SHA1(d701eb7f4520b57be54a7113d39f81d52800ee7e) )
	ROM_LOAD( "6.ic88",    0x068000, 0x10000, CRC(a9f280e5) SHA1(daff021d14f17da8c4469270a1e50e5a01d05d49) )

	/* the 1st half of 67 & 68 are empty and not used */
	ROM_REGION( 0x40000, "gfx2", 0 )
	ROM_LOAD16_BYTE( "10.ic67", 0x10001, 0x10000, CRC(aebf9938) SHA1(3778aacbde07e5a5d010e41ab62d5b0db8632ad8) )
	ROM_LOAD16_BYTE( "11.ic34", 0x00001, 0x10000, CRC(3eac30a5) SHA1(deefc668185bf30ad3eeba73853f97ce12b85293) )
	ROM_LOAD16_BYTE( "16.ic68", 0x10000, 0x10000, CRC(f398f2d7) SHA1(1eef64680101888425490eb4d5b86072e59753cf) )
	ROM_LOAD16_BYTE( "17.ic35", 0x00000, 0x10000, CRC(b47679ee) SHA1(0bd7d40dad214c54021c2014efbd374a7e4c7a3f) )
ROM_END



GAME( 1989, badlandsb, badlands, badlandsb, badlandsb, badlandsbl_state, 0, ROT0, "bootleg (Playmark)", "Bad Lands (bootleg)", MACHINE_NOT_WORKING )
GAME( 1989, badlandsb2,badlands, badlandsb, badlandsb, badlandsbl_state, 0, ROT0, "bootleg (Playmark)", "Bad Lands (bootleg, alternate)", MACHINE_NOT_WORKING )
