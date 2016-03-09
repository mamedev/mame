// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    Atari G42 hardware

    driver by Aaron Giles

    Games supported:
        * Road Riot 4WD (1991)
        * Guardians of the 'Hood (1992)

    Known bugs:
        * ASIC65 for Road Riot not quite perfect

****************************************************************************

    Memory map (TBA)

***************************************************************************/

#include "emu.h"
#include "video/atarirle.h"
#include "includes/atarig42.h"


/*************************************
 *
 *  Initialization & interrupts
 *
 *************************************/

void atarig42_state::update_interrupts()
{
	m_maincpu->set_input_line(4, m_video_int_state ? ASSERT_LINE : CLEAR_LINE);
	m_maincpu->set_input_line(5, m_sound_int_state ? ASSERT_LINE : CLEAR_LINE);
}


MACHINE_START_MEMBER(atarig42_state,atarig42)
{
	atarigen_state::machine_start();

	save_item(NAME(m_analog_data));
	save_item(NAME(m_sloop_bank));
	save_item(NAME(m_sloop_next_bank));
	save_item(NAME(m_sloop_offset));
	save_item(NAME(m_sloop_state));
}


MACHINE_RESET_MEMBER(atarig42_state,atarig42)
{
	atarigen_state::machine_reset();
	scanline_timer_reset(*m_screen, 8);
}



/*************************************
 *
 *  I/O read dispatch.
 *
 *************************************/

READ16_MEMBER(atarig42_state::special_port2_r)
{
	int temp = ioport("IN2")->read();
	temp ^= 0x0008;     /* A2D.EOC always high for now */
	return temp;
}


WRITE16_MEMBER(atarig42_state::a2d_select_w)
{
	static const char *const portnames[] = { "A2D0", "A2D1" };

	m_analog_data = ioport(portnames[offset != 0])->read();
}


READ16_MEMBER(atarig42_state::a2d_data_r)
{
	return m_analog_data << 8;
}


WRITE16_MEMBER(atarig42_state::io_latch_w)
{
	/* upper byte */
	if (ACCESSING_BITS_8_15)
	{
		/* bit 14 controls the ASIC65 reset line */
		m_asic65->reset_line((~data >> 14) & 1);

		/* bits 13-11 are the MO control bits */
		m_rle->control_write(space, 0, (data >> 11) & 7);
	}

	/* lower byte */
	if (ACCESSING_BITS_0_7)
	{
		/* bit 4 resets the sound CPU */
		m_jsa->soundcpu().set_input_line(INPUT_LINE_RESET, (data & 0x10) ? CLEAR_LINE : ASSERT_LINE);
		if (!(data & 0x10))
			m_jsa->reset();

		/* bit 5 is /XRESET, probably related to the ASIC */

		/* bits 3 and 0 are coin counters */
	}
}


WRITE16_MEMBER(atarig42_state::mo_command_w)
{
	COMBINE_DATA(m_mo_command);
	m_rle->command_write(space, offset, (data == 0) ? ATARIRLE_COMMAND_CHECKSUM : ATARIRLE_COMMAND_DRAW);
}



/*************************************
 *
 *  SLOOP banking -- Road Riot
 *
 *************************************/

DIRECT_UPDATE_MEMBER( atarig42_state::atarig42_sloop_direct_handler )
{
	if (address < 0x80000)
	{
		direct.explicit_configure(0x00000, 0x7ffff, 0x7ffff, m_sloop_base);
		return (offs_t)-1;
	}
	return address;
}


void atarig42_state::roadriot_sloop_tweak(int offset)
{
/*
    sequence 1:

        touch $68000
        touch $68eee and $124/$678/$abc/$1024(bank) in the same instruction
        touch $69158/$6a690/$6e708/$71166

    sequence 2:

        touch $5edb4 to add 2 to the bank
        touch $5db0a to add 1 to the bank
        touch $5f042
        touch $69158/$6a690/$6e708/$71166
        touch $68000
        touch $5d532/$5d534
*/

	switch (offset)
	{
		/* standard 68000 -> 68eee -> (bank) addressing */
		case 0x68000/2:
			m_sloop_state = 1;
			break;
		case 0x68eee/2:
			if (m_sloop_state == 1)
				m_sloop_state = 2;
			break;
		case 0x00124/2:
			if (m_sloop_state == 2)
			{
				m_sloop_next_bank = 0;
				m_sloop_state = 3;
			}
			break;
		case 0x00678/2:
			if (m_sloop_state == 2)
			{
				m_sloop_next_bank = 1;
				m_sloop_state = 3;
			}
			break;
		case 0x00abc/2:
			if (m_sloop_state == 2)
			{
				m_sloop_next_bank = 2;
				m_sloop_state = 3;
			}
			break;
		case 0x01024/2:
			if (m_sloop_state == 2)
			{
				m_sloop_next_bank = 3;
				m_sloop_state = 3;
			}
			break;

		/* lock in the change? */
		case 0x69158/2:
			/* written if $ff8007 == 0 */
		case 0x6a690/2:
			/* written if $ff8007 == 1 */
		case 0x6e708/2:
			/* written if $ff8007 == 2 */
		case 0x71166/2:
			/* written if $ff8007 == 3 */
			if (m_sloop_state == 3)
				m_sloop_bank = m_sloop_next_bank;
			m_sloop_state = 0;
			break;

		/* bank offsets */
		case 0x5edb4/2:
			if (m_sloop_state == 0)
			{
				m_sloop_state = 10;
				m_sloop_offset = 0;
			}
			m_sloop_offset += 2;
			break;
		case 0x5db0a/2:
			if (m_sloop_state == 0)
			{
				m_sloop_state = 10;
				m_sloop_offset = 0;
			}
			m_sloop_offset += 1;
			break;

		/* apply the offset */
		case 0x5f042/2:
			if (m_sloop_state == 10)
			{
				m_sloop_bank = (m_sloop_bank + m_sloop_offset) & 3;
				m_sloop_offset = 0;
				m_sloop_state = 0;
			}
			break;

		/* unknown */
		case 0x5d532/2:
			break;
		case 0x5d534/2:
			break;

		default:
			break;
	}
}


READ16_MEMBER(atarig42_state::roadriot_sloop_data_r)
{
	roadriot_sloop_tweak(offset);
	if (offset < 0x78000/2)
		return m_sloop_base[offset];
	else
		return m_sloop_base[0x78000/2 + m_sloop_bank * 0x1000 + (offset & 0xfff)];
}


WRITE16_MEMBER(atarig42_state::roadriot_sloop_data_w)
{
	roadriot_sloop_tweak(offset);
}



/*************************************
 *
 *  SLOOP banking -- Guardians
 *
 *************************************/

void atarig42_state::guardians_sloop_tweak(int offset)
{
	UINT32 *last_accesses = m_last_accesses;

	if (offset >= 0x7f7c0/2)
	{
		last_accesses[0] = last_accesses[1];
		last_accesses[1] = last_accesses[2];
		last_accesses[2] = last_accesses[3];
		last_accesses[3] = last_accesses[4];
		last_accesses[4] = last_accesses[5];
		last_accesses[5] = last_accesses[6];
		last_accesses[6] = last_accesses[7];
		last_accesses[7] = offset;

		if (last_accesses[0] == 0x7f7c0/2 && last_accesses[1] == 0x7f7ce/2 && last_accesses[2] == 0x7f7c2/2 && last_accesses[3] == 0x7f7cc/2 &&
			last_accesses[4] == 0x7f7c4/2 && last_accesses[5] == 0x7f7ca/2 && last_accesses[6] == 0x7f7c6/2 && last_accesses[7] == 0x7f7c8/2)
			m_sloop_bank = 0;

		if (last_accesses[0] == 0x7f7d0/2 && last_accesses[1] == 0x7f7de/2 && last_accesses[2] == 0x7f7d2/2 && last_accesses[3] == 0x7f7dc/2 &&
			last_accesses[4] == 0x7f7d4/2 && last_accesses[5] == 0x7f7da/2 && last_accesses[6] == 0x7f7d6/2 && last_accesses[7] == 0x7f7d8/2)
			m_sloop_bank = 1;

		if (last_accesses[0] == 0x7f7e0/2 && last_accesses[1] == 0x7f7ee/2 && last_accesses[2] == 0x7f7e2/2 && last_accesses[3] == 0x7f7ec/2 &&
			last_accesses[4] == 0x7f7e4/2 && last_accesses[5] == 0x7f7ea/2 && last_accesses[6] == 0x7f7e6/2 && last_accesses[7] == 0x7f7e8/2)
			m_sloop_bank = 2;

		if (last_accesses[0] == 0x7f7f0/2 && last_accesses[1] == 0x7f7fe/2 && last_accesses[2] == 0x7f7f2/2 && last_accesses[3] == 0x7f7fc/2 &&
			last_accesses[4] == 0x7f7f4/2 && last_accesses[5] == 0x7f7fa/2 && last_accesses[6] == 0x7f7f6/2 && last_accesses[7] == 0x7f7f8/2)
			m_sloop_bank = 3;
	}
}


READ16_MEMBER(atarig42_state::guardians_sloop_data_r)
{
	guardians_sloop_tweak(offset);
	if (offset < 0x78000/2)
		return m_sloop_base[offset];
	else
		return m_sloop_base[0x78000/2 + m_sloop_bank * 0x1000 + (offset & 0xfff)];
}


WRITE16_MEMBER(atarig42_state::guardians_sloop_data_w)
{
	guardians_sloop_tweak(offset);
}



/*************************************
 *
 *  Main CPU memory handlers
 *
 *************************************/

static ADDRESS_MAP_START( main_map, AS_PROGRAM, 16, atarig42_state )
	AM_RANGE(0x000000, 0x080001) AM_ROM
	AM_RANGE(0xe00000, 0xe00001) AM_READ_PORT("IN0")
	AM_RANGE(0xe00002, 0xe00003) AM_READ_PORT("IN1")
	AM_RANGE(0xe00010, 0xe00011) AM_READ(special_port2_r)
	AM_RANGE(0xe00012, 0xe00013) AM_READ_PORT("jsa:JSAIII")
	AM_RANGE(0xe00020, 0xe00027) AM_READWRITE(a2d_data_r, a2d_select_w)
	AM_RANGE(0xe00030, 0xe00031) AM_DEVREAD8("jsa", atari_jsa_iii_device, main_response_r, 0x00ff)
	AM_RANGE(0xe00040, 0xe00041) AM_DEVWRITE8("jsa", atari_jsa_iii_device, main_command_w, 0x00ff)
	AM_RANGE(0xe00050, 0xe00051) AM_WRITE(io_latch_w)
	AM_RANGE(0xe00060, 0xe00061) AM_DEVWRITE("eeprom", atari_eeprom_device, unlock_write)
	AM_RANGE(0xe03000, 0xe03001) AM_WRITE(video_int_ack_w)
	AM_RANGE(0xe03800, 0xe03801) AM_WRITE(watchdog_reset16_w)
	AM_RANGE(0xe80000, 0xe80fff) AM_RAM
	AM_RANGE(0xf40000, 0xf40001) AM_DEVREAD("asic65", asic65_device, io_r)
	AM_RANGE(0xf60000, 0xf60001) AM_DEVREAD("asic65", asic65_device, read)
	AM_RANGE(0xf80000, 0xf80003) AM_DEVWRITE("asic65", asic65_device, data_w)
	AM_RANGE(0xfa0000, 0xfa0fff) AM_DEVREADWRITE8("eeprom", atari_eeprom_device, read, write, 0x00ff)
	AM_RANGE(0xfc0000, 0xfc0fff) AM_RAM_DEVWRITE("palette", palette_device, write) AM_SHARE("palette")
	AM_RANGE(0xff0000, 0xff0fff) AM_RAM AM_SHARE("rle")
	AM_RANGE(0xff2000, 0xff5fff) AM_DEVWRITE("playfield", tilemap_device, write) AM_SHARE("playfield")
	AM_RANGE(0xff6000, 0xff6fff) AM_DEVWRITE("alpha", tilemap_device, write) AM_SHARE("alpha")
	AM_RANGE(0xff7000, 0xff7001) AM_WRITE(mo_command_w) AM_SHARE("mo_command")
	AM_RANGE(0xff0000, 0xffffff) AM_RAM
ADDRESS_MAP_END



/*************************************
 *
 *  Port definitions
 *
 *************************************/

static INPUT_PORTS_START( roadriot )
	PORT_START("IN0")       /* e00000 */
	PORT_BIT( 0x00ff, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0xf800, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN1")      /* e00002 */
	PORT_BIT( 0xffff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN2")       /* e00010 */
	PORT_BIT( 0x000f, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_ATARI_JSA_SOUND_TO_MAIN_READY("jsa")
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_ATARI_JSA_MAIN_TO_SOUND_READY("jsa")
	PORT_SERVICE( 0x0040, IP_ACTIVE_LOW )
	PORT_BIT(  0x0080, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_VBLANK("screen")
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("A2D0")      /* analog 0 */
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_X ) PORT_SENSITIVITY(50) PORT_KEYDELTA(10)

	PORT_START("A2D1")      /* analog 1 */
	PORT_BIT( 0xff, 0x00, IPT_PEDAL ) PORT_SENSITIVITY(100) PORT_KEYDELTA(16)
INPUT_PORTS_END


static INPUT_PORTS_START( guardian )
	PORT_START("IN0")       /* e00000 */
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_PLAYER(3)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(3)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(3)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(3)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(3)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(3)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(3)
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_PLAYER(1)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(1)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1)
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(1)

	PORT_START("IN1")      /* e00002 */
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(3)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(3)
	PORT_BIT( 0x01c0, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(2)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2)
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(2)

	PORT_START("IN2")       /* e00010 */
	PORT_BIT( 0x000f, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_ATARI_JSA_SOUND_TO_MAIN_READY("jsa")
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_ATARI_JSA_MAIN_TO_SOUND_READY("jsa")
	PORT_SERVICE( 0x0040, IP_ACTIVE_LOW )
	PORT_BIT(  0x0080, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_VBLANK("screen")
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("A2D0")      /* analog 0 */
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("A2D1")      /* analog 1 */
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END



/*************************************
 *
 *  Graphics definitions
 *
 *************************************/

static const gfx_layout pflayout =
{
	8,8,
	RGN_FRAC(1,3),
	5,
	{ 0, 0, 1, 2, 3 },
	{ RGN_FRAC(1,3)+0, RGN_FRAC(1,3)+4, 0, 4, RGN_FRAC(1,3)+8, RGN_FRAC(1,3)+12, 8, 12 },
	{ 0*8, 2*8, 4*8, 6*8, 8*8, 10*8, 12*8, 14*8 },
	16*8
};

static const gfx_layout pftoplayout =
{
	8,8,
	RGN_FRAC(1,3),
	6,
	{ RGN_FRAC(2,3)+0, RGN_FRAC(2,3)+4, 0, 0, 0, 0 },
	{ 3, 2, 1, 0, 11, 10, 9, 8 },
	{ 0*8, 2*8, 4*8, 6*8, 8*8, 10*8, 12*8, 14*8 },
	16*8
};

static const gfx_layout anlayout =
{
	8,8,
	RGN_FRAC(1,1),
	4,
	{ 0, 1, 2, 3 },
	{ 0, 4, 8, 12, 16, 20, 24, 28 },
	{ 0*8, 4*8, 8*8, 12*8, 16*8, 20*8, 24*8, 28*8 },
	32*8
};

static GFXDECODE_START( atarig42 )
	GFXDECODE_ENTRY( "gfx1", 0, pflayout, 0x000, 64 )
	GFXDECODE_ENTRY( "gfx2", 0, anlayout, 0x000, 16 )
	GFXDECODE_ENTRY( "gfx1", 0, pftoplayout, 0x000, 64 )
GFXDECODE_END


static const atari_rle_objects_config modesc_0x200 =
{
	0,          /* left clip coordinate */
	0,          /* right clip coordinate */
	0x200,      /* base palette entry */

	{{ 0x7fff,0,0,0,0,0,0,0 }}, /* mask for the code index */
	{{ 0,0x01f0,0,0,0,0,0,0 }}, /* mask for the color */
	{{ 0,0,0xffc0,0,0,0,0,0 }}, /* mask for the X position */
	{{ 0,0,0,0xffc0,0,0,0,0 }}, /* mask for the Y position */
	{{ 0,0,0,0,0xffff,0,0,0 }}, /* mask for the scale factor */
	{{ 0x8000,0,0,0,0,0,0,0 }}, /* mask for the horizontal flip */
	{{ 0,0,0,0,0,0,0x00ff,0 }}, /* mask for the order */
	{{ 0,0x0e00,0,0,0,0,0,0 }}, /* mask for the priority */
	{{ 0 }}                     /* mask for the VRAM target */
};


static const atari_rle_objects_config modesc_0x400 =
{
	0,          /* left clip coordinate */
	0,          /* right clip coordinate */
	0x400,      /* base palette entry */

	{{ 0x7fff,0,0,0,0,0,0,0 }}, /* mask for the code index */
	{{ 0,0x03f0,0,0,0,0,0,0 }}, /* mask for the color */
	{{ 0,0,0xffc0,0,0,0,0,0 }}, /* mask for the X position */
	{{ 0,0,0,0xffc0,0,0,0,0 }}, /* mask for the Y position */
	{{ 0,0,0,0,0xffff,0,0,0 }}, /* mask for the scale factor */
	{{ 0x8000,0,0,0,0,0,0,0 }}, /* mask for the horizontal flip */
	{{ 0,0,0,0,0,0,0x00ff,0 }}, /* mask for the order */
	{{ 0,0x0e00,0,0,0,0,0,0 }}, /* mask for the priority */
	{{ 0 }}                     /* mask for the VRAM target */
};



/*************************************
 *
 *  Machine driver
 *
 *************************************/

static MACHINE_CONFIG_START( atarig42, atarig42_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68000, ATARI_CLOCK_14MHz)
	MCFG_CPU_PROGRAM_MAP(main_map)
	MCFG_DEVICE_VBLANK_INT_DRIVER("screen", atarigen_state, video_int_gen)

	MCFG_MACHINE_START_OVERRIDE(atarig42_state,atarig42)
	MCFG_MACHINE_RESET_OVERRIDE(atarig42_state,atarig42)

	MCFG_ATARI_EEPROM_2816_ADD("eeprom")

	/* video hardware */
	MCFG_GFXDECODE_ADD("gfxdecode", "palette", atarig42)
	MCFG_PALETTE_ADD("palette", 2048)
	MCFG_PALETTE_FORMAT(IRRRRRGGGGGBBBBB)

	MCFG_TILEMAP_ADD_CUSTOM("playfield", "gfxdecode", 2, atarig42_state, get_playfield_tile_info, 8,8, atarig42_playfield_scan, 128,64)
	MCFG_TILEMAP_ADD_STANDARD_TRANSPEN("alpha", "gfxdecode", 2, atarig42_state, get_alpha_tile_info, 8,8, SCAN_ROWS, 64,32, 0)

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_VIDEO_ATTRIBUTES(VIDEO_UPDATE_BEFORE_VBLANK)
	/* note: these parameters are from published specs, not derived */
	/* the board uses an SOS chip to generate video signals */
	MCFG_SCREEN_RAW_PARAMS(ATARI_CLOCK_14MHz/2, 456, 0, 336, 262, 0, 240)
	MCFG_SCREEN_UPDATE_DRIVER(atarig42_state, screen_update_atarig42)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_VIDEO_START_OVERRIDE(atarig42_state,atarig42)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_ATARI_JSA_III_ADD("jsa", WRITELINE(atarigen_state, sound_int_write_line))
	MCFG_ATARI_JSA_TEST_PORT("IN2", 6)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( atarig42_0x200, atarig42 )
	MCFG_ATARIRLE_ADD("rle", modesc_0x200)

	/* ASIC65 */
	MCFG_ASIC65_ADD("asic65", ASIC65_ROMBASED)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( atarig42_0x400, atarig42 )
	MCFG_ATARIRLE_ADD("rle", modesc_0x400)

	/* ASIC65 */
	MCFG_ASIC65_ADD("asic65", ASIC65_GUARDIANS)
MACHINE_CONFIG_END




/*************************************
 *
 *  ROM definition(s)
 *
 *************************************/

ROM_START( roadriot )
	ROM_REGION( 0x80004, "maincpu", 0 ) /* 68000 code */
	ROM_LOAD16_BYTE( "136089-3114.8d", 0x00000, 0x20000, CRC(a2bd949c) SHA1(f96064d491b4d488cadebd3a63a6d3edf9236046) )
	ROM_LOAD16_BYTE( "136089-3113.8c", 0x00001, 0x20000, CRC(68c45cb1) SHA1(e38c7ad3f3d301e59a1d9f53e8f2c28e91d691fe) )
	ROM_LOAD16_BYTE( "136089-2016.9d", 0x40000, 0x20000, CRC(6191653c) SHA1(97d1a84a585149e8f2c49cab7af22dc755dff350) )
	ROM_LOAD16_BYTE( "136089-2015.9c", 0x40001, 0x20000, CRC(0d34419a) SHA1(f16e9fb4cd537d727611cb7dd5537c030671fe1e) )

	ROM_REGION( 0x2000, "asic65:asic65cpu", 0 )   /* ASIC65 TMS32015 code */
	ROM_LOAD( "136089-1012.3f", 0x00000, 0x0a80, CRC(7c5498e7) SHA1(9d8b235baf7b75bef8ef9b168647c5b2b80b2cb3) )

	ROM_REGION( 0x10000, "jsa:cpu", 0 ) /* 6502 code */
	ROM_LOAD( "136089-1047.12c", 0x00000, 0x10000, CRC(849dd26c) SHA1(05a0b2a5f7ee4437448b5f076d3066d96dec2320) )

	ROM_REGION( 0xc0000, "gfx1", 0 )
	ROM_LOAD( "136089-1041.22d",    0x000000, 0x20000, CRC(b7451f92) SHA1(9fd17913630e457e406e596f2d86afff98787750) ) /* playfield, planes 0-1 */
	ROM_LOAD( "136089-1038.22c",    0x020000, 0x20000, CRC(90f3c6ee) SHA1(7607509e2d3b2080a918cfaf2879dbed6b79d029) )
	ROM_LOAD( "136089-1037.2021d",  0x040000, 0x20000, CRC(d40de62b) SHA1(fa6dfd20bdad7874ae33a1027a9bb0ea200f86ca) ) /* playfield, planes 2-3 */
	ROM_LOAD( "136089-1039.2021c",  0x060000, 0x20000, CRC(a7e836b1) SHA1(d41f1e4166ca757176c6976be2a953db5db05e48) )
	ROM_LOAD( "136089-1040.20d",    0x080000, 0x20000, CRC(a81ae93f) SHA1(b694ba5fab35f8fa505a02039ae62f7af3c7ae1d) ) /* playfield, planes 4-5 */
	ROM_LOAD( "136089-1042.20c",    0x0a0000, 0x20000, CRC(b8a6d15a) SHA1(43d2be9d40a84b2c01d80bbcac737eda04d55999) )

	ROM_REGION( 0x020000, "gfx2", 0 )
	ROM_LOAD( "136089-1046.22j",    0x000000, 0x20000, CRC(0005bab0) SHA1(257e1b23eea117fe6701a67134b96d9d9fe10caf) ) /* alphanumerics */

	ROM_REGION16_BE( 0x200000, "rle", 0 )
	ROM_LOAD16_BYTE( "136089-1018.2s", 0x000000, 0x20000, CRC(19590a94) SHA1(e375b7e01a8b1f366bb4e7750e33f0b6d9ae2042) )
	ROM_LOAD16_BYTE( "136089-1017.2p", 0x000001, 0x20000, CRC(c2bf3f69) SHA1(f822359070b1907973ee7ee35469f4a59f720830) )
	ROM_LOAD16_BYTE( "136089-1020.3s", 0x040000, 0x20000, CRC(bab110e4) SHA1(0c4e3521474249517e7832df1bc63aca6d6a6c91) )
	ROM_LOAD16_BYTE( "136089-1019.3p", 0x040001, 0x20000, CRC(791ad2c5) SHA1(4ef218fbf38a9c6b58c86f203843988df1c935f6) )
	ROM_LOAD16_BYTE( "136089-1022.4s", 0x080000, 0x20000, CRC(79cba484) SHA1(ce361a432f1fe627086bab3c1131118fd15056f1) )
	ROM_LOAD16_BYTE( "136089-1021.4p", 0x080001, 0x20000, CRC(86a2e257) SHA1(98d95d2e67fecc332f6c66358a1f8d58b168c74b) )
	ROM_LOAD16_BYTE( "136089-1024.5s", 0x0c0000, 0x20000, CRC(67d28478) SHA1(cfc9da6d20c65d11c2a59a38660a8da4d1cc219d) )
	ROM_LOAD16_BYTE( "136089-1023.5p", 0x0c0001, 0x20000, CRC(74638838) SHA1(bea0fb21ccb946e023c88791ce5a8dd92b44baec) )
	ROM_LOAD16_BYTE( "136089-1026.6s", 0x100000, 0x20000, CRC(24933c37) SHA1(516393aae51fc9634a5c6d5134be058d6067e114) )
	ROM_LOAD16_BYTE( "136089-1025.6p", 0x100001, 0x20000, CRC(054a163b) SHA1(1b0b129c093398bc5c14b3fdd87dfe149f555fac) )
	ROM_LOAD16_BYTE( "136089-1028.7s", 0x140000, 0x20000, CRC(31ff090a) SHA1(7b43ed37901c3f94cae90c84b3c8c689d7b64dd6) )
	ROM_LOAD16_BYTE( "136089-1027.7p", 0x140001, 0x20000, CRC(bbe5b69b) SHA1(9eaa551fba763824d36fc41bfe0e6d735a9e68c5) )
	ROM_LOAD16_BYTE( "136089-1030.8s", 0x180000, 0x20000, CRC(6c89d2c5) SHA1(0bf2990ce1cd5ec5b84f7e3171725540e6238408) )
	ROM_LOAD16_BYTE( "136089-1029.8p", 0x180001, 0x20000, CRC(40d9bde5) SHA1(aca6e07ea96e4618412d32fe4d4cd293ae82d940) )
	ROM_LOAD16_BYTE( "136089-1032.9s", 0x1c0000, 0x20000, CRC(eca3c595) SHA1(5d067b7c02675b1e6dd3c4046697a16f873f80af) )
	ROM_LOAD16_BYTE( "136089-1031.9p", 0x1c0001, 0x20000, CRC(88acdb53) SHA1(5bf2424ee75a25248a8ce38c8605b6780da4e323) )

	ROM_REGION( 0x80000, "jsa:oki1", 0 )
	ROM_LOAD( "136089-1048.19e",  0x00000, 0x20000, CRC(2db638a7) SHA1(45da8088f7439beacc3056952a4d631d9633efa7) )
	ROM_LOAD( "136089-1049.17e",  0x20000, 0x20000, CRC(e1dd7f9e) SHA1(6b9a240aa84d210d3052daab6ea26f9cd0e62dc1) )
	ROM_LOAD( "136089-1050.15e",  0x40000, 0x20000, CRC(64d410bb) SHA1(877bccca7ff37a9dd8294bc1453487a2f516ca7d) )
	ROM_LOAD( "136089-1051.12e",  0x60000, 0x20000, CRC(bffd01c8) SHA1(f6de000f61ea0c1ddb31ee5301506e5e966638c2) )

	ROM_REGION( 0x800, "eeprom:eeprom", 0 )
	ROM_LOAD( "roadriot-eeprom.5c", 0x0000, 0x800, CRC(8d9b957d) SHA1(9d895c5977a3f405130594a10d530a82a6aa265f) )

	ROM_REGION( 0x0600, "proms", 0 )    /* microcode for growth renderer */
	ROM_LOAD( "136089-1001.20p",  0x0000, 0x0200, CRC(5836cb5a) SHA1(2c797f6a1227d6e1fd7a12f99f0254072c8c266e) )
	ROM_LOAD( "136089-1002.22p",  0x0200, 0x0200, CRC(44288753) SHA1(811582015264f85a32643196cdb331a41430318f) )
	ROM_LOAD( "136089-1003.21p",  0x0400, 0x0200, CRC(1f571706) SHA1(26d5ea59163b3482ab1f8a26178d0849c5fd9692) )
ROM_END

ROM_START( roadrioto )
	ROM_REGION( 0x80004, "maincpu", 0 ) /* 68000 code */
	ROM_LOAD16_BYTE( "136089-2014.8d", 0x00000, 0x20000, CRC(bf8aaafc) SHA1(1594d91b56609d49921c866d8f5796619e79217b) ) /* Program ROMs in Blue labels,  */
	ROM_LOAD16_BYTE( "136089-2013.8c", 0x00001, 0x20000, CRC(5dd2dd70) SHA1(8f6a0e809ec1f6feea8a18197a789086a7b9dd6a) ) /* other ROMs in Yellow labels   */
	ROM_LOAD16_BYTE( "136089-2016.9d", 0x40000, 0x20000, CRC(6191653c) SHA1(97d1a84a585149e8f2c49cab7af22dc755dff350) ) /* PALs & BPROMs in White labels */
	ROM_LOAD16_BYTE( "136089-2015.9c", 0x40001, 0x20000, CRC(0d34419a) SHA1(f16e9fb4cd537d727611cb7dd5537c030671fe1e) )

	ROM_REGION( 0x2000, "asic65:asic65cpu", 0 )   /* ASIC65 TMS32015 code */
	ROM_LOAD( "136089-1012.3f", 0x00000, 0x0a80, CRC(7c5498e7) SHA1(9d8b235baf7b75bef8ef9b168647c5b2b80b2cb3) )

	ROM_REGION( 0x10000, "jsa:cpu", 0 ) /* 6502 code */
	ROM_LOAD( "136089-1047.12c", 0x00000, 0x10000, CRC(849dd26c) SHA1(05a0b2a5f7ee4437448b5f076d3066d96dec2320) )

	ROM_REGION( 0xc0000, "gfx1", 0 )
	ROM_LOAD( "136089-1041.22d",    0x000000, 0x20000, CRC(b7451f92) SHA1(9fd17913630e457e406e596f2d86afff98787750) ) /* playfield, planes 0-1 */
	ROM_LOAD( "136089-1038.22c",    0x020000, 0x20000, CRC(90f3c6ee) SHA1(7607509e2d3b2080a918cfaf2879dbed6b79d029) )
	ROM_LOAD( "136089-1037.2021d",  0x040000, 0x20000, CRC(d40de62b) SHA1(fa6dfd20bdad7874ae33a1027a9bb0ea200f86ca) ) /* playfield, planes 2-3 */
	ROM_LOAD( "136089-1039.2021c",  0x060000, 0x20000, CRC(a7e836b1) SHA1(d41f1e4166ca757176c6976be2a953db5db05e48) )
	ROM_LOAD( "136089-1040.20d",    0x080000, 0x20000, CRC(a81ae93f) SHA1(b694ba5fab35f8fa505a02039ae62f7af3c7ae1d) ) /* playfield, planes 4-5 */
	ROM_LOAD( "136089-1042.20c",    0x0a0000, 0x20000, CRC(b8a6d15a) SHA1(43d2be9d40a84b2c01d80bbcac737eda04d55999) )

	ROM_REGION( 0x020000, "gfx2", 0 )
	ROM_LOAD( "136089-1046.22j",    0x000000, 0x20000, CRC(0005bab0) SHA1(257e1b23eea117fe6701a67134b96d9d9fe10caf) ) /* alphanumerics */

	ROM_REGION16_BE( 0x200000, "rle", 0 )
	ROM_LOAD16_BYTE( "136089-1018.2s", 0x000000, 0x20000, CRC(19590a94) SHA1(e375b7e01a8b1f366bb4e7750e33f0b6d9ae2042) )
	ROM_LOAD16_BYTE( "136089-1017.2p", 0x000001, 0x20000, CRC(c2bf3f69) SHA1(f822359070b1907973ee7ee35469f4a59f720830) )
	ROM_LOAD16_BYTE( "136089-1020.3s", 0x040000, 0x20000, CRC(bab110e4) SHA1(0c4e3521474249517e7832df1bc63aca6d6a6c91) )
	ROM_LOAD16_BYTE( "136089-1019.3p", 0x040001, 0x20000, CRC(791ad2c5) SHA1(4ef218fbf38a9c6b58c86f203843988df1c935f6) )
	ROM_LOAD16_BYTE( "136089-1022.4s", 0x080000, 0x20000, CRC(79cba484) SHA1(ce361a432f1fe627086bab3c1131118fd15056f1) )
	ROM_LOAD16_BYTE( "136089-1021.4p", 0x080001, 0x20000, CRC(86a2e257) SHA1(98d95d2e67fecc332f6c66358a1f8d58b168c74b) )
	ROM_LOAD16_BYTE( "136089-1024.5s", 0x0c0000, 0x20000, CRC(67d28478) SHA1(cfc9da6d20c65d11c2a59a38660a8da4d1cc219d) )
	ROM_LOAD16_BYTE( "136089-1023.5p", 0x0c0001, 0x20000, CRC(74638838) SHA1(bea0fb21ccb946e023c88791ce5a8dd92b44baec) )
	ROM_LOAD16_BYTE( "136089-1026.6s", 0x100000, 0x20000, CRC(24933c37) SHA1(516393aae51fc9634a5c6d5134be058d6067e114) )
	ROM_LOAD16_BYTE( "136089-1025.6p", 0x100001, 0x20000, CRC(054a163b) SHA1(1b0b129c093398bc5c14b3fdd87dfe149f555fac) )
	ROM_LOAD16_BYTE( "136089-1028.7s", 0x140000, 0x20000, CRC(31ff090a) SHA1(7b43ed37901c3f94cae90c84b3c8c689d7b64dd6) )
	ROM_LOAD16_BYTE( "136089-1027.7p", 0x140001, 0x20000, CRC(bbe5b69b) SHA1(9eaa551fba763824d36fc41bfe0e6d735a9e68c5) )
	ROM_LOAD16_BYTE( "136089-1030.8s", 0x180000, 0x20000, CRC(6c89d2c5) SHA1(0bf2990ce1cd5ec5b84f7e3171725540e6238408) )
	ROM_LOAD16_BYTE( "136089-1029.8p", 0x180001, 0x20000, CRC(40d9bde5) SHA1(aca6e07ea96e4618412d32fe4d4cd293ae82d940) )
	ROM_LOAD16_BYTE( "136089-1032.9s", 0x1c0000, 0x20000, CRC(eca3c595) SHA1(5d067b7c02675b1e6dd3c4046697a16f873f80af) )
	ROM_LOAD16_BYTE( "136089-1031.9p", 0x1c0001, 0x20000, CRC(88acdb53) SHA1(5bf2424ee75a25248a8ce38c8605b6780da4e323) )

	ROM_REGION( 0x80000, "jsa:oki1", 0 )
	ROM_LOAD( "136089-1048.19e",  0x00000, 0x20000, CRC(2db638a7) SHA1(45da8088f7439beacc3056952a4d631d9633efa7) )
	ROM_LOAD( "136089-1049.17e",  0x20000, 0x20000, CRC(e1dd7f9e) SHA1(6b9a240aa84d210d3052daab6ea26f9cd0e62dc1) )
	ROM_LOAD( "136089-1050.15e",  0x40000, 0x20000, CRC(64d410bb) SHA1(877bccca7ff37a9dd8294bc1453487a2f516ca7d) )
	ROM_LOAD( "136089-1051.12e",  0x60000, 0x20000, CRC(bffd01c8) SHA1(f6de000f61ea0c1ddb31ee5301506e5e966638c2) )

	ROM_REGION( 0x800, "eeprom:eeprom", 0 )
	ROM_LOAD( "roadriot-eeprom.5c", 0x0000, 0x800, CRC(8d9b957d) SHA1(9d895c5977a3f405130594a10d530a82a6aa265f) )

	ROM_REGION( 0x0600, "proms", 0 )    /* microcode for growth renderer */
	ROM_LOAD( "136089-1001.20p",  0x0000, 0x0200, CRC(5836cb5a) SHA1(2c797f6a1227d6e1fd7a12f99f0254072c8c266e) )
	ROM_LOAD( "136089-1002.22p",  0x0200, 0x0200, CRC(44288753) SHA1(811582015264f85a32643196cdb331a41430318f) )
	ROM_LOAD( "136089-1003.21p",  0x0400, 0x0200, CRC(1f571706) SHA1(26d5ea59163b3482ab1f8a26178d0849c5fd9692) )
ROM_END


ROM_START( guardian )
	ROM_REGION( 0x80004, "maincpu", 0 ) /* 68000 code */
	ROM_LOAD16_BYTE( "136092-2021.8e",  0x00000, 0x20000, CRC(efea1e02) SHA1(f0f1ef300f36953aff73b68ffe6d9950ac575f7d) )
	ROM_LOAD16_BYTE( "136092-2020.8cd", 0x00001, 0x20000, CRC(a8f655ba) SHA1(2defe4d138613e248718a617d103794e051746f7) )
	ROM_LOAD16_BYTE( "136092-2023.9e",  0x40000, 0x20000, CRC(cfa29316) SHA1(4e0e76304e29ee59bc2ce9a704e3f651dc9d473c) )
	ROM_LOAD16_BYTE( "136092-2022.9cd", 0x40001, 0x20000, CRC(ed2abc91) SHA1(81531040d5663f6ab82e924210056e3737e17a8d) )

	ROM_REGION( 0x2000, "asic65:asic65cpu", 0 )   /* ASIC65 TMS32015 code */
	ROM_LOAD( "136089-1012.3f", 0x00000, 0x0a80, NO_DUMP )

	ROM_REGION( 0x10000, "jsa:cpu", 0 ) /* 6502 code */
	ROM_LOAD( "136092-0080-snd.12c", 0x00000, 0x10000, CRC(0388f805) SHA1(49c11313bc4192dbe294cf68b652cb19047889fd) )

	ROM_REGION( 0x180000, "gfx1", 0 )
	ROM_LOAD( "136092-0037a.23e",  0x000000, 0x80000, CRC(ca10b63e) SHA1(243a2a440e1bc9135d3dbe6553d39c54b9bdcd13) ) /* playfield, planes 0-1 */
	ROM_LOAD( "136092-0038a.22e",  0x080000, 0x80000, CRC(cb1431a1) SHA1(d7b8f49a1e794ca2083e4bf0fa3870ce08caa53a) ) /* playfield, planes 2-3 */
	ROM_LOAD( "136092-0039a.20e",  0x100000, 0x80000, CRC(2eee7188) SHA1(d3adbd7b20bc898fee35b6ba781e7775f82acd19) ) /* playfield, planes 4-5 */

	ROM_REGION( 0x020000, "gfx2", 0 )
	ROM_LOAD( "136092-0030.23k",   0x000000, 0x20000, CRC(0fd7baa1) SHA1(7802d732e5173291628ed498ad0fab71aeef4688) ) /* alphanumerics */

	ROM_REGION16_BE( 0x600000, "rle", 0 )
	ROM_LOAD16_BYTE( "136092-0041.2s",  0x000000, 0x80000, CRC(a2a5ae08) SHA1(d99f925bbc9a72432e13328ee8422fde615db90f) )
	ROM_LOAD16_BYTE( "136092-0040.2p",  0x000001, 0x80000, CRC(ef95132e) SHA1(288de1d15956a612b7d19ceb2cf853490bf42b05) )
	ROM_LOAD16_BYTE( "136092-0043.3s",  0x100000, 0x80000, CRC(6438b8e4) SHA1(ee1446209fbcab8b17c88c53b65e754a85f279d1) )
	ROM_LOAD16_BYTE( "136092-0042.3p",  0x100001, 0x80000, CRC(46bf7c0d) SHA1(12414de2698178b158ec4ca0fbb176943c944cec) )
	ROM_LOAD16_BYTE( "136092-0045.4s",  0x200000, 0x80000, CRC(4f4f2bee) SHA1(8276cdcd252d2d8fa41ab28e76a6bd72613c14ec) )
	ROM_LOAD16_BYTE( "136092-0044.4p",  0x200001, 0x80000, CRC(20a4250b) SHA1(6a2e2596a9eef2792f7cdab648dd455b8e420a74) )
	ROM_LOAD16_BYTE( "136092-0063a.6s", 0x300000, 0x80000, CRC(93933bcf) SHA1(a67d4839ffdb0eafbc2d68a60fb3bf1507c793cf) )
	ROM_LOAD16_BYTE( "136092-0062a.6p", 0x300001, 0x80000, CRC(613e6f1d) SHA1(fd2ea18d245d0895e0bac6c5caa6d35fdd6a199f) )
	ROM_LOAD16_BYTE( "136092-0065a.7s", 0x400000, 0x80000, CRC(6bcd1205) SHA1(c883c55f88d274ba8aa48c962406b253e1f8001e) )
	ROM_LOAD16_BYTE( "136092-0064a.7p", 0x400001, 0x80000, CRC(7b4dce05) SHA1(36545917388e704f73a9b4d85189ec978d655b63) )
	ROM_LOAD16_BYTE( "136092-0067a.9s", 0x500000, 0x80000, CRC(15845fba) SHA1(f7b670a8d48a5e9c261150914a06ef9a938a84e7) )
	ROM_LOAD16_BYTE( "136092-0066a.9p", 0x500001, 0x80000, CRC(7130c575) SHA1(b3ea109981a1e5c631705b23dfad4a3a3daf7734) )

	ROM_REGION( 0x80000, "jsa:oki1", 0 )
	ROM_LOAD( "136092-0010-snd.19e",  0x00000, 0x80000, CRC(bca27f40) SHA1(91a41eac116eb7d9a790abc590eb06328726d1c2) )

	ROM_REGION( 0x800, "eeprom:eeprom", 0 )
	ROM_LOAD( "guardian-eeprom.5c", 0x0000, 0x800, CRC(85835fab) SHA1(747e2851c8baa0e7f1c0784b0d6900514230ab07) )

	ROM_REGION( 0x0600, "proms", 0 )    /* microcode for growth renderer */
	ROM_LOAD( "136092-1001.20p",  0x0000, 0x0200, CRC(b3251eeb) SHA1(5e83baa70aaa28f07f32657bf974fd87719972d3) )
	ROM_LOAD( "136092-1002.22p",  0x0200, 0x0200, CRC(0c5314da) SHA1(a9c7ee3ab015c7f3ada4200acd2854eb9a5c74b0) )
	ROM_LOAD( "136092-1003.21p",  0x0400, 0x0200, CRC(344b406a) SHA1(f4422f8c0d7004d0277a4fc77718d555f80fcf69) )

	ROM_REGION( 0x1500, "plds", 0 )
	ROM_LOAD( "gal16v8a.3l",  0x0000, 0x0117, NO_DUMP ) /* PAL is read protected */
	ROM_LOAD( "gal16v8a.7c",  0x0200, 0x0117, NO_DUMP ) /* PAL is read protected */
	ROM_LOAD( "gal16v8a.13s", 0x0400, 0x0117, NO_DUMP ) /* PAL is read protected */
	ROM_LOAD( "gal16v8a.16j", 0x0600, 0x0117, NO_DUMP ) /* PAL is read protected */
	ROM_LOAD( "gal16v8a.17c", 0x0800, 0x0117, NO_DUMP ) /* PAL is read protected */
	ROM_LOAD( "gal16v8a.18k", 0x0a00, 0x0117, NO_DUMP ) /* PAL is read protected */
	ROM_LOAD( "gal16v8a.20c", 0x0c00, 0x0117, NO_DUMP ) /* PAL is read protected */
	ROM_LOAD( "gal16v8a.22l", 0x0e00, 0x0117, NO_DUMP ) /* PAL is read protected */
	ROM_LOAD( "gal6001b.15h", 0x1000, 0x0410, NO_DUMP ) /* PAL is read protected */
ROM_END



/*************************************
 *
 *  Driver initialization
 *
 *************************************/

DRIVER_INIT_MEMBER(atarig42_state,roadriot)
{
	m_playfield_base = 0x400;

	address_space &main = m_maincpu->space(AS_PROGRAM);
	m_sloop_base = main.install_readwrite_handler(0x000000, 0x07ffff, read16_delegate(FUNC(atarig42_state::roadriot_sloop_data_r),this), write16_delegate(FUNC(atarig42_state::roadriot_sloop_data_w),this));
	main.set_direct_update_handler(direct_update_delegate(FUNC(atarig42_state::atarig42_sloop_direct_handler), this));

	/*
	Road Riot color MUX

	CRA10=!MGEP*!AN.VID7*AN.0               -- if (mopri < pfpri) && (!alpha)
	   +!AN.VID7*AN.0*MO.0                  or if (mopix == 0) && (!alpha)

	CRA9=MGEP*!AN.VID7*AN.0*!MO.0           -- if (mopri >= pfpri) && (mopix != 0) && (!alpha)
	   +!AN.VID7*AN.0*PF.VID9               or if (pfpix & 0x200) && (!alpha)

	CRA8=MGEP*!AN.VID7*AN.0*!MO.0*MVID8     -- if (mopri >= pfpri) && (mopix != 0) && (mopix & 0x100) && (!alpha)
	   +!MGEP*!AN.VID7*AN.0*PF.VID8         or if (mopri < pfpri) && (pfpix & 0x100) && (!alpha)
	   +!AN.VID7*AN.0*MO.0*PF.VID8          or if (pfpix & 0x100) && (!alpha)

	CRMUXB=!AN.VID7*AN.0                    -- if (!alpha)

	CRMUXA=!MGEP                            -- if (mopri < pfpri)
	   +MO.0                                or (mopix == 0)
	   +AN.VID7                             or (alpha)
	   +!AN.0
*/
}


DRIVER_INIT_MEMBER(atarig42_state,guardian)
{
	m_playfield_base = 0x000;

	/* it looks like they jsr to $80000 as some kind of protection */
	/* put an RTS there so we don't die */
	*(UINT16 *)&memregion("maincpu")->base()[0x80000] = 0x4E75;

	address_space &main = m_maincpu->space(AS_PROGRAM);
	m_sloop_base = main.install_readwrite_handler(0x000000, 0x07ffff, read16_delegate(FUNC(atarig42_state::guardians_sloop_data_r),this), write16_delegate(FUNC(atarig42_state::guardians_sloop_data_w),this));
	main.set_direct_update_handler(direct_update_delegate(FUNC(atarig42_state::atarig42_sloop_direct_handler), this));

	/*
	Guardians color MUX

	CRA10=MGEP*!AN.VID7*AN.0*!MO.0          -- if (mopri >= pfpri) && (!alpha) && (mopix != 0)

	CRA9=MGEP*!AN.VID7*AN.0*!MO.0*MVID9     -- if (mopri >= pfpri) && (!alpha) && (mopix != 0) && (mopix & 0x200)
	   +!MGEP*!AN.VID7*AN.0*PF.VID9         or if (mopri < pfpri) && (!alpha) && (pfpix & 0x200)
	   +!AN.VID7*AN.0*MO.0*PF.VID9          or if (mopix == 0) && (!alpha) && (pfpix & 0x200)

	CRA8=MGEP*!AN.VID7*AN.0*!MO.0*MVID8     -- if (mopri >= pfpri) && (!alpha) && (mopix != 0) && (mopix & 0x100)
	   +!MGEP*!AN.VID7*AN.0*PF.VID8         or if (mopri < pfpri) && (!alpha) && (pfpix & 0x100)
	   +!AN.VID7*AN.0*MO.0*PF.VID8          or if (mopix == 0) && (!alpha) && (pfpix & 0x100)

	CRMUXB=!AN.VID7*AN.0                    -- if (!alpha)

	CRMUXA=!MGEP                            -- if (mopri < pfpri)
	   +MO.0                                or (mopix == 0)
	   +AN.VID7                             or (alpha)
	   +!AN.0
*/
}



/*************************************
 *
 *  Game driver(s)
 *
 *************************************/

GAME( 1991, roadriot, 0,        atarig42_0x200, roadriot, atarig42_state, roadriot, ROT0, "Atari Games", "Road Riot 4WD (set 1, 13 Nov 1991)", MACHINE_UNEMULATED_PROTECTION )
GAME( 1991, roadrioto,roadriot, atarig42_0x200, roadriot, atarig42_state, roadriot, ROT0, "Atari Games", "Road Riot 4WD (set 2, 04 Jun 1991)", MACHINE_UNEMULATED_PROTECTION )
GAME( 1992, guardian, 0,        atarig42_0x400, guardian, atarig42_state, guardian, ROT0, "Atari Games", "Guardians of the 'Hood", 0 )
