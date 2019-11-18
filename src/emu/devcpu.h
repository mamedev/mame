// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    devcpu.h

    CPU device definitions.

***************************************************************************/
#ifndef MAME_EMU_DEVCPU_H
#define MAME_EMU_DEVCPU_H

#pragma once

#include "didisasm.h"
#include "diexec.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> cpu_device

class cpu_device :  public device_t,
					public device_execute_interface,
					public device_memory_interface,
					public device_state_interface,
					public device_disasm_interface
{
public:
	virtual ~cpu_device();

	// configuration helpers
	void set_force_no_drc(bool value) { m_force_no_drc = value; }
	bool allow_drc() const;

protected:
	// construction/destruction
	cpu_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock);

private:
	// configured state
	bool                    m_force_no_drc;             // whether or not to force DRC off
};

#endif // MAME_EMU_DEVCPU_H
