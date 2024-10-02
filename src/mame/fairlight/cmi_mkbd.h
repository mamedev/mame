// license:BSD-3-Clause
// copyright-holders:Phil Bennett
/***************************************************************************

    Fairlight CMI Music Keyboard

***************************************************************************/

#ifndef MAME_FAIRLIGHT_CMI_MKBD_H
#define MAME_FAIRLIGHT_CMI_MKBD_H

#pragma once

#include "machine/6821pia.h"
#include "machine/6850acia.h"
#include "video/dl1416.h"

class cmi_music_keyboard_device : public device_t
{
public:
	cmi_music_keyboard_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

	auto cmi_txd_handler() { return m_cmi_txd.bind(); }
	auto cmi_rts_handler() { return m_cmi_rts.bind(); }
	auto kbd_txd_handler() { return m_kbd_txd.bind(); }
	auto kbd_rts_handler() { return m_kbd_rts.bind(); }

	void cmi_rxd_w(int state);
	void cmi_cts_w(int state);
	void kbd_rxd_w(int state);
	void kbd_cts_w(int state);

	DECLARE_INPUT_CHANGED_MEMBER(key_changed);

	enum : u32
	{
		KEY_F0,
		KEY_F0S,
		KEY_G0,
		KEY_G0S,
		KEY_A1,
		KEY_A1S,
		KEY_B1,
		KEY_C1,
		KEY_C1S,
		KEY_D1,
		KEY_D1S,
		KEY_E1,
		KEY_F1,
		KEY_F1S,
		KEY_G1,
		KEY_G1S,
		KEY_A2,
		KEY_A2S,
		KEY_B2,
		KEY_C2,
		KEY_C2S,
		KEY_D2,
		KEY_D2S,
		KEY_E2,
		KEY_F2,
		KEY_F2S,
		KEY_G2,
		KEY_G2S,
		KEY_A3,
		KEY_A3S,
		KEY_B3,
		KEY_C3,
		KEY_C3S,
		KEY_D3,
		KEY_D3S,
		KEY_E3,
		KEY_F3,
		KEY_F3S,
		KEY_G3,
		KEY_G3S,
		KEY_A4,
		KEY_A4S,
		KEY_B4,
		KEY_C4,
		KEY_C4S,
		KEY_D4,
		KEY_D4S,
		KEY_E4,
		KEY_F4,
		KEY_F4S,
		KEY_G4,
		KEY_G4S,
		KEY_A5,
		KEY_A5S,
		KEY_B5,
		KEY_C5,
		KEY_C5S,
		KEY_D5,
		KEY_D5S,
		KEY_E5,
		KEY_F5,
		KEY_F5S,
		KEY_G5,
		KEY_G5S,
		KEY_A6,
		KEY_A6S,
		KEY_B6,
		KEY_C6,
		KEY_C6S,
		KEY_D6,
		KEY_D6S,
		KEY_E6,
		KEY_F6,

		KEY_COUNT
	};

protected:
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual void device_resolve_objects() override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;

	TIMER_CALLBACK_MEMBER(scnd_update);
	TIMER_CALLBACK_MEMBER(velkey_down);

private:
	void kbd_acia_w(offs_t offset, u8 data);
	u8 kbd_acia_r(offs_t offset);

	void cmi10_u20_a_w(u8 data);
	void cmi10_u20_b_w(u8 data);
	int cmi10_u20_cb1_r();
	void cmi10_u20_cb2_w(int state);
	void cmi10_u21_cb2_w(int state);
	u8 cmi10_u21_a_r();

	u32 get_key_for_indices(int mux, int module, int key);

	void kbd_acia_int(int state);
	void cmi_acia_int(int state);

	void cmi_txd_w(int state);
	void cmi_rts_w(int state);
	void kbd_txd_w(int state);
	void kbd_rts_w(int state);

	template <unsigned N> void update_dp(offs_t offset, u16 data);

	void muskeys_map(address_map &map) ATTR_COLD;

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
	required_ioport m_analog;

	required_ioport_array<3> m_key_mux_ports[4];

	output_finder<12> m_digit;

	emu_timer *m_cmi10_scnd_timer;
	emu_timer *m_velocity_timers[KEY_COUNT];
	bool m_key_held[KEY_COUNT];

	int     m_kbd_acia_irq;
	int     m_cmi_acia_irq;

	u8      m_scnd;
};

DECLARE_DEVICE_TYPE(CMI_MUSIC_KEYBOARD, cmi_music_keyboard_device)

#endif // MAME_FAIRLIGHT_CMI_MKBD_H
