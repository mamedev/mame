/******************************************************************************

    palette.c

    Palette handling functions.

    Copyright (c) 1996-2007, Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

******************************************************************************/

#include "palette.h"



/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

/* object to track dirty states */
typedef struct _dirty_state dirty_state;
struct _dirty_state
{
	UINT32 *		dirty;						/* bitmap of dirty entries */
	UINT32			mindirty;					/* minimum dirty entry */
	UINT32			maxdirty;					/* minimum dirty entry */
};


/* a single palette client */
struct _palette_client
{
	palette_client *next;						/* pointer to next client */
	palette_t *		palette;					/* reference to the palette */
	dirty_state		live;						/* live dirty state */
	dirty_state		previous;					/* previous dirty state */
};


/* a palette object */
struct _palette_t
{
	UINT32			refcount;					/* reference count on the palette */
	UINT32			numcolors;					/* number of colors in the palette */
	UINT32			numgroups;					/* number of groups in the palette */

	rgb_t *			entry_color;				/* array of raw colors */
	float *			entry_contrast;				/* contrast value for each entry */
	rgb_t *			adjusted_color;				/* array of adjusted colors */
	rgb_t *			adjusted_rgb15;				/* array of adjusted colors as RGB15 */

	float *			group_bright;				/* brightness value for each group */
	float *			group_contrast;				/* contrast value for each group */

	palette_client *client_list;				/* list of clients for this palette */
};



/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

static void internal_palette_free(palette_t *palette);
static void update_adjusted_color(palette_t *palette, UINT32 group, UINT32 index);



/***************************************************************************
    INLINE FUNCTIONS
***************************************************************************/

/*-------------------------------------------------
    adjust_palette_entry - adjust a palette
    entry for brightness
-------------------------------------------------*/

INLINE rgb_t adjust_palette_entry(rgb_t entry, float brightness, float contrast)
{
	int r = rgb_clamp((float)RGB_RED(entry) * contrast + brightness);
	int g = rgb_clamp((float)RGB_GREEN(entry) * contrast + brightness);
	int b = rgb_clamp((float)RGB_BLUE(entry) * contrast + brightness);
	return MAKE_RGB(r,g,b);
}



/***************************************************************************
    PALETTE ALLOCATION
***************************************************************************/

/*-------------------------------------------------
    palette_alloc - allocate a new palette object
    and take a single reference on it
-------------------------------------------------*/

palette_t *palette_alloc(UINT32 numcolors, UINT32 numgroups)
{
	palette_t *palette;
	UINT32 index;

	/* allocate memory */
	palette = malloc(sizeof(*palette));
	if (palette == NULL)
		goto error;
	memset(palette, 0, sizeof(*palette));

	/* allocate an array of palette entries and individual contrasts for each */
	palette->entry_color = malloc(sizeof(*palette->entry_color) * numcolors);
	palette->entry_contrast = malloc(sizeof(*palette->entry_contrast) * numcolors);
	if (palette->entry_color == NULL || palette->entry_contrast == NULL)
		goto error;

	/* initialize the entries */
	for (index = 0; index < numcolors; index++)
	{
		palette->entry_color[index] = RGB_BLACK;
		palette->entry_contrast[index] = 1.0f;
	}

	/* allocate an array of brightness and contrast for each group */
	palette->group_bright = malloc(sizeof(*palette->group_bright) * numgroups);
	palette->group_contrast = malloc(sizeof(*palette->group_contrast) * numgroups);
	if (palette->group_bright == NULL || palette->group_contrast == NULL)
		goto error;

	/* initialize the entries */
	for (index = 0; index < numgroups; index++)
	{
		palette->group_bright[index] = 0.0f;
		palette->group_contrast[index] = 1.0f;
	}

	/* allocate arrays for the adjusted colors */
	palette->adjusted_color = malloc(sizeof(*palette->adjusted_color) * (numcolors * numgroups + 2));
	palette->adjusted_rgb15 = malloc(sizeof(*palette->adjusted_rgb15) * (numcolors * numgroups + 2));
	if (palette->adjusted_color == NULL || palette->adjusted_rgb15 == NULL)
		goto error;

	/* initialize the arrays */
	for (index = 0; index < numcolors * numgroups; index++)
	{
		palette->adjusted_color[index] = RGB_BLACK;
		palette->adjusted_rgb15[index] = rgb_to_rgb15(RGB_BLACK);
	}

	/* add black and white as the last two colors */
	palette->adjusted_color[index] = RGB_BLACK;
	palette->adjusted_rgb15[index++] = rgb_to_rgb15(RGB_BLACK);
	palette->adjusted_color[index] = RGB_WHITE;
	palette->adjusted_rgb15[index++] = rgb_to_rgb15(RGB_WHITE);

	/* initialize the remainder of the structure */
	palette->refcount = 1;
	palette->numcolors = numcolors;
	palette->numgroups = numgroups;
	return palette;

error:
	if (palette != NULL)
		internal_palette_free(palette);

	return NULL;
}


/*-------------------------------------------------
    palette_ref - reference a palette object,
    incrementing its reference count
-------------------------------------------------*/

void palette_ref(palette_t *palette)
{
	palette->refcount++;
}


/*-------------------------------------------------
    palette_deref - dereference a palette object;
    if the reference count goes to 0, it is freed
-------------------------------------------------*/

void palette_deref(palette_t *palette)
{
	/* if the reference count goes to 0, free */
	if (--palette->refcount == 0)
		internal_palette_free(palette);
}



/***************************************************************************
    PALETTE INFORMATION
***************************************************************************/

/*-------------------------------------------------
    palette_get_num_colors - return the number of
    colors allocated in the palette
-------------------------------------------------*/

int palette_get_num_colors(palette_t *palette)
{
	return palette->numcolors;
}


/*-------------------------------------------------
    palette_get_num_groups - return the number of
    groups managed by the palette
-------------------------------------------------*/

int palette_get_num_groups(palette_t *palette)
{
	return palette->numgroups;
}


/*-------------------------------------------------
    palette_get_black_entry - return the index of
    the black entry
-------------------------------------------------*/

UINT32 palette_get_black_entry(palette_t *palette)
{
	return palette->numcolors * palette->numgroups + 0;
}


/*-------------------------------------------------
    palette_get_white_entry - return the index of
    the white entry
-------------------------------------------------*/

UINT32 palette_get_white_entry(palette_t *palette)
{
	return palette->numcolors * palette->numgroups + 1;
}



/***************************************************************************
    PALETTE CLIENTS
***************************************************************************/

/*-------------------------------------------------
    palette_client_alloc - add a new client to a
    palette
-------------------------------------------------*/

palette_client *palette_client_alloc(palette_t *palette)
{
	UINT32 total_colors = palette->numcolors * palette->numgroups;
	UINT32 dirty_dwords = (total_colors + 31) / 32;
	palette_client *client;

	/* allocate memory for the client */
	client = malloc(sizeof(*client));
	if (client == NULL)
		goto error;
	memset(client, 0, sizeof(*client));

	/* allocate dirty lists */
	client->live.dirty = malloc(dirty_dwords * sizeof(UINT32));
	client->previous.dirty = malloc(dirty_dwords * sizeof(UINT32));
	if (client->live.dirty == NULL || client->previous.dirty == NULL)
		goto error;

	/* mark everything dirty to start with */
	memset(client->live.dirty, 0xff, dirty_dwords * sizeof(UINT32));
	memset(client->previous.dirty, 0xff, dirty_dwords * sizeof(UINT32));
	client->live.dirty[dirty_dwords - 1] &= (1 << (total_colors % 32)) - 1;
	client->previous.dirty[dirty_dwords - 1] &= (1 << (total_colors % 32)) - 1;

	/* initialize the rest of the structure and add a reference to a palette */
	client->palette = palette;
	palette_ref(palette);
	client->live.mindirty = 0;
	client->live.maxdirty = total_colors - 1;

	/* now add us to the list of clients */
	client->next = palette->client_list;
	palette->client_list = client;
	return client;

error:
	if (client != NULL)
	{
		if (client->live.dirty != NULL)
			free(client->live.dirty);
		if (client->previous.dirty != NULL)
			free(client->previous.dirty);
		free(client);
	}
	return NULL;
}


/*-------------------------------------------------
    palette_client_free - remove a client from a
    palette
-------------------------------------------------*/

void palette_client_free(palette_client *client)
{
	palette_client **curptr;

	/* first locate and remove ourself from the palette's list */
	for (curptr = &client->palette->client_list; *curptr != NULL; curptr = &(*curptr)->next)
		if (*curptr == client)
		{
			*curptr = client->next;
			break;
		}

	/* now deref the palette */
	palette_deref(client->palette);

	/* free our data */
	if (client->live.dirty != NULL)
		free(client->live.dirty);
	if (client->previous.dirty != NULL)
		free(client->previous.dirty);
	free(client);
}


/*-------------------------------------------------
    palette_client_get_palette - return a pointer
    to the palette for this client
-------------------------------------------------*/

palette_t *palette_client_get_palette(palette_client *client)
{
	return client->palette;
}


/*-------------------------------------------------
    palette_client_get_dirty_list - atomically get
    the current dirty list for a client
-------------------------------------------------*/

const UINT32 *palette_client_get_dirty_list(palette_client *client, UINT32 *mindirty, UINT32 *maxdirty)
{
	dirty_state temp;

	/* fill in the mindirty/maxdirty */
	if (mindirty != NULL)
		*mindirty = client->live.mindirty;
	if (maxdirty != NULL)
		*maxdirty = client->live.maxdirty;

	/* if nothing to report, report nothing and don't swap */
	if (client->live.mindirty > client->live.maxdirty)
		return NULL;

	/* swap the live and previous lists */
	temp = client->live;
	client->live = client->previous;
	client->previous = temp;

	/* erase relevant entries in the new live one */
	if (client->live.mindirty <= client->live.maxdirty)
		memset(client->live.dirty, client->live.mindirty / 8, (client->live.maxdirty / 8) + 1 - (client->live.mindirty / 8));
	client->live.mindirty = client->palette->numcolors * client->palette->numgroups;
	client->live.maxdirty = 0;

	/* return a pointer to the previous table */
	return client->previous.dirty;
}



/***************************************************************************
    PALETTE COLOR MANAGEMENT
***************************************************************************/

/*-------------------------------------------------
    palette_entry_set_color - set the raw RGB
    color for a given palette index
-------------------------------------------------*/

void palette_entry_set_color(palette_t *palette, UINT32 index, rgb_t rgb)
{
	int groupnum;

	/* if out of range, or unchanged, ignore */
	if (index >= palette->numcolors || palette->entry_color[index] == rgb)
		return;

	/* set the color */
	palette->entry_color[index] = rgb;

	/* update across all groups */
	for (groupnum = 0; groupnum < palette->numgroups; groupnum++)
		update_adjusted_color(palette, groupnum, index);
}


/*-------------------------------------------------
    palette_entry_get_color - return the raw RGB
    color for a given palette index
-------------------------------------------------*/

rgb_t palette_entry_get_color(palette_t *palette, UINT32 index)
{
	return (index < palette->numcolors) ? palette->entry_color[index] : RGB_BLACK;
}


/*-------------------------------------------------
    palette_entry_get_adjusted_color - return the
    adjusted RGB color (after all adjustments) for
    a given palette index
-------------------------------------------------*/

rgb_t palette_entry_get_adjusted_color(palette_t *palette, UINT32 index)
{
	return (index < palette->numcolors * palette->numgroups) ? palette->adjusted_color[index] : RGB_BLACK;
}


/*-------------------------------------------------
    palette_entry_list_raw - return the entire
    palette as an array of raw RGB values
-------------------------------------------------*/

const rgb_t *palette_entry_list_raw(palette_t *palette)
{
	return palette->entry_color;
}


/*-------------------------------------------------
    palette_entry_list_adjusted - return the
    entire palette as an array of adjusted RGB
    values
-------------------------------------------------*/

const rgb_t *palette_entry_list_adjusted(palette_t *palette)
{
	return palette->adjusted_color;
}


/*-------------------------------------------------
    palette_entry_list_adjusted_rgb15 - return the
    entire palette as an array of adjusted RGB-15
    values
-------------------------------------------------*/

const rgb_t *palette_entry_list_adjusted_rgb15(palette_t *palette)
{
	return palette->adjusted_rgb15;
}



/***************************************************************************
    PALETTE ADJUSTMENTS
***************************************************************************/

/*-------------------------------------------------
    palette_entry_set_contrast - set the contrast
    adjustment for a single palette index
-------------------------------------------------*/

void palette_entry_set_contrast(palette_t *palette, UINT32 index, float contrast)
{
	int groupnum;

	/* if out of range, or unchanged, ignore */
	if (index >= palette->numcolors || palette->entry_contrast[index] == contrast)
		return;

	/* set the contrast */
	palette->entry_contrast[index] = contrast;

	/* update across all groups */
	for (groupnum = 0; groupnum < palette->numgroups; groupnum++)
		update_adjusted_color(palette, groupnum, index);
}


/*-------------------------------------------------
    palette_entry_get_contrast - return the
    contrast adjustment for a single palette index
-------------------------------------------------*/

float palette_entry_get_contrast(palette_t *palette, UINT32 index)
{
	return (index < palette->numcolors) ? palette->entry_contrast[index] : 1.0f;
}


/*-------------------------------------------------
    palette_group_set_brightness - configure
    overall brightness for a palette group
-------------------------------------------------*/

void palette_group_set_brightness(palette_t *palette, UINT32 group, float brightness)
{
	int index;

	/* if out of range, or unchanged, ignore */
	if (group >= palette->numgroups || palette->group_bright[group] == brightness)
		return;

	/* set the contrast */
	palette->group_bright[group] = brightness;

	/* update across all colors */
	for (index = 0; index < palette->numcolors; index++)
		update_adjusted_color(palette, group, index);
}


/*-------------------------------------------------
    palette_group_set_contrast - configure
    overall contrast for a palette group
-------------------------------------------------*/

void palette_group_set_contrast(palette_t *palette, UINT32 group, float contrast)
{
	int index;

	/* if out of range, or unchanged, ignore */
	if (group >= palette->numgroups || palette->group_contrast[group] == contrast)
		return;

	/* set the contrast */
	palette->group_contrast[group] = contrast;

	/* update across all colors */
	for (index = 0; index < palette->numcolors; index++)
		update_adjusted_color(palette, group, index);
}



/***************************************************************************
    PALETTE UTILITIES
***************************************************************************/

/*-------------------------------------------------
    palette_normalize_range - normalize a range
    of palette entries
-------------------------------------------------*/

void palette_normalize_range(palette_t *palette, UINT32 start, UINT32 end, int lum_min, int lum_max)
{
	UINT32 ymin = 1000 * 255, ymax = 0;
	UINT32 tmin, tmax;
	UINT32 index;

	/* clamp within range */
	start = MAX(start, 0);
	end = MIN(end, palette->numcolors - 1);

	/* find the minimum and maximum brightness of all the colors in the range */
	for (index = start; index <= end; index++)
	{
		rgb_t rgb = palette->entry_color[index];
		UINT32 y = 299 * RGB_RED(rgb) + 587 * RGB_GREEN(rgb) + 114 * RGB_BLUE(rgb);
		ymin = MIN(ymin, y);
		ymax = MAX(ymax, y);
	}

	/* determine target minimum/maximum */
	tmin = (lum_min < 0) ? ((ymin + 500) / 1000) : lum_min;
	tmax = (lum_max < 0) ? ((ymax + 500) / 1000) : lum_max;

	/* now normalize the palette */
	for (index = start; index <= end; index++)
	{
		rgb_t rgb = palette->entry_color[index];
		UINT32 y = 299 * RGB_RED(rgb) + 587 * RGB_GREEN(rgb) + 114 * RGB_BLUE(rgb);
		UINT32 target = tmin + ((y - ymin) * (tmax - tmin + 1)) / (ymax - ymin);
		UINT8 r = (y == 0) ? 0 : rgb_clamp(RGB_RED(rgb) * 1000 * target / y);
		UINT8 g = (y == 0) ? 0 : rgb_clamp(RGB_GREEN(rgb) * 1000 * target / y);
		UINT8 b = (y == 0) ? 0 : rgb_clamp(RGB_BLUE(rgb) * 1000 * target / y);
		palette_entry_set_color(palette, index, MAKE_RGB(r, g, b));
	}
}



/***************************************************************************
    INTERNAL ROUTINES
***************************************************************************/

/*-------------------------------------------------
    internal_palette_free - free all allocations
    from a palette
-------------------------------------------------*/

static void internal_palette_free(palette_t *palette)
{
	/* free per-color data */
	if (palette->entry_color != NULL)
		free(palette->entry_color);
	if (palette->entry_contrast != NULL)
		free(palette->entry_contrast);

	/* free per-group data */
	if (palette->group_bright != NULL)
		free(palette->group_bright);
	if (palette->group_contrast != NULL)
		free(palette->group_contrast);

	/* free adjusted data */
	if (palette->adjusted_color != NULL)
		free(palette->adjusted_color);
	if (palette->adjusted_rgb15 != NULL)
		free(palette->adjusted_rgb15);

	/* and the palette itself */
	free(palette);
}


/*-------------------------------------------------
    update_adjusted_color - update a color index
    by group and index pair
-------------------------------------------------*/

static void update_adjusted_color(palette_t *palette, UINT32 group, UINT32 index)
{
	UINT32 finalindex = group * palette->numcolors + index;
	palette_client *client;
	rgb_t adjusted;

	/* compute the adjusted value */
	adjusted = adjust_palette_entry(palette->entry_color[index], palette->group_bright[group], palette->group_contrast[group] * palette->entry_contrast[index]);

	/* if not different, ignore */
	if (palette->adjusted_color[finalindex] == adjusted)
		return;

	/* otherwise, modify the adjusted color array */
	palette->adjusted_color[finalindex] = adjusted;
	palette->adjusted_rgb15[finalindex] = rgb_to_rgb15(adjusted);

	/* mark dirty in all clients */
	for (client = palette->client_list; client != NULL; client = client->next)
	{
		client->live.dirty[finalindex / 32] |= 1 << (finalindex % 32);
		client->live.mindirty = MIN(client->live.mindirty, finalindex);
		client->live.maxdirty = MAX(client->live.maxdirty, finalindex);
	}
}
