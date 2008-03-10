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


#include "driver.h"
#include "machine/atarigen.h"
#include "relief.h"
#include "sound/okim6295.h"
#include "sound/2413intf.h"


/*************************************
 *
 *  Statics
 *
 *************************************/

static UINT8 ym2413_volume;
static UINT8 overall_volume;
static UINT32 adpcm_bank_base;



/*************************************
 *
 *  Interrupt handling
 *
 *************************************/

static void update_interrupts(running_machine *machine)
{
	int newstate = 0;

	if (atarigen_scanline_int_state)
		newstate = 4;

	if (newstate)
		cpunum_set_input_line(machine, 0, newstate, ASSERT_LINE);
	else
		cpunum_set_input_line(machine, 0, 7, CLEAR_LINE);
}



/*************************************
 *
 *  Video controller access
 *
 *************************************/

static READ16_HANDLER( relief_atarivc_r )
{
	return atarivc_r(machine->primary_screen, offset);
}


static WRITE16_HANDLER( relief_atarivc_w )
{
	atarivc_w(machine->primary_screen, offset, data, mem_mask);
}



/*************************************
 *
 *  Initialization
 *
 *************************************/

static MACHINE_RESET( relief )
{
	atarigen_eeprom_reset();
	atarivc_reset(machine->primary_screen, atarivc_eof_data, 2);
	atarigen_interrupt_reset(update_interrupts);

	OKIM6295_set_bank_base(0, 0);
	ym2413_volume = 15;
	overall_volume = 127;
	adpcm_bank_base = 0;
}



/*************************************
 *
 *  I/O handling
 *
 *************************************/

static READ16_HANDLER( special_port2_r )
{
	int result = readinputport(2);
	if (atarigen_cpu_to_sound_ready) result ^= 0x0020;
	if (!(result & 0x0080) || atarigen_get_hblank(machine->primary_screen)) result ^= 0x0001;
	return result;
}



/*************************************
 *
 *  Audio control I/O
 *
 *************************************/

static WRITE16_HANDLER( audio_control_w )
{
	if (ACCESSING_LSB)
	{
		ym2413_volume = (data >> 1) & 15;
		atarigen_set_ym2413_vol(machine, (ym2413_volume * overall_volume * 100) / (127 * 15));
		adpcm_bank_base = (0x040000 * ((data >> 6) & 3)) | (adpcm_bank_base & 0x100000);
	}
	if (ACCESSING_MSB)
		adpcm_bank_base = (0x100000 * ((data >> 8) & 1)) | (adpcm_bank_base & 0x0c0000);

	OKIM6295_set_bank_base(0, adpcm_bank_base);
}


static WRITE16_HANDLER( audio_volume_w )
{
	if (ACCESSING_LSB)
	{
		overall_volume = data & 127;
		atarigen_set_ym2413_vol(machine, (ym2413_volume * overall_volume * 100) / (127 * 15));
		atarigen_set_oki6295_vol(machine, overall_volume * 100 / 127);
	}
}



/*************************************
 *
 *  MSM5295 I/O
 *
 *************************************/

static READ16_HANDLER( adpcm_r )
{
	return OKIM6295_status_0_r(machine, offset) | 0xff00;
}


static WRITE16_HANDLER( adpcm_w )
{
	if (ACCESSING_LSB)
		OKIM6295_data_0_w(machine, offset, data & 0xff);
}



/*************************************
 *
 *  YM2413 I/O
 *
 *************************************/

static WRITE16_HANDLER( ym2413_w )
{
	if (ACCESSING_LSB)
	{
		if (offset & 1)
			YM2413_data_port_0_w(machine, 0, data & 0xff);
		else
			YM2413_register_port_0_w(machine, 0, data & 0xff);
	}
}



/*************************************
 *
 *  Main CPU memory handlers
 *
 *************************************/

static ADDRESS_MAP_START( main_map, ADDRESS_SPACE_PROGRAM, 16 )
	ADDRESS_MAP_UNMAP_HIGH
	ADDRESS_MAP_GLOBAL_MASK(0x3fffff)
	AM_RANGE(0x000000, 0x07ffff) AM_ROM
	AM_RANGE(0x140000, 0x140003) AM_WRITE(ym2413_w)
	AM_RANGE(0x140010, 0x140011) AM_READWRITE(adpcm_r, adpcm_w)
	AM_RANGE(0x140020, 0x140021) AM_WRITE(audio_volume_w)
	AM_RANGE(0x140030, 0x140031) AM_WRITE(audio_control_w)
	AM_RANGE(0x180000, 0x180fff) AM_READWRITE(atarigen_eeprom_upper_r, atarigen_eeprom_w) AM_BASE(&atarigen_eeprom) AM_SIZE(&atarigen_eeprom_size)
	AM_RANGE(0x1c0030, 0x1c0031) AM_WRITE(atarigen_eeprom_enable_w)
	AM_RANGE(0x260000, 0x260001) AM_READ(input_port_0_word_r)
	AM_RANGE(0x260002, 0x260003) AM_READ(input_port_1_word_r)
	AM_RANGE(0x260010, 0x260011) AM_READ(special_port2_r)
	AM_RANGE(0x260012, 0x260013) AM_READ(input_port_3_word_r)
	AM_RANGE(0x2a0000, 0x2a0001) AM_WRITE(watchdog_reset16_w)
	AM_RANGE(0x3e0000, 0x3e0fff) AM_READWRITE(SMH_RAM, atarigen_666_paletteram_w) AM_BASE(&paletteram16)
	AM_RANGE(0x3effc0, 0x3effff) AM_READWRITE(relief_atarivc_r, relief_atarivc_w) AM_BASE(&atarivc_data)
	AM_RANGE(0x3f0000, 0x3f1fff) AM_READWRITE(SMH_RAM, atarigen_playfield2_latched_msb_w) AM_BASE(&atarigen_playfield2)
	AM_RANGE(0x3f2000, 0x3f3fff) AM_READWRITE(SMH_RAM, atarigen_playfield_latched_lsb_w) AM_BASE(&atarigen_playfield)
	AM_RANGE(0x3f4000, 0x3f5fff) AM_READWRITE(SMH_RAM, atarigen_playfield_dual_upper_w) AM_BASE(&atarigen_playfield_upper)
	AM_RANGE(0x3f6000, 0x3f67ff) AM_READWRITE(SMH_RAM, atarimo_0_spriteram_w) AM_BASE(&atarimo_0_spriteram)
	AM_RANGE(0x3f6800, 0x3f8eff) AM_RAM
	AM_RANGE(0x3f8f00, 0x3f8f7f) AM_RAM AM_BASE(&atarivc_eof_data)
	AM_RANGE(0x3f8f80, 0x3f8fff) AM_READWRITE(SMH_RAM, atarimo_0_slipram_w) AM_BASE(&atarimo_0_slipram)
	AM_RANGE(0x3f9000, 0x3fffff) AM_RAM
ADDRESS_MAP_END



/*************************************
 *
 *  Port definitions
 *
 *************************************/

static INPUT_PORTS_START( relief )
	PORT_START	/* 260000 */
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

	PORT_START	/* 260002 */
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

	PORT_START	/* 260010 */
	PORT_BIT( 0x001f, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNUSED )	/* tested before writing to 260040 */
	PORT_SERVICE( 0x0040, IP_ACTIVE_LOW )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_VBLANK )
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START	/* 260012 */
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

static const gfx_layout pfmolayout =
{
	8,8,
	RGN_FRAC(1,4),
	4,
	{ RGN_FRAC(3,4), RGN_FRAC(2,4), RGN_FRAC(1,4), RGN_FRAC(0,4) },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 2*8, 4*8, 6*8, 8*8, 10*8, 12*8, 14*8 },
	16*8
};


static const gfx_layout moexlayout =
{
	8,8,
	RGN_FRAC(1,1),
	5,
	{ 0, 0, 0, 0, 0 },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8
};


static GFXDECODE_START( relief )
	GFXDECODE_ENTRY( REGION_GFX1, 0, pfmolayout,   0, 64 )		/* alpha & playfield */
	GFXDECODE_ENTRY( REGION_GFX1, 1, pfmolayout, 256, 16 )		/* sprites */
	GFXDECODE_ENTRY( REGION_GFX2, 0, moexlayout, 256, 16 )		/* extra sprite bit */
GFXDECODE_END



/*************************************
 *
 *  Machine driver
 *
 *************************************/

static MACHINE_DRIVER_START( relief )

	/* basic machine hardware */
	MDRV_CPU_ADD(M68000, ATARI_CLOCK_14MHz/2)
	MDRV_CPU_PROGRAM_MAP(main_map,0)

	MDRV_MACHINE_RESET(relief)
	MDRV_NVRAM_HANDLER(atarigen)

	/* video hardware */
	MDRV_VIDEO_ATTRIBUTES(VIDEO_UPDATE_BEFORE_VBLANK)
	MDRV_GFXDECODE(relief)
	MDRV_PALETTE_LENGTH(2048)

	MDRV_SCREEN_ADD("main", RASTER)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	/* note: these parameters are from published specs, not derived */
	/* the board uses a VAD chip to generate video signals */
	MDRV_SCREEN_RAW_PARAMS(ATARI_CLOCK_14MHz/2, 456, 0, 336, 262, 0, 240)

	MDRV_VIDEO_START(relief)
	MDRV_VIDEO_UPDATE(relief)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_SOUND_ADD(OKIM6295, ATARI_CLOCK_14MHz/4/3)
	MDRV_SOUND_CONFIG(okim6295_interface_region_1_pin7low)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)

	MDRV_SOUND_ADD(YM2413, ATARI_CLOCK_14MHz/4)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_DRIVER_END



/*************************************
 *
 *  ROM definition(s)
 *
 *************************************/

ROM_START( relief )
	ROM_REGION( 0x80000, REGION_CPU1, 0 )	/* 8*64k for 68000 code */
	ROM_LOAD16_BYTE( "136093-0011d.19e", 0x00000, 0x20000, CRC(cb3f73ad) SHA1(533a96095e678b4a414d6d9b861b1d4010ced30f) )
	ROM_LOAD16_BYTE( "136093-0012d.19j", 0x00001, 0x20000, CRC(90655721) SHA1(f50a2f317215a864d09e33a4acd927b873350425) )
	ROM_LOAD16_BYTE( "136093-0013.17e", 0x40000, 0x20000, CRC(1e1e82e5) SHA1(d33c84ae950db9775f9db9bf953aa63188d3f2f9) )
	ROM_LOAD16_BYTE( "136093-0014.17j", 0x40001, 0x20000, CRC(19e5decd) SHA1(8d93d93f966df46d59cf9f4cdaa689e4dcd2689a) )

	ROM_REGION( 0x200000, REGION_GFX1, ROMREGION_DISPOSE | ROMREGION_INVERT )
	ROM_LOAD( "136093-0025a.14s", 0x000000, 0x80000, CRC(1b9e5ef2) SHA1(d7d14e75ca2d56c5c67154506096570c9ccbcf8e) )
	ROM_LOAD( "136093-0026a.8d",  0x080000, 0x80000, CRC(09b25d93) SHA1(94d424b21410182b5121201066f4acfa415f4b6b) )
	ROM_LOAD( "136093-0027a.18s", 0x100000, 0x80000, CRC(5bc1c37b) SHA1(89f1bca55dd431ca3171b89347209decf0b25e12) )
	ROM_LOAD( "136093-0028a.10d", 0x180000, 0x80000, CRC(55fb9111) SHA1(a95508f0831842fa79ca2fc168cfadc8c6d3fbd4) )

	ROM_REGION( 0x040000, REGION_GFX2, ROMREGION_DISPOSE | ROMREGION_INVERT )
	ROM_LOAD( "136093-0029a.4d",  0x000000, 0x40000, CRC(e4593ff4) SHA1(7360ec7a65aabc90aa787dc30f39992e342495dd) )

	ROM_REGION( 0x200000, REGION_SOUND1, 0 )	/* 2MB for ADPCM data */
	ROM_LOAD( "136093-0030a.9b",  0x100000, 0x80000, CRC(f4c567f5) SHA1(7e8c1d54d918b0b41625eacbaf6dcb5bd99d1949) )
	ROM_LOAD( "136093-0031a.10b", 0x180000, 0x80000, CRC(ba908d73) SHA1(a83afd86f4c39394cf624b728a87b8d8b6de1944) )

	ROM_REGION( 0x001000, REGION_PLDS, ROMREGION_DISPOSE )
	ROM_LOAD( "gal16v8a-136093-0002.15f", 0x0000, 0x0117, CRC(b111d5f2) SHA1(0fe5b4ca786e839e6927a485109d33fe31c737a2) )
	ROM_LOAD( "gal16v8a-136093-0003.11r", 0x0200, 0x0117, CRC(67165ed2) SHA1(c7d9b9c45dd34e588c3db4e23c9c562334d2172a) )
	ROM_LOAD( "gal16v8a-136093-0004.5n",  0x0400, 0x0117, CRC(047b384a) SHA1(d268a65cf2d0fc0cfc7dd6b5c7a77572337c1707) )
	ROM_LOAD( "gal16v8a-136093-0005.3f",  0x0600, 0x0117, CRC(f76825b7) SHA1(2cd9cbb2564e5005a833403ae73f1a964dcb66fa) )
	ROM_LOAD( "gal16v8a-136093-0006.5f",  0x0800, 0x0117, CRC(c580d2a9) SHA1(b070fa53ec083fbeda8cd592bfbbd1e029b4dbe7) )
	ROM_LOAD( "gal16v8a-136093-0008.3e",  0x0a00, 0x0117, CRC(0117910e) SHA1(c8baecd24af201ad51cf13bf46c5cc0e7f7f2a94) )
	ROM_LOAD( "gal16v8a-136093-0009.8a",  0x0c00, 0x0117, CRC(b8679030) SHA1(09ab39c9c293381bbc082780f00d112c71e9c889) )
	ROM_LOAD( "gal16v8a-136093-0010.15a", 0x0e00, 0x0117, CRC(5f49c736) SHA1(91ff18e4780ee6c904735fc0f0e73ffb5a80b49a) )
ROM_END

ROM_START( relief2 )
	ROM_REGION( 0x80000, REGION_CPU1, 0 )	/* 8*64k for 68000 code */
	ROM_LOAD16_BYTE( "136093-0011b.19e", 0x00000, 0x20000, CRC(794cea33) SHA1(6e9830ce04a505746dea5aafaf37c629c28b061d) )
	ROM_LOAD16_BYTE( "136093-0012b.19j", 0x00001, 0x20000, CRC(577495f8) SHA1(f45b0928b13db7f49b7688620008fc03fca08cde) )
	ROM_LOAD16_BYTE( "136093-0013.17e", 0x40000, 0x20000, CRC(1e1e82e5) SHA1(d33c84ae950db9775f9db9bf953aa63188d3f2f9) )
	ROM_LOAD16_BYTE( "136093-0014.17j", 0x40001, 0x20000, CRC(19e5decd) SHA1(8d93d93f966df46d59cf9f4cdaa689e4dcd2689a) )

	ROM_REGION( 0x200000, REGION_GFX1, ROMREGION_DISPOSE | ROMREGION_INVERT )
	ROM_LOAD( "136093-0025a.14s", 0x000000, 0x80000, CRC(1b9e5ef2) SHA1(d7d14e75ca2d56c5c67154506096570c9ccbcf8e) )
	ROM_LOAD( "136093-0026a.8d",  0x080000, 0x80000, CRC(09b25d93) SHA1(94d424b21410182b5121201066f4acfa415f4b6b) )
	ROM_LOAD( "136093-0027a.18s", 0x100000, 0x80000, CRC(5bc1c37b) SHA1(89f1bca55dd431ca3171b89347209decf0b25e12) )
	ROM_LOAD( "136093-0028a.10d", 0x180000, 0x80000, CRC(55fb9111) SHA1(a95508f0831842fa79ca2fc168cfadc8c6d3fbd4) )

	ROM_REGION( 0x040000, REGION_GFX2, ROMREGION_DISPOSE | ROMREGION_INVERT )
	ROM_LOAD( "136093-0029.4d",  0x000000, 0x40000, CRC(e4593ff4) SHA1(7360ec7a65aabc90aa787dc30f39992e342495dd) )

	ROM_REGION( 0x200000, REGION_SOUND1, 0 )	/* 2MB for ADPCM data */
	ROM_LOAD( "136093-0030a.9b",  0x100000, 0x80000, CRC(f4c567f5) SHA1(7e8c1d54d918b0b41625eacbaf6dcb5bd99d1949) )
	ROM_LOAD( "136093-0031a.10b", 0x180000, 0x80000, CRC(ba908d73) SHA1(a83afd86f4c39394cf624b728a87b8d8b6de1944) )

	ROM_REGION( 0x001000, REGION_PLDS, ROMREGION_DISPOSE )
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

static void init_common(const UINT16 *def_eeprom)
{
	UINT8 *sound_base = memory_region(REGION_SOUND1);

	atarigen_eeprom_default = def_eeprom;

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


static DRIVER_INIT( relief )
{
	static const UINT16 default_eeprom[] =
	{
		0x0001,0x0166,0x0128,0x01E6,0x0100,0x012C,0x0300,0x0144,
		0x0700,0x01C0,0x2F00,0x01EC,0x0B00,0x0148,0x0140,0x0100,
		0x0124,0x0188,0x0120,0x0600,0x0196,0x013C,0x0192,0x0150,
		0x0166,0x0128,0x01E6,0x0100,0x012C,0x0300,0x0144,0x0700,
		0x01C0,0x2F00,0x01EC,0x0B00,0x0148,0x0140,0x0100,0x0124,
		0x0188,0x0120,0x0600,0x0196,0x013C,0x0192,0x0150,0xFF00,
		0x9500,0x0000
	};
	init_common(default_eeprom);
}


static DRIVER_INIT( relief2 )
{
	static const UINT16 default_eeprom[] =
	{
		0x0001,0x01FD,0x019F,0x015E,0x01FF,0x019E,0x03FF,0x015F,
		0x07FF,0x01FD,0x12FF,0x01FC,0x01FB,0x07FF,0x01F7,0x01FF,
		0x01DF,0x02FF,0x017F,0x03FF,0x0300,0x0110,0x0300,0x0140,
		0x0300,0x018E,0x0400,0x0180,0x0101,0x0300,0x0180,0x0204,
		0x0120,0x0182,0x0100,0x0102,0x0600,0x01D5,0x0138,0x0192,
		0x0150,0x01FD,0x019F,0x015E,0x01FF,0x019E,0x03FF,0x015F,
		0x07FF,0x01FD,0x12FF,0x01FC,0x01FB,0x07FF,0x01F7,0x01FF,
		0x01DF,0x02FF,0x017F,0x03FF,0x0300,0x0110,0x0300,0x0140,
		0x0300,0x018E,0x0400,0x0180,0x0101,0x0300,0x0180,0x0204,
		0x0120,0x0182,0x0100,0x0102,0x0600,0x01D5,0x0138,0x0192,
		0x0150,0xE600,0x01C3,0x019D,0x0131,0x0100,0x0116,0x0100,
		0x010A,0x0190,0x010E,0x014A,0x0200,0x010B,0x018D,0x0121,
		0x0100,0x0145,0x0100,0x0109,0x0184,0x012C,0x0200,0x0107,
		0x01AA,0x0149,0x60FF,0x3300,0x0000
	};
	init_common(default_eeprom);
}



/*************************************
 *
 *  Game driver(s)
 *
 *************************************/

GAME( 1992, relief,  0,      relief, relief, relief,  ROT0, "Atari Games", "Relief Pitcher (set 1)", 0 )
GAME( 1992, relief2, relief, relief, relief, relief2, ROT0, "Atari Games", "Relief Pitcher (set 2)", 0 )
