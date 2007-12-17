/***************************************************************************

    tilemap.h

    Generic tilemap management system.

    Copyright (c) 1996-2007, Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

****************************************************************************

    Core concepts:

        Tilemap = a 2-dimensional array of tiles; each tile has its own
            independent characteristics which describe how that tile is
            rendered. A tilemap is described by the number of rows and
            columns in the map. Each tile in the tilemap is a fixed size
            specified as width and height in pixels.

        Tile = a single rectangular element in a tilemap; tiles can be
            any size, but all tiles in a tilemap are the same size. Each
            tile is described by the following parameters:

            pen_data (required): pointer to an array of 8bpp pen data
                describing the raw pixels to be rendered for a tile;
                for example, an 8x8 tile requires 64 bytes of pen_data

            palette_base (required): the base index in the global palette;
                each pixel fetched from pen_data will have this base added
                to it before it is stored in the pixmap

            mask_data (optional): pointer to an array of 1bpp mask data
                which controls which pixels are to be considered visible
                or transparent (part of no layer); data is packed MSB
                first with no padding, so an 8x8 tile requires 64 bits or
                8 bytes data

            category (optional): specifies one of 16 categories for the
                pixels in the tile; the category controls which tiles are
                rendered during a tilemap_draw() call

            group (optional): specifies one of 256 groups for pen mapping;
                each pen in the tile is looked up in a table to determine
                which layer(s) it belongs to, and the group selects one of
                256 different tables to use for this mapping

            flags (optional): specifies one or more of the following bits:

                TILE_FLIPX means render the tile flipped horizontally

                TILE_FLIPY means render the tile flipped vertically

                TILE_4BPP means pen_data is actually packed 4bpp pixels
                    (low 4 bits first, then upper 4 bits)

                TILE_FORCE_LAYERn means all pixels are forced to be in
                    layer n, where n = 0,1,2

        Pen = another name for pixel value; tilemap graphics are either
            4 or 8 bits per pixel, meaning that pen values range from
            0-15 (4bpp) or 0-255 (8bpp).

        Logical index = index of a tile in a tilemap, assuming perfect
            packing in row-major order; this is always equal to
            (rownum * tilemap_columns + colnum).

        Memory index = index of a tile in memory; this often does not
            map 1:1 to the logical index (though it can). A mapper
            function is provided when creating a new tilemap that can
            map from (column,row) to memory index.

        Layer = one of 3 categories each pixel can belong to, based on
            the pens in a tile; traditionally, layer 0 is the only
            one used. If a pixel does not belong to any layers, it is
            considered transparent.

        Group = one of 256 per-tile selectable means of mapping from
            pens to layers; traditionally only group 0 is used.

        Category = a 4-bit value specified per-tile which allows for
            separating tiles into categories which can be rendered
            independently.

        Pixmap = a 16bpp bitmap containing the full tilemap with
            all tiles rendered.

        Flagsmap = an 8bpp bitmap containing per-pixel flags,
            specifically the category (specified in bits 0-3) and
            the layer (specified in bits 4-6).

****************************************************************************

    How to use a tilemap:

    1. First create a new tilemap by calling tilemap_create(). The
        parameters are as follows:

        tile_get_info = pointer to a callback function which accepts a
            memory index and in return fills in a tile_info structure
            that describes the characteristics of a tile; this function
            will be called whenever a dirty tile needs to be rendered

        mapper = pointer to a callback function which maps the logical
            column and row to a memory index; several standard mappers
            are provided, with tilemap_scan_rows being the most common

        type = the type of tilemap this is;

            TILEMAP_TYPE_PEN - this is the most common type,
                and means that transparency is determined by taking the
                raw pen value and looking it up in a table to determine
                which layer(s) each pixel belongs to

            TILEMAP_TYPE_COLORTABLE - this is less common and
                will eventually be deprecated; it means that transparency
                is determined by taking the raw pen value, looking it up
                in the machine's colortable, and then looking that value
                up in a table to determine which layer(s) each pixel
                belongs to

        tilewidth = the width, in pixels, of each individual tile

        tileheight = the height, in pixels, of each individual tile

        cols = the number of columns in the tilemap

        rows = the number of rows in the tilemap

    2. Once you have created your tilemap, you need to configure it.
        Common configuration tasks include:

            * marking one of the pens as transparent via
                tilemap_set_transparent_pen()

            * performing more complex pen-to-layer mapping via
                tilemap_map_pen_to_layer() or
                tilemap_map_pens_to_layer()

            * configuring global scroll offsets via
                tilemap_set_scrolldx() and tilemap_set_scrolldy()

            * specifying a pointer that is passed to your tile_get_info
                callback via tilemap_set_user_data()

            * setting a global palette offset via
                tilemap_set_palette_offset()

    3. In your memory write handlers for the tile memory, anytime tile
        data is modified, you need to mark the tile dirty so that it is
        re-rendered with the new data the next time the tilemap is drawn.
        Use tilemap_mark_tile_dirty() and pass in the memory index.

    4. In your handlers for scrolling, update the scroll values for the
        tilemap via tilemap_set_scrollx() and tilemap_set_scrolly().

    5. If any other major characteristics of the tilemap change (generally
        any global state that is used by the tile_get_info callback but
        which is not reported via other calls to the tilemap code), you
        should invalidate the entire tilemap. You can do this by calling
        tilemap_mark_all_tiles_dirty().

    6. In your VIDEO_UPDATE callback, render the tiles by calling
        tilemap_draw() or tilemap_draw_roz(). If you need to do custom
        rendering and want access to the raw pixels, call
        tilemap_get_pixmap() to get a pointer to the updated mame_bitmap
        containing the tilemap graphics.

****************************************************************************

    The following example shows how to use the tilemap system to create
    a simple tilemap where pen 0 is transparent. Each tile is 8x8 pixels
    in size, and there are 32 rows and 64 columns, stored in row-major
    order. Based on bits in the tile memory, tiles can be drawn either
    behind or in front of sprites.

        tilemap *tmap;
        UINT16 *my_tmap_memory;

        TILE_GET_INFO( my_get_info )
        {
            UINT8 tiledata = my_tmap_memory[tile_index];
            UINT8 code = tiledata & 0xff;
            UINT8 color = (tiledata >> 8) & 0x1f;
            UINT8 flipx = (tiledata >> 13) & 1;
            UINT8 flipy = (tiledata >> 14) & 1;
            UINT8 category = (tiledata >> 15) & 1;

            // set the common info for the tile
            SET_TILE_INFO(
                1,              // use machine->gfx[1] for tile graphics
                code,           // the index of the graphics for this tile
                color,          // the color to use for this tile
                (flipx ? TILE_FLIPX : 0) |  // flags for this tile; also
                (flipy ? TILE_FLIPY : 0);   // see the FLIP_YX macro
            );

            // set the category of each tile based on the high bit; this
            // allows us to draw each category independently
            tileinfo->category = category;
        }

        VIDEO_START( mydriver )
        {
            // first create the tilemap
            tmap = tilemap_create(
                    my_get_info,            // pointer to your get_info
                    tilemap_scan_rows,      // standard row-major mapper
                    TILEMAP_TYPE_PEN, // transparency from pens
                    8,8,                    // 8x8 tiles
                    64,32);                 // 64 columns, 32 rows

            // then set the transparent pen; all other pens will default
            // to being part of layer 0
            tilemap_set_transparent_pen(tmap, 0);
        }

        VIDEO_UPDATE( mydriver )
        {
            // draw the tilemap first, fully opaque since it needs to
            // erase all previous pixels
            tilemap_draw(
                bitmap,                 // destination bitmap
                cliprect,               // clipping rectangle
                tmap,                   // tilemap to draw
                TILEMAP_DRAW_OPAQUE,    // flags
                0);                     // don't use priority_bitmap

            // next draw the sprites
            my_draw_sprites();

            // then draw the tiles which have priority over sprites
            tilemap_draw(
                bitmap,                 // destination bitmap
                cliprect,               // clipping rectangle
                tmap,                   // tilemap to draw
                TILEMAP_DRAW_CATEGORY(1),// flags: draw category 1
                0);                     // don't use priority_bitmap

            return 0;
        }

****************************************************************************

    Tilemap techniques:

    * The previous tilemap code specified a number of different tilemap
        types that were fundamentally similar. This has been collapsed
        into just two types now. Here is how to replicate that
        functionality with the new system:

        TILEMAP_OPAQUE: If you want a tilemap with no transparency,
            you don't need to do anything; by default all pens map to
            layer 0 and are thus non-transparent. Note that a lot of
            code used to create OPAQUE tilemaps and then set the
            transparent pen for them; this no longer works.

        TILEMAP_TRANSPARENT: This described a tilemap with a single
            transparent pen. To create the same effect, make a new tilemap
            of type TILEMAP_TYPE_PEN, and then call
            tilemap_set_transparent_pen() to specify which pen is
            transparent; all other pens will map to layer 0.

        TILEMAP_TRANSPARENT_COLOR: This works just like before and
            even has the same type name. Create a tilemap of type
            TILEMAP_TYPE_COLORTABLE and call
            tilemap_set_transparent_pen() to specify which remapped pen
            is transparent.

        TILEMAP_BITMASK: This type is no longer special; with the new
            code, any tile_get_info callback can specify a bitmask which
            will be processed after rendering to make some pixels
            transparent.

        TILEMAP_SPLIT: This type used to let you map pens into two
            layers (called "front" and "back") based on their pens. It
            also allowed for you to choose one of 4 mappings on a per-tile
            basis. All of this functionality is now expanded: you can
            specify one of 3 layers and can choose from one of 256 mappings
            on a per-tile basis. You just create a tilemap of type
            TILEMAP_TYPE_PEN and call tilemap_set_transmask(),
            which still exists but maps onto the new behavior. The "front"
            layer is now "layer 0" and the "back" layer is now "layer 1".

        TILEMAP_SPLIT_PENBIT: This type was only used in one driver
            and is not worth describing in detail how the new mapping
            works. :)

    * By far the most common usage of layers is the simplest: layer 0 is
        the only one used and rendered. If you want a particular tile to
        render opaque, you can set the TILE_FORCE_LAYER0 file in your
        tile_get_info callback.

    * When drawing, you can specify in the flags which classes of pixels
        to draw. In the low 4 bits, you specify the category; this must
        match exactly the category specified for the tile in order to
        render. In addition, the flags TILEMAP_DRAW_LAYERn allow you to
        control which layer pattern to draw; note that combining these
        flags may produce interesting results. Specifying both layer 0
        and layer 1 will render only pixels that are in *both* layers.
        If you want to render everything, regardless of layer, specify
        the TILEMAP_DRAW_OPAQUE flag. If you don't specify any layers,
        TILEMAP_DRAW_LAYER0 is assumed.

    * If you want to render with alpha blending, you can call
        tilemap_draw() with the TILEMAP_DRAW_ALPHA flag. To configure the
        amount of alpha blending, call alpha_set_level() ahead of time.

    * To configure more complex pen-to-layer mapping, use the
        tilemap_map_pens_to_layer() call. This call takes a group number
        so that you can configure 1 of the 256 groups independently.
        It also takes a pen and a mask; the mapping is updated for all
        pens where ((pennum & mask) == pen). To set all the pens in a
        group to the same value, pass a mask of 0. To set a single pen in
        a group, pass a mask of ~0. The helper function
        tilemap_map_pen_to_layer() does this for you.

***************************************************************************/

#pragma once

#ifndef __TILEMAP_H__
#define __TILEMAP_H__

#include "mamecore.h"


/***************************************************************************
    CONSTANTS
***************************************************************************/

/* ALL_TILEMAPS may be used with: tilemap_set_flip, tilemap_mark_all_tiles_dirty */
#define ALL_TILEMAPS					NULL


/* maximum number of groups */
#define TILEMAP_NUM_GROUPS				256


/* these flags control tilemap_draw() behavior */
#define TILEMAP_DRAW_CATEGORY_MASK		0x0f		/* specify the category to draw */
#define TILEMAP_DRAW_CATEGORY(x)		(x)			/* specify category to draw */
#define TILEMAP_DRAW_LAYER0				0x10		/* draw layer 0 */
#define TILEMAP_DRAW_LAYER1				0x20		/* draw layer 1 */
#define TILEMAP_DRAW_LAYER2				0x40		/* draw layer 2 */
#define TILEMAP_DRAW_OPAQUE				0x80		/* draw everything, even transparent stuff */
#define TILEMAP_DRAW_ALPHA				0x100		/* draw with alpha blending */


/* per-pixel flags in the transparency_bitmap */
#define TILEMAP_PIXEL_CATEGORY_MASK		0x0f		/* category is stored in the low 4 bits */
#define TILEMAP_PIXEL_TRANSPARENT		0x00		/* transparent if in none of the layers below */
#define TILEMAP_PIXEL_LAYER0			0x10		/* pixel is opaque in layer 0 */
#define TILEMAP_PIXEL_LAYER1			0x20		/* pixel is opaque in layer 1 */
#define TILEMAP_PIXEL_LAYER2			0x40		/* pixel is opaque in layer 2 */


/* per-tile flags, set by get_tile_info callback */
#define TILE_FLIPX						0x01		/* draw this tile horizontally flipped */
#define TILE_FLIPY						0x02		/* draw this tile vertically flipped */
#define TILE_4BPP						0x04		/* tile data is packed 4bpp */
#define TILE_FORCE_LAYER0				TILEMAP_PIXEL_LAYER0 /* force all pixels to be layer 0 (no transparency) */
#define TILE_FORCE_LAYER1				TILEMAP_PIXEL_LAYER1 /* force all pixels to be layer 1 (no transparency) */
#define TILE_FORCE_LAYER2				TILEMAP_PIXEL_LAYER2 /* force all pixels to be layer 2 (no transparency) */


/* tilemap global flags, used by tilemap_set_flip() */
#define TILEMAP_FLIPX					TILE_FLIPX	/* draw the tilemap horizontally flipped */
#define TILEMAP_FLIPY					TILE_FLIPY	/* draw the tilemap vertically flipped */


/* set this value for a scroll row/column to fully disable it */
#define TILE_LINE_DISABLED				0x80000000



/***************************************************************************
    MACROS
***************************************************************************/

/* function definition for a get info callback */
#define TILE_GET_INFO(_name)			void _name(running_machine *machine, tile_data *tileinfo, tilemap_memory_index tile_index, void *param)

/* function definition for a logical-to-memory mapper */
#define TILEMAP_MAPPER(_name)			tilemap_memory_index _name(UINT32 col, UINT32 row, UINT32 num_cols, UINT32 num_rows)

/* useful macro inside of a TILE_GET_INFO callback to set tile information  */
#define SET_TILE_INFO(GFX,CODE,COLOR,FLAGS) tileinfo_set(machine, tileinfo, GFX, CODE, COLOR, FLAGS)

/* Macros for setting tile attributes in the TILE_GET_INFO callback: */
/*   TILE_FLIP_YX assumes that flipy is in bit 1 and flipx is in bit 0 */
/*   TILE_FLIP_XY assumes that flipy is in bit 0 and flipx is in bit 1 */
/*   TILE_GROUP shifts a split group number appropriately to OR into the tile flags */
#define TILE_FLIPYX(YX)					((YX) & 3)
#define TILE_FLIPXY(XY)					((((XY) & 2) >> 1) | (((XY) & 1) << 1))



/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

/* memory indexes are unsigned integers */
typedef UINT32 tilemap_memory_index;

/* opaque tilemap definition */
typedef struct _tilemap tilemap;


/* tilemap types */
enum _tilemap_type
{
	TILEMAP_TYPE_PEN,				/* pen-to-layer mapping is determined by pen lookup */
	TILEMAP_TYPE_COLORTABLE			/* pen-to-layer mapping is determined by colortable[pen] lookup */
};
typedef enum _tilemap_type tilemap_type;


/* tile_data is filled in by the get_tile_info callback */
typedef struct _tile_data tile_data;
struct _tile_data
{
	const UINT8 *	pen_data;		/* required */
	const UINT8 *	mask_data;		/* required for TILEMAP_TYPE_PEN */
	pen_t			palette_base;	/* defaults to 0 */
	UINT8 			category;		/* defaults to 0; range from 0..15 */
	UINT8			group;			/* defaults to 0; range from 0..TILEMAP_NUM_GROUPS */
	UINT8 			flags;			/* defaults to 0; one or more of TILE_* flags above */
};


/* callback function to get info about a tile */
typedef void (*tile_get_info_callback)(running_machine *machine, tile_data *tileinfo, tilemap_memory_index tile_index, void *param);

/* callback function to map a column,row pair to a memory index */
typedef tilemap_memory_index (*tilemap_mapper_callback)(UINT32 col, UINT32 row, UINT32 num_cols, UINT32 num_rows);



/***************************************************************************
    GLOBAL VARIABLES
***************************************************************************/

extern mame_bitmap *priority_bitmap;



/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/


/* ----- system-wide management ----- */

/* initialize the tilemap system -- not for use by drivers */
void tilemap_init(running_machine *machine);



/* ----- tilemap creation and configuration ----- */

/* create a new tilemap; note that tilemaps are tracked by the core so there is no dispose */
tilemap *tilemap_create(tile_get_info_callback tile_get_info, tilemap_mapper_callback mapper, tilemap_type type, int tilewidth, int tileheight, int cols, int rows);

/* specify a parameter to be passed into the tile_get_info callback */
void tilemap_set_user_data(tilemap *tmap, void *user_data);

/* specify an offset to be added to each pixel before looking up the palette */
void tilemap_set_palette_offset(tilemap *tmap, UINT32 offset);

/* set an enable flag for the tilemap; if 0, requests to draw the tilemap are ignored */
void tilemap_set_enable(tilemap *tmap, int enable);

/* set a global flip for the tilemap; ALL_TILEMAPS can be passed here as well */
void tilemap_set_flip(tilemap *tmap, UINT32 attributes);



/* ----- dirty tile marking ----- */

/* mark a single tile dirty based on its memory index */
void tilemap_mark_tile_dirty(tilemap *tmap, tilemap_memory_index memory_index);

/* mark all the tiles in a tilemap dirty; ALL_TILEMAPS can be passed here as well */
void tilemap_mark_all_tiles_dirty(tilemap *tmap);



/* ----- pen-to-layer mapping ----- */

/* specify the mapping of one or more pens (where (<pen> & mask == pen) to a layer */
void tilemap_map_pens_to_layer(tilemap *tmap, int group, pen_t pen, pen_t mask, UINT8 layermask);

/* set a single transparent pen into the tilemap, mapping all other pens to layer 0 */
void tilemap_set_transparent_pen(tilemap *tmap, pen_t pen);

/* set up the first 32 pens using a foreground (layer 0) mask and a background (layer 1) mask */
void tilemap_set_transmask(tilemap *tmap, int group, UINT32 fgmask, UINT32 bgmask);



/* ----- tilemap scrolling ----- */

/* specify the number of independently scrollable row units; each unit covers height/scroll_rows pixels */
void tilemap_set_scroll_rows(tilemap *tmap, UINT32 scroll_rows);

/* specify the number of independently scrollable column units; each unit covers width/scroll_cols pixels */
void tilemap_set_scroll_cols(tilemap *tmap, UINT32 scroll_cols);

/* specify global horizontal and vertical scroll offsets, for non-flipped and flipped cases */
void tilemap_set_scrolldx(tilemap *tmap, int dx, int dx_if_flipped);
void tilemap_set_scrolldy(tilemap *tmap, int dy, int dy_if_flipped);

/* return the global horizontal or vertical scroll offset, based on current flip state */
int tilemap_get_scrolldx(tilemap *tmap);
int tilemap_get_scrolldy(tilemap *tmap);

/* specify the scroll value for a row/column unit */
void tilemap_set_scrollx(tilemap *tmap, int row, int value);
void tilemap_set_scrolly(tilemap *tmap, int col, int value);



/* ----- internal map access ----- */

/* return a pointer to the (updated) internal pixmap for a tilemap */
mame_bitmap *tilemap_get_pixmap(tilemap *tmap);

/* return a pointer to the (updated) internal flagsmap for a tilemap */
mame_bitmap *tilemap_get_flagsmap(tilemap *tmap);

/* return a pointer to the (updated) internal per-tile flags for a tilemap */
UINT8 *tilemap_get_tile_flags(tilemap *tmap);



/* ----- tilemap rendering ----- */

/* draw a tilemap to the destination with clipping; pixels apply priority/priority_mask to the priority bitmap */
void tilemap_draw_primask(mame_bitmap *dest, const rectangle *cliprect, tilemap *tmap, UINT32 flags, UINT8 priority, UINT8 priority_mask);

/* draw a tilemap to the destination with clipping and arbitrary rotate/zoom; */
/* pixels apply priority/priority_mask to the priority bitmap */
void tilemap_draw_roz_primask(mame_bitmap *dest, const rectangle *cliprect, tilemap *tmap,
		UINT32 startx, UINT32 starty, int incxx, int incxy, int incyx, int incyy,
		int wraparound, UINT32 flags, UINT8 priority, UINT8 priority_mask);



/* ----- indexed tilemap handling ----- */

/* return the number of tilemaps */
int tilemap_count(void);

/* return the size of an indexed tilemap */
void tilemap_size_by_index(int number, UINT32 *width, UINT32 *height);

/* render an indexed tilemap with fixed characteristics (no priority) */
void tilemap_draw_by_index(mame_bitmap *dest, int number, UINT32 scrollx, UINT32 scrolly);



/* ----- common logical-to-memory mappers ----- */

/* scan in row-major order with optional flipping */
TILEMAP_MAPPER( tilemap_scan_rows );
TILEMAP_MAPPER( tilemap_scan_rows_flip_x );
TILEMAP_MAPPER( tilemap_scan_rows_flip_y );
TILEMAP_MAPPER( tilemap_scan_rows_flip_xy );

/* scan in column-major order with optional flipping */
TILEMAP_MAPPER( tilemap_scan_cols );
TILEMAP_MAPPER( tilemap_scan_cols_flip_x );
TILEMAP_MAPPER( tilemap_scan_cols_flip_y );
TILEMAP_MAPPER( tilemap_scan_cols_flip_xy );



/***************************************************************************
    INLINE FUNCTIONS
***************************************************************************/

/*-------------------------------------------------
    tileinfo_set - set the values of a tileinfo
    structure
-------------------------------------------------*/

INLINE void tileinfo_set(running_machine *machine, tile_data *tileinfo, int gfxnum, int rawcode, int rawcolor, int flags)
{
	const gfx_element *	gfx = machine->gfx[gfxnum];
	int code = rawcode % gfx->total_elements;
	tileinfo->pen_data = gfx->gfxdata + code * gfx->char_modulo;
	tileinfo->palette_base = gfx->color_base + gfx->color_granularity * rawcolor;
	tileinfo->flags = flags;
	if (gfx->flags & GFX_ELEMENT_PACKED)
		tileinfo->flags |= TILE_4BPP;
}


/*-------------------------------------------------
    tilemap_map_pen_to_layer - map a single pen
    to a layer
-------------------------------------------------*/

INLINE void tilemap_map_pen_to_layer(tilemap *tmap, int group, pen_t pen, UINT8 layermask)
{
	tilemap_map_pens_to_layer(tmap, group, pen, ~0, layermask);
}


/*-------------------------------------------------
    tilemap_draw - shortcut to
    tilemap_draw_primask
-------------------------------------------------*/

INLINE void tilemap_draw(mame_bitmap *dest, const rectangle *cliprect, tilemap *tmap, UINT32 flags, UINT8 priority)
{
	tilemap_draw_primask(dest, cliprect, tmap, flags, priority, 0xff);
}


/*-------------------------------------------------
    tilemap_draw_roz - shortcut to
    tilemap_draw_roz_primask
-------------------------------------------------*/

INLINE void tilemap_draw_roz(mame_bitmap *dest, const rectangle *cliprect, tilemap *tmap,
		UINT32 startx, UINT32 starty, int incxx, int incxy, int incyx, int incyy,
		int wraparound, UINT32 flags, UINT8 priority)
{
	tilemap_draw_roz_primask(dest, cliprect, tmap, startx, starty, incxx, incxy, incyx, incyy, wraparound, flags, priority, 0xff);
}



#endif	/* __TILEMAP_H__ */
