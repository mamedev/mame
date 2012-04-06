#include "emu.h"
#include "includes/dynduke.h"



/******************************************************************************/

WRITE16_MEMBER(dynduke_state::dynduke_paletteram_w)
{
	int color;

	COMBINE_DATA(&m_generic_paletteram_16[offset]);
	color=m_generic_paletteram_16[offset];
	palette_set_color_rgb(machine(),offset,pal4bit(color >> 0),pal4bit(color >> 4),pal4bit(color >> 8));
}

WRITE16_MEMBER(dynduke_state::dynduke_background_w)
{
	COMBINE_DATA(&m_back_data[offset]);
	m_bg_layer->mark_tile_dirty(offset);
}

WRITE16_MEMBER(dynduke_state::dynduke_foreground_w)
{
	COMBINE_DATA(&m_fore_data[offset]);
	m_fg_layer->mark_tile_dirty(offset);
}

WRITE16_MEMBER(dynduke_state::dynduke_text_w)
{
	UINT16 *videoram = m_videoram;
	COMBINE_DATA(&videoram[offset]);
	m_tx_layer->mark_tile_dirty(offset);
}

static TILE_GET_INFO( get_bg_tile_info )
{
	dynduke_state *state = machine.driver_data<dynduke_state>();
	int tile=state->m_back_data[tile_index];
	int color=tile >> 12;

	tile=tile&0xfff;

	SET_TILE_INFO(
			1,
			tile+state->m_back_bankbase,
			color,
			0);
}

static TILE_GET_INFO( get_fg_tile_info )
{
	dynduke_state *state = machine.driver_data<dynduke_state>();
	int tile=state->m_fore_data[tile_index];
	int color=tile >> 12;

	tile=tile&0xfff;

	SET_TILE_INFO(
			2,
			tile+state->m_fore_bankbase,
			color,
			0);
}

static TILE_GET_INFO( get_tx_tile_info )
{
	dynduke_state *state = machine.driver_data<dynduke_state>();
	UINT16 *videoram = state->m_videoram;
	int tile=videoram[tile_index];
	int color=(tile >> 8) & 0x0f;

	tile = (tile & 0xff) | ((tile & 0xc000) >> 6);

	SET_TILE_INFO(
			0,
			tile,
			color,
			0);
}

VIDEO_START( dynduke )
{
	dynduke_state *state = machine.driver_data<dynduke_state>();
	state->m_bg_layer = tilemap_create(machine, get_bg_tile_info,tilemap_scan_cols,      16,16,32,32);
	state->m_fg_layer = tilemap_create(machine, get_fg_tile_info,tilemap_scan_cols,16,16,32,32);
	state->m_tx_layer = tilemap_create(machine, get_tx_tile_info,tilemap_scan_rows, 8, 8,32,32);

	state->m_fg_layer->set_transparent_pen(15);
	state->m_tx_layer->set_transparent_pen(15);
}

WRITE16_MEMBER(dynduke_state::dynduke_gfxbank_w)
{

	if (ACCESSING_BITS_0_7)
	{
		if (data&0x01) m_back_bankbase=0x1000; else m_back_bankbase=0;
		if (data&0x10) m_fore_bankbase=0x1000; else m_fore_bankbase=0;

		if (m_back_bankbase!=m_old_back)
			m_bg_layer->mark_all_dirty();
		if (m_fore_bankbase!=m_old_fore)
			m_fg_layer->mark_all_dirty();

		m_old_back=m_back_bankbase;
		m_old_fore=m_fore_bankbase;
	}
}


WRITE16_MEMBER(dynduke_state::dynduke_control_w)
{

	if (ACCESSING_BITS_0_7)
	{
		// bit 0x80 toggles, maybe sprite buffering?
		// bit 0x40 is flipscreen
		// bit 0x20 not used?
		// bit 0x10 not used?
		// bit 0x08 is set on the title screen (sprite disable?)
		// bit 0x04 unused? txt disable?
		// bit 0x02 is used on the map screen (fore disable?)
		// bit 0x01 set when inserting coin.. bg disable?

		if (data&0x1) m_back_enable = 0; else m_back_enable = 1;
		if (data&0x2) m_fore_enable=0; else m_fore_enable=1;
		if (data&0x4) m_txt_enable = 0; else m_txt_enable = 1;
		if (data&0x8) m_sprite_enable=0; else m_sprite_enable=1;

		flip_screen_set(machine(), data & 0x40);
	}
}

static void draw_sprites(running_machine &machine, bitmap_ind16 &bitmap,const rectangle &cliprect,int pri)
{
	dynduke_state *state = machine.driver_data<dynduke_state>();
	UINT16 *buffered_spriteram16 = state->m_spriteram->buffer();
	int offs,fx,fy,x,y,color,sprite;

	if (!state->m_sprite_enable) return;

	for (offs = 0x800-4;offs >= 0;offs -= 4)
	{
		/* Don't draw empty sprite table entries */
		if ((buffered_spriteram16[offs+3] >> 8)!=0xf) continue;
		if (((buffered_spriteram16[offs+2]>>13)&3)!=pri) continue;

		fx= buffered_spriteram16[offs+0]&0x2000;
		fy= buffered_spriteram16[offs+0]&0x4000;
		y = buffered_spriteram16[offs+0] & 0xff;
		x = buffered_spriteram16[offs+2] & 0xff;

		if (buffered_spriteram16[offs+2]&0x100) x=0-(0x100-x);

		color = (buffered_spriteram16[offs+0]>>8)&0x1f;
		sprite = buffered_spriteram16[offs+1];
		sprite &= 0x3fff;

		if (flip_screen_get(machine)) {
			x=240-x;
			y=240-y;
			if (fx) fx=0; else fx=1;
			if (fy) fy=0; else fy=1;
		}

		drawgfx_transpen(bitmap,cliprect,machine.gfx[3],
				sprite,
				color,fx,fy,x,y,15);
	}
}

static void draw_background(running_machine &machine, bitmap_ind16 &bitmap, const rectangle &cliprect, int pri )
{
	dynduke_state *state = machine.driver_data<dynduke_state>();
	/* The transparency / palette handling on the background layer is very strange */
	bitmap_ind16 &bm = state->m_bg_layer->pixmap();
	int scrolly, scrollx;
	int x,y;

	/* if we're disabled, don't draw */
	if (!state->m_back_enable)
	{
		bitmap.fill(get_black_pen(machine), cliprect);
		return;
	}

	scrolly = ((state->m_scroll_ram[0x01]&0x30)<<4)+((state->m_scroll_ram[0x02]&0x7f)<<1)+((state->m_scroll_ram[0x02]&0x80)>>7);
	scrollx = ((state->m_scroll_ram[0x09]&0x30)<<4)+((state->m_scroll_ram[0x0a]&0x7f)<<1)+((state->m_scroll_ram[0x0a]&0x80)>>7);

	for (y=0;y<256;y++)
	{
		int realy = (y + scrolly) & 0x1ff;
		UINT16 *src = &bm.pix16(realy);
		UINT16 *dst = &bitmap.pix16(y);


		for (x=0;x<256;x++)
		{
			int realx = (x + scrollx) & 0x1ff;
			UINT16 srcdat = src[realx];

			/* 0x01 - data bits
               0x02
               0x04
               0x08
               0x10 - extra colour bit? (first boss)
               0x20 - priority over sprites
               the old driver also had 'bg_palbase' but I don't see what it's for?
            */

			if ((srcdat & 0x20) == pri)
			{
				if (srcdat & 0x10) srcdat += 0x400;
				//if (srcdat & 0x10) srcdat += machine.rand()&0x1f;

				srcdat = (srcdat & 0x000f) | ((srcdat & 0xffc0) >> 2);
				dst[x] = srcdat;
			}


		}
	}
}

SCREEN_UPDATE_IND16( dynduke )
{
	dynduke_state *state = screen.machine().driver_data<dynduke_state>();
	/* Setup the tilemaps */
	state->m_fg_layer->set_scrolly(0, ((state->m_scroll_ram[0x11]&0x30)<<4)+((state->m_scroll_ram[0x12]&0x7f)<<1)+((state->m_scroll_ram[0x12]&0x80)>>7) );
	state->m_fg_layer->set_scrollx(0, ((state->m_scroll_ram[0x19]&0x30)<<4)+((state->m_scroll_ram[0x1a]&0x7f)<<1)+((state->m_scroll_ram[0x1a]&0x80)>>7) );
	state->m_fg_layer->enable(state->m_fore_enable);
	state->m_tx_layer->enable(state->m_txt_enable);


	draw_background(screen.machine(), bitmap, cliprect,0x00);
	draw_sprites(screen.machine(),bitmap,cliprect,0); // Untested: does anything use it? Could be behind background
	draw_sprites(screen.machine(),bitmap,cliprect,1);
	draw_background(screen.machine(), bitmap, cliprect,0x20);

	draw_sprites(screen.machine(),bitmap,cliprect,2);
	state->m_fg_layer->draw(bitmap, cliprect, 0,0);
	draw_sprites(screen.machine(),bitmap,cliprect,3);
	state->m_tx_layer->draw(bitmap, cliprect, 0,0);

	return 0;
}
