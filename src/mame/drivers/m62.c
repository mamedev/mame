// license:BSD-3-Clause
// copyright-holders:smf, David Haywood
/****************************************************************************

Irem "M62" system

TODO:
- Kid Niki and Horizon are missing the drums. There is an analog section in
  the sound board.

Notes:
- I believe that both kungfum bootlegs are derived from an Irem original which we
  don't have (prototype/early revision?). They say "kanfu master" instead of
  "kung-fu master" on the introduction screen, the only original doing that is
  spartanx but the ROMs don't match after the copyright notice.

Battle Bird (c) 1985 - Not Dumped
 This game is on IREM's M64 platform which is a two tiered boardset.
 Battle Bird is believed to have a limited release.
 Two pictures of the PCB set appeared on the internet as well as showing two screen shots.
 The picture of the board showed a genuine IREM License Seal PCB 1037 sticker and Serial # 353887
     Top board: M64-S-A  Looks to contain Nec D780C, Yamaha YM2149 & YM2151 + 2 roms
  Bottom board: M64-A-C  Mostly out of the picture, so no additional info.


The following information is gathered from Kung Fu Master; the board was most
likely modified for other games (or, not all the games in this driver are
really M62).

The M62 board can be set up for different configurations through the use of
jumpers.

A board:
J1: \
J2: / ROM or RAM at 0x4000
J3: sound prg ROM size, 2764 or 27128
J4: send output C of the secondy AY-3-8910 to SOUND IO instead of SOUND. Is
    this to have it amplified more?
J5: enable a tristate on accesses to the range a000-bfff (must not be done
    when there is ROM at this address)
J6:
J7: main prg ROM type, 2764 or 27128

B board:
J1: selects whether bit 4 of obj color code selects or not high priority over tiles
J2: selects whether bit 4 of obj color code goes to A7 of obj color PROMS
J3: I'm not sure about this. It involves A8 of sprite ram.
J4: pixels per scanline, 256 or 384. There's also a PROM @ 6F that controls
    video timing and how long a scanline is.
J5: output Horizontal Sync or Composite Sync
J6: ??? where is this ???
J7: \ main xtal, 18.432 MHz (for low resolution games?) or
J8: / 24 MHz (for mid resolution games?)
J9: obj ROM type, 2764 or 27128

G board:
JP1: \
JP2: | Tiles with color code >= the value set here have priority over sprites
JP3: |
JP4: /


2008-08
Dip locations verified with dips listing for: kungfum, ldrun, kidniki,
spelunkr, spelunk2.
The other locations have been added assuming that the layout is the same
on all m62 boards. However, it would be nice to have them confirmed for
other supported games as well.

**************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "includes/iremipt.h"
#include "includes/m62.h"


/* Lode Runner 2 seems to have a simple protection on the bank switching */
/* circuitry. It writes data to ports 0x80 and 0x81, then reads port 0x80 */
/* a variable number of times (discarding the result) and finally retrieves */
/* data from the bankswitched ROM area. */
/* Since the data written to 0x80 is always the level number, I just use */
/* that to select the ROM. The only exception I make is a special case used in */
/* service mode to test the ROMs. */

READ8_MEMBER(m62_state::ldrun2_bankswitch_r)
{
	if (m_ldrun2_bankswap)
	{
		m_ldrun2_bankswap--;

		/* swap to bank #1 on second read */
		if (m_ldrun2_bankswap == 0)
			membank("bank1")->set_entry(1);
	}
	return 0;
}

WRITE8_MEMBER(m62_state::ldrun2_bankswitch_w)
{
	static const int banks[30] =
	{
		0,0,0,0,0,1,0,1,0,0,
		0,1,1,1,1,1,0,0,0,0,
		1,0,1,1,1,1,1,1,1,1
	};


	m_bankcontrol[offset] = data;

	if (offset == 0)
	{
		if (data < 1 || data > 30)
		{
			logerror("unknown bank select %02x\n",data);
			return;
		}
		membank("bank1")->set_entry(banks[data - 1]);
	}
	else
	{
		if (m_bankcontrol[0] == 0x01 && data == 0x0d)
		/* special case for service mode */
			m_ldrun2_bankswap = 2;
		else
			m_ldrun2_bankswap = 0;
	}
}


/* Lode Runner 3 has, it seems, a poor man's protection consisting of a PAL */
/* (I think; it's included in the ROM set) which is read at certain times, */
/* and the game crashes if it doesn't match the expected values. */
READ8_MEMBER(m62_state::ldrun3_prot_5_r)
{
	return 5;
}

READ8_MEMBER(m62_state::ldrun3_prot_7_r)
{
	return 7;
}


WRITE8_MEMBER(m62_state::ldrun4_bankswitch_w)
{
	membank("bank1")->set_entry(data & 0x01);
}

WRITE8_MEMBER(m62_state::kidniki_bankswitch_w)
{
	membank("bank1")->set_entry(data & 0x0f);
}

#define battroad_bankswitch_w kidniki_bankswitch_w

WRITE8_MEMBER(m62_state::spelunkr_bankswitch_w)
{
	membank("bank1")->set_entry(data & 0x03);
}

WRITE8_MEMBER(m62_state::spelunk2_bankswitch_w)
{
	membank("bank1")->set_entry((data & 0xc0) >> 6);
	membank("bank2")->set_entry((data & 0x3c) >> 2);
}

WRITE8_MEMBER(m62_state::youjyudn_bankswitch_w)
{
	membank("bank1")->set_entry(data & 0x01);
}


static ADDRESS_MAP_START( kungfum_map, AS_PROGRAM, 8, m62_state )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0xa000, 0xa000) AM_WRITE(m62_hscroll_low_w)
	AM_RANGE(0xb000, 0xb000) AM_WRITE(m62_hscroll_high_w)
	AM_RANGE(0xc000, 0xc0ff) AM_WRITEONLY AM_SHARE("spriteram")
	/* Kung Fu Master is the only game in this driver to have separated (but */
	/* contiguous) videoram and colorram. They are interleaved in all the others. */
	AM_RANGE(0xd000, 0xdfff) AM_RAM_WRITE(kungfum_tileram_w) AM_SHARE("m62_tileram")
	AM_RANGE(0xe000, 0xefff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( kungfum_io_map, AS_IO, 8, m62_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x00) AM_READ_PORT("SYSTEM") AM_DEVWRITE("irem_audio", irem_audio_device, cmd_w)
	AM_RANGE(0x01, 0x01) AM_READ_PORT("P1") AM_WRITE(m62_flipscreen_w)  /* + coin counters */
	AM_RANGE(0x02, 0x02) AM_READ_PORT("P2")
	AM_RANGE(0x03, 0x03) AM_READ_PORT("DSW1")
	AM_RANGE(0x04, 0x04) AM_READ_PORT("DSW2")
ADDRESS_MAP_END

static ADDRESS_MAP_START( battroad_map, AS_PROGRAM, 8, m62_state )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0xa000, 0xbfff) AM_ROMBANK("bank1")
	AM_RANGE(0xc000, 0xc0ff) AM_WRITEONLY AM_SHARE("spriteram")
	AM_RANGE(0xc800, 0xcfff) AM_RAM_WRITE(m62_textram_w) AM_SHARE("m62_textram")
	AM_RANGE(0xd000, 0xdfff) AM_RAM_WRITE(m62_tileram_w) AM_SHARE("m62_tileram")
	AM_RANGE(0xe000, 0xefff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( battroad_io_map, AS_IO, 8, m62_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x00) AM_READ_PORT("SYSTEM") AM_DEVWRITE("irem_audio", irem_audio_device, cmd_w)
	AM_RANGE(0x01, 0x01) AM_READ_PORT("P1") AM_WRITE(m62_flipscreen_w)  /* + coin counters */
	AM_RANGE(0x02, 0x02) AM_READ_PORT("P2")
	AM_RANGE(0x03, 0x03) AM_READ_PORT("DSW1")
	AM_RANGE(0x04, 0x04) AM_READ_PORT("DSW2")
	AM_RANGE(0x80, 0x80) AM_WRITE(m62_vscroll_low_w)
	AM_RANGE(0x81, 0x81) AM_WRITE(m62_hscroll_high_w)
	AM_RANGE(0x82, 0x82) AM_WRITE(m62_hscroll_low_w)
	AM_RANGE(0x83, 0x83) AM_WRITE(battroad_bankswitch_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( ldrun_map, AS_PROGRAM, 8, m62_state )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0xc000, 0xc0ff) AM_WRITEONLY AM_SHARE("spriteram")
	AM_RANGE(0xd000, 0xdfff) AM_RAM_WRITE(m62_tileram_w) AM_SHARE("m62_tileram")
	AM_RANGE(0xe000, 0xefff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( ldrun2_map, AS_PROGRAM, 8, m62_state )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0x8000, 0x9fff) AM_ROMBANK("bank1")
	AM_RANGE(0xc000, 0xc0ff) AM_WRITEONLY AM_SHARE("spriteram")
	AM_RANGE(0xd000, 0xdfff) AM_RAM_WRITE(m62_tileram_w) AM_SHARE("m62_tileram")
	AM_RANGE(0xe000, 0xefff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( ldrun2_io_map, AS_IO, 8, m62_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x00) AM_READ_PORT("SYSTEM") AM_DEVWRITE("irem_audio", irem_audio_device, cmd_w)
	AM_RANGE(0x01, 0x01) AM_READ_PORT("P1") AM_WRITE(m62_flipscreen_w)  /* + coin counters */
	AM_RANGE(0x02, 0x02) AM_READ_PORT("P2")
	AM_RANGE(0x03, 0x03) AM_READ_PORT("DSW1")
	AM_RANGE(0x04, 0x04) AM_READ_PORT("DSW2")
	AM_RANGE(0x80, 0x80) AM_READ(ldrun2_bankswitch_r)
	AM_RANGE(0x80, 0x81) AM_WRITE(ldrun2_bankswitch_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( ldrun3_map, AS_PROGRAM, 8, m62_state )
	AM_RANGE(0x0000, 0xbfff) AM_ROM
	AM_RANGE(0xc800, 0xc800) AM_READ(ldrun3_prot_5_r)
	AM_RANGE(0xcc00, 0xcc00) AM_READ(ldrun3_prot_7_r)
	AM_RANGE(0xcfff, 0xcfff) AM_READ(ldrun3_prot_7_r)
	AM_RANGE(0xc000, 0xc0ff) AM_WRITEONLY AM_SHARE("spriteram")
	AM_RANGE(0xd000, 0xdfff) AM_RAM_WRITE(m62_tileram_w) AM_SHARE("m62_tileram")
	AM_RANGE(0xd000, 0xefff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( ldrun3_io_map, AS_IO, 8, m62_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x00) AM_READ_PORT("SYSTEM") AM_DEVWRITE("irem_audio", irem_audio_device, cmd_w)
	AM_RANGE(0x01, 0x01) AM_READ_PORT("P1") AM_WRITE(m62_flipscreen_w)  /* + coin counters */
	AM_RANGE(0x02, 0x02) AM_READ_PORT("P2")
	AM_RANGE(0x03, 0x03) AM_READ_PORT("DSW1")
	AM_RANGE(0x04, 0x04) AM_READ_PORT("DSW2")
	AM_RANGE(0x80, 0x80) AM_WRITE(m62_vscroll_low_w)
	AM_RANGE(0x81, 0x81) AM_WRITE(ldrun3_topbottom_mask_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( ldrun4_map, AS_PROGRAM, 8, m62_state )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0x8000, 0xbfff) AM_ROMBANK("bank1")
	AM_RANGE(0xc000, 0xc0ff) AM_WRITEONLY AM_SHARE("spriteram")
	AM_RANGE(0xc800, 0xc800) AM_WRITE(ldrun4_bankswitch_w)
	AM_RANGE(0xd000, 0xdfff) AM_RAM_WRITE(m62_tileram_w) AM_SHARE("m62_tileram")
	AM_RANGE(0xe000, 0xefff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( ldrun4_io_map, AS_IO, 8, m62_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x00) AM_READ_PORT("SYSTEM") AM_DEVWRITE("irem_audio", irem_audio_device, cmd_w)
	AM_RANGE(0x01, 0x01) AM_READ_PORT("P1") AM_WRITE(m62_flipscreen_w)  /* + coin counters */
	AM_RANGE(0x02, 0x02) AM_READ_PORT("P2")
	AM_RANGE(0x03, 0x03) AM_READ_PORT("DSW1")
	AM_RANGE(0x04, 0x04) AM_READ_PORT("DSW2")
	AM_RANGE(0x82, 0x82) AM_WRITE(m62_hscroll_high_w)
	AM_RANGE(0x83, 0x83) AM_WRITE(m62_hscroll_low_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( lotlot_map, AS_PROGRAM, 8, m62_state )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0xa000, 0xafff) AM_RAM_WRITE(m62_textram_w) AM_SHARE("m62_textram")
	AM_RANGE(0xc000, 0xc0ff) AM_WRITEONLY AM_SHARE("spriteram")
	AM_RANGE(0xd000, 0xdfff) AM_RAM_WRITE(m62_tileram_w) AM_SHARE("m62_tileram")
	AM_RANGE(0xe000, 0xefff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( kidniki_map, AS_PROGRAM, 8, m62_state )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0x8000, 0x9fff) AM_ROMBANK("bank1")
	AM_RANGE(0xa000, 0xafff) AM_RAM_WRITE(m62_tileram_w) AM_SHARE("m62_tileram")
	AM_RANGE(0xc000, 0xc0ff) AM_WRITEONLY AM_SHARE("spriteram")
	AM_RANGE(0xd000, 0xdfff) AM_RAM_WRITE(m62_textram_w) AM_SHARE("m62_textram")
	AM_RANGE(0xe000, 0xefff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( kidniki_io_map, AS_IO, 8, m62_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x00) AM_READ_PORT("SYSTEM") AM_DEVWRITE("irem_audio", irem_audio_device, cmd_w)
	AM_RANGE(0x01, 0x01) AM_READ_PORT("P1") AM_WRITE(m62_flipscreen_w)  /* + coin counters */
	AM_RANGE(0x02, 0x02) AM_READ_PORT("P2")
	AM_RANGE(0x03, 0x03) AM_READ_PORT("DSW1")
	AM_RANGE(0x04, 0x04) AM_READ_PORT("DSW2")
	AM_RANGE(0x80, 0x80) AM_WRITE(m62_hscroll_low_w)
	AM_RANGE(0x81, 0x81) AM_WRITE(m62_hscroll_high_w)
	AM_RANGE(0x82, 0x82) AM_WRITE(kidniki_text_vscroll_low_w)
	AM_RANGE(0x83, 0x83) AM_WRITE(kidniki_text_vscroll_high_w)
	AM_RANGE(0x84, 0x84) AM_WRITE(kidniki_background_bank_w)
	AM_RANGE(0x85, 0x85) AM_WRITE(kidniki_bankswitch_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( spelunkr_map, AS_PROGRAM, 8, m62_state )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0x8000, 0x9fff) AM_ROMBANK("bank1")
	AM_RANGE(0xa000, 0xbfff) AM_RAM_WRITE(m62_tileram_w) AM_SHARE("m62_tileram")
	AM_RANGE(0xc000, 0xc0ff) AM_WRITEONLY AM_SHARE("spriteram")
	AM_RANGE(0xc800, 0xcfff) AM_RAM_WRITE(m62_textram_w) AM_SHARE("m62_textram")
	AM_RANGE(0xd000, 0xd000) AM_WRITE(m62_vscroll_low_w)
	AM_RANGE(0xd001, 0xd001) AM_WRITE(m62_vscroll_high_w)
	AM_RANGE(0xd002, 0xd002) AM_WRITE(m62_hscroll_low_w)
	AM_RANGE(0xd003, 0xd003) AM_WRITE(m62_hscroll_high_w)
	AM_RANGE(0xd004, 0xd004) AM_WRITE(spelunkr_bankswitch_w)
	AM_RANGE(0xd005, 0xd005) AM_WRITE(spelunkr_palbank_w)
	AM_RANGE(0xe000, 0xefff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( spelunk2_map, AS_PROGRAM, 8, m62_state )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0x8000, 0x8fff) AM_ROMBANK("bank1")
	AM_RANGE(0x9000, 0x9fff) AM_ROMBANK("bank2")
	AM_RANGE(0xa000, 0xbfff) AM_RAM_WRITE(m62_tileram_w) AM_SHARE("m62_tileram")
	AM_RANGE(0xc000, 0xc0ff) AM_WRITEONLY AM_SHARE("spriteram")
	AM_RANGE(0xc800, 0xcfff) AM_RAM_WRITE(m62_textram_w) AM_SHARE("m62_textram")
	AM_RANGE(0xd000, 0xd000) AM_WRITE(m62_vscroll_low_w)
	AM_RANGE(0xd001, 0xd001) AM_WRITE(m62_hscroll_low_w)
	AM_RANGE(0xd002, 0xd002) AM_WRITE(spelunk2_gfxport_w)
	AM_RANGE(0xd003, 0xd003) AM_WRITE(spelunk2_bankswitch_w)
	AM_RANGE(0xe000, 0xefff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( youjyudn_map, AS_PROGRAM, 8, m62_state )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0x8000, 0xbfff) AM_ROMBANK("bank1")
	AM_RANGE(0xc000, 0xc0ff) AM_WRITEONLY AM_SHARE("spriteram")
	AM_RANGE(0xc800, 0xcfff) AM_RAM_WRITE(m62_textram_w) AM_SHARE("m62_textram")
	AM_RANGE(0xd000, 0xd7ff) AM_RAM_WRITE(m62_tileram_w) AM_SHARE("m62_tileram")
	AM_RANGE(0xe000, 0xefff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( youjyudn_io_map, AS_IO, 8, m62_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x00) AM_READ_PORT("SYSTEM") AM_DEVWRITE("irem_audio", irem_audio_device, cmd_w)
	AM_RANGE(0x01, 0x01) AM_READ_PORT("P1") AM_WRITE(m62_flipscreen_w)  /* + coin counters */
	AM_RANGE(0x02, 0x02) AM_READ_PORT("P2")
	AM_RANGE(0x03, 0x03) AM_READ_PORT("DSW1")
	AM_RANGE(0x04, 0x04) AM_READ_PORT("DSW2")
	AM_RANGE(0x80, 0x80) AM_WRITE(m62_hscroll_high_w)
	AM_RANGE(0x81, 0x81) AM_WRITE(m62_hscroll_low_w)
	AM_RANGE(0x83, 0x83) AM_WRITE(youjyudn_bankswitch_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( horizon_map, AS_PROGRAM, 8, m62_state )
	AM_RANGE(0x0000, 0xbfff) AM_ROM
	AM_RANGE(0xc000, 0xc1ff) AM_RAM AM_SHARE("spriteram")
	AM_RANGE(0xc800, 0xc83f) AM_RAM_WRITE(horizon_scrollram_w) AM_SHARE("scrollram")
	AM_RANGE(0xd000, 0xdfff) AM_RAM_WRITE(m62_tileram_w) AM_SHARE("m62_tileram")
	AM_RANGE(0xe000, 0xefff) AM_RAM
ADDRESS_MAP_END


static INPUT_PORTS_START( common )
	PORT_START("SYSTEM")
	/* Start 1 & 2 also restarts and freezes the game with stop mode on
	   and are used in test mode to enter and esc the various tests */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START2 )
	/* service coin must be active for 19 frames to be consistently recognized */
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE1 ) PORT_IMPULSE(19)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN ) /* probably unused */
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN ) /* probably unused */
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON1 )

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN ) /* probably unused */
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL

	PORT_START("DSW2")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Flip_Screen ) ) PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Cabinet ) ) PORT_DIPLOCATION("SW2:2")
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x04, 0x04, "Coin Mode" ) PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(    0x04, "Mode 1" )
	PORT_DIPSETTING(    0x00, "Mode 2" )
	/* Bits 4,5,6 are different in each game, see below */
	PORT_DIPUNUSED_DIPLOC( 0x38, 0x38, "SW2:4,5,6" )
	PORT_DIPNAME( 0x40, 0x40, "Invulnerability (Cheat)" ) PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_SERVICE_DIPLOC( 0x80, IP_ACTIVE_LOW, "SW2:8" )
INPUT_PORTS_END


static INPUT_PORTS_START( kungfum )
	PORT_INCLUDE( common )

	PORT_MODIFY("DSW2")
	/* In slowmo mode, press 2 to slow game speed */
	PORT_DIPNAME( 0x08, 0x08, "Slow Motion Mode (Cheat)" ) PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	/* In freeze mode, press 2 to stop and 1 to restart */
	PORT_DIPNAME( 0x10, 0x10, "Freeze (Cheat)" ) PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	/* In level selection mode, press 1 to select and 2 to restart */
	PORT_DIPNAME( 0x20, 0x20, "Level Selection Mode (Cheat)" ) PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hard ) )
	PORT_DIPNAME( 0x02, 0x02, "Energy Loss" ) PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(    0x02, "Slow" )
	PORT_DIPSETTING(    0x00, "Fast" )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW1:3,4")
	PORT_DIPSETTING(    0x08, "2" )
	PORT_DIPSETTING(    0x0c, "3" )
	PORT_DIPSETTING(    0x04, "4" )
	PORT_DIPSETTING(    0x00, "5" )
	/* Manual says that only coin mode 1 is available and SW2:3 should be always OFF */
	/* However, coin mode 2 works perfectly. */
	IREM_Z80_COINAGE_TYPE_3_LOC(SW1)
INPUT_PORTS_END

static INPUT_PORTS_START( battroad )
	PORT_INCLUDE( common )

	PORT_MODIFY("DSW2")
	PORT_DIPUNKNOWN_DIPLOC( 0x08, 0x08, "SW2:4" )
	/* In stop mode, press 2 to stop and 1 to restart */
	PORT_DIPNAME( 0x10, 0x10, "Stop Mode (Cheat)" ) PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x03, 0x03, "Fuel Decrease" ) PORT_DIPLOCATION("SW1:1,2")
	PORT_DIPSETTING(    0x03, "Slow" )
	PORT_DIPSETTING(    0x02, DEF_STR( Medium ) )
	PORT_DIPSETTING(    0x01, "Fast" )
	PORT_DIPSETTING(    0x00, "Fastest" )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(    0x04, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hard ) )
	PORT_DIPUNKNOWN_DIPLOC( 0x08, 0x08, "SW1:4" )
	IREM_Z80_COINAGE_TYPE_3_LOC(SW1)
INPUT_PORTS_END

static INPUT_PORTS_START( ldrun )
	PORT_INCLUDE( common )

	PORT_MODIFY("DSW2")
	PORT_DIPUNKNOWN_DIPLOC( 0x08, 0x08, "SW2:4" )
	/* In stop mode, press 2 to stop and 1 to restart */
	PORT_DIPNAME( 0x10, 0x10, "Stop Mode (Cheat)" ) PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	/* In level selection mode, press 1 to select and 2 to restart */
	PORT_DIPNAME( 0x20, 0x20, "Level Selection Mode (Cheat)" ) PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x03, 0x03, "Timer" ) PORT_DIPLOCATION("SW1:1,2")
	PORT_DIPSETTING(    0x03, "Slow" )
	PORT_DIPSETTING(    0x02, DEF_STR( Medium ) )
	PORT_DIPSETTING(    0x01, "Fast" )
	PORT_DIPSETTING(    0x00, "Fastest" )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW1:3,4")
	PORT_DIPSETTING(    0x08, "2" )
	PORT_DIPSETTING(    0x0c, "3" )
	PORT_DIPSETTING(    0x04, "4" )
	PORT_DIPSETTING(    0x00, "5" )
	IREM_Z80_COINAGE_TYPE_3_LOC(SW1)
INPUT_PORTS_END

static INPUT_PORTS_START( ldrun2 )
	PORT_INCLUDE( common )

	PORT_MODIFY("DSW2")
	PORT_DIPUNKNOWN_DIPLOC( 0x08, 0x08, "SW2:4" )
	/* In freeze mode, press 2 to stop and 1 to restart */
	PORT_DIPNAME( 0x10, 0x10, "Freeze (Cheat)" ) PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	/* In level selection mode, press 1 to select and 2 to restart */
	PORT_DIPNAME( 0x20, 0x20, "Level Selection Mode (Cheat)" ) PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x01, "Timer" ) PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x01, "Slow" )
	PORT_DIPSETTING(    0x00, "Fast" )
	PORT_DIPNAME( 0x02, 0x02, "Game Speed" ) PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(    0x00, DEF_STR( Low ) )
	PORT_DIPSETTING(    0x02, DEF_STR( High ) )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW1:3,4")
	PORT_DIPSETTING(    0x08, "2" )
	PORT_DIPSETTING(    0x0c, "3" )
	PORT_DIPSETTING(    0x04, "4" )
	PORT_DIPSETTING(    0x00, "5" )
	IREM_Z80_COINAGE_TYPE_3_LOC(SW1)
INPUT_PORTS_END

static INPUT_PORTS_START( ldrun3 )
	PORT_INCLUDE( common )

	PORT_MODIFY("DSW2")
	PORT_DIPUNKNOWN_DIPLOC( 0x08, 0x08, "SW2:4" )
	/* In stop mode, press 2 to stop and 1 to restart */
	PORT_DIPNAME( 0x10, 0x10, "Stop Mode (Cheat)" ) PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	/* In level selection mode, press 1 to select and 2 to restart */
	PORT_DIPNAME( 0x20, 0x20, "Level Selection Mode (Cheat)" ) PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x01, "Timer" ) PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x01, "Slow" )
	PORT_DIPSETTING(    0x00, "Fast" )
	PORT_DIPNAME( 0x02, 0x02, "Game Speed" ) PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(    0x00, DEF_STR( Low ) )
	PORT_DIPSETTING(    0x02, DEF_STR( High ) )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW1:3,4")
	PORT_DIPSETTING(    0x08, "2" )
	PORT_DIPSETTING(    0x0c, "3" )
	PORT_DIPSETTING(    0x04, "4" )
	PORT_DIPSETTING(    0x00, "5" )
	IREM_Z80_COINAGE_TYPE_3_LOC(SW1)
INPUT_PORTS_END

static INPUT_PORTS_START( ldrun4 )
	PORT_INCLUDE( common )

	PORT_MODIFY("DSW2")
	PORT_DIPNAME( 0x02, 0x02, "2 Players Lives" ) PORT_DIPLOCATION("SW2:2")
	PORT_DIPSETTING(    0x02, "5" )
	PORT_DIPSETTING(    0x00, "6" )
	PORT_DIPUNKNOWN_DIPLOC( 0x08, 0x08, "SW2:4" )
	PORT_DIPNAME( 0x10, 0x10, "Allow 2 Players Game" ) PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Yes ) )
	/* In level selection mode, press 1 to select and 2 to restart */
	PORT_DIPNAME( 0x20, 0x20, "Level Selection Mode (Cheat)" ) PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Service Mode (must set 2P game to No)" ) PORT_TOGGLE PORT_CODE(KEYCODE_F2) PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x01, "Timer" ) PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x01, "Slow" )
	PORT_DIPSETTING(    0x00, "Fast" )
	PORT_DIPNAME( 0x02, 0x02, "2 Players Game" ) PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(    0x00, "1 Credit" )
	PORT_DIPSETTING(    0x02, "2 Credits" )
	PORT_DIPNAME( 0x0c, 0x0c, "1 Player Lives" ) PORT_DIPLOCATION("SW1:3,4")
	PORT_DIPSETTING(    0x08, "2" )
	PORT_DIPSETTING(    0x0c, "3" )
	PORT_DIPSETTING(    0x04, "4" )
	PORT_DIPSETTING(    0x00, "5" )
	IREM_Z80_COINAGE_TYPE_3_LOC(SW1)
INPUT_PORTS_END

static INPUT_PORTS_START( lotlot )
	PORT_INCLUDE( common )

	PORT_MODIFY("DSW2")
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	/* In freeze mode, press 2 to stop and 1 to restart */
	PORT_DIPNAME( 0x10, 0x10, "Freeze (Cheat)" ) PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPUNKNOWN_DIPLOC( 0x20, 0x20, "SW2:6" )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x03, 0x03, "Speed" ) PORT_DIPLOCATION("SW1:1,2")
	PORT_DIPSETTING(    0x03, "Very Slow" )
	PORT_DIPSETTING(    0x02, "Slow" )
	PORT_DIPSETTING(    0x01, "Fast" )
	PORT_DIPSETTING(    0x00, "Very Fast" )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW1:3,4")
	PORT_DIPSETTING(    0x08, "1" )
	PORT_DIPSETTING(    0x04, "2" )
	PORT_DIPSETTING(    0x0c, "3" )
	PORT_DIPSETTING(    0x00, "4" )
	IREM_Z80_COINAGE_TYPE_4_LOC(SW1)
INPUT_PORTS_END

static INPUT_PORTS_START( kidniki )
	PORT_INCLUDE( common )

	PORT_MODIFY("DSW2")
	PORT_DIPNAME( 0x08, 0x08, "Game Repeats" ) PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(    0x08, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Allow_Continue ) ) PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Yes ) )
	/* In freeze mode, press 2 to stop and 1 to restart */
	PORT_DIPNAME( 0x20, 0x20, "Freeze (Cheat)" ) PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW1:1,2")
	PORT_DIPSETTING(    0x02, "2" )
	PORT_DIPSETTING(    0x03, "3" )
	PORT_DIPSETTING(    0x01, "4" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(    0x04, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hard ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Bonus_Life ) ) PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(    0x08, "50000" )
	PORT_DIPSETTING(    0x00, "80000" )
	IREM_Z80_COINAGE_TYPE_4_LOC(SW1)
INPUT_PORTS_END

static INPUT_PORTS_START( spelunkr )
	PORT_INCLUDE( common )

	PORT_MODIFY("DSW2")
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Allow_Continue ) ) PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(    0x08, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	/* In teleport mode, keep 1 pressed and press up or down to move the character */
	PORT_DIPNAME( 0x10, 0x10, "Teleport (Cheat)" ) PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	/* In freeze mode, press 2 to stop and 1 to restart */
	PORT_DIPNAME( 0x20, 0x20, "Freeze (Cheat)" ) PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x03, 0x03, "Energy Decrease" ) PORT_DIPLOCATION("SW1:1,2")
	PORT_DIPSETTING(    0x03, "Slow" )
	PORT_DIPSETTING(    0x02, DEF_STR( Medium ) )
	PORT_DIPSETTING(    0x01, "Fast" )
	PORT_DIPSETTING(    0x00, "Fastest" )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW1:3,4")
	PORT_DIPSETTING(    0x08, "2" )
	PORT_DIPSETTING(    0x0c, "3" )
	PORT_DIPSETTING(    0x04, "4" )
	PORT_DIPSETTING(    0x00, "5" )
	IREM_Z80_COINAGE_TYPE_4_LOC(SW1)
INPUT_PORTS_END

static INPUT_PORTS_START( spelunk2 )
	PORT_INCLUDE( common )

	/* Factory shipment setting is all OFF */
	PORT_MODIFY("DSW2")
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Allow_Continue ) ) PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	/* In freeze mode, press 2 to stop and 1 to restart */
	PORT_DIPNAME( 0x20, 0x20, "Freeze (Cheat)" ) PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x03, 0x03, "Energy Decrease" ) PORT_DIPLOCATION("SW1:1,2")
	PORT_DIPSETTING(    0x03, "Slow" )
	PORT_DIPSETTING(    0x02, DEF_STR( Medium ) )
	PORT_DIPSETTING(    0x01, "Fast" )
	PORT_DIPSETTING(    0x00, "Fastest" )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW1:3,4")
	PORT_DIPSETTING(    0x08, "2" )
	PORT_DIPSETTING(    0x0c, "3" )
	PORT_DIPSETTING(    0x04, "4" )
	PORT_DIPSETTING(    0x00, "5" )
	IREM_Z80_COINAGE_TYPE_4_LOC(SW1)
INPUT_PORTS_END

static INPUT_PORTS_START( youjyudn )
	PORT_INCLUDE( common )

	PORT_MODIFY("DSW2")
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Bonus_Life ) ) PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(    0x08, "20000 60000" )
	PORT_DIPSETTING(    0x00, "40000 80000" )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	/* In freeze mode, press 2 to stop and 1 to restart */
	PORT_DIPNAME( 0x20, 0x20, "Freeze (Cheat)" ) PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW1:1,2")
	PORT_DIPSETTING(    0x02, "2" )
	PORT_DIPSETTING(    0x03, "3" )
	PORT_DIPSETTING(    0x01, "4" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPUNKNOWN_DIPLOC( 0x04, 0x04, "SW1:3" )
	PORT_DIPUNKNOWN_DIPLOC( 0x08, 0x08, "SW1:4" )
	IREM_Z80_COINAGE_TYPE_4_LOC(SW1)
INPUT_PORTS_END

static INPUT_PORTS_START( horizon )
	PORT_INCLUDE( common )

	PORT_MODIFY("DSW2")
	/* In freeze mode, press 2 to stop and 1 to restart */
	PORT_DIPNAME( 0x08, 0x08, "Freeze (Cheat)" ) PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	/* In frame advance mode, press 1 then 2 to advance a frame */
	PORT_DIPNAME( 0x10, 0x10, "Frame Advance (Cheat)" ) PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPUNKNOWN_DIPLOC( 0x20, 0x20, "SW2:6" )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW1:1,2")
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPSETTING(    0x03, "3" )
	PORT_DIPSETTING(    0x01, "4" )
	PORT_DIPSETTING(    0x02, "5" )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Bonus_Life ) )  PORT_DIPLOCATION("SW1:3,4")  // as per the service mode
	PORT_DIPSETTING(    0x00, "100 and 80k" )                                       // this one is blank but verified manually
	PORT_DIPSETTING(    0x0c, "40k and every 80k" )
	PORT_DIPSETTING(    0x08, "60k and every 100k" )
	PORT_DIPSETTING(    0x04, "80k and every 120k" )
	IREM_Z80_COINAGE_TYPE_5_LOC(SW1)
INPUT_PORTS_END


#define TILELAYOUT(NUM) static const gfx_layout tilelayout_##NUM =  \
{                                                                   \
	8,8,    /* 8*8 characters */                                    \
	NUM,    /* NUM characters */                                    \
	3,  /* 3 bits per pixel */                                      \
	{ 2*NUM*8*8, NUM*8*8, 0 },                                      \
	{ 0, 1, 2, 3, 4, 5, 6, 7 },                                     \
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },                     \
	8*8 /* every char takes 8 consecutive bytes */                  \
}

TILELAYOUT(1024);
TILELAYOUT(2048);
TILELAYOUT(4096);


static const gfx_layout battroad_charlayout =
{
	8,8,    /* 8*8 characters */
	1024,   /* number of characters */
	2,  /* 2 bits per pixel */
	{ 0, 1024*8*8 },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8 /* every char takes 8 consecutive bytes */
};

static const gfx_layout lotlot_charlayout =
{
	12,10, /* character size */
	256, /* number of characters */
	3, /* bits per pixel */
	{ 0, 256*32*8, 2*256*32*8 },
	{ 0, 1, 2, 3, 16*8+0, 16*8+1, 16*8+2, 16*8+3, 16*8+4, 16*8+5, 16*8+6, 16*8+7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8, 8*8, 9*8 },
	32*8    /* every char takes 32 consecutive bytes */
};

static const gfx_layout kidniki_charlayout =
{
	12,8, /* character size */
	1024, /* number of characters */
	3, /* bits per pixel */
	{ 0, 0x4000*8, 2*0x4000*8 },
	{ 0, 1, 2, 3, 64+0,64+1,64+2,64+3,64+4,64+5,64+6,64+7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	16*8    /* every char takes 16 consecutive bytes */
};

static const gfx_layout spelunk2_charlayout =
{
	12,8, /* character size */
	512, /* number of characters */
	3, /* bits per pixel */
	{ 0, 0x4000*8, 2*0x4000*8 },
	{
		0,1,2,3,
		0x2000*8+0,0x2000*8+1,0x2000*8+2,0x2000*8+3,
		0x2000*8+4,0x2000*8+5,0x2000*8+6,0x2000*8+7
	},
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8 /* every char takes 8 consecutive bytes */
};

static const gfx_layout youjyudn_tilelayout =
{
	8,16,
	RGN_FRAC(1,3),
	3,
	{ RGN_FRAC(2,3), RGN_FRAC(1,3), RGN_FRAC(0,3) },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8,
			8*8, 9*8, 10*8, 11*8, 12*8, 13*8, 14*8, 15*8 },
	16*8
};

static const gfx_layout spritelayout =
{
	16,16,
	RGN_FRAC(1,3),
	3,
	{ RGN_FRAC(2,3), RGN_FRAC(1,3), RGN_FRAC(0,3) },
	{ 0, 1, 2, 3, 4, 5, 6, 7,
			16*8+0, 16*8+1, 16*8+2, 16*8+3, 16*8+4, 16*8+5, 16*8+6, 16*8+7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8,
			8*8, 9*8, 10*8, 11*8, 12*8, 13*8, 14*8, 15*8 },
	32*8
};

static GFXDECODE_START( ldrun )
	GFXDECODE_ENTRY( "gfx1", 0, tilelayout_1024,       0, 32 )  /* use colors   0-255 */
	GFXDECODE_ENTRY( "gfx2", 0, spritelayout,        256, 32 )  /* use colors 256-511 */
GFXDECODE_END

static GFXDECODE_START( battroad )
	GFXDECODE_ENTRY( "gfx1", 0, tilelayout_1024,       0, 32 )  /* use colors   0-255 */
	GFXDECODE_ENTRY( "gfx2", 0, spritelayout,        256, 32 )  /* use colors 256-511 */
	GFXDECODE_ENTRY( "gfx3", 0, battroad_charlayout,    512, 32 )   /* use colors 512-543 */
GFXDECODE_END

static GFXDECODE_START( ldrun3 )
	GFXDECODE_ENTRY( "gfx1", 0, tilelayout_2048,      0, 32 )   /* use colors   0-255 */
	GFXDECODE_ENTRY( "gfx2", 0, spritelayout,       256, 32 )   /* use colors 256-511 */
GFXDECODE_END

static GFXDECODE_START( lotlot )
	GFXDECODE_ENTRY( "gfx1", 0, lotlot_charlayout,    0, 32 )   /* use colors   0-255 */
	GFXDECODE_ENTRY( "gfx2", 0, spritelayout,       256, 32 )   /* use colors 256-511 */
	GFXDECODE_ENTRY( "gfx3", 0, lotlot_charlayout,  512, 32 )   /* use colors 512-767 */
GFXDECODE_END

static GFXDECODE_START( kidniki )
	GFXDECODE_ENTRY( "gfx1", 0, tilelayout_4096,      0, 32 )   /* use colors   0-255 */
	GFXDECODE_ENTRY( "gfx2", 0, spritelayout,       256, 32 )   /* use colors 256-511 */
	GFXDECODE_ENTRY( "gfx3", 0, kidniki_charlayout,   0, 32 )   /* use colors   0-255 */
GFXDECODE_END

static GFXDECODE_START( spelunkr )
	GFXDECODE_ENTRY( "gfx1", 0, tilelayout_4096,         0, 32 )    /* use colors   0-255 */
	GFXDECODE_ENTRY( "gfx2", 0, spritelayout,       256, 32 )   /* use colors 256-511 */
	GFXDECODE_ENTRY( "gfx3", 0, spelunk2_charlayout,  0, 32 )   /* use colors   0-255 */
GFXDECODE_END

static GFXDECODE_START( spelunk2 )
	GFXDECODE_ENTRY( "gfx1", 0, tilelayout_4096,         0, 64 )    /* use colors   0-511 */
	GFXDECODE_ENTRY( "gfx2", 0, spritelayout,       512, 32 )   /* use colors 512-767 */
	GFXDECODE_ENTRY( "gfx3", 0, spelunk2_charlayout,  0, 64 )   /* use colors   0-511 */
GFXDECODE_END

static GFXDECODE_START( youjyudn )
	GFXDECODE_ENTRY( "gfx1", 0, youjyudn_tilelayout,  0, 32 )   /* use colors   0-255 */
	GFXDECODE_ENTRY( "gfx2", 0, spritelayout,       256, 32 )   /* use colors 256-511 */
	GFXDECODE_ENTRY( "gfx3", 0, kidniki_charlayout, 128, 16 )   /* use colors 128-255 */
GFXDECODE_END


void m62_state::machine_start()
{
	save_item(NAME(m_ldrun2_bankswap));
	save_item(NAME(m_bankcontrol));
}

void m62_state::machine_reset()
{
	m_flipscreen = 0;
	m_m62_background_hscroll = 0;
	m_m62_background_vscroll = 0;
	m_kidniki_background_bank = 0;
	m_kidniki_text_vscroll = 0;
	m_ldrun3_topbottom_mask = 0;
	m_spelunkr_palbank = 0;

	m_ldrun2_bankswap = 0;
	m_bankcontrol[0] = 0;
	m_bankcontrol[1] = 0;
}

static MACHINE_CONFIG_START( ldrun, m62_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, 24000000/6)
	MCFG_CPU_PROGRAM_MAP(ldrun_map)
	MCFG_CPU_IO_MAP(kungfum_io_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", m62_state,  irq0_line_hold)


	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(55)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(1790) /* frames per second and vblank duration from the Lode Runner manual */)
	MCFG_SCREEN_SIZE(64*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA((64*8-384)/2, 64*8-(64*8-384)/2-1, 0*8, 32*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(m62_state, screen_update_ldrun)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", ldrun)
	MCFG_PALETTE_ADD("palette", 512)
	MCFG_PALETTE_INIT_OWNER(m62_state,m62)

	/* sound hardware */
	//MCFG_FRAGMENT_ADD(m62_audio)
	MCFG_DEVICE_ADD("irem_audio", IREM_M62_AUDIO, 0)

MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( kungfum, ldrun )

	/* basic machine hardware */
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_CLOCK(18432000/6)
	MCFG_CPU_PROGRAM_MAP(kungfum_map)
	MCFG_CPU_IO_MAP(kungfum_io_map)

	/* video hardware */
	MCFG_SCREEN_MODIFY("screen")
	MCFG_SCREEN_VISIBLE_AREA((64*8-256)/2, 64*8-(64*8-256)/2-1, 0*8, 32*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(m62_state, screen_update_kungfum)

	MCFG_VIDEO_START_OVERRIDE(m62_state,kungfum)
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( battroad, ldrun )

	/* basic machine hardware */
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_CLOCK(18432000/6)
	MCFG_CPU_PROGRAM_MAP(battroad_map)
	MCFG_CPU_IO_MAP(battroad_io_map)

	/* video hardware */
	MCFG_SCREEN_MODIFY("screen")
	MCFG_SCREEN_VISIBLE_AREA((64*8-256)/2, 64*8-(64*8-256)/2-1, 0*8, 32*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(m62_state, screen_update_battroad)
	MCFG_GFXDECODE_MODIFY("gfxdecode", battroad)
	MCFG_PALETTE_MODIFY("palette")
	MCFG_PALETTE_ENTRIES(544)

	MCFG_PALETTE_INIT_OWNER(m62_state,battroad)
	MCFG_VIDEO_START_OVERRIDE(m62_state,battroad)
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( ldrun2, ldrun )

	/* basic machine hardware */
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(ldrun2_map)
	MCFG_CPU_IO_MAP(ldrun2_io_map)

	MCFG_VIDEO_START_OVERRIDE(m62_state,ldrun2)
	MCFG_SCREEN_MODIFY("screen")
	MCFG_SCREEN_UPDATE_DRIVER(m62_state, screen_update_ldrun)
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( ldrun3, ldrun )

	/* basic machine hardware */
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(ldrun3_map)
	MCFG_CPU_IO_MAP(ldrun3_io_map)

	/* video hardware */
	MCFG_GFXDECODE_MODIFY("gfxdecode", ldrun3)
	MCFG_VIDEO_START_OVERRIDE(m62_state,ldrun2)
	MCFG_SCREEN_MODIFY("screen")
	MCFG_SCREEN_UPDATE_DRIVER(m62_state, screen_update_ldrun3)
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( ldrun4, ldrun )

	/* basic machine hardware */
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(ldrun4_map)
	MCFG_CPU_IO_MAP(ldrun4_io_map)

	/* video hardware */
	MCFG_GFXDECODE_MODIFY("gfxdecode", ldrun3)
	MCFG_VIDEO_START_OVERRIDE(m62_state,ldrun4)
	MCFG_SCREEN_MODIFY("screen")
	MCFG_SCREEN_UPDATE_DRIVER(m62_state, screen_update_ldrun4)
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( lotlot, ldrun )

	/* basic machine hardware */
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(lotlot_map)

	/* video hardware */
	MCFG_GFXDECODE_MODIFY("gfxdecode", lotlot)

	MCFG_PALETTE_MODIFY("palette")
	MCFG_PALETTE_ENTRIES(768)

	MCFG_PALETTE_INIT_OWNER(m62_state,lotlot)
	MCFG_VIDEO_START_OVERRIDE(m62_state,lotlot)
	MCFG_SCREEN_MODIFY("screen")
	MCFG_SCREEN_UPDATE_DRIVER(m62_state, screen_update_lotlot)
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( kidniki, ldrun )

	/* basic machine hardware */
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(kidniki_map)
	MCFG_CPU_IO_MAP(kidniki_io_map)

	/* video hardware */
	MCFG_GFXDECODE_MODIFY("gfxdecode", kidniki)

	MCFG_VIDEO_START_OVERRIDE(m62_state,kidniki)
	MCFG_SCREEN_MODIFY("screen")
	MCFG_SCREEN_UPDATE_DRIVER(m62_state, screen_update_kidniki)
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( spelunkr, ldrun )

	/* basic machine hardware */
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(spelunkr_map)

	/* video hardware */
	MCFG_GFXDECODE_MODIFY("gfxdecode", spelunkr)

	MCFG_VIDEO_START_OVERRIDE(m62_state,spelunkr)
	MCFG_SCREEN_MODIFY("screen")
	MCFG_SCREEN_UPDATE_DRIVER(m62_state, screen_update_spelunkr)
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( spelunk2, ldrun )

	/* basic machine hardware */
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(spelunk2_map)

	/* video hardware */
	MCFG_GFXDECODE_MODIFY("gfxdecode", spelunk2)
	MCFG_PALETTE_MODIFY("palette")
	MCFG_PALETTE_ENTRIES(768)

	MCFG_PALETTE_INIT_OWNER(m62_state,spelunk2)
	MCFG_VIDEO_START_OVERRIDE(m62_state,spelunk2)
	MCFG_SCREEN_MODIFY("screen")
	MCFG_SCREEN_UPDATE_DRIVER(m62_state, screen_update_spelunk2)
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( youjyudn, ldrun )

	/* basic machine hardware */
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_CLOCK(18432000/6)
	MCFG_CPU_PROGRAM_MAP(youjyudn_map)
	MCFG_CPU_IO_MAP(youjyudn_io_map)

	/* video hardware */
	MCFG_SCREEN_MODIFY("screen")
	MCFG_SCREEN_VISIBLE_AREA((64*8-256)/2, 64*8-(64*8-256)/2-1, 0*8, 32*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(m62_state, screen_update_youjyudn)
	MCFG_GFXDECODE_MODIFY("gfxdecode", youjyudn)

	MCFG_VIDEO_START_OVERRIDE(m62_state,youjyudn)
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( horizon, ldrun )

	/* basic machine hardware */
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(horizon_map)

	/* video hardware */
	MCFG_SCREEN_MODIFY("screen")
	MCFG_SCREEN_VISIBLE_AREA((64*8-256)/2, 64*8-(64*8-256)/2-1, 0*8, 32*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(m62_state, screen_update_horizon)

	MCFG_VIDEO_START_OVERRIDE(m62_state,horizon)
MACHINE_CONFIG_END


/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( kungfum )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "a-4e-c.bin",   0x0000, 0x4000, CRC(b6e2d083) SHA1(17e2cfe2b9d6121239803aba7132918e54ae02bf) )
	ROM_LOAD( "a-4d-c.bin",   0x4000, 0x4000, CRC(7532918e) SHA1(9d513d5a3b99cc54c4491371cd44af048ef0fb33) )

	ROM_REGION( 0x10000, "irem_audio:iremsound", 0 )   /* 64k for the audio CPU (6803) */
	ROM_LOAD( "a-3e-.bin",    0xa000, 0x2000, CRC(58e87ab0) SHA1(3b03c101fec58eac13fc309a78df9a2cd44f7604) )    /* samples (ADPCM 4-bit) */
	ROM_LOAD( "a-3f-.bin",    0xc000, 0x2000, CRC(c81e31ea) SHA1(f0fc58b929188c8802cd85549bdf9f4566e6a677) )    /* samples (ADPCM 4-bit) */
	ROM_LOAD( "a-3h-.bin",    0xe000, 0x2000, CRC(d99fb995) SHA1(caa6acdbc3b02d248fd123be95ea6fdcb4f35b59) )

	ROM_REGION( 0x06000, "gfx1", 0 )
	ROM_LOAD( "g-4c-a.bin",   0x00000, 0x2000, CRC(6b2cc9c8) SHA1(ba7c902d08c21a1e33f450406bfbfa35abde3b3f) )   /* characters */
	ROM_LOAD( "g-4d-a.bin",   0x02000, 0x2000, CRC(c648f558) SHA1(7cc085d8dc4a770d2828e39859b7b18e80148a00) )
	ROM_LOAD( "g-4e-a.bin",   0x04000, 0x2000, CRC(fbe9276e) SHA1(84181c8da79e2c92af04aef3ab5d23f70969dad8) )

	ROM_REGION( 0x18000, "gfx2", 0 )
	ROM_LOAD( "b-4k-.bin",    0x00000, 0x2000, CRC(16fb5150) SHA1(a49faf617f948d3ccec2bc6ef97bd399f0958f65) )   /* sprites */
	ROM_LOAD( "b-4f-.bin",    0x02000, 0x2000, CRC(67745a33) SHA1(fcc642fb1b932676c84c1a0901b989673c57c0e5) )
	ROM_LOAD( "b-4l-.bin",    0x04000, 0x2000, CRC(bd1c2261) SHA1(7155789a01801a9e1a55d4e68c94a3a3ee7d1b2e) )
	ROM_LOAD( "b-4h-.bin",    0x06000, 0x2000, CRC(8ac5ed3a) SHA1(9c88e8c82420428b43923cdee7eb4504882bec69) )
	ROM_LOAD( "b-3n-.bin",    0x08000, 0x2000, CRC(28a213aa) SHA1(0d6d668490bdf4394bc9fed2f3cdc72f2fea46f9) )
	ROM_LOAD( "b-4n-.bin",    0x0a000, 0x2000, CRC(d5228df3) SHA1(836c4f95f873fbf07f9bec63a72c20a14651117c) )
	ROM_LOAD( "b-4m-.bin",    0x0c000, 0x2000, CRC(b16de4f2) SHA1(512260e76c9cd21b8add771de53fbd27c2719213) )
	ROM_LOAD( "b-3m-.bin",    0x0e000, 0x2000, CRC(eba0d66b) SHA1(028f82fc1853b86a3201b24871f41091c3e0b542) )
	ROM_LOAD( "b-4c-.bin",    0x10000, 0x2000, CRC(01298885) SHA1(d4edf5fe707c5b7231ba72b731b96120064a7ecd) )
	ROM_LOAD( "b-4e-.bin",    0x12000, 0x2000, CRC(c77b87d4) SHA1(c0f66f0130f6a290a58a3d77bba1d06f16016901) )
	ROM_LOAD( "b-4d-.bin",    0x14000, 0x2000, CRC(6a70615f) SHA1(f4683dc0a566567e95e85268612bcf0e6297d955) )
	ROM_LOAD( "b-4a-.bin",    0x16000, 0x2000, CRC(6189d626) SHA1(ce8e5e95c2684c685481e9c8d921380b20ac0460) )

	ROM_REGION( 0x0720, "proms", 0 )
	ROM_LOAD( "g-1j-.bin",    0x0000, 0x0100, CRC(668e6bca) SHA1(cd5262b1310821ba7b12873e4db35f081d6b9df4) )    /* character palette red component */
	ROM_LOAD( "b-1m-.bin",    0x0100, 0x0100, CRC(76c05a9c) SHA1(1f46f436a17f8c883bdd6d9804b828a81a76f880) )    /* sprite palette red component */
	ROM_LOAD( "g-1f-.bin",    0x0200, 0x0100, CRC(964b6495) SHA1(76f30a65a0ded14babad2006221aa40621fb7ea1) )    /* character palette green component */
	ROM_LOAD( "b-1n-.bin",    0x0300, 0x0100, CRC(23f06b99) SHA1(6b3d6349f019aeab33838ae392bc3f3f89906326) )    /* sprite palette green component */
	ROM_LOAD( "g-1h-.bin",    0x0400, 0x0100, CRC(550563e1) SHA1(11edb45acba8b28a462c49956ebb1ba0a8b2ff26) )    /* character palette blue component */
	ROM_LOAD( "b-1l-.bin",    0x0500, 0x0100, CRC(35e45021) SHA1(511b94507f41b377f38184ed9a85f34949b28d26) )    /* sprite palette blue component */
	ROM_LOAD( "b-5f-.bin",    0x0600, 0x0020, CRC(7a601c3d) SHA1(5c5cdf51b2c9fdb2b05402d9c260208ae73fe245) )    /* sprite height, one entry per 32 */
															/* sprites. Used at run time! */
	ROM_LOAD( "b-6f-.bin",    0x0620, 0x0100, CRC(82c20d12) SHA1(268903f7d9be58a70d030b02bf31a2d6b5b6e249) )    /* video timing - same as battroad */
ROM_END

ROM_START( kungfumd )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "snx_a-4e-d", 0x0000, 0x4000, CRC(fc330a46) SHA1(50edbd6131310afa17d476e278a7098ab1cfae73) )
	ROM_LOAD( "snx_a-4d-d", 0x4000, 0x4000, CRC(1b2fd32f) SHA1(34487dacf2ec9fbf55148953a9f248fae9442568) )

	ROM_REGION( 0x10000, "irem_audio:iremsound", 0 )   /* 64k for the audio CPU (6803) */
	ROM_LOAD( "snx_a-3d-b", 0x8000, 0x4000, CRC(85ca7956) SHA1(f97f744520770766cf9633c8762013d77b0e7a61) )  /* samples (ADPCM 4-bit) */
	ROM_LOAD( "snx_a-3f-b", 0xc000, 0x4000, CRC(3ef1100a) SHA1(901c40477b4c6fbc984446fa7171006d67b48652) )  /* samples (ADPCM 4-bit) */

	ROM_REGION( 0x06000, "gfx1", 0 )
	ROM_LOAD( "g-4c-a.bin", 0x00000, 0x2000, CRC(6b2cc9c8) SHA1(ba7c902d08c21a1e33f450406bfbfa35abde3b3f) ) /* characters */
	ROM_LOAD( "g-4d-a.bin", 0x02000, 0x2000, CRC(c648f558) SHA1(7cc085d8dc4a770d2828e39859b7b18e80148a00) )
	ROM_LOAD( "g-4e-a.bin", 0x04000, 0x2000, CRC(fbe9276e) SHA1(84181c8da79e2c92af04aef3ab5d23f70969dad8) )

	ROM_REGION( 0x18000, "gfx2", 0 )
	ROM_LOAD( "snx_b-4k-b", 0x00000, 0x4000, CRC(85591db2) SHA1(5962445a3a3172893cf0f617701966aebb522c61) ) /* sprites */
	ROM_LOAD( "snx_b-4f-b", 0x04000, 0x4000, CRC(ed719d7b) SHA1(34c7d9fc762b3f113b6115910898fde9713ae24f) )
	ROM_LOAD( "snx_b-3n-b", 0x08000, 0x4000, CRC(05fcce8b) SHA1(5116cd65c53aa09a1c1d84df883606883e565464) )
	ROM_LOAD( "snx_b-4n-b", 0x0c000, 0x4000, CRC(dc675003) SHA1(df9b6055b4a4c537504658e62e96f826d498b39c) )
	ROM_LOAD( "snx_b-4c-b", 0x10000, 0x4000, CRC(1df11d81) SHA1(5055f0c8046f57d4c899ecf90a9bfcf0c1a58f59) )
	ROM_LOAD( "snx_b-4e-b", 0x14000, 0x4000, CRC(2d3b69dd) SHA1(472e1c06fd3184b91d9b718bb590d45702ed84cd) )

	ROM_REGION( 0x0720, "proms", 0 )
	ROM_LOAD( "g-1j-.bin", 0x0000, 0x0100, CRC(668e6bca) SHA1(cd5262b1310821ba7b12873e4db35f081d6b9df4) )   /* character palette red component */
	ROM_LOAD( "b-1m-.bin", 0x0100, 0x0100, CRC(76c05a9c) SHA1(1f46f436a17f8c883bdd6d9804b828a81a76f880) )   /* sprite palette red component */
	ROM_LOAD( "g-1f-.bin", 0x0200, 0x0100, CRC(964b6495) SHA1(76f30a65a0ded14babad2006221aa40621fb7ea1) )   /* character palette green component */
	ROM_LOAD( "b-1n-.bin", 0x0300, 0x0100, CRC(23f06b99) SHA1(6b3d6349f019aeab33838ae392bc3f3f89906326) )   /* sprite palette green component */
	ROM_LOAD( "g-1h-.bin", 0x0400, 0x0100, CRC(550563e1) SHA1(11edb45acba8b28a462c49956ebb1ba0a8b2ff26) )   /* character palette blue component */
	ROM_LOAD( "b-1l-.bin", 0x0500, 0x0100, CRC(35e45021) SHA1(511b94507f41b377f38184ed9a85f34949b28d26) )   /* sprite palette blue component */
	ROM_LOAD( "b-5f-.bin", 0x0600, 0x0020, CRC(7a601c3d) SHA1(5c5cdf51b2c9fdb2b05402d9c260208ae73fe245) )   /* sprite height, one entry per 32 */
														/* sprites. Used at run time! */
	ROM_LOAD( "b-6f-.bin", 0x0620, 0x0100, CRC(82c20d12) SHA1(268903f7d9be58a70d030b02bf31a2d6b5b6e249) )   /* video timing - same as battroad */
ROM_END

ROM_START( spartanx )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "a-4e-c-j.bin", 0x0000, 0x4000, CRC(32a0a9a6) SHA1(fbb601a4c98a131013ea49a6877f2cd8139434fa) )
	ROM_LOAD( "a-4d-c-j.bin", 0x4000, 0x4000, CRC(3173ea78) SHA1(cb6102abf9cf8df531f6e808b59e076831456ff5) )

	ROM_REGION( 0x10000, "irem_audio:iremsound", 0 )   /* 64k for the audio CPU (6803) */
	ROM_LOAD( "a-3e-.bin",    0xa000, 0x2000, CRC(58e87ab0) SHA1(3b03c101fec58eac13fc309a78df9a2cd44f7604) )    /* samples (ADPCM 4-bit) */
	ROM_LOAD( "a-3f-.bin",    0xc000, 0x2000, CRC(c81e31ea) SHA1(f0fc58b929188c8802cd85549bdf9f4566e6a677) )    /* samples (ADPCM 4-bit) */
	ROM_LOAD( "a-3h-.bin",    0xe000, 0x2000, CRC(d99fb995) SHA1(caa6acdbc3b02d248fd123be95ea6fdcb4f35b59) )

	ROM_REGION( 0x06000, "gfx1", 0 )
	ROM_LOAD( "g-4c-a-j.bin", 0x00000, 0x2000, CRC(8af9c5a6) SHA1(3375ad92f230ea85d6db16ccfa8fec0832e8cc26) )   /* characters */
	ROM_LOAD( "g-4d-a-j.bin", 0x02000, 0x2000, CRC(b8300c72) SHA1(806be9da8a164d5f6a4cbd82deacb3fbd0032423) )
	ROM_LOAD( "g-4e-a-j.bin", 0x04000, 0x2000, CRC(b50429cd) SHA1(6fdaed316ec94fc5ccb560ed65c714151f8ee5fe) )

	ROM_REGION( 0x18000, "gfx2", 0 )
	ROM_LOAD( "b-4k-.bin",    0x00000, 0x2000, CRC(16fb5150) SHA1(a49faf617f948d3ccec2bc6ef97bd399f0958f65) )   /* sprites */
	ROM_LOAD( "b-4f-.bin",    0x02000, 0x2000, CRC(67745a33) SHA1(fcc642fb1b932676c84c1a0901b989673c57c0e5) )
	ROM_LOAD( "b-4l-.bin",    0x04000, 0x2000, CRC(bd1c2261) SHA1(7155789a01801a9e1a55d4e68c94a3a3ee7d1b2e) )
	ROM_LOAD( "b-4h-.bin",    0x06000, 0x2000, CRC(8ac5ed3a) SHA1(9c88e8c82420428b43923cdee7eb4504882bec69) )
	ROM_LOAD( "b-3n-.bin",    0x08000, 0x2000, CRC(28a213aa) SHA1(0d6d668490bdf4394bc9fed2f3cdc72f2fea46f9) )
	ROM_LOAD( "b-4n-.bin",    0x0a000, 0x2000, CRC(d5228df3) SHA1(836c4f95f873fbf07f9bec63a72c20a14651117c) )
	ROM_LOAD( "b-4m-.bin",    0x0c000, 0x2000, CRC(b16de4f2) SHA1(512260e76c9cd21b8add771de53fbd27c2719213) )
	ROM_LOAD( "b-3m-.bin",    0x0e000, 0x2000, CRC(eba0d66b) SHA1(028f82fc1853b86a3201b24871f41091c3e0b542) )
	ROM_LOAD( "b-4c-.bin",    0x10000, 0x2000, CRC(01298885) SHA1(d4edf5fe707c5b7231ba72b731b96120064a7ecd) )
	ROM_LOAD( "b-4e-.bin",    0x12000, 0x2000, CRC(c77b87d4) SHA1(c0f66f0130f6a290a58a3d77bba1d06f16016901) )
	ROM_LOAD( "b-4d-.bin",    0x14000, 0x2000, CRC(6a70615f) SHA1(f4683dc0a566567e95e85268612bcf0e6297d955) )
	ROM_LOAD( "b-4a-.bin",    0x16000, 0x2000, CRC(6189d626) SHA1(ce8e5e95c2684c685481e9c8d921380b20ac0460) )

	ROM_REGION( 0x0720, "proms", 0 )
	ROM_LOAD( "g-1j-.bin",    0x0000, 0x0100, CRC(668e6bca) SHA1(cd5262b1310821ba7b12873e4db35f081d6b9df4) )    /* character palette red component */
	ROM_LOAD( "b-1m-.bin",    0x0100, 0x0100, CRC(76c05a9c) SHA1(1f46f436a17f8c883bdd6d9804b828a81a76f880) )    /* sprite palette red component */
	ROM_LOAD( "g-1f-.bin",    0x0200, 0x0100, CRC(964b6495) SHA1(76f30a65a0ded14babad2006221aa40621fb7ea1) )    /* character palette green component */
	ROM_LOAD( "b-1n-.bin",    0x0300, 0x0100, CRC(23f06b99) SHA1(6b3d6349f019aeab33838ae392bc3f3f89906326) )    /* sprite palette green component */
	ROM_LOAD( "g-1h-.bin",    0x0400, 0x0100, CRC(550563e1) SHA1(11edb45acba8b28a462c49956ebb1ba0a8b2ff26) )    /* character palette blue component */
	ROM_LOAD( "b-1l-.bin",    0x0500, 0x0100, CRC(35e45021) SHA1(511b94507f41b377f38184ed9a85f34949b28d26) )    /* sprite palette blue component */
	ROM_LOAD( "b-5f-.bin",    0x0600, 0x0020, CRC(7a601c3d) SHA1(5c5cdf51b2c9fdb2b05402d9c260208ae73fe245) )    /* sprite height, one entry per 32 */
															/* sprites. Used at run time! */
	ROM_LOAD( "b-6f-.bin",    0x0620, 0x0100, CRC(82c20d12) SHA1(268903f7d9be58a70d030b02bf31a2d6b5b6e249) )    /* video timing - same as battroad */
ROM_END

ROM_START( kungfub )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "c5.5h",        0x0000, 0x4000, CRC(5d8e791d) SHA1(90cd911f715a50a90427abd89b38272a6df08d69) )
	ROM_LOAD( "c4.5k",        0x4000, 0x4000, CRC(4000e2b8) SHA1(719b0aa1fd0cbe671178ac728d76d439bd7932d9) )

	ROM_REGION( 0x10000, "irem_audio:iremsound", 0 )   /* 64k for the audio CPU (6803) */
	ROM_LOAD( "a-3e-.bin",    0xa000, 0x2000, CRC(58e87ab0) SHA1(3b03c101fec58eac13fc309a78df9a2cd44f7604) )    /* samples (ADPCM 4-bit) */
	ROM_LOAD( "a-3f-.bin",    0xc000, 0x2000, CRC(c81e31ea) SHA1(f0fc58b929188c8802cd85549bdf9f4566e6a677) )    /* samples (ADPCM 4-bit) */
	ROM_LOAD( "a-3h-.bin",    0xe000, 0x2000, CRC(d99fb995) SHA1(caa6acdbc3b02d248fd123be95ea6fdcb4f35b59) )

	ROM_REGION( 0x06000, "gfx1", 0 )
	ROM_LOAD( "g-4c-a.bin",   0x00000, 0x2000, CRC(6b2cc9c8) SHA1(ba7c902d08c21a1e33f450406bfbfa35abde3b3f) )   /* characters */
	ROM_LOAD( "g-4d-a.bin",   0x02000, 0x2000, CRC(c648f558) SHA1(7cc085d8dc4a770d2828e39859b7b18e80148a00) )
	ROM_LOAD( "g-4e-a.bin",   0x04000, 0x2000, CRC(fbe9276e) SHA1(84181c8da79e2c92af04aef3ab5d23f70969dad8) )

	ROM_REGION( 0x18000, "gfx2", 0 )
	ROM_LOAD( "b-4k-.bin",    0x00000, 0x2000, CRC(16fb5150) SHA1(a49faf617f948d3ccec2bc6ef97bd399f0958f65) )   /* sprites */
	ROM_LOAD( "b-4f-.bin",    0x02000, 0x2000, CRC(67745a33) SHA1(fcc642fb1b932676c84c1a0901b989673c57c0e5) )
	ROM_LOAD( "b-4l-.bin",    0x04000, 0x2000, CRC(bd1c2261) SHA1(7155789a01801a9e1a55d4e68c94a3a3ee7d1b2e) )
	ROM_LOAD( "b-4h-.bin",    0x06000, 0x2000, CRC(8ac5ed3a) SHA1(9c88e8c82420428b43923cdee7eb4504882bec69) )
	ROM_LOAD( "b-3n-.bin",    0x08000, 0x2000, CRC(28a213aa) SHA1(0d6d668490bdf4394bc9fed2f3cdc72f2fea46f9) )
	ROM_LOAD( "b-4n-.bin",    0x0a000, 0x2000, CRC(d5228df3) SHA1(836c4f95f873fbf07f9bec63a72c20a14651117c) )
	ROM_LOAD( "b-4m-.bin",    0x0c000, 0x2000, CRC(b16de4f2) SHA1(512260e76c9cd21b8add771de53fbd27c2719213) )
	ROM_LOAD( "b-3m-.bin",    0x0e000, 0x2000, CRC(eba0d66b) SHA1(028f82fc1853b86a3201b24871f41091c3e0b542) )
	ROM_LOAD( "b-4c-.bin",    0x10000, 0x2000, CRC(01298885) SHA1(d4edf5fe707c5b7231ba72b731b96120064a7ecd) )
	ROM_LOAD( "b-4e-.bin",    0x12000, 0x2000, CRC(c77b87d4) SHA1(c0f66f0130f6a290a58a3d77bba1d06f16016901) )
	ROM_LOAD( "b-4d-.bin",    0x14000, 0x2000, CRC(6a70615f) SHA1(f4683dc0a566567e95e85268612bcf0e6297d955) )
	ROM_LOAD( "b-4a-.bin",    0x16000, 0x2000, CRC(6189d626) SHA1(ce8e5e95c2684c685481e9c8d921380b20ac0460) )

	ROM_REGION( 0x14a0, "proms", 0 )
	ROM_LOAD( "tbp24s10-main-1c.bin", 0x0000, 0x0100, CRC(668e6bca) SHA1(cd5262b1310821ba7b12873e4db35f081d6b9df4) )    /* character palette red component */
	ROM_LOAD( "tbp24s10-gfx-1r.bin",  0x0100, 0x0100, CRC(76c05a9c) SHA1(1f46f436a17f8c883bdd6d9804b828a81a76f880) )    /* sprite palette red component */
	ROM_LOAD( "tbp24s10-main-1a.bin", 0x0200, 0x0100, CRC(964b6495) SHA1(76f30a65a0ded14babad2006221aa40621fb7ea1) )    /* character palette green component */
	ROM_LOAD( "tbp24s10-gfx-1s.bin",  0x0300, 0x0100, CRC(23f06b99) SHA1(6b3d6349f019aeab33838ae392bc3f3f89906326) )    /* sprite palette green component */
	ROM_LOAD( "tbp24s10-main-1b.bin", 0x0400, 0x0100, CRC(550563e1) SHA1(11edb45acba8b28a462c49956ebb1ba0a8b2ff26) )    /* character palette blue component */
	ROM_LOAD( "tbp24s10-gfx-1p.bin",  0x0500, 0x0100, CRC(35e45021) SHA1(511b94507f41b377f38184ed9a85f34949b28d26) )    /* sprite palette blue component */
	ROM_LOAD( "18s030-gfx-8t.bin",    0x0600, 0x0020, CRC(7a601c3d) SHA1(5c5cdf51b2c9fdb2b05402d9c260208ae73fe245) )    /* sprite height, one entry per 32 */
															/* sprites. Used at run time! */
	ROM_LOAD( "tbp24s10-gfx-9k.bin",  0x0620, 0x0100, CRC(82c20d12) SHA1(268903f7d9be58a70d030b02bf31a2d6b5b6e249) )    /* video timing - same as battroad */
	ROM_LOAD( "18s030-gfx-10a.bin",   0x0720, 0x0020, CRC(3858acd0) SHA1(49c96467c0e7146ed89f5107bcb7908bf4ce721a) )
	ROM_LOAD( "18s030-gfx-5d.bin",    0x0740, 0x0020, CRC(51304fcd) SHA1(be4d659e526f6fa5318b4cd3b6612c5b73f24437) )
	ROM_LOAD( "18s030-gfx-5e.bin",    0x0760, 0x0020, CRC(51304fcd) SHA1(be4d659e526f6fa5318b4cd3b6612c5b73f24437) )
	ROM_LOAD( "18s030-gfx-6l.bin",    0x0780, 0x0020, CRC(3858acd0) SHA1(49c96467c0e7146ed89f5107bcb7908bf4ce721a) )
	ROM_LOAD( "tbp24s10-gfx-3b.bin",  0x07a0, 0x0100, CRC(e6506ef4) SHA1(079841da7640b14d94aaaeb572bf018932b58293) )
	ROM_LOAD( "tbp24s10-gfx-4a.bin",  0x08a0, 0x0100, CRC(e0aa8869) SHA1(ac8bdfeba69420ba56ec561bf3d0f1229d02cea2) )
	ROM_LOAD( "tbp24s10-gfx-4c.bin",  0x09a0, 0x0100, CRC(b43d094f) SHA1(2bed4892d8a91d7faac5a07bf858d9294eb30606) )
	ROM_LOAD( "tbp24s10-gfx-6d.bin",  0x0aa0, 0x0100, CRC(48bb39c9) SHA1(fbe525cc45c9287ab5f6c02c2bd729a11540d6be) )
	ROM_LOAD( "tbp24s10-gfx-6e.bin",  0x0ba0, 0x0100, CRC(48bb39c9) SHA1(fbe525cc45c9287ab5f6c02c2bd729a11540d6be) )
	ROM_LOAD( "tbp24s10-gfx-6m.bin",  0x0ca0, 0x0100, CRC(9f7a1a4d) SHA1(2bc38cbf4d0d65311b60c71073d81ca58ac01a5b) )
	ROM_LOAD( "tbp24s10-gfx-6n.bin",  0x0da0, 0x0100, CRC(35e5b39e) SHA1(8889fad8a2c095129e4e50de5e2f66e986a4bedf) )
	ROM_LOAD( "tbp24s10-gfx-8a.bin",  0x0ea0, 0x0100, CRC(35e5b39e) SHA1(8889fad8a2c095129e4e50de5e2f66e986a4bedf) )
	ROM_LOAD( "tbp24s10-gfx-9a.bin",  0x0fa0, 0x0100, CRC(9f7a1a4d) SHA1(2bc38cbf4d0d65311b60c71073d81ca58ac01a5b) )
	ROM_LOAD( "tbp24s10-gfx-9k.bin",  0x10a0, 0x0100, CRC(82c20d12) SHA1(268903f7d9be58a70d030b02bf31a2d6b5b6e249) )
	ROM_LOAD( "tbp24s10-main-8b.bin", 0x11a0, 0x0100, CRC(180fbc57) SHA1(fe1cede9ec1002d48c4eb055d36f2b74c8dd4af8) )
	ROM_LOAD( "tbp24s10-main-8c.bin", 0x12a0, 0x0100, CRC(3bb32e5a) SHA1(b666e48cb7526b9a38e151cdcc56d298c640bc3f) )
	ROM_LOAD( "tbp24s10-main-8d.bin", 0x13a0, 0x0100, CRC(599c319f) SHA1(1e52e30f3beb2718fc382e3b85af6b6911863a08) )
ROM_END

ROM_START( kungfub2 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "kf4",          0x0000, 0x4000, CRC(3f65313f) SHA1(bd584896f558440a0f7bf8e1ca49bf478fe77553) )
	ROM_LOAD( "kf5",          0x4000, 0x4000, CRC(9ea325f3) SHA1(7c35128a3e70e5994a5e17db656dec1a53c1fe67) )

	ROM_REGION( 0x10000, "irem_audio:iremsound", 0 )   /* 64k for the audio CPU (6803) */
	ROM_LOAD( "a-3e-.bin",    0xa000, 0x2000, CRC(58e87ab0) SHA1(3b03c101fec58eac13fc309a78df9a2cd44f7604) )    /* samples (ADPCM 4-bit) */
	ROM_LOAD( "a-3f-.bin",    0xc000, 0x2000, CRC(c81e31ea) SHA1(f0fc58b929188c8802cd85549bdf9f4566e6a677) )    /* samples (ADPCM 4-bit) */
	ROM_LOAD( "a-3h-.bin",    0xe000, 0x2000, CRC(d99fb995) SHA1(caa6acdbc3b02d248fd123be95ea6fdcb4f35b59) )

	ROM_REGION( 0x06000, "gfx1", 0 )
	ROM_LOAD( "g-4c-a.bin",   0x00000, 0x2000, CRC(6b2cc9c8) SHA1(ba7c902d08c21a1e33f450406bfbfa35abde3b3f) )   /* characters */
	ROM_LOAD( "g-4d-a.bin",   0x02000, 0x2000, CRC(c648f558) SHA1(7cc085d8dc4a770d2828e39859b7b18e80148a00) )
	ROM_LOAD( "g-4e-a.bin",   0x04000, 0x2000, CRC(fbe9276e) SHA1(84181c8da79e2c92af04aef3ab5d23f70969dad8) )

	ROM_REGION( 0x18000, "gfx2", 0 )
	ROM_LOAD( "b-4k-.bin",    0x00000, 0x2000, CRC(16fb5150) SHA1(a49faf617f948d3ccec2bc6ef97bd399f0958f65) )   /* sprites */
	ROM_LOAD( "b-4f-.bin",    0x02000, 0x2000, CRC(67745a33) SHA1(fcc642fb1b932676c84c1a0901b989673c57c0e5) )
	ROM_LOAD( "b-4l-.bin",    0x04000, 0x2000, CRC(bd1c2261) SHA1(7155789a01801a9e1a55d4e68c94a3a3ee7d1b2e) )
	ROM_LOAD( "b-4h-.bin",    0x06000, 0x2000, CRC(8ac5ed3a) SHA1(9c88e8c82420428b43923cdee7eb4504882bec69) )
	ROM_LOAD( "b-3n-.bin",    0x08000, 0x2000, CRC(28a213aa) SHA1(0d6d668490bdf4394bc9fed2f3cdc72f2fea46f9) )
	ROM_LOAD( "b-4n-.bin",    0x0a000, 0x2000, CRC(d5228df3) SHA1(836c4f95f873fbf07f9bec63a72c20a14651117c) )
	ROM_LOAD( "b-4m-.bin",    0x0c000, 0x2000, CRC(b16de4f2) SHA1(512260e76c9cd21b8add771de53fbd27c2719213) )
	ROM_LOAD( "b-3m-.bin",    0x0e000, 0x2000, CRC(eba0d66b) SHA1(028f82fc1853b86a3201b24871f41091c3e0b542) )
	ROM_LOAD( "b-4c-.bin",    0x10000, 0x2000, CRC(01298885) SHA1(d4edf5fe707c5b7231ba72b731b96120064a7ecd) )
	ROM_LOAD( "b-4e-.bin",    0x12000, 0x2000, CRC(c77b87d4) SHA1(c0f66f0130f6a290a58a3d77bba1d06f16016901) )
	ROM_LOAD( "b-4d-.bin",    0x14000, 0x2000, CRC(6a70615f) SHA1(f4683dc0a566567e95e85268612bcf0e6297d955) )
	ROM_LOAD( "b-4a-.bin",    0x16000, 0x2000, CRC(6189d626) SHA1(ce8e5e95c2684c685481e9c8d921380b20ac0460) )

	ROM_REGION( 0x0720, "proms", 0 )
	ROM_LOAD( "g-1j-.bin",    0x0000, 0x0100, CRC(668e6bca) SHA1(cd5262b1310821ba7b12873e4db35f081d6b9df4) )    /* character palette red component */
	ROM_LOAD( "b-1m-.bin",    0x0100, 0x0100, CRC(76c05a9c) SHA1(1f46f436a17f8c883bdd6d9804b828a81a76f880) )    /* sprite palette red component */
	ROM_LOAD( "g-1f-.bin",    0x0200, 0x0100, CRC(964b6495) SHA1(76f30a65a0ded14babad2006221aa40621fb7ea1) )    /* character palette green component */
	ROM_LOAD( "b-1n-.bin",    0x0300, 0x0100, CRC(23f06b99) SHA1(6b3d6349f019aeab33838ae392bc3f3f89906326) )    /* sprite palette green component */
	ROM_LOAD( "g-1h-.bin",    0x0400, 0x0100, CRC(550563e1) SHA1(11edb45acba8b28a462c49956ebb1ba0a8b2ff26) )    /* character palette blue component */
	ROM_LOAD( "b-1l-.bin",    0x0500, 0x0100, CRC(35e45021) SHA1(511b94507f41b377f38184ed9a85f34949b28d26) )    /* sprite palette blue component */
	ROM_LOAD( "b-5f-.bin",    0x0600, 0x0020, CRC(7a601c3d) SHA1(5c5cdf51b2c9fdb2b05402d9c260208ae73fe245) )    /* sprite height, one entry per 32 */
															/* sprites. Used at run time! */
	ROM_LOAD( "b-6f-.bin",    0x0620, 0x0100, CRC(82c20d12) SHA1(268903f7d9be58a70d030b02bf31a2d6b5b6e249) )    /* video timing - same as battroad */
ROM_END

ROM_START( battroad )
	ROM_REGION( 0x1e000, "maincpu", 0 )
	ROM_LOAD( "br-a-4e.b",  0x00000, 0x2000, CRC(9bf14768) SHA1(53169553b956e5bcbd3fae13b86ab859cd08c955) )
	ROM_LOAD( "br-a-4d.b",  0x02000, 0x2000, CRC(39ca1627) SHA1(bbeb59ad93b4eb874dc3c1eebfc1136c0c5112c8) )
	ROM_LOAD( "br-a-4b.b",  0x04000, 0x2000, CRC(1865bb22) SHA1(990cfffc4fc0ade8bb4ebd02d107f8657728f976) )
	ROM_LOAD( "br-a-4a",    0x06000, 0x2000, CRC(65b61c21) SHA1(a440eb22a5824621ec7452c058de1329dbd5f168) )
	ROM_LOAD( "br-c-7c",    0x10000, 0x2000, CRC(2e1eca52) SHA1(d938bef795436232a6f58cba77b5f9004b254873) ) /* banked at a000-bfff */
	ROM_LOAD( "br-c-7l",    0x12000, 0x2000, CRC(f2178578) SHA1(2ddf867cb5bd372ceda58e794e417f34a9cbf1d5) )
	ROM_LOAD( "br-c-7d",    0x14000, 0x2000, CRC(3aa9fa30) SHA1(b7d943250bde9ec9ef8d51164652fc3ffee7e9ff) )
	ROM_LOAD( "br-c-7b",    0x16000, 0x2000, CRC(0b31b90b) SHA1(498d0deef7fa53600a821e64cf913d239659f784) )
	ROM_LOAD( "br-c-7a",    0x18000, 0x2000, CRC(ec3b0080) SHA1(83dc0d0c4ecf2ff62cfb213c02a3869c6f72f2e9) )
	ROM_LOAD( "br-c-7k",    0x1c000, 0x2000, CRC(edc75f7f) SHA1(3650521874c85a7cf403ede2588b989ac93fb92e) )

	ROM_REGION( 0x10000, "irem_audio:iremsound", 0 )   /* 64k for the audio CPU (6803) */
	ROM_LOAD( "br-a-3e",     0xa000, 0x2000, CRC(a7140871) SHA1(bf993e8ed776974ae59e1995eb5d3055c632eda7) )
	ROM_LOAD( "br-a-3f",     0xc000, 0x2000, CRC(1bb51b30) SHA1(c8e99d79e2fdb6b02c57cba475be0e5e35359124) )
	ROM_LOAD( "br-a-3h",     0xe000, 0x2000, CRC(afb3e083) SHA1(b1aaca64bbcce73203e55d2f16be3f3cef46f39b) )

	ROM_REGION( 0x06000, "gfx1", 0 )
	ROM_LOAD( "br-c-6h",    0x00000, 0x2000, CRC(ca50841c) SHA1(5cf159eb282d819f772331c1840a30d92732cf6e) ) /* tiles */
	ROM_LOAD( "br-c-6n",    0x02000, 0x2000, CRC(7d53163a) SHA1(0bdb470a29aaeb71ef4ec180dc9158f0753d3a3a) )
	ROM_LOAD( "br-c-6k",    0x04000, 0x2000, CRC(5951e12a) SHA1(bb5739678f1c1ea228443c7e300b098f6bcb8ba0) )

	ROM_REGION( 0x0c000, "gfx2", 0 )
	ROM_LOAD( "br-b-4k.a",  0x00000, 0x2000, CRC(d3c5e85b) SHA1(1a6a7b9c71f2209ef6c4d93d5bd4774a7239e569) ) /* sprites */
	ROM_LOAD( "br-b-4f.a",  0x02000, 0x2000, CRC(4354232a) SHA1(d2edcb74b630cc778d932788a191440397b99e22) )
	ROM_LOAD( "br-b-3n.a",  0x04000, 0x2000, CRC(2668dbef) SHA1(11528a4f8e5c75361686c1301f282cc2735ce2f5) )
	ROM_LOAD( "br-b-4n.a",  0x06000, 0x2000, CRC(c719a324) SHA1(b0a4e603a964f3c8aee77df4c51625fd1683b7b9) )
	ROM_LOAD( "br-b-4c.a",  0x08000, 0x2000, CRC(0b3193bf) SHA1(130d2d0ff3563d196eea50a23e581ac5e7de8d72) )
	ROM_LOAD( "br-b-4e.a",  0x0a000, 0x2000, CRC(3662e8fb) SHA1(c3c3deb7b2da42d6def279bd80b65eb2deef4038) )

	ROM_REGION( 0x04000, "gfx3", 0 )
	ROM_LOAD( "br-c-1b",    0x00000, 0x2000, CRC(8088911e) SHA1(d75d0a4ee5e51f14d93c8525486ee2cf2e87be9c) ) /* characters */
	ROM_LOAD( "br-c-1c",    0x02000, 0x2000, CRC(3d78b653) SHA1(b693d20ad28fed867ffbc23cda150f3201206d3c) )

	ROM_REGION( 0x0740, "proms", 0 )
	ROM_LOAD( "br-c-3j",     0x0000, 0x0100, CRC(aceaed79) SHA1(64cf6d012fc8d5163251812b4c2ac80d8f6dd353) ) /* tile palette red component */
	ROM_LOAD( "br-b-1m",     0x0100, 0x0100, CRC(3bd30c7d) SHA1(ce9812c47321820f144c5a285c15dbb2073c8847) ) /* sprite palette red component */
	ROM_LOAD( "br-c-3l",     0x0200, 0x0100, CRC(7cf6f380) SHA1(950a28dcb6e9d3d743c76ce07616ee7d6a0c138c) ) /* tile palette green component */
	ROM_LOAD( "br-b-1n",     0x0300, 0x0100, CRC(b7f3dc3b) SHA1(7bffb6f3ddd0459bd060b0c1ca22a291153672d5) ) /* sprite palette green component */
	ROM_LOAD( "br-c-3k",     0x0400, 0x0100, CRC(d90e4a54) SHA1(498c65773c83dfdb99703811bce7831f9a1af432) ) /* tile palette blue component */
	ROM_LOAD( "br-b-1l",     0x0500, 0x0100, CRC(5271c7d8) SHA1(1254b61133ed8fd6e032df04482fb775c3f66981) ) /* sprite palette blue component */
	ROM_LOAD( "br-c-1j",     0x0600, 0x0020, CRC(78eb5d77) SHA1(dd82f7843bea8c953f491faade6ced17e57ddf3c) ) /* character palette */
	ROM_LOAD( "br-b-5p",     0x0620, 0x0020, CRC(ce746937) SHA1(387e73a9ca684ac2e30061fca281970ff20ef0a5) ) /* sprite height, one entry per 32 */
															/* sprites. Used at run time! */
	ROM_LOAD( "br-b-6f",     0x0640, 0x0100, CRC(82c20d12) SHA1(268903f7d9be58a70d030b02bf31a2d6b5b6e249) ) /* video timing - same as kungfum */
ROM_END

ROM_START( ldrun )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "lr-a-4e",      0x0000, 0x2000, CRC(5d7e2a4d) SHA1(fe8aeff360f6c3a8606d67a8b95148c3c2ef7267) )
	ROM_LOAD( "lr-a-4d",      0x2000, 0x2000, CRC(96f20473) SHA1(e400c43f3f32e12f68ca204c60bcebdb2b3da55d) )
	ROM_LOAD( "lr-a-4b",      0x4000, 0x2000, CRC(b041c4a9) SHA1(77768b03ea2497e25c3e47b68a0eb2fe3e9aea35) )
	ROM_LOAD( "lr-a-4a",      0x6000, 0x2000, CRC(645e42aa) SHA1(c806ffce7ece418bad86854c987f78c70c13e492) )

	ROM_REGION( 0x10000, "irem_audio:iremsound", 0 )   /* 64k for the audio CPU (6803) */
	ROM_LOAD( "lr-a-3f",      0xc000, 0x2000, CRC(7a96accd) SHA1(e94815dbfaabbb562df8f3298060aa6bd7825904) )
	ROM_LOAD( "lr-a-3h",      0xe000, 0x2000, CRC(3f7f3939) SHA1(7ee25a21e74995bfb36ac11b45d384b33a6d8515) )

	ROM_REGION( 0x6000, "gfx1", 0 )
	ROM_LOAD( "lr-e-2d",      0x0000, 0x2000, CRC(24f9b58d) SHA1(e33224b910d37aaa85713b954c8dd50996245a8c) )    /* characters */
	ROM_LOAD( "lr-e-2j",      0x2000, 0x2000, CRC(43175e08) SHA1(9dbafb27d46cf7df35f343a8753e8d91ea706993) )
	ROM_LOAD( "lr-e-2f",      0x4000, 0x2000, CRC(e0317124) SHA1(b766bd21e2da1673d2054148f62d61c33c95d38e) )

	ROM_REGION( 0x6000, "gfx2", 0 )
	ROM_LOAD( "lr-b-4k",      0x0000, 0x2000, CRC(8141403e) SHA1(65fa6bc872fb07c71aacbbcc35cee766b2877896) )    /* sprites */
	ROM_LOAD( "lr-b-3n",      0x2000, 0x2000, CRC(55154154) SHA1(35304676e1ab55adccdabdc766a4e0e0901d3cd0) )
	ROM_LOAD( "lr-b-4c",      0x4000, 0x2000, CRC(924e34d0) SHA1(6a841419797a129235fc7d0405a5be55e8d703da) )

	ROM_REGION( 0x0720, "proms", 0 )
	ROM_LOAD( "lr-e-3m",      0x0000, 0x0100, CRC(53040416) SHA1(2c6915164d1c31afc60a21b557abdf023d5b3f46) )    /* character palette red component */
	ROM_LOAD( "lr-b-1m",      0x0100, 0x0100, CRC(4bae1c25) SHA1(17a9e2567d9d648dca69510bb201f8af0738b068) )    /* sprite palette red component */
	ROM_LOAD( "lr-e-3l",      0x0200, 0x0100, CRC(67786037) SHA1(cd40dfd94295afe57139733752643cf48b8566b1) )    /* character palette green component */
	ROM_LOAD( "lr-b-1n",      0x0300, 0x0100, CRC(9cd3db94) SHA1(bff95965f946df0e4af1f99db5b2468bf1d4403f) )    /* sprite palette green component */
	ROM_LOAD( "lr-e-3n",      0x0400, 0x0100, CRC(5b716837) SHA1(e3ea250891fec43a97e92ac1c3a4fbb5ee2d4a4d) )    /* character palette blue component */
	ROM_LOAD( "lr-b-1l",      0x0500, 0x0100, CRC(08d8cf9a) SHA1(a46213e0dc04e44b0544401eb341fd49eef331dd) )    /* sprite palette blue component */
	ROM_LOAD( "lr-b-5p",      0x0600, 0x0020, CRC(e01f69e2) SHA1(0d00ef348025ea4a9c274a7e3dbb006217d8449d) )    /* sprite height, one entry per 32 */
															/* sprites. Used at run time! */
	ROM_LOAD( "lr-b-6f",      0x0620, 0x0100, CRC(34d88d3c) SHA1(727f4c5cfff33538886fa0a29fd119aa085d7008) )    /* video timing - common to the other games */
ROM_END

ROM_START( ldruna )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "roma4c",       0x0000, 0x2000, CRC(279421e1) SHA1(051e103b2ade4a332053ba05aa7f126dd9b97c2c) )
	ROM_LOAD( "lr-a-4d",      0x2000, 0x2000, CRC(96f20473) SHA1(e400c43f3f32e12f68ca204c60bcebdb2b3da55d) )
	ROM_LOAD( "roma4b",       0x4000, 0x2000, CRC(3c464bad) SHA1(df34a9ec2f29f8d2a2ef1a2fc065ba7541ea4178) )
	ROM_LOAD( "roma4a",       0x6000, 0x2000, CRC(899df8e0) SHA1(e80154dccd04e2928124591f16e088de3554995b) )

	ROM_REGION( 0x10000, "irem_audio:iremsound", 0 )   /* 64k for the audio CPU (6803) */
	ROM_LOAD( "lr-a-3f",      0xc000, 0x2000, CRC(7a96accd) SHA1(e94815dbfaabbb562df8f3298060aa6bd7825904) )
	ROM_LOAD( "lr-a-3h",      0xe000, 0x2000, CRC(3f7f3939) SHA1(7ee25a21e74995bfb36ac11b45d384b33a6d8515) )

	ROM_REGION( 0x6000, "gfx1", 0 )
	ROM_LOAD( "lr-e-2d",      0x0000, 0x2000, CRC(24f9b58d) SHA1(e33224b910d37aaa85713b954c8dd50996245a8c) )    /* characters */
	ROM_LOAD( "lr-e-2j",      0x2000, 0x2000, CRC(43175e08) SHA1(9dbafb27d46cf7df35f343a8753e8d91ea706993) )
	ROM_LOAD( "lr-e-2f",      0x4000, 0x2000, CRC(e0317124) SHA1(b766bd21e2da1673d2054148f62d61c33c95d38e) )

	ROM_REGION( 0x6000, "gfx2", 0 )
	ROM_LOAD( "lr-b-4k",      0x0000, 0x2000, CRC(8141403e) SHA1(65fa6bc872fb07c71aacbbcc35cee766b2877896) )    /* sprites */
	ROM_LOAD( "lr-b-3n",      0x2000, 0x2000, CRC(55154154) SHA1(35304676e1ab55adccdabdc766a4e0e0901d3cd0) )
	ROM_LOAD( "lr-b-4c",      0x4000, 0x2000, CRC(924e34d0) SHA1(6a841419797a129235fc7d0405a5be55e8d703da) )

	ROM_REGION( 0x0720, "proms", 0 )
	ROM_LOAD( "lr-e-3m",      0x0000, 0x0100, CRC(53040416) SHA1(2c6915164d1c31afc60a21b557abdf023d5b3f46) )    /* character palette red component */
	ROM_LOAD( "lr-b-1m",      0x0100, 0x0100, CRC(4bae1c25) SHA1(17a9e2567d9d648dca69510bb201f8af0738b068) )    /* sprite palette red component */
	ROM_LOAD( "lr-e-3l",      0x0200, 0x0100, CRC(67786037) SHA1(cd40dfd94295afe57139733752643cf48b8566b1) )    /* character palette green component */
	ROM_LOAD( "lr-b-1n",      0x0300, 0x0100, CRC(9cd3db94) SHA1(bff95965f946df0e4af1f99db5b2468bf1d4403f) )    /* sprite palette green component */
	ROM_LOAD( "lr-e-3n",      0x0400, 0x0100, CRC(5b716837) SHA1(e3ea250891fec43a97e92ac1c3a4fbb5ee2d4a4d) )    /* character palette blue component */
	ROM_LOAD( "lr-b-1l",      0x0500, 0x0100, CRC(08d8cf9a) SHA1(a46213e0dc04e44b0544401eb341fd49eef331dd) )    /* sprite palette blue component */
	ROM_LOAD( "lr-b-5p",      0x0600, 0x0020, CRC(e01f69e2) SHA1(0d00ef348025ea4a9c274a7e3dbb006217d8449d) )    /* sprite height, one entry per 32 */
															/* sprites. Used at run time! */
	ROM_LOAD( "lr-b-6f",      0x0620, 0x0100, CRC(34d88d3c) SHA1(727f4c5cfff33538886fa0a29fd119aa085d7008) )    /* video timing - common to the other games */
ROM_END

ROM_START( ldrun2 )
	ROM_REGION( 0x14000, "maincpu", 0 ) /* 64k for code + 16k for banks */
	ROM_LOAD( "lr2-a-4e.a",   0x00000, 0x2000, CRC(22313327) SHA1(c82c9d3218e0384e26b86f9475fea5056cd9832b) )
	ROM_LOAD( "lr2-a-4d",     0x02000, 0x2000, CRC(ef645179) SHA1(d4cae3cb223d7d10e2cce751af5c77493c1c60df) )
	ROM_LOAD( "lr2-a-4a.a",   0x04000, 0x2000, CRC(b11ddf59) SHA1(9797cbf4b8cd7bfabe797bdc71c904f54d710491) )
	ROM_LOAD( "lr2-a-4a",     0x06000, 0x2000, CRC(470cc8a1) SHA1(72ca710a08c322a2b61dfb4c0c67af9d72078fc0) )
	ROM_LOAD( "lr2-h-1c.a",   0x10000, 0x2000, CRC(7ebcadbc) SHA1(ff8377cb1f349f2957280b6a633e596740a489be) )   /* banked at 8000-9fff */
	ROM_LOAD( "lr2-h-1d.a",   0x12000, 0x2000, CRC(64cbb7f9) SHA1(897d53c3892e636734f5a380c67c41c0d810955e) )   /* banked at 8000-9fff */

	ROM_REGION( 0x10000, "irem_audio:iremsound", 0 )   /* 64k for the audio CPU (6803) */
	ROM_LOAD( "lr2-a-3e",     0xa000, 0x2000, CRC(853f3898) SHA1(12ade2f6f10c85c6a2c380dd0727aad64b317d56) )
	ROM_LOAD( "lr2-a-3f",     0xc000, 0x2000, CRC(7a96accd) SHA1(e94815dbfaabbb562df8f3298060aa6bd7825904) )
	ROM_LOAD( "lr2-a-3h",     0xe000, 0x2000, CRC(2a0e83ca) SHA1(207bfb3912e7a9caa61b742fc3357154a0189434) )

	ROM_REGION( 0x6000, "gfx1", 0 )
	ROM_LOAD( "lr2-h-1e",     0x00000, 0x2000, CRC(9d63a8ff) SHA1(4281fd7a72313b58113e0e64e09ccff50eeccfa6) )   /* characters */
	ROM_LOAD( "lr2-h-1j",     0x02000, 0x2000, CRC(40332bbd) SHA1(4611d637bd8332f38c6b9c6c3c246a587632ac58) )
	ROM_LOAD( "lr2-h-1h",     0x04000, 0x2000, CRC(9404727d) SHA1(ed096c6406ec74418bbbb1e2cee3ce9a4919ba7c) )

	ROM_REGION( 0xc000, "gfx2", 0 )
	ROM_LOAD( "lr2-b-4k",     0x00000, 0x2000, CRC(79909871) SHA1(18fc113d159c902986b47a94894c982c74c2021e) )   /* sprites */
	ROM_LOAD( "lr2-b-4f",     0x02000, 0x2000, CRC(06ba1ef4) SHA1(e8ba4c270df95810d80cdfe87f6d585cfaf60574) )
	ROM_LOAD( "lr2-b-3n",     0x04000, 0x2000, CRC(3cc5893f) SHA1(c2e88d3473d575637c49c218d6099578386891cc) )
	ROM_LOAD( "lr2-b-4n",     0x06000, 0x2000, CRC(49c12f42) SHA1(56f1e4665fb4f84637a2c16c914657977f38a224) )
	ROM_LOAD( "lr2-b-4c",     0x08000, 0x2000, CRC(fbe6d24c) SHA1(d4d9bfa5abf7d9b2457543c56bb04edf209157b8) )
	ROM_LOAD( "lr2-b-4e",     0x0a000, 0x2000, CRC(75172d1f) SHA1(6771c31ad834ca94a6575e34d781add5bfadce22) )

	ROM_REGION( 0x0720, "proms", 0 )
	ROM_LOAD( "lr2-h-3m",     0x0000, 0x0100, CRC(2c5d834b) SHA1(4144a566d7eef06e9cd2d4c774e4b1e6f9ef56b1) )    /* character palette red component */
	ROM_LOAD( "lr2-b-1m",     0x0100, 0x0100, CRC(4ec9bb3d) SHA1(e36398e0e92525f0c5086cb2b5a4a976d1e4f126) )    /* sprite palette red component */
	ROM_LOAD( "lr2-h-3l",     0x0200, 0x0100, CRC(3ae69aca) SHA1(683bf617a36952d08bea53ea9c82b12f81c62c53) )    /* character palette green component */
	ROM_LOAD( "lr2-b-1n",     0x0300, 0x0100, CRC(1daf1fa4) SHA1(5742ceff566e1d9f1148df4e408571aa290996d3) )    /* sprite palette green component */
	ROM_LOAD( "lr2-h-3n",     0x0400, 0x0100, CRC(2b28aec5) SHA1(946633bd7203ba1481250f900f3232c18538613b) )    /* character palette blue component */
	ROM_LOAD( "lr2-b-1l",     0x0500, 0x0100, CRC(c8fb708a) SHA1(ed38f36fa7918179c7176c762c0fcc86b5ddb218) )    /* sprite palette blue component */
	ROM_LOAD( "lr2-b-5p",     0x0600, 0x0020, CRC(e01f69e2) SHA1(0d00ef348025ea4a9c274a7e3dbb006217d8449d) )    /* sprite height, one entry per 32 */
															/* sprites. Used at run time! */
	ROM_LOAD( "lr2-b-6f",     0x0620, 0x0100, CRC(34d88d3c) SHA1(727f4c5cfff33538886fa0a29fd119aa085d7008) )    /* video timing - common to the other games */
ROM_END

ROM_START( ldrun3 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "lr3a4eb.bin",  0x0000, 0x4000, CRC(09affc47) SHA1(1cd56b967e4c8ada6c6e1015aead5f7551034358) )
	ROM_LOAD( "lr3a4db.bin",  0x4000, 0x4000, CRC(23a02178) SHA1(0560b92dfc9b57d01abb62a7b270beb27fa42040) )
	ROM_LOAD( "lr3a4bb.bin",  0x8000, 0x4000, CRC(3d501a1a) SHA1(a92ff743eb21145154fa717ee9e6ede4da458aa9) )

	ROM_REGION( 0x10000, "irem_audio:iremsound", 0 )   /* 64k for the audio CPU (6803) */
	ROM_LOAD( "lr3-a-3d",     0x8000, 0x4000, CRC(28be68cd) SHA1(1e48cdf649bc861066fbef0293466091092045f3) )
	ROM_LOAD( "lr3-a-3f",     0xc000, 0x4000, CRC(cb7186b7) SHA1(cc99821f3f1523523598e4b7d68b95eee6c84e69) )

	ROM_REGION( 0xc000, "gfx1", 0 )
	ROM_LOAD( "lr3-n-2a",     0x00000, 0x4000, CRC(f9b74dee) SHA1(f4407024aea05d0c698f8a7a6a20cbbcbd8baf44) )   /* characters */
	ROM_LOAD( "lr3-n-2c",     0x04000, 0x4000, CRC(fef707ba) SHA1(ff6e64eeda6a9be672a1b8778a051886c38bd8f6) )
	ROM_LOAD( "lr3-n-2b",     0x08000, 0x4000, CRC(af3d27b9) SHA1(2eda0bf7ffd7bcb7b7dcd2ffb1482f748ee2edfc) )

	ROM_REGION( 0x18000, "gfx2", 0 )
	ROM_LOAD( "lr3b4kb.bin",  0x00000, 0x4000, CRC(21ecd8c5) SHA1(786d7d9b690764cfc0c65c3e58369e78f7cc4475) )   /* sprites */
	ROM_LOAD( "snxb4fb.bin",  0x04000, 0x4000, CRC(ed719d7b) SHA1(34c7d9fc762b3f113b6115910898fde9713ae24f) )
	ROM_LOAD( "lr3b3nb.bin",  0x08000, 0x4000, CRC(da8cffab) SHA1(3e194b656fa0c4771e37e6a8b7405edc495808ad) )
	ROM_LOAD( "snxb4nb.bin",  0x0c000, 0x4000, CRC(dc675003) SHA1(df9b6055b4a4c537504658e62e96f826d498b39c) )
	ROM_LOAD( "snxb4cb.bin",  0x10000, 0x4000, CRC(585aa244) SHA1(d90cf29280e5f73b14dc5b33b1a82970e8e1a560) )
	ROM_LOAD( "snxb4eb.bin",  0x14000, 0x4000, CRC(2d3b69dd) SHA1(472e1c06fd3184b91d9b718bb590d45702ed84cd) )

	ROM_REGION( 0x0820, "proms", 0 )
	ROM_LOAD( "lr3-n-2l",     0x0000, 0x0100, CRC(e880b86b) SHA1(3934f37dc45b725af1c7d862086249256366d572) ) /* character palette red component */
	ROM_LOAD( "lr3-b-1m",     0x0100, 0x0100, CRC(f02d7167) SHA1(385a9179143e3dcccd7052e70c7cc71473caaaca) ) /* sprite palette red component */
	ROM_LOAD( "lr3-n-2k",     0x0200, 0x0100, CRC(047ee051) SHA1(7c18a223d37ccc5fea20f8f856fba20335c75ea4) ) /* character palette green component */
	ROM_LOAD( "lr3-b-1n",     0x0300, 0x0100, CRC(9e37f181) SHA1(8e36eb8f4aefcc6d21dfbb2e86dcb4875bcf82cd) ) /* sprite palette green component */
	ROM_LOAD( "lr3-n-2m",     0x0400, 0x0100, CRC(69ad8678) SHA1(96134aa530cb93a5e3b56fffa996aefa08a666a2) ) /* character palette blue component */
	ROM_LOAD( "lr3-b-1l",     0x0500, 0x0100, CRC(5b11c41d) SHA1(186ca7bfa2894311fc573f3f5882da677e029f2a) ) /* sprite palette blue component */
	ROM_LOAD( "lr3-b-5p",     0x0600, 0x0020, CRC(e01f69e2) SHA1(0d00ef348025ea4a9c274a7e3dbb006217d8449d) )    /* sprite height, one entry per 32 */
															/* sprites. Used at run time! */
	ROM_LOAD( "lr3-n-4f",     0x0620, 0x0100, CRC(df674be9) SHA1(4d8c5378234bc24fac62dc227d8cd72f1ab7a35c) )    /* unknown */
	ROM_LOAD( "lr3-b-6f",     0x0720, 0x0100, CRC(34d88d3c) SHA1(727f4c5cfff33538886fa0a29fd119aa085d7008) )    /* video timing - common to the other games */
ROM_END

ROM_START( ldrun3j )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "lr3-a-4e",     0x0000, 0x4000, CRC(5b334e8e) SHA1(018ee450f88feaf5da025e01d2d839b29d5f1559) )
	ROM_LOAD( "lr3-a-4d.a",   0x4000, 0x4000, CRC(a84bc931) SHA1(0348d238a85a059a6423794910adec4462e14f27) )
	ROM_LOAD( "lr3-a-4b.a",   0x8000, 0x4000, CRC(be09031d) SHA1(c124163895d295969b66386fee91c89bbd8b8774) )

	ROM_REGION( 0x10000, "irem_audio:iremsound", 0 )   /* 64k for the audio CPU (6803) */
	ROM_LOAD( "lr3-a-3d",     0x8000, 0x4000, CRC(28be68cd) SHA1(1e48cdf649bc861066fbef0293466091092045f3) )
	ROM_LOAD( "lr3-a-3f",     0xc000, 0x4000, CRC(cb7186b7) SHA1(cc99821f3f1523523598e4b7d68b95eee6c84e69) )

	ROM_REGION( 0xc000, "gfx1", 0 )
	ROM_LOAD( "lr3-n-2a",     0x00000, 0x4000, CRC(f9b74dee) SHA1(f4407024aea05d0c698f8a7a6a20cbbcbd8baf44) )   /* characters */
	ROM_LOAD( "lr3-n-2c",     0x04000, 0x4000, CRC(fef707ba) SHA1(ff6e64eeda6a9be672a1b8778a051886c38bd8f6) )
	ROM_LOAD( "lr3-n-2b",     0x08000, 0x4000, CRC(af3d27b9) SHA1(2eda0bf7ffd7bcb7b7dcd2ffb1482f748ee2edfc) )

	ROM_REGION( 0xc000, "gfx2", 0 )
	ROM_LOAD( "lr3-b-4k",     0x00000, 0x4000, CRC(63f070c7) SHA1(beeb13dbba228827cf18e4c23deac041acbb2903) )   /* sprites */
	ROM_LOAD( "lr3-b-3n",     0x04000, 0x4000, CRC(eab7ad91) SHA1(c4e8dec38f6df27c0309172232aa8056be7982c4) )
	ROM_LOAD( "lr3-b-4c",     0x08000, 0x4000, CRC(1a460a46) SHA1(2f9e85ab45e8ec7a08edb9c1f82bce694cc2bc99) )

	ROM_REGION( 0x0820, "proms", 0 )
	ROM_LOAD( "lr3-n-2l",     0x0000, 0x0100, CRC(e880b86b) SHA1(3934f37dc45b725af1c7d862086249256366d572) ) /* character palette red component */
	ROM_LOAD( "lr3-b-1m",     0x0100, 0x0100, CRC(f02d7167) SHA1(385a9179143e3dcccd7052e70c7cc71473caaaca) ) /* sprite palette red component */
	ROM_LOAD( "lr3-n-2k",     0x0200, 0x0100, CRC(047ee051) SHA1(7c18a223d37ccc5fea20f8f856fba20335c75ea4) ) /* character palette green component */
	ROM_LOAD( "lr3-b-1n",     0x0300, 0x0100, CRC(9e37f181) SHA1(8e36eb8f4aefcc6d21dfbb2e86dcb4875bcf82cd) ) /* sprite palette green component */
	ROM_LOAD( "lr3-n-2m",     0x0400, 0x0100, CRC(69ad8678) SHA1(96134aa530cb93a5e3b56fffa996aefa08a666a2) ) /* character palette blue component */
	ROM_LOAD( "lr3-b-1l",     0x0500, 0x0100, CRC(5b11c41d) SHA1(186ca7bfa2894311fc573f3f5882da677e029f2a) ) /* sprite palette blue component */
	ROM_LOAD( "lr3-b-5p",     0x0600, 0x0020, CRC(e01f69e2) SHA1(0d00ef348025ea4a9c274a7e3dbb006217d8449d) )    /* sprite height, one entry per 32 */
															/* sprites. Used at run time! */
	ROM_LOAD( "lr3-n-4f",     0x0620, 0x0100, CRC(df674be9) SHA1(4d8c5378234bc24fac62dc227d8cd72f1ab7a35c) )    /* unknown */
	ROM_LOAD( "lr3-b-6f",     0x0720, 0x0100, CRC(34d88d3c) SHA1(727f4c5cfff33538886fa0a29fd119aa085d7008) )    /* video timing - common to the other games */
ROM_END

ROM_START( ldrun4 )
	ROM_REGION( 0x18000, "maincpu", 0 ) /* 64k for code + 32k for banked ROM */
	ROM_LOAD( "lr4-a-4e",     0x00000, 0x4000, CRC(5383e9bf) SHA1(01f6f76b768107b389d7240bd15a5e0720defcb6) )
	ROM_LOAD( "lr4-a-4d.c",   0x04000, 0x4000, CRC(298afa36) SHA1(077b5fa8a246059801232c5287225e3bb7507345) )
	ROM_LOAD( "lr4-v-4k",     0x10000, 0x8000, CRC(8b248abd) SHA1(3e755c8f8011d6f878a1777a2c22b2156ef926e6) )   /* banked at 8000-bfff */

	ROM_REGION( 0x10000, "irem_audio:iremsound", 0 )   /* 64k for the audio CPU (6803) */
	ROM_LOAD( "lr4-a-3d",     0x8000, 0x4000, CRC(86c6d445) SHA1(644b86fba745a5be545c4dd9c534157af75492b8) )
	ROM_LOAD( "lr4-a-3f",     0xc000, 0x4000, CRC(097c6c0a) SHA1(627ccdd1d77ae78db9660b51484d2a9110d035e5) )

	ROM_REGION( 0xc000, "gfx1", 0 )
	ROM_LOAD( "lr4-v-2b",     0x00000, 0x4000, CRC(4118e60a) SHA1(5c504e3f57e68f6049e422d979c8f6f4b795344f) )   /* characters */
	ROM_LOAD( "lr4-v-2d",     0x04000, 0x4000, CRC(542bb5b5) SHA1(e88eaf27ed72af1e6efa3c1500823be124fcf6b4) )
	ROM_LOAD( "lr4-v-2c",     0x08000, 0x4000, CRC(c765266c) SHA1(bf0ae987928034cf7b508d56f3e647a5c827e420) )

	ROM_REGION( 0x18000, "gfx2", 0 )
	ROM_LOAD( "lr4-b-4k",     0x00000, 0x4000, CRC(e7fe620c) SHA1(f5e35f5868355fa1dd53a9005d320b28b10d7a69) )   /* sprites */
	ROM_LOAD( "lr4-b-4f",     0x04000, 0x4000, CRC(6f0403db) SHA1(90f452f159c06b42bf536dce31f695a932332a50) )
	ROM_LOAD( "lr4-b-3n",     0x08000, 0x4000, CRC(ad1fba1b) SHA1(095feb824ad0b26e9c546cc3095aae3e49ee9705) )
	ROM_LOAD( "lr4-b-4n",     0x0c000, 0x4000, CRC(0e568fab) SHA1(4d9abb54953dc5c5598f83bc87861dfd5d8a1685) )
	ROM_LOAD( "lr4-b-4c",     0x10000, 0x4000, CRC(82c53669) SHA1(5e020e1df81ddc3ca0f0aeff0010ec3cd87ce426) )
	ROM_LOAD( "lr4-b-4e",     0x14000, 0x4000, CRC(767a1352) SHA1(675bda83bf46421a37dbfaa9323e5ecc4a3b63dd) )

	ROM_REGION( 0x0820, "proms", 0 )
	ROM_LOAD( "lr4-v-1m",     0x0000, 0x0100, CRC(fe51bf1d) SHA1(92461d6fcbc94bde9639720e8f58b974e5adb2dc) ) /* character palette red component */
	ROM_LOAD( "lr4-b-1m",     0x0100, 0x0100, CRC(5d8d17d0) SHA1(214f9f7f9fa9c2b616c5b4a3060c4bb96ea9fef4) ) /* sprite palette red component */
	ROM_LOAD( "lr4-v-1n",     0x0200, 0x0100, CRC(da0658e5) SHA1(5a7f665e4d63938b4e4415066eb6c986e82bd1a7) ) /* character palette green component */
	ROM_LOAD( "lr4-b-1n",     0x0300, 0x0100, CRC(da1129d2) SHA1(169e616c7340ab76f931493eba188756da48a8ec) ) /* sprite palette green component */
	ROM_LOAD( "lr4-v-1p",     0x0400, 0x0100, CRC(0df23ebe) SHA1(054736b762aa6698c1e6d827f8db62ae0342c83f) ) /* character palette blue component */
	ROM_LOAD( "lr4-b-1l",     0x0500, 0x0100, CRC(0d89b692) SHA1(b2854290c46c34934ff91980d72768070ebc8bf3) ) /* sprite palette blue component */
	ROM_LOAD( "lr4-b-5p",     0x0600, 0x0020, CRC(e01f69e2) SHA1(0d00ef348025ea4a9c274a7e3dbb006217d8449d) )    /* sprite height, one entry per 32 */
															/* sprites. Used at run time! */
	ROM_LOAD( "lr4-v-4h",     0x0620, 0x0100, CRC(df674be9) SHA1(4d8c5378234bc24fac62dc227d8cd72f1ab7a35c) )    /* unknown */
	ROM_LOAD( "lr4-b-6f",     0x0720, 0x0100, CRC(34d88d3c) SHA1(727f4c5cfff33538886fa0a29fd119aa085d7008) )    /* video timing - common to the other games */
ROM_END

ROM_START( lotlot )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "lot-a-4e",     0x0000, 0x4000, CRC(2913d08f) SHA1(829115333825a9483322a910efee9ee470d85a0e) )
	ROM_LOAD( "lot-a-4d",     0x4000, 0x4000, CRC(0443095f) SHA1(2c3ca107b54519632513a75b80a1a7d8b971f2b1) )

	ROM_REGION( 0x10000, "irem_audio:iremsound", 0 )   /* 64k for the audio CPU (6803) */
	ROM_LOAD( "lot-a-3h",     0xe000, 0x2000, CRC(0781cee7) SHA1(bff8592f96b43af6554f1a04e0c00f45b178cce6) )

	ROM_REGION( 0x6000, "gfx1", 0 )
	ROM_LOAD( "lot-k-4a",     0x00000, 0x2000, CRC(1b3695f4) SHA1(0a1a1df8bf0288434d47b323d97019b705f5d9d7) )   /* tiles */
	ROM_LOAD( "lot-k-4c",     0x02000, 0x2000, CRC(bd2b0730) SHA1(af66617a5fce6e72107c3949e9829121e8719648) )
	ROM_LOAD( "lot-k-4b",     0x04000, 0x2000, CRC(930ddd55) SHA1(bf2580c6b1df68fe2bca290b227c40f450a77576) )

	ROM_REGION( 0x6000, "gfx2", 0 )
	ROM_LOAD( "lot-b-4k",     0x00000, 0x2000, CRC(fd27cb90) SHA1(79d3f5ba8e271df05af55625e1db2f2adea25285) )   /* sprites */
	ROM_LOAD( "lot-b-3n",     0x02000, 0x2000, CRC(bd486fff) SHA1(a6159c0f55cf288b0382c3415811ea2a35f3564e) )
	ROM_LOAD( "lot-b-4c",     0x04000, 0x2000, CRC(3026ee6c) SHA1(6d4ea4b0409d3486eb0e7e9507a0ab79df5ee6c8) )

	ROM_REGION( 0x6000, "gfx3", 0 )
	ROM_LOAD( "lot-k-4p",     0x00000, 0x2000, CRC(3b7d95ba) SHA1(4be898c0d5b5c73f380abf5f5ea66f1797eefd1d) )   /* chars */
	ROM_LOAD( "lot-k-4l",     0x02000, 0x2000, CRC(f98dca1f) SHA1(b88d2b9cb3ac8d5523f3788fca8bae60f8fad6f7) )
	ROM_LOAD( "lot-k-4n",     0x04000, 0x2000, CRC(f0cd76a5) SHA1(3f7b1890ca36c190d3fe2571382ada93798a0a51) )

	ROM_REGION( 0x0e20, "proms", 0 )
	ROM_LOAD( "lot-k-2f",     0x0000, 0x0100, CRC(b820a05e) SHA1(79158f0cd64231c5cd90dc391e492a21aba4c30d) ) /* tile palette red component */
	ROM_LOAD( "lot-b-1m",     0x0100, 0x0100, CRC(c146461d) SHA1(87a5dc3a93a9f9f08e97eef77eb099792fdf70e6) ) /* sprite palette red component */
	ROM_LOAD( "lot-k-2l",     0x0200, 0x0100, CRC(ac3e230d) SHA1(e7d5afc707580a5c1df1201694a4db685af5f986) ) /* character palette red component */
	ROM_LOAD( "lot-k-2e",     0x0300, 0x0100, CRC(9b1fa005) SHA1(076af5d7a30a47b5884fcf33452a10aad91d30ee) ) /* tile palette green component */
	ROM_LOAD( "lot-b-1n",     0x0400, 0x0100, CRC(01e07db6) SHA1(3a18a6919b966d429d5ec9cf812768804407f92e) ) /* sprite palette green component */
	ROM_LOAD( "lot-k-2k",     0x0500, 0x0100, CRC(1811ad2b) SHA1(fb7aa262595010dd0fc1a94d74a37359f20e4cd7) ) /* character palette green component */
	ROM_LOAD( "lot-k-2d",     0x0600, 0x0100, CRC(315ed9a8) SHA1(7bfa91729cce7911a45035e2fa576a2b6b010a65) ) /* tile palette blue component */
	ROM_LOAD( "lot-b-1l",     0x0700, 0x0100, CRC(8b6fcde3) SHA1(04e9ce04b77a5f8737f2ec0aaeadaccdbbdda573) ) /* sprite palette blue component */
	ROM_LOAD( "lot-k-2j",     0x0800, 0x0100, CRC(e791ef2a) SHA1(cb1236630cbbc23e2e684ad3b7f51e52389eea2e) ) /* character palette blue component */
	ROM_LOAD( "lot-b-5p",     0x0900, 0x0020, CRC(110b21fd) SHA1(a7a660ff18540e2d73a80f341cd50c5f4d184085) )    /* sprite height, one entry per 32 */
															/* sprites. Used at run time! */
	ROM_LOAD( "lot-k-7e",     0x0920, 0x0200, CRC(6cef0fbd) SHA1(0c5c63a203e7bd852a3574c18f212487caf529ca) )    /* unknown */
	ROM_LOAD( "lot-k-7h",     0x0b20, 0x0200, CRC(04442bee) SHA1(37d10b605830b9355b00256af479c06cd4b97950) )    /* unknown */
	ROM_LOAD( "lot-b-6f",     0x0d20, 0x0100, CRC(34d88d3c) SHA1(727f4c5cfff33538886fa0a29fd119aa085d7008) )    /* video timing - common to the other games */
ROM_END

ROM_START( kidniki )
	ROM_REGION( 0x30000, "maincpu", 0 ) /* main CPU */
	ROM_LOAD( "ky_a-4e-g.bin", 0x00000, 0x04000, CRC(2edcbcd7) SHA1(fbb8e3cddfba01523c0d253cbc74ddb259b6197d) )
	ROM_LOAD( "dr03.4cd",      0x04000, 0x04000, CRC(dba20934) SHA1(a7aac4fcea5c1a94ddaf67e85bf2ce2e77c965cb) )
	ROM_LOAD( "ky_t-8k-g.bin", 0x10000, 0x08000, CRC(dbc42f31) SHA1(a7c7fdde1a8b63660d3e38786dbc80b6d91bde1b) ) /* banked at 8000-9fff */
	ROM_LOAD( "dr12.8l",       0x18000, 0x08000, CRC(c0b255fd) SHA1(bdd74239a3490eb35cb736cac6f5030bec4b2392) )
	ROM_CONTINUE(              0x28000, 0x08000 )

	ROM_REGION( 0x10000, "irem_audio:iremsound", 0 )   /* sound CPU */
	ROM_LOAD( "dr00.3a",      0x4000, 0x04000, CRC(458309f7) SHA1(7dfd77e0cd76a4b6f913eb434ac20ef5a172e2f0) )
	ROM_LOAD( "dr01.3cd",     0x8000, 0x04000, CRC(e66897bd) SHA1(04ea4a857a94d4e884fb28623ec6195dae701e25) )
	ROM_LOAD( "dr02.3f",      0xc000, 0x04000, CRC(f9e31e26) SHA1(712b1bde4b3c18c9ac26d58ade48316af004e733) ) /* 6803 code */

	ROM_REGION( 0x18000, "gfx1", 0 )
	ROM_LOAD( "dr06.2b",      0x00000, 0x8000, CRC(4d9a970f) SHA1(affeea31706644a9f1dcd1c4f739cadbdd58e597) )   /* tiles */
	ROM_LOAD( "dr07.2dc",     0x08000, 0x8000, CRC(ab59a4c4) SHA1(4e1eca8d4185ef35e2be13fa6a4af01f36d19f27) )
	ROM_LOAD( "dr05.2a",      0x10000, 0x8000, CRC(2e6dad0c) SHA1(fe79c510cea3f57ba61ac2e8d9858c887688897b) )

	ROM_REGION( 0x30000, "gfx2", 0 )
	ROM_LOAD( "dr21.4k",      0x00000, 0x4000, CRC(a06cea9a) SHA1(d78792b8e310fed3dc622c05d81303dad338afa7) )   /* sprites */
	ROM_LOAD( "dr19.4f",      0x04000, 0x4000, CRC(b34605ad) SHA1(56eba99c73527326f3428961732854753a6a5a2e) )
	ROM_LOAD( "dr22.4l",      0x08000, 0x4000, CRC(41303de8) SHA1(7bffda2e4c95b021f21d8375cf2d6b14280ea7b5) )
	ROM_LOAD( "dr20.4jh",     0x0c000, 0x4000, CRC(5fbe6f61) SHA1(18ab120777fffe912c8fc139fda2977a44aa453f) )
	ROM_LOAD( "dr14.3p",      0x10000, 0x4000, CRC(76cfbcbc) SHA1(3fdffc5893143b06535b5d85cd2a01c61d08e679) )
	ROM_LOAD( "dr24.4p",      0x14000, 0x4000, CRC(d51c8db5) SHA1(be48478afa0acc33e6a02a0a31f1f07ce3c7a3b8) )
	ROM_LOAD( "dr23.4nm",     0x18000, 0x4000, CRC(03469df8) SHA1(f3ffc0419c9d9c9a69387f62ceb9081170f861d4) )
	ROM_LOAD( "dr13.3nm",     0x1c000, 0x4000, CRC(d5c3dfe0) SHA1(c0e6b23bed9bf650d516a80c8b94f2d888d81c2d) )
	ROM_LOAD( "dr16.4cb",     0x20000, 0x4000, CRC(f1d1bb93) SHA1(42103d2e89a0d1edc834d08a6c443065fdacdbf2) )
	ROM_LOAD( "dr18.4e",      0x24000, 0x4000, CRC(edb7f25b) SHA1(394337132ff74b8d06d02233201f8ae8958e2aa6) )
	ROM_LOAD( "dr17.4dc",     0x28000, 0x4000, CRC(4fb87868) SHA1(73d779d687a6c319cd85793f95cb24413010b5e2) )
	ROM_LOAD( "dr15.4a",      0x2c000, 0x4000, CRC(e0b88de5) SHA1(08808ca90d34e494697a74e8a2314e32087e2f4d) )

	ROM_REGION( 0x0c000, "gfx3", 0 )
	ROM_LOAD( "dr08.4l",      0x00000, 0x4000, CRC(32d50643) SHA1(58bff370a3e24f21d5fb35289e85084b03178f88) )   /* chars */
	ROM_LOAD( "dr09.4m",      0x04000, 0x4000, CRC(17df6f95) SHA1(669a81906dfd81d807cbb2b5827ddb504536cb2c) )
	ROM_LOAD( "dr10.4n",      0x08000, 0x4000, CRC(820ce252) SHA1(910dbb910fdfcf9542360c0cd78c58c93d1d0c26) )

	ROM_REGION( 0x0920, "proms", 0 )
	ROM_LOAD( "dr25.3f",      0x0000, 0x0100, CRC(8e91430b) SHA1(a7a1567a0fd31cd65260f3ddb5280368704378bd) )    /* character palette red component */
	ROM_LOAD( "dr30.1m",      0x0100, 0x0100, CRC(28c73263) SHA1(ffeb8d1310759bf20b1624ab92fc91078726679c) )    /* sprite palette red component */
	ROM_LOAD( "dr26.3h",      0x0200, 0x0100, CRC(b563b93f) SHA1(86aefdaa63b35fe82f9f70eff3e4c14629f7a184) )    /* character palette green component */
	ROM_LOAD( "dr31.1n",      0x0300, 0x0100, CRC(3529210e) SHA1(3042ec941bdcb873077e77cffe36d4a28298bbbb) )    /* sprite palette green component */
	ROM_LOAD( "dr27.3j",      0x0400, 0x0100, CRC(70d668ef) SHA1(2cc647f2708932105bb9a5130aacc5a8a160e418) )    /* character palette blue component */
	ROM_LOAD( "dr29.1l",      0x0500, 0x0100, CRC(1173a754) SHA1(dbb7d02b72ae1842e0d17aee324a5b85ff2a2178) )    /* sprite palette blue component */
	ROM_LOAD( "dr32.5p",      0x0600, 0x0020, CRC(11cd1f2e) SHA1(45ceb84ff373127ff370610c1ce8d83fc6045bcb) )    /* sprite height, one entry per 32 */
															/* sprites. Used at run time! */
	ROM_LOAD( "dr28.8f",      0x0620, 0x0200, CRC(6cef0fbd) SHA1(0c5c63a203e7bd852a3574c18f212487caf529ca) )    /* unknown */
	ROM_LOAD( "dr33.6f",      0x0820, 0x0100, CRC(34d88d3c) SHA1(727f4c5cfff33538886fa0a29fd119aa085d7008) )    /* video timing - common to the other games */
ROM_END

ROM_START( kidnikiu )
	ROM_REGION( 0x30000, "maincpu", 0 ) /* main CPU */
	ROM_LOAD( "dr04.4e",      0x00000, 0x04000, CRC(80431858) SHA1(3a387f63ce0c7601264f91ae1f6fe604f2ef8ef1) )
	ROM_LOAD( "dr03.4cd",     0x04000, 0x04000, CRC(dba20934) SHA1(a7aac4fcea5c1a94ddaf67e85bf2ce2e77c965cb) )
	ROM_LOAD( "dr11.8k",      0x10000, 0x08000, CRC(04d82d93) SHA1(a901659cbe12a284f30a6b98ea6907df0222ddbf) )  /* banked at 8000-9fff */
	ROM_LOAD( "dr12.8l",      0x18000, 0x08000, CRC(c0b255fd) SHA1(bdd74239a3490eb35cb736cac6f5030bec4b2392) )
	ROM_CONTINUE(             0x28000, 0x08000 )

	ROM_REGION( 0x10000, "irem_audio:iremsound", 0 )   /* sound CPU */
	ROM_LOAD( "dr00.3a",      0x4000, 0x04000, CRC(458309f7) SHA1(7dfd77e0cd76a4b6f913eb434ac20ef5a172e2f0) )
	ROM_LOAD( "dr01.3cd",     0x8000, 0x04000, CRC(e66897bd) SHA1(04ea4a857a94d4e884fb28623ec6195dae701e25) )
	ROM_LOAD( "dr02.3f",      0xc000, 0x04000, CRC(f9e31e26) SHA1(712b1bde4b3c18c9ac26d58ade48316af004e733) ) /* 6803 code */

	ROM_REGION( 0x18000, "gfx1", 0 )
	ROM_LOAD( "dr06.2b",      0x00000, 0x8000, CRC(4d9a970f) SHA1(affeea31706644a9f1dcd1c4f739cadbdd58e597) )   /* tiles */
	ROM_LOAD( "dr07.2dc",     0x08000, 0x8000, CRC(ab59a4c4) SHA1(4e1eca8d4185ef35e2be13fa6a4af01f36d19f27) )
	ROM_LOAD( "dr05.2a",      0x10000, 0x8000, CRC(2e6dad0c) SHA1(fe79c510cea3f57ba61ac2e8d9858c887688897b) )

	ROM_REGION( 0x30000, "gfx2", 0 )
	ROM_LOAD( "dr21.4k",      0x00000, 0x4000, CRC(a06cea9a) SHA1(d78792b8e310fed3dc622c05d81303dad338afa7) )   /* sprites */
	ROM_LOAD( "dr19.4f",      0x04000, 0x4000, CRC(b34605ad) SHA1(56eba99c73527326f3428961732854753a6a5a2e) )
	ROM_LOAD( "dr22.4l",      0x08000, 0x4000, CRC(41303de8) SHA1(7bffda2e4c95b021f21d8375cf2d6b14280ea7b5) )
	ROM_LOAD( "dr20.4jh",     0x0c000, 0x4000, CRC(5fbe6f61) SHA1(18ab120777fffe912c8fc139fda2977a44aa453f) )
	ROM_LOAD( "dr14.3p",      0x10000, 0x4000, CRC(76cfbcbc) SHA1(3fdffc5893143b06535b5d85cd2a01c61d08e679) )
	ROM_LOAD( "dr24.4p",      0x14000, 0x4000, CRC(d51c8db5) SHA1(be48478afa0acc33e6a02a0a31f1f07ce3c7a3b8) )
	ROM_LOAD( "dr23.4nm",     0x18000, 0x4000, CRC(03469df8) SHA1(f3ffc0419c9d9c9a69387f62ceb9081170f861d4) )
	ROM_LOAD( "dr13.3nm",     0x1c000, 0x4000, CRC(d5c3dfe0) SHA1(c0e6b23bed9bf650d516a80c8b94f2d888d81c2d) )
	ROM_LOAD( "dr16.4cb",     0x20000, 0x4000, CRC(f1d1bb93) SHA1(42103d2e89a0d1edc834d08a6c443065fdacdbf2) )
	ROM_LOAD( "dr18.4e",      0x24000, 0x4000, CRC(edb7f25b) SHA1(394337132ff74b8d06d02233201f8ae8958e2aa6) )
	ROM_LOAD( "dr17.4dc",     0x28000, 0x4000, CRC(4fb87868) SHA1(73d779d687a6c319cd85793f95cb24413010b5e2) )
	ROM_LOAD( "dr15.4a",      0x2c000, 0x4000, CRC(e0b88de5) SHA1(08808ca90d34e494697a74e8a2314e32087e2f4d) )

	ROM_REGION( 0x0c000, "gfx3", 0 )
	ROM_LOAD( "dr08.4l",      0x00000, 0x4000, CRC(32d50643) SHA1(58bff370a3e24f21d5fb35289e85084b03178f88) )   /* chars */
	ROM_LOAD( "dr09.4m",      0x04000, 0x4000, CRC(17df6f95) SHA1(669a81906dfd81d807cbb2b5827ddb504536cb2c) )
	ROM_LOAD( "dr10.4n",      0x08000, 0x4000, CRC(820ce252) SHA1(910dbb910fdfcf9542360c0cd78c58c93d1d0c26) )

	ROM_REGION( 0x0920, "proms", 0 )
	ROM_LOAD( "dr25.3f",      0x0000, 0x0100, CRC(8e91430b) SHA1(a7a1567a0fd31cd65260f3ddb5280368704378bd) )    /* character palette red component */
	ROM_LOAD( "dr30.1m",      0x0100, 0x0100, CRC(28c73263) SHA1(ffeb8d1310759bf20b1624ab92fc91078726679c) )    /* sprite palette red component */
	ROM_LOAD( "dr26.3h",      0x0200, 0x0100, CRC(b563b93f) SHA1(86aefdaa63b35fe82f9f70eff3e4c14629f7a184) )    /* character palette green component */
	ROM_LOAD( "dr31.1n",      0x0300, 0x0100, CRC(3529210e) SHA1(3042ec941bdcb873077e77cffe36d4a28298bbbb) )    /* sprite palette green component */
	ROM_LOAD( "dr27.3j",      0x0400, 0x0100, CRC(70d668ef) SHA1(2cc647f2708932105bb9a5130aacc5a8a160e418) )    /* character palette blue component */
	ROM_LOAD( "dr29.1l",      0x0500, 0x0100, CRC(1173a754) SHA1(dbb7d02b72ae1842e0d17aee324a5b85ff2a2178) )    /* sprite palette blue component */
	ROM_LOAD( "dr32.5p",      0x0600, 0x0020, CRC(11cd1f2e) SHA1(45ceb84ff373127ff370610c1ce8d83fc6045bcb) )    /* sprite height, one entry per 32 */
															/* sprites. Used at run time! */
	ROM_LOAD( "dr28.8f",      0x0620, 0x0200, CRC(6cef0fbd) SHA1(0c5c63a203e7bd852a3574c18f212487caf529ca) )    /* unknown */
	ROM_LOAD( "dr33.6f",      0x0820, 0x0100, CRC(34d88d3c) SHA1(727f4c5cfff33538886fa0a29fd119aa085d7008) )    /* video timing - common to the other games */
ROM_END


ROM_START( yanchamr )
	ROM_REGION( 0x30000, "maincpu", 0 ) /* main CPU */
	ROM_LOAD( "ky_a-4e-.bin", 0x00000, 0x04000, CRC(c73ad2d6) SHA1(2e5d100e043f77c056b0d5bb80f310a6866fd2b1) )
	ROM_LOAD( "ky_a-4d-.bin", 0x04000, 0x04000, CRC(401af828) SHA1(eec1c082f42e441071fcf005803205b2275b0327) )
	ROM_LOAD( "ky_t-8k-.bin", 0x10000, 0x08000, CRC(e967de88) SHA1(75c0890eb98feb882fe01de5e93e228690e00904) )  /* banked at 8000-9fff */
	ROM_LOAD( "ky_t-8l-.bin", 0x18000, 0x08000, CRC(a929110b) SHA1(87334f946e14c79426bc7a14e8da984bb8ef9cfc) )
	/*  ROM_CONTINUE(             0x28000, 0x08000 ) */

	ROM_REGION( 0x10000, "irem_audio:iremsound", 0 )   /* sound CPU */
	ROM_LOAD( "ky_a-3a-.bin", 0x4000, 0x04000, CRC(cb365f3b) SHA1(fefad25459eb00d228ee29931c5714ae895b76c7) )
	ROM_LOAD( "dr01.3cd",     0x8000, 0x04000, CRC(e66897bd) SHA1(04ea4a857a94d4e884fb28623ec6195dae701e25) )
	ROM_LOAD( "dr02.3f",      0xc000, 0x04000, CRC(f9e31e26) SHA1(712b1bde4b3c18c9ac26d58ade48316af004e733) ) /* 6803 code */

	ROM_REGION( 0x18000, "gfx1", 0 )
	ROM_LOAD( "ky_t-2c-.bin", 0x00000, 0x8000, CRC(cb9761fc) SHA1(3eaf289ebd4ee1b1659dda0804fc0597ccc76218) )   /* tiles */
	ROM_LOAD( "ky_t-2d-.bin", 0x08000, 0x8000, CRC(59732741) SHA1(e77fbe3b0cd57a6a3fea7da46d8f23a4bcc7b583) )
	ROM_LOAD( "ky_t-2a-.bin", 0x10000, 0x8000, CRC(0370fd82) SHA1(0ddad9638b778f5651fccbec9b2c711c8ad07098) )

	ROM_REGION( 0x30000, "gfx2", 0 )
	ROM_LOAD( "ky_b-4k-.bin", 0x00000, 0x4000, CRC(263a9d10) SHA1(fd44163d8bb2e8b46d07f1ba827033f1fe873d29) )   /* sprites */
	ROM_LOAD( "ky_b-4f-.bin", 0x04000, 0x4000, CRC(86e3d4a8) SHA1(98d938e47308e90434e16b55ab90123cf18d34c6) )
	ROM_LOAD( "ky_b-4l-.bin", 0x08000, 0x4000, CRC(19fa7558) SHA1(0211e0d6af43b9ef6bb5e6115215c9c96e479e62) )
	ROM_LOAD( "ky_b-4h-.bin", 0x0c000, 0x4000, CRC(93e6665c) SHA1(c2ca394befcb01587882641d9b170a8a5c71646c) )
	ROM_LOAD( "ky_b-3n-.bin", 0x10000, 0x4000, CRC(0287c525) SHA1(5c19cc5806b5ef0846bcdf67ac762ba2a7934d5c) )
	ROM_LOAD( "ky_b-4n-.bin", 0x14000, 0x4000, CRC(764946e0) SHA1(224487fa62d80e7210bdff5ea90c82d97d6dee37) )
	ROM_LOAD( "ky_b-4m-.bin", 0x18000, 0x4000, CRC(eced5db9) SHA1(a6cf0c8cfe923166223c75e2ae8fbc35e625b21e) )
	ROM_LOAD( "ky_b-3m-.bin", 0x1c000, 0x4000, CRC(be6cee44) SHA1(c1a93bda531c8f227b7deb91368c08b6fc206baa) )
	ROM_LOAD( "ky_b-4c-.bin", 0x20000, 0x4000, CRC(84d6b65d) SHA1(675134ae87d64da8d8d4dcc344106888013eba35) )
	ROM_LOAD( "ky_b-4e-.bin", 0x24000, 0x4000, CRC(f91f9273) SHA1(a24f6b82eee10267f18751eb73a6dcd33e175a8a) )
	ROM_LOAD( "ky_b-4d-.bin", 0x28000, 0x4000, CRC(a2fc15f0) SHA1(27b668db7976325b66f7006aecebc6f5b196e16f) )
	ROM_LOAD( "ky_b-4a-.bin", 0x2c000, 0x4000, CRC(ff2b9c8a) SHA1(99bd093a7ad5c039740fbb73b61f1a309054dd68) )

	ROM_REGION( 0x0c000, "gfx3", 0 )
	ROM_LOAD( "ky_t-4l-.bin", 0x00000, 0x4000, CRC(1d0a9253) SHA1(4952c945502a19c6b4e7ab1ae6f5a374bad7fe60) )   /* chars */
	ROM_LOAD( "ky_t-4m-.bin", 0x04000, 0x4000, CRC(4075c396) SHA1(5d1612a89631800693c79dce01fa2494a8b1f49a) )
	ROM_LOAD( "ky_t-4n-.bin", 0x08000, 0x4000, CRC(7564f2ff) SHA1(fbf0adf3d8e15d899ece96e34019cfcc56c52ddb) )

	ROM_REGION( 0x0920, "proms", 0 )
	ROM_LOAD( "dr25.3f",      0x0000, 0x0100, CRC(8e91430b) SHA1(a7a1567a0fd31cd65260f3ddb5280368704378bd) )    /* character palette red component */
	ROM_LOAD( "dr30.1m",      0x0100, 0x0100, CRC(28c73263) SHA1(ffeb8d1310759bf20b1624ab92fc91078726679c) )    /* sprite palette red component */
	ROM_LOAD( "dr26.3h",      0x0200, 0x0100, CRC(b563b93f) SHA1(86aefdaa63b35fe82f9f70eff3e4c14629f7a184) )    /* character palette green component */
	ROM_LOAD( "dr31.1n",      0x0300, 0x0100, CRC(3529210e) SHA1(3042ec941bdcb873077e77cffe36d4a28298bbbb) )    /* sprite palette green component */
	ROM_LOAD( "dr27.3j",      0x0400, 0x0100, CRC(70d668ef) SHA1(2cc647f2708932105bb9a5130aacc5a8a160e418) )    /* character palette blue component */
	ROM_LOAD( "dr29.1l",      0x0500, 0x0100, CRC(1173a754) SHA1(dbb7d02b72ae1842e0d17aee324a5b85ff2a2178) )    /* sprite palette blue component */
	ROM_LOAD( "dr32.5p",      0x0600, 0x0020, CRC(11cd1f2e) SHA1(45ceb84ff373127ff370610c1ce8d83fc6045bcb) )    /* sprite height, one entry per 32 */
															/* sprites. Used at run time! */
	ROM_LOAD( "dr28.8f",      0x0620, 0x0200, CRC(6cef0fbd) SHA1(0c5c63a203e7bd852a3574c18f212487caf529ca) )    /* unknown */
	ROM_LOAD( "dr33.6f",      0x0820, 0x0100, CRC(34d88d3c) SHA1(727f4c5cfff33538886fa0a29fd119aa085d7008) )    /* video timing - common to the other games */
ROM_END

ROM_START( lithero )
	ROM_REGION( 0x30000, "maincpu", 0 ) /* main CPU */
	ROM_LOAD( "4.bin",        0x00000, 0x08000, CRC(80903766) SHA1(f16d603798f8a9ad62df8d89334758326257201a) )
	ROM_LOAD( "11.bin",       0x10000, 0x08000, CRC(7a1ef8cb) SHA1(5c94a06a5f64365068daee3d7da7f2a8e52479da) ) /* banked at 8000-9fff */
	ROM_LOAD( "12.bin",       0x18000, 0x08000, CRC(a929110b) SHA1(87334f946e14c79426bc7a14e8da984bb8ef9cfc) )
	/*  ROM_CONTINUE(             0x28000, 0x08000 ) */

	ROM_REGION( 0x10000, "irem_audio:iremsound", 0 )   /* sound CPU */
	ROM_LOAD( "ky_a-3a-.bin", 0x4000, 0x04000, CRC(cb365f3b) SHA1(fefad25459eb00d228ee29931c5714ae895b76c7) )
	ROM_LOAD( "dr01.3cd",     0x8000, 0x04000, CRC(e66897bd) SHA1(04ea4a857a94d4e884fb28623ec6195dae701e25) )
	ROM_LOAD( "dr02.3f",      0xc000, 0x04000, CRC(f9e31e26) SHA1(712b1bde4b3c18c9ac26d58ade48316af004e733) ) /* 6803 code */

	ROM_REGION( 0x18000, "gfx1", 0 )
	ROM_LOAD( "7.bin",        0x00000, 0x8000, CRC(b55e8d19) SHA1(a0ba1e3061aaecfac1fc879420c5b53884769ee9) )   /* tiles */
	ROM_LOAD( "6.bin",        0x08000, 0x8000, CRC(7bbbb209) SHA1(b03f2a1607bbc04f68a7689318129a25c8ccf71b) )
	ROM_LOAD( "5.bin",        0x10000, 0x8000, CRC(0370fd82) SHA1(0ddad9638b778f5651fccbec9b2c711c8ad07098) )

	ROM_REGION( 0x30000, "gfx2", 0 )
	ROM_LOAD( "16.bin",       0x00000, 0x8000, CRC(5045a507) SHA1(cdc0e0fc38262253f315b39a3b21cd1080a8b572) )   /* sprites */
	ROM_LOAD( "15.bin",       0x08000, 0x8000, CRC(946b16a0) SHA1(c7bef752a3597bbcc0ba43dc3fd9267b2b2ddeb0) )
	ROM_LOAD( "18.bin",       0x10000, 0x8000, CRC(901b69ff) SHA1(b3722a716cb4015ff56898bd21d87e56666292ff) )
	ROM_LOAD( "17.bin",       0x18000, 0x8000, CRC(504eed93) SHA1(4e512e8b3efcb09a9b56b0be4f8a25312be099be) )
	ROM_LOAD( "14.bin",       0x20000, 0x8000, CRC(429d760b) SHA1(d22d74193989fc3bb9a2a1c6c03479a87648939f) )
	ROM_LOAD( "13.bin",       0x28000, 0x8000, CRC(1700cd64) SHA1(6191824991634a79811805c08d3de07b045e92d9) )

	ROM_REGION( 0x0c000, "gfx3", 0 )
	ROM_LOAD( "8.bin",        0x00000, 0x4000, CRC(4f388d63) SHA1(a6bb88d33ed393a32fa354a363b7521639da458e) )   /* chars */
	ROM_LOAD( "9.bin",        0x04000, 0x4000, CRC(daafa2c1) SHA1(e7bd964faac5dfc1546e0ce629dbedf8d4da9ba6) )
	ROM_LOAD( "10.bin",       0x08000, 0x4000, CRC(60649d19) SHA1(38590a3d03655a5e1a95afa9279da49b65abfa2c) )

	ROM_REGION( 0x0920, "proms", 0 )
	ROM_LOAD( "dr25.3f",      0x0000, 0x0100, CRC(8e91430b) SHA1(a7a1567a0fd31cd65260f3ddb5280368704378bd) )    /* character palette red component */
	ROM_LOAD( "dr30.1m",      0x0100, 0x0100, CRC(28c73263) SHA1(ffeb8d1310759bf20b1624ab92fc91078726679c) )    /* sprite palette red component */
	ROM_LOAD( "dr26.3h",      0x0200, 0x0100, CRC(b563b93f) SHA1(86aefdaa63b35fe82f9f70eff3e4c14629f7a184) )    /* character palette green component */
	ROM_LOAD( "dr31.1n",      0x0300, 0x0100, CRC(3529210e) SHA1(3042ec941bdcb873077e77cffe36d4a28298bbbb) )    /* sprite palette green component */
	ROM_LOAD( "dr27.3j",      0x0400, 0x0100, CRC(70d668ef) SHA1(2cc647f2708932105bb9a5130aacc5a8a160e418) )    /* character palette blue component */
	ROM_LOAD( "dr29.1l",      0x0500, 0x0100, CRC(1173a754) SHA1(dbb7d02b72ae1842e0d17aee324a5b85ff2a2178) )    /* sprite palette blue component */
	ROM_LOAD( "dr32.5p",      0x0600, 0x0020, CRC(11cd1f2e) SHA1(45ceb84ff373127ff370610c1ce8d83fc6045bcb) )    /* sprite height, one entry per 32 */
															/* sprites. Used at run time! */
	ROM_LOAD( "dr28.8f",      0x0620, 0x0200, CRC(6cef0fbd) SHA1(0c5c63a203e7bd852a3574c18f212487caf529ca) )    /* unknown */
	ROM_LOAD( "dr33.6f",      0x0820, 0x0100, CRC(34d88d3c) SHA1(727f4c5cfff33538886fa0a29fd119aa085d7008) )    /* video timing - common to the other games */
ROM_END

ROM_START( spelunkr )
	ROM_REGION( 0x18000, "maincpu", 0 ) /* main CPU */
	ROM_LOAD( "spra.4e",      0x00000, 0x4000, CRC(cf811201) SHA1(9b29880b28e1d94b07a16b5455bc498dc882342f) )
	ROM_LOAD( "spra.4d",      0x04000, 0x4000, CRC(bb4faa4f) SHA1(350f7b086ed6357354d60548419d139fddb34c9d) )
	ROM_LOAD( "sprm.7c",      0x10000, 0x4000, CRC(fb6197e2) SHA1(8c3ccc2c14d076a1d6d14c2548a101a87af4211a) )   /* banked at 8000-9fff */
	ROM_LOAD( "sprm.7b",      0x14000, 0x4000, CRC(26bb25a4) SHA1(d384901042664a4e46d7b6b5d183ce49e360dac8) )

	ROM_REGION( 0x10000, "irem_audio:iremsound", 0 )   /* sound CPU */
	ROM_LOAD( "spra.3d",      0x8000, 0x04000, CRC(4110363c) SHA1(8c1f4966291887c17175ab921acd732be6266186) ) /* adpcm data */
	ROM_LOAD( "spra.3f",      0xc000, 0x04000, CRC(67a9d2e6) SHA1(d859648d7a3f05ae777a3909ddcd866b786c5b26) ) /* 6803 code */

	ROM_REGION( 0x18000, "gfx1", 0 )
	ROM_LOAD( "sprm.1d",      0x00000, 0x4000, CRC(4ef7ae89) SHA1(a32362d6002300838d99948b22d687d60d033060) )   /* tiles */
	ROM_LOAD( "sprm.1e",      0x04000, 0x4000, CRC(a3755180) SHA1(e0db62209a4e35efdaac30ab3343854bdb58d418) )
	ROM_LOAD( "sprm.3c",      0x08000, 0x4000, CRC(b4008e6a) SHA1(ee144514b18dcb54f2efc503181c9feaa898ea25) )
	ROM_LOAD( "sprm.3b",      0x0c000, 0x4000, CRC(f61cf012) SHA1(9803a80a2ef0f6469e7b00faf5b11c98ac47854f) )
	ROM_LOAD( "sprm.1c",      0x10000, 0x4000, CRC(58b21c76) SHA1(0e524be89035208a0c212ff160150fc82ac4da7f) )
	ROM_LOAD( "sprm.1b",      0x14000, 0x4000, CRC(a95cb3e5) SHA1(4bb1e3aa81f8594cda1646e0c50b82250ae3e3b0) )

	ROM_REGION( 0x18000, "gfx2", 0 )
	ROM_LOAD( "sprb.4k",      0x00000, 0x4000, CRC(e7f0e861) SHA1(864ea5bcd1a2f87c1d63ec9fb27cf69281b1697e) )   /* sprites */
	ROM_LOAD( "sprb.4f",      0x04000, 0x4000, CRC(32663097) SHA1(8cfa5e6b1713194ab435af6523e5df4ee266dc73) )
	ROM_LOAD( "sprb.3p",      0x08000, 0x4000, CRC(8fbaf373) SHA1(cc0bee147f09f417c1d588440de6622bf6967a73) )
	ROM_LOAD( "sprb.4p",      0x0c000, 0x4000, CRC(37069b76) SHA1(c31c2f5575219c927a8ed6d1a9fc88bbf494c0c0) )
	ROM_LOAD( "sprb.4c",      0x10000, 0x4000, CRC(cfe46a88) SHA1(150d106c0aeec87b993f29a01904e7a65eda0921) )
	ROM_LOAD( "sprb.4e",      0x14000, 0x4000, CRC(11c48979) SHA1(4500fb0f10b6421d82f89e18d6f20406061a3ecd) )

	ROM_REGION( 0x0c000, "gfx3", 0 )
	ROM_LOAD( "sprm.4p",      0x00000, 0x0800, CRC(4dfe2e63) SHA1(981950cabc40e052c021fbe882ce3f1187a832fd) )   /* chars */
	ROM_CONTINUE(             0x02000, 0x0800 )         /* first and second half identical, */
	ROM_CONTINUE(             0x00800, 0x0800 )         /* second half not used by the driver */
	ROM_CONTINUE(             0x02800, 0x0800 )
	ROM_CONTINUE(             0x01000, 0x0800 )
	ROM_CONTINUE(             0x03000, 0x0800 )
	ROM_CONTINUE(             0x00800, 0x0800 )
	ROM_CONTINUE(             0x03800, 0x0800 )
	ROM_LOAD( "sprm.4l",      0x04000, 0x0800, CRC(239f2cd4) SHA1(dbf602c33d8f2c613971f16bd1da4d7263f32c69) )
	ROM_CONTINUE(             0x06000, 0x0800 )
	ROM_CONTINUE(             0x04800, 0x0800 )
	ROM_CONTINUE(             0x06800, 0x0800 )
	ROM_CONTINUE(             0x05000, 0x0800 )
	ROM_CONTINUE(             0x07000, 0x0800 )
	ROM_CONTINUE(             0x05800, 0x0800 )
	ROM_CONTINUE(             0x07800, 0x0800 )
	ROM_LOAD( "sprm.4m",      0x08000, 0x0800, CRC(d6d07d70) SHA1(4f74efcc486775dfa95279b6e26a01b60cc43795) )
	ROM_CONTINUE(             0x0a000, 0x0800 )
	ROM_CONTINUE(             0x08800, 0x0800 )
	ROM_CONTINUE(             0x0a800, 0x0800 )
	ROM_CONTINUE(             0x09000, 0x0800 )
	ROM_CONTINUE(             0x0b000, 0x0800 )
	ROM_CONTINUE(             0x09800, 0x0800 )
	ROM_CONTINUE(             0x0b800, 0x0800 )

	ROM_REGION( 0x0920, "proms", 0 )
	ROM_LOAD( "sprm.2k",      0x0000, 0x0100, CRC(fd8fa991) SHA1(6e546a57de10223886a9a7480580b03b759dbd87) )    /* character palette red component */
	ROM_LOAD( "sprb.1m",      0x0100, 0x0100, CRC(8d8cccad) SHA1(e984d358b6fac9e3cb4618d11ddb22e9eb422dd0) )    /* sprite palette red component */
	ROM_LOAD( "sprm.2j",      0x0200, 0x0100, CRC(0e3890b4) SHA1(1b7c858a5729ddd3cbc7329b93082ec588a55131) )    /* character palette green component */
	ROM_LOAD( "sprb.1n",      0x0300, 0x0100, CRC(c40e1cb2) SHA1(fb2aac95c852ef67d03fd2c4b5f5f9330405d435) )    /* sprite palette green component */
	ROM_LOAD( "sprm.2h",      0x0400, 0x0100, CRC(0478082b) SHA1(e831ba7ef71632da2ab0bcc3cebbd6ef9f39a690) )    /* character palette blue component */
	ROM_LOAD( "sprb.1l",      0x0500, 0x0100, CRC(3ec46248) SHA1(734fe63b9f6e60cdd3bcc9664521b20ffe2765d9) )    /* sprite palette blue component */
	ROM_LOAD( "sprb.5p",      0x0600, 0x0020, CRC(746c6238) SHA1(10b901bb1eca69b274999ad7ada3dd6c58bc5d84) )    /* sprite height, one entry per 32 */
															/* sprites. Used at run time! */
	ROM_LOAD( "sprm.8h",      0x0620, 0x0200, CRC(875cc442) SHA1(1117b6ae516c361b4cc4d0b7146ca98472ce2b21) )    /* unknown */
	ROM_LOAD( "sprb.6f",      0x0820, 0x0100, CRC(34d88d3c) SHA1(727f4c5cfff33538886fa0a29fd119aa085d7008) )    /* video timing - common to the other games */
ROM_END

ROM_START( spelunkrj )
	ROM_REGION( 0x18000, "maincpu", 0 ) /* main CPU */
	ROM_LOAD( "spr_a4ec.bin", 0x00000, 0x4000, CRC(4e94a80c) SHA1(591ec5aa3fb508eedd120d9f7fb9454c3547148a) )
	ROM_LOAD( "spr_a4dd.bin", 0x04000, 0x4000, CRC(e7c0cbce) SHA1(19f914e9155972c91ccc8dc5b133f35246613c52) )
	ROM_LOAD( "spr_m7cc.bin", 0x10000, 0x4000, CRC(57598a36) SHA1(7d9c5790eb2a79c7977ca7b590d228685bd3a6b2) )   /* banked at 8000-9fff */
	ROM_LOAD( "spr_m7bd.bin", 0x14000, 0x4000, CRC(ecf5137f) SHA1(6daa88b40698e2a3a11206cd946465621bb3c059) )

	ROM_REGION( 0x10000, "irem_audio:iremsound", 0 )   /* sound CPU */
	ROM_LOAD( "spra.3d",      0x8000, 0x04000, CRC(4110363c) SHA1(8c1f4966291887c17175ab921acd732be6266186) ) /* adpcm data */
	ROM_LOAD( "spra.3f",      0xc000, 0x04000, CRC(67a9d2e6) SHA1(d859648d7a3f05ae777a3909ddcd866b786c5b26) ) /* 6803 code */

	ROM_REGION( 0x18000, "gfx1", 0 )
	ROM_LOAD( "sprm.1d",      0x00000, 0x4000, CRC(4ef7ae89) SHA1(a32362d6002300838d99948b22d687d60d033060) )   /* tiles */
	ROM_LOAD( "sprm.1e",      0x04000, 0x4000, CRC(a3755180) SHA1(e0db62209a4e35efdaac30ab3343854bdb58d418) )
	ROM_LOAD( "sprm.3c",      0x08000, 0x4000, CRC(b4008e6a) SHA1(ee144514b18dcb54f2efc503181c9feaa898ea25) )
	ROM_LOAD( "sprm.3b",      0x0c000, 0x4000, CRC(f61cf012) SHA1(9803a80a2ef0f6469e7b00faf5b11c98ac47854f) )
	ROM_LOAD( "sprm.1c",      0x10000, 0x4000, CRC(58b21c76) SHA1(0e524be89035208a0c212ff160150fc82ac4da7f) )
	ROM_LOAD( "sprm.1b",      0x14000, 0x4000, CRC(a95cb3e5) SHA1(4bb1e3aa81f8594cda1646e0c50b82250ae3e3b0) )

	ROM_REGION( 0x18000, "gfx2", 0 )
	ROM_LOAD( "sprb.4k",      0x00000, 0x4000, CRC(e7f0e861) SHA1(864ea5bcd1a2f87c1d63ec9fb27cf69281b1697e) )   /* sprites */
	ROM_LOAD( "sprb.4f",      0x04000, 0x4000, CRC(32663097) SHA1(8cfa5e6b1713194ab435af6523e5df4ee266dc73) )
	ROM_LOAD( "sprb.3p",      0x08000, 0x4000, CRC(8fbaf373) SHA1(cc0bee147f09f417c1d588440de6622bf6967a73) )
	ROM_LOAD( "sprb.4p",      0x0c000, 0x4000, CRC(37069b76) SHA1(c31c2f5575219c927a8ed6d1a9fc88bbf494c0c0) )
	ROM_LOAD( "sprb.4c",      0x10000, 0x4000, CRC(cfe46a88) SHA1(150d106c0aeec87b993f29a01904e7a65eda0921) )
	ROM_LOAD( "sprb.4e",      0x14000, 0x4000, CRC(11c48979) SHA1(4500fb0f10b6421d82f89e18d6f20406061a3ecd) )

	ROM_REGION( 0x0c000, "gfx3", 0 )
	ROM_LOAD( "sprm.4p",      0x00000, 0x0800, CRC(4dfe2e63) SHA1(981950cabc40e052c021fbe882ce3f1187a832fd) )   /* chars */
	ROM_CONTINUE(             0x02000, 0x0800 )         /* first and second half identical, */
	ROM_CONTINUE(             0x00800, 0x0800 )         /* second half not used by the driver */
	ROM_CONTINUE(             0x02800, 0x0800 )
	ROM_CONTINUE(             0x01000, 0x0800 )
	ROM_CONTINUE(             0x03000, 0x0800 )
	ROM_CONTINUE(             0x00800, 0x0800 )
	ROM_CONTINUE(             0x03800, 0x0800 )
	ROM_LOAD( "sprm.4l",      0x04000, 0x0800, CRC(239f2cd4) SHA1(dbf602c33d8f2c613971f16bd1da4d7263f32c69) )
	ROM_CONTINUE(             0x06000, 0x0800 )
	ROM_CONTINUE(             0x04800, 0x0800 )
	ROM_CONTINUE(             0x06800, 0x0800 )
	ROM_CONTINUE(             0x05000, 0x0800 )
	ROM_CONTINUE(             0x07000, 0x0800 )
	ROM_CONTINUE(             0x05800, 0x0800 )
	ROM_CONTINUE(             0x07800, 0x0800 )
	ROM_LOAD( "sprm.4m",      0x08000, 0x0800, CRC(d6d07d70) SHA1(4f74efcc486775dfa95279b6e26a01b60cc43795) )
	ROM_CONTINUE(             0x0a000, 0x0800 )
	ROM_CONTINUE(             0x08800, 0x0800 )
	ROM_CONTINUE(             0x0a800, 0x0800 )
	ROM_CONTINUE(             0x09000, 0x0800 )
	ROM_CONTINUE(             0x0b000, 0x0800 )
	ROM_CONTINUE(             0x09800, 0x0800 )
	ROM_CONTINUE(             0x0b800, 0x0800 )

	ROM_REGION( 0x0920, "proms", 0 )
	ROM_LOAD( "sprm.2k",      0x0000, 0x0100, CRC(fd8fa991) SHA1(6e546a57de10223886a9a7480580b03b759dbd87) )    /* character palette red component */
	ROM_LOAD( "sprb.1m",      0x0100, 0x0100, CRC(8d8cccad) SHA1(e984d358b6fac9e3cb4618d11ddb22e9eb422dd0) )    /* sprite palette red component */
	ROM_LOAD( "sprm.2j",      0x0200, 0x0100, CRC(0e3890b4) SHA1(1b7c858a5729ddd3cbc7329b93082ec588a55131) )    /* character palette green component */
	ROM_LOAD( "sprb.1n",      0x0300, 0x0100, CRC(c40e1cb2) SHA1(fb2aac95c852ef67d03fd2c4b5f5f9330405d435) )    /* sprite palette green component */
	ROM_LOAD( "sprm.2h",      0x0400, 0x0100, CRC(0478082b) SHA1(e831ba7ef71632da2ab0bcc3cebbd6ef9f39a690) )    /* character palette blue component */
	ROM_LOAD( "sprb.1l",      0x0500, 0x0100, CRC(3ec46248) SHA1(734fe63b9f6e60cdd3bcc9664521b20ffe2765d9) )    /* sprite palette blue component */
	ROM_LOAD( "sprb.5p",      0x0600, 0x0020, CRC(746c6238) SHA1(10b901bb1eca69b274999ad7ada3dd6c58bc5d84) )    /* sprite height, one entry per 32 */
															/* sprites. Used at run time! */
	ROM_LOAD( "sprm.8h",      0x0620, 0x0200, CRC(875cc442) SHA1(1117b6ae516c361b4cc4d0b7146ca98472ce2b21) )    /* unknown */
	ROM_LOAD( "sprb.6f",      0x0820, 0x0100, CRC(34d88d3c) SHA1(727f4c5cfff33538886fa0a29fd119aa085d7008) )    /* video timing - common to the other games */
ROM_END

ROM_START( spelunk2 )
	ROM_REGION( 0x24000, "maincpu", 0 ) /* main CPU */
	ROM_LOAD( "sp2-a.4e",     0x00000, 0x4000, CRC(96c04bbb) SHA1(5d7ee2d21d30e3ccbb428c2f9771568dbf3cfdb4) )
	ROM_LOAD( "sp2-a.4d",     0x04000, 0x4000, CRC(cb38c2ff) SHA1(28ab5f0c65657ee0eaa82275bdb60298eedd3821) )
	ROM_LOAD( "sp2-r.7d",     0x10000, 0x8000, CRC(558837ea) SHA1(5fa8a5ed55d155c3fc117391ab779c77e86fa349) )   /* banked at 9000-9fff */
	ROM_LOAD( "sp2-r.7c",     0x18000, 0x8000, CRC(4b380162) SHA1(867e441411e8b74d1d6ce0333c47b1aec3d4f5f2) )   /* banked at 9000-9fff */
	ROM_LOAD( "sp2-r.7b",     0x20000, 0x4000, CRC(7709a1fe) SHA1(4c2b57982b3d3e4524a8e0d24f38d3c3f5a809f3) )   /* banked at 8000-8fff */

	ROM_REGION( 0x10000, "irem_audio:iremsound", 0 )   /* sound CPU */
	ROM_LOAD( "sp2-a.3d",     0x8000, 0x04000, CRC(839ec7e2) SHA1(a2c45553b149fc16b8af6338627cd2a8a31e08a0) ) /* adpcm data */
	ROM_LOAD( "sp2-a.3f",     0xc000, 0x04000, CRC(ad3ce898) SHA1(36876b6d51a480a0664413dfcc57ef343e0f9965) ) /* 6803 code */

	ROM_REGION( 0x18000, "gfx1", 0 )
	ROM_LOAD( "sp2-r.1d",     0x00000, 0x8000, CRC(c19fa4c9) SHA1(49463ce1a813e4df2a3c1ae214a9d7c5cd1cc39f) )   /* tiles */
	ROM_LOAD( "sp2-r.3b",     0x08000, 0x8000, CRC(366604af) SHA1(1c7ab47693984971f94b70a5fc827a2dc33b1446) )
	ROM_LOAD( "sp2-r.1b",     0x10000, 0x8000, CRC(3a0c4d47) SHA1(b1fd056990074f1cec084cdc152006ec64f7e279) )

	ROM_REGION( 0x18000, "gfx2", 0 )
	ROM_LOAD( "sp2-b.4k",     0x00000, 0x4000, CRC(6cb67a17) SHA1(599c9dbf4fb649d8a19f9531ef5eb37216051715) )   /* sprites */
	ROM_LOAD( "sp2-b.4f",     0x04000, 0x4000, CRC(e4a1166f) SHA1(7ec14fc0b96ab4118cb4727c4292ac45805364d4) )
	ROM_LOAD( "sp2-b.3n",     0x08000, 0x4000, CRC(f59e8b76) SHA1(fd33d72235c8bb119e40f20a36bce750a532dbd1) )
	ROM_LOAD( "sp2-b.4n",     0x0c000, 0x4000, CRC(fa65bac9) SHA1(c394611e2323e7e0bc4bab897f88c3e7fbc2cbd3) )
	ROM_LOAD( "sp2-b.4c",     0x10000, 0x4000, CRC(1caf7013) SHA1(a2c59ce7dfc48913631d17ac4feedd873a6ad6ca) )
	ROM_LOAD( "sp2-b.4e",     0x14000, 0x4000, CRC(780a463b) SHA1(52b70027633cf58b2831dab3ab29fc54b336bac7) )

	ROM_REGION( 0x0c000, "gfx3", 0 )
	ROM_LOAD( "sp2-r.4l",     0x00000, 0x0800, CRC(6a4b2d8b) SHA1(34d3577d867882aa1f154ad000504831dd6262bc) )   /* chars */
	ROM_CONTINUE(             0x02000, 0x0800 )         /* first and second half identical, */
	ROM_CONTINUE(             0x00800, 0x0800 )         /* second half not used by the driver */
	ROM_CONTINUE(             0x02800, 0x0800 )
	ROM_CONTINUE(             0x00000, 0x0800 )
	ROM_CONTINUE(             0x03000, 0x0800 )
	ROM_CONTINUE(             0x00800, 0x0800 )
	ROM_CONTINUE(             0x03800, 0x0800 )
	ROM_LOAD( "sp2-r.4m",     0x04000, 0x0800, CRC(e1368b61) SHA1(75a46a5cc87605722c0407bfa2cf032b146c7674) )
	ROM_CONTINUE(             0x06000, 0x0800 )
	ROM_CONTINUE(             0x04800, 0x0800 )
	ROM_CONTINUE(             0x06800, 0x0800 )
	ROM_CONTINUE(             0x05000, 0x0800 )
	ROM_CONTINUE(             0x07000, 0x0800 )
	ROM_CONTINUE(             0x05800, 0x0800 )
	ROM_CONTINUE(             0x07800, 0x0800 )
	ROM_LOAD( "sp2-r.4p",     0x08000, 0x0800, CRC(fc138e13) SHA1(4f2a86e4f3fc0896a9ec35d5fdee9181b1c84aec) )
	ROM_CONTINUE(             0x0a000, 0x0800 )
	ROM_CONTINUE(             0x08800, 0x0800 )
	ROM_CONTINUE(             0x0a800, 0x0800 )
	ROM_CONTINUE(             0x09000, 0x0800 )
	ROM_CONTINUE(             0x0b000, 0x0800 )
	ROM_CONTINUE(             0x09800, 0x0800 )
	ROM_CONTINUE(             0x0b800, 0x0800 )

	ROM_REGION( 0x0a20, "proms", 0 )
	ROM_LOAD( "sp2-r.1k",     0x0000, 0x0200, CRC(31c1bcdc) SHA1(6504d5bafad427a1104562f84319d9e29f6c4800) )    /* chars red and green component */
	ROM_LOAD( "sp2-r.2k",     0x0200, 0x0100, CRC(1cf5987e) SHA1(811538304aff683d2a2a925b7e7ac990454d75f4) )    /* chars blue component */
	ROM_LOAD( "sp2-r.2j",     0x0300, 0x0100, CRC(1acbe2a5) SHA1(22b6eb43733eb40c6d2deb8a008e43c651d891e8) )    /* chars blue component */
	ROM_LOAD( "sp2-b.1m",     0x0400, 0x0100, CRC(906104c7) SHA1(e5e656d4da7f9dac32e2a112ce03be5dc3a4c46e) )    /* sprites red component */
	ROM_LOAD( "sp2-b.1n",     0x0500, 0x0100, CRC(5a564c06) SHA1(b9234983ccd69b90ae3aed19b1ac4c6c329d8302) )    /* sprites green component */
	ROM_LOAD( "sp2-b.1l",     0x0600, 0x0100, CRC(8f4a2e3c) SHA1(1d445ce63fb3e043d67f98d158dc7f0fab244248) )    /* sprites blue component */
	ROM_LOAD( "sp2-b.5p",     0x0700, 0x0020, CRC(cd126f6a) SHA1(f5a902bc93dbc98f1c78e08699ed7d1fc5d03481) )    /* sprite height, one entry per 32 */
															/* sprites. Used at run time! */
	ROM_LOAD( "sp2-r.8j",     0x0720, 0x0200, CRC(875cc442) SHA1(1117b6ae516c361b4cc4d0b7146ca98472ce2b21) )    /* unknown */
	ROM_LOAD( "sp2-b.6f",     0x0920, 0x0100, CRC(34d88d3c) SHA1(727f4c5cfff33538886fa0a29fd119aa085d7008) )    /* video timing - common to the other games */

	ROM_REGION( 0x0200, "plds", 0 )
	ROM_LOAD( "ampal16r4a-sp2-r-3h.bin", 0x0000, 0x0104, NO_DUMP ) /* PAL is read protected */
ROM_END

ROM_START( youjyudn )
	ROM_REGION( 0x18000, "maincpu", 0 ) /* main CPU */
	ROM_LOAD( "yju_a4eb.bin", 0x00000, 0x4000, CRC(0d356bdc) SHA1(20ddd68720d733791fb3cf791b26e100af12d609) )
	ROM_LOAD( "yju_a4db.bin", 0x04000, 0x4000, CRC(c169be13) SHA1(2f7e65924d152206fae97e5f50197963e2c566df) )
	ROM_LOAD( "yju_p4cb.0",   0x10000, 0x4000, CRC(60baf3b1) SHA1(2ab577bdbec7c8695a8f530b3e476aa91447cd60) )   /* banked at 8000-bfff */
	ROM_LOAD( "yju_p4eb.1",   0x14000, 0x4000, CRC(8d0521f8) SHA1(e03385b94194806e347cd6be4151686e2f38c890) )

	ROM_REGION( 0x10000, "irem_audio:iremsound", 0 )   /* sound CPU */
	ROM_LOAD( "yju_a3fb.bin", 0xc000, 0x04000, CRC(e15c8030) SHA1(bbbf4fa0349d614af86e356a106d5dd24b0b8baa) ) /* 6803 code */

	ROM_REGION( 0x0c000, "gfx1", 0 )
	ROM_LOAD( "yju_p3bb.0",   0x00000, 0x4000, CRC(c017913c) SHA1(587d143de2a3c057043cbc4b3f8e82ba7b35c256) )   /* tiles (first half empty) */
	ROM_CONTINUE(             0x00000, 0x4000 )
	ROM_LOAD( "yju_p1bb.1",   0x04000, 0x4000, CRC(94627523) SHA1(62c6dc736a891be9adb0f520263b0f1f2194ffed) )
	ROM_CONTINUE(             0x04000, 0x4000 )
	ROM_LOAD( "yju_p1cb.2",   0x08000, 0x4000, CRC(6a378c56) SHA1(c7bf28ac77dd7dd5ab44ad20e46e641df2def6b5) )
	ROM_CONTINUE(             0x08000, 0x4000 )

	ROM_REGION( 0x18000, "gfx2", 0 )
	ROM_LOAD( "yju_b4ka.00",  0x00000, 0x4000, CRC(1bbb864a) SHA1(facc45c363e402f348a0384ba402d9ee86baf508) )   /* sprites */
	ROM_LOAD( "yju_b4fa.01",  0x04000, 0x4000, CRC(14b4dd24) SHA1(c4d834ace82b2acc148415b24319410a901f3c44) )
	ROM_LOAD( "yju_b3na.10",  0x08000, 0x4000, CRC(68879321) SHA1(04410de27ad44e5bf9271006d169122726c3244d) )
	ROM_LOAD( "yju_b4na.11",  0x0c000, 0x4000, CRC(2860a68b) SHA1(7c9796d7e09c46ebcaf15239c3639f5b807291d6) )
	ROM_LOAD( "yju_b4ca.20",  0x10000, 0x4000, CRC(ab365829) SHA1(bd79af8fd332526da8274c2a4be608a49dadc197) )
	ROM_LOAD( "yju_b4ea.21",  0x14000, 0x4000, CRC(b36c31e4) SHA1(1fb920962abc2d7b32d3ac5c885b047e3434c0dd) )

	ROM_REGION( 0x0c000, "gfx3", 0 )
	ROM_LOAD( "yju_p4lb.2",   0x00000, 0x4000, CRC(87878d9b) SHA1(75f6ba4c8b93ceba2ed59958d7836f1b1ac9c6e1) )   /* chars */
	ROM_LOAD( "yju_p4mb.1",   0x04000, 0x4000, CRC(1e1a0d09) SHA1(7a4ef033d962f84e46368a21381456972d10b6dc) )
	ROM_LOAD( "yju_p4pb.0",   0x08000, 0x4000, CRC(b4ab520b) SHA1(646f2467e335e527662fec2bbcb771fbb5f1b696) )

	ROM_REGION( 0x0920, "proms", 0 )
	ROM_LOAD( "yju_p2jb.bpr", 0x0000, 0x0100, CRC(a4483631) SHA1(aa8a9fadb0b0624b5784f8e9b31f774c6360b47c) )    /* character palette red component */
	ROM_LOAD( "yju_b1ma.r",   0x0100, 0x0100, CRC(a8340e13) SHA1(258da6946043d9ba7a68402299f87fda26482a1a) )    /* sprite palette red component */
	ROM_LOAD( "yju_p2kb.bpr", 0x0200, 0x0100, CRC(85481103) SHA1(1216e359f9f2057f0c7f303f6e765ec39a316df6) )    /* character palette green component */
	ROM_LOAD( "yju_b1na.g",   0x0300, 0x0100, CRC(f5b4bc41) SHA1(78271b7078a8d485ce38e3a0d647f6c071441e62) )    /* sprite palette green component */
	ROM_LOAD( "yju_p2hb.bpr", 0x0400, 0x0100, CRC(a6fd355c) SHA1(98c25797c0f24cb2df775f18bcf899501d93ca2c) )    /* character palette blue component */
	ROM_LOAD( "yju_b1la.b",   0x0500, 0x0100, CRC(45e10491) SHA1(0ae8918a9854e44970b0d3eddc52867920711f1a) )    /* sprite palette blue component */
	ROM_LOAD( "yju_b-5p.bpr", 0x0600, 0x0020, CRC(2095e6a3) SHA1(32ef8b56d161807b6eff91b88636ffad558742ea) )    /* sprite height, one entry per 32 */
															/* sprites. Used at run time! */
	ROM_LOAD( "yju_p-8d.bpr", 0x0620, 0x0200, CRC(6cef0fbd) SHA1(0c5c63a203e7bd852a3574c18f212487caf529ca) )    /* unknown */
	ROM_LOAD( "yju_b-6f.bpr", 0x0820, 0x0100, CRC(82c20d12) SHA1(268903f7d9be58a70d030b02bf31a2d6b5b6e249) )    /* video timing - same as kungfum */

	ROM_REGION( 0x200, "plds", 0 )
	ROM_LOAD( "yju_b-pal16r4a-8m.pal", 0x0000, 0x0104, CRC(3ece8e61) SHA1(f7b04b80455068123e171a46e79d4b940bc4033d) )
ROM_END

ROM_START( horizon )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "hrza-4e",   0x0000, 0x4000, CRC(98b96ba2) SHA1(8478ec3b39c970c6008f078cbb5ab39462865015) )
	ROM_LOAD( "hrza-4d",   0x4000, 0x4000, CRC(06b06ac7) SHA1(d9ae9633455733fd14fc4d448b85365bfebef446) )
	ROM_LOAD( "hrza-4b",   0x8000, 0x4000, CRC(39c0bd02) SHA1(8d3b465ef7db11863cd2d343656e99cfb77c89ef) )

	ROM_REGION( 0x10000, "irem_audio:iremsound", 0 )   /* 64k for the audio CPU (6803) */
	ROM_LOAD( "hrza-3f",    0xc000, 0x4000, CRC(7412c99f) SHA1(33e7cffa08d9644c78e1bada9a2b08ce5d3f97e1) )

	ROM_REGION( 0x06000, "gfx1", 0 )
	ROM_LOAD( "hrzd-4d",    0x00000, 0x2000, CRC(b93ed581) SHA1(e9ab0925ebb3bceb12fb380ff157b162186e33b3) ) /* characters */
	ROM_LOAD( "hrzd-4c",    0x02000, 0x2000, CRC(1cf73b53) SHA1(f20a2154aedb40f3b454f13704354d0ae27e895b) )
	ROM_LOAD( "hrzd-4a",    0x04000, 0x2000, CRC(eace8d53) SHA1(6460c123795df42713571c00ed6a4be01629afa2) )

	ROM_REGION( 0x18000, "gfx2", 0 )
	ROM_LOAD( "hrzb-4k.00", 0x00000, 0x4000, CRC(11d2f3a1) SHA1(48e007a67c2473e680dfc5db38d130f13f3060b3) ) /* sprites */
	ROM_LOAD( "hrzb-4f.01", 0x04000, 0x4000, CRC(356902ea) SHA1(1f31b3ffa92a4c420f34f1348b7027b394c0a375) )
	ROM_LOAD( "hrzb-3n.10", 0x08000, 0x4000, CRC(87078a02) SHA1(538edcdcd63d811049a8d92ce8e4915ae8fb3a1a) )
	ROM_LOAD( "hrzb-4n.11", 0x0c000, 0x4000, CRC(5019cb1f) SHA1(02906541dc41df5a78a27389777cf1a3c7b5f392) )
	ROM_LOAD( "hrzb-4c.20", 0x10000, 0x4000, CRC(90b385e7) SHA1(d7a3698535bb4e9d96dd3e793b8051c74ea36eee) )
	ROM_LOAD( "hrzb-4e.21", 0x14000, 0x4000, CRC(d05d77a2) SHA1(b892f690ec4a0ed4e856c677867d6eac98afaa1d) )

	ROM_REGION( 0x0720, "proms", 0 )
	ROM_LOAD( "hrzd-1d",     0x0000, 0x0100, CRC(b33b08f9) SHA1(00b6c4be93c4d5d5f157d08e91dfea3a0ecdeb4a) ) /* character palette red component */
	ROM_LOAD( "hrzb-1m.r",   0x0100, 0x0100, CRC(0871690a) SHA1(8065598c64e44e4fd170632048161705f15c1d7d) ) /* sprite palette red component */
	ROM_LOAD( "hrzd-1c",     0x0200, 0x0100, CRC(6e696f3a) SHA1(d66ffe0cbc42889d750d9c8b7e57a84e5dacaf3d) ) /* character palette green component */
	ROM_LOAD( "hrzb-1n.g",   0x0300, 0x0100, CRC(f247d0a9) SHA1(7a2ae1e9699793fecb0abd84c2ee2b08e819b6f7) ) /* sprite palette green component */
	ROM_LOAD( "hrzd-1e",     0x0400, 0x0100, CRC(1fa60379) SHA1(4fdcc8d68f61afaae36075919b5bec4d12f7ed8e) ) /* character palette blue component */
	ROM_LOAD( "hrzb-1l.b",   0x0500, 0x0100, CRC(9ad0a0c8) SHA1(0c03906deafd6cc2247b022881e0190bd59c3c1b) ) /* sprite palette blue component */
	ROM_LOAD( "hrzb-5p",     0x0600, 0x0020, CRC(208b49e9) SHA1(065f1e05dd8bb94304969147e5d497931b5ff00a) ) /* sprite height, one entry per 32 */
															/* sprites. Used at run time! */
	ROM_LOAD( "hrzb-6f",     0x0620, 0x0100, CRC(82c20d12) SHA1(268903f7d9be58a70d030b02bf31a2d6b5b6e249) ) /* video timing - same as kungfum */
ROM_END


DRIVER_INIT_MEMBER(m62_state,battroad)
{
	/* configure memory banks */
	membank("bank1")->configure_entries(0, 16, memregion("maincpu")->base() + 0x10000, 0x2000);
}

DRIVER_INIT_MEMBER(m62_state,ldrun2)
{
	/* configure memory banks */
	membank("bank1")->configure_entries(0, 2, memregion("maincpu")->base() + 0x10000, 0x2000);
}

DRIVER_INIT_MEMBER(m62_state,ldrun4)
{
	/* configure memory banks */
	membank("bank1")->configure_entries(0, 2, memregion("maincpu")->base() + 0x10000, 0x4000);
}

DRIVER_INIT_MEMBER(m62_state,kidniki)
{
	/* configure memory banks */
	membank("bank1")->configure_entries(0, 16, memregion("maincpu")->base() + 0x10000, 0x2000);
}

DRIVER_INIT_MEMBER(m62_state,spelunkr)
{
	/* configure memory banks */
	membank("bank1")->configure_entries(0, 4, memregion("maincpu")->base() + 0x10000, 0x2000);
}

DRIVER_INIT_MEMBER(m62_state,spelunk2)
{
	/* configure memory banks */
	membank("bank1")->configure_entries(0,  4, memregion("maincpu")->base() + 0x20000, 0x1000);
	membank("bank2")->configure_entries(0, 16, memregion("maincpu")->base() + 0x10000, 0x1000);
}

DRIVER_INIT_MEMBER(m62_state,youjyudn)
{
	/* configure memory banks */
	membank("bank1")->configure_entries(0, 2, memregion("maincpu")->base() + 0x10000, 0x4000);
}

GAME( 1984, kungfum,  0,        kungfum,  kungfum,  driver_device, 0,        ROT0,   "Irem", "Kung-Fu Master (World)", MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_SOUND )
GAME( 1984, kungfumd, kungfum,  kungfum,  kungfum,  driver_device, 0,        ROT0,   "Irem (Data East USA license)", "Kung-Fu Master (US)", MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_SOUND )
GAME( 1984, spartanx, kungfum,  kungfum,  kungfum,  driver_device, 0,        ROT0,   "Irem", "Spartan X (Japan)", MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_SOUND )
GAME( 1984, kungfub,  kungfum,  kungfum,  kungfum,  driver_device, 0,        ROT0,   "bootleg", "Kung-Fu Master (bootleg set 1)", MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_SOUND )
GAME( 1984, kungfub2, kungfum,  kungfum,  kungfum,  driver_device, 0,        ROT0,   "bootleg", "Kung-Fu Master (bootleg set 2)", MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_SOUND )
GAME( 1984, battroad, 0,        battroad, battroad, m62_state,     battroad, ROT90,  "Irem", "The Battle-Road", MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_SOUND )
GAME( 1984, ldrun,    0,        ldrun,    ldrun,    driver_device, 0,        ROT0,   "Irem (licensed from Broderbund)", "Lode Runner (set 1)", MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_SOUND )
GAME( 1984, ldruna,   ldrun,    ldrun,    ldrun,    driver_device, 0,        ROT0,   "Irem (licensed from Broderbund, Digital Controls Inc. license)", "Lode Runner (set 2)", MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_SOUND )
GAME( 1984, ldrun2,   0,        ldrun2,   ldrun2,   m62_state,     ldrun2,   ROT0,   "Irem (licensed from Broderbund)", "Lode Runner II - The Bungeling Strikes Back", MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_SOUND ) /* Japanese version is called Bangeringu Teikoku No Gyakushuu */
GAME( 1985, ldrun3,   0,        ldrun3,   ldrun3,   driver_device, 0,        ROT0,   "Irem (licensed from Broderbund)", "Lode Runner III - The Golden Labyrinth", MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_SOUND )
GAME( 1985, ldrun3j,  ldrun3,   ldrun3,   ldrun3,   driver_device, 0,        ROT0,   "Irem (licensed from Broderbund)", "Lode Runner III - Majin No Fukkatsu (Japan)", MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_SOUND )
GAME( 1986, ldrun4,   0,        ldrun4,   ldrun4,   m62_state,     ldrun4,   ROT0,   "Irem (licensed from Broderbund)", "Lode Runner IV - Teikoku Karano Dasshutsu (Japan)", MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_SOUND )
GAME( 1985, lotlot,   0,        lotlot,   lotlot,   driver_device, 0,        ROT0,   "Irem (licensed from Tokuma Shoten)", "Lot Lot", MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_SOUND )
GAME( 1986, kidniki,  0,        kidniki,  kidniki,  m62_state,     kidniki,  ROT0,   "Irem", "Kid Niki - Radical Ninja (World)", MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_SOUND )
GAME( 1986, kidnikiu, kidniki,  kidniki,  kidniki,  m62_state,     kidniki,  ROT0,   "Irem (Data East USA license)", "Kid Niki - Radical Ninja (US)", MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_SOUND )
GAME( 1986, yanchamr, kidniki,  kidniki,  kidniki,  m62_state,     kidniki,  ROT0,   "Irem", "Kaiketsu Yanchamaru (Japan)", MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_SOUND )
GAME( 1987, lithero,  kidniki,  kidniki,  kidniki,  m62_state,     kidniki,  ROT0,   "bootleg", "Little Hero", MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_SOUND )
GAME( 1985, spelunkr, 0,        spelunkr, spelunkr, m62_state,     spelunkr, ROT0,   "Irem (licensed from Broderbund)", "Spelunker", MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_SOUND )
GAME( 1985, spelunkrj,spelunkr, spelunkr, spelunkr, m62_state,     spelunkr, ROT0,   "Irem (licensed from Broderbund)", "Spelunker (Japan)", MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_SOUND )
GAME( 1986, spelunk2, 0,        spelunk2, spelunk2, m62_state,     spelunk2, ROT0,   "Irem (licensed from Broderbund)", "Spelunker II", MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_SOUND )
GAME( 1986, youjyudn, 0,        youjyudn, youjyudn, m62_state,     youjyudn, ROT270, "Irem", "Youjyuden (Japan)", MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_SOUND )
GAME( 1985, horizon,  0,        horizon,  horizon,  driver_device, 0,        ROT0,   "Irem", "Horizon (Irem)", MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_SOUND )
