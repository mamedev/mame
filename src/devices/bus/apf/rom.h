// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
#ifndef MAME_BUS_APF_ROM_H
#define MAME_BUS_APF_ROM_H

#include "slot.h"


// ======================> apf_rom_device

class apf_rom_device : public device_t,
						public device_apf_cart_interface
{
public:
	// construction/destruction
	apf_rom_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// reading and writing
	virtual uint8_t read_rom(offs_t offset) override;

protected:
	apf_rom_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override { }
	virtual void device_reset() override { }
};

// ======================> apf_basic_device

class apf_basic_device : public apf_rom_device
{
public:
	// construction/destruction
	apf_basic_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// reading and writing
	virtual uint8_t extra_rom(offs_t offset) override;
};

// ======================> apf_spacedst_device

class apf_spacedst_device : public apf_rom_device
{
public:
	// construction/destruction
	apf_spacedst_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// reading and writing
	virtual uint8_t read_ram(offs_t offset) override;
	virtual void write_ram(offs_t offset, uint8_t data) override;
};



// device type definition
DECLARE_DEVICE_TYPE(APF_ROM_STD,      apf_rom_device)
DECLARE_DEVICE_TYPE(APF_ROM_BASIC,    apf_basic_device)
DECLARE_DEVICE_TYPE(APF_ROM_SPACEDST, apf_spacedst_device)

#endif // MAME_BUS_APF_ROM_H
