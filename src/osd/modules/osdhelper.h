// license:BSD-3-Clause
// copyright-holders:Olivier Galibert, R. Belmont, Couriersud
//============================================================
//
//  osdwindow.h - SDL window handling
//
//============================================================

#ifndef MAME_OSD_MODULES_OSDHELPER_H
#define MAME_OSD_MODULES_OSDHELPER_H

#pragma once

class osd_dim
{
public:
	constexpr osd_dim() : m_w(0), m_h(0) { }
	constexpr osd_dim(int w, int h) : m_w(w), m_h(h) { }

	constexpr int width() const { return m_w; }
	constexpr int height() const { return m_h; }

	constexpr bool operator!=(const osd_dim &other) { return (m_w != other.width()) || (m_h != other.height()); }
	constexpr bool operator==(const osd_dim &other) { return (m_w == other.width()) && (m_h == other.height()); }

private:
	int m_w;
	int m_h;
};

class osd_rect
{
public:
	constexpr osd_rect() : m_x(0), m_y(0), m_d(0, 0) { }
	constexpr osd_rect(int x, int y, int w, int h) : m_x(x), m_y(y), m_d(w, h) { }
	constexpr osd_rect(int x, int y, const osd_dim &d) : m_x(x), m_y(y), m_d(d) { }

	constexpr int left() const { return m_x; }
	constexpr int top() const { return m_y; }
	constexpr int width() const { return m_d.width(); }
	constexpr int height() const { return m_d.height(); }

	constexpr osd_dim dim() const { return m_d; }

	constexpr int right() const { return m_x + m_d.width(); }
	constexpr int bottom() const { return m_y + m_d.height(); }

	constexpr osd_rect move_by(int dx, int dy) const { return osd_rect(m_x + dx, m_y + dy, m_d); }
	constexpr osd_rect resize(int w, int h) const { return osd_rect(m_x, m_y, w, h); }

	constexpr bool operator!=(const osd_rect &other) { return (m_x != other.left()) || (m_y != other.top()) || (m_d != other.dim()); }
	constexpr bool operator==(const osd_rect &other) { return (m_x == other.left()) && (m_y == other.top()) && (m_d == other.dim()); }

private:
	int m_x;
	int m_y;
	osd_dim m_d;
};

#endif // MAME_OSD_MODULES_OSDHELPER_H
