// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
#ifndef MAME_BUS_SATURN_DRAM_H
#define MAME_BUS_SATURN_DRAM_H

#include "sat_slot.h"


// ======================> saturn_dram_device

class saturn_dram_device : public device_t,
							public device_sat_cart_interface
{
public:
	// reading and writing
	virtual uint32_t read_ext_dram0(offs_t offset) override;
	virtual uint32_t read_ext_dram1(offs_t offset) override;
	virtual void write_ext_dram0(offs_t offset, uint32_t data, uint32_t mem_mask = ~0) override;
	virtual void write_ext_dram1(offs_t offset, uint32_t data, uint32_t mem_mask = ~0) override;

protected:
	// construction/destruction
	saturn_dram_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, int cart_type);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
};

class saturn_dram8mb_device : public saturn_dram_device
{
public:
	// construction/destruction
	saturn_dram8mb_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};

class saturn_dram32mb_device : public saturn_dram_device
{
public:
	// construction/destruction
	saturn_dram32mb_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};



// device type definition
DECLARE_DEVICE_TYPE(SATURN_DRAM_8MB,  saturn_dram8mb_device)
DECLARE_DEVICE_TYPE(SATURN_DRAM_32MB, saturn_dram32mb_device)

#endif // MAME_BUS_SATURN_DRAM_H
