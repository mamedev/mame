// license:BSD-3-Clause
// copyright-holders:David Haywood

#ifndef MAME_MACHINE_SPECTRUM_MEM_H
#define MAME_MACHINE_SPECTRUM_MEM_H

#pragma once

#include "emu.h"

// device type definition
DECLARE_DEVICE_TYPE(SPECTRUM_MEMORY, spectrum_memory)


class spectrum_memory :
	public device_t,
	public device_memory_interface
{
public:
	// construction/destruction
	spectrum_memory(const machine_config& mconfig, const char* tag, device_t* owner, u32 clock = 0)
		: device_t(mconfig, SPECTRUM_MEMORY, tag, owner, clock),
		device_memory_interface(mconfig, *this),
		m_program(nullptr)
	{
	}

	template <typename... T> spectrum_memory& set_map(T &&... args) { set_addrmap(0, std::forward<T>(args)...); return *this; }
	template <typename... T> spectrum_memory& map(T &&... args) { set_addrmap(0, std::forward<T>(args)...); return *this; }

	address_space* get_space()
	{
		return m_program;
	}

protected:

	virtual void device_start() override
	{
		m_program = &space(AS_PROGRAM);
	}

	virtual void device_config_complete() override
	{
		m_program_config = address_space_config( "program", ENDIANNESS_LITTLE, 8, 16, 0 );
	}

	// device_memory_interface overrides
	virtual device_memory_interface::space_config_vector memory_space_config() const override
	{
		return space_config_vector {
			std::make_pair(AS_PROGRAM, &m_program_config)
		};
	}


private:
	// internal state
	address_space_config m_program_config;
	address_space *m_program;
};

#endif
