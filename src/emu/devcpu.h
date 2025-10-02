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

#include <utility>


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

	virtual bool cpu_is_interruptible() const;

	// To be used only for interruptible cpus
	bool access_to_be_redone() noexcept { return std::exchange(m_access_to_be_redone, false); }
	bool access_to_be_redone_noclear() noexcept { return m_access_to_be_redone; }

	// Returns true if the access must be aborted
	bool access_before_time(u64 access_time, u64 current_time) noexcept;
	bool access_before_delay(u32 cycles, const void *tag) noexcept;

	// The access has already happened, nothing to abort
	void access_after_delay(u32 cycles) noexcept;

	void defer_access() noexcept;
	void retry_access() noexcept;

protected:
	// construction/destruction
	cpu_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock);

private:
	// configured state
	bool m_force_no_drc;                   // whether or not to force DRC off

	bool m_access_to_be_redone;            // whether an access needs to be redone
	const void *m_access_before_delay_tag; // if the tag matches on access_before_delay, consider the delay to have already happened
};

#endif // MAME_EMU_DEVCPU_H
