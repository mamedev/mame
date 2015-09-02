// license:BSD-3-Clause
// copyright-holders:Ville Linde, Aaron Giles
/***************************************************************************

    polylgcy.h

    Legacy polygon helper routines.

****************************************************************************

    Pixel model:

    (0.0,0.0)       (1.0,0.0)       (2.0,0.0)       (3.0,0.0)
        +---------------+---------------+---------------+
        |               |               |               |
        |               |               |               |
        |   (0.5,0.5)   |   (1.5,0.5)   |   (2.5,0.5)   |
        |       *       |       *       |       *       |
        |               |               |               |
        |               |               |               |
    (0.0,1.0)       (1.0,1.0)       (2.0,1.0)       (3.0,1.0)
        +---------------+---------------+---------------+
        |               |               |               |
        |               |               |               |
        |   (0.5,1.5)   |   (1.5,1.5)   |   (2.5,1.5)   |
        |       *       |       *       |       *       |
        |               |               |               |
        |               |               |               |
        |               |               |               |
        +---------------+---------------+---------------+
    (0.0,2.0)       (1.0,2.0)       (2.0,2.0)       (3.0,2.0)

***************************************************************************/

#pragma once

#ifndef __POLYLGCY_H__
#define __POLYLGCY_H__


/***************************************************************************
    CONSTANTS
***************************************************************************/

#define MAX_VERTEX_PARAMS                   6
#define MAX_POLYGON_VERTS                   32

#define POLYFLAG_INCLUDE_BOTTOM_EDGE        0x01
#define POLYFLAG_INCLUDE_RIGHT_EDGE         0x02
#define POLYFLAG_NO_WORK_QUEUE              0x04
#define POLYFLAG_ALLOW_QUADS                0x08



/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

/* opaque reference to the poly manager */
struct legacy_poly_manager;


/* input vertex data */
struct poly_vertex
{
	float       x;                          /* X coordinate */
	float       y;                          /* Y coordinate */
	float       p[MAX_VERTEX_PARAMS];       /* interpolated parameter values */
};


/* poly_param_extent describes information for a single parameter in an extent */
struct poly_param_extent
{
	float       start;                      /* parameter value at starting X,Y */
	float       dpdx;                       /* dp/dx relative to starting X */
};


/* poly_extent describes start/end points for a scanline, along with per-scanline parameters */
struct poly_extent
{
	INT16       startx;                     /* starting X coordinate (inclusive) */
	INT16       stopx;                      /* ending X coordinate (exclusive) */
	poly_param_extent param[MAX_VERTEX_PARAMS]; /* starting and dx values for each parameter */
};


/* callback routine to process a batch of scanlines in a triangle */
typedef void (*poly_draw_scanline_func)(void *dest, INT32 scanline, const poly_extent *extent, const void *extradata, int threadid);



/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/


/* ----- initialization/teardown ----- */

/* allocate a new poly manager that can render triangles */
legacy_poly_manager *poly_alloc(running_machine &machine, int max_polys, size_t extra_data_size, UINT8 flags);

/* free a poly manager */
void poly_free(legacy_poly_manager *poly);



/* ----- common functions ----- */

/* wait until all polygons in the queue have been rendered */
void poly_wait(legacy_poly_manager *poly, const char *debug_reason);

/* get a pointer to the extra data for the next polygon */
void *poly_get_extra_data(legacy_poly_manager *poly);



/* ----- core triangle rendering ----- */

/* render a single triangle given 3 vertexes */
UINT32 poly_render_triangle(legacy_poly_manager *poly, void *dest, const rectangle &cliprect, poly_draw_scanline_func callback, int paramcount, const poly_vertex *v1, const poly_vertex *v2, const poly_vertex *v3);

/* render a set of triangles in a fan */
UINT32 poly_render_triangle_fan(legacy_poly_manager *poly, void *dest, const rectangle &cliprect, poly_draw_scanline_func callback, int paramcount, int numverts, const poly_vertex *v);

/* perform a custom render of an object, given specific extents */
UINT32 poly_render_triangle_custom(legacy_poly_manager *poly, void *dest, const rectangle &cliprect, poly_draw_scanline_func callback, int startscanline, int numscanlines, const poly_extent *extents);



/* ----- core quad rendering ----- */

/* render a single quad given 4 vertexes */
UINT32 poly_render_quad(legacy_poly_manager *poly, void *dest, const rectangle &cliprect, poly_draw_scanline_func callback, int paramcount, const poly_vertex *v1, const poly_vertex *v2, const poly_vertex *v3, const poly_vertex *v4);

/* render a set of quads in a fan */
UINT32 poly_render_quad_fan(legacy_poly_manager *poly, void *dest, const rectangle &cliprect, poly_draw_scanline_func callback, int paramcount, int numverts, const poly_vertex *v);



/* ----- core polygon rendering ----- */

/* render a single polygon up to 32 vertices */
UINT32 poly_render_polygon(legacy_poly_manager *poly, void *dest, const rectangle &cliprect, poly_draw_scanline_func callback, int paramcount, int numverts, const poly_vertex *v);



/* ----- clipping ----- */

/* zclip (assumes p[0] == z) a polygon */
int poly_zclip_if_less(int numverts, const poly_vertex *v, poly_vertex *outv, int paramcount, float clipval);


#endif  /* __POLYLGCY_H__ */
