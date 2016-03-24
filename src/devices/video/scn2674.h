// license:BSD-3-Clause
// copyright-holders:Carl
#ifndef SCN2674_H
#define SCN2674_H

#include "emu.h"

#define MCFG_SCN2674_VIDEO_ADD(_tag, _clock, _irq) \
	MCFG_DEVICE_ADD(_tag, SCN2674_VIDEO, _clock) \
	devcb = &scn2674_device::set_irq_callback(*device, DEVCB_##_irq);

#define MCFG_SCN2674_TEXT_CHARACTER_WIDTH(_value) \
	scn2674_device::static_set_character_width(*device, _value);

#define MCFG_SCN2674_GFX_CHARACTER_WIDTH(_value) \
	scn2674_device::static_set_gfx_character_width(*device, _value);

#define MCFG_SCN2674_DRAW_CHARACTER_CALLBACK_OWNER(_class, _method) \
	scn2674_device::static_set_display_callback(*device, scn2674_device::draw_character_delegate(&_class::_method, #_class "::" #_method, downcast<_class *>(owner)));

#define SCN2674_DRAW_CHARACTER_MEMBER(_name) void _name(bitmap_rgb32 &bitmap, int x, int y, UINT8 linecount, UINT8 charcode, UINT16 address, UINT8 cursor, UINT8 dw, UINT8 lg, UINT8 ul, UINT8 blink)

class scn2674_device : public device_t,
						public device_video_interface,
						public device_memory_interface
{
public:
	scn2674_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	typedef device_delegate<void (bitmap_rgb32 &bitmap, int x, int y, UINT8 linecount, UINT8 charcode, UINT16 address, UINT8 cursor, UINT8 dw, UINT8 lg, UINT8 ul, UINT8 blink)> draw_character_delegate;

	// static configuration
	template<class _Object> static devcb_base &set_irq_callback(device_t &device, _Object object) { return downcast<scn2674_device &>(device).m_irq_cb.set_callback(object); }
	static void static_set_character_width(device_t &device, int value) { downcast<scn2674_device &>(device).m_text_hpixels_per_column = value; }
	static void static_set_gfx_character_width(device_t &device, int value) { downcast<scn2674_device &>(device).m_gfx_hpixels_per_column = value; }
	static void static_set_display_callback(device_t &device, draw_character_delegate callback) { downcast<scn2674_device &>(device).m_display_cb = callback; }

	DECLARE_READ8_MEMBER( read );
	DECLARE_WRITE8_MEMBER( write );
	DECLARE_READ8_MEMBER( buffer_r ) { return m_buffer; }
	DECLARE_WRITE8_MEMBER( buffer_w ) { m_buffer = data; }

	UINT32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	virtual const address_space_config *memory_space_config(address_spacenum spacenum = AS_0) const override { return (spacenum == AS_0) ? &m_space_config : nullptr; }

protected:
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

private:
	bitmap_rgb32 m_bitmap;
	devcb_write_line m_irq_cb;

	UINT8 m_IR_pointer;
	UINT8 m_screen1_l;
	UINT8 m_screen1_h;
	UINT8 m_cursor_l;
	UINT8 m_cursor_h;
	UINT8 m_screen2_l;
	UINT8 m_screen2_h;
	UINT8 m_irq_register;
	UINT8 m_status_register;
	UINT8 m_irq_mask;
	UINT8 m_gfx_enabled;
	UINT8 m_display_enabled;
	UINT8 m_display_enabled_field;
	UINT8 m_display_enabled_scanline;
	UINT8 m_cursor_enabled;
	UINT8 m_hpixels_per_column;
	UINT8 m_text_hpixels_per_column;
	UINT8 m_gfx_hpixels_per_column;
	UINT8 m_IR0_double_ht_wd;
	UINT8 m_IR0_scanline_per_char_row;
	UINT8 m_IR0_sync_select;
	UINT8 m_IR0_buffer_mode_select;
	UINT8 m_IR1_interlace_enable;
	UINT8 m_IR1_equalizing_constant;
	UINT8 m_IR2_row_table;
	UINT8 m_IR2_horz_sync_width;
	UINT8 m_IR2_horz_back_porch;
	UINT8 m_IR3_vert_front_porch;
	UINT8 m_IR3_vert_back_porch;
	UINT8 m_IR4_rows_per_screen;
	UINT8 m_IR4_character_blink_rate_divisor;
	UINT8 m_IR5_character_per_row;
	UINT8 m_IR6_cursor_first_scanline;
	UINT8 m_IR6_cursor_last_scanline;
	UINT8 m_IR7_cursor_underline_position;
	UINT8 m_IR7_cursor_rate_divisor;
	UINT8 m_IR7_cursor_blink;
	UINT8 m_IR7_vsync_width;
	UINT8 m_IR8_display_buffer_first_address_LSB;
	UINT8 m_IR9_display_buffer_first_address_MSB;
	UINT8 m_IR9_display_buffer_last_address;
	UINT8 m_IR10_display_pointer_address_lower;
	UINT8 m_IR11_display_pointer_address_upper;
	UINT8 m_IR11_reset_scanline_counter_on_scrollup;
	UINT8 m_IR11_reset_scanline_counter_on_scrolldown;
	UINT8 m_IR12_scroll_start;
	UINT8 m_IR12_split_register_1;
	UINT8 m_IR13_scroll_end;
	UINT8 m_IR13_split_register_2;
	UINT8 m_IR14_scroll_lines;
	UINT8 m_IR14_double_1;
	UINT8 m_IR14_double_2;
	UINT8 m_spl1;
	UINT8 m_spl2;
	UINT8 m_dbl1;
	UINT8 m_buffer;
	int m_linecounter;
	UINT16 m_address;
	int m_start1change;

	UINT8 m_irq_state;

	void write_init_regs(UINT8 data);
	void write_command(UINT8 data);
	void recompute_parameters();

	draw_character_delegate m_display_cb;
	emu_timer *m_scanline_timer;
	const address_space_config m_space_config;
	enum
	{
		TIMER_SCANLINE
	};
};


extern const device_type SCN2674_VIDEO;

#endif
