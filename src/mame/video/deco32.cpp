// license:BSD-3-Clause
// copyright-holders:Bryan McPhail
#include "emu.h"
#include "includes/deco32.h"

/******************************************************************************/

WRITE32_MEMBER(deco32_state::pri_w)
{
	m_pri=data;
}

WRITE32_MEMBER(dragngun_state::sprite_control_w)
{
	m_sprite_ctrl=data;
}

WRITE32_MEMBER(dragngun_state::spriteram_dma_w)
{
	/* DMA spriteram to private sprite chip area, and clear cpu ram */
	m_spriteram->copy();
	memset(m_spriteram->live(),0,0x2000);
}

WRITE32_MEMBER(deco32_state::ace_ram_w)
{
	/* Some notes pieced together from Tattoo Assassins info:

	    Bytes 0 to 0x58 - object alpha control?
	    Bytes 0x5c to 0x7c - tilemap alpha control

	    0 = opaque, 0x10 = 50% transparent, 0x20 = fully transparent

	    Byte 0x00: ACEO000P0
	                        P8
	                        1P0
	                        1P8
	                        O010C1
	                        o010C8
	                        ??

	    Hardware fade registers:

	    Byte 0x80: fadeptred
	    Byte 0x84: fadeptgreen
	    Byte 0x88: fadeptblue
	    Byte 0x8c: fadestred
	    Byte 0x90: fadestgreen
	    Byte 0x94: fadestblue
	    Byte 0x98: fadetype

	    The 'ST' value lerps between the 'PT' value and the palette entries.  So, if PT==0,
	    then ST ranging from 0 to 255 will cause a fade to black (when ST==255 the palette
	    becomes zero).

	    'fadetype' - 1100 for multiplicative fade, 1000 for additive
	*/
	if (offset>=(0x80/4) && (data!=m_ace_ram[offset]))
		m_ace_ram_dirty=1;

	COMBINE_DATA(&m_ace_ram[offset]);
}

void deco32_state::updateAceRam()
{
	int r,g,b,i;
	UINT8 fadeptr=m_ace_ram[0x20];
	UINT8 fadeptg=m_ace_ram[0x21];
	UINT8 fadeptb=m_ace_ram[0x22];
	UINT8 fadepsr=m_ace_ram[0x23];
	UINT8 fadepsg=m_ace_ram[0x24];
	UINT8 fadepsb=m_ace_ram[0x25];
//  UINT8 mode=m_ace_ram[0x26];

	m_ace_ram_dirty=0;

	for (i=0; i<2048; i++)
	{
		/* Lerp palette entry to 'fadept' according to 'fadeps' */
		b = (m_generic_paletteram_32[i] >>16) & 0xff;
		g = (m_generic_paletteram_32[i] >> 8) & 0xff;
		r = (m_generic_paletteram_32[i] >> 0) & 0xff;

		if (i>255) /* Screenshots seem to suggest ACE fades do not affect playfield 1 palette (0-255) */
		{
			/* Yeah, this should really be fixed point, I know */
			b = (UINT8)((float)b + (((float)fadeptb - (float)b) * (float)fadepsb/255.0f));
			g = (UINT8)((float)g + (((float)fadeptg - (float)g) * (float)fadepsg/255.0f));
			r = (UINT8)((float)r + (((float)fadeptr - (float)r) * (float)fadepsr/255.0f));
		}

		m_palette->set_pen_color(i,rgb_t(r,g,b));
	}
}

/******************************************************************************/

/* Later games have double buffered paletteram - the real palette ram is
only updated on a DMA call */

WRITE32_MEMBER(deco32_state::nonbuffered_palette_w)
{
	int r,g,b;

	COMBINE_DATA(&m_generic_paletteram_32[offset]);

	b = (m_generic_paletteram_32[offset] >>16) & 0xff;
	g = (m_generic_paletteram_32[offset] >> 8) & 0xff;
	r = (m_generic_paletteram_32[offset] >> 0) & 0xff;

	m_palette->set_pen_color(offset,rgb_t(r,g,b));
}

WRITE32_MEMBER(deco32_state::buffered_palette_w)
{
	COMBINE_DATA(&m_generic_paletteram_32[offset]);
	m_dirty_palette[offset]=1;
}

WRITE32_MEMBER(deco32_state::palette_dma_w)
{
	const int m=m_palette->entries();
	int r,g,b,i;

	for (i=0; i<m; i++) {
		if (m_dirty_palette[i]) {
			m_dirty_palette[i]=0;

			if (m_has_ace_ram)
			{
				m_ace_ram_dirty=1;
			}
			else
			{
				b = (m_generic_paletteram_32[i] >>16) & 0xff;
				g = (m_generic_paletteram_32[i] >> 8) & 0xff;
				r = (m_generic_paletteram_32[i] >> 0) & 0xff;

				m_palette->set_pen_color(i,rgb_t(r,g,b));
			}
		}
	}
}


/******************************************************************************/

void deco32_state::video_start()
{
	save_item(NAME(m_pri));
	save_item(NAME(m_spriteram16));
	save_item(NAME(m_spriteram16_buffered));
	save_item(NAME(m_pf1_rowscroll));
	save_item(NAME(m_pf2_rowscroll));
	save_item(NAME(m_pf3_rowscroll));
	save_item(NAME(m_pf4_rowscroll));
}

VIDEO_START_MEMBER(deco32_state,captaven)
{
	m_has_ace_ram=0;

	deco32_state::video_start();
}

VIDEO_START_MEMBER(deco32_state,fghthist)
{
	m_dirty_palette = std::make_unique<UINT8[]>(4096);
	m_sprgen->alloc_sprite_bitmap();
	m_has_ace_ram=0;

	save_pointer(NAME(m_dirty_palette.get()), 4096);
	deco32_state::video_start();
}

VIDEO_START_MEMBER(deco32_state,nslasher)
{
	int width, height;
	m_dirty_palette = std::make_unique<UINT8[]>(4096);
	width = m_screen->width();
	height = m_screen->height();
	m_tilemap_alpha_bitmap=std::make_unique<bitmap_ind16>(width, height );
	m_sprgen1->alloc_sprite_bitmap();
	m_sprgen2->alloc_sprite_bitmap();
	memset(m_dirty_palette.get(),0,4096);
	m_has_ace_ram=1;

	save_pointer(NAME(m_dirty_palette.get()), 4096);
	save_item(NAME(m_ace_ram_dirty));
	save_item(NAME(m_spriteram16_2));
	save_item(NAME(m_spriteram16_2_buffered));

	deco32_state::video_start();
}

void dragngun_state::video_start()
{
	save_item(NAME(m_pf1_rowscroll));
	save_item(NAME(m_pf2_rowscroll));
	save_item(NAME(m_pf3_rowscroll));
	save_item(NAME(m_pf4_rowscroll));
}

VIDEO_START_MEMBER(dragngun_state,dragngun)
{
	m_dirty_palette = std::make_unique<UINT8[]>(4096);
	m_screen->register_screen_bitmap(m_temp_render_bitmap);

	memset(m_dirty_palette.get(),0,4096);

	m_has_ace_ram=0;

	save_item(NAME(m_sprite_ctrl));
	save_pointer(NAME(m_dirty_palette.get()), 4096);
}

VIDEO_START_MEMBER(dragngun_state,lockload)
{
	m_dirty_palette = std::make_unique<UINT8[]>(4096);
	m_screen->register_screen_bitmap(m_temp_render_bitmap);

	memset(m_dirty_palette.get(),0,4096);

	m_has_ace_ram=0;

	save_item(NAME(m_sprite_ctrl));
	save_pointer(NAME(m_dirty_palette.get()), 4096);
}
/******************************************************************************/


/******************************************************************************/

UINT32 deco32_state::screen_update_captaven(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	address_space &space = machine().driver_data()->generic_space();
	UINT16 flip = m_deco_tilegen1->pf_control_r(space, 0, 0xffff);
	flip_screen_set(BIT(flip, 7));

	screen.priority().fill(0, cliprect);
	bitmap.fill(m_palette->pen(0x000), cliprect); // Palette index not confirmed

	m_deco_tilegen2->set_pf1_8bpp_mode(1);

	m_deco_tilegen1->pf_update(m_pf1_rowscroll, m_pf2_rowscroll);
	m_deco_tilegen2->pf_update(m_pf3_rowscroll, m_pf4_rowscroll);

	// pf4 not used (because pf3 is in 8bpp mode)

	if ((m_pri&1)==0)
	{
		m_deco_tilegen2->tilemap_1_draw(screen, bitmap, cliprect, 0, 1);
		m_deco_tilegen1->tilemap_2_draw(screen, bitmap, cliprect, 0, 2);
	}
	else
	{
		m_deco_tilegen1->tilemap_2_draw(screen, bitmap, cliprect, 0, 1);
		m_deco_tilegen2->tilemap_1_draw(screen, bitmap, cliprect, 0, 2);
	}

	m_deco_tilegen1->tilemap_1_draw(screen, bitmap, cliprect, 0, 4);

	m_sprgen->set_alt_format(true);
	m_sprgen->draw_sprites(bitmap, cliprect, m_spriteram16_buffered, 0x400);

	return 0;
}

UINT32 dragngun_state::screen_update_dragngun(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	screen.priority().fill(0, cliprect);
	bitmap.fill(m_palette->black_pen(), cliprect);

	m_deco_tilegen1->pf_update(m_pf1_rowscroll, m_pf2_rowscroll);
	m_deco_tilegen2->pf_update(m_pf3_rowscroll, m_pf4_rowscroll);

	//m_deco_tilegen1->set_pf3_8bpp_mode(1); // despite being 8bpp this doesn't require the same shifting as captaven, why not?

	m_deco_tilegen2->tilemap_2_draw(screen, bitmap, cliprect, 0, 1); // it uses pf3 in 8bpp mode instead, like captaven
	m_deco_tilegen2->tilemap_1_draw(screen, bitmap, cliprect, 0, 2);
	m_deco_tilegen1->tilemap_2_draw(screen, bitmap, cliprect, 0, 4);
	m_deco_tilegen1->tilemap_1_draw(screen, bitmap, cliprect, 0, 8);

	// zooming sprite draw is very slow, and sprites are buffered.. however, one of the levels attempts to use
	// partial updates for every line, which causes things to be very slow... the sprites appear to support
	// multiple layers of alpha, so rendering to a buffer for layer isn't easy (maybe there are multiple sprite
	// chips at work?)
	//
	// really, it needs optimizing ..
	// so for now we only draw these 2 layers on the last update call
	if (cliprect.max_y == 247)
	{
		rectangle clip(cliprect.min_x, cliprect.max_x, 8, 247);

		m_sprgenzoom->dragngun_draw_sprites(bitmap,clip,m_spriteram->buffer(), m_sprite_layout_0_ram, m_sprite_layout_1_ram, m_sprite_lookup_0_ram, m_sprite_lookup_1_ram, m_sprite_ctrl, screen.priority(), m_temp_render_bitmap );

	}

	return 0;
}


UINT32 deco32_state::screen_update_fghthist(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	screen.priority().fill(0, cliprect);
	bitmap.fill(m_palette->pen(0x300), cliprect); // Palette index not confirmed

	m_deco_tilegen1->pf_update(m_pf1_rowscroll, m_pf2_rowscroll);
	m_deco_tilegen2->pf_update(m_pf3_rowscroll, m_pf4_rowscroll);

	m_sprgen->draw_sprites(bitmap, cliprect, m_spriteram16_buffered, 0x800, true);

	/* Draw screen */
	m_deco_tilegen2->tilemap_2_draw(screen, bitmap, cliprect, 0, 1);

	if(m_pri&1)
	{
		m_deco_tilegen1->tilemap_2_draw(screen, bitmap, cliprect, 0, 2);
		m_sprgen->inefficient_copy_sprite_bitmap(bitmap, cliprect, 0x0800, 0x0800, 1024, 0x1ff);
		m_deco_tilegen2->tilemap_1_draw(screen, bitmap, cliprect, 0, 4);
	}
	else
	{
		m_deco_tilegen2->tilemap_1_draw(screen, bitmap, cliprect, 0, 2);
		m_sprgen->inefficient_copy_sprite_bitmap(bitmap, cliprect, 0x0800, 0x0800, 1024, 0x1ff);
		m_deco_tilegen1->tilemap_2_draw(screen, bitmap, cliprect, 0, 4);
	}

	m_sprgen->inefficient_copy_sprite_bitmap(bitmap, cliprect, 0x0000, 0x0800, 1024, 0x1ff);

	m_deco_tilegen1->tilemap_1_draw(screen, bitmap, cliprect, 0, 0);
	return 0;
}

/*
    This function mimics the priority PROM/circuit on the pcb.  It takes
    the tilemaps & sprite bitmaps as inputs, and outputs a final pixel
    based on alpha & priority values.  Rendering sprites to temporary
    bitmaps is the only reasonable way to implement proper priority &
    blending support - it can't be done in-place on the final framebuffer
    without a lot of support bitmaps.
*/
void deco32_state::mixDualAlphaSprites(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect, gfx_element *gfx0, gfx_element *gfx1, int mixAlphaTilemap)
{
	const pen_t *pens = m_palette->pens();
	const pen_t *pal0 = &pens[gfx0->colorbase()];
	const pen_t *pal1 = &pens[gfx1->colorbase()];
	const pen_t *pal2 = &pens[m_gfxdecode->gfx((m_pri&1) ? 1 : 2)->colorbase()];
	int x,y;
	bitmap_ind16& sprite0_mix_bitmap = machine().device<decospr_device>("spritegen1")->get_sprite_temp_bitmap();
	bitmap_ind16& sprite1_mix_bitmap = machine().device<decospr_device>("spritegen2")->get_sprite_temp_bitmap();


	/* Mix sprites into main bitmap, based on priority & alpha */
	for (y=8; y<248; y++) {
		UINT8* tilemapPri=&screen.priority().pix8(y);
		UINT16* sprite0=&sprite0_mix_bitmap.pix16(y);
		UINT16* sprite1=&sprite1_mix_bitmap.pix16(y);
		UINT32* destLine=&bitmap.pix32(y);
		UINT16* alphaTilemap=&m_tilemap_alpha_bitmap->pix16(y);

		for (x=0; x<320; x++) {
			UINT16 priColAlphaPal0=sprite0[x];
			UINT16 priColAlphaPal1=sprite1[x];
			UINT16 pri0=(priColAlphaPal0&0x6000)>>13;
			UINT16 pri1=(priColAlphaPal1&0x6000)>>13;
			UINT16 col0=((priColAlphaPal0&0x1f00)>>8) % gfx0->colors();
			UINT16 col1=((priColAlphaPal1&0x0f00)>>8) % gfx1->colors();
			UINT16 alpha1=priColAlphaPal1&0x8000;

			// Apply sprite bitmap 0 according to priority rules
			if ((priColAlphaPal0&0xff)!=0)
			{
				/*
				    Sprite 0 priority rules:

				    0 = Sprite above all layers
				    1 = Sprite under top playfield
				    2 = Sprite under top two playfields
				    3 = Sprite under all playfields
				*/
				if ((pri0&0x3)==0 || (pri0&0x3)==1 || ((pri0&0x3)==2 && mixAlphaTilemap)) // Spri0 on top of everything, or under alpha playfield
				{
					destLine[x]=pal0[(priColAlphaPal0&0xff) + (gfx0->granularity() * col0)];
				}
				else if ((pri0&0x3)==2) // Spri0 under top playfield
				{
					if (tilemapPri[x]<4)
						destLine[x]=pal0[(priColAlphaPal0&0xff) + (gfx0->granularity() * col0)];
				}
				else // Spri0 under top & middle playfields
				{
					if (tilemapPri[x]<2)
						destLine[x]=pal0[(priColAlphaPal0&0xff) + (gfx0->granularity() * col0)];
				}
			}

			// Apply sprite bitmap 1 according to priority rules
			if ((priColAlphaPal1&0xff)!=0)
			{
				// Apply alpha for this pixel based on Ace setting
				if (alpha1)
				{
					/*
					    Alpha rules:

					    Pri 0 - Over all tilemaps, but under sprite 0 pri 0, pri 1, pri 2
					    Pri 1 -
					    Pri 2 -
					    Pri 3 -
					*/

					/* Alpha values are tied to ACE ram... */
					//int alpha=((m_ace_ram[0x0 + (((priColAlphaPal1&0xf0)>>4)/2)]) * 8)-1;
					//if (alpha<0)
					//  alpha=0;

					/* I don't really understand how object ACE ram is really hooked up,
					    the only obvious place in Night Slashers is the stagecoach in level 2 */

					if (pri1==0 && (((priColAlphaPal0&0xff)==0 || ((pri0&0x3)!=0 && (pri0&0x3)!=1 && (pri0&0x3)!=2))))
					{
						if ((m_pri&1)==0 || ((m_pri&1)==1 && tilemapPri[x]<4) || ((m_pri&1)==1 && mixAlphaTilemap))
							destLine[x]=alpha_blend_r32(destLine[x], pal1[(priColAlphaPal1&0xff) + (gfx1->granularity() * col1)], 0x80);
					}
					else if (pri1==1 && ((priColAlphaPal0&0xff)==0 || ((pri0&0x3)!=0 && (pri0&0x3)!=1 && (pri0&0x3)!=2)))
						destLine[x]=alpha_blend_r32(destLine[x], pal1[(priColAlphaPal1&0xff) + (gfx1->granularity() * col1)], 0x80);
					else if (pri1==2)// TOdo
						destLine[x]=alpha_blend_r32(destLine[x], pal1[(priColAlphaPal1&0xff) + (gfx1->granularity() * col1)], 0x80);
					else if (pri1==3)// TOdo
						destLine[x]=alpha_blend_r32(destLine[x], pal1[(priColAlphaPal1&0xff) + (gfx1->granularity() * col1)], 0x80);
				}
				else
				{
					/*
					    Non alpha rules:

					    Pri 0 - Under sprite 0 pri 0, over all tilemaps
					*/
					if (pri1==0 && ((priColAlphaPal0&0xff)==0 || ((pri0&0x3)!=0)))
						destLine[x]=pal1[(priColAlphaPal1&0xff) + (gfx1->granularity() * col1)];
					else if (pri1==1) // todo
						destLine[x]=pal1[(priColAlphaPal1&0xff) + (gfx1->granularity() * col1)];
					else if (pri1==2) // todo
						destLine[x]=pal1[(priColAlphaPal1&0xff) + (gfx1->granularity() * col1)];
					else if (pri1==3) // todo
						destLine[x]=pal1[(priColAlphaPal1&0xff) + (gfx1->granularity() * col1)];
				}
			}

			/* Optionally mix in alpha tilemap */
			if (mixAlphaTilemap)
			{
				UINT16 p=alphaTilemap[x];
				if (p&0xf)
				{
					/* Alpha tilemap under top two sprite 0 priorities */
					if (((priColAlphaPal0&0xff)==0 || (pri0&0x3)==2 || (pri0&0x3)==3)
						&& ((priColAlphaPal1&0xff)==0 || (pri1&0x3)==2 || (pri1&0x3)==3 || alpha1))
					{
						/* Alpha values are tied to ACE ram */
						int alpha=((m_ace_ram[0x17 + (((p&0xf0)>>4)/2)]) * 8)-1;
						if (alpha<0)
							alpha=0;

						destLine[x]=alpha_blend_r32(destLine[x], pal2[p], 255-alpha);
					}
				}
			}
		}
	}
}

UINT32 deco32_state::screen_update_nslasher(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	int alphaTilemap=0;
	m_deco_tilegen1->pf_update(m_pf1_rowscroll, m_pf2_rowscroll);
	m_deco_tilegen2->pf_update(m_pf3_rowscroll, m_pf4_rowscroll);

	/* This is not a conclusive test for deciding if tilemap needs alpha blending */
	if (m_ace_ram[0x17]!=0x0 && m_pri)
		alphaTilemap=1;

	if (m_ace_ram_dirty)
		updateAceRam();

	screen.priority().fill(0, cliprect);

	bitmap.fill(m_palette->pen(0x200), cliprect);

	/* Draw sprites to temporary bitmaps, saving alpha & priority info for later mixing */
	m_sprgen1->set_pix_raw_shift(8);
	m_sprgen2->set_pix_raw_shift(8);

	m_sprgen1->draw_sprites(bitmap, cliprect, m_spriteram16_buffered, 0x800, true);
	m_sprgen2->draw_sprites(bitmap, cliprect, m_spriteram16_2_buffered, 0x800, true);


	/* Render alpha-blended tilemap to separate buffer for proper mixing */
	m_tilemap_alpha_bitmap->fill(0, cliprect);

	/* Draw playfields & sprites */
	if (m_pri&2)
	{
		m_deco_tilegen2->tilemap_12_combine_draw(screen, bitmap, cliprect, 0, 1, 1);
		m_deco_tilegen1->tilemap_2_draw(screen, bitmap, cliprect, 0, 4);
	}
	else
	{
		m_deco_tilegen2->tilemap_2_draw(screen, bitmap, cliprect, 0, 1);
		if (m_pri&1)
		{
			m_deco_tilegen1->tilemap_2_draw(screen, bitmap, cliprect, 0, 2);
			if (alphaTilemap)
				m_deco_tilegen2->tilemap_1_draw(screen, *m_tilemap_alpha_bitmap, cliprect, 0, 4);
			else
				m_deco_tilegen2->tilemap_1_draw(screen, bitmap, cliprect, 0, 4);
		}
		else
		{
			m_deco_tilegen2->tilemap_1_draw(screen, bitmap, cliprect, 0, 2);
			if (alphaTilemap)
				m_deco_tilegen1->tilemap_2_draw(screen, *m_tilemap_alpha_bitmap, cliprect, 0, 4);
			else
				m_deco_tilegen1->tilemap_2_draw(screen, bitmap, cliprect, 0, 4);
		}
	}

	mixDualAlphaSprites(screen, bitmap, cliprect, m_gfxdecode->gfx(3), m_gfxdecode->gfx(4), alphaTilemap);

	m_deco_tilegen1->tilemap_1_draw(screen, bitmap, cliprect, 0, 0);
	return 0;
}
