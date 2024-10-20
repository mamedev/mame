// license:BSD-3-Clause
// copyright-holders:David Haywood

#ifndef MAME_TVGAMES_GENERALPLUS_GPL16250_M_H
#define MAME_TVGAMES_GENERALPLUS_GPL16250_M_H

#pragma once

class full_memory_device :
	public device_t,
	public device_memory_interface
{
public:
	// construction/destruction
	full_memory_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

	// configuration helpers
	template <typename... T> full_memory_device& set_map(T &&... args) { set_addrmap(0, std::forward<T>(args)...); return *this; }

	template <typename... T> full_memory_device& map(T &&... args) { set_addrmap(0, std::forward<T>(args)...); return *this; }

	address_space* get_program() { return m_program; }

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_config_complete() override;

	// device_memory_interface overrides
	virtual space_config_vector memory_space_config() const override;


private:
	// internal state
	address_space_config m_program_config;
	address_space *m_program;
};


// device type definition
DECLARE_DEVICE_TYPE(FULL_MEMORY, full_memory_device)

#endif  // MAME_TVGAMES_GENERALPLUS_GPL16250_M_H
