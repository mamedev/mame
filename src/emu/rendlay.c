/***************************************************************************

    rendlay.c

    Core rendering layout parser and manager.

    Copyright (c) 1996-2007, Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

****************************************************************************

    Notes:

        Unlike the old system, the artwork is not rotated with the game
        orientation. This is to support odd configurations like two
        monitors in different orientations. You can specify an orientation
        for a backdrop/screen/overlay/bezel element, but it only applies
        to the artwork itself, and does not affect coordinates in any way.


    Overview of objects:

        layout_file -- A layout_file comprises a list of elements and a
            list of views. The elements are reusable items that the views
            reference.

        layout_view -- A layout_view describes a single view within a
            layout_file. The view is described using arbitrary coordinates
            that are scaled to fit within the render target. Pixels within
            a view are assumed to be square.

        view_item -- Each view has four lists of view_items, one for each
            "layer." Each view item is specified using floating point
            coordinates in arbitrary units, and is assumed to have square
            pixels. Each view item can control its orientation independently.
            Each item can also have an optional name, and can be set at
            runtime into different "states", which control how the embedded
            elements are displayed.

        layout_element -- A layout_element is a description of a piece of
            visible artwork. Most view_items (except for those in the screen
            layer) have exactly one layout_element which describes the
            contents of the item. Elements are separate from items because
            they can be re-used multiple times within a layout. Even though
            an element can contain a number of components, they are treated
            as if they were a single bitmap.

        element_component -- Each layout_element contains one or more
            components. Each component can describe either an image or
            a rectangle/disk primitive. Each component also has a "state"
            associated with it, which controls whether or not the component
            is visible (if the owning item has the same state, it is
            visible).

***************************************************************************/

#include "render.h"
#include "rendfont.h"
#include "rendlay.h"
#include "rendutil.h"
#include "output.h"
#include "xmlfile.h"
#include "png.h"



/***************************************************************************
    STANDARD LAYOUTS
***************************************************************************/

/* single screen layouts */
#include "horizont.lh"
#include "vertical.lh"

/* dual screen layouts */
#include "dualhsxs.lh"
#include "dualhovu.lh"
#include "dualhuov.lh"

/* triple screen layouts */
#include "triphsxs.lh"

/* generic color overlay layouts */
#include "ho20ffff.lh"
#include "ho2eff2e.lh"
#include "ho4f893d.lh"
#include "ho88ffff.lh"
#include "hoa0a0ff.lh"
#include "hoffe457.lh"
#include "voffff20.lh"
#include "hoffff20.lh"



/***************************************************************************
    CONSTANTS
***************************************************************************/

#define LAYOUT_VERSION			2

#define LINE_CAP_NONE  0
#define LINE_CAP_START 1
#define LINE_CAP_END   2

enum
{
	COMPONENT_TYPE_IMAGE = 0,
	COMPONENT_TYPE_RECT,
	COMPONENT_TYPE_DISK,
	COMPONENT_TYPE_TEXT,
	COMPONENT_TYPE_LED7SEG,
	COMPONENT_TYPE_LED14SEG,
	COMPONENT_TYPE_LED16SEG,
	COMPONENT_TYPE_LED14SEGSC,
	COMPONENT_TYPE_LED16SEGSC,
	COMPONENT_TYPE_MAX
};


/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

/* an element_component represents an image, rectangle, or disk in an element */
struct _element_component
{
	element_component *	next;				/* link to next component */
	int					type;				/* type of component */
	int					state;				/* state where this component is visible (-1 means all states) */
	render_bounds		bounds;				/* bounds of the element */
	render_color		color;				/* color of the element */
	const char *		string;				/* string for text components */
	mame_bitmap *		bitmap;				/* source bitmap for images */
	const char *		dirname;			/* directory name of image file (for lazy loading) */
	const char *		imagefile;			/* name of the image file (for lazy loading) */
	const char *		alphafile;			/* name of the alpha file (for lazy loading) */
	int					hasalpha;			/* is there any alpha component present? */
};



/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

/* layout elements */
static void layout_element_scale(mame_bitmap *dest, const mame_bitmap *source, const rectangle *sbounds, void *param);
static void layout_element_draw_rect(mame_bitmap *dest, const rectangle *bounds, const render_color *color);
static void layout_element_draw_disk(mame_bitmap *dest, const rectangle *bounds, const render_color *color);
static void layout_element_draw_text(mame_bitmap *dest, const rectangle *bounds, const render_color *color, const char *string);
static void layout_element_draw_led7seg(mame_bitmap *dest, const rectangle *bounds, const render_color *color, int state);
static void layout_element_draw_led14seg(mame_bitmap *dest, const rectangle *bounds, const render_color *color, int state);
static void layout_element_draw_led16seg(mame_bitmap *dest, const rectangle *bounds, const render_color *color, int state);
static void layout_element_draw_led14segsc(mame_bitmap *dest, const rectangle *bounds, const render_color *color, int state);
static void layout_element_draw_led16segsc(mame_bitmap *dest, const rectangle *bounds, const render_color *color, int state);

/* layout file parsing */
static layout_element *load_layout_element(xml_data_node *elemnode, const char *dirname);
static element_component *load_element_component(xml_data_node *compnode, const char *dirname);
static layout_view *load_layout_view(xml_data_node *viewnode, layout_element *elemlist);
static view_item *load_view_item(xml_data_node *itemnode, layout_element *elemlist);
static mame_bitmap *load_component_bitmap(const char *dirname, const char *file, const char *alphafile, int *hasalpha);
static int load_bounds(xml_data_node *boundsnode, render_bounds *bounds);
static int load_color(xml_data_node *colornode, render_color *color);
static int load_orientation(xml_data_node *orientnode, int *orientation);
static void layout_view_free(layout_view *view);
static void layout_element_free(layout_element *element);



/***************************************************************************
    INLINE FUNCTIONS
***************************************************************************/

/*-------------------------------------------------
    gcd - compute the greatest common divisor (GCD)
    of two integers using the Euclidean algorithm
-------------------------------------------------*/

INLINE int gcd(int a, int b)
{
	while (b != 0)
	{
		int t = b;
		b = a % b;
		a = t;
	}
	return a;
}


/*-------------------------------------------------
    reduce_fraction - reduce a fraction by
    dividing out common factors
-------------------------------------------------*/

INLINE void reduce_fraction(int *num, int *den)
{
	int div;

	/* search the greatest common divisor */
	div = gcd(*num, *den);

	/* reduce the fraction if a common divisor has been found */
	if (div > 1)
	{
		*num /= div;
		*den /= div;
	}
}


/*-------------------------------------------------
    copy_string - make a copy of a string
-------------------------------------------------*/

INLINE const char *copy_string(const char *string)
{
	char *newstring = malloc_or_die(strlen(string) + 1);
	strcpy(newstring, string);
	return newstring;
}


/*-------------------------------------------------
    view_has_art - true if a render_view contains
    any non-screen elements
-------------------------------------------------*/

INLINE int view_has_art(layout_view *view)
{
	return (view->itemlist[ITEM_LAYER_BACKDROP] != 0 ||
			view->itemlist[ITEM_LAYER_OVERLAY] != 0 ||
			view->itemlist[ITEM_LAYER_BEZEL] != 0);
}



/***************************************************************************
    LAYOUT VIEWS
***************************************************************************/

/*-------------------------------------------------
    layout_view_recompute - recompute the bounds
    and aspect ratio of a view and all of its
    contained items
-------------------------------------------------*/

void layout_view_recompute(layout_view *view, int layerconfig)
{
	render_bounds target_bounds;
	float xscale, yscale;
	float xoffs, yoffs;
	int scrfirst = TRUE;
	int first = TRUE;
	int layer;

	/* reset the bounds */
	view->bounds.x0 = view->bounds.y0 = view->bounds.x1 = view->bounds.y1 = 0.0f;
	view->scrbounds.x0 = view->scrbounds.y0 = view->scrbounds.x1 = view->scrbounds.y1 = 0.0f;
	view->screens = 0;

	/* loop over all layers */
	for (layer = 0; layer < ITEM_LAYER_MAX; layer++)
	{
		static const int layer_mask[ITEM_LAYER_MAX] = { LAYER_CONFIG_ENABLE_BACKDROP, 0, LAYER_CONFIG_ENABLE_OVERLAY, LAYER_CONFIG_ENABLE_BEZEL };

		/* determine if this layer should be visible */
		view->layenabled[layer] = (layer_mask[layer] == 0 || (layerconfig & layer_mask[layer]));

		/* only do it if requested */
		if (view->layenabled[layer])
		{
			view_item *item;

			for (item = view->itemlist[layer]; item != NULL; item = item->next)
			{
				/* accumulate bounds */
				if (first)
					view->bounds = item->rawbounds;
				else
					union_render_bounds(&view->bounds, &item->rawbounds);
				first = FALSE;

				/* accumulate screen bounds */
				if (item->element == NULL)
				{
					if (scrfirst)
						view->scrbounds = item->rawbounds;
					else
						union_render_bounds(&view->scrbounds, &item->rawbounds);
					scrfirst = FALSE;
				}
			}
		}
	}

	/* if we have an explicit bounds, override it */
	if (view->expbounds.x1 > view->expbounds.x0)
		view->bounds = view->expbounds;

	/* compute the aspect ratio of the view */
	view->aspect = (view->bounds.x1 - view->bounds.x0) / (view->bounds.y1 - view->bounds.y0);
	view->scraspect = (view->scrbounds.x1 - view->scrbounds.x0) / (view->scrbounds.y1 - view->scrbounds.y0);

	/* if we're handling things normally, the target bounds are (0,0)-(1,1) */
	if (!(layerconfig & LAYER_CONFIG_ZOOM_TO_SCREEN))
	{
		target_bounds.x0 = target_bounds.y0 = 0.0f;
		target_bounds.x1 = target_bounds.y1 = 1.0f;
	}

	/* if we're cropping, we want the screen area to fill (0,0)-(1,1) */
	else
	{
		float targwidth = (view->bounds.x1 - view->bounds.x0) / (view->scrbounds.x1 - view->scrbounds.x0);
		float targheight = (view->bounds.y1 - view->bounds.y0) / (view->scrbounds.y1 - view->scrbounds.y0);
		target_bounds.x0 = (view->bounds.x0 - view->scrbounds.x0) / (view->bounds.x1 - view->bounds.x0) * targwidth;
		target_bounds.y0 = (view->bounds.y0 - view->scrbounds.y0) / (view->bounds.y1 - view->bounds.y0) * targheight;
		target_bounds.x1 = target_bounds.x0 + targwidth;
		target_bounds.y1 = target_bounds.y0 + targheight;
	}

	/* determine the scale/offset for normalization */
	xoffs = view->bounds.x0;
	yoffs = view->bounds.y0;
	xscale = (target_bounds.x1 - target_bounds.x0) / (view->bounds.x1 - view->bounds.x0);
	yscale = (target_bounds.y1 - target_bounds.y0) / (view->bounds.y1 - view->bounds.y0);

	/* normalize all the item bounds */
	for (layer = 0; layer < ITEM_LAYER_MAX; layer++)
	{
		view_item *item;

		/* adjust the bounds for each item */
		for (item = view->itemlist[layer]; item; item = item->next)
		{
			item->bounds.x0 = target_bounds.x0 + (item->rawbounds.x0 - xoffs) * xscale;
			item->bounds.x1 = target_bounds.x0 + (item->rawbounds.x1 - xoffs) * xscale;
			item->bounds.y0 = target_bounds.y0 + (item->rawbounds.y0 - yoffs) * yscale;
			item->bounds.y1 = target_bounds.y0 + (item->rawbounds.y1 - yoffs) * yscale;

			/* accumulate the screens in use while we're scanning */
			if (item->element == NULL)
				view->screens |= 1 << item->index;
		}
	}
}



/***************************************************************************
    LAYOUT ELEMENTS
***************************************************************************/

/*-------------------------------------------------
    layout_element_scale - scale an element by
    rendering all the components at the
    appropriate resolution
-------------------------------------------------*/

static void layout_element_scale(mame_bitmap *dest, const mame_bitmap *source, const rectangle *sbounds, void *param)
{
	element_texture *elemtex = param;
	element_component *component;

	/* iterate over components that are part of the current state */
	for (component = elemtex->element->complist; component != NULL; component = component->next)
		if (component->state == -1 || component->state == elemtex->state)
		{
			rectangle bounds;

			/* get the local scaled bounds */
			bounds.min_x = render_round_nearest(component->bounds.x0 * dest->width);
			bounds.min_y = render_round_nearest(component->bounds.y0 * dest->height);
			bounds.max_x = render_round_nearest(component->bounds.x1 * dest->width);
			bounds.max_y = render_round_nearest(component->bounds.y1 * dest->height);

			/* based on the component type, add to the texture */
			switch (component->type)
			{
				case COMPONENT_TYPE_IMAGE:
					if (component->bitmap == NULL)
						component->bitmap = load_component_bitmap(component->dirname, component->imagefile, component->alphafile, &component->hasalpha);
					render_resample_argb_bitmap_hq(
							BITMAP_ADDR32(dest, bounds.min_y, bounds.min_x),
							dest->rowpixels,
							bounds.max_x - bounds.min_x,
							bounds.max_y - bounds.min_y,
							component->bitmap, NULL, &component->color);
					break;

				case COMPONENT_TYPE_RECT:
					layout_element_draw_rect(dest, &bounds, &component->color);
					break;

				case COMPONENT_TYPE_DISK:
					layout_element_draw_disk(dest, &bounds, &component->color);
					break;

				case COMPONENT_TYPE_TEXT:
					layout_element_draw_text(dest, &bounds, &component->color, component->string);
					break;

				case COMPONENT_TYPE_LED7SEG:
					layout_element_draw_led7seg(dest, &bounds, &component->color, elemtex->state);
					break;

				case COMPONENT_TYPE_LED14SEG:
					layout_element_draw_led14seg(dest, &bounds, &component->color, elemtex->state);
					break;

				case COMPONENT_TYPE_LED16SEG:
					layout_element_draw_led16seg(dest, &bounds, &component->color, elemtex->state);
					break;

				case COMPONENT_TYPE_LED14SEGSC:
					layout_element_draw_led14segsc(dest, &bounds, &component->color, elemtex->state);
					break;

				case COMPONENT_TYPE_LED16SEGSC:
					layout_element_draw_led16segsc(dest, &bounds, &component->color, elemtex->state);
					break;
			}
		}
}


/*-------------------------------------------------
    layout_element_draw_rect - draw a rectangle
    in the specified color
-------------------------------------------------*/

static void layout_element_draw_rect(mame_bitmap *dest, const rectangle *bounds, const render_color *color)
{
	UINT32 r, g, b, inva;
	UINT32 x, y;

	/* compute premultiplied colors */
	r = color->r * color->a * 255.0;
	g = color->g * color->a * 255.0;
	b = color->b * color->a * 255.0;
	inva = (1.0f - color->a) * 255.0;

	/* iterate over X and Y */
	for (y = bounds->min_y; y < bounds->max_y; y++)
		for (x = bounds->min_x; x < bounds->max_x; x++)
		{
			UINT32 finalr = r;
			UINT32 finalg = g;
			UINT32 finalb = b;

			/* if we're translucent, add in the destination pixel contribution */
			if (inva > 0)
			{
				UINT32 dpix = *BITMAP_ADDR32(dest, y, x);
				finalr += (RGB_RED(dpix) * inva) >> 8;
				finalg += (RGB_GREEN(dpix) * inva) >> 8;
				finalb += (RGB_BLUE(dpix) * inva) >> 8;
			}

			/* store the target pixel, dividing the RGBA values by the overall scale factor */
			*BITMAP_ADDR32(dest, y, x) = MAKE_ARGB(0xff, finalr, finalg, finalb);
		}
}


/*-------------------------------------------------
    layout_element_draw_disk - draw an ellipse
    in the specified color
-------------------------------------------------*/

static void layout_element_draw_disk(mame_bitmap *dest, const rectangle *bounds, const render_color *color)
{
	float xcenter, ycenter;
	float xradius, yradius, ooyradius2;
	UINT32 r, g, b, inva;
	UINT32 x, y;

	/* compute premultiplied colors */
	r = color->r * color->a * 255.0;
	g = color->g * color->a * 255.0;
	b = color->b * color->a * 255.0;
	inva = (1.0f - color->a) * 255.0;

	/* find the center */
	xcenter = (float)(bounds->min_x + bounds->max_x) * 0.5f;
	ycenter = (float)(bounds->min_y + bounds->max_y) * 0.5f;
	xradius = (float)(bounds->max_x - bounds->min_x) * 0.5f;
	yradius = (float)(bounds->max_y - bounds->min_y) * 0.5f;
	ooyradius2 = 1.0f / (yradius * yradius);

	/* iterate over y */
	for (y = bounds->min_y; y < bounds->max_y; y++)
	{
		float ycoord = ycenter - ((float)y + 0.5f);
		float xval = xradius * sqrt(1.0f - (ycoord * ycoord) * ooyradius2);
		INT32 left, right;

		/* compute left/right coordinates */
		left = (INT32)(xcenter - xval + 0.5f);
		right = (INT32)(xcenter + xval + 0.5f);

		/* draw this scanline */
		for (x = left; x < right; x++)
		{
			UINT32 finalr = r;
			UINT32 finalg = g;
			UINT32 finalb = b;

			/* if we're translucent, add in the destination pixel contribution */
			if (inva > 0)
			{
				UINT32 dpix = *BITMAP_ADDR32(dest, y, x);
				finalr += (RGB_RED(dpix) * inva) >> 8;
				finalg += (RGB_GREEN(dpix) * inva) >> 8;
				finalb += (RGB_BLUE(dpix) * inva) >> 8;
			}

			/* store the target pixel, dividing the RGBA values by the overall scale factor */
			*BITMAP_ADDR32(dest, y, x) = MAKE_ARGB(0xff, finalr, finalg, finalb);
		}
	}
}


/*-------------------------------------------------
    layout_element_draw_text - draw text in the
    specified color
-------------------------------------------------*/

static void layout_element_draw_text(mame_bitmap *dest, const rectangle *bounds, const render_color *color, const char *string)
{
	render_font *font = render_font_alloc(NULL);
	mame_bitmap *tempbitmap;
	UINT32 r, g, b, a;
	float aspect = 1.0f;
	INT32 curx, width;
	const char *s;

	/* compute premultiplied colors */
	r = color->r * 255.0;
	g = color->g * 255.0;
	b = color->b * 255.0;
	a = color->a * 255.0;

	/* get the width of the string */
	while (1)
	{
		width = render_font_get_string_width(font, bounds->max_y - bounds->min_y, aspect, string);
		if (width < bounds->max_x - bounds->min_x)
			break;
		aspect *= 0.9f;
	}
	curx = bounds->min_x + (bounds->max_x - bounds->min_x - width) / 2;

	/* allocate a temporary bitmap */
	tempbitmap = bitmap_alloc(dest->width, dest->height, BITMAP_FORMAT_ARGB32);

	/* loop over characters */
	for (s = string; *s != 0; s++)
	{
		rectangle chbounds;
		int x, y;

		/* get the font bitmap */
		render_font_get_scaled_bitmap_and_bounds(font, tempbitmap, bounds->max_y - bounds->min_y, aspect, *s, &chbounds);

		/* copy the data into the target */
		for (y = 0; y < chbounds.max_y - chbounds.min_y; y++)
		{
			int effy = bounds->min_y + y;
			if (effy >= bounds->min_y && effy <= bounds->max_y)
			{
				UINT32 *s = BITMAP_ADDR32(tempbitmap, y, 0);
				UINT32 *d = BITMAP_ADDR32(dest, effy, 0);
				for (x = 0; x < chbounds.max_x - chbounds.min_x; x++)
				{
					int effx = curx + x + chbounds.min_x;
					if (effx >= bounds->min_x && effx <= bounds->max_x)
					{
						UINT32 spix = RGB_ALPHA(s[x]);
						if (spix != 0)
						{
							UINT32 dpix = d[effx];
							UINT32 ta, tr, tg, tb;

							ta = (a * (spix + 1)) >> 8;
							tr = (r * ta + RGB_RED(dpix) * (0x100 - ta)) >> 8;
							tg = (g * ta + RGB_GREEN(dpix) * (0x100 - ta)) >> 8;
							tb = (b * ta + RGB_BLUE(dpix) * (0x100 - ta)) >> 8;
							d[effx] = MAKE_ARGB(0xff, tr, tg, tb);
						}
					}
				}
			}
		}

		/* advance in the X direction */
		curx += render_font_get_char_width(font, bounds->max_y - bounds->min_y, aspect, *s);
	}

	/* free the temporary bitmap and font */
	bitmap_free(tempbitmap);
	render_font_free(font);
}


/*-------------------------------------------------
    draw_segment_horizontal_caps - draw a
    horizontal LED segment with definable end
    and start points
-------------------------------------------------*/

static void draw_segment_horizontal_caps(mame_bitmap *dest, int minx, int maxx, int midy, int width, int caps, rgb_t color)
{
	int x, y;

	/* loop over the width of the segment */
	for (y = 0; y < width / 2; y++)
	{
		UINT32 *d0 = BITMAP_ADDR32(dest, midy - y, 0);
		UINT32 *d1 = BITMAP_ADDR32(dest, midy + y, 0);
		int ty = (y < width / 8) ? width / 8 : y;

		/* loop over the length of the segment */
		for (x = minx + ((caps & LINE_CAP_START) ? ty : 0); x < maxx - ((caps & LINE_CAP_END) ? ty : 0); x++)
			d0[x] = d1[x] = color;
	}
}


/*-------------------------------------------------
    draw_segment_horizontal - draw a horizontal
    LED segment
-------------------------------------------------*/

static void draw_segment_horizontal(mame_bitmap *dest, int minx, int maxx, int midy, int width, rgb_t color)
{
	draw_segment_horizontal_caps(dest, minx, maxx, midy, width, LINE_CAP_START | LINE_CAP_END, color);
}


/*-------------------------------------------------
    draw_segment_vertical_caps - draw a
    vertical LED segment with definable end
    and start points
-------------------------------------------------*/

static void draw_segment_vertical_caps(mame_bitmap *dest, int miny, int maxy, int midx, int width, int caps, rgb_t color)
{
	int x, y;

	/* loop over the width of the segment */
	for (x = 0; x < width / 2; x++)
	{
		UINT32 *d0 = BITMAP_ADDR32(dest, 0, midx - x);
		UINT32 *d1 = BITMAP_ADDR32(dest, 0, midx + x);
		int tx = (x < width / 8) ? width / 8 : x;

		/* loop over the length of the segment */
		for (y = miny + ((caps & LINE_CAP_START) ? tx : 0); y < maxy - ((caps & LINE_CAP_END) ? tx : 0); y++)
			d0[y * dest->rowpixels] = d1[y * dest->rowpixels] = color;
	}
}


/*-------------------------------------------------
    draw_segment_vertical - draw a vertical
    LED segment
-------------------------------------------------*/

static void draw_segment_vertical(mame_bitmap *dest, int miny, int maxy, int midx, int width, rgb_t color)
{
	draw_segment_vertical_caps(dest, miny, maxy, midx, width, LINE_CAP_START | LINE_CAP_END, color);
}


/*-------------------------------------------------
    draw_segment_diagonal_1 - draw a diagonal
    LED segment that looks like this: /
-------------------------------------------------*/

static void draw_segment_diagonal_1(mame_bitmap *dest, int minx, int maxx, int miny, int maxy, int width, rgb_t color)
{
	int x, y;
	float ratio;

	/* compute parameters */
	width *= 1.5;
	ratio = (maxy - miny - width) / (float)(maxx - minx);

	/* draw line */
	for (x = minx; x < maxx; x++)
		if (x >= 0 && x < dest->width)
		{
			UINT32 *d = BITMAP_ADDR32(dest, 0, x);
			int step = (x - minx) * ratio;

			for (y = maxy - width - step; y < maxy - step; y++)
				if (y >= 0 && y < dest->height)
				{
					d[y * dest->rowpixels] = color;
				}
		}
}


/*-------------------------------------------------
    draw_segment_diagonal_2 - draw a diagonal
    LED segment that looks like this: \
-------------------------------------------------*/

static void draw_segment_diagonal_2(mame_bitmap *dest, int minx, int maxx, int miny, int maxy, int width, rgb_t color)
{
	int x, y;
	float ratio;

	/* compute parameters */
	width *= 1.5;
	ratio = (maxy - miny - width) / (float)(maxx - minx);

	/* draw line */
	for (x = minx; x < maxx; x++)
		if (x >= 0 && x < dest->width)
		{
			UINT32 *d = BITMAP_ADDR32(dest, 0, x);
			int step = (x - minx) * ratio;

			for (y = miny + step; y < miny + step + width; y++)
				if (y >= 0 && y < dest->height)
				{
					d[y * dest->rowpixels] = color;
				}
		}
}


/*-------------------------------------------------
    draw_segment_decimal - draw a decimal point
-------------------------------------------------*/

static void draw_segment_decimal(mame_bitmap *dest, int midx, int midy, int width, rgb_t color)
{
	float ooradius2;
	UINT32 x, y;

	/* compute parameters */
	width /= 2;
	ooradius2 = 1.0f / (float)(width * width);

	/* iterate over y */
	for (y = 0; y <= width; y++)
	{
		UINT32 *d0 = BITMAP_ADDR32(dest, midy - y, 0);
		UINT32 *d1 = BITMAP_ADDR32(dest, midy + y, 0);
		float xval = width * sqrt(1.0f - (float)(y * y) * ooradius2);
		INT32 left, right;

		/* compute left/right coordinates */
		left = midx - (INT32)(xval + 0.5f);
		right = midx + (INT32)(xval + 0.5f);

		/* draw this scanline */
		for (x = left; x < right; x++)
			d0[x] = d1[x] = color;
	}
}

/*-------------------------------------------------
    draw_segment_comma - draw a comma tail
-------------------------------------------------*/
#if 0
static void draw_segment_comma(mame_bitmap *dest, int minx, int maxx, int miny, int maxy, int width, rgb_t color)
{
	int x, y;
	float ratio;

	/* compute parameters */
	width *= 1.5;
	ratio = (maxy - miny - width) / (float)(maxx - minx);

	/* draw line */
	for (x = minx; x < maxx; x++)
	{
		UINT32 *d = BITMAP_ADDR32(dest, 0, x);
		int step = (x - minx) * ratio;

		for (y = maxy; y < maxy  - width - step; y--)
		{
			d[y * dest->rowpixels] = color;
		}
	}
}
#endif


/*-------------------------------------------------
    apply_skew - apply skew to a mame_bitmap
-------------------------------------------------*/

static void apply_skew(mame_bitmap *dest, int skewwidth)
{
	int x, y;

	for (y = 0; y < dest->height; y++)
	{
		UINT32 *destrow = BITMAP_ADDR32(dest, y, 0);
		int offs = skewwidth * (dest->height - y) / dest->height;
		for (x = dest->width - skewwidth - 1; x >= 0; x--)
			destrow[x + offs] = destrow[x];
		for (x = 0; x < offs; x++)
			destrow[x] = 0;
	}
}


/*-------------------------------------------------
    layout_element_draw_led7seg - draw a
    7-segment LCD
-------------------------------------------------*/

static void layout_element_draw_led7seg(mame_bitmap *dest, const rectangle *bounds, const render_color *color, int pattern)
{
	const rgb_t onpen = MAKE_ARGB(0xff,0xff,0xff,0xff);
	const rgb_t offpen = MAKE_ARGB(0xff,0x20,0x20,0x20);
	int bmwidth, bmheight, segwidth, skewwidth;
	mame_bitmap *tempbitmap;

	/* sizes for computation */
	bmwidth = 250;
	bmheight = 400;
	segwidth = 40;
	skewwidth = 40;

	/* allocate a temporary bitmap for drawing */
	tempbitmap = bitmap_alloc(bmwidth + skewwidth, bmheight, BITMAP_FORMAT_ARGB32);
	fillbitmap(tempbitmap, MAKE_ARGB(0xff,0x00,0x00,0x00), NULL);

	/* top bar */
	draw_segment_horizontal(tempbitmap, 0 + 2*segwidth/3, bmwidth - 2*segwidth/3, 0 + segwidth/2, segwidth, (pattern & (1 << 0)) ? onpen : offpen);

	/* top-right bar */
	draw_segment_vertical(tempbitmap, 0 + 2*segwidth/3, bmheight/2 - segwidth/3, bmwidth - segwidth/2, segwidth, (pattern & (1 << 1)) ? onpen : offpen);

	/* bottom-right bar */
	draw_segment_vertical(tempbitmap, bmheight/2 + segwidth/3, bmheight - 2*segwidth/3, bmwidth - segwidth/2, segwidth, (pattern & (1 << 2)) ? onpen : offpen);

	/* bottom bar */
	draw_segment_horizontal(tempbitmap, 0 + 2*segwidth/3, bmwidth - 2*segwidth/3, bmheight - segwidth/2, segwidth, (pattern & (1 << 3)) ? onpen : offpen);

	/* bottom-left bar */
	draw_segment_vertical(tempbitmap, bmheight/2 + segwidth/3, bmheight - 2*segwidth/3, 0 + segwidth/2, segwidth, (pattern & (1 << 4)) ? onpen : offpen);

	/* top-left bar */
	draw_segment_vertical(tempbitmap, 0 + 2*segwidth/3, bmheight/2 - segwidth/3, 0 + segwidth/2, segwidth, (pattern & (1 << 5)) ? onpen : offpen);

	/* middle bar */
	draw_segment_horizontal(tempbitmap, 0 + 2*segwidth/3, bmwidth - 2*segwidth/3, bmheight/2, segwidth, (pattern & (1 << 6)) ? onpen : offpen);

	/* apply skew */
	apply_skew(tempbitmap, 40);

	/* decimal point */
	draw_segment_decimal(tempbitmap, bmwidth + segwidth/2, bmheight - segwidth/2, segwidth, (pattern & (1 << 7)) ? onpen : offpen);

	/* resample to the target size */
	render_resample_argb_bitmap_hq(dest->base, dest->rowpixels, dest->width, dest->height, tempbitmap, NULL, color);

	bitmap_free(tempbitmap);
}


/*-------------------------------------------------
    layout_element_draw_led14seg - draw a
    14-segment LCD
-------------------------------------------------*/

static void layout_element_draw_led14seg(mame_bitmap *dest, const rectangle *bounds, const render_color *color, int pattern)
{
	const rgb_t onpen = MAKE_ARGB(0xff, 0xff, 0xff, 0xff);
	const rgb_t offpen = MAKE_ARGB(0xff, 0x20, 0x20, 0x20);
	int bmwidth, bmheight, segwidth, skewwidth;
	mame_bitmap *tempbitmap;

	/* sizes for computation */
	bmwidth = 250;
	bmheight = 400;
	segwidth = 40;
	skewwidth = 40;

	/* allocate a temporary bitmap for drawing */
	tempbitmap = bitmap_alloc(bmwidth + skewwidth, bmheight, BITMAP_FORMAT_ARGB32);
	fillbitmap(tempbitmap, MAKE_ARGB(0xff, 0x00, 0x00, 0x00), NULL);

	/* top bar */
	draw_segment_horizontal(tempbitmap,
		0 + 2*segwidth/3, bmwidth - 2*segwidth/3, 0 + segwidth/2,
		segwidth, (pattern & (1 << 0)) ? onpen : offpen);

	/* right-top bar */
	draw_segment_vertical(tempbitmap,
		0 + 2*segwidth/3, bmheight/2 - segwidth/3, bmwidth - segwidth/2,
		segwidth, (pattern & (1 << 1)) ? onpen : offpen);

	/* right-bottom bar */
	draw_segment_vertical(tempbitmap,
		bmheight/2 + segwidth/3, bmheight - 2*segwidth/3, bmwidth - segwidth/2,
		segwidth, (pattern & (1 << 2)) ? onpen : offpen);

	/* bottom bar */
	draw_segment_horizontal(tempbitmap,
		0 + 2*segwidth/3, bmwidth - 2*segwidth/3, bmheight - segwidth/2,
		segwidth, (pattern & (1 << 3)) ? onpen : offpen);

	/* left-bottom bar */
	draw_segment_vertical(tempbitmap,
		bmheight/2 + segwidth/3, bmheight - 2*segwidth/3, 0 + segwidth/2,
		segwidth, (pattern & (1 << 4)) ? onpen : offpen);

	/* left-top bar */
	draw_segment_vertical(tempbitmap,
		0 + 2*segwidth/3, bmheight/2 - segwidth/3, 0 + segwidth/2,
		segwidth, (pattern & (1 << 5)) ? onpen : offpen);

	/* horizontal-middle-left bar */
	draw_segment_horizontal_caps(tempbitmap,
		0 + 2*segwidth/3, bmwidth/2 - segwidth/10, bmheight/2,
		segwidth, LINE_CAP_START, (pattern & (1 << 6)) ? onpen : offpen);

	/* horizontal-middle-right bar */
	draw_segment_horizontal_caps(tempbitmap,
		0 + bmwidth/2 + segwidth/10, bmwidth - 2*segwidth/3, bmheight/2,
		segwidth, LINE_CAP_END, (pattern & (1 << 7)) ? onpen : offpen);

	/* vertical-middle-top bar */
	draw_segment_vertical_caps(tempbitmap,
		0 + segwidth + segwidth/3, bmheight/2 - segwidth/2 - segwidth/3, bmwidth/2,
		segwidth, LINE_CAP_NONE, (pattern & (1 << 8)) ? onpen : offpen);

	/* vertical-middle-bottom bar */
	draw_segment_vertical_caps(tempbitmap,
		bmheight/2 + segwidth/2 + segwidth/3, bmheight - segwidth - segwidth/3, bmwidth/2,
		segwidth, LINE_CAP_NONE, (pattern & (1 << 9)) ? onpen : offpen);

	/* diagonal-left-bottom bar */
	draw_segment_diagonal_1(tempbitmap,
		0 + segwidth + segwidth/5, bmwidth/2 - segwidth/2 - segwidth/5,
		bmheight/2 + segwidth/2 + segwidth/3, bmheight - segwidth - segwidth/3,
		segwidth, (pattern & (1 << 10)) ? onpen : offpen);

	/* diagonal-left-top bar */
	draw_segment_diagonal_2(tempbitmap,
		0 + segwidth + segwidth/5, bmwidth/2 - segwidth/2 - segwidth/5,
		0 + segwidth + segwidth/3, bmheight/2 - segwidth/2 - segwidth/3,
		segwidth, (pattern & (1 << 11)) ? onpen : offpen);

	/* diagonal-right-top bar */
	draw_segment_diagonal_1(tempbitmap,
		bmwidth/2 + segwidth/2 + segwidth/5, bmwidth - segwidth - segwidth/5,
		0 + segwidth + segwidth/3, bmheight/2 - segwidth/2 - segwidth/3,
		segwidth, (pattern & (1 << 12)) ? onpen : offpen);

	/* diagonal-right-bottom bar */
	draw_segment_diagonal_2(tempbitmap,
		bmwidth/2 + segwidth/2 + segwidth/5, bmwidth - segwidth - segwidth/5,
		bmheight/2 + segwidth/2 + segwidth/3, bmheight - segwidth - segwidth/3,
		segwidth, (pattern & (1 << 13)) ? onpen : offpen);

	/* apply skew */
	apply_skew(tempbitmap, 40);

	/* resample to the target size */
	render_resample_argb_bitmap_hq(dest->base, dest->rowpixels, dest->width, dest->height, tempbitmap, NULL, color);

	bitmap_free(tempbitmap);
}


/*-------------------------------------------------
    layout_element_draw_led14segsc - draw a
    14-segment LCD with semicolon (2 extra segments)
-------------------------------------------------*/

static void layout_element_draw_led14segsc(mame_bitmap *dest, const rectangle *bounds, const render_color *color, int pattern)
{
	const rgb_t onpen = MAKE_ARGB(0xff, 0xff, 0xff, 0xff);
	const rgb_t offpen = MAKE_ARGB(0xff, 0x20, 0x20, 0x20);
	int bmwidth, bmheight, segwidth, skewwidth;
	mame_bitmap *tempbitmap;

	/* sizes for computation */
	bmwidth = 250;
	bmheight = 400;
	segwidth = 40;
	skewwidth = 40;

	/* allocate a temporary bitmap for drawing, adding some extra space for the tail */
	tempbitmap = bitmap_alloc(bmwidth + skewwidth, bmheight + segwidth, BITMAP_FORMAT_ARGB32);
	fillbitmap(tempbitmap, MAKE_ARGB(0xff, 0x00, 0x00, 0x00), NULL);

	/* top bar */
	draw_segment_horizontal(tempbitmap,
		0 + 2*segwidth/3, bmwidth - 2*segwidth/3, 0 + segwidth/2,
		segwidth, (pattern & (1 << 0)) ? onpen : offpen);

	/* right-top bar */
	draw_segment_vertical(tempbitmap,
		0 + 2*segwidth/3, bmheight/2 - segwidth/3, bmwidth - segwidth/2,
		segwidth, (pattern & (1 << 1)) ? onpen : offpen);

	/* right-bottom bar */
	draw_segment_vertical(tempbitmap,
		bmheight/2 + segwidth/3, bmheight - 2*segwidth/3, bmwidth - segwidth/2,
		segwidth, (pattern & (1 << 2)) ? onpen : offpen);

	/* bottom bar */
	draw_segment_horizontal(tempbitmap,
		0 + 2*segwidth/3, bmwidth - 2*segwidth/3, bmheight - segwidth/2,
		segwidth, (pattern & (1 << 3)) ? onpen : offpen);

	/* left-bottom bar */
	draw_segment_vertical(tempbitmap,
		bmheight/2 + segwidth/3, bmheight - 2*segwidth/3, 0 + segwidth/2,
		segwidth, (pattern & (1 << 4)) ? onpen : offpen);

	/* left-top bar */
	draw_segment_vertical(tempbitmap,
		0 + 2*segwidth/3, bmheight/2 - segwidth/3, 0 + segwidth/2,
		segwidth, (pattern & (1 << 5)) ? onpen : offpen);

	/* horizontal-middle-left bar */
	draw_segment_horizontal_caps(tempbitmap,
		0 + 2*segwidth/3, bmwidth/2 - segwidth/10, bmheight/2,
		segwidth, LINE_CAP_START, (pattern & (1 << 6)) ? onpen : offpen);

	/* horizontal-middle-right bar */
	draw_segment_horizontal_caps(tempbitmap,
		0 + bmwidth/2 + segwidth/10, bmwidth - 2*segwidth/3, bmheight/2,
		segwidth, LINE_CAP_END, (pattern & (1 << 7)) ? onpen : offpen);

	/* vertical-middle-top bar */
	draw_segment_vertical_caps(tempbitmap,
		0 + segwidth + segwidth/3, bmheight/2 - segwidth/2 - segwidth/3, bmwidth/2,
		segwidth, LINE_CAP_NONE, (pattern & (1 << 8)) ? onpen : offpen);

	/* vertical-middle-bottom bar */
	draw_segment_vertical_caps(tempbitmap,
		bmheight/2 + segwidth/2 + segwidth/3, bmheight - segwidth - segwidth/3, bmwidth/2,
		segwidth, LINE_CAP_NONE, (pattern & (1 << 9)) ? onpen : offpen);

	/* diagonal-left-bottom bar */
	draw_segment_diagonal_1(tempbitmap,
		0 + segwidth + segwidth/5, bmwidth/2 - segwidth/2 - segwidth/5,
		bmheight/2 + segwidth/2 + segwidth/3, bmheight - segwidth - segwidth/3,
		segwidth, (pattern & (1 << 10)) ? onpen : offpen);

	/* diagonal-left-top bar */
	draw_segment_diagonal_2(tempbitmap,
		0 + segwidth + segwidth/5, bmwidth/2 - segwidth/2 - segwidth/5,
		0 + segwidth + segwidth/3, bmheight/2 - segwidth/2 - segwidth/3,
		segwidth, (pattern & (1 << 11)) ? onpen : offpen);

	/* diagonal-right-top bar */
	draw_segment_diagonal_1(tempbitmap,
		bmwidth/2 + segwidth/2 + segwidth/5, bmwidth - segwidth - segwidth/5,
		0 + segwidth + segwidth/3, bmheight/2 - segwidth/2 - segwidth/3,
		segwidth, (pattern & (1 << 12)) ? onpen : offpen);

	/* diagonal-right-bottom bar */
	draw_segment_diagonal_2(tempbitmap,
		bmwidth/2 + segwidth/2 + segwidth/5, bmwidth - segwidth - segwidth/5,
		bmheight/2 + segwidth/2 + segwidth/3, bmheight - segwidth - segwidth/3,
		segwidth, (pattern & (1 << 13)) ? onpen : offpen);

	/* apply skew */
	apply_skew(tempbitmap, 40);

	/* decimal point */
	draw_segment_decimal(tempbitmap, bmwidth + segwidth/2, bmheight - segwidth/2, segwidth, (pattern & (1 << 14)) ? onpen : offpen);

	/* comma tail */
	draw_segment_diagonal_1(tempbitmap,
		bmwidth - (segwidth/2), bmwidth + segwidth,
		bmheight - (segwidth), bmheight + segwidth*1.5,
		segwidth/2, (pattern & (1 << 15)) ? onpen : offpen);

	/* resample to the target size */
	render_resample_argb_bitmap_hq(dest->base, dest->rowpixels, dest->width, dest->height, tempbitmap, NULL, color);

	bitmap_free(tempbitmap);
}


/*-------------------------------------------------
    layout_element_draw_led16seg - draw a
    16-segment LCD
-------------------------------------------------*/

static void layout_element_draw_led16seg(mame_bitmap *dest, const rectangle *bounds, const render_color *color, int pattern)
{
	const rgb_t onpen = MAKE_ARGB(0xff, 0xff, 0xff, 0xff);
	const rgb_t offpen = MAKE_ARGB(0xff, 0x20, 0x20, 0x20);
	int bmwidth, bmheight, segwidth, skewwidth;
	mame_bitmap *tempbitmap;

	/* sizes for computation */
	bmwidth = 250;
	bmheight = 400;
	segwidth = 40;
	skewwidth = 40;

	/* allocate a temporary bitmap for drawing */
	tempbitmap = bitmap_alloc(bmwidth + skewwidth, bmheight, BITMAP_FORMAT_ARGB32);
	fillbitmap(tempbitmap, MAKE_ARGB(0xff, 0x00, 0x00, 0x00), NULL);

	/* top-left bar */
	draw_segment_horizontal_caps(tempbitmap,
		0 + 2*segwidth/3, bmwidth/2 - segwidth/10, 0 + segwidth/2,
		segwidth, LINE_CAP_START, (pattern & (1 << 0)) ? onpen : offpen);

	/* top-right bar */
	draw_segment_horizontal_caps(tempbitmap,
		0 + bmwidth/2 + segwidth/10, bmwidth - 2*segwidth/3, 0 + segwidth/2,
		segwidth, LINE_CAP_END, (pattern & (1 << 1)) ? onpen : offpen);

	/* right-top bar */
	draw_segment_vertical(tempbitmap,
		0 + 2*segwidth/3, bmheight/2 - segwidth/3, bmwidth - segwidth/2,
		segwidth, (pattern & (1 << 2)) ? onpen : offpen);

	/* right-bottom bar */
	draw_segment_vertical(tempbitmap,
		bmheight/2 + segwidth/3, bmheight - 2*segwidth/3, bmwidth - segwidth/2,
		segwidth, (pattern & (1 << 3)) ? onpen : offpen);

	/* bottom-right bar */
	draw_segment_horizontal_caps(tempbitmap,
		0 + bmwidth/2 + segwidth/10, bmwidth - 2*segwidth/3, bmheight - segwidth/2,
		segwidth, LINE_CAP_END, (pattern & (1 << 4)) ? onpen : offpen);

	/* bottom-left bar */
	draw_segment_horizontal_caps(tempbitmap,
		0 + 2*segwidth/3, bmwidth/2 - segwidth/10, bmheight - segwidth/2,
		segwidth, LINE_CAP_START, (pattern & (1 << 5)) ? onpen : offpen);

	/* left-bottom bar */
	draw_segment_vertical(tempbitmap,
		bmheight/2 + segwidth/3, bmheight - 2*segwidth/3, 0 + segwidth/2,
		segwidth, (pattern & (1 << 6)) ? onpen : offpen);

	/* left-top bar */
	draw_segment_vertical(tempbitmap,
		0 + 2*segwidth/3, bmheight/2 - segwidth/3, 0 + segwidth/2,
		segwidth, (pattern & (1 << 7)) ? onpen : offpen);

	/* horizontal-middle-left bar */
	draw_segment_horizontal_caps(tempbitmap,
		0 + 2*segwidth/3, bmwidth/2 - segwidth/10, bmheight/2,
		segwidth, LINE_CAP_START, (pattern & (1 << 8)) ? onpen : offpen);

	/* horizontal-middle-right bar */
	draw_segment_horizontal_caps(tempbitmap,
		0 + bmwidth/2 + segwidth/10, bmwidth - 2*segwidth/3, bmheight/2,
		segwidth, LINE_CAP_END, (pattern & (1 << 9)) ? onpen : offpen);

	/* vertical-middle-top bar */
	draw_segment_vertical_caps(tempbitmap,
		0 + segwidth + segwidth/3, bmheight/2 - segwidth/2 - segwidth/3, bmwidth/2,
		segwidth, LINE_CAP_NONE, (pattern & (1 << 10)) ? onpen : offpen);

	/* vertical-middle-bottom bar */
	draw_segment_vertical_caps(tempbitmap,
		bmheight/2 + segwidth/2 + segwidth/3, bmheight - segwidth - segwidth/3, bmwidth/2,
		segwidth, LINE_CAP_NONE, (pattern & (1 << 11)) ? onpen : offpen);

	/* diagonal-left-bottom bar */
	draw_segment_diagonal_1(tempbitmap,
		0 + segwidth + segwidth/5, bmwidth/2 - segwidth/2 - segwidth/5,
		bmheight/2 + segwidth/2 + segwidth/3, bmheight - segwidth - segwidth/3,
		segwidth, (pattern & (1 << 12)) ? onpen : offpen);

	/* diagonal-left-top bar */
	draw_segment_diagonal_2(tempbitmap,
		0 + segwidth + segwidth/5, bmwidth/2 - segwidth/2 - segwidth/5,
		0 + segwidth + segwidth/3, bmheight/2 - segwidth/2 - segwidth/3,
		segwidth, (pattern & (1 << 13)) ? onpen : offpen);

	/* diagonal-right-top bar */
	draw_segment_diagonal_1(tempbitmap,
		bmwidth/2 + segwidth/2 + segwidth/5, bmwidth - segwidth - segwidth/5,
		0 + segwidth + segwidth/3, bmheight/2 - segwidth/2 - segwidth/3,
		segwidth, (pattern & (1 << 14)) ? onpen : offpen);

	/* diagonal-right-bottom bar */
	draw_segment_diagonal_2(tempbitmap,
		bmwidth/2 + segwidth/2 + segwidth/5, bmwidth - segwidth - segwidth/5,
		bmheight/2 + segwidth/2 + segwidth/3, bmheight - segwidth - segwidth/3,
		segwidth, (pattern & (1 << 15)) ? onpen : offpen);

	/* apply skew */
	apply_skew(tempbitmap, 40);

	/* resample to the target size */
	render_resample_argb_bitmap_hq(dest->base, dest->rowpixels, dest->width, dest->height, tempbitmap, NULL, color);

	bitmap_free(tempbitmap);
}


/*-------------------------------------------------
    layout_element_draw_led16segsc - draw a
    16-segment LCD with semicolon (2 extra segments)
-------------------------------------------------*/

static void layout_element_draw_led16segsc(mame_bitmap *dest, const rectangle *bounds, const render_color *color, int pattern)
{
	const rgb_t onpen = MAKE_ARGB(0xff, 0xff, 0xff, 0xff);
	const rgb_t offpen = MAKE_ARGB(0xff, 0x20, 0x20, 0x20);
	int bmwidth, bmheight, segwidth, skewwidth;
	mame_bitmap *tempbitmap;

	/* sizes for computation */
	bmwidth = 250;
	bmheight = 400;
	segwidth = 40;
	skewwidth = 40;

	/* allocate a temporary bitmap for drawing */
	tempbitmap = bitmap_alloc(bmwidth + skewwidth, bmheight + segwidth, BITMAP_FORMAT_ARGB32);
	fillbitmap(tempbitmap, MAKE_ARGB(0xff, 0x00, 0x00, 0x00), NULL);

	/* top-left bar */
	draw_segment_horizontal_caps(tempbitmap,
		0 + 2*segwidth/3, bmwidth/2 - segwidth/10, 0 + segwidth/2,
		segwidth, LINE_CAP_START, (pattern & (1 << 0)) ? onpen : offpen);

	/* top-right bar */
	draw_segment_horizontal_caps(tempbitmap,
		0 + bmwidth/2 + segwidth/10, bmwidth - 2*segwidth/3, 0 + segwidth/2,
		segwidth, LINE_CAP_END, (pattern & (1 << 1)) ? onpen : offpen);

	/* right-top bar */
	draw_segment_vertical(tempbitmap,
		0 + 2*segwidth/3, bmheight/2 - segwidth/3, bmwidth - segwidth/2,
		segwidth, (pattern & (1 << 2)) ? onpen : offpen);

	/* right-bottom bar */
	draw_segment_vertical(tempbitmap,
		bmheight/2 + segwidth/3, bmheight - 2*segwidth/3, bmwidth - segwidth/2,
		segwidth, (pattern & (1 << 3)) ? onpen : offpen);

	/* bottom-right bar */
	draw_segment_horizontal_caps(tempbitmap,
		0 + bmwidth/2 + segwidth/10, bmwidth - 2*segwidth/3, bmheight - segwidth/2,
		segwidth, LINE_CAP_END, (pattern & (1 << 4)) ? onpen : offpen);

	/* bottom-left bar */
	draw_segment_horizontal_caps(tempbitmap,
		0 + 2*segwidth/3, bmwidth/2 - segwidth/10, bmheight - segwidth/2,
		segwidth, LINE_CAP_START, (pattern & (1 << 5)) ? onpen : offpen);

	/* left-bottom bar */
	draw_segment_vertical(tempbitmap,
		bmheight/2 + segwidth/3, bmheight - 2*segwidth/3, 0 + segwidth/2,
		segwidth, (pattern & (1 << 6)) ? onpen : offpen);

	/* left-top bar */
	draw_segment_vertical(tempbitmap,
		0 + 2*segwidth/3, bmheight/2 - segwidth/3, 0 + segwidth/2,
		segwidth, (pattern & (1 << 7)) ? onpen : offpen);

	/* horizontal-middle-left bar */
	draw_segment_horizontal_caps(tempbitmap,
		0 + 2*segwidth/3, bmwidth/2 - segwidth/10, bmheight/2,
		segwidth, LINE_CAP_START, (pattern & (1 << 8)) ? onpen : offpen);

	/* horizontal-middle-right bar */
	draw_segment_horizontal_caps(tempbitmap,
		0 + bmwidth/2 + segwidth/10, bmwidth - 2*segwidth/3, bmheight/2,
		segwidth, LINE_CAP_END, (pattern & (1 << 9)) ? onpen : offpen);

	/* vertical-middle-top bar */
	draw_segment_vertical_caps(tempbitmap,
		0 + segwidth + segwidth/3, bmheight/2 - segwidth/2 - segwidth/3, bmwidth/2,
		segwidth, LINE_CAP_NONE, (pattern & (1 << 10)) ? onpen : offpen);

	/* vertical-middle-bottom bar */
	draw_segment_vertical_caps(tempbitmap,
		bmheight/2 + segwidth/2 + segwidth/3, bmheight - segwidth - segwidth/3, bmwidth/2,
		segwidth, LINE_CAP_NONE, (pattern & (1 << 11)) ? onpen : offpen);

	/* diagonal-left-bottom bar */
	draw_segment_diagonal_1(tempbitmap,
		0 + segwidth + segwidth/5, bmwidth/2 - segwidth/2 - segwidth/5,
		bmheight/2 + segwidth/2 + segwidth/3, bmheight - segwidth - segwidth/3,
		segwidth, (pattern & (1 << 12)) ? onpen : offpen);

	/* diagonal-left-top bar */
	draw_segment_diagonal_2(tempbitmap,
		0 + segwidth + segwidth/5, bmwidth/2 - segwidth/2 - segwidth/5,
		0 + segwidth + segwidth/3, bmheight/2 - segwidth/2 - segwidth/3,
		segwidth, (pattern & (1 << 13)) ? onpen : offpen);

	/* diagonal-right-top bar */
	draw_segment_diagonal_1(tempbitmap,
		bmwidth/2 + segwidth/2 + segwidth/5, bmwidth - segwidth - segwidth/5,
		0 + segwidth + segwidth/3, bmheight/2 - segwidth/2 - segwidth/3,
		segwidth, (pattern & (1 << 14)) ? onpen : offpen);

	/* diagonal-right-bottom bar */
	draw_segment_diagonal_2(tempbitmap,
		bmwidth/2 + segwidth/2 + segwidth/5, bmwidth - segwidth - segwidth/5,
		bmheight/2 + segwidth/2 + segwidth/3, bmheight - segwidth - segwidth/3,
		segwidth, (pattern & (1 << 15)) ? onpen : offpen);

	/* decimal point */
	draw_segment_decimal(tempbitmap, bmwidth + segwidth/2, bmheight - segwidth/2, segwidth, (pattern & (1 << 16)) ? onpen : offpen);

	/* comma tail */
	draw_segment_diagonal_1(tempbitmap,
		bmwidth - (segwidth/2), bmwidth + segwidth,
		bmheight - (segwidth), bmheight + segwidth*1.5,
		segwidth/2, (pattern & (1 << 17)) ? onpen : offpen);

	/* apply skew */
	apply_skew(tempbitmap, 40);

	/* resample to the target size */
	render_resample_argb_bitmap_hq(dest->base, dest->rowpixels, dest->width, dest->height, tempbitmap, NULL, color);

	bitmap_free(tempbitmap);
}



/***************************************************************************
    LAYOUT FILE PARSING
***************************************************************************/

/*-------------------------------------------------
    get_variable_value - compute the value of
    a variable in an XML attribute
-------------------------------------------------*/

static int get_variable_value(const char *string, char **outputptr)
{
	int num, den, scrnum;
	char temp[100];

	/* screen 0 parameters */
	for (scrnum = 0; scrnum < MAX_SCREENS; scrnum++)
	{
		/* native X aspect factor */
		sprintf(temp, "~scr%dnativexaspect~", scrnum);
		if (!strncmp(string, temp, strlen(temp)))
		{
			num = Machine->drv->screen[scrnum].defstate.visarea.max_x + 1 - Machine->drv->screen[scrnum].defstate.visarea.min_x;
			den = Machine->drv->screen[scrnum].defstate.visarea.max_y + 1 - Machine->drv->screen[scrnum].defstate.visarea.min_y;
			reduce_fraction(&num, &den);
			*outputptr += sprintf(*outputptr, "%d", num);
			return strlen(temp);
		}

		/* native Y aspect factor */
		sprintf(temp, "~scr%dnativeyaspect~", scrnum);
		if (!strncmp(string, temp, strlen(temp)))
		{
			num = Machine->drv->screen[scrnum].defstate.visarea.max_x + 1 - Machine->drv->screen[scrnum].defstate.visarea.min_x;
			den = Machine->drv->screen[scrnum].defstate.visarea.max_y + 1 - Machine->drv->screen[scrnum].defstate.visarea.min_y;
			reduce_fraction(&num, &den);
			*outputptr += sprintf(*outputptr, "%d", den);
			return strlen(temp);
		}

		/* native width */
		sprintf(temp, "~scr%dwidth~", scrnum);
		if (!strncmp(string, temp, strlen(temp)))
		{
			*outputptr += sprintf(*outputptr, "%d", Machine->drv->screen[scrnum].defstate.visarea.max_x + 1 - Machine->drv->screen[0].defstate.visarea.min_x);
			return strlen(temp);
		}

		/* native height */
		sprintf(temp, "~scr%dheight~", scrnum);
		if (!strncmp(string, temp, strlen(temp)))
		{
			*outputptr += sprintf(*outputptr, "%d", Machine->drv->screen[scrnum].defstate.visarea.max_y + 1 - Machine->drv->screen[0].defstate.visarea.min_y);
			return strlen(temp);
		}
	}

	/* default: copy the first character and continue */
	**outputptr = *string;
	*outputptr += 1;
	return 1;
}


/*-------------------------------------------------
    xml_get_attribute_string_with_subst - analog
    to xml_get_attribute_string but with variable
    substitution
-------------------------------------------------*/

static const char *xml_get_attribute_string_with_subst(xml_data_node *node, const char *attribute, const char *defvalue)
{
	const char *str = xml_get_attribute_string(node, attribute, NULL);
	static char buffer[1000];
	const char *s;
	char *d;

	/* if nothing, just return the default */
	if (str == NULL)
		return defvalue;

	/* if no tildes, don't worry */
	if (strchr(str, '~') == NULL)
		return str;

	/* make a copy of the string, doing substitutions along the way */
	for (s = str, d = buffer; *s != 0; )
	{
		/* if not a variable, just copy */
		if (*s != '~')
			*d++ = *s++;

		/* extract the variable */
		else
			s += get_variable_value(s, &d);
	}
	*d = 0;
	return buffer;
}


/*-------------------------------------------------
    xml_get_attribute_int_with_subst - analog
    to xml_get_attribute_int but with variable
    substitution
-------------------------------------------------*/

static int xml_get_attribute_int_with_subst(xml_data_node *node, const char *attribute, int defvalue)
{
	const char *string = xml_get_attribute_string_with_subst(node, attribute, NULL);
	int value;

	if (!string || sscanf(string, "%d", &value) != 1)
		return defvalue;
	return value;
}


/*-------------------------------------------------
    xml_get_attribute_float_with_subst - analog
    to xml_get_attribute_float but with variable
    substitution
-------------------------------------------------*/

static float xml_get_attribute_float_with_subst(xml_data_node *node, const char *attribute, float defvalue)
{
	const char *string = xml_get_attribute_string_with_subst(node, attribute, NULL);
	float value;

	if (!string || sscanf(string, "%f", &value) != 1)
		return defvalue;
	return value;
}


/*-------------------------------------------------
    layout_file_load - parse a layout XML file
    into a layout_file
-------------------------------------------------*/

layout_file *layout_file_load(const char *dirname, const char *filename)
{
	xml_data_node *rootnode, *mamelayoutnode, *elemnode, *viewnode;
	layout_element **elemnext;
	layout_view **viewnext;
	layout_file *file;
	int version;

	/* if the first character of the "file" is an open brace, assume it is an XML string */
	if (filename[0] == '<')
		rootnode = xml_string_read(filename, NULL);

	/* otherwise, assume it is a file */
	else
	{
		file_error filerr;
		mame_file *layoutfile;
		astring *fname;

		fname = astring_assemble_2(astring_alloc(), filename, ".lay");
		if (dirname != NULL)
		{
			astring_insc(fname, 0, PATH_SEPARATOR);
			astring_insc(fname, 0, dirname);
		}
		filerr = mame_fopen(SEARCHPATH_ARTWORK, astring_c(fname), OPEN_FLAG_READ, &layoutfile);
		astring_free(fname);

		if (filerr != FILERR_NONE)
			return NULL;
		rootnode = xml_file_read(mame_core_file(layoutfile), NULL);
		mame_fclose(layoutfile);
	}

	/* if unable to parse the file, just bail */
	if (rootnode == NULL)
		return NULL;

	/* allocate the layout group object first */
	file = malloc_or_die(sizeof(*file));
	memset(file, 0, sizeof(*file));

	/* find the layout node */
	mamelayoutnode = xml_get_sibling(rootnode->child, "mamelayout");
	if (mamelayoutnode == NULL)
		fatalerror("Invalid XML file: missing mamelayout node");

	/* validate the config data version */
	version = xml_get_attribute_int(mamelayoutnode, "version", 0);
	if (version != LAYOUT_VERSION)
		fatalerror("Invalid XML file: unsupported version");

	/* parse all the elements */
	file->elemlist = NULL;
	elemnext = &file->elemlist;
	for (elemnode = xml_get_sibling(mamelayoutnode->child, "element"); elemnode; elemnode = xml_get_sibling(elemnode->next, "element"))
	{
		layout_element *element = load_layout_element(elemnode, dirname);
		if (element == NULL)
			goto error;

		/* add to the end of the list */
		*elemnext = element;
		elemnext = &element->next;
	}

	/* parse all the views */
	file->viewlist = NULL;
	viewnext = &file->viewlist;
	for (viewnode = xml_get_sibling(mamelayoutnode->child, "view"); viewnode; viewnode = xml_get_sibling(viewnode->next, "view"))
	{
		layout_view *view = load_layout_view(viewnode, file->elemlist);
		if (view == NULL)
			goto error;

		/* add to the end of the list */
		*viewnext = view;
		viewnext = &view->next;
	}
	xml_file_free(rootnode);
	return file;

error:
	layout_file_free(file);
	xml_file_free(rootnode);
	return NULL;
}


/*-------------------------------------------------
    load_layout_element - parse an element XML
    node from the layout file
-------------------------------------------------*/

static layout_element *load_layout_element(xml_data_node *elemnode, const char *dirname)
{
	render_bounds bounds = { 0 };
	element_component **nextcomp;
	element_component *component;
	xml_data_node *compnode;
	layout_element *element;
	float xscale, yscale;
	float xoffs, yoffs;
	const char *name;
	int state;
	int first;

	/* allocate a new element */
	element = malloc_or_die(sizeof(*element));
	memset(element, 0, sizeof(*element));

	/* extract the name */
	name = xml_get_attribute_string_with_subst(elemnode, "name", NULL);
	if (name == NULL)
	{
		logerror("All layout elements must have a name!\n");
		goto error;
	}
	element->name = copy_string(name);
	element->defstate = xml_get_attribute_int_with_subst(elemnode, "defstate", -1);

	/* parse components in order */
	first = TRUE;
	nextcomp = &element->complist;
	for (compnode = elemnode->child; compnode; compnode = compnode->next)
	{
		/* allocate a new component */
		element_component *component = load_element_component(compnode, dirname);
		if (component == NULL)
			goto error;

		/* link it into the list */
		*nextcomp = component;
		nextcomp = &component->next;

		/* accumulate bounds */
		if (first)
			bounds = component->bounds;
		else
			union_render_bounds(&bounds, &component->bounds);
		first = FALSE;

		/* determine the maximum state */
		if (component->state > element->maxstate)
			element->maxstate = component->state;
		if (component->type == COMPONENT_TYPE_LED7SEG)
			element->maxstate = 255;
		if (component->type == COMPONENT_TYPE_LED14SEG)
			element->maxstate = 16383;
		if (component->type == COMPONENT_TYPE_LED14SEGSC || component->type == COMPONENT_TYPE_LED16SEG)
			element->maxstate = 65535;
		if (component->type == COMPONENT_TYPE_LED16SEGSC)
			element->maxstate = 262143;
	}

	/* determine the scale/offset for normalization */
	xoffs = bounds.x0;
	yoffs = bounds.y0;
	xscale = 1.0f / (bounds.x1 - bounds.x0);
	yscale = 1.0f / (bounds.y1 - bounds.y0);

	/* normalize all the component bounds */
	for (component = element->complist; component != NULL; component = component->next)
	{
		component->bounds.x0 = (component->bounds.x0 - xoffs) * xscale;
		component->bounds.x1 = (component->bounds.x1 - xoffs) * xscale;
		component->bounds.y0 = (component->bounds.y0 - yoffs) * yscale;
		component->bounds.y1 = (component->bounds.y1 - yoffs) * yscale;
	}

	/* allocate an array of element textures for the states */
	element->elemtex = malloc_or_die((element->maxstate + 1) * sizeof(element->elemtex[0]));
	for (state = 0; state <= element->maxstate; state++)
	{
		element->elemtex[state].element = element;
		element->elemtex[state].state = state;
		element->elemtex[state].texture = render_texture_alloc(layout_element_scale, &element->elemtex[state]);
	}

	return element;

error:
	layout_element_free(element);
	return NULL;
}


/*-------------------------------------------------
    load_element_component - parse a component
    XML node (image/rect/disk)
-------------------------------------------------*/

static element_component *load_element_component(xml_data_node *compnode, const char *dirname)
{
	element_component *component;

	/* allocate memory for the component */
	component = malloc_or_die(sizeof(*component));
	memset(component, 0, sizeof(*component));

	/* fetch common data */
	component->state = xml_get_attribute_int_with_subst(compnode, "state", -1);
	if (load_bounds(xml_get_sibling(compnode->child, "bounds"), &component->bounds))
		goto error;
	if (load_color(xml_get_sibling(compnode->child, "color"), &component->color))
		goto error;

	/* image nodes */
	if (strcmp(compnode->name, "image") == 0)
	{
		const char *file = xml_get_attribute_string_with_subst(compnode, "file", NULL);
		const char *afile = xml_get_attribute_string_with_subst(compnode, "alphafile", NULL);

		/* load and allocate the bitmap */
		component->type = COMPONENT_TYPE_IMAGE;
		component->dirname = (dirname == NULL) ? NULL : copy_string(dirname);
		component->imagefile = (file == NULL) ? NULL : copy_string(file);
		component->alphafile = (afile == NULL) ? NULL : copy_string(afile);
	}

	/* text nodes */
	else if (strcmp(compnode->name, "text") == 0)
	{
		const char *text = xml_get_attribute_string_with_subst(compnode, "string", "");
		char *string;

		/* allocate a copy of the string */
		component->type = COMPONENT_TYPE_TEXT;
		string = malloc_or_die(strlen(text) + 1);
		strcpy(string, text);
		component->string = string;
	}

	/* led7seg nodes */
	else if (strcmp(compnode->name, "led7seg") == 0)
		component->type = COMPONENT_TYPE_LED7SEG;

	/* led14seg nodes */
	else if (strcmp(compnode->name, "led14seg") == 0)
		component->type = COMPONENT_TYPE_LED14SEG;

	/* led14segsc nodes */
	else if (strcmp(compnode->name, "led14segsc") == 0)
		component->type = COMPONENT_TYPE_LED14SEGSC;

	/* led16seg nodes */
	else if (strcmp(compnode->name, "led16seg") == 0)
		component->type = COMPONENT_TYPE_LED16SEG;

	/* led16segsc nodes */
	else if (strcmp(compnode->name, "led16segsc") == 0)
		component->type = COMPONENT_TYPE_LED16SEGSC;

	/* rect nodes */
	else if (strcmp(compnode->name, "rect") == 0)
		component->type = COMPONENT_TYPE_RECT;

	/* disk nodes */
	else if (strcmp(compnode->name, "disk") == 0)
		component->type = COMPONENT_TYPE_DISK;

	/* error otherwise */
	else
		fatalerror("Unknown element component: %s", compnode->name);

	return component;

error:
	free(component);
	return NULL;
}


/*-------------------------------------------------
    load_layout_view - parse a view XML node
-------------------------------------------------*/

static layout_view *load_layout_view(xml_data_node *viewnode, layout_element *elemlist)
{
	xml_data_node *boundsnode;
	view_item **itemnext;
	layout_view *view;
	int layer;

	/* first allocate memory */
	view = malloc_or_die(sizeof(*view));
	memset(view, 0, sizeof(*view));

	/* allocate a copy of the name */
	view->name = copy_string(xml_get_attribute_string_with_subst(viewnode, "name", ""));

	/* if we have a bounds item, load it */
	boundsnode = xml_get_sibling(viewnode->child, "bounds");
	if (boundsnode != NULL && load_bounds(xml_get_sibling(boundsnode, "bounds"), &view->expbounds))
		goto error;

	/* loop over all the layer types we support */
	for (layer = 0; layer < ITEM_LAYER_MAX; layer++)
	{
		static const char *layer_node_name[ITEM_LAYER_MAX] = { "backdrop", "screen", "overlay", "bezel" };
		xml_data_node *itemnode;

		/* initialize the list */
		view->itemlist[layer] = NULL;
		itemnext = &view->itemlist[layer];

		/* parse all of the elements of that type */
		for (itemnode = xml_get_sibling(viewnode->child, layer_node_name[layer]); itemnode; itemnode = xml_get_sibling(itemnode->next, layer_node_name[layer]))
		{
			view_item *item = load_view_item(itemnode, elemlist);
			if (!item)
				goto error;

			/* add to the end of the list */
			*itemnext = item;
			itemnext = &item->next;
		}
	}

	/* recompute the data for the view */
	layout_view_recompute(view, ~0);
	return view;

error:
	layout_view_free(view);
	return NULL;
}


/*-------------------------------------------------
    load_view_item - parse an item XML node
-------------------------------------------------*/

static view_item *load_view_item(xml_data_node *itemnode, layout_element *elemlist)
{
	view_item *item;
	const char *name;

	/* allocate a new item */
	item = malloc_or_die(sizeof(*item));
	memset(item, 0, sizeof(*item));

	/* allocate a copy of the name */
	item->name = copy_string(xml_get_attribute_string_with_subst(itemnode, "name", ""));

	/* find the associated element */
	name = xml_get_attribute_string_with_subst(itemnode, "element", NULL);
	if (name != NULL)
	{
		layout_element *element;

		/* search the list of elements for a match */
		for (element = elemlist; element; element = element->next)
			if (strcmp(name, element->name) == 0)
				break;

		/* error if not found */
		if (element == NULL)
			fatalerror("Unable to find layout element %s", name);
		item->element = element;
	}

	/* fetch common data */
	item->index = xml_get_attribute_int_with_subst(itemnode, "index", -1);
	if (item->name[0] != 0 && item->element != 0)
		output_set_value(item->name, item->element->defstate);
	if (load_bounds(xml_get_sibling(itemnode->child, "bounds"), &item->rawbounds))
		goto error;
	if (load_color(xml_get_sibling(itemnode->child, "color"), &item->color))
		goto error;
	if (load_orientation(xml_get_sibling(itemnode->child, "orientation"), &item->orientation))
		goto error;

	/* sanity checks */
	if (strcmp(itemnode->name, "screen") == 0)
	{
		if (item->index >= MAX_SCREENS)
			fatalerror("Layout references invalid screen index %d", item->index);
	}
	else
	{
		if (item->element == NULL)
			fatalerror("Layout item of type %s require an element tag", itemnode->name);
	}

	return item;

error:
	if (item->name != NULL)
		free((void *)item->name);
	free(item);
	return NULL;
}


/*-------------------------------------------------
    load_component_bitmap - load a PNG file
    with artwork for a component
-------------------------------------------------*/

static mame_bitmap *load_component_bitmap(const char *dirname, const char *file, const char *alphafile, int *hasalpha)
{
	mame_bitmap *bitmap;

	/* load the basic bitmap */
	bitmap = render_load_png(dirname, file, NULL, hasalpha);
	if (bitmap != NULL && alphafile != NULL)

		/* load the alpha bitmap if specified */
		if (render_load_png(dirname, alphafile, bitmap, hasalpha) == NULL)
		{
			bitmap_free(bitmap);
			bitmap = NULL;
		}

	/* if we can't load the bitmap, allocate a dummy one and report an error */
	if (bitmap == NULL)
	{
		int step, line;

		/* draw some stripes in the bitmap */
		bitmap = bitmap_alloc(100, 100, BITMAP_FORMAT_ARGB32);
		fillbitmap(bitmap, 0, NULL);
		for (step = 0; step < 100; step += 25)
			for (line = 0; line < 100; line++)
				*BITMAP_ADDR32(bitmap, (step + line) % 100, line % 100) = MAKE_ARGB(0xff,0xff,0xff,0xff);

		/* log an error */
		if (alphafile == NULL)
			logerror("Unable to load component bitmap '%s'", file);
		else
			logerror("Unable to load component bitmap '%s'/'%s'", file, alphafile);
	}

	return bitmap;
}


/*-------------------------------------------------
    load_bounds - parse a bounds XML node
-------------------------------------------------*/

static int load_bounds(xml_data_node *boundsnode, render_bounds *bounds)
{
	/* skip if nothing */
	if (boundsnode == NULL)
	{
		bounds->x0 = bounds->y0 = 0.0f;
		bounds->x1 = bounds->y1 = 1.0f;
		return 0;
	}

	/* parse out the data */
	if (xml_get_attribute(boundsnode, "left") != NULL)
	{
		/* left/right/top/bottom format */
		bounds->x0 = xml_get_attribute_float_with_subst(boundsnode, "left", 0.0);
		bounds->x1 = xml_get_attribute_float_with_subst(boundsnode, "right", 1.0);
		bounds->y0 = xml_get_attribute_float_with_subst(boundsnode, "top", 0.0);
		bounds->y1 = xml_get_attribute_float_with_subst(boundsnode, "bottom", 1.0);
	}
	else if (xml_get_attribute(boundsnode, "x") != NULL)
	{
		/* x/y/width/height format */
		bounds->x0 = xml_get_attribute_float_with_subst(boundsnode, "x", 0.0);
		bounds->x1 = bounds->x0 + xml_get_attribute_float_with_subst(boundsnode, "width", 1.0);
		bounds->y0 = xml_get_attribute_float_with_subst(boundsnode, "y", 0.0);
		bounds->y1 = bounds->y0 + xml_get_attribute_float_with_subst(boundsnode, "height", 1.0);
	}
	else
	{
		fatalerror("Illegal bounds value in XML");
		return 1;
	}

	/* check for errors */
	if (bounds->x0 > bounds->x1 || bounds->y0 > bounds->y1)
	{
		fatalerror("Illegal bounds value in XML: (%f-%f)-(%f-%f)", bounds->x0, bounds->x1, bounds->y0, bounds->y1);
		return 1;
	}
	return 0;
}


/*-------------------------------------------------
    load_color - parse a color XML node
-------------------------------------------------*/

static int load_color(xml_data_node *colornode, render_color *color)
{
	/* skip if nothing */
	if (colornode == NULL)
	{
		color->r = color->g = color->b = color->a = 1.0f;
		return 0;
	}

	/* parse out the data */
	color->r = xml_get_attribute_float_with_subst(colornode, "red", 1.0);
	color->g = xml_get_attribute_float_with_subst(colornode, "green", 1.0);
	color->b = xml_get_attribute_float_with_subst(colornode, "blue", 1.0);
	color->a = xml_get_attribute_float_with_subst(colornode, "alpha", 1.0);

	/* check for errors */
	if (color->r < 0.0 || color->r > 1.0 || color->g < 0.0 || color->g > 1.0 ||
		color->b < 0.0 || color->b > 1.0 || color->a < 0.0 || color->a > 1.0)
	{
		fatalerror("Illegal ARGB color value in XML: %f,%f,%f,%f", color->r, color->g, color->b, color->a);
		return 1;
	}
	return 0;
}


/*-------------------------------------------------
    load_orientation - parse an orientation XML
    node
-------------------------------------------------*/

static int load_orientation(xml_data_node *orientnode, int *orientation)
{
	int rotate;

	/* skip if nothing */
	if (orientnode == NULL)
	{
		*orientation = ROT0;
		return 0;
	}

	/* parse out the data */
	rotate = xml_get_attribute_int_with_subst(orientnode, "rotate", 0);
	switch (rotate)
	{
		case 0:		*orientation = ROT0;	break;
		case 90:	*orientation = ROT90;	break;
		case 180:	*orientation = ROT180;	break;
		case 270:	*orientation = ROT270;	break;
		default:
			fatalerror("Invalid rotation in XML orientation node: %d", rotate);
			return 1;
	}
	if (strcmp("yes", xml_get_attribute_string_with_subst(orientnode, "swapxy", "no")) == 0)
		*orientation ^= ORIENTATION_SWAP_XY;
	if (strcmp("yes", xml_get_attribute_string_with_subst(orientnode, "flipx", "no")) == 0)
		*orientation ^= ORIENTATION_FLIP_X;
	if (strcmp("yes", xml_get_attribute_string_with_subst(orientnode, "flipy", "no")) == 0)
		*orientation ^= ORIENTATION_FLIP_Y;
	return 0;
}


/*-------------------------------------------------
    layout_file_free - free memory for a
    layout_file and all of its subelements
-------------------------------------------------*/

void layout_file_free(layout_file *file)
{
	/* free each element in the list */
	while (file->elemlist != NULL)
	{
		layout_element *temp = file->elemlist;
		file->elemlist = temp->next;
		layout_element_free(temp);
	}

	/* free each layout */
	while (file->viewlist != NULL)
	{
		layout_view *temp = file->viewlist;
		file->viewlist = temp->next;
		layout_view_free(temp);
	}

	/* free the file itself */
	free(file);
}


/*-------------------------------------------------
    layout_view_free - free memory for a
    layout_view and all of its subelements
-------------------------------------------------*/

static void layout_view_free(layout_view *view)
{
	int layer;

	/* for each layer, free each item in that layer */
	for (layer = 0; layer < ITEM_LAYER_MAX; layer++)
		while (view->itemlist[layer] != NULL)
		{
			view_item *temp = view->itemlist[layer];
			view->itemlist[layer] = temp->next;
			if (temp->name != NULL)
				free((void *)temp->name);
			free(temp);
		}

	/* free the view itself */
	if (view->name != NULL)
		free((void *)view->name);
	free(view);
}


/*-------------------------------------------------
    layout_element_free - free memory for a
    layout_element and its components
-------------------------------------------------*/

static void layout_element_free(layout_element *element)
{
	/* free all allocated components */
	while (element->complist != NULL)
	{
		element_component *temp = element->complist;
		element->complist = temp->next;
		if (temp->string != NULL)
			free((void *)temp->string);
		if (temp->dirname != NULL)
			free((void *)temp->dirname);
		if (temp->imagefile != NULL)
			free((void *)temp->imagefile);
		if (temp->alphafile != NULL)
			free((void *)temp->alphafile);
		if (temp->bitmap != NULL)
			bitmap_free(temp->bitmap);
		free(temp);
	}

	/* free all textures */
	if (element->elemtex != NULL)
	{
		int state;

		/* loop over all states and free their textures */
		for (state = 0; state <= element->maxstate; state++)
			if (element->elemtex[state].texture != NULL)
				render_texture_free(element->elemtex[state].texture);

		free(element->elemtex);
	}

	/* free the element itself */
	if (element->name != NULL)
		free((void *)element->name);
	free(element);
}
