// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/***************************************************************************

    Generic 8 bit and 16 bit latch devices

***************************************************************************/

#include "emu.h"
#include "gen_latch.h"


//**************************************************************************
//  DEVICE TYPE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(GENERIC_LATCH_8, generic_latch_8_device, "generic_latch_8", "Generic 8-bit latch")
DEFINE_DEVICE_TYPE(GENERIC_LATCH_16, generic_latch_16_device, "generic_latch_16", "Generic 16-bit latch")


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  generic_latch_base_device - constructor
//-------------------------------------------------

generic_latch_base_device::generic_latch_base_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock) :
	device_t(mconfig, type, tag, owner, clock),
	m_separate_acknowledge(false),
	m_latch_written(false),
	m_data_pending_cb(*this)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void generic_latch_base_device::device_start()
{
	m_data_pending_cb.resolve_safe();
	save_item(NAME(m_latch_written));

	// synchronization is needed since other devices may not be initialized yet
	machine().scheduler().synchronize(timer_expired_delegate(FUNC(generic_latch_base_device::init_callback), this));
}

//-------------------------------------------------
//  init_callback - set initial state
//-------------------------------------------------

void generic_latch_base_device::init_callback(void *ptr, s32 param)
{
	m_data_pending_cb(m_latch_written ? 1 : 0);
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void generic_latch_base_device::device_reset()
{
	m_latch_written = false;
}

//-------------------------------------------------
//  pending_r - tell whether the latch is waiting
//  to be read
//-------------------------------------------------

READ_LINE_MEMBER(generic_latch_base_device::pending_r)
{
	return m_latch_written ? 1 : 0;
}

//-------------------------------------------------
//  set_latch_written - helper to signal that latch
//  has been written or has been read
//-------------------------------------------------

void generic_latch_base_device::set_latch_written(bool latch_written)
{
	if (m_latch_written != latch_written)
	{
		m_latch_written = latch_written;
		m_data_pending_cb(latch_written ? 1 : 0);
	}
}

u8 generic_latch_base_device::acknowledge_r(address_space &space)
{
	if (!machine().side_effects_disabled())
		set_latch_written(false);
	return space.unmap();
}

void generic_latch_base_device::acknowledge_w(u8 data)
{
	set_latch_written(false);
}

//-------------------------------------------------
//  generic_latch_8_device - constructor
//-------------------------------------------------

generic_latch_8_device::generic_latch_8_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	generic_latch_base_device(mconfig, GENERIC_LATCH_8, tag, owner, clock),
	m_latched_value(0)
{
}

u8 generic_latch_8_device::read()
{
	if (!has_separate_acknowledge() && !machine().side_effects_disabled())
		set_latch_written(false);
	return m_latched_value;
}

void generic_latch_8_device::write(u8 data)
{
	machine().scheduler().synchronize(timer_expired_delegate(FUNC(generic_latch_8_device::sync_callback), this), data);
}

void generic_latch_8_device::preset_w(u8 data)
{
	m_latched_value = data;
}

void generic_latch_8_device::clear_w(u8 data)
{
	m_latched_value = 0x00;
}

WRITE_LINE_MEMBER( generic_latch_8_device::preset )
{
	m_latched_value = 0xff;
}

WRITE_LINE_MEMBER( generic_latch_8_device::clear )
{
	m_latched_value = 0x00;
}

//-------------------------------------------------
//  soundlatch_sync_callback - time-delayed
//  callback to set a latch value
//-------------------------------------------------

void generic_latch_8_device::sync_callback(void *ptr, s32 param)
{
	u8 value = param;

	// if the latch has been written and the value is changed, log a warning
	if (is_latch_written() && m_latched_value != value)
		logerror("Warning: latch written before being read. Previous: %02x, new: %02x\n", m_latched_value, value);

	// store the new value and mark it not read
	m_latched_value = value;
	set_latch_written(true);
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void generic_latch_8_device::device_start()
{
	// register for state saving
	generic_latch_base_device::device_start();
	save_item(NAME(m_latched_value));
}

//-------------------------------------------------
//  generic_latch_16_device - constructor
//-------------------------------------------------

generic_latch_16_device::generic_latch_16_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	generic_latch_base_device(mconfig, GENERIC_LATCH_16, tag, owner, clock),
	m_latched_value(0)
{
}

u16 generic_latch_16_device::read()
{
	if (!has_separate_acknowledge() && !machine().side_effects_disabled())
		set_latch_written(false);
	return m_latched_value;
}

void generic_latch_16_device::write(u16 data)
{
	machine().scheduler().synchronize(timer_expired_delegate(FUNC(generic_latch_16_device::sync_callback), this), data);
}

void generic_latch_16_device::preset_w(u16 data)
{
	m_latched_value = data;
}

void generic_latch_16_device::clear_w(u16 data)
{
	m_latched_value = 0x0000;
}

WRITE_LINE_MEMBER( generic_latch_16_device::preset )
{
	m_latched_value = 0xffff;
}

WRITE_LINE_MEMBER( generic_latch_16_device::clear )
{
	m_latched_value = 0x0000;
}

//-------------------------------------------------
//  soundlatch_sync_callback - time-delayed
//  callback to set a latch value
//-------------------------------------------------

void generic_latch_16_device::sync_callback(void *ptr, s32 param)
{
	u16 value = param;

	// if the latch has been written and the value is changed, log a warning
	if (is_latch_written() && m_latched_value != value)
		logerror("Warning: latch written before being read. Previous: %02x, new: %02x\n", m_latched_value, value);

	// store the new value and mark it not read
	m_latched_value = value;
	set_latch_written(true);
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void generic_latch_16_device::device_start()
{
	// register for state saving
	generic_latch_base_device::device_start();
	save_item(NAME(m_latched_value));
}
