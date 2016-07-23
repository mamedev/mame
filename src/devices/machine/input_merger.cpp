// license:BSD-3-Clause
// copyright-holders:Dirk Best
/***************************************************************************

    Input Merger

    Used to connect multiple lines to a single device input while
    keeping it pulled high or low

***************************************************************************/

#include "input_merger.h"

#include <algorithm>
#include <iterator>


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type INPUT_MERGER_ACTIVE_HIGH = &device_creator<input_merger_active_high_device>;
const device_type INPUT_MERGER_ACTIVE_LOW = &device_creator<input_merger_active_low_device>;


//**************************************************************************
//  INPUT ADAPTER
//**************************************************************************

//-------------------------------------------------
//  input_merger_device - constructor
//-------------------------------------------------

input_merger_device::input_merger_device(machine_config const &mconfig, device_type type,
	char const *name, char const *tag, device_t *owner, UINT32 clock, char const *shortname, char const *source)
	: device_t(mconfig, type, name, tag, owner, clock, shortname, source),
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
}


//**************************************************************************
//  INPUT ADAPTER ACTIVE HIGH
//**************************************************************************

//-------------------------------------------------
//  input_merger_active_high_device - constructor
//-------------------------------------------------

input_merger_active_high_device::input_merger_active_high_device(machine_config const &mconfig, char const *tag, device_t *owner,	UINT32 clock)
	: input_merger_device(mconfig,	INPUT_MERGER_ACTIVE_HIGH, "Input Merger (Active High)", tag, owner, clock, "input_merger_hi", __FILE__)
{
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void input_merger_active_high_device::device_reset()
{
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

input_merger_active_low_device::input_merger_active_low_device(machine_config const &mconfig, char const *tag, device_t *owner,	UINT32 clock)
	: input_merger_device(mconfig,	INPUT_MERGER_ACTIVE_LOW, "Input Merger (Active Low)", tag, owner, clock, "input_merger_lo", __FILE__)
{
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void input_merger_active_low_device::device_reset()
{
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
