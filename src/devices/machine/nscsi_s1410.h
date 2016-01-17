// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
#ifndef __NSCSI_S1410_H__
#define __NSCSI_S1410_H__

#include "machine/nscsi_bus.h"
#include "machine/nscsi_hd.h"

class nscsi_s1410_device : public nscsi_harddisk_device
{
public:
	nscsi_s1410_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

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

	// SCSI commands
	enum {
		SC_TEST_UNIT_READY      = 0x00,
		SC_REZERO               = 0x01,
		SC_REQUEST_SENSE        = 0x03,
		SC_FORMAT_UNIT          = 0x04,
		SC_CHECK_TRACK_FORMAT   = 0x05,
		SC_FORMAT_TRACK         = 0x06,
		SC_REASSIGN_BLOCKS      = 0x07,
		SC_READ                 = 0x08,
		SC_WRITE                = 0x0a,
		SC_SEEK                 = 0x0b,
		SC_INIT_DRIVE_PARAMS    = 0x0c,
		SC_READ_ECC_BURST       = 0x0d,
		SC_FORMAT_ALT_TRACK     = 0x0e,
		SC_WRITE_SECTOR_BUFFER  = 0x0f,
		SC_READ_SECTOR_BUFFER   = 0x10,
		SC_RAM_DIAG             = 0xe0,
		SC_DRIVE_DIAG           = 0xe3,
		SC_CONTROLLER_DIAG      = 0xe4,
		SC_READ_LONG            = 0xe5,
		SC_WRITE_LONG           = 0xe6
	};

	virtual void device_reset() override;

	virtual void scsi_command() override;
	virtual UINT8 scsi_get_data(int id, int pos) override;
	virtual void scsi_put_data(int buf, int offset, UINT8 data) override;

	UINT8 params[8];
};

extern const device_type NSCSI_S1410;

#endif
