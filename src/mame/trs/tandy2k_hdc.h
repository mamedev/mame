// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Tandy 2000 hard disk controller emulation

    The 2000HD Winchester option is a WD1010 (u18) plus a one-sector
    buffer managed by a WD1100-11 (u12) and its static RAM.  The host
    moves a sector through the buffer at register 0 and drives the
    WD1010 task file at registers 1-7 (error/precomp, sector count,
    sector number, cylinder low, cylinder high, SDH, status/command).
    Register 0 belongs to the board, not to the chip -- the WD1010's own
    register 0 is unused.

    Transfers are programmed I/O: the Tandy MS-DOS 2.11 driver polls the
    status register and moves 512 bytes a byte at a time, so INTRQ and
    the board's DMA acknowledge window are not exercised by any known
    software.

*********************************************************************/

#ifndef MAME_TRS_TANDY2K_HDC_H
#define MAME_TRS_TANDY2K_HDC_H

#pragma once

#include "imagedev/harddriv.h"
#include "machine/wd1010.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> tandy2k_hdc_device

class tandy2k_hdc_device : public device_t
{
public:
	// construction/destruction
	tandy2k_hdc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	auto intrq_wr_callback() { return m_write_intrq.bind(); }

	// host interface: 0 = board sector buffer, 1-7 = WD1010 task file
	uint8_t read(offs_t offset);
	void write(offs_t offset, uint8_t data);

	// controller master reset, pulsed by reading the port
	uint8_t reset_r();
	void reset_w(uint8_t data);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

private:
	static constexpr unsigned BUFFER_SIZE = 0x200;

	uint8_t buf_in();               // controller drains the buffer (write to media)
	void buf_out(uint8_t data);     // controller fills the buffer (read from media)

	void intrq_w(int state);
	void bcr_w(int state);          // buffer counter reset

	void mr_w();                    // master reset

	required_device<wd1010_device> m_hdc;            // u18
	required_device<harddisk_image_device> m_hdd;

	devcb_write_line m_write_intrq;

	uint8_t m_buf[BUFFER_SIZE];     // u12 (WD1100-11) and its static RAM
	uint16_t m_ptr;
};


// device type definition
DECLARE_DEVICE_TYPE(TANDY2K_HDC, tandy2k_hdc_device)

#endif // MAME_TRS_TANDY2K_HDC_H
