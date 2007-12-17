/******************************************************************************

    emupal.c

    Emulator palette handling functions.

    Copyright (c) 1996-2007, Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

******************************************************************************/

#include "driver.h"
#include <math.h>

#define VERBOSE 0


/***************************************************************************
    CONSTANTS
***************************************************************************/

#define PEN_BRIGHTNESS_BITS		8
#define MAX_PEN_BRIGHTNESS		(4 << PEN_BRIGHTNESS_BITS)
#define MAX_SHADOW_PRESETS 		4



/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

/* information about a shadow table */
typedef struct _shadow_table_data shadow_table_data;
struct _shadow_table_data
{
	UINT32 *			base;				/* pointer to the base of the table */
	INT16				dr;					/* delta red value */
	INT16				dg;					/* delta green value */
	INT16				db;					/* delta blue value */
	UINT8				noclip;				/* clip? */
};


/* typedef struct _palette_private palette_private; */
struct _palette_private
{
	bitmap_format		format;				/* format assumed for palette data */

	UINT32 				shadow_group;		/* index of the shadow group, or 0 if none */
	UINT32				hilight_group;		/* index of the hilight group, or 0 if none */

	pen_t 				black_pen;			/* precomputed black pen value */
	pen_t				white_pen;			/* precomputed white pen value */

	shadow_table_data	shadow_table[MAX_SHADOW_PRESETS]; /* array of shadow table data */

	pen_t *				save_pen;			/* pens for save/restore */
	float *				save_bright;		/* brightness for save/restore */
};


/* typedef struct _colortable colortable; */
struct _colortable
{
	running_machine *	machine;			/* associated machine */
	UINT32				entries;			/* number of entries */
	UINT32				palentries;			/* number of palette entries */
	UINT16 *			raw;				/* raw data about each entry */
	rgb_t *				palette;			/* palette entries */
};



/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

static void palette_presave(void);
static void palette_postload(void);
static void palette_exit(running_machine *machine);
static void allocate_palette(running_machine *machine, palette_private *palette);
static void allocate_color_tables(running_machine *machine, palette_private *palette);
static void allocate_shadow_tables(running_machine *machine, palette_private *palette);
static void configure_rgb_shadows(running_machine *machine, int mode, float factor);



/***************************************************************************
    INITIALIZATION AND CONFIGURATION
***************************************************************************/

/*-------------------------------------------------
    palette_init - palette initialization that
    takes place before the display is created
-------------------------------------------------*/

void palette_init(running_machine *machine)
{
	palette_private *palette = auto_malloc(sizeof(*palette));

	/* request cleanup */
	machine->palette_data = palette;
	add_exit_callback(machine, palette_exit);

	/* reset all our data */
	memset(palette, 0, sizeof(*palette));
	palette->format = machine->screen[0].format;

	/* determine the color mode */
	switch (palette->format)
	{
		case BITMAP_FORMAT_INDEXED16:
			/* indexed modes are fine for everything */
			break;

		case BITMAP_FORMAT_RGB15:
		case BITMAP_FORMAT_RGB32:
			/* RGB modes can't use color tables */
			assert(machine->drv->color_table_len == 0);
			break;

		case BITMAP_FORMAT_INVALID:
			/* invalid format means no palette - or at least it should */
			assert(machine->drv->total_colors == 0);
			assert(machine->drv->color_table_len == 0);
			return;

		default:
			fatalerror("Unsupported screen bitmap format!");
			break;
	}

	/* allocate all the data structures */
	if (machine->drv->total_colors > 0)
	{
		int numcolors;

		allocate_palette(machine, palette);
		allocate_color_tables(machine, palette);
		allocate_shadow_tables(machine, palette);

		/* set up save/restore of the palette */
		numcolors = palette_get_num_colors(machine->palette);
		palette->save_pen = auto_malloc(sizeof(*palette->save_pen) * numcolors);
		palette->save_bright = auto_malloc(sizeof(*palette->save_bright) * numcolors);
		state_save_register_global_pointer(palette->save_pen, numcolors);
		state_save_register_global_pointer(palette->save_bright, numcolors);
		state_save_register_func_presave(palette_presave);
		state_save_register_func_postload(palette_postload);
	}
}


/*-------------------------------------------------
    palette_config - palette initialization that
    takes place after the display is created
-------------------------------------------------*/

void palette_config(running_machine *machine)
{
	int total_colors = (machine->palette == NULL) ? 0 : palette_get_num_colors(machine->palette) * palette_get_num_groups(machine->palette);
	pen_t *remapped_colortable = NULL;
	UINT16 *colortable = NULL;
	int i;

	/* allocate memory for the colortables, if needed */
	if (machine->drv->color_table_len > 0)
	{
		/* first for the raw colortable */
		colortable = auto_malloc(machine->drv->color_table_len * sizeof(colortable[0]));
		for (i = 0; i < machine->drv->color_table_len; i++)
			colortable[i] = i % total_colors;

		/* then for the remapped colortable */
		remapped_colortable = auto_malloc(machine->drv->color_table_len * sizeof(remapped_colortable[0]));
	}

	/* now let the driver modify the initial palette and colortable */
	if (machine->drv->init_palette)
		(*machine->drv->init_palette)(machine, colortable, memory_region(REGION_PROMS));

	/* now compute the remapped_colortable */
	for (i = 0; i < machine->drv->color_table_len; i++)
	{
		pen_t color = colortable[i];

		/* check for invalid colors set by machine->drv->init_palette */
		assert(color < total_colors);
		remapped_colortable[i] = machine->pens[color];
	}

	/* now we can set the final colortables */
	machine->game_colortable = colortable;
	machine->remapped_colortable = (remapped_colortable != NULL) ? remapped_colortable : machine->pens;

	/* set the color table base for each graphics element */
	for (i = 0; i < MAX_GFX_ELEMENTS; i++)
		if (machine->gfx[i] != NULL)
			machine->gfx[i]->color_base = machine->drv->gfxdecodeinfo[i].color_codes_start;
}



/***************************************************************************
    SHADOW/HIGHLIGHT CONFIGURATION
***************************************************************************/

/*-------------------------------------------------
    palette_set_shadow_factor - set the global
    shadow brightness factor
-------------------------------------------------*/

void palette_set_shadow_factor(running_machine *machine, double factor)
{
	palette_private *palette = machine->palette_data;

	assert(palette->shadow_group != 0);
	palette_group_set_contrast(machine->palette, palette->shadow_group, factor);
}


/*-------------------------------------------------
    palette_set_highlight_factor - set the global
    highlight brightness factor
-------------------------------------------------*/

void palette_set_highlight_factor(running_machine *machine, double factor)
{
	palette_private *palette = machine->palette_data;

	assert(palette->hilight_group != 0);
	palette_group_set_contrast(machine->palette, palette->hilight_group, factor);
}



/***************************************************************************
    SHADOW TABLE CONFIGURATION
***************************************************************************/

/*-------------------------------------------------
    palette_set_shadow_mode(mode)

        mode: 0 = use preset 0 (default shadow)
              1 = use preset 1 (default highlight)
              2 = use preset 2 *
              3 = use preset 3 *

    * Preset 2 & 3 work independently under 32bpp,
      supporting up to four different types of
      shadows at one time. They mirror preset 1 & 2
      in lower depth settings to maintain
      compatibility.


    palette_set_shadow_dRGB32(mode, dr, dg, db, noclip)

        mode:    0 to   3 (which preset to configure)

          dr: -255 to 255 ( red displacement )
          dg: -255 to 255 ( green displacement )
          db: -255 to 255 ( blue displacement )

        noclip: 0 = resultant RGB clipped at 0x00/0xff
                1 = resultant RGB wraparound 0x00/0xff


    * Color shadows only work under 32bpp.
      This function has no effect in lower color
      depths where

        palette_set_shadow_factor() or
        palette_set_highlight_factor()

      should be used instead.

    * 32-bit shadows are lossy. Even with zero RGB
      displacements the affected area will still look
      slightly darkened.

      Drivers should ensure all shadow pens in
      gfx_drawmode_table[] are set to DRAWMODE_NONE
      when RGB displacements are zero to avoid the
      darkening effect.
-------------------------------------------------*/

/*-------------------------------------------------
    palette_set_shadow_mode - select 1 of 4
    different live shadow tables
-------------------------------------------------*/

void palette_set_shadow_mode(running_machine *machine, int mode)
{
	palette_private *palette = machine->palette_data;
	assert(mode >= 0 && mode < MAX_SHADOW_PRESETS);
	machine->shadow_table = palette->shadow_table[mode].base;
}


/*-------------------------------------------------
    palette_set_shadow_dRGB32 - configure delta
    RGB values for 1 of 4 shadow tables
-------------------------------------------------*/

void palette_set_shadow_dRGB32(running_machine *machine, int mode, int dr, int dg, int db, int noclip)
{
	palette_private *palette = machine->palette_data;
	shadow_table_data *stable = &palette->shadow_table[mode];
	int i;

	/* only applies to RGB direct modes */
	assert(palette->format != BITMAP_FORMAT_INDEXED16);
	assert(stable->base != NULL);

	/* clamp the deltas (why?) */
	if (dr < -0xff) dr = -0xff; else if (dr > 0xff) dr = 0xff;
	if (dg < -0xff) dg = -0xff; else if (dg > 0xff) dg = 0xff;
	if (db < -0xff) db = -0xff; else if (db > 0xff) db = 0xff;

	/* early exit if nothing changed */
	if (dr == stable->dr && dg == stable->dg && db == stable->db && noclip == stable->noclip)
		return;
	stable->dr = dr;
	stable->dg = dg;
	stable->db = db;
	stable->noclip = noclip;

	#if VERBOSE
		popmessage("shadow %d recalc %d %d %d %02x", mode, dr, dg, db, noclip);
	#endif

	/* regenerate the table */
	for (i = 0; i < 32768; i++)
	{
		int r = pal5bit(i >> 10) + dr;
		int g = pal5bit(i >> 5) + dg;
		int b = pal5bit(i >> 0) + db;
		pen_t final;

		/* apply clipping */
		if (!noclip)
		{
			r = rgb_clamp(r);
			g = rgb_clamp(g);
			b = rgb_clamp(b);
		}
		final = MAKE_RGB(r, g, b);

		/* store either 16 or 32 bit */
		if (palette->format == BITMAP_FORMAT_RGB32)
			stable->base[i] = final;
		else
			stable->base[i] = rgb_to_rgb15(final);
	}
}



/***************************************************************************
    COLORTABLE MANAGEMENT
***************************************************************************/

/*-------------------------------------------------
    colortable_alloc - allocate a new colortable
    with the given number of entries
-------------------------------------------------*/

colortable *colortable_alloc(running_machine *machine, UINT32 palettesize)
{
	colortable *ctable;
	UINT32 index;

	/* allocate the colortable */
	ctable = auto_malloc(sizeof(*ctable));
	memset(ctable, 0, sizeof(*ctable));

	/* fill in the basics */
	ctable->machine = machine;
	ctable->entries = machine->drv->total_colors;
	ctable->palentries = palettesize;

	/* allocate the raw colortable */
	ctable->raw = auto_malloc(ctable->entries * sizeof(*ctable->raw));
	for (index = 0; index < ctable->entries; index++)
		ctable->raw[index] = index % ctable->palentries;
	state_save_register_global_pointer(ctable->raw, ctable->entries);

	/* allocate the palette */
	ctable->palette = auto_malloc(ctable->palentries * sizeof(*ctable->palette));
	for (index = 0; index < ctable->palentries; index++)
		ctable->palette[index] = MAKE_ARGB(0x80,0xff,0xff,0xff);
	state_save_register_global_pointer(ctable->palette, ctable->palentries);

	return ctable;
}


/*-------------------------------------------------
    colortable_entry_set_value - set the value
    of a colortable entry
-------------------------------------------------*/

void colortable_entry_set_value(colortable *ctable, UINT32 entry, UINT16 value)
{
	/* ensure values are within range */
	assert(entry < ctable->entries);
	assert(value < ctable->palentries);

	/* update if it has changed */
	if (ctable->raw[entry] != value)
	{
		ctable->raw[entry] = value;
		palette_set_color(ctable->machine, entry, ctable->palette[value]);
	}
}


/*-------------------------------------------------
    colortable_entry_get_value - return the value
    of a colortable entry
-------------------------------------------------*/

UINT16 colortable_entry_get_value(colortable *ctable, UINT32 entry)
{
	assert(entry < ctable->entries);
	return ctable->raw[entry];
}


/*-------------------------------------------------
    colortable_palette_set_color - change the
    color of a colortable palette entry
-------------------------------------------------*/

void colortable_palette_set_color(colortable *ctable, UINT32 entry, rgb_t color)
{
	/* ensure values are within range */
	assert(entry < ctable->palentries);

	/* alpha doesn't matter */
	color &= 0xffffff;

	/* update if it has changed */
	if (ctable->palette[entry] != color)
	{
		UINT32 index;

		ctable->palette[entry] = color;

		/* update the palette for any colortable entries that reference it */
		for (index = 0; index < ctable->entries; index++)
			if (ctable->raw[index] == entry)
				palette_set_color(ctable->machine, index, color);
	}
}


/*-------------------------------------------------
    colortable_entry_get_value - return the color
    of a colortable palette entry
-------------------------------------------------*/

rgb_t colortable_palette_get_color(colortable *ctable, UINT32 entry)
{
	assert(entry < ctable->palentries);
	return ctable->palette[entry];
}


/*-------------------------------------------------
    colortable_get_transpen_mask - return a 32-bit
    transparency mask for a given gfx element and
    color
-------------------------------------------------*/

UINT32 colortable_get_transpen_mask(colortable *ctable, const gfx_element *gfx, int color, int transcolor)
{
	UINT32 entry = gfx->color_base + (color % gfx->total_colors) * gfx->color_granularity;
	UINT32 mask = 0;
	UINT32 count, bit;

	/* make sure we are in range */
	assert(entry < ctable->entries);
	assert(gfx->color_depth <= 32);

	/* either gfx->color_depth entries or as many as we can get up until the end */
	count = MIN(gfx->color_depth, ctable->entries - entry);

	/* set a bit anywhere the transcolor matches */
	for (bit = 0; bit < count; bit++)
		if (ctable->raw[entry++] == transcolor)
			mask |= 1 << bit;

	/* return the final mask */
	return mask;
}


/*-------------------------------------------------
    colortable_configure_tilemap_groups -
    configure groups in a tilemap to represent
    transparency based on colortable entries
    (each group maps to a gfx color)
-------------------------------------------------*/

void colortable_configure_tilemap_groups(colortable *ctable, tilemap *tmap, const gfx_element *gfx, int transcolor)
{
	int color;

	assert(gfx->total_colors <= TILEMAP_NUM_GROUPS);

	/* iterate over all colors in the tilemap */
	for (color = 0; color < gfx->total_colors; color++)
		tilemap_set_transmask(tmap, color, colortable_get_transpen_mask(ctable, gfx, color, transcolor), 0);
}



/***************************************************************************
    UTILITIES
***************************************************************************/

/*-------------------------------------------------
    get_black_pen - return the pen for a fixed
    black color
-------------------------------------------------*/

pen_t get_black_pen(running_machine *machine)
{
	palette_private *palette = machine->palette_data;
	return palette->black_pen;
}


/*-------------------------------------------------
    get_white_pen - return the pen for a fixed
    white color
-------------------------------------------------*/

pen_t get_white_pen(running_machine *machine)
{
	palette_private *palette = machine->palette_data;
	return palette->white_pen;
}



/***************************************************************************
    INTERNAL FUNCTIONS
***************************************************************************/

/*-------------------------------------------------
    palette_presave - prepare the save arrays
    for saving
-------------------------------------------------*/

static void palette_presave(void)
{
	int numcolors = palette_get_num_colors(Machine->palette);
	palette_private *palette = Machine->palette_data;
	int index;

	/* fill the save arrays with updated pen and brightness information */
	for (index = 0; index < numcolors; index++)
	{
		palette->save_pen[index] = palette_entry_get_color(Machine->palette, index);
		palette->save_bright[index] = palette_entry_get_contrast(Machine->palette, index);
	}
}


/*-------------------------------------------------
    palette_postload - called after restore to
    actually update the palette
-------------------------------------------------*/

static void palette_postload(void)
{
	int numcolors = palette_get_num_colors(Machine->palette);
	palette_private *palette = Machine->palette_data;
	int index;

	/* reset the pen and brightness for each entry */
	for (index = 0; index < numcolors; index++)
	{
		palette_entry_set_color(Machine->palette, index, palette->save_pen[index]);
		palette_entry_set_contrast(Machine->palette, index, palette->save_bright[index]);
	}
}


/*-------------------------------------------------
    palette_exit - free any allocated memory
-------------------------------------------------*/

static void palette_exit(running_machine *machine)
{
	/* dereference the palette */
	if (machine->palette != NULL)
		palette_deref(machine->palette);
}


/*-------------------------------------------------
    allocate_palette - allocate and configure the
    palette object itself
-------------------------------------------------*/

static void allocate_palette(running_machine *machine, palette_private *palette)
{
	int numgroups, index;

	/* determine the number of groups we need */
	numgroups = 1;
	if (machine->drv->video_attributes & VIDEO_HAS_SHADOWS)
		palette->shadow_group = numgroups++;
	if (machine->drv->video_attributes & VIDEO_HAS_HIGHLIGHTS)
		palette->hilight_group = numgroups++;
	assert_always(machine->drv->total_colors * numgroups <= 65536, "Error: palette has more than 65536 colors.");

	/* allocate a palette object containing all the colors and groups */
	machine->palette = palette_alloc(machine->drv->total_colors, numgroups);
	assert_always(machine->palette != NULL, "Failed to allocate system palette");

	/* configure the groups */
	if (palette->shadow_group != 0)
		palette_group_set_contrast(machine->palette, palette->shadow_group, PALETTE_DEFAULT_SHADOW_FACTOR);
	if (palette->hilight_group != 0)
		palette_group_set_contrast(machine->palette, palette->hilight_group, PALETTE_DEFAULT_HIGHLIGHT_FACTOR);

	/* set the initial colors to a standard rainbow */
	for (index = 0; index < machine->drv->total_colors; index++)
		palette_entry_set_color(machine->palette, index, MAKE_RGB(pal1bit(index >> 0), pal1bit(index >> 1), pal1bit(index >> 2)));

	/* switch off the color mode */
	switch (palette->format)
	{
		/* 16-bit paletteized case */
		case BITMAP_FORMAT_INDEXED16:
			palette->black_pen = palette_get_black_entry(machine->palette);
			palette->white_pen = palette_get_white_entry(machine->palette);
			if (palette->black_pen >= 65536)
				palette->black_pen = 0;
			if (palette->white_pen >= 65536)
				palette->white_pen = 65536;
			break;

		/* 15-bit direct case */
		case BITMAP_FORMAT_RGB15:
			palette->black_pen = rgb_to_rgb15(MAKE_RGB(0x00,0x00,0x00));
			palette->white_pen = rgb_to_rgb15(MAKE_RGB(0xff,0xff,0xff));
			break;

		/* 32-bit direct case */
		case BITMAP_FORMAT_RGB32:
			palette->black_pen = MAKE_RGB(0x00,0x00,0x00);
			palette->white_pen = MAKE_RGB(0xff,0xff,0xff);
			break;

		/* screenless case */
		case BITMAP_FORMAT_INVALID:
		default:
			break;
	}
}


/*-------------------------------------------------
    allocate_color_tables - allocate memory for
    pen and color tables
-------------------------------------------------*/

static void allocate_color_tables(running_machine *machine, palette_private *palette)
{
	int total_colors = palette_get_num_colors(machine->palette) * palette_get_num_groups(machine->palette);
	pen_t *pentable;
	int i;

	/* allocate memory for the pen table */
	switch (palette->format)
	{
		case BITMAP_FORMAT_INDEXED16:
			/* create a dummy 1:1 mapping */
			machine->pens = pentable = auto_malloc((total_colors + 2) * sizeof(machine->pens[0]));
			for (i = 0; i < total_colors + 2; i++)
				pentable[i] = i;
			break;

		case BITMAP_FORMAT_RGB15:
			machine->pens = palette_entry_list_adjusted_rgb15(machine->palette);
			break;

		case BITMAP_FORMAT_RGB32:
			machine->pens = palette_entry_list_adjusted(machine->palette);
			break;

		default:
			machine->pens = NULL;
			break;
	}
}


/*-------------------------------------------------
    allocate_shadow_tables - allocate memory for
    shadow tables
-------------------------------------------------*/

static void allocate_shadow_tables(running_machine *machine, palette_private *palette)
{
	/* if we have shadows, allocate shadow tables */
	if (machine->drv->video_attributes & VIDEO_HAS_SHADOWS)
	{
		pen_t *table = auto_malloc(65536 * sizeof(*table));
		int i;

		/* palettized mode gets a single 64k table in slots 0 and 2 */
		if (palette->format == BITMAP_FORMAT_INDEXED16)
		{
			palette->shadow_table[0].base = palette->shadow_table[2].base = table;
			for (i = 0; i < 65536; i++)
				table[i] = (i < machine->drv->total_colors) ? (i + machine->drv->total_colors) : i;
		}

		/* RGB mode gets two 32k tables in slots 0 and 2 */
		else
		{
			palette->shadow_table[0].base = table;
			palette->shadow_table[2].base = table + 32768;
			configure_rgb_shadows(machine, 0, PALETTE_DEFAULT_SHADOW_FACTOR);
		}
	}

	/* if we have hilights, allocate shadow tables */
	if (machine->drv->video_attributes & VIDEO_HAS_HIGHLIGHTS)
	{
		pen_t *table = auto_malloc(65536 * sizeof(*table));
		int i;

		/* palettized mode gets a single 64k table in slots 1 and 3 */
		if (palette->format == BITMAP_FORMAT_INDEXED16)
		{
			palette->shadow_table[1].base = palette->shadow_table[3].base = table;
			for (i = 0; i < 65536; i++)
				table[i] = (i < machine->drv->total_colors) ? (i + 2 * machine->drv->total_colors) : i;
		}

		/* RGB mode gets two 32k tables in slots 1 and 3 */
		else
		{
			palette->shadow_table[1].base = table;
			palette->shadow_table[3].base = table + 32768;
			configure_rgb_shadows(machine, 1, PALETTE_DEFAULT_HIGHLIGHT_FACTOR);
		}
	}

	/* set the default table */
	machine->shadow_table = palette->shadow_table[0].base;
}


/*-------------------------------------------------
    configure_rgb_shadows - configure shadows
    for the RGB tables
-------------------------------------------------*/

static void configure_rgb_shadows(running_machine *machine, int mode, float factor)
{
	palette_private *palette = machine->palette_data;
	shadow_table_data *stable = &palette->shadow_table[mode];
	int ifactor = (int)(factor * 256.0f);
	int i;

	/* only applies to RGB direct modes */
	assert(palette->format != BITMAP_FORMAT_INDEXED16);
	assert(stable->base != NULL);

	#if VERBOSE
		popmessage("shadow %d recalc %d %d %d %02x", mode, dr, dg, db, noclip);
	#endif

	/* regenerate the table */
	for (i = 0; i < 32768; i++)
	{
		UINT8 r = rgb_clamp((pal5bit(i >> 10) * ifactor) >> 8);
		UINT8 g = rgb_clamp((pal5bit(i >> 5) * ifactor) >> 8);
		UINT8 b = rgb_clamp((pal5bit(i >> 0) * ifactor) >> 8);
		pen_t final = MAKE_RGB(r, g, b);

		/* store either 16 or 32 bit */
		if (palette->format == BITMAP_FORMAT_RGB32)
			stable->base[i] = final;
		else
			stable->base[i] = rgb_to_rgb15(final);
	}
}
