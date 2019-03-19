// license:BSD-3-Clause
// copyright-holders:David Haywood

// are these still part of the ADC? if so merge into xavix_adc.cpp

#include "emu.h"
#include "xavix_anport.h"

#define VERBOSE 0
#include "logmacro.h"

DEFINE_DEVICE_TYPE(XAVIX_ANPORT, xavix_anport_device, "xavix_anport", "XaviX Analog ports")

xavix_anport_device::xavix_anport_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, XAVIX_ANPORT, tag, owner, clock)
	, m_in0_cb(*this)
	, m_in1_cb(*this)
	, m_in2_cb(*this)
	, m_in3_cb(*this)
{
}

void xavix_anport_device::device_start()
{
	m_in0_cb.resolve_safe(0xff);
	m_in1_cb.resolve_safe(0xff);
	m_in2_cb.resolve_safe(0xff);
	m_in3_cb.resolve_safe(0xff);
}

void xavix_anport_device::device_reset()
{
}


READ8_MEMBER(xavix_anport_device::mouse_7b00_r)
{
	//printf("%s: mouse_7b00_r\n", machine().describe_context().c_str());
	return m_in0_cb();

}

READ8_MEMBER(xavix_anport_device::mouse_7b01_r)
{
	//printf("%s: mouse_7b01_r\n", machine().describe_context().c_str());
	return m_in1_cb();
}

READ8_MEMBER(xavix_anport_device::mouse_7b10_r)
{
	//printf("%s: mouse_7b10_r\n", machine().describe_context().c_str());
	return m_in2_cb();
}

READ8_MEMBER(xavix_anport_device::mouse_7b11_r)
{
	//printf("%s: mouse_7b11_r\n", machine().describe_context().c_str());
	return m_in3_cb();
}

WRITE8_MEMBER(xavix_anport_device::mouse_7b00_w)
{
	//printf("%s: mouse_7b00_w %02x\n", machine().describe_context().c_str(), data);
}

WRITE8_MEMBER(xavix_anport_device::mouse_7b01_w)
{
	//printf("%s: mouse_7b01_w %02x\n", machine().describe_context().c_str(), data);
}

WRITE8_MEMBER(xavix_anport_device::mouse_7b10_w)
{
	//printf("%s: mouse_7b10_w %02x\n", machine().describe_context().c_str(), data);
}

WRITE8_MEMBER(xavix_anport_device::mouse_7b11_w)
{
	//printf("%s: mouse_7b11_w %02x\n", machine().describe_context().c_str(), data);
}


