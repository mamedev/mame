// license:BSD-3-Clause
// copyright-holders:Jean-Francois DEL NERO
/*********************************************************************

    ef9365.h

    Thomson EF9365/EF9366/EF9367 video controller

*********************************************************************/

#ifndef MAME_VIDEO_EF9365_H
#define MAME_VIDEO_EF9365_H

#pragma once

#include "emupal.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> ef9365_device

class ef9365_device :   public device_t,
						public device_memory_interface,
						public device_video_interface
{
public:
	static constexpr unsigned BITPLANE_MAX_SIZE = 0x8000;
	static constexpr unsigned MAX_BITPLANES = 8;

	static constexpr int DISPLAY_MODE_256x256  = 0x00;
	static constexpr int DISPLAY_MODE_512x512  = 0x01;
	static constexpr int DISPLAY_MODE_512x256  = 0x02;
	static constexpr int DISPLAY_MODE_128x128  = 0x03;
	static constexpr int DISPLAY_MODE_64x64    = 0x04;
	static constexpr int DISPLAY_MODE_1024x512 = 0x05;

	// construction/destruction
	ef9365_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// configuration
	template <typename T> void set_palette_tag(T &&tag) { m_palette.set_tag(std::forward<T>(tag)); }
	void set_nb_bitplanes(int nb_bitplanes);
	void set_display_mode(int display_mode);
	auto irq_handler() { return m_irq_handler.bind(); } // IRQ pin
	auto write_msl() { return m_write_msl.bind(); } // memory select during pixel write

	// device interface
	uint8_t data_r(offs_t offset);
	void data_w(offs_t offset, uint8_t data);

	void update_scanline(uint16_t scanline);
	void set_color_filler(uint8_t color);
	void set_color_entry(int index, uint8_t r, uint8_t g, uint8_t b);
	uint8_t get_last_readback_word(int bitplane_number, int *pixel_offset);

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

	// device_config_memory_interface overrides
	virtual space_config_vector memory_space_config() const override;

	// address space configurations
	const address_space_config m_space_config;

	TIMER_CALLBACK_MEMBER(clear_busy_flag);

private:
	int get_char_pix(uint8_t c, int x, int y);
	void plot(int x_pos, int y_pos);
	int draw_character(uint8_t c, bool block, bool smallblock);
	int draw_vector(uint16_t start_x, uint16_t start_y, int16_t delta_x, int16_t delta_y);
	uint16_t get_x_reg();
	uint16_t get_y_reg();
	void set_x_reg(uint16_t x);
	void set_y_reg(uint16_t y);
	void set_msl_pins(uint16_t x, uint16_t y);
	void screen_scanning(bool force_clear);
	void set_busy_flag(int cycles);
	void set_video_mode();
	void draw_border(uint16_t line);
	void ef9365_exec(uint8_t cmd);
	void dump_bitplanes_word();
	void update_interrupts();

	void ef9365_map(address_map &map) ATTR_COLD;

	// internal state
	required_region_ptr<uint8_t> m_charset;
	address_space *m_videoram;

	uint8_t m_irq_state;
	uint8_t m_irq_vb;
	uint8_t m_irq_lb;
	uint8_t m_irq_rdy;
	uint8_t m_current_color;
	uint8_t m_bf;              // busy flag
	uint8_t m_registers[0x10]; // registers
	uint8_t m_state;           // status register
	uint8_t m_border[80];      // border color

	int m_nb_of_bitplanes;
	int m_nb_of_colors;
	int m_bitplane_xres;
	int m_bitplane_yres;
	uint16_t m_overflow_mask_x;
	uint16_t m_overflow_mask_y;
	int m_vsync_scanline_pos;

	uint8_t m_readback_latch[MAX_BITPLANES]; // Last DRAM Readback buffer (Filled after a Direct Memory Access Request command)
	int m_readback_latch_pix_offset;

	bitmap_rgb32 m_screen_out;

	// timers
	emu_timer *m_busy_timer;

	required_device<palette_device> m_palette;
	devcb_write_line m_irq_handler;
	devcb_write8 m_write_msl;
};

// device type definition
DECLARE_DEVICE_TYPE(EF9365, ef9365_device)

#endif // MAME_VIDEO_EF9365_H
