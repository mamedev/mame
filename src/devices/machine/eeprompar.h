// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    eeprompar.h

    Parallel EEPROM devices.

***************************************************************************/

#ifndef MAME_MACHINE_EEPROMPAR_H
#define MAME_MACHINE_EEPROMPAR_H

#pragma once

#include "eeprom.h"

class eeprom_parallel_base_device : public eeprom_base_device
{
protected:
	// construction/destruction
	eeprom_parallel_base_device(const machine_config &mconfig, device_type devtype, const char *tag, device_t *owner);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
};

class eeprom_parallel_28xx_device : public eeprom_parallel_base_device
{
public:
	// configuration helpers
	void lock_after_write(bool lock) { m_lock_after_write = lock; }

	// read/write data lines
	void write(offs_t offset, uint8_t data);
	uint8_t read(address_space &space, offs_t offset);

	// control lines
	void oe_w(int state);
	void unlock_write8(uint8_t data);
	void unlock_write16(uint16_t data);
	void unlock_write32(uint32_t data);

protected:
	// construction/destruction
	eeprom_parallel_28xx_device(const machine_config &mconfig, device_type devtype, const char *tag, device_t *owner);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	// configuration state
	bool            m_lock_after_write;         // lock EEPROM after writes

	// runtime state
	int             m_oe;                       // state of OE line (-1 = synchronized with read)
};



//**************************************************************************
//  DERIVED TYPES
//**************************************************************************

// macro for declaring a new device class
#define DECLARE_PARALLEL_EEPROM_DEVICE(_baseclass, _lowercase, _uppercase) \
class eeprom_parallel_##_lowercase##_device : public eeprom_parallel_##_baseclass##_device \
{ \
public: \
	eeprom_parallel_##_lowercase##_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0); \
}; \
DECLARE_DEVICE_TYPE(EEPROM_##_uppercase, eeprom_parallel_##_lowercase##_device)

// standard 28XX class of 8-bit EEPROMs
DECLARE_PARALLEL_EEPROM_DEVICE(28xx, 2804, 2804)
DECLARE_PARALLEL_EEPROM_DEVICE(28xx, 2816, 2816)
DECLARE_PARALLEL_EEPROM_DEVICE(28xx, 2864, 2864)
DECLARE_PARALLEL_EEPROM_DEVICE(28xx, 28256, 28256)
DECLARE_PARALLEL_EEPROM_DEVICE(28xx, 28512, 28512)
DECLARE_PARALLEL_EEPROM_DEVICE(28xx, 28010, 28010)
DECLARE_PARALLEL_EEPROM_DEVICE(28xx, 28020, 28020)
DECLARE_PARALLEL_EEPROM_DEVICE(28xx, 28040, 28040)

#endif // MAME_MACHINE_EEPROMPAR_H
