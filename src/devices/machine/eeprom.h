// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    eeprom.h

    Base class for EEPROM devices.

***************************************************************************/

#ifndef MAME_MACHINE_EEPROM_H
#define MAME_MACHINE_EEPROM_H

#pragma once



//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_EEPROM_SIZE(_cells, _cellbits) \
	eeprom_base_device::static_set_size(*device, _cells, _cellbits);
#define MCFG_EEPROM_DATA(_data, _size) \
	eeprom_base_device::static_set_default_data(*device, _data, _size);
#define MCFG_EEPROM_DEFAULT_VALUE(_value) \
	eeprom_base_device::static_set_default_value(*device, _value);

#define MCFG_EEPROM_WRITE_TIME(_value) \
	eeprom_base_device::static_set_timing(*device, eeprom_base_device::WRITE_TIME, _value);
#define MCFG_EEPROM_WRITE_ALL_TIME(_value) \
	eeprom_base_device::static_set_timing(*device, eeprom_base_device::WRITE_ALL_TIME, _value);
#define MCFG_EEPROM_ERASE_TIME(_value) \
	eeprom_base_device::static_set_timing(*device, eeprom_base_device::ERASE_TIME, _value);
#define MCFG_EEPROM_ERASE_ALL_TIME(_value) \
	eeprom_base_device::static_set_timing(*device, eeprom_base_device::ERASE_ALL_TIME, _value);



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> eeprom_base_device

class eeprom_base_device :  public device_t,
							public device_nvram_interface
{
public:
	// timing constants
	enum timing_type
	{
		WRITE_TIME,         // default = 2ms
		WRITE_ALL_TIME,     // default = 8ms
		ERASE_TIME,         // default = 1ms
		ERASE_ALL_TIME,     // default = 8ms
		TIMING_COUNT
	};

	// inline configuration helpers
	static void static_set_size(device_t &device, int cells, int cellbits);
	static void static_set_default_data(device_t &device, const uint8_t *data, uint32_t size);
	static void static_set_default_data(device_t &device, const uint16_t *data, uint32_t size);
	static void static_set_default_value(device_t &device, uint32_t value);
	static void static_set_timing(device_t &device, timing_type type, const attotime &duration);

	// read/write/erase data
	uint32_t read(offs_t address);
	void write(offs_t address, uint32_t data);
	void write_all(uint32_t data);
	void erase(offs_t address);
	void erase_all();

	// status
	bool ready() const { return machine().time() >= m_completion_time; }

	// internal read/write without side-effects
	uint32_t internal_read(offs_t address);
	void internal_write(offs_t address, uint32_t data);

protected:
	// construction/destruction
	eeprom_base_device(const machine_config &mconfig, device_type devtype, const char *tag, device_t *owner);

	// device-level overrides
	virtual void device_validity_check(validity_checker &valid) const override;
	virtual void device_start() override;
	virtual void device_reset() override;

	// device_nvram_interface overrides
	virtual void nvram_default() override;
	virtual void nvram_read(emu_file &file) override;
	virtual void nvram_write(emu_file &file) override;

	optional_memory_region  m_region;

	std::unique_ptr<uint8_t []> m_data;

	// configuration state
	uint32_t                  m_cells;
	uint8_t                   m_address_bits;
	uint8_t                   m_data_bits;
	const void *            m_default_data;
	uint32_t                  m_default_data_size;
	uint32_t                  m_default_value;
	bool                    m_default_value_set;
	attotime                m_operation_time[TIMING_COUNT];

	// live state
	attotime                m_completion_time;
};

#endif // MAME_MACHINE_EEPROM_H
