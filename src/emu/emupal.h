/******************************************************************************

    emupal.h

    Emulator palette handling functions.

    Copyright Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

    There are several levels of abstraction in the way MAME handles the palette,
    and several display modes which can be used by the drivers.

    Palette
    -------
    Note: in the following text, "color" refers to a color in the emulated
    game's virtual palette. For example, a game might be able to display 1024
    colors at the same time. If the game uses RAM to change the available
    colors, the term "palette" refers to the colors available at any given time,
    not to the whole range of colors which can be produced by the hardware. The
    latter is referred to as "color space".
    The term "pen" refers to one of the maximum MAX_PENS colors that can be
    used to generate the display.

    So, to summarize, the three layers of palette abstraction are:

    P1) The game virtual palette (the "colors")
    P2) MAME's MAX_PENS colors palette (the "pens")
    P3) The OS specific hardware color registers (the "OS specific pens")

    The array Machine->pens[] is a lookup table which maps game colors to OS
    specific pens (P1 to P3). When you are working on bitmaps at the pixel level,
    *always* use Machine->pens to map the color numbers. *Never* use constants.
    For example if you want to make pixel (x,y) of color 3, do:
    *BITMAP_ADDR(bitmap, <type>, y, x) = Machine->pens[3];


    Lookup table
    ------------
    Palettes are only half of the story. To map the gfx data to colors, the
    graphics routines use a lookup table. For example if we have 4bpp tiles,
    which can have 256 different color codes, the lookup table for them will have
    256 * 2^4 = 4096 elements. For games using a palette RAM, the lookup table is
    usually a 1:1 map. For games using PROMs, the lookup table is often larger
    than the palette itself so for example the game can display 256 colors out
    of a palette of 16.

    The palette and the lookup table are initialized to default values by the
    main core, but can be initialized by the driver using the function
    MachineDriver->vh_init_palette(). For games using palette RAM, that
    function is usually not needed, and the lookup table can be set up by
    properly initializing the color_codes_start and total_color_codes fields in
    the GfxDecodeInfo array.
    When vh_init_palette() initializes the lookup table, it maps gfx codes
    to game colors (P1 above). The lookup table will be converted by the core to
    map to OS specific pens (P3 above), and stored in Machine->remapped_colortable.


    Display modes
    -------------
    The available display modes can be summarized in three categories:
    1) Static palette. Use this for games which use PROMs for color generation.
        The palette is initialized by palette_init(), and never changed
        again.
    2) Dynamic palette. Use this for games which use RAM for color generation.
        The palette can be dynamically modified by the driver using the function
        palette_set_color().
    3) Direct mapped 16-bit or 32-bit color. This should only be used in special
        cases, e.g. to support alpha blending.
        MachineDriver->video_attributes must contain VIDEO_RGB_DIRECT.


    Shadows(Highlights) Quick Reference
    -----------------------------------

    1) declare MCFG_VIDEO_ATTRIBUTES( ... VIDEO_HAS_SHADOWS | VIDEO_HAS_HIGHLIGHTS ... )

    2) make a pen table fill with DRAWMODE_NONE, DRAWMODE_SOURCE or DRAWMODE_SHADOW

    3) (optional) set shadow darkness or highlight brightness by
        palette_set_shadow_factor(0.0-1.0) or
        palette_set_highlight_factor(1.0-n.n)

    4) before calling drawgfx use
        palette_set_shadow_mode(0) to arm shadows or
        palette_set_shadow_mode(1) to arm highlights

    5) call drawgfx_transtable
        drawgfx_transtable( ..., pentable )

******************************************************************************/

#pragma once

#ifndef __EMU_H__
#error Dont include this file directly; include emu.h instead.
#endif

#ifndef __EMUPAL_H__
#define __EMUPAL_H__


/***************************************************************************
    CONSTANTS
***************************************************************************/

#define PALETTE_DEFAULT_SHADOW_FACTOR (0.6)
#define PALETTE_DEFAULT_HIGHLIGHT_FACTOR (1/PALETTE_DEFAULT_SHADOW_FACTOR)



/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

class colortable_t;



/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/


/* ----- initialization and configuration ----- */

/* palette initialization that takes place before the display is created */
void palette_init(running_machine &machine);



/* ----- shadow/hilight configuration ----- */

/* set the global shadow brightness factor */
void palette_set_shadow_factor(running_machine &machine, double factor);

/* set the global highlight brightness factor */
void palette_set_highlight_factor(running_machine &machine, double factor);



/* ----- shadow table configuration ----- */

/* select 1 of 4 different live shadow tables */
void palette_set_shadow_mode(running_machine &machine, int mode);

/* configure delta RGB values for 1 of 4 shadow tables */
void palette_set_shadow_dRGB32(running_machine &machine, int mode, int dr, int dg, int db, int noclip);



/* ----- colortable management ----- */

/* allocate a new colortable with the given number of entries */
colortable_t *colortable_alloc(running_machine &machine, UINT32 palettesize);

/* set the value of a colortable entry */
void colortable_entry_set_value(colortable_t *ctable, UINT32 entry, UINT16 value);

/* return the value of a colortable entry */
UINT16 colortable_entry_get_value(colortable_t *ctable, UINT32 entry);

/* change the color of a colortable palette entry */
void colortable_palette_set_color(colortable_t *ctable, UINT32 entry, rgb_t color);

/* return the color of a colortable palette entry */
rgb_t colortable_palette_get_color(colortable_t *ctable, UINT32 entry);

/* return a 32-bit transparency mask for a given gfx element and color */
UINT32 colortable_get_transpen_mask(colortable_t *ctable, gfx_element *gfx, int color, int transcolor);

/* configure groups in a tilemap to represent transparency based on colortable entries (each group maps to a gfx color) */
void colortable_configure_tilemap_groups(colortable_t *ctable, tilemap_t *tmap, gfx_element *gfx, int transcolor);

/* return the number of entries in a colortable */
UINT32 colortable_palette_get_size(colortable_t *ctable);



/* ----- utilities ----- */

/* return the pen for a fixed black color */
pen_t get_black_pen(running_machine &machine);

/* return the pen for a fixed white color */
pen_t get_white_pen(running_machine &machine);




/***************************************************************************
    INLINE FUNCTIONS
***************************************************************************/

/*-------------------------------------------------
    palette_set_color - set a single palette
    entry
-------------------------------------------------*/

INLINE void palette_set_color(running_machine &machine, pen_t pen, rgb_t rgb)
{
	palette_entry_set_color(machine.palette, pen, rgb);
}


/*-------------------------------------------------
    palette_set_color_rgb - set a single palette
    entry with individual R,G,B components
-------------------------------------------------*/

INLINE void palette_set_color_rgb(running_machine &machine, pen_t pen, UINT8 r, UINT8 g, UINT8 b)
{
	palette_entry_set_color(machine.palette, pen, MAKE_RGB(r, g, b));
}


/*-------------------------------------------------
    palette_get_color - return a single palette
    entry
-------------------------------------------------*/

INLINE rgb_t palette_get_color(running_machine &machine, pen_t pen)
{
	return palette_entry_get_color(machine.palette, pen);
}


/*-------------------------------------------------
    palette_set_pen_contrast - set the per-pen
    contrast factor
-------------------------------------------------*/

INLINE void palette_set_pen_contrast(running_machine &machine, pen_t pen, double bright)
{
	palette_entry_set_contrast(machine.palette, pen, bright);
}


/*-------------------------------------------------
    palette_set_colors - set multiple palette
    entries from an array of rgb_t values
-------------------------------------------------*/

INLINE void palette_set_colors(running_machine &machine, pen_t color_base, const rgb_t *colors, int color_count)
{
	while (color_count--)
		palette_entry_set_color(machine.palette, color_base++, *colors++);
}


#endif  /* __PALETTE_H__ */
