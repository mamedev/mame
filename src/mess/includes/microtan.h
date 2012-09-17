/******************************************************************************
 *  Microtan 65
 *
 *  variables and function prototypes
 *
 *  Juergen Buchmueller <pullmoll@t-online.de>, Jul 2000
 *
 *  Thanks go to Geoff Macdonald <mail@geoff.org.uk>
 *  for his site http://www.geo255.redhotant.com
 *  and to Fabrice Frances <frances@ensica.fr>
 *  for his site http://www.ifrance.com/oric/microtan.html
 *
 ******************************************************************************/

#ifndef MICROTAN_H_
#define MICROTAN_H_

#include "imagedev/snapquik.h"
#include "machine/6522via.h"


class microtan_state : public driver_device
{
public:
	microtan_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) ,
		m_videoram(*this, "videoram"){ }

	required_shared_ptr<UINT8> m_videoram;
	UINT8 m_chunky_graphics;
	UINT8 *m_chunky_buffer;
	UINT8 m_keypad_column;
	UINT8 m_keyboard_ascii;
	emu_timer *m_timer;
	int m_via_0_irq_line;
	int m_via_1_irq_line;
	int m_kbd_irq_line;
	UINT8 m_keyrows[10];
	int m_lastrow;
	int m_mask;
	int m_key;
	int m_repeat;
	int m_repeater;
	tilemap_t *m_bg_tilemap;
	DECLARE_READ8_MEMBER(microtan_sound_r);
	DECLARE_WRITE8_MEMBER(microtan_sound_w);
	DECLARE_READ8_MEMBER(microtan_bffx_r);
	DECLARE_WRITE8_MEMBER(microtan_bffx_w);
	DECLARE_WRITE8_MEMBER(microtan_videoram_w);
	DECLARE_DRIVER_INIT(microtan);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	virtual void machine_reset();
	virtual void video_start();
	UINT32 screen_update_microtan(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
};


/*----------- defined in machine/microtan.c -----------*/

extern const via6522_interface microtan_via6522_0;
extern const via6522_interface microtan_via6522_1;



SNAPSHOT_LOAD( microtan );
QUICKLOAD_LOAD( microtan_hexfile );

INTERRUPT_GEN( microtan_interrupt );



/*----------- defined in video/microtan.c -----------*/


extern VIDEO_START( microtan );
extern SCREEN_UPDATE_IND16( microtan );


#endif /* MICROTAN_H_ */
