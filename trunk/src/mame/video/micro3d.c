/***************************************************************************

    Microprose Games 3D hardware

    TODO:
    * Fix buffering issue
    * Find correct noise-fill LFSR arrangement

****************************************************************************/

#include "emu.h"
#include "cpu/am29000/am29000.h"
#include "includes/micro3d.h"


/*************************************
 *
 *  Defines
 *
 *************************************/

#define VTX_SEX(x)				((x) | ((x) & (1 << 29) ? 0xc0000000 : 0))

enum
{
	STATE_DRAW_CMD,
	STATE_DRAW_CMD_DATA,
	STATE_DRAW_VTX_DATA,
};


/*************************************
 *
 *  Video initialisation
 *
 *************************************/

VIDEO_START( micro3d )
{
	micro3d_state *state = machine.driver_data<micro3d_state>();

	/* Allocate 512x12 x 2 3D frame buffers */
	state->m_frame_buffers[0] = auto_alloc_array(machine, UINT16, 1024 * 512);
	state->m_frame_buffers[1] = auto_alloc_array(machine, UINT16, 1024 * 512);
	state->m_tmp_buffer = auto_alloc_array(machine, UINT16, 1024 * 512);
}


VIDEO_RESET( micro3d )
{
	micro3d_state *state = machine.driver_data<micro3d_state>();

	state->m_pipeline_state  = 0;
	state->m_creg = 0;

	state->m_drawing_buffer = 0;
	state->m_display_buffer = 1;
}


/*************************************
 *
 *  2D graphics
 *
 *************************************/

void micro3d_scanline_update(screen_device &screen, bitmap_ind16 &bitmap, int scanline, const tms34010_display_params *params)
{
	micro3d_state *state = screen.machine().driver_data<micro3d_state>();

	UINT16 *src = &state->m_micro3d_sprite_vram[(params->rowaddr << 8) & 0x7fe00];
	UINT16 *dest = &bitmap.pix16(scanline);
	int coladdr = params->coladdr;
	int sd_11_7 = (state->m_creg & 0x1f) << 7;
	int x;

	UINT16 *frame_src;

	scanline = MAX((scanline - params->veblnk), 0);
	frame_src = state->m_frame_buffers[state->m_display_buffer] + (scanline << 10);

	/* TODO: XFER3DK - X/Y offsets for 3D */

	/* Copy the non-blanked portions of this scanline */
	for (x = params->heblnk; x < params->hsblnk; x += 2)
	{
		UINT16 pix = src[coladdr++ & 0x1ff];

		/*
            TODO: The upper four bits of the 3D buffer affect priority
            0xfxx forces 3D priority?
        */
		if (pix & 0x80)
			dest[x + 0] = sd_11_7 | (pix & 0x7f);
		else
			dest[x + 0] = *frame_src & 0xfff;

		pix >>= 8;
		frame_src++;

		if (pix & 0x80)
			dest[x + 1] = sd_11_7 | (pix & 0x7f);
		else
			dest[x + 1] = *frame_src & 0xfff;

		frame_src++;
	}
}

WRITE16_MEMBER(micro3d_state::micro3d_clut_w)
{
	UINT16 word;

	COMBINE_DATA(&m_generic_paletteram_16[offset]);
	word = m_generic_paletteram_16[offset];
	palette_set_color_rgb(machine(), offset, pal5bit(word >> 6), pal5bit(word >> 1), pal5bit(word >> 11));
}

WRITE16_MEMBER(micro3d_state::micro3d_creg_w)
{

	if (~data & 0x80)
		cputag_set_input_line(machine(), "vgb", 0, CLEAR_LINE);

	m_creg = data;
}

WRITE16_MEMBER(micro3d_state::micro3d_xfer3dk_w)
{

	m_xfer3dk = data;
}

void micro3d_tms_interrupt(device_t *device, int state)
{
//  mc68901_int_gen(device->machine(), GPIP4);
}


/*************************************
 *
 *  3D graphics
 *
 *************************************/

enum planes
{
	CLIP_Z_MIN,
	CLIP_Z_MAX,
	CLIP_X_MIN,
	CLIP_X_MAX,
	CLIP_Y_MIN,
	CLIP_Y_MAX,
};

static int inside(micro3d_state *state, micro3d_vtx *v, enum planes plane)
{
	switch (plane)
	{
		case CLIP_Z_MIN: return v->z >= state->m_z_min;
		case CLIP_Z_MAX: return v->z <= state->m_z_max;
		case CLIP_X_MIN: return v->x >= state->m_x_min;
		case CLIP_X_MAX: return v->x <= state->m_x_max;
		case CLIP_Y_MIN: return v->y >= state->m_y_min;
		case CLIP_Y_MAX: return v->y <= state->m_y_max;
	}

	return 0;
}

/* Calculate where two points intersect */
static micro3d_vtx intersect(micro3d_state *state, micro3d_vtx *v1, micro3d_vtx *v2, enum planes plane)
{
	float m = 0.0;
	micro3d_vtx vo = { 0, 0, 0 };

	if (v1->x != v2->x)
		m = (float)(v1->y - v2->y) / (float)(v1->x - v2->x);

	switch (plane)
	{
		case CLIP_Z_MIN:
		{
			float mxz, myz;

			if (v1->z != v2->z)
			{
				mxz = (float)(v1->x - v2->x) / (float)(v1->z - v2->z);
				myz = (float)(v1->y - v2->y) / (float)(v1->z - v2->z);
			}
			else
			{
				mxz = 0.0;
				myz = 0.0;
			}

			vo.x = v2->x + (state->m_z_min - v2->z) * mxz;
			vo.y = v2->y + (state->m_z_min - v2->z) * myz;
			vo.z = state->m_z_min;
			break;
		}
		case CLIP_Z_MAX:
		{
			float mxz, myz;

			if (v1->z != v2->z)
			{
				mxz = (float)(v1->x - v2->x) / (float)(v1->z - v2->z);
				myz = (float)(v1->y - v2->y) / (float)(v1->z - v2->z);
			}
			else
			{
				mxz = 0.0;
				myz = 0.0;
			}

			vo.x = v2->x + (state->m_z_max - v2->z) * mxz;
			vo.y = v2->y + (state->m_z_max - v2->z) * myz;
			vo.z = state->m_z_max;
			break;
		}
		case CLIP_X_MIN:
		{
			vo.x = state->m_x_min;
			vo.y = v2->y + (state->m_x_min - v2->x) * m;
			vo.z = 0;
			break;
		}
		case CLIP_X_MAX:
		{
			vo.x = state->m_x_max;
			vo.y = v2->y + (state->m_x_max - v2->x) * m;
			vo.z = 0;
			break;
		}
		case CLIP_Y_MIN:
		{
			if (v1->x != v2->x)
				vo.x = v2->x + (state->m_y_min - v2->y) / m;
			else
				vo.x = v2->x;

			vo.y = state->m_y_min;
			vo.z = 0;
			break;
		}
		case CLIP_Y_MAX:
		{
			if (v1->x != v2->x)
				vo.x = v2->x + (state->m_y_max - v2->y) / m;
			else
				vo.x = v2->x;

			vo.y = state->m_y_max;
			vo.z = 0;
			break;
		}
	}
	return vo;
}

INLINE void write_span(micro3d_state *state, UINT32 y, UINT32 x)
{
	UINT32 *draw_dpram = state->m_draw_dpram;
	int addr = y << 1;

	if (draw_dpram[addr] == 0x3ff000)
	{
		draw_dpram[addr] = (x << 12) | x;
	}
	else
	{
		/* Check start */
		if (x < (state->m_draw_dpram[addr] & 0x3ff))
		{
			draw_dpram[addr] &= ~0x3ff;
			draw_dpram[addr] |= x;
		}
		/* Check end */
		if (x > (draw_dpram[addr] >> 12))
		{
			draw_dpram[addr] &= ~0x3ff000;
			draw_dpram[addr] |= (x << 12);
		}
	}
}

/* This is the same algorithm used in the 3D tests */
static void draw_line(micro3d_state *state, UINT32 x1, UINT32 y1, UINT32 x2, UINT32 y2)
{
	UINT32 tmp2;
	UINT32 acc;
	UINT32 y_inc;

	UINT32 dx;
	UINT32 dy;

	if (x2 < x1)
	{
		UINT32 tmp;

		tmp = x1;
		x1 = x2;
		x2 = tmp;

		tmp = y1;
		y1 = y2;
		y2 = tmp;
	}

	dx = x2 - x1;

	if (y2 >= y1)
		dy = y2 - y1;
	else
		dy = y1 - y2;

	write_span(state, y1, x1);

	if (dx == 0 && dy == 0)
		return;

	if (y1 <= y2)
		y_inc = 1;
	else
		y_inc = -1;

	if (dx > dy)
	{
		if (y2 != y1)
		{
			tmp2 = dy << 1;
			acc = tmp2 - dx;
			dy = (dy - dx) << 1;

			do
			{
				if (~acc & 0x80000000)
				{
					write_span(state, y1, x1);
					y1 += y_inc;
					x1++;
					acc += dy;
					write_span(state, y1, x1);
				}
				else
				{
					acc += tmp2;
					x1++;
				}
			} while (y2 != y1);
		}

		if (x2 != x1)
			write_span(state, y1, x2);

	}
	else
	{
		tmp2 = dx << 1;
		acc = tmp2 - dy;
		dy = (dx - dy) << 1;

		if (y1 != y2)
		{
			do
			{
				if (acc & 0x80000000)
				{
					acc += tmp2;
					write_span(state, y1, x1);
					y1 += y_inc;
					write_span(state, y1, x1);
				}
				else
				{
					write_span(state, y1, x1);
					x1++;
					y1 += y_inc;
					write_span(state, y1, x1);

					acc += dy;
				}
			} while (y1 != y2);
		}

		if (x2 != x1)
			write_span(state, y1, x2);
	}
}

static void rasterise_spans(micro3d_state *state, UINT32 min_y, UINT32 max_y, UINT32 attr)
{
	int y;
	int color = attr & 0xfff;

	if ((attr >> 24) == 0x85)
	{
		for (y = min_y; y <= max_y; ++y)
		{
			int x;
			int addr = y << 1;
			UINT16 *dest = &state->m_tmp_buffer[y * 1024];

			if (state->m_draw_dpram[addr] == 0x3ff000)
			{
				continue;
			}
			else
			{
				int start = state->m_draw_dpram[addr] & 0x3ff;
				int end = (state->m_draw_dpram[addr] >> 12) & 0x3ff;

				for (x = start; x <= end; ++x)
					dest[x] = color;
			}
		}
	}
	else
	{
		/*
            I don't know the LFSR arrangement inside the DRAW2 ASIC
            but here are some possible tap arrangements
        */
		static const UINT8 taps[8][4] =
		{
			{9, 8, 7, 2},
			{9, 8, 6, 5},
			{9, 8, 5, 4},
			{9, 8, 5, 1},
			{9, 8, 4, 2},
			{9, 7, 6, 4},
			{9, 7, 5, 2},
			{9, 6, 5, 3},
		};

		int noise_val = (attr >> 12) & 0x3ff;
		int noise_taps = 0;

		for (y = min_y; y <= max_y; ++y)
		{
			int x;
			int addr = y << 1;
			UINT16 *dest = &state->m_tmp_buffer[y * 1024];

			if (state->m_draw_dpram[addr] == 0x3ff000)
			{
				continue;
			}
			else
			{
				int start = state->m_draw_dpram[addr] & 0x3ff;
				int end = (state->m_draw_dpram[addr] >> 12) & 0x3ff;

				for (x = start; x <= end; ++x)
				{
					int fb;

					if (noise_val & 0x1)
						dest[x] = color;

					fb = (BIT(noise_val, taps[noise_taps][0] - 1) ^ BIT(noise_val, taps[noise_taps][1] - 1) ^ BIT(noise_val, taps[noise_taps][2] - 1) ^ BIT(noise_val, taps[noise_taps][3] - 1));
					noise_val = ((noise_val << 1) | fb) & 0x1ff;
				}
			}
		}
	}
}

static int clip_triangle(micro3d_state *state, micro3d_vtx *v, micro3d_vtx *vout, int num_vertices, enum planes plane)
{
	micro3d_vtx clip_out[10];

	int i;
	int prev_i = num_vertices - 1;
	int clip_verts = 0;

	for (i = 0; i < num_vertices; ++i)
	{
		int v1_in = inside(state, &v[i], plane);
		int v2_in = inside(state, &v[prev_i], plane);

		/* Edge is inside */
		if (v1_in && v2_in)
		{
			clip_out[clip_verts++] = v[i];
		}
		/* Edge is leaving */
		else if (v1_in && !v2_in)
		{
			clip_out[clip_verts++] = intersect(state, &v[i], &v[prev_i], plane);
			clip_out[clip_verts++] = v[i];
		}
		/* Edge is entering */
		else if (!v1_in && v2_in)
		{
			clip_out[clip_verts++] = intersect(state, &v[i], &v[prev_i], plane);
		}

		prev_i = i;
	}

	memcpy(&vout[0], &clip_out[0], sizeof(vout[0]) * clip_verts);
	return clip_verts;
}

static void draw_triangles(micro3d_state *state, UINT32 attr)
{
	int i;
	int triangles = 0;
	int vertices = state->m_fifo_idx / 3;
	int min_y = 0x3ff;
	int max_y = 0;

	/* This satisifes the burst write test */
	if (vertices == 0)
	{
		int y;
		int val = ((state->m_x_mid + 16) << 12) | state->m_x_mid;

		for (y = state->m_y_mid; y <= state->m_y_mid + 16; ++y)
			state->m_draw_dpram[y << 1] = val;

		return;
	}

	/* Draw triangles as fans */
	for (i = 2; i < vertices; ++i)
	{
		int k;
		int clip_vertices = 3;

		micro3d_vtx vo, vm, vn;
		micro3d_vtx vclip_list[10];

		vo.x = state->m_vtx_fifo[0];
		vo.y = state->m_vtx_fifo[1];
		vo.z = state->m_vtx_fifo[2];

		vm.x = state->m_vtx_fifo[(i - 1) * 3 + 0];
		vm.y = state->m_vtx_fifo[(i - 1) * 3 + 1];
		vm.z = state->m_vtx_fifo[(i - 1) * 3 + 2];

		vn.x = state->m_vtx_fifo[i * 3 + 0];
		vn.y = state->m_vtx_fifo[i * 3 + 1];
		vn.z = state->m_vtx_fifo[i * 3 + 2];

		vclip_list[0] = vo;
		vclip_list[1] = vm;
		vclip_list[2] = vn;

		/* Clip against near Z and far Z planes */
		clip_vertices = clip_triangle(state, vclip_list, vclip_list, clip_vertices, CLIP_Z_MIN);
		clip_vertices = clip_triangle(state, vclip_list, vclip_list, clip_vertices, CLIP_Z_MAX);

		/* Perform perspective divide */
		for (k = 0; k < clip_vertices; ++k)
		{
			vclip_list[k].x = vclip_list[k].x * state->m_z_min / vclip_list[k].z;
			vclip_list[k].y = vclip_list[k].y * state->m_z_min / vclip_list[k].z;
			vclip_list[k].z = 0;
		}

		/* Perform screen-space clipping */
		clip_vertices = clip_triangle(state, vclip_list, vclip_list, clip_vertices, CLIP_Y_MAX);
		clip_vertices = clip_triangle(state, vclip_list, vclip_list, clip_vertices, CLIP_X_MIN);
		clip_vertices = clip_triangle(state, vclip_list, vclip_list, clip_vertices, CLIP_X_MAX);
		clip_vertices = clip_triangle(state, vclip_list, vclip_list, clip_vertices, CLIP_Y_MIN);

		/* Rasterise */
		if (clip_vertices >= 3)
		{
			micro3d_vtx a = vclip_list[0];
			micro3d_vtx b = vclip_list[1];

			triangles = TRUE;

			a.x += state->m_x_mid;
			a.y += state->m_y_mid;

			b.x += state->m_x_mid;
			b.y += state->m_y_mid;

			/* Keep track of the y-extents so we don't have to scan every line later */
			if (a.y < min_y)
				min_y = a.y;
			if (a.y > max_y)
				max_y = a.y;

			if (b.y < min_y)
				min_y = b.y;
			if (b.y > max_y)
				max_y = b.y;

			/* Draw the first line of the triangle/fan */
			draw_line(state, a.x, a.y, b.x, b.y);

			for (k = 2; k < clip_vertices; ++k)
			{
				micro3d_vtx c = vclip_list[k];

				c.x += state->m_x_mid;
				c.y += state->m_y_mid;

				if (c.y < min_y)
					min_y = c.y;
				if (c.y > max_y)
					max_y = c.y;

				draw_line(state, b.x, b.y, c.x, c.y);
				draw_line(state, a.x, a.y, c.x, c.y);
				b = c;
			}
		}
	}

	if (triangles == TRUE)
		rasterise_spans(state, min_y, max_y, attr);
}


/******************************************************************************

MPGDRAW commands

80000000     Start triangle list [...]
85000xxx     End of vertex list - draw (xxx = colour) [0]
8Axxxxxx     End of vertex list - random fill shading (xxx = colour) [0]

90000000     Set clipping Z min [1]
94000000     Set clipping Z max [1]
98000000     Set clipping Y max [1]
9c000000     Set clipping X min [1]
a0000000     Set clipping X max [1]
a4000001     Set clipping Y min [1] (what does 1 mean?)

d8000000     End of frame (will swap render buffer)    [0]
f8000000     Toggle health LED [0]

b8000000-1fe Draw ASIC DPRAM address
b80003ff     Data

bc000000-1fc DPRAM address for read access

******************************************************************************/

WRITE32_MEMBER(micro3d_state::micro3d_fifo_w)
{
	UINT32 opcode = data >> 24;

	switch (m_draw_state)
	{
		case STATE_DRAW_CMD:
		{
			m_draw_cmd = data;

			switch (opcode)
			{
				case 0xb4:
				{
					m_x_mid = data & 0x3ff;
					m_y_mid = (data >> 10) & 0x3ff;
					break;
				}
				case 0xc8:
				{
					m_dpram_bank ^= 1;
					break;
				}
				case 0xbc:
				{
					UINT32 dpram_r_addr = (((data & 0x01ff) << 1) | m_dpram_bank);
					m_pipe_data = m_draw_dpram[dpram_r_addr];
					cputag_set_input_line(machine(), "drmath", AM29000_INTR1, ASSERT_LINE);
					break;
				}
				case 0x80:
				{
					int addr;
					m_fifo_idx = 0;
					m_draw_state = STATE_DRAW_VTX_DATA;

					/* Invalidate the draw RAM
                     * TODO: Not sure this is the right place for it -
                     * causes monitor mode draw tests to fail
                     */
					for (addr = 0; addr < 512; ++addr)
						m_draw_dpram[addr << 1] = 0x3ff000;

					break;
				}
				case 0xf8:
				{
					/* 3D pipeline health LEDs toggle */
					break;
				}
				case 0xd8:
				{
					/* TODO: We shouldn't need this extra buffer - is there some sort of sync missing? */
					memcpy(m_frame_buffers[m_drawing_buffer], m_tmp_buffer, 512*1024*2);
					m_drawing_buffer ^= 1;
					cputag_set_input_line(machine(), "vgb", 0, ASSERT_LINE);
					break;
				}
				default:
					m_draw_state = STATE_DRAW_CMD_DATA;
			}
			break;
		}
		case STATE_DRAW_CMD_DATA:
		{
			switch (m_draw_cmd >> 24)
			{
				case 0x90: m_z_min = VTX_SEX(data); break;
				case 0x94: m_z_max = VTX_SEX(data); break;
				case 0x98: m_y_max = VTX_SEX(data); break;
				case 0x9c: m_x_min = VTX_SEX(data); break;
				case 0xa0: m_x_max = VTX_SEX(data); break;
				case 0xa4: m_y_min = VTX_SEX(data); break;
				case 0xb8:
				{
					m_draw_dpram[((m_draw_cmd & 0x1ff) << 1) | m_dpram_bank] = data & 0x00ffffff;
					break;
				}
				default:
					popmessage("Unknown 3D command: %x %x\n", m_draw_cmd, data);
			}
			m_draw_state = STATE_DRAW_CMD;
			break;
		}
		case STATE_DRAW_VTX_DATA:
		{
			if ((opcode == 0x85) || (opcode == 0x8a))
			{
				micro3d_state *state = machine().driver_data<micro3d_state>();
				draw_triangles(state, data);
				m_draw_state = STATE_DRAW_CMD;
			}
			else
			{
				m_vtx_fifo[m_fifo_idx++] = VTX_SEX(data);
			}
			break;
		}
	}
}

WRITE32_MEMBER(micro3d_state::micro3d_alt_fifo_w)
{

	m_vtx_fifo[m_fifo_idx++] = VTX_SEX(data);
}

READ32_MEMBER(micro3d_state::micro3d_pipe_r)
{

	cputag_set_input_line(machine(), "drmath", AM29000_INTR1, CLEAR_LINE);
	return m_pipe_data;
}

INTERRUPT_GEN( micro3d_vblank )
{
//  mc68901_int_gen(device->machine(), GPIP7);
	micro3d_state *state = device->machine().driver_data<micro3d_state>();

	state->m_display_buffer = state->m_drawing_buffer ^ 1;
}
