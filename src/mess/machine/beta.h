/*********************************************************************

    beta.h

    Implementation of Beta disk drive support for Spectrum and clones

    04/05/2008 Created by Miodrag Milanovic

*********************************************************************/
#ifndef __BETA_H__
#define __BETA_H__


int betadisk_is_active(device_t *device);
void betadisk_enable(device_t *device);
void betadisk_disable(device_t *device);
void betadisk_clear_status(device_t *device);

#define BETA_DISK_TAG	"beta"

DECLARE_LEGACY_DEVICE(BETA_DISK, beta_disk);

#define MCFG_BETA_DISK_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, BETA_DISK, 0)

#define MCFG_BETA_DISK_REMOVE(_tag)		\
    MCFG_DEVICE_REMOVE(_tag)

READ8_DEVICE_HANDLER(betadisk_status_r);
READ8_DEVICE_HANDLER(betadisk_track_r);
READ8_DEVICE_HANDLER(betadisk_sector_r);
READ8_DEVICE_HANDLER(betadisk_data_r);
READ8_DEVICE_HANDLER(betadisk_state_r);

WRITE8_DEVICE_HANDLER(betadisk_param_w);
WRITE8_DEVICE_HANDLER(betadisk_command_w);
WRITE8_DEVICE_HANDLER(betadisk_track_w);
WRITE8_DEVICE_HANDLER(betadisk_sector_w);
WRITE8_DEVICE_HANDLER(betadisk_data_w);
#endif /* __BETA_H__ */
