// license:BSD-3-Clause
// copyright-holders: Golden Child
/*
   bitmap printer (dot printer)

    *   provides a page bitmap to draw on
    *   reads and writes pixels (representing printer dots)
    *   function to save the bitmap
    *   updates the bitmap to screen and draws the printhead
    *   printhead position given in m_xpos and m_ypos
    *   also provides a cr_stepper and a pf_stepper
    *   moving the cr_stepper/pf_stepper will update m_xpos/m_ypos according to ratio specified

 */

#include "screen.h"
#include "machine/steppers.h"

#ifndef MAME_MACHINE_BITMAP_PRINTER_H
#define MAME_MACHINE_BITMAP_PRINTER_H

#pragma once

class bitmap_printer_device : public device_t
{
public:
	bitmap_printer_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	bitmap_printer_device(const machine_config &mconfig, const char *tag, device_t *owner, int paper_width, int paper_height, int hdpi, int vdpi);

	enum
	{
		LED_ERROR,
		LED_READY,
		LED_ONLINE
	};

	void set_led_state(int led, int value);
	void set_printhead_color(int headcolor, int bordcolor);
	void set_printhead_size(int xsize, int ysize, int bordersize);
	void setheadpos(int x, int y);

	void write_snapshot_to_file();

	void draw_pixel(int x, int y, int pixelval);
	int get_pixel(int x, int y);
	unsigned int &pix(int y, int x);

	void bitmap_clear_band(bitmap_rgb32 &bitmap, int from_line, int to_line, u32 color);
	void bitmap_clear_band(int from_line, int to_line, u32 color);
	void clear_to_pos(int to_line, u32 color = 0xffffff);

	int get_top_margin();
	int get_bottom_margin();
	bool check_new_page();

	int update_stepper_delta(stepper_device *stepper, uint8_t pattern);
	void update_cr_stepper(int pattern);
	void update_pf_stepper(int pattern);

	void set_pf_stepper_ratio(int ratio0, int ratio1);
	void set_cr_stepper_ratio(int ratio0, int ratio1);

	int m_cr_direction; // direction of carriage
	int m_xpos;
	int m_ypos;

protected:
	bitmap_printer_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_reset_after_children() override;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

private:
	required_device<screen_device> m_screen;
	required_device<stepper_device> m_pf_stepper;
	required_device<stepper_device> m_cr_stepper;

	required_ioport m_top_margin_ioport;
	required_ioport m_bottom_margin_ioport;
	required_ioport m_draw_marks_ioport;

	bitmap_rgb32 m_page_bitmap; // page bitmap

	static constexpr int PAPER_SCREEN_HEIGHT = 384; // match the height of the apple II driver
	static constexpr int m_distfrombottom = 50;  // print position from bottom of screen
	static constexpr int MAX_LEDS = 5;

	int m_printhead_color;
	int m_printhead_bordercolor;
	int m_printhead_bordersize;
	int m_printhead_xsize;
	int m_printhead_ysize;
	int m_page_dirty;
	int m_paper_width;
	int m_paper_height;
	int m_hdpi;
	int m_vdpi;
	int m_clear_pos;
	int m_newpage_flag;  // used to keep printhead at the top of page until actual printing
	int m_led_state[MAX_LEDS];
	int m_num_leds;
	int m_pf_stepper_ratio0;
	int m_pf_stepper_ratio1;
	int m_cr_stepper_ratio0;
	int m_cr_stepper_ratio1;

	void draw_printhead(bitmap_rgb32 &bitmap, int x, int y);
	u32 dimcolor(u32 incolor, int factor);

	int calc_scroll_y(bitmap_rgb32& bitmap);
	uint32_t screen_update_bitmap(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	void draw7seg(u8 data, bool is_digit, int x0, int y0, int width, int height, int thick, bitmap_rgb32 &bitmap, u32 color, u32 erasecolor);
	void draw_number(int number, int x, int y, bitmap_rgb32& bitmap);
	void draw_inch_marks(bitmap_rgb32& bitmap);
};

DECLARE_DEVICE_TYPE(BITMAP_PRINTER, bitmap_printer_device)

#endif // MAME_MACHINE_BITMAP_PRINTER_H
