// license:BSD-3-Clause
// copyright-holders:AJR

#ifndef MAME_SHARED_EFO_SOUND3_H
#define MAME_SHARED_EFO_SOUND3_H

#pragma once

#include "machine/cdp1852.h"
#include "sound/tms5220.h"


class efo_sound3_device : public device_t
{
public:
	// construction/destruction
	efo_sound3_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

	void input_w(u8 data);
	void clock_w(int state);

protected:
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;

private:
	void efo90435_mem(address_map &map) ATTR_COLD;
	void efo90435_io(address_map &map) ATTR_COLD;

	u8 input_r();
	void intf_cs_w(int state);

	required_device<cdp1852_device> m_inputlatch;
	required_device<cdp1852_device> m_intflatch;
	required_device<tms5220_device> m_tms;

	u8 m_input;
};

DECLARE_DEVICE_TYPE(EFO_SOUND3, efo_sound3_device)

#endif // MAME_SHARED_EFO_SOUND3_H
