/***************************************************************************

    video.h

    Core MAME video routines.

    Copyright Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

***************************************************************************/

#pragma once

#ifndef __VIDEO_H__
#define __VIDEO_H__

#include "mamecore.h"
#include "timer.h"


/***************************************************************************
    CONSTANTS
***************************************************************************/

/* maximum number of screens for one game */
#define MAX_SCREENS					8

/* number of levels of frameskipping supported */
#define FRAMESKIP_LEVELS			12
#define MAX_FRAMESKIP				(FRAMESKIP_LEVELS - 2)



/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

/*-------------------------------------------------
    screen_state - current live state of a screen
-------------------------------------------------*/

typedef struct _screen_state screen_state;
struct _screen_state
{
	int				width, height;				/* total width/height (HTOTAL, VTOTAL) */
	rectangle		visarea;					/* visible area (HBLANK end/start, VBLANK end/start) */
	UINT8			oldstyle_vblank_supplied;	/* MDRV_SCREEN_VBLANK_TIME macro used */
	attoseconds_t	refresh;					/* refresh period */
	attoseconds_t	vblank;						/* duration of a VBLANK */
	bitmap_format	format;						/* bitmap format */
};


/*-------------------------------------------------
    screen_config - configuration of a single
    screen
-------------------------------------------------*/

typedef struct _screen_config screen_config;
struct _screen_config
{
	const char *	tag;						/* nametag for the screen */
	UINT32			palette_base;				/* base palette entry for this screen */
	screen_state	defstate;					/* default state */
	float			xoffset, yoffset;			/* default X/Y offsets */
	float			xscale, yscale;				/* default X/Y scale factor */
};



/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

/* ----- core implementation ----- */

/* core initialization */
void video_init(running_machine *machine);

/* core VBLANK callback */
void video_vblank_start(running_machine *machine);


/* ----- screen management ----- */

/* set the resolution of a screen */
void video_screen_configure(int scrnum, int width, int height, const rectangle *visarea, attoseconds_t refresh);

/* set the visible area of a screen; this is a subset of video_screen_configure */
void video_screen_set_visarea(int scrnum, int min_x, int max_x, int min_y, int max_y);

/* force a partial update of the screen up to and including the requested scanline */
void video_screen_update_partial(int scrnum, int scanline);

/* return the current vertical or horizontal position of the beam for a screen */
int video_screen_get_vpos(int scrnum);
int video_screen_get_hpos(int scrnum);

/* return the current vertical or horizontal blanking state for a screen */
int video_screen_get_vblank(int scrnum);
int video_screen_get_hblank(int scrnum);

/* return the time when the beam will reach a particular H,V position */
attotime video_screen_get_time_until_pos(int scrnum, int vpos, int hpos);

/* return the amount of time the beam takes to draw one scan line */
attotime video_screen_get_scan_period(int scrnum);

/* return the amount of time the beam takes to draw one complete frame */
attotime video_screen_get_frame_period(int scrnum);

/* returns whether a given screen exists */
int video_screen_exists(int scrnum);



/* ----- global rendering ----- */

/* update the screen, handling frame skipping and rendering */
void video_frame_update(running_machine *machine, int debug);


/* ----- throttling/frameskipping/performance ----- */

/* are we skipping the current frame? */
int video_skip_this_frame(void);

/* get/set the speed factor as an integer * 100 */
int video_get_speed_factor(void);
void video_set_speed_factor(int speed);

/* return text to display about the current speed */
const char *video_get_speed_text(void);

/* get/set the current frameskip (-1 means auto) */
int video_get_frameskip(void);
void video_set_frameskip(int frameskip);

/* get/set the current throttle */
int video_get_throttle(void);
void video_set_throttle(int throttle);

/* get/set the current fastforward state */
int video_get_fastforward(void);
void video_set_fastforward(int fastforward);


/* ----- snapshots ----- */

/* save a snapshot of a given screen */
void video_screen_save_snapshot(running_machine *machine, mame_file *fp, int scrnum);

/* save a snapshot of all the active screens */
void video_save_active_screen_snapshots(running_machine *machine);


/* ----- movie recording ----- */

/* Movie recording */
int video_is_movie_active(running_machine *machine, int scrnum);
void video_movie_begin_recording(running_machine *machine, int scrnum, const char *name);
void video_movie_end_recording(running_machine *machine, int scrnum);


/* ----- crosshair rendering ----- */

void video_crosshair_toggle(void);

#endif	/* __VIDEO_H__ */
