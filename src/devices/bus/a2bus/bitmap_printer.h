// license:BSD-3-Clause
// copyright-holders:
/*
 *  bitmap printer
 *
 */
#include "screen.h"

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
	const int distfrombottom = 50;  // print position from bottom of screen

	int m_xpos = 0;
	int m_ypos = 0;

	required_device<screen_device> m_screen;
	
	bitmap_rgb32 m_internal_bitmap;  // pointer to bitmap
	bitmap_rgb32 *m_bitmap = &m_internal_bitmap;  // pointer to bitmap  use internal bitmap by default
public:
	bitmap_rgb32& get_bitmap(){ return *m_bitmap; }
private:
      	
	std::string m_printername;
    std::string m_snapshotdir;
    time_t m_session_time;
	
	int m_pagecount;  // page count
	int m_pagelimit = 0;  // limit on pages (0 = no limit)
	int m_printheadcolor       = 0xEEE8AA;
	int m_printheadbordercolor = 0xBDB76B;
	int m_printheadbordersize = 5;
	int m_printheadxsize = 15;
	int m_printheadysize = 30;
	int m_distfrombottom = 50;  // print head position from bottom of screen
	int m_clearlinepos = 0;
//	int m_papercolor=0xffffff;
	int m_pagedirty = 0;
	int m_paperwidth;
	int m_paperheight;

 private:
	uint32_t screen_update_bitmap(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

public:
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
		//	y += m_paperheight;

		if (y>=m_bitmap->height()) y = 0;
		if (x>=m_bitmap->width()) x = 0;

		return m_bitmap->pix(y,x);
	};

	void setheadpos(int x, int y){m_xpos = x; m_ypos=y;}

	void bitmap_clear_band(bitmap_rgb32 &bitmap, int from_line, int to_line, u32 color);
	void bitmap_clear_band(int from_line, int to_line, u32 color);

    device_t* getrootdev();
    std::string fixchar(std::string in, char from, char to);
    std::string fixcolons(std::string in);
    std::string sessiontime();
    std::string tagname();
    std::string simplename();
    void setprintername(std::string name){ m_printername = name; }
    std::string getprintername(){ return m_printername; }
    void initprintername(){ setprintername(sessiontime() + std::string(" ") + simplename()); }

	std::string padzeroes( std::string s, int len) { return std::string(len - s.length(), '0') + s; }    
};

DECLARE_DEVICE_TYPE(BITMAP_PRINTER, bitmap_printer_device)

#endif // MAME_MACHINE_BITMAP_PRINTER_H
