// license:BSD-3-Clause
// copyright-holders:
/*
 *  bitmap printer
 *
 */
#include "screen.h"
#include "session_time.h"

#ifndef MAME_MACHINE_BITMAP_PRINTER_H
#define MAME_MACHINE_BITMAP_PRINTER_H

#pragma once

class bitmap_printer_device : public device_t
{
public:
	bitmap_printer_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	bitmap_printer_device(const machine_config &mconfig, const char *tag, device_t *owner, int paperwidth, int paperheight) :
		bitmap_printer_device(mconfig, tag, owner, u32(0))
	{
		m_paperwidth = paperwidth;
		m_paperheight = paperheight;
	}

protected:
	bitmap_printer_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

public:

protected:
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

	bitmap_rgb32 m_internal_bitmap;  // pointer to bitmap
	bitmap_rgb32 *m_bitmap = &m_internal_bitmap;  // pointer to bitmap  use internal bitmap by default
public:
	bitmap_rgb32& get_bitmap(){ return *m_bitmap; }
private:

	std::string m_printername;
	std::string m_snapshotdir;
//  time_t m_session_time;

	int m_printheadcolor       = 0xEEE8AA;
	int m_printheadbordercolor = 0xBDB76B;
	int m_printheadbordersize = 3;
	int m_printheadxsize = 10;
	int m_printheadysize = 20;
//  int m_papercolor=0xffffff;
	int m_pagedirty = 0;
	int m_paperwidth;
	int m_paperheight;

public:
void setprintheadcolor(int headcolor, int bordcolor);
void setprintheadsize(int xsize, int ysize, int bordersize);


private:
void drawprinthead(bitmap_rgb32 &bitmap, int x, int y);


 private:
	uint32_t screen_update_bitmap(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

public:

	session_time_device * get_session_time_device() {return m_session_time;}

	void write_snapshot_to_file(std::string directory, std::string name);

	void drawpixel(int x, int y, int pixelval)
	{
		y += m_paperheight;
		m_bitmap->pix(y,x) = pixelval;

		m_pagedirty = 1;
	};

	int getpixel(int x, int y)
	{
		y += m_paperheight;
		return m_bitmap->pix(y,x);
	};

	unsigned int& pix(int y, int x)    // reversed y x
	{
		//  y += m_paperheight;

		if (y>=m_bitmap->height()) y = 0;
		if (x>=m_bitmap->width()) x = 0;

		return m_bitmap->pix(y,x);
	};


	int calc_scroll_y(bitmap_rgb32& bitmap)
	{
		return bitmap.height() - m_distfrombottom - m_ypos;
	}


	void draw7seg(u8 data, bool is_digit, int x0, int y0, int width, int height, int thick, bitmap_rgb32 &bitmap, u32 color, u32 erasecolor)
	{
		// pass nonzero erasecolor to erase blank segments
		const u8 pat[] = { 0x3f, 0x06,  0x5b, 0x4f, 0x66, 0x6d, 0x7d, 0x07, 0x7f, 0x6f, 0x77, 0x7c, 0x39, 0x5e, 0x79, 0x71 };
		u8 seg = is_digit ? pat[data & 0xf] : data;

		if (BIT(seg,0) || erasecolor) bitmap.plot_box(x0,       y0,                  width, thick,       BIT(seg,0) ? color : erasecolor);
		if (BIT(seg,1) || erasecolor) bitmap.plot_box(x0+width, y0+thick,            thick, height,      BIT(seg,1) ? color : erasecolor);
		if (BIT(seg,2) || erasecolor) bitmap.plot_box(x0+width, y0+2*thick+height,   thick, height,      BIT(seg,2) ? color : erasecolor);
		if (BIT(seg,3) || erasecolor) bitmap.plot_box(x0,       y0+2*thick+2*height, width, thick,       BIT(seg,3) ? color : erasecolor);
		if (BIT(seg,4) || erasecolor) bitmap.plot_box(x0-thick, y0+2*thick+height,   thick, height,      BIT(seg,4) ? color : erasecolor);
		if (BIT(seg,5) || erasecolor) bitmap.plot_box(x0-thick, y0+thick,            thick, height,      BIT(seg,5) ? color : erasecolor);
		if (BIT(seg,6) || erasecolor) bitmap.plot_box(x0,       y0+thick+height,     width, thick,       BIT(seg,6) ? color : erasecolor);
		if (BIT(seg,7) || erasecolor) bitmap.plot_box(x0+width+thick, y0+2*thick+2*height, thick, thick, BIT(seg,7) ? color : erasecolor); // draw dot
	}

	void draw_number(int number, int x, int y, bitmap_rgb32& bitmap)
	{
		std::string s(std::to_string(number));

		int width = 8;
		int height = 6;
		int thick = 2;

		for (int i = s.length()-1; i>=0; i--)
			draw7seg( s.at(i)-0x30, true,
						x + ( + i - s.length()) * (3 * width) - (width), y + height * 3 / 2,
						width, height, thick, bitmap, 0x000000, 0);
	}

	void draw_inch_marks(bitmap_rgb32& bitmap)
	{
		int drawmarks = ioport("DRAWMARKS")->read();
		if (!drawmarks) return;

		int vdpi = 144;
		for (int i = 0; i < vdpi * 11; i += vdpi / 4)
		{
			int adj_i = i + calc_scroll_y(bitmap) % m_paperheight;  // modding the bitmap height, not the paper height
			int barbase = vdpi / 6;
			int barwidth = ((i % vdpi) == 0) ? barbase * 2 : barbase;
			int barcolor = ((i % vdpi) == 0) ? 0x202020 : 0xc0c0c0;
			if (adj_i < bitmap.height())
			{
				bitmap.plot_box(bitmap.width() - 1 - barwidth, adj_i, barwidth, 1, barcolor);
				if ((i % vdpi) == 0)
				{
					double pct = (double) i / (vdpi * 11.0);
					if (drawmarks == 3)  // draw position bar
					{
						int barheight = 3;
						if (adj_i < bitmap.height() - 1 - barheight)
							// little dot to show paper position
							bitmap.plot_box( bitmap.width() - 1 - barwidth , adj_i, pct * barwidth, barheight, 0x000000);
					}
					if (drawmarks == 2)  // draw position mark
					{
						int marksize = 3;
						int barheight = 4;
						if (adj_i < bitmap.height() - 1 - barheight)
							// little bar to show paper position
							bitmap.plot_box( bitmap.width() - 1 - barwidth +  pct * barwidth - marksize, adj_i, marksize, barheight, 0x000000);
					}
					if (drawmarks & 4)
						draw_number(i / vdpi, bitmap.width(), adj_i, bitmap);
				}

			}
//          else break;
		}
	}

	void setheadpos(int x, int y){m_xpos = x; m_ypos=y;}

	void bitmap_clear_band(bitmap_rgb32 &bitmap, int from_line, int to_line, u32 color);
	void bitmap_clear_band(int from_line, int to_line, u32 color);
	std::string padzeroes( std::string s, int len) { return std::string(len - s.length(), '0') + s; }
};

DECLARE_DEVICE_TYPE(BITMAP_PRINTER, bitmap_printer_device)

#endif // MAME_MACHINE_BITMAP_PRINTER_H
