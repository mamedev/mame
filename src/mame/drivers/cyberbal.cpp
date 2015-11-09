// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    Atari Cyberball hardware

    driver by Aaron Giles

    Games supported:
        * Cyberball (1988) [3 sets]
        * Cyberball 2072, 2-players (1989) [4 sets]
        * Tournament Cyberball 2072 (1989) [2 sets]

    Known bugs:
        * none at this time

****************************************************************************

    Memory map (TBA)

***************************************************************************/


#include "emu.h"
#include "sound/2151intf.h"
#include "rendlay.h"
#include "includes/cyberbal.h"


/*************************************
 *
 *  Initialization
 *
 *************************************/

void cyberbal_state::update_interrupts()
{
	if (m_extracpu != NULL)
	{
		m_maincpu->set_input_line(1, m_sound_int_state ? ASSERT_LINE : CLEAR_LINE);
		m_extracpu->set_input_line(1, m_video_int_state ? ASSERT_LINE : CLEAR_LINE);
	}
	else
	{
		m_maincpu->set_input_line(1, m_video_int_state ? ASSERT_LINE : CLEAR_LINE);
		m_maincpu->set_input_line(3, m_sound_int_state ? ASSERT_LINE : CLEAR_LINE);
	}
}


MACHINE_START_MEMBER(cyberbal_state,cyberbal)
{
	atarigen_state::machine_start();

	save_item(NAME(m_fast_68k_int));
	save_item(NAME(m_io_68k_int));
	save_item(NAME(m_sound_data_from_68k));
	save_item(NAME(m_sound_data_from_6502));
	save_item(NAME(m_sound_data_from_68k_ready));
	save_item(NAME(m_sound_data_from_6502_ready));
}


MACHINE_RESET_MEMBER(cyberbal_state,cyberbal)
{
	atarigen_state::machine_reset();
	scanline_timer_reset(*m_lscreen, 8);

	cyberbal_sound_reset();

	/* Extra CPU (second M68k) doesn't run until reset */
	m_extracpu->set_input_line(INPUT_LINE_RESET, ASSERT_LINE);
}


MACHINE_RESET_MEMBER(cyberbal_state,cyberbal2p)
{
	atarigen_state::machine_reset();
	scanline_timer_reset(*m_screen, 8);
}



/*************************************
 *
 *  I/O read dispatch.
 *
 *************************************/

READ16_MEMBER(cyberbal_state::sound_state_r)
{
	int temp = 0xffff;
	if (m_jsa->main_to_sound_ready()) temp ^= 0xffff;
	return temp;
}



/*************************************
 *
 *  Extra I/O handlers.
 *
 *************************************/

WRITE16_MEMBER(cyberbal_state::p2_reset_w)
{
	m_extracpu->set_input_line(INPUT_LINE_RESET, CLEAR_LINE);
}



/*************************************
 *
 *  Main CPU memory handlers
 *
 *************************************/

static ADDRESS_MAP_START( main_map, AS_PROGRAM, 16, cyberbal_state )
	AM_RANGE(0x000000, 0x03ffff) AM_ROM
	AM_RANGE(0xfc0000, 0xfc0fff) AM_DEVREADWRITE8("eeprom", atari_eeprom_device, read, write, 0x00ff)
	AM_RANGE(0xfc8000, 0xfcffff) AM_DEVREAD8("soundcomm", atari_sound_comm_device, main_response_r, 0xff00)
	AM_RANGE(0xfd0000, 0xfd1fff) AM_DEVWRITE("eeprom", atari_eeprom_device, unlock_write)
	AM_RANGE(0xfd2000, 0xfd3fff) AM_DEVWRITE("soundcomm", atari_sound_comm_device, sound_reset_w)
	AM_RANGE(0xfd4000, 0xfd5fff) AM_WRITE(watchdog_reset16_w)
	AM_RANGE(0xfd6000, 0xfd7fff) AM_WRITE(p2_reset_w)
	AM_RANGE(0xfd8000, 0xfd9fff) AM_DEVWRITE8("soundcomm", atari_sound_comm_device, main_command_w, 0xff00)
	AM_RANGE(0xfe0000, 0xfe0fff) AM_READ_PORT("IN0")
	AM_RANGE(0xfe1000, 0xfe1fff) AM_READ_PORT("IN1")
	AM_RANGE(0xfe8000, 0xfe8fff) AM_RAM_DEVWRITE("rpalette", palette_device, write) AM_SHARE("rpalette")
	AM_RANGE(0xfec000, 0xfecfff) AM_RAM_DEVWRITE("lpalette", palette_device, write) AM_SHARE("lpalette")
	AM_RANGE(0xff0000, 0xff1fff) AM_RAM_DEVWRITE("playfield2", tilemap_device, write) AM_SHARE("playfield2")
	AM_RANGE(0xff2000, 0xff2fff) AM_RAM_DEVWRITE("alpha2", tilemap_device, write) AM_SHARE("alpha2")
	AM_RANGE(0xff3000, 0xff37ff) AM_RAM AM_SHARE("mob2")
	AM_RANGE(0xff3800, 0xff3fff) AM_RAM AM_SHARE("ff3800")
	AM_RANGE(0xff4000, 0xff5fff) AM_RAM_DEVWRITE("playfield", tilemap_device, write) AM_SHARE("playfield")
	AM_RANGE(0xff6000, 0xff6fff) AM_RAM_DEVWRITE("alpha", tilemap_device, write) AM_SHARE("alpha")
	AM_RANGE(0xff7000, 0xff77ff) AM_RAM AM_SHARE("mob")
	AM_RANGE(0xff7800, 0xff9fff) AM_RAM AM_SHARE("sharedram")
	AM_RANGE(0xffa000, 0xffbfff) AM_READONLY AM_WRITENOP AM_SHARE("extraram")
	AM_RANGE(0xffc000, 0xffffff) AM_RAM AM_SHARE("mainram")
ADDRESS_MAP_END



/*************************************
 *
 *  Extra CPU memory handlers
 *
 *************************************/

static ADDRESS_MAP_START( extra_map, AS_PROGRAM, 16, cyberbal_state )
	AM_RANGE(0x000000, 0x03ffff) AM_ROM
	AM_RANGE(0xfc0000, 0xfdffff) AM_WRITE(video_int_ack_w)
	AM_RANGE(0xfe0000, 0xfe0fff) AM_READ_PORT("IN0")
	AM_RANGE(0xfe1000, 0xfe1fff) AM_READ_PORT("IN1")
	AM_RANGE(0xfe8000, 0xfe8fff) AM_RAM_DEVWRITE("rpalette", palette_device, write) AM_SHARE("rpalette")
	AM_RANGE(0xfec000, 0xfecfff) AM_RAM_DEVWRITE("lpalette", palette_device, write) AM_SHARE("lpalette")
	AM_RANGE(0xff0000, 0xff1fff) AM_RAM_DEVWRITE("playfield2", tilemap_device, write) AM_SHARE("playfield2")
	AM_RANGE(0xff2000, 0xff2fff) AM_RAM_DEVWRITE("alpha2", tilemap_device, write) AM_SHARE("alpha2")
	AM_RANGE(0xff3000, 0xff37ff) AM_RAM AM_SHARE("mob2")
	AM_RANGE(0xff3800, 0xff3fff) AM_RAM AM_SHARE("ff3800")
	AM_RANGE(0xff4000, 0xff5fff) AM_RAM_DEVWRITE("playfield", tilemap_device, write) AM_SHARE("playfield")
	AM_RANGE(0xff6000, 0xff6fff) AM_RAM_DEVWRITE("alpha", tilemap_device, write) AM_SHARE("alpha")
	AM_RANGE(0xff7000, 0xff77ff) AM_RAM AM_SHARE("mob")
	AM_RANGE(0xff7800, 0xff9fff) AM_RAM AM_SHARE("sharedram")
	AM_RANGE(0xffa000, 0xffbfff) AM_RAM AM_SHARE("extraram")
	AM_RANGE(0xffc000, 0xffffff) AM_READONLY AM_WRITENOP AM_SHARE("mainram")
ADDRESS_MAP_END



/*************************************
 *
 *  Sound CPU memory handlers
 *
 *************************************/

static ADDRESS_MAP_START( sound_map, AS_PROGRAM, 8, cyberbal_state )
	AM_RANGE(0x0000, 0x1fff) AM_RAM
	AM_RANGE(0x2000, 0x2001) AM_DEVREADWRITE("ymsnd", ym2151_device, read, write)
	AM_RANGE(0x2800, 0x2801) AM_WRITE(sound_68k_6502_w)
	AM_RANGE(0x2802, 0x2803) AM_DEVREADWRITE("soundcomm", atari_sound_comm_device, sound_irq_ack_r, sound_irq_ack_w)
	AM_RANGE(0x2804, 0x2805) AM_DEVWRITE("soundcomm", atari_sound_comm_device, sound_response_w)
	AM_RANGE(0x2806, 0x2807) AM_WRITE(sound_bank_select_w)
	AM_RANGE(0x2c00, 0x2c01) AM_DEVREAD("soundcomm", atari_sound_comm_device, sound_command_r)
	AM_RANGE(0x2c02, 0x2c03) AM_READ(special_port3_r)
	AM_RANGE(0x2c04, 0x2c05) AM_READ(sound_68k_6502_r)
	AM_RANGE(0x2c06, 0x2c07) AM_READ(sound_6502_stat_r)
	AM_RANGE(0x3000, 0x3fff) AM_ROMBANK("soundbank")
	AM_RANGE(0x4000, 0xffff) AM_ROM
ADDRESS_MAP_END



/*************************************
 *
 *  68000 Sound CPU memory handlers
 *
 *************************************/

static ADDRESS_MAP_START( sound_68k_map, AS_PROGRAM, 16, cyberbal_state )
	AM_RANGE(0x000000, 0x03ffff) AM_ROM
	AM_RANGE(0xff8000, 0xff87ff) AM_READ(sound_68k_r)
	AM_RANGE(0xff8800, 0xff8fff) AM_WRITE(sound_68k_w)
	AM_RANGE(0xff9000, 0xff97ff) AM_WRITE(io_68k_irq_ack_w)
	AM_RANGE(0xff9800, 0xff9fff) AM_WRITE(sound_68k_dac_w)
	AM_RANGE(0xfff000, 0xffffff) AM_RAM
ADDRESS_MAP_END



/*************************************
 *
 *  2-player main CPU memory handlers
 *
 *************************************/

static ADDRESS_MAP_START( cyberbal2p_map, AS_PROGRAM, 16, cyberbal_state )
	AM_RANGE(0x000000, 0x07ffff) AM_ROM
	AM_RANGE(0xfc0000, 0xfc0003) AM_READ_PORT("IN0")
	AM_RANGE(0xfc2000, 0xfc2003) AM_READ_PORT("IN1")
	AM_RANGE(0xfc4000, 0xfc4003) AM_READ_PORT("IN2")
	AM_RANGE(0xfc6000, 0xfc6003) AM_DEVREAD8("jsa", atari_jsa_ii_device, main_response_r, 0xff00)
	AM_RANGE(0xfc8000, 0xfc8fff) AM_DEVREADWRITE8("eeprom", atari_eeprom_device, read, write, 0x00ff)
	AM_RANGE(0xfca000, 0xfcafff) AM_RAM_DEVWRITE("palette", palette_device, write) AM_SHARE("palette")
	AM_RANGE(0xfd0000, 0xfd0003) AM_DEVWRITE("eeprom", atari_eeprom_device, unlock_write)
	AM_RANGE(0xfd2000, 0xfd2003) AM_DEVWRITE("jsa", atari_jsa_ii_device, sound_reset_w)
	AM_RANGE(0xfd4000, 0xfd4003) AM_WRITE(watchdog_reset16_w)
	AM_RANGE(0xfd6000, 0xfd6003) AM_WRITE(video_int_ack_w)
	AM_RANGE(0xfd8000, 0xfd8003) AM_DEVWRITE8("jsa", atari_jsa_ii_device, main_command_w, 0xff00)
	AM_RANGE(0xfe0000, 0xfe0003) AM_READ(sound_state_r)
	AM_RANGE(0xff0000, 0xff1fff) AM_RAM_DEVWRITE("playfield", tilemap_device, write) AM_SHARE("playfield")
	AM_RANGE(0xff2000, 0xff2fff) AM_RAM_DEVWRITE("alpha", tilemap_device, write) AM_SHARE("alpha")
	AM_RANGE(0xff3000, 0xff37ff) AM_RAM AM_SHARE("mob")
	AM_RANGE(0xff3800, 0xffffff) AM_RAM
ADDRESS_MAP_END



/*************************************
 *
 *  Port definitions
 *
 *************************************/

static INPUT_PORTS_START( cyberbal )
	PORT_START("IN0")       /* fe0000 */
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(4)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(4)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(4)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(4)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(4)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_ATARI_COMM_MAIN_TO_SOUND_READY("soundcomm")
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(3)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(3)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(3)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(3)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(3)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_SERVICE( 0x8000, IP_ACTIVE_LOW )

	PORT_START("IN1")       /* fe1000 */
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(2)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x00c0, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(1)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_VBLANK("lscreen")

	PORT_START("IN2")       /* fake port for screen switching */
	PORT_BIT( 0xffff, IP_ACTIVE_LOW, IPT_UNUSED )

	/* 2008-06 FP: I tag this as JSAII (even if it's not) to simplify cyberbal_special_port3_r */
	PORT_START("jsa:JSAII")     /* audio board port */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_COIN4 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_COIN3 )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_SPECIAL )   /* output buffer full */
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SPECIAL )    /* input buffer full */
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_SPECIAL )   /* self test */
INPUT_PORTS_END


static INPUT_PORTS_START( cyberbal2p )
	PORT_START("IN0")       /* fc0000 */
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0xffc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN1")       /* fc2000 */
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(2)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0xffc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN2")       /* fc4000 */
	PORT_BIT( 0x1fff, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_ATARI_JSA_MAIN_TO_SOUND_READY("jsa")
	PORT_BIT( 0x4000, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_VBLANK("screen")
	PORT_SERVICE( 0x8000, IP_ACTIVE_LOW )
INPUT_PORTS_END



/*************************************
 *
 *  Graphics definitions
 *
 *************************************/

static const gfx_layout pfanlayout =
{
	16,8,
	RGN_FRAC(1,1),
	4,
	{ 0, 1, 2, 3 },
	{ 0,0, 4,4, 8,8, 12,12, 16,16, 20,20, 24,24, 28,28 },
	{ 0*8, 4*8, 8*8, 12*8, 16*8, 20*8, 24*8, 28*8 },
	32*8
};

static const gfx_layout pfanlayout_interleaved =
{
	16,8,
	RGN_FRAC(1,2),
	4,
	{ 0, 1, 2, 3 },
	{ RGN_FRAC(1,2)+0,RGN_FRAC(1,2)+0, RGN_FRAC(1,2)+4,RGN_FRAC(1,2)+4, 0,0, 4,4, RGN_FRAC(1,2)+8,RGN_FRAC(1,2)+8, RGN_FRAC(1,2)+12,RGN_FRAC(1,2)+12, 8,8, 12,12 },
	{ 0*8, 2*8, 4*8, 6*8, 8*8, 10*8, 12*8, 14*8 },
	16*8
};

static const gfx_layout molayout =
{
	16,8,
	RGN_FRAC(1,4),
	4,
	{ 0, 1, 2, 3 },
	{ RGN_FRAC(3,4)+0, RGN_FRAC(3,4)+4, RGN_FRAC(2,4)+0, RGN_FRAC(2,4)+4, RGN_FRAC(1,4)+0, RGN_FRAC(1,4)+4, 0, 4,
		RGN_FRAC(3,4)+8, RGN_FRAC(3,4)+12, RGN_FRAC(2,4)+8, RGN_FRAC(2,4)+12, RGN_FRAC(1,4)+8, RGN_FRAC(1,4)+12, 8, 12 },
	{ 0*8, 2*8, 4*8, 6*8, 8*8, 10*8, 12*8, 14*8 },
	16*8
};

static GFXDECODE_START( cyberbal )
	GFXDECODE_ENTRY( "gfx2", 0, pfanlayout,     0, 128 )
	GFXDECODE_ENTRY( "gfx1", 0, molayout,   0x600, 16 )
	GFXDECODE_ENTRY( "gfx3", 0, pfanlayout, 0x780, 8 )
GFXDECODE_END

static GFXDECODE_START( interleaved )
	GFXDECODE_ENTRY( "gfx2", 0, pfanlayout_interleaved,     0, 128 )
	GFXDECODE_ENTRY( "gfx1", 0, molayout,               0x600, 16 )
	GFXDECODE_ENTRY( "gfx3", 0, pfanlayout_interleaved, 0x780, 8 )
GFXDECODE_END



/*************************************
 *
 *  Machine driver
 *
 *************************************/

static MACHINE_CONFIG_START( cyberbal, cyberbal_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68000, ATARI_CLOCK_14MHz/2)
	MCFG_CPU_PROGRAM_MAP(main_map)

	MCFG_SLAPSTIC_ADD("slapstic")

	MCFG_CPU_ADD("audiocpu", M6502, ATARI_CLOCK_14MHz/8)
	MCFG_CPU_PROGRAM_MAP(sound_map)
	MCFG_DEVICE_PERIODIC_INT_DEVICE("soundcomm", atari_sound_comm_device, sound_irq_gen, (double)ATARI_CLOCK_14MHz/4/4/16/16/14)

	MCFG_CPU_ADD("extra", M68000, ATARI_CLOCK_14MHz/2)
	MCFG_CPU_PROGRAM_MAP(extra_map)
	MCFG_DEVICE_VBLANK_INT_DRIVER("lscreen", atarigen_state, video_int_gen) /* or is it "right?" */

	MCFG_CPU_ADD("dac", M68000, ATARI_CLOCK_14MHz/2)
	MCFG_CPU_PROGRAM_MAP(sound_68k_map)
	MCFG_CPU_PERIODIC_INT_DRIVER(cyberbal_state, sound_68k_irq_gen,  10000)

	MCFG_QUANTUM_TIME(attotime::from_hz(600))

	MCFG_MACHINE_START_OVERRIDE(cyberbal_state,cyberbal)
	MCFG_MACHINE_RESET_OVERRIDE(cyberbal_state,cyberbal)

	MCFG_ATARI_EEPROM_2804_ADD("eeprom")

	/* video hardware */
	MCFG_GFXDECODE_ADD("gfxdecode", "lpalette", interleaved)

	MCFG_PALETTE_ADD("lpalette", 2048)
	MCFG_PALETTE_FORMAT(IRRRRRGGGGGBBBBB)
	MCFG_PALETTE_ADD("rpalette", 2048)
	MCFG_PALETTE_FORMAT(IRRRRRGGGGGBBBBB)

	MCFG_TILEMAP_ADD_STANDARD("playfield", "gfxdecode", 2, cyberbal_state, get_playfield_tile_info, 16,8, SCAN_ROWS, 64,64)
	MCFG_TILEMAP_ADD_STANDARD_TRANSPEN("alpha", "gfxdecode", 2, cyberbal_state, get_alpha_tile_info, 16,8, SCAN_ROWS, 64,32, 0)
	MCFG_ATARI_MOTION_OBJECTS_ADD("mob", "lscreen", cyberbal_state::s_mob_config)
	MCFG_ATARI_MOTION_OBJECTS_GFXDECODE("gfxdecode")

	MCFG_TILEMAP_ADD_STANDARD("playfield2", "gfxdecode", 2, cyberbal_state, get_playfield_tile_info, 16,8, SCAN_ROWS, 64,64)
	MCFG_TILEMAP_ADD_STANDARD_TRANSPEN("alpha2", "gfxdecode", 2, cyberbal_state, get_alpha_tile_info, 16,8, SCAN_ROWS, 64,32, 0)
	MCFG_ATARI_MOTION_OBJECTS_ADD("mob2", "rscreen", cyberbal_state::s_mob_config)
	MCFG_ATARI_MOTION_OBJECTS_GFXDECODE("gfxdecode")

	MCFG_SCREEN_ADD("lscreen", RASTER)
	MCFG_SCREEN_VIDEO_ATTRIBUTES(VIDEO_UPDATE_BEFORE_VBLANK)
	/* note: these parameters are from published specs, not derived */
	/* the board uses an SOS-2 chip to generate video signals */
	MCFG_SCREEN_RAW_PARAMS(ATARI_CLOCK_14MHz, 456*2, 0, 336*2, 262, 0, 240)
	MCFG_SCREEN_UPDATE_DRIVER(cyberbal_state, screen_update_cyberbal_left)
	MCFG_SCREEN_PALETTE("lpalette")

	MCFG_SCREEN_ADD("rscreen", RASTER)
	MCFG_SCREEN_VIDEO_ATTRIBUTES(VIDEO_UPDATE_BEFORE_VBLANK)
	/* note: these parameters are from published specs, not derived */
	/* the board uses an SOS-2 chip to generate video signals */
	MCFG_SCREEN_RAW_PARAMS(ATARI_CLOCK_14MHz, 456*2, 0, 336*2, 262, 0, 240)
	MCFG_SCREEN_UPDATE_DRIVER(cyberbal_state, screen_update_cyberbal_right)
	MCFG_SCREEN_PALETTE("rpalette")

	MCFG_VIDEO_START_OVERRIDE(cyberbal_state,cyberbal)

	/* sound hardware */
	MCFG_ATARI_SOUND_COMM_ADD("soundcomm", "audiocpu", WRITELINE(atarigen_state, sound_int_write_line))
	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MCFG_YM2151_ADD("ymsnd", ATARI_CLOCK_14MHz/4)
	MCFG_YM2151_IRQ_HANDLER(DEVWRITELINE("soundcomm", atari_sound_comm_device, ym2151_irq_gen))
	MCFG_SOUND_ROUTE(0, "lspeaker", 0.60)
	MCFG_SOUND_ROUTE(1, "rspeaker", 0.60)

	MCFG_DAC_ADD("dac1")
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 1.0)

	MCFG_DAC_ADD("dac2")
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 1.0)
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( cyberbalt, cyberbal )
	MCFG_DEVICE_REMOVE("eeprom")
	MCFG_ATARI_EEPROM_2816_ADD("eeprom")
MACHINE_CONFIG_END


static MACHINE_CONFIG_START( cyberbal2p, cyberbal_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68000, ATARI_CLOCK_14MHz/2)
	MCFG_CPU_PROGRAM_MAP(cyberbal2p_map)
	MCFG_DEVICE_VBLANK_INT_DRIVER("screen", atarigen_state, video_int_gen)

	MCFG_MACHINE_START_OVERRIDE(cyberbal_state,cyberbal)
	MCFG_MACHINE_RESET_OVERRIDE(cyberbal_state,cyberbal2p)

	MCFG_ATARI_EEPROM_2816_ADD("eeprom")

	/* video hardware */
	MCFG_GFXDECODE_ADD("gfxdecode", "palette", cyberbal)
	MCFG_PALETTE_ADD("palette", 2048)
	MCFG_PALETTE_FORMAT(IRRRRRGGGGGBBBBB)

	MCFG_TILEMAP_ADD_STANDARD("playfield", "gfxdecode", 2, cyberbal_state, get_playfield_tile_info, 16,8, SCAN_ROWS, 64,64)
	MCFG_TILEMAP_ADD_STANDARD_TRANSPEN("alpha", "gfxdecode", 2, cyberbal_state, get_alpha_tile_info, 16,8, SCAN_ROWS, 64,32, 0)
	MCFG_ATARI_MOTION_OBJECTS_ADD("mob", "screen", cyberbal_state::s_mob_config)
	MCFG_ATARI_MOTION_OBJECTS_GFXDECODE("gfxdecode")

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_VIDEO_ATTRIBUTES(VIDEO_UPDATE_BEFORE_VBLANK)
	/* note: these parameters are from published specs, not derived */
	/* the board uses an SOS-2 chip to generate video signals */
	MCFG_SCREEN_RAW_PARAMS(ATARI_CLOCK_14MHz, 456*2, 0, 336*2, 262, 0, 240)
	MCFG_SCREEN_UPDATE_DRIVER(cyberbal_state, screen_update_cyberbal2p)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_VIDEO_START_OVERRIDE(cyberbal_state,cyberbal2p)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_ATARI_JSA_II_ADD("jsa", WRITELINE(atarigen_state, sound_int_write_line))
	MCFG_ATARI_JSA_TEST_PORT("IN2", 15)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_CONFIG_END



/*************************************
 *
 *  ROM definition(s)
 *
 *************************************/

ROM_START( cyberbal )
	ROM_REGION( 0x40000, "maincpu", 0 ) /* 4*64k for 68000 code */
	ROM_LOAD16_BYTE( "136064-4123.1m",  0x000000, 0x010000, CRC(fb872740) SHA1(15e6721d466fe56d7c97c6801e214b32803a0a0d) )
	ROM_LOAD16_BYTE( "136064-4124.1kl", 0x000001, 0x010000, CRC(87babad9) SHA1(acdc6b5976445e203de19eb03697e307fe6da77d) )

	ROM_REGION( 0x14000, "audiocpu", 0 )    /* 64k for 6502 code */
	ROM_LOAD( "136064-2131.2f",  0x010000, 0x004000, CRC(bd7e3d84) SHA1(f87878042fc79fa3883136b31ac15ddc22c6023c) )
	ROM_CONTINUE(             0x004000, 0x00c000 )

	ROM_REGION( 0x40000, "extra", 0 )   /* 4*64k for 68000 code */
	ROM_LOAD16_BYTE( "136064-2127.3cd", 0x000000, 0x010000, CRC(3e5feb1f) SHA1(9f92f496adbdf74e394e0d710d6471b9666ba5e5) )
	ROM_LOAD16_BYTE( "136064-2128.1b",  0x000001, 0x010000, CRC(4e642cc3) SHA1(f708b6dd4360380bb10059d66df66b07966210b4) )
	ROM_LOAD16_BYTE( "136064-1129.1cd", 0x020000, 0x010000, CRC(db11d2f0) SHA1(da9f49af533cbc732b17699040c7930070a90644) )
	ROM_LOAD16_BYTE( "136064-1130.3b",  0x020001, 0x010000, CRC(fd86b8aa) SHA1(46310efed762632ed176a08aaec41e48aad41cc1) )

	ROM_REGION16_BE( 0x40000, "dac", 0 )    /* 256k for 68000 sound code */
	ROM_LOAD16_BYTE( "136064-1132.3cd", 0x000000, 0x010000, CRC(ca5ce8d8) SHA1(69dc83d43d8c9dc7ce3207e70f48fcfc5ddda0cc) )
	ROM_LOAD16_BYTE( "136064-1133.1b",  0x000001, 0x010000, CRC(ffeb8746) SHA1(0d8d28b2d997ff3cf01b4ef25b75fa5a69754af4) )
	ROM_LOAD16_BYTE( "136064-1134.1cd", 0x020000, 0x010000, CRC(bcbd4c00) SHA1(f0bfcdf0b5491e15872b543e99b834ae384cbf18) )
	ROM_LOAD16_BYTE( "136064-1135.3b",  0x020001, 0x010000, CRC(d520f560) SHA1(fb0b8d021458379188c424a343622c46ad74edaa) )

	ROM_REGION( 0x140000, "gfx1", 0 )
	ROM_LOAD( "136064-1105.15a", 0x000000, 0x010000, CRC(e770eb3e) SHA1(e9f9e9e05774005c8be3bbdc19985b59a0081ef4) )
	ROM_LOAD( "136064-1109.16a", 0x010000, 0x010000, CRC(40db00da) SHA1(d92d856b06f6ba11621ba7aab3d40653b3c70159) )
	ROM_LOAD( "136064-2113.18a", 0x020000, 0x010000, CRC(52bb08fb) SHA1(08caa156923daf444e0caafb2cdff0704c90ef1f) )
	ROM_LOAD( "136064-1117.19a", 0x030000, 0x010000, CRC(0a11d877) SHA1(876cbeffd815c084d7cbd937067d65c04aeebce5) )
	ROM_LOAD( "136064-1106.11a", 0x050000, 0x010000, CRC(6f53c7c1) SHA1(5856d714c3af338be58156b404fb1e5a89c24cf9) )
	ROM_LOAD( "136064-1110.12a", 0x060000, 0x010000, CRC(5de609e5) SHA1(bbea36a4cbbfeab113925951ef097673eddf26a8) )
	ROM_LOAD( "136064-2114.13a", 0x070000, 0x010000, CRC(e6f95010) SHA1(42b14cf0dadfab9ce1032385fd21339b46edcfc2) )
	ROM_LOAD( "136064-1118.14a", 0x080000, 0x010000, CRC(47f56ced) SHA1(62e80191e1879ffb6c736aec004bbc30a366363f) )
	ROM_LOAD( "136064-1107.15c", 0x0a0000, 0x010000, CRC(c8f1f7ff) SHA1(2e0374901871d66a243f87bc4b9cbdde5505f0ec) )
	ROM_LOAD( "136064-1111.16c", 0x0b0000, 0x010000, CRC(6bf0bf98) SHA1(7d2b3b61da3749b352a6bf3f1ae1cb736b5b8386) )
	ROM_LOAD( "136064-2115.18c", 0x0c0000, 0x010000, CRC(c3168603) SHA1(43e00fc739d1b3dd6d925bad63058fe74c1efc74) )
	ROM_LOAD( "136064-1119.19c", 0x0d0000, 0x010000, CRC(7ff29d09) SHA1(81458b058f0b037778f255b5afe94a44aba74829) )
	ROM_LOAD( "136064-1108.11c", 0x0f0000, 0x010000, CRC(99629412) SHA1(53a91b2a1ac62259ec9f78421b22c7b22f4233d6) )
	ROM_LOAD( "136064-1112.12c", 0x100000, 0x010000, CRC(aa198cb7) SHA1(aad4a60210289d2e5a93aac336ba995ed6ac4886) )
	ROM_LOAD( "136064-2116.13c", 0x110000, 0x010000, CRC(6cf79a67) SHA1(7f3271b575cf0b5033b5b19f0e71fae251040fc6) )
	ROM_LOAD( "136064-1120.14c", 0x120000, 0x010000, CRC(40bdf767) SHA1(c57aaea838abeaea1a0060c45c2f33c38a51edb3) )

	ROM_REGION( 0x040000, "gfx2", 0 )
	ROM_LOAD( "136064-1101.9lm", 0x000000, 0x010000, CRC(a64b4da8) SHA1(f68778adb56a1eb964acdbc7e9d690a8a83f170b) )
	ROM_LOAD( "136064-1102.8lm", 0x010000, 0x010000, CRC(ca91ec1b) SHA1(970c64e19893503cae796daee63b2d7d08eaf551) )
	ROM_LOAD( "136064-1103.11lm", 0x020000, 0x010000, CRC(ee29d1d1) SHA1(2a7fea25728c66ce482de76299413ef5da01beef) )
	ROM_LOAD( "136064-1104.10lm", 0x030000, 0x010000, CRC(882649f8) SHA1(fbaea597b6e318234e41df245023643f448a4938) )

	ROM_REGION( 0x020000, "gfx3", 0 )
	ROM_LOAD( "136064-1121.15n", 0x000000, 0x010000, CRC(0ca1e3b3) SHA1(d934bc9a1def4404fb86175878404cbb18127a11) )
	ROM_LOAD( "136064-1122.16n", 0x010000, 0x010000, CRC(882f4e1c) SHA1(f7517ff03502ff029fb375260a35e45414567433) )

	ROM_REGION( 0x0a00, "plds", 0 )
	ROM_LOAD( "gal16v8-136064-1031.b21", 0x0000, 0x0117, CRC(e7e4ab09) SHA1(04f9413c77dca9f533d4cd796c3e416920b4dbbc) )
	ROM_LOAD( "gal16v8-136064-1027.d52", 0x0200, 0x0117, CRC(fbd6afcd) SHA1(7a3981ce4b0141b9e0877962b7db07d07ca4129c) )
	ROM_LOAD( "gal16v8-136064-1028.b56", 0x0400, 0x0117, CRC(12d1a257) SHA1(32914dffd58ce694913c2108a27b078422a9dc63) )
	ROM_LOAD( "gal16v8-136064-1029.d58", 0x0600, 0x0117, CRC(fd39d238) SHA1(55c1b9a56c9b2bfa434eed54f7baea436ea141b8) )
	ROM_LOAD( "gal16v8-136064-1030.d91", 0x0800, 0x0117, CRC(84102588) SHA1(b6bffb47e5975c96b056d07357eb020caf3f0a0a) )

	ROM_REGION( 0x200, "eeprom:eeprom", 0 )
	ROM_LOAD( "cyberbal-eeprom.bin", 0x0000, 0x200, CRC(c6f256b2) SHA1(e0c62adcd9fd38e9d3ac60e6b08d468e04a350c6) )
ROM_END


ROM_START( cyberbal2 )
	ROM_REGION( 0x40000, "maincpu", 0 ) /* 4*64k for 68000 code */
	ROM_LOAD16_BYTE( "136064-2123.1m",  0x000000, 0x010000, CRC(502676e8) SHA1(c0f1f1ce50d3df21cb81244268faef6c303cdfab) )
	ROM_LOAD16_BYTE( "136064-2124.1kl", 0x000001, 0x010000, CRC(30f55915) SHA1(ab93ec46f282ab9a0cd47c989537a7e036975e3f) )

	ROM_REGION( 0x14000, "audiocpu", 0 )    /* 64k for 6502 code */
	ROM_LOAD( "136064-2131.2f",  0x010000, 0x004000, CRC(bd7e3d84) SHA1(f87878042fc79fa3883136b31ac15ddc22c6023c) )
	ROM_CONTINUE(             0x004000, 0x00c000 )

	ROM_REGION( 0x40000, "extra", 0 )   /* 4*64k for 68000 code */
	ROM_LOAD16_BYTE( "136064-2127.3cd", 0x000000, 0x010000, CRC(3e5feb1f) SHA1(9f92f496adbdf74e394e0d710d6471b9666ba5e5) )
	ROM_LOAD16_BYTE( "136064-2128.1b",  0x000001, 0x010000, CRC(4e642cc3) SHA1(f708b6dd4360380bb10059d66df66b07966210b4) )
	ROM_LOAD16_BYTE( "136064-1129.1cd", 0x020000, 0x010000, CRC(db11d2f0) SHA1(da9f49af533cbc732b17699040c7930070a90644) )
	ROM_LOAD16_BYTE( "136064-1130.3b",  0x020001, 0x010000, CRC(fd86b8aa) SHA1(46310efed762632ed176a08aaec41e48aad41cc1) )

	ROM_REGION16_BE( 0x40000, "dac", 0 )    /* 256k for 68000 sound code */
	ROM_LOAD16_BYTE( "136064-1132.3cd", 0x000000, 0x010000, CRC(ca5ce8d8) SHA1(69dc83d43d8c9dc7ce3207e70f48fcfc5ddda0cc) )
	ROM_LOAD16_BYTE( "136064-1133.1b",  0x000001, 0x010000, CRC(ffeb8746) SHA1(0d8d28b2d997ff3cf01b4ef25b75fa5a69754af4) )
	ROM_LOAD16_BYTE( "136064-1134.1cd", 0x020000, 0x010000, CRC(bcbd4c00) SHA1(f0bfcdf0b5491e15872b543e99b834ae384cbf18) )
	ROM_LOAD16_BYTE( "136064-1135.3b",  0x020001, 0x010000, CRC(d520f560) SHA1(fb0b8d021458379188c424a343622c46ad74edaa) )

	ROM_REGION( 0x140000, "gfx1", 0 )
	ROM_LOAD( "136064-1105.15a", 0x000000, 0x010000, CRC(e770eb3e) SHA1(e9f9e9e05774005c8be3bbdc19985b59a0081ef4) )
	ROM_LOAD( "136064-1109.16a", 0x010000, 0x010000, CRC(40db00da) SHA1(d92d856b06f6ba11621ba7aab3d40653b3c70159) )
	ROM_LOAD( "136064-2113.18a", 0x020000, 0x010000, CRC(52bb08fb) SHA1(08caa156923daf444e0caafb2cdff0704c90ef1f) )
	ROM_LOAD( "136064-1117.19a", 0x030000, 0x010000, CRC(0a11d877) SHA1(876cbeffd815c084d7cbd937067d65c04aeebce5) )
	ROM_LOAD( "136064-1106.11a", 0x050000, 0x010000, CRC(6f53c7c1) SHA1(5856d714c3af338be58156b404fb1e5a89c24cf9) )
	ROM_LOAD( "136064-1110.12a", 0x060000, 0x010000, CRC(5de609e5) SHA1(bbea36a4cbbfeab113925951ef097673eddf26a8) )
	ROM_LOAD( "136064-2114.13a", 0x070000, 0x010000, CRC(e6f95010) SHA1(42b14cf0dadfab9ce1032385fd21339b46edcfc2) )
	ROM_LOAD( "136064-1118.14a", 0x080000, 0x010000, CRC(47f56ced) SHA1(62e80191e1879ffb6c736aec004bbc30a366363f) )
	ROM_LOAD( "136064-1107.15c", 0x0a0000, 0x010000, CRC(c8f1f7ff) SHA1(2e0374901871d66a243f87bc4b9cbdde5505f0ec) )
	ROM_LOAD( "136064-1111.16c", 0x0b0000, 0x010000, CRC(6bf0bf98) SHA1(7d2b3b61da3749b352a6bf3f1ae1cb736b5b8386) )
	ROM_LOAD( "136064-2115.18c", 0x0c0000, 0x010000, CRC(c3168603) SHA1(43e00fc739d1b3dd6d925bad63058fe74c1efc74) )
	ROM_LOAD( "136064-1119.19c", 0x0d0000, 0x010000, CRC(7ff29d09) SHA1(81458b058f0b037778f255b5afe94a44aba74829) )
	ROM_LOAD( "136064-1108.11c", 0x0f0000, 0x010000, CRC(99629412) SHA1(53a91b2a1ac62259ec9f78421b22c7b22f4233d6) )
	ROM_LOAD( "136064-1112.12c", 0x100000, 0x010000, CRC(aa198cb7) SHA1(aad4a60210289d2e5a93aac336ba995ed6ac4886) )
	ROM_LOAD( "136064-2116.13c", 0x110000, 0x010000, CRC(6cf79a67) SHA1(7f3271b575cf0b5033b5b19f0e71fae251040fc6) )
	ROM_LOAD( "136064-1120.14c", 0x120000, 0x010000, CRC(40bdf767) SHA1(c57aaea838abeaea1a0060c45c2f33c38a51edb3) )

	ROM_REGION( 0x040000, "gfx2", 0 )
	ROM_LOAD( "136064-1101.9lm", 0x000000, 0x010000, CRC(a64b4da8) SHA1(f68778adb56a1eb964acdbc7e9d690a8a83f170b) )
	ROM_LOAD( "136064-1102.8lm", 0x010000, 0x010000, CRC(ca91ec1b) SHA1(970c64e19893503cae796daee63b2d7d08eaf551) )
	ROM_LOAD( "136064-1103.11lm", 0x020000, 0x010000, CRC(ee29d1d1) SHA1(2a7fea25728c66ce482de76299413ef5da01beef) )
	ROM_LOAD( "136064-1104.10lm", 0x030000, 0x010000, CRC(882649f8) SHA1(fbaea597b6e318234e41df245023643f448a4938) )

	ROM_REGION( 0x020000, "gfx3", 0 )
	ROM_LOAD( "136064-1121.15n", 0x000000, 0x010000, CRC(0ca1e3b3) SHA1(d934bc9a1def4404fb86175878404cbb18127a11) )
	ROM_LOAD( "136064-1122.16n", 0x010000, 0x010000, CRC(882f4e1c) SHA1(f7517ff03502ff029fb375260a35e45414567433) )

	ROM_REGION( 0x200, "eeprom:eeprom", 0 )
	ROM_LOAD( "cyberbal-eeprom.bin", 0x0000, 0x200, CRC(c6f256b2) SHA1(e0c62adcd9fd38e9d3ac60e6b08d468e04a350c6) )
ROM_END


ROM_START( cyberbalp )
	ROM_REGION( 0x40000, "maincpu", 0 ) /* 4*64k for 68000 code */
	ROM_LOAD16_BYTE( "136064-0123.1m",  0x000000, 0x010000, CRC(59bac810) SHA1(d4742b2b554c2ad62a2ea7152db3f06a06cddd67) )
	ROM_LOAD16_BYTE( "136064-0124.1kl", 0x000001, 0x010000, CRC(e48e6dd3) SHA1(4d45bc66c0a3eb1174db7f19c5dee54dabad68f3) )

	ROM_REGION( 0x14000, "audiocpu", 0 )    /* 64k for 6502 code */
	ROM_LOAD( "136064-0131.2f",  0x010000, 0x004000, CRC(c72b71ce) SHA1(6d3d8f437cf55ccaaa4490daa69f402902944686) )
	ROM_CONTINUE(                0x004000, 0x00c000 )

	ROM_REGION( 0x40000, "extra", 0 )   /* 4*64k for 68000 code */
	ROM_LOAD16_BYTE( "136064-0127.3cd", 0x000000, 0x010000, CRC(37ad3420) SHA1(e9c1ea59f5f9a133822a49027b4abf02af855ca2) )
	ROM_LOAD16_BYTE( "136064-0128.1b",  0x000001, 0x010000, CRC(d89aa8af) SHA1(b9faca1a775c1d699335a5ac0e1d25e8370c02a7) )
	ROM_LOAD16_BYTE( "136064-1129.1cd", 0x020000, 0x010000, CRC(db11d2f0) SHA1(da9f49af533cbc732b17699040c7930070a90644) )
	ROM_LOAD16_BYTE( "136064-1130.3b",  0x020001, 0x010000, CRC(fd86b8aa) SHA1(46310efed762632ed176a08aaec41e48aad41cc1) )

	ROM_REGION16_BE( 0x40000, "dac", 0 )    /* 256k for 68000 sound code */
	ROM_LOAD16_BYTE( "136064-0132.3cd", 0x000000, 0x010000, CRC(392a442c) SHA1(e108456167f433808b452edb3351d283b5cf7a50) )
	ROM_LOAD16_BYTE( "136064-0133.1b",  0x000001, 0x010000, CRC(a0a11cf9) SHA1(b28a379cb49d051b6ccff877255409e1231d3030) )
	ROM_LOAD16_BYTE( "136064-1134.1cd", 0x020000, 0x010000, CRC(bcbd4c00) SHA1(f0bfcdf0b5491e15872b543e99b834ae384cbf18) )
	ROM_LOAD16_BYTE( "136064-1135.3b",  0x020001, 0x010000, CRC(d520f560) SHA1(fb0b8d021458379188c424a343622c46ad74edaa) )

	ROM_REGION( 0x140000, "gfx1", 0 )
	ROM_LOAD( "136064-1105.15a", 0x000000, 0x010000, CRC(e770eb3e) SHA1(e9f9e9e05774005c8be3bbdc19985b59a0081ef4) )
	ROM_LOAD( "136064-1109.16a", 0x010000, 0x010000, CRC(40db00da) SHA1(d92d856b06f6ba11621ba7aab3d40653b3c70159) )
	ROM_LOAD( "136064-2113.18a", 0x020000, 0x010000, CRC(52bb08fb) SHA1(08caa156923daf444e0caafb2cdff0704c90ef1f) )
	ROM_LOAD( "136064-1117.19a", 0x030000, 0x010000, CRC(0a11d877) SHA1(876cbeffd815c084d7cbd937067d65c04aeebce5) )
	ROM_LOAD( "136064-1106.11a", 0x050000, 0x010000, CRC(6f53c7c1) SHA1(5856d714c3af338be58156b404fb1e5a89c24cf9) )
	ROM_LOAD( "136064-1110.12a", 0x060000, 0x010000, CRC(5de609e5) SHA1(bbea36a4cbbfeab113925951ef097673eddf26a8) )
	ROM_LOAD( "136064-2114.13a", 0x070000, 0x010000, CRC(e6f95010) SHA1(42b14cf0dadfab9ce1032385fd21339b46edcfc2) )
	ROM_LOAD( "136064-1118.14a", 0x080000, 0x010000, CRC(47f56ced) SHA1(62e80191e1879ffb6c736aec004bbc30a366363f) )
	ROM_LOAD( "136064-1107.15c", 0x0a0000, 0x010000, CRC(c8f1f7ff) SHA1(2e0374901871d66a243f87bc4b9cbdde5505f0ec) )
	ROM_LOAD( "136064-1111.16c", 0x0b0000, 0x010000, CRC(6bf0bf98) SHA1(7d2b3b61da3749b352a6bf3f1ae1cb736b5b8386) )
	ROM_LOAD( "136064-2115.18c", 0x0c0000, 0x010000, CRC(c3168603) SHA1(43e00fc739d1b3dd6d925bad63058fe74c1efc74) )
	ROM_LOAD( "136064-1119.19c", 0x0d0000, 0x010000, CRC(7ff29d09) SHA1(81458b058f0b037778f255b5afe94a44aba74829) )
	ROM_LOAD( "136064-1108.11c", 0x0f0000, 0x010000, CRC(99629412) SHA1(53a91b2a1ac62259ec9f78421b22c7b22f4233d6) )
	ROM_LOAD( "136064-1112.12c", 0x100000, 0x010000, CRC(aa198cb7) SHA1(aad4a60210289d2e5a93aac336ba995ed6ac4886) )
	ROM_LOAD( "136064-2116.13c", 0x110000, 0x010000, CRC(6cf79a67) SHA1(7f3271b575cf0b5033b5b19f0e71fae251040fc6) )
	ROM_LOAD( "136064-1120.14c", 0x120000, 0x010000, CRC(40bdf767) SHA1(c57aaea838abeaea1a0060c45c2f33c38a51edb3) )

	ROM_REGION( 0x040000, "gfx2", 0 )
	ROM_LOAD( "136064-1101.9lm", 0x000000, 0x010000, CRC(a64b4da8) SHA1(f68778adb56a1eb964acdbc7e9d690a8a83f170b) )
	ROM_LOAD( "136064-1102.8lm", 0x010000, 0x010000, CRC(ca91ec1b) SHA1(970c64e19893503cae796daee63b2d7d08eaf551) )
	ROM_LOAD( "136064-1103.11lm", 0x020000, 0x010000, CRC(ee29d1d1) SHA1(2a7fea25728c66ce482de76299413ef5da01beef) )
	ROM_LOAD( "136064-1104.10lm", 0x030000, 0x010000, CRC(882649f8) SHA1(fbaea597b6e318234e41df245023643f448a4938) )

	ROM_REGION( 0x020000, "gfx3", 0 )
	ROM_LOAD( "136064-1121.15n", 0x000000, 0x010000, CRC(0ca1e3b3) SHA1(d934bc9a1def4404fb86175878404cbb18127a11) )
	ROM_LOAD( "136064-1122.16n", 0x010000, 0x010000, CRC(882f4e1c) SHA1(f7517ff03502ff029fb375260a35e45414567433) )

	ROM_REGION( 0x200, "eeprom:eeprom", 0 )
	ROM_LOAD( "cyberbal-eeprom.bin", 0x0000, 0x200, CRC(c6f256b2) SHA1(e0c62adcd9fd38e9d3ac60e6b08d468e04a350c6) )
ROM_END


ROM_START( cyberbal2p )
	ROM_REGION( 0x80000, "maincpu", 0 ) /* 8*64k for 68000 code */
	ROM_LOAD16_BYTE( "136071-4019.10c", 0x000000, 0x010000, CRC(20c6b09c) SHA1(b12f9247621fe0242171140256b7b92c996dcb19) )
	ROM_LOAD16_BYTE( "136071-4020.10d", 0x000001, 0x010000, CRC(eaa6c524) SHA1(0bc48dca1f10fbb3dec441d06f447637b6c70356) )
	ROM_LOAD16_BYTE( "136071-4021.21c", 0x020000, 0x010000, CRC(89ffa885) SHA1(d276209fd72c753d23571464e51d701a54cc3e0e) )
	ROM_LOAD16_BYTE( "136071-4022.21d", 0x020001, 0x010000, CRC(35da3402) SHA1(7b51e0fb202699457a3b8e2bd8a68fef0889435f) )
	ROM_LOAD16_BYTE( "136071-2023.15c", 0x040000, 0x010000, CRC(e541b08f) SHA1(345843209b02fb50cb08f55f80036046873b834f) )
	ROM_LOAD16_BYTE( "136071-2024.15d", 0x040001, 0x010000, CRC(5a77ee95) SHA1(441a45a358eb78cc140c96dc4c42b30e1929ad07) )
	ROM_LOAD16_BYTE( "136071-1025.27c", 0x060000, 0x010000, CRC(95ff68c6) SHA1(43f716a4c44fe1a38fcc6e2600bac948bb603504) )
	ROM_LOAD16_BYTE( "136071-1026.27d", 0x060001, 0x010000, CRC(f61c4898) SHA1(9e4a14eac6d197f63c3392af3d804e81c034cb09) )

	ROM_REGION( 0x14000, "jsa:cpu", 0 ) /* 64k for 6502 code */
	ROM_LOAD( "136071-1042.1b",  0x010000, 0x004000, CRC(e63cf125) SHA1(449880f561660ba67ac2d7f8ce6333768e0ae0be) )
	ROM_CONTINUE(                0x004000, 0x00c000 )

	ROM_REGION( 0x140000, "gfx1", 0 )
	ROM_LOAD( "136071-1001.55l", 0x000000, 0x020000, CRC(586ba107) SHA1(f15d4489f5834ea5fe695f43cb9d1c2401179870) )
	ROM_LOAD( "136071-1005.43l", 0x020000, 0x020000, CRC(a53e6248) SHA1(4f2466c6af74a5498468801b1de7adfc34873d5d) )
	ROM_LOAD( "136071-1032.18a", 0x040000, 0x010000, CRC(131f52a0) SHA1(fa50ea82d26c36dd6a135e22dee509676d1dfe86) )
	ROM_LOAD( "136071-1002.55n", 0x050000, 0x020000, CRC(0f71f86c) SHA1(783f33ba5cc1b2f0c42b8515b1cf8b6a2270acb9) )
	ROM_LOAD( "136071-1006.43n", 0x070000, 0x020000, CRC(df0ab373) SHA1(3d511236eb55a773c66643158c6ef2c4dce53b68) )
	ROM_LOAD( "136071-1033.13a", 0x090000, 0x010000, CRC(b6270943) SHA1(356e58dfd30c6db15264eceacef0eacda99aabae) )
	ROM_LOAD( "136071-1003.90l", 0x0a0000, 0x020000, CRC(1cf373a2) SHA1(c8538855bb82fc03e26c64fc9008fbd0c66fac09) )
	ROM_LOAD( "136071-1007.78l", 0x0c0000, 0x020000, CRC(f2ffab24) SHA1(6c5c90a9d9b342414a0d6258dd27b0b84bf0af0b) )
	ROM_LOAD( "136071-1034.18c", 0x0e0000, 0x010000, CRC(6514f0bd) SHA1(e887dfb9e0334a7d94e7124cea6101d9ac7f0ab6) )
	ROM_LOAD( "136071-1004.90n", 0x0f0000, 0x020000, CRC(537f6de3) SHA1(d5d385c3ff07aaef7bd3bd4f6c8066948a45ce9c) )
	ROM_LOAD( "136071-1008.78n", 0x110000, 0x020000, CRC(78525bbb) SHA1(98ece6c0672cb60f818b8005c76cc4ae1d24b104) )
	ROM_LOAD( "136071-1035.13c", 0x130000, 0x010000, CRC(1be3e5c8) SHA1(1a4d0e0d53b902c28977c8598e363c7b61c9c1c8) )

	ROM_REGION( 0x040000, "gfx2", 0 )
	ROM_LOAD( "136071-1013.10k", 0x000000, 0x010000, CRC(cdf6e3d6) SHA1(476644065b82e2ea553dfb51844c0bbf3313c481) )
	ROM_LOAD( "136071-1014.16f", 0x010000, 0x010000, CRC(ec2fef3e) SHA1(07ed472fa1723ebf82d608667df70511a50dca3e) )
	ROM_LOAD( "136071-1015.16k", 0x020000, 0x010000, CRC(e866848f) SHA1(35b438dcc1947151a7aafe919b9adf33d7a8fb77) )
	ROM_LOAD( "136071-1016.10f", 0x030000, 0x010000, CRC(9b9a393c) SHA1(db4ceb33756bab0ac7bea30e6c7345d06a0f4aa2) )

	ROM_REGION( 0x020000, "gfx3", 0 )
	ROM_LOAD( "136071-1017.32n", 0x000000, 0x010000, CRC(a4c116f9) SHA1(fc7becef35306ef99ffbd0cd9202759352eb6cbe) )
	ROM_LOAD( "136071-1018.32l", 0x010000, 0x010000, CRC(e25d7847) SHA1(3821c62f9bdc04eb774c2210a84e26b36f2e163d) )

	ROM_REGION( 0x40000, "jsa:oki1", 0 )   /* 256k for ADPCM samples */
	ROM_LOAD( "136071-1043.7k",  0x000000, 0x010000, CRC(94f24575) SHA1(b93b326e15cd328362ce409b7c0cc42b8a28c701) )
	ROM_LOAD( "136071-1044.7j",  0x010000, 0x010000, CRC(87208e1e) SHA1(3647867ddc36df7633ed740c0b9365a979ef5621) )
	ROM_LOAD( "136071-1045.7e",  0x020000, 0x010000, CRC(f82558b9) SHA1(afbecccc6203db9bdcf60638e0f4e95040d7aaf2) )
	ROM_LOAD( "136071-1046.7d",  0x030000, 0x010000, CRC(d96437ad) SHA1(b0b5cd75de4048e54b9d7d09a75381eb73c22ee1) )

	ROM_REGION( 0x800, "eeprom:eeprom", 0 )
	ROM_LOAD( "cyberbal2p-eeprom.bin", 0x0000, 0x800, CRC(3753f0e2) SHA1(26feab263a4d2d1dfcdf62e1225e0596cc036e1d) )
ROM_END


ROM_START( cyberbal2p3 )
	ROM_REGION( 0x80000, "maincpu", 0 ) /* 8*64k for 68000 code */
	ROM_LOAD16_BYTE( "136071-3019.10c", 0x000000, 0x010000, CRC(029f8cb6) SHA1(528ab6357592313b41964102c14b90862c05f248) )
	ROM_LOAD16_BYTE( "136071-3020.10d", 0x000001, 0x010000, CRC(1871b344) SHA1(2b6f2f4760af0f0e1e0b6cb34017dcdca7635e60) )
	ROM_LOAD16_BYTE( "136071-3021.21c", 0x020000, 0x010000, CRC(fd7ebead) SHA1(4118c865897c7a9f73de200ea9766fe190547606) )
	ROM_LOAD16_BYTE( "136071-3022.21d", 0x020001, 0x010000, CRC(173ccad4) SHA1(4a8d2751b338dbb6dc556dfab13799561fee4836) )
	ROM_LOAD16_BYTE( "136071-2023.15c", 0x040000, 0x010000, CRC(e541b08f) SHA1(345843209b02fb50cb08f55f80036046873b834f) )
	ROM_LOAD16_BYTE( "136071-2024.15d", 0x040001, 0x010000, CRC(5a77ee95) SHA1(441a45a358eb78cc140c96dc4c42b30e1929ad07) )
	ROM_LOAD16_BYTE( "136071-1025.27c", 0x060000, 0x010000, CRC(95ff68c6) SHA1(43f716a4c44fe1a38fcc6e2600bac948bb603504) )
	ROM_LOAD16_BYTE( "136071-1026.27d", 0x060001, 0x010000, CRC(f61c4898) SHA1(9e4a14eac6d197f63c3392af3d804e81c034cb09) )

	ROM_REGION( 0x14000, "jsa:cpu", 0 ) /* 64k for 6502 code */
	ROM_LOAD( "136071-1042.1b",  0x010000, 0x004000, CRC(e63cf125) SHA1(449880f561660ba67ac2d7f8ce6333768e0ae0be) )
	ROM_CONTINUE(                0x004000, 0x00c000 )

	ROM_REGION( 0x140000, "gfx1", 0 )
	ROM_LOAD( "136071-1001.55l", 0x000000, 0x020000, CRC(586ba107) SHA1(f15d4489f5834ea5fe695f43cb9d1c2401179870) )
	ROM_LOAD( "136071-1005.43l", 0x020000, 0x020000, CRC(a53e6248) SHA1(4f2466c6af74a5498468801b1de7adfc34873d5d) )
	ROM_LOAD( "136071-1032.18a", 0x040000, 0x010000, CRC(131f52a0) SHA1(fa50ea82d26c36dd6a135e22dee509676d1dfe86) )
	ROM_LOAD( "136071-1002.55n", 0x050000, 0x020000, CRC(0f71f86c) SHA1(783f33ba5cc1b2f0c42b8515b1cf8b6a2270acb9) )
	ROM_LOAD( "136071-1006.43n", 0x070000, 0x020000, CRC(df0ab373) SHA1(3d511236eb55a773c66643158c6ef2c4dce53b68) )
	ROM_LOAD( "136071-1033.13a", 0x090000, 0x010000, CRC(b6270943) SHA1(356e58dfd30c6db15264eceacef0eacda99aabae) )
	ROM_LOAD( "136071-1003.90l", 0x0a0000, 0x020000, CRC(1cf373a2) SHA1(c8538855bb82fc03e26c64fc9008fbd0c66fac09) )
	ROM_LOAD( "136071-1007.78l", 0x0c0000, 0x020000, CRC(f2ffab24) SHA1(6c5c90a9d9b342414a0d6258dd27b0b84bf0af0b) )
	ROM_LOAD( "136071-1034.18c", 0x0e0000, 0x010000, CRC(6514f0bd) SHA1(e887dfb9e0334a7d94e7124cea6101d9ac7f0ab6) )
	ROM_LOAD( "136071-1004.90n", 0x0f0000, 0x020000, CRC(537f6de3) SHA1(d5d385c3ff07aaef7bd3bd4f6c8066948a45ce9c) )
	ROM_LOAD( "136071-1008.78n", 0x110000, 0x020000, CRC(78525bbb) SHA1(98ece6c0672cb60f818b8005c76cc4ae1d24b104) )
	ROM_LOAD( "136071-1035.13c", 0x130000, 0x010000, CRC(1be3e5c8) SHA1(1a4d0e0d53b902c28977c8598e363c7b61c9c1c8) )

	ROM_REGION( 0x040000, "gfx2", 0 )
	ROM_LOAD( "136071-1013.10k", 0x000000, 0x010000, CRC(cdf6e3d6) SHA1(476644065b82e2ea553dfb51844c0bbf3313c481) )
	ROM_LOAD( "136071-1014.16f", 0x010000, 0x010000, CRC(ec2fef3e) SHA1(07ed472fa1723ebf82d608667df70511a50dca3e) )
	ROM_LOAD( "136071-1015.16k", 0x020000, 0x010000, CRC(e866848f) SHA1(35b438dcc1947151a7aafe919b9adf33d7a8fb77) )
	ROM_LOAD( "136071-1016.10f", 0x030000, 0x010000, CRC(9b9a393c) SHA1(db4ceb33756bab0ac7bea30e6c7345d06a0f4aa2) )

	ROM_REGION( 0x020000, "gfx3", 0 )
	ROM_LOAD( "136071-1017.32n", 0x000000, 0x010000, CRC(a4c116f9) SHA1(fc7becef35306ef99ffbd0cd9202759352eb6cbe) )
	ROM_LOAD( "136071-1018.32l", 0x010000, 0x010000, CRC(e25d7847) SHA1(3821c62f9bdc04eb774c2210a84e26b36f2e163d) )

	ROM_REGION( 0x40000, "jsa:oki1", 0 )   /* 256k for ADPCM samples */
	ROM_LOAD( "136071-1043.7k",  0x000000, 0x010000, CRC(94f24575) SHA1(b93b326e15cd328362ce409b7c0cc42b8a28c701) )
	ROM_LOAD( "136071-1044.7j",  0x010000, 0x010000, CRC(87208e1e) SHA1(3647867ddc36df7633ed740c0b9365a979ef5621) )
	ROM_LOAD( "136071-1045.7e",  0x020000, 0x010000, CRC(f82558b9) SHA1(afbecccc6203db9bdcf60638e0f4e95040d7aaf2) )
	ROM_LOAD( "136071-1046.7d",  0x030000, 0x010000, CRC(d96437ad) SHA1(b0b5cd75de4048e54b9d7d09a75381eb73c22ee1) )

	ROM_REGION( 0x800, "eeprom:eeprom", 0 )
	ROM_LOAD( "cyberbal2p-eeprom.bin", 0x0000, 0x800, CRC(3753f0e2) SHA1(26feab263a4d2d1dfcdf62e1225e0596cc036e1d) )
ROM_END


ROM_START( cyberbal2p2 )
	ROM_REGION( 0x80000, "maincpu", 0 ) /* 8*64k for 68000 code */
	ROM_LOAD16_BYTE( "136071-2019.10c", 0x000000, 0x010000, CRC(acb1e18b) SHA1(37a80f0c8d8b46ebb9660c7f038fc874d4817e93) )
	ROM_LOAD16_BYTE( "136071-2020.10d", 0x000001, 0x010000, CRC(fd6ec2fd) SHA1(8b871e5e66acd26b8301ac35e3f42fba5b9fce2c) )
	ROM_LOAD16_BYTE( "136071-1021.21c", 0x020000, 0x010000, CRC(9b6cecc3) SHA1(14291302ca6a35fc4c24d9f7d6a4ef7c88a95e5a) )
	ROM_LOAD16_BYTE( "136071-1022.21d", 0x020001, 0x010000, CRC(24949cfa) SHA1(283c1e2baf6774ebb7f4674b112bdb703fed5aa5) )
	ROM_LOAD16_BYTE( "136071-2023.15c", 0x040000, 0x010000, CRC(e541b08f) SHA1(345843209b02fb50cb08f55f80036046873b834f) )
	ROM_LOAD16_BYTE( "136071-2024.15d", 0x040001, 0x010000, CRC(5a77ee95) SHA1(441a45a358eb78cc140c96dc4c42b30e1929ad07) )
	ROM_LOAD16_BYTE( "136071-1025.27c", 0x060000, 0x010000, CRC(95ff68c6) SHA1(43f716a4c44fe1a38fcc6e2600bac948bb603504) )
	ROM_LOAD16_BYTE( "136071-1026.27d", 0x060001, 0x010000, CRC(f61c4898) SHA1(9e4a14eac6d197f63c3392af3d804e81c034cb09) )

	ROM_REGION( 0x14000, "jsa:cpu", 0 ) /* 64k for 6502 code */
	ROM_LOAD( "136071-1042.1b",  0x010000, 0x004000, CRC(e63cf125) SHA1(449880f561660ba67ac2d7f8ce6333768e0ae0be) )
	ROM_CONTINUE(             0x004000, 0x00c000 )

	ROM_REGION( 0x140000, "gfx1", 0 )
	ROM_LOAD( "136071-1001.55l", 0x000000, 0x020000, CRC(586ba107) SHA1(f15d4489f5834ea5fe695f43cb9d1c2401179870) )
	ROM_LOAD( "136071-1005.43l", 0x020000, 0x020000, CRC(a53e6248) SHA1(4f2466c6af74a5498468801b1de7adfc34873d5d) )
	ROM_LOAD( "136071-1032.18a", 0x040000, 0x010000, CRC(131f52a0) SHA1(fa50ea82d26c36dd6a135e22dee509676d1dfe86) )
	ROM_LOAD( "136071-1002.55n", 0x050000, 0x020000, CRC(0f71f86c) SHA1(783f33ba5cc1b2f0c42b8515b1cf8b6a2270acb9) )
	ROM_LOAD( "136071-1006.43n", 0x070000, 0x020000, CRC(df0ab373) SHA1(3d511236eb55a773c66643158c6ef2c4dce53b68) )
	ROM_LOAD( "136071-1033.13a", 0x090000, 0x010000, CRC(b6270943) SHA1(356e58dfd30c6db15264eceacef0eacda99aabae) )
	ROM_LOAD( "136071-1003.90l", 0x0a0000, 0x020000, CRC(1cf373a2) SHA1(c8538855bb82fc03e26c64fc9008fbd0c66fac09) )
	ROM_LOAD( "136071-1007.78l", 0x0c0000, 0x020000, CRC(f2ffab24) SHA1(6c5c90a9d9b342414a0d6258dd27b0b84bf0af0b) )
	ROM_LOAD( "136071-1034.18c", 0x0e0000, 0x010000, CRC(6514f0bd) SHA1(e887dfb9e0334a7d94e7124cea6101d9ac7f0ab6) )
	ROM_LOAD( "136071-1004.90n", 0x0f0000, 0x020000, CRC(537f6de3) SHA1(d5d385c3ff07aaef7bd3bd4f6c8066948a45ce9c) )
	ROM_LOAD( "136071-1008.78n", 0x110000, 0x020000, CRC(78525bbb) SHA1(98ece6c0672cb60f818b8005c76cc4ae1d24b104) )
	ROM_LOAD( "136071-1035.13c", 0x130000, 0x010000, CRC(1be3e5c8) SHA1(1a4d0e0d53b902c28977c8598e363c7b61c9c1c8) )

	ROM_REGION( 0x040000, "gfx2", 0 )
	ROM_LOAD( "136071-1013.10k", 0x000000, 0x010000, CRC(cdf6e3d6) SHA1(476644065b82e2ea553dfb51844c0bbf3313c481) )
	ROM_LOAD( "136071-1014.16f", 0x010000, 0x010000, CRC(ec2fef3e) SHA1(07ed472fa1723ebf82d608667df70511a50dca3e) )
	ROM_LOAD( "136071-1015.16k", 0x020000, 0x010000, CRC(e866848f) SHA1(35b438dcc1947151a7aafe919b9adf33d7a8fb77) )
	ROM_LOAD( "136071-1016.10f", 0x030000, 0x010000, CRC(9b9a393c) SHA1(db4ceb33756bab0ac7bea30e6c7345d06a0f4aa2) )

	ROM_REGION( 0x020000, "gfx3", 0 )
	ROM_LOAD( "136071-1017.32n", 0x000000, 0x010000, CRC(a4c116f9) SHA1(fc7becef35306ef99ffbd0cd9202759352eb6cbe) )
	ROM_LOAD( "136071-1018.32l", 0x010000, 0x010000, CRC(e25d7847) SHA1(3821c62f9bdc04eb774c2210a84e26b36f2e163d) )

	ROM_REGION( 0x40000, "jsa:oki1", 0 )   /* 256k for ADPCM samples */
	ROM_LOAD( "136071-1043.7k",  0x000000, 0x010000, CRC(94f24575) SHA1(b93b326e15cd328362ce409b7c0cc42b8a28c701) )
	ROM_LOAD( "136071-1044.7j",  0x010000, 0x010000, CRC(87208e1e) SHA1(3647867ddc36df7633ed740c0b9365a979ef5621) )
	ROM_LOAD( "136071-1045.7e",  0x020000, 0x010000, CRC(f82558b9) SHA1(afbecccc6203db9bdcf60638e0f4e95040d7aaf2) )
	ROM_LOAD( "136071-1046.7d",  0x030000, 0x010000, CRC(d96437ad) SHA1(b0b5cd75de4048e54b9d7d09a75381eb73c22ee1) )

	ROM_REGION( 0x800, "eeprom:eeprom", 0 )
	ROM_LOAD( "cyberbal2p-eeprom.bin", 0x0000, 0x800, CRC(3753f0e2) SHA1(26feab263a4d2d1dfcdf62e1225e0596cc036e1d) )
ROM_END


ROM_START( cyberbal2p1 )
	ROM_REGION( 0x80000, "maincpu", 0 ) /* 8*64k for 68000 code */
	ROM_LOAD16_BYTE( "136071-1019.10c", 0x000000, 0x010000, CRC(37f5f8ba) SHA1(80552b41d7d1c3044ccd1cbfbac6051447915d41) )
	ROM_LOAD16_BYTE( "136071-1020.10d", 0x000001, 0x010000, CRC(cae4faa2) SHA1(e3282416b1d0dccd52dd8763a02647470dd37114) )
	ROM_LOAD16_BYTE( "136071-1021.21c", 0x020000, 0x010000, CRC(9b6cecc3) SHA1(14291302ca6a35fc4c24d9f7d6a4ef7c88a95e5a) )
	ROM_LOAD16_BYTE( "136071-1022.21d", 0x020001, 0x010000, CRC(24949cfa) SHA1(283c1e2baf6774ebb7f4674b112bdb703fed5aa5) )
	ROM_LOAD16_BYTE( "136071-1023.15c", 0x040000, 0x010000, CRC(4043649d) SHA1(7fab7f8cc41fb923368db2f9b14e59ae12aa58e1) )
	ROM_LOAD16_BYTE( "136071-1024.15d", 0x040001, 0x010000, CRC(ed052dd6) SHA1(3755ca476865f7218dc1c837433cfd6068a61797) )
	ROM_LOAD16_BYTE( "136071-1025.27c", 0x060000, 0x010000, CRC(95ff68c6) SHA1(43f716a4c44fe1a38fcc6e2600bac948bb603504) )
	ROM_LOAD16_BYTE( "136071-1026.27d", 0x060001, 0x010000, CRC(f61c4898) SHA1(9e4a14eac6d197f63c3392af3d804e81c034cb09) )

	ROM_REGION( 0x14000, "jsa:cpu", 0 ) /* 64k for 6502 code */
	ROM_LOAD( "136071-1042.1b",  0x010000, 0x004000, CRC(e63cf125) SHA1(449880f561660ba67ac2d7f8ce6333768e0ae0be) )
	ROM_CONTINUE(                0x004000, 0x00c000 )

	ROM_REGION( 0x140000, "gfx1", 0 )
	ROM_LOAD( "136071-1001.55l", 0x000000, 0x020000, CRC(586ba107) SHA1(f15d4489f5834ea5fe695f43cb9d1c2401179870) )
	ROM_LOAD( "136071-1005.43l", 0x020000, 0x020000, CRC(a53e6248) SHA1(4f2466c6af74a5498468801b1de7adfc34873d5d) )
	ROM_LOAD( "136071-1032.18a", 0x040000, 0x010000, CRC(131f52a0) SHA1(fa50ea82d26c36dd6a135e22dee509676d1dfe86) )
	ROM_LOAD( "136071-1002.55n", 0x050000, 0x020000, CRC(0f71f86c) SHA1(783f33ba5cc1b2f0c42b8515b1cf8b6a2270acb9) )
	ROM_LOAD( "136071-1006.43n", 0x070000, 0x020000, CRC(df0ab373) SHA1(3d511236eb55a773c66643158c6ef2c4dce53b68) )
	ROM_LOAD( "136071-1033.13a", 0x090000, 0x010000, CRC(b6270943) SHA1(356e58dfd30c6db15264eceacef0eacda99aabae) )
	ROM_LOAD( "136071-1003.90l", 0x0a0000, 0x020000, CRC(1cf373a2) SHA1(c8538855bb82fc03e26c64fc9008fbd0c66fac09) )
	ROM_LOAD( "136071-1007.78l", 0x0c0000, 0x020000, CRC(f2ffab24) SHA1(6c5c90a9d9b342414a0d6258dd27b0b84bf0af0b) )
	ROM_LOAD( "136071-1034.18c", 0x0e0000, 0x010000, CRC(6514f0bd) SHA1(e887dfb9e0334a7d94e7124cea6101d9ac7f0ab6) )
	ROM_LOAD( "136071-1004.90n", 0x0f0000, 0x020000, CRC(537f6de3) SHA1(d5d385c3ff07aaef7bd3bd4f6c8066948a45ce9c) )
	ROM_LOAD( "136071-1008.78n", 0x110000, 0x020000, CRC(78525bbb) SHA1(98ece6c0672cb60f818b8005c76cc4ae1d24b104) )
	ROM_LOAD( "136071-1035.13c", 0x130000, 0x010000, CRC(1be3e5c8) SHA1(1a4d0e0d53b902c28977c8598e363c7b61c9c1c8) )

	ROM_REGION( 0x040000, "gfx2", 0 )
	ROM_LOAD( "136071-1013.10k", 0x000000, 0x010000, CRC(cdf6e3d6) SHA1(476644065b82e2ea553dfb51844c0bbf3313c481) )
	ROM_LOAD( "136071-1014.16f", 0x010000, 0x010000, CRC(ec2fef3e) SHA1(07ed472fa1723ebf82d608667df70511a50dca3e) )
	ROM_LOAD( "136071-1015.16k", 0x020000, 0x010000, CRC(e866848f) SHA1(35b438dcc1947151a7aafe919b9adf33d7a8fb77) )
	ROM_LOAD( "136071-1016.10f", 0x030000, 0x010000, CRC(9b9a393c) SHA1(db4ceb33756bab0ac7bea30e6c7345d06a0f4aa2) )

	ROM_REGION( 0x020000, "gfx3", 0 )
	ROM_LOAD( "136071-1017.32n", 0x000000, 0x010000, CRC(a4c116f9) SHA1(fc7becef35306ef99ffbd0cd9202759352eb6cbe) )
	ROM_LOAD( "136071-1018.32l", 0x010000, 0x010000, CRC(e25d7847) SHA1(3821c62f9bdc04eb774c2210a84e26b36f2e163d) )

	ROM_REGION( 0x40000, "jsa:oki1", 0 )   /* 256k for ADPCM samples */
	ROM_LOAD( "136071-1043.7k",  0x000000, 0x010000, CRC(94f24575) SHA1(b93b326e15cd328362ce409b7c0cc42b8a28c701) )
	ROM_LOAD( "136071-1044.7j",  0x010000, 0x010000, CRC(87208e1e) SHA1(3647867ddc36df7633ed740c0b9365a979ef5621) )
	ROM_LOAD( "136071-1045.7e",  0x020000, 0x010000, CRC(f82558b9) SHA1(afbecccc6203db9bdcf60638e0f4e95040d7aaf2) )
	ROM_LOAD( "136071-1046.7d",  0x030000, 0x010000, CRC(d96437ad) SHA1(b0b5cd75de4048e54b9d7d09a75381eb73c22ee1) )

	ROM_REGION( 0x800, "eeprom:eeprom", 0 )
	ROM_LOAD( "cyberbal2p-eeprom.bin", 0x0000, 0x800, CRC(3753f0e2) SHA1(26feab263a4d2d1dfcdf62e1225e0596cc036e1d) )
ROM_END


ROM_START( cyberbalt )
	ROM_REGION( 0x40000, "maincpu", 0 ) /* 4*64k for 68000 code */
	ROM_LOAD16_BYTE( "136073-2007.1m",  0x000000, 0x010000, CRC(adfa9e23) SHA1(5462030fa275ce7b261b7283e92df9f7f1383251) )
	ROM_LOAD16_BYTE( "136073-2008.1kl", 0x000001, 0x010000, CRC(c9191452) SHA1(583c1f916fbd54dbc188be7a181ccd60c7320cc8) )
	ROM_LOAD16_BYTE( "136073-2009.3m",  0x020000, 0x010000, CRC(88bfc6dd) SHA1(ac2a67c8b4dbae62497236d624d333992195c218) )
	ROM_LOAD16_BYTE( "136073-2010.3kl", 0x020001, 0x010000, CRC(3a121f29) SHA1(ebd088187abb863f2a632812811479dca7e31802) )

	ROM_REGION( 0x14000, "audiocpu", 0 )    /* 64k for 6502 code */
	ROM_LOAD( "136073-1029.2f",  0x010000, 0x004000, CRC(afee87e1) SHA1(da5e91167c68eecd2cb4436ac64cda14e5f6eae7) )
	ROM_CONTINUE(                0x004000, 0x00c000 )

	ROM_REGION( 0x40000, "extra", 0 )
	ROM_LOAD16_BYTE( "136073-2011.3cd", 0x000000, 0x010000, CRC(bb5c5926) SHA1(6f7424418559841053d0c874ac8bb72c793d0d62) )
	ROM_LOAD16_BYTE( "136073-2012.1b",  0x000001, 0x010000, CRC(a045f5f3) SHA1(c74f62ee0b3640caea818dca2deede83cd39e76a) )
	ROM_LOAD16_BYTE( "136073-1013.1cd", 0x020000, 0x010000, CRC(11d287c9) SHA1(a25095ab29a7103f2bf02d656414d9dab0b79215) )
	ROM_LOAD16_BYTE( "136073-1014.3b",  0x020001, 0x010000, CRC(be15db42) SHA1(f3b1a676106e9956f62d3f36fbb1f849695ff771) )

	ROM_REGION16_BE( 0x40000, "dac", 0 )    /* 256k for 68000 sound code */
	ROM_LOAD16_BYTE( "136064-1132.3cd", 0x000000, 0x010000, CRC(ca5ce8d8) SHA1(69dc83d43d8c9dc7ce3207e70f48fcfc5ddda0cc) )
	ROM_LOAD16_BYTE( "136064-1133.1b",  0x000001, 0x010000, CRC(ffeb8746) SHA1(0d8d28b2d997ff3cf01b4ef25b75fa5a69754af4) )
	ROM_LOAD16_BYTE( "136064-1134.1cd", 0x020000, 0x010000, CRC(bcbd4c00) SHA1(f0bfcdf0b5491e15872b543e99b834ae384cbf18) )
	ROM_LOAD16_BYTE( "136064-1135.3b",  0x020001, 0x010000, CRC(d520f560) SHA1(fb0b8d021458379188c424a343622c46ad74edaa) )

	ROM_REGION( 0x140000, "gfx1", 0 )
	ROM_LOAD( "136071-1001.55l", 0x000000, 0x020000, CRC(586ba107) SHA1(f15d4489f5834ea5fe695f43cb9d1c2401179870) )
	ROM_LOAD( "136071-1005.43l", 0x020000, 0x020000, CRC(a53e6248) SHA1(4f2466c6af74a5498468801b1de7adfc34873d5d) )
	ROM_LOAD( "136071-1032.18a", 0x040000, 0x010000, CRC(131f52a0) SHA1(fa50ea82d26c36dd6a135e22dee509676d1dfe86) )
	ROM_LOAD( "136071-1002.55n", 0x050000, 0x020000, CRC(0f71f86c) SHA1(783f33ba5cc1b2f0c42b8515b1cf8b6a2270acb9) )
	ROM_LOAD( "136071-1006.43n", 0x070000, 0x020000, CRC(df0ab373) SHA1(3d511236eb55a773c66643158c6ef2c4dce53b68) )
	ROM_LOAD( "136071-1033.13a", 0x090000, 0x010000, CRC(b6270943) SHA1(356e58dfd30c6db15264eceacef0eacda99aabae) )
	ROM_LOAD( "136071-1003.90l", 0x0a0000, 0x020000, CRC(1cf373a2) SHA1(c8538855bb82fc03e26c64fc9008fbd0c66fac09) )
	ROM_LOAD( "136071-1007.78l", 0x0c0000, 0x020000, CRC(f2ffab24) SHA1(6c5c90a9d9b342414a0d6258dd27b0b84bf0af0b) )
	ROM_LOAD( "136071-1034.18c", 0x0e0000, 0x010000, CRC(6514f0bd) SHA1(e887dfb9e0334a7d94e7124cea6101d9ac7f0ab6) )
	ROM_LOAD( "136071-1004.90n", 0x0f0000, 0x020000, CRC(537f6de3) SHA1(d5d385c3ff07aaef7bd3bd4f6c8066948a45ce9c) )
	ROM_LOAD( "136071-1008.78n", 0x110000, 0x020000, CRC(78525bbb) SHA1(98ece6c0672cb60f818b8005c76cc4ae1d24b104) )
	ROM_LOAD( "136071-1035.13c", 0x130000, 0x010000, CRC(1be3e5c8) SHA1(1a4d0e0d53b902c28977c8598e363c7b61c9c1c8) )

	ROM_REGION( 0x040000, "gfx2", 0 )
	ROM_LOAD( "136073-1001.9lm", 0x000000, 0x010000, CRC(dbbad153) SHA1(1004292e320037fc1d5e5e8e7b6a068b1305e872) )
	ROM_LOAD( "136073-1002.8lm", 0x010000, 0x010000, CRC(76e0d008) SHA1(2af4e48a229d23d85272d3c3203d977d81143a7f) )
	ROM_LOAD( "136073-1003.11lm", 0x020000, 0x010000, CRC(ddca9ca2) SHA1(19cb170fe6aeed6c67b68376b5bde07f7f115fb0) )
	ROM_LOAD( "136073-1004.10lm", 0x030000, 0x010000, CRC(aa495b6f) SHA1(c7d8e16d3084143928f25f66ee4d037ff7c43bcb) )

	ROM_REGION( 0x020000, "gfx3", 0 )
	ROM_LOAD( "136073-1005.15n", 0x000000, 0x010000, CRC(833b4768) SHA1(754f00089d439fb0aa1f650c1fef73cf7e5f33a1) )
	ROM_LOAD( "136073-1006.16n", 0x010000, 0x010000, CRC(4976cffd) SHA1(4cac8d9bd30743da6e6e4f013e6101ebc27060b6) )

	ROM_REGION( 0x800, "eeprom:eeprom", 0 )
	ROM_LOAD( "cyberbalt-eeprom.bin", 0x0000, 0x800, CRC(0743c0a6) SHA1(0b7421484f640b528e96aed103775e81bbb60f62) )
ROM_END


ROM_START( cyberbalt1 )
	ROM_REGION( 0x40000, "maincpu", 0 ) /* 4*64k for 68000 code */
	ROM_LOAD16_BYTE( "136073-1007.1m",  0x000000, 0x010000, CRC(d434b2d7) SHA1(af6d51399ad4fca01ffbc7afa2bf73d7ee2f89b6) )
	ROM_LOAD16_BYTE( "136073-1008.1kl", 0x000001, 0x010000, CRC(7d6c4163) SHA1(f1fe9d758f30bd0ebc990d8604ba32cc0d780683) )
	ROM_LOAD16_BYTE( "136073-1009.3m",  0x020000, 0x010000, CRC(3933e089) SHA1(4bd453bddabeafd07d193a1bc8ac0792e7aa99c3) )
	ROM_LOAD16_BYTE( "136073-1010.3kl", 0x020001, 0x010000, CRC(e7a7cae8) SHA1(91e0c6a1b0c138a0e6a599011518fe10df44e76e) )

	ROM_REGION( 0x14000, "audiocpu", 0 )    /* 64k for 6502 code */
	ROM_LOAD( "136073-1029.2f",  0x010000, 0x004000, CRC(afee87e1) SHA1(da5e91167c68eecd2cb4436ac64cda14e5f6eae7) )
	ROM_CONTINUE(                0x004000, 0x00c000 )

	ROM_REGION( 0x40000, "extra", 0 )
	ROM_LOAD16_BYTE( "136073-1011.3cd", 0x000000, 0x010000, CRC(22d3e09c) SHA1(18298951659badef39f839341c4d66958fcc86aa) )
	ROM_LOAD16_BYTE( "136073-1012.1b",  0x000001, 0x010000, CRC(a8eeed8c) SHA1(965765e5762ec09573243983db491a9fc85b37ef) )
	ROM_LOAD16_BYTE( "136073-1013.1cd", 0x020000, 0x010000, CRC(11d287c9) SHA1(a25095ab29a7103f2bf02d656414d9dab0b79215) )
	ROM_LOAD16_BYTE( "136073-1014.3b",  0x020001, 0x010000, CRC(be15db42) SHA1(f3b1a676106e9956f62d3f36fbb1f849695ff771) )

	ROM_REGION16_BE( 0x40000, "dac", 0 )    /* 256k for 68000 sound code */
	ROM_LOAD16_BYTE( "136064-1132.3cd", 0x000000, 0x010000, CRC(ca5ce8d8) SHA1(69dc83d43d8c9dc7ce3207e70f48fcfc5ddda0cc) )
	ROM_LOAD16_BYTE( "136064-1133.1b",  0x000001, 0x010000, CRC(ffeb8746) SHA1(0d8d28b2d997ff3cf01b4ef25b75fa5a69754af4) )
	ROM_LOAD16_BYTE( "136064-1134.1cd", 0x020000, 0x010000, CRC(bcbd4c00) SHA1(f0bfcdf0b5491e15872b543e99b834ae384cbf18) )
	ROM_LOAD16_BYTE( "136064-1135.3b",  0x020001, 0x010000, CRC(d520f560) SHA1(fb0b8d021458379188c424a343622c46ad74edaa) )

	ROM_REGION( 0x140000, "gfx1", 0 )
	ROM_LOAD( "136071-1001.55l", 0x000000, 0x020000, CRC(586ba107) SHA1(f15d4489f5834ea5fe695f43cb9d1c2401179870) )
	ROM_LOAD( "136071-1005.43l", 0x020000, 0x020000, CRC(a53e6248) SHA1(4f2466c6af74a5498468801b1de7adfc34873d5d) )
	ROM_LOAD( "136071-1032.18a", 0x040000, 0x010000, CRC(131f52a0) SHA1(fa50ea82d26c36dd6a135e22dee509676d1dfe86) )
	ROM_LOAD( "136071-1002.55n", 0x050000, 0x020000, CRC(0f71f86c) SHA1(783f33ba5cc1b2f0c42b8515b1cf8b6a2270acb9) )
	ROM_LOAD( "136071-1006.43n", 0x070000, 0x020000, CRC(df0ab373) SHA1(3d511236eb55a773c66643158c6ef2c4dce53b68) )
	ROM_LOAD( "136071-1033.13a", 0x090000, 0x010000, CRC(b6270943) SHA1(356e58dfd30c6db15264eceacef0eacda99aabae) )
	ROM_LOAD( "136071-1003.90l", 0x0a0000, 0x020000, CRC(1cf373a2) SHA1(c8538855bb82fc03e26c64fc9008fbd0c66fac09) )
	ROM_LOAD( "136071-1007.78l", 0x0c0000, 0x020000, CRC(f2ffab24) SHA1(6c5c90a9d9b342414a0d6258dd27b0b84bf0af0b) )
	ROM_LOAD( "136071-1034.18c", 0x0e0000, 0x010000, CRC(6514f0bd) SHA1(e887dfb9e0334a7d94e7124cea6101d9ac7f0ab6) )
	ROM_LOAD( "136071-1004.90n", 0x0f0000, 0x020000, CRC(537f6de3) SHA1(d5d385c3ff07aaef7bd3bd4f6c8066948a45ce9c) )
	ROM_LOAD( "136071-1008.78n", 0x110000, 0x020000, CRC(78525bbb) SHA1(98ece6c0672cb60f818b8005c76cc4ae1d24b104) )
	ROM_LOAD( "136071-1035.13c", 0x130000, 0x010000, CRC(1be3e5c8) SHA1(1a4d0e0d53b902c28977c8598e363c7b61c9c1c8) )

	ROM_REGION( 0x040000, "gfx2", 0 )
	ROM_LOAD( "136073-1001.9lm", 0x000000, 0x010000, CRC(dbbad153) SHA1(1004292e320037fc1d5e5e8e7b6a068b1305e872) )
	ROM_LOAD( "136073-1002.8lm", 0x010000, 0x010000, CRC(76e0d008) SHA1(2af4e48a229d23d85272d3c3203d977d81143a7f) )
	ROM_LOAD( "136073-1003.11lm", 0x020000, 0x010000, CRC(ddca9ca2) SHA1(19cb170fe6aeed6c67b68376b5bde07f7f115fb0) )
	ROM_LOAD( "136073-1004.10lm", 0x030000, 0x010000, CRC(aa495b6f) SHA1(c7d8e16d3084143928f25f66ee4d037ff7c43bcb) )

	ROM_REGION( 0x020000, "gfx3", 0 )
	ROM_LOAD( "136073-1005.15n", 0x000000, 0x010000, CRC(833b4768) SHA1(754f00089d439fb0aa1f650c1fef73cf7e5f33a1) )
	ROM_LOAD( "136073-1006.16n", 0x010000, 0x010000, CRC(4976cffd) SHA1(4cac8d9bd30743da6e6e4f013e6101ebc27060b6) )

	ROM_REGION( 0x800, "eeprom:eeprom", 0 )
	ROM_LOAD( "cyberbalt-eeprom.bin", 0x0000, 0x800, CRC(0743c0a6) SHA1(0b7421484f640b528e96aed103775e81bbb60f62) )
ROM_END



/*************************************
 *
 *  Machine initialization
 *
 *************************************/

DRIVER_INIT_MEMBER(cyberbal_state,cyberbal)
{
	slapstic_configure(*m_maincpu, 0x018000, 0, 0);
}


DRIVER_INIT_MEMBER(cyberbal_state,cyberbalt)
{
	slapstic_configure(*m_maincpu, 0x018000, 0, 116);
}


DRIVER_INIT_MEMBER(cyberbal_state,cyberbal2p)
{
}



/*************************************
 *
 *  Game driver(s)
 *
 *************************************/

GAMEL(1988, cyberbal,    0,        cyberbal,   cyberbal, cyberbal_state, cyberbal, ROT0, "Atari Games", "Cyberball (rev 4)", 0, layout_dualhsxs )
GAMEL(1988, cyberbal2,   cyberbal, cyberbal,   cyberbal, cyberbal_state, cyberbal, ROT0, "Atari Games", "Cyberball (rev 2)", 0, layout_dualhsxs )
GAMEL(1988, cyberbalp,   cyberbal, cyberbal,   cyberbal, cyberbal_state, cyberbal, ROT0, "Atari Games", "Cyberball (prototype)", 0, layout_dualhsxs )

GAME( 1989, cyberbal2p,  cyberbal, cyberbal2p, cyberbal2p, cyberbal_state, cyberbal2p, ROT0, "Atari Games", "Cyberball 2072 (2 player, rev 4)", 0 )
GAME( 1989, cyberbal2p3, cyberbal, cyberbal2p, cyberbal2p, cyberbal_state, cyberbal2p, ROT0, "Atari Games", "Cyberball 2072 (2 player, rev 3)", 0 )
GAME( 1989, cyberbal2p2, cyberbal, cyberbal2p, cyberbal2p, cyberbal_state, cyberbal2p, ROT0, "Atari Games", "Cyberball 2072 (2 player, rev 2)", 0 )
GAME( 1989, cyberbal2p1, cyberbal, cyberbal2p, cyberbal2p, cyberbal_state, cyberbal2p, ROT0, "Atari Games", "Cyberball 2072 (2 player, rev 1)", 0 )

GAMEL(1989, cyberbalt,   cyberbal, cyberbalt,  cyberbal, cyberbal_state, cyberbalt,  ROT0, "Atari Games", "Tournament Cyberball 2072 (rev 2)", 0, layout_dualhsxs )
GAMEL(1989, cyberbalt1,  cyberbal, cyberbalt,  cyberbal, cyberbal_state, cyberbalt,  ROT0, "Atari Games", "Tournament Cyberball 2072 (rev 1)", 0, layout_dualhsxs )
