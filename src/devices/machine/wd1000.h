// license:BSD-3-Clause
// copyright-holders:68bit
/***************************************************************************

    Western Digital WD1000-05 Winchester Disk Controller

***************************************************************************/

#ifndef MAME_MACHINE_WD1000_H
#define MAME_MACHINE_WD1000_H

#pragma once

#include "imagedev/harddriv.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> wd1000_device

class wd1000_device : public device_t
{
public:
	// construction/destruction
	wd1000_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	auto intrq_wr_callback() { return m_intrq_cb.bind(); }
	auto drq_wr_callback() { return m_drq_cb.bind(); }

	uint8_t data_r();
	void data_w(uint8_t val);
	void set_sector_base(uint32_t base);

	uint8_t read(offs_t offset);
	void write(offs_t offset, uint8_t data);

	// declared but not defined?
	int intrq_r();
	int drq_r();


protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	TIMER_CALLBACK_MEMBER(update_seek);
	TIMER_CALLBACK_MEMBER(delayed_drq);

private:
	enum
	{
		S_ERR = 0x01, // Error
		S_DRQ = 0x08, // Data request
		S_SC  = 0x10, // Seek complete
		S_WF  = 0x20, // Write fault
		S_RDY = 0x40, // Drive ready
		S_BSY = 0x80  // Busy
	};

	enum
	{
		ERR_DM   = 0x01, // data address mark not found
		ERR_TK   = 0x02, // track zero error
		ERR_AC   = 0x04, // aborted command
		ERR_ID   = 0x10, // id not found
		ERR_CRC  = 0x40, // crc error
		ERR_BB   = 0x80  // bad block
	};

	enum
	{
		CMD_RESTORE = 1,
		CMD_READ_SECTOR = 2,
		CMD_WRITE_SECTOR = 3,
		CMD_WRITE_FORMAT = 5,
		CMD_SEEK = 7
	};

	void set_error(int error);
	void set_intrq(int state);
	void set_drq();
	void drop_drq();
	attotime get_stepping_rate();
	void start_command();
	void end_command();
	int get_lbasector();

	int head() { return (m_sdh >> 0) & 0x07; }
	int drive() { return (m_sdh >> 3) & 0x03; }
	int sector_bytes()
	{
		const int bytes[4] = { 256, 512, 0, 128 };
		return bytes[(m_sdh >> 5) & 0x03];
	}

	void cmd_restore();
	void cmd_read_sector();
	void cmd_write_sector();
	void cmd_format_sector();
	void cmd_seek();

	devcb_write_line m_intrq_cb, m_drq_cb;

	optional_device_array<harddisk_image_device, 4> m_drives;
	uint16_t m_drive_cylinder[4];

	uint16_t m_sector_base;

	// Data buffer
	uint8_t m_buffer[512];
	uint16_t m_buffer_index;
	uint16_t m_buffer_end;

	emu_timer *m_seek_timer;
	emu_timer *m_drq_timer;

	int m_intrq;
	int m_drq;
	uint8_t m_stepping_rate;
	uint8_t m_command;

	uint8_t m_error;
	uint8_t m_precomp;
	uint8_t m_sector_count;
	uint8_t m_sector_number;
	uint16_t m_cylinder;
	uint8_t m_sdh;
	uint8_t m_status;
};

// device type definition
DECLARE_DEVICE_TYPE(WD1000, wd1000_device)

#endif // MAME_MACHINE_WD1000_H
