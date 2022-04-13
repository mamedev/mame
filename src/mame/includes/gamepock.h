// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
#ifndef MAME_INCLUDES_GAMEPOCK_H
#define MAME_INCLUDES_GAMEPOCK_H

#pragma once

#include "bus/generic/slot.h"
#include "sound/spkrdev.h"

class gamepock_state : public driver_device
{
public:
	gamepock_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_speaker(*this, "speaker"),
		m_cart(*this, "cartslot")
	{ }

	void gamepock(machine_config &config);

private:
	struct HD44102CH {
		uint8_t   enabled = 0U;
		uint8_t   start_page = 0U;
		uint8_t   address = 0U;
		uint8_t   y_inc = 0U;
		uint8_t   ram[256]{};   // There are actually 50 x 4 x 8 bits. This just makes addressing easier.
	};

	virtual void machine_reset() override;

	void hd44102ch_w(int which, int c_d, uint8_t data);
	void hd44102ch_init(int which);
	void lcd_update();

	void port_a_w(uint8_t data);
	uint8_t port_b_r();
	void port_b_w(uint8_t data);
	uint8_t port_c_r();
	uint32_t screen_update_gamepock(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	DECLARE_WRITE_LINE_MEMBER(gamepock_to_w);
	void gamepock_mem(address_map &map);

	uint8_t m_port_a = 0U;
	uint8_t m_port_b = 0U;
	HD44102CH m_hd44102ch[3];

	required_device<cpu_device> m_maincpu;
	required_device<speaker_sound_device> m_speaker;
	required_device<generic_slot_device> m_cart;
};

#endif // MAME_INCLUDES_GAMEPOCK_H
