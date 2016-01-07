// license:BSD-3-Clause
// copyright-holders:Phil Stroffolino
/***************************************************************************

Legend of Kage
(C)1985 Taito
CPU: Z80 (x2), MC68705
Sound: YM2203 (x2)

Phil Stroffolino
ptroffo@yahoo.com


Stephh's notes (based on the games Z80 code and some tests) :

1) 'lkage'

  - There is an ingame bug about the way difficulty is handled : if you look at code
    at 0xa05a, you'll notice that DSW3 bit 3 is tested as well as DSW3 bit 4.
    But DSW3 bit 4 also determines if coinage info shall be displayed (code at 0x1295) !
    In fact, DSW3 bit 4 is tested only when DSW3 bit 3 is ON ("Normal" difficulty),
    so you have 3 different "levels" of difficulty :

      bit 3   bit 4    Difficulty
       OFF     OFF       Easy
       OFF     ON        Easy
       ON      OFF       Normal
       ON      ON        Hard

    Flame length/color (and perhaps some other stuff) is also affected by DSW3 bit 3.
  - DSW3 bit 7 is supposed to determine how many coin slots there are (again, check
    the coingae display routine and code at 0x1295), but if you look at coinage insertion
    routines (0x091b for COIN1 and 0x0991 for COIN2), you'll notice that DSW3 bit 7
    is NOT tested !

2) 'lkageo'

  - Bugs are the same as in 'lkage'. Some routines addresses vary though :
      * difficulty : 0x9f50
      * coinage display : 0x128f
  - This set does more tests/things when TILT is pressed before jumping to 0x0000
    (see numerous instructions/calls at 0x0318 - most of them related to MCU).

3) 'lkageb*'

  - The difficulty bug is fixed : see code at 0x9e42 which reads DSW3 bits 2 and 3.
    So you have 4 different "levels" of difficulty :

      bit 2   bit 3    Difficulty
       OFF     OFF       Easy
       OFF     ON        Normal
       ON      OFF       Hard
       ON      ON        Hardest

    However DSW3 bit 3 isn't tested anywhere else, so flame length/color is constant.
  - The coin slots bug is not fixed as coinage insertion routines are unchanged
    (coinage display routine is now at 0x13f6).
  - These bootlegs are based on 'lkageo' (call to 0x0318 when TILT is pressed).
  - Hi-scores, scores, and as a consequence bonus lives, have been divided by 10,
    but it's only a cosmetical effect as data from 0xe200 to 0xe22f is unchanged.

4) 'bygone'

  - is it prototype ?
  - MCU (same code as lkage) is not used
  - No music ? (could be emulation bug) (the 2nd YM is not used...)

TODO:

  - The high score display uses a video attribute flag whose pupose isn't known.

  - purpose of the 0x200 byte prom, "a54-10.2" is unknown.  It contains values in range 0x0..0xf.

  - SOUND: lots of unknown writes to the YM2203 I/O ports

  - Note that all the bootlegs are derived from a different version of the
    original which hasn't been found yet.

  - lkage is verfied to be an original set, but it seems to work regardless of what
    the mcu does. Moreover, the mcu returns a checksum which is different from what
    is expected - the MCU computes 0x89, but the main CPU expects 0x5d.
    The game works anyway, it never gives the usual Taito "BAD HW" message
    (because there is no test at 0x033b after call at routine at 0xde1d).


***************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "cpu/m6805/m6805.h"
#include "sound/2203intf.h"
#include "includes/lkage.h"

#define MAIN_CPU_CLOCK      (XTAL_12MHz/2)
#define SOUND_CPU_CLOCK     (XTAL_8MHz/2)
#define AUDIO_CLOCK         (XTAL_8MHz/2)
#define MCU_CLOCK           (XTAL_12MHz/4)


TIMER_CALLBACK_MEMBER(lkage_state::nmi_callback)
{
	if (m_sound_nmi_enable)
		m_audiocpu->set_input_line(INPUT_LINE_NMI, PULSE_LINE);
	else
		m_pending_nmi = 1;
}

WRITE8_MEMBER(lkage_state::lkage_sound_command_w)
{
	soundlatch_byte_w(space, offset, data);
	machine().scheduler().synchronize(timer_expired_delegate(FUNC(lkage_state::nmi_callback),this), data);
}

WRITE8_MEMBER(lkage_state::lkage_sh_nmi_disable_w)
{
	m_sound_nmi_enable = 0;
}

WRITE8_MEMBER(lkage_state::lkage_sh_nmi_enable_w)
{
	m_sound_nmi_enable = 1;
	if (m_pending_nmi)
	{
		/* probably wrong but commands may go lost otherwise */
		m_audiocpu->set_input_line(INPUT_LINE_NMI, PULSE_LINE);
		m_pending_nmi = 0;
	}
}

READ8_MEMBER(lkage_state::sound_status_r)
{
	return 0xff;
}

static ADDRESS_MAP_START( lkage_map, AS_PROGRAM, 8, lkage_state )
	AM_RANGE(0x0000, 0xdfff) AM_ROM
	AM_RANGE(0xe000, 0xe7ff) AM_RAM /* work ram */
	AM_RANGE(0xe800, 0xefff) AM_RAM_DEVWRITE("palette", palette_device, write) AM_SHARE("palette")
	AM_RANGE(0xf000, 0xf003) AM_RAM AM_SHARE("vreg") /* video registers */
	AM_RANGE(0xf060, 0xf060) AM_WRITE(lkage_sound_command_w)
	AM_RANGE(0xf061, 0xf061) AM_WRITENOP AM_READ(sound_status_r)
	AM_RANGE(0xf062, 0xf062) AM_READWRITE(lkage_mcu_r,lkage_mcu_w)
	AM_RANGE(0xf063, 0xf063) AM_WRITENOP /* pulsed; nmi on sound cpu? */
	AM_RANGE(0xf080, 0xf080) AM_READ_PORT("DSW1")
	AM_RANGE(0xf081, 0xf081) AM_READ_PORT("DSW2")
	AM_RANGE(0xf082, 0xf082) AM_READ_PORT("DSW3")
	AM_RANGE(0xf083, 0xf083) AM_READ_PORT("SYSTEM")
	AM_RANGE(0xf084, 0xf084) AM_READ_PORT("P1")
	AM_RANGE(0xf086, 0xf086) AM_READ_PORT("P2")
	AM_RANGE(0xf087, 0xf087) AM_READ(lkage_mcu_status_r)
	AM_RANGE(0xf0a0, 0xf0a3) AM_RAM /* unknown */
	AM_RANGE(0xf0c0, 0xf0c5) AM_RAM AM_SHARE("scroll")
	AM_RANGE(0xf0e1, 0xf0e1) AM_WRITENOP /* pulsed */
	AM_RANGE(0xf100, 0xf15f) AM_RAM AM_SHARE("spriteram")
	AM_RANGE(0xf160, 0xf1ff) AM_RAM /* unknown - no valid sprite data */
	AM_RANGE(0xf400, 0xffff) AM_RAM_WRITE(lkage_videoram_w) AM_SHARE("videoram")
ADDRESS_MAP_END


READ8_MEMBER(lkage_state::port_fetch_r)
{
	return memregion("user1")->base()[offset];
}

static ADDRESS_MAP_START( lkage_io_map, AS_IO, 8, lkage_state )
	AM_RANGE(0x4000, 0x7fff) AM_READ(port_fetch_r)
ADDRESS_MAP_END

static ADDRESS_MAP_START( lkage_m68705_map, AS_PROGRAM, 8, lkage_state )
	ADDRESS_MAP_GLOBAL_MASK(0x7ff)
	AM_RANGE(0x0000, 0x0000) AM_READWRITE(lkage_68705_port_a_r,lkage_68705_port_a_w)
	AM_RANGE(0x0001, 0x0001) AM_READWRITE(lkage_68705_port_b_r,lkage_68705_port_b_w)
	AM_RANGE(0x0002, 0x0002) AM_READWRITE(lkage_68705_port_c_r,lkage_68705_port_c_w)
	AM_RANGE(0x0004, 0x0004) AM_WRITE(lkage_68705_ddr_a_w)
	AM_RANGE(0x0005, 0x0005) AM_WRITE(lkage_68705_ddr_b_w)
	AM_RANGE(0x0006, 0x0006) AM_WRITE(lkage_68705_ddr_c_w)
	AM_RANGE(0x0010, 0x007f) AM_RAM
	AM_RANGE(0x0080, 0x07ff) AM_ROM
ADDRESS_MAP_END

/***************************************************************************/

/* sound section is almost identical to Bubble Bobble, YM2203 instead of YM3526 */

static ADDRESS_MAP_START( lkage_sound_map, AS_PROGRAM, 8, lkage_state )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0x8000, 0x87ff) AM_RAM
	AM_RANGE(0x9000, 0x9001) AM_DEVREADWRITE("ym1", ym2203_device, read, write)
	AM_RANGE(0xa000, 0xa001) AM_DEVREADWRITE("ym2", ym2203_device, read, write)
	AM_RANGE(0xb000, 0xb000) AM_READ(soundlatch_byte_r) AM_WRITENOP /* ??? */
	AM_RANGE(0xb001, 0xb001) AM_READNOP /* ??? */ AM_WRITE(lkage_sh_nmi_enable_w)
	AM_RANGE(0xb002, 0xb002) AM_WRITE(lkage_sh_nmi_disable_w)
	AM_RANGE(0xb003, 0xb003) AM_WRITENOP
	AM_RANGE(0xe000, 0xefff) AM_ROM /* space for diagnostic ROM? */
ADDRESS_MAP_END

/***************************************************************************/

static INPUT_PORTS_START( lkage )
	PORT_START("DSW1")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Bonus_Life ) )       /* table at 0x04b8 */
	PORT_DIPSETTING(    0x03, "200k 700k 500k+" )
	PORT_DIPSETTING(    0x02, "200k 900k 700k+" )
	PORT_DIPSETTING(    0x01, "300k 1000k 700k+" )
	PORT_DIPSETTING(    0x00, "300k 1300k 1000k+" )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Free_Play ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x18, 0x18, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x18, "3" )
	PORT_DIPSETTING(    0x10, "4" )
	PORT_DIPSETTING(    0x08, "5" )
	PORT_DIPSETTING(    0x00, "255 (Cheat)")
	PORT_DIPUNUSED( 0x20, IP_ACTIVE_LOW )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Cocktail ) )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x0f, 0x00, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(    0x0f, DEF_STR( 9C_1C ) )
	PORT_DIPSETTING(    0x0e, DEF_STR( 8C_1C ) )
	PORT_DIPSETTING(    0x0d, DEF_STR( 7C_1C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 6C_1C ) )
	PORT_DIPSETTING(    0x0b, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x0a, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x09, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_7C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 1C_8C ) )
	PORT_DIPNAME( 0xf0, 0x00, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(    0xf0, DEF_STR( 9C_1C ) )
	PORT_DIPSETTING(    0xe0, DEF_STR( 8C_1C ) )
	PORT_DIPSETTING(    0xd0, DEF_STR( 7C_1C ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( 6C_1C ) )
	PORT_DIPSETTING(    0xb0, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0xa0, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x90, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x50, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(    0x60, DEF_STR( 1C_7C ) )
	PORT_DIPSETTING(    0x70, DEF_STR( 1C_8C ) )

	PORT_START("DSW3")
	PORT_DIPUNUSED( 0x01, IP_ACTIVE_LOW )
	PORT_DIPNAME( 0x02, 0x02, "Initial Season" )
	PORT_DIPSETTING(    0x02, "Spring" )
	PORT_DIPSETTING(    0x00, "Winter" )                    /* same as if you saved the princess twice ("HOWEVER ...") */
	PORT_DIPUNUSED( 0x04, IP_ACTIVE_LOW )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Difficulty ) )       /* see notes */
	PORT_DIPSETTING(    0x08, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Normal ) )
	PORT_DIPNAME( 0x10, 0x10, "Coinage Display" )           /* see notes */
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x20, 0x20, "Year Display" )
	PORT_DIPSETTING(    0x00, "1985" )
	PORT_DIPSETTING(    0x20, "MCMLXXXIV" )                 /* 1984(!) */
	PORT_DIPNAME( 0x40, 0x40, "Invulnerability (Cheat)")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Coin Slots" )
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x80, "2" )

	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_SERVICE1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_TILT )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )  PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )  PORT_8WAY
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )    PORT_8WAY
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )  PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )  PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )    PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END

static INPUT_PORTS_START( lkageb )
	PORT_INCLUDE( lkage )

	PORT_MODIFY("DSW1")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Bonus_Life ) )       /* table at 0x04b8 */
	PORT_DIPSETTING(    0x03, "20k 70k 50k+" )
	PORT_DIPSETTING(    0x02, "20k 90k 70k+" )
	PORT_DIPSETTING(    0x01, "30k 100k 70k+" )
	PORT_DIPSETTING(    0x00, "30k 130k 100k+" )

	PORT_MODIFY("DSW3")
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Difficulty ) )       /* see notes */
	PORT_DIPSETTING(    0x0c, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
INPUT_PORTS_END


static INPUT_PORTS_START( bygone )
	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Free_Play ) ) //
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x18, 0x18, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x18, "3" )
	PORT_DIPSETTING(    0x10, "4" )
	PORT_DIPSETTING(    0x08, "5" )
	PORT_DIPSETTING(    0x00, "255 (Cheat)")
	PORT_DIPUNUSED( 0x20, IP_ACTIVE_LOW )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Cocktail ) )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x0f, 0x00, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(    0x0f, DEF_STR( 9C_1C ) )
	PORT_DIPSETTING(    0x0e, DEF_STR( 8C_1C ) )
	PORT_DIPSETTING(    0x0d, DEF_STR( 7C_1C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 6C_1C ) )
	PORT_DIPSETTING(    0x0b, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x0a, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x09, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_7C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 1C_8C ) )
	PORT_DIPNAME( 0xf0, 0x00, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(    0xf0, DEF_STR( 9C_1C ) )
	PORT_DIPSETTING(    0xe0, DEF_STR( 8C_1C ) )
	PORT_DIPSETTING(    0xd0, DEF_STR( 7C_1C ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( 6C_1C ) )
	PORT_DIPSETTING(    0xb0, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0xa0, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x90, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x50, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(    0x60, DEF_STR( 1C_7C ) )
	PORT_DIPSETTING(    0x70, DEF_STR( 1C_8C ) )

	PORT_START("DSW3")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "Invulnerability (Cheat)")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )


	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_SERVICE1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_TILT )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )  PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )  PORT_8WAY
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )    PORT_8WAY
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )  PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )  PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )    PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END



static const gfx_layout tile_layout =
{
	8,8,
	RGN_FRAC(1,4),
	4,
	{ RGN_FRAC(1,4),RGN_FRAC(0,4),RGN_FRAC(3,4),RGN_FRAC(2,4) },
	{ 7, 6, 5, 4, 3, 2, 1, 0 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8
};

static const gfx_layout sprite_layout =
{
	16,16,
	RGN_FRAC(1,4),
	4,
	{ RGN_FRAC(1,4),RGN_FRAC(0,4),RGN_FRAC(3,4),RGN_FRAC(2,4) },
	{ 7, 6, 5, 4, 3, 2, 1, 0,
			64+7, 64+6, 64+5, 64+4, 64+3, 64+2, 64+1, 64+0 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8,
			128+0*8, 128+1*8, 128+2*8, 128+3*8, 128+4*8, 128+5*8, 128+6*8, 128+7*8 },
	32*8
};

static GFXDECODE_START( lkage )
	GFXDECODE_ENTRY( "gfx1", 0x0000, tile_layout,  /*128*/0, 64 )
	GFXDECODE_ENTRY( "gfx1", 0x0000, sprite_layout,  0, 16 )
GFXDECODE_END

WRITE_LINE_MEMBER(lkage_state::irqhandler)
{
	m_audiocpu->set_input_line(0, state ? ASSERT_LINE : CLEAR_LINE);
}

void lkage_state::machine_start()
{
	save_item(NAME(m_bg_tile_bank));
	save_item(NAME(m_fg_tile_bank));
	save_item(NAME(m_tx_tile_bank));

	save_item(NAME(m_sprite_dx));

	save_item(NAME(m_mcu_ready));
	save_item(NAME(m_mcu_val));
	save_item(NAME(m_sound_nmi_enable));
	save_item(NAME(m_pending_nmi));

	save_item(NAME(m_port_a_in));
	save_item(NAME(m_port_a_out));
	save_item(NAME(m_ddr_a));
	save_item(NAME(m_port_b_in));
	save_item(NAME(m_port_b_out));
	save_item(NAME(m_ddr_b));
	save_item(NAME(m_port_c_in));
	save_item(NAME(m_port_c_out));
	save_item(NAME(m_ddr_c));
	save_item(NAME(m_mcu_sent));
	save_item(NAME(m_main_sent));
	save_item(NAME(m_from_main));
	save_item(NAME(m_from_mcu));
}

void lkage_state::machine_reset()
{
	m_bg_tile_bank = m_fg_tile_bank = m_tx_tile_bank =0;

	m_mcu_ready = 3;
	m_mcu_val = 0;
	m_sound_nmi_enable = 0;
	m_pending_nmi = 0;

	m_port_a_in = 0;
	m_port_a_out = 0;
	m_ddr_a = 0;
	m_port_b_in = 0;
	m_port_b_out = 0;
	m_ddr_b = 0;
	m_port_c_in = 0;
	m_port_c_out = 0;
	m_ddr_c = 0;
	m_mcu_sent = 0;
	m_main_sent = 0;
	m_from_main = 0;
	m_from_mcu = 0;
}

static MACHINE_CONFIG_START( lkage, lkage_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, MAIN_CPU_CLOCK)
	MCFG_CPU_PROGRAM_MAP(lkage_map)
	MCFG_CPU_IO_MAP(lkage_io_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", lkage_state,  irq0_line_hold)

	MCFG_CPU_ADD("audiocpu", Z80, SOUND_CPU_CLOCK)
	MCFG_CPU_PROGRAM_MAP(lkage_sound_map)
								/* IRQs are triggered by the YM2203 */

	MCFG_CPU_ADD("mcu", M68705,MCU_CLOCK)
	MCFG_CPU_PROGRAM_MAP(lkage_m68705_map)


	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500) /* not accurate */)
	MCFG_SCREEN_SIZE(32*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(2*8, 32*8-1, 2*8, 30*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(lkage_state, screen_update_lkage)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", lkage)
	MCFG_PALETTE_ADD("palette", 1024)
	MCFG_PALETTE_FORMAT(xxxxRRRRGGGGBBBB)


	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("ym1", YM2203, AUDIO_CLOCK )
	MCFG_YM2203_IRQ_HANDLER(WRITELINE(lkage_state, irqhandler))
	MCFG_SOUND_ROUTE(0, "mono", 0.15)
	MCFG_SOUND_ROUTE(1, "mono", 0.15)
	MCFG_SOUND_ROUTE(2, "mono", 0.15)
	MCFG_SOUND_ROUTE(3, "mono", 0.40)

	MCFG_SOUND_ADD("ym2", YM2203, AUDIO_CLOCK )
	MCFG_SOUND_ROUTE(0, "mono", 0.15)
	MCFG_SOUND_ROUTE(1, "mono", 0.15)
	MCFG_SOUND_ROUTE(2, "mono", 0.15)
	MCFG_SOUND_ROUTE(3, "mono", 0.40)
MACHINE_CONFIG_END


static MACHINE_CONFIG_START( lkageb, lkage_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80,MAIN_CPU_CLOCK)
	MCFG_CPU_PROGRAM_MAP(lkage_map)
	MCFG_CPU_IO_MAP(lkage_io_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", lkage_state,  irq0_line_hold)

	MCFG_CPU_ADD("audiocpu", Z80, SOUND_CPU_CLOCK)
	MCFG_CPU_PROGRAM_MAP(lkage_sound_map)
								/* IRQs are triggered by the YM2203 */


	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500) /* not accurate */)
	MCFG_SCREEN_SIZE(32*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(2*8, 32*8-1, 2*8, 30*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(lkage_state, screen_update_lkage)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", lkage)
	MCFG_PALETTE_ADD("palette", 1024)
	MCFG_PALETTE_FORMAT(xxxxRRRRGGGGBBBB)


	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("ym1", YM2203, AUDIO_CLOCK)
	MCFG_YM2203_IRQ_HANDLER(WRITELINE(lkage_state, irqhandler))
	MCFG_SOUND_ROUTE(0, "mono", 0.15)
	MCFG_SOUND_ROUTE(1, "mono", 0.15)
	MCFG_SOUND_ROUTE(2, "mono", 0.15)
	MCFG_SOUND_ROUTE(3, "mono", 0.40)

	MCFG_SOUND_ADD("ym2", YM2203, AUDIO_CLOCK)
	MCFG_SOUND_ROUTE(0, "mono", 0.15)
	MCFG_SOUND_ROUTE(1, "mono", 0.15)
	MCFG_SOUND_ROUTE(2, "mono", 0.15)
	MCFG_SOUND_ROUTE(3, "mono", 0.40)
MACHINE_CONFIG_END

ROM_START( lkage )
	ROM_REGION( 0x14000, "maincpu", 0 ) /* Z80 code (main CPU) */
	ROM_LOAD( "a54-01-2.37", 0x0000, 0x8000, CRC(60fd9734) SHA1(33b444b887d80acb3a63ca4534db65c4d8147712) )
	ROM_LOAD( "a54-02-2.38", 0x8000, 0x8000, CRC(878a25ce) SHA1(6228a12774e116e333c3563ee6e20c0c70db514b) )

	ROM_REGION( 0x10000, "audiocpu", 0 ) /* Z80 code (sound CPU) */
	ROM_LOAD( "a54-04.54",   0x0000, 0x8000, CRC(541faf9a) SHA1(b142ff3bd198f700697ec06ea92db3109ab5818e) )

	ROM_REGION( 0x10000, "mcu", 0 ) /* 68705 MCU code */
	ROM_LOAD( "a54-09.53",   0x0000, 0x0800, CRC(0e8b8846) SHA1(a4a105462b0127229bb7edfadd2e581c7e40f1cc) )

	ROM_REGION( 0x4000, "user1", 0 ) /* data */
	ROM_LOAD( "a54-03.51",   0x0000, 0x4000, CRC(493e76d8) SHA1(13c6160edd94ba2801fd89bb33bcae3a1e3454ff) )

	ROM_REGION( 0x10000, "gfx1", 0 )
	ROM_LOAD( "a54-05-1.84", 0x0000, 0x4000, CRC(0033c06a) SHA1(89964503fc338817c6511fd15942741996b7037a) )
	ROM_LOAD( "a54-06-1.85", 0x4000, 0x4000, CRC(9f04d9ad) SHA1(3b9a4d30348fd02e5c8ae94655548bd4a02dd65d) )
	ROM_LOAD( "a54-07-1.86", 0x8000, 0x4000, CRC(b20561a4) SHA1(0d6d83dfae79ea133e37704ca47426b4c978fb36) )
	ROM_LOAD( "a54-08-1.87", 0xc000, 0x4000, CRC(3ff3b230) SHA1(ffcd964efb0af32b5d7a70305dfda615ea95acbe) )

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "a54-10.2",    0x0000, 0x0200, CRC(17dfbd14) SHA1(f8f0b6dfedd4ba108dad43ccc7697ef4ab9cbf86) ) /* unknown */

	ROM_REGION( 0x0800, "plds", 0 )
	ROM_LOAD( "pal16l8-a54-11.34",  0x0000, 0x0104, CRC(56232113) SHA1(4cdc6732aa3e7fbe8df51966a1295253711ecc8f) )
	ROM_LOAD( "pal16l8-a54-12.76",  0x0200, 0x0104, CRC(e57c3c89) SHA1(a23f91da254055bb990e8bb730564c40b5725f78) )
	ROM_LOAD( "pal16l8a-a54-13.27", 0x0400, 0x0104, CRC(c9b1938e) SHA1(2fd1adc4bde8f07cf4b6314d56b48bb3d7144cc3) )
	ROM_LOAD( "pal16l8a-a54-14.35", 0x0600, 0x0104, CRC(a89c644e) SHA1(b41a077d1d070d9563f924c776930c33a4ff27d0) )
ROM_END

ROM_START( lkageo )
	ROM_REGION( 0x14000, "maincpu", 0 ) /* Z80 code (main CPU) */
	ROM_LOAD( "a54-01-1.37", 0x0000, 0x8000, CRC(973da9c5) SHA1(ad3b5d6a329b784e47be563c6f8dc628f32ba0a5) )
	ROM_LOAD( "a54-02-1.38", 0x8000, 0x8000, CRC(27b509da) SHA1(c623950bd7dd2b5699ca948e3731455964106b89) )

	ROM_REGION( 0x10000, "audiocpu", 0 ) /* Z80 code (sound CPU) */
	ROM_LOAD( "a54-04.54",   0x0000, 0x8000, CRC(541faf9a) SHA1(b142ff3bd198f700697ec06ea92db3109ab5818e) )

	ROM_REGION( 0x10000, "mcu", 0 ) /* 68705 MCU code */
	ROM_LOAD( "a54-09.53",   0x0000, 0x0800, CRC(0e8b8846) SHA1(a4a105462b0127229bb7edfadd2e581c7e40f1cc) )

	ROM_REGION( 0x4000, "user1", 0 ) /* data */
	ROM_LOAD( "a54-03.51",   0x0000, 0x4000, CRC(493e76d8) SHA1(13c6160edd94ba2801fd89bb33bcae3a1e3454ff) )

	ROM_REGION( 0x10000, "gfx1", 0 )
	ROM_LOAD( "a54-05-1.84", 0x0000, 0x4000, CRC(0033c06a) SHA1(89964503fc338817c6511fd15942741996b7037a) )
	ROM_LOAD( "a54-06-1.85", 0x4000, 0x4000, CRC(9f04d9ad) SHA1(3b9a4d30348fd02e5c8ae94655548bd4a02dd65d) )
	ROM_LOAD( "a54-07-1.86", 0x8000, 0x4000, CRC(b20561a4) SHA1(0d6d83dfae79ea133e37704ca47426b4c978fb36) )
	ROM_LOAD( "a54-08-1.87", 0xc000, 0x4000, CRC(3ff3b230) SHA1(ffcd964efb0af32b5d7a70305dfda615ea95acbe) )

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "a54-10.2",    0x0000, 0x0200, CRC(17dfbd14) SHA1(f8f0b6dfedd4ba108dad43ccc7697ef4ab9cbf86) ) /* unknown */

	ROM_REGION( 0x0800, "plds", 0 )
	ROM_LOAD( "pal16l8-a54-11.34",  0x0000, 0x0104, CRC(56232113) SHA1(4cdc6732aa3e7fbe8df51966a1295253711ecc8f) )
	ROM_LOAD( "pal16l8-a54-12.76",  0x0200, 0x0104, CRC(e57c3c89) SHA1(a23f91da254055bb990e8bb730564c40b5725f78) )
	ROM_LOAD( "pal16l8a-a54-13.27", 0x0400, 0x0104, CRC(c9b1938e) SHA1(2fd1adc4bde8f07cf4b6314d56b48bb3d7144cc3) )
	ROM_LOAD( "pal16l8a-a54-14.35", 0x0600, 0x0104, CRC(a89c644e) SHA1(b41a077d1d070d9563f924c776930c33a4ff27d0) )
ROM_END

ROM_START( lkageoo )
	ROM_REGION( 0x14000, "maincpu", 0 ) /* Z80 code (main CPU) */
	ROM_LOAD( "a54-01.37", 0x0000, 0x8000, CRC(34eab2c5) SHA1(25bf2dc80d21aa68c3af5debf10b24c75d83a738) )
	ROM_LOAD( "a54-02.38", 0x8000, 0x8000, CRC(ea471d8a) SHA1(1ffc7f78e3e983e16a23e97019f7030f9846569b) )

	ROM_REGION( 0x10000, "audiocpu", 0 ) /* Z80 code (sound CPU) */
	ROM_LOAD( "a54-04.54",   0x0000, 0x8000, CRC(541faf9a) SHA1(b142ff3bd198f700697ec06ea92db3109ab5818e) )

	ROM_REGION( 0x10000, "mcu", 0 ) /* 68705 MCU code */
	ROM_LOAD( "a54-09.53",   0x0000, 0x0800, CRC(0e8b8846) SHA1(a4a105462b0127229bb7edfadd2e581c7e40f1cc) )

	ROM_REGION( 0x4000, "user1", 0 ) /* data */
	ROM_LOAD( "a54-03.51",   0x0000, 0x4000, CRC(493e76d8) SHA1(13c6160edd94ba2801fd89bb33bcae3a1e3454ff) )

	ROM_REGION( 0x10000, "gfx1", 0 )
	ROM_LOAD( "a54-05.84", 0x0000, 0x4000, CRC(76753e52) SHA1(13f61969d59b055a5ab40237148e091d7cabe190) )
	ROM_LOAD( "a54-06.85", 0x4000, 0x4000, CRC(f33c015c) SHA1(756326daab255d3a36d97e51ee141b9f7157f12e) )
	ROM_LOAD( "a54-07.86", 0x8000, 0x4000, CRC(0e02c2e8) SHA1(1d8a817ba66cf26a4fe51ae00874c0fe6e7cebe3) )
	ROM_LOAD( "a54-08.87", 0xc000, 0x4000, CRC(4ef5f073) SHA1(dfd234542b28cff74692a1c381772da01e8bb4a7) )

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "a54-10.2",    0x0000, 0x0200, CRC(17dfbd14) SHA1(f8f0b6dfedd4ba108dad43ccc7697ef4ab9cbf86) ) /* unknown */

	ROM_REGION( 0x0800, "plds", 0 )
	ROM_LOAD( "pal16l8-a54-11.34",  0x0000, 0x0104, CRC(56232113) SHA1(4cdc6732aa3e7fbe8df51966a1295253711ecc8f) )
	ROM_LOAD( "pal16l8-a54-12.76",  0x0200, 0x0104, CRC(e57c3c89) SHA1(a23f91da254055bb990e8bb730564c40b5725f78) )
	ROM_LOAD( "pal16l8a-a54-13.27", 0x0400, 0x0104, CRC(c9b1938e) SHA1(2fd1adc4bde8f07cf4b6314d56b48bb3d7144cc3) )
	ROM_LOAD( "pal16l8a-a54-14.35", 0x0600, 0x0104, CRC(a89c644e) SHA1(b41a077d1d070d9563f924c776930c33a4ff27d0) )
ROM_END

ROM_START( lkageb )
	ROM_REGION( 0x10000, "maincpu", 0 ) /* Z80 code (main CPU) */
	ROM_LOAD( "ic37_1",      0x0000, 0x8000, CRC(05694f7b) SHA1(08a3796d6cf04d64db52ed8208a51084c420e10a) )
	ROM_LOAD( "ic38_2",      0x8000, 0x8000, CRC(22efe29e) SHA1(f7a29d54081ca7509e822ad8823ec977bccc4a40) )

	ROM_REGION( 0x10000, "audiocpu", 0 ) /* Z80 code (sound CPU) */
	ROM_LOAD( "a54-04.54",   0x0000, 0x8000, CRC(541faf9a) SHA1(b142ff3bd198f700697ec06ea92db3109ab5818e) )

	ROM_REGION( 0x4000, "user1", 0 ) /* data */
	ROM_LOAD( "a54-03.51",   0x0000, 0x4000, CRC(493e76d8) SHA1(13c6160edd94ba2801fd89bb33bcae3a1e3454ff) )

	ROM_REGION( 0x10000, "gfx1", 0 )
	ROM_LOAD( "ic93_5",      0x0000, 0x4000, CRC(76753e52) SHA1(13f61969d59b055a5ab40237148e091d7cabe190) )
	ROM_LOAD( "ic94_6",      0x4000, 0x4000, CRC(f33c015c) SHA1(756326daab255d3a36d97e51ee141b9f7157f12e) )
	ROM_LOAD( "ic95_7",      0x8000, 0x4000, CRC(0e02c2e8) SHA1(1d8a817ba66cf26a4fe51ae00874c0fe6e7cebe3) )
	ROM_LOAD( "ic96_8",      0xc000, 0x4000, CRC(4ef5f073) SHA1(dfd234542b28cff74692a1c381772da01e8bb4a7) )

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "a54-10.2",    0x0000, 0x0200, CRC(17dfbd14) SHA1(f8f0b6dfedd4ba108dad43ccc7697ef4ab9cbf86) ) /* unknown */
ROM_END

ROM_START( lkageb2 )
	ROM_REGION( 0x10000, "maincpu", 0 ) /* Z80 code (main CPU) */
	ROM_LOAD( "lok.a",       0x0000, 0x8000, CRC(866df793) SHA1(44a9a773d7bbfc5f9d53f56682438ef8b23ecbd6) )
	ROM_LOAD( "lok.b",       0x8000, 0x8000, CRC(fba9400f) SHA1(fedcb9b717feaeec31afda098f0ac2744df6c7be) )

	ROM_REGION( 0x10000, "audiocpu", 0 ) /* Z80 code (sound CPU) */
	ROM_LOAD( "a54-04.54",   0x0000, 0x8000, CRC(541faf9a) SHA1(b142ff3bd198f700697ec06ea92db3109ab5818e) )

	ROM_REGION( 0x4000, "user1", 0 ) /* data */
	ROM_LOAD( "a54-03.51",   0x0000, 0x4000, CRC(493e76d8) SHA1(13c6160edd94ba2801fd89bb33bcae3a1e3454ff) )

	ROM_REGION( 0x10000, "gfx1", 0 )
	ROM_LOAD( "ic93_5",      0x0000, 0x4000, CRC(76753e52) SHA1(13f61969d59b055a5ab40237148e091d7cabe190) )
	ROM_LOAD( "ic94_6",      0x4000, 0x4000, CRC(f33c015c) SHA1(756326daab255d3a36d97e51ee141b9f7157f12e) )
	ROM_LOAD( "ic95_7",      0x8000, 0x4000, CRC(0e02c2e8) SHA1(1d8a817ba66cf26a4fe51ae00874c0fe6e7cebe3) )
	ROM_LOAD( "ic96_8",      0xc000, 0x4000, CRC(4ef5f073) SHA1(dfd234542b28cff74692a1c381772da01e8bb4a7) )

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "a54-10.2",    0x0000, 0x0200, CRC(17dfbd14) SHA1(f8f0b6dfedd4ba108dad43ccc7697ef4ab9cbf86) ) /* unknown */
ROM_END

ROM_START( lkageb3 )
	ROM_REGION( 0x10000, "maincpu", 0 ) /* Z80 code (main CPU) */
	ROM_LOAD( "z1.bin",      0x0000, 0x8000, CRC(60cac488) SHA1(b61df14159f37143b1faed22d77fc7be31602022) )
	ROM_LOAD( "z2.bin",      0x8000, 0x8000, CRC(22c95f17) SHA1(8ca438d508a36918778651adf599cf45a7c4a5d7) )

	ROM_REGION( 0x10000, "audiocpu", 0 ) /* Z80 code (sound CPU) */
	ROM_LOAD( "a54-04.54",   0x0000, 0x8000, CRC(541faf9a) SHA1(b142ff3bd198f700697ec06ea92db3109ab5818e) )

	ROM_REGION( 0x4000, "user1", 0 ) /* data */
	ROM_LOAD( "a54-03.51",   0x0000, 0x4000, CRC(493e76d8) SHA1(13c6160edd94ba2801fd89bb33bcae3a1e3454ff) )

	ROM_REGION( 0x10000, "gfx1", 0 )
	ROM_LOAD( "ic93_5",      0x0000, 0x4000, CRC(76753e52) SHA1(13f61969d59b055a5ab40237148e091d7cabe190) )
	ROM_LOAD( "ic94_6",      0x4000, 0x4000, CRC(f33c015c) SHA1(756326daab255d3a36d97e51ee141b9f7157f12e) )
	ROM_LOAD( "ic95_7",      0x8000, 0x4000, CRC(0e02c2e8) SHA1(1d8a817ba66cf26a4fe51ae00874c0fe6e7cebe3) )
	ROM_LOAD( "ic96_8",      0xc000, 0x4000, CRC(4ef5f073) SHA1(dfd234542b28cff74692a1c381772da01e8bb4a7) )

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "a54-10.2",    0x0000, 0x0200, CRC(17dfbd14) SHA1(f8f0b6dfedd4ba108dad43ccc7697ef4ab9cbf86) ) /* unknown */
ROM_END

/*
Bygone
Taito, 1985?

This is a rare prototype platform game conversion on a Legend Of Kage PCB.
There are some wire mods on the video board.


PCB Layouts
-----------

K1100135A
J1100057A
CPU PCB
M4300040A (sticker, also matches The Legend Of Kage)
|------------------------------------------------------|
|        VOL  LM324   TL074                    PC040DA |
|             PC010SA YM3014    YM2203         PC040DA |
|     MB3731                                   PC040DA |
|             PC010SA YM3014    YM2203      MB2148     |
|    PC030CM                                MB2148     |
|                      8MHz                 MB2148     |
|          6116                                       |-|
|2                                                    | |
|2         A53_07.IC54   Z80A                         | |
|W                                                    | |
|A         A51_09.IC53                                | |
|Y                                                    | |
|                                                     | |
|                                                     | |
|                                     A54-14.IC27     |-|
|          A53_08.IC51     A53_06.IC38                 |
|                                                      |
|                                                      |
|                                                      |
|          6264            A53_05.IC37   Z80B          |
|             DSW3 DSW2 DSW1    TL7700                 |
|------------------------------------------------------|
Notes:
      Z80A   - clock 4.000MHz [8/2]
      Z80B   - clock 6.000MHz [12/2]
      YM2203 - clock 4.000MHz [8/2]
      A51_09 - MC68705P5 microcontroller, clock 3.000MHz [12/4].
               It seems to be from Taito game A51, which is unknown?
               It was not protected ^_^
      A54-14 - PAL16L8
      A53*   - 27C128 and 27C256 EPROMs
      6264   - 8k x8 SRAM
      6116   - 2k x8 SRAM
      2148   - 1k x4 SRAM
      DIPs have 8 switches each


K1100136A
J1100058A
VIDEO PCB
|------------------------------------------------------|
|    12MHz                                             |
|                                                      |
|                                                      |
|                                                      |
|                                            6116      |
|                         6116                         |
|                                            6116     |-|
|1                                                    | |
|8                                                    | |
|W                                                    | |
|A        A54-11.IC76                                 | |
|Y                                    93422  93422    | |
|                                                     | |
|                                     93422  93422    | |
|  A53_04.IC87  MB112S146 MB112S146  A54-13.IC35      |-|
|                                                      |
|  A53_03.IC86  MB112S146 MB112S146  A54-12.IC34       |
|                                                      |
|  A53_02.IC85  MB112S146 MB112S146                    |
|                                                      |
|  A53_01.IC84  MB112S146 MB112S146          A54-10.IC2|
|------------------------------------------------------|
Notes:
      A54-10 - MB7122 PROM
      A54-11 - PAL16L8
      A54-12 - PAL16L8
      A54-13 - PAL16L8
      6116   - 2k x8 SRAM
      93422  - 256b x4 SRAM
      A53*   - 27C128 EPROMs

*/

ROM_START( bygone )
	ROM_REGION( 0x14000, "maincpu", 0 ) /* Z80 code (main CPU) */
	ROM_LOAD( "a53_05.ic37", 0x0000, 0x8000, CRC(63a3f08b) SHA1(781539077cb1d3b8eecc8bd3717330c0f281833d) )
	ROM_LOAD( "a53_06.ic38", 0x8000, 0x8000, CRC(cb0dcb08) SHA1(6b12b018b983b8225b5f33fb9fcd8004a00fd8ff) )

	ROM_REGION( 0x10000, "audiocpu", 0 ) /* Z80 code (sound CPU) */
	ROM_LOAD( "a53_07.ic54",   0x0000, 0x8000, CRC(72f69a77) SHA1(dfc1050a4123b3c83ae733ece1b6fe2836beb901) )

	ROM_REGION( 0x10000, "mcu", 0 ) /* 68705 MCU code */
	ROM_LOAD( "a51_09.ic53",   0x0000, 0x0800, CRC(0e8b8846) SHA1(a4a105462b0127229bb7edfadd2e581c7e40f1cc) ) /* the same as lkage */

	ROM_REGION( 0x4000, "user1", 0 ) /* data */
	ROM_LOAD( "a53_08.ic51",   0x0000, 0x4000, CRC(f85139f9) SHA1(7e089d1dd5c5fa8abb396b44aa15aabcf8677940) )

	ROM_REGION( 0x10000, "gfx1", 0 )
	ROM_LOAD( "a53_01.ic84", 0x0000, 0x4000, CRC(38cf7fb2) SHA1(424efabe2386fa5f1c22444a53952d85c05c2d64) )
	ROM_LOAD( "a53_02.ic85", 0x4000, 0x4000, CRC(dca7adfe) SHA1(83b159e92dab96d9e20fa3a9d1ce7a7d2e83b313) )
	ROM_LOAD( "a53_03.ic86", 0x8000, 0x4000, CRC(af3eb997) SHA1(ba66ffb9d83f91c98446ac38bb0e712ec0800625) )
	ROM_LOAD( "a53_04.ic87", 0xc000, 0x4000, CRC(65af72d3) SHA1(759a1dd7548075630ddb9c692bdb32ad4712c579) )

	ROM_REGION( 0x0400, "proms", 0 )
	ROM_LOAD( "a54-10.ic2",    0x0000, 0x0400, CRC(369722d9) SHA1(2df9932ad8ce87c0a9d2c89222a4cec12c29046d) )   /* unknown */
ROM_END


/*Note: This probably uses another MCU dump,which is undumped.*/

READ8_MEMBER(lkage_state::fake_mcu_r)
{
	int result = 0;

	switch (m_mcu_val)
	{
		/*These are for the attract mode*/
		case 0x01:
			result = m_mcu_val - 1;
			break;

		case 0x90:
			result = m_mcu_val + 0x43;
			break;

		/*Gameplay Protection,checked in this order at a start of a play*/
		case 0xa6:
			result = m_mcu_val + 0x27;
			break;

		case 0x34:
			result = m_mcu_val + 0x7f;
			break;

		case 0x48:
			result = m_mcu_val + 0xb7;
			break;

		default:
			result = m_mcu_val;
			break;
	}
	return result;
}

WRITE8_MEMBER(lkage_state::fake_mcu_w)
{
	m_mcu_val = data;
}

READ8_MEMBER(lkage_state::fake_status_r)
{
	return m_mcu_ready;
}

DRIVER_INIT_MEMBER(lkage_state,lkage)
{
	m_sprite_dx=0;
}

DRIVER_INIT_MEMBER(lkage_state,lkageb)
{
	m_maincpu->space(AS_PROGRAM).install_read_handler(0xf062, 0xf062, read8_delegate(FUNC(lkage_state::fake_mcu_r),this));
	m_maincpu->space(AS_PROGRAM).install_read_handler(0xf087, 0xf087, read8_delegate(FUNC(lkage_state::fake_status_r),this));
	m_maincpu->space(AS_PROGRAM).install_write_handler(0xf062, 0xf062, write8_delegate(FUNC(lkage_state::fake_mcu_w),this));
	m_sprite_dx=0;
}

DRIVER_INIT_MEMBER(lkage_state,bygone)
{
	m_sprite_dx=1;
}

GAME( 1984, lkage,    0,        lkage,    lkage, lkage_state,    lkage,    ROT0, "Taito Corporation", "The Legend of Kage", MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE )
GAME( 1984, lkageo,   lkage,    lkage,    lkage, lkage_state,    lkage,    ROT0, "Taito Corporation", "The Legend of Kage (older)", MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE )
GAME( 1984, lkageoo,  lkage,    lkage,    lkage, lkage_state,    lkage,    ROT0, "Taito Corporation", "The Legend of Kage (oldest)", MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE )
GAME( 1984, lkageb,   lkage,    lkageb,   lkageb, lkage_state,   lkageb,   ROT0, "bootleg", "The Legend of Kage (bootleg set 1)", MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE )
GAME( 1984, lkageb2,  lkage,    lkageb,   lkageb, lkage_state,   lkageb,   ROT0, "bootleg", "The Legend of Kage (bootleg set 2)", MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE )
GAME( 1984, lkageb3,  lkage,    lkageb,   lkageb, lkage_state,   lkageb,   ROT0, "bootleg", "The Legend of Kage (bootleg set 3)", MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE )
GAME( 1985, bygone,   0,        lkage,    bygone, lkage_state,   bygone,   ROT0, "Taito Corporation", "Bygone (prototype)", MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS | MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE )
