/*****************************************************************************
 *
 * includes/poly88.h
 *
 ****************************************************************************/

#ifndef POLY88_H_
#define POLY88_H_

#include "machine/i8251.h"
#include "imagedev/snapquik.h"

class poly88_state : public driver_device
{
public:
	poly88_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) ,
		m_video_ram(*this, "video_ram"){ }

	required_shared_ptr<UINT8> m_video_ram;
	UINT8 *m_FNT;
	UINT8 m_intr;
	UINT8 m_last_code;
	UINT8 m_int_vector;
	emu_timer * m_cassette_timer;
	emu_timer * m_usart_timer;
	int m_previous_level;
	int m_clk_level;
	int m_clk_level_tape;
	DECLARE_WRITE8_MEMBER(poly88_baud_rate_w);
	DECLARE_READ8_MEMBER(poly88_keyboard_r);
	DECLARE_WRITE8_MEMBER(poly88_intr_w);
	DECLARE_DRIVER_INIT(poly88);
	virtual void machine_reset();
	virtual void video_start();
	UINT32 screen_update_poly88(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(poly88_interrupt);
	TIMER_CALLBACK_MEMBER(poly88_usart_timer_callback);
	TIMER_CALLBACK_MEMBER(keyboard_callback);
	TIMER_CALLBACK_MEMBER(poly88_cassette_timer_callback);
	TIMER_CALLBACK_MEMBER(setup_machine_state);
	DECLARE_WRITE_LINE_MEMBER(poly88_usart_rxready);
};


/*----------- defined in machine/poly88.c -----------*/
extern const i8251_interface poly88_usart_interface;

extern SNAPSHOT_LOAD( poly88 );

#endif /* POLY88_H_ */
