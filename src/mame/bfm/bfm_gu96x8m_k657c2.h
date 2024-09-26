// license:BSD-3-Clause
// copyright-holders:David Haywood
#ifndef MAME_MACHINE_BFG_GU96X8M_K657C2_H
#define MAME_MACHINE_BFG_GU96X8M_K657C2_H

#pragma once


class bfm_gu96x8m_k657c2_device : public device_t
{
public:
	bfm_gu96x8m_k657c2_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, uint8_t port_val)
		: bfm_gu96x8m_k657c2_device(mconfig, tag, owner, clock)
	{
	}

	bfm_gu96x8m_k657c2_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void write_char(int data);
	virtual void update_display();

	void shift_data(int data);
	void setdata(int data);
	void set_char(int data);

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_post_load() override;

private:
	TIMER_CALLBACK_MEMBER(frame_update_callback);

	emu_timer *m_frame_timer;

	output_finder<1> m_vfd_background;
	output_finder<96 * 8> m_dotmatrix;
	output_finder<1> m_duty;

	uint8_t m_cursor_pos;
	uint8_t m_pcursor_pos;
	uint8_t m_window_start;
	uint8_t m_window_end;
	uint8_t m_window_size;
	uint8_t m_shift_count;
	uint8_t m_shift_data;
	uint8_t m_brightness;
	uint8_t m_scroll_active;
	uint8_t m_display_mode;
	uint8_t m_blank_control;
	uint8_t m_extended_commands_enabled;
	uint8_t m_graphics_commands_enabled;
	uint8_t m_ascii_charset;
	uint8_t m_graphics_data[96];
	uint8_t m_graphics_start;
	uint8_t m_graphics_end;
	uint8_t m_cursor;
	uint8_t m_charset_offset[128];
	uint8_t m_chars[16];
	uint8_t m_attributes[16];
	uint8_t m_extra_data[7];
	uint8_t m_extra_data_count;
	uint8_t m_load_extra_data;
	uint8_t m_udf[16][6];
	uint8_t m_led_colour;
	uint8_t m_flash_rate;
	uint8_t m_flash_count;
	uint8_t m_flash_blank;
	uint8_t m_led_flash_rate;
	uint8_t m_led_flash_count;
	uint8_t m_led_flash_blank;
	uint8_t m_led_flash_enabled;
};

DECLARE_DEVICE_TYPE(BFM_GU96X8M_K657C2, bfm_gu96x8m_k657c2_device)

#endif // MAME_MACHINE_BFG_GU96X8M_K657C2_H
