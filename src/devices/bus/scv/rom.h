// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
#ifndef MAME_BUS_SCV_ROM_H
#define MAME_BUS_SCV_ROM_H

#pragma once

#include "slot.h"


// ======================> scv_rom8_device

class scv_rom8_device : public device_t,
						public device_scv_cart_interface
{
public:
	// construction/destruction
	scv_rom8_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// reading and writing
	virtual uint8_t read_cart(offs_t offset) override;

protected:
	scv_rom8_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override { }
	virtual void device_reset() override { }
};

// ======================> scv_rom16_device

class scv_rom16_device : public scv_rom8_device
{
public:
	// construction/destruction
	scv_rom16_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// reading and writing
	virtual uint8_t read_cart(offs_t offset) override;
};


// ======================> scv_rom32_device

class scv_rom32_device : public scv_rom8_device
{
public:
	// construction/destruction
	scv_rom32_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// reading and writing
	virtual uint8_t read_cart(offs_t offset) override;
};


// ======================> scv_rom32ram8_device

class scv_rom32ram8_device : public scv_rom8_device
{
public:
	// construction/destruction
	scv_rom32ram8_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// reading and writing
	virtual uint8_t read_cart(offs_t offset) override;
	virtual void write_cart(offs_t offset, uint8_t data) override;
	virtual void write_bank(uint8_t data) override;

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	uint8_t m_ram_enabled;
};


// ======================> scv_rom64_device

class scv_rom64_device : public scv_rom8_device
{
public:
	// construction/destruction
	scv_rom64_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// reading and writing
	virtual uint8_t read_cart(offs_t offset) override;
	virtual void write_bank(uint8_t data) override;

private:
	uint8_t m_bank_base;
};


// ======================> scv_rom128_device

class scv_rom128_device : public scv_rom8_device
{
public:
	// construction/destruction
	scv_rom128_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// reading and writing
	virtual uint8_t read_cart(offs_t offset) override;
	virtual void write_bank(uint8_t data) override;

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	uint8_t m_bank_base;
};


// ======================> scv_rom128ram4_device

class scv_rom128ram4_device : public scv_rom8_device
{
public:
	// construction/destruction
	scv_rom128ram4_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// reading and writing
	virtual uint8_t read_cart(offs_t offset) override;
	virtual void write_cart(offs_t offset, uint8_t data) override;
	virtual void write_bank(uint8_t data) override;

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	uint8_t m_bank_base, m_ram_enabled;
};


// device type definition
DECLARE_DEVICE_TYPE(SCV_ROM8K,         scv_rom8_device)
DECLARE_DEVICE_TYPE(SCV_ROM16K,        scv_rom16_device)
DECLARE_DEVICE_TYPE(SCV_ROM32K,        scv_rom32_device)
DECLARE_DEVICE_TYPE(SCV_ROM32K_RAM8K,  scv_rom32ram8_device)
DECLARE_DEVICE_TYPE(SCV_ROM64K,        scv_rom64_device)
DECLARE_DEVICE_TYPE(SCV_ROM128K,       scv_rom128_device)
DECLARE_DEVICE_TYPE(SCV_ROM128K_RAM4K, scv_rom128ram4_device)

#endif // MAME_BUS_SCV_ROM_H
