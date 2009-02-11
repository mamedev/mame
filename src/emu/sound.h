/***************************************************************************

    sound.h

    Core sound interface functions and definitions.

    Copyright Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

***************************************************************************/

#pragma once

#ifndef __SOUND_H__
#define __SOUND_H__

#include "driver.h"


/***************************************************************************
    CONSTANTS
***************************************************************************/

#define MAX_OUTPUTS		256				/* maximum number of outputs a sound chip can support */
#define ALL_OUTPUTS 	(MAX_OUTPUTS)	/* special value indicating all outputs for the current chip */



/***************************************************************************
    MACROS
***************************************************************************/

/* these functions are macros primarily due to include file ordering */
/* plus, they are very simple */
#define sound_count(config)					device_list_items((config)->devicelist, SOUND)
#define sound_first(config)					device_list_first((config)->devicelist, SOUND)
#define sound_next(previous)				((previous)->typenext)

/* these functions are macros primarily due to include file ordering */
/* plus, they are very simple */
#define speaker_output_count(config)		device_list_items((config)->devicelist, SPEAKER_OUTPUT)
#define speaker_output_first(config)		device_list_first((config)->devicelist, SPEAKER_OUTPUT)
#define speaker_output_next(previous)		((previous)->typenext)



/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

/* sound type is just a device type */
typedef device_type sound_type;


/* Sound route for the machine driver */
typedef struct _sound_route sound_route;
struct _sound_route
{
	const char *		target;					/* tag of the target */
	int					input;					/* input ID, -1 is default behavior */
	float				gain;					/* gain */
};


/* Sound configuration for the machine driver */
typedef struct _sound_config sound_config;
struct _sound_config
{
	sound_type			type;					/* type of sound chip */
	sound_route			route[ALL_OUTPUTS+1];	/* one route per output */
};


/* Speaker configuration for the machine driver */
typedef struct _speaker_config speaker_config;
struct _speaker_config
{
	float				x, y, z;				/* positioning vector */
};



/***************************************************************************
    SOUND DEVICE CONFIGURATION MACROS
***************************************************************************/

#define MDRV_SOUND_ADD(_tag, _type, _clock) \
	MDRV_DEVICE_ADD(_tag, SOUND, _clock) \
	MDRV_DEVICE_CONFIG_DATAPTR(sound_config, type, SOUND_##_type)

#define MDRV_SOUND_REMOVE(_tag) \
	MDRV_DEVICE_REMOVE(_tag, SOUND)

#define MDRV_SOUND_MODIFY(_tag) \
	MDRV_DEVICE_MODIFY(_tag, SOUND)

#define MDRV_SOUND_TYPE(_type) \
	MDRV_DEVICE_CONFIG_DATAPTR(sound_config, type, SOUND_##_type)

#define MDRV_SOUND_CLOCK(_clock) \
	MDRV_DEVICE_CLOCK(_clock)

#define MDRV_SOUND_REPLACE(_tag, _type, _clock) \
	MDRV_DEVICE_MODIFY(_tag, SOUND) \
	MDRV_DEVICE_CONFIG_CLEAR() \
	MDRV_DEVICE_CONFIG_DATAPTR(sound_config, type, SOUND_##_type) \
	MDRV_DEVICE_CLOCK(_clock)

#define MDRV_SOUND_CONFIG(_config) \
	MDRV_DEVICE_CONFIG(_config)

#define MDRV_SOUND_ROUTE_EX(_output, _target, _gain, _input) \
	MDRV_DEVICE_CONFIG_DATAPTR_ARRAY_MEMBER(sound_config, route, _output, sound_route, target, _target) \
	MDRV_DEVICE_CONFIG_DATA32_ARRAY_MEMBER(sound_config, route, _output, sound_route, input, _input) \
	MDRV_DEVICE_CONFIG_DATAFP32_ARRAY_MEMBER(sound_config, route, _output, sound_route, gain, _gain, 24)

#define MDRV_SOUND_ROUTE(_output, _target, _gain) \
	MDRV_SOUND_ROUTE_EX(_output, _target, _gain, 0)



/* add/remove speakers */
#define MDRV_SPEAKER_ADD(_tag, _x, _y, _z) \
	MDRV_DEVICE_ADD(_tag, SPEAKER_OUTPUT, 0) \
	MDRV_DEVICE_CONFIG_DATAFP32(speaker_config, x, _x, 24) \
	MDRV_DEVICE_CONFIG_DATAFP32(speaker_config, y, _y, 24) \
	MDRV_DEVICE_CONFIG_DATAFP32(speaker_config, z, _z, 24)

#define MDRV_SPEAKER_REMOVE(_tag) \
	MDRV_DEVICE_REMOVE(_tag, SPEAKER_OUTPUT)

#define MDRV_SPEAKER_STANDARD_MONO(_tag) \
	MDRV_SPEAKER_ADD(_tag, 0.0, 0.0, 1.0)

#define MDRV_SPEAKER_STANDARD_STEREO(_tagl, _tagr) \
	MDRV_SPEAKER_ADD(_tagl, -0.2, 0.0, 1.0) \
	MDRV_SPEAKER_ADD(_tagr, 0.2, 0.0, 1.0)



/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

/* core interfaces */
void sound_init(running_machine *machine);



/* ----- sound device interface ----- */

/* device get info callback */
#define SOUND DEVICE_GET_INFO_NAME(sound)
DEVICE_GET_INFO( sound );



/* global sound controls */
void sound_mute(int mute);
void sound_set_attenuation(int attenuation);
int sound_get_attenuation(void);
void sound_global_enable(int enable);

/* user gain controls on speaker inputs for mixing */
int sound_get_user_gain_count(running_machine *machine);
void sound_set_user_gain(running_machine *machine, int index, float gain);
float sound_get_user_gain(running_machine *machine, int index);
float sound_get_default_gain(running_machine *machine, int index);
const char *sound_get_user_gain_name(running_machine *machine, int index);


/* driver gain controls on chip outputs */
void sound_set_output_gain(const device_config *device, int output, float gain);


/* ----- sound speaker device interface ----- */

/* device get info callback */
#define SPEAKER_OUTPUT DEVICE_GET_INFO_NAME(speaker_output)
DEVICE_GET_INFO( speaker_output );



/***************************************************************************
    INLINE FUNCTIONS
***************************************************************************/

/*-------------------------------------------------
    sound_get_type - return the type of the
    specified sound chip
-------------------------------------------------*/

INLINE sound_type sound_get_type(const device_config *device)
{
	const sound_config *config = device->inline_config;
	return config->type;
}


#endif	/* __SOUND_H__ */
