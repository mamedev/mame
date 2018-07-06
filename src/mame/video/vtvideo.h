// license:GPL-2.0+
// copyright-holders:Miodrag Milanovic,Karl-Ludwig Deisenhofer
/**********************************************************************

DEC VT Terminal video emulation
[ DC012 and DC011 emulation ]

01/05/2009 Initial implementation [Miodrag Milanovic]

**********************************************************************/

#ifndef MAME_VIDEO_VTVIDEO_H
#define MAME_VIDEO_VTVIDEO_H

#pragma once

#include "emupal.h"


class vt100_video_device : public device_t,
	public device_video_interface
{
public:
	vt100_video_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	template <class Object> devcb_base &set_ram_rd_callback(Object &&cb) { return m_read_ram.set_callback(std::forward<Object>(cb)); }
	template <class Object> devcb_base &set_vert_freq_intr_wr_callback(Object &&cb) { return m_write_vert_freq_intr.set_callback(std::forward<Object>(cb)); }
	template <class Object> devcb_base &set_lba3_lba4_wr_callback(Object &&cb) { return m_write_lba3_lba4.set_callback(std::forward<Object>(cb)); }
	template <class Object> devcb_base &set_lba7_wr_callback(Object &&cb) { return m_write_lba7.set_callback(std::forward<Object>(cb)); }

	void set_chargen_tag(const char *tag) { m_char_rom.set_tag(tag); }

	DECLARE_READ_LINE_MEMBER(lba7_r);
	DECLARE_WRITE8_MEMBER(dc012_w);
	DECLARE_WRITE8_MEMBER(dc011_w);
	DECLARE_WRITE8_MEMBER(brightness_w);

	virtual void video_update(bitmap_ind16 &bitmap, const rectangle &cliprect);

protected:
	vt100_video_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_add_mconfig(machine_config &config) override;

	// internal state
	void recompute_parameters();
	void vblank_callback(screen_device &screen, bool state);
	virtual void display_char(bitmap_ind16 &bitmap, uint8_t code, int x, int y, uint8_t scroll_region, uint8_t display_type);
	TIMER_CALLBACK_MEMBER(lba3_change);
	TIMER_CALLBACK_MEMBER(lba7_change);
	virtual void notify_vblank(bool choice) { }

	devcb_read8        m_read_ram;
	devcb_write_line   m_write_vert_freq_intr;
	devcb_write8       m_write_lba3_lba4;
	devcb_write_line   m_write_lba7;

	int m_lba7;

	bool MHFU_FLAG;
	int MHFU_counter;

	// dc012 attributes
	uint8_t m_scroll_latch;
	bool m_scroll_latch_valid;
	uint8_t m_blink_flip_flop;
	uint8_t m_reverse_field;
	uint8_t m_basic_attribute;
	// dc011 attributes
	uint8_t m_columns;
	uint8_t m_height;
	uint8_t m_height_MAX;
	uint8_t m_fill_lines;
	bool m_is_50hz;
	bool m_interlaced;
	emu_timer *m_lba3_change_timer;
	emu_timer *m_lba7_change_timer;

	required_region_ptr<uint8_t> m_char_rom; /* character rom region */
	required_device<palette_device> m_palette;

	bool m_notify_vblank;
	int m_last_scroll;

	bool m_linedoubler;
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
	virtual void display_char(bitmap_ind16 &bitmap, uint8_t code, int x, int y, uint8_t scroll_region, uint8_t display_type) override;
	virtual void notify_vblank(bool choice) override;
	virtual void device_reset() override;
	virtual void device_add_mconfig(machine_config &config) override;
};

DECLARE_DEVICE_TYPE(VT100_VIDEO, vt100_video_device)
DECLARE_DEVICE_TYPE(RAINBOW_VIDEO, rainbow_video_device)


#define MCFG_VT_SET_SCREEN MCFG_VIDEO_SET_SCREEN

#define MCFG_VT_CHARGEN(_tag) \
	downcast<vt100_video_device &>(*device).set_chargen_tag(_tag);

#define MCFG_VT_VIDEO_RAM_CALLBACK(_read) \
	downcast<vt100_video_device &>(*device).set_ram_rd_callback(DEVCB_##_read);

#define MCFG_VT_VIDEO_VERT_FREQ_INTR_CALLBACK(_write) \
	downcast<vt100_video_device &>(*device).set_vert_freq_intr_wr_callback(DEVCB_##_write);

#define MCFG_VT_VIDEO_LBA3_LBA4_CALLBACK(_write) \
	downcast<vt100_video_device &>(*device).set_lba3_lba4_wr_callback(DEVCB_##_write);

#define MCFG_VT_VIDEO_LBA7_CALLBACK(_write) \
	downcast<vt100_video_device &>(*device).set_lba7_wr_callback(DEVCB_##_write);

#endif // MAME_VIDEO_VTVIDEO_H
