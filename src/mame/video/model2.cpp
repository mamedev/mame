// license:BSD-3-Clause
// copyright-holders:R. Belmont, Olivier Galibert, ElSemi, Angelo Salese
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
    Unfortunately, it seems there's something not quite right, as there are some visible Z-Sort problems in various games.
    Perhaps the linked list of polygons need to be presorted by a unknown factor before rendering.



    Clip Notes and Known Bugs:

    - The Z-Clip happens at z=0. Since dividing by 0 during projection would give bogus values, we scale by a distance of
    1, thus our projection equation turns into vd.x = (d*vs.x) / (d+vs.z) where d = 1, since we know we won't have polygons
    with a z < 0. This, however, poses the problem that for objects originally between 0.0 and 1.0 are not going to be
    scaled properly. We still need to find a solution for this.
    - A small offset need to be added horizontally and vertically to the viewport and center variables for certain games (like
    the original Model 2 games). The coordinate system has been worked out from the 2B specifications, but the older games
    need a slight adjustment.



    Hardware Renderer Notes and Known Bugs:

    - Texturing code could use a real good speed optimization.
    - The U and V coordinates provided by the game are in 13.3 fixed point format.
    - The luma/texel combination algorithm is not known. There are currently some small color glitches here and
    there, and this might be the culprit.
    - The log tables and distance coefficients are used to calculate the number of texels per world unit that need to
    be used to render a texture. Textures can also be provided with smaller levels of details and a LOD bit selector
    in the texture header tells the rasterizer which texture map to use. The rasterizer then can average two texture
    maps to do mip mapping. More information can be found on the 2B manual, on the 'Texturing' and 'Data Format' chapters.
    This is currently unemulated. We always use the texture data from the bigger texture map.
    - The rasterizer supports up to 128x128 'microtex' textures, which are supposed to be higher resolution textures used
    to display more detail when a texture is real close to the viewer. This is currently unemulated.

*********************************************************************************************************************************/
#include "emu.h"
#include "video/segaic24.h"
#include "includes/model2.h"

#define MODEL2_VIDEO_DEBUG 0


#define pz      p[0]
#define pu      p[1]
#define pv      p[2]


/*******************************************
 *
 *  Generic 3D Math Functions
 *
 *******************************************/

INLINE void transform_point( poly_vertex *point, float *matrix )
{
	float tx = (point->x * matrix[0]) + (point->y * matrix[3]) + (point->pz * matrix[6]) + (matrix[9]);
	float ty = (point->x * matrix[1]) + (point->y * matrix[4]) + (point->pz * matrix[7]) + (matrix[10]);
	float tz = (point->x * matrix[2]) + (point->y * matrix[5]) + (point->pz * matrix[8]) + (matrix[11]);

	point->x = tx;
	point->y = ty;
	point->pz = tz;
}

INLINE void transform_vector( poly_vertex *vector, float *matrix )
{
	float tx = (vector->x * matrix[0]) + (vector->y * matrix[3]) + (vector->pz * matrix[6]);
	float ty = (vector->x * matrix[1]) + (vector->y * matrix[4]) + (vector->pz * matrix[7]);
	float tz = (vector->x * matrix[2]) + (vector->y * matrix[5]) + (vector->pz * matrix[8]);

	vector->x = tx;
	vector->y = ty;
	vector->pz = tz;
}

INLINE void normalize_vector( poly_vertex *vector )
{
	float n = sqrt( (vector->x * vector->x) + (vector->y * vector->y) + (vector->pz * vector->pz) );

	if ( n )
	{
		float oon = 1.0f / n;
		vector->x *= oon;
		vector->y *= oon;
		vector->pz *= oon;
	}
}

INLINE float dot_product( poly_vertex *v1, poly_vertex *v2 )
{
	return (v1->x * v2->x) + (v1->y * v2->y) + (v1->pz * v2->pz);
}

INLINE void vector_cross3( poly_vertex *dst, poly_vertex *v0, poly_vertex *v1, poly_vertex *v2 )
{
	poly_vertex p1, p2;

	p1.x = v1->x - v0->x;   p1.y = v1->y - v0->y;   p1.pz = v1->pz - v0->pz;
	p2.x = v2->x - v0->x;   p2.y = v2->y - v0->y;   p2.pz = v2->pz - v0->pz;

	dst->x = (p1.y * p2.pz) - (p1.pz * p2.y);
	dst->y = (p1.pz * p2.x) - (p1.x * p2.pz);
	dst->pz = (p1.x * p2.y) - (p1.y * p2.x);
}

/* 1.8.23 float to 4.12 float converter, courtesy of Aaron Giles */
static UINT16 float_to_zval( float floatval )
{
	INT32 fpint = f2u(floatval);
	INT32 exponent = ((fpint >> 23) & 0xff) - 127;
	UINT32 mantissa = fpint & 0x7fffff;

	/* round the low bits and reduce to 12 */
	mantissa += 0x400;
	if (mantissa > 0x7fffff) { exponent++; mantissa = (mantissa & 0x7fffff) >> 1; }
	mantissa >>= 11;

	/* if negative, clamp to 0 */
	if (fpint < 0)
		return 0x0000;

	/* the rest depends on the exponent */
	/* less than -12 is too small, return 0 */
	if ( exponent < -12 )
		return 0x0000;

	/* between -12 and 0 create a denormal with exponent of 0 */
	else if ( exponent < 0 )
		return (mantissa | 0x1000) >> -exponent;

	/* between 0 and 14 create a FP value with exponent + 1 */
	else if ( exponent < 15 )
		return (( exponent + 1 ) << 12) | mantissa;

	/* above 14 is too large */
	return 0xffff;
}

static INT32 clip_polygon(poly_vertex *v, INT32 num_vertices, plane *cp, poly_vertex *vout)
{
	poly_vertex *cur, *out;
	float   curdot, nextdot, scale;
	INT32   i, curin, nextin, nextvert, outcount;

	outcount = 0;

	cur = v;
	out = vout;

	curdot = dot_product( cur, &cp->normal );
	curin = (curdot >= cp->distance) ? 1 : 0;

	for( i = 0; i < num_vertices; i++ )
	{
		nextvert = (i + 1) % num_vertices;

		/* if the current point is inside the plane, add it */
		if ( curin ) memcpy( &out[outcount++], cur, sizeof( poly_vertex ) );

		nextdot = dot_product( &v[nextvert], &cp->normal );
		nextin = (nextdot >= cp->distance) ? 1 : 0;

		/* Add a clipped vertex if one end of the current edge is inside the plane and the other is outside */
		if ( curin != nextin )
		{
			scale = (cp->distance - curdot) / (nextdot - curdot);

			out[outcount].x = cur->x + ((v[nextvert].x - cur->x) * scale);
			out[outcount].y = cur->y + ((v[nextvert].y - cur->y) * scale);
			out[outcount].pz = cur->pz + ((v[nextvert].pz - cur->pz) * scale);
			out[outcount].pu = (UINT16)((float)cur->pu + (((float)v[nextvert].pu - (float)cur->pu) * scale));
			out[outcount].pv = (UINT16)((float)cur->pv + (((float)v[nextvert].pv - (float)cur->pv) * scale));
			outcount++;
		}

		curdot = nextdot;
		curin = nextin;
		cur++;
	}

	return outcount;
}

/***********************************************************************************************/

/*******************************************
 *
 *  Hardware 3D Rasterizer Internal State
 *
 *******************************************/

#define MAX_TRIANGLES       32768

struct raster_state
{
	UINT32              mode;               /* bit 0 = Test Mode, bit 2 = Switch 60Hz(1)/30Hz(0) operation */
	UINT16 *            texture_rom;        /* Texture ROM pointer */
	INT16               viewport[4];        /* View port (startx,starty,endx,endy) */
	INT16               center[4][2];       /* Centers (eye 0[x,y],1[x,y],2[x,y],3[x,y]) */
	UINT16              center_sel;         /* Selected center */
	UINT32              reverse;            /* Left/Right Reverse */
	float               z_adjust;           /* ZSort Mode */
	float               triangle_z;         /* Current Triangle z value */
	UINT8               master_z_clip;      /* Master Z-Clip value */
	UINT32              cur_command;        /* Current command */
	UINT32              command_buffer[32]; /* Command buffer */
	UINT32              command_index;      /* Command buffer index */
	triangle            tri_list[MAX_TRIANGLES];            /* Triangle list */
	UINT32              tri_list_index;     /* Triangle list index */
	triangle *          tri_sorted_list[0x10000];   /* Sorted Triangle list */
	UINT16              min_z;              /* Minimum sortable Z value */
	UINT16              max_z;              /* Maximum sortable Z value */
	UINT16          texture_ram[0x10000];       /* Texture RAM pointer */
	UINT8               log_ram[0x40000];           /* Log RAM pointer */
};


/*******************************************
 *
 *  Hardware 3D Rasterizer Initialization
 *
 *******************************************/

static void model2_3d_init( running_machine &machine, UINT16 *texture_rom )
{
	model2_state *state = machine.driver_data<model2_state>();

	state->m_raster = auto_alloc_clear( machine, raster_state );

	state->m_raster->texture_rom = texture_rom;
}

/*******************************************
 *
 *  Hardware 3D Rasterizer Z-Clip selection
 *
 *******************************************/

void model2_3d_set_zclip( running_machine &machine, UINT8 clip )
{
	model2_state *state = machine.driver_data<model2_state>();
	state->m_raster->master_z_clip = clip;
}

/*******************************************
 *
 *  Hardware 3D Rasterizer Processing
 *
 *******************************************/

static void model2_3d_process_quad( raster_state *raster, UINT32 attr )
{
	quad_m2     object;
	UINT16      *th, *tp;
	INT32       tho;
	UINT32      cull, i;
	float       zvalue;
	float       min_z, max_z;

	/* extract P0(n-1) */
	object.v[1].x = u2f( raster->command_buffer[2] << 8 );
	object.v[1].y = u2f( raster->command_buffer[3] << 8 );
	object.v[1].pz = u2f( raster->command_buffer[4] << 8 );

	/* extract P1(n-1) */
	object.v[0].x = u2f( raster->command_buffer[5] << 8 );
	object.v[0].y = u2f( raster->command_buffer[6] << 8 );
	object.v[0].pz = u2f( raster->command_buffer[7] << 8 );

	/* extract P0(n) */
	object.v[2].x = u2f( raster->command_buffer[11] << 8 );
	object.v[2].y = u2f( raster->command_buffer[12] << 8 );
	object.v[2].pz = u2f( raster->command_buffer[13] << 8 );

	/* extract P1(n) */
	object.v[3].x = u2f( raster->command_buffer[14] << 8 );
	object.v[3].y = u2f( raster->command_buffer[15] << 8 );
	object.v[3].pz = u2f( raster->command_buffer[16] << 8 );

	/* always calculate the min z and max z value */
	min_z = object.v[0].pz;
	if ( object.v[1].pz < min_z ) min_z = object.v[1].pz;
	if ( object.v[2].pz < min_z ) min_z = object.v[2].pz;
	if ( object.v[3].pz < min_z ) min_z = object.v[3].pz;

	max_z = object.v[0].pz;
	if ( object.v[1].pz > max_z ) max_z = object.v[1].pz;
	if ( object.v[2].pz > max_z ) max_z = object.v[2].pz;
	if ( object.v[3].pz > max_z ) max_z = object.v[3].pz;

	/* read in the texture information */

	/* texture point data */
	if ( raster->command_buffer[0] & 0x800000 )
		tp = &raster->texture_ram[raster->command_buffer[0] & 0xFFFF];
	else
		tp = &raster->texture_rom[raster->command_buffer[0] & 0x7FFFFF];

	object.v[0].pv = *tp++;
	object.v[0].pu = *tp++;
	object.v[1].pv = *tp++;
	object.v[1].pu = *tp++;
	object.v[2].pv = *tp++;
	object.v[2].pu = *tp++;
	object.v[3].pv = *tp++;
	object.v[3].pu = *tp++;

	/* update the address */
	raster->command_buffer[0] += 8;

	/* texture header data */
	if ( raster->command_buffer[1] & 0x800000 )
		th = &raster->texture_ram[raster->command_buffer[1] & 0xFFFF];
	else
		th = &raster->texture_rom[raster->command_buffer[1] & 0x7FFFFF];

	object.texheader[0] = *th++;
	object.texheader[1] = *th++;
	object.texheader[2] = *th++;
	object.texheader[3] = *th++;

	/* extract the texture header offset */
	tho = (attr >> 12) & 0x1F;

	/* adjust for sign */
	if ( tho & 0x10 )
		tho |= -16;

	/* update the address */
	raster->command_buffer[1] += tho * 4;

	/* set the luma value of this quad */
	object.luma = (raster->command_buffer[9] >> 15) & 0xFF;

	/* determine whether we can cull this quad */
	cull = 0;

	/* if doubleside is disabled */
	if ( ((attr >> 17) & 1) == 0 )
	{
		/* if it's the backface, cull it */
		if ( raster->command_buffer[9] & 0x00800000 )
			cull = 1;
	}

	/* if the linktype is 0, then we can also cull it */
	if ( ((attr >> 8) & 3) == 0 )
		cull = 1;

	/* if the minimum z value is bigger than the master z clip value, don't render */
	if ( (INT32)(1.0/min_z) > raster->master_z_clip )
		cull = 1;

	/* if the maximum z value is < 0 then we can safely clip the entire polygon */
	if ( max_z < 0 )
		cull = 1;

	/* set the object's z value */
	zvalue = raster->triangle_z;

	/* see if we need to recompute min/max z */
	if ( (attr >> 10) & 3 )
	{
		if ( (attr >> 10) & 1 ) /* min value */
		{
			zvalue = min_z;
		}
		else if ( (attr >> 10) & 2 ) /* max value */
		{
			zvalue = max_z;
		}

		raster->triangle_z = zvalue;
	}

	if ( cull == 0 )
	{
		INT32       clipped_verts;
		poly_vertex verts[10];
		plane       clip_plane;

		clip_plane.normal.x = 0;
		clip_plane.normal.y = 0;
		clip_plane.normal.pz = 1;
		clip_plane.distance = 0;

		/* do near z clipping */
		clipped_verts = clip_polygon( object.v, 4, &clip_plane, verts);

		if ( clipped_verts > 2 )
		{
			triangle *ztri;

			/* adjust and set the object z-sort value */
			object.z = float_to_zval( zvalue + raster->z_adjust );

			/* get our list read to add the triangles */
			ztri = raster->tri_sorted_list[object.z];

			if ( ztri != NULL )
			{
				while( ztri->next != NULL )
					ztri = (triangle *)ztri->next;
			}

			/* go through the clipped vertex list, adding triangles */
			for( i = 2; i < clipped_verts; i++ )
			{
				triangle    *tri;

				tri = &raster->tri_list[raster->tri_list_index++];

				if ( raster->tri_list_index >= MAX_TRIANGLES )
				{
					fatalerror( "SEGA 3D: Max triangle limit exceeded\n" );
				}

				/* copy the object information */
				tri->z = object.z;
				tri->texheader[0] = object.texheader[0];
				tri->texheader[1] = object.texheader[1];
				tri->texheader[2] = object.texheader[2];
				tri->texheader[3] = object.texheader[3];
				tri->luma = object.luma;

				/* set the viewport */
				tri->viewport[0] = raster->viewport[0];
				tri->viewport[1] = raster->viewport[1];
				tri->viewport[2] = raster->viewport[2];
				tri->viewport[3] = raster->viewport[3];

				/* set the center */
				tri->center[0] = raster->center[raster->center_sel][0];
				tri->center[1] = raster->center[raster->center_sel][1];

				memcpy( &tri->v[0], &verts[0], sizeof( poly_vertex ) );
				memcpy( &tri->v[1], &verts[i-1], sizeof( poly_vertex ) );
				memcpy( &tri->v[2], &verts[i], sizeof( poly_vertex ) );

				/* add to our sorted list */
				tri->next = NULL;

				if ( ztri == NULL )
				{
					raster->tri_sorted_list[object.z] = tri;
				}
				else
				{
					ztri->next = tri;
				}

				ztri = tri;
			}

			/* keep around the min and max z values for this frame */
			if ( object.z < raster->min_z ) raster->min_z = object.z;
			if ( object.z > raster->max_z ) raster->max_z = object.z;
		}
	}

	/* update linking */
	switch( ((attr >> 8) & 3) )
	{
		case 0:
		case 2:
		{
			/* reuse P0(n) and P1(n) */
			for( i = 0; i < 6; i++ )                                        /* P0(n) -> P0(n-1) */
				raster->command_buffer[2+i] = raster->command_buffer[11+i]; /* P1(n) -> P1(n-1) */
		}
		break;

		case 1:
		{
			/* reuse P0(n-1) and P0(n) */
			for( i = 0; i < 3; i++ )
				raster->command_buffer[5+i] = raster->command_buffer[11+i]; /* P0(n) -> P1(n-1) */
		}
		break;

		case 3:
		{
			/* reuse P1(n-1) and P1(n) */
			for( i = 0; i < 3; i++ )
				raster->command_buffer[2+i] = raster->command_buffer[14+i]; /* P1(n) -> P1(n-1) */
		}
		break;
	}
}

static void model2_3d_process_triangle( raster_state *raster, UINT32 attr )
{
	triangle    object;
	UINT16      *th, *tp;
	INT32       tho;
	UINT32      cull, i;
	float       zvalue;
	float       min_z, max_z;

	/* extract P0(n-1) */
	object.v[1].x = u2f( raster->command_buffer[2] << 8 );
	object.v[1].y = u2f( raster->command_buffer[3] << 8 );
	object.v[1].pz = u2f( raster->command_buffer[4] << 8 );

	/* extract P1(n-1) */
	object.v[0].x = u2f( raster->command_buffer[5] << 8 );
	object.v[0].y = u2f( raster->command_buffer[6] << 8 );
	object.v[0].pz = u2f( raster->command_buffer[7] << 8 );

	/* extract P0(n) */
	object.v[2].x = u2f( raster->command_buffer[11] << 8 );
	object.v[2].y = u2f( raster->command_buffer[12] << 8 );
	object.v[2].pz = u2f( raster->command_buffer[13] << 8 );

	/* for triangles, the rope of P1(n) is achieved by P0(n-1) (linktype 3) */
	raster->command_buffer[14] = raster->command_buffer[11];
	raster->command_buffer[15] = raster->command_buffer[12];
	raster->command_buffer[16] = raster->command_buffer[13];

	/* always calculate the min z and max z values */
	min_z = object.v[0].pz;
	if ( object.v[1].pz < min_z ) min_z = object.v[1].pz;
	if ( object.v[2].pz < min_z ) min_z = object.v[2].pz;

	max_z = object.v[0].pz;
	if ( object.v[1].pz > max_z ) max_z = object.v[1].pz;
	if ( object.v[2].pz > max_z ) max_z = object.v[2].pz;

	/* read in the texture information */

	/* texture point data */
	if ( raster->command_buffer[0] & 0x800000 )
		tp = &raster->texture_ram[raster->command_buffer[0] & 0xFFFF];
	else
		tp = &raster->texture_rom[raster->command_buffer[0] & 0x7FFFFF];

	object.v[0].pv = *tp++;
	object.v[0].pu = *tp++;
	object.v[1].pv = *tp++;
	object.v[1].pu = *tp++;
	object.v[2].pv = *tp++;
	object.v[2].pu = *tp++;

	/* update the address */
	raster->command_buffer[0] += 6;

	/* texture header data */
	if ( raster->command_buffer[1] & 0x800000 )
		th = &raster->texture_ram[raster->command_buffer[1] & 0xFFFF];
	else
		th = &raster->texture_rom[raster->command_buffer[1] & 0x7FFFFF];

	object.texheader[0] = *th++;
	object.texheader[1] = *th++;
	object.texheader[2] = *th++;
	object.texheader[3] = *th++;

	/* extract the texture header offset */
	tho = (attr >> 12) & 0x1F;

	/* adjust for sign */
	if ( tho & 0x10 )
		tho |= -16;

	/* update the address */
	raster->command_buffer[1] += tho * 4;

	/* set the luma value of this quad */
	object.luma = (raster->command_buffer[9] >> 15) & 0xFF;

	/* determine whether we can cull this quad */
	cull = 0;

	/* if doubleside is disabled */
	if ( ((attr >> 17) & 1) == 0 )
	{
		/* if it's the backface, cull it */
		if ( raster->command_buffer[9] & 0x00800000 )
			cull = 1;
	}

	/* if the linktype is 0, then we can also cull it */
	if ( ((attr >> 8) & 3) == 0 )
		cull = 1;

	/* if the minimum z value is bigger than the master z clip value, don't render */
	if ( (INT32)(1.0/min_z) > raster->master_z_clip )
		cull = 1;

	/* if the maximum z value is < 0 then we can safely clip the entire polygon */
	if ( max_z < 0 )
		cull = 1;

	/* set the object's z value */
	zvalue = raster->triangle_z;

	/* see if we need to recompute min/max z */
	if ( (attr >> 10) & 3 )
	{
		if ( (attr >> 10) & 1 ) /* min value */
		{
			zvalue = min_z;
		}
		else if ( (attr >> 10) & 2 ) /* max value */
		{
			zvalue = max_z;
		}

		raster->triangle_z = zvalue;
	}

	/* if we're not culling, do z-clip and add to out triangle list */
	if ( cull == 0 )
	{
		INT32       clipped_verts;
		poly_vertex verts[10];
		plane       clip_plane;

		clip_plane.normal.x = 0;
		clip_plane.normal.y = 0;
		clip_plane.normal.pz = 1;
		clip_plane.distance = 0;

		/* do near z clipping */
		clipped_verts = clip_polygon( object.v, 3, &clip_plane, verts);

		if ( clipped_verts > 2 )
		{
			triangle *ztri;

			/* adjust and set the object z-sort value */
			object.z = float_to_zval( zvalue + raster->z_adjust );

			/* get our list read to add the triangles */
			ztri = raster->tri_sorted_list[object.z];

			if ( ztri != NULL )
			{
				while( ztri->next != NULL )
					ztri = (triangle *)ztri->next;
			}

			/* go through the clipped vertex list, adding triangles */
			for( i = 2; i < clipped_verts; i++ )
			{
				triangle    *tri;

				tri = &raster->tri_list[raster->tri_list_index++];

				if ( raster->tri_list_index >= MAX_TRIANGLES )
				{
					fatalerror( "SEGA 3D: Max triangle limit exceeded\n" );
				}

				/* copy the object information */
				tri->z = object.z;
				tri->texheader[0] = object.texheader[0];
				tri->texheader[1] = object.texheader[1];
				tri->texheader[2] = object.texheader[2];
				tri->texheader[3] = object.texheader[3];
				tri->luma = object.luma;

				/* set the viewport */
				tri->viewport[0] = raster->viewport[0];
				tri->viewport[1] = raster->viewport[1];
				tri->viewport[2] = raster->viewport[2];
				tri->viewport[3] = raster->viewport[3];

				/* set the center */
				tri->center[0] = raster->center[raster->center_sel][0];
				tri->center[1] = raster->center[raster->center_sel][1];

				memcpy( &tri->v[0], &verts[0], sizeof( poly_vertex ) );
				memcpy( &tri->v[1], &verts[i-1], sizeof( poly_vertex ) );
				memcpy( &tri->v[2], &verts[i], sizeof( poly_vertex ) );

				/* add to our sorted list */
				tri->next = NULL;

				if ( ztri == NULL )
				{
					raster->tri_sorted_list[object.z] = tri;
				}
				else
				{
					ztri->next = tri;
				}

				ztri = tri;
			}

			/* keep around the min and max z values for this frame */
			if ( object.z < raster->min_z ) raster->min_z = object.z;
			if ( object.z > raster->max_z ) raster->max_z = object.z;
		}
	}

	/* update linking */
	switch( ((attr >> 8) & 3) )
	{
		case 0:
		case 2:
		{
			/* reuse P0(n) and P1(n) */
			for( i = 0; i < 6; i++ )                                        /* P0(n) -> P0(n-1) */
				raster->command_buffer[2+i] = raster->command_buffer[11+i]; /* P1(n) -> P1(n-1) */
		}
		break;

		case 1:
		{
			/* reuse P0(n-1) and P0(n) */
			for( i = 0; i < 3; i++ )
				raster->command_buffer[5+i] = raster->command_buffer[11+i]; /* P0(n) -> P1(n-1) */
		}
		break;

		case 3:
		{
			/* reuse P1(n-1) and P1(n) */
			for( i = 0; i < 3; i++ )
				raster->command_buffer[2+i] = raster->command_buffer[14+i]; /* P1(n) -> P1(n-1) */
		}
		break;
	}
}

/***********************************************************************************************/

/***********************************************************************************************/

void model2_renderer::model2_3d_render(triangle *tri, const rectangle &cliprect)
{
	model2_renderer *poly = m_state.m_poly;
	m2_poly_extra_data& extra = poly->object_data_alloc();
	UINT8 renderer;

	/* select renderer based on attributes (bit15 = checker, bit14 = textured, bit13 = transparent */
	renderer = (tri->texheader[0] >> 13) & 7;

	/* calculate and clip to viewport */
	rectangle vp(tri->viewport[0] - 8, tri->viewport[2] - 8, (384-tri->viewport[3])+90, (384-tri->viewport[1])+90);
	vp &= cliprect;

	extra.state = &m_state;
	extra.lumabase = ((tri->texheader[1] & 0xFF) << 7) + ((tri->luma >> 5) ^ 0x7);
	extra.colorbase = (tri->texheader[3] >> 6) & 0x3FF;

	if (renderer & 2)
	{
		extra.texwidth = 32 << ((tri->texheader[0] >> 0) & 0x7);
		extra.texheight = 32 << ((tri->texheader[0] >> 3) & 0x7);
		extra.texx = 32 * ((tri->texheader[2] >> 0) & 0x1f);
		extra.texy = 32 * (((tri->texheader[2] >> 6) & 0x1f) + ( tri->texheader[2] & 0x20 ));
		/* TODO: Virtua Striker contradicts with this. */
		extra.texmirrorx = 0;//(tri->texheader[0] >> 9) & 1;
		extra.texmirrory = 0;//(tri->texheader[0] >> 8) & 1;
		extra.texsheet = (tri->texheader[2] & 0x1000) ? m_state.m_textureram1 : m_state.m_textureram0;

		tri->v[0].pz = 1.0f / (1.0f + tri->v[0].pz);
		tri->v[0].pu = tri->v[0].pu * tri->v[0].pz * (1.0f / 8.0f);
		tri->v[0].pv = tri->v[0].pv * tri->v[0].pz * (1.0f / 8.0f);
		tri->v[1].pz = 1.0f / (1.0f + tri->v[1].pz);
		tri->v[1].pu = tri->v[1].pu * tri->v[1].pz * (1.0f / 8.0f);
		tri->v[1].pv = tri->v[1].pv * tri->v[1].pz * (1.0f / 8.0f);
		tri->v[2].pz = 1.0f / (1.0f + tri->v[2].pz);
		tri->v[2].pu = tri->v[2].pu * tri->v[2].pz * (1.0f / 8.0f);
		tri->v[2].pv = tri->v[2].pv * tri->v[2].pz * (1.0f / 8.0f);

		// Note : The class model2_renderer has an array of function pointers in it named m_renderfuncs, in theory this simply
		//        needs to be passed into the render_triangle function as such model2_renderer::m_renderfuncs[renderer], but
		//        I was unable to make it work when converting to the new polygon rasterizer interface.
		switch (renderer)
		{
		case 0: render_triangle(vp, render_delegate(FUNC(model2_renderer::model2_3d_render_0), this), 3, tri->v[0], tri->v[1], tri->v[2]); break;
		case 1: render_triangle(vp, render_delegate(FUNC(model2_renderer::model2_3d_render_1), this), 3, tri->v[0], tri->v[1], tri->v[2]); break;
		case 2: render_triangle(vp, render_delegate(FUNC(model2_renderer::model2_3d_render_2), this), 3, tri->v[0], tri->v[1], tri->v[2]); break;
		case 3: render_triangle(vp, render_delegate(FUNC(model2_renderer::model2_3d_render_3), this), 3, tri->v[0], tri->v[1], tri->v[2]); break;
		case 4: render_triangle(vp, render_delegate(FUNC(model2_renderer::model2_3d_render_4), this), 3, tri->v[0], tri->v[1], tri->v[2]); break;
		case 5: render_triangle(vp, render_delegate(FUNC(model2_renderer::model2_3d_render_5), this), 3, tri->v[0], tri->v[1], tri->v[2]); break;
		case 6: render_triangle(vp, render_delegate(FUNC(model2_renderer::model2_3d_render_6), this), 3, tri->v[0], tri->v[1], tri->v[2]); break;
		case 7: render_triangle(vp, render_delegate(FUNC(model2_renderer::model2_3d_render_7), this), 3, tri->v[0], tri->v[1], tri->v[2]); break;
		}
	}
	else
	{
		switch (renderer)
		{
		case 0: render_triangle(vp, render_delegate(FUNC(model2_renderer::model2_3d_render_0), this), 0, tri->v[0], tri->v[1], tri->v[2]); break;
		case 1: render_triangle(vp, render_delegate(FUNC(model2_renderer::model2_3d_render_1), this), 0, tri->v[0], tri->v[1], tri->v[2]); break;
		case 2: render_triangle(vp, render_delegate(FUNC(model2_renderer::model2_3d_render_2), this), 0, tri->v[0], tri->v[1], tri->v[2]); break;
		case 3: render_triangle(vp, render_delegate(FUNC(model2_renderer::model2_3d_render_3), this), 0, tri->v[0], tri->v[1], tri->v[2]); break;
		case 4: render_triangle(vp, render_delegate(FUNC(model2_renderer::model2_3d_render_4), this), 0, tri->v[0], tri->v[1], tri->v[2]); break;
		case 5: render_triangle(vp, render_delegate(FUNC(model2_renderer::model2_3d_render_5), this), 0, tri->v[0], tri->v[1], tri->v[2]); break;
		case 6: render_triangle(vp, render_delegate(FUNC(model2_renderer::model2_3d_render_6), this), 0, tri->v[0], tri->v[1], tri->v[2]); break;
		case 7: render_triangle(vp, render_delegate(FUNC(model2_renderer::model2_3d_render_7), this), 0, tri->v[0], tri->v[1], tri->v[2]); break;
		}
	}
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

/* 3D Rasterizer projection: projects a triangle into screen coordinates */
static void model2_3d_project( triangle *tri )
{
	UINT16  i;

	for( i = 0; i < 3; i++ )
	{
		/* project the vertices */
		tri->v[i].x = -8 + tri->center[0] + (tri->v[i].x / (1.0f+tri->v[i].pz));
		tri->v[i].y = ((384 - tri->center[1])+90) - (tri->v[i].y / (1.0f+tri->v[i].pz));
	}
}

/* 3D Rasterizer frame start: Resets frame variables */
static void model2_3d_frame_start( model2_state *state )
{
	raster_state *raster = state->m_raster;

	/* reset the triangle list index */
	raster->tri_list_index = 0;

	/* reset the sorted z list */
	memset( raster->tri_sorted_list, 0, 0x10000 * sizeof( triangle * ) );

	/* reset the min-max sortable Z values */
	raster->min_z = 0xFFFF;
	raster->max_z = 0;
}

void model2_state::model2_3d_frame_end( bitmap_rgb32 &bitmap, const rectangle &cliprect )
{
	raster_state *raster = m_raster;
	INT32 z;

	/* if we have nothing to render, bail */
	if ( raster->tri_list_index == 0 )
		return;

#if MODEL2_VIDEO_DEBUG
	if (machine().input().code_pressed(KEYCODE_Q))
	{
		UINT32  i;

		FILE *f = fopen( "triangles.txt", "w" );

		if ( f )
		{
			for( i = 0; i < raster->tri_list_index; i++ )
			{
				fprintf( f, "index: %d\n", i );
				fprintf( f, "v0.x = %f, v0.y = %f, v0.z = %f\n", raster->tri_list[i].v[0].x, raster->tri_list[i].v[0].y, raster->tri_list[i].v[0].pz );
				fprintf( f, "v1.x = %f, v1.y = %f, v1.z = %f\n", raster->tri_list[i].v[1].x, raster->tri_list[i].v[1].y, raster->tri_list[i].v[1].pz );
				fprintf( f, "v2.x = %f, v2.y = %f, v2.z = %f\n", raster->tri_list[i].v[2].x, raster->tri_list[i].v[2].y, raster->tri_list[i].v[2].pz );

				fprintf( f, "tri z: %04x\n", raster->tri_list[i].z );
				fprintf( f, "texheader - 0: %04x\n", raster->tri_list[i].texheader[0] );
				fprintf( f, "texheader - 1: %04x\n", raster->tri_list[i].texheader[1] );
				fprintf( f, "texheader - 2: %04x\n", raster->tri_list[i].texheader[2] );
				fprintf( f, "texheader - 3: %04x\n", raster->tri_list[i].texheader[3] );
				fprintf( f, "luma: %02x\n", raster->tri_list[i].luma );
				fprintf( f, "vp.sx: %04x\n", raster->tri_list[i].viewport[0] );
				fprintf( f, "vp.sy: %04x\n", raster->tri_list[i].viewport[1] );
				fprintf( f, "vp.ex: %04x\n", raster->tri_list[i].viewport[2] );
				fprintf( f, "vp.ey: %04x\n", raster->tri_list[i].viewport[3] );
				fprintf( f, "vp.swx: %04x\n", raster->tri_list[i].center[0] );
				fprintf( f, "vp.swy: %04x\n", raster->tri_list[i].center[1] );
				fprintf( f, "\n---\n\n" );
			}

			fprintf( f, "min_z = %04x, max_z = %04x\n", raster->min_z, raster->max_z );

			fclose( f );
		}
	}
#endif

	m_poly->destmap().fill(0x00000000, cliprect);

	/* go through the Z levels, and render each bucket */
	for( z = raster->max_z; z >= raster->min_z; z-- )
	{
		/* see if we have items at this z level */
		if ( raster->tri_sorted_list[z] != NULL )
		{
			/* get a pointer to the first triangle */
			triangle *tri = raster->tri_sorted_list[z];

			/* and loop clipping and rendering each triangle */
			while( tri != NULL )
			{
				/* project and render */
				model2_3d_project( tri );
				m_poly->model2_3d_render(tri, cliprect);

				tri = (triangle *)tri->next;
			}
		}
	}
	m_poly->wait("End of frame");

	copybitmap_trans(bitmap, m_poly->destmap(), 0, 0, 0, 0, cliprect, 0x00000000);
}

/* 3D Rasterizer main data input port */
static void model2_3d_push( raster_state *raster, UINT32 input )
{
	/* see if we have a command in progress */
	if ( raster->cur_command != 0 )
	{
		raster->command_buffer[raster->command_index++] = input;

		switch( raster->cur_command )
		{
			case 0x00:  /* NOP */
			break;

			case 0x01:  /* Polygon Data */
			{
				UINT32  attr;

				/* start by looking if we have the basic input data */
				if ( raster->command_index < 9 )
					return;

				/* get the attributes */
				attr = raster->command_buffer[8];

				/* see if we're done */
				if ( (attr & 3) == 0 )
				{
					raster->cur_command = 0;
					return;
				}

				/* see if it's a quad or a triangle */
				if ( attr & 1 )
				{
					/* it's a quad, wait for the rest of the points */
					if ( raster->command_index < 17 )
						return;

					/* we have a full quad info, fill up our quad structure */
					model2_3d_process_quad( raster, attr );

					/* back up and wait for more data */
					raster->command_index = 8;
				}
				else
				{
					/* it's a triangle, wait for the rest of the point */
					if ( raster->command_index < 14 )
						return;

					/* we have a full quad info, fill up our quad structure */
					model2_3d_process_triangle( raster, attr );

					/* back up and wait for more data */
					raster->command_index = 8;
				}
			}
			break;

			case 0x03:  /* Window Data */
			{
				UINT32  i;

				/* make sure we have all the data */
				if ( raster->command_index < 6 )
					return;

				/* coordinates are 12 bit signed */

				/* extract the viewport start x */
				raster->viewport[0] = (raster->command_buffer[0] >> 12) & 0xFFF;

				if ( raster->viewport[0] & 0x800 )
					raster->viewport[0] = -( 0x800 - (raster->viewport[0] & 0x7FF) );

				/* extract the viewport start y */
				raster->viewport[1] = raster->command_buffer[0] & 0xFFF;

				if ( raster->viewport[1] & 0x800 )
					raster->viewport[1] = -( 0x800 - (raster->viewport[1] & 0x7FF) );

				/* extract the viewport end x */
				raster->viewport[2] = (raster->command_buffer[1] >> 12) & 0xFFF;

				if ( raster->viewport[2] & 0x800 )
					raster->viewport[2] = -( 0x800 - (raster->viewport[2] & 0x7FF) );

				/* extract the viewport end y */
				raster->viewport[3] = raster->command_buffer[1] & 0xFFF;

				if ( raster->viewport[3] & 0x800 )
					raster->viewport[3] = -( 0x800 - (raster->viewport[3] & 0x7FF) );

				/* extract the centers */
				for( i = 0; i < 4; i++ )
				{
					/* center x */
					raster->center[i][0] = (raster->command_buffer[2+i] >> 12) & 0xFFF;

					if ( raster->center[i][0] & 0x800 )
						raster->center[i][0] = -( 0x800 - (raster->center[i][0] & 0x7FF) );

					/* center y */
					raster->center[i][1] = raster->command_buffer[2+i] & 0xFFF;

					if ( raster->center[i][1] & 0x800 )
						raster->center[i][1] = -( 0x800 - (raster->center[i][1] & 0x7FF) );
				}

				/* done with this command */
				raster->cur_command = 0;
			}
			break;

			case 0x04:  /* Texture/Log Data write */
			{
				/* make sure we have enough data */
				if ( raster->command_index < 2 )
					return;

				/* see if the count is non-zero */
				if ( raster->command_buffer[1] > 0 )
				{
					/* see if we have data available */
					if ( raster->command_index >= 3 )
					{
						/* get the address */
						UINT32  address = raster->command_buffer[0];

						/* do the write */
						if ( address & 0x800000 )
							raster->texture_ram[address&0xFFFF] = raster->command_buffer[2];
						else
							raster->log_ram[address&0xFFFF] = raster->command_buffer[2];

						/* increment the address and decrease the count */
						raster->command_buffer[0]++;
						raster->command_buffer[1]--;

						/* decrease the index, so we keep placing data in the same slot */
						raster->command_index--;
					}
				}

				/* see if we're done with this command */
				if ( raster->command_buffer[1] == 0 )
					raster->cur_command = 0;
			}
			break;

			case 0x08:  /* ZSort mode */
			{
				/* save the zsort mode value */
				raster->z_adjust = u2f( raster->command_buffer[0] << 8 );

				/* done with this command */
				raster->cur_command = 0;
			}
			break;

			default:
			{
				fatalerror( "SEGA 3D: Unknown rasterizer command %08x\n", raster->cur_command );
			}
		}
	}
	else
	{
		/* new command */
		raster->cur_command = input & 0x0F;
		raster->command_index = 0;

		/* see if it's object data */
		if ( raster->cur_command == 1 )
		{
			/* extract reverse bit */
			raster->reverse = (input >> 4) & 1;

			/* extract center select */
			raster->center_sel = ( input >> 6 ) & 3;

			/* reset the triangle z value */
			raster->triangle_z = 0;
		}
	}
}

/***********************************************************************************************/



/*******************************************
 *
 *  Geometry Engine Internal State
 *
 *******************************************/

struct geo_state
{
	raster_state *          raster;
	UINT32              mode;                   /* bit 0 = Enable Specular, bit 1 = Calculate Normals */
	UINT32 *            polygon_rom;            /* Polygon ROM pointer */
	float               matrix[12];             /* Current Transformation Matrix */
	poly_vertex         focus;                  /* Focus (x,y) */
	poly_vertex         light;                  /* Light Vector */
	float               lod;                    /* LOD */
	float               coef_table[32];         /* Distane Coefficient table */
	texture_parameter   texture_parameters[32]; /* Texture parameters */
	UINT32          polygon_ram0[0x8000];           /* Fast Polygon RAM pointer */
	UINT32          polygon_ram1[0x8000];           /* Slow Polygon RAM pointer */
	model2_state	*state;
};


/*******************************************
 *
 *  Geometry Engine Initialization
 *
 *******************************************/

static void geo_init( running_machine &machine, UINT32 *polygon_rom )
{
	model2_state *state = machine.driver_data<model2_state>();
	state->m_geo = auto_alloc_clear(machine, geo_state);
	state->m_geo->state = state;

	state->m_geo->raster = state->m_raster;
	state->m_geo->polygon_rom = polygon_rom;
}

/*******************************************
 *
 *  Geometry Engine Polygon Parsers
 *
 *******************************************/

/* Parse Polygons: Normals Present, No Specular case */
static void geo_parse_np_ns( geo_state *geo, UINT32 *input, UINT32 count )
{
	raster_state *raster = geo->raster;
	poly_vertex point, normal;
	UINT32  attr, i;

	/* read the 1st point */
	point.x = u2f( *input++ );
	point.y = u2f( *input++ );
	point.pz = u2f( *input++ );

	/* transform with the current matrix */
	transform_point( &point, geo->matrix );

	/* apply focus */
	point.x *= geo->focus.x;
	point.y *= geo->focus.y;

	/* push it to the 3d rasterizer */
	model2_3d_push( raster, f2u(point.x) >> 8 );
	model2_3d_push( raster, f2u(point.y) >> 8 );
	model2_3d_push( raster, f2u(point.pz) >> 8 );

	/* read the 2nd point */
	point.x = u2f( *input++ );
	point.y = u2f( *input++ );
	point.pz = u2f( *input++ );

	/* transform with the current matrix */
	transform_point( &point, geo->matrix );

	/* apply focus */
	point.x *= geo->focus.x;
	point.y *= geo->focus.y;

	/* push it to the 3d rasterizer */
	model2_3d_push( raster, f2u(point.x) >> 8 );
	model2_3d_push( raster, f2u(point.y) >> 8 );
	model2_3d_push( raster, f2u(point.pz) >> 8 );

	/* loop through the following links */
	for( i = 0; i < count; i++ )
	{
		/* read in the attributes */
		attr = *input++;

		/* push to the 3d rasterizer */
		model2_3d_push( raster, attr & 0x0003FFFF );

		/* read in the normal */
		normal.x = u2f(*input++);
		normal.y = u2f(*input++);
		normal.pz = u2f(*input++);

		/* transform with the current matrix */
		transform_vector( &normal, geo->matrix );

		if ( (attr & 3) != 0 ) /* quad or triangle */
		{
			float               dotl, dotp, luminance, distance;
			float               coef, face;
			INT32               luma;
			texture_parameter * texparam;

			/* read in the next point */
			point.x = u2f( *input++ );
			point.y = u2f( *input++ );
			point.pz = u2f( *input++ );

			/* transform with the current matrix */
			transform_point( &point, geo->matrix );

			/* calculate the dot product of the normal and the light vector */
			dotl = dot_product( &normal, &geo->light );

			/* calculate the dot product of the normal and the point */
			dotp = dot_product( &normal, &point );

			/* apply focus */
			point.x *= geo->focus.x;
			point.y *= geo->focus.y;

			/* determine whether this is the front or the back of the polygon */
			face = 0x100; /* rear */
			if ( dotp >= 0 ) face = 0; /* front */

			/* get the texture parameters */
			texparam = &geo->texture_parameters[(attr>>18) & 0x1f];

			/* calculate luminance */
			if ( (dotl * dotp) < 0 ) luminance = 0;
			else luminance = fabs( dotl );

			luminance = (luminance * texparam->diffuse) + texparam->ambient;
			luma = (INT32)luminance;

			if ( luma > 255 ) luma = 255;
			if ( luma < 0 ) luma = 0;

			/* add the face bit to the luma */
			luma += face;

			/* extract distance coefficient */
			coef = geo->coef_table[attr>>27];

			/* calculate texture level of detail */
			distance = coef * fabs( dotp ) * geo->lod;

			/* push to the 3d rasterizer */
			model2_3d_push( raster, luma << 15 );
			model2_3d_push( raster, f2u(distance) >> 8 );
			model2_3d_push( raster, f2u(point.x) >> 8 );
			model2_3d_push( raster, f2u(point.y) >> 8 );
			model2_3d_push( raster, f2u(point.pz) >> 8 );

			/* if it's a quad, push one more point */
			if ( attr & 1 )
			{
				/* read in the next point */
				point.x = u2f( *input++ );
				point.y = u2f( *input++ );
				point.pz = u2f( *input++ );

				/* transform with the current matrix */
				transform_point( &point, geo->matrix );

				/* apply focus */
				point.x *= geo->focus.x;
				point.y *= geo->focus.y;

				/* push to the 3d rasterizer */
				model2_3d_push( raster, f2u(point.x) >> 8 );
				model2_3d_push( raster, f2u(point.y) >> 8 );
				model2_3d_push( raster, f2u(point.pz) >> 8 );
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
	model2_3d_push( raster, 0 );
}

/* Parse Polygons: Normals Present, Specular case */
static void geo_parse_np_s( geo_state *geo, UINT32 *input, UINT32 count )
{
	raster_state *raster = geo->raster;
	poly_vertex point, normal;
	UINT32  attr, i;

	/* read the 1st point */
	point.x = u2f( *input++ );
	point.y = u2f( *input++ );
	point.pz = u2f( *input++ );

	/* transform with the current matrix */
	transform_point( &point, geo->matrix );

	/* apply focus */
	point.x *= geo->focus.x;
	point.y *= geo->focus.y;

	/* push it to the 3d rasterizer */
	model2_3d_push( raster, f2u(point.x) >> 8 );
	model2_3d_push( raster, f2u(point.y) >> 8 );
	model2_3d_push( raster, f2u(point.pz) >> 8 );

	/* read the 2nd point */
	point.x = u2f( *input++ );
	point.y = u2f( *input++ );
	point.pz = u2f( *input++ );

	/* transform with the current matrix */
	transform_point( &point, geo->matrix );

	/* apply focus */
	point.x *= geo->focus.x;
	point.y *= geo->focus.y;

	/* push it to the 3d rasterizer */
	model2_3d_push( raster, f2u(point.x) >> 8 );
	model2_3d_push( raster, f2u(point.y) >> 8 );
	model2_3d_push( raster, f2u(point.pz) >> 8 );

	/* loop through the following links */
	for( i = 0; i < count; i++ )
	{
		/* read in the attributes */
		attr = *input++;

		/* push to the 3d rasterizer */
		model2_3d_push( raster, attr & 0x0003FFFF );

		/* read in the normal */
		normal.x = u2f(*input++);
		normal.y = u2f(*input++);
		normal.pz = u2f(*input++);

		/* transform with the current matrix */
		transform_vector( &normal, geo->matrix );

		if ( (attr & 3) != 0 ) /* quad or triangle */
		{
			float               dotl, dotp, luminance, distance, specular;
			float               coef, face;
			INT32               luma;
			texture_parameter * texparam;

			/* read in the next point */
			point.x = u2f( *input++ );
			point.y = u2f( *input++ );
			point.pz = u2f( *input++ );

			/* transform with the current matrix */
			transform_point( &point, geo->matrix );

			/* calculate the dot product of the normal and the light vector */
			dotl = dot_product( &normal, &geo->light );

			/* calculate the dot product of the normal and the point */
			dotp = dot_product( &normal, &point );

			/* apply focus */
			point.x *= geo->focus.x;
			point.y *= geo->focus.y;

			/* determine whether this is the front or the back of the polygon */
			face = 0x100; /* rear */
			if ( dotp >= 0 ) face = 0; /* front */

			/* get the texture parameters */
			texparam = &geo->texture_parameters[(attr>>18) & 0x1f];

			/* calculate luminance and specular */
			if ( (dotl * dotp) < 0 ) luminance = 0;
			else luminance = fabs( dotl );

			specular = ((2*dotl) * normal.pz) - geo->light.pz;
			if ( specular < 0 ) specular = 0;
			if ( texparam->specular_control == 0 ) specular = 0;
			if ( (texparam->specular_control >> 1) != 0 ) specular *= specular;
			if ( (texparam->specular_control >> 2) != 0 ) specular *= specular;
			if ( ((texparam->specular_control+1) >> 3) != 0 ) specular *= specular;

			specular *= texparam->specular_scale;

			luminance = (luminance * texparam->diffuse) + texparam->ambient + specular;
			luma = (INT32)luminance;

			if ( luma > 255 ) luma = 255;
			if ( luma < 0 ) luma = 0;

			/* add the face bit to the luma */
			luma += face;

			/* extract distance coefficient */
			coef = geo->coef_table[attr>>27];

			/* calculate texture level of detail */
			distance = coef * fabs( dotp ) * geo->lod;

			/* push to the 3d rasterizer */
			model2_3d_push( raster, luma << 15 );
			model2_3d_push( raster, f2u(distance) >> 8 );
			model2_3d_push( raster, f2u(point.x) >> 8 );
			model2_3d_push( raster, f2u(point.y) >> 8 );
			model2_3d_push( raster, f2u(point.pz) >> 8 );

			/* if it's a quad, push one more point */
			if ( attr & 1 )
			{
				/* read in the next point */
				point.x = u2f( *input++ );
				point.y = u2f( *input++ );
				point.pz = u2f( *input++ );

				/* transform with the current matrix */
				transform_point( &point, geo->matrix );

				/* apply focus */
				point.x *= geo->focus.x;
				point.y *= geo->focus.y;

				/* push to the 3d rasterizer */
				model2_3d_push( raster, f2u(point.x) >> 8 );
				model2_3d_push( raster, f2u(point.y) >> 8 );
				model2_3d_push( raster, f2u(point.pz) >> 8 );
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
	model2_3d_push( raster, 0 );
}

/* Parse Polygons: No Normals, No Specular case */
static void geo_parse_nn_ns( geo_state *geo, UINT32 *input, UINT32 count )
{
	raster_state *raster = geo->raster;
	poly_vertex point, normal, p0, p1, p2, p3;
	UINT32  attr, i;

	/* read the 1st point */
	point.x = u2f( *input++ );
	point.y = u2f( *input++ );
	point.pz = u2f( *input++ );

	/* transform with the current matrix */
	transform_point( &point, geo->matrix );

	/* save for normal calculation */
	p0.x = point.x; p0.y = point.y; p0.pz = point.pz;

	/* apply focus */
	point.x *= geo->focus.x;
	point.y *= geo->focus.y;

	/* push it to the 3d rasterizer */
	model2_3d_push( raster, f2u(point.x) >> 8 );
	model2_3d_push( raster, f2u(point.y) >> 8 );
	model2_3d_push( raster, f2u(point.pz) >> 8 );

	/* read the 2nd point */
	point.x = u2f( *input++ );
	point.y = u2f( *input++ );
	point.pz = u2f( *input++ );

	/* transform with the current matrix */
	transform_point( &point, geo->matrix );

	/* save for normal calculation */
	p1.x = point.x; p1.y = point.y; p1.pz = point.pz;

	/* apply focus */
	point.x *= geo->focus.x;
	point.y *= geo->focus.y;

	/* push it to the 3d rasterizer */
	model2_3d_push( raster, f2u(point.x) >> 8 );
	model2_3d_push( raster, f2u(point.y) >> 8 );
	model2_3d_push( raster, f2u(point.pz) >> 8 );

	/* skip 4 */
	input += 4;

	/* loop through the following links */
	for( i = 0; i < count; i++ )
	{
		/* read in the attributes */
		attr = *input++;

		/* push to the 3d rasterizer */
		model2_3d_push( raster, attr & 0x0003FFFF );

		if ( (attr & 3) != 0 ) /* quad or triangle */
		{
			float               dotl, dotp, luminance, distance;
			float               coef, face;
			INT32               luma;
			texture_parameter * texparam;

			/* read in the next point */
			point.x = u2f( *input++ );
			point.y = u2f( *input++ );
			point.pz = u2f( *input++ );

			/* transform with the current matrix */
			transform_point( &point, geo->matrix );

			/* save for normal calculation */
			p2.x = point.x; p2.y = point.y; p2.pz = point.pz;

			/* compute the normal */
			vector_cross3( &normal, &p0, &p1, &p2 );

			/* normalize it */
			normalize_vector( &normal );

			/* calculate the dot product of the normal and the light vector */
			dotl = dot_product( &normal, &geo->light );

			/* calculate the dot product of the normal and the point */
			dotp = dot_product( &normal, &point );

			/* apply focus */
			point.x *= geo->focus.x;
			point.y *= geo->focus.y;

			/* determine whether this is the front or the back of the polygon */
			face = 0x100; /* rear */
			if ( dotp >= 0 ) face = 0; /* front */

			/* get the texture parameters */
			texparam = &geo->texture_parameters[(attr>>18) & 0x1f];

			/* calculate luminance */
			if ( (dotl * dotp) < 0 ) luminance = 0;
			else luminance = fabs( dotl );

			luminance = (luminance * texparam->diffuse) + texparam->ambient;
			luma = (INT32)luminance;

			if ( luma > 255 ) luma = 255;
			if ( luma < 0 ) luma = 0;

			/* add the face bit to the luma */
			luma += face;

			/* extract distance coefficient */
			coef = geo->coef_table[attr>>27];

			/* calculate texture level of detail */
			distance = coef * fabs( dotp ) * geo->lod;

			/* push to the 3d rasterizer */
			model2_3d_push( raster, luma << 15 );
			model2_3d_push( raster, f2u(distance) >> 8 );
			model2_3d_push( raster, f2u(point.x) >> 8 );
			model2_3d_push( raster, f2u(point.y) >> 8 );
			model2_3d_push( raster, f2u(point.pz) >> 8 );

			/* if it's a quad, push one more point */
			if ( attr & 1 )
			{
				/* read in the next point */
				point.x = u2f( *input++ );
				point.y = u2f( *input++ );
				point.pz = u2f( *input++ );

				/* transform with the current matrix */
				transform_point( &point, geo->matrix );

				/* save for normal calculation */
				p3.x = point.x; p3.y = point.y; p3.pz = point.pz;

				/* apply focus */
				point.x *= geo->focus.x;
				point.y *= geo->focus.y;

				/* push to the 3d rasterizer */
				model2_3d_push( raster, f2u(point.x) >> 8 );
				model2_3d_push( raster, f2u(point.y) >> 8 );
				model2_3d_push( raster, f2u(point.pz) >> 8 );
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
		switch( (attr>>8) & 3 )
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
	model2_3d_push( raster, 0 );
}

/* Parse Polygons: No Normals, Specular case */
static void geo_parse_nn_s( geo_state *geo, UINT32 *input, UINT32 count )
{
	raster_state *raster = geo->raster;
	poly_vertex point, normal, p0, p1, p2, p3;
	UINT32  attr, i;

	/* read the 1st point */
	point.x = u2f( *input++ );
	point.y = u2f( *input++ );
	point.pz = u2f( *input++ );

	/* transform with the current matrix */
	transform_point( &point, geo->matrix );

	/* save for normal calculation */
	p0.x = point.x; p0.y = point.y; p0.pz = point.pz;

	/* apply focus */
	point.x *= geo->focus.x;
	point.y *= geo->focus.y;

	/* push it to the 3d rasterizer */
	model2_3d_push( raster, f2u(point.x) >> 8 );
	model2_3d_push( raster, f2u(point.y) >> 8 );
	model2_3d_push( raster, f2u(point.pz) >> 8 );

	/* read the 2nd point */
	point.x = u2f( *input++ );
	point.y = u2f( *input++ );
	point.pz = u2f( *input++ );

	/* transform with the current matrix */
	transform_point( &point, geo->matrix );

	/* save for normal calculation */
	p1.x = point.x; p1.y = point.y; p1.pz = point.pz;

	/* apply focus */
	point.x *= geo->focus.x;
	point.y *= geo->focus.y;

	/* push it to the 3d rasterizer */
	model2_3d_push( raster, f2u(point.x) >> 8 );
	model2_3d_push( raster, f2u(point.y) >> 8 );
	model2_3d_push( raster, f2u(point.pz) >> 8 );

	/* skip 4 */
	input += 4;

	/* loop through the following links */
	for( i = 0; i < count; i++ )
	{
		/* read in the attributes */
		attr = *input++;

		/* push to the 3d rasterizer */
		model2_3d_push( raster, attr & 0x0003FFFF );

		if ( (attr & 3) != 0 ) /* quad or triangle */
		{
			float               dotl, dotp, luminance, distance, specular;
			float               coef, face;
			INT32               luma;
			texture_parameter * texparam;

			/* read in the next point */
			point.x = u2f( *input++ );
			point.y = u2f( *input++ );
			point.pz = u2f( *input++ );

			/* transform with the current matrix */
			transform_point( &point, geo->matrix );

			/* save for normal calculation */
			p2.x = point.x; p2.y = point.y; p2.pz = point.pz;

			/* compute the normal */
			vector_cross3( &normal, &p0, &p1, &p2 );

			/* normalize it */
			normalize_vector( &normal );

			/* calculate the dot product of the normal and the light vector */
			dotl = dot_product( &normal, &geo->light );

			/* calculate the dot product of the normal and the point */
			dotp = dot_product( &normal, &point );

			/* apply focus */
			point.x *= geo->focus.x;
			point.y *= geo->focus.y;

			/* determine whether this is the front or the back of the polygon */
			face = 0x100; /* rear */
			if ( dotp >= 0 ) face = 0; /* front */

			/* get the texture parameters */
			texparam = &geo->texture_parameters[(attr>>18) & 0x1f];

			/* calculate luminance and specular */
			if ( (dotl * dotp) < 0 ) luminance = 0;
			else luminance = fabs( dotl );

			specular = ((2*dotl) * normal.pz) - geo->light.pz;
			if ( specular < 0 ) specular = 0;
			if ( texparam->specular_control == 0 ) specular = 0;
			if ( (texparam->specular_control >> 1) != 0 ) specular *= specular;
			if ( (texparam->specular_control >> 2) != 0 ) specular *= specular;
			if ( ((texparam->specular_control+1) >> 3) != 0 ) specular *= specular;

			specular *= texparam->specular_scale;

			luminance = (luminance * texparam->diffuse) + texparam->ambient + specular;
			luma = (INT32)luminance;

			if ( luma > 255 ) luma = 255;
			if ( luma < 0 ) luma = 0;

			/* add the face bit to the luma */
			luma += face;

			/* extract distance coefficient */
			coef = geo->coef_table[attr>>27];

			/* calculate texture level of detail */
			distance = coef * fabs( dotp ) * geo->lod;

			/* push to the 3d rasterizer */
			model2_3d_push( raster, luma << 15 );
			model2_3d_push( raster, f2u(distance) >> 8 );
			model2_3d_push( raster, f2u(point.x) >> 8 );
			model2_3d_push( raster, f2u(point.y) >> 8 );
			model2_3d_push( raster, f2u(point.pz) >> 8 );

			/* if it's a quad, push one more point */
			if ( attr & 1 )
			{
				/* read in the next point */
				point.x = u2f( *input++ );
				point.y = u2f( *input++ );
				point.pz = u2f( *input++ );

				/* transform with the current matrix */
				transform_point( &point, geo->matrix );

				/* save for normal calculation */
				p3.x = point.x; p3.y = point.y; p3.pz = point.pz;

				/* apply focus */
				point.x *= geo->focus.x;
				point.y *= geo->focus.y;

				/* push to the 3d rasterizer */
				model2_3d_push( raster, f2u(point.x) >> 8 );
				model2_3d_push( raster, f2u(point.y) >> 8 );
				model2_3d_push( raster, f2u(point.pz) >> 8 );
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
		switch( (attr>>8) & 3 )
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
	model2_3d_push( raster, 0 );
}

/*******************************************
 *
 *  Geometry Engine Commands
 *
 *******************************************/

/* Command 00: NOP */
static UINT32 * geo_nop( geo_state *geo, UINT32 opcode, UINT32 *input )
{
	raster_state *raster = geo->raster;

	/* push the opcode to the 3d rasterizer */
	model2_3d_push( raster, opcode >> 23 );

	return input;
}

/* Command 01: Object Data */
static UINT32 * geo_object_data( geo_state *geo, UINT32 opcode, UINT32 *input )
{
	raster_state *raster = geo->raster;
	UINT32  tpa = *input++;     /* Texture Point Address */
	UINT32  tha = *input++;     /* Texture Header Address */
	UINT32  oba = *input++;     /* Object Address */
	UINT32  obc = *input++;     /* Object Count */

	UINT32 *obp;                /* Object Pointer */

	/* push the initial set of data to the 3d rasterizer */
	model2_3d_push( raster, opcode >> 23 );
	model2_3d_push( raster, tpa );
	model2_3d_push( raster, tha );

	/* select where we're reading polygon information from */
	if ( oba & 0x01000000 )
	{
		/* Fast polygon RAM */
		obp = &geo->polygon_ram0[oba & 0x7FFF];
	}
	else if ( oba & 0x00800000 )
	{
		/* Polygon ROM */
		obp = &geo->polygon_rom[oba & 0x7FFFFF];
	}
	else
	{
		/* Slow Polygon RAM */
		obp = &geo->polygon_ram1[oba & 0x7FFF];
	}

	switch( geo->mode & 3 )
	{
		/* Normals present, No Specular */
		case 0: geo_parse_np_ns( geo, obp, obc ); break;

		/* Normals present, Specular */
		case 1: geo_parse_np_s( geo, obp, obc ); break;

		/* No Normals present, No Specular */
		case 2: geo_parse_nn_ns( geo, obp, obc ); break;

		/* No Normals present, Specular */
		case 3: geo_parse_nn_s( geo, obp, obc ); break;
	}

	/* move by 4 parameters */
	return input;
}

/* Command 02: Direct Data */
static UINT32 * geo_direct_data( geo_state *geo, UINT32 opcode, UINT32 *input )
{
	raster_state *raster = geo->raster;
	UINT32  tpa = *input++;     /* Texture Point Address */
	UINT32  tha = *input++;     /* Texture Header Address */
	UINT32  attr;

	/* push the initial set of data to the 3d rasterizer */
	model2_3d_push( raster, (opcode >> 23) - 1 );
	model2_3d_push( raster, tpa );
	model2_3d_push( raster, tha );

	/* push the initial points */
	model2_3d_push( raster, (*input++) >> 8 ); /* x */
	model2_3d_push( raster, (*input++) >> 8 ); /* y */
	model2_3d_push( raster, (*input++) >> 8 ); /* z */

	model2_3d_push( raster, (*input++) >> 8 ); /* x */
	model2_3d_push( raster, (*input++) >> 8 ); /* y */
	model2_3d_push( raster, (*input++) >> 8 ); /* z */

	do
	{
		/* read in the attributes */
		attr = *input++;

		if ( (attr & 3) == 0 )
			break;

		/* push attributes */
		model2_3d_push( raster, attr & 0x00FFFFFF );

		/* push luma */
		model2_3d_push( raster, (*input++) >> 8 );

		/* push distance */
		model2_3d_push( raster, (*input++) >> 8 );

		/* push the next point */
		model2_3d_push( raster, (*input++) >> 8 ); /* x */
		model2_3d_push( raster, (*input++) >> 8 ); /* y */
		model2_3d_push( raster, (*input++) >> 8 ); /* z */

		/* if it's a quad, output another point */
		if ( attr & 1 )
		{
			model2_3d_push( raster, (*input++) >> 8 ); /* x */
			model2_3d_push( raster, (*input++) >> 8 ); /* y */
			model2_3d_push( raster, (*input++) >> 8 ); /* z */
		}
	} while( 1 );

	/* we're done */
	model2_3d_push( raster, 0 );

	return input;
}

/* Command 03: Window Data */
static UINT32 * geo_window_data( geo_state *geo, UINT32 opcode, UINT32 *input )
{
	raster_state *raster = geo->raster;
	UINT32  x, y, i;

	/* start by pushing the opcode */
	model2_3d_push( raster, opcode >> 23 );

	/*
	    we're going to move 6 coordinates to the 3d rasterizer:
	    - starting coordinate
	    - completion coordinate
	    - vanishing point 0 (eye mode 0)
	    - vanishing point 1 (eye mode 1)
	    - vanishing point 2 (eye mode 2)
	    - vanishing point 3 (eye mode 3)
	*/

	for( i = 0; i < 6; i++ )
	{
		/* read in the coordinate */
		y = *input++;

		/* convert to the 3d rasterizer format (00XXXYYY) */
		x = ( y & 0x0FFF0000 ) >> 4 ;
		y &= 0xFFF;

		/* push it */
		model2_3d_push( raster, x | y );
	}

	return input;
}

/* Command 04: Texture Data Write */
static UINT32 * geo_texture_data( geo_state *geo, UINT32 opcode, UINT32 *input )
{
	raster_state *raster = geo->raster;
	UINT32  i, count;

	/* start by pushing the opcode */
	model2_3d_push( raster, opcode >> 23 );

	/* push the starting address/dsp id */
	model2_3d_push( raster, *input++ );

	/* get the count */
	count = *input++;

	/* push the count */
	model2_3d_push( raster, count );

	/* loop and send the data */
	for( i = 0; i < count; i++ )
		model2_3d_push( raster, *input++ );

	return input;
}

/* Command 05: Polygon Data */
static UINT32 * geo_polygon_data( geo_state *geo, UINT32 opcode, UINT32 *input )
{
	UINT32  address, count, i;
	UINT32 *p;

	(void)opcode;

	/* read in the address */
	address = *input++;

	/* prepare the pointer */
	if ( address & 0x01000000 )
	{
		/* Fast polygon RAM */
		p = &geo->polygon_ram0[address & 0x7FFF];
	}
	else
	{
		/* Slow Polygon RAM */
		p = &geo->polygon_ram1[address & 0x7FFF];
	}

	/* read the count */
	count = *input++;

	/* move the data */
	for( i = 0; i < count; i++ )
		*p++ = *input++;

	return input;
}

/* Command 06: Texture Parameters */
static UINT32 * geo_texture_parameters( geo_state *geo, UINT32 opcode, UINT32 *input )
{
	UINT32  index, count, i, param;

	(void)opcode;

	/* read in the index */
	index = *input++;

	/* read in the conut */
	count = *input++;

	for( i = 0; i < count; i++ )
	{
		/* read in the texture parameters */
		param = *input++;

		geo->texture_parameters[index].diffuse = (float)( param & 0xFF );
		geo->texture_parameters[index].ambient = (float)( (param >> 8) & 0xFF );
		geo->texture_parameters[index].specular_control = (param >> 24) & 0xFF;
		geo->texture_parameters[index].specular_scale = (float)( (param >> 16) & 0xFF );

		/* read in the distance coefficient */
		geo->coef_table[index] = u2f(*input++);

		index = (index + 1) & 0x1F;
	}

	return input;
}

/* Command 07: Geo Mode */
static UINT32 * geo_mode( geo_state *geo, UINT32 opcode, UINT32 *input )
{
	(void)opcode;

	/* read in the mode */
	geo->mode = *input++;

	return input;
}

/* Command 08: ZSort Mode */
static UINT32 * geo_zsort_mode( geo_state *geo, UINT32 opcode, UINT32 *input )
{
	raster_state *raster = geo->raster;

	/* push the opcode */
	model2_3d_push( raster, opcode >> 23 );

	/* push the mode */
	model2_3d_push( raster, (*input++) >> 8 );

	return input;
}

/* Command 09: Focal Distance */
static UINT32 * geo_focal_distance( geo_state *geo, UINT32 opcode, UINT32 *input )
{
	(void)opcode;

	/* read the x focus value */
	geo->focus.x = u2f( *input++ );

	/* read the y focus value */
	geo->focus.y = u2f( *input++ );

	return input;
}

/* Command 0A: Light Source Vector Write */
static UINT32 * geo_light_source( geo_state *geo, UINT32 opcode, UINT32 *input )
{
	(void)opcode;

	/* read the x light value */
	geo->light.x = u2f( *input++ );

	/* read the y light value */
	geo->light.y = u2f( *input++ );

	/* read the z light value */
	geo->light.pz = u2f( *input++ );

	return input;
}

/* Command 0B: Transformation Matrix Write */
static UINT32 * geo_matrix_write( geo_state *geo, UINT32 opcode, UINT32 *input )
{
	UINT32  i;

	(void)opcode;

	/* read in the transformation matrix */
	for( i = 0; i < 12; i++ )
		geo->matrix[i] = u2f( *input++ );

	return input;
}

/* Command 0C: Parallel Transfer Vector Write */
static UINT32 * geo_translate_write( geo_state *geo, UINT32 opcode, UINT32 *input )
{
	UINT32  i;

	(void)opcode;

	/* read in the translation vector */
	for( i = 0; i < 3; i++ )
		geo->matrix[i+9] = u2f( *input++ );

	return input;
}

/* Command 0D: Geo Data Memory Push (undocumented, unsupported) */
static UINT32 * geo_data_mem_push( geo_state *geo, UINT32 opcode, UINT32 *input )
{
	UINT32  address, count, i;

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

	geo->state->logerror( "SEGA GEO: Executing unsupported geo_data_mem_push (address = %08x, count = %08x)\n", address, count );

	(void)i;
/*
    for( i = 0; i < count; i++ )
        model2_3d_push( 0 );
*/

	return input;
}

/* Command 0E: Geo Test */
static UINT32 * geo_test( geo_state *geo, UINT32 opcode, UINT32 *input )
{
	UINT32      data, blocks, address, count, checksum, i;

	(void)opcode;

	/* fifo test */
	data = 1;

	for( i = 0; i < 32; i++ )
	{
		if ( *input++ != data )
		{
			/* TODO: Set Red LED on */
			geo->state->logerror( "SEGA GEO: FIFO Test failed\n" );
		}

		data <<= 1;
	}

	/* get the number of checksums we have to run */
	blocks = *input++;

	for( i = 0; i < blocks; i++ )
	{
		UINT32  sum_even, sum_odd, j;

		/* read in the address */
		address = (*input++) & 0x7FFFFF;

		/* read in the count */
		count = *input++;

		/* read in the checksum */
		checksum = *input++;

		/* reset the checksum counters */
		sum_even = 0;
		sum_odd = 0;

		for( j = 0; j < count; j++ )
		{
			data = geo->polygon_rom[address++];

			address &= 0x7FFFFF;

			sum_even += data >> 16;
			sum_even &= 0xFFFF;

			sum_odd += data & 0xFFFF;
			sum_odd &= 0xFFFF;
		}

		sum_even += checksum >> 16;
		sum_even &= 0xFFFF;

		sum_odd += checksum & 0xFFFF;
		sum_odd &= 0xFFFF;

		if ( sum_even != 0 || sum_odd != 0 )
		{
			/* TODO: Set Green LED on */
			geo->state->logerror( "SEGA GEO: Polygon ROM Test failed\n" );
		}
	}

	return input;
}

/* Command 0F: End */
static UINT32 * geo_end( geo_state *geo, UINT32 opcode, UINT32 *input )
{
	raster_state *raster = geo->raster;

	(void)opcode;

	/* signal the end of this data block the rasterizer */
	model2_3d_push( raster, 0xFF000000 );

	/* signal end by returning NULL */
	return NULL;
}

/* Command 10: Dummy */
static UINT32 * geo_dummy( geo_state *geo, UINT32 opcode, UINT32 *input )
{
//  UINT32  data;
	(void)opcode;

	/* do the dummy read cycle */
//  data = *input++;
	input++;

	return input;
}

/* Command 14: Log Data Write */
static UINT32 * geo_log_data( geo_state *geo, UINT32 opcode, UINT32 *input )
{
	raster_state *raster = geo->raster;
	UINT32  i, count;

	/* start by pushing the opcode */
	model2_3d_push( raster, opcode >> 23 );

	/* push the starting address/dsp id */
	model2_3d_push( raster, *input++ );

	/* get the count */
	count = *input++;

	/* push the count */
	model2_3d_push( raster, count << 2 );

	/* loop and send the data */
	for( i = 0; i < count; i++ )
	{
		UINT32  data = *input++;

		model2_3d_push( raster, data & 0xff );
		model2_3d_push( raster, (data >> 8) & 0xff );
		model2_3d_push( raster, (data >> 16) & 0xff );
		model2_3d_push( raster, (data >> 24) & 0xff );
	}

	return input;
}

/* Command 16: LOD */
static UINT32 * geo_lod( geo_state *geo, UINT32 opcode, UINT32 *input )
{
	(void)opcode;

	/* read in the LOD */
	geo->lod = u2f(*input++);

	return input;
}

/* Command 1D: Code Upload  (undocumented, unsupported) */
static UINT32 * geo_code_upload( geo_state *geo, UINT32 opcode, UINT32 *input )
{
	UINT32  count, i;

	/*
	    This command uploads code to program memory and
	    optionally runs it. Probably used for debugging.

	    No games are known to use this command yet.
	*/

	geo->state->logerror( "SEGA GEO: Uploading debug code (unimplemented)\n" );

	(void)opcode;

	/* read in the flags */
//  flags = *input++;
	input++;

	/* read in the count */
	count = *input++;

	for( i = 0; i < count; i++ )
	{
		UINT64  code;

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
    if ( flags & 0x400 )
        code_jump();
*/

	return input;
}

/* Command 1E: Code Jump (undocumented, unsupported) */
static UINT32 * geo_code_jump( geo_state *geo, UINT32 opcode, UINT32 *input )
{
//  UINT32  address;

	/*
	    This command jumps to a specified address in program
	    memory. Code can be uploaded with function 1D.
	    Probably used for debugging.

	    No games are known to use this command yet.
	*/

	geo->state->logerror( "SEGA GEO: Jumping to debug code (unimplemented)\n" );

	(void)opcode;

//  address = *input++ & 0x3FF;
	input++;

/*
    code_jump( address )
*/
	return input;
}

static UINT32 * geo_process_command( geo_state *geo, UINT32 opcode, UINT32 *input )
{
	switch( (opcode >> 23) & 0x1f )
	{
		case 0x00: input = geo_nop( geo, opcode, input );               break;
		case 0x01: input = geo_object_data( geo, opcode, input );       break;
		case 0x02: input = geo_direct_data( geo, opcode, input );       break;
		case 0x03: input = geo_window_data( geo, opcode, input );       break;
		case 0x04: input = geo_texture_data( geo, opcode, input );      break;
		case 0x05: input = geo_polygon_data( geo, opcode, input );      break;
		case 0x06: input = geo_texture_parameters( geo, opcode, input );    break;
		case 0x07: input = geo_mode( geo, opcode, input );              break;
		case 0x08: input = geo_zsort_mode( geo, opcode, input );            break;
		case 0x09: input = geo_focal_distance( geo, opcode, input );        break;
		case 0x0A: input = geo_light_source( geo, opcode, input );      break;
		case 0x0B: input = geo_matrix_write( geo, opcode, input );      break;
		case 0x0C: input = geo_translate_write( geo, opcode, input );   break;
		case 0x0D: input = geo_data_mem_push( geo, opcode, input );     break;
		case 0x0E: input = geo_test( geo, opcode, input );              break;
		case 0x0F: input = geo_end( geo, opcode, input );               break;
		case 0x10: input = geo_dummy( geo, opcode, input );             break;
		case 0x11: input = geo_object_data( geo, opcode, input );       break;
		case 0x12: input = geo_direct_data( geo, opcode, input );       break;
		case 0x13: input = geo_window_data( geo, opcode, input );       break;
		case 0x14: input = geo_log_data( geo, opcode, input );          break;
		case 0x15: input = geo_polygon_data( geo, opcode, input );      break;
		case 0x16: input = geo_lod( geo, opcode, input );               break;
		case 0x17: input = geo_mode( geo, opcode, input );              break;
		case 0x18: input = geo_zsort_mode( geo, opcode, input );            break;
		case 0x19: input = geo_focal_distance( geo, opcode, input );        break;
		case 0x1A: input = geo_light_source( geo, opcode, input );      break;
		case 0x1B: input = geo_matrix_write( geo, opcode, input );      break;
		case 0x1C: input = geo_translate_write( geo, opcode, input );   break;
		case 0x1D: input = geo_code_upload( geo, opcode, input );       break;
		case 0x1E: input = geo_code_jump( geo, opcode, input );         break;
		case 0x1F: input = geo_end( geo, opcode, input );               break;
	}

	return input;
}

static void geo_parse( model2_state *state )
{
	UINT32  address = (state->m_geo_read_start_address/4);
	UINT32 *input = &state->m_bufferram[address];
	UINT32  opcode;

	while( input != NULL && (input - state->m_bufferram) < 0x20000  )
	{
		/* read in the opcode */
		opcode = *input++;

		/* if it's a jump opcode, do the jump */
		if ( opcode & 0x80000000 )
		{
			/* get the address */
			address = (opcode & 0x7FFFF) / 4;

			/* update our pointer */
			input = &state->m_bufferram[address];

			/* go again */
			continue;
		}

		/* process it */
		input = geo_process_command( state->m_geo, opcode, input );
	}
}

/***********************************************************************************************/


VIDEO_START_MEMBER(model2_state,model2)
{
	const rectangle &visarea = m_screen->visible_area();
	int width = visarea.width();
	int height = visarea.height();

	m_sys24_bitmap.allocate(width, height+4);

	m_poly = auto_alloc(machine(), model2_renderer(*this));

	/* initialize the hardware rasterizer */
	model2_3d_init( machine(), (UINT16*)memregion("user3")->base() );

	/* initialize the geometry engine */
	geo_init( machine(), (UINT32*)memregion("user2")->base() );

	/* init various video-related pointers */
	m_palram = auto_alloc_array_clear(machine(), UINT16, 0x2000);
}

UINT32 model2_state::screen_update_model2(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	//logerror("--- frame ---\n");

	bitmap.fill(m_palette->pen(0), cliprect);
	m_sys24_bitmap.fill(0, cliprect);

	segas24_tile *tile = machine().device<segas24_tile>("tile");
	tile->draw(screen, m_sys24_bitmap, cliprect, 7, 0, 0);
	tile->draw(screen, m_sys24_bitmap, cliprect, 6, 0, 0);
	tile->draw(screen, m_sys24_bitmap, cliprect, 5, 0, 0);
	tile->draw(screen, m_sys24_bitmap, cliprect, 4, 0, 0);

	copybitmap_trans(bitmap, m_sys24_bitmap, 0, 0, 0, 0, cliprect, 0);

	/* tell the rasterizer we're starting a frame */
	model2_3d_frame_start(this);

	/* let the geometry engine do it's thing */ /* TODO: don't do it here! */
	geo_parse(this);

	/* have the rasterizer output the frame */
	model2_3d_frame_end( bitmap, cliprect );

	m_sys24_bitmap.fill(0, cliprect);
	tile->draw(screen, m_sys24_bitmap, cliprect, 3, 0, 0);
	tile->draw(screen, m_sys24_bitmap, cliprect, 2, 0, 0);
	tile->draw(screen, m_sys24_bitmap, cliprect, 1, 0, 0);
	tile->draw(screen, m_sys24_bitmap, cliprect, 0, 0, 0);

	copybitmap_trans(bitmap, m_sys24_bitmap, 0, 0, 0, 0, cliprect, 0);

	return 0;
}
