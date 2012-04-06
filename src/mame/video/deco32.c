#include "emu.h"
#include "includes/deco32.h"
#include "video/decospr.h"
#include "video/deco16ic.h"

/******************************************************************************/

WRITE32_MEMBER(deco32_state::deco32_pri_w)
{
	m_pri=data;
}

WRITE32_MEMBER(dragngun_state::dragngun_sprite_control_w)
{
	m_dragngun_sprite_ctrl=data;
}

WRITE32_MEMBER(dragngun_state::dragngun_spriteram_dma_w)
{
	/* DMA spriteram to private sprite chip area, and clear cpu ram */
	m_spriteram->copy();
	memset(m_spriteram->live(),0,0x2000);
}

WRITE32_MEMBER(deco32_state::deco32_ace_ram_w)
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

static void updateAceRam(running_machine& machine)
{
	deco32_state *state = machine.driver_data<deco32_state>();
	int r,g,b,i;
	UINT8 fadeptr=state->m_ace_ram[0x20];
	UINT8 fadeptg=state->m_ace_ram[0x21];
	UINT8 fadeptb=state->m_ace_ram[0x22];
	UINT8 fadepsr=state->m_ace_ram[0x23];
	UINT8 fadepsg=state->m_ace_ram[0x24];
	UINT8 fadepsb=state->m_ace_ram[0x25];
//  UINT8 mode=state->m_ace_ram[0x26];

	state->m_ace_ram_dirty=0;

	for (i=0; i<2048; i++)
	{
		/* Lerp palette entry to 'fadept' according to 'fadeps' */
		b = (state->m_generic_paletteram_32[i] >>16) & 0xff;
		g = (state->m_generic_paletteram_32[i] >> 8) & 0xff;
		r = (state->m_generic_paletteram_32[i] >> 0) & 0xff;

		if (i>255) /* Screenshots seem to suggest ACE fades do not affect playfield 1 palette (0-255) */
		{
			/* Yeah, this should really be fixed point, I know */
			b = (UINT8)((float)b + (((float)fadeptb - (float)b) * (float)fadepsb/255.0f));
			g = (UINT8)((float)g + (((float)fadeptg - (float)g) * (float)fadepsg/255.0f));
			r = (UINT8)((float)r + (((float)fadeptr - (float)r) * (float)fadepsr/255.0f));
		}

		palette_set_color(machine,i,MAKE_RGB(r,g,b));
	}
}

/******************************************************************************/

/* Later games have double buffered paletteram - the real palette ram is
only updated on a DMA call */

WRITE32_MEMBER(deco32_state::deco32_nonbuffered_palette_w)
{
	int r,g,b;

	COMBINE_DATA(&m_generic_paletteram_32[offset]);

	b = (m_generic_paletteram_32[offset] >>16) & 0xff;
	g = (m_generic_paletteram_32[offset] >> 8) & 0xff;
	r = (m_generic_paletteram_32[offset] >> 0) & 0xff;

	palette_set_color(machine(),offset,MAKE_RGB(r,g,b));
}

WRITE32_MEMBER(deco32_state::deco32_buffered_palette_w)
{
	COMBINE_DATA(&m_generic_paletteram_32[offset]);
	m_dirty_palette[offset]=1;
}

WRITE32_MEMBER(deco32_state::deco32_palette_dma_w)
{
	const int m=machine().total_colors();
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

				palette_set_color(machine(),i,MAKE_RGB(r,g,b));
			}
		}
	}
}

/******************************************************************************/


INLINE void dragngun_drawgfxzoom(
		bitmap_rgb32 &dest_bmp,const rectangle &clip,const gfx_element *gfx,
		UINT32 code,UINT32 color,int flipx,int flipy,int sx,int sy,
		int transparent_color,
		int scalex, int scaley,bitmap_ind8 *pri_buffer,UINT32 pri_mask, int sprite_screen_width, int  sprite_screen_height, UINT8 alpha )
{
	rectangle myclip;

	if (!scalex || !scaley) return;

	/*
    scalex and scaley are 16.16 fixed point numbers
    1<<15 : shrink to 50%
    1<<16 : uniform scale
    1<<17 : double to 200%
    */

	/* KW 991012 -- Added code to force clip to bitmap boundary */
	myclip = clip;
	myclip &= dest_bmp.cliprect();

	{
		if( gfx )
		{
			const pen_t *pal = &gfx->machine().pens[gfx->color_base + gfx->color_granularity * (color % gfx->total_colors)];
			const UINT8 *code_base = gfx_element_get_data(gfx, code % gfx->total_elements);

			if (sprite_screen_width && sprite_screen_height)
			{
				/* compute sprite increment per screen pixel */
				int dx = (gfx->width<<16)/sprite_screen_width;
				int dy = (gfx->height<<16)/sprite_screen_height;

				int ex = sx+sprite_screen_width;
				int ey = sy+sprite_screen_height;

				int x_index_base;
				int y_index;

				if( flipx )
				{
					x_index_base = (sprite_screen_width-1)*dx;
					dx = -dx;
				}
				else
				{
					x_index_base = 0;
				}

				if( flipy )
				{
					y_index = (sprite_screen_height-1)*dy;
					dy = -dy;
				}
				else
				{
					y_index = 0;
				}

				if( sx < clip.min_x)
				{ /* clip left */
					int pixels = clip.min_x-sx;
					sx += pixels;
					x_index_base += pixels*dx;
				}
				if( sy < clip.min_y )
				{ /* clip top */
					int pixels = clip.min_y-sy;
					sy += pixels;
					y_index += pixels*dy;
				}
				/* NS 980211 - fixed incorrect clipping */
				if( ex > clip.max_x+1 )
				{ /* clip right */
					int pixels = ex-clip.max_x-1;
					ex -= pixels;
				}
				if( ey > clip.max_y+1 )
				{ /* clip bottom */
					int pixels = ey-clip.max_y-1;
					ey -= pixels;
				}

				if( ex>sx )
				{ /* skip if inner loop doesn't draw anything */
					int y;

					/* case 1: no alpha */
					if (alpha == 0xff)
					{
						if (pri_buffer)
						{
							for( y=sy; y<ey; y++ )
							{
								const UINT8 *source = code_base + (y_index>>16) * gfx->line_modulo;
								UINT32 *dest = &dest_bmp.pix32(y);
								UINT8 *pri = &pri_buffer->pix8(y);

								int x, x_index = x_index_base;
								for( x=sx; x<ex; x++ )
								{
									int c = source[x_index>>16];
									if( c != transparent_color )
									{
										if (((1 << pri[x]) & pri_mask) == 0)
											dest[x] = pal[c];
										pri[x] = 31;
									}
									x_index += dx;
								}

								y_index += dy;
							}
						}
						else
						{
							for( y=sy; y<ey; y++ )
							{
								const UINT8 *source = code_base + (y_index>>16) * gfx->line_modulo;
								UINT32 *dest = &dest_bmp.pix32(y);

								int x, x_index = x_index_base;
								for( x=sx; x<ex; x++ )
								{
									int c = source[x_index>>16];
									if( c != transparent_color ) dest[x] = pal[c];
									x_index += dx;
								}

								y_index += dy;
							}
						}
					}

					/* alpha-blended */
					else
					{
						if (pri_buffer)
						{
							for( y=sy; y<ey; y++ )
							{
								const UINT8 *source = code_base + (y_index>>16) * gfx->line_modulo;
								UINT32 *dest = &dest_bmp.pix32(y);
								UINT8 *pri = &pri_buffer->pix8(y);

								int x, x_index = x_index_base;
								for( x=sx; x<ex; x++ )
								{
									int c = source[x_index>>16];
									if( c != transparent_color )
									{
										if (((1 << pri[x]) & pri_mask) == 0)
											dest[x] = alpha_blend_r32(dest[x], pal[c], alpha);
										pri[x] = 31;
									}
									x_index += dx;
								}

								y_index += dy;
							}
						}
						else
						{
							for( y=sy; y<ey; y++ )
							{
								const UINT8 *source = code_base + (y_index>>16) * gfx->line_modulo;
								UINT32 *dest = &dest_bmp.pix32(y);

								int x, x_index = x_index_base;
								for( x=sx; x<ex; x++ )
								{
									int c = source[x_index>>16];
									if( c != transparent_color ) dest[x] = alpha_blend_r32(dest[x], pal[c], alpha);
									x_index += dx;
								}

								y_index += dy;
							}
						}
					}
				}
			}
		}
	}
}

static void dragngun_draw_sprites(running_machine& machine, bitmap_rgb32 &bitmap, const rectangle &cliprect, const UINT32 *spritedata)
{
	dragngun_state *state = machine.driver_data<dragngun_state>();
	const UINT32 *layout_ram;
	const UINT32 *lookup_ram;
	int offs;

	/*
        Sprites are built from main control ram, which references tile
        layout ram, which finally references tile lookup ram which holds
        the actual tile indices to draw and index into the banking
        control.  Tile lookup and tile layout ram are double buffered.


        Main sprite control ram, 8 * 32 bit words per sprite, so

        Word 0:
            0x0400 - Banking control for tile layout RAM + tile lookup ram
            0x0200 - ?
            0x01ff - Index into tile layout RAM
        Word 1 :
        Word 2 : X base position
        Word 3 : Y base position
        Word 4 :
            0x8000: X flip
            0x03ff: X size of block in pixels (for scaling)
        Word 5 :
            0x8000: Y flip
            0x03ff: Y size of block in pixels (for scaling)
        Word 6 :
            0x1f - colour.
            0x20 - ?  Used for background at 'frog' boss and title screen dragon.
            0x40 - ?  priority?
            0x80 - Alpha blending enable
        Word 7 :


        Tile layout ram, 4 * 32 bit words per sprite, so

        Word 0:
            0x2000 - Selector for tile lookup bank!?!?!?!?!?!?
            0x1fff - Index into tile lookup ram (16 bit word based, NOT 32)
        Word 1:
            0xff00 - ?
            0x00f0 - Width
            0x000f - Height
        Word 2:
            0x01ff - X block offset
        Word 3:
            0x01ff - Y block offset
    */

	/* Sprite global disable bit */
	if (state->m_dragngun_sprite_ctrl&0x40000000)
		return;

	for (offs = 0;offs < 0x800;offs += 8)
	{
		int sx,sy,colour,fx,fy,w,h,x,y,bx,by,alpha,scalex,scaley;
		int zoomx,zoomy;
		int xpos,ypos;

		scalex=spritedata[offs+4]&0x3ff;
		scaley=spritedata[offs+5]&0x3ff;
		if (!scalex || !scaley) /* Zero pixel size in X or Y - skip block */
			continue;

		if (spritedata[offs+0]&0x400)
			layout_ram = state->m_dragngun_sprite_layout_1_ram + ((spritedata[offs+0]&0x1ff)*4); //CHECK!
		else
			layout_ram = state->m_dragngun_sprite_layout_0_ram + ((spritedata[offs+0]&0x1ff)*4); //1ff in drag gun code??
		h = (layout_ram[1]>>0)&0xf;
		w = (layout_ram[1]>>4)&0xf;
		if (!h || !w)
			continue;

		sx = spritedata[offs+2] & 0x3ff;
		sy = spritedata[offs+3] & 0x3ff;
		bx = layout_ram[2] & 0x1ff;
		by = layout_ram[3] & 0x1ff;
		if (bx&0x100) bx=1-(bx&0xff);
		if (by&0x100) by=1-(by&0xff); /* '1 - ' is strange, but correct for Dragongun 'Winners' screen. */
		if (sx >= 512) sx -= 1024;
		if (sy >= 512) sy -= 1024;

		colour = spritedata[offs+6]&0x1f;

		if (spritedata[offs+6]&0x80)
			alpha=0x80;
		else
			alpha=0xff;

		fx = spritedata[offs+4]&0x8000;
		fy = spritedata[offs+5]&0x8000;

//      if (spritedata[offs+0]&0x400)
		if (layout_ram[0]&0x2000)
			lookup_ram = state->m_dragngun_sprite_lookup_1_ram + (layout_ram[0]&0x1fff);
		else
			lookup_ram = state->m_dragngun_sprite_lookup_0_ram + (layout_ram[0]&0x1fff);

		zoomx=scalex * 0x10000 / (w*16);
		zoomy=scaley * 0x10000 / (h*16);

		if (!fy)
			ypos=(sy<<16) - (by*zoomy); /* The block offset scales with zoom, the base position does not */
		else
			ypos=(sy<<16) + (by*zoomy) - (16*zoomy);

		for (y=0; y<h; y++) {
			if (!fx)
				xpos=(sx<<16) - (bx*zoomx); /* The block offset scales with zoom, the base position does not */
			else
				xpos=(sx<<16) + (bx*zoomx) - (16*zoomx);

			for (x=0; x<w; x++) {
				int bank,sprite;

				sprite = ((*(lookup_ram++))&0x3fff);

				/* High bits of the sprite reference into the sprite control bits for banking */
				switch (sprite&0x3000) {
				default:
				case 0x0000: sprite=(sprite&0xfff) | ((state->m_dragngun_sprite_ctrl&0x000f)<<12); break;
				case 0x1000: sprite=(sprite&0xfff) | ((state->m_dragngun_sprite_ctrl&0x00f0)<< 8); break;
				case 0x2000: sprite=(sprite&0xfff) | ((state->m_dragngun_sprite_ctrl&0x0f00)<< 4); break;
				case 0x3000: sprite=(sprite&0xfff) | ((state->m_dragngun_sprite_ctrl&0xf000)<< 0); break;
				}

				/* Because of the unusual interleaved rom layout, we have to mangle the bank bits
                even further to suit our gfx decode */
				switch (sprite&0xf000) {
				case 0x0000: sprite=0xc000 | (sprite&0xfff); break;
				case 0x1000: sprite=0xd000 | (sprite&0xfff); break;
				case 0x2000: sprite=0xe000 | (sprite&0xfff); break;
				case 0x3000: sprite=0xf000 | (sprite&0xfff); break;

				case 0xc000: sprite=0x0000 | (sprite&0xfff); break;
				case 0xd000: sprite=0x1000 | (sprite&0xfff); break;
				case 0xe000: sprite=0x2000 | (sprite&0xfff); break;
				case 0xf000: sprite=0x3000 | (sprite&0xfff); break;
				}

				if (sprite&0x8000) bank=4; else bank=3;
				sprite&=0x7fff;

				if (zoomx!=0x10000 || zoomy!=0x10000)
					dragngun_drawgfxzoom(
						bitmap,cliprect,machine.gfx[bank],
						sprite,
						colour,
						fx,fy,
						xpos>>16,ypos>>16,
						15,zoomx,zoomy,NULL,0,
						((xpos+(zoomx<<4))>>16) - (xpos>>16), ((ypos+(zoomy<<4))>>16) - (ypos>>16), alpha );
				else
					drawgfx_alpha(bitmap,cliprect,machine.gfx[bank],
						sprite,
						colour,
						fx,fy,
						xpos>>16,ypos>>16,
						15,alpha);

				if (fx)
					xpos-=zoomx<<4;
				else
					xpos+=zoomx<<4;
			}
			if (fy)
				ypos-=zoomy<<4;
			else
				ypos+=zoomy<<4;
		}
	}
}

/******************************************************************************/

VIDEO_START( captaven )
{
	deco32_state *state = machine.driver_data<deco32_state>();
	state->m_has_ace_ram=0;
}

VIDEO_START( fghthist )
{
	deco32_state *state = machine.driver_data<deco32_state>();
	state->m_dirty_palette = auto_alloc_array(machine, UINT8, 4096);
	machine.device<decospr_device>("spritegen")->alloc_sprite_bitmap();
	state->m_has_ace_ram=0;
}

VIDEO_START( dragngun )
{
	dragngun_state *state = machine.driver_data<dragngun_state>();
	state->m_dirty_palette = auto_alloc_array(machine, UINT8, 4096);

	memset(state->m_dirty_palette,0,4096);

	state_save_register_global(machine, state->m_dragngun_sprite_ctrl);
	state->m_has_ace_ram=0;
}

VIDEO_START( lockload )
{
	dragngun_state *state = machine.driver_data<dragngun_state>();
	state->m_dirty_palette = auto_alloc_array(machine, UINT8, 4096);

	memset(state->m_dirty_palette,0,4096);

	state_save_register_global(machine, state->m_dragngun_sprite_ctrl);
	state->m_has_ace_ram=0;
}

VIDEO_START( nslasher )
{
	deco32_state *state = machine.driver_data<deco32_state>();
	int width, height;
	state->m_dirty_palette = auto_alloc_array(machine, UINT8, 4096);
	width = machine.primary_screen->width();
	height = machine.primary_screen->height();
	state->m_tilemap_alpha_bitmap=auto_bitmap_ind16_alloc(machine, width, height );
	machine.device<decospr_device>("spritegen1")->alloc_sprite_bitmap();
	machine.device<decospr_device>("spritegen2")->alloc_sprite_bitmap();
	memset(state->m_dirty_palette,0,4096);
	state_save_register_global(machine, state->m_pri);
	state->m_has_ace_ram=1;
}

/******************************************************************************/

SCREEN_VBLANK( captaven )
{

}

SCREEN_VBLANK( dragngun )
{

}


/******************************************************************************/

SCREEN_UPDATE_IND16( captaven )
{
	deco32_state *state = screen.machine().driver_data<deco32_state>();
	state->m_deco_tilegen1 = screen.machine().device("tilegen1");
	state->m_deco_tilegen2 = screen.machine().device("tilegen2");

	screen.machine().tilemap().set_flip_all(flip_screen_get(screen.machine()) ? (TILEMAP_FLIPY | TILEMAP_FLIPX) : 0);

	screen.machine().priority_bitmap.fill(0, cliprect);
	bitmap.fill(screen.machine().pens[0x000], cliprect); // Palette index not confirmed

	deco16ic_set_pf1_8bpp_mode(state->m_deco_tilegen2, 1);

	deco16ic_pf_update(state->m_deco_tilegen1, state->m_pf1_rowscroll, state->m_pf2_rowscroll);
	deco16ic_pf_update(state->m_deco_tilegen2, state->m_pf3_rowscroll, state->m_pf4_rowscroll);

	// pf4 not used (because pf3 is in 8bpp mode)

	if ((state->m_pri&1)==0)
	{
		deco16ic_tilemap_1_draw(state->m_deco_tilegen2, bitmap, cliprect, 0, 1);
		deco16ic_tilemap_2_draw(state->m_deco_tilegen1, bitmap, cliprect, 0, 2);
	}
	else
	{
		deco16ic_tilemap_2_draw(state->m_deco_tilegen1, bitmap, cliprect, 0, 1);
		deco16ic_tilemap_1_draw(state->m_deco_tilegen2, bitmap, cliprect, 0, 2);
	}

	deco16ic_tilemap_1_draw(state->m_deco_tilegen1, bitmap, cliprect, 0, 4);

	screen.machine().device<decospr_device>("spritegen")->set_alt_format(true);
	screen.machine().device<decospr_device>("spritegen")->draw_sprites(bitmap, cliprect, state->m_spriteram16_buffered, 0x400);

	return 0;
}

SCREEN_UPDATE_RGB32( dragngun )
{
	deco32_state *state = screen.machine().driver_data<deco32_state>();
	state->m_deco_tilegen1 = screen.machine().device("tilegen1");
	state->m_deco_tilegen2 = screen.machine().device("tilegen2");

	bitmap.fill(get_black_pen(screen.machine()), cliprect);

	deco16ic_pf_update(state->m_deco_tilegen1, state->m_pf1_rowscroll, state->m_pf2_rowscroll);
	deco16ic_pf_update(state->m_deco_tilegen2, state->m_pf3_rowscroll, state->m_pf4_rowscroll);

	//deco16ic_set_pf3_8bpp_mode(state->m_deco_tilegen1, 1); // despite being 8bpp this doesn't require the same shifting as captaven, why not?

	deco16ic_tilemap_2_draw(state->m_deco_tilegen2, bitmap, cliprect, 0, 0); // it uses pf3 in 8bpp mode instead, like captaven
	deco16ic_tilemap_1_draw(state->m_deco_tilegen2, bitmap, cliprect, 0, 0);
	deco16ic_tilemap_2_draw(state->m_deco_tilegen1, bitmap, cliprect, 0, 0);

	// zooming sprite draw is very slow, and sprites are buffered.. however, one of the levels attempts to use
	// partial updates for every line, which causes things to be very slow... the sprites appear to support
	// multiple layers of alpha, so rendering to a buffer for layer isn't easy (maybe there are multiple sprite
	// chips at work?)
	//
	// really, it needs optimizing .. the raster effects also need fixing properly, they're not correct using
	// partial updates right now but the old buffering of scroll values was a hack and doesn't work properly
	// with the concept of generic tilemap code.
	//
	// for now we only draw these 2 layers on the last update call
	if (cliprect.max_y == 247)
	{
		rectangle clip(cliprect.min_x, cliprect.max_x, 8, 247);

		dragngun_draw_sprites(screen.machine(),bitmap,clip,state->m_spriteram->buffer());
		deco16ic_tilemap_1_draw(state->m_deco_tilegen1, bitmap, clip, 0, 0);

	}

	return 0;
}


SCREEN_UPDATE_RGB32( fghthist )
{
	deco32_state *state = screen.machine().driver_data<deco32_state>();
	state->m_deco_tilegen1 = screen.machine().device("tilegen1");
	state->m_deco_tilegen2 = screen.machine().device("tilegen2");

	screen.machine().priority_bitmap.fill(0, cliprect);
	bitmap.fill(screen.machine().pens[0x000], cliprect); // Palette index not confirmed

	deco16ic_pf_update(state->m_deco_tilegen1, state->m_pf1_rowscroll, state->m_pf2_rowscroll);
	deco16ic_pf_update(state->m_deco_tilegen2, state->m_pf3_rowscroll, state->m_pf4_rowscroll);

	screen.machine().device<decospr_device>("spritegen")->draw_sprites(bitmap, cliprect, state->m_spriteram16_buffered, 0x800, true);

	/* Draw screen */
	deco16ic_tilemap_2_draw(state->m_deco_tilegen2, bitmap, cliprect, 0, 1);

	if(state->m_pri&1)
	{
		deco16ic_tilemap_2_draw(state->m_deco_tilegen1, bitmap, cliprect, 0, 2);
		screen.machine().device<decospr_device>("spritegen")->inefficient_copy_sprite_bitmap(bitmap, cliprect, 0x0800, 0x0800, 1024, 0x1ff);
		deco16ic_tilemap_1_draw(state->m_deco_tilegen2, bitmap, cliprect, 0, 4);
	}
	else
	{
		deco16ic_tilemap_1_draw(state->m_deco_tilegen2, bitmap, cliprect, 0, 2);
		screen.machine().device<decospr_device>("spritegen")->inefficient_copy_sprite_bitmap(bitmap, cliprect, 0x0800, 0x0800, 1024, 0x1ff);
		deco16ic_tilemap_2_draw(state->m_deco_tilegen1, bitmap, cliprect, 0, 4);
	}

	screen.machine().device<decospr_device>("spritegen")->inefficient_copy_sprite_bitmap(bitmap, cliprect, 0x0000, 0x0800, 1024, 0x1ff);

	deco16ic_tilemap_1_draw(state->m_deco_tilegen1, bitmap, cliprect, 0, 0);
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
static void mixDualAlphaSprites(bitmap_rgb32 &bitmap, const rectangle &cliprect, const gfx_element *gfx0, const gfx_element *gfx1, int mixAlphaTilemap)
{
	deco32_state *state = gfx0->machine().driver_data<deco32_state>();
	running_machine &machine = gfx0->machine();
	const pen_t *pens = machine.pens;
	const pen_t *pal0 = &pens[gfx0->color_base];
	const pen_t *pal1 = &pens[gfx1->color_base];
	const pen_t *pal2 = &pens[machine.gfx[(state->m_pri&1) ? 1 : 2]->color_base];
	int x,y;
	bitmap_ind16& sprite0_mix_bitmap = machine.device<decospr_device>("spritegen1")->get_sprite_temp_bitmap();
	bitmap_ind16& sprite1_mix_bitmap = machine.device<decospr_device>("spritegen2")->get_sprite_temp_bitmap();


	/* Mix sprites into main bitmap, based on priority & alpha */
	for (y=8; y<248; y++) {
		UINT8* tilemapPri=&machine.priority_bitmap.pix8(y);
		UINT16* sprite0=&sprite0_mix_bitmap.pix16(y);
		UINT16* sprite1=&sprite1_mix_bitmap.pix16(y);
		UINT32* destLine=&bitmap.pix32(y);
		UINT16* alphaTilemap=&state->m_tilemap_alpha_bitmap->pix16(y);

		for (x=0; x<320; x++) {
			UINT16 priColAlphaPal0=sprite0[x];
			UINT16 priColAlphaPal1=sprite1[x];
			UINT16 pri0=(priColAlphaPal0&0x6000)>>13;
			UINT16 pri1=(priColAlphaPal1&0x6000)>>13;
			UINT16 col0=((priColAlphaPal0&0x1f00)>>8) % gfx0->total_colors;
			UINT16 col1=((priColAlphaPal1&0x0f00)>>8) % gfx1->total_colors;
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
					destLine[x]=pal0[(priColAlphaPal0&0xff) + (gfx0->color_granularity * col0)];
				}
				else if ((pri0&0x3)==2) // Spri0 under top playfield
				{
					if (tilemapPri[x]<4)
						destLine[x]=pal0[(priColAlphaPal0&0xff) + (gfx0->color_granularity * col0)];
				}
				else // Spri0 under top & middle playfields
				{
					if (tilemapPri[x]<2)
						destLine[x]=pal0[(priColAlphaPal0&0xff) + (gfx0->color_granularity * col0)];
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
					//int alpha=((state->m_ace_ram[0x0 + (((priColAlphaPal1&0xf0)>>4)/2)]) * 8)-1;
					//if (alpha<0)
					//  alpha=0;

					/* I don't really understand how object ACE ram is really hooked up,
                        the only obvious place in Night Slashers is the stagecoach in level 2 */

					if (pri1==0 && (((priColAlphaPal0&0xff)==0 || ((pri0&0x3)!=0 && (pri0&0x3)!=1 && (pri0&0x3)!=2))))
					{
						if ((state->m_pri&1)==0 || ((state->m_pri&1)==1 && tilemapPri[x]<4) || ((state->m_pri&1)==1 && mixAlphaTilemap))
							destLine[x]=alpha_blend_r32(destLine[x], pal1[(priColAlphaPal1&0xff) + (gfx1->color_granularity * col1)], 0x80);
					}
					else if (pri1==1 && ((priColAlphaPal0&0xff)==0 || ((pri0&0x3)!=0 && (pri0&0x3)!=1 && (pri0&0x3)!=2)))
						destLine[x]=alpha_blend_r32(destLine[x], pal1[(priColAlphaPal1&0xff) + (gfx1->color_granularity * col1)], 0x80);
					else if (pri1==2)// TOdo
						destLine[x]=alpha_blend_r32(destLine[x], pal1[(priColAlphaPal1&0xff) + (gfx1->color_granularity * col1)], 0x80);
					else if (pri1==3)// TOdo
						destLine[x]=alpha_blend_r32(destLine[x], pal1[(priColAlphaPal1&0xff) + (gfx1->color_granularity * col1)], 0x80);
				}
				else
				{
					/*
                        Non alpha rules:

                        Pri 0 - Under sprite 0 pri 0, over all tilemaps
                    */
					if (pri1==0 && ((priColAlphaPal0&0xff)==0 || ((pri0&0x3)!=0)))
						destLine[x]=pal1[(priColAlphaPal1&0xff) + (gfx1->color_granularity * col1)];
					else if (pri1==1) // todo
						destLine[x]=pal1[(priColAlphaPal1&0xff) + (gfx1->color_granularity * col1)];
					else if (pri1==2) // todo
						destLine[x]=pal1[(priColAlphaPal1&0xff) + (gfx1->color_granularity * col1)];
					else if (pri1==3) // todo
						destLine[x]=pal1[(priColAlphaPal1&0xff) + (gfx1->color_granularity * col1)];
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
						int alpha=((state->m_ace_ram[0x17 + (((p&0xf0)>>4)/2)]) * 8)-1;
						if (alpha<0)
							alpha=0;

						destLine[x]=alpha_blend_r32(destLine[x], pal2[p], 255-alpha);
					}
				}
			}
		}
	}
}

SCREEN_UPDATE_RGB32( nslasher )
{
	deco32_state *state = screen.machine().driver_data<deco32_state>();
	int alphaTilemap=0;
	state->m_deco_tilegen1 = screen.machine().device("tilegen1");
	state->m_deco_tilegen2 = screen.machine().device("tilegen2");

	deco16ic_pf_update(state->m_deco_tilegen1, state->m_pf1_rowscroll, state->m_pf2_rowscroll);
	deco16ic_pf_update(state->m_deco_tilegen2, state->m_pf3_rowscroll, state->m_pf4_rowscroll);

	/* This is not a conclusive test for deciding if tilemap needs alpha blending */
	if (state->m_ace_ram[0x17]!=0x0 && state->m_pri)
		alphaTilemap=1;

	if (state->m_ace_ram_dirty)
		updateAceRam(screen.machine());

	screen.machine().priority_bitmap.fill(0, cliprect);

	bitmap.fill(screen.machine().pens[0x200], cliprect);

	/* Draw sprites to temporary bitmaps, saving alpha & priority info for later mixing */
	screen.machine().device<decospr_device>("spritegen1")->set_pix_raw_shift(8);
	screen.machine().device<decospr_device>("spritegen2")->set_pix_raw_shift(8);

	screen.machine().device<decospr_device>("spritegen1")->draw_sprites(bitmap, cliprect, state->m_spriteram16_buffered, 0x800, true);
	screen.machine().device<decospr_device>("spritegen2")->draw_sprites(bitmap, cliprect, state->m_spriteram16_2_buffered, 0x800, true);


	/* Render alpha-blended tilemap to separate buffer for proper mixing */
	state->m_tilemap_alpha_bitmap->fill(0, cliprect);

	/* Draw playfields & sprites */
	if (state->m_pri&2)
	{
		deco16ic_tilemap_12_combine_draw(state->m_deco_tilegen2, bitmap, cliprect, 0, 1, 1);
		deco16ic_tilemap_2_draw(state->m_deco_tilegen1, bitmap, cliprect, 0, 4);
	}
	else
	{
		deco16ic_tilemap_2_draw(state->m_deco_tilegen2, bitmap, cliprect, 0, 1);
		if (state->m_pri&1)
		{
			deco16ic_tilemap_2_draw(state->m_deco_tilegen1, bitmap, cliprect, 0, 2);
			if (alphaTilemap)
				deco16ic_tilemap_1_draw(state->m_deco_tilegen2, *state->m_tilemap_alpha_bitmap, cliprect, 0, 4);
			else
				deco16ic_tilemap_1_draw(state->m_deco_tilegen2, bitmap, cliprect, 0, 4);
		}
		else
		{
			deco16ic_tilemap_1_draw(state->m_deco_tilegen2, bitmap, cliprect, 0, 2);
			if (alphaTilemap)
				deco16ic_tilemap_2_draw(state->m_deco_tilegen1, *state->m_tilemap_alpha_bitmap, cliprect, 0, 4);
			else
				deco16ic_tilemap_2_draw(state->m_deco_tilegen1, bitmap, cliprect, 0, 4);
		}
	}

	mixDualAlphaSprites(bitmap, cliprect, screen.machine().gfx[3], screen.machine().gfx[4], alphaTilemap);

	deco16ic_tilemap_1_draw(state->m_deco_tilegen1, bitmap, cliprect, 0, 0);
	return 0;
}
