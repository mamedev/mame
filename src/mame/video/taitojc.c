#include "emu.h"
#include "video/polynew.h"
#include "includes/taitojc.h"

static const gfx_layout taitojc_char_layout =
{
	16,16,
	0x80,
	4,
	{ 0,1,2,3 },
	{ 24,28,16,20,8,12,0,4, 56,60,48,52,40,44,32,36 },
	{ 0*64, 1*64, 2*64, 3*64, 4*64, 5*64, 6*64, 7*64,8*64,9*64,10*64,11*64,12*64,13*64,14*64,15*64 },
	16*64
};

static TILE_GET_INFO( taitojc_tile_info )
{
	taitojc_state *state = machine.driver_data<taitojc_state>();

	UINT32 val = state->m_tile_ram[tile_index];
	int color = (val >> 22) & 0xff;
	int tile = (val >> 2) & 0x7f;
	SET_TILE_INFO(state->m_gfx_index, tile, color, 0);
}

READ32_MEMBER(taitojc_state::taitojc_palette_r)
{
	return m_palette_ram[offset];
}

WRITE32_MEMBER(taitojc_state::taitojc_palette_w)
{
	int r, g, b;
	UINT32 color;

	COMBINE_DATA( m_palette_ram + offset );

	color = m_palette_ram[offset];
	r = (color >>  8) & 0xff;
	g = (color >> 16) & 0xff;
	b = (color >>  0) & 0xff;

	palette_set_color(machine(),offset, MAKE_RGB(r, g, b));
}

READ32_MEMBER(taitojc_state::taitojc_tile_r)
{
	return m_tile_ram[offset];
}

READ32_MEMBER(taitojc_state::taitojc_char_r)
{
	return m_char_ram[offset];
}

WRITE32_MEMBER(taitojc_state::taitojc_tile_w)
{
	COMBINE_DATA(m_tile_ram + offset);
	m_tilemap->mark_tile_dirty(offset);
}

WRITE32_MEMBER(taitojc_state::taitojc_char_w)
{
	COMBINE_DATA(m_char_ram + offset);
	machine().gfx[m_gfx_index]->mark_dirty(offset/32);
}

// Object data format:
//
// 0x00:   xxxxxx-- -------- -------- --------   Height
// 0x00:   ------xx xxxxxxxx -------- --------   Y
// 0x00:   -------- -------- xxxxxx-- --------   Width
// 0x00:   -------- -------- ------xx xxxxxxxx   X
// 0x01:   ---xxxxx xx------ -------- --------   Palette
// 0x01:   -------- --x----- -------- --------   Priority (0 = below 3D, 1 = above 3D)
// 0x01:   -------- -------x -------- --------   Color depth (0) 4bpp / (1) 8bpp
// 0x01:   -------- -------- -xxxxxxx xxxxxxxx   VRAM data address

/*
    Object RAM is grouped in three different banks (0-0x400 / 0x400-0x800 / 0x800-0xc00),
    Initial 6 dwords aren't surely for object stuff (setting global object flags?)
    0xd00-0xdff seems to be a per-bank vregister. Usage of this is mostly unknown, the only
    clue we have so far is this config change in dendego:
    0x2000db3f 0x3f3f3f3f 0xfec00090 0x403f00ff 0xd20-0xd2f on Taito logo
    0x2000db3f 0x3f3f3f3f 0xff600090 0x207f00ff 0xd20-0xd2f on intro FMV

    dword 0 bits 14-15 looks up to the object RAM for the given bank. (it's mostly fixed to 0,
    1 and 2 for each bank). Then dwords 2 and 3 should presumably configure bank 1 to a bigger
    (doubled?) height and width and a different x/y start point.


    0xfc0-0xfff is global vregs. 0xfc4 bit 13 is used to swap between bank 0 and bank 1?
    It's unknown at current time how bank 2 should show up.

        fc0 00000000   always
        fc4 c01f0000   boot-up, testmode, sidebs always, sidebs2 always
            c0100000   landgear in-game, dendego2 in-game
            c0310000   dendego in-game
            c0312000   dendego intro 3d parts
            c031f000   dendego disclaimer screen (only for a few frames)

        fc4 11000000 00------ ----0000 00000000   always 0/1
            -------- --xx---- -------- --------   ?
            -------- ----xxxx -------- --------   one of these probably disables textlayer, unknown function otherwise
            -------- -------- xxxx---- --------   object bank related

        fc8 40000000   always
        fcc 00000000   always
        fd0 c0000000   always
        ...
        ffc c0000000   always

*/

static void draw_object(running_machine &machine, bitmap_ind16 &bitmap, const rectangle &cliprect, UINT32 w1, UINT32 w2, UINT8 bank_type)
{
	taitojc_state *state = machine.driver_data<taitojc_state>();
	int x, y, width, height, palette;
	int i, j;
	int x1, x2, y1, y2;
	int ix, iy;
	UINT32 address;
	UINT8 *v;
	UINT8 color_depth;
	UINT8 mask_screen;

	color_depth = (w2 & 0x10000) >> 16;
	mask_screen = (w2 & 0x20000) >> 17;

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

	/* TODO: untangle this! */
	if(address >= 0xff000)
		v = (UINT8*)&state->m_objlist[(address-0xff000)/4];
	if(address >= 0xfc000)
		v = (UINT8*)&state->m_char_ram[(address-0xfc000)/4];
	else if(address >= 0xf8000)
		v = (UINT8*)&state->m_tile_ram[(address-0xf8000)/4];
	else
		v = (UINT8*)&state->m_vram[address/4];

	/* guess, but it's probably doable via a vreg ... */
	if ((width == 0 || height == 0) && bank_type == 2)
		width = height = 16;

	if(width == 0 || height == 0)
		return;

	x1 = x;
	x2 = x + width;
	y1 = y;
	y2 = y + height;

	// trivial rejection
	if (x1 > cliprect.max_x || x2 < cliprect.min_x || y1 > cliprect.max_y || y2 < cliprect.min_y)
	{
		return;
	}

//  mame_printf_debug("draw_object: %08X %08X, X: %d, Y: %d, W: %d, H: %d\n", w1, w2, x, y, width, height);

	ix = 0;
	iy = 0;

	// clip
	if (x1 < cliprect.min_x)
	{
		ix = abs(cliprect.min_x - x1);
		x1 = cliprect.min_x;
	}
	if (x2 > cliprect.max_x)
	{
		x2 = cliprect.max_x;
	}
	if (y1 < cliprect.min_y)
	{
		iy = abs(cliprect.min_y - y1);
		y1 = cliprect.min_y;
	}
	if (y2 > cliprect.max_y)
	{
		y2 = cliprect.max_y;
	}

	/* this bit seems to set up border at left/right of screen (reads at 0xffc00) */
	if(mask_screen)
	{
		if(address != 0xffc00)
		{
			popmessage("mask screen with %08x, contact MAMEdev",address);
			return;
		}

		for (j=y1; j < y2; j++)
		{
			UINT16 *d = &bitmap.pix16(j);

			for (i=x1; i < x2; i++)
			{
				d[i] = 0x78; //TODO: black

				//index++;
			}

			//iy++;
		}
	}
	else if(!color_depth) // Densha de Go 2/2X "credit text", 4bpp
	{
		for (j=y1; j < y2; j++)
		{
			UINT16 *d = &bitmap.pix16(j);
			int index = (iy * (width / 2)) + ix;

			for (i=x1; i < x2; i+=2)
			{
				UINT8 pen = (v[BYTE4_XOR_BE(index)] & 0xf0) >> 4;
				if (pen != 0)
					d[i] = palette + pen;

				pen = (v[BYTE4_XOR_BE(index)] & 0x0f);
				if (pen != 0)
					d[i+1] = palette + pen;

				index++;
			}

			iy++;
		}
	}
	else // 8bpp
	{
		{
			for (j=y1; j < y2; j++)
			{
				UINT16 *d = &bitmap.pix16(j);
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
	}
}

static void draw_object_bank(running_machine &machine, bitmap_ind16 &bitmap, const rectangle &cliprect, UINT8 bank_type, UINT8 pri)
{
	taitojc_state *state = machine.driver_data<taitojc_state>();
	UINT16 start_offs;
//  UINT8 double_xy;
	int i;

	start_offs = ((bank_type+1)*0x400)/4;
//  double_xy = (state->m_objlist[(0xd1c+bank_type*0x10)/4] & 0x20000000) >> 29;

	/* probably a core bug in there (otherwise objects sticks on screen in Densha de Go) */
	if(bank_type == 1 && (!(state->m_objlist[0xfc4/4] & 0x2000)))
		return;

	for (i=start_offs-2; i >= (start_offs-0x400/4); i-=2)
	{
		UINT32 w1 = state->m_objlist[i + 0];
		UINT32 w2 = state->m_objlist[i + 1];

		if (((w2 & 0x200000) >> 21) == pri)
		{
			draw_object(machine, bitmap, cliprect, w1, w2, bank_type);
		}
	}
}



static void taitojc_exit(running_machine &machine)
{
}

VIDEO_START( taitojc )
{
	taitojc_state *state = machine.driver_data<taitojc_state>();

	machine.add_notifier(MACHINE_NOTIFY_EXIT, machine_notify_delegate(FUNC(taitojc_exit), &machine));

	/* find first empty slot to decode gfx */
	for (state->m_gfx_index = 0; state->m_gfx_index < MAX_GFX_ELEMENTS; state->m_gfx_index++)
		if (machine.gfx[state->m_gfx_index] == 0)
			break;

	assert(state->m_gfx_index != MAX_GFX_ELEMENTS);

	state->m_tilemap = tilemap_create(machine, taitojc_tile_info, TILEMAP_SCAN_ROWS, 16, 16, 64, 64);

	state->m_tilemap->set_transparent_pen(0);

	state->m_char_ram = auto_alloc_array_clear(machine, UINT32, 0x4000/4);
	state->m_tile_ram = auto_alloc_array_clear(machine, UINT32, 0x4000/4);

	/* create the char set (gfx will then be updated dynamically from RAM) */
	machine.gfx[state->m_gfx_index] = auto_alloc(machine, gfx_element(machine, taitojc_char_layout, (UINT8 *)state->m_char_ram, machine.total_colors() / 16, 0));

	state->m_texture = auto_alloc_array(machine, UINT8, 0x400000);

	machine.primary_screen->register_screen_bitmap(state->m_framebuffer);
	machine.primary_screen->register_screen_bitmap(state->m_zbuffer);

	/* create renderer */
	state->m_renderer = auto_alloc(machine, taitojc_renderer(machine, &state->m_framebuffer, &state->m_zbuffer, state->m_texture));
}

SCREEN_UPDATE_IND16( taitojc )
{
	taitojc_state *state = screen.machine().driver_data<taitojc_state>();

	bitmap.fill(0, cliprect);

	// low priority objects
	draw_object_bank(screen.machine(), bitmap, cliprect, 0, 0);
	draw_object_bank(screen.machine(), bitmap, cliprect, 1, 0);
	draw_object_bank(screen.machine(), bitmap, cliprect, 2, 0);

	// 3D layer
	copybitmap_trans(bitmap, state->m_framebuffer, 0, 0, 0, 0, cliprect, 0);

	// high priority objects
	draw_object_bank(screen.machine(), bitmap, cliprect, 0, 1);
	draw_object_bank(screen.machine(), bitmap, cliprect, 1, 1);
	draw_object_bank(screen.machine(), bitmap, cliprect, 2, 1);

	// text layer
	if (state->m_objlist[0xfc4/4] & 0x10000)
		state->m_tilemap->draw(bitmap, cliprect, 0, 0);

	return 0;
}

SCREEN_UPDATE_IND16( dendego )
{
	taitojc_state *state = screen.machine().driver_data<taitojc_state>();

	// update controller state in artwork
	UINT8 btn = (state->ioport("BUTTONS")->read() & 0x77);
	int level;
	for (level = 5; level > 0; level--)
		if (btn == dendego_mascon_table[level]) break;

	if (level != output_get_value("counter0"))
		output_set_value("counter0", level);

	btn = (state->ioport("ANALOG1")->read() & 0xff);
	for (level = 10; level > 0; level--)
		if (btn >= dendego_brake_table[level]) break;

	if (level != output_get_value("counter1"))
		output_set_value("counter1", level);

	return SCREEN_UPDATE16_CALL(taitojc);
}



void taitojc_renderer::render_solid_scan(INT32 scanline, const extent_t &extent, const taitojc_polydata &extradata, int threadid)
{
	float z = extent.param[0].start;
	int color = extent.param[1].start;
	float dz = extent.param[0].dpdx;
	UINT16 *fb = &m_framebuffer->pix16(scanline);
	UINT16 *zb = &m_zbuffer->pix16(scanline);

	for (int x = extent.startx; x < extent.stopx; x++)
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

void taitojc_renderer::render_shade_scan(INT32 scanline, const extent_t &extent, const taitojc_polydata &extradata, int threadid)
{
	float z = extent.param[0].start;
	float color = extent.param[1].start;
	float dz = extent.param[0].dpdx;
	float dcolor = extent.param[1].dpdx;
	UINT16 *fb = &m_framebuffer->pix16(scanline);
	UINT16 *zb = &m_zbuffer->pix16(scanline);

	for (int x = extent.startx; x < extent.stopx; x++)
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

void taitojc_renderer::render_texture_scan(INT32 scanline, const extent_t &extent, const taitojc_polydata &extradata, int threadid)
{
	float z = extent.param[0].start;
	float u = extent.param[1].start;
	float v = extent.param[2].start;
	float color = extent.param[3].start;
	float dz = extent.param[0].dpdx;
	float du = extent.param[1].dpdx;
	float dv = extent.param[2].dpdx;
	float dcolor = extent.param[3].dpdx;
	UINT16 *fb = &m_framebuffer->pix16(scanline);
	UINT16 *zb = &m_zbuffer->pix16(scanline);
	int tex_wrap_x = extradata.tex_wrap_x;
	int tex_wrap_y = extradata.tex_wrap_y;
	int tex_base_x = extradata.tex_base_x;
	int tex_base_y = extradata.tex_base_y;

	for (int x = extent.startx; x < extent.stopx; x++)
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

		texel = m_texture[(iv * 2048) + iu];

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

void taitojc_renderer::render_polygons(running_machine &machine, UINT16 *polygon_fifo, int length)
{
//  taitojc_state *state = machine.driver_data<taitojc_state>();
	const rectangle visarea = machine.primary_screen->visible_area();
	vertex_t vert[4];
	int i;
	int ptr;

	ptr = 0;
	while (ptr < length)
	{
		UINT16 cmd = polygon_fifo[ptr++];

		switch (cmd & 0x7)
		{
			// screen global clipping for 3d(?)
			case 0x00:
			{
				UINT16 min_x,min_y,min_z,max_x,max_y,max_z;

				min_x = polygon_fifo[ptr+1];
				min_y = polygon_fifo[ptr+0];
				min_z = polygon_fifo[ptr+2];
				max_x = polygon_fifo[ptr+4];
				max_y = polygon_fifo[ptr+3];
				max_z = polygon_fifo[ptr+5];

				/* let's check if we need to implement this ... */
				if(min_x != 0 || min_y != 0 || min_z != 0 || max_x != 512 || max_y != 400 || max_z != 0x7fff)
				{
					printf("CMD %04x\n",cmd);
					printf("MIN Y %04x\n",polygon_fifo[ptr+0]);
					printf("MIN X %04x\n",polygon_fifo[ptr+1]);
					printf("MIN Z %04x\n",polygon_fifo[ptr+2]);
					printf("MAX Y %04x\n",polygon_fifo[ptr+3]);
					printf("MAX X %04x\n",polygon_fifo[ptr+4]);
					printf("MAX Z %04x\n",polygon_fifo[ptr+5]);
				}
				ptr += 6;
				break;
			}

			// Gouraud Shaded Triangle (Landing Gear)
			case 0x01:
			{
				// 0x00: Command ID (0x0001)
				// 0x01: Vertex 1 color
				// 0x02: Vertex 1 Y
				// 0x03: Vertex 1 X
				// 0x04: Vertex 1 Z
				// 0x05: Vertex 2 color
				// 0x06: Vertex 2 Y
				// 0x07: Vertex 2 X
				// 0x08: Vertex 2 Z
				// 0x09: Vertex 3 color
				// 0x0a: Vertex 3 Y
				// 0x0b: Vertex 3 X
				// 0x0c: Vertex 3 Z

#if 0
                printf("CMD1: ");
                for (i=0; i < 0x0c; i++)
                {
                    printf("%04X ", polygon_fifo[ptr+i]);
                }
                printf("\n");
#endif

				for (i=0; i < 3; i++)
				{
					vert[i].p[1] = polygon_fifo[ptr++];
					vert[i].y =  (INT16)(polygon_fifo[ptr++]);
					vert[i].x =  (INT16)(polygon_fifo[ptr++]);
					vert[i].p[0] = (UINT16)(polygon_fifo[ptr++]);
				}

				if (vert[0].p[0] < 0x8000 && vert[1].p[0] < 0x8000 && vert[2].p[0] < 0x8000)
				{
					if (vert[0].p[1] == vert[1].p[1] &&
						vert[1].p[1] == vert[2].p[1])
					{
						// optimization: all colours the same -> render solid
						render_triangle(visarea, render_delegate(FUNC(taitojc_renderer::render_solid_scan), this), 2, vert[0], vert[1], vert[2]);
					}
					else
					{
						render_triangle(visarea, render_delegate(FUNC(taitojc_renderer::render_shade_scan), this), 2, vert[0], vert[1], vert[2]);
					}
				}
				break;
			}

			// Textured Triangle
			case 0x03:
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

#if 0
                printf("CMD3: ");
                for (i=0; i < 0x13; i++)
                {
                    printf("%04X ", polygon_fifo[ptr+i]);
                }
                printf("\n");
#endif

				taitojc_polydata &extra = object_data_alloc();
				UINT16 texbase = polygon_fifo[ptr++];

				extra.tex_base_x = ((texbase >> 0) & 0xff) << 4;
				extra.tex_base_y = ((texbase >> 8) & 0xff) << 4;

				extra.tex_wrap_x = (cmd & 0xc0) ? 1 : 0;
				extra.tex_wrap_y = (cmd & 0x30) ? 1 : 0;

				for (i=0; i < 3; i++)
				{
					vert[i].p[3] = polygon_fifo[ptr++] + 0.5;	// palette
					vert[i].p[2] = (UINT16)(polygon_fifo[ptr++]);
					vert[i].p[1] = (UINT16)(polygon_fifo[ptr++]);
					vert[i].y =  (INT16)(polygon_fifo[ptr++]);
					vert[i].x =  (INT16)(polygon_fifo[ptr++]);
					vert[i].p[0] = (UINT16)(polygon_fifo[ptr++]);
				}

				if (vert[0].p[0] < 0x8000 && vert[1].p[0] < 0x8000 && vert[2].p[0] < 0x8000)
				{
					render_triangle(visarea, render_delegate(FUNC(taitojc_renderer::render_texture_scan), this), 4, vert[0], vert[1], vert[2]);
				}
				break;
			}

			// Gouraud shaded Quad
			case 0x04:
			{
				// 0x00: Command ID (0x0004)
				// 0x01: Vertex 1 color
				// 0x02: Vertex 1 Y
				// 0x03: Vertex 1 X
				// 0x04: Vertex 1 Z
				// 0x05: Vertex 2 color
				// 0x06: Vertex 2 Y
				// 0x07: Vertex 2 X
				// 0x08: Vertex 2 Z
				// 0x09: Vertex 3 color
				// 0x0a: Vertex 3 Y
				// 0x0b: Vertex 3 X
				// 0x0c: Vertex 3 Z
				// 0x0d: Vertex 4 color
				// 0x0e: Vertex 4 Y
				// 0x0f: Vertex 4 X
				// 0x10: Vertex 4 Z

#if 0
                printf("CMD4: ");
                for (i=0; i < 0x10; i++)
                {
                    printf("%04X ", polygon_fifo[ptr+i]);
                }
                printf("\n");
#endif

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
						render_polygon<4>(visarea, render_delegate(FUNC(taitojc_renderer::render_solid_scan), this), 2, vert);
					}
					else
					{
						render_polygon<4>(visarea, render_delegate(FUNC(taitojc_renderer::render_shade_scan), this), 2, vert);
					}
				}
				break;
			}

			// Textured Quad
			case 0x06:
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

#if 0
                printf("CMD6: ");
                for (i=0; i < 0x19; i++)
                {
                    printf("%04X ", polygon_fifo[ptr+i]);
                }
                printf("\n");
#endif

				taitojc_polydata &extra = object_data_alloc();
				UINT16 texbase = polygon_fifo[ptr++];

				extra.tex_base_x = ((texbase >> 0) & 0xff) << 4;
				extra.tex_base_y = ((texbase >> 8) & 0xff) << 4;

				extra.tex_wrap_x = (cmd & 0xc0) ? 1 : 0;
				extra.tex_wrap_y = (cmd & 0x30) ? 1 : 0;

				for (i=0; i < 4; i++)
				{
					vert[i].p[3] = polygon_fifo[ptr++] + 0.5;	// palette
					vert[i].p[2] = (UINT16)(polygon_fifo[ptr++]);
					vert[i].p[1] = (UINT16)(polygon_fifo[ptr++]);
					vert[i].y =  (INT16)(polygon_fifo[ptr++]);
					vert[i].x =  (INT16)(polygon_fifo[ptr++]);
					vert[i].p[0] = (UINT16)(polygon_fifo[ptr++]);
				}

				if (vert[0].p[0] < 0x8000 && vert[1].p[0] < 0x8000 && vert[2].p[0] < 0x8000 && vert[3].p[0] < 0x8000)
				{
					render_polygon<4>(visarea, render_delegate(FUNC(taitojc_renderer::render_texture_scan), this), 4, vert);
				}
				break;
			}

			default:
			{
				printf("render_polygons: unknown command %04X %d\n", cmd,ptr);
				break;
			}
		}
	}

	wait("Finished render");
}

void taitojc_clear_frame(running_machine &machine)
{
	taitojc_state *state = machine.driver_data<taitojc_state>();

	state->m_framebuffer.fill(0, machine.primary_screen->visible_area());
	state->m_zbuffer.fill(0xffff, machine.primary_screen->visible_area());
}
