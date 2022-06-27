// license:BSD-3-Clause
// copyright-holders:David Haywood

#include "emu.h"
#include "generalplus_gpl16250_m.h"

// device type definition
DEFINE_DEVICE_TYPE(FULL_MEMORY, full_memory_device, "full_memory", "SunPlus Full CS Memory Map")

full_memory_device::full_memory_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	device_t(mconfig, FULL_MEMORY, tag, owner, clock),
	device_memory_interface(mconfig, *this),
	m_program(nullptr)
{
}

device_memory_interface::space_config_vector full_memory_device::memory_space_config() const
{
	return space_config_vector {
		std::make_pair(AS_PROGRAM, &m_program_config)
	};
}


void full_memory_device::device_config_complete()
{
	m_program_config = address_space_config( "program", ENDIANNESS_BIG, 16, 32, -1 );
}

void full_memory_device::device_start()
{
	m_program = &space(AS_PROGRAM);
}

