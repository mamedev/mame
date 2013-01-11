/**********************************************************************

    Commodore 1700/1750/1764 RAM Expansion Unit emulation

    Copyright MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

**********************************************************************/

#pragma once

#ifndef __REU__
#define __REU__


#include "emu.h"
#include "imagedev/cartslot.h"
#include "machine/c64exp.h"
#include "machine/mos8726.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> c64_reu_cartridge_device

class c64_reu_cartridge_device : public device_t,
									public device_c64_expansion_card_interface
{
public:
	// construction/destruction
	c64_reu_cartridge_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, UINT32 variant, int jp1, size_t ram_size);

	// optional information overrides
	virtual const rom_entry *device_rom_region() const;
	virtual machine_config_constructor device_mconfig_additions() const;

protected:
	enum
	{
		TYPE_1700,
		TYPE_1750,
		TYPE_1764
	};

	// device-level overrides
	virtual void device_config_complete() { m_shortname = "c64_reu"; }
	virtual void device_start();
	virtual void device_reset();

	// device_c64_expansion_card_interface overrides
	virtual UINT8 c64_cd_r(address_space &space, offs_t offset, UINT8 data, int sphi2, int ba, int roml, int romh, int io1, int io2);
	virtual void c64_cd_w(address_space &space, offs_t offset, UINT8 data, int sphi2, int ba, int roml, int romh, int io1, int io2);

	required_device<mos8726_device> m_dmac;

	int m_variant;
	int m_jp1;
	size_t m_ram_size;
};


// ======================> c64_reu1700_cartridge_device

class c64_reu1700_cartridge_device :  public c64_reu_cartridge_device
{
public:
	// construction/destruction
	c64_reu1700_cartridge_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};


// ======================> c64_reu1750_cartridge_device

class c64_reu1750_cartridge_device :  public c64_reu_cartridge_device
{
public:
	// construction/destruction
	c64_reu1750_cartridge_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};

// ======================> c64_reu1700_cartridge_device

class c64_reu1764_cartridge_device :  public c64_reu_cartridge_device
{
public:
	// construction/destruction
	c64_reu1764_cartridge_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};


// device type definition
extern const device_type C64_REU1700;
extern const device_type C64_REU1750;
extern const device_type C64_REU1764;



#endif
