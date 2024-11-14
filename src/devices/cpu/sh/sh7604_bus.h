// license:BSD-3-Clause
// copyright-holders:Angelo Salese
/***************************************************************************

  SH7604 BUS Controller

***************************************************************************/

#ifndef MAME_CPU_SH_SH7604_BUS_H
#define MAME_CPU_SH_SH7604_BUS_H

#pragma once



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> sh7604_bus_device

class sh7604_bus_device : public device_t
{
public:
	// construction/destruction
	sh7604_bus_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// I/O operations
	void bus_regs(address_map &map) ATTR_COLD;

	void write(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t read(address_space &space, offs_t offset);
	uint16_t bus_control_1_r();
	void bus_control_1_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t bus_control_2_r();
	void bus_control_2_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t wait_control_r();
	void wait_control_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t memory_control_r();
	void memory_control_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t refresh_timer_status_r();
	void refresh_timer_control_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t refresh_timer_counter_r();
	void refresh_timer_counter_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t refresh_timer_constant_r();
	void refresh_timer_constant_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

protected:
	// device-level overrides
	//virtual void device_validity_check(validity_checker &valid) const;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	bool m_is_slave;
	const address_space_config      m_space_config;

	uint16_t m_bcr1;
	uint16_t m_bcr2;
	uint16_t m_wcr;
	uint16_t m_mcr;
	uint16_t m_rtcsr;
	uint16_t m_rtcor;
};


// device type definition
DECLARE_DEVICE_TYPE(SH7604_BUS, sh7604_bus_device)

#endif // MAME_CPU_SH_SH7604_BUS_H
