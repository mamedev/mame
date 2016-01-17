// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    screen.c

    Core MAME screen device.

***************************************************************************/

#include "emu.h"
#include "emuopts.h"
#include "png.h"
#include "rendutil.h"



//**************************************************************************
//  DEBUGGING
//**************************************************************************

#define VERBOSE                     (0)
#define LOG_PARTIAL_UPDATES(x)      do { if (VERBOSE) logerror x; } while (0)



//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

// device type definition
const device_type SCREEN = &device_creator<screen_device>;

const attotime screen_device::DEFAULT_FRAME_PERIOD(attotime::from_hz(DEFAULT_FRAME_RATE));

UINT32 screen_device::m_id_counter = 0;

//**************************************************************************
//  SCREEN DEVICE
//**************************************************************************

//-------------------------------------------------
//  screen_device - constructor
//-------------------------------------------------

screen_device::screen_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, SCREEN, "Video Screen", tag, owner, clock, "screen", __FILE__),
		m_type(SCREEN_TYPE_RASTER),
		m_oldstyle_vblank_supplied(false),
		m_refresh(0),
		m_vblank(0),
		m_xoffset(0.0f),
		m_yoffset(0.0f),
		m_xscale(1.0f),
		m_yscale(1.0f),
		m_palette(*this),
		m_video_attributes(0),
		m_container(nullptr),
		m_width(100),
		m_height(100),
		m_visarea(0, 99, 0, 99),
		m_texformat(),
		m_curbitmap(0),
		m_curtexture(0),
		m_changed(true),
		m_last_partial_scan(0),
		m_partial_scan_hpos(0),
		m_color(rgb_t(0xff, 0xff, 0xff, 0xff)),
		m_brightness(0xff),
		m_frame_period(DEFAULT_FRAME_PERIOD.as_attoseconds()),
		m_scantime(1),
		m_pixeltime(1),
		m_vblank_period(0),
		m_vblank_start_time(attotime::zero),
		m_vblank_end_time(attotime::zero),
		m_vblank_begin_timer(nullptr),
		m_vblank_end_timer(nullptr),
		m_scanline0_timer(nullptr),
		m_scanline_timer(nullptr),
		m_frame_number(0),
		m_partial_updates_this_frame(0)
{
	m_unique_id = m_id_counter;
	m_id_counter++;
	memset(m_texture, 0, sizeof(m_texture));
}


//-------------------------------------------------
//  ~screen_device - destructor
//-------------------------------------------------

screen_device::~screen_device()
{
}


//-------------------------------------------------
//  static_set_type - configuration helper
//  to set the screen type
//-------------------------------------------------

void screen_device::static_set_type(device_t &device, screen_type_enum type)
{
	downcast<screen_device &>(device).m_type = type;
}


//-------------------------------------------------
//  static_set_raw - configuration helper
//  to set the raw screen parameters
//-------------------------------------------------

void screen_device::static_set_raw(device_t &device, UINT32 pixclock, UINT16 htotal, UINT16 hbend, UINT16 hbstart, UINT16 vtotal, UINT16 vbend, UINT16 vbstart)
{
	screen_device &screen = downcast<screen_device &>(device);
	screen.m_clock = pixclock;
	screen.m_refresh = HZ_TO_ATTOSECONDS(pixclock) * htotal * vtotal;
	screen.m_vblank = screen.m_refresh / vtotal * (vtotal - (vbstart - vbend));
	screen.m_width = htotal;
	screen.m_height = vtotal;
	screen.m_visarea.set(hbend, hbstart - 1, vbend, vbstart - 1);
}


//-------------------------------------------------
//  static_set_refresh - configuration helper
//  to set the refresh rate
//-------------------------------------------------

void screen_device::static_set_refresh(device_t &device, attoseconds_t rate)
{
	downcast<screen_device &>(device).m_refresh = rate;
}


//-------------------------------------------------
//  static_set_vblank_time - configuration helper
//  to set the VBLANK duration
//-------------------------------------------------

void screen_device::static_set_vblank_time(device_t &device, attoseconds_t time)
{
	screen_device &screen = downcast<screen_device &>(device);
	screen.m_vblank = time;
	screen.m_oldstyle_vblank_supplied = true;
}


//-------------------------------------------------
//  static_set_size - configuration helper to set
//  the width/height of the screen
//-------------------------------------------------

void screen_device::static_set_size(device_t &device, UINT16 width, UINT16 height)
{
	screen_device &screen = downcast<screen_device &>(device);
	screen.m_width = width;
	screen.m_height = height;
}


//-------------------------------------------------
//  static_set_visarea - configuration helper to
//  set the visible area of the screen
//-------------------------------------------------

void screen_device::static_set_visarea(device_t &device, INT16 minx, INT16 maxx, INT16 miny, INT16 maxy)
{
	downcast<screen_device &>(device).m_visarea.set(minx, maxx, miny, maxy);
}


//-------------------------------------------------
//  static_set_default_position - configuration
//  helper to set the default position and scale
//  factors for the screen
//-------------------------------------------------

void screen_device::static_set_default_position(device_t &device, double xscale, double xoffs, double yscale, double yoffs)
{
	screen_device &screen = downcast<screen_device &>(device);
	screen.m_xscale = xscale;
	screen.m_xoffset = xoffs;
	screen.m_yscale = yscale;
	screen.m_yoffset = yoffs;
}


//-------------------------------------------------
//  static_set_screen_update - set the legacy(?)
//  screen update callback in the device
//  configuration
//-------------------------------------------------

void screen_device::static_set_screen_update(device_t &device, screen_update_ind16_delegate callback)
{
	screen_device &screen = downcast<screen_device &>(device);
	screen.m_screen_update_ind16 = callback;
	screen.m_screen_update_rgb32 = screen_update_rgb32_delegate();
}

void screen_device::static_set_screen_update(device_t &device, screen_update_rgb32_delegate callback)
{
	screen_device &screen = downcast<screen_device &>(device);
	screen.m_screen_update_ind16 = screen_update_ind16_delegate();
	screen.m_screen_update_rgb32 = callback;
}


//-------------------------------------------------
//  static_set_screen_vblank - set the screen
//  VBLANK callback in the device configuration
//-------------------------------------------------

void screen_device::static_set_screen_vblank(device_t &device, screen_vblank_delegate callback)
{
	downcast<screen_device &>(device).m_screen_vblank = callback;
}


//-------------------------------------------------
//  static_set_palette - set the screen palette
//  configuration
//-------------------------------------------------

void screen_device::static_set_palette(device_t &device, std::string tag)
{
	downcast<screen_device &>(device).m_palette.set_tag(tag);
}


//-------------------------------------------------
//  static_set_video_attributes - set the screen
//  video attributes
//-------------------------------------------------

void screen_device::static_set_video_attributes(device_t &device, UINT32 flags)
{
	screen_device &screen = downcast<screen_device &>(device);
	screen.m_video_attributes = flags;
}
//-------------------------------------------------
//  device_validity_check - verify device
//  configuration
//-------------------------------------------------

void screen_device::device_validity_check(validity_checker &valid) const
{
	// sanity check dimensions
	if (m_width <= 0 || m_height <= 0)
		osd_printf_error("Invalid display dimensions\n");

	// sanity check display area
	if (m_type != SCREEN_TYPE_VECTOR)
	{
		if (m_visarea.empty() || m_visarea.max_x >= m_width || m_visarea.max_y >= m_height)
			osd_printf_error("Invalid display area\n");

		// sanity check screen formats
		if (m_screen_update_ind16.isnull() && m_screen_update_rgb32.isnull())
			osd_printf_error("Missing SCREEN_UPDATE function\n");
	}

	// check for zero frame rate
	if (m_refresh == 0)
		osd_printf_error("Invalid (zero) refresh rate\n");

	texture_format texformat = !m_screen_update_ind16.isnull() ? TEXFORMAT_PALETTE16 : TEXFORMAT_RGB32;
	if (m_palette == nullptr && texformat == TEXFORMAT_PALETTE16)
		osd_printf_error("Screen does not have palette defined\n");
	if (m_palette != nullptr && texformat == TEXFORMAT_RGB32)
		osd_printf_warning("Screen does not need palette defined\n");
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void screen_device::device_start()
{
	// bind our handlers
	m_screen_update_ind16.bind_relative_to(*owner());
	m_screen_update_rgb32.bind_relative_to(*owner());
	m_screen_vblank.bind_relative_to(*owner());

	// if we have a palette and it's not started, wait for it
	if (m_palette != nullptr && !m_palette->started())
		throw device_missing_dependencies();

	// configure bitmap formats and allocate screen bitmaps
	texture_format texformat = !m_screen_update_ind16.isnull() ? TEXFORMAT_PALETTE16 : TEXFORMAT_RGB32;

	for (auto & elem : m_bitmap)
	{
		elem.set_format(format(), texformat);
		register_screen_bitmap(elem);
	}
	register_screen_bitmap(m_priority);

	// allocate raw textures
	m_texture[0] = machine().render().texture_alloc();
	m_texture[0]->set_osd_data((UINT64)((m_unique_id << 1) | 0));
	m_texture[1] = machine().render().texture_alloc();
	m_texture[1]->set_osd_data((UINT64)((m_unique_id << 1) | 1));

	// configure the default cliparea
	render_container::user_settings settings;
	m_container->get_user_settings(settings);
	settings.m_xoffset = m_xoffset;
	settings.m_yoffset = m_yoffset;
	settings.m_xscale = m_xscale;
	settings.m_yscale = m_yscale;
	m_container->set_user_settings(settings);

	// allocate the VBLANK timers
	m_vblank_begin_timer = timer_alloc(TID_VBLANK_START);
	m_vblank_end_timer = timer_alloc(TID_VBLANK_END);

	// allocate a timer to reset partial updates
	m_scanline0_timer = timer_alloc(TID_SCANLINE0);

	// allocate a timer to generate per-scanline updates
	if ((m_video_attributes & VIDEO_UPDATE_SCANLINE) != 0)
		m_scanline_timer = timer_alloc(TID_SCANLINE);

	// configure the screen with the default parameters
	configure(m_width, m_height, m_visarea, m_refresh);

	// reset VBLANK timing
	m_vblank_start_time = attotime::zero;
	m_vblank_end_time = attotime(0, m_vblank_period);

	// start the timer to generate per-scanline updates
	if ((m_video_attributes & VIDEO_UPDATE_SCANLINE) != 0)
		m_scanline_timer->adjust(time_until_pos(0));

	// create burn-in bitmap
	if (machine().options().burnin())
	{
		int width, height;
		if (sscanf(machine().options().snap_size(), "%dx%d", &width, &height) != 2 || width == 0 || height == 0)
			width = height = 300;
		m_burnin.allocate(width, height);
		m_burnin.fill(0);
	}

	// load the effect overlay
	const char *overname = machine().options().effect();
	if (overname != nullptr && strcmp(overname, "none") != 0)
		load_effect_overlay(overname);

	// register items for saving
	save_item(NAME(m_width));
	save_item(NAME(m_height));
	save_item(NAME(m_visarea.min_x));
	save_item(NAME(m_visarea.min_y));
	save_item(NAME(m_visarea.max_x));
	save_item(NAME(m_visarea.max_y));
	save_item(NAME(m_last_partial_scan));
	save_item(NAME(m_frame_period));
	save_item(NAME(m_brightness));
	save_item(NAME(m_scantime));
	save_item(NAME(m_pixeltime));
	save_item(NAME(m_vblank_period));
	save_item(NAME(m_vblank_start_time));
	save_item(NAME(m_vblank_end_time));
	save_item(NAME(m_frame_number));
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void screen_device::device_reset()
{
	// reset brightness to default
	m_brightness = 0xff;
}


//-------------------------------------------------
//  device_stop - clean up before the machine goes
//  away
//-------------------------------------------------

void screen_device::device_stop()
{
	machine().render().texture_free(m_texture[0]);
	machine().render().texture_free(m_texture[1]);
	if (m_burnin.valid())
		finalize_burnin();
}


//-------------------------------------------------
//  device_post_load - device-specific update
//  after a save state is loaded
//-------------------------------------------------

void screen_device::device_post_load()
{
	realloc_screen_bitmaps();
}


//-------------------------------------------------
//  device_timer - called whenever a device timer
//  fires
//-------------------------------------------------

void screen_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	switch (id)
	{
		// signal VBLANK start
		case TID_VBLANK_START:
			vblank_begin();
			break;

		// signal VBLANK end
		case TID_VBLANK_END:
			vblank_end();
			break;

		// first scanline
		case TID_SCANLINE0:
			reset_partial_updates();
			break;

		// subsequent scanlines when scanline updates are enabled
		case TID_SCANLINE:

			// force a partial update to the current scanline
			update_partial(param);

			// compute the next visible scanline
			param++;
			if (param > m_visarea.max_y)
				param = m_visarea.min_y;
			m_scanline_timer->adjust(time_until_pos(param), param);
			break;
	}
}


//-------------------------------------------------
//  configure - configure screen parameters
//-------------------------------------------------

void screen_device::configure(int width, int height, const rectangle &visarea, attoseconds_t frame_period)
{
	// validate arguments
	assert(width > 0);
	assert(height > 0);
	assert(visarea.min_x >= 0);
	assert(visarea.min_y >= 0);
//  assert(visarea.max_x < width);
//  assert(visarea.max_y < height);
	assert(m_type == SCREEN_TYPE_VECTOR || visarea.min_x < width);
	assert(m_type == SCREEN_TYPE_VECTOR || visarea.min_y < height);
	assert(frame_period > 0);

	// fill in the new parameters
	m_width = width;
	m_height = height;
	m_visarea = visarea;

	// reallocate bitmap if necessary
	realloc_screen_bitmaps();

	// compute timing parameters
	m_frame_period = frame_period;
	m_scantime = frame_period / height;
	m_pixeltime = frame_period / (height * width);

	// if an old style VBLANK_TIME was specified in the MACHINE_CONFIG,
	// use it; otherwise calculate the VBLANK period from the visible area
	if (m_oldstyle_vblank_supplied)
	{
		m_vblank_period = m_vblank;
		logerror("%s: Deprecated legacy Old Style screen configured (MCFG_SCREEN_VBLANK_TIME), please use MCFG_SCREEN_RAW_PARAMS instead.\n",this->tag().c_str());
	}
	else
		m_vblank_period = m_scantime * (height - visarea.height());

	// we are now fully configured with the new parameters
	// and can safely call time_until_pos(), etc.

	// if the frame period was reduced so that we are now past the end of the frame,
	// call the VBLANK start timer now; otherwise, adjust it for the future
	attoseconds_t delta = (machine().time() - m_vblank_start_time).as_attoseconds();
	if (delta >= m_frame_period)
		vblank_begin();
	else
		m_vblank_begin_timer->adjust(time_until_vblank_start());

	// if we are on scanline 0 already, call the scanline 0 timer
	// by hand now; otherwise, adjust it for the future
	if (vpos() == 0)
		reset_partial_updates();
	else
		m_scanline0_timer->adjust(time_until_pos(0));

	// adjust speed if necessary
	machine().video().update_refresh_speed();
}


//-------------------------------------------------
//  reset_origin - reset the timing such that the
//  given (x,y) occurs at the current time
//-------------------------------------------------

void screen_device::reset_origin(int beamy, int beamx)
{
	// compute the effective VBLANK start/end times
	attotime curtime = machine().time();
	m_vblank_end_time = curtime - attotime(0, beamy * m_scantime + beamx * m_pixeltime);
	m_vblank_start_time = m_vblank_end_time - attotime(0, m_vblank_period);

	// if we are resetting relative to (0,0) == VBLANK end, call the
	// scanline 0 timer by hand now; otherwise, adjust it for the future
	if (beamy == 0 && beamx == 0)
		reset_partial_updates();
	else
		m_scanline0_timer->adjust(time_until_pos(0));

	// if we are resetting relative to (visarea.max_y + 1, 0) == VBLANK start,
	// call the VBLANK start timer now; otherwise, adjust it for the future
	if (beamy == ((m_visarea.max_y + 1) % m_height) && beamx == 0)
		vblank_begin();
	else
		m_vblank_begin_timer->adjust(time_until_vblank_start());
}


//-------------------------------------------------
//  realloc_screen_bitmaps - reallocate bitmaps
//  and textures as necessary
//-------------------------------------------------

void screen_device::realloc_screen_bitmaps()
{
	// doesn't apply for vector games
	if (m_type == SCREEN_TYPE_VECTOR)
		return;

	// determine effective size to allocate
	INT32 effwidth = MAX(m_width, m_visarea.max_x + 1);
	INT32 effheight = MAX(m_height, m_visarea.max_y + 1);

	// reize all registered screen bitmaps
	for (auto_bitmap_item *item = m_auto_bitmap_list.first(); item != nullptr; item = item->next())
		item->m_bitmap.resize(effwidth, effheight);

	// re-set up textures
	if (m_palette != nullptr)
	{
		m_bitmap[0].set_palette(m_palette->palette());
		m_bitmap[1].set_palette(m_palette->palette());
	}
	m_texture[0]->set_bitmap(m_bitmap[0], m_visarea, m_bitmap[0].texformat());
	m_texture[1]->set_bitmap(m_bitmap[1], m_visarea, m_bitmap[1].texformat());
}


//-------------------------------------------------
//  set_visible_area - just set the visible area
//-------------------------------------------------

void screen_device::set_visible_area(int min_x, int max_x, int min_y, int max_y)
{
	rectangle visarea(min_x, max_x, min_y, max_y);
	assert(!visarea.empty());
	configure(m_width, m_height, visarea, m_frame_period);
}


//-------------------------------------------------
//  update_partial - perform a partial update from
//  the last scanline up to and including the
//  specified scanline
//-----------------------------------------------*/

bool screen_device::update_partial(int scanline)
{
	// validate arguments
	assert(scanline >= 0);

	LOG_PARTIAL_UPDATES(("Partial: update_partial(%s, %d): ", tag().c_str(), scanline));

	// these two checks only apply if we're allowed to skip frames
	if (!(m_video_attributes & VIDEO_ALWAYS_UPDATE))
	{
		// if skipping this frame, bail
		if (machine().video().skip_this_frame())
		{
			LOG_PARTIAL_UPDATES(("skipped due to frameskipping\n"));
			return FALSE;
		}

		// skip if this screen is not visible anywhere
		if (!machine().render().is_live(*this))
		{
			LOG_PARTIAL_UPDATES(("skipped because screen not live\n"));
			return FALSE;
		}
	}

	// skip if we already rendered this line
	if (scanline < m_last_partial_scan)
	{
		LOG_PARTIAL_UPDATES(("skipped because line was already rendered\n"));
		return false;
	}

	// set the range of scanlines to render
	rectangle clip = m_visarea;
	if (m_last_partial_scan > clip.min_y)
		clip.min_y = m_last_partial_scan;
	if (scanline < clip.max_y)
		clip.max_y = scanline;

	// skip if entirely outside of visible area
	if (clip.min_y > clip.max_y)
	{
		LOG_PARTIAL_UPDATES(("skipped because outside of visible area\n"));
		return false;
	}

	// otherwise, render
	LOG_PARTIAL_UPDATES(("updating %d-%d\n", clip.min_y, clip.max_y));
	g_profiler.start(PROFILER_VIDEO);

	UINT32 flags;
	screen_bitmap &curbitmap = m_bitmap[m_curbitmap];
	switch (curbitmap.format())
	{
		default:
		case BITMAP_FORMAT_IND16:   flags = m_screen_update_ind16(*this, curbitmap.as_ind16(), clip);   break;
		case BITMAP_FORMAT_RGB32:   flags = m_screen_update_rgb32(*this, curbitmap.as_rgb32(), clip);   break;
	}

	m_partial_updates_this_frame++;
	g_profiler.stop();

	// if we modified the bitmap, we have to commit
	m_changed |= ~flags & UPDATE_HAS_NOT_CHANGED;

	// remember where we left off
	m_last_partial_scan = scanline + 1;
	return true;
}


//-------------------------------------------------
//  update_now - perform an update from the last
//  beam position up to the current beam position
//-------------------------------------------------

void screen_device::update_now()
{
	// these two checks only apply if we're allowed to skip frames
	if (!(m_video_attributes & VIDEO_ALWAYS_UPDATE))
	{
		// if skipping this frame, bail
		if (machine().video().skip_this_frame())
		{
			LOG_PARTIAL_UPDATES(("skipped due to frameskipping\n"));
			return;
		}

		// skip if this screen is not visible anywhere
		if (!machine().render().is_live(*this))
		{
			LOG_PARTIAL_UPDATES(("skipped because screen not live\n"));
			return;
		}
	}

	int current_vpos = vpos();
	int current_hpos = hpos();
	rectangle clip = m_visarea;

	LOG_PARTIAL_UPDATES(("update_now(): Y=%d, X=%d, last partial %d, partial hpos %d  (vis %d %d)\n", current_vpos, current_hpos, m_last_partial_scan, m_partial_scan_hpos, m_visarea.max_x, m_visarea.max_y));

	// start off by doing a partial update up to the line before us, in case that was necessary
	if (current_vpos > m_last_partial_scan)
	{
		// if the line before us was incomplete, we must do it in two pieces
		if (m_partial_scan_hpos > 0)
		{
			INT32 save_scan = m_partial_scan_hpos;
			update_partial(current_vpos - 2);
			m_partial_scan_hpos = save_scan;

			// now finish the previous partial scanline
			int scanline = current_vpos - 1;
			if (m_partial_scan_hpos > clip.min_x)
				clip.min_x = m_partial_scan_hpos;
			if (current_hpos < clip.max_x)
				clip.max_x = current_hpos;
			if (m_last_partial_scan > clip.min_y)
				clip.min_y = m_last_partial_scan;
			if (scanline < clip.max_y)
				clip.max_y = scanline;

			// if there's something to draw, do it
			if ((clip.min_x <= clip.max_x) && (clip.min_y <= clip.max_y))
			{
				g_profiler.start(PROFILER_VIDEO);

				screen_bitmap &curbitmap = m_bitmap[m_curbitmap];
				switch (curbitmap.format())
				{
					default:
					case BITMAP_FORMAT_IND16: m_screen_update_ind16(*this, curbitmap.as_ind16(), clip);   break;
					case BITMAP_FORMAT_RGB32: m_screen_update_rgb32(*this, curbitmap.as_rgb32(), clip);   break;
				}

				m_partial_updates_this_frame++;
				g_profiler.stop();
				m_partial_scan_hpos = 0;
				m_last_partial_scan = current_vpos + 1;
			}
		}
		else
		{
			update_partial(current_vpos - 1);
		}
	}

	// now draw this partial scanline
	clip = m_visarea;

	if (m_partial_scan_hpos > clip.min_x)
		clip.min_x = m_partial_scan_hpos;
	if (current_hpos < clip.max_x)
		clip.max_x = current_hpos;
	if (current_vpos > clip.min_y)
		clip.min_y = current_vpos;
	if (current_vpos < clip.max_y)
		clip.max_y = current_vpos;

	// and if there's something to draw, do it
	if ((clip.min_x <= clip.max_x) && (clip.min_y <= clip.max_y))
	{
		g_profiler.start(PROFILER_VIDEO);

		LOG_PARTIAL_UPDATES(("doing scanline partial draw: Y %d X %d-%d\n", clip.max_y, clip.min_x, clip.max_x));

		UINT32 flags;
		screen_bitmap &curbitmap = m_bitmap[m_curbitmap];
		switch (curbitmap.format())
		{
			default:
			case BITMAP_FORMAT_IND16:   flags = m_screen_update_ind16(*this, curbitmap.as_ind16(), clip);   break;
			case BITMAP_FORMAT_RGB32:   flags = m_screen_update_rgb32(*this, curbitmap.as_rgb32(), clip);   break;
		}

		m_partial_updates_this_frame++;
		g_profiler.stop();

		// if we modified the bitmap, we have to commit
		m_changed |= ~flags & UPDATE_HAS_NOT_CHANGED;

		// remember where we left off
		m_partial_scan_hpos = current_hpos;
		m_last_partial_scan = current_vpos;

		// if we completed the line, mark it so
		if (current_hpos >= m_visarea.max_x)
		{
			m_partial_scan_hpos = 0;
			m_last_partial_scan = current_vpos + 1;
		}
	}
}


//-------------------------------------------------
//  reset_partial_updates - reset the partial
//  updating state
//-------------------------------------------------

void screen_device::reset_partial_updates()
{
	m_last_partial_scan = 0;
	m_partial_scan_hpos = 0;
	m_partial_updates_this_frame = 0;
	m_scanline0_timer->adjust(time_until_pos(0));
}


//-------------------------------------------------
//  vpos - returns the current vertical position
//  of the beam
//-------------------------------------------------

int screen_device::vpos() const
{
	attoseconds_t delta = (machine().time() - m_vblank_start_time).as_attoseconds();
	int vpos;

	// round to the nearest pixel
	delta += m_pixeltime / 2;

	// compute the v position relative to the start of VBLANK
	vpos = delta / m_scantime;

	// adjust for the fact that VBLANK starts at the bottom of the visible area
	return (m_visarea.max_y + 1 + vpos) % m_height;
}


//-------------------------------------------------
//  hpos - returns the current horizontal position
//  of the beam
//-------------------------------------------------

int screen_device::hpos() const
{
	attoseconds_t delta = (machine().time() - m_vblank_start_time).as_attoseconds();

	// round to the nearest pixel
	delta += m_pixeltime / 2;

	// compute the v position relative to the start of VBLANK
	int vpos = delta / m_scantime;

	// subtract that from the total time
	delta -= vpos * m_scantime;

	// return the pixel offset from the start of this scanline
	return delta / m_pixeltime;
}


//-------------------------------------------------
//  time_until_pos - returns the amount of time
//  remaining until the beam is at the given
//  hpos,vpos
//-------------------------------------------------

attotime screen_device::time_until_pos(int vpos, int hpos) const
{
	// validate arguments
	assert(vpos >= 0);
	assert(hpos >= 0);

	// since we measure time relative to VBLANK, compute the scanline offset from VBLANK
	vpos += m_height - (m_visarea.max_y + 1);
	vpos %= m_height;

	// compute the delta for the given X,Y position
	attoseconds_t targetdelta = (attoseconds_t)vpos * m_scantime + (attoseconds_t)hpos * m_pixeltime;

	// if we're past that time (within 1/2 of a pixel), head to the next frame
	attoseconds_t curdelta = (machine().time() - m_vblank_start_time).as_attoseconds();
	if (targetdelta <= curdelta + m_pixeltime / 2)
		targetdelta += m_frame_period;
	while (targetdelta <= curdelta)
		targetdelta += m_frame_period;

	// return the difference
	return attotime(0, targetdelta - curdelta);
}


//-------------------------------------------------
//  time_until_vblank_end - returns the amount of
//  time remaining until the end of the current
//  VBLANK (if in progress) or the end of the next
//  VBLANK
//-------------------------------------------------

attotime screen_device::time_until_vblank_end() const
{
	// if we are in the VBLANK region, compute the time until the end of the current VBLANK period
	attotime target_time = m_vblank_end_time;
	if (!vblank())
		target_time += attotime(0, m_frame_period);
	return target_time - machine().time();
}


//-------------------------------------------------
//  register_vblank_callback - registers a VBLANK
//  callback
//-------------------------------------------------

void screen_device::register_vblank_callback(vblank_state_delegate vblank_callback)
{
	// validate arguments
	assert(!vblank_callback.isnull());

	// check if we already have this callback registered
	callback_item *item;
	for (item = m_callback_list.first(); item != nullptr; item = item->next())
		if (item->m_callback == vblank_callback)
			break;

	// if not found, register
	if (item == nullptr)
		m_callback_list.append(*global_alloc(callback_item(vblank_callback)));
}


//-------------------------------------------------
//  register_screen_bitmap - registers a bitmap
//  that should track the screen size
//-------------------------------------------------

void screen_device::register_screen_bitmap(bitmap_t &bitmap)
{
	// append to the list
	m_auto_bitmap_list.append(*global_alloc(auto_bitmap_item(bitmap)));

	// if allocating now, just do it
	bitmap.allocate(width(), height());
	if (m_palette != nullptr)
		bitmap.set_palette(m_palette->palette());
}


//-------------------------------------------------
//  vblank_begin - call any external callbacks to
//  signal the VBLANK period has begun
//-------------------------------------------------

void screen_device::vblank_begin()
{
	// reset the starting VBLANK time
	m_vblank_start_time = machine().time();
	m_vblank_end_time = m_vblank_start_time + attotime(0, m_vblank_period);

	// if this is the primary screen and we need to update now
	if (this == machine().first_screen() && !(m_video_attributes & VIDEO_UPDATE_AFTER_VBLANK))
		machine().video().frame_update();

	// call the screen specific callbacks
	for (callback_item *item = m_callback_list.first(); item != nullptr; item = item->next())
		item->m_callback(*this, true);
	if (!m_screen_vblank.isnull())
		m_screen_vblank(*this, true);

	// reset the VBLANK start timer for the next frame
	m_vblank_begin_timer->adjust(time_until_vblank_start());

	// if no VBLANK period, call the VBLANK end callback immediately, otherwise reset the timer
	if (m_vblank_period == 0)
		vblank_end();
	else
		m_vblank_end_timer->adjust(time_until_vblank_end());
}


//-------------------------------------------------
//  vblank_end - call any external callbacks to
//  signal the VBLANK period has ended
//-------------------------------------------------

void screen_device::vblank_end()
{
	// call the screen specific callbacks
	for (callback_item *item = m_callback_list.first(); item != nullptr; item = item->next())
		item->m_callback(*this, false);
	if (!m_screen_vblank.isnull())
		m_screen_vblank(*this, false);

	// if this is the primary screen and we need to update now
	if (this == machine().first_screen() && (m_video_attributes & VIDEO_UPDATE_AFTER_VBLANK))
		machine().video().frame_update();

	// increment the frame number counter
	m_frame_number++;
}


//-------------------------------------------------
//  update_quads - set up the quads for this
//  screen
//-------------------------------------------------

bool screen_device::update_quads()
{
	// only update if live
	if (machine().render().is_live(*this))
	{
		// only update if empty and not a vector game; otherwise assume the driver did it directly
		if (m_type != SCREEN_TYPE_VECTOR && (m_video_attributes & VIDEO_SELF_RENDER) == 0)
		{
			// if we're not skipping the frame and if the screen actually changed, then update the texture
			if (!machine().video().skip_this_frame() && m_changed)
			{
				m_texture[m_curbitmap]->set_bitmap(m_bitmap[m_curbitmap], m_visarea, m_bitmap[m_curbitmap].texformat());
				m_curtexture = m_curbitmap;
				m_curbitmap = 1 - m_curbitmap;
			}

			// brightness adjusted render color
			rgb_t color = m_color - rgb_t(0, 0xff - m_brightness, 0xff - m_brightness, 0xff - m_brightness);

			// create an empty container with a single quad
			m_container->empty();
			m_container->add_quad(0.0f, 0.0f, 1.0f, 1.0f, color, m_texture[m_curtexture], PRIMFLAG_BLENDMODE(BLENDMODE_NONE) | PRIMFLAG_SCREENTEX(1));
		}
	}

	// reset the screen changed flags
	bool result = m_changed;
	m_changed = false;
	return result;
}


//-------------------------------------------------
//  update_burnin - update the burnin bitmap
//-------------------------------------------------

void screen_device::update_burnin()
{
#undef rand
	if (!m_burnin.valid())
		return;

	screen_bitmap &curbitmap = m_bitmap[m_curtexture];
	if (!curbitmap.valid())
		return;

	int srcwidth = curbitmap.width();
	int srcheight = curbitmap.height();
	int dstwidth = m_burnin.width();
	int dstheight = m_burnin.height();
	int xstep = (srcwidth << 16) / dstwidth;
	int ystep = (srcheight << 16) / dstheight;
	int xstart = ((UINT32)rand() % 32767) * xstep / 32767;
	int ystart = ((UINT32)rand() % 32767) * ystep / 32767;
	int srcx, srcy;
	int x, y;

	switch (curbitmap.format())
	{
		default:
		case BITMAP_FORMAT_IND16:
		{
			// iterate over rows in the destination
			bitmap_ind16 &srcbitmap = curbitmap.as_ind16();
			for (y = 0, srcy = ystart; y < dstheight; y++, srcy += ystep)
			{
				UINT64 *dst = &m_burnin.pix64(y);
				const UINT16 *src = &srcbitmap.pix16(srcy >> 16);
				const rgb_t *palette = m_palette->palette()->entry_list_adjusted();
				for (x = 0, srcx = xstart; x < dstwidth; x++, srcx += xstep)
				{
					rgb_t pixel = palette[src[srcx >> 16]];
					dst[x] += pixel.g() + pixel.r() + pixel.b();
				}
			}
			break;
		}

		case BITMAP_FORMAT_RGB32:
		{
			// iterate over rows in the destination
			bitmap_rgb32 &srcbitmap = curbitmap.as_rgb32();
			for (y = 0, srcy = ystart; y < dstheight; y++, srcy += ystep)
			{
				UINT64 *dst = &m_burnin.pix64(y);
				const UINT32 *src = &srcbitmap.pix32(srcy >> 16);
				for (x = 0, srcx = xstart; x < dstwidth; x++, srcx += xstep)
				{
					rgb_t pixel = src[srcx >> 16];
					dst[x] += pixel.g() + pixel.r() + pixel.b();
				}
			}
			break;
		}
	}
}


//-------------------------------------------------
//  finalize_burnin - finalize the burnin bitmap
//-------------------------------------------------

void screen_device::finalize_burnin()
{
	if (!m_burnin.valid())
		return;

	// compute the scaled visible region
	rectangle scaledvis;
	scaledvis.min_x = m_visarea.min_x * m_burnin.width() / m_width;
	scaledvis.max_x = m_visarea.max_x * m_burnin.width() / m_width;
	scaledvis.min_y = m_visarea.min_y * m_burnin.height() / m_height;
	scaledvis.max_y = m_visarea.max_y * m_burnin.height() / m_height;

	// wrap a bitmap around the memregion we care about
	bitmap_argb32 finalmap(scaledvis.width(), scaledvis.height());
	int srcwidth = m_burnin.width();
	int srcheight = m_burnin.height();
	int dstwidth = finalmap.width();
	int dstheight = finalmap.height();
	int xstep = (srcwidth << 16) / dstwidth;
	int ystep = (srcheight << 16) / dstheight;

	// find the maximum value
	UINT64 minval = ~(UINT64)0;
	UINT64 maxval = 0;
	for (int y = 0; y < srcheight; y++)
	{
		UINT64 *src = &m_burnin.pix64(y);
		for (int x = 0; x < srcwidth; x++)
		{
			minval = MIN(minval, src[x]);
			maxval = MAX(maxval, src[x]);
		}
	}

	if (minval == maxval)
		return;

	// now normalize and convert to RGB
	for (int y = 0, srcy = 0; y < dstheight; y++, srcy += ystep)
	{
		UINT64 *src = &m_burnin.pix64(srcy >> 16);
		UINT32 *dst = &finalmap.pix32(y);
		for (int x = 0, srcx = 0; x < dstwidth; x++, srcx += xstep)
		{
			int brightness = (UINT64)(maxval - src[srcx >> 16]) * 255 / (maxval - minval);
			dst[x] = rgb_t(0xff, brightness, brightness, brightness);
		}
	}

	// write the final PNG

	// compute the name and create the file
	emu_file file(machine().options().snapshot_directory(), OPEN_FLAG_WRITE | OPEN_FLAG_CREATE | OPEN_FLAG_CREATE_PATHS);
	file_error filerr = file.open(machine().basename(), PATH_SEPARATOR "burnin-", this->tag().c_str() + 1, ".png") ;
	if (filerr == FILERR_NONE)
	{
		png_info pnginfo = { nullptr };
//      png_error pngerr;
		char text[256];

		// add two text entries describing the image
		sprintf(text,"%s %s", emulator_info::get_appname(), build_version);
		png_add_text(&pnginfo, "Software", text);
		sprintf(text, "%s %s", machine().system().manufacturer, machine().system().description);
		png_add_text(&pnginfo, "System", text);

		// now do the actual work
		png_write_bitmap(file, &pnginfo, finalmap, 0, nullptr);

		// free any data allocated
		png_free(&pnginfo);
	}
}


//-------------------------------------------------
//  finalize_burnin - finalize the burnin bitmap
//-------------------------------------------------

void screen_device::load_effect_overlay(const char *filename)
{
	// ensure that there is a .png extension
	std::string fullname(filename);
	int extension = fullname.find_last_of('.');
	if (extension != -1)
		fullname.erase(extension, -1);
	fullname.append(".png");

	// load the file
	emu_file file(machine().options().art_path(), OPEN_FLAG_READ);
	render_load_png(m_screen_overlay_bitmap, file, nullptr, fullname.c_str());
	if (m_screen_overlay_bitmap.valid())
		m_container->set_overlay(&m_screen_overlay_bitmap);
	else
		osd_printf_warning("Unable to load effect PNG file '%s'\n", fullname.c_str());
}
