// license:BSD-3-Clause
// copyright-holders:David Haywood

// UIO seems to be a different IO module to the vt1682_io.cpp one, 8-bit ports instead of 4-bit

#include "emu.h"
#include "vt1682_uio.h"

#define LOG_UIO     (1U << 1)

#define LOG_ALL           ( LOG_UIO )

#define VERBOSE             (0)
#include "logmacro.h"


DEFINE_DEVICE_TYPE(VT_VT1682_UIO, vrt_vt1682_uio_device, "vt1682uio", "VRT VT1682 UIO")

vrt_vt1682_uio_device::vrt_vt1682_uio_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, VT_VT1682_UIO, tag, owner, clock),
	m_porta_out(*this),
	m_porta_in(*this),
	m_portb_out(*this),
	m_portb_in(*this)
{
}

uint8_t vrt_vt1682_uio_device::inteact_2129_uio_a_data_r()
{
	// TODO, use direction register etc.
	uint8_t dat = m_porta_in();
	//logerror("%s: bankswitch inteact_2129_uio_a_data_r %02x\n", machine().describe_context(), m_2129_uio_a_data);
	return dat;
}


void vrt_vt1682_uio_device::inteact_2129_uio_a_data_w(uint8_t data)
{
	// TODO, use direction register etc.

	LOGMASKED(LOG_UIO, "%s: inteact_2129_uio_a_data_w %02x\n", machine().describe_context(), data);
	m_2129_uio_a_data = data;
	m_porta_out(m_2129_uio_a_data);
}

uint8_t vrt_vt1682_uio_device::inteact_212a_uio_a_direction_r()
{
	return m_212a_uio_a_direction;
}

void vrt_vt1682_uio_device::inteact_212a_uio_a_direction_w(uint8_t data)
{
	LOGMASKED(LOG_UIO, "%s: inteact_212a_uio_a_direction_w %02x\n", machine().describe_context(), data);
	m_212a_uio_a_direction = data;
}

uint8_t vrt_vt1682_uio_device::inteact_212b_uio_a_attribute_r()
{
	return m_212b_uio_a_attribute;
}

void vrt_vt1682_uio_device::inteact_212b_uio_a_attribute_w(uint8_t data)
{
	LOGMASKED(LOG_UIO, "%s: inteact_212b_uio_a_attribute_w %02x\n", machine().describe_context(), data);
	m_212b_uio_a_attribute = data;
}


uint8_t vrt_vt1682_uio_device::inteact_2149_uio_b_data_r()
{
	// TODO, use direction register etc.
	uint8_t dat = m_portb_in();
	//logerror("%s: bankswitch inteact_2149_uio_b_data_r %02x\n", machine().describe_context(), m_2149_uio_b_data);
	return dat;
}


void vrt_vt1682_uio_device::inteact_2149_uio_b_data_w(uint8_t data)
{
	// TODO, use direction register etc.
	LOGMASKED(LOG_UIO, "%s: inteact_2149_uio_b_data_w %02x\n", machine().describe_context(), data);
	m_2149_uio_b_data = data;
	m_portb_out(m_2149_uio_b_data);
}

uint8_t vrt_vt1682_uio_device::inteact_214a_uio_b_direction_r()
{
	return m_214a_uio_b_direction;
}

void vrt_vt1682_uio_device::inteact_214a_uio_b_direction_w(uint8_t data)
{
	LOGMASKED(LOG_UIO, "%s: inteact_214a_uio_b_direction_w %02x\n", machine().describe_context(), data);
	m_214a_uio_b_direction = data;
}

uint8_t vrt_vt1682_uio_device::inteact_214b_uio_b_attribute_r()
{
	return m_214b_uio_b_attribute;
}

void vrt_vt1682_uio_device::inteact_214b_uio_b_attribute_w(uint8_t data)
{
	LOGMASKED(LOG_UIO, "%s: inteact_214b_uio_b_attribute_w %02x\n", machine().describe_context(), data);
	m_214b_uio_b_attribute = data;
}



void vrt_vt1682_uio_device::device_start()
{
	m_porta_out.resolve_safe();
	m_porta_in.resolve_safe(0);
	m_portb_out.resolve_safe();
	m_portb_in.resolve_safe(0);

	save_item(NAME(m_2129_uio_a_data));
	save_item(NAME(m_212a_uio_a_direction));
	save_item(NAME(m_212b_uio_a_attribute));
	save_item(NAME(m_2149_uio_b_data));
	save_item(NAME(m_214a_uio_b_direction));
	save_item(NAME(m_214b_uio_b_attribute));
}

void vrt_vt1682_uio_device::device_reset()
{
	m_2129_uio_a_data = 0;
	m_212a_uio_a_direction = 0;
	m_212b_uio_a_attribute = 0;
	m_2149_uio_b_data = 0;
	m_214a_uio_b_direction = 0;
	m_214b_uio_b_attribute = 0;
}
