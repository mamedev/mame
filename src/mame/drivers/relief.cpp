// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    Atari "Round" hardware

    driver by Aaron Giles

    Games supported:
        * Relief Pitcher (1990) [2 sets]

    Known bugs:
        * none at this time

****************************************************************************

    Memory map (TBA)

***************************************************************************/


#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "machine/atarigen.h"
#include "sound/okim6295.h"
#include "sound/2413intf.h"
#include "includes/relief.h"


/*************************************
 *
 *  Interrupt handling
 *
 *************************************/

void relief_state::update_interrupts()
{
	m_maincpu->set_input_line(4, m_scanline_int_state ? ASSERT_LINE : CLEAR_LINE);
}



/*************************************
 *
 *  Initialization
 *
 *************************************/

MACHINE_RESET_MEMBER(relief_state,relief)
{
	atarigen_state::machine_reset();

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

READ16_MEMBER(relief_state::special_port2_r)
{
	int result = ioport("260010")->read();
	if (!(result & 0x0080) || get_hblank(*m_screen)) result ^= 0x0001;
	return result;
}



/*************************************
 *
 *  Audio control I/O
 *
 *************************************/

WRITE16_MEMBER(relief_state::audio_control_w)
{
	if (ACCESSING_BITS_0_7)
	{
		m_ym2413_volume = (data >> 1) & 15;
		set_ym2413_volume((m_ym2413_volume * m_overall_volume * 100) / (127 * 15));
		m_adpcm_bank = ((data >> 6) & 3) | (m_adpcm_bank & 4);
	}
	if (ACCESSING_BITS_8_15)
		m_adpcm_bank = (((data >> 8) & 1)<<2) | (m_adpcm_bank & 3);

	m_okibank->set_entry(m_adpcm_bank);
}


WRITE16_MEMBER(relief_state::audio_volume_w)
{
	if (ACCESSING_BITS_0_7)
	{
		m_overall_volume = data & 127;
		set_ym2413_volume((m_ym2413_volume * m_overall_volume * 100) / (127 * 15));
		set_oki6295_volume(m_overall_volume * 100 / 127);
	}
}

static ADDRESS_MAP_START( oki_map, AS_0, 8, relief_state )
	AM_RANGE(0x00000, 0x1ffff) AM_ROMBANK("okibank")
	AM_RANGE(0x20000, 0x3ffff) AM_ROM
ADDRESS_MAP_END


/*************************************
 *
 *  Main CPU memory handlers
 *
 *************************************/

static ADDRESS_MAP_START( main_map, AS_PROGRAM, 16, relief_state )
	ADDRESS_MAP_UNMAP_HIGH
	ADDRESS_MAP_GLOBAL_MASK(0x3fffff)
	AM_RANGE(0x000000, 0x07ffff) AM_ROM
	AM_RANGE(0x140000, 0x140003) AM_DEVWRITE8("ymsnd", ym2413_device, write, 0x00ff)
	AM_RANGE(0x140010, 0x140011) AM_DEVREADWRITE8("oki", okim6295_device, read, write, 0x00ff)
	AM_RANGE(0x140020, 0x140021) AM_WRITE(audio_volume_w)
	AM_RANGE(0x140030, 0x140031) AM_WRITE(audio_control_w)
	AM_RANGE(0x180000, 0x180fff) AM_DEVREADWRITE8("eeprom", atari_eeprom_device, read, write, 0xff00)
	AM_RANGE(0x1c0030, 0x1c0031) AM_DEVWRITE("eeprom", atari_eeprom_device, unlock_write)
	AM_RANGE(0x260000, 0x260001) AM_READ_PORT("260000")
	AM_RANGE(0x260002, 0x260003) AM_READ_PORT("260002")
	AM_RANGE(0x260010, 0x260011) AM_READ(special_port2_r)
	AM_RANGE(0x260012, 0x260013) AM_READ_PORT("260012")
	AM_RANGE(0x2a0000, 0x2a0001) AM_WRITE(watchdog_reset16_w)
	AM_RANGE(0x3e0000, 0x3e0fff) AM_RAM_DEVWRITE("palette", palette_device, write) AM_SHARE("palette")
	AM_RANGE(0x3effc0, 0x3effff) AM_DEVREADWRITE("vad", atari_vad_device, control_read, control_write)
	AM_RANGE(0x3f0000, 0x3f1fff) AM_RAM_DEVWRITE("vad", atari_vad_device, playfield2_latched_msb_w) AM_SHARE("vad:playfield2")
	AM_RANGE(0x3f2000, 0x3f3fff) AM_RAM_DEVWRITE("vad", atari_vad_device, playfield_latched_lsb_w) AM_SHARE("vad:playfield")
	AM_RANGE(0x3f4000, 0x3f5fff) AM_RAM_DEVWRITE("vad", atari_vad_device, playfield_upper_w) AM_SHARE("vad:playfield_ext")
	AM_RANGE(0x3f6000, 0x3f67ff) AM_RAM AM_SHARE("vad:mob")
	AM_RANGE(0x3f6800, 0x3f8eff) AM_RAM
	AM_RANGE(0x3f8f00, 0x3f8f7f) AM_RAM AM_SHARE("vad:eof")
	AM_RANGE(0x3f8f80, 0x3f8fff) AM_SHARE("vad:mob:slip")
	AM_RANGE(0x3f9000, 0x3fffff) AM_RAM
ADDRESS_MAP_END



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


static GFXDECODE_START( relief )
	GFXDECODE_ENTRY( "gfx1", 0, pflayout,   0, 64 )     /* alpha & playfield */
	GFXDECODE_ENTRY( "gfx1", 1, molayout, 256, 16 )     /* sprites */
GFXDECODE_END



/*************************************
 *
 *  Machine driver
 *
 *************************************/

static MACHINE_CONFIG_START( relief, relief_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68000, ATARI_CLOCK_14MHz/2)
	MCFG_CPU_PROGRAM_MAP(main_map)

	MCFG_MACHINE_RESET_OVERRIDE(relief_state,relief)

	MCFG_ATARI_EEPROM_2816_ADD("eeprom")

	/* video hardware */
	MCFG_GFXDECODE_ADD("gfxdecode", "palette", relief)
	MCFG_PALETTE_ADD("palette", 2048)
	MCFG_PALETTE_FORMAT(IRRRRRGGGGGBBBBB)

	MCFG_ATARI_VAD_ADD("vad", "screen", WRITELINE(atarigen_state, scanline_int_write_line))
	MCFG_ATARI_VAD_PLAYFIELD(relief_state, "gfxdecode", get_playfield_tile_info)
	MCFG_ATARI_VAD_PLAYFIELD2(relief_state, "gfxdecode", get_playfield2_tile_info)
	MCFG_ATARI_VAD_MOB(relief_state::s_mob_config, "gfxdecode")

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_VIDEO_ATTRIBUTES(VIDEO_UPDATE_BEFORE_VBLANK)
	/* note: these parameters are from published specs, not derived */
	/* the board uses a VAD chip to generate video signals */
	MCFG_SCREEN_RAW_PARAMS(ATARI_CLOCK_14MHz/2, 456, 0, 336, 262, 0, 240)
	MCFG_SCREEN_UPDATE_DRIVER(relief_state, screen_update_relief)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_VIDEO_START_OVERRIDE(relief_state,relief)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_OKIM6295_ADD("oki", ATARI_CLOCK_14MHz/4/3, OKIM6295_PIN7_LOW)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)
	MCFG_DEVICE_ADDRESS_MAP(AS_0, oki_map)

	MCFG_SOUND_ADD("ymsnd", YM2413, ATARI_CLOCK_14MHz/4)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_CONFIG_END



/*************************************
 *
 *  ROM definition(s)
 *
 *************************************/

// OS:   28 MAY 1992 14:00:14
// MAIN: 07 JUN 1992 20:12:30
ROM_START( relief )
	ROM_REGION( 0x80000, "maincpu", 0 ) /* 8*64k for 68000 code */
	ROM_LOAD16_BYTE( "136093-0011d.19e", 0x00000, 0x20000, CRC(cb3f73ad) SHA1(533a96095e678b4a414d6d9b861b1d4010ced30f) )
	ROM_LOAD16_BYTE( "136093-0012d.19j", 0x00001, 0x20000, CRC(90655721) SHA1(f50a2f317215a864d09e33a4acd927b873350425) )
	ROM_LOAD16_BYTE( "136093-0013.17e", 0x40000, 0x20000, CRC(1e1e82e5) SHA1(d33c84ae950db9775f9db9bf953aa63188d3f2f9) )
	ROM_LOAD16_BYTE( "136093-0014.17j", 0x40001, 0x20000, CRC(19e5decd) SHA1(8d93d93f966df46d59cf9f4cdaa689e4dcd2689a) )

	ROM_REGION( 0x280000, "gfx1", ROMREGION_INVERT )
	ROM_LOAD( "136093-0025a.14s",       0x000000, 0x80000, CRC(1b9e5ef2) SHA1(d7d14e75ca2d56c5c67154506096570c9ccbcf8e) )
	ROM_LOAD( "136093-0026a.8d",        0x080000, 0x80000, CRC(09b25d93) SHA1(94d424b21410182b5121201066f4acfa415f4b6b) )
	ROM_LOAD( "136093-0027a.18s",       0x100000, 0x80000, CRC(5bc1c37b) SHA1(89f1bca55dd431ca3171b89347209decf0b25e12) )
	ROM_LOAD( "136093-0028a.10d",       0x180000, 0x80000, CRC(55fb9111) SHA1(a95508f0831842fa79ca2fc168cfadc8c6d3fbd4) )
	ROM_LOAD16_BYTE( "136093-0029a.4d", 0x200001, 0x40000, CRC(e4593ff4) SHA1(7360ec7a65aabc90aa787dc30f39992e342495dd) )

	ROM_REGION( 0x100000, "oki", 0 )    /* 2MB for ADPCM data */
	ROM_LOAD( "136093-0030a.9b",  0x000000, 0x80000, CRC(f4c567f5) SHA1(7e8c1d54d918b0b41625eacbaf6dcb5bd99d1949) )
	ROM_LOAD( "136093-0031a.10b", 0x080000, 0x80000, CRC(ba908d73) SHA1(a83afd86f4c39394cf624b728a87b8d8b6de1944) )

	ROM_REGION( 0x800, "eeprom:eeprom", 0 )
	ROM_LOAD( "relief-eeprom.bin", 0x0000, 0x800, CRC(66069f60) SHA1(fac3797888f7ffe972f642aca44c6ca7d208c814) )

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
	ROM_LOAD16_BYTE( "19e", 0x00000, 0x20000, CRC(41373e02) SHA1(1982be3d2b959f3504cd7e4afacd96bbebc27b8e) )
	ROM_LOAD16_BYTE( "19j", 0x00001, 0x20000, CRC(8187b026) SHA1(1408b5482194161c1fbb30911bb5b64a14b8ffb0) )
	ROM_LOAD16_BYTE( "136093-0013.17e", 0x40000, 0x20000, CRC(1e1e82e5) SHA1(d33c84ae950db9775f9db9bf953aa63188d3f2f9) )
	ROM_LOAD16_BYTE( "136093-0014.17j", 0x40001, 0x20000, CRC(19e5decd) SHA1(8d93d93f966df46d59cf9f4cdaa689e4dcd2689a) )

	ROM_REGION( 0x280000, "gfx1", ROMREGION_INVERT )
	ROM_LOAD( "136093-0025a.14s",      0x000000, 0x80000, CRC(1b9e5ef2) SHA1(d7d14e75ca2d56c5c67154506096570c9ccbcf8e) )
	ROM_LOAD( "136093-0026a.8d",       0x080000, 0x80000, CRC(09b25d93) SHA1(94d424b21410182b5121201066f4acfa415f4b6b) )
	ROM_LOAD( "136093-0027a.18s",      0x100000, 0x80000, CRC(5bc1c37b) SHA1(89f1bca55dd431ca3171b89347209decf0b25e12) )
	ROM_LOAD( "136093-0028a.10d",      0x180000, 0x80000, CRC(55fb9111) SHA1(a95508f0831842fa79ca2fc168cfadc8c6d3fbd4) )
	ROM_LOAD16_BYTE( "136093-0029.4d", 0x200001, 0x40000, CRC(e4593ff4) SHA1(7360ec7a65aabc90aa787dc30f39992e342495dd) )

	ROM_REGION( 0x100000, "oki", 0 )    /* 2MB for ADPCM data */
	ROM_LOAD( "136093-0030a.9b",  0x000000, 0x80000, CRC(f4c567f5) SHA1(7e8c1d54d918b0b41625eacbaf6dcb5bd99d1949) )
	ROM_LOAD( "136093-0031a.10b", 0x080000, 0x80000, CRC(ba908d73) SHA1(a83afd86f4c39394cf624b728a87b8d8b6de1944) )

	ROM_REGION( 0x800, "eeprom:eeprom", 0 )
	ROM_LOAD( "relief2-eeprom.bin", 0x0000, 0x800, CRC(2131fc40) SHA1(72a9f5f6647fbc74e645b6639db2fdbfbe6456e2) )

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
	ROM_LOAD16_BYTE( "136093-0011b.19e", 0x00000, 0x20000, CRC(794cea33) SHA1(6e9830ce04a505746dea5aafaf37c629c28b061d) )
	ROM_LOAD16_BYTE( "136093-0012b.19j", 0x00001, 0x20000, CRC(577495f8) SHA1(f45b0928b13db7f49b7688620008fc03fca08cde) )
	ROM_LOAD16_BYTE( "136093-0013.17e", 0x40000, 0x20000, CRC(1e1e82e5) SHA1(d33c84ae950db9775f9db9bf953aa63188d3f2f9) )
	ROM_LOAD16_BYTE( "136093-0014.17j", 0x40001, 0x20000, CRC(19e5decd) SHA1(8d93d93f966df46d59cf9f4cdaa689e4dcd2689a) )

	ROM_REGION( 0x280000, "gfx1", ROMREGION_INVERT )
	ROM_LOAD( "136093-0025a.14s",      0x000000, 0x80000, CRC(1b9e5ef2) SHA1(d7d14e75ca2d56c5c67154506096570c9ccbcf8e) )
	ROM_LOAD( "136093-0026a.8d",       0x080000, 0x80000, CRC(09b25d93) SHA1(94d424b21410182b5121201066f4acfa415f4b6b) )
	ROM_LOAD( "136093-0027a.18s",      0x100000, 0x80000, CRC(5bc1c37b) SHA1(89f1bca55dd431ca3171b89347209decf0b25e12) )
	ROM_LOAD( "136093-0028a.10d",      0x180000, 0x80000, CRC(55fb9111) SHA1(a95508f0831842fa79ca2fc168cfadc8c6d3fbd4) )
	ROM_LOAD16_BYTE( "136093-0029.4d", 0x200001, 0x40000, CRC(e4593ff4) SHA1(7360ec7a65aabc90aa787dc30f39992e342495dd) )

	ROM_REGION( 0x100000, "oki", 0 )    /* 2MB for ADPCM data */
	ROM_LOAD( "136093-0030a.9b",  0x000000, 0x80000, CRC(f4c567f5) SHA1(7e8c1d54d918b0b41625eacbaf6dcb5bd99d1949) )
	ROM_LOAD( "136093-0031a.10b", 0x080000, 0x80000, CRC(ba908d73) SHA1(a83afd86f4c39394cf624b728a87b8d8b6de1944) )

	ROM_REGION( 0x800, "eeprom:eeprom", 0 )
	ROM_LOAD( "relief3-eeprom.bin", 0x0000, 0x800, CRC(2131fc40) SHA1(72a9f5f6647fbc74e645b6639db2fdbfbe6456e2) )

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

DRIVER_INIT_MEMBER(relief_state,relief)
{
	m_okibank->configure_entries(0, 8, memregion("oki")->base(), 0x20000);
	m_okibank->set_entry(0);
}




/*************************************
 *
 *  Game driver(s)
 *
 *************************************/

GAME( 1992, relief,  0,      relief, relief, relief_state, relief, ROT0, "Atari Games", "Relief Pitcher (set 1, 07 Jun 1992 / 28 May 1992)", 0 )
GAME( 1992, relief2, relief, relief, relief, relief_state, relief, ROT0, "Atari Games", "Relief Pitcher (set 2, 26 Apr 1992 / 08 Apr 1992)", 0 )
GAME( 1992, relief3, relief, relief, relief, relief_state, relief, ROT0, "Atari Games", "Relief Pitcher (set 3, 10 Apr 1992 / 08 Apr 1992)", 0 )
