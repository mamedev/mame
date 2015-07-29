// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    Atari Escape hardware

    driver by Aaron Giles

    Games supported:
        * Escape From The Planet Of The Robot Monsters (1989) [2 sets]
        * Klax prototypes [2 sets]
        * Guts n' Glory (prototype)

    Known bugs:
        * none at this time

    TODO:
        * the RGB generator visible in the schematics is not properly modeled.


****************************************************************************

    Memory map (TBA)

***************************************************************************/


#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "includes/eprom.h"



/*************************************
 *
 *  Initialization
 *
 *************************************/

void eprom_state::update_interrupts()
{
	m_maincpu->set_input_line(4, m_video_int_state ? ASSERT_LINE : CLEAR_LINE);

	if (m_extra != NULL)
		m_extra->set_input_line(4, m_video_int_state ? ASSERT_LINE : CLEAR_LINE);

	m_maincpu->set_input_line(6, m_sound_int_state ? ASSERT_LINE : CLEAR_LINE);
}


MACHINE_RESET_MEMBER(eprom_state,eprom)
{
	atarigen_state::machine_reset();
	scanline_timer_reset(*m_screen, 8);
}



/*************************************
 *
 *  I/O handling
 *
 *************************************/

READ16_MEMBER(eprom_state::special_port1_r)
{
	int result = ioport("260010")->read();
	result ^= 0x0010;
	return result;
}


READ16_MEMBER(eprom_state::adc_r)
{
	static const char *const adcnames[] = { "ADC0", "ADC1", "ADC2", "ADC3" };
	int result = ioport(adcnames[m_last_offset & 3])->read();

	m_last_offset = offset;
	return result;
}



/*************************************
 *
 *  Latch write handler
 *
 *************************************/

WRITE16_MEMBER(eprom_state::eprom_latch_w)
{
	if (ACCESSING_BITS_0_7 && (m_extra != NULL))
	{
		/* bit 0: reset extra CPU */
		if (data & 1)
			m_extra->set_input_line(INPUT_LINE_RESET, CLEAR_LINE);
		else
			m_extra->set_input_line(INPUT_LINE_RESET, ASSERT_LINE);

		/* bits 1-4: screen intensity */
		m_screen_intensity = (data & 0x1e) >> 1;

		/* bit 5: video disable */
		m_video_disable = (data & 0x20);
	}
}



/*************************************
 *
 *  Synchronization
 *
 *************************************/

READ16_MEMBER(eprom_state::sync_r)
{
	return m_sync_data[offset];
}


WRITE16_MEMBER(eprom_state::sync_w)
{
	int oldword = m_sync_data[offset];
	int newword = oldword;
	COMBINE_DATA(&newword);

	m_sync_data[offset] = newword;
	if ((oldword & 0xff00) != (newword & 0xff00))
		space.device().execute().yield();
}



/*************************************
 *
 *  Main CPU memory handlers
 *
 *************************************/

static ADDRESS_MAP_START( main_map, AS_PROGRAM, 16, eprom_state )
	AM_RANGE(0x000000, 0x09ffff) AM_ROM
	AM_RANGE(0x0e0000, 0x0e0fff) AM_DEVREADWRITE8("eeprom", atari_eeprom_device, read, write, 0x00ff)
	AM_RANGE(0x16cc00, 0x16cc01) AM_RAM AM_SHARE("sync_data")
	AM_RANGE(0x160000, 0x16ffff) AM_RAM AM_SHARE("share1")
	AM_RANGE(0x1f0000, 0x1fffff) AM_DEVWRITE("eeprom", atari_eeprom_device, unlock_write)
	AM_RANGE(0x260000, 0x26000f) AM_READ_PORT("260000")
	AM_RANGE(0x260010, 0x26001f) AM_READ(special_port1_r)
	AM_RANGE(0x260020, 0x26002f) AM_READ(adc_r)
	AM_RANGE(0x260030, 0x260031) AM_DEVREAD8("jsa", atari_jsa_base_device, main_response_r, 0x00ff)
	AM_RANGE(0x2e0000, 0x2e0001) AM_WRITE(watchdog_reset16_w)
	AM_RANGE(0x360000, 0x360001) AM_WRITE(video_int_ack_w)
	AM_RANGE(0x360010, 0x360011) AM_WRITE(eprom_latch_w)
	AM_RANGE(0x360020, 0x360021) AM_DEVWRITE("jsa", atari_jsa_base_device, sound_reset_w)
	AM_RANGE(0x360030, 0x360031) AM_DEVWRITE8("jsa", atari_jsa_base_device, main_command_w, 0x00ff)
	AM_RANGE(0x3e0000, 0x3e0fff) AM_RAM AM_SHARE("paletteram")
	AM_RANGE(0x3f0000, 0x3f1fff) AM_DEVWRITE("playfield", tilemap_device, write) AM_SHARE("playfield")
	AM_RANGE(0x3f2000, 0x3f3fff) AM_RAM AM_SHARE("mob")
	AM_RANGE(0x3f4000, 0x3f4f7f) AM_DEVWRITE("alpha", tilemap_device, write) AM_SHARE("alpha")
	AM_RANGE(0x3f4f80, 0x3f4fff) AM_RAM AM_SHARE("mob:slip")
	AM_RANGE(0x3f8000, 0x3f9fff) AM_DEVWRITE("playfield", tilemap_device, write_ext) AM_SHARE("playfield_ext")
	AM_RANGE(0x3f0000, 0x3f9fff) AM_RAM
ADDRESS_MAP_END


static ADDRESS_MAP_START( guts_map, AS_PROGRAM, 16, eprom_state )
	AM_RANGE(0x000000, 0x09ffff) AM_ROM
	AM_RANGE(0x0e0000, 0x0e0fff) AM_DEVREADWRITE8("eeprom", atari_eeprom_device, read, write, 0x00ff)
	AM_RANGE(0x16cc00, 0x16cc01) AM_RAM AM_SHARE("sync_data")
	AM_RANGE(0x160000, 0x16ffff) AM_RAM AM_SHARE("share1")
	AM_RANGE(0x1f0000, 0x1fffff) AM_DEVWRITE("eeprom", atari_eeprom_device, unlock_write)
	AM_RANGE(0x260000, 0x26000f) AM_READ_PORT("260000")
	AM_RANGE(0x260010, 0x26001f) AM_READ(special_port1_r)
	AM_RANGE(0x260020, 0x26002f) AM_READ(adc_r)
	AM_RANGE(0x260030, 0x260031) AM_DEVREAD8("jsa", atari_jsa_ii_device, main_response_r, 0x00ff)
	AM_RANGE(0x2e0000, 0x2e0001) AM_WRITE(watchdog_reset16_w)
	AM_RANGE(0x360000, 0x360001) AM_WRITE(video_int_ack_w)
//  AM_RANGE(0x360010, 0x360011) AM_WRITE(eprom_latch_w)
	AM_RANGE(0x360020, 0x360021) AM_DEVWRITE("jsa", atari_jsa_ii_device, sound_reset_w)
	AM_RANGE(0x360030, 0x360031) AM_DEVWRITE8("jsa", atari_jsa_ii_device, main_command_w, 0x00ff)
	AM_RANGE(0x3e0000, 0x3e0fff) AM_RAM AM_SHARE("paletteram")
	AM_RANGE(0xff0000, 0xff1fff) AM_DEVWRITE("playfield", tilemap_device, write_ext) AM_SHARE("playfield_ext")
	AM_RANGE(0xff8000, 0xff9fff) AM_DEVWRITE("playfield", tilemap_device, write) AM_SHARE("playfield")
	AM_RANGE(0xffa000, 0xffbfff) AM_RAM AM_SHARE("mob")
	AM_RANGE(0xffc000, 0xffcf7f) AM_DEVWRITE("alpha", tilemap_device, write) AM_SHARE("alpha")
	AM_RANGE(0xffcf80, 0xffcfff) AM_RAM AM_SHARE("mob:slip")
	AM_RANGE(0xff0000, 0xff1fff) AM_RAM
	AM_RANGE(0xff8000, 0xffffff) AM_RAM
ADDRESS_MAP_END



/*************************************
 *
 *  Extra CPU memory handlers
 *
 *************************************/

static ADDRESS_MAP_START( extra_map, AS_PROGRAM, 16, eprom_state )
	AM_RANGE(0x000000, 0x07ffff) AM_ROM
	AM_RANGE(0x16cc00, 0x16cc01) AM_READWRITE(sync_r, sync_w) AM_SHARE("sync_data")
	AM_RANGE(0x160000, 0x16ffff) AM_RAM AM_SHARE("share1")
	AM_RANGE(0x260000, 0x26000f) AM_READ_PORT("260000")
	AM_RANGE(0x260010, 0x26001f) AM_READ(special_port1_r)
	AM_RANGE(0x260020, 0x26002f) AM_READ(adc_r)
	AM_RANGE(0x260030, 0x260031) AM_DEVREAD8("jsa", atari_jsa_base_device, main_response_r, 0x00ff)
	AM_RANGE(0x360000, 0x360001) AM_WRITE(video_int_ack_w)
	AM_RANGE(0x360010, 0x360011) AM_WRITE(eprom_latch_w)
	AM_RANGE(0x360020, 0x360021) AM_DEVWRITE("jsa", atari_jsa_base_device, sound_reset_w)
	AM_RANGE(0x360030, 0x360031) AM_DEVWRITE8("jsa", atari_jsa_base_device, main_command_w, 0x00ff)
ADDRESS_MAP_END



/*************************************
 *
 *  Port definitions
 *
 *************************************/

static INPUT_PORTS_START( eprom )
	PORT_START("260000")
	PORT_BIT( 0x00ff, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0xf000, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("260010")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_VBLANK("screen")
	PORT_SERVICE( 0x0002, IP_ACTIVE_LOW )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_ATARI_JSA_SOUND_TO_MAIN_READY("jsa") /* Input buffer full (@260030) */
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_ATARI_JSA_MAIN_TO_SOUND_READY("jsa") /* Output buffer full (@360030) */
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_UNUSED ) /* ADEOC, end of conversion */
	PORT_BIT( 0x00e0, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0xf000, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("ADC0")          /* ADC0 @ 0x260020 */
	PORT_BIT( 0x00ff, 0x0080, IPT_AD_STICK_Y ) PORT_MINMAX(0x10,0xf0) PORT_SENSITIVITY(100) PORT_KEYDELTA(10) PORT_PLAYER(1)
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("ADC1")          /* ADC1 @ 0x260022 */
	PORT_BIT( 0x00ff, 0x0080, IPT_AD_STICK_X ) PORT_MINMAX(0x10,0xf0) PORT_SENSITIVITY(100) PORT_KEYDELTA(10) PORT_REVERSE PORT_PLAYER(1)
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("ADC2")          /* ADC0 @ 0x260024 */
	PORT_BIT( 0x00ff, 0x0080, IPT_AD_STICK_Y ) PORT_MINMAX(0x10,0xf0) PORT_SENSITIVITY(100) PORT_KEYDELTA(10) PORT_PLAYER(2)
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("ADC3")          /* ADC1 @ 0x260026 */
	PORT_BIT( 0x00ff, 0x0080, IPT_AD_STICK_X ) PORT_MINMAX(0x10,0xf0) PORT_SENSITIVITY(100) PORT_KEYDELTA(10) PORT_REVERSE PORT_PLAYER(2)
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END


static INPUT_PORTS_START( klaxp )
	PORT_START("260000")
	PORT_BIT( 0x00ff, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(1)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1)
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(1)

	PORT_START("260010")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_VBLANK("screen")
	PORT_SERVICE( 0x0002, IP_ACTIVE_LOW )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_ATARI_JSA_SOUND_TO_MAIN_READY("jsa") /* Input buffer full (@260030) */
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_ATARI_JSA_MAIN_TO_SOUND_READY("jsa") /* Output buffer full (@360030) */
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_UNUSED ) /* ADEOC, end of conversion */
	PORT_BIT( 0x00e0, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(2)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2)
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(2)
INPUT_PORTS_END


static INPUT_PORTS_START( guts )
	PORT_START("260000")        /* 260000 */
	PORT_BIT( 0x00ff, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0xf000, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("260010")        /* 260010 */
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_VBLANK("screen")
	PORT_SERVICE( 0x0002, IP_ACTIVE_LOW )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_ATARI_JSA_SOUND_TO_MAIN_READY("jsa") /* Input buffer full (@260030) */
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_ATARI_JSA_MAIN_TO_SOUND_READY("jsa") /* Output buffer full (@360030) */
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_UNUSED ) /* ADEOC, end of conversion */
	PORT_BIT( 0x00e0, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0xf000, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("ADC0")          /* ADC0 @ 0x260020 */
	PORT_BIT( 0x00ff, 0x0080, IPT_AD_STICK_Y ) PORT_MINMAX(0x10,0xf0) PORT_SENSITIVITY(100) PORT_KEYDELTA(10) PORT_PLAYER(1)
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("ADC1")          /* ADC1 @ 0x260022 */
	PORT_BIT( 0x00ff, 0x0080, IPT_AD_STICK_X ) PORT_MINMAX(0x10,0xf0) PORT_SENSITIVITY(100) PORT_KEYDELTA(10) PORT_PLAYER(1) PORT_REVERSE
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("ADC2")          /* ADC0 @ 0x260024 */
	PORT_BIT( 0x00ff, 0x0080, IPT_AD_STICK_Y ) PORT_MINMAX(0x10,0xf0) PORT_SENSITIVITY(100) PORT_KEYDELTA(10) PORT_PLAYER(2)
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("ADC3")          /* ADC1 @ 0x260026 */
	PORT_BIT( 0x00ff, 0x0080, IPT_AD_STICK_X ) PORT_MINMAX(0x10,0xf0) PORT_SENSITIVITY(100) PORT_KEYDELTA(10) PORT_PLAYER(2) PORT_REVERSE
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END



/*************************************
 *
 *  Graphics definitions
 *
 *************************************/

static const gfx_layout anlayout =
{
	8,8,
	RGN_FRAC(1,1),
	2,
	{ 0, 4 },
	{ STEP4(0,1), STEP4(8,1) },
	{ STEP8(0,16) },
	8*16
};


static const gfx_layout pfmolayout =
{
	8,8,
	RGN_FRAC(1,4),
	4,
	{ RGN_FRAC(0,4), RGN_FRAC(1,4), RGN_FRAC(2,4), RGN_FRAC(3,4) },
	{ STEP8(0,1) },
	{ STEP8(0,8) },
	8*8
};


static GFXDECODE_START( eprom )
	GFXDECODE_ENTRY( "gfx1", 0, pfmolayout,  256, 32 )  /* sprites & playfield */
	GFXDECODE_ENTRY( "gfx2", 0, anlayout,      0, 64 )  /* characters 8x8 */
GFXDECODE_END


static GFXDECODE_START( guts )
	GFXDECODE_ENTRY( "gfx1", 0, pfmolayout,  256, 32 )  /* sprites */
	GFXDECODE_ENTRY( "gfx2", 0, anlayout,      0, 64 )  /* characters 8x8 */
	GFXDECODE_ENTRY( "gfx3", 0, pfmolayout,  256, 32 )  /* playfield */
GFXDECODE_END



/*************************************
 *
 *  Machine drivers
 *
 *************************************/

static MACHINE_CONFIG_START( eprom, eprom_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68000, ATARI_CLOCK_14MHz/2)
	MCFG_CPU_PROGRAM_MAP(main_map)
	MCFG_DEVICE_VBLANK_INT_DRIVER("screen", atarigen_state, video_int_gen)

	MCFG_CPU_ADD("extra", M68000, ATARI_CLOCK_14MHz/2)
	MCFG_CPU_PROGRAM_MAP(extra_map)

	MCFG_QUANTUM_TIME(attotime::from_hz(6000))

	MCFG_MACHINE_RESET_OVERRIDE(eprom_state,eprom)

	MCFG_ATARI_EEPROM_2804_ADD("eeprom")

	/* video hardware */
	MCFG_GFXDECODE_ADD("gfxdecode", "palette", eprom)
	MCFG_PALETTE_ADD("palette", 2048)

	MCFG_TILEMAP_ADD_STANDARD("playfield", "gfxdecode", 2, eprom_state, get_playfield_tile_info, 8,8, SCAN_COLS, 64,64)
	MCFG_TILEMAP_ADD_STANDARD_TRANSPEN("alpha", "gfxdecode", 2, eprom_state, get_alpha_tile_info, 8,8, SCAN_ROWS, 64,32, 0)
	MCFG_ATARI_MOTION_OBJECTS_ADD("mob", "screen", eprom_state::s_mob_config)
	MCFG_ATARI_MOTION_OBJECTS_GFXDECODE("gfxdecode")

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_VIDEO_ATTRIBUTES(VIDEO_UPDATE_BEFORE_VBLANK)
	/* note: these parameters are from published specs, not derived */
	/* the board uses a SYNGEN chip to generate video signals */
	MCFG_SCREEN_RAW_PARAMS(ATARI_CLOCK_14MHz/2, 456, 0, 336, 262, 0, 240)
	MCFG_SCREEN_UPDATE_DRIVER(eprom_state, screen_update_eprom)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_VIDEO_START_OVERRIDE(eprom_state,eprom)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_ATARI_JSA_I_ADD("jsa", WRITELINE(atarigen_state, sound_int_write_line))
	MCFG_ATARI_JSA_TEST_PORT("260010", 1)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
	MCFG_DEVICE_REMOVE("jsa:pokey")
MACHINE_CONFIG_END


static MACHINE_CONFIG_START( klaxp, eprom_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68000, ATARI_CLOCK_14MHz/2)
	MCFG_CPU_PROGRAM_MAP(main_map)
	MCFG_DEVICE_VBLANK_INT_DRIVER("screen", atarigen_state, video_int_gen)

	MCFG_QUANTUM_TIME(attotime::from_hz(600))

	MCFG_MACHINE_RESET_OVERRIDE(eprom_state,eprom)

	MCFG_ATARI_EEPROM_2804_ADD("eeprom")

	/* video hardware */
	MCFG_GFXDECODE_ADD("gfxdecode", "palette", eprom)
	MCFG_PALETTE_ADD("palette", 2048)

	MCFG_TILEMAP_ADD_STANDARD("playfield", "gfxdecode", 2, eprom_state, get_playfield_tile_info, 8,8, SCAN_COLS, 64,64)
	MCFG_TILEMAP_ADD_STANDARD_TRANSPEN("alpha", "gfxdecode", 2, eprom_state, get_alpha_tile_info, 8,8, SCAN_ROWS, 64,32, 0)
	MCFG_ATARI_MOTION_OBJECTS_ADD("mob", "screen", eprom_state::s_mob_config)
	MCFG_ATARI_MOTION_OBJECTS_GFXDECODE("gfxdecode")

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_VIDEO_ATTRIBUTES(VIDEO_UPDATE_BEFORE_VBLANK)
	/* note: these parameters are from published specs, not derived */
	/* the board uses a SYNGEN chip to generate video signals */
	MCFG_SCREEN_RAW_PARAMS(ATARI_CLOCK_14MHz/2, 456, 0, 336, 262, 0, 240)
	MCFG_SCREEN_UPDATE_DRIVER(eprom_state, screen_update_eprom)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_VIDEO_START_OVERRIDE(eprom_state,eprom)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_ATARI_JSA_II_ADD("jsa", WRITELINE(atarigen_state, sound_int_write_line))
	MCFG_ATARI_JSA_TEST_PORT("260010", 1)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_CONFIG_END


static MACHINE_CONFIG_START( guts, eprom_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68000, ATARI_CLOCK_14MHz/2)
	MCFG_CPU_PROGRAM_MAP(guts_map)
	MCFG_DEVICE_VBLANK_INT_DRIVER("screen", atarigen_state, video_int_gen)

	MCFG_QUANTUM_TIME(attotime::from_hz(600))

	MCFG_MACHINE_RESET_OVERRIDE(eprom_state,eprom)

	MCFG_ATARI_EEPROM_2804_ADD("eeprom")

	/* video hardware */
	MCFG_GFXDECODE_ADD("gfxdecode", "palette", guts)
	MCFG_PALETTE_ADD("palette", 2048)

	MCFG_TILEMAP_ADD_STANDARD("playfield", "gfxdecode", 2, eprom_state, guts_get_playfield_tile_info, 8,8, SCAN_COLS, 64,64)
	MCFG_TILEMAP_ADD_STANDARD_TRANSPEN("alpha", "gfxdecode", 2, eprom_state, get_alpha_tile_info, 8,8, SCAN_ROWS, 64,32, 0)
	MCFG_ATARI_MOTION_OBJECTS_ADD("mob", "screen", eprom_state::s_guts_mob_config)
	MCFG_ATARI_MOTION_OBJECTS_GFXDECODE("gfxdecode")

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_VIDEO_ATTRIBUTES(VIDEO_UPDATE_BEFORE_VBLANK)
	/* note: these parameters are from published specs, not derived */
	/* the board uses a SYNGEN chip to generate video signals */
	MCFG_SCREEN_RAW_PARAMS(ATARI_CLOCK_14MHz/2, 456, 0, 336, 262, 0, 240)
	MCFG_SCREEN_UPDATE_DRIVER(eprom_state, screen_update_guts)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_VIDEO_START_OVERRIDE(eprom_state,guts)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_ATARI_JSA_II_ADD("jsa", WRITELINE(atarigen_state, sound_int_write_line))
	MCFG_ATARI_JSA_TEST_PORT("260010", 1)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_CONFIG_END



/*************************************
 *
 *  ROM definition(s)
 *
 *************************************/

ROM_START( eprom )
	ROM_REGION( 0xa0000, "maincpu", 0 ) /* 10*64k for 68000 code */
	ROM_LOAD16_BYTE( "136069-3025.50a",   0x00000, 0x10000, CRC(08888dec) SHA1(a0a137828b9e1efbdbc0e5ddaf4d73d24b36948a) )
	ROM_LOAD16_BYTE( "136069-3024.40a",   0x00001, 0x10000, CRC(29cb1e97) SHA1(ccf7024dccbd61983d61450f15c805422e4eee09) )
	ROM_LOAD16_BYTE( "136069-4027.50b",   0x20000, 0x10000, CRC(702241c9) SHA1(cba27e92f64fd201c16aed6a8f2dc64c4f887e4f) )
	ROM_LOAD16_BYTE( "136069-4026.40b",   0x20001, 0x10000, CRC(fecbf9e2) SHA1(cd06bfab296e9496564fc2716b26874b55dc2188) )
	ROM_LOAD16_BYTE( "136069-4029.50d",   0x40000, 0x10000, CRC(0f2f1502) SHA1(2aa65c03d4cd94839a2c2ba338177202bc1185ee) )
	ROM_LOAD16_BYTE( "136069-4028.40d",   0x40001, 0x10000, CRC(bc6f6ae8) SHA1(43a947cf9db7cda825924e167529305f63bb2a5c) )
	ROM_LOAD16_BYTE( "136069-2033.40k",   0x60000, 0x10000, CRC(130650f6) SHA1(bea7780d54a4e1f3e93f14494c82446a4bb48e19) )
	ROM_LOAD16_BYTE( "136069-2032.50k",   0x60001, 0x10000, CRC(1da21ed8) SHA1(3b00e3cf5a25918c1f3158d8b2192158f77cb521) )

	ROM_REGION( 0x80000, "extra", 0 )   /* 8*64k for 68000 code */
	ROM_LOAD16_BYTE( "136069-2035.10s",   0x00000, 0x10000, CRC(deff6469) SHA1(2fe4d42d60965581579e8edad49b86fbd321d1db) )
	ROM_LOAD16_BYTE( "136069-2034.10u",   0x00001, 0x10000, CRC(5d7afca2) SHA1(a37ecd2909049dd0b3ddbe602f0173c44b065f6f) )
	ROM_COPY( "maincpu", 0x60000,  0x60000, 0x20000 )

	ROM_REGION( 0x14000, "jsa:cpu", 0 ) /* 64k + 16k for 6502 code */
	ROM_LOAD( "136069-1040.7b",    0x10000, 0x4000, CRC(86e93695) SHA1(63ddab02df139dd41a8260c303798b2a550b9fe6) )
	ROM_CONTINUE(             0x04000, 0xc000 )

	ROM_REGION( 0x100000, "gfx1", ROMREGION_INVERT )
	ROM_LOAD( "136069-1020.47s",   0x00000, 0x10000, CRC(0de9d98d) SHA1(c2f963a8a4573e135a2825929cbc5535ce3b0215) )
	ROM_LOAD( "136069-1013.43s",   0x10000, 0x10000, CRC(8eb106ad) SHA1(ece0ddba8fafe6e720f843c4d3f69ae654ae9d92) )
	ROM_LOAD( "136069-1018.38s",   0x20000, 0x10000, CRC(bf3d0e18) SHA1(c81dacd06ce2580e37ff480d1182ab6c7e74d600) )
	ROM_LOAD( "136069-1023.32s",   0x30000, 0x10000, CRC(48fb2e42) SHA1(480edc87f7ca547c3d8e09bf1a98e04ac464f4c6) )
	ROM_LOAD( "136069-1016.76s",   0x40000, 0x10000, CRC(602d939d) SHA1(2ce9899f4cf0786df8c5f0e8cc63ce5206ea514f) )
	ROM_LOAD( "136069-1011.70s",   0x50000, 0x10000, CRC(f6c973af) SHA1(048d5a9b89cb83186ca594e71521675248970735) )
	ROM_LOAD( "136069-1017.64s",   0x60000, 0x10000, CRC(9cd52e30) SHA1(59233a87f2b50e9390f297abe7489864222f98e2) )
	ROM_LOAD( "136069-1022.57s",   0x70000, 0x10000, CRC(4e2c2e7e) SHA1(6bf203e8c029d955634dcbaef9a6932d42035b25) )
	ROM_LOAD( "136069-1012.47u",   0x80000, 0x10000, CRC(e7edcced) SHA1(4c19ea8b15332681bfc73a3d2b063985c1bbac1d) )
	ROM_LOAD( "136069-1010.43u",   0x90000, 0x10000, CRC(9d3e144d) SHA1(7f4c7ee14d10a733f8b4169b41023bda1b5702c8) )
	ROM_LOAD( "136069-1015.38u",   0xa0000, 0x10000, CRC(23f40437) SHA1(567aa09a986dd8765c54f413f906e1cb323568c6) )
	ROM_LOAD( "136069-1021.32u",   0xb0000, 0x10000, CRC(2a47ff7b) SHA1(89935eac8fbeed87668fe1dcb4645c96a9df2c03) )
	ROM_LOAD( "136069-1008.76u",   0xc0000, 0x10000, CRC(b0cead58) SHA1(b50b0125bedc1740d02c50e0547a2d2e25b2c42e) )
	ROM_LOAD( "136069-1009.70u",   0xd0000, 0x10000, CRC(fbc3934b) SHA1(08c581359a005df4d63fa07733bb343c5ab653a9) )
	ROM_LOAD( "136069-1014.64u",   0xe0000, 0x10000, CRC(0e07493b) SHA1(c5839ac4824b6fedb5397779cd30f6b1eff962d5) )
	ROM_LOAD( "136069-1019.57u",   0xf0000, 0x10000, CRC(34f8f0ed) SHA1(9096aa2a188a15c2e78acf48d33def0c9f2a419f) )

	ROM_REGION( 0x04000, "gfx2", 0 )
	ROM_LOAD( "136069-1007.125d",  0x00000, 0x04000, CRC(409d818e) SHA1(63dcde3ce87c1a9d5afef8089432c499cc70f8f0) )

	ROM_REGION( 0x0c00, "plds", 0 )
	ROM_LOAD( "gal16v8-136069.100t",  0x0000, 0x0117, CRC(fd9d472e) SHA1(ba2357e355d062f3ce8bdbed59856e28130999d0) )
	ROM_LOAD( "gal16v8-136069.100v",  0x0200, 0x0117, CRC(cd472121) SHA1(634354363866af39e0d69cbe7c5e80abbd428cec) )
	ROM_LOAD( "gal16v8-136069.50f",   0x0400, 0x0117, CRC(db013b25) SHA1(8090ad125cc48b9bd3070bc38003392ef561da58) )
	ROM_LOAD( "gal16v8-136069.50p",   0x0600, 0x0117, CRC(4a765b00) SHA1(6af38b44890b630b6bb8f51d56c202b5b2558969) )
	ROM_LOAD( "gal16v8-136069.55p",   0x0800, 0x0117, CRC(48abc939) SHA1(54612ffe6fc27d0e1cb401931e6b9636cef1130e) )
	ROM_LOAD( "gal16v8-136069.70j",   0x0a00, 0x0117, CRC(3b4ebe41) SHA1(5d8e550500bddc8cc06e0240bcc81737a75bf5af) )
ROM_END


ROM_START( eprom2 )
	ROM_REGION( 0xa0000, "maincpu", 0 ) /* 10*64k for 68000 code */
	ROM_LOAD16_BYTE( "136069-1025.50a",   0x00000, 0x10000, CRC(b0c9a476) SHA1(6d0edeeb9458e92191f6623307eddc9b2f830d4d) )
	ROM_LOAD16_BYTE( "136069-1024.40a",   0x00001, 0x10000, CRC(4cc2c50c) SHA1(088908cc57b07d71a5d664674e38fa02c55bb4fc) )
	ROM_LOAD16_BYTE( "136069-1027.50b",   0x20000, 0x10000, CRC(84f533ea) SHA1(c1da671be5149bff26acd19b14cd18db0df695b7) )
	ROM_LOAD16_BYTE( "136069-1026.40b",   0x20001, 0x10000, CRC(506396ce) SHA1(9457d346ab3aabec17f2c9ea32b9058aabdce831) )
	ROM_LOAD16_BYTE( "136069-1029.50d",   0x40000, 0x10000, CRC(99810b9b) SHA1(f744afa559798e58b0d7ad5c7f02746e5ef94524) )
	ROM_LOAD16_BYTE( "136069-1028.40d",   0x40001, 0x10000, CRC(08ab41f2) SHA1(1801c01efbeca64c1beecc9ca31ec12e02000a6c) )
	ROM_LOAD16_BYTE( "136069-1033.40k",   0x60000, 0x10000, CRC(395fc203) SHA1(5f5ceb286f5e4efd88c9a9368b0486da9f318365) )
	ROM_LOAD16_BYTE( "136069-1032.50k",   0x60001, 0x10000, CRC(a19c8acb) SHA1(77405d1e9ca82f7967ea7e54ffa81b74d81f5b56) )
	ROM_LOAD16_BYTE( "136069-1037.50e",   0x80000, 0x10000, CRC(ad39a3dd) SHA1(00dcdcb30b7f8441df4216f9be4de15791ac5fc8) )
	ROM_LOAD16_BYTE( "136069-1036.40e",   0x80001, 0x10000, CRC(34fc8895) SHA1(0c167c3a778e064a37517b52fd7a52f16d844f77) )

	ROM_REGION( 0x80000, "extra", 0 )   /* 8*64k for 68000 code */
	ROM_LOAD16_BYTE( "136069-1035.10s",    0x00000, 0x10000, CRC(ffeb5647) SHA1(fbd9217a96e51dd0c0cbc0ba9dfdaaa36fbc1ae9) )
	ROM_LOAD16_BYTE( "136069-1034.10u",    0x00001, 0x10000, CRC(c68f58dd) SHA1(0ec300f32e67b710ac33efb60b8eccceb43faca6) )
	ROM_COPY( "maincpu", 0x60000, 0x60000, 0x20000 )

	ROM_REGION( 0x14000, "jsa:cpu", 0 ) /* 64k + 16k for 6502 code */
	ROM_LOAD( "136069-1040.7b",    0x10000, 0x4000, CRC(86e93695) SHA1(63ddab02df139dd41a8260c303798b2a550b9fe6) )
	ROM_CONTINUE(             0x04000, 0xc000 )

	ROM_REGION( 0x100000, "gfx1", ROMREGION_INVERT )
	ROM_LOAD( "136069-1020.47s",   0x00000, 0x10000, CRC(0de9d98d) SHA1(c2f963a8a4573e135a2825929cbc5535ce3b0215) )
	ROM_LOAD( "136069-1013.43s",   0x10000, 0x10000, CRC(8eb106ad) SHA1(ece0ddba8fafe6e720f843c4d3f69ae654ae9d92) )
	ROM_LOAD( "136069-1018.38s",   0x20000, 0x10000, CRC(bf3d0e18) SHA1(c81dacd06ce2580e37ff480d1182ab6c7e74d600) )
	ROM_LOAD( "136069-1023.32s",   0x30000, 0x10000, CRC(48fb2e42) SHA1(480edc87f7ca547c3d8e09bf1a98e04ac464f4c6) )
	ROM_LOAD( "136069-1016.76s",   0x40000, 0x10000, CRC(602d939d) SHA1(2ce9899f4cf0786df8c5f0e8cc63ce5206ea514f) )
	ROM_LOAD( "136069-1011.70s",   0x50000, 0x10000, CRC(f6c973af) SHA1(048d5a9b89cb83186ca594e71521675248970735) )
	ROM_LOAD( "136069-1017.64s",   0x60000, 0x10000, CRC(9cd52e30) SHA1(59233a87f2b50e9390f297abe7489864222f98e2) )
	ROM_LOAD( "136069-1022.57s",   0x70000, 0x10000, CRC(4e2c2e7e) SHA1(6bf203e8c029d955634dcbaef9a6932d42035b25) )
	ROM_LOAD( "136069-1012.47u",   0x80000, 0x10000, CRC(e7edcced) SHA1(4c19ea8b15332681bfc73a3d2b063985c1bbac1d) )
	ROM_LOAD( "136069-1010.43u",   0x90000, 0x10000, CRC(9d3e144d) SHA1(7f4c7ee14d10a733f8b4169b41023bda1b5702c8) )
	ROM_LOAD( "136069-1015.38u",   0xa0000, 0x10000, CRC(23f40437) SHA1(567aa09a986dd8765c54f413f906e1cb323568c6) )
	ROM_LOAD( "136069-1021.32u",   0xb0000, 0x10000, CRC(2a47ff7b) SHA1(89935eac8fbeed87668fe1dcb4645c96a9df2c03) )
	ROM_LOAD( "136069-1008.76u",   0xc0000, 0x10000, CRC(b0cead58) SHA1(b50b0125bedc1740d02c50e0547a2d2e25b2c42e) )
	ROM_LOAD( "136069-1009.70u",   0xd0000, 0x10000, CRC(fbc3934b) SHA1(08c581359a005df4d63fa07733bb343c5ab653a9) )
	ROM_LOAD( "136069-1014.64u",   0xe0000, 0x10000, CRC(0e07493b) SHA1(c5839ac4824b6fedb5397779cd30f6b1eff962d5) )
	ROM_LOAD( "136069-1019.57u",   0xf0000, 0x10000, CRC(34f8f0ed) SHA1(9096aa2a188a15c2e78acf48d33def0c9f2a419f) )

	ROM_REGION( 0x04000, "gfx2", 0 )
	ROM_LOAD( "136069.125d",  0x00000, 0x04000, CRC(409d818e) SHA1(63dcde3ce87c1a9d5afef8089432c499cc70f8f0) )

	ROM_REGION( 0x0c00, "plds", 0 )
	ROM_LOAD( "gal16v8-136069.100t",  0x0000, 0x0117, CRC(fd9d472e) SHA1(ba2357e355d062f3ce8bdbed59856e28130999d0) )
	ROM_LOAD( "gal16v8-136069.100v",  0x0200, 0x0117, CRC(cd472121) SHA1(634354363866af39e0d69cbe7c5e80abbd428cec) )
	ROM_LOAD( "gal16v8-136069.50f",   0x0400, 0x0117, CRC(db013b25) SHA1(8090ad125cc48b9bd3070bc38003392ef561da58) )
	ROM_LOAD( "gal16v8-136069.50p",   0x0600, 0x0117, CRC(4a765b00) SHA1(6af38b44890b630b6bb8f51d56c202b5b2558969) )
	ROM_LOAD( "gal16v8-136069.55p",   0x0800, 0x0117, CRC(48abc939) SHA1(54612ffe6fc27d0e1cb401931e6b9636cef1130e) )
	ROM_LOAD( "gal16v8-136069.70j",   0x0a00, 0x0117, CRC(3b4ebe41) SHA1(5d8e550500bddc8cc06e0240bcc81737a75bf5af) )
ROM_END


ROM_START( klaxp1 )
	ROM_REGION( 0xa0000, "maincpu", 0 ) /* 10*64k for 68000 code */
	ROM_LOAD16_BYTE( "klax_ft1.50a",   0x00000, 0x10000, CRC(87ee72d1) SHA1(39ae6f8406f0768480bcc80d395a14d9c2c65dca) )
	ROM_LOAD16_BYTE( "klax_ft1.40a",   0x00001, 0x10000, CRC(ba139fdb) SHA1(98a8ac5e0349b934f55d0d9de85abacd3fd0d77d) )

	ROM_REGION( 0x14000, "jsa:cpu", 0 ) /* 64k + 16k for 6502 code */
	ROM_LOAD( "klaxsnd.10c",  0x10000, 0x4000, CRC(744734cb) SHA1(3630428d69ddd2a4d5dd76bb4ee9485c943129e9) )
	ROM_CONTINUE(             0x04000, 0xc000 )

	ROM_REGION( 0x40000, "gfx1", ROMREGION_INVERT )
	ROM_LOAD( "klaxprot.43s",   0x00000, 0x10000, CRC(a523c966) SHA1(8e284901cd1c68b25aa9dec1c87374b93cceeb40) )
	ROM_LOAD( "klaxprot.76s",   0x10000, 0x10000, CRC(dbc678cd) SHA1(4e6db153d29300e8d5960937d3bfebbd1ae2e78a) )
	ROM_LOAD( "klaxprot.47u",   0x20000, 0x10000, CRC(af184754) SHA1(4567337e1af1f748b1663e0b4c3e8ea746aac56c) )
	ROM_LOAD( "klaxprot.76u",   0x30000, 0x10000, CRC(7a56ffab) SHA1(96c491e51931c6641e63e55da173ecd41df7c7b3) )

	ROM_REGION( 0x04000, "gfx2", 0 )
	ROM_LOAD( "klax125d",  0x00000, 0x04000, CRC(409d818e) SHA1(63dcde3ce87c1a9d5afef8089432c499cc70f8f0) )

	ROM_REGION( 0x40000, "jsa:oki1", 0 )   /* ADPCM data */
	ROM_LOAD( "klaxadp0.1f", 0x00000, 0x10000, CRC(ba1e864f) SHA1(7c45e9040701b54c8be398c6e5cdf9201dc37c17) )
	ROM_LOAD( "klaxadp1.1e", 0x10000, 0x10000, CRC(dec9a5ac) SHA1(8039d946ac3613fa6193b557cc8775c81871831d) )
ROM_END


ROM_START( klaxp2 )
	ROM_REGION( 0xa0000, "maincpu", 0 ) /* 10*64k for 68000 code */
	ROM_LOAD16_BYTE( "klax_ft2.50a",   0x00000, 0x10000, CRC(7d401937) SHA1(8db0560528a86b9cb01c4598a49694bd44b00dba) )
	ROM_LOAD16_BYTE( "klax_ft2.40a",   0x00001, 0x10000, CRC(c5ca33a9) SHA1(c2e2948f987ba43f61c043baed06ffea8787be43) )

	ROM_REGION( 0x14000, "jsa:cpu", 0 ) /* 64k + 16k for 6502 code */
	ROM_LOAD( "klaxsnd.10c",  0x10000, 0x4000, CRC(744734cb) SHA1(3630428d69ddd2a4d5dd76bb4ee9485c943129e9) )
	ROM_CONTINUE(             0x04000, 0xc000 )

	ROM_REGION( 0x40000, "gfx1", ROMREGION_INVERT )
	ROM_LOAD( "klaxprot.43s",   0x00000, 0x10000, CRC(a523c966) SHA1(8e284901cd1c68b25aa9dec1c87374b93cceeb40) )
	ROM_LOAD( "klaxprot.76s",   0x10000, 0x10000, CRC(dbc678cd) SHA1(4e6db153d29300e8d5960937d3bfebbd1ae2e78a) )
	ROM_LOAD( "klaxprot.47u",   0x20000, 0x10000, CRC(af184754) SHA1(4567337e1af1f748b1663e0b4c3e8ea746aac56c) )
	ROM_LOAD( "klaxprot.76u",   0x30000, 0x10000, CRC(7a56ffab) SHA1(96c491e51931c6641e63e55da173ecd41df7c7b3) )

	ROM_REGION( 0x04000, "gfx2", 0 )
	ROM_LOAD( "klax125d",  0x00000, 0x04000, CRC(409d818e) SHA1(63dcde3ce87c1a9d5afef8089432c499cc70f8f0) )

	ROM_REGION( 0x40000, "jsa:oki1", 0 )   /* ADPCM data */
	ROM_LOAD( "klaxadp0.1f", 0x00000, 0x10000, CRC(ba1e864f) SHA1(7c45e9040701b54c8be398c6e5cdf9201dc37c17) )
	ROM_LOAD( "klaxadp1.1e", 0x10000, 0x10000, CRC(dec9a5ac) SHA1(8039d946ac3613fa6193b557cc8775c81871831d) )
ROM_END


ROM_START( guts )
	ROM_REGION( 0xa0000, "maincpu", 0 ) /* 10*64k for 68000 code */
	ROM_LOAD16_BYTE( "guts-hi0.50a", 0x00000, 0x10000, CRC(3afca24a) SHA1(4910c958ac2124de13d4069420fb2cfd18b12cec) )
	ROM_LOAD16_BYTE( "guts-lo0.40a", 0x00001, 0x10000, CRC(ce86cf23) SHA1(28504e2e8dcf1eaa96364eed1faf00fec9e98788) )
	ROM_LOAD16_BYTE( "guts-hi1.50b", 0x20000, 0x10000, CRC(a231f65d) SHA1(9c8ccd265ed0e9f6d7181d216ed41a0c5cc0cd5f) )
	ROM_LOAD16_BYTE( "guts-lo1.40b", 0x20001, 0x10000, CRC(dbdd4910) SHA1(9ca22321398b6397902aa99a3ef46f1a78ccc438) )

	ROM_REGION( 0x14000, "jsa:cpu", 0 ) /* 64k + 16k for 6502 code */
	ROM_LOAD( "guts-snd.10c", 0x10000, 0x4000, CRC(9fe065d7) SHA1(0d202af3d6c62fdcfc3bb2ea95bbf4e37c0d43cf) )
	ROM_CONTINUE(             0x04000, 0xc000 )

	ROM_REGION( 0x100000, "gfx1", ROMREGION_INVERT )
	ROM_LOAD( "guts-mo0.bin", 0x00000, 0x10000, CRC(b8d8d8da) SHA1(6426607402aa9f1c872290910eefc57a8dd60e17) )
	ROM_LOAD( "guts-mo1.bin", 0x10000, 0x10000, CRC(d01b5a7f) SHA1(19aead56ddb92e0b1fc78d9065b1f139fe2a5668) )
	ROM_LOAD( "guts-mo2.bin", 0x20000, 0x10000, CRC(4577b807) SHA1(df0ead177342ab360cfab9b6defb7d0129d921e4) )
	ROM_LOAD( "guts-mo3.bin", 0x30000, 0x10000, CRC(4ab03c84) SHA1(b37d4abaaa42b9b847cc928a2b40b4e58ba9887f) )
	ROM_LOAD( "guts-mo4.bin", 0x40000, 0x10000, CRC(04cae4fb) SHA1(1b2cde0a97f687f67836f9eb09a45875a81c994a) )
	ROM_LOAD( "guts-mo5.bin", 0x50000, 0x10000, CRC(c65322ec) SHA1(0e77fb3db5760e12ee7b321906c2211e60a63ea4) )
	ROM_LOAD( "guts-mo6.bin", 0x60000, 0x10000, CRC(92602a5f) SHA1(a52376d326368e989cb7c3aa3913b918b4639307) )
	ROM_LOAD( "guts-mo7.bin", 0x70000, 0x10000, CRC(71a1911d) SHA1(6fd7c8b7a340466672e9db7a44e472e8e078e469) )
	ROM_LOAD( "guts-mo8.bin", 0x80000, 0x10000, CRC(aa273234) SHA1(41c310d53b4a847323cfd1f2080653cbc19216bf) )
	ROM_LOAD( "guts-mo9.bin", 0x90000, 0x10000, CRC(e85a12ef) SHA1(cbdb19e2e075c56288b9ed6aec07f03a2f265951) )
	ROM_LOAD( "guts-moa.bin", 0xa0000, 0x10000, CRC(da1cc76f) SHA1(ba91026464fec3cd94d1625c8780d5b18ecbb6ae) )
	ROM_LOAD( "guts-mob.bin", 0xb0000, 0x10000, CRC(246e7955) SHA1(cf146be2855d0a9c28c8da926c7fa381d06d5dd4) )
	ROM_LOAD( "guts-moc.bin", 0xc0000, 0x10000, CRC(1764c272) SHA1(0682fdc20cc1cd9355a50804a3f13c913c33f52b) )
	ROM_LOAD( "guts-mod.bin", 0xd0000, 0x10000, CRC(8220f2f6) SHA1(8a2900e223c203507c11e0185c2172ddddb0f4ee) )
	ROM_LOAD( "guts-moe.bin", 0xe0000, 0x10000, CRC(ee372eac) SHA1(bb1248bb3415853e16819a7b6b64ec67f87a2c58) )
	ROM_LOAD( "guts-mof.bin", 0xf0000, 0x10000, CRC(028ec56e) SHA1(8a95cffe5c00296e4df725335046a810efab533a) )

	ROM_REGION( 0x04000, "gfx2", 0 )
	ROM_LOAD( "guts-alpha.bin", 0x00000, 0x04000, CRC(ee965058) SHA1(ccd3f6550bd5b531e8dd70ca88c30cdabf30e1e9) )

	ROM_REGION( 0x100000, "gfx3", ROMREGION_INVERT )
	ROM_LOAD( "guts-pf0.bin", 0x00000, 0x10000, CRC(1669fdb3) SHA1(d8d27b0f5877e215bf3d5343893c979860b1b45f) )
	ROM_LOAD( "guts-pf1.bin", 0x10000, 0x10000, CRC(135c41bd) SHA1(945ad9edbfcc5fe807cb15aa72b1be9e05974cb2) )
	ROM_LOAD( "guts-pf2.bin", 0x20000, 0x10000, CRC(c0408d39) SHA1(187ecea51f607c7f2565a9853c728e80118fefb1) )
	ROM_LOAD( "guts-pf4.bin", 0x40000, 0x10000, CRC(577f25a6) SHA1(3eaa3de6aa7e5b3938d34823f2cccc60e9b211e7) )
	ROM_LOAD( "guts-pf5.bin", 0x50000, 0x10000, CRC(43cbc0e3) SHA1(51e5f90391ebb402f715f389168baf401e3c03e9) )
	ROM_LOAD( "guts-pf6.bin", 0x60000, 0x10000, CRC(03c096f4) SHA1(fc0ffd5b61ab8bda37db508ec6dc5c70e1007187) )
	ROM_LOAD( "guts-pf8.bin", 0x80000, 0x10000, CRC(2f078b09) SHA1(e9b1aba767339d4677c3366d3f2df5a8deb5105e) )
	ROM_LOAD( "guts-pf9.bin", 0x90000, 0x10000, CRC(7cb7302d) SHA1(9d6ae58f1f64d1e28634dd42daedebb39570cd95) )
	ROM_LOAD( "guts-pfa.bin", 0xa0000, 0x10000, CRC(a3919dae) SHA1(0eba64afcc35e37322f9eb3a0e254026443ce092) )
	ROM_LOAD( "guts-pfc.bin", 0xc0000, 0x10000, CRC(7c571ee8) SHA1(044744ca95b2bd7486b0b057f1d7bdbd391958ab) )
	ROM_LOAD( "guts-pfd.bin", 0xd0000, 0x10000, CRC(979af5b2) SHA1(574a41552eb641668841cf01aeed442ccd3bc8e5) )
	ROM_LOAD( "guts-pfe.bin", 0xe0000, 0x10000, CRC(bf384e4d) SHA1(c4810b5a3ee754b169efa01f06941a02b50c53a0) )

	ROM_REGION( 0x40000, "jsa:oki1", 0 )   /* ADPCM data */
	ROM_LOAD( "guts-adpcm0.1f", 0x00000, 0x10000, CRC(92e9c35d) SHA1(dcc724f915e113bc34f499af9fd7c8ebb6d8ba98) )
	ROM_LOAD( "guts-adpcm1.1e", 0x10000, 0x10000, CRC(0afddd3a) SHA1(e1a43825ad02325a64869ec8048c8176da01b286) )
ROM_END



/*************************************
 *
 *  Driver initialization
 *
 *************************************/

DRIVER_INIT_MEMBER(eprom_state,eprom)
{
	/* install CPU synchronization handlers */
	m_sync_data = m_maincpu->space(AS_PROGRAM).install_readwrite_handler(0x16cc00, 0x16cc01, read16_delegate(FUNC(eprom_state::sync_r),this), write16_delegate(FUNC(eprom_state::sync_w),this));
	m_sync_data = m_extra->space(AS_PROGRAM).install_readwrite_handler(0x16cc00, 0x16cc01, read16_delegate(FUNC(eprom_state::sync_r),this), write16_delegate(FUNC(eprom_state::sync_w),this));
}


DRIVER_INIT_MEMBER(eprom_state,klaxp)
{
}


DRIVER_INIT_MEMBER(eprom_state,guts)
{
}



/*************************************
 *
 *  Game driver(s)
 *
 *************************************/

GAME( 1989, eprom,  0,     eprom, eprom, eprom_state, eprom, ROT0, "Atari Games", "Escape from the Planet of the Robot Monsters (set 1)", MACHINE_SUPPORTS_SAVE )
GAME( 1989, eprom2, eprom, eprom, eprom, eprom_state, eprom, ROT0, "Atari Games", "Escape from the Planet of the Robot Monsters (set 2)", MACHINE_SUPPORTS_SAVE )
GAME( 1989, klaxp1, klax,  klaxp, klaxp, eprom_state, klaxp, ROT0, "Atari Games", "Klax (prototype set 1)", MACHINE_SUPPORTS_SAVE )
GAME( 1989, klaxp2, klax,  klaxp, klaxp, eprom_state, klaxp, ROT0, "Atari Games", "Klax (prototype set 2)", MACHINE_SUPPORTS_SAVE )
GAME( 1989, guts,   0,     guts,  guts, eprom_state,  guts,  ROT0, "Atari Games", "Guts n' Glory (prototype)", 0 )
