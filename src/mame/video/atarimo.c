/***************************************************************************

    atarimo.c

    Common motion object management functions for Atari raster games.

***************************************************************************/

#include "driver.h"
#include "atarimo.h"


/***************************************************************************
    TYPES & STRUCTURES
***************************************************************************/

/* internal structure containing a word index, shift and mask */
typedef struct _atarimo_mask atarimo_mask;
struct _atarimo_mask
{
	int					word;				/* word index */
	int					shift;				/* shift amount */
	int					mask;				/* final mask */
};

/* internal structure containing the state of the motion objects */
typedef struct _atarimo_data atarimo_data;
struct _atarimo_data
{
	running_machine *	machine;			/* pointer back to the machine */

	UINT32				gfxchanged;			/* true if the gfx info has changed */
	gfx_element			gfxelement[MAX_GFX_ELEMENTS]; /* local copy of graphics elements */
	int					gfxgranularity[MAX_GFX_ELEMENTS];

	bitmap_t			*bitmap;			/* temporary bitmap to render to */

	int					linked;				/* are the entries linked? */
	int					split;				/* are entries split or together? */
	int					reverse;			/* render in reverse order? */
	int					swapxy;				/* render in swapped X/Y order? */
	UINT8				nextneighbor;		/* does the neighbor bit affect the next object? */
	int					slipshift;			/* log2(pixels_per_SLIP) */
	int					slipoffset;			/* pixel offset for SLIP */

	int					entrycount;			/* number of entries per bank */
	int					entrybits;			/* number of bits needed to represent entrycount */
	int					bankcount;			/* number of banks */

	int					tilewidth;			/* width of non-rotated tile */
	int					tileheight;			/* height of non-rotated tile */
	int					tilexshift;			/* bits to shift X coordinate when drawing */
	int					tileyshift;			/* bits to shift Y coordinate when drawing */
	int					bitmapwidth;		/* width of the full playfield bitmap */
	int					bitmapheight;		/* height of the full playfield bitmap */
	int					bitmapxmask;		/* x coordinate mask for the playfield bitmap */
	int					bitmapymask;		/* y coordinate mask for the playfield bitmap */

	int					spriterammask;		/* combined mask when accessing sprite RAM with raw addresses */
	int					spriteramsize;		/* total size of sprite RAM, in entries */
	int					sliprammask;		/* combined mask when accessing SLIP RAM with raw addresses */
	int					slipramsize;		/* total size of SLIP RAM, in entries */

	UINT32				palettebase;		/* base palette entry */
	int					maxcolors;			/* maximum number of colors */
	int					transpen;			/* transparent pen index */

	UINT32				bank;				/* current bank number */
	UINT32				xscroll;			/* current x scroll offset */
	UINT32				yscroll;			/* current y scroll offset */

	int					maxperline;			/* maximum number of entries/line */

	atarimo_mask		linkmask;			/* mask for the link */
	atarimo_mask		gfxmask;			/* mask for the graphics bank */
	atarimo_mask		codemask;			/* mask for the code index */
	atarimo_mask		codehighmask;		/* mask for the upper code index */
	atarimo_mask		colormask;			/* mask for the color */
	atarimo_mask		xposmask;			/* mask for the X position */
	atarimo_mask		yposmask;			/* mask for the Y position */
	atarimo_mask		widthmask;			/* mask for the width, in tiles*/
	atarimo_mask		heightmask;			/* mask for the height, in tiles */
	atarimo_mask		hflipmask;			/* mask for the horizontal flip */
	atarimo_mask		vflipmask;			/* mask for the vertical flip */
	atarimo_mask		prioritymask;		/* mask for the priority */
	atarimo_mask		neighbormask;		/* mask for the neighbor */
	atarimo_mask		absolutemask;		/* mask for absolute coordinates */

	atarimo_mask		specialmask;		/* mask for the special value */
	int					specialvalue;		/* resulting value to indicate "special" */
	atarimo_special_func specialcb;			/* callback routine for special entries */
	int					codehighshift;		/* shift count for the upper code */

	atarimo_entry *		spriteram;			/* pointer to sprite RAM */
	UINT16 **			slipram;			/* pointer to the SLIP RAM pointer */
	UINT16 *			codelookup;			/* lookup table for codes */
	UINT8 *				colorlookup;		/* lookup table for colors */
	UINT8 *				gfxlookup;			/* lookup table for graphics */

	atarimo_entry *		activelist[ATARIMO_MAXPERBANK];	/* pointers to active motion objects */
	atarimo_entry **	activelast;			/* pointer to the last pointer in the active list */

	UINT8 *				dirtygrid;			/* grid of dirty rects for blending */
	int					dirtywidth;			/* width of dirty grid */
	int					dirtyheight;		/* height of dirty grid */

	rectangle			rectlist[ATARIMO_MAXPERBANK];	/* list of bounding rectangles */
	int					rectcount;

	UINT32				last_xpos;			/* (during processing) the previous X position */
	UINT32				next_xpos;			/* (during processing) the next X position */
};



/***************************************************************************
    MACROS
***************************************************************************/

/* data extraction */
#define EXTRACT_DATA(_input, _mask) (((_input)->data[(_mask).word] >> (_mask).shift) & (_mask).mask)



/***************************************************************************
    GLOBAL VARIABLES
***************************************************************************/

UINT16 *atarimo_0_spriteram;
UINT16 *atarimo_0_slipram;

UINT16 *atarimo_1_spriteram;
UINT16 *atarimo_1_slipram;



/***************************************************************************
    STATIC VARIABLES
***************************************************************************/

static atarimo_data atarimo[ATARIMO_MAX];
static emu_timer *force_update_timer;


/***************************************************************************
    STATIC FUNCTION DECLARATIONS
***************************************************************************/

static int mo_render_object(atarimo_data *mo, const atarimo_entry *entry, const rectangle *cliprect);



/***************************************************************************
    INLINE FUNCTIONS
***************************************************************************/

/*---------------------------------------------------------------
    compute_log: Computes the number of bits necessary to
    hold a given value. The input must be an even power of
    two.
---------------------------------------------------------------*/

INLINE int compute_log(int value)
{
	int log = 0;

	if (value == 0)
		return -1;
	while (!(value & 1))
		log++, value >>= 1;
	if (value != 1)
		return -1;
	return log;
}


/*---------------------------------------------------------------
    round_to_powerof2: Rounds a number up to the nearest
    power of 2. Even powers of 2 are rounded up to the
    next greatest power (e.g., 4 returns 8).
---------------------------------------------------------------*/

INLINE int round_to_powerof2(int value)
{
	int log = 0;

	if (value == 0)
		return 1;
	while ((value >>= 1) != 0)
		log++;
	return 1 << (log + 1);
}


/*---------------------------------------------------------------
    convert_mask: Converts a 4-word mask into a word index,
    shift, and adjusted mask. Returns 0 if invalid.
---------------------------------------------------------------*/

INLINE int convert_mask(const atarimo_entry *input, atarimo_mask *result)
{
	int i, temp;

	/* determine the word and make sure it's only 1 */
	result->word = -1;
	for (i = 0; i < 4; i++)
		if (input->data[i])
		{
			if (result->word == -1)
				result->word = i;
			else
				return 0;
		}

	/* if all-zero, it's valid */
	if (result->word == -1)
	{
		result->word = result->shift = result->mask = 0;
		return 1;
	}

	/* determine the shift and final mask */
	result->shift = 0;
	temp = input->data[result->word];
	while (!(temp & 1))
	{
		result->shift++;
		temp >>= 1;
	}
	result->mask = temp;
	return 1;
}


/*---------------------------------------------------------------
    init_gfxelement: Make a copy of the main gfxelement that
    gives us full control over colors.
---------------------------------------------------------------*/

INLINE void init_gfxelement(atarimo_data *mo, int idx)
{
	mo->gfxelement[idx] = *mo->machine->gfx[idx];
	mo->gfxgranularity[idx] = mo->gfxelement[idx].color_granularity;
	mo->gfxelement[idx].color_granularity = 1;
	mo->gfxelement[idx].color_base = 0;
	mo->gfxelement[idx].total_colors = 65536;
}


/*---------------------------------------------------------------
    init_savestate: Initialize save states
---------------------------------------------------------------*/

static void init_savestate(running_machine *machine, int index, atarimo_data *mo)
{
	state_save_register_item(machine, "atarimo", NULL, index, mo->gfxchanged);
	state_save_register_item(machine, "atarimo", NULL, index, mo->palettebase);
	state_save_register_item(machine, "atarimo", NULL, index, mo->bank);
	state_save_register_item(machine, "atarimo", NULL, index, mo->xscroll);
	state_save_register_item(machine, "atarimo", NULL, index, mo->yscroll);
	state_save_register_item(machine, "atarimo", NULL, index, mo->last_xpos);
	state_save_register_item(machine, "atarimo", NULL, index, mo->next_xpos);

#if 0
	// These are not modified in code
	// Left in for completeness
	state_save_register_item(machine, "atarimo", NULL, index, mo->reverse);
	state_save_register_item(machine, "atarimo", NULL, index, mo->split);
	state_save_register_item(machine, "atarimo", NULL, index, mo->linked);
	state_save_register_item(machine, "atarimo", NULL, index, mo->swapxy);
	state_save_register_item(machine, "atarimo", NULL, index, mo->nextneighbor);
	state_save_register_item(machine, "atarimo", NULL, index, mo->slipshift);
	state_save_register_item(machine, "atarimo", NULL, index, mo->slipoffset);
	state_save_register_item(machine, "atarimo", NULL, index, mo->slipramsize);
	state_save_register_item(machine, "atarimo", NULL, index, mo->sliprammask);
	state_save_register_item(machine, "atarimo", NULL, index, mo->entrycount);
	state_save_register_item(machine, "atarimo", NULL, index, mo->entrybits);
	state_save_register_item(machine, "atarimo", NULL, index, mo->bankcount);
	state_save_register_item(machine, "atarimo", NULL, index, mo->tilewidth);
	state_save_register_item(machine, "atarimo", NULL, index, mo->tileheight);
	state_save_register_item(machine, "atarimo", NULL, index, mo->tilexshift);
	state_save_register_item(machine, "atarimo", NULL, index, mo->tileyshift);
	state_save_register_item(machine, "atarimo", NULL, index, mo->bitmapwidth);
	state_save_register_item(machine, "atarimo", NULL, index, mo->bitmapheight);
	state_save_register_item(machine, "atarimo", NULL, index, mo->bitmapxmask);
	state_save_register_item(machine, "atarimo", NULL, index, mo->bitmapymask);
	state_save_register_item(machine, "atarimo", NULL, index, mo->spriteramsize);
	state_save_register_item(machine, "atarimo", NULL, index, mo->spriterammask);
	state_save_register_item(machine, "atarimo", NULL, index, mo->maxcolors);
	state_save_register_item(machine, "atarimo", NULL, index, mo->transpen);
	state_save_register_item(machine, "atarimo", NULL, index, mo->maxperline);
	state_save_register_item(machine, "atarimo", NULL, index, mo->specialvalue);
	state_save_register_item(machine, "atarimo", NULL, index, mo->codehighshift);
	state_save_register_item(machine, "atarimo", NULL, index, mo->dirtywidth);
	state_save_register_item(machine, "atarimo", NULL, index, mo->dirtyheight);
#endif

	state_save_register_bitmap(machine, "atarimo", NULL, index, "bitmap", mo->bitmap, __FILE__, __LINE__);

	state_save_register_memory(machine, "atarimo", NULL, index, "spriteram", mo->spriteram, sizeof(atarimo_entry), mo->spriteramsize, __FILE__, __LINE__);

	state_save_register_item_pointer(machine, "atarimo", NULL, index, mo->codelookup, round_to_powerof2(mo->codemask.mask));

	state_save_register_item_pointer(machine, "atarimo", NULL, index, mo->colorlookup, round_to_powerof2(mo->colormask.mask));

	state_save_register_item_pointer(machine, "atarimo", NULL, index, mo->dirtygrid, mo->dirtywidth * mo->dirtyheight);

	state_save_register_item_pointer(machine, "atarimo", NULL, index, mo->gfxlookup, round_to_powerof2(mo->gfxmask.mask));

}

/***************************************************************************
    GLOBAL FUNCTIONS
***************************************************************************/

static TIMER_CALLBACK( force_update )
{
	int scanline = param;

	if (scanline > 0)
		video_screen_update_partial(machine->primary_screen, scanline - 1);

	scanline += 64;
	if (scanline >= video_screen_get_visible_area(machine->primary_screen)->max_y)
		scanline = 0;
	timer_adjust_oneshot(force_update_timer, video_screen_get_time_until_pos(machine->primary_screen, scanline, 0), scanline);
}

/*---------------------------------------------------------------
    atarimo_init: Configures the motion objects using the input
    description. Allocates all memory necessary and generates
    the attribute lookup table.
---------------------------------------------------------------*/

void atarimo_init(running_machine *machine, int map, const atarimo_desc *desc)
{
	gfx_element *gfx = machine->gfx[desc->gfxindex];
	atarimo_data *mo = &atarimo[map];
	int i;

	assert_always(map >= 0 && map < ATARIMO_MAX, "atarimo_init: map out of range");

	/* determine the masks first */
	convert_mask(&desc->linkmask,      &mo->linkmask);
	convert_mask(&desc->gfxmask,       &mo->gfxmask);
	convert_mask(&desc->codemask,      &mo->codemask);
	convert_mask(&desc->codehighmask,  &mo->codehighmask);
	convert_mask(&desc->colormask,     &mo->colormask);
	convert_mask(&desc->xposmask,      &mo->xposmask);
	convert_mask(&desc->yposmask,      &mo->yposmask);
	convert_mask(&desc->widthmask,     &mo->widthmask);
	convert_mask(&desc->heightmask,    &mo->heightmask);
	convert_mask(&desc->hflipmask,     &mo->hflipmask);
	convert_mask(&desc->vflipmask,     &mo->vflipmask);
	convert_mask(&desc->prioritymask,  &mo->prioritymask);
	convert_mask(&desc->neighbormask,  &mo->neighbormask);
	convert_mask(&desc->absolutemask,  &mo->absolutemask);

	/* copy in the basic data */
	mo->machine       = machine;
	mo->gfxchanged    = FALSE;

	mo->linked        = desc->linked;
	mo->split         = desc->split;
	mo->reverse       = desc->reverse;
	mo->swapxy        = desc->swapxy;
	mo->nextneighbor  = desc->nextneighbor;
	mo->slipshift     = desc->slipheight ? compute_log(desc->slipheight) : 0;
	mo->slipoffset    = desc->slipoffset;

	mo->entrycount    = round_to_powerof2(mo->linkmask.mask);
	mo->entrybits     = compute_log(mo->entrycount);
	mo->bankcount     = desc->banks;

	mo->tilewidth     = gfx->width;
	mo->tileheight    = gfx->height;
	mo->tilexshift    = compute_log(mo->tilewidth);
	mo->tileyshift    = compute_log(mo->tileheight);
	mo->bitmapwidth   = round_to_powerof2(mo->xposmask.mask);
	mo->bitmapheight  = round_to_powerof2(mo->yposmask.mask);
	mo->bitmapxmask   = mo->bitmapwidth - 1;
	mo->bitmapymask   = mo->bitmapheight - 1;

	mo->spriteramsize = mo->bankcount * mo->entrycount;
	mo->spriterammask = mo->spriteramsize - 1;
	mo->slipramsize   = mo->bitmapheight >> mo->slipshift;
	mo->sliprammask   = mo->slipramsize - 1;

	mo->palettebase   = desc->palettebase;
	mo->maxcolors     = desc->maxcolors / gfx->color_granularity;
	mo->transpen      = desc->transpen;

	mo->bank          = 0;
	mo->xscroll       = 0;
	mo->yscroll       = 0;

	mo->maxperline    = desc->maxlinks ? desc->maxlinks : 0x400;

	convert_mask(&desc->specialmask, &mo->specialmask);
	mo->specialvalue  = desc->specialvalue;
	mo->specialcb     = desc->specialcb;
	mo->codehighshift = compute_log(round_to_powerof2(mo->codemask.mask));

	mo->slipram       = (map == 0) ? &atarimo_0_slipram : &atarimo_1_slipram;

	/* allocate the temp bitmap */
	mo->bitmap        = auto_bitmap_alloc(machine, video_screen_get_width(machine->primary_screen), video_screen_get_height(machine->primary_screen), BITMAP_FORMAT_INDEXED16);
	bitmap_fill(mo->bitmap, NULL, desc->transpen);

	/* allocate the spriteram */
	mo->spriteram = auto_alloc_array_clear(machine, atarimo_entry, mo->spriteramsize);

	/* allocate the code lookup */
	mo->codelookup = auto_alloc_array(machine, UINT16, round_to_powerof2(mo->codemask.mask));

	/* initialize it 1:1 */
	for (i = 0; i < round_to_powerof2(mo->codemask.mask); i++)
		mo->codelookup[i] = i;

	/* allocate the color lookup */
	mo->colorlookup = auto_alloc_array(machine, UINT8, round_to_powerof2(mo->colormask.mask));

	/* initialize it 1:1 */
	for (i = 0; i < round_to_powerof2(mo->colormask.mask); i++)
		mo->colorlookup[i] = i;

	/* allocate dirty grid */
	mo->dirtywidth = (video_screen_get_width(machine->primary_screen) >> mo->tilexshift) + 2;
	mo->dirtyheight = (video_screen_get_height(machine->primary_screen) >> mo->tileyshift) + 2;
	mo->dirtygrid = auto_alloc_array(machine, UINT8, mo->dirtywidth * mo->dirtyheight);

	/* allocate the gfx lookup */
	mo->gfxlookup = auto_alloc_array(machine, UINT8, round_to_powerof2(mo->gfxmask.mask));

	/* initialize it with the gfxindex we were passed in */
	for (i = 0; i < round_to_powerof2(mo->gfxmask.mask); i++)
		mo->gfxlookup[i] = desc->gfxindex;

	/* initialize the gfx elements so we have full control over colors */
	init_gfxelement(mo, desc->gfxindex);

	/* start a timer to update a few times during refresh */
	force_update_timer = timer_alloc(machine, force_update, NULL);
	timer_adjust_oneshot(force_update_timer,video_screen_get_time_until_pos(machine->primary_screen, 0, 0), 0);

	init_savestate(machine, map, mo);

	logerror("atarimo_init:\n");
	logerror("  width=%d (shift=%d),  height=%d (shift=%d)\n", mo->tilewidth, mo->tilexshift, mo->tileheight, mo->tileyshift);
	logerror("  spriteram mask=%X, size=%d\n", mo->spriterammask, mo->spriteramsize);
	logerror("  slipram mask=%X, size=%d\n", mo->sliprammask, mo->slipramsize);
	logerror("  bitmap size=%dx%d\n", mo->bitmapwidth, mo->bitmapheight);
}


/*---------------------------------------------------------------
    atarimo_get_code_lookup: Returns a pointer to the code
    lookup table.
---------------------------------------------------------------*/

UINT16 *atarimo_get_code_lookup(int map, int *size)
{
	atarimo_data *mo = &atarimo[map];

	if (size)
		*size = round_to_powerof2(mo->codemask.mask);
	return mo->codelookup;
}


/*---------------------------------------------------------------
    atarimo_get_color_lookup: Returns a pointer to the color
    lookup table.
---------------------------------------------------------------*/

UINT8 *atarimo_get_color_lookup(int map, int *size)
{
	atarimo_data *mo = &atarimo[map];

	if (size)
		*size = round_to_powerof2(mo->colormask.mask);
	return mo->colorlookup;
}


/*---------------------------------------------------------------
    atarimo_get_gfx_lookup: Returns a pointer to the graphics
    lookup table.
---------------------------------------------------------------*/

UINT8 *atarimo_get_gfx_lookup(int map, int *size)
{
	atarimo_data *mo = &atarimo[map];

	mo->gfxchanged = TRUE;
	if (size)
		*size = round_to_powerof2(mo->gfxmask.mask);
	return mo->gfxlookup;
}


/*---------------------------------------------------------------
    build_active_list: Build a list of active objects.
---------------------------------------------------------------*/

static void build_active_list(atarimo_data *mo, int link)
{
	atarimo_entry *bankbase = &mo->spriteram[mo->bank << mo->entrybits];
	UINT8 movisit[ATARIMO_MAXPERBANK];
	atarimo_entry **current;
	int i;

	/* reset the visit map */
	memset(movisit, 0, mo->entrycount);

	/* visit all the motion objects and copy their data into the display list */
	for (i = 0, current = mo->activelist; i < mo->maxperline && !movisit[link]; i++)
	{
		atarimo_entry *modata = &bankbase[link];

		/* copy the current entry into the list */
		*current++ = modata;

		/* link to the next object */
		movisit[link] = 1;
		if (mo->linked)
			link = EXTRACT_DATA(modata, mo->linkmask);
		else
			link = (link + 1) & mo->linkmask.mask;
	}

	/* note the last entry */
	mo->activelast = current;
}


/*---------------------------------------------------------------
    get_dirty_base: Return the dirty grid pointer for a given
    X and Y position.
---------------------------------------------------------------*/

INLINE UINT8 *get_dirty_base(atarimo_data *mo, int x, int y)
{
	UINT8 *result = mo->dirtygrid;
	result += ((y >> mo->tileyshift) + 1) * mo->dirtywidth;
	result += (x >> mo->tilexshift) + 1;
	return result;
}


/*---------------------------------------------------------------
    erase_dirty_grid: Erases the dirty grid within a given
    cliprect.
---------------------------------------------------------------*/

static void erase_dirty_grid(atarimo_data *mo, const rectangle *cliprect)
{
	int sx = cliprect->min_x >> mo->tilexshift;
	int ex = cliprect->max_x >> mo->tilexshift;
	int sy = cliprect->min_y >> mo->tileyshift;
	int ey = cliprect->max_y >> mo->tileyshift;
	int y;

	/* loop over all grid rows that intersect our cliprect */
	for (y = sy; y <= ey; y++)
	{
		/* get the base pointer and memset the row */
		UINT8 *dirtybase = get_dirty_base(mo, cliprect->min_x, y << mo->tileyshift);
		memset(dirtybase, 0, ex - sx + 1);
	}
}


/*---------------------------------------------------------------
    convert_dirty_grid_to_rects: Converts a dirty grid into a
    series of cliprects.
---------------------------------------------------------------*/

static void convert_dirty_grid_to_rects(atarimo_data *mo, const rectangle *cliprect, atarimo_rect_list *rectlist)
{
	int sx = cliprect->min_x >> mo->tilexshift;
	int ex = cliprect->max_x >> mo->tilexshift;
	int sy = cliprect->min_y >> mo->tileyshift;
	int ey = cliprect->max_y >> mo->tileyshift;
	int tilewidth = 1 << mo->tilexshift;
	int tileheight = 1 << mo->tileyshift;
	rectangle *rect;
	int x, y;

	/* initialize the rect list */
	rectlist->numrects = 0;
	rectlist->rect = mo->rectlist;
	rect = &mo->rectlist[-1];

	/* loop over all grid rows that intersect our cliprect */
	for (y = sy; y <= ey; y++)
	{
		UINT8 *dirtybase = get_dirty_base(mo, cliprect->min_x, y << mo->tileyshift);
		int can_add_to_existing = 0;

		/* loop over all grid columns that intersect our cliprect */
		for (x = sx; x <= ex; x++)
		{
			/* if this tile is dirty, add that to our rectlist */
			if (*dirtybase++)
			{
				/* if we can't add to an existing rect, create a new one */
				if (!can_add_to_existing)
				{
					/* advance pointers */
					rectlist->numrects++;
					rect++;

					/* make a rect describing this grid square */
					rect->min_x = x << mo->tilexshift;
					rect->max_x = rect->min_x + tilewidth - 1;
					rect->min_y = y << mo->tileyshift;
					rect->max_y = rect->min_y + tileheight - 1;

					/* neighboring grid squares can add to this one */
					can_add_to_existing = 1;
				}

				/* if we can add to the previous rect, just expand its width */
				else
					rect->max_x += tilewidth;
			}

			/* once we hit a non-dirty square, we can no longer add on */
			else
				can_add_to_existing = 0;
		}
	}
}


/*---------------------------------------------------------------
    atarimo_render: Render the motion objects to the
    destination bitmap.
---------------------------------------------------------------*/

bitmap_t *atarimo_render(int map, const rectangle *cliprect, atarimo_rect_list *rectlist)
{
	atarimo_data *mo = &atarimo[map];
	int startband, stopband, band, i;
	rectangle *rect;

	/* if the graphics info has changed, recompute */
	if (mo->gfxchanged)
	{
		mo->gfxchanged = FALSE;
		for (i = 0; i < round_to_powerof2(mo->gfxmask.mask); i++)
			init_gfxelement(mo, mo->gfxlookup[i]);
	}

	/* compute start/stop bands */
	startband = ((cliprect->min_y + mo->yscroll - mo->slipoffset) & mo->bitmapymask) >> mo->slipshift;
	stopband = ((cliprect->max_y + mo->yscroll - mo->slipoffset) & mo->bitmapymask) >> mo->slipshift;
	if (startband > stopband)
		startband -= mo->bitmapheight >> mo->slipshift;
	if (!mo->slipshift)
		stopband = startband;

	/* erase the dirty grid */
	erase_dirty_grid(mo, cliprect);

	/* loop over SLIP bands */
	for (band = startband; band <= stopband; band++)
	{
		atarimo_entry **first, **current, **last;
		rectangle bandclip;
		int link, step;

		/* if we don't use SLIPs, just recapture from 0 */
		if (!mo->slipshift)
		{
			link = 0;
			bandclip = *cliprect;
		}

		/* otherwise, grab the SLIP and compute the bandrect */
		else
		{
			int slipentry = band & mo->sliprammask;
			link = ((*mo->slipram)[slipentry] >> mo->linkmask.shift) & mo->linkmask.mask;

			/* start with the cliprect */
			bandclip = *cliprect;

			/* compute minimum Y and wrap around if necessary */
			bandclip.min_y = ((band << mo->slipshift) - mo->yscroll + mo->slipoffset) & mo->bitmapymask;
			if (bandclip.min_y > video_screen_get_visible_area(mo->machine->primary_screen)->max_y)
				bandclip.min_y -= mo->bitmapheight;

			/* maximum Y is based on the minimum */
			bandclip.max_y = bandclip.min_y + (1 << mo->slipshift) - 1;

			/* keep within the cliprect */
			sect_rect(&bandclip, cliprect);
		}

		/* if this matches the last link, we don't need to re-process the list */
		build_active_list(mo, link);

		/* set the start and end points */
		if (mo->reverse)
		{
			first = mo->activelast - 1;
			last = mo->activelist - 1;
			step = -1;
		}
		else
		{
			first = mo->activelist;
			last = mo->activelast;
			step = 1;
		}

		/* initialize the parameters */
		mo->next_xpos = 123456;

		/* render the mos */
		for (current = first; current != last; current += step)
			mo_render_object(mo, *current, &bandclip);
	}

	/* convert the dirty grid to a rectlist */
	convert_dirty_grid_to_rects(mo, cliprect, rectlist);

	/* clip the rectlist */
	for (i = 0, rect = rectlist->rect; i < rectlist->numrects; i++, rect++)
		sect_rect(rect, cliprect);

	/* return the bitmap */
	return mo->bitmap;
}


/*---------------------------------------------------------------
    mo_render_object: Internal processing callback that
    renders to the backing bitmap and then copies the result
    to the destination.
---------------------------------------------------------------*/

static int mo_render_object(atarimo_data *mo, const atarimo_entry *entry, const rectangle *cliprect)
{
	int gfxindex = mo->gfxlookup[EXTRACT_DATA(entry, mo->gfxmask)];
	const gfx_element *gfx = &mo->gfxelement[gfxindex];
	bitmap_t *bitmap = mo->bitmap;
	int x, y, sx, sy;

	/* extract data from the various words */
	int code = mo->codelookup[EXTRACT_DATA(entry, mo->codemask)] | (EXTRACT_DATA(entry, mo->codehighmask) << mo->codehighshift);
	int color = mo->colorlookup[EXTRACT_DATA(entry, mo->colormask)];
	int xpos = EXTRACT_DATA(entry, mo->xposmask);
	int ypos = -EXTRACT_DATA(entry, mo->yposmask);
	int hflip = EXTRACT_DATA(entry, mo->hflipmask);
	int vflip = EXTRACT_DATA(entry, mo->vflipmask);
	int width = EXTRACT_DATA(entry, mo->widthmask) + 1;
	int height = EXTRACT_DATA(entry, mo->heightmask) + 1;
	int priority = EXTRACT_DATA(entry, mo->prioritymask);
	int xadv, yadv, rendered = 0;
	UINT8 *dirtybase;

#ifdef TEMPDEBUG
int temp = EXTRACT_DATA(entry, mo->codemask);
if ((temp & 0xff00) == 0xc800)
{
	static UINT8 hits[256];
	if (!hits[temp & 0xff])
	{
		fprintf(stderr, "code = %04X\n", temp);
		hits[temp & 0xff] = 1;
	}
}
#endif

	/* compute the effective color, merging in priority */
	color = (color * mo->gfxgranularity[gfxindex]) | (priority << ATARIMO_PRIORITY_SHIFT);
	color += mo->palettebase;

	/* add in the scroll positions if we're not in absolute coordinates */
	if (!EXTRACT_DATA(entry, mo->absolutemask))
	{
		xpos -= mo->xscroll;
		ypos -= mo->yscroll;
	}

	/* adjust for height */
	ypos -= height << mo->tileyshift;

	/* handle previous hold bits */
	if (mo->next_xpos != 123456)
		xpos = mo->next_xpos;
	mo->next_xpos = 123456;

	/* check for the hold bit */
	if (EXTRACT_DATA(entry, mo->neighbormask))
	{
		if (!mo->nextneighbor)
			xpos = mo->last_xpos + mo->tilewidth;
		else
			mo->next_xpos = xpos + mo->tilewidth;
	}
	mo->last_xpos = xpos;

	/* adjust the final coordinates */
	xpos &= mo->bitmapxmask;
	ypos &= mo->bitmapymask;
	if (xpos > video_screen_get_visible_area(mo->machine->primary_screen)->max_x) xpos -= mo->bitmapwidth;
	if (ypos > video_screen_get_visible_area(mo->machine->primary_screen)->max_y) ypos -= mo->bitmapheight;

	/* is this one special? */
	if (mo->specialmask.mask != 0 && EXTRACT_DATA(entry, mo->specialmask) == mo->specialvalue)
	{
		if (mo->specialcb)
			return (*mo->specialcb)(bitmap, cliprect, code, color, xpos, ypos, NULL);
		return 0;
	}

	/* adjust for h flip */
	xadv = mo->tilewidth;
	if (hflip)
	{
		xpos += (width - 1) << mo->tilexshift;
		xadv = -xadv;
	}

	/* adjust for v flip */
	yadv = mo->tileheight;
	if (vflip)
	{
		ypos += (height - 1) << mo->tileyshift;
		yadv = -yadv;
	}

	/* standard order is: loop over Y first, then X */
	if (!mo->swapxy)
	{
		/* loop over the height */
		for (y = 0, sy = ypos; y < height; y++, sy += yadv)
		{
			/* clip the Y coordinate */
			if (sy <= cliprect->min_y - mo->tileheight)
			{
				code += width;
				continue;
			}
			else if (sy > cliprect->max_y)
				break;

			/* loop over the width */
			for (x = 0, sx = xpos; x < width; x++, sx += xadv, code++)
			{
				/* clip the X coordinate */
				if (sx <= -cliprect->min_x - mo->tilewidth || sx > cliprect->max_x)
					continue;

				/* draw the sprite */
				drawgfx_transpen_raw(bitmap, cliprect, gfx, code, color, hflip, vflip, sx, sy, mo->transpen);
				rendered = 1;

				/* mark the grid dirty */
				dirtybase = get_dirty_base(mo, sx, sy);
				dirtybase[0] = 1;
				dirtybase[1] = 1;
				dirtybase[mo->dirtywidth] = 1;
				dirtybase[mo->dirtywidth + 1] = 1;
			}
		}
	}

	/* alternative order is swapped */
	else
	{
		/* loop over the width */
		for (x = 0, sx = xpos; x < width; x++, sx += xadv)
		{
			/* clip the X coordinate */
			if (sx <= cliprect->min_x - mo->tilewidth)
			{
				code += height;
				continue;
			}
			else if (sx > cliprect->max_x)
				break;

			/* loop over the height */
			dirtybase = get_dirty_base(mo, sx, ypos);
			for (y = 0, sy = ypos; y < height; y++, sy += yadv, code++)
			{
				/* clip the X coordinate */
				if (sy <= -cliprect->min_y - mo->tileheight || sy > cliprect->max_y)
					continue;

				/* draw the sprite */
				drawgfx_transpen_raw(bitmap, cliprect, gfx, code, color, hflip, vflip, sx, sy, mo->transpen);
				rendered = 1;

				/* mark the grid dirty */
				dirtybase = get_dirty_base(mo, sx, sy);
				dirtybase[0] = 1;
				dirtybase[1] = 1;
				dirtybase[mo->dirtywidth] = 1;
				dirtybase[mo->dirtywidth + 1] = 1;
			}
		}
	}

	return rendered;
}


/*---------------------------------------------------------------
    atarimo_set_bank: Set the banking value for
    the motion objects.
---------------------------------------------------------------*/

void atarimo_set_bank(int map, int bank)
{
	atarimo_data *mo = &atarimo[map];
	mo->bank = bank;
}


/*---------------------------------------------------------------
    atarimo_set_xscroll: Set the horizontal scroll value for
    the motion objects.
---------------------------------------------------------------*/

void atarimo_set_xscroll(int map, int xscroll)
{
	atarimo_data *mo = &atarimo[map];
	mo->xscroll = xscroll;
}


/*---------------------------------------------------------------
    atarimo_set_yscroll: Set the vertical scroll value for
    the motion objects.
---------------------------------------------------------------*/

void atarimo_set_yscroll(int map, int yscroll)
{
	atarimo_data *mo = &atarimo[map];
	mo->yscroll = yscroll;
}


/*---------------------------------------------------------------
    atarimo_get_bank: Returns the banking value
    for the motion objects.
---------------------------------------------------------------*/

int atarimo_get_bank(int map)
{
	return atarimo[map].bank;
}


/*---------------------------------------------------------------
    atarimo_get_xscroll: Returns the horizontal scroll value
    for the motion objects.
---------------------------------------------------------------*/

int atarimo_get_xscroll(int map)
{
	return atarimo[map].xscroll;
}


/*---------------------------------------------------------------
    atarimo_get_yscroll: Returns the vertical scroll value for
    the motion objects.
---------------------------------------------------------------*/

int atarimo_get_yscroll(int map)
{
	return atarimo[map].yscroll;
}


/*---------------------------------------------------------------
    atarimo_0_spriteram_w: Write handler for the spriteram.
---------------------------------------------------------------*/

WRITE16_HANDLER( atarimo_0_spriteram_w )
{
	int entry, idx, bank;

	COMBINE_DATA(&atarimo_0_spriteram[offset]);
	if (atarimo[0].split)
	{
		entry = offset & atarimo[0].linkmask.mask;
		idx = (offset >> atarimo[0].entrybits) & 3;
	}
	else
	{
		entry = (offset >> 2) & atarimo[0].linkmask.mask;
		idx = offset & 3;
	}
	bank = offset >> (2 + atarimo[0].entrybits);
	COMBINE_DATA(&atarimo[0].spriteram[(bank << atarimo[0].entrybits) + entry].data[idx]);
}


/*---------------------------------------------------------------
    atarimo_1_spriteram_w: Write handler for the spriteram.
---------------------------------------------------------------*/

WRITE16_HANDLER( atarimo_1_spriteram_w )
{
	int entry, idx, bank;

	COMBINE_DATA(&atarimo_1_spriteram[offset]);
	if (atarimo[1].split)
	{
		entry = offset & atarimo[1].linkmask.mask;
		idx = (offset >> atarimo[1].entrybits) & 3;
	}
	else
	{
		entry = (offset >> 2) & atarimo[1].linkmask.mask;
		idx = offset & 3;
	}
	bank = offset >> (2 + atarimo[1].entrybits);
	COMBINE_DATA(&atarimo[1].spriteram[(bank << atarimo[1].entrybits) + entry].data[idx]);
}


/*---------------------------------------------------------------
    atarimo_0_spriteram_expanded_w: Write handler for the
    expanded form of spriteram.
---------------------------------------------------------------*/

WRITE16_HANDLER( atarimo_0_spriteram_expanded_w )
{
	int entry, idx, bank;

	COMBINE_DATA(&atarimo_0_spriteram[offset]);
	if (!(offset & 1))
	{
		offset >>= 1;
		if (atarimo[0].split)
		{
			entry = offset & atarimo[0].linkmask.mask;
			idx = (offset >> atarimo[0].entrybits) & 3;
		}
		else
		{
			entry = (offset >> 2) & atarimo[0].linkmask.mask;
			idx = offset & 3;
		}
		bank = offset >> (2 + atarimo[0].entrybits);
		COMBINE_DATA(&atarimo[0].spriteram[(bank << atarimo[0].entrybits) + entry].data[idx]);
	}
}


/*---------------------------------------------------------------
    atarimo_0_slipram_w: Write handler for the slipram.
---------------------------------------------------------------*/

WRITE16_HANDLER( atarimo_0_slipram_w )
{
	COMBINE_DATA(&atarimo_0_slipram[offset]);
}


/*---------------------------------------------------------------
    atarimo_1_slipram_w: Write handler for the slipram.
---------------------------------------------------------------*/

WRITE16_HANDLER( atarimo_1_slipram_w )
{
	COMBINE_DATA(&atarimo_1_slipram[offset]);
}
