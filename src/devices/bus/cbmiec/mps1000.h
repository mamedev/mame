// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Commodore MPS-1000 Dot Matrix Printer emulation

**********************************************************************/

#ifndef MAME_BUS_CBMIEC_MPS1000_H
#define MAME_BUS_CBMIEC_MPS1000_H

#pragma once

#include "cbmiec.h"

#include "cpu/upd7810/upd7810.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> mps1000_device

class mps1000_device : public device_t, public device_cbm_iec_interface
{
public:
	static constexpr flags_type emulation_flags() { return flags::NOT_WORKING; }

	// construction/destruction
	mps1000_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

private:
	required_device<upd7810_device> m_maincpu;

	void mps1000_mem(address_map &map) ATTR_COLD;
};


// device type definition
DECLARE_DEVICE_TYPE(MPS1000, mps1000_device)


#endif // MAME_BUS_CBMIEC_MPS1000_H
