// license:GPL-2.0+
// copyright-holders:Felipe Sanches

// KN7000 front panel: three boards, left (CPL), centre (CPC) and right (CPR).

#ifndef MAME_MATSUSHITA_KN7000_CPANEL_H
#define MAME_MATSUSHITA_KN7000_CPANEL_H

#pragma once

#include "kn_cpanel.h"

class kn7000_cpanel_device : public kn_cpanel_base_device
{
public:
	kn7000_cpanel_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

protected:
	// device_t implementation
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

	// kn_cpanel_base_device implementation
	virtual void panel_led_frame(u8 addr, u8 data) override;

private:
	output_finder<512> m_cpl_leds;
	output_finder<256> m_cpc_leds;
	output_finder<512> m_cpr_leds;
};

DECLARE_DEVICE_TYPE(KN7000_CPANEL, kn7000_cpanel_device)

#endif // MAME_MATSUSHITA_KN7000_CPANEL_H
