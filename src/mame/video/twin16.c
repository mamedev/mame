/*

    Konami Twin16 Hardware - Video

    TODO:

    - clean up sprite system
    - bad sprites in devilw, eg. odd colours for the mud/lava monster in the 1st level,
      or wrong sprite-sprite priority sometimes -- check real arcade first
    - unsure about some sprite preprocessor attributes (see spriteram_process)

*/

#include "emu.h"
#include "includes/twin16.h"




enum
{
	TWIN16_SCREEN_FLIPY     = 0x01,
	TWIN16_SCREEN_FLIPX     = 0x02, // confirmed: Hard Puncher Intro
	TWIN16_UNKNOWN1         = 0x04, // ? Hard Puncher uses this
	TWIN16_PLANE_ORDER      = 0x08, // confirmed: Devil Worlds
	TWIN16_TILE_FLIPX       = 0x10, // unused?
	TWIN16_TILE_FLIPY       = 0x20  // confirmed? Vulcan Venture
};

enum
{
	// user-defined priorities
	TWIN16_BG_LAYER1            = 0x01,
	TWIN16_SPRITE_PRI_L1        = 0x02,
	TWIN16_BG_LAYER2            = 0x04,
	TWIN16_SPRITE_PRI_L2        = 0x08,
	TWIN16_SPRITE_OCCUPIED      = 0x10, // sprite on screen pixel
	TWIN16_SPRITE_CAST_SHADOW   = 0x20
};


WRITE16_MEMBER(twin16_state::fixram_w)
{
	COMBINE_DATA(&m_fixram[offset]);
	m_fixed_tmap->mark_tile_dirty(offset);
}

WRITE16_MEMBER(twin16_state::videoram0_w)
{
	COMBINE_DATA(&m_videoram[0][offset]);
	m_scroll_tmap[0]->mark_tile_dirty(offset);
}

WRITE16_MEMBER(twin16_state::videoram1_w)
{
	COMBINE_DATA(&m_videoram[1][offset]);
	m_scroll_tmap[1]->mark_tile_dirty(offset);
}

WRITE16_MEMBER(twin16_state::zipram_w)
{
	UINT16 old = m_zipram[offset];
	COMBINE_DATA(&m_zipram[offset]);
	if (m_zipram[offset] != old)
		m_gfxdecode->gfx(1)->mark_dirty(offset / 16);
}

WRITE16_MEMBER(fround_state::gfx_bank_w)
{
	int changed = 0;

	if (ACCESSING_BITS_0_7)
	{
		int oldbank0 = m_gfx_bank[0];
		int oldbank1 = m_gfx_bank[1];
		m_gfx_bank[0] = (data >> 0) & 0xf;
		m_gfx_bank[1] = (data >> 4) & 0xf;
		changed |= (oldbank0 ^ m_gfx_bank[0]) | (oldbank1 ^ m_gfx_bank[1]);
	}
	if (ACCESSING_BITS_8_15)
	{
		int oldbank2 = m_gfx_bank[2];
		int oldbank3 = m_gfx_bank[3];
		m_gfx_bank[2] = (data >> 8) & 0xf;
		m_gfx_bank[3] = (data >> 12) & 0xf;
		changed |= (oldbank2 ^ m_gfx_bank[2]) | (oldbank3 ^ m_gfx_bank[3]);
	}
	if (changed)
	{
		m_scroll_tmap[0]->mark_all_dirty();
		m_scroll_tmap[1]->mark_all_dirty();
	}
}

WRITE16_MEMBER(twin16_state::video_register_w)
{
	switch (offset)
	{
		case 0:
		{
			int old = m_video_register;
			COMBINE_DATA( &m_video_register );
			int changed = old ^ m_video_register;
			if (changed & (TWIN16_SCREEN_FLIPX | TWIN16_SCREEN_FLIPY))
			{
				int flip = (m_video_register&TWIN16_SCREEN_FLIPX) ? TILEMAP_FLIPX : 0;
				flip |= (m_video_register&TWIN16_SCREEN_FLIPY) ? TILEMAP_FLIPY : 0;
				machine().tilemap().set_flip_all(flip);
			}
			if (changed & (TWIN16_TILE_FLIPX | TWIN16_TILE_FLIPY))
			{
				m_scroll_tmap[0]->mark_all_dirty();
				m_scroll_tmap[1]->mark_all_dirty();
			}
			break;
		}

		case 1: COMBINE_DATA( &m_scrollx[0] ); break;
		case 2: COMBINE_DATA( &m_scrolly[0] ); break;
		case 3:
			COMBINE_DATA( &m_scrollx[1] );
			m_scroll_tmap[0]->set_scrollx(0, m_scrollx[1]);
			break;
		case 4:
			COMBINE_DATA( &m_scrolly[1] );
			m_scroll_tmap[0]->set_scrolly(0, m_scrolly[1]);
			break;
		case 5:
			COMBINE_DATA( &m_scrollx[2] );
			m_scroll_tmap[1]->set_scrollx(0, m_scrollx[2]);
			break;
		case 6:
			COMBINE_DATA( &m_scrolly[2] );
			m_scroll_tmap[1]->set_scrolly(0, m_scrolly[2]);
			break;

		default:
			logerror("unknown video_register write:%d", data );
			break;
	}
}

/*
 * Sprite Format
 * ----------------------------------
 * preprocessor (not much data to test with):
 * Word | Bit(s)           | Use
 * -----+-fedcba9876543210-+----------------
 *   0  | x--------------- | enable
 *   0  | -xxxxxxx-------- | ?
 *   0  | --------xxxxxxxx | sprite-sprite priority
 * -----+------------------+
 *   1  | xxxxxxxxxxxxxxxx | ?
 * -----+------------------+
 *   2  | xxxxxx---------- | ?
 *   2  | ------x--------- | yflip (devilw)
 *   2  | -------x-------- | xflip
 *   2  | --------xx------ | height
 *   2  | ----------xx---- | width
 *   2  | ------------xxxx | color
 * -----+------------------+
 *   3  | -xxxxxxxxxxxxxxx | code
 * -----+------------------+
 *   4  | -------xxxxxxxxx | xpos high, other bits probably no effect
 *   5  | xxxxxxxx-------- | xpos low, other bits probably no effect
 *   6  | -------xxxxxxxxx | xpos high, other bits probably no effect
 *   7  | xxxxxxxx-------- | ypos low, other bits probably no effect
 *
 * ----------------------------------
 * normal/after preprocessing:
 * Word | Bit(s)           | Use
 * -----+-fedcba9876543210-+----------------
 *   0  | -xxxxxxxxxxxxxxx | code
 * -----+------------------+
 *   1  | -------xxxxxxxxx | ypos
 * -----+------------------+
 *   2  | -------xxxxxxxxx | xpos
 * -----+------------------+
 *   3  | x--------------- | enable
 *   3  | -x-------------- | priority   ?
 *   3  | -----x---------- | no shadow  ?
 *   3  | ------x--------- | yflip  ?
 *   3  | -------x-------- | xflip
 *   3  | --------xx------ | height
 *   3  | ----------xx---- | width
 *   3  | ------------xxxx | color
 */

READ16_MEMBER(twin16_state::sprite_status_r)
{
	// bit 0: busy, other bits: dunno
	return m_sprite_busy;
}

TIMER_CALLBACK_MEMBER(twin16_state::sprite_tick)
{
	m_sprite_busy = 0;
}

int twin16_state::set_sprite_timer(  )
{
	if (m_sprite_busy) return 1;

	// sprite system busy, maybe a dma? time is guessed, assume 4 scanlines
	m_sprite_busy = 1;
	m_sprite_timer->adjust(m_screen->frame_period() / m_screen->height() * 4);

	return 0;
}

void twin16_state::spriteram_process(  )
{
	UINT16 *spriteram16 = m_spriteram->live();
	UINT16 dx = m_scrollx[0];
	UINT16 dy = m_scrolly[0];

	const UINT16 *source = &spriteram16[0x0000];
	const UINT16 *finish = &spriteram16[0x1800];

	set_sprite_timer();
	memset(&spriteram16[0x1800],0xff,0x800*sizeof(UINT16));

	while( source<finish )
	{
		UINT16 priority = source[0];
		if( priority & 0x8000 )
		{
			UINT16 *dest = &spriteram16[0x1800|(priority&0xff)<<2];

			UINT32 xpos = (0x10000*source[4])|source[5];
			UINT32 ypos = (0x10000*source[6])|source[7];

			/* notes on uncertain attributes:
			shadows: pen $F only (like other Konami hw), used in devilw, fround,
			 miaj? (shadows are solid in tmnt hw version),
			 gradius2? (ship exhaust)

			sprite-background priority: in devilw, most sprites look best at high priority,
			in gradius2, most sprites look best at low priority. exceptions:
			- devilw prologue: sprites behind crowd (maybe more, haven't completed the game)
			- gradius2 intro showing earlier games: sprites above layers

			currently using (priority&0x200), broken:
			- devilw prologue: sprites should be behind crowd
			- gradius2 level 7: bosses should be behind portal (ok except brain boss and mouth boss)
			- gradius2 ending: sun should be behind planet

			does TWIN16_PLANE_ORDER affect it?

			more?
			devilw monster dens exploding monochrome, players fading to white in prologue, and trees in
			the 1st level shrinking with a solid green color look odd, maybe alpha blended?

			fround, hpuncher, miaj, cuebrickj, don't use the preprocessor. all sprites are expected
			to be high priority, and shadows are enabled
			*/
			UINT16 attributes = 0x8000| // enabled
				(source[2]&0x03ff)| // scale,size,color
				(source[2]&0x4000)>>4|  // no-shadow? (gradius2 level 7 boss sets this bit and appears to expect pen $F to be solid)
				(priority&0x200)<<5;    // sprite-background priority?

			dest[0] = source[3]; /* gfx data */
			dest[1] = ((xpos>>8) - dx)&0xffff;
			dest[2] = ((ypos>>8) - dy)&0xffff;
			dest[3] = attributes;
		}
		source += 0x50/2;
	}
	m_need_process_spriteram = 0;
}

void twin16_state::draw_sprites( screen_device &screen, bitmap_ind16 &bitmap )
{
	const UINT16 *source = 0x1800+m_spriteram->buffer() + 0x800 - 4;
	const UINT16 *finish = 0x1800+m_spriteram->buffer();

	for (; source >= finish; source -= 4)
	{
		UINT16 attributes = source[3];
		UINT16 code = source[0];

		if((code!=0xffff) && (attributes&0x8000))
		{
			int xpos = source[1];
			int ypos = source[2];
			int x,y;

			int pal_base = ((attributes&0xf)+0x10)*16;
			int height  = 16<<((attributes>>6)&0x3);
			int width   = 16<<((attributes>>4)&0x3);
			const UINT16 *pen_data = 0;
			int flipy = attributes&0x0200;
			int flipx = attributes&0x0100;
			int priority = (attributes&0x4000)?TWIN16_SPRITE_PRI_L1:TWIN16_SPRITE_PRI_L2;

			if( m_is_fround ) {
				/* fround board */
				pen_data = m_gfxrom;
			}
			else
			{
				switch( (code>>12)&0x3 )
				{
					/* bank select */
					case 0:
						pen_data = m_gfxrom;
						break;

					case 1:
						pen_data = m_gfxrom + 0x40000;
						break;

					case 2:
						pen_data = m_gfxrom + 0x80000;
						if( code&0x4000 ) pen_data += 0x40000;
						break;

					case 3:
						pen_data = m_sprite_gfx_ram;
						break;
				}
				code &= 0xfff;
			}

			/* some code masking */
			if ((height&width) == 64) code &= ~8;       // gradius2 ending sequence 64*64
			else if ((height&width) == 32) code &= ~3;  // devilw 32*32
			else if ((height|width) == 48) code &= ~1;  // devilw 32*16 / 16*32

			pen_data += code*0x40;

			if( m_video_register&TWIN16_SCREEN_FLIPY )
			{
				if (ypos>65000) ypos=ypos-65536; /* Bit hacky */
				ypos = 256-ypos-height;
				flipy = !flipy;
			}
			if( m_video_register&TWIN16_SCREEN_FLIPX )
			{
				if (xpos>65000) xpos=xpos-65536; /* Bit hacky */
				xpos = 320-xpos-width;
				flipx = !flipx;
			}
			if( xpos>=320 ) xpos -= 65536;
			if( ypos>=256 ) ypos -= 65536;

			/* slow slow slow, but it's ok for now */
			for( y=0; y<height; y++, pen_data += width/4 )
			{
				int sy = (flipy)?(ypos+height-1-y):(ypos+y);
				if( sy>=16 && sy<256-16 )
				{
					UINT16 *dest = &bitmap.pix16(sy);
					UINT8 *pdest = &screen.priority().pix8(sy);

					for( x=0; x<width; x++ )
					{
						int sx = (flipx)?(xpos+width-1-x):(xpos+x);
						if( sx>=0 && sx<320 )
						{
							UINT16 pen = pen_data[x>>2]>>((~x&3)<<2)&0xf;

							if( pen )
							{
								int shadow = (pen==0xf) & ((attributes&0x400)==0);

								if (pdest[sx]<priority) {
									if (shadow) {
										dest[sx] = m_palette->shadow_table()[dest[sx]];
										pdest[sx]|=TWIN16_SPRITE_CAST_SHADOW;
									}
									else {
										dest[sx] = pal_base + pen;
									}
								}
								else if (!shadow && pdest[sx]&TWIN16_SPRITE_CAST_SHADOW && (pdest[sx]&0xf)<priority) {
									// shadow cast onto sprite below, evident in devilw lava level
									dest[sx] = m_palette->shadow_table()[pal_base + pen];
									pdest[sx]^=TWIN16_SPRITE_CAST_SHADOW;
								}

								pdest[sx]|=TWIN16_SPRITE_OCCUPIED;
							}
						}
					}
				}
			}
		}
	}
}


TILE_GET_INFO_MEMBER(twin16_state::fix_tile_info)
{
	int attr = m_fixram[tile_index];
	/* fedcba9876543210
	   -x-------------- yflip
	   --x------------- xflip
	   ---xxxx--------- color
	   -------xxxxxxxxx tile number
	*/
	int code = attr & 0x1ff;
	int color = (attr >> 9) & 0x0f;
	int flags=0;

	if (attr&0x2000) flags|=TILE_FLIPX;
	if (attr&0x4000) flags|=TILE_FLIPY;

	SET_TILE_INFO_MEMBER(0, code, color, flags);
}

void twin16_state::tile_get_info(tile_data &tileinfo, UINT16 data, int color_base)
{
	/* fedcba9876543210
	   xxx------------- color
	   ---xxxxxxxxxxxxx tile number
	*/
	int code = (data & 0x1fff);
	int color = color_base + (data >> 13);
	int flags = 0;
	if (m_video_register & TWIN16_TILE_FLIPX) flags |= TILE_FLIPX;
	if (m_video_register & TWIN16_TILE_FLIPY) flags |= TILE_FLIPY;
	SET_TILE_INFO_MEMBER(1, code, color, flags);
}

void fround_state::tile_get_info(tile_data &tileinfo, UINT16 data, int color_base)
{
	/* fedcba9876543210
	   xxx------------- color
	   ---xx----------- tile bank
	   -----xxxxxxxxxxx tile number
	*/
	int bank = (data >> 11) & 3;
	int code = (m_gfx_bank[bank] << 11) | (data & 0x7ff);
	int color = color_base | (data >> 13);
	int flags = 0;
	if (m_video_register & TWIN16_TILE_FLIPX) flags |= TILE_FLIPX;
	if (m_video_register & TWIN16_TILE_FLIPY) flags |= TILE_FLIPY;
	SET_TILE_INFO_MEMBER(1, code, color, flags);
}

TILE_GET_INFO_MEMBER(twin16_state::layer0_tile_info)
{
	tile_get_info(tileinfo, m_videoram[0][tile_index], 0);
}

TILE_GET_INFO_MEMBER(twin16_state::layer1_tile_info)
{
	tile_get_info(tileinfo, m_videoram[1][tile_index], 8);
}

void twin16_state::video_start()
{
	m_fixed_tmap = &machine().tilemap().create(m_gfxdecode,tilemap_get_info_delegate(FUNC(twin16_state::fix_tile_info),this),TILEMAP_SCAN_ROWS,8,8,64,32);
	m_scroll_tmap[0] = &machine().tilemap().create(m_gfxdecode,tilemap_get_info_delegate(FUNC(twin16_state::layer0_tile_info),this),TILEMAP_SCAN_ROWS,8,8,64,64);
	m_scroll_tmap[1] = &machine().tilemap().create(m_gfxdecode,tilemap_get_info_delegate(FUNC(twin16_state::layer1_tile_info),this),TILEMAP_SCAN_ROWS,8,8,64,64);

	m_fixed_tmap->set_transparent_pen(0);
	m_scroll_tmap[0]->set_transparent_pen(0);
	m_scroll_tmap[1]->set_transparent_pen(0);

	m_palette->set_shadow_factor(0.4); // screenshots estimate

	memset(m_sprite_buffer,0xff,0x800*sizeof(UINT16));
	m_video_register = 0;
	m_sprite_busy = 0;
	m_sprite_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(twin16_state::sprite_tick),this));
	m_sprite_timer->adjust(attotime::never);

	/* register for savestates */
	save_item(NAME(m_sprite_buffer));
	save_item(NAME(m_scrollx));
	save_item(NAME(m_scrolly));

	save_item(NAME(m_need_process_spriteram));
	save_item(NAME(m_video_register));
	save_item(NAME(m_sprite_busy));
}

void fround_state::video_start()
{
	twin16_state::video_start();
	save_item(NAME(m_gfx_bank));
}

UINT32 twin16_state::screen_update_twin16(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int layer = m_video_register & TWIN16_PLANE_ORDER ? 0 : 1;

	m_scroll_tmap[layer]->draw(screen, bitmap, cliprect, TILEMAP_DRAW_OPAQUE, TWIN16_BG_LAYER1, 0);
	m_scroll_tmap[layer^1]->draw(screen, bitmap, cliprect, 0, TWIN16_BG_LAYER2);

	draw_sprites( screen, bitmap );

	m_fixed_tmap->draw(screen, bitmap, cliprect, 0);
	return 0;
}

void twin16_state::screen_eof_twin16(screen_device &screen, bool state)
{
	// rising edge
	if (state)
	{
		set_sprite_timer();

		if (spriteram_process_enable()) {
			if (m_need_process_spriteram) spriteram_process();
			m_need_process_spriteram = 1;

			/* if the sprite preprocessor is used, sprite ram is copied to an external buffer first,
			as evidenced by 1-frame sprite lag in gradius2 and devilw otherwise, though there's probably
			more to it than that */
			memcpy(&m_spriteram->buffer()[0x1800],m_sprite_buffer,0x800*sizeof(UINT16));
			memcpy(m_sprite_buffer,&m_spriteram->live()[0x1800],0x800*sizeof(UINT16));
		}
		else {
			m_spriteram->copy();
		}
	}
}
