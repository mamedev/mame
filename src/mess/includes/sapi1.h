/*****************************************************************************
 *
 * includes/sapi1.h
 *
 ****************************************************************************/

#ifndef SAPI_1_H_
#define SAPI_1_H_

#include "emu.h"
#include "cpu/i8085/i8085.h"
#include "cpu/z80/z80.h"
#include "machine/ram.h"
#include "machine/keyboard.h"
#include "machine/terminal.h"


class sapi1_state : public driver_device
{
public:
	sapi1_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) ,
		m_sapi_video_ram(*this, "sapi_video_ram"){ }

	required_shared_ptr<UINT8> m_sapi_video_ram;
	UINT8 m_keyboard_mask;
	UINT8 m_refresh_counter;
	UINT8 m_zps3_25;
	DECLARE_READ8_MEMBER(sapi1_keyboard_r);
	DECLARE_WRITE8_MEMBER(sapi1_keyboard_w);
	DECLARE_READ8_MEMBER(sapi2_keyboard_status_r);
	DECLARE_READ8_MEMBER(sapi2_keyboard_data_r);
	DECLARE_WRITE8_MEMBER(sapi3_00_w);
	DECLARE_READ8_MEMBER(sapi3_25_r);
	DECLARE_WRITE8_MEMBER(sapi3_25_w);
	DECLARE_WRITE8_MEMBER(kbd_put);
	UINT8 m_term_data;
	DECLARE_DRIVER_INIT(sapi1);
	DECLARE_DRIVER_INIT(sapizps3);
};


/*----------- defined in machine/sapi1.c -----------*/

extern MACHINE_START( sapi1 );
extern MACHINE_RESET( sapi1 );
extern MACHINE_RESET( sapizps3 );


/*----------- defined in video/sapi1.c -----------*/

extern VIDEO_START( sapi1 );
extern SCREEN_UPDATE_IND16( sapi1 );
extern VIDEO_START( sapizps3 );
extern SCREEN_UPDATE_IND16( sapizps3 );

#endif
