// copyright-holder: Golden Child
/***************************************************************************

    luaprinter.h

    Utility class for printing functions

***************************************************************************/

#ifndef MAME_LUAPRINTER_H
#define MAME_LUAPRINTER_H

#pragma once

class device_luaprinter_interface : public device_interface {

public:
	device_luaprinter_interface(const machine_config &mconfig, device_t &device);

	const static int BUFFERSIZE = 128000;

	void setpagelimit(int num){ m_lp_pagelimit = num; }
	int getpagelimit(){ return m_lp_pagelimit; }
	void lp_register_bitmap(bitmap_rgb32 &mybitmap){m_lp_bitmap = &mybitmap;}
	void savepage();
	void clearpage();
	void cleartoline(int line);

	void cleartobottom() { cleartoline(m_lp_bitmap->height()-1); }
	int getclearlinepos() { return m_lp_clearlinepos; }
	void setclearlinepos(int line) { m_lp_clearlinepos=line; }
	void setclearlinebottom() { setclearlinepos(m_lp_bitmap->height()); }
	int getdistfrombottom() { return m_lp_distfrombottom; }

	void drawpixel(int x, int y, int pixelval);
	int getpixel(int x, int y);

	std::tuple<int,int>getheadpos() { return std::make_tuple(m_lp_xpos,m_lp_ypos); };
	int pagewidth() { return m_lp_bitmap->width(); }
	int pageheight() { return m_lp_bitmap->height(); }
	std::tuple<int,int> pagesize() { return std::make_tuple(m_lp_bitmap->width(), m_lp_bitmap->height()); }

	std::tuple<std::array<unsigned char,BUFFERSIZE>&,int,int>  getbuffer() {
		return std::make_tuple(std::ref(m_lp_printerbuffer),m_lp_head,m_lp_tail);
	};

	int getnextchar();
	void putnextchar(int c);

	void setheadpos(int x, int y) { m_lp_xpos=x;m_lp_ypos=y; };
	void setprintheadcolor(int headcolor, int bordcolor);
	void setprintheadsize(int xsize, int ysize, int bordersize);
	void drawprinthead(bitmap_rgb32 &bitmap, int x, int y);

	uint32_t lp_screen_update (screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	bool checkbottomofpageandsave(int y, int ybottom, bool clrpage);

	device_t* getrootdev();
	std::string fixchar(std::string in, char from, char to);
	std::string fixcolons(std::string in);
	std::string sessiontime();
	std::string tagname();
	std::string simplename();
	void setprintername(std::string name){ m_lp_luaprintername = name; }
	std::string getprintername(){ return m_lp_luaprintername; }
	void initprintername(){ setprintername(sessiontime()+std::string(" ")+tagname()); }

	void initluaprinter(bitmap_rgb32 &mybitmap);
	void setsnapshotdir(std::string dir){ m_lp_snapshotdir = dir; };
	std::string getsnapshotdir(std::string dir){ return m_lp_snapshotdir; };

	bitmap_rgb32 *m_lp_bitmap;  // pointer to bitmap

	std::array<unsigned char, BUFFERSIZE>  m_lp_printerbuffer;
	int m_lp_head = 0;  // buffer head
	int m_lp_tail = 0;  // buffer tail
	int m_lp_xpos = 0;  // head xpos
	int m_lp_ypos = 0;  // head ypos
	int m_lp_pagecount;  // page count
	int m_lp_pagelimit = 0;  // limit on pages (0 = no limit)
	int m_lp_printheadcolor       = 0xEEE8AA;
	int m_lp_printheadbordercolor = 0xBDB76B;
	int m_lp_printheadbordersize = 5;
	int m_lp_printheadxsize = 15;
	int m_lp_printheadysize = 30;
	int m_lp_distfrombottom = 50;  // print head position from bottom of screen
	int m_lp_clearlinepos = 0;
	int m_lp_papercolor=0xffffff;
	int m_lp_pagedirty = 0;

	std::string m_lp_luaprintername;
	std::string m_lp_snapshotdir;
	time_t m_lp_session_time;
};

typedef device_interface_iterator<device_luaprinter_interface> device_luaprinter_iterator;
#endif  // MAME_LUAPRINTER_H
