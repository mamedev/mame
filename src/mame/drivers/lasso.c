// license:BSD-3-Clause
// copyright-holders:Phil Stroffolino, Nicola Salmoria, Luca Elia
/***************************************************************************

 Lasso and similar hardware

        driver by Phil Stroffolino, Nicola Salmoria, Luca Elia

--------------------------------------------------------------------------------------
Year + Game                 By              CPUs        Sound Chips         Misc Info
--------------------------------------------------------------------------------------
82  Lasso                   SNK             3 x 6502    2 x SN76489
83  Chameleon               Jaleco          2 x 6502    2 x SN76489
84  Wai Wai Jockey Gate-In! Jaleco/Casio    2 x 6502    2 x SN76489 + DAC
84  Pinbo                   Jaleco          6502 + Z80  2 x AY-8910         6502 @ 18MHz/24, Z80 @ 18MHz/6, AY @ 18MHz/12
--------------------------------------------------------------------------------------

Notes:

- unknown CPU speeds unless noted above (affect game timing), currently using
  same as the Rock-Ola games of the same area.  Lot of similarities between
  these hardware.  The music ends at the perfect time with this clock speed
- Lasso: fire button auto-repeats on high score entry screen (real behavior?)
- Pinbo: bgcolor is wrong, how does it generate it? ($a0, should be darkblue)

***************************************************************************

DIP locations verified for:
    - lasso

***************************************************************************/

#include "emu.h"
#include "cpu/m6502/m6502.h"
#include "cpu/z80/z80.h"
#include "sound/dac.h"
#include "sound/ay8910.h"
#include "includes/lasso.h"


INPUT_CHANGED_MEMBER(lasso_state::coin_inserted)
{
	/* coin insertion causes an NMI */
	m_maincpu->set_input_line(INPUT_LINE_NMI, newval ? CLEAR_LINE : ASSERT_LINE);
}


/* Write to the sound latch and generate an IRQ on the sound CPU */
WRITE8_MEMBER(lasso_state::sound_command_w)
{
	soundlatch_byte_w(space, offset, data);
	m_audiocpu->set_input_line(0, HOLD_LINE);
}

READ8_MEMBER(lasso_state::sound_status_r)
{
	/*  0x01: chip#0 ready; 0x02: chip#1 ready */
	return 0x03;
}

WRITE8_MEMBER(lasso_state::sound_select_w)
{
	UINT8 to_write = BITSWAP8(*m_chip_data, 0, 1, 2, 3, 4, 5, 6, 7);

	if (~data & 0x01)   /* chip #0 */
		m_sn_1->write(space, 0, to_write);

	if (~data & 0x02)   /* chip #1 */
		m_sn_2->write(space, 0, to_write);
}


static ADDRESS_MAP_START( lasso_main_map, AS_PROGRAM, 8, lasso_state )
	AM_RANGE(0x0000, 0x03ff) AM_RAM
	AM_RANGE(0x0400, 0x07ff) AM_RAM_WRITE(lasso_videoram_w) AM_SHARE("videoram")
	AM_RANGE(0x0800, 0x0bff) AM_RAM_WRITE(lasso_colorram_w) AM_SHARE("colorram")
	AM_RANGE(0x0c00, 0x0c7f) AM_RAM AM_SHARE("spriteram")
	AM_RANGE(0x1000, 0x17ff) AM_RAM AM_SHARE("share1")
	AM_RANGE(0x1800, 0x1800) AM_WRITE(sound_command_w)
	AM_RANGE(0x1801, 0x1801) AM_WRITEONLY AM_SHARE("back_color")
	AM_RANGE(0x1802, 0x1802) AM_WRITE(lasso_video_control_w)
	AM_RANGE(0x1804, 0x1804) AM_READ_PORT("1804")
	AM_RANGE(0x1805, 0x1805) AM_READ_PORT("1805")
	AM_RANGE(0x1806, 0x1806) AM_READ_PORT("1806") AM_WRITENOP   /* game uses 'lsr' to read port */
	AM_RANGE(0x1807, 0x1807) AM_READ_PORT("1807")
	AM_RANGE(0x8000, 0xbfff) AM_MIRROR(0x4000) AM_ROM
ADDRESS_MAP_END


static ADDRESS_MAP_START( lasso_audio_map, AS_PROGRAM, 8, lasso_state )
	AM_RANGE(0x0000, 0x01ff) AM_RAM
	AM_RANGE(0x5000, 0x7fff) AM_ROM
	AM_RANGE(0xb000, 0xb000) AM_WRITEONLY AM_SHARE("chip_data")
	AM_RANGE(0xb001, 0xb001) AM_WRITE(sound_select_w)
	AM_RANGE(0xb004, 0xb004) AM_READ(sound_status_r)
	AM_RANGE(0xb005, 0xb005) AM_READ(soundlatch_byte_r)
	AM_RANGE(0xf000, 0xffff) AM_ROM AM_REGION("audiocpu", 0x7000)
ADDRESS_MAP_END


static ADDRESS_MAP_START( lasso_coprocessor_map, AS_PROGRAM, 8, lasso_state )
	AM_RANGE(0x0000, 0x07ff) AM_RAM AM_SHARE("share1")
	AM_RANGE(0x2000, 0x3fff) AM_RAM AM_SHARE("bitmap_ram")
	AM_RANGE(0x8000, 0x8fff) AM_MIRROR(0x7000) AM_ROM
ADDRESS_MAP_END


static ADDRESS_MAP_START( chameleo_main_map, AS_PROGRAM, 8, lasso_state )
	AM_RANGE(0x0000, 0x03ff) AM_RAM
	AM_RANGE(0x0400, 0x07ff) AM_RAM_WRITE(lasso_videoram_w) AM_SHARE("videoram")
	AM_RANGE(0x0800, 0x0bff) AM_RAM_WRITE(lasso_colorram_w) AM_SHARE("colorram")
	AM_RANGE(0x0c00, 0x0fff) AM_RAM
	AM_RANGE(0x1000, 0x107f) AM_RAM AM_SHARE("spriteram")
	AM_RANGE(0x1080, 0x10ff) AM_RAM
	AM_RANGE(0x1800, 0x1800) AM_WRITE(sound_command_w)
	AM_RANGE(0x1801, 0x1801) AM_WRITEONLY AM_SHARE("back_color")
	AM_RANGE(0x1802, 0x1802) AM_WRITE(lasso_video_control_w)
	AM_RANGE(0x1804, 0x1804) AM_READ_PORT("1804")
	AM_RANGE(0x1805, 0x1805) AM_READ_PORT("1805")
	AM_RANGE(0x1806, 0x1806) AM_READ_PORT("1806")
	AM_RANGE(0x1807, 0x1807) AM_READ_PORT("1807")
	AM_RANGE(0x4000, 0xbfff) AM_ROM
	AM_RANGE(0xe000, 0xffff) AM_ROM AM_REGION("maincpu", 0xa000)
ADDRESS_MAP_END


static ADDRESS_MAP_START( chameleo_audio_map, AS_PROGRAM, 8, lasso_state )
	AM_RANGE(0x0000, 0x01ff) AM_RAM
	AM_RANGE(0x1000, 0x1fff) AM_ROM
	AM_RANGE(0x6000, 0x7fff) AM_ROM
	AM_RANGE(0xb000, 0xb000) AM_WRITEONLY AM_SHARE("chip_data")
	AM_RANGE(0xb001, 0xb001) AM_WRITE(sound_select_w)
	AM_RANGE(0xb004, 0xb004) AM_READ(sound_status_r)
	AM_RANGE(0xb005, 0xb005) AM_READ(soundlatch_byte_r)
	AM_RANGE(0xf000, 0xffff) AM_ROM AM_REGION("audiocpu", 0x7000)
ADDRESS_MAP_END


static ADDRESS_MAP_START( wwjgtin_main_map, AS_PROGRAM, 8, lasso_state )
	AM_RANGE(0x0000, 0x07ff) AM_RAM
	AM_RANGE(0x0800, 0x0bff) AM_RAM_WRITE(lasso_videoram_w) AM_SHARE("videoram")
	AM_RANGE(0x0c00, 0x0fff) AM_RAM_WRITE(lasso_colorram_w) AM_SHARE("colorram")
	AM_RANGE(0x1000, 0x10ff) AM_RAM AM_SHARE("spriteram")
	AM_RANGE(0x1800, 0x1800) AM_WRITE(sound_command_w)
	AM_RANGE(0x1801, 0x1801) AM_WRITEONLY AM_SHARE("back_color")
	AM_RANGE(0x1802, 0x1802) AM_WRITE(wwjgtin_video_control_w)
	AM_RANGE(0x1804, 0x1804) AM_READ_PORT("1804")
	AM_RANGE(0x1805, 0x1805) AM_READ_PORT("1805")
	AM_RANGE(0x1806, 0x1806) AM_READ_PORT("1806")
	AM_RANGE(0x1807, 0x1807) AM_READ_PORT("1807")
	AM_RANGE(0x1c00, 0x1c02) AM_WRITEONLY AM_SHARE("last_colors")
	AM_RANGE(0x1c04, 0x1c07) AM_WRITEONLY AM_SHARE("track_scroll")
	AM_RANGE(0x4000, 0xbfff) AM_ROM
	AM_RANGE(0xc000, 0xffff) AM_ROM AM_REGION("maincpu", 0x8000)
ADDRESS_MAP_END


static ADDRESS_MAP_START( wwjgtin_audio_map, AS_PROGRAM, 8, lasso_state )
	AM_RANGE(0x0000, 0x01ff) AM_RAM
	AM_RANGE(0x4000, 0x7fff) AM_MIRROR(0x8000) AM_ROM
	AM_RANGE(0xb000, 0xb000) AM_WRITEONLY AM_SHARE("chip_data")
	AM_RANGE(0xb001, 0xb001) AM_WRITE(sound_select_w)
	AM_RANGE(0xb003, 0xb003) AM_DEVWRITE("dac", dac_device, write_unsigned8)
	AM_RANGE(0xb004, 0xb004) AM_READ(sound_status_r)
	AM_RANGE(0xb005, 0xb005) AM_READ(soundlatch_byte_r)
ADDRESS_MAP_END


static ADDRESS_MAP_START( pinbo_main_map, AS_PROGRAM, 8, lasso_state )
	AM_RANGE(0x0000, 0x03ff) AM_RAM
	AM_RANGE(0x0400, 0x07ff) AM_RAM_WRITE(lasso_videoram_w) AM_SHARE("videoram")
	AM_RANGE(0x0800, 0x0bff) AM_RAM_WRITE(lasso_colorram_w) AM_SHARE("colorram")
	AM_RANGE(0x1000, 0x10ff) AM_RAM AM_SHARE("spriteram")
	AM_RANGE(0x1800, 0x1800) AM_WRITE(sound_command_w)
	AM_RANGE(0x1801, 0x1801) AM_WRITEONLY AM_SHARE("back_color")
	AM_RANGE(0x1802, 0x1802) AM_WRITE(pinbo_video_control_w)
	AM_RANGE(0x1804, 0x1804) AM_READ_PORT("1804")
	AM_RANGE(0x1805, 0x1805) AM_READ_PORT("1805")
	AM_RANGE(0x1806, 0x1806) AM_READ_PORT("1806")
	AM_RANGE(0x1807, 0x1807) AM_READ_PORT("1807")
	AM_RANGE(0x2000, 0x3fff) AM_ROM
	AM_RANGE(0x6000, 0xbfff) AM_ROM
	AM_RANGE(0xe000, 0xffff) AM_ROM AM_REGION("maincpu", 0xa000)
ADDRESS_MAP_END


static ADDRESS_MAP_START( pinbo_audio_map, AS_PROGRAM, 8, lasso_state )
	AM_RANGE(0x0000, 0x1fff) AM_ROM
	AM_RANGE(0xf000, 0xffff) AM_RAM
ADDRESS_MAP_END


static ADDRESS_MAP_START( pinbo_audio_io_map, AS_IO, 8, lasso_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x01) AM_DEVWRITE("ay1", ay8910_device, address_data_w)
	AM_RANGE(0x02, 0x02) AM_DEVREAD("ay1", ay8910_device, data_r)
	AM_RANGE(0x04, 0x05) AM_DEVWRITE("ay2", ay8910_device, address_data_w)
	AM_RANGE(0x06, 0x06) AM_DEVREAD("ay2", ay8910_device, data_r)
	AM_RANGE(0x08, 0x08) AM_READ(soundlatch_byte_r) AM_WRITENOP /* ??? */
	AM_RANGE(0x14, 0x14) AM_WRITENOP    /* ??? */
ADDRESS_MAP_END



static INPUT_PORTS_START( lasso )
	PORT_START("1804")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_4WAY
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_4WAY
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_4WAY
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_4WAY
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 ) /* lasso */
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON2 ) /* shoot */
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("1805")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_COCKTAIL
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("1806")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Cabinet ) )      PORT_DIPLOCATION("SW2:!1")
	PORT_DIPSETTING(    0x01, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x0e, 0x00, DEF_STR( Coin_A ) )       PORT_DIPLOCATION("SW2:!2,!3,!4")
	PORT_DIPSETTING(    0x02, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_6C ) )
//  PORT_DIPSETTING(    0x06, DEF_STR( 1C_1C ) )        /* Not documented */
//  PORT_DIPSETTING(    0x0a, DEF_STR( 1C_1C ) )        /* Not documented */
//  PORT_DIPSETTING(    0x0e, DEF_STR( 1C_1C ) )        /* Not documented */
	PORT_DIPNAME( 0x30, 0x00, DEF_STR( Lives ) )        PORT_DIPLOCATION("SW2:!5,!6")
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x10, "4" )
	PORT_DIPSETTING(    0x20, "5" )
//  PORT_DIPSETTING(    0x30, "3" )                     /* Not documented */
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Coin_B ) )       PORT_DIPLOCATION("SW1:!1")
	PORT_DIPSETTING(    0x40, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPNAME( 0x80, 0x80, "Warm-Up Instructions" )  PORT_DIPLOCATION("SW1:!4") /* Listed as "Unused" */
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Yes ) )

	PORT_START("1807")
	PORT_DIPNAME( 0x01, 0x00, "Warm-Up" )               PORT_DIPLOCATION("SW1:!3")
	PORT_DIPSETTING(    0x01, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x02, 0x00, "Warm-Up Language" )      PORT_DIPLOCATION("SW1:!2")
	PORT_DIPSETTING(    0x00, DEF_STR( English ) )
	PORT_DIPSETTING(    0x02, DEF_STR( German ) )
	PORT_DIPUNUSED_DIPLOC( 0x04, 0x00, "SW1:!5" )       /* Listed as "Unused" */
	PORT_DIPNAME( 0x08, 0x00, "Invulnerability (Cheat)") PORT_DIPLOCATION("SW1:!6") /* Listed as "Test" */
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_CHANGED_MEMBER(DEVICE_SELF, lasso_state,coin_inserted, 0)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_CHANGED_MEMBER(DEVICE_SELF, lasso_state,coin_inserted, 0)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_START2  )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_START1  )
INPUT_PORTS_END

static INPUT_PORTS_START( chameleo )
	PORT_INCLUDE( lasso )

	PORT_MODIFY("1804")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_MODIFY("1805")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_MODIFY("1806")
	PORT_DIPNAME( 0x30, 0x00, DEF_STR( Lives ) )        PORT_DIPLOCATION("SW2:!5,!6")
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x30, "5" )
//  PORT_DIPSETTING(    0x10, "5" )
	PORT_DIPSETTING(    0x20, "Infinite (Cheat)")
	PORT_DIPUNKNOWN_DIPLOC( 0x80, 0x00, "SW1:!4" )

	PORT_MODIFY("1807")
	PORT_DIPUNKNOWN_DIPLOC( 0x01, 0x00, "SW1:!3" )      /* Probably unused */
	PORT_DIPUNKNOWN_DIPLOC( 0x02, 0x00, "SW1:!2" )      /* Probably unused */
	PORT_DIPUNKNOWN_DIPLOC( 0x04, 0x00, "SW1:!5" )      /* Probably unused */
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SW1:!6")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( wwjgtin )
	PORT_INCLUDE( lasso )

	PORT_MODIFY("1805")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_4WAY PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_4WAY PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_4WAY PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_4WAY PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(2)

	PORT_MODIFY("1806")
	PORT_DIPUNKNOWN_DIPLOC( 0x01, 0x00, "SW2:!1" )      /* used - has to do with the controls */
	PORT_DIPUNKNOWN_DIPLOC( 0x10, 0x00, "SW2:!5" )      /* probably unused */
	PORT_DIPUNKNOWN_DIPLOC( 0x20, 0x00, "SW2:!6" )      /* probably unused */
	PORT_DIPUNKNOWN_DIPLOC( 0x80, 0x00, "SW1:!4" )      /* probably unused */

	PORT_MODIFY("1807")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Bonus_Life) )    PORT_DIPLOCATION("SW1:!3")
	PORT_DIPSETTING(    0x00, "20k" )
	PORT_DIPSETTING(    0x01, "50k" )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SW1:!2")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPUNKNOWN_DIPLOC( 0x04, 0x00, "SW1:!5" )      /* probably unused */
	PORT_DIPUNKNOWN_DIPLOC( 0x08, 0x00, "SW1:!6" )      /* probably unused */
	PORT_BIT( 0x10, IP_ACTIVE_LOW,  IPT_COIN2 ) PORT_CHANGED_MEMBER(DEVICE_SELF, lasso_state,coin_inserted, 0)
	PORT_BIT( 0x20, IP_ACTIVE_LOW,  IPT_COIN1 ) PORT_CHANGED_MEMBER(DEVICE_SELF, lasso_state,coin_inserted, 0)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_START2 )
INPUT_PORTS_END

static INPUT_PORTS_START( pinbo )
	PORT_INCLUDE( lasso )

	PORT_MODIFY("1804")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON2 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_MODIFY("1805")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_MODIFY("1806")
	PORT_DIPNAME( 0x30, 0x00, DEF_STR( Lives ) )        PORT_DIPLOCATION("SW2:!5,!6")
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x10, "4" )
	PORT_DIPSETTING(    0x20, "5" )
	PORT_DIPSETTING(    0x30, "70 (Cheat)")
	PORT_DIPUNKNOWN_DIPLOC( 0x80, 0x00, "SW1:!4" )      /* probably unused */

	PORT_MODIFY("1807")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Bonus_Life) )    PORT_DIPLOCATION("SW1:!3")
	PORT_DIPSETTING(    0x00, "500000, 1000000" )
	PORT_DIPSETTING(    0x01, DEF_STR( None ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Controls ) )     PORT_DIPLOCATION("SW1:!2")
	PORT_DIPSETTING(    0x02, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x00, "Reversed" )
	PORT_DIPUNUSED_DIPLOC( 0x04, 0x00, "SW1:!5" )       /* probably unused */
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SW1:!6")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( pinboa )
	PORT_INCLUDE( pinbo )

	PORT_MODIFY("1806")
	PORT_DIPNAME( 0x30, 0x00, DEF_STR( Lives ) )        PORT_DIPLOCATION("SW2:!5,!6")
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x10, "2" )
	PORT_DIPSETTING(    0x20, "3" )
	PORT_DIPSETTING(    0x30, "70 (Cheat)")
INPUT_PORTS_END


static const gfx_layout lasso_charlayout =
{
	8,8,
	RGN_FRAC(1,4),
	2,
	{ RGN_FRAC(0,4), RGN_FRAC(2,4) },
	{ STEP8(0,1) },
	{ STEP8(0,8) },
	8*8
};

static const gfx_layout lasso_spritelayout =
{
	16,16,
	RGN_FRAC(1,4),
	2,
	{ RGN_FRAC(1,4), RGN_FRAC(3,4) },
	{ STEP8(0,1), STEP8(8*8*1,1) },
	{ STEP8(0,8), STEP8(8*8*2,8) },
	16*16
};

static const gfx_layout wwjgtin_tracklayout =
{
	16,16,
	RGN_FRAC(1,4),
	4,
	{ RGN_FRAC(1,4), RGN_FRAC(3,4), RGN_FRAC(0,4), RGN_FRAC(2,4) },
	{ STEP8(0,1), STEP8(8*8*1,1) },
	{ STEP8(0,8), STEP8(8*8*2,8) },
	16*16
};

/* Pinbo is 3bpp, otherwise the same */
static const gfx_layout pinbo_charlayout =
{
	8,8,
	RGN_FRAC(1,6),
	3,
	{ RGN_FRAC(0,6), RGN_FRAC(2,6), RGN_FRAC(4,6) },
	{ STEP8(0,1) },
	{ STEP8(0,8) },
	8*8
};

static const gfx_layout pinbo_spritelayout =
{
	16,16,
	RGN_FRAC(1,6),
	3,
	{ RGN_FRAC(1,6), RGN_FRAC(3,6), RGN_FRAC(5,6) },
	{ STEP8(0,1), STEP8(8*8*1,1) },
	{ STEP8(0,8), STEP8(8*8*2,8) },
	16*16
};


static GFXDECODE_START( lasso )
	GFXDECODE_ENTRY( "gfx1", 0, lasso_charlayout,   0, 16 )
	GFXDECODE_ENTRY( "gfx1", 0, lasso_spritelayout, 0, 16 )
GFXDECODE_END

static GFXDECODE_START( wwjgtin )
	GFXDECODE_ENTRY( "gfx1", 0, lasso_charlayout,       0, 16 )
	GFXDECODE_ENTRY( "gfx1", 0, lasso_spritelayout,     0, 16 )
	GFXDECODE_ENTRY( "gfx2", 0, wwjgtin_tracklayout, 4*16, 16 )
GFXDECODE_END

static GFXDECODE_START( pinbo )
	GFXDECODE_ENTRY( "gfx1", 0, pinbo_charlayout,   0, 16 )
	GFXDECODE_ENTRY( "gfx1", 0, pinbo_spritelayout, 0, 16 )
GFXDECODE_END


void lasso_state::machine_start()
{
	save_item(NAME(m_gfxbank));
}

MACHINE_START_MEMBER(lasso_state,wwjgtin)
{
	lasso_state::machine_start();

	save_item(NAME(m_track_enable));
}

void lasso_state::machine_reset()
{
	m_gfxbank = 0;
}

MACHINE_RESET_MEMBER(lasso_state,wwjgtin)
{
	lasso_state::machine_reset();

	m_track_enable = 0;
}

static MACHINE_CONFIG_START( base, lasso_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M6502, 11289000/16) /* guess */
	MCFG_CPU_PROGRAM_MAP(lasso_main_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", lasso_state,  irq0_line_hold)

	MCFG_CPU_ADD("audiocpu", M6502, 600000)
	MCFG_CPU_PROGRAM_MAP(lasso_audio_map)

	MCFG_QUANTUM_TIME(attotime::from_hz(6000))

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(57)    /* guess, but avoids glitching of Chameleon's high score table */
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500) /* not accurate */)
	MCFG_SCREEN_SIZE(32*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0, 32*8-1, 2*8, 30*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(lasso_state, screen_update_lasso)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", lasso)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("sn76489.1", SN76489, 2000000)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)

	MCFG_SOUND_ADD("sn76489.2", SN76489, 2000000)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( lasso, base )

	/* basic machine hardware */
	MCFG_CPU_ADD("blitter", M6502, 11289000/16) /* guess */
	MCFG_CPU_PROGRAM_MAP(lasso_coprocessor_map)

	MCFG_PALETTE_ADD("palette", 0x40)
	MCFG_PALETTE_INIT_OWNER(lasso_state, lasso)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( chameleo, base )

	/* basic machine hardware */
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(chameleo_main_map)

	MCFG_CPU_MODIFY("audiocpu")
	MCFG_CPU_PROGRAM_MAP(chameleo_audio_map)

	/* video hardware */
	MCFG_PALETTE_ADD("palette", 0x40)
	MCFG_PALETTE_INIT_OWNER(lasso_state, lasso)

	MCFG_SCREEN_MODIFY("screen")
	MCFG_SCREEN_UPDATE_DRIVER(lasso_state, screen_update_chameleo)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( wwjgtin, base )

	/* basic machine hardware */
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(wwjgtin_main_map)

	MCFG_CPU_MODIFY("audiocpu")
	MCFG_CPU_PROGRAM_MAP(wwjgtin_audio_map)

	MCFG_MACHINE_START_OVERRIDE(lasso_state,wwjgtin)
	MCFG_MACHINE_RESET_OVERRIDE(lasso_state,wwjgtin)

	/* video hardware */
	MCFG_SCREEN_MODIFY("screen")
	MCFG_SCREEN_VISIBLE_AREA(1*8, 31*8-1, 2*8, 30*8-1)  // Smaller visible area?
	MCFG_SCREEN_UPDATE_DRIVER(lasso_state, screen_update_wwjgtin)
	MCFG_GFXDECODE_MODIFY("gfxdecode", wwjgtin) // Has 1 additional layer

	MCFG_PALETTE_ADD("palette", 0x40 + 16*16)
	MCFG_PALETTE_INDIRECT_ENTRIES(64)
	MCFG_PALETTE_INIT_OWNER(lasso_state,wwjgtin)
	MCFG_VIDEO_START_OVERRIDE(lasso_state,wwjgtin)

	/* sound hardware */
	MCFG_DAC_ADD("dac")
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( pinbo, base )

	/* basic machine hardware */
	MCFG_CPU_REPLACE("maincpu", M6502, XTAL_18MHz/24)
	MCFG_CPU_PROGRAM_MAP(pinbo_main_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", lasso_state,  irq0_line_hold)

	MCFG_CPU_REPLACE("audiocpu", Z80, XTAL_18MHz/6)
	MCFG_CPU_PROGRAM_MAP(pinbo_audio_map)
	MCFG_CPU_IO_MAP(pinbo_audio_io_map)

	/* video hardware */
	MCFG_GFXDECODE_MODIFY("gfxdecode", pinbo)

	MCFG_PALETTE_ADD_RRRRGGGGBBBB_PROMS("palette", 256)
	MCFG_VIDEO_START_OVERRIDE(lasso_state,pinbo)
	MCFG_SCREEN_MODIFY("screen")
	MCFG_SCREEN_UPDATE_DRIVER(lasso_state, screen_update_chameleo)

	/* sound hardware */
	MCFG_DEVICE_REMOVE("sn76489.1")
	MCFG_DEVICE_REMOVE("sn76489.2")

	MCFG_SOUND_ADD("ay1", AY8910, XTAL_18MHz/12)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.55)

	MCFG_SOUND_ADD("ay2", AY8910, XTAL_18MHz/12)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.55)
MACHINE_CONFIG_END


ROM_START( lasso )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "wm3",       0x8000, 0x2000, CRC(f93addd6) SHA1(b0a1b263874da8608c3bab4e8785358e2aa19c2e) )
	ROM_LOAD( "wm4",       0xa000, 0x2000, CRC(77719859) SHA1(d206b6af9a567f70d69624866ae9973652527065) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "wmc",       0x5000, 0x1000, CRC(8b4eb242) SHA1(55ada50036abbaa10799f37e35254e6ff70ee947) )
	ROM_LOAD( "wmb",       0x6000, 0x1000, CRC(4658bcb9) SHA1(ecc83ef99edbe5f69a884a142478ff0f56edba12) )
	ROM_LOAD( "wma",       0x7000, 0x1000, CRC(2e7de3e9) SHA1(665a89b9914ca16b9c08b751e142cf7320aaf793) )

	ROM_REGION( 0x10000, "blitter", 0 )
	ROM_LOAD( "wm5",       0x8000, 0x1000, CRC(7dc3ff07) SHA1(46aaa9186940d06fd679a573330e9ad3796aa647) )

	ROM_REGION( 0x4000, "gfx1", 0 )
	ROM_LOAD( "wm1",       0x0000, 0x0800, CRC(7db77256) SHA1(d12305bdfb6923c32982982a5544ae9bd8dbc2cb) )
	ROM_CONTINUE(          0x1000, 0x0800 )
	ROM_CONTINUE(          0x0800, 0x0800 )
	ROM_CONTINUE(          0x1800, 0x0800 )
	ROM_LOAD( "wm2",       0x2000, 0x0800, CRC(9e7d0b6f) SHA1(c82be332209bf7331718e51926004fe9aa6f5ebd) )
	ROM_CONTINUE(          0x3000, 0x0800 )
	ROM_CONTINUE(          0x2800, 0x0800 )
	ROM_CONTINUE(          0x3800, 0x0800 )

	ROM_REGION( 0x40, "proms", 0 )
	ROM_LOAD( "82s123.69", 0x0000, 0x0020, CRC(1eabb04d) SHA1(3dc5b407bc1b1dea77337b4e913f1e945386d5c9) )
	ROM_LOAD( "82s123.70", 0x0020, 0x0020, CRC(09060f8c) SHA1(8f14b00bcfb7ab89d2e443cc82f7a65dc96ee819) )
ROM_END

ROM_START( chameleo )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "chamel4.bin", 0x4000, 0x2000, CRC(97379c47) SHA1(b29fa2318d4260c29fc95d22a461173dc960ad1a) )
	ROM_LOAD( "chamel5.bin", 0x6000, 0x2000, CRC(0a2cadfd) SHA1(1ccc43accd60ca15b8f03ed1c3fda76a840a2bb1) )
	ROM_LOAD( "chamel6.bin", 0x8000, 0x2000, CRC(b023c354) SHA1(0424ecf81ac9f0e055f9ff01cf0bd6d5c9ff866c) )
	ROM_LOAD( "chamel7.bin", 0xa000, 0x2000, CRC(a5a03375) SHA1(c1eac4596c2bda419f3c513ecd3df9fae49ae159) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "chamel3.bin", 0x1000, 0x1000, CRC(52eab9ec) SHA1(554c34134e3af970262da89fe82feeaf47fd30bc) )
	ROM_LOAD( "chamel2.bin", 0x6000, 0x1000, CRC(81dcc49c) SHA1(7e1b4351775f9c140a43f531da8b055271b7b28c) )
	ROM_LOAD( "chamel1.bin", 0x7000, 0x1000, CRC(96031d3b) SHA1(a143b54b98891423d355e0ba08c3b88d70fa0e23) )

	ROM_REGION( 0x4000, "gfx1", 0 )
	ROM_LOAD( "chamel8.bin", 0x0800, 0x0800, CRC(dc67916b) SHA1(8b3fad0d5d42925b44e51df7f88ea4b6a8dbb4f6) )
	ROM_CONTINUE(            0x1800, 0x0800 )
	ROM_CONTINUE(            0x0000, 0x0800 )
	ROM_CONTINUE(            0x1000, 0x0800 )
	ROM_LOAD( "chamel9.bin", 0x2800, 0x0800, CRC(6b559bf1) SHA1(b7b8b8bccbd88ea868e2d3ccb42513615120d8e6) )
	ROM_CONTINUE(            0x3800, 0x0800 )
	ROM_CONTINUE(            0x2000, 0x0800 )
	ROM_CONTINUE(            0x3000, 0x0800 )

	ROM_REGION( 0x40, "proms", 0 )
	ROM_LOAD( "chambprm.bin", 0x0000, 0x0020, CRC(e3ad76df) SHA1(cd115cece4931bfcfc0f60147b942998a5c21bf7) )
	ROM_LOAD( "chamaprm.bin", 0x0020, 0x0020, CRC(c7063b54) SHA1(53baed3806848207ab3a8fafd182cabec3be4b04) )
ROM_END

ROM_START( wwjgtin )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ic2.6", 0x4000, 0x4000, CRC(744ba45b) SHA1(cccf3e2dd3c27bf54d2abd366cd9a044311aa031) )
	ROM_LOAD( "ic5.5", 0x8000, 0x4000, CRC(af751614) SHA1(fc0f0a3967524b1743a182c1da4f9b0c3097a157) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "ic59.9", 0x4000, 0x4000, CRC(2ecb4d98) SHA1(d5b0d447b24f64fca452dc13e6ff95b090fce2d7) )

	ROM_REGION( 0x8000, "gfx1", 0 )
	ROM_LOAD( "ic81.7", 0x0000, 0x0800, CRC(a27f1a63) SHA1(3c770424bd4996f648687afce4aecea252da83a7) )
	ROM_CONTINUE(       0x2000, 0x0800 )
	ROM_CONTINUE(       0x0800, 0x0800 )
	ROM_CONTINUE(       0x2800, 0x0800 )
	ROM_CONTINUE(       0x1000, 0x0800 )
	ROM_CONTINUE(       0x3000, 0x0800 )
	ROM_CONTINUE(       0x1800, 0x0800 )
	ROM_CONTINUE(       0x3800, 0x0800 )
	ROM_LOAD( "ic82.8", 0x4000, 0x0800, CRC(ea2862b3) SHA1(f7604fd324560c54311c35f806a17e30e018032a) )
	ROM_CONTINUE(       0x6000, 0x0800 )
	ROM_CONTINUE(       0x4800, 0x0800 )
	ROM_CONTINUE(       0x6800, 0x0800 )
	ROM_CONTINUE(       0x5000, 0x0800 )
	ROM_CONTINUE(       0x7000, 0x0800 )
	ROM_CONTINUE(       0x5800, 0x0800 )
	ROM_CONTINUE(       0x7800, 0x0800 )

	ROM_REGION( 0x4000, "gfx2", 0 )
	ROM_LOAD( "ic47.3", 0x0000, 0x2000, CRC(40594c59) SHA1(94533be8e267d9aa5bcdd52b45f6974436d3fed5) )  // 1xxxxxxxxxxxx = 0xFF
	ROM_LOAD( "ic46.4", 0x2000, 0x2000, CRC(d1921348) SHA1(8b5506ff80a31ce721aed515cad1b4a7e52e47a2) )

	ROM_REGION( 0x4000, "user1", 0 )                /* tilemap */
	ROM_LOAD( "ic48.2", 0x0000, 0x2000, CRC(a4a7df77) SHA1(476aab702346a402169ab404a8b06589e4932d37) )
	ROM_LOAD( "ic49.1", 0x2000, 0x2000, CRC(e480fbba) SHA1(197c86747ef8477040169f90eb6e04d928aedbe5) )  // FIXED BITS (1111xxxx)

	ROM_REGION( 0x40, "proms", 0 )
	ROM_LOAD( "2.bpr",  0x0000, 0x0020, CRC(79adda5d) SHA1(e54de3eb02f744d49f524cd81e1cf993338916e3) )
	ROM_LOAD( "1.bpr",  0x0020, 0x0020, CRC(c1a93cc8) SHA1(805641ea2ce86589b968f1ff44e5d3ab9377769d) )
ROM_END

ROM_START( photof )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ic2.bin", 0x4000, 0x4000, CRC(4d960b54) SHA1(fe6c4943cbf9a9c79a2fd1dd86bb6e1f414b3c8d) )
	ROM_LOAD( "ic6.bin", 0x8000, 0x4000, CRC(a4ad21dc) SHA1(55b3ecdf80b4a384a0d9932756330fb3021502f8) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "ic59.bin", 0x4000, 0x4000, CRC(2ecb4d98) SHA1(d5b0d447b24f64fca452dc13e6ff95b090fce2d7) )

	ROM_REGION( 0x8000, "gfx1", 0 )
	ROM_LOAD( "ic81.bin", 0x0000, 0x0800, CRC(0f170253) SHA1(e8b09cf4e9bae6c762ff325a559fb860a80133aa) )
	ROM_CONTINUE(       0x2000, 0x0800 )
	ROM_CONTINUE(       0x0800, 0x0800 )
	ROM_CONTINUE(       0x2800, 0x0800 )
	ROM_CONTINUE(       0x1000, 0x0800 )
	ROM_CONTINUE(       0x3000, 0x0800 )
	ROM_CONTINUE(       0x1800, 0x0800 )
	ROM_CONTINUE(       0x3800, 0x0800 )
	ROM_LOAD( "ic82.bin", 0x4000, 0x0800, CRC(c4cadee9) SHA1(46cc0ecc3642c432625c0d131aa31fec2e060d2f) )
	ROM_CONTINUE(       0x6000, 0x0800 )
	ROM_CONTINUE(       0x4800, 0x0800 )
	ROM_CONTINUE(       0x6800, 0x0800 )
	ROM_CONTINUE(       0x5000, 0x0800 )
	ROM_CONTINUE(       0x7000, 0x0800 )
	ROM_CONTINUE(       0x5800, 0x0800 )
	ROM_CONTINUE(       0x7800, 0x0800 )

	ROM_REGION( 0x4000, "gfx2", 0 )
	ROM_LOAD( "3-ic47.bin", 0x0000, 0x2000, CRC(40594c59) SHA1(94533be8e267d9aa5bcdd52b45f6974436d3fed5) )  // 1xxxxxxxxxxxx = 0xFF
	ROM_LOAD( "4-ic46.bin", 0x2000, 0x2000, CRC(d1921348) SHA1(8b5506ff80a31ce721aed515cad1b4a7e52e47a2) )

	ROM_REGION( 0x4000, "user1", 0 )                /* tilemap */
	ROM_LOAD( "2-ic48.bin", 0x0000, 0x2000, CRC(a4a7df77) SHA1(476aab702346a402169ab404a8b06589e4932d37) )
	ROM_LOAD( "1-ic49.bin", 0x2000, 0x2000, CRC(e480fbba) SHA1(197c86747ef8477040169f90eb6e04d928aedbe5) )  // FIXED BITS (1111xxxx)

	ROM_REGION( 0x40, "proms", 0 )
	ROM_LOAD( "2.bpr",  0x0000, 0x0020, CRC(79adda5d) SHA1(e54de3eb02f744d49f524cd81e1cf993338916e3) )
	ROM_LOAD( "1.bpr",  0x0020, 0x0020, CRC(c1a93cc8) SHA1(805641ea2ce86589b968f1ff44e5d3ab9377769d) )
ROM_END

ROM_START( pinbo )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "rom2.b7",     0x2000, 0x2000, CRC(9a185338) SHA1(4029cf927686b5e14ef7600b17ea3056cc58b15b) )
	ROM_LOAD( "rom3.e7",     0x6000, 0x2000, CRC(1cd1b3bd) SHA1(388ea72568f5bfd39856d872415327a2afaf7fad) )
	ROM_LOAD( "rom4.h7",     0x8000, 0x2000, CRC(ba043fa7) SHA1(ef3d67b6dab5c82035c58290879a3ca969a0256d) )
	ROM_LOAD( "rom5.j7",     0xa000, 0x2000, CRC(e71046c4) SHA1(f49133544c98df5f3e1a1d2ae92e17261b1504fc) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "rom1.s8",     0x0000, 0x2000, CRC(ca45a1be) SHA1(d0b2d8f1e6d01b60cba83d2bd458a57548549b4b) )

	ROM_REGION( 0xc000, "gfx1", 0 )
	ROM_LOAD( "rom6.a1",     0x0000, 0x0800, CRC(74fe8e98) SHA1(3c9ac38d7054b2831a515786b6f204b1804aaea3) ) /* tiles   */
	ROM_CONTINUE(            0x2000, 0x0800 )
	ROM_CONTINUE(            0x0800, 0x0800 )
	ROM_CONTINUE(            0x2800, 0x0800 )
	ROM_CONTINUE(            0x1000, 0x0800 )
	ROM_CONTINUE(            0x3000, 0x0800 )
	ROM_CONTINUE(            0x1800, 0x0800 )
	ROM_CONTINUE(            0x3800, 0x0800 )
	ROM_LOAD( "rom8.c1",     0x4000, 0x0800, CRC(5a800fe7) SHA1(375269ec73fab7f0cf017a79e002e31b006f5ad7) )
	ROM_CONTINUE(            0x6000, 0x0800 )
	ROM_CONTINUE(            0x4800, 0x0800 )
	ROM_CONTINUE(            0x6800, 0x0800 )
	ROM_CONTINUE(            0x5000, 0x0800 )
	ROM_CONTINUE(            0x7000, 0x0800 )
	ROM_CONTINUE(            0x5800, 0x0800 )
	ROM_CONTINUE(            0x7800, 0x0800 )
	ROM_LOAD( "rom7.d1",     0x8000, 0x0800, CRC(327a3c21) SHA1(e938915d28ac4ec033b20d33728788493e3f30f6) ) /* 3rd bitplane */
	ROM_CONTINUE(            0xa000, 0x0800 )
	ROM_CONTINUE(            0x8800, 0x0800 )
	ROM_CONTINUE(            0xa800, 0x0800 )
	ROM_CONTINUE(            0x9000, 0x0800 )
	ROM_CONTINUE(            0xb000, 0x0800 )
	ROM_CONTINUE(            0x9800, 0x0800 )
	ROM_CONTINUE(            0xb800, 0x0800 )

	ROM_REGION( 0x00300, "proms", 0 )
	ROM_LOAD( "red.l10",     0x0000, 0x0100, CRC(e6c9ba52) SHA1(6ea96f9bd71de6181d675b0f2d59a8c5e1be5aa3) ) // 2nd half is garbage?
	ROM_LOAD( "green.k10",   0x0100, 0x0100, CRC(1bf2d335) SHA1(dcb074d3de939dfc652743e25bc66bd6fbdc3289) ) // "
	ROM_LOAD( "blue.n10",    0x0200, 0x0100, CRC(e41250ad) SHA1(2e9a2babbacb1753057d46cf1dd6dc183611747e) ) // "
ROM_END

ROM_START( pinboa )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "rom2.b7",     0x2000, 0x2000, CRC(9a185338) SHA1(4029cf927686b5e14ef7600b17ea3056cc58b15b) )
	ROM_LOAD( "6.bin",       0x6000, 0x2000, CRC(f80b204c) SHA1(ee9b4ae1d8ea2fc062022fcfae67df87ed7aff41) )
	ROM_LOAD( "5.bin",       0x8000, 0x2000, CRC(c57fe503) SHA1(11b7371c07c9b2c73ab61420a2cc609653c48d37) )
	ROM_LOAD( "4.bin",       0xa000, 0x2000, CRC(d632b598) SHA1(270a5a790a66eaf3d90bf8081ab144fd1af9db3d) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "8.bin",       0x0000, 0x2000, CRC(32d1df14) SHA1(c0d4181378bbd6f2c594e923e2f8b21647c7fb0e) )

	ROM_REGION( 0xc000, "gfx1", 0 )
	ROM_LOAD( "rom6.a1",     0x0000, 0x0800, CRC(74fe8e98) SHA1(3c9ac38d7054b2831a515786b6f204b1804aaea3) ) /* tiles   */
	ROM_CONTINUE(            0x2000, 0x0800 )
	ROM_CONTINUE(            0x0800, 0x0800 )
	ROM_CONTINUE(            0x2800, 0x0800 )
	ROM_CONTINUE(            0x1000, 0x0800 )
	ROM_CONTINUE(            0x3000, 0x0800 )
	ROM_CONTINUE(            0x1800, 0x0800 )
	ROM_CONTINUE(            0x3800, 0x0800 )
	ROM_LOAD( "rom8.c1",     0x4000, 0x0800, CRC(5a800fe7) SHA1(375269ec73fab7f0cf017a79e002e31b006f5ad7) )
	ROM_CONTINUE(            0x6000, 0x0800 )
	ROM_CONTINUE(            0x4800, 0x0800 )
	ROM_CONTINUE(            0x6800, 0x0800 )
	ROM_CONTINUE(            0x5000, 0x0800 )
	ROM_CONTINUE(            0x7000, 0x0800 )
	ROM_CONTINUE(            0x5800, 0x0800 )
	ROM_CONTINUE(            0x7800, 0x0800 )
	ROM_LOAD( "2.bin",       0x8000, 0x0800, CRC(33cac92e) SHA1(55d4ff3ae9c9519a59bd6021a53584c873b4d327) ) /* 3rd bitplane */
	ROM_CONTINUE(            0xa000, 0x0800 )
	ROM_CONTINUE(            0x8800, 0x0800 )
	ROM_CONTINUE(            0xa800, 0x0800 )
	ROM_CONTINUE(            0x9000, 0x0800 )
	ROM_CONTINUE(            0xb000, 0x0800 )
	ROM_CONTINUE(            0x9800, 0x0800 )
	ROM_CONTINUE(            0xb800, 0x0800 )

	ROM_REGION( 0x00300, "proms", 0 )
	ROM_LOAD( "red.l10",     0x0000, 0x0100, CRC(e6c9ba52) SHA1(6ea96f9bd71de6181d675b0f2d59a8c5e1be5aa3) )
	ROM_LOAD( "green.k10",   0x0100, 0x0100, CRC(1bf2d335) SHA1(dcb074d3de939dfc652743e25bc66bd6fbdc3289) )
	ROM_LOAD( "blue.n10",    0x0200, 0x0100, CRC(e41250ad) SHA1(2e9a2babbacb1753057d46cf1dd6dc183611747e) )
ROM_END

ROM_START( pinbos )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "b4.bin",      0x2000, 0x2000, CRC(d9452d4f) SHA1(c744ee037275b880c0ddc2fd83b3c05eb0a53621) )
	ROM_LOAD( "b5.bin",      0x6000, 0x2000, CRC(f80b204c) SHA1(ee9b4ae1d8ea2fc062022fcfae67df87ed7aff41) )
	ROM_LOAD( "b6.bin",      0x8000, 0x2000, CRC(ae967d83) SHA1(e79db85917a31821d10f919c4c429da33e97894d) )
	ROM_LOAD( "b7.bin",      0xa000, 0x2000, CRC(7a584b4e) SHA1(2eb55b706815228b3b12ee5c0f6c415cd1d612e6) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "b8.bin",      0x0000, 0x2000, CRC(32d1df14) SHA1(c0d4181378bbd6f2c594e923e2f8b21647c7fb0e) )

	ROM_REGION( 0xc000, "gfx1", 0 )
	ROM_LOAD( "rom6.a1",     0x0000, 0x0800, CRC(74fe8e98) SHA1(3c9ac38d7054b2831a515786b6f204b1804aaea3) ) /* tiles   */
	ROM_CONTINUE(            0x2000, 0x0800 )
	ROM_CONTINUE(            0x0800, 0x0800 )
	ROM_CONTINUE(            0x2800, 0x0800 )
	ROM_CONTINUE(            0x1000, 0x0800 )
	ROM_CONTINUE(            0x3000, 0x0800 )
	ROM_CONTINUE(            0x1800, 0x0800 )
	ROM_CONTINUE(            0x3800, 0x0800 )
	ROM_LOAD( "rom8.c1",     0x4000, 0x0800, CRC(5a800fe7) SHA1(375269ec73fab7f0cf017a79e002e31b006f5ad7) )
	ROM_CONTINUE(            0x6000, 0x0800 )
	ROM_CONTINUE(            0x4800, 0x0800 )
	ROM_CONTINUE(            0x6800, 0x0800 )
	ROM_CONTINUE(            0x5000, 0x0800 )
	ROM_CONTINUE(            0x7000, 0x0800 )
	ROM_CONTINUE(            0x5800, 0x0800 )
	ROM_CONTINUE(            0x7800, 0x0800 )
	ROM_LOAD( "rom7.d1",     0x8000, 0x0800, CRC(327a3c21) SHA1(e938915d28ac4ec033b20d33728788493e3f30f6) ) /* 3rd bitplane */
	ROM_CONTINUE(            0xa000, 0x0800 )
	ROM_CONTINUE(            0x8800, 0x0800 )
	ROM_CONTINUE(            0xa800, 0x0800 )
	ROM_CONTINUE(            0x9000, 0x0800 )
	ROM_CONTINUE(            0xb000, 0x0800 )
	ROM_CONTINUE(            0x9800, 0x0800 )
	ROM_CONTINUE(            0xb800, 0x0800 )

	ROM_REGION( 0x00300, "proms", 0 )
	ROM_LOAD( "red.l10",     0x0000, 0x0100, CRC(e6c9ba52) SHA1(6ea96f9bd71de6181d675b0f2d59a8c5e1be5aa3) )
	ROM_LOAD( "green.k10",   0x0100, 0x0100, CRC(1bf2d335) SHA1(dcb074d3de939dfc652743e25bc66bd6fbdc3289) )
	ROM_LOAD( "blue.n10",    0x0200, 0x0100, CRC(e41250ad) SHA1(2e9a2babbacb1753057d46cf1dd6dc183611747e) )
ROM_END



/***************************************************************************

                                Game Drivers

***************************************************************************/

GAME( 1982, lasso,    0,       lasso,    lasso,    driver_device,  0, ROT90, "SNK",              "Lasso", MACHINE_SUPPORTS_SAVE )
GAME( 1983, chameleo, 0,       chameleo, chameleo, driver_device,  0, ROT0,  "Jaleco",           "Chameleon", MACHINE_SUPPORTS_SAVE )
GAME( 1984, wwjgtin,  0,       wwjgtin,  wwjgtin,  driver_device,  0, ROT0,  "Jaleco / Casio",   "Wai Wai Jockey Gate-In!", MACHINE_SUPPORTS_SAVE )
GAME( 1991, photof,   wwjgtin, wwjgtin,  wwjgtin,  driver_device,  0, ROT0,  "Jaleco / Casio",   "Photo Finish (bootleg?)", MACHINE_SUPPORTS_SAVE )
GAME( 1984, pinbo,    0,       pinbo,    pinbo,    driver_device,  0, ROT90, "Jaleco",           "Pinbo (set 1)", MACHINE_SUPPORTS_SAVE )
GAME( 1984, pinboa,   pinbo,   pinbo,    pinboa,   driver_device,  0, ROT90, "Jaleco",           "Pinbo (set 2)", MACHINE_SUPPORTS_SAVE )
GAME( 1985, pinbos,   pinbo,   pinbo,    pinboa,   driver_device,  0, ROT90, "bootleg (Strike)", "Pinbo (bootleg)", MACHINE_SUPPORTS_SAVE )
