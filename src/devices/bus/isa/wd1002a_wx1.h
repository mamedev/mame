// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Western Digital WD1002A-WX1 Winchester Disk Controller emulation

    Chip set: WD11C00-17-02 (PC/XT host interface), WD1015A-02 (buffer
    manager control processor), WD1010A-05 (Winchester disk controller/
    sequencer), WD10C20B (self-adjusting MFM data separator).  Supports
    two ST-506/412 (MFM) drives.

    The WD1015A is a masked 8049; its firmware is undumped (the 8000h
    ROM already on this board, "600693-001 type 5", is the host BIOS
    extension, not the WD1015A's own microcode -- compare the genuine
    WD1015 dump used by wdxt_gen.cpp, which uses the same WD11C00-17
    host chip and a real i8049).  So the WD1015A's command-block
    decode and buffer management are high-level emulated by the shared
    wd1015_device (machine/wd1015.h) in front of the genuine
    wd11c00_17_device (host bus interface) and wd1010_device (task
    file / seek / read-write sequencer), the same approach used for
    the WD1002-HD0/-05 boards (wd1002_hd0.cpp).

    Host protocol: the BIOS/driver polls the status register (port
    321h) then issues a 6-byte Command Descriptor Block through the
    data register (port 320h), following the WD1002/XT command set
    (also implemented by the Linux "xd" driver's WD support, and
    documented in the IBM Fixed Disk Adapter Technical Reference).
    There is no WD1015A ROM to verify this board's own firmware
    against, but wdxt_gen.cpp's WD1015 dump implements the identical
    published command set on the same WD11C00-17/WD1015 pairing, so
    the shared wd1015_device's CDB layouts, completion byte and sense
    codes are verified against that disassembly rather than being
    best-effort reconstructions.

**********************************************************************/

#ifndef MAME_BUS_ISA_WD1002A_WX1_H
#define MAME_BUS_ISA_WD1002A_WX1_H

#pragma once

#include "isa.h"
#include "machine/wd11c00_17.h"
#include "machine/wd1010.h"
#include "machine/wd1015.h"
#include "imagedev/harddriv.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> isa8_wd1002a_wx1_device

class isa8_wd1002a_wx1_device : public device_t,
								public device_isa8_card_interface
{
public:
	// construction/destruction
	isa8_wd1002a_wx1_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// optional information overrides
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	// device_isa8_card_interface overrides
	virtual uint8_t dack_r(int line) override;
	virtual void dack_w(int line, uint8_t data) override;
	virtual void dack_line_w(int line, int state) override;

private:
	void irq5_w(int state);
	void drq3_w(int state);
	uint8_t rd322_r();

	required_device<wd11c00_17_device> m_host;
	required_device<wd1010_device> m_hdc;
	required_device<wd1015_device> m_mcu;
};


// device type definition
DECLARE_DEVICE_TYPE(ISA8_WD1002A_WX1, isa8_wd1002a_wx1_device)

#endif // MAME_BUS_ISA_WD1002A_WX1_H
