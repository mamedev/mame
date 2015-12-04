// license:BSD-3-Clause
// copyright-holders:smf
/***************************************************************************

    idehd.h

    IDE Harddisk

***************************************************************************/

#pragma once

#ifndef __IDEHD_H__
#define __IDEHD_H__

#include "atahle.h"
#include "harddisk.h"
#include "imagedev/harddriv.h"

class ata_mass_storage_device : public ata_hle_device
{
public:
	ata_mass_storage_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock,const char *shortname, const char *source);

	UINT16 *identify_device_buffer() { return m_identify_buffer; }

	void set_master_password(const UINT8 *password)
	{
		m_master_password = password;
		m_master_password_enable = (password != nullptr);
	}

	void set_user_password(const UINT8 *password)
	{
		m_user_password = password;
		m_user_password_enable = (password != nullptr);
	}

protected:
	virtual void device_start();

	virtual int read_sector(UINT32 lba, void *buffer) = 0;
	virtual int write_sector(UINT32 lba, const void *buffer) = 0;
	virtual attotime seek_time();

	void ide_build_identify_device();

	static const int IDE_DISK_SECTOR_SIZE = 512;
	virtual int sector_length() { return IDE_DISK_SECTOR_SIZE; }
	virtual void process_buffer();
	virtual void fill_buffer();
	virtual bool is_ready() { return true; }
	virtual void process_command();
	virtual void finished_command();
	virtual void perform_diagnostic();
	virtual void signature();

	int m_can_identify_device;
	UINT16          m_num_cylinders;
	UINT8           m_num_sectors;
	UINT8           m_num_heads;

	virtual UINT32 lba_address();

private:
	void set_geometry(UINT8 sectors, UINT8 heads) { m_num_sectors = sectors; m_num_heads = heads; }
	void finished_read();
	void finished_write();
	void next_sector();
	void security_error();
	void read_first_sector();
	void soft_reset();

	UINT32          m_cur_lba;
	UINT16          m_block_count;
	UINT16          m_sectors_until_int;

	UINT8           m_master_password_enable;
	UINT8           m_user_password_enable;
	const UINT8 *   m_master_password;
	const UINT8 *   m_user_password;
};

// ======================> ide_hdd_device

class ide_hdd_device : public ata_mass_storage_device
{
public:
	// construction/destruction
	ide_hdd_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	ide_hdd_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source);

protected:
	// device-level overrides
	virtual void device_start();
	virtual void device_reset();

	// optional information overrides
	virtual machine_config_constructor device_mconfig_additions() const;

	virtual int read_sector(UINT32 lba, void *buffer) { if (m_disk == nullptr) return 0; return hard_disk_read(m_disk, lba, buffer); }
	virtual int write_sector(UINT32 lba, const void *buffer) { if (m_disk == nullptr) return 0; return hard_disk_write(m_disk, lba, buffer); }
	virtual UINT8 calculate_status();

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
extern const device_type IDE_HARDDISK;

#endif
