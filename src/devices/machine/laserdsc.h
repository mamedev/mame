// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/*************************************************************************

    Core laserdisc player implementation.

*************************************************************************/

#ifndef MAME_MACHINE_LASERDSC_H
#define MAME_MACHINE_LASERDSC_H

#pragma once

#include "emupal.h"
#include "screen.h"

#include "avhuff.h"
#include "vbiparse.h"

#include <algorithm>
#include <system_error>
#include <utility>
#include <vector>


//**************************************************************************
//  CONSTANTS
//**************************************************************************

// laserdisc field codes
enum laserdisc_field_code
{
	LASERDISC_CODE_WHITE_FLAG = 11,     // boolean white flag
	LASERDISC_CODE_LINE16 = 16,         // 24-bit line 16 code
	LASERDISC_CODE_LINE17 = 17,         // 24-bit line 17 code
	LASERDISC_CODE_LINE18 = 18,         // 24-bit line 18 code
	LASERDISC_CODE_LINE1718 = 1718      // 24-bit best of line 17/18 code
};


// special frame and chapter numbers from VBI conversion
#define FRAME_NOT_PRESENT           -2                      // no frame number information present
#define FRAME_LEAD_IN               -1                      // lead-in code detected
#define FRAME_LEAD_OUT              99999                   // lead-out code detected
#define CHAPTER_NOT_PRESENT         -2                      // no chapter number information present
#define CHAPTER_LEAD_IN             -1                      // lead-in code detected
#define CHAPTER_LEAD_OUT            100                     // lead-out code detected

// generic head movement speeds; use player-specific information where appropriate
#define GENERIC_SLOW_SPEED          (5)                     // 1/5 normal speed
#define GENERIC_FAST_SPEED          (3)                     // 3x normal speed
#define GENERIC_SCAN_SPEED          (50)                    // 50x normal speed
#define GENERIC_SEARCH_SPEED        (5000)                  // 5000x normal speed

// generic timings; use player-specific information where appropriate
#define GENERIC_EJECT_TIME          (attotime::from_seconds(5))
#define GENERIC_SPINUP_TIME         (attotime::from_seconds(2))
#define GENERIC_LOAD_TIME           (attotime::from_seconds(5))



//**************************************************************************
//  MACROS
//**************************************************************************

#define SCANNING_PARAM(speed,duration)  (((speed) << 8) | ((duration) & 0xff))



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> laserdisc_overlay_config

// overlay configuration
struct laserdisc_overlay_config
{
	float                   m_overposx;
	float                   m_overposy;
	float                   m_overscalex;
	float                   m_overscaley;
};


// ======================> laserdisc_device

// base laserdisc class
class laserdisc_device :    public device_t,
							public device_sound_interface,
							public device_video_interface,
							public laserdisc_overlay_config
{
protected:
	// construction/destruction
	virtual ~laserdisc_device();

public:
	// delegates
	typedef device_delegate<chd_file * ()> get_disc_delegate;
	typedef device_delegate<void (int samplerate, int samples, const int16_t *ch0, const int16_t *ch1)> audio_delegate;

	laserdisc_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// use these to add laserdisc screens with proper video update parameters
	// TODO: actually move these SCREEN_RAW_PARAMS to a common screen info header
	// TODO: someday we'll kill the pixel clock hack ...
	void add_ntsc_screen(machine_config &config, const char *tag);
	void add_pal_screen(machine_config &config, const char *tag);

	// core control and status
	bool video_active() { return (!m_videosquelch && current_frame().m_numfields >= 2); }
	bitmap_yuy16 &get_video() { return (!video_active()) ? m_emptyframe : current_frame().m_visbitmap; }
	uint32_t get_field_code(laserdisc_field_code code, bool zero_if_squelched);

	// video interface
	void video_enable(bool enable) { m_videoenable = enable; }
	void overlay_enable(bool enable) { m_overenable = enable; }

	// video update callback
	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	// configuration
	bool overlay_configured() const { return (m_overwidth > 0 && m_overheight > 0 && (!m_overupdate_rgb32.isnull())); }
	void get_overlay_config(laserdisc_overlay_config &config) { config = static_cast<laserdisc_overlay_config &>(*this); }
	void set_overlay_config(const laserdisc_overlay_config &config) { static_cast<laserdisc_overlay_config &>(*this) = config; }

	// configuration helpers
	template <typename... T> void set_get_disc(T &&... args) { m_getdisc_callback.set(std::forward<T>(args)...); }
	template <typename... T> void set_audio(T &&... args) { m_audio_callback.set(std::forward<T>(args)...); }
	template <typename... T>
	void set_overlay(uint32_t width, uint32_t height, T &&... args)
	{
		m_overwidth = width;
		m_overheight = height;
		m_overclip.set(0, width - 1, 0, height - 1);
		m_overupdate_rgb32.set(std::forward<T>(args)...);
	}
	void set_overlay_clip(int32_t minx, int32_t maxx, int32_t miny, int32_t maxy) { m_overclip.set(minx, maxx, miny, maxy); }
	void set_overlay_position(float posx, float posy)
	{
		m_orig_config.m_overposx = m_overposx = posx;
		m_orig_config.m_overposy = m_overposy = posy;
	}
	void set_overlay_scale(float scalex, float scaley)
	{
		m_orig_config.m_overscalex = m_overscalex = scalex;
		m_orig_config.m_overscaley = m_overscaley = scaley;
	}

protected:
	// common laserdisc states
	enum player_state : uint32_t
	{
		LDSTATE_NONE,                           // unspecified state
		LDSTATE_EJECTING,                       // in the process of ejecting
		LDSTATE_EJECTED,                        // fully ejected
		LDSTATE_PARKED,                         // head parked in lead-in
		LDSTATE_LOADING,                        // loading from ejected state
		LDSTATE_SPINUP,                         // spinning up
		LDSTATE_PAUSING,                        // looking for a frame boundary to pause
		LDSTATE_PAUSED,                         // found a frame boundary; now paused
												//   parameter specifies the fieldnum of the first frame
		LDSTATE_PLAYING,                        // playing forward normally, with audio
												//   parameter specifies the target frame, or 0 if none
		LDSTATE_PLAYING_SLOW_REVERSE,           // playing slow in the reverse direction, with no audio
												//   parameter specifies the number of times to repeat each track
		LDSTATE_PLAYING_SLOW_FORWARD,           // playing slow in the forward direction, with no audio
												//   parameter specifies the number of times to repeat each track
		LDSTATE_PLAYING_FAST_REVERSE,           // playing fast in the reverse direction, with no audio
												//   parameter specifies the number of frames to skip backwards after each frame
		LDSTATE_PLAYING_FAST_FORWARD,           // playing fast in the forward direction, with no audio
												//   parameter specifies the number of frames to skip forwards after each frame
		LDSTATE_STEPPING_REVERSE,               // single frame stepping in the reverse direction
		LDSTATE_STEPPING_FORWARD,               // single frame stepping in the forward direction
		LDSTATE_SCANNING,                       // scanning in the forward or reverse direction
												//   parameter(0:7) controls how many vsyncs until revert to savestate
												//   parameter(8:31) specifies the speed
		LDSTATE_SEEKING,                        // seeking to a specific frame
												//   parameter specifies the target frame
		LDSTATE_OTHER                           // other states start here
	};

	// slider position
	enum slider_position : uint32_t
	{
		SLIDER_MINIMUM,                         // at the minimum value
		SLIDER_VIRTUAL_LEADIN,                  // within the virtual lead-in area
		SLIDER_CHD,                             // within the boundaries of the CHD
		SLIDER_OUTSIDE_CHD,                     // outside of the CHD area but before the virtual lead-out area
		SLIDER_VIRTUAL_LEADOUT,                 // within the virtual lead-out area
		SLIDER_MAXIMUM                          // at the maximum value
	};

	// information about the current player state
	struct player_state_info
	{
		player_state    m_state;                // current state
		int32_t         m_substate;             // internal sub-state; starts at 0 on any state change
		int32_t         m_param;                // parameter for current state
		attotime        m_endtime;              // minimum ending time for current state
	};

	// subclass overrides
	virtual void player_vsync(const vbi_metadata &vbi, int fieldnum, const attotime &curtime) = 0;
	virtual int32_t player_update(const vbi_metadata &vbi, int fieldnum, const attotime &curtime) = 0;
	virtual void player_overlay(bitmap_yuy16 &bitmap) = 0;

	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_stop() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_validity_check(validity_checker &valid) const override;

	// device_sound_interface overrides
	virtual void sound_stream_update(sound_stream &stream) override;

	virtual TIMER_CALLBACK_MEMBER(fetch_vbi_data);

	// subclass helpers
	void set_audio_squelch(bool squelchleft, bool squelchright) { m_stream->update(); m_audiosquelch = (squelchleft ? 1 : 0) | (squelchright ? 2 : 0); }
	void set_video_squelch(bool squelch) { m_videosquelch = squelch; }
	void set_slider_speed(const double tracks_per_vsync);
	void advance_slider(int32_t numtracks);
	slider_position get_slider_position();
	int32_t generic_update(const vbi_metadata &vbi, int fieldnum, const attotime &curtime, player_state_info &curstate);

	// general helpers
	bool is_cav_disc() const { return m_is_cav_disc; }
	bool is_start_of_frame(const vbi_metadata &vbi);
	int frame_from_metadata(const vbi_metadata &metadata);
	int chapter_from_metadata(const vbi_metadata &metadata);

	player_state_info   m_player_state;         // active state
	player_state_info   m_saved_state;          // saved state during temporary operations

	emu_timer           *m_vbi_fetch_timer;     // fetcher for our VBI data

private:
	// internal type definitions
	struct frame_data
	{
		bitmap_yuy16        m_bitmap;               // cached bitmap
		bitmap_yuy16        m_visbitmap;            // wrapper around bitmap with only visible lines
		uint8_t             m_numfields;            // number of fields in this frame
		int32_t             m_lastfield;            // last absolute field number
	};

	// internal helpers
	void init_disc();
	void init_video();
	void init_audio();
	void add_and_clamp_track(int32_t delta) { m_curtrack += delta; m_curtrack = std::max(m_curtrack, 1); m_curtrack = std::min(m_curtrack, int32_t(m_maxtrack) - 1); }
	void fillbitmap_yuy16(bitmap_yuy16 &bitmap, uint8_t yval, uint8_t cr, uint8_t cb);
	void update_slider_pos();
	void vblank_state_changed(screen_device &screen, bool vblank_state);
	frame_data &current_frame();
	void read_track_data();
	static void *read_async_static(void *param, int threadid);
	void process_track_data();
	void config_load(config_type cfg_type, config_level cfg_level, util::xml::data_node const *parentnode);
	void config_save(config_type cfg_type, util::xml::data_node *parentnode);

	// configuration
	get_disc_delegate m_getdisc_callback;
	audio_delegate m_audio_callback;  // audio streaming callback
	laserdisc_overlay_config m_orig_config;     // original overlay configuration
	uint32_t            m_overwidth;            // overlay screen width
	uint32_t            m_overheight;           // overlay screen height
	rectangle           m_overclip;             // overlay visarea
	screen_update_rgb32_delegate m_overupdate_rgb32; // overlay update delegate

	// disc parameters
	chd_file *          m_disc;                 // handle to the disc itself
	std::vector<uint8_t> m_vbidata;             // pointer to precomputed VBI data
	bool                m_is_cav_disc;          // precomputed check if the mounted disc is CAV
	int                 m_width;                // width of video
	int                 m_height;               // height of video
	uint32_t            m_fps_times_1million;   // frame rate of video
	int                 m_samplerate;           // audio samplerate
	std::error_condition m_readresult;          // result of the most recent read
	uint32_t            m_chdtracks;            // number of tracks in the CHD
	bitmap_yuy16        m_avhuff_video;         // decompresed frame buffer
	avhuff_decoder::config m_avhuff_config;     // decompression configuration

	// async operations
	osd_work_queue *    m_work_queue;           // work queue
	uint32_t            m_queued_hunknum;       // queued hunk

	// core states
	uint8_t             m_audiosquelch;         // audio squelch state: bit 0 = audio 1, bit 1 = audio 2
	uint8_t             m_videosquelch;         // video squelch state: bit 0 = on/off
	uint8_t             m_fieldnum;             // field number (0 or 1)
	int32_t             m_curtrack;             // current track at this end of this vsync
	uint32_t            m_maxtrack;             // maximum track number
	attoseconds_t       m_attospertrack;        // attoseconds per track, or 0 if not moving
	attotime            m_sliderupdate;         // time of last slider update

	// video data
	frame_data          m_frame[3];             // circular list of frames
	uint8_t             m_videoindex;           // index of the current video buffer
	bitmap_yuy16        m_emptyframe;           // blank frame

	// audio data
	sound_stream *      m_stream;
	std::vector<int16_t> m_audiobuffer[2];      // buffer for audio samples
	uint32_t            m_audiobufsize;         // size of buffer
	uint32_t            m_audiobufin;           // input index
	uint32_t            m_audiobufout;          // output index
	uint32_t            m_audiocursamples;      // current samples this track
	uint32_t            m_audiomaxsamples;      // maximum samples per track

	// metadata
	vbi_metadata        m_metadata[2];          // metadata parsed from the stream, for each field

	// video updating
	bool                m_videoenable;          // is video enabled?
	render_texture *    m_videotex;             // texture for the video
	palette_t *         m_videopalette;         // palette for the video

	// overlays
	bool                m_overenable;           // is the overlay enabled?
	screen_bitmap       m_overbitmap[2];        // overlay bitmaps
	int                 m_overindex;            // index of the overlay bitmap
	render_texture *    m_overtex;              // texture for the overlay
};

// iterator - interface iterator works for subclasses too
typedef device_interface_enumerator<laserdisc_device> laserdisc_device_enumerator;



//**************************************************************************
//  INLINE FUNCTIONS
//**************************************************************************

// ======================> parallel_laserdisc_device

class parallel_laserdisc_device : public laserdisc_device
{
public:

	virtual void data_w(u8 data) = 0;
	virtual u8 data_r() = 0;
	virtual void enter_w(int state) {}
	virtual int data_available_r() { return CLEAR_LINE; }
	virtual int status_strobe_r() { return CLEAR_LINE; }
	virtual int ready_r() { return ASSERT_LINE; }

protected:
	parallel_laserdisc_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock = 0);

	virtual void player_overlay(bitmap_yuy16 &bitmap) override { }
};

//-------------------------------------------------
//  is_start_of_frame - return true if this is
//  the start of a frame
//-------------------------------------------------

inline bool laserdisc_device::is_start_of_frame(const vbi_metadata &vbi)
{
	// is it not known if the white flag or the presence of a frame code
	// determines the start of frame; the former seems to be the "official"
	// way, but the latter seems to be the practical implementation
	return (vbi.white || (vbi.line1718 & VBI_MASK_CAV_PICTURE) == VBI_CODE_CAV_PICTURE);
}


//-------------------------------------------------
//  frame_from_metadata - return the frame number
//  encoded in the metadata, if present, or
//  FRAME_NOT_PRESENT
//-------------------------------------------------

inline int laserdisc_device::frame_from_metadata(const vbi_metadata &metadata)
{
	if ((metadata.line1718 & VBI_MASK_CAV_PICTURE) == VBI_CODE_CAV_PICTURE)
		return VBI_CAV_PICTURE(metadata.line1718);
	else if (metadata.line1718 == VBI_CODE_LEADIN)
		return FRAME_LEAD_IN;
	else if (metadata.line1718 == VBI_CODE_LEADOUT)
		return FRAME_LEAD_OUT;
	return FRAME_NOT_PRESENT;
}


//-------------------------------------------------
//  chapter_from_metadata - return the chapter
//  number encoded in the metadata, if present,
//  or CHAPTER_NOT_PRESENT
//-------------------------------------------------

inline int laserdisc_device::chapter_from_metadata(const vbi_metadata &metadata)
{
	if ((metadata.line1718 & VBI_MASK_CHAPTER) == VBI_CODE_CHAPTER)
		return VBI_CHAPTER(metadata.line1718);
	else if (metadata.line1718 == VBI_CODE_LEADIN)
		return CHAPTER_LEAD_IN;
	else if (metadata.line1718 == VBI_CODE_LEADOUT)
		return CHAPTER_LEAD_OUT;
	return CHAPTER_NOT_PRESENT;
}

#endif // MAME_MACHINE_LASERDSC_H
