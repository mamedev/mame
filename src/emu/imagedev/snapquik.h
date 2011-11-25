/*********************************************************************

    snapquik.h

    Snapshots and quickloads

*********************************************************************/

#ifndef __SNAPQUIK_H__
#define __SNAPQUIK_H__

#include "image.h"

/***************************************************************************
    CONSTANTS
***************************************************************************/

enum
{
	DEVINFO_FCT_SNAPSHOT_QUICKLOAD_LOAD = DEVINFO_FCT_DEVICE_SPECIFIC
};



/***************************************************************************
    MACROS
***************************************************************************/

DECLARE_LEGACY_IMAGE_DEVICE(SNAPSHOT, snapshot);
DECLARE_LEGACY_IMAGE_DEVICE(QUICKLOAD, quickload);

#define SNAPSHOT_LOAD_NAME(name)	snapshot_load_##name
#define SNAPSHOT_LOAD(name)			int SNAPSHOT_LOAD_NAME(name)(device_image_interface &image, const char *file_type, int snapshot_size)

#define QUICKLOAD_LOAD_NAME(name)	quickload_load_##name
#define QUICKLOAD_LOAD(name)		int QUICKLOAD_LOAD_NAME(name)(device_image_interface &image, const char *file_type, int quickload_size)

/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

typedef int (*snapquick_load_func)(device_image_interface &image, const char *file_type, int file_size);

typedef struct _snapquick_config snapquick_config;
struct _snapquick_config
{
	snapquick_load_func	load;				/* loading function */
	const char *		file_extensions;	/* file extensions */
	seconds_t			delay_seconds;		/* loading delay (seconds) */
	attoseconds_t		delay_attoseconds;	/* loading delay (attoseconds) */
};

/***************************************************************************
    SNAPSHOT DEVICE CONFIGURATION MACROS
***************************************************************************/

#define MCFG_SNAPSHOT_ADD(_tag, _load, _file_extensions, _delay)	\
	MCFG_DEVICE_ADD(_tag, SNAPSHOT, 0) \
	MCFG_DEVICE_CONFIG_DATAPTR(snapquick_config, load, SNAPSHOT_LOAD_NAME(_load))	\
	MCFG_DEVICE_CONFIG_DATAPTR(snapquick_config, file_extensions, _file_extensions) \
	MCFG_DEVICE_CONFIG_DATA64(snapquick_config, delay_seconds, (seconds_t) (_delay)) \
	MCFG_DEVICE_CONFIG_DATA64(snapquick_config, delay_attoseconds, (attoseconds_t) (((_delay) - (int)(_delay)) * ATTOSECONDS_PER_SECOND)) \

/***************************************************************************
    QUICKLOAD DEVICE CONFIGURATION MACROS
***************************************************************************/

#define MCFG_QUICKLOAD_ADD(_tag, _load, _file_extensions, _delay)	\
	MCFG_DEVICE_ADD(_tag, QUICKLOAD, 0) \
	MCFG_DEVICE_CONFIG_DATAPTR(snapquick_config, load, QUICKLOAD_LOAD_NAME(_load))	\
	MCFG_DEVICE_CONFIG_DATAPTR(snapquick_config, file_extensions, _file_extensions) \
	MCFG_DEVICE_CONFIG_DATA64(snapquick_config, delay_seconds, (seconds_t) (_delay)) \
	MCFG_DEVICE_CONFIG_DATA64(snapquick_config, delay_attoseconds, (attoseconds_t) (((_delay) - (int)(_delay)) * ATTOSECONDS_PER_SECOND)) \

#endif /* __SNAPQUIK_H__ */
