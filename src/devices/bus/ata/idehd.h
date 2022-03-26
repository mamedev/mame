// license:BSD-3-Clause
// copyright-holders:smf
/***************************************************************************

    idehd.h

    IDE Harddisk

***************************************************************************/

#ifndef MAME_BUS_ATA_IDEHD_H
#define MAME_BUS_ATA_IDEHD_H

#pragma once

#include "atahle.h"
#include "harddisk.h"
#include "imagedev/harddriv.h"

class ata_mass_storage_device : public ata_hle_device
{
public:
	uint16_t *identify_device_buffer() { return m_identify_buffer; }

	void set_master_password(const uint8_t *password)
	{
		m_master_password = password;
		m_master_password_enable = (password != nullptr);
	}

	void set_user_password(const uint8_t *password)
	{
		m_user_password = password;
		m_user_password_enable = (password != nullptr);
	}

	void set_dma_transfer_time(const attotime time) { m_dma_transfer_time = time; }
protected:
	ata_mass_storage_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_start() override;

	virtual int read_sector(uint32_t lba, void *buffer) = 0;
	virtual int write_sector(uint32_t lba, const void *buffer) = 0;
	virtual attotime seek_time();

	void ide_build_identify_device();

	static const int IDE_DISK_SECTOR_SIZE = 512;
	virtual int sector_length() override { return IDE_DISK_SECTOR_SIZE; }
	virtual void process_buffer() override;
	virtual void fill_buffer() override;
	virtual bool is_ready() override { return true; }
	virtual void process_command() override;
	virtual void finished_command() override;
	virtual void perform_diagnostic() override;
	virtual void signature() override;

	int m_can_identify_device;
	uint16_t          m_num_cylinders;
	uint8_t           m_num_sectors;
	uint8_t           m_num_heads;

	virtual uint32_t lba_address();

private:
	void set_geometry(uint8_t sectors, uint8_t heads) { m_num_sectors = sectors; m_num_heads = heads; }
	void finished_read();
	void finished_write();
	void next_sector();
	void security_error();
	void read_first_sector();
	void soft_reset() override;

	uint32_t          m_cur_lba;
	uint16_t          m_block_count;
	uint16_t          m_sectors_until_int;

	uint8_t           m_master_password_enable;
	uint8_t           m_user_password_enable;
	const uint8_t *   m_master_password;
	const uint8_t *   m_user_password;
	// DMA data transfer time for 1 sector
	attotime          m_dma_transfer_time;
};

// ======================> ide_hdd_device

class ide_hdd_device : public ata_mass_storage_device
{
public:
	// construction/destruction
	ide_hdd_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	ide_hdd_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override;

	virtual int read_sector(uint32_t lba, void *buffer) override { return !m_disk ? 0 : hard_disk_read(m_disk, lba, buffer); }
	virtual int write_sector(uint32_t lba, const void *buffer) override { return !m_disk ? 0 : hard_disk_write(m_disk, lba, buffer); }
	virtual uint8_t calculate_status() override;

	chd_file       *m_handle;
	hard_disk_file *m_disk;

	enum
	{
		TID_NULL = TID_BUSY + 1
	};

private:
	required_device<harddisk_image_device> m_image;

	emu_timer *     m_last_status_timer;
};

// device type definition
DECLARE_DEVICE_TYPE(IDE_HARDDISK, ide_hdd_device)

#endif // MAME_BUS_ATA_IDEHD_H
