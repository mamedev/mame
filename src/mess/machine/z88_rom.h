#pragma once

#ifndef __Z88_ROM_H__
#define __Z88_ROM_H__

#include "emu.h"
#include "machine/z88cart.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> z88_32k_rom_device

class z88_32k_rom_device : public device_t,
							public device_z88cart_interface
{
public:
	// construction/destruction
	z88_32k_rom_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	z88_32k_rom_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock);

protected:
	// device-level overrides
	virtual void device_start();
	virtual void device_config_complete() { m_shortname = "z88_32k_rom"; }

	// z88cart_interface overrides
	virtual DECLARE_READ8_MEMBER(read);
	virtual UINT8* get_cart_base();
	virtual UINT32 get_cart_size() { return 0x8000; }

protected:
	// internal state
	UINT8 *     m_rom;
};

// ======================> z88_128k_rom_device

class z88_128k_rom_device : public z88_32k_rom_device
{
public:
	// construction/destruction
	z88_128k_rom_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

protected:
	// device-level overrides
	virtual void device_config_complete() { m_shortname = "z88_128k_rom"; }

	// z88cart_interface overrides
	virtual UINT32 get_cart_size() { return 0x20000; }
};

// ======================> z88_256k_rom_device

class z88_256k_rom_device : public z88_32k_rom_device
{
public:
	// construction/destruction
	z88_256k_rom_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

protected:
	// device-level overrides
	virtual void device_config_complete() { m_shortname = "z88_256k_rom"; }

	// z88cart_interface overrides
	virtual UINT32 get_cart_size() { return 0x200000; }
};

// device type definition
extern const device_type Z88_32K_ROM;
extern const device_type Z88_128K_ROM;
extern const device_type Z88_256K_ROM;

#endif  /* __Z88_ROM_H__ */
