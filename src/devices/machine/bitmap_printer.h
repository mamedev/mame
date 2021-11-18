// license:BSD-3-Clause
// copyright-holders:
/*
   bitmap printer

	*	provides a page bitmap to draw on
	*	reads and writes pixels (representing printer dots)
	*	function to save the bitmap
	*	updates the bitmap to screen and draws the printhead

 */
#include "screen.h"
#include "machine/session_time.h"

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

	const int PAPER_SCREEN_HEIGHT = 384; // match the height of the apple II driver
	const int m_distfrombottom = 50;  // print position from bottom of screen

	int m_xpos = 0;
	int m_ypos = 0;

	required_device<screen_device> m_screen;
	required_device<session_time_device> m_session_time;

	bitmap_rgb32  m_internal_bitmap;  // internal bitmap
	bitmap_rgb32* m_bitmap = &m_internal_bitmap;  // pointer to bitmap, use internal bitmap by default

	std::string m_printername;
	std::string m_snapshotdir;

	int m_printheadcolor       = 0xEEE8AA;
	int m_printheadbordercolor = 0xBDB76B;
	int m_printheadbordersize = 3;
	int m_printheadxsize = 10;
	int m_printheadysize = 20;
	int m_pagedirty = 0;
	int m_paperwidth;
	int m_paperheight;
	int m_hdpi;
	int m_vdpi;
	int clear_pos = 0;

public:
	bitmap_rgb32& get_bitmap(){ return *m_bitmap; }

	void setprintheadcolor(int headcolor, int bordcolor);
	void setprintheadsize(int xsize, int ysize, int bordersize);

	session_time_device* get_session_time_device() {return m_session_time;}

	void write_snapshot_to_file(std::string directory, std::string name);

	void drawpixel(int x, int y, int pixelval);
	int getpixel(int x, int y);
	unsigned int& pix(int y, int x);
	void setheadpos(int x, int y){  if (m_xpos != x) newpageflag = 0; m_xpos = x; m_ypos=y;}
	void bitmap_clear_band(bitmap_rgb32 &bitmap, int from_line, int to_line, u32 color);
	void bitmap_clear_band(int from_line, int to_line, u32 color);
	std::string padzeroes( std::string s, int len) { return std::string(len - s.length(), '0') + s; }
	int get_top_margin();
	int get_bottom_margin();
	bool check_new_page();
	bool check_ypos() { return 0; }
	void clear_to_pos(int to_line, u32 color = 0xffffff);
	int newpageflag = 0;
private:
	void drawprinthead(bitmap_rgb32 &bitmap, int x, int y);

	uint32_t screen_update_bitmap(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	int calc_scroll_y(bitmap_rgb32& bitmap);

	void draw7seg(u8 data, bool is_digit, int x0, int y0, int width, int height, int thick, bitmap_rgb32 &bitmap, u32 color, u32 erasecolor);
	void draw_number(int number, int x, int y, bitmap_rgb32& bitmap);
	void draw_inch_marks(bitmap_rgb32& bitmap);

};

DECLARE_DEVICE_TYPE(BITMAP_PRINTER, bitmap_printer_device)

#endif // MAME_MACHINE_BITMAP_PRINTER_H
