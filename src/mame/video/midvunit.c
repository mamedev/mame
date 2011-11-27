/*************************************************************************

    Driver for Midway V-Unit games

**************************************************************************/

#include "emu.h"
#include "cpu/tms34010/tms34010.h"
#include "cpu/adsp2100/adsp2100.h"
#include "audio/williams.h"
#include "includes/midvunit.h"


#define WATCH_RENDER		(0)
#define LOG_DMA				(0)


#define DMA_CLOCK			40000000


/* for when we implement DMA timing */
#define DMA_QUEUE_SIZE		273
#define TIME_PER_PIXEL		41e-9


midvunit_renderer::midvunit_renderer(midvunit_state &state)
	: poly_manager<float, midvunit_object_data, 2, 4000>(state.machine()),
	  m_state(state) { }


/*************************************
 *
 *  Video system start
 *
 *************************************/

static TIMER_CALLBACK( scanline_timer_cb )
{
	midvunit_state *state = machine.driver_data<midvunit_state>();
	int scanline = param;

	if (scanline != -1)
	{
		cputag_set_input_line(machine, "maincpu", 0, ASSERT_LINE);
		state->m_scanline_timer->adjust(machine.primary_screen->time_until_pos(scanline + 1), scanline);
		machine.scheduler().timer_set(attotime::from_hz(25000000), FUNC(scanline_timer_cb), -1);
	}
	else
		cputag_set_input_line(machine, "maincpu", 0, CLEAR_LINE);
}


VIDEO_START( midvunit )
{
	midvunit_state *state = machine.driver_data<midvunit_state>();
	state->m_scanline_timer = machine.scheduler().timer_alloc(FUNC(scanline_timer_cb));

	state->m_poly = auto_alloc(machine, midvunit_renderer(*state));

	state_save_register_global_array(machine, state->m_video_regs);
	state_save_register_global_array(machine, state->m_dma_data);
	state_save_register_global(machine, state->m_dma_data_index);
	state_save_register_global(machine, state->m_page_control);
	state_save_register_global(machine, state->m_video_changed);
}



/*************************************
 *
 *  Generic flat quad renderer
 *
 *************************************/

void midvunit_renderer::render_flat(INT32 scanline, const extent_t &extent, const midvunit_object_data &objectdata, int threadid)
{
	UINT16 pixdata = objectdata.pixdata;
	int xstep = objectdata.dither + 1;
	UINT16 *dest = objectdata.destbase + scanline * 512;
	int startx = extent.startx;
	int x;

	/* if dithering, ensure that we start on an appropriate pixel */
	startx += (scanline ^ startx) & objectdata.dither;

	/* non-dithered 0 pixels can use a memset */
	if (pixdata == 0 && xstep == 1)
		memset(&dest[startx], 0, 2 * (extent.stopx - startx + 1));

	/* otherwise, we fill manually */
	else
	{
		for (x = startx; x < extent.stopx; x += xstep)
			dest[x] = pixdata;
	}
}



/*************************************
 *
 *  Generic textured quad renderers
 *
 *************************************/

void midvunit_renderer::render_tex(INT32 scanline, const extent_t &extent, const midvunit_object_data &objectdata, int threadid)
{
	UINT16 pixdata = objectdata.pixdata & 0xff00;
	const UINT8 *texbase = objectdata.texbase;
	int xstep = objectdata.dither + 1;
	UINT16 *dest = objectdata.destbase + scanline * 512;
	int startx = extent.startx;
	int stopx = extent.stopx;
	INT32 u = extent.param[0].start;
	INT32 v = extent.param[1].start;
	INT32 dudx = extent.param[0].dpdx;
	INT32 dvdx = extent.param[1].dpdx;
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


void midvunit_renderer::render_textrans(INT32 scanline, const extent_t &extent, const midvunit_object_data &objectdata, int threadid)
{
	UINT16 pixdata = objectdata.pixdata & 0xff00;
	const UINT8 *texbase = objectdata.texbase;
	int xstep = objectdata.dither + 1;
	UINT16 *dest = objectdata.destbase + scanline * 512;
	int startx = extent.startx;
	int stopx = extent.stopx;
	INT32 u = extent.param[0].start;
	INT32 v = extent.param[1].start;
	INT32 dudx = extent.param[0].dpdx;
	INT32 dvdx = extent.param[1].dpdx;
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


void midvunit_renderer::render_textransmask(INT32 scanline, const extent_t &extent, const midvunit_object_data &objectdata, int threadid)
{
	UINT16 pixdata = objectdata.pixdata;
	const UINT8 *texbase = objectdata.texbase;
	int xstep = objectdata.dither + 1;
	UINT16 *dest = objectdata.destbase + scanline * 512;
	int startx = extent.startx;
	int stopx = extent.stopx;
	INT32 u = extent.param[0].start;
	INT32 v = extent.param[1].start;
	INT32 dudx = extent.param[0].dpdx;
	INT32 dvdx = extent.param[1].dpdx;
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

void midvunit_renderer::make_vertices_inclusive(vertex_t *vert)
{
	/* build up a mask of right and bottom points */
	/* note we assume clockwise orientation here */
	UINT8 rmask = 0, bmask = 0, eqmask = 0;
	for (int vnum = 0; vnum < 4; vnum++)
	{
		vertex_t *currv = &vert[vnum];
		vertex_t *nextv = &vert[(vnum + 1) & 3];

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
	for (int vnum = 0; vnum < 4; vnum++)
	{
		vertex_t *currv = &vert[vnum];
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


void midvunit_renderer::process_dma_queue()
{
	/* if we're rendering to the same page we're viewing, it has changed */
	if ((((m_state.m_page_control >> 2) ^ m_state.m_page_control) & 1) == 0 || WATCH_RENDER)
		m_state.m_video_changed = TRUE;

	/* fill in the vertex data */
	vertex_t vert[4];
	vert[0].x = (float)(INT16)m_state.m_dma_data[2] + 0.5f;
	vert[0].y = (float)(INT16)m_state.m_dma_data[3] + 0.5f;
	vert[1].x = (float)(INT16)m_state.m_dma_data[4] + 0.5f;
	vert[1].y = (float)(INT16)m_state.m_dma_data[5] + 0.5f;
	vert[2].x = (float)(INT16)m_state.m_dma_data[6] + 0.5f;
	vert[2].y = (float)(INT16)m_state.m_dma_data[7] + 0.5f;
	vert[3].x = (float)(INT16)m_state.m_dma_data[8] + 0.5f;
	vert[3].y = (float)(INT16)m_state.m_dma_data[9] + 0.5f;

	/* make the vertices inclusive of right/bottom points */
	make_vertices_inclusive(vert);

	/* handle flat-shaded quads here */
	render_delegate callback;
	bool textured = ((m_state.m_dma_data[0] & 0x300) == 0x100);
	if (!textured)
		callback = render_delegate(FUNC(midvunit_renderer::render_flat), this);

	/* handle textured quads here */
	else
	{
		/* if textured, add the texture info */
		vert[0].p[0] = (float)(m_state.m_dma_data[10] & 0xff) * 65536.0f + 32768.0f;
		vert[0].p[1] = (float)(m_state.m_dma_data[10] >> 8) * 65536.0f + 32768.0f;
		vert[1].p[0] = (float)(m_state.m_dma_data[11] & 0xff) * 65536.0f + 32768.0f;
		vert[1].p[1] = (float)(m_state.m_dma_data[11] >> 8) * 65536.0f + 32768.0f;
		vert[2].p[0] = (float)(m_state.m_dma_data[12] & 0xff) * 65536.0f + 32768.0f;
		vert[2].p[1] = (float)(m_state.m_dma_data[12] >> 8) * 65536.0f + 32768.0f;
		vert[3].p[0] = (float)(m_state.m_dma_data[13] & 0xff) * 65536.0f + 32768.0f;
		vert[3].p[1] = (float)(m_state.m_dma_data[13] >> 8) * 65536.0f + 32768.0f;

		/* handle non-masked, non-transparent quads */
		if ((m_state.m_dma_data[0] & 0xc00) == 0x000)
			callback = render_delegate(FUNC(midvunit_renderer::render_tex), this);

		/* handle non-masked, transparent quads */
		else if ((m_state.m_dma_data[0] & 0xc00) == 0x800)
			callback = render_delegate(FUNC(midvunit_renderer::render_textrans), this);

		/* handle masked, transparent quads */
		else if ((m_state.m_dma_data[0] & 0xc00) == 0xc00)
			callback = render_delegate(FUNC(midvunit_renderer::render_textransmask), this);

		/* handle masked, non-transparent quads */
		else
			callback = render_delegate(FUNC(midvunit_renderer::render_flat), this);
	}

	/* set up the object data for this triangle */
	midvunit_object_data &objectdata = object_data_alloc();
	objectdata.destbase = &m_state.m_videoram[(m_state.m_page_control & 4) ? 0x40000 : 0x00000];
	objectdata.texbase = (UINT8 *)m_state.m_textureram + (m_state.m_dma_data[14] * 256);
	objectdata.pixdata = m_state.m_dma_data[1] | (m_state.m_dma_data[0] & 0x00ff);
	objectdata.dither = ((m_state.m_dma_data[0] & 0x2000) != 0);

	/* render as a quad */
	render_polygon<4>(machine().primary_screen->visible_area(), callback, textured ? 2 : 0, vert);
}



/*************************************
 *
 *  DMA pipe control control
 *
 *************************************/

WRITE32_HANDLER( midvunit_dma_queue_w )
{
	midvunit_state *state = space->machine().driver_data<midvunit_state>();
	if (LOG_DMA && space->machine().input().code_pressed(KEYCODE_L))
		logerror("%06X:queue(%X) = %08X\n", cpu_get_pc(&space->device()), state->m_dma_data_index, data);
	if (state->m_dma_data_index < 16)
		state->m_dma_data[state->m_dma_data_index++] = data;
}


READ32_HANDLER( midvunit_dma_queue_entries_r )
{
	/* always return 0 entries */
	return 0;
}


READ32_HANDLER( midvunit_dma_trigger_r )
{
	midvunit_state *state = space->machine().driver_data<midvunit_state>();
	if (offset)
	{
		if (LOG_DMA && space->machine().input().code_pressed(KEYCODE_L))
			logerror("%06X:trigger\n", cpu_get_pc(&space->device()));
		state->m_poly->process_dma_queue();
		state->m_dma_data_index = 0;
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
	midvunit_state *state = space->machine().driver_data<midvunit_state>();
	/* watch for the display page to change */
	if ((state->m_page_control ^ data) & 1)
	{
		state->m_video_changed = TRUE;
		if (LOG_DMA && space->machine().input().code_pressed(KEYCODE_L))
			logerror("##########################################################\n");
		space->machine().primary_screen->update_partial(space->machine().primary_screen->vpos() - 1);
	}
	state->m_page_control = data;
}


READ32_HANDLER( midvunit_page_control_r )
{
	midvunit_state *state = space->machine().driver_data<midvunit_state>();
	return state->m_page_control;
}



/*************************************
 *
 *  Video control
 *
 *************************************/

WRITE32_HANDLER( midvunit_video_control_w )
{
	midvunit_state *state = space->machine().driver_data<midvunit_state>();
	UINT16 old = state->m_video_regs[offset];

	/* update the data */
	COMBINE_DATA(&state->m_video_regs[offset]);

	/* update the scanline timer */
	if (offset == 0)
		state->m_scanline_timer->adjust(space->machine().primary_screen->time_until_pos((data & 0x1ff) + 1, 0), data & 0x1ff);

	/* if something changed, update our parameters */
	if (old != state->m_video_regs[offset] && state->m_video_regs[6] != 0 && state->m_video_regs[11] != 0)
	{
		rectangle visarea;

		/* derive visible area from blanking */
		visarea.min_x = 0;
		visarea.max_x = (state->m_video_regs[6] + state->m_video_regs[2] - state->m_video_regs[5]) % state->m_video_regs[6];
		visarea.min_y = 0;
		visarea.max_y = (state->m_video_regs[11] + state->m_video_regs[7] - state->m_video_regs[10]) % state->m_video_regs[11];
		space->machine().primary_screen->configure(state->m_video_regs[6], state->m_video_regs[11], visarea, HZ_TO_ATTOSECONDS(MIDVUNIT_VIDEO_CLOCK / 2) * state->m_video_regs[6] * state->m_video_regs[11]);
	}
}


READ32_HANDLER( midvunit_scanline_r )
{
	return space->machine().primary_screen->vpos();
}



/*************************************
 *
 *  Video RAM access
 *
 *************************************/

WRITE32_HANDLER( midvunit_videoram_w )
{
	midvunit_state *state = space->machine().driver_data<midvunit_state>();
	state->m_poly->wait("Video RAM write");
	if (!state->m_video_changed)
	{
		int visbase = (state->m_page_control & 1) ? 0x40000 : 0x00000;
		if ((offset & 0x40000) == visbase)
			state->m_video_changed = TRUE;
	}
	COMBINE_DATA(&state->m_videoram[offset]);
}


READ32_HANDLER( midvunit_videoram_r )
{
	midvunit_state *state = space->machine().driver_data<midvunit_state>();
	state->m_poly->wait("Video RAM read");
	return state->m_videoram[offset];
}



/*************************************
 *
 *  Palette RAM access
 *
 *************************************/

WRITE32_HANDLER( midvunit_paletteram_w )
{
	int newword;

	COMBINE_DATA(&space->machine().generic.paletteram.u32[offset]);
	newword = space->machine().generic.paletteram.u32[offset];
	palette_set_color_rgb(space->machine(), offset, pal5bit(newword >> 10), pal5bit(newword >> 5), pal5bit(newword >> 0));
}



/*************************************
 *
 *  Texture RAM access
 *
 *************************************/

WRITE32_HANDLER( midvunit_textureram_w )
{
	midvunit_state *state = space->machine().driver_data<midvunit_state>();
	UINT8 *base = (UINT8 *)state->m_textureram;
	state->m_poly->wait("Texture RAM write");
	base[offset * 2] = data;
	base[offset * 2 + 1] = data >> 8;
}


READ32_HANDLER( midvunit_textureram_r )
{
	midvunit_state *state = space->machine().driver_data<midvunit_state>();
	UINT8 *base = (UINT8 *)state->m_textureram;
	return (base[offset * 2 + 1] << 8) | base[offset * 2];
}




/*************************************
 *
 *  Video system update
 *
 *************************************/

SCREEN_UPDATE( midvunit )
{
	midvunit_state *state = screen->machine().driver_data<midvunit_state>();
	int x, y, width, xoffs;
	UINT32 offset;

	state->m_poly->wait("Refresh Time");

	/* if the video didn't change, indicate as much */
	if (!state->m_video_changed)
		return UPDATE_HAS_NOT_CHANGED;
	state->m_video_changed = FALSE;

	/* determine the base of the videoram */
#if WATCH_RENDER
	offset = (state->m_page_control & 4) ? 0x40000 : 0x00000;
#else
	offset = (state->m_page_control & 1) ? 0x40000 : 0x00000;
#endif

	/* determine how many pixels to copy */
	xoffs = cliprect->min_x;
	width = cliprect->max_x - xoffs + 1;

	/* adjust the offset */
	offset += xoffs;
	offset += 512 * (cliprect->min_y - screen->visible_area().min_y);

	/* loop over rows */
	for (y = cliprect->min_y; y <= cliprect->max_y; y++)
	{
		UINT16 *dest = (UINT16 *)bitmap->base + y * bitmap->rowpixels + cliprect->min_x;
		for (x = 0; x < width; x++)
			*dest++ = state->m_videoram[offset + x] & 0x7fff;
		offset += 512;
	}
	return 0;
}
