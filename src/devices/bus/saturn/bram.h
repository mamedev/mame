// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
#ifndef MAME_BUS_SATURN_BRAM_H
#define MAME_BUS_SATURN_BRAM_H

#include "sat_slot.h"


// ======================> saturn_bram_device

class saturn_bram_device : public device_t,
							public device_sat_cart_interface,
							public device_nvram_interface
{
public:
	// reading and writing
	virtual uint32_t read_ext_bram(offs_t offset) override;
	virtual void write_ext_bram(offs_t offset, uint32_t data, uint32_t mem_mask = ~0) override;

protected:
	// construction/destruction
	saturn_bram_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, int cart_type);

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// device_nvram_interface overrides
	virtual void nvram_default() override;
	virtual bool nvram_read(util::read_stream &file) override;
	virtual bool nvram_write(util::write_stream &file) override;
	virtual bool nvram_can_write() override;
};

class saturn_bram4mb_device : public saturn_bram_device
{
public:
	// construction/destruction
	saturn_bram4mb_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};

class saturn_bram8mb_device : public saturn_bram_device
{
public:
	// construction/destruction
	saturn_bram8mb_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};

class saturn_bram16mb_device : public saturn_bram_device
{
public:
	// construction/destruction
	saturn_bram16mb_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};

class saturn_bram32mb_device : public saturn_bram_device
{
public:
	// construction/destruction
	saturn_bram32mb_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};



// device type definition
DECLARE_DEVICE_TYPE(SATURN_BRAM_4MB,  saturn_bram4mb_device)
DECLARE_DEVICE_TYPE(SATURN_BRAM_8MB,  saturn_bram8mb_device)
DECLARE_DEVICE_TYPE(SATURN_BRAM_16MB, saturn_bram16mb_device)
DECLARE_DEVICE_TYPE(SATURN_BRAM_32MB, saturn_bram32mb_device)

#endif // MAME_BUS_SATURN_BRAM_H
