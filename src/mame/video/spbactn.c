
#include "emu.h"
#include "includes/spbactn.h"


static void blendbitmaps(palette_device &palette,
		bitmap_rgb32 &dest,bitmap_ind16 &src1,bitmap_ind16 &src2,
		const rectangle &cliprect)
{
	int y,x;
	const pen_t *paldata = palette.pens();

	for (y = cliprect.min_y; y <= cliprect.max_y; y++)
	{
		UINT32 *dd  = &dest.pix32(y);
		UINT16 *sd1 = &src1.pix16(y);
		UINT16 *sd2 = &src2.pix16(y);

		for (x = cliprect.min_x; x <= cliprect.max_x; x++)
		{
			if (sd2[x])
			{
				if (sd2[x] & 0x1000)
					dd[x] = paldata[sd1[x] & 0x07ff] + paldata[sd2[x]];
				else
					dd[x] = paldata[sd2[x]];
			}
			else
				dd[x] = paldata[sd1[x]];
		}
	}
}



WRITE16_MEMBER(spbactn_state::bg_videoram_w)
{
	COMBINE_DATA(&m_bgvideoram[offset]);
	m_bg_tilemap->mark_tile_dirty(offset&0x1fff);
}

TILE_GET_INFO_MEMBER(spbactn_state::get_bg_tile_info)
{
	int attr = m_bgvideoram[tile_index];
	int tileno = m_bgvideoram[tile_index+0x2000];
	SET_TILE_INFO_MEMBER(1, tileno, ((attr & 0x00f0)>>4)+0x80, 0);
}


WRITE16_MEMBER(spbactn_state::fg_videoram_w)
{
	COMBINE_DATA(&m_fgvideoram[offset]);
	m_fg_tilemap->mark_tile_dirty(offset&0x1fff);
}

TILE_GET_INFO_MEMBER(spbactn_state::get_fg_tile_info)
{
	int attr = m_fgvideoram[tile_index];
	int tileno = m_fgvideoram[tile_index+0x2000];

	int color = ((attr & 0x00f0)>>4);

	/* blending */
	if (attr & 0x0008)
		color += 0x00f0;
	else
		color |= 0x0080;

	SET_TILE_INFO_MEMBER(0, tileno, color, 0);
}



VIDEO_START_MEMBER(spbactn_state,spbactn)
{
	/* allocate bitmaps */
	m_screen->register_screen_bitmap(m_tile_bitmap_bg);
	m_screen->register_screen_bitmap(m_tile_bitmap_fg);

	m_bg_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(spbactn_state::get_bg_tile_info),this), TILEMAP_SCAN_ROWS, 16, 8, 64, 128);
	m_fg_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(spbactn_state::get_fg_tile_info),this), TILEMAP_SCAN_ROWS, 16, 8, 64, 128);
	m_bg_tilemap->set_transparent_pen(0);
	m_fg_tilemap->set_transparent_pen(0);

}

VIDEO_START_MEMBER(spbactn_state,spbactnp)
{
	VIDEO_START_CALL_MEMBER(spbactn);
	// no idea..
	m_extra_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(spbactn_state::get_extra_tile_info),this), TILEMAP_SCAN_ROWS, 8, 8, 16, 16);
}
WRITE16_MEMBER( spbactn_state::spbatnp_90002_w )
{
	//printf("spbatnp_90002_w %04x\n",data);
}

WRITE16_MEMBER( spbactn_state::spbatnp_90006_w )
{
	//printf("spbatnp_90006_w %04x\n",data);
}


WRITE16_MEMBER( spbactn_state::spbatnp_9000c_w )
{
	//printf("spbatnp_9000c_w %04x\n",data);
}

WRITE16_MEMBER( spbactn_state::spbatnp_9000e_w )
{
	//printf("spbatnp_9000e_w %04x\n",data);
}

WRITE16_MEMBER( spbactn_state::spbatnp_9000a_w )
{
	//printf("spbatnp_9000a_w %04x\n",data);
}

WRITE16_MEMBER( spbactn_state::spbatnp_90124_w )
{
	//printf("spbatnp_90124_w %04x\n",data);
	m_bg_tilemap->set_scrolly(0, data);

}

WRITE16_MEMBER( spbactn_state::spbatnp_9012c_w )
{
	//printf("spbatnp_9012c_w %04x\n",data);
	m_bg_tilemap->set_scrollx(0, data);
}


WRITE8_MEMBER(spbactn_state::extraram_w)
{
	COMBINE_DATA(&m_extraram[offset]);
	m_extra_tilemap->mark_tile_dirty(offset/2);
}

TILE_GET_INFO_MEMBER(spbactn_state::get_extra_tile_info)
{
	int tileno = m_extraram[(tile_index*2)+1];
	tileno |= m_extraram[(tile_index*2)] << 8;
	SET_TILE_INFO_MEMBER(3, tileno, 0, 0);
}




int spbactn_state::draw_video(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect, bool alt_sprites)
{
	m_tile_bitmap_fg.fill(0, cliprect);

	m_bg_tilemap->draw(screen, m_tile_bitmap_bg, cliprect, TILEMAP_DRAW_OPAQUE, 0);



	if (spbactn_draw_sprites(screen, m_gfxdecode, m_tile_bitmap_bg, cliprect, 0, alt_sprites, m_spvideoram))
	{
		m_bg_tilemap->draw(screen, m_tile_bitmap_bg, cliprect, 0, 0);
	}

	spbactn_draw_sprites(screen, m_gfxdecode, m_tile_bitmap_bg, cliprect, 1, alt_sprites, m_spvideoram);

	m_fg_tilemap->draw(screen, m_tile_bitmap_fg, cliprect, 0, 0);


	spbactn_draw_sprites(screen, m_gfxdecode, m_tile_bitmap_fg, cliprect, 2, alt_sprites, m_spvideoram);
	spbactn_draw_sprites(screen, m_gfxdecode, m_tile_bitmap_fg, cliprect, 3, alt_sprites, m_spvideoram);

	/* mix & blend the tilemaps and sprites into a 32-bit bitmap */
	blendbitmaps(m_palette, bitmap, m_tile_bitmap_bg, m_tile_bitmap_fg, cliprect);
	return 0;
}

UINT32 spbactn_state::screen_update_spbactn(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	return draw_video(screen,bitmap,cliprect,false);
}

UINT32 spbactn_state::screen_update_spbactnp(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	// hack to make the extra cpu do something..
	m_extraram2[0x104] = machine().rand();
	m_extraram2[0x105] = machine().rand();

	return draw_video(screen,bitmap,cliprect,true);
}
