/***************************************************************************

    video.h

    Core MAME video routines.

    Copyright Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

***************************************************************************/

#pragma once

#ifndef __VIDEO_H__
#define __VIDEO_H__


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
	int				width, height;				/* current total width/height (HTOTAL, VTOTAL) */
	rectangle		visarea;					/* current visible area (HBLANK end/start, VBLANK end/start) */
	bitmap_format	format;						/* bitmap format (a copy of screen_config) */
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
	int				width, height;				/* default total width/height (HTOTAL, VTOTAL) */
	rectangle		visarea;					/* default visible area (HBLANK end/start, VBLANK end/start) */
	UINT8			oldstyle_vblank_supplied;	/* MDRV_SCREEN_VBLANK_TIME macro used */
	attoseconds_t	refresh;					/* default refresh period */
	attoseconds_t	vblank;						/* duration of a VBLANK */
	bitmap_format	format;						/* bitmap format */
	float			xoffset, yoffset;			/* default X/Y offsets */
	float			xscale, yscale;				/* default X/Y scale factor */
};


/*-------------------------------------------------
    vblank_state_changed_func
    vblank_state_changed_global_func -
    callback that is called to notify of a change
    in the VBLANK state
-------------------------------------------------*/

typedef void (*vblank_state_changed_func)(const device_config *device, int vblank_state);
typedef void (*vblank_state_changed_global_func)(running_machine *machine, int vblank_state);



/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

/* ----- core implementation ----- */

/* core initialization */
void video_init(running_machine *machine);


/* ----- screen management ----- */

/* set the resolution of a screen */
void video_screen_configure(const device_config *screen, int width, int height, const rectangle *visarea, attoseconds_t refresh);

/* set the visible area of a screen; this is a subset of video_screen_configure */
void video_screen_set_visarea(const device_config *screen, int min_x, int max_x, int min_y, int max_y);

/* force a partial update of the screen up to and including the requested scanline */
void video_screen_update_partial(const device_config *screen, int scanline);

/* force an update from the last beam position up to the current beam position */
void video_screen_update_now(const device_config *screen);

/* return the current vertical or horizontal position of the beam for a screen */
int video_screen_get_vpos(const device_config *screen);
int video_screen_get_hpos(const device_config *screen);

/* return the current vertical or horizontal blanking state for a screen */
int video_screen_get_vblank(const device_config *screen);
int video_screen_get_hblank(const device_config *screen);

/* return the current width for a screen */
int video_screen_get_width(const device_config *screen);

/* return the current height for a screen */
int video_screen_get_height(const device_config *screen);

/* return the current visible area for a screen */
const rectangle *video_screen_get_visible_area(const device_config *screen);

/* return the time when the beam will reach a particular H,V position */
attotime video_screen_get_time_until_pos(const device_config *screen, int vpos, int hpos);

/* return the time when the beam will reach the start of VBLANK */
attotime video_screen_get_time_until_vblank_start(const device_config *screen);

/* return the time when the beam will reach the end of VBLANK */
attotime video_screen_get_time_until_vblank_end(const device_config *screen);

/* return the time when the VIDEO_UPDATE function will be called */
attotime video_screen_get_time_until_update(const device_config *screen);

/* return the amount of time the beam takes to draw one scan line */
attotime video_screen_get_scan_period(const device_config *screen);

/* return the amount of time the beam takes to draw one complete frame */
attotime video_screen_get_frame_period(const device_config *screen);

/* return the current frame number -- this is always increasing */
UINT64 video_screen_get_frame_number(const device_config *screen);

/* registers a VBLANK callback for the given screen */
void video_screen_register_vbl_cb(const device_config *screen, vblank_state_changed_func vbl_cb);

/* registers a VBLANK callback independent of a screen  */
void video_screen_register_global_vbl_cb(vblank_state_changed_global_func vbl_cb);


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
const char *video_get_speed_text(running_machine *machine);

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
void video_screen_save_snapshot(const device_config *screen, mame_file *fp);

/* save a snapshot of all the active screens */
void video_save_active_screen_snapshots(running_machine *machine);


/* ----- movie recording ----- */

int video_is_movie_active(const device_config *screen);
void video_movie_begin_recording(const device_config *screen, const char *name);
void video_movie_end_recording(const device_config *screen);


#endif	/* __VIDEO_H__ */
