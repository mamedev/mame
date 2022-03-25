// license:BSD-3-Clause
// copyright-holders: Fabio Dalla Libera
/*
  Based on bitmap_printer.cpp

  TODO: - save snapshots a vector graphics
  - optimize copybitmapregion


*/

#include <algorithm>
#include "plotter.h"
#include "emuopts.h"
#include "fileio.h"
#include "png.h"


void draw_with_thickness(bitmap_rgb32 &bitmap,generic_plotter_device::Position pos,int thickness,uint32_t col)
{
	if (thickness<=0) return;
	int left = std::clamp(pos.x-thickness/2, 0, bitmap.width()-1);
	int right = std::clamp(pos.x+thickness/2, 0, bitmap.width()-1);
	int top = std::clamp(pos.y-thickness/2, 0, bitmap.height()-1);
	int bottom = std::clamp(pos.y+thickness/2, 0, bitmap.height()-1);
	bitmap.plot_box(left, top, right-left+1, bottom-top+1, col);
}


void copybitmapregion(bitmap_rgb32 &dest,const bitmap_rgb32 &src,const generic_plotter_device::Position& destp, const generic_plotter_device::Position& srcp, int32_t width, int32_t height,const rectangle &cliprect )
{
	int32_t minx = std::max(std::max(0, destp.x-srcp.x),
			      std::max(destp.x, cliprect.min_x)
		); //assume arguments may be negative
	int32_t miny = std::max(std::max(0, destp.y-srcp.y),
			      std::max(destp.y, cliprect.min_y)
		);
	int32_t maxx = std::min(std::min(dest.width(), src.width()+destp.x-srcp.x),
			      std::min(destp.x+width, cliprect.max_x+1)
		);
	int32_t maxy = std::min(std::min(dest.height(), src.height()+destp.y-srcp.y),
			      std::min(destp.y+height, cliprect.max_y+1)
		);

	copybitmap(dest, src, 0, 0, destp.x-srcp.x, destp.y-srcp.y, rectangle(minx, maxx, miny, maxy));
}


generic_plotter_device::Element::Element(int w,int h,uint32_t c):
	width(w),height(h),color(c)
{
}

generic_plotter_device::Position::Position(int _x,int _y):
	x(_x),y(_y)
{
}

generic_plotter_device::Position generic_plotter_device::Position::operator-(const Position& b)
{
	return Position(x-b.x, y-b.y);
}

generic_plotter_device::Position generic_plotter_device::Position::operator+(const Element& b)
{
	return Position(x+b.width, y+b.height);
}


generic_plotter_device::generic_plotter_device(const machine_config &mconfig,  const char *tag, device_t *owner, uint32_t clock):
	generic_plotter_device(mconfig, GENERIC_PLOTTER, tag, owner, Element(), Element(), Position(), Position(), 1, 1)
{
}

generic_plotter_device::generic_plotter_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, const Element& view, const Element& paperwin,const Position& paperwinpos, const Position & motorpos,  double xscale, double yscale):
	device_t(mconfig, type, tag, owner, 0),
	m_xstepper(*this, "xstepper"),
	m_ystepper(*this, "ystepper"),
	m_screen(*this, "screen"),
	m_view(view),
	m_paperwin(paperwin),
	m_paperwinpos(paperwinpos),
	m_paperorigin(paperwinpos),
	m_motorpos(motorpos),
	m_xscale(xscale),m_yscale(yscale),
	m_pendown(false),
	m_headcolor(0x00FFFF),
	m_headsize(10),
	m_pencolor(0x000000),
	m_penthickness(1),
	m_panel_update(*this)
{
}

void generic_plotter_device::device_start()
{
	m_paper_bitmap.allocate(m_paperwin.width, m_paperwin.height);
	m_paper_bitmap.fill(m_paperwin.color);
}

void generic_plotter_device::set_pencolor(uint32_t col)
{
	m_pencolor = col;
}

void generic_plotter_device::set_penthickness(int t)
{
	m_penthickness = t;
}

void generic_plotter_device::set_headcolor(uint32_t col)
{
	m_headcolor = col;
}

void generic_plotter_device::set_headsize(int s)
{
	m_headsize = s;
}

void generic_plotter_device::pen_down()
{
	m_pendown = true;
}

void generic_plotter_device::pen_up()
{
	m_pendown = false;
}

void generic_plotter_device::write_snapshot_to_file()
{
	machine().popmessage("writing printer snapshot");

	emu_file file(machine().options().snapshot_directory(), OPEN_FLAG_WRITE | OPEN_FLAG_CREATE | OPEN_FLAG_CREATE_PATHS);
	std::error_condition const filerr = machine().video().open_next(file, "png");

	if (!filerr)
	{
		util::png_write_bitmap(file, nullptr, m_paper_bitmap, 0, nullptr);
	}
}

void generic_plotter_device::change_paper()
{
	machine().popmessage("changing paper");
	m_paper_bitmap.resize(m_paperwin.width, m_paperwin.height);
	m_paper_bitmap.fill(m_paperwin.color);
	m_paperorigin = m_paperwinpos;
}

void generic_plotter_device::draw_head(bitmap_rgb32 &bitmap)
{
	draw_with_thickness(bitmap, get_head_pos(), m_headsize, m_headcolor);
}

uint32_t generic_plotter_device::screen_update_bitmap(screen_device &screen,bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(m_view.color, cliprect);
	rectangle rect(m_paperwinpos.x, m_paperwinpos.x+m_paperwin.width-1,
		       m_paperwinpos.y, m_paperwinpos.y+m_paperwin.height-1);
	rect &= cliprect;
	copybitmapregion(bitmap, m_paper_bitmap, m_paperwinpos, m_paperwinpos-get_paper_pos(),
			 m_paperwin.width, m_paperwin.height, cliprect);
	draw_head(bitmap);
	if (m_panel_update.isnull())
	{
		return 0;
	}
	else
	{
		return m_panel_update(screen, bitmap, cliprect);
	}
}

void generic_plotter_device::device_add_mconfig(machine_config &config)
{
	STEPPER(config, m_xstepper, (uint8_t) 0xa);
	STEPPER(config, m_ystepper, (uint8_t) 0xa);
	screen_device &screen(SCREEN(config, m_screen, SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(m_view.width, m_view.height);
	screen.set_physical_aspect(m_view.width, m_view.height);
	screen.set_visarea(0, m_view.width - 1, 0, m_view.height - 1);
	screen.set_screen_update(FUNC(generic_plotter_device::screen_update_bitmap));
}

void generic_plotter_device::update_motors(uint8_t xpattern,uint8_t ypattern)
{
	m_xstepper->update(xpattern);
	m_ystepper->update(ypattern);
	extend_paper();
	if (m_pendown)
	{
		draw_with_thickness(m_paper_bitmap, get_head_pos()-get_paper_pos(), m_penthickness, m_pencolor);
	}

}

generic_plotter_device::Position paper_roll_plotter_device::get_head_pos()
{
	int32_t x = m_motorpos.x+m_xstepper->get_absolute_position()*m_xscale;
	int32_t y = m_motorpos.y;
	return Position(x, y);
}

generic_plotter_device::Position paper_roll_plotter_device::get_paper_pos()
{
	int32_t x = m_paperorigin.x;
	int32_t y = m_paperorigin.y+m_ystepper->get_absolute_position()*m_yscale;
	return Position(x, y);
}

void paper_roll_plotter_device:: extend_paper()
{
	if (m_paperwinpos.y+m_paperwin.height > get_paper_pos().y+m_paper_bitmap.height())
	{
		bitmap_rgb32 tmp;
		tmp.allocate(m_paper_bitmap.width(), m_paper_bitmap.height());
		copybitmap(tmp, m_paper_bitmap, 0, 0, 0, 0,
			   rectangle(0, tmp.width(), 0, tmp.height()));
		m_paper_bitmap.resize(m_paper_bitmap.width(), m_paper_bitmap.height()+m_paperwin.height);
		m_paper_bitmap.fill(m_paperwin.color);
		copybitmap(m_paper_bitmap, tmp, 0, 0, 0, 0,
			   rectangle(0, tmp.width(), 0, tmp.height()));
	}
}

void paper_roll_plotter_device::change_paper()
{
	generic_plotter_device::change_paper();
	m_ystepper->set_absolute_position(0);
}

int  generic_plotter_device::get_xmotor_pos()
{
	return  m_xstepper->get_absolute_position();
}

int  generic_plotter_device::get_ymotor_pos()
{
	return m_ystepper->get_absolute_position();
}


paper_roll_plotter_device::paper_roll_plotter_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock):paper_roll_plotter_device(mconfig, PAPER_ROLL_PLOTTER, tag, owner, Element(), Element(), Position(), Position(), 1, 1)
{
}

paper_roll_plotter_device::paper_roll_plotter_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, const Element& view, const Element& paperwin, const Position& paperwinpos, const Position& motorpos, double xscale, double yscale):
	generic_plotter_device(mconfig, type, tag, owner, view, paperwin, paperwinpos, motorpos, xscale, yscale)
{
}


DEFINE_DEVICE_TYPE(GENERIC_PLOTTER, generic_plotter_device, "plotter", "Generic Plotter")
DEFINE_DEVICE_TYPE(PAPER_ROLL_PLOTTER, paper_roll_plotter_device, "paper_roll_plotter", "Paper Roll Plotter")
