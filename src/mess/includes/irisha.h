/*****************************************************************************
 *
 * includes/irisha.h
 *
 ****************************************************************************/

#ifndef IRISHA_H_
#define IRISHA_H_

#include "emu.h"
#include "cpu/i8085/i8085.h"
#include "machine/i8255.h"
#include "machine/pit8253.h"
#include "machine/pic8259.h"
#include "machine/i8251.h"
#include "sound/speaker.h"

class irisha_state : public driver_device
{
public:
	irisha_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		  m_pit(*this, "pit8253"),
		  m_speaker(*this, SPEAKER_TAG)
		{ }

	required_device<device_t> m_pit;
	required_device<device_t> m_speaker;

	int m_keyboard_mask;
	UINT8 m_keypressed;
	UINT8 m_keyboard_cnt;
	UINT8 m_ppi_porta;
	UINT8 m_ppi_portc;
	int   m_sg1_line;
	DECLARE_READ8_MEMBER(irisha_keyboard_r);
	DECLARE_READ8_MEMBER(irisha_8255_portb_r);
	DECLARE_READ8_MEMBER(irisha_8255_portc_r);
	DECLARE_WRITE8_MEMBER(irisha_8255_porta_w);
	DECLARE_WRITE8_MEMBER(irisha_8255_portb_w);
	DECLARE_WRITE8_MEMBER(irisha_8255_portc_w);
	DECLARE_WRITE_LINE_MEMBER(speaker_w);
	void update_speaker();

	DECLARE_DRIVER_INIT(irisha);
	virtual void machine_start();
	virtual void machine_reset();
	virtual void video_start();
	UINT32 screen_update_irisha(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	TIMER_CALLBACK_MEMBER(irisha_key);
	DECLARE_WRITE_LINE_MEMBER(irisha_pic_set_int_line);
};


/*----------- defined in machine/irisha.c -----------*/

extern const i8255_interface irisha_ppi8255_interface;
extern const struct pit8253_config irisha_pit8253_intf;
extern const struct pic8259_interface irisha_pic8259_config;

#endif /* IRISHA_H_ */
