// license:BSD-3-Clause
// copyright-holders:Sandro Ronco
/***************************************************************************

        VTech Laser PC4

****************************************************************************/

#pragma once

#ifndef _PC4_H_
#define _PC4_H_


#include "sound/beep.h"

class pc4_state : public driver_device
{
public:
	pc4_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_beep(*this, "beeper"),
		m_region_charset(*this, "charset"),
		m_rombank(*this, "rombank") { }

	required_device<cpu_device> m_maincpu;
	required_device<beep_device> m_beep;

	UINT32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	virtual void machine_start();

	DECLARE_WRITE8_MEMBER( beep_w );
	DECLARE_WRITE8_MEMBER( bank_w );
	DECLARE_READ8_MEMBER( kb_r );

	//LCD controller
	void update_ac(void);
	void set_busy_flag(UINT16 usec);
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr);

	DECLARE_WRITE8_MEMBER(lcd_control_w);
	DECLARE_READ8_MEMBER(lcd_control_r);
	DECLARE_WRITE8_MEMBER(lcd_data_w);
	DECLARE_READ8_MEMBER(lcd_data_r);
	DECLARE_WRITE8_MEMBER( lcd_offset_w );

	static const device_timer_id BUSY_TIMER = 0;
	static const device_timer_id BLINKING_TIMER = 1;

	emu_timer *m_blink_timer;
	emu_timer *m_busy_timer;

	UINT8 m_busy_flag;
	UINT8 m_ddram[0xa0];
	UINT8 m_cgram[0x40];
	INT16 m_ac;
	UINT8 m_ac_mode;
	UINT8 m_data_bus_flag;
	INT16 m_cursor_pos;
	UINT8 m_display_on;
	UINT8 m_cursor_on;
	UINT8 m_blink_on;
	UINT8 m_shift_on;
	INT8 m_disp_shift;
	INT8 m_direction;
	UINT8 m_blink;
	DECLARE_PALETTE_INIT(pc4);

protected:
	required_memory_region m_region_charset;
	required_memory_bank m_rombank;
	ioport_port *io_port[8];
};

#endif  // _PC4_H_
