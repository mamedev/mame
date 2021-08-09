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

	uint8_t *m_rom;
	uint8_t m_ram[256];

//	bitmap_rgb32 m_bitmap;

	int m_xpos = 250;
	int m_ypos = 0;

	required_device<screen_device> m_screen;
	
	bitmap_rgb32 m_lp_internal_bitmap;  // pointer to bitmap
	bitmap_rgb32 *m_lp_bitmap = &m_lp_internal_bitmap;  // pointer to bitmap  use internal bitmap by default
public:
	bitmap_rgb32& get_m_lp_bitmap(){ return *m_lp_bitmap; }
private:
      	
	std::string m_lp_luaprintername;
    std::string m_lp_snapshotdir;
    time_t m_lp_session_time;
	
	int m_lp_pagecount;  // page count
	int m_lp_pagelimit = 0;  // limit on pages (0 = no limit)
	int m_lp_printheadcolor       = 0xEEE8AA;
	int m_lp_printheadbordercolor = 0xBDB76B;
	int m_lp_printheadbordersize = 5;
	int m_lp_printheadxsize = 15;
	int m_lp_printheadysize = 30;
	int m_lp_distfrombottom = 50;  // print head position from bottom of screen
	int m_lp_clearlinepos = 0;
//	int m_lp_papercolor=0xffffff;
	int m_lp_pagedirty = 0;
	int m_lp_paperwidth;
	int m_lp_paperheight;

 private:
	uint32_t screen_update_bitmap(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	const int dpi=60;
	const int PAPER_WIDTH = 8.5 * dpi;  // 8.5 inches wide at 60 dpi
	const int PAPER_HEIGHT = 11 * dpi;   // 11  inches high at 60 dpi
	const int PAPER_SCREEN_HEIGHT = 384; // match the height of the apple II driver
	const int distfrombottom = 50;  // print position from bottom of screen

	uint32_t BITS(uint32_t x, u8 m, u8 n) {return ( ((x) >> (n)) & ( ((uint32_t) 1 << ((m) - (n) + 1)) - 1));}

	int wrap(int x, int mod) {if (x<0) return (x + ((-1 * (x / mod)) + 1) * mod) % mod; else return x % mod;}
public:
	void write_snapshot_to_file(std::string directory, std::string name);

public:


//	void drawpixel(int x, int y, int pixelval);
//	int getpixel(int x, int y);

void drawpixel(int x, int y, int pixelval)
{
	y += m_lp_paperheight;
//	m_lp_bitmap->pix32(y,x) = pixelval;
	m_lp_bitmap->pix(y,x) = pixelval;

	m_lp_pagedirty = 1;
};

int getpixel(int x, int y)
{
	y += m_lp_paperheight;
//	return m_lp_bitmap->pix32(y,x);
	return m_lp_bitmap->pix(y,x);
};

//unsigned int& pix(int x, int y)
unsigned int& pix(int y, int x)    // reversed y x
{
//	y += m_lp_paperheight;

	if (y>=m_lp_bitmap->height()) y = 0;
	if (x>=m_lp_bitmap->width()) x = 0;

	return m_lp_bitmap->pix(y,x);
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
    void setprintername(std::string name){ m_lp_luaprintername = name; }
    std::string getprintername(){ return m_lp_luaprintername; }
    void initprintername(){ 
    	//setprintername(sessiontime()+std::string(" ")+tagname());
    	setprintername(sessiontime()+std::string(" ")+simplename());  
    }

//	std::string padzeroes( std::string s, int len) { return std::string(len -s.length(), '0').append(s); }
	std::string padzeroes( std::string s, int len) { return std::string(len - s.length(), '0') + s; }

//    void initluaprinter(bitmap_rgb32 &mybitmap);
//  void setsnapshotdir(std::string dir){ m_lp_snapshotdir = dir; };
//   std::string getsnapshotdir(std::string dir){ return m_lp_snapshotdir; };

    
};

DECLARE_DEVICE_TYPE(BITMAP_PRINTER, bitmap_printer_device)

#endif // MAME_MACHINE_BITMAP_PRINTER_H
