// license: BSD-3-Clause
// copyright-holders: Dirk Best
/**********************************************************************

    ColecoVision 'Super Game Cartridge' emulation

**********************************************************************/

#ifndef MAME_BUS_COLECO_CARTRIDGE_SGC_H
#define MAME_BUS_COLECO_CARTRIDGE_SGC_H

#pragma once

#include "exp.h"
#include "machine/intelfsh.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class colecovision_sgc_cartridge_device : public device_t, public device_colecovision_cartridge_interface
{
protected:
	colecovision_sgc_cartridge_device(const machine_config &mconfig, device_type type, const char* tag, device_t* owner, uint32_t clock);

	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// cartridge interface
	virtual void load_done() override;
	virtual uint8_t read(offs_t offset, int _8000, int _a000, int _c000, int _e000) override;
	virtual void write(offs_t offset, uint8_t data, int _8000, int _a000, int _c000, int _e000) override;

	required_device<intelfsh8_device> m_flash;
	required_memory_region m_flash_region;

private:
	uint8_t m_slot0_bank;
	uint8_t m_slot1_bank;
	uint8_t m_slot2_bank;
	uint8_t m_slot3_bank;
	uint8_t m_flash_a16;

	offs_t banked_address(offs_t offset);
};

class colecovision_sgc_1mbit_cartridge_device : public colecovision_sgc_cartridge_device
{
public:
	colecovision_sgc_1mbit_cartridge_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
};

class colecovision_sgc_2mbit_cartridge_device : public colecovision_sgc_cartridge_device
{
public:
	colecovision_sgc_2mbit_cartridge_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
};

class colecovision_sgc_4mbit_cartridge_device : public colecovision_sgc_cartridge_device
{
public:
	colecovision_sgc_4mbit_cartridge_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
};

// device type declaration
DECLARE_DEVICE_TYPE(COLECOVISION_SGC_1MBIT, colecovision_sgc_1mbit_cartridge_device)
DECLARE_DEVICE_TYPE(COLECOVISION_SGC_2MBIT, colecovision_sgc_2mbit_cartridge_device)
DECLARE_DEVICE_TYPE(COLECOVISION_SGC_4MBIT, colecovision_sgc_4mbit_cartridge_device)

#endif // MAME_BUS_COLECO_CARTRIDGE_SGC_H
