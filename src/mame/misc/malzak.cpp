// license:BSD-3-Clause
// copyright-holders:Pierpaolo Prazzoli, Barry Rodewald
/*

  Malzak

  Produced by Kitronix, cabinet artwork suggests it was
  released in 1980.

  Was sold around New Zealand in the early '80s.
  Sold about 500 sets, fairly decent for the time.

  There are more games on this hardware, as far as I am aware.

  Driver by Reip, Barry Rodewald
  SAA 5050 display code "borrowed" from the Philips 2000T/2000M driver in MESS

  Basic memory map

    0x0000 - 0x0fff | S2650 code [malzak.5, malzak.4, malzak.2 (data only)]
    0x1000 - 0x13ff | Work RAM
    0x1400 - 0x14ff | S2636 #1 video
    0x1500 - 0x15ff | S2636 #2 video
    0x1600 - 0x16ff | Playfield gfx
    0x1700 - 0x17ff | Work RAM - hi-score table (0x1700), coin meter (0x1780),
                    |            config byte (0x17fd); battery NVRAM on Malzak II
    0x1800 - 0x1fff | SAA 5050 video RAM

  TODO:
  - sprite-playfield colors are dubious at best;
  - Tiles may be flipped for playfield in final stage;
  - Ranking table doesn't sort properly when player gets a better score than
    the ones listed, CPU core bug?
  - improve gfx layers superimposing, honor PAL timings;
  - sound (2x SN76477)
  - sprite->sprite collisions aren't quite perfect
    (you can often fly through flying missiles) <- does this still occur -AS?

  Notes:
  - Test mode in Malzak II should be enabled by setting a POT to position
  '4' and pressing button 1 three times.  Then the POT is set to position
  '1' to go back to the game.

  POT / service switch and the Malzak I/II version select (decoded):
  - The cabinet's only configuration input is the POT, a 4-position rotary
    switch read through S2636 #1 port A at 0x14cc.  The boot dispatcher at
    0x00a8 maps its positions to:
      1  Normal play
      2  Clear the high-score table (zeroes 0x1700-0x175a)
      3  Coin / bookkeeping audit (shows the 8-digit BCD coin meter at
         0x1780, incremented once per coin at 0x0705; reset with start+coin)
      4  Settings / test menu (writes the NVRAM config byte 0x17fd)
  - The POT=4 menu toggles three bits of battery NVRAM 0x17fd with button 1:
      0x01  "Initials shoot"   (high-score entry style)
      0x02  "Malzak version"   one / two
      0x04  "Continue option"  on / off
  - "Malzak version two" (0x17fd & 0x02) is what makes the sequel play like
    the sequel: it gates the per-frame 0x2800 routine and, via port 0x40 bit
    6, banks in the version-II screen data (second half of "user2").  With
    the bit clear the malzak2 board plays like the original.  Version-II
    extras behind this gate include the HUD edge arrows, a right-edge scroll
    "turbo" (per-frame scroll step 0x1023 is loaded from 0x10cf, which
    becomes 2 at the right edge), and a 0x0f "guided-target" scoring
    structure spawned on a survival milestone.  malzak (v1) has none of this
    code and does not read the POT at all.

  Guru Notes:
  This board looks like an early proto, it's hand etched, with
  a ton of wire mods. I'm told the game is kind of like Scramble.
  It's probably _ultra_ rare too.

  The board looks like pure crap actually, with lot's of resistors
  jumpering tracks and a partial harness that is wired directly to
  the logic chips which has been *ruthlessly* chopped off!

  Here's what I can see.....

  EPROMs (x5, dumped, MALZAK.1 by itself, the other 4 grouped together)
  S 8039 2650AI (DIP 40)
  S 8051 2636N (DIP 40)
  S 8112 2636N (DIP 40)
  5101 (SRAM, x3)
  6MHz Xtal
  2114 (SRAM, x4)
  SAA 5020 (DIP 24)
  SAA 5050 (DIP 28)
  SN76477 (x2, DIP28)
  INS/DP8212N P8212 (x2, DIP 24)
  plenty of logic chips, resistors, caps etc

  Note: There's no PALs or PROMs

*/

#include "emu.h"

#include "cpu/s2650/s2650.h"
#include "machine/nvram.h"
#include "machine/s2636.h"
#include "sound/sn76477.h"
#include "video/saa5050.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"

#include <algorithm>

#define LOG_COLL (1U << 1)   // sprite<->playfield collision decision trace (see screen_vblank)
#define VERBOSE (0)
#include "logmacro.h"

namespace {

class malzak_state : public driver_device
{
public:
	malzak_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_s2636(*this, "s2636%u", 0U)
		, m_trom(*this, "saa5050")
		, m_videoram(*this, "videoram")
		, m_gfxdecode(*this, "gfxdecode")
		, m_screen(*this, "screen")
		, m_palette(*this, "palette")
		, m_mainbank(*this, "mainbank")
	{ }

	void malzak(machine_config &config) ATTR_COLD;
	void malzak2(machine_config &config) ATTR_COLD;

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	uint8_t fake_VRLE_r();
	uint8_t s2636_portA_r();
	uint8_t s2650_data_r();
	void port40_w(uint8_t data);
	void port60_w(uint8_t data);
	void portc0_w(uint8_t data);
	uint8_t collision_r();
	void playfield_w(offs_t offset, uint8_t data);
	uint8_t videoram_r(offs_t offset);

	void palette_init(palette_device &palette) const;
	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void screen_vblank(int state);
	virtual void video_start() override ATTR_COLD;

	void malzak2_map(address_map &map) ATTR_COLD;
	void malzak_data_map(address_map &map) ATTR_COLD;
	void malzak_io_map(address_map &map) ATTR_COLD;
	void malzak_map(address_map &map) ATTR_COLD;

	TILE_GET_INFO_MEMBER(get_tile_info);

	/* devices */
	required_device<s2650_device> m_maincpu;
	required_device_array<s2636_device, 2> m_s2636;
	required_device<saa5050_device> m_trom;
	required_shared_ptr<uint8_t> m_videoram;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	required_memory_bank m_mainbank;

	bitmap_rgb32 m_trom_bitmap;
	bitmap_rgb32 m_playfield_bitmap;
	tilemap_t *m_playfield_tilemap = nullptr;
	int m_playfield_code[256]{};
	int m_scrollx = 0;
	int m_scrolly = 0;
	u8  m_collision = 0;
	u8  m_playfield_bank = 0;
};

/*
 * Video HW
 *
 * SAA 5050 -- Character display
 * S2636 (x2) -- Sprites, Sprite->Sprite collisions
 * Playfield graphics generator
 *
 */

void malzak_state::palette_init(palette_device &palette) const
{
	for (int i = 0; i < 8 * 8; i++)
	{
		palette.set_pen_color(i * 2 + 0, pal1bit(i >> 3), pal1bit(i >> 4), pal1bit(i >> 5));
		palette.set_pen_color(i * 2 + 1, pal1bit(i >> 0), pal1bit(i >> 1), pal1bit(i >> 2));
	}
}

void malzak_state::video_start()
{
	m_scrollx = 0;

	int width = m_screen->width();
	int height = m_screen->height();
	m_trom_bitmap.allocate(width, height);
	m_playfield_bitmap.allocate(width, height);
	m_playfield_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(malzak_state::get_tile_info)), TILEMAP_SCAN_COLS, 16, 16, 16, 16);
}

uint8_t malzak_state::videoram_r(offs_t offset)
{
	return m_videoram[offset];
}

void malzak_state::port60_w(uint8_t data)
{
	m_scrollx = data;
//  logerror("I/O: port 0x60 write 0x%02x\n", data);
	m_playfield_tilemap->set_scrollx(0, m_scrollx + 48);
}

void malzak_state::portc0_w(uint8_t data)
{
	m_scrolly = data;
//  logerror("I/O: port 0xc0 write 0x%02x\n", data);
	m_playfield_tilemap->set_scrolly(0, m_scrolly);
}

//TODO: how readback works with this arrangement? Never occurs in-game
void malzak_state::playfield_w(offs_t offset, uint8_t data)
{
	int tile = ((m_scrollx / 16) * 16) + (offset / 16);

//  m_playfield_x[tile] = m_malzak_x / 16;
//  m_playfield_y[tile] = m_malzak_y;
	m_playfield_code[tile] = data;
	m_playfield_tilemap->mark_tile_dirty(tile);
	// POST only, adds to the scrollx base address?
//  if (offset & 0xf)
//      popmessage("GFX: 0x16%02x write 0x%02x\n", offset, data);
}

TILE_GET_INFO_MEMBER(malzak_state::get_tile_info)
{
	u8 code = (m_playfield_code[tile_index] & 0x1f) | (m_playfield_bank << 5);
	u8 color = ((m_playfield_code[tile_index] & 0xe0) >> 5);
	tileinfo.set(0, code, color, 0);
}


uint32_t malzak_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	const rgb_t *palette = m_palette->palette()->entry_list_raw();

	bitmap.fill(rgb_t::black(), cliprect);

	// prepare bitmaps
	m_trom->screen_update(screen, m_trom_bitmap, cliprect);
	m_playfield_tilemap->draw(screen, m_playfield_bitmap, cliprect, 0, 0);
	bitmap_ind16 const &s2636_0_bitmap = m_s2636[0]->update(cliprect);
	bitmap_ind16 const &s2636_1_bitmap = m_s2636[1]->update(cliprect);

	// Superimpose
	// TODO: update for PAL size
	for (int y = cliprect.min_y; y <= cliprect.max_y; ++y)
	{
		int sy = y / 2;
		for (int x = cliprect.min_x; x <= cliprect.max_x; ++x)
		{
			int sx = x / 2;
			int s2636_pix_0 = s2636_0_bitmap.pix(sy, sx);
			int s2636_pix_1 = s2636_1_bitmap.pix(sy, sx);
			rgb_t trom_pix = m_trom_bitmap.pix(y, x);
			rgb_t play_pix = m_playfield_bitmap.pix(sy, sx);

			// SAA5050 > s2636[1] > s2636[0] > playfield
			if (trom_pix != rgb_t::black())
				bitmap.pix(y, x) = trom_pix;
			else if (S2636_IS_PIXEL_DRAWN(s2636_pix_1))
				bitmap.pix(y, x) = palette[S2636_PIXEL_COLOR(s2636_pix_1)];
			else if (S2636_IS_PIXEL_DRAWN(s2636_pix_0))
				bitmap.pix(y, x) = palette[S2636_PIXEL_COLOR(s2636_pix_0)];
			else
				bitmap.pix(y, x) = play_pix;
		}
	}

	return 0;
}

void malzak_state::screen_vblank(int state)
{
	// Reconstruct the sprite<->playfield collision once per emulated frame, on the rising edge
	// of VBLANK.  This is machine logic (it drives the collision register the CPU reads), so it
	// runs in emulation time rather than in screen_update(): computing it in the view would make
	// it depend on frameskip/host drawing and break -record/-playback.  The S2636 sprite<->sprite
	// collision is likewise refreshed here (m_s2636[n]->update()), so the screen no longer needs
	// the VIDEO_ALWAYS_UPDATE attribute.
	if (!state)
		return;

	const rectangle &va = m_screen->visible_area();
	m_playfield_tilemap->draw(*m_screen, m_playfield_bitmap, va, 0, 0);
	bitmap_ind16 const &s2636_0_bitmap = m_s2636[0]->update(va);
	bitmap_ind16 const &s2636_1_bitmap = m_s2636[1]->update(va);
	address_space &prog = m_maincpu->space(AS_PROGRAM);

	// Sprite <-> playfield collision.  The single collision register is a shared POSITION latch
	// (board glue, not an S2636 feature); the game classifies each hit itself.  0x0A50 looks up the
	// per-column terrain table [0x1060 + col] and, for a structure column whose surface row equals
	// the reported row (gates 1/2), consumes the structure (writes 0x18), applies its effect at
	// 0x08A0 (fuel depot: [0x10D1] += 0x20; buildings: bonus), spawns the explosion entry and then
	// FALLS THROUGH (0x0AC5) to 0x412F, which transforms the reported (row,col) by the ship scroll
	// and matches it against each sprite -- missile/enemies/bomb while their timers are armed, and
	// the ship always (0x41EC -> crash 0x2400).  So a ship touching a depot destroys it, banks its
	// fuel and dies in the same frame (Scramble semantics); there is no safe "skim refuel".
	//
	// The classifier is TIME-GATED: each non-ship object carries a countdown timer in RAM (missile
	// 0x10C3, enemies 0x10C4/0x10BF, bomb 0x10BE) that is armed to 0x0A when the object is activated
	// and decremented every frame (routine ~0x20B2); 0x412F tests an object's position only while its
	// timer is non-zero, i.e. for ~10 frames.  The SHIP has no timer -- its check (0x41EC) is always
	// live, which is why it is the gate a mis-reported object falls onto.  (This is also why the game
	// lets a shot pass through distant terrain: the missile's window has closed.)
	//
	// We report the CONTACT PIXEL (the first drawn object pixel that overlaps a playfield pixel):
	// its cell is the structure's surface row when something lands on a structure (so gates 1/2
	// register the hit), and it lies inside the object's footprint, so 0x412F reliably identifies
	// the colliding object.  Ship first, then the forward missile, then the bomb.  The danger with
	// the shared latch is the ship crash gate at 0x41EC: it has NO arming flag and a ~26px tolerance,
	// so any reported non-ship object whose transformed position lands near the ship kills the player
	// (a shot or a bomb hitting ground right next to a low ship).  For those we run `would_crash_ship`
	// (the game's own 0x412F transform tested against the ship gate) and suppress the report; the
	// object is simply reported a frame or two later, once the scroll has carried it clear.
	{
		u8 const scrolly = prog.read_byte(0x1022);
		// True where a drawn pixel of an object coincides with a playfield pixel.  In the S2636
		// bitmap (same coordinate space as m_playfield_bitmap) object i is drawn at top-left
		// (py = VC + m_y_offset, px = (HC + m_x_offset) * m_divider); malzak sets m_y_offset = 0,
		// m_x_offset = -16, m_divider = 1, so top-left = (yreg, xreg - 16), footprint
		// (8 << scale) x (10 << scale).  We scan that footprint only, so pixels from other objects
		// are out of range and cannot false-trigger this object's collision.
		// On a hit, report the bitmap coords of the contact pixel (out_px/out_py) so the caller can
		// encode the cell of the actual contact point.
		auto overlaps = [this](bitmap_ind16 const &spr, s2636_device &dev, int obj, u8 xreg, u8 yreg, int &out_px, int &out_py) -> bool
		{
			if (yreg >= 0xf0)                      // parked / inactive object (y == 0xf8)
				return false;
			int const scale = (dev.read_data(0xc0) >> (2 * obj)) & 0x03;   // REG_OBJ_SIZE
			int const w = 8 << scale, h = 10 << scale;
			int const by = yreg;                   // sprite top bitmap row (VC + m_y_offset, both 0-based)
			int const bx = int(xreg) - 16;         // sprite left bitmap column (HC + m_x_offset, divider 1)
			for (int dy = 0; dy < h; dy++)
				for (int dx = 0; dx < w; dx++)
				{
					int const py = by + dy, px = bx + dx;
					// A drawn, visible sprite pixel (bit 0x08 set, colour != 0) coinciding with a
					// non-black playfield pixel is a real contact.  malzak's S2636 pixels do not
					// carry the object-tag bits, so match on the drawn+colour flags within the
					// object's own footprint rather than a tag.
					if (py >= 0 && px >= 0 && py < m_playfield_bitmap.height() && px < m_playfield_bitmap.width()
						&& S2636_IS_PIXEL_DRAWN(spr.pix(py, px)) && S2636_PIXEL_COLOR(spr.pix(py, px)) != 0
						&& m_playfield_bitmap.pix(py, px) != rgb_t::black())
					{
						out_px = px; out_py = py;
						return true;
					}
				}
			return false;
		};
		// Encode a contact position into the shared latch value: the 16x16 cell containing the point
		// in the game's scroll-adjusted frame.  The column formula matches the game's [0x1060] slot
		// mapping; the row formula puts a tile-aligned structure's top edge in its own surface row,
		// which is what the game's height gate (0x0A6A) tests against.
		auto encode = [this, scrolly](u8 x, u8 y) -> u8
		{
			u8 const col = ((x + m_scrollx + 0x1f) >> 4) & 0x0f;
			u8 const row = ((y + scrolly) >> 4) & 0x0f;
			return (row << 4) | col;
		};
		u8 const ship_x = m_s2636[0]->read_data(0x0a), ship_y = m_s2636[0]->read_data(0x0c);
		u8 const bomb_x = m_s2636[0]->read_data(0x4a), bomb_y = m_s2636[0]->read_data(0x4c);
		u8 const miss_x = m_s2636[1]->read_data(0x0a), miss_y = m_s2636[1]->read_data(0x0c);
		// Predict whether a plain-terrain report of a non-ship object at (mx,my) would land on the
		// *ship* crash gate (0x41EC -> 0x2400), which has no arming flag.  We apply the game's own
		// 0x412F transform to the value we would report and test the ship's tolerance window; if it
		// hits, reporting that object would kill the player on their own shot/bomb, so we suppress.
		// The latch we set is read one frame later, after the scroll ([0x1021]/[0x1022]) has advanced
		// a pixel or two, so we widen the ship window by DRIFT on each side to cover that lag -- else
		// a bomb sitting exactly at the tolerance boundary tips onto the crash gate next frame (a rare
		// "phantom death" seen via -playback).
		constexpr int DRIFT = 4;   // px of scroll advance between our compute and the game's read
		auto would_crash_ship = [this, &prog, scrolly, ship_x, ship_y](u8 mx, u8 my) -> bool
		{
			u8 const col = ((mx + m_scrollx + 0x1f) >> 4) & 0x0f;
			u8 const row = ((my + scrolly) >> 4) & 0x0f;      // must match encode()
			u8 const X = u8((col << 4) - prog.read_byte(0x1021) - 0x12);
			u8 const Y = u8((row << 4) - prog.read_byte(0x1022) - 0x0c);
			return u8(ship_x - X) >= u8(0xe6 - DRIFT) && u8(ship_y - Y) <= u8(0x1a + DRIFT);
		};
		// The reported position is the contact pixel of each object (xreg - 16 undoes the S2636 draw
		// offset, so px + 16 is back in register space).  px/py are set by overlaps() on a hit and
		// read immediately, before the next overlaps() call overwrites them.
		int px = 0, py = 0;
		bool const ship_hit = overlaps(s2636_0_bitmap, *m_s2636[0], 0, ship_x, ship_y, px, py);
		u8 const ship_ex = u8(px + 16), ship_ey = u8(py);
		bool const miss_hit = overlaps(s2636_1_bitmap, *m_s2636[1], 0, miss_x, miss_y, px, py);
		u8 const miss_ex = u8(px + 16), miss_ey = u8(py);
		bool const bomb_hit = overlaps(s2636_0_bitmap, *m_s2636[0], 3, bomb_x, bomb_y, px, py);
		u8 const bomb_ex = u8(px + 16), bomb_ey = u8(py);

		if (ship_hit)
			m_collision = encode(ship_ex, ship_ey);   // ship -> crash (the game destroys a structure it hits)
		else if (miss_hit && !would_crash_ship(miss_ex, miss_ey))
			m_collision = encode(miss_ex, miss_ey);   // missile -> target, never the ship
		else if (bomb_hit && !would_crash_ship(bomb_ex, bomb_ey))
			m_collision = encode(bomb_ex, bomb_ey);   // bomb -> structure / ground, never the ship
		else
			m_collision = 0;

		// Per-frame decision trace (VERBOSE LOG_COLL): raw and reported coords, guards, verdict.
		if (ship_hit || miss_hit || bomb_hit)
			LOGMASKED(LOG_COLL, "COL scx=%02x scy=%02x | ship %d %02x/%02x->%02x/%02x | miss %d %02x/%02x->%02x/%02x wcs=%d | bomb %d %02x/%02x->%02x/%02x wcs=%d | -> %02x\n",
				u8(m_scrollx), scrolly,
				ship_hit, ship_x, ship_y, ship_ex, ship_ey,
				miss_hit, miss_x, miss_y, miss_ex, miss_ey, miss_hit ? would_crash_ship(miss_ex, miss_ey) : 0,
				bomb_hit, bomb_x, bomb_y, bomb_ex, bomb_ey, bomb_hit ? would_crash_ship(bomb_ex, bomb_ey) : 0,
				m_collision);
	}
}

/*
 * System
 */

uint8_t malzak_state::fake_VRLE_r()
{
	return (m_s2636[0]->read_data(0xcb) & 0x3f) + (m_screen->vblank() ? 0x40 : 0x00);
}

uint8_t malzak_state::s2636_portA_r()
{
	// POT switch position, read from port A of the first S2636
	// Not sure of the correct values to return, but these should
	// do based on the game code.
	switch (ioport("POT")->read())
	{
		case 0:  // Normal play
			return 0xf0;
		case 1:
			return 0x90;
		case 2:
			return 0x70;
		case 3:  // Change settings
			return 0x00;
		default:
			return 0xf0;
	}
}

void malzak_state::malzak_map(address_map &map)
{
	map.global_mask(0x7fff);
	map(0x0000, 0x0bff).rom();
	map(0x0c00, 0x0fff).bankr("mainbank");
	map(0x1000, 0x10ff).mirror(0x6000).ram();
	map(0x1100, 0x11ff).mirror(0x6000).ram();
	map(0x1200, 0x12ff).mirror(0x6000).ram();
	map(0x1300, 0x13ff).mirror(0x6000).ram();
	map(0x1400, 0x14ff).mirror(0x6000).rw(m_s2636[0], FUNC(s2636_device::read_data), FUNC(s2636_device::write_data));
	map(0x14cb, 0x14cb).mirror(0x6000).r(FUNC(malzak_state::fake_VRLE_r));
	map(0x1500, 0x15ff).mirror(0x6000).rw(m_s2636[1], FUNC(s2636_device::read_data), FUNC(s2636_device::write_data));
	map(0x1600, 0x16ff).mirror(0x6000).ram().w(FUNC(malzak_state::playfield_w));
	map(0x1700, 0x17ff).mirror(0x6000).ram();
	map(0x1800, 0x1fff).mirror(0x6000).ram().share("videoram");
	map(0x2000, 0x2fff).rom();
	map(0x4000, 0x4fff).rom();
	map(0x6000, 0x6fff).rom();
}

void malzak_state::malzak2_map(address_map &map)
{
	map.global_mask(0x7fff);
	map(0x0000, 0x0bff).rom();
	map(0x0c00, 0x0fff).bankr("mainbank");
	map(0x1000, 0x10ff).mirror(0x6000).ram();
	map(0x1100, 0x11ff).mirror(0x6000).ram();
	map(0x1200, 0x12ff).mirror(0x6000).ram();
	map(0x1300, 0x13ff).mirror(0x6000).ram();
	map(0x1400, 0x14ff).mirror(0x6000).rw(m_s2636[0], FUNC(s2636_device::read_data), FUNC(s2636_device::write_data));
	map(0x14cb, 0x14cb).mirror(0x6000).r(FUNC(malzak_state::fake_VRLE_r));
	map(0x14cc, 0x14cc).mirror(0x6000).r(FUNC(malzak_state::s2636_portA_r));
	map(0x1500, 0x15ff).mirror(0x6000).rw(m_s2636[1], FUNC(s2636_device::read_data), FUNC(s2636_device::write_data));
	map(0x1600, 0x16ff).mirror(0x6000).ram().w(FUNC(malzak_state::playfield_w));
	map(0x1700, 0x17ff).mirror(0x6000).ram().share("nvram");
	map(0x1800, 0x1fff).mirror(0x6000).ram().share("videoram");
	map(0x2000, 0x2fff).rom();
	map(0x4000, 0x4fff).rom();
	map(0x6000, 0x6fff).rom();
}


uint8_t malzak_state::s2650_data_r()
{
//  popmessage("S2650 data port read");
	return 0xff;
}

void malzak_state::port40_w(uint8_t data)
{
//  Bit 0 is constantly set high during gameplay
//  Bit 4 is set high, then low, upon death
//  Bit 1 is set high on boot, and on the title screens.
//  Bits 1-3 are all set high upon death, until the game continues
//  Bit 6 is used only in Malzak II, and is set high after checking
//        the selected version
//  logerror("%s S2650: port 0x40 write: 0x%02x\n", machine().describe_context(), data);
	m_mainbank->set_entry((data & 0x40) >> 6);
	// bit 7 is set at final stage
	u8 gfx_bank = ((data & 0x80) >> 7);
	if (m_playfield_bank != gfx_bank)
	{
		m_playfield_bank = gfx_bank;
		m_playfield_tilemap->mark_all_dirty();
	}
}

uint8_t malzak_state::collision_r()
{
	// ship <-> playfield collision, latched once per frame at VBLANK (see screen_vblank()).
	// yyyy xxxx = (row << 4) | col; 0 = no collision.
	return m_collision;
}

void malzak_state::malzak_io_map(address_map &map)
{
	map(0x00, 0x00).r(FUNC(malzak_state::collision_r)); // returns where a collision can occur.
	map(0x40, 0x40).w(FUNC(malzak_state::port40_w));  // possibly sound codes for dual SN76477s
	map(0x60, 0x60).w(FUNC(malzak_state::port60_w));  // possibly playfield scroll X offset
	map(0x80, 0x80).portr("IN0");  //controls
	map(0xa0, 0xa0).nopw();  // echoes I/O port read from port 0x80
	map(0xc0, 0xc0).w(FUNC(malzak_state::portc0_w));  // possibly playfield row selection for writing and/or collisions
}

void malzak_state::malzak_data_map(address_map &map)
{
	map(S2650_DATA_PORT, S2650_DATA_PORT).r(FUNC(malzak_state::s2650_data_r));  // read upon death
}


static INPUT_PORTS_START( malzak )

	/* Malzak has an 8-way stick
	   and only one button (firing and bomb dropping on the same button) */

	PORT_START("IN0")       /* I/O port 0x80 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT  )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT  )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP    )

	PORT_START("POT")
	/* No POT switch on Malzak as far as I know */

INPUT_PORTS_END

static INPUT_PORTS_START( malzak2 )

	/* Same as Malzak, but with additional POT switch, and
	   possibly a reset button too. */

	PORT_START("IN0")       /* I/O port 0x80 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT  )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT  )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP    )

	PORT_START("POT")       /* Fake DIP switch to handle the POT service switch */
	PORT_DIPNAME( 0x03, 0x00, "POT switch position" )
	PORT_DIPSETTING( 0x00, "1" )  // Normal play
	PORT_DIPSETTING( 0x01, "2" )  // Clear high-score table
	PORT_DIPSETTING( 0x02, "3" )  // Coin / bookkeeping audit
	PORT_DIPSETTING( 0x03, "4" )  // Settings / test menu

INPUT_PORTS_END


static const gfx_layout charlayout =
{
	16,16,
	RGN_FRAC(1,1),
	1,
	{ 0 },
	{ 7, 6, 5, 4, 3, 2, 1, 0, 15, 14, 13, 12, 11, 10, 9, 8 },
	{ 0*16, 1*16, 2*16, 3*16, 4*16, 5*16, 6*16, 7*16,
		8*16, 9*16, 10*16, 11*16, 12*16, 13*16, 14*16, 15*16 },
//  { 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	16*16
//  8*8
};


static GFXDECODE_START( gfx_malzak )
	GFXDECODE_ENTRY( "gfx1", 0x0000, charlayout, 0,  16*16 )
GFXDECODE_END

void malzak_state::machine_start()
{
	m_mainbank->configure_entries(0, 2, memregion("user2")->base(), 0x400);

	save_item(NAME(m_playfield_code));
	save_item(NAME(m_scrollx));
	save_item(NAME(m_scrolly));
	save_item(NAME(m_collision));
}

void malzak_state::machine_reset()
{
	std::fill(std::begin(m_playfield_code), std::end(m_playfield_code), 0);
}

void malzak_state::malzak(machine_config &config)
{
	/* basic machine hardware */
	S2650(config, m_maincpu, 3800000/4); // ???
	m_maincpu->set_addrmap(AS_PROGRAM, &malzak_state::malzak_map);
	m_maincpu->set_addrmap(AS_IO, &malzak_state::malzak_io_map);
	m_maincpu->set_addrmap(AS_DATA, &malzak_state::malzak_data_map);
	m_maincpu->sense_handler().set(m_screen, FUNC(screen_device::vblank));

	/* video hardware */
	SCREEN(config, m_screen);
	// TODO: handtuned, unverified
	m_screen->set_raw(6'000'000 * 4, 768, 0, 80 * 6, 312 * 2, 0, 240 * 2);
	m_screen->set_screen_update(FUNC(malzak_state::screen_update));
	m_screen->screen_vblank().set(FUNC(malzak_state::screen_vblank));

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_malzak);
	PALETTE(config, m_palette, FUNC(malzak_state::palette_init), 128);

	S2636(config, m_s2636[0]);
	m_s2636[0]->set_offsets(0, -16);  // -8, -16
	m_s2636[0]->add_route(ALL_OUTPUTS, "mono", 0.25);

	S2636(config, m_s2636[1]);
	m_s2636[1]->set_offsets(0, -16);  // -9, -16
	m_s2636[1]->add_route(ALL_OUTPUTS, "mono", 0.25);

	SAA5050(config, m_trom, 6'000'000);
	m_trom->d_cb().set(FUNC(malzak_state::videoram_r));
	m_trom->set_screen_size(42, 24, 64);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	sn76477_device &sn1(SN76477(config, "sn1"));
	sn1.set_noise_params(0, 0, 0);
	sn1.set_decay_res(0);
	sn1.set_attack_params(0, RES_K(100));
	sn1.set_amp_res(RES_K(56));
	sn1.set_feedback_res(RES_K(10));
	sn1.set_vco_params(0, CAP_U(0.1), RES_K(8.2));
	sn1.set_pitch_voltage(5.0);
	sn1.set_slf_params(CAP_U(1.0), RES_K(120));
	sn1.set_oneshot_params(0, 0);
	sn1.set_vco_mode(0);
	sn1.set_mixer_params(1, 1, 1);
	sn1.set_envelope_params(1, 1);
	sn1.set_enable(1);
	sn1.add_route(ALL_OUTPUTS, "mono", 0.25);

	sn76477_device &sn2(SN76477(config, "sn2"));
	sn2.set_noise_params(0, 0, 0);
	sn2.set_decay_res(0);
	sn2.set_attack_params(0, RES_K(100));
	sn2.set_amp_res(RES_K(56));
	sn2.set_feedback_res(RES_K(10));
	sn2.set_vco_params(0, CAP_U(0.1), RES_K(8.2));
	sn2.set_pitch_voltage(5.0);
	sn2.set_slf_params(CAP_U(1.0), RES_K(120));
	sn2.set_oneshot_params(0, 0);
	sn2.set_vco_mode(0);
	sn2.set_mixer_params(1, 1, 1);
	sn2.set_envelope_params(1, 1);
	sn2.set_enable(1);
	sn2.add_route(ALL_OUTPUTS, "mono", 0.25);
}

void malzak_state::malzak2(machine_config &config)
{
	malzak(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &malzak_state::malzak2_map);

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);
}

ROM_START( malzak )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "malzak.5",     0x0000, 0x0800, CRC(75355c98) SHA1(7036ed5d9ee38585b1a6bc204d410d5fb5ddd81f) )
	ROM_CONTINUE( 0x2000, 0x0800 )
	ROM_LOAD( "malzak.4",     0x0800, 0x0400, CRC(744c81e3) SHA1(c08d6df3cf2808a5f99d8247fc19a59be88121a9) )
	ROM_CONTINUE( 0x4000, 0x0c00 )
	// Terrain data
	ROM_LOAD( "malzak.3",     0x4400, 0x0800, CRC(b947229e) SHA1(37b88b5aa91a483fcfe60a9bdd67a66f6378c487) )

	// Screen data
	ROM_REGION(0x0800, "user2", 0)
	ROM_LOAD( "malzak.2",     0x0000, 0x0800, CRC(2a12ad67) SHA1(f89a50b62311a170004c061abd8dedc3ebd84748) )

	ROM_REGION( 0x0800, "gfx1", 0 )
	ROM_LOAD( "malzak.1",     0x0000, 0x0800, CRC(74d5ff7b) SHA1(cae326370dc83b86542f9d070e2dc91b1b833356) )
ROM_END

ROM_START( malzak2 )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "malz1a.bin",   0x000000, 0x000800, CRC(5c3cb14c) SHA1(2d3b5703cb9a47e34aa593f0e8d42d4e67c167d9) )
	ROM_CONTINUE( 0x2000, 0x0800 )

	ROM_LOAD( "malz2b.bin",     0x0800, 0x0400, CRC(2af8aace) SHA1(7aaf03d4848c2cce72b2b3729661e7826834ad44) )
	ROM_CONTINUE( 0x4000, 0x0400 )
	ROM_CONTINUE( 0x2800, 0x0400 )
	ROM_CONTINUE( 0x6000, 0x0400 )
	// Terrain data
	ROM_LOAD( "malz3c.bin",     0x4400, 0x0800, CRC(54d6a02e) SHA1(80c550d74da770689fe451cb0ee8e550a63b1b96) )

	// Screen data
	ROM_REGION(0x0800, "user2", 0)
	ROM_LOAD( "malz4d.bin",     0x0000, 0x0800, CRC(5c6ca415) SHA1(e7571519ac7911507d2c1cf975a7663f41321cb9) )

	ROM_REGION( 0x0800, "gfx1", 0 )
	ROM_LOAD( "malzak.1",     0x0000, 0x0800, CRC(74d5ff7b) SHA1(cae326370dc83b86542f9d070e2dc91b1b833356) )

	ROM_REGION( 0x0100, "nvram", 0 )
	// default nvram so that game boots in version II off the bat
	ROM_LOAD( "malzak2.nv",    0x0000, 0x0100, CRC(aadf03d8) SHA1(9d751c4249faab7f5d88d0c99f33468f439641ad) )
ROM_END

} // anonymous namespace


GAME( 19??, malzak,  0,      malzak,  malzak,  malzak_state, empty_init, ROT0, "Kitronix", "Malzak",    MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_COLORS | MACHINE_SUPPORTS_SAVE )
GAME( 19??, malzak2, malzak, malzak2, malzak2, malzak_state, empty_init, ROT0, "Kitronix", "Malzak II", MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_COLORS | MACHINE_SUPPORTS_SAVE )
