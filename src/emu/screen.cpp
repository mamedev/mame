// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    screen.cpp

    Core MAME screen device.

***************************************************************************/

#include "emu.h"
#include "screen.h"

#include "emuopts.h"
#include "fileio.h"
#include "main.h"
#include "render.h"
#include "rendutil.h"
#include "video.h"

#include "png.h"

#include <cstdio>
#include <set>


//**************************************************************************
//  DEBUGGING
//**************************************************************************

#define VERBOSE                     (0)
#define LOG_PARTIAL_UPDATES(x)      do { if (VERBOSE) logerror x; } while (0)



//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

// device type definition
DEFINE_DEVICE_TYPE(SCREEN, screen_device, "screen", "Video Screen")

const attotime screen_device::DEFAULT_FRAME_PERIOD(attotime::from_hz(DEFAULT_FRAME_RATE));

//**************************************************************************
//  SCREEN DEVICE
//**************************************************************************

//-------------------------------------------------
//  screen_device - constructor
//-------------------------------------------------

screen_device::screen_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, SCREEN, tag, owner, clock)
	, device_video_output_interface(mconfig, *this)
	, m_is_lcd(false)
	, m_phys_aspect(0U, 0U)
	, m_oldstyle_vblank_supplied(false)
	, m_vblank(0)
	, m_xoffset(0.0f)
	, m_yoffset(0.0f)
	, m_xscale(1.0f)
	, m_yscale(1.0f)
	, m_screen_update_ind16(*this)
	, m_screen_update_rgb32(*this)
	, m_screen_vblank(*this)
	, m_scanline_cb(*this)
	, m_palette(*this, finder_base::DUMMY_TAG)
	, m_video_attributes(0)
	, m_max_width(100)
	, m_width(100)
	, m_height(100)
	, m_visarea(0, 99, 0, 99)
	, m_has_setup(false)
	, m_texformat()
	, m_curbitmap(0)
	, m_curtexture(0)
	, m_changed(true)
	, m_last_partial_reset(attotime::zero)
	, m_last_partial_scan(0)
	, m_partial_scan_hpos(0)
	, m_color(rgb_t(0xff, 0xff, 0xff, 0xff))
	, m_brightness(0xff)
	, m_frame_period(DEFAULT_FRAME_PERIOD.as_attoseconds())
	, m_scantime(1)
	, m_pixeltime(1)
	, m_vblank_period(0)
	, m_vblank_start_time(attotime::zero)
	, m_vblank_end_time(attotime::zero)
	, m_vblank_begin_timer(nullptr)
	, m_vblank_end_timer(nullptr)
	, m_scanline0_timer(nullptr)
	, m_scanline_timer(nullptr)
	, m_frame_number(0)
	, m_partial_updates_this_frame(0)
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
	destroy_scan_bitmaps();
}

//-------------------------------------------------
//  output_type_name - return "raster" or "lcd"
//-------------------------------------------------

const char *screen_device::output_type_name() const
{
	return m_is_lcd ? "lcd" : "raster";
}

//-------------------------------------------------
//  destroy_scan_bitmaps - destroy per-scanline
//  bitmaps if applicable
//-------------------------------------------------

void screen_device::destroy_scan_bitmaps()
{
	if (m_video_attributes & VIDEO_VARIABLE_WIDTH)
	{
		const bool screen16 = !m_screen_update_ind16.isnull();
		for (int j = 0; j < 2; j++)
		{
			for (bitmap_t* bitmap : m_scan_bitmaps[j])
			{
				if (screen16)
					delete (bitmap_ind16*)bitmap;
				else
					delete (bitmap_rgb32*)bitmap;
			}
			m_scan_bitmaps[j].clear();
		}
	}
}


//-------------------------------------------------
//  allocate_scan_bitmaps - allocate per-scanline
//  bitmaps if applicable
//-------------------------------------------------

void screen_device::allocate_scan_bitmaps()
{
	if (m_video_attributes & VIDEO_VARIABLE_WIDTH)
	{
		const bool screen16 = !m_screen_update_ind16.isnull();
		s32 effwidth = std::max(m_max_width, m_visarea.right() + 1);
		const s32 old_height = (s32)m_scan_widths.size();
		s32 effheight = std::max(m_height, m_visarea.bottom() + 1);
		if (old_height < effheight)
		{
			for (int i = old_height; i < effheight; i++)
			{
				for (int j = 0; j < 2; j++)
				{
					if (screen16)
						m_scan_bitmaps[j].push_back(new bitmap_ind16(effwidth, 1));
					else
						m_scan_bitmaps[j].push_back(new bitmap_rgb32(effwidth, 1));
				}
				m_scan_widths.push_back(effwidth);
			}
		}
		else
		{
			for (int i = old_height - 1; i >= effheight; i--)
			{
				for (int j = 0; j < 2; j++)
				{
					if (screen16)
						delete (bitmap_ind16 *)m_scan_bitmaps[j][i];
					else
						delete (bitmap_rgb32 *)m_scan_bitmaps[j][i];
					m_scan_bitmaps[j].erase(m_scan_bitmaps[j].begin() + i);
				}
				m_scan_widths.erase(m_scan_widths.begin() + i);
			}
		}
	}
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
	if (m_visarea.empty() || m_visarea.right() >= m_width || m_visarea.bottom() >= m_height)
		osd_printf_error("Invalid display area\n");

	// sanity check screen formats
	if (m_screen_update_ind16.isnull() && m_screen_update_rgb32.isnull())
		osd_printf_error("Missing SCREEN_UPDATE function\n");

	// check for invalid frame rate
	if (m_frame_period == 0 || m_frame_period > ATTOSECONDS_PER_SECOND)
		osd_printf_error("Invalid (under 1Hz) refresh rate\n");

	texture_format texformat = !m_screen_update_ind16.isnull() ? TEXFORMAT_PALETTE16 : TEXFORMAT_RGB32;
	if (m_palette.finder_tag() != finder_base::DUMMY_TAG)
	{
		if (!m_palette)
			osd_printf_error("Screen references non-existent palette tag %s\n", m_palette.finder_tag());

		if (texformat == TEXFORMAT_RGB32)
			osd_printf_warning("Screen does not need palette defined\n");
	}
	else if (texformat == TEXFORMAT_PALETTE16)
	{
		osd_printf_error("Screen does not have palette defined\n");
	}
}


//-------------------------------------------------
//  physical_aspect - determine the physical
//  aspect ratio to be used for rendering
//-------------------------------------------------

std::pair<unsigned, unsigned> screen_device::physical_aspect() const
{
	assert(configured());

	std::pair<unsigned, unsigned> phys_aspect = m_phys_aspect;

	// physical aspect ratio unconfigured
	if (!phys_aspect.first || !phys_aspect.second)
	{
		if (m_is_lcd)
			phys_aspect = std::make_pair(~0U, ~0U); // assume square pixels
		else
			phys_aspect = std::make_pair(4, 3); // assume standard CRT
	}

	// square pixels?
	if ((~0U == phys_aspect.first) && (~0U == phys_aspect.second))
	{
		phys_aspect.first = visible_area().width();
		phys_aspect.second = visible_area().height();
	}

	// always keep this in reduced form
	util::reduce_fraction(phys_aspect.first, phys_aspect.second);

	return phys_aspect;
}


//-------------------------------------------------
//  device_resolve_objects - resolve objects that
//  may be needed for other devices to set
//  initial conditions at start time
//-------------------------------------------------

void screen_device::device_resolve_objects()
{
	// bind our handlers
	m_screen_update_ind16.resolve();
	m_screen_update_rgb32.resolve();

	// assign our format to the palette before it starts
	if (m_palette)
		m_palette->m_format = format();
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void screen_device::device_start()
{
	// if we have a palette and it's not started, wait for it
	if (m_palette && !m_palette->device().started())
		throw device_missing_dependencies();

	// configure bitmap formats and allocate screen bitmaps
	const bool screen16 = !m_screen_update_ind16.isnull();
	texture_format texformat = screen16 ? TEXFORMAT_PALETTE16 : TEXFORMAT_RGB32;

	for (auto & elem : m_bitmap)
	{
		elem.set_format(format(), texformat);
		register_screen_bitmap(elem);
	}
	register_screen_bitmap(m_priority);

	// allocate raw textures
	m_texture[0] = machine().render().texture_alloc();
	m_texture[0]->set_id(u64(m_unique_id) << 57);
	m_texture[1] = machine().render().texture_alloc();
	m_texture[1]->set_id((u64(m_unique_id) << 57) | 1);

	// configure the default cliparea
	render_container::user_settings settings = m_container->get_user_settings();
	settings.m_xoffset = m_xoffset;
	settings.m_yoffset = m_yoffset;
	settings.m_xscale = m_xscale;
	settings.m_yscale = m_yscale;
	m_container->set_user_settings(settings);

	// allocate the VBLANK timers
	m_vblank_begin_timer = timer_alloc(FUNC(screen_device::vblank_begin), this);
	m_vblank_end_timer = timer_alloc(FUNC(screen_device::vblank_end), this);

	// allocate a timer to reset partial updates
	m_scanline0_timer = timer_alloc(FUNC(screen_device::first_scanline_tick), this);

	// allocate a timer to generate per-scanline updates
	if ((m_video_attributes & VIDEO_UPDATE_SCANLINE) != 0 || !m_scanline_cb.isunset())
		m_scanline_timer = timer_alloc(FUNC(screen_device::scanline_tick), this);

	// configure the screen with the default parameters
	configure(m_width, m_height, m_visarea, attotime(0, m_frame_period));

	// reset VBLANK timing
	m_vblank_start_time = attotime::zero;
	m_vblank_end_time = attotime(0, m_vblank_period);

	// start the timer to generate per-scanline updates
	if ((m_video_attributes & VIDEO_UPDATE_SCANLINE) != 0 || !m_scanline_cb.isunset())
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
	save_item(NAME(m_last_partial_reset));
	save_item(NAME(m_last_partial_scan));
	save_item(NAME(m_frame_period));
	save_item(NAME(m_brightness));
	save_item(NAME(m_scantime));
	save_item(NAME(m_pixeltime));
	save_item(NAME(m_vblank_period));
	save_item(NAME(m_vblank_start_time));
	save_item(NAME(m_vblank_end_time));
	save_item(NAME(m_frame_number));
	if (m_oldstyle_vblank_supplied)
		logerror("%s: Deprecated legacy Old Style screen configured (set_vblank_time), please use set_raw instead.\n",this->tag());
}


//-------------------------------------------------
//  device_reset - device-specific startup
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
//  timer events
//-------------------------------------------------

TIMER_CALLBACK_MEMBER(screen_device::first_scanline_tick)
{
	// first scanline
	reset_partial_updates();
	if (m_video_attributes & VIDEO_VARIABLE_WIDTH)
	{
		pre_update_scanline(0);
	}
}

TIMER_CALLBACK_MEMBER(screen_device::scanline_tick)
{
	// subsequent scanlines when scanline updates are enabled
	if (m_video_attributes & VIDEO_VARIABLE_WIDTH)
	{
		pre_update_scanline(param);
	}
	if (m_video_attributes & VIDEO_UPDATE_SCANLINE)
	{
		// force a partial update to the current scanline
		update_partial(param);
	}
	if (!m_scanline_cb.isunset())
		m_scanline_cb(param);

	// compute the next visible scanline
	param++;
	if (param > m_visarea.bottom())
		param = m_visarea.top();
	m_scanline_timer->adjust(time_until_pos(param), param);
}


//-------------------------------------------------
//  configure - configure screen parameters
//-------------------------------------------------

void screen_device::configure(int width, int height, const rectangle &visarea, attotime frame_period)
{
	// validate arguments
	assert(width > 0);
	assert(height > 0);
	assert(visarea.left() >= 0);
	assert(visarea.top() >= 0);
//  assert(visarea.right() < width);
//  assert(visarea.bottom() < height);
	assert(visarea.left() < width);
	assert(visarea.top() < height);

	// fill in the new parameters
	m_max_width = std::max(m_max_width, width);
	m_width = width;
	m_height = height;
	m_visarea = visarea;

	// reallocate bitmap(s) if necessary
	realloc_screen_bitmaps();

	// compute timing parameters
	m_frame_period = frame_period.as_attoseconds();
	m_scantime = m_frame_period / height;
	m_pixeltime = m_frame_period / (height * width);

	// if an old style VBLANK_TIME was specified in the MACHINE_CONFIG,
	// use it; otherwise calculate the VBLANK period from the visible area
	if (m_oldstyle_vblank_supplied)
		m_vblank_period = m_vblank;
	else
		m_vblank_period = m_scantime * (height - visarea.height());

	// we are now fully configured with the new parameters
	// and can safely call time_until_pos(), etc.

	// if the frame period was reduced so that we are now past the end of the frame,
	// call the VBLANK start timer now; otherwise, adjust it for the future
	attoseconds_t delta = (machine().time() - m_vblank_start_time).as_attoseconds();
	if (delta >= m_frame_period)
		vblank_begin(0);
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
//  override_frame_period - forcibly change the
//  refresh rate (from the sliders)
//-------------------------------------------------

void screen_device::override_frame_period(attotime period)
{
	configure(m_width, m_height, m_visarea, period);
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

	// if we are resetting relative to (visarea.bottom() + 1, 0) == VBLANK start,
	// call the VBLANK start timer now; otherwise, adjust it for the future
	if (beamy == ((m_visarea.bottom() + 1) % m_height) && beamx == 0)
		vblank_begin(0);
	else
		m_vblank_begin_timer->adjust(time_until_vblank_start());

	// if we are resetting relative to (0,0) == VBLANK end, call the
	// scanline 0 timer by hand now; otherwise, adjust it for the future
	if (beamy == 0 && beamx == 0)
		reset_partial_updates();
	else
		m_scanline0_timer->adjust(time_until_pos(0));
}


//-------------------------------------------------
//  update_scan_bitmap_size - reallocate the
//  bitmap for a specific scanline
//-------------------------------------------------

void screen_device::update_scan_bitmap_size(int y)
{
	// don't update this line if it exceeds the allocated size, which can happen on initial configuration
	if (y >= m_scan_widths.size())
		return;

	// determine effective size to allocate
	s32 effwidth = std::max(m_max_width, m_visarea.right() + 1);

	if (m_scan_widths[y] == effwidth)
		return;

	m_scan_bitmaps[m_curbitmap][y]->resize(effwidth, 1);
	m_scan_widths[y] = effwidth;
}


//-------------------------------------------------
//  realloc_screen_bitmaps - reallocate bitmaps
//  and textures as necessary
//-------------------------------------------------

void screen_device::realloc_screen_bitmaps()
{
	// determine effective size to allocate
	const bool per_scanline = (m_video_attributes & VIDEO_VARIABLE_WIDTH);
	s32 effwidth = std::max(per_scanline ? m_max_width : m_width, m_visarea.right() + 1);
	s32 effheight = std::max(m_height, m_visarea.bottom() + 1);

	// resize all registered screen bitmaps
	for (auto &item : m_auto_bitmap_list)
		item->m_bitmap.resize(effwidth, effheight);

	// re-set up textures
	if (m_palette)
	{
		m_bitmap[0].set_palette(m_palette->palette());
		m_bitmap[1].set_palette(m_palette->palette());
	}
	m_texture[0]->set_bitmap(m_bitmap[0], m_visarea, m_bitmap[0].texformat());
	m_texture[1]->set_bitmap(m_bitmap[1], m_visarea, m_bitmap[1].texformat());

	allocate_scan_bitmaps();
}


//-------------------------------------------------
//  pre_update_scanline - check if the bitmap for
//  a specific scanline needs its size updated
//-------------------------------------------------

void screen_device::pre_update_scanline(int y)
{
	update_scan_bitmap_size(y);
}


//-------------------------------------------------
//  set_visible_area - just set the visible area
//-------------------------------------------------

void screen_device::set_visible_area(int min_x, int max_x, int min_y, int max_y)
{
	rectangle visarea(min_x, max_x, min_y, max_y);
	assert(!visarea.empty());
	configure(m_width, m_height, visarea, attotime(0, m_frame_period));
}


//-------------------------------------------------
//  update_partial (scanline) - perform a partial
//  update from the last scanline up to and
//  including the specified scanline
//-----------------------------------------------*/

bool screen_device::update_partial(int scanline)
{
	LOG_PARTIAL_UPDATES(("Partial: update_partial(%s, %d): ", tag(), scanline));

	// these two checks only apply if we're allowed to skip frames
	if (!(m_video_attributes & VIDEO_ALWAYS_UPDATE))
	{
		// if skipping this frame, bail
		if (machine().video().skip_this_frame())
		{
			LOG_PARTIAL_UPDATES(("skipped due to frameskipping\n"));
			return false;
		}

		// skip if this screen is not visible anywhere
		if (!machine().render().is_live(*this))
		{
			LOG_PARTIAL_UPDATES(("skipped because screen not live\n"));
			return false;
		}
	}

	// skip if we already rendered this line
	if (scanline < m_last_partial_scan)
	{
		LOG_PARTIAL_UPDATES(("skipped because line was already rendered\n"));
		return false;
	}

	// skip if we already rendered this frame
	// this can happen if a cpu timeslice that called update_partial is in the previous frame while scanline 0 already started
	if (m_last_partial_scan == 0 && m_last_partial_reset > machine().time())
	{
		LOG_PARTIAL_UPDATES(("skipped because frame was already rendered\n"));
		return false;
	}

	// if we left off in the middle of a scanline, eg. with update_now(), finish that scanline first
	if (m_partial_scan_hpos > 0)
	{
		update_partial(m_last_partial_scan + 1, 0);

		// check again if scanline was already rendered
		if (scanline < m_last_partial_scan)
			return true;
	}

	// set the range of scanlines to render
	rectangle clip(m_visarea);
	clip.sety((std::max)(clip.top(), m_last_partial_scan), (std::min)(clip.bottom(), scanline));

	// skip if entirely outside of visible area
	if (clip.top() > clip.bottom())
	{
		LOG_PARTIAL_UPDATES(("skipped because outside of visible area\n"));
		return false;
	}

	// otherwise, render
	LOG_PARTIAL_UPDATES(("updating %d-%d\n", clip.top(), clip.bottom()));

	u32 flags = 0;
	{
		auto profile = g_profiler.start(PROFILER_VIDEO);
		if (m_video_attributes & VIDEO_VARIABLE_WIDTH)
		{
			rectangle scan_clip(clip);
			for (int y = clip.top(); y <= clip.bottom(); y++)
			{
				scan_clip.sety(y, y);
				pre_update_scanline(y);

				screen_bitmap &curbitmap = m_bitmap[m_curbitmap];
				switch (curbitmap.format())
				{
					default:
					case BITMAP_FORMAT_IND16:   flags |= m_screen_update_ind16(*this, *(bitmap_ind16 *)m_scan_bitmaps[m_curbitmap][y], scan_clip);   break;
					case BITMAP_FORMAT_RGB32:   flags |= m_screen_update_rgb32(*this, *(bitmap_rgb32 *)m_scan_bitmaps[m_curbitmap][y], scan_clip);   break;
				}

				m_partial_updates_this_frame++;
			}
		}
		else
		{
			screen_bitmap &curbitmap = m_bitmap[m_curbitmap];
			switch (curbitmap.format())
			{
				default:
				case BITMAP_FORMAT_IND16:   flags = m_screen_update_ind16(*this, curbitmap.as_ind16(), clip);   break;
				case BITMAP_FORMAT_RGB32:   flags = m_screen_update_rgb32(*this, curbitmap.as_rgb32(), clip);   break;
			}
			m_partial_updates_this_frame++;
		}
		// stop profiling
	}

	// if we modified the bitmap, we have to commit
	m_changed |= ~flags & UPDATE_HAS_NOT_CHANGED;

	// remember where we left off
	m_last_partial_scan = scanline + 1;
	m_partial_scan_hpos = 0;
	return true;
}


//-------------------------------------------------
//  update_partial (vpos, hpos) - perform an update
//  from the last beam position up to (but not
//  including!) the newly specified beam position
//-------------------------------------------------

bool screen_device::update_partial(int vpos, int hpos)
{
	// these two checks only apply if we're allowed to skip frames
	if (!(m_video_attributes & VIDEO_ALWAYS_UPDATE))
	{
		// if skipping this frame, bail
		if (machine().video().skip_this_frame())
		{
			LOG_PARTIAL_UPDATES(("skipped due to frameskipping\n"));
			return false;
		}

		// skip if this screen is not visible anywhere
		if (!machine().render().is_live(*this))
		{
			LOG_PARTIAL_UPDATES(("skipped because screen not live\n"));
			return false;
		}
	}

	rectangle clip = m_visarea;

	// skip if we already rendered this line
	if (vpos < m_last_partial_scan)
	{
		LOG_PARTIAL_UPDATES(("skipped because line was already rendered\n"));
		return false;
	}

	// if beam position is the same or less, there's nothing to update
	if (vpos == m_last_partial_scan && hpos <= m_partial_scan_hpos)
	{
		LOG_PARTIAL_UPDATES(("skipped because beam position already passed\n"));
		return false;
	}

	// skip if we already rendered this frame
	// this can happen if a cpu timeslice that called update_partial is in the previous frame while scanline 0 already started
	if (m_last_partial_scan == 0 && m_partial_scan_hpos == 0 && m_last_partial_reset > machine().time())
	{
		LOG_PARTIAL_UPDATES(("skipped because frame was already rendered\n"));
		return false;
	}

	LOG_PARTIAL_UPDATES(("update_partial(): Y=%d, X=%d, last partial %d, partial hpos %d  (vis %d %d)\n", vpos, hpos, m_last_partial_scan, m_partial_scan_hpos, m_visarea.right(), m_visarea.bottom()));

	// start off by doing a partial update up to the line before us, in case that was necessary
	if (vpos > m_last_partial_scan)
	{
		// if the line before us was incomplete, we must do it in two pieces
		if (m_partial_scan_hpos > 0)
		{
			// now finish the previous partial scanline
			clip.set((std::max)(clip.left(), m_partial_scan_hpos),
					 clip.right(),
					 (std::max)(clip.top(), m_last_partial_scan),
					 (std::min)(clip.bottom(), m_last_partial_scan));

			// if there's something to draw, do it
			if (!clip.empty())
			{
				auto profile = g_profiler.start(PROFILER_VIDEO);

				u32 flags = 0;
				screen_bitmap &curbitmap = m_bitmap[m_curbitmap];
				if (m_video_attributes & VIDEO_VARIABLE_WIDTH)
				{
					pre_update_scanline(m_last_partial_scan);
					switch (curbitmap.format())
					{
						default:
						case BITMAP_FORMAT_IND16:   flags = m_screen_update_ind16(*this, *(bitmap_ind16 *)m_scan_bitmaps[m_curbitmap][m_last_partial_scan], clip);   break;
						case BITMAP_FORMAT_RGB32:   flags = m_screen_update_rgb32(*this, *(bitmap_rgb32 *)m_scan_bitmaps[m_curbitmap][m_last_partial_scan], clip);   break;
					}
				}
				else
				{
					switch (curbitmap.format())
					{
						default:
						case BITMAP_FORMAT_IND16:   flags = m_screen_update_ind16(*this, curbitmap.as_ind16(), clip);   break;
						case BITMAP_FORMAT_RGB32:   flags = m_screen_update_rgb32(*this, curbitmap.as_rgb32(), clip);   break;
					}
				}

				m_partial_updates_this_frame++;

				// if we modified the bitmap, we have to commit
				m_changed |= ~flags & UPDATE_HAS_NOT_CHANGED;
			}

			m_partial_scan_hpos = 0;
			m_last_partial_scan++;
		}
		if (vpos > m_last_partial_scan)
		{
			update_partial(vpos - 1);
		}
	}

	// now draw this partial scanline
	if (hpos > 0)
	{
		clip = m_visarea;

		clip.set((std::max)(clip.left(), m_partial_scan_hpos),
				(std::min)(clip.right(), hpos - 1),
				(std::max)(clip.top(), vpos),
				(std::min)(clip.bottom(), vpos));

		// and if there's something to draw, do it
		if (!clip.empty())
		{
			auto profile = g_profiler.start(PROFILER_VIDEO);

			LOG_PARTIAL_UPDATES(("doing scanline partial draw: Y %d X %d-%d\n", clip.bottom(), clip.left(), clip.right()));

			u32 flags = 0;
			screen_bitmap &curbitmap = m_bitmap[m_curbitmap];
			if (m_video_attributes & VIDEO_VARIABLE_WIDTH)
			{
				pre_update_scanline(vpos);
				switch (curbitmap.format())
				{
					default:
					case BITMAP_FORMAT_IND16:   flags = m_screen_update_ind16(*this, *(bitmap_ind16 *)m_scan_bitmaps[m_curbitmap][vpos], clip);   break;
					case BITMAP_FORMAT_RGB32:   flags = m_screen_update_rgb32(*this, *(bitmap_rgb32 *)m_scan_bitmaps[m_curbitmap][vpos], clip);   break;
				}
			}
			else
			{
				switch (curbitmap.format())
				{
					default:
					case BITMAP_FORMAT_IND16:   flags = m_screen_update_ind16(*this, curbitmap.as_ind16(), clip);   break;
					case BITMAP_FORMAT_RGB32:   flags = m_screen_update_rgb32(*this, curbitmap.as_rgb32(), clip);   break;
				}
			}

			m_partial_updates_this_frame++;

			// if we modified the bitmap, we have to commit
			m_changed |= ~flags & UPDATE_HAS_NOT_CHANGED;
		}
	}

	// remember where we left off
	m_partial_scan_hpos = hpos;
	m_last_partial_scan = vpos;
	return true;
}


//-------------------------------------------------
//  reset_partial_updates - reset the partial
//  updating state
//-------------------------------------------------

void screen_device::reset_partial_updates()
{
	m_last_partial_reset = machine().time();
	m_last_partial_scan = 0;
	m_partial_scan_hpos = 0;
	m_partial_updates_this_frame = 0;
	m_scanline0_timer->adjust(time_until_pos(0));
}


//-------------------------------------------------
//  pixel - returns the RGB value of the specified
//  pixel location
//-------------------------------------------------

u32 screen_device::pixel(s32 x, s32 y)
{
	screen_bitmap &curbitmap = m_bitmap[m_curbitmap];
	if (!curbitmap.valid())
		return 0;

	const int srcwidth = curbitmap.width();
	const int srcheight = curbitmap.height();

	if (x < 0 || y < 0 || x >= srcwidth || y >= srcheight)
		return 0;

	const bool per_scanline = (m_video_attributes & VIDEO_VARIABLE_WIDTH);

	switch (curbitmap.format())
	{
		case BITMAP_FORMAT_IND16:
		{
			bitmap_ind16 &srcbitmap = per_scanline ? *(bitmap_ind16 *)m_scan_bitmaps[m_curbitmap][y] : curbitmap.as_ind16();
			const u16 src = per_scanline ? srcbitmap.pix(0, x) : srcbitmap.pix(y, x);
			const rgb_t *palette = m_palette->palette()->entry_list_adjusted();
			return (u32)palette[src];
		}

		case BITMAP_FORMAT_RGB32:
		{
			if (per_scanline)
			{
				return (u32)(*(bitmap_rgb32 *)m_scan_bitmaps[m_curbitmap][y]).pix(0, x);
			}
			else
			{
				return (u32)curbitmap.as_rgb32().pix(y, x);
			}
		}

		default:
			return 0;
	}
}


//-------------------------------------------------
//  pixels - fills the specified buffer with the
//  RGB values of each pixel in the screen.
//-------------------------------------------------

void screen_device::pixels(u32 *buffer)
{
	screen_bitmap &curbitmap = m_bitmap[m_curbitmap];
	if (!curbitmap.valid())
		return;

	const rectangle &visarea = visible_area();

	const bool per_scanline = (m_video_attributes & VIDEO_VARIABLE_WIDTH);

	switch (curbitmap.format())
	{
		case BITMAP_FORMAT_IND16:
		{
			const rgb_t *palette = m_palette->palette()->entry_list_adjusted();
			for (int y = visarea.min_y; y <= visarea.max_y; y++)
			{
				bitmap_ind16 &srcbitmap = per_scanline ? *(bitmap_ind16 *)m_scan_bitmaps[m_curbitmap][y] : curbitmap.as_ind16();
				const u16 *src = &srcbitmap.pix(per_scanline ? 0 : y, visarea.min_x);
				for (int x = visarea.min_x; x <= visarea.max_x; x++)
				{
					*buffer++ = palette[*src++];
				}
			}
			break;
		}

		case BITMAP_FORMAT_RGB32:
		{
			for (int y = visarea.min_y; y <= visarea.max_y; y++)
			{
				bitmap_rgb32 &srcbitmap = per_scanline ? *(bitmap_rgb32 *)m_scan_bitmaps[m_curbitmap][y] : curbitmap.as_rgb32();
				const u32 *src = &srcbitmap.pix(per_scanline ? 0 : y, visarea.min_x);
				for (int x = visarea.min_x; x <= visarea.max_x; x++)
				{
					*buffer++ = *src++;
				}
			}
			break;
		}

		default:
			break;
	}
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
	return (m_visarea.bottom() + 1 + vpos) % m_height;
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
//  vpos,hpos
//-------------------------------------------------

attotime screen_device::time_until_pos(int vpos, int hpos) const
{
	// validate arguments
	assert(vpos >= 0);
	assert(hpos >= 0);

	// since we measure time relative to VBLANK, compute the scanline offset from VBLANK
	vpos += m_height - (m_visarea.bottom() + 1);
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

	// do nothing if we already have this callback registered
	for (auto &item : m_callback_list)
		if (item->m_callback == vblank_callback)
			return;

	// if not found, register
	m_callback_list.push_back(std::make_unique<callback_item>(vblank_callback));
}


//-------------------------------------------------
//  register_screen_bitmap - registers a bitmap
//  that should track the screen size
//-------------------------------------------------

void screen_device::register_screen_bitmap(bitmap_t &bitmap)
{
	// append to the list
	m_auto_bitmap_list.push_back(std::make_unique<auto_bitmap_item>(bitmap));

	// if allocating now, just do it
	bitmap.allocate(width(), height());
	if (m_palette)
		bitmap.set_palette(m_palette->palette());
}


//-------------------------------------------------
//  vblank_begin - call any external callbacks to
//  signal the VBLANK period has begun
//-------------------------------------------------

TIMER_CALLBACK_MEMBER(screen_device::vblank_begin)
{
	// reset the starting VBLANK time
	m_vblank_start_time = machine().time();
	m_vblank_end_time = m_vblank_start_time + attotime(0, m_vblank_period);

	// if this is the primary screen and we need to update now
	if (!(m_video_attributes & VIDEO_UPDATE_AFTER_VBLANK))
		update_if_primary();

	// call the screen specific callbacks
	for (auto &item : m_callback_list)
		item->m_callback(*this, true);
	m_screen_vblank(1);

	// reset the VBLANK start timer for the next frame
	m_vblank_begin_timer->adjust(time_until_vblank_start());

	// if no VBLANK period, call the VBLANK end callback immediately, otherwise reset the timer
	if (m_vblank_period == 0)
		vblank_end(0);
	else
		m_vblank_end_timer->adjust(time_until_vblank_end());
}


//-------------------------------------------------
//  vblank_end - call any external callbacks to
//  signal the VBLANK period has ended
//-------------------------------------------------

TIMER_CALLBACK_MEMBER(screen_device::vblank_end)
{
	// call the screen specific callbacks
	for (auto &item : m_callback_list)
		item->m_callback(*this, false);
	m_screen_vblank(0);

	// if this is the primary screen and we need to update now
	if (m_video_attributes & VIDEO_UPDATE_AFTER_VBLANK)
		update_if_primary();

	// increment the frame number counter
	m_frame_number++;
}


//-------------------------------------------------
//  create_composited_bitmap - composite scanline
//  bitmaps into the output bitmap
//-------------------------------------------------

void screen_device::create_composited_bitmap()
{
	screen_bitmap &curbitmap = m_bitmap[m_curtexture];
	if (!curbitmap.valid())
		return;

	s32 dstwidth = std::max(m_max_width, m_visarea.right() + 1);
	int dstheight = curbitmap.height();

	switch (curbitmap.format())
	{
		default:
		case BITMAP_FORMAT_IND16:
		{
			for (int y = 0; y < dstheight; y++)
			{
				const bitmap_ind16 &srcbitmap = *(bitmap_ind16 *)m_scan_bitmaps[m_curbitmap][y];
				u16 *dst = &curbitmap.as_ind16().pix(y);
				const u16 *src = &srcbitmap.pix(0);
				const int dx = (m_scan_widths[y] << 15) / dstwidth;
				for (int x = 0; x < m_scan_widths[y]; x += dx)
				{
					*dst++ = src[x >> 15];
				}
			}
			break;
		}

		case BITMAP_FORMAT_RGB32:
		{
			for (int y = 0; y < dstheight; y++)
			{
				const bitmap_rgb32 &srcbitmap = *(bitmap_rgb32 *)m_scan_bitmaps[m_curbitmap][y];
				u32 *dst = &curbitmap.as_rgb32().pix(y);
				const u32 *src = &srcbitmap.pix(0);
				const int dx = (m_scan_widths[y] << 15) / dstwidth;
				for (int x = 0; x < dstwidth << 15; x += dx)
				{
					*dst++ = src[x >> 15];
				}
			}
			break;
		}
	}
}


//-------------------------------------------------
//  video_output_update - set up the quads for this
//  screen
//-------------------------------------------------

bool screen_device::video_output_update()
{
	// only update if live
	if (machine().render().is_live(*this))
	{
		// only update if empty; otherwise assume the driver did it directly
		if ((m_video_attributes & VIDEO_SELF_RENDER) == 0)
		{
			// if we're not skipping the frame and if the screen actually changed, then update the texture
			if (!machine().video().skip_this_frame() && m_changed)
			{
				if (m_video_attributes & VIDEO_VARIABLE_WIDTH)
				{
					create_composited_bitmap();
				}
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
// TODO: other than being unnecessary, this is a simplification of how analog signals really works!
// It's important not to use machine().rand() here, it can cause machine().rand() used in emulation to desync.
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
	int xstart = (u32(rand()) % 32767) * xstep / 32767;
	int ystart = (u32(rand()) % 32767) * ystep / 32767;

	bool per_scanline = (m_video_attributes & VIDEO_VARIABLE_WIDTH);

	switch (curbitmap.format())
	{
		default:
		case BITMAP_FORMAT_IND16:
		{
			// iterate over rows in the destination
			for (int y = 0, srcy = ystart; y < dstheight; y++, srcy += ystep)
			{
				const bitmap_ind16 &srcbitmap = per_scanline ? *(bitmap_ind16 *)m_scan_bitmaps[m_curbitmap][y] : curbitmap.as_ind16();
				u64 *const dst = &m_burnin.pix(y);
				u16 const *const src = &srcbitmap.pix(per_scanline ? 0 : (srcy >> 16));
				rgb_t const *const palette = m_palette->palette()->entry_list_adjusted();
				for (int x = 0, srcx = xstart; x < dstwidth; x++, srcx += xstep)
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
			for (int y = 0, srcy = ystart; y < dstheight; y++, srcy += ystep)
			{
				const bitmap_rgb32 &srcbitmap = per_scanline ? *(bitmap_rgb32 *)m_scan_bitmaps[m_curbitmap][y] : curbitmap.as_rgb32();
				u64 *const dst = &m_burnin.pix(y);
				u32 const *const src = &srcbitmap.pix(per_scanline ? 0 : (srcy >> 16));
				for (int x = 0, srcx = xstart; x < dstwidth; x++, srcx += xstep)
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
	rectangle scaledvis(
			m_visarea.left() * m_burnin.width() / m_width,
			m_visarea.right() * m_burnin.width() / m_width,
			m_visarea.top() * m_burnin.height() / m_height,
			m_visarea.bottom() * m_burnin.height() / m_height);

	// wrap a bitmap around the subregion we care about
	bitmap_argb32 finalmap(scaledvis.width(), scaledvis.height());
	int srcwidth = m_burnin.width();
	int srcheight = m_burnin.height();
	int dstwidth = finalmap.width();
	int dstheight = finalmap.height();
	int xstep = (srcwidth << 16) / dstwidth;
	int ystep = (srcheight << 16) / dstheight;

	// find the maximum value
	u64 minval = ~u64(0);
	u64 maxval = 0;
	for (int y = 0; y < srcheight; y++)
	{
		u64 const *const src = &m_burnin.pix(y);
		for (int x = 0; x < srcwidth; x++)
		{
			minval = std::min(minval, src[x]);
			maxval = std::max(maxval, src[x]);
		}
	}

	if (minval == maxval)
		return;

	// now normalize and convert to RGB
	for (int y = 0, srcy = 0; y < dstheight; y++, srcy += ystep)
	{
		u64 const *const src = &m_burnin.pix(srcy >> 16);
		u32 *const dst = &finalmap.pix(y);
		for (int x = 0, srcx = 0; x < dstwidth; x++, srcx += xstep)
		{
			int brightness = u64(maxval - src[srcx >> 16]) * 255 / (maxval - minval);
			dst[x] = rgb_t(0xff, brightness, brightness, brightness);
		}
	}

	// write the final PNG

	// compute the name and create the file
	emu_file file(machine().options().snapshot_directory(), OPEN_FLAG_WRITE | OPEN_FLAG_CREATE | OPEN_FLAG_CREATE_PATHS);
	std::error_condition const filerr = file.open(util::string_format("%s" PATH_SEPARATOR "burnin-%s.png", machine().basename(), tag() + 1));
	if (!filerr)
	{
		util::png_info pnginfo;

		// add two text entries describing the image
		pnginfo.add_text("Software", util::string_format("%s %s", emulator_info::get_appname(), emulator_info::get_build_version()));
		pnginfo.add_text("System", util::string_format("%s %s", machine().system().manufacturer, machine().system().type.fullname()));

		// now do the actual work
		util::png_write_bitmap(file, &pnginfo, finalmap, 0, nullptr);
	}
}


//-------------------------------------------------
//  load_effect_overlay -
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
	m_screen_overlay_bitmap.reset();
	emu_file file(machine().options().art_path(), OPEN_FLAG_READ);
	if (!file.open(fullname))
	{
		render_load_png(m_screen_overlay_bitmap, file);
		file.close();
	}
	if (m_screen_overlay_bitmap.valid())
		m_container->set_overlay(&m_screen_overlay_bitmap);
	else
		osd_printf_warning("Unable to load effect PNG file '%s'\n", fullname);
}
