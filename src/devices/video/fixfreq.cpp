// license:BSD-3-Clause
// copyright-holders:Couriersud
/***************************************************************************

    fixfreq.h

    2013-2021 Couriersud

    Fixed frequency monochrome monitor emulation

    The driver is intended for drivers which provide an analog video signal.
    VSYNC and HSYNC levels are used to create the bitmap.

***************************************************************************/

// emu.h must be first to be included
#include "emu.h"

#include "fixfreq.h"

#include "render.h"
#include "ui/uimain.h"

#include <iostream>

// for debug logging
#define VERBOSE 0
#define LOG_OUTPUT_STREAM std::cerr

#include "logmacro.h"

#include <algorithm>

// --------------------------------------------------------------------------
//    Fixed frequency monitor
// --------------------------------------------------------------------------

// device type definition
DEFINE_DEVICE_TYPE(FIXFREQ, fixedfreq_device, "fixfreq",
				   "Fixed-Frequency Monochrome Monitor")

// --------------------------------------------------------------------------
//    Port adjuster support
// --------------------------------------------------------------------------

#define PORT_ADJUSTERX(_id, _name, _min, _max)                                 \
	PORT_START(#_id)                                                           \
	configurer.field_alloc(                                                    \
		IPT_ADJUSTER,                                                          \
		(static_cast<fixedfreq_device &>(owner).monitor_val(_id)), 0xffff,     \
		("Monitor - " _name));                                                 \
	PORT_MINMAX(_min, _max)                                                    \
	PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(fixedfreq_device::port_changed), _id)      \
	PORT_CONDITION("ENABLE", 0x01, EQUALS, 0x01)

#define IOPORT_ID(_id) ioport(#_id)

enum fixedfreq_tag_id_e
{
	HVISIBLE,
	HFRONTPORCH,
	HSYNC,
	HBACKPORCH,
	VVISIBLE,
	VFRONTPORCH,
	VSYNC,
	VBACKPORCH,
	SYNCTHRESHOLD,
	VSYNCTHRESHOLD,
	GAIN,
	SCANLINE_HEIGHT
};

void fixedfreq_monitor_state::update_sync_channel(const time_type &time, double newval)
{
	const time_type delta_time = time - m_last_sync_time;

	const int last_vsync = m_sig_vsync;
	const int last_comp = m_sig_composite;

	m_vsync_filter += ((double)last_comp - m_vsync_filter)
					  * (1.0
						 - exp(-delta_time * m_desc.vsync_filter_timeconst()));
	m_sig_composite = (newval < m_desc.m_sync_threshold) ? 1 : 0;

	m_sig_vsync = (m_vsync_filter > m_desc.m_vsync_threshold) ? 1 : 0;

	if (!last_vsync && m_sig_vsync)
	{
		LOG("VSYNC UP %f %d\n", m_last_x, m_last_y);
		const int has_fields = (m_desc.m_fieldcount > 1) ? 1 : 0;

		// FIXME: add modes: true interlaced, overlayed, false progressive (see
		// popeye video)
		if (has_fields)
		{
			const auto avg_line_dur = (time - m_last_field_time)
									  * m_desc.m_fieldcount / (m_last_y + 1);
			m_last_field_time = time;
			m_sig_field = avg_line_dur * 0.75 > m_last_line_duration;
			LOG("%d %f %f %f\n", m_sig_field, m_last_line_duration,
				avg_line_dur, time);
		}

		// notify the controlling device about the vsync and the field.
		m_intf.vsync_end_cb(time - m_last_vsync_time, m_sig_field);
		m_last_vsync_time = time;

		m_last_y = 0;
	}
	else if (last_vsync && !m_sig_vsync)
	{
		LOG("VSYNC DOWN %f %d\n", m_last_x, m_last_y);
	}

	if (!last_comp && m_sig_composite)
	{
		if (m_sig_vsync)
			LOG("Hsync in vsync\n");
		// LOG("HSYNC up %d\n", m_last_x);
		//  FIXME: pixels > 0 filters some spurious hysnc on line
		//         23/24 in breakout
		//         The hsync signal transition from high to low is 7 pixels too
		//         early, goes up again after 6.8 pix and down after 7.2 pix.
		//         Therefore we need to filter early low to high transitions
		//         and base hsync on the start of the hsync signal.
		if (!m_sig_vsync && (m_last_x > 0))
		{
			m_last_y += m_desc.m_fieldcount;
			m_last_x = 0;
			m_line_time = time + 1.0 / m_desc.hsync_filter_timeconst();

			m_last_line_duration = time - m_last_hsync_time;
			m_last_hsync_time = time;
		}
	}
	else if (last_comp && !m_sig_composite)
	{
		/* falling composite */
		// LOG("HSYNC down %f %d %f\n", time * 1e6, m_last_x, m_sync_signal);
	}
	m_last_sync_val = newval;
	m_last_sync_time = time;
}

void fixedfreq_monitor_state::update_bm(const time_type &time)
{
	const float pixels = (time - m_line_time) * (double)m_desc.monitor_clock();
	const int   has_fields = (m_desc.m_fieldcount > 1) ? 1 : 0;
	const float fhscale(static_cast<float>(m_desc.m_hscale));

	// uint32_t col(0xffff0000); // Mark sync areas
	// if (m_last_sync >= m_desc.m_sync_threshold)
	//   col = m_col;

	if (!m_sig_vsync && !m_sig_composite)
	{
		// uint32_t mask = m_sig_field ? 0xffffffff : 0xffff0000;
		m_fragments.push_back(
			{static_cast<float>(m_last_y + m_sig_field * has_fields),
			 m_last_x * fhscale, pixels * fhscale, m_col}); // & mask});
	}
	// m_intf.plot_hline(m_last_x, m_last_y + m_sig_field * has_fields, pixels,
	// col);
	m_last_x = pixels;
}

void fixedfreq_monitor_state::update_composite_monochrome(const time_type &time, double data)
{
	update_bm(time);
	update_sync_channel(time, data);

	//#int colv = int((data - m_desc.m_sync_threshold) * m_desc.m_gain * 255.0);
	int colv = int((data - 1.5) * m_desc.m_gain * 255.0);
	if (colv > 255)
		colv = 255;
	if (colv < 0)
		// m_col = 0xffff0000;
		m_col = 0x0000000;
	else
		m_col = 0xff000000 | (colv << 16) | (colv << 8) | colv;
}

void fixedfreq_monitor_state::update_red(const time_type &time, double data)
{
	update_bm(time);

	int colv = int((data - m_desc.m_sync_threshold) * m_desc.m_gain * 255.0);
	if (colv > 255)
		colv = 255;
	if (colv < 0)
		colv = 0;
	m_col = (m_col & 0xff00ffff) | (colv << 16);
}

void fixedfreq_monitor_state::update_green(const time_type &time, double data)
{
	update_bm(time);
	// update_sync_channel(ctime, data);

	int colv = int((data - m_desc.m_sync_threshold) * m_desc.m_gain * 255.0);
	if (colv > 255)
		colv = 255;
	if (colv < 0)
		colv = 0;
	m_col = (m_col & 0xffff00ff) | (colv << 8);
}

void fixedfreq_monitor_state::update_blue(const time_type &time, double data)
{
	update_bm(time);
	// update_sync_channel(ctime, data);

	int colv = int((data - m_desc.m_sync_threshold) * m_desc.m_gain * 255.0);
	if (colv > 255)
		colv = 255;
	if (colv < 0)
		colv = 0;
	m_col = (m_col & 0xffffff00) | colv;
}

void fixedfreq_monitor_state::update_sync(const time_type &time, double data)
{
	update_bm(time);
	update_sync_channel(time, data);
}

fixedfreq_device::fixedfreq_device(
		const machine_config &mconfig,
		device_type type,
		const char *tag,
		device_t *owner,
		uint32_t clock)
	: device_t(mconfig, type, tag, owner, clock)
	, device_video_interface(mconfig, *this, false)
	, m_enable(*this, "ENABLE")
	, m_vector(*this, "VECTOR")
	, m_scanline_height(1.0)
	, m_last_rt(0.0)
	, m_monitor()
	, m_state(m_monitor, *this)
{
}

fixedfreq_device::fixedfreq_device(
		const machine_config &mconfig,
		const char *tag,
		device_t *owner,
		uint32_t clock)
	: fixedfreq_device(mconfig, FIXFREQ, tag, owner, clock)
{
}

void fixedfreq_device::device_config_complete()
{
	if (!has_screen())
		return;
	// Video signal processing will be moved into netlist to avoid
	// aborting cpu slices. When this is done, the monitor specifications
	// need to move to the netlist as well.
	//
	// At the time of device_config_complete the monitor specification will
	// not be known - the netlist is parsed during device_start.
	// In this case we have to use some temporary fixed values, e.g.
	// screen().set_raw(7158196, 454, 0, 454, 262, 0, 262);
	// This will be overwritten during the first vblank anyhow.
	//
	// However the width and height determine the width of the mame window.
	// It is therefore recommended to use `set_raw` in the mame driver
	// to specify the window size.
	if (!screen().refresh_attoseconds())
	{
		screen().set_raw(
				m_monitor.m_monitor_clock, m_monitor.htotal(), 0,
				m_monitor.htotal(), m_monitor.vtotal(), 0,
				m_monitor.vtotal());
	}
	if (!screen().has_screen_update())
		screen().set_screen_update(*this, FUNC(fixedfreq_device::screen_update));
	LOG("config complete\n");
}

void fixedfreq_device::device_start()
{
	LOG("start\n");

	m_state.start();

	// FIXME: will be done by netlist going forward
	save_item(NAME(m_state.m_last_sync_val));
	save_item(NAME(m_state.m_last_x));
	save_item(NAME(m_state.m_last_y));
	save_item(NAME(m_state.m_last_sync_time));
	save_item(NAME(m_state.m_line_time));
	save_item(NAME(m_state.m_last_hsync_time));
	save_item(NAME(m_state.m_last_vsync_time));
	save_item(NAME(m_state.m_last_line_duration));
	save_item(NAME(m_state.m_last_field_time));

	/* sync separator */
	save_item(NAME(m_state.m_vsync_filter));
	save_item(NAME(m_state.m_sig_vsync));
	save_item(NAME(m_state.m_sig_composite));
	save_item(NAME(m_state.m_sig_field));

	save_item(NAME(m_last_rt));
}

void fixedfreq_device::device_reset()
{
	m_state.reset();
	LOG("Reset\n");
	// ioport("YYY")->field(0xffff)->live().value = 20;
#if 0
	//IOPORT_ID(HVISIBLE)->field(~0)->set_value(m_monitor.m_hvisible);
	//IOPORT_ID(HVISIBLE)->update_defvalue(false);
	IOPORT_ID(HVISIBLE)->live().defvalue = m_monitor.m_hvisible;
	IOPORT_ID(HFRONTPORCH)->write(m_monitor.m_hsync);
	IOPORT_ID(HSYNC)->write(m_monitor.m_hfrontporch);
	IOPORT_ID(HBACKPORCH)->write(m_monitor.m_hbackporch);
	IOPORT_ID(VVISIBLE)->write(m_monitor.m_vvisible);
	IOPORT_ID(VFRONTPORCH)->write(m_monitor.m_vfrontporch);
	IOPORT_ID(VSYNC)->write(m_monitor.m_vsync);
	IOPORT_ID(VBACKPORCH)->write(m_monitor.m_vbackporch);
	IOPORT_ID(SYNCTHRESHOLD)->write(m_monitor.m_sync_threshold * 1000.0);
	IOPORT_ID(GAIN)->write(m_monitor.m_gain * 1000.0);
#endif
}

void fixedfreq_device::device_post_load()
{
	// recompute_parameters();
	LOG("post load\n");
}

static uint32_t nom_col(uint32_t col)
{
	float const r = ((col >> 16) & 0xff);
	float const g = ((col >>  8) & 0xff);
	float const b = ((col >>  0) & 0xff);

	float const m = std::max(r, std::max(g, b));
	if (m == 0.0f)
		return 0;
	return
		(uint32_t(m)              << 24) |
		(uint32_t(r / m * 255.0f) << 16) |
		(uint32_t(g / m * 255.0f) <<  8) |
		(uint32_t(b / m * 255.0f) <<  0);
}

static void draw_testpat(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	// Test pattern Grey scale
	const int stripes = 255;
	// auto va(screen.visible_area());
	auto &va(cliprect);

	for (int i = 0; i < stripes; i++)
	{
		int l = va.left() + (i * va.width() / stripes);
		int w = (va.left() + (i + 1) * va.width() / stripes) - l;
		int v = (255 * i) / stripes;
		bitmap.plot_box(l, va.top() + 20, w, va.height() / 2 - 20, rgb_t(0xff, v, v, v));
	}

	int l(va.left() + va.width() / 4);
	int w(va.width() / 4);
	int t(va.top() + va.height() / 2);
	int h(va.height() / 2);
	// 50% Test pattern
	for (int i = t; i < t + h; i += 2)
	{
		bitmap.plot_box(l, i, w, i, rgb_t(0xff, 0xff, 0xff, 0xff));
		bitmap.plot_box(l, i + 1, w, i + 1, rgb_t(0xff, 0, 0, 0));
	}
	l += va.width() / 4;
	bitmap.plot_box(l, t, w, h, rgb_t(0xff, 0xc3, 0xc3, 0xc3)); // 195
}

uint32_t fixedfreq_device::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	// printf("%f\n", machine().time().as_double());
	// printf("%d %lu %f %f\n", m_state.m_sig_vsync, m_state.m_fragments.size(),
	// m_state.m_fragments[0].y,
	// m_state.m_fragments[m_state.m_fragments.size()-1].y);
	bool force_vector = screen.screen_type() == SCREEN_TYPE_VECTOR
						|| (m_vector->read() & 1);
	bool  debug_timing = (m_enable->read() & 2) == 2;
	bool  test_pat = (m_enable->read() & 4) == 4;
	rgb_t backcol = debug_timing ? rgb_t(0xff, 0xff, 0x00, 0x00)
								 : rgb_t(0xff, 0x00, 0x00, 0x00);

	if (!force_vector)
	{
		screen.set_video_attributes(0);
		bitmap.fill(backcol);
		for (auto &f : m_state.m_fragments)
			if (f.y < bitmap.height())
				bitmap.plot_box(f.x, f.y, f.xr - f.x, 1, f.col);

		if (test_pat)
			draw_testpat(screen, bitmap, cliprect);
	}
	else
	{
		screen.set_video_attributes(VIDEO_SELF_RENDER);

		const uint32_t flags(
			PRIMFLAG_ANTIALIAS(1) | PRIMFLAG_BLENDMODE(BLENDMODE_ADD)
			| (screen.screen_type() == SCREEN_TYPE_VECTOR ? PRIMFLAG_VECTOR(1)
														  : 0));
		const rectangle &visarea = screen.visible_area();
		float            xscale = 1.0f / (float)visarea.width();
		float            yscale = 1.0f / (float)visarea.height();
		float            xoffs = (float)visarea.min_x;
		float            yoffs = (float)visarea.min_y;
		screen.container().empty();
		screen.container().add_rect(
			0.0f, 0.0f, 1.0f, 1.0f, rgb_t(0xff, 0x00, 0x00, 0x00),
			PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA)
				| (screen.screen_type() == SCREEN_TYPE_VECTOR
					   ? PRIMFLAG_VECTORBUF(1)
					   : 0));

		float last_y = -1e6;
		for (auto &f : m_state.m_fragments)
		{
			const float x0((f.x - xoffs) * xscale);
			const float y0((f.y - yoffs) * yscale);
			const float x1((f.xr - xoffs) * xscale);

			rgb_t col = (debug_timing && f.y < last_y) ? backcol : (rgb_t)f.col;
			// FIXME: Debug check for proper vsync timing
#if 0
				auto w = m_scanline_height * xscale * 0.5;
				screen.container().add_line(
					x0+w, y0, x1-w, y0, m_scanline_height*yscale,
					nom_col(f.col),
//                  (0xff << 24) | (f.col & 0xffffff),
					flags);
#elif 1
			const float y1((f.y + m_scanline_height - yoffs) * yscale);
			screen.container().add_rect(x0, y0, x1, y1, nom_col(col),
										//                  (0xaf << 24) |
										//                  (f.col & 0xffffff),
										flags);
#else
			const float y1((f.y + m_scanline_height - yoffs) * yscale);
			// Crashes with bgfx
			screen.container().add_quad(x0, y0, x1, y1, rgb_t(nom_col(f.col)),
										//                  (0xaf << 24) |
										//                  (f.col & 0xffffff),
										m_texture, flags);
#endif
			last_y = f.y;
		}
	}
	m_state.m_fragments.clear();
	return 0;
}

void fixedfreq_device::vsync_end_cb(double refresh_time, uint32_t field)
{
	const auto expected_frame_period(m_monitor.clock_period()
									 * m_monitor.vtotal() * m_monitor.htotal());
	bool       progressive = (m_enable->read() & 8) == 8;

	double mult = 0.5;

	if (!progressive && (m_monitor.m_fieldcount == 2))
	{
		if (field == 0)
		{
			m_last_rt = refresh_time;
			return;
		}
		else
			mult = 1.0;
	}

	const auto refresh_limited(std::min(
		4.0 * expected_frame_period, std::max((refresh_time + m_last_rt) * mult,
											  0.25 * expected_frame_period)));

	m_last_rt = refresh_time;
	rectangle visarea(m_monitor.minh(), m_monitor.maxh(), m_monitor.minv(),
					  m_monitor.maxv());

	// reset_origin must be called first.
	screen().reset_origin(
			m_state.m_last_y - (m_monitor.vsync_width() + m_monitor.vbackporch_width()),
			0);
	screen().configure(
			m_monitor.htotal_scaled(), m_monitor.vtotal(), visarea,
			DOUBLE_TO_ATTOSECONDS(refresh_limited));
}

NETDEV_ANALOG_CALLBACK_MEMBER(fixedfreq_device::update_composite_monochrome)
{
	// double is good enough for this exercise;

	const time_type ctime = time.as_double();
	m_state.update_composite_monochrome(ctime, data);
}

NETDEV_ANALOG_CALLBACK_MEMBER(fixedfreq_device::update_red)
{
	// double is good enough for this exercise;

	const time_type ctime = time.as_double();
	m_state.update_red(ctime, data);
}

NETDEV_ANALOG_CALLBACK_MEMBER(fixedfreq_device::update_green)
{
	// double is good enough for this exercise;

	const time_type ctime = time.as_double();
	m_state.update_green(ctime, data);
}

NETDEV_ANALOG_CALLBACK_MEMBER(fixedfreq_device::update_blue)
{
	// double is good enough for this exercise;

	const time_type ctime = time.as_double();
	m_state.update_blue(ctime, data);
}

NETDEV_ANALOG_CALLBACK_MEMBER(fixedfreq_device::update_sync)
{
	// double is good enough for this exercise;

	const time_type ctime = time.as_double();
	m_state.update_sync(ctime, data);
}

/***************************************************************************/

// clang-format off

static INPUT_PORTS_START(fixedfreq_base_ports)
	PORT_START("ENABLE")
	PORT_CONFNAME( 0x01, 0x00, "Display Monitor sliders" )
	PORT_CONFSETTING(    0x00, DEF_STR( Off ) )
	PORT_CONFSETTING(    0x01, DEF_STR( On ) )
	PORT_CONFNAME( 0x02, 0x00, "Visual Timing Debug" )
	PORT_CONFSETTING(    0x00, DEF_STR( Off ) )
	PORT_CONFSETTING(    0x02, DEF_STR( On ) )
	PORT_CONFNAME( 0x04, 0x00, "Display gray test pattern" ) PORT_CONDITION("VECTOR", 0x01, EQUALS, 0x00)
	PORT_CONFSETTING(    0x00, DEF_STR( Off ) )
	PORT_CONFSETTING(    0x04, DEF_STR( On ) )

	PORT_CONFNAME( 0x08, 0x00, "Interlace mode" ) PORT_CONDITION("VECTOR", 0x01, EQUALS, 0x00)
	PORT_CONFSETTING(    0x00, "Interlaced" )
	PORT_CONFSETTING(    0x08, "Progressive" )

	PORT_ADJUSTERX(HVISIBLE, "H Visible", 10, 1000)
	PORT_ADJUSTERX(HFRONTPORCH, "H Front porch width", 1, 100)
	PORT_ADJUSTERX(HSYNC, "H Sync width", 1, 100)
	PORT_ADJUSTERX(HBACKPORCH, "H Back porch width", 1, 1000)
	PORT_ADJUSTERX(VVISIBLE, "V Visible", 1, 1000)
	PORT_ADJUSTERX(VFRONTPORCH, "V Front porch width", 0, 100)
	PORT_ADJUSTERX(VSYNC, "V Sync width", 1, 100)
	PORT_ADJUSTERX(VBACKPORCH, "V Back porch width", 1, 100)
	PORT_ADJUSTERX(SYNCTHRESHOLD, "Sync threshold mV", 10, 2000)
	PORT_ADJUSTERX(VSYNCTHRESHOLD, "V Sync threshold mV", 10, 1000)
	PORT_ADJUSTERX(GAIN, "Signal Gain", 10, 1000)
INPUT_PORTS_END


static INPUT_PORTS_START(fixedfreq_raster_ports)
	PORT_START("VECTOR")
	PORT_CONFNAME( 0x01, 0x00, "Use vector rendering" )
	PORT_CONFSETTING(    0x00, DEF_STR( Off ) )
	PORT_CONFSETTING(    0x01, DEF_STR( On ) )

	PORT_INCLUDE(fixedfreq_base_ports)

	PORT_ADJUSTERX(SCANLINE_HEIGHT, "Scanline Height", 10, 300)

INPUT_PORTS_END

static INPUT_PORTS_START(fixedfreq_vector_ports)
	PORT_INCLUDE(fixedfreq_base_ports)

	PORT_ADJUSTERX(SCANLINE_HEIGHT, "Scanline Height", 10, 300)
INPUT_PORTS_END

//
// clang-format on

ioport_constructor fixedfreq_device::device_input_ports() const
{
	LOG("input ports\n");
	if (has_screen())
	{
		if (screen().screen_type() == SCREEN_TYPE_RASTER)
			return INPUT_PORTS_NAME(fixedfreq_raster_ports);
		else
			return INPUT_PORTS_NAME(fixedfreq_vector_ports);
	}
	else
		return nullptr;
}

unsigned fixedfreq_device::monitor_val(unsigned param) const
{
	switch (param)
	{
		case HVISIBLE: return m_monitor.hvisible_width();
		case HFRONTPORCH: return m_monitor.hfrontporch_width();
		case HSYNC: return m_monitor.hsync_width();
		case HBACKPORCH: return m_monitor.hbackporch_width();
		case VVISIBLE: return m_monitor.vvisible_width();
		case VFRONTPORCH: return m_monitor.vfrontporch_width();
		case VSYNC: return m_monitor.vsync_width();
		case VBACKPORCH: return m_monitor.vbackporch_width();
		case SYNCTHRESHOLD: return m_monitor.m_sync_threshold * 1000.0;
		case VSYNCTHRESHOLD: return m_monitor.m_vsync_threshold * 1000.0;
		case GAIN: return m_monitor.m_gain * 100.0;
		case SCANLINE_HEIGHT: return m_scanline_height * 100.0;
	}
	return 0;
}

INPUT_CHANGED_MEMBER(fixedfreq_device::port_changed)
{
	auto &m(m_monitor);

	LOG("%d %d\n", param, newval);
	switch (param)
	{
		case HVISIBLE:
			m.set_h_rel(newval, m.hfrontporch_width(), m.hsync_width(),
						m.hbackporch_width());
			break;
		case HFRONTPORCH:
			m.set_h_rel(m.hvisible_width(), newval, m.hsync_width(),
						m.hbackporch_width());
			break;
		case HSYNC:
			m.set_h_rel(m.hvisible_width(), m.hfrontporch_width(), newval,
						m.hbackporch_width());
			break;
		case HBACKPORCH:
			m.set_h_rel(m.hvisible_width(), m.hfrontporch_width(),
						m.hsync_width(), newval);
			break;
		case VVISIBLE:
			m.set_v_rel(newval, m.vfrontporch_width(), m.vsync_width(),
						m.vbackporch_width());
			break;
		case VFRONTPORCH:
			m.set_v_rel(m.vvisible_width(), newval, m.vsync_width(),
						m.vbackporch_width());
			break;
		case VSYNC:
			m.set_v_rel(m.vvisible_width(), m.vfrontporch_width(), newval,
						m.vbackporch_width());
			break;
		case VBACKPORCH:
			m.set_v_rel(m.vvisible_width(), m.vfrontporch_width(),
						m.vsync_width(), newval);
			break;
		case SYNCTHRESHOLD:
			m.m_sync_threshold = static_cast<double>(newval) / 1000.0;
			break;
		case VSYNCTHRESHOLD:
			m.m_vsync_threshold = static_cast<double>(newval) / 1000.0;
			break;
		case GAIN: m.m_gain = static_cast<double>(newval) / 100.0; break;
		case SCANLINE_HEIGHT:
			m_scanline_height = static_cast<double>(newval) / 100.0;
			break;
	}
	machine().ui().popup_time(5, "Screen Dim %d x %d\n", m.htotal(),
							  m.vtotal());
	// ioport("YYY")->update_defvalue(true);
}
