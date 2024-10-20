// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    COMX-35 Disk Controller Card emulation

**********************************************************************/

#ifndef MAME_BUS_COMX35_FD_H
#define MAME_BUS_COMX35_FD_H

#pragma once

#include "exp.h"
#include "imagedev/floppy.h"
#include "machine/wd_fdc.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> comx_fd_device

class comx_fd_device : public device_t,
						public device_comx_expansion_card_interface
{
public:
	// construction/destruction
	comx_fd_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// optional information overrides
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	// device_comx_expansion_card_interface overrides
	virtual int comx_ef4_r() override;
	virtual void comx_q_w(int state) override;
	virtual uint8_t comx_mrd_r(offs_t offset, int *extrom) override;
	virtual uint8_t comx_io_r(offs_t offset) override;
	virtual void comx_io_w(offs_t offset, uint8_t data) override;

private:
	static void floppy_formats(format_registration &fr);

	// internal state
	required_device<wd1770_device> m_fdc;
	required_device<floppy_connector> m_floppy0;
	required_device<floppy_connector> m_floppy1;
	required_memory_region m_rom;

	// floppy state
	int m_q;                // FDC register select
	int m_addr;             // FDC address
	int m_disb;             // data request disable
};


// device type definition
DECLARE_DEVICE_TYPE(COMX_FD, comx_fd_device)


#endif // MAME_BUS_COMX35_FD_H
