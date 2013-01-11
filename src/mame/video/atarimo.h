/***************************************************************************

    atarimo.h

    Common motion object management functions for Atari raster games.

***************************************************************************/

#ifndef __ATARIMO__
#define __ATARIMO__


/***************************************************************************
    CONSTANTS
***************************************************************************/

/* maximum number of motion object processors */
#define ATARIMO_MAX             2

/* maximum objects per bank */
#define ATARIMO_MAXPERBANK      1024

/* shift to get to priority in raw data */
#define ATARIMO_PRIORITY_SHIFT  12
#define ATARIMO_PRIORITY_MASK   ((~0 << ATARIMO_PRIORITY_SHIFT) & 0xffff)
#define ATARIMO_DATA_MASK       (ATARIMO_PRIORITY_MASK ^ 0xffff)



/***************************************************************************
    TYPES & STRUCTURES
***************************************************************************/

/* callback for special processing */
typedef int (*atarimo_special_func)(bitmap_ind16 &bitmap, const rectangle &clip, int code, int color, int xpos, int ypos, rectangle *mobounds);

/* description for a four-word mask */
struct atarimo_entry
{
	UINT16          data[4];
};

/* description of the motion objects */
struct atarimo_desc
{
	UINT8               gfxindex;           /* index to which gfx system */
	UINT8               banks;              /* number of motion object banks */
	UINT8               linked;             /* are the entries linked? */
	UINT8               split;              /* are the entries split? */
	UINT8               reverse;            /* render in reverse order? */
	UINT8               swapxy;             /* render in swapped X/Y order? */
	UINT8               nextneighbor;       /* does the neighbor bit affect the next object? */
	UINT16              slipheight;         /* pixels per SLIP entry (0 for no-slip) */
	UINT8               slipoffset;         /* pixel offset for SLIPs */
	UINT16              maxlinks;           /* maximum number of links to visit/scanline (0=all) */

	UINT16              palettebase;        /* base palette entry */
	UINT16              maxcolors;          /* maximum number of colors */
	UINT8               transpen;           /* transparent pen index */

	atarimo_entry       linkmask;           /* mask for the link */
	atarimo_entry       gfxmask;            /* mask for the graphics bank */
	atarimo_entry       codemask;           /* mask for the code index */
	atarimo_entry       codehighmask;       /* mask for the upper code index */
	atarimo_entry       colormask;          /* mask for the color */
	atarimo_entry       xposmask;           /* mask for the X position */
	atarimo_entry       yposmask;           /* mask for the Y position */
	atarimo_entry       widthmask;          /* mask for the width, in tiles*/
	atarimo_entry       heightmask;         /* mask for the height, in tiles */
	atarimo_entry       hflipmask;          /* mask for the horizontal flip */
	atarimo_entry       vflipmask;          /* mask for the vertical flip */
	atarimo_entry       prioritymask;       /* mask for the priority */
	atarimo_entry       neighbormask;       /* mask for the neighbor */
	atarimo_entry       absolutemask;       /* mask for absolute coordinates */

	atarimo_entry       specialmask;        /* mask for the special value */
	UINT16              specialvalue;       /* resulting value to indicate "special" */
	atarimo_special_func specialcb;         /* callback routine for special entries */
};

/* rectangle list */
struct atarimo_rect_list
{
	int                 numrects;
	rectangle *         rect;
};


/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

/* setup/shutdown */
void atarimo_init(running_machine &machine, int map, const atarimo_desc *desc);
UINT16 *atarimo_get_code_lookup(int map, int *size);
UINT8 *atarimo_get_color_lookup(int map, int *size);
UINT8 *atarimo_get_gfx_lookup(int map, int *size);

/* core processing */
bitmap_ind16 *atarimo_render(int map, const rectangle &cliprect, atarimo_rect_list *rectlist);

/* atrribute setters */
void atarimo_set_bank(int map, int bank);
void atarimo_set_xscroll(int map, int xscroll);
void atarimo_set_yscroll(int map, int yscroll);
void atarimo_set_slipram(int map, UINT16 *ram);

/* atrribute getters */
int atarimo_get_bank(int map);
int atarimo_get_xscroll(int map);
int atarimo_get_yscroll(int map);

/* read/write handlers */
DECLARE_READ16_HANDLER( atarimo_0_spriteram_r );
DECLARE_READ16_HANDLER( atarimo_0_slipram_r );

DECLARE_READ16_HANDLER( atarimo_1_spriteram_r );
DECLARE_READ16_HANDLER( atarimo_1_slipram_r );

DECLARE_WRITE16_HANDLER( atarimo_0_spriteram_w );
DECLARE_WRITE16_HANDLER( atarimo_0_spriteram_expanded_w );
DECLARE_WRITE16_HANDLER( atarimo_0_slipram_w );

DECLARE_WRITE16_HANDLER( atarimo_1_spriteram_w );
DECLARE_WRITE16_HANDLER( atarimo_1_slipram_w );

void atarimo_mark_high_palette(bitmap_ind16 &bitmap, UINT16 *pf, UINT16 *mo, int x, int y);


#endif
