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
#include "video/atarimo.h"
#include "includes/relief.h"


/*************************************
 *
 *  Interrupt handling
 *
 *************************************/

static void update_interrupts(running_machine &machine)
{
	relief_state *state = machine.driver_data<relief_state>();
	cputag_set_input_line(machine, "maincpu", 4, state->m_scanline_int_state ? ASSERT_LINE : CLEAR_LINE);
}



/*************************************
 *
 *  Video controller access
 *
 *************************************/

static READ16_HANDLER( relief_atarivc_r )
{
	return atarivc_r(*space->machine().primary_screen, offset);
}


static WRITE16_HANDLER( relief_atarivc_w )
{
	atarivc_w(*space->machine().primary_screen, offset, data, mem_mask);
}



/*************************************
 *
 *  Initialization
 *
 *************************************/

static MACHINE_START( relief )
{
	atarigen_init(machine);
}


static MACHINE_RESET( relief )
{
	relief_state *state = machine.driver_data<relief_state>();

	atarigen_eeprom_reset(state);
	atarigen_interrupt_reset(state, update_interrupts);
	atarivc_reset(*machine.primary_screen, state->m_atarivc_eof_data, 2);

	machine.device<okim6295_device>("oki")->set_bank_base(0);
	state->m_ym2413_volume = 15;
	state->m_overall_volume = 127;
	state->m_adpcm_bank_base = 0;
}



/*************************************
 *
 *  I/O handling
 *
 *************************************/

static READ16_HANDLER( special_port2_r )
{
	relief_state *state = space->machine().driver_data<relief_state>();
	int result = input_port_read(space->machine(), "260010");
	if (state->m_cpu_to_sound_ready) result ^= 0x0020;
	if (!(result & 0x0080) || atarigen_get_hblank(*space->machine().primary_screen)) result ^= 0x0001;
	return result;
}



/*************************************
 *
 *  Audio control I/O
 *
 *************************************/

static WRITE16_HANDLER( audio_control_w )
{
	relief_state *state = space->machine().driver_data<relief_state>();
	if (ACCESSING_BITS_0_7)
	{
		state->m_ym2413_volume = (data >> 1) & 15;
		atarigen_set_ym2413_vol(space->machine(), (state->m_ym2413_volume * state->m_overall_volume * 100) / (127 * 15));
		state->m_adpcm_bank_base = (0x040000 * ((data >> 6) & 3)) | (state->m_adpcm_bank_base & 0x100000);
	}
	if (ACCESSING_BITS_8_15)
		state->m_adpcm_bank_base = (0x100000 * ((data >> 8) & 1)) | (state->m_adpcm_bank_base & 0x0c0000);

	okim6295_device *oki = space->machine().device<okim6295_device>("oki");
	oki->set_bank_base(state->m_adpcm_bank_base);
}


static WRITE16_HANDLER( audio_volume_w )
{
	relief_state *state = space->machine().driver_data<relief_state>();
	if (ACCESSING_BITS_0_7)
	{
		state->m_overall_volume = data & 127;
		atarigen_set_ym2413_vol(space->machine(), (state->m_ym2413_volume * state->m_overall_volume * 100) / (127 * 15));
		atarigen_set_oki6295_vol(space->machine(), state->m_overall_volume * 100 / 127);
	}
}



/*************************************
 *
 *  Main CPU memory handlers
 *
 *************************************/

static ADDRESS_MAP_START( main_map, AS_PROGRAM, 16, relief_state )
	ADDRESS_MAP_UNMAP_HIGH
	ADDRESS_MAP_GLOBAL_MASK(0x3fffff)
	AM_RANGE(0x000000, 0x07ffff) AM_ROM
	AM_RANGE(0x140000, 0x140003) AM_DEVWRITE8("ymsnd", ym2413_w, 0x00ff)
	AM_RANGE(0x140010, 0x140011) AM_DEVREADWRITE8_MODERN("oki", okim6295_device, read, write, 0x00ff)
	AM_RANGE(0x140020, 0x140021) AM_WRITE(audio_volume_w)
	AM_RANGE(0x140030, 0x140031) AM_WRITE(audio_control_w)
	AM_RANGE(0x180000, 0x180fff) AM_READWRITE(atarigen_eeprom_upper_r, atarigen_eeprom_w) AM_SHARE("eeprom")
	AM_RANGE(0x1c0030, 0x1c0031) AM_WRITE(atarigen_eeprom_enable_w)
	AM_RANGE(0x260000, 0x260001) AM_READ_PORT("260000")
	AM_RANGE(0x260002, 0x260003) AM_READ_PORT("260002")
	AM_RANGE(0x260010, 0x260011) AM_READ(special_port2_r)
	AM_RANGE(0x260012, 0x260013) AM_READ_PORT("260012")
	AM_RANGE(0x2a0000, 0x2a0001) AM_WRITE(watchdog_reset16_w)
	AM_RANGE(0x3e0000, 0x3e0fff) AM_RAM_WRITE(atarigen_666_paletteram_w) AM_BASE_GENERIC(paletteram)
	AM_RANGE(0x3effc0, 0x3effff) AM_READWRITE(relief_atarivc_r, relief_atarivc_w) AM_BASE_MEMBER(relief_state, m_atarivc_data)
	AM_RANGE(0x3f0000, 0x3f1fff) AM_RAM_WRITE(atarigen_playfield2_latched_msb_w) AM_BASE_MEMBER(relief_state, m_playfield2)
	AM_RANGE(0x3f2000, 0x3f3fff) AM_RAM_WRITE(atarigen_playfield_latched_lsb_w) AM_BASE_MEMBER(relief_state, m_playfield)
	AM_RANGE(0x3f4000, 0x3f5fff) AM_RAM_WRITE(atarigen_playfield_dual_upper_w) AM_BASE_MEMBER(relief_state, m_playfield_upper)
	AM_RANGE(0x3f6000, 0x3f67ff) AM_READWRITE(atarimo_0_spriteram_r, atarimo_0_spriteram_w)
	AM_RANGE(0x3f6800, 0x3f8eff) AM_RAM
	AM_RANGE(0x3f8f00, 0x3f8f7f) AM_RAM AM_BASE_MEMBER(relief_state, m_atarivc_eof_data)
	AM_RANGE(0x3f8f80, 0x3f8fff) AM_READWRITE(atarimo_0_slipram_r, atarimo_0_slipram_w)
	AM_RANGE(0x3f9000, 0x3fffff) AM_RAM
ADDRESS_MAP_END



/*************************************
 *
 *  Port definitions
 *
 *************************************/

static INPUT_PORTS_START( relief )
	PORT_START("260000")	/* 260000 */
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("Button D0") PORT_CODE(KEYCODE_Z)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("Button D1") PORT_CODE(KEYCODE_X)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("Button D2") PORT_CODE(KEYCODE_C)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("Button D3") PORT_CODE(KEYCODE_V)
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Unknown ) )
	PORT_DIPSETTING(	  0x0010, DEF_STR( Off ) )
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

	PORT_START("260002")	/* 260002 */
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

	PORT_START("260010")	/* 260010 */
	PORT_BIT( 0x001f, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNUSED )	/* tested before writing to 260040 */
	PORT_SERVICE( 0x0040, IP_ACTIVE_LOW )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_VBLANK )
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("260012")	/* 260012 */
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
	GFXDECODE_ENTRY( "gfx1", 0, pflayout,   0, 64 )		/* alpha & playfield */
	GFXDECODE_ENTRY( "gfx1", 1, molayout, 256, 16 )		/* sprites */
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

	MCFG_MACHINE_START(relief)
	MCFG_MACHINE_RESET(relief)
	MCFG_NVRAM_ADD_1FILL("eeprom")

	/* video hardware */
	MCFG_VIDEO_ATTRIBUTES(VIDEO_UPDATE_BEFORE_VBLANK)
	MCFG_GFXDECODE(relief)
	MCFG_PALETTE_LENGTH(2048)

	MCFG_SCREEN_ADD("screen", RASTER)
	/* note: these parameters are from published specs, not derived */
	/* the board uses a VAD chip to generate video signals */
	MCFG_SCREEN_RAW_PARAMS(ATARI_CLOCK_14MHz/2, 456, 0, 336, 262, 0, 240)
	MCFG_SCREEN_UPDATE_STATIC(relief)

	MCFG_VIDEO_START(relief)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_OKIM6295_ADD("oki", ATARI_CLOCK_14MHz/4/3, OKIM6295_PIN7_LOW)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)

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
	ROM_REGION( 0x80000, "maincpu", 0 )	/* 8*64k for 68000 code */
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

	ROM_REGION( 0x200000, "oki", 0 )	/* 2MB for ADPCM data */
	ROM_LOAD( "136093-0030a.9b",  0x100000, 0x80000, CRC(f4c567f5) SHA1(7e8c1d54d918b0b41625eacbaf6dcb5bd99d1949) )
	ROM_LOAD( "136093-0031a.10b", 0x180000, 0x80000, CRC(ba908d73) SHA1(a83afd86f4c39394cf624b728a87b8d8b6de1944) )

	ROM_REGION( 0x1000, "eeprom", 0 )
	ROM_LOAD( "relief-eeprom.bin", 0x0000, 0x1000, CRC(71090711) SHA1(fc516dd84635e544c5f89640f77645e4a7d5bcc0) )

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
	ROM_REGION( 0x80000, "maincpu", 0 )	/* 8*64k for 68000 code */
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

	ROM_REGION( 0x200000, "oki", 0 )	/* 2MB for ADPCM data */
	ROM_LOAD( "136093-0030a.9b",  0x100000, 0x80000, CRC(f4c567f5) SHA1(7e8c1d54d918b0b41625eacbaf6dcb5bd99d1949) )
	ROM_LOAD( "136093-0031a.10b", 0x180000, 0x80000, CRC(ba908d73) SHA1(a83afd86f4c39394cf624b728a87b8d8b6de1944) )

	ROM_REGION( 0x1000, "eeprom", ROMREGION_ERASEFF )
	/* todo */
//  ROM_LOAD( "relief_26apr-eeprom.bin", 0x0000, 0x1000, CRC(6cd70719) SHA1(1bbc52924931b68a546c85f689419200a4b6547b) )

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
	ROM_REGION( 0x80000, "maincpu", 0 )	/* 8*64k for 68000 code */
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

	ROM_REGION( 0x200000, "oki", 0 )	/* 2MB for ADPCM data */
	ROM_LOAD( "136093-0030a.9b",  0x100000, 0x80000, CRC(f4c567f5) SHA1(7e8c1d54d918b0b41625eacbaf6dcb5bd99d1949) )
	ROM_LOAD( "136093-0031a.10b", 0x180000, 0x80000, CRC(ba908d73) SHA1(a83afd86f4c39394cf624b728a87b8d8b6de1944) )

	ROM_REGION( 0x1000, "eeprom", 0 )
	ROM_LOAD( "relief2-eeprom.bin", 0x0000, 0x1000, CRC(6cd70719) SHA1(1bbc52924931b68a546c85f689419200a4b6547b) )

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

static DRIVER_INIT( relief )
{
	UINT8 *sound_base = machine.region("oki")->base();

	/* expand the ADPCM data to avoid lots of memcpy's during gameplay */
	/* the upper 128k is fixed, the lower 128k is bankswitched */
	memcpy(&sound_base[0x000000], &sound_base[0x100000], 0x20000);
	memcpy(&sound_base[0x040000], &sound_base[0x100000], 0x20000);
	memcpy(&sound_base[0x080000], &sound_base[0x140000], 0x20000);
	memcpy(&sound_base[0x0c0000], &sound_base[0x160000], 0x20000);
	memcpy(&sound_base[0x100000], &sound_base[0x180000], 0x20000);
	memcpy(&sound_base[0x140000], &sound_base[0x1a0000], 0x20000);
	memcpy(&sound_base[0x180000], &sound_base[0x1c0000], 0x20000);
	memcpy(&sound_base[0x1c0000], &sound_base[0x1e0000], 0x20000);

	memcpy(&sound_base[0x020000], &sound_base[0x120000], 0x20000);
	memcpy(&sound_base[0x060000], &sound_base[0x120000], 0x20000);
	memcpy(&sound_base[0x0a0000], &sound_base[0x120000], 0x20000);
	memcpy(&sound_base[0x0e0000], &sound_base[0x120000], 0x20000);
	memcpy(&sound_base[0x160000], &sound_base[0x120000], 0x20000);
	memcpy(&sound_base[0x1a0000], &sound_base[0x120000], 0x20000);
	memcpy(&sound_base[0x1e0000], &sound_base[0x120000], 0x20000);
}



/*************************************
 *
 *  Game driver(s)
 *
 *************************************/

GAME( 1992, relief,  0,      relief, relief, relief, ROT0, "Atari Games", "Relief Pitcher (set 1, 07 Jun 1992 / 28 May 1992)", 0 )
GAME( 1992, relief2, relief, relief, relief, relief, ROT0, "Atari Games", "Relief Pitcher (set 2, 26 Apr 1992 / 08 Apr 1992)", 0 )
GAME( 1992, relief3, relief, relief, relief, relief, ROT0, "Atari Games", "Relief Pitcher (set 3, 10 Apr 1992 / 08 Apr 1992)", 0 )
