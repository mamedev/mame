// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    MOS Technology 8563 VDC emulation

**********************************************************************/

#ifndef MAME_VIDEO_MOS8563_H
#define MAME_VIDEO_MOS8563_H

#pragma once

#include "video/mc6845.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> mos8563_device

class mos8563_device : public mc6845_device,
						public device_memory_interface,
						public device_palette_interface
{
public:
	mos8563_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void address_w(uint8_t data) override;
	virtual uint8_t status_r() override;
	virtual uint8_t register_r() override;
	virtual void register_w(uint8_t data) override;

	inline uint8_t read_videoram(offs_t offset);
	inline void write_videoram(offs_t offset, uint8_t data);

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

protected:
	static constexpr uint8_t RETRACE_LINES = 10;
	static constexpr uint8_t RETRACE_DOTS = 40;
	static constexpr uint8_t FRONT_PORCH_DOTS = 80;
	static constexpr int LINE_COUNT = 256 * 32 + 32;

	mos8563_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_post_load() override ATTR_COLD;

	// device_memory_interface overrides
	virtual space_config_vector memory_space_config() const override;

	// device_palette_interface overrides
	virtual uint32_t palette_entries() const noexcept override { return 16; }

	TIMER_CALLBACK_MEMBER(block_copy_tick);

	const address_space_config      m_videoram_space_config;

	uint8_t m_char_buffer[80];
	uint8_t m_attr_buffer[80];

	bool    m_char_blink_state;
	uint8_t   m_char_blink_count;

	/* register file */
	uint16_t  m_attribute_addr;       /* 0x14/0x15 */
	uint8_t   m_horiz_char;           /* 0x16 */
	uint8_t   m_vert_char_disp;       /* 0x17 */
	uint8_t   m_vert_scroll;          /* 0x18 */
	uint8_t   m_horiz_scroll;         /* 0x19 */
	uint8_t   m_color;                /* 0x1a */
	uint8_t   m_row_addr_incr;        /* 0x1b */
	uint8_t   m_char_base_addr;       /* 0x1c */
	uint8_t   m_underline_ras;        /* 0x1d */
	uint8_t   m_word_count;           /* 0x1e */
	uint8_t   m_data;                 /* 0x1f */
	uint16_t  m_block_addr;           /* 0x20/0x21 */
	uint8_t   m_de_begin;             /* 0x22 */
	uint8_t   m_de_end;               /* 0x23 */
	uint8_t   m_dram_refresh;         /* 0x24 */
	uint8_t   m_sync_polarity;        /* 0x25 */

	int m_revision;
	uint8_t m_draw_ra;
	uint8_t m_draw_row_ra;
	uint16_t m_draw_row;
	uint16_t m_draw_addr;
	uint16_t m_attr_row_addr;
	uint16_t m_bitmap_addr;

	bool m_frame_started;
	int32_t m_line_index;
	int32_t m_frame_lines;
	int32_t m_field_lines;
	int32_t m_screen_line;
	uint8_t m_retrace_lines;
	uint16_t m_latched_disp_addr;
	uint16_t m_latched_attr_addr;
	uint16_t m_frame_disp_addr;
	uint16_t m_frame_attr_addr;
	bool m_start_addr_latched;
	bool m_vsync_seen;
	bitmap_rgb32 m_field_bitmap;

	uint16_t m_line_addr[LINE_COUNT];
	uint16_t m_line_bitmap_addr[LINE_COUNT];
	uint16_t m_line_attr_addr[LINE_COUNT];
	uint8_t m_line_ra[LINE_COUNT];
	bool m_line_shown[LINE_COUNT];
	uint8_t m_line_field[LINE_COUNT];

	offs_t vram_address(offs_t offset) const;
	void update_char_width();
	static int displayed_width(uint8_t r22);
	static bool semigraphics_gap(uint8_t r22);

	bool interlace_video() const { return (m_mode_control & 0x03) == 3; }
	void update_screen_geometry();
	void reload_draw_counters();
	void advance_draw_line();
	void start_draw_line();
	void draw_line(int line, int row, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void draw_row(bitmap_rgb32 &bitmap, const rectangle &cliprect, uint16_t ma, uint16_t attr_row_addr, uint8_t ra, int y, int8_t cursor_x, int hbp);

	virtual void update_cursor_state() override;
	virtual void set_vsync(int state) override;
	virtual void configure_screen(int width, int height, const rectangle &visarea, const attotime &refresh) override;
	virtual void frame_start() override;

	emu_timer *m_block_copy_timer;

	void mos8563_videoram_map(address_map &map) ATTR_COLD;
};


// ======================> mos8568_device

class mos8568_device : public mos8563_device
{
public:
	mos8568_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
};


// device type definitions
DECLARE_DEVICE_TYPE(MOS8563, mos8563_device)
DECLARE_DEVICE_TYPE(MOS8568, mos8568_device)

#endif // MAME_VIDEO_MOS8563_H
