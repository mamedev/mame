// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
#ifndef __SNS_SUFAMI_H
#define __SNS_SUFAMI_H

#include "snes_slot.h"
#include "rom.h"


// ======================> sns_rom_sufami_device

class sns_rom_sufami_device : public sns_rom_device
{
public:
	// construction/destruction
	sns_rom_sufami_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual void device_start();
	virtual machine_config_constructor device_mconfig_additions() const;

	// additional reading and writing
	virtual DECLARE_READ8_MEMBER(read_l);
	virtual DECLARE_READ8_MEMBER(read_h);
	virtual DECLARE_WRITE8_MEMBER(write_l);
	virtual DECLARE_WRITE8_MEMBER(write_h);

private:
	required_device<sns_sufami_cart_slot_device> m_slot1;
	required_device<sns_sufami_cart_slot_device> m_slot2;
};

// ======================> sns_rom_strom_device

class sns_rom_strom_device : public sns_rom_device
{
public:
	// construction/destruction
	sns_rom_strom_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual void device_start();

	// additional reading and writing
	virtual DECLARE_READ8_MEMBER(read_l);
};


// device type definition
extern const device_type SNS_LOROM_SUFAMI;
extern const device_type SNS_STROM;

#endif
