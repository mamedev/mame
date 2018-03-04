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


//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

// standard 28XX class of 8-bit parallel EEPROMs
#define MCFG_EEPROM_2804_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, EEPROM_PARALLEL_2804, 0)
#define MCFG_EEPROM_2816_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, EEPROM_PARALLEL_2816, 0)
#define MCFG_EEPROM_2864_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, EEPROM_PARALLEL_2864, 0)
#define MCFG_EEPROM_28256_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, EEPROM_PARALLEL_28256, 0)
#define MCFG_EEPROM_28512_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, EEPROM_PARALLEL_28512, 0)
#define MCFG_EEPROM_28010_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, EEPROM_PARALLEL_28010, 0)
#define MCFG_EEPROM_28020_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, EEPROM_PARALLEL_28020, 0)
#define MCFG_EEPROM_28040_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, EEPROM_PARALLEL_28040, 0)

// true when external circuit is used to lock EEPROM after every write
#define MCFG_EEPROM_28XX_LOCK_AFTER_WRITE(_lock) \
	downcast<eeprom_parallel_28xx_device &>(*device).set_lock_after_write(_lock);


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************


// ======================> eeprom_parallel_base_device

class eeprom_parallel_base_device : public eeprom_base_device
{
protected:
	// construction/destruction
	eeprom_parallel_base_device(const machine_config &mconfig, device_type devtype, const char *tag, device_t *owner);

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
};



// ======================> eeprom_parallel_28xx_device

class eeprom_parallel_28xx_device : public eeprom_parallel_base_device
{
public:
	// configuration helpers
	void set_lock_after_write(bool lock) { m_lock_after_write = lock; }

	// read/write data lines
	DECLARE_WRITE8_MEMBER(write);
	DECLARE_READ8_MEMBER(read);

	// control lines
	DECLARE_WRITE_LINE_MEMBER(oe_w);
	DECLARE_WRITE8_MEMBER(unlock_write8);
	DECLARE_WRITE16_MEMBER(unlock_write16);
	DECLARE_WRITE32_MEMBER(unlock_write32);

protected:
	// construction/destruction
	eeprom_parallel_28xx_device(const machine_config &mconfig, device_type devtype, const char *tag, device_t *owner);

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

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
	eeprom_parallel_##_lowercase##_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock); \
}; \
DECLARE_DEVICE_TYPE(EEPROM_PARALLEL_##_uppercase, eeprom_parallel_##_lowercase##_device)

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
