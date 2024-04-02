// license:BSD-3-Clause
// copyright-holders:Andrei I. Holub
/**********************************************************************
    Spectrum Next Multiface
**********************************************************************/

#include "emu.h"
#include "specnext_multiface.h"


// device type definition
DEFINE_DEVICE_TYPE(SPECNEXT_MULTIFACE, specnext_multiface_device, "specnext_multiface", "Spectrum Next Multiface")


specnext_multiface_device::specnext_multiface_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, SPECNEXT_MULTIFACE, tag, owner, clock)
{
}


bool specnext_multiface_device::mode_48()
{
	return m_mf_mode == 0b11;
}

bool specnext_multiface_device::mode_128()
{
	return !(mode_p3() || mode_48());
}

bool specnext_multiface_device::mode_p3()
{
	return m_mf_mode == 0b00;
}

bool specnext_multiface_device::button_pulse()
{
	return m_button && !m_nmi_active;
}

bool specnext_multiface_device::invisible_eff()
{
	return m_invisible && !mode_48();
}

bool specnext_multiface_device::fetch_66()
{
	return m_cpu_a_0066 && !m_cpu_m1_n && m_nmi_active;
}

bool specnext_multiface_device::mf_enable_eff()
{
	return m_mf_enable || fetch_66();
}

void specnext_multiface_device::clock_w()
{
	// Rising edge
	bool port_io_dly = m_port_mf_enable_rd || m_port_mf_enable_wr || m_port_mf_disable_rd || m_port_mf_disable_wr;

	if (button_pulse())
		m_invisible = 0;
	else if (((m_port_mf_disable_wr && !mode_p3()) || (m_port_mf_enable_wr && mode_p3())) && !port_io_dly)
		m_invisible = 1;

	if (button_pulse())
		m_nmi_active = 1;
	else if (m_cpu_retn_seen || ((m_port_mf_enable_wr || m_port_mf_disable_wr || (m_port_mf_disable_rd && mode_p3())) && !port_io_dly))
		m_nmi_active = 0;

	if (fetch_66() && !m_cpu_mreq_n)
		m_mf_enable = 1;
	else if (m_port_mf_disable_rd || m_cpu_retn_seen)
		m_mf_enable = 0;
	else if (m_port_mf_enable_rd)
		m_mf_enable = !invisible_eff();
}

void specnext_multiface_device::device_start()
{
	save_item(NAME(m_cpu_a_0066));
	save_item(NAME(m_cpu_mreq_n));
	save_item(NAME(m_cpu_m1_n));
	save_item(NAME(m_cpu_retn_seen));
	save_item(NAME(m_enable));
	save_item(NAME(m_button));
	save_item(NAME(m_mf_mode));
	save_item(NAME(m_port_mf_enable_rd));
	save_item(NAME(m_port_mf_enable_wr));
	save_item(NAME(m_port_mf_disable_rd));
	save_item(NAME(m_port_mf_disable_wr));
	save_item(NAME(m_nmi_active));
	save_item(NAME(m_invisible));
	save_item(NAME(m_mf_enable));
}

void specnext_multiface_device::device_reset()
{
	m_cpu_a_0066 = 0;
	m_cpu_mreq_n = 0;;
	m_cpu_m1_n = 1;
	m_cpu_retn_seen = 0;
	m_enable = 1;
	m_button = 0;
	m_mf_mode = 0;
	m_port_mf_enable_rd = 0;
	m_port_mf_enable_wr = 0;
	m_port_mf_disable_rd = 0;
	m_port_mf_disable_wr = 0;

	m_nmi_active = 0;
	m_invisible = 1;
	m_mf_enable = 0;
}
