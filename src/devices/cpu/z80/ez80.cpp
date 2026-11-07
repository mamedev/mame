// license:BSD-3-Clause
// copyright-holders:AJR
/***************************************************************************

    Zilog eZ80 CPU

    TODO: all differences from Z80

***************************************************************************/

#include "emu.h"
#include "ez80.h"


//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

// device type definition
DEFINE_DEVICE_TYPE(EZ80, ez80_device, "ez80", "Zilog eZ80")


//**************************************************************************
//  DEVICE IMPLEMENTATION
//**************************************************************************

//-------------------------------------------------
//  ez80_device - constructor
//-------------------------------------------------

ez80_device::ez80_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock)
	: z80_device(mconfig, type, tag, owner, clock),
	  m_program_config("program", ENDIANNESS_LITTLE, 8, 24, 0),
	  m_opcodes_config("opcodes", ENDIANNESS_LITTLE, 8, 24, 0)
{
}

ez80_device::ez80_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: ez80_device(mconfig, EZ80, tag, owner, clock)
{
}

device_memory_interface::space_config_vector ez80_device::memory_space_config() const
{
	space_config_vector mem_space {
		std::make_pair(AS_PROGRAM, &m_program_config),
		std::make_pair(AS_IO, &m_io_config)
	};

	if (has_configured_map(AS_OPCODES))
		mem_space.emplace_back(AS_OPCODES, &m_opcodes_config);

	return mem_space;
}
