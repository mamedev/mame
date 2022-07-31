// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    Atari "Round" hardware

    driver by Aaron Giles

    Games supported:
        * Relief Pitcher (1990) [3 sets]

    Known bugs:
        * none at this time

From Atari Games Bulletin:

                              ATARI
                              GAMES
                            BULLENTIN
                      From Atari field Service

Attention Service Managers                    June 4th 1992

               Relief Pitcher Software updates

The codes on two EPROMS have been updated, the part numbers
and locations are 13693-0011 Rev D location 19E and
136093-0012 Rev D location 19J

The main features of the NEW program are:

REDUCE GAME TIME
If you are experiencing long game time on your Relief Pitcher
game these two new Rev D EPROMS will give you more difficulty
settings to help eliminate that problem.

Eight new difficulty settings have been added to the Rev D
EPROMS, with the default set to Hard.

RESET PROBLEM:
There is a possibility the game my HANG UP and RESET if
there is a problem with the base running or when the fielder
catches a flyball.

MISCELLANEOUS.
When set to NO, AUDIO IN THE ATTRACT will be completely
disabled

In the two player game, the players were given a free second
inning - this has been eliminated.

Fix the various cosmetic problems in the newspaper and audio.

Eprom updates will be available upon request through you
Atari Distributor

If you have any questions please call your
Atari Distributor or Atari Games Customer Service


ROM labels are in this format:

  RELIEF PITCHER             RELIEF PITCHER
 (c) 1992 ATARI       or    (c) 1992 ATARI
   136093-0011                136093-0011D
     CS ACFF REV D              CS ACFF

 ROM number   Checksum    Revision
----------------------------------
136093-0011     ACFF      Rev. D
136093-0012     E5FE

136093-0011     BAFF      Rev. C
136093-0012     8AFE

136093-0011     23FF      Rev. B
136093-0012     3CFE

136093-0013     CBBF
136093-0014     3EBE

****************************************************************************

    Memory map (TBA)

***************************************************************************/


#include "emu.h"
#include "relief.h"

#include "cpu/m68000/m68000.h"
#include "machine/eeprompar.h"
#include "machine/watchdog.h"
#include "sound/okim6295.h"
#include "sound/ymopl.h"
#include "emupal.h"
#include "speaker.h"


/*************************************
 *
 *  Initialization
 *
 *************************************/

void relief_state::machine_reset()
{
	m_adpcm_bank = 0;
	m_okibank->set_entry(m_adpcm_bank);
	m_ym2413_volume = 15;
	m_overall_volume = 127;
}



/*************************************
 *
 *  I/O handling
 *
 *************************************/

uint16_t relief_state::special_port2_r()
{
	int result = ioport("260010")->read();
	if (!(result & 0x0080) || m_screen->hblank()) result ^= 0x0001;
	return result;
}



/*************************************
 *
 *  Audio control I/O
 *
 *************************************/

void relief_state::audio_control_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	if (ACCESSING_BITS_0_7)
	{
		m_ym2413_volume = (data >> 1) & 15;
		m_ym2413->set_output_gain(ALL_OUTPUTS, (m_ym2413_volume * m_overall_volume) / (127.0f * 15.0f));
		m_adpcm_bank = ((data >> 6) & 3) | (m_adpcm_bank & 4);
	}
	if (ACCESSING_BITS_8_15)
		m_adpcm_bank = (((data >> 8) & 1)<<2) | (m_adpcm_bank & 3);

	m_okibank->set_entry(m_adpcm_bank);
}


void relief_state::audio_volume_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	if (ACCESSING_BITS_0_7)
	{
		m_overall_volume = data & 127;
		m_ym2413->set_output_gain(ALL_OUTPUTS, (m_ym2413_volume * m_overall_volume) / (127.0f * 15.0f));
		m_oki->set_output_gain(ALL_OUTPUTS, m_overall_volume / 127.0f);
	}
}

void relief_state::oki_map(address_map &map)
{
	map(0x00000, 0x1ffff).bankr("okibank");
	map(0x20000, 0x3ffff).rom();
}


/*************************************
 *
 *  Main CPU memory handlers
 *
 *************************************/

void relief_state::main_map(address_map &map)
{
	map.unmap_value_high();
	map.global_mask(0x3fffff);
	map(0x000000, 0x07ffff).rom();
	map(0x140000, 0x140003).w(m_ym2413, FUNC(ym2413_device::write)).umask16(0x00ff);
	map(0x140011, 0x140011).rw(m_oki, FUNC(okim6295_device::read), FUNC(okim6295_device::write));
	map(0x140020, 0x140021).w(FUNC(relief_state::audio_volume_w));
	map(0x140030, 0x140031).w(FUNC(relief_state::audio_control_w));
	map(0x180000, 0x180fff).rw("eeprom", FUNC(eeprom_parallel_28xx_device::read), FUNC(eeprom_parallel_28xx_device::write)).umask16(0xff00);
	map(0x1c0030, 0x1c0031).w("eeprom", FUNC(eeprom_parallel_28xx_device::unlock_write16));
	map(0x260000, 0x260001).portr("260000");
	map(0x260002, 0x260003).portr("260002");
	map(0x260010, 0x260011).r(FUNC(relief_state::special_port2_r));
	map(0x260012, 0x260013).portr("260012");
	map(0x2a0000, 0x2a0001).w("watchdog", FUNC(watchdog_timer_device::reset16_w));
	map(0x3e0000, 0x3e0fff).ram().w("palette", FUNC(palette_device::write16)).share("palette");
	map(0x3effc0, 0x3effff).rw(m_vad, FUNC(atari_vad_device::control_read), FUNC(atari_vad_device::control_write));
	map(0x3f0000, 0x3f1fff).ram().w(m_vad, FUNC(atari_vad_device::playfield2_latched_msb_w)).share("vad:playfield2");
	map(0x3f2000, 0x3f3fff).ram().w(m_vad, FUNC(atari_vad_device::playfield_latched_lsb_w)).share("vad:playfield");
	map(0x3f4000, 0x3f5fff).ram().w(m_vad, FUNC(atari_vad_device::playfield_upper_w)).share("vad:playfield_ext");
	map(0x3f6000, 0x3f67ff).ram().share("vad:mob");
	map(0x3f6800, 0x3f8eff).ram();
	map(0x3f8f00, 0x3f8f7f).ram().share("vad:eof");
	map(0x3f8f80, 0x3f8fff).share("vad:mob:slip");
	map(0x3f9000, 0x3fffff).ram();
}



/*************************************
 *
 *  Port definitions
 *
 *************************************/

static INPUT_PORTS_START( relief )
	PORT_START("260000")    /* 260000 */
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("Button D0") PORT_CODE(KEYCODE_Z)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("Button D1") PORT_CODE(KEYCODE_X)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("Button D2") PORT_CODE(KEYCODE_C)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("Button D3") PORT_CODE(KEYCODE_V)
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )

	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1)
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(1)

	PORT_START("260002")    /* 260002 */
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("White") PORT_CODE(KEYCODE_COMMA)
	PORT_DIPNAME( 0x0002, 0x0002, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0004, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0008, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("Yellow") PORT_CODE(KEYCODE_B)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("Blue") PORT_CODE(KEYCODE_N)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("Red") PORT_CODE(KEYCODE_M)

	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2)
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(2)

	PORT_START("260010")    /* 260010 */
	PORT_BIT( 0x001f, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_SERVICE( 0x0040, IP_ACTIVE_LOW )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_VBLANK("screen")
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("260012")    /* 260012 */
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x000c, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0xffe0, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END



/*************************************
 *
 *  Graphics definitions
 *
 *************************************/

static const gfx_layout pflayout =
{
	8,8,
	RGN_FRAC(1,5),
	4,
	{ RGN_FRAC(3,5), RGN_FRAC(2,5), RGN_FRAC(1,5), RGN_FRAC(0,5) },
	{ STEP8(0,1) },
	{ STEP8(0,16) },
	16*8
};


static const gfx_layout molayout =
{
	8,8,
	RGN_FRAC(1,5),
	5,
	{ RGN_FRAC(4,5), RGN_FRAC(3,5), RGN_FRAC(2,5), RGN_FRAC(1,5), RGN_FRAC(0,5) },
	{ STEP8(0,1) },
	{ STEP8(0,16) },
	16*8
};


static GFXDECODE_START( gfx_relief )
	GFXDECODE_ENTRY( "gfx1", 0, pflayout,   0, 64 )     /* alpha & playfield */
	GFXDECODE_ENTRY( "gfx1", 1, molayout, 256, 16 )     /* sprites */
GFXDECODE_END



/*************************************
 *
 *  Machine driver
 *
 *************************************/

void relief_state::relief(machine_config &config)
{
	/* basic machine hardware */
	M68000(config, m_maincpu, 14.318181_MHz_XTAL/2);
	m_maincpu->set_addrmap(AS_PROGRAM, &relief_state::main_map);

	EEPROM_2816(config, "eeprom").lock_after_write(true);

	WATCHDOG_TIMER(config, "watchdog");

	/* video hardware */
	GFXDECODE(config, m_gfxdecode, "palette", gfx_relief);
	PALETTE(config, "palette").set_format(palette_device::IRGB_1555, 2048);

	ATARI_VAD(config, m_vad, 0, m_screen);
	m_vad->scanline_int_cb().set_inputline(m_maincpu, M68K_IRQ_4);
	TILEMAP(config, "vad:playfield", m_gfxdecode, 2, 8, 8, TILEMAP_SCAN_COLS, 64, 64).set_info_callback(FUNC(relief_state::get_playfield_tile_info));
	TILEMAP(config, "vad:playfield2", m_gfxdecode, 2, 8, 8, TILEMAP_SCAN_COLS, 64, 64, 0).set_info_callback(FUNC(relief_state::get_playfield2_tile_info));
	ATARI_MOTION_OBJECTS(config, "vad:mob", 0, m_screen, relief_state::s_mob_config).set_gfxdecode(m_gfxdecode);

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_video_attributes(VIDEO_UPDATE_BEFORE_VBLANK);
	/* note: these parameters are from published specs, not derived */
	/* the board uses a VAD chip to generate video signals */
	m_screen->set_raw(14.318181_MHz_XTAL/2, 456, 0, 336, 262, 0, 240);
	m_screen->set_screen_update(FUNC(relief_state::screen_update_relief));
	m_screen->set_palette("palette");

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	OKIM6295(config, m_oki, 14.318181_MHz_XTAL/4/3, okim6295_device::PIN7_LOW);
	m_oki->add_route(ALL_OUTPUTS, "mono", 0.50);
	m_oki->set_addrmap(0, &relief_state::oki_map);

	YM2413(config, m_ym2413, 14.318181_MHz_XTAL/4).add_route(ALL_OUTPUTS, "mono", 1.0);
}



/*************************************
 *
 *  ROM definition(s)
 *
 *************************************/

// OS:   28 MAY 1992 14:00:14
// MAIN: 07 JUN 1992 20:12:30
ROM_START( relief )
	ROM_REGION( 0x80000, "maincpu", 0 ) /* 8*64k for 68000 code */
	ROM_LOAD16_BYTE( "136093-0011d_cs_acff.19e", 0x00000, 0x20000, CRC(cb3f73ad) SHA1(533a96095e678b4a414d6d9b861b1d4010ced30f) ) // checksum == ACFF
	ROM_LOAD16_BYTE( "136093-0012d_cs_e5fe.19j", 0x00001, 0x20000, CRC(90655721) SHA1(f50a2f317215a864d09e33a4acd927b873350425) ) // checksum == E5FE
	ROM_LOAD16_BYTE( "136093-0013_cs_cbbf.17e",  0x40000, 0x20000, CRC(1e1e82e5) SHA1(d33c84ae950db9775f9db9bf953aa63188d3f2f9) )
	ROM_LOAD16_BYTE( "136093-0014_cs_3ebe.17j",  0x40001, 0x20000, CRC(19e5decd) SHA1(8d93d93f966df46d59cf9f4cdaa689e4dcd2689a) )

	ROM_REGION( 0x280000, "gfx1", ROMREGION_INVERT )
	ROM_LOAD( "136093-0025a.14s",       0x000000, 0x80000, CRC(1b9e5ef2) SHA1(d7d14e75ca2d56c5c67154506096570c9ccbcf8e) )
	ROM_LOAD( "136093-0026a.8d",        0x080000, 0x80000, CRC(09b25d93) SHA1(94d424b21410182b5121201066f4acfa415f4b6b) )
	ROM_LOAD( "136093-0027a.18s",       0x100000, 0x80000, CRC(5bc1c37b) SHA1(89f1bca55dd431ca3171b89347209decf0b25e12) )
	ROM_LOAD( "136093-0028a.10d",       0x180000, 0x80000, CRC(55fb9111) SHA1(a95508f0831842fa79ca2fc168cfadc8c6d3fbd4) )
	ROM_LOAD16_BYTE( "136093-0029a.4d", 0x200001, 0x40000, CRC(e4593ff4) SHA1(7360ec7a65aabc90aa787dc30f39992e342495dd) )

	ROM_REGION( 0x100000, "oki", 0 )    /* 2MB for ADPCM data */
	ROM_LOAD( "136093-0030a.9b",  0x000000, 0x80000, CRC(f4c567f5) SHA1(7e8c1d54d918b0b41625eacbaf6dcb5bd99d1949) )
	ROM_LOAD( "136093-0031a.10b", 0x080000, 0x80000, CRC(ba908d73) SHA1(a83afd86f4c39394cf624b728a87b8d8b6de1944) )

	ROM_REGION( 0x800, "eeprom", 0 )
	ROM_LOAD( "relief_rev_d-eeprom.bin", 0x0000, 0x800, CRC(66069f60) SHA1(fac3797888f7ffe972f642aca44c6ca7d208c814) )

	ROM_REGION( 0x001000, "plds", 0 )
	ROM_LOAD( "gal16v8a-136093-0002.15f", 0x0000, 0x0117, CRC(b111d5f2) SHA1(0fe5b4ca786e839e6927a485109d33fe31c737a2) )
	ROM_LOAD( "gal16v8a-136093-0003.11r", 0x0200, 0x0117, CRC(67165ed2) SHA1(c7d9b9c45dd34e588c3db4e23c9c562334d2172a) )
	ROM_LOAD( "gal16v8a-136093-0004.5n",  0x0400, 0x0117, CRC(047b384a) SHA1(d268a65cf2d0fc0cfc7dd6b5c7a77572337c1707) )
	ROM_LOAD( "gal16v8a-136093-0005.3f",  0x0600, 0x0117, CRC(f76825b7) SHA1(2cd9cbb2564e5005a833403ae73f1a964dcb66fa) )
	ROM_LOAD( "gal16v8a-136093-0006.5f",  0x0800, 0x0117, CRC(c580d2a9) SHA1(b070fa53ec083fbeda8cd592bfbbd1e029b4dbe7) )
	ROM_LOAD( "gal16v8a-136093-0008.3e",  0x0a00, 0x0117, CRC(0117910e) SHA1(c8baecd24af201ad51cf13bf46c5cc0e7f7f2a94) )
	ROM_LOAD( "gal16v8a-136093-0009.8a",  0x0c00, 0x0117, CRC(b8679030) SHA1(09ab39c9c293381bbc082780f00d112c71e9c889) )
	ROM_LOAD( "gal16v8a-136093-0010.15a", 0x0e00, 0x0117, CRC(5f49c736) SHA1(91ff18e4780ee6c904735fc0f0e73ffb5a80b49a) )
ROM_END


// OS:   08 APR 1992 09:09:02
// MAIN: 26 APR 1992 21:18:13
ROM_START( relief2 )
	ROM_REGION( 0x80000, "maincpu", 0 ) /* 8*64k for 68000 code */
	ROM_LOAD16_BYTE( "136093-0011c_cs_baff.19e", 0x00000, 0x20000, CRC(41373e02) SHA1(1982be3d2b959f3504cd7e4afacd96bbebc27b8e) ) // checksum == BAFF
	ROM_LOAD16_BYTE( "136093-0012c_cs_8afe.19j", 0x00001, 0x20000, CRC(8187b026) SHA1(1408b5482194161c1fbb30911bb5b64a14b8ffb0) ) // checksum == 8AFE
	ROM_LOAD16_BYTE( "136093-0013_cs_cbbf.17e",  0x40000, 0x20000, CRC(1e1e82e5) SHA1(d33c84ae950db9775f9db9bf953aa63188d3f2f9) )
	ROM_LOAD16_BYTE( "136093-0014_cs_3ebe.17j",  0x40001, 0x20000, CRC(19e5decd) SHA1(8d93d93f966df46d59cf9f4cdaa689e4dcd2689a) )

	ROM_REGION( 0x280000, "gfx1", ROMREGION_INVERT )
	ROM_LOAD( "136093-0025a.14s",      0x000000, 0x80000, CRC(1b9e5ef2) SHA1(d7d14e75ca2d56c5c67154506096570c9ccbcf8e) )
	ROM_LOAD( "136093-0026a.8d",       0x080000, 0x80000, CRC(09b25d93) SHA1(94d424b21410182b5121201066f4acfa415f4b6b) )
	ROM_LOAD( "136093-0027a.18s",      0x100000, 0x80000, CRC(5bc1c37b) SHA1(89f1bca55dd431ca3171b89347209decf0b25e12) )
	ROM_LOAD( "136093-0028a.10d",      0x180000, 0x80000, CRC(55fb9111) SHA1(a95508f0831842fa79ca2fc168cfadc8c6d3fbd4) )
	ROM_LOAD16_BYTE( "136093-0029.4d", 0x200001, 0x40000, CRC(e4593ff4) SHA1(7360ec7a65aabc90aa787dc30f39992e342495dd) )

	ROM_REGION( 0x100000, "oki", 0 )    /* 2MB for ADPCM data */
	ROM_LOAD( "136093-0030a.9b",  0x000000, 0x80000, CRC(f4c567f5) SHA1(7e8c1d54d918b0b41625eacbaf6dcb5bd99d1949) )
	ROM_LOAD( "136093-0031a.10b", 0x080000, 0x80000, CRC(ba908d73) SHA1(a83afd86f4c39394cf624b728a87b8d8b6de1944) )

	ROM_REGION( 0x800, "eeprom", 0 )
	ROM_LOAD( "relief-eeprom.bin", 0x0000, 0x800, CRC(2131fc40) SHA1(72a9f5f6647fbc74e645b6639db2fdbfbe6456e2) )

	ROM_REGION( 0x001000, "plds", 0 )
	ROM_LOAD( "gal16v8a-136093-0002.15f", 0x0000, 0x0117, CRC(b111d5f2) SHA1(0fe5b4ca786e839e6927a485109d33fe31c737a2) )
	ROM_LOAD( "gal16v8a-136093-0003.11r", 0x0200, 0x0117, CRC(67165ed2) SHA1(c7d9b9c45dd34e588c3db4e23c9c562334d2172a) )
	ROM_LOAD( "gal16v8a-136093-0004.5n",  0x0400, 0x0117, CRC(047b384a) SHA1(d268a65cf2d0fc0cfc7dd6b5c7a77572337c1707) )
	ROM_LOAD( "gal16v8a-136093-0005.3f",  0x0600, 0x0117, CRC(f76825b7) SHA1(2cd9cbb2564e5005a833403ae73f1a964dcb66fa) )
	ROM_LOAD( "gal16v8a-136093-0006.5f",  0x0800, 0x0117, CRC(c580d2a9) SHA1(b070fa53ec083fbeda8cd592bfbbd1e029b4dbe7) )
	ROM_LOAD( "gal16v8a-136093-0008.3e",  0x0a00, 0x0117, CRC(0117910e) SHA1(c8baecd24af201ad51cf13bf46c5cc0e7f7f2a94) )
	ROM_LOAD( "gal16v8a-136093-0009.8a",  0x0c00, 0x0117, CRC(b8679030) SHA1(09ab39c9c293381bbc082780f00d112c71e9c889) )
	ROM_LOAD( "gal16v8a-136093-0010.15a", 0x0e00, 0x0117, CRC(5f49c736) SHA1(91ff18e4780ee6c904735fc0f0e73ffb5a80b49a) )
ROM_END

// OS:   08 APR 1992 09:09:02
// MAIN: 10 APR 1992 09:50:05
ROM_START( relief3 )
	ROM_REGION( 0x80000, "maincpu", 0 ) /* 8*64k for 68000 code */
	ROM_LOAD16_BYTE( "136093-0011b_cs_23ff.19e", 0x00000, 0x20000, CRC(794cea33) SHA1(6e9830ce04a505746dea5aafaf37c629c28b061d) ) // checksum == 23FF
	ROM_LOAD16_BYTE( "136093-0012b_cs_3cfe.19j", 0x00001, 0x20000, CRC(577495f8) SHA1(f45b0928b13db7f49b7688620008fc03fca08cde) ) // checksum == 3CFE
	ROM_LOAD16_BYTE( "136093-0013_cs_cbbf.17e",  0x40000, 0x20000, CRC(1e1e82e5) SHA1(d33c84ae950db9775f9db9bf953aa63188d3f2f9) )
	ROM_LOAD16_BYTE( "136093-0014_cs_3ebe.17j",  0x40001, 0x20000, CRC(19e5decd) SHA1(8d93d93f966df46d59cf9f4cdaa689e4dcd2689a) )

	ROM_REGION( 0x280000, "gfx1", ROMREGION_INVERT )
	ROM_LOAD( "136093-0025a.14s",      0x000000, 0x80000, CRC(1b9e5ef2) SHA1(d7d14e75ca2d56c5c67154506096570c9ccbcf8e) )
	ROM_LOAD( "136093-0026a.8d",       0x080000, 0x80000, CRC(09b25d93) SHA1(94d424b21410182b5121201066f4acfa415f4b6b) )
	ROM_LOAD( "136093-0027a.18s",      0x100000, 0x80000, CRC(5bc1c37b) SHA1(89f1bca55dd431ca3171b89347209decf0b25e12) )
	ROM_LOAD( "136093-0028a.10d",      0x180000, 0x80000, CRC(55fb9111) SHA1(a95508f0831842fa79ca2fc168cfadc8c6d3fbd4) )
	ROM_LOAD16_BYTE( "136093-0029.4d", 0x200001, 0x40000, CRC(e4593ff4) SHA1(7360ec7a65aabc90aa787dc30f39992e342495dd) )

	ROM_REGION( 0x100000, "oki", 0 )    /* 2MB for ADPCM data */
	ROM_LOAD( "136093-0030a.9b",  0x000000, 0x80000, CRC(f4c567f5) SHA1(7e8c1d54d918b0b41625eacbaf6dcb5bd99d1949) )
	ROM_LOAD( "136093-0031a.10b", 0x080000, 0x80000, CRC(ba908d73) SHA1(a83afd86f4c39394cf624b728a87b8d8b6de1944) )

	ROM_REGION( 0x800, "eeprom", 0 )
	ROM_LOAD( "relief-eeprom.bin", 0x0000, 0x800, CRC(2131fc40) SHA1(72a9f5f6647fbc74e645b6639db2fdbfbe6456e2) )

	ROM_REGION( 0x001000, "plds", 0 )
	ROM_LOAD( "gal16v8a-136093-0002.15f", 0x0000, 0x0117, CRC(b111d5f2) SHA1(0fe5b4ca786e839e6927a485109d33fe31c737a2) )
	ROM_LOAD( "gal16v8a-136093-0003.11r", 0x0200, 0x0117, CRC(67165ed2) SHA1(c7d9b9c45dd34e588c3db4e23c9c562334d2172a) )
	ROM_LOAD( "gal16v8a-136093-0004.5n",  0x0400, 0x0117, CRC(047b384a) SHA1(d268a65cf2d0fc0cfc7dd6b5c7a77572337c1707) )
	ROM_LOAD( "gal16v8a-136093-0005.3f",  0x0600, 0x0117, CRC(f76825b7) SHA1(2cd9cbb2564e5005a833403ae73f1a964dcb66fa) )
	ROM_LOAD( "gal16v8a-136093-0006.5f",  0x0800, 0x0117, CRC(c580d2a9) SHA1(b070fa53ec083fbeda8cd592bfbbd1e029b4dbe7) )
	ROM_LOAD( "gal16v8a-136093-0008.3e",  0x0a00, 0x0117, CRC(0117910e) SHA1(c8baecd24af201ad51cf13bf46c5cc0e7f7f2a94) )
	ROM_LOAD( "gal16v8a-136093-0009.8a",  0x0c00, 0x0117, CRC(b8679030) SHA1(09ab39c9c293381bbc082780f00d112c71e9c889) )
	ROM_LOAD( "gal16v8a-136093-0010.15a", 0x0e00, 0x0117, CRC(5f49c736) SHA1(91ff18e4780ee6c904735fc0f0e73ffb5a80b49a) )
ROM_END



/*************************************
 *
 *  Driver initialization
 *
 *************************************/

void relief_state::init_relief()
{
	m_okibank->configure_entries(0, 8, memregion("oki")->base(), 0x20000);
	m_okibank->set_entry(0);
}




/*************************************
 *
 *  Game driver(s)
 *
 *************************************/

GAME( 1992, relief,  0,      relief, relief, relief_state, init_relief, ROT0, "Atari Games", "Relief Pitcher (Rev D, 07 Jun 1992 / 28 May 1992)", 0 )
GAME( 1992, relief2, relief, relief, relief, relief_state, init_relief, ROT0, "Atari Games", "Relief Pitcher (Rev C, 26 Apr 1992 / 08 Apr 1992)", 0 )
GAME( 1992, relief3, relief, relief, relief, relief_state, init_relief, ROT0, "Atari Games", "Relief Pitcher (Rev B, 10 Apr 1992 / 08 Apr 1992)", 0 )
