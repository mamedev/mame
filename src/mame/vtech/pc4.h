// license:BSD-3-Clause
// copyright-holders:Sandro Ronco
/***************************************************************************

        VTech Laser PC4

****************************************************************************/
#ifndef MAME_VTECH_PC4_H
#define MAME_VTECH_PC4_H

#pragma once


#include "sound/beep.h"
#include "emupal.h"

class pc4_state : public driver_device
{
public:
	pc4_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_beep(*this, "beeper"),
		m_region_charset(*this, "charset"),
		m_rombank(*this, "rombank")
	{ }

	void pc4(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;

	TIMER_CALLBACK_MEMBER(clear_busy_flag);
	TIMER_CALLBACK_MEMBER(blink_tick);

private:
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void beep_w(uint8_t data);
	void bank_w(uint8_t data);
	uint8_t kb_r(offs_t offset);

	//LCD controller
	void update_ac();
	void set_busy_flag(uint16_t usec);

	void lcd_control_w(uint8_t data);
	uint8_t lcd_control_r();
	void lcd_data_w(uint8_t data);
	uint8_t lcd_data_r();
	void lcd_offset_w(uint8_t data);

	void pc4_palette(palette_device &palette) const;

	void pc4_io(address_map &map) ATTR_COLD;
	void pc4_mem(address_map &map) ATTR_COLD;

	emu_timer *m_blink_timer = nullptr;
	emu_timer *m_busy_timer = nullptr;

	uint8_t m_busy_flag = 0;
	uint8_t m_ddram[0xa0]{};
	uint8_t m_cgram[0x40]{};
	int16_t m_ac = 0;
	uint8_t m_ac_mode = 0;
	uint8_t m_data_bus_flag = 0;
	int16_t m_cursor_pos = 0;
	uint8_t m_display_on = 0;
	uint8_t m_cursor_on = 0;
	uint8_t m_blink_on = 0;
	uint8_t m_shift_on = 0;
	int8_t m_disp_shift = 0;
	int8_t m_direction = 0;
	uint8_t m_blink = 0;

	required_device<cpu_device> m_maincpu;
	required_device<beep_device> m_beep;

	required_memory_region m_region_charset;
	required_memory_bank m_rombank;
	ioport_port *io_port[8];
};

#endif // MAME_VTECH_PC4_H
