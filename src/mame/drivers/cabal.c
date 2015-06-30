// license:BSD-3-Clause
// copyright-holders:Carlos A. Lozano
/******************************************************************

Cabal  (c)1998 Tad

driver by Carlos A. Lozano Baides

68000 + Z80

The original uses 2xYM3931 for sound
The bootleg uses YM2151 + 2xZ80 used as ADPCM players


MEMORY MAP
0x00000 - 0x3ffff   ROM
0x40000 - 0x4ffff   RAM
[of which: 0x43800 - 0x43fff   VRAM (Sprites)]
0x60000 - 0x607ff   VRAM (Tiles)
0x80000 - 0x803ff   VRAM (Background)
0xa0000 - 0xa000f   Input Ports
0xc0040 - 0xc0040   Watchdog??
0xc0080 - 0xc0080   Screen Flip (+ others?)
0xe0000 - 0xe07ff   COLORRAM (----BBBBGGGGRRRR)
0xe8000 - 0xe800f   Communication with sound CPU (also coins)

VRAM (Background)
0x80000 - 0x801ff  (16x16 of 16x16 tiles, 2 bytes per tile)
0x80200 - 0x803ff  unused foreground layer??

VRAM (Text)
0x60000 - 0x607ff  (32x32 of 8x8 tiles, 2 bytes per tile)

VRAM (Sprites)
0x43800 - 0x43bff  (128 sprites, 8 bytes every sprite)

COLORRAM (Colors)
0xe0000 - 0xe07ff  (1024 colors, ----BBBBGGGGRRRR)


2008-07
Dip locations verified with Fabtek manual for the trackball version

******************************************************************/

#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "cpu/z80/z80.h"
#include "sound/2151intf.h"
#include "sound/msm5205.h"
#include "includes/cabal.h"

MACHINE_START_MEMBER(cabal_state,cabal)
{
	save_item(NAME(m_last));
}

MACHINE_START_MEMBER(cabal_state,cabalbl)
{
	save_item(NAME(m_sound_command1));
	save_item(NAME(m_sound_command2));
}

MACHINE_RESET_MEMBER(cabal_state,cabalbl)
{
	m_sound_command1 = m_sound_command2 = 0xff;
}


/******************************************************************************************/

WRITE16_MEMBER(cabal_state::cabalbl_sndcmd_w)
{
	switch (offset)
	{
		case 0x0:
			m_sound_command1 = data;
			break;

		case 0x1: /* ?? */
			m_sound_command2 = data & 0xff;
			break;
	}
}



WRITE16_MEMBER(cabal_state::track_reset_w)
{
	int i;
	static const char *const track_names[] = { "IN0", "IN1", "IN2", "IN3" };

	for (i = 0; i < 4; i++)
		m_last[i] = ioport(track_names[i])->read();
}

READ16_MEMBER(cabal_state::track_r)
{
	switch (offset)
	{
		default:
		case 0: return (( ioport("IN0")->read() - m_last[0]) & 0x00ff)           | (((ioport("IN2")->read() - m_last[2]) & 0x00ff) << 8);       /* X lo */
		case 1: return (((ioport("IN0")->read() - m_last[0]) & 0xff00) >> 8) | (( ioport("IN2")->read() - m_last[2]) & 0xff00);                 /* X hi */
		case 2: return (( ioport("IN1")->read() - m_last[1]) & 0x00ff)           | (((ioport("IN3")->read() - m_last[3]) & 0x00ff) << 8);       /* Y lo */
		case 3: return (((ioport("IN1")->read() - m_last[1]) & 0xff00) >> 8) | (( ioport("IN3")->read() - m_last[3]) & 0xff00);                 /* Y hi */
	}
}


WRITE16_MEMBER(cabal_state::sound_irq_trigger_word_w)
{
	m_seibu_sound->main_word_w(space,4,data,mem_mask);

	/* spin for a while to let the Z80 read the command, otherwise coins "stick" */
	space.device().execute().spin_until_time(attotime::from_usec(50));
}

WRITE16_MEMBER(cabal_state::cabalbl_sound_irq_trigger_word_w)
{
	m_audiocpu->set_input_line(INPUT_LINE_NMI, PULSE_LINE );
}



static ADDRESS_MAP_START( main_map, AS_PROGRAM, 16, cabal_state )
	AM_RANGE(0x00000, 0x3ffff) AM_ROM
	AM_RANGE(0x40000, 0x437ff) AM_RAM
	AM_RANGE(0x43800, 0x43fff) AM_RAM AM_SHARE("spriteram")
	AM_RANGE(0x44000, 0x4ffff) AM_RAM
	AM_RANGE(0x60000, 0x607ff) AM_RAM_WRITE(text_videoram_w) AM_SHARE("colorram")
	AM_RANGE(0x80000, 0x801ff) AM_RAM_WRITE(background_videoram_w) AM_SHARE("videoram")
	AM_RANGE(0x80200, 0x803ff) AM_RAM
	AM_RANGE(0xa0000, 0xa0001) AM_READ_PORT("DSW")
	AM_RANGE(0xa0008, 0xa000f) AM_READ(track_r)
	AM_RANGE(0xa0010, 0xa0011) AM_READ_PORT("INPUTS")
	AM_RANGE(0xc0000, 0xc0001) AM_WRITE(track_reset_w)
	AM_RANGE(0xc0040, 0xc0041) AM_WRITENOP /* ??? */
	AM_RANGE(0xc0080, 0xc0081) AM_WRITE(flipscreen_w)
	AM_RANGE(0xe0000, 0xe07ff) AM_RAM_DEVWRITE("palette", palette_device, write) AM_SHARE("palette")
	AM_RANGE(0xe8008, 0xe8009) AM_WRITE(sound_irq_trigger_word_w) // fix coin insertion
	AM_RANGE(0xe8000, 0xe800d) AM_DEVREADWRITE("seibu_sound", seibu_sound_device, main_word_r, main_word_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( cabalbl_main_map, AS_PROGRAM, 16, cabal_state )
	AM_RANGE(0x00000, 0x3ffff) AM_ROM
	AM_RANGE(0x40000, 0x437ff) AM_RAM
	AM_RANGE(0x43800, 0x43fff) AM_RAM AM_SHARE("spriteram")
	AM_RANGE(0x44000, 0x4ffff) AM_RAM
	AM_RANGE(0x60000, 0x607ff) AM_RAM_WRITE(text_videoram_w) AM_SHARE("colorram")
	AM_RANGE(0x80000, 0x801ff) AM_RAM_WRITE(background_videoram_w) AM_SHARE("videoram")
	AM_RANGE(0x80200, 0x803ff) AM_RAM
	AM_RANGE(0xa0000, 0xa0001) AM_READ_PORT("DSW")
	AM_RANGE(0xa0008, 0xa0009) AM_READ_PORT("JOY")
	AM_RANGE(0xa0010, 0xa0011) AM_READ_PORT("INPUTS")
	AM_RANGE(0xc0040, 0xc0041) AM_WRITENOP /* ??? */
	AM_RANGE(0xc0080, 0xc0081) AM_WRITE(flipscreen_w)
	AM_RANGE(0xe0000, 0xe07ff) AM_RAM_DEVWRITE("palette", palette_device, write) AM_SHARE("palette")
	AM_RANGE(0xe8000, 0xe8003) AM_WRITE(cabalbl_sndcmd_w)
	AM_RANGE(0xe8004, 0xe8005) AM_READ(soundlatch2_word_r)
	AM_RANGE(0xe8008, 0xe8009) AM_WRITE(cabalbl_sound_irq_trigger_word_w)
ADDRESS_MAP_END

/*********************************************************************/


READ8_MEMBER(cabal_state::cabalbl_snd2_r)
{
	return BITSWAP8(m_sound_command2, 7,2,4,5,3,6,1,0);
}

READ8_MEMBER(cabal_state::cabalbl_snd1_r)
{
	return BITSWAP8(m_sound_command1, 7,2,4,5,3,6,1,0);
}

WRITE8_MEMBER(cabal_state::cabalbl_coin_w)
{
	coin_counter_w(machine(), 0, data & 1);
	coin_counter_w(machine(), 1, data & 2);

	//data & 0x40? video enable?
}

static ADDRESS_MAP_START( sound_map, AS_PROGRAM, 8, cabal_state )
	AM_RANGE(0x0000, 0x1fff) AM_ROM
	AM_RANGE(0x2000, 0x27ff) AM_RAM
	AM_RANGE(0x4001, 0x4001) AM_DEVWRITE("seibu_sound", seibu_sound_device, irq_clear_w)
	AM_RANGE(0x4002, 0x4002) AM_DEVWRITE("seibu_sound", seibu_sound_device, rst10_ack_w)
	AM_RANGE(0x4003, 0x4003) AM_DEVWRITE("seibu_sound", seibu_sound_device, rst18_ack_w)
	AM_RANGE(0x4005, 0x4006) AM_DEVWRITE("adpcm1", seibu_adpcm_device, adr_w)
	AM_RANGE(0x4008, 0x4009) AM_DEVREADWRITE("ymsnd", ym2151_device, read, write)
	AM_RANGE(0x4010, 0x4011) AM_DEVREAD("seibu_sound", seibu_sound_device, soundlatch_r)
	AM_RANGE(0x4012, 0x4012) AM_DEVREAD("seibu_sound", seibu_sound_device, main_data_pending_r)
	AM_RANGE(0x4013, 0x4013) AM_READ_PORT("COIN")
	AM_RANGE(0x4018, 0x4019) AM_DEVWRITE("seibu_sound", seibu_sound_device, main_data_w)
	AM_RANGE(0x401a, 0x401a) AM_DEVWRITE("adpcm1", seibu_adpcm_device, ctl_w)
	AM_RANGE(0x401b, 0x401b) AM_DEVWRITE("seibu_sound", seibu_sound_device, coin_w)
	AM_RANGE(0x6005, 0x6006) AM_DEVWRITE("adpcm2", seibu_adpcm_device, adr_w)
	AM_RANGE(0x601a, 0x601a) AM_DEVWRITE("adpcm2", seibu_adpcm_device, ctl_w)
	AM_RANGE(0x8000, 0xffff) AM_ROMBANK("seibu_bank1")
ADDRESS_MAP_END

static ADDRESS_MAP_START( cabalbl_sound_map, AS_PROGRAM, 8, cabal_state )
	AM_RANGE(0x0000, 0x1fff) AM_ROM
	AM_RANGE(0x2000, 0x2fff) AM_RAM
	AM_RANGE(0x4000, 0x4000) AM_WRITE(soundlatch3_byte_w)
	AM_RANGE(0x4002, 0x4002) AM_WRITE(soundlatch4_byte_w)
	AM_RANGE(0x4004, 0x4004) AM_WRITE(cabalbl_coin_w)
	AM_RANGE(0x4006, 0x4006) AM_READ_PORT("COIN")
	AM_RANGE(0x4008, 0x4008) AM_READ(cabalbl_snd2_r)
	AM_RANGE(0x400a, 0x400a) AM_READ(cabalbl_snd1_r)
	AM_RANGE(0x400c, 0x400c) AM_WRITE(soundlatch2_byte_w)
	AM_RANGE(0x400e, 0x400f) AM_DEVREADWRITE("ymsnd", ym2151_device, read, write)
	AM_RANGE(0x6000, 0x6000) AM_WRITENOP  /* ??? */
	AM_RANGE(0x8000, 0xffff) AM_ROM
ADDRESS_MAP_END

/* the bootleg has 2x z80 sample players */

WRITE8_MEMBER(cabal_state::cabalbl_1_adpcm_w)
{
	m_msm1->reset_w(BIT(data, 7));
	/* ?? bit 6?? */
	m_msm1->data_w(data);
	m_msm1->vclk_w(1);
	m_msm1->vclk_w(0);
}
WRITE8_MEMBER(cabal_state::cabalbl_2_adpcm_w)
{
	m_msm2->reset_w(BIT(data, 7));
	/* ?? bit 6?? */
	m_msm2->data_w(data);
	m_msm2->vclk_w(1);
	m_msm2->vclk_w(0);
}
static ADDRESS_MAP_START( cabalbl_talk1_map, AS_PROGRAM, 8, cabal_state )
	AM_RANGE(0x0000, 0xffff) AM_ROM AM_WRITENOP
ADDRESS_MAP_END

static ADDRESS_MAP_START( cabalbl_talk1_portmap, AS_IO, 8, cabal_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x00) AM_READ(soundlatch3_byte_r)
	AM_RANGE(0x01, 0x01) AM_WRITE(cabalbl_1_adpcm_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( cabalbl_talk2_map, AS_PROGRAM, 8, cabal_state )
	AM_RANGE(0x0000, 0xffff) AM_ROM AM_WRITENOP
ADDRESS_MAP_END

static ADDRESS_MAP_START( cabalbl_talk2_portmap, AS_IO, 8, cabal_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x00) AM_READ(soundlatch4_byte_r)
	AM_RANGE(0x01, 0x01) AM_WRITE(cabalbl_2_adpcm_w)
ADDRESS_MAP_END

/***************************************************************************/

static INPUT_PORTS_START( common )
	PORT_START("DSW")
	PORT_DIPNAME( 0x000f, 0x000f, DEF_STR( Coinage ) ) PORT_DIPLOCATION("SW1:1,2,3,4") PORT_CONDITION("DSW", 0x0010, NOTEQUALS, 0x00)
	PORT_DIPSETTING(      0x000a, DEF_STR( 6C_1C ) )
	PORT_DIPSETTING(      0x000b, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(      0x000c, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x000d, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( 8C_3C ) )
	PORT_DIPSETTING(      0x000e, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( 5C_3C ) )
	PORT_DIPSETTING(      0x0003, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(      0x000f, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(      0x0009, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0007, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x0006, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(      0x0005, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x0003, 0x0003, DEF_STR( Coin_A )) PORT_DIPLOCATION("SW1:1,2") PORT_CONDITION("DSW", 0x0010, EQUALS, 0x00)
	PORT_DIPSETTING(      0x0000, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0003, DEF_STR( 1C_1C ) )
	PORT_DIPNAME( 0x000c, 0x000c, DEF_STR( Coin_B )) PORT_DIPLOCATION("SW1:3,4") PORT_CONDITION("DSW", 0x0010, EQUALS, 0x00)
	PORT_DIPSETTING(      0x000c, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 1C_6C ) )
	PORT_DIPNAME( 0x0010, 0x0010, "Coin Mode" ) PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(      0x0010, "Mode 1" )
	PORT_DIPSETTING(      0x0000, "Mode 2" )
	PORT_DIPNAME( 0x0020, 0x0020, "Invert Buttons" ) PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Flip_Screen ) ) PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Trackball ) ) PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(      0x0080, "Small" )
	PORT_DIPSETTING(      0x0000, "Large" )
	PORT_DIPNAME( 0x0300, 0x0300, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(      0x0200, "2" )
	PORT_DIPSETTING(      0x0300, "3" )
	PORT_DIPSETTING(      0x0100, "5" )
	PORT_DIPSETTING(      0x0000, "121 (Cheat)")
	PORT_DIPNAME( 0x0c00, 0x0c00, DEF_STR( Bonus_Life ) ) PORT_DIPLOCATION("SW2:3,4")
	PORT_DIPSETTING(      0x0c00, "150k 650k 500k+" )
	PORT_DIPSETTING(      0x0800, "200k 800k 600k+" )
	PORT_DIPSETTING(      0x0400, "300k 1000k 700k+" )
	PORT_DIPSETTING(      0x0000, "300k Only" )
	PORT_DIPNAME( 0x3000, 0x2000, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW2:5,6")
	PORT_DIPSETTING(      0x3000, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Very_Hard ) )
	PORT_DIPUNKNOWN_DIPLOC( 0x4000, 0x4000, "SW2:7" )   /* Left blank in the manual */
	PORT_DIPNAME( 0x8000, 0x8000, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x8000, DEF_STR( On ) )

	PORT_START("COIN")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 ) PORT_IMPULSE(4) /* read through sound cpu */
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN2 ) PORT_IMPULSE(4) /* read through sound cpu */
INPUT_PORTS_END

static INPUT_PORTS_START( cabalt )
	PORT_INCLUDE( common )

	PORT_START("INPUTS")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x0ff0, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_START1 )

	PORT_START("IN0")
	PORT_BIT( 0x0fff, 0x0000, IPT_TRACKBALL_X ) PORT_SENSITIVITY(100) PORT_KEYDELTA(30) PORT_PLAYER(1)

	PORT_START("IN1")
	PORT_BIT( 0x0fff, 0x0000, IPT_TRACKBALL_Y ) PORT_SENSITIVITY(100) PORT_KEYDELTA(30) PORT_REVERSE PORT_PLAYER(1)

	PORT_START("IN2")
	PORT_BIT( 0x0fff, 0x0000, IPT_TRACKBALL_X ) PORT_SENSITIVITY(100) PORT_KEYDELTA(30) PORT_PLAYER(2)

	PORT_START("IN3")
	PORT_BIT( 0x0fff, 0x0000, IPT_TRACKBALL_Y ) PORT_SENSITIVITY(100) PORT_KEYDELTA(30) PORT_PLAYER(2)
INPUT_PORTS_END



static INPUT_PORTS_START( cabalj )
	PORT_INCLUDE( common )

	PORT_MODIFY("COIN")
	PORT_BIT( 0xfc, IP_ACTIVE_LOW, IPT_UNKNOWN )

	/* Since the Trackball version was produced first, and it doesn't use
	   the third button,  Pin 24 of the JAMMA connector ('JAMMA button 3')
	   has no trace on the pcb.  To work around this design issue the
	   manufacturer had to use pin 15 which is usually the test / service
	   button
	*/
	PORT_START("INPUTS")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x0ff0, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2) /* the 3rd button connects to the service switch */
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_START1 )

	/* The joystick version has a PCB marked "Joystick sub" containing a 74ls245. It plugs in the
	   sockets of the two D4701AC */
	PORT_START("IN0")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN1")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)

	PORT_START("IN3")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END

static INPUT_PORTS_START( cabalbl )
	PORT_INCLUDE( common )

	PORT_START("INPUTS")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x00f0, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0f00, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_START1 )

	PORT_START("JOY")
	PORT_BIT( 0x00ff, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
INPUT_PORTS_END

static const gfx_layout text_layout =
{
	8,8,
	RGN_FRAC(1,1),
	2,
	{ 0,4 },
	{ 3, 2, 1, 0, 8+3, 8+2, 8+1, 8+0},
	{ 0*16, 1*16, 2*16, 3*16, 4*16, 5*16, 6*16, 7*16 },
	16*8
};

static const gfx_layout tile_layout =
{
	16,16,
	RGN_FRAC(1,1),
	4,
	{ 8, 12, 0, 4 },
	{ 3, 2, 1, 0, 16+3, 16+2, 16+1, 16+0,
			32*16+3, 32*16+2, 32*16+1, 32*16+0, 33*16+3, 33*16+2, 33*16+1, 33*16+0 },
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32,
			8*32, 9*32, 10*32,  11*32,  12*32,  13*32, 14*32,  15*32 },
	64*16
};

static const gfx_layout sprite_layout =
{
	16,16,
	RGN_FRAC(1,1),
	4,
	{ 8, 12, 0, 4 },
	{ 3, 2, 1, 0, 16+3, 16+2, 16+1, 16+0,
			32+3, 32+2, 32+1, 32+0, 48+3, 48+2, 48+1, 48+0 },
	{ 30*32, 28*32, 26*32, 24*32, 22*32, 20*32, 18*32, 16*32,
			14*32, 12*32, 10*32,  8*32,  6*32,  4*32,  2*32,  0*32 },
	64*16
};



static GFXDECODE_START( cabal )
	GFXDECODE_ENTRY( "gfx1", 0x000000, text_layout,   0, 1024/4 )
	GFXDECODE_ENTRY( "gfx2", 0x000000, tile_layout,   32*16, 16 )
	GFXDECODE_ENTRY( "gfx3", 0x000000, sprite_layout, 16*16, 16 )
GFXDECODE_END


static MACHINE_CONFIG_START( cabal_base, cabal_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68000, XTAL_20MHz/2) /* verified on pcb */
	MCFG_CPU_PROGRAM_MAP(main_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", cabal_state,  irq1_line_hold)

	MCFG_CPU_ADD("audiocpu", Z80, XTAL_3_579545MHz) /* verified on pcb */
	MCFG_CPU_PROGRAM_MAP(sound_map)

	MCFG_MACHINE_START_OVERRIDE(cabal_state,cabal)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(59.60)   /* verified on pcb */
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500) /* not accurate */)
	MCFG_SCREEN_SIZE(256, 256)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 2*8, 30*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(cabal_state, screen_update)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", cabal)
	MCFG_PALETTE_ADD("palette", 1024)
	MCFG_PALETTE_FORMAT(xxxxBBBBGGGGRRRR)

	/* sound hardware */
	MCFG_DEVICE_ADD("seibu_sound", SEIBU_SOUND, 0)

	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_YM2151_ADD("ymsnd", XTAL_3_579545MHz) /* verified on pcb */
	MCFG_YM2151_IRQ_HANDLER(DEVWRITELINE("seibu_sound", seibu_sound_device, fm_irqhandler))
	MCFG_SOUND_ROUTE(ALL_OUTPUTS,"mono", 0.80)

	MCFG_SOUND_ADD("adpcm1", SEIBU_ADPCM, 8000) /* it should use the msm5205 */
	MCFG_SEIBU_ADPCM_ROMREGION("adpcm1")
	MCFG_SOUND_ROUTE(ALL_OUTPUTS,"mono", 0.40)

	MCFG_SOUND_ADD("adpcm2", SEIBU_ADPCM, 8000) /* it should use the msm5205 */
	MCFG_SEIBU_ADPCM_ROMREGION("adpcm2")
	MCFG_SOUND_ROUTE(ALL_OUTPUTS,"mono", 0.40)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( cabal, cabal_base )
	SEIBU_SOUND_SYSTEM_ENCRYPTED_LOW()
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( cabalbl2, cabal_base )
	SEIBU_SOUND_SYSTEM_ENCRYPTED_CUSTOM()
MACHINE_CONFIG_END


/* the bootleg has different sound hardware (2 extra Z80s for ADPCM playback) */
static MACHINE_CONFIG_START( cabalbl, cabal_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68000, XTAL_20MHz/2) /* verified on pcb */
	MCFG_CPU_PROGRAM_MAP(cabalbl_main_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", cabal_state,  irq1_line_hold)

	MCFG_CPU_ADD("audiocpu", Z80, XTAL_3_579545MHz) /* verified on pcb */
	MCFG_CPU_PROGRAM_MAP(cabalbl_sound_map)

	/* there are 2x z80s for the ADPCM */
	MCFG_CPU_ADD("adpcm_1", Z80, XTAL_3_579545MHz) /* verified on pcb */
	MCFG_CPU_PROGRAM_MAP(cabalbl_talk1_map)
	MCFG_CPU_IO_MAP(cabalbl_talk1_portmap)
	MCFG_CPU_PERIODIC_INT_DRIVER(cabal_state, irq0_line_hold, 8000)

	MCFG_CPU_ADD("adpcm_2", Z80, XTAL_3_579545MHz) /* verified on pcb */
	MCFG_CPU_PROGRAM_MAP(cabalbl_talk2_map)
	MCFG_CPU_IO_MAP(cabalbl_talk2_portmap)
	MCFG_CPU_PERIODIC_INT_DRIVER(cabal_state, irq0_line_hold, 8000)

	MCFG_QUANTUM_TIME(attotime::from_hz(600))

	MCFG_MACHINE_START_OVERRIDE(cabal_state,cabalbl)
	MCFG_MACHINE_RESET_OVERRIDE(cabal_state,cabalbl)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500) /* not accurate */)
	MCFG_SCREEN_SIZE(256, 256)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 2*8, 30*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(cabal_state, screen_update)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", cabal)
	MCFG_PALETTE_ADD("palette", 1024)
	MCFG_PALETTE_FORMAT(xxxxBBBBGGGGRRRR)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_YM2151_ADD("ymsnd", XTAL_3_579545MHz) /* verified on pcb */
	MCFG_YM2151_IRQ_HANDLER(INPUTLINE("audiocpu", 0))
	MCFG_SOUND_ROUTE(ALL_OUTPUTS,"mono", 0.80)

	MCFG_SOUND_ADD("msm1", MSM5205, XTAL_12MHz/32) /* verified on pcb (no resonator) */
	MCFG_MSM5205_PRESCALER_SELECTOR(MSM5205_SEX_4B)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.60)

	MCFG_SOUND_ADD("msm2", MSM5205, XTAL_12MHz/32) /* verified on pcb (no resonator)*/
	MCFG_MSM5205_PRESCALER_SELECTOR(MSM5205_SEX_4B)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.60)
MACHINE_CONFIG_END

ROM_START( cabal )
	ROM_REGION( 0x50000, "maincpu", 0 ) /* 64k for cpu code */
	ROM_LOAD16_BYTE( "13.7h",    0x00000, 0x10000, CRC(00abbe0c) SHA1(bacf17444abfb4f56248ff56e37b0aa2b1a3800d) )
	ROM_LOAD16_BYTE( "11.6h",    0x00001, 0x10000, CRC(44736281) SHA1(1d6da95ef96d9c02aea70791e1cb87b70097d5ed) )
	ROM_LOAD16_BYTE( "12.7j",    0x20000, 0x10000, CRC(d763a47c) SHA1(146d8082a404b6eddaf2dc9ba41a997949c17f8a) )
	ROM_LOAD16_BYTE( "10.6j",    0x20001, 0x10000, CRC(96d5e8af) SHA1(ed7d854f08e87db5ae6cf526eafa029dfd2bfb9f) )

	ROM_REGION( 0x10000, "audiocpu", 0 )    /* 64k for sound cpu code */
	ROM_LOAD( "4-3n",         0x0000, 0x2000, CRC(4038eff2) SHA1(0bcafc1b78c3bef9a0e9b822c482ea4a942fd180) )
	ROM_LOAD( "3-3p",         0x8000, 0x8000, CRC(d9defcbf) SHA1(f26b10b1dbe5aa6446f70fd18e5f1379455578ec) )

	ROM_REGION( 0x4000,  "gfx1", 0 )
	ROM_LOAD( "5-6s",           0x00000, 0x04000, CRC(6a76955a) SHA1(733cb4b862b5dac97c2641b58f2362471e62fcf2) ) /* characters */

	/* The Joystick versions use a sub-board instead of the mask roms
	   the content is the same as the mask roms */
	ROM_REGION( 0x80000, "gfx2", 0 )
	ROM_LOAD16_BYTE( "bg_rom1.bin",   0x00000, 0x10000, CRC(1023319b) SHA1(38fcc8159776b82779b3163329b07c61be939fae) )
	ROM_LOAD16_BYTE( "bg_rom2.bin",   0x00001, 0x10000, CRC(3b6d2b09) SHA1(4cdcd22836dce4ee6348c4e6df7c6360d12ef912) )
	ROM_LOAD16_BYTE( "bg_rom3.bin",   0x20000, 0x10000, CRC(420b0801) SHA1(175be6e3ca3cb98672e4cdbc9b5f5b007bc531c9) )
	ROM_LOAD16_BYTE( "bg_rom4.bin",   0x20001, 0x10000, CRC(77bc7a60) SHA1(4d148241835f6a6b63f66494636c09a1fc1d3c06) )
	ROM_LOAD16_BYTE( "bg_rom5.bin",   0x40000, 0x10000, CRC(543fcb37) SHA1(78c40f6a78a8b9ca9f73fc67fc87f78b15e7abbe) )
	ROM_LOAD16_BYTE( "bg_rom6.bin",   0x40001, 0x10000, CRC(0bc50075) SHA1(565eb59b41f71fb69f62397f9747f5ae18b83009) )
	ROM_LOAD16_BYTE( "bg_rom7.bin",   0x60000, 0x10000, CRC(d28d921e) SHA1(e133de5129a33ca9ff449948a959621bbfc58c11) )
	ROM_LOAD16_BYTE( "bg_rom8.bin",   0x60001, 0x10000, CRC(67e4fe47) SHA1(15620fc5e985a249677da333b77331e40d2b24ab) )

	ROM_REGION( 0x80000, "gfx3", 0 )
	ROM_LOAD16_BYTE( "sp_rom1.bin",   0x00000, 0x10000, CRC(34d3cac8) SHA1(a6a2304fb576267db2c72cfbf0a3f66740ebe60e) )
	ROM_LOAD16_BYTE( "sp_rom2.bin",   0x00001, 0x10000, CRC(4e49c28e) SHA1(ea74443a9423b14611a1f97e44692badfedd0ead) )
	ROM_LOAD16_BYTE( "sp_rom3.bin",   0x20000, 0x10000, CRC(7065e840) SHA1(baa8cd28be60c678d782ecfabde6cd5e36480415) )
	ROM_LOAD16_BYTE( "sp_rom4.bin",   0x20001, 0x10000, CRC(6a0e739d) SHA1(e3f4f5b4587f573426ec00417f33e94a257c77e6) )
	ROM_LOAD16_BYTE( "sp_rom5.bin",   0x40000, 0x10000, CRC(0e1ec30e) SHA1(4b1f092fc1e92da0f92e55d1548db7961a13f717) )
	ROM_LOAD16_BYTE( "sp_rom6.bin",   0x40001, 0x10000, CRC(581a50c1) SHA1(5afd65c15a0a63a54727e6d882011f0718a9fefc) )
	ROM_LOAD16_BYTE( "sp_rom7.bin",   0x60000, 0x10000, CRC(55c44764) SHA1(7fad1f2084664b5b4d1384c8081371b0c79c4f5e) )
	ROM_LOAD16_BYTE( "sp_rom8.bin",   0x60001, 0x10000, CRC(702735c9) SHA1(e4ac799dc85ff5b7c8e578611605989c78f9e8b3) )

	ROM_REGION( 0x10000, "adpcm1", 0 )  /* Samples */
	ROM_LOAD( "2-1s",           0x00000, 0x10000, CRC(850406b4) SHA1(23ac1650c6d6f35607a5264b3aa89868401a645a) )

	ROM_REGION( 0x10000, "adpcm2", 0 )  /* Samples */
	ROM_LOAD( "1-1u",           0x00000, 0x10000, CRC(8b3e0789) SHA1(b1450db1b1bada237c90930623e4def321099f13) )
ROM_END

ROM_START( cabala )
	ROM_REGION( 0x50000, "maincpu", 0 ) /* 64k for cpu code */
	ROM_LOAD16_BYTE( "epr-a-9.7h",    0x00000, 0x10000, CRC(00abbe0c) SHA1(bacf17444abfb4f56248ff56e37b0aa2b1a3800d) )
	ROM_LOAD16_BYTE( "epr-a-7.6h",    0x00001, 0x10000, CRC(c89608db) SHA1(a56e77526227af5b693eea9ef74da0d9d57cc55c) )
	ROM_LOAD16_BYTE( "epr-a-8.7k",    0x20000, 0x08000, CRC(fe84788a) SHA1(29c49ebbe62357c27befcdcc4c19841a8bf32b2d) )
	ROM_RELOAD(0x30000,0x08000)
	ROM_LOAD16_BYTE( "epr-a-6.6k",    0x20001, 0x08000, CRC(81eb1355) SHA1(bbf926d40164d78319e982da0e8fb8ec4d4f8b87) )
	ROM_RELOAD(0x30001,0x08000)

	ROM_REGION( 0x10000, "audiocpu", 0 )    /* 64k for sound cpu code */
	ROM_LOAD( "epr-a-4.3n",         0x0000, 0x2000, CRC(4038eff2) SHA1(0bcafc1b78c3bef9a0e9b822c482ea4a942fd180) )
	ROM_LOAD( "epr-a-3.3p",         0x8000, 0x4000, CRC(c0097c55) SHA1(874f813c1b466dab2d15a707e340b9bdb200246c) )

	ROM_REGION( 0x8000,  "gfx1", 0 )
	ROM_LOAD( "epr-a-5.6s",           0x00000, 0x08000, CRC(189033fd) SHA1(814f0cbc5f72345c04922d6d7c986f99d57335fa) ) /* characters */

	ROM_REGION( 0x80000, "gfx2", 0 )
	ROM_LOAD( "tad-2.7s",       0x00000, 0x80000, CRC(13ca7ae1) SHA1(b26bb4876a6518e3809e0fa4d442616508b3e7e8) ) /* tiles */

	ROM_REGION( 0x80000, "gfx3", 0 )
	ROM_LOAD( "tad-1.5e",       0x00000, 0x80000, CRC(8324a7fe) SHA1(aed4470df35ec18e65e35bddc9c217a5019fdcbf) ) /* sprites */

	ROM_REGION( 0x10000, "adpcm1", 0 )  /* Samples */
	ROM_LOAD( "epr-a-2.1s",           0x00000, 0x10000, CRC(850406b4) SHA1(23ac1650c6d6f35607a5264b3aa89868401a645a) )

	ROM_REGION( 0x10000, "adpcm2", 0 )  /* Samples */
	ROM_LOAD( "epr-a-1.1u",           0x00000, 0x10000, CRC(8b3e0789) SHA1(b1450db1b1bada237c90930623e4def321099f13) )
ROM_END

ROM_START( cabalus )
	ROM_REGION( 0x50000, "maincpu", 0 ) /* 64k for cpu code */
	ROM_LOAD16_BYTE( "h7_512.bin",      0x00000, 0x10000, CRC(8fe16fb4) SHA1(fedb2d0c6c21516f68cfa99093772fe8fa862389) )
	ROM_LOAD16_BYTE( "h6_512.bin",      0x00001, 0x10000, CRC(6968101c) SHA1(d65005ac235dae5c32bbcd182cb365e8fa067fe7) )
	ROM_LOAD16_BYTE( "k7_512.bin",      0x20000, 0x10000, CRC(562031a2) SHA1(ed5ef50a66c7797a7f345e479162cf83d6777f7c) )
	ROM_LOAD16_BYTE( "k6_512.bin",      0x20001, 0x10000, CRC(4fda2856) SHA1(a213cb7443cdccbad3f2610e8d42b2e149cbedb9) )

	ROM_REGION( 0x10000, "audiocpu", 0 )    /* 64k for sound cpu code */
	ROM_LOAD( "4-3n",         0x0000, 0x2000, CRC(4038eff2) SHA1(0bcafc1b78c3bef9a0e9b822c482ea4a942fd180) )
	ROM_LOAD( "3-3p",         0x8000, 0x8000, CRC(d9defcbf) SHA1(f26b10b1dbe5aa6446f70fd18e5f1379455578ec) )

	ROM_REGION( 0x4000,  "gfx1", 0 )
	ROM_LOAD( "t6_128.bin",     0x00000, 0x04000, CRC(1ccee214) SHA1(7c842bc1c6002ec90693160fd5407345092420bb) ) /* characters */

	ROM_REGION( 0x80000, "gfx2", 0 )
	ROM_LOAD( "tad-2.7s",       0x00000, 0x80000, CRC(13ca7ae1) SHA1(b26bb4876a6518e3809e0fa4d442616508b3e7e8) ) /* tiles */

	ROM_REGION( 0x80000, "gfx3", 0 )
	ROM_LOAD( "tad-1.5e",       0x00000, 0x80000, CRC(8324a7fe) SHA1(aed4470df35ec18e65e35bddc9c217a5019fdcbf) ) /* sprites */

	ROM_REGION( 0x10000, "adpcm1", 0 )  /* Samples? */
	ROM_LOAD( "2-1s",           0x00000, 0x10000, CRC(850406b4) SHA1(23ac1650c6d6f35607a5264b3aa89868401a645a) )

	ROM_REGION( 0x10000, "adpcm2", 0 )  /* Samples */
	ROM_LOAD( "1-1u",           0x00000, 0x10000, CRC(8b3e0789) SHA1(b1450db1b1bada237c90930623e4def321099f13) )

	ROM_REGION( 0x0200, "proms", 0 )    /* unknown */
	ROM_LOAD( "prom05.8e",      0x0000, 0x0100, CRC(a94b18c2) SHA1(e7db4c1efc9e313e36eef3f53ae5b2e573a38920) )
	ROM_LOAD( "prom10.4j",      0x0100, 0x0100, CRC(261c93bc) SHA1(942470198143d584d3766f28587d1879abd912c1) )
ROM_END

ROM_START( cabalus2 )
	ROM_REGION( 0x50000, "maincpu", 0 ) /* 64k for cpu code */
	ROM_LOAD16_BYTE( "9-7h",            0x00000, 0x10000, CRC(ebbb9484) SHA1(2c77d5b4acdc37720dc7ccab526862981bf8da51) )
	ROM_LOAD16_BYTE( "7-6h",            0x00001, 0x10000, CRC(51aeb49e) SHA1(df38dc58d8c6fa3d35904bf34e29111e7bd523ad) )
	ROM_LOAD16_BYTE( "8-7k",            0x20000, 0x10000, CRC(4c24ed9a) SHA1(f0fc25c3e7dc8ac71fdad3e91ab618cd7a037123) )
	ROM_LOAD16_BYTE( "6-6k",            0x20001, 0x10000, CRC(681620e8) SHA1(c9eacfb55059986dbecc2fae1339069a852f917b) )

	ROM_REGION( 0x10000, "audiocpu", 0 )    /* 64k for sound cpu code */
	ROM_LOAD( "4-3n",         0x0000, 0x2000, CRC(4038eff2) SHA1(0bcafc1b78c3bef9a0e9b822c482ea4a942fd180) )
	ROM_LOAD( "3-3p",         0x8000, 0x8000, CRC(d9defcbf) SHA1(f26b10b1dbe5aa6446f70fd18e5f1379455578ec) )

	ROM_REGION( 0x4000,  "gfx1", 0 )
	ROM_LOAD( "5-6s",           0x00000, 0x04000, CRC(6a76955a) SHA1(733cb4b862b5dac97c2641b58f2362471e62fcf2) ) /* characters */

	ROM_REGION( 0x80000, "gfx2", 0 )
	ROM_LOAD( "tad-2.7s",       0x00000, 0x80000, CRC(13ca7ae1) SHA1(b26bb4876a6518e3809e0fa4d442616508b3e7e8) ) /* tiles */

	ROM_REGION( 0x80000, "gfx3", 0 )
	ROM_LOAD( "tad-1.5e",       0x00000, 0x80000, CRC(8324a7fe) SHA1(aed4470df35ec18e65e35bddc9c217a5019fdcbf) ) /* sprites */

	ROM_REGION( 0x10000, "adpcm1", 0 )  /* Samples */
	ROM_LOAD( "2-1s",           0x00000, 0x10000, CRC(850406b4) SHA1(23ac1650c6d6f35607a5264b3aa89868401a645a) )

	ROM_REGION( 0x10000, "adpcm2", 0 )  /* Samples */
	ROM_LOAD( "1-1u",           0x00000, 0x10000, CRC(8b3e0789) SHA1(b1450db1b1bada237c90930623e4def321099f13) )

	ROM_REGION( 0x0200, "proms", 0 )    /* unknown */
	ROM_LOAD( "prom05.8e",      0x0000, 0x0100, CRC(a94b18c2) SHA1(e7db4c1efc9e313e36eef3f53ae5b2e573a38920) )
	ROM_LOAD( "prom10.4j",      0x0100, 0x0100, CRC(261c93bc) SHA1(942470198143d584d3766f28587d1879abd912c1) )
ROM_END

/*

cabal - tad corporation ? - (clone)

2 boards

1st board

(prg)
1 x 68000
from cabal_21 to cabal_24

(snd)
1 x z80
1 x ym2151
cabal_11

(gfx ?)
from cabal_12 to cabal_19

(?)
2 x z80
cabal_09 and cabal_10

(?)
cabal_20 (near the snd area)

2nd board

(gfx)
from cabal_01 to cabal_08

Note: The bootleg has *3* Z80s

*/

ROM_START( cabalbl )
	ROM_REGION( 0x50000, "maincpu", 0 ) /* 64k for cpu code */
	ROM_LOAD16_BYTE( "cabal_24.bin",    0x00000, 0x10000, CRC(00abbe0c) SHA1(bacf17444abfb4f56248ff56e37b0aa2b1a3800d) )
	ROM_LOAD16_BYTE( "cabal_22.bin",    0x00001, 0x10000, CRC(78c4af27) SHA1(31049d1ec76d76284682de7a0592f63d97019240) )
	ROM_LOAD16_BYTE( "cabal_23.bin",    0x20000, 0x10000, CRC(d763a47c) SHA1(146d8082a404b6eddaf2dc9ba41a997949c17f8a) )
	ROM_LOAD16_BYTE( "cabal_21.bin",    0x20001, 0x10000, CRC(96d5e8af) SHA1(ed7d854f08e87db5ae6cf526eafa029dfd2bfb9f) )

	ROM_REGION( 0x10000, "audiocpu", 0 )    /* 64k for sound cpu code */
	ROM_LOAD( "cabal_11.bin",    0x0000, 0x10000, CRC(d308a543) SHA1(4f45db42512f83266001daee55d06f49e7908e35) )

	ROM_REGION( 0x8000,  "gfx1", 0 )
	ROM_LOAD( "cabal_20.bin",           0x00000, 0x08000, CRC(189033fd) SHA1(814f0cbc5f72345c04922d6d7c986f99d57335fa) ) /* characters */

	/* The bootleg versions use a sub-board instead of the mask roms
	   the content is the same as the mask roms */
	ROM_REGION( 0x80000, "gfx2", 0 )
	ROM_LOAD16_BYTE( "cabal_15.bin",   0x00000, 0x10000, CRC(1023319b) SHA1(38fcc8159776b82779b3163329b07c61be939fae) )
	ROM_LOAD16_BYTE( "cabal_17.bin",   0x00001, 0x10000, CRC(3b6d2b09) SHA1(4cdcd22836dce4ee6348c4e6df7c6360d12ef912) )
	ROM_LOAD16_BYTE( "cabal_14.bin",   0x20000, 0x10000, CRC(420b0801) SHA1(175be6e3ca3cb98672e4cdbc9b5f5b007bc531c9) )
	ROM_LOAD16_BYTE( "cabal_16.bin",   0x20001, 0x10000, CRC(77bc7a60) SHA1(4d148241835f6a6b63f66494636c09a1fc1d3c06) )
	ROM_LOAD16_BYTE( "cabal_12.bin",   0x40000, 0x10000, CRC(543fcb37) SHA1(78c40f6a78a8b9ca9f73fc67fc87f78b15e7abbe) )
	ROM_LOAD16_BYTE( "cabal_18.bin",   0x40001, 0x10000, CRC(0bc50075) SHA1(565eb59b41f71fb69f62397f9747f5ae18b83009) )
	ROM_LOAD16_BYTE( "cabal_13.bin",   0x60000, 0x10000, CRC(d28d921e) SHA1(e133de5129a33ca9ff449948a959621bbfc58c11) )
	ROM_LOAD16_BYTE( "cabal_19.bin",   0x60001, 0x10000, CRC(67e4fe47) SHA1(15620fc5e985a249677da333b77331e40d2b24ab) )

	ROM_REGION( 0x80000, "gfx3", 0 )
	ROM_LOAD16_BYTE( "cabal_04.bin",   0x00000, 0x10000, CRC(34d3cac8) SHA1(a6a2304fb576267db2c72cfbf0a3f66740ebe60e) )
	ROM_LOAD16_BYTE( "cabal_05.bin",   0x00001, 0x10000, CRC(4e49c28e) SHA1(ea74443a9423b14611a1f97e44692badfedd0ead) )
	ROM_LOAD16_BYTE( "cabal_03.bin",   0x20000, 0x10000, CRC(7065e840) SHA1(baa8cd28be60c678d782ecfabde6cd5e36480415) )
	ROM_LOAD16_BYTE( "cabal_06.bin",   0x20001, 0x10000, CRC(6a0e739d) SHA1(e3f4f5b4587f573426ec00417f33e94a257c77e6) )
	ROM_LOAD16_BYTE( "cabal_02.bin",   0x40000, 0x10000, CRC(0e1ec30e) SHA1(4b1f092fc1e92da0f92e55d1548db7961a13f717) )
	ROM_LOAD16_BYTE( "cabal_07.bin",   0x40001, 0x10000, CRC(581a50c1) SHA1(5afd65c15a0a63a54727e6d882011f0718a9fefc) )
	ROM_LOAD16_BYTE( "cabal_01.bin",   0x60000, 0x10000, CRC(55c44764) SHA1(7fad1f2084664b5b4d1384c8081371b0c79c4f5e) )
	ROM_LOAD16_BYTE( "cabal_08.bin",   0x60001, 0x10000, CRC(702735c9) SHA1(e4ac799dc85ff5b7c8e578611605989c78f9e8b3) )

	ROM_REGION( 0x10000, "adpcm_1", 0 )
	ROM_LOAD( "cabal_09.bin",   0x00000, 0x10000, CRC(4ffa7fe3) SHA1(381d8e765a7b94678fb3308965c748bbe9f8e247) ) /* Z80 code/adpcm data */

	ROM_REGION( 0x10000, "adpcm_2", 0 )
	ROM_LOAD( "cabal_10.bin",   0x00000, 0x10000, CRC(958789b6) SHA1(344c3ee8a1e272b56499e5c0415bb714aec0ddcf) ) /* Z80 code/adpcm data */
ROM_END


// alternate bootleg
// this is much closer to the original, the only real difference is the soundcpu has been pre-decrypted,
// with the encrypted/decrypted data split across the rom
// based on stickers present on the board it appears to have been manufactured by 'TAB-Austria' and is marked 'CA02'

ROM_START( cabalbl2 )
	ROM_REGION( 0x40000, "maincpu", 0 ) /* 64k for cpu code */
	ROM_LOAD16_BYTE( "c9.bin",    0x00000, 0x10000, CRC(00abbe0c) SHA1(bacf17444abfb4f56248ff56e37b0aa2b1a3800d) )
	ROM_LOAD16_BYTE( "c7.bin",    0x00001, 0x10000, CRC(44736281) SHA1(1d6da95ef96d9c02aea70791e1cb87b70097d5ed) )
	ROM_LOAD16_BYTE( "c8.bin",    0x20000, 0x10000, CRC(d763a47c) SHA1(146d8082a404b6eddaf2dc9ba41a997949c17f8a) )
	ROM_LOAD16_BYTE( "c6.bin",    0x20001, 0x10000, CRC(96d5e8af) SHA1(ed7d854f08e87db5ae6cf526eafa029dfd2bfb9f) )

	ROM_REGION( 0x10000, "audiocpu", 0 )    /* 64k for sound cpu code */
	ROM_LOAD( "c4.bin",    0x2000, 0x2000, CRC(82f9f296) SHA1(2769ffdc28f003684e77d4806be07b87d50be31c) )
	ROM_CONTINUE(0x0000,0x2000)
	ROM_IGNORE(0x4000)
	ROM_LOAD( "c3.bin",    0x8000, 0x8000,  CRC(d9defcbf) SHA1(f26b10b1dbe5aa6446f70fd18e5f1379455578ec) )

	ROM_REGION( 0x4000,  "gfx1", 0 )
	ROM_LOAD( "c5.bin",           0x00000, 0x04000, CRC(183e4834) SHA1(05ab0c388be8701930a9de437978206cda6fed68) ) /* characters */
	ROM_CONTINUE(0x0000,0x4000)

	ROM_REGION( 0x80000, "gfx2", 0 )
	ROM_LOAD16_BYTE( "c14.bin",   0x00000, 0x10000, CRC(1023319b) SHA1(38fcc8159776b82779b3163329b07c61be939fae) )
	ROM_LOAD16_BYTE( "c10.bin",   0x00001, 0x10000, CRC(3b6d2b09) SHA1(4cdcd22836dce4ee6348c4e6df7c6360d12ef912) )
	ROM_LOAD16_BYTE( "c15.bin",   0x20000, 0x10000, CRC(420b0801) SHA1(175be6e3ca3cb98672e4cdbc9b5f5b007bc531c9) )
	ROM_LOAD16_BYTE( "c11.bin",   0x20001, 0x10000, CRC(77bc7a60) SHA1(4d148241835f6a6b63f66494636c09a1fc1d3c06) )
	ROM_LOAD16_BYTE( "c16.bin",   0x40000, 0x10000, CRC(543fcb37) SHA1(78c40f6a78a8b9ca9f73fc67fc87f78b15e7abbe) )
	ROM_LOAD16_BYTE( "c12.bin",   0x40001, 0x10000, CRC(0bc50075) SHA1(565eb59b41f71fb69f62397f9747f5ae18b83009) )
	ROM_LOAD16_BYTE( "c17.bin",   0x60000, 0x10000, CRC(d28d921e) SHA1(e133de5129a33ca9ff449948a959621bbfc58c11) )
	ROM_LOAD16_BYTE( "c13.bin",   0x60001, 0x10000, CRC(67e4fe47) SHA1(15620fc5e985a249677da333b77331e40d2b24ab) )

	ROM_REGION( 0x80000, "gfx3", 0 )
	ROM_LOAD16_BYTE( "c18.bin",   0x00000, 0x10000, CRC(34d3cac8) SHA1(a6a2304fb576267db2c72cfbf0a3f66740ebe60e) )
	ROM_LOAD16_BYTE( "c22.bin",   0x00001, 0x10000, CRC(4e49c28e) SHA1(ea74443a9423b14611a1f97e44692badfedd0ead) )
	ROM_LOAD16_BYTE( "c19.bin",   0x20000, 0x10000, CRC(7065e840) SHA1(baa8cd28be60c678d782ecfabde6cd5e36480415) )
	ROM_LOAD16_BYTE( "c23.bin",   0x20001, 0x10000, CRC(6a0e739d) SHA1(e3f4f5b4587f573426ec00417f33e94a257c77e6) )
	ROM_LOAD16_BYTE( "c20.bin",   0x40000, 0x10000, CRC(0e1ec30e) SHA1(4b1f092fc1e92da0f92e55d1548db7961a13f717) )
	ROM_LOAD16_BYTE( "c24.bin",   0x40001, 0x10000, CRC(581a50c1) SHA1(5afd65c15a0a63a54727e6d882011f0718a9fefc) )
	ROM_LOAD16_BYTE( "c21.bin",   0x60000, 0x10000, CRC(55c44764) SHA1(7fad1f2084664b5b4d1384c8081371b0c79c4f5e) )
	ROM_LOAD16_BYTE( "c25.bin",   0x60001, 0x10000, CRC(702735c9) SHA1(e4ac799dc85ff5b7c8e578611605989c78f9e8b3) )

	ROM_REGION( 0x10000, "adpcm1", 0 )  /* Samples */
	ROM_LOAD( "c2.bin",           0x00000, 0x10000, CRC(850406b4) SHA1(23ac1650c6d6f35607a5264b3aa89868401a645a) )

	ROM_REGION( 0x10000, "adpcm2", 0 )  /* Samples */
	ROM_LOAD( "c1.bin",           0x00000, 0x10000, CRC(8b3e0789) SHA1(b1450db1b1bada237c90930623e4def321099f13) )
ROM_END


DRIVER_INIT_MEMBER(cabal_state,cabal)
{
	m_adpcm1->decrypt("adpcm1");
	m_adpcm2->decrypt("adpcm2");
}

DRIVER_INIT_MEMBER(cabal_state,cabalbl2)
{
	UINT8 *decrypt = m_seibu_sound->get_custom_decrypt();
	memcpy(decrypt,        memregion("audiocpu")->base()+0x2000, 0x2000);
	memcpy(decrypt+0x8000, memregion("audiocpu")->base()+0x8000, 0x8000);
	m_adpcm1->decrypt("adpcm1");
	m_adpcm2->decrypt("adpcm2");
}


GAME( 1988, cabal,   0,     cabal,   cabalj, cabal_state,   cabal,   ROT0, "TAD Corporation", "Cabal (World, Joystick version)", GAME_SUPPORTS_SAVE )
GAME( 1989, cabala,  cabal, cabal,   cabalj, cabal_state,   cabal,   ROT0, "TAD Corporation (Alpha Trading license)", "Cabal (Alpha Trading)", GAME_SUPPORTS_SAVE ) // korea?
GAME( 1988, cabalbl, cabal, cabalbl, cabalbl, driver_device,  0,       ROT0, "bootleg (Red Corporation)", "Cabal (bootleg of Joystick version, set 1, alternate sound hardware)", GAME_IMPERFECT_SOUND | GAME_SUPPORTS_SAVE )
GAME( 1988, cabalbl2,cabal, cabalbl2,cabalj, cabal_state,   cabalbl2,ROT0, "bootleg", "Cabal (bootleg of Joystick version, set 2)", GAME_SUPPORTS_SAVE )

GAME( 1988, cabalus, cabal, cabal,   cabalt, cabal_state,   cabal,  ROT0, "TAD Corporation (Fabtek license)", "Cabal (US set 1, Trackball version)", GAME_SUPPORTS_SAVE )
GAME( 1988, cabalus2,cabal, cabal,   cabalt, cabal_state,   cabal,  ROT0, "TAD Corporation (Fabtek license)", "Cabal (US set 2, Trackball version)", GAME_SUPPORTS_SAVE )
