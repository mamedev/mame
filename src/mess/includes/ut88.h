/*****************************************************************************
 *
 * includes/ut88.h
 *
 ****************************************************************************/

#ifndef UT88_H_
#define UT88_H_

#include "emu.h"
#include "cpu/i8085/i8085.h"
#include "sound/dac.h"
#include "sound/wave.h"
#include "machine/i8255.h"
#include "imagedev/cassette.h"


class ut88_state : public driver_device
{
public:
	ut88_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
	m_cass(*this, CASSETTE_TAG),
	m_ppi(*this, "ppi8255"),
	m_dac(*this, "dac"),
	m_p_videoram(*this, "p_videoram")
	{ }

	required_device<cassette_image_device> m_cass;
	optional_device<i8255_device> m_ppi;
	optional_device<dac_device> m_dac;
	DECLARE_READ8_MEMBER(ut88_keyboard_r);
	DECLARE_WRITE8_MEMBER(ut88_keyboard_w);
	DECLARE_WRITE8_MEMBER(ut88_sound_w);
	DECLARE_READ8_MEMBER(ut88_tape_r);
	DECLARE_READ8_MEMBER(ut88mini_keyboard_r);
	DECLARE_WRITE8_MEMBER(ut88mini_write_led);
	DECLARE_READ8_MEMBER(ut88_8255_portb_r);
	DECLARE_READ8_MEMBER(ut88_8255_portc_r);
	DECLARE_WRITE8_MEMBER(ut88_8255_porta_w);
	optional_shared_ptr<UINT8> m_p_videoram;
	int m_keyboard_mask;
	int m_lcd_digit[6];
	DECLARE_DRIVER_INIT(ut88);
	DECLARE_DRIVER_INIT(ut88mini);
	DECLARE_MACHINE_RESET(ut88);
	DECLARE_VIDEO_START(ut88);
	DECLARE_MACHINE_START(ut88mini);
	DECLARE_MACHINE_RESET(ut88mini);
	UINT32 screen_update_ut88(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
};


/*----------- defined in machine/ut88.c -----------*/

extern const i8255_interface ut88_ppi8255_interface;

/*----------- defined in video/ut88.c -----------*/

extern const gfx_layout ut88_charlayout;


#endif /* UT88_H_ */
