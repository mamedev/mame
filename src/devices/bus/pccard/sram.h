// license: BSD-3-Clause
// copyright-holders: Dirk Best
/***************************************************************************

    SRAM PC Cards

***************************************************************************/

#ifndef MAME_BUS_PCCARD_SRAM_H
#define MAME_BUS_PCCARD_SRAM_H

#pragma once

#include "pccard.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> pccard_sram_device

class pccard_sram_device :
	public device_t,
	public device_memory_interface,
	public device_image_interface,
	public device_pccard_interface
{
public:
	void battery_voltage_1_w(int state);
	void battery_voltage_2_w(int state);
	void write_protect_w(int state);

protected:
	// construction/destruction
	pccard_sram_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device_t overrides
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;

	// device_memory_interface overrides
	virtual space_config_vector memory_space_config() const override;

	// device_image_interface overrides
	virtual bool is_readable()  const noexcept override { return true; }
	virtual bool is_writeable() const noexcept override { return true; }
	virtual bool is_creatable() const noexcept override { return true; }
	virtual bool is_reset_on_load() const noexcept override { return false; }
	virtual bool support_command_line_image_creation() const noexcept override { return true; }
	virtual char const *file_extensions() const noexcept override { return "bin"; }
	virtual const char *image_type_name() const noexcept override { return "sramcard"; }
	virtual const char *image_brief_type_name() const noexcept override { return "sram"; }

	// device_pccard_interface overrides
	virtual uint16_t read_memory(offs_t offset, uint16_t mem_mask = ~0) override;
	virtual uint16_t read_reg(offs_t offset, uint16_t mem_mask = ~0) override;
	virtual void write_memory(offs_t offset, uint16_t data, uint16_t mem_mask = ~0) override;
	virtual void write_reg(offs_t offset, uint16_t data, uint16_t mem_mask = ~0) override;

	void set_cd(bool state);

	address_space_config m_memory_space_config;
	address_space_config m_attribute_space_config;

	int8_t m_cd;
	int8_t m_bvd1;
	int8_t m_bvd2;
	int8_t m_wp;
};

class pccard_mitsubishi_sram_device : public pccard_sram_device
{
protected:
	pccard_mitsubishi_sram_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device_image_interface overrides
	virtual std::pair<std::error_condition, std::string> call_load() override;
	virtual std::pair<std::error_condition, std::string> call_create(int format_type, util::option_resolution *format_options) override;
	virtual void call_unload() override;

private:
	required_shared_ptr<uint16_t> m_sram;
};

class pccard_mitsubishi_mf31m1_lycat01_device : public pccard_mitsubishi_sram_device
{
public:
	// construction/destruction
	pccard_mitsubishi_mf31m1_lycat01_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

private:
	void memory_map(address_map &map) ATTR_COLD;
};

class pccard_centennial_sram_device : public pccard_sram_device
{
protected:
	pccard_centennial_sram_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device_image_interface overrides
	virtual std::pair<std::error_condition, std::string> call_load() override;
	virtual std::pair<std::error_condition, std::string> call_create(int format_type, util::option_resolution *format_options) override;
	virtual void call_unload() override;

private:
	required_shared_ptr<uint16_t> m_sram;
	required_shared_ptr<uint16_t> m_eeprom;
	required_memory_region m_eeprom_default;
};

class pccard_centennial_sl01m_15_11194_device : public pccard_centennial_sram_device
{
public:
	// construction/destruction
	pccard_centennial_sl01m_15_11194_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

protected:
	// device_t overrides
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

private:
	void memory_map(address_map &map) ATTR_COLD;
	void attribute_map(address_map &map) ATTR_COLD;
};

class pccard_centennial_sl02m_15_11194_device : public pccard_centennial_sram_device
{
public:
	// construction/destruction
	pccard_centennial_sl02m_15_11194_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

protected:
	// device_t overrides
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

private:
	void memory_map(address_map &map) ATTR_COLD;
	void attribute_map(address_map &map) ATTR_COLD;
};

class pccard_centennial_sl04m_15_11194_device : public pccard_centennial_sram_device
{
public:
	// construction/destruction
	pccard_centennial_sl04m_15_11194_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

protected:
	// device_t overrides
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

private:
	void memory_map(address_map &map) ATTR_COLD;
	void attribute_map(address_map &map) ATTR_COLD;
};

// device type definition
DECLARE_DEVICE_TYPE(PCCARD_SRAM_MITSUBISHI_1M, pccard_mitsubishi_mf31m1_lycat01_device)
DECLARE_DEVICE_TYPE(PCCARD_SRAM_CENTENNIAL_1M, pccard_centennial_sl01m_15_11194_device)
DECLARE_DEVICE_TYPE(PCCARD_SRAM_CENTENNIAL_2M, pccard_centennial_sl02m_15_11194_device)
DECLARE_DEVICE_TYPE(PCCARD_SRAM_CENTENNIAL_4M, pccard_centennial_sl04m_15_11194_device)

#endif // MAME_BUS_PCCARD_SRAM_H
