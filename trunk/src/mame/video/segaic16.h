/***************************************************************************

    Sega 16-bit common hardware

***************************************************************************/

#include "devcb.h"

/* globals */
extern UINT8 segaic16_display_enable;
extern UINT16 *segaic16_tileram_0;
extern UINT16 *segaic16_textram_0;
extern UINT16 *segaic16_roadram_0;
extern UINT16 *segaic16_rotateram_0;

/* misc functions */
void segaic16_set_display_enable(running_machine &machine, int enable);

/* tilemap systems */
#define SEGAIC16_MAX_TILEMAPS       1

#define SEGAIC16_TILEMAP_HANGON     0
#define SEGAIC16_TILEMAP_16A        1
#define SEGAIC16_TILEMAP_16B        2
#define SEGAIC16_TILEMAP_16B_ALT    3

#define SEGAIC16_TILEMAP_FOREGROUND 0
#define SEGAIC16_TILEMAP_BACKGROUND 1
#define SEGAIC16_TILEMAP_TEXT       2

void segaic16_tilemap_init(running_machine &machine, int which, int type, int colorbase, int xoffs, int numbanks);
void segaic16_tilemap_reset(running_machine &machine, int which);
void segaic16_tilemap_draw(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int which, int map, int priority, int priority_mark);
void segaic16_tilemap_set_bank(running_machine &machine, int which, int banknum, int offset);
void segaic16_tilemap_set_flip(running_machine &machine, int which, int flip);
void segaic16_tilemap_set_rowscroll(running_machine &machine, int which, int enable);
void segaic16_tilemap_set_colscroll(running_machine &machine, int which, int enable);

DECLARE_WRITE16_HANDLER( segaic16_tileram_0_w );
DECLARE_WRITE16_HANDLER( segaic16_textram_0_w );

/* road systems */
#define SEGAIC16_MAX_ROADS          1

#define SEGAIC16_ROAD_HANGON        0
#define SEGAIC16_ROAD_SHARRIER      1
#define SEGAIC16_ROAD_OUTRUN        2
#define SEGAIC16_ROAD_XBOARD        3

#define SEGAIC16_ROAD_BACKGROUND    0
#define SEGAIC16_ROAD_FOREGROUND    1

void segaic16_road_init(running_machine &machine, int which, int type, int colorbase1, int colorbase2, int colorbase3, int xoffs);
void segaic16_road_draw(int which, bitmap_ind16 &bitmap, const rectangle &cliprect, int priority);
DECLARE_READ16_HANDLER( segaic16_road_control_0_r );
DECLARE_WRITE16_HANDLER( segaic16_road_control_0_w );

/* rotation systems */
#define SEGAIC16_MAX_ROTATE         1

#define SEGAIC16_ROTATE_YBOARD      0

void segaic16_rotate_init(running_machine &machine, int which, int type, int colorbase);
void segaic16_rotate_draw(running_machine &machine, int which, bitmap_ind16 &bitmap, const rectangle &cliprect, bitmap_ind16 &srcbitmap);
DECLARE_READ16_HANDLER( segaic16_rotate_control_0_r );

/*************************************
 *
 *  Type definitions
 *
 *************************************/



struct tilemap_callback_info
{
	UINT16 *        rambase;                        /* base of RAM for this tilemap page */
	const UINT8 *   bank;                           /* pointer to bank array */
	UINT16          banksize;                       /* size of banks */
};


struct tilemap_info
{
	UINT8           index;                          /* index of this structure */
	UINT8           type;                           /* type of tilemap (see segaic16.h for details) */
	UINT8           numpages;                       /* number of allocated pages */
	UINT8           flip;                           /* screen flip? */
	UINT8           rowscroll, colscroll;           /* are rowscroll/colscroll enabled (if external enables are used) */
	UINT8           bank[8];                        /* indexes of the tile banks */
	UINT16          banksize;                       /* number of tiles per bank */
	UINT16          latched_xscroll[4];             /* latched X scroll values */
	UINT16          latched_yscroll[4];             /* latched Y scroll values */
	UINT16          latched_pageselect[4];          /* latched page select values */
	INT32           xoffs;                          /* X scroll offset */
	tilemap_t *     tilemaps[16];                   /* up to 16 tilemap pages */
	tilemap_t *     textmap;                        /* a single text tilemap */
	struct tilemap_callback_info tmap_info[16];     /* callback info for 16 tilemap pages */
	struct tilemap_callback_info textmap_info;      /* callback info for a single textmap page */
	void            (*reset)(running_machine &machine, struct tilemap_info *info);/* reset callback */
	void            (*draw_layer)(running_machine &machine, struct tilemap_info *info, bitmap_ind16 &bitmap, const rectangle &cliprect, int which, int flags, int priority);
	UINT16 *        textram;                        /* pointer to textram pointer */
	UINT16 *        tileram;                        /* pointer to tileram pointer */
	emu_timer *     latch_timer;                    /* timer for latching 16b tilemap scroll values */
};

struct road_info
{
	UINT8           index;                          /* index of this structure */
	UINT8           type;                           /* type of road system (see segaic16.h for details) */
	UINT8           control;                        /* control register value */
	UINT16          colorbase1;                     /* color base for road ROM data */
	UINT16          colorbase2;                     /* color base for road background data */
	UINT16          colorbase3;                     /* color base for sky data */
	INT32           xoffs;                          /* X scroll offset */
	void            (*draw)(struct road_info *info, bitmap_ind16 &bitmap, const rectangle &cliprect, int priority);
	UINT16 *        roadram;                        /* pointer to roadram pointer */
	UINT16 *        buffer;                         /* buffered roadram pointer */
	UINT8 *         gfx;                            /* expanded road graphics */
};

struct rotate_info
{
	UINT8           index;                          /* index of this structure */
	UINT8           type;                           /* type of rotate system (see segaic16.h for details) */
	UINT16          colorbase;                      /* base color index */
	INT32           ramsize;                        /* size of rotate RAM */
	UINT16 *        rotateram;                      /* pointer to rotateram pointer */
	UINT16 *        buffer;                         /* buffered data */
};


/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

extern struct rotate_info segaic16_rotate[SEGAIC16_MAX_ROTATE];
extern struct road_info segaic16_road[SEGAIC16_MAX_ROADS];
