/*********************************************************************

    chd_cd.h

    Interface to the CHD CDROM code

*********************************************************************/

#ifndef CHD_CD_H
#define CHD_CD_H

#include "cdrom.h"

/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

typedef struct cdrom_config_t	cdrom_config;
struct cdrom_config_t
{
	const char *					interface;
};

/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/
DECLARE_LEGACY_IMAGE_DEVICE(CDROM, cdrom);


cdrom_file *cd_get_cdrom_file(device_t *device);

/***************************************************************************
    DEVICE CONFIGURATION MACROS
***************************************************************************/


#define MCFG_CDROM_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, CDROM, 0) \

#define MCFG_CDROM_INTERFACE(_interface)							\
	MCFG_DEVICE_CONFIG_DATAPTR(cdrom_config, interface, _interface )

#endif /* CHD_CD_H */
