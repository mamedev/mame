// license:BSD-3-Clause
// copyright-holders:wurthless-elektroniks

#ifndef MAME_VIDEO_SSD1306_H
#define MAME_VIDEO_SSD1306_H

#pragma once

#include "screen.h"

DECLARE_DEVICE_TYPE(SSD1306,  ssd1306_device)

class ssd1306_device :  public device_t,
						public device_video_interface,
						public device_palette_interface
{
public:

	// see datasheet, Table 7-1. bits are in order BS2..BS0
	enum : uint8_t
	{
		SSD1306_INTERFACE_MODE_SPI_4WIRE     = 0b000,
		SSD1306_INTERFACE_MODE_SPI_3WIRE     = 0b001,
		SSD1306_INTERFACE_MODE_I2C           = 0b010,
		SSD1306_INTERFACE_MODE_PARALLEL_6800 = 0b100,
		SSD1306_INTERFACE_MODE_PARALLEL_8080 = 0b110,
	};

	enum : uint8_t
	{
		SSD1306_ADDRESSING_MODE_HORIZONTAL = 0b00,
		SSD1306_ADDRESSING_MODE_VERTICAL   = 0b01,
		SSD1306_ADDRESSING_MODE_PAGE       = 0b10,
		SSD1306_ADDRESSING_MODE_INVALID    = 0b11
	};

	ssd1306_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual uint32_t palette_entries() const noexcept override;

	void set_external_oscillator(bool use_external_oscillator);
	void set_intf_mode(uint8_t mode);
	void set_base_rowscan_invert(bool base_rowscan_invert);

	void spi_cs_w(int state);
	void spi_si_w(int state);
	void spi_sck_w(int state);

	void write(offs_t offset, uint8_t data);
	u8   read(offs_t offset);

	/**
	 * Set the state of the D/C# pin.
	 *
	 * In four-wire SPI mode, if the D/C# pin is high,
	 * then next write is data, otherwise, it's a command.
	 * The same behavior applies for parallel bus modes.
	 * In three-wire SPI mode, D/C# is set within the command itself.
	 *
	 * In I2C mode, D/C# changes the I2C slave address.
	 * If low, the address is 0x3c (0b0111100).
	 * If high, it's 0x3d (0b0111101).
	 */
	void dc_w(int dc);
	void rst_w(int rst);

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	void 	raw_write(int dc_line, uint8_t data);
	uint8_t raw_read(int dc_line);

	/**
	 * Buffers FIFO until n bytes have been read, then reset the FIFO pointer.
	 * Return true when n bytes are in FIFO.
	 */
	bool populate_fifo_until_n_bytes(uint8_t data, int num_bytes);
	
	void handle_invalid_command();

	void exec_command(uint8_t data);
	void exec_command_2x(uint8_t data);
	void exec_command_ax(uint8_t data);
	void exec_command_dx(uint8_t data);

	void update_scan_rate();

	bool m_dc_line;
	bool m_reset_asserted;
	bool m_spi_cs_asserted;
	bool m_spi_si;
	bool m_spi_sck_asserted;
	bool m_using_external_oscillator;
	uint8_t m_pending_interface_mode;
	bool m_dc_internal_state; // last latched D/C# value
	
	// ... the following are all set or affected by commands ...

	// Commands 0x01-0x1f
	uint8_t m_pagemode_column_start_address;

	// Command 0x20
	uint8_t m_addressing_mode;

	// Command 0x21
	uint8_t m_hvmode_column_start_address;
	uint8_t m_hvmode_column_end_address;

	// Command 0x22
	uint8_t m_hvmode_page_start_address;
	uint8_t m_hvmode_page_end_address;

	// Commands 0x26,0x27,0x29,0x2a
	bool m_horizontal_scroll_pending;
	bool m_vertical_scroll_pending;
	bool m_horizontal_scrolling_left_pending;
	uint8_t m_horizontal_scroll_page_start_address_pending;
	uint8_t m_horizontal_scroll_interval_pending;
	uint8_t m_horizontal_scroll_page_end_address_pending;
	uint8_t m_vertical_scroll_offset_pending;

	// Commands 0x40-0x7f
	uint8_t m_display_start_line;

	// Command 0x81
	uint8_t m_contrast;

	// Command 0xa0/0xa1
	bool m_seg0_column_remapped;

	// Command 0xa3
	uint8_t m_vertical_scroll_top_fixed_rows;
	uint8_t m_vertical_scroll_bottom_scrolled_rows;

	// Commands 0xa4/0xa5
	bool m_display_blanking; // false = draw framebuffer, true = set all pixels to 1

	// Commands 0xa6/0xa7
	bool m_inverting_pixels; // false = normal display, true = invert all pixels

	// Command 0xa8
	uint8_t m_mux_ratio; // valid values 16-63

	// Command 0xae/0xaf
	bool m_display_enabled; // false = display not driven, true = display will be driven

	// Command 0xB0-0xB7
	uint8_t m_pagemode_page_start_address; // =0-7

	// commands 0xc0, 0xc8
	bool m_row_scan_direction_inverse;
	
	// command 0xd3
	uint8_t m_display_offset;

	// command 0xd5
	uint8_t m_clk_div;  // clock divider, minus 1 (=0-15)
	uint8_t m_osc_freq; // internal oscillator frequency select

	// command 0xd9
	uint8_t m_phase_1_period;
	uint8_t m_phase_2_period;

	// command 0xda
	bool m_row_scan_interleaved;  // false = scan 0, 1, 2, 3, ... n; true = scan 0, 32, 1, 33, ... n
	bool m_row_scan_split_invert; // false = scan 0-31 then 32-63; true = scan 32-63 then 0-31

	// command 0xdb
	uint8_t m_vcomh_deselect_level;

	// internal state stuff
	uint8_t m_page_address_pointer;
	uint8_t m_column_address_pointer;

	uint8_t m_command_fifo[8];
	uint8_t m_command_pointer;

	bool m_horizontal_scroll_enabled;
	bool m_vertical_scroll_enabled;
	bool m_horizontal_scrolling_left;
	uint8_t m_horizontal_scroll_page_start_address;
	uint8_t m_horizontal_scroll_interval;
	uint8_t m_horizontal_scroll_page_end_address;
	uint8_t m_vertical_scroll_offset;

	uint8_t m_current_interface_mode;

	uint8_t m_spi_shift;
	int m_spi_bits_left;

	bool m_base_rowscan_invert;

	// display memory: one "page" is 8 pixels tall, one line is 128 pixels long
	uint8_t m_gddram[128 * 8];
};



#endif
