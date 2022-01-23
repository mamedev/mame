// license:BSD-3-Clause
// copyright-holders:Dirk Best, Vas Crabb
/***************************************************************************

Input Merger

Used to connect multiple lines to a single device input while keeping it
pulled high or low.

Default initial input pin(s) state:
- ANY_HIGH: 0
- ALL_HIGH: 1
- ANY_LOW: 1
- ALL_LOW: 0

TODO:
- Change the strange initial input state, eg. right now it's all 1 for an AND
  gate. All 0 for all devices would be more intuitive, but that would need a
  config handler for setting the number of input pins since otherwise it can't
  know if the state is active or not.
- Call m_output_handler at reset, eg. a NOR gate whose inputs are low at power-on
  should output 1 (to avoid surprises, this should be done after machine_reset).
- Related to currently not calling m_output_handler at reset, if the hardware
  1st value written equals the initial state, m_output_handler is not called,
  eg. writing 1 to an AND gate pin when the default initial state is 1.

***************************************************************************/

#include "emu.h"
#include "input_merger.h"

#include <algorithm>
#include <iterator>

//#define VERBOSE 1
#include "logmacro.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(INPUT_MERGER_ANY_HIGH, input_merger_any_high_device, "ipt_merge_any_hi", "Input Merger (any high)") // OR
DEFINE_DEVICE_TYPE(INPUT_MERGER_ALL_HIGH, input_merger_all_high_device, "ipt_merge_all_hi", "Input Merger (all high)") // AND
DEFINE_DEVICE_TYPE(INPUT_MERGER_ANY_LOW,  input_merger_any_low_device,  "ipt_merge_any_lo", "Input Merger (any low)") // NAND
DEFINE_DEVICE_TYPE(INPUT_MERGER_ALL_LOW,  input_merger_all_low_device,  "ipt_merge_all_lo", "Input Merger (all low)") // NOR


//**************************************************************************
//  INPUT ADAPTER
//**************************************************************************

//-------------------------------------------------
//  input_merger_device - constructor
//-------------------------------------------------

input_merger_device::input_merger_device(
		machine_config const &mconfig,
		device_type type,
		char const *tag,
		device_t *owner,
		uint32_t clock,
		u32 initval,
		u32 xorval,
		int active)
	: device_t(mconfig, type, tag, owner, clock)
	, m_output_handler(*this)
	, m_initval(initval)
	, m_xorval(xorval)
	, m_active(active)
	, m_state(initval)
{
}

//-------------------------------------------------
//  input_merger_device - destructor
//-------------------------------------------------

input_merger_device::~input_merger_device()
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void input_merger_device::device_start()
{
	m_output_handler.resolve_safe();
	save_item(NAME(m_state));
	m_state = m_initval;
}

//-------------------------------------------------
//  update_state - verify current input line state
//-------------------------------------------------

TIMER_CALLBACK_MEMBER(input_merger_device::update_state)
{
	if (BIT(m_state, param >> 1) != BIT(param, 0))
	{
		LOG("state[%d] = %d\n", param >> 1, BIT(param, 0));
		m_state ^= u32(1) << (param >> 1);
		m_output_handler((m_state ^ m_xorval) ? m_active : !m_active);
	}
}


//**************************************************************************
//  SPECIALISATIONS
//**************************************************************************

input_merger_any_high_device::input_merger_any_high_device(machine_config const &mconfig, char const *tag, device_t *owner, uint32_t clock)
	: input_merger_device(mconfig, INPUT_MERGER_ANY_HIGH, tag, owner, clock, u32(0), u32(0), 1)
{
}

input_merger_all_high_device::input_merger_all_high_device(machine_config const &mconfig, char const *tag, device_t *owner, uint32_t clock)
	: input_merger_device(mconfig, INPUT_MERGER_ALL_HIGH, tag, owner, clock, ~u32(0), ~u32(0), 0)
{
}

input_merger_any_low_device::input_merger_any_low_device(machine_config const &mconfig, char const *tag, device_t *owner, uint32_t clock)
	: input_merger_device(mconfig, INPUT_MERGER_ANY_LOW, tag, owner, clock, ~u32(0), ~u32(0), 1)
{
}

input_merger_all_low_device::input_merger_all_low_device(machine_config const &mconfig, char const *tag, device_t *owner, uint32_t clock)
	: input_merger_device(mconfig, INPUT_MERGER_ALL_LOW, tag, owner, clock, u32(0), u32(0), 0)
{
}
