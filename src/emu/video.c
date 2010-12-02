/***************************************************************************

    video.c

    Core MAME video routines.

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
#include "debugger.h"
#include "debugint/debugint.h"
#include "ui.h"
#include "aviio.h"
#include "crsshair.h"

#include "snap.lh"



//**************************************************************************
//  DEBUGGING
//**************************************************************************

#define LOG_THROTTLE				(0)



//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

// frameskipping tables
const UINT8 video_manager::s_skiptable[FRAMESKIP_LEVELS][FRAMESKIP_LEVELS] =
{
	{ 0,0,0,0,0,0,0,0,0,0,0,0 },
	{ 0,0,0,0,0,0,0,0,0,0,0,1 },
	{ 0,0,0,0,0,1,0,0,0,0,0,1 },
	{ 0,0,0,1,0,0,0,1,0,0,0,1 },
	{ 0,0,1,0,0,1,0,0,1,0,0,1 },
	{ 0,1,0,0,1,0,1,0,0,1,0,1 },
	{ 0,1,0,1,0,1,0,1,0,1,0,1 },
	{ 0,1,0,1,1,0,1,0,1,1,0,1 },
	{ 0,1,1,0,1,1,0,1,1,0,1,1 },
	{ 0,1,1,1,0,1,1,1,0,1,1,1 },
	{ 0,1,1,1,1,1,0,1,1,1,1,1 },
	{ 0,1,1,1,1,1,1,1,1,1,1,1 }
};



//**************************************************************************
//  FUNCTION PROTOTYPES
//**************************************************************************

// software rendering
static void rgb888_draw_primitives(const render_primitive_list &primlist, void *dstdata, UINT32 width, UINT32 height, UINT32 pitch);



//**************************************************************************
//  VIDEO MANAGER
//**************************************************************************

//-------------------------------------------------
//  video_manager - constructor
//-------------------------------------------------

video_manager::video_manager(running_machine &machine)
	: m_machine(machine),
	  m_screenless_frame_timer(NULL),
	  m_throttle_last_ticks(0),
	  m_throttle_realtime(attotime_zero),
	  m_throttle_emutime(attotime_zero),
	  m_throttle_history(0),
	  m_speed_last_realtime(0),
	  m_speed_last_emutime(attotime_zero),
	  m_speed_percent(1.0),
	  m_overall_real_seconds(0),
	  m_overall_real_ticks(0),
	  m_overall_emutime(attotime_zero),
	  m_overall_valid_counter(0),
	  m_throttle(options_get_bool(machine.options(), OPTION_THROTTLE)),
	  m_fastforward(false),
	  m_seconds_to_run(options_get_int(machine.options(), OPTION_SECONDS_TO_RUN)),
	  m_auto_frameskip(options_get_bool(machine.options(), OPTION_AUTOFRAMESKIP)),
	  m_speed(original_speed_setting()),
	  m_empty_skip_count(0),
	  m_frameskip_level(options_get_int(machine.options(), OPTION_FRAMESKIP)),
	  m_frameskip_counter(0),
	  m_frameskip_adjust(0),
	  m_skipping_this_frame(false),
	  m_average_oversleep(0),
	  m_snap_target(NULL),
	  m_snap_bitmap(NULL),
	  m_snap_native(true),
	  m_snap_width(0),
	  m_snap_height(0),
	  m_mngfile(NULL),
	  m_avifile(NULL),
	  m_movie_frame_period(attotime_zero),
	  m_movie_next_frame_time(attotime_zero),
	  m_movie_frame(0)
{
	// request a callback upon exiting
	machine.add_notifier(MACHINE_NOTIFY_EXIT, exit_static);
	state_save_register_postload(&machine, state_postload_stub<video_manager, &video_manager::postload>, this);

	// extract initial execution state from global configuration settings
	update_refresh_speed();

	// create spriteram buffers if necessary
	if (machine.config->m_video_attributes & VIDEO_BUFFERS_SPRITERAM)
		init_buffered_spriteram();

	// create a render target for snapshots
	const char *viewname = options_get_string(machine.options(), OPTION_SNAPVIEW);
	m_snap_native = (machine.primary_screen != NULL && (viewname[0] == 0 || strcmp(viewname, "native") == 0));

	// the native target is hard-coded to our internal layout and has all options disabled
	if (m_snap_native)
	{
		m_snap_target = machine.render().target_alloc(layout_snap, RENDER_CREATE_SINGLE_FILE | RENDER_CREATE_HIDDEN);
		m_snap_target->set_backdrops_enabled(false);
		m_snap_target->set_overlays_enabled(false);
		m_snap_target->set_bezels_enabled(false);
		m_snap_target->set_screen_overlay_enabled(false);
		m_snap_target->set_zoom_to_screen(false);
	}

	// other targets select the specified view and turn off effects
	else
	{
		m_snap_target = machine.render().target_alloc(NULL, RENDER_CREATE_HIDDEN);
		m_snap_target->set_view(m_snap_target->configured_view(viewname, 0, 1));
		m_snap_target->set_screen_overlay_enabled(false);
	}

	// extract snap resolution if present
	if (sscanf(options_get_string(machine.options(), OPTION_SNAPSIZE), "%dx%d", &m_snap_width, &m_snap_height) != 2)
		m_snap_width = m_snap_height = 0;

	// start recording movie if specified
	const char *filename = options_get_string(machine.options(), OPTION_MNGWRITE);
	if (filename[0] != 0)
		begin_recording(filename, MF_MNG);

	filename = options_get_string(machine.options(), OPTION_AVIWRITE);
	if (filename[0] != 0)
		begin_recording(filename, MF_AVI);

	// if no screens, create a periodic timer to drive updates
	if (machine.primary_screen == NULL)
	{
		m_screenless_frame_timer = timer_alloc(&machine, &screenless_update_callback, this);
		timer_adjust_periodic(m_screenless_frame_timer, screen_device::DEFAULT_FRAME_PERIOD, 0, screen_device::DEFAULT_FRAME_PERIOD);
	}
}


//-------------------------------------------------
//  set_frameskip - set the current actual
//  frameskip (-1 means autoframeskip)
//-------------------------------------------------

void video_manager::set_frameskip(int frameskip)
{
	// -1 means autoframeskip
	if (frameskip == -1)
	{
		m_auto_frameskip = true;
		m_frameskip_level = 0;
	}

	// any other level is a direct control
	else if (frameskip >= 0 && frameskip <= MAX_FRAMESKIP)
	{
		m_auto_frameskip = false;
		m_frameskip_level = frameskip;
	}
}


//-------------------------------------------------
//  frame_update - handle frameskipping and UI, 
//  plus updating the screen during normal 
//  operations
//-------------------------------------------------

void video_manager::frame_update(bool debug)
{
	// only render sound and video if we're in the running phase
	int phase = m_machine.phase();
	bool skipped_it = m_skipping_this_frame;
	if (phase == MACHINE_PHASE_RUNNING && (!m_machine.paused() || options_get_bool(m_machine.options(), OPTION_UPDATEINPAUSE)))
	{
		bool anything_changed = finish_screen_updates();

		// if none of the screens changed and we haven't skipped too many frames in a row,
        // mark this frame as skipped to prevent throttling; this helps for games that
        // don't update their screen at the monitor refresh rate
		if (!anything_changed && !m_auto_frameskip && m_frameskip_level == 0 && m_empty_skip_count++ < 3)
			skipped_it = true;
		else
			m_empty_skip_count = 0;
	}

	// draw the user interface
	ui_update_and_render(&m_machine, &m_machine.render().ui_container());

	// update the internal render debugger
	debugint_update_during_game(&m_machine);

	// if we're throttling, synchronize before rendering
	attotime current_time = timer_get_time(&m_machine);
	if (!debug && !skipped_it && effective_throttle())
		update_throttle(current_time);

	// ask the OSD to update
	g_profiler.start(PROFILER_BLIT);
	m_machine.osd().update(!debug && skipped_it);
	g_profiler.stop();

	// perform tasks for this frame
	if (!debug)
		m_machine.call_notifiers(MACHINE_NOTIFY_FRAME);

	// update frameskipping
	if (!debug)
		update_frameskip();

	// update speed computations
	if (!debug && !skipped_it)
		recompute_speed(current_time);

	// call the end-of-frame callback
	if (phase == MACHINE_PHASE_RUNNING)
	{
		// reset partial updates if we're paused or if the debugger is active
		if (m_machine.primary_screen != NULL && (m_machine.paused() || debug || debugger_within_instruction_hook(&m_machine)))
			m_machine.primary_screen->scanline0_callback();

		// otherwise, call the video EOF callback
		else
		{
			g_profiler.start(PROFILER_VIDEO);
			m_machine.driver_data<driver_device>()->video_eof();
			g_profiler.stop();
		}
	}
}


//-------------------------------------------------
//  speed_text - print the text to be displayed 
//  into a string buffer
//-------------------------------------------------

astring &video_manager::speed_text(astring &string)
{
	string.reset();

	// if we're paused, just display Paused
	bool paused = m_machine.paused();
	if (paused)
		string.cat("paused");

	// if we're fast forwarding, just display Fast-forward
	else if (m_fastforward)
		string.cat("fast ");

	// if we're auto frameskipping, display that plus the level
	else if (effective_autoframeskip())
		string.catprintf("auto%2d/%d", effective_frameskip(), MAX_FRAMESKIP);

	// otherwise, just display the frameskip plus the level
	else
		string.catprintf("skip %d/%d", effective_frameskip(), MAX_FRAMESKIP);

	// append the speed for all cases except paused
	if (!paused)
		string.catprintf("%4d%%", (int)(100 * m_speed_percent + 0.5));

	// display the number of partial updates as well
	int partials = 0;
	for (screen_device *screen = m_machine.first_screen(); screen != NULL; screen = screen->next_screen())
		partials += screen->partial_updates();
	if (partials > 1)
		string.catprintf("\n%d partial updates", partials);

	return string;
}


//-------------------------------------------------
//  save_snapshot - save a snapshot to the given
//  file handle
//-------------------------------------------------

void video_manager::save_snapshot(screen_device *screen, mame_file &fp)
{
	// validate
	assert(!m_snap_native || screen != NULL);

	// create the bitmap to pass in
	create_snapshot_bitmap(screen);

	// add two text entries describing the image
	astring text1(APPNAME, " ", build_version);
	astring text2(m_machine.m_game.manufacturer, " ", m_machine.m_game.description);
	png_info pnginfo = { 0 };
	png_add_text(&pnginfo, "Software", text1);
	png_add_text(&pnginfo, "System", text2);

	// now do the actual work
	const rgb_t *palette = (m_machine.palette != NULL) ? palette_entry_list_adjusted(m_machine.palette) : NULL;
	png_error error = png_write_bitmap(mame_core_file(&fp), &pnginfo, m_snap_bitmap, m_machine.total_colors(), palette);
	if (error != PNGERR_NONE)
		mame_printf_error("Error generating PNG for snapshot: png_error = %d\n", error);

	// free any data allocated
	png_free(&pnginfo);
}


//-------------------------------------------------
//  save_active_screen_snapshots - save a
//  snapshot of all active screens
//-------------------------------------------------

void video_manager::save_active_screen_snapshots()
{
	// if we're native, then write one snapshot per visible screen
	if (m_snap_native)
	{
		// write one snapshot per visible screen
		for (screen_device *screen = m_machine.first_screen(); screen != NULL; screen = screen->next_screen())
			if (m_machine.render().is_live(*screen))
			{
				mame_file *fp;
				file_error filerr = mame_fopen_next(SEARCHPATH_SCREENSHOT, "png", fp);
				if (filerr == FILERR_NONE)
				{
					save_snapshot(screen, *fp);
					mame_fclose(fp);
				}
			}
	}

	// otherwise, just write a single snapshot
	else
	{
		mame_file *fp;
		file_error filerr = mame_fopen_next(SEARCHPATH_SCREENSHOT, "png", fp);
		if (filerr == FILERR_NONE)
		{
			save_snapshot(NULL, *fp);
			mame_fclose(fp);
		}
	}
}


//-------------------------------------------------
//  begin_recording - begin recording of a movie
//-------------------------------------------------

void video_manager::begin_recording(const char *name, movie_format format)
{
	// stop any existign recording
	end_recording();

	// create a snapshot bitmap so we know what the target size is
	create_snapshot_bitmap(NULL);

	// reset the state
	m_movie_frame = 0;
	m_movie_next_frame_time = timer_get_time(&m_machine);

	// start up an AVI recording
	if (format == MF_AVI)
	{
		// build up information about this new movie
		avi_movie_info info;
		info.video_format = 0;
		info.video_timescale = 1000 * ((m_machine.primary_screen != NULL) ? ATTOSECONDS_TO_HZ(m_machine.primary_screen->frame_period().attoseconds) : screen_device::DEFAULT_FRAME_RATE);
		info.video_sampletime = 1000;
		info.video_numsamples = 0;
		info.video_width = m_snap_bitmap->width;
		info.video_height = m_snap_bitmap->height;
		info.video_depth = 24;

		info.audio_format = 0;
		info.audio_timescale = m_machine.sample_rate;
		info.audio_sampletime = 1;
		info.audio_numsamples = 0;
		info.audio_channels = 2;
		info.audio_samplebits = 16;
		info.audio_samplerate = m_machine.sample_rate;

		// create a new temporary movie file
		mame_file *tempfile;
		file_error filerr;
		if (name != NULL)
			filerr = mame_fopen(SEARCHPATH_MOVIE, name, OPEN_FLAG_WRITE | OPEN_FLAG_CREATE | OPEN_FLAG_CREATE_PATHS, &tempfile);
		else
			filerr = mame_fopen_next(SEARCHPATH_MOVIE, "avi", tempfile);

		// compute the frame time
		m_movie_frame_period = attotime_div(ATTOTIME_IN_SEC(1000), info.video_timescale);

		// if we succeeded, make a copy of the name and create the real file over top
		if (filerr == FILERR_NONE)
		{
			astring fullname(mame_file_full_name(tempfile));
			mame_fclose(tempfile);

			// create the file and free the string
			avi_error avierr = avi_create(fullname, &info, &m_avifile);
			if (avierr != AVIERR_NONE)
				mame_printf_error("Error creating AVI: %s\n", avi_error_string(avierr));
		}
	}
	
	// start up a MNG recording
	else if (format == MF_MNG)
	{
		// create a new movie file and start recording
		file_error filerr;
		if (name != NULL)
			filerr = mame_fopen(SEARCHPATH_MOVIE, name, OPEN_FLAG_WRITE | OPEN_FLAG_CREATE | OPEN_FLAG_CREATE_PATHS, &m_mngfile);
		else
			filerr = mame_fopen_next(SEARCHPATH_MOVIE, "mng", m_mngfile);

		// start the capture
		int rate = (m_machine.primary_screen != NULL) ? ATTOSECONDS_TO_HZ(m_machine.primary_screen->frame_period().attoseconds) : screen_device::DEFAULT_FRAME_RATE;
		png_error pngerr = mng_capture_start(mame_core_file(m_mngfile), m_snap_bitmap, rate);
		if (pngerr != PNGERR_NONE)
			return end_recording();

		// compute the frame time
		m_movie_frame_period = ATTOTIME_IN_HZ(rate);
	}
}


//-------------------------------------------------
//  end_recording - stop recording of a movie
//-------------------------------------------------

void video_manager::end_recording()
{
	// close the file if it exists
	if (m_avifile != NULL)
	{
		avi_close(m_avifile);
		m_avifile = NULL;
	}

	// close the file if it exists
	if (m_mngfile != NULL)
	{
		mng_capture_stop(mame_core_file(m_mngfile));
		mame_fclose(m_mngfile);
		m_mngfile = NULL;
	}

	// reset the state
	m_movie_frame = 0;
}


//-------------------------------------------------
//  add_sound_to_recording - add sound to a movie
//  recording
//-------------------------------------------------

void video_manager::add_sound_to_recording(const INT16 *sound, int numsamples)
{
	// only record if we have a file
	if (m_avifile != NULL)
	{
		g_profiler.start(PROFILER_MOVIE_REC);

		// write the next frame
		avi_error avierr = avi_append_sound_samples(m_avifile, 0, sound + 0, numsamples, 1);
		if (avierr == AVIERR_NONE)
			avierr = avi_append_sound_samples(m_avifile, 1, sound + 1, numsamples, 1);
		if (avierr != AVIERR_NONE)
			end_recording();

		g_profiler.stop();
	}
}



//-------------------------------------------------
//  init_buffered_spriteram - initialize the
//  double-buffered spriteram
//-------------------------------------------------

void video_manager::init_buffered_spriteram()
{
	assert_always(m_machine.generic.spriteram_size != 0, "Video buffers spriteram but spriteram size is 0");

	// allocate memory for the back buffer
	m_machine.generic.buffered_spriteram.u8 = auto_alloc_array(&m_machine, UINT8, m_machine.generic.spriteram_size);

	// register for saving it
	state_save_register_global_pointer(&m_machine, m_machine.generic.buffered_spriteram.u8, m_machine.generic.spriteram_size);

	// do the same for the second back buffer, if present
	if (m_machine.generic.spriteram2_size)
	{
		// allocate memory
		m_machine.generic.buffered_spriteram2.u8 = auto_alloc_array(&m_machine, UINT8, m_machine.generic.spriteram2_size);

		// register for saving it
		state_save_register_global_pointer(&m_machine, m_machine.generic.buffered_spriteram2.u8, m_machine.generic.spriteram2_size);
	}
}


//-------------------------------------------------
//  video_exit - close down the video system
//-------------------------------------------------

void video_manager::exit_static(running_machine &machine)
{
	machine.video().exit();
}

void video_manager::exit()
{
	// stop recording any movie
	end_recording();

	// free all the graphics elements
	for (int i = 0; i < MAX_GFX_ELEMENTS; i++)
		gfx_element_free(m_machine.gfx[i]);

	// free the snapshot target
	m_machine.render().target_free(m_snap_target);
	if (m_snap_bitmap != NULL)
		global_free(m_snap_bitmap);

	// print a final result if we have at least 5 seconds' worth of data
	if (m_overall_emutime.seconds >= 5)
	{
		osd_ticks_t tps = osd_ticks_per_second();
		double final_real_time = (double)m_overall_real_seconds + (double)m_overall_real_ticks / (double)tps;
		double final_emu_time = attotime_to_double(m_overall_emutime);
		mame_printf_info("Average speed: %.2f%% (%d seconds)\n", 100 * final_emu_time / final_real_time, attotime_add_attoseconds(m_overall_emutime, ATTOSECONDS_PER_SECOND / 2).seconds);
	}
}


//-------------------------------------------------
//  screenless_update_callback - update generator
//  when there are no screens to drive it
//-------------------------------------------------

TIMER_CALLBACK( video_manager::screenless_update_callback )
{
	// force an update
	reinterpret_cast<video_manager *>(ptr)->frame_update(false);
}


//-------------------------------------------------
//  postload - callback for resetting things after
//  state has been loaded
//-------------------------------------------------

void video_manager::postload()
{
	m_movie_next_frame_time = timer_get_time(&m_machine);
}


//-------------------------------------------------
//  effective_autoframeskip - return the effective
//  autoframeskip value, accounting for fast
//  forward
//-------------------------------------------------

inline int video_manager::effective_autoframeskip() const
{
	// if we're fast forwarding or paused, autoframeskip is disabled
	if (m_fastforward || m_machine.paused())
		return false;

	// otherwise, it's up to the user
	return m_auto_frameskip;
}


//-------------------------------------------------
//  effective_frameskip - return the effective
//  frameskip value, accounting for fast
//  forward
//-------------------------------------------------

inline int video_manager::effective_frameskip() const
{
	// if we're fast forwarding, use the maximum frameskip
	if (m_fastforward)
		return FRAMESKIP_LEVELS - 1;

	// otherwise, it's up to the user
	return m_frameskip_level;
}


//-------------------------------------------------
//  effective_throttle - return the effective
//  throttle value, accounting for fast
//  forward and user interface
//-------------------------------------------------

inline bool video_manager::effective_throttle() const
{
	// if we're paused, or if the UI is active, we always throttle
	if (m_machine.paused() || ui_is_menu_active())
		return true;

	// if we're fast forwarding, we don't throttle
	if (m_fastforward)
		return false;

	// otherwise, it's up to the user
	return m_throttle;
}


//-------------------------------------------------
//  original_speed_setting - return the original
//  speed setting
//-------------------------------------------------

inline int video_manager::original_speed_setting() const
{
	return options_get_float(mame_options(), OPTION_SPEED) * 100.0 + 0.5;
}


//-------------------------------------------------
//  finish_screen_updates - finish updating all
//  the screens
//-------------------------------------------------

bool video_manager::finish_screen_updates()
{
	// finish updating the screens
	for (screen_device *screen = m_machine.first_screen(); screen != NULL; screen = screen->next_screen())
		screen->update_partial(screen->visible_area().max_y);

	// now add the quads for all the screens
	bool anything_changed = false;
	for (screen_device *screen = m_machine.first_screen(); screen != NULL; screen = screen->next_screen())
		if (screen->update_quads())
			anything_changed = true;

	// update our movie recording and burn-in state
	if (!m_machine.paused())
	{
		record_frame();

		// iterate over screens and update the burnin for the ones that care
		for (screen_device *screen = m_machine.first_screen(); screen != NULL; screen = screen->next_screen())
			screen->update_burnin();
	}

	// draw any crosshairs
	for (screen_device *screen = m_machine.first_screen(); screen != NULL; screen = screen->next_screen())
		crosshair_render(*screen);

	return anything_changed;
}



//-------------------------------------------------
//  update_throttle - throttle to the game's
//  natural speed
//-------------------------------------------------

void video_manager::update_throttle(attotime emutime)
{
/*

   Throttling theory:

   This routine is called periodically with an up-to-date emulated time.
   The idea is to synchronize real time with emulated time. We do this
   by "throttling", or waiting for real time to catch up with emulated
   time.

   In an ideal world, it will take less real time to emulate and render
   each frame than the emulated time, so we need to slow things down to
   get both times in sync.

   There are many complications to this model:

       * some games run too slow, so each frame we get further and
           further behind real time; our only choice here is to not
           throttle

       * some games have very uneven frame rates; one frame will take
           a long time to emulate, and the next frame may be very fast

       * we run on top of multitasking OSes; sometimes execution time
           is taken away from us, and this means we may not get enough
           time to emulate one frame

       * we may be paused, and emulated time may not be marching
           forward

       * emulated time could jump due to resetting the machine or
           restoring from a saved state

*/
	static const UINT8 popcount[256] =
	{
		0,1,1,2,1,2,2,3, 1,2,2,3,2,3,3,4, 1,2,2,3,2,3,3,4, 2,3,3,4,3,4,4,5,
		1,2,2,3,2,3,3,4, 2,3,3,4,3,4,4,5, 2,3,3,4,3,4,4,5, 3,4,4,5,4,5,5,6,
		1,2,2,3,2,3,3,4, 2,3,3,4,3,4,4,5, 2,3,3,4,3,4,4,5, 3,4,4,5,4,5,5,6,
		2,3,3,4,3,4,4,5, 3,4,4,5,4,5,5,6, 3,4,4,5,4,5,5,6, 4,5,5,6,5,6,6,7,
		1,2,2,3,2,3,3,4, 2,3,3,4,3,4,4,5, 2,3,3,4,3,4,4,5, 3,4,4,5,4,5,5,6,
		2,3,3,4,3,4,4,5, 3,4,4,5,4,5,5,6, 3,4,4,5,4,5,5,6, 4,5,5,6,5,6,6,7,
		2,3,3,4,3,4,4,5, 3,4,4,5,4,5,5,6, 3,4,4,5,4,5,5,6, 4,5,5,6,5,6,6,7,
		3,4,4,5,4,5,5,6, 4,5,5,6,5,6,6,7, 4,5,5,6,5,6,6,7, 5,6,6,7,6,7,7,8
	};

	// outer scope so we can break out in case of a resync
	while (1)
	{
		// apply speed factor to emu time
		if (m_speed != 0 && m_speed != 100)
		{
			// multiply emutime by 100, then divide by the global speed factor
			emutime = attotime_div(attotime_mul(emutime, 100), m_speed);
		}

		// compute conversion factors up front
		osd_ticks_t ticks_per_second = osd_ticks_per_second();
		attoseconds_t attoseconds_per_tick = ATTOSECONDS_PER_SECOND / ticks_per_second;

		// if we're paused, emutime will not advance; instead, we subtract a fixed
	    // amount of time (1/60th of a second) from the emulated time that was passed in,
	    // and explicitly reset our tracked real and emulated timers to that value ...
	    // this means we pretend that the last update was exactly 1/60th of a second
	    // ago, and was in sync in both real and emulated time
		if (m_machine.paused())
		{
			m_throttle_emutime = attotime_sub_attoseconds(emutime, ATTOSECONDS_PER_SECOND / PAUSED_REFRESH_RATE);
			m_throttle_realtime = m_throttle_emutime;
		}

		// attempt to detect anomalies in the emulated time by subtracting the previously
	    // reported value from our current value; this should be a small value somewhere
	    // between 0 and 1/10th of a second ... anything outside of this range is obviously
	    // wrong and requires a resync
		attoseconds_t emu_delta_attoseconds = attotime_to_attoseconds(attotime_sub(emutime, m_throttle_emutime));
		if (emu_delta_attoseconds < 0 || emu_delta_attoseconds > ATTOSECONDS_PER_SECOND / 10)
		{
			if (LOG_THROTTLE)
				logerror("Resync due to weird emutime delta: %s\n", attotime_string(attotime_make(0, emu_delta_attoseconds), 18));
			break;
		}

		// now determine the current real time in OSD-specified ticks; we have to be careful
	    // here because counters can wrap, so we only use the difference between the last
	    // read value and the current value in our computations
		osd_ticks_t diff_ticks = osd_ticks() - m_throttle_last_ticks;
		m_throttle_last_ticks += diff_ticks;

		// if it has been more than a full second of real time since the last call to this
	    // function, we just need to resynchronize
		if (diff_ticks >= ticks_per_second)
		{
			if (LOG_THROTTLE)
				logerror("Resync due to real time advancing by more than 1 second\n");
			break;
		}

		// convert this value into attoseconds for easier comparison
		attoseconds_t real_delta_attoseconds = diff_ticks * attoseconds_per_tick;

		// now update our real and emulated timers with the current values
		m_throttle_emutime = emutime;
		m_throttle_realtime = attotime_add_attoseconds(m_throttle_realtime, real_delta_attoseconds);

		// keep a history of whether or not emulated time beat real time over the last few
	    // updates; this can be used for future heuristics
		m_throttle_history = (m_throttle_history << 1) | (emu_delta_attoseconds > real_delta_attoseconds);

		// determine how far ahead real time is versus emulated time; note that we use the
	    // accumulated times for this instead of the deltas for the current update because
	    // we want to track time over a longer duration than a single update
		attoseconds_t real_is_ahead_attoseconds = attotime_to_attoseconds(attotime_sub(m_throttle_emutime, m_throttle_realtime));

		// if we're more than 1/10th of a second out, or if we are behind at all and emulation
	    // is taking longer than the real frame, we just need to resync
		if (real_is_ahead_attoseconds < -ATTOSECONDS_PER_SECOND / 10 ||
			(real_is_ahead_attoseconds < 0 && popcount[m_throttle_history & 0xff] < 6))
		{
			if (LOG_THROTTLE)
				logerror("Resync due to being behind: %s (history=%08X)\n", attotime_string(attotime_make(0, -real_is_ahead_attoseconds), 18), m_throttle_history);
			break;
		}

		// if we're behind, it's time to just get out
		if (real_is_ahead_attoseconds < 0)
			return;

		// compute the target real time, in ticks, where we want to be
		osd_ticks_t target_ticks = m_throttle_last_ticks + real_is_ahead_attoseconds / attoseconds_per_tick;

		// throttle until we read the target, and update real time to match the final time
		diff_ticks = throttle_until_ticks(target_ticks) - m_throttle_last_ticks;
		m_throttle_last_ticks += diff_ticks;
		m_throttle_realtime = attotime_add_attoseconds(m_throttle_realtime, diff_ticks * attoseconds_per_tick);
		return;
	}

	// reset realtime and emutime to the same value
	m_throttle_realtime = m_throttle_emutime = emutime;
}


//-------------------------------------------------
//  throttle_until_ticks - spin until the
//  specified target time, calling the OSD code
//  to sleep if possible
//-------------------------------------------------

osd_ticks_t video_manager::throttle_until_ticks(osd_ticks_t target_ticks)
{
	// we're allowed to sleep via the OSD code only if we're configured to do so
    // and we're not frameskipping due to autoframeskip, or if we're paused
	bool allowed_to_sleep = false;
    if (options_get_bool(m_machine.options(), OPTION_SLEEP) && (!effective_autoframeskip() || effective_frameskip() == 0))
    	allowed_to_sleep = true;
    if (m_machine.paused())
    	allowed_to_sleep = true;

	// loop until we reach our target
	g_profiler.start(PROFILER_IDLE);
	osd_ticks_t minimum_sleep = osd_ticks_per_second() / 1000;
	osd_ticks_t current_ticks = osd_ticks();
	while (current_ticks < target_ticks)
	{
		// compute how much time to sleep for, taking into account the average oversleep
		osd_ticks_t delta = (target_ticks - current_ticks) * 1000 / (1000 + m_average_oversleep);

		// see if we can sleep
		bool slept = false;
		if (allowed_to_sleep && delta >= minimum_sleep)
		{
			osd_sleep(delta);
			slept = true;
		}

		// read the new value
		osd_ticks_t new_ticks = osd_ticks();

		// keep some metrics on the sleeping patterns of the OSD layer
		if (slept)
		{
			// if we overslept, keep an average of the amount
			osd_ticks_t actual_ticks = new_ticks - current_ticks;
			if (actual_ticks > delta)
			{
				// take 90% of the previous average plus 10% of the new value
				osd_ticks_t oversleep_milliticks = 1000 * (actual_ticks - delta) / delta;
				m_average_oversleep = (m_average_oversleep * 99 + oversleep_milliticks) / 100;

				if (LOG_THROTTLE)
					logerror("Slept for %d ticks, got %d ticks, avgover = %d\n", (int)delta, (int)actual_ticks, (int)m_average_oversleep);
			}
		}
		current_ticks = new_ticks;
	}
	g_profiler.stop();

	return current_ticks;
}


//-------------------------------------------------
//  update_frameskip - update frameskipping
//  counters and periodically update autoframeskip
//-------------------------------------------------

void video_manager::update_frameskip()
{
	// if we're throttling and autoframeskip is on, adjust
	if (effective_throttle() && effective_autoframeskip() && m_frameskip_counter == 0)
	{
		// if we're too fast, attempt to increase the frameskip
		double speed = m_speed * 0.01;
		if (m_speed_percent >= 0.995 * speed)
		{
			// but only after 3 consecutive frames where we are too fast
			if (++m_frameskip_adjust >= 3)
			{
				m_frameskip_adjust = 0;
				if (m_frameskip_level > 0)
					m_frameskip_level--;
			}
		}

		// if we're too slow, attempt to increase the frameskip
		else
		{
			// if below 80% speed, be more aggressive
			if (m_speed_percent < 0.80 *  speed)
				m_frameskip_adjust -= (0.90 * speed - m_speed_percent) / 0.05;

			// if we're close, only force it up to frameskip 8
			else if (m_frameskip_level < 8)
				m_frameskip_adjust--;

			// perform the adjustment
			while (m_frameskip_adjust <= -2)
			{
				m_frameskip_adjust += 2;
				if (m_frameskip_level < MAX_FRAMESKIP)
					m_frameskip_level++;
			}
		}
	}

	// increment the frameskip counter and determine if we will skip the next frame
	m_frameskip_counter = (m_frameskip_counter + 1) % FRAMESKIP_LEVELS;
	m_skipping_this_frame = s_skiptable[effective_frameskip()][m_frameskip_counter];
}


//-------------------------------------------------
//  update_refresh_speed - update the m_speed
//  based on the maximum refresh rate supported
//-------------------------------------------------

void video_manager::update_refresh_speed()
{
	// only do this if the refreshspeed option is used
	if (options_get_bool(m_machine.options(), OPTION_REFRESHSPEED))
	{
		float minrefresh = m_machine.render().max_update_rate();
		if (minrefresh != 0)
		{
			// find the screen with the shortest frame period (max refresh rate)
			// note that we first check the token since this can get called before all screens are created
			attoseconds_t min_frame_period = ATTOSECONDS_PER_SECOND;
			for (screen_device *screen = m_machine.first_screen(); screen != NULL; screen = screen->next_screen())
			{
				attoseconds_t period = screen->frame_period().attoseconds;
				if (period != 0)
					min_frame_period = MIN(min_frame_period, period);
			}

			// compute a target speed as an integral percentage
			// note that we lop 0.25Hz off of the minrefresh when doing the computation to allow for
            // the fact that most refresh rates are not accurate to 10 digits...
			UINT32 target_speed = floor((minrefresh - 0.25f) * 100.0 / ATTOSECONDS_TO_HZ(min_frame_period));
			UINT32 original_speed = original_speed_setting();
			target_speed = MIN(target_speed, original_speed);

			// if we changed, log that verbosely
			if (target_speed != m_speed)
			{
				mame_printf_verbose("Adjusting target speed to %d%% (hw=%.2fHz, game=%.2fHz, adjusted=%.2fHz)\n", target_speed, minrefresh, ATTOSECONDS_TO_HZ(min_frame_period), ATTOSECONDS_TO_HZ(min_frame_period * 100 / target_speed));
				m_speed = target_speed;
			}
		}
	}
}


//-------------------------------------------------
//  recompute_speed - recompute the current
//  overall speed; we assume this is called only
//  if we did not skip a frame
//-------------------------------------------------

void video_manager::recompute_speed(attotime emutime)
{
	// if we don't have a starting time yet, or if we're paused, reset our starting point
	if (m_speed_last_realtime == 0 || m_machine.paused())
	{
		m_speed_last_realtime = osd_ticks();
		m_speed_last_emutime = emutime;
	}

	// if it has been more than the update interval, update the time
	attoseconds_t delta_emutime = attotime_to_attoseconds(attotime_sub(emutime, m_speed_last_emutime));
	if (delta_emutime > SUBSECONDS_PER_SPEED_UPDATE)
	{
		// convert from ticks to attoseconds
		osd_ticks_t realtime = osd_ticks();
		osd_ticks_t delta_realtime = realtime - m_speed_last_realtime;
		osd_ticks_t tps = osd_ticks_per_second();
		m_speed_percent = (double)delta_emutime * (double)tps / ((double)delta_realtime * (double)ATTOSECONDS_PER_SECOND);

		// remember the last times
		m_speed_last_realtime = realtime;
		m_speed_last_emutime = emutime;

		// if we're throttled, this time period counts for overall speed; otherwise, we reset the counter
		if (!m_fastforward)
			m_overall_valid_counter++;
		else
			m_overall_valid_counter = 0;

		// if we've had at least 4 consecutive valid periods, accumulate stats
		if (m_overall_valid_counter >= 4)
		{
			m_overall_real_ticks += delta_realtime;
			while (m_overall_real_ticks >= tps)
			{
				m_overall_real_ticks -= tps;
				m_overall_real_seconds++;
			}
			m_overall_emutime = attotime_add_attoseconds(m_overall_emutime, delta_emutime);
		}
	}

	// if we're past the "time-to-execute" requested, signal an exit
	if (m_seconds_to_run != 0 && emutime.seconds >= m_seconds_to_run)
	{
		if (m_machine.primary_screen != NULL)
		{
			astring fname(m_machine.basename(), PATH_SEPARATOR "final.png");

			// create a final screenshot
			mame_file *file;
			file_error filerr = mame_fopen(SEARCHPATH_SCREENSHOT, fname, OPEN_FLAG_WRITE | OPEN_FLAG_CREATE | OPEN_FLAG_CREATE_PATHS, &file);
			if (filerr == FILERR_NONE)
			{
				save_snapshot(m_machine.primary_screen, *file);
				mame_fclose(file);
			}
		}

		// schedule our demise
		m_machine.schedule_exit();
	}
}


//-------------------------------------------------
//  create_snapshot_bitmap - creates a
//  bitmap containing the screenshot for the
//  given screen
//-------------------------------------------------

void video_manager::create_snapshot_bitmap(device_t *screen)
{
	// select the appropriate view in our dummy target
	if (m_snap_native && screen != NULL)
	{
		int view_index = m_machine.m_devicelist.index(SCREEN, screen->tag());
		assert(view_index != -1);
		m_snap_target->set_view(view_index);
	}

	// get the minimum width/height and set it on the target
	INT32 width = m_snap_width;
	INT32 height = m_snap_height;
	if (width == 0 || height == 0)
		m_snap_target->compute_minimum_size(width, height);
	m_snap_target->set_bounds(width, height);

	// if we don't have a bitmap, or if it's not the right size, allocate a new one
	if (m_snap_bitmap == NULL || width != m_snap_bitmap->width || height != m_snap_bitmap->height)
	{
		if (m_snap_bitmap != NULL)
			auto_free(&m_machine, m_snap_bitmap);
		m_snap_bitmap = auto_alloc(&m_machine, bitmap_t(width, height, BITMAP_FORMAT_RGB32));
	}

	// render the screen there
	render_primitive_list &primlist = m_snap_target->get_primitives();
	primlist.acquire_lock();
	rgb888_draw_primitives(primlist, m_snap_bitmap->base, width, height, m_snap_bitmap->rowpixels);
	primlist.release_lock();
}


//-------------------------------------------------
//  mame_fopen_next - open the next non-existing
//  file of type filetype according to our
//  numbering scheme
//-------------------------------------------------

file_error video_manager::mame_fopen_next(const char *pathoption, const char *extension, mame_file *&file)
{
	// handle defaults
	const char *snapname = options_get_string(m_machine.options(), OPTION_SNAPNAME);
	if (snapname == NULL || snapname[0] == 0)
		snapname = "%g/%i";
	astring snapstr(snapname);

	// strip any extension in the provided name and add our own
	int index = snapstr.rchr(0, '.');
	if (index != -1)
		snapstr.substr(0, index);
	snapstr.cat(".").cat(extension);

	// substitute path and gamename up front
	snapstr.replace(0, "/", PATH_SEPARATOR);
	snapstr.replace(0, "%g", m_machine.basename());

	// determine if the template has an index; if not, we always use the same name
	astring fname;
	if (snapstr.find(0, "%i") == -1)
		fname.cpy(snapstr);

	// otherwise, we scan for the next available filename
	else
	{
		// try until we succeed
		astring seqtext;
		for (int seq = 0; ; seq++)
		{
			// build up the filename
			fname.cpy(snapstr).replace(0, "%i", seqtext.format("%04d", seq).cstr());

			// try to open the file; stop when we fail
			file_error filerr = mame_fopen(pathoption, fname, OPEN_FLAG_READ, &file);
			if (filerr != FILERR_NONE)
				break;
			mame_fclose(file);
		}
	}

	// create the final file
    return mame_fopen(pathoption, fname, OPEN_FLAG_WRITE | OPEN_FLAG_CREATE | OPEN_FLAG_CREATE_PATHS, &file);
}



//-------------------------------------------------
//  record_frame - record a frame of a movie
//-------------------------------------------------

void video_manager::record_frame()
{
	// ignore if nothing to do
	if (m_mngfile == NULL && m_avifile == NULL)
		return;

	// start the profiler and get the current time
	g_profiler.start(PROFILER_MOVIE_REC);
	attotime curtime = timer_get_time(&m_machine);

	// create the bitmap
	create_snapshot_bitmap(NULL);

	// loop until we hit the right time
	while (attotime_compare(m_movie_next_frame_time, curtime) <= 0)
	{
		// handle an AVI recording
		if (m_avifile != NULL)
		{
			// write the next frame
			avi_error avierr = avi_append_video_frame_rgb32(m_avifile, m_snap_bitmap);
			if (avierr != AVIERR_NONE)
			{
				g_profiler.stop();
				return end_recording();
			}
		}

		// handle a MNG recording
		if (m_mngfile != NULL)
		{
			// set up the text fields in the movie info
			png_info pnginfo = { 0 };
			if (m_movie_frame == 0)
			{
				astring text1(APPNAME, " ", build_version);
				astring text2(m_machine.m_game.manufacturer, " ", m_machine.m_game.description);
				png_add_text(&pnginfo, "Software", text1);
				png_add_text(&pnginfo, "System", text2);
			}

			// write the next frame
			const rgb_t *palette = (m_machine.palette != NULL) ? palette_entry_list_adjusted(m_machine.palette) : NULL;
			png_error error = mng_capture_frame(mame_core_file(m_mngfile), &pnginfo, m_snap_bitmap, m_machine.total_colors(), palette);
			png_free(&pnginfo);
			if (error != PNGERR_NONE)
			{
				g_profiler.stop();
				return end_recording();
			}
		}

		// advance time
		m_movie_next_frame_time = attotime_add(m_movie_next_frame_time, m_movie_frame_period);
		m_movie_frame++;
	}
	g_profiler.stop();
}



/*-------------------------------------------------
    video_assert_out_of_range_pixels - assert if
    any pixels in the given bitmap contain an
    invalid palette index
-------------------------------------------------*/

void video_assert_out_of_range_pixels(running_machine *machine, bitmap_t *bitmap)
{
#ifdef MAME_DEBUG
	int maxindex = palette_get_max_index(machine->palette);
	int x, y;

	// this only applies to indexed16 bitmaps
	if (bitmap->format != BITMAP_FORMAT_INDEXED16)
		return;

	// iterate over rows
	for (y = 0; y < bitmap->height; y++)
	{
		UINT16 *rowbase = BITMAP_ADDR16(bitmap, y, 0);
		for (x = 0; x < bitmap->width; x++)
			assert(rowbase[x] < maxindex);
	}
#endif
}



//**************************************************************************
//  SOFTWARE RENDERING
//**************************************************************************

#define FUNC_PREFIX(x)		rgb888_##x
#define PIXEL_TYPE			UINT32
#define SRCSHIFT_R			0
#define SRCSHIFT_G			0
#define SRCSHIFT_B			0
#define DSTSHIFT_R			16
#define DSTSHIFT_G			8
#define DSTSHIFT_B			0
#define BILINEAR_FILTER		1

#include "rendersw.c"
