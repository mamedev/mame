// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
#ifndef MAME_MACHINE_NSCSI_CD_H
#define MAME_MACHINE_NSCSI_CD_H

#pragma once

#include "machine/nscsi_bus.h"
#include "imagedev/chd_cd.h"
#include "cdrom.h"

class nscsi_cdrom_device : public nscsi_full_device
{
public:
	nscsi_cdrom_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void set_block_size(u32 block_size);

protected:
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_add_mconfig(machine_config &config) override;

	virtual void scsi_command() override;
	virtual uint8_t scsi_get_data(int id, int pos) override;
	virtual void scsi_put_data(int buf, int offset, uint8_t data) override;

private:
	static constexpr uint32_t bytes_per_sector = 2048;

	uint8_t sector_buffer[bytes_per_sector];
	cdrom_file *cdrom;
	uint32_t bytes_per_block;
	int lba, cur_sector;
	required_device<cdrom_image_device> image;
	uint8_t mode_data[12];

	void return_no_cd();
};

DECLARE_DEVICE_TYPE(NSCSI_CDROM, nscsi_cdrom_device)

#endif // MAME_MACHINE_NSCSI_CD_H
