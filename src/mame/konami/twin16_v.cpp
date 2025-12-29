// license:BSD-3-Clause
// copyright-holders:Phil Stroffolino
/*

    Konami Twin16 Video subsystem

    TODO:

    - clean up sprite system
    - bad sprites in devilw, eg. odd colours for the mud/lava monster in the 1st level,
      or wrong sprite-sprite priority sometimes -- check real arcade first
    - unsure about some sprite preprocessor attributes (see spriteram_process)

*/

#include "emu.h"
#include "twin16_v.h"

#include <algorithm>

#define LOG_UNKNOWN (1 << 1)

#define VERBOSE (LOG_UNKNOWN)

#include "logmacro.h"


enum
{
	TWIN16_SCREEN_FLIPY = 0x01,
	TWIN16_SCREEN_FLIPX = 0x02, // confirmed: Hard Puncher Intro
	TWIN16_PRI0         = 0x04, // PRI0 input into 007789 PAL
	TWIN16_PRI1         = 0x08, // PRI1 input into 007789 PAL
	TWIN16_PRI2_UNUSED  = 0x10, // schematic shows as PRI2 input, but unused
	TWIN16_TILE_FLIPY   = 0x20  // confirmed: Vulcan Venture
};

enum
{
	// user-defined priorities
	TWIN16_BG_OVER_SPRITES = 0x01, // BG pixel has priority over opaque sprite pixels
	TWIN16_BG_NO_SHADOW    = 0x02, // BG pixel has priority over shadow sprite pixels
	TWIN16_SPRITE_OCCUPIED = 0x04
};

DEFINE_DEVICE_TYPE(KONAMI_TWIN16_VIDEO, konami_twin16_video_device, "konami_twin16_video", "Konami Twin16 Video Subsystem")

konami_twin16_video_device::konami_twin16_video_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, KONAMI_TWIN16_VIDEO, tag, owner, clock)
	, device_video_interface(mconfig, *this, true)
	, device_gfx_interface(mconfig, *this, nullptr)
	, m_fixram(*this, "fixram", 0x4000U, ENDIANNESS_BIG)
	, m_videoram(*this, "videoram_%u", 0U, 0x2000U, ENDIANNESS_BIG)
	, m_spriteram{*this, "spriteram_%u", 0U, 0x4000U, ENDIANNESS_BIG}
	, m_sprite_buffer(nullptr)
	, m_sprite_timer(nullptr)
	, m_sprite_process_enable(1)
	, m_sprite_busy(0)
	, m_need_process_spriteram(0)
	, m_scrollx{0}
	, m_scrolly{0}
	, m_video_register(0)
	, m_fixed_tmap(nullptr)
	, m_scroll_tmap{nullptr, nullptr}
	, m_virq_cb(*this)
	, m_sprite_cb(*this)
	, m_tile_cb(*this, DEVICE_SELF, FUNC(konami_twin16_video_device::default_tile))
{
}

void konami_twin16_video_device::device_start()
{
	if (!palette().device().started())
		throw device_missing_dependencies();

	if (!palette().shadows_enabled())
		fatalerror("%s: palette shadows must be enabled!", machine().describe_context());

	m_sprite_cb.resolve_safe(0);
	m_tile_cb.resolve_safe(0);

	const size_t spritebuffer_size = 0x1000 / 2;
	m_sprite_buffer = std::make_unique<uint16_t []>(spritebuffer_size);

	m_fixed_tmap = &machine().tilemap().create(*this, tilemap_get_info_delegate(*this, FUNC(konami_twin16_video_device::fix_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 64, 32);
	m_scroll_tmap[0] = &machine().tilemap().create(*this, tilemap_get_info_delegate(*this, FUNC(konami_twin16_video_device::scroll_tile_info<0>)), TILEMAP_SCAN_ROWS, 8, 8, 64, 64);
	m_scroll_tmap[1] = &machine().tilemap().create(*this, tilemap_get_info_delegate(*this, FUNC(konami_twin16_video_device::scroll_tile_info<1>)), TILEMAP_SCAN_ROWS, 8, 8, 64, 64);

	m_fixed_tmap->set_transparent_pen(0);
	m_scroll_tmap[0]->set_transparent_pen(0);
	m_scroll_tmap[1]->set_transparent_pen(0);

	std::fill_n(&m_sprite_buffer[0], 0x800, uint16_t(~0));
	m_sprite_timer = timer_alloc(FUNC(konami_twin16_video_device::sprite_tick), this);
	m_sprite_timer->adjust(attotime::never);

	save_pointer(NAME(m_sprite_buffer), spritebuffer_size);
	save_item(NAME(m_sprite_busy));
	save_item(NAME(m_need_process_spriteram));
	save_item(NAME(m_scrollx));
	save_item(NAME(m_scrolly));
	save_item(NAME(m_video_register));
}

void konami_twin16_video_device::device_reset()
{
	m_sprite_process_enable = 1;
	m_sprite_timer->adjust(attotime::never);
	m_sprite_busy = 0;
}

void konami_twin16_video_device::sprite_process_enable_w(uint8_t data)
{
	const uint8_t old = m_sprite_process_enable;
	m_sprite_process_enable = data;
	if ((old == 0) && (m_sprite_process_enable != 0))
		spriteram_process();
}

void konami_twin16_video_device::spriteram_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_spriteram[0][offset]);
}

void konami_twin16_video_device::fixram_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_fixram[offset]);
	m_fixed_tmap->mark_tile_dirty(offset);
}

void konami_twin16_video_device::video_register_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	switch (offset)
	{
		case 0:
		{
			const int old = m_video_register;
			COMBINE_DATA(&m_video_register);
			const int changed = old ^ m_video_register;
			if (changed & (TWIN16_SCREEN_FLIPX | TWIN16_SCREEN_FLIPY))
			{
				int flip = (m_video_register & TWIN16_SCREEN_FLIPX) ? TILEMAP_FLIPX : 0;
				flip |= (m_video_register & TWIN16_SCREEN_FLIPY) ? TILEMAP_FLIPY : 0;
				machine().tilemap().set_flip_all(flip);
			}
			if (changed & TWIN16_TILE_FLIPY)
			{
				m_scroll_tmap[0]->mark_all_dirty();
				m_scroll_tmap[1]->mark_all_dirty();
			}
			break;
		}

		case 1: COMBINE_DATA(&m_scrollx[0]); break;
		case 2: COMBINE_DATA(&m_scrolly[0]); break;
		case 3:
			COMBINE_DATA(&m_scrollx[1]);
			m_scroll_tmap[0]->set_scrollx(0, m_scrollx[1]);
			break;
		case 4:
			COMBINE_DATA(&m_scrolly[1]);
			m_scroll_tmap[0]->set_scrolly(0, m_scrolly[1]);
			break;
		case 5:
			COMBINE_DATA(&m_scrollx[2]);
			m_scroll_tmap[1]->set_scrollx(0, m_scrollx[2]);
			break;
		case 6:
			COMBINE_DATA(&m_scrolly[2]);
			m_scroll_tmap[1]->set_scrolly(0, m_scrolly[2]);
			break;

		default:
			LOGMASKED(LOG_UNKNOWN, "%s: unknown video_register write %02x: %04x & %04x", machine().describe_context(), offset, data, mem_mask);
			break;
	}
}

/*
 * Sprite Format
 * ----------------------------------
 * preprocessor (not much data to test with):
 * Word | Bit(s)           | Use
 * -----+-fedcba9876543210-+----------------
 *   0  | x--------------- | enable
 *   0  | -xxxxxxx-------- | ?
 *   0  | --------xxxxxxxx | sprite-sprite priority
 * -----+------------------+
 *   1  | xxxxxxxxxxxxxxxx | ?
 * -----+------------------+
 *   2  | xxxxxx---------- | ?
 *   2  | ------x--------- | yflip (devilw)
 *   2  | -------x-------- | xflip
 *   2  | --------xx------ | height
 *   2  | ----------xx---- | width
 *   2  | ------------xxxx | color
 * -----+------------------+
 *   3  | -xxxxxxxxxxxxxxx | code
 * -----+------------------+
 *   4  | -------xxxxxxxxx | xpos high, other bits probably no effect
 *   5  | xxxxxxxx-------- | xpos low, other bits probably no effect
 *   6  | -------xxxxxxxxx | xpos high, other bits probably no effect
 *   7  | xxxxxxxx-------- | ypos low, other bits probably no effect
 *
 * ----------------------------------
 * normal/after preprocessing:
 * Word | Bit(s)           | Use
 * -----+-fedcba9876543210-+----------------
 *   0  | -xxxxxxxxxxxxxxx | code
 * -----+------------------+
 *   1  | -------xxxxxxxxx | ypos
 * -----+------------------+
 *   2  | -------xxxxxxxxx | xpos
 * -----+------------------+
 *   3  | x--------------- | enable
 *   3  | -xxxxx---------- | ?
 *   3  | ------x--------- | yflip  ?
 *   3  | -------x-------- | xflip
 *   3  | --------xx------ | height
 *   3  | ----------xx---- | width
 *   3  | ------------xxxx | color
 */

uint16_t konami_twin16_video_device::sprite_status_r()
{
	// bit 0: busy, other bits: dunno
	return m_sprite_busy;
}

TIMER_CALLBACK_MEMBER(konami_twin16_video_device::sprite_tick)
{
	m_sprite_busy = 0;
}

int konami_twin16_video_device::set_sprite_timer()
{
	if (m_sprite_busy) return 1;

	// sprite system busy, maybe a dma? time is guessed, assume 4 scanlines
	m_sprite_busy = 1;
	m_sprite_timer->adjust(screen().frame_period() / screen().height() * 4);

	return 0;
}

void konami_twin16_video_device::spriteram_process()
{
	const uint16_t dx = m_scrollx[0];
	const uint16_t dy = m_scrolly[0];

	const uint16_t *source = &m_spriteram[0][0x0000];
	const uint16_t *finish = &m_spriteram[0][0x1800];

	set_sprite_timer();
	std::fill_n(&m_spriteram[0][0x1800], 0x800, uint16_t(~0));

	while (source < finish)
	{
		const uint16_t priority = source[0];
		if (BIT(priority, 15))
		{
			uint16_t *dest = &m_spriteram[0][0x1800 | ((priority & 0xff) << 2)];

			const uint32_t xpos = (uint32_t(source[4]) << 16) | source[5];
			const uint32_t ypos = (uint32_t(source[6]) << 16) | source[7];

			/* notes on sprite attributes:

			The only inputs from the sprite hardware into the mixer PAL are four bits of
			pixel data, with 0000 being transparent, 1111 being shadow, and anything else
			opaque. Sprite to background priority, and whether shadows are visible, depends
			entirely on the priority mode bits in m_video_register and on the underlying
			background pixel, and not on any of the sprite attribute bits.

			Shadows in the devilw lava stages look a bit strange; the shadows "punch holes"
			in the platforms and reveal the lava underneath. As far as I can tell from the
			schematics this has to be correct; unlike later Konami hardware there seems to
			be no way for a sprite to cast a shadow onto another sprite.

			fround, hpuncher, miaj, cuebrickj, don't use the preprocessor.
			*/
			const uint16_t attributes = 0x8000 | (source[2] & 0x03ff); // scale,size,color

			dest[0] = source[3]; /* gfx data */
			dest[1] = ((xpos >> 8) - dx) & 0xffff;
			dest[2] = ((ypos >> 8) - dy) & 0xffff;
			dest[3] = attributes;
		}
		source += 0x50/2;
	}
	m_need_process_spriteram = 0;
}

void konami_twin16_video_device::draw_sprites(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	const uint16_t *source = &m_spriteram[1][0x2000 - 4];
	const uint16_t *finish = &m_spriteram[1][0x1800];

	for (; source >= finish; source -= 4)
	{
		const uint16_t attributes = source[3];
		uint16_t code = source[0];

		if ((code != 0xffff) && BIT(attributes, 15))
		{
			int xpos = source[1];
			int ypos = source[2];

			const int pal_base = 0x100 + ((attributes & 0xf) << 4);
			const int height = 16 << ((attributes >> 6) & 0x3);
			const int width = 16 << ((attributes >> 4) & 0x3);
			bool flipy = BIT(attributes, 9);
			bool flipx = BIT(attributes, 8);

			/* some code masking */
			if ((height & width) == 64) code &= ~8;      // gradius2 ending sequence 64*64
			else if ((height & width) == 32) code &= ~3; // devilw 32*32
			else if ((height | width) == 48) code &= ~1; // devilw 32*16 / 16*32

			uint32_t pen_addr = code << 6;

			if (m_video_register & TWIN16_SCREEN_FLIPY)
			{
				if (ypos > 65000) ypos = ypos - 65536; /* Bit hacky */
				ypos = 256 - ypos - height;
				flipy = !flipy;
			}
			if (m_video_register & TWIN16_SCREEN_FLIPX)
			{
				if (xpos > 65000) xpos = xpos - 65536; /* Bit hacky */
				xpos = 320 - xpos - width;
				flipx = !flipx;
			}
			if (xpos >= 320) xpos -= 65536;
			if (ypos >= 256) ypos -= 65536;

			const int sx_start = flipx ? (xpos + width - 1) : xpos;
			const int sy_start = flipy ? (ypos + height - 1) : ypos;
			const int xinc = flipx ? -1 : 1;
			const int yinc = flipy ? -1 : 1;
			const int pitch = (width >> 2);

			/* slow slow slow, but it's ok for now */
			int sy = sy_start;
			for (int y = 0; y < height; y++, sy += yinc, pen_addr += pitch)
			{
				if (sy >= cliprect.min_y && sy <= cliprect.max_y)
				{
					uint16_t *const dest = &bitmap.pix(sy);
					uint8_t *const pdest = &screen.priority().pix(sy);

					int sx = sx_start;
					uint32_t pen_addr_x = pen_addr;
					for (int x = 0; x < width; x += 4, pen_addr_x++)
					{
						uint16_t pen = m_sprite_cb(pen_addr_x);
						for (int ix = 0; ix < 4; ix++, sx += xinc, pen <<= 4)
						{
							if (sx >= cliprect.min_x && sx <= cliprect.max_x)
							{
								const uint16_t pixel = (pen >> 12) & 0xf;

								if (pixel && !(pdest[sx] & TWIN16_SPRITE_OCCUPIED))
								{
									pdest[sx] |= TWIN16_SPRITE_OCCUPIED;

									if (pixel == 0xf) // shadow
									{
										if (!(pdest[sx] & TWIN16_BG_NO_SHADOW))
											dest[sx] = palette().shadow_table()[dest[sx]];
									}
									else // opaque pixel
									{
										if (!(pdest[sx] & TWIN16_BG_OVER_SPRITES))
											dest[sx] = pal_base + pixel;
									}
								}
							}
						}
					}
				}
			}
		}
	}
}


TILE_GET_INFO_MEMBER(konami_twin16_video_device::fix_tile_info)
{
	const uint16_t attr = m_fixram[tile_index];
	/* fedcba9876543210
	   -x-------------- yflip
	   --x------------- xflip
	   ---xxxx--------- color
	   -------xxxxxxxxx tile number
	*/
	const uint32_t code = attr & 0x1ff;
	const uint32_t color = (attr >> 9) & 0x0f;
	uint8_t flags = 0;

	if (BIT(attr, 13)) flags |= TILE_FLIPX;
	if (BIT(attr, 14)) flags |= TILE_FLIPY;

	tileinfo.set(0, code, color, flags);
}

template <unsigned Which>
TILE_GET_INFO_MEMBER(konami_twin16_video_device::scroll_tile_info)
{
	/* fedcba9876543210
	   xxx------------- color; high bit is also priority over sprites
	   ---xxxxxxxxxxxxx tile number
	*/
	const uint16_t data = m_videoram[Which][tile_index];
	const uint32_t code = m_tile_cb(data & 0x1fff);
	const uint32_t color = (Which << 3) + (data >> 13);
	uint8_t flags = 0;
	if (m_video_register & TWIN16_TILE_FLIPY)
		flags |= TILE_FLIPY;

	tileinfo.set(1, code, color, flags);
	tileinfo.category = BIT(data, 15);
}

uint32_t konami_twin16_video_device::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
/*
    PAL equations (007789 @ 11J):

    /SHAD = /FIX * /PRI1 * OCO0 * OCO1 * OCO2 * OCO3
          + /FIX * /PRI0 * OCO0 * OCO1 * OCO2 * OCO3 * /V1C0
          + /FIX * PRI0 * OCO0 * OCO1 * OCO2 * OCO3 * /V2C6
          + /FIX * PRI0 * /V2C0 * /V2C3 * OCO0 * /V2C2 * OCO1 * /V2C1 * OCO2 * OCO3

    /SELB = /FIX * OCO0 * OCO1 * OCO2 * OCO3
          + /FIX * /OCO0 * /OCO1 * /OCO2 * /OCO3
          + /FIX * PRI0 * /PRI1 * V1C0
          + /FIX * PRI0 * PRI1 * V2C0 * V2C6
          + /FIX * PRI0 * PRI1 * V2C1 * V2C6
          + /FIX * PRI0 * PRI1 * V2C2 * V2C6
          + /FIX * PRI0 * PRI1 * V2C3 * V2C6

     SELA = FIX
          + PRI0 * /PRI1 * V1C0
          + /PRI1 * OCO0 * OCO1 * OCO2 * OCO3 * V1C0
          + /PRI1 * /OCO0 * /OCO1 * /OCO2 * /OCO3 * V1C0
          + PRI1 * /V2C0 * /V2C3 * OCO0 * /V2C2 * OCO1 * /V2C1 * OCO2 * OCO3
          + PRI1 * /V2C0 * /V2C3 * /OCO0 * /V2C2 * /OCO1 * /V2C1 * /OCO2 * /OCO3

     SELB  SELA  Visible layer
      0     0    VRAM2
      0     1    VRAM1
      1     0    Object
      1     1    Fix

    Final Round uses a PROM (not dumped) instead of the PAL
    and some discrete logic to combine some of the inputs.
    Inputs to the PROM are:

    A0 = V1C0 | V1C1 | V1C2 | V1C3
    A1 = V2C0 | V2C1 | V2C2 | V2C3
    A2 = OCO0 | OCO1 | OCO2 | OCO3
    A3 = FIX0 | FIX1 | FIX2 | FIX3
    A4 = ~(OCD0 & OCD1 & OCD2 & OCD3)
    A5 = V2C6
    A6 = PRI0
    A7 = PRI1
*/
	screen.priority().fill(0, cliprect);

	switch ((m_video_register >> 2) & 0x3)
	{
		case 0: // PRI1 = 0, PRI0 = 0
			m_scroll_tmap[1]->draw(screen, bitmap, cliprect, TILEMAP_DRAW_ALL_CATEGORIES | TILEMAP_DRAW_OPAQUE);
			m_scroll_tmap[0]->draw(screen, bitmap, cliprect, TILEMAP_DRAW_ALL_CATEGORIES);
			break;
		case 1: // PRI1 = 0, PRI0 = 1
			m_scroll_tmap[1]->draw(screen, bitmap, cliprect, TILEMAP_DRAW_ALL_CATEGORIES | TILEMAP_DRAW_OPAQUE);
			m_scroll_tmap[0]->draw(screen, bitmap, cliprect, TILEMAP_DRAW_ALL_CATEGORIES, TWIN16_BG_OVER_SPRITES);
			break;
		case 2: // PRI1 = 1, PRI0 = 0
			m_scroll_tmap[0]->draw(screen, bitmap, cliprect, TILEMAP_DRAW_ALL_CATEGORIES | TILEMAP_DRAW_OPAQUE);
			m_scroll_tmap[0]->draw(screen, bitmap, cliprect, TILEMAP_DRAW_ALL_CATEGORIES, TWIN16_BG_NO_SHADOW);
			m_scroll_tmap[1]->draw(screen, bitmap, cliprect, TILEMAP_DRAW_ALL_CATEGORIES);
			break;
		case 3: // PRI1 = 1, PRI0 = 1
			m_scroll_tmap[0]->draw(screen, bitmap, cliprect, TILEMAP_DRAW_ALL_CATEGORIES | TILEMAP_DRAW_OPAQUE);
			m_scroll_tmap[1]->draw(screen, bitmap, cliprect, 0);
			m_scroll_tmap[1]->draw(screen, bitmap, cliprect, 1, TWIN16_BG_OVER_SPRITES | TWIN16_BG_NO_SHADOW);
			break;
	}

	draw_sprites(screen, bitmap, cliprect);

	m_fixed_tmap->draw(screen, bitmap, cliprect, 0);
	return 0;
}

void konami_twin16_video_device::screen_vblank(int state)
{
	// rising edge
	if (state)
	{
		set_sprite_timer();

		if (m_sprite_process_enable)
		{
			if (m_need_process_spriteram)
				spriteram_process();
			m_need_process_spriteram = 1;

			/* if the sprite preprocessor is used, sprite ram is copied to an external buffer first,
			as evidenced by 1-frame sprite lag in gradius2 and devilw otherwise, though there's probably
			more to it than that */
			std::copy_n(&m_sprite_buffer[0], 0x800, &m_spriteram[1][0x1800]);
			std::copy_n(&m_spriteram[0][0x1800], 0x800, &m_sprite_buffer[0]);
		}
		else
		{
			std::copy_n(&m_spriteram[0][0], 0x2000, &m_spriteram[1][0]);
		}

		// IRQ generation
		m_virq_cb(ASSERT_LINE);
	}
}
