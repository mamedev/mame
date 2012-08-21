/*****************************************************************************
 *
 * includes/pk8020.h
 *
 ****************************************************************************/

#ifndef PK8020_H_
#define PK8020_H_

#include "machine/i8255.h"
#include "machine/pit8253.h"
#include "machine/pic8259.h"
#include "machine/i8251.h"
#include "imagedev/cassette.h"
#include "sound/speaker.h"
#include "sound/wave.h"

class pk8020_state : public driver_device
{
public:
	pk8020_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	UINT8 m_color;
	UINT8 m_video_page;
	UINT8 m_wide;
	UINT8 m_font;
	UINT8 m_attr;
	UINT8 m_text_attr;
	UINT8 m_takt;
	UINT8 m_video_page_access;
	UINT8 m_portc_data;
	UINT8 m_sound_gate;
	UINT8 m_sound_level;
	DECLARE_READ8_MEMBER(keyboard_r);
	DECLARE_READ8_MEMBER(sysreg_r);
	DECLARE_WRITE8_MEMBER(sysreg_w);
	DECLARE_READ8_MEMBER(text_r);
	DECLARE_WRITE8_MEMBER(text_w);
	DECLARE_READ8_MEMBER(gzu_r);
	DECLARE_WRITE8_MEMBER(gzu_w);
	DECLARE_READ8_MEMBER(devices_r);
	DECLARE_WRITE8_MEMBER(devices_w);
};


/*----------- defined in machine/pk8020.c -----------*/
extern MACHINE_RESET( pk8020 );
extern const i8255_interface pk8020_ppi8255_interface_1;
extern const i8255_interface pk8020_ppi8255_interface_2;
extern const i8255_interface pk8020_ppi8255_interface_3;
extern const struct pit8253_config pk8020_pit8253_intf;
extern const struct pic8259_interface pk8020_pic8259_config;
extern INTERRUPT_GEN( pk8020_interrupt );
/*----------- defined in video/pk8020.c -----------*/

extern PALETTE_INIT( pk8020 );
extern VIDEO_START( pk8020 );
extern SCREEN_UPDATE_IND16( pk8020 );

#endif /* pk8020_H_ */
