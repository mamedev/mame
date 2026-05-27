// license:BSD-3-Clause
// copyright-holders:Devin Acker
/**********************************************************************

    Xetec C-5181 serial printer interface

**********************************************************************/

#ifndef MAME_BUS_CBMIEC_C5181_H
#define MAME_BUS_CBMIEC_C5181_H

#pragma once

#include "cbmiec.h"
#include "bus/rs232/rs232.h"
#include "cpu/m6805/m68705.h"

class xetec_c5181_device : public device_t, public device_cbm_iec_interface
{
public:
	xetec_c5181_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	void map(address_map &map) ATTR_COLD;

	u8 pa_r();
	void pa_w(u8);

	required_device<m146805e2_device> m_cpu;
	required_device<rs232_port_device> m_rs232;
};

// device type definition
DECLARE_DEVICE_TYPE(XETEC_C5181, xetec_c5181_device)

#endif // MAME_BUS_CBMIEC_C5181_H
