// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    VTS Data Professional-DOS

**********************************************************************/

#ifndef MAME_BUS_CBMIEC_PRODOS_H
#define MAME_BUS_CBMIEC_PRODOS_H

#pragma once

#include "c1541.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> c1541_professional_dos_v1_device

class c1541_professional_dos_v1_device : public c1541_device_base,
		public device_c64_floppy_parallel_interface
{
public:
	// construction/destruction
	c1541_professional_dos_v1_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	// device_c64_floppy_parallel_interface implementation
	virtual void parallel_data_w(u8 data) override;
	virtual void parallel_strobe_w(int state) override;

private:
	void c1541pd_mem(address_map &map) ATTR_COLD;

	u8 decoder_r(offs_t offset);
	u8 via0_pa_r();
	void via0_pa_w(u8 data);
	void via0_ca2_w(int state);

	required_region_ptr<u8> m_rom;

	u8 m_latch;
};


// device type definition
DECLARE_DEVICE_TYPE(C1541_PROFESSIONAL_DOS_V1, c1541_professional_dos_v1_device)


#endif // MAME_BUS_CBMIEC_PRODOS_H
