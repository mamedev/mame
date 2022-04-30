// license:BSD-3-Clause
// copyright-holders:Nathan Woods, Dirk Best
/*************************************************************************

    RAM device

    Provides a configurable amount of RAM to drivers

**************************************************************************/

#ifndef MAME_MACHINE_RAM_H
#define MAME_MACHINE_RAM_H

#pragma once

#include <memory>
#include <utility>
#include <vector>


/***************************************************************************
    CONSTANTS
***************************************************************************/

#define RAM_TAG             "ram"


/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

class ram_device : public device_t
{
public:
	using extra_option = std::pair<std::string, u32>;
	using extra_option_vector = std::vector<extra_option>;

	// construction/destruction
	ram_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock = 0);

	// accessors
	u32 size() const { return m_size; }
	u32 mask() const { return m_size - 1; }
	u8 *pointer() { return &m_pointer[0]; }
	char const *default_size_string() const { return m_default_size; }
	u32 default_size() const;
	extra_option_vector const &extra_options() const;

	// read/write
	u8 read(offs_t offset)              { return m_pointer[offset % m_size]; }
	void write(offs_t offset, u8 data)  { m_pointer[offset % m_size] = data; }

	// inline configuration helpers
	ram_device &set_default_size(char const *default_size) { m_default_size = default_size; return *this; }
	ram_device &set_extra_options(char const *extra_options)
	{
		m_extra_options_string = extra_options && extra_options[0] ? extra_options : nullptr;
		m_extra_options.clear();
		return *this;
	}
	ram_device &set_default_value(u8 default_value) { m_default_value = default_value; return *this; }

protected:
	virtual void device_start() override;
	virtual void device_validity_check(validity_checker &valid) const override;

private:
	bool is_valid_size(u32 size) const;

	// device state
	u32                         m_size;
	std::unique_ptr<u8 []>      m_pointer;

	// device config
	char const *                m_default_size;
	u8                          m_default_value;
	mutable extra_option_vector m_extra_options;
	char const *                m_extra_options_string;
};


// device type definition
DECLARE_DEVICE_TYPE(RAM, ram_device)

// device iterator
typedef device_type_enumerator<ram_device> ram_device_enumerator;

#endif // MAME_MACHINE_RAM_H
