/***************************************************************************

    render.h

    Core rendering routines for MAME.

    Copyright Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

***************************************************************************/

#ifndef __RENDER_H__
#define __RENDER_H__

#include "osdepend.h"

#include <math.h>



/***************************************************************************

    Theory of operation
    -------------------

    A render "target" is described by 5 parameters:

        - width = width, in pixels
        - height = height, in pixels
        - bpp = depth, in bits per pixel
        - orientation = orientation of the target
        - pixel_aspect = aspect ratio of the pixels

    Width, height, and bpp are self-explanatory. The remaining parameters
    need some additional explanation.

    Regarding orientation, there are three orientations that need to be
    dealt with: target orientation, UI orientation, and game orientation.
    In the current model, the UI orientation tracks the target orientation
    so that the UI is (in theory) facing the correct direction. The game
    orientation is specified by the game driver and indicates how the
    game and artwork are rotated.

    Regarding pixel_aspect, this is the aspect ratio of the individual
    pixels, not the aspect ratio of the screen. You can determine this by
    dividing the aspect ratio of the screen by the aspect ratio of the
    resolution. For example, a 4:3 screen displaying 640x480 gives a
    pixel aspect ratio of (4/3)/(640/480) = 1.0, meaning the pixels are
    square. That same screen displaying 1280x1024 would have a pixel
    aspect ratio of (4/3)/(1280/1024) = 1.06666, meaning the pixels are
    slightly wider than they are tall.

    Artwork is always assumed to be a 1.0 pixel aspect ratio. The game
    screens themselves can be variable aspect ratios.

***************************************************************************/


/***************************************************************************
    CONSTANTS
***************************************************************************/

/* render primitive types */
enum
{
	RENDER_PRIMITIVE_LINE,							/* a single line */
	RENDER_PRIMITIVE_QUAD							/* a rectilinear quad */
};


/* render creation flags */
#define RENDER_CREATE_NO_ART		0x01			/* ignore any views that have art in them */
#define RENDER_CREATE_SINGLE_FILE	0x02			/* only load views from the file specified */
#define RENDER_CREATE_HIDDEN		0x04			/* don't make this target visible */

/* layer config masks */
#define LAYER_CONFIG_ENABLE_BACKDROP 0x01			/* enable backdrop layers */
#define LAYER_CONFIG_ENABLE_OVERLAY	0x02			/* enable overlay layers */
#define LAYER_CONFIG_ENABLE_BEZEL	0x04			/* enable bezel layers */
#define LAYER_CONFIG_ZOOM_TO_SCREEN	0x08			/* zoom to screen area by default */
#define LAYER_CONFIG_ENABLE_SCREEN_OVERLAY 0x10		/* enable screen overlays */

#define LAYER_CONFIG_DEFAULT		(LAYER_CONFIG_ENABLE_BACKDROP | \
									 LAYER_CONFIG_ENABLE_OVERLAY | \
									 LAYER_CONFIG_ENABLE_BEZEL | \
									 LAYER_CONFIG_ENABLE_SCREEN_OVERLAY)

/* texture formats */
enum
{
	TEXFORMAT_UNDEFINED = 0,						/* require a format to be specified */
	TEXFORMAT_PALETTE16,							/* 16bpp palettized, alpha ignored */
	TEXFORMAT_PALETTEA16,							/* 16bpp palettized, alpha respected */
	TEXFORMAT_RGB15,								/* 16bpp 5-5-5 RGB */
	TEXFORMAT_RGB32,								/* 32bpp 8-8-8 RGB */
	TEXFORMAT_ARGB32,								/* 32bpp 8-8-8-8 ARGB */
	TEXFORMAT_YUY16									/* 16bpp 8-8 Y/Cb, Y/Cr in sequence */
};

/* blending modes */
enum
{
	BLENDMODE_NONE = 0,								/* no blending */
	BLENDMODE_ALPHA,								/* standard alpha blend */
	BLENDMODE_RGB_MULTIPLY,							/* apply source alpha to source pix, then multiply RGB values */
	BLENDMODE_ADD									/* apply source alpha to source pix, then add to destination */
};


/* flags for primitives */
#define PRIMFLAG_TEXORIENT_SHIFT	0
#define PRIMFLAG_TEXORIENT_MASK		(15 << PRIMFLAG_TEXORIENT_SHIFT)
#define PRIMFLAG_TEXORIENT(x)		((x) << PRIMFLAG_TEXORIENT_SHIFT)
#define PRIMFLAG_GET_TEXORIENT(x)	(((x) & PRIMFLAG_TEXORIENT_MASK) >> PRIMFLAG_TEXORIENT_SHIFT)

#define PRIMFLAG_TEXFORMAT_SHIFT	4
#define PRIMFLAG_TEXFORMAT_MASK		(15 << PRIMFLAG_TEXFORMAT_SHIFT)
#define PRIMFLAG_TEXFORMAT(x)		((x) << PRIMFLAG_TEXFORMAT_SHIFT)
#define PRIMFLAG_GET_TEXFORMAT(x)	(((x) & PRIMFLAG_TEXFORMAT_MASK) >> PRIMFLAG_TEXFORMAT_SHIFT)

#define PRIMFLAG_BLENDMODE_SHIFT	8
#define PRIMFLAG_BLENDMODE_MASK		(15 << PRIMFLAG_BLENDMODE_SHIFT)
#define PRIMFLAG_BLENDMODE(x)		((x) << PRIMFLAG_BLENDMODE_SHIFT)
#define PRIMFLAG_GET_BLENDMODE(x)	(((x) & PRIMFLAG_BLENDMODE_MASK) >> PRIMFLAG_BLENDMODE_SHIFT)

#define PRIMFLAG_ANTIALIAS_SHIFT	12
#define PRIMFLAG_ANTIALIAS_MASK		(1 << PRIMFLAG_ANTIALIAS_SHIFT)
#define PRIMFLAG_ANTIALIAS(x)		((x) << PRIMFLAG_ANTIALIAS_SHIFT)
#define PRIMFLAG_GET_ANTIALIAS(x)	(((x) & PRIMFLAG_ANTIALIAS_MASK) >> PRIMFLAG_ANTIALIAS_SHIFT)

#define PRIMFLAG_SCREENTEX_SHIFT	13
#define PRIMFLAG_SCREENTEX_MASK		(1 << PRIMFLAG_SCREENTEX_SHIFT)
#define PRIMFLAG_SCREENTEX(x)		((x) << PRIMFLAG_SCREENTEX_SHIFT)
#define PRIMFLAG_GET_SCREENTEX(x)	(((x) & PRIMFLAG_SCREENTEX_MASK) >> PRIMFLAG_SCREENTEX_SHIFT)

#define PRIMFLAG_TEXWRAP_SHIFT		14
#define PRIMFLAG_TEXWRAP_MASK		(1 << PRIMFLAG_TEXWRAP_SHIFT)
#define PRIMFLAG_TEXWRAP(x)			((x) << PRIMFLAG_TEXWRAP_SHIFT)
#define PRIMFLAG_GET_TEXWRAP(x)		(((x) & PRIMFLAG_TEXWRAP_MASK) >> PRIMFLAG_TEXWRAP_SHIFT)



/***************************************************************************
    MACROS
***************************************************************************/

/* convenience macros for adding items to the UI container */
#define render_container_add_point(c, x0,y0,diam,argb,flags)	render_container_add_line(c, x0, y0, x0, y0, diam, argb, flags)
#define render_container_add_rect(c, x0,y0,x1,y1,argb,flags)	render_container_add_quad(c, x0, y0, x1, y1, argb, NULL, flags)

/* convenience macros for adding items to a screen container */
#define render_screen_add_point(scr,x0,y0,diam,argb,flags)		render_container_add_line(render_container_get_screen(scr), x0, y0, x0, y0, diam, argb, flags)
#define render_screen_add_line(scr,x0,y0,x1,y1,diam,argb,flags)	render_container_add_line(render_container_get_screen(scr), x0, y0, x1, y1, diam, argb, flags)
#define render_screen_add_rect(scr,x0,y0,x1,y1,argb,flags)		render_container_add_quad(render_container_get_screen(scr), x0, y0, x1, y1, argb, NULL, flags)
#define render_screen_add_quad(scr,x0,y0,x1,y1,argb,tex,flags)	render_container_add_quad(render_container_get_screen(scr), x0, y0, x1, y1, argb, tex, flags)
#define render_screen_add_char(scr,x0,y0,ht,asp,argb,font,ch)	render_container_add_char(render_container_get_screen(scr), x0, y0, ht, asp, argb, font, ch)



/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

// forward definitions
class device_t;
class device_config;
class screen_device;


/*-------------------------------------------------
    callbacks
-------------------------------------------------*/

/* texture scaling callback */
typedef void (*texture_scaler_func)(bitmap_t *dest, const bitmap_t *source, const rectangle *sbounds, void *param);



/*-------------------------------------------------
    opaque types
-------------------------------------------------*/

typedef struct _render_container render_container;
class render_target;
struct render_texture;
typedef struct _render_font render_font;
typedef struct _render_ref render_ref;



/*-------------------------------------------------
    render_bounds - floating point bounding
    rectangle
-------------------------------------------------*/

typedef struct _render_bounds render_bounds;
struct _render_bounds
{
	float				x0;					/* leftmost X coordinate */
	float				y0;					/* topmost Y coordinate */
	float				x1;					/* rightmost X coordinate */
	float				y1;					/* bottommost Y coordinate */
};


/*-------------------------------------------------
    render_color - floating point set of ARGB
    values
-------------------------------------------------*/

typedef struct _render_color render_color;
struct _render_color
{
	float				a;					/* alpha component (0.0 = transparent, 1.0 = opaque) */
	float				r;					/* red component (0.0 = none, 1.0 = max) */
	float				g;					/* green component (0.0 = none, 1.0 = max) */
	float				b;					/* blue component (0.0 = none, 1.0 = max) */
};


/*-------------------------------------------------
    render_texuv - floating point set of UV
    texture coordinates
-------------------------------------------------*/

typedef struct _render_texuv render_texuv;
struct _render_texuv
{
	float				u;					/* U coodinate (0.0-1.0) */
	float				v;					/* V coordinate (0.0-1.0) */
};


/*-------------------------------------------------
    render_quad_texuv - floating point set of UV
    texture coordinates
-------------------------------------------------*/

typedef struct _render_quad_texuv render_quad_texuv;
struct _render_quad_texuv
{
	render_texuv		tl;					/* top-left UV coordinate */
	render_texuv		tr;					/* top-right UV coordinate */
	render_texuv		bl;					/* bottom-left UV coordinate */
	render_texuv		br;					/* bottom-right UV coordinate */
};


/*-------------------------------------------------
    render_texinfo - texture information
-------------------------------------------------*/

typedef struct _render_texinfo render_texinfo;
struct _render_texinfo
{
	void *				base;				/* base of the data */
	UINT32				rowpixels;			/* pixels per row */
	UINT32				width;				/* width of the image */
	UINT32				height;				/* height of the image */
	const rgb_t *		palette;			/* palette for PALETTE16 textures, LUTs for RGB15/RGB32 */
	UINT32				seqid;				/* sequence ID */
};


/*-------------------------------------------------
    render_primitive - a single low-level
    primitive for the rendering engine
-------------------------------------------------*/

typedef struct _render_primitive render_primitive;
struct _render_primitive
{
	render_primitive *	next;				/* pointer to next element */
	int					type;				/* type of primitive */
	render_bounds		bounds;				/* bounds or positions */
	render_color		color;				/* RGBA values */
	UINT32				flags;				/* flags */
	float				width;				/* width (for line primitives) */
	render_texinfo		texture;			/* texture info (for quad primitives) */
	render_quad_texuv	texcoords;			/* texture coordinates (for quad primitives) */
};


/*-------------------------------------------------
    render_primitive_list - an object containing
    a list head plus a lock
-------------------------------------------------*/

typedef struct _render_primitive_list render_primitive_list;
struct _render_primitive_list
{
	render_primitive *	head;				/* head of the list */
	render_primitive **	nextptr;			/* pointer to the next tail pointer */
	osd_lock *			lock;				/* should only should be accessed under this lock */
	render_ref *		reflist;			/* list of references */
};


/*-------------------------------------------------
    render_container_user_settings - an object
    containing user-controllable settings for
    a container
-------------------------------------------------*/

typedef struct _render_container_user_settings render_container_user_settings;
struct _render_container_user_settings
{
	int					orientation;		/* orientation */
	float				brightness;			/* brightness */
	float				contrast;			/* contrast */
	float				gamma;				/* gamma */
	float				xscale;				/* horizontal scale factor */
	float				yscale;				/* vertical scale factor */
	float				xoffset;			/* horizontal offset */
	float				yoffset;			/* vertical offset */
};



/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/


/* ----- core implementation ----- */

/* allocate base structures for the rendering system */
void render_init(running_machine *machine);

/* return a boolean indicating if the screen is live */
int render_is_live_screen(device_t *screen);

/* return the smallest maximum update rate across all targets */
float render_get_max_update_rate(void);

/* select the UI target */
void render_set_ui_target(render_target *target);

/* return the UI target */
render_target *render_get_ui_target(void);

/* return the aspect ratio for UI fonts */
float render_get_ui_aspect(void);



/* ----- render target management ----- */

/* allocate a new render target */
render_target *render_target_alloc(running_machine *machine, const char *layout, UINT32 flags);

/* free memory for a render target */
void render_target_free(render_target *target);

/* get a render_target by index */
render_target *render_target_get_indexed(int index);

/* return the name of the indexed view, or NULL if it doesn't exist */
const char *render_target_get_view_name(render_target *target, int viewindex);

/* return a bitmask of which screens are visible on a given view */
UINT32 render_target_get_view_screens(render_target *target, int viewindex);

/* get the bounds and pixel aspect of a target */
void render_target_get_bounds(render_target *target, INT32 *width, INT32 *height, float *pixel_aspect);

/* set the bounds and pixel aspect of a target */
void render_target_set_bounds(render_target *target, INT32 width, INT32 height, float pixel_aspect);

/* get the maximum update rate (refresh rate) of a target, or 0 if no maximum */
float render_target_get_max_update_rate(render_target *target);

/* set the maximum update rate (refresh rate) of a target, or 0 if no maximum */
void render_target_set_max_update_rate(render_target *target, float updates_per_second);

/* get the orientation of a target */
int render_target_get_orientation(render_target *target);

/* set the orientation of a target */
void render_target_set_orientation(render_target *target, int orientation);

/* get the layer config of a target */
int render_target_get_layer_config(render_target *target);

/* set the layer config of a target */
void render_target_set_layer_config(render_target *target, int layerconfig);

/* return the currently selected view index */
int render_target_get_view(render_target *target);

/* dynamically change the view for a target */
void render_target_set_view(render_target *target, int viewindex);

/* set the upper bound on the texture size */
void render_target_set_max_texture_size(render_target *target, int maxwidth, int maxheight);

/* compute the visible area for the given target with the current layout and proposed new parameters */
void render_target_compute_visible_area(render_target *target, INT32 target_width, INT32 target_height, float target_pixel_aspect, int target_orientation, INT32 *visible_width, INT32 *visible_height);

/* get the "minimum" size of a target, which is the smallest bounds that will ensure at least
   1 target pixel per source pixel for all included screens */
void render_target_get_minimum_size(render_target *target, INT32 *minwidth, INT32 *minheight);

/* return a list of primitives for a given render target */
const render_primitive_list *render_target_get_primitives(render_target *target);

/* attempts to map a point on the specified render_target to the specified container, if possible */
int render_target_map_point_container(render_target *target, INT32 target_x, INT32 target_y, render_container *container, float *container_x, float *container_y);

/* attempts to map a point on the specified render_target to the specified container, if possible */
int render_target_map_point_input(render_target *target, INT32 target_x, INT32 target_y, const char **input_tag, UINT32 *input_mask, float *input_x, float *input_y);



/* ----- render texture management ----- */

/* allocate a new texture */
render_texture *render_texture_alloc(texture_scaler_func scaler, void *param);

/* free an allocated texture */
void render_texture_free(render_texture *texture);

/* set a new source bitmap */
void render_texture_set_bitmap(render_texture *texture, bitmap_t *bitmap, const rectangle *sbounds, int format, palette_t *palette);

/* generic high quality resampling scaler */
void render_texture_hq_scale(bitmap_t *dest, const bitmap_t *source, const rectangle *sbounds, void *param);



/* ----- render containers ----- */

/* empty a container in preparation for new stuff */
void render_container_empty(render_container *container);

/* return true if a container has nothing in it */
int render_container_is_empty(render_container *container);

/* get the current user settings for a container */
void render_container_get_user_settings(render_container *container, render_container_user_settings *settings);

/* set the current user settings for a container */
void render_container_set_user_settings(render_container *container, const render_container_user_settings *settings);

/* set the overlay bitmap for the container */
void render_container_set_overlay(render_container *container, bitmap_t *bitmap);

/* return a pointer to the UI container */
render_container *render_container_get_ui(void);

/* return a pointer to the container for the given screen */
render_container *render_container_get_screen(screen_device *screen);

/* add a line item to the specified container */
void render_container_add_line(render_container *container, float x0, float y0, float x1, float y1, float width, rgb_t argb, UINT32 flags);

/* add a quad item to the specified container */
void render_container_add_quad(render_container *container, float x0, float y0, float x1, float y1, rgb_t argb, render_texture *texture, UINT32 flags);

/* add a char item to the specified container */
void render_container_add_char(render_container *container, float x0, float y0, float height, float aspect, rgb_t argb, render_font *font, UINT16 ch);

/* "drawable" handling for internal debugger */
render_container *render_debug_alloc(render_target *target);
void render_debug_free(render_target *target, render_container *container);
void render_debug_top(render_target *target, render_container *container);

#endif	/* __RENDER_H__ */
