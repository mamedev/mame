// license:BSD-3-Clause
// copyright-holders:Ernesto Corvi
/***************************************************************************

King of Boxer - (c) 1985 Woodplace Inc.
Ring King     - (c) 1985 Data East USA Inc. / Woodplace Inc.

Driver by:
Ernesto Corvi
ernesto@imagina.com

Notes:
-----
Main CPU:
- There's a memory area from 0xf000 to 0xf7ff, which is clearly
  initialized at startup and never used anymore.

***************************************************************************/

#include "emu.h"
#include "includes/kingobox.h"

#include "cpu/z80/z80.h"
#include "machine/clock.h"
#include "sound/ay8910.h"
#include "sound/dac.h"
#include "sound/volt_reg.h"
#include "screen.h"
#include "speaker.h"


void kingofb_state::video_interrupt_w(uint8_t data)
{
	m_video_cpu->set_input_line_and_vector(0, HOLD_LINE, 0xff); // Z80
}

void kingofb_state::sprite_interrupt_w(uint8_t data)
{
	m_sprite_cpu->set_input_line_and_vector(0, HOLD_LINE, 0xff); // Z80
}

void kingofb_state::scroll_interrupt_w(uint8_t data)
{
	sprite_interrupt_w(data);
	*m_scroll_y = data;
}

void kingofb_state::sound_command_w(uint8_t data)
{
	m_soundlatch->write(data);
	m_audiocpu->set_input_line_and_vector(0, HOLD_LINE, 0xff); // Z80
}


void kingofb_state::kingobox_map(address_map &map)
{
	map(0x0000, 0xbfff).rom();
	map(0xc000, 0xc3ff).ram(); /* work ram */
	map(0xe000, 0xe7ff).ram().share("share2"); /* shared with sprite cpu */
	map(0xe800, 0xefff).ram().share("share1"); /* shared with video cpu */
	map(0xf000, 0xf7ff).ram(); /* ???? */
	map(0xf800, 0xf800).w(FUNC(kingofb_state::kingofb_f800_w));   /* NMI enable, palette bank */
	map(0xf801, 0xf801).nopw(); /* ???? */
	map(0xf802, 0xf802).writeonly().share("scroll_y");
	map(0xf803, 0xf803).w(FUNC(kingofb_state::scroll_interrupt_w));
	map(0xf804, 0xf804).w(FUNC(kingofb_state::video_interrupt_w));
	map(0xf807, 0xf807).w(FUNC(kingofb_state::sound_command_w)); /* sound latch */
	map(0xfc00, 0xfc00).portr("DSW1");
	map(0xfc01, 0xfc01).portr("DSW2");
	map(0xfc02, 0xfc02).portr("P1");
	map(0xfc03, 0xfc03).portr("P2");
	map(0xfc04, 0xfc04).portr("SYSTEM");
	map(0xfc05, 0xfc05).portr("EXTRA");
}

void kingofb_state::kingobox_video_map(address_map &map)
{
	map(0x0000, 0x3fff).rom();
	map(0x8000, 0x87ff).ram(); /* work ram */
	map(0xa000, 0xa7ff).ram().share("share1"); /* shared with main */
	map(0xc000, 0xc0ff).ram().w(FUNC(kingofb_state::kingofb_videoram_w)).share("videoram"); /* background vram */
	map(0xc400, 0xc4ff).ram().w(FUNC(kingofb_state::kingofb_colorram_w)).share("colorram"); /* background colorram */
	map(0xc800, 0xcbff).ram().w(FUNC(kingofb_state::kingofb_videoram2_w)).share("videoram2"); /* foreground vram */
	map(0xcc00, 0xcfff).ram().w(FUNC(kingofb_state::kingofb_colorram2_w)).share("colorram2"); /* foreground colorram */
}

void kingofb_state::kingobox_sprite_map(address_map &map)
{
	map(0x0000, 0x1fff).rom();
	map(0x8000, 0x87ff).ram(); /* work ram */
	map(0xa000, 0xa7ff).ram().share("share2"); /* shared with main */
	map(0xc000, 0xc3ff).ram().share("spriteram"); /* sprite ram */
	map(0xc400, 0xc43f).ram();  /* something related to scroll? */
}

void kingofb_state::kingobox_sound_map(address_map &map)
{
	map(0x0000, 0xbfff).rom();
	map(0x8000, 0x8000).nopw(); /* ??? */
	map(0xc000, 0xc3ff).ram(); /* work ram */
}

void kingofb_state::kingobox_sound_io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x00).w("dac", FUNC(dac_byte_interface::data_w));
	map(0x08, 0x08).rw("aysnd", FUNC(ay8910_device::data_r), FUNC(ay8910_device::data_w));
	map(0x0c, 0x0c).w("aysnd", FUNC(ay8910_device::address_w));
}

/* Ring King */
void kingofb_state::ringking_map(address_map &map)
{
	map(0x0000, 0xbfff).rom();
	map(0xc000, 0xc3ff).ram(); /* work ram */
	map(0xc800, 0xcfff).ram().share("share2"); /* shared with sprite cpu */
	map(0xd000, 0xd7ff).ram().share("share1"); /* shared with video cpu */
	map(0xd800, 0xd800).w(FUNC(kingofb_state::kingofb_f800_w));
	map(0xd801, 0xd801).w(FUNC(kingofb_state::sprite_interrupt_w));
	map(0xd802, 0xd802).w(FUNC(kingofb_state::video_interrupt_w));
	map(0xd803, 0xd803).w(FUNC(kingofb_state::sound_command_w));
	map(0xe000, 0xe000).portr("DSW1");
	map(0xe001, 0xe001).portr("DSW2");
	map(0xe002, 0xe002).portr("P1");
	map(0xe003, 0xe003).portr("P2");
	map(0xe004, 0xe004).portr("SYSTEM");
	map(0xe005, 0xe005).portr("EXTRA");
	map(0xe800, 0xe800).writeonly().share("scroll_y");
	map(0xf000, 0xf7ff).ram(); /* ???? */
}

void kingofb_state::ringking_video_map(address_map &map)
{
	map(0x0000, 0x3fff).rom();
	map(0x8000, 0x87ff).ram(); /* work ram */
	map(0xc000, 0xc7ff).ram().share("share1"); /* shared with main */
	map(0xa800, 0xa8ff).ram().w(FUNC(kingofb_state::kingofb_videoram_w)).share("videoram"); /* background vram */
	map(0xac00, 0xacff).ram().w(FUNC(kingofb_state::kingofb_colorram_w)).share("colorram"); /* background colorram */
	map(0xa000, 0xa3ff).ram().w(FUNC(kingofb_state::kingofb_videoram2_w)).share("videoram2"); /* foreground vram */
	map(0xa400, 0xa7ff).ram().w(FUNC(kingofb_state::kingofb_colorram2_w)).share("colorram2"); /* foreground colorram */
}

void kingofb_state::ringking_sprite_map(address_map &map)
{
	map(0x0000, 0x1fff).rom();
	map(0x8000, 0x87ff).ram(); /* work ram */
	map(0xc800, 0xcfff).ram().share("share2"); /* shared with main */
	map(0xa000, 0xa3ff).ram().share("spriteram"); /* sprite ram */
	map(0xa400, 0xa43f).ram(); /* something related to scroll? */
}

void kingofb_state::ringking_sound_io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x00).w("dac", FUNC(dac_byte_interface::data_w));
	map(0x02, 0x02).r("aysnd", FUNC(ay8910_device::data_r));
	map(0x02, 0x03).w("aysnd", FUNC(ay8910_device::data_address_w));
}


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

static GFXDECODE_START( gfx_kingobox )
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


static GFXDECODE_START( gfx_rk )
	GFXDECODE_ENTRY( "gfx1", 0x00000, rk_charlayout1,  256,  8 )    /* characters */
	GFXDECODE_ENTRY( "gfx1", 0x00000, rk_charlayout2,  256,  8 )    /* characters */
	GFXDECODE_ENTRY( "gfx2", 0x00000, rk_spritelayout,   0, 32 )    /* sprites */
	GFXDECODE_ENTRY( "gfx3", 0x00000, rk_tilelayout,     0, 32 )    /* sprites/bg tiles */
	GFXDECODE_ENTRY( "gfx4", 0x00000, rk_bglayout,       0, 32 )    /* bg tiles */
GFXDECODE_END

void kingofb_state::machine_start()
{
	save_item(NAME(m_palette_bank));
}

void kingofb_state::machine_reset()
{
	kingofb_f800_w(0); // LS174 reset
}

void kingofb_state::kingofb(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, 4000000);        // 4.0 MHz
	m_maincpu->set_addrmap(AS_PROGRAM, &kingofb_state::kingobox_map);

	Z80(config, m_video_cpu, 4000000);      // 4.0 MHz
	m_video_cpu->set_addrmap(AS_PROGRAM, &kingofb_state::kingobox_video_map);

	Z80(config, m_sprite_cpu, 4000000);     // 4.0 MHz
	m_sprite_cpu->set_addrmap(AS_PROGRAM, &kingofb_state::kingobox_sprite_map);

	INPUT_MERGER_ALL_HIGH(config, m_nmigate);
	m_nmigate->output_handler().set_inputline(m_maincpu, INPUT_LINE_NMI);
	m_nmigate->output_handler().append_inputline(m_video_cpu, INPUT_LINE_NMI);
	m_nmigate->output_handler().append_inputline(m_sprite_cpu, INPUT_LINE_NMI);

	Z80(config, m_audiocpu, 4000000);       // 4.0 MHz
	m_audiocpu->set_addrmap(AS_PROGRAM, &kingofb_state::kingobox_sound_map);
	m_audiocpu->set_addrmap(AS_IO, &kingofb_state::kingobox_sound_io_map);

	CLOCK(config, "soundnmi", 6000).signal_handler().set_inputline(m_audiocpu, INPUT_LINE_NMI);

	config.m_minimum_quantum = attotime::from_hz(6000); // We really need heavy synching among the processors


	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500) /* not accurate */);
	screen.set_size(32*8, 32*8);
	screen.set_visarea(0*8, 32*8-1, 2*8, 30*8-1);
	screen.set_screen_update(FUNC(kingofb_state::screen_update_kingofb));
	screen.set_palette(m_palette);
	screen.screen_vblank().set(m_nmigate, FUNC(input_merger_device::in_w<0>));

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_kingobox);
	PALETTE(config, m_palette, FUNC(kingofb_state::kingofb_palette), 256+8*2, 256+8);
	MCFG_VIDEO_START_OVERRIDE(kingofb_state,kingofb)

	/* sound hardware */
	SPEAKER(config, "speaker").front_center();

	GENERIC_LATCH_8(config, m_soundlatch);

	ay8910_device &aysnd(AY8910(config, "aysnd", 1500000));
	aysnd.port_a_read_callback().set(m_soundlatch, FUNC(generic_latch_8_device::read));
	aysnd.add_route(ALL_OUTPUTS, "speaker", 0.25);

	DAC_8BIT_R2R(config, "dac", 0).add_route(ALL_OUTPUTS, "speaker", 0.125); // 100K (R30-44 even)/200K (R31-45 odd) ladder network
	voltage_regulator_device &vref(VOLTAGE_REGULATOR(config, "vref"));
	vref.add_route(0, "dac", 1.0, DAC_VREF_POS_INPUT);
	vref.add_route(0, "dac", -1.0, DAC_VREF_NEG_INPUT);
}


/* Ring King */
void kingofb_state::ringking(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, 4000000);        // 4.0 MHz
	m_maincpu->set_addrmap(AS_PROGRAM, &kingofb_state::ringking_map);

	Z80(config, m_video_cpu, 4000000);      // 4.0 MHz
	m_video_cpu->set_addrmap(AS_PROGRAM, &kingofb_state::ringking_video_map);

	Z80(config, m_sprite_cpu, 4000000);     // 4.0 MHz
	m_sprite_cpu->set_addrmap(AS_PROGRAM, &kingofb_state::ringking_sprite_map);

	INPUT_MERGER_ALL_HIGH(config, m_nmigate);
	m_nmigate->output_handler().set_inputline(m_maincpu, INPUT_LINE_NMI);
	m_nmigate->output_handler().append_inputline(m_video_cpu, INPUT_LINE_NMI);
	m_nmigate->output_handler().append_inputline(m_sprite_cpu, INPUT_LINE_NMI);

	Z80(config, m_audiocpu, 4000000);       // 4.0 MHz
	m_audiocpu->set_addrmap(AS_PROGRAM, &kingofb_state::kingobox_sound_map);
	m_audiocpu->set_addrmap(AS_IO, &kingofb_state::ringking_sound_io_map);

	CLOCK(config, "soundnmi", 6000).signal_handler().set_inputline(m_audiocpu, INPUT_LINE_NMI);

	config.m_minimum_quantum = attotime::from_hz(6000); // We really need heavy synching among the processors


	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500) /* not accurate */);
	screen.set_size(32*8, 32*8);
	screen.set_visarea(0*8, 32*8-1, 2*8, 30*8-1);
	screen.set_screen_update(FUNC(kingofb_state::screen_update_ringking));
	screen.set_palette(m_palette);
	screen.screen_vblank().set(m_nmigate, FUNC(input_merger_device::in_w<0>));

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_rk);
	PALETTE(config, m_palette, FUNC(kingofb_state::ringking_palette), 256+8*2, 256+8);
	MCFG_VIDEO_START_OVERRIDE(kingofb_state,ringking)

	/* sound hardware */
	SPEAKER(config, "speaker").front_center();

	GENERIC_LATCH_8(config, m_soundlatch);

	ay8910_device &aysnd(AY8910(config, "aysnd", 1500000));
	aysnd.port_a_read_callback().set(m_soundlatch, FUNC(generic_latch_8_device::read));
	aysnd.add_route(ALL_OUTPUTS, "speaker", 0.25);

	DAC_8BIT_R2R(config, "dac", 0).add_route(ALL_OUTPUTS, "speaker", 0.125); // unknown DAC
	voltage_regulator_device &vref(VOLTAGE_REGULATOR(config, "vref"));
	vref.add_route(0, "dac", 1.0, DAC_VREF_POS_INPUT);
	vref.add_route(0, "dac", -1.0, DAC_VREF_NEG_INPUT);
}


/***************************************************************************

  Game driver(s)

***************************************************************************/

/* King of Boxer sets */
ROM_START( kingofb )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "22.d9",  0x00000, 0x4000, CRC(6220bfa2) SHA1(cb329406ed07b71f9d2c40fc6c2c196daaa56fc8) )
	ROM_LOAD( "23.e9",  0x04000, 0x4000, CRC(5782fdd8) SHA1(6c8c1114ce7863f9e8331796e2c5fb4928904b55) )

	ROM_REGION( 0x10000, "video", 0 )     /* 64k for the video cpu */
	ROM_LOAD( "21.b9",  0x00000, 0x4000, CRC(3fb39489) SHA1(cddd939cb57bb684427cf5c8538ad0e9f8f4586d) )

	ROM_REGION( 0x10000, "sprite", 0 )     /* 64k for the sprite cpu */
	ROM_LOAD( "17.j9",  0x00000, 0x2000, CRC(379f4f84) SHA1(c8171e15fe243857b6ca8f32c1cc09f12fa4c07c) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "18.f4",  0x00000, 0x4000, CRC(c057e28e) SHA1(714d8f14d55a070efcf205f8946269181bf2198b) )
	ROM_LOAD( "19.h4",  0x04000, 0x4000, CRC(060253dd) SHA1(9a24fc6aca64262e935971f96b3a103df9711f20) )
	ROM_LOAD( "20.j4",  0x08000, 0x4000, CRC(64c137a4) SHA1(e38adeb19e24357cc5581f0a3097c1d24914e25c) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "13.d14", 0x00000, 0x2000, CRC(e36d4f4f) SHA1(059799b04a7d3e02c1a7f9a5b878d06afef305df) ) /* characters */

	ROM_REGION( 0x18000, "gfx2", 0 )    /* sprites */
	ROM_LOAD( "1.b1",   0x00000, 0x4000, CRC(ce6580af) SHA1(9a94c681d4c54ca6c2f41ba1e51c61f54e844c77) )
	ROM_LOAD( "3.b4",   0x04000, 0x4000, CRC(cf74ea50) SHA1(9b0bdf636f9b31e6c7074d606d431a849a51e518) )
	ROM_LOAD( "5.b7",   0x08000, 0x4000, CRC(d8b53975) SHA1(52ad0b26fef7bb20d1bf953c5ebd519656682bac) )
	ROM_LOAD( "2.b3",   0x0c000, 0x4000, CRC(4ab506d2) SHA1(8c293d38429a1462f49462d623c47c402e3372f0) )
	ROM_LOAD( "4.b5",   0x10000, 0x4000, CRC(ecf95a2c) SHA1(b93d0ebdbde9311194a91fb3d6e5d5f33cc87e9d) )
	ROM_LOAD( "6.b8",   0x14000, 0x4000, CRC(8200cb2b) SHA1(c9e66027d796dd523eddf378d0e9a62ebcc8f6c8) )

	ROM_REGION( 0xc000, "gfx3", 0 ) /* tiles */
	ROM_LOAD( "7.d1",   0x00000, 0x2000, CRC(3d472a22) SHA1(85a2e25cee8f85ac0d2ee12f60f97e26539ebd52) )
	ROM_LOAD( "9.d4",   0x02000, 0x2000, CRC(cc002ea9) SHA1(194aa60809c2ae12be2d7533170988e47549c239) )
	ROM_LOAD( "11.d7",  0x04000, 0x2000, CRC(23c1b3ee) SHA1(8a8a187920243f3d3870a2fa71b0f6494e53107a) )
	ROM_LOAD( "8.d3",   0x06000, 0x2000, CRC(d6b1b8fe) SHA1(6bbe02a0a9e080f3ed3c32d64afb81905b42082f) )
	ROM_LOAD( "10.d5",  0x08000, 0x2000, CRC(fce71e5a) SHA1(a68ad30e8e207d24bc5543dcbcbc3e39260b6cc5) )
	ROM_LOAD( "12.d8",  0x0a000, 0x2000, CRC(3f68b991) SHA1(487e7d793fe6c1dbecd5f54f790105bbb44a21de) )

	ROM_REGION( 0x0300, "proms", 0 )
	ROM_LOAD( "vb14_col.bin", 0x0000, 0x0100, CRC(c58e5121) SHA1(2e6658e24c183d8dacf4ff84a38060e57d11f265) )    /* red component */
	ROM_LOAD( "vb15_col.bin", 0x0100, 0x0100, CRC(5ab06f25) SHA1(f5e0aabf40ce6d11771e0678fea248abd5b95b3c) )    /* green component */
	ROM_LOAD( "vb16_col.bin", 0x0200, 0x0100, CRC(1171743f) SHA1(ddfce0ff213381a2fc94337681e599cb28db840c) )    /* blue component */

	ROM_REGION( 0x0200, "plds", 0 )
	ROM_LOAD( "pal12h6-vh02.bin",   0x0000, 0x0034, CRC(6cc0fdf2) SHA1(12eef0d49def671aa7dbb4cce91cfbe40697dcea) )
	ROM_LOAD( "pal14h4-vh07.bin",   0x0100, 0x003c, CRC(7e59d45a) SHA1(4a900e424c9edc9f8664f935edccbe4b4759188a) )
ROM_END

ROM_START( kingofbj )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "22.d9",  0x00000, 0x4000, CRC(6220bfa2) SHA1(cb329406ed07b71f9d2c40fc6c2c196daaa56fc8) ) // 27128
	ROM_LOAD( "23.e9",  0x04000, 0x4000, CRC(5782fdd8) SHA1(6c8c1114ce7863f9e8331796e2c5fb4928904b55) ) // 27128

	ROM_REGION( 0x10000, "video", 0 )     /* 64k for the video cpu */
	ROM_LOAD( "21.b9",  0x00000, 0x4000, CRC(3fb39489) SHA1(cddd939cb57bb684427cf5c8538ad0e9f8f4586d) ) // 27128

	ROM_REGION( 0x10000, "sprite", 0 )     /* 64k for the sprite cpu */
	ROM_LOAD( "17.j9",  0x00000, 0x2000, CRC(379f4f84) SHA1(c8171e15fe243857b6ca8f32c1cc09f12fa4c07c) ) // 2764

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "18.f4",  0x00000, 0x4000, CRC(c057e28e) SHA1(714d8f14d55a070efcf205f8946269181bf2198b) ) // 27128
	ROM_LOAD( "19.h4",  0x04000, 0x4000, CRC(060253dd) SHA1(9a24fc6aca64262e935971f96b3a103df9711f20) ) // 27128
	ROM_LOAD( "20.j4",  0x08000, 0x4000, CRC(64c137a4) SHA1(e38adeb19e24357cc5581f0a3097c1d24914e25c) ) // 27128

	ROM_REGION( 0x2000, "gfx1", 0 )     /* characters (Japanese) */
	ROM_LOAD( "13.d14", 0x00000, 0x2000, CRC(988a77bf) SHA1(c047c076d47479448ce2454c10010b672a1b457d) ) // 2764 - Same label different data than kingofb

	ROM_REGION( 0x18000, "gfx2", 0 )    /* sprites */
	ROM_LOAD( "1.b1",   0x00000, 0x4000, CRC(7b6f390e) SHA1(8338d6af1c1825ce0b1fc118f33f49a4f7222b9c) ) // 27128 - Same label different data than kingofb
	ROM_LOAD( "3.b4",   0x04000, 0x4000, CRC(cf74ea50) SHA1(9b0bdf636f9b31e6c7074d606d431a849a51e518) ) // 27128
	ROM_LOAD( "5.b7",   0x08000, 0x4000, CRC(d8b53975) SHA1(52ad0b26fef7bb20d1bf953c5ebd519656682bac) ) // 27128
	ROM_LOAD( "2.b3",   0x0c000, 0x4000, CRC(4ab506d2) SHA1(8c293d38429a1462f49462d623c47c402e3372f0) ) // 27128
	ROM_LOAD( "4.b5",   0x10000, 0x4000, CRC(ecf95a2c) SHA1(b93d0ebdbde9311194a91fb3d6e5d5f33cc87e9d) ) // 27128
	ROM_LOAD( "6.b8",   0x14000, 0x4000, CRC(8200cb2b) SHA1(c9e66027d796dd523eddf378d0e9a62ebcc8f6c8) ) // 27128

	ROM_REGION( 0xc000, "gfx3", 0 ) /* tiles */
	ROM_LOAD( "7.d1",   0x00000, 0x2000, CRC(3d472a22) SHA1(85a2e25cee8f85ac0d2ee12f60f97e26539ebd52) ) // 2764
	ROM_LOAD( "9.d4",   0x02000, 0x2000, CRC(cc002ea9) SHA1(194aa60809c2ae12be2d7533170988e47549c239) ) // 2764
	ROM_LOAD( "11.d7",  0x04000, 0x2000, CRC(23c1b3ee) SHA1(8a8a187920243f3d3870a2fa71b0f6494e53107a) ) // 2764
	ROM_LOAD( "8.d3",   0x06000, 0x2000, CRC(d6b1b8fe) SHA1(6bbe02a0a9e080f3ed3c32d64afb81905b42082f) ) // 2764
	ROM_LOAD( "10.d5",  0x08000, 0x2000, CRC(fce71e5a) SHA1(a68ad30e8e207d24bc5543dcbcbc3e39260b6cc5) ) // 2764
	ROM_LOAD( "12.d8",  0x0a000, 0x2000, CRC(3f68b991) SHA1(487e7d793fe6c1dbecd5f54f790105bbb44a21de) ) // 2764

	ROM_REGION( 0x0300, "proms", 0 ) // not dumped for this set, probably identical
	ROM_LOAD( "vb14_col.bin", 0x0000, 0x0100, CRC(c58e5121) SHA1(2e6658e24c183d8dacf4ff84a38060e57d11f265) )    /* red component */
	ROM_LOAD( "vb15_col.bin", 0x0100, 0x0100, CRC(5ab06f25) SHA1(f5e0aabf40ce6d11771e0678fea248abd5b95b3c) )    /* green component */
	ROM_LOAD( "vb16_col.bin", 0x0200, 0x0100, CRC(1171743f) SHA1(ddfce0ff213381a2fc94337681e599cb28db840c) )    /* blue component */

	ROM_REGION( 0x0200, "plds", 0 )  // not dumped for this set, probably identical
	ROM_LOAD( "pal12h6-vh02.bin",   0x0000, 0x0034, CRC(6cc0fdf2) SHA1(12eef0d49def671aa7dbb4cce91cfbe40697dcea) )
	ROM_LOAD( "pal14h4-vh07.bin",   0x0100, 0x003c, CRC(7e59d45a) SHA1(4a900e424c9edc9f8664f935edccbe4b4759188a) )
ROM_END

/* Ring King sets */
ROM_START( ringkingw )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "15.d9",        0x00000, 0x4000, CRC(8263f517) SHA1(942012bfcc98dd2cd0437e015a164933c99d0f36) )
	ROM_LOAD( "16.e9",        0x04000, 0x4000, CRC(daadd700) SHA1(2405e954a28d18ae8c30955d0ad7c25c9abb2bd3) )

	ROM_REGION( 0x10000, "video", 0 )     /* 64k for the video cpu */
	ROM_LOAD( "14.b9",        0x00000, 0x4000, CRC(76a73c95) SHA1(ca47917d8843b2867b66f74c6bc2f29bb90e11dc) )

	ROM_REGION( 0x10000, "sprite", 0 )     /* 64k for the sprite cpu */
	ROM_LOAD( "17.j9",        0x00000, 0x2000, CRC(379f4f84) SHA1(c8171e15fe243857b6ca8f32c1cc09f12fa4c07c) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "18.f4",  0x00000, 0x4000, CRC(c057e28e) SHA1(714d8f14d55a070efcf205f8946269181bf2198b) )
	ROM_LOAD( "19.h4",  0x04000, 0x4000, CRC(060253dd) SHA1(9a24fc6aca64262e935971f96b3a103df9711f20) )
	ROM_LOAD( "20.j4",  0x08000, 0x4000, CRC(64c137a4) SHA1(e38adeb19e24357cc5581f0a3097c1d24914e25c) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "13.d14", 0x00000, 0x2000, CRC(e36d4f4f) SHA1(059799b04a7d3e02c1a7f9a5b878d06afef305df) ) /* characters */

	ROM_REGION( 0x18000, "gfx2", 0 )    /* sprites */
	ROM_LOAD( "1.b1",   0x00000, 0x4000, CRC(ce6580af) SHA1(9a94c681d4c54ca6c2f41ba1e51c61f54e844c77) )
	ROM_LOAD( "3.b4",   0x04000, 0x4000, CRC(cf74ea50) SHA1(9b0bdf636f9b31e6c7074d606d431a849a51e518) )
	ROM_LOAD( "5.b7",   0x08000, 0x4000, CRC(d8b53975) SHA1(52ad0b26fef7bb20d1bf953c5ebd519656682bac) )
	ROM_LOAD( "2.b3",   0x0c000, 0x4000, CRC(4ab506d2) SHA1(8c293d38429a1462f49462d623c47c402e3372f0) )
	ROM_LOAD( "4.b5",   0x10000, 0x4000, CRC(ecf95a2c) SHA1(b93d0ebdbde9311194a91fb3d6e5d5f33cc87e9d) )
	ROM_LOAD( "6.b8",   0x14000, 0x4000, CRC(8200cb2b) SHA1(c9e66027d796dd523eddf378d0e9a62ebcc8f6c8) )

	ROM_REGION( 0xc000, "gfx3", 0 ) /* tiles */
	ROM_LOAD( "7.d1",   0x00000, 0x2000, CRC(019a88b0) SHA1(9c2d4bb643b7bd14c4f347906707854d7a5cd340) ) // Same label different data than kingofb
	ROM_LOAD( "9.d4",   0x02000, 0x2000, CRC(bfdc741a) SHA1(2b874ef61eae8fab99d08a0273d69b90bb52b3f1) ) // Same label different data than kingofb
	ROM_LOAD( "11.d7",  0x04000, 0x2000, CRC(3cc7bdc5) SHA1(31f3fd5892232701f375822a146853b71bad804b) ) // Same label different data than kingofb
	ROM_LOAD( "8.d3",   0x06000, 0x2000, CRC(65f1281b) SHA1(a7db40464d52c615ffa40a577edf09fd6b1a677a) ) // Same label different data than kingofb
	ROM_LOAD( "10.d5",  0x08000, 0x2000, CRC(af5013e7) SHA1(26e737138ab0e8dc28bea1f81d1f83345419e611) ) // Same label different data than kingofb
	ROM_LOAD( "12.d8",  0x0a000, 0x2000, CRC(1f6654d6) SHA1(edd234b6daeaeaad335c8c725380bebd5c11063e) ) // Same label different data than kingofb

	ROM_REGION( 0x0300, "proms", ROMREGION_ERASE00 )
	/* PROMs are encoded here like the kingofb ones... */

	ROM_REGION( 0x0c00, "user1", 0 ) /* color proms */
	ROM_LOAD( "prom2.bin",    0x0000, 0x0400, CRC(8ce34029) SHA1(b5150afe72ced9a396997dc11691a4ac4ed2cf2a) ) /* red component */
	ROM_LOAD( "prom3.bin",    0x0400, 0x0400, CRC(54cfe913) SHA1(d93f4bbf232e3893b953470e3e8f66426b4d9a64) ) /* green component */
	ROM_LOAD( "prom1.bin",    0x0800, 0x0400, CRC(913f5975) SHA1(3d1e40eeb4d5a3a4bd42ec73d05bfca13b2f1805) ) /* blue component */
ROM_END

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
	ROM_LOAD( "20.j4",        0x08000, 0x4000, CRC(64c137a4) SHA1(e38adeb19e24357cc5581f0a3097c1d24914e25c) )

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
	ROM_LOAD( "20.j4",        0x08000, 0x4000, CRC(64c137a4) SHA1(e38adeb19e24357cc5581f0a3097c1d24914e25c) )

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
	ROM_LOAD( "14.d9",        0x00000, 0x4000, CRC(63627b8b) SHA1(eea736c8eec59fa561b9d1b5aa43df5410d8dde7) )
	ROM_LOAD( "15.e9",        0x04000, 0x4000, CRC(e7557489) SHA1(49dce8f6ce26283fbdca17d75699de4d636a900a) )
	ROM_LOAD( "16.f9",        0x08000, 0x4000, CRC(a3b3bb16) SHA1(4b4cb95a6bf4608ada1669208d9cabc3f856585a) )

	ROM_REGION( 0x10000, "video", 0 )     /* 64k for the video cpu */
	ROM_LOAD( "13.b9",        0x00000, 0x4000, CRC(f33f94a2) SHA1(58e9eef6525f6bbec4d9586e1fe5884a8af84739) )

	ROM_REGION( 0x10000, "sprite", 0 )     /* 64k for the sprite cpu */
	ROM_LOAD( "j09_dcr.bin",  0x00000, 0x2000, CRC(379f4f84) SHA1(c8171e15fe243857b6ca8f32c1cc09f12fa4c07c) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "18.f4",  0x00000, 0x4000, CRC(c057e28e) SHA1(714d8f14d55a070efcf205f8946269181bf2198b) )
	ROM_LOAD( "19.h4",  0x04000, 0x4000, CRC(060253dd) SHA1(9a24fc6aca64262e935971f96b3a103df9711f20) )
	ROM_LOAD( "20.j4",  0x08000, 0x4000, CRC(64c137a4) SHA1(e38adeb19e24357cc5581f0a3097c1d24914e25c) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "12.d15",       0x00000, 0x2000, CRC(988a77bf) SHA1(c047c076d47479448ce2454c10010b672a1b457d) ) /* characters (Japanese) - same data as 13.d14 from kingofbj */

	ROM_REGION( 0x18000, "gfx2", 0 )    /* sprites */
	ROM_LOAD( "1.b1",   0x00000, 0x4000, CRC(ce6580af) SHA1(9a94c681d4c54ca6c2f41ba1e51c61f54e844c77) )
	ROM_LOAD( "3.b4",   0x04000, 0x4000, CRC(cf74ea50) SHA1(9b0bdf636f9b31e6c7074d606d431a849a51e518) )
	ROM_LOAD( "5.b7",   0x08000, 0x4000, CRC(d8b53975) SHA1(52ad0b26fef7bb20d1bf953c5ebd519656682bac) )
	ROM_LOAD( "2.b3",   0x0c000, 0x4000, CRC(4ab506d2) SHA1(8c293d38429a1462f49462d623c47c402e3372f0) )
	ROM_LOAD( "4.b5",   0x10000, 0x4000, CRC(ecf95a2c) SHA1(b93d0ebdbde9311194a91fb3d6e5d5f33cc87e9d) )
	ROM_LOAD( "6.b8",   0x14000, 0x4000, CRC(8200cb2b) SHA1(c9e66027d796dd523eddf378d0e9a62ebcc8f6c8) )

	ROM_REGION( 0xc000, "gfx3", 0 ) /* tiles */
	ROM_LOAD( "7.d1",   0x00000, 0x2000, CRC(019a88b0) SHA1(9c2d4bb643b7bd14c4f347906707854d7a5cd340) ) // Same label different data than kingofb
	ROM_LOAD( "9.d4",   0x02000, 0x2000, CRC(bfdc741a) SHA1(2b874ef61eae8fab99d08a0273d69b90bb52b3f1) ) // Same label different data than kingofb
	ROM_LOAD( "11.d7",  0x04000, 0x2000, CRC(3cc7bdc5) SHA1(31f3fd5892232701f375822a146853b71bad804b) ) // Same label different data than kingofb
	ROM_LOAD( "8.d3",   0x06000, 0x2000, CRC(65f1281b) SHA1(a7db40464d52c615ffa40a577edf09fd6b1a677a) ) // Same label different data than kingofb
	ROM_LOAD( "10.d5",  0x08000, 0x2000, CRC(af5013e7) SHA1(26e737138ab0e8dc28bea1f81d1f83345419e611) ) // Same label different data than kingofb
	ROM_LOAD( "12.d8",  0x0a000, 0x2000, CRC(1f6654d6) SHA1(edd234b6daeaeaad335c8c725380bebd5c11063e) ) // Same label different data than kingofb

	ROM_REGION( 0x0300, "proms", 0 )
	/* we load the ringking PROMs and then expand the first to look like the kingofb ones... */
	ROM_LOAD( "82s135.2a",    0x0100, 0x0100, CRC(0e723a83) SHA1(51d2274be70506308b3bfa9c2d23606290f8b3b5) )    /* red and green component */
	ROM_LOAD( "82s129.1a",    0x0200, 0x0100, CRC(d345cbb3) SHA1(6318022ebbbe59d4c0a207801fffed1167b98a66) )    /* blue component */
ROM_END

void kingofb_state::init_ringking3()
{
	uint8_t *RAM = memregion("proms")->base();

	/* expand the first color PROM to look like the kingofb ones... */
	for (int i = 0; i < 0x100; i++)
		RAM[i] = RAM[i + 0x100] >> 4;
	m_palette->update();
}

void kingofb_state::init_ringkingw()
{
	uint8_t *PROMS = memregion("proms")->base();
	uint8_t *USER1 = memregion("user1")->base();

	/* change the PROMs encode in a simple format to use kingofb decode */
	for (int i = 0, j = 0; j < 0x40; i++, j++)
	{
		if((i & 0xf) == 8)
			i += 8;

		for (int k = 0; k <= 3; k++)
		{
			PROMS[j + 0x000 + 0x40 * k] = USER1[i + 0x000 + 0x100 * k]; /* R */
			PROMS[j + 0x100 + 0x40 * k] = USER1[i + 0x400 + 0x100 * k]; /* G */
			PROMS[j + 0x200 + 0x40 * k] = USER1[i + 0x800 + 0x100 * k]; /* B */
		}
	}
	m_palette->update();
}


GAME( 1985, kingofb,   0,       kingofb,  kingofb,  kingofb_state, empty_init,     ROT90, "Woodplace Inc.", "King of Boxer (World)", MACHINE_SUPPORTS_SAVE )
GAME( 1985, kingofbj,  kingofb, kingofb,  kingofb,  kingofb_state, empty_init,     ROT90, "Woodplace Inc.", "King of Boxer (Japan)", MACHINE_SUPPORTS_SAVE )
GAME( 1985, ringkingw, kingofb, kingofb,  kingofb,  kingofb_state, init_ringkingw, ROT90, "Woodplace Inc.", "Ring King (US, Woodplace Inc.)", MACHINE_SUPPORTS_SAVE )
GAME( 1985, ringking,  kingofb, ringking, ringking, kingofb_state, empty_init,     ROT90, "Woodplace Inc. (Data East USA license)", "Ring King (US set 1)", MACHINE_SUPPORTS_SAVE )
GAME( 1985, ringking2, kingofb, ringking, ringking, kingofb_state, empty_init,     ROT90, "Woodplace Inc. (Data East USA license)", "Ring King (US set 2)", MACHINE_SUPPORTS_SAVE )
GAME( 1985, ringking3, kingofb, kingofb,  kingofb,  kingofb_state, init_ringking3, ROT90, "Woodplace Inc. (Data East USA license)", "Ring King (US set 3)", MACHINE_SUPPORTS_SAVE )
