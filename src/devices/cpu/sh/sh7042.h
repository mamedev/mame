// license:BSD-3-Clause
// copyright-holders:Olivier Galibert

// SH7042, sh2 variant

#ifndef MAME_CPU_SH_SH7042_H
#define MAME_CPU_SH_SH7042_H

#pragma once

#include "sh2.h"
#include "sh_adc.h"

class sh7042_device : public sh2_device
{
public:
	sh7042_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	template<int Port> auto read_adc() { return m_read_adc[Port].bind(); }

	u16 do_read_adc(int port) { return m_read_adc[port](); }

protected:
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_add_mconfig(machine_config &config) override;

private:
	required_device<sh_adc_device> m_adc;

	devcb_read16::array<8> m_read_adc;

	u16 adc_default(int adc);

	void map(address_map &map);
};

DECLARE_DEVICE_TYPE(SH7042, sh7042_device)

#endif // MAME_CPU_SH_SH7042_H
