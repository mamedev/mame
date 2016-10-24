// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
#ifndef _GAMEPOCK_H_
#define _GAMEPOCK_H_
#include "sound/speaker.h"
#include "bus/generic/slot.h"

struct HD44102CH {
	uint8_t   enabled;
	uint8_t   start_page;
	uint8_t   address;
	uint8_t   y_inc;
	uint8_t   ram[256];   /* There are actually 50 x 4 x 8 bits. This just makes addressing easier. */
};

class gamepock_state : public driver_device
{
public:
	gamepock_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_speaker(*this, "speaker"),
		m_cart(*this, "cartslot")
		{ }

	virtual void machine_reset() override;

	uint8_t m_port_a;
	uint8_t m_port_b;
	HD44102CH m_hd44102ch[3];

	void hd44102ch_w( int which, int c_d, uint8_t data );
	void hd44102ch_init( int which );
	void lcd_update();

	void port_a_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t port_b_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void port_b_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t port_c_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint32_t screen_update_gamepock(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	required_device<cpu_device> m_maincpu;
	required_device<speaker_sound_device> m_speaker;
	required_device<generic_slot_device> m_cart;
	void gamepock_to_w(int state);
};

#endif
