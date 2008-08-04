/*************************************************************************

    laserdsc.h

    Generic laserdisc support.

    Copyright Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

*************************************************************************/

#pragma once

#ifndef __LASERDSC_H__
#define __LASERDSC_H__



/***************************************************************************
    CONSTANTS
***************************************************************************/

/* types of players supported */
enum
{
	LASERDISC_TYPE_PIONEER_PR7820,			/* Pioneer PR-7820 */
	LASERDISC_TYPE_PIONEER_PR8210,			/* Pioneer PR-8210 / LD-V1100 */
	LASERDISC_TYPE_PIONEER_LDV1000,			/* Pioneer LD-V1000 */
	LASERDISC_TYPE_PHILLIPS_22VP932,		/* Phillips 22VP932 (PAL) */
	LASERDISC_TYPE_SONY_LDP1450				/* Sony LDP-1450 */
};

/* laserdisc control lines */
#define LASERDISC_LINE_ENTER		0			/* "ENTER" key/line */
#define LASERDISC_LINE_CONTROL		1			/* "CONTROL" line */
#define LASERDISC_INPUT_LINES		2

/* laserdisc status lines */
#define LASERDISC_LINE_READY		0			/* "READY" line */
#define LASERDISC_LINE_STATUS		1			/* "STATUS STROBE" line */
#define LASERDISC_LINE_COMMAND		2			/* "COMMAND STROBE" line */
#define LASERDISC_LINE_DATA_AVAIL	3			/* data available "line" */
#define LASERDISC_OUTPUT_LINES		4

/* laserdisc field codes */
#define LASERDISC_CODE_WHITE_FLAG	0
#define LASERDISC_CODE_LINE16		1
#define LASERDISC_CODE_LINE17		2
#define LASERDISC_CODE_LINE18		3

/* device configuration */
enum
{
	LDINFO_INT_TYPE = DEVINFO_INT_DEVICE_SPECIFIC
};



/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

typedef void (*laserdisc_audio_func)(const device_config *device, int samplerate, int samples, const INT16 *ch0, const INT16 *ch1);

typedef struct _laserdisc_config laserdisc_config;
struct _laserdisc_config
{
	int						type;
	laserdisc_audio_func	audio;
};



/***************************************************************************
    DEVICE CONFIGURATION MACROS
***************************************************************************/

#define MDRV_LASERDISC_ADD(_tag, _type) \
	MDRV_DEVICE_ADD(_tag, LASERDISC) \
	MDRV_DEVICE_CONFIG_DATA32(laserdisc_config, type, LASERDISC_TYPE_##_type)

#define MDRV_LASERDISC_AUDIO(_func) \
	MDRV_DEVICE_CONFIG_DATAPTR(laserdisc_config, audio, _func)
	
#define MDRV_LASERDISC_REMOVE(_tag, _type) \
	MDRV_DEVICE_REMOVE(_tag, _type)



/***************************************************************************
    GLOBAL VARIABLES
***************************************************************************/

extern const struct CustomSound_interface laserdisc_custom_interface;



/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/


/* ----- core control and status ----- */

/* call this once per field (i.e., 59.94 times/second for NTSC) */
void laserdisc_vsync(const device_config *device);

/* return a textual description of the current state (for debugging) */
const char *laserdisc_describe_state(const device_config *device);

/* get a bitmap for the current frame (and the frame number) */
UINT32 laserdisc_get_video(const device_config *device, bitmap_t **bitmap);

/* return the raw philips or white flag codes */
UINT32 laserdisc_get_field_code(const device_config *device, UINT8 code);



/* ----- input and output ----- */

/* write to the parallel data port of the player */
void laserdisc_data_w(const device_config *device, UINT8 data);

/* assert or clear a signal line connected to the player */
void laserdisc_line_w(const device_config *device, UINT8 line, UINT8 newstate);

/* read from the parallel data port of the player */
UINT8 laserdisc_data_r(const device_config *device);

/* read the state of a signal line connected to the player */
UINT8 laserdisc_line_r(const device_config *device, UINT8 line);



/* ----- player specifics ----- */

/* specify the "slow" speed of the Pioneer PR-7820 */
void pr7820_set_slow_speed(const device_config *device, double frame_rate_scaler);



/* ----- device interface ----- */

/* device get info callback */
#define LASERDISC DEVICE_GET_INFO_NAME(laserdisc)
DEVICE_GET_INFO( laserdisc );

#endif 	/* __LASERDSC_H__ */
