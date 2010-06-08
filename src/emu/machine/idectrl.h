/***************************************************************************

    idectrl.h

    Generic (PC-style) IDE controller implementation.

    Copyright Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

***************************************************************************/

#pragma once

#ifndef __IDECTRL_H__
#define __IDECTRL_H__

#include "devlegcy.h"

#include "harddisk.h"


/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

typedef struct _ide_config ide_config;
struct _ide_config
{
	void	(*interrupt)(running_device *device, int state);
	const char *master;		/* name of master region (defaults to device tag) */
	const char *slave;		/* name of slave region (defaults to NULL) */
	const char *bmcpu;		/* name of bus master CPU */
	UINT32 bmspace;			/* address space of bus master transfer */
};



/***************************************************************************
    DEVICE CONFIGURATION MACROS
***************************************************************************/

#define MDRV_IDE_CONTROLLER_ADD(_tag, _callback) \
	MDRV_DEVICE_ADD(_tag, IDE_CONTROLLER, 0) \
	MDRV_DEVICE_CONFIG_DATAPTR(ide_config, interrupt, _callback)

#define MDRV_IDE_CONTROLLER_REGIONS(_master, _slave) \
	MDRV_DEVICE_CONFIG_DATAPTR(ide_config, master, _master) \
	MDRV_DEVICE_CONFIG_DATAPTR(ide_config, master, _slave)

#define MDRV_IDE_BUS_MASTER_SPACE(_cpu, _space) \
	MDRV_DEVICE_CONFIG_DATAPTR(ide_config, bmcpu, _cpu) \
	MDRV_DEVICE_CONFIG_DATA32(ide_config, bmspace, ADDRESS_SPACE_##_space)



/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

UINT8 *ide_get_features(running_device *device);

void ide_set_master_password(running_device *device, const UINT8 *password);
void ide_set_user_password(running_device *device, const UINT8 *password);

void ide_set_gnet_readlock(running_device *device, const UINT8 onoff);

int ide_bus_r(running_device *config, int select, int offset);
void ide_bus_w(running_device *config, int select, int offset, int data);

UINT32 ide_controller_r(running_device *config, int reg, int size);
void ide_controller_w(running_device *config, int reg, int size, UINT32 data);

READ32_DEVICE_HANDLER( ide_controller32_r );
WRITE32_DEVICE_HANDLER( ide_controller32_w );
READ32_DEVICE_HANDLER( ide_controller32_pcmcia_r );
WRITE32_DEVICE_HANDLER( ide_controller32_pcmcia_w );
READ32_DEVICE_HANDLER( ide_bus_master32_r );
WRITE32_DEVICE_HANDLER( ide_bus_master32_w );

READ16_DEVICE_HANDLER( ide_controller16_r );
WRITE16_DEVICE_HANDLER( ide_controller16_w );


/* ----- device interface ----- */

DECLARE_LEGACY_DEVICE(IDE_CONTROLLER, ide_controller);


#endif	/* __IDECTRL_H__ */
