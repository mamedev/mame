/***************************************************************************

    screen.h

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

#pragma once

#ifndef __EMU_H__
#error Dont include this file directly; include emu.h instead.
#endif

#ifndef __SCREEN_H__
#define __SCREEN_H__


//**************************************************************************
//  CONSTANTS
//**************************************************************************

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
class render_texture;
class screen_device;


// ======================> screen_bitmap

class screen_bitmap
{
private:
	// internal helpers
	bitmap_t &appropriate_bitmap()
	{
		switch (m_format)
		{
			case BITMAP_FORMAT_IND16: return m_ind16;
			case BITMAP_FORMAT_RGB32: return m_rgb32;
			default: throw emu_fatalerror("Invalid screen_bitmap format");
		}
	}

	const bitmap_t &appropriate_bitmap() const
	{
		switch (m_format)
		{
			case BITMAP_FORMAT_IND16: return m_ind16;
			case BITMAP_FORMAT_RGB32: return m_rgb32;
			default: throw emu_fatalerror("Invalid screen_bitmap format");
		}
	}

public:
	// construction/destruction
	screen_bitmap()
		: m_format(BITMAP_FORMAT_INVALID),
		  m_texformat(TEXFORMAT_UNDEFINED) { }
	screen_bitmap(bitmap_ind16 &orig)
		: m_format(BITMAP_FORMAT_IND16),
		  m_texformat(TEXFORMAT_PALETTE16), 
		  m_ind16(orig, orig.cliprect()) { }
	screen_bitmap(bitmap_rgb32 &orig)
		: m_format(BITMAP_FORMAT_RGB32),
		  m_texformat(TEXFORMAT_RGB32), 
		  m_rgb32(orig, orig.cliprect()) { }
	
	// allocation
	void allocate(int width, int height)
	{
		switch (m_format)
		{
			case BITMAP_FORMAT_IND16: m_ind16.allocate(width, height); break;
			case BITMAP_FORMAT_RGB32: m_rgb32.allocate(width, height); break;
			default: throw emu_fatalerror("Invalid screen_bitmap format");
		}
	}

	// conversion
	operator bitmap_t &() { return appropriate_bitmap(); }
	bitmap_ind16 &as_ind16() { assert(m_format == BITMAP_FORMAT_IND16); return m_ind16; }
	bitmap_rgb32 &as_rgb32() { assert(m_format == BITMAP_FORMAT_RGB32); return m_rgb32; }
	
	// getters
	INT32 width() const { return appropriate_bitmap().width(); }
	INT32 height() const { return appropriate_bitmap().height(); }
	INT32 rowpixels() const { return appropriate_bitmap().rowpixels(); }
	INT32 rowbytes() const { return appropriate_bitmap().rowbytes(); }
	UINT8 bpp() const { return appropriate_bitmap().bpp(); }
	bitmap_format format() const { return m_format; }
	texture_format texformat() const { return m_texformat; }
	bool valid() const { return appropriate_bitmap().valid(); }
	palette_t *palette() const { return appropriate_bitmap().palette(); }
	const rectangle &cliprect() const { return appropriate_bitmap().cliprect(); }

	// operations
	void set_palette(palette_t *palette) { appropriate_bitmap().set_palette(palette); }
	void set_format(bitmap_format format, texture_format texformat)
	{
		m_format = format;
		m_texformat = texformat;
		m_ind16.reset();
		m_rgb32.reset();
	}
	
private:
	// internal state
	bitmap_format		m_format;
	texture_format		m_texformat;
	bitmap_ind16	m_ind16;
	bitmap_rgb32		m_rgb32;
};


// ======================> screen_update_delegate

// composite "smart" delegate with late binding
class screen_update_delegate
{
public:
	// construction
	screen_update_delegate() { }

	screen_update_delegate(UINT32 (*callback)(screen_device *, screen_device &, bitmap_ind16 &, const rectangle &), const char *name)
		: m_ind16(callback, name, (screen_device *)0) { }

	screen_update_delegate(UINT32 (*callback)(screen_device *, screen_device &, bitmap_rgb32 &, const rectangle &), const char *name)
		: m_rgb32(callback, name, (screen_device *)0) { }
	
	template<class _FunctionClass>
	screen_update_delegate(UINT32 (_FunctionClass::*callback)(screen_device &, bitmap_ind16 &, const rectangle &), const char *name)
		: m_ind16(callback, name, (_FunctionClass *)0) { }

	template<class _FunctionClass>
	screen_update_delegate(UINT32 (_FunctionClass::*callback)(screen_device &, bitmap_rgb32 &, const rectangle &), const char *name)
		: m_rgb32(callback, name, (_FunctionClass *)0) { }

	// queries
	bool isnull() const { return m_ind16.isnull() && m_rgb32.isnull(); }
	bitmap_format format() const { return (!m_ind16.isnull()) ? BITMAP_FORMAT_IND16 : (!m_rgb32.isnull()) ? BITMAP_FORMAT_RGB32 : BITMAP_FORMAT_INVALID; }
	
	// binding
	void late_bind(delegate_late_bind &object)
	{ 
		if (!m_ind16.isnull()) m_ind16.late_bind(object);
		if (!m_rgb32.isnull()) m_rgb32.late_bind(object);
	}

	// calling
	UINT32 operator()(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &rectangle) const
	{
		return m_ind16.isnull() ? UPDATE_HAS_NOT_CHANGED : m_ind16(screen, bitmap, rectangle);
	}
	
	UINT32 operator()(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &rectangle) const
	{
		return m_rgb32.isnull() ? UPDATE_HAS_NOT_CHANGED : m_rgb32(screen, bitmap, rectangle);
	}

private:
	delegate<UINT32 (screen_device &, bitmap_ind16 &, const rectangle &)> m_ind16;
	delegate<UINT32 (screen_device &, bitmap_rgb32 &, const rectangle &)> m_rgb32;
};


// callback that is called to notify of a change in the VBLANK state
typedef delegate<void (screen_device &, bool)> vblank_state_delegate;

typedef delegate<void (screen_device &)> screen_eof_delegate;

typedef void (*screen_eof_func)(screen_device &screen);


// ======================> screen_device

class screen_device : public device_t
{
	friend class render_manager;

public:
	// construction/destruction
	screen_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	~screen_device();

	// configuration readers
	screen_type_enum screen_type() const { return m_type; }
	int width() const { return m_width; }
	int height() const { return m_height; }
	const rectangle &visible_area() const { return m_visarea; }
	bool oldstyle_vblank_supplied() const { return m_oldstyle_vblank_supplied; }
	attoseconds_t refresh_attoseconds() const { return m_refresh; }
	attoseconds_t vblank_attoseconds() const { return m_vblank; }
	bitmap_format format() const { return m_screen_update.format(); }
	float xoffset() const { return m_xoffset; }
	float yoffset() const { return m_yoffset; }
	float xscale() const { return m_xscale; }
	float yscale() const { return m_yscale; }
	bool have_screen_update() const { return !m_screen_update.isnull(); }

	// inline configuration helpers
	static void static_set_type(device_t &device, screen_type_enum type);
	static void static_set_raw(device_t &device, UINT32 pixclock, UINT16 htotal, UINT16 hbend, UINT16 hbstart, UINT16 vtotal, UINT16 vbend, UINT16 vbstart);
	static void static_set_refresh(device_t &device, attoseconds_t rate);
	static void static_set_vblank_time(device_t &device, attoseconds_t time);
	static void static_set_size(device_t &device, UINT16 width, UINT16 height);
	static void static_set_visarea(device_t &device, INT16 minx, INT16 maxx, INT16 miny, INT16 maxy);
	static void static_set_default_position(device_t &device, double xscale, double xoffs, double yscale, double yoffs);
	static void static_set_screen_update(device_t &device, screen_update_delegate callback, const char *devicename = NULL);
	static void static_set_screen_eof(device_t &device, screen_eof_func callback);

	// information getters
	screen_device *next_screen() const { return downcast<screen_device *>(typenext()); }
	render_container &container() const { assert(m_container != NULL); return *m_container; }
	void screen_eof();

	// dynamic configuration
	void configure(int width, int height, const rectangle &visarea, attoseconds_t frame_period);
	void reset_origin(int beamy = 0, int beamx = 0);
	void set_visible_area(int min_x, int max_x, int min_y, int max_y);

	// beam positioning and state
	int vpos() const;
	int hpos() const;
	bool vblank() const { return (machine().time() < m_vblank_end_time); }
	bool hblank() const { int curpos = hpos(); return (curpos < m_visarea.min_x || curpos > m_visarea.max_x); }

	// timing
	attotime time_until_pos(int vpos, int hpos = 0) const;
	attotime time_until_vblank_start() const { return time_until_pos(m_visarea.max_y + 1); }
	attotime time_until_vblank_end() const;
	attotime time_until_update() const { return (machine().config().m_video_attributes & VIDEO_UPDATE_AFTER_VBLANK) ? time_until_vblank_end() : time_until_vblank_start(); }
	attotime scan_period() const { return attotime(0, m_scantime); }
	attotime frame_period() const { return (this == NULL) ? DEFAULT_FRAME_PERIOD : attotime(0, m_frame_period); };
	UINT64 frame_number() const { return m_frame_number; }
	int partial_updates() const { return m_partial_updates_this_frame; }

	// updating
	bool update_partial(int scanline);
	void update_now();
	
	// additional helpers
	void register_vblank_callback(vblank_state_delegate vblank_callback);

	// internal to the video system
	bool update_quads();
	void update_burnin();

	// globally accessible constants
	static const int DEFAULT_FRAME_RATE = 60;
	static const attotime DEFAULT_FRAME_PERIOD;

private:
	// device-level overrides
	virtual bool device_validity_check(emu_options &options, const game_driver &driver) const;
	virtual void device_start();
	virtual void device_stop();
	virtual void device_post_load();

	// internal helpers
	void set_container(render_container &container) { m_container = &container; }
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
	void load_effect_overlay(const char *filename);

	// inline configuration data
	screen_type_enum	m_type;						// type of screen
	bool				m_oldstyle_vblank_supplied;	// MCFG_SCREEN_VBLANK_TIME macro used
	attoseconds_t		m_refresh;					// default refresh period
	attoseconds_t		m_vblank;					// duration of a VBLANK
	float				m_xoffset, m_yoffset;		// default X/Y offsets
	float				m_xscale, m_yscale;			// default X/Y scale factor
	screen_update_delegate m_screen_update;			// screen update callback (16-bit palette)
	const char *		m_screen_update_device;		// device for resolving the screen update
	screen_eof_func		m_screen_eof;				// screen eof callback

	// internal state
	render_container *	m_container;				// pointer to our container

	// dimensions
	int					m_width;					// current width (HTOTAL)
	int					m_height;					// current height (VTOTAL)
	rectangle			m_visarea;					// current visible area (HBLANK end/start, VBLANK end/start)

	// textures and bitmaps
	texture_format		m_texformat;				// texture format
	render_texture *	m_texture[2];				// 2x textures for the screen bitmap
	screen_bitmap		m_bitmap[2];				// 2x bitmaps for rendering
	bitmap_ind64		m_burnin;					// burn-in bitmap
	UINT8				m_curbitmap;				// current bitmap index
	UINT8				m_curtexture;				// current texture index
	bool				m_changed;					// has this bitmap changed?
	INT32				m_last_partial_scan;		// scanline of last partial update
	bitmap_argb32		m_screen_overlay_bitmap;	// screen overlay bitmap

	// screen timing
	attoseconds_t		m_frame_period;				// attoseconds per frame
	attoseconds_t		m_scantime;					// attoseconds per scanline
	attoseconds_t		m_pixeltime;				// attoseconds per pixel
	attoseconds_t		m_vblank_period;			// attoseconds per VBLANK period
	attotime			m_vblank_start_time;		// time of last VBLANK start
	attotime			m_vblank_end_time;			// time of last VBLANK end
	emu_timer *			m_vblank_begin_timer;		// timer to signal VBLANK start
	emu_timer *			m_vblank_end_timer;			// timer to signal VBLANK end
	emu_timer *			m_scanline0_timer;			// scanline 0 timer
	emu_timer *			m_scanline_timer;			// scanline timer
	UINT64				m_frame_number;				// the current frame number
	UINT32				m_partial_updates_this_frame;// partial update counter this frame

	class callback_item
	{
	public:
		callback_item(vblank_state_delegate callback)
			: m_next(NULL),
			  m_callback(callback) { }
		callback_item *next() const { return m_next; }

		callback_item *				m_next;
		vblank_state_delegate		m_callback;
	};
	simple_list<callback_item> m_callback_list;		// list of VBLANK callbacks
};

// device type definition
extern const device_type SCREEN;



//**************************************************************************
//  SCREEN DEVICE CONFIGURATION MACROS
//**************************************************************************

#define SCREEN_UPDATE_NAME(name)		screen_update_##name
#define SCREEN_UPDATE_IND16(name)	UINT32 SCREEN_UPDATE_NAME(name)(screen_device *__dummy, screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
#define SCREEN_UPDATE_RGB32(name)		UINT32 SCREEN_UPDATE_NAME(name)(screen_device *__dummy, screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
#define SCREEN_UPDATE16_CALL(name)		SCREEN_UPDATE_NAME(name)(__dummy, screen, bitmap, cliprect)
#define SCREEN_UPDATE32_CALL(name)		SCREEN_UPDATE_NAME(name)(__dummy, screen, bitmap, cliprect)

#define SCREEN_EOF_NAME(name)			screen_eof_##name
#define SCREEN_EOF(name)				void SCREEN_EOF_NAME(name)(screen_device &screen)
#define SCREEN_EOF_CALL(name)			SCREEN_EOF_NAME(name)(screen)

#define screen_eof_0					NULL

#define MCFG_SCREEN_ADD(_tag, _type) \
	MCFG_DEVICE_ADD(_tag, SCREEN, 0) \
	MCFG_SCREEN_TYPE(_type) \

#define MCFG_SCREEN_MODIFY(_tag) \
	MCFG_DEVICE_MODIFY(_tag)

#define MCFG_SCREEN_TYPE(_type) \
	screen_device::static_set_type(*device, SCREEN_TYPE_##_type); \

#define MCFG_SCREEN_RAW_PARAMS(_pixclock, _htotal, _hbend, _hbstart, _vtotal, _vbend, _vbstart) \
	screen_device::static_set_raw(*device, _pixclock, _htotal, _hbend, _hbstart, _vtotal, _vbend, _vbstart);

#define MCFG_SCREEN_REFRESH_RATE(_rate) \
	screen_device::static_set_refresh(*device, HZ_TO_ATTOSECONDS(_rate)); \

#define MCFG_SCREEN_VBLANK_TIME(_time) \
	screen_device::static_set_vblank_time(*device, _time); \

#define MCFG_SCREEN_SIZE(_width, _height) \
	screen_device::static_set_size(*device, _width, _height); \

#define MCFG_SCREEN_VISIBLE_AREA(_minx, _maxx, _miny, _maxy) \
	screen_device::static_set_visarea(*device, _minx, _maxx, _miny, _maxy); \

#define MCFG_SCREEN_DEFAULT_POSITION(_xscale, _xoffs, _yscale, _yoffs)	\
	screen_device::static_set_default_position(*device, _xscale, _xoffs, _yscale, _yoffs); \

#define MCFG_SCREEN_UPDATE_STATIC(_func) \
	screen_device::static_set_screen_update(*device, screen_update_delegate(&screen_update_##_func, "screen_update_" #_func), device->tag()); \

#define MCFG_SCREEN_UPDATE_DRIVER(_class, _method) \
	screen_device::static_set_screen_update(*device, screen_update_delegate(&_class::_method, #_class "::" #_method)); \

#define MCFG_SCREEN_UPDATE_DEVICE(_device, _class, _method) \
	screen_device::static_set_screen_update(*device, screen_update_delegate(&_class::_method, #_class "::" #_method), _device); \

#define MCFG_SCREEN_EOF(_func) \
	screen_device::static_set_screen_eof(*device, SCREEN_EOF_NAME(_func)); \


#endif	/* __SCREEN_H__ */
