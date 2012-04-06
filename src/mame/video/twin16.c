/*

    Konami Twin16 Hardware - Video

    TODO:

    - convert background to tilemap
    - clean up sprite system
    - bad sprites in devilw, eg. odd colours for the mud/lava monster in the 1st level,
      or wrong sprite-sprite priority sometimes -- check real arcade first
    - unsure about some sprite preprocessor attributes (see twin16_spriteram_process)

*/

#include "emu.h"
#include "includes/twin16.h"

static TIMER_CALLBACK( twin16_sprite_tick );


enum
{
	TWIN16_SCREEN_FLIPY		= 0x01,
	TWIN16_SCREEN_FLIPX		= 0x02,	// confirmed: Hard Puncher Intro
	TWIN16_UNKNOWN1			= 0x04,	// ? Hard Puncher uses this
	TWIN16_PLANE_ORDER		= 0x08,	// confirmed: Devil Worlds
	TWIN16_TILE_FLIPX		= 0x10,	// unused?
	TWIN16_TILE_FLIPY		= 0x20	// confirmed? Vulcan Venture
};

enum
{
	// user-defined priorities
	TWIN16_BG_LAYER1			= 0x01,
	TWIN16_SPRITE_PRI_L1		= 0x02,
	TWIN16_BG_LAYER2			= 0x04,
	TWIN16_SPRITE_PRI_L2		= 0x08,
	TWIN16_SPRITE_OCCUPIED		= 0x10, // sprite on screen pixel
	TWIN16_SPRITE_CAST_SHADOW	= 0x20
};


WRITE16_MEMBER(twin16_state::twin16_text_ram_w)
{
	COMBINE_DATA(&m_text_ram[offset]);
	m_text_tilemap->mark_tile_dirty(offset);
}

WRITE16_MEMBER(twin16_state::twin16_paletteram_word_w)
{	// identical to tmnt_paletteram_w
	COMBINE_DATA(m_generic_paletteram_16 + offset);
	offset &= ~1;

	data = ((m_generic_paletteram_16[offset] & 0xff) << 8) | (m_generic_paletteram_16[offset + 1] & 0xff);
	palette_set_color_rgb(machine(), offset / 2, pal5bit(data >> 0), pal5bit(data >> 5), pal5bit(data >> 10));
}

WRITE16_MEMBER(twin16_state::fround_gfx_bank_w)
{
	COMBINE_DATA(&m_gfx_bank);
}

WRITE16_MEMBER(twin16_state::twin16_video_register_w)
{
	switch (offset)
	{
		case 0:
			COMBINE_DATA( &m_video_register );

			flip_screen_x_set(machine(), m_video_register & TWIN16_SCREEN_FLIPX);
			flip_screen_y_set(machine(), m_video_register & TWIN16_SCREEN_FLIPY);

			break;

		case 1: COMBINE_DATA( &m_scrollx[0] ); break;
		case 2: COMBINE_DATA( &m_scrolly[0] ); break;
		case 3: COMBINE_DATA( &m_scrollx[1] ); break;
		case 4: COMBINE_DATA( &m_scrolly[1] ); break;
		case 5: COMBINE_DATA( &m_scrollx[2] ); break;
		case 6: COMBINE_DATA( &m_scrolly[2] ); break;

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

READ16_MEMBER(twin16_state::twin16_sprite_status_r)
{
	// bit 0: busy, other bits: dunno
	return m_sprite_busy;
}

static TIMER_CALLBACK( twin16_sprite_tick )
{
	twin16_state *state = machine.driver_data<twin16_state>();
	state->m_sprite_busy = 0;
}

static int twin16_set_sprite_timer( running_machine &machine )
{
	twin16_state *state = machine.driver_data<twin16_state>();
	if (state->m_sprite_busy) return 1;

	// sprite system busy, maybe a dma? time is guessed, assume 4 scanlines
	state->m_sprite_busy = 1;
	state->m_sprite_timer->adjust(machine.primary_screen->frame_period() / machine.primary_screen->height() * 4);

	return 0;
}

void twin16_spriteram_process( running_machine &machine )
{
	twin16_state *state = machine.driver_data<twin16_state>();
	UINT16 *spriteram16 = state->m_spriteram->live();
	UINT16 dx = state->m_scrollx[0];
	UINT16 dy = state->m_scrolly[0];

	const UINT16 *source = &spriteram16[0x0000];
	const UINT16 *finish = &spriteram16[0x1800];

	twin16_set_sprite_timer(machine);
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
			UINT16 attributes = 0x8000|	// enabled
				(source[2]&0x03ff)|	// scale,size,color
				(source[2]&0x4000)>>4|	// no-shadow? (gradius2 level 7 boss sets this bit and appears to expect pen $F to be solid)
				(priority&0x200)<<5;	// sprite-background priority?

			dest[0] = source[3]; /* gfx data */
			dest[1] = ((xpos>>8) - dx)&0xffff;
			dest[2] = ((ypos>>8) - dy)&0xffff;
			dest[3] = attributes;
		}
		source += 0x50/2;
	}
	state->m_need_process_spriteram = 0;
}

static void draw_sprites( running_machine &machine, bitmap_ind16 &bitmap )
{
	twin16_state *state = machine.driver_data<twin16_state>();
	const UINT16 *source = 0x1800+state->m_spriteram->buffer() + 0x800 - 4;
	const UINT16 *finish = 0x1800+state->m_spriteram->buffer();

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
			int height	= 16<<((attributes>>6)&0x3);
			int width	= 16<<((attributes>>4)&0x3);
			const UINT16 *pen_data = 0;
			int flipy = attributes&0x0200;
			int flipx = attributes&0x0100;
			int priority = (attributes&0x4000)?TWIN16_SPRITE_PRI_L1:TWIN16_SPRITE_PRI_L2;

			if( state->m_custom_video ) {
				/* fround board */
				pen_data = state->m_gfx_rom + 0x80000;
			}
			else
			{
				switch( (code>>12)&0x3 )
				{
					/* bank select */
					case 0:
						pen_data = state->m_gfx_rom;
						break;

					case 1:
						pen_data = state->m_gfx_rom + 0x40000;
						break;

					case 2:
						pen_data = state->m_gfx_rom + 0x80000;
						if( code&0x4000 ) pen_data += 0x40000;
						break;

					case 3:
						pen_data = state->m_sprite_gfx_ram;
						break;
				}
				code &= 0xfff;
			}

			/* some code masking */
			if ((height&width) == 64) code &= ~8;		// gradius2 ending sequence 64*64
			else if ((height&width) == 32) code &= ~3;	// devilw 32*32
			else if ((height|width) == 48) code &= ~1;	// devilw 32*16 / 16*32

			pen_data += code*0x40;

			if( state->m_video_register&TWIN16_SCREEN_FLIPY )
			{
				if (ypos>65000) ypos=ypos-65536; /* Bit hacky */
				ypos = 256-ypos-height;
				flipy = !flipy;
			}
			if( state->m_video_register&TWIN16_SCREEN_FLIPX )
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
					UINT8 *pdest = &machine.priority_bitmap.pix8(sy);

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
										dest[sx] = machine.shadow_table[dest[sx]];
										pdest[sx]|=TWIN16_SPRITE_CAST_SHADOW;
									}
									else {
										dest[sx] = pal_base + pen;
									}
								}
								else if (!shadow && pdest[sx]&TWIN16_SPRITE_CAST_SHADOW && (pdest[sx]&0xf)<priority) {
									// shadow cast onto sprite below, evident in devilw lava level
									dest[sx] = machine.shadow_table[pal_base + pen];
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



static void draw_layer( running_machine &machine, bitmap_ind16 &bitmap, int opaque )
{
	twin16_state *state = machine.driver_data<twin16_state>();
	UINT16 *videoram = state->m_videoram;
	const UINT16 *gfx_base;
	const UINT16 *source = videoram;
	int i, xxor, yxor;
	int bank_table[4];
	int dx, dy, palette;
	int tile_flipx = state->m_video_register&TWIN16_TILE_FLIPX;
	int tile_flipy = state->m_video_register&TWIN16_TILE_FLIPY;

	if( ((state->m_video_register&TWIN16_PLANE_ORDER)?1:0) != opaque ) {
		source += 0x1000;
		dx = state->m_scrollx[2];
		dy = state->m_scrolly[2];
		palette = 1;
	}
	else {
		source += 0x0000;
		dx = state->m_scrollx[1];
		dy = state->m_scrolly[1];
		palette = 0;
	}

	if( state->m_custom_video ) {
		/* fround board */
		gfx_base = state->m_gfx_rom;
		bank_table[3] = (state->m_gfx_bank>>(4*3))&0xf;
		bank_table[2] = (state->m_gfx_bank>>(4*2))&0xf;
		bank_table[1] = (state->m_gfx_bank>>(4*1))&0xf;
		bank_table[0] = (state->m_gfx_bank>>(4*0))&0xf;
	}
	else {
		gfx_base = state->m_tile_gfx_ram;
		bank_table[0] = 0;
		bank_table[1] = 1;
		bank_table[2] = 2;
		bank_table[3] = 3;
	}

	if( state->m_video_register&TWIN16_SCREEN_FLIPX )
	{
		dx = 256-dx-64;
		tile_flipx = !tile_flipx;
	}

	if( state->m_video_register&TWIN16_SCREEN_FLIPY )
	{
		dy = 256-dy;
		tile_flipy = !tile_flipy;
	}

	xxor = tile_flipx ? 7 : 0;
	yxor = tile_flipy ? 7 : 0;

	for( i=0; i<64*64; i++ )
	{
		int y1, y2, x1, x2;
		int sx = (i%64)*8;
		int sy = (i/64)*8;
		int xpos,ypos;

		if( state->m_video_register&TWIN16_SCREEN_FLIPX ) sx = 63*8 - sx;
		if( state->m_video_register&TWIN16_SCREEN_FLIPY ) sy = 63*8 - sy;

		xpos = (sx-dx)&0x1ff;
		ypos = (sy-dy)&0x1ff;
		if( xpos>=320 ) xpos -= 512;
		if( ypos>=256 ) ypos -= 512;

		x1 = MAX(xpos, 0);
		x2 = MIN(xpos+7, bitmap.width()-1);
		y1 = MAX(ypos, 0);
		y2 = MIN(ypos+7, bitmap.height()-1);

		if (x1 <= x2 && y1 <= y2)
		{
			int code = source[i];
			/* fedcba9876543210
               xxx------------- color
               ---xx----------- tile bank
               -----xxxxxxxxxxx tile number
            */
			const UINT16 *gfx_data = gfx_base + (code&0x7ff)*16 + bank_table[(code>>11)&0x3]*0x8000;
			int color = (code>>13);
			int pal_base = 16*(0x20+color+8*palette);
			int x, y;

			if( opaque )
			{
				for (y = y1; y <= y2; y++)
				{
					const UINT16 *gfxptr = gfx_data + ((y - ypos) ^ yxor) * 2;
					UINT16 *dest = &bitmap.pix16(y);
					UINT8 *pdest = &machine.priority_bitmap.pix8(y);

					for (x = x1; x <= x2; x++)
					{
						int effx = (x - xpos) ^ xxor;
						UINT16 data = gfxptr[effx / 4];
						dest[x] = pal_base + ((data >> 4*(~effx & 3)) & 0x0f);
						pdest[x] |= TWIN16_BG_LAYER1;
					}
				}
			}
			else
			{
				for (y = y1; y <= y2; y++)
				{
					const UINT16 *gfxptr = gfx_data + ((y - ypos) ^ yxor) * 2;
					UINT16 *dest = &bitmap.pix16(y);
					UINT8 *pdest = &machine.priority_bitmap.pix8(y);

					for (x = x1; x <= x2; x++)
					{
						int effx = (x - xpos) ^ xxor;
						UINT16 data = gfxptr[effx / 4];
						UINT8 pen = (data >> 4*(~effx & 3)) & 0x0f;
						if (pen)
						{
							dest[x] = pal_base + pen;
							pdest[x] |= TWIN16_BG_LAYER2;
						}
					}
				}
			}
		}
	}
}

static TILE_GET_INFO( get_text_tile_info )
{
	twin16_state *state = machine.driver_data<twin16_state>();
	const UINT16 *source = state->m_text_ram;
	int attr = source[tile_index];
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

	SET_TILE_INFO(0, code, color, flags);
}

VIDEO_START( twin16 )
{
	twin16_state *state = machine.driver_data<twin16_state>();
	state->m_text_tilemap = tilemap_create(machine, get_text_tile_info, tilemap_scan_rows, 8, 8, 64, 32);
	state->m_text_tilemap->set_transparent_pen(0);

	palette_set_shadow_factor(machine,0.4); // screenshots estimate

	memset(state->m_sprite_buffer,0xff,0x800*sizeof(UINT16));
	state->m_sprite_busy = 0;
	state->m_sprite_timer = machine.scheduler().timer_alloc(FUNC(twin16_sprite_tick));
	state->m_sprite_timer->adjust(attotime::never);

	/* register for savestates */
	state_save_register_global_array(machine, state->m_sprite_buffer);
	state_save_register_global_array(machine, state->m_scrollx);
	state_save_register_global_array(machine, state->m_scrolly);

	state_save_register_global(machine, state->m_need_process_spriteram);
	state_save_register_global(machine, state->m_gfx_bank);
	state_save_register_global(machine, state->m_video_register);
	state_save_register_global(machine, state->m_sprite_busy);
}

SCREEN_UPDATE_IND16( twin16 )
{
	twin16_state *state = screen.machine().driver_data<twin16_state>();
	int text_flip=0;
	if (state->m_video_register&TWIN16_SCREEN_FLIPX) text_flip|=TILEMAP_FLIPX;
	if (state->m_video_register&TWIN16_SCREEN_FLIPY) text_flip|=TILEMAP_FLIPY;

	screen.machine().priority_bitmap.fill(0, cliprect);
	draw_layer( screen.machine(), bitmap, 1 );
	draw_layer( screen.machine(), bitmap, 0 );
	draw_sprites( screen.machine(), bitmap );

	if (text_flip) state->m_text_tilemap->set_flip(text_flip);
	state->m_text_tilemap->draw(bitmap, cliprect, 0, 0);
	return 0;
}

SCREEN_VBLANK( twin16 )
{
	// rising edge
	if (vblank_on)
	{
		twin16_state *state = screen.machine().driver_data<twin16_state>();
		twin16_set_sprite_timer(screen.machine());

		if (twin16_spriteram_process_enable(screen.machine())) {
			if (state->m_need_process_spriteram) twin16_spriteram_process(screen.machine());
			state->m_need_process_spriteram = 1;

			/* if the sprite preprocessor is used, sprite ram is copied to an external buffer first,
            as evidenced by 1-frame sprite lag in gradius2 and devilw otherwise, though there's probably
            more to it than that */
			memcpy(&state->m_spriteram->buffer()[0x1800],state->m_sprite_buffer,0x800*sizeof(UINT16));
			memcpy(state->m_sprite_buffer,&state->m_spriteram->live()[0x1800],0x800*sizeof(UINT16));
		}
		else {
			state->m_spriteram->copy();
		}
	}
}

