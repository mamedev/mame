/******************************************************************************

    palette.c

    Core palette routines.

    Copyright (c) 1996-2007, Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

***************************************************************************/

#pragma once

#ifndef __PALETTE_H__
#define __PALETTE_H__

#include "osdcore.h"


/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

/* an rgb_t is a single combined R,G,B (and optionally alpha) value */
typedef UINT32 rgb_t;

/* an rgb15_t is a single combined 15-bit R,G,B value */
typedef UINT16 rgb15_t;

/* a palette is an opaque, reference counted object */
typedef struct _palette_t palette_t;

/* a palette client is someone who is tracking the dirty state of a palette */
typedef struct _palette_client palette_client;



/***************************************************************************
    MACROS
***************************************************************************/

/* macros to assemble rgb_t values */
#define MAKE_RGB(r,g,b) 	((((rgb_t)(r) & 0xff) << 16) | (((rgb_t)(g) & 0xff) << 8) | ((rgb_t)(b) & 0xff))
#define MAKE_ARGB(a,r,g,b)	(MAKE_RGB(r,g,b) | (((rgb_t)(a) & 0xff) << 24))

/* macros to extract components from rgb_t values */
#define RGB_ALPHA(rgb)		(((rgb) >> 24) & 0xff)
#define RGB_RED(rgb)		(((rgb) >> 16) & 0xff)
#define RGB_GREEN(rgb)		(((rgb) >> 8) & 0xff)
#define RGB_BLUE(rgb)		((rgb) & 0xff)

/* common colors */
#define RGB_BLACK			(MAKE_RGB(0,0,0))
#define RGB_WHITE			(MAKE_RGB(255,255,255))



/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/


/* ----- palette allocation ----- */

/* allocate a new palette object and take a single reference on it */
palette_t *palette_alloc(UINT32 numcolors, UINT32 numgroups);

/* reference a palette object, incrementing its reference count */
void palette_ref(palette_t *palette);

/* dereference a palette object; if the reference count goes to 0, it is freed */
void palette_deref(palette_t *palette);



/* ----- palette information ----- */

/* return the number of colors managed by the palette */
int palette_get_num_colors(palette_t *palette);

/* return the number of groups managed by the palette */
int palette_get_num_groups(palette_t *palette);

/* return the index of the black entry */
UINT32 palette_get_black_entry(palette_t *palette);

/* return the index of the white entry */
UINT32 palette_get_white_entry(palette_t *palette);



/* ----- palette clients ----- */

/* add a new client to a palette */
palette_client *palette_client_alloc(palette_t *palette);

/* remove a client from a palette */
void palette_client_free(palette_client *client);

/* return a pointer to the palette for this client */
palette_t *palette_client_get_palette(palette_client *client);

/* atomically get the current dirty list for a client */
const UINT32 *palette_client_get_dirty_list(palette_client *client, UINT32 *mindirty, UINT32 *maxdirty);



/* ----- color management ----- */

/* set the raw RGB color for a given palette index */
void palette_entry_set_color(palette_t *palette, UINT32 index, rgb_t rgb);

/* return the raw RGB color for a given palette index */
rgb_t palette_entry_get_color(palette_t *palette, UINT32 index);

/* return the adjusted RGB color (after all adjustments) for a given palette index */
rgb_t palette_entry_get_adjusted_color(palette_t *palette, UINT32 index);

/* return the entire palette as an array of raw RGB values */
const rgb_t *palette_entry_list_raw(palette_t *palette);

/* return the entire palette as an array of adjusted RGB values */
const rgb_t *palette_entry_list_adjusted(palette_t *palette);

/* return the entire palette as an array of adjusted RGB-15 values */
const rgb_t *palette_entry_list_adjusted_rgb15(palette_t *palette);



/* ----- palette adjustments ----- */

/* set the contrast adjustment for a single palette index */
void palette_entry_set_contrast(palette_t *palette, UINT32 index, float contrast);

/* return the contrast adjustment for a single palette index */
float palette_entry_get_contrast(palette_t *palette, UINT32 index);

/* configure overall brightness for a palette group */
void palette_group_set_brightness(palette_t *palette, UINT32 group, float brightness);

/* configure overall contrast for a palette group */
void palette_group_set_contrast(palette_t *palette, UINT32 group, float contrast);



/* ----- palette utilities ----- */

/* normalize a range of palette entries, mapping minimum brightness to lum_min and maximum
   brightness to lum_max; if either value is < 0, that boundary value is not modified */
void palette_normalize_range(palette_t *palette, UINT32 start, UINT32 end, int lum_min, int lum_max);



/***************************************************************************
    INLINE FUNCTIONS
***************************************************************************/

/*-------------------------------------------------
    rgb_to_rgb15 - convert an RGB triplet to
    a 15-bit OSD-specified RGB value
-------------------------------------------------*/

INLINE rgb15_t rgb_to_rgb15(rgb_t rgb)
{
	return ((RGB_RED(rgb) >> 3) << 10) | ((RGB_GREEN(rgb) >> 3) << 5) | ((RGB_BLUE(rgb) >> 3) << 0);
}


/*-------------------------------------------------
    rgb_clamp - clamp an RGB component to 0-255
-------------------------------------------------*/

INLINE UINT8 rgb_clamp(INT32 value)
{
	if (value < 0)
		return 0;
	if (value > 255)
		return 255;
	return value;
}


/*-------------------------------------------------
    pal1bit - convert a 1-bit value to 8 bits
-------------------------------------------------*/

INLINE UINT8 pal1bit(UINT8 bits)
{
	return (bits & 1) ? 0xff : 0x00;
}


/*-------------------------------------------------
    pal2bit - convert a 2-bit value to 8 bits
-------------------------------------------------*/

INLINE UINT8 pal2bit(UINT8 bits)
{
	bits &= 3;
	return (bits << 6) | (bits << 4) | (bits << 2) | bits;
}


/*-------------------------------------------------
    pal3bit - convert a 3-bit value to 8 bits
-------------------------------------------------*/

INLINE UINT8 pal3bit(UINT8 bits)
{
	bits &= 7;
	return (bits << 5) | (bits << 2) | (bits >> 1);
}


/*-------------------------------------------------
    pal4bit - convert a 4-bit value to 8 bits
-------------------------------------------------*/

INLINE UINT8 pal4bit(UINT8 bits)
{
	bits &= 0xf;
	return (bits << 4) | bits;
}


/*-------------------------------------------------
    pal5bit - convert a 5-bit value to 8 bits
-------------------------------------------------*/

INLINE UINT8 pal5bit(UINT8 bits)
{
	bits &= 0x1f;
	return (bits << 3) | (bits >> 2);
}


/*-------------------------------------------------
    pal6bit - convert a 6-bit value to 8 bits
-------------------------------------------------*/

INLINE UINT8 pal6bit(UINT8 bits)
{
	bits &= 0x3f;
	return (bits << 2) | (bits >> 4);
}


/*-------------------------------------------------
    pal7bit - convert a 7-bit value to 8 bits
-------------------------------------------------*/

INLINE UINT8 pal7bit(UINT8 bits)
{
	bits &= 0x7f;
	return (bits << 1) | (bits >> 6);
}


#endif	/* __PALETTE_H__ */
