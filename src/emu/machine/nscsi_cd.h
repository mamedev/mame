// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
#ifndef __NSCSI_CD_H__
#define __NSCSI_CD_H__

#include "machine/nscsi_bus.h"
#include "cdrom.h"

class nscsi_cdrom_device : public nscsi_full_device
{
public:
	nscsi_cdrom_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	virtual machine_config_constructor device_mconfig_additions() const;

protected:
	virtual void device_start();
	virtual void device_reset();

	virtual void scsi_command();
	virtual UINT8 scsi_get_data(int id, int pos);

private:
	UINT8 block[2048];
	cdrom_file *cdrom;
	int bytes_per_sector;
	int lba, cur_lba, blocks;

	void return_no_cd();
};

extern const device_type NSCSI_CDROM;

#endif
