// license:BSD-3-Clause
// copyright-holders:Uki
/*****************************************************************************

Quiz DNA no Hanran (c) 1992 Face
Quiz Gakuen Paradise (c) 1991 NMK
Quiz Gekiretsu Scramble (Gakuen Paradise 2) (c) 1993 Face

    Driver by Uki

*****************************************************************************/

#include "emu.h"
#include "includes/quizdna.h"

#include "cpu/z80/z80.h"
#include "sound/2203intf.h"
#include "sound/okim6295.h"
#include "screen.h"
#include "speaker.h"

#define MCLK 16000000

WRITE8_MEMBER(quizdna_state::rombank_w)
{
	membank("mainbank")->set_entry(data & 0x3f);
}

WRITE8_MEMBER(quizdna_state::gekiretu_rombank_w)
{
	membank("mainbank")->set_entry((data & 0x3f) ^ 0x0a);
}

/****************************************************************************/

void quizdna_state::quizdna_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0xbfff).bankr("mainbank");
	map(0x8000, 0x9fff).w(FUNC(quizdna_state::fg_ram_w));
	map(0xa000, 0xbfff).w(FUNC(quizdna_state::bg_ram_w));
	map(0xc000, 0xdfff).ram();
	map(0xe000, 0xe1ff).ram().share("spriteram");
	map(0xe200, 0xefff).ram();
	map(0xf000, 0xffff).ram().w(FUNC(quizdna_state::paletteram_xBGR_RRRR_GGGG_BBBB_w)).share("paletteram");
}

void quizdna_state::gekiretu_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0xbfff).bankr("mainbank");
	map(0x8000, 0x9fff).w(FUNC(quizdna_state::fg_ram_w));
	map(0xa000, 0xbfff).w(FUNC(quizdna_state::bg_ram_w));
	map(0xc000, 0xdfff).ram();
	map(0xe000, 0xefff).ram().w(FUNC(quizdna_state::paletteram_xBGR_RRRR_GGGG_BBBB_w)).share("paletteram");
	map(0xf000, 0xf1ff).ram().share("spriteram");
	map(0xf200, 0xffff).ram();
}

void quizdna_state::quizdna_io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x02, 0x03).w(FUNC(quizdna_state::bg_xscroll_w));
	map(0x04, 0x04).w(FUNC(quizdna_state::bg_yscroll_w));
	map(0x05, 0x06).nopw(); /* unknown */
	map(0x80, 0x80).portr("P1");
	map(0x81, 0x81).portr("P2");
	map(0x90, 0x90).portr("SYSTEM");
	map(0x91, 0x91).portr("SERVICE");
	map(0xc0, 0xc0).w(FUNC(quizdna_state::rombank_w));
	map(0xd0, 0xd0).w(FUNC(quizdna_state::screen_ctrl_w));
	map(0xe0, 0xe1).rw("ymsnd", FUNC(ym2203_device::read), FUNC(ym2203_device::write));
	map(0xf0, 0xf0).rw("oki", FUNC(okim6295_device::read), FUNC(okim6295_device::write));
}

void quizdna_state::gakupara_io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x01).w(FUNC(quizdna_state::bg_xscroll_w));
	map(0x02, 0x02).w(FUNC(quizdna_state::bg_yscroll_w));
	map(0x03, 0x04).nopw(); /* unknown */
	map(0x80, 0x80).portr("P1");
	map(0x81, 0x81).portr("P2");
	map(0x90, 0x90).portr("SYSTEM");
	map(0x91, 0x91).portr("SERVICE");
	map(0xc0, 0xc0).w(FUNC(quizdna_state::rombank_w));
	map(0xd0, 0xd0).w(FUNC(quizdna_state::screen_ctrl_w));
	map(0xe0, 0xe1).rw("ymsnd", FUNC(ym2203_device::read), FUNC(ym2203_device::write));
	map(0xf0, 0xf0).rw("oki", FUNC(okim6295_device::read), FUNC(okim6295_device::write));
}

void quizdna_state::gekiretu_io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x02, 0x03).w(FUNC(quizdna_state::bg_xscroll_w));
	map(0x04, 0x04).w(FUNC(quizdna_state::bg_yscroll_w));
	map(0x05, 0x06).nopw(); /* unknown */
	map(0x80, 0x80).portr("P1");
	map(0x81, 0x81).portr("P2");
	map(0x90, 0x90).portr("SYSTEM");
	map(0x91, 0x91).portr("SERVICE");
	map(0xc0, 0xc0).w(FUNC(quizdna_state::gekiretu_rombank_w));
	map(0xd0, 0xd0).w(FUNC(quizdna_state::screen_ctrl_w));
	map(0xe0, 0xe1).rw("ymsnd", FUNC(ym2203_device::read), FUNC(ym2203_device::write));
	map(0xf0, 0xf0).rw("oki", FUNC(okim6295_device::read), FUNC(okim6295_device::write));
}


/****************************************************************************/

static INPUT_PORTS_START( quizdna )
	PORT_START("DSW2")
	PORT_DIPNAME( 0x0f, 0x0f, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 9C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 8C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 7C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 6C_1C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x09, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x0f, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x0e, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x0d, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x0b, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x0a, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "Unknown 2-6" )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_SERVICE( 0x80, IP_ACTIVE_LOW )

	PORT_START("DSW3")
	PORT_DIPNAME( 0x03, 0x02, "Timer" )
	PORT_DIPSETTING(    0x03, "Slow" )
	PORT_DIPSETTING(    0x02, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x01, "Fast" )
	PORT_DIPSETTING(    0x00, "Very Fast" )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x08, "1" )
	PORT_DIPSETTING(    0x04, "2" )
	PORT_DIPSETTING(    0x0c, "3" )
	PORT_DIPSETTING(    0x00, "4" )
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x30, "Every 500k" )
	PORT_DIPSETTING(    0x20, "Every 1000k" )
	PORT_DIPSETTING(    0x10, "Every 2000k" )
	PORT_DIPSETTING(    0x00, DEF_STR( None ) )
	PORT_DIPNAME( 0x40, 0x40, "Carat" )
	PORT_DIPSETTING(    0x40, "20" )
	PORT_DIPSETTING(    0x00, "0" )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Allow_Continue ) )
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Yes ) )

	PORT_START("P1")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON3 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON4 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("P2")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("SYSTEM")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START2 )

	PORT_START("SERVICE")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END

static INPUT_PORTS_START( gakupara )
	PORT_START("DSW2")
	PORT_DIPNAME( 0x0f, 0x0f, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x00, "10 Coins/1 Credit" )
	PORT_DIPSETTING(    0x01, DEF_STR( 9C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 8C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 7C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 6C_1C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x09, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x0f, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x0e, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x0d, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x0b, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x0a, DEF_STR( 1C_6C ) )
	PORT_DIPNAME( 0x10, 0x00, "Demo Music" )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0xc0, 0x80, "Timer" )
	PORT_DIPSETTING(    0xc0, "Slow" )
	PORT_DIPSETTING(    0x80, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x40, "Fast" )
	PORT_DIPSETTING(    0x00, "Very Fast" )

	PORT_START("DSW3")
	PORT_SERVICE( 0x01, IP_ACTIVE_LOW )
	PORT_DIPNAME( 0x02, 0x02, "Unknown 3-2" )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "Unknown 3-3" )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "Unknown 3-4" )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "Unknown 3-5" )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "Unknown 3-6" )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "Unknown 3-7" )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Unknown 3-8" )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("P1")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON3 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON4 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("P2")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("SYSTEM")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START2 )

	PORT_START("SERVICE")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END

static INPUT_PORTS_START( gekiretu )
	PORT_START("DSW2")
	PORT_DIPNAME( 0x0f, 0x0f, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 9C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 8C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 7C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 6C_1C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x09, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x0f, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x0e, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x0d, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x0b, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x0a, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "Unknown 2-7" )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_SERVICE( 0x80, IP_ACTIVE_LOW )

	PORT_START("DSW3")
	PORT_DIPNAME( 0x03, 0x03, "Timer" )
	PORT_DIPSETTING(    0x03, "Slow" )
	PORT_DIPSETTING(    0x02, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x01, "Fast" )
	PORT_DIPSETTING(    0x00, "Very Fast" )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x04, "1" )
	PORT_DIPSETTING(    0x08, "2" )
	PORT_DIPSETTING(    0x0c, "3" )
	PORT_DIPSETTING(    0x00, "4" )
	PORT_DIPNAME( 0x10, 0x10, "Unknown 3-5" )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "Unknown 3-6" )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "Unknown 3-7" )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Allow_Continue ) )
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Yes ) )

	PORT_START("P1")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON3 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON4 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("P2")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("SYSTEM")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START2 )

	PORT_START("SERVICE")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END

/****************************************************************************/

static const gfx_layout fglayout =
{
	16,8,     /* 16*8 characters */
	8192*2,   /* 16384 characters */
	1,        /* 1 bit per pixel */
	{0},
	{ STEP16(0,1) },
	{ STEP8(0,16) },
	16*8
};

static const gfx_layout bglayout =
{
	8,8,        /* 8*8 characters */
	32768+1024, /* 32768+1024 characters */
	4,          /* 4 bits per pixel */
	{0,1,2,3},
	{ STEP8(0,4) },
	{ STEP8(0,32) },
	8*8*4
};

static const gfx_layout objlayout =
{
	16,16,    /* 16*16 characters */
	8192+256, /* 8192+256 characters */
	4,        /* 4 bits per pixel */
	{0,1,2,3},
	{ STEP16(0,4) },
	{ STEP16(0,64) },
	16*16*4
};

static GFXDECODE_START( gfx_quizdna )
	GFXDECODE_ENTRY( "gfx1", 0x0000, fglayout,  0x7e0,  16 )
	GFXDECODE_ENTRY( "gfx2", 0x0000, bglayout,  0x000, 128 )
	GFXDECODE_ENTRY( "gfx3", 0x0000, objlayout, 0x600,  32 )
GFXDECODE_END

void quizdna_state::machine_start()
{
	membank("mainbank")->configure_entries(0, 64, memregion("maincpu")->base() + 0x10000, 0x4000);
}


MACHINE_CONFIG_START(quizdna_state::quizdna)

	/* basic machine hardware */
	MCFG_DEVICE_ADD("maincpu", Z80, MCLK/2) /* 8.000 MHz */
	MCFG_DEVICE_PROGRAM_MAP(quizdna_map)
	MCFG_DEVICE_IO_MAP(quizdna_io_map)
	MCFG_DEVICE_VBLANK_INT_DRIVER("screen", quizdna_state,  irq0_line_hold)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500) /* not accurate */)
	MCFG_SCREEN_SIZE(64*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(8*8, 56*8-1, 2*8, 30*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(quizdna_state, screen_update)
	MCFG_SCREEN_PALETTE("palette")

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_quizdna);
	MCFG_PALETTE_ADD("palette", 2048)


	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	ym2203_device &ymsnd(YM2203(config, "ymsnd", MCLK/4));
	ymsnd.port_a_read_callback().set_ioport("DSW3");
	ymsnd.port_b_read_callback().set_ioport("DSW2");
	ymsnd.add_route(0, "mono", 0.10);
	ymsnd.add_route(1, "mono", 0.10);
	ymsnd.add_route(2, "mono", 0.10);
	ymsnd.add_route(3, "mono", 0.40);

	MCFG_DEVICE_ADD("oki", OKIM6295, (MCLK/1024)*132, okim6295_device::PIN7_HIGH) // clock frequency & pin 7 not verified
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.30)
MACHINE_CONFIG_END

MACHINE_CONFIG_START(quizdna_state::gakupara)
	quizdna(config);

	/* basic machine hardware */

	MCFG_DEVICE_MODIFY("maincpu")
	MCFG_DEVICE_IO_MAP(gakupara_io_map)

MACHINE_CONFIG_END

MACHINE_CONFIG_START(quizdna_state::gekiretu)
	quizdna(config);

	/* basic machine hardware */

	MCFG_DEVICE_MODIFY("maincpu")
	MCFG_DEVICE_PROGRAM_MAP(gekiretu_map)
	MCFG_DEVICE_IO_MAP(gekiretu_io_map)

MACHINE_CONFIG_END


/****************************************************************************/

ROM_START( quizdna )
	ROM_REGION( 0xd0000, "maincpu", 0 ) /* CPU */
	ROM_LOAD( "quiz2-pr.28",  0x00000,  0x08000, CRC(a428ede4) SHA1(cdca3bd84b2ea421fb05502ea29e9eb605e574eb) )
	ROM_CONTINUE(             0x18000,  0x78000 ) /* banked */
	/* empty */

	ROM_REGION( 0x40000, "gfx1", 0 ) /* fg */
	ROM_LOAD( "quiz2.102",    0x00000,  0x20000, CRC(62402ac9) SHA1(bf52d22b119d54410dad4949b0687bb0edf3e143) )
	/* empty */

	ROM_REGION( 0x108000, "gfx2", 0 ) /* bg */
	ROM_LOAD( "quiz2-bg.100", 0x000000,  0x100000, CRC(f1d0cac2) SHA1(26d25c1157d1916dfe4496c6cf119c4a9339e31c) )
	/* empty */

	ROM_REGION( 0x108000, "gfx3", 0 ) /* obj */
	ROM_LOAD( "quiz2-ob.98",  0x000000,  0x100000, CRC(682f19a6) SHA1(6b8e6e583f423cf8ef9095f2c300201db7d7b8b3) )
	ROM_LOAD( "quiz2ob2.97",  0x100000,  0x008000, CRC(03736b1a) SHA1(bc42ac293260f58a8a138702d890f69aec99c05e) )

	ROM_REGION( 0x80000, "oki", 0 ) /* samples */
	ROM_LOAD( "quiz2-sn.32",  0x000000,  0x040000, CRC(1c044637) SHA1(dc749295e149f968495272f1a3ec27c6b719be8e) )

	ROM_REGION( 0x00020, "user1", 0 ) /* fg control */
	ROM_LOAD( "quiz2.148",    0x000000,  0x000020, CRC(91267e8a) SHA1(ae5bd8efea5322c4d9986d06680a781392f9a642) )
ROM_END

ROM_START( gakupara )
	ROM_REGION( 0xd0000, "maincpu", 0 ) /* CPU */
	ROM_LOAD( "u28.bin",  0x00000,  0x08000, CRC(72124bb8) SHA1(e734acff7e9d6b8c6a95c76860732320a2e3a828) )
	ROM_CONTINUE(         0x18000,  0x78000 )             /* banked */
	ROM_LOAD( "u29.bin",  0x90000,  0x40000, CRC(09f4948e) SHA1(21ccf5af6935cf40c0cf73fbee14bff3c4e1d23d) ) /* banked */

	ROM_REGION( 0x40000, "gfx1", 0 ) /* fg */
	ROM_LOAD( "u102.bin", 0x00000,  0x20000, CRC(62402ac9) SHA1(bf52d22b119d54410dad4949b0687bb0edf3e143) )
	ROM_LOAD( "u103.bin", 0x20000,  0x20000, CRC(38644251) SHA1(ebfdc43c38e1380709ed08575c346b2467ad1592) )

	ROM_REGION( 0x108000, "gfx2", 0 ) /* bg */
	ROM_LOAD( "u100.bin", 0x000000,  0x100000, CRC(f9d886ea) SHA1(d7763f54a165af720216b96e601a66fbc59e3568) )
	ROM_LOAD( "u99.bin",  0x100000,  0x008000, CRC(ac224d0a) SHA1(f187c3b74bc18606d0fe638f6a829f71c109998d) )

	ROM_REGION( 0x108000, "gfx3", 0 ) /* obj */
	ROM_LOAD( "u98.bin",  0x000000,  0x100000, CRC(a6e8cb56) SHA1(2fc85c1769513cc7aa5e23afaf0c99c38de9b855) )
	ROM_LOAD( "u97.bin",  0x100000,  0x008000, CRC(9dacd5c9) SHA1(e40211059e71408be3d67807463304f4d4ecae37) )

	ROM_REGION( 0x80000, "oki", 0 ) /* samples */
	ROM_LOAD( "u32.bin",  0x000000,  0x040000, CRC(eb03c535) SHA1(4d6c749ccab4681eee0a1fb243e9f3dbe61b9f94) )

	ROM_REGION( 0x00020, "user1", 0 ) /* fg control */
	ROM_LOAD( "u148.bin", 0x000000,  0x000020, CRC(971df9d2) SHA1(280f5b386922b9902ca9211c719642c2bd0ba899) )
ROM_END

ROM_START( gekiretu )
	ROM_REGION( 0xd0000, "maincpu", 0 ) /* CPU */
	ROM_LOAD( "quiz3-pr.28",  0x00000,  0x08000, CRC(a761e86f) SHA1(85331ef53598491e78c2d123b1ebd358aff46436) )
	ROM_CONTINUE(             0x18000,  0x78000 ) /* banked */
	/* empty */

	ROM_REGION( 0x40000, "gfx1", 0 ) /* fg */
	ROM_LOAD( "quiz3.102",    0x00000,  0x20000, CRC(62402ac9) SHA1(bf52d22b119d54410dad4949b0687bb0edf3e143) )
	/* empty */

	ROM_REGION( 0x108000, "gfx2", 0 ) /* bg */
	ROM_LOAD( "quiz3-bg.100", 0x000000,  0x100000, CRC(cb9272fd) SHA1(cfc1ff93d1fdc7d144e161a77e534cea75d7f181) )
	/* empty */

	ROM_REGION( 0x108000, "gfx3", 0 ) /* obj */
	ROM_LOAD( "quiz3-ob.98",  0x000000,  0x100000, CRC(01bed020) SHA1(5cc56c8823ee5e538371debe1cbeb57c4976677b) )
	/* empty */

	ROM_REGION( 0x80000, "oki", 0 ) /* samples */
	ROM_LOAD( "quiz3-sn.32",  0x000000,  0x040000, CRC(36dca582) SHA1(2607602e942244cfaae931da2ad36da9a8f855f7) )

	ROM_REGION( 0x00020, "user1", 0 ) /* fg control */
	ROM_LOAD( "quiz3.148",    0x000000,  0x000020, CRC(91267e8a) SHA1(ae5bd8efea5322c4d9986d06680a781392f9a642) )
ROM_END

GAME( 1991, gakupara, 0, gakupara, gakupara, quizdna_state, empty_init, ROT0, "NMK",  "Quiz Gakuen Paradise (Japan)",    MACHINE_SUPPORTS_SAVE )
GAME( 1992, quizdna,  0, quizdna,  quizdna,  quizdna_state, empty_init, ROT0, "Face", "Quiz DNA no Hanran (Japan)",      MACHINE_SUPPORTS_SAVE )
GAME( 1992, gekiretu, 0, gekiretu, gekiretu, quizdna_state, empty_init, ROT0, "Face", "Quiz Gekiretsu Scramble (Japan)", MACHINE_SUPPORTS_SAVE )
