// copyright-holder: Golden Child
/***************************************************************************

    luaprinter.h

    Utility class for printing functions

***************************************************************************/

#ifndef MAME_LUAPRINTER_H
#define MAME_LUAPRINTER_H

#pragma once

#include "emu.h"
#include "png.h"
#include "emuopts.h"

class luaprinter {

public:
	luaprinter(device_t& thisdevice, std::string dummyname);
	~luaprinter() { lp_removefromprinterlist(); }

	device_t *m_lp_mydevice;
	static std::vector<luaprinter *> luaprinterlist;
    const static int BUFFERSIZE = 128000;
    std::array<unsigned char, BUFFERSIZE>  m_lp_printerbuffer;
    int m_lp_head = 0;
    int m_lp_tail = 0;
    int m_lp_xpos = 0;
    int m_lp_ypos = 0;
	int m_lp_pagecount;
	int m_lp_pagelimit = 0;
	int m_lp_printheadcolor       = 0xEEE8AA;
	int m_lp_printheadbordercolor = 0xBDB76B;
	int m_lp_printheadbordersize = 5;
	int m_lp_printheadxsize = 15;
	int m_lp_printheadysize = 30;
	int m_lp_distfrombottom = 50;
	int m_lp_clearlinepos = 0;
	int m_lp_papercolor=0xffffff;
	int m_lp_pagedirty = 0;
    bitmap_rgb32 *m_lp_bitmap;  // pointer to bitmap
    std::string m_lp_luaprintername;
    std::string m_lp_snapshotdir;
	static time_t m_lp_session_time;

	void lp_addtoprinterlist(){ luaprinterlist.emplace_back(& (*this)); }
	void lp_removefromprinterlist() {
		luaprinterlist.erase(std::remove_if(luaprinterlist.begin(), luaprinterlist.end(),
				[this](luaprinter *item){return item==this;}), luaprinterlist.end());
	}
	int count(){ return luaprinterlist.size();}

	void setpagelimit(int num){ m_lp_pagelimit = num; }
	int getpagelimit(){ return m_lp_pagelimit; }
    void setprintername(std::string name){ m_lp_luaprintername = name; }
    std::string getprintername(){ return m_lp_luaprintername; }

    void lp_register_bitmap(bitmap_rgb32 &mybitmap){m_lp_bitmap = &mybitmap;}
    void savepage();
    void clearpage(){ m_lp_bitmap->fill(m_lp_papercolor); cleartoline(0); m_lp_pagedirty = 0;};
	void cleartoline(int line) {
		if (line >= m_lp_clearlinepos) {
			m_lp_bitmap->plot_box(0, m_lp_clearlinepos, m_lp_bitmap->width(), line-m_lp_clearlinepos, m_lp_papercolor);
		}
			m_lp_clearlinepos=line;
		}
    void cleartobottom() { cleartoline( m_lp_bitmap->height()-1 ); }
    int getclearlinepos() { return m_lp_clearlinepos; }
    void setclearlinepos(int line) { m_lp_clearlinepos=line; }
    void setclearlinebottom() { setclearlinepos(m_lp_bitmap->height()); }
    int getdistfrombottom() { return m_lp_distfrombottom; }
    void drawpixel(int x, int y, int pixelval) { m_lp_bitmap->pix32(y,x) = pixelval; m_lp_pagedirty = 1; };
    int getpixel(int x, int y) { return m_lp_bitmap->pix32(y,x); };

    void setheadpos(int x, int y) { m_lp_xpos=x;m_lp_ypos=y; };
    std::tuple<int,int>getheadpos() { return std::make_tuple(m_lp_xpos,m_lp_ypos); };
	int pagewidth() { return m_lp_bitmap->width(); }
	int pageheight() { return m_lp_bitmap->height(); }
	std::tuple<int,int> pagesize() { return std::make_tuple(m_lp_bitmap->width(), m_lp_bitmap->height()); }

    std::tuple<std::array<unsigned char,BUFFERSIZE>&,int,int>  getbuffer() {
		return std::make_tuple(std::ref(m_lp_printerbuffer),m_lp_head,m_lp_tail); };
	int getnextchar();
	void putnextchar(int c);
    void setprintheadcolor(int headcolor, int bordcolor) {
		m_lp_printheadcolor = headcolor;
		m_lp_printheadbordercolor = bordcolor;
	}
    void setprintheadsize(int xsize, int ysize, int bordersize) {
		m_lp_printheadxsize = xsize;
		m_lp_printheadysize = ysize;
		m_lp_printheadbordersize = bordersize;
	}
    void drawprinthead(bitmap_rgb32 &bitmap, int x, int y);
    uint32_t lp_screen_update (screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	bool checkbottomofpageandsave(int y, int ybottom, bool clrpage);
	device_t* getrootdev();
	std::string fixchar(std::string in, char from, char to);
	std::string fixcolons(std::string in);
	std::string sessiontime();
	std::string tagname();
	std::string simplename();
	void initprintername(){ setprintername(sessiontime()+std::string(" ")+tagname()); }
	void initluaprinter(bitmap_rgb32 &mybitmap) {
		lp_addtoprinterlist();
		lp_register_bitmap(mybitmap);
		setsnapshotdir(std::string(m_lp_mydevice->machine().options().snapshot_directory()));
	}
	void setsnapshotdir(std::string dir){ m_lp_snapshotdir = dir; };
	std::string getsnapshotdir(std::string dir){ return m_lp_snapshotdir; };
};

#endif  // MAME_LUAPRINTER_H
