// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
/* Taito TC0180VCU */
// Sprite/Framebuffer emulation by Jarek Burczynski (from taito_b.cpp)

#include "emu.h"
#include "tc0180vcu.h"
#include "screen.h"

//#define TC0180VCU_RAM_SIZE          0x10000
//#define TC0180VCU_SCROLLRAM_SIZE    0x0800

const gfx_layout tc0180vcu_device::charlayout =
{
	8,8,
	RGN_FRAC(1,2),
	4,
	{ 0, 8, RGN_FRAC(1,2), RGN_FRAC(1,2)+8 },
	{ STEP8(0,1) },
	{ STEP8(0,8*2) },
	16*8
};

const gfx_layout tc0180vcu_device::tilelayout =
{
	16,16,
	RGN_FRAC(1,2),
	4,
	{ 0, 8, RGN_FRAC(1,2), RGN_FRAC(1,2)+8 },
	{ STEP8(0,1), STEP8(8*8*2,1) },
	{ STEP8(0,8*2), STEP8(8*8*2*2,8*2) },
	64*8
};

GFXDECODE_MEMBER( tc0180vcu_device::gfxinfo )
	GFXDECODE_DEVICE( DEVICE_SELF, 0, charlayout,  0, 256 )
	GFXDECODE_DEVICE( DEVICE_SELF, 0, tilelayout,  0, 256 )
GFXDECODE_END

void tc0180vcu_device::tc0180vcu_memrw(address_map &map)
{
	map(0x00000, 0x0ffff).ram().w(FUNC(tc0180vcu_device::word_w)).share("vram");
	map(0x10000, 0x1197f).ram().share("spriteram");
	map(0x11980, 0x137ff).ram();
	map(0x13800, 0x13fff).ram().share("scrollram");
	map(0x18000, 0x1801f).ram().w(FUNC(tc0180vcu_device::ctrl_w)).share("ctrl");
	map(0x40000, 0x7ffff).rw(FUNC(tc0180vcu_device::framebuffer_word_r), FUNC(tc0180vcu_device::framebuffer_word_w));
}

DEFINE_DEVICE_TYPE(TC0180VCU, tc0180vcu_device, "tc0180vcu", "Taito TC0180VCU")

tc0180vcu_device::tc0180vcu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, TC0180VCU, tag, owner, clock),
	device_gfx_interface(mconfig, *this, gfxinfo),
	device_video_interface(mconfig, *this),
	m_spriteram(*this, "spriteram"),
	m_vram(*this, "vram"),
	m_scrollram(*this, "scrollram"),
	m_ctrl(*this, "ctrl"),
	//m_bg_rambank(0),
	//m_fg_rambank(0),
	//m_tx_rambank(0),
	m_framebuffer_page(0),
	m_video_control(0),
	m_fb_color_base(0),
	m_bg_color_base(0),
	m_fg_color_base(0),
	m_tx_color_base(0),
	m_inth_callback(*this),
	m_intl_callback(*this),
	m_intl_timer(nullptr)
{
}

//-------------------------------------------------
//  device_resolve_objects - resolve objects that
//  may be needed for other devices to set
//  initial conditions at start time
//-------------------------------------------------

void tc0180vcu_device::device_resolve_objects()
{
	m_inth_callback.resolve_safe();
	m_intl_callback.resolve_safe();
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void tc0180vcu_device::device_start()
{
	m_tilemap[0] = &machine().tilemap().create(*this, tilemap_get_info_delegate(*this, FUNC(tc0180vcu_device::get_bg_tile_info)), TILEMAP_SCAN_ROWS, 16, 16, 64, 64);
	m_tilemap[1] = &machine().tilemap().create(*this, tilemap_get_info_delegate(*this, FUNC(tc0180vcu_device::get_fg_tile_info)), TILEMAP_SCAN_ROWS, 16, 16, 64, 64);
	m_tilemap[2] = &machine().tilemap().create(*this, tilemap_get_info_delegate(*this, FUNC(tc0180vcu_device::get_tx_tile_info)), TILEMAP_SCAN_ROWS,  8,  8, 64, 32);

	m_tilemap[1]->set_transparent_pen(0);
	m_tilemap[2]->set_transparent_pen(0);

	m_framebuffer[0] = std::make_unique<bitmap_ind16>(512, 256);
	m_framebuffer[1] = std::make_unique<bitmap_ind16>(512, 256);

	screen().register_vblank_callback(vblank_state_delegate(&tc0180vcu_device::vblank_callback, this));
	m_intl_timer = timer_alloc(TIMER_INTL);

	save_item(NAME(m_bg_rambank));
	save_item(NAME(m_fg_rambank));
	save_item(NAME(m_tx_rambank));

	save_item(NAME(m_framebuffer_page));

	save_item(NAME(m_video_control));

	save_item(NAME(*m_framebuffer[0]));
	save_item(NAME(*m_framebuffer[1]));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void tc0180vcu_device::device_reset()
{
	int i;

	for (i = 0; i < 0x10; i++)
		m_ctrl[i] = 0;

	m_bg_rambank[0] = 0;
	m_bg_rambank[1] = 0;
	m_fg_rambank[0] = 0;
	m_fg_rambank[1] = 0;
	m_tx_rambank = 0;

	m_framebuffer_page = 0;
	m_video_control = 0;
}

//-------------------------------------------------
//  vblank_callback - generate blanking-related
//  interrupts
//-------------------------------------------------

void tc0180vcu_device::vblank_callback(screen_device &screen, bool state)
{
	// TODO: Measure the actual duty cycle of the INTH/"INT5" (pin 67)
	// and INTL/"INT4" (pin 66) interrupt outputs (both of which are externally
	// encoded and acknowledged through a PAL and additional flip-flops).
	// Is their timing programmable at all? Most registers are accounted for...

	// TODO: Verify that INTH indeed fires before INTL, and not vice versa.

	if (state)
	{
		vblank_update();

		m_inth_callback(ASSERT_LINE);
		m_intl_timer->adjust(screen.time_until_pos(screen.vpos() + 8));
	}
	else
		m_intl_callback(CLEAR_LINE);
}

//-------------------------------------------------
//  device_timer - called whenever a device timer
//  fires
//-------------------------------------------------

void tc0180vcu_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	switch (id)
	{
	case TIMER_INTL:
		m_inth_callback(CLEAR_LINE);
		m_intl_callback(ASSERT_LINE);
		break;
	}
}


/*****************************************************************************
    DEVICE HANDLERS
*****************************************************************************/

/* TC0180VCU control registers:
* offset:
* 0 - -----xxx bg ram page 0 (tile codes)
*     -xxx---- bg ram page 1 (attributes)
* 1 - -----xxx fg ram page 0 (tile codes)
*     -xxx---- fg ram page 1 (attributes)
* 2 - xxxxxxxx number of independent foreground scrolling blocks (see below)
* 3 - xxxxxxxx number of independent background scrolling blocks
* 4 - --xxxxxx text tile bank 0
* 5 - --xxxxxx text tile bank 1
* 6 - ----xxxx text ram page
* 7 - xxxxxxxx video control: pixelram page and enable, screen flip, sprite to foreground priority (see below)
* 8 to f - unused (always zero)
*
******************************************************************************************
*
* offset 6 - text video page register:
*            This location controls which page of video text ram to view
* hitice:
*     0x08 (00001000) - show game text: credits XX, player1 score
*     0x09 (00001001) - show FBI logo
* rambo3:
*     0x08 (00001000) - show game text
*     0x09 (00001001) - show taito logo
*     0x0a (00001010) - used in pair with 0x09 to smooth screen transitions (attract mode)
*
* Is bit 3 (0x08) video text enable/disable ?
*
******************************************************************************************
*
* offset 7 - video control register:
*            bit 0 (0x01) 1 = don't erase sprite frame buffer "after the beam"
*            bit 3 (0x08) sprite to foreground priority
*                         1 = bg, fg, obj, tx
*                         0 = bg, obj1, fg, obj0, tx (obj0/obj1 selected by bit 0 of color code)
*            bit 4 (0x10) screen flip (active HI) (this one is for sure)
*            bit 5 (0x20) could be global video enable switch (Hit the Ice clears this
*                         bit, clears videoram portions and sets this bit)
*            bit 6 (0x40) frame buffer page to show when bit 7 is set
*            bit 7 (0x80) don't flip frame buffer every vblank, use the page selected by bit 6
*
*/

void tc0180vcu_device::video_control( uint8_t data )
{
#if 0
	if (data != m_video_control)
		popmessage("video control = %02x", data);
#endif

	m_video_control = data;

	if (m_video_control & 0x80)
		m_framebuffer_page = BIT(~m_video_control, 6);

	machine().tilemap().set_flip_all((m_video_control & 0x10) ? (TILEMAP_FLIPX | TILEMAP_FLIPY) : 0 );
}

WRITE16_MEMBER( tc0180vcu_device::ctrl_w )
{
	uint16_t oldword = m_ctrl[offset];

	COMBINE_DATA (&m_ctrl[offset]);

	if (oldword ^ m_ctrl[offset])
	{
		if (ACCESSING_BITS_8_15)
		{
			switch(offset)
			{
			case 0:
				m_tilemap[1]->mark_all_dirty();
				m_fg_rambank[0] = (((m_ctrl[offset] >> 8) & 0x0f) << 12);
				m_fg_rambank[1] = (((m_ctrl[offset] >> 12) & 0x0f) << 12);
				break;
			case 1:
				m_tilemap[0]->mark_all_dirty();
				m_bg_rambank[0] = (((m_ctrl[offset] >> 8) & 0x0f) << 12);
				m_bg_rambank[1] = (((m_ctrl[offset] >> 12) & 0x0f) << 12);
				break;
			case 4:
			case 5:
				m_tilemap[2]->mark_all_dirty();
				break;
			case 6:
				m_tilemap[2]->mark_all_dirty();
				m_tx_rambank = (((m_ctrl[offset] >> 8) & 0x0f) << 11);
				break;
			case 7:
				video_control((m_ctrl[offset] >> 8) & 0xff);
				break;
			default:
				break;
			}
		}
	}
}

READ16_MEMBER(tc0180vcu_device::framebuffer_word_r)
{
	int sy = offset >> 8;
	int sx = 2 * (offset & 0xff);

	return (m_framebuffer[sy >> 8]->pix16(sy & 0xff, sx + 0) << 8) | m_framebuffer[sy >> 8]->pix16(sy & 0xff, sx + 1);
}

WRITE16_MEMBER(tc0180vcu_device::framebuffer_word_w)
{
	int sy = offset >> 8;
	int sx = 2 * (offset & 0xff);

	if (ACCESSING_BITS_8_15)
		m_framebuffer[sy >> 8]->pix16(sy & 0xff, sx + 0) = data >> 8;
	if (ACCESSING_BITS_0_7)
		m_framebuffer[sy >> 8]->pix16(sy & 0xff, sx + 1) = data & 0xff;
}

TILE_GET_INFO_MEMBER(tc0180vcu_device::get_bg_tile_info)
{
	int tile  = m_vram[tile_index + m_bg_rambank[0]];
	int color = m_vram[tile_index + m_bg_rambank[1]];

	SET_TILE_INFO_MEMBER(1, tile,
		m_bg_color_base + (color & 0x3f),
		TILE_FLIPYX((color & 0x00c0) >> 6));
}

TILE_GET_INFO_MEMBER(tc0180vcu_device::get_fg_tile_info)
{
	int tile  = m_vram[tile_index + m_fg_rambank[0]];
	int color = m_vram[tile_index + m_fg_rambank[1]];

	SET_TILE_INFO_MEMBER(1, tile,
		m_fg_color_base + (color & 0x3f),
		TILE_FLIPYX((color & 0x00c0) >> 6));
}

TILE_GET_INFO_MEMBER(tc0180vcu_device::get_tx_tile_info)
{
	int tile = m_vram[tile_index + m_tx_rambank];

	SET_TILE_INFO_MEMBER(0,
		(tile & 0x07ff) | ((m_ctrl[4 + ((tile & 0x800) >> 11)]>>8) << 11),
		m_tx_color_base + ((tile >> 12) & 0x0f),
		0);
}

WRITE16_MEMBER( tc0180vcu_device::word_w )
{
	COMBINE_DATA(&m_vram[offset]);

	if ((offset & 0x7000) == m_fg_rambank[0] || (offset & 0x7000) == m_fg_rambank[1])
		m_tilemap[1]->mark_tile_dirty(offset & 0x0fff);

	if ((offset & 0x7000) == m_bg_rambank[0] || (offset & 0x7000) == m_bg_rambank[1])
		m_tilemap[0]->mark_tile_dirty(offset & 0x0fff);

	if ((offset & 0x7800) == m_tx_rambank)
		m_tilemap[2]->mark_tile_dirty(offset & 0x7ff);
}

void tc0180vcu_device::tilemap_draw( screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int tmap_num, int plane )
{
	assert(tmap_num < 3);

	if (tmap_num == 2)
		m_tilemap[2]->draw(screen, bitmap, cliprect, 0, 0);    /* not much to do for tx_tilemap */
	else
	{
		/*plane = 0 fg tilemap*/
		/*plane = 1 bg tilemap*/
		rectangle my_clip;
		int i;
		int scrollx, scrolly;
		int lines_per_block;    /* number of lines scrolled by the same amount (per one scroll value) */
		int number_of_blocks;   /* number of such blocks per _screen_ (256 lines) */

		lines_per_block = 256 - (m_ctrl[2 + plane] >> 8);
		number_of_blocks = 256 / lines_per_block;

		my_clip.min_x = cliprect.min_x;
		my_clip.max_x = cliprect.max_x;

		for (i = 0; i < number_of_blocks; i++)
		{
			scrollx = m_scrollram[plane * 0x200 + i * 2 * lines_per_block];
			scrolly = m_scrollram[plane * 0x200 + i * 2 * lines_per_block + 1];

			my_clip.min_y = i * lines_per_block;
			my_clip.max_y = (i + 1) * lines_per_block - 1;

			if (m_video_control & 0x10)   /*flip screen*/
			{
				my_clip.min_y = bitmap.height() - 1 - (i + 1) * lines_per_block - 1;
				my_clip.max_y = bitmap.height() - 1 - i * lines_per_block;
			}

			my_clip &= cliprect;

			if (my_clip.min_y <= my_clip.max_y)
			{
				m_tilemap[tmap_num]->set_scrollx(0, -scrollx);
				m_tilemap[tmap_num]->set_scrolly(0, -scrolly);
				m_tilemap[tmap_num]->draw(screen, bitmap, my_clip, 0, 0);
			}
		}
	}
}

void tc0180vcu_device::draw_sprites( bitmap_ind16 &bitmap, const rectangle &cliprect )
{
/*  Sprite format: (16 bytes per sprite)
  offs:             bits:
  0000: 0xxxxxxxxxxxxxxx: tile code - 0x0000 to 0x7fff in qzshowby
  0002: 0000000000xxxxxx: color (0x00 - 0x3f)
        x000000000000000: flipy
        0x00000000000000: flipx
        00????????000000: unused ?
  0004: xxxxxx0000000000: doesn't matter - some games (eg nastar) fill this with sign bit, some (eg ashura) do not
        000000xxxxxxxxxx: x-coordinate 10 bits signed (all zero for sprites forming up a big sprite, except for the first one)
  0006: xxxxxx0000000000: doesn't matter - same as x
        000000xxxxxxxxxx: y-coordinate 10 bits signed (same as x)
  0008: xxxxxxxx00000000: sprite x-zoom level
        00000000xxxxxxxx: sprite y-zoom level
      0x00 - non scaled = 100%
      0x80 - scaled to 50%
      0xc0 - scaled to 25%
      0xe0 - scaled to 12.5%
      0xff - scaled to zero pixels size (off)
      Sprite zoom is used in Ashura Blaster just in the beginning
      where you can see a big choplifter and a japanese title.
      This japanese title is a scaled sprite.
      It is used in Crime City also at the end of the third level (in the garage)
      where there are four columns on the sides of the screen
      Heaviest usage is in Rambo 3 - almost every sprite in game is scaled
  000a: xxxxxxxx00000000: x-sprites number (big sprite) decremented by one
        00000000xxxxxxxx: y-sprites number (big sprite) decremented by one
  000c - 000f: unused
*/

	int x, y, xlatch = 0, ylatch = 0, x_no = 0, y_no = 0, x_num = 0, y_num = 0, big_sprite = 0;
	int offs, code, color, flipx, flipy;
	uint32_t data, zoomx, zoomy, zx, zy, zoomxlatch = 0, zoomylatch = 0;

	for (offs = (0x1980 - 16) / 2; offs >=0; offs -= 8)
	{
		code = m_spriteram[offs];

		color = m_spriteram[offs + 1];
		flipx = color & 0x4000;
		flipy = color & 0x8000;
#if 0
		/*check the unknown bits*/
		if (color & 0x3fc0)
		{
			logerror("sprite color (taitob)=%4x ofs=%4x\n", color, offs);
			color = machine().rand() & 0x3f;
		}
#endif
		color = (color & 0x3f) * 16;

		x = m_spriteram[offs + 2] & 0x3ff;
		y = m_spriteram[offs + 3] & 0x3ff;
		if (x >= 0x200)  x -= 0x400;
		if (y >= 0x200)  y -= 0x400;

		data = m_spriteram[offs + 5];
		if (data)
		{
			if (!big_sprite)
			{
				x_num = (data >> 8) & 0xff;
				y_num = (data >> 0) & 0xff;
				x_no  = 0;
				y_no  = 0;
				xlatch = x;
				ylatch = y;
				data = m_spriteram[offs + 4];
				zoomxlatch = (data >> 8) & 0xff;
				zoomylatch = (data >> 0) & 0xff;
				big_sprite = 1;
			}
		}

		data = m_spriteram[offs + 4];
		zoomx = (data >> 8) & 0xff;
		zoomy = (data >> 0) & 0xff;
		zx = (0x100 - zoomx) / 16;
		zy = (0x100 - zoomy) / 16;

		if (big_sprite)
		{
			zoomx = zoomxlatch;
			zoomy = zoomylatch;

			/* Note: like taito_f2.cpp, this zoom implementation is wrong,
			chopped up into 16x16 sections instead of one sprite. This
			is especially visible in rambo3. */

			x = xlatch + (x_no * (0xff - zoomx) + 15) / 16;
			y = ylatch + (y_no * (0xff - zoomy) + 15) / 16;
			zx = xlatch + ((x_no + 1) * (0xff - zoomx) + 15) / 16 - x;
			zy = ylatch + ((y_no + 1) * (0xff - zoomy) + 15) / 16 - y;
			y_no++;

			if (y_no > y_num)
			{
				y_no = 0;
				x_no++;

				if (x_no > x_num)
					big_sprite = 0;
			}
		}

		if ( zoomx || zoomy )
		{
			gfx(1)->zoom_transpen_raw(bitmap,cliprect,
				code,
				color,
				flipx,flipy,
				x,y,
				(zx << 16) / 16,(zy << 16) / 16,0);
		}
		else
		{
			gfx(1)->transpen_raw(bitmap,cliprect,
				code,
				color,
				flipx,flipy,
				x,y,
				0);
		}
	}
}

void tc0180vcu_device::draw_framebuffer( bitmap_ind16 &bitmap, const rectangle &cliprect, int priority )
{
	rectangle myclip = cliprect;
	int x, y;

g_profiler.start(PROFILER_USER1);

	priority <<= 4;

	if (m_video_control & 0x08)
	{
		if (priority)
		{
			g_profiler.stop();
			return;
		}

		if (m_video_control & 0x10)   /*flip screen*/
		{
			/*popmessage("1. X[%3i;%3i] Y[%3i;%3i]", myclip.min_x, myclip.max_x, myclip.min_y, myclip.max_y);*/
			for (y = myclip.min_y; y <= myclip.max_y; y++)
			{
				uint16_t *src = &m_framebuffer[m_framebuffer_page]->pix16(y, myclip.min_x);
				uint16_t *dst;

				dst = &bitmap.pix16(bitmap.height()-1-y, myclip.max_x);

				for (x = myclip.min_x; x <= myclip.max_x; x++)
				{
					uint16_t c = *src++;

					if (c != 0)
						*dst = m_fb_color_base + c;

					dst--;
				}
			}
		}
		else
		{
			for (y = myclip.min_y; y <= myclip.max_y; y++)
			{
				uint16_t *src = &m_framebuffer[m_framebuffer_page]->pix16(y, myclip.min_x);
				uint16_t *dst = &bitmap.pix16(y, myclip.min_x);

				for (x = myclip.min_x; x <= myclip.max_x; x++)
				{
					uint16_t c = *src++;

					if (c != 0)
						*dst = m_fb_color_base + c;

					dst++;
				}
			}
		}
	}
	else
	{
		if (m_video_control & 0x10)   /*flip screen*/
		{
			/*popmessage("3. X[%3i;%3i] Y[%3i;%3i]", myclip.min_x, myclip.max_x, myclip.min_y, myclip.max_y);*/
			for (y = myclip.min_y ;y <= myclip.max_y; y++)
			{
				uint16_t *src = &m_framebuffer[m_framebuffer_page]->pix16(y, myclip.min_x);
				uint16_t *dst;

				dst = &bitmap.pix16(bitmap.height()-1-y, myclip.max_x);

				for (x = myclip.min_x; x <= myclip.max_x; x++)
				{
					uint16_t c = *src++;

					if (c != 0 && (c & 0x10) == priority)
						*dst = m_fb_color_base + c;

					dst--;
				}
			}
		}
		else
		{
			for (y = myclip.min_y; y <= myclip.max_y; y++)
			{
				uint16_t *src = &m_framebuffer[m_framebuffer_page]->pix16(y, myclip.min_x);
				uint16_t *dst = &bitmap.pix16(y, myclip.min_x);

				for (x = myclip.min_x; x <= myclip.max_x; x++)
				{
					uint16_t c = *src++;

					if (c != 0 && (c & 0x10) == priority)
						*dst = m_fb_color_base + c;

					dst++;
				}
			}
		}
	}
g_profiler.stop();
}

void tc0180vcu_device::vblank_update()
{
	if (~m_video_control & 0x01)
		m_framebuffer[m_framebuffer_page]->fill(0, screen().visible_area());

	if (~m_video_control & 0x80)
		m_framebuffer_page ^= 1;

	draw_sprites(*m_framebuffer[m_framebuffer_page], screen().visible_area());
}
