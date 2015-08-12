// license:BSD-3-Clause
// copyright-holders:Mirko Buffoni
/***************************************************************************

Senjyo / Star Force / Baluba-louk

driver by Mirko Buffoni

TODO:
- wrong background colors in baluba, intermissions after round 13 (btanb or
  fixed at some point)
- Star Force: the Larios is supposed to blink at the third loop of the BGM.
  Right now it does at second and half, presumably due of the unknown PCB clocks.

Note:
- Star Force shows default MAME palette at POST. Flipped to all_black for now.

This board was obviously born to run Senjyo. Four scrolling layers, gradient
background, sprite/background priorities, and even a small bitmap for the
radar. Star Force uses only a small subset of the features.

MAIN BOARD:
0000-7fff ROM
8000-8fff RAM
9000-93ff Video RAM
9400-97ff Color RAM
9800-987f Sprites
9c00-9dff Palette RAM
a000-a37f Background Video RAM #3
a800-aaff Background Video RAM #2
b000-b1ff Background Video RAM #1
b800-bbff Radar bitmap

read:
d000      IN0
d001      IN1
d002      IN2
d003      ?
d004      DSW1
d005      DSW2

write:
9e20-9e21 background #1 x position
9e25      background #1 y position
9e28-9e29 background #? x position ??
9e30-9e31 background #2 & #3 x position
9e35      background #2 & #3 y position
d000      flip screen
d002      watchdog reset?
          IN0/IN1 latch ? ( write before read IN0/IN1 )
d004      sound command ( pio-a )

SOUND BOARD
memory read/write
0000-3fff ROM
4000-43ff RAM

write
8000 sound chip channel 1 1st 9f,bf,df,ff
9000 sound chip channel 2 1st 9f,bf,df,ff
a000 sound chip channel 3 1st 9f,bf,df,ff
d000 bit 0-3 single sound volume ( freq = ctc2 )
e000 ? ( initialize only )
f000 ? ( initialize only )

I/O read/write
00   z80pio-A data     ( from sound command )
01   z80pio-A controll ( mode 1 input )
02   z80pio-B data     ( no use )
03   z80pio-B controll ( mode 3 bit i/o )
08   z80ctc-ch1        ( timer mode cysclk/16, bas clock 15.625KHz )
09   z80ctc-ch2        ( cascade from ctc-1  , tempo interrupt 88.778Hz )
0a   z80ctc-ch3        ( timer mode , single sound freq. )
0b   z80ctc-ch4        ( no use )

***************************************************************************/

/* 26.February 2012 Tsuyoshi Hasegawa fixed palette intensity */

#include "emu.h"
#include "cpu/z80/z80.h"
#include "sound/sn76496.h"
#include "machine/segacrpt.h"
#include "includes/senjyo.h"

void senjyo_state::machine_start()
{
	save_item(NAME(m_sound_cmd));
	save_item(NAME(m_single_volume));
	save_item(NAME(m_sound_state));
	save_item(NAME(m_bgstripes));
}

void senjyo_state::machine_reset()
{
	m_sound_cmd = 0;
	m_single_volume = 0;
	m_sound_state = 0;
	m_bgstripes = 0;
}

WRITE8_MEMBER(senjyo_state::irq_ctrl_w)
{
	// irq ack is mandatory for senjyo: it's basically used as an irq mask during POST.
	m_maincpu->set_input_line(0, CLEAR_LINE);
}

WRITE8_MEMBER(senjyo_state::flip_screen_w)
{
	flip_screen_set(data);
}

WRITE8_MEMBER(senjyo_state::sound_cmd_w)
{
	m_sound_cmd = data;

	m_pio->strobe_a(0);
	m_pio->strobe_a(1);
}

static ADDRESS_MAP_START( senjyo_map, AS_PROGRAM, 8, senjyo_state )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0x8000, 0x8fff) AM_RAM
	AM_RANGE(0x9000, 0x93ff) AM_RAM_WRITE(fgvideoram_w) AM_SHARE("fgvideoram")
	AM_RANGE(0x9400, 0x97ff) AM_RAM_WRITE(fgcolorram_w) AM_SHARE("fgcolorram")
	AM_RANGE(0x9800, 0x987f) AM_RAM AM_SHARE("spriteram")
	AM_RANGE(0x9c00, 0x9dff) AM_RAM_DEVWRITE("palette", palette_device, write) AM_SHARE("palette")
	AM_RANGE(0x9e00, 0x9e1f) AM_RAM AM_SHARE("fgscroll")
	AM_RANGE(0x9e20, 0x9e21) AM_RAM AM_SHARE("scrolly3")
/*  AM_RANGE(0x9e22, 0x9e23) height of the layer (Senjyo only, fixed at 0x380) */
	AM_RANGE(0x9e25, 0x9e25) AM_RAM AM_SHARE("scrollx3")
	AM_RANGE(0x9e27, 0x9e27) AM_RAM AM_SHARE("bgstripesram")  /* controls width of background stripes */
	AM_RANGE(0x9e28, 0x9e29) AM_RAM AM_SHARE("scrolly2")
/*  AM_RANGE(0x9e2a, 0x9e2b) height of the layer (Senjyo only, fixed at 0x200) */
	AM_RANGE(0x9e2d, 0x9e2d) AM_RAM AM_SHARE("scrollx2")
	AM_RANGE(0x9e30, 0x9e31) AM_RAM AM_SHARE("scrolly1")
/*  AM_RANGE(0x9e32, 0x9e33) height of the layer (Senjyo only, fixed at 0x100) */
	AM_RANGE(0x9e35, 0x9e35) AM_RAM AM_SHARE("scrollx1")
/*  AM_RANGE(0x9e38, 0x9e38) probably radar y position (Senjyo only, fixed at 0x61) */
/*  AM_RANGE(0x9e3d, 0x9e3d) probably radar x position (Senjyo only, 0x00/0xc0 depending on screen flip) */
	AM_RANGE(0x9e00, 0x9e3f) AM_RAM
	AM_RANGE(0xa000, 0xa7ff) AM_RAM_WRITE(bg3videoram_w) AM_SHARE("bg3videoram")
	AM_RANGE(0xa800, 0xafff) AM_RAM_WRITE(bg2videoram_w) AM_SHARE("bg2videoram")
	AM_RANGE(0xb000, 0xb7ff) AM_RAM_WRITE(bg1videoram_w) AM_SHARE("bg1videoram")
	AM_RANGE(0xb800, 0xbbff) AM_RAM AM_SHARE("radarram")
	AM_RANGE(0xd000, 0xd000) AM_READ_PORT("P1") AM_WRITE(flip_screen_w)
	AM_RANGE(0xd001, 0xd001) AM_READ_PORT("P2")
	AM_RANGE(0xd002, 0xd002) AM_READ_PORT("SYSTEM") AM_WRITE(irq_ctrl_w)
	AM_RANGE(0xd003, 0xd003) AM_READNOP // debug cheat port? (i.e. bit 0 in starforc: invincibility, bit 3-0 in senyjo: disables enemy fire)
	AM_RANGE(0xd004, 0xd004) AM_READ_PORT("DSW1") AM_WRITE(sound_cmd_w)
	AM_RANGE(0xd005, 0xd005) AM_READ_PORT("DSW2")
ADDRESS_MAP_END

static ADDRESS_MAP_START( decrypted_opcodes_map, AS_DECRYPTED_OPCODES, 8, senjyo_state )
	AM_RANGE(0x0000, 0x7fff) AM_ROM AM_SHARE("decrypted_opcodes")
ADDRESS_MAP_END

static ADDRESS_MAP_START( senjyo_sound_map, AS_PROGRAM, 8, senjyo_state )
	AM_RANGE(0x0000, 0x1fff) AM_ROM
	AM_RANGE(0x4000, 0x43ff) AM_RAM
	AM_RANGE(0x8000, 0x8000) AM_DEVWRITE("sn1", sn76496_device, write)
	AM_RANGE(0x9000, 0x9000) AM_DEVWRITE("sn2", sn76496_device, write)
	AM_RANGE(0xa000, 0xa000) AM_DEVWRITE("sn3", sn76496_device, write)
	AM_RANGE(0xd000, 0xd000) AM_WRITE(volume_w)
#if 0
	AM_RANGE(0xe000, 0xe000) AM_WRITE(unknown)
	AM_RANGE(0xf000, 0xf000) AM_WRITE(unknown)
#endif
ADDRESS_MAP_END

static ADDRESS_MAP_START( senjyo_sound_io_map, AS_IO, 8, senjyo_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x03) AM_DEVREADWRITE("z80pio", z80pio_device, read_alt, write_alt)
	AM_RANGE(0x08, 0x0b) AM_DEVREADWRITE("z80ctc", z80ctc_device, read, write)
ADDRESS_MAP_END

/* For the bootleg */

/* are scroll registers 1+2 linked on the bootleg?, only one copy is written */
WRITE8_MEMBER(senjyo_state::starforb_scrolly2)
{
	m_scrolly2[offset] = data;
	m_scrolly1[offset] = data;
}

WRITE8_MEMBER(senjyo_state::starforb_scrollx2)
{
	m_scrollx2[offset] = data;
	m_scrollx1[offset] = data;
}

static ADDRESS_MAP_START( starforb_map, AS_PROGRAM, 8, senjyo_state )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0x8000, 0x8fff) AM_RAM
	AM_RANGE(0x9000, 0x93ff) AM_RAM_WRITE(fgvideoram_w) AM_SHARE("fgvideoram")
	AM_RANGE(0x9400, 0x97ff) AM_RAM_WRITE(fgcolorram_w) AM_SHARE("fgcolorram")
	AM_RANGE(0x9800, 0x987f) AM_RAM AM_SHARE("spriteram")
	AM_RANGE(0x9c00, 0x9dff) AM_RAM_DEVWRITE("palette", palette_device, write) AM_SHARE("palette")
	/* The format / use of the ram here is different on the bootleg */
	AM_RANGE(0x9e20, 0x9e21) AM_RAM AM_SHARE("scrolly3")
	AM_RANGE(0x9e25, 0x9e25) AM_RAM AM_SHARE("scrollx3")
	AM_RANGE(0x9e30, 0x9e31) AM_RAM_WRITE(starforb_scrolly2) AM_SHARE("scrolly2") // ok
	AM_RANGE(0x9e35, 0x9e35) AM_RAM_WRITE(starforb_scrollx2) AM_SHARE("scrollx2") // ok
	AM_RANGE(0x9e00, 0x9e3f) AM_RAM

	AM_RANGE(0xa000, 0xa7ff) AM_RAM_WRITE(bg3videoram_w) AM_SHARE("bg3videoram")
	AM_RANGE(0xa800, 0xafff) AM_RAM_WRITE(bg2videoram_w) AM_SHARE("bg2videoram")
	AM_RANGE(0xb000, 0xb7ff) AM_RAM_WRITE(bg1videoram_w) AM_SHARE("bg1videoram")
	AM_RANGE(0xb800, 0xbbff) AM_RAM AM_SHARE("radarram")
	AM_RANGE(0xd000, 0xd000) AM_READ_PORT("P1") AM_WRITE(flip_screen_w)
	AM_RANGE(0xd001, 0xd001) AM_READ_PORT("P2")
	AM_RANGE(0xd002, 0xd002) AM_READ_PORT("SYSTEM") AM_WRITE(irq_ctrl_w)
	AM_RANGE(0xd004, 0xd004) AM_READ_PORT("DSW1") AM_WRITE(sound_cmd_w)
	AM_RANGE(0xd005, 0xd005) AM_READ_PORT("DSW2")

	/* these aren't used / written, left here to make sure memory is allocated */
	AM_RANGE(0xfe00, 0xfe1f) AM_RAM AM_SHARE("fgscroll")
	AM_RANGE(0xfe27, 0xfe27) AM_RAM AM_SHARE("bgstripesram")  /* controls width of background stripes */
	AM_RANGE(0xfe28, 0xfe29) AM_RAM AM_SHARE("scrolly1")
	AM_RANGE(0xfe2d, 0xfe2d) AM_RAM AM_SHARE("scrollx1")
ADDRESS_MAP_END


static ADDRESS_MAP_START( starforb_sound_map, AS_PROGRAM, 8, senjyo_state )
	AM_RANGE(0x0000, 0x1fff) AM_ROM
	AM_RANGE(0x4000, 0x43ff) AM_RAM
	AM_RANGE(0x8000, 0x8000) AM_DEVWRITE("sn1", sn76496_device, write)
	AM_RANGE(0x9000, 0x9000) AM_DEVWRITE("sn2", sn76496_device, write)
	AM_RANGE(0xa000, 0xa000) AM_DEVWRITE("sn3", sn76496_device, write)
	AM_RANGE(0xd000, 0xd000) AM_WRITE(volume_w)
#if 0
	AM_RANGE(0xe000, 0xe000) AM_WRITE(unknown)
	AM_RANGE(0xf000, 0xf000) AM_WRITE(unknown)
#endif
	AM_RANGE(0xf000, 0xffff) AM_RAM
ADDRESS_MAP_END


static INPUT_PORTS_START( senjyo )
	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("SYSTEM")
	/* coin input for both must be active between 2 and 9 frames to be consistently recognized */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 ) PORT_IMPULSE(2)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN2 ) PORT_IMPULSE(2)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_6C ) )
	PORT_DIPNAME( 0x0c, 0x00, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_3C ) )
	PORT_DIPNAME( 0x30, 0x00, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x30, "2" )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x10, "4" )
	PORT_DIPSETTING(    0x20, "5" )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x02, "100k only" )
	PORT_DIPSETTING(    0x00, DEF_STR( None ) )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0xc0, 0x40, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Medium ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( Hardest ) )
INPUT_PORTS_END

static INPUT_PORTS_START( starforc )
	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("SYSTEM")
	/* coin input for both must be active between 2 and 9 frames to be consistently recognized */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 ) PORT_IMPULSE(2)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN2 ) PORT_IMPULSE(2)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_3C ) )
	PORT_DIPNAME( 0x0c, 0x00, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_3C ) )
	PORT_DIPNAME( 0x30, 0x00, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x30, "2" )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x10, "4" )
	PORT_DIPSETTING(    0x20, "5" )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x07, 0x00, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x00, "50k, 200k and 500k" )
	PORT_DIPSETTING(    0x01, "100k, 300k and 800k" )
	PORT_DIPSETTING(    0x02, "50k and 200k" )
	PORT_DIPSETTING(    0x03, "100k and 300k" )
	PORT_DIPSETTING(    0x04, "50k only" )
	PORT_DIPSETTING(    0x05, "100k only" )
	PORT_DIPSETTING(    0x06, "200k only" )
	PORT_DIPSETTING(    0x07, DEF_STR( None ) )
	PORT_DIPNAME( 0x38, 0x00, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Easiest ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Medium ) )
	PORT_DIPSETTING(    0x18, DEF_STR( Difficult ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x28, DEF_STR( Hardest ) )
	/* 0x30 and x038 are unused */
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( baluba )
	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("SYSTEM")
	/* coin input for both must be active between 2 and 9 frames to be consistently recognized */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 ) PORT_IMPULSE(2)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN2 ) PORT_IMPULSE(2)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_3C ) )
	PORT_DIPNAME( 0x0c, 0x00, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_3C ) )
	PORT_DIPNAME( 0x30, 0x00, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x30, "2" )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x10, "4" )
	PORT_DIPSETTING(    0x20, "5" )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x07, 0x00, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x00, "30k, 100k and 200k" )
	PORT_DIPSETTING(    0x01, "50k, 200k and 500k" )
	PORT_DIPSETTING(    0x02, "30k and 100k" )
	PORT_DIPSETTING(    0x03, "50k and 200k" )
	PORT_DIPSETTING(    0x04, "30k only" )
	PORT_DIPSETTING(    0x05, "100k only" )
	PORT_DIPSETTING(    0x06, "200k only" )
	PORT_DIPSETTING(    0x07, DEF_STR( None ) )
	PORT_DIPNAME( 0x38, 0x00, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x00, "0" )
	PORT_DIPSETTING(    0x08, "1" )
	PORT_DIPSETTING(    0x10, "2" )
	PORT_DIPSETTING(    0x18, "3" )
	PORT_DIPSETTING(    0x20, "4" )
	PORT_DIPSETTING(    0x28, "5" )
	PORT_DIPSETTING(    0x30, "6" )
	PORT_DIPSETTING(    0x38, "7" )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )
INPUT_PORTS_END



static const gfx_layout charlayout =
{
	8,8,    /* 8*8 characters */
	RGN_FRAC(1,3),  /* 512 characters */
	3,  /* 3 bits per pixel */
	{ RGN_FRAC(0,3), RGN_FRAC(1,3), RGN_FRAC(2,3) },    /* the bitplanes are separated */
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8 /* every char takes 8 consecutive bytes */
};
static const gfx_layout tilelayout =
{
	16,16,  /* 16*16 characters */
	RGN_FRAC(1,3),  /* 256 characters */
	3,  /* 3 bits per pixel */
	{ RGN_FRAC(0,3), RGN_FRAC(1,3), RGN_FRAC(2,3) },    /* the bitplanes are separated */
	{ 0, 1, 2, 3, 4, 5, 6, 7,
			8*8+0, 8*8+1, 8*8+2, 8*8+3, 8*8+4, 8*8+5, 8*8+6, 8*8+7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8,
			16*8, 17*8, 18*8, 19*8, 20*8, 21*8, 22*8, 23*8 },
	32*8    /* every character takes 32 consecutive bytes */
};

static const gfx_layout spritelayout1 =
{
	16,16,  /* 16*16 sprites */
	RGN_FRAC(1,3),
	3,  /* 3 bits per pixel */
	{ RGN_FRAC(0,3), RGN_FRAC(1,3), RGN_FRAC(2,3) },    /* the bitplanes are separated */
	{ 0, 1, 2, 3, 4, 5, 6, 7,
			8*8+0, 8*8+1, 8*8+2, 8*8+3, 8*8+4, 8*8+5, 8*8+6, 8*8+7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8,
			16*8, 17*8, 18*8, 19*8, 20*8, 21*8, 22*8, 23*8 },
	32*8    /* every sprite takes 32 consecutive bytes */
};
static const gfx_layout spritelayout2 =
{
	32,32,  /* 32*32 sprites */
	RGN_FRAC(1,3),  /* 128 sprites */
	3,  /* 3 bits per pixel */
	{ RGN_FRAC(0,3), RGN_FRAC(1,3), RGN_FRAC(2,3) },    /* the bitplanes are separated */
	{ 0, 1, 2, 3, 4, 5, 6, 7,
			8*8+0, 8*8+1, 8*8+2, 8*8+3, 8*8+4, 8*8+5, 8*8+6, 8*8+7,
			32*8+0, 32*8+1, 32*8+2, 32*8+3, 32*8+4, 32*8+5, 32*8+6, 32*8+7,
			40*8+0, 40*8+1, 40*8+2, 40*8+3, 40*8+4, 40*8+5, 40*8+6, 40*8+7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8,
			16*8, 17*8, 18*8, 19*8, 20*8, 21*8, 22*8, 23*8,
			64*8, 65*8, 66*8, 67*8, 68*8, 69*8, 70*8, 71*8,
			80*8, 81*8, 82*8, 83*8, 84*8, 85*8, 86*8, 87*8 },
	128*8   /* every sprite takes 128 consecutive bytes */
};

static GFXDECODE_START( senjyo )
	GFXDECODE_ENTRY( "gfx1", 0, charlayout,       0, 8 )    /*   0- 63 characters */
	GFXDECODE_ENTRY( "gfx2", 0, tilelayout,  64, 8 )    /*  64-127 background #1 */
	GFXDECODE_ENTRY( "gfx3", 0, tilelayout, 128, 8 )    /* 128-191 background #2 */
	GFXDECODE_ENTRY( "gfx4", 0, tilelayout, 192, 8 )    /* 192-255 background #3 */
	GFXDECODE_ENTRY( "gfx5", 0, spritelayout1,  320, 8 )    /* 320-383 normal sprites */
	GFXDECODE_ENTRY( "gfx5", 0, spritelayout2,  320, 8 )    /* 320-383 large sprites */
													/* 384-399 is background */
GFXDECODE_END


static MACHINE_CONFIG_START( senjyo, senjyo_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, 4000000)   /* 4 MHz? */
	MCFG_CPU_PROGRAM_MAP(senjyo_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", senjyo_state, irq0_line_assert)

	MCFG_CPU_ADD("sub", Z80, 2000000)   /* 2 MHz? */
	MCFG_CPU_CONFIG(senjyo_daisy_chain)
	MCFG_CPU_PROGRAM_MAP(senjyo_sound_map)
	MCFG_CPU_IO_MAP(senjyo_sound_io_map)

	MCFG_DEVICE_ADD("z80pio", Z80PIO, 2000000)
	MCFG_Z80PIO_OUT_INT_CB(INPUTLINE("sub", INPUT_LINE_IRQ0))
	MCFG_Z80PIO_IN_PA_CB(READ8(senjyo_state, pio_pa_r))

	MCFG_DEVICE_ADD("z80ctc", Z80CTC, 2000000 /* same as "sub" */)
	MCFG_Z80CTC_INTR_CB(INPUTLINE("sub", INPUT_LINE_IRQ0))
	MCFG_Z80CTC_ZC0_CB(DEVWRITELINE("z80ctc", z80ctc_device, trg1))
	MCFG_Z80CTC_ZC2_CB(WRITELINE(senjyo_state, sound_line_clock))

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(32*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 2*8, 30*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(senjyo_state, screen_update)

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", senjyo)

	MCFG_PALETTE_ADD_INIT_BLACK("palette", 512)
	MCFG_PALETTE_FORMAT_CLASS(1, senjyo_state, IIBBGGRR)

	MCFG_PALETTE_ADD("radar_palette", 2)
	MCFG_PALETTE_INIT_OWNER(senjyo_state, radar)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("sn1", SN76496, 2000000)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)

	MCFG_SOUND_ADD("sn2", SN76496, 2000000)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)

	MCFG_SOUND_ADD("sn3", SN76496, 2000000)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)

	MCFG_DAC_ADD("dac")
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.05)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( senjyox, senjyo )
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_DECRYPTED_OPCODES_MAP(decrypted_opcodes_map)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( starforb, senjyox )

	/* basic machine hardware */
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(starforb_map)

	MCFG_CPU_MODIFY("sub")
	MCFG_CPU_PROGRAM_MAP(starforb_sound_map)
MACHINE_CONFIG_END


/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( senjyo )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "08m_05t.bin", 0x0000, 0x2000, CRC(b1f3544d) SHA1(59997164dfb740fce1862d89754be7517303161a) )
	ROM_LOAD( "08k_04t.bin", 0x2000, 0x2000, CRC(e34468a8) SHA1(1931788e4ebe0dab9525f795b639be6544a6b31a) )
	ROM_LOAD( "08j_03t.bin", 0x4000, 0x2000, CRC(c33aedee) SHA1(5adf83268ef7b91194dea41204bdb931a14f2158) )
	ROM_LOAD( "08f_02t.bin", 0x6000, 0x2000, CRC(0ef4db9e) SHA1(0dcb216495f4328c44cc0af87ffb0bd255b7dc1a) )

	ROM_REGION( 0x2000, "sub", 0 )
	ROM_LOAD( "02h_01t.bin", 0x0000, 0x2000, CRC(c1c24455) SHA1(24a2ab9e4df793f68f51bbe6a1313f38d951a8af) )

	ROM_REGION( 0x03000, "gfx1", 0 )
	ROM_LOAD( "08h_08b.bin", 0x00000, 0x1000, CRC(0c875994) SHA1(6e4119ade0261eacf8349ff18f1cb7a50be2a9a4) )    /* fg */
	ROM_LOAD( "08f_07b.bin", 0x01000, 0x1000, CRC(497bea8e) SHA1(940592e04ef9dff0e410de040dafe4f6fc745070) )
	ROM_LOAD( "08d_06b.bin", 0x02000, 0x1000, CRC(4ef69b00) SHA1(bad4bbb7159a03efcc9dee1180c231c22bea8f47) )

	ROM_REGION( 0x06000, "gfx2", 0 )
	ROM_LOAD( "05n_16m.bin", 0x00000, 0x1000, CRC(0d3e00fb) SHA1(da144da56733e13c754d066932a32eb6fcd9c83a) )    /* bg1 */
	ROM_LOAD( "05k_15m.bin", 0x02000, 0x1000, CRC(93442213) SHA1(01ceed1124022328b47607ee66d60fe06fdd46ea) )
	ROM_CONTINUE(            0x04000, 0x1000             )

	ROM_REGION( 0x06000, "gfx3", 0 )
	ROM_LOAD( "07n_18m.bin", 0x00000, 0x1000, CRC(d50fced3) SHA1(41f503b2d980548a564a414847b2b6c5ae71da2b) )    /* bg2 */
	ROM_LOAD( "07k_17m.bin", 0x02000, 0x1000, CRC(10c3a5f0) SHA1(ccf7e0b6686129afc6af542d20734e51702cd8a7) )
	ROM_CONTINUE(            0x04000, 0x1000             )

	ROM_REGION( 0x03000, "gfx4", 0 )
	ROM_LOAD( "09n_20m.bin", 0x00000, 0x1000, CRC(54cb8126) SHA1(f2d0b38d1c47a48240bc9e4bc962ef63f5c28ad6) )    /* bg3 */
	ROM_LOAD( "09k_19m.bin", 0x01000, 0x2000, CRC(373e047c) SHA1(0168e22ca72af515980c0975eb8525a416c6dd79) )

	ROM_REGION( 0x0c000, "gfx5", 0 )
	ROM_LOAD( "08p_13b.bin", 0x00000, 0x2000, CRC(40127efd) SHA1(6f8c0f7e4658d54d8fcc6b6e6d2650483788eec1) )    /* sprites */
	ROM_LOAD( "08s_14b.bin", 0x02000, 0x2000, CRC(42648ffa) SHA1(61965428306f94c717b03208be9ac8c27265fcaa) )
	ROM_LOAD( "08m_11b.bin", 0x04000, 0x2000, CRC(ccc4680b) SHA1(641d7b57c442074136f01fe288175ed6621813c5) )
	ROM_LOAD( "08n_12b.bin", 0x06000, 0x2000, CRC(742fafed) SHA1(345683cb9eff1b987721042c36b4d1e0debddd5d) )
	ROM_LOAD( "08j_09b.bin", 0x08000, 0x2000, CRC(1ee63b5c) SHA1(14dea762446cc3c0d4e407dc1e68c2010999fd58) )
	ROM_LOAD( "08k_10b.bin", 0x0a000, 0x2000, CRC(a9f41ec9) SHA1(c24f9d54593e764a0b4530b1a2550b999916992c) )

	ROM_REGION( 0x0020, "proms", 0 )    /* PROMs */
	ROM_LOAD( "07b.bin",    0x0000, 0x0020, CRC(68db8300) SHA1(33cd6b5ed92d7b73a708f2e4b12b6e7f6496d0c6) )  /* unknown - timing? */
ROM_END

ROM_START( starforc )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "3.3p",    0x0000, 0x4000, CRC(8ba27691) SHA1(2b8b1e634ef5bed5c61a078e64a6dda77f84cdf5) )
	ROM_LOAD( "2.3mn",   0x4000, 0x4000, CRC(0fc4d2d6) SHA1(0743e3928d5cc0e3f1bcdaf4b0cc83aeb7a2f7a8) )

	ROM_REGION( 0x2000, "sub", 0 )     /* 64k for sound board */
	ROM_LOAD( "1.3hj",   0x0000, 0x2000, CRC(2735bb22) SHA1(1bd0558e05b41aebab3911991969512df904fea5) )

	ROM_REGION( 0x03000, "gfx1", 0 )
	ROM_LOAD( "7.2fh",   0x00000, 0x1000, CRC(f4803339) SHA1(a119d68c2dd1c0e191231ce77353b31f30f7aa76) )    /* fg */
	ROM_LOAD( "8.3fh",   0x01000, 0x1000, CRC(96979684) SHA1(bb4f7d3afc8dfaa723dfb5374996cc4bfd76fa3c) )
	ROM_LOAD( "9.3fh",   0x02000, 0x1000, CRC(eead1d5c) SHA1(7c9165ed227c5228122b494a265cbfd6e843ba61) )

	ROM_REGION( 0x06000, "gfx2", 0 )
	ROM_LOAD( "15.10jk", 0x00000, 0x2000, CRC(c3bda12f) SHA1(3748ea8e34222a31a365a02ec77430f268b0b397) )    /* bg1 */
	ROM_LOAD( "14.9jk",  0x02000, 0x2000, CRC(9e9384fe) SHA1(3aaa9cc64ef3775325f64733da4f6c328abf6514) )
	ROM_LOAD( "13.8jk",  0x04000, 0x2000, CRC(84603285) SHA1(f4d6dfa3968fbd8ebf1a6451d5ea1821d65d9b49) )

	ROM_REGION( 0x06000, "gfx3", 0 )
	ROM_LOAD( "12.10de", 0x00000, 0x2000, CRC(fdd9e38b) SHA1(3766835d9e9fc7e5dd99521e7303562029b78a65) )    /* bg2 */
	ROM_LOAD( "11.9de",  0x02000, 0x2000, CRC(668aea14) SHA1(62eb0df48f2f0c5778bb230cc3bf0b8eb3b4e3f8) )
	ROM_LOAD( "10.8de",  0x04000, 0x2000, CRC(c62a19c1) SHA1(9ce0e29630d3c8cba4db4cff333b250481348968) )

	ROM_REGION( 0x03000, "gfx4", 0 )
	ROM_LOAD( "18.10pq", 0x00000, 0x1000, CRC(6455c3ad) SHA1(b163ccd3dc26ccfa8be1d16d52e17bc660ff84e3) )    /* bg3 */
	ROM_LOAD( "17.9pq",  0x01000, 0x1000, CRC(68c60d0f) SHA1(1152ba0c274ecadb534133a860bbc8a93577dcf2) )
	ROM_LOAD( "16.8pq",  0x02000, 0x1000, CRC(ce20b469) SHA1(60177a669d9c8cbeedd03ca5e2edf3f589c1c815) )

	ROM_REGION( 0x0c000, "gfx5", 0 )
	ROM_LOAD( "6.10lm",  0x00000, 0x4000, CRC(5468a21d) SHA1(4a1196d4cfb99616efdac9b3927609a85c6f1758) )    /* sprites */
	ROM_LOAD( "5.9lm",   0x04000, 0x4000, CRC(f71717f8) SHA1(bf673571f772d8e0eddae89c00f31390c49a25d2) )
	ROM_LOAD( "4.8lm",   0x08000, 0x4000, CRC(dd9d68a4) SHA1(34c60d2b34c7980bf65a5ebadb9c73f89128141f) )

	ROM_REGION( 0x0020, "proms", 0 )    /* PROMs */
	ROM_LOAD( "07b.bin", 0x0000, 0x0020, CRC(68db8300) SHA1(33cd6b5ed92d7b73a708f2e4b12b6e7f6496d0c6) ) /* unknown - timing? */
ROM_END

ROM_START( starforcb )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "a2.8m",   0x0000, 0x2000, CRC(e81e8b7d) SHA1(829a082a335a0cf5bf8cc97bee96e62271f03ff4) )
	ROM_LOAD( "a3.8k",   0x2000, 0x2000, CRC(7e98f0ab) SHA1(76eb387fbf5d04b7951a115d29076d1fd21346c4) )
	ROM_LOAD( "a4.8j",   0x4000, 0x2000, CRC(285bc599) SHA1(c2f294a5e3ca39084e8f8503d6e9e00885ad0609) )
	ROM_LOAD( "a5.8f",   0x6000, 0x2000, CRC(74d328b1) SHA1(3098438d47f650507de8b1d0aecfca81663cbe9c) )

	ROM_REGION( 0x2000, "sub", 0 )
	ROM_LOAD( "a0.2e",   0x0000, 0x2000, CRC(5ab0e2fa) SHA1(78f0290d1e006a39bb8ee5a29cf229ed2c1f2b09) )

	ROM_REGION( 0x03000, "gfx1", 0 )
	ROM_LOAD( "b8.8h",   0x00000, 0x1000, CRC(f4803339) SHA1(a119d68c2dd1c0e191231ce77353b31f30f7aa76) )    /* fg */
	ROM_LOAD( "b7.8f",   0x01000, 0x1000, CRC(96979684) SHA1(bb4f7d3afc8dfaa723dfb5374996cc4bfd76fa3c) )
	ROM_LOAD( "b6.8d",   0x02000, 0x1000, CRC(eead1d5c) SHA1(7c9165ed227c5228122b494a265cbfd6e843ba61) )

	ROM_REGION( 0x06000, "gfx2", 0 )
	ROM_LOAD( "c17.8a",  0x00000, 0x2000, CRC(c3bda12f) SHA1(3748ea8e34222a31a365a02ec77430f268b0b397) )    /* bg1 */
	ROM_LOAD( "c16.7a",  0x02000, 0x2000, CRC(9e9384fe) SHA1(3aaa9cc64ef3775325f64733da4f6c328abf6514) )
	ROM_LOAD( "c15.6a",  0x04000, 0x2000, CRC(84603285) SHA1(f4d6dfa3968fbd8ebf1a6451d5ea1821d65d9b49) )

	ROM_REGION( 0x06000, "gfx3", 0 )
	ROM_LOAD( "c20.8k",  0x00000, 0x2000, CRC(fdd9e38b) SHA1(3766835d9e9fc7e5dd99521e7303562029b78a65) )    /* bg2 */
	ROM_LOAD( "c19.7k",  0x02000, 0x2000, CRC(668aea14) SHA1(62eb0df48f2f0c5778bb230cc3bf0b8eb3b4e3f8) )
	ROM_LOAD( "c18.6k",  0x04000, 0x2000, CRC(c62a19c1) SHA1(9ce0e29630d3c8cba4db4cff333b250481348968) )

	ROM_REGION( 0x03000, "gfx4", 0 )
	ROM_LOAD( "c5.8n",  0x00000, 0x1000, CRC(6455c3ad) SHA1(b163ccd3dc26ccfa8be1d16d52e17bc660ff84e3) ) /* bg3 */
	ROM_LOAD( "c4.7n",  0x01000, 0x1000, CRC(68c60d0f) SHA1(1152ba0c274ecadb534133a860bbc8a93577dcf2) )
	ROM_LOAD( "c3.6n",  0x02000, 0x1000, CRC(ce20b469) SHA1(60177a669d9c8cbeedd03ca5e2edf3f589c1c815) )

	ROM_REGION( 0x0c000, "gfx5", 0 )
	ROM_LOAD( "b13.8p",       0x00000, 0x2000, CRC(1cfc88a8) SHA1(2948864ed88ba3b1d500047e2ef594b67274710c) )   /* sprites */
	ROM_LOAD( "b14.8r",       0x02000, 0x2000, CRC(902060b4) SHA1(f371aa12ba3f554918e8a482114df166cd007b0e) )   /* sprites */
	ROM_LOAD( "b11.8m",       0x04000, 0x2000, CRC(7676b970) SHA1(f5fcee4ca555e7c880c6bf5d5ea01ff8d619a837) )
	ROM_LOAD( "b12.8n",       0x06000, 0x2000, CRC(6f4a5d67) SHA1(182be475dfee4d272f57c030e3acd4e8cfa4fc53) )
	ROM_LOAD( "b9.8j",        0x08000, 0x2000, CRC(e7d51959) SHA1(34d9afb0f31dc1d02e7b85aa69345fc66cf0f554) )
	ROM_LOAD( "b10.8l",       0x0a000, 0x2000, CRC(6ea27bec) SHA1(30da81a99d5920107751afda359576e426c497c4) )

	ROM_REGION( 0x0020, "proms", 0 )    /* PROMs */
	ROM_LOAD( "a18s030.7b",    0x0000, 0x0020, CRC(68db8300) SHA1(33cd6b5ed92d7b73a708f2e4b12b6e7f6496d0c6) )   /* unknown - timing? */
ROM_END

ROM_START( starforca )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "5.bin",   0x0000, 0x2000, CRC(7691bbd4) SHA1(efcab11ea0ed58b6a47c9d7a994c921dfaa1b47e) )
	ROM_LOAD( "4.bin",   0x2000, 0x2000, CRC(32f3c34e) SHA1(9ecaa46fe296c2f2e9c8faf3d40085c0f10acbe1) )
	ROM_LOAD( "3.bin",   0x4000, 0x2000, CRC(5e99cfa0) SHA1(d16d5247f4afb7abb5b8331ad7ae9d4d1f6d6554) )
	ROM_LOAD( "2.bin",   0x6000, 0x2000, CRC(311c6e59) SHA1(ff3ba96ffade7602d3b150dae4bdc1c02a148576) )

	ROM_REGION( 0x2000, "sub", 0 )
	ROM_LOAD( "0.bin", 0x0000, 0x2000, CRC(a277c268) SHA1(99ed8439119fa4b850ad8aadb7ff3e54d4cd40be) )

	ROM_REGION( 0x03000, "gfx1", 0 )
	ROM_LOAD( "8.bin",   0x00000, 0x1000, CRC(f4803339) SHA1(a119d68c2dd1c0e191231ce77353b31f30f7aa76) )    /* fg */
	ROM_LOAD( "7.bin",   0x01000, 0x1000, CRC(96979684) SHA1(bb4f7d3afc8dfaa723dfb5374996cc4bfd76fa3c) )
	ROM_LOAD( "6.bin",   0x02000, 0x1000, CRC(eead1d5c) SHA1(7c9165ed227c5228122b494a265cbfd6e843ba61) )

	ROM_REGION( 0x06000, "gfx2", 0 )
	ROM_LOAD( "17.bin",  0x00000, 0x2000, CRC(c3bda12f) SHA1(3748ea8e34222a31a365a02ec77430f268b0b397) )    /* bg1 */
	ROM_LOAD( "16.bin",  0x02000, 0x2000, CRC(9e9384fe) SHA1(3aaa9cc64ef3775325f64733da4f6c328abf6514) )
	ROM_LOAD( "15.bin",  0x04000, 0x2000, CRC(84603285) SHA1(f4d6dfa3968fbd8ebf1a6451d5ea1821d65d9b49) )

	ROM_REGION( 0x06000, "gfx3", 0 )
	ROM_LOAD( "20.bin",  0x00000, 0x2000, CRC(fdd9e38b) SHA1(3766835d9e9fc7e5dd99521e7303562029b78a65) )    /* bg2 */
	ROM_LOAD( "19.bin",  0x02000, 0x2000, CRC(668aea14) SHA1(62eb0df48f2f0c5778bb230cc3bf0b8eb3b4e3f8) )
	ROM_LOAD( "18.bin",  0x04000, 0x2000, CRC(c62a19c1) SHA1(9ce0e29630d3c8cba4db4cff333b250481348968) )

	ROM_REGION( 0x06000, "gfx4", 0 ) // twice the size of other sets, but 2nd half is just blank
	ROM_LOAD( "sw5.bin",  0x00000, 0x2000, CRC(ce6bbc11) SHA1(c8f4b22f5ac1c95fff7758c67bf8c39452f5945b) )   /* bg3 */
	ROM_LOAD( "sw4.bin",  0x02000, 0x2000, CRC(f5b4b629) SHA1(d777a144e6dea63f2c3dcd25e32525aa185367ee) )
	ROM_LOAD( "sw3.bin",  0x04000, 0x2000, CRC(0965346d) SHA1(20b223a6aef8dc9c37ab45c575864bce1e9e50db) )

	ROM_REGION( 0x0c000, "gfx5", 0 )
	ROM_LOAD( "13.bin",       0x00000, 0x2000, CRC(1cfc88a8) SHA1(2948864ed88ba3b1d500047e2ef594b67274710c) )   /* sprites */
	ROM_LOAD( "14.bin",       0x02000, 0x2000, CRC(902060b4) SHA1(f371aa12ba3f554918e8a482114df166cd007b0e) )   /* sprites */
	ROM_LOAD( "11.bin",       0x04000, 0x2000, CRC(7676b970) SHA1(f5fcee4ca555e7c880c6bf5d5ea01ff8d619a837) )
	ROM_LOAD( "12.bin",       0x06000, 0x2000, CRC(6f4a5d67) SHA1(182be475dfee4d272f57c030e3acd4e8cfa4fc53) )
	ROM_LOAD( "9.bin",        0x08000, 0x2000, CRC(e7d51959) SHA1(34d9afb0f31dc1d02e7b85aa69345fc66cf0f554) )
	ROM_LOAD( "10.bin",       0x0a000, 0x2000, CRC(6ea27bec) SHA1(30da81a99d5920107751afda359576e426c497c4) )

	ROM_REGION( 0x0020, "proms", 0 )    /* PROMs */
	ROM_LOAD( "prom.bin",    0x0000, 0x0020, CRC(68db8300) SHA1(33cd6b5ed92d7b73a708f2e4b12b6e7f6496d0c6) ) /* unknown - timing? */
ROM_END

ROM_START( starforce )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "starfore.005", 0x0000, 0x2000, CRC(825f7ebe) SHA1(d63fd516e075bcc28d42189216b95bbf491a4cd1) )
	ROM_LOAD( "starfore.004", 0x2000, 0x2000, CRC(fbcecb65) SHA1(0406ae134915539a171603ecdd1b549f98dd048c) )
	ROM_LOAD( "starfore.003", 0x4000, 0x2000, CRC(9f8013b9) SHA1(5398c97d84b4458ff926e07d6189d60565fbd8f1) )
	ROM_LOAD( "starfore.002", 0x6000, 0x2000, CRC(f8111eba) SHA1(cf3295ffae4e36e87aea4332613f2cafb51522ce) )

	ROM_REGION( 0x2000, "sub", 0 )
	ROM_LOAD( "starfore.000", 0x0000, 0x2000, CRC(a277c268) SHA1(99ed8439119fa4b850ad8aadb7ff3e54d4cd40be) )

	ROM_REGION( 0x03000, "gfx1", 0 )
	ROM_LOAD( "7.2fh",   0x00000, 0x1000, CRC(f4803339) SHA1(a119d68c2dd1c0e191231ce77353b31f30f7aa76) )    /* fg */
	ROM_LOAD( "8.3fh",   0x01000, 0x1000, CRC(96979684) SHA1(bb4f7d3afc8dfaa723dfb5374996cc4bfd76fa3c) )
	ROM_LOAD( "9.3fh",   0x02000, 0x1000, CRC(eead1d5c) SHA1(7c9165ed227c5228122b494a265cbfd6e843ba61) )

	ROM_REGION( 0x06000, "gfx2", 0 )
	ROM_LOAD( "15.10jk", 0x00000, 0x2000, CRC(c3bda12f) SHA1(3748ea8e34222a31a365a02ec77430f268b0b397) )    /* bg1 */
	ROM_LOAD( "14.9jk",  0x02000, 0x2000, CRC(9e9384fe) SHA1(3aaa9cc64ef3775325f64733da4f6c328abf6514) )
	ROM_LOAD( "13.8jk",  0x04000, 0x2000, CRC(84603285) SHA1(f4d6dfa3968fbd8ebf1a6451d5ea1821d65d9b49) )

	ROM_REGION( 0x06000, "gfx3", 0 )
	ROM_LOAD( "12.10de", 0x00000, 0x2000, CRC(fdd9e38b) SHA1(3766835d9e9fc7e5dd99521e7303562029b78a65) )    /* bg2 */
	ROM_LOAD( "11.9de",  0x02000, 0x2000, CRC(668aea14) SHA1(62eb0df48f2f0c5778bb230cc3bf0b8eb3b4e3f8) )
	ROM_LOAD( "10.8de",  0x04000, 0x2000, CRC(c62a19c1) SHA1(9ce0e29630d3c8cba4db4cff333b250481348968) )

	ROM_REGION( 0x03000, "gfx4", 0 )
	ROM_LOAD( "18.10pq", 0x00000, 0x1000, CRC(6455c3ad) SHA1(b163ccd3dc26ccfa8be1d16d52e17bc660ff84e3) )    /* bg3 */
	ROM_LOAD( "17.9pq",  0x01000, 0x1000, CRC(68c60d0f) SHA1(1152ba0c274ecadb534133a860bbc8a93577dcf2) )
	ROM_LOAD( "16.8pq",  0x02000, 0x1000, CRC(ce20b469) SHA1(60177a669d9c8cbeedd03ca5e2edf3f589c1c815) )

	ROM_REGION( 0x0c000, "gfx5", 0 )
	ROM_LOAD( "6.10lm",  0x00000, 0x4000, CRC(5468a21d) SHA1(4a1196d4cfb99616efdac9b3927609a85c6f1758) )    /* sprites */
	ROM_LOAD( "5.9lm",   0x04000, 0x4000, CRC(f71717f8) SHA1(bf673571f772d8e0eddae89c00f31390c49a25d2) )
	ROM_LOAD( "4.8lm",   0x08000, 0x4000, CRC(dd9d68a4) SHA1(34c60d2b34c7980bf65a5ebadb9c73f89128141f) )

	ROM_REGION( 0x0020, "proms", 0 )    /* PROMs */
	ROM_LOAD( "07b.bin", 0x0000, 0x0020, CRC(68db8300) SHA1(33cd6b5ed92d7b73a708f2e4b12b6e7f6496d0c6) ) /* unknown - timing? */
ROM_END

ROM_START( megaforc )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "mf3.bin",  0x0000, 0x4000, CRC(d3ea82ec) SHA1(e15fda65ba24517cc04abc55b5d079a33327553c) )
	ROM_LOAD( "mf2.bin",  0x4000, 0x4000, CRC(aa320718) SHA1(cbbf8e4d06a1ecf77d776058d965afdaa7f5b47f) )

	ROM_REGION( 0x2000, "sub", 0 )
	ROM_LOAD( "1.3hj",    0x0000, 0x2000, CRC(2735bb22) SHA1(1bd0558e05b41aebab3911991969512df904fea5) )

	ROM_REGION( 0x03000, "gfx1", 0 )
	ROM_LOAD( "mf7.bin",  0x00000, 0x1000, CRC(43ef8d20) SHA1(07ebe3e10fa56b671788a122cdc02e661b624f40) )   /* fg */
	ROM_LOAD( "mf8.bin",  0x01000, 0x1000, CRC(c36fb746) SHA1(01960e068046bcc0e3e9370fdfe73f9fd64491ae) )
	ROM_LOAD( "mf9.bin",  0x02000, 0x1000, CRC(62e7c9ec) SHA1(24dd1de3e268865c36c732714dc257c58cb88d67) )

	ROM_REGION( 0x06000, "gfx2", 0 )
	ROM_LOAD( "15.10jk",  0x00000, 0x2000, CRC(c3bda12f) SHA1(3748ea8e34222a31a365a02ec77430f268b0b397) )   /* bg1 */
	ROM_LOAD( "14.9jk",   0x02000, 0x2000, CRC(9e9384fe) SHA1(3aaa9cc64ef3775325f64733da4f6c328abf6514) )
	ROM_LOAD( "13.8jk",   0x04000, 0x2000, CRC(84603285) SHA1(f4d6dfa3968fbd8ebf1a6451d5ea1821d65d9b49) )

	ROM_REGION( 0x06000, "gfx3", 0 )
	ROM_LOAD( "12.10de",  0x00000, 0x2000, CRC(fdd9e38b) SHA1(3766835d9e9fc7e5dd99521e7303562029b78a65) )   /* bg2 */
	ROM_LOAD( "11.9de",   0x02000, 0x2000, CRC(668aea14) SHA1(62eb0df48f2f0c5778bb230cc3bf0b8eb3b4e3f8) )
	ROM_LOAD( "10.8de",   0x04000, 0x2000, CRC(c62a19c1) SHA1(9ce0e29630d3c8cba4db4cff333b250481348968) )

	ROM_REGION( 0x03000, "gfx4", 0 )
	ROM_LOAD( "18.10pq",  0x00000, 0x1000, CRC(6455c3ad) SHA1(b163ccd3dc26ccfa8be1d16d52e17bc660ff84e3) )   /* bg3 */
	ROM_LOAD( "17.9pq",   0x01000, 0x1000, CRC(68c60d0f) SHA1(1152ba0c274ecadb534133a860bbc8a93577dcf2) )
	ROM_LOAD( "16.8pq",   0x02000, 0x1000, CRC(ce20b469) SHA1(60177a669d9c8cbeedd03ca5e2edf3f589c1c815) )

	ROM_REGION( 0x0c000, "gfx5", 0 )
	ROM_LOAD( "6.10lm",   0x00000, 0x4000, CRC(5468a21d) SHA1(4a1196d4cfb99616efdac9b3927609a85c6f1758) )   /* sprites */
	ROM_LOAD( "5.9lm",    0x04000, 0x4000, CRC(f71717f8) SHA1(bf673571f772d8e0eddae89c00f31390c49a25d2) )
	ROM_LOAD( "4.8lm",    0x08000, 0x4000, CRC(dd9d68a4) SHA1(34c60d2b34c7980bf65a5ebadb9c73f89128141f) )

	ROM_REGION( 0x0020, "proms", 0 )    /* PROMs */
	ROM_LOAD( "07b.bin",  0x0000, 0x0020, CRC(68db8300) SHA1(33cd6b5ed92d7b73a708f2e4b12b6e7f6496d0c6) )    /* unknown - timing? */
ROM_END

ROM_START( baluba )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "0",            0x0000, 0x4000, CRC(0e2ebe32) SHA1(d5cac260b19dc4e8d2064a7e3de5d52ab0eb95d0) )
	ROM_LOAD( "1",            0x4000, 0x4000, CRC(cde97076) SHA1(ef47851b2ed0d820e1564545795b707d00d5c6ce) )

	ROM_REGION( 0x2000, "sub", 0 )
	ROM_LOAD( "2",            0x0000, 0x2000, CRC(441fbc64) SHA1(3853f80043e28e06a3ee399e3cd261b3ee94e0b9) )

	ROM_REGION( 0x03000, "gfx1", 0 )
	ROM_LOAD( "15",           0x00000, 0x1000, CRC(3dda0d84) SHA1(473c307c157bb229a31cd82ce4cdeca1ff604019) )   /* fg */
	ROM_LOAD( "16",           0x01000, 0x1000, CRC(3ebc79d8) SHA1(a29b4e314446821cd4a2b1a9d3ff16ee3b6a8f7a) )
	ROM_LOAD( "17",           0x02000, 0x1000, CRC(c4430deb) SHA1(e4c18ff2e2c82f3bce346267bc86d4160cb11995) )

	ROM_REGION( 0x06000, "gfx2", 0 )
	ROM_LOAD( "9",            0x00000, 0x2000, CRC(90f88c43) SHA1(e4ea963d9c31e34f70aa2b710760e0a102567988) )   /* bg1 */
	ROM_LOAD( "10",           0x02000, 0x2000, CRC(ab117070) SHA1(d9a8580f3b0919208801b00501579cf81665fc36) )
	ROM_LOAD( "11",           0x04000, 0x2000, CRC(e13b44b0) SHA1(70f3d2465a7652405e23809c81d7ec6ec501835b) )

	ROM_REGION( 0x06000, "gfx3", 0 )
	ROM_LOAD( "12",           0x00000, 0x2000, CRC(a6541c8d) SHA1(d7a211c58c2067f257f5a9e343ca4bf689edd514) )   /* bg2 */
	ROM_LOAD( "13",           0x02000, 0x2000, CRC(afccdd18) SHA1(d238b52a9bb2dfffaf82ca38bc81c0cbd256f79c) )
	ROM_LOAD( "14",           0x04000, 0x2000, CRC(69542e65) SHA1(4119a6f784ed57592d45d325123b261c8f118ca7) )

	ROM_REGION( 0x03000, "gfx4", 0 )
	ROM_LOAD( "8",            0x00000, 0x1000, CRC(31e97ef9) SHA1(ed25db4bdaf06f66cfb7179d80425dcb2cb41363) )   /* bg3 */
	ROM_LOAD( "7",            0x01000, 0x1000, CRC(5915c5e2) SHA1(58301087d91b34747d5cff3c0dca8e9b441ce62d) )
	ROM_LOAD( "6",            0x02000, 0x1000, CRC(ad6881da) SHA1(df629bd9192279b8ebd9d655a94949559e1f118d) )

	ROM_REGION( 0x0c000, "gfx5", 0 )
	ROM_LOAD( "5",            0x00000, 0x4000, CRC(3b6b6e96) SHA1(c55f4b6a5f7738a082c02d1adadd9e1d68a0d293) )   /* sprites */
	ROM_LOAD( "4",            0x04000, 0x4000, CRC(dd954124) SHA1(f37687197d1564331dc27dace23dec462d02202c) )
	ROM_LOAD( "3",            0x08000, 0x4000, CRC(7ac24983) SHA1(4ac32d95af3147af5b9b1af1f292bb629c5d4fb9) )

	ROM_REGION( 0x0020, "proms", 0 )    /* PROMs */
	ROM_LOAD( "07b.bin",    0x0000, 0x0020, CRC(68db8300) SHA1(33cd6b5ed92d7b73a708f2e4b12b6e7f6496d0c6) )  /* unknown - timing? */
ROM_END


DRIVER_INIT_MEMBER(senjyo_state,starforc)
{
	m_is_senjyo = 0;
	m_scrollhack = 1;
}
DRIVER_INIT_MEMBER(senjyo_state,starfore)
{
	static const UINT8 convtable[32][4] =
	{
		/*       opcode                   data                     address      */
		/*  A    B    C    D         A    B    C    D                           */
		{ 0x20,0x00,0xa0,0x80 }, { 0xa8,0xa0,0x88,0x80 },   /* ...0...0...0...0 */
		{ 0x20,0x00,0xa0,0x80 }, { 0xa8,0xa0,0x88,0x80 },   /* ...0...0...0...1 */
		{ 0x20,0x00,0xa0,0x80 }, { 0xa8,0xa0,0x88,0x80 },   /* ...0...0...1...0 */
		{ 0x88,0x08,0x80,0x00 }, { 0xa0,0x80,0xa8,0x88 },   /* ...0...0...1...1 */
		{ 0x88,0x08,0x80,0x00 }, { 0xa0,0x80,0xa8,0x88 },   /* ...0...1...0...0 */
		{ 0x20,0x00,0xa0,0x80 }, { 0xa8,0xa0,0x88,0x80 },   /* ...0...1...0...1 */
		{ 0x88,0x08,0x80,0x00 }, { 0xa0,0x80,0xa8,0x88 },   /* ...0...1...1...0 */
		{ 0x28,0xa8,0x08,0x88 }, { 0x88,0x80,0x08,0x00 },   /* ...0...1...1...1 */
		{ 0x20,0x00,0xa0,0x80 }, { 0xa8,0xa0,0x88,0x80 },   /* ...1...0...0...0 */
		{ 0x88,0x08,0x80,0x00 }, { 0xa0,0x80,0xa8,0x88 },   /* ...1...0...0...1 */
		{ 0x88,0x08,0x80,0x00 }, { 0xa0,0x80,0xa8,0x88 },   /* ...1...0...1...0 */
		{ 0x20,0x00,0xa0,0x80 }, { 0xa8,0xa0,0x88,0x80 },   /* ...1...0...1...1 */
		{ 0x88,0x08,0x80,0x00 }, { 0xa0,0x80,0xa8,0x88 },   /* ...1...1...0...0 */
		{ 0x28,0xa8,0x08,0x88 }, { 0x88,0x80,0x08,0x00 },   /* ...1...1...0...1 */
		{ 0x20,0x00,0xa0,0x80 }, { 0xa8,0xa0,0x88,0x80 },   /* ...1...1...1...0 */
		{ 0x88,0x08,0x80,0x00 }, { 0xa0,0x80,0xa8,0x88 }    /* ...1...1...1...1 */
	};

	sega_decode(memregion("maincpu")->base(), m_decrypted_opcodes, 0x8000, convtable);

	m_is_senjyo = 0;
	m_scrollhack = 0;
}

DRIVER_INIT_MEMBER(senjyo_state,starfora)
{
	static const UINT8 convtable[32][4] =
	{
		/*       opcode                   data                     address      */
		/*  A    B    C    D         A    B    C    D                           */
		{ 0x88,0xa8,0x08,0x28 }, { 0x88,0xa8,0x80,0xa0 },   /* ...0...0...0...0 */
		{ 0x20,0xa0,0x28,0xa8 }, { 0x88,0xa8,0x80,0xa0 },   /* ...0...0...0...1 */
		{ 0x88,0xa8,0x80,0xa0 }, { 0x88,0xa8,0x80,0xa0 },   /* ...0...0...1...0 */
		{ 0x88,0xa8,0x80,0xa0 }, { 0x20,0xa0,0x28,0xa8 },   /* ...0...0...1...1 */
		{ 0x88,0xa8,0x08,0x28 }, { 0x88,0xa8,0x08,0x28 },   /* ...0...1...0...0 */
		{ 0x88,0xa8,0x80,0xa0 }, { 0x88,0xa8,0x80,0xa0 },   /* ...0...1...0...1 */
		{ 0x20,0xa0,0x28,0xa8 }, { 0x20,0xa0,0x28,0xa8 },   /* ...0...1...1...0 */
		{ 0x88,0xa8,0x80,0xa0 }, { 0x88,0xa8,0x80,0xa0 },   /* ...0...1...1...1 */
		{ 0x20,0xa0,0x28,0xa8 }, { 0x88,0xa8,0x08,0x28 },   /* ...1...0...0...0 */
		{ 0x20,0xa0,0x28,0xa8 }, { 0x28,0x20,0xa8,0xa0 },   /* ...1...0...0...1 */
		{ 0xa0,0x20,0x80,0x00 }, { 0x20,0xa0,0x28,0xa8 },   /* ...1...0...1...0 */
		{ 0x28,0x20,0xa8,0xa0 }, { 0x20,0xa0,0x28,0xa8 },   /* ...1...0...1...1 */
		{ 0x20,0xa0,0x28,0xa8 }, { 0x88,0xa8,0x08,0x28 },   /* ...1...1...0...0 */
		{ 0x88,0xa8,0x08,0x28 }, { 0x88,0xa8,0x08,0x28 },   /* ...1...1...0...1 */
		{ 0xa0,0x20,0x80,0x00 }, { 0x88,0x08,0x80,0x00 },   /* ...1...1...1...0 */
		{ 0x20,0xa0,0x28,0xa8 }, { 0x00,0x08,0x20,0x28 }    /* ...1...1...1...1 */
	};

	sega_decode(memregion("maincpu")->base(), m_decrypted_opcodes, 0x8000, convtable);

	m_is_senjyo = 0;
	m_scrollhack = 1;
}

DRIVER_INIT_MEMBER(senjyo_state,senjyo)
{
	m_is_senjyo = 1;
	m_scrollhack = 0;
}


GAME( 1983, senjyo,   0,        senjyo,  senjyo, senjyo_state,   senjyo,   ROT90, "Tehkan", "Senjyo", MACHINE_SUPPORTS_SAVE )
GAME( 1984, starforc, 0,        senjyo,  starforc, senjyo_state, starforc, ROT90, "Tehkan", "Star Force", MACHINE_SUPPORTS_SAVE )
GAME( 1984, starforce,starforc, senjyox, starforc, senjyo_state, starfore, ROT90, "Tehkan", "Star Force (encrypted, set 1)", MACHINE_SUPPORTS_SAVE )
GAME( 1984, starforcb,starforc, starforb,starforc, senjyo_state, starfore, ROT90, "bootleg", "Star Force (encrypted, bootleg)", MACHINE_SUPPORTS_SAVE )
GAME( 1984, starforca,starforc, senjyox, starforc, senjyo_state, starfora, ROT90, "Tehkan", "Star Force (encrypted, set 2)", MACHINE_SUPPORTS_SAVE )
GAME( 1985, megaforc, starforc, senjyo,  starforc, senjyo_state, starforc, ROT90, "Tehkan (Video Ware license)", "Mega Force", MACHINE_SUPPORTS_SAVE )
GAME( 1986, baluba,   0,        senjyo,  baluba, senjyo_state,   starforc, ROT90, "Able Corp, Ltd.", "Baluba-louk no Densetsu (Japan)", MACHINE_SUPPORTS_SAVE )
