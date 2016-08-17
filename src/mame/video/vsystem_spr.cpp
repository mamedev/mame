// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria, David Haywood
// Video System Sprites
// todo:
//  update drivers which call multiple priority passes to use the pdrawgfx version (aerofgt, gstriker)

//  according to gstriker this is probably the Fujitsu CG10103

// Aero Fighters (newer hardware)
// Quiz & Variety Sukusuku Inufuku
// 3 On 3 Dunk Madness
// Super Slams
// Formula 1 Grand Prix 2
// (Lethal) Crash Race
// Grand Striker
// V Goal Soccer
// Tecmo World Cup '94
// Tao Taido

/* old notes */

/*** Fujitsu CG10103 **********************************************/

/*
    Fujitsu CG10103 sprite generator
    --------------------------------

- Tile based
- 16x16 4bpp tiles
- Up to 7x7 in each block
- 5 bit of palette selection for the mixer
- Scaling (x/y)
- Flipping
- Indipendent sorting list
- 1 bit of pri for the mixer

Note that this chip can be connected to a VS9210 which adds a level of indirection for
tile numbers. Basically, the VS9210 indirects the tilet number through a table in its attached
memory, before accessing the ROMs.


    Sorting list format (VideoRAM offset 0)
    ---------------------------------------

de-- ---f ssss ssss

e=end of list
f=sprite present in this position
s=sprite index
d=disable sprite?


TODO:
Priorities should be right, but they probably need to be orthogonal with the mixer priorities.
Zoom factor is not correct, the scale is probably non-linear
Horizontal wrapping is just a hack. The chip probably calculates if it needs to draw the sprite at the
  normal position, or wrapped along X/Y.
Abstracts the VS9210

*/



#include "emu.h"
#include "vsystem_spr.h"






const device_type VSYSTEM_SPR = &device_creator<vsystem_spr_device>;

vsystem_spr_device::vsystem_spr_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, VSYSTEM_SPR, "Video System Sprites", tag, owner, clock, "vsystem_spr", __FILE__),
		m_gfxdecode(*this)
{
	m_transpen = 15;
	m_pal_base = 0;
	m_xoffs = 0;
	m_yoffs = 0;
	m_pdraw = false;
	m_gfx_region = -1;
	m_pal_mask = 0x3f;

	m_newtilecb =  vsystem_tile_indirection_delegate(FUNC(vsystem_spr_device::tile_callback_noindirect), this);

	memset(&m_curr_sprite, 0, sizeof(m_curr_sprite));
}

//-------------------------------------------------
//  static_set_gfxdecode_tag: Set the tag of the
//  gfx decoder
//-------------------------------------------------

void vsystem_spr_device::static_set_gfxdecode_tag(device_t &device, const char *tag)
{
	downcast<vsystem_spr_device &>(device).m_gfxdecode.set_tag(tag);
}

UINT32 vsystem_spr_device::tile_callback_noindirect(UINT32 tile)
{
	return tile;
}


// static
void vsystem_spr_device::set_tile_indirect_cb(device_t &device,vsystem_tile_indirection_delegate newtilecb)
{
	vsystem_spr_device &dev = downcast<vsystem_spr_device &>(device);
	dev.m_newtilecb = newtilecb;
}


// static
void vsystem_spr_device::set_offsets(device_t &device, int xoffs, int yoffs)
{
	vsystem_spr_device &dev = downcast<vsystem_spr_device &>(device);
	dev.m_xoffs = xoffs;
	dev.m_yoffs = yoffs;
}

// static
void vsystem_spr_device::set_pdraw(device_t &device, bool pdraw)
{
	vsystem_spr_device &dev = downcast<vsystem_spr_device &>(device);
	dev.m_pdraw = pdraw;
}

// static
void vsystem_spr_device::set_gfx_region(device_t &device, int gfx_region)
{
	vsystem_spr_device &dev = downcast<vsystem_spr_device &>(device);
	dev.m_gfx_region = gfx_region;
}

// static
void vsystem_spr_device::CG10103_set_pal_base(device_t &device, int pal_base)
{
	vsystem_spr_device &dev = downcast<vsystem_spr_device &>(device);
	dev.m_pal_base = pal_base;
}


void vsystem_spr_device::set_pal_base(int pal_base)
{
	m_pal_base = pal_base;
}

// static
void vsystem_spr_device::set_pal_mask(device_t &device, int pal_mask)
{
	vsystem_spr_device &dev = downcast<vsystem_spr_device &>(device);
	dev.m_pal_mask = pal_mask;
}

// static
void vsystem_spr_device::CG10103_set_transpen(device_t &device, int transpen)
{
	vsystem_spr_device &dev = downcast<vsystem_spr_device &>(device);
	dev.m_transpen = transpen;
}


void vsystem_spr_device::device_start()
{
	// bind our handler
	m_newtilecb.bind_relative_to(*owner());

	save_item(NAME(m_pal_base));

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

void vsystem_spr_device::device_reset()
{
}

void vsystem_spr_device::get_sprite_attributes(UINT16* ram)
{
	/*
	    attr_start + 0x0000
	    ---- ---x xxxx xxxx oy
	    ---- xxx- ---- ---- ysize
	    xxxx ---- ---- ---- zoomy

	    attr_start + 0x0001
	    ---- ---x xxxx xxxx ox
	    ---- xxx- ---- ---- xsize
	    xxxx ---- ---- ---- zoomx

	    attr_start + 0x0002
	    -x-- ---- ---- ---- flipx
	    x--- ---- ---- ---- flipy
	    --xx xxxx ---- ---- color
	    --xx ---- ---- ---- priority? (upper color bits)
	    ---- ---- ---- ---x map start (msb)

	    attr_start + 0x0003
	    xxxx xxxx xxxx xxxx map start (lsb)
	*/

	m_curr_sprite.oy =    (ram[0] & 0x01ff);
	m_curr_sprite.ysize = (ram[0] & 0x0e00) >> 9;
	m_curr_sprite.zoomy = (ram[0] & 0xf000) >> 12;

	m_curr_sprite.ox =    (ram[1] & 0x01ff);
	m_curr_sprite.xsize = (ram[1] & 0x0e00) >> 9;
	m_curr_sprite.zoomx = (ram[1] & 0xf000) >> 12;

	m_curr_sprite.flipx = (ram[2] & 0x4000);
	m_curr_sprite.flipy = (ram[2] & 0x8000);
	m_curr_sprite.color = (ram[2] & 0x3f00) >> 8;
	m_curr_sprite.pri   = (ram[2] & 0x3000) >> 12;
	m_curr_sprite.map   = (ram[2] & 0x0001) << 16;

	m_curr_sprite.map  |= (ram[3] & 0xffff);
}


void vsystem_spr_device::common_sprite_drawgfx(bitmap_ind16 &bitmap, const rectangle &cliprect, bitmap_ind8 &priority_bitmap)
{
	gfx_element *gfx = m_gfxdecode->gfx(m_gfx_region);
	int priority_mask = 0x00;

	m_curr_sprite.oy += m_yoffs;
	m_curr_sprite.ox += m_xoffs;

	if (m_pdraw)
	{
		switch (m_curr_sprite.pri)
		{
			default:
			case 0: priority_mask = 0x00; break;
			case 3: priority_mask = 0xfe; break;
			case 2: priority_mask = 0xfc; break;
			case 1: priority_mask = 0xf0; break;
		}
	}

	m_curr_sprite.zoomx = 32 - m_curr_sprite.zoomx;
	m_curr_sprite.zoomy = 32 - m_curr_sprite.zoomy;

	int ystart, yend, yinc;

	if (!m_curr_sprite.flipy) { ystart = 0; yend = m_curr_sprite.ysize+1; yinc = 1; }
	else                    { ystart = m_curr_sprite.ysize; yend = -1; yinc = -1; }

	int ycnt = ystart;
	while (ycnt != yend)
	{
		int xstart, xend, xinc;

		if (!m_curr_sprite.flipx) { xstart = 0; xend = m_curr_sprite.xsize+1; xinc = 1; }
		else                    { xstart = m_curr_sprite.xsize; xend = -1; xinc = -1; }

		int xcnt = xstart;
		while (xcnt != xend)
		{
			int startno = m_newtilecb(m_curr_sprite.map++);
			if (m_pdraw)
			{
				gfx->prio_zoom_transpen(bitmap,cliprect, startno, m_curr_sprite.color + m_pal_base, m_curr_sprite.flipx, m_curr_sprite.flipy, m_curr_sprite.ox + xcnt * m_curr_sprite.zoomx/2,        m_curr_sprite.oy + ycnt * m_curr_sprite.zoomy/2,        m_curr_sprite.zoomx << 11, m_curr_sprite.zoomy << 11, priority_bitmap, priority_mask, m_transpen);
				gfx->prio_zoom_transpen(bitmap,cliprect, startno, m_curr_sprite.color + m_pal_base, m_curr_sprite.flipx, m_curr_sprite.flipy, -0x200+m_curr_sprite.ox + xcnt * m_curr_sprite.zoomx/2, m_curr_sprite.oy + ycnt * m_curr_sprite.zoomy/2,        m_curr_sprite.zoomx << 11, m_curr_sprite.zoomy << 11, priority_bitmap, priority_mask, m_transpen);
				gfx->prio_zoom_transpen(bitmap,cliprect, startno, m_curr_sprite.color + m_pal_base, m_curr_sprite.flipx, m_curr_sprite.flipy, m_curr_sprite.ox + xcnt * m_curr_sprite.zoomx/2,        -0x200+m_curr_sprite.oy + ycnt * m_curr_sprite.zoomy/2, m_curr_sprite.zoomx << 11, m_curr_sprite.zoomy << 11, priority_bitmap, priority_mask, m_transpen);
				gfx->prio_zoom_transpen(bitmap,cliprect, startno, m_curr_sprite.color + m_pal_base, m_curr_sprite.flipx, m_curr_sprite.flipy, -0x200+m_curr_sprite.ox + xcnt * m_curr_sprite.zoomx/2, -0x200+m_curr_sprite.oy + ycnt * m_curr_sprite.zoomy/2, m_curr_sprite.zoomx << 11, m_curr_sprite.zoomy << 11, priority_bitmap, priority_mask, m_transpen);
			}
			else
			{
				gfx->zoom_transpen(bitmap,cliprect, startno, m_curr_sprite.color + m_pal_base, m_curr_sprite.flipx, m_curr_sprite.flipy, m_curr_sprite.ox + xcnt * m_curr_sprite.zoomx/2,        m_curr_sprite.oy + ycnt * m_curr_sprite.zoomy/2,        m_curr_sprite.zoomx << 11, m_curr_sprite.zoomy << 11, m_transpen);
				gfx->zoom_transpen(bitmap,cliprect, startno, m_curr_sprite.color + m_pal_base, m_curr_sprite.flipx, m_curr_sprite.flipy, -0x200+m_curr_sprite.ox + xcnt * m_curr_sprite.zoomx/2, m_curr_sprite.oy + ycnt * m_curr_sprite.zoomy/2,        m_curr_sprite.zoomx << 11, m_curr_sprite.zoomy << 11, m_transpen);
				gfx->zoom_transpen(bitmap,cliprect, startno, m_curr_sprite.color + m_pal_base, m_curr_sprite.flipx, m_curr_sprite.flipy, m_curr_sprite.ox + xcnt * m_curr_sprite.zoomx/2,        -0x200+m_curr_sprite.oy + ycnt * m_curr_sprite.zoomy/2, m_curr_sprite.zoomx << 11, m_curr_sprite.zoomy << 11, m_transpen);
				gfx->zoom_transpen(bitmap,cliprect, startno, m_curr_sprite.color + m_pal_base, m_curr_sprite.flipx, m_curr_sprite.flipy, -0x200+m_curr_sprite.ox + xcnt * m_curr_sprite.zoomx/2, -0x200+m_curr_sprite.oy + ycnt * m_curr_sprite.zoomy/2, m_curr_sprite.zoomx << 11, m_curr_sprite.zoomy << 11, m_transpen);
			}
			xcnt+=xinc;
		}
		ycnt+=yinc;
	}

}



void vsystem_spr_device::draw_sprites( UINT16* spriteram, int spriteram_bytes, screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int prihack_mask, int prihack_val )
{
	int offs;
	int end = 0;

	// find the end of the list
	for (offs = 0; offs < (spriteram_bytes / 16 ); offs++)
	{
		if (spriteram[offs] & 0x4000) break;
	}
	end = offs;

	// decide our drawing order (if we're using pdrawgfx we must go in reverse)
	int first, last, inc;
	if (m_pdraw)
	{
		first = end - 1;
		last = -1;
		inc = -1;
	}
	else
	{
		first = 0;
		last = end;
		inc = 1;
	}

	// draw
	offs = first;
	while (offs != last)
	{
		if ((spriteram[offs] & 0x8000) == 0x0000)
		{
			int attr_start;

			attr_start = 4 * (spriteram[offs] & 0x03ff);

			get_sprite_attributes(&spriteram[attr_start]);

			m_curr_sprite.color &= m_pal_mask;

			// hack for aero fighters and other which still call us multiple times with different priorities instead of using the pdrawgfx version
			if (prihack_mask != -1)
			{
				if ((m_curr_sprite.pri & prihack_mask) == prihack_val)
					common_sprite_drawgfx(bitmap, cliprect, screen.priority());
			}
			else
			{
				common_sprite_drawgfx(bitmap, cliprect, screen.priority());
			}
		}

		offs+=inc;
	}
}
