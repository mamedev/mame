// license: BSD-3-Clause
// copyright-holders: Dirk Best, Luca Elia
/***************************************************************************

    SN54/74165

    8-Bit Parallel-In/Serial-Out Shift Register

***************************************************************************/

#include "emu.h"
#include "74165.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(TTL165, ttl165_device, "ttl165", "SN54/74165")

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void ttl165_device::device_add_mconfig(machine_config &config)
{
	TIMER(config, m_timer).configure_generic(FUNC(ttl165_device::qh_output));
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  ttl153_device - constructor
//-------------------------------------------------

ttl165_device::ttl165_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, TTL165, tag, owner, clock),
	m_timer(*this, "timer"),
	m_data_cb(*this), m_qh_cb(*this),
	m_data(0x00),
	m_ser(0), m_clk(0), m_shld(0)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void ttl165_device::device_start()
{
	// resolve callbacks
	m_data_cb.resolve_safe(0x00);
	m_qh_cb.resolve_safe();

	// register for save states
	save_item(NAME(m_data));
	save_item(NAME(m_ser));
	save_item(NAME(m_clk));
	save_item(NAME(m_shld));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void ttl165_device::device_reset()
{
	m_data = 0x00;
	m_ser = 0;
	m_clk = 0;
	m_shld = 0;
}


//**************************************************************************
//  IMPLEMENTATION
//**************************************************************************

TIMER_DEVICE_CALLBACK_MEMBER( ttl165_device::qh_output )
{
	m_qh_cb(param);
}


//**************************************************************************
//  INTERFACE
//**************************************************************************

WRITE_LINE_MEMBER( ttl165_device::serial_w )
{
	m_ser = state;
}

void ttl165_device::update_qh()
{
	// we need to delay the output a bit to allow for serial input
	m_timer->adjust(attotime::from_nsec(25), BIT(m_data, 7));
}

WRITE_LINE_MEMBER( ttl165_device::clock_w )
{
	if (m_shld && !m_clk && state)
	{
		// shift next bit
		m_data <<= 1;
		m_data |= m_ser;

		update_qh();
	}

	m_clk = state;
}

WRITE_LINE_MEMBER( ttl165_device::shift_load_w )
{
	if (!m_shld || !state)
	{
		// load external data
		m_data = m_data_cb(0);

		update_qh(); // FIXME: Qh should be updated continuosly while SH//LD is low
	}
	m_shld = state;
}
