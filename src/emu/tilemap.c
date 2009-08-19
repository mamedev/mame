/***************************************************************************

    tilemap.c

    Generic tilemap management system.

    Copyright Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

***************************************************************************/

#include "driver.h"
#include "tilemap.h"
#include "profiler.h"


/***************************************************************************
    CONSTANTS
***************************************************************************/

/* internal usage to mark tiles dirty */
#define TILE_FLAG_DIRTY					0xff

/* invalid logical index */
#define INVALID_LOGICAL_INDEX			((tilemap_logical_index)~0)

/* maximum index in each array */
#define MAX_PEN_TO_FLAGS				256


/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

/* logical index */
typedef UINT32 tilemap_logical_index;


/* internal set of transparency states for rendering */
typedef enum
{
	WHOLLY_TRANSPARENT,
	WHOLLY_OPAQUE,
	MASKED
} trans_t;


/* internal blitting callbacks */
typedef void (*blitmask_func)(void *dest, const UINT16 *source, const UINT8 *maskptr, int mask, int value, int count, const pen_t *pens, UINT8 *pri, UINT32 pcode, UINT8 alpha);
typedef void (*blitopaque_func)(void *dest, const UINT16 *source, int count, const pen_t *pens, UINT8 *pri, UINT32 pcode, UINT8 alpha);


/* blitting parameters for rendering */
typedef struct _blit_parameters blit_parameters;
struct _blit_parameters
{
	bitmap_t *			bitmap;
	rectangle			cliprect;
	blitmask_func 		draw_masked;
	blitopaque_func		draw_opaque;
	UINT32 				tilemap_priority_code;
	UINT8				mask;
	UINT8				value;
	UINT8				alpha;
};


/* core tilemap structure */
struct _tilemap
{
	tilemap *					next;				/* pointer to next tilemap */
	running_machine *			machine;			/* pointer back to the owning machine */

	/* basic tilemap metrics */
	UINT32						rows;				/* number of tile rows */
	UINT32						cols;				/* number of tile columns */
	UINT32						tilewidth;			/* width of a single tile in pixels */
	UINT32						tileheight;			/* height of a single tile in pixels */
	UINT32						width;				/* width of the full tilemap in pixels */
	UINT32						height;				/* height of the full tilemap in pixels */

	/* logical <-> memory mappings */
	tilemap_mapper_func			mapper;				/* callback to map a row/column to a memory index */
	tilemap_logical_index *		memory_to_logical;	/* map from memory index to logical index */
	tilemap_logical_index		max_logical_index;	/* maximum valid logical index */
	tilemap_memory_index *		logical_to_memory;	/* map from logical index to memory index */
	tilemap_memory_index		max_memory_index;	/* maximum valid memory index */

	/* callback to interpret video RAM for the tilemap */
	tile_get_info_func			tile_get_info;		/* callback to get information about a tile */
	tile_data					tileinfo;			/* structure to hold the data for a tile */
	void *						user_data;			/* user data value passed to the callback */

	/* global tilemap states */
	UINT8						enable;				/* true if we are enabled */
	UINT8						attributes;			/* global attributes (flipx/y) */
	UINT8						all_tiles_dirty;	/* true if all tiles are dirty */
	UINT8						all_tiles_clean;	/* true if all tiles are clean */
	UINT32						palette_offset;		/* palette offset */
	UINT32						pen_data_offset;	/* pen data offset */
	UINT32						gfx_used;			/* bitmask of gfx items used */
	UINT32						gfx_dirtyseq[MAX_GFX_ELEMENTS]; /* dirtyseq values from last check */

	/* scroll information */
	UINT32						scrollrows;			/* number of independently scrolled rows */
	UINT32						scrollcols;			/* number of independently scrolled colums */
	INT32 *						rowscroll;			/* array of rowscroll values */
	INT32 *						colscroll;			/* array of colscroll values */
	INT32 						dx;					/* global horizontal scroll offset */
	INT32						dx_flipped;			/* global horizontal scroll offset when flipped */
	INT32 						dy;					/* global vertical scroll offset */
	INT32						dy_flipped;			/* global vertical scroll offset when flipped */

	/* pixel data */
	bitmap_t *					pixmap;				/* cached pixel data */

	/* transparency mapping */
	bitmap_t *					flagsmap;			/* per-pixel flags */
	UINT8 *						tileflags;			/* per-tile flags */
	UINT8 *						pen_to_flags; 		/* mapping of pens to flags */
};


struct _tilemap_private
{
	tilemap *		list;
	tilemap **		tailptr;
	int				instance;
};



/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

/* system management helpers */
static void tilemap_exit(running_machine *machine);
static STATE_POSTLOAD( tilemap_postload );
static void tilemap_dispose(tilemap *tmap);

/* logical <-> memory index mapping */
static void mappings_create(tilemap *tmap);
static void mappings_update(tilemap *tmap);

/* tile rendering */
static void pixmap_update(tilemap *tmap, const rectangle *cliprect);
static void tile_update(tilemap *tmap, tilemap_logical_index logindex, UINT32 cached_col, UINT32 cached_row);
static UINT8 tile_draw(tilemap *tmap, const UINT8 *pendata, UINT32 x0, UINT32 y0, UINT32 palette_base, UINT8 category, UINT8 group, UINT8 flags, UINT8 pen_mask);
static UINT8 tile_apply_bitmask(tilemap *tmap, const UINT8 *maskdata, UINT32 x0, UINT32 y0, UINT8 category, UINT8 flags);

/* drawing helpers */
static void configure_blit_parameters(blit_parameters *blit, tilemap *tmap, bitmap_t *dest, const rectangle *cliprect, UINT32 flags, UINT8 priority, UINT8 priority_mask);
static void tilemap_draw_instance(tilemap *tmap, const blit_parameters *blit, int xpos, int ypos);
static void tilemap_draw_roz_core(tilemap *tmap, const blit_parameters *blit,
		UINT32 startx, UINT32 starty, int incxx, int incxy, int incyx, int incyy, int wraparound);

/* scanline rasterizers for drawing to the pixmap */
static void scanline_draw_opaque_null(void *dest, const UINT16 *source, int count, const pen_t *pens, UINT8 *pri, UINT32 pcode, UINT8 alpha);
static void scanline_draw_masked_null(void *dest, const UINT16 *source, const UINT8 *maskptr, int mask, int value, int count, const pen_t *pens, UINT8 *pri, UINT32 pcode, UINT8 alpha);
static void scanline_draw_opaque_ind16(void *dest, const UINT16 *source, int count, const pen_t *pens, UINT8 *pri, UINT32 pcode, UINT8 alpha);
static void scanline_draw_masked_ind16(void *dest, const UINT16 *source, const UINT8 *maskptr, int mask, int value, int count, const pen_t *pens, UINT8 *pri, UINT32 pcode, UINT8 alpha);
static void scanline_draw_opaque_rgb16(void *dest, const UINT16 *source, int count, const pen_t *pens, UINT8 *pri, UINT32 pcode, UINT8 alpha);
static void scanline_draw_masked_rgb16(void *dest, const UINT16 *source, const UINT8 *maskptr, int mask, int value, int count, const pen_t *pens, UINT8 *pri, UINT32 pcode, UINT8 alpha);
static void scanline_draw_opaque_rgb16_alpha(void *dest, const UINT16 *source, int count, const pen_t *pens, UINT8 *pri, UINT32 pcode, UINT8 alpha);
static void scanline_draw_masked_rgb16_alpha(void *dest, const UINT16 *source, const UINT8 *maskptr, int mask, int value, int count, const pen_t *pens, UINT8 *pri, UINT32 pcode, UINT8 alpha);
static void scanline_draw_opaque_rgb32(void *dest, const UINT16 *source, int count, const pen_t *pens, UINT8 *pri, UINT32 pcode, UINT8 alpha);
static void scanline_draw_masked_rgb32(void *dest, const UINT16 *source, const UINT8 *maskptr, int mask, int value, int count, const pen_t *pens, UINT8 *pri, UINT32 pcode, UINT8 alpha);
static void scanline_draw_opaque_rgb32_alpha(void *dest, const UINT16 *source, int count, const pen_t *pens, UINT8 *pri, UINT32 pcode, UINT8 alpha);
static void scanline_draw_masked_rgb32_alpha(void *dest, const UINT16 *source, const UINT8 *maskptr, int mask, int value, int count, const pen_t *pens, UINT8 *pri, UINT32 pcode, UINT8 alpha);



/***************************************************************************
    INLINE FUNCTIONS
***************************************************************************/

/*-------------------------------------------------
    effective_rowscroll - return the effective
    rowscroll value for a given index, taking into
    account tilemap flip states
-------------------------------------------------*/

INLINE INT32 effective_rowscroll(tilemap *tmap, int index, UINT32 screen_width)
{
	INT32 value;

	/* if we're flipping vertically, adjust the row number */
	if (tmap->attributes & TILEMAP_FLIPY)
		index = tmap->scrollrows - 1 - index;

	/* adjust final result based on the horizontal flip and dx values */
	if (!(tmap->attributes & TILEMAP_FLIPX))
		value = tmap->dx - tmap->rowscroll[index];
	else
		value = screen_width - tmap->width - (tmap->dx_flipped - tmap->rowscroll[index]);

	/* clamp to 0..width */
	if (value < 0)
		value = tmap->width - (-value) % tmap->width;
	else
		value %= tmap->width;
	return value;
}


/*-------------------------------------------------
    effective_colscroll - return the effective
    colscroll value for a given index, taking into
    account tilemap flip states
-------------------------------------------------*/

INLINE INT32 effective_colscroll(tilemap *tmap, int index, UINT32 screen_height)
{
	INT32 value;

	/* if we're flipping horizontally, adjust the column number */
	if (tmap->attributes & TILEMAP_FLIPX)
		index = tmap->scrollcols - 1 - index;

	/* adjust final result based on the vertical flip and dx values */
	if (!(tmap->attributes & TILEMAP_FLIPY))
		value = tmap->dy - tmap->colscroll[index];
	else
		value = screen_height - tmap->height - (tmap->dy_flipped - tmap->colscroll[index]);

	/* clamp to 0..height */
	if (value < 0)
		value = tmap->height - (-value) % tmap->height;
	else
		value %= tmap->height;
	return value;
}


/*-------------------------------------------------
    indexed_tilemap - return a tilemap by index
-------------------------------------------------*/

INLINE tilemap *indexed_tilemap(running_machine *machine, int index)
{
	tilemap *tmap;

	/* find by the tilemap index */
	for (tmap = machine->tilemap_data->list; tmap != NULL; tmap = tmap->next)
		if (index-- == 0)
			return tmap;

	return NULL;
}


/*-------------------------------------------------
    gfx_tiles_changed - return TRUE if any
    gfx_elements used by this tilemap have
    changed
-------------------------------------------------*/

INLINE int gfx_elements_changed(tilemap *tmap)
{
	UINT32 usedmask = tmap->gfx_used;
	int isdirty = FALSE;
	int gfxnum;

	/* iterate over all used gfx types and set the dirty flag if any of them have changed */
	for (gfxnum = 0; usedmask != 0; usedmask >>= 1, gfxnum++)
		if ((usedmask & 1) != 0)
			if (tmap->gfx_dirtyseq[gfxnum] != tmap->machine->gfx[gfxnum]->dirtyseq)
			{
				tmap->gfx_dirtyseq[gfxnum] = tmap->machine->gfx[gfxnum]->dirtyseq;
				isdirty = TRUE;
			}

	return isdirty;
}


/***************************************************************************
    SYSTEM-WIDE MANAGEMENT
***************************************************************************/

/*-------------------------------------------------
    tilemap_init - initialize the tilemap system
-------------------------------------------------*/

void tilemap_init(running_machine *machine)
{
	UINT32 screen_width, screen_height;

	if (machine->primary_screen == NULL)
		return;

	screen_width  = video_screen_get_width(machine->primary_screen);
	screen_height = video_screen_get_height(machine->primary_screen);

	if (screen_width != 0 && screen_height != 0)
	{
		machine->tilemap_data = auto_alloc_clear(machine, tilemap_private);
		machine->tilemap_data->tailptr = &machine->tilemap_data->list;

		machine->priority_bitmap = auto_bitmap_alloc(machine, screen_width, screen_height, BITMAP_FORMAT_INDEXED8);
		add_exit_callback(machine, tilemap_exit);
	}
}



/***************************************************************************
    TILEMAP CREATION AND CONFIGURATION
***************************************************************************/

/*-------------------------------------------------
    tilemap_create - allocate a new tilemap
-------------------------------------------------*/

tilemap *tilemap_create(running_machine *machine, tile_get_info_func tile_get_info, tilemap_mapper_func mapper, int tilewidth, int tileheight, int cols, int rows)
{
	tilemap *tmap;
	int tilemap_instance = machine->tilemap_data->instance;
	int group;

	/* allocate the tilemap itself */
	tmap = alloc_clear_or_die(tilemap);

	/* fill in the basic metrics */
	tmap->machine = machine;
	tmap->rows = rows;
	tmap->cols = cols;
	tmap->tilewidth = tilewidth;
	tmap->tileheight = tileheight;
	tmap->width = cols * tilewidth;
	tmap->height = rows * tileheight;

	/* set up the logical <-> memory mappings */
	tmap->mapper = mapper;
	mappings_create(tmap);

	/* set up the tile map callbacks */
	tmap->tile_get_info = tile_get_info;

	/* set up the default pen mask */
	tmap->tileinfo.pen_mask = 0xff;
	tmap->tileinfo.gfxnum = 0xff;

	/* initialize global states */
	tmap->enable = TRUE;
	tmap->all_tiles_dirty = TRUE;

	/* initialize scroll information */
	tmap->scrollrows = 1;
	tmap->scrollcols = 1;
	tmap->rowscroll = alloc_array_clear_or_die(INT32, tmap->height);
	tmap->colscroll = alloc_array_clear_or_die(INT32, tmap->width);

	/* allocate the pixel data cache */
	tmap->pixmap = bitmap_alloc(tmap->width, tmap->height, BITMAP_FORMAT_INDEXED16);

	/* allocate transparency mapping data */
	tmap->tileflags = alloc_array_or_die(UINT8, tmap->max_logical_index);
	tmap->flagsmap = bitmap_alloc(tmap->width, tmap->height, BITMAP_FORMAT_INDEXED8);
	tmap->pen_to_flags = alloc_array_clear_or_die(UINT8, MAX_PEN_TO_FLAGS * TILEMAP_NUM_GROUPS);
	for (group = 0; group < TILEMAP_NUM_GROUPS; group++)
		tilemap_map_pens_to_layer(tmap, group, 0, 0, TILEMAP_PIXEL_LAYER0);

	/* add us to the end of the list of tilemaps */
	*machine->tilemap_data->tailptr = tmap;
	machine->tilemap_data->tailptr = &tmap->next;

	/* save relevant state */
	state_save_register_item(machine, "tilemap", NULL, tilemap_instance, tmap->enable);
	state_save_register_item(machine, "tilemap", NULL, tilemap_instance, tmap->attributes);
	state_save_register_item(machine, "tilemap", NULL, tilemap_instance, tmap->palette_offset);
	state_save_register_item(machine, "tilemap", NULL, tilemap_instance, tmap->pen_data_offset);
	state_save_register_item(machine, "tilemap", NULL, tilemap_instance, tmap->scrollrows);
	state_save_register_item(machine, "tilemap", NULL, tilemap_instance, tmap->scrollcols);
	state_save_register_item_pointer(machine, "tilemap", NULL, tilemap_instance, tmap->rowscroll, rows * tileheight);
	state_save_register_item_pointer(machine, "tilemap", NULL, tilemap_instance, tmap->colscroll, cols * tilewidth);
	state_save_register_item(machine, "tilemap", NULL, tilemap_instance, tmap->dx);
	state_save_register_item(machine, "tilemap", NULL, tilemap_instance, tmap->dx_flipped);
	state_save_register_item(machine, "tilemap", NULL, tilemap_instance, tmap->dy);
	state_save_register_item(machine, "tilemap", NULL, tilemap_instance, tmap->dy_flipped);
	machine->tilemap_data->instance++;

	/* reset everything after a load */
	state_save_register_postload(machine, tilemap_postload, tmap);
	return tmap;
}


/*-------------------------------------------------
    tilemap_set_user_data - specify a parameter
    to be passed into the tile_get_info callback
-------------------------------------------------*/

void tilemap_set_user_data(tilemap *tmap, void *user_data)
{
	tmap->user_data = user_data;
}


/*-------------------------------------------------
    tilemap_set_palette_offset - specify an offset
    to be added to each pixel before looking up
    the palette
-------------------------------------------------*/

void tilemap_set_palette_offset(tilemap *tmap, UINT32 offset)
{
	tmap->palette_offset = offset;
}


/*-------------------------------------------------
    tilemap_set_enable - set an enable flag for
    the tilemap; if 0, requests to draw the
    tilemap are ignored
-------------------------------------------------*/

void tilemap_set_enable(tilemap *tmap, int enable)
{
	tmap->enable = (enable != 0);
}


/*-------------------------------------------------
    tilemap_set_flip - set a global flip for the
    tilemap
-------------------------------------------------*/

void tilemap_set_flip(tilemap *tmap, UINT32 attributes)
{
	/* if we're changing things, force a refresh of the mappings and mark it all dirty */
	if (tmap->attributes != attributes)
	{
		tmap->attributes = attributes;
		mappings_update(tmap);
	}
}


/*-------------------------------------------------
    tilemap_set_flip_all - set a global flip for all
    the tilemaps
-------------------------------------------------*/

void tilemap_set_flip_all(running_machine *machine, UINT32 attributes)
{
	tilemap *tmap;
	for (tmap = machine->tilemap_data->list; tmap != NULL; tmap = tmap->next)
		tilemap_set_flip(tmap, attributes);
}



/***************************************************************************
    DIRTY TILE MARKING
***************************************************************************/

/*-------------------------------------------------
    tilemap_mark_tile_dirty - mark a single tile
    dirty based on its memory index
-------------------------------------------------*/

void tilemap_mark_tile_dirty(tilemap *tmap, tilemap_memory_index memindex)
{
	/* only mark if within range */
	if (memindex < tmap->max_memory_index)
	{
		tilemap_logical_index logindex = tmap->memory_to_logical[memindex];

		/* there may be no logical index for a given memory index */
		if (logindex != INVALID_LOGICAL_INDEX)
		{
			tmap->tileflags[logindex] = TILE_FLAG_DIRTY;
			tmap->all_tiles_clean = FALSE;
		}
	}
}


/*-------------------------------------------------
    tilemap_mark_all_tiles_dirty - mark all the
    tiles in a tilemap dirty
-------------------------------------------------*/

void tilemap_mark_all_tiles_dirty(tilemap *tmap)
{
	/* mark all tiles dirty and clear the clean flag */
	tmap->all_tiles_dirty = TRUE;
	tmap->all_tiles_clean = FALSE;
}


/*-------------------------------------------------
    tilemap_mark_all_tiles_dirty_all - mark all the
    tiles in all the tilemaps dirty
-------------------------------------------------*/

void tilemap_mark_all_tiles_dirty_all(running_machine *machine)
{
	tilemap *tmap;
	for (tmap = machine->tilemap_data->list; tmap != NULL; tmap = tmap->next)
		tilemap_mark_all_tiles_dirty(tmap);
}


/***************************************************************************
    PEN-TO-LAYER MAPPING
***************************************************************************/

/*-------------------------------------------------
    tilemap_map_pens_to_layer - specify the
    mapping of one or more pens (where
    (<pen> & mask) == pen) to a layer
-------------------------------------------------*/

void tilemap_map_pens_to_layer(tilemap *tmap, int group, pen_t pen, pen_t mask, UINT8 layermask)
{
	UINT8 *array = tmap->pen_to_flags + group * MAX_PEN_TO_FLAGS;
	pen_t start, stop, cur;
	UINT8 changed = FALSE;

	assert(group < TILEMAP_NUM_GROUPS);
	assert((layermask & TILEMAP_PIXEL_CATEGORY_MASK) == 0);

	/* we start at the index where (pen & mask) == pen, and all other bits are 0 */
	start = pen & mask;

	/* we stop at the index where (pen & mask) == pen, and all other bits are 1 */
	stop = start | ~mask;

	/* clamp to the number of entries actually there */
	stop = MIN(stop, MAX_PEN_TO_FLAGS - 1);

	/* iterate and set */
	for (cur = start; cur <= stop; cur++)
		if ((cur & mask) == pen && array[cur] != layermask)
		{
			changed = TRUE;
			array[cur] = layermask;
		}

	/* everything gets dirty if anything changed */
	if (changed)
		tilemap_mark_all_tiles_dirty(tmap);
}


/*-------------------------------------------------
    tilemap_set_transparent_pen - set a single
    transparent pen into the tilemap, mapping
    all other pens to layer 0
-------------------------------------------------*/

void tilemap_set_transparent_pen(tilemap *tmap, pen_t pen)
{
	/* reset the whole pen map to opaque */
	tilemap_map_pens_to_layer(tmap, 0, 0, 0, TILEMAP_PIXEL_LAYER0);

	/* set the single pen to transparent */
    tilemap_map_pen_to_layer(tmap, 0, pen, TILEMAP_PIXEL_TRANSPARENT);
}


/*-------------------------------------------------
    tilemap_set_transmask - set up the first 32
    pens using a foreground mask (mapping to
    layer 0) and a background mask (mapping to
    layer 1)
-------------------------------------------------*/

void tilemap_set_transmask(tilemap *tmap, int group, UINT32 fgmask, UINT32 bgmask)
{
	pen_t pen;

	/* iterate over all 32 pens specified */
	for (pen = 0; pen < 32; pen++)
	{
		UINT8 fgbits = ((fgmask >> pen) & 1) ? TILEMAP_PIXEL_TRANSPARENT : TILEMAP_PIXEL_LAYER0;
		UINT8 bgbits = ((bgmask >> pen) & 1) ? TILEMAP_PIXEL_TRANSPARENT : TILEMAP_PIXEL_LAYER1;
		tilemap_map_pen_to_layer(tmap, group, pen, fgbits | bgbits);
	}
}



/***************************************************************************
    TILEMAP SCROLLING
***************************************************************************/

/*-------------------------------------------------
    tilemap_set_scroll_rows - specify the number of
    independently scrollable row units; each unit
    covers height/scroll_rows pixels
-------------------------------------------------*/

void tilemap_set_scroll_rows(tilemap *tmap, UINT32 scroll_rows)
{
	assert(scroll_rows <= tmap->height);
	tmap->scrollrows = scroll_rows;
}


/*-------------------------------------------------
    tilemap_set_scroll_cols - specify the number of
    independently scrollable column units; each
    unit covers width/scroll_cols pixels
-------------------------------------------------*/

void tilemap_set_scroll_cols(tilemap *tmap, UINT32 scroll_cols)
{
	assert(scroll_cols <= tmap->width);
	tmap->scrollcols = scroll_cols;
}


/*-------------------------------------------------
    tilemap_set_scrolldx - specify global
    horizontal scroll offset, for non-flipped and
    flipped cases
-------------------------------------------------*/

void tilemap_set_scrolldx(tilemap *tmap, int dx, int dx_flipped)
{
	tmap->dx = dx;
	tmap->dx_flipped = dx_flipped;
}


/*-------------------------------------------------
    tilemap_set_scrolldy - specify global
    vertical scroll offset, for non-flipped and
    flipped cases
-------------------------------------------------*/

void tilemap_set_scrolldy(tilemap *tmap, int dy, int dy_flipped)
{
	tmap->dy = dy;
	tmap->dy_flipped = dy_flipped;
}


/*-------------------------------------------------
    tilemap_get_scrolldx - return the global
    horizontal scroll offset, based on current
    flip state
-------------------------------------------------*/

int tilemap_get_scrolldx(tilemap *tmap)
{
	return (tmap->attributes & TILEMAP_FLIPX) ? tmap->dx_flipped : tmap->dx;
}


/*-------------------------------------------------
    tilemap_get_scrolldy - return the global
    vertical scroll offset, based on current
    flip state
-------------------------------------------------*/

int tilemap_get_scrolldy(tilemap *tmap)
{
	return (tmap->attributes & TILEMAP_FLIPY) ? tmap->dy_flipped : tmap->dy;
}


/*-------------------------------------------------
    tilemap_set_scrollx - specify the scroll value
    for a row unit
-------------------------------------------------*/

void tilemap_set_scrollx(tilemap *tmap, int which, int value)
{
	if (which < tmap->scrollrows)
		tmap->rowscroll[which] = value;
}


/*-------------------------------------------------
    tilemap_set_scrolly - specify the scroll value
    for a column unit
-------------------------------------------------*/

void tilemap_set_scrolly(tilemap *tmap, int which, int value)
{
	if (which < tmap->scrollcols)
		tmap->colscroll[which] = value;
}


/*-------------------------------------------------
    tilemap_get_scrollx - return the scroll value
    for a row unit
-------------------------------------------------*/

int tilemap_get_scrollx(tilemap *tmap, int which)
{
	if (which < tmap->scrollrows)
		return tmap->rowscroll[which];
	else
		return 0;
}


/*-------------------------------------------------
    tilemap_get_scrolly - return the scroll value
    for a column unit
-------------------------------------------------*/

int tilemap_get_scrolly(tilemap *tmap, int which)
{
	if (which < tmap->scrollcols)
		return tmap->colscroll[which];
	else
		return 0;
}



/***************************************************************************
    INTERNAL MAP ACCESS
***************************************************************************/

/*-------------------------------------------------
    tilemap_get_pixmap - return a pointer to the
    (updated) internal pixmap for a tilemap
-------------------------------------------------*/

bitmap_t *tilemap_get_pixmap(tilemap *tmap)
{
	/* ensure all the tiles are up-to-date and then return the pixmap */
	pixmap_update(tmap, NULL);
	return tmap->pixmap;
}


/*-------------------------------------------------
    tilemap_get_flagsmap - return a pointer to the
    (updated) internal flagsmap for a tilemap
-------------------------------------------------*/

bitmap_t *tilemap_get_flagsmap(tilemap *tmap)
{
	/* ensure all the tiles are up-to-date and then return the pixmap */
	pixmap_update(tmap, NULL);
	return tmap->flagsmap;
}


/*-------------------------------------------------
    tilemap_get_pixmap - return a pointer to the
    (updated) internal per-tile flags for a tilemap
-------------------------------------------------*/

UINT8 *tilemap_get_tile_flags(tilemap *tmap)
{
	/* ensure all the tiles are up-to-date and then return the pixmap */
	pixmap_update(tmap, NULL);
	return tmap->tileflags;
}



/***************************************************************************
    TILEMAP RENDERING
***************************************************************************/

/*-------------------------------------------------
    tilemap_draw_primask - draw a tilemap to the
    destination with clipping; pixels apply
    priority/priority_mask to the priority bitmap
-------------------------------------------------*/

void tilemap_draw_primask(bitmap_t *dest, const rectangle *cliprect, tilemap *tmap, UINT32 flags, UINT8 priority, UINT8 priority_mask)
{
	UINT32 width, height;
	blit_parameters blit;
	int xpos, ypos;

	/* skip if disabled */
	if (!tmap->enable)
		return;

profiler_mark_start(PROFILER_TILEMAP_DRAW);
	/* configure the blit parameters based on the input parameters */
	configure_blit_parameters(&blit, tmap, dest, cliprect, flags, priority, priority_mask);

	/* if the whole map is dirty, mark it as such */
	if (tmap->all_tiles_dirty || gfx_elements_changed(tmap))
	{
		memset(tmap->tileflags, TILE_FLAG_DIRTY, tmap->max_logical_index);
		tmap->all_tiles_dirty = FALSE;
		tmap->gfx_used = 0;
	}

	width  = video_screen_get_width(tmap->machine->primary_screen);
	height = video_screen_get_height(tmap->machine->primary_screen);

	/* XY scrolling playfield */
	if (tmap->scrollrows == 1 && tmap->scrollcols == 1)
	{
		int scrollx = effective_rowscroll(tmap, 0, width);
		int scrolly = effective_colscroll(tmap, 0, height);

		/* iterate to handle wraparound */
		for (ypos = scrolly - tmap->height; ypos <= blit.cliprect.max_y; ypos += tmap->height)
			for (xpos = scrollx - tmap->width; xpos <= blit.cliprect.max_x; xpos += tmap->width)
				tilemap_draw_instance(tmap, &blit, xpos, ypos);
	}

	/* scrolling rows + vertical scroll */
	else if (tmap->scrollcols == 1)
	{
		const rectangle original_cliprect = blit.cliprect;
		int rowheight = tmap->height / tmap->scrollrows;
		int scrolly = effective_colscroll(tmap, 0, height);
		int currow, nextrow;

		/* iterate over Y to handle wraparound */
		for (ypos = scrolly - tmap->height; ypos <= original_cliprect.max_y; ypos += tmap->height)
		{
			int const firstrow = MAX((original_cliprect.min_y - ypos) / rowheight, 0);
			int const lastrow =  MIN((original_cliprect.max_y - ypos) / rowheight, tmap->scrollrows - 1);

			/* iterate over rows in the tilemap */
			for (currow = firstrow; currow <= lastrow; currow = nextrow)
			{
				int scrollx = effective_rowscroll(tmap, currow, width);

				/* scan forward until we find a non-matching row */
				for (nextrow = currow + 1; nextrow <= lastrow; nextrow++)
					if (effective_rowscroll(tmap, nextrow, width) != scrollx)
						break;

				/* skip if disabled */
				if (scrollx == TILE_LINE_DISABLED)
					continue;

				/* update the cliprect just for this set of rows */
				blit.cliprect.min_y = currow * rowheight + ypos;
				blit.cliprect.max_y = nextrow * rowheight - 1 + ypos;
				sect_rect(&blit.cliprect, &original_cliprect);

				/* iterate over X to handle wraparound */
				for (xpos = scrollx - tmap->width; xpos <= original_cliprect.max_x; xpos += tmap->width)
					tilemap_draw_instance(tmap, &blit, xpos, ypos);
			}
		}
	}

	/* scrolling columns + horizontal scroll */
	else if (tmap->scrollrows == 1)
	{
		const rectangle original_cliprect = blit.cliprect;
		int colwidth = tmap->width / tmap->scrollcols;
		int scrollx = effective_rowscroll(tmap, 0, width);
		int curcol, nextcol;

		/* iterate over columns in the tilemap */
		for (curcol = 0; curcol < tmap->scrollcols; curcol = nextcol)
		{
			int scrolly	= effective_colscroll(tmap, curcol, height);

			/* scan forward until we find a non-matching column */
			for (nextcol = curcol + 1; nextcol < tmap->scrollcols; nextcol++)
				if (effective_colscroll(tmap, nextcol, height) != scrolly)
					break;

 			/* skip if disabled */
			if (scrolly == TILE_LINE_DISABLED)
				continue;

			/* iterate over X to handle wraparound */
			for (xpos = scrollx - tmap->width; xpos <= original_cliprect.max_x; xpos += tmap->width)
			{
				/* update the cliprect just for this set of columns */
				blit.cliprect.min_x = curcol * colwidth + xpos;
				blit.cliprect.max_x = nextcol * colwidth - 1 + xpos;
				sect_rect(&blit.cliprect, &original_cliprect);

				/* iterate over Y to handle wraparound */
				for (ypos = scrolly - tmap->height; ypos <= original_cliprect.max_y; ypos += tmap->height)
					tilemap_draw_instance(tmap, &blit, xpos, ypos);
			}
		}
	}
profiler_mark_end();
}


/*-------------------------------------------------
    tilemap_draw_primask - draw a tilemap to the
    destination with clipping and arbitrary
    rotate/zoom; pixels apply priority/
    priority_mask to the priority bitmap
-------------------------------------------------*/

void tilemap_draw_roz_primask(bitmap_t *dest, const rectangle *cliprect, tilemap *tmap,
		UINT32 startx, UINT32 starty, int incxx, int incxy, int incyx, int incyy,
		int wraparound, UINT32 flags, UINT8 priority, UINT8 priority_mask)
{
	blit_parameters blit;

/* notes:
   - startx and starty MUST be UINT32 for calculations to work correctly
   - srcbitmap->width and height are assumed to be a power of 2 to speed up wraparound
   */

	/* skip if disabled */
	if (!tmap->enable)
		return;

	/* see if this is just a regular render and if so, do a regular render */
	if (incxx == (1 << 16) && incxy == 0 && incyx == 0 && incyy == (1 << 16) && wraparound)
	{
		tilemap_set_scrollx(tmap, 0, startx >> 16);
		tilemap_set_scrolly(tmap, 0, starty >> 16);
		tilemap_draw(dest, cliprect, tmap, flags, priority);
		return;
	}

profiler_mark_start(PROFILER_TILEMAP_DRAW_ROZ);
	/* configure the blit parameters */
	configure_blit_parameters(&blit, tmap, dest, cliprect, flags, priority, priority_mask);

	/* get the full pixmap for the tilemap */
	tilemap_get_pixmap(tmap);

	/* then do the roz copy */
	tilemap_draw_roz_core(tmap, &blit, startx, starty, incxx, incxy, incyx, incyy, wraparound);
profiler_mark_end();
}



/***************************************************************************
    INDEXED TILEMAP HANDLING
***************************************************************************/

/*-------------------------------------------------
    tilemap_count - return the number of tilemaps
-------------------------------------------------*/

int tilemap_count(running_machine *machine)
{
	tilemap *tmap;
	int count = 0;

	/* find by the tilemap index */
	for (tmap = machine->tilemap_data->list; tmap != NULL; tmap = tmap->next)
		count++;
	return count;
}


/*-------------------------------------------------
    tilemap_size_by_index - return the size of an
    indexed tilemap
-------------------------------------------------*/

void tilemap_size_by_index(running_machine *machine, int number, UINT32 *width, UINT32 *height)
{
	tilemap *tmap = indexed_tilemap(machine, number);
	*width = tmap->width;
	*height = tmap->height;
}


/*-------------------------------------------------
    tilemap_draw_by_index - render an indexed
    tilemap with fixed characteristics (no
    priority)
-------------------------------------------------*/

void tilemap_draw_by_index(running_machine *machine, bitmap_t *dest, int number, UINT32 scrollx, UINT32 scrolly)
{
	tilemap *tmap = indexed_tilemap(machine, number);
	blit_parameters blit;
	int xpos,ypos;

	/* set up for the blit, using hard-coded parameters (no priority, etc) */
	configure_blit_parameters(&blit, tmap, dest, NULL, TILEMAP_DRAW_OPAQUE | TILEMAP_DRAW_ALL_CATEGORIES, 0, 0xff);

	/* compute the effective scroll positions */
	scrollx = tmap->width  - scrollx % tmap->width;
	scrolly = tmap->height - scrolly % tmap->height;

	/* if the whole map is dirty, mark it as such */
	if (tmap->all_tiles_dirty || gfx_elements_changed(tmap))
	{
		memset(tmap->tileflags, TILE_FLAG_DIRTY, tmap->max_logical_index);
		tmap->all_tiles_dirty = FALSE;
		tmap->gfx_used = 0;
	}

	/* iterate to handle wraparound */
	for (ypos = scrolly - tmap->height; ypos <= blit.cliprect.max_y; ypos += tmap->height)
		for (xpos = scrollx - tmap->width; xpos <= blit.cliprect.max_x; xpos += tmap->width)
			tilemap_draw_instance(tmap, &blit, xpos, ypos);
}



/***************************************************************************
    COMMON LOGICAL-TO-MEMORY MAPPERS
***************************************************************************/

/*-------------------------------------------------
    tilemap_scan_rows
    tilemap_scan_rows_flip_x
    tilemap_scan_rows_flip_y
    tilemap_scan_rows_flip_xy - scan in row-major
    order with optional flipping
-------------------------------------------------*/

TILEMAP_MAPPER( tilemap_scan_rows )
{
	return row * num_cols + col;
}

TILEMAP_MAPPER( tilemap_scan_rows_flip_x )
{
	return row * num_cols + (num_cols - 1 - col);
}

TILEMAP_MAPPER( tilemap_scan_rows_flip_y )
{
	return (num_rows - 1 - row) * num_cols + col;
}

TILEMAP_MAPPER( tilemap_scan_rows_flip_xy )
{
	return (num_rows - 1 - row) * num_cols + (num_cols - 1 - col);
}


/*-------------------------------------------------
    tilemap_scan_cols
    tilemap_scan_cols_flip_x
    tilemap_scan_cols_flip_y
    tilemap_scan_cols_flip_xy - scan in column-
    major order with optional flipping
-------------------------------------------------*/

TILEMAP_MAPPER( tilemap_scan_cols )
{
	return col * num_rows + row;
}

TILEMAP_MAPPER( tilemap_scan_cols_flip_x )
{
	return (num_cols - 1 - col) * num_rows + row;
}

TILEMAP_MAPPER( tilemap_scan_cols_flip_y )
{
	return col * num_rows + (num_rows - 1 - row);
}

TILEMAP_MAPPER( tilemap_scan_cols_flip_xy )
{
	return (num_cols - 1 - col) * num_rows + (num_rows - 1 - row);
}



/***************************************************************************
    SYSTEM MANAGEMENT HELPERS
***************************************************************************/

/*-------------------------------------------------
    tilemap_exit - free memory allocated by the
    tilemap system
-------------------------------------------------*/

static void tilemap_exit(running_machine *machine)
{
	tilemap_private *tilemap_data = machine->tilemap_data;

	/* free all the tilemaps in the list */
	while (tilemap_data->list != NULL)
	{
		tilemap *next = tilemap_data->list->next;
		tilemap_dispose(tilemap_data->list);
		tilemap_data->list = next;
	}
	tilemap_data->tailptr = &tilemap_data->list;
}


/*-------------------------------------------------
    tilemap_postload - after loading a save state
    invalidate everything
-------------------------------------------------*/

static STATE_POSTLOAD( tilemap_postload )
{
	/* recompute the mappings for this tilemap */
	tilemap *tmap = (tilemap *)param;
	mappings_update(tmap);
}


/*-------------------------------------------------
    tilemap_dispose - dispose of a tilemap
-------------------------------------------------*/

static void tilemap_dispose(tilemap *tmap)
{
	tilemap **tmapptr;

	/* walk the list of tilemaps; when we find ourself, remove it */
	for (tmapptr = &tmap->machine->tilemap_data->list; *tmapptr != NULL; tmapptr = &(*tmapptr)->next)
		if (*tmapptr == tmap)
		{
			*tmapptr = tmap->next;
			break;
		}

	/* free allocated memory */
	free(tmap->pen_to_flags);
	free(tmap->tileflags);
	bitmap_free(tmap->flagsmap);
	bitmap_free(tmap->pixmap);
	free(tmap->colscroll);
	free(tmap->rowscroll);
	free(tmap->logical_to_memory);
	free(tmap->memory_to_logical);
	free(tmap);
}



/***************************************************************************
    LOGICAL <-> MEMORY INDEX MAPPING
***************************************************************************/

/*-------------------------------------------------
    mappings_create - allocate memory for the
    mapping tables and compute their extents
-------------------------------------------------*/

static void mappings_create(tilemap *tmap)
{
	UINT32 col, row;

	/* compute the maximum logical index */
	tmap->max_logical_index = tmap->rows * tmap->cols;

	/* compute the maximum memory index */
	tmap->max_memory_index = 0;
	for (row = 0; row < tmap->rows; row++)
		for (col = 0; col < tmap->cols; col++)
		{
			tilemap_memory_index memindex = (*tmap->mapper)(col, row, tmap->cols, tmap->rows);
			tmap->max_memory_index = MAX(tmap->max_memory_index, memindex);
		}
	tmap->max_memory_index++;

	/* allocate the necessary mappings */
	tmap->memory_to_logical = alloc_array_or_die(tilemap_logical_index, tmap->max_memory_index);
	tmap->logical_to_memory = alloc_array_or_die(tilemap_memory_index, tmap->max_logical_index);

	/* update the mappings */
	mappings_update(tmap);
}


/*-------------------------------------------------
    mappings_update - update the mappings after
    a major change (flip x/y changes)
-------------------------------------------------*/

static void mappings_update(tilemap *tmap)
{
	tilemap_logical_index logindex;
	tilemap_memory_index memindex;

	/* initialize all the mappings to invalid values */
	for (memindex = 0; memindex < tmap->max_memory_index; memindex++)
		tmap->memory_to_logical[memindex] = -1;

	/* now iterate over all logical indexes and populate the memory index */
	for (logindex = 0; logindex < tmap->max_logical_index; logindex++)
	{
		UINT32 logical_col = logindex % tmap->cols;
		UINT32 logical_row = logindex / tmap->cols;
		tilemap_memory_index memindex = (*tmap->mapper)(logical_col, logical_row, tmap->cols, tmap->rows);
		UINT32 flipped_logindex;

		/* apply tilemap flip to get the final location to store */
		if (tmap->attributes & TILEMAP_FLIPX)
			logical_col = (tmap->cols - 1) - logical_col;
		if (tmap->attributes & TILEMAP_FLIPY)
			logical_row = (tmap->rows - 1) - logical_row;
		flipped_logindex = logical_row * tmap->cols + logical_col;

		/* fill in entries in both arrays */
		tmap->memory_to_logical[memindex] = flipped_logindex;
		tmap->logical_to_memory[flipped_logindex] = memindex;
	}

	/* mark the whole tilemap dirty */
	tilemap_mark_all_tiles_dirty(tmap);
}



/***************************************************************************
    TILE RENDERING
***************************************************************************/

/*-------------------------------------------------
    pixmap_update - update the portion of the
    pixmap described by the cliprect
-------------------------------------------------*/

static void pixmap_update(tilemap *tmap, const rectangle *cliprect)
{
	int mincol, maxcol, minrow, maxrow;
	int row, col;

	/* if the graphics changed, we need to mark everything dirty */
 	if (gfx_elements_changed(tmap))
		tilemap_mark_all_tiles_dirty(tmap);

	/* if everything is clean, do nothing */
	if (tmap->all_tiles_clean)
		return;

profiler_mark_start(PROFILER_TILEMAP_DRAW);

	/* compute which columns and rows to update */
	if (cliprect != NULL)
	{
		mincol = cliprect->min_x / tmap->tilewidth;
		maxcol = cliprect->max_x / tmap->tilewidth;
		minrow = cliprect->min_y / tmap->tileheight;
		maxrow = cliprect->max_y / tmap->tileheight;
	}
	else
	{
		mincol = minrow = 0;
		maxcol = tmap->cols - 1;
		maxrow = tmap->rows - 1;
	}

	/* if the whole map is dirty, mark it as such */
	if (tmap->all_tiles_dirty)
	{
		memset(tmap->tileflags, TILE_FLAG_DIRTY, tmap->max_logical_index);
		tmap->all_tiles_dirty = FALSE;
		tmap->gfx_used = 0;
	}

	/* iterate over rows */
	for (row = minrow; row <= maxrow; row++)
	{
		tilemap_logical_index logindex = row * tmap->cols;

		/* iterate over colums */
		for (col = mincol; col <= maxcol; col++)
			if (tmap->tileflags[logindex + col] == TILE_FLAG_DIRTY)
				tile_update(tmap, logindex + col, col, row);
	}

	/* mark it all clean */
	if (mincol == 0 && minrow == 0 && maxcol == tmap->cols - 1 && maxcol == tmap->rows - 1)
		tmap->all_tiles_clean = TRUE;

profiler_mark_end();
}


/*-------------------------------------------------
    tile_update - update a single dirty tile
-------------------------------------------------*/

static void tile_update(tilemap *tmap, tilemap_logical_index logindex, UINT32 col, UINT32 row)
{
	UINT32 x0 = tmap->tilewidth * col;
	UINT32 y0 = tmap->tileheight * row;
	tilemap_memory_index memindex;
	UINT32 flags;

profiler_mark_start(PROFILER_TILEMAP_UPDATE);

	/* call the get info callback for the associated memory index */
	memindex = tmap->logical_to_memory[logindex];
	(*tmap->tile_get_info)(tmap->machine, &tmap->tileinfo, memindex, tmap->user_data);

	/* apply the global tilemap flip to the returned flip flags */
	flags = tmap->tileinfo.flags ^ (tmap->attributes & 0x03);

	/* draw the tile, using either direct or transparent */
	tmap->tileflags[logindex] = tile_draw(tmap, tmap->tileinfo.pen_data + tmap->pen_data_offset, x0, y0,
		tmap->tileinfo.palette_base, tmap->tileinfo.category, tmap->tileinfo.group, flags, tmap->tileinfo.pen_mask);

	/* if mask data is specified, apply it */
	if ((flags & (TILE_FORCE_LAYER0 | TILE_FORCE_LAYER1 | TILE_FORCE_LAYER2)) == 0 && tmap->tileinfo.mask_data != NULL)
		tmap->tileflags[logindex] = tile_apply_bitmask(tmap, tmap->tileinfo.mask_data, x0, y0, tmap->tileinfo.category, flags);

	/* track which gfx have been used for this tilemap */
	if (tmap->tileinfo.gfxnum != 0xff && (tmap->gfx_used & (1 << tmap->tileinfo.gfxnum)) == 0)
	{
		tmap->gfx_used |= 1 << tmap->tileinfo.gfxnum;
		tmap->gfx_dirtyseq[tmap->tileinfo.gfxnum] = tmap->machine->gfx[tmap->tileinfo.gfxnum]->dirtyseq;
	}

profiler_mark_end();
}


/*-------------------------------------------------
    tile_draw - draw a single tile to the
    tilemap's internal pixmap, using the pen as
    the pen_to_flags lookup value, and adding
    the palette_base
-------------------------------------------------*/

static UINT8 tile_draw(tilemap *tmap, const UINT8 *pendata, UINT32 x0, UINT32 y0, UINT32 palette_base, UINT8 category, UINT8 group, UINT8 flags, UINT8 pen_mask)
{
	const UINT8 *penmap = tmap->pen_to_flags + group * MAX_PEN_TO_FLAGS;
	bitmap_t *flagsmap = tmap->flagsmap;
	bitmap_t *pixmap = tmap->pixmap;
	int height = tmap->tileheight;
	int width = tmap->tilewidth;
	UINT8 andmask = ~0, ormask = 0;
	int dx0 = 1, dy0 = 1;
	int tx, ty;

	/* OR in the force layer flags */
	category |= flags & (TILE_FORCE_LAYER0 | TILE_FORCE_LAYER1 | TILE_FORCE_LAYER2);

	/* if we're vertically flipped, point to the bottom row and work backwards */
	if (flags & TILE_FLIPY)
	{
		y0 += height - 1;
		dy0 = -1;
	}

	/* if we're horizontally flipped, point to the rightmost column and work backwards */
	if (flags & TILE_FLIPX)
	{
		x0 += width - 1;
		dx0 = -1;
	}

	/* in 4bpp mode, we draw in groups of 2 pixels, so halve the width now */
	if (flags & TILE_4BPP)
	{
		assert(width % 2 == 0);
		width /= 2;
	}

	/* iterate over rows */
	for (ty = 0; ty < height; ty++)
	{
		UINT16 *pixptr = BITMAP_ADDR16(pixmap, y0, x0);
		UINT8 *flagsptr = BITMAP_ADDR8(flagsmap, y0, x0);
		int xoffs = 0;

		/* pre-advance to the next row */
		y0 += dy0;

		/* 8bpp data */
		if (!(flags & TILE_4BPP))
		{
			for (tx = 0; tx < width; tx++)
			{
				UINT8 pen = (*pendata++) & pen_mask;
				UINT8 map = penmap[pen];
				pixptr[xoffs] = palette_base + pen;
				flagsptr[xoffs] = map | category;
				andmask &= map;
				ormask |= map;
				xoffs += dx0;
			}
		}

		/* 4bpp data */
		else
		{
			for (tx = 0; tx < width; tx++)
			{
				UINT8 data = *pendata++;
				UINT8 pen, map;

				pen = (data & 0x0f) & pen_mask;
				map = penmap[pen];
				pixptr[xoffs] = palette_base + pen;
				flagsptr[xoffs] = map | category;
				andmask &= map;
				ormask |= map;
				xoffs += dx0;

				pen = (data >> 4) & pen_mask;
				map = penmap[pen];
				pixptr[xoffs] = palette_base + pen;
				flagsptr[xoffs] = map | category;
				andmask &= map;
				ormask |= map;
				xoffs += dx0;
			}
		}
	}
	return andmask ^ ormask;
}


/*-------------------------------------------------
    tile_apply_bitmask - apply a bitmask to an
    already-rendered tile by modifying the
    flagsmap appropriately
-------------------------------------------------*/

static UINT8 tile_apply_bitmask(tilemap *tmap, const UINT8 *maskdata, UINT32 x0, UINT32 y0, UINT8 category, UINT8 flags)
{
	bitmap_t *flagsmap = tmap->flagsmap;
	int height = tmap->tileheight;
	int width = tmap->tilewidth;
	UINT8 andmask = ~0, ormask = 0;
	int dx0 = 1, dy0 = 1;
	int bitoffs = 0;
	int tx, ty;

	/* if we're vertically flipped, point to the bottom row and work backwards */
	if (flags & TILE_FLIPY)
	{
		y0 += height - 1;
		dy0 = -1;
	}

	/* if we're horizontally flipped, point to the rightmost column and work backwards */
	if (flags & TILE_FLIPX)
	{
		x0 += width - 1;
		dx0 = -1;
	}

	/* iterate over rows */
	for (ty = 0; ty < height; ty++)
	{
		UINT8 *flagsptr = BITMAP_ADDR8(flagsmap, y0, x0);
		int xoffs = 0;

		/* pre-advance to the next row */
		y0 += dy0;

		/* anywhere the bitmask is 0 should be transparent */
		for (tx = 0; tx < width; tx++)
		{
			UINT8 map = flagsptr[xoffs];

			if ((maskdata[bitoffs / 8] & (0x80 >> (bitoffs & 7))) == 0)
				map = flagsptr[xoffs] = TILEMAP_PIXEL_TRANSPARENT | category;
			andmask &= map;
			ormask |= map;
			xoffs += dx0;
			bitoffs++;
		}
	}
	return andmask ^ ormask;
}



/***************************************************************************
    DRAWING HELPERS
***************************************************************************/

/*-------------------------------------------------
    configure_blit_parameters - fill in the
    standard blit parameters based on the input
    data; this code is shared by normal, roz,
    and indexed drawing code
-------------------------------------------------*/

static void configure_blit_parameters(blit_parameters *blit, tilemap *tmap, bitmap_t *dest, const rectangle *cliprect, UINT32 flags, UINT8 priority, UINT8 priority_mask)
{
	/* start with nothing */
	memset(blit, 0, sizeof(*blit));

	/* set the target bitmap */
	blit->bitmap = dest;

	/* if we have a cliprect, copy */
	if (cliprect != NULL)
		blit->cliprect = *cliprect;

	/* otherwise, make one up */
	else
	{
		blit->cliprect.min_x = blit->cliprect.min_y = 0;
		blit->cliprect.max_x = dest->width - 1;
		blit->cliprect.max_y = dest->height - 1;
	}

	/* set the priority code and alpha */
	blit->tilemap_priority_code = priority | (priority_mask << 8) | (tmap->palette_offset << 16);
	blit->alpha = (flags & TILEMAP_DRAW_ALPHA_FLAG) ? (flags >> 24) : 0xff;

	/* if no destination, just render priority */
	if (dest == NULL)
	{
		blit->draw_masked = scanline_draw_masked_null;
		blit->draw_opaque = scanline_draw_opaque_null;
	}

	/* otherwise get the appropriate callbacks for the format and flags */
	else
	{
		switch (dest->format)
		{
			case BITMAP_FORMAT_RGB32:
				blit->draw_masked = (blit->alpha < 0xff) ? scanline_draw_masked_rgb32_alpha : scanline_draw_masked_rgb32;
				blit->draw_opaque = (blit->alpha < 0xff) ? scanline_draw_opaque_rgb32_alpha : scanline_draw_opaque_rgb32;
				break;

			case BITMAP_FORMAT_RGB15:
				blit->draw_masked = (blit->alpha < 0xff) ? scanline_draw_masked_rgb16_alpha : scanline_draw_masked_rgb16;
				blit->draw_opaque = (blit->alpha < 0xff) ? scanline_draw_opaque_rgb16_alpha : scanline_draw_opaque_rgb16;
				break;

			case BITMAP_FORMAT_INDEXED16:
				blit->draw_masked = scanline_draw_masked_ind16;
				blit->draw_opaque = scanline_draw_opaque_ind16;
				break;

			default:
				fatalerror("tilemap_draw_primask: Invalid bitmap format");
				break;
		}
	}

	/* tile priority; unless otherwise specified, draw anything in layer 0 */
	blit->mask = TILEMAP_PIXEL_CATEGORY_MASK;
	blit->value	= flags & TILEMAP_PIXEL_CATEGORY_MASK;

	/* if no layers specified, draw layer 0 */
	if ((flags & (TILEMAP_DRAW_LAYER0 | TILEMAP_DRAW_LAYER1 | TILEMAP_DRAW_LAYER2)) == 0)
		flags |= TILEMAP_DRAW_LAYER0;

	/* OR in the bits from the draw masks */
	blit->mask |= flags & (TILEMAP_DRAW_LAYER0 | TILEMAP_DRAW_LAYER1 | TILEMAP_DRAW_LAYER2);
	blit->value |= flags & (TILEMAP_DRAW_LAYER0 | TILEMAP_DRAW_LAYER1 | TILEMAP_DRAW_LAYER2);

	/* for all-opaque rendering, don't check any of the layer bits */
	if (flags & TILEMAP_DRAW_OPAQUE)
	{
		blit->mask &= ~(TILEMAP_PIXEL_LAYER0 | TILEMAP_PIXEL_LAYER1 | TILEMAP_PIXEL_LAYER2);
		blit->value &= ~(TILEMAP_PIXEL_LAYER0 | TILEMAP_PIXEL_LAYER1 | TILEMAP_PIXEL_LAYER2);
	}

	/* don't check category if requested */
	if (flags & TILEMAP_DRAW_ALL_CATEGORIES)
	{
		blit->mask &= ~TILEMAP_PIXEL_CATEGORY_MASK;
		blit->value &= ~TILEMAP_PIXEL_CATEGORY_MASK;
	}
}


/*-------------------------------------------------
    tilemap_draw_instance - draw a single
    instance of the tilemap to the internal
    pixmap at the given xpos,ypos
-------------------------------------------------*/

static void tilemap_draw_instance(tilemap *tmap, const blit_parameters *blit, int xpos, int ypos)
{
	bitmap_t *priority_bitmap = tmap->machine->priority_bitmap;
	bitmap_t *dest = blit->bitmap;
	const UINT16 *source_baseaddr;
	const UINT8 *mask_baseaddr;
	void *dest_baseaddr = NULL;
	UINT8 *priority_baseaddr;
	int dest_line_pitch_bytes = 0;
	int dest_bytespp = 0;
	int mincol, maxcol;
	int x1, y1, x2, y2;
	int y, nexty;

	/* clip destination coordinates to the tilemap */
	/* note that x2/y2 are exclusive, not inclusive */
	x1 = MAX(xpos, blit->cliprect.min_x);
	x2 = MIN(xpos + (int)tmap->width, blit->cliprect.max_x + 1);
	y1 = MAX(ypos, blit->cliprect.min_y);
	y2 = MIN(ypos + (int)tmap->height, blit->cliprect.max_y + 1);

	/* if totally clipped, stop here */
	if (x1 >= x2 || y1 >= y2)
		return;

	/* look up priority and destination base addresses for y1 */
	priority_baseaddr = BITMAP_ADDR8(priority_bitmap, y1, xpos);
	if (dest != NULL)
	{
		dest_bytespp = dest->bpp / 8;
		dest_line_pitch_bytes = dest->rowpixels * dest_bytespp;
		dest_baseaddr = (UINT8 *)dest->base + (y1 * dest->rowpixels + xpos) * dest_bytespp;
	}

	/* convert screen coordinates to source tilemap coordinates */
	x1 -= xpos;
	y1 -= ypos;
	x2 -= xpos;
	y2 -= ypos;

	/* get tilemap pixels */
	source_baseaddr = BITMAP_ADDR16(tmap->pixmap, y1, 0);
	mask_baseaddr = BITMAP_ADDR8(tmap->flagsmap, y1, 0);

	/* get start/stop columns, rounding outward */
	mincol = x1 / tmap->tilewidth;
	maxcol = (x2 + tmap->tilewidth - 1) / tmap->tilewidth;

	/* set up row counter */
	y = y1;
	nexty = tmap->tileheight * (y1 / tmap->tileheight) + tmap->tileheight;
	nexty = MIN(nexty, y2);

	/* loop over tilemap rows */
	for (;;)
	{
		int row = y / tmap->tileheight;
		trans_t prev_trans = WHOLLY_TRANSPARENT;
		trans_t cur_trans;
		int x_start = x1;
		int column;

		/* iterate across the applicable tilemap columns */
		for (column = mincol; column <= maxcol; column++)
		{
			int x_end;

			/* we don't actually render the last column; it is always just used for flushing */
			if (column == maxcol)
				cur_trans = WHOLLY_TRANSPARENT;

			/* for other columns we look up the transparency information */
			else
			{
				tilemap_logical_index logindex = row * tmap->cols + column;

				/* if the current tile is dirty, fix it */
				if (tmap->tileflags[logindex] == TILE_FLAG_DIRTY)
					tile_update(tmap, logindex, column, row);

				/* if the current summary data is non-zero, we must draw masked */
				if ((tmap->tileflags[logindex] & blit->mask) != 0)
					cur_trans = MASKED;

				/* otherwise, our transparency state is constant across the tile; fetch it */
				else
					cur_trans = ((mask_baseaddr[column * tmap->tilewidth] & blit->mask) == blit->value) ? WHOLLY_OPAQUE : WHOLLY_TRANSPARENT;
			}

			/* if the transparency state is the same as last time, don't render yet */
			if (cur_trans == prev_trans)
				continue;

			/* compute the end of this run, in pixels */
			x_end = column * tmap->tilewidth;
			x_end = MAX(x_end, x1);
			x_end = MIN(x_end, x2);

			/* if we're rendering something, compute the pointers */
			if (prev_trans != WHOLLY_TRANSPARENT)
			{
				const UINT16 *source0 = source_baseaddr + x_start;
				void *dest0 = (UINT8 *)dest_baseaddr + x_start * dest_bytespp;
				UINT8 *pmap0 = priority_baseaddr + x_start;
				int cury;

				/* if we were opaque, use the opaque renderer */
				if (prev_trans == WHOLLY_OPAQUE)
				{
					for (cury = y; cury < nexty; cury++)
					{
						(*blit->draw_opaque)(dest0, source0, x_end - x_start, tmap->machine->pens, pmap0, blit->tilemap_priority_code, blit->alpha);

						dest0 = (UINT8 *)dest0 + dest_line_pitch_bytes;
						source0 += tmap->pixmap->rowpixels;
						pmap0 += priority_bitmap->rowpixels;
					}
				}

				/* otherwise use the masked renderer */
				else
				{
					const UINT8 *mask0 = mask_baseaddr + x_start;
					for (cury = y; cury < nexty; cury++)
					{
						(*blit->draw_masked)(dest0, source0, mask0, blit->mask, blit->value, x_end - x_start, tmap->machine->pens, pmap0, blit->tilemap_priority_code, blit->alpha);

						dest0 = (UINT8 *)dest0 + dest_line_pitch_bytes;
						source0 += tmap->pixmap->rowpixels;
						mask0 += tmap->flagsmap->rowpixels;
						pmap0 += priority_bitmap->rowpixels;
					}
				}
			}

			/* the new start is the end */
			x_start = x_end;
			prev_trans = cur_trans;
		}

		/* if this was the last row, stop */
		if (nexty == y2)
			break;

		/* advance to the next row on all our bitmaps */
		priority_baseaddr += priority_bitmap->rowpixels * (nexty - y);
		source_baseaddr += tmap->pixmap->rowpixels * (nexty - y);
		mask_baseaddr += tmap->flagsmap->rowpixels * (nexty - y);
		dest_baseaddr = (UINT8 *)dest_baseaddr + dest_line_pitch_bytes * (nexty - y);

		/* increment the Y counter */
		y = nexty;
		nexty += tmap->tileheight;
		nexty = MIN(nexty, y2);
	}
}


/*-------------------------------------------------
    tilemap_draw_roz_core - render the tilemap's
    pixmap to the destination with rotation
    and zoom
-------------------------------------------------*/

#define ROZ_PLOT_PIXEL(INPUT_VAL)											\
do {																		\
	if (blit->draw_masked == scanline_draw_masked_ind16)					\
		*(UINT16 *)dest = (INPUT_VAL) + (priority >> 16);					\
	else if (blit->draw_masked == scanline_draw_masked_rgb32)				\
		*(UINT32 *)dest = clut[INPUT_VAL];									\
	else if (blit->draw_masked == scanline_draw_masked_rgb16)				\
		*(UINT16 *)dest = clut[INPUT_VAL];									\
	else if (blit->draw_masked == scanline_draw_masked_rgb32_alpha)			\
		*(UINT32 *)dest = alpha_blend_r32(*(UINT32 *)dest, clut[INPUT_VAL], alpha);	\
	else if (blit->draw_masked == scanline_draw_masked_rgb16_alpha)			\
		*(UINT16 *)dest = alpha_blend_r16(*(UINT16 *)dest, clut[INPUT_VAL], alpha);	\
} while (0)

static void tilemap_draw_roz_core(tilemap *tmap, const blit_parameters *blit,
		UINT32 startx, UINT32 starty, int incxx, int incxy, int incyx, int incyy, int wraparound)
{
	const pen_t *clut = &tmap->machine->pens[blit->tilemap_priority_code >> 16];
	bitmap_t *priority_bitmap = tmap->machine->priority_bitmap;
	bitmap_t *destbitmap = blit->bitmap;
	bitmap_t *srcbitmap = tmap->pixmap;
	bitmap_t *flagsmap = tmap->flagsmap;
	const int xmask = srcbitmap->width-1;
	const int ymask = srcbitmap->height-1;
	const int widthshifted = srcbitmap->width << 16;
	const int heightshifted = srcbitmap->height << 16;
	UINT32 priority = blit->tilemap_priority_code;
	UINT8 mask = blit->mask;
	UINT8 value = blit->value;
	UINT8 alpha = blit->alpha;
	UINT32 cx;
	UINT32 cy;
	int x;
	int sx;
	int sy;
	int ex;
	int ey;
	void *dest;
	UINT8 *pri;
	const UINT16 *src;
	const UINT8 *maskptr;
	int destadvance = destbitmap->bpp / 8;

	/* pre-advance based on the cliprect */
	startx += blit->cliprect.min_x * incxx + blit->cliprect.min_y * incyx;
	starty += blit->cliprect.min_x * incxy + blit->cliprect.min_y * incyy;

	/* extract start/end points */
	sx = blit->cliprect.min_x;
	sy = blit->cliprect.min_y;
	ex = blit->cliprect.max_x;
	ey = blit->cliprect.max_y;

	/* optimized loop for the not rotated case */
	if (incxy == 0 && incyx == 0 && !wraparound)
	{
		/* skip without drawing until we are within the bitmap */
		while (startx >= widthshifted && sx <= ex)
		{
			startx += incxx;
			sx++;
		}

		/* early exit if we're done already */
		if (sx > ex)
			return;

		/* loop over rows */
		while (sy <= ey)
		{
			/* only draw if Y is within range */
			if (starty < heightshifted)
			{
				/* initialize X counters */
				x = sx;
				cx = startx;
				cy = starty >> 16;

				/* get source and priority pointers */
				pri = BITMAP_ADDR8(priority_bitmap, sy, sx);
				src = BITMAP_ADDR16(srcbitmap, cy, 0);
				maskptr = BITMAP_ADDR8(flagsmap, cy, 0);
				dest = (UINT8 *)destbitmap->base + (destbitmap->rowpixels * sy + sx) * destadvance;

				/* loop over columns */
				while (x <= ex && cx < widthshifted)
				{
					/* plot if we match the mask */
					if ((maskptr[cx >> 16] & mask) == value)
					{
						ROZ_PLOT_PIXEL(src[cx >> 16]);
						*pri = (*pri & (priority >> 8)) | priority;
					}

					/* advance in X */
					cx += incxx;
					x++;
					dest = (UINT8 *)dest + destadvance;
					pri++;
				}
			}

			/* advance in Y */
			starty += incyy;
			sy++;
		}
	}

	/* wraparound case */
	else if (wraparound)
	{
		/* loop over rows */
		while (sy <= ey)
		{
			/* initialize X counters */
			x = sx;
			cx = startx;
			cy = starty;

			/* get dest and priority pointers */
			dest = (UINT8 *)destbitmap->base + (destbitmap->rowpixels * sy + sx) * destadvance;
			pri = BITMAP_ADDR8(priority_bitmap, sy, sx);

			/* loop over columns */
			while (x <= ex)
			{
				/* plot if we match the mask */
				if ((*BITMAP_ADDR8(flagsmap, (cy >> 16) & ymask, (cx >> 16) & xmask) & mask) == value)
				{
					ROZ_PLOT_PIXEL(*BITMAP_ADDR16(srcbitmap, (cy >> 16) & ymask, (cx >> 16) & xmask));
					*pri = (*pri & (priority >> 8)) | priority;
				}

				/* advance in X */
				cx += incxx;
				cy += incxy;
				x++;
				dest = (UINT8 *)dest + destadvance;
				pri++;
			}

			/* advance in Y */
			startx += incyx;
			starty += incyy;
			sy++;
		}
	}

	/* non-wraparound case */
	else
	{
		/* loop over rows */
		while (sy <= ey)
		{
			/* initialize X counters */
			x = sx;
			cx = startx;
			cy = starty;

			/* get dest and priority pointers */
			dest = (UINT8 *)destbitmap->base + (destbitmap->rowpixels * sy + sx) * destadvance;
			pri = BITMAP_ADDR8(priority_bitmap, sy, sx);

			/* loop over columns */
			while (x <= ex)
			{
				/* plot if we're within the bitmap and we match the mask */
				if (cx < widthshifted && cy < heightshifted)
					if ((*BITMAP_ADDR8(flagsmap, cy >> 16, cx >> 16) & mask) == value)
					{
						ROZ_PLOT_PIXEL(*BITMAP_ADDR16(srcbitmap, cy >> 16, cx >> 16));
						*pri = (*pri & (priority >> 8)) | priority;
					}

				/* advance in X */
				cx += incxx;
				cy += incxy;
				x++;
				dest = (UINT8 *)dest + destadvance;
				pri++;
			}

			/* advance in Y */
			startx += incyx;
			starty += incyy;
			sy++;
		}
	}
}



/***************************************************************************
    SCANLINE RASTERIZERS
***************************************************************************/

/*-------------------------------------------------
    scanline_draw_opaque_null - draw to a NULL
    bitmap, setting priority only
-------------------------------------------------*/

static void scanline_draw_opaque_null(void *dest, const UINT16 *source, int count, const pen_t *pens, UINT8 *pri, UINT32 pcode, UINT8 alpha)
{
	int i;

	/* skip entirely if not changing priority */
	if (pcode != 0xff00)
	{
		for (i = 0; i < count; i++)
			pri[i] = (pri[i] & (pcode >> 8)) | pcode;
	}
}


/*-------------------------------------------------
    scanline_draw_masked_null - draw to a NULL
    bitmap using a mask, setting priority only
-------------------------------------------------*/

static void scanline_draw_masked_null(void *dest, const UINT16 *source, const UINT8 *maskptr, int mask, int value, int count, const pen_t *pens, UINT8 *pri, UINT32 pcode, UINT8 alpha)
{
	int i;

	/* skip entirely if not changing priority */
	if (pcode != 0xff00)
	{
		for (i = 0; i < count; i++)
			if ((maskptr[i] & mask) == value)
				pri[i] = (pri[i] & (pcode >> 8)) | pcode;
	}
}



/*-------------------------------------------------
    scanline_draw_opaque_ind16 - draw to a 16bpp
    indexed bitmap
-------------------------------------------------*/

static void scanline_draw_opaque_ind16(void *_dest, const UINT16 *source, int count, const pen_t *pens, UINT8 *pri, UINT32 pcode, UINT8 alpha)
{
	UINT16 *dest = (UINT16 *)_dest;
	int pal = pcode >> 16;
	int i;

	/* special case for no palette offset */
	if (pal == 0)
	{
		memcpy(dest, source, count * 2);

		/* priority if necessary */
		if (pcode != 0xff00)
		{
			for (i = 0; i < count; i++)
				pri[i] = (pri[i] & (pcode >> 8)) | pcode;
		}
	}

	/* priority case */
	else if ((pcode & 0xffff) != 0xff00)
	{
		for (i = 0; i < count; i++)
		{
			dest[i] = source[i] + pal;
			pri[i] = (pri[i] & (pcode >> 8)) | pcode;
		}
	}

	/* no priority case */
	else
	{
		for (i = 0; i < count; i++)
			dest[i] = source[i] + pal;
	}
}


/*-------------------------------------------------
    scanline_draw_masked_ind16 - draw to a 16bpp
    indexed bitmap using a mask
-------------------------------------------------*/

static void scanline_draw_masked_ind16(void *_dest, const UINT16 *source, const UINT8 *maskptr, int mask, int value, int count, const pen_t *pens, UINT8 *pri, UINT32 pcode, UINT8 alpha)
{
	UINT16 *dest = (UINT16 *)_dest;
	int pal = pcode >> 16;
	int i;

	/* priority case */
	if ((pcode & 0xffff) != 0xff00)
	{
		for (i = 0; i < count; i++)
			if ((maskptr[i] & mask) == value)
			{
				dest[i] = source[i] + pal;
				pri[i] = (pri[i] & (pcode >> 8)) | pcode;
			}
	}

	/* no priority case */
	else
	{
		for (i = 0; i < count; i++)
			if ((maskptr[i] & mask) == value)
				dest[i] = source[i] + pal;
	}
}



/*-------------------------------------------------
    scanline_draw_opaque_rgb16 - draw to a 16bpp
    RGB bitmap
-------------------------------------------------*/

static void scanline_draw_opaque_rgb16(void *_dest, const UINT16 *source, int count, const pen_t *pens, UINT8 *pri, UINT32 pcode, UINT8 alpha)
{
	const pen_t *clut = &pens[pcode >> 16];
	UINT16 *dest = (UINT16 *)_dest;
	int i;

	/* priority case */
	if ((pcode & 0xffff) != 0xff00)
	{
		for (i = 0; i < count; i++)
		{
			dest[i] = clut[source[i]];
			pri[i] = (pri[i] & (pcode >> 8)) | pcode;
		}
	}

	/* no priority case */
	else
	{
		for (i = 0; i < count; i++)
			dest[i] = clut[source[i]];
	}
}


/*-------------------------------------------------
    scanline_draw_masked_rgb16 - draw to a 16bpp
    RGB bitmap using a mask
-------------------------------------------------*/

static void scanline_draw_masked_rgb16(void *_dest, const UINT16 *source, const UINT8 *maskptr, int mask, int value, int count, const pen_t *pens, UINT8 *pri, UINT32 pcode, UINT8 alpha)
{
	const pen_t *clut = &pens[pcode >> 16];
	UINT16 *dest = (UINT16 *)_dest;
	int i;

	/* priority case */
	if ((pcode & 0xffff) != 0xff00)
	{
		for (i = 0; i < count; i++)
			if ((maskptr[i] & mask) == value)
			{
				dest[i] = clut[source[i]];
				pri[i] = (pri[i] & (pcode >> 8)) | pcode;
			}
	}

	/* no priority case */
	else
	{
		for (i = 0; i < count; i++)
			if ((maskptr[i] & mask) == value)
				dest[i] = clut[source[i]];
	}
}


/*-------------------------------------------------
    scanline_draw_opaque_rgb16_alpha - draw to a
    16bpp RGB bitmap with alpha blending
-------------------------------------------------*/

static void scanline_draw_opaque_rgb16_alpha(void *_dest, const UINT16 *source, int count, const pen_t *pens, UINT8 *pri, UINT32 pcode, UINT8 alpha)
{
	const pen_t *clut = &pens[pcode >> 16];
	UINT16 *dest = (UINT16 *)_dest;
	int i;

	/* priority case */
	if ((pcode & 0xffff) != 0xff00)
	{
		for (i = 0; i < count; i++)
		{
			dest[i] = alpha_blend_r16(dest[i], clut[source[i]], alpha);
			pri[i] = (pri[i] & (pcode >> 8)) | pcode;
		}
	}

	/* no priority case */
	else
	{
		for (i = 0; i < count; i++)
			dest[i] = alpha_blend_r16(dest[i], clut[source[i]], alpha);
	}
}


/*-------------------------------------------------
    scanline_draw_masked_rgb16_alpha - draw to a
    16bpp RGB bitmap using a mask and alpha
    blending
-------------------------------------------------*/

static void scanline_draw_masked_rgb16_alpha(void *_dest, const UINT16 *source, const UINT8 *maskptr, int mask, int value, int count, const pen_t *pens, UINT8 *pri, UINT32 pcode, UINT8 alpha)
{
	const pen_t *clut = &pens[pcode >> 16];
	UINT16 *dest = (UINT16 *)_dest;
	int i;

	/* priority case */
	if ((pcode & 0xffff) != 0xff00)
	{
		for (i = 0; i < count; i++)
			if ((maskptr[i] & mask) == value)
			{
				dest[i] = alpha_blend_r16(dest[i], clut[source[i]], alpha);
				pri[i] = (pri[i] & (pcode >> 8)) | pcode;
			}
	}

	/* no priority case */
	else
	{
		for (i = 0; i < count; i++)
			if ((maskptr[i] & mask) == value)
				dest[i] = alpha_blend_r16(dest[i], clut[source[i]], alpha);
	}
}


/*-------------------------------------------------
    scanline_draw_opaque_rgb32 - draw to a 32bpp
    RGB bitmap
-------------------------------------------------*/

static void scanline_draw_opaque_rgb32(void *_dest, const UINT16 *source, int count, const pen_t *pens, UINT8 *pri, UINT32 pcode, UINT8 alpha)
{
	const pen_t *clut = &pens[pcode >> 16];
	UINT32 *dest = (UINT32 *)_dest;
	int i;

	/* priority case */
	if ((pcode & 0xffff) != 0xff00)
	{
		for (i = 0; i < count; i++)
		{
			dest[i] = clut[source[i]];
			pri[i] = (pri[i] & (pcode >> 8)) | pcode;
		}
	}

	/* no priority case */
	else
	{
		for (i = 0; i < count; i++)
			dest[i] = clut[source[i]];
	}
}


/*-------------------------------------------------
    scanline_draw_masked_rgb32 - draw to a 32bpp
    RGB bitmap using a mask
-------------------------------------------------*/

static void scanline_draw_masked_rgb32(void *_dest, const UINT16 *source, const UINT8 *maskptr, int mask, int value, int count, const pen_t *pens, UINT8 *pri, UINT32 pcode, UINT8 alpha)
{
	const pen_t *clut = &pens[pcode >> 16];
	UINT32 *dest = (UINT32 *)_dest;
	int i;

	/* priority case */
	if ((pcode & 0xffff) != 0xff00)
	{
		for (i = 0; i < count; i++)
			if ((maskptr[i] & mask) == value)
			{
				dest[i] = clut[source[i]];
				pri[i] = (pri[i] & (pcode >> 8)) | pcode;
			}
	}

	/* no priority case */
	else
	{
		for (i = 0; i < count; i++)
			if ((maskptr[i] & mask) == value)
				dest[i] = clut[source[i]];
	}
}


/*-------------------------------------------------
    scanline_draw_opaque_rgb32_alpha - draw to a
    32bpp RGB bitmap with alpha blending
-------------------------------------------------*/

static void scanline_draw_opaque_rgb32_alpha(void *_dest, const UINT16 *source, int count, const pen_t *pens, UINT8 *pri, UINT32 pcode, UINT8 alpha)
{
	const pen_t *clut = &pens[pcode >> 16];
	UINT32 *dest = (UINT32 *)_dest;
	int i;

	/* priority case */
	if ((pcode & 0xffff) != 0xff00)
	{
		for (i = 0; i < count; i++)
		{
			dest[i] = alpha_blend_r32(dest[i], clut[source[i]], alpha);
			pri[i] = (pri[i] & (pcode >> 8)) | pcode;
		}
	}

	/* no priority case */
	else
	{
		for (i = 0; i < count; i++)
			dest[i] = alpha_blend_r32(dest[i], clut[source[i]], alpha);
	}
}


/*-------------------------------------------------
    scanline_draw_masked_rgb32_alpha - draw to a
    32bpp RGB bitmap using a mask and alpha
    blending
-------------------------------------------------*/

static void scanline_draw_masked_rgb32_alpha(void *_dest, const UINT16 *source, const UINT8 *maskptr, int mask, int value, int count, const pen_t *pens, UINT8 *pri, UINT32 pcode, UINT8 alpha)
{
	const pen_t *clut = &pens[pcode >> 16];
	UINT32 *dest = (UINT32 *)_dest;
	int i;

	/* priority case */
	if ((pcode & 0xffff) != 0xff00)
	{
		for (i = 0; i < count; i++)
			if ((maskptr[i] & mask) == value)
			{
				dest[i] = alpha_blend_r32(dest[i], clut[source[i]], alpha);
				pri[i] = (pri[i] & (pcode >> 8)) | pcode;
			}
	}

	/* no priority case */
	else
	{
		for (i = 0; i < count; i++)
			if ((maskptr[i] & mask) == value)
				dest[i] = alpha_blend_r32(dest[i], clut[source[i]], alpha);
	}
}
