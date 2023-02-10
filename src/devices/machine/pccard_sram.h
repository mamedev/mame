// license: BSD-3-Clause
// copyright-holders: Dirk Best
/***************************************************************************

    SRAM PC Cards

***************************************************************************/

#ifndef MAME_MACHINE_PCCARD_SRAM_H
#define MAME_MACHINE_PCCARD_SRAM_H

#pragma once

#include "machine/pccard.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> pccard_sram_device

class pccard_sram_device : public device_t, public device_memory_interface, public device_pccard_interface
{
public:
	void card_detect_w(int state) { m_slot->card_detect_w(state); }
	void battery_voltage_1_w(int state) { m_slot->battery_voltage_1_w(state); }
	void battery_voltage_2_w(int state) { m_slot->battery_voltage_2_w(state); }
	void write_protect_w(int state) { m_slot->write_protect_w(state); }

protected:
	// construction/destruction
	pccard_sram_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device_t overrides
	virtual ioport_constructor device_input_ports() const override;
	virtual void device_add_mconfig(machine_config &config) override;
	virtual void device_start() override;
	virtual void device_reset() override;

	// device_memory_interface overrides
	virtual space_config_vector memory_space_config() const override;

	// device_pccard_interface overrides
	virtual uint16_t read_memory(offs_t offset, uint16_t mem_mask = ~0) override;
	virtual uint16_t read_reg(offs_t offset, uint16_t mem_mask = ~0) override;
	virtual void write_memory(offs_t offset, uint16_t data, uint16_t mem_mask = ~0) override;
	virtual void write_reg(offs_t offset, uint16_t data, uint16_t mem_mask = ~0) override;

	address_space_config m_memory_space_config;
	address_space_config m_attribute_space_config;

private:
	required_ioport m_switches;
};

class pccard_centennial_sl02m_15_11194_device : public pccard_sram_device
{
public:
	// construction/destruction
	pccard_centennial_sl02m_15_11194_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

protected:
	virtual const tiny_rom_entry *device_rom_region() const override;

private:
	void memory_map(address_map &map);
	void attribute_map(address_map &map);
};

class pccard_centennial_sl04m_15_11194_device : public pccard_sram_device
{
public:
	// construction/destruction
	pccard_centennial_sl04m_15_11194_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

protected:
	virtual const tiny_rom_entry *device_rom_region() const override;

private:
	void memory_map(address_map &map);
	void attribute_map(address_map &map);
};

// device type definition
DECLARE_DEVICE_TYPE(PCCARD_SRAM_CENTENNIAL_2M, pccard_centennial_sl02m_15_11194_device)
DECLARE_DEVICE_TYPE(PCCARD_SRAM_CENTENNIAL_4M, pccard_centennial_sl04m_15_11194_device)

#endif // MAME_MACHINE_PCCARD_SRAM_H
