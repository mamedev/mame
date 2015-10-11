// license:BSD-3-Clause
// copyright-holders:Ernesto Corvi
/***************************************************************************

King of Boxer - (c) 1985 Wood Place Inc.
Ring King     - (c) 1985 Data East USA Inc. / Wood Place Inc.

Driver by:
Ernesto Corvi
ernesto@imagina.com

Notes:
-----
Main CPU:
- Theres a memory area from 0xf000 to 0xf7ff, which is clearly
  initialized at startup and never used anymore.

***************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "sound/ay8910.h"
#include "sound/dac.h"
#include "includes/kingobox.h"

WRITE8_MEMBER(kingofb_state::video_interrupt_w)
{
	m_video_cpu->set_input_line_and_vector(0, HOLD_LINE, 0xff);
}

WRITE8_MEMBER(kingofb_state::sprite_interrupt_w)
{
	m_sprite_cpu->set_input_line_and_vector(0, HOLD_LINE, 0xff);
}

WRITE8_MEMBER(kingofb_state::scroll_interrupt_w)
{
	sprite_interrupt_w(space, offset, data);
	*m_scroll_y = data;
}

WRITE8_MEMBER(kingofb_state::sound_command_w)
{
	soundlatch_byte_w(space, 0, data);
	m_audiocpu->set_input_line_and_vector(0, HOLD_LINE, 0xff);
}


static ADDRESS_MAP_START( kingobox_map, AS_PROGRAM, 8, kingofb_state )
	AM_RANGE(0x0000, 0xbfff) AM_ROM
	AM_RANGE(0xc000, 0xc3ff) AM_RAM /* work ram */
	AM_RANGE(0xe000, 0xe7ff) AM_RAM AM_SHARE("share2") /* shared with sprite cpu */
	AM_RANGE(0xe800, 0xefff) AM_RAM AM_SHARE("share1") /* shared with video cpu */
	AM_RANGE(0xf000, 0xf7ff) AM_RAM /* ???? */
	AM_RANGE(0xf800, 0xf800) AM_WRITE(kingofb_f800_w)   /* NMI enable, palette bank */
	AM_RANGE(0xf801, 0xf801) AM_WRITENOP /* ???? */
	AM_RANGE(0xf802, 0xf802) AM_WRITEONLY AM_SHARE("scroll_y")
	AM_RANGE(0xf803, 0xf803) AM_WRITE(scroll_interrupt_w)
	AM_RANGE(0xf804, 0xf804) AM_WRITE(video_interrupt_w)
	AM_RANGE(0xf807, 0xf807) AM_WRITE(sound_command_w) /* sound latch */
	AM_RANGE(0xfc00, 0xfc00) AM_READ_PORT("DSW1")
	AM_RANGE(0xfc01, 0xfc01) AM_READ_PORT("DSW2")
	AM_RANGE(0xfc02, 0xfc02) AM_READ_PORT("P1")
	AM_RANGE(0xfc03, 0xfc03) AM_READ_PORT("P2")
	AM_RANGE(0xfc04, 0xfc04) AM_READ_PORT("SYSTEM")
	AM_RANGE(0xfc05, 0xfc05) AM_READ_PORT("EXTRA")
ADDRESS_MAP_END

static ADDRESS_MAP_START( kingobox_video_map, AS_PROGRAM, 8, kingofb_state )
	AM_RANGE(0x0000, 0x3fff) AM_ROM
	AM_RANGE(0x8000, 0x87ff) AM_RAM /* work ram */
	AM_RANGE(0xa000, 0xa7ff) AM_RAM AM_SHARE("share1") /* shared with main */
	AM_RANGE(0xc000, 0xc0ff) AM_RAM_WRITE(kingofb_videoram_w) AM_SHARE("videoram") /* background vram */
	AM_RANGE(0xc400, 0xc4ff) AM_RAM_WRITE(kingofb_colorram_w) AM_SHARE("colorram") /* background colorram */
	AM_RANGE(0xc800, 0xcbff) AM_RAM_WRITE(kingofb_videoram2_w) AM_SHARE("videoram2") /* foreground vram */
	AM_RANGE(0xcc00, 0xcfff) AM_RAM_WRITE(kingofb_colorram2_w) AM_SHARE("colorram2") /* foreground colorram */
ADDRESS_MAP_END

static ADDRESS_MAP_START( kingobox_sprite_map, AS_PROGRAM, 8, kingofb_state )
	AM_RANGE(0x0000, 0x1fff) AM_ROM
	AM_RANGE(0x8000, 0x87ff) AM_RAM /* work ram */
	AM_RANGE(0xa000, 0xa7ff) AM_RAM AM_SHARE("share2") /* shared with main */
	AM_RANGE(0xc000, 0xc3ff) AM_RAM AM_SHARE("spriteram") /* sprite ram */
	AM_RANGE(0xc400, 0xc43f) AM_RAM  /* something related to scroll? */
ADDRESS_MAP_END

static ADDRESS_MAP_START( kingobox_sound_map, AS_PROGRAM, 8, kingofb_state )
	AM_RANGE(0x0000, 0xbfff) AM_ROM
	AM_RANGE(0x8000, 0x8000) AM_WRITENOP /* ??? */
	AM_RANGE(0xc000, 0xc3ff) AM_RAM /* work ram */
ADDRESS_MAP_END

static ADDRESS_MAP_START( kingobox_sound_io_map, AS_IO, 8, kingofb_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x00) AM_DEVWRITE("dac", dac_device, write_unsigned8)
	AM_RANGE(0x08, 0x08) AM_DEVREADWRITE("aysnd", ay8910_device, data_r, data_w)
	AM_RANGE(0x0c, 0x0c) AM_DEVWRITE("aysnd", ay8910_device, address_w)
ADDRESS_MAP_END

/* Ring King */
static ADDRESS_MAP_START( ringking_map, AS_PROGRAM, 8, kingofb_state )
	AM_RANGE(0x0000, 0xbfff) AM_ROM
	AM_RANGE(0xc000, 0xc3ff) AM_RAM /* work ram */
	AM_RANGE(0xc800, 0xcfff) AM_RAM AM_SHARE("share2") /* shared with sprite cpu */
	AM_RANGE(0xd000, 0xd7ff) AM_RAM AM_SHARE("share1") /* shared with video cpu */
	AM_RANGE(0xd800, 0xd800) AM_WRITE(kingofb_f800_w)
	AM_RANGE(0xd801, 0xd801) AM_WRITE(sprite_interrupt_w)
	AM_RANGE(0xd802, 0xd802) AM_WRITE(video_interrupt_w)
	AM_RANGE(0xd803, 0xd803) AM_WRITE(sound_command_w)
	AM_RANGE(0xe000, 0xe000) AM_READ_PORT("DSW1")
	AM_RANGE(0xe001, 0xe001) AM_READ_PORT("DSW2")
	AM_RANGE(0xe002, 0xe002) AM_READ_PORT("P1")
	AM_RANGE(0xe003, 0xe003) AM_READ_PORT("P2")
	AM_RANGE(0xe004, 0xe004) AM_READ_PORT("SYSTEM")
	AM_RANGE(0xe005, 0xe005) AM_READ_PORT("EXTRA")
	AM_RANGE(0xe800, 0xe800) AM_WRITEONLY AM_SHARE("scroll_y")
	AM_RANGE(0xf000, 0xf7ff) AM_RAM /* ???? */
ADDRESS_MAP_END

static ADDRESS_MAP_START( ringking_video_map, AS_PROGRAM, 8, kingofb_state )
	AM_RANGE(0x0000, 0x3fff) AM_ROM
	AM_RANGE(0x8000, 0x87ff) AM_RAM /* work ram */
	AM_RANGE(0xc000, 0xc7ff) AM_RAM AM_SHARE("share1") /* shared with main */
	AM_RANGE(0xa800, 0xa8ff) AM_RAM_WRITE(kingofb_videoram_w) AM_SHARE("videoram") /* background vram */
	AM_RANGE(0xac00, 0xacff) AM_RAM_WRITE(kingofb_colorram_w) AM_SHARE("colorram") /* background colorram */
	AM_RANGE(0xa000, 0xa3ff) AM_RAM_WRITE(kingofb_videoram2_w) AM_SHARE("videoram2") /* foreground vram */
	AM_RANGE(0xa400, 0xa7ff) AM_RAM_WRITE(kingofb_colorram2_w) AM_SHARE("colorram2") /* foreground colorram */
ADDRESS_MAP_END

static ADDRESS_MAP_START( ringking_sprite_map, AS_PROGRAM, 8, kingofb_state )
	AM_RANGE(0x0000, 0x1fff) AM_ROM
	AM_RANGE(0x8000, 0x87ff) AM_RAM /* work ram */
	AM_RANGE(0xc800, 0xcfff) AM_RAM AM_SHARE("share2") /* shared with main */
	AM_RANGE(0xa000, 0xa3ff) AM_RAM AM_SHARE("spriteram") /* sprite ram */
	AM_RANGE(0xa400, 0xa43f) AM_RAM /* something related to scroll? */
ADDRESS_MAP_END

static ADDRESS_MAP_START( ringking_sound_io_map, AS_IO, 8, kingofb_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x00) AM_DEVWRITE("dac", dac_device, write_unsigned8)
	AM_RANGE(0x02, 0x02) AM_DEVREAD("aysnd", ay8910_device, data_r)
	AM_RANGE(0x02, 0x03) AM_DEVWRITE("aysnd", ay8910_device, data_address_w)
ADDRESS_MAP_END


static INPUT_PORTS_START( kingofb )
	PORT_START("DSW1")  /* 0xfc01 */
	PORT_DIPNAME( 0x03, 0x01, "Rest Up Points" )
	PORT_DIPSETTING(    0x02, "70000" )
	PORT_DIPSETTING(    0x01, "100000" )
	PORT_DIPSETTING(    0x03, "150000" )
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x18, 0x00, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Medium ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x18, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_SERVICE( 0x80, IP_ACTIVE_HIGH )

	PORT_START("DSW2")  /* 0xfc01 */
	PORT_DIPNAME( 0x07, 0x00, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_5C ) )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, "Freeze" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START("P1")    /* 0xfc02 */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON2 )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("P2")    /* 0xfc03 */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_COCKTAIL
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("SYSTEM")    /* 0xfc04 */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("EXTRA") /* 0xfc05 */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON3 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )
INPUT_PORTS_END

static INPUT_PORTS_START( ringking )
	PORT_START("DSW1")  /* 0xe000 */
	PORT_DIPNAME( 0x03, 0x03, "Replay" )
	PORT_DIPSETTING(    0x01, "70000" )
	PORT_DIPSETTING(    0x02, "100000" )
	PORT_DIPSETTING(    0x00, "150000" )
	PORT_DIPSETTING(    0x03, DEF_STR( No ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x18, 0x10, "Difficulty (2P)" )
	PORT_DIPSETTING(    0x18, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Medium ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Cocktail ) )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_SERVICE( 0x80, IP_ACTIVE_LOW )

	PORT_START("DSW2")  /* 0xe001 */
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_3C ) )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_3C ) )
	PORT_DIPNAME( 0x30, 0x10, "Difficulty (1P)" )
	PORT_DIPSETTING(    0x30, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Medium ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x40, 0x40, "Boxing Match" )
	PORT_DIPSETTING(    0x40, "2 Win, End" )
	PORT_DIPSETTING(    0x00, "1 Win, End" )
	PORT_DIPNAME( 0x80, 0x80, "Freeze" )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("P1")    /* 0xe002 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("P2")    /* 0xe003 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("SYSTEM")    /* 0xe004 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN ) /* Sound busy??? */
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_VBLANK("screen")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("EXTRA") /* 0xfc05 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON3 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END



static const gfx_layout charlayout =
{
	8,8,    /* 8*8 characters */
	512,   /* 1024 characters */
	1,      /* 1 bits per pixel */
	{ 0 },     /* only 1 plane */
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8     /* every char takes 8 consecutive bytes */
};

static const gfx_layout spritelayout =
{
	16,16,  /* 16*16 chars */
	1024,   /* 1024 characters */
	3,      /* bits per pixel */
	{ 2*0x4000*8, 1*0x4000*8, 0*0x4000*8 },
	{ 3*0x4000*8+0,3*0x4000*8+1,3*0x4000*8+2,3*0x4000*8+3,
			3*0x4000*8+4,3*0x4000*8+5,3*0x4000*8+6,3*0x4000*8+7,
			0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8,
			8*8, 9*8, 10*8, 11*8, 12*8, 13*8, 14*8, 15*8 },
	16*8
};

static const gfx_layout tilelayout =
{
	16,16,  /* 16*16 chars */
	512,    /* 512 characters */
	3,      /* bits per pixel */
	{ 2*0x2000*8, 1*0x2000*8, 0*0x2000*8 },
	{ 3*0x2000*8+0,3*0x2000*8+1,3*0x2000*8+2,3*0x2000*8+3,
			3*0x2000*8+4,3*0x2000*8+5,3*0x2000*8+6,3*0x2000*8+7,
			0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8,
			8*8, 9*8, 10*8, 11*8, 12*8, 13*8, 14*8, 15*8 },
	16*8
};

static GFXDECODE_START( kingobox )
	GFXDECODE_ENTRY( "gfx1", 0x00000, charlayout,   256,  8 )   /* characters */
	GFXDECODE_ENTRY( "gfx1", 0x01000, charlayout,   256,  8 )   /* characters */
	GFXDECODE_ENTRY( "gfx2", 0x00000, spritelayout,   0, 32 )   /* sprites */
	GFXDECODE_ENTRY( "gfx3", 0x00000, tilelayout,     0, 32 )   /* bg tiles */
GFXDECODE_END

/* Ring King */
static const gfx_layout rk_charlayout1 =
{
	8,8,    /* 8*8 characters */
	512,   /* 1024 characters */
	1,      /* 1 bits per pixel */
	{ 0 },     /* only 1 plane */
	{ 7, 6, 5, 4, (0x1000*8)+7, (0x1000*8)+6, (0x1000*8)+5, (0x1000*8)+4 },
	{ 7*8, 6*8, 5*8, 4*8, 3*8, 2*8, 1*8, 0*8 },
	8*8     /* every char takes 8 consecutive bytes */
};

static const gfx_layout rk_charlayout2 =
{
	8,8,    /* 8*8 characters */
	512,   /* 1024 characters */
	1,      /* 1 bits per pixel */
	{ 0 },     /* only 1 plane */
	{ 3, 2, 1, 0, (0x1000*8)+3, (0x1000*8)+2, (0x1000*8)+1, (0x1000*8)+0 },
	{ 7*8, 6*8, 5*8, 4*8, 3*8, 2*8, 1*8, 0*8 },
	8*8     /* every char takes 8 consecutive bytes */
};

static const gfx_layout rk_spritelayout =
{
	16,16,  /* 16*16 chars */
	1024,   /* 1024 characters */
	3,      /* bits per pixel */
	{ 0*0x8000*8, 1*0x8000*8, 2*0x8000*8 },
	{ 7, 6, 5, 4, 3, 2, 1, 0,
		16*8+7, 16*8+6, 16*8+5, 16*8+4, 16*8+3, 16*8+2, 16*8+1, 16*8+0 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8,
			8*8, 9*8, 10*8, 11*8, 12*8, 13*8, 14*8, 15*8 },
	32*8
};

static const gfx_layout rk_tilelayout =
{
	16,16,  /* 16*16 chars */
	512,    /* 1024 characters */
	3,      /* bits per pixel */
	{ 0*0x4000*8, 1*0x4000*8, 2*0x4000*8 },
	{ 7, 6, 5, 4, 3, 2, 1, 0,
		16*8+7, 16*8+6, 16*8+5, 16*8+4, 16*8+3, 16*8+2, 16*8+1, 16*8+0 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8,
			8*8, 9*8, 10*8, 11*8, 12*8, 13*8, 14*8, 15*8 },
	32*8
};

static const gfx_layout rk_bglayout =
{
	16,16,  /* 16*16 chars */
	256,    /* 1024 characters */
	3,      /* bits per pixel */
	{ 0x4000*8+4, 0, 4 },
	{ 16*8+3, 16*8+2, 16*8+1, 16*8+0, 0x2000*8+3, 0x2000*8+2, 0x2000*8+1, 0x2000*8+0,
		3, 2, 1, 0, 0x2010*8+3, 0x2010*8+2, 0x2010*8+1, 0x2010*8+0 },
	{ 15*8, 14*8, 13*8, 12*8, 11*8, 10*8, 9*8, 8*8,
			7*8, 6*8, 5*8, 4*8, 3*8, 2*8, 1*8, 0*8 },
	32*8
};


static GFXDECODE_START( rk )
	GFXDECODE_ENTRY( "gfx1", 0x00000, rk_charlayout1,  256,  8 )    /* characters */
	GFXDECODE_ENTRY( "gfx1", 0x00000, rk_charlayout2,  256,  8 )    /* characters */
	GFXDECODE_ENTRY( "gfx2", 0x00000, rk_spritelayout,   0, 32 )    /* sprites */
	GFXDECODE_ENTRY( "gfx3", 0x00000, rk_tilelayout,     0, 32 )    /* sprites/bg tiles */
	GFXDECODE_ENTRY( "gfx4", 0x00000, rk_bglayout,       0, 32 )    /* bg tiles */
GFXDECODE_END

INTERRUPT_GEN_MEMBER(kingofb_state::kingofb_interrupt)
{
	if (m_nmi_enable)
		device.execute().set_input_line(INPUT_LINE_NMI, PULSE_LINE);
}

void kingofb_state::machine_start()
{
	save_item(NAME(m_nmi_enable));
	save_item(NAME(m_palette_bank));
}

void kingofb_state::machine_reset()
{
	m_nmi_enable = 0;
	m_palette_bank = 0;
}

static MACHINE_CONFIG_START( kingofb, kingofb_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, 4000000)        /* 4.0 MHz */
	MCFG_CPU_PROGRAM_MAP(kingobox_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", kingofb_state,  kingofb_interrupt)

	MCFG_CPU_ADD("video", Z80, 4000000)        /* 4.0 MHz */
	MCFG_CPU_PROGRAM_MAP(kingobox_video_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", kingofb_state,  kingofb_interrupt)

	MCFG_CPU_ADD("sprite", Z80, 4000000)        /* 4.0 MHz */
	MCFG_CPU_PROGRAM_MAP(kingobox_sprite_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", kingofb_state,  kingofb_interrupt)

	MCFG_CPU_ADD("audiocpu", Z80, 4000000)        /* 4.0 MHz */
	MCFG_CPU_PROGRAM_MAP(kingobox_sound_map)
	MCFG_CPU_IO_MAP(kingobox_sound_io_map)
	MCFG_CPU_PERIODIC_INT_DRIVER(kingofb_state, nmi_line_pulse,  6000)  /* Hz */

	MCFG_QUANTUM_TIME(attotime::from_hz(6000)) /* We really need heavy synching among the processors */


	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500) /* not accurate */)
	MCFG_SCREEN_SIZE(32*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 2*8, 30*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(kingofb_state, screen_update_kingofb)
	MCFG_SCREEN_PALETTE("palette")
	MCFG_SCREEN_ORIENTATION(ROT90)

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", kingobox)
	MCFG_PALETTE_ADD("palette", 256+8*2)
	MCFG_PALETTE_INDIRECT_ENTRIES(256+8)
	MCFG_PALETTE_INIT_OWNER(kingofb_state,kingofb)
	MCFG_VIDEO_START_OVERRIDE(kingofb_state,kingofb)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("aysnd", AY8910, 1500000)
	MCFG_AY8910_PORT_A_READ_CB(READ8(driver_device, soundlatch_byte_r))
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)

	MCFG_DAC_ADD("dac")
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
MACHINE_CONFIG_END


/* Ring King */
static MACHINE_CONFIG_START( ringking, kingofb_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, 4000000)        /* 4.0 MHz */
	MCFG_CPU_PROGRAM_MAP(ringking_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", kingofb_state,  kingofb_interrupt)

	MCFG_CPU_ADD("video", Z80, 4000000)        /* 4.0 MHz */
	MCFG_CPU_PROGRAM_MAP(ringking_video_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", kingofb_state,  kingofb_interrupt)

	MCFG_CPU_ADD("sprite", Z80, 4000000)        /* 4.0 MHz */
	MCFG_CPU_PROGRAM_MAP(ringking_sprite_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", kingofb_state,  kingofb_interrupt)

	MCFG_CPU_ADD("audiocpu", Z80, 4000000)        /* 4.0 MHz */
	MCFG_CPU_PROGRAM_MAP(kingobox_sound_map)
	MCFG_CPU_IO_MAP(ringking_sound_io_map)
	MCFG_CPU_PERIODIC_INT_DRIVER(kingofb_state, nmi_line_pulse,  6000)  /* Hz */

	MCFG_QUANTUM_TIME(attotime::from_hz(6000)) /* We really need heavy synching among the processors */


	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500) /* not accurate */)
	MCFG_SCREEN_SIZE(32*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 2*8, 30*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(kingofb_state, screen_update_ringking)
	MCFG_SCREEN_PALETTE("palette")
	MCFG_SCREEN_ORIENTATION(ROT90)

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", rk)
	MCFG_PALETTE_ADD("palette", 256+8*2)
	MCFG_PALETTE_INDIRECT_ENTRIES(256+8)
	MCFG_PALETTE_INIT_OWNER(kingofb_state,ringking)
	MCFG_VIDEO_START_OVERRIDE(kingofb_state,ringking)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("aysnd", AY8910, 1500000)
	MCFG_AY8910_PORT_A_READ_CB(READ8(driver_device, soundlatch_byte_r))
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)

	MCFG_DAC_ADD("dac")
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
MACHINE_CONFIG_END


/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( kingofb )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "d09_22.bin",   0x00000, 0x4000, CRC(6220bfa2) SHA1(cb329406ed07b71f9d2c40fc6c2c196daaa56fc8) )
	ROM_LOAD( "e09_23.bin",   0x04000, 0x4000, CRC(5782fdd8) SHA1(6c8c1114ce7863f9e8331796e2c5fb4928904b55) )

	ROM_REGION( 0x10000, "video", 0 )     /* 64k for the video cpu */
	ROM_LOAD( "b09_21.bin",   0x00000, 0x4000, CRC(3fb39489) SHA1(cddd939cb57bb684427cf5c8538ad0e9f8f4586d) )

	ROM_REGION( 0x10000, "sprite", 0 )     /* 64k for the sprite cpu */
	ROM_LOAD( "j09_dcr.bin",  0x00000, 0x2000, CRC(379f4f84) SHA1(c8171e15fe243857b6ca8f32c1cc09f12fa4c07c) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "f05_18.bin",   0x00000, 0x4000, CRC(c057e28e) SHA1(714d8f14d55a070efcf205f8946269181bf2198b) )
	ROM_LOAD( "h05_19.bin",   0x04000, 0x4000, CRC(060253dd) SHA1(9a24fc6aca64262e935971f96b3a103df9711f20) )
	ROM_LOAD( "j05_20.bin",   0x08000, 0x4000, CRC(64c137a4) SHA1(e38adeb19e24357cc5581f0a3097c1d24914e25c) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "vd15_13.bin",  0x00000, 0x2000, CRC(e36d4f4f) SHA1(059799b04a7d3e02c1a7f9a5b878d06afef305df) ) /* characters */

	ROM_REGION( 0x18000, "gfx2", 0 )    /* sprites */
	ROM_LOAD( "vb01_01.bin",  0x00000, 0x4000, CRC(ce6580af) SHA1(9a94c681d4c54ca6c2f41ba1e51c61f54e844c77) )
	ROM_LOAD( "vb04_03.bin",  0x04000, 0x4000, CRC(cf74ea50) SHA1(9b0bdf636f9b31e6c7074d606d431a849a51e518) )
	ROM_LOAD( "vb07_05.bin",  0x08000, 0x4000, CRC(d8b53975) SHA1(52ad0b26fef7bb20d1bf953c5ebd519656682bac) )
	ROM_LOAD( "vb03_02.bin",  0x0c000, 0x4000, CRC(4ab506d2) SHA1(8c293d38429a1462f49462d623c47c402e3372f0) )
	ROM_LOAD( "vb05_04.bin",  0x10000, 0x4000, CRC(ecf95a2c) SHA1(b93d0ebdbde9311194a91fb3d6e5d5f33cc87e9d) )
	ROM_LOAD( "vb08_06.bin",  0x14000, 0x4000, CRC(8200cb2b) SHA1(c9e66027d796dd523eddf378d0e9a62ebcc8f6c8) )

	ROM_REGION( 0xc000, "gfx3", 0 ) /* tiles */
	ROM_LOAD( "vd01_07.bin",  0x00000, 0x2000, CRC(3d472a22) SHA1(85a2e25cee8f85ac0d2ee12f60f97e26539ebd52) )
	ROM_LOAD( "vd04_09.bin",  0x02000, 0x2000, CRC(cc002ea9) SHA1(194aa60809c2ae12be2d7533170988e47549c239) )
	ROM_LOAD( "vd07_11.bin",  0x04000, 0x2000, CRC(23c1b3ee) SHA1(8a8a187920243f3d3870a2fa71b0f6494e53107a) )
	ROM_LOAD( "vd03_08.bin",  0x06000, 0x2000, CRC(d6b1b8fe) SHA1(6bbe02a0a9e080f3ed3c32d64afb81905b42082f) )
	ROM_LOAD( "vd05_10.bin",  0x08000, 0x2000, CRC(fce71e5a) SHA1(a68ad30e8e207d24bc5543dcbcbc3e39260b6cc5) )
	ROM_LOAD( "vd08_12.bin",  0x0a000, 0x2000, CRC(3f68b991) SHA1(487e7d793fe6c1dbecd5f54f790105bbb44a21de) )

	ROM_REGION( 0x0300, "proms", 0 )
	ROM_LOAD( "vb14_col.bin", 0x0000, 0x0100, CRC(c58e5121) SHA1(2e6658e24c183d8dacf4ff84a38060e57d11f265) )    /* red component */
	ROM_LOAD( "vb15_col.bin", 0x0100, 0x0100, CRC(5ab06f25) SHA1(f5e0aabf40ce6d11771e0678fea248abd5b95b3c) )    /* green component */
	ROM_LOAD( "vb16_col.bin", 0x0200, 0x0100, CRC(1171743f) SHA1(ddfce0ff213381a2fc94337681e599cb28db840c) )    /* blue component */

	ROM_REGION( 0x0200, "plds", 0 )
	ROM_LOAD( "pal12h6-vh02.bin",   0x0000, 0x0034, CRC(6cc0fdf2) SHA1(12eef0d49def671aa7dbb4cce91cfbe40697dcea) )
	ROM_LOAD( "pal14h4-vh07.bin",   0x0100, 0x003c, CRC(7e59d45a) SHA1(4a900e424c9edc9f8664f935edccbe4b4759188a) )
ROM_END

/* Ring King */
ROM_START( ringking )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "cx13.9f",      0x00000, 0x8000, CRC(93e38c02) SHA1(8f96f16f2904ef83101448fdf201b98b8e75e1d6) )
	ROM_LOAD( "cx14.11f",     0x08000, 0x4000, CRC(a435acb0) SHA1(2c9d4e8471d87ce148f9c2180769350401914fc0) )

	ROM_REGION( 0x10000, "video", 0 )     /* 64k for the video cpu */
	ROM_LOAD( "cx07.10c",     0x00000, 0x4000, CRC(9f074746) SHA1(fc7cb0b1348b9a4ada9a99786a365ffacabbeed3) )

	ROM_REGION( 0x10000, "sprite", 0 )     /* 64k for the sprite cpu */
	ROM_LOAD( "cx00.4c",      0x00000, 0x2000, CRC(880b8aa7) SHA1(e5ee80cac85a62ae5a677115a74c08e433cd4fc9) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "cx12.4ef",     0x00000, 0x8000, CRC(1d5d6c6b) SHA1(ea771f3e25850319f2fecfc91400fc1b9df606ef) )
	ROM_LOAD( "j05_20.bin",   0x08000, 0x4000, CRC(64c137a4) SHA1(e38adeb19e24357cc5581f0a3097c1d24914e25c) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "cx08.13b",     0x00000, 0x2000, CRC(dbd7c1c2) SHA1(57cba817c4499a2677866911a8df5df26f899b8f) )   /* characters */

	ROM_REGION( 0x18000, "gfx2", 0 )
	ROM_LOAD( "cx04.11j",     0x00000, 0x8000, CRC(506a2ed9) SHA1(ee348925f6602dc8a1dbe66df5de615e413cb7af) )
	ROM_LOAD( "cx02.8j",      0x08000, 0x8000, CRC(009dde6a) SHA1(0b892229854c1b291ea923be8b11efee3afbf96e) )
	ROM_LOAD( "cx06.13j",     0x10000, 0x8000, CRC(d819a3b2) SHA1(342db49b807e9b8980dc0e3092fc1305050563be) )

	ROM_REGION( 0xc000, "gfx3", 0 )
	ROM_LOAD( "cx03.9j",      0x00000, 0x4000, CRC(682fd1c4) SHA1(ff98ec6f5166b0b1d10a98ca1c30992b6d0c53a6) )   /* sprites */
	ROM_LOAD( "cx01.7j",      0x04000, 0x4000, CRC(85130b46) SHA1(c4d123174bd107eb5ed6d869416d7d241a32c15e) )
	ROM_LOAD( "cx05.12j",     0x08000, 0x4000, CRC(f7c4f3dc) SHA1(8d99d952c93991038144098e19a1a75106d37821) )

	ROM_REGION( 0x8000, "gfx4", 0 )
	ROM_LOAD( "cx09.17d",     0x00000, 0x4000, CRC(37a082cf) SHA1(057cd3429b021827d14dce9f070c8e13008d6ef7) )   /* tiles */
	ROM_LOAD( "cx10.17e",     0x04000, 0x4000, CRC(ab9446c5) SHA1(9afdc830efb263e5a95c73359cf808d06f23f47a) )

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "82s135.2a",    0x0000, 0x0100, CRC(0e723a83) SHA1(51d2274be70506308b3bfa9c2d23606290f8b3b5) )    /* red and green component */
	ROM_LOAD( "82s129.1a",    0x0100, 0x0100, CRC(d345cbb3) SHA1(6318022ebbbe59d4c0a207801fffed1167b98a66) )    /* blue component */
ROM_END

ROM_START( ringking2 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "rkngm1.bin",   0x00000, 0x8000, CRC(086921ea) SHA1(c5a594be0738a80c5f912dc819332ff61aa6fc4b) )
	ROM_LOAD( "rkngm2.bin",   0x08000, 0x4000, CRC(c0b636a4) SHA1(c3640a5597242e735673e1dbf8bf866e9122a20f) )

	ROM_REGION( 0x10000, "video", 0 )     /* 64k for the video cpu */
	ROM_LOAD( "rkngtram.bin", 0x00000, 0x4000, CRC(d9dc1a0a) SHA1(969608e6e8f3bed11721e657403bf961551a9e38) )

	ROM_REGION( 0x10000, "sprite", 0 )     /* 64k for the sprite cpu */
	ROM_LOAD( "cx00.4c",      0x00000, 0x2000, CRC(880b8aa7) SHA1(e5ee80cac85a62ae5a677115a74c08e433cd4fc9) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "cx12.4ef",     0x00000, 0x8000, CRC(1d5d6c6b) SHA1(ea771f3e25850319f2fecfc91400fc1b9df606ef) )
	ROM_LOAD( "j05_20.bin",   0x08000, 0x4000, CRC(64c137a4) SHA1(e38adeb19e24357cc5581f0a3097c1d24914e25c) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "cx08.13b",     0x00000, 0x2000, CRC(dbd7c1c2) SHA1(57cba817c4499a2677866911a8df5df26f899b8f) )   /* characters */

	ROM_REGION( 0x18000, "gfx2", 0 )
	ROM_LOAD( "cx04.11j",     0x00000, 0x8000, CRC(506a2ed9) SHA1(ee348925f6602dc8a1dbe66df5de615e413cb7af) )
	ROM_LOAD( "cx02.8j",      0x08000, 0x8000, CRC(009dde6a) SHA1(0b892229854c1b291ea923be8b11efee3afbf96e) )
	ROM_LOAD( "cx06.13j",     0x10000, 0x8000, CRC(d819a3b2) SHA1(342db49b807e9b8980dc0e3092fc1305050563be) )

	ROM_REGION( 0xc000, "gfx3", 0 )
	ROM_LOAD( "cx03.9j",      0x00000, 0x4000, CRC(682fd1c4) SHA1(ff98ec6f5166b0b1d10a98ca1c30992b6d0c53a6) )   /* sprites */
	ROM_LOAD( "cx01.7j",      0x04000, 0x4000, CRC(85130b46) SHA1(c4d123174bd107eb5ed6d869416d7d241a32c15e) )
	ROM_LOAD( "cx05.12j",     0x08000, 0x4000, CRC(f7c4f3dc) SHA1(8d99d952c93991038144098e19a1a75106d37821) )

	ROM_REGION( 0x8000, "gfx4", 0 )
	ROM_LOAD( "cx09.17d",     0x00000, 0x4000, CRC(37a082cf) SHA1(057cd3429b021827d14dce9f070c8e13008d6ef7) )   /* tiles */
	ROM_LOAD( "cx10.17e",     0x04000, 0x4000, CRC(ab9446c5) SHA1(9afdc830efb263e5a95c73359cf808d06f23f47a) )

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "82s135.2a",    0x0000, 0x0100, CRC(0e723a83) SHA1(51d2274be70506308b3bfa9c2d23606290f8b3b5) )    /* red and green component */
	ROM_LOAD( "82s129.1a",    0x0100, 0x0100, CRC(d345cbb3) SHA1(6318022ebbbe59d4c0a207801fffed1167b98a66) )    /* blue component */
ROM_END

ROM_START( ringking3 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "14.9d",        0x00000, 0x4000, CRC(63627b8b) SHA1(eea736c8eec59fa561b9d1b5aa43df5410d8dde7) )
	ROM_LOAD( "15.9e",        0x04000, 0x4000, CRC(e7557489) SHA1(49dce8f6ce26283fbdca17d75699de4d636a900a) )
	ROM_LOAD( "16.9f",        0x08000, 0x4000, CRC(a3b3bb16) SHA1(4b4cb95a6bf4608ada1669208d9cabc3f856585a) )

	ROM_REGION( 0x10000, "video", 0 )     /* 64k for the video cpu */
	ROM_LOAD( "13.9b",        0x00000, 0x4000, CRC(f33f94a2) SHA1(58e9eef6525f6bbec4d9586e1fe5884a8af84739) )

	ROM_REGION( 0x10000, "sprite", 0 )     /* 64k for the sprite cpu */
	ROM_LOAD( "j09_dcr.bin",  0x00000, 0x2000, CRC(379f4f84) SHA1(c8171e15fe243857b6ca8f32c1cc09f12fa4c07c) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "f05_18.bin",   0x00000, 0x4000, CRC(c057e28e) SHA1(714d8f14d55a070efcf205f8946269181bf2198b) )
	ROM_LOAD( "h05_19.bin",   0x04000, 0x4000, CRC(060253dd) SHA1(9a24fc6aca64262e935971f96b3a103df9711f20) )
	ROM_LOAD( "j05_20.bin",   0x08000, 0x4000, CRC(64c137a4) SHA1(e38adeb19e24357cc5581f0a3097c1d24914e25c) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "12.15d",       0x00000, 0x2000, CRC(988a77bf) SHA1(c047c076d47479448ce2454c10010b672a1b457d) ) /* characters (Japanese) */

	ROM_REGION( 0x18000, "gfx2", 0 )    /* sprites */
	ROM_LOAD( "vb01_01.bin",  0x00000, 0x4000, CRC(ce6580af) SHA1(9a94c681d4c54ca6c2f41ba1e51c61f54e844c77) )
	ROM_LOAD( "vb04_03.bin",  0x04000, 0x4000, CRC(cf74ea50) SHA1(9b0bdf636f9b31e6c7074d606d431a849a51e518) )
	ROM_LOAD( "vb07_05.bin",  0x08000, 0x4000, CRC(d8b53975) SHA1(52ad0b26fef7bb20d1bf953c5ebd519656682bac) )
	ROM_LOAD( "vb03_02.bin",  0x0c000, 0x4000, CRC(4ab506d2) SHA1(8c293d38429a1462f49462d623c47c402e3372f0) )
	ROM_LOAD( "vb05_04.bin",  0x10000, 0x4000, CRC(ecf95a2c) SHA1(b93d0ebdbde9311194a91fb3d6e5d5f33cc87e9d) )
	ROM_LOAD( "vb08_06.bin",  0x14000, 0x4000, CRC(8200cb2b) SHA1(c9e66027d796dd523eddf378d0e9a62ebcc8f6c8) )

	ROM_REGION( 0xc000, "gfx3", 0 ) /* tiles */
	ROM_LOAD( "7.1d",         0x00000, 0x2000, CRC(019a88b0) SHA1(9c2d4bb643b7bd14c4f347906707854d7a5cd340) )
	ROM_LOAD( "9.4d",         0x02000, 0x2000, CRC(bfdc741a) SHA1(2b874ef61eae8fab99d08a0273d69b90bb52b3f1) )
	ROM_LOAD( "11.7d",        0x04000, 0x2000, CRC(3cc7bdc5) SHA1(31f3fd5892232701f375822a146853b71bad804b) )
	ROM_LOAD( "8.3d",         0x06000, 0x2000, CRC(65f1281b) SHA1(a7db40464d52c615ffa40a577edf09fd6b1a677a) )
	ROM_LOAD( "10.5d",        0x08000, 0x2000, CRC(af5013e7) SHA1(26e737138ab0e8dc28bea1f81d1f83345419e611) )
	ROM_LOAD( "12.8d",        0x0a000, 0x2000, CRC(1f6654d6) SHA1(edd234b6daeaeaad335c8c725380bebd5c11063e) )

	ROM_REGION( 0x0300, "proms", 0 )
	/* we load the ringking PROMs and then expand the first to look like the kingofb ones... */
	ROM_LOAD( "82s135.2a",    0x0100, 0x0100, CRC(0e723a83) SHA1(51d2274be70506308b3bfa9c2d23606290f8b3b5) )    /* red and green component */
	ROM_LOAD( "82s129.1a",    0x0200, 0x0100, CRC(d345cbb3) SHA1(6318022ebbbe59d4c0a207801fffed1167b98a66) )    /* blue component */
ROM_END

ROM_START( ringkingw )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "15.9d",        0x00000, 0x4000, CRC(8263f517) SHA1(942012bfcc98dd2cd0437e015a164933c99d0f36) )
	ROM_LOAD( "16.9e",        0x04000, 0x4000, CRC(daadd700) SHA1(2405e954a28d18ae8c30955d0ad7c25c9abb2bd3) )

	ROM_REGION( 0x10000, "video", 0 )     /* 64k for the video cpu */
	ROM_LOAD( "14.9b",        0x00000, 0x4000, CRC(76a73c95) SHA1(ca47917d8843b2867b66f74c6bc2f29bb90e11dc) )

	ROM_REGION( 0x10000, "sprite", 0 )     /* 64k for the sprite cpu */
	ROM_LOAD( "17.xx",        0x00000, 0x2000, CRC(379f4f84) SHA1(c8171e15fe243857b6ca8f32c1cc09f12fa4c07c) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "18.4f",        0x00000, 0x4000, CRC(c057e28e) SHA1(714d8f14d55a070efcf205f8946269181bf2198b) )
	ROM_LOAD( "19.4h",        0x04000, 0x4000, CRC(060253dd) SHA1(9a24fc6aca64262e935971f96b3a103df9711f20) )
	ROM_LOAD( "20.4j",        0x08000, 0x4000, CRC(64c137a4) SHA1(e38adeb19e24357cc5581f0a3097c1d24914e25c) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "13.14d",       0x00000, 0x2000, CRC(e36d4f4f) SHA1(059799b04a7d3e02c1a7f9a5b878d06afef305df) ) /* characters */

	ROM_REGION( 0x18000, "gfx2", 0 )    /* sprites */
	ROM_LOAD( "1.1b",         0x00000, 0x4000, CRC(ce6580af) SHA1(9a94c681d4c54ca6c2f41ba1e51c61f54e844c77) )
	ROM_LOAD( "3.3b",         0x04000, 0x4000, CRC(cf74ea50) SHA1(9b0bdf636f9b31e6c7074d606d431a849a51e518) )
	ROM_LOAD( "5.5b",         0x08000, 0x4000, CRC(d8b53975) SHA1(52ad0b26fef7bb20d1bf953c5ebd519656682bac) )
	ROM_LOAD( "2.2b",         0x0c000, 0x4000, CRC(4ab506d2) SHA1(8c293d38429a1462f49462d623c47c402e3372f0) )
	ROM_LOAD( "4.4b",         0x10000, 0x4000, CRC(ecf95a2c) SHA1(b93d0ebdbde9311194a91fb3d6e5d5f33cc87e9d) )
	ROM_LOAD( "6.6b",         0x14000, 0x4000, CRC(8200cb2b) SHA1(c9e66027d796dd523eddf378d0e9a62ebcc8f6c8) )

	ROM_REGION( 0xc000, "gfx3", 0 ) /* tiles */
	ROM_LOAD( "7.1d",         0x00000, 0x2000, CRC(019a88b0) SHA1(9c2d4bb643b7bd14c4f347906707854d7a5cd340) )
	ROM_LOAD( "9.1d",         0x02000, 0x2000, CRC(bfdc741a) SHA1(2b874ef61eae8fab99d08a0273d69b90bb52b3f1) )
	ROM_LOAD( "11.1d",        0x04000, 0x2000, CRC(3cc7bdc5) SHA1(31f3fd5892232701f375822a146853b71bad804b) )
	ROM_LOAD( "8.1d",         0x06000, 0x2000, CRC(65f1281b) SHA1(a7db40464d52c615ffa40a577edf09fd6b1a677a) )
	ROM_LOAD( "10.1d",        0x08000, 0x2000, CRC(af5013e7) SHA1(26e737138ab0e8dc28bea1f81d1f83345419e611) )
	ROM_LOAD( "12.1d",        0x0a000, 0x2000, CRC(1f6654d6) SHA1(edd234b6daeaeaad335c8c725380bebd5c11063e) )

	ROM_REGION( 0x0300, "proms", ROMREGION_ERASE00 )
	/* PROMs are encoded here like the kingofb ones... */

	ROM_REGION( 0x0c00, "user1", 0 ) /* color proms */
	ROM_LOAD( "prom2.bin",    0x0000, 0x0400, CRC(8ce34029) SHA1(b5150afe72ced9a396997dc11691a4ac4ed2cf2a) ) /* red component */
	ROM_LOAD( "prom3.bin",    0x0400, 0x0400, CRC(54cfe913) SHA1(d93f4bbf232e3893b953470e3e8f66426b4d9a64) ) /* green component */
	ROM_LOAD( "prom1.bin",    0x0800, 0x0400, CRC(913f5975) SHA1(3d1e40eeb4d5a3a4bd42ec73d05bfca13b2f1805) ) /* blue component */
ROM_END

DRIVER_INIT_MEMBER(kingofb_state,ringking3)
{
	int i;
	UINT8 *RAM = memregion("proms")->base();

	/* expand the first color PROM to look like the kingofb ones... */
	for (i = 0; i < 0x100; i++)
		RAM[i] = RAM[i + 0x100] >> 4;
	m_palette->update();
}

DRIVER_INIT_MEMBER(kingofb_state,ringkingw)
{
	int i,j,k;
	UINT8 *PROMS = memregion("proms")->base();
	UINT8 *USER1 = memregion("user1")->base();

	/* change the PROMs encode in a simple format to use kingofb decode */
	for(i = 0, j = 0; j < 0x40; i++, j++)
	{
		if((i & 0xf) == 8)
			i += 8;

		for(k = 0; k <= 3; k++)
		{
			PROMS[j + 0x000 + 0x40 * k] = USER1[i + 0x000 + 0x100 * k]; /* R */
			PROMS[j + 0x100 + 0x40 * k] = USER1[i + 0x400 + 0x100 * k]; /* G */
			PROMS[j + 0x200 + 0x40 * k] = USER1[i + 0x800 + 0x100 * k]; /* B */
		}
	}
	m_palette->update();
}


GAME( 1985, kingofb,   0,       kingofb,  kingofb, driver_device,  0,        ROT90, "Wood Place Inc.", "King of Boxer (English)", MACHINE_SUPPORTS_SAVE )
GAME( 1985, ringking,  kingofb, ringking, ringking, driver_device, 0,        ROT90, "Wood Place Inc. (Data East USA license)", "Ring King (US set 1)", MACHINE_SUPPORTS_SAVE )
GAME( 1985, ringking2, kingofb, ringking, ringking, driver_device, 0,        ROT90, "Wood Place Inc. (Data East USA license)", "Ring King (US set 2)", MACHINE_SUPPORTS_SAVE )
GAME( 1985, ringking3, kingofb, kingofb,  kingofb, kingofb_state,  ringking3,ROT90, "Wood Place Inc. (Data East USA license)", "Ring King (US set 3)", MACHINE_SUPPORTS_SAVE )
GAME( 1985, ringkingw, kingofb, kingofb,  kingofb, kingofb_state,  ringkingw,ROT90, "Wood Place Inc.", "Ring King (US, Wood Place Inc.)", MACHINE_SUPPORTS_SAVE )
