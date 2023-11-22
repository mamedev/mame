// license:BSD-3-Clause
// copyright-holders:Frode van der Meeren
/***************************************************************************

	Tandberg TDV-2115 Terminal module, "Display Logic" version

****************************************************************************/

#ifndef MAME_TANDBERG_DISP_LOGIC_H
#define MAME_TANDBERG_DISP_LOGIC_H

#pragma once

#include "machine/clock.h"
#include "machine/ram.h"
#include "sound/beep.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"

class tandberg_disp_logic_device : public device_t
{
public:
	tandberg_disp_logic_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

protected:
	virtual void device_add_mconfig(machine_config &config) override;
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual const tiny_rom_entry *device_rom_region() const override;

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	required_device<screen_device>  	m_screen;
	required_device<palette_device> 	m_palette;
	required_memory_region          	m_font;
	memory_share_creator<uint8_t>   	m_vram;
	required_device<beep_device>		m_beep;

private:
	uint8_t attribute;
	int frame_counter;
	bool blank_ctrl_chars;
	bool blank_attr_chars;
	bool underline_mode;
	bool extend_dot7;
	bool full_inverse_video;
	bool video_enable;

	int cursor_x;
	int cursor_y;
	bool block_cursor;
	bool blinking_cursor;
	bool auto_cr_lf;
	bool auto_roll_up;
	bool underline_input;
	bool cursor_x_input;
	bool cursor_y_input;

	bool speed_check;
	emu_timer *m_beep_trigger;
	emu_timer *m_speed_ctrl;
	TIMER_CALLBACK_MEMBER(expire_speed_check);
	TIMER_CALLBACK_MEMBER(end_beep);

	void data_to_display(uint8_t byte);
	void char_to_display(uint8_t byte);
	void advance_cursor();
};

// device type definition
DECLARE_DEVICE_TYPE(TANDBERG_DISPLAY_LOGIC, tandberg_disp_logic_device)

#endif // MAME_TANDBERG_DISP_LOGIC_H
