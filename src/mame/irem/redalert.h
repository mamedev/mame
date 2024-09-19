// license:BSD-3-Clause
// copyright-holders:Mike Balfour
/***************************************************************************

    Irem M27 hardware

    If you have any questions about how this driver works, don't hesitate to
    ask.  - Mike Balfour (mab22@po.cwru.edu)

    To Do:
    - Device-ify video hardware.

****************************************************************************/

#ifndef MAME_IREM_REDALERT_H
#define MAME_IREM_REDALERT_H

#pragma once

#include "screen.h"

class redalert_state : public driver_device
{
public:
	redalert_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_bitmap_videoram(*this, "bitmap_videoram"),
		m_charmap_videoram(*this, "charram"),
		m_video_control(*this, "video_control"),
		m_bitmap_color(*this, "bitmap_color"),
		m_maincpu(*this, "maincpu"),
		m_screen(*this, "screen")
	{
	}

	void redalert_video_common(machine_config &config);
	void redalert_video(machine_config &config);
	void ww3_video(machine_config &config);
	void panther_video(machine_config &config);
	void demoneye_video(machine_config &config);
	void demoneye(machine_config &config);
	void ww3(machine_config &config);
	void panther(machine_config &config);
	void redalert(machine_config &config);

	DECLARE_INPUT_CHANGED_MEMBER(coin_inserted);

private:
	required_shared_ptr<uint8_t> m_bitmap_videoram;
	required_shared_ptr<uint8_t> m_charmap_videoram;
	required_shared_ptr<uint8_t> m_video_control;
	required_shared_ptr<uint8_t> m_bitmap_color;

	required_device<cpu_device> m_maincpu;
	required_device<screen_device> m_screen;

	std::unique_ptr<uint8_t[]> m_bitmap_colorram;
	uint8_t m_control_xor = 0;
	uint8_t redalert_interrupt_clear_r();
	void redalert_interrupt_clear_w(uint8_t data);
	uint8_t panther_interrupt_clear_r();
	void redalert_bitmap_videoram_w(offs_t offset, uint8_t data);
	DECLARE_VIDEO_START(redalert);
	DECLARE_VIDEO_START(ww3);
	DECLARE_VIDEO_START(demoneye);
	uint32_t screen_update_redalert(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_demoneye(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_panther(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(redalert_vblank_interrupt);
	void get_redalert_pens(pen_t *pens);
	void get_panther_pens(pen_t *pens);
	void get_demoneye_pens(pen_t *pens);
	void demoneye_bitmap_layer_w(offs_t offset, uint8_t data);
	void demoneye_bitmap_ypos_w(u8 data);

	void redalert_main_map(address_map &map) ATTR_COLD;
	void ww3_main_map(address_map &map) ATTR_COLD;
	void panther_main_map(address_map &map) ATTR_COLD;
	void demoneye_main_map(address_map &map) ATTR_COLD;

	u8 m_demoneye_bitmap_reg[4];
	u8 m_demoneye_bitmap_yoffs = 0;
};

#endif // MAME_IREM_REDALERT_H
