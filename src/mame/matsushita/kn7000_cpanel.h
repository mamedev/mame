// license:GPL2+
// copyright-holders:Felipe Sanches

// KN7000 control panel.

#ifndef MAME_MATSUSHITA_KN7000_CPANEL_H
#define MAME_MATSUSHITA_KN7000_CPANEL_H

#pragma once

#include "kn_cpanel.h"

class kn7000_cpanel_device : public kn_cpanel_base_device
{
public:
	kn7000_cpanel_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	// Global-effect ON state, read from the LED frames the firmware sends the panel.
	virtual bool chorus_led() const override { return m_chorus_led; }
	virtual bool multi_led() const override { return m_multi_led; }

protected:
	// device_t overrides
	virtual void device_start() override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;   // the CP{board}_SEG{col} button matrix

	// per-model geometry (see kn_cpanel.h)
	virtual int     num_scan_ports() const override { return 22; }
	virtual uint8_t scan_port_read(int port) override { return m_phys[port].read_safe(0); }
	virtual uint8_t port_seg(int port) const override;
	virtual int     num_segs() const override { return 0x21; }
	virtual uint8_t seg_wire_addr(int seg) const override;
	virtual void    panel_led_frame(uint8_t addr, uint8_t data) override;

private:
	// Global-effect LED shadow (set in panel_led_frame; see chorus_led()/multi_led()).
	bool m_chorus_led = false;          // D1082 CHORUS (cpr_led29)
	bool m_multi_led  = false;          // D1054 MULTI (cpr_led28)

	// Button scan-matrix ports -- OWNED by this device (declared in device_input_ports(),
	// bound by tag in the constructor). One per physical board SEG column.
	optional_ioport_array<22> m_phys;      // CP{board}_SEG{col}

	// LED outputs (the three panel PCBs: left / center / right).
	output_finder<512> m_cpl_leds;
	output_finder<256> m_cpc_leds;
	output_finder<512> m_cpr_leds;
};

DECLARE_DEVICE_TYPE(KN7000_CPANEL, kn7000_cpanel_device)

#endif // MAME_MATSUSHITA_KN7000_CPANEL_H
