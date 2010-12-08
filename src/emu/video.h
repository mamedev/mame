/***************************************************************************

    video.h

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



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// forward references
class render_target;
class screen_device;
typedef struct _avi_file avi_file;



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
	bool skip_this_frame() const { return m_skipping_this_frame; }
	int speed_factor() const { return m_speed; }
	int frameskip() const { return m_auto_frameskip ? -1 : m_frameskip_level; }
	bool throttled() const { return m_throttle; }
	bool fastforward() const { return m_fastforward; }
	bool is_recording() const { return (m_mngfile != NULL || m_avifile != NULL); }

	// setters
	void set_speed_factor(int speed) { m_speed = speed; }
	void set_frameskip(int frameskip);
	void set_throttled(bool throttled = true) { m_throttle = throttled; }
	void set_fastforward(bool ffwd = true) { m_fastforward = ffwd; }

	// render a frame
	void frame_update(bool debug = false);

	// current speed helpers
	astring &speed_text(astring &string);
	double speed_percent() const { return m_speed_percent; }

	// snapshots
	void save_snapshot(screen_device *screen, mame_file &file);
	void save_active_screen_snapshots();

	// movies
	void begin_recording(const char *name, movie_format format = MF_AVI);
	void end_recording();
	void add_sound_to_recording(const INT16 *sound, int numsamples);

private:
	// internal helpers
	static void exit_static(running_machine &machine);
	void exit();
	static TIMER_CALLBACK( screenless_update_callback );
	void postload();

	// effective value helpers
	int effective_autoframeskip() const;
	int effective_frameskip() const;
	bool effective_throttle() const;

	// speed and throttling helpers
	int original_speed_setting() const;
	bool finish_screen_updates();
	void update_throttle(attotime emutime);
	osd_ticks_t throttle_until_ticks(osd_ticks_t target_ticks);
	void update_frameskip();
	void update_refresh_speed();
	void recompute_speed(attotime emutime);

	// snapshot/movie helpers
	void create_snapshot_bitmap(device_t *screen);
	file_error mame_fopen_next(const char *pathoption, const char *extension, mame_file *&file);
	void record_frame();

	// internal state
	running_machine &	m_machine;					// reference to our machine

	// screenless systems
	emu_timer *			m_screenless_frame_timer;	// timer to signal VBLANK start

	// throttling calculations
	osd_ticks_t			m_throttle_last_ticks;		// osd_ticks the last call to throttle
	attotime			m_throttle_realtime;		// real time the last call to throttle
	attotime			m_throttle_emutime;			// emulated time the last call to throttle
	UINT32				m_throttle_history;			// history of frames where we were fast enough

	// dynamic speed computation
	osd_ticks_t 		m_speed_last_realtime;		// real time at the last speed calculation
	attotime			m_speed_last_emutime;		// emulated time at the last speed calculation
	double				m_speed_percent;			// most recent speed percentage

	// overall speed computation
	UINT32				m_overall_real_seconds;		// accumulated real seconds at normal speed
	osd_ticks_t			m_overall_real_ticks;		// accumulated real ticks at normal speed
	attotime			m_overall_emutime;			// accumulated emulated time at normal speed
	UINT32				m_overall_valid_counter;	// number of consecutive valid time periods

	// configuration
	bool				m_throttle;					// flag: TRUE if we're currently throttled
	bool				m_fastforward;				// flag: TRUE if we're currently fast-forwarding
	UINT32				m_seconds_to_run;			// number of seconds to run before quitting
	bool				m_auto_frameskip;			// flag: TRUE if we're automatically frameskipping
	UINT32				m_speed;					// overall speed (*100)

	// frameskipping
	UINT8				m_empty_skip_count;			// number of empty frames we have skipped
	UINT8				m_frameskip_level;			// current frameskip level
	UINT8				m_frameskip_counter;		// counter that counts through the frameskip steps
	INT8				m_frameskip_adjust;
	bool				m_skipping_this_frame;		// flag: TRUE if we are skipping the current frame
	osd_ticks_t			m_average_oversleep;		// average number of ticks the OSD oversleeps

	// snapshot stuff
	render_target *		m_snap_target;				// screen shapshot target
	bitmap_t *			m_snap_bitmap;				// screen snapshot bitmap
	bool				m_snap_native;				// are we using native per-screen layouts?
	INT32				m_snap_width;				// width of snapshots (0 == auto)
	INT32				m_snap_height;				// height of snapshots (0 == auto)

	// movie recording
	mame_file *			m_mngfile;					// handle to the open movie file
	avi_file *			m_avifile;					// handle to the open movie file
	attotime			m_movie_frame_period;		// period of a single movie frame
	attotime			m_movie_next_frame_time;	// time of next frame
	UINT32				m_movie_frame;				// current movie frame number

	static const UINT8		s_skiptable[FRAMESKIP_LEVELS][FRAMESKIP_LEVELS];

	static const attoseconds_t SUBSECONDS_PER_SPEED_UPDATE = ATTOSECONDS_PER_SECOND / 4;
	static const int PAUSED_REFRESH_RATE = 30;
};



// ----- debugging helpers -----

// assert if any pixels in the given bitmap contain an invalid palette index
void video_assert_out_of_range_pixels(running_machine *machine, bitmap_t *bitmap);


#endif	/* __VIDEO_H__ */
