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
	generic_ram_plain_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, UINT32 size, const char *shortname, const char *source);

	// device-level overrides
	virtual void device_start();

	// reading and writing
	virtual DECLARE_READ8_MEMBER(read_ram);
	virtual DECLARE_WRITE8_MEMBER(write_ram);

private:
	UINT32 m_size;
};


// ======================> generic_ram_linear_device

class generic_ram_linear_device : public device_t,
							public device_generic_cart_interface
{
public:
	// construction/destruction
	generic_ram_linear_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, UINT32 size, const char *shortname, const char *source);

	// device-level overrides
	virtual void device_start();

	// reading and writing
	virtual DECLARE_READ8_MEMBER(read_ram);
	virtual DECLARE_WRITE8_MEMBER(write_ram);

private:
	UINT32 m_size;
};


// ======================> generic_ram_*k_plain_device

class generic_ram_32k_plain_device : public generic_ram_plain_device
{
public:
	// construction/destruction
	generic_ram_32k_plain_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};

class generic_ram_64k_plain_device : public generic_ram_plain_device
{
public:
	// construction/destruction
	generic_ram_64k_plain_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};

class generic_ram_128k_plain_device : public generic_ram_plain_device
{
public:
	// construction/destruction
	generic_ram_128k_plain_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};


// ======================> generic_ram_*k_linear_device

class generic_ram_32k_linear_device : public generic_ram_linear_device
{
public:
	// construction/destruction
	generic_ram_32k_linear_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};

class generic_ram_64k_linear_device : public generic_ram_linear_device
{
public:
	// construction/destruction
	generic_ram_64k_linear_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};

class generic_ram_128k_linear_device : public generic_ram_linear_device
{
public:
	// construction/destruction
	generic_ram_128k_linear_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};



// device type definition
extern const device_type GENERIC_RAM_32K_PLAIN;
extern const device_type GENERIC_RAM_64K_PLAIN;
extern const device_type GENERIC_RAM_128K_PLAIN;
extern const device_type GENERIC_RAM_32K_LINEAR;
extern const device_type GENERIC_RAM_64K_LINEAR;
extern const device_type GENERIC_RAM_128K_LINEAR;


#endif
