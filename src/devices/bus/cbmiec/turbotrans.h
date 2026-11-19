// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Rossmöller Turbo Trans cartridge emulation

**********************************************************************/

#ifndef MAME_BUS_CBMIEC_TURBOTRANS_H
#define MAME_BUS_CBMIEC_TURBOTRANS_H

#pragma once

#include "c1541.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> c1541_turbotrans_device

class c1541_turbotrans_device :  public c1541_device_base,
		public device_c64_floppy_parallel_interface
{
public:
	// construction/destruction
	c1541_turbotrans_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	DECLARE_INPUT_CHANGED_MEMBER( nmi_w );

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

	// device_c64_floppy_parallel_interface implementation
	virtual void parallel_data_w(u8 data) override;
	virtual void parallel_strobe_w(int state) override;

private:
	required_ioport m_config;
	memory_view m_ext;
	memory_bank_creator m_dos;
	memory_share_creator<u8> m_sram;
	std::unique_ptr<u8[]> m_dram;

	bool m_dram_enable;
	u8 m_counter;
	u8 m_dram_banks;

	void c1541tt_mem(address_map &map) ATTR_COLD;

	template <int Bank> u8 dram_r(offs_t offset);
	template <int Bank> void dram_w(offs_t offset, u8 data);

	u8 via0_pa_r();
	void via0_pa_w(u8 data);
	void via0_ca2_w(int state);
};


// device type definition
DECLARE_DEVICE_TYPE(C1541_TURBOTRANS, c1541_turbotrans_device)


#endif // MAME_BUS_CBMIEC_TURBOTRANS_H
