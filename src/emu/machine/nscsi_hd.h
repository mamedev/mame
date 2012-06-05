#ifndef __NSCSI_HD_H__
#define __NSCSI_HD_H__

#include "machine/nscsi_bus.h"
#include "harddisk.h"

class nscsi_harddisk_device : public nscsi_full_device
{
public:
	nscsi_harddisk_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	virtual machine_config_constructor device_mconfig_additions() const;

	static struct harddisk_interface hd_intf;
protected:
	virtual void device_start();
	virtual void device_reset();
	virtual void device_config_complete() { m_shortname = "scsi_harddisk"; }

	virtual void scsi_command();
	virtual UINT8 scsi_get_data(int id, int pos);
	virtual void scsi_put_data(int buf, int offset, UINT8 data);

private:
	UINT8 block[512];
	hard_disk_file *harddisk;
	int lba, cur_lba, blocks;
	int bytes_per_sector;
};

extern const device_type NSCSI_HARDDISK;

#endif
