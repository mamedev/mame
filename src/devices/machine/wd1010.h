// license:GPL-2.0+
// copyright-holders:Dirk Best
/***************************************************************************

    Western Digital WD1010-05 Winchester Disk Controller

***************************************************************************/

#ifndef MAME_MACHINE_WD1010_H
#define MAME_MACHINE_WD1010_H

#pragma once

#include "imagedev/harddriv.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> wd1010_device

class wd1010_device : public device_t
{
public:
	// construction/destruction
	wd1010_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	auto out_intrq_callback() { return m_out_intrq_cb.bind(); }
	auto out_bcr_callback() { return m_out_bcr_cb.bind(); }
	auto in_data_callback() { return m_in_data_cb.bind(); }
	auto out_data_callback() { return m_out_data_cb.bind(); }

	DECLARE_READ8_MEMBER(read);
	DECLARE_WRITE8_MEMBER(write);

	DECLARE_WRITE_LINE_MEMBER(drdy_w);
	DECLARE_WRITE_LINE_MEMBER(brdy_w);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

private:
	enum
	{
		STATUS_ERR = 0x01, // error
		STATUS_CIP = 0x02, // command in progress
		STATUS_RSV = 0x04, // reserved
		STATUS_DRQ = 0x08, // data request
		STATUS_SC  = 0x10, // seek complete
		STATUS_WF  = 0x20, // write fault
		STATUS_RDY = 0x40, // drive ready
		STATUS_BSY = 0x80  // controller busy
	};

	enum
	{
		ERR_DM   = 0x01, // data address mark not found
		ERR_TK   = 0x02, // track zero error
		ERR_AC   = 0x04, // aborted command
		ERR_RSV1 = 0x08, // reserved, forced to 0
		ERR_ID   = 0x10, // id not found
		ERR_RSV2 = 0x20, // reserved, forced to 0
		ERR_CRC  = 0x40, // crc error
		ERR_BB   = 0x80  // bad block
	};

	enum
	{
		CMD_RESTORE = 1,
		CMD_READ_SECTOR = 2,
		CMD_WRITE_SECTOR = 3,
		CMD_SCAN_ID = 4,
		CMD_WRITE_FORMAT = 5,
		CMD_SEEK = 7
	};

	enum
	{
		TIMER_SEEK,
		TIMER_DATA
	};

	void set_error(int error);
	void set_intrq(int state);
	attotime get_stepping_rate();
	void start_command();
	void end_command();
	int get_lbasector();

	// extract values from sdh
	int head() { return (m_sdh >> 0) & 0x07; }
	int drive() { return (m_sdh >> 3) & 0x03; }
	int sector_size()
	{
		const int S[4] = { 256, 512, 1024, 128 };
		return S[(m_sdh >> 5) & 0x03];
	}

	void cmd_restore();
	void cmd_read_sector();
	void cmd_write_sector();
	void cmd_scan_id();
	void cmd_seek();

	devcb_write_line m_out_intrq_cb;
	devcb_write_line m_out_bcr_cb;
	devcb_read8 m_in_data_cb;
	devcb_write8 m_out_data_cb;

	struct
	{
		harddisk_image_device *drive;
		uint8_t head;
		uint16_t cylinder;
		uint8_t sector;
	} m_drives[4];

	emu_timer *m_seek_timer;
	emu_timer *m_data_timer;

	int m_intrq;
	int m_brdy;
	uint8_t m_stepping_rate;
	uint8_t  m_command;

	// task file registers
	uint8_t m_error;
	uint8_t m_precomp;
	uint8_t m_sector_count;
	uint8_t m_sector_number;
	uint16_t m_cylinder;
	uint8_t m_sdh;
	uint8_t m_status;
};

// device type definition
DECLARE_DEVICE_TYPE(WD1010, wd1010_device)

#endif // MAME_MACHINE_WD1010_H
