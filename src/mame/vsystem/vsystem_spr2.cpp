// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria, David Haywood
// Video System Sprites (type 2)
// todo:
//  move various vsystem sprite functions here
//  unify common ones + convert to device

//  this is probably the VS 8904/8905 combo

// Spinal Breakers
// Power Spikes
// Karate Blazers
// Turbo Force
// Aero Fighters (older hardware types)
// World Beach Championships 97 (modern Power Spikes bootleg, not original hw)
// Welltris
// Formula 1 Grand Prix (1)
// Pipe Dream

// there were lots of comments saying drivers using the
//  static const uint8_t zoomtable[16] = { 0,7,14,20,25,30,34,38,42,46,49,52,54,57,59,61 };
// table for zooming needed upgrading, are we sure this isn't one of the
// differences between this sprite chip and the one in vsystem_spr.c, pspikes zooming is very rough


#include "emu.h"
#include "vsystem_spr2.h"


DEFINE_DEVICE_TYPE(VSYSTEM_SPR2, vsystem_spr2_device, "vsystem2_spr", "Video System VS8904/VS8905 Sprites")

vsystem_spr2_device::vsystem_spr2_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, VSYSTEM_SPR2, tag, owner, clock)
	, device_gfx_interface(mconfig, *this)
	, m_newtilecb(*this, DEVICE_SELF, FUNC(vsystem_spr2_device::tile_callback_noindirect))
	, m_pritype(0) // hack until we have better handling
	, m_xoffs(0)
	, m_yoffs(0)
	, m_curr_sprite()
{
}

uint32_t vsystem_spr2_device::tile_callback_noindirect(uint32_t tile)
{
	return tile;
}


void vsystem_spr2_device::device_start()
{
	// bind our handler
	m_newtilecb.resolve();

	save_item(NAME(m_curr_sprite.ox));
	save_item(NAME(m_curr_sprite.xsize));
	save_item(NAME(m_curr_sprite.zoomx));
	save_item(NAME(m_curr_sprite.oy));
	save_item(NAME(m_curr_sprite.ysize));
	save_item(NAME(m_curr_sprite.zoomy));
	save_item(NAME(m_curr_sprite.flipx));
	save_item(NAME(m_curr_sprite.flipy));
	save_item(NAME(m_curr_sprite.color));
	save_item(NAME(m_curr_sprite.pri));
	save_item(NAME(m_curr_sprite.map));
}

void vsystem_spr2_device::device_reset()
{
}


bool vsystem_spr2_device::sprite_attributes::get(uint16_t const *ram)
{
	// sprite is disabled
	if (BIT(~ram[2], 7))
		return false;

	oy    =  (ram[0] & 0x01ff);
	zoomy =  (ram[0] & 0xf000) >> 12;

	ox =     (ram[1] & 0x01ff);
	zoomx =  (ram[1] & 0xf000) >> 12;

	xsize =  (ram[2] & 0x0700) >> 8;
	flipx =  BIT(ram[2], 11);

	ysize =  (ram[2] & 0x7000) >> 12;
	flipy =  BIT(ram[2], 15);

	color =  (ram[2] & 0x000f);
	pri =    (ram[2] & 0x0010);

	map =    (ram[3]);

	return true;
}

void vsystem_spr2_device::sprite_attributes::handle_xsize_map_inc()
{
	if (xsize == 2) map += 1;
	if (xsize == 4) map += 3;
	if (xsize == 5) map += 2;
	if (xsize == 6) map += 1;
}

template<class BitmapClass>
void vsystem_spr2_device::draw_sprites_common(uint16_t const *spriteram3,  int spriteram3_bytes, int spritepalettebank, BitmapClass &bitmap, const rectangle &cliprect, bitmap_ind8 &priority_bitmap, int pri_param, bool flip_screen)
{
	int first = 4 * spriteram3[0x1fe];

	first &= 0x1ff;
	if (first > 0x200 - 4)
		first = 0x200 - 4;

	int start, end, inc;

	if (m_pritype == 0 || m_pritype == 1 || m_pritype == 2) // prdrawgfx cases
	{
		start = 0x200 - 8;
		end = first - 4;
		inc = -4;
	}
	else // drawgfx cases
	{
		start = first;
		end = 0x200 - 4;
		inc = 4;
	}

	for (int attr_start = start; attr_start != end; attr_start += inc)
	{
		if (!m_curr_sprite.get(&spriteram3[attr_start]))
			continue;

		// pipedrm
		m_curr_sprite.ox  += m_xoffs;
		m_curr_sprite.oy  += m_yoffs;

		uint32_t usepri = 0;

		// these are still calling the function multiple times to filter out priorities, even if some are also using pdrawgfx(!)
		if (m_pritype == 0 || m_pritype == 1 || m_pritype == 3) // turbo force, spinlbrk, pipedrm etc.
		{
			if ((m_curr_sprite.pri>>4) != pri_param)
				continue;
		}


		if (m_pritype == 0) // turbo force etc.
		{
			usepri = m_curr_sprite.pri ? 0 : 2;
		}
		else if (m_pritype == 1) // spinlbrk
		{
			usepri = m_curr_sprite.pri ? 2 : 0;
		}
		else if (m_pritype == 2) // f1gp
		{
			usepri = pri_param;
		}

		bool fx = m_curr_sprite.flipx;
		bool fy = m_curr_sprite.flipy;
		if (flip_screen)
		{
			m_curr_sprite.ox = 308 - m_curr_sprite.ox;
			m_curr_sprite.oy = 208 - m_curr_sprite.oy;
			fx = !fx;
			fy = !fy;
		}

		m_curr_sprite.color += 16 * spritepalettebank;

		m_curr_sprite.zoomx = 32 - m_curr_sprite.zoomx;
		m_curr_sprite.zoomy = 32 - m_curr_sprite.zoomy;

		uint32_t const zx = m_curr_sprite.zoomx << 11;
		uint32_t const zy = m_curr_sprite.zoomy << 11;

		for (int y = 0; y <= m_curr_sprite.ysize; y++)
		{
			int const cy = m_curr_sprite.flipy ? (m_curr_sprite.ysize - y) : y;
			int const sy = ((m_curr_sprite.oy + m_curr_sprite.zoomy * (flip_screen ? -cy : cy) / 2 + 16) & 0x1ff) - 16;

			for (int x = 0; x <= m_curr_sprite.xsize; x++)
			{
				int const cx = m_curr_sprite.flipx ? (m_curr_sprite.xsize - x) : x;
				int const sx = ((m_curr_sprite.ox + m_curr_sprite.zoomx * (flip_screen ? -cx : cx) / 2 + 16) & 0x1ff) - 16;

				uint32_t const curr = m_newtilecb(m_curr_sprite.map++);

				if (m_pritype == 0 || m_pritype == 1 || m_pritype == 2) // pdrawgfx cases
				{
					gfx(0)->prio_zoom_transpen(bitmap, cliprect, curr, m_curr_sprite.color, fx, fy, sx-0x000,sy-0x000, zx, zy,  priority_bitmap, usepri, 15);
					gfx(0)->prio_zoom_transpen(bitmap, cliprect, curr, m_curr_sprite.color, fx, fy, sx-0x200,sy-0x000, zx, zy,  priority_bitmap, usepri, 15);
					gfx(0)->prio_zoom_transpen(bitmap, cliprect, curr, m_curr_sprite.color, fx, fy, sx-0x000,sy-0x200, zx, zy,  priority_bitmap, usepri, 15);
					gfx(0)->prio_zoom_transpen(bitmap, cliprect, curr, m_curr_sprite.color, fx, fy, sx-0x200,sy-0x200, zx, zy,  priority_bitmap, usepri, 15);
				}
				else // drawgfx cases (welltris, pipedrm)
				{
					gfx(0)->zoom_transpen(bitmap, cliprect, curr, m_curr_sprite.color, fx, fy, sx-0x000, sy-0x000, zx, zy, 15);
					gfx(0)->zoom_transpen(bitmap, cliprect, curr, m_curr_sprite.color, fx, fy, sx-0x200, sy-0x000, zx, zy, 15);
					gfx(0)->zoom_transpen(bitmap, cliprect, curr, m_curr_sprite.color, fx, fy, sx-0x000, sy-0x200, zx, zy, 15);
					gfx(0)->zoom_transpen(bitmap, cliprect, curr, m_curr_sprite.color, fx, fy, sx-0x200, sy-0x200, zx, zy, 15);
				}

			}
			m_curr_sprite.handle_xsize_map_inc();
		}
	}
}

void vsystem_spr2_device::draw_sprites(uint16_t const *spriteram3,  int spriteram3_bytes, int spritepalettebank, bitmap_ind16 &bitmap, const rectangle &cliprect, bitmap_ind8 &priority_bitmap, int pri_param, bool flip_screen)
{
	draw_sprites_common(spriteram3, spriteram3_bytes, spritepalettebank, bitmap, cliprect, priority_bitmap, pri_param, flip_screen);
}

void vsystem_spr2_device::draw_sprites(uint16_t const *spriteram3,  int spriteram3_bytes, int spritepalettebank, bitmap_rgb32 &bitmap, const rectangle &cliprect, bitmap_ind8 &priority_bitmap, int pri_param, bool flip_screen)
{
	draw_sprites_common(spriteram3, spriteram3_bytes, spritepalettebank, bitmap, cliprect, priority_bitmap, pri_param, flip_screen);
}
