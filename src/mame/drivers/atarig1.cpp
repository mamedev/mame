// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    Atari G1 hardware

    driver by Aaron Giles

    Games supported:
        * Hydra (1990)
        * Pit Fighter (1990)

    Known bugs:
        * none

****************************************************************************

    Memory map (TBA)

***************************************************************************/


#include "emu.h"
#include "video/atarirle.h"
#include "includes/atarig1.h"



/*************************************
 *
 *  Initialization & interrupts
 *
 *************************************/

void atarig1_state::update_interrupts()
{
	m_maincpu->set_input_line(1, m_video_int_state ? ASSERT_LINE : CLEAR_LINE);
	m_maincpu->set_input_line(2, m_sound_int_state ? ASSERT_LINE : CLEAR_LINE);
}


MACHINE_START_MEMBER(atarig1_state,atarig1)
{
	atarigen_state::machine_start();
	save_item(NAME(m_which_input));
}


MACHINE_RESET_MEMBER(atarig1_state,atarig1)
{
	atarigen_state::machine_reset();
	scanline_timer_reset(*m_screen, 8);
}



/*************************************
 *
 *  Sprite control latch
 *
 *************************************/

WRITE16_MEMBER(atarig1_state::mo_command_w)
{
	COMBINE_DATA(m_mo_command);
	m_rle->command_write(space, offset, (data == 0 && m_is_pitfight) ? ATARIRLE_COMMAND_CHECKSUM : ATARIRLE_COMMAND_DRAW);
}



/*************************************
 *
 *  I/O read dispatch.
 *
 *************************************/

READ16_MEMBER(atarig1_state::special_port0_r)
{
	int temp = ioport("IN0")->read();
	temp ^= 0x2000;     /* A2DOK always high for now */
	return temp;
}


WRITE16_MEMBER(atarig1_state::a2d_select_w)
{
	m_which_input = offset;
}


READ16_MEMBER(atarig1_state::a2d_data_r)
{
	static const char *const adcnames[] = { "ADC0", "ADC1", "ADC2" };

	/* Pit Fighter has no A2D, just another input port */
	if (m_is_pitfight)
		return ioport("ADC0")->read();

	/* otherwise, assume it's hydra */
	if (m_which_input < 3)
		return ioport(adcnames[m_which_input])->read() << 8;

	return 0;
}



/*************************************
 *
 *  Bootleg "slapstic" handler
 *
 *************************************/

void atarig1_state::update_bank(int bank)
{
	/* if the bank has changed, copy the memory; Pit Fighter needs this */
	if (bank != m_bslapstic_bank)
	{
		/* bank 0 comes from the copy we made earlier */
		if (bank == 0)
			memcpy(m_bslapstic_base, m_bslapstic_bank0, 0x2000);
		else
			memcpy(m_bslapstic_base, &m_bslapstic_base[bank * 0x1000], 0x2000);

		/* remember the current bank */
		m_bslapstic_bank = bank;
	}
}


void atarig1_state::device_post_load()
{
	if (m_bslapstic_base != NULL)
	{
		int bank = m_bslapstic_bank;
		m_bslapstic_bank = -1;
		update_bank(bank);
	}
}


READ16_MEMBER(atarig1_state::pitfightb_cheap_slapstic_r)
{
	int result = m_bslapstic_base[offset & 0xfff];

	/* the cheap replacement slapstic just triggers on the simple banking */
	/* addresses; a software patch ensure that this is good enough */

	/* offset 0 primes the chip */
	if (offset == 0)
		m_bslapstic_primed = true;

	/* one of 4 bankswitchers produces the result */
	else if (m_bslapstic_primed)
	{
		if (offset == 0x42)
			update_bank(0), m_bslapstic_primed = false;
		else if (offset == 0x52)
			update_bank(1), m_bslapstic_primed = false;
		else if (offset == 0x62)
			update_bank(2), m_bslapstic_primed = false;
		else if (offset == 0x72)
			update_bank(3), m_bslapstic_primed = false;
	}
	return result;
}


void atarig1_state::pitfightb_cheap_slapstic_init()
{
	/* install a read handler */
	m_bslapstic_base = m_maincpu->space(AS_PROGRAM).install_read_handler(0x038000, 0x03ffff, read16_delegate(FUNC(atarig1_state::pitfightb_cheap_slapstic_r),this));

	/* allocate memory for a copy of bank 0 */
	m_bslapstic_bank0 = auto_alloc_array(machine(), UINT8, 0x2000);
	memcpy(m_bslapstic_bank0, m_bslapstic_base, 0x2000);

	/* not primed by default */
	m_bslapstic_primed = false;
}



/*************************************
 *
 *  Main CPU memory handlers
 *
 *************************************/

static ADDRESS_MAP_START( main_map, AS_PROGRAM, 16, atarig1_state )
	AM_RANGE(0x000000, 0x037fff) AM_ROM
	AM_RANGE(0x038000, 0x03ffff) AM_ROM /* pitfight slapstic goes here */
	AM_RANGE(0x040000, 0x077fff) AM_ROM
	AM_RANGE(0x078000, 0x07ffff) AM_ROM /* hydra slapstic goes here */
	AM_RANGE(0xf80000, 0xf80001) AM_WRITE(watchdog_reset16_w)
	AM_RANGE(0xf88000, 0xf8ffff) AM_DEVWRITE("eeprom", atari_eeprom_device, unlock_write)
	AM_RANGE(0xf90000, 0xf90001) AM_DEVWRITE8("jsa", atari_jsa_ii_device, main_command_w, 0xff00)
	AM_RANGE(0xf98000, 0xf98001) AM_DEVWRITE("jsa", atari_jsa_ii_device, sound_reset_w)
	AM_RANGE(0xfa0000, 0xfa0001) AM_DEVWRITE8("rle", atari_rle_objects_device, control_write, 0x00ff)
	AM_RANGE(0xfb0000, 0xfb0001) AM_WRITE(video_int_ack_w)
	AM_RANGE(0xfc0000, 0xfc0001) AM_READ(special_port0_r)
	AM_RANGE(0xfc8000, 0xfc8007) AM_READWRITE(a2d_data_r, a2d_select_w)
	AM_RANGE(0xfd0000, 0xfd0001) AM_DEVREAD8("jsa", atari_jsa_ii_device, main_response_r, 0xff00)
	AM_RANGE(0xfd8000, 0xfdffff) AM_DEVREADWRITE8("eeprom", atari_eeprom_device, read, write, 0x00ff)
/*  AM_RANGE(0xfe0000, 0xfe7fff) AM_READ(from_r)*/
	AM_RANGE(0xfe8000, 0xfe89ff) AM_RAM_DEVWRITE("palette", palette_device, write) AM_SHARE("palette")
	AM_RANGE(0xff0000, 0xff0fff) AM_RAM AM_SHARE("rle")
	AM_RANGE(0xff2000, 0xff2001) AM_WRITE(mo_command_w) AM_SHARE("mo_command")
	AM_RANGE(0xff4000, 0xff5fff) AM_DEVWRITE("playfield", tilemap_device, write) AM_SHARE("playfield")
	AM_RANGE(0xff6000, 0xff6fff) AM_DEVWRITE("alpha", tilemap_device, write) AM_SHARE("alpha")
	AM_RANGE(0xff0000, 0xffffff) AM_RAM
ADDRESS_MAP_END



/*************************************
 *
 *  Port definitions
 *
 *************************************/

static INPUT_PORTS_START( hydra )
	PORT_START("IN0")       /* fc0000 */
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_NAME("Left Trigger")
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("Right Trigger")
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_NAME("Left Thumb")
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("Right Thumb")
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_NAME("Boost")
	PORT_BIT( 0x0fe0, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_ATARI_JSA_MAIN_TO_SOUND_READY("jsa")
	PORT_BIT( 0x2000, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_SERVICE( 0x4000, IP_ACTIVE_LOW )
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_VBLANK("screen")

	PORT_START("ADC0")      /* ADC 0 @ fc8000 */
	PORT_BIT( 0x00ff, 0x0080, IPT_AD_STICK_X ) PORT_SENSITIVITY(50) PORT_KEYDELTA(10)
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("ADC1")      /* ADC 1 @ fc8000 */
	PORT_BIT( 0x00ff, 0x0080, IPT_AD_STICK_Y ) PORT_SENSITIVITY(70) PORT_KEYDELTA(10)
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("ADC2")      /* ADC 2 @ fc8000 */
	PORT_BIT( 0xff, 0x00, IPT_PEDAL ) PORT_SENSITIVITY(100) PORT_KEYDELTA(16)

// todo:
//  PORT_MODIFY( "jsa:JSAII" )
//  PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END


static INPUT_PORTS_START( pitfight )
	PORT_START("IN0")       /* fc0000 */
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1) PORT_NAME("P1 Punch")
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1) PORT_NAME("P1 Kick")
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1) PORT_NAME("P1 Jump/Start")
	PORT_BIT( 0x0f80, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_ATARI_JSA_MAIN_TO_SOUND_READY("jsa")
	PORT_BIT( 0x2000, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_SERVICE( 0x4000, IP_ACTIVE_LOW )
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_VBLANK("screen")

	PORT_START("ADC0")      /* fc8000 */
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(3)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(3)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(3)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(3)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(3) PORT_NAME("P3 Punch")
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(3) PORT_NAME("P3 Kick")
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(3) PORT_NAME("P3 Jump/Start")
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2) PORT_NAME("P2 Punch")
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2) PORT_NAME("P2 Kick")
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2) PORT_NAME("P2 Jump/Start")
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("ADC1")      /* not used */
	PORT_BIT( 0xffff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("ADC2")      /* not used */
	PORT_BIT( 0xffff, IP_ACTIVE_LOW, IPT_UNUSED )

// todo:
//  PORT_MODIFY( "jsa:JSAII" )
//  PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END


static INPUT_PORTS_START( pitfightj )
	PORT_START("IN0")       /* fc0000 */
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1) PORT_NAME("Punch")
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1) PORT_NAME("Kick")
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1) PORT_NAME("Jump/P1 Start")
	PORT_BIT( 0x0f80, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_ATARI_JSA_MAIN_TO_SOUND_READY("jsa")
	PORT_BIT( 0x2000, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_SERVICE( 0x4000, IP_ACTIVE_LOW )
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_VBLANK("screen")

	PORT_START("ADC0")      /* fc8000 */
	PORT_BIT( 0x00ff, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2) PORT_NAME("Punch")
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2) PORT_NAME("Kick")
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2) PORT_NAME("Jump/P2 Start")
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("ADC1")      /* not used */
	PORT_BIT( 0xffff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("ADC2")      /* not used */
	PORT_BIT( 0xffff, IP_ACTIVE_LOW, IPT_UNUSED )

// todo:
//  PORT_MODIFY( "jsa:JSAII" )
//  PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END



/*************************************
 *
 *  Graphics definitions
 *
 *************************************/

static const gfx_layout pflayout =
{
	8,8,
	RGN_FRAC(2,5),
	5,
	{ 0, 0, 1, 2, 3 },
	{ RGN_FRAC(2,5)+0, RGN_FRAC(2,5)+4, 0, 4, RGN_FRAC(2,5)+8, RGN_FRAC(2,5)+12, 8, 12 },
	{ STEP8(0,16) },
	16*8
};

static const gfx_layout pftoplayout =
{
	8,8,
	RGN_FRAC(1,5),
	5,
	{ RGN_FRAC(4,5), RGN_FRAC(4,5), RGN_FRAC(4,5), RGN_FRAC(4,5), RGN_FRAC(4,5) },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ STEP8(0,8) },
	8*8
};

static const gfx_layout anlayout =
{
	8,8,
	RGN_FRAC(1,1),
	4,
	{ STEP4(0,1) },
	{ STEP8(0,4) },
	{ STEP8(0,32) },
	32*8
};


static GFXDECODE_START( atarig1 )
	GFXDECODE_ENTRY( "gfx1", 0, pflayout, 0x300, 8 )
	GFXDECODE_ENTRY( "gfx2", 0, anlayout, 0x100, 16 )
	GFXDECODE_ENTRY( "gfx1", 0, pftoplayout, 0x300, 8 )
GFXDECODE_END



static const atari_rle_objects_config modesc_hydra =
{
	0,          /* left clip coordinate */
	255,        /* right clip coordinate */
	0x200,      /* base palette entry */

	{{ 0x7fff,0,0,0,0,0,0,0 }}, /* mask for the code index */
	{{ 0,0x00f0,0,0,0,0,0,0 }}, /* mask for the color */
	{{ 0,0,0xffc0,0,0,0,0,0 }}, /* mask for the X position */
	{{ 0,0,0,0xffc0,0,0,0,0 }}, /* mask for the Y position */
	{{ 0,0,0,0,0xffff,0,0,0 }}, /* mask for the scale factor */
	{{ 0x8000,0,0,0,0,0,0,0 }}, /* mask for the horizontal flip */
	{{ 0,0,0,0,0,0x00ff,0,0 }}, /* mask for the order */
	{{ 0 }},                    /* mask for the priority */
	{{ 0 }}                     /* mask for the VRAM target */
};

static const atari_rle_objects_config modesc_pitfight =
{
	40,         /* left clip coordinate */
	295,        /* right clip coordinate */
	0x200,      /* base palette entry */

	{{ 0x7fff,0,0,0,0,0,0,0 }}, /* mask for the code index */
	{{ 0,0x00f0,0,0,0,0,0,0 }}, /* mask for the color */
	{{ 0,0,0xffc0,0,0,0,0,0 }}, /* mask for the X position */
	{{ 0,0,0,0xffc0,0,0,0,0 }}, /* mask for the Y position */
	{{ 0,0,0,0,0xffff,0,0,0 }}, /* mask for the scale factor */
	{{ 0x8000,0,0,0,0,0,0,0 }}, /* mask for the horizontal flip */
	{{ 0,0,0,0,0,0,0x00ff,0 }}, /* mask for the order */
	{{ 0 }},                    /* mask for the priority */
	{{ 0 }}                     /* mask for the VRAM target */
};

/*************************************
 *
 *  Machine driver
 *
 *************************************/

static MACHINE_CONFIG_START( atarig1, atarig1_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68000, ATARI_CLOCK_14MHz)
	MCFG_CPU_PROGRAM_MAP(main_map)
	MCFG_DEVICE_VBLANK_INT_DRIVER("screen", atarigen_state, video_int_gen)

	MCFG_SLAPSTIC_ADD("slapstic")

	MCFG_MACHINE_START_OVERRIDE(atarig1_state,atarig1)
	MCFG_MACHINE_RESET_OVERRIDE(atarig1_state,atarig1)

	MCFG_ATARI_EEPROM_2816_ADD("eeprom")

	/* video hardware */
	MCFG_GFXDECODE_ADD("gfxdecode", "palette", atarig1)
	MCFG_PALETTE_ADD("palette", 1280)
	MCFG_PALETTE_FORMAT(IRRRRRGGGGGBBBBB)

	/* initialize the playfield */
	MCFG_TILEMAP_ADD_STANDARD("playfield", "gfxdecode", 2, atarig1_state, get_playfield_tile_info, 8,8, SCAN_ROWS, 64,64)
	MCFG_TILEMAP_ADD_STANDARD_TRANSPEN("alpha", "gfxdecode", 2, atarig1_state, get_alpha_tile_info, 8,8, SCAN_ROWS, 64,32, 0)

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_VIDEO_ATTRIBUTES(VIDEO_UPDATE_BEFORE_VBLANK)
	/* note: these parameters are from published specs, not derived */
	MCFG_SCREEN_RAW_PARAMS(ATARI_CLOCK_14MHz/2, 456, 0, 336, 262, 0, 240)
	MCFG_SCREEN_UPDATE_DRIVER(atarig1_state, screen_update_atarig1)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_VIDEO_START_OVERRIDE(atarig1_state,atarig1)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_ATARI_JSA_II_ADD("jsa", WRITELINE(atarigen_state, sound_int_write_line))
	MCFG_ATARI_JSA_TEST_PORT("IN0", 14)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( hydra, atarig1 )
	MCFG_ATARIRLE_ADD("rle", modesc_hydra)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( pitfight, atarig1 )
	MCFG_ATARIRLE_ADD("rle", modesc_pitfight)
MACHINE_CONFIG_END


/*************************************
 *
 *  ROM definition(s)
 *
 *************************************/

ROM_START( hydra )
	ROM_REGION( 0x80000, "maincpu", 0 ) /* 8*64k for 68000 code */
	ROM_LOAD16_BYTE( "136079-3028.bin", 0x00000, 0x10000, CRC(43475f73) SHA1(48a0de5bcbfa2f986edeee93e5a4ef99e13be4de) )
	ROM_LOAD16_BYTE( "136079-3029.bin", 0x00001, 0x10000, CRC(886e1de8) SHA1(5728f5823c6020ff28cbb52faf1e06cb7088eb75) )
	ROM_LOAD16_BYTE( "136079-3034.bin", 0x20000, 0x10000, CRC(5115aa36) SHA1(dce43800ae611166d59e60f9671cf1354e7f91de) )
	ROM_LOAD16_BYTE( "136079-3035.bin", 0x20001, 0x10000, CRC(a28ba44b) SHA1(7d0fe0d10eb32b06da8f2fe059a3b58624b6e1b3) )
	ROM_LOAD16_BYTE( "136079-1032.bin", 0x40000, 0x10000, CRC(ecd1152a) SHA1(b3111c9963316e2726269fd0242824c59651bf45) )
	ROM_LOAD16_BYTE( "136079-1033.bin", 0x40001, 0x10000, CRC(2ebe1939) SHA1(23a9aee465a856cb558517a608285f77fc774b4d) )
	ROM_LOAD16_BYTE( "136079-1030.bin", 0x60000, 0x10000, CRC(b31fd41f) SHA1(1738d31b3262b32f89ce64fe262682b6bb544e79) )
	ROM_LOAD16_BYTE( "136079-1031.bin", 0x60001, 0x10000, CRC(453d076f) SHA1(a7fd8e5efebf56c22e0a7e0b224597b4dba4692a) )

	ROM_REGION( 0x14000, "jsa:cpu", 0 ) /* 64k for 6502 code */
	ROM_LOAD( "hydraa0.bin", 0x10000, 0x4000, CRC(619d7319) SHA1(3c58f18ca5c93ae049bfca91043718fff43e674c) )
	ROM_CONTINUE(            0x04000, 0xc000 )

	ROM_REGION( 0x0a0000, "gfx1", 0 )
	ROM_LOAD( "136079-1017.bin",  0x000000, 0x10000, CRC(bd77b747) SHA1(da57e305468c159ca3d2cfae807a85e643bbf053) ) /* playfield, planes 0-3 odd */
	ROM_LOAD( "136079-1018.bin",  0x010000, 0x10000, CRC(7c24e637) SHA1(dd9fa8a59cbd692b0d8c0e452df4fa18d770c602) )
	ROM_LOAD( "136079-1019.bin",  0x020000, 0x10000, CRC(aa2fb07b) SHA1(ed5aa82d5bac112f0507be3e4e2a5bad184eceeb) )
	ROM_LOAD( "136079-1020.bin",  0x030000, 0x10000, CRC(906ccd98) SHA1(6c226a5058a7432a9fc6e82e0f0608a2ae1a0963) )
	ROM_LOAD( "136079-1021.bin",  0x040000, 0x10000, CRC(f88cdac2) SHA1(891426db0078cda61ff6c8c4ac323cb541c260d8) ) /* playfield, planes 0-3 even */
	ROM_LOAD( "136079-1022.bin",  0x050000, 0x10000, CRC(a9c612ff) SHA1(732d4b7dd6a181fe9a692858d2a72d8994e97829) )
	ROM_LOAD( "136079-1023.bin",  0x060000, 0x10000, CRC(b706aa6e) SHA1(4a0b919668047c24db77b6602edd67bf62e35464) )
	ROM_LOAD( "136079-1024.bin",  0x070000, 0x10000, CRC(c49eac53) SHA1(7b5634aaee20fa8b46de871c2dc3b380fb059449) )
	ROM_LOAD( "136079-1025.bin",  0x080000, 0x10000, CRC(98b5b1a1) SHA1(dfee7d334c4541eb13ee96b43d4d3e1a3c8deb72) ) /* playfield plane 4 */
	ROM_LOAD( "136079-1026.bin",  0x090000, 0x10000, CRC(d68d44aa) SHA1(8fc8b82f4f90515f2af93d3f2d6903a74aac0cc9) )

	ROM_REGION( 0x020000, "gfx2", 0 )
	ROM_LOAD( "136079-1027.bin",  0x000000, 0x20000, CRC(f9135b9b) SHA1(48c0ad0d3e592d191d1385e30530bdb69a095452) ) /* alphanumerics */

	ROM_REGION16_BE( 0x100000, "rle", 0 )
	ROM_LOAD16_BYTE( "136079-1001.bin", 0x00001, 0x10000, CRC(3f757a53) SHA1(be2b7f8b907ef9ea24b24b7210ead70cdbad3506) )
	ROM_LOAD16_BYTE( "136079-1002.bin", 0x00000, 0x10000, CRC(a1169469) SHA1(b5ab65ca9d98ef1e79518eaa519fba0cee92c86e) )
	ROM_LOAD16_BYTE( "136079-1003.bin", 0x20001, 0x10000, CRC(aa21ec33) SHA1(dec65b670c64b3630f6ccbbcc3212f6771908de9) )
	ROM_LOAD16_BYTE( "136079-1004.bin", 0x20000, 0x10000, CRC(c0a2be66) SHA1(b1e454d8b8c80a3f3d087a6eccd555c1ee1e5be6) )
	ROM_LOAD16_BYTE( "136079-1005.bin", 0x40001, 0x10000, CRC(80c285b3) SHA1(bfcb342d2ea5d91562bfca7fac27744682c1d9af) )
	ROM_LOAD16_BYTE( "136079-1006.bin", 0x40000, 0x10000, CRC(ad831c59) SHA1(bd6b52fe4ecfacb5a8c7edb9a67f7d2ed51122e2) )
	ROM_LOAD16_BYTE( "136079-1007.bin", 0x60001, 0x10000, CRC(e0688cc0) SHA1(984266a4f0a4b38be06a20f346851c4d8643512c) )
	ROM_LOAD16_BYTE( "136079-1008.bin", 0x60000, 0x10000, CRC(e6827f6b) SHA1(fd8ca175a065e199a383597f12fbf241f101b608) )
	ROM_LOAD16_BYTE( "136079-1009.bin", 0x80001, 0x10000, CRC(33624d07) SHA1(7847c51c75ad2f0432ebeb7a224ae833a03b5d87) )
	ROM_LOAD16_BYTE( "136079-1010.bin", 0x80000, 0x10000, CRC(9de4c689) SHA1(2ceb3db68ab368105c324a7763ff90448ecd3c49) )
	ROM_LOAD16_BYTE( "136079-1011.bin", 0xa0001, 0x10000, CRC(d55c6e49) SHA1(dd49b4082d645770a3e5bf7f4b043f2ecc84bf89) )
	ROM_LOAD16_BYTE( "136079-1012.bin", 0xa0000, 0x10000, CRC(43af45d0) SHA1(8fc14d534a2f0b3e6df0090c3dd5284b0028aa04) )
	ROM_LOAD16_BYTE( "136079-1013.bin", 0xc0001, 0x10000, CRC(2647a82b) SHA1(b261919842a8277bff15bf6e0f16ca046b580f77) )
	ROM_LOAD16_BYTE( "136079-1014.bin", 0xc0000, 0x10000, CRC(8897d5e9) SHA1(3a5cdc7bf633118453f0028b16f5c22b78cd5904) )
	ROM_LOAD16_BYTE( "136079-1015.bin", 0xe0001, 0x10000, CRC(cf7f69fd) SHA1(93866f66ae7f4071abc66bd310bd15847e2a950a) )
	ROM_LOAD16_BYTE( "136079-1016.bin", 0xe0000, 0x10000, CRC(61aaf14f) SHA1(946caff64902ebdda991372b54c29bd0a0fa13c3) )

	ROM_REGION( 0x40000, "jsa:oki1", 0 )
	ROM_LOAD( "136079-1037.bin",  0x00000, 0x10000, CRC(b974d3d0) SHA1(67ecb17386f4be00c03661de14deff77b8ca85d0) )
	ROM_LOAD( "136079-1038.bin",  0x10000, 0x10000, CRC(a2eda15b) SHA1(358888ffdeb3d0e98f59e239de6d7e1f7e15aca2) )
	ROM_LOAD( "136079-1039.bin",  0x20000, 0x10000, CRC(eb9eaeb7) SHA1(cd8e076b07588879f1a0e6c0fb9de9889480bebb) )

	ROM_REGION( 0x0600, "proms", 0 )    /* microcode for growth renderer */
	ROM_LOAD( "136079-1040.bin",  0x0000, 0x0200, CRC(43d6f3d4) SHA1(a072099df1db8db3589130c67a86a362e03d70ff) )
	ROM_LOAD( "136079-1041.bin",  0x0200, 0x0200, CRC(341dc4bb) SHA1(175143e29cf9e6a4cecb43b3801356085944d168) )
	ROM_LOAD( "136079-1042.bin",  0x0400, 0x0200, CRC(2e49b52e) SHA1(f8abffbcafe2cba7d1410175bb75ec07faac3b47) )
ROM_END


ROM_START( hydrap )
	ROM_REGION( 0x80000, "maincpu", 0 ) /* 8*64k for 68000 code */
	ROM_LOAD16_BYTE( "hydhi0.bin", 0x00000, 0x10000, CRC(dab2e8a2) SHA1(ba13b973b2c351fcf36e8dc1a481e797c6e5911e) )
	ROM_LOAD16_BYTE( "hydlo0.bin", 0x00001, 0x10000, CRC(c18d4f16) SHA1(93a165b1726ea6fc6488ddfd49ba4cec960612e4) )
	ROM_LOAD16_BYTE( "hydhi1.bin", 0x20000, 0x10000, CRC(50c12bb9) SHA1(dea919be2878c8079de4be690ce91d8de5c42771) )
	ROM_LOAD16_BYTE( "hydlo1.bin", 0x20001, 0x10000, CRC(5ee0a846) SHA1(2ee9695d386de951c8fdac629a67dc516ac4ffe4) )
	ROM_LOAD16_BYTE( "hydhi2.bin", 0x40000, 0x10000, CRC(436a6d81) SHA1(9002ef646eab6a27639d712e1b86aea68089a965) )
	ROM_LOAD16_BYTE( "hydlo2.bin", 0x40001, 0x10000, CRC(182bfd6a) SHA1(5f8b9979985e11a3f3b704a56cc50b364e18f0bc) )
	ROM_LOAD16_BYTE( "hydhi3.bin", 0x60000, 0x10000, CRC(29e9e03e) SHA1(0b03482834c1c8fcdd902d513c23c0cc04900f5f) )
	ROM_LOAD16_BYTE( "hydlo3.bin", 0x60001, 0x10000, CRC(7b5047f0) SHA1(99b59dfebc0df0b876e69a885a3e3b07ef958fd4) )

	ROM_REGION( 0x14000, "jsa:cpu", 0 ) /* 64k for 6502 code */
	ROM_LOAD( "hydraa0.bin", 0x10000, 0x4000, BAD_DUMP CRC(619d7319) SHA1(3c58f18ca5c93ae049bfca91043718fff43e674c)  )
	ROM_CONTINUE(            0x04000, 0xc000 )

	ROM_REGION( 0x0a0000, "gfx1", 0 )
	ROM_LOAD( "136079-1017.bin",  0x000000, 0x10000, CRC(bd77b747) SHA1(da57e305468c159ca3d2cfae807a85e643bbf053) ) /* playfield, planes 0-3 odd */
	ROM_LOAD( "136079-1018.bin",  0x010000, 0x10000, CRC(7c24e637) SHA1(dd9fa8a59cbd692b0d8c0e452df4fa18d770c602) )
	ROM_LOAD( "136079-1019.bin",  0x020000, 0x10000, CRC(aa2fb07b) SHA1(ed5aa82d5bac112f0507be3e4e2a5bad184eceeb) )
	ROM_LOAD( "hydpl03.bin",   0x030000, 0x10000, CRC(1f0dfe60) SHA1(579d06b52807c1f0081bab3dfe76a10fc3d05260) )
	ROM_LOAD( "136079-1021.bin",  0x040000, 0x10000, CRC(f88cdac2) SHA1(891426db0078cda61ff6c8c4ac323cb541c260d8) ) /* playfield, planes 0-3 even */
	ROM_LOAD( "136079-1022.bin",  0x050000, 0x10000, CRC(a9c612ff) SHA1(732d4b7dd6a181fe9a692858d2a72d8994e97829) )
	ROM_LOAD( "136079-1023.bin",  0x060000, 0x10000, CRC(b706aa6e) SHA1(4a0b919668047c24db77b6602edd67bf62e35464) )
	ROM_LOAD( "hydphi3.bin",   0x070000, 0x10000, CRC(917e250c) SHA1(197c9b719b00d750365a477e60a623b910ef3a1f) )
	ROM_LOAD( "136079-1025.bin",  0x080000, 0x10000, CRC(98b5b1a1) SHA1(dfee7d334c4541eb13ee96b43d4d3e1a3c8deb72) ) /* playfield plane 4 */
	ROM_LOAD( "hydpl41.bin",   0x090000, 0x10000, CRC(85f9afa6) SHA1(01d8e07ff249bfab83791fc8916542f38133fadf) )

	ROM_REGION( 0x020000, "gfx2", 0 )
	ROM_LOAD( "hydalph.bin",   0x000000, 0x20000, CRC(7dd2b062) SHA1(789b35b1e8cce73e2314d1b6688b5066df91b604) ) /* alphanumerics */

	ROM_REGION16_BE( 0x100000, "rle", 0 )
	ROM_LOAD16_BYTE( "hydmhi0.bin", 0x00001, 0x10000, CRC(3c83b42d) SHA1(3c6de7e6aab08a673227f06b04e000714a9111b1) )
	ROM_LOAD16_BYTE( "hydmlo0.bin", 0x00000, 0x10000, CRC(6d49650c) SHA1(97f9dbdfa5cc620705eec1da2398b2ac63cef30f) )
	ROM_LOAD16_BYTE( "hydmhi1.bin", 0x20001, 0x10000, CRC(689b3376) SHA1(85ffa31f10483317db615cf8e520a8c8a40d6013) )
	ROM_LOAD16_BYTE( "hydmlo1.bin", 0x20000, 0x10000, CRC(c81a4e88) SHA1(6f88edde381a1114fb6dac33ced2db255e6bfd21) )
	ROM_LOAD16_BYTE( "hydmhi2.bin", 0x40001, 0x10000, CRC(77098e14) SHA1(56e716bea36eec62c4142715794ec6c220d10298) )
	ROM_LOAD16_BYTE( "hydmlo2.bin", 0x40000, 0x10000, CRC(40015d9d) SHA1(fa8b6ad77219bc6702edef0c35b09b50bbe68713) )
	ROM_LOAD16_BYTE( "hydmhi3.bin", 0x60001, 0x10000, CRC(dfebdcbd) SHA1(eac79102601fb6d45f263d183bef5132d3896d13) )
	ROM_LOAD16_BYTE( "hydmlo3.bin", 0x60000, 0x10000, CRC(213c407c) SHA1(fe8c9fdabe4b4a3bc69db2381e19f16413d10ae5) )
	ROM_LOAD16_BYTE( "hydmhi4.bin", 0x80001, 0x10000, CRC(2897765f) SHA1(59c2ba99bae38a84f57865588dbc8a34d85e06a9) )
	ROM_LOAD16_BYTE( "hydmlo4.bin", 0x80000, 0x10000, CRC(730157f3) SHA1(dd7d639591e25b9fc43c0fa64f6aa2abebb4bd9c) )
	ROM_LOAD16_BYTE( "hydmhi5.bin", 0xa0001, 0x10000, CRC(ecd061ae) SHA1(698ea30aca2174b71df5fc4030e8c8c45de7bd89) )
	ROM_LOAD16_BYTE( "hydmlo5.bin", 0xa0000, 0x10000, CRC(a5a08c53) SHA1(f3eef37aad8ad72897141341741b9c915a3af10a) )
	ROM_LOAD16_BYTE( "hydmhi6.bin", 0xc0001, 0x10000, CRC(aa3f2903) SHA1(940eb8907a02bd306fcf20c1bb3d1520a1864575) )
	ROM_LOAD16_BYTE( "hydmlo6.bin", 0xc0000, 0x10000, CRC(db8ea56f) SHA1(7c4fb5b8ab6e27d469e363b3b81af61eb4e6afee) )
	ROM_LOAD16_BYTE( "hydmhi7.bin", 0xe0001, 0x10000, CRC(71fc3e43) SHA1(ce6bdd68ed40c2b48d679ece740f0376b2442e2b) )
	ROM_LOAD16_BYTE( "hydmlo7.bin", 0xe0000, 0x10000, CRC(7960b0c2) SHA1(56e7d0b48d6afce6a3c1a940fc578a5775abd940) )

	ROM_REGION( 0x40000, "jsa:oki1", 0 )
	ROM_LOAD( "136079-1037.bin",  0x00000, 0x10000, BAD_DUMP CRC(b974d3d0) SHA1(67ecb17386f4be00c03661de14deff77b8ca85d0)  )
	ROM_LOAD( "136079-1038.bin",  0x10000, 0x10000, BAD_DUMP CRC(a2eda15b) SHA1(358888ffdeb3d0e98f59e239de6d7e1f7e15aca2)  )
	ROM_LOAD( "136079-1039.bin",  0x20000, 0x10000, BAD_DUMP CRC(eb9eaeb7) SHA1(cd8e076b07588879f1a0e6c0fb9de9889480bebb)  )

	ROM_REGION( 0x0600, "proms", 0 )    /* microcode for growth renderer */
	ROM_LOAD( "136079-1040.bin",  0x0000, 0x0200, CRC(43d6f3d4) SHA1(a072099df1db8db3589130c67a86a362e03d70ff) )
	ROM_LOAD( "136079-1041.bin",  0x0200, 0x0200, CRC(341dc4bb) SHA1(175143e29cf9e6a4cecb43b3801356085944d168) )
	ROM_LOAD( "136079-1042.bin",  0x0400, 0x0200, CRC(2e49b52e) SHA1(f8abffbcafe2cba7d1410175bb75ec07faac3b47) )
ROM_END


ROM_START( hydrap2 )
	ROM_REGION( 0x80000, "maincpu", 0 ) /* 8*64k for 68000 code */
	ROM_LOAD16_BYTE( "05c", 0x00001, 0x10000, CRC(531ebb3b) SHA1(866de3e2c747bd272c5235f9717ebeaeca90735b) )
	ROM_LOAD16_BYTE( "05e", 0x00000, 0x10000, CRC(6d77b124) SHA1(a485a783211a052ca01aa400b3c5e59a2dba6faa) )
	ROM_LOAD16_BYTE( "15c", 0x20001, 0x10000, CRC(2f823b49) SHA1(db457b43e528a6d447802259707a00f02bf92f2e) )
	ROM_LOAD16_BYTE( "15e", 0x20000, 0x10000, CRC(cfda9f58) SHA1(7b2727751978f35b57f8a56d8db7d1cd9378f6af) )
	ROM_LOAD16_BYTE( "20c", 0x40001, 0x10000, CRC(a501e37b) SHA1(7cf9dbe19d305304543793045cf2da934ff34d1e) )
	ROM_LOAD16_BYTE( "20e", 0x40000, 0x10000, CRC(f75541ca) SHA1(c3f8756b25b0d9f4d2d0e51a2fcc23f5a158ff87) )
	ROM_LOAD16_BYTE( "30c", 0x60001, 0x10000, CRC(89604306) SHA1(ccac6eabb174903f4ee144fce53a169daa734e07) )
	ROM_LOAD16_BYTE( "30e", 0x60000, 0x10000, CRC(25221b17) SHA1(bb14117f256c3db6881bb91cace297d4c636e684) )

	ROM_REGION( 0x14000, "jsa:cpu", 0 ) /* 64k for 6502 code */
	ROM_LOAD( "aud.1b",      0x10000, 0x4000, CRC(e1b5188a) SHA1(e9f2a78df49fa085a9363ca194e2ceb5fa5409c4) )
	ROM_CONTINUE(            0x04000, 0xc000 )

	ROM_REGION( 0x0a0000, "gfx1", 0 )
	ROM_LOAD( "136079-1017.bin",  0x000000, 0x10000, CRC(bd77b747) SHA1(da57e305468c159ca3d2cfae807a85e643bbf053) ) /* playfield, planes 0-3 odd */
	ROM_LOAD( "136079-1018.bin",  0x010000, 0x10000, CRC(7c24e637) SHA1(dd9fa8a59cbd692b0d8c0e452df4fa18d770c602) )
	ROM_LOAD( "136079-1019.bin",  0x020000, 0x10000, CRC(aa2fb07b) SHA1(ed5aa82d5bac112f0507be3e4e2a5bad184eceeb) )
	ROM_LOAD( "136079-1020.bin",  0x030000, 0x10000, CRC(906ccd98) SHA1(6c226a5058a7432a9fc6e82e0f0608a2ae1a0963) )
	ROM_LOAD( "136079-1021.bin",  0x040000, 0x10000, CRC(f88cdac2) SHA1(891426db0078cda61ff6c8c4ac323cb541c260d8) ) /* playfield, planes 0-3 even */
	ROM_LOAD( "136079-1022.bin",  0x050000, 0x10000, CRC(a9c612ff) SHA1(732d4b7dd6a181fe9a692858d2a72d8994e97829) )
	ROM_LOAD( "136079-1023.bin",  0x060000, 0x10000, CRC(b706aa6e) SHA1(4a0b919668047c24db77b6602edd67bf62e35464) )
	ROM_LOAD( "136079-1024.bin",  0x070000, 0x10000, CRC(c49eac53) SHA1(7b5634aaee20fa8b46de871c2dc3b380fb059449) )
	ROM_LOAD( "136079-1025.bin",  0x080000, 0x10000, CRC(98b5b1a1) SHA1(dfee7d334c4541eb13ee96b43d4d3e1a3c8deb72) ) /* playfield plane 4 */
	ROM_LOAD( "136079-1026.bin",  0x090000, 0x10000, CRC(d68d44aa) SHA1(8fc8b82f4f90515f2af93d3f2d6903a74aac0cc9) )

	ROM_REGION( 0x020000, "gfx2", 0 )
	ROM_LOAD( "136079-1027.bin",  0x000000, 0x20000, CRC(f9135b9b) SHA1(48c0ad0d3e592d191d1385e30530bdb69a095452) ) /* alphanumerics */

	ROM_REGION16_BE( 0x100000, "rle", 0 )
	ROM_LOAD16_BYTE( "136079-1001.bin", 0x00001, 0x10000, BAD_DUMP CRC(3f757a53) SHA1(be2b7f8b907ef9ea24b24b7210ead70cdbad3506) ) // not dumped from this pcb, rom taken from another set instead
	ROM_LOAD16_BYTE( "136079-1002.bin", 0x00000, 0x10000, BAD_DUMP CRC(a1169469) SHA1(b5ab65ca9d98ef1e79518eaa519fba0cee92c86e) ) // "
	ROM_LOAD16_BYTE( "136079-1003.bin", 0x20001, 0x10000, BAD_DUMP CRC(aa21ec33) SHA1(dec65b670c64b3630f6ccbbcc3212f6771908de9) ) // "
	ROM_LOAD16_BYTE( "136079-1004.bin", 0x20000, 0x10000, BAD_DUMP CRC(c0a2be66) SHA1(b1e454d8b8c80a3f3d087a6eccd555c1ee1e5be6) ) // "
	ROM_LOAD16_BYTE( "136079-1005.bin", 0x40001, 0x10000, BAD_DUMP CRC(80c285b3) SHA1(bfcb342d2ea5d91562bfca7fac27744682c1d9af) ) // "
	ROM_LOAD16_BYTE( "136079-1006.bin", 0x40000, 0x10000, BAD_DUMP CRC(ad831c59) SHA1(bd6b52fe4ecfacb5a8c7edb9a67f7d2ed51122e2) ) // "
	ROM_LOAD16_BYTE( "136079-1007.bin", 0x60001, 0x10000, BAD_DUMP CRC(e0688cc0) SHA1(984266a4f0a4b38be06a20f346851c4d8643512c) ) // "
	ROM_LOAD16_BYTE( "136079-1008.bin", 0x60000, 0x10000, BAD_DUMP CRC(e6827f6b) SHA1(fd8ca175a065e199a383597f12fbf241f101b608) ) // "
	ROM_LOAD16_BYTE( "136079-1009.bin", 0x80001, 0x10000, BAD_DUMP CRC(33624d07) SHA1(7847c51c75ad2f0432ebeb7a224ae833a03b5d87) ) // "
	ROM_LOAD16_BYTE( "136079-1010.bin", 0x80000, 0x10000, BAD_DUMP CRC(9de4c689) SHA1(2ceb3db68ab368105c324a7763ff90448ecd3c49) ) // "
	ROM_LOAD16_BYTE( "136079-1011.bin", 0xa0001, 0x10000, BAD_DUMP CRC(d55c6e49) SHA1(dd49b4082d645770a3e5bf7f4b043f2ecc84bf89) ) // "
	ROM_LOAD16_BYTE( "136079-1012.bin", 0xa0000, 0x10000, BAD_DUMP CRC(43af45d0) SHA1(8fc14d534a2f0b3e6df0090c3dd5284b0028aa04) ) // "
	ROM_LOAD16_BYTE( "136079-1013.bin", 0xc0001, 0x10000, BAD_DUMP CRC(2647a82b) SHA1(b261919842a8277bff15bf6e0f16ca046b580f77) ) // "
	ROM_LOAD16_BYTE( "136079-1014.bin", 0xc0000, 0x10000, BAD_DUMP CRC(8897d5e9) SHA1(3a5cdc7bf633118453f0028b16f5c22b78cd5904) ) // "
	ROM_LOAD16_BYTE( "136079-1015.bin", 0xe0001, 0x10000, BAD_DUMP CRC(cf7f69fd) SHA1(93866f66ae7f4071abc66bd310bd15847e2a950a) ) // "
	ROM_LOAD16_BYTE( "136079-1016.bin", 0xe0000, 0x10000, BAD_DUMP CRC(61aaf14f) SHA1(946caff64902ebdda991372b54c29bd0a0fa13c3) ) // "

	ROM_REGION( 0x40000, "jsa:oki1", 0 )
	ROM_LOAD( "136079-1037.bin",  0x00000, 0x10000, BAD_DUMP CRC(b974d3d0) SHA1(67ecb17386f4be00c03661de14deff77b8ca85d0)  ) // not dumped from this pcb, rom taken from another set instead
	ROM_LOAD( "136079-1038.bin",  0x10000, 0x10000, BAD_DUMP CRC(a2eda15b) SHA1(358888ffdeb3d0e98f59e239de6d7e1f7e15aca2)  ) // "
	ROM_LOAD( "136079-1039.bin",  0x20000, 0x10000, BAD_DUMP CRC(eb9eaeb7) SHA1(cd8e076b07588879f1a0e6c0fb9de9889480bebb)  ) // "

	ROM_REGION( 0x0600, "proms", 0 )    /* microcode for growth renderer */
	ROM_LOAD( "136079-1040.bin",  0x0000, 0x0200, CRC(43d6f3d4) SHA1(a072099df1db8db3589130c67a86a362e03d70ff) )
	ROM_LOAD( "136079-1041.bin",  0x0200, 0x0200, CRC(341dc4bb) SHA1(175143e29cf9e6a4cecb43b3801356085944d168) )
	ROM_LOAD( "136079-1042.bin",  0x0400, 0x0200, CRC(2e49b52e) SHA1(f8abffbcafe2cba7d1410175bb75ec07faac3b47) )
ROM_END


/*
Pit Fighter (rev 9)

There are printed Atari stickers that show the following:

2 main board stickers:

PITFIGHTER
A0478936-03

WO#-PITPCB08
G1 REV.D BD.

2 stickers on JSA Audio II board:

JSA II PITJSA10
PIT
FIGHTER
A047184-05

SBOM
00R5
001

The card stuck into location 10P has 1 sticker:

OOPSPCB01
G1 OOPS
PCB
A048490-01

The chip on the 10P card is labeled 136079-2053
*/
ROM_START( pitfight )
	ROM_REGION( 0x80000, "maincpu", 0 ) /* 8*64k for 68000 code */
	ROM_LOAD16_BYTE( "136081-9028.05d", 0x00000, 0x10000, CRC(427713e9) SHA1(8b4913188cac9fe3845c812a07c81f5fdb4a2ded) )
	ROM_LOAD16_BYTE( "136081-9029.05b", 0x00001, 0x10000, CRC(2cdeaeba) SHA1(e02c99d7f5a7080ea75dc2dd59d4678b62f6bd55) )
	ROM_LOAD16_BYTE( "136081-9030.15d", 0x20000, 0x10000, CRC(3bace9ef) SHA1(29072871b268f343fa1e7fcc9682674df2b2e34f) )
	ROM_LOAD16_BYTE( "136081-9031.15b", 0x20001, 0x10000, CRC(c717f011) SHA1(3c5d6c12b85285422345a1aba3f8c497f74c6889) )

	ROM_REGION( 0x14000, "jsa:cpu", 0 ) /* 64k for 6502 code */
	ROM_LOAD( "136081-1060.1b", 0x10000, 0x4000, CRC(231d71d7) SHA1(24622eee5fe873ef81e1df2691bd7a1d3ea7ef6b) )
	ROM_CONTINUE(               0x04000, 0xc000 )

	ROM_REGION( 0x0a0000, "gfx1", 0 )
	ROM_LOAD( "136081-1017.130m", 0x000000, 0x10000, CRC(ad3cfea5) SHA1(7b6fec131230e84ab87b7fc95f08989916f30e02) ) /* playfield, planes 0-3 odd */
	ROM_LOAD( "136081-1018.120m", 0x010000, 0x10000, CRC(1a0f8bcf) SHA1(b965e73246db9507d2ad42dfcb033692b43b9b7a) )
	ROM_LOAD( "136081-1021.90m",  0x040000, 0x10000, CRC(777efee3) SHA1(07591f11685c4c75c24c55fd242878253d32481b) ) /* playfield, planes 0-3 even */
	ROM_LOAD( "136081-1022.75m",  0x050000, 0x10000, CRC(524319d0) SHA1(6f47a69d7d4e2a8f79b7470138e8b4edd6d0b2bb) )
	ROM_LOAD( "136081-1025.45m",  0x080000, 0x10000, CRC(fc41691a) SHA1(4ef2f9093f20d27e1ba7d218b90ff6abb1f33646) ) /* playfield plane 4 */

	ROM_REGION( 0x020000, "gfx2", 0 )
	ROM_LOAD( "136081-1027.15l",  0x000000, 0x10000, CRC(a59f381d) SHA1(b14e878340ad2adbf4f6d4fc331c58f62037c7c7) ) /* alphanumerics */

	ROM_REGION16_BE( 0x200000, "rle", 0 )
	ROM_LOAD16_BYTE( "136081-1001.65r",  0x000001, 0x20000, CRC(3af31444) SHA1(91fc02786b82abdf12ebdbaacdd1f158f8ce6d06) )
	ROM_LOAD16_BYTE( "136081-1002.65n",  0x000000, 0x20000, CRC(f1d76a4c) SHA1(ca769d2cdd096f4a54f7bcaa4840fc9ffaabf499) )
	ROM_LOAD16_BYTE( "136081-1003.70r",  0x040001, 0x20000, CRC(28c41c2a) SHA1(75cd527f98c8475e3b880f53c5a355d6c3bd8766) )
	ROM_LOAD16_BYTE( "136081-1004.70n",  0x040000, 0x20000, CRC(977744da) SHA1(10f388eef999f702863e2b1a02a188df9a0a6086) )
	ROM_LOAD16_BYTE( "136081-1005.75r",  0x080001, 0x20000, CRC(ae59aef2) SHA1(44eda8a01016f6911b47df973b9427ff9f2ecef0) )
	ROM_LOAD16_BYTE( "136081-1006.75n",  0x080000, 0x20000, CRC(b6ccd77e) SHA1(f09a829143abd972699f3e671d4a1362fd362b19) )
	ROM_LOAD16_BYTE( "136081-1007.90r",  0x0c0001, 0x20000, CRC(ba33b0c0) SHA1(be09ba8796d3db2a859a2776336c1e9acbdaee95) )
	ROM_LOAD16_BYTE( "136081-1008.90n",  0x0c0000, 0x20000, CRC(09bd047c) SHA1(c283d526c7b08ce06e3981d1fc69433ea0dae657) )
	ROM_LOAD16_BYTE( "136081-1009.100r", 0x100001, 0x20000, CRC(ab85b00b) SHA1(6c60b9b58ff93ffdd1cffc49c3ea67400d56bf61) )
	ROM_LOAD16_BYTE( "136081-1010.100n", 0x100000, 0x20000, CRC(eca94bdc) SHA1(c474dfc9dc4460cd2f40cc6012214e760b133c0d) )
	ROM_LOAD16_BYTE( "136081-1011.115r", 0x140001, 0x20000, CRC(a86582fd) SHA1(8b557622c53c8ff388248ea54d3cc85b0d77cafb) )
	ROM_LOAD16_BYTE( "136081-1012.115n", 0x140000, 0x20000, CRC(efd1152d) SHA1(77d1752b76b079c9e834a8b73e8601873d3afdbe) )
	ROM_LOAD16_BYTE( "136081-1013.120r", 0x180001, 0x20000, CRC(a141379e) SHA1(d4c98b364495e19e2e7bee5431834f38ba20a514) )
	ROM_LOAD16_BYTE( "136081-1014.120n", 0x180000, 0x20000, CRC(93bfcc15) SHA1(4d8f6c8c279533b2fc35f87c3eb5ed614cb03248) )
	ROM_LOAD16_BYTE( "136081-1015.130r", 0x1c0001, 0x20000, CRC(9378ad0b) SHA1(909f9879f5b8fc3ed0622fd27d903ccb1f7a90c6) )
	ROM_LOAD16_BYTE( "136081-1016.130n", 0x1c0000, 0x20000, CRC(19c3fbe0) SHA1(ba28f71edb04387f009afe39bfe0ffeff8fbf5e9) )

	ROM_REGION( 0x40000, "jsa:oki1", 0 )   /* 256k for ADPCM samples */
	ROM_LOAD( "136081-1061.7k",  0x00000, 0x10000, CRC(5b0468c6) SHA1(c910344622386e6be336fe04bc0be758ac6c59db) )
	ROM_LOAD( "136081-1062.7j",  0x10000, 0x10000, CRC(f73fe3cb) SHA1(547b5c4add617237c4c851751a27cda091fb7933) )
	ROM_LOAD( "136081-1063.7e",  0x20000, 0x10000, CRC(aa93421d) SHA1(f319057dadcb77a489d0bcffb24e0afe88adc769) )
	ROM_LOAD( "136081-1064.7d",  0x30000, 0x10000, CRC(33f045d5) SHA1(1fc7bedafeb3c1ffa0f115538cc300959c8d4601) )

	ROM_REGION( 0x0600, "proms", 0 )    /* microcode for growth renderer */
	ROM_LOAD( "136081-1040.30s",  0x0000, 0x0200, CRC(9b0f8b95) SHA1(d03987fe2d50a9f7769c600885bd1c7e1dee0789) )
	ROM_LOAD( "136081-1041.25s",  0x0200, 0x0200, CRC(f7ba6153) SHA1(d58792c9e9ea72d8f53f41ac1b420a86db6da3a3) )
	ROM_LOAD( "136081-1042.20s",  0x0400, 0x0200, CRC(3572fe68) SHA1(ab34ff337c16cd4d568cd2bd6a5063f5ed97368f) )

	ROM_REGION( 0x1200, "plds", 0 )
	ROM_LOAD( "gal16v8a-136081-1043.15e",  0x0000, 0x0117, CRC(649d9c0d) SHA1(bba2f3cf4607946340de27157c799f6b84c48584) )
	ROM_LOAD( "gal16v8a-136079-1045.40p",  0x0200, 0x0117, CRC(535c4d2e) SHA1(a428eef81723546566216e6af0db3baebda9a1f2) )
	ROM_LOAD( "gal16v8a-136079-1046.65d",  0x0400, 0x0117, CRC(38b22b2c) SHA1(cf9051129a790f3427e1c6d74a5c66f16acbd4be) )
	ROM_LOAD( "gal16v8a-136079-1047.40s",  0x0600, 0x0117, CRC(20839904) SHA1(134d1b379b7708ce33fbbf75e5c879dad971cd22) )
	ROM_LOAD( "gal16v8a-136079-1048.125f", 0x0800, 0x0117, CRC(d16e8d6e) SHA1(61cc270d80cb34f63fc407e41b55f95d59717757) )
	ROM_LOAD( "gal16v8a-136079-1049.120f", 0x0a00, 0x0117, CRC(68440ab1) SHA1(2f4eabac27d9a5778fcba0d9aa47460bcb153378) )
	ROM_LOAD( "gal16v8a-136079-1050.115f", 0x0c00, 0x0117, CRC(20dc296a) SHA1(9179fca2efcdd0782bb15cbf1923670806755bc8) )
	ROM_LOAD( "gal16v8a-136079-1051.105f", 0x0e00, 0x0117, CRC(45f0cb5f) SHA1(a7043231f68374871253da50cfd905519f283d9e) )
	ROM_LOAD( "gal16v8a-136079-1052.30n",  0x1000, 0x0117, CRC(6b3d2669) SHA1(143178d9c36a87aa58034faea2e4d4038ae565e7) )
ROM_END


/*
Pit Fighter (rev 7)

There are printed Atari stickers that show the following:

2 main board stickers:

PIT FIGHTER
A0478936-03

WO#-PITPCB07
G1 REV.D BD.

2 stickers on JSA Audio II board:

JSA II REV.B BD.
PIT
FIGHTER
A047184-05

PIT
JSA
007

The card stuck into location 10P has 1 sticker:

OOPSPCB01
G1 OOPS
PCB
A048490-01

The chip on the 10P card is labeled 136079-2053.
*/
ROM_START( pitfight7 )
	ROM_REGION( 0x80000, "maincpu", 0 ) /* 8*64k for 68000 code */
	ROM_LOAD16_BYTE( "136081-7028.05d", 0x00000, 0x10000, CRC(7391dc8b) SHA1(2140b21e71871c2cb81cccd92f536763d955ec97) )
	ROM_LOAD16_BYTE( "136081-7029.05b", 0x00001, 0x10000, CRC(b3b88382) SHA1(0a0de330d7261c7eaa5aa705328d6c0c28d27536) )
	ROM_LOAD16_BYTE( "136081-7030.15d", 0x20000, 0x10000, CRC(5fd5a0b1) SHA1(5d4711e8d10176b6989c4db012dbb4e29860590c) )
	ROM_LOAD16_BYTE( "136081-7031.15b", 0x20001, 0x10000, CRC(e14a1d0c) SHA1(734fa1cd5ad835fa77c686006993ea9358e3b072) )

	ROM_REGION( 0x14000, "jsa:cpu", 0 ) /* 64k for 6502 code */
	ROM_LOAD( "136081-1060.1b", 0x10000, 0x4000, CRC(231d71d7) SHA1(24622eee5fe873ef81e1df2691bd7a1d3ea7ef6b) )
	ROM_CONTINUE(               0x04000, 0xc000 )

	ROM_REGION( 0x0a0000, "gfx1", 0 )
	ROM_LOAD( "136081-1017.130m", 0x000000, 0x10000, CRC(ad3cfea5) SHA1(7b6fec131230e84ab87b7fc95f08989916f30e02) ) /* playfield, planes 0-3 odd */
	ROM_LOAD( "136081-1018.120m", 0x010000, 0x10000, CRC(1a0f8bcf) SHA1(b965e73246db9507d2ad42dfcb033692b43b9b7a) )
	ROM_LOAD( "136081-1021.90m",  0x040000, 0x10000, CRC(777efee3) SHA1(07591f11685c4c75c24c55fd242878253d32481b) ) /* playfield, planes 0-3 even */
	ROM_LOAD( "136081-1022.75m",  0x050000, 0x10000, CRC(524319d0) SHA1(6f47a69d7d4e2a8f79b7470138e8b4edd6d0b2bb) )
	ROM_LOAD( "136081-1025.45m",  0x080000, 0x10000, CRC(fc41691a) SHA1(4ef2f9093f20d27e1ba7d218b90ff6abb1f33646) ) /* playfield plane 4 */

	ROM_REGION( 0x020000, "gfx2", 0 )
	ROM_LOAD( "136081-1027.15l",  0x000000, 0x10000, CRC(a59f381d) SHA1(b14e878340ad2adbf4f6d4fc331c58f62037c7c7) ) /* alphanumerics */

	ROM_REGION16_BE( 0x200000, "rle", 0 )
	ROM_LOAD16_BYTE( "136081-1001.65r",  0x000001, 0x20000, CRC(3af31444) SHA1(91fc02786b82abdf12ebdbaacdd1f158f8ce6d06) )
	ROM_LOAD16_BYTE( "136081-1002.65n",  0x000000, 0x20000, CRC(f1d76a4c) SHA1(ca769d2cdd096f4a54f7bcaa4840fc9ffaabf499) )
	ROM_LOAD16_BYTE( "136081-1003.70r",  0x040001, 0x20000, CRC(28c41c2a) SHA1(75cd527f98c8475e3b880f53c5a355d6c3bd8766) )
	ROM_LOAD16_BYTE( "136081-1004.70n",  0x040000, 0x20000, CRC(977744da) SHA1(10f388eef999f702863e2b1a02a188df9a0a6086) )
	ROM_LOAD16_BYTE( "136081-1005.75r",  0x080001, 0x20000, CRC(ae59aef2) SHA1(44eda8a01016f6911b47df973b9427ff9f2ecef0) )
	ROM_LOAD16_BYTE( "136081-1006.75n",  0x080000, 0x20000, CRC(b6ccd77e) SHA1(f09a829143abd972699f3e671d4a1362fd362b19) )
	ROM_LOAD16_BYTE( "136081-1007.90r",  0x0c0001, 0x20000, CRC(ba33b0c0) SHA1(be09ba8796d3db2a859a2776336c1e9acbdaee95) )
	ROM_LOAD16_BYTE( "136081-1008.90n",  0x0c0000, 0x20000, CRC(09bd047c) SHA1(c283d526c7b08ce06e3981d1fc69433ea0dae657) )
	ROM_LOAD16_BYTE( "136081-1009.100r", 0x100001, 0x20000, CRC(ab85b00b) SHA1(6c60b9b58ff93ffdd1cffc49c3ea67400d56bf61) )
	ROM_LOAD16_BYTE( "136081-1010.100n", 0x100000, 0x20000, CRC(eca94bdc) SHA1(c474dfc9dc4460cd2f40cc6012214e760b133c0d) )
	ROM_LOAD16_BYTE( "136081-1011.115r", 0x140001, 0x20000, CRC(a86582fd) SHA1(8b557622c53c8ff388248ea54d3cc85b0d77cafb) )
	ROM_LOAD16_BYTE( "136081-1012.115n", 0x140000, 0x20000, CRC(efd1152d) SHA1(77d1752b76b079c9e834a8b73e8601873d3afdbe) )
	ROM_LOAD16_BYTE( "136081-1013.120r", 0x180001, 0x20000, CRC(a141379e) SHA1(d4c98b364495e19e2e7bee5431834f38ba20a514) )
	ROM_LOAD16_BYTE( "136081-1014.120n", 0x180000, 0x20000, CRC(93bfcc15) SHA1(4d8f6c8c279533b2fc35f87c3eb5ed614cb03248) )
	ROM_LOAD16_BYTE( "136081-1015.130r", 0x1c0001, 0x20000, CRC(9378ad0b) SHA1(909f9879f5b8fc3ed0622fd27d903ccb1f7a90c6) )
	ROM_LOAD16_BYTE( "136081-1016.130n", 0x1c0000, 0x20000, CRC(19c3fbe0) SHA1(ba28f71edb04387f009afe39bfe0ffeff8fbf5e9) )

	ROM_REGION( 0x40000, "jsa:oki1", 0 )   /* 256k for ADPCM samples */
	ROM_LOAD( "136081-1061.7k",  0x00000, 0x10000, CRC(5b0468c6) SHA1(c910344622386e6be336fe04bc0be758ac6c59db) )
	ROM_LOAD( "136081-1062.7j",  0x10000, 0x10000, CRC(f73fe3cb) SHA1(547b5c4add617237c4c851751a27cda091fb7933) )
	ROM_LOAD( "136081-1063.7e",  0x20000, 0x10000, CRC(aa93421d) SHA1(f319057dadcb77a489d0bcffb24e0afe88adc769) )
	ROM_LOAD( "136081-1064.7d",  0x30000, 0x10000, CRC(33f045d5) SHA1(1fc7bedafeb3c1ffa0f115538cc300959c8d4601) )

	ROM_REGION( 0x0600, "proms", 0 )    /* microcode for growth renderer */
	ROM_LOAD( "136081-1040.30s",  0x0000, 0x0200, CRC(9b0f8b95) SHA1(d03987fe2d50a9f7769c600885bd1c7e1dee0789) )
	ROM_LOAD( "136081-1041.25s",  0x0200, 0x0200, CRC(f7ba6153) SHA1(d58792c9e9ea72d8f53f41ac1b420a86db6da3a3) )
	ROM_LOAD( "136081-1042.20s",  0x0400, 0x0200, CRC(3572fe68) SHA1(ab34ff337c16cd4d568cd2bd6a5063f5ed97368f) )

	ROM_REGION( 0x1200, "plds", 0 )
	ROM_LOAD( "gal16v8a-136081-1043.15e",  0x0000, 0x0117, CRC(649d9c0d) SHA1(bba2f3cf4607946340de27157c799f6b84c48584) )
	ROM_LOAD( "gal16v8a-136079-1045.40p",  0x0200, 0x0117, CRC(535c4d2e) SHA1(a428eef81723546566216e6af0db3baebda9a1f2) )
	ROM_LOAD( "gal16v8a-136079-1046.65d",  0x0400, 0x0117, CRC(38b22b2c) SHA1(cf9051129a790f3427e1c6d74a5c66f16acbd4be) )
	ROM_LOAD( "gal16v8a-136079-1047.40s",  0x0600, 0x0117, CRC(20839904) SHA1(134d1b379b7708ce33fbbf75e5c879dad971cd22) )
	ROM_LOAD( "gal16v8a-136079-1048.125f", 0x0800, 0x0117, CRC(d16e8d6e) SHA1(61cc270d80cb34f63fc407e41b55f95d59717757) )
	ROM_LOAD( "gal16v8a-136079-1049.120f", 0x0a00, 0x0117, CRC(68440ab1) SHA1(2f4eabac27d9a5778fcba0d9aa47460bcb153378) )
	ROM_LOAD( "gal16v8a-136079-1050.115f", 0x0c00, 0x0117, CRC(20dc296a) SHA1(9179fca2efcdd0782bb15cbf1923670806755bc8) )
	ROM_LOAD( "gal16v8a-136079-1051.105f", 0x0e00, 0x0117, CRC(45f0cb5f) SHA1(a7043231f68374871253da50cfd905519f283d9e) )
	ROM_LOAD( "gal16v8a-136079-1052.30n",  0x1000, 0x0117, CRC(6b3d2669) SHA1(143178d9c36a87aa58034faea2e4d4038ae565e7) )
ROM_END


ROM_START( pitfight6 )
	ROM_REGION( 0x80000, "maincpu", 0 ) /* 8*64k for 68000 code */
	ROM_LOAD16_BYTE( "136081-6028.05d", 0x00000, 0x10000, CRC(dae1d895) SHA1(d3852b932e318e3f6ab034aba8210d896f9f08da) )
	ROM_LOAD16_BYTE( "136081-6029.05b", 0x00001, 0x10000, CRC(4df13418) SHA1(e5469fbdd7263ca651d5cb7518576e4f9c4892e7) )
	ROM_LOAD16_BYTE( "136081-6030.15d", 0x20000, 0x10000, CRC(72b4b249) SHA1(295c707783ca40d6b68eb36b4511774e889bf447) )
	ROM_LOAD16_BYTE( "136081-6031.15b", 0x20001, 0x10000, CRC(f0c5d03b) SHA1(53aed44930ebaad98d833bc86837c57ac623937d) )

	ROM_REGION( 0x14000, "jsa:cpu", 0 ) /* 64k for 6502 code */
	ROM_LOAD( "136081-1060.1b", 0x10000, 0x4000, CRC(231d71d7) SHA1(24622eee5fe873ef81e1df2691bd7a1d3ea7ef6b) )
	ROM_CONTINUE(               0x04000, 0xc000 )

	ROM_REGION( 0x0a0000, "gfx1", 0 )
	ROM_LOAD( "136081-1017.130m", 0x000000, 0x10000, CRC(ad3cfea5) SHA1(7b6fec131230e84ab87b7fc95f08989916f30e02) ) /* playfield, planes 0-3 odd */
	ROM_LOAD( "136081-1018.120m", 0x010000, 0x10000, CRC(1a0f8bcf) SHA1(b965e73246db9507d2ad42dfcb033692b43b9b7a) )
	ROM_LOAD( "136081-1021.90m",  0x040000, 0x10000, CRC(777efee3) SHA1(07591f11685c4c75c24c55fd242878253d32481b) ) /* playfield, planes 0-3 even */
	ROM_LOAD( "136081-1022.75m",  0x050000, 0x10000, CRC(524319d0) SHA1(6f47a69d7d4e2a8f79b7470138e8b4edd6d0b2bb) )
	ROM_LOAD( "136081-1025.45m",  0x080000, 0x10000, CRC(fc41691a) SHA1(4ef2f9093f20d27e1ba7d218b90ff6abb1f33646) ) /* playfield plane 4 */

	ROM_REGION( 0x020000, "gfx2", 0 )
	ROM_LOAD( "136081-1027.15l",  0x000000, 0x10000, CRC(a59f381d) SHA1(b14e878340ad2adbf4f6d4fc331c58f62037c7c7) ) /* alphanumerics */

	ROM_REGION16_BE( 0x200000, "rle", 0 )
	ROM_LOAD16_BYTE( "136081-1065.65r", 0x000001, 0x80000, CRC(6d3a834f) SHA1(6e153fee39b6a932aeb101e69e3557355a637c76) ) /* Same data as other sets, but in four mask roms */
	ROM_LOAD16_BYTE( "136081-1066.65n", 0x000000, 0x80000, CRC(0f7f5117) SHA1(e9d3d0db388a5f7d76de363018d92fa59bf8113a) )
	ROM_LOAD16_BYTE( "136081-1067.70r", 0x100001, 0x80000, CRC(ca4f75a8) SHA1(f8b8b03df4ad043a48970a0f8a4c3b85c7140493) )
	ROM_LOAD16_BYTE( "136081-1068.70n", 0x100000, 0x80000, CRC(85240517) SHA1(f3d5c0803a7958569d2f3b9c25c73d33defcabe7) )

	ROM_REGION( 0x40000, "jsa:oki1", 0 )   /* 256k for ADPCM samples */
	ROM_LOAD( "136081-1061.7k",  0x00000, 0x10000, CRC(5b0468c6) SHA1(c910344622386e6be336fe04bc0be758ac6c59db) )
	ROM_LOAD( "136081-1062.7j",  0x10000, 0x10000, CRC(f73fe3cb) SHA1(547b5c4add617237c4c851751a27cda091fb7933) )
	ROM_LOAD( "136081-1063.7e",  0x20000, 0x10000, CRC(aa93421d) SHA1(f319057dadcb77a489d0bcffb24e0afe88adc769) )
	ROM_LOAD( "136081-1064.7d",  0x30000, 0x10000, CRC(33f045d5) SHA1(1fc7bedafeb3c1ffa0f115538cc300959c8d4601) )

	ROM_REGION( 0x0600, "proms", 0 )    /* microcode for growth renderer */
	ROM_LOAD( "136081-1040.30s",  0x0000, 0x0200, CRC(9b0f8b95) SHA1(d03987fe2d50a9f7769c600885bd1c7e1dee0789) )
	ROM_LOAD( "136081-1041.25s",  0x0200, 0x0200, CRC(f7ba6153) SHA1(d58792c9e9ea72d8f53f41ac1b420a86db6da3a3) )
	ROM_LOAD( "136081-1042.20s",  0x0400, 0x0200, CRC(3572fe68) SHA1(ab34ff337c16cd4d568cd2bd6a5063f5ed97368f) )

	ROM_REGION( 0x1200, "plds", 0 )
	ROM_LOAD( "gal16v8a-136081-1043.15e",  0x0000, 0x0117, CRC(649d9c0d) SHA1(bba2f3cf4607946340de27157c799f6b84c48584) )
	ROM_LOAD( "gal16v8a-136079-1045.40p",  0x0200, 0x0117, CRC(535c4d2e) SHA1(a428eef81723546566216e6af0db3baebda9a1f2) )
	ROM_LOAD( "gal16v8a-136079-1046.65d",  0x0400, 0x0117, CRC(38b22b2c) SHA1(cf9051129a790f3427e1c6d74a5c66f16acbd4be) )
	ROM_LOAD( "gal16v8a-136079-1047.40s",  0x0600, 0x0117, CRC(20839904) SHA1(134d1b379b7708ce33fbbf75e5c879dad971cd22) )
	ROM_LOAD( "gal16v8a-136079-1048.125f", 0x0800, 0x0117, CRC(d16e8d6e) SHA1(61cc270d80cb34f63fc407e41b55f95d59717757) )
	ROM_LOAD( "gal16v8a-136079-1049.120f", 0x0a00, 0x0117, CRC(68440ab1) SHA1(2f4eabac27d9a5778fcba0d9aa47460bcb153378) )
	ROM_LOAD( "gal16v8a-136079-1050.115f", 0x0c00, 0x0117, CRC(20dc296a) SHA1(9179fca2efcdd0782bb15cbf1923670806755bc8) )
	ROM_LOAD( "gal16v8a-136079-1051.105f", 0x0e00, 0x0117, CRC(45f0cb5f) SHA1(a7043231f68374871253da50cfd905519f283d9e) )
	ROM_LOAD( "gal16v8a-136079-1052.30n",  0x1000, 0x0117, CRC(6b3d2669) SHA1(143178d9c36a87aa58034faea2e4d4038ae565e7) )
ROM_END


ROM_START( pitfight5 )
	ROM_REGION( 0x80000, "maincpu", 0 ) /* 8*64k for 68000 code */
	ROM_LOAD16_BYTE( "136081-5028.05d", 0x00000, 0x10000, CRC(dd34b528) SHA1(806e01f3fd6a1335cdcfd2e41f04e9046fe433b5) )
	ROM_LOAD16_BYTE( "136081-5029.05b", 0x00001, 0x10000, CRC(b0ee9a09) SHA1(df85aeae2c497fbb22732704c2d581a3c195fcfb) )
	ROM_LOAD16_BYTE( "136081-5030.15d", 0x20000, 0x10000, CRC(6a094723) SHA1(a77046a8c5fab81cf0207122e494c32aab3b220d) )
	ROM_LOAD16_BYTE( "136081-5031.15b", 0x20001, 0x10000, CRC(47400d94) SHA1(07ba297a9b3ae574bc501a24fb6e46db7a5b3de5) )

	ROM_REGION( 0x14000, "jsa:cpu", 0 ) /* 64k for 6502 code */
	ROM_LOAD( "136081-1060.1b", 0x10000, 0x4000, CRC(231d71d7) SHA1(24622eee5fe873ef81e1df2691bd7a1d3ea7ef6b) )
	ROM_CONTINUE(               0x04000, 0xc000 )

	ROM_REGION( 0x0a0000, "gfx1", 0 )
	ROM_LOAD( "136081-1017.130m", 0x000000, 0x10000, CRC(ad3cfea5) SHA1(7b6fec131230e84ab87b7fc95f08989916f30e02) ) /* playfield, planes 0-3 odd */
	ROM_LOAD( "136081-1018.120m", 0x010000, 0x10000, CRC(1a0f8bcf) SHA1(b965e73246db9507d2ad42dfcb033692b43b9b7a) )
	ROM_LOAD( "136081-1021.90m",  0x040000, 0x10000, CRC(777efee3) SHA1(07591f11685c4c75c24c55fd242878253d32481b) ) /* playfield, planes 0-3 even */
	ROM_LOAD( "136081-1022.75m",  0x050000, 0x10000, CRC(524319d0) SHA1(6f47a69d7d4e2a8f79b7470138e8b4edd6d0b2bb) )
	ROM_LOAD( "136081-1025.45m",  0x080000, 0x10000, CRC(fc41691a) SHA1(4ef2f9093f20d27e1ba7d218b90ff6abb1f33646) ) /* playfield plane 4 */

	ROM_REGION( 0x020000, "gfx2", 0 )
	ROM_LOAD( "136081-1027.15l",  0x000000, 0x10000, CRC(a59f381d) SHA1(b14e878340ad2adbf4f6d4fc331c58f62037c7c7) ) /* alphanumerics */

	ROM_REGION16_BE( 0x200000, "rle", 0 )
	ROM_LOAD16_BYTE( "136081-1065.65r", 0x000001, 0x80000, CRC(6d3a834f) SHA1(6e153fee39b6a932aeb101e69e3557355a637c76) ) /* Same data as other sets, but in four mask roms */
	ROM_LOAD16_BYTE( "136081-1066.65n", 0x000000, 0x80000, CRC(0f7f5117) SHA1(e9d3d0db388a5f7d76de363018d92fa59bf8113a) )
	ROM_LOAD16_BYTE( "136081-1067.70r", 0x100001, 0x80000, CRC(ca4f75a8) SHA1(f8b8b03df4ad043a48970a0f8a4c3b85c7140493) )
	ROM_LOAD16_BYTE( "136081-1068.70n", 0x100000, 0x80000, CRC(85240517) SHA1(f3d5c0803a7958569d2f3b9c25c73d33defcabe7) )

	ROM_REGION( 0x40000, "jsa:oki1", 0 )   /* 256k for ADPCM samples */
	ROM_LOAD( "136081-1061.7k",  0x00000, 0x10000, CRC(5b0468c6) SHA1(c910344622386e6be336fe04bc0be758ac6c59db) )
	ROM_LOAD( "136081-1062.7j",  0x10000, 0x10000, CRC(f73fe3cb) SHA1(547b5c4add617237c4c851751a27cda091fb7933) )
	ROM_LOAD( "136081-1063.7e",  0x20000, 0x10000, CRC(aa93421d) SHA1(f319057dadcb77a489d0bcffb24e0afe88adc769) )
	ROM_LOAD( "136081-1064.7d",  0x30000, 0x10000, CRC(33f045d5) SHA1(1fc7bedafeb3c1ffa0f115538cc300959c8d4601) )

	ROM_REGION( 0x0600, "proms", 0 )    /* microcode for growth renderer */
	ROM_LOAD( "136081-1040.30s",  0x0000, 0x0200, CRC(9b0f8b95) SHA1(d03987fe2d50a9f7769c600885bd1c7e1dee0789) )
	ROM_LOAD( "136081-1041.25s",  0x0200, 0x0200, CRC(f7ba6153) SHA1(d58792c9e9ea72d8f53f41ac1b420a86db6da3a3) )
	ROM_LOAD( "136081-1042.20s",  0x0400, 0x0200, CRC(3572fe68) SHA1(ab34ff337c16cd4d568cd2bd6a5063f5ed97368f) )

	ROM_REGION( 0x1200, "plds", 0 )
	ROM_LOAD( "gal16v8a-136081-1043.15e",  0x0000, 0x0117, CRC(649d9c0d) SHA1(bba2f3cf4607946340de27157c799f6b84c48584) )
	ROM_LOAD( "gal16v8a-136079-1045.40p",  0x0200, 0x0117, CRC(535c4d2e) SHA1(a428eef81723546566216e6af0db3baebda9a1f2) )
	ROM_LOAD( "gal16v8a-136079-1046.65d",  0x0400, 0x0117, CRC(38b22b2c) SHA1(cf9051129a790f3427e1c6d74a5c66f16acbd4be) )
	ROM_LOAD( "gal16v8a-136079-1047.40s",  0x0600, 0x0117, CRC(20839904) SHA1(134d1b379b7708ce33fbbf75e5c879dad971cd22) )
	ROM_LOAD( "gal16v8a-136079-1048.125f", 0x0800, 0x0117, CRC(d16e8d6e) SHA1(61cc270d80cb34f63fc407e41b55f95d59717757) )
	ROM_LOAD( "gal16v8a-136079-1049.120f", 0x0a00, 0x0117, CRC(68440ab1) SHA1(2f4eabac27d9a5778fcba0d9aa47460bcb153378) )
	ROM_LOAD( "gal16v8a-136079-1050.115f", 0x0c00, 0x0117, CRC(20dc296a) SHA1(9179fca2efcdd0782bb15cbf1923670806755bc8) )
	ROM_LOAD( "gal16v8a-136079-1051.105f", 0x0e00, 0x0117, CRC(45f0cb5f) SHA1(a7043231f68374871253da50cfd905519f283d9e) )
	ROM_LOAD( "gal16v8a-136079-1052.30n",  0x1000, 0x0117, CRC(6b3d2669) SHA1(143178d9c36a87aa58034faea2e4d4038ae565e7) )
ROM_END


ROM_START( pitfight4 )
	ROM_REGION( 0x80000, "maincpu", 0 ) /* 8*64k for 68000 code */
	ROM_LOAD16_BYTE( "136081-4028.05d", 0x00000, 0x10000, CRC(f7cb1a4b) SHA1(024eb80d822559d9f3756710d1988f592639cd5e) )
	ROM_LOAD16_BYTE( "136081-4029.05b", 0x00001, 0x10000, CRC(13ae0d4f) SHA1(1816f40f7e3fee2427e11623c9f1d1b3515cbf72) )
	ROM_LOAD16_BYTE( "136081-3030.15d", 0x20000, 0x10000, CRC(b053e779) SHA1(f143f0e16850ad98366db208e956f7402d1ca848) )
	ROM_LOAD16_BYTE( "136081-3031.15b", 0x20001, 0x10000, CRC(2b8c4d13) SHA1(6f1679ef5974bf44848bfa6db0b9b05f71f6e7d6) )

	ROM_REGION( 0x14000, "jsa:cpu", 0 ) /* 64k for 6502 code */
	ROM_LOAD( "136081-1060.1b", 0x10000, 0x4000, CRC(231d71d7) SHA1(24622eee5fe873ef81e1df2691bd7a1d3ea7ef6b) )
	ROM_CONTINUE(               0x04000, 0xc000 )

	ROM_REGION( 0x0a0000, "gfx1", 0 )
	ROM_LOAD( "136081-1017.130m", 0x000000, 0x10000, CRC(ad3cfea5) SHA1(7b6fec131230e84ab87b7fc95f08989916f30e02) ) /* playfield, planes 0-3 odd */
	ROM_LOAD( "136081-1018.120m", 0x010000, 0x10000, CRC(1a0f8bcf) SHA1(b965e73246db9507d2ad42dfcb033692b43b9b7a) )
	ROM_LOAD( "136081-1021.90m",  0x040000, 0x10000, CRC(777efee3) SHA1(07591f11685c4c75c24c55fd242878253d32481b) ) /* playfield, planes 0-3 even */
	ROM_LOAD( "136081-1022.75m",  0x050000, 0x10000, CRC(524319d0) SHA1(6f47a69d7d4e2a8f79b7470138e8b4edd6d0b2bb) )
	ROM_LOAD( "136081-1025.45m",  0x080000, 0x10000, CRC(fc41691a) SHA1(4ef2f9093f20d27e1ba7d218b90ff6abb1f33646) ) /* playfield plane 4 */

	ROM_REGION( 0x020000, "gfx2", 0 )
	ROM_LOAD( "136081-1027.15l",  0x000000, 0x10000, CRC(a59f381d) SHA1(b14e878340ad2adbf4f6d4fc331c58f62037c7c7) ) /* alphanumerics */

	ROM_REGION16_BE( 0x200000, "rle", 0 )
	ROM_LOAD16_BYTE( "136081-1001.65r",  0x000001, 0x20000, CRC(3af31444) SHA1(91fc02786b82abdf12ebdbaacdd1f158f8ce6d06) )
	ROM_LOAD16_BYTE( "136081-1002.65n",  0x000000, 0x20000, CRC(f1d76a4c) SHA1(ca769d2cdd096f4a54f7bcaa4840fc9ffaabf499) )
	ROM_LOAD16_BYTE( "136081-1003.70r",  0x040001, 0x20000, CRC(28c41c2a) SHA1(75cd527f98c8475e3b880f53c5a355d6c3bd8766) )
	ROM_LOAD16_BYTE( "136081-1004.70n",  0x040000, 0x20000, CRC(977744da) SHA1(10f388eef999f702863e2b1a02a188df9a0a6086) )
	ROM_LOAD16_BYTE( "136081-1005.75r",  0x080001, 0x20000, CRC(ae59aef2) SHA1(44eda8a01016f6911b47df973b9427ff9f2ecef0) )
	ROM_LOAD16_BYTE( "136081-1006.75n",  0x080000, 0x20000, CRC(b6ccd77e) SHA1(f09a829143abd972699f3e671d4a1362fd362b19) )
	ROM_LOAD16_BYTE( "136081-1007.90r",  0x0c0001, 0x20000, CRC(ba33b0c0) SHA1(be09ba8796d3db2a859a2776336c1e9acbdaee95) )
	ROM_LOAD16_BYTE( "136081-1008.90n",  0x0c0000, 0x20000, CRC(09bd047c) SHA1(c283d526c7b08ce06e3981d1fc69433ea0dae657) )
	ROM_LOAD16_BYTE( "136081-1009.100r", 0x100001, 0x20000, CRC(ab85b00b) SHA1(6c60b9b58ff93ffdd1cffc49c3ea67400d56bf61) )
	ROM_LOAD16_BYTE( "136081-1010.100n", 0x100000, 0x20000, CRC(eca94bdc) SHA1(c474dfc9dc4460cd2f40cc6012214e760b133c0d) )
	ROM_LOAD16_BYTE( "136081-1011.115r", 0x140001, 0x20000, CRC(a86582fd) SHA1(8b557622c53c8ff388248ea54d3cc85b0d77cafb) )
	ROM_LOAD16_BYTE( "136081-1012.115n", 0x140000, 0x20000, CRC(efd1152d) SHA1(77d1752b76b079c9e834a8b73e8601873d3afdbe) )
	ROM_LOAD16_BYTE( "136081-1013.120r", 0x180001, 0x20000, CRC(a141379e) SHA1(d4c98b364495e19e2e7bee5431834f38ba20a514) )
	ROM_LOAD16_BYTE( "136081-1014.120n", 0x180000, 0x20000, CRC(93bfcc15) SHA1(4d8f6c8c279533b2fc35f87c3eb5ed614cb03248) )
	ROM_LOAD16_BYTE( "136081-1015.130r", 0x1c0001, 0x20000, CRC(9378ad0b) SHA1(909f9879f5b8fc3ed0622fd27d903ccb1f7a90c6) )
	ROM_LOAD16_BYTE( "136081-1016.130n", 0x1c0000, 0x20000, CRC(19c3fbe0) SHA1(ba28f71edb04387f009afe39bfe0ffeff8fbf5e9) )

	ROM_REGION( 0x40000, "jsa:oki1", 0 )   /* 256k for ADPCM samples */
	ROM_LOAD( "136081-1061.7k",  0x00000, 0x10000, CRC(5b0468c6) SHA1(c910344622386e6be336fe04bc0be758ac6c59db) )
	ROM_LOAD( "136081-1062.7j",  0x10000, 0x10000, CRC(f73fe3cb) SHA1(547b5c4add617237c4c851751a27cda091fb7933) )
	ROM_LOAD( "136081-1063.7e",  0x20000, 0x10000, CRC(aa93421d) SHA1(f319057dadcb77a489d0bcffb24e0afe88adc769) )
	ROM_LOAD( "136081-1064.7d",  0x30000, 0x10000, CRC(33f045d5) SHA1(1fc7bedafeb3c1ffa0f115538cc300959c8d4601) )

	ROM_REGION( 0x0600, "proms", 0 )    /* microcode for growth renderer */
	ROM_LOAD( "136081-1040.30s",  0x0000, 0x0200, CRC(9b0f8b95) SHA1(d03987fe2d50a9f7769c600885bd1c7e1dee0789) )
	ROM_LOAD( "136081-1041.25s",  0x0200, 0x0200, CRC(f7ba6153) SHA1(d58792c9e9ea72d8f53f41ac1b420a86db6da3a3) )
	ROM_LOAD( "136081-1042.20s",  0x0400, 0x0200, CRC(3572fe68) SHA1(ab34ff337c16cd4d568cd2bd6a5063f5ed97368f) )

	ROM_REGION( 0x1200, "plds", 0 )
	ROM_LOAD( "gal16v8a-136081-1043.15e",  0x0000, 0x0117, CRC(649d9c0d) SHA1(bba2f3cf4607946340de27157c799f6b84c48584) )
	ROM_LOAD( "gal16v8a-136079-1045.40p",  0x0200, 0x0117, CRC(535c4d2e) SHA1(a428eef81723546566216e6af0db3baebda9a1f2) )
	ROM_LOAD( "gal16v8a-136079-1046.65d",  0x0400, 0x0117, CRC(38b22b2c) SHA1(cf9051129a790f3427e1c6d74a5c66f16acbd4be) )
	ROM_LOAD( "gal16v8a-136079-1047.40s",  0x0600, 0x0117, CRC(20839904) SHA1(134d1b379b7708ce33fbbf75e5c879dad971cd22) )
	ROM_LOAD( "gal16v8a-136079-1048.125f", 0x0800, 0x0117, CRC(d16e8d6e) SHA1(61cc270d80cb34f63fc407e41b55f95d59717757) )
	ROM_LOAD( "gal16v8a-136079-1049.120f", 0x0a00, 0x0117, CRC(68440ab1) SHA1(2f4eabac27d9a5778fcba0d9aa47460bcb153378) )
	ROM_LOAD( "gal16v8a-136079-1050.115f", 0x0c00, 0x0117, CRC(20dc296a) SHA1(9179fca2efcdd0782bb15cbf1923670806755bc8) )
	ROM_LOAD( "gal16v8a-136079-1051.105f", 0x0e00, 0x0117, CRC(45f0cb5f) SHA1(a7043231f68374871253da50cfd905519f283d9e) )
	ROM_LOAD( "gal16v8a-136079-1052.30n",  0x1000, 0x0117, CRC(6b3d2669) SHA1(143178d9c36a87aa58034faea2e4d4038ae565e7) )
ROM_END


ROM_START( pitfight3 )
	ROM_REGION( 0x80000, "maincpu", 0 ) /* 8*64k for 68000 code */
	ROM_LOAD16_BYTE( "136081-3028.05d", 0x00000, 0x10000, CRC(99530da4) SHA1(b542910127553d285f699d2b75e5d6fb071ff046) )
	ROM_LOAD16_BYTE( "136081-3029.05b", 0x00001, 0x10000, CRC(78c7afbf) SHA1(7588dfee1e120b69591499ddf2860490b1c66885) )
	ROM_LOAD16_BYTE( "136081-3030.15d", 0x20000, 0x10000, CRC(b053e779) SHA1(f143f0e16850ad98366db208e956f7402d1ca848) )
	ROM_LOAD16_BYTE( "136081-3031.15b", 0x20001, 0x10000, CRC(2b8c4d13) SHA1(6f1679ef5974bf44848bfa6db0b9b05f71f6e7d6) )

	ROM_REGION( 0x14000, "jsa:cpu", 0 ) /* 64k for 6502 code */
	ROM_LOAD( "136081-1060.1b", 0x10000, 0x4000, CRC(231d71d7) SHA1(24622eee5fe873ef81e1df2691bd7a1d3ea7ef6b) )
	ROM_CONTINUE(               0x04000, 0xc000 )

	ROM_REGION( 0x0a0000, "gfx1", 0 )
	ROM_LOAD( "136081-1017.130m", 0x000000, 0x10000, CRC(ad3cfea5) SHA1(7b6fec131230e84ab87b7fc95f08989916f30e02) ) /* playfield, planes 0-3 odd */
	ROM_LOAD( "136081-1018.120m", 0x010000, 0x10000, CRC(1a0f8bcf) SHA1(b965e73246db9507d2ad42dfcb033692b43b9b7a) )
	ROM_LOAD( "136081-1021.90m",  0x040000, 0x10000, CRC(777efee3) SHA1(07591f11685c4c75c24c55fd242878253d32481b) ) /* playfield, planes 0-3 even */
	ROM_LOAD( "136081-1022.75m",  0x050000, 0x10000, CRC(524319d0) SHA1(6f47a69d7d4e2a8f79b7470138e8b4edd6d0b2bb) )
	ROM_LOAD( "136081-1025.45m",  0x080000, 0x10000, CRC(fc41691a) SHA1(4ef2f9093f20d27e1ba7d218b90ff6abb1f33646) ) /* playfield plane 4 */

	ROM_REGION( 0x020000, "gfx2", 0 )
	ROM_LOAD( "136081-1027.15l",  0x000000, 0x10000, CRC(a59f381d) SHA1(b14e878340ad2adbf4f6d4fc331c58f62037c7c7) ) /* alphanumerics */

	ROM_REGION16_BE( 0x200000, "rle", 0 )
	ROM_LOAD16_BYTE( "136081-1001.65r",  0x000001, 0x20000, CRC(3af31444) SHA1(91fc02786b82abdf12ebdbaacdd1f158f8ce6d06) )
	ROM_LOAD16_BYTE( "136081-1002.65n",  0x000000, 0x20000, CRC(f1d76a4c) SHA1(ca769d2cdd096f4a54f7bcaa4840fc9ffaabf499) )
	ROM_LOAD16_BYTE( "136081-1003.70r",  0x040001, 0x20000, CRC(28c41c2a) SHA1(75cd527f98c8475e3b880f53c5a355d6c3bd8766) )
	ROM_LOAD16_BYTE( "136081-1004.70n",  0x040000, 0x20000, CRC(977744da) SHA1(10f388eef999f702863e2b1a02a188df9a0a6086) )
	ROM_LOAD16_BYTE( "136081-1005.75r",  0x080001, 0x20000, CRC(ae59aef2) SHA1(44eda8a01016f6911b47df973b9427ff9f2ecef0) )
	ROM_LOAD16_BYTE( "136081-1006.75n",  0x080000, 0x20000, CRC(b6ccd77e) SHA1(f09a829143abd972699f3e671d4a1362fd362b19) )
	ROM_LOAD16_BYTE( "136081-1007.90r",  0x0c0001, 0x20000, CRC(ba33b0c0) SHA1(be09ba8796d3db2a859a2776336c1e9acbdaee95) )
	ROM_LOAD16_BYTE( "136081-1008.90n",  0x0c0000, 0x20000, CRC(09bd047c) SHA1(c283d526c7b08ce06e3981d1fc69433ea0dae657) )
	ROM_LOAD16_BYTE( "136081-1009.100r", 0x100001, 0x20000, CRC(ab85b00b) SHA1(6c60b9b58ff93ffdd1cffc49c3ea67400d56bf61) )
	ROM_LOAD16_BYTE( "136081-1010.100n", 0x100000, 0x20000, CRC(eca94bdc) SHA1(c474dfc9dc4460cd2f40cc6012214e760b133c0d) )
	ROM_LOAD16_BYTE( "136081-1011.115r", 0x140001, 0x20000, CRC(a86582fd) SHA1(8b557622c53c8ff388248ea54d3cc85b0d77cafb) )
	ROM_LOAD16_BYTE( "136081-1012.115n", 0x140000, 0x20000, CRC(efd1152d) SHA1(77d1752b76b079c9e834a8b73e8601873d3afdbe) )
	ROM_LOAD16_BYTE( "136081-1013.120r", 0x180001, 0x20000, CRC(a141379e) SHA1(d4c98b364495e19e2e7bee5431834f38ba20a514) )
	ROM_LOAD16_BYTE( "136081-1014.120n", 0x180000, 0x20000, CRC(93bfcc15) SHA1(4d8f6c8c279533b2fc35f87c3eb5ed614cb03248) )
	ROM_LOAD16_BYTE( "136081-1015.130r", 0x1c0001, 0x20000, CRC(9378ad0b) SHA1(909f9879f5b8fc3ed0622fd27d903ccb1f7a90c6) )
	ROM_LOAD16_BYTE( "136081-1016.130n", 0x1c0000, 0x20000, CRC(19c3fbe0) SHA1(ba28f71edb04387f009afe39bfe0ffeff8fbf5e9) )

	ROM_REGION( 0x40000, "jsa:oki1", 0 )   /* 256k for ADPCM samples */
	ROM_LOAD( "136081-1061.7k",  0x00000, 0x10000, CRC(5b0468c6) SHA1(c910344622386e6be336fe04bc0be758ac6c59db) )
	ROM_LOAD( "136081-1062.7j",  0x10000, 0x10000, CRC(f73fe3cb) SHA1(547b5c4add617237c4c851751a27cda091fb7933) )
	ROM_LOAD( "136081-1063.7e",  0x20000, 0x10000, CRC(aa93421d) SHA1(f319057dadcb77a489d0bcffb24e0afe88adc769) )
	ROM_LOAD( "136081-1064.7d",  0x30000, 0x10000, CRC(33f045d5) SHA1(1fc7bedafeb3c1ffa0f115538cc300959c8d4601) )

	ROM_REGION( 0x0600, "proms", 0 )    /* microcode for growth renderer */
	ROM_LOAD( "136081-1040.30s",  0x0000, 0x0200, CRC(9b0f8b95) SHA1(d03987fe2d50a9f7769c600885bd1c7e1dee0789) )
	ROM_LOAD( "136081-1041.25s",  0x0200, 0x0200, CRC(f7ba6153) SHA1(d58792c9e9ea72d8f53f41ac1b420a86db6da3a3) )
	ROM_LOAD( "136081-1042.20s",  0x0400, 0x0200, CRC(3572fe68) SHA1(ab34ff337c16cd4d568cd2bd6a5063f5ed97368f) )

	ROM_REGION( 0x1200, "plds", 0 )
	ROM_LOAD( "gal16v8a-136081-1043.15e",  0x0000, 0x0117, CRC(649d9c0d) SHA1(bba2f3cf4607946340de27157c799f6b84c48584) )
	ROM_LOAD( "gal16v8a-136079-1045.40p",  0x0200, 0x0117, CRC(535c4d2e) SHA1(a428eef81723546566216e6af0db3baebda9a1f2) )
	ROM_LOAD( "gal16v8a-136079-1046.65d",  0x0400, 0x0117, CRC(38b22b2c) SHA1(cf9051129a790f3427e1c6d74a5c66f16acbd4be) )
	ROM_LOAD( "gal16v8a-136079-1047.40s",  0x0600, 0x0117, CRC(20839904) SHA1(134d1b379b7708ce33fbbf75e5c879dad971cd22) )
	ROM_LOAD( "gal16v8a-136079-1048.125f", 0x0800, 0x0117, CRC(d16e8d6e) SHA1(61cc270d80cb34f63fc407e41b55f95d59717757) )
	ROM_LOAD( "gal16v8a-136079-1049.120f", 0x0a00, 0x0117, CRC(68440ab1) SHA1(2f4eabac27d9a5778fcba0d9aa47460bcb153378) )
	ROM_LOAD( "gal16v8a-136079-1050.115f", 0x0c00, 0x0117, CRC(20dc296a) SHA1(9179fca2efcdd0782bb15cbf1923670806755bc8) )
	ROM_LOAD( "gal16v8a-136079-1051.105f", 0x0e00, 0x0117, CRC(45f0cb5f) SHA1(a7043231f68374871253da50cfd905519f283d9e) )
	ROM_LOAD( "gal16v8a-136079-1052.30n",  0x1000, 0x0117, CRC(6b3d2669) SHA1(143178d9c36a87aa58034faea2e4d4038ae565e7) )
ROM_END


ROM_START( pitfightj )
	ROM_REGION( 0x80000, "maincpu", 0 ) /* 8*64k for 68000 code */
	ROM_LOAD16_BYTE( "136081-3428.05d", 0x00000, 0x10000, CRC(39be5484) SHA1(683ab8cf21f2b82aee11ce21b9dfbdb82083d6b6) )
	ROM_LOAD16_BYTE( "136081-3429.05b", 0x00001, 0x10000, CRC(2cb14a58) SHA1(004178b4869766c11904d1fdf72725ba481bc8cc) )
	ROM_LOAD16_BYTE( "136081-3430.15d", 0x20000, 0x10000, CRC(80707ac0) SHA1(39ddd228bb630bbdf32c76c7906e54f6a62c06ad) )
	ROM_LOAD16_BYTE( "136081-3431.15b", 0x20001, 0x10000, CRC(9bf43aa6) SHA1(b41c30118a0c0032303d1b1de471aac292a4968a) )

	ROM_REGION( 0x14000, "jsa:cpu", 0 ) /* 64k for 6502 code */
	ROM_LOAD( "136081-2060.1b", 0x10000, 0x4000, CRC(4317a9f3) SHA1(310154be47fd16b417699338e04e08f3ed973198) )
	ROM_CONTINUE(               0x04000, 0xc000 )

	ROM_REGION( 0x0a0000, "gfx1", 0 )
	ROM_LOAD( "136081-1017.130m", 0x000000, 0x10000, CRC(ad3cfea5) SHA1(7b6fec131230e84ab87b7fc95f08989916f30e02) ) /* playfield, planes 0-3 odd */
	ROM_LOAD( "136081-1018.120m", 0x010000, 0x10000, CRC(1a0f8bcf) SHA1(b965e73246db9507d2ad42dfcb033692b43b9b7a) )
	ROM_LOAD( "136081-1021.90m",  0x040000, 0x10000, CRC(777efee3) SHA1(07591f11685c4c75c24c55fd242878253d32481b) ) /* playfield, planes 0-3 even */
	ROM_LOAD( "136081-1022.75m",  0x050000, 0x10000, CRC(524319d0) SHA1(6f47a69d7d4e2a8f79b7470138e8b4edd6d0b2bb) )
	ROM_LOAD( "136081-1025.45m",  0x080000, 0x10000, CRC(fc41691a) SHA1(4ef2f9093f20d27e1ba7d218b90ff6abb1f33646) ) /* playfield plane 4 */

	ROM_REGION( 0x020000, "gfx2", 0 )
	ROM_LOAD( "136081-1427.dat",  0x000000, 0x20000, CRC(b2c51dff) SHA1(7ad82a6a55d3a68e39d113c92f9e89a43408b5b2) ) /* alphanumerics */

	ROM_REGION16_BE( 0x200000, "rle", 0 )
	ROM_LOAD16_BYTE( "136081-1001.65r",  0x000001, 0x20000, CRC(3af31444) SHA1(91fc02786b82abdf12ebdbaacdd1f158f8ce6d06) )
	ROM_LOAD16_BYTE( "136081-1002.65n",  0x000000, 0x20000, CRC(f1d76a4c) SHA1(ca769d2cdd096f4a54f7bcaa4840fc9ffaabf499) )
	ROM_LOAD16_BYTE( "136081-1003.70r",  0x040001, 0x20000, CRC(28c41c2a) SHA1(75cd527f98c8475e3b880f53c5a355d6c3bd8766) )
	ROM_LOAD16_BYTE( "136081-1004.70n",  0x040000, 0x20000, CRC(977744da) SHA1(10f388eef999f702863e2b1a02a188df9a0a6086) )
	ROM_LOAD16_BYTE( "136081-1005.75r",  0x080001, 0x20000, CRC(ae59aef2) SHA1(44eda8a01016f6911b47df973b9427ff9f2ecef0) )
	ROM_LOAD16_BYTE( "136081-1006.75n",  0x080000, 0x20000, CRC(b6ccd77e) SHA1(f09a829143abd972699f3e671d4a1362fd362b19) )
	ROM_LOAD16_BYTE( "136081-1007.90r",  0x0c0001, 0x20000, CRC(ba33b0c0) SHA1(be09ba8796d3db2a859a2776336c1e9acbdaee95) )
	ROM_LOAD16_BYTE( "136081-1008.90n",  0x0c0000, 0x20000, CRC(09bd047c) SHA1(c283d526c7b08ce06e3981d1fc69433ea0dae657) )
	ROM_LOAD16_BYTE( "136081-1009.100r", 0x100001, 0x20000, CRC(ab85b00b) SHA1(6c60b9b58ff93ffdd1cffc49c3ea67400d56bf61) )
	ROM_LOAD16_BYTE( "136081-1010.100n", 0x100000, 0x20000, CRC(eca94bdc) SHA1(c474dfc9dc4460cd2f40cc6012214e760b133c0d) )
	ROM_LOAD16_BYTE( "136081-1011.115r", 0x140001, 0x20000, CRC(a86582fd) SHA1(8b557622c53c8ff388248ea54d3cc85b0d77cafb) )
	ROM_LOAD16_BYTE( "136081-1012.115n", 0x140000, 0x20000, CRC(efd1152d) SHA1(77d1752b76b079c9e834a8b73e8601873d3afdbe) )
	ROM_LOAD16_BYTE( "136081-1013.120r", 0x180001, 0x20000, CRC(a141379e) SHA1(d4c98b364495e19e2e7bee5431834f38ba20a514) )
	ROM_LOAD16_BYTE( "136081-1014.120n", 0x180000, 0x20000, CRC(93bfcc15) SHA1(4d8f6c8c279533b2fc35f87c3eb5ed614cb03248) )
	ROM_LOAD16_BYTE( "136081-1015.130r", 0x1c0001, 0x20000, CRC(9378ad0b) SHA1(909f9879f5b8fc3ed0622fd27d903ccb1f7a90c6) )
	ROM_LOAD16_BYTE( "136081-1016.130n", 0x1c0000, 0x20000, CRC(19c3fbe0) SHA1(ba28f71edb04387f009afe39bfe0ffeff8fbf5e9) )

	ROM_REGION( 0x40000, "jsa:oki1", 0 )   /* 256k for ADPCM samples */
	ROM_LOAD( "136081-1061.7k",  0x00000, 0x10000, CRC(5b0468c6) SHA1(c910344622386e6be336fe04bc0be758ac6c59db) )
	ROM_LOAD( "136081-1062.7j",  0x10000, 0x10000, CRC(f73fe3cb) SHA1(547b5c4add617237c4c851751a27cda091fb7933) )
	ROM_LOAD( "136081-1063.7e",  0x20000, 0x10000, CRC(aa93421d) SHA1(f319057dadcb77a489d0bcffb24e0afe88adc769) )
	ROM_LOAD( "136081-1064.7d",  0x30000, 0x10000, CRC(33f045d5) SHA1(1fc7bedafeb3c1ffa0f115538cc300959c8d4601) )

	ROM_REGION( 0x0600, "proms", 0 )    /* microcode for growth renderer */
	ROM_LOAD( "136081-1040.30s",  0x0000, 0x0200, CRC(9b0f8b95) SHA1(d03987fe2d50a9f7769c600885bd1c7e1dee0789) )
	ROM_LOAD( "136081-1041.25s",  0x0200, 0x0200, CRC(f7ba6153) SHA1(d58792c9e9ea72d8f53f41ac1b420a86db6da3a3) )
	ROM_LOAD( "136081-1042.20s",  0x0400, 0x0200, CRC(3572fe68) SHA1(ab34ff337c16cd4d568cd2bd6a5063f5ed97368f) )

	ROM_REGION( 0x1200, "plds", 0 )
	ROM_LOAD( "gal16v8a-136081-1043.15e",  0x0000, 0x0117, CRC(649d9c0d) SHA1(bba2f3cf4607946340de27157c799f6b84c48584) )
	ROM_LOAD( "gal16v8a-136079-1045.40p",  0x0200, 0x0117, CRC(535c4d2e) SHA1(a428eef81723546566216e6af0db3baebda9a1f2) )
	ROM_LOAD( "gal16v8a-136079-1046.65d",  0x0400, 0x0117, CRC(38b22b2c) SHA1(cf9051129a790f3427e1c6d74a5c66f16acbd4be) )
	ROM_LOAD( "gal16v8a-136079-1047.40s",  0x0600, 0x0117, CRC(20839904) SHA1(134d1b379b7708ce33fbbf75e5c879dad971cd22) )
	ROM_LOAD( "gal16v8a-136079-1048.125f", 0x0800, 0x0117, CRC(d16e8d6e) SHA1(61cc270d80cb34f63fc407e41b55f95d59717757) )
	ROM_LOAD( "gal16v8a-136079-1049.120f", 0x0a00, 0x0117, CRC(68440ab1) SHA1(2f4eabac27d9a5778fcba0d9aa47460bcb153378) )
	ROM_LOAD( "gal16v8a-136079-1050.115f", 0x0c00, 0x0117, CRC(20dc296a) SHA1(9179fca2efcdd0782bb15cbf1923670806755bc8) )
	ROM_LOAD( "gal16v8a-136079-1051.105f", 0x0e00, 0x0117, CRC(45f0cb5f) SHA1(a7043231f68374871253da50cfd905519f283d9e) )
	ROM_LOAD( "gal16v8a-136079-1052.30n",  0x1000, 0x0117, CRC(6b3d2669) SHA1(143178d9c36a87aa58034faea2e4d4038ae565e7) )
ROM_END


ROM_START( pitfightb )
	ROM_REGION( 0x80000, "maincpu", 0 ) /* 8*64k for 68000 code */
	ROM_LOAD16_BYTE( "pit9.bin", 0x00000, 0x10000, CRC(946fb15b) SHA1(dbde8daf60a6bda242ea0d627c1fe5034de18090) )
	ROM_LOAD16_BYTE( "pit7.bin", 0x00001, 0x10000, CRC(a9e7163a) SHA1(d1536bde0df20fb5f78e5ce55d453cb0c2c0b663) )
	ROM_LOAD16_BYTE( "pit8.bin", 0x20000, 0x10000, CRC(b74a8258) SHA1(779990ed95c25dd0a8e9f30c4d9a8d69162d14fc) )
	ROM_LOAD16_BYTE( "pit6.bin", 0x20001, 0x10000, CRC(40204ecd) SHA1(73d827e119cc1408356e28c1e67f6c8e287eeb15) )

	ROM_REGION( 0x14000, "jsa:cpu", 0 ) /* 64k for 6502 code */
	ROM_LOAD( "136081-1060.1b", 0x10000, 0x4000, CRC(231d71d7) SHA1(24622eee5fe873ef81e1df2691bd7a1d3ea7ef6b) )
	ROM_CONTINUE(               0x04000, 0xc000 )

	ROM_REGION( 0x0a0000, "gfx1", 0 )
	ROM_LOAD( "136081-1017.130m", 0x000000, 0x10000, CRC(ad3cfea5) SHA1(7b6fec131230e84ab87b7fc95f08989916f30e02) ) /* playfield, planes 0-3 odd */
	ROM_LOAD( "136081-1018.120m", 0x010000, 0x10000, CRC(1a0f8bcf) SHA1(b965e73246db9507d2ad42dfcb033692b43b9b7a) )
	ROM_LOAD( "136081-1021.90m",  0x040000, 0x10000, CRC(777efee3) SHA1(07591f11685c4c75c24c55fd242878253d32481b) ) /* playfield, planes 0-3 even */
	ROM_LOAD( "136081-1022.75m",  0x050000, 0x10000, CRC(524319d0) SHA1(6f47a69d7d4e2a8f79b7470138e8b4edd6d0b2bb) )
	ROM_LOAD( "136081-1025.45m",  0x080000, 0x10000, CRC(fc41691a) SHA1(4ef2f9093f20d27e1ba7d218b90ff6abb1f33646) ) /* playfield plane 4 */

	ROM_REGION( 0x020000, "gfx2", 0 )
	ROM_LOAD( "136081-1027.15l",  0x000000, 0x10000, CRC(a59f381d) SHA1(b14e878340ad2adbf4f6d4fc331c58f62037c7c7) ) /* alphanumerics */

	ROM_REGION16_BE( 0x200000, "rle", 0 )
	ROM_LOAD16_BYTE( "136081-1001.65r",  0x000001, 0x20000, CRC(3af31444) SHA1(91fc02786b82abdf12ebdbaacdd1f158f8ce6d06) )
	ROM_LOAD16_BYTE( "136081-1002.65n",  0x000000, 0x20000, CRC(f1d76a4c) SHA1(ca769d2cdd096f4a54f7bcaa4840fc9ffaabf499) )
	ROM_LOAD16_BYTE( "136081-1003.70r",  0x040001, 0x20000, CRC(28c41c2a) SHA1(75cd527f98c8475e3b880f53c5a355d6c3bd8766) )
	ROM_LOAD16_BYTE( "136081-1004.70n",  0x040000, 0x20000, CRC(977744da) SHA1(10f388eef999f702863e2b1a02a188df9a0a6086) )
	ROM_LOAD16_BYTE( "136081-1005.75r",  0x080001, 0x20000, CRC(ae59aef2) SHA1(44eda8a01016f6911b47df973b9427ff9f2ecef0) )
	ROM_LOAD16_BYTE( "136081-1006.75n",  0x080000, 0x20000, CRC(b6ccd77e) SHA1(f09a829143abd972699f3e671d4a1362fd362b19) )
	ROM_LOAD16_BYTE( "136081-1007.90r",  0x0c0001, 0x20000, CRC(ba33b0c0) SHA1(be09ba8796d3db2a859a2776336c1e9acbdaee95) )
	ROM_LOAD16_BYTE( "136081-1008.90n",  0x0c0000, 0x20000, CRC(09bd047c) SHA1(c283d526c7b08ce06e3981d1fc69433ea0dae657) )
	ROM_LOAD16_BYTE( "136081-1009.100r", 0x100001, 0x20000, CRC(ab85b00b) SHA1(6c60b9b58ff93ffdd1cffc49c3ea67400d56bf61) )
	ROM_LOAD16_BYTE( "136081-1010.100n", 0x100000, 0x20000, CRC(eca94bdc) SHA1(c474dfc9dc4460cd2f40cc6012214e760b133c0d) )
	ROM_LOAD16_BYTE( "136081-1011.115r", 0x140001, 0x20000, CRC(a86582fd) SHA1(8b557622c53c8ff388248ea54d3cc85b0d77cafb) )
	ROM_LOAD16_BYTE( "136081-1012.115n", 0x140000, 0x20000, CRC(efd1152d) SHA1(77d1752b76b079c9e834a8b73e8601873d3afdbe) )
	ROM_LOAD16_BYTE( "136081-1013.120r", 0x180001, 0x20000, CRC(a141379e) SHA1(d4c98b364495e19e2e7bee5431834f38ba20a514) )
	ROM_LOAD16_BYTE( "136081-1014.120n", 0x180000, 0x20000, CRC(93bfcc15) SHA1(4d8f6c8c279533b2fc35f87c3eb5ed614cb03248) )
	ROM_LOAD16_BYTE( "136081-1015.130r", 0x1c0001, 0x20000, CRC(9378ad0b) SHA1(909f9879f5b8fc3ed0622fd27d903ccb1f7a90c6) )
	ROM_LOAD16_BYTE( "136081-1016.130n", 0x1c0000, 0x20000, CRC(19c3fbe0) SHA1(ba28f71edb04387f009afe39bfe0ffeff8fbf5e9) )

	ROM_REGION( 0x40000, "jsa:oki1", 0 )   /* 256k for ADPCM samples */
	ROM_LOAD( "136081-1061.7k",  0x00000, 0x10000, CRC(5b0468c6) SHA1(c910344622386e6be336fe04bc0be758ac6c59db) )
	ROM_LOAD( "136081-1062.7j",  0x10000, 0x10000, CRC(f73fe3cb) SHA1(547b5c4add617237c4c851751a27cda091fb7933) )
	ROM_LOAD( "136081-1063.7e",  0x20000, 0x10000, CRC(aa93421d) SHA1(f319057dadcb77a489d0bcffb24e0afe88adc769) )
	ROM_LOAD( "136081-1064.7d",  0x30000, 0x10000, CRC(33f045d5) SHA1(1fc7bedafeb3c1ffa0f115538cc300959c8d4601) )

	ROM_REGION( 0x0600, "proms", 0 )    /* microcode for growth renderer */
	ROM_LOAD( "136081-1040.30s",  0x0000, 0x0200, CRC(9b0f8b95) SHA1(d03987fe2d50a9f7769c600885bd1c7e1dee0789) )
	ROM_LOAD( "136081-1041.25s",  0x0200, 0x0200, CRC(f7ba6153) SHA1(d58792c9e9ea72d8f53f41ac1b420a86db6da3a3) )
	ROM_LOAD( "136081-1042.20s",  0x0400, 0x0200, CRC(3572fe68) SHA1(ab34ff337c16cd4d568cd2bd6a5063f5ed97368f) )
ROM_END



/*************************************
 *
 *  Driver initialization
 *
 *************************************/

void atarig1_state::init_common(offs_t slapstic_base, int slapstic, bool is_pitfight)
{
	if (slapstic == -1)
	{
		pitfightb_cheap_slapstic_init();
		save_item(NAME(m_bslapstic_bank));
		save_item(NAME(m_bslapstic_primed));
	}
	else if (slapstic != 0)
		slapstic_configure(*m_maincpu, slapstic_base, 0, slapstic);

	m_is_pitfight = is_pitfight;
}

DRIVER_INIT_MEMBER(atarig1_state,hydra)     { init_common(0x078000, 116, 0); }
DRIVER_INIT_MEMBER(atarig1_state,hydrap)    { init_common(0x000000,   0, 0); }

DRIVER_INIT_MEMBER(atarig1_state,pitfight9)  { init_common(0x038000, 114, 1); }
DRIVER_INIT_MEMBER(atarig1_state,pitfight7)  { init_common(0x038000, 112, 1); }
DRIVER_INIT_MEMBER(atarig1_state,pitfight)   { init_common(0x038000, 111, 1); }
DRIVER_INIT_MEMBER(atarig1_state,pitfightj)  { init_common(0x038000, 113, 1); }
DRIVER_INIT_MEMBER(atarig1_state,pitfightb)  { init_common(0x038000,  -1, 1); }


/*************************************
 *
 *  Game driver(s)
 *
 *************************************/

GAME( 1990, hydra,    0,        hydra, hydra, atarig1_state,    hydra,    ROT0, "Atari Games", "Hydra", MACHINE_SUPPORTS_SAVE )
GAME( 1990, hydrap,   hydra,    hydra, hydra, atarig1_state,    hydrap,   ROT0, "Atari Games", "Hydra (prototype 5/14/90)", MACHINE_SUPPORTS_SAVE )
GAME( 1990, hydrap2,  hydra,    hydra, hydra, atarig1_state,    hydrap,   ROT0, "Atari Games", "Hydra (prototype 5/25/90)", MACHINE_SUPPORTS_SAVE )

GAME( 1990, pitfight,  0,        pitfight, pitfight, atarig1_state, pitfight9, ROT0, "Atari Games", "Pit Fighter (rev 9)", MACHINE_SUPPORTS_SAVE )
GAME( 1990, pitfight7, pitfight, pitfight, pitfight, atarig1_state, pitfight7, ROT0, "Atari Games", "Pit Fighter (rev 7)", MACHINE_SUPPORTS_SAVE )
GAME( 1990, pitfight6, pitfight, pitfight, pitfight, atarig1_state, pitfightj, ROT0, "Atari Games", "Pit Fighter (rev 6)", MACHINE_SUPPORTS_SAVE )
GAME( 1990, pitfight5, pitfight, pitfight, pitfight, atarig1_state, pitfight7, ROT0, "Atari Games", "Pit Fighter (rev 5)", MACHINE_SUPPORTS_SAVE )
GAME( 1990, pitfight4, pitfight, pitfight, pitfight, atarig1_state, pitfight,  ROT0, "Atari Games", "Pit Fighter (rev 4)", MACHINE_SUPPORTS_SAVE )
GAME( 1990, pitfight3, pitfight, pitfight, pitfight, atarig1_state, pitfight,  ROT0, "Atari Games", "Pit Fighter (rev 3)", MACHINE_SUPPORTS_SAVE )
GAME( 1990, pitfightj, pitfight, pitfight, pitfightj, atarig1_state,pitfightj, ROT0, "Atari Games", "Pit Fighter (Japan, 2 players)", MACHINE_SUPPORTS_SAVE )
GAME( 1990, pitfightb, pitfight, pitfight, pitfight, atarig1_state, pitfightb, ROT0, "bootleg",     "Pit Fighter (bootleg)", MACHINE_SUPPORTS_SAVE )
