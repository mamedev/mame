// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    video.h

    Core MAME video routines.

***************************************************************************/

#pragma once

#ifndef __EMU_H__
#error Dont include this file directly; include emu.h instead.
#endif

#ifndef __VIDEO_H__
#define __VIDEO_H__


//**************************************************************************
//  CONSTANTS
//**************************************************************************

// number of levels of frameskipping supported
const int FRAMESKIP_LEVELS = 12;
const int MAX_FRAMESKIP = FRAMESKIP_LEVELS - 2;

#define LCD_FRAMES_PER_SECOND   30

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// forward references
class render_target;
class screen_device;
struct avi_file;



// ======================> video_manager

class video_manager
{
	friend class screen_device;

public:
	// movie format options
	enum movie_format
	{
		MF_MNG,
		MF_AVI
	};

	// construction/destruction
	video_manager(running_machine &machine);

	// getters
	running_machine &machine() const { return m_machine; }
	bool skip_this_frame() const { return m_skipping_this_frame; }
	int speed_factor() const { return m_speed; }
	int frameskip() const { return m_auto_frameskip ? -1 : m_frameskip_level; }
	bool throttled() const { return m_throttled; }
	float throttle_rate() const { return m_throttle_rate; }
	bool fastforward() const { return m_fastforward; }
	bool is_recording() const { return (m_mng_file != nullptr || m_avi_file != nullptr); }

	// setters
	void set_frameskip(int frameskip);
	void set_throttled(bool throttled = true) { m_throttled = throttled; }
	void set_throttle_rate(float throttle_rate) { m_throttle_rate = throttle_rate; }
	void set_fastforward(bool ffwd = true) { m_fastforward = ffwd; }
	void set_output_changed() { m_output_changed = true; }

	// misc
	void toggle_throttle();
	void toggle_record_movie();

	// render a frame
	void frame_update(bool debug = false);

	// current speed helpers
	std::string speed_text();
	double speed_percent() const { return m_speed_percent; }

	// snapshots
	void save_snapshot(screen_device *screen, emu_file &file);
	void save_active_screen_snapshots();

	// movies
	void begin_recording(const char *name, movie_format format);
	void end_recording(movie_format format);
	void add_sound_to_recording(const INT16 *sound, int numsamples);

private:
	// internal helpers
	void exit();
	void screenless_update_callback(void *ptr, int param);
	void postload();

	// effective value helpers
	bool effective_autoframeskip() const;
	int effective_frameskip() const;
	bool effective_throttle() const;

	// speed and throttling helpers
	int original_speed_setting() const;
	bool finish_screen_updates();
	void update_throttle(attotime emutime);
	osd_ticks_t throttle_until_ticks(osd_ticks_t target_ticks);
	void update_frameskip();
	void update_refresh_speed();
	void recompute_speed(const attotime &emutime);

	// snapshot/movie helpers
	void create_snapshot_bitmap(screen_device *screen);
	file_error open_next(emu_file &file, const char *extension);
	void record_frame();

	// internal state
	running_machine &   m_machine;                  // reference to our machine

	// screenless systems
	emu_timer *         m_screenless_frame_timer;   // timer to signal VBLANK start
	bool                m_output_changed;           // did an output element change?

	// throttling calculations
	osd_ticks_t         m_throttle_last_ticks;      // osd_ticks the last call to throttle
	attotime            m_throttle_realtime;        // real time the last call to throttle
	attotime            m_throttle_emutime;         // emulated time the last call to throttle
	UINT32              m_throttle_history;         // history of frames where we were fast enough

	// dynamic speed computation
	osd_ticks_t         m_speed_last_realtime;      // real time at the last speed calculation
	attotime            m_speed_last_emutime;       // emulated time at the last speed calculation
	double              m_speed_percent;            // most recent speed percentage

	// overall speed computation
	UINT32              m_overall_real_seconds;     // accumulated real seconds at normal speed
	osd_ticks_t         m_overall_real_ticks;       // accumulated real ticks at normal speed
	attotime            m_overall_emutime;          // accumulated emulated time at normal speed
	UINT32              m_overall_valid_counter;    // number of consecutive valid time periods

	// configuration
	bool                m_throttled;                // flag: TRUE if we're currently throttled
	float               m_throttle_rate;            // target rate for throttling
	bool                m_fastforward;              // flag: TRUE if we're currently fast-forwarding
	UINT32              m_seconds_to_run;           // number of seconds to run before quitting
	bool                m_auto_frameskip;           // flag: TRUE if we're automatically frameskipping
	UINT32              m_speed;                    // overall speed (*1000)

	// frameskipping
	UINT8               m_empty_skip_count;         // number of empty frames we have skipped
	UINT8               m_frameskip_level;          // current frameskip level
	UINT8               m_frameskip_counter;        // counter that counts through the frameskip steps
	INT8                m_frameskip_adjust;
	bool                m_skipping_this_frame;      // flag: TRUE if we are skipping the current frame
	osd_ticks_t         m_average_oversleep;        // average number of ticks the OSD oversleeps

	// snapshot stuff
	render_target *     m_snap_target;              // screen shapshot target
	bitmap_rgb32        m_snap_bitmap;              // screen snapshot bitmap
	bool                m_snap_native;              // are we using native per-screen layouts?
	INT32               m_snap_width;               // width of snapshots (0 == auto)
	INT32               m_snap_height;              // height of snapshots (0 == auto)

	// movie recording - MNG
	std::unique_ptr<emu_file> m_mng_file;              // handle to the open movie file
	attotime            m_mng_frame_period;         // period of a single movie frame
	attotime            m_mng_next_frame_time;      // time of next frame
	UINT32              m_mng_frame;                // current movie frame number

	// movie recording - AVI
	avi_file *          m_avi_file;                 // handle to the open movie file
	attotime            m_avi_frame_period;         // period of a single movie frame
	attotime            m_avi_next_frame_time;      // time of next frame
	UINT32              m_avi_frame;                // current movie frame number

	// movie recording - dummy
	bool                m_dummy_recording;          // indicates if snapshot should be created of every frame

	static const UINT8      s_skiptable[FRAMESKIP_LEVELS][FRAMESKIP_LEVELS];

	static const attoseconds_t ATTOSECONDS_PER_SPEED_UPDATE = ATTOSECONDS_PER_SECOND / 4;
	static const int PAUSED_REFRESH_RATE = 30;
};

#endif  /* __VIDEO_H__ */
