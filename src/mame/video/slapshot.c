#include "driver.h"
#include "video/taitoic.h"

#define TC0480SCP_GFX_NUM 1
#define TC0100SCN_GFX_NUM 1

struct tempsprite
{
	int gfx;
	int code,color;
	int flipx,flipy;
	int x,y;
	int zoomx,zoomy;
	int primask;
};
static struct tempsprite *spritelist;

static INT32 sprites_disabled,sprites_active_area,sprites_master_scrollx,sprites_master_scrolly;
static int sprites_flipscreen = 0;
static UINT16 *spriteram_buffered,*spriteram_delayed;

static INT32 taito_sprite_type = 0;
UINT16 *taito_sprite_ext;
size_t taito_spriteext_size;
static UINT16 spritebank[8];

static INT32 taito_hide_pixels;



/**********************************************************/

static VIDEO_START( slapshot_core )
{
	int i;

	spriteram_delayed = auto_alloc_array(machine, UINT16, machine->generic.spriteram_size/2);
	spriteram_buffered = auto_alloc_array(machine, UINT16, machine->generic.spriteram_size/2);
	spritelist = auto_alloc_array(machine, struct tempsprite, 0x400);

	if (has_TC0480SCP(machine))	/* it's a tc0480scp game */
		TC0480SCP_vh_start(machine,TC0480SCP_GFX_NUM,taito_hide_pixels,30,9,-1,1,0,2,256);
	else	/* it's a tc0100scn game */
		TC0100SCN_vh_start(machine,1,TC0100SCN_GFX_NUM,taito_hide_pixels,0,0,0,0,0,0);

	TC0360PRI_vh_start(machine);	/* Purely for save-state purposes */

	for (i = 0; i < 8; i ++)
		spritebank[i] = 0x400 * i;

	sprites_disabled = 1;
	sprites_active_area = 0;

	state_save_register_global(machine, taito_hide_pixels);
	state_save_register_global(machine, taito_sprite_type);
	state_save_register_global_array(machine, spritebank);
	state_save_register_global(machine, sprites_disabled);
	state_save_register_global(machine, sprites_active_area);
	state_save_register_global_pointer(machine, spriteram_delayed, machine->generic.spriteram_size/2);
	state_save_register_global_pointer(machine, spriteram_buffered, machine->generic.spriteram_size/2);
}

VIDEO_START( slapshot )
{
	taito_hide_pixels = 3;
	taito_sprite_type = 2;
	VIDEO_START_CALL(slapshot_core);
}


/************************************************************
            SPRITE DRAW ROUTINES
************************************************************/

static void draw_sprites(running_machine *machine, bitmap_t *bitmap,const rectangle *cliprect,int *primasks,int y_offset)
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

    */
	int x,y,off,extoffs;
	int code,color,spritedata,spritecont,flipx,flipy;
	int xcurrent,ycurrent,big_sprite=0;
	int y_no=0, x_no=0, xlatch=0, ylatch=0, last_continuation_tile=0;   /* for zooms */
	UINT32 zoomword, zoomx, zoomy, zx=0, zy=0, zoomxlatch=0, zoomylatch=0;   /* for zooms */
	int scroll1x, scroll1y;
	int scrollx=0, scrolly=0;
	int curx,cury;
	int x_offset;

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

	x_offset = taito_hide_pixels;   /* Get rid of 0-3 unwanted pixels on edge of screen. */
	if (sprites_flipscreen) x_offset = -x_offset;

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
			x_offset = taito_hide_pixels;   /* Get rid of 0-3 unwanted pixels on edge of screen. */
			if (sprites_flipscreen) x_offset = -x_offset;
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


// DG: the bigsprite == 0 check fixes "tied-up" little sprites in Thunderfox
// which (mostly?) have spritecont = 0x20 when they are not continuations
// of anything.
		if (big_sprite == 0 || (spritecont & 0xf0) == 0)
		{
			x = spriteram_buffered[(offs+4)/2];

// DG: some absolute x values deduced here are 1 too high (scenes when you get
// home run in Koshien, and may also relate to BG layer woods and stuff as you
// journey in MjnQuest). You will see they are 1 pixel too far to the right.
// Where is this extra pixel offset coming from??

			if (x & 0x8000)   /* absolute (koshien) */
			{
				scrollx = - x_offset - 0x60;
				scrolly = 0;
			}
			else if (x & 0x4000)   /* ignore extra scroll */
			{
				scrollx = master_scrollx - x_offset - 0x60;
				scrolly = master_scrolly;
			}
			else   /* all scrolls applied */
			{
				scrollx = scroll1x + master_scrollx - x_offset - 0x60;
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

/* Black lines between flames in Gunfront attract before the zoom
   finishes suggest these calculations are flawed? */

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

				x = xlatch + x_no * (0x100 - zoomx) / 16;
				y = ylatch + y_no * (0x100 - zoomy) / 16;
				zx = xlatch + (x_no+1) * (0x100 - zoomx) / 16 - x;
				zy = ylatch + (y_no+1) * (0x100 - zoomy) / 16 - y;
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
		if (extoffs >= 0x8000) extoffs -= 0x4000;   /* spriteram[0x4000-7fff] has no corresponding extension area */

		if (taito_sprite_type == 0)
		{
			int bank;

			code = spriteram_buffered[(offs)/2] & 0x1fff;

			bank = (code & 0x1c00) >> 10;
			code = spritebank[bank] + (code & 0x3ff);
		}

		if (taito_sprite_type == 1)   /* Yuyugogo */
		{
			int i;

			code = spriteram_buffered[(offs)/2] & 0x3ff;
			i = (taito_sprite_ext[(extoffs >> 4)] & 0x3f ) << 10;
			code = (i | code);
		}

		if (taito_sprite_type == 2)   /* Pulirula, Slapshot */
		{
			int i;

			code = spriteram_buffered[(offs)/2] & 0xff;
			i = (taito_sprite_ext[(extoffs >> 4)] & 0xff00 );
			code = (i | code);
		}

		if (taito_sprite_type == 3)   /* Dinorex and a few quizzes */
		{
			int i;

			code = spriteram_buffered[(offs)/2] & 0xff;
			i = (taito_sprite_ext[(extoffs >> 4)] & 0xff ) << 8;
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

			curx = 319 - curx - zx;
			cury = 256 - cury - zy;
			flipx = !flipx;
			flipy = !flipy;
		}

		cury += y_offset;

		{
			sprite_ptr->code = code;
			sprite_ptr->color = color;
			if (machine->gfx[0]->color_granularity == 64)	/* Final Blow, Slapshot are 6bpp */
				sprite_ptr->color /= 4;
			sprite_ptr->flipx = flipx;
			sprite_ptr->flipy = flipy;
			sprite_ptr->x = curx;
			sprite_ptr->y = cury;
			sprite_ptr->zoomx = zx << 12;
			sprite_ptr->zoomy = zy << 12;

			if (primasks)
			{
				sprite_ptr->primask = primasks[(color & 0xc0) >> 6];

				sprite_ptr++;
			}
			else
			{
				drawgfxzoom_transpen(bitmap,cliprect,machine->gfx[0],
						sprite_ptr->code,
						sprite_ptr->color,
						sprite_ptr->flipx,sprite_ptr->flipy,
						sprite_ptr->x,sprite_ptr->y,
						sprite_ptr->zoomx,sprite_ptr->zoomy,0);
			}
		}
	}


	/* this happens only if primsks != NULL */
	while (sprite_ptr != spritelist)
	{
		sprite_ptr--;

		pdrawgfxzoom_transpen(bitmap,cliprect,machine->gfx[0],
				sprite_ptr->code,
				sprite_ptr->color,
				sprite_ptr->flipx,sprite_ptr->flipy,
				sprite_ptr->x,sprite_ptr->y,
				sprite_ptr->zoomx,sprite_ptr->zoomy,
				machine->priority_bitmap,sprite_ptr->primask,0);
	}
}

static int prepare_sprites;

static void taito_handle_sprite_buffering(running_machine *machine)
{
	if (prepare_sprites)	/* no buffering */
	{
		memcpy(spriteram_buffered,machine->generic.spriteram.u16,machine->generic.spriteram_size);
		prepare_sprites = 0;
	}
}

static void taito_update_sprites_active_area(running_machine *machine)
{
	int off;


	/* if the frame was skipped, we'll have to do the buffering now */
	taito_handle_sprite_buffering(machine);

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
			sprites_active_area = 0x8000 * (spriteram_buffered[(offs+10)/2] & 0x0001);
			continue;
		}

		/* check for extra scroll offset */
		if ((spriteram_buffered[(offs+4)/2] & 0xf000) == 0xa000)
		{
			sprites_master_scrollx = spriteram_buffered[(offs+4)/2] & 0xfff;
			if (sprites_master_scrollx >= 0x800) sprites_master_scrollx -= 0x1000;   /* signed value */
			sprites_master_scrolly = spriteram_buffered[(offs+6)/2] & 0xfff;
			if (sprites_master_scrolly >= 0x800) sprites_master_scrolly -= 0x1000;   /* signed value */
		}
	}
}

VIDEO_EOF( taito_no_buffer )
{
	taito_update_sprites_active_area(machine);

	prepare_sprites = 1;
}


/**************************************************************
                SCREEN REFRESH

Slapshot and Metalb use in the PRI chip
---------------------------------------

+4  xxxx0000   BG1
    0000xxxx   BG0
+6  xxxx0000   BG3
    0000xxxx   BG2

Slapshot mostly keeps all sprites above the bg layers.
One exception is the "puck" in early attract which is
a bg layer given priority over some sprites.
********************************************************************/

VIDEO_UPDATE( slapshot )
{
	UINT8 layer[5];
	UINT8 tilepri[5];
	UINT8 spritepri[4];
	UINT16 priority;

#ifdef MAME_DEBUG
	static int dislayer[5];	/* Layer toggles to help get the layers correct */
#endif

#ifdef MAME_DEBUG
	if (input_code_pressed_once (screen->machine, KEYCODE_Z))
	{
		dislayer[0] ^= 1;
		popmessage("bg0: %01x",dislayer[0]);
	}

	if (input_code_pressed_once (screen->machine, KEYCODE_X))
	{
		dislayer[1] ^= 1;
		popmessage("bg1: %01x",dislayer[1]);
	}

	if (input_code_pressed_once (screen->machine, KEYCODE_C))
	{
		dislayer[2] ^= 1;
		popmessage("bg2: %01x",dislayer[2]);
	}

	if (input_code_pressed_once (screen->machine, KEYCODE_V))
	{
		dislayer[3] ^= 1;
		popmessage("bg3: %01x",dislayer[3]);
	}

	if (input_code_pressed_once (screen->machine, KEYCODE_B))
	{
		dislayer[4] ^= 1;
		popmessage("text: %01x",dislayer[4]);
	}
#endif

	taito_handle_sprite_buffering(screen->machine);

	TC0480SCP_tilemap_update(screen->machine);

	priority = TC0480SCP_get_bg_priority();

	layer[0] = (priority &0xf000) >> 12;	/* tells us which bg layer is bottom */
	layer[1] = (priority &0x0f00) >>  8;
	layer[2] = (priority &0x00f0) >>  4;
	layer[3] = (priority &0x000f) >>  0;	/* tells us which is top */
	layer[4] = 4;   /* text layer always over bg layers */

	tilepri[0] = TC0360PRI_regs[4] & 0x0f;     /* bg0 */
	tilepri[1] = TC0360PRI_regs[4] >> 4;       /* bg1 */
	tilepri[2] = TC0360PRI_regs[5] & 0x0f;     /* bg2 */
	tilepri[3] = TC0360PRI_regs[5] >> 4;       /* bg3 */

/* we actually assume text layer is on top of everything anyway, but FWIW... */
	tilepri[layer[4]] = TC0360PRI_regs[7] & 0x0f;    /* fg (text layer) */

	spritepri[0] = TC0360PRI_regs[6] & 0x0f;
	spritepri[1] = TC0360PRI_regs[6] >> 4;
	spritepri[2] = TC0360PRI_regs[7] & 0x0f;
	spritepri[3] = TC0360PRI_regs[7] >> 4;

	bitmap_fill(screen->machine->priority_bitmap,cliprect,0);
	bitmap_fill(bitmap,cliprect,0);

#ifdef MAME_DEBUG
	if (dislayer[layer[0]]==0)
#endif
		TC0480SCP_tilemap_draw(screen->machine,bitmap,cliprect,layer[0],0,1);

#ifdef MAME_DEBUG
	if (dislayer[layer[1]]==0)
#endif
		TC0480SCP_tilemap_draw(screen->machine,bitmap,cliprect,layer[1],0,2);

#ifdef MAME_DEBUG
	if (dislayer[layer[2]]==0)
#endif
		TC0480SCP_tilemap_draw(screen->machine,bitmap,cliprect,layer[2],0,4);

#ifdef MAME_DEBUG
	if (dislayer[layer[3]]==0)
#endif
		TC0480SCP_tilemap_draw(screen->machine,bitmap,cliprect,layer[3],0,8);

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

		draw_sprites(screen->machine,bitmap,cliprect,primasks,0);
	}

	/*
    TODO: This isn't the correct way to handle the priority. At the moment of
    writing, pdrawgfx() doesn't support 5 layers, so I have to cheat, assuming
    that the FG layer is always on top of sprites.
    */

#ifdef MAME_DEBUG
	if (dislayer[layer[4]]==0)
#endif
	TC0480SCP_tilemap_draw(screen->machine,bitmap,cliprect,layer[4],0,0);
	return 0;
}

