/*****************************************************************************
 *
 * includes/mikro80.h
 *
 ****************************************************************************/

#ifndef MIKRO80_H_
#define MIKRO80_H_

#include "machine/i8255.h"

class mikro80_state : public driver_device
{
public:
	mikro80_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) ,
		m_cursor_ram(*this, "cursor_ram"),
		m_video_ram(*this, "video_ram"){ }

	required_shared_ptr<UINT8> m_cursor_ram;
	required_shared_ptr<UINT8> m_video_ram;
	int m_keyboard_mask;
	int m_key_mask;
	DECLARE_READ8_MEMBER(mikro80_8255_portb_r);
	DECLARE_READ8_MEMBER(mikro80_8255_portc_r);
	DECLARE_WRITE8_MEMBER(mikro80_8255_porta_w);
	DECLARE_WRITE8_MEMBER(mikro80_8255_portc_w);
	DECLARE_READ8_MEMBER(mikro80_keyboard_r);
	DECLARE_WRITE8_MEMBER(mikro80_keyboard_w);
	DECLARE_WRITE8_MEMBER(mikro80_tape_w);
	DECLARE_READ8_MEMBER(mikro80_tape_r);
	DECLARE_DRIVER_INIT(radio99);
	DECLARE_DRIVER_INIT(mikro80);
	virtual void machine_reset();
	virtual void video_start();
	UINT32 screen_update_mikro80(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
};


/*----------- defined in machine/mikro80.c -----------*/

extern const i8255_interface mikro80_ppi8255_interface;

extern MACHINE_RESET( mikro80 );

/*----------- defined in video/mikro80.c -----------*/

extern VIDEO_START( mikro80 );
extern SCREEN_UPDATE_IND16( mikro80 );

#endif /* UT88_H_ */
