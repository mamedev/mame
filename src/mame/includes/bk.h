// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/*****************************************************************************
 *
 * includes/bk.h
 *
 ****************************************************************************/
#ifndef MAME_INCLUDES_BK_H
#define MAME_INCLUDES_BK_H

#pragma once

#include "cpu/t11/t11.h"
#include "imagedev/cassette.h"

class bk_state : public driver_device
{
public:
	bk_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_vram(*this, "videoram")
		, m_maincpu(*this, "maincpu")
		, m_cassette(*this, "cassette")
		, m_io_keyboard(*this, "LINE%u", 0U)
	{ }

	void bk0010(machine_config &config);
	void bk0010fd(machine_config &config);

private:
	uint16_t m_scroll = 0U;
	uint16_t m_kbd_state = 0U;
	uint16_t m_key_code = 0U;
	uint16_t m_key_pressed = 0U;
	uint16_t m_key_irq_vector = 0U;
	uint16_t m_drive = 0U;
	emu_timer *m_kbd_timer = nullptr;
	uint16_t key_state_r();
	uint16_t key_code_r();
	uint16_t vid_scroll_r();
	uint16_t key_press_r();
	void key_state_w(uint16_t data);
	void vid_scroll_w(uint16_t data);
	void key_press_w(uint16_t data);
	uint16_t floppy_cmd_r();
	void floppy_cmd_w(uint16_t data);
	uint16_t floppy_data_r();
	void floppy_data_w(uint16_t data);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	TIMER_CALLBACK_MEMBER(keyboard_callback);
	uint8_t irq_callback(offs_t offset);
	required_shared_ptr<uint16_t> m_vram;
	required_device<t11_device> m_maincpu;
	required_device<cassette_image_device> m_cassette;
	required_ioport_array<12> m_io_keyboard;
	void bk0010_mem(address_map &map);
	void bk0010fd_mem(address_map &map);
};

#endif // MAME_INCLUDES_BK_H
