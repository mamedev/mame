// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Shugart SA1403D Winchester/Floppy Disk Controller emulation

    The 820-II uses the floppy personality of this controller: it
    auto-detects the inserted media format and serves logical sectors
    over SASI. (Stage-2 WIP: floppy read path.)

**********************************************************************/

#ifndef MAME_BUS_SCSI_SA1403D_H
#define MAME_BUS_SCSI_SA1403D_H

#pragma once

#include "scsihd.h"
#include "imagedev/harddriv.h"
#include "imagedev/floppy.h"

#include <vector>

class sa1403d_device  : public scsihd_device
{
public:
	// construction/destruction
	sa1403d_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// driver wires the floppy drive(s) this controller serves, one per SASI LUN
	// (LUNs with no floppy fall through to the scsihd hard-disk image, e.g. SA1004).
	void set_floppy(int lun, floppy_image_device *f) { if (unsigned(lun) < 4) m_floppy[lun] = f; }

	virtual void ExecCommand() override;
	virtual void ReadData( uint8_t *data, int dataLength ) override;
	virtual void WriteData( uint8_t *data, int dataLength ) override;
	virtual bool exists() const override;

protected:
	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

private:
	// per-track decoded sector cache (built on demand from the floppy image)
	struct track_cache
	{
		bool valid = false;
		bool is_mfm = false;
		int sector_size = 0;
		std::vector<uint8_t> first_sector_id; // sector IDs present, in physical order
		std::vector<std::vector<uint8_t>> by_id; // index = sector id, value = data
	};

	floppy_image_device *m_floppy[4] = { nullptr, nullptr, nullptr, nullptr };
	int m_lun = 0;            // LUN of the command being executed
	int m_spt = 17;           // detected physical sectors/track of the selected drive
	bool m_sel_dd = true;     // density selected by the last C0 Define-Floppy (true=DD/MFM)
	bool m_sel_ds = false;    // sides selected by the last C0 Define-Floppy (true=double-sided)

	floppy_image_device *cur_floppy() const { return (unsigned(m_lun) < 4) ? m_floppy[m_lun] : nullptr; }
	bool decode_track(int cyl, int head, track_cache &tc);
	bool read_logical_block(uint32_t lba, std::vector<uint8_t> &out);
};


// device type definition
DECLARE_DEVICE_TYPE(SA1403D, sa1403d_device)

#endif // MAME_BUS_SCSI_SA1403D_H
