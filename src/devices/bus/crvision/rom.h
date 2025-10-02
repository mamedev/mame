// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
#ifndef MAME_BUS_CRVISION_ROM_H
#define MAME_BUS_CRVISION_ROM_H

#pragma once

#include "slot.h"


// ======================> crvision_rom_device

class crvision_rom_device : public device_t,
						public device_crvision_cart_interface
{
public:
	// construction/destruction
	crvision_rom_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// reading and writing
	virtual uint8_t read_rom80(offs_t offset) override;

protected:
	crvision_rom_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override {}
	virtual void device_reset() override {}
};

// ======================> crvision_rom6k_device

class crvision_rom6k_device : public crvision_rom_device
{
public:
	// construction/destruction
	crvision_rom6k_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// reading and writing
	virtual uint8_t read_rom80(offs_t offset) override;
};

// ======================> crvision_rom8k_device

class crvision_rom8k_device : public crvision_rom_device
{
public:
	// construction/destruction
	crvision_rom8k_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// reading and writing
	virtual uint8_t read_rom80(offs_t offset) override;
};

// ======================> crvision_rom10k_device

class crvision_rom10k_device : public crvision_rom_device
{
public:
	// construction/destruction
	crvision_rom10k_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// reading and writing
	virtual uint8_t read_rom40(offs_t offset) override;
	virtual uint8_t read_rom80(offs_t offset) override;
};

// ======================> crvision_rom12k_device

class crvision_rom12k_device : public crvision_rom_device
{
public:
	// construction/destruction
	crvision_rom12k_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// reading and writing
	virtual uint8_t read_rom40(offs_t offset) override;
	virtual uint8_t read_rom80(offs_t offset) override;
};

// ======================> crvision_rom16k_device

class crvision_rom16k_device : public crvision_rom_device
{
public:
	// construction/destruction
	crvision_rom16k_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// reading and writing
	virtual uint8_t read_rom80(offs_t offset) override;
};

// ======================> crvision_rom18k_device

class crvision_rom18k_device : public crvision_rom_device
{
public:
	// construction/destruction
	crvision_rom18k_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// reading and writing
	virtual uint8_t read_rom40(offs_t offset) override;
	virtual uint8_t read_rom80(offs_t offset) override;
};


// device type definition
DECLARE_DEVICE_TYPE(CRVISION_ROM_4K,  crvision_rom_device)
DECLARE_DEVICE_TYPE(CRVISION_ROM_6K,  crvision_rom6k_device)
DECLARE_DEVICE_TYPE(CRVISION_ROM_8K,  crvision_rom8k_device)
DECLARE_DEVICE_TYPE(CRVISION_ROM_10K, crvision_rom10k_device)
DECLARE_DEVICE_TYPE(CRVISION_ROM_12K, crvision_rom12k_device)
DECLARE_DEVICE_TYPE(CRVISION_ROM_16K, crvision_rom16k_device)
DECLARE_DEVICE_TYPE(CRVISION_ROM_18K, crvision_rom18k_device)

#endif // MAME_BUS_CRVISION_ROM_H
