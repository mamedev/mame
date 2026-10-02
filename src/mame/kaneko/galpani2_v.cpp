// license:BSD-3-Clause
// copyright-holders:Luca Elia
/***************************************************************************

                            -= Gal's Panic II =-

                    driver by   Luca Elia (l.elia@tin.it)


***************************************************************************/

#include "emu.h"
#include "galpani2.h"

#include "input.h" // for video debug keys
#include "screen.h"

/*
304000:0040 0000 0100 0000-0000 0000 0000 0000      (Sprites regs)
304010:16C0 0200 16C0 0200-16C0 0200 16C0 0200
*/

/***************************************************************************


                        Palettized Background Layers


***************************************************************************/


#ifdef UNUSED_DEFINITION
inline uint16_t galpani2_state::galpani2_bg8_regs_r(offs_t offset, int n)
{
	switch (offset * 2)
	{
		case 0x16:  return machine().rand() & 1;
		default:
			logerror("%s: Warning, bg8 #%d screen reg %04X read\n",machine().describe_context(),_n_,offset*2);
	}
	return m_bg8_regs[_n_][offset];
}

/*
    000-3ff     row? scroll
    400         ?
    800-bff     col? scroll
    c04         0003 flip, 0300 flip?
    c1c/e       01ff scroll, 3000 ?
*/
inline void galpani2_state::galpani2_bg8_regs_w(offs_t offset, uint16_t data, uint16_t mem_mask, int _n_)
{
	COMBINE_DATA(&m_bg8_regs[_n_][offset]);
}

uint16_t galpani2_state::galpani2_bg8_regs_0_r(offs_t offset) { return galpani2_bg8_regs_r(offset, 0); }
uint16_t galpani2_state::galpani2_bg8_regs_1_r(offs_t offset) { return galpani2_bg8_regs_r(offset, 1); }

void galpani2_state::galpani2_bg8_regs_0_w(offs_t offset, uint16_t data, uint16_t mem_mask) { galpani2_bg8_regs_w(offset, data, mem_mask, 0); }
void galpani2_state::galpani2_bg8_regs_1_w(offs_t offset, uint16_t data, uint16_t mem_mask) { galpani2_bg8_regs_w(offset, data, mem_mask, 1); }
#endif



/***************************************************************************


                                Screen Drawing


***************************************************************************/

// Native player color records use F9/FD for captured field cells. Their
// flashing palette colors also set bit 15, so that bit alone cannot select
// the covered silhouette. UI words using the same palette remain opaque.
static bool bg8_captured_field(uint16_t pen)
{
	return !BIT(pen, 11) && ((pen & 0xff) == 0xf9 || (pen & 0xff) == 0xfd);
}

// TODO: verify captured-field tinting and the remaining mixer flags on PCB.

// scrolling is based on sync between sprite and this layer during the 'keyhole viewer' between rounds (use cheats)
void galpani2_state::copybg8(bitmap_rgb32 &bitmap, bitmap_ind8 &priority_bitmap, const rectangle &cliprect, int layer, bool foreground)
{
	// Plane 0's window/mask starts one pixel later in RAM than plane 1's field.
	int x = *m_bg8_scrollx[layer] + 0x42 + (layer == 0);
	int y = + ( *m_bg8_scrolly[layer] + 0x200 - 0x0f5 );
	uint16_t* ram = m_bg8[layer];

	pen_t const *const clut = &m_bg8palette->pen(0);
	for (int xx = cliprect.min_x; xx <= cliprect.max_x; xx++)
	{
		for (int yy = cliprect.min_y; yy <= cliprect.max_y; yy++)
		{
			uint16_t pen = ram[(((y + yy) & 0xff) * 512) + ((x + xx) & 0x1ff)];
			uint16_t const color = m_bg8_palette_ram[pen & 0xff];
			// Plane 0 supplies the foreground window and field mask.
			// Plane 1 has scenery (low palette bank) and field/UI (high bank).
			// SE uses 12xx/16xx for its border UI; English uses 28xx. Both need
			// foreground promotion so they draw above stale BG15 photo data.
			// Japanese scenery 0x0Cxx has bit 11+10 and must stay behind RGB.
			bool pixel_foreground;
			if (layer == 0)
			{
				// Plane 0 has field mask cells and window/border graphics.
				// Field mask cells must NOT draw over the photo:
				// - English/Asia: 00A0/80A0, palette=0x0001 (pass-through below)
				// - SE: 0050/8050/4050 (mask), 0060 (captured)
				// Window/border graphics (other pen values) must remain opaque.
				uint8_t const pen_lo = pen & 0xff;
				bool const is_mask_cell = (pen_lo == (m_mask_nongirl_pen & 0xff))
					|| (pen_lo == (m_mask_girl_pen & 0xff))
					|| (pen_lo == m_mask_captured_lo);
				pixel_foreground = !is_mask_cell;
			}
			else
			{
				// Plane 1: high palette bank or palette bit 15 → foreground.
				// Bit 11 without bit 10 → UI text (0x28xx, 0x12xx, 0x16xx).
				// SE uses 12xx (bit 12+11, no bit 10) and 16xx (bit 12+10+11);
				// 16xx has bit 10 set, so extend the check for SE's upper bytes.
				pixel_foreground = BIT(pen, 7) || BIT(color, 15)
					|| (BIT(pen, 11) && !BIT(pen, 10))
					|| (BIT(pen, 12) && BIT(pen, 9));
			}
			if (pixel_foreground != foreground)
				continue;
			// Palette colors 0x0000 and 0x0001 are transparent, letting the
			// BG15 photo show through. The game uses 0x0001 for revealed
			// field cells during gameplay, and 0x0000 (via pen 0x50E5) for
			// the full-photo reward display after completing a level.
			// Plane 0's low palette bank window retains its coverage when
			// the palette fades to 0x0001, or the other framebuffer half leaks.
			bool const passthrough = ((color <= 0x0001) && (layer || BIT(pen, 7)))
				|| (layer == 1 && bg8_captured_field(pen));
			// UI words remain opaque, including flagged black colors.
			bool const covered_field = BIT(color, 15) && !BIT(pen, 11);
			if ((layer || pen) && !passthrough && !covered_field)
			{
				bitmap.pix(yy, xx) = clut[pen & 0xff];
				priority_bitmap.pix(yy, xx) = foreground ? 3 : 0;
			}
		}
	}
}

// Four 512x256 framebuffers at 0x400000, with registers at 0x500000 + layer*0x40000.
// The native decoders advance 0x400 bytes per row and alternate 256-pixel halves.
// C04 low bits contain the plane order; C10's low byte is ramped during fades.
// TODO: verify BG8 mixing, color key, component brightness, flips and row/column scrolling.
void galpani2_state::copybg15(bitmap_rgb32 &bitmap, bitmap_ind8 &priority_bitmap, const rectangle &cliprect, int priority)
{
	pen_t const *const clut = &m_bg15palette->pen(0);
	int const mask_scrollx = *m_bg8_scrollx[1] + 0x42;
	int const mask_scrolly = *m_bg8_scrolly[1] + 0x0b;
	for (int layer = 0; layer < 4; layer++)
	{
		uint16_t const *const ram = &m_bg15[layer * 0x20000];
		uint16_t const *const regs = &m_bg15[0x80000 + layer * 0x20000];
		if (!(regs[0xc02 / 2] & 1))
			continue;
		if ((regs[0xc04 / 2] & 3) != priority)
			continue;

		// The native fade routine can leave a plane enabled at zero brightness.
		// Ignoring this makes the previous photo persist through title transitions.
		int const brightness = regs[0xc10 / 2] & 0xff;
		if (!brightness)
			continue;

		// Match the BG8 pixel origin: gameplay's 0x018e register value puts
		// framebuffer x=0 at screen x=48, the BG8 window's left edge.
		int const scrollx = regs[0x400 / 2] + 0x42;
		int const scrolly = regs[0xc00 / 2] + 0x0b;

		// For the foreground photo plane, clip to the gameplay field area.
		// The 512-pixel-wide framebuffer wraps around, and stale photo data
		// from the inactive half appears at the border/HUD positions.
		// The field spans 256 columns starting at m_field_origin_x.
		int const field_min_x = (layer == 3) ? m_field_origin_x : cliprect.min_x;
		int const field_max_x = (layer == 3) ? (m_field_origin_x + 255) : cliprect.max_x;
		for (int y = cliprect.min_y; y <= cliprect.max_y; y++)
		{
			int const sy = (y + scrolly) & 0xff;
			for (int x = field_min_x; x <= field_max_x; x++)
			{
				uint16_t const pen = ram[sy * 0x200 + ((x + scrollx) & 0x1ff)];
				// Native decoders set bit 15; native clear jobs remove it while
				// preserving RGB. Drawing unflagged words exposes POST ramps/old girls.
				// Decoded black padding must let the intro scenery show around a girl.
				if (!BIT(pen, 15) || !(pen & 0x7fff))
					continue;

				uint16_t color = pen & 0x7fff;
				// The normal game's foreground photo is plane 3. Covered field
				// colors select its solid silhouette; captured cells reveal RGB
				// even while their flashing palette colors retain bit 15.
				// Keep plane 2's scenery visible through holes in the foreground.
				// TODO: verify the mixer selection for the other RGB plane pairs.
				if (layer == 3)
				{
					uint16_t const mask = m_bg8[1][((y + mask_scrolly) & 0xff) * 512 + ((x + mask_scrollx) & 0x1ff)];
					if (!BIT(mask, 11) && !bg8_captured_field(mask) && BIT(m_bg8_palette_ram[mask & 0xff], 15))
						color = regs[0xc06 / 2] & 0x7fff;
				}
				bitmap.pix(y, x) = (brightness == 0xff)
					? clut[color]
					: alpha_blend_r32(bitmap.pix(y, x), clut[color], brightness);
				priority_bitmap.pix(y, x) = priority;
			}
		}
	}
}

uint32_t galpani2_state::screen_update_galpani2(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	int layers_ctrl = -1;

#if 1 // MAME_DEBUG
if (machine().input().code_pressed(KEYCODE_Z))
{
	int msk = 0;
	if (machine().input().code_pressed(KEYCODE_Q))  msk |= 1;
	if (machine().input().code_pressed(KEYCODE_W))  msk |= 2;
	if (machine().input().code_pressed(KEYCODE_E))  msk |= 4;
	if (machine().input().code_pressed(KEYCODE_A))  msk |= 8;
	if (msk != 0) layers_ctrl &= msk;
}
#endif

	// Phase 2 of the girl-bitmap HLE: regenerate the real bitmap after
	// the photo decode completes and re-expand it into BG8 plane 0.
	// The placeholder (girl_total=1, empty bitmap) was written at MCU
	// command time so the consumer's polling loop exited immediately.
	if (m_girl_bitmap_pending_addr)
	{
		if (m_girl_bitmap_delay > 0)
			m_girl_bitmap_delay--;
		else
		{
			address_space &mspace = m_maincpu->space(AS_PROGRAM);
			uint16_t girl_total = generate_girl_bitmap(mspace);
			logerror("MCU girl bitmap phase 2: %u girl pixels\n", unsigned(girl_total));

			// Re-expand the bitmap into BG8 plane 0 at field positions,
			// preserving cells that the native merge already captured.
			// The native merge routine (e.g. Quiz 24FCA) clears girl
			// flags on captured cells and recounts them. Our re-expansion
			// must not restore girl flags at positions where BG8 plane 1
			// already shows a captured state (F9/FD), or the accounting
			// and display will be inconsistent.
			int const ox = m_field_origin_x;
			int bit_idx = 0;
			for (int row = 0; row < 224; row++)
			{
				for (int col = 0; col < 256; col++)
				{
					int byte_pos = bit_idx / 8;
					int bit_pos = 7 - (bit_idx & 7);
					uint8_t byte_val = mspace.read_byte(GIRL_BITMAP_ADDR + byte_pos);
					bool is_girl = BIT(byte_val, bit_pos);
					int const vram_row = (8 + row) & 0xff;
					int const vram_col = ox + col;
					int const idx = vram_row * 512 + (vram_col & 0x1ff);
					// If plane 1 has a captured cell, keep plane 0 as non-girl
					// and subtract from girl_total to match native merge behavior.
					if (is_girl && bg8_captured_field(m_bg8[1][idx]))
					{
						m_bg8[0][idx] = m_mask_nongirl_pen;
						girl_total--;
					}
					else
					{
						m_bg8[0][idx] = is_girl ? m_mask_girl_pen : m_mask_nongirl_pen;
					}
					bit_idx++;
				}
			}

			// Edge removal: clear girl pixels at borders and adjust count.
			auto clear_edge = [&](int vrow, int vcol) {
				uint16_t &p = m_bg8[0][(vrow & 0xff) * 512 + (vcol & 0x1ff)];
				if (p & 0xc000)
				{
					p = m_mask_nongirl_pen;
					girl_total--;
				}
			};
			for (int c = 0; c < 256; c++)
			{
				clear_edge(8, ox + c);
				clear_edge(8 + 222, ox + c);
				clear_edge(8 + 223, ox + c);
			}
			for (int r = 0; r < 221; r++)
			{
				clear_edge(8 + r, ox);
				clear_edge(8 + r, ox + 254);
				clear_edge(8 + r, ox + 255);
			}

			// Update the game's counters at the per-region addresses.
			if (!girl_total)
				girl_total = 1;
			mspace.write_word(m_girl_total_addr, girl_total);
			mspace.write_word(m_other_total_addr, 0xe000 - girl_total);
			logerror("MCU girl bitmap final: %u girl pixels after edge removal\n",
				unsigned(girl_total));
			m_girl_bitmap_pending_addr = 0;
		}
	}

	bitmap.fill(0, cliprect);
	screen.priority().fill(0, cliprect);

/*  test mode:
    304000:0040 0000 0100 0000-0000 0000 0000 0000      (Sprite regs)
    304010:16C0 0200 16C0 0200-16C0 0200 16C0 0200
    16c0/40 = 5b        200/40 = 8
    scrollx = f5, on screen x should be 0 (f5+5b = 150) */

	if (layers_ctrl & 0x2) copybg8(bitmap, screen.priority(), cliprect, 0, false);
	if (layers_ctrl & 0x4) copybg8(bitmap, screen.priority(), cliprect, 1, false);
	if (layers_ctrl & 0x8)
	{
		m_kaneko_spr->render_sprites(cliprect, m_spriteram, m_spriteram.bytes());
		m_kaneko_spr->copybitmap(bitmap, cliprect, screen.priority());
	}
	if (layers_ctrl & 0x1)
		for (int priority = 0; priority < 4; priority++)
		{
			copybg15(bitmap, screen.priority(), cliprect, priority);
			// Keep lower sprite classes in the input to subsequent fades, while
			// restoring higher classes above the planes just composited.
			if (layers_ctrl & 0x8)
				m_kaneko_spr->copybitmap(bitmap, cliprect, screen.priority());
		}
	if (layers_ctrl & 0x2) copybg8(bitmap, screen.priority(), cliprect, 0, true);
	if (layers_ctrl & 0x4) copybg8(bitmap, screen.priority(), cliprect, 1, true);
	if (layers_ctrl & 0x8)
	{
		m_kaneko_spr->copybitmap(bitmap, cliprect, screen.priority());
	}
	return 0;
}
