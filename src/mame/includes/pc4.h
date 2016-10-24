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

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	virtual void machine_start() override;

	void beep_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void bank_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t kb_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);

	//LCD controller
	void update_ac(void);
	void set_busy_flag(uint16_t usec);
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

	void lcd_control_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t lcd_control_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void lcd_data_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t lcd_data_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void lcd_offset_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

	static const device_timer_id BUSY_TIMER = 0;
	static const device_timer_id BLINKING_TIMER = 1;

	emu_timer *m_blink_timer;
	emu_timer *m_busy_timer;

	uint8_t m_busy_flag;
	uint8_t m_ddram[0xa0];
	uint8_t m_cgram[0x40];
	int16_t m_ac;
	uint8_t m_ac_mode;
	uint8_t m_data_bus_flag;
	int16_t m_cursor_pos;
	uint8_t m_display_on;
	uint8_t m_cursor_on;
	uint8_t m_blink_on;
	uint8_t m_shift_on;
	int8_t m_disp_shift;
	int8_t m_direction;
	uint8_t m_blink;
	void palette_init_pc4(palette_device &palette);

protected:
	required_memory_region m_region_charset;
	required_memory_bank m_rombank;
	ioport_port *io_port[8];
};

#endif  // _PC4_H_
