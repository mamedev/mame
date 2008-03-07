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
#include "devintrf.h"
#include "timer.h"


/***************************************************************************
    CONSTANTS
***************************************************************************/

/* maximum number of screens for one game */
#define MAX_SCREENS					8

/* number of levels of frameskipping supported */
#define FRAMESKIP_LEVELS			12
#define MAX_FRAMESKIP				(FRAMESKIP_LEVELS - 2)

/* screen types */
enum
{
	SCREEN_TYPE_INVALID = 0,
	SCREEN_TYPE_RASTER,
	SCREEN_TYPE_VECTOR,
	SCREEN_TYPE_LCD
};



/***************************************************************************
    MACROS
***************************************************************************/

/* these functions are macros primarily due to include file ordering */
/* plus, they are very simple */
#define video_screen_count(config)		device_list_items((config)->devicelist, VIDEO_SCREEN)
#define video_screen_first(config)		device_list_first((config)->devicelist, VIDEO_SCREEN)
#define video_screen_next(previous)		device_list_next((previous), VIDEO_SCREEN)



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
	void *			private_data;				/* pointer to the private data structure */
};


/*-------------------------------------------------
    screen_config - configuration of a single
    screen
-------------------------------------------------*/

typedef struct _screen_config screen_config;
struct _screen_config
{
	int				type;						/* type of screen */
	screen_state	defstate;					/* default state */
	float			xoffset, yoffset;			/* default X/Y offsets */
	float			xscale, yscale;				/* default X/Y scale factor */
};


/*-------------------------------------------------
    vblank_state_changed_func - callback that a
    screen calls to notify of a change of
    the VBLANK state
-------------------------------------------------*/

typedef void (*vblank_state_changed_func)(const device_config *device, int vblank_state);



/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

/* ----- core implementation ----- */

/* core initialization */
void video_init(running_machine *machine);


/* ----- screen management ----- */

/* set the resolution of a screen */
void video_screen_configure(int scrnum, int width, int height, const rectangle *visarea, attoseconds_t refresh);

/* set the visible area of a screen; this is a subset of video_screen_configure */
void video_screen_set_visarea(int scrnum, int min_x, int max_x, int min_y, int max_y);

/* force a partial update of the screen up to and including the requested scanline */
void video_screen_update_partial(int scrnum, int scanline);

/* force an update from the last beam position up to the current beam position */
void video_screen_update_now(int scrnum);

/* return the current vertical or horizontal position of the beam for a screen */
int video_screen_get_vpos(int scrnum);
int video_screen_get_hpos(int scrnum);

/* return the current vertical or horizontal blanking state for a screen */
int video_screen_get_vblank(int scrnum);
int video_screen_get_hblank(int scrnum);

/* return the time when the beam will reach a particular H,V position */
attotime video_screen_get_time_until_pos(int scrnum, int vpos, int hpos);

/* return the time when the beam will reach the start of VBLANK */
attotime video_screen_get_time_until_vblank_start(int scrnum);

/* return the time when the beam will reach the end of VBLANK */
attotime video_screen_get_time_until_vblank_end(int scrnum);

/* return the time when the VIDEO_UPDATE function will be called */
attotime video_screen_get_time_until_update(int scrnum);

/* return the amount of time the beam takes to draw one scan line */
attotime video_screen_get_scan_period(int scrnum);

/* return the amount of time the beam takes to draw one complete frame */
attotime video_screen_get_frame_period(int scrnum);

/* return the current frame number -- this is always increasing */
UINT64 video_screen_get_frame_number(int scrnum);

/* returns whether a given screen exists */
int video_screen_exists(int scrnum);

/* registers a VBLANK callback for the given screen*/
void video_screen_register_vbl_cb(running_machine *machine, void *screen, vblank_state_changed_func vbl_cb);


/* ----- video screen device interface ----- */

/* device get info callback */
#define VIDEO_SCREEN DEVICE_GET_INFO_NAME(video_screen)
DEVICE_GET_INFO( video_screen );



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
