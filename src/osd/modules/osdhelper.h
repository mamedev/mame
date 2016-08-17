// license:BSD-3-Clause
// copyright-holders:Olivier Galibert, R. Belmont, Couriersud
//============================================================
//
//  osdwindow.h - SDL window handling
//
//============================================================

#pragma once

#ifndef __OSDHELPER__
#define __OSDHELPER__

class osd_dim
{
public:
	osd_dim(const int &w, const int &h)
	: m_w(w), m_h(h)
	{
	}
	int width() const { return m_w; }
	int height() const { return m_h; }

	bool operator!=(const osd_dim &other) { return (m_w != other.width()) || (m_h != other.height()); }
	bool operator==(const osd_dim &other) { return (m_w == other.width()) && (m_h == other.height()); }
private:
	int m_w;
	int m_h;
};

class osd_rect
{
public:
	osd_rect()
	: m_x(0), m_y(0), m_d(0,0)
	{
	}
	osd_rect(const int x, const int y, const int &w, const int &h)
	: m_x(x), m_y(y), m_d(w,h)
	{
	}
	osd_rect(const int x, const int y, const osd_dim &d)
	: m_x(x), m_y(y), m_d(d)
	{
	}
	int top() const { return m_y; }
	int left() const { return m_x; }
	int width() const { return m_d.width(); }
	int height() const { return m_d.height(); }

	osd_dim dim() const { return m_d; }

	int bottom() const { return m_y + m_d.height(); }
	int right() const { return m_x + m_d.width(); }

	osd_rect move_by(int dx, int dy) const { return osd_rect(m_x + dx, m_y + dy, m_d); }
	osd_rect resize(int w, int h) const { return osd_rect(m_x, m_y, w, h); }

private:
	int m_x;
	int m_y;
	osd_dim m_d;
};

#endif // __OSDHELPER__
