// license:BSD-3-Clause
// copyright-holders:Olivier Galibert

#include "emu.h"
#include "storagehle.h"

DEFINE_DEVICE_TYPE(APPLEPP_PROFILE, applepp_profile_device, "profilehle", "Apple Profile drive HLE")
DEFINE_DEVICE_TYPE(APPLEPP_WIDGET,  applepp_widget_device,  "widgethle",  "Apple Widget drive HLE")

void applepp_storage_device::device_start()
{
	save_item(NAME(m_strb));
	save_item(NAME(m_rw));
	save_item(NAME(m_cmd));
	save_item(NAME(m_res));
	save_item(NAME(m_state));
	save_item(NAME(m_pd));

	m_strb = m_rw = m_cmd = m_res = 1;
	m_pd = 0;
}

void applepp_storage_device::device_reset()
{
	m_connector->pchk_set(0);
	m_connector->pbsy_set(1);
	m_connector->pd_set(0xff);

	m_state = IDLE;
}

void applepp_storage_device::pd_w(u8 data)
{
	if(m_pd == data)
		return;
	m_pd = data;

	logerror("pd_w %02x (%s)\n", data, machine().describe_context());
}

void applepp_storage_device::pstrb_w(int level)
{
	if(m_strb == level)
		return;
	m_strb = level;

	logerror("pstrb_w %02x (%s)\n", level, machine().describe_context());
}

void applepp_storage_device::prw_w(int level)
{
	if(m_rw == level)
		return;
	m_rw = level;

	logerror("prw_w %02x (%s)\n", level, machine().describe_context());
}

void applepp_storage_device::pcmd_w(int level)
{
	if(m_cmd == level)
		return;
	m_cmd = level;

	logerror("pcmd_w %02x (%s)\n", level, machine().describe_context());

	if(!m_cmd && m_state == IDLE) {
		m_state = WAIT_CMD;
		m_connector->pbsy_set(0);
		m_connector->pd_set(0x01);
		return;
	}

	if(m_cmd && m_state == WAIT_CMD) {
		m_connector->pd_set(0xff);
		logerror("command %02x ?\n", m_pd);
		m_state = IDLE;
		m_connector->pbsy_set(1);
	}
}

void applepp_storage_device::pres_w(int level)
{
	if(m_res == level)
		return;
	m_res = level;

	logerror("pres_w %02x (%s)\n", level, machine().describe_context());
}

applepp_storage_device::applepp_storage_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, type, tag, owner, clock),
	  device_applepp_interface(mconfig, *this)
{
}

applepp_profile_device::applepp_profile_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: applepp_storage_device(mconfig, APPLEPP_PROFILE, tag, owner, clock)
{
}

applepp_widget_device::applepp_widget_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: applepp_storage_device(mconfig, APPLEPP_WIDGET, tag, owner, clock)
{
}
