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
	virtual DECLARE_READ32_MEMBER(read_ext_dram0) override;
	virtual DECLARE_READ32_MEMBER(read_ext_dram1) override;
	virtual DECLARE_WRITE32_MEMBER(write_ext_dram0) override;
	virtual DECLARE_WRITE32_MEMBER(write_ext_dram1) override;

protected:
	// construction/destruction
	saturn_dram_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, int cart_type);

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
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
