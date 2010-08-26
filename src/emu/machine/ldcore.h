/*************************************************************************

    ldcore.h

    Private core laserdisc player implementation.

    Copyright Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

*************************************************************************/

#pragma once

#ifndef __LDCORE_H__
#define __LDCORE_H__

#include "laserdsc.h"
#include "vbiparse.h"


/***************************************************************************
    CONSTANTS
***************************************************************************/

/* common laserdisc states */
enum
{
	LDSTATE_NONE,							/* unspecified state */
	LDSTATE_EJECTING,						/* in the process of ejecting */
	LDSTATE_EJECTED,						/* fully ejected */
	LDSTATE_PARKED,							/* head parked in lead-in */
	LDSTATE_LOADING,						/* loading from ejected state */
	LDSTATE_SPINUP,							/* spinning up */
	LDSTATE_PAUSING,						/* looking for a frame boundary to pause */
	LDSTATE_PAUSED,							/* found a frame boundary; now paused */
											/*   parameter specifies the fieldnum of the first frame */
	LDSTATE_PLAYING,						/* playing forward normally, with audio */
											/*   parameter specifies the target frame, or 0 if none */
	LDSTATE_PLAYING_SLOW_REVERSE,			/* playing slow in the reverse direction, with no audio */
											/*   parameter specifies the number of times to repeat each track */
	LDSTATE_PLAYING_SLOW_FORWARD,			/* playing slow in the forward direction, with no audio */
											/*   parameter specifies the number of times to repeat each track */
	LDSTATE_PLAYING_FAST_REVERSE,			/* playing fast in the reverse direction, with no audio */
											/*   parameter specifies the number of frames to skip backwards after each frame */
	LDSTATE_PLAYING_FAST_FORWARD,			/* playing fast in the forward direction, with no audio */
											/*   parameter specifies the number of frames to skip forwards after each frame */
	LDSTATE_STEPPING_REVERSE,				/* single frame stepping in the reverse direction */
	LDSTATE_STEPPING_FORWARD,				/* single frame stepping in the forward direction */
	LDSTATE_SCANNING,						/* scanning in the forward or reverse direction */
											/*   parameter(0:7) controls how many vsyncs until revert to savestate */
											/*   parameter(8:31) specifies the speed */
	LDSTATE_SEEKING,						/* seeking to a specific frame */
											/*   parameter specifies the target frame */
	LDSTATE_OTHER							/* other states start here */
};


/* slider position */
enum _slider_position
{
	SLIDER_MINIMUM,							/* at the minimum value */
	SLIDER_VIRTUAL_LEADIN,					/* within the virtual lead-in area */
	SLIDER_CHD,								/* within the boundaries of the CHD */
	SLIDER_OUTSIDE_CHD,						/* outside of the CHD area but before the virtual lead-out area */
	SLIDER_VIRTUAL_LEADOUT,					/* within the virtual lead-out area */
	SLIDER_MAXIMUM							/* at the maximum value */
};
typedef enum _slider_position slider_position;


/* special frame and chapter numbers from VBI conversion */
#define FRAME_NOT_PRESENT			-2						/* no frame number information present */
#define FRAME_LEAD_IN				-1						/* lead-in code detected */
#define FRAME_LEAD_OUT				99999					/* lead-out code detected */
#define CHAPTER_NOT_PRESENT			-2						/* no chapter number information present */
#define CHAPTER_LEAD_IN				-1						/* lead-in code detected */
#define CHAPTER_LEAD_OUT			100						/* lead-out code detected */

/* generic head movement speeds; use player-specific information where appropriate */
#define GENERIC_SLOW_SPEED			(5)						/* 1/5 normal speed */
#define GENERIC_FAST_SPEED			(3)						/* 3x normal speed */
#define GENERIC_SCAN_SPEED			(50)					/* 50x normal speed */
#define GENERIC_SEARCH_SPEED		(5000)					/* 5000x normal speed */

/* generic timings; use player-specific information where appropriate */
#define GENERIC_EJECT_TIME			(ATTOTIME_IN_SEC(5))
#define GENERIC_SPINUP_TIME			(ATTOTIME_IN_SEC(2))
#define GENERIC_LOAD_TIME			(ATTOTIME_IN_SEC(5))



/***************************************************************************
    MACROS
***************************************************************************/

#define SCANNING_PARAM(speed,duration)	(((speed) << 8) | ((duration) & 0xff))



/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

/* core-specific and player-specific data */
typedef struct _ldplayer_data ldplayer_data;
typedef struct _ldcore_data ldcore_data;


/* player state */
typedef struct _ldplayer_state ldplayer_state;
struct _ldplayer_state
{
	UINT8					state;					/* current state */
	INT32					substate;				/* internal sub-state; starts at 0 on any state change */
	INT32					param;					/* parameter for current state */
	attotime				endtime;				/* minimum ending time for current state */
};


/* generic data */
typedef struct _laserdisc_state laserdisc_state;
struct _laserdisc_state
{
	running_device *		device;					/* pointer to owning device */
	screen_device *	screen;					/* pointer to the screen device */
	ldcore_data *			core;					/* private core data */
	ldplayer_data *			player;					/* private player data */

	ldplayer_state			state;					/* active state */
	ldplayer_state			savestate;				/* saved state during temporary operations */
};


/* player-specific callbacks */
typedef void (*laserdisc_init_func)(laserdisc_state *ld);
typedef void (*laserdisc_vsync_func)(laserdisc_state *ld, const vbi_metadata *vbi, int fieldnum, attotime curtime);
typedef INT32 (*laserdisc_update_func)(laserdisc_state *ld, const vbi_metadata *vbi, int fieldnum, attotime curtime);
typedef void (*laserdisc_overlay_func)(laserdisc_state *ld, bitmap_t *bitmap);
typedef void (*laserdisc_w_func)(laserdisc_state *ld, UINT8 prev, UINT8 newval);
typedef UINT8 (*laserdisc_r_func)(laserdisc_state *ld);


/* player configuration */
typedef struct _ldplayer_interface ldplayer_interface;
struct _ldplayer_interface
{
	int						type;					/* type of the player */
	size_t					statesize;				/* size of the state */
	const char *			name;					/* name of the player */
	const rom_entry *		romregion;				/* pointer to ROM region information */
	machine_config_constructor machine_config;		/* pointer to machine configuration */
	laserdisc_init_func		init;					/* initialization callback */
	laserdisc_vsync_func	vsync;					/* vsync begin callback */
	laserdisc_update_func	update;					/* update callback (line 16) */
	laserdisc_overlay_func	overlay;				/* overlay callback */
	laserdisc_w_func		writedata;				/* parallel data write */
	laserdisc_w_func		writeline[LASERDISC_INPUT_LINES]; /* single line write */
	laserdisc_r_func		readdata;				/* parallel data read */
	laserdisc_r_func		readline[LASERDISC_OUTPUT_LINES]; /* single line read */
};



/***************************************************************************
    GLOBAL VARIABLES
***************************************************************************/

/* defined by each player */
extern const ldplayer_interface pr7820_interface;
extern const ldplayer_interface pr8210_interface;
extern const ldplayer_interface simutrek_interface;
extern const ldplayer_interface ldv1000_interface;
extern const ldplayer_interface ldp1450_interface;
extern const ldplayer_interface vp931_interface;
extern const ldplayer_interface vp932_interface;



/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/


/* ----- player interface ----- */

/* return a token with type checking from a device */
laserdisc_state *ldcore_get_safe_token(running_device *device);

/* set the left/right audio squelch states */
void ldcore_set_audio_squelch(laserdisc_state *ld, UINT8 squelchleft, UINT8 squelchright);

/* set the video squelch state */
void ldcore_set_video_squelch(laserdisc_state *ld, UINT8 squelch);

/* dynamically change the slider speed */
void ldcore_set_slider_speed(laserdisc_state *ld, INT32 tracks_per_vsync);

/* advance the slider by a certain number of tracks */
void ldcore_advance_slider(laserdisc_state *ld, INT32 numtracks);

/* get the current slider position */
slider_position ldcore_get_slider_position(laserdisc_state *ld);



/* ----- generic implementations ----- */

/* generically update in a way that works for most situations */
INT32 ldcore_generic_update(laserdisc_state *ld, const vbi_metadata *vbi, int fieldnum, attotime curtime, ldplayer_state *curstate);



/***************************************************************************
    INLINE FUNCTIONS
***************************************************************************/

/*-------------------------------------------------
    is_start_of_frame - return TRUE if this is
    the start of a frame
-------------------------------------------------*/

INLINE int is_start_of_frame(const vbi_metadata *vbi)
{
	/* is it not known if the white flag or the presence of a frame code
       determines the start of frame; the former seems to be the "official"
       way, but the latter seems to be the practical implementation */
	return (vbi->white || (vbi->line1718 & VBI_MASK_CAV_PICTURE) == VBI_CODE_CAV_PICTURE);
}


/*-------------------------------------------------
    frame_from_metadata - return the frame number
    encoded in the metadata, if present, or
    FRAME_NOT_PRESENT
-------------------------------------------------*/

INLINE int frame_from_metadata(const vbi_metadata *metadata)
{
	if ((metadata->line1718 & VBI_MASK_CAV_PICTURE) == VBI_CODE_CAV_PICTURE)
		return VBI_CAV_PICTURE(metadata->line1718);
	else if (metadata->line1718 == VBI_CODE_LEADIN)
		return FRAME_LEAD_IN;
	else if (metadata->line1718 == VBI_CODE_LEADOUT)
		return FRAME_LEAD_OUT;
	return FRAME_NOT_PRESENT;
}


/*-------------------------------------------------
    chapter_from_metadata - return the chapter
    number encoded in the metadata, if present,
    or CHAPTER_NOT_PRESENT
-------------------------------------------------*/

INLINE int chapter_from_metadata(const vbi_metadata *metadata)
{
	if ((metadata->line1718 & VBI_MASK_CHAPTER) == VBI_CODE_CHAPTER)
		return VBI_CHAPTER(metadata->line1718);
	else if (metadata->line1718 == VBI_CODE_LEADIN)
		return CHAPTER_LEAD_IN;
	else if (metadata->line1718 == VBI_CODE_LEADOUT)
		return CHAPTER_LEAD_OUT;
	return CHAPTER_NOT_PRESENT;
}


#endif
