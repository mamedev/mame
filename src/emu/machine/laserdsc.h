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
#define LASERDISC_TYPE_PR7820		1			/* Pioneer PR-7820 */
#define LASERDISC_TYPE_LDV1000		2			/* Pioneer LD-V1000 */
#define LASERDISC_TYPE_22VP932		3			/* Phillips 22VP932 (PAL) */
#define LASERDISC_TYPE_LDP1450		4			/* Sony LDP-1450 */
#define LASERDISC_TYPE_PR8210		5			/* Pioneer PR-8210 / LD-V1100 */

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
#define LASERDISC_CODE_FRAME_FLAGS	0
#define LASERDISC_CODE_WHITE_FLAG	1
#define LASERDISC_CODE_LINE16		2
#define LASERDISC_CODE_LINE17		3
#define LASERDISC_CODE_LINE18		4



/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

typedef struct _laserdisc_info laserdisc_info;



/***************************************************************************
    GLOBAL VARIABLES
***************************************************************************/

extern const struct CustomSound_interface laserdisc_custom_interface;



/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

laserdisc_info *laserdisc_init(int type, chd_file *chd, int custom_index);
void laserdisc_reset(laserdisc_info *info, int type);
void laserdisc_exit(laserdisc_info *info);
void laserdisc_vsync(laserdisc_info *info);
const char *laserdisc_describe_state(laserdisc_info *info);
UINT32 laserdisc_get_video(laserdisc_info *info, bitmap_t **bitmap);
UINT32 laserdisc_get_field_code(laserdisc_info *info, UINT8 code);

void laserdisc_data_w(laserdisc_info *info, UINT8 data);
void laserdisc_line_w(laserdisc_info *info, UINT8 line, UINT8 newstate);
UINT8 laserdisc_data_r(laserdisc_info *info);
UINT8 laserdisc_line_r(laserdisc_info *info, UINT8 line);

void pr7820_set_slow_speed(laserdisc_info *info, double frame_rate_scaler);

#endif 	/* __LASERDSC_H__ */
