// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
#ifndef MAME_BUS_NSCSI_HD_H
#define MAME_BUS_NSCSI_HD_H

#pragma once

#include "machine/nscsi_bus.h"
#include "imagedev/harddriv.h"

class nscsi_harddisk_device : public nscsi_full_device
{
public:
	nscsi_harddisk_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	nscsi_harddisk_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_add_mconfig(machine_config &config) override;

	virtual void scsi_command() override;
	virtual uint8_t scsi_get_data(int id, int pos) override;
	virtual void scsi_put_data(int buf, int offset, uint8_t data) override;

	required_device<harddisk_image_device> image;
	uint8_t block[512];
	hard_disk_file *harddisk;
	int lba, cur_lba, blocks;
	int bytes_per_sector;

	std::vector<u8> m_inquiry_data;
};

DECLARE_DEVICE_TYPE(NSCSI_HARDDISK, nscsi_harddisk_device)

#endif // MAME_BUS_NSCSI_HD_H
