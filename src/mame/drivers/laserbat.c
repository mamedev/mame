/*

    Laser Battle / Lazarian (c) 1981 Zaccaria
    Cat and Mouse           (c) 1982 Zaccaria

    driver by Pierpaolo Prazzoli

    The 2 games have a similar video hardware, but sound hardware is very different
    and they don't use the collision detection provided by the s2636 chips.

TODO:
- how to use the 82S100 PLA dump
- colors (tile_index in tilemap needs to be adjusted)
- sound in laserbat (with schematics) and in catnmous
- cocktail support

*/

#include "emu.h"
#include "cpu/m6800/m6800.h"
#include "cpu/s2650/s2650.h"
#include "machine/6821pia.h"
#include "sound/ay8910.h"
#include "sound/sn76477.h"
#include "sound/tms3615.h"
#include "video/s2636.h"
#include "includes/laserbat.h"


WRITE8_MEMBER(laserbat_state::laserbat_videoram_w)
{
	if (m_video_page == 0)
	{
		m_videoram[offset] = data;
		m_bg_tilemap->mark_tile_dirty(offset);
	}
	else if (m_video_page == 1)
	{
		m_colorram[offset] = data;
		m_bg_tilemap->mark_tile_dirty(offset); // wrong!
	}
}

WRITE8_MEMBER(laserbat_state::video_extra_w)
{
	m_video_page = (data & 0x10) >> 4;
	m_sprite_enable = (data & 1) ^ 1;
	m_sprite_code = (data & 0xe0) >> 5;
	m_sprite_color = (data & 0x0e) >> 1;
}

WRITE8_MEMBER(laserbat_state::sprite_x_y_w)
{
	if (offset == 0)
		m_sprite_x = 256 - data;
	else
		m_sprite_y = 256 - data;
}

WRITE8_MEMBER(laserbat_state::laserbat_input_mux_w)
{
	m_input_mux = (data & 0x30) >> 4;

	flip_screen_set_no_update(data & 0x08);

	coin_counter_w(machine(), 0,data & 1);

	//data & 0x02 ?
	//data & 0x04 ?
}

READ8_MEMBER(laserbat_state::laserbat_input_r)
{
	static const char *const portnames[] = { "IN0", "IN1", "IN2", "IN3" };

	return ioport(portnames[m_input_mux])->read();
}

WRITE8_MEMBER(laserbat_state::laserbat_cnteff_w)
{
	// 0x01 = _ABEFF1
	// 0x02 = _ABEFF2
	// 0x04 = MPX EFF2-_SW
	// 0x08 = COLEFF 0
	// 0x10 = COLEFF 1
	// 0x20 = _NEG 1
	// 0x40 = _NEG 2
	// 0x80 = MPX P 1/2
}

#ifdef UNUSED_FUNCTION
WRITE8_MEMBER(laserbat_state::laserbat_cntmov_w)
{
	// 0x01 = AB MOVE
	// 0x02 = CLH0
	// 0x04 = CLH1
	// 0x08 = LUM
	// 0x10 = MPX BKEFF
	// 0x20 = SHPA
	// 0x40 = SHPB
	// 0x80 = SHPC
}
#endif

/*

    Color handling with 2716.14L and 82S100.10M

    2716.14L address lines are connected as follows:

    A0  4H
    A1  8H
    A2  16H
    A3  1V
    A4  2V
    A5  4V
    A6  8V
    A7  16V
    A8  SHPA
    A9  SHPB
    A10 SHPC

    The output of the 2716.14L is sent to the 82S100.10M
    through a parallel-to-serial shift register that is clocked
    on (1H && 2H). The serial data sent is as follows:

    NAV0    D6, D4, D2, D0, 0, 0, 0, 0
    NAV1    D7, D5, D3, D1, 0, 0, 0, 0

    82S100.10M lines are connected as follows:

    I0  NAV0
    I1  NAV1
    I2  CLH0
    I3  CLH1
    I4  LUM
    I5  C1*
    I6  C2*
    I7  C3*
    I8  BKR
    I9  BKG
    I10 BKB
    I11 SHELL
    I12 EFF1
    I13 EFF2
    I14 COLEFF0
    I15 COLEFF1

    F0  -> 820R -> RED
    F1  -> 820R -> GREEN
    F2  -> 820R -> BLUE
    F3  -> 270R -> RED
    F4  -> 270R -> GREEN
    F5  -> 270R -> BLUE
    F6  -> 1K -> RED
    F7  -> 1K -> GREEN

*/

static ADDRESS_MAP_START( laserbat_map, AS_PROGRAM, 8, laserbat_state )
	AM_RANGE(0x0000, 0x13ff) AM_ROM
	AM_RANGE(0x2000, 0x33ff) AM_ROM
	AM_RANGE(0x3800, 0x3bff) AM_ROM
	AM_RANGE(0x4000, 0x53ff) AM_ROM
	AM_RANGE(0x6000, 0x73ff) AM_ROM
	AM_RANGE(0x7800, 0x7bff) AM_ROM

	AM_RANGE(0x1400, 0x14ff) AM_MIRROR(0x6000) AM_WRITENOP // always 0 (bullet ram in Quasar)
	AM_RANGE(0x1500, 0x15ff) AM_MIRROR(0x6000) AM_DEVREADWRITE_LEGACY("s2636_1", s2636_work_ram_r, s2636_work_ram_w)
	AM_RANGE(0x1600, 0x16ff) AM_MIRROR(0x6000) AM_DEVREADWRITE_LEGACY("s2636_2", s2636_work_ram_r, s2636_work_ram_w)
	AM_RANGE(0x1700, 0x17ff) AM_MIRROR(0x6000) AM_DEVREADWRITE_LEGACY("s2636_3", s2636_work_ram_r, s2636_work_ram_w)
	AM_RANGE(0x1800, 0x1bff) AM_MIRROR(0x6000) AM_WRITE(laserbat_videoram_w)
	AM_RANGE(0x1c00, 0x1fff) AM_MIRROR(0x6000) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( laserbat_io_map, AS_IO, 8, laserbat_state )
	AM_RANGE(0x00, 0x00) AM_WRITE(laserbat_cnteff_w)
	AM_RANGE(0x01, 0x01) AM_WRITE(video_extra_w)
	AM_RANGE(0x02, 0x02) AM_READ(laserbat_input_r) AM_WRITE(laserbat_csound1_w)
	AM_RANGE(0x04, 0x05) AM_WRITE(sprite_x_y_w)
	AM_RANGE(0x06, 0x06) AM_WRITE(laserbat_input_mux_w)
	AM_RANGE(0x07, 0x07) AM_WRITE(laserbat_csound2_w)
	AM_RANGE(S2650_SENSE_PORT, S2650_SENSE_PORT) AM_READ_PORT("SENSE")
	AM_RANGE(S2650_FO_PORT, S2650_FO_PORT) AM_RAM AM_SHARE("fo_state")
ADDRESS_MAP_END


static ADDRESS_MAP_START( catnmous_io_map, AS_IO, 8, laserbat_state )
	AM_RANGE(0x00, 0x00) AM_WRITE(soundlatch_byte_w) // soundlatch ?
	AM_RANGE(0x01, 0x01) AM_WRITE(video_extra_w)
	AM_RANGE(0x02, 0x02) AM_READ(laserbat_input_r)
	AM_RANGE(0x02, 0x02) AM_WRITENOP // unknown
	AM_RANGE(0x04, 0x05) AM_WRITE(sprite_x_y_w)
	AM_RANGE(0x06, 0x06) AM_WRITE(laserbat_input_mux_w)
	AM_RANGE(0x07, 0x07) AM_WRITENOP // unknown
	AM_RANGE(S2650_SENSE_PORT, S2650_SENSE_PORT) AM_READ_PORT("SENSE")
	AM_RANGE(S2650_FO_PORT, S2650_FO_PORT) AM_RAM AM_SHARE("fo_state")
ADDRESS_MAP_END

// the same as in zaccaria.c ?
static ADDRESS_MAP_START( catnmous_sound_map, AS_PROGRAM, 8, laserbat_state )
	AM_RANGE(0x0000, 0x007f) AM_RAM
	AM_RANGE(0x500c, 0x500f) AM_DEVREADWRITE("pia", pia6821_device, read, write)
	AM_RANGE(0xf000, 0xffff) AM_ROM
ADDRESS_MAP_END


static INPUT_PORTS_START( laserbat )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON4 )

	PORT_START("IN1")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("Reset")

	PORT_START("IN2")
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_5C ) )
	PORT_DIPNAME( 0x0c, 0x00, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_7C ) )
	PORT_DIPNAME( 0x70, 0x10, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPSETTING(    0x10, "3" )
	PORT_DIPSETTING(    0x20, "5" )
	PORT_DIPSETTING(    0x30, "6" )
	PORT_DIPSETTING(    0x40, "Infinite" )
//  PORT_DIPSETTING(    0x50, "Infinite" )
//  PORT_DIPSETTING(    0x60, "Infinite" )
//  PORT_DIPSETTING(    0x70, "Infinite" )
	PORT_DIPNAME( 0x80, 0x80, "Collision Detection" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START("IN3")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )

	PORT_START("SENSE")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_VBLANK("screen")
INPUT_PORTS_END

static INPUT_PORTS_START( lazarian )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON4 )

	PORT_START("IN1")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("Reset")

	PORT_START("IN2")
	PORT_DIPNAME( 0x03, 0x01, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_3C ) )
	PORT_DIPNAME( 0x0c, 0x00, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_7C ) )
	PORT_DIPNAME( 0x30, 0x10, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPSETTING(    0x10, "3" )
	PORT_DIPSETTING(    0x20, "4" )
	PORT_DIPSETTING(    0x30, "5" )
	PORT_DIPNAME( 0x40, 0x00, "Calibration Display" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Collision Detection" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START("IN3")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "Firing" )
	PORT_DIPSETTING(    0x02, "Rapid" )
	PORT_DIPSETTING(    0x00, DEF_STR( Normal ) )
	PORT_DIPNAME( 0x04, 0x00, "Freeze" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )

	PORT_START("SENSE")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_VBLANK("screen")
INPUT_PORTS_END

static INPUT_PORTS_START( catnmous )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN1")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("Reset")

	PORT_START("IN2")
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_5C ) )
	PORT_DIPNAME( 0x0c, 0x00, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_7C ) )
	PORT_DIPNAME( 0x70, 0x10, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPSETTING(    0x10, "3" )
	PORT_DIPSETTING(    0x20, "4" )
	PORT_DIPSETTING(    0x30, "5" )
	PORT_DIPSETTING(    0x40, "Infinite" )
//  PORT_DIPSETTING(    0x50, "Infinite" )
//  PORT_DIPSETTING(    0x60, "Infinite" )
//  PORT_DIPSETTING(    0x70, "Infinite" )
	PORT_DIPNAME( 0x80, 0x80, "Game Over Melody" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START("IN3")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("SENSE")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_VBLANK("screen")
INPUT_PORTS_END

static const gfx_layout charlayout =
{
	8,8,
	RGN_FRAC(1,3),
	3,
	{ RGN_FRAC(0,3), RGN_FRAC(1,3), RGN_FRAC(2,3) },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8
};

static const gfx_layout sprites_layout =
{
	32,32,
	RGN_FRAC(1,1),
	2,
	{ 0, 1 },
	{  0, 2, 4, 6, 8,10,12,14,16,18,20,22,24,26,28,30,
	  32,34,36,38,40,42,44,46,48,50,52,54,56,58,60,62
	},
	{  0*32, 2*32, 4*32, 6*32, 8*32,10*32,12*32,14*32,
	  16*32,18*32,20*32,22*32,24*32,26*32,28*32,30*32,
	  32*32,34*32,36*32,38*32,40*32,42*32,44*32,46*32,
	  48*32,50*32,52*32,54*32,56*32,58*32,60*32,62*32
	},
	32*32*2
};

static GFXDECODE_START( laserbat )
	GFXDECODE_ENTRY( "gfx1", 0x0000, charlayout,       0, 256 )	/* Rom chars */
	GFXDECODE_ENTRY( "gfx2", 0x0000, sprites_layout,   0,   8 )	/* Sprites   */
GFXDECODE_END

TILE_GET_INFO_MEMBER(laserbat_state::get_tile_info)
{

	// wrong color index!
	SET_TILE_INFO_MEMBER(0, m_videoram[tile_index], m_colorram[tile_index] & 0x7f, 0);
}

static VIDEO_START( laserbat )
{
	laserbat_state *state = machine.driver_data<laserbat_state>();

	state->m_bg_tilemap = &machine.tilemap().create(tilemap_get_info_delegate(FUNC(laserbat_state::get_tile_info),state), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);

	state->save_item(NAME(state->m_videoram));
	state->save_item(NAME(state->m_colorram));
}

static SCREEN_UPDATE_IND16( laserbat )
{
	laserbat_state *state = screen.machine().driver_data<laserbat_state>();
	int y;

	state->m_bg_tilemap->draw(bitmap, cliprect, 0, 0);

	/* update the S2636 chips */
	bitmap_ind16 &s2636_1_bitmap = s2636_update(state->m_s2636_1, cliprect);
	bitmap_ind16 &s2636_2_bitmap = s2636_update(state->m_s2636_2, cliprect);
	bitmap_ind16 &s2636_3_bitmap = s2636_update(state->m_s2636_3, cliprect);

	/* copy the S2636 images into the main bitmap */
	for (y = cliprect.min_y; y <= cliprect.max_y; y++)
	{
		int x;

		for (x = cliprect.min_x; x <= cliprect.max_x; x++)
		{
			int pixel1 = s2636_1_bitmap.pix16(y, x);
			int pixel2 = s2636_2_bitmap.pix16(y, x);
			int pixel3 = s2636_3_bitmap.pix16(y, x);

			if (S2636_IS_PIXEL_DRAWN(pixel1))
				bitmap.pix16(y, x) = S2636_PIXEL_COLOR(pixel1);

			if (S2636_IS_PIXEL_DRAWN(pixel2))
				bitmap.pix16(y, x) = S2636_PIXEL_COLOR(pixel2);

			if (S2636_IS_PIXEL_DRAWN(pixel3))
				bitmap.pix16(y, x) = S2636_PIXEL_COLOR(pixel3);
		}
	}

	if (state->m_sprite_enable)
		drawgfx_transpen(bitmap,cliprect,screen.machine().gfx[1],
		        state->m_sprite_code,
				state->m_sprite_color,
				0,0,
				state->m_sprite_x - 6,state->m_sprite_y,0);

	return 0;
}

/* Laser Battle sound **********************************/

static const sn76477_interface laserbat_sn76477_interface =
{
	RES_K(47),		/*  4 noise_res         R21    47K */
	0,				/*  5 filter_res (variable) */
	CAP_P(1000),	/*  6 filter_cap        C21    1000 pF */
	0,				/*  7 decay_res         */
	0,				/*  8 attack_decay_cap  */
	0,				/* 10 attack_res        */
	RES_K(47),		/* 11 amplitude_res     R26    47K */
	0,				/* 12 feedback_res (variable) */
	5.0 * RES_K(2.2) / (RES_K(2.2) + RES_K(4.7)),	/* 16  vco_voltage       */
	0,				/* 17 vco_cap           */
	0,				/* 18 vco_res (variable) */
	5.0,			/* 19 pitch_voltage     */
	0,				/* 20 slf_res (variable) */
	CAP_U(4.7),		/* 21 slf_cap           C24    4.7 uF */
	0,				/* 23 oneshot_cap       */
	0,				/* 24 oneshot_res       */
	0,			    /* 22 vco (variable) */
	0,			    /* 26 mixer A           */
	0,			    /* 25 mixer B (variable) */
	0,			    /* 27 mixer C           */
	0,			    /* 1  envelope 1        */
	1,			    /* 28 envelope 2        */
	1			    /* 9  enable (variable) */
};

/* Cat'N Mouse sound ***********************************/

WRITE_LINE_MEMBER(laserbat_state::zaccaria_irq0a)
{
	device_set_input_line(m_audiocpu, INPUT_LINE_NMI, state ? ASSERT_LINE : CLEAR_LINE);
}

WRITE_LINE_MEMBER(laserbat_state::zaccaria_irq0b)
{
	device_set_input_line(m_audiocpu, 0, state ? ASSERT_LINE : CLEAR_LINE);
}

READ8_MEMBER(laserbat_state::zaccaria_port0a_r)
{
	device_t *ay = (m_active_8910 == 0) ? m_ay1 : m_ay2;
	return ay8910_r(ay, 0);
}

WRITE8_MEMBER(laserbat_state::zaccaria_port0a_w)
{
	m_port0a = data;
}

WRITE8_MEMBER(laserbat_state::zaccaria_port0b_w)
{
	/* bit 1 goes to 8910 #0 BDIR pin  */
	if ((m_last_port0b & 0x02) == 0x02 && (data & 0x02) == 0x00)
	{
		/* bit 0 goes to the 8910 #0 BC1 pin */
		ay8910_data_address_w(m_ay1, m_last_port0b >> 0, m_port0a);
	}
	else if ((m_last_port0b & 0x02) == 0x00 && (data & 0x02) == 0x02)
	{
		/* bit 0 goes to the 8910 #0 BC1 pin */
		if (m_last_port0b & 0x01)
			m_active_8910 = 0;
	}
	/* bit 3 goes to 8910 #1 BDIR pin  */
	if ((m_last_port0b & 0x08) == 0x08 && (data & 0x08) == 0x00)
	{
		/* bit 2 goes to the 8910 #1 BC1 pin */
		ay8910_data_address_w(m_ay2, m_last_port0b >> 2, m_port0a);
	}
	else if ((m_last_port0b & 0x08) == 0x00 && (data & 0x08) == 0x08)
	{
		/* bit 2 goes to the 8910 #1 BC1 pin */
		if (m_last_port0b & 0x04)
			m_active_8910 = 1;
	}

	m_last_port0b = data;
}

static const pia6821_interface pia_intf =
{
	DEVCB_DRIVER_MEMBER(laserbat_state,zaccaria_port0a_r),		/* port A in */
	DEVCB_NULL,		/* port B in */
	DEVCB_NULL,		/* line CA1 in */
	DEVCB_NULL,		/* line CB1 in */
	DEVCB_NULL,		/* line CA2 in */
	DEVCB_NULL,		/* line CB2 in */
	DEVCB_DRIVER_MEMBER(laserbat_state,zaccaria_port0a_w),		/* port A out */
	DEVCB_DRIVER_MEMBER(laserbat_state,zaccaria_port0b_w),		/* port B out */
	DEVCB_NULL,		/* line CA2 out */
	DEVCB_NULL,		/* port CB2 out */
	DEVCB_DRIVER_LINE_MEMBER(laserbat_state,zaccaria_irq0a),		/* IRQA */
	DEVCB_DRIVER_LINE_MEMBER(laserbat_state,zaccaria_irq0b)		/* IRQB */
};

static const ay8910_interface ay8910_config =
{
	AY8910_LEGACY_OUTPUT,
	AY8910_DEFAULT_LOADS,
	DEVCB_NULL,
	DEVCB_DRIVER_MEMBER(driver_device, soundlatch_byte_r),
	DEVCB_NULL,//ay8910_port0a_w,
	DEVCB_NULL
};


static INTERRUPT_GEN( laserbat_interrupt )
{
	device_set_input_line_and_vector(device, 0, HOLD_LINE, 0x0a);
}

static INTERRUPT_GEN( zaccaria_cb1_toggle )
{
	laserbat_state *state = device->machine().driver_data<laserbat_state>();

	state->m_pia->cb1_w(state->m_cb1_toggle & 1);
	state->m_cb1_toggle ^= 1;
}


static const s2636_interface s2636_1_config =
{
	"screen",
	0x100,
	0, -19,
	NULL
};

static const s2636_interface s2636_2_config =
{
	"screen",
	0x100,
	0, -19,
	NULL
};

static const s2636_interface s2636_3_config =
{
	"screen",
	0x100,
	0, -19,
	NULL
};

static MACHINE_START( laserbat )
{
	laserbat_state *state = machine.driver_data<laserbat_state>();

	state->m_s2636_1 = machine.device("s2636_1");
	state->m_s2636_2 = machine.device("s2636_2");
	state->m_s2636_3 = machine.device("s2636_3");
	state->m_pia = machine.device<pia6821_device>("pia");
	state->m_sn = machine.device("snsnd");
	state->m_tms1 = machine.device("tms1");
	state->m_tms2 = machine.device("tms2");
	state->m_ay1 = machine.device("ay1");
	state->m_ay2 = machine.device("ay2");

	state->save_item(NAME(state->m_video_page));
	state->save_item(NAME(state->m_input_mux));
	state->save_item(NAME(state->m_active_8910));
	state->save_item(NAME(state->m_port0a));
	state->save_item(NAME(state->m_last_port0b));
	state->save_item(NAME(state->m_cb1_toggle));
	state->save_item(NAME(state->m_sprite_x));
	state->save_item(NAME(state->m_sprite_y));
	state->save_item(NAME(state->m_sprite_code));
	state->save_item(NAME(state->m_sprite_color));
	state->save_item(NAME(state->m_sprite_enable));
	state->save_item(NAME(state->m_csound1));
	state->save_item(NAME(state->m_ksound1));
	state->save_item(NAME(state->m_ksound2));
	state->save_item(NAME(state->m_ksound3));
	state->save_item(NAME(state->m_degr));
	state->save_item(NAME(state->m_filt));
	state->save_item(NAME(state->m_a));
	state->save_item(NAME(state->m_us));
	state->save_item(NAME(state->m_bit14));
}

static MACHINE_RESET( laserbat )
{
	laserbat_state *state = machine.driver_data<laserbat_state>();

	state->m_video_page = 0;
	state->m_input_mux = 0;
	state->m_active_8910 = 0;
	state->m_port0a = 0;
	state->m_last_port0b = 0;
	state->m_cb1_toggle = 0;
	state->m_sprite_x = 0;
	state->m_sprite_y = 0;
	state->m_sprite_code = 0;
	state->m_sprite_color = 0;
	state->m_sprite_enable = 0;
	state->m_csound1 = 0;
	state->m_ksound1 = 0;
	state->m_ksound2 = 0;
	state->m_ksound3 = 0;
	state->m_degr = 0;
	state->m_filt = 0;
	state->m_a = 0;
	state->m_us = 0;
	state->m_bit14 = 0;
}

static MACHINE_CONFIG_START( laserbat, laserbat_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", S2650, 14318180/4) // ???
	MCFG_CPU_PROGRAM_MAP(laserbat_map)
	MCFG_CPU_IO_MAP(laserbat_io_map)
	MCFG_CPU_VBLANK_INT("screen", laserbat_interrupt)

	MCFG_MACHINE_START(laserbat)
	MCFG_MACHINE_RESET(laserbat)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(50)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500) /* not accurate */)
	MCFG_SCREEN_SIZE(256, 256)
	MCFG_SCREEN_VISIBLE_AREA(1*8, 29*8-1, 2*8, 32*8-1)
	MCFG_SCREEN_UPDATE_STATIC(laserbat)

	MCFG_GFXDECODE(laserbat)
	MCFG_PALETTE_LENGTH(1024)

	MCFG_S2636_ADD("s2636_1", s2636_1_config)
	MCFG_S2636_ADD("s2636_2", s2636_2_config)
	MCFG_S2636_ADD("s2636_3", s2636_3_config)

	MCFG_VIDEO_START(laserbat)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("snsnd", SN76477, 0) // output not connected
	MCFG_SOUND_CONFIG(laserbat_sn76477_interface)

	MCFG_SOUND_ADD("tms1", TMS3615, 4000000/8/2) // 250 kHz, from second chip's clock out
	MCFG_SOUND_ROUTE(TMS3615_FOOTAGE_8, "mono", 1.0)

	MCFG_SOUND_ADD("tms2", TMS3615, 4000000/8) // 500 kHz
	MCFG_SOUND_ROUTE(TMS3615_FOOTAGE_8, "mono", 1.0)
MACHINE_CONFIG_END

static MACHINE_CONFIG_START( catnmous, laserbat_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", S2650, 14318000/4)	/* ? */
	MCFG_CPU_PROGRAM_MAP(laserbat_map)
	MCFG_CPU_IO_MAP(catnmous_io_map)
	MCFG_CPU_VBLANK_INT("screen", laserbat_interrupt)

	MCFG_CPU_ADD("audiocpu", M6802,3580000) /* ? */
	MCFG_CPU_PROGRAM_MAP(catnmous_sound_map)
	MCFG_CPU_PERIODIC_INT(zaccaria_cb1_toggle, (double)3580000/4096)

	MCFG_PIA6821_ADD("pia", pia_intf)

	MCFG_MACHINE_START(laserbat)
	MCFG_MACHINE_RESET(laserbat)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500) /* not accurate */)
	MCFG_SCREEN_SIZE(256, 256)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 2*8, 32*8-1)
	MCFG_SCREEN_UPDATE_STATIC(laserbat)

	MCFG_GFXDECODE(laserbat)
	MCFG_PALETTE_LENGTH(1024)

	MCFG_S2636_ADD("s2636_1", s2636_1_config)
	MCFG_S2636_ADD("s2636_2", s2636_2_config)
	MCFG_S2636_ADD("s2636_3", s2636_3_config)

	MCFG_VIDEO_START(laserbat)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("ay1", AY8910, 3580000/2) // ?
	MCFG_SOUND_CONFIG(ay8910_config)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)

	MCFG_SOUND_ADD("ay2", AY8910, 3580000/2) // ?
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_CONFIG_END


/*

Main cpu : 2650 signetics
quartz   : can't read it
Sub      : 2636 signetics (3 pieces)
ram      : 2114 (6 pieces in total)
special  : 82s100
special  : 2621N

Sound board info :

TMS SN76477N
TMS 3615NS-28 (x2)
Xtal : 4.000 Mhz

+ a few usual 74 chips

*/

ROM_START( laserbat )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "lb02.7c",      0x0000, 0x0400, CRC(23a257cd) SHA1(08d9e1ff1a5cd8a5e5af6a12ba6104d3b2ccfddf) )
	ROM_CONTINUE(			  0x4000, 0x0400 )
	ROM_LOAD( "lb02.6c",      0x0400, 0x0400, CRC(d1d6a67a) SHA1(727898c733633daffb0193cf4a556f89fe7e8a5a) )
	ROM_CONTINUE(			  0x4400, 0x0400 )
	ROM_LOAD( "lb02.5c",      0x0800, 0x0400, CRC(8116f1d3) SHA1(f84ace44434c55ca5d0be9f0beb2d4df75694b2f) )
	ROM_CONTINUE(			  0x4800, 0x0400 )
	ROM_LOAD( "lb02.3c",      0x0c00, 0x0400, CRC(443ef61e) SHA1(2849af0551bba7be2b4792739e04f18d6ace254c) )
	ROM_CONTINUE(			  0x4c00, 0x0400 )
	ROM_LOAD( "lb02.2c",      0x1000, 0x0400, CRC(0cb8f5f1) SHA1(4ce22c5ae277033cb9905339d24cad272a878088) )
	ROM_CONTINUE(			  0x5000, 0x0400 )
	ROM_LOAD( "lb02.7b",      0x2000, 0x0400, CRC(bdc769d1) SHA1(1291c159e779187efbdc3eb4a59a57d8d25ce08e) )
	ROM_CONTINUE(			  0x6000, 0x0400 )
	ROM_LOAD( "lb02.6b",      0x2400, 0x0400, CRC(2103646f) SHA1(bbd15a19524aeb8647014914a0b3025a975dfe7c) )
	ROM_CONTINUE(			  0x6400, 0x0400 )
	ROM_LOAD( "lb02.5b",      0x2800, 0x0400, CRC(3f8c4246) SHA1(b0d5e3733327140f54ac5a93f3f14d4afe085514) )
	ROM_CONTINUE(			  0x6800, 0x0400 )
	ROM_LOAD( "lb02.3b",      0x2c00, 0x0400, CRC(3e557d52) SHA1(860046fcc2d952f3e677e576f1ac23deac2e7caf) )
	ROM_CONTINUE(			  0x6c00, 0x0400 )
	ROM_LOAD( "lb02.2b",      0x3000, 0x0400, CRC(39000248) SHA1(58c6d1c588f4d1a3f579fe14faa8d2ccdfdc001e) )
	ROM_CONTINUE(			  0x7000, 0x0400 )

	ROM_REGION( 0x1800, "gfx1", 0 )
	ROM_LOAD( "lb02.8g",      0x0000, 0x0800, CRC(4bb9f452) SHA1(1ff4ef94f0da3b59377548f3341b083af83f83c6) )
	ROM_LOAD( "lb02.10g",     0x0800, 0x0800, CRC(5fec6517) SHA1(868e57e8498cf1ab0fa3635845cdb5800fd96855) )
	ROM_LOAD( "lb02.11g",     0x1000, 0x0800, CRC(ceaf00a4) SHA1(2e789898207caa7619dcbb01f52c3532d1482618) )

	ROM_REGION( 0x0800, "gfx2", 0 )
	ROM_LOAD( "lb02.14l",     0x0000, 0x0800, CRC(d29962d1) SHA1(5b6d0856c3ebbd5833b522f7c0240309cf3c9777) )

	ROM_REGION( 0x0100, "plds", 0 )
	ROM_LOAD( "82s100_prom",  0x0000, 0x00f5, NO_DUMP )
ROM_END

ROM_START( lazarian )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "laz.7c",      0x0000, 0x0400, CRC(a2454cf2) SHA1(163b9323e77ee0107e13860b3468e002c335df9e) )
	ROM_CONTINUE(			 0x4000, 0x0400 )
	ROM_LOAD( "laz.6c",      0x0400, 0x0400, CRC(23ee6013) SHA1(7ad53d6c321b0161906a512f6575620fd049d2f7) )
	ROM_CONTINUE(			 0x4400, 0x0400 )
	ROM_LOAD( "laz.5c",      0x0800, 0x0400, CRC(4234a2ed) SHA1(dc98b04ae7dd1c35687bd8bdf42e8feb5eed321d) )
	ROM_CONTINUE(			 0x4800, 0x0400 )
	ROM_LOAD( "laz.3c",      0x0c00, 0x0400, CRC(e901a636) SHA1(86320181a4d697fedfe8d8cbf9189854781e3d8c) )
	ROM_CONTINUE(			 0x4c00, 0x0400 )
	ROM_LOAD( "laz.2c",      0x1000, 0x0400, CRC(657ed7c2) SHA1(8611912001d18af8c932efc7700c0d8b60efb2e8) )
	ROM_CONTINUE(			 0x5000, 0x0400 )
	ROM_LOAD( "laz.7b",      0x2000, 0x0400, CRC(43135808) SHA1(2b704ca2f7a0fc46fddd5d7fb7d832a29d0562d0) )
	ROM_CONTINUE(			 0x6000, 0x0400 )
	ROM_LOAD( "laz.6b",      0x2400, 0x0400, CRC(95701e50) SHA1(61d6a268696cefb760bf288bcc4eab7ac5f32ec7) )
	ROM_CONTINUE(			 0x6400, 0x0400 )
	ROM_LOAD( "laz.5b",      0x2800, 0x0400, CRC(685842ba) SHA1(ee842d1d2c0676fddddf6e4e9cfd0b2962ae900d) )
	ROM_CONTINUE(			 0x6800, 0x0400 )
	ROM_LOAD( "laz.3b",      0x2c00, 0x0400, CRC(9ddbe048) SHA1(70d1e8af073c85aba08e5251691842069617e6ac) )
	ROM_CONTINUE(			 0x6c00, 0x0400 )
	ROM_LOAD( "laz10-62.2b", 0x3800, 0x0400, CRC(4ad9f7af) SHA1(71bcb9d148a7372b7be0abccdf71eeedba8b6c0a) )
	ROM_CONTINUE(			 0x7800, 0x0400 )
	ROM_CONTINUE(			 0x3000, 0x0400 )
	ROM_CONTINUE(			 0x7000, 0x0400 )

	ROM_REGION( 0x1800, "gfx1", 0 )
	ROM_LOAD( "laz.8g",      0x0000, 0x0800, CRC(3cf76c01) SHA1(1824bc05e8dd2a522409e95fe81d2ad64182dcac) )
	ROM_LOAD( "laz.10g",     0x0800, 0x0800, CRC(256ae65d) SHA1(7f9e8ea1bbcb9e2175544556795c88c9981db571) )
	ROM_LOAD( "laz.11g",     0x1000, 0x0800, CRC(fec8266a) SHA1(7b90ae8d9eeb148012cca1bc93546dc3bf509258) )

	ROM_REGION( 0x0800, "gfx2", 0 )
	ROM_LOAD( "laz.14l",      0x0000, 0x0800, CRC(d29962d1) SHA1(5b6d0856c3ebbd5833b522f7c0240309cf3c9777) )

	ROM_REGION( 0x0100, "plds", 0 )
	ROM_LOAD( "lz82s100.10m", 0x0000, 0x00f5, CRC(c3eb562a) SHA1(65dff81b2e5321d530e5171dab9aa3809ab38b4d) )
ROM_END

/*
Zaccaria "Cat 'N Mouse" 1982

similar to "Quasar" execept it uses an 82s100 for color table lookup
and has a larger program prom


Cat N Mouse (Zaccaria 1982)

CPU Board

               2650    7b 6b 5b 3b 2b
                       7c 6c 5c 3c 2c

                       2636 2636 2636
        11g 10g 8g
     14l
                  clr

Sound Board 1b11107

6802
6821
8910
*/

ROM_START( catnmous )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "02-1.7c",      0x0000, 0x0400, CRC(d26ec566) SHA1(ceb16f64a3c1ff25a9eab6549f1ae24085bb9e27) )
	ROM_CONTINUE(			  0x4000, 0x0400 )
	ROM_LOAD( "02-2.6c",      0x0400, 0x0400, CRC(02a7e36c) SHA1(8495b2906ecb0791a47e9b6f1959ed6cbc14cce8) )
	ROM_CONTINUE(			  0x4400, 0x0400 )
	ROM_LOAD( "02-3.5c",      0x0800, 0x0400, CRC(ee9f90ee) SHA1(dc280dae3a18a9044497bdee41827d2510a04d06) )
	ROM_CONTINUE(			  0x4800, 0x0400 )
	ROM_LOAD( "02-4.3c",      0x0c00, 0x0400, CRC(71b97af9) SHA1(6735184dc16c8db3050be3b7b5dfdb7d46a671fe) )
	ROM_CONTINUE(			  0x4c00, 0x0400 )
	ROM_LOAD( "02-5.2c",      0x1000, 0x0400, CRC(887a1da2) SHA1(9e2548d1792c2d2b76811a1e0daae4d378f1f354) )
	ROM_CONTINUE(			  0x5000, 0x0400 )
	ROM_LOAD( "02-6.7b",      0x2000, 0x0400, CRC(22e045e9) SHA1(dd332e918500d8024d1329bc12c6f939fd41e4a7) )
	ROM_CONTINUE(			  0x6000, 0x0400 )
	ROM_LOAD( "02-7.6b",      0x2400, 0x0400, CRC(af330ad2) SHA1(cac70341687edd1daee323c0e332297c80057e1e) )
	ROM_CONTINUE(			  0x6400, 0x0400 )
	ROM_LOAD( "02-8.5b",      0x2800, 0x0400, CRC(c7d38401) SHA1(33a3bb393451cd3fefa23b5c8013068b5b0de7a5) )
	ROM_CONTINUE(			  0x6800, 0x0400 )
	ROM_LOAD( "02-9.3b",      0x2c00, 0x0400, CRC(c4a33f20) SHA1(355c4345daa681fa2bcfa1e345d2db34f9d94113) )
	ROM_CONTINUE(			  0x6c00, 0x0400 )
	ROM_LOAD( "02-10-11.2b",  0x3800, 0x0400, CRC(3f7d4b89) SHA1(c8e9be0149a2f728526a416ec5663e69cc2e6758) )
	ROM_CONTINUE(			  0x7800, 0x0400 )
	ROM_CONTINUE(			  0x3000, 0x0400 )
	ROM_CONTINUE(			  0x7000, 0x0400 )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "sound01.1d",   0xd000, 0x1000, CRC(f65cb9d0) SHA1(a2fe7563c6da055bf6aa20797b2d9fa184f0133c) )
	ROM_LOAD( "sound01.1f",   0xe000, 0x1000, CRC(473c44de) SHA1(ff08b02d45a2c23cabb5db716aa203225a931424) )
	ROM_LOAD( "sound01.1e",   0xf000, 0x1000, CRC(1bd90c93) SHA1(20fd2b765a42e25cf7f716e6631b8c567785a866) )

	ROM_REGION( 0x1800, "gfx1", 0 )
	ROM_LOAD( "type01.8g",    0x0000, 0x0800, CRC(2b180d4a) SHA1(b6f48ffdbad64b4d9f1fe838000187800c51228c) )
	ROM_LOAD( "type01.10g",   0x0800, 0x0800, CRC(e5259f9b) SHA1(396753291ab36c3ed72208d619665fc0f33d1e17) )
	ROM_LOAD( "type01.11g",   0x1000, 0x0800, CRC(2999f378) SHA1(929082383b2b0006de171587adb932ce57316963) )

	ROM_REGION( 0x0800, "gfx2", 0 )
	ROM_LOAD( "type01.14l",   0x0000, 0x0800, CRC(af79179a) SHA1(de61af7d02c93be326a33ee51572e3da7a25dab0) )

	ROM_REGION( 0x0100, "plds", 0 )
	ROM_LOAD( "82s100.13m",   0x0000, 0x00f5, CRC(6b724cdb) SHA1(8a0ca3b171b103661a3b2fffbca3d7162089e243) )
ROM_END

ROM_START( catnmousa )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "catnmous.7c",  0x0000, 0x0400, CRC(0bf9fc06) SHA1(7d5857121fe51f43e4ae7db34df720198994afdd) )
	ROM_CONTINUE(			  0x4000, 0x0400 )
	ROM_LOAD( "catnmous.6c",  0x0400, 0x0400, CRC(b0e140a0) SHA1(68d8ca25642e872f2177d09b78d553c033411dd5) )
	ROM_CONTINUE(			  0x4400, 0x0400 )
	ROM_LOAD( "catnmous.5c",  0x0800, 0x0400, CRC(7bbc0fe5) SHA1(d20e89d89a0958d45ac31b6d2c540fcf3d326068) )
	ROM_CONTINUE(			  0x4800, 0x0400 )
	ROM_LOAD( "catnmous.3c",  0x0c00, 0x0400, CRC(0350531d) SHA1(6115f907544ab317e0090a10cce3adce26f4afd9) )
	ROM_CONTINUE(			  0x4c00, 0x0400 )
	ROM_LOAD( "catnmous.2c",  0x1000, 0x0400, CRC(4a26e963) SHA1(be8dd98d3810319a228ce4c07b097eb75f2d1e5c) )
	ROM_CONTINUE(			  0x5000, 0x0400 )
	ROM_LOAD( "catnmous.7b",  0x2000, 0x0400, CRC(d8d6a029) SHA1(7e5688fd3af97620ed07d9375335fe1deb6e483f) )
	ROM_CONTINUE(			  0x6000, 0x0400 )
	ROM_LOAD( "catnmous.6b",  0x2400, 0x0400, CRC(ccc871d9) SHA1(355eff250ab3d1a75ed690369add1639e7061ee8) )
	ROM_CONTINUE(			  0x6400, 0x0400 )
	ROM_LOAD( "catnmous.5b",  0x2800, 0x0400, CRC(23783b84) SHA1(97a3ef7c64e1ded5cc1999d3aa58652ca541166c) )
	ROM_CONTINUE(			  0x6800, 0x0400 )
	ROM_LOAD( "catnmous.3b",  0x2c00, 0x0400, CRC(e99fce4b) SHA1(2c8efdea55bae5526b547fec53e8f3642fe2bd2e) )
	ROM_CONTINUE(			  0x6c00, 0x0400 )
	// missing half rom
	ROM_LOAD( "catnmous.2b",  0x3000, 0x0400, BAD_DUMP CRC(880728fa) SHA1(f204d669c190ad0cf2c885af12625026534db655) )
	ROM_CONTINUE(			  0x7000, 0x0400 )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "snd.1d",       0xd000, 0x1000, CRC(f65cb9d0) SHA1(a2fe7563c6da055bf6aa20797b2d9fa184f0133c) )
	ROM_LOAD( "snd.1f",       0xe000, 0x1000, CRC(473c44de) SHA1(ff08b02d45a2c23cabb5db716aa203225a931424) )
	ROM_LOAD( "snd.1e",       0xf000, 0x1000, CRC(1bd90c93) SHA1(20fd2b765a42e25cf7f716e6631b8c567785a866) )

	ROM_REGION( 0x1800, "gfx1", 0 )
	ROM_LOAD( "catnmous.8g",  0x0000, 0x0800, CRC(2b180d4a) SHA1(b6f48ffdbad64b4d9f1fe838000187800c51228c) )
	ROM_LOAD( "catnmous.10g", 0x0800, 0x0800, CRC(e5259f9b) SHA1(396753291ab36c3ed72208d619665fc0f33d1e17) )
	ROM_LOAD( "catnmous.11g", 0x1000, 0x0800, CRC(2999f378) SHA1(929082383b2b0006de171587adb932ce57316963) )

	ROM_REGION( 0x0800, "gfx2", 0 )
	ROM_LOAD( "catnmous.14l", 0x0000, 0x0800, CRC(af79179a) SHA1(de61af7d02c93be326a33ee51572e3da7a25dab0) )

	ROM_REGION( 0x0100, "plds", 0 )
	ROM_LOAD( "catnmousa_82s100.13m", 0x0000, 0x00f5, NO_DUMP )
ROM_END


GAME( 1981, laserbat, 0,        laserbat, laserbat, driver_device, 0, ROT0,  "Zaccaria", "Laser Battle",                    GAME_IMPERFECT_SOUND | GAME_WRONG_COLORS | GAME_NO_COCKTAIL | GAME_SUPPORTS_SAVE )
GAME( 1981, lazarian, laserbat, laserbat, lazarian, driver_device, 0, ROT0,  "Zaccaria (Bally Midway license)", "Lazarian", GAME_IMPERFECT_SOUND | GAME_WRONG_COLORS | GAME_NO_COCKTAIL | GAME_SUPPORTS_SAVE )
GAME( 1982, catnmous, 0,        catnmous, catnmous, driver_device, 0, ROT90, "Zaccaria", "Cat and Mouse (set 1)",           GAME_NO_SOUND | GAME_WRONG_COLORS | GAME_NO_COCKTAIL | GAME_SUPPORTS_SAVE)
GAME( 1982, catnmousa,catnmous, catnmous, catnmous, driver_device, 0, ROT90, "Zaccaria", "Cat and Mouse (set 2)",           GAME_NO_SOUND | GAME_WRONG_COLORS | GAME_NO_COCKTAIL | GAME_NOT_WORKING | GAME_SUPPORTS_SAVE)
