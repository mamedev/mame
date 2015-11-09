// license:???
// copyright-holders:Paul Leaman, Curt Coder
/***************************************************************************

  Sidearms
  ========

  Driver provided by Paul Leaman


Change Log:

MAY-2015 System11

- added turtshipko and turtshipkn.
- amended comments for T-5 instances, A14 is tied high on the PCBs hence the need to load the higher half of the ROM only
- order of age is guessed - turtshipko has grey bullets and a different level order, 3x horizontal and then 3x vertical (as far as tested).  Bullets probably fixed based on feedback before it was licensed to Sharp Image and Pacific Games - differences between US/JP/Korea versions previously in MAME are minimal.
- turtshipkn I assume is newer, the disclaimer is in pure Korean and the orange bullets are retained.  Given the 88/9 release date from the startup screen it would seem unlikely that this came out first, then got grey bullets and back to orange in time for it to still be a 1988 game in other countries.

JUL-2003 AAT

- cleaned video and corrected screen flipping

JUN-2003 (Curt Coder)

- converted driver to use tilemaps

FEB-2003 AAT

- added preliminary starfield emulation. circuit transcribed from
  schematics but still not perfect.

- rewrote video update and the following bugs seem to be fixed:

  sidearms060red:  attract mode and stage six crashing
  sidearms055gre:  strange background color
  turtship37b5yel: various graphics glitches and priority problems

Notes:

  Unknown PROMs are mostly used for timing. Only the first four sprite
  encoding parameters have been identified, the other 28(!) are
  believed to be line-buffer controls.

***************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "sound/2203intf.h"
#include "sound/2151intf.h"
#include "includes/sidearms.h"

void sidearms_state::machine_start()
{
	membank("bank1")->configure_entries(0, 16, memregion("maincpu")->base() + 0x8000, 0x4000);
}

WRITE8_MEMBER(sidearms_state::bankswitch_w)
{
	membank("bank1")->set_entry(data & 0x07);
}


/* Turtle Ship input ports are rotated 90 degrees */
IOPORT_ARRAY_MEMBER(sidearms_state::ports) { "SYSTEM", "P1", "P2", "DSW0", "DSW1" };

READ8_MEMBER(sidearms_state::turtship_ports_r)
{
	int res = 0;
	for (int i = 0; i < 5;i++)
		res |= ((read_safe(m_ports[i], 0) >> offset) & 1) << i;

	return res;
}


static ADDRESS_MAP_START( sidearms_map, AS_PROGRAM, 8, sidearms_state )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0x8000, 0xbfff) AM_ROMBANK("bank1")
	AM_RANGE(0xc000, 0xc3ff) AM_RAM_DEVWRITE("palette", palette_device, write) AM_SHARE("palette")
	AM_RANGE(0xc400, 0xc7ff) AM_RAM_DEVWRITE("palette", palette_device, write_ext) AM_SHARE("palette_ext")
	AM_RANGE(0xc800, 0xc800) AM_READ_PORT("SYSTEM") AM_WRITE(soundlatch_byte_w)
	AM_RANGE(0xc801, 0xc801) AM_READ_PORT("P1") AM_WRITE(bankswitch_w)
	AM_RANGE(0xc802, 0xc802) AM_READ_PORT("P2") AM_WRITE(watchdog_reset_w)
	AM_RANGE(0xc803, 0xc803) AM_READ_PORT("DSW0")
	AM_RANGE(0xc804, 0xc804) AM_READ_PORT("DSW1") AM_WRITE(c804_w)
	AM_RANGE(0xc805, 0xc805) AM_READ_PORT("DSW2") AM_WRITE(star_scrollx_w)
	AM_RANGE(0xc806, 0xc806) AM_WRITE(star_scrolly_w)
	AM_RANGE(0xc808, 0xc809) AM_WRITEONLY AM_SHARE("bg_scrollx")
	AM_RANGE(0xc80a, 0xc80b) AM_WRITEONLY AM_SHARE("bg_scrolly")
	AM_RANGE(0xc80c, 0xc80c) AM_WRITE(gfxctrl_w)   /* background and sprite enable */
	AM_RANGE(0xd000, 0xd7ff) AM_RAM_WRITE(videoram_w) AM_SHARE("videoram")
	AM_RANGE(0xd800, 0xdfff) AM_RAM_WRITE(colorram_w) AM_SHARE("colorram")
	AM_RANGE(0xe000, 0xefff) AM_RAM
	AM_RANGE(0xf000, 0xffff) AM_RAM AM_SHARE("spriteram")
ADDRESS_MAP_END

static ADDRESS_MAP_START( turtship_map, AS_PROGRAM, 8, sidearms_state )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0x8000, 0xbfff) AM_ROMBANK("bank1")
	AM_RANGE(0xc000, 0xcfff) AM_RAM
	AM_RANGE(0xd000, 0xdfff) AM_RAM AM_SHARE("spriteram")
	AM_RANGE(0xe000, 0xe3ff) AM_RAM_DEVWRITE("palette", palette_device, write) AM_SHARE("palette")
	AM_RANGE(0xe400, 0xe7ff) AM_RAM_DEVWRITE("palette", palette_device, write_ext) AM_SHARE("palette_ext")
	AM_RANGE(0xe800, 0xe807) AM_READ(turtship_ports_r)
	AM_RANGE(0xe800, 0xe800) AM_WRITE(soundlatch_byte_w)
	AM_RANGE(0xe801, 0xe801) AM_WRITE(bankswitch_w)
	AM_RANGE(0xe802, 0xe802) AM_WRITE(watchdog_reset_w)
	AM_RANGE(0xe804, 0xe804) AM_WRITE(c804_w)
	AM_RANGE(0xe805, 0xe805) AM_WRITE(star_scrollx_w)
	AM_RANGE(0xe806, 0xe806) AM_WRITE(star_scrolly_w)
	AM_RANGE(0xe808, 0xe809) AM_WRITEONLY AM_SHARE("bg_scrollx")
	AM_RANGE(0xe80a, 0xe80b) AM_WRITEONLY AM_SHARE("bg_scrolly")
	AM_RANGE(0xe80c, 0xe80c) AM_WRITE(gfxctrl_w)   /* background and sprite enable */
	AM_RANGE(0xf000, 0xf7ff) AM_RAM_WRITE(videoram_w) AM_SHARE("videoram")
	AM_RANGE(0xf800, 0xffff) AM_RAM_WRITE(colorram_w) AM_SHARE("colorram")
ADDRESS_MAP_END

static ADDRESS_MAP_START( sidearms_sound_map, AS_PROGRAM, 8, sidearms_state )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0xc000, 0xc7ff) AM_RAM
	AM_RANGE(0xd000, 0xd000) AM_READ(soundlatch_byte_r)
	AM_RANGE(0xf000, 0xf001) AM_DEVREADWRITE("ym1", ym2203_device, read, write)
	AM_RANGE(0xf002, 0xf003) AM_DEVREADWRITE("ym2", ym2203_device, read, write)
ADDRESS_MAP_END

/* Whizz */

WRITE8_MEMBER(sidearms_state::whizz_bankswitch_w)
{
	int bank = 0;
	switch (data & 0xC0)
	{
		case 0x00 : bank = 0;   break;
		case 0x40 : bank = 2;   break;
		case 0x80 : bank = 1;   break;
		case 0xC0 : bank = 3;   break;
	}
	membank("bank1")->set_entry(bank);
}

static ADDRESS_MAP_START( whizz_map, AS_PROGRAM, 8, sidearms_state )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0x8000, 0xbfff) AM_ROMBANK("bank1")
	AM_RANGE(0xc000, 0xc3ff) AM_RAM_DEVWRITE("palette", palette_device, write) AM_SHARE("palette")
	AM_RANGE(0xc400, 0xc7ff) AM_RAM_DEVWRITE("palette", palette_device, write_ext) AM_SHARE("palette_ext")
	AM_RANGE(0xc800, 0xc800) AM_READ_PORT("DSW0") AM_WRITE(soundlatch_byte_w)
	AM_RANGE(0xc801, 0xc801) AM_READ_PORT("DSW1") AM_WRITE(whizz_bankswitch_w)
	AM_RANGE(0xc802, 0xc802) AM_READ_PORT("DSW2") AM_WRITE(watchdog_reset_w)
	AM_RANGE(0xc803, 0xc803) AM_READ_PORT("IN0") AM_WRITENOP
	AM_RANGE(0xc804, 0xc804) AM_READ_PORT("IN1") AM_WRITE(c804_w)
	AM_RANGE(0xc805, 0xc805) AM_READ_PORT("IN2") AM_WRITENOP
	AM_RANGE(0xc806, 0xc806) AM_READ_PORT("IN3")
	AM_RANGE(0xc807, 0xc807) AM_READ_PORT("IN4")
	AM_RANGE(0xc808, 0xc809) AM_WRITEONLY AM_SHARE("bg_scrollx")
	AM_RANGE(0xc80a, 0xc80b) AM_WRITEONLY AM_SHARE("bg_scrolly")
	AM_RANGE(0xe805, 0xe805) AM_WRITE(star_scrollx_w)
	AM_RANGE(0xe806, 0xe806) AM_WRITE(star_scrolly_w)
	AM_RANGE(0xc80c, 0xc80c) AM_WRITE(gfxctrl_w)
	AM_RANGE(0xd000, 0xd7ff) AM_RAM_WRITE(videoram_w) AM_SHARE("videoram")
	AM_RANGE(0xd800, 0xdfff) AM_RAM_WRITE(colorram_w) AM_SHARE("colorram")
	AM_RANGE(0xe000, 0xefff) AM_RAM
	AM_RANGE(0xf000, 0xffff) AM_RAM AM_SHARE("spriteram")
ADDRESS_MAP_END

static ADDRESS_MAP_START( whizz_sound_map, AS_PROGRAM, 8, sidearms_state )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0xf800, 0xffff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( whizz_io_map, AS_IO, 8, sidearms_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x01) AM_DEVREADWRITE("ymsnd", ym2151_device, read, write)
	AM_RANGE(0x40, 0x40) AM_WRITENOP
	AM_RANGE(0xc0, 0xc0) AM_READ(soundlatch_byte_r)
ADDRESS_MAP_END


static INPUT_PORTS_START( sidearms )
	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_DIPNAME( 0x08, 0x08, "Freeze" )    /* I'm not sure it's really a dip switch */
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN2 )

	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_COCKTAIL
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW0")
	PORT_DIPNAME( 0x07, 0x04, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SW1:1,2,3")
	PORT_DIPSETTING(    0x07, "0 (Easiest)" )
	PORT_DIPSETTING(    0x06, "1" )
	PORT_DIPSETTING(    0x05, "2" )
	PORT_DIPSETTING(    0x04, "3 (Normal)" )
	PORT_DIPSETTING(    0x03, "4" )
	PORT_DIPSETTING(    0x02, "5" )
	PORT_DIPSETTING(    0x01, "6" )
	PORT_DIPSETTING(    0x00, "7 (Hardest)" )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Lives ) )        PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(    0x08, "3" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Bonus_Life ) )   PORT_DIPLOCATION("SW1:5,6")
	PORT_DIPSETTING(    0x30, "100000" )
	PORT_DIPSETTING(    0x20, "100000 100000" )
	PORT_DIPSETTING(    0x10, "150000 150000" )
	PORT_DIPSETTING(    0x00, "200000 200000" )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Flip_Screen ) )  PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_SERVICE_DIPLOC(  0x80, IP_ACTIVE_LOW, "SW1:8" )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x07, 0x07, DEF_STR( Coin_A ) )       PORT_DIPLOCATION("SW2:1,2,3")
	PORT_DIPSETTING(    0x00, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_6C ) )
	PORT_DIPNAME( 0x38, 0x38, DEF_STR( Coin_B ) )       PORT_DIPLOCATION("SW2:4,5,6")
	PORT_DIPSETTING(    0x00, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x38, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x28, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x18, DEF_STR( 1C_6C ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Allow_Continue ) )   PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START("DSW2")
	PORT_BIT( 0x7f, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_VBLANK("screen")     /* not sure, but likely */
INPUT_PORTS_END

static INPUT_PORTS_START( turtship )
	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN2 )

	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_COCKTAIL
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW0")
	PORT_DIPNAME( 0x01, 0x01, "Invulnerability (Cheat)")    PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING( 0x01, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Allow_Continue ) )   PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(    0x02, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Flip_Screen ) )  PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(    0x10, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hard ) )
	PORT_DIPNAME( 0xe0, 0xa0, DEF_STR( Lives ) )        PORT_DIPLOCATION("SW1:3,2,1")
	PORT_DIPSETTING(    0xe0, "1" )
	PORT_DIPSETTING(    0x60, "2" )
	PORT_DIPSETTING(    0xa0, "3" )
	PORT_DIPSETTING(    0x20, "4" )
	PORT_DIPSETTING(    0xc0, "5" )
	PORT_DIPSETTING(    0x40, "6" )
	PORT_DIPSETTING(    0x80, "7" )
	PORT_DIPSETTING(    0x00, "8" )

	PORT_START("DSW1")
	PORT_SERVICE_DIPLOC(   0x01, IP_ACTIVE_LOW, "SW2:8" )
	PORT_DIPUNUSED_DIPLOC( 0x02, IP_ACTIVE_LOW, "SW2:7" )
	PORT_DIPNAME( 0x0c, 0x08, DEF_STR( Bonus_Life ) )   PORT_DIPLOCATION("SW2:6,5")
	PORT_DIPSETTING(    0x08, "Every 150000" )
	PORT_DIPSETTING(    0x00, "Every 200000" )
	PORT_DIPSETTING(    0x0c, "150000 only" )
	PORT_DIPSETTING(    0x04, "200000 only" )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0xe0, 0xe0, DEF_STR( Coinage ) )      PORT_DIPLOCATION("SW2:3,2,1")
	PORT_DIPSETTING(    0x00, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0xe0, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x60, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0xa0, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 1C_4C ) )
	/* 0xc0 1 Coin/1 Credit */
INPUT_PORTS_END

static INPUT_PORTS_START( dyger )
	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )    /* seems to be 1-player only */
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("P2")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNKNOWN )    /* seems to be 1-player only */

	PORT_START("DSW0")
	PORT_DIPUNUSED_DIPLOC( 0x01, IP_ACTIVE_LOW, "SW1:8" )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Allow_Continue ) )   PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(    0x02, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPUNUSED_DIPLOC( 0x04, IP_ACTIVE_LOW, "SW1:6" )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Flip_Screen ) )  PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(    0x10, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hard ) )
	PORT_DIPNAME( 0xe0, 0xa0, DEF_STR( Lives ) )        PORT_DIPLOCATION("SW1:3,2,1")
	PORT_DIPSETTING(    0xe0, "1" )
	PORT_DIPSETTING(    0x60, "2" )
	PORT_DIPSETTING(    0xa0, "3" )
	PORT_DIPSETTING(    0x20, "4" )
	PORT_DIPSETTING(    0xc0, "5" )
	PORT_DIPSETTING(    0x40, "6" )
	PORT_DIPSETTING(    0x80, "7" )
	PORT_DIPSETTING(    0x00, "8" )

	PORT_START("DSW1")
	PORT_SERVICE_DIPLOC(   0x01, IP_ACTIVE_LOW, "SW2:8" )
	PORT_DIPUNUSED_DIPLOC( 0x02, IP_ACTIVE_LOW, "SW2:7" )
	PORT_DIPNAME( 0x0c, 0x04, DEF_STR( Bonus_Life ) )   PORT_DIPLOCATION("SW2:6,5")
	PORT_DIPSETTING(    0x04, "Every 150000" )
	PORT_DIPSETTING(    0x00, "Every 200000" )
	PORT_DIPSETTING(    0x0c, "150000 only" )
	PORT_DIPSETTING(    0x08, "200000 only" )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0xe0, 0xe0, DEF_STR( Coinage ) )      PORT_DIPLOCATION("SW2:3,2,1")
	PORT_DIPSETTING(    0x00, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0xe0, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x60, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0xa0, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 1C_4C ) )
	/* 0xc0 1 Coin/1 Credit */
INPUT_PORTS_END

static INPUT_PORTS_START( whizz )
	PORT_START("DSW0")  /* 8-bit */
	PORT_DIPNAME( 0x07, 0x04, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SW1:1,2,3")
	PORT_DIPSETTING(    0x07, "0 (Easiest)" )
	PORT_DIPSETTING(    0x06, "1" )
	PORT_DIPSETTING(    0x05, "2" )
	PORT_DIPSETTING(    0x04, "3 (Normal)" )
	PORT_DIPSETTING(    0x03, "4" )
	PORT_DIPSETTING(    0x02, "5" )
	PORT_DIPSETTING(    0x01, "6" )
	PORT_DIPSETTING(    0x00, "7 (Hardest)" )
	PORT_DIPUNUSED_DIPLOC( 0x08, IP_ACTIVE_LOW, "SW2:8" )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Flip_Screen ) )  PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0xe0, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW1")  /* 8-bit */
	PORT_DIPNAME( 0x07, 0x07, DEF_STR( Coin_A ) )       PORT_DIPLOCATION("SW2:1,2,3")
	PORT_DIPSETTING(    0x00, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_5C ) )
	PORT_DIPNAME( 0x18, 0x18, DEF_STR( Bonus_Life ) )   PORT_DIPLOCATION("SW1:5,6")
	PORT_DIPSETTING(    0x18, "100000 Only" )
	PORT_DIPSETTING(    0x10, "Every 100000" )
	PORT_DIPSETTING(    0x08, "Every 150000" )
	PORT_DIPSETTING(    0x00, "Every 200000" )
	PORT_BIT( 0xe0, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW2")  /* 8-bit */
	PORT_DIPNAME( 0x07, 0x07, DEF_STR( Coin_B ) )       PORT_DIPLOCATION("SW2:4,5,6")
	PORT_DIPSETTING(    0x00, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_5C ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Lives ) )        PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(    0x10, "3" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_BIT( 0xe0, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Allow_Continue ) )   PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Yes ) )
	PORT_BIT( 0xe0, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN1")   /* 8-bit */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0xe0, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN2")   /* 8-bit */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH,IPT_CUSTOM ) PORT_VBLANK("screen")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END


static const gfx_layout charlayout =
{
	8,8,    /* 8*8 characters */
	RGN_FRAC(1,1),   /* 1024 characters */
	2,      /* 2 bits per pixel */
	{ 4, 0 },
	{ 0, 1, 2, 3, 8+0, 8+1, 8+2, 8+3 },
	{ 0*16, 1*16, 2*16, 3*16, 4*16, 5*16, 6*16, 7*16 },
	16*8    /* every char takes 16 consecutive bytes */
};

static const gfx_layout spritelayout =
{
	16,16,  /* 16*16 sprites */
	RGN_FRAC(1,2),   /* 2048 sprites */
	4,      /* 4 bits per pixel */
	{ RGN_FRAC(1,2)+4, RGN_FRAC(1,2)+0, 4, 0 },
	{ 0, 1, 2, 3, 8+0, 8+1, 8+2, 8+3,
			32*8+0, 32*8+1, 32*8+2, 32*8+3, 33*8+0, 33*8+1, 33*8+2, 33*8+3 },
	{ 0*16, 1*16, 2*16, 3*16, 4*16, 5*16, 6*16, 7*16,
			8*16, 9*16, 10*16, 11*16, 12*16, 13*16, 14*16, 15*16 },
	64*8    /* every sprite takes 64 consecutive bytes */
};

static const gfx_layout tilelayout =
{
	32,32,  /* 32*32 tiles */
	RGN_FRAC(1,2),    /* 512 tiles */
	4,      /* 4 bits per pixel */
	{ RGN_FRAC(1,2)+4, RGN_FRAC(1,2)+0, 4, 0 },
	{
		0,       1,       2,       3,       8+0,       8+1,       8+2,       8+3,
		32*16+0, 32*16+1, 32*16+2, 32*16+3, 32*16+8+0, 32*16+8+1, 32*16+8+2, 32*16+8+3,
		64*16+0, 64*16+1, 64*16+2, 64*16+3, 64*16+8+0, 64*16+8+1, 64*16+8+2, 64*16+8+3,
		96*16+0, 96*16+1, 96*16+2, 96*16+3, 96*16+8+0, 96*16+8+1, 96*16+8+2, 96*16+8+3,
	},
	{
		0*16,  1*16,  2*16,  3*16,  4*16,  5*16,  6*16,  7*16,
		8*16,  9*16, 10*16, 11*16, 12*16, 13*16, 14*16, 15*16,
		16*16, 17*16, 18*16, 19*16, 20*16, 21*16, 22*16, 23*16,
		24*16, 25*16, 26*16, 27*16, 28*16, 29*16, 30*16, 31*16
	},
	256*8   /* every tile takes 256 consecutive bytes */
};

static GFXDECODE_START( sidearms )
	GFXDECODE_ENTRY( "gfx1", 0, charlayout,   768, 64 ) /* colors 768-1023 */
	GFXDECODE_ENTRY( "gfx2", 0, tilelayout,     0, 32 ) /* colors   0-511 */
	GFXDECODE_ENTRY( "gfx3", 0, spritelayout, 512, 16 ) /* colors 512-767 */
GFXDECODE_END



static const gfx_layout turtship_tilelayout =
{
	32,32,  /* 32*32 tiles */
	RGN_FRAC(1,2),    /* 768 tiles */
	4,      /* 4 bits per pixel */
	{ RGN_FRAC(1,2)+4, RGN_FRAC(1,2)+0, 4, 0 },
	{
		0,       1,       2,       3,       8+0,       8+1,       8+2,       8+3,
		32*16+0, 32*16+1, 32*16+2, 32*16+3, 32*16+8+0, 32*16+8+1, 32*16+8+2, 32*16+8+3,
		64*16+0, 64*16+1, 64*16+2, 64*16+3, 64*16+8+0, 64*16+8+1, 64*16+8+2, 64*16+8+3,
		96*16+0, 96*16+1, 96*16+2, 96*16+3, 96*16+8+0, 96*16+8+1, 96*16+8+2, 96*16+8+3,
	},
	{
		0*16,  1*16,  2*16,  3*16,  4*16,  5*16,  6*16,  7*16,
		8*16,  9*16, 10*16, 11*16, 12*16, 13*16, 14*16, 15*16,
		16*16, 17*16, 18*16, 19*16, 20*16, 21*16, 22*16, 23*16,
		24*16, 25*16, 26*16, 27*16, 28*16, 29*16, 30*16, 31*16
	},
	256*8   /* every tile takes 256 consecutive bytes */
};

static GFXDECODE_START( turtship )
	GFXDECODE_ENTRY( "gfx1", 0, charlayout,          768, 64 )  /* colors 768-1023 */
	GFXDECODE_ENTRY( "gfx2", 0, turtship_tilelayout,   0, 32 )  /* colors   0-511 */
	GFXDECODE_ENTRY( "gfx3", 0, spritelayout,        512, 16 )  /* colors 512-767 */
GFXDECODE_END


static MACHINE_CONFIG_START( sidearms, sidearms_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, 4000000) /* 4 MHz (?) */
	MCFG_CPU_PROGRAM_MAP(sidearms_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", sidearms_state,  irq0_line_hold)

	MCFG_CPU_ADD("audiocpu", Z80, 4000000) /* 4 MHz (?) */
	MCFG_CPU_PROGRAM_MAP(sidearms_sound_map)

	/* video hardware */
	MCFG_BUFFERED_SPRITERAM8_ADD("spriteram")

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500) /* not accurate */)
	MCFG_SCREEN_SIZE(64*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(8*8, (64-8)*8-1, 2*8, 30*8-1 )
	MCFG_SCREEN_UPDATE_DRIVER(sidearms_state, screen_update)
	MCFG_SCREEN_VBLANK_DEVICE("spriteram", buffered_spriteram8_device, vblank_copy_rising)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", sidearms)

	MCFG_PALETTE_ADD("palette", 1024)
	MCFG_PALETTE_FORMAT(xxxxBBBBRRRRGGGG)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("ym1", YM2203, 4000000)
	MCFG_YM2203_IRQ_HANDLER(INPUTLINE("audiocpu", 0))
	MCFG_SOUND_ROUTE(0, "mono", 0.15)
	MCFG_SOUND_ROUTE(1, "mono", 0.15)
	MCFG_SOUND_ROUTE(2, "mono", 0.15)
	MCFG_SOUND_ROUTE(3, "mono", 0.25)

	MCFG_SOUND_ADD("ym2", YM2203, 4000000)
	MCFG_SOUND_ROUTE(0, "mono", 0.15)
	MCFG_SOUND_ROUTE(1, "mono", 0.15)
	MCFG_SOUND_ROUTE(2, "mono", 0.15)
	MCFG_SOUND_ROUTE(3, "mono", 0.25)
MACHINE_CONFIG_END


static MACHINE_CONFIG_START( turtship, sidearms_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, 4000000) /* 4 MHz (?) */
	MCFG_CPU_PROGRAM_MAP(turtship_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", sidearms_state,  irq0_line_hold)

	MCFG_CPU_ADD("audiocpu", Z80, 4000000) /* 4 MHz (?) */
	MCFG_CPU_PROGRAM_MAP(sidearms_sound_map)

	/* video hardware */
	MCFG_BUFFERED_SPRITERAM8_ADD("spriteram")

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500) /* not accurate */)
	MCFG_SCREEN_SIZE(64*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(8*8, (64-8)*8-1, 2*8, 30*8-1 )
	MCFG_SCREEN_VBLANK_DEVICE("spriteram", buffered_spriteram8_device, vblank_copy_rising)
	MCFG_SCREEN_UPDATE_DRIVER(sidearms_state, screen_update)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", turtship)

	MCFG_PALETTE_ADD("palette", 1024)
	MCFG_PALETTE_FORMAT(xxxxBBBBRRRRGGGG)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("ym1", YM2203, 4000000)
	MCFG_YM2203_IRQ_HANDLER(INPUTLINE("audiocpu", 0))
	MCFG_SOUND_ROUTE(0, "mono", 0.15)
	MCFG_SOUND_ROUTE(1, "mono", 0.15)
	MCFG_SOUND_ROUTE(2, "mono", 0.15)
	MCFG_SOUND_ROUTE(3, "mono", 0.25)

	MCFG_SOUND_ADD("ym2", YM2203, 4000000)
	MCFG_SOUND_ROUTE(0, "mono", 0.15)
	MCFG_SOUND_ROUTE(1, "mono", 0.15)
	MCFG_SOUND_ROUTE(2, "mono", 0.15)
	MCFG_SOUND_ROUTE(3, "mono", 0.25)
MACHINE_CONFIG_END

static MACHINE_CONFIG_START( whizz, sidearms_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, 4000000)        /* 4 MHz (?) */
	MCFG_CPU_PROGRAM_MAP(whizz_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", sidearms_state,  irq0_line_hold)

	MCFG_CPU_ADD("audiocpu", Z80, 4000000)
	MCFG_CPU_PROGRAM_MAP(whizz_sound_map)
	MCFG_CPU_IO_MAP(whizz_io_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", sidearms_state,  irq0_line_hold)

	MCFG_QUANTUM_TIME(attotime::from_hz(60000))

	/* video hardware */
	MCFG_BUFFERED_SPRITERAM8_ADD("spriteram")

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500) /* not accurate */)
	MCFG_SCREEN_SIZE(64*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(8*8, (64-8)*8-1, 2*8, 30*8-1 )
	MCFG_SCREEN_UPDATE_DRIVER(sidearms_state, screen_update)
	MCFG_SCREEN_VBLANK_DEVICE("spriteram", buffered_spriteram8_device, vblank_copy_rising)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", turtship)

	MCFG_PALETTE_ADD("palette", 1024)
	MCFG_PALETTE_FORMAT(xxxxBBBBRRRRGGGG)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_YM2151_ADD("ymsnd", 4000000)
	MCFG_YM2151_IRQ_HANDLER(INPUTLINE("audiocpu", 0))
	MCFG_SOUND_ROUTE(0, "mono", 1.0)
	MCFG_SOUND_ROUTE(1, "mono", 1.0)
MACHINE_CONFIG_END




ROM_START( sidearms )
	ROM_REGION( 0x20000, "maincpu", 0 )     /* 64k for code + banked ROMs images */
	ROM_LOAD( "sa03.bin",     0x00000, 0x08000, CRC(e10fe6a0) SHA1(ae59461768d044f14b9aac3e4e491c76cec7adac) )        /* CODE */
	ROM_LOAD( "a_14e.rom",    0x08000, 0x08000, CRC(4925ed03) SHA1(b11dbd9889db89cff008ca21beb6b1b70d983e16) )        /* 0+1 */
	ROM_LOAD( "a_12e.rom",    0x10000, 0x08000, CRC(81d0ece7) SHA1(5c1d154f9c1de6b5f5d7abf5d413e9c493461e6f) )        /* 2+3 */

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "a_04k.rom",    0x0000, 0x8000, CRC(34efe2d2) SHA1(e1d8895c113e4dee1a132e2471d75dfa6c36b620) )

	ROM_REGION( 0x08000, "user1", 0 )    /* starfield data */
	ROM_LOAD( "b_11j.rom",    0x0000, 0x8000, CRC(134dc35b) SHA1(6360c1efa7c4e1d6d817a97ca43dd4af8ed6afe5) )

	ROM_REGION( 0x04000, "gfx1", 0 )
	ROM_LOAD( "a_10j.rom",    0x00000, 0x4000, CRC(651fef75) SHA1(9c821a2ee30c222987f0d4192133776490d6a4e0) ) /* characters */

	ROM_REGION( 0x40000, "gfx2", 0 )
	ROM_LOAD( "b_13d.rom",    0x00000, 0x8000, CRC(3c59afe1) SHA1(5459a5795cf13012674993aa55bbd39e9a5c2f1b) ) /* tiles */
	ROM_LOAD( "b_13e.rom",    0x08000, 0x8000, CRC(64bc3b77) SHA1(54fe6f258fda509a92eb0f5aa238102efce729e0) )
	ROM_LOAD( "b_13f.rom",    0x10000, 0x8000, CRC(e6bcea6f) SHA1(19477e284967beafc4e7cd0d0da3534eb6dec388) )
	ROM_LOAD( "b_13g.rom",    0x18000, 0x8000, CRC(c71a3053) SHA1(963e105aa0b0174e8aa5e1f7676c5c604ca72d1c) )
	ROM_LOAD( "b_14d.rom",    0x20000, 0x8000, CRC(826e8a97) SHA1(ad5ed9a81805dde54fb2703345b2ab7b56853ec6) )
	ROM_LOAD( "b_14e.rom",    0x28000, 0x8000, CRC(6cfc02a4) SHA1(491e880e85d5256fa2eea6d0fb402f0a1176b675) )
	ROM_LOAD( "b_14f.rom",    0x30000, 0x8000, CRC(9b9f6730) SHA1(0f8fe5dc32ee50ebb2051c0c0c4d635582416317) )
	ROM_LOAD( "b_14g.rom",    0x38000, 0x8000, CRC(ef6af630) SHA1(499b17eeb5e7256ede477510b0547df520316996) )

	ROM_REGION( 0x40000, "gfx3", 0 )
	ROM_LOAD( "b_11b.rom",    0x00000, 0x8000, CRC(eb6f278c) SHA1(15e250aa98ee69ac3983d4511976c35833b37cab) ) /* sprites */
	ROM_LOAD( "b_13b.rom",    0x08000, 0x8000, CRC(e91b4014) SHA1(6557344ce8bc05309ab8ebe846871ed554b256b8) )
	ROM_LOAD( "b_11a.rom",    0x10000, 0x8000, CRC(2822c522) SHA1(00b3cab899e5ac1af6300f2ec2a54303df9ab014) )
	ROM_LOAD( "b_13a.rom",    0x18000, 0x8000, CRC(3e8a9f75) SHA1(b1bfb7604791950aa0454b68b24f6ad3b9131be8) )
	ROM_LOAD( "b_12b.rom",    0x20000, 0x8000, CRC(86e43eda) SHA1(c33b0ab6f7f0f886410a3943988b737d175635be) )
	ROM_LOAD( "b_14b.rom",    0x28000, 0x8000, CRC(076e92d1) SHA1(27144834b5b2849be8c46e97aaaeaa8b304ea810) )
	ROM_LOAD( "b_12a.rom",    0x30000, 0x8000, CRC(ce107f3c) SHA1(2235281449247cb2446b008b36077788c5b15026) )
	ROM_LOAD( "b_14a.rom",    0x38000, 0x8000, CRC(dba06076) SHA1(87b3b3437bc4bd727ce7e34dd914e6fe23bcac3d) )

	ROM_REGION( 0x08000, "gfx4", 0 )    /* background tilemaps */
	ROM_LOAD( "b_03d.rom",    0x0000, 0x8000, CRC(6f348008) SHA1(b500bc32ba47e9cc9dcf2254b9455ac4d61992db) )

	ROM_REGION( 0x0320, "proms", 0 )
	ROM_LOAD( "63s141.16h",   0x0000, 0x0100, CRC(75af3553) SHA1(14da009592877a6097b34ea844fa897ceda7465e) )    // timing
	ROM_LOAD( "63s141.11h",   0x0100, 0x0100, CRC(a6e4d68f) SHA1(b9367e0c959cdf0397d33a49d778a66a407572b7) )    // color mixing
	ROM_LOAD( "63s141.15h",   0x0200, 0x0100, CRC(c47c182a) SHA1(47d6139256e6838f633a04084bd0a7a84912f7fb) )    // timing
	ROM_LOAD( "63s081.3j",    0x0300, 0x0020, CRC(c5817816) SHA1(cc642daafa0bcb160ee04e74e2d168fd44087608) )    // unknown
ROM_END

ROM_START( sidearmsu )
	ROM_REGION( 0x20000, "maincpu", 0 )     /* 64k for code + banked ROMs images */
	ROM_LOAD( "SAA_03.15E",   0x00000, 0x08000, CRC(32ef2739) SHA1(15e0535a6e3508c0d1ed73157a052c3716571000) )        /* CODE */
	ROM_LOAD( "a_14e.rom",    0x08000, 0x08000, CRC(4925ed03) SHA1(b11dbd9889db89cff008ca21beb6b1b70d983e16) )        /* 0+1 */
	ROM_LOAD( "a_12e.rom",    0x10000, 0x08000, CRC(81d0ece7) SHA1(5c1d154f9c1de6b5f5d7abf5d413e9c493461e6f) )        /* 2+3 */

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "a_04k.rom",    0x0000, 0x8000, CRC(34efe2d2) SHA1(e1d8895c113e4dee1a132e2471d75dfa6c36b620) )

	ROM_REGION( 0x08000, "user1", 0 )    /* starfield data */
	ROM_LOAD( "b_11j.rom",    0x0000, 0x8000, CRC(134dc35b) SHA1(6360c1efa7c4e1d6d817a97ca43dd4af8ed6afe5) )

	ROM_REGION( 0x04000, "gfx1", 0 )
	ROM_LOAD( "a_10j.rom",    0x00000, 0x4000, CRC(651fef75) SHA1(9c821a2ee30c222987f0d4192133776490d6a4e0) ) /* characters */

	ROM_REGION( 0x40000, "gfx2", 0 )
	ROM_LOAD( "b_13d.rom",    0x00000, 0x8000, CRC(3c59afe1) SHA1(5459a5795cf13012674993aa55bbd39e9a5c2f1b) ) /* tiles */
	ROM_LOAD( "b_13e.rom",    0x08000, 0x8000, CRC(64bc3b77) SHA1(54fe6f258fda509a92eb0f5aa238102efce729e0) )
	ROM_LOAD( "b_13f.rom",    0x10000, 0x8000, CRC(e6bcea6f) SHA1(19477e284967beafc4e7cd0d0da3534eb6dec388) )
	ROM_LOAD( "b_13g.rom",    0x18000, 0x8000, CRC(c71a3053) SHA1(963e105aa0b0174e8aa5e1f7676c5c604ca72d1c) )
	ROM_LOAD( "b_14d.rom",    0x20000, 0x8000, CRC(826e8a97) SHA1(ad5ed9a81805dde54fb2703345b2ab7b56853ec6) )
	ROM_LOAD( "b_14e.rom",    0x28000, 0x8000, CRC(6cfc02a4) SHA1(491e880e85d5256fa2eea6d0fb402f0a1176b675) )
	ROM_LOAD( "b_14f.rom",    0x30000, 0x8000, CRC(9b9f6730) SHA1(0f8fe5dc32ee50ebb2051c0c0c4d635582416317) )
	ROM_LOAD( "b_14g.rom",    0x38000, 0x8000, CRC(ef6af630) SHA1(499b17eeb5e7256ede477510b0547df520316996) )

	ROM_REGION( 0x40000, "gfx3", 0 )
	ROM_LOAD( "b_11b.rom",    0x00000, 0x8000, CRC(eb6f278c) SHA1(15e250aa98ee69ac3983d4511976c35833b37cab) ) /* sprites */
	ROM_LOAD( "b_13b.rom",    0x08000, 0x8000, CRC(e91b4014) SHA1(6557344ce8bc05309ab8ebe846871ed554b256b8) )
	ROM_LOAD( "b_11a.rom",    0x10000, 0x8000, CRC(2822c522) SHA1(00b3cab899e5ac1af6300f2ec2a54303df9ab014) )
	ROM_LOAD( "b_13a.rom",    0x18000, 0x8000, CRC(3e8a9f75) SHA1(b1bfb7604791950aa0454b68b24f6ad3b9131be8) )
	ROM_LOAD( "b_12b.rom",    0x20000, 0x8000, CRC(86e43eda) SHA1(c33b0ab6f7f0f886410a3943988b737d175635be) )
	ROM_LOAD( "b_14b.rom",    0x28000, 0x8000, CRC(076e92d1) SHA1(27144834b5b2849be8c46e97aaaeaa8b304ea810) )
	ROM_LOAD( "b_12a.rom",    0x30000, 0x8000, CRC(ce107f3c) SHA1(2235281449247cb2446b008b36077788c5b15026) )
	ROM_LOAD( "b_14a.rom",    0x38000, 0x8000, CRC(dba06076) SHA1(87b3b3437bc4bd727ce7e34dd914e6fe23bcac3d) )

	ROM_REGION( 0x08000, "gfx4", 0 )    /* background tilemaps */
	ROM_LOAD( "b_03d.rom",    0x0000, 0x8000, CRC(6f348008) SHA1(b500bc32ba47e9cc9dcf2254b9455ac4d61992db) )

	ROM_REGION( 0x0320, "proms", 0 )
	ROM_LOAD( "63s141.16h",   0x0000, 0x0100, CRC(75af3553) SHA1(14da009592877a6097b34ea844fa897ceda7465e) )    // timing
	ROM_LOAD( "63s141.11h",   0x0100, 0x0100, CRC(a6e4d68f) SHA1(b9367e0c959cdf0397d33a49d778a66a407572b7) )    // color mixing
	ROM_LOAD( "63s141.15h",   0x0200, 0x0100, CRC(c47c182a) SHA1(47d6139256e6838f633a04084bd0a7a84912f7fb) )    // timing
	ROM_LOAD( "63s081.3j",    0x0300, 0x0020, CRC(c5817816) SHA1(cc642daafa0bcb160ee04e74e2d168fd44087608) )    // unknown
ROM_END

ROM_START( sidearmsur1 )
	ROM_REGION( 0x20000, "maincpu", 0 )     /* 64k for code + banked ROMs images */
	ROM_LOAD( "03",           0x00000, 0x08000, CRC(9a799c45) SHA1(cf6836108506929ee2449546a4867a7cbf00bcc8) )        /* CODE */
	ROM_LOAD( "a_14e.rom",    0x08000, 0x08000, CRC(4925ed03) SHA1(b11dbd9889db89cff008ca21beb6b1b70d983e16) )        /* 0+1 */
	ROM_LOAD( "a_12e.rom",    0x10000, 0x08000, CRC(81d0ece7) SHA1(5c1d154f9c1de6b5f5d7abf5d413e9c493461e6f) )        /* 2+3 */

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "a_04k.rom",    0x0000, 0x8000, CRC(34efe2d2) SHA1(e1d8895c113e4dee1a132e2471d75dfa6c36b620) )

	ROM_REGION( 0x08000, "user1", 0 )    /* starfield data */
	ROM_LOAD( "b_11j.rom",    0x0000, 0x8000, CRC(134dc35b) SHA1(6360c1efa7c4e1d6d817a97ca43dd4af8ed6afe5) )

	ROM_REGION( 0x04000, "gfx1", 0 )
	ROM_LOAD( "a_10j.rom",    0x00000, 0x4000, CRC(651fef75) SHA1(9c821a2ee30c222987f0d4192133776490d6a4e0) ) /* characters */

	ROM_REGION( 0x40000, "gfx2", 0 )
	ROM_LOAD( "b_13d.rom",    0x00000, 0x8000, CRC(3c59afe1) SHA1(5459a5795cf13012674993aa55bbd39e9a5c2f1b) ) /* tiles */
	ROM_LOAD( "b_13e.rom",    0x08000, 0x8000, CRC(64bc3b77) SHA1(54fe6f258fda509a92eb0f5aa238102efce729e0) )
	ROM_LOAD( "b_13f.rom",    0x10000, 0x8000, CRC(e6bcea6f) SHA1(19477e284967beafc4e7cd0d0da3534eb6dec388) )
	ROM_LOAD( "b_13g.rom",    0x18000, 0x8000, CRC(c71a3053) SHA1(963e105aa0b0174e8aa5e1f7676c5c604ca72d1c) )
	ROM_LOAD( "b_14d.rom",    0x20000, 0x8000, CRC(826e8a97) SHA1(ad5ed9a81805dde54fb2703345b2ab7b56853ec6) )
	ROM_LOAD( "b_14e.rom",    0x28000, 0x8000, CRC(6cfc02a4) SHA1(491e880e85d5256fa2eea6d0fb402f0a1176b675) )
	ROM_LOAD( "b_14f.rom",    0x30000, 0x8000, CRC(9b9f6730) SHA1(0f8fe5dc32ee50ebb2051c0c0c4d635582416317) )
	ROM_LOAD( "b_14g.rom",    0x38000, 0x8000, CRC(ef6af630) SHA1(499b17eeb5e7256ede477510b0547df520316996) )

	ROM_REGION( 0x40000, "gfx3", 0 )
	ROM_LOAD( "b_11b.rom",    0x00000, 0x8000, CRC(eb6f278c) SHA1(15e250aa98ee69ac3983d4511976c35833b37cab) ) /* sprites */
	ROM_LOAD( "b_13b.rom",    0x08000, 0x8000, CRC(e91b4014) SHA1(6557344ce8bc05309ab8ebe846871ed554b256b8) )
	ROM_LOAD( "b_11a.rom",    0x10000, 0x8000, CRC(2822c522) SHA1(00b3cab899e5ac1af6300f2ec2a54303df9ab014) )
	ROM_LOAD( "b_13a.rom",    0x18000, 0x8000, CRC(3e8a9f75) SHA1(b1bfb7604791950aa0454b68b24f6ad3b9131be8) )
	ROM_LOAD( "b_12b.rom",    0x20000, 0x8000, CRC(86e43eda) SHA1(c33b0ab6f7f0f886410a3943988b737d175635be) )
	ROM_LOAD( "b_14b.rom",    0x28000, 0x8000, CRC(076e92d1) SHA1(27144834b5b2849be8c46e97aaaeaa8b304ea810) )
	ROM_LOAD( "b_12a.rom",    0x30000, 0x8000, CRC(ce107f3c) SHA1(2235281449247cb2446b008b36077788c5b15026) )
	ROM_LOAD( "b_14a.rom",    0x38000, 0x8000, CRC(dba06076) SHA1(87b3b3437bc4bd727ce7e34dd914e6fe23bcac3d) )

	ROM_REGION( 0x08000, "gfx4", 0 )    /* background tilemaps */
	ROM_LOAD( "b_03d.rom",    0x0000, 0x8000, CRC(6f348008) SHA1(b500bc32ba47e9cc9dcf2254b9455ac4d61992db) )

	ROM_REGION( 0x0320, "proms", 0 )
	ROM_LOAD( "63s141.16h",   0x0000, 0x0100, CRC(75af3553) SHA1(14da009592877a6097b34ea844fa897ceda7465e) )    // timing
	ROM_LOAD( "63s141.11h",   0x0100, 0x0100, CRC(a6e4d68f) SHA1(b9367e0c959cdf0397d33a49d778a66a407572b7) )    // color mixing
	ROM_LOAD( "63s141.15h",   0x0200, 0x0100, CRC(c47c182a) SHA1(47d6139256e6838f633a04084bd0a7a84912f7fb) )    // timing
	ROM_LOAD( "63s081.3j",    0x0300, 0x0020, CRC(c5817816) SHA1(cc642daafa0bcb160ee04e74e2d168fd44087608) )    // unknown
ROM_END

ROM_START( sidearmsj )
	ROM_REGION( 0x20000, "maincpu", 0 )     /* 64k for code + banked ROMs images */
	ROM_LOAD( "a_15e.rom",    0x00000, 0x08000, CRC(61ceb0cc) SHA1(bacf28e5e02b90a9d404c3ade0267e0a7cd73cd8) )        /* CODE */
	ROM_LOAD( "a_14e.rom",    0x08000, 0x08000, CRC(4925ed03) SHA1(b11dbd9889db89cff008ca21beb6b1b70d983e16) )        /* 0+1 */
	ROM_LOAD( "a_12e.rom",    0x10000, 0x08000, CRC(81d0ece7) SHA1(5c1d154f9c1de6b5f5d7abf5d413e9c493461e6f) )        /* 2+3 */

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "a_04k.rom",    0x0000, 0x8000, CRC(34efe2d2) SHA1(e1d8895c113e4dee1a132e2471d75dfa6c36b620) )

	ROM_REGION( 0x08000, "user1", 0 )    /* starfield data */
	ROM_LOAD( "b_11j.rom",    0x0000, 0x8000, CRC(134dc35b) SHA1(6360c1efa7c4e1d6d817a97ca43dd4af8ed6afe5) )

	ROM_REGION( 0x04000, "gfx1", 0 )
	ROM_LOAD( "a_10j.rom",    0x00000, 0x4000, CRC(651fef75) SHA1(9c821a2ee30c222987f0d4192133776490d6a4e0) ) /* characters */

	ROM_REGION( 0x40000, "gfx2", 0 )
	ROM_LOAD( "b_13d.rom",    0x00000, 0x8000, CRC(3c59afe1) SHA1(5459a5795cf13012674993aa55bbd39e9a5c2f1b) ) /* tiles */
	ROM_LOAD( "b_13e.rom",    0x08000, 0x8000, CRC(64bc3b77) SHA1(54fe6f258fda509a92eb0f5aa238102efce729e0) )
	ROM_LOAD( "b_13f.rom",    0x10000, 0x8000, CRC(e6bcea6f) SHA1(19477e284967beafc4e7cd0d0da3534eb6dec388) )
	ROM_LOAD( "b_13g.rom",    0x18000, 0x8000, CRC(c71a3053) SHA1(963e105aa0b0174e8aa5e1f7676c5c604ca72d1c) )
	ROM_LOAD( "b_14d.rom",    0x20000, 0x8000, CRC(826e8a97) SHA1(ad5ed9a81805dde54fb2703345b2ab7b56853ec6) )
	ROM_LOAD( "b_14e.rom",    0x28000, 0x8000, CRC(6cfc02a4) SHA1(491e880e85d5256fa2eea6d0fb402f0a1176b675) )
	ROM_LOAD( "b_14f.rom",    0x30000, 0x8000, CRC(9b9f6730) SHA1(0f8fe5dc32ee50ebb2051c0c0c4d635582416317) )
	ROM_LOAD( "b_14g.rom",    0x38000, 0x8000, CRC(ef6af630) SHA1(499b17eeb5e7256ede477510b0547df520316996) )

	ROM_REGION( 0x40000, "gfx3", 0 )
	ROM_LOAD( "b_11b.rom",    0x00000, 0x8000, CRC(eb6f278c) SHA1(15e250aa98ee69ac3983d4511976c35833b37cab) ) /* sprites */
	ROM_LOAD( "b_13b.rom",    0x08000, 0x8000, CRC(e91b4014) SHA1(6557344ce8bc05309ab8ebe846871ed554b256b8) )
	ROM_LOAD( "b_11a.rom",    0x10000, 0x8000, CRC(2822c522) SHA1(00b3cab899e5ac1af6300f2ec2a54303df9ab014) )
	ROM_LOAD( "b_13a.rom",    0x18000, 0x8000, CRC(3e8a9f75) SHA1(b1bfb7604791950aa0454b68b24f6ad3b9131be8) )
	ROM_LOAD( "b_12b.rom",    0x20000, 0x8000, CRC(86e43eda) SHA1(c33b0ab6f7f0f886410a3943988b737d175635be) )
	ROM_LOAD( "b_14b.rom",    0x28000, 0x8000, CRC(076e92d1) SHA1(27144834b5b2849be8c46e97aaaeaa8b304ea810) )
	ROM_LOAD( "b_12a.rom",    0x30000, 0x8000, CRC(ce107f3c) SHA1(2235281449247cb2446b008b36077788c5b15026) )
	ROM_LOAD( "b_14a.rom",    0x38000, 0x8000, CRC(dba06076) SHA1(87b3b3437bc4bd727ce7e34dd914e6fe23bcac3d) )

	ROM_REGION( 0x08000, "gfx4", 0 )    /* background tilemaps */
	ROM_LOAD( "b_03d.rom",    0x0000, 0x8000, CRC(6f348008) SHA1(b500bc32ba47e9cc9dcf2254b9455ac4d61992db) )

	ROM_REGION( 0x0320, "proms", 0 )
	ROM_LOAD( "63s141.16h",   0x0000, 0x0100, CRC(75af3553) SHA1(14da009592877a6097b34ea844fa897ceda7465e) )    // timing
	ROM_LOAD( "63s141.11h",   0x0100, 0x0100, CRC(a6e4d68f) SHA1(b9367e0c959cdf0397d33a49d778a66a407572b7) )    // color mixing
	ROM_LOAD( "63s141.15h",   0x0200, 0x0100, CRC(c47c182a) SHA1(47d6139256e6838f633a04084bd0a7a84912f7fb) )    // timing
	ROM_LOAD( "63s081.3j",    0x0300, 0x0020, CRC(c5817816) SHA1(cc642daafa0bcb160ee04e74e2d168fd44087608) )    // unknown
ROM_END

ROM_START( turtship )
	ROM_REGION( 0x20000, "maincpu", 0 )     /* 64k for code + banked ROMs images */
	ROM_LOAD( "t-3.bin",   0x00000, 0x08000, CRC(b73ed7f2) SHA1(bb98fe41b989d6568fe8cf1900a0d15c176b61a0) )
	ROM_LOAD( "t-2.3g",    0x08000, 0x08000, CRC(2327b35a) SHA1(bf7b5e11c3f75aff7d09c0fc4ad61fb4bcb38100) )
	ROM_LOAD( "t-1.bin",   0x10000, 0x08000, CRC(a258ffec) SHA1(caa689607ebe450a68736933dbfaf6bf9b6d3487) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "t-4.8a",    0x00000, 0x08000, CRC(1cbe48e8) SHA1(6ac5981d36a44595bb8dc847c54c7be7b374f82c) )

	ROM_REGION( 0x04000, "gfx1", 0 )
	ROM_LOAD( "t-5.8k",    0x00000, 0x04000, CRC(35c3dbc5) SHA1(6700c72e5e0f7bd1429d342cb5d3daccd6b1b70f) ) /* characters */
	ROM_CONTINUE(          0x00000, 0x04000 )   /* A14 tied high, only upper half is used */

	ROM_REGION( 0x80000, "gfx2", 0 )
	ROM_LOAD( "t-8.1d",    0x00000, 0x10000, CRC(30a857f0) SHA1(a2d261e8104d0459067bdbdd71662fe8d6917da1) ) /* tiles */
	ROM_LOAD( "t-10.3c",   0x10000, 0x10000, CRC(76bb73bb) SHA1(4c4acd205421674878948a0d2bed6032bde3f97f) )
	ROM_RELOAD( 0x30000,   0x10000)
	ROM_LOAD( "t-11.3d",   0x20000, 0x10000, CRC(53da6cb1) SHA1(52720746298adb01828f959f81b385d268c94343) )
	ROM_LOAD( "t-6.1a",    0x40000, 0x10000, CRC(45ce41ad) SHA1(6e2f559adc4aee80326b3ae5ae6c6688a3491962) )
	ROM_LOAD( "t-7.1c",    0x50000, 0x10000, CRC(3ccf11b9) SHA1(777cc853bfcf2db4027b35d516fa5bef8b010e63) )
	ROM_RELOAD( 0x70000,   0x10000)
	ROM_LOAD( "t-9.3a",    0x60000, 0x10000, CRC(44762916) SHA1(3427066fc02d1b9b71a59ac41d3332d5cd8d1423) )

	ROM_REGION( 0x40000, "gfx3", 0 )
	ROM_LOAD( "t-13.1i",   0x00000, 0x10000, CRC(599f5246) SHA1(b7e5bbff3b6117613744970c8680b7bc171516bd) ) /* sprites */
	ROM_LOAD( "t-15.bin",  0x10000, 0x10000, CRC(6489b7b4) SHA1(438d088db131f5bb4ef2124eee814b25c92115e3) )
	ROM_LOAD( "t-12.1g",   0x20000, 0x10000, CRC(fb54cd33) SHA1(49f7b728a4de8b93f5fd929f59a65509e4556161) )
	ROM_LOAD( "t-14.bin",  0x30000, 0x10000, CRC(1b67b674) SHA1(a77ef1b4ba4d544aa230acf779f9c339d0fc55db) )

	ROM_REGION( 0x08000, "gfx4", 0 )    /* background tilemaps */
	ROM_LOAD( "t-16.9f",   0x00000, 0x08000, CRC(1a5a45d7) SHA1(51ceeae938fbda207c3f8ce65593d271dc8c4a41) )
ROM_END

ROM_START( turtshipj )
	ROM_REGION( 0x20000, "maincpu", 0 )     /* 64k for code + banked ROMs images */
	ROM_LOAD( "t-3.5g",    0x00000, 0x08000, CRC(0863fc1c) SHA1(b583e06e05e466c2344a4a420a47227c9ab8705c) )
	ROM_LOAD( "t-2.3g",    0x08000, 0x08000, CRC(2327b35a) SHA1(bf7b5e11c3f75aff7d09c0fc4ad61fb4bcb38100) )
	ROM_LOAD( "t-1.3e",    0x10000, 0x08000, CRC(845a9ab0) SHA1(f1455aeca92d129c7ed145d76e5093f41ce62ccb) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "t-4.8a",    0x00000, 0x08000, CRC(1cbe48e8) SHA1(6ac5981d36a44595bb8dc847c54c7be7b374f82c) )

	ROM_REGION( 0x04000, "gfx1", 0 )
	ROM_LOAD( "t-5.8k",    0x00000, 0x04000, CRC(35c3dbc5) SHA1(6700c72e5e0f7bd1429d342cb5d3daccd6b1b70f) ) /* characters */
	ROM_CONTINUE(          0x00000, 0x04000 )   /* A14 tied high, only upper half is used */


	ROM_REGION( 0x80000, "gfx2", 0 )
	ROM_LOAD( "t-8.1d",    0x00000, 0x10000, CRC(30a857f0) SHA1(a2d261e8104d0459067bdbdd71662fe8d6917da1) ) /* tiles */
	ROM_LOAD( "t-10.3c",   0x10000, 0x10000, CRC(76bb73bb) SHA1(4c4acd205421674878948a0d2bed6032bde3f97f) )
	ROM_RELOAD( 0x30000,   0x10000)
	ROM_LOAD( "t-11.3d",   0x20000, 0x10000, CRC(53da6cb1) SHA1(52720746298adb01828f959f81b385d268c94343) )
	ROM_LOAD( "t-6.1a",    0x40000, 0x10000, CRC(45ce41ad) SHA1(6e2f559adc4aee80326b3ae5ae6c6688a3491962) )
	ROM_LOAD( "t-7.1c",    0x50000, 0x10000, CRC(3ccf11b9) SHA1(777cc853bfcf2db4027b35d516fa5bef8b010e63) )
	ROM_RELOAD( 0x70000,   0x10000)
	ROM_LOAD( "t-9.3a",    0x60000, 0x10000, CRC(44762916) SHA1(3427066fc02d1b9b71a59ac41d3332d5cd8d1423) )

	ROM_REGION( 0x40000, "gfx3", 0 )
	ROM_LOAD( "t-13.1i",   0x00000, 0x10000, CRC(599f5246) SHA1(b7e5bbff3b6117613744970c8680b7bc171516bd) ) /* sprites */
	ROM_LOAD( "t-15.3i",   0x10000, 0x10000, CRC(f30cfa90) SHA1(0e4ecea069df6a6bb6ec03eff51c0f37e7531aa8) )
	ROM_LOAD( "t-12.1g",   0x20000, 0x10000, CRC(fb54cd33) SHA1(49f7b728a4de8b93f5fd929f59a65509e4556161) )
	ROM_LOAD( "t-14.3g",   0x30000, 0x10000, CRC(d636873c) SHA1(6edf01d0bd6d085eda491c600b1f4b4cbede5a74) )

	ROM_REGION( 0x08000, "gfx4", 0 )    /* background tilemaps */
	ROM_LOAD( "t-16.9f",   0x00000, 0x08000, CRC(1a5a45d7) SHA1(51ceeae938fbda207c3f8ce65593d271dc8c4a41) )
ROM_END

ROM_START( turtshipk )
	ROM_REGION( 0x20000, "maincpu", 0 )     /* 64k for code + banked ROMs images */
	ROM_LOAD( "turtship.003",  0x00000, 0x08000, CRC(e7a7fc2e) SHA1(1a9147e82a5e56e8e5b68bbce144f96261e88669) )
	ROM_LOAD( "turtship.002",  0x08000, 0x08000, CRC(e576f482) SHA1(3be3792cb437bff0345681a3a2fdefefa3439357) )
	ROM_LOAD( "turtship.001",  0x10000, 0x08000, CRC(a9b64240) SHA1(38c59877de6055230c3250ef74abc97e4ed88cb6) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "t-4.8a",        0x00000, 0x08000, CRC(1cbe48e8) SHA1(6ac5981d36a44595bb8dc847c54c7be7b374f82c) )

	ROM_REGION( 0x04000, "gfx1", 0 ) /* Really a 27128? */
	ROM_LOAD( "turtship.005",  0x00000, 0x04000, CRC(651fef75) SHA1(9c821a2ee30c222987f0d4192133776490d6a4e0) ) /* characters */

	ROM_REGION( 0x80000, "gfx2", 0 )
	ROM_LOAD( "turtship.008",  0x00000, 0x10000, CRC(e0658469) SHA1(931c41cd6af759b30f6018248c3bab4d544acb98) ) /* tiles */
	ROM_LOAD( "t-10.3c",       0x10000, 0x10000, CRC(76bb73bb) SHA1(4c4acd205421674878948a0d2bed6032bde3f97f) )
	ROM_RELOAD( 0x30000,       0x10000)
	ROM_LOAD( "t-11.3d",       0x20000, 0x10000, CRC(53da6cb1) SHA1(52720746298adb01828f959f81b385d268c94343) )
	ROM_LOAD( "turtship.006",  0x40000, 0x10000, CRC(a7cce654) SHA1(f6c99622dcacc1d76021ca29b0bbceefbb75c499) )
	ROM_LOAD( "t-7.1c",        0x50000, 0x10000, CRC(3ccf11b9) SHA1(777cc853bfcf2db4027b35d516fa5bef8b010e63) )
	ROM_RELOAD( 0x70000,       0x10000)
	ROM_LOAD( "t-9.3a",        0x60000, 0x10000, CRC(44762916) SHA1(3427066fc02d1b9b71a59ac41d3332d5cd8d1423) )

	ROM_REGION( 0x40000, "gfx3", 0 )
	ROM_LOAD( "t-13.1i",       0x00000, 0x10000, CRC(599f5246) SHA1(b7e5bbff3b6117613744970c8680b7bc171516bd) ) /* sprites */
	ROM_LOAD( "turtship.015",  0x10000, 0x10000, CRC(69fd202f) SHA1(67d7d6d08f5daa0460ce51516f1d27dfd6aef297) )
	ROM_LOAD( "t-12.1g",       0x20000, 0x10000, CRC(fb54cd33) SHA1(49f7b728a4de8b93f5fd929f59a65509e4556161) )
	ROM_LOAD( "turtship.014",  0x30000, 0x10000, CRC(b3ea74a3) SHA1(aa347a6cd75408a3ba4ce26d3e1015a1be1faa64) )

	ROM_REGION( 0x08000, "gfx4", 0 )    /* background tilemaps */
	ROM_LOAD( "turtship.016",  0x00000, 0x08000, CRC(affd51dd) SHA1(3338aa1fdd6b9926acc215f7f3656d70803f1832) )
ROM_END

ROM_START( turtshipko )
		ROM_REGION( 0x20000, "maincpu", 0 )     /* 64k for code + banked ROMs images */
		ROM_LOAD( "T-3.G5",  0x00000, 0x08000, CRC(cd789535) SHA1(3c4f94c751645b61066177fbf3157924ad177c32) )
		ROM_LOAD( "T-2.G3",  0x08000, 0x08000, CRC(253678c0) SHA1(1470fd936003462d480c759658628ea085d4bd71) )
		ROM_LOAD( "T-1.E3",  0x10000, 0x08000, CRC(d6fdc376) SHA1(3f4e1fde8b83e3762f9499dfe291309efe940093) )

		ROM_REGION( 0x10000, "audiocpu", 0 )
		ROM_LOAD( "T-4.A8",        0x00000, 0x08000, CRC(1cbe48e8) SHA1(6ac5981d36a44595bb8dc847c54c7be7b374f82c) )

		ROM_REGION( 0x04000, "gfx1", 0 )
		ROM_LOAD( "T-5.K8",  0x00000, 0x04000, CRC(35c3dbc5) SHA1(6700c72e5e0f7bd1429d342cb5d3daccd6b1b70f) ) /* characters */
		ROM_CONTINUE(          0x00000, 0x04000 )   /* A14 tied high, only upper half is used */

		ROM_REGION( 0x80000, "gfx2", 0 )
		ROM_LOAD( "T-8.D1",  0x00000, 0x10000, CRC(2f0b2336) SHA1(a869e0a50aab7d29afbca46fa04bd470488a8eeb) ) /* tiles */
		ROM_LOAD( "T-10.C3",       0x10000, 0x10000, CRC(6a0072f4) SHA1(d74b53ed90a4d01020a179f263a39b7547b8f82e) )
		ROM_RELOAD( 0x30000,       0x10000)
		ROM_LOAD( "T-11.D3",       0x20000, 0x10000, CRC(53da6cb1) SHA1(52720746298adb01828f959f81b385d268c94343) )
		ROM_LOAD( "T-6.A1",  0x40000, 0x10000, CRC(a7cce654) SHA1(f6c99622dcacc1d76021ca29b0bbceefbb75c499) )
		ROM_LOAD( "T-7.C1",        0x50000, 0x10000, CRC(90dd8415) SHA1(8e9d43ff9164fb287ab82df7da8890976b9d21c7) )
		ROM_RELOAD( 0x70000,       0x10000)
		ROM_LOAD( "T-9.A3",        0x60000, 0x10000, CRC(44762916) SHA1(3427066fc02d1b9b71a59ac41d3332d5cd8d1423) )

		ROM_REGION( 0x40000, "gfx3", 0 )
		ROM_LOAD( "T-13.I1",       0x00000, 0x10000, CRC(1cc87f50) SHA1(d7d8a4376b556675dafa0a407bb34b6017f17e7d) ) /* sprites */
		ROM_LOAD( "T-15.I3",       0x10000, 0x10000, CRC(775ee5d9) SHA1(e39eb558cc2d5cdf4c87b96f85af72e5600b995e) )
		ROM_LOAD( "T-12.G1",       0x20000, 0x10000, CRC(57783312) SHA1(57942e8c3b7be63ea62bae3c104cb2842eb6b755) )
		ROM_LOAD( "T-14.G3",       0x30000, 0x10000, CRC(a30e3346) SHA1(150a837fb5d4705df9e8e9a94f78cff0e1c57d64) )

		ROM_REGION( 0x08000, "gfx4", 0 )    /* background tilemaps */
		ROM_LOAD( "T-16.F9",  0x00000, 0x08000, CRC(9b377277) SHA1(4858560e35144727aea958023f3df785baa994a8) )
ROM_END

ROM_START( turtshipkn )
		ROM_REGION( 0x20000, "maincpu", 0 )     /* 64k for code + banked ROMs images */
		ROM_LOAD( "T-3.G5",   0x00000, 0x08000, CRC(529b091c) SHA1(9a3a885dbf1f9d3c3c326418efdcb4f6f96eb4ae) )
		ROM_LOAD( "T-2.G3",    0x08000, 0x08000, CRC(d2f30195) SHA1(d64f088ed776658563943e8cde086842d0d899f8) )
		ROM_LOAD( "T-1.E3",   0x10000, 0x08000, CRC(2d02da90) SHA1(5cf059e04e145861f9877cefa2c7168e6ded19ac) )

		ROM_REGION( 0x10000, "audiocpu", 0 )
		ROM_LOAD( "T-4.A8",    0x00000, 0x08000, CRC(1cbe48e8) SHA1(6ac5981d36a44595bb8dc847c54c7be7b374f82c) )

		ROM_REGION( 0x04000, "gfx1", 0 )
		ROM_LOAD( "T-5.K8",    0x00000, 0x04000, CRC(5c2ee02d) SHA1(c8d3dbdaab943c1639795915cf275951501a2a77) ) /* characters */
		ROM_CONTINUE(          0x00000, 0x04000 )   /* A14 tied high, only upper half is used */

		ROM_REGION( 0x80000, "gfx2", 0 )
		ROM_LOAD( "T-8.D1",  0x00000, 0x10000, CRC(2f0b2336) SHA1(a869e0a50aab7d29afbca46fa04bd470488a8eeb) ) /* tiles */
		ROM_LOAD( "T-10.C3",       0x10000, 0x10000, CRC(6a0072f4) SHA1(d74b53ed90a4d01020a179f263a39b7547b8f82e) )
		ROM_RELOAD( 0x30000,       0x10000)
		ROM_LOAD( "T-11.D3",       0x20000, 0x10000, CRC(53da6cb1) SHA1(52720746298adb01828f959f81b385d268c94343) )
		ROM_LOAD( "T-6.A1",  0x40000, 0x10000, CRC(a7cce654) SHA1(f6c99622dcacc1d76021ca29b0bbceefbb75c499) )
		ROM_LOAD( "T-7.C1",        0x50000, 0x10000, CRC(90dd8415) SHA1(8e9d43ff9164fb287ab82df7da8890976b9d21c7) )
		ROM_RELOAD( 0x70000,       0x10000)
		ROM_LOAD( "T-9.A3",        0x60000, 0x10000, CRC(44762916) SHA1(3427066fc02d1b9b71a59ac41d3332d5cd8d1423) )

		ROM_REGION( 0x40000, "gfx3", 0 )
		ROM_LOAD( "T-13.I1",       0x00000, 0x10000, CRC(1cc87f50) SHA1(d7d8a4376b556675dafa0a407bb34b6017f17e7d) ) /* sprites */
		ROM_LOAD( "T-15.I3",       0x10000, 0x10000, CRC(3bf91fb8) SHA1(1c8368dc8d52c3c48a85391f00c91a80fa5d781d) )
		ROM_LOAD( "T-12.G1",       0x20000, 0x10000, CRC(57783312) SHA1(57942e8c3b7be63ea62bae3c104cb2842eb6b755) )
		ROM_LOAD( "T-14.G3",       0x30000, 0x10000, CRC(ee162dc0) SHA1(127b3cb3ddd47aa8ee70cad2d54b1306ad8f10e8) )

		ROM_REGION( 0x08000, "gfx4", 0 )    /* background tilemaps */
		ROM_LOAD( "T-16.F9",  0x00000, 0x08000, CRC(9b377277) SHA1(4858560e35144727aea958023f3df785baa994a8) )
ROM_END


ROM_START( dyger )
	ROM_REGION( 0x20000, "maincpu", 0 )     /* 64k for code + banked ROMs images */
	ROM_LOAD( "d-3.5g",  0x00000, 0x08000, CRC(bae9882e) SHA1(88194e58673ebd0841e9e07482842f6dbb823afc) )
	ROM_LOAD( "d-2.3g",  0x08000, 0x08000, CRC(059ac4dc) SHA1(fe46d819946e168b4a8188302737fdde957743ea) )
	ROM_LOAD( "d-1.3e",  0x10000, 0x08000, CRC(d8440f66) SHA1(3b2ee8c09d40edbe76d5004ed9074add0d4e4fd0) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "d-4.8a",  0x0000, 0x8000, CRC(8a256c09) SHA1(2c692af62da7c12b7d4f3f79264ee045a2cfa39f) )

	ROM_REGION( 0x04000, "gfx1", 0 )
	ROM_LOAD( "d-5.8k",  0x00000, 0x04000, CRC(c4bc72a5) SHA1(ee4ac5cbc9e97dd6fd0c9f507ee22a3eb36ba1b2) )   /* characters */
	ROM_CONTINUE(        0x00000, 0x04000 ) /* is the first half used? */

	ROM_REGION( 0x80000, "gfx2", 0 )
	ROM_LOAD( "d-10.1d", 0x00000, 0x10000, CRC(9715880d) SHA1(a6a400a0f4a80f3d151851a8ed182a6695a468b7) )   /* tiles */
	ROM_LOAD( "d-9.3c",  0x10000, 0x10000, CRC(628dae72) SHA1(5cfd5b87f702650afaf0999a45670f956b8254b2) )
	ROM_RELOAD( 0x30000, 0x10000)
	ROM_LOAD( "d-11.3d", 0x20000, 0x10000, CRC(23248db1) SHA1(47c5ef86e74be142faa0b896749d964ea1adc958) )
	ROM_LOAD( "d-6.1a",  0x40000, 0x10000, CRC(4ba7a437) SHA1(14bd939e3c5c28c5c7379e57832a0d3d707984f7) )
	ROM_LOAD( "d-8.1c",  0x50000, 0x10000, CRC(6c0f0e0c) SHA1(aac2b31346ebc6f2fb664faca732cd3738efcbab) )
	ROM_RELOAD( 0x70000, 0x10000)
	ROM_LOAD( "d-7.3a",  0x60000, 0x10000, CRC(2c50a229) SHA1(14498a06ec7c683c161f46633b270548ca8a9b85) )

	ROM_REGION( 0x40000, "gfx3", 0 )
	ROM_LOAD( "d-14.1i", 0x00000, 0x10000, CRC(99c60b26) SHA1(bcd56df5ef93c6133b61bce6472a708e340fbaaf) )   /* sprites */
	ROM_LOAD( "d-15.3i", 0x10000, 0x10000, CRC(d6475ecc) SHA1(61f6a9b443810742a2d39e61d14b92924cc27da7) )
	ROM_LOAD( "d-12.1g", 0x20000, 0x10000, CRC(e345705f) SHA1(0c51c0c598c0f51268108c7351b1b24977ae2b9f) )
	ROM_LOAD( "d-13.3g", 0x30000, 0x10000, CRC(faf4be3a) SHA1(dcf1958a17b587845174374f9598d0a979d7a6d5) )

	ROM_REGION( 0x08000, "gfx4", 0 )    /* background tilemaps */
	ROM_LOAD( "d-16.9f", 0x0000, 0x8000, CRC(0792e8f2) SHA1(3716839502679ecc973571d824065b40771d5bfa) )
ROM_END

ROM_START( dygera )
	ROM_REGION( 0x20000, "maincpu", 0 )     /* 64k for code + banked ROMs images */
	ROM_LOAD( "d-3.bin", 0x00000, 0x08000, CRC(fc63da8b) SHA1(f324a314cda167ae05e2eb017da355709489a7a3) )
	ROM_LOAD( "d-2.3g",  0x08000, 0x08000, CRC(059ac4dc) SHA1(fe46d819946e168b4a8188302737fdde957743ea) )
	ROM_LOAD( "d-1.3e",  0x10000, 0x08000, CRC(d8440f66) SHA1(3b2ee8c09d40edbe76d5004ed9074add0d4e4fd0) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "d-4.8a",  0x0000, 0x8000, CRC(8a256c09) SHA1(2c692af62da7c12b7d4f3f79264ee045a2cfa39f) )

	ROM_REGION( 0x04000, "gfx1", 0 )
	ROM_LOAD( "d-5.8k",  0x00000, 0x04000, CRC(c4bc72a5) SHA1(ee4ac5cbc9e97dd6fd0c9f507ee22a3eb36ba1b2) )   /* characters */
	ROM_CONTINUE(        0x00000, 0x04000 ) /* is the first half used? */

	ROM_REGION( 0x80000, "gfx2", 0 )
	ROM_LOAD( "d-10.1d", 0x00000, 0x10000, CRC(9715880d) SHA1(a6a400a0f4a80f3d151851a8ed182a6695a468b7) )   /* tiles */
	ROM_LOAD( "d-9.3c",  0x10000, 0x10000, CRC(628dae72) SHA1(5cfd5b87f702650afaf0999a45670f956b8254b2) )
	ROM_RELOAD( 0x30000, 0x10000)
	ROM_LOAD( "d-11.3d", 0x20000, 0x10000, CRC(23248db1) SHA1(47c5ef86e74be142faa0b896749d964ea1adc958) )
	ROM_LOAD( "d-6.1a",  0x40000, 0x10000, CRC(4ba7a437) SHA1(14bd939e3c5c28c5c7379e57832a0d3d707984f7) )
	ROM_LOAD( "d-8.1c",  0x50000, 0x10000, CRC(6c0f0e0c) SHA1(aac2b31346ebc6f2fb664faca732cd3738efcbab) )
	ROM_RELOAD( 0x70000, 0x10000)
	ROM_LOAD( "d-7.3a",  0x60000, 0x10000, CRC(2c50a229) SHA1(14498a06ec7c683c161f46633b270548ca8a9b85) )

	ROM_REGION( 0x40000, "gfx3", 0 )
	ROM_LOAD( "d-14.1i", 0x00000, 0x10000, CRC(99c60b26) SHA1(bcd56df5ef93c6133b61bce6472a708e340fbaaf) )   /* sprites */
	ROM_LOAD( "d-15.3i", 0x10000, 0x10000, CRC(d6475ecc) SHA1(61f6a9b443810742a2d39e61d14b92924cc27da7) )
	ROM_LOAD( "d-12.1g", 0x20000, 0x10000, CRC(e345705f) SHA1(0c51c0c598c0f51268108c7351b1b24977ae2b9f) )
	ROM_LOAD( "d-13.3g", 0x30000, 0x10000, CRC(faf4be3a) SHA1(dcf1958a17b587845174374f9598d0a979d7a6d5) )

	ROM_REGION( 0x08000, "gfx4", 0 )    /* background tilemaps */
	ROM_LOAD( "d-16.9f", 0x0000, 0x8000, CRC(0792e8f2) SHA1(3716839502679ecc973571d824065b40771d5bfa) )
ROM_END

ROM_START( twinfalc )   /* Shows "Notice  This game is for use in Korea only..." The real PCB displays the same :-) */
	ROM_REGION( 0x20000, "maincpu", 0 )     /* 64k for code + banked ROMs images */
	ROM_LOAD( "t-15.bin",    0x00000, 0x08000, CRC(e1f20144) SHA1(911781232fc1a7d6e36abb1c45e68a4398d8deac) )
	ROM_LOAD( "t-14.bin",    0x08000, 0x10000, CRC(c499ff83) SHA1(d99bb8cb04485638c5f05584cffdd2fbbe061af7) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "t-1.b4",     0x0000, 0x8000, CRC(b84bc980) SHA1(d2d302a96a9e3197f27144e525a901cfb9da09e4) )

	ROM_REGION( 0x8000, "gfx1", 0 )
	ROM_LOAD( "t-6.r6",     0x04000, 0x04000, CRC(8e4ca776) SHA1(412a47f030e3b491e23e5696ef88d065f9de0220) ) /* characters */
	ROM_CONTINUE(           0x00000, 0x04000 )  /* is the first half used? */

	ROM_REGION( 0x80000, "gfx2", 0 )
	ROM_LOAD( "t-10.y10",    0x00000, 0x10000, CRC(b678ef5b) SHA1(cdddd2a033291585e25839e864e898ef36f4d287) )
	ROM_LOAD( "t-9.w10",     0x10000, 0x10000, CRC(d7345fb9) SHA1(9da907c2bcacc750426a2989bae3c3e5fcc3e3ab) )
	ROM_RELOAD( 0x30000,     0x10000)
	ROM_LOAD( "t-8.u10",     0x20000, 0x10000, CRC(41428dac) SHA1(16ae6c178b91e5cd859deb13176b7333f05c378a) )
	ROM_LOAD( "t-13.y11",    0x40000, 0x10000, CRC(0eba10bd) SHA1(e2504a5576c6af6c5bdb0263e1d3cb9ccabde3f8) )
	ROM_LOAD( "t-12.w11",    0x50000, 0x10000, CRC(c65050ce) SHA1(f90616aa4e1f80d8d7fccf5748f564cb7bc2d83a) )
	ROM_RELOAD( 0x70000,     0x10000)
	ROM_LOAD( "t-11.u11",    0x60000, 0x10000, CRC(51a2c65d) SHA1(a89f46d581d2907b7813454925ce690af007997d) )

	ROM_REGION( 0x40000, "gfx3", 0 )
	ROM_LOAD( "t-2.a5",    0x00000, 0x10000, CRC(9c106835) SHA1(7e032e65e78c380b5f03a4febd6dcd3f0bdb642b) ) /* sprites */
	ROM_LOAD( "t-3.b5",    0x10000, 0x10000, CRC(9b421ccf) SHA1(0365d48437da0f90c1c146da0605139a3da0b03b) )
	ROM_LOAD( "t-4.a7",    0x20000, 0x10000, CRC(3a1db986) SHA1(5435e891eebe5b95a5a97ee8743a8a10282e4d19) )
	ROM_LOAD( "t-5.b7",    0x30000, 0x10000, CRC(9bd22190) SHA1(7a571becde02ea4b64db4138f00408f312bf54c0) )

	ROM_REGION( 0x08000, "gfx4", 0 )    /* background tilemaps */
	ROM_LOAD( "t-7.y8",    0x0000, 0x8000, CRC(a8b5f750) SHA1(94eb7af3cb8bee87ce3d31260e3bde062ebbc8f0) )
ROM_END

ROM_START( whizz )  /* Whizz Philko 1989. Original pcb. Boardnumber: 01-90 / Serial: WZ-089-00845 */
	ROM_REGION( 0x20000, "maincpu", 0 )     /* 64k for code + banked ROMs images */
	ROM_LOAD( "t-15.l11",    0x00000, 0x08000, CRC(73161302) SHA1(de815bba66c376cea775139f4285de0b1a589d88) )
	ROM_LOAD( "t-14.k11",    0x08000, 0x10000, CRC(bf248879) SHA1(f46f15e3949221e59d8c37de9c23473a74c2927e) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "t-1.b4",     0x0000, 0x8000, CRC(b84bc980) SHA1(d2d302a96a9e3197f27144e525a901cfb9da09e4) )

	ROM_REGION( 0x8000, "gfx1", 0 )
	ROM_LOAD( "t-6.r6",     0x04000, 0x04000, CRC(8e4ca776) SHA1(412a47f030e3b491e23e5696ef88d065f9de0220) ) /* characters */
	ROM_CONTINUE(           0x00000, 0x04000 )  /* is the first half used? */

	ROM_REGION( 0x80000, "gfx2", 0 )
	ROM_LOAD( "t-10.y10",    0x00000, 0x10000, CRC(b678ef5b) SHA1(cdddd2a033291585e25839e864e898ef36f4d287) )
	ROM_LOAD( "t-9.w10",     0x10000, 0x10000, CRC(d7345fb9) SHA1(9da907c2bcacc750426a2989bae3c3e5fcc3e3ab) )
	ROM_RELOAD( 0x30000,     0x10000)
	ROM_LOAD( "t-8.u10",     0x20000, 0x10000, CRC(41428dac) SHA1(16ae6c178b91e5cd859deb13176b7333f05c378a) )
	ROM_LOAD( "t-13.y11",    0x40000, 0x10000, CRC(0eba10bd) SHA1(e2504a5576c6af6c5bdb0263e1d3cb9ccabde3f8) )
	ROM_LOAD( "t-12.w11",    0x50000, 0x10000, CRC(c65050ce) SHA1(f90616aa4e1f80d8d7fccf5748f564cb7bc2d83a) )
	ROM_RELOAD( 0x70000,     0x10000)
	ROM_LOAD( "t-11.u11",    0x60000, 0x10000, CRC(51a2c65d) SHA1(a89f46d581d2907b7813454925ce690af007997d) )

	ROM_REGION( 0x40000, "gfx3", 0 )
	ROM_LOAD( "t-2.a5",    0x00000, 0x10000, CRC(9c106835) SHA1(7e032e65e78c380b5f03a4febd6dcd3f0bdb642b) ) /* sprites */
	ROM_LOAD( "t-3.b5",    0x10000, 0x10000, CRC(9b421ccf) SHA1(0365d48437da0f90c1c146da0605139a3da0b03b) )
	ROM_LOAD( "t-4.a7",    0x20000, 0x10000, CRC(3a1db986) SHA1(5435e891eebe5b95a5a97ee8743a8a10282e4d19) )
	ROM_LOAD( "t-5.b7",    0x30000, 0x10000, CRC(9bd22190) SHA1(7a571becde02ea4b64db4138f00408f312bf54c0) )

	ROM_REGION( 0x08000, "gfx4", 0 )    /* background tilemaps */
	ROM_LOAD( "t-7.y8",    0x0000, 0x8000, CRC(a8b5f750) SHA1(94eb7af3cb8bee87ce3d31260e3bde062ebbc8f0) )
ROM_END

DRIVER_INIT_MEMBER(sidearms_state,sidearms)
{
	m_gameid = 0;
}

DRIVER_INIT_MEMBER(sidearms_state,turtship)
{
	m_gameid = 1;
}

DRIVER_INIT_MEMBER(sidearms_state,dyger)
{
	m_gameid = 2;
}

DRIVER_INIT_MEMBER(sidearms_state,whizz)
{
	m_gameid = 3;
}

// date string is at 0xaa2 in 'rom 03' it does not appear to be displayed

GAME( 1986, sidearms,   0,        sidearms, sidearms, sidearms_state, sidearms, ROT0,   "Capcom",                   "Side Arms - Hyper Dyne (World, 861129)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )
GAME( 1986, sidearmsu,  sidearms, sidearms, sidearms, sidearms_state, sidearms, ROT0,   "Capcom (Romstar license)", "Side Arms - Hyper Dyne (US, 861202)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )
GAME( 1986, sidearmsur1,sidearms, sidearms, sidearms, sidearms_state, sidearms, ROT0,   "Capcom (Romstar license)", "Side Arms - Hyper Dyne (US, 861128)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )
GAME( 1986, sidearmsj,  sidearms, sidearms, sidearms, sidearms_state, sidearms, ROT0,   "Capcom",                   "Side Arms - Hyper Dyne (Japan, 861128)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )

GAME( 1988, turtship, 0,        turtship, turtship, sidearms_state, turtship, ROT0,   "Philko (Sharp Image license)",   "Turtle Ship (North America)", MACHINE_SUPPORTS_SAVE )
GAME( 1988, turtshipj,turtship, turtship, turtship, sidearms_state, turtship, ROT0,   "Philko (Pacific Games license)", "Turtle Ship (Japan)", MACHINE_SUPPORTS_SAVE )
GAME( 1988, turtshipk,turtship, turtship, turtship, sidearms_state, turtship, ROT0,   "Philko",                         "Turtle Ship (Korea)", MACHINE_SUPPORTS_SAVE )
GAME( 1988, turtshipko,turtship, turtship, turtship, sidearms_state, turtship, ROT0,   "Philko",                         "Turtle Ship (Korea, older)", MACHINE_SUPPORTS_SAVE )
GAME( 1988, turtshipkn, turtship, turtship, turtship, sidearms_state, turtship, ROT0,   "Philko",                       "Turtle Ship (Korea, 88/9)", MACHINE_SUPPORTS_SAVE )

GAME( 1989, dyger,    0,        turtship, dyger, sidearms_state,    dyger,    ROT270, "Philko", "Dyger (Korea set 1)", MACHINE_SUPPORTS_SAVE )
GAME( 1989, dygera,   dyger,    turtship, dyger, sidearms_state,    dyger,    ROT270, "Philko", "Dyger (Korea set 2)", MACHINE_SUPPORTS_SAVE )

GAME( 1989, twinfalc, 0,        whizz,    whizz, sidearms_state,    whizz,    ROT0,   "Philko (Poara Enterprises license)", "Twin Falcons", MACHINE_SUPPORTS_SAVE )
GAME( 1989, whizz,    twinfalc, whizz,    whizz, sidearms_state,    whizz,    ROT0,   "Philko",                             "Whizz", MACHINE_SUPPORTS_SAVE )
