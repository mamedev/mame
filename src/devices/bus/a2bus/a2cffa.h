// license:BSD-3-Clause
// copyright-holders:R. Belmont
/*********************************************************************

    a2cffa.h

    Implementation of Rich Dreher's IDE/CompactFlash board for
    the Apple II

*********************************************************************/

#ifndef MAME_BUS_A2BUS_A2CFFA_H
#define MAME_BUS_A2BUS_A2CFFA_H

#pragma once

#include "a2bus.h"
#include "bus/ata/ataintf.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class a2bus_cffa2000_device:
	public device_t,
	public device_a2bus_card_interface
{
protected:
	// construction/destruction
	a2bus_cffa2000_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_add_mconfig(machine_config &config) override;
	virtual const tiny_rom_entry *device_rom_region() const override;

	// overrides of standard a2bus slot functions
	virtual uint8_t read_c0nx(uint8_t offset) override;
	virtual void write_c0nx(uint8_t offset, uint8_t data) override;
	virtual uint8_t read_cnxx(uint8_t offset) override;
	virtual uint8_t read_c800(uint16_t offset) override;
	virtual void write_c800(uint16_t offset, uint8_t data) override;

	required_device<ata_interface_device> m_ata;

	uint8_t *m_rom;
	uint8_t m_eeprom[0x1000];

private:
	uint16_t m_lastdata, m_lastreaddata;
	bool m_writeprotect;
	bool m_inwritecycle;
};

class a2bus_cffa2_device : public a2bus_cffa2000_device, public device_nvram_interface
{
public:
	a2bus_cffa2_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device_config_nvram_interface overrides
	virtual void nvram_default() override;
	virtual void nvram_read(emu_file &file) override;
	virtual void nvram_write(emu_file &file) override;
};

class a2bus_cffa2_6502_device : public a2bus_cffa2000_device, public device_nvram_interface
{
public:
	a2bus_cffa2_6502_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	virtual const tiny_rom_entry *device_rom_region() const override;

protected:
	// device_config_nvram_interface overrides
	virtual void nvram_default() override;
	virtual void nvram_read(emu_file &file) override;
	virtual void nvram_write(emu_file &file) override;
};

// device type definition
DECLARE_DEVICE_TYPE(A2BUS_CFFA2,      a2bus_cffa2_device)
DECLARE_DEVICE_TYPE(A2BUS_CFFA2_6502, a2bus_cffa2_6502_device)

#endif // MAME_BUS_A2BUS_A2CFFA_H
