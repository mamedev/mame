/***************************************************************************

    idectrl.h

    Generic (PC-style) IDE controller implementation.

    Copyright Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

***************************************************************************/

#pragma once

#ifndef __IDECTRL_H__
#define __IDECTRL_H__

#include "harddisk.h"


/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

typedef struct _ide_config ide_config;
struct _ide_config
{
	void 	(*interrupt)(const device_config *device, int state);
	const char *master;		/* name of master region (defaults to device tag) */
	const char *slave;		/* name of slave region (defaults to NULL) */
};



/***************************************************************************
    DEVICE CONFIGURATION MACROS
***************************************************************************/

#define MDRV_IDE_CONTROLLER_ADD(_tag, _callback) \
	MDRV_DEVICE_ADD(_tag, IDE_CONTROLLER) \
	MDRV_DEVICE_CONFIG_DATAPTR(ide_config, interrupt, _callback)

#define MDRV_IDE_CONTROLLER_REGIONS(_master, _slave) \
	MDRV_DEVICE_CONFIG_DATAPTR(ide_config, master, _master) \
	MDRV_DEVICE_CONFIG_DATAPTR(ide_config, master, _slave)

#define MDRV_IDE_CONTROLLER_REMOVE(_tag) \
	MDRV_DEVICE_REMOVE(_tag, IDE_CONTROLLER)



/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

UINT8 *ide_get_features(const device_config *device);

void ide_set_master_password(const device_config *device, const UINT8 *password);
void ide_set_user_password(const device_config *device, const UINT8 *password);

int ide_bus_r(const device_config *config, int select, int offset);
void ide_bus_w(const device_config *config, int select, int offset, int data);

int ide_controller_r(const device_config *config, int reg);
void ide_controller_w(const device_config *config, int reg, int data);

READ32_DEVICE_HANDLER( ide_controller32_r );
WRITE32_DEVICE_HANDLER( ide_controller32_w );
READ32_DEVICE_HANDLER( ide_bus_master32_r );
WRITE32_DEVICE_HANDLER( ide_bus_master32_w );

READ16_DEVICE_HANDLER( ide_controller16_r );
WRITE16_DEVICE_HANDLER( ide_controller16_w );


/* ----- device interface ----- */

/* device get info callback */
#define IDE_CONTROLLER DEVICE_GET_INFO_NAME(ide_controller)
DEVICE_GET_INFO( ide_controller );


#endif	/* __IDECTRL_H__ */
