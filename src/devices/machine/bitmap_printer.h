// license:BSD-3-Clause
// copyright-holders:
/*
   bitmap printer

	*	provides a page bitmap to draw on
	*	reads and writes pixels (representing printer dots)
	*	function to save the bitmap
	*	updates the bitmap to screen and draws the printhead
	*	printhead position given in m_xpos and m_ypos
	*	also provides a cr_stepper and a pf_stepper

 */
#include "screen.h"
#include "machine/session_time.h"
#include "machine/steppers.h"

#ifndef MAME_MACHINE_BITMAP_PRINTER_H
#define MAME_MACHINE_BITMAP_PRINTER_H

#pragma once

class bitmap_printer_device : public device_t
{
public:
	bitmap_printer_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	bitmap_printer_device(const machine_config &mconfig, const char *tag, device_t *owner, int paperwidth, int paperheight, int hdpi, int vdpi) :
		bitmap_printer_device(mconfig, tag, owner, u32(0))
	{
		m_paperwidth = paperwidth;
		m_paperheight = paperheight;
		m_hdpi = hdpi;
		m_vdpi = vdpi;
	}

protected:
	bitmap_printer_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_reset_after_children() override;
	virtual ioport_constructor device_input_ports() const override;
	virtual void device_add_mconfig(machine_config &config) override;

private:
	required_device<screen_device> m_screen;
	required_device<session_time_device> m_session_time;
public:
	required_device<stepper_device> m_pf_stepper;
	required_device<stepper_device> m_cr_stepper;
	
	int m_cr_direction = 1; // direction of carriage
	int m_pf_stepper_ratio0 = 1;
	int m_pf_stepper_ratio1 = 1;
	int m_cr_stepper_ratio0 = 1;
	int m_cr_stepper_ratio1 = 1;
	int m_xpos = 0;
	int m_ypos = 0;

private:	
	bitmap_rgb32  m_internal_bitmap;  // internal bitmap
	bitmap_rgb32* m_bitmap = &m_internal_bitmap;  // pointer to bitmap, use internal bitmap by default

	const int PAPER_SCREEN_HEIGHT = 384; // match the height of the apple II driver
	const int m_distfrombottom = 50;  // print position from bottom of screen

	int m_printhead_color       = 0xEEE8AA;
	int m_printhead_bordercolor = 0xBDB76B;
	int m_printhead_bordersize = 3;
	int m_printhead_xsize = 10;
	int m_printhead_ysize = 20;
	int m_pagedirty = 0;
	int m_paperwidth;
	int m_paperheight;
	int m_hdpi;
	int m_vdpi;
	int clear_pos = 0;
	int newpageflag = 0;  // used to keep printhead at the top of page until actual printing
	
public:
	bitmap_rgb32& get_bitmap(){ return *m_bitmap; }

	void set_printhead_color(int headcolor, int bordcolor);
	void set_printhead_size(int xsize, int ysize, int bordersize);
	void setheadpos(int x, int y){  if (m_xpos != x) newpageflag = 0; m_xpos = x; m_ypos = y;}
	
	session_time_device* get_session_time_device() {return m_session_time;}

	void write_snapshot_to_file(std::string directory, std::string name);

	void draw_pixel(int x, int y, int pixelval);
	int get_pixel(int x, int y);
	unsigned int& pix(int y, int x);

	void bitmap_clear_band(bitmap_rgb32 &bitmap, int from_line, int to_line, u32 color);
	void bitmap_clear_band(int from_line, int to_line, u32 color);
	void clear_to_pos(int to_line, u32 color = 0xffffff);

	std::string padzeroes( std::string s, int len) { return std::string(len - s.length(), '0') + s; }

	int get_top_margin();
	int get_bottom_margin();
	bool check_new_page();

	int update_stepper_delta(stepper_device * stepper, uint8_t pattern);	
	void update_cr_stepper(int pattern);
	void update_pf_stepper(int pattern);

	void set_pf_stepper_ratio(int ratio0, int ratio1) { m_pf_stepper_ratio0 = ratio0; m_pf_stepper_ratio1 = ratio1;}
	void set_cr_stepper_ratio(int ratio0, int ratio1) { m_cr_stepper_ratio0 = ratio0; m_cr_stepper_ratio1 = ratio1;}
private:
	void draw_printhead(bitmap_rgb32 &bitmap, int x, int y);

	int calc_scroll_y(bitmap_rgb32& bitmap);
	uint32_t screen_update_bitmap(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	void draw7seg(u8 data, bool is_digit, int x0, int y0, int width, int height, int thick, bitmap_rgb32 &bitmap, u32 color, u32 erasecolor);
	void draw_number(int number, int x, int y, bitmap_rgb32& bitmap);
	void draw_inch_marks(bitmap_rgb32& bitmap);
};

DECLARE_DEVICE_TYPE(BITMAP_PRINTER, bitmap_printer_device)


#endif // MAME_MACHINE_BITMAP_PRINTER_H
