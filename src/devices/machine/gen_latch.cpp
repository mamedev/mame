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

generic_latch_8_device::generic_latch_8_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, GENERIC_LATCH_8, "Generic 8-bit latch", tag, owner, clock, "generic_latch_8", __FILE__),
	m_latched_value(0),
	m_latch_read(0)
{
}

READ8_MEMBER( generic_latch_8_device::read )
{
	m_latch_read = 1;
	return m_latched_value;
}

WRITE8_MEMBER( generic_latch_8_device::write )
{
	machine().scheduler().synchronize(timer_expired_delegate(FUNC(generic_latch_8_device::sync_callback), this), data);
}

WRITE8_MEMBER( generic_latch_8_device::preset_w )
{
	m_latched_value = 0xff;
}

WRITE8_MEMBER( generic_latch_8_device::clear_w )
{
	m_latched_value = 0x00;
}

WRITE_LINE_MEMBER( generic_latch_8_device::preset_w )
{
	m_latched_value = 0xff;
}

WRITE_LINE_MEMBER( generic_latch_8_device::clear_w )
{
	m_latched_value = 0x00;
}

//-------------------------------------------------
//  soundlatch_sync_callback - time-delayed
//  callback to set a latch value
//-------------------------------------------------

void generic_latch_8_device::sync_callback(void *ptr, INT32 param)
{
	UINT8 value = param;

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

generic_latch_16_device::generic_latch_16_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, GENERIC_LATCH_16, "Generic 16-bit latch", tag, owner, clock, "generic_latch_16", __FILE__),
	m_latched_value(0),
	m_latch_read(0)
{
}

READ16_MEMBER( generic_latch_16_device::read )
{
	m_latch_read = 1;
	return m_latched_value;
}

WRITE16_MEMBER( generic_latch_16_device::write )
{
	machine().scheduler().synchronize(timer_expired_delegate(FUNC(generic_latch_16_device::sync_callback), this), data);
}

WRITE16_MEMBER( generic_latch_16_device::preset_w )
{
	m_latched_value = 0xffff;
}

WRITE16_MEMBER( generic_latch_16_device::clear_w )
{
	m_latched_value = 0x0000;
}

WRITE_LINE_MEMBER( generic_latch_16_device::preset_w )
{
	m_latched_value = 0xffff;
}

WRITE_LINE_MEMBER( generic_latch_16_device::clear_w )
{
	m_latched_value = 0x0000;
}

//-------------------------------------------------
//  soundlatch_sync_callback - time-delayed
//  callback to set a latch value
//-------------------------------------------------

void generic_latch_16_device::sync_callback(void *ptr, INT32 param)
{
	UINT16 value = param;

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
