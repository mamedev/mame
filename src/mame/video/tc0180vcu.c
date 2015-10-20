// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
/* Taito TC0180VCU */

#include "emu.h"
#include "tc0180vcu.h"

#define TC0180VCU_RAM_SIZE          0x10000
#define TC0180VCU_SCROLLRAM_SIZE    0x0800


const device_type TC0180VCU = &device_creator<tc0180vcu_device>;

tc0180vcu_device::tc0180vcu_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, TC0180VCU, "Taito TC0180VCU", tag, owner, clock, "tc0180vcu", __FILE__),
	m_ram(NULL),
	//m_scrollram(NULL),
	//m_bg_rambank(0),
	//m_fg_rambank(0),
	//m_tx_rambank(0),
	m_framebuffer_page(0),
	m_video_control(0),
	m_bg_color_base(0),
	m_fg_color_base(0),
	m_tx_color_base(0),
	m_gfxdecode(*this)
{
}

//-------------------------------------------------
//  static_set_gfxdecode_tag: Set the tag of the
//  gfx decoder
//-------------------------------------------------

void tc0180vcu_device::static_set_gfxdecode_tag(device_t &device, const char *tag)
{
	downcast<tc0180vcu_device &>(device).m_gfxdecode.set_tag(tag);
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void tc0180vcu_device::device_start()
{
	if(!m_gfxdecode->started())
		throw device_missing_dependencies();

	m_tilemap[0] = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(tc0180vcu_device::get_bg_tile_info),this), TILEMAP_SCAN_ROWS, 16, 16, 64, 64);
	m_tilemap[1] = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(tc0180vcu_device::get_fg_tile_info),this), TILEMAP_SCAN_ROWS, 16, 16, 64, 64);
	m_tilemap[2] = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(tc0180vcu_device::get_tx_tile_info),this), TILEMAP_SCAN_ROWS, 8, 8, 64, 32);

	m_tilemap[1]->set_transparent_pen(0);
	m_tilemap[2]->set_transparent_pen(0);

	m_ram = auto_alloc_array_clear(machine(), UINT16, TC0180VCU_RAM_SIZE / 2);
	m_scrollram = auto_alloc_array_clear(machine(), UINT16, TC0180VCU_SCROLLRAM_SIZE / 2);

	save_pointer(NAME(m_ram), TC0180VCU_RAM_SIZE / 2);
	save_pointer(NAME(m_scrollram), TC0180VCU_SCROLLRAM_SIZE / 2);

	save_item(NAME(m_bg_rambank));
	save_item(NAME(m_fg_rambank));
	save_item(NAME(m_tx_rambank));

	save_item(NAME(m_framebuffer_page));

	save_item(NAME(m_video_control));
	save_item(NAME(m_ctrl));
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

READ8_MEMBER( tc0180vcu_device::get_fb_page )
{
	return m_framebuffer_page;
}

WRITE8_MEMBER( tc0180vcu_device::set_fb_page )
{
	m_framebuffer_page = data;
}

READ8_MEMBER( tc0180vcu_device::get_videoctrl )
{
	return m_video_control;
}

void tc0180vcu_device::video_control( UINT8 data )
{
#if 0
	if (data != m_video_control)
		popmessage("video control = %02x", data);
#endif

	m_video_control = data;

	if (m_video_control & 0x80)
		m_framebuffer_page = (~m_video_control & 0x40) >> 6;

	machine().tilemap().set_flip_all((m_video_control & 0x10) ? (TILEMAP_FLIPX | TILEMAP_FLIPY) : 0 );
}

READ16_MEMBER( tc0180vcu_device::ctrl_r )
{
	return m_ctrl[offset];
}

WRITE16_MEMBER( tc0180vcu_device::ctrl_w )
{
	UINT16 oldword = m_ctrl[offset];

	COMBINE_DATA (&m_ctrl[offset]);

	if (oldword != m_ctrl[offset])
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

TILE_GET_INFO_MEMBER(tc0180vcu_device::get_bg_tile_info)
{
	int tile  = m_ram[tile_index + m_bg_rambank[0]];
	int color = m_ram[tile_index + m_bg_rambank[1]];

	SET_TILE_INFO_MEMBER(1, tile,
		m_bg_color_base + (color & 0x3f),
		TILE_FLIPYX((color & 0x00c0) >> 6));
}

TILE_GET_INFO_MEMBER(tc0180vcu_device::get_fg_tile_info)
{
	int tile  = m_ram[tile_index + m_fg_rambank[0]];
	int color = m_ram[tile_index + m_fg_rambank[1]];

	SET_TILE_INFO_MEMBER(1, tile,
		m_fg_color_base + (color & 0x3f),
		TILE_FLIPYX((color & 0x00c0) >> 6));
}

TILE_GET_INFO_MEMBER(tc0180vcu_device::get_tx_tile_info)
{
	int tile = m_ram[tile_index + m_tx_rambank];

	SET_TILE_INFO_MEMBER(0,
		(tile & 0x07ff) | ((m_ctrl[4 + ((tile & 0x800) >> 11)]>>8) << 11),
		m_tx_color_base + ((tile >> 12) & 0x0f),
		0);
}

READ16_MEMBER( tc0180vcu_device::scroll_r )
{
	return m_scrollram[offset];
}

WRITE16_MEMBER( tc0180vcu_device::scroll_w )
{
	COMBINE_DATA(&m_scrollram[offset]);
}

READ16_MEMBER( tc0180vcu_device::word_r )
{
	return m_ram[offset];
}

WRITE16_MEMBER( tc0180vcu_device::word_w )
{
	COMBINE_DATA(&m_ram[offset]);

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
