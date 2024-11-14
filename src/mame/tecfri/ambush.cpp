// license: BSD-3-Clause
// copyright-holders: Zsolt Vasvari, Dirk Best
/***************************************************************************

    Ambush

    © 1983 Tecfri

    PCB connector pinout

                  +5V   1  A  GND
                  +5V   2  B  GND
    +12V Coin Counter   3  C  +12V Coin Counter
          1P Button 1   4  D  1P Up
          1P Button 2   5  E  1P Down
          2P Button 1   6  F  1P Left
          2P Button 2   7  G  1P Right
             2P Start   8  H  2P Up
             1P Start   9  I  2P Down
               Coin 2  10  J  2P Left
               Coin 1  11  K  2P Right
                 Blue  12  L  Counter
                  Red  13  M  Counter
                Green  14  N  Counter
                 Sync  15  O  Speaker Right
                 +12V  16  P  Speaker Left
             Speaker-  17  Q  +5V
            Video GND  18  R  +5V

    The bootlegs are running on a kind of extended hardware. It has
    double the amount of work RAM, an updated graphics system to
    accommodate the bootlegged games and the AY8912s were changed to
    AY8910s.

    TODO:
    - Verify actual Z80 and AY891x clock speeds from PCB (XTAL confirmed)
    - VBlank IRQ ACK
    - Verify screen raw parameters
    - Identify the changes in the 'hacked' set and why they were made
    - Is 'b' really a second color PROM?
    - Figure out what the additional PROMs do (and dump the missing ones)
    - Unknown dkong3abl dip switch: Coinage related?

***************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "machine/74259.h"
#include "machine/watchdog.h"
#include "sound/ay8910.h"
#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class ambush_state : public driver_device
{
public:
	ambush_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_gfxdecode(*this, "gfxdecode"),
		m_outlatch(*this, "outlatch%u", 1),
		m_video_ram(*this, "video_ram"),
		m_sprite_ram(*this, "sprite_ram"),
		m_attribute_ram(*this, "attribute_ram"),
		m_scroll_ram(*this, "scroll_ram"),
		m_char_tilemap(nullptr),
		m_color_bank(0)
	{ }

	void ambush_base(machine_config &config);
	void ambush(machine_config &config);
	void mariobl(machine_config &config);
	void mariobla(machine_config &config);
	void dkong3abl(machine_config &config);

private:
	void ambush_palette(palette_device &palette) const;
	void mario_palette(palette_device &palette) const;
	void mariobla_palette(palette_device &palette) const;
	void dkong3_palette(palette_device &palette) const;

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_bootleg(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	TILE_GET_INFO_MEMBER(ambush_char_tile_info);
	TILE_GET_INFO_MEMBER(mariobl_char_tile_info);
	TILE_GET_INFO_MEMBER(dkong3abl_char_tile_info);

	void scroll_ram_w(offs_t offset, uint8_t data);
	void color_bank_1_w(int state);
	void color_bank_2_w(int state);

	DECLARE_MACHINE_START(ambush);
	DECLARE_MACHINE_START(mariobl);
	DECLARE_MACHINE_START(dkong3abl);

	void coin_counter_1_w(int state);
	void coin_counter_2_w(int state);
	void output_latches_w(offs_t offset, uint8_t data);

	void bootleg_map(address_map &map) ATTR_COLD;
	void main_map(address_map &map) ATTR_COLD;
	void main_portmap(address_map &map) ATTR_COLD;

	void register_save_states();

	required_device<gfxdecode_device> m_gfxdecode;
	optional_device_array<ls259_device, 2> m_outlatch;
	required_shared_ptr<uint8_t> m_video_ram;
	required_shared_ptr<uint8_t> m_sprite_ram;
	required_shared_ptr<uint8_t> m_attribute_ram;
	required_shared_ptr<uint8_t> m_scroll_ram;

	tilemap_t *m_char_tilemap;
	uint8_t m_color_bank;
};


//**************************************************************************
//  ADDRESS MAPS
//**************************************************************************

void ambush_state::main_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0x87ff).ram();
	map(0xa000, 0xa000).r("watchdog", FUNC(watchdog_timer_device::reset_r));
	map(0xc000, 0xc07f).ram();
	map(0xc080, 0xc09f).ram().w(FUNC(ambush_state::scroll_ram_w)).share("scroll_ram");  // 1 byte for each column
	map(0xc0a0, 0xc0ff).ram();
	map(0xc100, 0xc1ff).ram().share("attribute_ram");  // 1 line corresponds to 4 in the video ram
	map(0xc200, 0xc3ff).ram().share("sprite_ram");
	map(0xc400, 0xc7ff).ram().share("video_ram");
	map(0xc800, 0xc800).portr("sw1");
	map(0xcc00, 0xcc07).w(FUNC(ambush_state::output_latches_w));
}

void ambush_state::main_portmap(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x00).rw("ay1", FUNC(ay8910_device::data_r), FUNC(ay8910_device::address_w));
	map(0x01, 0x01).w("ay1", FUNC(ay8910_device::data_w));
	map(0x80, 0x80).rw("ay2", FUNC(ay8910_device::data_r), FUNC(ay8910_device::address_w));
	map(0x81, 0x81).w("ay2", FUNC(ay8910_device::data_w));
}

void ambush_state::bootleg_map(address_map &map)
{
	map(0x0000, 0x5fff).rom();
	map(0x6000, 0x6fff).ram();
	map(0x7000, 0x71ff).ram().share("sprite_ram");
	map(0x7200, 0x72ff).ram().share("attribute_ram");
	map(0x7300, 0x737f).ram();
	map(0x7380, 0x739f).ram().share("scroll_ram");  // not used on bootlegs?
	map(0x73a0, 0x73ff).ram();
	map(0x7400, 0x77ff).ram().share("video_ram");
	map(0x8000, 0x9fff).rom();
	map(0xa000, 0xa000).r("watchdog", FUNC(watchdog_timer_device::reset_r));
	map(0xa100, 0xa100).portr("sw1");
	map(0xa200, 0xa207).w("outlatch", FUNC(ls259_device::write_d0));
	map(0xb000, 0xbfff).rom();
	map(0xe000, 0xffff).rom();
}


//**************************************************************************
//  INPUTS
//**************************************************************************

static INPUT_PORTS_START( ambush )
	PORT_START("buttons")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_BUTTON2)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_BUTTON1)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_BUTTON2)  PORT_COCKTAIL
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_BUTTON1)  PORT_COCKTAIL
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_START2)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_START1)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_COIN2)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_COIN1)

	PORT_START("joystick")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP)     PORT_8WAY
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN)   PORT_8WAY
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT)   PORT_8WAY
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT)  PORT_8WAY
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_UP)     PORT_8WAY  PORT_COCKTAIL
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN)   PORT_8WAY  PORT_COCKTAIL
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT)   PORT_8WAY  PORT_COCKTAIL
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT)  PORT_8WAY  PORT_COCKTAIL

	PORT_START("sw1")
	PORT_DIPNAME(0x03, 0x00, DEF_STR( Lives ))           PORT_DIPLOCATION("SW1:1,2")
	PORT_DIPSETTING(   0x00, "3")
	PORT_DIPSETTING(   0x01, "4")
	PORT_DIPSETTING(   0x02, "5")
	PORT_DIPSETTING(   0x03, "6")
	PORT_DIPNAME(0x1c, 0x00, DEF_STR( Coinage ))         PORT_DIPLOCATION("SW1:3,4,5")
	PORT_DIPSETTING(   0x10, DEF_STR( 2C_1C ))
	PORT_DIPSETTING(   0x00, DEF_STR( 1C_1C ))
	PORT_DIPSETTING(   0x14, DEF_STR( 2C_3C ))
	PORT_DIPSETTING(   0x04, DEF_STR( 1C_2C ))
	PORT_DIPSETTING(   0x18, DEF_STR( 2C_5C ))
	PORT_DIPSETTING(   0x08, DEF_STR( 1C_3C ))
	PORT_DIPSETTING(   0x0c, DEF_STR( 1C_4C ))
	PORT_DIPSETTING(   0x1c, "Service Mode/Free Play")
	PORT_DIPNAME(0x20, 0x00, DEF_STR( Difficulty ))      PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(   0x00, DEF_STR( Easy ))
	PORT_DIPSETTING(   0x20, DEF_STR( Hard ))
	PORT_DIPNAME(0x40, 0x40, DEF_STR( Bonus_Life ))      PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(   0x40, "80000")
	PORT_DIPSETTING(   0x00, "120000")
	PORT_DIPNAME(0x80, 0x80, DEF_STR( Cabinet ))         PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(   0x80, DEF_STR( Upright ))
	PORT_DIPSETTING(   0x00, DEF_STR( Cocktail ))
INPUT_PORTS_END

static INPUT_PORTS_START( ambusht )
	PORT_INCLUDE(ambush)

	PORT_MODIFY("sw1")
	PORT_DIPNAME(0x04, 0x04, DEF_STR( Allow_Continue ))  PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(   0x00, DEF_STR( No ))
	PORT_DIPSETTING(   0x04, DEF_STR( Yes ))
	PORT_DIPNAME(0x08, 0x00, "Service Mode/Free Play")   PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(   0x00, DEF_STR( Off ))
	PORT_DIPSETTING(   0x08, DEF_STR( On ))
	PORT_DIPNAME(0x10, 0x00, DEF_STR( Coinage ))         PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(   0x00, "A 1 Coin/1 Credit B 1 Coin/2 Credits")
	PORT_DIPSETTING(   0x10, "A 2 Coins/3 Credits B 1 Coin/3 Credits")
INPUT_PORTS_END

static INPUT_PORTS_START( mariobl )
	PORT_INCLUDE(ambush)

	PORT_MODIFY("buttons")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_SERVICE)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_BUTTON1)        PORT_PLAYER(2)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_MODIFY("joystick")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT)  PORT_PLAYER(1) PORT_2WAY
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT) PORT_PLAYER(1) PORT_2WAY
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT) PORT_PLAYER(2) PORT_2WAY
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT)  PORT_PLAYER(2) PORT_2WAY
	PORT_BIT(0x33, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_MODIFY("sw1")
	PORT_DIPNAME(0x1c, 0x00, DEF_STR( Coinage ))      PORT_DIPLOCATION("SW1:3,4,5")
	PORT_DIPSETTING(   0x08, DEF_STR( 3C_1C ))
	PORT_DIPSETTING(   0x10, DEF_STR( 2C_1C ))
	PORT_DIPSETTING(   0x00, DEF_STR( 1C_1C ))
	PORT_DIPSETTING(   0x18, DEF_STR( 1C_2C ))
	PORT_DIPSETTING(   0x04, DEF_STR( 1C_3C ))
	PORT_DIPSETTING(   0x0c, DEF_STR( 1C_4C ))
	PORT_DIPSETTING(   0x14, DEF_STR( 1C_5C ))
	PORT_DIPSETTING(   0x1c, DEF_STR( 1C_6C ))
	PORT_DIPNAME(0x20, 0x20, "2 Players Game")        PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(   0x00, "1 Credit")
	PORT_DIPSETTING(   0x20, "2 Credits")
	PORT_DIPNAME(0xc0, 0x00, DEF_STR( Bonus_Life ))   PORT_DIPLOCATION("SW1:7,8")
	PORT_DIPSETTING(   0x00, "20k 50k 30k+")
	PORT_DIPSETTING(   0x40, "30k 60k 30k+")
	PORT_DIPSETTING(   0x80, "40k 70k 30k+")
	PORT_DIPSETTING(   0xc0, DEF_STR( None ))
INPUT_PORTS_END

static INPUT_PORTS_START( dkong3abl )
	PORT_INCLUDE(ambush)

	PORT_MODIFY("buttons")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_BUTTON1)        PORT_PLAYER(2)
	PORT_BIT(0x05, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_MODIFY("joystick")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_UP)    PORT_PLAYER(2)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN)  PORT_PLAYER(2)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT) PORT_PLAYER(2)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT)  PORT_PLAYER(2)

	PORT_MODIFY("sw1")
	PORT_DIPNAME(0x0c, 0x00, DEF_STR( Bonus_Life ))   PORT_DIPLOCATION("SW1:3,4")
	PORT_DIPSETTING(   0x00, "30k")
	PORT_DIPSETTING(   0x04, "40k")
	PORT_DIPSETTING(   0x08, "50k")
	PORT_DIPSETTING(   0x0c, DEF_STR( None ))
	PORT_DIPNAME(0x10, 0x10, DEF_STR( Unknown ))      PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(   0x10, DEF_STR( Off ))
	PORT_DIPSETTING(   0x00, DEF_STR( On ))
	PORT_DIPNAME(0x20, 0x00, DEF_STR( Coinage ))      PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(   0x00, DEF_STR( 1C_1C ))
	PORT_DIPSETTING(   0x20, DEF_STR( 2C_1C ))
	PORT_DIPNAME(0xc0, 0x00, DEF_STR( Difficulty ))   PORT_DIPLOCATION("SW1:7,8")
	PORT_DIPSETTING(   0x00, DEF_STR( Easy ))
	PORT_DIPSETTING(   0x40, DEF_STR( Medium ))
	PORT_DIPSETTING(   0x80, DEF_STR( Hard ))
	PORT_DIPSETTING(   0xc0, DEF_STR( Hardest ))
INPUT_PORTS_END


//**************************************************************************
//  PALETTES
//**************************************************************************

void ambush_state::ambush_palette(palette_device &palette) const
{
	uint8_t const *const color_prom = memregion("colors")->base();

	for (int i = 0; i < palette.entries(); i++)
	{
		int bit0, bit1, bit2;

		// red component
		bit0 = BIT(color_prom[i], 0);
		bit1 = BIT(color_prom[i], 1);
		bit2 = BIT(color_prom[i], 2);
		int const r = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		// green component
		bit0 = BIT(color_prom[i], 3);
		bit1 = BIT(color_prom[i], 4);
		bit2 = BIT(color_prom[i], 5);
		int const g = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		// blue component
		bit0 = 0;
		bit1 = BIT(color_prom[i], 6);
		bit2 = BIT(color_prom[i], 7);
		int const b = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		palette.set_pen_color(i, rgb_t(r, g, b));
	}
}

void ambush_state::mario_palette(palette_device &palette) const
{
	uint8_t const *const color_prom = memregion("colors")->base();

	for (int i = 0; i < palette.entries(); i++)
	{
		int bit0, bit1, bit2;

		// red component
		bit0 = BIT(color_prom[i], 5);
		bit1 = BIT(color_prom[i], 6);
		bit2 = BIT(color_prom[i], 7);
		int const r = 255 - (0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2);

		// green component
		bit0 = BIT(color_prom[i], 2);
		bit1 = BIT(color_prom[i], 3);
		bit2 = BIT(color_prom[i], 4);
		int const g = 255 - (0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2);

		// blue component
		bit0 = BIT(color_prom[i], 0);
		bit1 = BIT(color_prom[i], 1);
		int const b = 255 - (0x55 * bit0 + 0xaa * bit1);

		palette.set_pen_color(i, rgb_t(r, g, b));
	}
}

void ambush_state::mariobla_palette(palette_device &palette) const
{
	uint8_t const *const color_prom = memregion("colors")->base();

	for (int c = 0; c < palette.entries(); c++)
	{
		int const i = bitswap<9>(c, 2, 7, 6, 8, 5, 4, 3, 1, 0);
		int bit0, bit1, bit2;

		// red component
		bit0 = BIT(color_prom[i], 0);
		bit1 = BIT(color_prom[i], 1);
		bit2 = BIT(color_prom[i], 2);
		int const r = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		// green component
		bit0 = BIT(color_prom[i], 3);
		bit1 = BIT(color_prom[i], 4);
		bit2 = BIT(color_prom[i], 5);
		int const g = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		// blue component
		bit0 = 0;
		bit1 = BIT(color_prom[i], 6);
		bit2 = BIT(color_prom[i], 7);
		int const b = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		palette.set_pen_color(c, rgb_t(r, g, b));
	}
}

void ambush_state::dkong3_palette(palette_device &palette) const
{
	uint8_t const *const color_prom = memregion("colors")->base();

	for (int i = 0; i < palette.entries(); i++)
	{
		int bit0, bit1, bit2, bit3;

		// red component
		bit0 = BIT(color_prom[i], 4);
		bit1 = BIT(color_prom[i], 5);
		bit2 = BIT(color_prom[i], 6);
		bit3 = BIT(color_prom[i], 7);
		int const r = 255 - (0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3);

		// green component
		bit0 = BIT(color_prom[i], 0);
		bit1 = BIT(color_prom[i], 1);
		bit2 = BIT(color_prom[i], 2);
		bit3 = BIT(color_prom[i], 3);
		int const g = 255 - (0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3);

		// blue component
		bit0 = BIT(color_prom[i + 0x200], 0);
		bit1 = BIT(color_prom[i + 0x200], 1);
		bit2 = BIT(color_prom[i + 0x200], 2);
		bit3 = BIT(color_prom[i + 0x200], 3);
		int const b = 255 - (0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3);

		palette.set_pen_color(i, rgb_t(r, g, b));
	}
}


//**************************************************************************
//  VIDEO EMULATION
//**************************************************************************

uint32_t ambush_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(0, cliprect);

	// always draw all tiles
	m_char_tilemap->mark_all_dirty();

	// draw the background characters
	m_char_tilemap->draw(screen, bitmap, cliprect, TILEMAP_DRAW_OPAQUE | TILEMAP_DRAW_ALL_CATEGORIES, 0);

	// draw the sprites
	for (int offs = m_sprite_ram.bytes() - 4; offs >= 0; offs -= 4)
	{
		// 0  76653210  Y coordinate
		// 1  7-------  Flip Y
		// 1  -6------  Flip X
		// 1  --543210  Code low bits
		// 2  7-------  Size (1 = 16x16, 0 = 8x8)
		// 2  -65-----  Code high bits
		// 2  ---4----  Wrap allowed? (1 = wrap right, 0 = wrap left)
		// 2  ----3210  Color
		// 3  76653210  X coordinate

		int sx = m_sprite_ram[offs + 3];
		int sy = m_sprite_ram[offs + 0];
		int wrap = BIT(m_sprite_ram[offs + 2], 4);

		// sprite disabled?
		if (sy == 0x00 || sy == 0xff)
			continue;

		// prevent wraparound
		if (((sx < 0x40) && wrap) || ((sx >= 0xc0) && !wrap))
			continue;

		int gfx = BIT(m_sprite_ram[offs + 2], 7);
		int code = ((m_sprite_ram[offs + 2] & 0x60) << 1) | (m_sprite_ram[offs + 1] & 0x3f);

		if (gfx == 1)
		{
			// 16x16 sprites
			if (!flip_screen())
				sy = 240 - sy;
			else
				sx = 240 - sx;
		}
		else
		{
			// 8x8 sprites
			code <<= 2;

			if (!flip_screen())
				sy = 248 - sy;
			else
				sx = 248 - sx;
		}

		int flipx = BIT(m_sprite_ram[offs + 1], 6);
		int flipy = BIT(m_sprite_ram[offs + 1], 7);

		if (flip_screen())
		{
			flipx = !flipx;
			flipy = !flipy;
		}

		int color = (m_color_bank << 4) | (m_sprite_ram[offs + 2] & 0x0f);

		m_gfxdecode->gfx(gfx)->transpen(bitmap, cliprect, code, color, flipx, flipy, sx, sy, 0);
	}

	// draw the foreground priority characters with transparency over the sprites
	m_char_tilemap->draw(screen, bitmap, cliprect, 1, 0);

	return 0;
}

uint32_t ambush_state::screen_update_bootleg(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(0, cliprect);

	// always draw all tiles
	m_char_tilemap->mark_all_dirty();

	// draw the background characters
	m_char_tilemap->draw(screen, bitmap, cliprect, 0, 0);

	// draw the sprites
	for (int offs = m_sprite_ram.bytes() - 4; offs >= 0; offs -= 4)
	{
		// 0  76653210  X coordinate
		// 1  7-------  Flip X
		// 1  -6------  Code high bit
		// 1  --54----  ?
		// 1  ----3210  Color
		// 2  7-------  Flip Y
		// 2  -6543210  Code low bits
		// 3  76653210  Y coordinate

		int sx = m_sprite_ram[offs + 0];
		int sy = m_sprite_ram[offs + 3];

		// sprite disabled?
		if (sy == 0x00 || sy == 0xff)
			continue;

		// adjust for rotation
		sy = 240 - sy;

		int flipx = BIT(m_sprite_ram[offs + 1], 7);
		int flipy = BIT(m_sprite_ram[offs + 2], 7);

		int code = ((m_sprite_ram[offs + 1] & 0x40) << 1) | (m_sprite_ram[offs + 2] & 0x7f);
		int color = (m_color_bank << 4) | (m_sprite_ram[offs + 1] & 0x0f);

		m_gfxdecode->gfx(1)->transpen(bitmap, cliprect, code, color, flipx, flipy, sx, sy, 0);
	}

	return 0;
}

void ambush_state::scroll_ram_w(offs_t offset, uint8_t data)
{
	m_scroll_ram[offset] = data;
	m_char_tilemap->set_scrolly(offset, data + 1);
}

void ambush_state::color_bank_1_w(int state)
{
	m_color_bank = (m_color_bank & 0x02) | (state ? 0x01 : 0x00);
}

void ambush_state::color_bank_2_w(int state)
{
	m_color_bank = (m_color_bank & 0x01) | (state ? 0x02 : 0x00);
}


//**************************************************************************
//  DRAWGFX LAYOUTS
//**************************************************************************

static const gfx_layout spritelayout =
{
	16,16,
	RGN_FRAC(1,2), // 256 sprites
	2,
	{ RGN_FRAC(1,2), RGN_FRAC(0,2) },
	{     0,     1,     2,     3,     4,     5,     6,     7,
		8*8+0, 8*8+1, 8*8+2, 8*8+3, 8*8+4, 8*8+5, 8*8+6, 8*8+7 },
	{  0*8,  1*8,  2*8,  3*8,  4*8,  5*8,  6*8,  7*8,
		16*8, 17*8, 18*8, 19*8, 20*8, 21*8, 22*8, 23*8 },
	32*8
};

static GFXDECODE_START( gfx_ambush )
	GFXDECODE_ENTRY("gfx1", 0, gfx_8x8x2_planar, 0, 64)
	GFXDECODE_ENTRY("gfx1", 0, spritelayout,     0, 64)
GFXDECODE_END

// same layout as ambush, just 3 bits per pixel
static const gfx_layout spritelayout_mariobl =
{
	16,16,
	RGN_FRAC(1,3), // 256 sprites
	3,
	{ RGN_FRAC(2,3), RGN_FRAC(1,3), RGN_FRAC(0,3) },
	{     0,     1,     2,     3,     4,     5,     6,     7,
		8*8+0, 8*8+1, 8*8+2, 8*8+3, 8*8+4, 8*8+5, 8*8+6, 8*8+7 },
	{  0*8,  1*8,  2*8,  3*8,  4*8,  5*8,  6*8,  7*8,
		16*8, 17*8, 18*8, 19*8, 20*8, 21*8, 22*8, 23*8 },
	32*8
};

static GFXDECODE_START( gfx_mariobl )
	GFXDECODE_ENTRY("gfx1", 0, gfx_8x8x2_planar,     0, 32)
	GFXDECODE_ENTRY("gfx2", 0, spritelayout_mariobl, 0, 32)
GFXDECODE_END

static GFXDECODE_START( gfx_dkong3abl )
	GFXDECODE_ENTRY("gfx1", 0, gfx_8x8x2_planar, 0, 64)
	GFXDECODE_ENTRY("gfx2", 0, spritelayout,     0, 32)
GFXDECODE_END

TILE_GET_INFO_MEMBER( ambush_state::ambush_char_tile_info )
{
	uint8_t attr = m_attribute_ram[((tile_index >> 2) & 0xe0) | (tile_index & 0x1f)];

	// 7-------  Unused
	// -65-----  Code high bits
	// ---4----  Priority
	// ----3210  Color

	int code = ((attr & 0x60) << 3) | m_video_ram[tile_index];
	int color = (m_color_bank << 4) | (attr & 0x0f);
	tileinfo.category = BIT(attr, 4);

	tileinfo.set(0, code, color, 0);
}

TILE_GET_INFO_MEMBER( ambush_state::mariobl_char_tile_info )
{
	uint8_t attr = m_attribute_ram[((tile_index >> 2) & 0xe0) | (tile_index & 0x1f)];

	// -6------ Color bank

	int code = ((attr & 0x40) << 2) | m_video_ram[tile_index];
	int color = ((attr & 0x40) >> 2) | 8 | (m_video_ram[tile_index] >> 5);

	tileinfo.set(0, code, color, 0);
}

TILE_GET_INFO_MEMBER( ambush_state::dkong3abl_char_tile_info )
{
	uint8_t attr = m_attribute_ram[((tile_index >> 2) & 0xe0) | (tile_index & 0x1f)];

	// -6------ Color bank
	// -----210 Color

	int code = ((attr & 0x40) << 2) | m_video_ram[tile_index];
	int color = (BIT(attr, 6) << 5) | (BIT(attr, 6) << 4) | (attr & 0x07);

	tileinfo.set(0, code, color, 0);
}


//**************************************************************************
//  MACHINE EMULATION
//**************************************************************************

void ambush_state::register_save_states()
{
	save_item(NAME(m_color_bank));
}

MACHINE_START_MEMBER( ambush_state, ambush )
{
	register_save_states();

	// create character tilemap
	m_char_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(ambush_state::ambush_char_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);
	m_char_tilemap->set_transparent_pen(0);
	m_char_tilemap->set_scroll_cols(32);
}

MACHINE_START_MEMBER( ambush_state, mariobl )
{
	register_save_states();

	// create character tilemap
	m_char_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(ambush_state::mariobl_char_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);
	m_char_tilemap->set_transparent_pen(0);
	m_gfxdecode->gfx(0)->set_granularity(8);
}

MACHINE_START_MEMBER( ambush_state, dkong3abl )
{
	register_save_states();

	// create character tilemap
	m_char_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(ambush_state::dkong3abl_char_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);
	m_char_tilemap->set_transparent_pen(0);
}

void ambush_state::coin_counter_1_w(int state)
{
	machine().bookkeeping().coin_counter_w(0, state);
}

void ambush_state::coin_counter_2_w(int state)
{
	machine().bookkeeping().coin_counter_w(1, state);
}

void ambush_state::output_latches_w(offs_t offset, uint8_t data)
{
	m_outlatch[0]->write_bit(offset, BIT(data, 0));
	m_outlatch[1]->write_bit(offset, BIT(data, 1));
}


//**************************************************************************
//  MACHINE DEFINTIONS
//**************************************************************************

void ambush_state::ambush_base(machine_config &config)
{
	z80_device &maincpu(Z80(config, "maincpu", XTAL(18'432'000)/6));
	maincpu.set_addrmap(AS_PROGRAM, &ambush_state::main_map);
	maincpu.set_addrmap(AS_IO, &ambush_state::main_portmap);
	maincpu.set_vblank_int("screen", FUNC(ambush_state::irq0_line_hold));

	WATCHDOG_TIMER(config, "watchdog");

	MCFG_MACHINE_START_OVERRIDE(ambush_state, ambush)

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(XTAL(18'432'000)/3, 384, 0, 256, 264, 16, 240);
	screen.set_screen_update(FUNC(ambush_state::screen_update));
	screen.set_palette("palette");

	GFXDECODE(config, m_gfxdecode, "palette", gfx_ambush);

	PALETTE(config, "palette", FUNC(ambush_state::ambush_palette), 256);

	// sound hardware
	SPEAKER(config, "mono").front_center();
	ay8912_device &ay1(AY8912(config, "ay1", XTAL(18'432'000)/6/2));
	ay1.port_a_read_callback().set_ioport("buttons");
	ay1.add_route(ALL_OUTPUTS, "mono", 0.33);

	ay8912_device &ay2(AY8912(config, "ay2", XTAL(18'432'000)/6/2));
	ay2.port_a_read_callback().set_ioport("joystick");
	ay2.add_route(ALL_OUTPUTS, "mono", 0.33);
}

void ambush_state::ambush(machine_config &config)
{
	ambush_base(config);

	// addressable latches at 8B and 8C
	LS259(config, m_outlatch[0]);
	m_outlatch[0]->q_out_cb<4>().set(FUNC(ambush_state::flip_screen_set));
	m_outlatch[0]->q_out_cb<5>().set(FUNC(ambush_state::color_bank_1_w));
	m_outlatch[0]->q_out_cb<7>().set(FUNC(ambush_state::coin_counter_1_w));

	LS259(config, m_outlatch[1]);
	m_outlatch[1]->q_out_cb<5>().set(FUNC(ambush_state::color_bank_2_w));
	m_outlatch[1]->q_out_cb<7>().set(FUNC(ambush_state::coin_counter_2_w));
}

void ambush_state::mariobl(machine_config &config)
{
	ambush_base(config);
	subdevice<z80_device>("maincpu")->set_addrmap(AS_PROGRAM, &ambush_state::bootleg_map);

	// To be verified: do these bootlegs only have one LS259?
	ls259_device &outlatch(LS259(config, "outlatch"));
	outlatch.q_out_cb<4>().set(FUNC(ambush_state::coin_counter_1_w));
	outlatch.q_out_cb<6>().set(FUNC(ambush_state::color_bank_1_w));

	MCFG_MACHINE_START_OVERRIDE(ambush_state, mariobl)

	subdevice<screen_device>("screen")->set_screen_update(FUNC(ambush_state::screen_update_bootleg));

	m_gfxdecode->set_info(gfx_mariobl);

	subdevice<palette_device>("palette")->set_init(FUNC(ambush_state::mario_palette));

	ay8910_device &ay1(AY8910(config.replace(), "ay1", XTAL(18'432'000)/6/2));
	ay1.port_a_read_callback().set_ioport("buttons");
	ay1.add_route(ALL_OUTPUTS, "mono", 0.33);

	ay8910_device &ay2(AY8910(config.replace(), "ay2", XTAL(18'432'000)/6/2));
	ay2.port_a_read_callback().set_ioport("joystick");
	ay2.add_route(ALL_OUTPUTS, "mono", 0.33);
}

void ambush_state::mariobla(machine_config &config)
{
	mariobl(config);

	subdevice<palette_device>("palette")->set_init(FUNC(ambush_state::mariobla_palette));

	auto &outlatch(*subdevice<ls259_device>("outlatch"));
	outlatch.q_out_cb<5>().set(FUNC(ambush_state::color_bank_1_w));
	outlatch.q_out_cb<6>().set_nop();
}

void ambush_state::dkong3abl(machine_config &config)
{
	mariobl(config);

	MCFG_MACHINE_START_OVERRIDE(ambush_state, dkong3abl)

	m_gfxdecode->set_info(gfx_dkong3abl);

	subdevice<palette_device>("palette")->set_init(FUNC(ambush_state::dkong3_palette));
}


//**************************************************************************
//  ROM DEFINITIONS
//**************************************************************************

ROM_START( ambush )
	ROM_REGION(0x8000, "maincpu", 0)
	ROM_LOAD("a1.i7", 0x0000, 0x2000, CRC(31b85d9d) SHA1(24e60d053cf70ea15430d970ac2385bdd7341ab1))
	ROM_LOAD("a2.g7", 0x2000, 0x2000, CRC(8328d88a) SHA1(690f0af10a0550566b67ee570f849b2764448d15))
	ROM_LOAD("a3.f7", 0x4000, 0x2000, CRC(8db57ab5) SHA1(5cc7d7ebdfc91fb8d9ed52836d70c1de68001402))
	ROM_LOAD("a4.e7", 0x6000, 0x2000, CRC(4a34d2a4) SHA1(ad623161cd6031cb6503ff7445fdd9fb4ea83c8c))

	ROM_REGION(0x4000, "gfx1", 0)
	ROM_LOAD("fa1.m4", 0x0000, 0x2000, CRC(ad10969e) SHA1(4cfccdc4ca377693e92d77cde16f88bbdb840b38))
	ROM_LOAD("fa2.n4", 0x2000, 0x2000, CRC(e7f134ba) SHA1(c38321f3da049f756337cba5b3c71f6935922f80))

	ROM_REGION(0x200, "colors", 0)
	ROM_LOAD("a.bpr", 0x000, 0x100, CRC(5f27f511) SHA1(fe3ae701443ff50d3d03c0a5d0e0ab0e716b05cc))
	ROM_LOAD("b.bpr", 0x100, 0x100, CRC(1b03fd3b) SHA1(1a58b212476cacace6065056f23b59a69053a2f6))

	ROM_REGION(0x200, "proms", 0)
	ROM_LOAD("13.bpr", 0x000, 0x100, CRC(547e970f) SHA1(e2ec0bece49fb283e43549703d6d5d6f561c69a6))
	ROM_LOAD("14.bpr", 0x100, 0x100, CRC(622a8ce7) SHA1(6834f67874251f2ef3a33aec893311f5d10e496f))
ROM_END

ROM_START( ambushh )
	ROM_REGION(0x8000, "maincpu", 0)
	// displays an M ("Mal" is Bad in Spanish) next to ROM 1 during the test, why is internal checksum wrong (0x02 instead of 0x00) ?
	// I think the ROM has been hacked(?)
	ROM_LOAD("a1_hack.i7", 0x0000, 0x2000, CRC(a7cd149d) SHA1(470ebe60bc23a7908fb96caef8074d65f8c57625))
	// 1A6D:    0C -> 00
	// 1A75:    18 -> 0D
	// 1A76:    D5 -> 18
	// 1A77:    00 -> D6
	ROM_LOAD("a2.g7",      0x2000, 0x2000, CRC(8328d88a) SHA1(690f0af10a0550566b67ee570f849b2764448d15))
	ROM_LOAD("a3.f7",      0x4000, 0x2000, CRC(8db57ab5) SHA1(5cc7d7ebdfc91fb8d9ed52836d70c1de68001402))
	ROM_LOAD("a4.e7",      0x6000, 0x2000, CRC(4a34d2a4) SHA1(ad623161cd6031cb6503ff7445fdd9fb4ea83c8c))

	ROM_REGION(0x4000, "gfx1", 0)
	ROM_LOAD("fa1.m4", 0x0000, 0x2000, CRC(ad10969e) SHA1(4cfccdc4ca377693e92d77cde16f88bbdb840b38))
	ROM_LOAD("fa2.n4", 0x2000, 0x2000, CRC(e7f134ba) SHA1(c38321f3da049f756337cba5b3c71f6935922f80))

	ROM_REGION(0x200, "colors", 0)
	ROM_LOAD("a.bpr", 0x000, 0x100, CRC(5f27f511) SHA1(fe3ae701443ff50d3d03c0a5d0e0ab0e716b05cc))
	ROM_LOAD("b.bpr", 0x100, 0x100, CRC(1b03fd3b) SHA1(1a58b212476cacace6065056f23b59a69053a2f6))

	ROM_REGION(0x200, "proms", 0)
	ROM_LOAD("13.bpr", 0x000, 0x100, CRC(547e970f) SHA1(e2ec0bece49fb283e43549703d6d5d6f561c69a6))
	ROM_LOAD("14.bpr", 0x100, 0x100, CRC(622a8ce7) SHA1(6834f67874251f2ef3a33aec893311f5d10e496f))
ROM_END

ROM_START( ambushj )
	ROM_REGION(0x8000, "maincpu", 0)
	ROM_LOAD("ambush.h7", 0x0000, 0x2000, CRC(ce306563) SHA1(c69b5c4465187a8eda6367d6cd3e0b71a57588d1))
	ROM_LOAD("ambush.g7", 0x2000, 0x2000, CRC(90291409) SHA1(82f1e109bd066ad9fdea1ce0086be6c334e2658a))
	ROM_LOAD("ambush.f7", 0x4000, 0x2000, CRC(d023ca29) SHA1(1ac44960cf6d79936517a9ad4bae6ccd825c9496))
	ROM_LOAD("ambush.e7", 0x6000, 0x2000, CRC(6cc2d3ee) SHA1(dccb417d156460ca745d7b62f1df733cbf85d092))

	ROM_REGION(0x4000, "gfx1", 0)
	ROM_LOAD("ambush.m4", 0x0000, 0x2000, CRC(e86ca98a) SHA1(fae0786bb78ead81653adddd2edb3058371ca5bc))
	ROM_LOAD("ambush.n4", 0x2000, 0x2000, CRC(ecc0dc85) SHA1(577304bb575293b97b50eea4faafb5394e3da0f5))

	ROM_REGION(0x200, "colors", 0)
	ROM_LOAD("a.bpr", 0x000, 0x100, CRC(5f27f511) SHA1(fe3ae701443ff50d3d03c0a5d0e0ab0e716b05cc))
	ROM_LOAD("b.bpr", 0x100, 0x100, CRC(1b03fd3b) SHA1(1a58b212476cacace6065056f23b59a69053a2f6))

	ROM_REGION(0x200, "proms", 0)
	ROM_LOAD("13.bpr", 0x000, 0x100, CRC(547e970f) SHA1(e2ec0bece49fb283e43549703d6d5d6f561c69a6))
	ROM_LOAD("14.bpr", 0x100, 0x100, CRC(622a8ce7) SHA1(6834f67874251f2ef3a33aec893311f5d10e496f))
ROM_END

ROM_START( ambushv )
	ROM_REGION(0x8000, "maincpu", 0)
	ROM_LOAD("n1.h7", 0x0000, 0x2000, CRC(3c0833b4) SHA1(dd0cfb6da281742114abfe652d38038b078b959e))
	ROM_LOAD("n2.g7", 0x2000, 0x2000, CRC(90291409) SHA1(82f1e109bd066ad9fdea1ce0086be6c334e2658a))
	ROM_LOAD("n3.f7", 0x4000, 0x2000, CRC(d023ca29) SHA1(1ac44960cf6d79936517a9ad4bae6ccd825c9496))
	ROM_LOAD("n4.e7", 0x6000, 0x2000, CRC(6cc2d3ee) SHA1(dccb417d156460ca745d7b62f1df733cbf85d092))

	ROM_REGION(0x4000, "gfx1", 0)
	ROM_LOAD("f1.m4", 0x0000, 0x2000, CRC(e86ca98a) SHA1(fae0786bb78ead81653adddd2edb3058371ca5bc))
	ROM_LOAD("f2.n4", 0x2000, 0x2000, CRC(ecc0dc85) SHA1(577304bb575293b97b50eea4faafb5394e3da0f5))

	ROM_REGION(0x200, "colors", 0)
	ROM_LOAD("a.bpr", 0x000, 0x100, CRC(5f27f511) SHA1(fe3ae701443ff50d3d03c0a5d0e0ab0e716b05cc))
	ROM_LOAD("b.bpr", 0x100, 0x100, CRC(1b03fd3b) SHA1(1a58b212476cacace6065056f23b59a69053a2f6))

	ROM_REGION(0x200, "proms", 0)
	ROM_LOAD("13.bpr", 0x000, 0x100, CRC(547e970f) SHA1(e2ec0bece49fb283e43549703d6d5d6f561c69a6))
	ROM_LOAD("14.bpr", 0x100, 0x100, CRC(622a8ce7) SHA1(6834f67874251f2ef3a33aec893311f5d10e496f))
ROM_END

ROM_START( mariobl )
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("mbjba-8.7h", 0x0000, 0x4000, CRC(344c959d) SHA1(162e39c3a17e0dcde3b7eefebe224318c8884de2))
	ROM_LOAD("mbjba-7.7g", 0x4000, 0x2000, CRC(06faf308) SHA1(8c213d9390c168034c1673f3dd97b99322b3485a))
	ROM_LOAD("mbjba-6.7f", 0xe000, 0x2000, CRC(761dd670) SHA1(6d6e45ced8c535cdf56e0ed1bcedb342e9e10a55))

	ROM_REGION(0x2000, "gfx1", ROMREGION_INVERT)
	ROM_LOAD("mbjba-5.4n", 0x0000, 0x1000, CRC(9bbcf9fb) SHA1(a917241a3bd94bff72f509d6b3ab8358b9c03eac))
	ROM_LOAD("mbjba-4.4l", 0x1000, 0x1000, CRC(9379e836) SHA1(fcce66c1b2c5120441840b80723c7d209d42bc45))

	ROM_REGION(0x6000, "gfx2", 0)
	ROM_LOAD("mbjba-3.3ns", 0x0000, 0x2000, CRC(3981adb2) SHA1(c12a0c2ae04de6969f4b2dae3bdefc4515d87c55))
	ROM_LOAD("mbjba-2.3ls", 0x2000, 0x2000, CRC(7b58c92e) SHA1(25dfce7a4a93f661f495cc80378d445a2b064ba7))
	ROM_LOAD("mbjba-1.3l",  0x4000, 0x2000, CRC(c772cb8f) SHA1(7fd6dd9888928fad5c50d96b4ecff954ea8975ce))

	ROM_REGION(0x200, "colors", 0)
	ROM_LOAD("a.bpr", 0x000, 0x100, NO_DUMP)
	ROM_LOAD("b.bpr", 0x100, 0x100, NO_DUMP)
	// taken from mario
	ROM_LOAD("tma1-c-4p.4p", 0x000, 0x200, CRC(afc9bd41) SHA1(90b739c4c7f24a88b6ac5ca29b06c032906a2801))
ROM_END

ROM_START( mariobla )
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("4.7i", 0x0000, 0x4000, CRC(9a364905) SHA1(82215e4724cc01cdf552adce0df739cb2a6bc3cd))
	ROM_LOAD("3.7g", 0x4000, 0x2000, CRC(5c50209c) SHA1(e3fb4dbf9c866a346b20ec21a03d13e85b58669d))
	ROM_LOAD("2.7f", 0xe000, 0x2000, CRC(5718fe0e) SHA1(dad7158f4e7a5552f6107b825387cb6aab8dd652))

	ROM_REGION(0x8000, "gfx", 0)
	ROM_LOAD("5.4n", 0x0000, 0x2000, CRC(903615df) SHA1(d20cdf64894ad9f48af71cf2e4e4c457ccb3d365)) // mbjba-3.3ns
	ROM_CONTINUE(0x6000, 0x0800) // mbjba-5.4n [1/2]
	ROM_CONTINUE(0x6000, 0x0800) // mbjba-5.4n [1/2]
	ROM_CONTINUE(0x6800, 0x0800) // mbjba-5.4n [2/2]
	ROM_CONTINUE(0x6800, 0x0800) // mbjba-5.4n [2/2]
	ROM_LOAD("6.4l", 0x2000, 0x2000, CRC(7b58c92e) SHA1(25dfce7a4a93f661f495cc80378d445a2b064ba7)) // mbjba-2.3ls
	ROM_LOAD("7.4n", 0x4000, 0x2000, CRC(04ef8165) SHA1(71d0ee903f2e442fd7f1c76e48d99a8bec49d482)) // mbjba-1.3l
	ROM_CONTINUE(0x7000, 0x0800) // mbjba-4.4l [1/2]
	ROM_CONTINUE(0x7000, 0x0800) // mbjba-4.4l [1/2]
	ROM_CONTINUE(0x7800, 0x0800) // mbjba-4.4l [2/2]
	ROM_CONTINUE(0x7800, 0x0800) // mbjba-4.4l [2/2]

	ROM_REGION(0x2000, "gfx1", ROMREGION_INVERT)
	ROM_COPY("gfx", 0x6000, 0x0000, 0x2000)

	ROM_REGION(0x6000, "gfx2", 0)
	ROM_COPY("gfx", 0x0000, 0x0000, 0x6000)

	ROM_REGION(0x2eb, "prom", 0)
	ROM_LOAD( "6349-2n.2m",     0x000000, 0x000200, CRC(a334e4f3) SHA1(b15e3d9851b43976e98c47e3365c1b69022b0a7d))
	ROM_LOAD( "6349-2n-cpu.5b", 0x000000, 0x000200, CRC(7250ad28) SHA1(8f5342562cdcc67890cb4c4880d75f9a40e63cf8))
	ROM_LOAD( "82s153.7n",      0x000000, 0x0000eb, CRC(9da5e80d) SHA1(3bd1a55e68a7e6b7590fe3c15ae2e3a36b298fa6))

	ROM_REGION(0x200, "colors", 0)
	ROM_LOAD("6349-2n.8h", 0x000, 0x200, CRC(6a109f4b) SHA1(b117f85728afc6d3efeff0a7075b797996916f6e))
ROM_END

ROM_START( dkong3abl )
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("dk3ba-7.7i", 0x0000, 0x4000, CRC(a9263275) SHA1(c3867f6b0d379b70669b3b954e582533406db203))
	ROM_LOAD("dk3ba-6.7g", 0x4000, 0x2000, CRC(31b8401d) SHA1(0e3dfea0c7fe99d48c5d984c47fa746caf0879f3))
	ROM_CONTINUE(0x8000, 0x2000)
	ROM_LOAD("dk3ba-5.7f", 0xb000, 0x1000, CRC(07d3fd88) SHA1(721f401d077e3e051672513f9df5614eeb0f6466))

	ROM_REGION(0x2000, "gfx1", 0)
	ROM_LOAD("dk3ba-3.4l", 0x0000, 0x1000, CRC(67ac65d4) SHA1(d28bdb99310370513597ca80185ac6c56a11f63c))
	ROM_LOAD("dk3ba-4.4n", 0x1000, 0x1000, CRC(84b319d6) SHA1(eaf160948f8cd4fecfdd909876de7cd16340885c))

	ROM_REGION(0x4000, "gfx2", 0)
	ROM_LOAD("dk3ba-1.3l", 0x0000, 0x2000, CRC(d4a88e04) SHA1(4f797c25d26c1022dcf026021979ef0fbab48baf))
	ROM_LOAD("dk3ba-2.3m", 0x2000, 0x2000, CRC(f71185ee) SHA1(6652cf958d7afa8bb8dcfded997bb418a75223d8))

	ROM_REGION(0x400, "colors", 0)
	ROM_LOAD("a.bpr", 0x000, 0x100, NO_DUMP)
	ROM_LOAD("b.bpr", 0x100, 0x100, NO_DUMP)
	// taken from dkong3
	ROM_LOAD("dkc1-c.1d", 0x000, 0x200, CRC(df54befc) SHA1(7912dbf0a0c8ef68f4ae0f95e55ab164da80e4a1))
	ROM_LOAD("dkc1-c.1c", 0x200, 0x200, CRC(66a77f40) SHA1(c408d65990f0edd78c4590c447426f383fcd2d88))
ROM_END


//**************************************************************************
//  SYSTEM DRIVERS
//**************************************************************************

//    YEAR  NAME       PARENT   MACHINE    INPUT      CLASS         INIT        ROTATION  COMPANY                              FULLNAME                                           FLAGS
GAME( 1983, ambush,    0,       ambush,    ambusht,   ambush_state, empty_init, ROT0,     "Tecfri",                            "Ambush",                                          MACHINE_SUPPORTS_SAVE )
GAME( 1983, ambushh,   ambush,  ambush,    ambusht,   ambush_state, empty_init, ROT0,     "Tecfri",                            "Ambush (hack?)",                                  MACHINE_SUPPORTS_SAVE )
GAME( 1983, ambushj,   ambush,  ambush,    ambush,    ambush_state, empty_init, ROT0,     "Tecfri (Nippon Amuse license)",     "Ambush (Japan)",                                  MACHINE_SUPPORTS_SAVE )
GAME( 1983, ambushv,   ambush,  ambush,    ambush,    ambush_state, empty_init, ROT0,     "Tecfri (Volt Electronics license)", "Ambush (Volt Electronics)",                       MACHINE_SUPPORTS_SAVE )
GAME( 1983, mariobl,   mario,   mariobl,   mariobl,   ambush_state, empty_init, ROT180,   "bootleg",                           "Mario Bros. (bootleg on Ambush Hardware, set 1)", MACHINE_SUPPORTS_SAVE )
GAME( 1983, mariobla,  mario,   mariobla,  mariobl,   ambush_state, empty_init, ROT180,   "bootleg",                           "Mario Bros. (bootleg on Ambush Hardware, set 2)", MACHINE_SUPPORTS_SAVE )
GAME( 1983, dkong3abl, dkong3,  dkong3abl, dkong3abl, ambush_state, empty_init, ROT90,    "bootleg",                           "Donkey Kong 3 (bootleg on Ambush hardware)",      MACHINE_SUPPORTS_SAVE )
