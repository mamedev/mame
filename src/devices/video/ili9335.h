// license:BSD-3-Clause
// copyright-holders:grubbyplaya
/***************************************************************************

        Ilitek ILI9335 LCD controller

***************************************************************************/

#ifndef MAME_VIDEO_ILI9335_H
#define MAME_VIDEO_ILI9335_H

#pragma once

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> ili9335_device

class ili9335_device : public device_t
{
public:
	// construction/destruction
	ili9335_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// device interface
	void control_write(uint8_t data);
	uint8_t control_read();
	void data_write(uint8_t data);
	uint8_t data_read();

    void set_screen(screen_device &screen);
	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
    uint16_t m_lcdregs[256];
    uint8_t m_gram[(320 * 240 * 18) / 8];

    uint8_t m_readlatch, m_writelatch, m_currreg;
    uint32_t m_datalatch;
    uint16_t m_currcol, m_currrow, m_height, m_width;

    screen_device *m_screen = nullptr;

    void write_reg(uint16_t val);
    bool update_cursor_reg(uint16_t *cursor, uint16_t start, uint16_t end, uint8_t flag);
    void reset_cursor_regs(uint16_t mode, bool reset_x, bool reset_y);

    uint32_t get_gram_data(uint32_t addr);
    uint32_t get_pixel(uint16_t x, uint16_t y);
    uint16_t rgb666_to_rgb565(uint32_t pixel);
    uint32_t rgb666_swap_bgr(uint32_t pixel);

    void write_pixel_565(uint16_t pixel);
    void write_pixel_666(uint32_t pixel);
    void write_pixel_666_unpacked(uint32_t pixel);

    void draw_interlaced_frame(bitmap_rgb32 &bitmap);
    void draw_partial_image(bitmap_rgb32 &bitmap, uint16_t disp_pos, uint16_t start_line, uint16_t end_line);
    void plot_pixel(bitmap_rgb32 &bitmap, uint32_t pixel18, uint8_t x, uint16_t y);
};

// device type definition
DECLARE_DEVICE_TYPE(ILI9335, ili9335_device)

#endif // MAME_VIDEO_ILI9335_H
