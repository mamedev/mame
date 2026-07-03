// license:BSD-3-Clause
// copyright-holders:Dave Rand
/***************************************************************************

    Western Digital WD1002-HD0 Winchester Disk Controller board

    A WD1010 Winchester controller behind the board's host interface: a
    512-byte on-board sector buffer (the WD1015 buffer manager + static RAM,
    holding one sector of 128/256/512 bytes as the mounted drive dictates) and
    the WD1010 task file.  The host moves a sector through the buffer at
    the data port (register 0, INIR/OTIR) and drives the WD1010 task file at
    registers 1-7 (error/precomp, sector count, sector number, cyl low, cyl
    high, SDH, status/command).

    The WD1015 (a masked Toshiba TMP8049 micro) front-end is high-level
    emulated -- its mask ROM is undumped -- as buffer management plus a thin
    command filter over the real WD1010; everything below it is the genuine
    WD1010.

    This is the host-direct front-end shared with the WD1002-05 (which adds a
    WD2797 floppy section); the -HD0 is the same board without the floppy
    interface.  Used by the Kaypro 10/1084, the Xerox 16/8 EM-II, the Tandy
    2000, and other WD1002 task-file machines.

***************************************************************************/

#ifndef MAME_MACHINE_WD1002_HD0_H
#define MAME_MACHINE_WD1002_HD0_H

#pragma once

#include "machine/wd1010.h"
#include "imagedev/harddriv.h"

#include <memory>


class wd1002_hd0_device : public device_t
{
public:
	wd1002_hd0_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	auto intrq_callback() { return m_intrq_cb.bind(); } // WD1010 INTRQ, forwarded

	// Host interface: register 0 = the board sector buffer; 1-7 = the WD1010 task
	// file.  virtual so the WD1002-05 subclass can intercept the floppy path while
	// sharing this rigid/buffer implementation.
	virtual uint8_t read(offs_t offset);
	virtual void write(offs_t offset, uint8_t data);

protected:
	wd1002_hd0_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock); // for the WD1002-05 subclass

	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	static constexpr unsigned BUFFER_SIZE = 0x200; // 512-byte board sector buffer (TMM2016 SRAM)

	// shared sector buffer (the WD1002-05 floppy section fills/drains it the same way)
	uint8_t buf_in();             // controller -> host buffer (drained on a write-to-media)
	void buf_out(uint8_t data);   // controller -> host buffer (filled on a read-from-media)

	std::unique_ptr<uint8_t[]> m_buf;
	uint16_t m_ptr;

private:
	void intrq_w(int state);      // WD1010 INTRQ, forwarded to the host callback
	void bcr_w(int state);        // WD1010 buffer-counter reset: rewind the pointer

	required_device<wd1010_device> m_hdc;
	required_device<harddisk_image_device> m_hdd; // the fixed drive (its CHD may or may not be mounted)
	devcb_write_line m_intrq_cb;

	uint16_t m_sector_bytes; // sector size taken from the mounted drive (128/256/512)
};

DECLARE_DEVICE_TYPE(WD1002_HD0, wd1002_hd0_device)

#endif // MAME_MACHINE_WD1002_HD0_H
