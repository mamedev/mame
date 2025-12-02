// license:BSD-3-Clause
// copyright-holders:R. Belmont, Olivier Galibert, ElSemi, Angelo Salese, Matthew Daniels
/*********************************************************************************************************************************

    Sega Model 2 Geometry Engine and 3D Rasterizer Emulation

    General Notes:

    - The 3D rendering system has 3 main parts:
        1) The Geometry Engine: Model 2B and 2C hardware upload the geometry code to a DSP. The original and 2A hardware
        have the geometry code in an internal ROM inside the DSP itself. The simulation written here for the geometry engine
        is based off the disassembly of the code uploaded to a 2B game.
        2) The Z-Sort and Clip Hardware: Not much information about this hardware. There are a couple of variables that
        can be passed to this stage, like a master Z-Clip enable register, and a Z-Sort mode variable.
        3) The Hardware Renderer: Not much information about the inner workings of this hardware. Based on the available
        tidbits from the 2B manual, it is a double-buffered device (can be toggled between page flipping and single
        buffer mode), and can also be switched to generate output at 60Hz or 30Hz.

    - Most of the information used to write this code come from 3 main sources:
        1) The Geometry Engine code disassembly from a 2B game.
        2) The Model 2B-CRX Manual.
        3) ElSemi's Direct3D implementation of the geometrizer engine.

    - The emulation strategy used here is to decouple the geometrizer code from the Z-Sort/Clip/Renderer code, so that for
    original and 2A games, the HLE version of the geometrizer can be used, while for 2B and 2C games, the original geometrizer
    DSP can be emulated, and data can be pushed to the Z-Sort/Clip/Renderer code.



    Geometry Engine Notes and Known Bugs:

    - Top Skater seem to use a slightly different geometry code that has a secondary transformation matrix. Once we implement
    the real DSP code upload and emulation for 2B and 2C games, this should not be an issue.



    Z-Sort Notes and Known Bugs:

    - The Z-Sort algorithm is not entirely understood. The manual states that the Z-Sort works internally in 1.4.8 bit format,
    but then it states that the Z-Sort Mode register value is added to the floating point z value, and converted to a 16 bit
    format (most likely 4.12).
    - The system allows for Z-Sort override, by means of specifying whether a polygon will use the same Z value ordinal as the
    previous polygon, or the calculated minimum or maximum from it's points. This allows for a full object to be in front of
    another, even though the first object might have some z coordinates that are bigger than the second object's z coordinates.
    - Polygons from later windows will always be rendered on top of polygons from earlier windows, regardless of z value. This
    can be seen in the name entry screen in Sega Rally for example, where letters meant to be behind the car are rendered in an
    earlier window and letters in front of the car are rendered in a later window.
    - The current implementation takes the effective computed z value for the polygon and converts it into a 4.12 fixed point
    representation, used as an index into an array of linked polygons. Every polygon with the same z value is linked with any
    previous polygon that had that same z value:

        z-value index   linked list of polygons
            0000        triangle3->triangle2->triangle1
            0001        triangle22->triangle8->triangle7
            ....
            FFFF        triangle33->triangle11->triangle9

    As we add polygons to the array, we also keep track of the minimum and maximum z-value indexes seen this frame.
    When it's time to render, we start and the max z-value seen, and iterate through the array going down to the minimum
    z-value seen, clipping and rendering the linked polygons as we go.



    Clip Notes and Known Bugs:

    - Four clipping planes representing the viewing frustum are used to clip polygons prior to projection. In the rare event
    that a polygon intersects with the origin exactly, it will produce a clipped vertex at (0.0, 0.0, 0,0) which would result
    in NaNs being generated during projection; a tiny value is added to z to prevent this.
    - A small offset need to be added horizontally and vertically to the viewport and center variables for certain games (like
    the original Model 2 games). The coordinate system has been worked out from the 2B specifications, but the older games
    need a slight adjustment.



    Hardware Renderer Notes and Known Bugs:

    - Texturing code could use a real good speed optimization.
    - The U and V coordinates provided by the game are in 13.3 fixed point format.
    - The log tables and distance coefficients are used to calculate the number of texels per world unit that need to
    be used to render a texture. Textures can also be provided with smaller levels of details and a LOD bit selector
    in the texture header tells the rasterizer which texture map to use. The rasterizer then can average two texture
    maps to do mip mapping. More information can be found on the 2B manual, on the 'Texturing' and 'Data Format' chapters.
    - The rasterizer supports 128x128 'microtextures' which are typically used to add more details to a texture when it is
    close enough to the viewer.
    - Polygons are rendered from front-to-back. A fill buffer is used to track which pixels in the framebuffer have already
    been drawn to and prevents them from being overwritten. The real hardware does this to ensure that the polygons closest
    to the camera are rendered first, in case the renderer runs out of time and has to skip ahead to the next frame.

*********************************************************************************************************************************/

#include "emu.h"
#include "model2.h"
#include "model2rd.ipp"

#include <cmath>
#include <limits>

#define pz      p[0]
#define pu      p[1]
#define pv      p[2]



/*******************************************
 *
 *  Generic 3D Math Functions
 *
 *******************************************/

static inline void transform_point(poly_vertex *point, float *matrix)
{
	float tx = (point->x * matrix[0]) + (point->y * matrix[3]) + (point->pz * matrix[6]) + (matrix[9]);
	float ty = (point->x * matrix[1]) + (point->y * matrix[4]) + (point->pz * matrix[7]) + (matrix[10]);
	float tz = (point->x * matrix[2]) + (point->y * matrix[5]) + (point->pz * matrix[8]) + (matrix[11]);

	point->x = tx;
	point->y = ty;
	point->pz = tz;
}

static inline void transform_vector(poly_vertex *vector, float *matrix)
{
	float tx = (vector->x * matrix[0]) + (vector->y * matrix[3]) + (vector->pz * matrix[6]);
	float ty = (vector->x * matrix[1]) + (vector->y * matrix[4]) + (vector->pz * matrix[7]);
	float tz = (vector->x * matrix[2]) + (vector->y * matrix[5]) + (vector->pz * matrix[8]);

	vector->x = tx;
	vector->y = ty;
	vector->pz = tz;
}

static inline void normalize_vector(poly_vertex *vector)
{
	const float n = sqrt((vector->x * vector->x) + (vector->y * vector->y) + (vector->pz * vector->pz));

	if (n)
	{
		float oon = 1.0f / n;
		vector->x *= oon;
		vector->y *= oon;
		vector->pz *= oon;
	}
}

static inline float dot_product(const poly_vertex &v1, const poly_vertex &v2)
{
	return (v1.x * v2.x) + (v1.y * v2.y) + (v1.pz * v2.pz);
}

static inline void vector_cross3(poly_vertex *dst, const poly_vertex *v0, const poly_vertex *v1, const poly_vertex *v2)
{
	poly_vertex p1, p2;

	p1.x = v1->x - v0->x;   p1.y = v1->y - v0->y;   p1.pz = v1->pz - v0->pz;
	p2.x = v2->x - v0->x;   p2.y = v2->y - v0->y;   p2.pz = v2->pz - v0->pz;

	dst->x = (p1.y * p2.pz) - (p1.pz * p2.y);
	dst->y = (p1.pz * p2.x) - (p1.x * p2.pz);
	dst->pz = (p1.x * p2.y) - (p1.y * p2.x);
}

static inline void apply_focus(model2_state::geo_state *geo, poly_vertex *p0)
{
	p0->x *= geo->focus.x;
	p0->y *= geo->focus.y;
}

/* 1.8.23 float to 4.12 float converter, courtesy of Aaron Giles */
inline u16 model2_state::float_to_zval(float floatval, s32 z_adjust)
{
	s32 fpint = f2u(floatval);
	s32 exponent = ((fpint >> 23) & 0xff) - ((z_adjust >> 23) & 0xff);
	u32 mantissa = fpint & 0x7fffff;

	/* round the low bits and reduce to 12 */
	mantissa += 0x400;
	if (mantissa > 0x7fffff)
	{
		exponent++;
		mantissa = (mantissa & 0x7fffff) >> 1;
	}
	mantissa >>= 11;

	// if negative, clamp to 0
	if (fpint < 0)
		return 0x0000;

	// the rest depends on the exponent
	if (exponent < -12)
		return 0x0000; // less than -12 is too small, return 0
	else if (exponent < 0)
		return (mantissa | 0x1000) >> -exponent; // between -12 and 0 create a denormal with exponent of 0
	else if (exponent < 15)
		return ((exponent + 1) << 12) | mantissa; // between 0 and 14 create a FP value with exponent + 1
	else
		return 0xffff; // above 14 is too large
}

static int32_t clip_polygon(poly_vertex *v, int32_t num_vertices, poly_vertex *vout, model2_state::plane clip_plane)
{
	int32_t outcount = 0;

	const poly_vertex *cur = v;
	poly_vertex *out = vout;

	float curdot = dot_product(*cur, clip_plane.normal);
	int32_t curin = (curdot >= clip_plane.distance) ? 1 : 0;

	for (int32_t i = 0; i < num_vertices; i++)
	{
		const int32_t nextvert = (i + 1) % num_vertices;

		/* if the current point is inside the plane, add it */
		if (curin)
			out[outcount++] = *cur;

		const float nextdot = dot_product(v[nextvert], clip_plane.normal);
		const int32_t nextin = (nextdot >= clip_plane.distance) ? 1 : 0;

		/* Add a clipped vertex if one end of the current edge is inside the plane and the other is outside */
		if ((curin != nextin) && !std::isnan(curdot) && !std::isnan(nextdot))
		{
			const float scale = (clip_plane.distance - curdot) / (nextdot - curdot);

			out[outcount].x = cur->x + ((v[nextvert].x - cur->x) * scale);
			out[outcount].y = cur->y + ((v[nextvert].y - cur->y) * scale);
			out[outcount].pz = cur->pz + ((v[nextvert].pz - cur->pz) * scale);
			out[outcount].pu = cur->pu + ((v[nextvert].pu - cur->pu) * scale);
			out[outcount].pv = cur->pv + ((v[nextvert].pv - cur->pv) * scale);
			outcount++;
		}

		curdot = nextdot;
		curin = nextin;
		cur++;
	}

	return outcount;
}

inline bool model2_state::check_culling(raster_state *raster, u32 attr, float min_z, float max_z)
{
	/* if doubleside is disabled */
	if (((attr >> 17) & 1) == 0)
	{
		/* if it's the backface, cull it */
		if (raster->command_buffer[9] & 0x00800000)
			return true;
	}

	/* if the linktype is 0, then we can also cull it */
	if (((attr >> 8) & 3) == 0)
		return true;

	/* if the minimum z value is bigger than the master z clip value, don't render */
	if (raster->master_z_clip != 0xff && (int32_t)(1.0 / min_z) > raster->master_z_clip)
		return true;

	/* if the maximum z value is < 0 then we can safely clip the entire polygon */
	if (max_z < 0)
		return true;

	return false;
}

/***********************************************************************************************/

/*******************************************
 *
 *  Hardware 3D Rasterizer Initialization
 *
 *******************************************/

void model2_state::raster_init(memory_region *texture_rom)
{
	m_raster = std::make_unique<raster_state>();

	m_raster->texture_rom = (u16 *)texture_rom->base();
	m_raster->texture_rom_mask = (texture_rom->bytes() / 2) - 1;

	save_item(NAME(m_raster->min_z));
	save_item(NAME(m_raster->max_z));
//  save_item(NAME(m_raster->tri_list));
	save_item(NAME(m_raster->tri_list_index));
	save_item(NAME(m_raster->command_buffer));
	save_item(NAME(m_raster->command_index));
	save_item(NAME(m_raster->cur_command));
	save_item(NAME(m_raster->master_z_clip));
	save_item(NAME(m_raster->triangle_z));
	save_item(NAME(m_raster->z_adjust));
	save_item(NAME(m_raster->reverse));
	save_item(NAME(m_raster->viewport));
	save_item(NAME(m_raster->center));
	save_item(NAME(m_raster->center_sel));
	save_item(NAME(m_raster->texture_ram));
	save_item(NAME(m_raster->log_ram));
}

/*******************************************
 *
 *  Hardware 3D Rasterizer Z-Clip selection
 *
 *******************************************/

void model2_state::model2_3d_zclip_w(u32 data)
{
	// setting this register to 0xff disables z-clip
	m_raster->master_z_clip = data;
}

// TODO: only Sky Target seems to use this for unknown purpose
u32 model2_state::polygon_count_r()
{
//  printf("%08x\n",m_raster->tri_list_index);

	return m_raster->tri_list_index;
}

/*******************************************
 *
 *  Hardware 3D Rasterizer Processing
 *
 *******************************************/

template <unsigned NumVerts>
void model2_state::model2_3d_process_polygon(raster_state *raster, u32 attr)
{
	quad_m2 object;
	u16 *th, *tp;
	int32_t tho;
	u32 i;
	bool cull;
	float zvalue;
	float min_z, max_z;

	static_assert(NumVerts == 3 || NumVerts == 4, "Polygon must have 3 or 4 vertices");

	/* extract P0(n-1) */
	object.v[1].x = u2f(raster->command_buffer[2] << 8);
	object.v[1].y = u2f(raster->command_buffer[3] << 8);
	object.v[1].pz = u2f(raster->command_buffer[4] << 8);

	/* extract P1(n-1) */
	object.v[0].x = u2f(raster->command_buffer[5] << 8);
	object.v[0].y = u2f(raster->command_buffer[6] << 8);
	object.v[0].pz = u2f(raster->command_buffer[7] << 8);

	/* extract P0(n) */
	object.v[2].x = u2f(raster->command_buffer[11] << 8);
	object.v[2].y = u2f(raster->command_buffer[12] << 8);
	object.v[2].pz = u2f(raster->command_buffer[13] << 8);

	if (NumVerts == 4)
	{
		/* extract P1(n) */
		object.v[3].x = u2f(raster->command_buffer[14] << 8);
		object.v[3].y = u2f(raster->command_buffer[15] << 8);
		object.v[3].pz = u2f(raster->command_buffer[16] << 8);
	}
	else
	{
		/* for triangles, the rope of P1(n) is achieved by P0(n-1) (linktype 3) */
		raster->command_buffer[14] = raster->command_buffer[11];
		raster->command_buffer[15] = raster->command_buffer[12];
		raster->command_buffer[16] = raster->command_buffer[13];
	}

	/* always calculate the min z and max z value */
	min_z = object.v[0].pz;
	if (object.v[1].pz < min_z) min_z = object.v[1].pz;
	if (object.v[2].pz < min_z) min_z = object.v[2].pz;
	if (NumVerts == 4 && object.v[3].pz < min_z) min_z = object.v[3].pz;

	max_z = object.v[0].pz;
	if (object.v[1].pz > max_z) max_z = object.v[1].pz;
	if (object.v[2].pz > max_z) max_z = object.v[2].pz;
	if (NumVerts == 4 && object.v[3].pz > max_z) max_z = object.v[3].pz;

	/* read in the texture information */

	/* texture point data */
	if (raster->command_buffer[0] & 0x800000)
		tp = &raster->texture_ram[raster->command_buffer[0] & 0xffff];
	else
		tp = &raster->texture_rom[raster->command_buffer[0] & raster->texture_rom_mask];

	object.v[0].pv = *tp++;
	object.v[0].pu = *tp++;
	object.v[1].pv = *tp++;
	object.v[1].pu = *tp++;
	object.v[2].pv = *tp++;
	object.v[2].pu = *tp++;
	if (NumVerts == 4)
	{
		object.v[3].pv = *tp++;
		object.v[3].pu = *tp++;
	}

	/* update the address */
	raster->command_buffer[0] += NumVerts * 2;

	/* texture header data */
	if (raster->command_buffer[1] & 0x800000)
		th = &raster->texture_ram[raster->command_buffer[1] & 0xffff];
	else
		th = &raster->texture_rom[raster->command_buffer[1] & raster->texture_rom_mask];

	object.texheader[0] = *th++;
	object.texheader[1] = *th++;
	object.texheader[2] = *th++;
	object.texheader[3] = *th++;

	/* extract the texture header offset */
	tho = (attr >> 12) & 0x1f;

	/* adjust for sign */
	if (tho & 0x10)
		tho |= -16;

	/* update the address */
	raster->command_buffer[1] += tho * 4;

	/* set the luma value of this quad */
	object.luma = (raster->command_buffer[9] >> 15) & 0xff;

	/* set the texture LOD of this quad */
	object.texlod = ((raster->command_buffer[10] >> 8) & 0x7f80) - 0x3f80;
	object.texlod += raster->log_ram[raster->command_buffer[10] & 0x7fff];

	/* determine whether we can cull this quad */
	cull = check_culling(raster,attr,min_z,max_z);

	/* set the object's z value */
	switch ((attr >> 10) & 3)
	{
		case 0: // old value
			zvalue = raster->triangle_z;
			break;
		case 1: // min z
			zvalue = min_z;
			break;
		case 2: // max z
			zvalue = max_z;
			break;
		case 3: // error
		default:
			zvalue = 1e10;
			break;
	}

	raster->triangle_z = zvalue;

	if (cull == false)
	{
		int32_t clipped_verts;
		poly_vertex verts_in[10], verts_out[10];

		for (int i = 0; i < NumVerts; i++)
			verts_in[i] = object.v[i];

		clipped_verts = NumVerts;

		/* do clipping */
		for (int i = 0; i < 4; i++)
		{
			clipped_verts = clip_polygon(verts_in, clipped_verts, verts_out, raster->clip_plane[raster->center_sel][i]);
			for (int j = 0; j < clipped_verts; j++)
				verts_in[j] = verts_out[j];
		}

		if (clipped_verts > 2)
		{
			triangle *ztri;

			/* adjust and set the object z-sort value */
			object.z = float_to_zval(zvalue, raster->z_adjust);

			/* get our list read to add the triangles */
			ztri = raster->tri_sorted_list[object.z];

			/* go through the clipped vertex list, adding triangles */
			for (i = 2; i < clipped_verts; i++)
			{
				triangle    *tri;

				tri = &raster->tri_list[raster->tri_list_index++];

				if (raster->tri_list_index >= MAX_TRIANGLES)
				{
					fatalerror("SEGA 3D: Max triangle limit exceeded\n");
				}

				/* copy the object information */
				tri->z = object.z;
				tri->texheader[0] = object.texheader[0];
				tri->texheader[1] = object.texheader[1];
				tri->texheader[2] = object.texheader[2];
				tri->texheader[3] = object.texheader[3];
				tri->luma = object.luma;
				tri->texlod = object.texlod;

				/* set the viewport */
				tri->viewport[0] = raster->viewport[0];
				tri->viewport[1] = raster->viewport[1];
				tri->viewport[2] = raster->viewport[2];
				tri->viewport[3] = raster->viewport[3];

				/* set the center */
				tri->center[0] = raster->center[raster->center_sel][0];
				tri->center[1] = raster->center[raster->center_sel][1];

				/* set the window */
				tri->window = raster->cur_window;

				memcpy(&tri->v[0], &verts_out[0], sizeof(poly_vertex));
				memcpy(&tri->v[1], &verts_out[i-1], sizeof(poly_vertex));
				memcpy(&tri->v[2], &verts_out[i], sizeof(poly_vertex));

				/* add to our sorted list */
				raster->tri_sorted_list[object.z] = tri;
				tri->next = ztri;
				ztri = tri;
			}

			/* keep around the min and max z values for this frame */
			if (object.z < raster->min_z) raster->min_z = object.z;
			if (object.z > raster->max_z) raster->max_z = object.z;
		}
	}

	/* update linking */
	switch (((attr >> 8) & 3))
	{
		case 0:
		case 2:
		{
			/* reuse P0(n) and P1(n) */
			for (i = 0; i < 6; i++)                                        /* P0(n) -> P0(n-1) */
				raster->command_buffer[2+i] = raster->command_buffer[11+i]; /* P1(n) -> P1(n-1) */
		}
		break;

		case 1:
		{
			/* reuse P0(n-1) and P0(n) */
			for (i = 0; i < 3; i++)
				raster->command_buffer[5+i] = raster->command_buffer[11+i]; /* P0(n) -> P1(n-1) */
		}
		break;

		case 3:
		{
			/* reuse P1(n-1) and P1(n) */
			for (i = 0; i < 3; i++)
				raster->command_buffer[2+i] = raster->command_buffer[14+i]; /* P1(n) -> P1(n-1) */
		}
		break;
	}
}

/***********************************************************************************************/

/***********************************************************************************************/

void model2_renderer::model2_3d_render(triangle *tri, const rectangle &cliprect)
{
	model2_renderer *poly = m_state.m_poly.get();
	m2_poly_extra_data& extra = poly->object_data().next();
	u8 renderer;

	/* select renderer based on attributes (bit14 = textured, bit13 = transparent) */
	renderer = (tri->texheader[0] >> 13) & 3;

	/* calculate and clip to viewport */
	rectangle vp(tri->viewport[0] + m_xoffs, tri->viewport[2] + m_xoffs, (384-tri->viewport[3]) + m_yoffs, (384-tri->viewport[1]) + m_yoffs);
	vp &= cliprect;

	extra.state = &m_state;
	extra.checker = (tri->texheader[0] >> 15) & 1;
	extra.lumabase = (tri->texheader[1] & 0xff) << 7;
	extra.colorbase = (tri->texheader[3] >> 6) & 0x3ff;
	extra.luma = tri->luma;
	extra.texlod = tri->texlod;

	if (renderer & 2)
	{
		extra.texmirrorx = (tri->texheader[0] >> 8) & 1;
		extra.texmirrory = (tri->texheader[0] >> 9) & 1;

		// disable smooth wrapping if mirroring is enabled
		extra.texwrapx = (tri->texheader[0] >> 6) & 1 & ~extra.texmirrorx;
		extra.texwrapy = (tri->texheader[0] >> 7) & 1 & ~extra.texmirrory;

		extra.texsheet[0] = (tri->texheader[2] & 0x1000) ? m_state.m_textureram1 : m_state.m_textureram0;
		extra.texsheet[1] = (tri->texheader[2] & 0x1000) ? m_state.m_textureram0 : m_state.m_textureram1;
		extra.texwidth = 32 << ((tri->texheader[0] >> 0) & 0x7);
		extra.texheight = 32 << ((tri->texheader[0] >> 3) & 0x7);
		extra.texx = 32 * ((tri->texheader[2] >> 0) & 0x3f);
		extra.texy = 32 * ((tri->texheader[2] >> 6) & 0x1f);

		// microtexture parameters
		extra.utex = (tri->texheader[0] >> 12) & 1;
		extra.utexminlod = (tri->texheader[0] >> 10) & 3;
		extra.utexx = ((tri->texheader[2] >> 13) & 1) * 128;
		extra.utexy = ((tri->texheader[2] >> 14) & 3) * 128;

		tri->v[0].pz = 1.0f / (tri->v[0].pz + std::numeric_limits<float>::min());
		tri->v[0].pu = tri->v[0].pu * tri->v[0].pz * (1.0f / 8.0f);
		tri->v[0].pv = tri->v[0].pv * tri->v[0].pz * (1.0f / 8.0f);
		tri->v[1].pz = 1.0f / (tri->v[1].pz + std::numeric_limits<float>::min());
		tri->v[1].pu = tri->v[1].pu * tri->v[1].pz * (1.0f / 8.0f);
		tri->v[1].pv = tri->v[1].pv * tri->v[1].pz * (1.0f / 8.0f);
		tri->v[2].pz = 1.0f / (tri->v[2].pz + std::numeric_limits<float>::min());
		tri->v[2].pu = tri->v[2].pu * tri->v[2].pz * (1.0f / 8.0f);
		tri->v[2].pv = tri->v[2].pv * tri->v[2].pz * (1.0f / 8.0f);
	}

	render_triangle<3>(vp, m_render_callbacks[renderer], tri->v[0], tri->v[1], tri->v[2]);
}

/*
    Projection:

    According to the 2B Manual the screen coordinates are:

    (8,474)                         (504,474)
       +--------------------------------+
       |                                |
       |                                |
       |                                |
       |                                |
       |                                |
       |                                |
       |                                |
       |                                |
       +--------------------------------+
    (8,90)                          (504,90)
*/

void model2_state::horizontal_sync_w(u16 data)
{
	m_crtc_xoffset = 84 + (int16_t)data;
//  printf("H %04x %d %d\n",data,(int16_t)data,m_crtc_xoffset);
	m_poly->set_xoffset(m_crtc_xoffset);
}

void model2_state::vertical_sync_w(u16 data)
{
	m_crtc_yoffset = 130 + (int16_t)data;
//  printf("V %04x %d %d\n",data,(int16_t)data,m_crtc_yoffset);
	m_poly->set_yoffset(m_crtc_yoffset);
}

/* 3D Rasterizer projection: projects a triangle into screen coordinates */
inline void model2_state::model2_3d_project(triangle *tri)
{
	u16  i;

	for (i = 0; i < 3; i++)
	{
		/* project the vertices */
		tri->v[i].x = m_crtc_xoffset + tri->center[0] + (tri->v[i].x / (tri->v[i].pz + std::numeric_limits<float>::min()));
		tri->v[i].y = ((384 - tri->center[1])+m_crtc_yoffset) - (tri->v[i].y / (tri->v[i].pz + std::numeric_limits<float>::min()));
	}
}

/* 3D Rasterizer frame start: Resets frame variables */
void model2_state::render_frame_start()
{
	raster_state *raster = m_raster.get();

	/* reset the triangle list index */
	raster->tri_list_index = 0;

	/* reset the sorted z list */
	memset(raster->tri_sorted_list, 0, 0x10000 * sizeof(triangle *));

	/* reset the min-max sortable Z values */
	raster->min_z = 0xffff;
	raster->max_z = 0;

	/* reset the triangle z value */
	// Zero Gunner sets backgrounds with "previous z value" mode at the start of the display list,
	// needs this to be this big in order to work properly
	raster->triangle_z = 1e10;

	raster->cur_window = 0;

	m_render_done = false;
}

void model2_state::render_polygons(bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	raster_state *raster = m_raster.get();
	int32_t z;

	// if the geometrizer hasn't presented a new frame, just copy the previous frame and bail
	if (m_render_done)
	{
		copybitmap_trans(bitmap, m_poly->destmap(), 0, 0, 0, 0, cliprect, 0x00000000);
		return;
	}

	/* if we have nothing to render, bail */
	if (raster->tri_list_index == 0)
		return;

	m_poly->destmap().fill(0x00000000, cliprect);
	m_poly->fillmap().fill(0x00, cliprect);

	for (int window = raster->cur_window; window >= 0; window--)
	{
		/* go through the Z levels, and render each bucket */
		for (z = raster->min_z; z <= raster->max_z; z++)
		{
			/* see if we have items at this z level */
			if (raster->tri_sorted_list[z] != nullptr)
			{
				/* get a pointer to the first triangle */
				triangle *tri = raster->tri_sorted_list[z];

				/* and loop clipping and rendering each triangle */
				while (tri != nullptr)
				{
					if (tri->window == window)
					{
						/* project and render */
						model2_3d_project(tri);
						m_poly->model2_3d_render(tri, cliprect);
					}

					tri = (triangle *)tri->next;
				}
			}
		}
	}
	m_poly->wait("End of frame");

	copybitmap_trans(bitmap, m_poly->destmap(), 0, 0, 0, 0, cliprect, 0x00000000);

	m_render_done = true;
}

// direct framebuffer drawing (enabled with render test mode, Last Bronx title screen)
// pretty sure test mode cuts off DSP framebuffer drawing/clear, according to the manual description too
void model2_state::draw_framebuffer(bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	u16 *fbvram = &(m_screen->frame_number() & 1 ? m_fbvramB[0] : m_fbvramA[0]);
	int xoffs = -m_crtc_xoffset;
	int yoffs = (512 - 384) - m_crtc_yoffset;

	for (int y = cliprect.min_y; y < cliprect.max_y; ++y)
	{
		for (int x = cliprect.min_x; x < cliprect.max_x; x++)
		{
			int offset = (x + xoffs) + (y + yoffs)*512;
			// assuming the scroll color table is used
			int r = m_colorxlat[0x0080 / 2 + ((fbvram[offset] >> 5) & 0x1f) * 0x100];
			int g = m_colorxlat[0x4080 / 2 + ((fbvram[offset] >> 10) & 0x1f) * 0x100];
			int b = m_colorxlat[0x8080 / 2 + ((fbvram[offset] >> 0) & 0x1f) * 0x100];
			r = m_gamma_table[r];
			g = m_gamma_table[g];
			b = m_gamma_table[b];
			bitmap.pix(y, x) = r << 16 | g << 8 | b;
		}
	}
}

/* 3D Rasterizer main data input port */
void model2_state::model2_3d_push(raster_state *raster, u32 input)
{
	/* see if we have a command in progress */
	if (raster->cur_command != 0)
	{
		raster->command_buffer[raster->command_index++] = input;

		switch (raster->cur_command)
		{
			case 0x00:  /* NOP */
			break;

			case 0x01:  /* Polygon Data */
			{
				u32  attr;

				/* start by looking if we have the basic input data */
				if (raster->command_index < 9)
					return;

				/* get the attributes */
				attr = raster->command_buffer[8];

				/* see if we're done */
				if ((attr & 3) == 0)
				{
					raster->cur_command = 0;
					return;
				}

				/* see if it's a quad or a triangle */
				if (attr & 1)
				{
					/* it's a quad, wait for the rest of the points */
					if (raster->command_index < 17)
						return;

					/* we have a full polygon info, fill up our polygon structure */
					model2_3d_process_polygon<4>(raster, attr);

					/* back up and wait for more data */
					raster->command_index = 8;
				}
				else
				{
					/* it's a triangle, wait for the rest of the point */
					if (raster->command_index < 14)
						return;

					/* we have a full polygon info, fill up our polygon structure */
					model2_3d_process_polygon<3>(raster, attr);

					/* back up and wait for more data */
					raster->command_index = 8;
				}
			}
			break;

			case 0x03:  /* Window Data */
			{
				u32  i;

				/* make sure we have all the data */
				if (raster->command_index < 6)
					return;

				/* coordinates are 12 bit signed */

				/* extract the viewport start x */
				raster->viewport[0] = (raster->command_buffer[0] >> 12) & 0xfff;

				if (raster->viewport[0] & 0x800)
					raster->viewport[0] = -(0x800 - (raster->viewport[0] & 0x7ff));

				/* extract the viewport start y */
				raster->viewport[1] = raster->command_buffer[0] & 0xfff;

				if (raster->viewport[1] & 0x800)
					raster->viewport[1] = -(0x800 - (raster->viewport[1] & 0x7ff));

				/* extract the viewport end x */
				raster->viewport[2] = (raster->command_buffer[1] >> 12) & 0xfff;

				if (raster->viewport[2] & 0x800)
					raster->viewport[2] = -(0x800 - (raster->viewport[2] & 0x7ff));

				/* extract the viewport end y */
				raster->viewport[3] = raster->command_buffer[1] & 0xfff;

				if (raster->viewport[3] & 0x800)
					raster->viewport[3] = -(0x800 - (raster->viewport[3] & 0x7ff));

				/* extract the centers */
				for (i = 0; i < 4; i++)
				{
					/* center x */
					raster->center[i][0] = (raster->command_buffer[2+i] >> 12) & 0xfff;

					if (raster->center[i][0] & 0x800)
						raster->center[i][0] = -(0x800 - (raster->center[i][0] & 0x7ff));

					/* center y */
					raster->center[i][1] = raster->command_buffer[2+i] & 0xfff;

					if (raster->center[i][1] & 0x800)
						raster->center[i][1] = -(0x800 - (raster->center[i][1] & 0x7ff));

					// calculate clipping planes
					float left_plane = float(raster->center[i][0] - raster->viewport[0]);
					float right_plane = float(raster->viewport[2] - raster->center[i][0]);
					float top_plane = float(raster->viewport[3] - raster->center[i][1]);
					float bottom_plane = float(raster->center[i][1] - raster->viewport[1]);

					raster->clip_plane[i][0].normal.x = 1.0f / std::hypot(1.0f, left_plane);
					raster->clip_plane[i][0].normal.y = 0.0f;
					raster->clip_plane[i][0].normal.pz = left_plane / std::hypot(1.0f, left_plane);

					raster->clip_plane[i][1].normal.x = -1.0f / std::hypot(-1.0f, right_plane);
					raster->clip_plane[i][1].normal.y = 0.0f;
					raster->clip_plane[i][1].normal.pz = right_plane / std::hypot(-1.0f, right_plane);

					raster->clip_plane[i][2].normal.x = 0.0f;
					raster->clip_plane[i][2].normal.y = -1.0f / std::hypot(-1.0f, top_plane);
					raster->clip_plane[i][2].normal.pz = top_plane / std::hypot(-1.0f, top_plane);

					raster->clip_plane[i][3].normal.x = 0.0f;
					raster->clip_plane[i][3].normal.y = 1.0f / std::hypot(1.0f, bottom_plane);
					raster->clip_plane[i][3].normal.pz = bottom_plane / std::hypot(1.0f, bottom_plane);
				}

				/* done with this command */
				raster->cur_command = 0;
			}
			break;

			case 0x04:  /* Texture/Log Data write */
			{
				/* make sure we have enough data */
				if (raster->command_index < 2)
					return;

				/* see if the count is non-zero */
				if (raster->command_buffer[1] > 0)
				{
					/* see if we have data available */
					if (raster->command_index >= 3)
					{
						/* get the address */
						u32  address = raster->command_buffer[0];

						/* do the write */
						if (address & 0x800000)
							raster->texture_ram[address & 0xffff] = raster->command_buffer[2];
						else
							raster->log_ram[address & 0x7fff] = raster->command_buffer[2];

						/* increment the address and decrease the count */
						raster->command_buffer[0]++;
						raster->command_buffer[1]--;

						/* decrease the index, so we keep placing data in the same slot */
						raster->command_index--;
					}
				}

				/* see if we're done with this command */
				if (raster->command_buffer[1] == 0)
					raster->cur_command = 0;
			}
			break;

			case 0x08:  /* ZSort mode */
			{
				/* save the zsort mode value */
				raster->z_adjust = raster->command_buffer[0] << 8;

				/* done with this command */
				raster->cur_command = 0;
			}
			break;

			default:
			{
				fatalerror("SEGA 3D: Unknown rasterizer command %08x\n", raster->cur_command);
			}
		}
	}
	else
	{
		/* new command */
		raster->cur_command = input & 0x0f;
		raster->command_index = 0;

		/* see if it's object data */
		if (raster->cur_command == 1)
		{
			/* extract reverse bit */
			raster->reverse = (input >> 4) & 1;

			/* extract center select */
			raster->center_sel = (input >> 6) & 3;
		}
	}
}

/***********************************************************************************************/

/*******************************************
 *
 *  Geometry Engine Initialization
 *
 *******************************************/

void model2_state::geo_init(memory_region *polygon_rom)
{
	m_geo = std::make_unique<geo_state>();
	m_geo->state = this;

	m_geo->raster = m_raster.get();
	m_geo->polygon_rom = (u32 *)polygon_rom->base();
	m_geo->polygon_rom_mask = (polygon_rom->bytes() / 4) - 1;

	save_item(NAME(m_geo->mode));
	save_item(NAME(m_geo->matrix));
	save_item(NAME(m_geo->lod));
	save_item(NAME(m_geo->coef_table));
	save_item(NAME(m_geo->polygon_ram0));
	save_item(NAME(m_geo->polygon_ram1));
}

/*******************************************
 *
 *  Geometry Engine Polygon Parsers
 *
 *******************************************/

/* Parse Polygons: Normals Present, No Specular case */
void model2_state::geo_parse_np_ns(geo_state *geo, u32 *input, u32 count)
{
	raster_state *raster = geo->raster;
	poly_vertex point, normal;
	u32  attr, i;

	/* read the 1st point */
	point.x = u2f(*input++);
	point.y = u2f(*input++);
	point.pz = u2f(*input++);

	/* transform with the current matrix */
	transform_point(&point, geo->matrix);

	/* apply focus */
	apply_focus(geo, &point);

	/* push it to the 3d rasterizer */
	model2_3d_push(raster, f2u(point.x) >> 8);
	model2_3d_push(raster, f2u(point.y) >> 8);
	model2_3d_push(raster, f2u(point.pz) >> 8);

	/* read the 2nd point */
	point.x = u2f(*input++);
	point.y = u2f(*input++);
	point.pz = u2f(*input++);

	/* transform with the current matrix */
	transform_point(&point, geo->matrix);

	/* apply focus */
	apply_focus(geo, &point);

	/* push it to the 3d rasterizer */
	model2_3d_push(raster, f2u(point.x) >> 8);
	model2_3d_push(raster, f2u(point.y) >> 8);
	model2_3d_push(raster, f2u(point.pz) >> 8);

	/* loop through the following links */
	for (i = 0; i < count; i++)
	{
		/* read in the attributes */
		attr = *input++;

		/* push to the 3d rasterizer */
		model2_3d_push(raster, attr & 0x0003ffff);

		/* read in the normal */
		normal.x = u2f(*input++);
		normal.y = u2f(*input++);
		normal.pz = u2f(*input++);

		/* transform with the current matrix */
		transform_vector(&normal, geo->matrix);

		if ((attr & 3) != 0) /* quad or triangle */
		{
			float               dotl, dotp, luminance, distance;
			float               coef, face;
			int32_t               luma;
			texture_parameter * texparam;

			/* read in the next point */
			point.x = u2f(*input++);
			point.y = u2f(*input++);
			point.pz = u2f(*input++);

			/* transform with the current matrix */
			transform_point(&point, geo->matrix);

			/* calculate the dot product of the normal and the light vector */
			dotl = dot_product(normal, geo->light);

			/* calculate the dot product of the normal and the point */
			dotp = dot_product(normal, point);

			/* apply focus */
			apply_focus(geo, &point);

			/* determine whether this is the front or the back of the polygon */
			face = 0x100; /* rear */
			if (dotp >= 0) face = 0; /* front */

			/* get the texture parameters */
			texparam = &geo->texture_parameters[(attr>>18) & 0x1f];

			/* calculate luminance */
			if ((dotl * dotp) < 0) luminance = 0;
			else luminance = fabs(dotl);

			luminance = (luminance * texparam->diffuse) + texparam->ambient;
			luminance = std::clamp(luminance, 0.0f, 255.0f);

			luma = (int32_t)luminance;

			/* add the face bit to the luma */
			luma += face;

			/* extract distance coefficient */
			coef = geo->coef_table[attr>>27];

			/* calculate texture level of detail */
			distance = coef * fabs(dotp) * geo->lod;

			/* push to the 3d rasterizer */
			model2_3d_push(raster, luma << 15);
			model2_3d_push(raster, f2u(distance) >> 8);
			model2_3d_push(raster, f2u(point.x) >> 8);
			model2_3d_push(raster, f2u(point.y) >> 8);
			model2_3d_push(raster, f2u(point.pz) >> 8);

			/* if it's a quad, push one more point */
			if (attr & 1)
			{
				/* read in the next point */
				point.x = u2f(*input++);
				point.y = u2f(*input++);
				point.pz = u2f(*input++);

				/* transform with the current matrix */
				transform_point(&point, geo->matrix);

				/* apply focus */
				apply_focus(geo, &point);

				/* push to the 3d rasterizer */
				model2_3d_push(raster, f2u(point.x) >> 8);
				model2_3d_push(raster, f2u(point.y) >> 8);
				model2_3d_push(raster, f2u(point.pz) >> 8);
			}
			else /* triangle */
			{
				/* skip the next 3 points */
				input += 3;
			}
		}
		else /* we're done */
		{
			break;
		}
	}

	/* notify the 3d rasterizer we're done */
	model2_3d_push(raster, 0);
}

/* Parse Polygons: Normals Present, Specular case */
void model2_state::geo_parse_np_s(geo_state *geo, u32 *input, u32 count)
{
	raster_state *raster = geo->raster;
	poly_vertex point, normal;
	u32  attr, i;

	/* read the 1st point */
	point.x = u2f(*input++);
	point.y = u2f(*input++);
	point.pz = u2f(*input++);

	/* transform with the current matrix */
	transform_point(&point, geo->matrix);

	/* apply focus */
	apply_focus(geo, &point);

	/* push it to the 3d rasterizer */
	model2_3d_push(raster, f2u(point.x) >> 8);
	model2_3d_push(raster, f2u(point.y) >> 8);
	model2_3d_push(raster, f2u(point.pz) >> 8);

	/* read the 2nd point */
	point.x = u2f(*input++);
	point.y = u2f(*input++);
	point.pz = u2f(*input++);

	/* transform with the current matrix */
	transform_point(&point, geo->matrix);

	/* apply focus */
	apply_focus(geo, &point);

	/* push it to the 3d rasterizer */
	model2_3d_push(raster, f2u(point.x) >> 8);
	model2_3d_push(raster, f2u(point.y) >> 8);
	model2_3d_push(raster, f2u(point.pz) >> 8);

	/* loop through the following links */
	for (i = 0; i < count; i++)
	{
		/* read in the attributes */
		attr = *input++;

		/* push to the 3d rasterizer */
		model2_3d_push(raster, attr & 0x0003ffff);

		/* read in the normal */
		normal.x = u2f(*input++);
		normal.y = u2f(*input++);
		normal.pz = u2f(*input++);

		/* transform with the current matrix */
		transform_vector(&normal, geo->matrix);

		if ((attr & 3) != 0) /* quad or triangle */
		{
			float               dotl, dotp, luminance, distance, specular;
			float               coef, face;
			int32_t             luma;
			texture_parameter * texparam;

			/* read in the next point */
			point.x = u2f(*input++);
			point.y = u2f(*input++);
			point.pz = u2f(*input++);

			/* transform with the current matrix */
			transform_point(&point, geo->matrix);

			/* calculate the dot product of the normal and the light vector */
			dotl = dot_product(normal, geo->light);

			/* calculate the dot product of the normal and the point */
			dotp = dot_product(normal, point);

			/* apply focus */
			apply_focus(geo, &point);

			/* determine whether this is the front or the back of the polygon */
			face = 0x100; /* rear */
			if (dotp >= 0) face = 0; /* front */

			/* get the texture parameters */
			texparam = &geo->texture_parameters[(attr>>18) & 0x1f];

			/* calculate luminance and specular */
			if ((dotl * dotp) < 0) luminance = 0;
			else luminance = fabs(dotl);

			specular = ((2*dotl) * normal.pz) - geo->light.pz;
			if (specular < 0) specular = 0;
			if (texparam->specular_control == 0) specular = 0;
			if ((texparam->specular_control >> 1) != 0) specular *= specular;
			if ((texparam->specular_control >> 2) != 0) specular *= specular;
			if (((texparam->specular_control+1) >> 3) != 0) specular *= specular;

			specular *= texparam->specular_scale;

			luminance = (luminance * texparam->diffuse) + texparam->ambient + specular;
			luminance = std::clamp(luminance, 0.0f, 255.0f);

			luma = (int32_t)luminance;

			/* add the face bit to the luma */
			luma += face;

			/* extract distance coefficient */
			coef = geo->coef_table[attr>>27];

			/* calculate texture level of detail */
			distance = coef * fabs(dotp) * geo->lod;

			/* push to the 3d rasterizer */
			model2_3d_push(raster, luma << 15);
			model2_3d_push(raster, f2u(distance) >> 8);
			model2_3d_push(raster, f2u(point.x) >> 8);
			model2_3d_push(raster, f2u(point.y) >> 8);
			model2_3d_push(raster, f2u(point.pz) >> 8);

			/* if it's a quad, push one more point */
			if (attr & 1)
			{
				/* read in the next point */
				point.x = u2f(*input++);
				point.y = u2f(*input++);
				point.pz = u2f(*input++);

				/* transform with the current matrix */
				transform_point(&point, geo->matrix);

				/* apply focus */
				apply_focus(geo, &point);

				/* push to the 3d rasterizer */
				model2_3d_push(raster, f2u(point.x) >> 8);
				model2_3d_push(raster, f2u(point.y) >> 8);
				model2_3d_push(raster, f2u(point.pz) >> 8);
			}
			else /* triangle */
			{
				/* skip the next 3 points */
				input += 3;
			}
		}
		else /* we're done */
		{
			break;
		}
	}

	/* notify the 3d rasterizer we're done */
	model2_3d_push(raster, 0);
}

/* Parse Polygons: No Normals, No Specular case */
void model2_state::geo_parse_nn_ns(geo_state *geo, u32 *input, u32 count)
{
	raster_state *raster = geo->raster;
	poly_vertex point, normal, p0, p1, p2, p3;
	u32  attr, i;

	/* read the 1st point */
	point.x = u2f(*input++);
	point.y = u2f(*input++);
	point.pz = u2f(*input++);

	/* transform with the current matrix */
	transform_point(&point, geo->matrix);

	/* save for normal calculation */
	p0.x = point.x; p0.y = point.y; p0.pz = point.pz;

	/* apply focus */
	apply_focus(geo, &point);

	/* push it to the 3d rasterizer */
	model2_3d_push(raster, f2u(point.x) >> 8);
	model2_3d_push(raster, f2u(point.y) >> 8);
	model2_3d_push(raster, f2u(point.pz) >> 8);

	/* read the 2nd point */
	point.x = u2f(*input++);
	point.y = u2f(*input++);
	point.pz = u2f(*input++);

	/* transform with the current matrix */
	transform_point(&point, geo->matrix);

	/* save for normal calculation */
	p1.x = point.x; p1.y = point.y; p1.pz = point.pz;

	/* apply focus */
	apply_focus(geo, &point);

	/* push it to the 3d rasterizer */
	model2_3d_push(raster, f2u(point.x) >> 8);
	model2_3d_push(raster, f2u(point.y) >> 8);
	model2_3d_push(raster, f2u(point.pz) >> 8);

	/* loop through the following links */
	for (i = 0; i < count; i++)
	{
		/* read in the attributes */
		attr = *input++;

		/* push to the 3d rasterizer */
		model2_3d_push(raster, attr & 0x0003ffff);

		if ((attr & 3) != 0) /* quad or triangle */
		{
			float               dotl, dotp, luminance, distance;
			float               coef, face;
			int32_t             luma;
			texture_parameter * texparam;

			/* Skip normal */
			input += 3;

			/* read in the next point */
			point.x = u2f(*input++);
			point.y = u2f(*input++);
			point.pz = u2f(*input++);

			/* transform with the current matrix */
			transform_point(&point, geo->matrix);

			/* save for normal calculation */
			p2.x = point.x; p2.y = point.y; p2.pz = point.pz;

			/* compute the normal */
			vector_cross3(&normal, &p0, &p1, &p2);

			/* normalize it */
			normalize_vector(&normal);

			/* calculate the dot product of the normal and the light vector */
			dotl = dot_product(normal, geo->light);

			/* calculate the dot product of the normal and the point */
			dotp = dot_product(normal, point);

			/* apply focus */
			apply_focus(geo, &point);

			/* determine whether this is the front or the back of the polygon */
			face = 0x100; /* rear */
			if (dotp >= 0) face = 0; /* front */

			/* get the texture parameters */
			texparam = &geo->texture_parameters[(attr>>18) & 0x1f];

			/* calculate luminance */
			if ((dotl * dotp) < 0) luminance = 0;
			else luminance = fabs(dotl);

			luminance = (luminance * texparam->diffuse) + texparam->ambient;
			luminance = std::clamp(luminance, 0.0f, 255.0f);

			luma = (int32_t)luminance;

			/* add the face bit to the luma */
			luma += face;

			/* extract distance coefficient */
			coef = geo->coef_table[attr>>27];

			/* calculate texture level of detail */
			distance = coef * fabs(dotp) * geo->lod;

			/* push to the 3d rasterizer */
			model2_3d_push(raster, luma << 15);
			model2_3d_push(raster, f2u(distance) >> 8);
			model2_3d_push(raster, f2u(point.x) >> 8);
			model2_3d_push(raster, f2u(point.y) >> 8);
			model2_3d_push(raster, f2u(point.pz) >> 8);

			/* if it's a quad, push one more point */
			if (attr & 1)
			{
				/* read in the next point */
				point.x = u2f(*input++);
				point.y = u2f(*input++);
				point.pz = u2f(*input++);

				/* transform with the current matrix */
				transform_point(&point, geo->matrix);

				/* save for normal calculation */
				p3.x = point.x; p3.y = point.y; p3.pz = point.pz;

				/* apply focus */
				apply_focus(geo, &point);

				/* push to the 3d rasterizer */
				model2_3d_push(raster, f2u(point.x) >> 8);
				model2_3d_push(raster, f2u(point.y) >> 8);
				model2_3d_push(raster, f2u(point.pz) >> 8);
			}
			else
			{
				/* skip the next 3 points */
				input += 3;

				/* for triangles, the rope of P1(n) is achieved by P0(n-1) (linktype 3) */
				p3.x = p2.x; p3.y = p2.y; p3.pz = p2.pz;
			}
		}
		else /* we're done */
		{
			break;
		}

		/* link type */
		switch ((attr>>8) & 3)
		{
			case 0:
			case 2:
			{
				/* reuse P0(n) and P1(n) */
				p0.x = p2.x; p0.y = p2.y; p0.pz = p2.pz;
				p1.x = p3.x; p1.y = p3.y; p1.pz = p3.pz;
			}
			break;

			case 1:
			{
				/* reuse P0(n-1) and P0(n) */
				p1.x = p2.x; p1.y = p2.y; p1.pz = p2.pz;
			}
			break;

			case 3:
			{
				/* reuse P1(n-1) and P1(n) */
				p0.x = p3.x; p0.y = p3.y; p0.pz = p3.pz;
			}
			break;
		}
	}

	/* notify the 3d rasterizer we're done */
	model2_3d_push(raster, 0);
}

/* Parse Polygons: No Normals, Specular case */
void model2_state::geo_parse_nn_s(geo_state *geo, u32 *input, u32 count)
{
	raster_state *raster = geo->raster;
	poly_vertex point, normal, p0, p1, p2, p3;
	u32  attr, i;

	/* read the 1st point */
	point.x = u2f(*input++);
	point.y = u2f(*input++);
	point.pz = u2f(*input++);

	/* transform with the current matrix */
	transform_point(&point, geo->matrix);

	/* save for normal calculation */
	p0.x = point.x; p0.y = point.y; p0.pz = point.pz;

	/* apply focus */
	apply_focus(geo, &point);

	/* push it to the 3d rasterizer */
	model2_3d_push(raster, f2u(point.x) >> 8);
	model2_3d_push(raster, f2u(point.y) >> 8);
	model2_3d_push(raster, f2u(point.pz) >> 8);

	/* read the 2nd point */
	point.x = u2f(*input++);
	point.y = u2f(*input++);
	point.pz = u2f(*input++);

	/* transform with the current matrix */
	transform_point(&point, geo->matrix);

	/* save for normal calculation */
	p1.x = point.x; p1.y = point.y; p1.pz = point.pz;

	/* apply focus */
	apply_focus(geo, &point);

	/* push it to the 3d rasterizer */
	model2_3d_push(raster, f2u(point.x) >> 8);
	model2_3d_push(raster, f2u(point.y) >> 8);
	model2_3d_push(raster, f2u(point.pz) >> 8);

	/* loop through the following links */
	for (i = 0; i < count; i++)
	{
		/* read in the attributes */
		attr = *input++;

		/* push to the 3d rasterizer */
		model2_3d_push(raster, attr & 0x0003ffff);

		if ((attr & 3) != 0) /* quad or triangle */
		{
			float               dotl, dotp, luminance, distance, specular;
			float               coef, face;
			int32_t             luma;
			texture_parameter * texparam;

			/* Skip normal */
			input += 3;

			/* read in the next point */
			point.x = u2f(*input++);
			point.y = u2f(*input++);
			point.pz = u2f(*input++);

			/* transform with the current matrix */
			transform_point(&point, geo->matrix);

			/* save for normal calculation */
			p2.x = point.x; p2.y = point.y; p2.pz = point.pz;

			/* compute the normal */
			vector_cross3(&normal, &p0, &p1, &p2);

			/* normalize it */
			normalize_vector(&normal);

			/* calculate the dot product of the normal and the light vector */
			dotl = dot_product(normal, geo->light);

			/* calculate the dot product of the normal and the point */
			dotp = dot_product(normal, point);

			/* apply focus */
			apply_focus(geo, &point);

			/* determine whether this is the front or the back of the polygon */
			face = 0x100; /* rear */
			if (dotp >= 0) face = 0; /* front */

			/* get the texture parameters */
			texparam = &geo->texture_parameters[(attr>>18) & 0x1f];

			/* calculate luminance and specular */
			if ((dotl * dotp) < 0) luminance = 0;
			else luminance = fabs(dotl);

			specular = ((2*dotl) * normal.pz) - geo->light.pz;
			if (specular < 0) specular = 0;
			if (texparam->specular_control == 0) specular = 0;
			if ((texparam->specular_control >> 1) != 0) specular *= specular;
			if ((texparam->specular_control >> 2) != 0) specular *= specular;
			if (((texparam->specular_control+1) >> 3) != 0) specular *= specular;

			specular *= texparam->specular_scale;

			luminance = (luminance * texparam->diffuse) + texparam->ambient + specular;
			luminance = std::clamp(luminance, 0.0f, 255.0f);

			luma = (int32_t)luminance;

			/* add the face bit to the luma */
			luma += face;

			/* extract distance coefficient */
			coef = geo->coef_table[attr>>27];

			/* calculate texture level of detail */
			distance = coef * fabs(dotp) * geo->lod;

			/* push to the 3d rasterizer */
			model2_3d_push(raster, luma << 15);
			model2_3d_push(raster, f2u(distance) >> 8);
			model2_3d_push(raster, f2u(point.x) >> 8);
			model2_3d_push(raster, f2u(point.y) >> 8);
			model2_3d_push(raster, f2u(point.pz) >> 8);

			/* if it's a quad, push one more point */
			if (attr & 1)
			{
				/* read in the next point */
				point.x = u2f(*input++);
				point.y = u2f(*input++);
				point.pz = u2f(*input++);

				/* transform with the current matrix */
				transform_point(&point, geo->matrix);

				/* save for normal calculation */
				p3.x = point.x; p3.y = point.y; p3.pz = point.pz;

				/* apply focus */
				apply_focus(geo, &point);

				/* push to the 3d rasterizer */
				model2_3d_push(raster, f2u(point.x) >> 8);
				model2_3d_push(raster, f2u(point.y) >> 8);
				model2_3d_push(raster, f2u(point.pz) >> 8);
			}
			else
			{
				/* skip the next 3 points */
				input += 3;

				/* for triangles, the rope of P1(n) is achieved by P0(n-1) (linktype 3) */
				p3.x = p2.x; p3.y = p2.y; p3.pz = p2.pz;
			}
		}
		else /* we're done */
		{
			break;
		}

		/* link type */
		switch ((attr>>8) & 3)
		{
			case 0:
			case 2:
			{
				/* reuse P0(n) and P1(n) */
				p0.x = p2.x; p0.y = p2.y; p0.pz = p2.pz;
				p1.x = p3.x; p1.y = p3.y; p1.pz = p3.pz;
			}
			break;

			case 1:
			{
				/* reuse P0(n-1) and P0(n) */
				p1.x = p2.x; p1.y = p2.y; p1.pz = p2.pz;
			}
			break;

			case 3:
			{
				/* reuse P1(n-1) and P1(n) */
				p0.x = p3.x; p0.y = p3.y; p0.pz = p3.pz;
			}
			break;
		}
	}

	/* notify the 3d rasterizer we're done */
	model2_3d_push(raster, 0);
}

/*******************************************
 *
 *  Geometry Engine Commands
 *
 *******************************************/

/* Command 00: NOP */
u32 *model2_state::geo_nop(geo_state *geo, u32 opcode, u32 *input)
{
	raster_state *raster = geo->raster;

	/* push the opcode to the 3d rasterizer */
	model2_3d_push(raster, opcode >> 23);

	return input;
}

/* Command 01: Object Data */
u32 *model2_state::geo_object_data(geo_state *geo, u32 opcode, u32 *input)
{
	raster_state *raster = geo->raster;
	u32  tpa = *input++;     /* Texture Point Address */
	u32  tha = *input++;     /* Texture Header Address */
	u32  oba = *input++;     /* Object Address */
	u32  obc = *input++;     /* Object Count */

	u32 *obp;                /* Object Pointer */

	/* push the initial set of data to the 3d rasterizer */
	model2_3d_push(raster, opcode >> 23);
	model2_3d_push(raster, tpa);
	model2_3d_push(raster, tha);

	/* select where we're reading polygon information from */
	if (oba & 0x01000000)
	{
		/* Fast polygon RAM */
		obp = &geo->polygon_ram1[oba & 0x7fff];
	}
	else if (oba & 0x00800000)
	{
		/* Polygon ROM */
		obp = &geo->polygon_rom[oba & geo->polygon_rom_mask];
	}
	else
	{
		/* Slow Polygon RAM */
		obp = &geo->polygon_ram0[oba & 0x7fff];
	}

	// if count == 0 then rolls over to max size
	// Virtual On & Gunblade NY
	if (obc == 0)
		obc = 0xfffff;

	switch (geo->mode & 3)
	{
		/* Normals present, No Specular */
		case 0: geo_parse_np_ns(geo, obp, obc); break;

		/* Normals present, Specular */
		case 1: geo_parse_np_s(geo, obp, obc); break;

		/* No Normals present, No Specular */
		case 2: geo_parse_nn_ns(geo, obp, obc); break;

		/* No Normals present, Specular */
		case 3: geo_parse_nn_s(geo, obp, obc); break;
	}

	/* move by 4 parameters */
	return input;
}

/* Command 02: Direct Data */
u32 *model2_state::geo_direct_data(geo_state *geo, u32 opcode, u32 *input)
{
	raster_state *raster = geo->raster;
	u32  tpa = *input++;     /* Texture Point Address */
	u32  tha = *input++;     /* Texture Header Address */

	/* push the initial set of data to the 3d rasterizer */
	model2_3d_push(raster, (opcode >> 23) - 1);
	model2_3d_push(raster, tpa);
	model2_3d_push(raster, tha);

	/* push the initial points */
	model2_3d_push(raster, (*input++) >> 8); /* x */
	model2_3d_push(raster, (*input++) >> 8); /* y */
	model2_3d_push(raster, (*input++) >> 8); /* z */

	model2_3d_push(raster, (*input++) >> 8); /* x */
	model2_3d_push(raster, (*input++) >> 8); /* y */
	model2_3d_push(raster, (*input++) >> 8); /* z */

	/* read in the attributes */
	u32  attr;
	while (((attr = *input++) & 3) != 0)
	{
		/* push attributes */
		model2_3d_push(raster, attr & 0x00ffffff);

		/* push luma */
		model2_3d_push(raster, (*input++) >> 8);

		/* push distance */
		model2_3d_push(raster, (*input++) >> 8);

		/* push the next point */
		model2_3d_push(raster, (*input++) >> 8); /* x */
		model2_3d_push(raster, (*input++) >> 8); /* y */
		model2_3d_push(raster, (*input++) >> 8); /* z */

		/* if it's a quad, output another point */
		if (attr & 1)
		{
			model2_3d_push(raster, (*input++) >> 8); /* x */
			model2_3d_push(raster, (*input++) >> 8); /* y */
			model2_3d_push(raster, (*input++) >> 8); /* z */
		}
	}

	/* we're done */
	model2_3d_push(raster, 0);

	return input;
}

/* Command 03: Window Data */
u32 *model2_state::geo_window_data(geo_state *geo, u32 opcode, u32 *input)
{
	raster_state *raster = geo->raster;

	/* start by pushing the opcode */
	model2_3d_push(raster, opcode >> 23);

	raster->cur_window++;

	/*
	    we're going to move 6 coordinates to the 3d rasterizer:
	    - starting coordinate
	    - completion coordinate
	    - vanishing point 0 (eye mode 0)
	    - vanishing point 1 (eye mode 1)
	    - vanishing point 2 (eye mode 2)
	    - vanishing point 3 (eye mode 3)
	*/

	for (u32 i = 0; i < 6; i++)
	{
		/* read in the coordinate */
		u32 y = *input++;

		/* convert to the 3d rasterizer format (00XXXYYY) */
		u32 x = (y & 0x0fff0000) >> 4 ;
		y &= 0xfff;

		/* push it */
		model2_3d_push(raster, x | y);
	}

	return input;
}

/* Command 04: Texture Data Write */
u32 *model2_state::geo_texture_data(geo_state *geo, u32 opcode, u32 *input)
{
	raster_state *raster = geo->raster;

	/* start by pushing the opcode */
	model2_3d_push(raster, opcode >> 23);

	/* push the starting address/dsp id */
	model2_3d_push(raster, *input++);

	/* get the count */
	u32 count = *input++;

	/* push the count */
	model2_3d_push(raster, count);

	/* loop and send the data */
	for (u32 i = 0; i < count; i++)
		model2_3d_push(raster, *input++);

	return input;
}

/* Command 05: Polygon Data */
u32 *model2_state::geo_polygon_data(geo_state *geo, u32 opcode, u32 *input)
{
	u32  address, count, i;
	u32 *p;

	(void)opcode;

	/* read in the address */
	address = *input++;

	/* prepare the pointer */
	if (address & 0x01000000)
	{
		/* Fast polygon RAM */
		p = &geo->polygon_ram1[address & 0x7fff];
	}
	else
	{
		/* Slow Polygon RAM */
		p = &geo->polygon_ram0[address & 0x7fff];
	}

	/* read the count */
	count = *input++;

	/* move the data */
	for (i = 0; i < count; i++)
		*p++ = *input++;

	return input;
}

/* Command 06: Texture Parameters */
u32 *model2_state::geo_texture_parameters(geo_state *geo, u32 opcode, u32 *input)
{
	u32  index, count, i, param;

	(void)opcode;

	/* read in the index */
	index = (*input++) >> 2;

	/* read in the conut */
	count = *input++;

	for (i = 0; i < count; i++)
	{
		/* read in the texture parameters */
		param = *input++;

		geo->texture_parameters[index].diffuse = float(param & 0xff);
		geo->texture_parameters[index].ambient = float((param >> 8) & 0xff);
		geo->texture_parameters[index].specular_control = (param >> 24) & 0xff;
		geo->texture_parameters[index].specular_scale = float((param >> 16) & 0xff);

		/* read in the distance coefficient */
		geo->coef_table[index] = u2f(*input++);

		index = (index + 1) & 0x1f;
	}

	return input;
}

/* Command 07: Geo Mode */
u32 *model2_state::geo_mode(geo_state *geo, u32 opcode, u32 *input)
{
	(void)opcode;

	/* read in the mode */
	geo->mode = *input++;

	return input;
}

/* Command 08: ZSort Mode */
u32 *model2_state::geo_zsort_mode(geo_state *geo, u32 opcode, u32 *input)
{
	raster_state *raster = geo->raster;

	/* push the opcode */
	model2_3d_push(raster, opcode >> 23);

	/* push the mode */
	model2_3d_push(raster, (*input++) >> 8);

	return input;
}

/* Command 09: Focal Distance */
u32 *model2_state::geo_focal_distance(geo_state *geo, u32 opcode, u32 *input)
{
	(void)opcode;

	/* read the x focus value */
	geo->focus.x = u2f(*input++);

	/* read the y focus value */
	geo->focus.y = u2f(*input++);

	return input;
}

/* Command 0A: Light Source Vector Write */
u32 *model2_state::geo_light_source(geo_state *geo, u32 opcode, u32 *input)
{
	(void)opcode;

	/* read the x light value */
	geo->light.x = u2f(*input++);

	/* read the y light value */
	geo->light.y = u2f(*input++);

	/* read the z light value */
	geo->light.pz = u2f(*input++);

	return input;
}

/* Command 0B: Transformation Matrix Write */
u32 *model2_state::geo_matrix_write(geo_state *geo, u32 opcode, u32 *input)
{
	u32  i;

	(void)opcode;

	/* read in the transformation matrix */
	for (i = 0; i < 12; i++)
		geo->matrix[i] = u2f(*input++);

	return input;
}

/* Command 0C: Parallel Transfer Vector Write */
u32 *model2_state::geo_translate_write(geo_state *geo, u32 opcode, u32 *input)
{
	u32  i;

	(void)opcode;

	/* read in the translation vector */
	for (i = 0; i < 3; i++)
		geo->matrix[i+9] = u2f(*input++);

	return input;
}

/* Command 0D: Geo Data Memory Push (undocumented, unsupported) */
u32 *model2_state::geo_data_mem_push(geo_state *geo, u32 opcode, u32 *input)
{
	u32  address, count, i;

	/*
	    This command pushes data stored in the Geometry DSP's RAM
	    to the hardware 3D rasterizer. Since we don't emulate the
	    DSP, we don't know what the RAM contents are.

	    Eventually, we could check for the address, and if it
	    happens to point to a polygon ROM, we could potentially
	    emulate it partially.

	    No games are known to use this command yet.
	*/


	(void)opcode;

	/* read in the address */
	address = *input++;

	/* read in the count */
	count = *input++;

	logerror("SEGA GEO: Executing unsupported geo_data_mem_push (address = %08x, count = %08x)\n", address, count);

	(void)i;
/*
    for (i = 0; i < count; i++)
        model2_3d_push(0);
*/

	return input;
}

/* Command 0E: Geo Test */
u32 *model2_state::geo_test(geo_state *geo, u32 opcode, u32 *input)
{
	u32      data, blocks, address, count, checksum, i;

	(void)opcode;

	/* fifo test */
	data = 1;

	for (i = 0; i < 32; i++)
	{
		if (*input++ != data)
		{
			/* TODO: Set Red LED on */
			logerror("SEGA GEO: FIFO Test failed\n");
		}

		data <<= 1;
	}

	/* get the number of checksums we have to run */
	blocks = *input++;

	for (i = 0; i < blocks; i++)
	{
		u32  sum_even, sum_odd, j;

		/* read in the address */
		address = (*input++) & 0x7fffff;

		/* read in the count */
		count = *input++;

		/* read in the checksum */
		checksum = *input++;

		/* reset the checksum counters */
		sum_even = 0;
		sum_odd = 0;

		for (j = 0; j < count; j++)
		{
			data = geo->polygon_rom[address++];

			address &= geo->polygon_rom_mask;

			sum_even += data >> 16;
			sum_even &= 0xffff;

			sum_odd += data & 0xffff;
			sum_odd &= 0xffff;
		}

		sum_even += checksum >> 16;
		sum_even &= 0xffff;

		sum_odd += checksum & 0xffff;
		sum_odd &= 0xffff;

		if (sum_even != 0 || sum_odd != 0)
		{
			/* TODO: Set Green LED on */
			logerror("SEGA GEO: Polygon ROM Test failed\n");
		}
	}

	return input;
}

/* Command 0F: End */
u32 *model2_state::geo_end(geo_state *geo, u32 opcode, u32 *input)
{
	raster_state *raster = geo->raster;

	(void)opcode;

	/* signal the end of this data block the rasterizer */
	model2_3d_push(raster, 0xff000000);

	/* signal end by returning nullptr */
	return nullptr;
}

/* Command 10: Dummy */
u32 *model2_state::geo_dummy(geo_state *geo, u32 opcode, u32 *input)
{
//  u32  data;
	(void)opcode;

	/* do the dummy read cycle */
//  data = *input++;
	input++;

	return input;
}

/* Command 14: Log Data Write */
u32 *model2_state::geo_log_data(geo_state *geo, u32 opcode, u32 *input)
{
	raster_state *raster = geo->raster;
	u32  i, count;

	/* start by pushing the opcode */
	model2_3d_push(raster, opcode >> 23);

	/* push the starting address/dsp id */
	model2_3d_push(raster, *input++);

	/* get the count */
	count = *input++;

	/* push the count */
	model2_3d_push(raster, count << 2);

	/* loop and send the data */
	for (i = 0; i < count; i++)
	{
		u32  data = *input++;

		model2_3d_push(raster, data & 0xff);
		model2_3d_push(raster, (data >> 8) & 0xff);
		model2_3d_push(raster, (data >> 16) & 0xff);
		model2_3d_push(raster, (data >> 24) & 0xff);
	}

	return input;
}

/* Command 16: LOD */
u32 *model2_state::geo_lod(geo_state *geo, u32 opcode, u32 *input)
{
	(void)opcode;

	/* read in the LOD */
	geo->lod = u2f(*input++);

	return input;
}

/* Command 1D: Code Upload  (undocumented, unsupported) */
u32 *model2_state::geo_code_upload(geo_state *geo, u32 opcode, u32 *input)
{
	u32  count, i;

	/*
	    This command uploads code to program memory and
	    optionally runs it. Probably used for debugging.

	    No games are known to use this command yet.
	*/

	logerror("SEGA GEO: Uploading debug code (unimplemented)\n");

	(void)opcode;

	/* read in the flags */
//  flags = *input++;
	input++;

	/* read in the count */
	count = *input++;

	for (i = 0; i < count; i++)
	{
		[[maybe_unused]] u64  code;

		/* read the top part of the opcode */
		code = *input++;

		code <<= 32;

		/* the bottom part comes in two pieces */
		code |= *input++;
		code |= (*input++) << 16;
	}

	/*
	    Bit 10 of flags indicate whether to run iummediately after upload
	*/

/*
    if (flags & 0x400)
        code_jump();
*/

	return input;
}

/* Command 1E: Code Jump (undocumented, unsupported) */
u32 *model2_state::geo_code_jump(geo_state *geo, u32 opcode, u32 *input)
{
//  u32  address;

	/*
	    This command jumps to a specified address in program
	    memory. Code can be uploaded with function 1D.
	    Probably used for debugging.

	    No games are known to use this command yet.
	*/

	logerror("SEGA GEO: Jumping to debug code (unimplemented)\n");

	(void)opcode;

//  address = *input++ & 0x3ff;
	input++;

/*
    code_jump(address)
*/
	return input;
}

u32 *model2_state::geo_process_command(geo_state *geo, u32 opcode, u32 *input, bool *end_code)
{
	switch ((opcode >> 23) & 0x1f)
	{
		case 0x00: input = geo_nop(geo, opcode, input);                   break;
		case 0x01: input = geo_object_data(geo, opcode, input);           break;
		case 0x02: input = geo_direct_data(geo, opcode, input);           break;
		case 0x03: input = geo_window_data(geo, opcode, input);           break;
		case 0x04: input = geo_texture_data(geo, opcode, input);          break;
		case 0x05: input = geo_polygon_data(geo, opcode, input);          break;
		case 0x06: input = geo_texture_parameters(geo, opcode, input);    break;
		case 0x07: input = geo_mode(geo, opcode, input);                  break;
		case 0x08: input = geo_zsort_mode(geo, opcode, input);            break;
		case 0x09: input = geo_focal_distance(geo, opcode, input);        break;
		case 0x0a: input = geo_light_source(geo, opcode, input);          break;
		case 0x0b: input = geo_matrix_write(geo, opcode, input);          break;
		case 0x0c: input = geo_translate_write(geo, opcode, input);       break;
		case 0x0d: input = geo_data_mem_push(geo, opcode, input);         break;
		case 0x0e: input = geo_test(geo, opcode, input);                  break;
		case 0x0f: input = geo_end(geo, opcode, input); *end_code = true; break;
		case 0x10: input = geo_dummy(geo, opcode, input);                 break;
		case 0x11: input = geo_object_data(geo, opcode, input);           break;
		case 0x12: input = geo_direct_data(geo, opcode, input);           break;
		case 0x13: input = geo_window_data(geo, opcode, input);           break;
		case 0x14: input = geo_log_data(geo, opcode, input);              break;
		case 0x15: input = geo_polygon_data(geo, opcode, input);          break;
		case 0x16: input = geo_lod(geo, opcode, input);                   break;
		case 0x17: input = geo_mode(geo, opcode, input);                  break;
		case 0x18: input = geo_zsort_mode(geo, opcode, input);            break;
		case 0x19: input = geo_focal_distance(geo, opcode, input);        break;
		case 0x1a: input = geo_light_source(geo, opcode, input);          break;
		case 0x1b: input = geo_matrix_write(geo, opcode, input);          break;
		case 0x1c: input = geo_translate_write(geo, opcode, input);       break;
		case 0x1d: input = geo_code_upload(geo, opcode, input);           break;
		case 0x1e: input = geo_code_jump(geo, opcode, input);             break;
		case 0x1f: input = geo_end(geo, opcode, input); *end_code = true; break;
	}

	return input;
}

void model2_state::geo_parse()
{
	u32  address = (m_geo_read_start_address & 0x1ffff)/4;
	u32 *input = &m_bufferram[address];
	u32  opcode;
	u32  op_count = 0;
	bool end_code = false;

	// reset raster frame variables
	render_frame_start();

	while (end_code == false && (input - m_bufferram) < 0x20000/4 && op_count++ < 0x8000)
	{
		/* read in the opcode */
		opcode = *input++;

		/* if it's a jump opcode, do the jump */
		if (opcode & 0x80000000)
		{
			/* get the address */
			address = (opcode & 0x1ffff) / 4;

			/* update our pointer */
			input = &m_bufferram[address];

			/* go again */
			continue;
		}

		/* process it */
		input = geo_process_command(m_geo.get(), opcode, input, &end_code);
	}
}

/***********************************************************************************************/


void model2_state::video_start()
{
	const rectangle &visarea = m_screen->visible_area();
	int width = visarea.width();
	int height = visarea.height();

	m_sys24_bitmap.allocate(width, height+4);

	m_poly = std::make_unique<model2_renderer>(*this);

	/* initialize the hardware rasterizer */
	raster_init(memregion("textures"));

	/* initialize the geometry engine */
	geo_init(memregion("polygons"));

	/* init various video-related pointers */
	m_palram = make_unique_clear<u16[]>(0x4000/2);
	m_colorxlat = make_unique_clear<u16[]>(0xc000/2);
	m_lumaram = make_unique_clear<u8[]>(0x8000);
	m_fbvramA = make_unique_clear<u16[]>(0x80000/2);
	m_fbvramB = make_unique_clear<u16[]>(0x80000/2);

	// convert color space; this works OK for most games
	// real cabinets probably have their monitors calibrated depending on the game
	// optimal settings for vf2, fvipers and schamp are bias 64, gain 51
	for (int i = 0; i < 256; i++)
	{
		double raw_value = std::max(((double)i - 64.0) * 255.0 / 191.0, 0.0);
		m_gamma_table[i] = (u8)raw_value;
//      printf("%02x: %02x %lf\n",i,m_gamma_table[i],raw_value);
	}

	save_item(NAME(m_render_test_mode));
	save_item(NAME(m_render_unk));
	save_item(NAME(m_render_mode));
	save_item(NAME(m_render_done));
	save_pointer(NAME(m_palram), 0x4000/2);
	save_pointer(NAME(m_colorxlat), 0xc000/2);
	save_pointer(NAME(m_lumaram), 0x8000);
	save_pointer(NAME(m_gamma_table), 256);
}

u32 model2_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	// if the scroll color table was written to, we need to refresh the palette
	if (m_palette_dirty)
	{
		for (int i = 0; i < 0x1000; i++)
		{
			u16 palcolor = m_palram[i];
			u8 r = m_colorxlat[0x0080 / 2 + ((palcolor >> 0) & 0x1f) * 0x100];
			u8 g = m_colorxlat[0x4080 / 2 + ((palcolor >> 5) & 0x1f) * 0x100];
			u8 b = m_colorxlat[0x8080 / 2 + ((palcolor >> 10) & 0x1f) * 0x100];
			r = m_gamma_table[r];
			g = m_gamma_table[g];
			b = m_gamma_table[b];
			m_palette->set_pen_color(i, r, g, b);
		}
	}

	//logerror("--- frame ---\n");
	bitmap.fill(m_palette->pen(0), cliprect);
	m_sys24_bitmap.fill(0, cliprect);

	// draw tilemap B as opaque
	for (int layer = 3; layer >= 2; layer--)
		m_tiles->draw(screen, m_sys24_bitmap, cliprect, layer << 1, 0, TILEMAP_DRAW_OPAQUE);

	for (int layer = 1; layer >= 0; layer--)
		m_tiles->draw(screen, m_sys24_bitmap, cliprect, layer << 1, 0, 0);

	copybitmap_trans(bitmap, m_sys24_bitmap, 0, 0, 0, 0, cliprect, 0);

	/* tell the rasterizer we're starting a frame */
	if (m_render_test_mode == true)
		draw_framebuffer(bitmap, cliprect);
	else
		render_polygons(bitmap, cliprect);

	m_sys24_bitmap.fill(0, cliprect);

	for (int layer = 3; layer >= 0; layer--)
		m_tiles->draw(screen, m_sys24_bitmap, cliprect, (layer<<1) | 1, 0, 0);

	copybitmap_trans(bitmap, m_sys24_bitmap, 0, 0, 0, 0, cliprect, 0);

	return 0;
}

// called from machine/model2.cpp trilist command
// TODO: fix forward declaration mess and move this function there instead
void model2_state::tri_list_dump(FILE *dst)
{
	u32 i;
	for (i = 0; i < m_raster->tri_list_index; i++)
	{
		fprintf(dst, "index: %d\n", i);
		fprintf(dst, "v0.x = %f, v0.y = %f, v0.z = %f\n", m_raster->tri_list[i].v[0].x, m_raster->tri_list[i].v[0].y, m_raster->tri_list[i].v[0].pz);
		fprintf(dst, "v1.x = %f, v1.y = %f, v1.z = %f\n", m_raster->tri_list[i].v[1].x, m_raster->tri_list[i].v[1].y, m_raster->tri_list[i].v[1].pz);
		fprintf(dst, "v2.x = %f, v2.y = %f, v2.z = %f\n", m_raster->tri_list[i].v[2].x, m_raster->tri_list[i].v[2].y, m_raster->tri_list[i].v[2].pz);

		fprintf(dst, "tri z: %04x\n", m_raster->tri_list[i].z);
		fprintf(dst, "texheader - 0: %04x\n", m_raster->tri_list[i].texheader[0]);
		fprintf(dst, "texheader - 1: %04x\n", m_raster->tri_list[i].texheader[1]);
		fprintf(dst, "texheader - 2: %04x\n", m_raster->tri_list[i].texheader[2]);
		fprintf(dst, "texheader - 3: %04x\n", m_raster->tri_list[i].texheader[3]);
		fprintf(dst, "luma: %02x\n", m_raster->tri_list[i].luma);
		fprintf(dst, "texlod: %08x\n", m_raster->tri_list[i].texlod);
		fprintf(dst, "vp.sx: %04x\n", m_raster->tri_list[i].viewport[0]);
		fprintf(dst, "vp.sy: %04x\n", m_raster->tri_list[i].viewport[1]);
		fprintf(dst, "vp.ex: %04x\n", m_raster->tri_list[i].viewport[2]);
		fprintf(dst, "vp.ey: %04x\n", m_raster->tri_list[i].viewport[3]);
		fprintf(dst, "vp.swx: %04x\n", m_raster->tri_list[i].center[0]);
		fprintf(dst, "vp.swy: %04x\n", m_raster->tri_list[i].center[1]);
		fprintf(dst, "\n---\n\n");
	}

	fprintf(dst, "min_z = %04x, max_z = %04x\n", m_raster->min_z, m_raster->max_z);

	fclose(dst);
}
