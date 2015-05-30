// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
#ifndef __VBOY_ROM_H
#define __VBOY_ROM_H

#include "slot.h"


// ======================> vboy_rom_device

class vboy_rom_device : public device_t,
						public device_vboy_cart_interface
{
public:
	// construction/destruction
	vboy_rom_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source);
	vboy_rom_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual void device_start() {}

	// reading and writing
	virtual DECLARE_READ32_MEMBER(read_cart);
};

// ======================> vboy_eeprom_device

class vboy_eeprom_device : public vboy_rom_device
{
public:
	// construction/destruction
	vboy_eeprom_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// reading and writing
	virtual DECLARE_READ32_MEMBER(read_eeprom);
	virtual DECLARE_WRITE32_MEMBER(write_eeprom);
};



// device type definition
extern const device_type VBOY_ROM_STD;
extern const device_type VBOY_ROM_EEPROM;



#endif
