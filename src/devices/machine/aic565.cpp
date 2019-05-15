// license:BSD-3-Clause
// copyright-holders:AJR
/**********************************************************************

    Adaptec AIC-565 Bus Auxiliary Interface Chip

    This ASIC implements the standard mailbox interface for Adaptec's
    ISA SCSI host adapters. It provides no specific SCSI functions,
    however, and most of the status register bits are defined by
    firmware implementation and have no special I/O features.

    The local processor interface is an enhanced version of that which
    the AHA-1542A provides using generic logic chips.

**********************************************************************/

#include "emu.h"
#include "aic565.h"


//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

// device type definition
DEFINE_DEVICE_TYPE(AIC565, aic565_device, "aic565", "AIC-565 Bus Auxiliary Interface Chip")


//**************************************************************************
//  DEVICE IMPLEMENTATION
//**************************************************************************

//-------------------------------------------------
//  aic565_device - constructor
//-------------------------------------------------

aic565_device::aic565_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, AIC565, tag, owner, clock)
	, m_hrst_callback(*this)
	, m_srst_callback(*this)
	, m_irq_callback(*this)
	, m_data_to_host(0)
	, m_data_from_host(0)
	, m_local_status(0)
	, m_aux_status(0)
	, m_interrupt_flags(0)
{
}


//-------------------------------------------------
//  device_resolve_objects - resolve objects that
//  may be needed for other devices to set
//  initial conditions at start time
//-------------------------------------------------

void aic565_device::device_resolve_objects()
{
	// resolve callbacks
	m_hrst_callback.resolve_safe();
	m_srst_callback.resolve_safe();
	m_irq_callback.resolve_safe();
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void aic565_device::device_start()
{
	// register state
	save_item(NAME(m_data_to_host));
	save_item(NAME(m_data_from_host));
	save_item(NAME(m_local_status));
	save_item(NAME(m_aux_status));
	save_item(NAME(m_interrupt_flags));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void aic565_device::device_reset()
{
	m_local_status = 0;
	m_aux_status = 0;
	m_interrupt_flags = 0;

	m_hrst_callback(CLEAR_LINE);
	m_srst_callback(CLEAR_LINE);
	m_irq_callback(CLEAR_LINE);
}


//-------------------------------------------------
//  host_r - read one byte from a host register
//-------------------------------------------------

u8 aic565_device::host_r(offs_t offset)
{
	switch (offset & 0x03)
	{
	case 0:
		// Bit 2 = data in port full
		// Bit 3 = command/data out port full
		return (m_local_status & 0xf1) | (m_aux_status & 0x80) >> 5 | (m_aux_status & 0x04) << 1;

	case 1:
		// Data in port is no longer full
		if (!machine().side_effects_disabled())
			 m_aux_status &= 0x7f;
		return m_data_to_host;

	case 2:
		return m_interrupt_flags;

	default:
		if (!machine().side_effects_disabled())
			logerror("Host read from undocumented register %d\n", offset);
		return 0;
	}
}


//-------------------------------------------------
//  host_w - write one byte to a host register
//-------------------------------------------------

void aic565_device::host_w(offs_t offset, u8 data)
{
	machine().scheduler().synchronize(timer_expired_delegate(FUNC(aic565_device::host_sync_w), this), (offset & 3) << 8 | data);
}


//-------------------------------------------------
//  host_sync_w - synchronization callback for
//  host register writes
//-------------------------------------------------

TIMER_CALLBACK_MEMBER(aic565_device::host_sync_w)
{
	const offs_t offset = param >> 8;
	const u8 data = param & 0xff;

	switch (offset)
	{
	case 0:
		if (BIT(data, 7))
		{
			// Hard reset (self-clearing)
			m_hrst_callback(ASSERT_LINE);
			m_hrst_callback(CLEAR_LINE);
		}
		if (BIT(data, 6))
		{
			// Soft reset (may cause interrupt on local processor)
			m_aux_status |= 0x10;
			m_srst_callback(ASSERT_LINE);
		}
		if (BIT(data, 5))
		{
			// Host interrupt reset
			m_aux_status &= 0xfe;
			m_interrupt_flags = 0;
			m_irq_callback(CLEAR_LINE);
		}
		if (BIT(data, 4))
		{
			// SCSI bus reset
			m_aux_status |= 0x08;
		}
		break;

	case 1:
		// Fill the command/data out port
		m_data_from_host = data;
		m_aux_status |= 0x04;
		break;

	default:
		logerror("Host write to undocumented register %d = %02X\n", offset, data);
		break;
	}
}


//-------------------------------------------------
//  local_r - read one byte from a local register
//-------------------------------------------------

u8 aic565_device::local_r(offs_t offset)
{
	// Local read register 3 is the auxiliary status register
	// Local read register 2 seems to be an undocumented mirror of this
	if (BIT(offset, 1))
		return m_aux_status;
	else if (BIT(offset, 0))
	{
		// Command/data out port is no longer full
		if (!machine().side_effects_disabled())
			m_aux_status &= 0xfb;
		return m_data_from_host;
	}
	else
		return m_interrupt_flags; // also not documented (bits 3 and 0 tested)
}


//-------------------------------------------------
//  local_w - write one byte to a local register
//-------------------------------------------------

void aic565_device::local_w(offs_t offset, u8 data)
{
	machine().scheduler().synchronize(timer_expired_delegate(FUNC(aic565_device::local_sync_w), this), (offset & 3) << 8 | data);
}


//-------------------------------------------------
//  local_sync_w - synchronization callback for
//  local writes
//-------------------------------------------------

TIMER_CALLBACK_MEMBER(aic565_device::local_sync_w)
{
	const offs_t offset = param >> 8;
	const u8 data = param & 0xff;

	switch (offset)
	{
	case 0:
		// Set the local status
		m_local_status = data;
		break;

	case 1:
		// Fill the data in port
		m_data_to_host = data;
		m_aux_status |= 0x80;
		break;

	case 2:
		// Bit 7 = host interrupt active
		if (BIT(m_interrupt_flags, 7) != BIT(data, 7))
		{
			if (BIT(data, 7))
			{
				m_aux_status |= 0x01;
				m_irq_callback(ASSERT_LINE);
			}
			else
			{
				m_aux_status &= 0xfe;
				m_irq_callback(CLEAR_LINE);
			}
		}

		// Other bits of this register are firmware-defined
		m_interrupt_flags = data;
		break;

	case 3:
		// Clear reset (data is irrelevant)
		if (BIT(m_aux_status, 4))
			m_srst_callback(CLEAR_LINE);
		m_aux_status &= 0xe7;
		break;
	}
}
