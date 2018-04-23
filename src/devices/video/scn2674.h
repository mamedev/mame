// license:BSD-3-Clause
// copyright-holders:Carl
#ifndef MAME_VIDEO_SCN2674_H
#define MAME_VIDEO_SCN2674_H

#pragma once


#define MCFG_SCN2672_INTR_CALLBACK(_intr) \
	devcb = &downcast<scn2672_device &>(*device).set_intr_callback(DEVCB_##_intr);

#define MCFG_SCN2672_CHARACTER_WIDTH(_value) \
	downcast<scn2672_device &>(*device).set_character_width(_value);

#define MCFG_SCN2672_DRAW_CHARACTER_CALLBACK_OWNER(_class, _method) \
	downcast<scn2672_device &>(*device).set_display_callback(scn2672_device::draw_character_delegate(&_class::_method, #_class "::" #_method, this));

#define SCN2672_DRAW_CHARACTER_MEMBER(_name) void _name(bitmap_rgb32 &bitmap, int x, int y, uint8_t linecount, uint8_t charcode, uint16_t address, uint8_t cursor, uint8_t dw, uint8_t lg, uint8_t ul, uint8_t blink)


#define MCFG_SCN2674_INTR_CALLBACK(_intr) \
	devcb = &downcast<scn2674_device &>(*device).set_intr_callback(DEVCB_##_intr);

#define MCFG_SCN2674_CHARACTER_WIDTH(_value) \
	downcast<scn2674_device &>(*device).set_character_width(_value);

#define MCFG_SCN2674_DRAW_CHARACTER_CALLBACK_OWNER(_class, _method) \
	downcast<scn2674_device &>(*device).set_display_callback(scn2674_device::draw_character_delegate(&_class::_method, #_class "::" #_method, this));

#define SCN2674_DRAW_CHARACTER_MEMBER(_name) void _name(bitmap_rgb32 &bitmap, int x, int y, uint8_t linecount, uint8_t charcode, uint16_t address, uint8_t cursor, uint8_t dw, uint8_t lg, uint8_t ul, uint8_t blink)


class scn2674_device : public device_t,
						public device_video_interface,
						public device_memory_interface
{
public:
	scn2674_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	typedef device_delegate<void (bitmap_rgb32 &bitmap, int x, int y, uint8_t linecount, uint8_t charcode, uint16_t address, uint8_t cursor, uint8_t dw, uint8_t lg, uint8_t ul, uint8_t blink)> draw_character_delegate;

	// static configuration
	template <class Object> devcb_base &set_intr_callback(Object &&cb) { return m_intr_cb.set_callback(std::forward<Object>(cb)); }
	void set_character_width(int value) { m_hpixels_per_column = value; }
	template <typename Object> void set_display_callback(Object &&cb) { m_display_cb = std::forward<Object>(cb); }

	DECLARE_READ8_MEMBER( read );
	DECLARE_WRITE8_MEMBER( write );
	DECLARE_READ8_MEMBER( buffer_r ) { return m_buffer; }
	DECLARE_WRITE8_MEMBER( buffer_w ) { m_buffer = data; }

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

protected:
	scn2674_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

	virtual space_config_vector memory_space_config() const override;

//protected:
	bitmap_rgb32 m_bitmap;
	devcb_write_line m_intr_cb;

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
	uint8_t m_buffer;
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
	const address_space_config m_space_config;
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
