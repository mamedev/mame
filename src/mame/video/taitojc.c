#include "emu.h"
#include "video/poly.h"
#include "includes/taitojc.h"

typedef struct _poly_extra_data poly_extra_data;
struct _poly_extra_data
{
	bitmap_t *zbuffer;
	UINT8 *texture;

	int tex_base_x;
	int tex_base_y;
	int tex_wrap_x;
	int tex_wrap_y;
};



#define TAITOJC_NUM_TILES		0x80

static const gfx_layout taitojc_char_layout =
{
	16,16,
	TAITOJC_NUM_TILES,
	4,
	{ 0,1,2,3 },
	{ 24,28,16,20,8,12,0,4, 56,60,48,52,40,44,32,36 },
	{ 0*64, 1*64, 2*64, 3*64, 4*64, 5*64, 6*64, 7*64,8*64,9*64,10*64,11*64,12*64,13*64,14*64,15*64 },
	16*64
};

static TILE_GET_INFO( taitojc_tile_info )
{
	taitojc_state *state = (taitojc_state *)machine->driver_data;

	UINT32 val = state->tile_ram[tile_index];
	int color = (val >> 22) & 0xff;
	int tile = (val >> 2) & 0x7f;
	SET_TILE_INFO(state->gfx_index, tile, color, 0);
}

READ32_HANDLER(taitojc_tile_r)
{
	taitojc_state *state = (taitojc_state *)space->machine->driver_data;

	return state->tile_ram[offset];
}

READ32_HANDLER(taitojc_char_r)
{
	taitojc_state *state = (taitojc_state *)space->machine->driver_data;

	return state->char_ram[offset];
}

WRITE32_HANDLER(taitojc_tile_w)
{
	taitojc_state *state = (taitojc_state *)space->machine->driver_data;

	COMBINE_DATA(state->tile_ram + offset);
	tilemap_mark_tile_dirty(state->tilemap, offset);
}

WRITE32_HANDLER(taitojc_char_w)
{
	taitojc_state *state = (taitojc_state *)space->machine->driver_data;

	COMBINE_DATA(state->char_ram + offset);
	gfx_element_mark_dirty(space->machine->gfx[state->gfx_index], offset/32);
}

// Object data format:
//
// 0x00:   xxxxxx-- -------- -------- --------   Height
// 0x00:   ------xx xxxxxxxx -------- --------   Y
// 0x00:   -------- -------- xxxxxx-- --------   Width
// 0x00:   -------- -------- ------xx xxxxxxxx   X
// 0x01:   ---xxxxx xx------ -------- --------   Palette
// 0x01:   -------- --x----- -------- --------   Priority (0 = below 3D, 1 = above 3D)
// 0x01:   -------- -------- -xxxxxxx xxxxxxxx   VRAM data address

static void draw_object(running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect, UINT32 w1, UINT32 w2)
{
	taitojc_state *state = (taitojc_state *)machine->driver_data;
	int x, y, width, height, palette;
	int i, j;
	int x1, x2, y1, y2;
	int ix, iy;
	UINT32 address;
	UINT8 *v;

	address		= (w2 & 0x7fff) * 0x20;
	if (w2 & 0x4000)
		address |= 0x40000;

	x			= ((w1 >>  0) & 0x3ff);
	if (x & 0x200)
		x |= ~0x1ff;		// sign-extend

	y			= ((w1 >> 16) & 0x3ff);
	if (y & 0x200)
		y |= ~0x1ff;		// sign-extend

	width		= ((w1 >> 10) & 0x3f) * 16;
	height		= ((w1 >> 26) & 0x3f) * 16;
	palette		= ((w2 >> 22) & 0x7f) << 8;

	v = (UINT8*)&state->vram[address/4];

	if (address >= 0xf8000 || width == 0 || height == 0)
		return;

	x1 = x;
	x2 = x + width;
	y1 = y;
	y2 = y + height;

	// trivial rejection
	if (x1 > cliprect->max_x || x2 < cliprect->min_x || y1 > cliprect->max_y || y2 < cliprect->min_y)
	{
		return;
	}

//  mame_printf_debug("draw_object: %08X %08X, X: %d, Y: %d, W: %d, H: %d\n", w1, w2, x, y, width, height);

	ix = 0;
	iy = 0;

	// clip
	if (x1 < cliprect->min_x)
	{
		ix = abs(cliprect->min_x - x1);
		x1 = cliprect->min_x;
	}
	if (x2 > cliprect->max_x)
	{
		x2 = cliprect->max_x;
	}
	if (y1 < cliprect->min_y)
	{
		iy = abs(cliprect->min_y - y1);
		y1 = cliprect->min_y;
	}
	if (y2 > cliprect->max_y)
	{
		y2 = cliprect->max_y;
	}

	for (j=y1; j < y2; j++)
	{
		UINT16 *d = BITMAP_ADDR16(bitmap, j,  0);
		int index = (iy * width) + ix;

		for (i=x1; i < x2; i++)
		{
			UINT8 pen = v[BYTE4_XOR_BE(index)];
			if (pen != 0)
			{
				d[i] = palette + pen;
			}
			index++;
		}

		iy++;
	}
}

static void taitojc_exit(running_machine *machine)
{
	taitojc_state *state = (taitojc_state *)machine->driver_data;
	poly_free(state->poly);
}

VIDEO_START( taitojc )
{
	taitojc_state *state = (taitojc_state *)machine->driver_data;
	int width, height;

	state->poly = poly_alloc(machine, 4000, sizeof(poly_extra_data), POLYFLAG_ALLOW_QUADS);
	add_exit_callback(machine, taitojc_exit);

	/* find first empty slot to decode gfx */
	for (state->gfx_index = 0; state->gfx_index < MAX_GFX_ELEMENTS; state->gfx_index++)
		if (machine->gfx[state->gfx_index] == 0)
			break;

	assert(state->gfx_index != MAX_GFX_ELEMENTS);

	state->tilemap = tilemap_create(machine, taitojc_tile_info, tilemap_scan_rows, 16, 16, 64, 64);

	tilemap_set_transparent_pen(state->tilemap, 0);

	state->char_ram = auto_alloc_array_clear(machine, UINT32, 0x4000/4);
	state->tile_ram = auto_alloc_array_clear(machine, UINT32, 0x4000/4);

	/* create the char set (gfx will then be updated dynamically from RAM) */
	machine->gfx[state->gfx_index] = gfx_element_alloc(machine, &taitojc_char_layout, (UINT8 *)state->char_ram, machine->config->total_colors / 16, 0);

	state->texture = auto_alloc_array(machine, UINT8, 0x400000);

	state->framebuffer = video_screen_auto_bitmap_alloc(machine->primary_screen);

	width = video_screen_get_width(machine->primary_screen);
	height = video_screen_get_height(machine->primary_screen);
	state->zbuffer = auto_bitmap_alloc(machine, width, height, BITMAP_FORMAT_INDEXED16);
}

//static int tick = 0;
VIDEO_UPDATE( taitojc )
{
	taitojc_state *state = (taitojc_state *)screen->machine->driver_data;
	int i;

#if 0
    tick++;
    if( tick >= 5 ) {
        tick = 0;

        if( input_code_pressed(screen->machine, KEYCODE_O) )
            debug_tex_pal++;

        if( input_code_pressed(screen->machine, KEYCODE_I) )
            debug_tex_pal--;

        debug_tex_pal &= 0x7f;
    }
#endif

	bitmap_fill(bitmap, cliprect, 0);

	for (i=(0xc00/4)-2; i >= 0; i-=2)
	{
		UINT32 w1 = state->objlist[i + 0];
		UINT32 w2 = state->objlist[i + 1];

		if ((w2 & 0x200000) == 0)
		{
			draw_object(screen->machine, bitmap, cliprect, w1, w2);
		}
	}

	copybitmap_trans(bitmap, state->framebuffer, 0, 0, 0, 0, cliprect, 0);

	for (i=(0xc00/4)-2; i >= 0; i-=2)
	{
		UINT32 w1 = state->objlist[i + 0];
		UINT32 w2 = state->objlist[i + 1];

		if ((w2 & 0x200000) != 0)
		{
			draw_object(screen->machine, bitmap, cliprect, w1, w2);
		}
	}

	tilemap_draw(bitmap, cliprect, state->tilemap, 0,0);

#if 0
    if (debug_tex_pal > 0)
    {
        int j;
        for (j=cliprect->min_y; j <= cliprect->max_y; j++)
        {
            UINT16 *d = BITMAP_ADDR16(bitmap, j, 0);
            int index = 2048 * j;

            for (i=cliprect->min_x; i <= cliprect->max_x; i++)
            {
                UINT8 t = state->texture[index+i];
                UINT32 color;

                //color = 0xff000000 | (t << 16) | (t << 8) | (t);
                color = (state->debug_tex_pal << 8) | t;

                d[i] = color;
            }
        }

        {
            char string[200];
            sprintf(string, "Texture palette %d", debug_tex_pal);
            popmessage("%s", string);
        }
    }
#endif

	return 0;
}




static void render_solid_scan(void *dest, INT32 scanline, const poly_extent *extent, const void *extradata, int threadid)
{
	const poly_extra_data *extra = (const poly_extra_data *)extradata;
	bitmap_t *destmap = (bitmap_t *)dest;
	float z = extent->param[0].start;
	int color = extent->param[1].start;
	float dz = extent->param[0].dpdx;
	UINT16 *fb = BITMAP_ADDR16(destmap, scanline, 0);
	UINT16 *zb = BITMAP_ADDR16(extra->zbuffer, scanline, 0);
	int x;

	for (x = extent->startx; x < extent->stopx; x++)
	{
		int iz = (int)z & 0xffff;

		if (iz <= zb[x])
		{
			fb[x] = color;
			zb[x] = iz;
		}

		z += dz;
	}
}

static void render_shade_scan(void *dest, INT32 scanline, const poly_extent *extent, const void *extradata, int threadid)
{
	const poly_extra_data *extra = (const poly_extra_data *)extradata;
	bitmap_t *destmap = (bitmap_t *)dest;
	float z = extent->param[0].start;
	float color = extent->param[1].start;
	float dz = extent->param[0].dpdx;
	float dcolor = extent->param[1].dpdx;
	UINT16 *fb = BITMAP_ADDR16(destmap, scanline, 0);
	UINT16 *zb = BITMAP_ADDR16(extra->zbuffer, scanline, 0);
	int x;

	for (x = extent->startx; x < extent->stopx; x++)
	{
		int ic = (int)color & 0xffff;
		int iz = (int)z & 0xffff;

		if (iz <= zb[x])
		{
			fb[x] = ic;
			zb[x] = iz;
		}

		color += dcolor;
		z += dz;
	}
}

static void render_texture_scan(void *dest, INT32 scanline, const poly_extent *extent, const void *extradata, int threadid)
{
	const poly_extra_data *extra = (const poly_extra_data *)extradata;
	bitmap_t *destmap = (bitmap_t *)dest;
	float z = extent->param[0].start;
	float u = extent->param[1].start;
	float v = extent->param[2].start;
	float color = extent->param[3].start;
	float dz = extent->param[0].dpdx;
	float du = extent->param[1].dpdx;
	float dv = extent->param[2].dpdx;
	float dcolor = extent->param[3].dpdx;
	UINT16 *fb = BITMAP_ADDR16(destmap, scanline, 0);
	UINT16 *zb = BITMAP_ADDR16(extra->zbuffer, scanline, 0);
	int tex_wrap_x = extra->tex_wrap_x;
	int tex_wrap_y = extra->tex_wrap_y;
	int tex_base_x = extra->tex_base_x;
	int tex_base_y = extra->tex_base_y;
	int x;

	for (x = extent->startx; x < extent->stopx; x++)
	{
		int iu, iv;
		UINT8 texel;
		int palette = ((int)color & 0x7f) << 8;
		int iz = (int)z & 0xffff;

		if (!tex_wrap_x)
		{
			iu = ((int)u >> 4) & 0x7ff;
		}
		else
		{
			iu = (tex_base_x + (((int)u >> 4) & 0x3f)) & 0x7ff;
		}

		if (!tex_wrap_y)
		{
			iv = ((int)v >> 4) & 0x7ff;
		}
		else
		{
			iv = (tex_base_y + (((int)v >> 4) & 0x3f)) & 0x7ff;
		}

		texel = extra->texture[(iv * 2048) + iu];

		if (iz <= zb[x] && texel != 0)
		{
			fb[x] = palette | texel;
			zb[x] = iz;
		}

		u += du;
		v += dv;
		color += dcolor;
		z += dz;
	}
}

void taitojc_render_polygons(running_machine *machine, UINT16 *polygon_fifo, int length)
{
	taitojc_state *state = (taitojc_state *)machine->driver_data;
	poly_vertex vert[4];
	int i;
	int ptr;

	ptr = 0;
	while (ptr < length)
	{
		UINT16 cmd = polygon_fifo[ptr++];

		switch (cmd & 0x7)
		{
			case 0x03:		// Textured Triangle
			{
				// 0x00: Command ID (0x0003)
				// 0x01: Texture base
				// 0x02: Vertex 1 Palette
				// 0x03: Vertex 1 V
				// 0x04: Vertex 1 U
				// 0x05: Vertex 1 Y
				// 0x06: Vertex 1 X
				// 0x07: Vertex 1 Z
				// 0x08: Vertex 2 Palette
				// 0x09: Vertex 2 V
				// 0x0a: Vertex 2 U
				// 0x0b: Vertex 2 Y
				// 0x0c: Vertex 2 X
				// 0x0d: Vertex 2 Z
				// 0x0e: Vertex 3 Palette
				// 0x0f: Vertex 3 V
				// 0x10: Vertex 3 U
				// 0x11: Vertex 3 Y
				// 0x12: Vertex 3 X
				// 0x13: Vertex 3 Z

				poly_extra_data *extra = (poly_extra_data *)poly_get_extra_data(state->poly);
				UINT16 texbase;

				/*
                printf("CMD3: ");
                for (i=0; i < 0x13; i++)
                {
                    printf("%04X ", polygon_fifo[ptr+i]);
                }
                printf("\n");
                */

				texbase = polygon_fifo[ptr++];

				extra->zbuffer = state->zbuffer;
				extra->texture = state->texture;
				extra->tex_base_x = ((texbase >> 0) & 0xff) << 4;
				extra->tex_base_y = ((texbase >> 8) & 0xff) << 4;

				extra->tex_wrap_x = (cmd & 0xc0) ? 1 : 0;
				extra->tex_wrap_y = (cmd & 0x30) ? 1 : 0;

				for (i=0; i < 3; i++)
				{
					vert[i].p[3] = polygon_fifo[ptr++];	// palette
					vert[i].p[2] = (UINT16)(polygon_fifo[ptr++]);
					vert[i].p[1] = (UINT16)(polygon_fifo[ptr++]);
					vert[i].y =  (INT16)(polygon_fifo[ptr++]);
					vert[i].x =  (INT16)(polygon_fifo[ptr++]);
					vert[i].p[0] = (UINT16)(polygon_fifo[ptr++]);
				}

				if (vert[0].p[0] < 0x8000 && vert[1].p[0] < 0x8000 && vert[2].p[0] < 0x8000)
				{
					poly_render_triangle(state->poly, state->framebuffer, video_screen_get_visible_area(machine->primary_screen), render_texture_scan, 4, &vert[0], &vert[1], &vert[2]);
				}
				break;
			}
			case 0x04:		// Gouraud shaded Quad
			{
				// 0x00: Command ID (0x0004)
				// 0x01: Vertex 0 color
				// 0x02: Vertex 0 Y
				// 0x03: Vertex 0 X
				// 0x04: Vertex 0 Z
				// 0x05: Vertex 1 color
				// 0x06: Vertex 1 Y
				// 0x07: Vertex 1 X
				// 0x08: Vertex 1 Z
				// 0x09: Vertex 2 color
				// 0x0a: Vertex 2 Y
				// 0x0b: Vertex 2 X
				// 0x0c: Vertex 2 Z
				// 0x0d: Vertex 3 color
				// 0x0e: Vertex 3 Y
				// 0x0f: Vertex 3 X
				// 0x10: Vertex 3 Z

				/*
                printf("CMD4: ");
                for (i=0; i < 0x10; i++)
                {
                    printf("%04X ", polygon_fifo[ptr+i]);
                }
                printf("\n");
                */

				for (i=0; i < 4; i++)
				{
					vert[i].p[1] = polygon_fifo[ptr++];
					vert[i].y =  (INT16)(polygon_fifo[ptr++]);
					vert[i].x =  (INT16)(polygon_fifo[ptr++]);
					vert[i].p[0] = (UINT16)(polygon_fifo[ptr++]);
				}

				if (vert[0].p[0] < 0x8000 && vert[1].p[0] < 0x8000 && vert[2].p[0] < 0x8000 && vert[3].p[0] < 0x8000)
				{
					if (vert[0].p[1] == vert[1].p[1] &&
						vert[1].p[1] == vert[2].p[1] &&
						vert[2].p[1] == vert[3].p[1])
					{
						// optimization: all colours the same -> render solid
						poly_render_quad(state->poly, state->framebuffer, video_screen_get_visible_area(machine->primary_screen), render_solid_scan, 2, &vert[0], &vert[1], &vert[2], &vert[3]);
					}
					else
					{
						poly_render_quad(state->poly, state->framebuffer, video_screen_get_visible_area(machine->primary_screen), render_shade_scan, 2, &vert[0], &vert[1], &vert[2], &vert[3]);
					}
				}
				break;
			}
			case 0x06:		// Textured Quad
			{
				// 0x00: Command ID (0x0006)
				// 0x01: Texture base
				// 0x02: Vertex 1 Palette
				// 0x03: Vertex 1 V
				// 0x04: Vertex 1 U
				// 0x05: Vertex 1 Y
				// 0x06: Vertex 1 X
				// 0x07: Vertex 1 Z
				// 0x08: Vertex 2 Palette
				// 0x09: Vertex 2 V
				// 0x0a: Vertex 2 U
				// 0x0b: Vertex 2 Y
				// 0x0c: Vertex 2 X
				// 0x0d: Vertex 2 Z
				// 0x0e: Vertex 3 Palette
				// 0x0f: Vertex 3 V
				// 0x10: Vertex 3 U
				// 0x11: Vertex 3 Y
				// 0x12: Vertex 3 X
				// 0x13: Vertex 3 Z
				// 0x14: Vertex 4 Palette
				// 0x15: Vertex 4 V
				// 0x16: Vertex 4 U
				// 0x17: Vertex 4 Y
				// 0x18: Vertex 4 X
				// 0x19: Vertex 4 Z

				poly_extra_data *extra = (poly_extra_data *)poly_get_extra_data(state->poly);
				UINT16 texbase;

				/*
                printf("CMD6: ");
                for (i=0; i < 0x19; i++)
                {
                    printf("%04X ", polygon_fifo[ptr+i]);
                }
                printf("\n");
                */

				texbase = polygon_fifo[ptr++];

				extra->zbuffer = state->zbuffer;
				extra->texture = state->texture;
				extra->tex_base_x = ((texbase >> 0) & 0xff) << 4;
				extra->tex_base_y = ((texbase >> 8) & 0xff) << 4;

				extra->tex_wrap_x = (cmd & 0xc0) ? 1 : 0;
				extra->tex_wrap_y = (cmd & 0x30) ? 1 : 0;

				for (i=0; i < 4; i++)
				{
					vert[i].p[3] = polygon_fifo[ptr++];	// palette
					vert[i].p[2] = (UINT16)(polygon_fifo[ptr++]);
					vert[i].p[1] = (UINT16)(polygon_fifo[ptr++]);
					vert[i].y =  (INT16)(polygon_fifo[ptr++]);
					vert[i].x =  (INT16)(polygon_fifo[ptr++]);
					vert[i].p[0] = (UINT16)(polygon_fifo[ptr++]);
				}

				if (vert[0].p[0] < 0x8000 && vert[1].p[0] < 0x8000 && vert[2].p[0] < 0x8000 && vert[3].p[0] < 0x8000)
				{
					poly_render_quad(state->poly, state->framebuffer, video_screen_get_visible_area(machine->primary_screen), render_texture_scan, 4, &vert[0], &vert[1], &vert[2], &vert[3]);
				}
				break;
			}
			case 0x00:
			{
				ptr += 6;
				break;
			}
			default:
			{
				//printf("render_polygons: unknown command %04X\n", cmd);
			}
		}
	}

	poly_wait(state->poly, "Finished render");
}

void taitojc_clear_frame(running_machine *machine)
{
	taitojc_state *state = (taitojc_state *)machine->driver_data;
	rectangle cliprect;

	cliprect.min_x = 0;
	cliprect.min_y = 0;
	cliprect.max_x = video_screen_get_width(machine->primary_screen) - 1;
	cliprect.max_y = video_screen_get_height(machine->primary_screen) - 1;

	bitmap_fill(state->framebuffer, &cliprect, 0);
	bitmap_fill(state->zbuffer, &cliprect, 0xffff);
}
