// license:BSD-3-Clause
// copyright-holders:Olivier Galibert

#include "emu.h"
#include "applepp.h"
#include "storagehle.h"

DEFINE_DEVICE_TYPE(APPLEPP_CONNECTOR, applepp_connector, "applepp_connector", "Apple parallel connector")

applepp_connector::applepp_connector(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, APPLEPP_CONNECTOR, tag, owner, clock),
	device_single_card_slot_interface<device_applepp_interface>(mconfig, *this),
	m_write_pd(*this),
	m_write_pchk(*this),
	m_write_pbsy(*this),
	m_write_pparity(*this)
{
}

applepp_connector::~applepp_connector()
{
}

void applepp_connector::device_start()
{
}

void applepp_connector::device_reset()
{
	if(!get_card_device()) {
		m_write_pchk(1);
		m_write_pbsy(1);
		m_write_pparity(1);
		m_write_pd(1);
	}
}

void applepp_connector::pd_w(u8 data)
{
	auto card = get_card_device();
	if(card)
		card->pd_w(data);
}

void applepp_connector::pstrb_w(int level)
{
	auto card = get_card_device();
	if(card)
		card->pstrb_w(level);
}

void applepp_connector::prw_w(int level)
{
	auto card = get_card_device();
	if(card)
		card->prw_w(level);
}

void applepp_connector::pcmd_w(int level)
{
	auto card = get_card_device();
	if(card)
		card->pcmd_w(level);
}

void applepp_connector::pres_w(int level)
{
	auto card = get_card_device();
	if(card)
		card->pres_w(level);
}

void applepp_connector::pd_set(u8 data)
{
	m_write_pd(data);
	m_write_pparity((population_count_32(data) + 1) & 1);
}

void applepp_connector::pbsy_set(int state)
{
	m_write_pbsy(state);
}

void applepp_connector::pchk_set(int state)
{
	m_write_pchk(state);
}


device_applepp_interface::device_applepp_interface(const machine_config &mconfig, device_t &device) :
	device_interface(device, "applepp")
{
}

void device_applepp_interface::interface_pre_start()
{
	m_connector = downcast<applepp_connector *>(device().owner());
}

void applepp_intf(device_slot_interface &device)
{
	device.option_add("profile", APPLEPP_PROFILE);
	device.option_add("widget",  APPLEPP_WIDGET);
}
