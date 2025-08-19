// license:BSD-3-Clause
// copyright-holders:Aaron Giles, Vas Crabb
/***************************************************************************

    render.h

    Core rendering routines for MAME.

****************************************************************************

    Theory of operation
    -------------------

    A render "target" is described by 5 parameters:

        - width = width, in pixels
        - height = height, in pixels
        - bpp = depth, in bits per pixel
        - orientation = orientation of the target
        - pixel_aspect = aspect ratio of the pixels

    Width, height, and bpp are self-explanatory. The remaining parameters
    need some additional explanation.

    Regarding orientation, there are three orientations that need to be
    dealt with: target orientation, UI orientation, and game orientation.
    In the current model, the UI orientation tracks the target orientation
    so that the UI is (in theory) facing the correct direction. The game
    orientation is specified by the game driver and indicates how the
    game and artwork are rotated.

    Regarding pixel_aspect, this is the aspect ratio of the individual
    pixels, not the aspect ratio of the screen. You can determine this by
    dividing the aspect ratio of the screen by the aspect ratio of the
    resolution. For example, a 4:3 screen displaying 640x480 gives a
    pixel aspect ratio of (4/3)/(640/480) = 1.0, meaning the pixels are
    square. That same screen displaying 1280x1024 would have a pixel
    aspect ratio of (4/3)/(1280/1024) = 1.06666, meaning the pixels are
    slightly wider than they are tall.

    Artwork is always assumed to be a 1.0 pixel aspect ratio. The game
    screens themselves can be variable aspect ratios.

***************************************************************************/

#ifndef MAME_EMU_RENDER_H
#define MAME_EMU_RENDER_H

#include "rendertypes.h"

#include "interface/uievents.h"

#include <cmath>
#include <list>
#include <memory>
#include <mutex>
#include <utility>
#include <vector>


//**************************************************************************
//  CONSTANTS
//**************************************************************************


// render creation flags
constexpr u8 RENDER_CREATE_NO_ART       = 0x01;         // ignore any views that have art in them
constexpr u8 RENDER_CREATE_SINGLE_FILE  = 0x02;         // only load views from the file specified
constexpr u8 RENDER_CREATE_HIDDEN       = 0x04;         // don't make this target visible

// render scaling modes
enum
{
	SCALE_FRACTIONAL = 0,                               // compute fractional scaling factors for both axes
	SCALE_FRACTIONAL_X,                                 // compute fractional scaling factor for x-axis, and integer factor for y-axis
	SCALE_FRACTIONAL_Y,                                 // compute fractional scaling factor for y-axis, and integer factor for x-axis
	SCALE_FRACTIONAL_AUTO,                              // automatically compute fractional scaling for x/y-axes based on source native orientation
	SCALE_INTEGER                                       // compute integer scaling factors for both axes, based on target dimensions
};

// flags for primitives
constexpr int PRIMFLAG_TEXORIENT_SHIFT = 0;
constexpr u32 PRIMFLAG_TEXORIENT_MASK = 15 << PRIMFLAG_TEXORIENT_SHIFT;

constexpr int PRIMFLAG_TEXFORMAT_SHIFT = 4;
constexpr u32 PRIMFLAG_TEXFORMAT_MASK = 15 << PRIMFLAG_TEXFORMAT_SHIFT;

constexpr int PRIMFLAG_BLENDMODE_SHIFT = 8;
constexpr u32 PRIMFLAG_BLENDMODE_MASK = 15 << PRIMFLAG_BLENDMODE_SHIFT;

constexpr int PRIMFLAG_ANTIALIAS_SHIFT = 12;
constexpr u32 PRIMFLAG_ANTIALIAS_MASK = 1 << PRIMFLAG_ANTIALIAS_SHIFT;
constexpr int PRIMFLAG_SCREENTEX_SHIFT = 13;
constexpr u32 PRIMFLAG_SCREENTEX_MASK = 1 << PRIMFLAG_SCREENTEX_SHIFT;

constexpr int PRIMFLAG_TEXWRAP_SHIFT = 14;
constexpr u32 PRIMFLAG_TEXWRAP_MASK = 1 << PRIMFLAG_TEXWRAP_SHIFT;

constexpr int PRIMFLAG_TEXSHADE_SHIFT = 15;
constexpr u32 PRIMFLAG_TEXSHADE_MASK = 3 << PRIMFLAG_TEXSHADE_SHIFT;

constexpr int PRIMFLAG_VECTOR_SHIFT = 17;
constexpr u32 PRIMFLAG_VECTOR_MASK = 1 << PRIMFLAG_VECTOR_SHIFT;

constexpr int PRIMFLAG_VECTORBUF_SHIFT = 18;
constexpr u32 PRIMFLAG_VECTORBUF_MASK = 1 << PRIMFLAG_VECTORBUF_SHIFT;

constexpr int PRIMFLAG_TYPE_SHIFT = 19;
constexpr u32 PRIMFLAG_TYPE_MASK = 3 << PRIMFLAG_TYPE_SHIFT;
constexpr u32 PRIMFLAG_TYPE_LINE = 0 << PRIMFLAG_TYPE_SHIFT;
constexpr u32 PRIMFLAG_TYPE_VECTOR = 2 << PRIMFLAG_TYPE_SHIFT;
constexpr u32 PRIMFLAG_TYPE_QUAD = 1 << PRIMFLAG_TYPE_SHIFT;

constexpr int PRIMFLAG_PACKABLE_SHIFT = 21;
constexpr u32 PRIMFLAG_PACKABLE = 1 << PRIMFLAG_PACKABLE_SHIFT;

//**************************************************************************
//  MACROS
//**************************************************************************

constexpr u32 PRIMFLAG_TEXORIENT(u32 x)     { return x << PRIMFLAG_TEXORIENT_SHIFT; }
constexpr u32 PRIMFLAG_GET_TEXORIENT(u32 x) { return (x & PRIMFLAG_TEXORIENT_MASK) >> PRIMFLAG_TEXORIENT_SHIFT; }

constexpr u32 PRIMFLAG_TEXFORMAT(u32 x)     { return x << PRIMFLAG_TEXFORMAT_SHIFT; }
constexpr u32 PRIMFLAG_GET_TEXFORMAT(u32 x) { return (x & PRIMFLAG_TEXFORMAT_MASK) >> PRIMFLAG_TEXFORMAT_SHIFT; }

constexpr u32 PRIMFLAG_BLENDMODE(u32 x)     { return x << PRIMFLAG_BLENDMODE_SHIFT; }
constexpr u32 PRIMFLAG_GET_BLENDMODE(u32 x) { return (x & PRIMFLAG_BLENDMODE_MASK) >> PRIMFLAG_BLENDMODE_SHIFT; }

constexpr u32 PRIMFLAG_ANTIALIAS(u32 x)     { return x << PRIMFLAG_ANTIALIAS_SHIFT; }
constexpr u32 PRIMFLAG_GET_ANTIALIAS(u32 x) { return (x & PRIMFLAG_ANTIALIAS_MASK) >> PRIMFLAG_ANTIALIAS_SHIFT; }

constexpr u32 PRIMFLAG_SCREENTEX(u32 x)     { return x << PRIMFLAG_SCREENTEX_SHIFT; }
constexpr u32 PRIMFLAG_GET_SCREENTEX(u32 x) { return (x & PRIMFLAG_SCREENTEX_MASK) >> PRIMFLAG_SCREENTEX_SHIFT; }

constexpr u32 PRIMFLAG_TEXWRAP(u32 x)       { return x << PRIMFLAG_TEXWRAP_SHIFT; }
constexpr u32 PRIMFLAG_GET_TEXWRAP(u32 x)   { return (x & PRIMFLAG_TEXWRAP_MASK) >> PRIMFLAG_TEXWRAP_SHIFT; }

constexpr u32 PRIMFLAG_TEXSHADE(u32 x)      { return x << PRIMFLAG_TEXSHADE_SHIFT; }
constexpr u32 PRIMFLAG_GET_TEXSHADE(u32 x)  { return (x & PRIMFLAG_TEXSHADE_MASK) >> PRIMFLAG_TEXSHADE_SHIFT; }

constexpr u32 PRIMFLAG_VECTOR(u32 x)        { return x << PRIMFLAG_VECTOR_SHIFT; }
constexpr u32 PRIMFLAG_GET_VECTOR(u32 x)    { return (x & PRIMFLAG_VECTOR_MASK) >> PRIMFLAG_VECTOR_SHIFT; }

constexpr u32 PRIMFLAG_VECTORBUF(u32 x)     { return x << PRIMFLAG_VECTORBUF_SHIFT; }
constexpr u32 PRIMFLAG_GET_VECTORBUF(u32 x) { return (x & PRIMFLAG_VECTORBUF_MASK) >> PRIMFLAG_VECTORBUF_SHIFT; }


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// texture scaling callback
typedef void (*texture_scaler_func)(bitmap_argb32 &dest, bitmap_argb32 &source, const rectangle &sbounds, void *param);


// render_texinfo - texture information
struct render_texinfo
{
	void *              base;               // base of the data
	u32                 rowpixels;          // pixels per row
	u32                 width;              // width of the image
	u32                 width_margin;       // left margin of the scaled bounds, if applicable
	u32                 height;             // height of the image
	u32                 seqid;              // sequence ID
	u64                 unique_id;          // unique identifier to pass to osd
	u64                 old_id;             // previously allocated id, if applicable
	const rgb_t *       palette;            // palette for PALETTE16 textures, bcg lookup table for RGB32/YUY16
	u32                 palette_length;
};


// ======================> render_layer_config

// render_layer_config - describes the state of layers
class render_layer_config
{
private:
	static constexpr u8 ZOOM_TO_SCREEN           = 0x01; // zoom to screen area by default
	static constexpr u8 ENABLE_SCREEN_OVERLAY    = 0x02; // enable screen overlays
	static constexpr u8 DEFAULT = ENABLE_SCREEN_OVERLAY;

	u8               m_state = DEFAULT;

	render_layer_config &set_flag(u8 flag, bool enable)
	{
		if (enable) m_state |= flag;
		else m_state &= ~flag;
		return *this;
	}

public:
	constexpr render_layer_config() { }

	bool operator==(const render_layer_config &rhs) const { return m_state == rhs.m_state; }
	bool operator!=(const render_layer_config &rhs) const { return m_state != rhs.m_state; }

	constexpr bool screen_overlay_enabled() const   { return (m_state & ENABLE_SCREEN_OVERLAY) != 0; }
	constexpr bool zoom_to_screen() const           { return (m_state & ZOOM_TO_SCREEN) != 0; }

	render_layer_config &set_screen_overlay_enabled(bool enable)    { return set_flag(ENABLE_SCREEN_OVERLAY, enable); }
	render_layer_config &set_zoom_to_screen(bool zoom)              { return set_flag(ZOOM_TO_SCREEN, zoom); }
};


// ======================> render_primitive

// render_primitive - a single low-level primitive for the rendering engine
class render_primitive
{
	friend class simple_list<render_primitive>;

public:
	render_primitive() { }

	// render primitive types
	enum primitive_type
	{
		INVALID = 0,                        // invalid type
		LINE,                               // a single line
		VECTOR,                             // a single line with additional timing information
		QUAD                                // a rectilinear quad
	};

	// getters
	render_primitive *next() const { return m_next; }
	bool packable(const s32 pack_size) const { return (flags & PRIMFLAG_PACKABLE) && texture.base != nullptr && texture.width <= pack_size && texture.height <= pack_size; }
	float get_quad_width() const { return fabsf(bounds.x1 - bounds.x0); }
	float get_quad_height() const { return fabsf(bounds.y1 - bounds.y0); }
	float get_full_quad_width() const { return fabsf(full_bounds.x1 - full_bounds.x0); }
	float get_full_quad_height() const { return fabsf(full_bounds.y1 - full_bounds.y0); }

	// reset to prepare for re-use
	void reset();

	// public state
	primitive_type      type = INVALID;     // type of primitive
	render_bounds       bounds;             // bounds or positions
	render_bounds       full_bounds;        // bounds or positions (unclipped)
	render_color        color;              // RGBA values
	u32                 flags = 0U;         // flags
	float               width = 0.0F;       // width (for line primitives)
	render_texinfo      texture;            // texture info (for quad primitives)
	render_quad_texuv   texcoords;          // texture coordinates (for quad primitives)
	double              draw_duration;      // period of time in milliseconds that the emulated machine took to draw this (for vector primitives)
	render_container *  container = nullptr;// the render container we belong to

private:
	// internal state
	render_primitive *  m_next = nullptr;   // pointer to next element
};


// ======================> render_primitive_list

// render_primitive_list - an object containing a list head plus a lock
class render_primitive_list
{
	friend class render_target;

	// construction/destruction
	render_primitive_list();
	~render_primitive_list();

public:
	// getters
	render_primitive *first() const { return m_primlist.first(); }

	// range iterators
	using auto_iterator = simple_list<render_primitive>::auto_iterator;
	auto_iterator begin() const { return m_primlist.begin(); }
	auto_iterator end() const { return m_primlist.end(); }

	// lock management
	void acquire_lock() { m_lock.lock(); }
	void release_lock() { m_lock.unlock(); }

	// reference management
	void add_reference(void *refptr);
	bool has_reference(void *refptr) const;

private:
	// helpers for our friends to manipulate the list
	render_primitive *alloc(render_primitive::primitive_type type);
	void release_all();
	void append(render_primitive &prim) { append_or_return(prim, false); }
	void append_or_return(render_primitive &prim, bool clipped);

	// a reference is an abstract reference to an internal object of some sort
	class reference
	{
	public:
		reference *next() const { return m_next; }
		reference *         m_next;             // link to the next reference
		void *              m_refptr;           // reference pointer
	};

	// internal state
	simple_list<render_primitive> m_primlist;               // list of primitives
	simple_list<reference> m_reflist;                       // list of references

	fixed_allocator<render_primitive> m_primitive_allocator;// allocator for primitives
	fixed_allocator<reference> m_reference_allocator;       // allocator for references

	std::recursive_mutex     m_lock;                             // lock to protect list accesses
};


// ======================> render_texture

// a render_texture is used to track transformations when building an object list
class render_texture
{
	friend class simple_list<render_texture>;
	friend class fixed_allocator<render_texture>;
	friend class render_manager;
	friend class render_target;

	// construction/destruction
	render_texture();
	~render_texture();

	// reset before re-use
	void reset(render_manager &manager, texture_scaler_func scaler = nullptr, void *param = nullptr);

	// release resources when freed
	void release();

public:
	// getters
	int format() const { return m_format; }
	render_manager *manager() const { return m_manager; }

	// configure the texture bitmap
	void set_bitmap(bitmap_t &bitmap, const rectangle &sbounds, texture_format format);

	// set a unique identifier
	void set_id(u64 id) { m_old_id = m_id; m_id = id; }

	// generic high-quality bitmap scaler
	static void hq_scale(bitmap_argb32 &dest, bitmap_argb32 &source, const rectangle &sbounds, void *param);

private:
	// internal helpers
	void get_scaled(u32 dwidth, u32 dheight, render_texinfo &texinfo, render_primitive_list &primlist, u32 flags = 0);
	const rgb_t *get_adjusted_palette(render_container &container, u32 &out_length);

	static constexpr int MAX_TEXTURE_SCALES = 100;

	// a scaled_texture contains a single scaled entry for a texture
	struct scaled_texture
	{
		std::unique_ptr<bitmap_argb32>  bitmap;     // final bitmap
		u32                             seqid;      // sequence number
	};

	// internal state
	render_manager *    m_manager;                  // reference to our manager
	render_texture *    m_next;                     // next texture (for free list)
	bitmap_t *          m_bitmap;                   // pointer to the original bitmap
	rectangle           m_sbounds;                  // source bounds within the bitmap
	texture_format      m_format;                   // format of the texture data
	u64                 m_id;                       // unique id to pass to osd
	u64                 m_old_id;                   // previous id, if applicable

	// scaling state (ARGB32 only)
	texture_scaler_func m_scaler;                   // scaling callback
	void *              m_param;                    // scaling callback parameter
	u32                 m_curseq;                   // current sequence number
	scaled_texture      m_scaled[MAX_TEXTURE_SCALES];// array of scaled variants of this texture
};


// ======================> render_container

// a render_container holds a list of items and an orientation for the entire collection
class render_container
{
	friend class render_manager;
	friend class render_target;

public:
	// construction/destruction
	render_container(render_manager &manager, screen_device *screen = nullptr);
	~render_container();

	// user settings describes the collected user-controllable settings
	struct user_settings
	{
		// construction/destruction
		user_settings();

		// public state
		int                 m_orientation;      // orientation
		float               m_brightness;       // brightness
		float               m_contrast;         // contrast
		float               m_gamma;            // gamma
		float               m_xscale;           // horizontal scale factor
		float               m_yscale;           // vertical scale factor
		float               m_xoffset;          // horizontal offset
		float               m_yoffset;          // vertical offset
	};

	// getters
	screen_device *screen() const { return m_screen; }
	render_manager &manager() const { return m_manager; }
	render_texture *overlay() const { return m_overlaytexture; }
	int orientation() const { return m_user.m_orientation; }
	float xscale() const { return m_user.m_xscale; }
	float yscale() const { return m_user.m_yscale; }
	float xoffset() const { return m_user.m_xoffset; }
	float yoffset() const { return m_user.m_yoffset; }
	bool is_empty() const { return m_itemlist.empty(); }
	const user_settings &get_user_settings() const { return m_user; }

	// setters
	void set_overlay(bitmap_argb32 *bitmap);
	void set_user_settings(const user_settings &settings);

	// empty the item list
	void empty() { m_item_allocator.reclaim_all(m_itemlist); }

	// add items to the list
	void add_line(float x0, float y0, float x1, float y1, float width, rgb_t argb, u32 flags);
	void add_vector(float x0, float y0, float x1, float y1, float width, rgb_t argb, double draw_duration, u32 flags);
	void add_quad(float x0, float y0, float x1, float y1, rgb_t argb, render_texture *texture, u32 flags);
	void add_char(float x0, float y0, float height, float aspect, rgb_t argb, render_font &font, u16 ch);
	void add_point(float x0, float y0, float diameter, rgb_t argb, u32 flags) { add_line(x0, y0, x0, y0, diameter, argb, flags); }
	void add_rect(float x0, float y0, float x1, float y1, rgb_t argb, u32 flags) { add_quad(x0, y0, x1, y1, argb, nullptr, flags); }

	// brightness/contrast/gamma helpers
	bool has_brightness_contrast_gamma_changes() const { return (m_user.m_brightness != 1.0f || m_user.m_contrast != 1.0f || m_user.m_gamma != 1.0f); }
	u8 apply_brightness_contrast_gamma(u8 value);
	float apply_brightness_contrast_gamma_fp(float value);
	const rgb_t *bcg_lookup_table(int texformat, u32 &out_length, palette_t *palette = nullptr);

private:
	// an item describes a high level primitive that is added to a container
	class item
	{
		friend class render_container;
		friend class simple_list<item>;

	public:
		item() : m_next(nullptr), m_type(0), m_flags(0), m_internal(0), m_width(0), m_texture(nullptr), m_draw_duration(0) { }

		// getters
		item *next() const { return m_next; }
		u8 type() const { return m_type; }
		const render_bounds &bounds() const { return m_bounds; }
		const render_color &color() const { return m_color; }
		u32 flags() const { return m_flags; }
		u32 internal() const { return m_internal; }
		float width() const { return m_width; }
		render_texture *texture() const { return m_texture; }
		double draw_duration() const { return m_draw_duration; }

	private:
		// internal state
		item *              m_next;             // pointer to the next element in the list
		u8                  m_type;             // type of element (eg. CONTAINER_ITEM_LINE)
		render_bounds       m_bounds;           // bounds of the element
		render_color        m_color;            // RGBA factors
		u32                 m_flags;            // option flags
		u32                 m_internal;         // internal flags
		float               m_width;            // width of the line (lines only)
		render_texture *    m_texture;          // pointer to the source texture (quads only)
		double              m_draw_duration;    // period of time in milliseconds that the emulated machine took to draw this (vectors only)
	};

	// generic screen overlay scaler
	static void overlay_scale(bitmap_argb32 &dest, bitmap_argb32 &source, const rectangle &sbounds, void *param);

	// internal helpers
	const simple_list<item> &items() const { return m_itemlist; }
	item &add_generic(u8 type, float x0, float y0, float x1, float y1, rgb_t argb);
	void recompute_lookups();
	void update_palette();

	// internal state
	render_manager &        m_manager;              // reference back to the owning manager
	simple_list<item>       m_itemlist;             // head of the item list
	fixed_allocator<item>   m_item_allocator;       // free container items
	screen_device *         m_screen;               // the screen device
	user_settings           m_user;                 // user settings
	bitmap_argb32 *         m_overlaybitmap;        // overlay bitmap
	render_texture *        m_overlaytexture;       // overlay texture
	std::unique_ptr<palette_client> m_palclient;    // client to the screen palette
	std::vector<rgb_t>      m_bcglookup;            // copy of screen palette with bcg adjustment
	rgb_t                   m_bcglookup256[0x400];  // lookup table for brightness/contrast/gamma
};


// ======================> render_target

// a render_target describes a surface that is being rendered to
class render_target
{
	friend class simple_list<render_target>;
	friend class render_manager;

	// construction/destruction
	render_target(render_manager &manager, render_container *ui, const internal_layout *layoutfile, u32 flags);
	render_target(render_manager &manager, render_container *ui, util::xml::data_node const &layout, u32 flags);
	~render_target();

public:
	// getters
	render_target *next() const { return m_next; }
	render_manager &manager() const { return m_manager; }
	render_container *ui_container() const { return m_ui_container; }
	u32 width() const { return m_width; }
	u32 height() const { return m_height; }
	float pixel_aspect() const { return m_pixel_aspect; }
	bool keepaspect() const { return m_keepaspect; }
	int scale_mode() const { return m_scale_mode; }
	float max_update_rate() const { return m_max_refresh; }
	int orientation() const { return m_orientation; }
	render_layer_config layer_config() const { return m_layerconfig; }
	layout_view &current_view() const { return m_views[m_curview].first; }
	unsigned view() const { return m_curview; }
	bool external_artwork() const { return m_external_artwork; }
	bool hidden() const { return ((m_flags & RENDER_CREATE_HIDDEN) != 0); }
	bool is_ui_target() const;
	int index() const;

	// setters
	void set_bounds(s32 width, s32 height, float pixel_aspect = 0);
	void set_max_update_rate(float updates_per_second) { m_max_refresh = updates_per_second; }
	void set_orientation(int orientation) { m_orientation = orientation; }
	void set_view(unsigned viewindex);
	void set_max_texture_size(int maxwidth, int maxheight);
	void set_transform_container(bool transform_container) { m_transform_container = transform_container; }
	void set_keepaspect(bool keepaspect) { m_keepaspect = keepaspect; }
	void set_scale_mode(int scale_mode) { m_scale_mode = scale_mode; }

	// pointer input handling
	void pointer_updated(osd::ui_event_handler::pointer type, u16 ptrid, u16 device, s32 x, s32 y, u32 buttons, u32 pressed, u32 released, s16 clicks);
	void pointer_left(osd::ui_event_handler::pointer type, u16 ptrid, u16 device, s32 x, s32 y, u32 released, s16 clicks);
	void pointer_aborted(osd::ui_event_handler::pointer type, u16 ptrid, u16 device, s32 x, s32 y, u32 released, s16 clicks);
	void forget_pointers();
	void update_pointer_fields();

	// layer config getters
	bool screen_overlay_enabled() const { return m_layerconfig.screen_overlay_enabled(); }
	bool zoom_to_screen() const { return m_layerconfig.zoom_to_screen(); }
	u32 visibility_mask() const { return m_views[m_curview].second; }

	// layer config setters
	void set_visibility_toggle(unsigned index, bool enable);
	void set_screen_overlay_enabled(bool enable) { m_layerconfig.set_screen_overlay_enabled(enable); update_layer_config(); }
	void set_zoom_to_screen(bool zoom) { m_layerconfig.set_zoom_to_screen(zoom); update_layer_config(); }

	// view configuration helper
	unsigned configured_view(const char *viewname, int targetindex, int numtargets);

	// view information
	char const *view_name(unsigned index);

	// bounds computations
	void compute_visible_area(s32 target_width, s32 target_height, float target_pixel_aspect, int target_orientation, s32 &visible_width, s32 &visible_height);
	void compute_minimum_size(s32 &minwidth, s32 &minheight);

	// get a primitive list
	render_primitive_list &get_primitives();

	// hit testing
	bool map_point_container(s32 target_x, s32 target_y, render_container &container, float &container_x, float &container_y);

	// reference tracking
	void invalidate_all(void *refptr);

	// resolve tag lookups
	void resolve_tags();

private:
	// constants
	static inline constexpr int NUM_PRIMLISTS = 3;
	static inline constexpr int MAX_CLEAR_EXTENTS = 1000;

	using view_mask_pair = std::pair<layout_view &, u32>;
	using view_mask_vector = std::vector<view_mask_pair>;

	// private classes declared in render.cpp
	struct object_transform;
	struct pointer_info;
	struct hit_test;

	using pointer_info_vector = std::vector<pointer_info>;
	using hit_test_vector = std::vector<hit_test>;

	// internal helpers
	enum constructor_impl_t { CONSTRUCTOR_IMPL };
	template <typename T> render_target(render_manager &manager, render_container *ui, T&& layout, u32 flags, constructor_impl_t);
	void update_layer_config();
	void load_layout_files(const internal_layout *layoutfile, bool singlefile);
	void load_layout_files(util::xml::data_node const &rootnode, bool singlefile);
	void load_additional_layout_files(const char *basename, bool have_artwork);
	bool load_layout_file(const char *dirname, const char *filename);
	bool load_layout_file(const char *dirname, const internal_layout &layout_data, device_t *device = nullptr);
	bool load_layout_file(device_t &device, util::xml::data_node const &rootnode, const char *searchpath, const char *dirname);
	void add_container_primitives(render_primitive_list &list, const object_transform &root_xform, const object_transform &xform, render_container &container, int blendmode);
	void add_element_primitives(render_primitive_list &list, const object_transform &xform, layout_view_item &item);
	std::pair<float, float> map_point_internal(s32 target_x, s32 target_y);
	std::pair<float, float> map_point_layout(s32 target_x, s32 target_y);

	// config callbacks
	void config_load(util::xml::data_node const *targetnode);
	bool config_save(util::xml::data_node &targetnode);

	// view lookups
	layout_view *view_by_index(unsigned index);
	int view_index(layout_view &view) const;

	// optimized clearing
	void init_clear_extents();
	bool remove_clear_extent(const render_bounds &bounds);
	void add_clear_extents(render_primitive_list &list);
	void add_clear_and_optimize_primitive_list(render_primitive_list &list);

	// internal state
	render_target *         m_next;                     // link to next target
	render_manager &        m_manager;                  // reference to our owning manager
	render_container *const m_ui_container;             // container for drawing UI elements
	std::list<layout_file>  m_filelist;                 // list of layout files
	view_mask_vector        m_views;                    // views we consider
	unsigned                m_curview;                  // current view index
	u32                     m_flags;                    // creation flags
	render_primitive_list   m_primlist[NUM_PRIMLISTS];  // list of primitives
	int                     m_listindex;                // index of next primlist to use
	s32                     m_width;                    // width in pixels
	s32                     m_height;                   // height in pixels
	render_bounds           m_bounds;                   // bounds of the target
	bool                    m_keepaspect;               // constrain aspect ratio
	bool                    m_int_overscan;             // allow overscan on integer scaled targets
	float                   m_pixel_aspect;             // aspect ratio of individual pixels
	int                     m_scale_mode;               // type of scale to apply
	int                     m_int_scale_x;              // horizontal integer scale factor
	int                     m_int_scale_y;              // vertical integer scale factor
	float                   m_max_refresh;              // maximum refresh rate, 0 or if none
	int                     m_orientation;              // orientation
	render_layer_config     m_layerconfig;              // layer configuration
	pointer_info_vector     m_pointers;                 // state of pointers over this target
	hit_test_vector         m_clickable_items;          // for tracking clicked elements
	layout_view *           m_base_view;                // the view at the time of first frame
	int                     m_base_orientation;         // the orientation at the time of first frame
	render_layer_config     m_base_layerconfig;         // the layer configuration at the time of first frame
	int                     m_maxtexwidth;              // maximum width of a texture
	int                     m_maxtexheight;             // maximum height of a texture
	s32                     m_clear_extent_count;       // number of clear extents
	s32                     m_clear_extents[MAX_CLEAR_EXTENTS]; // array of clear extents
	bool                    m_transform_container;      // determines whether the screen container is transformed by the core renderer,
														// otherwise the respective render API will handle the transformation (scale, offset)
	bool                    m_external_artwork;         // external artwork was loaded (driver file or override)
};


// ======================> render_manager

// contains machine-global information and operations
class render_manager
{
	friend class render_target;

public:
	// construction/destruction
	render_manager(running_machine &machine);
	~render_manager();

	// getters
	running_machine &machine() const { return m_machine; }

	// global queries
	bool is_live(screen_device &screen) const;
	float max_update_rate() const;

	// targets
	render_target *target_alloc(const internal_layout *layoutfile = nullptr, u32 flags = 0);
	render_target *target_alloc(util::xml::data_node const &layout, u32 flags = 0);
	void target_free(render_target *target);
	const simple_list<render_target> &targets() const { return m_targetlist; }
	render_target *first_target() { return m_targetlist.first(); }
	render_target *target_by_index(int index) const;

	// UI targets
	render_target &ui_target() const { assert(m_ui_target != nullptr); return *m_ui_target; }
	void set_ui_target(render_target &target) { m_ui_target = &target; }
	float ui_aspect(render_container *rc = nullptr);

	// UI containers
	render_container &ui_container() const { assert(ui_target().ui_container()); return *ui_target().ui_container(); }

	// textures
	render_texture *texture_alloc(texture_scaler_func scaler = nullptr, void *param = nullptr);
	void texture_free(render_texture *texture);

	// fonts
	std::unique_ptr<render_font> font_alloc(const char *filename = nullptr);

	// reference tracking
	void invalidate_all(void *refptr);

	// resolve tag lookups
	void resolve_tags();

private:
	// config callbacks
	void config_load(config_type cfg_type, config_level cfg_lvl, util::xml::data_node const *parentnode);
	void config_save(config_type cfg_type, util::xml::data_node *parentnode);

	// internal state
	running_machine &               m_machine;                  // reference back to the machine

	// array of live targets
	simple_list<render_target>      m_targetlist;               // list of targets
	render_target *                 m_ui_target;                // current UI target

	// texture lists
	u32                             m_live_textures;            // number of live textures
	u64                             m_texture_id;               // rolling texture ID counter
	fixed_allocator<render_texture> m_texture_allocator;        // texture allocator

	// containers for UI elements and for screens
	std::list<render_container>     m_ui_containers;            // containers for drawing UI elements
	std::list<render_container>     m_screen_container_list;    // list of containers for the screen
};

#endif  // MAME_EMU_RENDER_H
