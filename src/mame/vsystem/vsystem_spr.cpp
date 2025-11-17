// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria, David Haywood
// Video System Sprites

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

// old notes

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
- Independent sorting list
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
#include "screen.h"


DEFINE_DEVICE_TYPE(VSYSTEM_SPR, vsystem_spr_device, "vsystem_spr", "Video System VS9108 Sprites")

vsystem_spr_device::vsystem_spr_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, VSYSTEM_SPR, tag, owner, clock)
	, device_gfx_interface(mconfig, *this)
	, m_newtile_cb(*this, DEVICE_SELF, FUNC(vsystem_spr_device::tile_callback_noindirect))
	, m_pri_cb(*this)
	, m_xoffs(0)
	, m_yoffs(0)
	, m_pal_mask(0x3f)
	, m_transpen(15)
	, m_pal_base(0)
	, m_curr_sprite()
{
}

uint32_t vsystem_spr_device::tile_callback_noindirect(uint32_t tile)
{
	return tile;
}

void vsystem_spr_device::device_start()
{
	// bind our handler
	m_newtile_cb.resolve();
	m_pri_cb.resolve();

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
	save_item(NAME(m_curr_sprite.map));
}

void vsystem_spr_device::device_reset()
{
}

void vsystem_spr_device::sprite_attributes::get(uint16_t const *ram)
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

	oy =    (ram[0] & 0x01ff);
	ysize = (ram[0] & 0x0e00) >> 9;
	zoomy = (ram[0] & 0xf000) >> 12;

	ox =    (ram[1] & 0x01ff);
	xsize = (ram[1] & 0x0e00) >> 9;
	zoomx = (ram[1] & 0xf000) >> 12;

	flipx = BIT(ram[2], 14);
	flipy = BIT(ram[2], 15);
	color = (ram[2] & 0x3f00) >> 8;
	map   = (ram[2] & 0x0001) << 16;

	map  |= (ram[3] & 0xffff);
}


void vsystem_spr_device::common_sprite_drawgfx(bitmap_ind16 &bitmap, const rectangle &cliprect, bitmap_ind8 &priority_bitmap)
{
	gfx_element &sprgfx = *gfx(0);
	uint32_t priority_mask = 0x00;

	m_curr_sprite.oy += m_yoffs;
	m_curr_sprite.ox += m_xoffs;

	if (!m_pri_cb.isnull())
		priority_mask = m_pri_cb(m_curr_sprite.color);

	m_curr_sprite.color &= m_pal_mask;

	m_curr_sprite.zoomx = 32 - m_curr_sprite.zoomx;
	m_curr_sprite.zoomy = 32 - m_curr_sprite.zoomy;

	int const ystart = !m_curr_sprite.flipy ? 0 : m_curr_sprite.ysize;
	int const yend = !m_curr_sprite.flipy ? (m_curr_sprite.ysize + 1) : -1;
	int const yinc = !m_curr_sprite.flipy ? 1 : -1;

	uint32_t const color = m_curr_sprite.color + m_pal_base;
	uint32_t const zx = m_curr_sprite.zoomx << 11;
	uint32_t const zy = m_curr_sprite.zoomy << 11;

	for (int ycnt = ystart; ycnt != yend; ycnt += yinc)
	{
		int const xstart = !m_curr_sprite.flipx ? 0 : m_curr_sprite.xsize;
		int const xend = !m_curr_sprite.flipx ? (m_curr_sprite.xsize + 1) : -1;
		int const xinc = !m_curr_sprite.flipx ? 1 : -1;
		auto const yoffs = m_curr_sprite.oy + ycnt * m_curr_sprite.zoomy/2;

		for (int xcnt = xstart; xcnt != xend; xcnt += xinc)
		{
			uint32_t const startno = m_newtile_cb(m_curr_sprite.map++);
			auto const xoffs = m_curr_sprite.ox + xcnt * m_curr_sprite.zoomx/2;
			if (!m_pri_cb.isnull())
			{
				sprgfx.prio_zoom_transpen(bitmap,cliprect, startno, color, m_curr_sprite.flipx, m_curr_sprite.flipy, xoffs,          yoffs,          zx, zy, priority_bitmap, priority_mask, m_transpen);
				sprgfx.prio_zoom_transpen(bitmap,cliprect, startno, color, m_curr_sprite.flipx, m_curr_sprite.flipy, -0x200 + xoffs, yoffs,          zx, zy, priority_bitmap, priority_mask, m_transpen);
				sprgfx.prio_zoom_transpen(bitmap,cliprect, startno, color, m_curr_sprite.flipx, m_curr_sprite.flipy, xoffs,          -0x200 + yoffs, zx, zy, priority_bitmap, priority_mask, m_transpen);
				sprgfx.prio_zoom_transpen(bitmap,cliprect, startno, color, m_curr_sprite.flipx, m_curr_sprite.flipy, -0x200 + xoffs, -0x200 + yoffs, zx, zy, priority_bitmap, priority_mask, m_transpen);
			}
			else
			{
				sprgfx.zoom_transpen(bitmap,cliprect, startno, color, m_curr_sprite.flipx, m_curr_sprite.flipy, xoffs,          yoffs,          zx, zy, m_transpen);
				sprgfx.zoom_transpen(bitmap,cliprect, startno, color, m_curr_sprite.flipx, m_curr_sprite.flipy, -0x200 + xoffs, yoffs,          zx, zy, m_transpen);
				sprgfx.zoom_transpen(bitmap,cliprect, startno, color, m_curr_sprite.flipx, m_curr_sprite.flipy, xoffs,          -0x200 + yoffs, zx, zy, m_transpen);
				sprgfx.zoom_transpen(bitmap,cliprect, startno, color, m_curr_sprite.flipx, m_curr_sprite.flipy, -0x200 + xoffs, -0x200 + yoffs, zx, zy, m_transpen);
			}
		}
	}

}



void vsystem_spr_device::draw_sprites(uint16_t const *spriteram, int spriteram_bytes, screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int offs;
	int end = 0;

	// find the end of the list
	for (offs = 0; offs < (spriteram_bytes / 16 ); offs++)
	{
		if (BIT(spriteram[offs], 14)) break;
	}
	end = offs;

	// decide our drawing order (if we're using pdrawgfx we must go in reverse)
	int first, last, inc;
	if (!m_pri_cb.isnull())
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
			int const attr_start = 4 * (spriteram[offs] & 0x03ff);

			m_curr_sprite.get(&spriteram[attr_start]);

			common_sprite_drawgfx(bitmap, cliprect, screen.priority());
		}

		offs+=inc;
	}
}
