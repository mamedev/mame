// license:BSD-3-Clause
// copyright-holders:Carl
#ifndef MAME_VIDEO_SCN2674_H
#define MAME_VIDEO_SCN2674_H

#pragma once


#define SCN2672_DRAW_CHARACTER_MEMBER(_name) void _name(bitmap_rgb32 &bitmap, int x, int y, uint8_t linecount, uint8_t charcode, uint8_t attrcode, uint16_t address, bool cursor, bool dw, bool lg, bool ul, bool blink)

#define SCN2674_DRAW_CHARACTER_MEMBER(_name) void _name(bitmap_rgb32 &bitmap, int x, int y, uint8_t linecount, uint8_t charcode, uint8_t attrcode, uint16_t address, bool cursor, bool dw, bool lg, bool ul, bool blink)


class scn2674_device : public device_t,
						public device_video_interface,
						public device_memory_interface
{
public:
	scn2674_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	typedef device_delegate<void (bitmap_rgb32 &bitmap, int x, int y, uint8_t linecount, uint8_t charcode, uint8_t attrcode, uint16_t address, bool cursor, bool dw, bool lg, bool ul, bool blink)> draw_character_delegate;

	// static configuration
	auto intr_callback() { return m_intr_cb.bind(); }
	auto breq_callback() { return m_breq_cb.bind(); }
	void set_character_width(int value) { m_hpixels_per_column = value; }

	template <class FunctionClass>
	void set_display_callback(void (FunctionClass::*callback)(bitmap_rgb32 &, int, int, uint8_t, uint8_t, uint8_t, uint16_t, bool, bool, bool, bool, bool), const char *name)
	{
		set_display_callback(draw_character_delegate(callback, name, nullptr, static_cast<FunctionClass *>(nullptr)));
	}
	template <class FunctionClass>
	void set_display_callback(const char *devname, void (FunctionClass::*callback)(bitmap_rgb32 &, int, int, uint8_t, uint8_t, uint8_t, uint16_t, bool, bool, bool, bool, bool), const char *name)
	{
		set_display_callback(draw_character_delegate(callback, name, devname, static_cast<FunctionClass *>(nullptr)));
	}
	void set_display_callback(draw_character_delegate callback)
	{
		m_display_cb = callback;
	}

	DECLARE_READ8_MEMBER( read );
	DECLARE_WRITE8_MEMBER( write );
	DECLARE_READ8_MEMBER( buffer_r ) { return m_char_buffer; }
	DECLARE_WRITE8_MEMBER( buffer_w ) { m_char_buffer = data; }
	DECLARE_READ8_MEMBER( attr_buffer_r ) { return m_attr_buffer; }
	DECLARE_WRITE8_MEMBER( attr_buffer_w ) { m_attr_buffer = data; }

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

protected:
	scn2674_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, bool extend_addressing);

	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

	virtual space_config_vector memory_space_config() const override;

//protected:
	bitmap_rgb32 m_bitmap;
	devcb_write_line m_intr_cb;
	devcb_write_line m_breq_cb;

	uint8_t m_IR_pointer;
	uint16_t m_screen1_address;
	uint16_t m_screen2_address;
	uint16_t m_cursor_address;
	uint8_t m_irq_register;
	uint8_t m_status_register;
	uint8_t m_irq_mask;
	bool m_gfx_enabled;
	bool m_display_enabled;
	bool m_display_enabled_field;
	bool m_display_enabled_scanline;
	bool m_cursor_enabled;
	uint8_t m_hpixels_per_column;
	bool m_double_ht_wd;
	uint8_t m_scanline_per_char_row;
	bool m_csync_select;
	uint8_t m_buffer_mode_select;
	bool m_interlace_enable;
	uint8_t m_equalizing_constant;
	bool m_use_row_table;
	uint8_t m_horz_sync_width;
	uint8_t m_horz_back_porch;
	uint8_t m_vert_front_porch;
	uint8_t m_vert_back_porch;
	uint8_t m_rows_per_screen;
	uint8_t m_character_blink_rate_divisor;
	uint8_t m_character_per_row;
	uint8_t m_cursor_first_scanline;
	uint8_t m_cursor_last_scanline;
	uint8_t m_cursor_underline_position;
	uint8_t m_cursor_rate_divisor;
	bool m_cursor_blink;
	uint8_t m_vsync_width;
	uint16_t m_display_buffer_first_address;
	uint8_t m_display_buffer_last_address;
	uint16_t m_display_pointer_address;
	uint8_t m_reset_scanline_counter_on_scrollup;
	uint8_t m_reset_scanline_counter_on_scrolldown;
	bool m_scroll_start;
	bool m_scroll_end;
	uint8_t m_scroll_lines;
	uint8_t m_split_register[2];
	uint8_t m_double[2];
	bool m_spl[2];
	uint8_t m_dbl1;
	uint8_t m_char_buffer;
	uint8_t m_attr_buffer;
	int m_linecounter;
	uint16_t m_address;
	int m_start1change;

	virtual void write_init_regs(uint8_t data);
	void set_gfx_enabled(bool enabled);
	void set_display_enabled(bool enabled, int n);
	void set_cursor_enabled(bool enabled);
	void reset_interrupt_status(uint8_t bits);
	void write_interrupt_mask(bool enabled, uint8_t bits);
	void write_delayed_command(uint8_t data);
	void write_command(uint8_t data);

	void recompute_parameters();

	void scn2674_vram(address_map &map);

	draw_character_delegate m_display_cb;
	emu_timer *m_scanline_timer;
	address_space *m_char_space;
	address_space *m_attr_space;
	const address_space_config m_char_space_config;
	const address_space_config m_attr_space_config;
	enum
	{
		TIMER_SCANLINE
	};
};

class scn2672_device : public scn2674_device
{
public:
	scn2672_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void write_init_regs(uint8_t data) override;
};


DECLARE_DEVICE_TYPE(SCN2672, scn2672_device)
DECLARE_DEVICE_TYPE(SCN2674, scn2674_device)

#endif // MAME_VIDEO_SCN2674_H
