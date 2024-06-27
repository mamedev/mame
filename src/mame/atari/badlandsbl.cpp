// license:BSD-3-Clause
// copyright-holders:Aaron Giles, Angelo Salese
/***************************************************************************************

 Badlands - Playmark Bootleg support

 TODO:
 - inputs looks weird, left/right change meaning as per what side the car is moving?
 - playfield color (changes bitplane on the fly?);
 - sprite-tilemap priority;
 - fix sound;
 - inputs (coins & accelerators);
 - any way to disable sprites in service mode?

========================================================================================

 Year: 1989
 Producer: Playmark

 cpu: 68000
 sound cpu:  Z80
 sound ics: YM2151 + 3012

 other ics: 28c16 2kx8 eeprom. Used to store book-keeping, settings etc. like original pcb.

 Osc: 20 Mhz, 28 Mhz

 ROMs:

 blb21, blb22, blb27, blb28 main program
 blb26 sound program
 blb29 to blb40 graphics

 All eproms are 27c512

 Note

 This romset comes from a bootleg pcb produced by Playmark. This pcb was been modified to use as control standard joysticks instead of steering wheels. Game differences are: Copyright string removed.

***************************************************************************************/

#include "emu.h"
#include "badlands.h"
#include "emupal.h"

uint32_t badlandsbl_state::screen_update_badlandsbl(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	// start drawing
	//m_mob->draw_async(cliprect);

	// draw the playfield
	m_playfield_tilemap->draw(screen, bitmap, cliprect, 0, 0);

	// draw bootleg sprites chip
	// TODO: is this derived from something else?
	{
		gfx_element *gfx = m_gfxdecode->gfx(1);

		for(int count = 0; count < (0x100-0x10) / 2; count+=4)
		{
			if((m_spriteram[count + 3] & 0xff) == 0xff)
				return 0;

			const u16 tile = m_spriteram[count];
			const int y = (511 - 14) - (m_spriteram[count+1] & 0x1ff);
			const int x = (m_spriteram[count+2] >> 7) - 7;
			const u8 color = (m_spriteram[count+3] >> 8) & 7;
			const u8 height = (m_spriteram[count+3] & 0xf) + 1;

			for(int yi = 0; yi < height; yi++)
				gfx->transpen(bitmap, cliprect, tile + yi, color, 0, 0, x, y + yi*8, 0);
		}
	}

	return 0;
}


uint16_t badlandsbl_state::badlandsb_unk_r()
{
	return 0xffff;
}

// TODO: this probably mimics audio_io_r/_w in original version
uint8_t badlandsbl_state::bootleg_shared_r(offs_t offset)
{
	return m_b_sharedram[offset];
}

void badlandsbl_state::bootleg_shared_w(offs_t offset, uint8_t data)
{
	m_b_sharedram[offset] = data;
}

uint8_t badlandsbl_state::sound_response_r()
{
	m_maincpu->set_input_line(2, CLEAR_LINE);
	return m_sound_response;
}

void badlandsbl_state::bootleg_map(address_map &map)
{
	map(0x000000, 0x03ffff).rom();

	map(0x400000, 0x400005).rw(FUNC(badlandsbl_state::bootleg_shared_r), FUNC(badlandsbl_state::bootleg_shared_w));
	map(0x400006, 0x400006).r(FUNC(badlandsbl_state::sound_response_r));
	map(0x400008, 0x40000f).ram(); // breaks tilemap gfxs otherwise?
	map(0x400010, 0x4000ff).ram().share("spriteram");

	// sound comms?
	map(0xfc0000, 0xfc0001).r(FUNC(badlandsbl_state::badlandsb_unk_r)).nopw();

	map(0xfd0000, 0xfd1fff).rw("eeprom", FUNC(eeprom_parallel_28xx_device::read), FUNC(eeprom_parallel_28xx_device::write)).umask16(0x00ff);
	//map(0xfe0000, 0xfe1fff).w("watchdog", FUNC(watchdog_timer_device::reset_w));
	map(0xfe2000, 0xfe3fff).w(FUNC(badlandsbl_state::video_int_ack_w));

	map(0xfe0000, 0xfe0001).nopw();
	map(0xfe4000, 0xfe4001).portr("FE4000");
	map(0xfe4004, 0xfe4005).portr("P1");
	map(0xfe4006, 0xfe4007).portr("P2");
	map(0xfe4008, 0xfe4009).w(FUNC(badlandsbl_state::badlands_pf_bank_w));
	map(0xfe400c, 0xfe400d).w("eeprom", FUNC(eeprom_parallel_28xx_device::unlock_write16));

	map(0xffc000, 0xffc3ff).rw("palette", FUNC(palette_device::read8), FUNC(palette_device::write8)).umask16(0xff00).share("palette");
	map(0xffe000, 0xffefff).ram().w("playfield", FUNC(tilemap_device::write16)).share("playfield");

	map(0xfff000, 0xffffff).ram();
}

void badlandsbl_state::bootleg_main_irq_w(uint8_t data)
{
	m_maincpu->set_input_line(2, ASSERT_LINE);
	m_sound_response = data;
}

void badlandsbl_state::bootleg_audio_map(address_map &map)
{
	map(0x0000, 0x1fff).rom().region("audiorom", 0);
	map(0x2000, 0x2005).ram().share("b_sharedram");
	map(0x2006, 0x3fff).ram();
	map(0x4000, 0xcfff).rom().region("audiorom", 0x4000);
	map(0xd400, 0xd400).w(FUNC(badlandsbl_state::bootleg_main_irq_w));
	map(0xd800, 0xd801).rw("ymsnd", FUNC(ym2151_device::read), FUNC(ym2151_device::write));
	map(0xe000, 0xffff).noprw(); // either RAM mirror or left-over
}


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
	RGN_FRAC(1, 4),
	4,
	{ RGN_FRAC(0, 4), RGN_FRAC(1, 4), RGN_FRAC(2, 4), RGN_FRAC(3, 4) },
//  TODO: service mode and (most of) in-game uses this arrangement
//  { RGN_FRAC(1,4), RGN_FRAC(3,4), RGN_FRAC(0,4), RGN_FRAC(2,4) },
	{ STEP8(0, 1) },
	{ STEP8(0, 8) },
	8*8
};

static const gfx_layout badlands_molayout =
{
	16,8,
	RGN_FRAC(1,1),
	4,
	{ 0, 1, 2, 3 },
	{ STEP16(0, 4) },
	{ STEP8(0, 8 * 8) },
	64*8
};

static GFXDECODE_START( gfx_badlandsb )
	GFXDECODE_ENTRY( "gfx1", 0, pflayout_bootleg,    0, 8 )
	GFXDECODE_ENTRY( "gfx2", 0, badlands_molayout,  128, 8 )
GFXDECODE_END


void badlandsbl_state::machine_reset()
{
//  m_pedal_value[0] = m_pedal_value[1] = 0x80;
//  scanline_timer_reset(*m_screen, 32);

//  memcpy(m_bank_base, &m_bank_source_data[0x0000], 0x1000);
}

TIMER_DEVICE_CALLBACK_MEMBER(badlandsbl_state::bootleg_sound_scanline)
{
	int scanline = param;

	// 32V
	if ((scanline % 64) == 0 && scanline < 240)
		m_audiocpu->set_input_line(0, HOLD_LINE);
}

void badlandsbl_state::badlandsb(machine_config &config)
{
	/* basic machine hardware */
	M68000(config, m_maincpu, XTAL(28'000'000) / 4);   // Divisor guessed
	m_maincpu->set_addrmap(AS_PROGRAM, &badlandsbl_state::bootleg_map);
	m_maincpu->set_vblank_int("screen", FUNC(badlandsbl_state::irq1_line_hold)); //vblank_int)

	Z80(config, m_audiocpu, XTAL(20'000'000) / 12);    // Divisor guessed
	m_audiocpu->set_addrmap(AS_PROGRAM, &badlandsbl_state::bootleg_audio_map);
	TIMER(config, "scantimer").configure_scanline(FUNC(badlandsbl_state::bootleg_sound_scanline), "screen", 0, 1);

//  config.m_perfect_cpu_quantum = subtag("maincpu");

	EEPROM_2816(config, "eeprom").lock_after_write(true);

	/* video hardware */
	GFXDECODE(config, m_gfxdecode, "palette", gfx_badlandsb);
	palette_device &palette(PALETTE(config, "palette"));
	palette.set_format(palette_device::IRGB_1555, 256);
	palette.set_membits(8);

	TILEMAP(config, m_playfield_tilemap, m_gfxdecode, 2, 8,8, TILEMAP_SCAN_ROWS, 64,32).set_info_callback(FUNC(badlandsbl_state::get_playfield_tile_info));

//  ATARI_MOTION_OBJECTS(config, m_mob, 0, m_screen, badlands_state::s_mob_config);
//  m_mob->set_gfxdecode(m_gfxdecode);

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_video_attributes(VIDEO_UPDATE_BEFORE_VBLANK);
	/* note: these parameters are from published specs, not derived */
	/* the board uses an SOS-2 chip to generate video signals */
	m_screen->set_raw(14.318181_MHz_XTAL/2, 456, 0, 336, 262, 0, 240);
	m_screen->set_screen_update(FUNC(badlandsbl_state::screen_update_badlandsbl));
	m_screen->set_palette("palette");

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	YM2151(config, "ymsnd", XTAL(20'000'000) / 8).add_route(0, "mono", 0.30).add_route(1, "mono", 0.30); // Divisor guessed
}



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



GAME( 1989, badlandsb,  badlands, badlandsb, badlandsb, badlandsbl_state, empty_init, ROT0, "bootleg (Playmark)", "Bad Lands (bootleg)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND )
GAME( 1989, badlandsb2, badlands, badlandsb, badlandsb, badlandsbl_state, empty_init, ROT0, "bootleg (Playmark)", "Bad Lands (bootleg, alternate)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND )
