// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    SpeedDOS-compatible 1541 parallel DOS emulation

**********************************************************************/

#ifndef MAME_BUS_CBMIEC_SPEEDDOS_H
#define MAME_BUS_CBMIEC_SPEEDDOS_H

#pragma once

#include "c1541.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> c1541_speeddos_device

class c1541_speeddos_device : public c1541_device_base,
		public device_c64_floppy_parallel_interface
{
public:
	// construction/destruction
	c1541_speeddos_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	// device_c64_floppy_parallel_interface implementation
	virtual void parallel_data_w(u8 data) override;
	virtual void parallel_strobe_w(int state) override;

private:
	u8 via0_pa_r();
	void via0_pa_w(u8 data);
	void via0_ca2_w(int state);
};


// device type definition
DECLARE_DEVICE_TYPE(C1541_SPEEDDOS, c1541_speeddos_device)


#endif // MAME_BUS_CBMIEC_SPEEDDOS_H
