// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
#ifndef _GAMEPOCK_H_
#define _GAMEPOCK_H_
#include "sound/speaker.h"
#include "bus/generic/slot.h"

struct HD44102CH {
	UINT8   enabled;
	UINT8   start_page;
	UINT8   address;
	UINT8   y_inc;
	UINT8   ram[256];   /* There are actually 50 x 4 x 8 bits. This just makes addressing easier. */
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

	UINT8 m_port_a;
	UINT8 m_port_b;
	HD44102CH m_hd44102ch[3];

	void hd44102ch_w( int which, int c_d, UINT8 data );
	void hd44102ch_init( int which );
	void lcd_update();

	DECLARE_WRITE8_MEMBER( port_a_w );
	DECLARE_READ8_MEMBER( port_b_r );
	DECLARE_WRITE8_MEMBER( port_b_w );
	DECLARE_READ8_MEMBER( port_c_r );
	UINT32 screen_update_gamepock(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	required_device<cpu_device> m_maincpu;
	required_device<speaker_sound_device> m_speaker;
	required_device<generic_slot_device> m_cart;
	DECLARE_WRITE_LINE_MEMBER(gamepock_to_w);
};

#endif
