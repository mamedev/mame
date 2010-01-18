/*************************************************************************

    laserdsc.h

    Generic laserdisc support.

    Copyright Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

*************************************************************************/

#pragma once

#ifndef __LASERDSC_H__
#define __LASERDSC_H__

#include "chd.h"



/***************************************************************************
    CONSTANTS
***************************************************************************/

/* types of players supported */
enum
{
	LASERDISC_TYPE_UNKNOWN,
	LASERDISC_TYPE_PIONEER_PR7820,			/* Pioneer PR-7820 */
	LASERDISC_TYPE_PIONEER_PR8210,			/* Pioneer PR-8210 / LD-V1100 */
	LASERDISC_TYPE_SIMUTREK_SPECIAL,		/* Pioneer PR-8210 with mods */
	LASERDISC_TYPE_PIONEER_LDV1000,			/* Pioneer LD-V1000 */
	LASERDISC_TYPE_PHILLIPS_22VP931,		/* Phillips 22VP931 */
	LASERDISC_TYPE_PHILLIPS_22VP932,		/* Phillips 22VP932 (PAL) */
	LASERDISC_TYPE_SONY_LDP1450				/* Sony LDP-1450 */
};

/* laserdisc control lines */
#define LASERDISC_LINE_ENTER		0			/* "ENTER" key/line */
#define LASERDISC_LINE_CONTROL		1			/* "CONTROL" line */
#define LASERDISC_LINE_RESET		2			/* "RESET" line */
#define LASERDISC_INPUT_LINES		3

/* laserdisc status lines */
#define LASERDISC_LINE_READY		0			/* "READY" line */
#define LASERDISC_LINE_STATUS		1			/* "STATUS STROBE" line */
#define LASERDISC_LINE_COMMAND		2			/* "COMMAND STROBE" line */
#define LASERDISC_LINE_DATA_AVAIL	3			/* data available "line" */
#define LASERDISC_OUTPUT_LINES		4

/* laserdisc field codes */
#define LASERDISC_CODE_WHITE_FLAG	11			/* boolean white flag */
#define LASERDISC_CODE_LINE16		16			/* 24-bit line 16 code */
#define LASERDISC_CODE_LINE17		17			/* 24-bit line 17 code */
#define LASERDISC_CODE_LINE18		18			/* 24-bit line 18 code */
#define LASERDISC_CODE_LINE1718		1718		/* 24-bit best of line 17/18 code */



/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

typedef chd_file *(*laserdisc_get_disc_func)(running_device *device);

typedef void (*laserdisc_audio_func)(running_device *device, int samplerate, int samples, const INT16 *ch0, const INT16 *ch1);

typedef void (*vp931_data_ready_func)(running_device *device, int state);

typedef struct _laserdisc_config laserdisc_config;
struct _laserdisc_config
{
	UINT32					type;
	laserdisc_get_disc_func	getdisc;
	laserdisc_audio_func	audio;
	const char *			sound;
	const char *			screen;

	/* overlay information */
	video_update_func		overupdate;
	UINT32					overwidth, overheight, overformat;
	rectangle				overclip;
	float					overposx, overposy;
	float					overscalex, overscaley;
};



/***************************************************************************
    DEVICE CONFIGURATION MACROS
***************************************************************************/

#define MDRV_LASERDISC_ADD(_tag, _type, _screen, _sound) \
	MDRV_DEVICE_ADD(_tag, LASERDISC, 0) \
	MDRV_DEVICE_CONFIG_DATA32(laserdisc_config, type, LASERDISC_TYPE_##_type) \
	MDRV_DEVICE_CONFIG_DATAPTR(laserdisc_config, screen, _screen) \
	MDRV_DEVICE_CONFIG_DATAPTR(laserdisc_config, sound, _sound) \

#define MDRV_LASERDISC_GET_DISC(_func) \
	MDRV_DEVICE_CONFIG_DATAPTR(laserdisc_config, getdisc, _func)

#define MDRV_LASERDISC_AUDIO(_func) \
	MDRV_DEVICE_CONFIG_DATAPTR(laserdisc_config, audio, _func)

#define MDRV_LASERDISC_OVERLAY(_update, _width, _height, _format) \
	MDRV_DEVICE_CONFIG_DATAPTR(laserdisc_config, overupdate, VIDEO_UPDATE_NAME(_update)) \
	MDRV_DEVICE_CONFIG_DATA32(laserdisc_config, overwidth, _width) \
	MDRV_DEVICE_CONFIG_DATA32(laserdisc_config, overheight, _height) \
	MDRV_DEVICE_CONFIG_DATA32(laserdisc_config, overformat, _format)

#define MDRV_LASERDISC_OVERLAY_CLIP(_minx, _maxx, _miny, _maxy) \
	MDRV_DEVICE_CONFIG_DATA32(laserdisc_config, overclip.min_x, _minx) \
	MDRV_DEVICE_CONFIG_DATA32(laserdisc_config, overclip.max_x, _maxx) \
	MDRV_DEVICE_CONFIG_DATA32(laserdisc_config, overclip.min_y, _miny) \
	MDRV_DEVICE_CONFIG_DATA32(laserdisc_config, overclip.max_y, _maxy)

#define MDRV_LASERDISC_OVERLAY_POSITION(_posx, _posy) \
	MDRV_DEVICE_CONFIG_DATAFP32(laserdisc_config, overposx, _posx, 24) \
	MDRV_DEVICE_CONFIG_DATAFP32(laserdisc_config, overposy, _posy, 24)

#define MDRV_LASERDISC_OVERLAY_SCALE(_scalex, _scaley) \
	MDRV_DEVICE_CONFIG_DATAFP32(laserdisc_config, overscalex, _scalex, 24) \
	MDRV_DEVICE_CONFIG_DATAFP32(laserdisc_config, overscaley, _scaley, 24)


/* use these to add laserdisc screens with proper video update parameters */
#define MDRV_LASERDISC_SCREEN_ADD_NTSC(_tag, _overlayformat) \
	MDRV_VIDEO_ATTRIBUTES(VIDEO_SELF_RENDER) \
	MDRV_VIDEO_UPDATE(laserdisc) \
	\
	MDRV_SCREEN_ADD(_tag, RASTER) \
	MDRV_SCREEN_FORMAT(_overlayformat) \
	MDRV_SCREEN_RAW_PARAMS(XTAL_14_31818MHz*2, 910, 0, 704, 525, 44, 524) \

/* not correct yet; fix me... */
#define MDRV_LASERDISC_SCREEN_ADD_PAL(_tag, _format) \
	MDRV_VIDEO_ATTRIBUTES(VIDEO_SELF_RENDER) \
	MDRV_VIDEO_UPDATE(laserdisc) \
	\
	MDRV_SCREEN_ADD(_tag, RASTER) \
	MDRV_SCREEN_FORMAT(_format) \
	MDRV_SCREEN_RAW_PARAMS(XTAL_14_31818MHz, 910, 0, 704, 525.0/2, 0, 480/2) \



/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/


/* ----- core control and status ----- */

/* get a bitmap for the current frame; return TRUE if valid or FALSE if video off */
int laserdisc_get_video(running_device *device, bitmap_t **bitmap);

/* return the raw philips or white flag codes */
UINT32 laserdisc_get_field_code(running_device *device, UINT32 code, UINT8 zero_if_squelched);



/* ----- input and output ----- */

/* write to the parallel data port of the player */
void laserdisc_data_w(running_device *device, UINT8 data);

/* assert or clear a signal line connected to the player */
void laserdisc_line_w(running_device *device, UINT8 line, UINT8 newstate);

/* read from the parallel data port of the player */
UINT8 laserdisc_data_r(running_device *device);

/* read the state of a signal line connected to the player */
UINT8 laserdisc_line_r(running_device *device, UINT8 line);



/* ----- player specifics ----- */

/* specify the "slow" speed of the Pioneer PR-7820 */
void pr7820_set_slow_speed(running_device *device, double frame_rate_scaler);

/* set a callback for data ready on the Phillips 22VP931 */
void vp931_set_data_ready_callback(running_device *device, vp931_data_ready_func callback);

/* control the audio squelch of the Simutrek modified players */
void simutrek_set_audio_squelch(running_device *device, int state);



/* ----- video interface ----- */

/* enable/disable the video */
void laserdisc_video_enable(running_device *device, int enable);

/* enable/disable the overlay */
void laserdisc_overlay_enable(running_device *device, int enable);

/* video update callback */
VIDEO_UPDATE( laserdisc );



/* ----- configuration ----- */

/* return a copy of the current live configuration settings */
void laserdisc_get_config(running_device *device, laserdisc_config *config);

/* change the current live configuration settings */
void laserdisc_set_config(running_device *device, const laserdisc_config *config);



/* ----- device interface ----- */

/* device get info callback */
#define LASERDISC DEVICE_GET_INFO_NAME(laserdisc)
DEVICE_GET_INFO( laserdisc );

/* audio get info callback */
#define SOUND_LASERDISC DEVICE_GET_INFO_NAME(laserdisc_sound)
DEVICE_GET_INFO( laserdisc_sound );

/* type setter */
int laserdisc_get_type(running_device *device);
void laserdisc_set_type(running_device *device, int type);


#endif	/* __LASERDSC_H__ */
