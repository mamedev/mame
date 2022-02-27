// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    video.cpp

    Core MAME video routines.

***************************************************************************/

#include "emu.h"
#include "emuopts.h"
#include "debugger.h"
#include "fileio.h"
#include "ui/uimain.h"
#include "crsshair.h"
#include "rendersw.hxx"
#include "output.h"
#include "screen.h"

#include "corestr.h"
#include "png.h"
#include "xmlfile.h"

#include "osdepend.h"


//**************************************************************************
//  DEBUGGING
//**************************************************************************

#define LOG_THROTTLE                (0)



//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

// frameskipping tables
const bool video_manager::s_skiptable[FRAMESKIP_LEVELS][FRAMESKIP_LEVELS] =
{
	{ false, false, false, false, false, false, false, false, false, false, false, false },
	{ false, false, false, false, false, false, false, false, false, false, false, true  },
	{ false, false, false, false, false, true , false, false, false, false, false, true  },
	{ false, false, false, true , false, false, false, true , false, false, false, true  },
	{ false, false, true , false, false, true , false, false, true , false, false, true  },
	{ false, true , false, false, true , false, true , false, false, true , false, true  },
	{ false, true , false, true , false, true , false, true , false, true , false, true  },
	{ false, true , false, true , true , false, true , false, true , true , false, true  },
	{ false, true , true , false, true , true , false, true , true , false, true , true  },
	{ false, true , true , true , false, true , true , true , false, true , true , true  },
	{ false, true , true , true , true , true , false, true , true , true , true , true  },
	{ false, true , true , true , true , true , true , true , true , true , true , true  }
};



//**************************************************************************
//  VIDEO MANAGER
//**************************************************************************

static void video_notifier_callback(const char *outname, s32 value, void *param)
{
	video_manager *vm = (video_manager *)param;

	vm->set_output_changed();
}


//-------------------------------------------------
//  video_manager - constructor
//-------------------------------------------------

video_manager::video_manager(running_machine &machine)
	: m_machine(machine)
	, m_screenless_frame_timer(nullptr)
	, m_output_changed(false)
	, m_throttle_last_ticks(0)
	, m_throttle_realtime(attotime::zero)
	, m_throttle_emutime(attotime::zero)
	, m_throttle_history(0)
	, m_speed_last_realtime(0)
	, m_speed_last_emutime(attotime::zero)
	, m_speed_percent(1.0)
	, m_overall_real_seconds(0)
	, m_overall_real_ticks(0)
	, m_overall_emutime(attotime::zero)
	, m_overall_valid_counter(0)
	, m_throttled(true)
	, m_throttle_rate(1.0f)
	, m_fastforward(false)
	, m_seconds_to_run(machine.options().seconds_to_run())
	, m_auto_frameskip(machine.options().auto_frameskip())
	, m_speed(original_speed_setting())
	, m_low_latency(machine.options().low_latency())
	, m_empty_skip_count(0)
	, m_frameskip_max(m_auto_frameskip ? machine.options().frameskip() : 0)
	, m_frameskip_level(m_auto_frameskip ? 0 : machine.options().frameskip())
	, m_frameskip_counter(0)
	, m_frameskip_adjust(0)
	, m_skipping_this_frame(false)
	, m_average_oversleep(0)
	, m_snap_target(nullptr)
	, m_snap_native(true)
	, m_snap_width(0)
	, m_snap_height(0)
{
	// request a callback upon exiting
	machine.add_notifier(MACHINE_NOTIFY_EXIT, machine_notify_delegate(&video_manager::exit, this));
	machine.save().register_postload(save_prepost_delegate(FUNC(video_manager::postload), this));

	// extract initial execution state from global configuration settings
	update_refresh_speed();

	const unsigned screen_count(screen_device_enumerator(machine.root_device()).count());
	const bool no_screens(!screen_count);

	// create a render target for snapshots
	const char *viewname = machine.options().snap_view();
	m_snap_native = !no_screens && !strcmp(viewname, "native");

	if (m_snap_native)
	{
		// the native target is hard-coded to our internal layout and has all options disabled
		util::xml::file::ptr const root(util::xml::file::create());
		if (!root)
			throw emu_fatalerror("Couldn't create XML document??");
		util::xml::data_node *const layoutnode(root->add_child("mamelayout", nullptr));
		if (!layoutnode)
			throw emu_fatalerror("Couldn't create XML node??");
		layoutnode->set_attribute_int("version", 2);

		for (unsigned i = 0; screen_count > i; ++i)
		{
			util::xml::data_node *const viewnode(layoutnode->add_child("view", nullptr));
			if (!viewnode)
				throw emu_fatalerror("Couldn't create XML node??");
			viewnode->set_attribute("name", util::string_format("s%1$u", i).c_str());
			util::xml::data_node *const screennode(viewnode->add_child("screen", nullptr));
			if (!screennode)
				throw emu_fatalerror("Couldn't create XML node??");
			screennode->set_attribute_int("index", i);
			util::xml::data_node *const boundsnode(screennode->add_child("bounds", nullptr));
			if (!boundsnode)
				throw emu_fatalerror("Couldn't create XML node??");
			boundsnode->set_attribute_int("left", 0);
			boundsnode->set_attribute_int("top", 0);
			boundsnode->set_attribute_int("right", 1);
			boundsnode->set_attribute_int("bottom", 1);
		}

		m_snap_target = machine.render().target_alloc(*root, RENDER_CREATE_SINGLE_FILE | RENDER_CREATE_HIDDEN);
		m_snap_target->set_screen_overlay_enabled(false);
		m_snap_target->set_zoom_to_screen(false);
	}
	else
	{
		// otherwise, non-default targets select the specified view and turn off effects
		m_snap_target = machine.render().target_alloc(nullptr, RENDER_CREATE_HIDDEN);
		m_snap_target->set_view(m_snap_target->configured_view(viewname, 0, 1));
		m_snap_target->set_screen_overlay_enabled(false);
	}

	// extract snap resolution if present
	if (sscanf(machine.options().snap_size(), "%dx%d", &m_snap_width, &m_snap_height) != 2)
		m_snap_width = m_snap_height = 0;

	// if no screens, create a periodic timer to drive updates
	if (no_screens)
	{
		m_screenless_frame_timer = machine.scheduler().timer_alloc(timer_expired_delegate(FUNC(video_manager::screenless_update_callback), this));
		m_screenless_frame_timer->adjust(screen_device::DEFAULT_FRAME_PERIOD, 0, screen_device::DEFAULT_FRAME_PERIOD);
		machine.output().set_global_notifier(video_notifier_callback, this);
	}
}


//-------------------------------------------------
//  set_frameskip - set the current actual
//  frameskip (-1 means autoframeskip)
//-------------------------------------------------

void video_manager::set_frameskip(int frameskip)
{
	if (0 > frameskip)
	{
		// -1 means autoframeskip
		if (!m_auto_frameskip)
			m_frameskip_level = 0;
		m_auto_frameskip = true;
	}
	else
	{
		// any other level is a direct control
		m_auto_frameskip = false;
		m_frameskip_level = std::min<int>(frameskip, MAX_FRAMESKIP);
	}
}


//-------------------------------------------------
//  frame_update - handle frameskipping and UI,
//  plus updating the screen during normal
//  operations
//-------------------------------------------------

void video_manager::frame_update(bool from_debugger)
{
	// only render sound and video if we're in the running phase
	machine_phase const phase = machine().phase();
	bool skipped_it = m_skipping_this_frame;
	bool const update_screens = (phase == machine_phase::RUNNING) && (!machine().paused() || machine().options().update_in_pause());
	bool anything_changed = update_screens && finish_screen_updates();

	// draw the user interface
	emulator_info::draw_user_interface(machine());

	// let plugins draw over the UI
	anything_changed = emulator_info::frame_hook() || anything_changed;

	// if none of the screens changed and we haven't skipped too many frames in a row,
	// mark this frame as skipped to prevent throttling; this helps for games that
	// don't update their screen at the monitor refresh rate
	if (!anything_changed && !m_auto_frameskip && (m_frameskip_level == 0) && (m_empty_skip_count++ < 3))
		skipped_it = true;
	else
		m_empty_skip_count = 0;

	// if we're throttling, synchronize before rendering
	attotime current_time = machine().time();
	if (!from_debugger && !skipped_it && phase > machine_phase::INIT && !m_low_latency && effective_throttle())
		update_throttle(current_time);

	// ask the OSD to update
	g_profiler.start(PROFILER_BLIT);
	machine().osd().update(!from_debugger && skipped_it);
	g_profiler.stop();

	// we synchronize after rendering instead of before, if low latency mode is enabled
	if (!from_debugger && !skipped_it && phase > machine_phase::INIT && m_low_latency && effective_throttle())
		update_throttle(current_time);

	// get most recent input now
	machine().osd().input_update();

	emulator_info::periodic_check();

	if (!from_debugger)
	{
		// perform tasks for this frame
		machine().call_notifiers(MACHINE_NOTIFY_FRAME);

		// update frameskipping
		if (phase > machine_phase::INIT)
			update_frameskip();

		// update speed computations
		if (!skipped_it && phase > machine_phase::INIT)
			recompute_speed(current_time);
	}

	// call the end-of-frame callback
	if (phase == machine_phase::RUNNING)
	{
		// reset partial updates if we're paused or if the debugger is active
		screen_device *const screen = screen_device_enumerator(machine().root_device()).first();
		bool const debugger_enabled = machine().debug_flags & DEBUG_FLAG_ENABLED;
		bool const within_instruction_hook = debugger_enabled && machine().debugger().within_instruction_hook();
		if (screen && ((machine().paused() && machine().options().update_in_pause()) || from_debugger || within_instruction_hook))
			screen->reset_partial_updates();
	}
}


//-------------------------------------------------
//  speed_text - print the text to be displayed
//  into a string buffer
//-------------------------------------------------

std::string video_manager::speed_text()
{
	std::ostringstream str;

	// if we're paused, just display Paused
	bool paused = machine().paused();
	if (paused)
		str << "paused";

	// if we're fast forwarding, just display Fast-forward
	else if (m_fastforward)
		str << "fast ";

	// if we're auto frameskipping, display that plus the level
	else if (effective_autoframeskip())
		util::stream_format(str, "auto%2d/%d", effective_frameskip(), m_frameskip_max ? m_frameskip_max : MAX_FRAMESKIP);

	// otherwise, just display the frameskip plus the level
	else
		util::stream_format(str, "skip %d/%d", effective_frameskip(), MAX_FRAMESKIP);

	// append the speed for all cases except paused
	if (!paused)
		util::stream_format(str, "%4d%%", (int)(100 * m_speed_percent + 0.5));

	// display the number of partial updates as well
	int partials = 0;
	for (screen_device &screen : screen_device_enumerator(machine().root_device()))
		partials += screen.partial_updates();
	if (partials > 1)
		util::stream_format(str, "\n%d partial updates", partials);

	return str.str();
}


//-------------------------------------------------
//  save_snapshot - save a snapshot to the given
//  file handle
//-------------------------------------------------

void video_manager::save_snapshot(screen_device *screen, util::core_file &file)
{
	// validate
	assert(!m_snap_native || screen != nullptr);

	// create the bitmap to pass in
	create_snapshot_bitmap(screen);

	// add two text entries describing the image
	std::string text1 = std::string(emulator_info::get_appname()).append(" ").append(emulator_info::get_build_version());
	std::string text2 = std::string(machine().system().manufacturer).append(" ").append(machine().system().type.fullname());
	util::png_info pnginfo;
	pnginfo.add_text("Software", text1);
	pnginfo.add_text("System", text2);

	// now do the actual work
	const rgb_t *palette = (screen != nullptr && screen->has_palette()) ? screen->palette().palette()->entry_list_adjusted() : nullptr;
	int entries = (screen != nullptr && screen->has_palette()) ? screen->palette().entries() : 0;
	std::error_condition const error = util::png_write_bitmap(file, &pnginfo, m_snap_bitmap, entries, palette);
	if (error)
		osd_printf_error("Error generating PNG for snapshot (%s:%d %s)\n", error.category().name(), error.value(), error.message());
}


//-------------------------------------------------
//  save_active_screen_snapshots - save a
//  snapshot of all active screens
//-------------------------------------------------

void video_manager::save_active_screen_snapshots()
{
	if (m_snap_native)
	{
		// if we're native, then write one snapshot per visible screen
		for (screen_device &screen : screen_device_enumerator(machine().root_device()))
			if (machine().render().is_live(screen))
			{
				emu_file file(machine().options().snapshot_directory(), OPEN_FLAG_WRITE | OPEN_FLAG_CREATE | OPEN_FLAG_CREATE_PATHS);
				std::error_condition const filerr = open_next(file, "png");
				if (!filerr)
					save_snapshot(&screen, file);
			}
	}
	else
	{
		// otherwise, just write a single snapshot
		emu_file file(machine().options().snapshot_directory(), OPEN_FLAG_WRITE | OPEN_FLAG_CREATE | OPEN_FLAG_CREATE_PATHS);
		std::error_condition const filerr = open_next(file, "png");
		if (!filerr)
			save_snapshot(nullptr, file);
	}
}


//-------------------------------------------------
//  begin_recording_screen - begin recording a
//  movie for a specific screen
//-------------------------------------------------

void video_manager::begin_recording_screen(const std::string &filename, uint32_t index, screen_device *screen, movie_recording::format format)
{
	// determine the file extension
	const char *extension = movie_recording::format_file_extension(format);

	// create the emu_file
	bool is_absolute_path = !filename.empty() && osd_is_absolute_path(filename);
	std::unique_ptr<emu_file> movie_file = std::make_unique<emu_file>(
			is_absolute_path ? "" : machine().options().snapshot_directory(),
			OPEN_FLAG_WRITE | OPEN_FLAG_CREATE | OPEN_FLAG_CREATE_PATHS);

	// and open the actual file
	std::error_condition filerr = filename.empty()
			? open_next(*movie_file, extension)
			: movie_file->open(filename);
	if (filerr)
	{
		osd_printf_error("Error creating movie, %s:%d %s\n", filerr.category().name(), filerr.value(), filerr.message());
		return;
	}

	// we have a file; try to create the recording
	std::unique_ptr<movie_recording> recording = movie_recording::create(machine(), screen, format, std::move(movie_file), m_snap_bitmap);

	// if successful push it onto the list
	if (recording)
		m_movie_recordings.push_back(std::move(recording));
}


//-------------------------------------------------
//  begin_recording - begin recording of a movie
//-------------------------------------------------

void video_manager::begin_recording(const char *name, movie_recording::format format)
{
	// create a snapshot bitmap so we know what the target size is
	screen_device_enumerator iterator(machine().root_device());
	screen_device_enumerator::iterator iter(iterator.begin());
	uint32_t count = (uint32_t)iterator.count();
	const bool no_screens(!count);

	if (no_screens)
	{
		assert(!m_snap_native);
		count = 1;
	}

	// clear out existing recordings
	m_movie_recordings.clear();

	if (m_snap_native)
	{
		for (uint32_t index = 0; index < count; index++, iter++)
		{
			create_snapshot_bitmap(iter.current());

			std::string tempname;
			if (name)
				tempname = index > 0 ? name : util::string_format("%s%d", name, index);
			begin_recording_screen(
				tempname,
				index,
				iter.current(),
				format);
		}
	}
	else
	{
		create_snapshot_bitmap(nullptr);
		begin_recording_screen(name ? name : "", 0, iter.current(), format);
	}
}


//-------------------------------------------------
//  add_sound_to_recording - add sound to a movie
//  recording
//-------------------------------------------------

void video_manager::add_sound_to_recording(const s16 *sound, int numsamples)
{
	for (auto &recording : m_movie_recordings)
		recording->add_sound_to_recording(sound, numsamples);
}


//-------------------------------------------------
//  video_exit - close down the video system
//-------------------------------------------------

void video_manager::exit()
{
	// stop recording any movie
	m_movie_recordings.clear();

	// free the snapshot target
	machine().render().target_free(m_snap_target);
	m_snap_bitmap.reset();

	// print a final result if we have at least 2 seconds' worth of data
	if (!emulator_info::standalone() && m_overall_emutime.seconds() >= 1)
	{
		osd_ticks_t tps = osd_ticks_per_second();
		double final_real_time = (double)m_overall_real_seconds + (double)m_overall_real_ticks / (double)tps;
		double final_emu_time = m_overall_emutime.as_double();
		osd_printf_info("Average speed: %.2f%% (%d seconds)\n", 100 * final_emu_time / final_real_time, (m_overall_emutime + attotime(0, ATTOSECONDS_PER_SECOND / 2)).seconds());
	}
}


//-------------------------------------------------
//  screenless_update_callback - update generator
//  when there are no screens to drive it
//-------------------------------------------------

void video_manager::screenless_update_callback(int param)
{
	// force an update
	frame_update(false);
}


//-------------------------------------------------
//  postload - callback for resetting things after
//  state has been loaded
//-------------------------------------------------

void video_manager::postload()
{
	attotime const emutime = machine().time();
	for (const auto &x : m_movie_recordings)
		x->set_next_frame_time(emutime);

	// reset speed measurements
	m_speed_last_realtime = osd_ticks();
	m_speed_last_emutime = emutime;
}


//-------------------------------------------------
//  effective_autoframeskip - return the effective
//  autoframeskip value, accounting for fast
//  forward
//-------------------------------------------------

inline bool video_manager::effective_autoframeskip() const
{
	// if we're fast forwarding or paused, autoframeskip is disabled
	if (m_fastforward || machine().paused())
		return false;

	// otherwise, it's up to the user
	return m_auto_frameskip;
}


//-------------------------------------------------
//  effective_frameskip - return the effective
//  frameskip value, accounting for fast
//  forward
//-------------------------------------------------

int video_manager::effective_frameskip() const
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
	if (machine().paused() && !machine().options().update_in_pause()) //|| machine().ui().is_menu_active())
		return true;

	// if we're fast forwarding, we don't throttle
	if (m_fastforward)
		return false;

	// otherwise, it's up to the user
	return throttled();
}


//-------------------------------------------------
//  original_speed_setting - return the original
//  speed setting
//-------------------------------------------------

inline int video_manager::original_speed_setting() const
{
	return machine().options().speed() * 1000.0f + 0.5f;
}


//-------------------------------------------------
//  finish_screen_updates - finish updating all
//  the screens
//-------------------------------------------------

bool video_manager::finish_screen_updates()
{
	// finish updating the screens
	screen_device_enumerator iter(machine().root_device());

	bool has_live_screen = false;
	for (screen_device &screen : iter)
	{
		if (screen.partial_scan_hpos() >= 0) // previous update ended mid-scanline
			screen.update_now();
		screen.update_partial(screen.visible_area().max_y);

		if (machine().render().is_live(screen))
			has_live_screen = true;
	}

	bool anything_changed = !has_live_screen || m_output_changed;
	m_output_changed = false;

	// now add the quads for all the screens
	for (screen_device &screen : iter)
		if (screen.update_quads())
			anything_changed = true;

	// update our movie recording and burn-in state
	if (!machine().paused())
	{
		record_frame();

		// iterate over screens and update the burnin for the ones that care
		for (screen_device &screen : iter)
			screen.update_burnin();
	}

	// draw any crosshairs
	for (screen_device &screen : iter)
		machine().crosshair().render(screen);

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

	// outer scope so we can break out in case of a resync
	while (1)
	{
		// apply speed factor to emu time
		if (m_speed != 0 && m_speed != 1000)
		{
			// multiply emutime by 1000, then divide by the global speed factor
			emutime = (emutime * 1000) / m_speed;
		}

		// compute conversion factors up front
		osd_ticks_t ticks_per_second = osd_ticks_per_second();
		attoseconds_t attoseconds_per_tick = ATTOSECONDS_PER_SECOND / ticks_per_second * m_throttle_rate;

		// if we're paused, emutime will not advance; instead, we subtract a fixed
		// amount of time (1/60th of a second) from the emulated time that was passed in,
		// and explicitly reset our tracked real and emulated timers to that value ...
		// this means we pretend that the last update was exactly 1/60th of a second
		// ago, and was in sync in both real and emulated time
		if (machine().paused())
		{
			m_throttle_emutime = emutime - attotime(0, ATTOSECONDS_PER_SECOND / PAUSED_REFRESH_RATE);
			m_throttle_realtime = m_throttle_emutime;
		}

		// attempt to detect anomalies in the emulated time by subtracting the previously
		// reported value from our current value; this should be a small value somewhere
		// between 0 and 1/10th of a second ... anything outside of this range is obviously
		// wrong and requires a resync
		attoseconds_t emu_delta_attoseconds = (emutime - m_throttle_emutime).as_attoseconds();
		if (emu_delta_attoseconds < 0 || emu_delta_attoseconds > ATTOSECONDS_PER_SECOND / 10)
		{
			if (LOG_THROTTLE)
				machine().logerror("Resync due to weird emutime delta: %s\n", attotime(0, emu_delta_attoseconds).as_string(18));
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
				machine().logerror("Resync due to real time advancing by more than 1 second\n");
			break;
		}

		// convert this value into attoseconds for easier comparison
		attoseconds_t real_delta_attoseconds = diff_ticks * attoseconds_per_tick;

		// now update our real and emulated timers with the current values
		m_throttle_emutime = emutime;
		m_throttle_realtime += attotime(0, real_delta_attoseconds);

		// keep a history of whether or not emulated time beat real time over the last few
		// updates; this can be used for future heuristics
		m_throttle_history = (m_throttle_history << 1) | (emu_delta_attoseconds > real_delta_attoseconds);

		// determine how far ahead real time is versus emulated time; note that we use the
		// accumulated times for this instead of the deltas for the current update because
		// we want to track time over a longer duration than a single update
		attoseconds_t real_is_ahead_attoseconds = (m_throttle_emutime - m_throttle_realtime).as_attoseconds();

		// if we're more than 1/10th of a second out, or if we are behind at all and emulation
		// is taking longer than the real frame, we just need to resync
		if (real_is_ahead_attoseconds < -ATTOSECONDS_PER_SECOND / 10 ||
			(real_is_ahead_attoseconds < 0 && population_count_32(m_throttle_history & 0xff) < 6))
		{
			if (LOG_THROTTLE)
				machine().logerror("Resync due to being behind: %s (history=%08X)\n", attotime(0, -real_is_ahead_attoseconds).as_string(18), m_throttle_history);
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
		m_throttle_realtime += attotime(0, diff_ticks * attoseconds_per_tick);
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
	bool const allowed_to_sleep = (machine().options().sleep() && (!effective_autoframeskip() || effective_frameskip() == 0)) || machine().paused();

	// loop until we reach our target
	g_profiler.start(PROFILER_IDLE);
	osd_ticks_t current_ticks = osd_ticks();
	while (current_ticks < target_ticks)
	{
		// compute how much time to sleep for, taking into account the average oversleep
		osd_ticks_t delta = target_ticks - current_ticks;
		if (delta > m_average_oversleep / 1000)
			delta -= m_average_oversleep / 1000;
		else
			delta = 0;

		// see if we can sleep
		bool const slept = allowed_to_sleep && delta;
		if (slept)
			osd_sleep(delta);

		// read the new value
		osd_ticks_t const new_ticks = osd_ticks();

		// keep some metrics on the sleeping patterns of the OSD layer
		if (slept)
		{
			// if we overslept, keep an average of the amount
			osd_ticks_t const actual_ticks = new_ticks - current_ticks;
			if (actual_ticks > delta)
			{
				// take 99% of the previous average plus 1% of the new value
				osd_ticks_t const oversleep_milliticks = 1000 * (actual_ticks - delta);
				m_average_oversleep = (m_average_oversleep * 99 + oversleep_milliticks) / 100;

				if (LOG_THROTTLE)
					machine().logerror("Slept for %d ticks, got %d ticks, avgover = %d\n", (int)delta, (int)actual_ticks, (int)m_average_oversleep);
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
		// calibrate the "adjusted speed" based on the target
		double adjusted_speed_percent = m_speed_percent / double(m_throttle_rate);

		double speed = m_speed * 0.001;
		if (adjusted_speed_percent >= 0.995 * speed)
		{
			// if we're too fast, attempt to decrease the frameskip
			// but only after 3 consecutive frames where we are too fast
			if (++m_frameskip_adjust >= 3)
			{
				m_frameskip_adjust = 0;
				if (m_frameskip_level > 0)
					m_frameskip_level--;
			}
		}
		else
		{
			// if we're too slow, attempt to increase the frameskip
			if (adjusted_speed_percent < 0.80 *  speed) // if below 80% speed, be more aggressive
				m_frameskip_adjust -= (0.90 * speed - m_speed_percent) / 0.05;
			else if (m_frameskip_level < 8) // if we're close, only force it up to frameskip 8
				m_frameskip_adjust--;

			// perform the adjustment
			while (m_frameskip_adjust <= -2)
			{
				m_frameskip_adjust += 2;
				if (m_frameskip_level < (m_frameskip_max ? m_frameskip_max : MAX_FRAMESKIP))
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
	if (machine().options().refresh_speed())
	{
		double minrefresh = machine().render().max_update_rate();
		if (minrefresh != 0)
		{
			// find the screen with the shortest frame period (max refresh rate)
			// note that we first check the token since this can get called before all screens are created
			attoseconds_t min_frame_period = ATTOSECONDS_PER_SECOND;
			for (screen_device &screen : screen_device_enumerator(machine().root_device()))
			{
				attoseconds_t period = screen.frame_period().attoseconds();
				if (period != 0)
					min_frame_period = std::min(min_frame_period, period);
			}

			// compute a target speed as an integral percentage
			// note that we lop 0.25Hz off of the minrefresh when doing the computation to allow for
			// the fact that most refresh rates are not accurate to 10 digits...
			u32 target_speed = floor((minrefresh - 0.25) * 1000.0 / ATTOSECONDS_TO_HZ(min_frame_period));
			u32 original_speed = original_speed_setting();
			target_speed = std::min(target_speed, original_speed);

			// if we changed, log that verbosely
			if (target_speed != m_speed)
			{
				osd_printf_verbose("Adjusting target speed to %.1f%% (hw=%.2fHz, game=%.2fHz, adjusted=%.2fHz)\n", target_speed / 10.0, minrefresh, ATTOSECONDS_TO_HZ(min_frame_period), ATTOSECONDS_TO_HZ(min_frame_period * 1000.0 / target_speed));
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

void video_manager::recompute_speed(const attotime &emutime)
{
	// if we don't have a starting time yet, or if we're paused, reset our starting point
	if (m_speed_last_realtime == 0 || machine().paused())
	{
		m_speed_last_realtime = osd_ticks();
		m_speed_last_emutime = emutime;
	}

	// if it has been more than the update interval, update the time
	attotime delta_emutime = emutime - m_speed_last_emutime;
	if (delta_emutime > attotime(0, ATTOSECONDS_PER_SPEED_UPDATE))
	{
		// convert from ticks to attoseconds
		osd_ticks_t realtime = osd_ticks();
		osd_ticks_t delta_realtime = realtime - m_speed_last_realtime;
		osd_ticks_t tps = osd_ticks_per_second();
		m_speed_percent = delta_emutime.as_double() * (double)tps / (double)delta_realtime;

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
			m_overall_emutime += delta_emutime;
		}
	}

	// if we're past the "time-to-execute" requested, signal an exit
	if (m_seconds_to_run != 0 && emutime.seconds() >= m_seconds_to_run)
	{
		// create a final screenshot
		emu_file file(machine().options().snapshot_directory(), OPEN_FLAG_WRITE | OPEN_FLAG_CREATE | OPEN_FLAG_CREATE_PATHS);
		std::error_condition const filerr = open_next(file, "png");
		if (!filerr)
			save_snapshot(nullptr, file);

		//printf("Scheduled exit at %f\n", emutime.as_double());
		// schedule our demise
		machine().schedule_exit();
	}
}


//-------------------------------------------------
//  create_snapshot_bitmap - creates a
//  bitmap containing the screenshot for the
//  given screen
//-------------------------------------------------

typedef software_renderer<u32, 0,0,0, 16,8,0, false, true> snap_renderer_bilinear;
typedef software_renderer<u32, 0,0,0, 16,8,0, false, false> snap_renderer;

void video_manager::create_snapshot_bitmap(screen_device *screen)
{
	// select the appropriate view in our dummy target
	if (m_snap_native && screen)
	{
		screen_device_enumerator iter(machine().root_device());
		int view_index = iter.indexof(*screen);
		assert(view_index != -1);
		m_snap_target->set_view(view_index);
	}

	// get the minimum width/height and set it on the target and bitmap
	s32 width, height;
	compute_snapshot_size(width, height);
	m_snap_target->set_bounds(width, height);
	if (width != m_snap_bitmap.width() || height != m_snap_bitmap.height())
		m_snap_bitmap.resize(width, height);

	// render the screen there
	render_primitive_list &primlist = m_snap_target->get_primitives();
	primlist.acquire_lock();
	if (machine().options().snap_bilinear())
		snap_renderer_bilinear::draw_primitives(primlist, &m_snap_bitmap.pix(0), width, height, m_snap_bitmap.rowpixels());
	else
		snap_renderer::draw_primitives(primlist, &m_snap_bitmap.pix(0), width, height, m_snap_bitmap.rowpixels());
	primlist.release_lock();
}


//-------------------------------------------------
//  compute_snapshot_size - computes width and
//  height of the current snapshot target
//  accounting for OPTION_SNAPSIZE
//-------------------------------------------------

void video_manager::compute_snapshot_size(s32 &width, s32 &height)
{
	width = m_snap_width;
	height = m_snap_height;
	if (width == 0 || height == 0)
		m_snap_target->compute_minimum_size(width, height);
}


//-------------------------------------------------
//  pixels - fills the specified buffer with the
//  RGB values of each pixel in the snapshot target
//-------------------------------------------------

void video_manager::pixels(u32 *buffer)
{
	create_snapshot_bitmap(nullptr);
	for (int y = 0; y < m_snap_bitmap.height(); y++)
	{
		const u32 *src = &m_snap_bitmap.pix(y, 0);
		for (int x = 0; x < m_snap_bitmap.width(); x++)
		{
			*buffer++ = *src++;
		}
	}
}


//-------------------------------------------------
//  open_next - open the next non-existing file of
//  type filetype according to our numbering
//  scheme
//-------------------------------------------------

std::error_condition video_manager::open_next(emu_file &file, const char *extension, uint32_t added_index)
{
	u32 origflags = file.openflags();

	// handle defaults
	const char *snapname = machine().options().snap_name();

	if (snapname == nullptr || snapname[0] == 0)
		snapname = "%g/%i";
	std::string snapstr(snapname);

	// strip any extension in the provided name
	int index = snapstr.find_last_of('.');
	if (index != -1)
		snapstr = snapstr.substr(0, index);

	// handle %d in the template (for image devices)
	std::string snapdev("%d_");
	int pos = snapstr.find(snapdev);

	if (pos != -1)
	{
		// if more %d are found, revert to default and ignore them all
		if (snapstr.find(snapdev, pos + 3) != -1)
			snapstr.assign("%g/%i");
		// else if there is a single %d, try to create the correct snapname
		else
		{
			int name_found = 0;

			// find length of the device name
			int end = snapstr.find_first_not_of("abcdefghijklmnopqrstuvwxyz1234567890", pos + 3);
			if (end == -1)
				end = snapstr.length();

			// copy the device name to an std::string
			std::string snapdevname;
			snapdevname.assign(snapstr.substr(pos + 3, end - pos - 3));
			//printf("check template: %s\n", snapdevname.c_str());

			// verify that there is such a device for this system
			for (device_image_interface &image : image_interface_enumerator(machine().root_device()))
			{
				// get the device name
				std::string tempdevname(image.brief_instance_name());
				//printf("check device: %s\n", tempdevname.c_str());

				if (snapdevname.compare(tempdevname) == 0)
				{
					// verify that such a device has an image mounted
					if (image.basename() != nullptr)
					{
						std::string filename(image.basename());

						// strip extension
						filename = filename.substr(0, filename.find_last_of('.'));

						// setup snapname and remove the %d_
						strreplace(snapstr, snapdevname, filename);
						snapstr.erase(pos, 3);
						//printf("check image: %s\n", filename.c_str());

						name_found = 1;
					}
				}
			}

			// or fallback to default
			if (name_found == 0)
				snapstr.assign("%g/%i");
		}
	}

	// handle %t in the template (for timestamp)
	std::string snaptime("%t");
	int pos_time = snapstr.find(snaptime);

	if (pos_time != -1)
	{
		char t_str[16];
		const std::time_t cur_time = std::time(nullptr);
		strftime(t_str, sizeof(t_str), "%Y%m%d_%H%M%S", std::localtime(&cur_time));
		strreplace(snapstr, "%t", t_str);
	}

	// add our own extension
	snapstr.append(".").append(extension);

	// substitute path and gamename up front
	strreplace(snapstr, "/", PATH_SEPARATOR);
	strreplace(snapstr, "%g", machine().basename());

	// determine if the template has an index; if not, we always use the same name
	std::string fname;
	if (snapstr.find("%i") == -1)
		fname.assign(snapstr);

	// otherwise, we scan for the next available filename
	else
	{
		// try until we succeed
		file.set_openflags(OPEN_FLAG_WRITE);
		for (int seq = 0; ; seq++)
		{
			// build up the filename
			fname.assign(snapstr);
			strreplace(fname, "%i", string_format("%04d", seq));

			// try to open the file; stop when we fail
			std::error_condition const filerr = file.open(fname);
			if (std::errc::no_such_file_or_directory == filerr)
				break;
		}
	}

	// create the final file
	file.set_openflags(origflags);
	return file.open(fname);
}


//-------------------------------------------------
//  record_frame - record a frame of a movie
//-------------------------------------------------

void video_manager::record_frame()
{
	// ignore if nothing to do
	if (!is_recording())
		return;

	// start the profiler and get the current time
	g_profiler.start(PROFILER_MOVIE_REC);
	attotime curtime = machine().time();

	bool error = false;
	for (auto &recording : m_movie_recordings)
	{
		// create the bitmap
		create_snapshot_bitmap(recording->screen());

		// and append the frame
		if (!recording->append_video_frame(m_snap_bitmap, curtime))
		{
			error = true;
			break;
		}
	}

	if (error)
		end_recording();
	g_profiler.stop();
}


//-------------------------------------------------
//  toggle_record_movie
//-------------------------------------------------

void video_manager::toggle_record_movie(movie_recording::format format)
{
	if (!is_recording())
	{
		begin_recording(nullptr, format);
		machine().popmessage("REC START (%s)", format == movie_recording::format::MNG ? "MNG" : "AVI");
	}
	else
	{
		end_recording();
		machine().popmessage("REC STOP");
	}
}

void video_manager::end_recording()
{
	m_movie_recordings.clear();
}
