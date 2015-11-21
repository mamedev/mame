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
//  static const UINT8 zoomtable[16] = { 0,7,14,20,25,30,34,38,42,46,49,52,54,57,59,61 };
// table for zooming needed upgrading, are we sure this isn't one of the
// differences between this sprite chip and the one in vsystem_spr.c, pspikes zooming is very rough


#include "emu.h"
#include "vsystem_spr2.h"


const device_type VSYSTEM_SPR2 = &device_creator<vsystem_spr2_device>;

vsystem_spr2_device::vsystem_spr2_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, VSYSTEM_SPR2, "Video System Sprites Type 2", tag, owner, clock, "vsystem_spr2", __FILE__),
		m_gfxdecode(*this),
		m_palette(*this)
{
	m_newtilecb =  vsystem_tile2_indirection_delegate(FUNC(vsystem_spr2_device::tile_callback_noindirect), this);
	m_pritype = 0; // hack until we have better handling
	m_gfx_region = 0;
	m_xoffs = 0;
	m_yoffs = 0;
}

//-------------------------------------------------
//  static_set_gfxdecode_tag: Set the tag of the
//  gfx decoder
//-------------------------------------------------

void vsystem_spr2_device::static_set_gfxdecode_tag(device_t &device, const char *tag)
{
	downcast<vsystem_spr2_device &>(device).m_gfxdecode.set_tag(tag);
}

//-------------------------------------------------
//  static_set_palette_tag: Set the tag of the
//  palette device
//-------------------------------------------------

void vsystem_spr2_device::static_set_palette_tag(device_t &device, const char *tag)
{
	downcast<vsystem_spr2_device &>(device).m_palette.set_tag(tag);
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

UINT32 vsystem_spr2_device::tile_callback_noindirect(UINT32 tile)
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


int vsystem_spr2_device::get_sprite_attributes(UINT16* ram)
{
	// sprite is disabled
	if (!(ram[2] & 0x0080))
		return 0;

	curr_sprite.oy    =  (ram[0] & 0x01ff);
	curr_sprite.zoomy =  (ram[0] & 0xf000) >> 12;

	curr_sprite.ox =     (ram[1] & 0x01ff);
	curr_sprite.zoomx =  (ram[1] & 0xf000) >> 12;

	curr_sprite.xsize =  (ram[2] & 0x0700) >> 8;
	curr_sprite.flipx =  (ram[2] & 0x0800);

	curr_sprite.ysize =  (ram[2] & 0x7000) >> 12;
	curr_sprite.flipy =  (ram[2] & 0x8000);

	curr_sprite.color =  (ram[2] & 0x000f);
	curr_sprite.pri =    (ram[2] & 0x0010);

	curr_sprite.map =    (ram[3]);

	return 1;
}

void vsystem_spr2_device::handle_xsize_map_inc(void)
{
	if (curr_sprite.xsize == 2) curr_sprite.map += 1;
	if (curr_sprite.xsize == 4) curr_sprite.map += 3;
	if (curr_sprite.xsize == 5) curr_sprite.map += 2;
	if (curr_sprite.xsize == 6) curr_sprite.map += 1;
}

template<class _BitmapClass>
void vsystem_spr2_device::turbofrc_draw_sprites_common( UINT16* spriteram3,  int spriteram3_bytes, int spritepalettebank, _BitmapClass &bitmap, const rectangle &cliprect, bitmap_ind8 &priority_bitmap, int pri_param )
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

		if (!get_sprite_attributes(&spriteram3[attr_start]))
			continue;

		// pipedrm
		curr_sprite.ox  += m_xoffs;
		curr_sprite.oy  += m_yoffs;

		int usepri = 0;

		// these are still calling the function multiple times to filter out priorities, even if some are also using pdrawgfx(!)
		if (m_pritype == 0 || m_pritype == 1 || m_pritype == 3) // turbo force, spinlbrk, pipedrm etc.
		{
			if ((curr_sprite.pri>>4) != pri_param)
				continue;
		}


		if (m_pritype == 0) // turbo force etc.
		{
			usepri = curr_sprite.pri ? 0 : 2;
		}
		else if (m_pritype == 1) // spinlbrk
		{
			usepri = curr_sprite.pri ? 2 : 0;
		}
		else if (m_pritype == 2) // f1gp
		{
			usepri = pri_param;
		}



		curr_sprite.color += 16 * spritepalettebank;

		curr_sprite.zoomx = 32 - curr_sprite.zoomx;
		curr_sprite.zoomy = 32 - curr_sprite.zoomy;

		for (y = 0; y <= curr_sprite.ysize; y++)
		{
			int sx, sy;

			if (curr_sprite.flipy)
				sy = ((curr_sprite.oy + curr_sprite.zoomy * (curr_sprite.ysize - y)/2 + 16) & 0x1ff) - 16;
			else
				sy = ((curr_sprite.oy + curr_sprite.zoomy * y / 2 + 16) & 0x1ff) - 16;

			for (x = 0; x <= curr_sprite.xsize; x++)
			{
				int curr;

				if (curr_sprite.flipx)
					sx = ((curr_sprite.ox + curr_sprite.zoomx * (curr_sprite.xsize - x) / 2 + 16) & 0x1ff) - 16;
				else
					sx = ((curr_sprite.ox + curr_sprite.zoomx * x / 2 + 16) & 0x1ff) - 16;

				curr = m_newtilecb(curr_sprite.map++);


				if (m_pritype == 0 || m_pritype == 1 || m_pritype == 2) // pdrawgfx cases
				{
					m_gfxdecode->gfx(m_gfx_region)->prio_zoom_transpen(bitmap,cliprect, curr, curr_sprite.color, curr_sprite.flipx,curr_sprite.flipy, sx-0x000,sy-0x000, curr_sprite.zoomx << 11, curr_sprite.zoomy << 11,  priority_bitmap,usepri,15);
					m_gfxdecode->gfx(m_gfx_region)->prio_zoom_transpen(bitmap,cliprect, curr, curr_sprite.color, curr_sprite.flipx,curr_sprite.flipy, sx-0x200,sy-0x000, curr_sprite.zoomx << 11, curr_sprite.zoomy << 11,  priority_bitmap,usepri,15);
					m_gfxdecode->gfx(m_gfx_region)->prio_zoom_transpen(bitmap,cliprect, curr, curr_sprite.color, curr_sprite.flipx,curr_sprite.flipy, sx-0x000,sy-0x200, curr_sprite.zoomx << 11, curr_sprite.zoomy << 11,  priority_bitmap,usepri,15);
					m_gfxdecode->gfx(m_gfx_region)->prio_zoom_transpen(bitmap,cliprect, curr, curr_sprite.color, curr_sprite.flipx,curr_sprite.flipy, sx-0x200,sy-0x200, curr_sprite.zoomx << 11, curr_sprite.zoomy << 11,  priority_bitmap,usepri,15);
				}
				else // drawgfx cases (welltris, pipedrm)
				{
					m_gfxdecode->gfx(m_gfx_region)->zoom_transpen(bitmap,cliprect, curr, curr_sprite.color, curr_sprite.flipx,curr_sprite.flipy, sx-0x000,sy-0x000, curr_sprite.zoomx << 11, curr_sprite.zoomy << 11,15);
					m_gfxdecode->gfx(m_gfx_region)->zoom_transpen(bitmap,cliprect, curr, curr_sprite.color, curr_sprite.flipx,curr_sprite.flipy, sx-0x200,sy-0x000, curr_sprite.zoomx << 11, curr_sprite.zoomy << 11,15);
					m_gfxdecode->gfx(m_gfx_region)->zoom_transpen(bitmap,cliprect, curr, curr_sprite.color, curr_sprite.flipx,curr_sprite.flipy, sx-0x000,sy-0x200, curr_sprite.zoomx << 11, curr_sprite.zoomy << 11,15);
					m_gfxdecode->gfx(m_gfx_region)->zoom_transpen(bitmap,cliprect, curr, curr_sprite.color, curr_sprite.flipx,curr_sprite.flipy, sx-0x200,sy-0x200, curr_sprite.zoomx << 11, curr_sprite.zoomy << 11,15);
				}

			}
			handle_xsize_map_inc();
		}
	}
}

void vsystem_spr2_device::turbofrc_draw_sprites( UINT16* spriteram3,  int spriteram3_bytes, int spritepalettebank, bitmap_ind16 &bitmap, const rectangle &cliprect, bitmap_ind8 &priority_bitmap, int pri_param )
{ turbofrc_draw_sprites_common( spriteram3, spriteram3_bytes, spritepalettebank, bitmap, cliprect, priority_bitmap, pri_param ); }

void vsystem_spr2_device::turbofrc_draw_sprites( UINT16* spriteram3,  int spriteram3_bytes, int spritepalettebank, bitmap_rgb32 &bitmap, const rectangle &cliprect, bitmap_ind8 &priority_bitmap, int pri_param )
{ turbofrc_draw_sprites_common( spriteram3, spriteram3_bytes, spritepalettebank, bitmap, cliprect, priority_bitmap, pri_param ); }
