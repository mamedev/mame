// license:BSD-3-Clause
// copyright-holders:Sergey Svishchev
/***************************************************************************

    HP 1LL3-0005 GPU emulation.

 ***************************************************************************/

#ifndef MAME_VIDEO_HP1LL3_H
#define MAME_VIDEO_HP1LL3_H

#pragma once


///*************************************************************************
//  TYPE DEFINITIONS
///*************************************************************************


// ======================> hp1ll3_device

class hp1ll3_device : public device_t, public device_video_interface
{
public:
	// construction/destruction
	hp1ll3_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// Configuration
	// Set VRAM size (in kw). Parameter must be <= 128 and it must be a power of 2.
	void set_vram_size(unsigned kw) { m_vram_size = kw * 1024; }

	uint8_t read(offs_t offset);
	void write(offs_t offset, uint8_t data);

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	struct Point
	{
		uint16_t x, y;
	};
	struct Rectangle
	{
		Point origin;
		Point size;
	};

	void command(int command);

	uint16_t get_pix_addr(uint16_t x, uint16_t y) const;
	inline void point(int x, int y, bool pix, const uint16_t masks[]);
	void get_font(uint16_t& font_data, uint16_t& font_height) const;
	void label(uint8_t chr, int width);
	void line(int x_from, int y_from, int x_to, int y_to);
	void wr_video(uint16_t addr, uint16_t v);
	uint16_t rd_video(uint16_t addr) const;
	static void get_rop_masks(uint16_t rop, uint16_t masks[]);
	static uint16_t apply_rop(uint16_t old_pix, uint16_t new_pix, uint16_t glob_mask, const uint16_t masks[]);
	void rmw_rop(uint16_t addr, uint16_t new_pix, uint16_t glob_mask, const uint16_t masks[]);
	void clip_coord(int size_1, int &p1, int origin_clip, int size_clip, int &origin_2, int &size_2) const;
	bool bitblt(uint16_t src_base_addr, unsigned src_width, unsigned src_height, Point src_p,
				const Rectangle &clip_rect, const Rectangle &dst_rect, uint16_t rop, bool use_m_org = true);
	void rowbltpos(unsigned p1_pix, unsigned p2_pix, int width, const uint16_t masks[]);
	void rowbltneg(unsigned p1_pix, unsigned p2_pix, int width, const uint16_t masks[]);
	void fill(const Rectangle &fill_rect, uint16_t pattern_no);
	uint16_t get_pattern_addr(uint16_t pattern_no) const;
	void draw_cursor();
	void draw_sprite();
	void draw_cursor_sprite();
	void set_pen_pos(Point p);
	void set_sprite_pos(Point p);
	void disable_cursor();
	void disable_sprite();
	Rectangle get_window() const;
	Rectangle get_screen() const;
	void get_hv_timing(bool vertical, unsigned& total, unsigned& active) const;
	void apply_conf();

	uint16_t m_conf[11], m_input[2], m_io_word;
	int m_rd_ptr, m_wr_ptr, m_memory_ptr;
	int m_command, m_horiz_pix_total, m_vert_pix_total;

	uint16_t m_sad;
	uint16_t m_org;
	uint16_t m_dad;
	uint16_t m_rr;
	uint16_t m_fad;
	uint16_t m_udl;

	bool m_enable_video, m_enable_cursor, m_enable_sprite;
	uint16_t m_cursor_x, m_cursor_y, m_saved_x;
	uint16_t m_cursor_pattern;
	uint16_t m_cursor_offset;
	uint16_t m_sprite_x, m_sprite_y;
	uint16_t m_sprite_pattern;
	struct {
		uint16_t width, height, org_x, org_y;
	} m_window;
	std::unique_ptr<uint16_t[]> m_videoram;
	unsigned m_vram_size;
	uint16_t m_ram_addr_mask;
	uint16_t m_rw_win_x, m_rw_win_y;
	uint8_t m_rw_win_off;

	bool m_busy;

	bitmap_ind16 m_bitmap;
};


// device type definition
DECLARE_DEVICE_TYPE(HP1LL3, hp1ll3_device)

#endif // MAME_VIDEO_HP1LL3_H
