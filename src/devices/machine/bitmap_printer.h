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
#include "util/png.h"

#ifndef MAME_MACHINE_BITMAP_PRINTER_H
#define MAME_MACHINE_BITMAP_PRINTER_H

#pragma once

class bitmap_printer_device;
class daisywheel_bitmap_printer_device;

DECLARE_DEVICE_TYPE(BITMAP_PRINTER, bitmap_printer_device)
DECLARE_DEVICE_TYPE(DAISYWHEEL_BITMAP_PRINTER, daisywheel_bitmap_printer_device)

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

	bitmap_rgb32& get_bitmap() { return m_page_bitmap; }

protected:
	bitmap_printer_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_reset_after_children() override;
	virtual ioport_constructor device_input_ports() const override;
	virtual void device_add_mconfig(machine_config &config) override;

	bitmap_rgb32 m_page_bitmap; // page bitmap

private:
	required_device<screen_device> m_screen;
	required_device<stepper_device> m_pf_stepper;
	required_device<stepper_device> m_cr_stepper;

	required_ioport m_top_margin_ioport;
	required_ioport m_bottom_margin_ioport;
	required_ioport m_draw_marks_ioport;

	static constexpr int PAPER_SCREEN_HEIGHT = 384; // match the height of the apple II driver
	static constexpr int m_distfrombottom = 50;  // print position from bottom of screen
	static constexpr int MAX_LEDS = 5;

	int m_printhead_color;
	int m_printhead_bordercolor;
	int m_printhead_bordersize;
	int m_printhead_xsize;
	int m_printhead_ysize;
	int m_page_dirty;
protected:
	int m_paper_width;
	int m_paper_height;
	int m_hdpi;
	int m_vdpi;
private:
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




class daisywheel_bitmap_printer_device : public bitmap_printer_device
{
public:

	daisywheel_bitmap_printer_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: bitmap_printer_device(mconfig, type, tag, owner, clock)
	, m_dw_stepper(*this, "daisywheel_stepper")
	, m_wheelpos(0)
	, m_typesheet_path("")
	{
	}

	virtual void device_add_mconfig(machine_config &config) override
	{
		bitmap_printer_device::device_add_mconfig(config);
		STEPPER(config, m_dw_stepper, (uint8_t) 0xa);
	}

	daisywheel_bitmap_printer_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: daisywheel_bitmap_printer_device(mconfig, DAISYWHEEL_BITMAP_PRINTER, tag, owner, clock)
	{
	}

	daisywheel_bitmap_printer_device(const machine_config &mconfig, const char *tag, device_t *owner, int paper_width, int paper_height, int hdpi, int vdpi, std::string typesheet_path)
	: daisywheel_bitmap_printer_device(mconfig, tag, owner, u32(0))
	{

		m_paper_width = paper_width;
		m_paper_height = paper_height;
		m_hdpi = hdpi;
		m_vdpi = vdpi;

		m_typesheet_path = typesheet_path;
	}


	virtual void device_start() override
	{
		std::string fullname = m_typesheet_path;
		std::error_condition filerr;
		util::core_file::ptr file;

		/* open the file */
		filerr = util::core_file::open(fullname, OPEN_FLAG_READ, file);

		/* if that worked, load the file */
		if (!filerr)
		{
				util::png_read_bitmap(*file, m_typesheet);
				file.reset();
		}
		bitmap_printer_device::device_start();
	}

private:
	required_device<stepper_device> m_dw_stepper;
	bitmap_argb32 m_typesheet;


	int mod_positive(uint16_t num, uint16_t mod_value)
	{
		int retvalue = num % mod_value;  if (retvalue < 0) retvalue += mod_value; return retvalue;
	}
public:
	int m_wheelpos;
private:
	std::string m_typesheet_path;
public:
	void stamp(u8 charnum)
	{
		if (m_typesheet.width() != 0)
		{
			int horigin = ((u32 *) m_typesheet.raw_pixptr(0,0))[0] & 0xffffff;
			int vorigin = ((u32 *) m_typesheet.raw_pixptr(0,0))[1] & 0xffffff;
			int hsize =   ((u32 *) m_typesheet.raw_pixptr(0,0))[2] & 0xffffff;
			int vsize =   ((u32 *) m_typesheet.raw_pixptr(0,0))[3] & 0xffffff;

			int i = charnum;

			int srcx1 = hsize * ((i-32) % 16) + horigin;
			int srcy1 = vsize * ((i-32) / 16) + vorigin;

			int destx1 = m_xpos;
			int destx2 = destx1 + hsize - 1;
			if (destx2 > m_page_bitmap.width()) destx2 = m_page_bitmap.width();
			int desty1 = m_ypos;
			int desty2 = desty1 + vsize;
			if (desty2 > m_page_bitmap.height()) desty2 = m_page_bitmap.height();

			rectangle m_clip(destx1, destx2, desty1, desty2);
			copybitmap_trans(m_page_bitmap, (bitmap_rgb32&) m_typesheet, 0, 0,
				destx1 - srcx1, desty1 - srcy1, m_clip, 0xfffffffe);  // FFFFFFFF treated as no transparency special case
		}
	}

	void update_dw_stepper(int pattern)
	{
		m_dw_stepper->update(pattern);

		// wrap daisywheel around
		if (m_dw_stepper->get_absolute_position() >= 96 * 2)
			m_dw_stepper->set_absolute_position(m_dw_stepper->get_absolute_position() - 96 * 2);

		if (m_dw_stepper->get_absolute_position() < 0)
			m_dw_stepper->set_absolute_position(m_dw_stepper->get_absolute_position() + 96 * 2);

		m_wheelpos = m_dw_stepper->get_absolute_position();
	}
};

#endif // MAME_MACHINE_BITMAP_PRINTER_H
