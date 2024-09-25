// license:BSD-3-Clause
// copyright-holders:Pierpaolo Prazzoli
/******************************************************************************

    Dacholer    (c) 1983 Nichibutsu
    Kick Boy    (c) 1983 Nichibutsu

    Driver by Pierpaolo Prazzoli

    TODO:
      - is the background color pen correct for all games?

    Mods by Tomasz Slanina (2008.06.12):
      - fixed sound cpu interrupts (mode 2 (two vectors)+ nmi)
      - added sound and music.
      - ay/msm clocks are arbitrary
      - just a guess - upper nibble of byte from port 3 _probably_
        contains sound command (sound cpu writes it to port c)

    Itazura Tenshi (Japan Ver.)
    (c)1984 Nichibutsu / Alice



    --- Team Japump!!! ---
    Dumped by Chack'n
    Driver written by Hau

    based on driver from drivers/dacholer.c by Pierpaolo Prazzoli
    note:
    Sound test does not work.

******************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "machine/gen_latch.h"
#include "sound/msm5205.h"
#include "sound/ay8910.h"
#include "video/resnet.h"
#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"


namespace {

class dacholer_state : public driver_device
{
public:
	dacholer_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this,"maincpu")
		, m_audiocpu(*this,"audiocpu")
		, m_msm(*this, "msm")
		, m_gfxdecode(*this, "gfxdecode")
		, m_palette(*this, "palette")
		, m_soundlatch(*this, "soundlatch")
		, m_bgvideoram(*this, "bgvideoram")
		, m_fgvideoram(*this, "fgvideoram")
		, m_spriteram(*this, "spriteram")
		, m_leds(*this, "led%u", 0U)
	{ }

	void itaten(machine_config &config);
	void dacholer(machine_config &config);

	int snd_ack_r();

private:
	void bg_scroll_x_w(uint8_t data);
	void bg_scroll_y_w(uint8_t data);
	void background_w(offs_t offset, uint8_t data);
	void foreground_w(offs_t offset, uint8_t data);
	void bg_bank_w(uint8_t data);
	void coins_w(uint8_t data);
	void main_irq_ack_w(uint8_t data);
	void adpcm_w(uint8_t data);
	void snd_ack_w(uint8_t data);
	void snd_irq_w(uint8_t data);
	void music_irq_w(uint8_t data);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	TILE_GET_INFO_MEMBER(get_fg_tile_info);
	void dacholer_palette(palette_device &palette) const;
	uint32_t screen_update_dacholer(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(sound_irq);
	void draw_sprites( bitmap_ind16 &bitmap, const rectangle &cliprect );
	void adpcm_int(int state);
	void itaten_main_map(address_map &map) ATTR_COLD;
	void itaten_snd_io_map(address_map &map) ATTR_COLD;
	void itaten_snd_map(address_map &map) ATTR_COLD;
	void main_io_map(address_map &map) ATTR_COLD;
	void main_map(address_map &map) ATTR_COLD;
	void snd_io_map(address_map &map) ATTR_COLD;
	void snd_map(address_map &map) ATTR_COLD;

	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	optional_device<msm5205_device> m_msm;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_device<generic_latch_8_device> m_soundlatch;

	/* memory pointers */
	required_shared_ptr<uint8_t> m_bgvideoram;
	required_shared_ptr<uint8_t> m_fgvideoram;
	required_shared_ptr<uint8_t> m_spriteram;

	output_finder<2> m_leds;

	/* video-related */
	tilemap_t  *m_bg_tilemap = nullptr;
	tilemap_t  *m_fg_tilemap = nullptr;
	int      m_bg_bank = 0;
	uint8_t    m_scroll_x = 0;
	uint8_t    m_scroll_y = 0;

	/* sound-related */
	int m_msm_data = 0;
	int m_msm_toggle = 0;
	uint8_t m_snd_interrupt_enable = 0;
	uint8_t m_music_interrupt_enable = 0;
	uint8_t m_snd_ack = 0;

};

TILE_GET_INFO_MEMBER(dacholer_state::get_bg_tile_info)
{
	tileinfo.set(1, m_bgvideoram[tile_index] + m_bg_bank * 0x100, 0, 0);
}

TILE_GET_INFO_MEMBER(dacholer_state::get_fg_tile_info)
{
	tileinfo.set(0, m_fgvideoram[tile_index], 0, 0);
}

void dacholer_state::video_start()
{
	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(dacholer_state::get_bg_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);
	m_fg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(dacholer_state::get_fg_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);

	m_fg_tilemap->set_transparent_pen(0);
}

void dacholer_state::bg_scroll_x_w(uint8_t data)
{
	m_scroll_x = data;
}

void dacholer_state::bg_scroll_y_w(uint8_t data)
{
	m_scroll_y = data;
}

void dacholer_state::draw_sprites( bitmap_ind16 &bitmap, const rectangle &cliprect )
{
	int offs, code, attr, sx, sy, flipx, flipy;

	for (offs = 0; offs < m_spriteram.bytes(); offs += 4)
	{
		code = m_spriteram[offs + 1];
		attr = m_spriteram[offs + 2];

		flipx = attr & 0x10;
		flipy = attr & 0x20;

		sx = (m_spriteram[offs + 3] - 128) + 256 * (attr & 0x01);
		sy = 255 - m_spriteram[offs];

		if (flip_screen())
		{
			sx = 240 - sx;
			sy = 240 - sy;
			flipx = !flipx;
			flipy = !flipy;
		}

		m_gfxdecode->gfx(2)->transpen(bitmap,cliprect,
				code,
				0,
				flipx,flipy,
				sx,sy,0);
	}
}

uint32_t dacholer_state::screen_update_dacholer(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	if (flip_screen())
	{
		m_bg_tilemap->set_scrollx(0, 256 - m_scroll_x);
		m_bg_tilemap->set_scrolly(0, 256 - m_scroll_y);
	}
	else
	{
		m_bg_tilemap->set_scrollx(0, m_scroll_x);
		m_bg_tilemap->set_scrolly(0, m_scroll_y);
	}

	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	draw_sprites(bitmap, cliprect);
	m_fg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	return 0;
}

void dacholer_state::background_w(offs_t offset, uint8_t data)
{
	m_bgvideoram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);
}

void dacholer_state::foreground_w(offs_t offset, uint8_t data)
{
	m_fgvideoram[offset] = data;
	m_fg_tilemap->mark_tile_dirty(offset);
}

void dacholer_state::bg_bank_w(uint8_t data)
{
	if ((data & 3) != m_bg_bank)
	{
		m_bg_bank = data & 3;
		m_bg_tilemap->mark_all_dirty();
	}

	flip_screen_set(data & 0xc); // probably one bit for flipx and one for flipy

}

void dacholer_state::coins_w(uint8_t data)
{
	machine().bookkeeping().coin_counter_w(0, data & 1);
	machine().bookkeeping().coin_counter_w(1, data & 2);

	m_leds[0] = BIT(data, 2);
	m_leds[1] = BIT(data, 3);
}

void dacholer_state::main_irq_ack_w(uint8_t data)
{
	m_maincpu->set_input_line(0, CLEAR_LINE);
}


void dacholer_state::main_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0x8800, 0x97ff).ram();
	map(0xc000, 0xc3ff).mirror(0x400).ram().w(FUNC(dacholer_state::background_w)).share("bgvideoram");
	map(0xd000, 0xd3ff).ram().w(FUNC(dacholer_state::foreground_w)).share("fgvideoram");
	map(0xe000, 0xe0ff).ram().share("spriteram");
}

void dacholer_state::itaten_main_map(address_map &map)
{
	main_map(map);
	map(0x0000, 0x9fff).rom();
	map(0xa000, 0xb7ff).ram();
}

void dacholer_state::main_io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x00).portr("P1");
	map(0x01, 0x01).portr("P2");
	map(0x02, 0x02).portr("SYSTEM");
	map(0x03, 0x03).portr("DSWA");
	map(0x04, 0x04).portr("DSWB");
	map(0x05, 0x05).nopr(); // watchdog in itaten
	map(0x20, 0x20).w(FUNC(dacholer_state::coins_w));
	map(0x21, 0x21).w(FUNC(dacholer_state::bg_bank_w));
	map(0x22, 0x22).w(FUNC(dacholer_state::bg_scroll_x_w));
	map(0x23, 0x23).w(FUNC(dacholer_state::bg_scroll_y_w));
	map(0x24, 0x24).w(FUNC(dacholer_state::main_irq_ack_w));
	map(0x27, 0x27).w(m_soundlatch, FUNC(generic_latch_8_device::write));
}


void dacholer_state::snd_map(address_map &map)
{
	map(0x0000, 0x5fff).rom();
	map(0xd000, 0xe7ff).ram();
}


void dacholer_state::itaten_snd_map(address_map &map)
{
	map(0x0000, 0x2fff).rom();
	map(0xe000, 0xe7ff).ram();
}


void dacholer_state::adpcm_w(uint8_t data)
{
	m_msm_data = data;
	m_msm_toggle = 0;
}

void dacholer_state::snd_ack_w(uint8_t data)
{
	m_snd_ack = data;
}

int dacholer_state::snd_ack_r()
{
	return m_snd_ack;       //guess ...
}

void dacholer_state::snd_irq_w(uint8_t data)
{
	m_snd_interrupt_enable = data;
}

void dacholer_state::music_irq_w(uint8_t data)
{
	m_music_interrupt_enable = data;
}

void dacholer_state::snd_io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x00).rw(m_soundlatch, FUNC(generic_latch_8_device::read), FUNC(generic_latch_8_device::acknowledge_w));
	map(0x04, 0x04).w(FUNC(dacholer_state::music_irq_w));
	map(0x08, 0x08).w(FUNC(dacholer_state::snd_irq_w));
	map(0x0c, 0x0c).w(FUNC(dacholer_state::snd_ack_w));
	map(0x80, 0x80).w(FUNC(dacholer_state::adpcm_w));
	map(0x86, 0x87).w("ay1", FUNC(ay8910_device::data_address_w));
	map(0x8a, 0x8b).w("ay2", FUNC(ay8910_device::data_address_w));
	map(0x8e, 0x8f).w("ay3", FUNC(ay8910_device::data_address_w));
}

void dacholer_state::itaten_snd_io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x00).rw(m_soundlatch, FUNC(generic_latch_8_device::read), FUNC(generic_latch_8_device::acknowledge_w));
	map(0x86, 0x87).w("ay1", FUNC(ay8910_device::data_address_w));
	map(0x8a, 0x8b).w("ay2", FUNC(ay8910_device::data_address_w));
	map(0x8e, 0x8f).w("ay3", FUNC(ay8910_device::data_address_w));
}


static INPUT_PORTS_START( dacholer )
	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_SERVICE( 0x08, IP_ACTIVE_LOW )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSWA")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Coin_A ) )           /* table at 0x0a8c */
	PORT_DIPSETTING(    0x01, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Coin_B ) )           /* table at 0x0a94 */
	PORT_DIPSETTING(    0x00, DEF_STR( 6C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_1C ) )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(dacholer_state, snd_ack_r)

	PORT_START("DSWB")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Lives ) )            /* table at 0x0a9c */
	PORT_DIPSETTING(    0x03, "1" )
	PORT_DIPSETTING(    0x02, "2" )
	PORT_DIPSETTING(    0x01, "3" )
	PORT_DIPSETTING(    0x00, "4" )
//  PORT_DIPNAME( 0x0c, 0x0c, "1st Bonus Life" )            /* table at 0x0aa0 */
//  PORT_DIPSETTING(    0x0c, "20k" )
//  PORT_DIPSETTING(    0x08, "30k" )
//  PORT_DIPSETTING(    0x04, "40k" )
//  PORT_DIPSETTING(    0x00, DEF_STR( None ) )
//  PORT_DIPNAME( 0x30, 0x30, "Next Bonus Lifes" )          /* table at 0x0aa8 */
//  PORT_DIPSETTING(    0x30, "+50k" )
//  PORT_DIPSETTING(    0x20, "+70k" )
//  PORT_DIPSETTING(    0x10, "+100k" )
//  PORT_DIPSETTING(    0x00, DEF_STR( None ) )
	PORT_DIPNAME( 0x3c, 0x3c, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x3c, "20k 70k then every 50k" )
	PORT_DIPSETTING(    0x38, "30k 80k then every 50k" )
	PORT_DIPSETTING(    0x34, "40k 90k then every 50k" )
//  PORT_DIPSETTING(    0x30, DEF_STR( None ) )
	PORT_DIPSETTING(    0x2c, "20k 90k then every 70k" )
	PORT_DIPSETTING(    0x28, "30k 100k then every 70k" )
	PORT_DIPSETTING(    0x24, "40k 110k then every 70k" )
//  PORT_DIPSETTING(    0x20, DEF_STR( None ) )
	PORT_DIPSETTING(    0x1c, "20k 120k then every 100k" )
	PORT_DIPSETTING(    0x18, "30k 130k then every 100k" )
	PORT_DIPSETTING(    0x14, "40k 140k then every 100k" )
//  PORT_DIPSETTING(    0x10, DEF_STR( None ) )
	PORT_DIPSETTING(    0x0c, "20k only" )
	PORT_DIPSETTING(    0x08, "30k only" )
	PORT_DIPSETTING(    0x04, "40k only" )
	PORT_DIPSETTING(    0x00, DEF_STR( None ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Cocktail ) )
INPUT_PORTS_END

static INPUT_PORTS_START( kickboy )
	PORT_INCLUDE(dacholer)

	PORT_MODIFY("DSWA")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Coin_A ) )           /* table at 0x0f71 - same as in 'dacholer' */
	PORT_DIPSETTING(    0x01, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Coin_B ) )           /* table at 0x0f79 */
	PORT_DIPSETTING(    0x04, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Free_Play ) )

	PORT_MODIFY("DSWB")
	PORT_DIPNAME( 0x03, 0x01, DEF_STR( Lives ) )            /* table at 0x0f81 */
	PORT_DIPSETTING(    0x03, "3" )
	PORT_DIPSETTING(    0x02, "4" )
	PORT_DIPSETTING(    0x01, "5" )
	PORT_DIPSETTING(    0x00, "99 (Cheat)" )
//  PORT_DIPNAME( 0x0c, 0x0c, "1st Bonus Life" )            /* table at 0x0f85 - same as in 'dacholer' */
//  PORT_DIPSETTING(    0x0c, "20k" )
//  PORT_DIPSETTING(    0x08, "30k" )
//  PORT_DIPSETTING(    0x04, "40k" )
//  PORT_DIPSETTING(    0x00, DEF_STR( None ) )
//  PORT_DIPNAME( 0x30, 0x30, "Next Bonus Lifes" )          /* table at 0x0f8d */
//  PORT_DIPSETTING(    0x30, "+30k" )
//  PORT_DIPSETTING(    0x20, "+50k" )
//  PORT_DIPSETTING(    0x10, "+70k" )
//  PORT_DIPSETTING(    0x00, DEF_STR( None ) )
	PORT_DIPNAME( 0x3c, 0x3c, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x3c, "20k 50k then every 30k" )
	PORT_DIPSETTING(    0x38, "30k 60k then every 30k" )
	PORT_DIPSETTING(    0x34, "40k 70k then every 30k" )
//  PORT_DIPSETTING(    0x30, DEF_STR( None ) )
	PORT_DIPSETTING(    0x2c, "20k 70k then every 50k" )
	PORT_DIPSETTING(    0x28, "30k 80k then every 50k" )
	PORT_DIPSETTING(    0x24, "40k 90k then every 50k" )
//  PORT_DIPSETTING(    0x20, DEF_STR( None ) )
	PORT_DIPSETTING(    0x1c, "20k 90k then every 70k" )
	PORT_DIPSETTING(    0x18, "30k 100k then every 70k" )
	PORT_DIPSETTING(    0x14, "40k 110k then every 70k" )
//  PORT_DIPSETTING(    0x10, DEF_STR( None ) )
	PORT_DIPSETTING(    0x0c, "20k only" )
	PORT_DIPSETTING(    0x08, "30k only" )
	PORT_DIPSETTING(    0x04, "40k only" )
	PORT_DIPSETTING(    0x00, DEF_STR( None ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Difficulty ) )       /* stored at 0x920f */
	PORT_DIPSETTING(    0x40, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hard ) )
INPUT_PORTS_END

static INPUT_PORTS_START( itaten )
	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_SERVICE( 0x08, IP_ACTIVE_LOW )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSWA")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Coin_A ) ) PORT_DIPLOCATION("SW1:1,2")
	PORT_DIPSETTING(    0x01, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Coin_B ) ) PORT_DIPLOCATION("SW1:3,4")
	PORT_DIPSETTING(    0x00, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_6C ) )
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSWB")
	PORT_DIPNAME( 0x03, 0x02, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(    0x03, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Medium ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW2:3,4")
	PORT_DIPSETTING(    0x0c, "3" )
	PORT_DIPSETTING(    0x08, "4" )
	PORT_DIPSETTING(    0x04, "5" )
	PORT_DIPSETTING(    0x00, "6" )
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Bonus_Life ) ) PORT_DIPLOCATION("SW2:5,6")
	PORT_DIPSETTING(    0x30, "30k then every 50k" )
	PORT_DIPSETTING(    0x20, "60k then every 50k" )
	PORT_DIPSETTING(    0x10, "30k then every 90k" )
	PORT_DIPSETTING(    0x00, "60k then every 90k" )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Cabinet ) ) PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Cocktail ) )
INPUT_PORTS_END

static GFXDECODE_START( gfx_dacholer )
	GFXDECODE_ENTRY( "gfx1", 0, gfx_8x8x4_packed_lsb,   0x00, 1 )
	GFXDECODE_ENTRY( "gfx2", 0, gfx_8x8x4_packed_lsb,   0x10, 1 )
	GFXDECODE_ENTRY( "gfx3", 0, gfx_16x16x4_packed_lsb, 0x10, 1 )
GFXDECODE_END

static GFXDECODE_START( gfx_itaten )
	GFXDECODE_ENTRY( "gfx1", 0, gfx_8x8x4_packed_lsb,   0x00, 1 )
	GFXDECODE_ENTRY( "gfx2", 0, gfx_8x8x4_packed_lsb,   0x00, 1 )
	GFXDECODE_ENTRY( "gfx3", 0, gfx_16x16x4_packed_lsb, 0x10, 1 )
GFXDECODE_END


INTERRUPT_GEN_MEMBER(dacholer_state::sound_irq)
{
	if (m_music_interrupt_enable == 1)
	{
		device.execute().set_input_line_and_vector(0, HOLD_LINE, 0x30); // Z80
	}
}

void dacholer_state::adpcm_int(int state)
{
	if (m_snd_interrupt_enable == 1 || (m_snd_interrupt_enable == 0 && m_msm_toggle == 1))
	{
		m_msm->data_w(m_msm_data >> 4);
		m_msm_data <<= 4;
		m_msm_toggle ^= 1;
		if (m_msm_toggle == 0)
		{
			m_audiocpu->set_input_line_and_vector(0, HOLD_LINE, 0x38); // Z80
		}
	}
}

void dacholer_state::machine_start()
{
	m_leds.resolve();

	save_item(NAME(m_bg_bank));
	save_item(NAME(m_msm_data));
	save_item(NAME(m_msm_toggle));
	save_item(NAME(m_snd_interrupt_enable));
	save_item(NAME(m_music_interrupt_enable));
	save_item(NAME(m_snd_ack));
}

void dacholer_state::machine_reset()
{
	m_msm_data = 0;
	m_msm_toggle = 0;

	m_bg_bank = 0;
	m_snd_interrupt_enable = 0;
	m_music_interrupt_enable = 0;
	m_snd_ack = 0;
}

// guess: use the same resistor values as Crazy Climber (needs checking on the real hardware)
void dacholer_state::dacholer_palette(palette_device &palette) const
{
	const uint8_t *color_prom = memregion("proms")->base();
	static constexpr int resistances_rg[3] = { 1000, 470, 220 };
	static constexpr int resistances_b [2] = { 470, 220 };

	// compute the color output resistor weights
	double weights_rg[3], weights_b[2];
	compute_resistor_weights(0, 255, -1.0,
			3, resistances_rg, weights_rg, 0, 0,
			2, resistances_b,  weights_b,  0, 0,
			0, nullptr, nullptr, 0, 0);

	for (int i = 0; i < palette.entries(); i++)
	{
		int bit0, bit1, bit2;

		// red component
		bit0 = BIT(color_prom[i], 0);
		bit1 = BIT(color_prom[i], 1);
		bit2 = BIT(color_prom[i], 2);
		int const r = combine_weights(weights_rg, bit0, bit1, bit2);

		// green component
		bit0 = BIT(color_prom[i], 3);
		bit1 = BIT(color_prom[i], 4);
		bit2 = BIT(color_prom[i], 5);
		int const g = combine_weights(weights_rg, bit0, bit1, bit2);

		// blue component
		bit0 = BIT(color_prom[i], 6);
		bit1 = BIT(color_prom[i], 7);
		int const b = combine_weights(weights_b, bit0, bit1);

		palette.set_pen_color(i, rgb_t(r, g, b));
	}
}

/* note: clocks are taken from itaten sound reference recording */
void dacholer_state::dacholer(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, XTAL(16'000'000)/4);  /* Dacholer PCB has a 15.46848 MHz OSC here */
	m_maincpu->set_addrmap(AS_PROGRAM, &dacholer_state::main_map);
	m_maincpu->set_addrmap(AS_IO, &dacholer_state::main_io_map);
	m_maincpu->set_vblank_int("screen", FUNC(dacholer_state::irq0_line_assert));

	Z80(config, m_audiocpu, XTAL(19'968'000)/8); /* ? */
	m_audiocpu->set_addrmap(AS_PROGRAM, &dacholer_state::snd_map);
	m_audiocpu->set_addrmap(AS_IO, &dacholer_state::snd_io_map);
	m_audiocpu->set_vblank_int("screen", FUNC(dacholer_state::sound_irq));

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(256, 256);
	screen.set_visarea(0, 256-1, 16, 256-1-16);
	screen.set_screen_update(FUNC(dacholer_state::screen_update_dacholer));
	screen.set_palette(m_palette);

	PALETTE(config, m_palette, FUNC(dacholer_state::dacholer_palette), 32);
	GFXDECODE(config, m_gfxdecode, m_palette, gfx_dacholer);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	GENERIC_LATCH_8(config, "soundlatch").data_pending_callback().set_inputline(m_audiocpu, INPUT_LINE_NMI);

	AY8910(config, "ay1", XTAL(19'968'000)/16).add_route(ALL_OUTPUTS, "mono", 0.15);
	AY8910(config, "ay2", XTAL(19'968'000)/16).add_route(ALL_OUTPUTS, "mono", 0.15);
	AY8910(config, "ay3", XTAL(19'968'000)/16).add_route(ALL_OUTPUTS, "mono", 0.15);

	MSM5205(config, m_msm, XTAL(384'000));
	m_msm->vck_legacy_callback().set(FUNC(dacholer_state::adpcm_int));  /* interrupt function */
	m_msm->set_prescaler_selector(msm5205_device::S96_4B);  /* 1 / 96 = 3906.25Hz playback  - guess */
	m_msm->add_route(ALL_OUTPUTS, "mono", 0.30);
}

void dacholer_state::itaten(machine_config &config)
{
	dacholer(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &dacholer_state::itaten_main_map);

	m_audiocpu->set_addrmap(AS_PROGRAM, &dacholer_state::itaten_snd_map);
	m_audiocpu->set_addrmap(AS_IO, &dacholer_state::itaten_snd_io_map);
	m_audiocpu->remove_vblank_int();

	m_gfxdecode->set_info(gfx_itaten);

	config.device_remove("msm");
}

ROM_START( dacholer ) /* AM-1-A & AM-1-B two board stack */
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "dp_1.5k", 0x0000, 0x2000, CRC(8b73a441) SHA1(6de9e4845b9063af8df42aa82ad536737190582c) ) /* these 4 ROMs located on the AM-1-A top board */
	ROM_LOAD( "dp_2.5l", 0x2000, 0x2000, CRC(9499289f) SHA1(bcfe554eb1f8e686d193050c18278b6bf93f179f) )
	ROM_LOAD( "dp_3.5m", 0x4000, 0x2000, CRC(39d37281) SHA1(daaf84079dd18dd854946e066e2dcde994bcbba4) )
	ROM_LOAD( "dp_4.5n", 0x6000, 0x2000, CRC(bb781ea4) SHA1(170966c4bcd0246968850d908a69f81ea1e136d5) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "ds_1.6g", 0x0000, 0x2000, CRC(cc3a4b68) SHA1(29344dc10c5d236f9a452196b3809565b4101327) ) /* these 4 ROMs located on the AM-1-A top board */
	ROM_LOAD( "ds_2.6h", 0x2000, 0x2000, CRC(aa18e126) SHA1(e6af334188d0edbc37a7fb4a00a325b2039172b7) )
	ROM_LOAD( "ds_3.6j", 0x4000, 0x2000, CRC(3b0131c7) SHA1(338ca2c2c7480e1cd0bb15ee6b90d683ce06f0fd) )
	/* 6K not populated */

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "dc_7.12j", 0x0000, 0x2000, CRC(fd649d36) SHA1(77d78eef44f348b635dbc0711e662a5236c00d51) ) /* this ROM located on the AM-1-B bottom board */

	ROM_REGION( 0x6000, "gfx2", 0 )
	ROM_LOAD( "dc_3.13a", 0x0000, 0x2000, CRC(9cca0fd2) SHA1(3ca1b4cca9611232df1195ae6ac172a79c8368c3) ) /* these 3 ROMs located on the AM-1-B bottom board */
	ROM_LOAD( "dc_2.12a", 0x2000, 0x2000, CRC(c1322b27) SHA1(8022f59b8ae10a7a911563b01bffc2d5646108a5) )
	ROM_LOAD( "dc_1.11a", 0x4000, 0x2000, CRC(9e1e7198) SHA1(7a75da1ae09f6cf095976b48f462ede42625b244) )
	/* 10A not populated */

	ROM_REGION( 0x6000, "gfx3", 0 )
	ROM_LOAD( "dc_5.2d", 0x0000, 0x2000, CRC(dd4818f0) SHA1(718236932248512f8779f640e0367b5d92e6497e) ) /* these 3 ROMs located on the AM-1-B bottom board */
	ROM_LOAD( "dc_4.1d", 0x2000, 0x2000, CRC(7f338ae0) SHA1(9206ed044feb44c55990803cdf608dd899e976ff) )
	ROM_LOAD( "dc_6.3d", 0x4000, 0x2000, CRC(0a6d4ec4) SHA1(419ea1f6ead3afb2de98432d9f8ead7447842a1e) )
	/* 4D not populated */

	ROM_REGION( 0x0060, "proms", 0 )
	ROM_LOAD( "dc.13d",  0x0000, 0x0020, CRC(d273abe5) SHA1(219bcba7f3e961f6b2cfbf48ac6ae6b6d80b974c) ) /* this MB7051 (or compatible 82S123) BPROM located on bottom board */
	ROM_LOAD( "af-2.1h", 0x0020, 0x0020, CRC(e1cac297) SHA1(f15326d04d006d9d029a6565aebf9daf3657bc2a) ) /* this PBROM located on the AM-1-B bottom board */
	ROM_LOAD( "af-1.3n", 0x0040, 0x0020, CRC(5638e485) SHA1(5d892111936a8eb7646c03a17300069be9a2b442) ) /* this PBROM located on the AM-1-A top board */
ROM_END

ROM_START( kickboy )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "k_1.5k", 0x0000, 0x2000, CRC(525746f1) SHA1(4044f880f271f77b56b2d8964ab97d34fb507c7a) )
	ROM_LOAD( "k_2.5l", 0x2000, 0x2000, CRC(9d091725) SHA1(827cea1c371094720b47fda271945cee20c9d956) )
	ROM_LOAD( "k_3.5m", 0x4000, 0x2000, CRC(d61b6ff6) SHA1(071ab4c05ed54526144f2ba751c111e8c4bdc61a) )
	ROM_LOAD( "k_4.5n", 0x6000, 0x2000, CRC(a8985bfe) SHA1(a8e466a7df381dfc8dd2e3483eba0215bfec7551) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "k_1.6g", 0x0000, 0x2000, CRC(cc3a4b68) SHA1(29344dc10c5d236f9a452196b3809565b4101327) )
	ROM_LOAD( "k_2.6h", 0x2000, 0x2000, CRC(aa18e126) SHA1(e6af334188d0edbc37a7fb4a00a325b2039172b7) )
	ROM_LOAD( "k_3.6j", 0x4000, 0x2000, CRC(3b0131c7) SHA1(338ca2c2c7480e1cd0bb15ee6b90d683ce06f0fd) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "k_7.12j", 0x0000, 0x2000, CRC(22be46e8) SHA1(d92b3913d8eba881c69acd1d85ca73ee58489fae) )

	ROM_REGION( 0x4000, "gfx2", 0 )
	ROM_LOAD( "k_3.13a", 0x0000, 0x2000, CRC(7eac2a64) SHA1(b4a44770bbded59cd572ac5d0ae178affc8cdab8) )
	ROM_LOAD( "k_2.12a", 0x2000, 0x2000, CRC(b8829572) SHA1(01009ec63449c809608923fd9dcecd82b29c5d6d) )

	ROM_REGION( 0x6000, "gfx3", 0 )
	ROM_LOAD( "k_5.2d", 0x0000, 0x2000, CRC(4b769a1c) SHA1(fde17dcd4b7cda9cc54572e81bc2f0e48c19277d) )
	ROM_LOAD( "k_4.1d", 0x2000, 0x2000, CRC(45199750) SHA1(a04b4d6d0defa613d269625b089d28dc68d5b73a) )
	ROM_LOAD( "k_6.3d", 0x4000, 0x2000, CRC(d1795506) SHA1(e0f7a64e301cf43c4739031461dba16aa44100a1) )

	ROM_REGION( 0x0060, "proms", 0 )
	ROM_LOAD( "k.13d",   0x0000, 0x0020, CRC(82f87a36) SHA1(5dc2059eb5b6cd541b014347c36198b8838d98fa) )
	ROM_LOAD( "af-2.1h", 0x0020, 0x0020, CRC(e1cac297) SHA1(f15326d04d006d9d029a6565aebf9daf3657bc2a) )
	ROM_LOAD( "af-1.3n", 0x0040, 0x0020, CRC(5638e485) SHA1(5d892111936a8eb7646c03a17300069be9a2b442) )
ROM_END

/*
--------------------------------
IT A-1
CPU  :LH0080 Z80,LH0080A Z80A
Sound:AY-3-8910 x3
OSC  :16.000MHz
--------------------------------
1.5K         [84c8a010] 2764
2.5L         [19946038]  |
3.5M         [4f9e26fd] /

6.6G         [dfcb1a3e] 2764
7.6H         [844e78d6] 2732

AF-1.3N      [5638e485] 82S123


--------------------------------
ITA-EXP
--------------------------------
4.1F         [35f85aeb] 2764
5.1E         [6cf30924] /


--------------------------------
IT A-2
OSC  :19.968MHz
--------------------------------
8.10A        [c32b0859] 2764
9.11A        [919cac5e]  |
10.12A       [d2b60e5d]  |
11.13A       [ed3279d5] /

12.1D        [f0f64636] 2764
13.2D        [d32559f5]  |
14.3D        [8c532c74]  |
15.4D        [d119b483] /

16.12J       [8af2bfb8] 2764

AF-2.1H      [e1cac297] 82S123
AF-3.13D     [875429ba] /
--------------------------------
*/


ROM_START( itaten )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "1.5k",  0x0000, 0x2000, CRC(84c8a010) SHA1(52d78ac70b3d5e905a11efd76acd99810c56e467) )
	ROM_LOAD( "2.5l",  0x2000, 0x2000, CRC(19946038) SHA1(74f76096e676535ead4386755fce853caac7673b) )
	ROM_LOAD( "3.5m",  0x4000, 0x2000, CRC(4f9e26fd) SHA1(33062724c46108611c9db16fdbf2cb9feed7e213) )
	ROM_LOAD( "4.1f",  0x6000, 0x2000, CRC(35f85aeb) SHA1(ecd8f62e304d1277332a5a2b7ec6aace9f77d8ad) )
	ROM_LOAD( "5.1e",  0x8000, 0x2000, CRC(6cf30924) SHA1(5e82e9aa0811ec1b853d300368c5ceec44938363) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "6.6g",  0x0000, 0x2000, CRC(dfcb1a3e) SHA1(cee0906cfbddd0254a947737da1cfbe47c445c32) )
	ROM_LOAD( "7.6h",  0x2000, 0x1000, CRC(844e78d6) SHA1(11b48af650809f8504b56e9a8e53c9f043782c5f) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "16.12j", 0x0000, 0x2000, CRC(8af2bfb8) SHA1(6744db0deb4fda7920fcfddf7f9c1ed6681d3622) )

	ROM_REGION( 0x8000, "gfx2", 0 )
	ROM_LOAD( "11.13a", 0x0000, 0x2000, CRC(ed3279d5) SHA1(e4bcae8038739c588f896ff35fa95288979fa683) )
	ROM_LOAD( "10.12a", 0x2000, 0x2000, CRC(d2b60e5d) SHA1(c833ac6e5d4d0a244ace600f5f02c6f43d3bd34b) )
	ROM_LOAD( "9.11a",  0x4000, 0x2000, CRC(919cac5e) SHA1(9602ad7618b5e4d93fa22676aa855256d4690dd1) )
	ROM_LOAD( "8.10a",  0x6000, 0x2000, CRC(c32b0859) SHA1(1bb00de55742a1f2cbbf3970b43b122114b0911b) )

	ROM_REGION( 0x8000, "gfx3", 0 )
	ROM_LOAD( "13.2d",  0x0000, 0x2000, CRC(d32559f5) SHA1(a4f05b1c8c48aad367ff675c29a9a1828c71b693) )
	ROM_LOAD( "12.1d",  0x2000, 0x2000, CRC(f0f64636) SHA1(a3354be74460e45453fea62c8dd910f98b5d2fb5) )
	ROM_LOAD( "14.3d",  0x4000, 0x2000, CRC(8c532c74) SHA1(c95786c81f82f7211f4411a9f39fd4ba4def9073) )
	ROM_LOAD( "15.4d",  0x6000, 0x2000, CRC(d119b483) SHA1(c1e403369bfbda0233ec5764fc703522e9f312a7) )

	ROM_REGION( 0x0060, "proms", 0 )
	ROM_LOAD( "af-3.13d", 0x0000, 0x0020, CRC(875429ba) SHA1(7186e1fb15806d60fd3e704be8db94c7b6c8c058) )
	ROM_LOAD( "af-2.1h",  0x0020, 0x0020, CRC(e1cac297) SHA1(f15326d04d006d9d029a6565aebf9daf3657bc2a) )
	ROM_LOAD( "af-1.3n",  0x0040, 0x0020, CRC(5638e485) SHA1(5d892111936a8eb7646c03a17300069be9a2b442) )
ROM_END

} // anonymous namespace


GAME( 1983, dacholer, 0, dacholer, dacholer, dacholer_state, empty_init, ROT0, "Nichibutsu",         "Dacholer",               MACHINE_SUPPORTS_SAVE )
GAME( 1983, kickboy,  0, dacholer, kickboy,  dacholer_state, empty_init, ROT0, "Nichibutsu",         "Kick Boy",               MACHINE_SUPPORTS_SAVE )
GAME( 1984, itaten,   0, itaten,   itaten,   dacholer_state, empty_init, ROT0, "Nichibutsu / Alice", "Itazura Tenshi (Japan)", MACHINE_SUPPORTS_SAVE )
