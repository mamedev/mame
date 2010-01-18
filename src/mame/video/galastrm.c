#include "emu.h"
#include "video/taitoic.h"
#include "video/poly.h"

#define X_OFFSET 96
#define Y_OFFSET 60

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
static bitmap_t *tmpbitmaps;
static bitmap_t *polybitmap;
static poly_manager *poly;
INT16 galastrm_tc0610_ctrl_reg[2][8];

typedef struct _poly_extra_data poly_extra_data;
struct _poly_extra_data
{
	bitmap_t *texbase;
};

typedef struct _polygon polygon;
struct _polygon
{
	float x;
	float y;
	float z;
};

/******************************************************************/

static void galastrm_exit(running_machine *machine)
{
	poly_free(poly);
}

VIDEO_START( galastrm )
{
	spritelist = auto_alloc_array(machine, struct tempsprite, 0x4000);

	tmpbitmaps = video_screen_auto_bitmap_alloc(machine->primary_screen);
	polybitmap = video_screen_auto_bitmap_alloc(machine->primary_screen);

	poly = poly_alloc(machine, 16, sizeof(poly_extra_data), POLYFLAG_ALLOW_QUADS);
	add_exit_callback(machine, galastrm_exit);
}

/************************************************************
            SPRITE DRAW ROUTINES

We draw a series of small tiles ("chunks") together to
create each big sprite. The spritemap rom provides the lookup
table for this. The game hardware looks up 16x16 sprite chunks
from the spritemap rom, creating a 64x64 sprite like this:

     0  1  2  3
     4  5  6  7
     8  9 10 11
    12 13 14 15

(where the number is the word offset into the spritemap rom).
It can also create 32x32 sprites.

NB: unused portions of the spritemap rom contain hex FF's.
It is a useful coding check to warn in the log if these
are being accessed. [They can be inadvertently while
spriteram is being tested, take no notice of that.]

Heavy use is made of sprite zooming.

        ***

    Sprite table layout (4 long words per entry)

    ------------------------------------------
     0 | ........ x....... ........ ........ | Flip X
     0 | ........ .xxxxxxx ........ ........ | ZoomX
     0 | ........ ........ .xxxxxxx xxxxxxxx | Sprite Tile
       |                                     |
     2 | ........ ....xx.. ........ ........ | Sprite/tile priority [*]
     2 | ........ ......xx xxxxxx.. ........ | Palette bank
     2 | ........ ........ ......xx xxxxxxxx | X position
       |                                     |
     3 | ........ .....x.. ........ ........ | Sprite size (0=32x32, 1=64x64)
     3 | ........ ......x. ........ ........ | Flip Y
     3 | ........ .......x xxxxxx.. ........ | ZoomY
     3 | ........ ........ ......xx xxxxxxxx | Y position
    ------------------------------------------

    [* 00=over BG1; 01=BG2; 10=BG3; 11=over text ???]

********************************************************/

static struct tempsprite *sprite_ptr_pre;

static void draw_sprites_pre(running_machine *machine, int x_offs, int y_offs)
{
	UINT32 *spriteram32 = machine->generic.spriteram.u32;
	UINT16 *spritemap = (UINT16 *)memory_region(machine, "user1");
	int offs, data, tilenum, color, flipx, flipy;
	int x, y, priority, dblsize, curx, cury;
	int sprites_flipscreen = 0;
	int zoomx, zoomy, zx, zy;
	int sprite_chunk,map_offset,code,j,k,px,py;
	int dimension,total_chunks,bad_chunks;

	/* pdrawgfx() needs us to draw sprites front to back, so we have to build a list
       while processing sprite ram and then draw them all at the end */
	sprite_ptr_pre = spritelist;

	for (offs = (machine->generic.spriteram_size/4-4);offs >= 0;offs -= 4)
	{
		data = spriteram32[offs+0];
		flipx =    (data & 0x00800000) >> 23;
		zoomx =    (data & 0x007f0000) >> 16;
		tilenum =  (data & 0x00007fff);

		if (!tilenum) continue;

		data = spriteram32[offs+2];
		priority = (data & 0x000c0000) >> 18;
		color =    (data & 0x0003fc00) >> 10;
		x =        (data & 0x000003ff);

		data = spriteram32[offs+3];
		dblsize =  (data & 0x00040000) >> 18;
		flipy =    (data & 0x00020000) >> 17;
		zoomy =    (data & 0x0001fc00) >> 10;
		y =        (data & 0x000003ff);

		bad_chunks = 0;
		dimension = ((dblsize*2) + 2);	// 2 or 4
		total_chunks = ((dblsize*3) + 1) << 2;	// 4 or 16
		map_offset = tilenum << 2;

		zoomx += 1;
		zoomy += 1;

		if (x > 713) x -= 1024;		/* 1024x512 */
		if (y < 117) y += 512;

		y = (-y & 0x3ff);
		x -= x_offs;
		y += y_offs;
		if (flipy) y += (128 - zoomy);

		for (sprite_chunk=0;sprite_chunk<total_chunks;sprite_chunk++)
		{
			j = sprite_chunk / dimension;   /* rows */
			k = sprite_chunk % dimension;   /* chunks per row */

			px = k;
			py = j;
			/* pick tiles back to front for x and y flips */
			if (flipx)  px = dimension-1-k;
			if (flipy)  py = dimension-1-j;

			code = spritemap[map_offset + px + (py<<(dblsize+1))];

			if (code==0xffff)
			{
				bad_chunks += 1;
				continue;
			}

			curx = x + ((k*zoomx)/dimension);
			cury = y + ((j*zoomy)/dimension);

			zx= x + (((k+1)*zoomx)/dimension) - curx;
			zy= y + (((j+1)*zoomy)/dimension) - cury;

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

			sprite_ptr_pre->gfx = 0;
			sprite_ptr_pre->code = code;
			sprite_ptr_pre->color = color;
			sprite_ptr_pre->flipx = !flipx;
			sprite_ptr_pre->flipy = flipy;
			sprite_ptr_pre->x = curx;
			sprite_ptr_pre->y = cury;
			sprite_ptr_pre->zoomx = zx << 12;
			sprite_ptr_pre->zoomy = zy << 12;
			sprite_ptr_pre->primask = priority;

			sprite_ptr_pre++;
		}
		if (bad_chunks)
logerror("Sprite number %04x had %02x invalid chunks\n",tilenum,bad_chunks);
	}
}

static void draw_sprites(running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect, const int *primasks, int priority)
{
	struct tempsprite *sprite_ptr = sprite_ptr_pre;

	while (sprite_ptr != spritelist)
	{
		sprite_ptr--;

		if ((priority != 0 && sprite_ptr->primask != 0) ||
			(priority == 0 && sprite_ptr->primask == 0))
		{
			pdrawgfxzoom_transpen(bitmap,cliprect,machine->gfx[sprite_ptr->gfx],
				sprite_ptr->code,
				sprite_ptr->color,
				sprite_ptr->flipx,sprite_ptr->flipy,
				sprite_ptr->x,sprite_ptr->y,
				sprite_ptr->zoomx,sprite_ptr->zoomy,
				machine->priority_bitmap,primasks[sprite_ptr->primask],0);
		}
	}
}

/**************************************************************
                POLYGON RENDERER
**************************************************************/

static void tc0610_draw_scanline(void *dest, INT32 scanline, const poly_extent *extent, const void *extradata, int threadid)
{
	const poly_extra_data *extra = (const poly_extra_data *)extradata;
	bitmap_t *destmap = (bitmap_t *)dest;
	UINT16 *framebuffer = BITMAP_ADDR16(destmap, scanline, 0);
	bitmap_t *texbase = extra->texbase;
	int startx = extent->startx;
	int stopx = extent->stopx;
	INT32 u = extent->param[0].start;
	INT32 v = extent->param[1].start;
	INT32 dudx = extent->param[0].dpdx;
	INT32 dvdx = extent->param[1].dpdx;
	int x;

	for (x = startx; x < stopx; x++)
	{
		framebuffer[x] = *BITMAP_ADDR16(texbase, v >> 16, u >> 16);
		u += dudx;
		v += dvdx;
	}
}

static void tc0610_rotate_draw(running_machine *machine, bitmap_t *bitmap, bitmap_t *srcbitmap, const rectangle *clip)
{
	poly_extra_data *extra = (poly_extra_data *)poly_get_extra_data(poly);
	poly_draw_scanline_func callback;
	poly_vertex vert[4];
	int rsx = galastrm_tc0610_ctrl_reg[1][0];
	int rsy = galastrm_tc0610_ctrl_reg[1][1];
	const int rzx = galastrm_tc0610_ctrl_reg[1][2];
	const int rzy = galastrm_tc0610_ctrl_reg[1][3];
	const int ryx = galastrm_tc0610_ctrl_reg[1][5];
	const int ryy = galastrm_tc0610_ctrl_reg[1][4];
	const int lx  = srcbitmap->width;
	const int ly  = srcbitmap->height;

	static int rsxb=0, rsyb=0, rsxoffs=0, rsyoffs=0;
	int sx, sy, yx, yy, zx, zy, pxx, pxy, pyx, pyy;
	float ssn, scs, ysn, ycs, zsn, zcs;


	pxx = 0;
	pxy = 0;
	pyx = 0;
	pyy = 0;
	zx  = 0;
	zy  = 0;

	if (rzx != 0 || rzy != 0)
	{
		while (sqrt(pow((float)pxx/4096.0, 2.0) + pow((float)pxy/4096.0, 2.0)) < (float)(lx / 2))
		{
			pxx += rzx;
			pxy += rzy;
			zx++;
		}
		while (sqrt(pow((float)pyy/4096.0, 2.0) + pow((float)pyx/4096.0, 2.0)) < (float)(ly / 2))
		{
			pyy += rzx;
			pyx += -rzy;
			zy++;
		}
	}
	zsn = ((float)pyx/4096.0) / (float)(ly / 2);
	zcs = ((float)pxx/4096.0) / (float)(lx / 2);


	if ((rsx == -240 && rsy == 1072) || !galastrm_tc0610_ctrl_reg[1][7])
	{
		rsxoffs = 0;
		rsyoffs = 0;
	}
	else
	{
		if (rsx > rsxb && rsxb < 0 && rsx-rsxb > 0x8000)
		{
			if (rsxoffs == 0)
				rsxoffs = -0x10000;
			else
				rsxoffs = 0;
		}
		if (rsx < rsxb && rsxb > 0 && rsxb-rsx > 0x8000)
		{
			if (rsxoffs == 0)
				rsxoffs = 0x10000-1;
			else
				rsxoffs = 0;
		}
		if (rsy > rsyb && rsyb < 0 && rsy-rsyb > 0x8000)
		{
			if (rsyoffs == 0)
				rsyoffs = -0x10000;
			else
				rsyoffs = 0;
		}
		if (rsy < rsyb && rsyb > 0 && rsyb-rsy > 0x8000)
		{
			if (rsyoffs == 0)
				rsyoffs = 0x10000-1;
			else
				rsyoffs = 0;
		}
	}
	rsxb = rsx;
	rsyb = rsy;
	if (rsxoffs) rsx += rsxoffs;
	if (rsyoffs) rsy += rsyoffs;
	if (rsx < -0x14000 || rsx >= 0x14000) rsxoffs = 0;
	if (rsy < -0x14000 || rsy >= 0x14000) rsyoffs = 0;


	pxx = 0;
	pxy = 0;
	pyx = 0;
	pyy = 0;
	sx  = 0;
	sy  = 0;
	yx  = 0;
	yy  = 0;
	ssn = 0.0;
	scs = 0.0;
	ysn = 0.0;
	ycs = 0.0;

	if (galastrm_tc0610_ctrl_reg[1][7])
	{

		if (ryx != 0 || ryy != 0)
		{
			while (sqrt(pow((float)pxx/4096.0, 2.0) + pow((float)pxy/4096.0, 2.0)) < (float)(lx / 2))
			{
				pxx += ryx;
				pxy += ryy;
				yx++;
			}
			while (sqrt(pow((float)pyy/4096.0, 2.0) + pow((float)pyx/4096.0, 2.0)) < (float)(ly / 2))
			{
				pyy += ryx;
				pyx += -ryy;
				yy++;
			}
			if (yx >= 0.0)
			{
				yx = (int)((8.0 - log((double)yx) / log(2.0)) * 6.0);
				ysn = sin(DEGREE_TO_RADIAN(yx));
				ycs = 1.0 - ysn*ysn;
			}
		}

		pxx = 0;
		pxy = 0;
		pyx = 0;
		pyy = 0;

		if (rsx != 0 || rsy != 0)
		{
			while (sqrt(pow((float)pxx/65536.0, 2.0) + pow((float)pxy/65536.0, 2.0)) < (float)(lx / 2))
			{
				pxx += rsx;
				pxy += rsy;
				sx++;
			}
			while (sqrt(pow((float)pyy/65536.0, 2.0) + pow((float)pyx/65536.0, 2.0)) < (float)(ly / 2))
			{
				pyy += rsx;
				pyx += -rsy;
				sy++;
			}
		}
		ssn = ((float)pxy/65536.0) / (float)(lx / 2);
		scs = ((float)pyy/65536.0) / (float)(ly / 2);
	}


	{
		polygon tmpz[4];

		tmpz[0].x = ((float)(-zx)  * zcs) - ((float)(-zy)  * zsn);
		tmpz[0].y = ((float)(-zx)  * zsn) + ((float)(-zy)  * zcs);
		tmpz[0].z = 0.0;
		tmpz[1].x = ((float)(-zx)  * zcs) - ((float)(zy-1) * zsn);
		tmpz[1].y = ((float)(-zx)  * zsn) + ((float)(zy-1) * zcs);
		tmpz[1].z = 0.0;
		tmpz[2].x = ((float)(zx-1) * zcs) - ((float)(zy-1) * zsn);
		tmpz[2].y = ((float)(zx-1) * zsn) + ((float)(zy-1) * zcs);
		tmpz[2].z = 0.0;
		tmpz[3].x = ((float)(zx-1) * zcs) - ((float)(-zy)  * zsn);
		tmpz[3].y = ((float)(zx-1) * zsn) + ((float)(-zy)  * zcs);
		tmpz[3].z = 0.0;


		vert[0].x = tmpz[0].x + (float)(lx / 2);
		vert[0].y = tmpz[0].y + (float)(ly / 2);
		vert[1].x = tmpz[1].x + (float)(lx / 2);
		vert[1].y = tmpz[1].y + (float)(ly / 2);
		vert[2].x = tmpz[2].x + (float)(lx / 2);
		vert[2].y = tmpz[2].y + (float)(ly / 2);
		vert[3].x = tmpz[3].x + (float)(lx / 2);
		vert[3].y = tmpz[3].y + (float)(ly / 2);
	}

	vert[0].p[0] = 0.0;
	vert[0].p[1] = 0.0;
	vert[1].p[0] = 0.0;
	vert[1].p[1] = (float)(ly - 1) * 65536.0;
	vert[2].p[0] = (float)(lx - 1) * 65536.0;
	vert[2].p[1] = (float)(ly - 1) * 65536.0;
	vert[3].p[0] = (float)(lx - 1) * 65536.0;
	vert[3].p[1] = 0.0;

	extra->texbase = srcbitmap;
	callback = tc0610_draw_scanline;
	poly_render_quad(poly, bitmap, clip, callback, 2, &vert[0], &vert[1], &vert[2], &vert[3]);
}

/**************************************************************
                SCREEN REFRESH
**************************************************************/

VIDEO_UPDATE( galastrm )
{
	running_device *tc0100scn = devtag_get_device(screen->machine, "tc0100scn");
	running_device *tc0480scp = devtag_get_device(screen->machine, "tc0480scp");
	UINT8 layer[5];
	UINT8 pivlayer[3];
	UINT16 priority;
	static const int primasks[4] = {0xfffc, 0xfff0, 0xff00, 0x0};
	rectangle clip;
	bitmap_t *priority_bitmap = screen->machine->priority_bitmap;

	clip.min_x = 0;
	clip.min_y = 0;
	clip.max_x = video_screen_get_width(screen) -1;
	clip.max_y = video_screen_get_height(screen) -1;

	tc0100scn_tilemap_update(tc0100scn);
	tc0480scp_tilemap_update(tc0480scp);

	priority = tc0480scp_get_bg_priority(tc0480scp);
	layer[0] = (priority & 0xf000) >> 12;	/* tells us which bg layer is bottom */
	layer[1] = (priority & 0x0f00) >>  8;
	layer[2] = (priority & 0x00f0) >>  4;
	layer[3] = (priority & 0x000f) >>  0;	/* tells us which is top */
	layer[4] = 4;   /* text layer always over bg layers */

	pivlayer[0] = tc0100scn_bottomlayer(tc0100scn);
	pivlayer[1] = pivlayer[0] ^ 1;
	pivlayer[2] = 2;

	bitmap_fill(bitmap, cliprect, 0);
	bitmap_fill(priority_bitmap, &clip, 0);
	bitmap_fill(tmpbitmaps, &clip, 0);

	tc0100scn_tilemap_draw(tc0100scn, bitmap, cliprect, pivlayer[0], 0, 0);
	tc0100scn_tilemap_draw(tc0100scn, bitmap, cliprect, pivlayer[1], 0, 0);

#if 0
	if (layer[0]==0 && layer[1]==3 && layer[2]==2 && layer[3]==1)
	{
		if (!input_code_pressed(screen->machine, KEYCODE_Z)) tc0480scp_tilemap_draw(tc0480scp, tmpbitmaps, &clip, layer[0], 0, 1);
		if (!input_code_pressed(screen->machine, KEYCODE_X)) tc0480scp_tilemap_draw(tc0480scp, tmpbitmaps, &clip, layer[1], 0, 4);
		if (!input_code_pressed(screen->machine, KEYCODE_C)) tc0480scp_tilemap_draw(tc0480scp, tmpbitmaps, &clip, layer[2], 0, 4);
		if (!input_code_pressed(screen->machine, KEYCODE_V)) tc0480scp_tilemap_draw(tc0480scp, tmpbitmaps, &clip, layer[3], 0, 4);
	}
	else
	{
		if (!input_code_pressed(screen->machine, KEYCODE_Z)) tc0480scp_tilemap_draw(tc0480scp, tmpbitmaps, &clip, layer[0], 0, 1);
		if (!input_code_pressed(screen->machine, KEYCODE_X)) tc0480scp_tilemap_draw(tc0480scp, tmpbitmaps, &clip, layer[1], 0, 2);
		if (!input_code_pressed(screen->machine, KEYCODE_C)) tc0480scp_tilemap_draw(tc0480scp, tmpbitmaps, &clip, layer[2], 0, 4);
		if (!input_code_pressed(screen->machine, KEYCODE_V)) tc0480scp_tilemap_draw(tc0480scp, tmpbitmaps, &clip, layer[3], 0, 8);
	}

	if (layer[0]==3 && layer[1]==0 && layer[2]==1 && layer[3]==2)
	{
		int x,y;
		UINT8 *pri;

		for (y=0; y < priority_bitmap->height; y++)
		{
			for (x=0; x < priority_bitmap->width; x++)
			{
				pri = BITMAP_ADDR8(priority_bitmap, y, x);
				if (!(*pri & 0x02) && *BITMAP_ADDR16(tmpbitmaps, y, x))
					 *pri |= 0x04;
			}
		}
	}

	draw_sprites_pre(screen->machine, 42-X_OFFSET, -571+Y_OFFSET);
	draw_sprites(screen->machine,tmpbitmaps,&clip,primasks,1);

	copybitmap_trans(bitmap,polybitmap,0,0, 0,0,cliprect,0);
	bitmap_fill(polybitmap, &clip, 0);
	tc0610_rotate_draw(screen->machine,polybitmap,tmpbitmaps,cliprect);

	bitmap_fill(priority_bitmap, cliprect, 0);
	draw_sprites(screen->machine,bitmap,cliprect,primasks,0);

	if (!input_code_pressed(screen->machine, KEYCODE_B)) tc0480scp_tilemap_draw(tc0480scp, bitmap, cliprect, layer[4], 0, 0);
	if (!input_code_pressed(screen->machine, KEYCODE_M)) tc0100scn_tilemap_draw(tc0100scn, bitmap, cliprect, pivlayer[2], 0, 0);



#else
	if (layer[0]==0 && layer[1]==3 && layer[2]==2 && layer[3]==1)
	{
		tc0480scp_tilemap_draw(tc0480scp, tmpbitmaps, &clip, layer[0], 0, 1);
		tc0480scp_tilemap_draw(tc0480scp, tmpbitmaps, &clip, layer[1], 0, 4);
		tc0480scp_tilemap_draw(tc0480scp, tmpbitmaps, &clip, layer[2], 0, 4);
		tc0480scp_tilemap_draw(tc0480scp, tmpbitmaps, &clip, layer[3], 0, 4);
	}
	else
	{
		tc0480scp_tilemap_draw(tc0480scp, tmpbitmaps, &clip, layer[0], 0, 1);
		tc0480scp_tilemap_draw(tc0480scp, tmpbitmaps, &clip, layer[1], 0, 2);
		tc0480scp_tilemap_draw(tc0480scp, tmpbitmaps, &clip, layer[2], 0, 4);
		tc0480scp_tilemap_draw(tc0480scp, tmpbitmaps, &clip, layer[3], 0, 8);
	}

	if (layer[0]==3 && layer[1]==0 && layer[2]==1 && layer[3]==2)
	{
		int x,y;
		UINT8 *pri;

		for (y=0; y < priority_bitmap->height; y++)
		{
			for (x=0; x < priority_bitmap->width; x++)
			{
				pri = BITMAP_ADDR8(priority_bitmap, y, x);
				if (!(*pri & 0x02) && *BITMAP_ADDR16(tmpbitmaps, y, x))
					 *pri |= 0x04;
			}
		}
	}

	draw_sprites_pre(screen->machine, 42-X_OFFSET, -571+Y_OFFSET);
	draw_sprites(screen->machine,tmpbitmaps,&clip,primasks,1);

	copybitmap_trans(bitmap,polybitmap,0,0, 0,0,cliprect,0);
	bitmap_fill(polybitmap, &clip, 0);
	tc0610_rotate_draw(screen->machine,polybitmap,tmpbitmaps,cliprect);

	bitmap_fill(priority_bitmap, cliprect, 0);
	draw_sprites(screen->machine,bitmap,cliprect,primasks,0);

	tc0480scp_tilemap_draw(tc0480scp, bitmap, cliprect, layer[4], 0, 0);
	tc0100scn_tilemap_draw(tc0100scn, bitmap, cliprect, pivlayer[2], 0, 0);
#endif

	return 0;
}
