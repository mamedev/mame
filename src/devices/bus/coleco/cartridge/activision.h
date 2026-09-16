// license: BSD-3-Clause
// copyright-holders: Dirk Best
/**********************************************************************

    ColecoVision 'Activision' cartridge emulation

**********************************************************************/

#ifndef MAME_BUS_COLECO_CARTRIDGE_ACTIVISION_H
#define MAME_BUS_COLECO_CARTRIDGE_ACTIVISION_H

#pragma once

#include "exp.h"
#include "machine/i2cmem.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class colecovision_activision_cartridge_device : public device_t, public device_colecovision_cartridge_interface
{
public:
	colecovision_activision_cartridge_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	colecovision_activision_cartridge_device(const machine_config &mconfig, device_type type, const char* tag, device_t* owner, uint32_t clock);

	optional_device<i2cmem_device> m_eeprom;

	virtual void device_start() override ATTR_COLD;

	// cartridge interface
	virtual uint8_t read(offs_t offset, int _8000, int _a000, int _c000, int _e000) override;
	virtual void write(offs_t offset, uint8_t data, int _8000, int _a000, int _c000, int _e000) override;

private:
	uint8_t m_active_bank;
};

class colecovision_activision_256b_cartridge_device : public colecovision_activision_cartridge_device
{
public:
	colecovision_activision_256b_cartridge_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
};

class colecovision_activision_32k_cartridge_device : public colecovision_activision_cartridge_device
{
public:
	colecovision_activision_32k_cartridge_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
};

// device type declaration
DECLARE_DEVICE_TYPE(COLECOVISION_ACTIVISION, colecovision_activision_cartridge_device)
DECLARE_DEVICE_TYPE(COLECOVISION_ACTIVISION_256B, colecovision_activision_256b_cartridge_device)
DECLARE_DEVICE_TYPE(COLECOVISION_ACTIVISION_32K, colecovision_activision_32k_cartridge_device)

#endif // MAME_BUS_COLECO_CARTRIDGE_ACTIVISION_H
