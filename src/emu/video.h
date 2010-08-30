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

// screen types
enum screen_type_enum
{
	SCREEN_TYPE_INVALID = 0,
	SCREEN_TYPE_RASTER,
	SCREEN_TYPE_VECTOR,
	SCREEN_TYPE_LCD
};



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// forward references
class render_target;
struct render_texture;
class screen_device;


// callback that is called to notify of a change in the VBLANK state
typedef void (*vblank_state_changed_func)(screen_device &device, void *param, bool vblank_state);


// ======================> screen_device_config

class screen_device_config : public device_config
{
	friend class screen_device;

	// construction/destruction
	screen_device_config(const machine_config &mconfig, const char *tag, const device_config *owner, UINT32 clock);

public:
	// allocators
	static device_config *static_alloc_device_config(const machine_config &mconfig, const char *tag, const device_config *owner, UINT32 clock);
	virtual device_t *alloc_device(running_machine &machine) const;

	// configuration readers
	screen_type_enum screen_type() const { return m_type; }
	int width() const { return m_width; }
	int height() const { return m_height; }
	const rectangle &visible_area() const { return m_visarea; }
	bool oldstyle_vblank_supplied() const { return m_oldstyle_vblank_supplied; }
	attoseconds_t refresh() const { return m_refresh; }
	attoseconds_t vblank() const { return m_vblank; }
	bitmap_format format() const { return m_format; }
	float xoffset() const { return m_xoffset; }
	float yoffset() const { return m_yoffset; }
	float xscale() const { return m_xscale; }
	float yscale() const { return m_yscale; }

	// inline configuration helpers
	static void static_set_format(device_config *device, bitmap_format format);
	static void static_set_type(device_config *device, screen_type_enum type);
	static void static_set_raw(device_config *device, UINT32 pixclock, UINT16 htotal, UINT16 hbend, UINT16 hbstart, UINT16 vtotal, UINT16 vbend, UINT16 vbstart);
	static void static_set_refresh(device_config *device, attoseconds_t rate);
	static void static_set_vblank_time(device_config *device, attoseconds_t time);
	static void static_set_size(device_config *device, UINT16 width, UINT16 height);
	static void static_set_visarea(device_config *device, INT16 minx, INT16 maxx, INT16 miny, INT16 maxy);
	static void static_set_default_position(device_config *device, double xscale, double xoffs, double yscale, double yoffs);

private:
	// device_config overrides
	virtual bool device_validity_check(const game_driver &driver) const;

	// inline configuration data
	screen_type_enum	m_type;						// type of screen
	int					m_width, m_height;			// default total width/height (HTOTAL, VTOTAL)
	rectangle			m_visarea;					// default visible area (HBLANK end/start, VBLANK end/start)
	bool				m_oldstyle_vblank_supplied;	// MDRV_SCREEN_VBLANK_TIME macro used
	attoseconds_t		m_refresh;					// default refresh period
	attoseconds_t		m_vblank;					// duration of a VBLANK
	bitmap_format		m_format;					// bitmap format
	float				m_xoffset, m_yoffset;		// default X/Y offsets
	float				m_xscale, m_yscale;			// default X/Y scale factor
};



// ======================> screen_device

class screen_device : public device_t
{
	friend class screen_device_config;
	friend resource_pool_object<screen_device>::~resource_pool_object();

	// construction/destruction
	screen_device(running_machine &_machine, const screen_device_config &config);
	virtual ~screen_device();

public:
	// information getters
	const screen_device_config &config() const { return m_config; }
	screen_type_enum screen_type() const { return m_config.m_type; }
	int width() const { return m_width; }
	int height() const { return m_height; }
	const rectangle &visible_area() const { return m_visarea; }
	bitmap_format format() const { return m_config.m_format; }

	// dynamic configuration
	void configure(int width, int height, const rectangle &visarea, attoseconds_t frame_period);
	void set_visible_area(int min_x, int max_x, int min_y, int max_y);

	// beam positioning and state
	int vpos() const;
	int hpos() const;
	bool vblank() const { return attotime_compare(timer_get_time(machine), m_vblank_end_time) < 0; }
	bool hblank() const { int curpos = hpos(); return (curpos < m_visarea.min_x || curpos > m_visarea.max_x); }

	// timing
	attotime time_until_pos(int vpos, int hpos = 0) const;
	attotime time_until_vblank_start() const { return time_until_pos(m_visarea.max_y + 1); }
	attotime time_until_vblank_end() const;
	attotime time_until_update() const { return (machine->config->m_video_attributes & VIDEO_UPDATE_AFTER_VBLANK) ? time_until_vblank_end() : time_until_vblank_start(); }
	attotime scan_period() const { return attotime_make(0, m_scantime); }
	attotime frame_period() const { return (this == NULL || !started()) ? k_default_frame_period : attotime_make(0, m_frame_period); };
	UINT64 frame_number() const { return m_frame_number; }

	// updating
	bool update_partial(int scanline);
	void update_now();

	// additional helpers
	void save_snapshot(mame_file *fp);
	void register_vblank_callback(vblank_state_changed_func vblank_callback, void *param);
	bitmap_t *alloc_compatible_bitmap(int width = 0, int height = 0) { return auto_bitmap_alloc(machine, (width == 0) ? m_width : width, (height == 0) ? m_height : height, m_config.m_format); }

	// internal to the video system
	bool update_quads();
	void update_burnin();

	// globally accessible constants
	static const int k_default_frame_rate = 60;
	static const attotime k_default_frame_period;

private:
	// device-level overrides
	virtual void device_start();
	virtual void device_post_load();

	// internal helpers
	void realloc_screen_bitmaps();

	static TIMER_CALLBACK( static_vblank_begin_callback ) { reinterpret_cast<screen_device *>(ptr)->vblank_begin_callback(); }
	void vblank_begin_callback();

	static TIMER_CALLBACK( static_vblank_end_callback ) { reinterpret_cast<screen_device *>(ptr)->vblank_end_callback(); }
	void vblank_end_callback();

	static TIMER_CALLBACK( static_scanline0_callback ) { reinterpret_cast<screen_device *>(ptr)->scanline0_callback(); }
public:	// temporary
	void scanline0_callback();
private:

	static TIMER_CALLBACK( static_scanline_update_callback ) { reinterpret_cast<screen_device *>(ptr)->scanline_update_callback(param); }
	void scanline_update_callback(int scanline);

	void finalize_burnin();

	// internal state
	const screen_device_config &m_config;

	// dimensions
	int						m_width;				// current width (HTOTAL)
	int						m_height;				// current height (VTOTAL)
	rectangle				m_visarea;				// current visible area (HBLANK end/start, VBLANK end/start)

	// textures and bitmaps
	render_texture *		m_texture[2];			// 2x textures for the screen bitmap
	bitmap_t *				m_bitmap[2];			// 2x bitmaps for rendering
	bitmap_t *				m_burnin;				// burn-in bitmap
	UINT8					m_curbitmap;			// current bitmap index
	UINT8					m_curtexture;			// current texture index
	INT32					m_texture_format;		// texture format of bitmap for this screen
	bool					m_changed;				// has this bitmap changed?
	INT32					m_last_partial_scan;	// scanline of last partial update

	// screen timing
	attoseconds_t			m_frame_period;			// attoseconds per frame
	attoseconds_t			m_scantime;				// attoseconds per scanline
	attoseconds_t			m_pixeltime;			// attoseconds per pixel
	attoseconds_t			m_vblank_period;		// attoseconds per VBLANK period
	attotime				m_vblank_start_time;	// time of last VBLANK start
	attotime				m_vblank_end_time;		// time of last VBLANK end
	emu_timer *				m_vblank_begin_timer;	// timer to signal VBLANK start
	emu_timer *				m_vblank_end_timer;		// timer to signal VBLANK end
	emu_timer *				m_scanline0_timer;		// scanline 0 timer
	emu_timer *				m_scanline_timer;		// scanline timer
	UINT64					m_frame_number;			// the current frame number

	struct callback_item
	{
		callback_item *				m_next;
		vblank_state_changed_func	m_callback;
		void *						m_param;
	};
	callback_item *			m_callback_list;		// list of VBLANK callbacks
};


// device type definition
extern const device_type SCREEN;



//**************************************************************************
//  SCREEN DEVICE CONFIGURATION MACROS
//**************************************************************************

#define MDRV_SCREEN_ADD(_tag, _type) \
	MDRV_DEVICE_ADD(_tag, SCREEN, 0) \
	MDRV_SCREEN_TYPE(_type) \

#define MDRV_SCREEN_MODIFY(_tag) \
	MDRV_DEVICE_MODIFY(_tag)

#define MDRV_SCREEN_FORMAT(_format) \
	screen_device_config::static_set_format(device, _format); \

#define MDRV_SCREEN_TYPE(_type) \
	screen_device_config::static_set_type(device, SCREEN_TYPE_##_type); \

#define MDRV_SCREEN_RAW_PARAMS(_pixclock, _htotal, _hbend, _hbstart, _vtotal, _vbend, _vbstart) \
	screen_device_config::static_set_raw(device, _pixclock, _htotal, _hbend, _hbstart, _vtotal, _vbend, _vbstart);

#define MDRV_SCREEN_REFRESH_RATE(_rate) \
	screen_device_config::static_set_refresh(device, HZ_TO_ATTOSECONDS(_rate)); \

#define MDRV_SCREEN_VBLANK_TIME(_time) \
	screen_device_config::static_set_vblank_time(device, _time); \

#define MDRV_SCREEN_SIZE(_width, _height) \
	screen_device_config::static_set_size(device, _width, _height); \

#define MDRV_SCREEN_VISIBLE_AREA(_minx, _maxx, _miny, _maxy) \
	screen_device_config::static_set_visarea(device, _minx, _maxx, _miny, _maxy); \

#define MDRV_SCREEN_DEFAULT_POSITION(_xscale, _xoffs, _yscale, _yoffs)	\
	screen_device_config::static_set_default_position(device, _xscale, _xoffs, _yscale, _yoffs); \



//**************************************************************************
//  FUNCTION PROTOTYPES
//**************************************************************************

/* ----- core implementation ----- */

/* core initialization */
void video_init(running_machine *machine);


/* ----- global rendering ----- */

/* update the screen, handling frame skipping and rendering */
void video_frame_update(running_machine *machine, int debug);


/* ----- throttling/frameskipping/performance ----- */

/* are we skipping the current frame? */
int video_skip_this_frame(void);

/* get/set the speed factor as an integer * 100 */
int video_get_speed_factor(void);
void video_set_speed_factor(int speed);

/* return text to display about the current speed */
const char *video_get_speed_text(running_machine *machine);

/* return the current effective speed percentage */
double video_get_speed_percent(running_machine *machine);

/* get/set the current frameskip (-1 means auto) */
int video_get_frameskip(void);
void video_set_frameskip(int frameskip);

/* get/set the current throttle */
int video_get_throttle(void);
void video_set_throttle(int throttle);

/* get/set the current fastforward state */
int video_get_fastforward(void);
void video_set_fastforward(int fastforward);


/* ----- snapshots ----- */

/* save a snapshot of a given screen */
void screen_save_snapshot(running_machine *machine, device_t *screen, mame_file *fp);

/* save a snapshot of all the active screens */
void video_save_active_screen_snapshots(running_machine *machine);


/* ----- movie recording ----- */

int video_mng_is_movie_active(running_machine *machine);
void video_mng_begin_recording(running_machine *machine, const char *name);
void video_mng_end_recording(running_machine *machine);

void video_avi_begin_recording(running_machine *machine, const char *name);
void video_avi_end_recording(running_machine *machine);
void video_avi_add_sound(running_machine *machine, const INT16 *sound, int numsamples);


/* ----- configuration helpers ----- */

/* select a view for a given target */
int video_get_view_for_target(running_machine *machine, render_target *target, const char *viewname, int targetindex, int numtargets);


/* ----- debugging helpers ----- */

/* assert if any pixels in the given bitmap contain an invalid palette index */
void video_assert_out_of_range_pixels(running_machine *machine, bitmap_t *bitmap);



//**************************************************************************
//  INLINE HELPERS
//**************************************************************************

//-------------------------------------------------
//  screen_count - return the number of
//  video screen devices in a machine_config
//-------------------------------------------------

inline int screen_count(const machine_config &config)
{
	return config.m_devicelist.count(SCREEN);
}


//-------------------------------------------------
//  screen_first - return the first
//  video screen device config in a machine_config
//-------------------------------------------------

inline const screen_device_config *screen_first(const machine_config &config)
{
	return downcast<screen_device_config *>(config.m_devicelist.first(SCREEN));
}


//-------------------------------------------------
//  screen_next - return the next
//  video screen device config in a machine_config
//-------------------------------------------------

inline const screen_device_config *screen_next(const screen_device_config *previous)
{
	return downcast<screen_device_config *>(previous->typenext());
}


//-------------------------------------------------
//  screen_count - return the number of
//  video screen devices in a machine
//-------------------------------------------------

inline int screen_count(running_machine &machine)
{
	return machine.m_devicelist.count(SCREEN);
}


//-------------------------------------------------
//  screen_first - return the first
//  video screen device in a machine
//-------------------------------------------------

inline screen_device *screen_first(running_machine &machine)
{
	return downcast<screen_device *>(machine.m_devicelist.first(SCREEN));
}


//-------------------------------------------------
//  screen_next - return the next
//  video screen device in a machine
//-------------------------------------------------

inline screen_device *screen_next(screen_device *previous)
{
	return downcast<screen_device *>(previous->typenext());
}


#endif	/* __VIDEO_H__ */
