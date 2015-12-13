// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
#ifndef __GENERIC_ROM_H
#define __GENERIC_ROM_H

#include "slot.h"


// ======================> generic_rom_device

class generic_rom_device : public device_t,
						public device_generic_cart_interface
{
public:
	// construction/destruction
	generic_rom_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source);

	// device-level overrides
	virtual void device_start() override {}
};


// ======================> generic_rom_plain_device

class generic_rom_plain_device : public generic_rom_device
{
public:
	// construction/destruction
	generic_rom_plain_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source);
	generic_rom_plain_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// reading and writing
	virtual DECLARE_READ8_MEMBER(read_rom) override;
	virtual DECLARE_READ16_MEMBER(read16_rom) override;
	virtual DECLARE_READ32_MEMBER(read32_rom) override;
};


// ======================> generic_romram_plain_device

class generic_romram_plain_device : public generic_rom_plain_device
{
public:
	// construction/destruction
	generic_romram_plain_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// reading and writing
	virtual DECLARE_READ8_MEMBER(read_ram) override;
	virtual DECLARE_WRITE8_MEMBER(write_ram) override;
};


// ======================> generic_rom_linear_device

class generic_rom_linear_device : public generic_rom_device
{
public:
	// construction/destruction
	generic_rom_linear_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// reading and writing
	virtual DECLARE_READ8_MEMBER(read_rom) override;
	virtual DECLARE_READ16_MEMBER(read16_rom) override;
	virtual DECLARE_READ32_MEMBER(read32_rom) override;
};



// device type definition
extern const device_type GENERIC_ROM_PLAIN;
extern const device_type GENERIC_ROM_LINEAR;
extern const device_type GENERIC_ROMRAM_PLAIN;


#endif
