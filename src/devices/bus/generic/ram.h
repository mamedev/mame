// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
#ifndef __GENERIC_RAM_H
#define __GENERIC_RAM_H

#include "slot.h"


// ======================> generic_ram_plain_device

class generic_ram_plain_device : public device_t,
							public device_generic_cart_interface
{
public:
	// construction/destruction
	generic_ram_plain_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, uint32_t clock, uint32_t size, const char *shortname, const char *source);

	// device-level overrides
	virtual void device_start() override;

	// reading and writing
	virtual uint8_t read_ram(address_space &space, offs_t offset, uint8_t mem_mask = 0xff) override;
	virtual void write_ram(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff) override;

private:
	uint32_t m_size;
};


// ======================> generic_ram_linear_device

class generic_ram_linear_device : public device_t,
							public device_generic_cart_interface
{
public:
	// construction/destruction
	generic_ram_linear_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, uint32_t clock, uint32_t size, const char *shortname, const char *source);

	// device-level overrides
	virtual void device_start() override;

	// reading and writing
	virtual uint8_t read_ram(address_space &space, offs_t offset, uint8_t mem_mask = 0xff) override;
	virtual void write_ram(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff) override;

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
extern const device_type GENERIC_RAM_32K_PLAIN;
extern const device_type GENERIC_RAM_64K_PLAIN;
extern const device_type GENERIC_RAM_128K_PLAIN;
extern const device_type GENERIC_RAM_32K_LINEAR;
extern const device_type GENERIC_RAM_64K_LINEAR;
extern const device_type GENERIC_RAM_128K_LINEAR;


#endif
