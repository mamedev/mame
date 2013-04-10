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
		: driver_device(mconfig, type, tag),
		m_sapi_video_ram(*this, "sapi_video_ram"),
		m_bank1(*this, "bank1"),
		m_line0(*this, "LINE0"),
		m_line1(*this, "LINE1"),
		m_line2(*this, "LINE2"),
		m_line3(*this, "LINE3"),
		m_line4(*this, "LINE4") { }

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
	DECLARE_MACHINE_START(sapi1);
	DECLARE_MACHINE_RESET(sapi1);
	DECLARE_VIDEO_START(sapi1);
	DECLARE_MACHINE_RESET(sapizps3);
	DECLARE_VIDEO_START(sapizps3);
	UINT32 screen_update_sapi1(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_sapizps3(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

protected:
	optional_memory_bank m_bank1;   // Only for sapi3
	required_ioport m_line0;
	required_ioport m_line1;
	required_ioport m_line2;
	required_ioport m_line3;
	required_ioport m_line4;
};

#endif
