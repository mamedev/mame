// copyright-holder: Golden Child
/***************************************************************************

    luaprinter.cpp

    Utility class for printing functions

***************************************************************************/

#include "emu.h"
#include "luaprinter.h"
#include "screen.h"

device_luaprinter_interface::device_luaprinter_interface(const machine_config &mconfig, device_t &device) : device_interface(device, "luaprinter")
{
	m_lp_mydevice = &device;
	time(&m_lp_session_time);
	initprintername();
};

int device_luaprinter_interface::getnextchar() {
	if (m_lp_head==m_lp_tail) return -1;
	else {
		int retval = m_lp_printerbuffer.at(m_lp_tail++);
		m_lp_tail %= BUFFERSIZE;
		return retval;
	}
}

void device_luaprinter_interface::putnextchar(int c) {
	m_lp_printerbuffer.at(m_lp_head++) = c;
	m_lp_head %= BUFFERSIZE;
}

std::string device_luaprinter_interface::fixchar(std::string in, char from, char to) {
    std::string final;
    for(std::string::const_iterator it = in.begin(); it != in.end(); ++it)
    {
        if((*it) != from)
        {
            final += *it;
        }
        else final += to;
    }
    return final;
}

std::string device_luaprinter_interface::fixcolons(std::string in) { return fixchar(in, ':', '-'); }

std::string device_luaprinter_interface::sessiontime() {
	struct tm *info;
	char buffer[80];
	info = localtime( &m_lp_session_time );
	strftime(buffer,120,"%Y-%m-%d %H-%M-%S", info);
	return std::string(buffer);
}

std::string device_luaprinter_interface::tagname() {
	return fixcolons(std::string(getrootdev()->shortname())+std::string(m_lp_mydevice->tag()));
}

std::string device_luaprinter_interface::simplename() {
	device_t * dev;
	dev = m_lp_mydevice;
	std::string s(dev->owner()->shortname());
	while (dev){
		s=std::string(dev->shortname())+std::string(" ")+s;
		dev=dev->owner();
	}
	return s;
}

device_t* device_luaprinter_interface::getrootdev(){
	device_t* dev;
	device_t* lastdev = NULL;
	dev = m_lp_mydevice;
	while (dev){
		lastdev = dev;
		dev=dev->owner();
	}
	return lastdev;
}

void device_luaprinter_interface::drawprinthead(bitmap_rgb32 &bitmap, int x, int y) {
	int bordx = m_lp_printheadbordersize;
	int bordy = m_lp_printheadbordersize;
	int offy = 9 + bordy;
	int sizex = m_lp_printheadxsize;
	int sizey = m_lp_printheadysize;
	bitmap.plot_box(x-sizex/2-bordx, y+offy-bordy, sizex+2*bordx, sizey+bordy*2, m_lp_printheadbordercolor);
	bitmap.plot_box(x-sizex/2,       y+offy,       sizex,         sizey,         m_lp_printheadcolor);
}

uint32_t device_luaprinter_interface::lp_screen_update (screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect) {
	int scrolly=bitmap.height() - m_lp_distfrombottom - m_lp_ypos;
	copyscrollbitmap(bitmap, *m_lp_bitmap, 0, nullptr, 1, &scrolly, cliprect);
	drawprinthead(bitmap, m_lp_xpos, bitmap.height()-m_lp_distfrombottom);
	bitmap.plot_box(0, bitmap.height() - m_lp_ypos-m_lp_distfrombottom, bitmap.width(), 2, 0xD8CAA4);  // top of page line
	bitmap.plot_box(0, screen.height() - m_lp_ypos-m_lp_distfrombottom+m_lp_bitmap->height() - 2, bitmap.width(), 2, 0xD80000);  // bottom of page line
	return 0;
}

bool device_luaprinter_interface::checkbottomofpageandsave(int y, int ybottom, bool clrpage) {
	if ( y >= ybottom ) {
		if ((m_lp_pagelimit == 0) || (m_lp_pagecount < m_lp_pagelimit))
			savepage();
		else //  page limit to stop runaway generation of pages
			printf("Page %d exceeded page limit: %d\n", m_lp_pagecount, m_lp_pagelimit);
		if (clrpage) {
			// clearpage();
			cleartoline(0);
			cleartobottom();  // if you want to manage page clearing yourself, do cleartoline(0) to reset the clearline to the top
		}
		return true;
	}
	else return false;
}

void device_luaprinter_interface::savepage(){
	if (!m_lp_bitmap) return;
	emu_file file(m_lp_snapshotdir + std::string("/") +
				std::string(getrootdev()->shortname() ) + std::string("/"),
				OPEN_FLAG_WRITE | OPEN_FLAG_CREATE | OPEN_FLAG_CREATE_PATHS);
    auto const filerr = file.open(std::string(getprintername())+" Page "+std::to_string(m_lp_pagecount++)+".PNG");

    if (filerr == osd_file::error::NONE)
        {
			cleartobottom();  // clear to the end of the page
			static const rgb_t png_palette[] = { rgb_t::white(), rgb_t::black() };
			png_write_bitmap(file, nullptr, (bitmap_t &) (* m_lp_bitmap), 2, png_palette);
        }
}

