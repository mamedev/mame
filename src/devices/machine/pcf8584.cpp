// license:BSD-3-Clause
// copyright-holders:AJR
/**********************************************************************

    Philips PCF8584 I²C Bus Controller

    This is a comprehensive protocol interface chip for Philips' I²C
    serial bus, supporting master and slave modes for both receiving
    and transmitting data. The register interface is similar but not
    identical to that provided by certain Philips 80C51-derived
    microcontrollers.

    TODO: actually implement the protocol

**********************************************************************/

#include "emu.h"
#include "pcf8584.h"

#define VERBOSE 1
#include "logmacro.h"


//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

// device type definition
DEFINE_DEVICE_TYPE(PCF8584, pcf8584_device, "pcf8584", "PCF8584 I2C Bus Controller")


//**************************************************************************
//  LOOKUP TABLES
//**************************************************************************

// prescalers based on 1.5 MHz internal frequency
const u32 pcf8584_device::s_prescaler[4] = { 50, 100, 400, 3000 };

// dividers for clock input
const u32 pcf8584_device::s_divider[8] = { 2, 2, 2, 2, 3, 4, 5, 8 };

// debugging strings
const char *const pcf8584_device::s_nominal_clock[8] = { "3", "3", "3", "3", "4.43", "6", "8", "12" };
const char *const pcf8584_device::s_bus_function[4] = { "NOP", "stop", "start", "data chaining" };


//**************************************************************************
//  DEVICE CONSTRUCTION AND INITIALIZATION
//**************************************************************************

//-------------------------------------------------
//  pcf8584_device - constructor
//-------------------------------------------------

pcf8584_device::pcf8584_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, PCF8584, tag, owner, clock)
	, m_sda_callback(*this)
	, m_sda_out_callback(*this)
	, m_scl_callback(*this)
	, m_int_callback(*this)
	, m_68k_bus(false)
	, m_s0_data_buffer(0)
	, m_s0_shift_register(0)
	, m_s0_own_address(0)
	, m_s1_status(0)
	, m_s1_control(0)
	, m_s2_clock(0)
	, m_s3_vector(0)
{
}


//-------------------------------------------------
//  device_resolve_objects - resolve objects that
//  may be needed for other devices to set
//  initial conditions at start time
//-------------------------------------------------

void pcf8584_device::device_resolve_objects()
{
	m_sda_callback.resolve_safe(1);
	m_sda_out_callback.resolve_safe();
	m_scl_callback.resolve_safe();
	m_int_callback.resolve_safe();
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void pcf8584_device::device_start()
{
	// register internal state
	save_item(NAME(m_s0_data_buffer));
	save_item(NAME(m_s0_shift_register));
	save_item(NAME(m_s0_own_address));
	save_item(NAME(m_s1_status));
	save_item(NAME(m_s1_control));
	save_item(NAME(m_s2_clock));
	save_item(NAME(m_s3_vector));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void pcf8584_device::device_reset()
{
	// reset all flags except for PIN
	m_s1_status = 0x80;
	m_s1_control = 0x00;

	// vector isn't actually set to 0x0f until R/_W is pulled low without _CS, but who's counting?
	m_s3_vector = m_68k_bus ? 0x0f : 0x00;
}


//**************************************************************************
//  REGISTER INTERFACE
//**************************************************************************

//-------------------------------------------------
//  set_shift_register - write data to S0
//-------------------------------------------------

void pcf8584_device::set_shift_register(u8 data)
{
	LOG("%s: Shift register = %s%02XH\n", machine().describe_context(), data >= 0xa0 ? "0" : "", data);
	m_s0_shift_register = data;
}


//-------------------------------------------------
//  set_own_address - write data to S0'
//-------------------------------------------------

void pcf8584_device::set_own_address(u8 data)
{
	LOG("%s: Own address = %s%02XH\n", machine().describe_context(), (data & 0x7f) >= 0x60 ? "0" : "", (data << 1) & 0xfe);
	m_s0_own_address = data;
}


//-------------------------------------------------
//  set_control - write data to S1
//-------------------------------------------------

void pcf8584_device::set_control(u8 data)
{
	if (BIT(data, 7) != BIT(m_s1_status, 7))
	{
		LOG("%s: PIN %sactive\n", machine().describe_context(), BIT(data, 7) ? "in" : "");
		if (BIT(data, 7))
			m_s1_status = 0x80;
		else
			m_s1_status &= 0x7f;
	}

	if (BIT(data, 6) != BIT(m_s1_control, 6))
		LOG("%s: Serial output %sabled%s\n", machine().describe_context(), BIT(data, 6) ? "en" : "dis",
			(data & 0x60) == 0x60 ? " (4-wire)" : "");
	if (BIT(data, 3) != BIT(m_s1_control, 3))
		LOG("%s: Interrupt output %sabled\n", machine().describe_context(), BIT(data, 3) ? "en" : "dis");
	if ((data & 0x06) != (m_s1_control & 0x06))
		LOG("%s: Bus function = %s\n", machine().describe_context(), s_bus_function[(data & 0x06) >> 1]);
	if (BIT(data, 0) != BIT(m_s1_control, 0))
		LOG("%s: Acknowledge pulse %sabled\n", machine().describe_context(), BIT(data, 0) ? "en" : "dis");

	m_s1_control = data & 0x7f;
}


//-------------------------------------------------
//  set_clock_frequency - write data to S2
//-------------------------------------------------

void pcf8584_device::set_clock_frequency(u8 data)
{
	LOG("%s: SCL frequency = %.2f kHz (fCLK is nominally %s MHz)\n", machine().describe_context(),
		(clocks_to_attotime(s_divider[(data & 0x1c) >> 2] * s_prescaler[data & 0x03]) / 3).as_hz() / 1000.0,
		s_nominal_clock[(data & 0x1c) >> 2]);
	m_s2_clock = data & 0x1f;
}


//-------------------------------------------------
//  set_vector - write data to S3
//-------------------------------------------------

void pcf8584_device::set_vector(u8 data)
{
	LOG("%s: Interrupt vector = %s%02XH\n", machine().describe_context(), data >= 0xa0 ? "0" : "", data);
	m_s3_vector = data;
}


//-------------------------------------------------
//  read - input from register to bus
//-------------------------------------------------

u8 pcf8584_device::read(offs_t offset)
{
	if (BIT(offset, 0))
		return eso() ? m_s1_status : m_s1_control | (m_s1_status & 0x80);
	else if (es2() && !es1())
		return m_s3_vector;
	else if (eso())
		return m_s0_data_buffer;
	else if (es1())
		return m_s2_clock;
	else
		return m_s0_own_address;
}


//-------------------------------------------------
//  write - output from bus to register
//-------------------------------------------------

void pcf8584_device::write(offs_t offset, u8 data)
{
	if (BIT(offset, 0))
		set_control(data);
	else if (es2() && !es1())
		set_vector(data);
	else if (eso())
		set_shift_register(data);
	else if (es1())
		set_clock_frequency(data);
	else
		set_own_address(data);

}


//-------------------------------------------------
//  iack - acknowledge interrupt with vector
//-------------------------------------------------

IRQ_CALLBACK_MEMBER(pcf8584_device::iack)
{
	return m_s3_vector;
}
