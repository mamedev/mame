// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
#ifndef MAME_MACHINE_NSCSI_CD_H
#define MAME_MACHINE_NSCSI_CD_H

#pragma once

#include "machine/nscsi_bus.h"
#include "cdrom.h"

class nscsi_cdrom_device : public nscsi_full_device
{
public:
	nscsi_cdrom_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_add_mconfig(machine_config &config) override;

	virtual void scsi_command() override;
	virtual uint8_t scsi_get_data(int id, int pos) override;

private:
	uint8_t block[2048];
	cdrom_file *cdrom;
	int bytes_per_sector;
	int lba, cur_lba, blocks;

	void return_no_cd();
};

DECLARE_DEVICE_TYPE(NSCSI_CDROM, nscsi_cdrom_device)

#endif // MAME_MACHINE_NSCSI_CD_H
