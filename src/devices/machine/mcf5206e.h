// license:BSD-3-Clause
// copyright-holders:David Haywood, NaokiS
/***************************************************************************

    MCF5206E Peripherals

***************************************************************************/

#ifndef MAME_MACHINE_MCF5206E_H
#define MAME_MACHINE_MCF5206E_H

#pragma once

/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

// ======================> mcf5206e_peripheral_device

class mcf5206e_peripheral_device :  public device_t,
									public device_memory_interface
{
public:
	// construction/destruction
	template <typename T>
	mcf5206e_peripheral_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, T &&cpu_tag)
		: mcf5206e_peripheral_device(mconfig, tag, owner, clock)
	{
	}

	mcf5206e_peripheral_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	uint32_t dev_r(offs_t offset, uint32_t mem_mask = ~0);
	void dev_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t seta2_coldfire_regs_r(offs_t offset);
	void seta2_coldfire_regs_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);

protected:
	// device-level overrides
	virtual void device_start() override {}
	virtual void device_reset() override {}
	virtual space_config_vector memory_space_config() const override;

private:
	
	uint32_t m_coldfire_regs[0x400/4];

};


// device type definition
DECLARE_DEVICE_TYPE(MCF5206E_PERIPHERAL, mcf5206e_peripheral_device)

#endif // MAME_MACHINE_MCF5206E_H
