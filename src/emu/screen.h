// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    screen.h

    Core MAME screen device.

***************************************************************************/

#ifndef MAME_EMU_SCREEN_H
#define MAME_EMU_SCREEN_H

#pragma once

#include "rendertypes.h"

#include <type_traits>
#include <utility>


//**************************************************************************
//  CONSTANTS
//**************************************************************************

// screen types
enum screen_type_enum
{
	SCREEN_TYPE_INVALID = 0,
	SCREEN_TYPE_RASTER,
	SCREEN_TYPE_VECTOR,
	SCREEN_TYPE_LCD,
	SCREEN_TYPE_SVG
};

// screen_update callback flags
constexpr u32 UPDATE_HAS_NOT_CHANGED = 0x0001;   // the video has not changed

/*!
 @defgroup flags for video_attributes
 @{
 @def VIDEO_UPDATE_BEFORE_VBLANK
 update_video called at the start of the VBLANK period
 @todo hack, remove me

 @def VIDEO_UPDATE_AFTER_VBLANK
 update_video called at the end of the VBLANK period
 @todo hack, remove me

 @def VIDEO_SELF_RENDER
 indicates VIDEO_UPDATE will add container bits itself

 @def VIDEO_ALWAYS_UPDATE
 force VIDEO_UPDATE to be called even for skipped frames.
 @todo in case you need this one for model updating, then you're doing it wrong (read: hack)

 @def VIDEO_UPDATE_SCANLINE
 calls VIDEO_UPDATE for every visible scanline, even for skipped frames

 @def VIDEO_VARIABLE_WIDTH
 causes the screen to construct its final bitmap from a composite upscale of individual scanline bitmaps

 @}
 */

constexpr u32 VIDEO_UPDATE_BEFORE_VBLANK    = 0x0000;
constexpr u32 VIDEO_UPDATE_AFTER_VBLANK     = 0x0004;

constexpr u32 VIDEO_SELF_RENDER             = 0x0008;
constexpr u32 VIDEO_ALWAYS_UPDATE           = 0x0080;
constexpr u32 VIDEO_UPDATE_SCANLINE         = 0x0100;
constexpr u32 VIDEO_VARIABLE_WIDTH          = 0x0200;


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> screen_bitmap

class screen_bitmap
{
private:
	// internal helpers
	bitmap_t &live() { assert(m_live != nullptr); return *m_live; }
	const bitmap_t &live() const { assert(m_live != nullptr); return *m_live; }

public:
	// construction/destruction
	screen_bitmap()
		: m_format(BITMAP_FORMAT_RGB32)
		, m_texformat(TEXFORMAT_RGB32)
		, m_live(&m_rgb32)
	{ }
	screen_bitmap(bitmap_ind16 &orig)
		: m_format(BITMAP_FORMAT_IND16)
		, m_texformat(TEXFORMAT_PALETTE16)
		, m_live(&m_ind16)
		, m_ind16(orig, orig.cliprect())
	{ }
	screen_bitmap(bitmap_rgb32 &orig)
		: m_format(BITMAP_FORMAT_RGB32)
		, m_texformat(TEXFORMAT_RGB32)
		, m_live(&m_rgb32)
		, m_rgb32(orig, orig.cliprect())
	{ }

	// resizing
	void resize(int width, int height) { live().resize(width, height); }

	// conversion
	operator bitmap_t &() { return live(); }
	bitmap_ind16 &as_ind16() { assert(m_format == BITMAP_FORMAT_IND16); return m_ind16; }
	bitmap_rgb32 &as_rgb32() { assert(m_format == BITMAP_FORMAT_RGB32); return m_rgb32; }

	// getters
	s32 width() const { return live().width(); }
	s32 height() const { return live().height(); }
	s32 rowpixels() const { return live().rowpixels(); }
	s32 rowbytes() const { return live().rowbytes(); }
	u8 bpp() const { return live().bpp(); }
	bitmap_format format() const { return m_format; }
	texture_format texformat() const { return m_texformat; }
	bool valid() const { return live().valid(); }
	palette_t *palette() const { return live().palette(); }
	const rectangle &cliprect() const { return live().cliprect(); }

	// operations
	void set_palette(palette_t *palette) { live().set_palette(palette); }
	void set_format(bitmap_format format, texture_format texformat)
	{
		m_format = format;
		m_texformat = texformat;
		switch (format)
		{
			case BITMAP_FORMAT_IND16:   m_live = &m_ind16;  break;
			case BITMAP_FORMAT_RGB32:   m_live = &m_rgb32;  break;
			default:                    m_live = nullptr;      break;
		}
		m_ind16.reset();
		m_rgb32.reset();
	}

private:
	// internal state
	bitmap_format       m_format;
	texture_format      m_texformat;
	bitmap_t *          m_live;
	bitmap_ind16        m_ind16;
	bitmap_rgb32        m_rgb32;
};


// ======================> other delegate types

typedef delegate<void (screen_device &, bool)> vblank_state_delegate;

typedef device_delegate<u32 (screen_device &, bitmap_ind16 &, const rectangle &)> screen_update_ind16_delegate;
typedef device_delegate<u32 (screen_device &, bitmap_rgb32 &, const rectangle &)> screen_update_rgb32_delegate;


// ======================> screen_device

class screen_device : public device_t
{
	friend class render_manager;

public:
	// construction/destruction
	screen_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	screen_device(const machine_config &mconfig, const char *tag, device_t *owner, screen_type_enum type)
		: screen_device(mconfig, tag, owner, u32(0))
	{
		set_type(type);
	}
	screen_device(const machine_config &mconfig, const char *tag, device_t *owner, screen_type_enum type, rgb_t color)
		: screen_device(mconfig, tag, owner, u32(0))
	{
		set_type(type);
		set_color(color);
	}
	~screen_device();

	// configuration readers
	screen_type_enum screen_type() const { return m_type; }
	int orientation() const { assert(configured()); return m_orientation; }
	std::pair<unsigned, unsigned> physical_aspect() const;
	int width() const { return m_width; }
	int height() const { return m_height; }
	const rectangle &visible_area() const { return m_visarea; }
	const rectangle &cliprect() const { return m_bitmap[0].cliprect(); }
	bool oldstyle_vblank_supplied() const { return m_oldstyle_vblank_supplied; }
	attoseconds_t refresh_attoseconds() const { return m_refresh; }
	attoseconds_t vblank_attoseconds() const { return m_vblank; }
	bitmap_format format() const { return !m_screen_update_ind16.isnull() ? BITMAP_FORMAT_IND16 : BITMAP_FORMAT_RGB32; }
	float xoffset() const { return m_xoffset; }
	float yoffset() const { return m_yoffset; }
	float xscale() const { return m_xscale; }
	float yscale() const { return m_yscale; }
	bool has_screen_update() const { return !m_screen_update_ind16.isnull() || !m_screen_update_rgb32.isnull(); }

	// inline configuration helpers
	void set_type(screen_type_enum type) { assert(!configured()); m_type = type; }
	void set_orientation(int orientation) { assert(!configured()); m_orientation = orientation; }
	void set_physical_aspect(unsigned x, unsigned y) { assert(!configured()); m_phys_aspect = std::make_pair(x, y); }
	void set_native_aspect() { assert(!configured()); m_phys_aspect = std::make_pair(~0U, ~0U); }

	/// \brief Configure screen parameters
	///
	/// \param [in] pixclock Pixel clock frequency in Hertz.
	/// \param [in] htotal Total pixel clocks per line, including
	///   horizontal blanking period.
	/// \param [in] hbend Index of first visible pixel after horizontal
	///   blanking period ends.
	/// \param [in] hbstart Index of first pixel in horzontal blanking
	///   period after visible pixels.
	/// \param [in] vtotal Total lines per frame, including vertical
	///   blanking period.
	/// \param [in] vbend Index of first visible line after vertical
	///   blanking period ends.
	/// \param [in] vbstart Index of first line in vertical blanking
	///   period after visible lines.
	/// \return Reference to device for method chaining.
	screen_device &set_raw(u32 pixclock, u16 htotal, u16 hbend, u16 hbstart, u16 vtotal, u16 vbend, u16 vbstart)
	{
		assert(pixclock != 0);
		set_clock(pixclock);
		m_refresh = HZ_TO_ATTOSECONDS(pixclock) * htotal * vtotal;
		m_vblank = m_refresh / vtotal * (vtotal - (vbstart - vbend));
		m_width = htotal;
		m_height = vtotal;
		m_visarea.set(hbend, hbstart ? hbstart - 1 : htotal - 1, vbend, vbstart - 1);
		return *this;
	}
	screen_device &set_raw(const XTAL &xtal, u16 htotal, u16 hbend, u16 hbstart, u16 vtotal, u16 vbend, u16 vbstart)
	{
		xtal.validate(std::string("Configuring screen ") + tag());
		return set_raw(xtal.value(), htotal, hbend, hbstart, vtotal, vbend, vbstart);
	}
	screen_device &set_raw(const XTAL &xtal, u16 htotal, u16 vtotal, rectangle visarea)
	{
		return set_raw(xtal, htotal, visarea.left(), visarea.right() + 1, vtotal, visarea.top(), visarea.bottom() + 1);
	}
	void set_refresh(attoseconds_t rate) { m_refresh = rate; }

	/// \brief Set refresh rate in Hertz
	///
	/// Sets refresh rate in Hertz (frames per second). Used in
	/// conjunction with #set_vblank_time, #set_size and #set_visarea.
	/// For raster displays, please use #set_raw to configure screen
	/// parameters in terms of pixel clock.
	/// \param [in] hz Desired refresh rate.
	/// \return Reference to device for method chaining.
	template <typename T> screen_device &set_refresh_hz(T &&hz)
	{
		set_refresh(HZ_TO_ATTOSECONDS(std::forward<T>(hz)));
		return *this;
	}

	/// \brief Set vertical blanking interval time
	///
	/// Sets vertical blanking interval period.  Used in conjunction
	/// with #set_refresh_hz, #set_size and #set_visarea.  For raster
	/// displays, please use #set_raw to configure screen parameters in
	/// terms of pixel clock.
	/// \param [in] time Length of vertical blanking interval.
	/// \return Reference to device for method chaining.
	screen_device &set_vblank_time(attoseconds_t time)
	{
		m_vblank = time;
		m_oldstyle_vblank_supplied = true;
		return *this;
	}

	/// \brief Set total screen size
	///
	/// Set the total screen size in pixels, including blanking areas if
	/// applicable.  This sets the size of the screen bitmap.  Used in
	/// conjunction with #set_refresh_hz, #set_vblank_time and
	/// #set_visarea.  For raster displays, please use #set_raw to
	/// configure screen parameters in terms of pixel clock.
	/// \param [in] width Total width in pixels, including horizontal
	///   blanking period if applicable.
	/// \param [in] height Total height in lines, including vertical
	///   blanking period if applicable.
	/// \return Reference to device for method chaining.
	screen_device &set_size(u16 width, u16 height)
	{
		m_width = width;
		m_height = height;
		return *this;
	}

	/// \brief Set visible screen area
	///
	/// Set visible screen area.  This should fit within the total
	/// screen area.  Used in conjunction with #set_refresh_hz,
	/// #set_vblank_time and #set_size.  For raster displays, please
	/// use #set_raw to configure screen parameters in terms of pixel
	/// clock.
	/// \param [in] minx First visible pixel index after horizontal
	///   blanking period ends.
	/// \param [in] maxx Last visible pixel index before horizontal
	///   blanking period starts.
	/// \param [in] miny First visible line index after vertical
	///   blanking period ends.
	/// \param [in] maxy Last visible line index before vertical
	///   blanking period starts.
	/// \return Reference to device for method chaining.
	screen_device &set_visarea(s16 minx, s16 maxx, s16 miny, s16 maxy)
	{
		m_visarea.set(minx, maxx, miny, maxy);
		return *this;
	}

	/// \brief Set visible area to full area
	///
	/// Set visible screen area to the full screen area (i.e. no
	/// horizontal or vertical blanking period).  This is generally not
	/// possible for raster displays, but is useful for other display
	/// simulations.  Must be called after calling #set_size.
	/// \return Reference to device for method chaining.
	/// \sa set_visarea
	screen_device &set_visarea_full()
	{
		m_visarea.set(0, m_width - 1, 0, m_height - 1);
		return *this;
	}

	void set_default_position(double xscale, double xoffs, double yscale, double yoffs)
	{
		m_xscale = xscale;
		m_xoffset = xoffs;
		m_yscale = yscale;
		m_yoffset = yoffs;
	}

	template <typename F>
	std::enable_if_t<screen_update_ind16_delegate::supports_callback<F>::value> set_screen_update(F &&callback, const char *name)
	{
		m_screen_update_ind16.set(std::forward<F>(callback), name);
		m_screen_update_rgb32 = screen_update_rgb32_delegate(*this);
	}
	template <typename F>
	std::enable_if_t<screen_update_rgb32_delegate::supports_callback<F>::value> set_screen_update(F &&callback, const char *name)
	{
		m_screen_update_ind16 = screen_update_ind16_delegate(*this);
		m_screen_update_rgb32.set(std::forward<F>(callback), name);
	}
	template <typename T, typename F>
	std::enable_if_t<screen_update_ind16_delegate::supports_callback<F>::value> set_screen_update(T &&target, F &&callback, const char *name)
	{
		m_screen_update_ind16.set(std::forward<T>(target), std::forward<F>(callback), name);
		m_screen_update_rgb32 = screen_update_rgb32_delegate(*this);
	}
	template <typename T, typename F>
	std::enable_if_t<screen_update_rgb32_delegate::supports_callback<F>::value> set_screen_update(T &&target, F &&callback, const char *name)
	{
		m_screen_update_ind16 = screen_update_ind16_delegate(*this);
		m_screen_update_rgb32.set(std::forward<T>(target), std::forward<F>(callback), name);
	}

	auto screen_vblank() { return m_screen_vblank.bind(); }
	auto scanline() { m_video_attributes |= VIDEO_UPDATE_SCANLINE; return m_scanline_cb.bind(); }
	template <typename T> screen_device &set_palette(T &&tag) { m_palette.set_tag(std::forward<T>(tag)); return *this; }
	screen_device &set_no_palette() { m_palette.set_tag(finder_base::DUMMY_TAG); return *this; }
	screen_device &set_video_attributes(u32 flags) { m_video_attributes = flags; return *this; }
	screen_device &set_color(rgb_t color) { m_color = color; return *this; }
	template <typename T> screen_device &set_svg_region(T &&tag) { m_svg_region.set_tag(std::forward<T>(tag)); return *this; } // default region is device tag

	// information getters
	render_container &container() const { assert(m_container != nullptr); return *m_container; }
	bitmap_ind8 &priority() { return m_priority; }
	device_palette_interface &palette() const { assert(m_palette != nullptr); return *m_palette; }
	bool has_palette() const { return m_palette != nullptr; }
	screen_bitmap &curbitmap() { return m_bitmap[m_curtexture]; }

	// dynamic configuration
	void configure(int width, int height, const rectangle &visarea, attoseconds_t frame_period);
	void reset_origin(int beamy = 0, int beamx = 0);
	void set_visible_area(int min_x, int max_x, int min_y, int max_y);
	void set_brightness(u8 brightness) { m_brightness = brightness; }

	// beam positioning and state
	int vpos() const;
	int hpos() const;
	int vblank() const { return (machine().time() < m_vblank_end_time) ? 1 : 0; }
	int hblank() const { int const curpos = hpos(); return (curpos < m_visarea.left() || curpos > m_visarea.right()) ? 1 : 0; }

	// timing
	attotime time_until_pos(int vpos, int hpos = 0) const;
	attotime time_until_vblank_start() const { return time_until_pos(m_visarea.bottom() + 1); }
	attotime time_until_vblank_end() const;
	attotime time_until_update() const { return (m_video_attributes & VIDEO_UPDATE_AFTER_VBLANK) ? time_until_vblank_end() : time_until_vblank_start(); }
	attotime scan_period() const { return attotime(0, m_scantime); }
	attotime pixel_period() const { return attotime(0, m_pixeltime); }
	attotime frame_period() const { return attotime(0, m_frame_period); }
	u64 frame_number() const { return m_frame_number; }

	// pixel-level access
	u32 pixel(s32 x, s32 y);
	void pixels(u32* buffer);

	// updating
	int partial_updates() const { return m_partial_updates_this_frame; }
	int partial_scan_hpos() const { return m_partial_scan_hpos; }
	bool update_partial(int scanline);
	void update_now();
	void reset_partial_updates();

	// additional helpers
	void register_vblank_callback(vblank_state_delegate vblank_callback);
	void register_screen_bitmap(bitmap_t &bitmap);

	// internal to the video system
	bool update_quads();
	void update_burnin();

	// globally accessible constants
	static constexpr int DEFAULT_FRAME_RATE = 60;
	static const attotime DEFAULT_FRAME_PERIOD;

private:
	class svg_renderer;

	// device-level overrides
	virtual void device_validity_check(validity_checker &valid) const override;
	virtual void device_config_complete() override;
	virtual void device_resolve_objects() override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_stop() override ATTR_COLD;
	virtual void device_post_load() override;

	// internal helpers
	void set_container(render_container &container) { m_container = &container; }
	void realloc_screen_bitmaps();
	TIMER_CALLBACK_MEMBER(vblank_begin);
	TIMER_CALLBACK_MEMBER(vblank_end);
	TIMER_CALLBACK_MEMBER(first_scanline_tick);
	TIMER_CALLBACK_MEMBER(scanline_tick);
	void finalize_burnin();
	void load_effect_overlay(const char *filename);
	void update_scan_bitmap_size(int y);
	void pre_update_scanline(int y);
	void create_composited_bitmap();
	void destroy_scan_bitmaps();
	void allocate_scan_bitmaps();

	// inline configuration data
	screen_type_enum    m_type;                     // type of screen
	int                 m_orientation;              // orientation flags combined with system flags
	std::pair<unsigned, unsigned> m_phys_aspect;    // physical aspect ratio
	bool                m_oldstyle_vblank_supplied; // set_vblank_time call used
	attoseconds_t       m_refresh;                  // default refresh period
	attoseconds_t       m_vblank;                   // duration of a VBLANK
	float               m_xoffset, m_yoffset;       // default X/Y offsets
	float               m_xscale, m_yscale;         // default X/Y scale factor
	screen_update_ind16_delegate m_screen_update_ind16; // screen update callback (16-bit palette)
	screen_update_rgb32_delegate m_screen_update_rgb32; // screen update callback (32-bit RGB)
	devcb_write_line    m_screen_vblank;            // screen vblank line callback
	devcb_write32       m_scanline_cb;              // screen scanline callback
	optional_device<device_palette_interface> m_palette; // our palette
	u32                 m_video_attributes;         // flags describing the video system
	optional_memory_region m_svg_region;            // the region in which the svg data is in

	// internal state
	render_container *  m_container;                // pointer to our container
	std::unique_ptr<svg_renderer> m_svg; // the svg renderer
	// dimensions
	int                 m_max_width;                // maximum width encountered
	int                 m_width;                    // current width (HTOTAL)
	int                 m_height;                   // current height (VTOTAL)
	rectangle           m_visarea;                  // current visible area (HBLANK end/start, VBLANK end/start)
	std::vector<int>    m_scan_widths;              // current width, in samples, of each individual scanline

	// textures and bitmaps
	texture_format      m_texformat;                // texture format
	render_texture *    m_texture[2];               // 2x textures for the screen bitmap
	screen_bitmap       m_bitmap[2];                // 2x bitmaps for rendering
	std::vector<bitmap_t *> m_scan_bitmaps[2];      // 2x bitmaps for each individual scanline
	bitmap_ind8         m_priority;                 // priority bitmap
	bitmap_ind64        m_burnin;                   // burn-in bitmap
	u8                  m_curbitmap;                // current bitmap index
	u8                  m_curtexture;               // current texture index
	bool                m_changed;                  // has this bitmap changed?
	attotime            m_last_partial_reset;       // last time partial updates were reset
	s32                 m_last_partial_scan;        // scanline of last partial update
	s32                 m_partial_scan_hpos;        // horizontal pixel last rendered on this partial scanline
	bitmap_argb32       m_screen_overlay_bitmap;    // screen overlay bitmap
	u32                 m_unique_id;                // unique id for this screen_device
	rgb_t               m_color;                    // render color
	u8                  m_brightness;               // global brightness

	// screen timing
	attoseconds_t       m_frame_period;             // attoseconds per frame
	attoseconds_t       m_scantime;                 // attoseconds per scanline
	attoseconds_t       m_pixeltime;                // attoseconds per pixel
	attoseconds_t       m_vblank_period;            // attoseconds per VBLANK period
	attotime            m_vblank_start_time;        // time of last VBLANK start
	attotime            m_vblank_end_time;          // time of last VBLANK end
	emu_timer *         m_vblank_begin_timer;       // timer to signal VBLANK start
	emu_timer *         m_vblank_end_timer;         // timer to signal VBLANK end
	emu_timer *         m_scanline0_timer;          // scanline 0 timer
	emu_timer *         m_scanline_timer;           // scanline timer
	u64                 m_frame_number;             // the current frame number
	u32                 m_partial_updates_this_frame; // partial update counter this frame

	bool                m_is_primary_screen;

	// VBLANK callbacks
	class callback_item
	{
	public:
		callback_item(vblank_state_delegate callback)
			: m_callback(std::move(callback)) { }

		vblank_state_delegate       m_callback;
	};
	std::vector<std::unique_ptr<callback_item>> m_callback_list; // list of VBLANK callbacks

	// auto-sizing bitmaps
	class auto_bitmap_item
	{
	public:
		auto_bitmap_item(bitmap_t &bitmap)
			: m_bitmap(bitmap) { }
		bitmap_t &                  m_bitmap;
	};
	std::vector<std::unique_ptr<auto_bitmap_item>> m_auto_bitmap_list; // list of registered bitmaps

	// static data
	static u32          m_id_counter;   // incremented for each constructed screen_device,
										// used as a unique identifier during runtime
};

// device type definition
DECLARE_DEVICE_TYPE(SCREEN, screen_device)

// iterator helper
typedef device_type_enumerator<screen_device> screen_device_enumerator;

#endif // MAME_EMU_SCREEN_H
