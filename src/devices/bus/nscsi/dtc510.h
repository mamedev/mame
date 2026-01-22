// license:BSD-3-Clause
// copyright-holders:Angelo Salese
#ifndef MAME_BUS_NSCSI_DTC510_H
#define MAME_BUS_NSCSI_DTC510_H

#pragma once

#include "machine/nscsi_bus.h"
#include "bus/nscsi/hd.h"

class nscsi_dtc510_device : public nscsi_harddisk_device
{
public:
	nscsi_dtc510_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// SCSI status returns
	enum {
		SS_GOOD                 = 0x00,
		SS_NO_INDEX             = 0x01,
		SS_NO_SEEK_COMPLETE     = 0x02,
		SS_WRITE_FAULT          = 0x03,
		SS_NOT_READY            = 0x04,
		SS_TK00_NOT_FOUND       = 0x06,
		SS_SEEK_IN_PROGRESS     = 0x08,
		SS_ID_FIELD_ERROR       = 0x10,
		SS_DATA_ERROR           = 0x11,
		SS_SAM_NOT_FOUND        = 0x12,
		SS_SECTOR_NOT_FOUND     = 0x14,
		SS_SEEK_ERROR           = 0x15,
		SS_ECC                  = 0x18,
		SS_BAD_TRACK            = 0x19,
		SS_FORMAT_ERROR         = 0x1a,
		SS_ALT_TRACK            = 0x1c,
		SS_ALT_TRACK_DEFECT     = 0x1d,
		SS_ALT_TRACK_NOT_FOUND  = 0x1e,
		SS_ALT_TRACK_SAME       = 0x1f,
		SS_RAM_ERROR            = 0x30,
		SS_ROM_ERROR            = 0x31,
		SS_ECC_CHECK_FAILURE    = 0x32
	};

	// SCSI commands (page 2-6/22 of the OMTI manual)
	// NOTE: some of these are for the tape version
	enum {
		// group 0
		SC_TEST_UNIT_READY      = 0x00,
		SC_RECALIBRATE          = 0x01,
		SC_REWIND               = 0x01, // tape
		SC_RETENTION            = 0x02,
		SC_REQUEST_SENSE        = 0x03,
		SC_FORMAT_UNIT          = 0x04,
		SC_CHECK_TRACK_FORMAT   = 0x05,
		SC_FORMAT_TRACK         = 0x06,
		SC_FORMAT_BAD_TRACK     = 0x07,
		SC_READ                 = 0x08,
		SC_WRITE                = 0x0a,
		SC_SEEK                 = 0x0b,
		SC_ASSIGN_ALT_TRACK     = 0x0e,
		SC_WRITE_FILE_MARK      = 0x10,
		SC_SPACE_FORWARD        = 0x11,
		SC_VERIFY               = 0x13,
		SC_ERASE                = 0x19,
		SC_CHANGE_CARTRIDGE     = 0x1b,
		// group 1
		SC_COPY                 = 0x20,
		SC_BACKUP               = 0x22,
		SC_RESTORE              = 0x23,
		SC_BACKUP_WITH_HEADER   = 0x24,
		SC_RESTORE_CONTINUE     = 0x26,
		// group 2 (tape only)
		SC_RETENTION_TAPE       = 0x44,
		SC_VERIFY_TAPE          = 0x45,
		SC_READ_SENSE           = 0x46,
		SC_WRITE_HEADER         = 0x48,
		SC_READ_HEADER          = 0x49,
		SC_WRITE_BLOCKS         = 0x4a,
		SC_READ_BLOCKS          = 0x4b,
		// group 6
		SC_DEFINE_FLEXIBLE_DISK_FORMAT = 0xc0,
		SC_ASSIGN_DISK_PARAMETERS = 0xc2,
		// group 7
		SC_RAM_DIAGNOSTICS      = 0xe0,
		SC_WRITE_ECC            = 0xe1,
		SC_READ_ID              = 0xe2,
		SC_READ_DATA_BUFFER     = 0xec,
		SC_WRITE_DATA_BUFFER    = 0xef
	};

	// SCSI sense keys
	// TODO: pickup defs
	enum {
		SK_NO_ERROR             = 0x00,
		SK_DRIVE_NOT_READY      = 0x04,
		SK_FORMAT_ERROR         = 0x1a
	};

	virtual void device_reset() override ATTR_COLD;

	virtual bool scsi_command_done(uint8_t command, uint8_t length) override;
	virtual void scsi_command() override;
	virtual uint8_t scsi_get_data(int id, int pos) override;
	virtual void scsi_put_data(int buf, int offset, uint8_t data) override;
	virtual attotime scsi_data_byte_period() override;
	virtual attotime scsi_data_command_delay() override;

	uint8_t m_param[10];
	//u32 m_seek;
};

DECLARE_DEVICE_TYPE(NSCSI_DTC510, nscsi_dtc510_device)

#endif // MAME_BUS_NSCSI_DTC510_H
