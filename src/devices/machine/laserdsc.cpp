// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/*************************************************************************

    laserdsc.c

    Core laserdisc player implementation.

*************************************************************************/

#include "emu.h"
#include "laserdsc.h"
#include "avhuff.h"
#include "vbiparse.h"
#include "config.h"
#include "render.h"
#include "chd.h"



//**************************************************************************
//  DEBUGGING
//**************************************************************************

#define LOG_SLIDER                  0



//**************************************************************************
//  CONSTANTS
//**************************************************************************

// these specs code from IEC 60857, for NTSC players
const UINT32 LEAD_IN_MIN_RADIUS_IN_UM = 53500;      // 53.5 mm
const UINT32 PROGRAM_MIN_RADIUS_IN_UM = 55000;      // 55 mm
const UINT32 PROGRAM_MAX_RADIUS_IN_UM = 145000;     // 145 mm
const UINT32 LEAD_OUT_MIN_SIZE_IN_UM = 2000;        // 2 mm

// the track pitch is defined as a range; we pick a nominal pitch
// that ensures we can fit 54,000 tracks
//const UINT32 MIN_TRACK_PITCH_IN_NM = 1400;          // 1.4 um
//const UINT32 MAX_TRACK_PITCH_IN_NM = 2000;          // 2 um
const UINT32 NOMINAL_TRACK_PITCH_IN_NM = (PROGRAM_MAX_RADIUS_IN_UM - PROGRAM_MIN_RADIUS_IN_UM) * 1000 / 54000;

// we simulate extra lead-in and lead-out tracks
const UINT32 VIRTUAL_LEAD_IN_TRACKS = (PROGRAM_MIN_RADIUS_IN_UM - LEAD_IN_MIN_RADIUS_IN_UM) * 1000 / NOMINAL_TRACK_PITCH_IN_NM;
const UINT32 MAX_TOTAL_TRACKS = 54000;
const UINT32 VIRTUAL_LEAD_OUT_TRACKS = LEAD_OUT_MIN_SIZE_IN_UM * 1000 / NOMINAL_TRACK_PITCH_IN_NM;



//**************************************************************************
//  CORE IMPLEMENTATION
//**************************************************************************

//-------------------------------------------------
//  laserdisc_device - constructor
//-------------------------------------------------

laserdisc_device::laserdisc_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source)
	: device_t(mconfig, type, name, tag, owner, clock, shortname, source),
		device_sound_interface(mconfig, *this),
		device_video_interface(mconfig, *this),
		m_overwidth(0),
		m_overheight(0),
		m_overclip(0, -1, 0, -1),
		m_disc(nullptr),
		m_width(0),
		m_height(0),
		m_fps_times_1million(0),
		m_samplerate(0),
		m_readresult(CHDERR_NONE),
		m_chdtracks(0),
		m_work_queue(osd_work_queue_alloc(WORK_QUEUE_FLAG_IO)),
		m_audiosquelch(0),
		m_videosquelch(0),
		m_fieldnum(0),
		m_curtrack(0),
		m_maxtrack(0),
		m_attospertrack(0),
		m_sliderupdate(attotime::zero),
		m_videoindex(0),
		m_stream(nullptr),
		m_audiobufsize(0),
		m_audiobufin(0),
		m_audiobufout(0),
		m_audiocursamples(0),
		m_audiomaxsamples(0),
		m_videoenable(false),
		m_videotex(nullptr),
		m_videopalette(nullptr),
		m_overenable(false),
		m_overindex(0),
		m_overtex(nullptr),
		m_overlay_palette(*this)
{
	// initialize overlay_config
	m_orig_config.m_overposx = m_orig_config.m_overposy = 0.0f;
	m_orig_config.m_overscalex = m_orig_config.m_overscaley = 1.0f;
	*static_cast<laserdisc_overlay_config *>(this) = m_orig_config;
}


//-------------------------------------------------
//  ~laserdisc_device - destructor
//-------------------------------------------------

laserdisc_device::~laserdisc_device()
{
	osd_work_queue_free(m_work_queue);
}



//**************************************************************************
//  PUBLIC INTERFACES
//**************************************************************************

//-------------------------------------------------
//  get_field_code - return raw field information
//  read from the disc
//-------------------------------------------------

UINT32 laserdisc_device::get_field_code(laserdisc_field_code code, bool zero_if_squelched)
{
	// return nothing if the video is off (external devices can't sense)
	if (zero_if_squelched && m_videosquelch)
		return 0;

	switch (code)
	{
		case LASERDISC_CODE_WHITE_FLAG:
			return m_metadata[m_fieldnum].white;

		case LASERDISC_CODE_LINE16:
			return m_metadata[m_fieldnum].line16;

		case LASERDISC_CODE_LINE17:
			return m_metadata[m_fieldnum].line17;

		case LASERDISC_CODE_LINE18:
			return m_metadata[m_fieldnum].line18;

		case LASERDISC_CODE_LINE1718:
			return m_metadata[m_fieldnum].line1718;
	}
	return 0;
}


//-------------------------------------------------
//  screen_update - handle updating the screen
//-------------------------------------------------

UINT32 laserdisc_device::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	// handle the overlay if present
	screen_bitmap &overbitmap = m_overbitmap[m_overindex];
	if (overbitmap.valid() && (!m_overupdate_ind16.isnull() || !m_overupdate_rgb32.isnull()))
	{
		// scale the cliprect to the overlay size
		rectangle clip(m_overclip);
		clip.min_y = cliprect.min_y * overbitmap.height() / bitmap.height();
		if (cliprect.min_y == screen.visible_area().min_y)
			clip.min_y = MIN(clip.min_y, m_overclip.min_y);
		clip.max_y = (cliprect.max_y + 1) * overbitmap.height() / bitmap.height() - 1;

		// call the update callback
		if (!m_overupdate_ind16.isnull())
			m_overupdate_ind16(screen, overbitmap.as_ind16(), clip);
		else
			m_overupdate_rgb32(screen, overbitmap.as_rgb32(), clip);
	}

	// if this is the last update, do the rendering
	if (cliprect.max_y == screen.visible_area().max_y)
	{
		// update the texture with the overlay contents
		if (overbitmap.valid())
			m_overtex->set_bitmap(overbitmap, m_overclip, overbitmap.texformat());

		// get the laserdisc video
		bitmap_yuy16 &vidbitmap = get_video();
		m_videotex->set_bitmap(vidbitmap, vidbitmap.cliprect(), TEXFORMAT_YUY16);

		// reset the screen contents
		screen.container().empty();

		// add the video texture
		if (m_videoenable)
			screen.container().add_quad(0.0f, 0.0f, 1.0f, 1.0f, rgb_t(0xff,0xff,0xff,0xff), m_videotex, PRIMFLAG_BLENDMODE(BLENDMODE_NONE) | PRIMFLAG_SCREENTEX(1));

		// add the overlay
		if (m_overenable && overbitmap.valid())
		{
			float x0 = 0.5f - 0.5f * m_overscalex + m_overposx;
			float y0 = 0.5f - 0.5f * m_overscaley + m_overposy;
			float x1 = x0 + m_overscalex;
			float y1 = y0 + m_overscaley;
			screen.container().add_quad(x0, y0, x1, y1, rgb_t(0xff,0xff,0xff,0xff), m_overtex, PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA) | PRIMFLAG_SCREENTEX(1));
		}

		// swap to the next bitmap
		m_overindex = (m_overindex + 1) % ARRAY_LENGTH(m_overbitmap);
	}
	return 0;
}


//-------------------------------------------------
//  static_set_get_disc - set the get disc
//  delegate
//-------------------------------------------------

void laserdisc_device::static_set_get_disc(device_t &device, laserdisc_get_disc_delegate callback)
{
	downcast<laserdisc_device &>(device).m_getdisc_callback = callback;
}


//-------------------------------------------------
//  static_set_get_disc - set the audio interceptor
//  delegate
//-------------------------------------------------

void laserdisc_device::static_set_audio(device_t &device, laserdisc_audio_delegate callback)
{
	downcast<laserdisc_device &>(device).m_audio_callback = callback;
}


//-------------------------------------------------
//  static_set_overlay - set the overlay parameters
//-------------------------------------------------

void laserdisc_device::static_set_overlay(device_t &device, UINT32 width, UINT32 height, screen_update_ind16_delegate update)
{
	laserdisc_device &ld = downcast<laserdisc_device &>(device);
	ld.m_overwidth = width;
	ld.m_overheight = height;
	ld.m_overclip.set(0, width - 1, 0, height - 1);
	ld.m_overupdate_ind16 = update;
	ld.m_overupdate_rgb32 = screen_update_rgb32_delegate();
}

void laserdisc_device::static_set_overlay(device_t &device, UINT32 width, UINT32 height, screen_update_rgb32_delegate update)
{
	laserdisc_device &ld = downcast<laserdisc_device &>(device);
	ld.m_overwidth = width;
	ld.m_overheight = height;
	ld.m_overclip.set(0, width - 1, 0, height - 1);
	ld.m_overupdate_ind16 = screen_update_ind16_delegate();
	ld.m_overupdate_rgb32 = update;
}


//-------------------------------------------------
//  static_set_overlay - set the overlay visible
//  memregion
//-------------------------------------------------

void laserdisc_device::static_set_overlay_clip(device_t &device, INT32 minx, INT32 maxx, INT32 miny, INT32 maxy)
{
	downcast<laserdisc_device &>(device).m_overclip.set(minx, maxx, miny, maxy);
}


//-------------------------------------------------
//  static_set_overlay_position - set the overlay
//  position parameters
//-------------------------------------------------

void laserdisc_device::static_set_overlay_position(device_t &device, float posx, float posy)
{
	laserdisc_device &ld = downcast<laserdisc_device &>(device);
	ld.m_orig_config.m_overposx = ld.m_overposx = posx;
	ld.m_orig_config.m_overposy = ld.m_overposy = posy;
}


//-------------------------------------------------
//  static_set_overlay_scale - set the overlay
//  scale parameters
//-------------------------------------------------

void laserdisc_device::static_set_overlay_scale(device_t &device, float scalex, float scaley)
{
	laserdisc_device &ld = downcast<laserdisc_device &>(device);
	ld.m_orig_config.m_overscalex = ld.m_overscalex = scalex;
	ld.m_orig_config.m_overscaley = ld.m_overscaley = scaley;

}

//-------------------------------------------------
//  static_set_overlay_palette - set the screen palette
//  configuration
//-------------------------------------------------

void laserdisc_device::static_set_overlay_palette(device_t &device, const char *tag)
{
	downcast<laserdisc_device &>(device).m_overlay_palette.set_tag(tag);
}

//**************************************************************************
//  DEVICE INTERFACE
//**************************************************************************

//-------------------------------------------------
//  device start callback
//-------------------------------------------------

void laserdisc_device::device_start()
{
	// if we have a palette and it's not started, wait for it
	if (m_overlay_palette != nullptr && !m_overlay_palette->started())
		throw device_missing_dependencies();

	// initialize the various pieces
	init_disc();
	init_video();
	init_audio();

	// register callbacks
	config_register(machine(), "laserdisc", config_saveload_delegate(FUNC(laserdisc_device::config_load), this), config_saveload_delegate(FUNC(laserdisc_device::config_save), this));
}


//-------------------------------------------------
//  device stop callback
//-------------------------------------------------

void laserdisc_device::device_stop()
{
	// make sure all async operations have completed
	if (m_disc != nullptr)
		osd_work_queue_wait(m_work_queue, osd_ticks_per_second() * 10);

	// free any textures and palettes
	if (m_videotex != nullptr)
		machine().render().texture_free(m_videotex);
	if (m_videopalette != nullptr)
		m_videopalette->deref();
	if (m_overtex != nullptr)
		machine().render().texture_free(m_overtex);
}


//-------------------------------------------------
//  device reset callback
//-------------------------------------------------

void laserdisc_device::device_reset()
{
	// attempt to wire up the audio
	m_stream->set_sample_rate(m_samplerate);

	// set up the general ld
	m_audiosquelch = 3;
	m_videosquelch = 1;
	m_fieldnum = 0;
	m_curtrack = 1;
	m_attospertrack = 0;
	m_sliderupdate = machine().time();
}


//-------------------------------------------------
//  device_validity_check - verify device
//  configuration
//-------------------------------------------------

void laserdisc_device::device_validity_check(validity_checker &valid) const
{
	texture_format texformat = !m_overupdate_ind16.isnull() ? TEXFORMAT_PALETTE16 : TEXFORMAT_RGB32;
	if (m_overlay_palette == nullptr && texformat == TEXFORMAT_PALETTE16)
		osd_printf_error("Overlay screen does not have palette defined\n");
}

//-------------------------------------------------
//  device_timer - handle timers set by this
//  device
//-------------------------------------------------

void laserdisc_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	switch (id)
	{
		case TID_VBI_FETCH:
		{
			// wait for previous read and decode to finish
			process_track_data();

			// update current track based on slider speed
			update_slider_pos();

			// update the state
			add_and_clamp_track(player_update(m_metadata[m_fieldnum], m_fieldnum, machine().time()));

			// flush any audio before we read more
			m_stream->update();

			// start reading the track data for the next round
			m_fieldnum ^= 1;
			read_track_data();
			break;
		}
	}
}


//-------------------------------------------------
//  sound_stream_update - audio streamer for
//  laserdiscs
//-------------------------------------------------

void laserdisc_device::sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples)
{
	// compute AND values based on the squelch
	INT16 leftand = (m_audiosquelch & 1) ? 0x0000 : 0xffff;
	INT16 rightand = (m_audiosquelch & 2) ? 0x0000 : 0xffff;

	// see if we have enough samples to fill the buffer; if not, drop out
	int samples_avail = m_audiobufin - m_audiobufout;
	if (samples_avail < 0)
		samples_avail += m_audiobufsize;

	// if no attached ld, just clear the buffers
	stream_sample_t *dst0 = outputs[0];
	stream_sample_t *dst1 = outputs[1];
	if (samples_avail < samples)
	{
		memset(dst0, 0, samples * sizeof(dst0[0]));
		memset(dst1, 0, samples * sizeof(dst1[0]));
	}

	// otherwise, stream from our buffer
	else
	{
		INT16 *buffer0 = &m_audiobuffer[0][0];
		INT16 *buffer1 = &m_audiobuffer[1][0];
		int sampout = m_audiobufout;

		// copy samples, clearing behind us as we go
		while (sampout != m_audiobufin && samples-- > 0)
		{
			*dst0++ = buffer0[sampout] & leftand;
			*dst1++ = buffer1[sampout] & rightand;
			buffer0[sampout] = 0;
			buffer1[sampout] = 0;
			sampout++;
			if (sampout >= m_audiobufsize)
				sampout = 0;
		}
		m_audiobufout = sampout;

		// clear out the rest of the buffer
		if (samples > 0)
		{
			sampout = (m_audiobufout == 0) ? m_audiobufsize - 1 : m_audiobufout - 1;
			stream_sample_t fill0 = buffer0[sampout] & leftand;
			stream_sample_t fill1 = buffer1[sampout] & rightand;

			while (samples-- > 0)
			{
				*dst0++ = fill0;
				*dst1++ = fill1;
			}
		}
	}
}


//**************************************************************************
//  SUBCLASS HELPERS
//**************************************************************************

//-------------------------------------------------
//  set_slider_speed - dynamically change the
//  slider speed
//-------------------------------------------------

void laserdisc_device::set_slider_speed(INT32 tracks_per_vsync)
{
	// update to the current time
	update_slider_pos();

	// if 0, set the time to 0
	attotime vsyncperiod = m_screen->frame_period();
	if (tracks_per_vsync == 0)
		m_attospertrack = 0;

	// positive values store positive times
	else if (tracks_per_vsync > 0)
		m_attospertrack = (vsyncperiod / tracks_per_vsync).as_attoseconds();

	// negative values store negative times
	else
		m_attospertrack = -(vsyncperiod / -tracks_per_vsync).as_attoseconds();

	if (LOG_SLIDER)
		printf("Slider speed = %d\n", tracks_per_vsync);
}


//-------------------------------------------------
//  advance_slider - advance the slider by
//  a certain number of tracks
//-------------------------------------------------

void laserdisc_device::advance_slider(INT32 numtracks)
{
	// first update to the current time
	update_slider_pos();

	// then update the track position
	add_and_clamp_track(numtracks);
	if (LOG_SLIDER)
		printf("Advance by %d\n", numtracks);
}


//-------------------------------------------------
//  get_slider_position - get the current
//  slider position
//-------------------------------------------------

laserdisc_device::slider_position laserdisc_device::get_slider_position()
{
	// update the slider position first
	update_slider_pos();

	// return the status
	if (m_curtrack == 1)
		return SLIDER_MINIMUM;
	else if (m_curtrack < VIRTUAL_LEAD_IN_TRACKS)
		return SLIDER_VIRTUAL_LEADIN;
	else if (m_curtrack < VIRTUAL_LEAD_IN_TRACKS + m_chdtracks)
		return SLIDER_CHD;
	else if (m_curtrack < VIRTUAL_LEAD_IN_TRACKS + MAX_TOTAL_TRACKS)
		return SLIDER_OUTSIDE_CHD;
	else if (m_curtrack < m_maxtrack - 1)
		return SLIDER_VIRTUAL_LEADOUT;
	else
		return SLIDER_MAXIMUM;
}


//-------------------------------------------------
//  generic_update - generically update in a way
//  that works for most situations
//-------------------------------------------------

INT32 laserdisc_device::generic_update(const vbi_metadata &vbi, int fieldnum, const attotime &curtime, player_state_info &newstate)
{
	INT32 advanceby = 0;
	int frame;

	// start by assuming the state doesn't change
	newstate = m_player_state;

	// handle things based on the state
	switch (m_player_state.m_state)
	{
		case LDSTATE_EJECTING:
			// when time expires, switch to the ejected state
			if (curtime >= m_player_state.m_endtime)
				newstate.m_state = LDSTATE_EJECTED;
			break;

		case LDSTATE_EJECTED:
			// do nothing
			break;

		case LDSTATE_PARKED:
			// do nothing
			break;

		case LDSTATE_LOADING:
			// when time expires, switch to the spinup state
			if (curtime >= m_player_state.m_endtime)
				newstate.m_state = LDSTATE_SPINUP;
			advanceby = -GENERIC_SEARCH_SPEED;
			break;

		case LDSTATE_SPINUP:
			// when time expires, switch to the playing state
			if (curtime >= m_player_state.m_endtime)
				newstate.m_state = LDSTATE_PLAYING;
			advanceby = -GENERIC_SEARCH_SPEED;
			break;

		case LDSTATE_PAUSING:
			// if he hit the start of a frame, switch to paused state
			if (is_start_of_frame(vbi))
			{
				newstate.m_state = LDSTATE_PAUSED;
				newstate.m_param = fieldnum;
			}

			// else advance until we hit it
			else if (fieldnum == 1)
				advanceby = 1;
			break;

		case LDSTATE_PAUSED:
			// if we paused on field 1, we must flip back and forth
			if (m_player_state.m_param == 1)
				advanceby = (fieldnum == 1) ? 1 : -1;
			break;

		case LDSTATE_PLAYING:
			// if we hit the target frame, switch to the paused state
			if (m_player_state.m_param > 0 && is_start_of_frame(vbi) && frame_from_metadata(vbi) == m_player_state.m_param)
			{
				newstate.m_state = LDSTATE_PAUSED;
				newstate.m_param = fieldnum;
			}

			// otherwise after the second field of each frame
			else if (fieldnum == 1)
				advanceby = 1;
			break;

		case LDSTATE_PLAYING_SLOW_REVERSE:
			// after the second field of each frame, see if we need to advance
			if (fieldnum == 1 && ++m_player_state.m_substate > m_player_state.m_param)
			{
				advanceby = -1;
				m_player_state.m_substate = 0;
			}
			break;

		case LDSTATE_PLAYING_SLOW_FORWARD:
			// after the second field of each frame, see if we need to advance
			if (fieldnum == 1 && ++m_player_state.m_substate > m_player_state.m_param)
			{
				advanceby = 1;
				m_player_state.m_substate = 0;
			}
			break;

		case LDSTATE_PLAYING_FAST_REVERSE:
			// advance after the second field of each frame
			if (fieldnum == 1)
				advanceby = -m_player_state.m_param;
			break;

		case LDSTATE_PLAYING_FAST_FORWARD:
			// advance after the second field of each frame
			if (fieldnum == 1)
				advanceby = m_player_state.m_param;
			break;

		case LDSTATE_SCANNING:
			// advance after the second field of each frame
			if (fieldnum == 1)
				advanceby = m_player_state.m_param >> 8;

			// after we run out of vsyncs, revert to the saved state
			if (++m_player_state.m_substate >= (m_player_state.m_param & 0xff))
				newstate = m_saved_state;
			break;

		case LDSTATE_STEPPING_REVERSE:
			// wait for the first field of the frame and then leap backwards
			if (is_start_of_frame(vbi))
			{
				advanceby = (fieldnum == 1) ? -1 : -2;
				newstate.m_state = LDSTATE_PAUSING;
			}
			break;

		case LDSTATE_STEPPING_FORWARD:
			// wait for the first field of the frame and then switch to pausing state
			if (is_start_of_frame(vbi))
				newstate.m_state = LDSTATE_PAUSING;
			break;

		case LDSTATE_SEEKING:
			// if we're in the final state, look for a matching frame and pause there
			frame = frame_from_metadata(vbi);
			if (m_player_state.m_substate == 1 && is_start_of_frame(vbi) && frame == m_player_state.m_param)
			{
				newstate.m_state = LDSTATE_PAUSED;
				newstate.m_param = fieldnum;
			}

			// otherwise, if we got frame data from the VBI, update our seeking logic
			else if (m_player_state.m_substate == 0 && frame != FRAME_NOT_PRESENT)
			{
				INT32 delta = (m_player_state.m_param - 2) - frame;

				// if we're within a couple of frames, just play until we hit it
				if (delta >= 0 && delta <= 2)
					m_player_state.m_substate++;

				// otherwise, compute the delta assuming 1:1 track to frame; this will correct eventually
				else
				{
					if (delta < 0)
						delta--;
					advanceby = delta;
					advanceby = MIN(advanceby, GENERIC_SEARCH_SPEED);
					advanceby = MAX(advanceby, -GENERIC_SEARCH_SPEED);
				}
			}

			// otherwise, keep advancing until we know what's up
			else
			{
				if (fieldnum == 1)
					advanceby = 1;
			}
			break;

		default:
			// do nothing
			break;
	}

	return advanceby;
}


//**************************************************************************
//  INITIALIZATION
//**************************************************************************

//-------------------------------------------------
//  init_disc - initialize the state of the
//  CHD disc
//-------------------------------------------------

void laserdisc_device::init_disc()
{
	// get a handle to the disc to play
	if (!m_getdisc_callback.isnull())
		m_disc = m_getdisc_callback(*this);
	else
		m_disc = get_disk_handle(machine(), tag());

	// set default parameters
	m_width = 720;
	m_height = 240;
	m_fps_times_1million = 59940000;
	m_samplerate = 48000;

	// get the disc metadata and extract the ld
	m_chdtracks = 0;
	m_maxtrack = VIRTUAL_LEAD_IN_TRACKS + MAX_TOTAL_TRACKS + VIRTUAL_LEAD_OUT_TRACKS;
	if (m_disc != nullptr)
	{
		// require the A/V codec and nothing else
		if (m_disc->compression(0) != CHD_CODEC_AVHUFF || m_disc->compression(1) != CHD_CODEC_NONE)
			throw emu_fatalerror("Laserdisc video must be compressed with the A/V codec!");

		// read the metadata
		std::string metadata;
		chd_error err = m_disc->read_metadata(AV_METADATA_TAG, 0, metadata);
		if (err != CHDERR_NONE)
			throw emu_fatalerror("Non-A/V CHD file specified");

		// extract the metadata
		int fps, fpsfrac, interlaced, channels;
		if (sscanf(metadata.c_str(), AV_METADATA_FORMAT, &fps, &fpsfrac, &m_width, &m_height, &interlaced, &channels, &m_samplerate) != 7)
			throw emu_fatalerror("Invalid metadata in CHD file");
		else
			m_fps_times_1million = fps * 1000000 + fpsfrac;

		// require interlaced video
		if (!interlaced)
			throw emu_fatalerror("Laserdisc video must be interlaced!");

		// determine the maximum track and allocate a frame buffer
		UINT32 totalhunks = m_disc->hunk_count();
		m_chdtracks = totalhunks / 2;

		// allocate memory for the precomputed per-frame metadata
		err = m_disc->read_metadata(AV_LD_METADATA_TAG, 0, m_vbidata);
		if (err != CHDERR_NONE || m_vbidata.size() != totalhunks * VBI_PACKED_BYTES)
			throw emu_fatalerror("Precomputed VBI metadata missing or incorrect size");
	}
	m_maxtrack = MAX(m_maxtrack, VIRTUAL_LEAD_IN_TRACKS + VIRTUAL_LEAD_OUT_TRACKS + m_chdtracks);
}


//-------------------------------------------------
//  init_video - initialize the state of the
//  video rendering
//-------------------------------------------------

void laserdisc_device::init_video()
{
	// register for VBLANK callbacks
	m_screen->register_vblank_callback(vblank_state_delegate(FUNC(laserdisc_device::vblank_state_changed), this));

	// allocate palette for applying brightness/contrast/gamma
	m_videopalette = palette_t::alloc(256);
	if (m_videopalette == nullptr)
		throw emu_fatalerror("Out of memory allocating video palette");
	for (int index = 0; index < 256; index++)
		m_videopalette->entry_set_color(index, rgb_t(index, index, index));

	// allocate video frames
	for (auto & frame : m_frame)
	{
		// first allocate a YUY16 bitmap at 2x the height

		frame.m_bitmap.allocate(m_width, m_height * 2);
		frame.m_bitmap.set_palette(m_videopalette);
		fillbitmap_yuy16(frame.m_bitmap, 40, 109, 240);

		// make a copy of the bitmap that clips out the VBI and horizontal blanking areas
		frame.m_visbitmap.wrap(&frame.m_bitmap.pix16(44, frame.m_bitmap.width() * 8 / 720),
								frame.m_bitmap.width() - 2 * frame.m_bitmap.width() * 8 / 720,
								frame.m_bitmap.height() - 44,
								frame.m_bitmap.rowpixels());
		frame.m_visbitmap.set_palette(m_videopalette);
	}

	// allocate an empty frame of the same size
	m_emptyframe.allocate(m_width, m_height * 2);
	m_emptyframe.set_palette(m_videopalette);
	fillbitmap_yuy16(m_emptyframe, 0, 128, 128);

	// allocate texture for rendering
	m_videoenable = true;
	m_videotex = machine().render().texture_alloc();
	if (m_videotex == nullptr)
		fatalerror("Out of memory allocating video texture\n");

	// allocate overlay
	m_overenable = overlay_configured();
	if (m_overenable)
	{
		// bind our handlers
		m_overupdate_ind16.bind_relative_to(*owner());
		m_overupdate_rgb32.bind_relative_to(*owner());

		// configure bitmap formats
		bitmap_format format = !m_overupdate_ind16.isnull() ? BITMAP_FORMAT_IND16 : BITMAP_FORMAT_RGB32;
		texture_format texformat = !m_overupdate_ind16.isnull() ? TEXFORMAT_PALETTEA16 : TEXFORMAT_ARGB32;

		// allocate overlay bitmaps
		for (auto & elem : m_overbitmap)
		{
			elem.set_format(format, texformat);
			if (format==BITMAP_FORMAT_IND16)
				elem.set_palette(m_overlay_palette->palette());
			elem.resize(m_overwidth, m_overheight);
		}

		// allocate overlay texture
		m_overtex = machine().render().texture_alloc();
		if (m_overtex == nullptr)
			fatalerror("Out of memory allocating overlay texture\n");
	}
}


//-------------------------------------------------
//  init_audio - initialize the state of the
//  audio rendering
//-------------------------------------------------

void laserdisc_device::init_audio()
{
	// allocate a stream
	m_stream = stream_alloc(0, 2, 48000);

	// allocate audio buffers
	m_audiomaxsamples = ((UINT64)m_samplerate * 1000000 + m_fps_times_1million - 1) / m_fps_times_1million;
	m_audiobufsize = m_audiomaxsamples * 4;
	m_audiobuffer[0].resize(m_audiobufsize);
	m_audiobuffer[1].resize(m_audiobufsize);
}


//**************************************************************************
//  INTERNAL HELPERS
//**************************************************************************

//-------------------------------------------------
//  fillbitmap_yuy16 - fill a YUY16 bitmap with a
//  given color pattern
//-------------------------------------------------

void laserdisc_device::fillbitmap_yuy16(bitmap_yuy16 &bitmap, UINT8 yval, UINT8 cr, UINT8 cb)
{
	UINT16 color0 = (yval << 8) | cb;
	UINT16 color1 = (yval << 8) | cr;

	// write 32 bits of color (2 pixels at a time)
	for (int y = 0; y < bitmap.height(); y++)
	{
		UINT16 *dest = &bitmap.pix16(y);
		for (int x = 0; x < bitmap.width() / 2; x++)
		{
			*dest++ = color0;
			*dest++ = color1;
		}
	}
}


//-------------------------------------------------
//  update_slider_pos - based on the current
//  speed and elapsed time, update the current
//  track position
//-------------------------------------------------

void laserdisc_device::update_slider_pos()
{
	attotime curtime = machine().time();

	// if not moving, update to now
	if (m_attospertrack == 0)
		m_sliderupdate = curtime;

	// otherwise, compute the number of tracks covered
	else
	{
		attoseconds_t delta = (curtime - m_sliderupdate).as_attoseconds();

		// determine how many tracks we covered and advance
		if (m_attospertrack >= 0)
		{
			INT32 tracks_covered = delta / m_attospertrack;
			add_and_clamp_track(tracks_covered);
			if (tracks_covered != 0)
				m_sliderupdate += attotime(0, tracks_covered * m_attospertrack);
		}
		else
		{
			INT32 tracks_covered = delta / -m_attospertrack;
			add_and_clamp_track(-tracks_covered);
			if (tracks_covered != 0)
				m_sliderupdate += attotime(0, tracks_covered * -m_attospertrack);
		}
	}
}


//-------------------------------------------------
//  vblank_state_changed - called on each state
//  change of the VBLANK signal
//-------------------------------------------------

void laserdisc_device::vblank_state_changed(screen_device &screen, bool vblank_state)
{
	// update current track based on slider speed
	update_slider_pos();

	// on rising edge, process previously-read frame and inform the player
	if (vblank_state)
	{
		// call the player's VSYNC callback
		player_vsync(m_metadata[m_fieldnum], m_fieldnum, machine().time());

		// set a timer to begin fetching the next frame just before the VBI data would be fetched
		timer_set(screen.time_until_pos(16*2), TID_VBI_FETCH);
	}
}


//-------------------------------------------------
//  current_frame - return a reference to the
//  currently visible frame
//-------------------------------------------------

laserdisc_device::frame_data &laserdisc_device::current_frame()
{
	// determine the most recent live set of frames
	frame_data *frame = &m_frame[m_videoindex];
	if (frame->m_numfields < 2)
		frame = &m_frame[(m_videoindex + ARRAY_LENGTH(m_frame) - 1) % ARRAY_LENGTH(m_frame)];
	return *frame;
}


//-------------------------------------------------
//  read_track_data - read and process data for
//  a particular video track
//-------------------------------------------------

void laserdisc_device::read_track_data()
{
	// compute the chdhunk number we are going to read
	INT32 chdtrack = m_curtrack - 1 - VIRTUAL_LEAD_IN_TRACKS;
	chdtrack = MAX(chdtrack, 0);
	chdtrack = MIN(chdtrack, m_chdtracks - 1);
	UINT32 readhunk = chdtrack * 2 + m_fieldnum;

	// cheat and look up the metadata we are about to retrieve
	vbi_metadata vbidata = { 0 };
	if (!m_vbidata.empty())
		vbi_metadata_unpack(&vbidata, nullptr, &m_vbidata[readhunk * VBI_PACKED_BYTES]);

	// if we're in the lead-in area, force the VBI data to be standard lead-in
	if (m_curtrack - 1 < VIRTUAL_LEAD_IN_TRACKS)
	{
		vbidata.line16 = 0;
		vbidata.line17 = vbidata.line18 = vbidata.line1718 = VBI_CODE_LEADIN;
	}
//printf("track %5d.%d: %06X %06X %06X\n", m_curtrack, m_fieldnum, vbidata.line16, vbidata.line17, vbidata.line18);

	// if we're about to read the first field in a frame, advance
	frame_data *frame = &m_frame[m_videoindex];
	if ((vbidata.line1718 & VBI_MASK_CAV_PICTURE) == VBI_CODE_CAV_PICTURE)
	{
		if (frame->m_numfields >= 2)
			m_videoindex = (m_videoindex + 1) % ARRAY_LENGTH(m_frame);
		frame = &m_frame[m_videoindex];
		frame->m_numfields = 0;
	}

	// if we're squelched, reset the frame counter
	if (m_videosquelch)
		frame->m_numfields = 0;

	// remember the last field number
	frame->m_lastfield = m_curtrack * 2 + m_fieldnum;

	// set the video target information
	m_avhuff_config.video.wrap(&frame->m_bitmap.pix16(m_fieldnum), frame->m_bitmap.width(), frame->m_bitmap.height() / 2, frame->m_bitmap.rowpixels() * 2);

	// set the audio target information
	if (m_audiobufin + m_audiomaxsamples <= m_audiobufsize)
	{
		// if we can fit without wrapping, just read the data directly
		m_avhuff_config.audio[0] = &m_audiobuffer[0][m_audiobufin];
		m_avhuff_config.audio[1] = &m_audiobuffer[1][m_audiobufin];
	}
	else
	{
		// otherwise, read to the beginning of the buffer
		m_avhuff_config.audio[0] = &m_audiobuffer[0][0];
		m_avhuff_config.audio[1] = &m_audiobuffer[1][0];
	}

	// override if we're not decoding
	m_avhuff_config.maxsamples = m_audiomaxsamples;
	m_avhuff_config.actsamples = &m_audiocursamples;
	m_audiocursamples = 0;

	// set the VBI data for the new field from our precomputed data
	if (!m_vbidata.empty())
	{
		UINT32 vbiframe;
		vbi_metadata_unpack(&m_metadata[m_fieldnum], &vbiframe, &m_vbidata[readhunk * VBI_PACKED_BYTES]);
	}

	// if we're in the lead-in area, force the VBI data to be standard lead-in
	if (m_curtrack - 1 < VIRTUAL_LEAD_IN_TRACKS)
	{
		m_metadata[m_fieldnum].line16 = 0;
		m_metadata[m_fieldnum].line17 = m_metadata[m_fieldnum].line18 = m_metadata[m_fieldnum].line1718 = VBI_CODE_LEADIN;
	}

	// configure the codec and then read
	m_readresult = CHDERR_FILE_NOT_FOUND;
	if (m_disc != nullptr && !m_videosquelch)
	{
		m_readresult = m_disc->codec_configure(CHD_CODEC_AVHUFF, AVHUFF_CODEC_DECOMPRESS_CONFIG, &m_avhuff_config);
		if (m_readresult == CHDERR_NONE)
		{
			m_queued_hunknum = readhunk;
			m_readresult = CHDERR_OPERATION_PENDING;
			osd_work_item_queue(m_work_queue, read_async_static, this, WORK_ITEM_FLAG_AUTO_RELEASE);
		}
	}
}


//-------------------------------------------------
//  read_async_static - work item callback for
//  asynchronous reads
//-------------------------------------------------

void *laserdisc_device::read_async_static(void *param, int threadid)
{
	laserdisc_device &ld = *reinterpret_cast<laserdisc_device *>(param);
	ld.m_readresult = ld.m_disc->read_hunk(ld.m_queued_hunknum, nullptr);
	return nullptr;
}


//-------------------------------------------------
//  process_track_data - process data from a
//  track after it has been read
//-------------------------------------------------

void laserdisc_device::process_track_data()
{
	// wait for the async operation to complete
	if (m_readresult == CHDERR_OPERATION_PENDING)
		osd_work_queue_wait(m_work_queue, osd_ticks_per_second() * 10);
	assert(m_readresult != CHDERR_OPERATION_PENDING);

	// remove the video if we had an error
	if (m_readresult != CHDERR_NONE)
		m_avhuff_config.video.reset();

	// count the field as read if we are successful
	if (m_avhuff_config.video.valid())
	{
		m_frame[m_videoindex].m_numfields++;
		player_overlay(m_avhuff_config.video);
	}

	// pass the audio to the callback
	if (!m_audio_callback.isnull())
		m_audio_callback(*this, m_samplerate, m_audiocursamples, m_avhuff_config.audio[0], m_avhuff_config.audio[1]);

	// shift audio data if we read it into the beginning of the buffer
	if (m_audiocursamples != 0 && m_audiobufin != 0)
		for (int chnum = 0; chnum < 2; chnum++)
			if (m_avhuff_config.audio[chnum] == &m_audiobuffer[chnum][0])
			{
				// move data to the end
				int samplesleft = m_audiobufsize - m_audiobufin;
				samplesleft = MIN(samplesleft, m_audiocursamples);
				memmove(&m_audiobuffer[chnum][m_audiobufin], &m_audiobuffer[chnum][0], samplesleft * 2);

				// shift data at the beginning
				if (samplesleft < m_audiocursamples)
					memmove(&m_audiobuffer[chnum][0], &m_audiobuffer[chnum][samplesleft], (m_audiocursamples - samplesleft) * 2);
			}

	// update the input buffer pointer
	m_audiobufin = (m_audiobufin + m_audiocursamples) % m_audiobufsize;
}



//**************************************************************************
//  CONFIG SETTINGS ACCESS
//**************************************************************************

//-------------------------------------------------
//  config_load - read and apply data from the
//  configuration file
//-------------------------------------------------

void laserdisc_device::config_load(int config_type, xml_data_node *parentnode)
{
	// we only care about game files
	if (config_type != CONFIG_TYPE_GAME)
		return;

	// might not have any data
	if (parentnode == nullptr)
		return;

	// iterate over overlay nodes
	for (xml_data_node *ldnode = xml_get_sibling(parentnode->child, "device"); ldnode != nullptr; ldnode = xml_get_sibling(ldnode->next, "device"))
	{
		const char *devtag = xml_get_attribute_string(ldnode, "tag", "");
		if (strcmp(devtag, tag()) == 0)
		{
			// handle the overlay node
			xml_data_node *overnode = xml_get_sibling(ldnode->child, "overlay");
			if (overnode != nullptr)
			{
				// fetch positioning controls
				m_overposx = xml_get_attribute_float(overnode, "hoffset", m_overposx);
				m_overscalex = xml_get_attribute_float(overnode, "hstretch", m_overscalex);
				m_overposy = xml_get_attribute_float(overnode, "voffset", m_overposy);
				m_overscaley = xml_get_attribute_float(overnode, "vstretch", m_overscaley);
			}
		}
	}
}


//-------------------------------------------------
//  config_save - save data to the configuration
//  file
//-------------------------------------------------

void laserdisc_device::config_save(int config_type, xml_data_node *parentnode)
{
	// we only care about game files
	if (config_type != CONFIG_TYPE_GAME)
		return;

	// create a node
	xml_data_node *ldnode = xml_add_child(parentnode, "device", nullptr);
	if (ldnode != nullptr)
	{
		// output the basics
		xml_set_attribute(ldnode, "tag", tag());

		// add an overlay node
		xml_data_node *overnode = xml_add_child(ldnode, "overlay", nullptr);
		bool changed = false;
		if (overnode != nullptr)
		{
			// output the positioning controls
			if (m_overposx != m_orig_config.m_overposx)
			{
				xml_set_attribute_float(overnode, "hoffset", m_overposx);
				changed = true;
			}

			if (m_overscalex != m_orig_config.m_overscalex)
			{
				xml_set_attribute_float(overnode, "hstretch", m_overscalex);
				changed = true;
			}

			if (m_overposy != m_orig_config.m_overposy)
			{
				xml_set_attribute_float(overnode, "voffset", m_overposy);
				changed = true;
			}

			if (m_overscaley != m_orig_config.m_overscaley)
			{
				xml_set_attribute_float(overnode, "vstretch", m_overscaley);
				changed = true;
			}
		}

		// if nothing changed, kill the node
		if (!changed)
			xml_delete_node(ldnode);
	}
}
