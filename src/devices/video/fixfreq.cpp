// license:BSD-3-Clause
// copyright-holders:Couriersud
/***************************************************************************

    fixfreq.h

    2013 Couriersud

    Fixed frequency monochrome monitor emulation

    The driver is intended for drivers which provide an analog video signal.
    VSYNC and HSYNC levels are used to create the bitmap.

***************************************************************************/

#include "emu.h"
#include "ui/uimain.h"
#include "rendutil.h"
#include "fixfreq.h"

// for quick and dirty debugging
#define VERBOSE 0
#define LOG_OUTPUT_FUNC printf
#include "logmacro.h"

#include <algorithm>

// --------------------------------------------------------------------------
//    Fixed frequency monitor
// --------------------------------------------------------------------------

// device type definition
DEFINE_DEVICE_TYPE(FIXFREQ, fixedfreq_device, "fixfreq", "Fixed-Frequency Monochrome Monitor")

// --------------------------------------------------------------------------
//    Port adjuster support
// --------------------------------------------------------------------------

#define PORT_ADJUSTERX(_id, _name, _min, _max) \
	PORT_START(# _id) \
	configurer.field_alloc(IPT_ADJUSTER, (static_cast<fixedfreq_device &>(owner).monitor_val(_id)), 0xffff, ("Monitor - " _name)); \
	PORT_MINMAX(_min, _max) PORT_CHANGED_MEMBER(DEVICE_SELF, fixedfreq_device, port_changed, _id) \
	PORT_CONDITION("ENABLE", 0x01, EQUALS, 0x01)

#define IOPORT_ID(_id) ioport(# _id)

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
	SCANLINE_HEIGHT,
	USE_VECTOR
};

void fixedfreq_monitor_state::update_sync_channel(const time_type &time, const double newval)
{
	const time_type delta_time = time - m_last_sync_time;

	const int last_vsync = m_sig_vsync;
	const int last_comp = m_sig_composite;

	m_vsync_filter += ((double) last_comp - m_vsync_filter) * (1.0 - exp(-delta_time * m_desc.vsync_filter_timeconst()));
	m_sig_composite = (newval < m_desc.m_sync_threshold) ? 1 : 0 ;

	m_sig_vsync = (m_vsync_filter > m_desc.m_vsync_threshold) ? 1 : 0;

	if (!last_vsync && m_sig_vsync)
	{
		LOG("VSYNC UP %f %d\n", m_last_x, m_last_y);
		m_intf.vsync_end_cb(time - m_last_vsync_time);
		m_last_vsync_time = time;
		// FIXME: this is wrong: we need to count lines with half-scanline width
		//        to determine the field.
		m_sig_field = last_comp;   /* force false-progressive */
		m_sig_field = (m_sig_field ^ 1) ^ last_comp;   /* if there is no field switch, auto switch */
		m_last_y = 0; //m_desc.vbackporch_width();
	}
	else if (last_vsync && !m_sig_vsync)
	{
		LOG("VSYNC DOWN %f %d\n", m_last_x, m_last_y);
		//LOG("Field: %d\n", m_sig_field);
	}

	if (!last_comp && m_sig_composite)
	{
		if (m_sig_vsync)
			LOG("Hsync in vsync\n");
		/* TODO - time since last hsync and field detection */
		//LOG("HSYNC up %d\n", m_last_x);
		// FIXME: pixels > 50 filters some spurious hysnc on line 23/24 in breakout
		//        The hsync signal transition from high to low is 7 pixels too
		//        early, goes up again after 6.8 pix and down after 7.2 pix.
		//        Therefore we need to filter early low to high transitions
		//        and base hsync on the start of the hsync signal.
		if (!m_sig_vsync && (m_last_x > m_desc.m_hscale * 100))
		{
			m_last_y += m_desc.m_fieldcount;
			m_last_x = 0;
			m_line_time = time + 1.0 / m_desc.hsync_filter_timeconst();
		}
		//if (!m_sig_vsync && (m_last_x < m_desc.m_hscale * 100))
		//	printf("HSYNC up %d %f\n", m_last_y, m_last_x);
	}
	else if (last_comp && !m_sig_composite)
	{
		/* falling composite */
		//LOG("HSYNC down %f %d %f\n", time * 1e6, m_last_x, m_sync_signal);
	}
	m_last_sync = newval;
	m_last_sync_time = time;
}

void fixedfreq_monitor_state::update_bm(const time_type &time)
{
	const float pixels = (time - m_line_time) * (double) m_desc.monitor_clock();
	const int has_fields = (m_desc.m_fieldcount > 1) ? 1: 0;
	float fhscale(static_cast<float>(m_desc.m_hscale));

	//uint32_t col(0xffff0000); // Mark sync areas

	//if (m_last_sync >= m_desc.m_sync_threshold)
	//	col = m_col;

	if (!m_sig_vsync && !m_sig_composite)
	{
		m_fragments.push_back({static_cast<float>(m_last_y + m_sig_field * has_fields),
			m_last_x * fhscale, pixels * fhscale, m_col});
	}
	//m_intf.plot_hline(m_last_x, m_last_y + m_sig_field * has_fields, pixels, col);
	m_last_x = pixels;
}

void fixedfreq_monitor_state::update_composite_monochrome(const time_type &time, const double data)
{
	update_bm(time);
	update_sync_channel(time, data);

	//int colv = (int) ((data - m_desc.m_sync_threshold) * m_desc.m_gain * 255.0);
	int colv = (int) ((data - 1.5) * m_desc.m_gain * 255.0);
	if (colv > 255)
		colv = 255;
	if (colv < 0)
		//m_col = 0xffff0000;
		m_col = 0x0000000;
	else
		m_col = 0xff000000 | (colv<<16) | (colv<<8) | colv;
}

void fixedfreq_monitor_state::update_red(const time_type &time, const double data)
{
	update_bm(time);

	int colv = (int) ((data - m_desc.m_sync_threshold) * m_desc.m_gain * 255.0);
	if (colv > 255)
		colv = 255;
	if (colv < 0)
		colv = 0;
	m_col = (m_col & 0xff00ffff) | (colv<<16);
}

void fixedfreq_monitor_state::update_green(const time_type &time, const double data)
{
	update_bm(time);
	//update_sync_channel(ctime, data);

	int colv = (int) ((data - m_desc.m_sync_threshold) * m_desc.m_gain * 255.0);
	if (colv > 255)
		colv = 255;
	if (colv < 0)
		colv = 0;
	m_col = (m_col & 0xffff00ff) | (colv<<8);
}

void fixedfreq_monitor_state::update_blue(const time_type &time, const double data)
{
	update_bm(time);
	//update_sync_channel(ctime, data);

	int colv = (int) ((data - m_desc.m_sync_threshold) * m_desc.m_gain * 255.0);
	if (colv > 255)
		colv = 255;
	if (colv < 0)
		colv = 0;
	m_col = (m_col & 0xffffff00) | colv;

}

void fixedfreq_monitor_state::update_sync(const time_type &time, const double data)
{
	update_bm(time);
	update_sync_channel(time, data);
}

fixedfreq_device::fixedfreq_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, type, tag, owner, clock),
		device_video_interface(mconfig, *this, false),
		m_force_vector(false),
		m_scanline_height(1.0),
		m_texture(nullptr),
		m_monitor(),
		m_state(m_monitor, *this)
{
}

fixedfreq_device::fixedfreq_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: fixedfreq_device(mconfig, FIXFREQ, tag, owner, clock)
{
}

void fixedfreq_device::device_config_complete()
{
	if (!has_screen())
		return;
	if (!screen().refresh_attoseconds())
		screen().set_raw(m_monitor.m_monitor_clock, m_monitor.htotal(), 0,
			m_monitor.htotal(), m_monitor.vtotal(), 0,
			m_monitor.vtotal());
	if (!screen().has_screen_update())
		screen().set_screen_update(*this, FUNC(fixedfreq_device::screen_update));
	LOG("config complete\n");
}

void fixedfreq_device::device_start()
{
	m_state.start();
	if (m_texture == nullptr)
		create_texture();

	// FIXME: will be done by netlist going forward
	save_item(NAME(m_state.m_last_sync));
	save_item(NAME(m_state.m_last_x));
	save_item(NAME(m_state.m_last_y));
	save_item(NAME(m_state.m_last_sync_time));
	save_item(NAME(m_state.m_line_time));
	save_item(NAME(m_state.m_last_hsync_time));
	save_item(NAME(m_state.m_last_vsync_time));

	/* sync separator */
	save_item(NAME(m_state.m_vsync_filter));

	save_item(NAME(m_state.m_sig_vsync));
	save_item(NAME(m_state.m_sig_composite));
	save_item(NAME(m_state.m_sig_field));
	LOG("start\n");
}

void fixedfreq_device::device_reset()
{
	m_state.reset();
	LOG("Reset\n");
	//ioport("YYY")->field(0xffff)->live().value = 20;
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
	//recompute_parameters();
	LOG("post load\n");
}

void fixedfreq_device::create_texture()
{
	int x, y;

	//if (m_texture == nullptr)
	{
		m_bitmap = std::make_unique<bitmap_argb32>();
		m_texture = machine().render().texture_alloc(); // render_texture::hq_scale
	}

#if 0
	emu_file crossfile(m_machine.options().crosshair_path(), OPEN_FLAG_READ);
	if (!m_name.empty())
	{
		// look for user specified file
		std::string filename = m_name + ".png";
		render_load_png(*m_bitmap, crossfile, nullptr, filename.c_str());
	}
	else
	{
		// look for default cross?.png in crsshair/game dir
		std::string filename = string_format("cross%d.png", m_player + 1);
		render_load_png(*m_bitmap, crossfile, m_machine.system().name, filename.c_str());

		// look for default cross?.png in crsshair dir
		if (!m_bitmap->valid())
			render_load_png(*m_bitmap, crossfile, nullptr, filename.c_str());
	}
#endif


	/* if that didn't work, use the built-in one */
	if (!m_bitmap->valid())
	{
		const int WX = 1;
		const int WY = 16;
		/* allocate a blank bitmap to start with */
		m_bitmap->allocate(WX, WY);
		m_bitmap->fill(rgb_t(0x00,0xff,0xff,0xff));

		/* extract the raw source data to it */
		for (y = 0; y < WY / 2; y++)
		{
			/* assume it is mirrored vertically */
			u32 *dest0 = &m_bitmap->pix32(y);
			u32 *dest1 = &m_bitmap->pix32(WY - 1 - y);

			/* extract to two rows simultaneously */
			for (x = 0; x < WX; x++)
			{
				uint8_t t = 255 / (WY/2 - y);
				dest0[x] = dest1[x] = rgb_t(0xff,t,t,t);
				//dest0[x] = dest1[x] = rgb_t(t, 0xff,0xff,0xff);
				//dest0[x] = dest1[x] = rgb_t(t,t,t,t);
			}
		}
	}

	/* reference the new bitmap */
	m_texture->set_bitmap(*m_bitmap, m_bitmap->cliprect(), TEXFORMAT_ARGB32);
}

static uint32_t nom_col(uint32_t col)
{
	float r = ((col >> 16) & 0xff);
	float g = ((col >>  8) & 0xff);
	float b = ((col >>  0) & 0xff);

	float m = std::max(r, std::max(g,b));
	if (m == 0.0f)
		return 0;
	return (((uint32_t) m ) << 24) | (((uint32_t) (r/m*255.0f) ) << 16)
		| (((uint32_t) (g/m*255.0f) ) << 8) | (((uint32_t) (b/m*255.0f) ) <<  0);
}

uint32_t fixedfreq_device::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	//printf("%f %f\n", m_state.m_fragments[0].y, m_state.m_fragments[m_state.m_fragments.size()-1].y);
	if (!m_force_vector && screen.screen_type() == SCREEN_TYPE_RASTER)
	{
		bitmap.fill(rgb_t(0xff, 0xff, 0x00, 0x00));
		for (auto &f : m_state.m_fragments)
			if (f.y < bitmap.height())
				bitmap.plot_box(f.x, f.y, f.xr - f.x, 1, f.col);
	}
	else if (m_force_vector || screen.screen_type() == SCREEN_TYPE_VECTOR)
	{
		if (m_texture == nullptr)
			create_texture();

		const uint32_t flags(PRIMFLAG_ANTIALIAS(1)
			| PRIMFLAG_BLENDMODE(BLENDMODE_ADD)
			| (screen.screen_type() == SCREEN_TYPE_VECTOR ? PRIMFLAG_VECTOR(1) : 0));
		const rectangle &visarea = screen.visible_area();
		float xscale = 1.0f / (float)visarea.width();
		float yscale = 1.0f / (float)visarea.height();
		float xoffs = (float)visarea.min_x;
		float yoffs = (float)visarea.min_y;
		screen.container().empty();
		screen.container().add_rect(0.0f, 0.0f, 1.0f, 1.0f, rgb_t(0xff,0x00,0x00,0x00),
			PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA)
			| (screen.screen_type() == SCREEN_TYPE_VECTOR ? PRIMFLAG_VECTORBUF(1) : 0));

		float last_y = -1e6;
		for (auto &f : m_state.m_fragments)
		{

			// FIXME: Debug check for proper vsync timing
			if (f.y >= last_y)
			{
				const float x0((f.x - xoffs) * xscale);
				const float y0((f.y - yoffs) * yscale);
				const float x1((f.xr - xoffs) * xscale);
#if 0
				auto w = m_scanline_height * xscale * 0.5;
				screen.container().add_line(
					x0+w, y0, x1-w, y0, m_scanline_height*yscale,
					nom_col(f.col),
//					(0xff << 24) | (f.col & 0xffffff),
					flags);
#elif 1
				const float y1((f.y + m_scanline_height - yoffs) * yscale);
				screen.container().add_rect(
					x0, y0, x1, y1,
					nom_col(f.col),
//					(0xaf << 24) | (f.col & 0xffffff),
					flags);
#else
				const float y1((f.y + m_scanline_height - yoffs) * yscale);
				// Crashes with bgfx
				screen.container().add_quad(
					x0, y0, x1, y1,
					rgb_t(nom_col(f.col)),
//					(0xaf << 24) | (f.col & 0xffffff),
					m_texture,
					flags);
#endif
				last_y = f.y;
			}
		}
	}
	m_state.m_fragments.clear();
	return 0;
}

void fixedfreq_device::vsync_end_cb(double refresh_time)
{
	const auto expected_frame_period(m_monitor.clock_period() * m_monitor.vtotal() * m_monitor.htotal());

	const auto refresh_limited(std::min(4.0 * expected_frame_period,
			std::max(refresh_time, 0.25 * expected_frame_period)));

	rectangle visarea(m_monitor.minh(), m_monitor.maxh(), m_monitor.minv(), m_monitor.maxv());

	screen().configure(m_monitor.htotal_scaled(), m_monitor.vtotal(), visarea, DOUBLE_TO_ATTOSECONDS(refresh_limited));
	screen().reset_origin(m_state.m_last_y-(m_monitor.vsync_width() + m_monitor.vbackporch_width()), 0);
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

static INPUT_PORTS_START(fixedfreq_base_ports)
	PORT_START("ENABLE")
	PORT_CONFNAME( 0x01, 0x00, "Display Monitor sliders" )
	PORT_CONFSETTING(    0x00, DEF_STR( Off ) )
	PORT_CONFSETTING(    0x01, DEF_STR( On ) )

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
	PORT_ADJUSTERX(SCANLINE_HEIGHT, "Scanline Height", 10, 300)
INPUT_PORTS_END


static INPUT_PORTS_START(fixedfreq_raster_ports)
	PORT_START("ZZZ")
	PORT_CONFNAME( 0x01, 0x00, "Use vector rendering" ) PORT_CHANGED_MEMBER(DEVICE_SELF, fixedfreq_device, port_changed, USE_VECTOR)
	PORT_CONFSETTING(    0x00, DEF_STR( Off ) )
	PORT_CONFSETTING(    0x01, DEF_STR( On ) )

	PORT_INCLUDE(fixedfreq_base_ports)

	PORT_START("YYY")
	PORT_ADJUSTER(50, "TEST")
INPUT_PORTS_END

static INPUT_PORTS_START(fixedfreq_vector_ports)
	PORT_INCLUDE(fixedfreq_base_ports)
INPUT_PORTS_END

ioport_constructor fixedfreq_device::device_input_ports() const
{
	LOG("input ports\n");
	if (screen().screen_type() == SCREEN_TYPE_RASTER)
		return INPUT_PORTS_NAME(fixedfreq_raster_ports);
	else
		return INPUT_PORTS_NAME(fixedfreq_vector_ports);
}

unsigned fixedfreq_device::monitor_val(unsigned param) const
{
	switch (param)
	{
		case HVISIBLE:
			return m_monitor.hvisible_width();
		case HFRONTPORCH:
			return m_monitor.hfrontporch_width();
		case HSYNC:
			return m_monitor.hsync_width();
		case HBACKPORCH:
			return m_monitor.hbackporch_width();
		case VVISIBLE:
			return m_monitor.vvisible_width();
		case VFRONTPORCH:
			return m_monitor.vfrontporch_width();
		case VSYNC:
			return m_monitor.vsync_width();
		case VBACKPORCH:
			return m_monitor.vbackporch_width();
		case SYNCTHRESHOLD:
			return m_monitor.m_sync_threshold * 1000.0;
		case VSYNCTHRESHOLD:
			return m_monitor.m_vsync_threshold * 1000.0;
		case GAIN:
			return m_monitor.m_gain * 100.0;
		case SCANLINE_HEIGHT:
			return m_scanline_height * 100.0;
		case USE_VECTOR:
			return 0;
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
			m.set_h_rel(newval, m.hfrontporch_width(), m.hsync_width(), m.hbackporch_width());
			break;
		case HFRONTPORCH:
			m.set_h_rel(m.hvisible_width(), newval, m.hsync_width(), m.hbackporch_width());
			break;
		case HSYNC:
			m.set_h_rel(m.hvisible_width(), m.hfrontporch_width(), newval, m.hbackporch_width());
			break;
		case HBACKPORCH:
			m.set_h_rel(m.hvisible_width(), m.hfrontporch_width(), m.hsync_width(), newval);
			break;
		case VVISIBLE:
			m.set_v_rel(newval, m.vfrontporch_width(), m.vsync_width(), m.vbackporch_width());
			break;
		case VFRONTPORCH:
			m.set_v_rel(m.vvisible_width(), newval, m.vsync_width(), m.vbackporch_width());
			break;
		case VSYNC:
			m.set_v_rel(m.vvisible_width(), m.vfrontporch_width(), newval, m.vbackporch_width());
			break;
		case VBACKPORCH:
			m.set_v_rel(m.vvisible_width(), m.vfrontporch_width(), m.vsync_width(), newval);
			break;
		case SYNCTHRESHOLD:
			m.m_sync_threshold = static_cast<double>(newval) / 1000.0;
			break;
		case VSYNCTHRESHOLD:
			m.m_vsync_threshold = static_cast<double>(newval) / 1000.0;
			break;
		case GAIN:
			m.m_gain = static_cast<double>(newval) / 100.0;
			break;
		case SCANLINE_HEIGHT:
			m_scanline_height = static_cast<double>(newval) / 100.0;
			break;
		case USE_VECTOR:
			if (screen().screen_type() == SCREEN_TYPE_RASTER)
			{
				m_force_vector = newval;
				screen().set_video_attributes(newval ? VIDEO_SELF_RENDER : 0);
			}
			break;
	}
	machine().ui().popup_time(5, "Screen Dim %d x %d\n", m.htotal(), m.vtotal());
	//ioport("YYY")->update_defvalue(true);
}
