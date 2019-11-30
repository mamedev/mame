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
		, m_bk0010_video_ram(*this, "video_ram")
		, m_maincpu(*this, "maincpu")
		, m_cassette(*this, "cassette")
		, m_io_keyboard(*this, "LINE%u", 0)
	{ }

	void bk0010(machine_config &config);
	void bk0010fd(machine_config &config);

private:
	uint16_t m_scrool;
	required_shared_ptr<uint16_t> m_bk0010_video_ram;
	uint16_t m_kbd_state;
	uint16_t m_key_code;
	uint16_t m_key_pressed;
	uint16_t m_key_irq_vector;
	uint16_t m_drive;
	emu_timer *m_kbd_timer;
	DECLARE_READ16_MEMBER(bk_key_state_r);
	DECLARE_READ16_MEMBER(bk_key_code_r);
	DECLARE_READ16_MEMBER(bk_vid_scrool_r);
	DECLARE_READ16_MEMBER(bk_key_press_r);
	DECLARE_WRITE16_MEMBER(bk_key_state_w);
	DECLARE_WRITE16_MEMBER(bk_vid_scrool_w);
	DECLARE_WRITE16_MEMBER(bk_key_press_w);
	DECLARE_READ16_MEMBER(bk_floppy_cmd_r);
	DECLARE_WRITE16_MEMBER(bk_floppy_cmd_w);
	DECLARE_READ16_MEMBER(bk_floppy_data_r);
	DECLARE_WRITE16_MEMBER(bk_floppy_data_w);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	uint32_t screen_update_bk0010(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	TIMER_CALLBACK_MEMBER(keyboard_callback);
	IRQ_CALLBACK_MEMBER(bk0010_irq_callback);
	required_device<t11_device> m_maincpu;
	required_device<cassette_image_device> m_cassette;
	required_ioport_array<12> m_io_keyboard;
	void bk0010_mem(address_map &map);
	void bk0010fd_mem(address_map &map);
};

#endif // MAME_INCLUDES_BK_H
