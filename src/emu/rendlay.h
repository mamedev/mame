/***************************************************************************

    rendlay.h

    Core rendering layout parser and manager.

    Copyright (c) 1996-2007, Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

***************************************************************************/

#ifndef __RENDLAY_H__
#define __RENDLAY_H__

#include "driver.h"
#include "osdepend.h"
#include "render.h"

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

enum
{
	ITEM_LAYER_BACKDROP = 0,
	ITEM_LAYER_SCREEN,
	ITEM_LAYER_OVERLAY,
	ITEM_LAYER_BEZEL,
	ITEM_LAYER_MAX
};



/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

typedef struct _element_component element_component;
typedef struct _element_texture element_texture;
typedef struct _layout_element layout_element;
typedef struct _view_item_state view_item_state;
typedef struct _view_item view_item;
typedef struct _layout_view layout_view;
typedef struct _layout_file layout_file;

/* an element_texture encapsulates a texture for a given element in a given state */
struct _element_texture
{
	layout_element *	element;			/* pointer back to the element */
	render_texture *	texture;			/* texture for this state */
	int					state;				/* associated state number */
};


/* a layout_element is a single named element, which may have multiple components */
struct _layout_element
{
	layout_element *	next;				/* link to next element */
	const char *		name;				/* name of this element */
	element_component *	complist;			/* head of the list of components */
	int					defstate;			/* default state of this element */
	int					maxstate;			/* maximum state value for all components */
	element_texture *	elemtex;			/* array of element textures used for managing the scaled bitmaps */
};


/* a view_item_state contains the string-tagged state of a view item */
struct _view_item_state
{
	view_item_state *	next;				/* pointer to the next one */
	const char *		name;				/* string that was set */
	int					curstate;			/* current state */
};


/* a view_item is a single backdrop, screen, overlay, or bezel item */
struct _view_item
{
	view_item *			next;				/* link to next item */
	layout_element *	element;			/* pointer to the associated element (non-screens only) */
	const char *		name;				/* name of this item */
	int					index;				/* index for this item (screens only) */
	int					orientation;		/* orientation of this item */
	render_bounds		bounds;				/* bounds of the item */
	render_bounds		rawbounds;			/* raw (original) bounds of the item */
	render_color		color;				/* color of the item */
};


/* a layout_view encapsulates a named list of items */
struct _layout_view
{
	layout_view *		next;				/* pointer to next layout in the list */
	const char *		name;				/* name of the layout */
	float				aspect;				/* X/Y of the layout */
	float				scraspect;			/* X/Y of the screen areas */
	UINT32				screens;			/* bitmask of screens used */
	render_bounds		bounds;				/* computed bounds of the view */
	render_bounds		scrbounds;			/* computed bounds of the screens within the view */
	render_bounds		expbounds;			/* explicit bounds of the view */
	UINT8				layenabled[ITEM_LAYER_MAX]; /* is this layer enabled? */
	view_item *			itemlist[ITEM_LAYER_MAX]; /* list of layout items for each layer */
};


/* a layout_file consists of a list of elements and a list of views */
struct _layout_file
{
	layout_file *		next;				/* pointer to the next file in the list */
	layout_element *	elemlist;			/* list of shared layout elements */
	layout_view *		viewlist;			/* list of views */
};



/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

/* ----- layout views ----- */
void layout_view_recompute(layout_view *view, int layerconfig);


/* ----- layout file parsing ----- */
layout_file *layout_file_load(const char *dirname, const char *filename);
void layout_file_free(layout_file *file);



/***************************************************************************
    GLOBAL VARIABLES
***************************************************************************/

/* single screen layouts */
extern const char layout_horizont[];	/* horizontal 4:3 screens */
extern const char layout_vertical[];	/* vertical 4:3 screens */

/* dual screen layouts */
extern const char layout_dualhsxs[];	/* dual 4:3 screens side-by-side */
extern const char layout_dualhovu[];	/* dual 4:3 screens above and below */
extern const char layout_dualhuov[];	/* dual 4:3 screens below and above */

/* triple screen layouts */
extern const char layout_triphsxs[];	/* triple 4:3 screens side-by-side */

/* generic color overlay layouts */
extern const char layout_ho20ffff[];	/* horizontal 4:3 with 20,FF,FF color overlay */
extern const char layout_ho2eff2e[];	/* horizontal 4:3 with 2E,FF,2E color overlay */
extern const char layout_ho4f893d[];	/* horizontal 4:3 with 4F,89,3D color overlay */
extern const char layout_ho88ffff[];	/* horizontal 4:3 with 88,FF,FF color overlay */
extern const char layout_hoa0a0ff[];	/* horizontal 4:3 with A0,A0,FF color overlay */
extern const char layout_hoffe457[];	/* horizontal 4:3 with FF,E4,57 color overlay */
extern const char layout_hoffff20[];	/* horizontal 4:3 with FF,FF,20 color overlay */
extern const char layout_voffff20[];	/* vertical 4:3 with FF,FF,20 color overlay */



/***************************************************************************
    INLINE FUNCTIONS
***************************************************************************/

/*-------------------------------------------------
    layout_view_has_art - true if a render_view contains
    any non-screen elements
-------------------------------------------------*/

INLINE int layout_view_has_art(layout_view *view)
{
	return (view->itemlist[ITEM_LAYER_BACKDROP] != 0 ||
			view->itemlist[ITEM_LAYER_OVERLAY] != 0 ||
			view->itemlist[ITEM_LAYER_BEZEL] != 0);
}



#endif	/* __RENDLAY_H__ */
