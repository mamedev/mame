// license:BSD-3-Clause
// copyright-holders:Brad Oliver
/***************************************************************************

Jack the Giant Killer memory map (preliminary)

driver by Brad Oliver


Main CPU
--------
0000-3fff  ROM
4000-5fff  RAM
b000-b07f  sprite ram
b400       command for sound CPU
b500-b505  input ports
b506       screen flip off
b507       screen flip on
b600-b61f  palette ram
b800-bbff  video ram
bc00-bfff  color ram
c000-ffff  More ROM

Sound CPU (appears to run in interrupt mode 1)
---------
0000-0fff  ROM
1000-1fff  ROM (Zzyzzyxx only)
4000-43ff  RAM
6000-6fff  R/C filter ???

I/O
---
0x40: Read - ay-8910 port 0
      Write - ay-8910 write
0x80: Write - ay-8910 control

The 2 ay-8910 read ports are responsible for reading the sound commands.


Notes:
  - "Jack to Mame no Ki (Jack the Giant Killer)" and
    "Pro Billiard (Tri Pool)" is developed by Noma Trading
    (distributed via SNK).  Hara Industries probably a bootlegger.

Todo:
  - fix striv hanging notes
  - fix tripool palette problems (see attract)
    The tripool driver used to have a hack making the vblank interrupt go off
    twice per frame, this made the game run way too fast, but no palette bug.
  - what's the correct irq0 frequency of joinem/unclepoo/loverboy?
  - some remaining unknown memorymap writes


****************************************************************************

Stephh's Notes:

  'unclepoo'

  SYSTEM bit 7 is sort of "freeze", but it doesn't seem to have any effect when playing
  (only during boot up sequence - unsure about attract mode)

  DSW1 bit 5 is "Bonus Lives" :
    - when Off (0x00), you get an extra life EVERY 30000 points
    - When On  (0x20), you get an extra life at 30000 points ONLY

  DSW1 bits 6 and 7 might be used for difficulty (to be confirmed)

  DSW2 bit 0 is the "Cabinet" Dip Switch :
    - when Off (0x00), cabinet is cocktail
    - When On  (0x01), cabinet is upright
  This affects write to 0xb700 (bit 7) and reads from 0xb506 and 0xb507 ...

  DSW2 bit 7 overwrites the number of lives :
    - When Off (0x00), lives are based on DSW1 bit 4
    - When On  (0x80), lives are set to 255 (0xff) but they are NOT infinite

  Other bits from DSW2 (but bit 5) don't seem to be read / tested at all ...


***************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "sound/ay8910.h"
#include "includes/jack.h"



/*************************************
 *
 *  Memory handlers
 *
 *************************************/

READ8_MEMBER(jack_state::timer_r)
{
	/* wrong! there should be no need for timer_rate, the same function */
	/* should work for both games */
	return m_audiocpu->total_cycles() / m_timer_rate;
}

WRITE8_MEMBER(jack_state::jack_sh_command_w)
{
	soundlatch_byte_w(space, 0, data);
	m_audiocpu->set_input_line(0, HOLD_LINE);
}


/***************************************************************/

READ8_MEMBER(jack_state::striv_question_r)
{
	// Set-up the remap table for every 16 bytes
	if ((offset & 0xc00) == 0x800)
	{
		m_remap_address[offset & 0x0f] = (offset & 0xf0) >> 4;
	}
	// Select which rom to read and the high 5 bits of address
	else if ((offset & 0xc00) == 0xc00)
	{
		m_question_rom = offset & 7;
		m_question_address = (offset & 0xf8) << 7;
	}
	// Read the actual byte from question roms
	else
	{
		UINT8 *ROM = memregion("user1")->base();
		int real_address;

		real_address = m_question_address | (offset & 0x3f0) | m_remap_address[offset & 0x0f];

		// Check if it wants to read from the upper 8 roms or not
		if (offset & 0x400)
			real_address |= 0x8000 * (m_question_rom + 8);
		else
			real_address |= 0x8000 * m_question_rom;

		return ROM[real_address];
	}

	return 0; // the value read from the configuration reads is discarded
}


WRITE8_MEMBER(jack_state::joinem_control_w)
{
	// d0: related to test mode?
	// d1: unused?
	// d2: ?

	// d3-d4: palette bank
	int palette_bank = data & (m_palette->entries() - 1) >> 3 & 0x18;
	if (m_joinem_palette_bank != palette_bank)
	{
		m_joinem_palette_bank = palette_bank;
		m_bg_tilemap->mark_all_dirty();
	}

	// d5: assume nmi enable
	m_joinem_nmi_enable = data & 0x20;

	// d6: unused?

	// d7: flip screen
	flip_screen_set(data & 0x80);
}



/*************************************
 *
 *  Address maps
 *
 *************************************/

static ADDRESS_MAP_START( jack_map, AS_PROGRAM, 8, jack_state )
	AM_RANGE(0x0000, 0x3fff) AM_ROM
	AM_RANGE(0x4000, 0x5fff) AM_RAM
	AM_RANGE(0xb000, 0xb07f) AM_RAM AM_SHARE("spriteram")
	AM_RANGE(0xb400, 0xb400) AM_WRITE(jack_sh_command_w)
	AM_RANGE(0xb500, 0xb500) AM_READ_PORT("DSW1")
	AM_RANGE(0xb501, 0xb501) AM_READ_PORT("DSW2")
	AM_RANGE(0xb502, 0xb502) AM_READ_PORT("IN0")
	AM_RANGE(0xb503, 0xb503) AM_READ_PORT("IN1")
	AM_RANGE(0xb504, 0xb504) AM_READ_PORT("IN2")
	AM_RANGE(0xb505, 0xb505) AM_READ_PORT("IN3")
	AM_RANGE(0xb506, 0xb507) AM_READWRITE(jack_flipscreen_r, jack_flipscreen_w)
	AM_RANGE(0xb600, 0xb61f) AM_DEVWRITE("palette", palette_device, write) AM_SHARE("palette")
	AM_RANGE(0xb800, 0xbbff) AM_RAM_WRITE(jack_videoram_w) AM_SHARE("videoram")
	AM_RANGE(0xbc00, 0xbfff) AM_RAM_WRITE(jack_colorram_w) AM_SHARE("colorram")
	AM_RANGE(0xc000, 0xffff) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START( decrypted_opcodes_map, AS_DECRYPTED_OPCODES, 8, jack_state )
	AM_RANGE(0x0000, 0x3fff) AM_ROM AM_SHARE("decrypted_opcodes")
ADDRESS_MAP_END

static ADDRESS_MAP_START( striv_map, AS_PROGRAM, 8, jack_state )
	AM_RANGE(0xb000, 0xb0ff) AM_WRITENOP
	AM_RANGE(0xc000, 0xcfff) AM_READ(striv_question_r)
	AM_IMPORT_FROM( jack_map )
ADDRESS_MAP_END


static ADDRESS_MAP_START( joinem_map, AS_PROGRAM, 8, jack_state )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0x8000, 0x8fff) AM_RAM
	AM_RANGE(0xb000, 0xb07f) AM_RAM AM_SHARE("spriteram")
	AM_RANGE(0xb080, 0xb0ff) AM_RAM_WRITE(joinem_scroll_w) AM_SHARE("scrollram")
	AM_RANGE(0xb400, 0xb400) AM_WRITE(jack_sh_command_w)
	AM_RANGE(0xb500, 0xb500) AM_READ_PORT("DSW1")
	AM_RANGE(0xb501, 0xb501) AM_READ_PORT("DSW2")
	AM_RANGE(0xb502, 0xb502) AM_READ_PORT("IN0")
	AM_RANGE(0xb503, 0xb503) AM_READ_PORT("IN1")
	AM_RANGE(0xb504, 0xb504) AM_READ_PORT("IN2")
	AM_RANGE(0xb506, 0xb507) AM_READWRITE(jack_flipscreen_r, jack_flipscreen_w)
	AM_RANGE(0xb700, 0xb700) AM_WRITE(joinem_control_w)
	AM_RANGE(0xb800, 0xbbff) AM_RAM_WRITE(jack_videoram_w) AM_SHARE("videoram")
	AM_RANGE(0xbc00, 0xbfff) AM_RAM_WRITE(jack_colorram_w) AM_SHARE("colorram")
ADDRESS_MAP_END

static ADDRESS_MAP_START( unclepoo_map, AS_PROGRAM, 8, jack_state )
	AM_RANGE(0x9000, 0x97ff) AM_RAM
	AM_IMPORT_FROM( joinem_map )
ADDRESS_MAP_END


static ADDRESS_MAP_START( sound_map, AS_PROGRAM, 8, jack_state )
	AM_RANGE(0x0000, 0x1fff) AM_ROM
	AM_RANGE(0x4000, 0x43ff) AM_RAM
	AM_RANGE(0x6000, 0x6fff) AM_WRITENOP  /* R/C filter ??? */
ADDRESS_MAP_END

static ADDRESS_MAP_START( sound_io_map, AS_IO, 8, jack_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x40, 0x40) AM_DEVREADWRITE("aysnd", ay8910_device, data_r, data_w)
	AM_RANGE(0x80, 0x80) AM_DEVWRITE("aysnd", ay8910_device, address_w)
ADDRESS_MAP_END



/*************************************
 *
 *  Input ports
 *
 *************************************/

static INPUT_PORTS_START( jack )
	PORT_START("DSW1")
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Coin_B ) )       PORT_DIPLOCATION("SW1:!1,!2")
	PORT_DIPSETTING(    0x01, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 4C_3C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_3C ) )
	PORT_DIPNAME( 0x0c, 0x00, DEF_STR( Coin_A ) )       PORT_DIPLOCATION("SW1:!3,!4")
	PORT_DIPSETTING(    0x08, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 4C_3C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Lives ) )        PORT_DIPLOCATION("SW1:!5")
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x10, "5" )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Bonus_Life ) )   PORT_DIPLOCATION("SW1:!6")
	PORT_DIPSETTING(    0x00, "Every 10000" )
	PORT_DIPSETTING(    0x20, "10000 Only" )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SW1:!7")
	PORT_DIPSETTING(    0x00, "Start on Level 1" )
	PORT_DIPSETTING(    0x40, "Start on Level 13" )
	PORT_DIPNAME( 0x80, 0x00, "Per Bean/Bullets" )      PORT_DIPLOCATION("SW1:!8")
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x80, "2" )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Cabinet ) )      PORT_DIPLOCATION("SW2:!1")
	PORT_DIPSETTING(    0x01, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ) )
	PORT_DIPUNKNOWN_DIPLOC( 0x02, 0x00, "SW2:!2" )
	PORT_DIPUNKNOWN_DIPLOC( 0x04, 0x00, "SW2:!3" )
	PORT_DIPUNKNOWN_DIPLOC( 0x08, 0x00, "SW2:!4" )
	PORT_DIPUNKNOWN_DIPLOC( 0x10, 0x00, "SW2:!5" )
	PORT_SERVICE( 0x20, IP_ACTIVE_HIGH )                PORT_DIPLOCATION("SW2:!6")
	PORT_DIPNAME( 0x40, 0x00, "Invulnerability (Cheat)")    PORT_DIPLOCATION("SW2:!7")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, "255 Lives (Cheat)")      PORT_DIPLOCATION("SW2:!8")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x1c, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP    ) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN  ) PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT  ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP    ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN  ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT  ) PORT_8WAY PORT_COCKTAIL

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON2 )
	PORT_BIT( 0xfc, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN3")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_COCKTAIL
	PORT_BIT( 0xfc, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END


/* Same as 'jack', but different coinage */
static INPUT_PORTS_START( jack2 )
	PORT_INCLUDE( jack )

	PORT_MODIFY("DSW1")
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Coin_B ) )       PORT_DIPLOCATION("SW1:!1,!2")
	PORT_DIPSETTING(    0x03, DEF_STR( 4C_3C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_3C ) )
	PORT_DIPNAME( 0x0c, 0x00, DEF_STR( Coin_A ) )       PORT_DIPLOCATION("SW1:!3,!4")
	PORT_DIPSETTING(    0x08, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 4C_3C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
INPUT_PORTS_END


/* Same as 'jack', but another different coinage */
static INPUT_PORTS_START( jack3 )
	PORT_INCLUDE( jack )

	PORT_MODIFY("DSW1")
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Coin_B ) )       PORT_DIPLOCATION("SW1:!1,!2")
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_5C ) )
	PORT_DIPNAME( 0x0c, 0x00, DEF_STR( Coin_A ) )       PORT_DIPLOCATION("SW1:!3,!4")
	PORT_DIPSETTING(    0x0c, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
INPUT_PORTS_END


/* Same as 'jack', but different "Bullets per Bean Collected" and "Difficulty" Dip Switches */
static INPUT_PORTS_START( treahunt )
	PORT_INCLUDE( jack )

	PORT_MODIFY("DSW1")
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SW1:!7")
	PORT_DIPSETTING(    0x00, "Start on Level 1" )
	PORT_DIPSETTING(    0x40, "Start on Level 6" )
	PORT_DIPNAME( 0x80, 0x00, "Per Bean/Bullets" )      PORT_DIPLOCATION("SW1:!8")
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPSETTING(    0x80, "20" )
INPUT_PORTS_END


static INPUT_PORTS_START( zzyzzyxx )
	PORT_START("DSW1")
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Coinage ) )      PORT_DIPLOCATION("SW1:!1,!2")
	PORT_DIPSETTING(    0x01, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 4C_3C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_3C ) )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Lives ) )        PORT_DIPLOCATION("SW1:!3")
	PORT_DIPSETTING(    0x04, "2" )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPNAME( 0x08, 0x00, "2 Credits on Reset" )    PORT_DIPLOCATION("SW1:!4")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SW1:!5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Cabinet ) )      PORT_DIPLOCATION("SW1:!6")
	PORT_DIPSETTING(    0x20, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ) )
	PORT_SERVICE( 0x40, IP_ACTIVE_HIGH )                PORT_DIPLOCATION("SW1:!7")
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Free_Play ) )    PORT_DIPLOCATION("SW1:!8")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Bonus_Life ) )   PORT_DIPLOCATION("SW2:!1,!2")
	PORT_DIPSETTING(    0x02, DEF_STR( None ) )
	PORT_DIPSETTING(    0x00, "10000 50000" )
	PORT_DIPSETTING(    0x01, "25000 100000" )
	PORT_DIPSETTING(    0x03, "100000 300000" )
	PORT_DIPNAME( 0x04, 0x04, "2nd Bonus Given" )       PORT_DIPLOCATION("SW2:!3")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x08, 0x00, "Starting Laps" )         PORT_DIPLOCATION("SW2:!4")
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPSETTING(    0x08, "3" )
	PORT_DIPNAME( 0x10, 0x00, "Difficulty of Pleasing Lola" )   PORT_DIPLOCATION("SW2:!5")
	PORT_DIPSETTING(    0x00, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Hard ) )
	PORT_DIPNAME( 0x20, 0x00, "Show Intermissions" )    PORT_DIPLOCATION("SW2:!6")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Yes ) )
	PORT_DIPNAME( 0xc0, 0x40, "Extra Lives" )           PORT_DIPLOCATION("SW2:!7,!8")
	PORT_DIPSETTING(    0x00, "3 under 4000 pts" )
	PORT_DIPSETTING(    0x80, "5 under 4000 pts" )
	PORT_DIPSETTING(    0x40, DEF_STR( None ) )         // 3 under 0 pts
//  PORT_DIPSETTING(    0xc0, DEF_STR( None ) )         // 5 under 0 pts

	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x1c, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0xc0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP   ) PORT_2WAY
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_2WAY
	PORT_BIT( 0x0c, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP   ) PORT_2WAY PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_2WAY PORT_COCKTAIL
	PORT_BIT( 0xc0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0xfe, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN3")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0xfe, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END


static INPUT_PORTS_START( freeze )
	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Flip_Screen ) )  PORT_DIPLOCATION("SW1:!1")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_SERVICE( 0x02, IP_ACTIVE_HIGH )                PORT_DIPLOCATION("SW1:!2")
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SW1:!3")
	PORT_DIPSETTING(    0x00, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Hard ) )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Lives ) )        PORT_DIPLOCATION("SW1:!4")
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x08, "5" )
	PORT_DIPNAME( 0x30, 0x00, DEF_STR( Bonus_Life ) )   PORT_DIPLOCATION("SW1:!5,!6")
	PORT_DIPSETTING(    0x00, "10000" )
	PORT_DIPSETTING(    0x10, "10000 & Every 40000" )
	PORT_DIPSETTING(    0x20, "10000 & Every 60000" )
	PORT_DIPSETTING(    0x30, "20000 & Every 100000" )
	PORT_DIPNAME( 0xc0, 0x00, DEF_STR( Coinage ) )      PORT_DIPLOCATION("SW1:!7,!8")
	PORT_DIPSETTING(    0x80, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( Free_Play ) )

	PORT_START("DSW2")
	PORT_DIPUNKNOWN_DIPLOC( 0x01, 0x00, "SW2:!1" )
	PORT_DIPUNKNOWN_DIPLOC( 0x02, 0x00, "SW2:!2" )
	PORT_DIPUNKNOWN_DIPLOC( 0x04, 0x00, "SW2:!3" )
	PORT_DIPUNKNOWN_DIPLOC( 0x08, 0x00, "SW2:!4" )
	PORT_DIPUNKNOWN_DIPLOC( 0x10, 0x00, "SW2:!5" )
	PORT_DIPUNKNOWN_DIPLOC( 0x20, 0x00, "SW2:!6" )
	PORT_DIPUNKNOWN_DIPLOC( 0x40, 0x00, "SW2:!7" )
	PORT_DIPUNKNOWN_DIPLOC( 0x80, 0x00, "SW2:!8" )

	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_2WAY
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT  ) PORT_2WAY
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON2 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("IN3")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )
INPUT_PORTS_END


static INPUT_PORTS_START( sucasino )
	PORT_START("DSW1")
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Coinage ) )      PORT_DIPLOCATION("SW1:!1,!2")
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_4C ) )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Cabinet ) )      PORT_DIPLOCATION("SW1:!3")
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Flip_Screen ) )  PORT_DIPLOCATION("SW1:!4")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPUNKNOWN_DIPLOC( 0x10, 0x00, "SW1:!5" )      // bit 5-8, Check code at 0xf700
	PORT_DIPUNKNOWN_DIPLOC( 0x20, 0x00, "SW1:!6" )
	PORT_DIPUNKNOWN_DIPLOC( 0x40, 0x00, "SW1:!7" )
	PORT_DIPUNKNOWN_DIPLOC( 0x80, 0x00, "SW1:!8" )

	PORT_START("DSW2")
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x3c, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN1")
	PORT_BIT( 0x03, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_2WAY
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT  ) PORT_2WAY
	PORT_BIT( 0x30, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_2WAY PORT_COCKTAIL
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT  ) PORT_2WAY PORT_COCKTAIL

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0xfe, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN3")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0xfe, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END


static INPUT_PORTS_START( tripool )
	PORT_START("DSW1")
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Coin_B ) )       PORT_DIPLOCATION("SW1:!1,!2")
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_5C ) )
	PORT_DIPNAME( 0x0c, 0x00, DEF_STR( Coin_A ) )       PORT_DIPLOCATION("SW1:!3,!4")
	PORT_DIPSETTING(    0x0c, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPUNKNOWN_DIPLOC( 0x10, 0x00, "SW1:!5" )
	PORT_DIPUNKNOWN_DIPLOC( 0x20, 0x00, "SW1:!6" )
	PORT_DIPUNKNOWN_DIPLOC( 0x40, 0x00, "SW1:!7" )
	PORT_DIPUNKNOWN_DIPLOC( 0x80, 0x00, "SW1:!8" )

	PORT_START("DSW2")
	PORT_DIPUNKNOWN_DIPLOC( 0x01, 0x00, "SW2:!1" )
	PORT_DIPUNKNOWN_DIPLOC( 0x02, 0x00, "SW2:!2" )
	PORT_DIPUNKNOWN_DIPLOC( 0x04, 0x00, "SW2:!3" )
	PORT_DIPUNKNOWN_DIPLOC( 0x08, 0x00, "SW2:!4" )
	PORT_DIPUNKNOWN_DIPLOC( 0x10, 0x00, "SW2:!5" )
	PORT_DIPUNKNOWN_DIPLOC( 0x20, 0x00, "SW2:!6" )
	PORT_DIPUNKNOWN_DIPLOC( 0x40, 0x00, "SW2:!7" )
	PORT_DIPUNKNOWN_DIPLOC( 0x80, 0x00, "SW2:!8" )

	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_NAME ("Select Game 1")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_NAME ("Select Game 2")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON5 ) PORT_NAME ("Select Game 3")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP    ) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN  ) PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT  ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP    ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN  ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT  ) PORT_8WAY PORT_COCKTAIL

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON2 ) // not needed?
	PORT_BIT( 0xfc, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN3")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_COCKTAIL  // not needed?
	PORT_BIT( 0xfc, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END


static INPUT_PORTS_START( striv )
	PORT_START("DSW1")
	PORT_DIPNAME( 0x02, 0x00, "Monitor" )               PORT_DIPLOCATION("SW1:!2")
	PORT_DIPSETTING(    0x02, "Horizontal" )
	PORT_DIPSETTING(    0x00, "Vertical" )
	PORT_DIPNAME( 0x05, 0x05, "Gaming Option Number" )  PORT_DIPLOCATION("SW1:!1,!3")
	PORT_DIPSETTING(    0x01, "2" ) PORT_CONDITION("DSW1", 0x20, EQUALS, 0x20)
	PORT_DIPSETTING(    0x05, "3" ) PORT_CONDITION("DSW1", 0x20, EQUALS, 0x20)
	PORT_DIPSETTING(    0x00, "4" ) PORT_CONDITION("DSW1", 0x20, EQUALS, 0x20)
	PORT_DIPSETTING(    0x04, "5" ) PORT_CONDITION("DSW1", 0x20, EQUALS, 0x20)
	PORT_DIPSETTING(    0x01, "4" ) PORT_CONDITION("DSW1", 0x20, NOTEQUALS, 0x20)
	PORT_DIPSETTING(    0x05, "5" ) PORT_CONDITION("DSW1", 0x20, NOTEQUALS, 0x20)
	PORT_DIPSETTING(    0x00, "6" ) PORT_CONDITION("DSW1", 0x20, NOTEQUALS, 0x20)
	PORT_DIPSETTING(    0x04, "7" ) PORT_CONDITION("DSW1", 0x20, NOTEQUALS, 0x20)
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Cabinet ) )      PORT_DIPLOCATION("SW1:!4")
	PORT_DIPSETTING(    0x08, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Coinage ) )      PORT_DIPLOCATION("SW1:!5")
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 1C_1C ) )
	PORT_DIPNAME( 0x20, 0x20, "Gaming Option" )         PORT_DIPLOCATION("SW1:!6")
	PORT_DIPSETTING(    0x20, "Number of Wrong Answer" )
	PORT_DIPSETTING(    0x00, "Number of Questions" )
	PORT_DIPNAME( 0x40, 0x40, "Show Correct Answer" )   PORT_DIPLOCATION("SW1:!7")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Yes ) )
	PORT_SERVICE( 0x80, IP_ACTIVE_LOW )                 PORT_DIPLOCATION("SW1:!8")

	PORT_START("DSW2")
	PORT_DIPUNKNOWN_DIPLOC( 0x01, 0x00, "SW2:!1" )
	PORT_DIPUNKNOWN_DIPLOC( 0x02, 0x00, "SW2:!2" )
	PORT_DIPUNKNOWN_DIPLOC( 0x04, 0x00, "SW2:!3" )
	PORT_DIPUNKNOWN_DIPLOC( 0x08, 0x00, "SW2:!4" )
	PORT_DIPUNKNOWN_DIPLOC( 0x10, 0x00, "SW2:!5" )
	PORT_DIPUNKNOWN_DIPLOC( 0x20, 0x00, "SW2:!6" )
	PORT_DIPUNKNOWN_DIPLOC( 0x40, 0x00, "SW2:!7" )
	PORT_DIPUNKNOWN_DIPLOC( 0x80, 0x00, "SW2:!8" )

	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON2 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON3 )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON4 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN2")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN1 ) PORT_IMPULSE(2)
	PORT_BIT( 0xfd, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN3")
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNUSED ) //?
INPUT_PORTS_END


static INPUT_PORTS_START( joinem )
	PORT_START("DSW1")
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Coin_A ) )       PORT_DIPLOCATION("SW1:!1,!2")
	PORT_DIPSETTING(    0x01, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 4C_3C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_3C ) )
	PORT_DIPNAME( 0x0c, 0x00, DEF_STR( Coin_B ) )       PORT_DIPLOCATION("SW1:!3,!4")
	PORT_DIPSETTING(    0x08, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 4C_3C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Lives ) )        PORT_DIPLOCATION("SW1:!5")
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPSETTING(    0x10, "5" )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Bonus_Life ) )   PORT_DIPLOCATION("SW1:!6")
	PORT_DIPSETTING(    0x00, "Every 30000" )
	PORT_DIPSETTING(    0x20, "30000 Only" )
	PORT_DIPUNKNOWN_DIPLOC( 0x40, 0x00, "SW1:!7" )
	PORT_DIPUNKNOWN_DIPLOC( 0x80, 0x00, "SW1:!8" )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Cabinet ) )      PORT_DIPLOCATION("SW2:!1")
	PORT_DIPSETTING(    0x01, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ) )
	PORT_DIPUNKNOWN_DIPLOC( 0x02, 0x00, "SW2:!2" )
	PORT_DIPUNKNOWN_DIPLOC( 0x04, 0x00, "SW2:!3" )
	PORT_DIPUNKNOWN_DIPLOC( 0x08, 0x00, "SW2:!4" )
	PORT_DIPUNKNOWN_DIPLOC( 0x10, 0x00, "SW2:!5" )
	PORT_SERVICE_DIPLOC( 0x20, IP_ACTIVE_HIGH, "SW2:!6" )
	PORT_DIPUNKNOWN_DIPLOC( 0x40, 0x00, "SW2:!7" )
	PORT_DIPNAME( 0x80, 0x00, "255 Lives (Cheat)" )     PORT_DIPLOCATION("SW2:!8")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP    ) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN  ) PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT  ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNKNOWN ) // 1S in testmode
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNKNOWN ) // 1J "
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN ) // X6 "
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN ) // X7 "

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP    ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN  ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT  ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNKNOWN ) // 2S in testmode
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNKNOWN ) // 2J "
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN ) // Y6 "
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN ) // Y7 "

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNKNOWN ) // Z4 in testmode
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNKNOWN ) // Z5 "
	PORT_BIT( 0x40, IP_ACTIVE_LOW,  IPT_UNKNOWN ) // Z6 ", locks up at boot if low?
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN ) // ?
INPUT_PORTS_END


static INPUT_PORTS_START( unclepoo )
	PORT_INCLUDE( joinem )

	PORT_MODIFY("IN0")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON2 )

	PORT_MODIFY("IN1")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_COCKTAIL
INPUT_PORTS_END


static INPUT_PORTS_START( loverboy )
	PORT_INCLUDE( joinem )

	PORT_MODIFY("DSW1")
	PORT_DIPNAME( 0x0f, 0x00, DEF_STR( Coinage ) )      PORT_DIPLOCATION("SW1:!1,!2,!3,!4")
	PORT_DIPSETTING(    0x0c, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x0d, DEF_STR( 4C_2C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x09, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(    0x0e, DEF_STR( 4C_3C ) )
	PORT_DIPSETTING(    0x0f, DEF_STR( 4C_4C ) )
	PORT_DIPSETTING(    0x0a, DEF_STR( 3C_3C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 2C_2C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x0b, DEF_STR( 3C_4C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 2C_4C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_4C ) )
	PORT_DIPUNKNOWN_DIPLOC( 0x10, 0x00, "SW1:!5" )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Bonus_Life ) )   PORT_DIPLOCATION("SW1:!6")
	PORT_DIPSETTING(    0x00, "20000" )
	PORT_DIPSETTING(    0x20, "30000" )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Lives ) )        PORT_DIPLOCATION("SW1:!7")
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x40, "5" )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Cabinet ) )      PORT_DIPLOCATION("SW1:!8")
	PORT_DIPSETTING(    0x80, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ) )

	PORT_MODIFY("DSW2")
	PORT_DIPUNKNOWN_DIPLOC( 0x01, 0x00, "SW2:!1" )
	PORT_DIPUNKNOWN_DIPLOC( 0x02, 0x00, "SW2:!2" )
	PORT_DIPUNKNOWN_DIPLOC( 0x04, 0x00, "SW2:!3" )
	PORT_DIPUNKNOWN_DIPLOC( 0x08, 0x00, "SW2:!4" )
	PORT_DIPUNKNOWN_DIPLOC( 0x10, 0x00, "SW2:!5" )
	PORT_DIPUNKNOWN_DIPLOC( 0x20, 0x00, "SW2:!6" )
	PORT_DIPUNKNOWN_DIPLOC( 0x40, 0x00, "SW2:!7" )
	PORT_DIPUNKNOWN_DIPLOC( 0x80, 0x00, "SW2:!8" )

	PORT_MODIFY("IN0")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 )

	PORT_MODIFY("IN1")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_COCKTAIL
INPUT_PORTS_END



/*************************************
 *
 *  Machine configs
 *
 *************************************/

static const gfx_layout charlayout =
{
	8,8,
	RGN_FRAC(1,2),
	2,
	{ RGN_FRAC(0,2), RGN_FRAC(1,2) },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8
};

static GFXDECODE_START( jack )
	GFXDECODE_ENTRY( "gfx1", 0, charlayout, 0, 8 )
GFXDECODE_END


static const gfx_layout joinem_charlayout =
{
	8,8,
	RGN_FRAC(1,3),
	3,
	{ RGN_FRAC(0,3),RGN_FRAC(1,3),RGN_FRAC(2,3) },
	{ 0,1,2,3,4,5,6,7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8
};

static GFXDECODE_START( joinem )
	GFXDECODE_ENTRY( "gfx1", 0, joinem_charlayout, 0, 32 )
GFXDECODE_END

/***************************************************************/

void jack_state::machine_start()
{
}

void jack_state::machine_reset()
{
}


MACHINE_START_MEMBER(jack_state,striv)
{
	save_item(NAME(m_question_address));
	save_item(NAME(m_question_rom));
	save_item(NAME(m_remap_address));
}

MACHINE_RESET_MEMBER(jack_state,striv)
{
	m_question_address = 0;
	m_question_rom = 0;

	for (int i = 0; i < 16; i++)
		m_remap_address[i] = 0;
}


MACHINE_START_MEMBER(jack_state,joinem)
{
	m_joinem_palette_bank = 0;

	save_item(NAME(m_joinem_nmi_enable));
	save_item(NAME(m_joinem_palette_bank));
}

MACHINE_RESET_MEMBER(jack_state,joinem)
{
	joinem_control_w(m_maincpu->space(AS_PROGRAM), 0, 0, 0xff);
}


/***************************************************************/

static MACHINE_CONFIG_START( jack, jack_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, XTAL_18MHz/6)
	MCFG_CPU_PROGRAM_MAP(jack_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", jack_state, irq0_line_hold)

	MCFG_CPU_ADD("audiocpu", Z80, XTAL_18MHz/6)
	MCFG_CPU_PROGRAM_MAP(sound_map)
	MCFG_CPU_IO_MAP(sound_io_map)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(32*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 2*8, 30*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(jack_state, screen_update_jack)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", jack)

	MCFG_PALETTE_ADD("palette", 32)
	MCFG_PALETTE_FORMAT(BBGGGRRR_inverted)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("aysnd", AY8910, XTAL_18MHz/12)
	MCFG_AY8910_PORT_A_READ_CB(READ8(driver_device, soundlatch_byte_r))
	MCFG_AY8910_PORT_B_READ_CB(READ8(jack_state, timer_r))
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( treahunt, jack )
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_DECRYPTED_OPCODES_MAP(decrypted_opcodes_map)
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( striv, jack )

	/* basic machine hardware */
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(striv_map)

	MCFG_MACHINE_START_OVERRIDE(jack_state,striv)
	MCFG_MACHINE_RESET_OVERRIDE(jack_state,striv)

	/* video hardware */
	MCFG_SCREEN_MODIFY("screen")
	MCFG_SCREEN_UPDATE_DRIVER(jack_state, screen_update_striv)
MACHINE_CONFIG_END


/***************************************************************/

INTERRUPT_GEN_MEMBER(jack_state::joinem_vblank_irq)
{
	if (m_joinem_nmi_enable)
		device.execute().set_input_line(INPUT_LINE_NMI, PULSE_LINE);
}

static MACHINE_CONFIG_DERIVED( joinem, jack )

	/* basic machine hardware */
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(joinem_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", jack_state, joinem_vblank_irq)
	MCFG_CPU_PERIODIC_INT_DRIVER(jack_state, irq0_line_hold, 250) // ??? controls game speed

	MCFG_MACHINE_START_OVERRIDE(jack_state,joinem)
	MCFG_MACHINE_RESET_OVERRIDE(jack_state,joinem)

	/* video hardware */
	MCFG_SCREEN_MODIFY("screen")
	MCFG_SCREEN_UPDATE_DRIVER(jack_state, screen_update_joinem)

	MCFG_GFXDECODE_MODIFY("gfxdecode", joinem)

	MCFG_DEVICE_REMOVE("palette")
	MCFG_PALETTE_ADD("palette", 64)
	MCFG_PALETTE_INIT_OWNER(jack_state, joinem)

	MCFG_VIDEO_START_OVERRIDE(jack_state,joinem)
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( unclepoo, joinem )

	/* basic machine hardware */
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(unclepoo_map)

	/* video hardware */
	MCFG_SCREEN_MODIFY("screen")
	MCFG_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 1*8, 31*8-1)

	MCFG_PALETTE_MODIFY("palette")
	MCFG_PALETTE_ENTRIES(256)
MACHINE_CONFIG_END



/*************************************
 *
 *  ROM definition(s)
 *
 *************************************/

ROM_START( jack )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "j8",           0x0000, 0x1000, CRC(c8e73998) SHA1(1332c8dee99d07cc2823797ecc3551d720428b36) )
	ROM_LOAD( "jgk.j6",       0x1000, 0x1000, CRC(36d7810e) SHA1(b8757222586eb6aa31fc3b1d1fd00ddb1c68cb0b) )
	ROM_LOAD( "jgk.j7",       0x2000, 0x1000, CRC(b15ff3ee) SHA1(fa99b4c2d96fb355ff8ba12c2f40ee4d00bb04da) )
	ROM_LOAD( "jgk.j5",       0x3000, 0x1000, CRC(4a63d242) SHA1(afecfb515144963eb819a58ef3b368c20e6fc4ff) )
	ROM_LOAD( "jgk.j3",       0xc000, 0x1000, CRC(605514a8) SHA1(74769053a977cea0324b1198e582f8e712af9a22) )
	ROM_LOAD( "jgk.j4",       0xd000, 0x1000, CRC(bce489b7) SHA1(8c1bb82f38f1757b08c99230454a6e7eca8709f3) )
	ROM_LOAD( "jgk.j2",       0xe000, 0x1000, CRC(db21bd55) SHA1(5518c34d381129c7940de85c476639cafd0e5025) )
	ROM_LOAD( "jgk.j1",       0xf000, 0x1000, CRC(49fffe31) SHA1(b5a0a7d021c8001368bb5d3b41a728734eb50ac5) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "jgk.j9",       0x0000, 0x1000, CRC(c2dc1e00) SHA1(57e8abf5a5eb3f5a22e206ee2562b64ea0ba2d05) )

	ROM_REGION( 0x4000, "gfx1", 0 )
	ROM_LOAD( "jgk.j12",      0x0000, 0x1000, CRC(ce726df0) SHA1(d0b83c5ceb558dafb6387445d5cfb4668f2f4386) )
	ROM_LOAD( "jgk.j13",      0x1000, 0x1000, CRC(6aec2c8d) SHA1(f81c44e79e18a864abfeb8769f012a6e93679164) )
	ROM_LOAD( "jgk.j11",      0x2000, 0x1000, CRC(fd14c525) SHA1(5e6a8274d008c5dd276aaf85f7f943810b5ac987) )
	ROM_LOAD( "jgk.j10",      0x3000, 0x1000, CRC(eab890b2) SHA1(a5b83dff6bc6fd51f80db136fad8075262720f01) )
ROM_END


ROM_START( jack2 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "jgk.j8",       0x0000, 0x1000, CRC(fe229e20) SHA1(191cfb7bb08d46cab713e23abd69f27db1685346) )
	ROM_LOAD( "jgk.j6",       0x1000, 0x1000, CRC(36d7810e) SHA1(b8757222586eb6aa31fc3b1d1fd00ddb1c68cb0b) )
	ROM_LOAD( "jgk.j7",       0x2000, 0x1000, CRC(b15ff3ee) SHA1(fa99b4c2d96fb355ff8ba12c2f40ee4d00bb04da) )
	ROM_LOAD( "jgk.j5",       0x3000, 0x1000, CRC(4a63d242) SHA1(afecfb515144963eb819a58ef3b368c20e6fc4ff) )
	ROM_LOAD( "jgk.j3",       0xc000, 0x1000, CRC(605514a8) SHA1(74769053a977cea0324b1198e582f8e712af9a22) )
	ROM_LOAD( "jgk.j4",       0xd000, 0x1000, CRC(bce489b7) SHA1(8c1bb82f38f1757b08c99230454a6e7eca8709f3) )
	ROM_LOAD( "jgk.j2",       0xe000, 0x1000, CRC(db21bd55) SHA1(5518c34d381129c7940de85c476639cafd0e5025) )
	ROM_LOAD( "jgk.j1",       0xf000, 0x1000, CRC(49fffe31) SHA1(b5a0a7d021c8001368bb5d3b41a728734eb50ac5) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "jgk.j9",       0x0000, 0x1000, CRC(c2dc1e00) SHA1(57e8abf5a5eb3f5a22e206ee2562b64ea0ba2d05) )

	ROM_REGION( 0x4000, "gfx1", 0 )
	ROM_LOAD( "jgk.j12",      0x0000, 0x1000, CRC(ce726df0) SHA1(d0b83c5ceb558dafb6387445d5cfb4668f2f4386) )
	ROM_LOAD( "jgk.j13",      0x1000, 0x1000, CRC(6aec2c8d) SHA1(f81c44e79e18a864abfeb8769f012a6e93679164) )
	ROM_LOAD( "jgk.j11",      0x2000, 0x1000, CRC(fd14c525) SHA1(5e6a8274d008c5dd276aaf85f7f943810b5ac987) )
	ROM_LOAD( "jgk.j10",      0x3000, 0x1000, CRC(eab890b2) SHA1(a5b83dff6bc6fd51f80db136fad8075262720f01) )
ROM_END


ROM_START( jack3 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "jack8",        0x0000, 0x1000, CRC(632151d2) SHA1(080f29818d537474c821b9920427bda47f5a7254) )
	ROM_LOAD( "jack6",        0x1000, 0x1000, CRC(f94f80d9) SHA1(2301e6d0b814bf897e5c8ed43a342e3213be0a27) )
	ROM_LOAD( "jack7",        0x2000, 0x1000, CRC(c830ff1e) SHA1(f85b8bf39600212846f0b68012fbdb6b5fd3ad5c) )
	ROM_LOAD( "jack5",        0x3000, 0x1000, CRC(8dea17e7) SHA1(7e70bce78eaa40963ba981c9e7926ee0529898dd) )
	ROM_LOAD( "jgk.j3",       0xc000, 0x1000, CRC(605514a8) SHA1(74769053a977cea0324b1198e582f8e712af9a22) )
	ROM_LOAD( "jgk.j4",       0xd000, 0x1000, CRC(bce489b7) SHA1(8c1bb82f38f1757b08c99230454a6e7eca8709f3) )
	ROM_LOAD( "jgk.j2",       0xe000, 0x1000, CRC(db21bd55) SHA1(5518c34d381129c7940de85c476639cafd0e5025) )
	ROM_LOAD( "jack1",        0xf000, 0x1000, CRC(7e75ea3d) SHA1(9f3b998a8a494d67e3aa8933eb113fa2d2adae61) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "jgk.j9",       0x0000, 0x1000, CRC(c2dc1e00) SHA1(57e8abf5a5eb3f5a22e206ee2562b64ea0ba2d05) )

	ROM_REGION( 0x4000, "gfx1", 0 )
	ROM_LOAD( "jack12",       0x0000, 0x1000, CRC(80320647) SHA1(5e39891033e23256456aad1a3f53cd1e516de51d) )
	ROM_LOAD( "jgk.j13",      0x1000, 0x1000, CRC(6aec2c8d) SHA1(f81c44e79e18a864abfeb8769f012a6e93679164) )
	ROM_LOAD( "jgk.j11",      0x2000, 0x1000, CRC(fd14c525) SHA1(5e6a8274d008c5dd276aaf85f7f943810b5ac987) )
	ROM_LOAD( "jgk.j10",      0x3000, 0x1000, CRC(eab890b2) SHA1(a5b83dff6bc6fd51f80db136fad8075262720f01) )
ROM_END


ROM_START( treahunt )
	ROM_REGION( 0x10000, "maincpu", 0 ) /* 64k for code + 64k for decrypted opcodes */
	ROM_LOAD( "thunt-1.f2",   0x0000, 0x1000, CRC(0b35858c) SHA1(b8f80c69fcbce71e1b85c8f39599f8bebfeb2585) )
	ROM_LOAD( "thunt-2.f3",   0x1000, 0x1000, CRC(67305a51) SHA1(c00b9592c4e146892313e8d32261338957a6a04a) )
	ROM_LOAD( "thunt-3.4f",   0x2000, 0x1000, CRC(d7a969c3) SHA1(7edcbc90836e32aff4a26b0c55a76bbc9bb488fe) )
	ROM_LOAD( "thunt-4.6f",   0x3000, 0x1000, CRC(2483f14d) SHA1(ffb7965433b0caaaae74e8eca19633fcecbdb4f8) )
	ROM_LOAD( "thunt-5.7f",   0xc000, 0x1000, CRC(c69d5e21) SHA1(27b734b2997bc95d04c79b992969db19b743b086) )
	ROM_LOAD( "thunt-6.7e",   0xd000, 0x1000, CRC(11bf3d49) SHA1(6c566aa81568985662461df7bd2386ee72ee3ba7) )
	ROM_LOAD( "thunt-7.6e",   0xe000, 0x1000, CRC(7c2d6279) SHA1(b3dd9875faf9cd91034193794a7b187d79741353) )
	ROM_LOAD( "thunt-8.4e",   0xf000, 0x1000, CRC(f73b86fb) SHA1(7fd4d0876ffee74ec73def085fc845535bb7e451) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "jgk.j9",       0x0000, 0x1000, CRC(c2dc1e00) SHA1(57e8abf5a5eb3f5a22e206ee2562b64ea0ba2d05) )

	ROM_REGION( 0x4000, "gfx1", 0 )
	ROM_LOAD( "thunt-13.a4",  0x0000, 0x1000, CRC(e03f1f09) SHA1(546b270aeeb2d35b718ddd6f15829d4cbe0f7ef6) )
	ROM_LOAD( "thunt-12.a3",  0x1000, 0x1000, CRC(da4ee9eb) SHA1(e01c9cfa426d2b94e6bc976622b888b2ca224771) )
	ROM_LOAD( "thunt-10.a1",  0x2000, 0x1000, CRC(51ec7934) SHA1(f39d99c356d8d9960022fa2c068b5f7206404d85) )
	ROM_LOAD( "thunt-11.a2",  0x3000, 0x1000, CRC(f9781143) SHA1(f168648a78240fdf02063d39f324838f4dfe9a56) )
ROM_END


ROM_START( zzyzzyxx )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "a.2f",         0x0000, 0x1000, CRC(a9102e34) SHA1(80d71df7d235980603f35aa3f474aaf58fb39946) )
	ROM_LOAD( "zzyzzyxx.b",   0x1000, 0x1000, CRC(efa9d4c6) SHA1(aaa66723fed87f1134b59634050d1eb6a83c8159) )
	ROM_LOAD( "zzyzzyxx.c",   0x2000, 0x1000, CRC(b0a365b1) SHA1(67e3c2bab8b2b35c42a986b0ace120724008f555) )
	ROM_LOAD( "zzyzzyxx.d",   0x3000, 0x1000, CRC(5ed6dd9a) SHA1(1279cee868eacefdc26524f2effa7b35f24ec30d) )
	ROM_LOAD( "zzyzzyxx.e",   0xc000, 0x1000, CRC(5966fdbf) SHA1(c1476db9e8508cb71684b568a19ae32c8c0e012a) )
	ROM_LOAD( "f.7e",         0xd000, 0x1000, CRC(12f24c68) SHA1(6d4181d3f044de491d810a3406e9d253d2c669d6) )
	ROM_LOAD( "g.6e",         0xe000, 0x1000, CRC(408f2326) SHA1(fe45084ed50701577eade2da8f4f787ee41d7acf) )
	ROM_LOAD( "h.4e",         0xf000, 0x1000, CRC(f8bbabe0) SHA1(59b2223219712f8a572b2cfbbc14f80ec2b32aae) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "i.5a",         0x0000, 0x1000, CRC(c7742460) SHA1(1dbf0f5be1e2666feef83f256e2993a6c23d7cfc) )
	ROM_LOAD( "j.6a",         0x1000, 0x1000, CRC(72166ccd) SHA1(4f4efcd8ed7f729f4630446607b0e9c93098aa3a) )

	ROM_REGION( 0x4000, "gfx1", 0 )
	ROM_LOAD( "n.1c",         0x0000, 0x1000, CRC(4f64538d) SHA1(1d48f12ff0d1c5604d19338b26e800a91f1be9c1) )
	ROM_LOAD( "m.1d",         0x1000, 0x1000, CRC(217b1402) SHA1(b842b2bde8ff5be6b240ccfb35c7a9f701dab5f4) )
	ROM_LOAD( "k.1b",         0x2000, 0x1000, CRC(b8b2b8cc) SHA1(e149fc91043f3233e10c81358b8624a4bc0baf4e) )
	ROM_LOAD( "l.1a",         0x3000, 0x1000, CRC(ab421a83) SHA1(1cc3e1bcf9e90ffbf7bfeeb0caa8a4f63b34146a) )
ROM_END


ROM_START( zzyzzyxx2 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "a.2f",         0x0000, 0x1000, CRC(a9102e34) SHA1(80d71df7d235980603f35aa3f474aaf58fb39946) )
	ROM_LOAD( "b.3f",         0x1000, 0x1000, CRC(4277beab) SHA1(269338a165286ed44b0fad1873e409f847b8d476) )
	ROM_LOAD( "c.4f",         0x2000, 0x1000, CRC(72ac99e1) SHA1(66b99a0271ae31cf109749159ddd1652b804f077) )
	ROM_LOAD( "d.6f",         0x3000, 0x1000, CRC(7c7eec2b) SHA1(fa62950d9db718069905331140e129711c707775) )
	ROM_LOAD( "e.7f",         0xc000, 0x1000, CRC(cffc4a68) SHA1(95b13cbf9dc2196844038ce23ddfc33fecc9caef) )
	ROM_LOAD( "f.7e",         0xd000, 0x1000, CRC(12f24c68) SHA1(6d4181d3f044de491d810a3406e9d253d2c669d6) )
	ROM_LOAD( "g.6e",         0xe000, 0x1000, CRC(408f2326) SHA1(fe45084ed50701577eade2da8f4f787ee41d7acf) )
	ROM_LOAD( "h.4e",         0xf000, 0x1000, CRC(f8bbabe0) SHA1(59b2223219712f8a572b2cfbbc14f80ec2b32aae) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "i.5a",         0x0000, 0x1000, CRC(c7742460) SHA1(1dbf0f5be1e2666feef83f256e2993a6c23d7cfc) )
	ROM_LOAD( "j.6a",         0x1000, 0x1000, CRC(72166ccd) SHA1(4f4efcd8ed7f729f4630446607b0e9c93098aa3a) )

	ROM_REGION( 0x4000, "gfx1", 0 )
	ROM_LOAD( "n.1c",         0x0000, 0x1000, CRC(4f64538d) SHA1(1d48f12ff0d1c5604d19338b26e800a91f1be9c1) )
	ROM_LOAD( "m.1d",         0x1000, 0x1000, CRC(217b1402) SHA1(b842b2bde8ff5be6b240ccfb35c7a9f701dab5f4) )
	ROM_LOAD( "k.1b",         0x2000, 0x1000, CRC(b8b2b8cc) SHA1(e149fc91043f3233e10c81358b8624a4bc0baf4e) )
	ROM_LOAD( "l.1a",         0x3000, 0x1000, CRC(ab421a83) SHA1(1cc3e1bcf9e90ffbf7bfeeb0caa8a4f63b34146a) )
ROM_END


ROM_START( brix )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "a",            0x0000, 0x1000, CRC(050e0d70) SHA1(f5e0ed0845443701233de194d9ce24ec35e03a27) )
	ROM_LOAD( "b",            0x1000, 0x1000, CRC(668118ae) SHA1(688d6f79d30186bade15dbb1f08e8b25cbefa852) )
	ROM_LOAD( "c",            0x2000, 0x1000, CRC(ff5ed6cf) SHA1(b6309ed322c2bb12626dfaca705e296723ee7e47) )
	ROM_LOAD( "d",            0x3000, 0x1000, CRC(c3ae45a9) SHA1(879f0a495d9de855ffcbb0907b9b733ca626a7ef) )
	ROM_LOAD( "e",            0xc000, 0x1000, CRC(def99fa9) SHA1(e28d32934e1ad31595ec6097befd8518178c9d51) )
	ROM_LOAD( "f",            0xd000, 0x1000, CRC(dde717ed) SHA1(cf9063aa25faf2027770a4b27831e2e20d1801a0) )
	ROM_LOAD( "g",            0xe000, 0x1000, CRC(adca02d8) SHA1(75703a6f6d8b5eeb609ed5829d12b97b62309ba4) )
	ROM_LOAD( "h",            0xf000, 0x1000, CRC(bc3b878c) SHA1(91a5daa90a4c46a354f4ef64730b4a0a8348b6a0) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "i.5a",         0x0000, 0x1000, CRC(c7742460) SHA1(1dbf0f5be1e2666feef83f256e2993a6c23d7cfc) )
	ROM_LOAD( "j.6a",         0x1000, 0x1000, CRC(72166ccd) SHA1(4f4efcd8ed7f729f4630446607b0e9c93098aa3a) )

	ROM_REGION( 0x4000, "gfx1", 0 )
	ROM_LOAD( "n",            0x0000, 0x1000, CRC(8064910e) SHA1(331048e30604ef2a0ae0d7ee5ca5c230b601aec7) )
	ROM_LOAD( "m.1d",         0x1000, 0x1000, CRC(217b1402) SHA1(b842b2bde8ff5be6b240ccfb35c7a9f701dab5f4) )
	ROM_LOAD( "k",            0x2000, 0x1000, CRC(c7d7e2a0) SHA1(9790e78abf4f57ddfcef8e5632699152f9440a67) )
	ROM_LOAD( "l.1a",         0x3000, 0x1000, CRC(ab421a83) SHA1(1cc3e1bcf9e90ffbf7bfeeb0caa8a4f63b34146a) )
ROM_END


ROM_START( freeze )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "freeze.f2",    0x0000, 0x1000, CRC(0a431665) SHA1(57b7fc72c7e3b0d09b4a0676a4e7094657e2b742) )
	ROM_LOAD( "freeze.f3",    0x1000, 0x1000, CRC(1189b8ad) SHA1(8feb9387783e63a98efb60778fdf9eb9d5392cd9) )
	ROM_LOAD( "freeze.f4",    0x2000, 0x1000, CRC(10c4a5ea) SHA1(9ace2cff0280f10b03752568258b2e3a13ac964f) )
	ROM_LOAD( "freeze.f5",    0x3000, 0x1000, CRC(16024c53) SHA1(354b91ad880ce0ea0f1481c3aea91570d05797c7) )
	ROM_LOAD( "freeze.f7",    0xc000, 0x1000, CRC(ea0b0765) SHA1(17923177d31ab4ca9f9bba1fc95fff825d8113e3) )
	ROM_LOAD( "freeze.e7",    0xd000, 0x1000, CRC(1155c00b) SHA1(734eb7cc77432f7112e6032a298f8d38152a0717) )
	ROM_LOAD( "freeze.e5",    0xe000, 0x1000, CRC(95c18d75) SHA1(02c8b9738049f61d1d34053f508b26ee588b2025) )
	ROM_LOAD( "freeze.e4",    0xf000, 0x1000, CRC(7e8f5afc) SHA1(5694982671ef5c7564f216150825f4e81c4ba617) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "freeze.a1",    0x0000, 0x1000, CRC(7771f5b9) SHA1(48715945f67a0d736c86d1fdd738964c6cf74c35) )

	ROM_REGION( 0x4000, "gfx1", 0 )
	ROM_LOAD( "freeze.5a",    0x0000, 0x1000, CRC(6c8a98a0) SHA1(358a88377a227566962251c2a6ad7aea52ae1d17) )
	ROM_LOAD( "freeze.3a",    0x1000, 0x1000, CRC(6d2125e4) SHA1(6c3a12af512a1243b73759a758da8329bca38833) )
	ROM_LOAD( "freeze.1a",    0x2000, 0x1000, CRC(3a7f2fa9) SHA1(5f0811ea4e61b9918de2d16ffcfa4a02af833613) )
	ROM_LOAD( "freeze.2a",    0x3000, 0x1000, CRC(dd70ddd6) SHA1(d03cac0b4248da5d49ffac6ee57a3f8dd368731b) )
ROM_END


ROM_START( sucasino )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "1",            0x0000, 0x1000, CRC(e116e979) SHA1(99b0c783ace93e643738a1a924cafb690d2c1127) )
	ROM_LOAD( "2",            0x1000, 0x1000, CRC(2a2635f5) SHA1(e3b70942adc4eab81000287c8da67d3732ddda70) )
	ROM_LOAD( "3",            0x2000, 0x1000, CRC(69864d90) SHA1(244eaf4079b90f367c671e00e8081d885f26e26d) )
	ROM_LOAD( "4",            0x3000, 0x1000, CRC(174c9373) SHA1(070175bf1b7b14f34549d03a8288c8ff1f2f4eaa) )
	ROM_LOAD( "5",            0xc000, 0x1000, CRC(115bcb1e) SHA1(9b50e1dcb77db1b60ab5fd7d9843261e25580647) )
	ROM_LOAD( "6",            0xd000, 0x1000, CRC(434caa17) SHA1(2f537063db14cfdfb771dece2ea33841c874c708) )
	ROM_LOAD( "7",            0xe000, 0x1000, CRC(67c68b82) SHA1(b5d3977bf1f1337a96ae7bb60fe11e6ca9e87485) )
	ROM_LOAD( "8",            0xf000, 0x1000, CRC(f5b63006) SHA1(a069fb9b9b6d47ac3f0fbbd9b2c89da31d6b1202) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "9",            0x0000, 0x1000, CRC(67cf8aec) SHA1(95be671d5f7526610b175fc4121459e0ffc3649b) )

	ROM_REGION( 0x4000, "gfx1", 0 )
	ROM_LOAD( "11",           0x0000, 0x1000, CRC(f92c4c5b) SHA1(a415c8f55d1792e79d05ece223ef423f8578f896) )
	ROM_FILL(                 0x1000, 0x1000, 0 )
	ROM_LOAD( "10",           0x2000, 0x1000, CRC(3b0783ce) SHA1(880f258351a8b0d76abe433cc77d95b991ae1adc) )
	ROM_FILL(                 0x3000, 0x1000, 0 )
ROM_END


ROM_START( tripool )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "tri73a.bin",   0x0000, 0x1000, CRC(96893aa7) SHA1(ea1dc5824d89c1bb131850625a65d018a9127179) )
	ROM_FILL(                 0x1000, 0x1000, 0 )
	ROM_LOAD( "tri62a.bin",   0x2000, 0x1000, CRC(3299dc65) SHA1(8f93247e2f49be6b601006be62f4ad539ec899fe) )
	ROM_LOAD( "tri52b.bin",   0x3000, 0x1000, CRC(27ef765e) SHA1(2a18a9b74fd4d9f3a724270cd3a98adbfdf22a5e) )
	ROM_LOAD( "tri33c.bin",   0xc000, 0x1000, CRC(d7ef061d) SHA1(3ea3a136ecb3b5753a1dd929212b93ad8c7e9157) )
	ROM_LOAD( "tri45c.bin",   0xd000, 0x1000, CRC(51b813b1) SHA1(11ace37869a44a8c4bec76f19815a7f2fcc1d23e) )
	ROM_LOAD( "tri25d.bin",   0xe000, 0x1000, CRC(8e64512d) SHA1(c4983db1e8143dc90f9a8c99bdbb73dc31529a6c) )
	ROM_LOAD( "tri13d.bin",   0xf000, 0x1000, CRC(ad268e9b) SHA1(5d8d9b1c57b332b5a28b01d6a4f4885239d80b00) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "trisnd.bin",   0x0000, 0x1000, CRC(945c4b8b) SHA1(f574de1633e7dd71d29c0bcdbc6fa675d1a3f7d1) )

	ROM_REGION( 0x4000, "gfx1", 0 )
	ROM_LOAD( "tri105a.bin",  0x0000, 0x1000, CRC(366a753c) SHA1(30fa8d80e42287e3e8677aefd15beab384265728) )
	ROM_FILL(                 0x1000, 0x1000, 0 )
	ROM_LOAD( "tri93a.bin",   0x2000, 0x1000, CRC(35213782) SHA1(05d5a67ffa3d26377c54777917d3ba51677ebd28) )
	ROM_FILL(                 0x3000, 0x1000, 0 )
ROM_END


ROM_START( tripoola )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "tri73a.bin",   0x0000, 0x1000, CRC(96893aa7) SHA1(ea1dc5824d89c1bb131850625a65d018a9127179) )
	ROM_FILL(                 0x1000, 0x1000, 0 )
	ROM_LOAD( "tri62a.bin",   0x2000, 0x1000, CRC(3299dc65) SHA1(8f93247e2f49be6b601006be62f4ad539ec899fe) )
	ROM_LOAD( "tri52b.bin",   0x3000, 0x1000, CRC(27ef765e) SHA1(2a18a9b74fd4d9f3a724270cd3a98adbfdf22a5e) )
	ROM_LOAD( "tri33c.bin",   0xc000, 0x1000, CRC(d7ef061d) SHA1(3ea3a136ecb3b5753a1dd929212b93ad8c7e9157) )
	ROM_LOAD( "tri45c.bin",   0xd000, 0x1000, CRC(51b813b1) SHA1(11ace37869a44a8c4bec76f19815a7f2fcc1d23e) )
	ROM_LOAD( "tri25d.bin",   0xe000, 0x1000, CRC(8e64512d) SHA1(c4983db1e8143dc90f9a8c99bdbb73dc31529a6c) )
	ROM_LOAD( "tp1ckt",       0xf000, 0x1000, CRC(72ec43a3) SHA1(a4f5b20872e41845340db627321e0dbcad4b964e) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "trisnd.bin",   0x0000, 0x1000, CRC(945c4b8b) SHA1(f574de1633e7dd71d29c0bcdbc6fa675d1a3f7d1) )

	ROM_REGION( 0x4000, "gfx1", 0 )
	ROM_LOAD( "tri105a.bin",  0x0000, 0x1000, CRC(366a753c) SHA1(30fa8d80e42287e3e8677aefd15beab384265728) )
	ROM_FILL(                 0x1000, 0x1000, 0 )
	ROM_LOAD( "tri93a.bin",   0x2000, 0x1000, CRC(35213782) SHA1(05d5a67ffa3d26377c54777917d3ba51677ebd28) )
	ROM_FILL(                 0x3000, 0x1000, 0 )
ROM_END


/*

Super Triv
?, 1985

PCB Layout
----------

Top Board

P-1244-1A MADE IN JAPAN
HARA INDUSTRIES JAPAN CO., LTD.
|-------------------------------------------|
|                                           |
|                                      2114 |
|       VOL                            2114 |
|                                DIP40      |
|                                           |
|  DSW1    6116                             |
|4 DSW2    6116    6116                     |
|4         DIP24   BC3.7E                   |
|W         PR4.6F  BC2.6E             SND.5A|
|A         PR3.5F  BC1.5E                   |
|Y         PR2.4F                           |
|          DIP24         Z80                |
|                                 Z80       |
|--|---CN-J----|----------------------------|
   |-----------|

Notes:
      DIP24/40 - Empty sockets used to connect ROM daughterboard to main board
      Z80      - Z80 CPU running at 3.000MHz (both)
      6116     - 2K x8 SRAM
      2114     - 1K x4 SRAM
      CN-J     - 50 pin flat cable PCB joiner

Bottom Board

P-1244-2A MADE IN JAPAN
|-------------------------------------------|
|                  82S16              18MHz |
|                  82S16                    |
|                  82S16                    |
|                  82S16                    |
|                  82S16                    |
|   2114  2114                              |
|   2114  2114                              |
|   2114  2114                       CHR3.5A|
|   2114  2114                              |
|   2114                             CHR2.4A|
|   2114                                    |
|   2114                             CHR1.2A|
|   2114                             CHR0.1A|
|--|---CN-J----|----------------------------|
   |-----------|
Notes:
      82S16 - 256bytes x 1bit Bipolar DRAM
      2114  - 1K x4 SRAM
      CN-J  - 50 pin flat cable PCB joiner


ROM Daughterboard

|-----------------------------|
|ROM.U7  TBFT1.U14  AY3-8910  |
|ROM.U6  TBFT0.U13  TBFD0.U21 |
|        TBFL2.U12            |
|        TBFL1.U11            |
|        TBFL0.U10            |
|        TBFD3.U9             |
|PR1.F2  TBFD2.U8   TBFD1.U15 |
|-----------------------------|

*/

ROM_START( striv )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "pr1.f2",       0x0000, 0x1000, CRC(dcf5da6e) SHA1(e88c8226ae4d4a0af717f0760e551e0ce4c79c5e) )
	ROM_LOAD( "pr2.4f",       0x1000, 0x1000, CRC(921610ba) SHA1(7dea7a57543dd79325da34cebd7b9dd8a767bb2a) )
	ROM_LOAD( "pr3.5f",       0x2000, 0x1000, CRC(c36f0e21) SHA1(d036a56798bbb42bee269450524172ec071dcf03) )
	ROM_LOAD( "pr4.6f",       0x3000, 0x1000, CRC(0dc98a97) SHA1(36c1c61d3330e2c00d9aa94ae80bcb1b9c5aea21) )
	/* 0xc000 - 0xcfff questions rom space */
	ROM_LOAD( "bc3.7e",       0xd000, 0x1000, CRC(83f03885) SHA1(d83f03752ccf85fd9f10c2d801ecd5f4ef729cde) )
	ROM_LOAD( "bc2.6e",       0xe000, 0x1000, CRC(75f18361) SHA1(4966418f3b6204e888bd56f36db88518dcd08640) )
	ROM_LOAD( "bc1.5e",       0xf000, 0x1000, CRC(0d150385) SHA1(090139797a1b6935fcf4c239e11bdd7ae55fac76) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "snd.5a",       0x0000, 0x1000, CRC(b7ddf84f) SHA1(fa4cc0b2e5a88c82c62492c03e97ac6aa8a905b1) )

	ROM_REGION( 0x4000, "gfx1", 0 )
	ROM_LOAD( "chr3.5a",      0x0000, 0x1000, CRC(8f982a9c) SHA1(dd6f454dfd3e03d008080890881cfafd79758a40) )
	ROM_LOAD( "chr2.4a",      0x1000, 0x1000, CRC(8f982a9c) SHA1(dd6f454dfd3e03d008080890881cfafd79758a40) )
	ROM_LOAD( "chr1.2a",      0x2000, 0x1000, CRC(7ad4358e) SHA1(dd3a03c78fa8bf435e9905b901dc5a9987cd52e4) )
	ROM_LOAD( "chr0.1a",      0x3000, 0x1000, CRC(8f60229b) SHA1(96a888ae02797a205e1c6202395d3b42a820ad4d) )

	ROM_REGION( 0x80000, "user1", ROMREGION_ERASEFF ) /* Question roms */
	ROM_LOAD( "rom.u6",       0x00000, 0x8000, CRC(a32d7a28) SHA1(fbad0b5c9f1dbeb4f245a2198248c18ceae556fa) )
	ROM_LOAD( "rom.u7",       0x08000, 0x8000, CRC(bc44ae18) SHA1(815cc3c87b89fc702a9ca88d5117ab46464b53c0) )
	ROM_LOAD( "tbfd2.u8",     0x10000, 0x8000, CRC(9572984a) SHA1(0edd668754b84cc8c3dce9c8db17ea6e4a397765) )
	ROM_LOAD( "tbfd3.u9",     0x18000, 0x8000, CRC(d904a2f1) SHA1(a7adc07319f04f4bd383145ec07f1924eb5cbd4d) )
	ROM_LOAD( "tbfl0.u10",    0x20000, 0x8000, CRC(680264a2) SHA1(a3637aa36a31c0cda1d530c8307996516a05f9ef) )
	ROM_LOAD( "tbfl1.u11",    0x28000, 0x8000, CRC(33e99d00) SHA1(0b6c6b564507e0ee189fe8ba1d85c77c2c4c77e7) )
	ROM_LOAD( "tbfl2.u12",    0x30000, 0x8000, CRC(2e7a941f) SHA1(8f5331df91f865c559381114e21aa118c8fe34eb) )
	ROM_LOAD( "tbft0.u13",    0x38000, 0x8000, CRC(7d2e5e89) SHA1(4e56fe325b93d577602e8bcab3130f426cee3de4) )
	ROM_LOAD( "tbft1.u14",    0x40000, 0x8000, CRC(d36246cf) SHA1(553ce72c35822694461adc2e572522a9eeff5667) )
	ROM_LOAD( "tbfd1.u15",    0x48000, 0x8000, CRC(745db398) SHA1(52b6999699ebae8ed9ada45d47a8f8ee68e36bf1) )
	// 0x50000 - 0x7ffff empty

	ROM_REGION( 0x2000, "user2", 0 ) // ? probably leftover / unused, it's a program rom from Hyper Olympic
	ROM_LOAD( "tbfd0.u21",    0x0000, 0x2000, CRC(15b83099) SHA1(79827590d74f20c9a95723e06b05af2b15c34f5f) )
ROM_END


ROM_START( joinem )
	ROM_REGION( 0x10000, "maincpu", 0 ) /* main z80 cpu */
	ROM_LOAD( "join1.r0", 0x0000, 0x2000, CRC(b5b2e2cc) SHA1(e939478d19ac27807ba4180835c512b5fcb8d0c5) )
	ROM_LOAD( "join2.r2", 0x2000, 0x2000, CRC(bcf140e6) SHA1(3fb4fbb758518d8ae26abbe76f12678cf988bd0e) )
	ROM_LOAD( "join3.r4", 0x4000, 0x2000, CRC(fe04e4d4) SHA1(9b34cc5915dd78340d1cedb34f5d397d3b39ca14) )

	ROM_REGION( 0x10000, "audiocpu", 0 ) /* sound z80 cpu */
	ROM_LOAD( "join7.s0", 0x0000, 0x1000, CRC(bb8a7814) SHA1(cfb85408827b96a81401223256e23082b7e9598f) )

	ROM_REGION( 0x3000, "gfx1", 0 ) /* gfx - 8x8x3bpp */
	ROM_LOAD( "join4.p3", 0x0000, 0x1000, CRC(4964c82c) SHA1(7a45399db20f9bbdb2de58243732e3951ffe358c) )
	ROM_LOAD( "join5.p2", 0x1000, 0x1000, CRC(ae78fa89) SHA1(8f43fd2ec037185a1b9bd9c61c49ad891c504d4d) )
	ROM_LOAD( "join6.p1", 0x2000, 0x1000, CRC(2b533261) SHA1(ce6c1fa833b34aeb401f430d212415c33beb2922) )

	ROM_REGION( 0x100, "proms", 0 ) /* colours */
	ROM_LOAD_NIB_LOW(  "l82s129.11n", 0x000, 0x100, CRC(7b724211) SHA1(7396c773e8d48dea856d9482d6c48de966616c83) )
	ROM_LOAD_NIB_HIGH( "h82s129.12n", 0x000, 0x100, CRC(2e81c5ff) SHA1(e103c8813af704d5de11fe705de5105ff3a691c3) )
ROM_END


ROM_START( unclepoo )
	ROM_REGION( 0x10000, "maincpu", 0 ) /* main z80 cpu */
	ROM_LOAD( "01.f17", 0x0000, 0x2000, CRC(92fb238c) SHA1(e9476c5c1a0bf9e8c6c364ac022ed1d97ae66d2e) )
	ROM_LOAD( "02.f14", 0x2000, 0x2000, CRC(b99214ef) SHA1(c8e4af0efbc5ea543277b2764dc6f119aae477ca) )
	ROM_LOAD( "03.f11", 0x4000, 0x2000, CRC(a136af97) SHA1(cfa610bf357870053617fed8aef6bb30bd996422) )
	ROM_LOAD( "04.f09", 0x6000, 0x2000, CRC(c4bcd414) SHA1(df3125358530f5fb8d202bddcb0ef5e322fabb7b) )

	ROM_REGION( 0x10000, "audiocpu", 0 ) /* sound z80 cpu */
	ROM_LOAD( "08.c15", 0x0000, 0x1000, CRC(fd84106b) SHA1(891853d2b39850a981016108b74ca20337d2cdd8) )

	ROM_REGION( 0x6000, "gfx1", 0 ) /* gfx - 8x8x3bpp */
	ROM_LOAD( "07.h04", 0x0000, 0x2000, CRC(e2f73e99) SHA1(61cb09ff424ba63b892b4822e7ed916af73412f1) )
	ROM_LOAD( "06.j04", 0x2000, 0x2000, CRC(94b5f676) SHA1(32c27854726636c4ce03bb6a83b32d04ed6c42af) )
	ROM_LOAD( "05.k04", 0x4000, 0x2000, CRC(64026934) SHA1(a5342335d02d34fa6ba2b29484ed71ecc96292f2) )

	ROM_REGION( 0x200, "proms", 0 ) /* colours */
	ROM_LOAD_NIB_LOW(  "diatec_l.bin", 0x000, 0x100, CRC(b04d466a) SHA1(1438abeae76ef807ba34bd6d3e4c44f707dbde6e) )
	ROM_LOAD_NIB_HIGH( "diatec_h.bin", 0x000, 0x100, CRC(938601b1) SHA1(8213284989bebb5f7375878181840de8079dc1f3) )
ROM_END


ROM_START( loverboy )
	ROM_REGION( 0x10000, "maincpu", 0 ) /* main z80 cpu */
	ROM_LOAD( "lover.r0", 0x0000, 0x2000, CRC(ffec4e41) SHA1(65428ebcb3af47071fef70a35388e070a019f692) )
	ROM_LOAD( "lover.r2", 0x2000, 0x2000, CRC(04052262) SHA1(056a225c8625e53881753b0b0330f9b277d14a7d) )
	ROM_LOAD( "lover.r4", 0x4000, 0x2000, CRC(ce5f3b49) SHA1(cb55e1f7c3df59389ac14b7da4f584ae054abca3) )
	ROM_LOAD( "lover.r6", 0x6000, 0x1000, CRC(839d79b7) SHA1(ac1c0fbf23e7d1a53b47dae16170857c55e6ae48) )

	ROM_REGION( 0x10000, "audiocpu", 0 ) /* sound z80 cpu */
	ROM_LOAD( "lover.s0", 0x0000, 0x1000, CRC(ec38111c) SHA1(09efded9e905658bdbcde4ad4f0b4cb9585bdb33) )

	ROM_REGION( 0x6000, "gfx1", 0 ) /* gfx - 8x8x3bpp */
	ROM_LOAD( "lover.p3", 0x0000, 0x2000, CRC(1a519c8f) SHA1(36f546deaf36e8cd3bd113d84fd5e5f6e98d5de5) )
	ROM_LOAD( "lover.p2", 0x2000, 0x2000, CRC(e465372f) SHA1(345b769ebc33f60daa9692b64e8ef43062552a33) )
	ROM_LOAD( "lover.p1", 0x4000, 0x2000, CRC(cda0d87e) SHA1(efff230e994e21705902f252e50ee40a20444c0f) )

	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD_NIB_LOW(  "color.n11", 0x000, 0x200, CRC(cf4a16ae) SHA1(e17c5dfd73c5bc55c0a929cf65ee5b516c9776a5) )
	ROM_LOAD_NIB_HIGH( "color.n12", 0x000, 0x200, CRC(4b11ac21) SHA1(d9e7cecfb7237335288ab6f94bb35696d8291bdf) )
ROM_END



/*************************************
 *
 *  Driver initialization
 *
 *************************************/

DRIVER_INIT_MEMBER(jack_state,jack)
{
	m_timer_rate = 256;
}


DRIVER_INIT_MEMBER(jack_state,zzyzzyxx)
{
	m_timer_rate = 32;
}


void jack_state::treahunt_decode(  )
{
	UINT8 *rom = memregion("maincpu")->base();

	/* Thanks to Mike Balfour for helping out with the decryption */
	for (int A = 0; A < 0x4000; A++)
	{
		UINT8 data = rom[A];

		if (A & 0x1000)
		{
			/* unencrypted = D0 D2 D5 D1 D3 D6 D4 D7 */
			m_decrypted_opcodes[A] =
					((data & 0x01) << 7) |
					((data & 0x02) << 3) |
					((data & 0x04) << 4) |
					(data & 0x28) |
					((data & 0x10) >> 3) |
					((data & 0x40) >> 4) |
					((data & 0x80) >> 7);

			if ((A & 0x04) == 0)
			/* unencrypted = !D0 D2 D5 D1 D3 D6 D4 !D7 */
				m_decrypted_opcodes[A] ^= 0x81;
		}
		else
		{
			/* unencrypted = !D7 D2 D5 D1 D3 D6 D4 !D0 */
			m_decrypted_opcodes[A] =
					(~data & 0x81) |
					((data & 0x02) << 3) |
					((data & 0x04) << 4) |
						(data & 0x28) |
					((data & 0x10) >> 3) |
					((data & 0x40) >> 4);
		}
	}
}

DRIVER_INIT_MEMBER(jack_state,treahunt)
{
	m_timer_rate = 256;
	treahunt_decode();
}


DRIVER_INIT_MEMBER(jack_state,loverboy)
{
	/* this doesn't make sense.. the startup code, and irq0 have jumps to 0..
	   I replace the startup jump with another jump to what appears to be
	   the start of the game code.

	   ToDo: Figure out what's really going on
	   EDIT: this is fun, it's in im0 and trips ei ... my best guess is that
	   there's a protection device enabled at 0xf000-0xf001-0xf002-0xf008 that
	   sends a custom irq (either ld hl,$019d or jp $019d). After the initial
	   code, the protection device is disabled or changes behaviour via
	   writes at 0xf000 and 0xf008. -AS
	*/
	UINT8 *ROM = memregion("maincpu")->base();
	ROM[0x13] = 0x01;
	ROM[0x12] = 0x9d;

	m_timer_rate = 32;
}


DRIVER_INIT_MEMBER(jack_state,striv)
{
	UINT8 *ROM = memregion("maincpu")->base();
	UINT8 data;
	int A;

	/* decrypt program rom */
	/* thanks to David Widel to have helped with the decryption */
	for (A = 0; A < 0x4000; A++)
	{
		data = ROM[A];

		if (A & 0x1000)
		{
			if (A & 4)
				ROM[A] = BITSWAP8(data,7,2,5,1,3,6,4,0) ^ 1;
			else
				ROM[A] = BITSWAP8(data,0,2,5,1,3,6,4,7) ^ 0x81;
		}
		else
		{
			if (A & 4)
				ROM[A] = BITSWAP8(data,7,2,5,1,3,6,4,0) ^ 1;
			else
				ROM[A] = BITSWAP8(data,0,2,5,1,3,6,4,7);
		}
	}

	m_timer_rate = 256;
}



/*************************************
 *
 *  Game driver(s)
 *
 *************************************/

GAME( 1982, jack,      0,        jack,     jack,     jack_state, jack,     ROT90,  "Hara Industries (Cinematronics license)", "Jack the Giantkiller (set 1)", MACHINE_SUPPORTS_SAVE )
GAME( 1982, jack2,     jack,     jack,     jack2,    jack_state, jack,     ROT90,  "Hara Industries (Cinematronics license)", "Jack the Giantkiller (set 2)", MACHINE_SUPPORTS_SAVE )
GAME( 1982, jack3,     jack,     jack,     jack3,    jack_state, jack,     ROT90,  "Hara Industries (Cinematronics license)", "Jack the Giantkiller (set 3)", MACHINE_SUPPORTS_SAVE )
GAME( 1982, treahunt,  jack,     treahunt, treahunt, jack_state, treahunt, ROT90,  "Hara Industries", "Treasure Hunt", MACHINE_SUPPORTS_SAVE )
GAME( 1982, zzyzzyxx,  0,        jack,     zzyzzyxx, jack_state, zzyzzyxx, ROT90,  "Cinematronics / Advanced Microcomputer Systems", "Zzyzzyxx (set 1)", MACHINE_SUPPORTS_SAVE )
GAME( 1982, zzyzzyxx2, zzyzzyxx, jack,     zzyzzyxx, jack_state, zzyzzyxx, ROT90,  "Cinematronics / Advanced Microcomputer Systems", "Zzyzzyxx (set 2)", MACHINE_SUPPORTS_SAVE )
GAME( 1982, brix,      zzyzzyxx, jack,     zzyzzyxx, jack_state, zzyzzyxx, ROT90,  "Cinematronics / Advanced Microcomputer Systems", "Brix", MACHINE_SUPPORTS_SAVE )
GAME( 1984, freeze,    0,        jack,     freeze,   jack_state, jack,     ROT90,  "Cinematronics", "Freeze", MACHINE_SUPPORTS_SAVE | MACHINE_NO_COCKTAIL )
GAME( 1981, tripool,   0,        jack,     tripool,  jack_state, jack,     ROT90,  "Noma (Casino Tech license)", "Tri-Pool (Casino Tech)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )
GAME( 1981, tripoola,  tripool,  jack,     tripool,  jack_state, jack,     ROT90,  "Noma (Costal Games license)", "Tri-Pool (Costal Games)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )
GAME( 1984, sucasino,  0,        jack,     sucasino, jack_state, jack,     ROT90,  "Data Amusement", "Super Casino", MACHINE_SUPPORTS_SAVE )
GAME( 1985, striv,     0,        striv,    striv,    jack_state, striv,    ROT270, "Nova du Canada", "Super Triv", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE ) // Hara Industries PCB
GAME( 1983, joinem,    0,        joinem,   joinem,   jack_state, zzyzzyxx, ROT90,  "Global Corporation", "Joinem", MACHINE_SUPPORTS_SAVE )
GAME( 1983, unclepoo,  0,        unclepoo, unclepoo, jack_state, zzyzzyxx, ROT90,  "Diatec", "Uncle Poo", MACHINE_SUPPORTS_SAVE ) // based on Joinem?
GAME( 1983, loverboy,  0,        joinem,   loverboy, jack_state, loverboy, ROT90,  "G.T Enterprise Inc.", "Lover Boy", MACHINE_SUPPORTS_SAVE )
