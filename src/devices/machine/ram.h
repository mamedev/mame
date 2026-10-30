// license:BSD-3-Clause
// copyright-holders:Nathan Woods, Dirk Best
/*************************************************************************

    RAM device

    Provides a configurable amount of RAM to drivers

**************************************************************************/

#ifndef MAME_MACHINE_RAM_H
#define MAME_MACHINE_RAM_H

#pragma once

#include <cassert>
#include <cstdlib>
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

class device_ram_interface : public device_interface
{
public:
	virtual ~device_ram_interface();

	virtual u32 size() const = 0;

protected:
	device_ram_interface(const machine_config& mconfig, device_t& device);
};

class ram_device : public device_t, public device_single_card_slot_interface<device_ram_interface>
{
public:
	// construction/destruction
	ram_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock = 0);

	// accessors
	u32 size() const { return m_size; }
	u32 mask() const { return m_size - 1; }
	template <typename T> T* pointer() { static_assert(std::is_integral_v<T>); allocate_ram(); return reinterpret_cast<T*>(m_pointer.get()); }
	u8 *pointer() { allocate_ram(); return pointer<u8>(); }

	// read/write
	u8 read(offs_t offset)              { return pointer<u8>()[offset % m_size]; }
	void write(offs_t offset, u8 data)  { pointer<u8>()[offset % m_size] = data; }

	// inline configuration helpers
	ram_device &set_default_size(char const *default_size) { m_default_size = sanitise_option(default_size); update_options(); return *this; }
	ram_device &set_bits(u8 bits) { m_bits = bits; return *this; }
	ram_device &set_extra_options(char const *extra_options)
	{
		m_extra_options_string = (extra_options && extra_options[0]) ? extra_options : nullptr;
		update_options();
		return *this;
	}
	ram_device &set_default_value(u8 default_value) { m_default_value = default_value; return *this; }

protected:
	virtual void device_validity_check(validity_checker& valid) const override ATTR_COLD;
	virtual void device_config_complete() override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;

	std::string sanitise_option(std::string_view option);
	void update_options();
	void allocate_ram();

	struct stdlib_deleter { void operator()(void *p) const { std::free(p); } };

	// device state
	u32                                     m_size;
	std::unique_ptr<void, stdlib_deleter>   m_pointer;

	// device config
	u8                                      m_bits;
	std::string                             m_default_size;
	u8                                      m_default_value;
	char const *                            m_extra_options_string;
};


// device type definition
DECLARE_DEVICE_TYPE(RAM, ram_device)

#endif // MAME_MACHINE_RAM_H
