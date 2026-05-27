// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/***************************************************************************

  HLE emulation of Profile & Widget

***************************************************************************/

#ifndef MAME_BUS_APPLEPP_STORAGEHLE_H
#define MAME_BUS_APPLEPP_STORAGEHLE_H

#pragma once

#include "applepp.h"

class applepp_storage_device : public device_t, public device_applepp_interface
{
public:
	virtual void pd_w(u8 data) override;

	virtual void pstrb_w(int level) override;
	virtual void prw_w(int level) override;
	virtual void pcmd_w(int level) override;
	virtual void pres_w(int level) override;

protected:
	enum {
		IDLE,
		WAIT_CMD,
	};

	applepp_storage_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	void device_start() override;
	void device_reset() override;

	int m_strb, m_rw, m_cmd, m_res;
	u32 m_state;
	u8 m_pd;
};

class applepp_profile_device : public applepp_storage_device
{
public:
	applepp_profile_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};

class applepp_widget_device : public applepp_storage_device
{
public:
	applepp_widget_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};


DECLARE_DEVICE_TYPE(APPLEPP_PROFILE, applepp_profile_device)
DECLARE_DEVICE_TYPE(APPLEPP_WIDGET,  applepp_widget_device)

#endif // MAME_BUS_APPLEPP_STORAGEHLE_H
