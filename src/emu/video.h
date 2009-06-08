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
#include "render.h"


/***************************************************************************
    CONSTANTS
***************************************************************************/

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

#define video_screen_get_format(screen)	(((screen_config *)(screen)->inline_config)->format)

/* allocates a bitmap that has the same dimensions and format as the passed in screen */
#define video_screen_auto_bitmap_alloc(screen)	auto_bitmap_alloc(screen->machine, video_screen_get_width(screen), video_screen_get_height(screen), video_screen_get_format(screen))




/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

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
    vblank_state_changed_func -
    callback that is called to notify of a change
    in the VBLANK state
-------------------------------------------------*/

typedef void (*vblank_state_changed_func)(const device_config *device, void *param, int vblank_state);



/***************************************************************************
    SCREEN DEVICE CONFIGURATION MACROS
***************************************************************************/

#define MDRV_SCREEN_ADD(_tag, _type) \
	MDRV_DEVICE_ADD(_tag, VIDEO_SCREEN, 0) \
	MDRV_DEVICE_CONFIG_DATA32(screen_config, type, SCREEN_TYPE_##_type)

#define MDRV_SCREEN_MODIFY(_tag) \
	MDRV_DEVICE_MODIFY(_tag)

#define MDRV_SCREEN_FORMAT(_format) \
	MDRV_DEVICE_CONFIG_DATA32(screen_config, format, _format)

#define MDRV_SCREEN_TYPE(_type) \
	MDRV_DEVICE_CONFIG_DATA32(screen_config, type, SCREEN_TYPE_##_type)

#define MDRV_SCREEN_RAW_PARAMS(_pixclock, _htotal, _hbend, _hbstart, _vtotal, _vbend, _vbstart) \
	MDRV_DEVICE_CONFIG_DATA64(screen_config, refresh, HZ_TO_ATTOSECONDS(_pixclock) * (_htotal) * (_vtotal)) \
	MDRV_DEVICE_CONFIG_DATA64(screen_config, vblank, ((HZ_TO_ATTOSECONDS(_pixclock) * (_htotal) * (_vtotal)) / (_vtotal)) * ((_vtotal) - ((_vbstart) - (_vbend)))) \
	MDRV_DEVICE_CONFIG_DATA32(screen_config, width, _htotal)	\
	MDRV_DEVICE_CONFIG_DATA32(screen_config, height, _vtotal)	\
	MDRV_DEVICE_CONFIG_DATA32(screen_config, visarea.min_x, _hbend) \
	MDRV_DEVICE_CONFIG_DATA32(screen_config, visarea.max_x, (_hbstart) - 1) \
	MDRV_DEVICE_CONFIG_DATA32(screen_config, visarea.min_y, _vbend) \
	MDRV_DEVICE_CONFIG_DATA32(screen_config, visarea.max_y, (_vbstart) - 1)

#define MDRV_SCREEN_REFRESH_RATE(_rate) \
	MDRV_DEVICE_CONFIG_DATA64(screen_config, refresh, HZ_TO_ATTOSECONDS(_rate))

#define MDRV_SCREEN_VBLANK_TIME(_time) \
	MDRV_DEVICE_CONFIG_DATA64(screen_config, vblank, _time) \
	MDRV_DEVICE_CONFIG_DATA32(screen_config, oldstyle_vblank_supplied, TRUE)

#define MDRV_SCREEN_SIZE(_width, _height) \
	MDRV_DEVICE_CONFIG_DATA32(screen_config, width, _width) \
	MDRV_DEVICE_CONFIG_DATA32(screen_config, height, _height)

#define MDRV_SCREEN_VISIBLE_AREA(_minx, _maxx, _miny, _maxy) \
	MDRV_DEVICE_CONFIG_DATA32(screen_config, visarea.min_x, _minx) \
	MDRV_DEVICE_CONFIG_DATA32(screen_config, visarea.max_x, _maxx) \
	MDRV_DEVICE_CONFIG_DATA32(screen_config, visarea.min_y, _miny) \
	MDRV_DEVICE_CONFIG_DATA32(screen_config, visarea.max_y, _maxy)

#define MDRV_SCREEN_DEFAULT_POSITION(_xscale, _xoffs, _yscale, _yoffs)	\
	MDRV_DEVICE_CONFIG_DATAFP32(screen_config, xoffset, _xoffs, 24) \
	MDRV_DEVICE_CONFIG_DATAFP32(screen_config, xscale, _xscale, 24) \
	MDRV_DEVICE_CONFIG_DATAFP32(screen_config, yoffset, _yoffs, 24) \
	MDRV_DEVICE_CONFIG_DATAFP32(screen_config, yscale, _yscale, 24)



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
int video_screen_update_partial(const device_config *screen, int scanline);

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
void video_screen_register_vblank_callback(const device_config *screen, vblank_state_changed_func vblank_callback, void *param);


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

/* return the current effective speed percentage */
double video_get_speed_percent(running_machine *machine);

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
void video_screen_save_snapshot(running_machine *machine, const device_config *screen, mame_file *fp);

/* save a snapshot of all the active screens */
void video_save_active_screen_snapshots(running_machine *machine);


/* ----- movie recording ----- */

int video_mng_is_movie_active(running_machine *machine);
void video_mng_begin_recording(running_machine *machine, const char *name);
void video_mng_end_recording(running_machine *machine);

void video_avi_begin_recording(running_machine *machine, const char *name);
void video_avi_end_recording(running_machine *machine);
void video_avi_add_sound(running_machine *machine, const INT16 *sound, int numsamples);


/* ----- configuration helpers ----- */

/* select a view for a given target */
int video_get_view_for_target(running_machine *machine, render_target *target, const char *viewname, int targetindex, int numtargets);


/* ----- debugging helpers ----- */

/* assert if any pixels in the given bitmap contain an invalid palette index */
void video_assert_out_of_range_pixels(running_machine *machine, bitmap_t *bitmap);


#endif	/* __VIDEO_H__ */
