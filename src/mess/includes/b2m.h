/*****************************************************************************
 *
 * includes/b2m.h
 *
 ****************************************************************************/

#ifndef B2M_H_
#define B2M_H_

#include "machine/i8255.h"
#include "machine/pit8253.h"
#include "machine/pic8259.h"
#include "sound/speaker.h"
#include "sound/wave.h"

class b2m_state : public driver_device
{
public:
	b2m_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	UINT8 m_b2m_8255_porta;
	UINT8 m_b2m_video_scroll;
	UINT8 m_b2m_8255_portc;

	UINT8 m_b2m_video_page;
	UINT8 m_b2m_drive;
	UINT8 m_b2m_side;

	UINT8 m_b2m_romdisk_lsb;
	UINT8 m_b2m_romdisk_msb;

	UINT8 m_b2m_color[4];
	UINT8 m_b2m_localmachine;
	UINT8 m_vblank_state;

	/* devices */
	device_t *m_fdc;
	device_t *m_pic;
	device_t *m_speaker;
	DECLARE_READ8_MEMBER(b2m_keyboard_r);
	DECLARE_WRITE8_MEMBER(b2m_palette_w);
	DECLARE_READ8_MEMBER(b2m_palette_r);
	DECLARE_WRITE8_MEMBER(b2m_localmachine_w);
	DECLARE_READ8_MEMBER(b2m_localmachine_r);
	DECLARE_DRIVER_INIT(b2m);
	virtual void machine_start();
	virtual void machine_reset();
	virtual void video_start();
	virtual void palette_init();
};

/*----------- defined in machine/b2m.c -----------*/

extern const struct pit8253_config b2m_pit8253_intf;
extern const struct pic8259_interface b2m_pic8259_config;

extern const i8255_interface b2m_ppi8255_interface_1;
extern const i8255_interface b2m_ppi8255_interface_2;
extern const i8255_interface b2m_ppi8255_interface_3;

extern MACHINE_START( b2m );
extern MACHINE_RESET( b2m );
extern INTERRUPT_GEN( b2m_vblank_interrupt );

/*----------- defined in video/b2m.c -----------*/

extern VIDEO_START( b2m );
extern SCREEN_UPDATE_IND16( b2m );
extern PALETTE_INIT( b2m );

#endif
