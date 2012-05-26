/***************************************************************************

    screen.c

    Core MAME screen device.

****************************************************************************

    Copyright Aaron Giles
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are
    met:

        * Redistributions of source code must retain the above copyright
          notice, this list of conditions and the following disclaimer.
        * Redistributions in binary form must reproduce the above copyright
          notice, this list of conditions and the following disclaimer in
          the documentation and/or other materials provided with the
          distribution.
        * Neither the name 'MAME' nor the names of its contributors may be
          used to endorse or promote products derived from this software
          without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY AARON GILES ''AS IS'' AND ANY EXPRESS OR
    IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL AARON GILES BE LIABLE FOR ANY DIRECT,
    INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
    SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
    HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
    STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
    IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
    POSSIBILITY OF SUCH DAMAGE.

***************************************************************************/

#include "emu.h"
#include "emuopts.h"
#include "png.h"
#include "rendutil.h"



//**************************************************************************
//  DEBUGGING
//**************************************************************************

#define VERBOSE						(0)
#define LOG_PARTIAL_UPDATES(x)		do { if (VERBOSE) logerror x; } while (0)

//#define USE_MONITOR					(1)


//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

// device type definition
const device_type SCREEN = &device_creator<screen_device>;

const attotime screen_device::DEFAULT_FRAME_PERIOD(attotime::from_hz(DEFAULT_FRAME_RATE));



//**************************************************************************
//  SCREEN DEVICE
//**************************************************************************

//-------------------------------------------------
//  screen_device - constructor
//-------------------------------------------------

screen_device::screen_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, SCREEN, "Video Screen", tag, owner, clock),
	  m_type(SCREEN_TYPE_RASTER),
	  m_oldstyle_vblank_supplied(false),
	  m_refresh(0),
	  m_vblank(0),
	  m_xoffset(0.0f),
	  m_yoffset(0.0f),
	  m_xscale(1.0f),
	  m_yscale(1.0f),
	  m_container(NULL),
	  m_width(100),
	  m_height(100),
	  m_visarea(0, 99, 0, 99),
	  m_curbitmap(0),
	  m_curtexture(0),
	  m_changed(true),
	  m_last_partial_scan(0),
	  m_frame_period(DEFAULT_FRAME_PERIOD.as_attoseconds()),
	  m_scantime(1),
	  m_pixeltime(1),
	  m_vblank_period(0),
	  m_vblank_start_time(attotime::zero),
	  m_vblank_end_time(attotime::zero),
	  m_vblank_begin_timer(NULL),
	  m_vblank_end_timer(NULL),
	  m_scanline0_timer(NULL),
	  m_scanline_timer(NULL),
	  m_frame_number(0),
	  m_partial_updates_this_frame(0)
{
	memset(m_texture, 0, sizeof(m_texture));

	m_render_width = m_width;
	m_render_height = m_height;

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
//  static_set_screen_update - set the legacy
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
//  device_validity_check - verify device
//  configuration
//-------------------------------------------------

void screen_device::device_validity_check(validity_checker &valid) const
{
	// sanity check dimensions
	if (m_width <= 0 || m_height <= 0)
		mame_printf_error("Invalid display dimensions\n");

	// sanity check display area
	if (m_type != SCREEN_TYPE_VECTOR)
	{
		if (m_visarea.empty() || m_visarea.max_x >= m_width || m_visarea.max_y >= m_height)
			mame_printf_error("Invalid display area\n");

		// sanity check screen formats
		if (m_screen_update_ind16.isnull() && m_screen_update_rgb32.isnull())
			mame_printf_error("Missing SCREEN_UPDATE function\n");
	}

	// check for zero frame rate
	if (m_refresh == 0)
		mame_printf_error("Invalid (zero) refresh rate\n");
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

	// configure bitmap formats and allocate screen bitmaps
	texture_format texformat = !m_screen_update_ind16.isnull() ? TEXFORMAT_PALETTE16 : TEXFORMAT_RGB32;
	for (int index = 0; index < ARRAY_LENGTH(m_bitmap); index++)
	{
		m_bitmap[index].set_format(format(), texformat);
		register_screen_bitmap(m_bitmap[index]);
	}

	// allocate raw textures
	m_texture[0] = machine().render().texture_alloc();
	m_texture[1] = machine().render().texture_alloc();

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
	if ((machine().config().m_video_attributes & VIDEO_UPDATE_SCANLINE) != 0)
		m_scanline_timer = timer_alloc(TID_SCANLINE);

	// configure the screen with the default parameters
	configure(m_width, m_height, m_visarea, m_refresh);

	// reset VBLANK timing
	m_vblank_start_time = attotime::zero;
	m_vblank_end_time = attotime(0, m_vblank_period);

	// start the timer to generate per-scanline updates
	if ((machine().config().m_video_attributes & VIDEO_UPDATE_SCANLINE) != 0)
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
	if (overname != NULL && strcmp(overname, "none") != 0)
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
	save_item(NAME(m_scantime));
	save_item(NAME(m_pixeltime));
	save_item(NAME(m_vblank_period));
	save_item(NAME(m_vblank_start_time));
	save_item(NAME(m_vblank_end_time));
	save_item(NAME(m_frame_number));
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

		// first visible scanline
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

	// if there has been no VBLANK time specified in the MACHINE_DRIVER, compute it now
    // from the visible area, otherwise just used the supplied value
	if (m_vblank == 0 && !m_oldstyle_vblank_supplied)
		m_vblank_period = m_scantime * (height - visarea.height());
	else
		m_vblank_period = m_vblank;

	// if we are on scanline 0 already, reset the update timer immediately
	// otherwise, defer until the next scanline 0
	if (vpos() == 0)
		m_scanline0_timer->adjust(attotime::zero);
	else
		m_scanline0_timer->adjust(time_until_pos(0));

	// start the VBLANK timer
	m_vblank_begin_timer->adjust(time_until_vblank_start());

	// adjust speed if necessary
	machine().video().update_refresh_speed();
}

const rectangle &screen_device::visible_area() const
{
	if (0 && m_crt.type() != crt_monitor::PASSTHROUGHX)
		return m_crt.get_visible();
	else
		return m_visarea;
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
	if (beamy == m_visarea.max_y + 1 && beamx == 0)
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
	for (auto_bitmap_item *item = m_auto_bitmap_list.first(); item != NULL; item = item->next())
		item->m_bitmap.resize(effwidth, effheight);

	// set up textures

	if (m_crt.type() != crt_monitor::PASSTHROUGHX)
	{
		m_crt.resize(m_width, m_height);
		rectangle crt_vis = m_crt.get_visible();
		m_texture[0]->set_bitmap(m_crt.final_bitmap(0), crt_vis, TEXFORMAT_RGB32);
		m_texture[1]->set_bitmap(m_crt.final_bitmap(1), crt_vis, TEXFORMAT_RGB32);
	}
	else
	{
	m_texture[0]->set_bitmap(m_bitmap[0], m_visarea, m_bitmap[0].texformat());
	m_texture[1]->set_bitmap(m_bitmap[1], m_visarea, m_bitmap[1].texformat());
	}
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

	LOG_PARTIAL_UPDATES(("Partial: update_partial(%s, %d): ", tag(), scanline));

	// these two checks only apply if we're allowed to skip frames
	if (!(machine().config().m_video_attributes & VIDEO_ALWAYS_UPDATE))
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

	// skip if less than the lowest so far
	if (scanline < m_last_partial_scan)
	{
		LOG_PARTIAL_UPDATES(("skipped because less than previous\n"));
		return FALSE;
	}

	// set the start/end scanlines
	rectangle clip = m_visarea;
	if (m_last_partial_scan > clip.min_y)
		clip.min_y = m_last_partial_scan;
	if (scanline < clip.max_y)
		clip.max_y = scanline;

	// render if necessary
	bool result = false;
	if (clip.min_y <= clip.max_y)
	{
		UINT32 flags = UPDATE_HAS_NOT_CHANGED;

		g_profiler.start(PROFILER_VIDEO);
		LOG_PARTIAL_UPDATES(("updating %d-%d\n", clip.min_y, clip.max_y));

		flags = 0;
		screen_bitmap &curbitmap = m_bitmap[m_curbitmap];
		switch (curbitmap.format())
		{
			default:
			case BITMAP_FORMAT_IND16:	flags = m_screen_update_ind16(*this, curbitmap.as_ind16(), clip);	break;
			case BITMAP_FORMAT_RGB32:	flags = m_screen_update_rgb32(*this, curbitmap.as_rgb32(), clip);	break;
		}

		m_partial_updates_this_frame++;
		g_profiler.stop();

		// if we modified the bitmap, we have to commit
		m_changed |= ~flags & UPDATE_HAS_NOT_CHANGED;
		result = true;
	}

	// remember where we left off
	m_last_partial_scan = scanline + 1;
	return result;
}


//-------------------------------------------------
//  update_now - perform an update from the last
//  beam position up to the current beam position
//-------------------------------------------------

void screen_device::update_now()
{
	int current_vpos = vpos();
	int current_hpos = hpos();

	// since we can currently update only at the scanline
    // level, we are trying to do the right thing by
    // updating including the current scanline, only if the
    // beam is past the halfway point horizontally.
    // If the beam is in the first half of the scanline,
    // we only update up to the previous scanline.
    // This minimizes the number of pixels that might be drawn
    // incorrectly until we support a pixel level granularity
	if (current_hpos < (m_width / 2) && current_vpos > 0)
		current_vpos = current_vpos - 1;

	update_partial(current_vpos);
}


//-------------------------------------------------
//  reset_partial_updates - reset the partial
//  updating state
//-------------------------------------------------

void screen_device::reset_partial_updates()
{
	m_last_partial_scan = 0;
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
	for (item = m_callback_list.first(); item != NULL; item = item->next())
		if (item->m_callback == vblank_callback)
			break;

	// if not found, register
	if (item == NULL)
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
	bitmap.set_palette(machine().palette);
}


//-------------------------------------------------
//  vblank_port_read - custom port handler to
//  return a VBLANK state
//-------------------------------------------------

int screen_device::vblank_port_read()
{
	return vblank() ? -1 : 0;
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
	if (this == machine().primary_screen && !(machine().config().m_video_attributes & VIDEO_UPDATE_AFTER_VBLANK))
		machine().video().frame_update();

	// call the screen specific callbacks
	for (callback_item *item = m_callback_list.first(); item != NULL; item = item->next())
		item->m_callback(*this, true);
	if (!m_screen_vblank.isnull())
		m_screen_vblank(*this, true);

	// reset the VBLANK start timer for the next frame
	m_vblank_begin_timer->adjust(time_until_vblank_start());

	// if no VBLANK period, call the VBLANK end callback immedietely, otherwise reset the timer
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
	for (callback_item *item = m_callback_list.first(); item != NULL; item = item->next())
		item->m_callback(*this, false);
	if (!m_screen_vblank.isnull())
		m_screen_vblank(*this, false);

	// if this is the primary screen and we need to update now
	if (this == machine().primary_screen && (machine().config().m_video_attributes & VIDEO_UPDATE_AFTER_VBLANK))
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
		if (m_type != SCREEN_TYPE_VECTOR && (machine().config().m_video_attributes & VIDEO_SELF_RENDER) == 0)
		{
			if (m_crt.type() != crt_monitor::PASSTHROUGHX)
			{
				m_crt.resize(m_render_width, m_render_height);
				m_crt.process(machine(), m_bitmap[m_curbitmap], m_curbitmap, m_visarea);
			}

			// if we're not skipping the frame and if the screen actually changed, then update the texture
			if (!machine().video().skip_this_frame() && m_changed)
			{
				if (m_crt.type() != crt_monitor::PASSTHROUGHX)
				{
					rectangle fixedvis = m_crt.get_visible();
					m_texture[m_curbitmap]->set_bitmap(m_crt.final_bitmap(m_curbitmap), fixedvis, TEXFORMAT_RGB32);
				}
				else
				{
					m_texture[m_curbitmap]->set_bitmap(m_bitmap[m_curbitmap], m_visarea, m_bitmap[m_curbitmap].texformat());
				}

				m_curtexture = m_curbitmap;
				m_curbitmap = 1 - m_curbitmap;
			}

			// create an empty container with a single quad
			m_container->empty();
			m_container->add_quad(0.0f, 0.0f, 1.0f, 1.0f, MAKE_ARGB(0xff,0xff,0xff,0xff), m_texture[m_curtexture], PRIMFLAG_BLENDMODE(BLENDMODE_NONE) | PRIMFLAG_SCREENTEX(1));
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
				const rgb_t *palette = palette_entry_list_adjusted(machine().palette);
				for (x = 0, srcx = xstart; x < dstwidth; x++, srcx += xstep)
				{
					rgb_t pixel = palette[src[srcx >> 16]];
					dst[x] += RGB_GREEN(pixel) + RGB_RED(pixel) + RGB_BLUE(pixel);
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
					dst[x] += RGB_GREEN(pixel) + RGB_RED(pixel) + RGB_BLUE(pixel);
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
			dst[x] = MAKE_ARGB(0xff, brightness, brightness, brightness);
		}
	}

	// write the final PNG

	// compute the name and create the file
	emu_file file(machine().options().snapshot_directory(), OPEN_FLAG_WRITE | OPEN_FLAG_CREATE | OPEN_FLAG_CREATE_PATHS);
	file_error filerr = file.open(machine().basename(), PATH_SEPARATOR "burnin-", tag(), ".png") ;
	if (filerr == FILERR_NONE)
	{
		png_info pnginfo = { 0 };
//      png_error pngerr;
		char text[256];

		// add two text entries describing the image
		sprintf(text,"%s %s", emulator_info::get_appname(), build_version);
		png_add_text(&pnginfo, "Software", text);
		sprintf(text, "%s %s", machine().system().manufacturer, machine().system().description);
		png_add_text(&pnginfo, "System", text);

		// now do the actual work
		png_write_bitmap(file, &pnginfo, finalmap, 0, NULL);

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
	astring fullname(filename);
	int extension = fullname.rchr(0, '.');
	if (extension != -1)
		fullname.del(extension, -1);
	fullname.cat(".png");

	// load the file
	emu_file file(machine().options().art_path(), OPEN_FLAG_READ);
	render_load_png(m_screen_overlay_bitmap, file, NULL, fullname);
	if (m_screen_overlay_bitmap.valid())
		m_container->set_overlay(&m_screen_overlay_bitmap);
	else
		mame_printf_warning("Unable to load effect PNG file '%s'\n", fullname.cstr());
}

void screen_device::set_render_size(int width, int height)
{
	if (width<=4096 && height <=4096)
	{
		m_render_width = width;
		m_render_height = height;
	}
}

//-------------------------------------------------
//  The 3x3 matrix class
//-------------------------------------------------

class vec3d
{
	friend class mat33d;
public:
	vec3d()
	{
	}
	vec3d(double a0, double a1, double a2)
	{
		a[0] = a0;
		a[1] = a1;
		a[2] = a2;
	}
	void set(double a0, double a1, double a2)
	{
		a[0] = a0;
		a[1] = a1;
		a[2] = a2;
	}
	double elem(int row)
	{
		return a[row];
	}

	void dump(void)
	{
		printf("%10.4f %10.4f %10.4f\n", a[0], a[1], a[2]);
	}
protected:
	double a[3];
};

class mat33d
{
public:

	void setConst(double val)
	{
		for (int i=0;i<3;i++)
			for (int j=0;j<3; j++)
				a[i][j] = val;
	}

	void multScalar(double val)
	{
		for (int i=0;i<3;i++)
			for (int j=0;j<3; j++)
				a[i][j] = a[i][j]*val;
	}

	void copyFrom(mat33d &src)
	{
		for (int i=0;i<3;i++)
			for (int j=0;j<3; j++)
				a[i][j] = src.a[i][j];
	}

	double det(void)
	{
		// a11(a33a22-a32a23)-a21(a33a12-a32a13)+a31(a23a12-a22a13)
		return    a[0][0] * (a[2][2]*a[1][1] - a[2][1]*a[1][2])
				- a[1][0] * (a[2][2]*a[0][1] - a[2][1]*a[0][2])
				+ a[2][0] * (a[1][2]*a[0][1] - a[1][1]*a[0][2]);
	}

	void invert(void)
	{
		mat33d ret;
		double temdet = det();

		//| a11 a12 a13 |-1             |   a33a22-a32a23  -(a33a12-a32a13)   a23a12-a22a13  |
		//| a21 a22 a23 |    =  1/DET * | -(a33a21-a31a23)   a33a11-a31a13  -(a23a11-a21a13) |
		//| a31 a32 a33 |               |   a32a21-a31a22  -(a32a11-a31a12)   a22a11-a21a12  |


		ret.a[0][0] =   a[2][2]*a[1][1]-a[2][1]*a[1][2];
		ret.a[0][1] = -(a[2][2]*a[0][1]-a[2][1]*a[0][2]);
		ret.a[0][2] =   a[1][2]*a[0][1]-a[1][1]*a[0][2];
		ret.a[1][0] = -(a[2][2]*a[1][0]-a[2][0]*a[1][2]);
		ret.a[1][1] =   a[2][2]*a[0][0]-a[2][0]*a[0][2];
		ret.a[1][2] = -(a[1][2]*a[0][0]-a[1][0]*a[0][2]);
		ret.a[2][0] =   a[2][1]*a[1][0]-a[2][0]*a[1][1];
		ret.a[2][1] = -(a[2][1]*a[0][0]-a[2][0]*a[0][1]);
		ret.a[2][2] =   a[1][1]*a[0][0]-a[1][0]*a[0][1];
		copyFrom(ret);
		multScalar(1.0f / temdet);
	}

	void multVec(vec3d &r, vec3d &v)
	{
		for (int row = 0; row < 3; row++)
		{
			r.a[row] = a[row][0] * v.a[0] + a[row][1] * v.a[1] + a[row][2] * v.a[2];
		}
	}

	void setRow(int row, double c0, double c1, double c2)
	{
		a[row][0] = c0;
		a[row][1] = c1;
		a[row][2] = c2;
	}

	void setCol(int col, double r0, double r1, double r2)
	{
		a[0][col] = r0;
		a[1][col] = r1;
		a[2][col] = r2;
	}

	double elem(int row, int col)
	{
		return a[row][col];
	}

	mat33d()
	{
		setConst(0.0f);
	}

protected:
	double a[3][3];
};

//-------------------------------------------------
//  The monitor class
//-------------------------------------------------

typedef struct
{
	const char *desc;
	double xr, yr, xg, yg, xb, yb, xw, yw, gamma;
} CIE_Chroma_t;

/*
 * White points below use CIE 1964
 */

static const CIE_Chroma_t CIE_Mons[] =
{
		{ "Guru", 				0.57585, 	0.34435,	0.28773, 	0.55582, 	0.18631,0.10346,	0.3138, 0.3310, 2.2 },	// D65
		//{ "Short-Persistence", 	0.61,	0.35, 	0.29, 	0.59, 	0.15, 	0.063 }, // 	N/A 	N/A 	[Foley96, p. 583]
		//{ "Long-Persistence", 	0.62, 	0.33, 	0.21, 	0.685, 	0.15, 	0.063 }, // 	N/A 	N/A 	[Foley96, p. 583]
		//CCIR 601-1 is the old NTSC standard. Now called Rec 601-1.
		{ "NTSC",			 	0.67, 	0.33, 	0.21, 	0.71, 	0.14, 	0.08, 0.3104, 0.319, 2.2  }, 	// 1ILLUMINANT_C [Walker98]
		{ "EBU",			 	0.64, 	0.33, 	0.30, 	0.60, 	0.15, 	0.06, 0.3138, 0.3310, 2.4 }, 	// D65 [Walker98]
		// (all monitors except 21, " Mitsubishi p/n 65532)
		{ "Dell",				0.625, 	0.340, 	0.275, 	0.605, 	0.150, 	0.065, 	0.280, 	0.315,	2.2 },	// 9300K [Dell, E-mail, 12 Jan 99]
		{ "SMPTE",			 	0.630, 	0.340, 	0.310, 	0.595, 	0.155, 	0.070,	0.3138, 0.3310,	2.2 },	// D65	[Walker98]
		{ "P22 NEC C400",		0.610, 	0.350, 	0.307, 	0.595, 	0.150, 	0.065, 	0.280, 	0.315,	2.2 },	// [NEC98], 9300 K 	Gamma = 2.2
		{ "P22 KDS VS19",		0.625, 	0.340, 	0.285, 	0.605, 	0.150, 	0.065, 	0.281, 	0.311,	2.2 },	//
		{ "High Brightness LED",0.700, 	0.300, 	0.170, 	0.700, 	0.130, 	0.075, 	0.310, 	0.320,	2.2 },	// Nichia Corporation
		{ "sRGB", 				0.64, 	0.33,	0.30, 	0.60, 	0.15,   	0.06,	0.3138, 0.3310, 2.4 },	// D65
		{ "Couriersud",			0.633,	0.336,	0.279,	0.598,	0.149,	0.073,	0.327,	0.355,	2.4 },

		{ NULL }
};

/*
 * Calculate the RGB to CIE matrix ...
 */

static void calc_mat(CIE_Chroma_t cie, mat33d &m)
{
	mat33d ms;
	vec3d vwhite;
	vec3d s;

	ms.setCol(0, cie.xr / cie.yr, 1.0, (1.0f - cie.xr - cie.yr) / cie.yr);
	ms.setCol(1, cie.xg / cie.yg, 1.0, (1.0f - cie.xg - cie.yg) / cie.yg);
	ms.setCol(2, cie.xb / cie.yb, 1.0, (1.0f - cie.xb - cie.yb) / cie.yb);

	ms.invert();

	vwhite.set(cie.xw / cie.yw, 1.0, (1.0f - cie.xw - cie.yw) / cie.yw);
	ms.multVec(s, vwhite);

	m.setCol(0, s.elem(0) * cie.xr / cie.yr, s.elem(0) * 1.0, s.elem(0) * (1.0f - cie.xr - cie.yr) / cie.yr);
	m.setCol(1, s.elem(1) * cie.xg / cie.yg, s.elem(1) * 1.0, s.elem(1) * (1.0f - cie.xg - cie.yg) / cie.yg);
	m.setCol(2, s.elem(2) * cie.xb / cie.yb, s.elem(2) * 1.0, s.elem(2) * (1.0f - cie.xb - cie.yb) / cie.yb);
}

static double src_gamma(double v, double g)
{
	return pow(v, g );
}

static double srgb_gamma(double v)
{
	return v;
	if ( v > 0.0031308 )
		return 1.055 * ( pow(v, 1.0f / 2.4f ) ) - 0.055;
	else
		return 12.92 * v;
}

static rgb_t vec_to_rgb_t(vec3d v)
{
	return MAKE_RGB(
			rgb_clamp((INT32) (srgb_gamma(v.elem(0)) * 255.0f)),
			rgb_clamp((INT32) (srgb_gamma(v.elem(1)) * 255.0f)),
			rgb_clamp((INT32) (srgb_gamma(v.elem(2)) * 255.0f))
			);
}

static void CIE_xy_to_rgbA(CIE_Chroma_t cie, rgb_t *base_r, rgb_t *base_g, rgb_t *base_b)
{
	mat33d rgb_to_cie;
	mat33d cie_to_rgb;
	vec3d tt, tn;

	tt.set(1,1,1);

	calc_mat(cie, rgb_to_cie);
	rgb_to_cie.multVec(tn, tt);
	tn.dump();

	cie_to_rgb.setRow(0,	 3.2406,	-1.5372,	-0.4986);
	cie_to_rgb.setRow(1,	-0.9689,	 1.8758,	 0.0415);
	cie_to_rgb.setRow(2,	 0.0557,	-0.2040,	 1.0570);

	for (int i = 0; i < 768; i++)
	{
		vec3d v_r, v_g, v_b, v_cie, v_rgb;
		double val = src_gamma((double) i / 256.0f, cie.gamma);

		/* desaturate colors for more brightness for val > 1.0
		 * at the expense of accuracy
		 */

		double val2 = (val < 1.0) ? 0 : (val - 1.0) * 0.25;
		v_r.set(val,  val2, val2);
		v_g.set(val2, val,  val2);
		v_b.set(val2, val2, val);

		rgb_to_cie.multVec(v_cie, v_r);
		cie_to_rgb.multVec(v_rgb, v_cie);
		base_r[i] = vec_to_rgb_t(v_rgb);

		if (i == 256)
		{
			printf("val %10.4f", val);
			v_rgb.dump();
		}


		rgb_to_cie.multVec(v_cie, v_g);
		cie_to_rgb.multVec(v_rgb, v_cie);
		base_g[i] = vec_to_rgb_t(v_rgb);
		if (i == 256)
		{
			printf("val %10.4f", val);
			v_rgb.dump();
		}

		rgb_to_cie.multVec(v_cie, v_b);
		cie_to_rgb.multVec(v_rgb, v_cie);
		base_b[i] = vec_to_rgb_t(v_rgb);
		if (i == 256)
		{
			printf("val %10.4f", val);
			v_rgb.dump();
		}

	}
}


void crt_monitor::set_param(crt_monitor_param &param) {
	m_param = param;

	CIE_xy_to_rgbA(CIE_Mons[m_param.source_cie], base_red, base_green, base_blue);

	for (int i=0; i<256; i++)
	{
		m_exp[i] = 255.0f * (1.0f - exp(-(double) i / 255.0f * (double) (m_param.bandwidth * m_param.bandwidth) / 4096.0f));
		//printf("%d %d\n", i, m_exp[i]);
	}
}


crt_monitor::crt_monitor(void)
  : m_amplifier_r(NULL), m_amplifier_g(NULL), m_amplifier_b(NULL)
{
	m_param.r_off = -256 * 3 / 10;
	m_param.g_off = -256 * 3 / 10;
	m_param.b_off = -256 * 3 / 10;
	m_param.r_gain = 256 * 11 / 10;
	m_param.g_gain = 256 * 11 / 10;
	m_param.b_gain = 256 * 11 / 10;
	m_param.bandwidth = 255 * 32 / 100;
	m_param.focus = 255 * 21 / 100 ;
	m_param.decay = 255 * 30 / 100;
	m_param.source_cie = 0;

	//m_param.m_type =  CROMA_CLEAR;
	//m_param.m_type =  SHADOW_MASK;
	m_param.m_type =  PASSTHROUGHX;

	set_param(m_param);
	m_horz_pixel = 0;
	m_vert_pixel = 0;
	resize(288*11/8 * 3, 288*11/8 * 3 / 4 * 3);

}

crt_monitor::~crt_monitor(void)
{
	free_bitmaps();
}

void crt_monitor::free_bitmaps(void)
{
	if (m_amplifier_r != NULL)
		global_free(m_amplifier_r);
	if (m_amplifier_g != NULL)
		global_free(m_amplifier_g);
	if (m_amplifier_b != NULL)
		global_free(m_amplifier_b);
#if 0
	// FIXME: should be done automatically ?
	if (m_mask_bm != NULL)
		bitmap_free(m_mask_bm);
	if (m_phos_bm[0] != NULL)
		bitmap_free(m_phos_bm[0]);
	if (m_phos_bm[1] != NULL)
		bitmap_free(m_phos_bm[1]);
#endif
}

void crt_monitor::resize(int new_width, int new_height)
{
	int scal_width;
	int scal_height;
	int adj = (m_param.m_type == SHADOW_MASK) ? 6 : 0;

	//printf("new %d %d\n", new_width, new_height);

	if (new_width > new_height)
	{
		scal_width = (new_width - adj) / 3;
		scal_height = (new_height - adj) / 3;
	}
	else
	{
		scal_height = (new_width - adj) / 3;
		scal_width = (new_height - adj) / 3;
		int temp = new_height;
		new_height = new_width;
		new_width = temp;
	}


	if (scal_width == m_horz_pixel || scal_width <= 0)
		return;

	int vis_height = 2048; // be safe(scal_height + 3);
	int do_realloc = (scal_width > m_horz_pixel);

	m_horz_pixel = scal_width;
	m_vert_pixel = m_horz_pixel * 3 / 4;


	if (0 && m_param.m_type == SHADOW_MASK)
	{
		m_width = new_width; //m_horz_pixel * 3 + 6;
		m_height = new_height;//m_vert_pixel * 3 + 6;
		m_visible.min_x = 3;
		m_visible.min_y = 3;
		m_visible.max_x = m_width - 4;
		m_visible.max_y = m_height - 4;
	}
	else
	{
		m_width = new_width; //m_horz_pixel * 3 + 6;
		m_height = new_height;//m_vert_pixel * 3 + 6;
		m_visible.min_x = 0;
		m_visible.min_y = 0;
		m_visible.max_x = m_width -1;
		m_visible.max_y = m_height - 1;
	}

	if (0 || do_realloc)
	{
		//free_bitmaps();

		printf("realloc %d %d\n", m_width, m_height);
		m_amplifier_r = global_alloc_array(INT16, m_horz_pixel * vis_height);
		m_amplifier_g = global_alloc_array(INT16, m_horz_pixel * vis_height);
		m_amplifier_b = global_alloc_array(INT16, m_horz_pixel * vis_height);

		m_mask_bm.allocate(m_width, m_height);
		m_phos_bm[0].allocate(m_width, m_height);
		m_phos_bm[1].allocate(m_width, m_height);

#if 0
		bitmap_fill(m_phos_bm[0], NULL, 0);
		bitmap_fill(m_phos_bm[1], NULL, 0);
#endif
	}
}


INLINE rgb_t bpixel(running_machine &machine, bitmap_t &bm, int y, int x)
{
	//return  machine->  pens[*((UINT16 *) bm->base + y * bm->rowpixels + x)];

	if (bm.format() == BITMAP_FORMAT_RGB32)
		return  bm.pixt<rgb_t>(y, x);
	else
		return  palette_get_color(machine, bm.pixt<UINT16>(y, x));

	//return  *((UINT16 *) bm->base + y * bm->rowpixels + x);
}
#if 1

INLINE void set_chan(INT16 *pix, int val, int w)
{
#if 1
	if (val-*pix>0)
		*pix += ((val-*pix)*w)>>8;
#else
	int nv = (val * w) >> 8;
	if (nv > *pix)
		*pix = nv;
#endif

}

void crt_monitor::scale_and_bandwith(running_machine &machine, bitmap_t &bm_src, const rectangle &cliprect)
{
	int y,w,x;

	for (y = cliprect.min_y - 1; y <= cliprect.max_y + 1; y++)
	{
		INT16 *dst_r = m_amplifier_r + (y - cliprect.min_y + 1) * m_horz_pixel;
		INT16 *dst_g = m_amplifier_g + (y - cliprect.min_y + 1) * m_horz_pixel;
		INT16 *dst_b = m_amplifier_b + (y - cliprect.min_y + 1) * m_horz_pixel;
		for (x=0;x<m_horz_pixel;x++)
		{
			dst_r[x] = (dst_r[x] * m_param.decay)>>8;
			dst_g[x] = (dst_g[x] * m_param.decay)>>8;
			dst_b[x] = (dst_b[x] * m_param.decay)>>8;
		}
	}

	w = MAX((cliprect.max_x - cliprect.min_x/* + 1*/), m_horz_pixel);
	int x_add_s = (cliprect.max_x - cliprect.min_x /*+ 1*/) * 16384 / w;
	int x_add_d = 16384 * m_horz_pixel / w;;

	for (y = cliprect.min_y; y <= cliprect.max_y; y++)
	{
		int r = 0;
		int g = 0;
		int b = 0;
		int rn = 0;
		int bn = 0;
		int gn = 0;
		int lx_d = -1;
		int time_r;

		INT16 *dst_r = m_amplifier_r + (y - cliprect.min_y + 1) * m_horz_pixel;
		INT16 *dst_g = m_amplifier_g + (y - cliprect.min_y + 1) * m_horz_pixel;
		INT16 *dst_b = m_amplifier_b + (y - cliprect.min_y + 1) * m_horz_pixel;

		int sx_s = cliprect.min_x * 16384;
		int sx_d = 0;
		time_r = 0;
		for (x=0; x<w; x++)
		{
			rgb_t src = bpixel(machine, bm_src, y,  (sx_s >> 14));
			int x_d = (sx_d >> 14);
			int frac = (sx_s & 16383) >> 6;

			int bandwidth1;
			int bandwidth0;
			if (frac>time_r)
				bandwidth1 = m_exp[(frac-time_r)];
			else
			{
				bandwidth1 = m_exp[(255 - time_r)];
				bandwidth0 = 255- bandwidth1;
				r = (r * bandwidth0 + rn * bandwidth1) >> 8;
				g = (g * bandwidth0 + gn * bandwidth1) >> 8;
				b = (b * bandwidth0 + bn * bandwidth1) >> 8;
				bandwidth1 = m_exp[(frac)];
			}
			time_r = frac;
			rn = ((MAX((int)RGB_RED(src) - m_param.r_off, 0) * m_param.r_gain) >> 8);
			gn = ((MAX((int)RGB_GREEN(src) - m_param.g_off, 0) * m_param.g_gain) >> 8);
			bn = ((MAX((int)RGB_BLUE(src) - m_param.b_off, 0) * m_param.b_gain ) >> 8);
			bandwidth0 = 255- bandwidth1;

			r = (r * bandwidth0 + rn * bandwidth1) >> 8;
			g = (g * bandwidth0 + gn * bandwidth1) >> 8;
			b = (b * bandwidth0 + bn * bandwidth1) >> 8;

			if (x_d>lx_d)
			{
				set_chan(&dst_r[x_d], r, 255);
				set_chan(&dst_g[x_d], g, 255);
				set_chan(&dst_b[x_d], b, 255);

				set_chan(&dst_r[x_d-m_horz_pixel], r, m_param.focus);
				set_chan(&dst_g[x_d-m_horz_pixel], g, m_param.focus);
				set_chan(&dst_b[x_d-m_horz_pixel], b, m_param.focus);

				set_chan(&dst_r[x_d+m_horz_pixel], r, m_param.focus);
				set_chan(&dst_g[x_d+m_horz_pixel], g, m_param.focus);
				set_chan(&dst_b[x_d+m_horz_pixel], b, m_param.focus);
				lx_d = x_d;
			}

			sx_s += x_add_s;
			sx_d += x_add_d;
		}
	}
}
#else
void crt_monitor::scale_and_bandwith(running_machine *machine, bitmap_t *bm_src, const rectangle *cliprect)
{
	int x;
	int y;
	int bandwidth0 = m_param.bandwidth;
	int bandwidth1 = 256 - bandwidth0;
	UINT16 tr[4*1024];
	UINT16 tg[4*1024];
	UINT16 tb[4*1024];

	int x_add = (cliprect.max_x - cliprect.min_x + 1) * 1024 * 4/ (m_horz_pixel);

	for (y = cliprect.min_y; y <= cliprect.max_y; y++)
	{
		int r = 0;
		int g = 0;
		int b = 0;

		/* bandwidth first ! */
		for (x=cliprect.min_x*4; x<=cliprect.max_x*4; x++)
		{
			rgb_t src = bpixel(machine, bm_src, y,  x/4);

			r = (r * bandwidth0 + ((MAX((int)RGB_RED(src) - m_param.r_off, 0) * m_param.r_gain) >> 8) * bandwidth1) >> 8;
			g = (g * bandwidth0 + ((MAX((int)RGB_GREEN(src) - m_param.g_off, 0) * m_param.g_gain) >> 8) * bandwidth1) >> 8;
			b = (b * bandwidth0 + ((MAX((int)RGB_BLUE(src) - m_param.b_off, 0) * m_param.b_gain ) >> 8) * bandwidth1) >> 8;

			tr[x] = r;
			tg[x] = g;
			tb[x] = b;
		}

		int sx = cliprect.min_x * 1024 * 4;

		INT16 *dst_r = m_amplifier_r + (y - cliprect.min_y) * m_horz_pixel;
		INT16 *dst_g = m_amplifier_g + (y - cliprect.min_y) * m_horz_pixel;
		INT16 *dst_b = m_amplifier_b + (y - cliprect.min_y) * m_horz_pixel;

		/* doing a linear interpolation is an approximation,
		 * however should suffice here.
		 */
		for (x = 0; x < m_horz_pixel; x++)
		{
			UINT16 frac = (sx & 0x3ff)>>2;
			UINT16 xw = sx >> 10;

			frac = 0;

			*dst_r++ = (tr[xw] * (256-frac) + tr[xw+1] * frac)>>8;
			*dst_g++ = (tg[xw] * (256-frac) + tg[xw+1] * frac)>>8;
			*dst_b++ = (tb[xw] * (256-frac) + tb[xw+1] * frac)>>8;

			sx += x_add;
		}
	}
}
#endif

void crt_monitor::copyonly(bitmap_rgb32 &bm_dst, rectangle &clip)
{
	int x;
	int y;

	int sy = 0;
	int s_add = (clip.max_y - clip.min_y + 1) * 256 / (m_visible.max_y - m_visible.min_y + 1) ;

	for (y = m_visible.min_y; y <= m_visible.max_y; y++)
	{
		rgb_t *dst = (rgb_t *) bm_dst.raw_pixptr(y, 0);
		for (x = m_visible.min_x; x <= m_visible.max_x; x++)
		{
			int offset = (sy >> 8) * m_horz_pixel + ((x-m_visible.min_x) / 3);
			UINT8 r = MIN(255, m_amplifier_r[offset]);
			UINT8 g = MIN(255, m_amplifier_g[offset]);
			UINT8 b = MIN(255, m_amplifier_b[offset]);
			dst[0] =  MAKE_RGB(r, g, b);
			dst++;
		}
		sy += s_add;
	}
}

INLINE void set_pixel(rgb_t *pix, int r, int g, int b)
{
	int rn = r;
	int gn = g;
	int bn = b;

	*pix = MAKE_RGB(
			(rgb_clamp((int)RGB_RED(*pix) + rn)),
			(rgb_clamp((int)RGB_GREEN(*pix) + gn)),
			(rgb_clamp((int)RGB_BLUE(*pix) + bn))
	);
}

INLINE void set_pixela(rgb_t *pix, rgb_t val, int w)
{
	UINT8 rn = RGB_RED(val);
	UINT8 gn = RGB_GREEN(val);
	UINT8 bn = RGB_BLUE(val);

	UINT8 ro = RGB_RED(*pix);
	UINT8 go = RGB_GREEN(*pix);
	UINT8 bo = RGB_BLUE(*pix);

	if (rn-ro>0)
		ro += ((rn-ro)*w)>>8;
	if (gn-go>0)
		go += ((gn-go)*w)>>8;
	if (bn-bo>0)
		bo += ((bn-bo)*w)>>8;

	*pix = MAKE_RGB(ro, go, bo);
}

void crt_monitor::copy_shadow_mask(bitmap_rgb32 &bm_dst, const rectangle &clip)
{
	int gainv[3] = { 100, 100, 100};
	int sy = 0;
	int s_add = (clip.max_y - clip.min_y + 1) * 16384 / (m_visible.max_y - m_visible.min_y + 1) ;

	int x;
	int y;

	gainv[1] = 255;
	gainv[0] = m_param.focus;
	gainv[2] = m_param.focus;

	for (y = m_visible.min_y+3; y <= m_visible.max_y-4; y++)
	{
		rgb_t *dst0 = (rgb_t *) bm_dst.raw_pixptr(y, m_visible.min_x);
		rgb_t *dst1 = (rgb_t *) bm_dst.raw_pixptr(y + 1, m_visible.min_x);
		int gaina = gainv[y % 3];

		for (x = m_visible.min_x+3; x <= m_visible.max_x-4; x+=3)
		{
			int offset = ((sy + 8191) >> 14) * m_horz_pixel + (((y & 1) == 0) ? (x-m_visible.min_x) / 3 : (2*(x-m_visible.min_x)-3) / 6) ;

			int rn = (m_amplifier_r[offset] * gaina)>>8;
			int gn = (m_amplifier_g[offset] * gaina)>>8;
			int bn = (m_amplifier_b[offset] * gaina)>>8;
#if 0
			int r_ov = MAX(rn - 255, 0);
			int g_ov = MAX(gn - 255, 0);
			int b_ov = MAX(bn - 255, 0);

			if ((y & 1) == 0)
			{
				set_pixel(dst0 - 1, r_ov, 0,    0);
				set_pixel(dst0 + 0, rn,    g_ov, b_ov);
				set_pixel(dst0 + 1, r_ov, gn,    b_ov);
				set_pixel(dst0 + 2, 0,    g_ov, 0);
				set_pixel(dst1 + 0, 0,    0,    b_ov);
				set_pixel(dst1 + 1, r_ov, g_ov, bn);
				set_pixel(dst1 + 2, 0,    0,    b_ov);
			}
			else
			{
				set_pixel(dst0 - 2, r_ov, 0,    0);
				set_pixel(dst0 - 1, rn,    g_ov, b_ov);
				set_pixel(dst0 + 0, r_ov, gn,    b_ov);
				set_pixel(dst0 + 1, 0,    g_ov, 0);
				set_pixel(dst1 - 2, 0,    0,    b_ov);
				set_pixel(dst1 - 1, r_ov, g_ov, bn);
				set_pixel(dst1 - 0, 0,    0,    b_ov);
			}
#else
			if ((y & 1) == 0)
			{
				*(dst0 + 0) = base_red[rn];
				*(dst0 + 1) = base_green[gn];
				*(dst1 + 1) = base_blue[bn];
			}
			else
			{
				*(dst0 - 1) = base_red[rn];
				*(dst0 + 0) = base_green[gn];
				*(dst1 - 1) = base_blue[bn];
			}
#endif
			dst1 += 3;
			dst0 += 3;
		}
		sy += s_add;
	}
}

void crt_monitor::copy_chroma(bitmap_rgb32 &bm_dst, const rectangle &clip)
{
#if 1
	int gainv[4][2] = {
			{ 256 / 4, 256 },
			{ 256,     256 },
			{ 256,     256 / 4 },
			{ 256,     256 }
	};
#else
	int gainv[4][2] = {
			{ m_param.focus, 256 },
			{ m_param.focus,     m_param.focus },
			{ 256,               m_param.focus },
			{ m_param.focus,     m_param.focus }
	};
#endif
	int sy = 0;
	int s_add = (clip.max_y - clip.min_y + 1) * 16384 / (m_visible.max_y - m_visible.min_y + 1) ;

	int x;
	int y;

	//gainv[0][0] = m_param.focus;
	//gainv[1][0] = m_param.focus;

	for (y = m_visible.min_y; y <= m_visible.max_y; y++)
	{
		rgb_t *dst0 = (rgb_t *) bm_dst.raw_pixptr(y, m_visible.min_x);
		rgb_t *dstm1 = dst0 - 3 * bm_dst.rowpixels();
		rgb_t *dstp1 = dst0 + 3 * bm_dst.rowpixels();

		for (x = m_visible.min_x; x <= m_visible.max_x; x+=3)
		{
			int offset = (((sy + 8191) >> 14)+1) * m_horz_pixel + (x-m_visible.min_x) / 3;
			int gaina = gainv[y & 3][((x-m_visible.min_x)/3) & 1];

			int rn = (m_amplifier_r[offset] * gaina)>>8;
			int gn = (m_amplifier_g[offset] * gaina)>>8;
			int bn = (m_amplifier_b[offset] * gaina)>>8;


#if 0
			int blank=0;

			if ((((x / 3) & 1) == 0) && ((y % 4) == 0))
				blank = 1;
			else if ((((x / 3) & 1) == 1) && ((y % 4) == 2))
				blank = 1;

			if (!blank)
			{
#endif
#if 0
				int r_ov = MAX(rn - 255, 0);
				int g_ov = MAX(gn - 255, 0);
				int b_ov = MAX(bn - 255, 0);

				set_pixel(dst0 - 1, r_ov, 0,    0);
				set_pixel(dst0 + 0, rn,	  g_ov,  0);
				set_pixel(dst0 + 1, 0,    gn,	b_ov);
				set_pixel(dst0 + 2, 0, 	  0,    bn);
#elif 0
				int r_ov = MAX(rn - 255, 0);
				int g_ov = MAX(gn - 255, 0);
				int b_ov = MAX(bn - 255, 0);

				set_pixel(dst0 + 0, rn,	  r_ov,  r_ov);
				set_pixel(dst0 + 1, g_ov,    gn,	g_ov);
				set_pixel(dst0 + 2, b_ov, 	  b_ov,    bn);
#elif 0
				int frac1 = (sy & 16383)>>6;
				int frac = (255-frac1);
				int rn1 = (m_amplifier_r[offset+m_horz_pixel] * gaina)>>8;
				int gn1 = (m_amplifier_g[offset+m_horz_pixel] * gaina)>>8;
				int bn1 = (m_amplifier_b[offset+m_horz_pixel] * gaina)>>8;

				*(dst0+0) = base_red[(rn*frac+rn1*frac1)>>8];
				*(dst0+1) = base_green[(gn*frac+gn1*frac1)>>8];
				*(dst0+2) = base_blue[(bn*frac+bn1*frac1)>>8];
#elif 1
				*(dst0+0) = base_red[rn];
				*(dst0+1) = base_green[gn];
				*(dst0+2) = base_blue[bn];
#elif 1
				set_pixela(dst0  , base_red[rn],192);
				set_pixela(dst0+1, base_green[gn],192);
				set_pixela(dst0+2, base_blue[bn],192);
				if (m_param.focus > 0)
				{
					set_pixela(dstm1  , base_red[rn],m_param.focus);
					set_pixela(dstm1+1, base_green[gn],m_param.focus);
					set_pixela(dstm1+2, base_blue[bn],m_param.focus);
					set_pixela(dstp1  , base_red[rn],m_param.focus);
					set_pixela(dstp1+1, base_green[gn],m_param.focus);
					set_pixela(dstp1+2, base_blue[bn],m_param.focus);
#if 0
					set_pixela(dst0-3, base_red[rn],m_param.focus);
					set_pixela(dst0-2, base_green[gn],m_param.focus);
					set_pixela(dst0-1, base_blue[bn],m_param.focus);
					set_pixela(dst0+3, base_red[rn],m_param.focus);
					set_pixela(dst0+4, base_green[gn],m_param.focus);
					set_pixela(dst0+5, base_blue[bn],m_param.focus);
#endif
				}

#else
				/* looks horrible */
				int r = RGB_RED(base_red[rn]) + RGB_RED(base_green[gn]) + RGB_RED(base_blue[bn]);
				int g = RGB_GREEN(base_red[rn]) + RGB_GREEN(base_green[gn]) + RGB_GREEN(base_blue[bn]);
				int b = RGB_BLUE(base_red[rn]) + RGB_BLUE(base_green[gn]) + RGB_BLUE(base_blue[bn]);

				rgb_t c = MAKE_RGB(rgb_clamp(r), rgb_clamp(g), rgb_clamp(b));
				*(dst0+0) = c;
				*(dst0+1) = c;
				*(dst0+2) = c;
#endif
//			}
			dst0 += 3;
			dstm1 += 3;
			dstp1 += 3;
		}
		sy += s_add;
	}
}

void crt_monitor::phosphor(bitmap_rgb32 &bm_dst, bitmap_rgb32 &bm_phos, bitmap_rgb32 &bm_src, const rectangle &cliprect)
{
	const int phos_decay = m_param.decay;

	const int phos_red_decay = phos_decay;
	const int phos_green_decay = phos_decay;
	const int phos_blue_decay = phos_decay;

	//int phos_red_decay1 = 256 - phos_red_decay;
	//int phos_green_decay1 = 256 - phos_green_decay;
	//int phos_blue_decay1 = 256 - phos_blue_decay;

	int x;
	int y;

	for (y = cliprect.min_y; y <= cliprect.max_y; y++)
	{
		rgb_t *src = (rgb_t *) bm_src.raw_pixptr(y, cliprect.min_x);
		rgb_t *sp  = (rgb_t *) bm_phos.raw_pixptr(y, cliprect.min_x);
		rgb_t *dst = (rgb_t *) bm_dst.raw_pixptr(y, cliprect.min_x);

		for (x = cliprect.min_x; x <= cliprect.max_x; x++)
		{
#if 0
			int r = ((int)RGB_RED(*src) * phos_red_decay1 + (int)RGB_RED(*sp) * phos_red_decay) >> 8;
			int g = ((int)RGB_GREEN(*src) * phos_green_decay1 + (int)RGB_GREEN(*sp) * phos_green_decay) >> 8;;
			int b = ((int)RGB_BLUE(*src) * phos_blue_decay1 + (int)RGB_BLUE(*sp) * phos_blue_decay) >> 8;
#else
			int r = ((int)RGB_RED(*sp) * phos_red_decay) >> 8;
			int g = ((int)RGB_GREEN(*sp) * phos_green_decay) >> 8;;
			int b = ((int)RGB_BLUE(*sp) * phos_blue_decay) >> 8;
			int r1 = RGB_RED(*src);
			int g1 = RGB_GREEN(*src);
			int b1 = RGB_BLUE(*src);
			if (r1 > r) r = r1;
			if (g1 > g) g = g1;
			if (b1 > b) b = b1;
#endif
			*dst = MAKE_RGB(r, g, b);

			src++; sp++; dst++;
		}
	}
}

void crt_monitor::process(running_machine &machine, screen_bitmap &src_bm, int cur_bm, rectangle &clip)
{
	bitmap_rgb32 *temp = (0 && m_param.decay > 0) ? &m_mask_bm : &m_phos_bm[cur_bm];

	switch (m_param.m_type)
	{
	case PASSTHROUGHX:
		break;
	case SHADOW_MASK:
		scale_and_bandwith(machine, src_bm, clip);
		temp->fill(0);
	    copy_shadow_mask(*temp, clip );
		break;
	case CROMA_CLEAR:
		scale_and_bandwith(machine, src_bm, clip);
		temp->fill(0);
		copy_chroma(*temp, clip );
		break;
	//case APERTURE_GRILLE:
		//break;
	}
	if (m_param.m_type != PASSTHROUGHX)
	{
		if (0 && m_param.decay > 0)
			phosphor(m_phos_bm[cur_bm],m_phos_bm[1-cur_bm], m_mask_bm, m_visible);
	}
}

int crt_monitor::get_cie_count(void)
{
	int num = 0;
	const CIE_Chroma_t *t = &CIE_Mons[0];

	while (t->desc !=NULL)
	{
		t++;
		num++;
	}
	return num;
}

const char * crt_monitor::get_cie_name(void)
{
	return CIE_Mons[m_param.source_cie].desc;
}
