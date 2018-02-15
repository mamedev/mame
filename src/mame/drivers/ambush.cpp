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
#include "screen.h"
#include "speaker.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class ambush_state : public driver_device
{
public:
	ambush_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_gfxdecode(*this, "gfxdecode"),
		m_outlatch(*this, "outlatch%u", 1),
		m_video_ram(*this, "video_ram"),
		m_sprite_ram(*this, "sprite_ram"),
		m_attribute_ram(*this, "attribute_ram"),
		m_scroll_ram(*this, "scroll_ram"),
		m_char_tilemap(nullptr),
		m_color_bank(0)
	{ }

	DECLARE_PALETTE_INIT(ambush);
	DECLARE_PALETTE_INIT(mario);
	DECLARE_PALETTE_INIT(dkong3);

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_bootleg(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	TILE_GET_INFO_MEMBER(ambush_char_tile_info);
	TILE_GET_INFO_MEMBER(mariobl_char_tile_info);
	TILE_GET_INFO_MEMBER(dkong3abl_char_tile_info);

	DECLARE_WRITE_LINE_MEMBER(flip_screen_w);
	DECLARE_WRITE8_MEMBER(scroll_ram_w);
	DECLARE_WRITE_LINE_MEMBER(color_bank_1_w);
	DECLARE_WRITE_LINE_MEMBER(color_bank_2_w);

	DECLARE_MACHINE_START(ambush);
	DECLARE_MACHINE_START(mariobl);
	DECLARE_MACHINE_START(dkong3abl);

	DECLARE_WRITE_LINE_MEMBER(coin_counter_1_w);
	DECLARE_WRITE_LINE_MEMBER(coin_counter_2_w);
	DECLARE_WRITE8_MEMBER(output_latches_w);

	void mariobl(machine_config &config);
	void ambush(machine_config &config);
	void dkong3abl(machine_config &config);
	void bootleg_map(address_map &map);
	void main_map(address_map &map);
	void main_portmap(address_map &map);
private:
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

ADDRESS_MAP_START(ambush_state::main_map)
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0x8000, 0x87ff) AM_RAM
	AM_RANGE(0xa000, 0xa000) AM_DEVREAD("watchdog", watchdog_timer_device, reset_r)
	AM_RANGE(0xc000, 0xc07f) AM_RAM
	AM_RANGE(0xc080, 0xc09f) AM_RAM_WRITE(scroll_ram_w) AM_SHARE("scroll_ram")  // 1 byte for each column
	AM_RANGE(0xc0a0, 0xc0ff) AM_RAM
	AM_RANGE(0xc100, 0xc1ff) AM_RAM AM_SHARE("attribute_ram")  // 1 line corresponds to 4 in the video ram
	AM_RANGE(0xc200, 0xc3ff) AM_RAM AM_SHARE("sprite_ram")
	AM_RANGE(0xc400, 0xc7ff) AM_RAM AM_SHARE("video_ram")
	AM_RANGE(0xc800, 0xc800) AM_READ_PORT("sw1")
	AM_RANGE(0xcc00, 0xcc07) AM_WRITE(output_latches_w)
ADDRESS_MAP_END

ADDRESS_MAP_START(ambush_state::main_portmap)
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x00) AM_DEVREADWRITE("ay1", ay8910_device, data_r, address_w)
	AM_RANGE(0x01, 0x01) AM_DEVWRITE("ay1", ay8910_device, data_w)
	AM_RANGE(0x80, 0x80) AM_DEVREADWRITE("ay2", ay8910_device, data_r, address_w)
	AM_RANGE(0x81, 0x81) AM_DEVWRITE("ay2", ay8910_device, data_w)
ADDRESS_MAP_END

ADDRESS_MAP_START(ambush_state::bootleg_map)
	AM_RANGE(0x0000, 0x5fff) AM_ROM
	AM_RANGE(0x6000, 0x6fff) AM_RAM
	AM_RANGE(0x7000, 0x77ff) AM_RAM
	AM_RANGE(0x7000, 0x71ff) AM_SHARE("sprite_ram")
	AM_RANGE(0x7200, 0x72ff) AM_SHARE("attribute_ram")
	AM_RANGE(0x7380, 0x739f) AM_SHARE("scroll_ram")  // not used on bootlegs?
	AM_RANGE(0x7400, 0x77ff) AM_SHARE("video_ram")
	AM_RANGE(0x8000, 0x9fff) AM_ROM
	AM_RANGE(0xa000, 0xa000) AM_DEVREAD("watchdog", watchdog_timer_device, reset_r)
	AM_RANGE(0xa100, 0xa100) AM_READ_PORT("sw1")
	AM_RANGE(0xa200, 0xa207) AM_DEVWRITE("outlatch", ls259_device, write_d0)
	AM_RANGE(0xb000, 0xbfff) AM_ROM
	AM_RANGE(0xe000, 0xffff) AM_ROM
ADDRESS_MAP_END


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

PALETTE_INIT_MEMBER( ambush_state, ambush )
{
	const uint8_t *color_prom = memregion("colors")->base();

	for (int i = 0; i < palette.entries(); i++)
	{
		int bit0, bit1, bit2, r, g, b;

		// red component
		bit0 = (color_prom[i] >> 0) & 0x01;
		bit1 = (color_prom[i] >> 1) & 0x01;
		bit2 = (color_prom[i] >> 2) & 0x01;
		r = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		// green component
		bit0 = (color_prom[i] >> 3) & 0x01;
		bit1 = (color_prom[i] >> 4) & 0x01;
		bit2 = (color_prom[i] >> 5) & 0x01;
		g = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		// blue component
		bit0 = 0;
		bit1 = (color_prom[i] >> 6) & 0x01;
		bit2 = (color_prom[i] >> 7) & 0x01;
		b = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		palette.set_pen_color(i, rgb_t(r,g,b));
	}
}

PALETTE_INIT_MEMBER( ambush_state, mario )
{
	const uint8_t *color_prom = memregion("colors")->base();

	for (int i = 0; i < palette.entries(); i++)
	{
		int bit0, bit1, bit2, r, g, b;

		// red component
		bit0 = (color_prom[i] >> 5) & 1;
		bit1 = (color_prom[i] >> 6) & 1;
		bit2 = (color_prom[i] >> 7) & 1;
		r = 255 - (0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2);

		// green component
		bit0 = (color_prom[i] >> 2) & 1;
		bit1 = (color_prom[i] >> 3) & 1;
		bit2 = (color_prom[i] >> 4) & 1;
		g = 255 - (0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2);

		// blue component
		bit0 = (color_prom[i] >> 0) & 1;
		bit1 = (color_prom[i] >> 1) & 1;
		b = 255 - (0x55 * bit0 + 0xaa * bit1);

		palette.set_pen_color(i, rgb_t(r,g,b));
	}
}

PALETTE_INIT_MEMBER( ambush_state, dkong3 )
{
	const uint8_t *color_prom = memregion("colors")->base();

	for (int i = 0; i < palette.entries(); i++)
	{
		int bit0, bit1, bit2, bit3, r, g, b;

		// red component
		bit0 = (color_prom[i] >> 4) & 0x01;
		bit1 = (color_prom[i] >> 5) & 0x01;
		bit2 = (color_prom[i] >> 6) & 0x01;
		bit3 = (color_prom[i] >> 7) & 0x01;
		r = 255 - (0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3);

		// green component
		bit0 = (color_prom[i] >> 0) & 0x01;
		bit1 = (color_prom[i] >> 1) & 0x01;
		bit2 = (color_prom[i] >> 2) & 0x01;
		bit3 = (color_prom[i] >> 3) & 0x01;
		g = 255 - (0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3);

		// blue component
		bit0 = (color_prom[i + 0x200] >> 0) & 0x01;
		bit1 = (color_prom[i + 0x200] >> 1) & 0x01;
		bit2 = (color_prom[i + 0x200] >> 2) & 0x01;
		bit3 = (color_prom[i + 0x200] >> 3) & 0x01;
		b = 255 - (0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3);

		palette.set_pen_color(i, rgb_t(r,g,b));
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

WRITE_LINE_MEMBER(ambush_state::flip_screen_w)
{
	flip_screen_set(state);
}

WRITE8_MEMBER( ambush_state::scroll_ram_w )
{
	m_scroll_ram[offset] = data;
	m_char_tilemap->set_scrolly(offset, data + 1);
}

WRITE_LINE_MEMBER(ambush_state::color_bank_1_w)
{
	m_color_bank = (m_color_bank & 0x02) | (state ? 0x01 : 0x00);
}

WRITE_LINE_MEMBER(ambush_state::color_bank_2_w)
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

static GFXDECODE_START( ambush )
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

static GFXDECODE_START( mariobl )
	GFXDECODE_ENTRY("gfx1", 0, gfx_8x8x2_planar,     0, 32)
	GFXDECODE_ENTRY("gfx2", 0, spritelayout_mariobl, 0, 32)
GFXDECODE_END

static GFXDECODE_START( dkong3abl )
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

	SET_TILE_INFO_MEMBER(0, code, color, 0);
}

TILE_GET_INFO_MEMBER( ambush_state::mariobl_char_tile_info )
{
	uint8_t attr = m_attribute_ram[((tile_index >> 2) & 0xe0) | (tile_index & 0x1f)];

	// -6------ Color bank

	int code = ((attr & 0x40) << 2) | m_video_ram[tile_index];
	int color = ((attr & 0x40) >> 2) | 8 | (m_video_ram[tile_index] >> 5);

	SET_TILE_INFO_MEMBER(0, code, color, 0);
}

TILE_GET_INFO_MEMBER( ambush_state::dkong3abl_char_tile_info )
{
	uint8_t attr = m_attribute_ram[((tile_index >> 2) & 0xe0) | (tile_index & 0x1f)];

	// -6------ Color bank
	// -----210 Color

	int code = ((attr & 0x40) << 2) | m_video_ram[tile_index];
	int color = (BIT(attr, 6) << 5) | (BIT(attr, 6) << 4) | (attr & 0x07);

	SET_TILE_INFO_MEMBER(0, code, color, 0);
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
	m_char_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(FUNC(ambush_state::ambush_char_tile_info), this), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);
	m_char_tilemap->set_transparent_pen(0);
	m_char_tilemap->set_scroll_cols(32);
}

MACHINE_START_MEMBER( ambush_state, mariobl )
{
	register_save_states();

	// create character tilemap
	m_char_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(FUNC(ambush_state::mariobl_char_tile_info), this), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);
	m_char_tilemap->set_transparent_pen(0);
	m_gfxdecode->gfx(0)->set_granularity(8);
}

MACHINE_START_MEMBER( ambush_state, dkong3abl )
{
	register_save_states();

	// create character tilemap
	m_char_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(FUNC(ambush_state::dkong3abl_char_tile_info), this), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);
	m_char_tilemap->set_transparent_pen(0);
}

WRITE_LINE_MEMBER(ambush_state::coin_counter_1_w)
{
	machine().bookkeeping().coin_counter_w(0, state);
}

WRITE_LINE_MEMBER(ambush_state::coin_counter_2_w)
{
	machine().bookkeeping().coin_counter_w(1, state);
}

WRITE8_MEMBER(ambush_state::output_latches_w)
{
	m_outlatch[0]->write_bit(offset, BIT(data, 0));
	m_outlatch[1]->write_bit(offset, BIT(data, 1));
}


//**************************************************************************
//  MACHINE DEFINTIONS
//**************************************************************************

MACHINE_CONFIG_START(ambush_state::ambush)
	MCFG_CPU_ADD("maincpu", Z80, XTAL(18'432'000)/6)
	MCFG_CPU_PROGRAM_MAP(main_map)
	MCFG_CPU_IO_MAP(main_portmap)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", ambush_state, irq0_line_hold)

	// addressable latches at 8B and 8C
	MCFG_DEVICE_ADD("outlatch1", LS259, 0)
	MCFG_ADDRESSABLE_LATCH_Q4_OUT_CB(WRITELINE(ambush_state, flip_screen_w))
	MCFG_ADDRESSABLE_LATCH_Q5_OUT_CB(WRITELINE(ambush_state, color_bank_1_w))
	MCFG_ADDRESSABLE_LATCH_Q7_OUT_CB(WRITELINE(ambush_state, coin_counter_1_w))
	MCFG_DEVICE_ADD("outlatch2", LS259, 0)
	MCFG_ADDRESSABLE_LATCH_Q5_OUT_CB(WRITELINE(ambush_state, color_bank_2_w))
	MCFG_ADDRESSABLE_LATCH_Q7_OUT_CB(WRITELINE(ambush_state, coin_counter_2_w))

	MCFG_WATCHDOG_ADD("watchdog")

	MCFG_MACHINE_START_OVERRIDE(ambush_state, ambush)

	// video hardware
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_RAW_PARAMS(XTAL(18'432'000)/3, 384, 0, 256, 264, 16, 240)
	MCFG_SCREEN_UPDATE_DRIVER(ambush_state, screen_update)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", ambush)

	MCFG_PALETTE_ADD("palette", 256)
	MCFG_PALETTE_INIT_OWNER(ambush_state, ambush)

	// sound hardware
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("ay1", AY8912, XTAL(18'432'000)/6/2)
	MCFG_AY8910_PORT_A_READ_CB(IOPORT("buttons"))
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.33)

	MCFG_SOUND_ADD("ay2", AY8912, XTAL(18'432'000)/6/2)
	MCFG_AY8910_PORT_A_READ_CB(IOPORT("joystick"))
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.33)
MACHINE_CONFIG_END

MACHINE_CONFIG_START(ambush_state::mariobl)
	ambush(config);
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(bootleg_map)

	// To be verified: do these bootlegs only have one LS259?
	MCFG_DEVICE_REMOVE("outlatch1")
	MCFG_DEVICE_REMOVE("outlatch2")
	MCFG_DEVICE_ADD("outlatch", LS259, 0)
	MCFG_ADDRESSABLE_LATCH_Q4_OUT_CB(WRITELINE(ambush_state, coin_counter_1_w))
	MCFG_ADDRESSABLE_LATCH_Q6_OUT_CB(WRITELINE(ambush_state, color_bank_1_w))

	MCFG_MACHINE_START_OVERRIDE(ambush_state, mariobl)

	MCFG_SCREEN_MODIFY("screen")
	MCFG_SCREEN_UPDATE_DRIVER(ambush_state, screen_update_bootleg)

	MCFG_GFXDECODE_MODIFY("gfxdecode", mariobl)

	MCFG_PALETTE_MODIFY("palette")
	MCFG_PALETTE_INIT_OWNER(ambush_state, mario)

	MCFG_SOUND_REPLACE("ay1", AY8910, XTAL(18'432'000)/6/2)
	MCFG_AY8910_PORT_A_READ_CB(IOPORT("buttons"))
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.33)

	MCFG_SOUND_REPLACE("ay2", AY8910, XTAL(18'432'000)/6/2)
	MCFG_AY8910_PORT_A_READ_CB(IOPORT("joystick"))
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.33)
MACHINE_CONFIG_END

MACHINE_CONFIG_START(ambush_state::dkong3abl)
	mariobl(config);
	MCFG_MACHINE_START_OVERRIDE(ambush_state, dkong3abl)

	MCFG_GFXDECODE_MODIFY("gfxdecode", dkong3abl)

	MCFG_PALETTE_MODIFY("palette")
	MCFG_PALETTE_INIT_OWNER(ambush_state, dkong3)
MACHINE_CONFIG_END


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

//    YEAR  NAME       PARENT   MACHINE    INPUT      CLASS         INIT  ROTATION  COMPANY                              FULLNAME                                      FLAGS
GAME( 1983, ambush,    0,       ambush,    ambusht,   ambush_state, 0,    ROT0,     "Tecfri",                            "Ambush",                                     MACHINE_SUPPORTS_SAVE )
GAME( 1983, ambushh,   ambush,  ambush,    ambusht,   ambush_state, 0,    ROT0,     "Tecfri",                            "Ambush (hack?)",                             MACHINE_SUPPORTS_SAVE )
GAME( 1983, ambushj,   ambush,  ambush,    ambush,    ambush_state, 0,    ROT0,     "Tecfri (Nippon Amuse license)",     "Ambush (Japan)",                             MACHINE_SUPPORTS_SAVE )
GAME( 1983, ambushv,   ambush,  ambush,    ambush,    ambush_state, 0,    ROT0,     "Tecfri (Volt Electronics license)", "Ambush (Volt Electronics)",                  MACHINE_SUPPORTS_SAVE )
GAME( 1983, mariobl,   mario,   mariobl,   mariobl,   ambush_state, 0,    ROT180,   "bootleg",                           "Mario Bros. (bootleg on Ambush Hardware)",   MACHINE_SUPPORTS_SAVE )
GAME( 1983, dkong3abl, dkong3,  dkong3abl, dkong3abl, ambush_state, 0,    ROT90,    "bootleg",                           "Donkey Kong 3 (bootleg on Ambush hardware)", MACHINE_SUPPORTS_SAVE )
