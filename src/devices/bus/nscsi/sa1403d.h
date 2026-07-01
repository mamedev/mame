// license:BSD-3-Clause
// copyright-holders:Dave Rand
/*********************************************************************

    Shugart SA1403D Winchester/Floppy Disk Controller

    Microprogrammed SASI controller serving up to four drives, any
    combination of SA1000-family rigid disks and SA800/SA850 (and,
    with later OEM firmware, 5.25") floppy drives.  Per-LUN drive
    types are selected by the drive-type DIP switch at board
    location 2H (PROM set AS30 semantics); machines may instead fix
    a LUN's type from machine configuration (set_drive_type), which
    models an OEM PROM set with its own switch interpretation.

    Reference: Shugart 39022-0 SA1403D OEM Manual, December 1980.

    Rigid-disk LUNs are served from CHD hard-disk images (the host
    only ever addresses flat logical sectors; physical geometry
    exists solely for the controller's bounds checks).  Floppy LUNs
    are served through an on-board floppy section driven at the
    flux level (an internal wd_fdc as the data-separator/serdes
    stand-in), so reads AND writes take the real media path.

*********************************************************************/

#ifndef MAME_BUS_NSCSI_SA1403D_H
#define MAME_BUS_NSCSI_SA1403D_H

#pragma once

#include "machine/nscsi_hle.h"
#include "machine/wd_fdc.h"
#include "imagedev/floppy.h"
#include "imagedev/harddriv.h"

#include <vector>

class nscsi_sa1403d_device : public nscsi_full_device
{
public:
	static constexpr flags_type emulation_flags() { return flags::SAVE_UNSUPPORTED; }

	nscsi_sa1403d_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	// drive types, in the AS30 DIP encoding (0-3); 4+ are later-firmware
	// extensions reachable only through set_drive_type()
	enum drive_type : uint8_t {
		SA1002 = 0,     // 8" rigid, 2 heads, 256 cylinders
		SA1004 = 1,     // 8" rigid, 4 heads, 256 cylinders
		SA800  = 2,     // 8" floppy, 1 head, 77 cylinders
		SA850  = 3,     // 8" floppy, 2 heads, 77 cylinders
		SA400  = 4,     // 5.25" floppy, 1 head, 35 cylinders
		SA450  = 5,     // 5.25" floppy, 2 heads, 40 cylinders
		TYPE_FROM_DIP = 0xff
	};

	// machine-config override of the drive-type DIP for one LUN
	void set_drive_type(int lun, drive_type t) { if (lun >= 0 && lun < 4) m_cfg_type[lun] = t; }

protected:
	// for machine-local subclasses preconfiguring a drive complement (the slot
	// option machine-config hook runs too late to affect subdevice creation)
	nscsi_sa1403d_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_reset_after_children() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

	virtual void scsi_command() override;
	virtual bool scsi_command_done(uint8_t command, uint8_t length) override;
	virtual uint8_t scsi_get_data(int id, int pos) override;
	virtual void scsi_put_data(int id, int pos, uint8_t data) override;

	virtual attotime scsi_data_byte_period() override;

private:
	// SASI error codes (request-sense block), per OEM manual 5.4
	enum : uint8_t {
		// type 0 - drive
		ERR_NONE             = 0x00,
		ERR_NO_INDEX         = 0x01,
		ERR_NO_SEEK_COMPLETE = 0x02,
		ERR_WRITE_FAULT      = 0x03,
		ERR_NOT_READY        = 0x04,
		ERR_NOT_SELECTED     = 0x05,
		ERR_NO_TRACK00       = 0x06,
		// type 1 - controller
		ERR_ID_READ          = 0x10,
		ERR_DATA_UNCORR      = 0x11,
		ERR_ID_AM_NOT_FOUND  = 0x12,
		ERR_DATA_AM_NOT_FOUND= 0x13,
		ERR_RECORD_NOT_FOUND = 0x14,
		ERR_SEEK_ERROR       = 0x15,
		ERR_DMA_TIMEOUT      = 0x16,
		ERR_WRITE_PROTECTED  = 0x17,
		ERR_DATA_CORRECTABLE = 0x18,
		ERR_BAD_TRACK        = 0x19,
		ERR_FORMAT_ERROR     = 0x1a,
		// type 2 - command
		ERR_INVALID_COMMAND  = 0x20,
		ERR_ILLEGAL_ADDRESS  = 0x21,
		ERR_ILLEGAL_FUNCTION = 0x22,
		// type 3 - misc
		ERR_RAM_ERROR        = 0x30
	};

	// floppy-section operation state
	enum class fop : uint8_t {
		IDLE,
		RESTORE,        // recalibrate to track 0
		SEEK,           // implied seek before a data command
		RW,             // read/write sectors through the buffer
		WTRACK          // write track (format)
	};

	struct drive_geom
	{
		const char *name;
		uint8_t heads;
		uint16_t cyls;
		uint8_t spt_fm;         // sectors/track, single-density format
		uint8_t spt_mfm;        // sectors/track, double-density format (rigid: 32/32)
		bool floppy;
	};
	static const drive_geom s_geom[6];

	// per-LUN media subdevices: only the ones matching the configured drive
	// types are instantiated (a LUN left on TYPE_FROM_DIP gets both, since the
	// DIP is only readable at runtime).  A machine that fixes one rigid LUN
	// thus exposes a single -hard image, matching the real single-drive units.
	optional_device_array<harddisk_image_device, 4> m_hd;
	optional_device_array<floppy_connector, 4> m_fd;
	required_device<fd1793_device> m_fdc;
	required_ioport m_dip;

	uint8_t m_cfg_type[4];      // TYPE_FROM_DIP or a fixed drive_type
	uint8_t m_lun_type[4];      // resolved drive type per LUN (at reset)
	uint8_t m_format_code[4];   // class-6 Define Floppy Track Format, per LUN
	uint8_t m_lun_cyl[4];       // per-LUN head position (0xff = unknown -> restore)

	uint32_t m_lba;             // current command's logical address
	uint32_t m_blocks;          // current command's block count
	int m_xfer_lun;             // LUN whose media serves the data phase
	uint32_t m_cur_lba;         // block currently in the sector buffer (rigid)
	uint8_t m_block[256];       // the on-board sector buffer (rigid path)

	// floppy-section state
	std::vector<uint8_t> m_xferbuf; // whole-transfer buffer (floppy path)
	fop m_fop;
	bool m_f_write;
	int m_f_lun;
	uint32_t m_f_lba;
	uint32_t m_f_left;          // blocks (RW) or tracks (WTRACK) remaining
	uint32_t m_f_pos;           // byte position in m_xferbuf
	uint32_t m_f_secstart;      // m_xferbuf position at the current sector's start
	int m_f_retry;
	bool m_f_noretry;
	uint8_t m_f_cmd;            // pending command opcode (dispatch in fdc handlers)
	uint8_t m_f_interleave;     // format interleave code from the CDB

	// helpers, shared
	const drive_geom &geom(int lun) const { return s_geom[m_lun_type[lun]]; }
	bool lun_is_floppy(int lun) const { return geom(lun).floppy; }
	bool lun_ready(int lun);
	uint32_t lun_max_lba(int lun) const;
	void sasi_status(int lun, bool error);
	void sasi_good(int lun);
	void sasi_error(int lun, uint8_t code, bool addr_valid = false, uint32_t lba = 0);

	// helpers, floppy section
	floppy_image_device *cur_fd() const { return m_fd[m_f_lun]->get_device(); }
	void f_decode(int lun, uint32_t lba, int &cyl, int &head, int &sec, int &size, bool &fm) const;
	uint32_t f_xfer_bytes(int lun, uint32_t lba, uint32_t blocks) const;
	void f_select(int cyl, int head, bool fm);
	void f_start(uint8_t cmd, int lun, uint32_t lba, uint32_t blocks, bool write);
	void f_issue_seek(int cyl);
	void f_issue_rw();
	void f_issue_wtrack();
	void f_compose_track(int cyl, int head, bool fm, bool bad);
	void f_finish(uint8_t err);
	void fdc_intrq_w(int state);
	void fdc_drq_w(int state);
};

DECLARE_DEVICE_TYPE(NSCSI_SA1403D, nscsi_sa1403d_device)

#endif // MAME_BUS_NSCSI_SA1403D_H
