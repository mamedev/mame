// license:BSD-3-Clause
// copyright-holders:Dave Rand
/***************************************************************************

    Western Digital WD1002-05 Winchester/Floppy Disk Controller board

    The WD1002-HD0 plus the floppy section: a WD2797 floppy controller behind
    the same host task file.  The host issues a single Winchester command
    format; the on-board WD1015 routes it to the WD2797 when the SDH register
    selects a floppy drive, otherwise to the WD1010 (rigid).  The board's
    sector buffer is shared by both data paths.

    Used by the Xerox 16/8 EM-II (5.25" rigid + floppy).

***************************************************************************/

#ifndef MAME_MACHINE_WD1002_05_H
#define MAME_MACHINE_WD1002_05_H

#pragma once

#include "machine/wd1002_hd0.h"
#include "machine/wd_fdc.h"
#include "imagedev/floppy.h"


class wd1002_05_device : public wd1002_hd0_device
{
public:
	wd1002_05_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	virtual uint8_t read(offs_t offset) override;
	virtual void write(offs_t offset, uint8_t data) override;

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

private:
	// the floppy section runs the WD2797 at the flux level; a read/write is deferred
	// and completes via the FDC callback chain (the task-file BSY bit holds the host off)
	enum : uint8_t { FOP_IDLE, FOP_RESTORE, FOP_SEEK, FOP_RW }; // floppy operation phase
	void f_select(int drive, int head);
	void f_start(bool write);
	void f_issue_seek();
	void f_issue_rw();
	void fdc_intrq_w(int state);
	void fdc_drq_w(int state);

	required_device<wd2797_device> m_fdc;
	required_device_array<floppy_connector, 2> m_floppy; // two 5.25" drives (A: and B:)

	// task-file shadow needed for the floppy path (the base keeps the rigid copy)
	uint8_t m_secno = 0, m_cyllo = 0, m_cylhi = 0, m_sdh = 0;

	uint8_t  m_fop = FOP_IDLE;
	bool     m_f_write = false;
	bool     m_f_wr_pending = false; // a floppy write command is buffering host data
	int      m_f_cyl = 0, m_f_head = 0, m_f_sec = 0, m_f_drive = 0;
	uint8_t  m_floppy_cyl[2] = { 0xff, 0xff }; // per-drive head position (0xff = unknown -> restore)
	uint8_t  m_f_err = 0;            // floppy error register (WD1015 front-end status)
};

DECLARE_DEVICE_TYPE(WD1002_05, wd1002_05_device)

#endif // MAME_MACHINE_WD1002_05_H
