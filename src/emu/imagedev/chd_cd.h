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
	device_image_display_info_func	device_displayinfo;
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

#define MCFG_CDROM_DISPLAY_INFO(_displayinfo)										\
	MCFG_DEVICE_CONFIG_DATAPTR(cdrom_config, device_displayinfo, DEVICE_IMAGE_DISPLAY_INFO_NAME(_displayinfo))

#endif /* CHD_CD_H */
