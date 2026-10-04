// license:BSD-3-Clause
// copyright-holders:AJR
/***************************************************************************

    Zilog eZ80 CPU

***************************************************************************/

#ifndef MAME_CPU_Z80_EZ80_H
#define MAME_CPU_Z80_EZ80_H

#pragma once

#include "z80.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class ez80_device : public z80_device
{
public:
	// device type constructor
	ez80_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	// construction/destruction
	ez80_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock);
    
	virtual device_memory_interface::space_config_vector memory_space_config() const override;

	const address_space_config m_program_config;
	const address_space_config m_opcodes_config;
};

// device type declaration
DECLARE_DEVICE_TYPE(EZ80, ez80_device)

#endif // MAME_CPU_Z80_EZ80_H
