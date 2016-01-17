// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
#ifndef __NSCSI_HD_H__
#define __NSCSI_HD_H__

#include "machine/nscsi_bus.h"
#include "harddisk.h"

class nscsi_harddisk_device : public nscsi_full_device
{
public:
	nscsi_harddisk_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);
	nscsi_harddisk_device(const machine_config &mconfig, device_type type, std::string name, std::string tag, device_t *owner, UINT32 clock, std::string shortname, std::string source);

	virtual machine_config_constructor device_mconfig_additions() const override;

protected:
	virtual void device_start() override;
	virtual void device_reset() override;

	virtual void scsi_command() override;
	virtual UINT8 scsi_get_data(int id, int pos) override;
	virtual void scsi_put_data(int buf, int offset, UINT8 data) override;

	UINT8 block[512];
	hard_disk_file *harddisk;
	int lba, cur_lba, blocks;
	int bytes_per_sector;
};

extern const device_type NSCSI_HARDDISK;

#endif
