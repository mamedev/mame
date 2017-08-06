// license:BSD-3-Clause
// copyright-holders:Dirk Best
/***************************************************************************

    Input Merger

    Used to connect multiple lines to a single device input while
    keeping it pulled high or low

***************************************************************************/

#include "emu.h"
#include "input_merger.h"

#include <algorithm>
#include <iterator>


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(INPUT_MERGER_ACTIVE_HIGH, input_merger_active_high_device, "ipt_merger_hi", "Input Merger (active high)")
DEFINE_DEVICE_TYPE(INPUT_MERGER_ACTIVE_LOW,  input_merger_active_low_device,  "ipt_merger_lo", "Input Merger (active low)")


//**************************************************************************
//  INPUT ADAPTER
//**************************************************************************

//-------------------------------------------------
//  input_merger_device - constructor
//-------------------------------------------------

input_merger_device::input_merger_device(machine_config const &mconfig, device_type type, char const *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, type, tag, owner, clock),
	m_output_handler(*this)
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
}


//**************************************************************************
//  INPUT ADAPTER ACTIVE HIGH
//**************************************************************************

//-------------------------------------------------
//  input_merger_active_high_device - constructor
//-------------------------------------------------

input_merger_active_high_device::input_merger_active_high_device(machine_config const &mconfig, char const *tag, device_t *owner,   uint32_t clock)
	: input_merger_device(mconfig, INPUT_MERGER_ACTIVE_HIGH, tag, owner, clock)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void input_merger_active_high_device::device_start()
{
	input_merger_device::device_start();
	std::fill(std::begin(m_state), std::end(m_state), false);
}

//-------------------------------------------------
//  update_state - verify current input line state
//-------------------------------------------------

void input_merger_active_high_device::update_state()
{
	bool state = std::any_of(std::begin(m_state), std::end(m_state), [&](bool state) { return state == true; });
	m_output_handler(state ? ASSERT_LINE : CLEAR_LINE);
}


//**************************************************************************
//  INPUT ADAPTER ACTIVE LOW
//**************************************************************************

//-------------------------------------------------
//  input_merger_active_low_device - constructor
//-------------------------------------------------

input_merger_active_low_device::input_merger_active_low_device(machine_config const &mconfig, char const *tag, device_t *owner, uint32_t clock)
	: input_merger_device(mconfig, INPUT_MERGER_ACTIVE_LOW, tag, owner, clock)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void input_merger_active_low_device::device_start()
{
	input_merger_device::device_start();
	std::fill(std::begin(m_state), std::end(m_state), true);
}

//-------------------------------------------------
//  update_state - verify current input line state
//-------------------------------------------------

void input_merger_active_low_device::update_state()
{
	bool state = std::any_of(std::begin(m_state), std::end(m_state), [&](bool state) { return state == false; });
	m_output_handler(state ? ASSERT_LINE : CLEAR_LINE);
}
