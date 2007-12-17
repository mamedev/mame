#include "driver.h"
#include "video/taitoic.h"
#include "includes/taito_f2.h"

#define TC0100SCN_GFX_NUM 1
#define TC0480SCP_GFX_NUM 1
#define TC0280GRD_GFX_NUM 2
#define TC0430GRW_GFX_NUM 2

extern UINT8 TC0360PRI_regs[16];

struct tempsprite
{
	int code,color;
	int flipx,flipy;
	int x,y;
	int zoomx,zoomy;
	int primask;
};
static struct tempsprite *spritelist;

static UINT16 *spriteram_buffered,*spriteram_delayed;


/************************************************************
                      SPRITE BANKING

  Four sprite banking methods are used for games with more
  than $2000 sprite tiles, because the sprite ram only has
  13 bits available for tile numbers.

   0 = standard (only a limited selection of sprites are
                  available for display at a given time)
   1 = use sprite extension area lo bytes for hi 6 bits
   2 = use sprite extension area hi bytes
   3 = use sprite extension area lo bytes as hi bytes
            (sprite extension areas mean all sprite
             tiles are always accessible)
************************************************************/

int f2_sprite_type = 0;
UINT16 *f2_sprite_extension;
size_t f2_spriteext_size;

static UINT16 spritebank[8];
//static UINT16 spritebank_eof[8];
static UINT16 spritebank_buffered[8];
static UINT16 koshien_spritebank;

static INT32 sprites_disabled,sprites_active_area,sprites_master_scrollx,sprites_master_scrolly;
/* remember flip status over frames because driftout can fail to set it */
static INT32 sprites_flipscreen = 0;


/* On the left hand screen edge (assuming horiz screen, no
   screenflip: in screenflip it is the right hand edge etc.)
   there may be 0-3 unwanted pixels in both tilemaps *and*
   sprites. To erase this we use f2_hide_pixels (0 to +3). */

static INT32 f2_hide_pixels;
static INT32 f2_flip_hide_pixels;	/* Different in some games */

static INT32 f2_pivot_xdisp = 0;	/* Needed in games with a pivot layer */
static INT32 f2_pivot_ydisp = 0;

static INT32 f2_tilemap_xoffs = 0;	/* Needed in TC0480SCP games */
static INT32 f2_tilemap_yoffs = 0;
static INT32 f2_text_xoffs = 0;

int f2_tilemap_col_base = 0;

static INT32 f2_game = 0;
static INT32 FOOTCHMP = 1;

static UINT8 f2_tilepri[6]; // todo - move into taitoic.c
static UINT8 f2_spritepri[6]; // todo - move into taitoic.c
static UINT8 f2_spriteblendmode; // todo - move into taitoic.c

/***********************************************************************************/

static void taitof2_core_vh_start (running_machine *machine, int sprite_type,int hide,int flip_hide,int x_offs,int y_offs,
		int flip_xoffs,int flip_yoffs,int flip_text_x_offs,int flip_text_yoffs)
{
	int i,chips;
	f2_sprite_type = sprite_type;
	f2_hide_pixels = hide;
	f2_flip_hide_pixels = flip_hide;

	spriteram_delayed = auto_malloc(spriteram_size);
	spriteram_buffered = auto_malloc(spriteram_size);
	spritelist = auto_malloc(0x400 * sizeof(*spritelist));

	chips = number_of_TC0100SCN();

	assert_always(chips >= 0, "erroneous TC0100SCN configuratiom");

	if (has_TC0480SCP())	/* it's a tc0480scp game */
	{
		TC0480SCP_vh_start(machine,TC0480SCP_GFX_NUM,f2_hide_pixels,f2_tilemap_xoffs,
		   f2_tilemap_yoffs,f2_text_xoffs,0,-1,0,f2_tilemap_col_base);
	}
	else	/* it's a tc0100scn game */
	{
		TC0100SCN_vh_start(machine,chips,TC0100SCN_GFX_NUM,f2_hide_pixels,0,
			flip_xoffs,flip_yoffs,flip_text_x_offs,flip_text_yoffs,TC0100SCN_SINGLE_VDU);
	}

	if (has_TC0110PCR())
		TC0110PCR_vh_start();

	if (has_TC0280GRD())
		TC0280GRD_vh_start(TC0280GRD_GFX_NUM);

	if (has_TC0430GRW())
		TC0430GRW_vh_start(TC0430GRW_GFX_NUM);

	if (has_TC0360PRI())
		TC0360PRI_vh_start();	/* Purely for save-state purposes */

	for (i = 0; i < 8; i ++)
	{
		spritebank_buffered[i] = 0x400 * i;
		spritebank[i] = spritebank_buffered[i];
	}

	sprites_disabled = 1;
	sprites_active_area = 0;

	f2_game = 0;	/* means NOT footchmp */

	state_save_register_global(f2_hide_pixels);
	state_save_register_global(f2_sprite_type);
	state_save_register_global_array(spritebank);
	state_save_register_global(koshien_spritebank);
	state_save_register_global(sprites_disabled);
	state_save_register_global(sprites_active_area);
	state_save_register_global_pointer(spriteram_delayed, spriteram_size/2);
	state_save_register_global_pointer(spriteram_buffered, spriteram_size/2);
}


/**************************************************************************************/
/*    ( spritetype, hide, hideflip, xoffs, yoffs, flipx, flipy, textflipx, textflipy) */
/**************************************************************************************/

VIDEO_START( taitof2_default )
{
	taitof2_core_vh_start(machine, 0,0,0,0,0,0,0,0,0);
}

VIDEO_START( taitof2_megab )   /* Megab, Liquidk */
{
	taitof2_core_vh_start(machine, 0,3,3,0,0,0,0,0,0);
}

VIDEO_START( taitof2_quiz )   /* Quiz Crayons, Quiz Jinsei */
{
	taitof2_core_vh_start(machine, 3,3,3,0,0,0,0,0,0);
}

VIDEO_START( taitof2_finalb )
{
	taitof2_core_vh_start(machine, 0,1,1,0,0,0,0,0,0);
}

VIDEO_START( taitof2_ssi )
{
	taitof2_core_vh_start(machine, 0,3,3,0,0,0,0,0,0);
}

VIDEO_START( taitof2_growl )
{
	taitof2_core_vh_start(machine, 0,3,3,0,0,0,0,0,0);
}

VIDEO_START( taitof2_ninjak )
{
	taitof2_core_vh_start(machine, 0,0,0,0,0,0,0,1,2);
}

VIDEO_START( taitof2_qzchikyu )
{
	taitof2_core_vh_start(machine, 0,0,4,0,0,-4,0,-11,0);
}

VIDEO_START( taitof2_solfigtr )
{
	taitof2_core_vh_start(machine, 0,3,-3,0,0,6,0,6,0);
}

VIDEO_START( taitof2_koshien )
{
	taitof2_core_vh_start(machine, 0,1,-1,0,0,2,0,0,0);
}

VIDEO_START( taitof2_gunfront )
{
	taitof2_core_vh_start(machine, 0,3,3,0,0,0,0,0,0);
}

VIDEO_START( taitof2_thundfox )
{
	taitof2_core_vh_start(machine, 0,3,-3,0,0,5,0,4,1);
}

VIDEO_START( taitof2_mjnquest )
{
	taitof2_core_vh_start(machine, 0,0,0,0,0,0,0,0,0);
	TC0100SCN_set_bg_tilemask(0x7fff);
}

VIDEO_START( taitof2_footchmp )
{
	f2_tilemap_xoffs = 0x1d;
	f2_tilemap_yoffs = 0x08;
	f2_text_xoffs = -1;
	f2_tilemap_col_base = 0;
	taitof2_core_vh_start(machine, 0,3,3,0,0,0,0,0,0);

	f2_game = FOOTCHMP;
}

VIDEO_START( taitof2_hthero )
{
	f2_tilemap_xoffs = 0x33;
	f2_tilemap_yoffs = - 0x04;
	f2_text_xoffs = -1;
	f2_tilemap_col_base = 0;
	taitof2_core_vh_start(machine, 0,3,3,0,0,0,0,0,0);

	f2_game = FOOTCHMP;
}

VIDEO_START( taitof2_deadconx )
{
	f2_tilemap_xoffs = 0x1e;
	f2_tilemap_yoffs = 0x08;
	f2_text_xoffs = -1;
	f2_tilemap_col_base = 0;
	taitof2_core_vh_start(machine, 0,3,3,0,0,0,0,0,0);
}

VIDEO_START( taitof2_deadconj )
{
	f2_tilemap_xoffs = 0x34;
	f2_tilemap_yoffs = - 0x05;
	f2_text_xoffs = -1;
	f2_tilemap_col_base = 0;
	taitof2_core_vh_start(machine, 0,3,3,0,0,0,0,0,0);
}

VIDEO_START( taitof2_metalb )
{
	f2_tilemap_xoffs = 0x32;
	f2_tilemap_yoffs = - 0x04;
	f2_text_xoffs = 1;	/* not the usual -1 */
	f2_tilemap_col_base = 256;   /* separate palette area for tilemaps */
	taitof2_core_vh_start(machine, 0,3,3,0,0,0,0,0,0);
}

VIDEO_START( taitof2_yuyugogo )
{
	taitof2_core_vh_start(machine, 1,3,3,0,0,0,0,0,0);
}

VIDEO_START( taitof2_yesnoj )
{
	taitof2_core_vh_start(machine, 0,3,3,0,0,0,0,0,0);
}

VIDEO_START( taitof2_dinorex )
{
	taitof2_core_vh_start(machine, 3,3,3,0,0,0,0,0,0);
}

VIDEO_START( taitof2_dondokod )	/* dondokod, cameltry */
{
	f2_pivot_xdisp = -16;
	f2_pivot_ydisp = 0;
	taitof2_core_vh_start(machine, 0,3,3,0,0,0,0,0,0);
}

VIDEO_START( taitof2_pulirula )
{
	f2_pivot_xdisp = -10;	/* alignment seems correct (see level 2, falling */
	f2_pivot_ydisp = 16;	/* block of ice after armour man) */
	taitof2_core_vh_start(machine, 2,3,3,0,0,0,0,0,0);
}

VIDEO_START( taitof2_driftout )
{
	f2_pivot_xdisp = -16;
	f2_pivot_ydisp = 16;
	taitof2_core_vh_start(machine, 0,3,3,0,0,0,0,0,0);
}


/********************************************************
          SPRITE READ AND WRITE HANDLERS

The spritebank buffering is currently not needed.

If we wanted to buffer sprites by an extra frame, it
might be for Footchmp. That seems to be the only game
altering spritebanks of sprites while they're on screen.
********************************************************/

WRITE16_HANDLER( taitof2_sprite_extension_w )
{
	/* areas above 0x1000 cleared in some games, but not used */

	if (offset < 0x800)
	{
		COMBINE_DATA(&f2_sprite_extension[offset]);
	}
}


WRITE16_HANDLER( taitof2_spritebank_w )
{
	int i=0;
	int j=0;

	if (offset < 2) return;   /* irrelevant zero writes */

	if (offset < 4)   /* special bank pairs */
	{
		j = (offset & 1) << 1;   /* either set pair 0&1 or 2&3 */
		i = data << 11;
		spritebank_buffered[j] = i;
		spritebank_buffered[j+1] = (i + 0x400);

//logerror("bank %d, set to: %04x\n", j, i);
//logerror("bank %d, paired so: %04x\n", j + 1, i + 0x400);

	}
	else   /* last 4 are individual banks */
	{
		i = data << 10;
		spritebank_buffered[offset] = i;

//logerror("bank %d, new value: %04x\n", offset, i);
	}

}

READ16_HANDLER( koshien_spritebank_r )
{
	return koshien_spritebank;
}

WRITE16_HANDLER( koshien_spritebank_w )
{
	koshien_spritebank = data;

	spritebank_buffered[0]=0x0000;   /* never changes */
	spritebank_buffered[1]=0x0400;

	spritebank_buffered[2] =  ((data & 0x00f) + 1) * 0x800;
	spritebank_buffered[4] = (((data & 0x0f0) >> 4) + 1) * 0x800;
	spritebank_buffered[6] = (((data & 0xf00) >> 8) + 1) * 0x800;
	spritebank_buffered[3] = spritebank_buffered[2] + 0x400;
	spritebank_buffered[5] = spritebank_buffered[4] + 0x400;
	spritebank_buffered[7] = spritebank_buffered[6] + 0x400;
}

static void taito_f2_tc360_spritemixdraw( mame_bitmap *dest_bmp,const gfx_element *gfx,
		UINT32 code,UINT32 color,int flipx,int flipy,int sx,int sy,
		const rectangle *clip,int scalex, int scaley)
{
	const pen_t *pal = &Machine->remapped_colortable[gfx->color_base + gfx->color_granularity * (color % gfx->total_colors)];
	UINT8 *source_base = gfx->gfxdata + (code % gfx->total_elements) * gfx->char_modulo;

	int sprite_screen_height = (scaley*gfx->height+0x8000)>>16;
	int sprite_screen_width = (scalex*gfx->width+0x8000)>>16;

	if (!scalex || !scaley) return;

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

		if( clip )
		{
			if( sx < clip->min_x)
			{ /* clip left */
				int pixels = clip->min_x-sx;
				sx += pixels;
				x_index_base += pixels*dx;
			}
			if( sy < clip->min_y )
			{ /* clip top */
				int pixels = clip->min_y-sy;
				sy += pixels;
				y_index += pixels*dy;
			}
			/* NS 980211 - fixed incorrect clipping */
			if( ex > clip->max_x+1 )
			{ /* clip right */
				int pixels = ex-clip->max_x-1;
				ex -= pixels;
			}
			if( ey > clip->max_y+1 )
			{ /* clip bottom */
				int pixels = ey-clip->max_y-1;
				ey -= pixels;
			}
		}

		if( ex>sx )
		{
			/* skip if inner loop doesn't draw anything */
			int y;

			for( y=sy; y<ey; y++ )
			{
				UINT8 *source = source_base + (y_index>>16) * gfx->line_modulo;
				UINT16 *dest = BITMAP_ADDR16(dest_bmp, y, 0);
				UINT8 *pri = BITMAP_ADDR8(priority_bitmap, y, 0);

				int x, x_index = x_index_base;
				for( x=sx; x<ex; x++ )
				{
					int c = source[x_index>>16];
					if( c && (pri[x]&0x80)==0 )
					{
						UINT8 tilemap_priority=0, sprite_priority=0;

						// Get tilemap priority (0 - 0xf) for this destination pixel
						if (pri[x]&0x10) tilemap_priority=f2_tilepri[4];
						else if (pri[x]&0x8) tilemap_priority=f2_tilepri[3];
						else if (pri[x]&0x4) tilemap_priority=f2_tilepri[2];
						else if (pri[x]&0x2) tilemap_priority=f2_tilepri[1];
						else if (pri[x]&0x1) tilemap_priority=f2_tilepri[0];

						// Get sprite priority (0 - 0xf) for this source pixel
						if ((color&0xc0)==0xc0)
							sprite_priority=f2_spritepri[3];
						else if ((color&0xc0)==0x80)
							sprite_priority=f2_spritepri[2];
						else if ((color&0xc0)==0x40)
							sprite_priority=f2_spritepri[1];
						else if ((color&0xc0)==0x00)
							sprite_priority=f2_spritepri[0];

						// Blend mode 1 - Sprite under tilemap, use sprite palette with tilemap data
						if ((f2_spriteblendmode&0xc0)==0xc0 && sprite_priority==(tilemap_priority-1))
						{
							dest[x]=(pal[c]&0xfff0)|(dest[x]&0xf);
						}
						// Blend mode 1 - Sprite over tilemap, use sprite data with tilemap palette
						else if ((f2_spriteblendmode&0xc0)==0xc0 && sprite_priority==(tilemap_priority+1))
						{
							if (dest[x]&0xf)
								dest[x]=(dest[x]&0xfff0)|(pal[c]&0xf);
							else
								dest[x]=pal[c];
						}
						// Blend mode 2 - Sprite under tilemap, use sprite data with tilemap palette
						else if ((f2_spriteblendmode&0xc0)==0x80 && sprite_priority==(tilemap_priority-1))
						{
							dest[x]=(dest[x]&0xffef);
						}
						// Blend mode 2 - Sprite over tilemap, alternate sprite palette, confirmed in Pulirula level 2
						else if ((f2_spriteblendmode&0xc0)==0x80 && sprite_priority==(tilemap_priority+1))
						{
							dest[x]=(pal[c]&0xffef); // Pulirula level 2, Liquid Kids attract mode
						}
						// No blending
						else
						{
							if (sprite_priority>tilemap_priority) // Ninja Kids confirms tilemap takes priority in equal value case
								dest[x]=pal[c];
						}
						pri[x] |= 0x80;
					}

					x_index += dx;
				}

				y_index += dy;
			}
		}
	}
}

static void draw_sprites(running_machine *machine, mame_bitmap *bitmap,const rectangle *cliprect,int *primasks,int uses_tc360_mixer)
{
	/*
        Sprite format:
        0000: ---xxxxxxxxxxxxx tile code (0x0000 - 0x1fff)
        0002: xxxxxxxx-------- sprite y-zoom level
              --------xxxxxxxx sprite x-zoom level

              0x00 - non scaled = 100%
              0x80 - scaled to 50%
              0xc0 - scaled to 25%
              0xe0 - scaled to 12.5%
              0xff - scaled to zero pixels size (off)

        [this zoom scale may not be 100% correct, see Gunfront flame screen]

        0004: ----xxxxxxxxxxxx x-coordinate (-0x800 to 0x07ff)
              ---x------------ latch extra scroll
              --x------------- latch master scroll
              -x-------------- don't use extra scroll compensation
              x--------------- absolute screen coordinates (ignore all sprite scrolls)
              xxxx------------ the typical use of the above is therefore
                               1010 = set master scroll
                               0101 = set extra scroll
        0006: ----xxxxxxxxxxxx y-coordinate (-0x800 to 0x07ff)
              x--------------- marks special control commands (used in conjunction with 00a)
                               If the special command flag is set:
              ---------------x related to sprite ram bank
              ---x------------ unknown (deadconx, maybe others)
              --x------------- unknown, some games (growl, gunfront) set it to 1 when
                               screen is flipped
        0008: --------xxxxxxxx color (0x00 - 0xff)
              -------x-------- flipx
              ------x--------- flipy
              -----x---------- if set, use latched color, else use & latch specified one
              ----x----------- if set, next sprite entry is part of sequence
              ---x------------ if clear, use latched y coordinate, else use current y
              --x------------- if set, y += 16
              -x-------------- if clear, use latched x coordinate, else use current x
              x--------------- if set, x += 16
        000a: only valid when the special command bit in 006 is set
              ---------------x related to sprite ram bank. I think this is the one causing
                               the bank switch, implementing it this way all games seem
                               to properly bank switch except for footchmp which uses the
                               bit in byte 006 instead.
              ------------x--- unknown; some games toggle it before updating sprite ram.
              ------xx-------- unknown (finalb)
              -----x---------- unknown (mjnquest)
              ---x------------ disable the following sprites until another marker with
                               this bit clear is found
              --x------------- flip screen

        000b - 000f : unused

    DG comment: the sprite zoom code grafted on from Jarek's TaitoB
    may mean I have pointlessly duplicated x,y latches in the zoom &
    non zoom parts.

    */
	int i,x,y,off,extoffs;
	int code,color,spritedata,spritecont,flipx,flipy;
	int xcurrent,ycurrent,big_sprite=0;
	int y_no=0, x_no=0, xlatch=0, ylatch=0, last_continuation_tile=0;   /* for zooms */
	UINT32 zoomword, zoomx, zoomy, zx=0, zy=0, zoomxlatch=0, zoomylatch=0;   /* for zooms */
	int scroll1x, scroll1y;
	int scrollx=0, scrolly=0;
	int curx,cury;
	int f2_x_offset;

	/* pdrawgfx() needs us to draw sprites front to back, so we have to build a list
       while processing sprite ram and then draw them all at the end */
	struct tempsprite *sprite_ptr = spritelist;

	/* must remember enable status from last frame because driftout fails to
       reactivate them from a certain point onwards. */
	int disabled = sprites_disabled;

	/* must remember master scroll from previous frame because driftout
       sometimes doesn't set it. */
	int master_scrollx = sprites_master_scrollx;
	int master_scrolly = sprites_master_scrolly;

	/* must also remember the sprite bank from previous frame. */
	int area = sprites_active_area;

	scroll1x = 0;
	scroll1y = 0;
	x = y = 0;
	xcurrent = ycurrent = 0;
	color = 0;

	f2_x_offset = f2_hide_pixels;   /* Get rid of 0-3 unwanted pixels on edge of screen. */
	if (sprites_flipscreen) f2_x_offset = -f2_flip_hide_pixels;		// was -f2_x_offset

	/* safety check to avoid getting stuck in bank 2 for games using only one bank */
	if (area == 0x8000 &&
			spriteram_buffered[(0x8000+6)/2] == 0 &&
			spriteram_buffered[(0x8000+10)/2] == 0)
		area = 0;


	for (off = 0;off < 0x4000;off += 16)
	{
		/* sprites_active_area may change during processing */
		int offs = off + area;

		if (spriteram_buffered[(offs+6)/2] & 0x8000)
		{
			disabled = spriteram_buffered[(offs+10)/2] & 0x1000;
			sprites_flipscreen = spriteram_buffered[(offs+10)/2] & 0x2000;

			/* Get rid of 0-3 unwanted pixels on edge of screen. */
			f2_x_offset = f2_hide_pixels;
			if (sprites_flipscreen) f2_x_offset = -f2_flip_hide_pixels;		// was -f2_x_offset

			if (f2_game == FOOTCHMP)
				area = 0x8000 * (spriteram_buffered[(offs+6)/2] & 0x0001);
			else
				area = 0x8000 * (spriteram_buffered[(offs+10)/2] & 0x0001);
			continue;
		}

//popmessage("%04x",area);

		/* check for extra scroll offset */
		if ((spriteram_buffered[(offs+4)/2] & 0xf000) == 0xa000)
		{
			master_scrollx = spriteram_buffered[(offs+4)/2] & 0xfff;
			if (master_scrollx >= 0x800) master_scrollx -= 0x1000;   /* signed value */
			master_scrolly = spriteram_buffered[(offs+6)/2] & 0xfff;
			if (master_scrolly >= 0x800) master_scrolly -= 0x1000;   /* signed value */
		}

		if ((spriteram_buffered[(offs+4)/2] & 0xf000) == 0x5000)
		{
			scroll1x = spriteram_buffered[(offs+4)/2] & 0xfff;
			if (scroll1x >= 0x800) scroll1x -= 0x1000;   /* signed value */

			scroll1y = spriteram_buffered[(offs+6)/2] & 0xfff;
			if (scroll1y >= 0x800) scroll1y -= 0x1000;   /* signed value */
		}

		if (disabled)
			continue;

		spritedata = spriteram_buffered[(offs+8)/2];

		spritecont = (spritedata & 0xff00) >> 8;

		if ((spritecont & 0x08) != 0)   /* sprite continuation flag set */
		{
			if (big_sprite == 0)   /* are we starting a big sprite ? */
			{
				xlatch = spriteram_buffered[(offs+4)/2] & 0xfff;
				ylatch = spriteram_buffered[(offs+6)/2] & 0xfff;
				x_no = 0;
				y_no = 0;
				zoomword = spriteram_buffered[(offs+2)/2];
				zoomylatch = (zoomword>>8) & 0xff;
				zoomxlatch = (zoomword) & 0xff;
				big_sprite = 1;   /* we have started a new big sprite */
			}
		}
		else if (big_sprite)
		{
			last_continuation_tile = 1;   /* don't clear big_sprite until last tile done */
		}


		if ((spritecont & 0x04) == 0)
			color = spritedata & 0xff;


// The bigsprite == 0 check fixes "tied-up" little sprites in Thunderfox
// which (mostly?) have spritecont = 0x20 when they are not continuations
// of anything.
		if (big_sprite == 0 || (spritecont & 0xf0) == 0)
		{
			x = spriteram_buffered[(offs+4)/2];

// Some absolute x values deduced here are 1 too high (scenes when you get
// home run in Koshien, and may also relate to BG layer woods and stuff as you
// journey in MjnQuest). You will see they are 1 pixel too far to the right.
// Where is this extra pixel offset coming from??

			if (x & 0x8000)   /* absolute (koshien) */
			{
				scrollx = - f2_x_offset - 0x60;
				scrolly = 0;
			}
			else if (x & 0x4000)   /* ignore extra scroll */
			{
				scrollx = master_scrollx - f2_x_offset - 0x60;
				scrolly = master_scrolly;
			}
			else   /* all scrolls applied */
			{
				scrollx = scroll1x + master_scrollx - f2_x_offset - 0x60;
				scrolly = scroll1y + master_scrolly;
			}
			x &= 0xfff;
			y = spriteram_buffered[(offs+6)/2] & 0xfff;

			xcurrent = x;
			ycurrent = y;
		}
		else
		{
			if ((spritecont & 0x10) == 0)
				y = ycurrent;
			else if ((spritecont & 0x20) != 0)
			{
				y += 16;
				y_no++;   /* keep track of y tile for zooms */
			}
			if ((spritecont & 0x40) == 0)
				x = xcurrent;
			else if ((spritecont & 0x80) != 0)
			{
				x += 16;
				y_no=0;
				x_no++;   /* keep track of x tile for zooms */
			}
		}

		if (big_sprite)
		{
			zoomx = zoomxlatch;
			zoomy = zoomylatch;
			zx = 0x10;	/* default, no zoom: 16 pixels across */
			zy = 0x10;	/* default, no zoom: 16 pixels vertical */

			if (zoomx || zoomy)
			{
				/* "Zoom" zx&y is pixel size horizontally and vertically
                   of our sprite chunk. So it is difference in x and y
                   coords of our chunk and diagonally adjoining one. */

// These calcs caused black lines between flames in Gunfront attract...
//              x = xlatch + x_no * (0x100 - zoomx) / 16;
//              y = ylatch + y_no * (0x100 - zoomy) / 16;
//              zx = xlatch + (x_no+1) * (0x100 - zoomx) / 16 - x;
//              zy = ylatch + (y_no+1) * (0x100 - zoomy) / 16 - y;

				x = xlatch + (x_no * (0x100 - zoomx)+12) / 16;    //ks
				y = ylatch + (y_no * (0x100 - zoomy)+12) / 16;    //ks
				zx = xlatch + ((x_no+1) * (0x100 - zoomx)+12) / 16 - x;  //ks
				zy = ylatch + ((y_no+1) * (0x100 - zoomy)+12) / 16 - y;  //ks
			}
		}
		else
		{
			zoomword = spriteram_buffered[(offs+2)/2];
			zoomy = (zoomword>>8) & 0xff;
			zoomx = (zoomword) & 0xff;
			zx = (0x100 - zoomx) / 16;
			zy = (0x100 - zoomy) / 16;
		}

		if (last_continuation_tile)
		{
			big_sprite=0;
			last_continuation_tile=0;
		}

		code = 0;
		extoffs = offs;
		/* spriteram[0x4000-7fff] has no corresponding extension area */
		if (extoffs >= 0x8000) extoffs -= 0x4000;

		if (f2_sprite_type == 0)
		{
			code = spriteram_buffered[(offs)/2] & 0x1fff;
			i = (code & 0x1c00) >> 10;
			code = spritebank[i] + (code & 0x3ff);
		}

		if (f2_sprite_type == 1)   /* Yuyugogo */
		{
			code = spriteram_buffered[(offs)/2] & 0x3ff;
			i = (f2_sprite_extension[(extoffs >> 4)] & 0x3f ) << 10;
			code = (i | code);
		}

		if (f2_sprite_type == 2)   /* Pulirula */
		{
			code = spriteram_buffered[(offs)/2] & 0xff;
			i = (f2_sprite_extension[(extoffs >> 4)] & 0xff00 );
			code = (i | code);
		}

		if (f2_sprite_type == 3)   /* Dinorex and a few quizzes */
		{
			code = spriteram_buffered[(offs)/2] & 0xff;
			i = (f2_sprite_extension[(extoffs >> 4)] & 0xff ) << 8;
			code = (i | code);
		}

		if (code == 0) continue;

		flipx = spritecont & 0x01;
		flipy = spritecont & 0x02;

		curx = (x + scrollx) & 0xfff;
		if (curx >= 0x800)	curx -= 0x1000;   /* treat it as signed */

		cury = (y + scrolly) & 0xfff;
		if (cury >= 0x800)	cury -= 0x1000;   /* treat it as signed */

		if (sprites_flipscreen)
		{
			/* -zx/y is there to fix zoomed sprite coords in screenflip.
               drawgfxzoom does not know to draw from flip-side of sprites when
               screen is flipped; so we must correct the coords ourselves. */

			curx = 320 - curx - zx;
			cury = 256 - cury - zy;
			flipx = !flipx;
			flipy = !flipy;
		}

		{
			sprite_ptr->code = code;
			sprite_ptr->color = color;
			if (machine->gfx[0]->color_granularity == 64)	/* Final Blow is 6-bit deep */
				sprite_ptr->color /= 4;
			sprite_ptr->flipx = flipx;
			sprite_ptr->flipy = flipy;
			sprite_ptr->x = curx;
			sprite_ptr->y = cury;
			sprite_ptr->zoomx = zx << 12;
			sprite_ptr->zoomy = zy << 12;

			if (primasks || uses_tc360_mixer)
			{
				if (primasks)
					sprite_ptr->primask = primasks[(color & 0xc0) >> 6];

				sprite_ptr++;
			}
			else
			{
				drawgfxzoom(bitmap,machine->gfx[0],
						sprite_ptr->code,
						sprite_ptr->color,
						sprite_ptr->flipx,sprite_ptr->flipy,
						sprite_ptr->x,sprite_ptr->y,
						cliprect,TRANSPARENCY_PEN,0,
						sprite_ptr->zoomx,sprite_ptr->zoomy);
			}
		}
	}


	/* this happens only if primsks != NULL */
	while (sprite_ptr != spritelist)
	{
		sprite_ptr--;

		if (!uses_tc360_mixer)
			pdrawgfxzoom(bitmap,machine->gfx[0],
					sprite_ptr->code,
					sprite_ptr->color,
					sprite_ptr->flipx,sprite_ptr->flipy,
					sprite_ptr->x,sprite_ptr->y,
					cliprect,TRANSPARENCY_PEN,0,
					sprite_ptr->zoomx,sprite_ptr->zoomy,
					sprite_ptr->primask);
		else
			taito_f2_tc360_spritemixdraw(bitmap,machine->gfx[0],
					sprite_ptr->code,
					sprite_ptr->color,
					sprite_ptr->flipx,sprite_ptr->flipy,
					sprite_ptr->x,sprite_ptr->y,
					cliprect,
					sprite_ptr->zoomx,sprite_ptr->zoomy);
	}
}

static int prepare_sprites;

static void update_spritebanks(void)
{
	int i;
#if 1
	for (i = 0; i < 8; i ++)
	{
		spritebank[i] = spritebank_buffered[i];
	}
#else
	/* this makes footchmp blobbing worse! */
	for (i = 0; i < 8; i ++)
	{
		spritebank[i] = spritebank_eof[i];
		spritebank_eof[i] = spritebank_buffered[i];
	}
#endif
}

static void taitof2_handle_sprite_buffering(void)
{
	if (prepare_sprites)	/* no buffering */
	{
		memcpy(spriteram_buffered,spriteram16,spriteram_size);
		prepare_sprites = 0;
	}
}

static void taitof2_update_sprites_active_area(void)
{
	int off;

	update_spritebanks();

	/* if the frame was skipped, we'll have to do the buffering now */
	taitof2_handle_sprite_buffering();

	/* safety check to avoid getting stuck in bank 2 for games using only one bank */
	if (sprites_active_area == 0x8000 &&
			spriteram_buffered[(0x8000+6)/2] == 0 &&
			spriteram_buffered[(0x8000+10)/2] == 0)
		sprites_active_area = 0;

	for (off = 0;off < 0x4000;off += 16)
	{
		/* sprites_active_area may change during processing */
		int offs = off + sprites_active_area;

		if (spriteram_buffered[(offs+6)/2] & 0x8000)
		{
			sprites_disabled = spriteram_buffered[(offs+10)/2] & 0x1000;
			if (f2_game == FOOTCHMP)
				sprites_active_area = 0x8000 * (spriteram_buffered[(offs+6)/2] & 0x0001);
			else
				sprites_active_area = 0x8000 * (spriteram_buffered[(offs+10)/2] & 0x0001);
			continue;
		}

		/* check for extra scroll offset */
		if ((spriteram_buffered[(offs+4)/2] & 0xf000) == 0xa000)
		{
			sprites_master_scrollx = spriteram_buffered[(offs+4)/2] & 0xfff;
			if (sprites_master_scrollx >= 0x800)
				sprites_master_scrollx -= 0x1000;   /* signed value */

			sprites_master_scrolly = spriteram_buffered[(offs+6)/2] & 0xfff;
			if (sprites_master_scrolly >= 0x800)
				sprites_master_scrolly -= 0x1000;   /* signed value */
		}
	}
}

VIDEO_EOF( taitof2_no_buffer )
{
	taitof2_update_sprites_active_area();

	prepare_sprites = 1;
}

VIDEO_EOF( taitof2_full_buffer_delayed )
{
	int i;

	taitof2_update_sprites_active_area();

	prepare_sprites = 0;
	memcpy(spriteram_buffered,spriteram_delayed,spriteram_size);
	for (i = 0;i < spriteram_size/2;i++)
		spriteram_buffered[i] = spriteram16[i];
	memcpy(spriteram_delayed,spriteram16,spriteram_size);
}

VIDEO_EOF( taitof2_partial_buffer_delayed )
{
	int i;

	taitof2_update_sprites_active_area();

	prepare_sprites = 0;
	memcpy(spriteram_buffered,spriteram_delayed,spriteram_size);
	for (i = 0;i < spriteram_size/2;i += 4)
		spriteram_buffered[i] = spriteram16[i];
	memcpy(spriteram_delayed,spriteram16,spriteram_size);
}

VIDEO_EOF( taitof2_partial_buffer_delayed_thundfox )
{
	int i;

	taitof2_update_sprites_active_area();

	prepare_sprites = 0;
	memcpy(spriteram_buffered,spriteram_delayed,spriteram_size);
	for (i = 0;i < spriteram_size/2;i += 8)
	{
		spriteram_buffered[i]   = spriteram16[i];
		spriteram_buffered[i+1] = spriteram16[i+1];
		spriteram_buffered[i+4] = spriteram16[i+4];
	}
	memcpy(spriteram_delayed,spriteram16,spriteram_size);
}

VIDEO_EOF( taitof2_partial_buffer_delayed_qzchikyu )
{
	/* spriteram[2] and [3] are 1 frame behind...
       probably thundfox_eof_callback would work fine */

	int i;

	taitof2_update_sprites_active_area();

	prepare_sprites = 0;
	memcpy(spriteram_buffered,spriteram_delayed,spriteram_size);
	for (i = 0;i < spriteram_size/2;i += 8)
	{
		spriteram_buffered[i]   = spriteram16[i];
		spriteram_buffered[i+1] = spriteram16[i+1];
		spriteram_buffered[i+4] = spriteram16[i+4];
		spriteram_buffered[i+5] = spriteram16[i+5];	// not needed?
		spriteram_buffered[i+6] = spriteram16[i+6];	// not needed?
		spriteram_buffered[i+7] = spriteram16[i+7];	// not needed?
	}
	memcpy(spriteram_delayed,spriteram16,spriteram_size);
}


/* SSI */
VIDEO_UPDATE( ssi )
{
	taitof2_handle_sprite_buffering();

	/* SSI only uses sprites, the tilemap registers are not even initialized.
       (they are in Majestic 12, but the tilemaps are not used anyway) */
	fillbitmap(priority_bitmap,0,cliprect);
	fillbitmap(bitmap,machine->pens[0],cliprect);
	draw_sprites(machine, bitmap,cliprect,NULL, 0);
	return 0;
}


VIDEO_UPDATE( yesnoj )
{
	taitof2_handle_sprite_buffering();

	TC0100SCN_tilemap_update(machine);

	fillbitmap(priority_bitmap,0,cliprect);
	fillbitmap(bitmap,machine->pens[0],cliprect);	/* wrong color? */
	draw_sprites(machine, bitmap,cliprect,NULL, 0);
	TC0100SCN_tilemap_draw(machine,bitmap,cliprect,0,TC0100SCN_bottomlayer(0),0,0);
	TC0100SCN_tilemap_draw(machine,bitmap,cliprect,0,TC0100SCN_bottomlayer(0)^1,0,0);
	TC0100SCN_tilemap_draw(machine,bitmap,cliprect,0,2,0,0);
	return 0;
}


VIDEO_UPDATE( taitof2 )
{
	taitof2_handle_sprite_buffering();

	TC0100SCN_tilemap_update(machine);

	fillbitmap(priority_bitmap,0,cliprect);
	fillbitmap(bitmap,machine->pens[0],cliprect);	/* wrong color? */
	TC0100SCN_tilemap_draw(machine,bitmap,cliprect,0,TC0100SCN_bottomlayer(0),0,0);
	TC0100SCN_tilemap_draw(machine,bitmap,cliprect,0,TC0100SCN_bottomlayer(0)^1,0,0);
	draw_sprites(machine, bitmap,cliprect,NULL, 0);
	TC0100SCN_tilemap_draw(machine,bitmap,cliprect,0,2,0,0);
	return 0;
}


VIDEO_UPDATE( taitof2_pri )
{
	int layer[3];

	taitof2_handle_sprite_buffering();

	TC0100SCN_tilemap_update(machine);

	layer[0] = TC0100SCN_bottomlayer(0);
	layer[1] = layer[0]^1;
	layer[2] = 2;
	f2_tilepri[layer[0]] = TC0360PRI_regs[5] & 0x0f;
	f2_tilepri[layer[1]] = TC0360PRI_regs[5] >> 4;
	f2_tilepri[layer[2]] = TC0360PRI_regs[4] >> 4;

	f2_spritepri[0] = TC0360PRI_regs[6] & 0x0f;
	f2_spritepri[1] = TC0360PRI_regs[6] >> 4;
	f2_spritepri[2] = TC0360PRI_regs[7] & 0x0f;
	f2_spritepri[3] = TC0360PRI_regs[7] >> 4;

	f2_spriteblendmode = TC0360PRI_regs[0]&0xc0;

	fillbitmap(priority_bitmap,0,cliprect);
	fillbitmap(bitmap,machine->pens[0],cliprect);	/* wrong color? */

	TC0100SCN_tilemap_draw(machine,bitmap,cliprect,0,layer[0],0,1);
	TC0100SCN_tilemap_draw(machine,bitmap,cliprect,0,layer[1],0,2);
	TC0100SCN_tilemap_draw(machine,bitmap,cliprect,0,layer[2],0,4);

	draw_sprites(machine, bitmap,cliprect,NULL,1);
	return 0;
}



static void draw_roz_layer(mame_bitmap *bitmap,const rectangle *cliprect,UINT32 priority)
{
	if (has_TC0280GRD())
		TC0280GRD_zoom_draw(bitmap,cliprect,f2_pivot_xdisp,f2_pivot_ydisp,priority);

	if (has_TC0430GRW())
		TC0430GRW_zoom_draw(bitmap,cliprect,f2_pivot_xdisp,f2_pivot_ydisp,priority);
}

VIDEO_UPDATE( taitof2_pri_roz )
{
	int tilepri[3];
	int rozpri;
	int layer[3];
	int drawn;
	int i,j;
	int roz_base_color = (TC0360PRI_regs[1] & 0x3f) << 2;

	taitof2_handle_sprite_buffering();

	if (has_TC0280GRD())
		TC0280GRD_tilemap_update(roz_base_color);

	if (has_TC0430GRW())
		TC0430GRW_tilemap_update(roz_base_color);

	TC0100SCN_tilemap_update(machine);

	rozpri = (TC0360PRI_regs[1] & 0xc0) >> 6;
	rozpri = (TC0360PRI_regs[8 + rozpri/2] >> 4*(rozpri & 1)) & 0x0f;

	layer[0] = TC0100SCN_bottomlayer(0);
	layer[1] = layer[0]^1;
	layer[2] = 2;

	tilepri[layer[0]] = TC0360PRI_regs[5] & 0x0f;
	tilepri[layer[1]] = TC0360PRI_regs[5] >> 4;
	tilepri[layer[2]] = TC0360PRI_regs[4] >> 4;

	f2_spritepri[0] = TC0360PRI_regs[6] & 0x0f;
	f2_spritepri[1] = TC0360PRI_regs[6] >> 4;
	f2_spritepri[2] = TC0360PRI_regs[7] & 0x0f;
	f2_spritepri[3] = TC0360PRI_regs[7] >> 4;

	f2_spriteblendmode = TC0360PRI_regs[0]&0xc0;

	fillbitmap(priority_bitmap,0,cliprect);
	fillbitmap(bitmap,machine->pens[0],cliprect);	/* wrong color? */

	drawn=0;
	for (i=0; i<16; i++)
	{
		if (rozpri==i)
		{
			draw_roz_layer(bitmap,cliprect,1 << drawn);
			f2_tilepri[drawn]=i;
			drawn++;
		}

		for (j=0; j<3; j++)
		{
			if (tilepri[layer[j]]==i)
			{
				TC0100SCN_tilemap_draw(machine,bitmap,cliprect,0,layer[j],0,1<<drawn);
				f2_tilepri[drawn]=i;
				drawn++;
			}
		}
	}

	draw_sprites(machine, bitmap,cliprect,NULL,1);
	return 0;
}



/* Thunderfox */
VIDEO_UPDATE( thundfox )
{
	int tilepri[2][3];
	int spritepri[4];
	int layer[2][3];
	int drawn[2];


	taitof2_handle_sprite_buffering();

	TC0100SCN_tilemap_update(machine);

	layer[0][0] = TC0100SCN_bottomlayer(0);
	layer[0][1] = layer[0][0]^1;
	layer[0][2] = 2;
	tilepri[0][layer[0][0]] = TC0360PRI_regs[5] & 0x0f;
	tilepri[0][layer[0][1]] = TC0360PRI_regs[5] >> 4;
	tilepri[0][layer[0][2]] = TC0360PRI_regs[4] >> 4;

	layer[1][0] = TC0100SCN_bottomlayer(1);
	layer[1][1] = layer[1][0]^1;
	layer[1][2] = 2;
	tilepri[1][layer[1][0]] = TC0360PRI_regs[9] & 0x0f;
	tilepri[1][layer[1][1]] = TC0360PRI_regs[9] >> 4;
	tilepri[1][layer[1][2]] = TC0360PRI_regs[8] >> 4;

	spritepri[0] = TC0360PRI_regs[6] & 0x0f;
	spritepri[1] = TC0360PRI_regs[6] >> 4;
	spritepri[2] = TC0360PRI_regs[7] & 0x0f;
	spritepri[3] = TC0360PRI_regs[7] >> 4;


	fillbitmap(priority_bitmap,0,cliprect);
	fillbitmap(bitmap,machine->pens[0],cliprect);	/* wrong color? */


	/*
    TODO: This isn't the correct way to handle the priority. At the moment of
    writing, pdrawgfx() doesn't support 6 layers, so I have to cheat, assuming
    that the two FG layers are always on top of sprites.
    */

	drawn[0] = drawn[1] = 0;
	while (drawn[0] < 2 && drawn[1] < 2)
	{
		int pick;

		if (tilepri[0][drawn[0]] < tilepri[1][drawn[1]])
			pick = 0;
		else pick = 1;

		TC0100SCN_tilemap_draw(machine,bitmap,cliprect,pick,layer[pick][drawn[pick]],0,1<<(drawn[pick]+2*pick));
		drawn[pick]++;
	}
	while (drawn[0] < 2)
	{
		TC0100SCN_tilemap_draw(machine,bitmap,cliprect,0,layer[0][drawn[0]],0,1<<drawn[0]);
		drawn[0]++;
	}
	while (drawn[1] < 2)
	{
		TC0100SCN_tilemap_draw(machine,bitmap,cliprect,1,layer[1][drawn[1]],0,1<<(drawn[1]+2));
		drawn[1]++;
	}

	{
		int primasks[4] = {0,0,0,0};
		int i;

		for (i = 0;i < 4;i++)
		{
			if (spritepri[i] < tilepri[0][0]) primasks[i] |= 0xaaaa;
			if (spritepri[i] < tilepri[0][1]) primasks[i] |= 0xcccc;
			if (spritepri[i] < tilepri[1][0]) primasks[i] |= 0xf0f0;
			if (spritepri[i] < tilepri[1][1]) primasks[i] |= 0xff00;
		}

		draw_sprites(machine, bitmap,cliprect,primasks,0);
	}


	/*
    TODO: This isn't the correct way to handle the priority. At the moment of
    writing, pdrawgfx() doesn't support 6 layers, so I have to cheat, assuming
    that the two FG layers are always on top of sprites.
    */

	if (tilepri[0][2] < tilepri[1][2])
	{
		TC0100SCN_tilemap_draw(machine,bitmap,cliprect,0,layer[0][2],0,0);
		TC0100SCN_tilemap_draw(machine,bitmap,cliprect,1,layer[1][2],0,0);
	}
	else
	{
		TC0100SCN_tilemap_draw(machine,bitmap,cliprect,1,layer[1][2],0,0);
		TC0100SCN_tilemap_draw(machine,bitmap,cliprect,0,layer[0][2],0,0);
	}
	return 0;
}



/*********************************************************************

Deadconx and Footchmp use in the PRI chip
-----------------------------------------

+4  xxxx0000   BG0
    0000xxxx   BG3
+6  xxxx0000   BG2
    0000xxxx   BG1

Deadconx = 0x7db9 (bg0-3) 0x8eca (sprites)
So it has bg0 [back] / s / bg1 / s / bg2 / s / bg3 / s

Footchmp = 0x8db9 (bg0-3) 0xe5ac (sprites)
So it has s / bg0 [grass] / bg1 [crowd] / s / bg2 [goal] / s / bg3 [messages] / s [scan dots]

Metalb uses in the PRI chip
---------------------------

+4  xxxx0000   BG1
    0000xxxx   BG0
+6  xxxx0000   BG3
    0000xxxx   BG2

and it changes these (and the sprite pri settings) a lot.

********************************************************************/

VIDEO_UPDATE( metalb )
{
	UINT8 layer[5], invlayer[4];
	UINT16 priority;

	taitof2_handle_sprite_buffering();

	TC0480SCP_tilemap_update(machine);

	priority = TC0480SCP_get_bg_priority();

	layer[0] = (priority &0xf000) >> 12;	/* tells us which bg layer is bottom */
	layer[1] = (priority &0x0f00) >>  8;
	layer[2] = (priority &0x00f0) >>  4;
	layer[3] = (priority &0x000f) >>  0;	/* tells us which is top */
	layer[4] = 4;   /* text layer always over bg layers */

	invlayer[layer[0]]=0;
	invlayer[layer[1]]=1;
	invlayer[layer[2]]=2;
	invlayer[layer[3]]=3;

	f2_tilepri[invlayer[0]] = TC0360PRI_regs[4] & 0x0f;	/* bg0 */
	f2_tilepri[invlayer[1]] = TC0360PRI_regs[4] >> 4;	/* bg1 */
	f2_tilepri[invlayer[2]] = TC0360PRI_regs[5] & 0x0f;	/* bg2 */
	f2_tilepri[invlayer[3]] = TC0360PRI_regs[5] >> 4;	/* bg3 */
	f2_tilepri[4] = TC0360PRI_regs[9] & 0x0f;			/* fg (text layer) */

	f2_spritepri[0] = TC0360PRI_regs[6] & 0x0f;
	f2_spritepri[1] = TC0360PRI_regs[6] >> 4;
	f2_spritepri[2] = TC0360PRI_regs[7] & 0x0f;
	f2_spritepri[3] = TC0360PRI_regs[7] >> 4;

	f2_spriteblendmode = TC0360PRI_regs[0]&0xc0;

	fillbitmap(priority_bitmap,0,cliprect);
	fillbitmap(bitmap,machine->pens[0],cliprect);

	TC0480SCP_tilemap_draw(bitmap,cliprect,layer[0],0,1);
	TC0480SCP_tilemap_draw(bitmap,cliprect,layer[1],0,2);
	TC0480SCP_tilemap_draw(bitmap,cliprect,layer[2],0,4);
	TC0480SCP_tilemap_draw(bitmap,cliprect,layer[3],0,8);
	TC0480SCP_tilemap_draw(bitmap,cliprect,layer[4],0,16);

	draw_sprites(machine, bitmap,cliprect,NULL,1);
	return 0;
}


/* Deadconx, Footchmp */
VIDEO_UPDATE( deadconx )
{
	UINT8 layer[5];
	UINT8 tilepri[5];
	UINT8 spritepri[4];
	UINT16 priority;

	taitof2_handle_sprite_buffering();

	TC0480SCP_tilemap_update(machine);

	priority = TC0480SCP_get_bg_priority();

	layer[0] = (priority &0xf000) >> 12;	/* tells us which bg layer is bottom */
	layer[1] = (priority &0x0f00) >>  8;
	layer[2] = (priority &0x00f0) >>  4;
	layer[3] = (priority &0x000f) >>  0;	/* tells us which is top */
	layer[4] = 4;   /* text layer always over bg layers */

	tilepri[0] = TC0360PRI_regs[4] >> 4;      /* bg0 */
	tilepri[1] = TC0360PRI_regs[5] & 0x0f;    /* bg1 */
	tilepri[2] = TC0360PRI_regs[5] >> 4;      /* bg2 */
	tilepri[3] = TC0360PRI_regs[4] & 0x0f;    /* bg3 */

/* we actually assume text layer is on top of everything anyway, but FWIW... */
	tilepri[layer[4]] = TC0360PRI_regs[7] >> 4;    /* fg (text layer) */

	spritepri[0] = TC0360PRI_regs[6] & 0x0f;
	spritepri[1] = TC0360PRI_regs[6] >> 4;
	spritepri[2] = TC0360PRI_regs[7] & 0x0f;
	spritepri[3] = TC0360PRI_regs[7] >> 4;

	fillbitmap(priority_bitmap,0,cliprect);
	fillbitmap(bitmap,machine->pens[0],cliprect);

	TC0480SCP_tilemap_draw(bitmap,cliprect,layer[0],0,1);
	TC0480SCP_tilemap_draw(bitmap,cliprect,layer[1],0,2);
	TC0480SCP_tilemap_draw(bitmap,cliprect,layer[2],0,4);
	TC0480SCP_tilemap_draw(bitmap,cliprect,layer[3],0,8);

	{
		int primasks[4] = {0,0,0,0};
		int i;

		for (i = 0;i < 4;i++)
		{
			if (spritepri[i] < tilepri[(layer[0])]) primasks[i] |= 0xaaaa;
			if (spritepri[i] < tilepri[(layer[1])]) primasks[i] |= 0xcccc;
			if (spritepri[i] < tilepri[(layer[2])]) primasks[i] |= 0xf0f0;
			if (spritepri[i] < tilepri[(layer[3])]) primasks[i] |= 0xff00;
		}

		draw_sprites(machine, bitmap,cliprect,primasks,0);
	}

	/*
    TODO: This isn't the correct way to handle the priority. At the moment of
    writing, pdrawgfx() doesn't support 5 layers, so I have to cheat, assuming
    that the FG layer is always on top of sprites.
    */

	TC0480SCP_tilemap_draw(bitmap,cliprect,layer[4],0,0);
	return 0;
}
