// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
#ifndef __APF_ROM_H
#define __APF_ROM_H

#include "slot.h"


// ======================> apf_rom_device

class apf_rom_device : public device_t,
						public device_apf_cart_interface
{
public:
	// construction/destruction
	apf_rom_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source);
	apf_rom_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual void device_start() {}
	virtual void device_reset() {}

	// reading and writing
	virtual DECLARE_READ8_MEMBER(read_rom);
};

// ======================> apf_basic_device

class apf_basic_device : public apf_rom_device
{
public:
	// construction/destruction
	apf_basic_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// reading and writing
	virtual DECLARE_READ8_MEMBER(extra_rom);
};

// ======================> apf_spacedst_device

class apf_spacedst_device : public apf_rom_device
{
public:
	// construction/destruction
	apf_spacedst_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// reading and writing
	virtual DECLARE_READ8_MEMBER(read_ram);
	virtual DECLARE_WRITE8_MEMBER(write_ram);
};





// device type definition
extern const device_type APF_ROM_STD;
extern const device_type APF_ROM_BASIC;
extern const device_type APF_ROM_SPACEDST;


#endif
