// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    devcpu.cpp

    CPU device definitions.

***************************************************************************/

#include "emu.h"
#include "emuopts.h"
#include <cctype>


//**************************************************************************
//  CPU RUNNING DEVICE
//**************************************************************************

//-------------------------------------------------
//  cpu_device - constructor
//-------------------------------------------------

cpu_device::cpu_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, type, tag, owner, clock),
		device_execute_interface(mconfig, *this),
		device_memory_interface(mconfig, *this),
		device_state_interface(mconfig, *this),
		device_disasm_interface(mconfig, *this),
		m_force_no_drc(false),
		m_access_to_be_redone(false),
		m_access_before_delay_tag(nullptr)
{
}


//-------------------------------------------------
//  cpu_device - destructor
//-------------------------------------------------

cpu_device::~cpu_device()
{
}


//-------------------------------------------------
//  allow_drc - return true if DRC is allowed
//-------------------------------------------------

bool cpu_device::allow_drc() const
{
	return mconfig().options().drc() && !m_force_no_drc;
}



bool cpu_device::cpu_is_interruptible() const
{
	return false;
}

bool cpu_device::access_before_time(u64 access_time, u64 current_time) noexcept
{
	s32 delta = access_time - current_time;
	if(*m_icountptr <= delta) {
		if(*m_icountptr > 0)
			*m_icountptr = 0;
		m_access_to_be_redone = true;
		return true;
	}

	*m_icountptr -= delta;

	return false;
}

bool cpu_device::access_before_delay(u32 cycles, const void *tag) noexcept
{
	if(tag == m_access_before_delay_tag) {
		m_access_before_delay_tag = nullptr;
		return false;
	}

	*m_icountptr -= cycles;

	if(*m_icountptr <= 0) {
		m_access_before_delay_tag = tag;
		m_access_to_be_redone = true;
		return true;
	}

	m_access_before_delay_tag = nullptr;
	return false;
}

void cpu_device::access_after_delay(u32 cycles) noexcept
{
	*m_icountptr -= cycles;
}

void cpu_device::defer_access() noexcept
{
	if(*m_icountptr > 0)
		*m_icountptr = 0;
	m_access_to_be_redone = true;
}
