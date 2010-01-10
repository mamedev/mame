/*************************************************************************

    Driver for Midway V-Unit games

**************************************************************************/

#include "emu.h"
#include "cpu/tms34010/tms34010.h"
#include "cpu/adsp2100/adsp2100.h"
#include "audio/williams.h"
#include "video/poly.h"
#include "includes/midvunit.h"


#define WATCH_RENDER		(0)
#define LOG_DMA				(0)


#define DMA_CLOCK			40000000


/* for when we implement DMA timing */
#define DMA_QUEUE_SIZE		273
#define TIME_PER_PIXEL		41e-9



UINT16 *midvunit_videoram;
UINT32 *midvunit_textureram;

static UINT16 video_regs[16];
static UINT16 dma_data[16];
static UINT8 dma_data_index;
static UINT16 page_control;
static UINT8 video_changed;

static emu_timer *scanline_timer;
static poly_manager *poly;

typedef struct _poly_extra_data poly_extra_data;
struct _poly_extra_data
{
	UINT8 *		texbase;
	UINT16		pixdata;
	UINT8		dither;
};



/*************************************
 *
 *  Video system start
 *
 *************************************/

static TIMER_CALLBACK( scanline_timer_cb )
{
	int scanline = param;

	if (scanline != -1)
	{
		cputag_set_input_line(machine, "maincpu", 0, ASSERT_LINE);
		timer_adjust_oneshot(scanline_timer, video_screen_get_time_until_pos(machine->primary_screen, scanline + 1, 0), scanline);
		timer_set(machine, ATTOTIME_IN_HZ(25000000), NULL, -1, scanline_timer_cb);
	}
	else
		cputag_set_input_line(machine, "maincpu", 0, CLEAR_LINE);
}


static void midvunit_exit(running_machine *machine)
{
	poly_free(poly);
}


VIDEO_START( midvunit )
{
	scanline_timer = timer_alloc(machine, scanline_timer_cb, NULL);
	poly = poly_alloc(machine, 4000, sizeof(poly_extra_data), POLYFLAG_ALLOW_QUADS);
	add_exit_callback(machine, midvunit_exit);

	state_save_register_global_array(machine, video_regs);
	state_save_register_global_array(machine, dma_data);
	state_save_register_global(machine, dma_data_index);
	state_save_register_global(machine, page_control);
	state_save_register_global(machine, video_changed);
}



/*************************************
 *
 *  Generic flat quad renderer
 *
 *************************************/

static void render_flat(void *destbase, INT32 scanline, const poly_extent *extent, const void *extradata, int threadid)
{
	const poly_extra_data *extra = (const poly_extra_data *)extradata;
	UINT16 pixdata = extra->pixdata;
	int xstep = extra->dither + 1;
	UINT16 *dest = (UINT16 *)destbase + scanline * 512;
	int startx = extent->startx;
	int x;

	/* if dithering, ensure that we start on an appropriate pixel */
	startx += (scanline ^ startx) & extra->dither;

	/* non-dithered 0 pixels can use a memset */
	if (pixdata == 0 && xstep == 1)
		memset(&dest[startx], 0, 2 * (extent->stopx - startx + 1));

	/* otherwise, we fill manually */
	else
	{
		for (x = startx; x < extent->stopx; x += xstep)
			dest[x] = pixdata;
	}
}



/*************************************
 *
 *  Generic textured quad renderers
 *
 *************************************/

static void render_tex(void *destbase, INT32 scanline, const poly_extent *extent, const void *extradata, int threadid)
{
	const poly_extra_data *extra = (const poly_extra_data *)extradata;
	UINT16 pixdata = extra->pixdata & 0xff00;
	const UINT8 *texbase = extra->texbase;
	int xstep = extra->dither + 1;
	UINT16 *dest = (UINT16 *)destbase + scanline * 512;
	int startx = extent->startx;
	int stopx = extent->stopx;
	INT32 u = extent->param[0].start;
	INT32 v = extent->param[1].start;
	INT32 dudx = extent->param[0].dpdx;
	INT32 dvdx = extent->param[1].dpdx;
	int x;

	/* if dithering, we advance by 2x; also ensure that we start on an appropriate pixel */
	if (xstep == 2)
	{
		if ((scanline ^ startx) & 1)
		{
			startx++;
			u += dudx;
			v += dvdx;
		}
		dudx *= 2;
		dvdx *= 2;
	}

	/* general case; render every pixel */
	for (x = startx; x < stopx; x += xstep)
	{
		dest[x] = pixdata | texbase[((v >> 8) & 0xff00) + (u >> 16)];
		u += dudx;
		v += dvdx;
	}
}


static void render_textrans(void *destbase, INT32 scanline, const poly_extent *extent, const void *extradata, int threadid)
{
	const poly_extra_data *extra = (const poly_extra_data *)extradata;
	UINT16 pixdata = extra->pixdata & 0xff00;
	const UINT8 *texbase = extra->texbase;
	int xstep = extra->dither + 1;
	UINT16 *dest = (UINT16 *)destbase + scanline * 512;
	int startx = extent->startx;
	int stopx = extent->stopx;
	INT32 u = extent->param[0].start;
	INT32 v = extent->param[1].start;
	INT32 dudx = extent->param[0].dpdx;
	INT32 dvdx = extent->param[1].dpdx;
	int x;

	/* if dithering, we advance by 2x; also ensure that we start on an appropriate pixel */
	if (xstep == 2)
	{
		if ((scanline ^ startx) & 1)
		{
			startx++;
			u += dudx;
			v += dvdx;
		}
		dudx *= 2;
		dvdx *= 2;
	}

	/* general case; render every non-zero texel */
	for (x = startx; x < stopx; x += xstep)
	{
		UINT8 pix = texbase[((v >> 8) & 0xff00) + (u >> 16)];
		if (pix != 0)
			dest[x] = pixdata | pix;
		u += dudx;
		v += dvdx;
	}
}


static void render_textransmask(void *destbase, INT32 scanline, const poly_extent *extent, const void *extradata, int threadid)
{
	const poly_extra_data *extra = (const poly_extra_data *)extradata;
	UINT16 pixdata = extra->pixdata;
	const UINT8 *texbase = extra->texbase;
	int xstep = extra->dither + 1;
	UINT16 *dest = (UINT16 *)destbase + scanline * 512;
	int startx = extent->startx;
	int stopx = extent->stopx;
	INT32 u = extent->param[0].start;
	INT32 v = extent->param[1].start;
	INT32 dudx = extent->param[0].dpdx;
	INT32 dvdx = extent->param[1].dpdx;
	int x;

	/* if dithering, we advance by 2x; also ensure that we start on an appropriate pixel */
	if (xstep == 2)
	{
		if ((scanline ^ startx) & 1)
		{
			startx++;
			u += dudx;
			v += dvdx;
		}
		dudx *= 2;
		dvdx *= 2;
	}

	/* general case; every non-zero texel renders pixdata */
	for (x = startx; x < stopx; x += xstep)
	{
		UINT8 pix = texbase[((v >> 8) & 0xff00) + (u >> 16)];
		if (pix != 0)
			dest[x] = pixdata;
		u += dudx;
		v += dvdx;
	}
}



/*************************************
 *
 *  DMA queue processor
 *
 *************************************/

static void make_vertices_inclusive(poly_vertex *vert)
{
	UINT8 rmask = 0, bmask = 0, eqmask = 0;
	int vnum;

	/* build up a mask of right and bottom points */
	/* note we assume clockwise orientation here */
	for (vnum = 0; vnum < 4; vnum++)
	{
		poly_vertex *currv = &vert[vnum];
		poly_vertex *nextv = &vert[(vnum + 1) & 3];

		/* if this vertex equals the next one, tag it */
		if (nextv->y == currv->y && nextv->x == currv->x)
			eqmask |= 1 << vnum;

		/* if the next vertex is down from us, we are a right coordinate */
		if (nextv->y > currv->y || (nextv->y == currv->y && nextv->x < currv->x))
			rmask |= 1 << vnum;

		/* if the next vertex is left from us, we are a bottom coordinate */
		if (nextv->x < currv->x || (nextv->x == currv->x && nextv->y < currv->y))
			bmask |= 1 << vnum;
	}

	/* bail on the edge case */
	if (eqmask == 0x0f)
		return;

	/* adjust the right/bottom points so that they get included */
	for (vnum = 0; vnum < 4; vnum++)
	{
		poly_vertex *currv = &vert[vnum];
		int effvnum = vnum;

		/* if we're equal to the next vertex, use that instead */
		while (eqmask & (1 << effvnum))
			effvnum = (effvnum + 1) & 3;

		/* adjust the points */
		if (rmask & (1 << effvnum))
			currv->x += 0.001f;
		if (bmask & (1 << effvnum))
			currv->y += 0.001f;
	}
}


static void process_dma_queue(running_machine *machine)
{
	poly_extra_data *extra = (poly_extra_data *)poly_get_extra_data(poly);
	UINT16 *dest = &midvunit_videoram[(page_control & 4) ? 0x40000 : 0x00000];
	int textured = ((dma_data[0] & 0x300) == 0x100);
	poly_draw_scanline_func callback;
	poly_vertex vert[4];

	/* if we're rendering to the same page we're viewing, it has changed */
	if ((((page_control >> 2) ^ page_control) & 1) == 0)
		video_changed = TRUE;

	/* fill in the vertex data */
	vert[0].x = (float)(INT16)dma_data[2] + 0.5f;
	vert[0].y = (float)(INT16)dma_data[3] + 0.5f;
	vert[1].x = (float)(INT16)dma_data[4] + 0.5f;
	vert[1].y = (float)(INT16)dma_data[5] + 0.5f;
	vert[2].x = (float)(INT16)dma_data[6] + 0.5f;
	vert[2].y = (float)(INT16)dma_data[7] + 0.5f;
	vert[3].x = (float)(INT16)dma_data[8] + 0.5f;
	vert[3].y = (float)(INT16)dma_data[9] + 0.5f;

	/* make the vertices inclusive of right/bottom points */
	make_vertices_inclusive(vert);

	/* handle flat-shaded quads here */
	if (!textured)
		callback = render_flat;

	/* handle textured quads here */
	else
	{
		/* if textured, add the texture info */
		vert[0].p[0] = (float)(dma_data[10] & 0xff) * 65536.0f + 32768.0f;
		vert[0].p[1] = (float)(dma_data[10] >> 8) * 65536.0f + 32768.0f;
		vert[1].p[0] = (float)(dma_data[11] & 0xff) * 65536.0f + 32768.0f;
		vert[1].p[1] = (float)(dma_data[11] >> 8) * 65536.0f + 32768.0f;
		vert[2].p[0] = (float)(dma_data[12] & 0xff) * 65536.0f + 32768.0f;
		vert[2].p[1] = (float)(dma_data[12] >> 8) * 65536.0f + 32768.0f;
		vert[3].p[0] = (float)(dma_data[13] & 0xff) * 65536.0f + 32768.0f;
		vert[3].p[1] = (float)(dma_data[13] >> 8) * 65536.0f + 32768.0f;

		/* handle non-masked, non-transparent quads */
		if ((dma_data[0] & 0xc00) == 0x000)
			callback = render_tex;

		/* handle non-masked, transparent quads */
		else if ((dma_data[0] & 0xc00) == 0x800)
			callback = render_textrans;

		/* handle masked, transparent quads */
		else if ((dma_data[0] & 0xc00) == 0xc00)
			callback = render_textransmask;

		/* handle masked, non-transparent quads */
		else
			callback = render_flat;
	}

	/* set up the extra data for this triangle */
	extra->texbase = (UINT8 *)midvunit_textureram + (dma_data[14] * 256);
	extra->pixdata = dma_data[1] | (dma_data[0] & 0x00ff);
	extra->dither = ((dma_data[0] & 0x2000) != 0);

	/* render as a quad */
	poly_render_quad(poly, dest, video_screen_get_visible_area(machine->primary_screen), callback, textured ? 2 : 0, &vert[0], &vert[1], &vert[2], &vert[3]);
}



/*************************************
 *
 *  DMA pipe control control
 *
 *************************************/

WRITE32_HANDLER( midvunit_dma_queue_w )
{
	if (LOG_DMA && input_code_pressed(space->machine, KEYCODE_L))
		logerror("%06X:queue(%X) = %08X\n", cpu_get_pc(space->cpu), dma_data_index, data);
	if (dma_data_index < 16)
		dma_data[dma_data_index++] = data;
}


READ32_HANDLER( midvunit_dma_queue_entries_r )
{
	/* always return 0 entries */
	return 0;
}


READ32_HANDLER( midvunit_dma_trigger_r )
{
	if (offset)
	{
		if (LOG_DMA && input_code_pressed(space->machine, KEYCODE_L))
			logerror("%06X:trigger\n", cpu_get_pc(space->cpu));
		process_dma_queue(space->machine);
		dma_data_index = 0;
	}
	return 0;
}



/*************************************
 *
 *  Paging control
 *
 *************************************/

WRITE32_HANDLER( midvunit_page_control_w )
{
	/* watch for the display page to change */
	if ((page_control ^ data) & 1)
	{
		video_changed = TRUE;
		if (LOG_DMA && input_code_pressed(space->machine, KEYCODE_L))
			logerror("##########################################################\n");
		video_screen_update_partial(space->machine->primary_screen, video_screen_get_vpos(space->machine->primary_screen) - 1);
	}
	page_control = data;
}


READ32_HANDLER( midvunit_page_control_r )
{
	return page_control;
}



/*************************************
 *
 *  Video control
 *
 *************************************/

WRITE32_HANDLER( midvunit_video_control_w )
{
	UINT16 old = video_regs[offset];

	/* update the data */
	COMBINE_DATA(&video_regs[offset]);

	/* update the scanline timer */
	if (offset == 0)
		timer_adjust_oneshot(scanline_timer, video_screen_get_time_until_pos(space->machine->primary_screen, (data & 0x1ff) + 1, 0), data & 0x1ff);

	/* if something changed, update our parameters */
	if (old != video_regs[offset] && video_regs[6] != 0 && video_regs[11] != 0)
	{
		rectangle visarea;

		/* derive visible area from blanking */
		visarea.min_x = 0;
		visarea.max_x = (video_regs[6] + video_regs[2] - video_regs[5]) % video_regs[6];
		visarea.min_y = 0;
		visarea.max_y = (video_regs[11] + video_regs[7] - video_regs[10]) % video_regs[11];
		video_screen_configure(space->machine->primary_screen, video_regs[6], video_regs[11], &visarea, HZ_TO_ATTOSECONDS(MIDVUNIT_VIDEO_CLOCK / 2) * video_regs[6] * video_regs[11]);
	}
}


READ32_HANDLER( midvunit_scanline_r )
{
	return video_screen_get_vpos(space->machine->primary_screen);
}



/*************************************
 *
 *  Video RAM access
 *
 *************************************/

WRITE32_HANDLER( midvunit_videoram_w )
{
	poly_wait(poly, "Video RAM write");
	if (!video_changed)
	{
		int visbase = (page_control & 1) ? 0x40000 : 0x00000;
		if ((offset & 0x40000) == visbase)
			video_changed = TRUE;
	}
	COMBINE_DATA(&midvunit_videoram[offset]);
}


READ32_HANDLER( midvunit_videoram_r )
{
	poly_wait(poly, "Video RAM read");
	return midvunit_videoram[offset];
}



/*************************************
 *
 *  Palette RAM access
 *
 *************************************/

WRITE32_HANDLER( midvunit_paletteram_w )
{
	int newword;

	COMBINE_DATA(&space->machine->generic.paletteram.u32[offset]);
	newword = space->machine->generic.paletteram.u32[offset];
	palette_set_color_rgb(space->machine, offset, pal5bit(newword >> 10), pal5bit(newword >> 5), pal5bit(newword >> 0));
}



/*************************************
 *
 *  Texture RAM access
 *
 *************************************/

WRITE32_HANDLER( midvunit_textureram_w )
{
	UINT8 *base = (UINT8 *)midvunit_textureram;
	poly_wait(poly, "Texture RAM write");
	base[offset * 2] = data;
	base[offset * 2 + 1] = data >> 8;
}


READ32_HANDLER( midvunit_textureram_r )
{
	UINT8 *base = (UINT8 *)midvunit_textureram;
	return (base[offset * 2 + 1] << 8) | base[offset * 2];
}




/*************************************
 *
 *  Video system update
 *
 *************************************/

VIDEO_UPDATE( midvunit )
{
	int x, y, width, xoffs;
	UINT32 offset;

	poly_wait(poly, "Refresh Time");

	/* if the video didn't change, indicate as much */
	if (!video_changed)
		return UPDATE_HAS_NOT_CHANGED;
	video_changed = FALSE;

	/* determine the base of the videoram */
#if WATCH_RENDER
	offset = (page_control & 4) ? 0x40000 : 0x00000;
#else
	offset = (page_control & 1) ? 0x40000 : 0x00000;
#endif

	/* determine how many pixels to copy */
	xoffs = cliprect->min_x;
	width = cliprect->max_x - xoffs + 1;

	/* adjust the offset */
	offset += xoffs;
	offset += 512 * (cliprect->min_y - video_screen_get_visible_area(screen)->min_y);

	/* loop over rows */
	for (y = cliprect->min_y; y <= cliprect->max_y; y++)
	{
		UINT16 *dest = (UINT16 *)bitmap->base + y * bitmap->rowpixels + cliprect->min_x;
		for (x = 0; x < width; x++)
			*dest++ = midvunit_videoram[offset + x] & 0x7fff;
		offset += 512;
	}
	return 0;
}
