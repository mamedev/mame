// license:BSD-3-Clause
// copyright-holders:Phil Bennett
/***************************************************************************

    Fairlight CMI Music Keyboard

***************************************************************************/

#ifndef MAME_MACHINE_CMI_MKBD_H
#define MAME_MACHINE_CMI_MKBD_H

#pragma once

#include "machine/6821pia.h"
#include "machine/6850acia.h"
#include "video/dl1416.h"

class cmi_music_keyboard_device : public device_t
{
	static const device_timer_id TIMER_CMI10_SCND = 0;

public:
	cmi_music_keyboard_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

	auto cmi_txd_handler() { return m_cmi_txd.bind(); }
	auto cmi_rts_handler() { return m_cmi_rts.bind(); }
	auto kbd_txd_handler() { return m_kbd_txd.bind(); }
	auto kbd_rts_handler() { return m_kbd_rts.bind(); }

	DECLARE_WRITE_LINE_MEMBER( cmi_rxd_w );
	DECLARE_WRITE_LINE_MEMBER( cmi_cts_w );
	DECLARE_WRITE_LINE_MEMBER( kbd_rxd_w );
	DECLARE_WRITE_LINE_MEMBER( kbd_cts_w );

protected:
	virtual const tiny_rom_entry *device_rom_region() const override;
	virtual void device_add_mconfig(machine_config &config) override;
	virtual ioport_constructor device_input_ports() const override;
	virtual void device_resolve_objects() override;
	virtual void device_start() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

private:
	DECLARE_WRITE8_MEMBER( cmi10_u20_a_w );
	DECLARE_WRITE8_MEMBER( cmi10_u20_b_w );
	DECLARE_READ_LINE_MEMBER( cmi10_u20_cb1_r );
	DECLARE_WRITE_LINE_MEMBER( cmi10_u20_cb2_w );
	DECLARE_WRITE_LINE_MEMBER( cmi10_u21_cb2_w );
	DECLARE_READ8_MEMBER( cmi10_u21_a_r );

	DECLARE_WRITE_LINE_MEMBER( kbd_acia_int );
	DECLARE_WRITE_LINE_MEMBER( cmi_acia_int );

	DECLARE_WRITE_LINE_MEMBER( cmi_txd_w );
	DECLARE_WRITE_LINE_MEMBER( cmi_rts_w );
	DECLARE_WRITE_LINE_MEMBER( kbd_txd_w );
	DECLARE_WRITE_LINE_MEMBER( kbd_rts_w );

	template <unsigned N> DECLARE_WRITE16_MEMBER( update_dp );

	void muskeys_map(address_map &map);

	devcb_write_line m_cmi_txd;
	devcb_write_line m_cmi_rts;
	devcb_write_line m_kbd_txd;
	devcb_write_line m_kbd_rts;

	required_device<cpu_device> m_cpu;
	required_device<acia6850_device> m_acia_kbd;
	required_device<acia6850_device> m_acia_cmi;
	required_device<pia6821_device> m_cmi10_pia_u20;
	required_device<pia6821_device> m_cmi10_pia_u21;
	required_device<dl1416_device> m_dp1;
	required_device<dl1416_device> m_dp2;
	required_device<dl1416_device> m_dp3;

	required_ioport m_keypad_a_port;
	required_ioport m_keypad_b_port;

	required_ioport_array<3> m_key_mux_ports[4];

	output_finder<12> m_digit;

	emu_timer *m_cmi10_scnd_timer;

	int     m_kbd_acia_irq;
	int     m_cmi_acia_irq;

	u8      m_scnd;
};

DECLARE_DEVICE_TYPE(CMI_MUSIC_KEYBOARD, cmi_music_keyboard_device)

#endif // MAME_MACHINE_CMI_MKBD_H
