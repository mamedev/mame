// license:BSD-3-Clause
// copyright-holders:Sandro Ronco
#pragma once

#ifndef __Z88_RAM_H__
#define __Z88_RAM_H__

#include "emu.h"
#include "z88.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> z88_32k_ram_device

class z88_32k_ram_device : public device_t,
							public device_z88cart_interface
{
public:
	// construction/destruction
	z88_32k_ram_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	z88_32k_ram_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source);

protected:
	// device-level overrides
	virtual void device_start() override;

	// z88cart_interface overrides
	virtual DECLARE_READ8_MEMBER(read) override;
	virtual DECLARE_WRITE8_MEMBER(write) override;
	virtual UINT8* get_cart_base() override;
	virtual UINT32 get_cart_size() override { return 0x8000; }

protected:
	// internal state
	UINT8 *     m_ram;
};

// ======================> z88_128k_ram_device

class z88_128k_ram_device : public z88_32k_ram_device
{
public:
	// construction/destruction
	z88_128k_ram_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

protected:
	// z88cart_interface overrides
	virtual UINT32 get_cart_size() override { return 0x20000; }
};

// ======================> z88_512k_ram_device

class z88_512k_ram_device : public z88_32k_ram_device
{
public:
	// construction/destruction
	z88_512k_ram_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

protected:
	// z88cart_interface overrides
	virtual UINT32 get_cart_size() override { return 0x80000; }
};

// ======================> z88_1024k_ram_device

class z88_1024k_ram_device : public z88_32k_ram_device
{
public:
	// construction/destruction
	z88_1024k_ram_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

protected:
	// z88cart_interface overrides
	virtual UINT32 get_cart_size() override { return 0x100000; }
};

// device type definition
extern const device_type Z88_32K_RAM;
extern const device_type Z88_128K_RAM;
extern const device_type Z88_512K_RAM;
extern const device_type Z88_1024K_RAM;

#endif  /* __Z88_RAM_H__ */
