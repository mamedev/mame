/*********************************************************************

    harddriv.h

    MESS interface to the MAME CHD code

*********************************************************************/

#ifndef MESS_HD_H
#define MESS_HD_H

#include "image.h"
#include "harddisk.h"

DECLARE_LEGACY_IMAGE_DEVICE(HARDDISK, mess_hd);
DECLARE_LEGACY_IMAGE_DEVICE(IDE_HARDDISK, mess_ide);

#define MCFG_HARDDISK_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, HARDDISK, 0) \

#define MCFG_IDE_HARDDISK_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, IDE_HARDDISK, 0) \

hard_disk_file *mess_hd_get_hard_disk_file(device_t *device);
chd_file *mess_hd_get_chd_file(device_t *device);

struct harddisk_callback_config
{
	device_image_load_func		device_image_load;
	device_image_unload_func	device_image_unload;
};


#endif /* MESS_HD_H */
