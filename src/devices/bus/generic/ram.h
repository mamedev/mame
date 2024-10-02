// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
#ifndef MAME_BUS_GENERIC_RAM_H
#define MAME_BUS_GENERIC_RAM_H

#pragma once

#include "slot.h"


// ======================> generic_ram_plain_device

class generic_ram_plain_device : public device_t, public device_generic_cart_interface
{
public:
	// reading and writing
	virtual uint8_t read_ram(offs_t offset) override;
	virtual void write_ram(offs_t offset, uint8_t data) override;

protected:
	// construction/destruction
	generic_ram_plain_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, uint32_t size);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;

private:
	uint32_t m_size;
};


// ======================> generic_ram_linear_device

class generic_ram_linear_device : public device_t, public device_generic_cart_interface
{
public:
	// reading and writing
	virtual uint8_t read_ram(offs_t offset) override;
	virtual void write_ram(offs_t offset, uint8_t data) override;

protected:
	// construction/destruction
	generic_ram_linear_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, uint32_t size);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;

private:
	uint32_t m_size;
};


// ======================> generic_ram_*k_plain_device

class generic_ram_32k_plain_device : public generic_ram_plain_device
{
public:
	// construction/destruction
	generic_ram_32k_plain_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};

class generic_ram_64k_plain_device : public generic_ram_plain_device
{
public:
	// construction/destruction
	generic_ram_64k_plain_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};

class generic_ram_128k_plain_device : public generic_ram_plain_device
{
public:
	// construction/destruction
	generic_ram_128k_plain_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};


// ======================> generic_ram_*k_linear_device

class generic_ram_32k_linear_device : public generic_ram_linear_device
{
public:
	// construction/destruction
	generic_ram_32k_linear_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};

class generic_ram_64k_linear_device : public generic_ram_linear_device
{
public:
	// construction/destruction
	generic_ram_64k_linear_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};

class generic_ram_128k_linear_device : public generic_ram_linear_device
{
public:
	// construction/destruction
	generic_ram_128k_linear_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};



// device type definition
DECLARE_DEVICE_TYPE(GENERIC_RAM_32K_PLAIN,   generic_ram_32k_plain_device)
DECLARE_DEVICE_TYPE(GENERIC_RAM_64K_PLAIN,   generic_ram_64k_plain_device)
DECLARE_DEVICE_TYPE(GENERIC_RAM_128K_PLAIN,  generic_ram_128k_plain_device)
DECLARE_DEVICE_TYPE(GENERIC_RAM_32K_LINEAR,  generic_ram_32k_linear_device)
DECLARE_DEVICE_TYPE(GENERIC_RAM_64K_LINEAR,  generic_ram_64k_linear_device)
DECLARE_DEVICE_TYPE(GENERIC_RAM_128K_LINEAR, generic_ram_128k_linear_device)


#endif // MAME_BUS_GENERIC_RAM_H
