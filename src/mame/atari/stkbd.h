// license:BSD-3-Clause
// copyright-holders:Curt Coder, Olivier Galibert

#ifndef MAME_ATARI_STKBD_H
#define MAME_ATARI_STKBD_H

// Atari ST family keyboard implementation

// Composed of a hd6301 that scans the keyboard, manages the two atari
// joystick ports (one for mouse or joystick, one for joystick).
// Communicates with the main cpu through serial.

#include "cpu/m6800/m6801.h"

class st_kbd_device : public device_t {
public:
	st_kbd_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	void tx_w(int state);
	auto rx_cb() { return m_rx_cb.bind(); }

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

private:
	enum {
		PHASE_STATIC,
		PHASE_POSITIVE,
		PHASE_NEGATIVE
	};

	static const int mouse_xya[3][4];
	static const int mouse_xyb[3][4];

	required_device<hd6301_cpu_device> m_cpu;
	output_finder<> m_led;
	devcb_write_line m_rx_cb;
	required_ioport_array<16> m_keys;
	required_ioport_array<2> m_joy;
	required_ioport m_mousex;
	required_ioport m_mousey;
	required_ioport m_mouseb;
	required_ioport m_config;

	uint16_t m_keylatch;
	uint8_t m_mouse;
	uint8_t m_mouse_x;
	uint8_t m_mouse_y;
	uint8_t m_mouse_px;
	uint8_t m_mouse_py;
	uint8_t m_mouse_pc;
	int m_tx;
	int m_joy_disabled;
	emu_timer *m_mouse_timer;

	TIMER_CALLBACK_MEMBER(mouse_tick);

	uint8_t port1_r();
	uint8_t port2_r();
	void port2_w(uint8_t data);
	void port3_w(uint8_t data);
	uint8_t port4_r();
	void port4_w(uint8_t data);
};

DECLARE_DEVICE_TYPE(ST_KBD, st_kbd_device)

#endif
