// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/***************************************************************************

    Generic 8bit and 16 bit latch devices

***************************************************************************/
#include "emu.h"
#include "gen_latch.h"

//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type GENERIC_LATCH_8 = &device_creator<generic_latch_8_device>;


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  generic_latch_8_device - constructor
//-------------------------------------------------

generic_latch_8_device::generic_latch_8_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, GENERIC_LATCH_8, "Generic 8-bit latch", tag, owner, clock, "generic_latch_8", __FILE__),
	m_latched_value(0),
	m_latch_read(0)
{
}

uint8_t generic_latch_8_device::read(address_space &space, offs_t offset, uint8_t mem_mask)
{
	m_latch_read = 1;
	return m_latched_value;
}

void generic_latch_8_device::write(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask)
{
	machine().scheduler().synchronize(timer_expired_delegate(FUNC(generic_latch_8_device::sync_callback), this), data);
}

void generic_latch_8_device::preset_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask)
{
	m_latched_value = 0xff;
}

void generic_latch_8_device::clear_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask)
{
	m_latched_value = 0x00;
}

void generic_latch_8_device::preset_w(int state)
{
	m_latched_value = 0xff;
}

void generic_latch_8_device::clear_w(int state)
{
	m_latched_value = 0x00;
}

//-------------------------------------------------
//  soundlatch_sync_callback - time-delayed
//  callback to set a latch value
//-------------------------------------------------

void generic_latch_8_device::sync_callback(void *ptr, int32_t param)
{
	uint8_t value = param;

	// if the latch hasn't been read and the value is changed, log a warning
	if (!m_latch_read && m_latched_value != value)
		logerror("Warning: latch %s written before being read. Previous: %02x, new: %02x\n", tag(), m_latched_value, value);

	// store the new value and mark it not read
	m_latched_value = value;
	m_latch_read = 0;
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void generic_latch_8_device::device_start()
{
	// register for state saving
	save_item(NAME(m_latched_value));
	save_item(NAME(m_latch_read));
}

//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type GENERIC_LATCH_16 = &device_creator<generic_latch_16_device>;


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  generic_latch_16_device - constructor
//-------------------------------------------------

generic_latch_16_device::generic_latch_16_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, GENERIC_LATCH_16, "Generic 16-bit latch", tag, owner, clock, "generic_latch_16", __FILE__),
	m_latched_value(0),
	m_latch_read(0)
{
}

uint16_t generic_latch_16_device::read(address_space &space, offs_t offset, uint16_t mem_mask)
{
	m_latch_read = 1;
	return m_latched_value;
}

void generic_latch_16_device::write(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask)
{
	machine().scheduler().synchronize(timer_expired_delegate(FUNC(generic_latch_16_device::sync_callback), this), data);
}

void generic_latch_16_device::preset_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask)
{
	m_latched_value = 0xffff;
}

void generic_latch_16_device::clear_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask)
{
	m_latched_value = 0x0000;
}

void generic_latch_16_device::preset_w(int state)
{
	m_latched_value = 0xffff;
}

void generic_latch_16_device::clear_w(int state)
{
	m_latched_value = 0x0000;
}

//-------------------------------------------------
//  soundlatch_sync_callback - time-delayed
//  callback to set a latch value
//-------------------------------------------------

void generic_latch_16_device::sync_callback(void *ptr, int32_t param)
{
	uint16_t value = param;

	// if the latch hasn't been read and the value is changed, log a warning
	if (!m_latch_read && m_latched_value != value)
		logerror("Warning: latch %s written before being read. Previous: %02x, new: %02x\n", tag(), m_latched_value, value);

	// store the new value and mark it not read
	m_latched_value = value;
	m_latch_read = 0;
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void generic_latch_16_device::device_start()
{
	// register for state saving
	save_item(NAME(m_latched_value));
	save_item(NAME(m_latch_read));
}
