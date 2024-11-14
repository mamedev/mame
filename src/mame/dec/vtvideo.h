// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic,Karl-Ludwig Deisenhofer
/**********************************************************************

DEC VT Terminal video emulation
[ DC012 and DC011 emulation ]

01/05/2009 Initial implementation [Miodrag Milanovic]

**********************************************************************/

#ifndef MAME_DEC_VTVIDEO_H
#define MAME_DEC_VTVIDEO_H

#pragma once

#include "emupal.h"


class vt100_video_device : public device_t,
	public device_video_interface
{
public:
	vt100_video_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	auto ram_rd_callback() { return m_read_ram.bind(); }
	auto vert_freq_intr_wr_callback() { return m_write_vert_freq_intr.bind(); }
	auto lba3_lba4_wr_callback() { return m_write_lba3_lba4.bind(); }
	auto lba7_wr_callback() { return m_write_lba7.bind(); }

	template <typename T> void set_chargen(T &&tag) { m_char_rom.set_tag(std::forward<T>(tag)); }

	int lba7_r();
	void dc012_w(offs_t offset, uint8_t data);
	void dc011_w(uint8_t data);
	void brightness_w(uint8_t data);

	virtual void video_update(bitmap_ind16 &bitmap, const rectangle &cliprect);

protected:
	vt100_video_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	// internal state
	void recompute_parameters();
	void vblank_callback(screen_device &screen, bool state);
	void display_char(bitmap_ind16 &bitmap, uint8_t code, int x, int y, uint8_t scroll_region, uint8_t display_type, bool invert, bool bold, bool blink, bool underline, bool blank);
	TIMER_CALLBACK_MEMBER(lba3_change);
	TIMER_CALLBACK_MEMBER(lba7_change);
	virtual void notify_vblank(bool choice) { }

	devcb_read8        m_read_ram;
	devcb_write_line   m_write_vert_freq_intr;
	devcb_write8       m_write_lba3_lba4;
	devcb_write_line   m_write_lba7;

	int m_lba7 = 0;

	bool MHFU_FLAG = false;
	int MHFU_counter = 0;

	// dc012 attributes
	uint8_t m_scroll_latch = 0;
	bool m_scroll_latch_valid = false;
	uint8_t m_blink_flip_flop = 0;
	uint8_t m_reverse_field = 0;
	uint8_t m_basic_attribute = 0;
	// dc011 attributes
	uint8_t m_columns = 0;
	uint8_t m_height = 0;
	uint8_t m_height_MAX = 0;
	uint8_t m_fill_lines = 0;
	bool m_is_50hz = false;
	bool m_interlaced = false;
	emu_timer *m_lba3_change_timer = nullptr;
	emu_timer *m_lba7_change_timer = nullptr;

	required_region_ptr<uint8_t> m_char_rom; /* character rom region */
	required_device<palette_device> m_palette;

	bool m_notify_vblank = false;
	int m_last_scroll = 0;

	bool m_linedoubler = false;
};


class rainbow_video_device : public vt100_video_device
{
public:
	rainbow_video_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void video_update(bitmap_ind16 &bitmap, const rectangle &cliprect) override;
	virtual void video_blanking(bitmap_ind16 &bitmap, const rectangle &cliprect);

	int MHFU(int);
	void palette_select(int choice);

protected:
	virtual void notify_vblank(bool choice) override;
	virtual void device_reset() override ATTR_COLD;
};

DECLARE_DEVICE_TYPE(VT100_VIDEO, vt100_video_device)
DECLARE_DEVICE_TYPE(RAINBOW_VIDEO, rainbow_video_device)


#endif // MAME_DEC_VTVIDEO_H
