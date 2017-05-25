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


DEFINE_DEVICE_TYPE(VSYSTEM_SPR2, vsystem_spr2_device, "vsystem2_spr", "Video System Sprites Type 2")

vsystem_spr2_device::vsystem_spr2_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, VSYSTEM_SPR2, tag, owner, clock)
	, m_newtilecb(vsystem_tile2_indirection_delegate(FUNC(vsystem_spr2_device::tile_callback_noindirect), this))
	, m_pritype(0) // hack until we have better handling
	, m_gfx_region(0)
	, m_xoffs(0)
	, m_yoffs(0)
	, m_curr_sprite()
	, m_gfxdecode(*this, finder_base::DUMMY_TAG)
{
}

//-------------------------------------------------
//  static_set_gfxdecode_tag: Set the tag of the
//  gfx decoder
//-------------------------------------------------

void vsystem_spr2_device::static_set_gfxdecode_tag(device_t &device, const char *tag)
{
	downcast<vsystem_spr2_device &>(device).m_gfxdecode.set_tag(tag);
}


void vsystem_spr2_device::set_tile_indirect_cb(device_t &device,vsystem_tile2_indirection_delegate newtilecb)
{
	vsystem_spr2_device &dev = downcast<vsystem_spr2_device &>(device);
	dev.m_newtilecb = newtilecb;
}

void vsystem_spr2_device::set_pritype(device_t &device,int pritype)
{
	vsystem_spr2_device &dev = downcast<vsystem_spr2_device &>(device);
	dev.m_pritype = pritype;
}

void vsystem_spr2_device::set_gfx_region(device_t &device, int gfx_region)
{
	vsystem_spr2_device &dev = downcast<vsystem_spr2_device &>(device);
	dev.m_gfx_region = gfx_region;
}

void vsystem_spr2_device::set_offsets(device_t &device, int xoffs, int yoffs)
{
	vsystem_spr2_device &dev = downcast<vsystem_spr2_device &>(device);
	dev.m_xoffs = xoffs;
	dev.m_yoffs = yoffs;
}

uint32_t vsystem_spr2_device::tile_callback_noindirect(uint32_t tile)
{
	return tile;
}


void vsystem_spr2_device::device_start()
{
	// bind our handler
	m_newtilecb.bind_relative_to(*owner());
}

void vsystem_spr2_device::device_reset()
{
}


bool vsystem_spr2_device::sprite_attributes::get(uint16_t const *ram)
{
	// sprite is disabled
	if (!(ram[2] & 0x0080))
		return false;

	oy    =  (ram[0] & 0x01ff);
	zoomy =  (ram[0] & 0xf000) >> 12;

	ox =     (ram[1] & 0x01ff);
	zoomx =  (ram[1] & 0xf000) >> 12;

	xsize =  (ram[2] & 0x0700) >> 8;
	flipx =  (ram[2] & 0x0800);

	ysize =  (ram[2] & 0x7000) >> 12;
	flipy =  (ram[2] & 0x8000);

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
void vsystem_spr2_device::turbofrc_draw_sprites_common(uint16_t const *spriteram3,  int spriteram3_bytes, int spritepalettebank, BitmapClass &bitmap, const rectangle &cliprect, bitmap_ind8 &priority_bitmap, int pri_param )
{
	int attr_start, first;
	first = 4 * spriteram3[0x1fe];

	first &= 0x1ff;
	if (first>0x200-4)
		first = 0x200-4;

	int start,end,inc;

	if (m_pritype == 0 || m_pritype == 1 || m_pritype == 2) // prdrawgfx cases
	{
		start = 0x200 - 8;
		end = first-4;
		inc = -4;
	}
	else // drawgfx cases
	{
		start = first;
		end = 0x200 -4;
		inc = 4;
	}

	for (attr_start = start; attr_start != end; attr_start += inc)
	{
		int x, y;

		if (!m_curr_sprite.get(&spriteram3[attr_start]))
			continue;

		// pipedrm
		m_curr_sprite.ox  += m_xoffs;
		m_curr_sprite.oy  += m_yoffs;

		int usepri = 0;

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



		m_curr_sprite.color += 16 * spritepalettebank;

		m_curr_sprite.zoomx = 32 - m_curr_sprite.zoomx;
		m_curr_sprite.zoomy = 32 - m_curr_sprite.zoomy;

		for (y = 0; y <= m_curr_sprite.ysize; y++)
		{
			int sx, sy;

			if (m_curr_sprite.flipy)
				sy = ((m_curr_sprite.oy + m_curr_sprite.zoomy * (m_curr_sprite.ysize - y)/2 + 16) & 0x1ff) - 16;
			else
				sy = ((m_curr_sprite.oy + m_curr_sprite.zoomy * y / 2 + 16) & 0x1ff) - 16;

			for (x = 0; x <= m_curr_sprite.xsize; x++)
			{
				int curr;

				if (m_curr_sprite.flipx)
					sx = ((m_curr_sprite.ox + m_curr_sprite.zoomx * (m_curr_sprite.xsize - x) / 2 + 16) & 0x1ff) - 16;
				else
					sx = ((m_curr_sprite.ox + m_curr_sprite.zoomx * x / 2 + 16) & 0x1ff) - 16;

				curr = m_newtilecb(m_curr_sprite.map++);


				if (m_pritype == 0 || m_pritype == 1 || m_pritype == 2) // pdrawgfx cases
				{
					m_gfxdecode->gfx(m_gfx_region)->prio_zoom_transpen(bitmap,cliprect, curr, m_curr_sprite.color, m_curr_sprite.flipx,m_curr_sprite.flipy, sx-0x000,sy-0x000, m_curr_sprite.zoomx << 11, m_curr_sprite.zoomy << 11,  priority_bitmap,usepri,15);
					m_gfxdecode->gfx(m_gfx_region)->prio_zoom_transpen(bitmap,cliprect, curr, m_curr_sprite.color, m_curr_sprite.flipx,m_curr_sprite.flipy, sx-0x200,sy-0x000, m_curr_sprite.zoomx << 11, m_curr_sprite.zoomy << 11,  priority_bitmap,usepri,15);
					m_gfxdecode->gfx(m_gfx_region)->prio_zoom_transpen(bitmap,cliprect, curr, m_curr_sprite.color, m_curr_sprite.flipx,m_curr_sprite.flipy, sx-0x000,sy-0x200, m_curr_sprite.zoomx << 11, m_curr_sprite.zoomy << 11,  priority_bitmap,usepri,15);
					m_gfxdecode->gfx(m_gfx_region)->prio_zoom_transpen(bitmap,cliprect, curr, m_curr_sprite.color, m_curr_sprite.flipx,m_curr_sprite.flipy, sx-0x200,sy-0x200, m_curr_sprite.zoomx << 11, m_curr_sprite.zoomy << 11,  priority_bitmap,usepri,15);
				}
				else // drawgfx cases (welltris, pipedrm)
				{
					m_gfxdecode->gfx(m_gfx_region)->zoom_transpen(bitmap,cliprect, curr, m_curr_sprite.color, m_curr_sprite.flipx,m_curr_sprite.flipy, sx-0x000,sy-0x000, m_curr_sprite.zoomx << 11, m_curr_sprite.zoomy << 11,15);
					m_gfxdecode->gfx(m_gfx_region)->zoom_transpen(bitmap,cliprect, curr, m_curr_sprite.color, m_curr_sprite.flipx,m_curr_sprite.flipy, sx-0x200,sy-0x000, m_curr_sprite.zoomx << 11, m_curr_sprite.zoomy << 11,15);
					m_gfxdecode->gfx(m_gfx_region)->zoom_transpen(bitmap,cliprect, curr, m_curr_sprite.color, m_curr_sprite.flipx,m_curr_sprite.flipy, sx-0x000,sy-0x200, m_curr_sprite.zoomx << 11, m_curr_sprite.zoomy << 11,15);
					m_gfxdecode->gfx(m_gfx_region)->zoom_transpen(bitmap,cliprect, curr, m_curr_sprite.color, m_curr_sprite.flipx,m_curr_sprite.flipy, sx-0x200,sy-0x200, m_curr_sprite.zoomx << 11, m_curr_sprite.zoomy << 11,15);
				}

			}
			m_curr_sprite.handle_xsize_map_inc();
		}
	}
}

void vsystem_spr2_device::turbofrc_draw_sprites(uint16_t const *spriteram3,  int spriteram3_bytes, int spritepalettebank, bitmap_ind16 &bitmap, const rectangle &cliprect, bitmap_ind8 &priority_bitmap, int pri_param )
{
	turbofrc_draw_sprites_common( spriteram3, spriteram3_bytes, spritepalettebank, bitmap, cliprect, priority_bitmap, pri_param );
}

void vsystem_spr2_device::turbofrc_draw_sprites(uint16_t const *spriteram3,  int spriteram3_bytes, int spritepalettebank, bitmap_rgb32 &bitmap, const rectangle &cliprect, bitmap_ind8 &priority_bitmap, int pri_param )
{
	turbofrc_draw_sprites_common( spriteram3, spriteram3_bytes, spritepalettebank, bitmap, cliprect, priority_bitmap, pri_param );
}
