// license:BSD-3-Clause
// copyright-holders:Angelo Salese
/***************************************************************************

  SH7604 BUS Controller

***************************************************************************/

#pragma once

#ifndef __SH7604_BUSDEV_H__
#define __SH7604_BUSDEV_H__



//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_SH7604_BUS_ADD(_tag,_freq) \
	MCFG_DEVICE_ADD(_tag, SH7604_BUS, _freq)

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> sh7604_bus_device

class sh7604_bus_device : public device_t,
						  public device_memory_interface
{
public:
	// construction/destruction
	sh7604_bus_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// I/O operations
	void write(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
	uint32_t read(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
	uint16_t bus_control_1_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void bus_control_1_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t bus_control_2_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void bus_control_2_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t wait_control_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void wait_control_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t memory_control_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void memory_control_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t refresh_timer_status_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void refresh_timer_control_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t refresh_timer_counter_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void refresh_timer_counter_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t refresh_timer_constant_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void refresh_timer_constant_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	virtual const address_space_config *memory_space_config(address_spacenum spacenum = AS_0) const override;

protected:
	// device-level overrides
	//virtual void device_validity_check(validity_checker &valid) const;
	virtual void device_start() override;
	virtual void device_reset() override;

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
extern const device_type SH7604_BUS;



//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************



#endif
