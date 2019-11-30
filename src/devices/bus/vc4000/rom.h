// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
#ifndef MAME_BUS_VC4000_ROM_H
#define MAME_BUS_VC4000_ROM_H

#include "slot.h"


// ======================> vc4000_rom_device

class vc4000_rom_device : public device_t,
						public device_vc4000_cart_interface
{
public:
	// construction/destruction
	vc4000_rom_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// reading and writing
	virtual uint8_t read_rom(offs_t offset) override;

protected:
	vc4000_rom_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override { }
	virtual void device_reset() override { }
};

// ======================> vc4000_rom4k_device

class vc4000_rom4k_device : public vc4000_rom_device
{
public:
	// construction/destruction
	vc4000_rom4k_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};

// ======================> vc4000_ram1k_device

class vc4000_ram1k_device : public vc4000_rom_device
{
public:
	// construction/destruction
	vc4000_ram1k_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// reading and writing
	virtual uint8_t read_ram(offs_t offset) override;
	virtual void write_ram(offs_t offset, uint8_t data) override;
};

// ======================> vc4000_chess2_device

class vc4000_chess2_device : public vc4000_rom_device
{
public:
	// construction/destruction
	vc4000_chess2_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// reading and writing
	virtual uint8_t extra_rom(offs_t offset) override;
	virtual uint8_t read_ram(offs_t offset) override;
	virtual void write_ram(offs_t offset, uint8_t data) override;
};



// device type definition
DECLARE_DEVICE_TYPE(VC4000_ROM_STD,    vc4000_rom_device)
DECLARE_DEVICE_TYPE(VC4000_ROM_ROM4K,  vc4000_rom4k_device)
DECLARE_DEVICE_TYPE(VC4000_ROM_RAM1K,  vc4000_ram1k_device)
DECLARE_DEVICE_TYPE(VC4000_ROM_CHESS2, vc4000_chess2_device)

#endif // MAME_BUS_VC4000_ROM_H
