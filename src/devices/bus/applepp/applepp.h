// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/***************************************************************************

  Apple parallel port devices (Profile, Widget...)

***************************************************************************/

#ifndef MAME_BUS_APPLEPP_APPLEPP_H
#define MAME_BUS_APPLEPP_APPLEPP_H

#pragma once

class device_applepp_interface;

class applepp_connector: public device_t, public device_single_card_slot_interface<device_applepp_interface>
{
public:
	template <typename T>
	applepp_connector(const machine_config &mconfig, const char *tag, device_t *owner, T &&opts, const char *dflt, bool fixed = false)
		: applepp_connector(mconfig, tag, owner, (uint32_t)0)
	{
		option_reset();
		opts(*this);
		set_default_option(dflt);
		set_fixed(fixed);
	}

	applepp_connector(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	virtual ~applepp_connector();

	auto write_pd() { return m_write_pd.bind(); }
	void pd_w(u8 data);

	void pstrb_w(int level);
	void prw_w(int level);
	void pcmd_w(int level);
	void pres_w(int level);

	auto write_pchk()    { return m_write_pchk.bind(); }
	auto write_pbsy()    { return m_write_pbsy.bind(); }
	auto write_pparity() { return m_write_pparity.bind(); }

	void pd_set(u8 data);
	void pbsy_set(int level);
	void pchk_set(int level);

protected:
	devcb_write8 m_write_pd;
	devcb_write_line m_write_pchk, m_write_pbsy, m_write_pparity;

	virtual void device_start() override;
	virtual void device_reset() override;
};

class device_applepp_interface : public device_interface
{
public:
	virtual void pd_w(u8 data) = 0;

	virtual void pstrb_w(int level) = 0;
	virtual void prw_w(int level) = 0;
	virtual void pcmd_w(int level) = 0;
	virtual void pres_w(int level) = 0;

protected:
	device_applepp_interface(const machine_config &mconfig, device_t &device);

	virtual void interface_pre_start() override;

	applepp_connector *m_connector;
};

DECLARE_DEVICE_TYPE(APPLEPP_CONNECTOR, applepp_connector)

void applepp_intf(device_slot_interface &device);

#endif // MAME_BUS_APPLEPP_APPLEPP_H
