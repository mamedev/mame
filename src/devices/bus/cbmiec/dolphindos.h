// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Evesham Micros Dolphin-DOS

**********************************************************************/

#ifndef MAME_BUS_CBMIEC_DOLPHINDOS_H
#define MAME_BUS_CBMIEC_DOLPHINDOS_H

#pragma once

#include "c1541.h"
#include "c1571.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> c1541_dolphin_dos_v2_device

class c1541_dolphin_dos_v2_device : public c1541_device_base,
					       	  	    public device_c64_floppy_parallel_interface
{
public:
	// construction/destruction
	c1541_dolphin_dos_v2_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	// optional information overrides
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	// device_c64_floppy_parallel_interface overrides
	virtual void parallel_data_w(u8 data) override;
	virtual void parallel_strobe_w(int state) override;

private:
	uint8_t via0_pa_r();
	void via0_pa_w(uint8_t data);
	void via0_ca2_w(int state);

	void c1541dd_mem(address_map &map) ATTR_COLD;
};


// ======================> c1571_dolphin_dos_v3_device

class c1571_dolphin_dos_v3_device : public c1571_device,
								 	public device_c64_floppy_parallel_interface
{
public:
	// construction/destruction
	c1571_dolphin_dos_v3_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	// optional information overrides
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	// device_c64_floppy_parallel_interface overrides
	virtual void parallel_data_w(u8 data) override;
	virtual void parallel_strobe_w(int state) override;

private:
	uint8_t cia_pb_r();
	void cia_pb_w(uint8_t data);
	void cia_pc_w(int state);
};


// device type definition
DECLARE_DEVICE_TYPE(C1541_DOLPHIN_DOS_V2, c1541_dolphin_dos_v2_device)
DECLARE_DEVICE_TYPE(C1571_DOLPHIN_DOS_V3, c1571_dolphin_dos_v3_device)


#endif // MAME_BUS_CBMIEC_DOLPHINDOS_H
