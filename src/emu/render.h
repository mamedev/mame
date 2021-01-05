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
#include "screen.h"

#include <array>
#include <cmath>
#include <functional>
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <string_view>
#include <tuple>
#include <unordered_map>
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
	u32                 height;             // height of the image
	u32                 seqid;              // sequence ID
	u64                 unique_id;          // unique identifier to pass to osd
	u64                 old_id;             // previously allocated id, if applicable
	const rgb_t *       palette;            // palette for PALETTE16 textures, bcg lookup table for RGB32/YUY16
	u32                 palette_length;
};


namespace emu::render::detail {

struct bounds_step
{
	void get(render_bounds &result) const { result = bounds; }

	int             state;
	render_bounds   bounds;
	render_bounds   delta;
};
using bounds_vector = std::vector<bounds_step>;

struct color_step
{
	void get(render_color &result) const { result = color; }

	int             state;
	render_color    color;
	render_color    delta;
};
using color_vector = std::vector<color_step>;


class layout_environment;
class view_environment;

} // namespace emu::render::detail


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
	friend resource_pool_object<render_texture>::~resource_pool_object();
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

	static const int MAX_TEXTURE_SCALES = 16;

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
	friend resource_pool_object<render_container>::~resource_pool_object();
	friend class simple_list<render_container>;
	friend class render_manager;
	friend class render_target;

	// construction/destruction
	render_container(render_manager &manager, screen_device *screen = nullptr);
	~render_container();

public:
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
	render_container *next() const { return m_next; }
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
		item() : m_next(nullptr), m_type(0), m_flags(0), m_internal(0), m_width(0), m_texture(nullptr) { }

		// getters
		item *next() const { return m_next; }
		u8 type() const { return m_type; }
		const render_bounds &bounds() const { return m_bounds; }
		const render_color &color() const { return m_color; }
		u32 flags() const { return m_flags; }
		u32 internal() const { return m_internal; }
		float width() const { return m_width; }
		render_texture *texture() const { return m_texture; }

	private:
		// internal state
		item *              m_next;             // pointer to the next element in the list
		u8                  m_type;             // type of element
		render_bounds       m_bounds;           // bounds of the element
		render_color        m_color;            // RGBA factors
		u32                 m_flags;            // option flags
		u32                 m_internal;         // internal flags
		float               m_width;            // width of the line (lines only)
		render_texture *    m_texture;          // pointer to the source texture (quads only)
	};

	// generic screen overlay scaler
	static void overlay_scale(bitmap_argb32 &dest, bitmap_argb32 &source, const rectangle &sbounds, void *param);

	// internal helpers
	const simple_list<item> &items() const { return m_itemlist; }
	item &add_generic(u8 type, float x0, float y0, float x1, float y1, rgb_t argb);
	void recompute_lookups();
	void update_palette();

	// internal state
	render_container *      m_next;                 // the next container in the list
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



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************


/// \brief A description of a piece of visible artwork
///
/// Most view_items (except for those in the screen layer) have exactly
/// one layout_element which describes the contents of the item.
/// Elements are separate from items because they can be re-used
/// multiple times within a layout.  Even though an element can contain
/// a number of components, they are treated as if they were a single
/// bitmap.
class layout_element
{
public:
	using environment = emu::render::detail::layout_environment;

	// construction/destruction
	layout_element(environment &env, util::xml::data_node const &elemnode);
	virtual ~layout_element();

	// getters
	running_machine &machine() const { return m_machine; }
	int default_state() const { return m_defstate; }
	render_texture *state_texture(int state);

	// operations
	void preload();

private:
	/// \brief An image, rectangle, or disk in an element
	///
	/// Each layout_element contains one or more components. Each
	/// component can describe either an image or a rectangle/disk
	/// primitive. Each component also has a "state" associated with it,
	/// which controls whether or not the component is visible (if the
	/// owning item has the same state, it is visible).
	class component
	{
	public:
		typedef std::unique_ptr<component> ptr;

		// construction/destruction
		component(environment &env, util::xml::data_node const &compnode);
		virtual ~component() = default;

		// setup
		void normalize_bounds(float xoffs, float yoffs, float xscale, float yscale);

		// getters
		int statemask() const { return m_statemask; }
		int stateval() const { return m_stateval; }
		std::pair<int, bool> statewrap() const;
		render_bounds overall_bounds() const;
		render_bounds bounds(int state) const;
		render_color color(int state) const;

		// operations
		virtual void preload(running_machine &machine);
		virtual void draw(running_machine &machine, bitmap_argb32 &dest, int state);

	protected:
		// helpers
		virtual int maxstate() const;
		virtual void draw_aligned(running_machine &machine, bitmap_argb32 &dest, const rectangle &bounds, int state);

		// drawing helpers
		void draw_text(render_font &font, bitmap_argb32 &dest, const rectangle &bounds, std::string_view str, int align, const render_color &color);
		void draw_segment_horizontal_caps(bitmap_argb32 &dest, int minx, int maxx, int midy, int width, int caps, rgb_t color);
		void draw_segment_horizontal(bitmap_argb32 &dest, int minx, int maxx, int midy, int width, rgb_t color);
		void draw_segment_vertical_caps(bitmap_argb32 &dest, int miny, int maxy, int midx, int width, int caps, rgb_t color);
		void draw_segment_vertical(bitmap_argb32 &dest, int miny, int maxy, int midx, int width, rgb_t color);
		void draw_segment_diagonal_1(bitmap_argb32 &dest, int minx, int maxx, int miny, int maxy, int width, rgb_t color);
		void draw_segment_diagonal_2(bitmap_argb32 &dest, int minx, int maxx, int miny, int maxy, int width, rgb_t color);
		void draw_segment_decimal(bitmap_argb32 &dest, int midx, int midy, int width, rgb_t color);
		void draw_segment_comma(bitmap_argb32 &dest, int minx, int maxx, int miny, int maxy, int width, rgb_t color);
		void apply_skew(bitmap_argb32 &dest, int skewwidth);

	private:
		using bounds_vector = emu::render::detail::bounds_vector;
		using color_vector = emu::render::detail::color_vector;

		// internal state
		int const           m_statemask;                // bits of state used to control visibility
		int const           m_stateval;                 // masked state value to make component visible
		bounds_vector       m_bounds;                   // bounds of the element
		color_vector        m_color;                    // color of the element
	};

	// component implementations
	class image_component;
	class rect_component;
	class disk_component;
	class text_component;
	class led7seg_component;
	class led8seg_gts1_component;
	class led14seg_component;
	class led16seg_component;
	class led14segsc_component;
	class led16segsc_component;
	class dotmatrix_component;
	class simplecounter_component;
	class reel_component;

	// a texture encapsulates a texture for a given element in a given state
	class texture
	{
	public:
		texture();
		texture(texture const &that) = delete;
		texture(texture &&that);

		~texture();

		texture &operator=(texture const &that) = delete;
		texture &operator=(texture &&that);

		layout_element *    m_element;      // pointer back to the element
		render_texture *    m_texture;      // texture for this state
		int                 m_state;        // associated state number
	};

	typedef component::ptr (*make_component_func)(environment &env, util::xml::data_node const &compnode);
	typedef std::map<std::string, make_component_func> make_component_map;

	// internal helpers
	static void element_scale(bitmap_argb32 &dest, bitmap_argb32 &source, const rectangle &sbounds, void *param);
	template <typename T> static component::ptr make_component(environment &env, util::xml::data_node const &compnode);
	template <int D> static component::ptr make_dotmatrix_component(environment &env, util::xml::data_node const &compnode);

	static make_component_map const s_make_component; // maps component XML names to creator functions

	// internal state
	running_machine &           m_machine;      // reference to the owning machine
	std::vector<component::ptr> m_complist;     // list of components
	int const                   m_defstate;     // default state of this element
	int                         m_statemask;    // mask to apply to state values
	bool                        m_foldhigh;     // whether we need to fold state values above the mask range
	std::vector<texture>        m_elemtex;      // array of element textures used for managing the scaled bitmaps
};


/// \brief A reusable group of elements
///
/// Views expand/flatten groups into their component elements applying
/// an optional coordinate transform.  This is mainly useful duplicating
/// the same sublayout in multiple views.  It would be more useful
/// within a view if it could be parameterised.  Groups only exist while
/// parsing a layout file - no information about element grouping is
/// preserved.
class layout_group
{
public:
	using environment = emu::render::detail::layout_environment;
	using group_map = std::unordered_map<std::string, layout_group>;
	using transform = std::array<std::array<float, 3>, 3>;

	layout_group(util::xml::data_node const &groupnode);
	~layout_group();

	util::xml::data_node const &get_groupnode() const { return m_groupnode; }

	transform make_transform(int orientation, render_bounds const &dest) const;
	transform make_transform(int orientation, transform const &trans) const;
	transform make_transform(int orientation, render_bounds const &dest, transform const &trans) const;

	void set_bounds_unresolved();
	void resolve_bounds(environment &env, group_map &groupmap);

private:
	void resolve_bounds(environment &env, group_map &groupmap, std::vector<layout_group const *> &seen);
	void resolve_bounds(
			environment &env,
			util::xml::data_node const &parentnode,
			group_map &groupmap,
			std::vector<layout_group const *> &seen,
			bool &empty,
			bool vistoggle,
			bool repeat,
			bool init);

	util::xml::data_node const &    m_groupnode;
	render_bounds                   m_bounds;
	bool                            m_bounds_resolved;
};


/// \brief A single view within a layout_file
///
/// The view is described using arbitrary coordinates that are scaled to
/// fit within the render target.  Pixels within a view are assumed to
/// be square.
class layout_view
{
public:
	using layout_environment = emu::render::detail::layout_environment;
	using view_environment = emu::render::detail::view_environment;
	using element_map = std::unordered_map<std::string, layout_element>;
	using group_map = std::unordered_map<std::string, layout_group>;
	using screen_ref_vector = std::vector<std::reference_wrapper<screen_device> >;
	using prepare_items_delegate = delegate<void ()>;
	using preload_delegate = delegate<void ()>;
	using recomputed_delegate = delegate<void ()>;

	/// \brief A single item in a view
	///
	/// Each view has a list of item structures describing the visual
	/// elements to draw, where they are located, additional blending
	/// modes, and bindings for inputs and outputs.
	class item
	{
		friend class layout_view;

	public:
		using state_delegate = delegate<int ()>;
		using bounds_delegate = delegate<void (render_bounds &)>;
		using color_delegate = delegate<void (render_color &)>;

		// construction/destruction
		item(
				view_environment &env,
				util::xml::data_node const &itemnode,
				element_map &elemmap,
				int orientation,
				layout_group::transform const &trans,
				render_color const &color);
		~item();

		// getters
		std::string const &id() const { return m_id; }
		layout_element *element() const { return m_element; }
		screen_device *screen() { return m_screen; }
		bool bounds_animated() const { return m_bounds.size() > 1U; }
		bool color_animated() const { return m_color.size() > 1U; }
		render_bounds bounds() const { render_bounds result; m_get_bounds(result); return result; }
		render_color color() const { render_color result; m_get_color(result); return result; }
		int blend_mode() const { return m_blend_mode; }
		u32 visibility_mask() const { return m_visibility_mask; }
		int orientation() const { return m_orientation; }
		render_container *screen_container() const { return m_screen ? &m_screen->container() : nullptr; }

		// interactivity
		bool has_input() const { return bool(m_input_port); }
		std::pair<ioport_port *, ioport_value> input_tag_and_mask() const { return std::make_pair(m_input_port, m_input_mask); };
		bool clickthrough() const { return m_clickthrough; }

		// fetch state based on configured source
		int element_state() const { return m_get_elem_state(); }
		int animation_state() const { return m_get_anim_state(); }

		// set state
		void set_state(int state) { m_elem_state = state; }

		// set handlers
		void set_element_state_callback(state_delegate &&handler);
		void set_animation_state_callback(state_delegate &&handler);
		void set_bounds_callback(bounds_delegate &&handler);
		void set_color_callback(color_delegate &&handler);

		// resolve tags, if any
		void resolve_tags();

	private:
		using bounds_vector = emu::render::detail::bounds_vector;
		using color_vector = emu::render::detail::color_vector;

		state_delegate default_get_elem_state();
		state_delegate default_get_anim_state();
		bounds_delegate default_get_bounds();
		color_delegate default_get_color();
		int get_state() const;
		int get_output() const;
		int get_input_raw() const;
		int get_input_field_cached() const;
		int get_input_field_conditional() const;
		int get_anim_output() const;
		int get_anim_input() const;
		void get_interpolated_bounds(render_bounds &result) const;
		void get_interpolated_color(render_color &result) const;

		static layout_element *find_element(view_environment &env, util::xml::data_node const &itemnode, element_map &elemmap);
		static bounds_vector make_bounds(view_environment &env, util::xml::data_node const &itemnode, layout_group::transform const &trans);
		static color_vector make_color(view_environment &env, util::xml::data_node const &itemnode, render_color const &mult);
		static std::string make_animoutput_tag(view_environment &env, util::xml::data_node const &itemnode);
		static std::string make_animinput_tag(view_environment &env, util::xml::data_node const &itemnode);
		static ioport_value make_animmask(view_environment &env, util::xml::data_node const &itemnode);
		static std::string make_input_tag(view_environment &env, util::xml::data_node const &itemnode);
		static int get_blend_mode(view_environment &env, util::xml::data_node const &itemnode);
		static unsigned get_state_shift(ioport_value mask);

		// internal state
		layout_element *const   m_element;          // pointer to the associated element (non-screens only)
		state_delegate          m_get_elem_state;   // resolved element state function
		state_delegate          m_get_anim_state;   // resolved animation state function
		bounds_delegate         m_get_bounds;       // resolved bounds function
		color_delegate          m_get_color;        // resolved color function
		output_finder<>         m_output;           // associated output
		output_finder<>         m_animoutput;       // associated output for animation if different
		ioport_port *           m_animinput_port;   // input port used for animation
		int                     m_elem_state;       // element state used in absence of bindings
		ioport_value const      m_animmask;         // mask for animation state
		u8 const                m_animshift;        // shift for animation state
		ioport_port *           m_input_port;       // input port of this item
		ioport_field const *    m_input_field;      // input port field of this item
		ioport_value const      m_input_mask;       // input mask of this item
		u8 const                m_input_shift;      // input mask rightshift for raw (trailing 0s)
		bool                    m_clickthrough;     // should click pass through to lower elements
		screen_device *         m_screen;           // pointer to screen
		int const               m_orientation;      // orientation of this item
		bounds_vector           m_bounds;           // bounds of the item
		color_vector const      m_color;            // color of the item
		int                     m_blend_mode;       // blending mode to use when drawing
		u32                     m_visibility_mask;  // combined mask of parent visibility groups

		// cold items
		std::string const       m_id;               // optional unique item identifier
		std::string const       m_input_tag;        // input tag of this item
		std::string const       m_animinput_tag;    // tag of input port for animation state
		bounds_vector const     m_rawbounds;        // raw (original) bounds of the item
		bool const              m_have_output;      // whether we actually have an output
		bool const              m_input_raw;        // get raw data from input port
		bool const              m_have_animoutput;  // whether we actually have an output for animation
		bool const              m_has_clickthrough; // whether clickthrough was explicitly configured
	};
	using item_list = std::list<item>;
	using item_ref_vector = std::vector<std::reference_wrapper<item> >;

	/// \brief A subset of items in a view that can be hidden or shown
	///
	/// Visibility toggles allow the user to show or hide selected parts
	/// of a view.
	class visibility_toggle
	{
	public:
		// construction/destruction/assignment
		visibility_toggle(std::string &&name, u32 mask);
		visibility_toggle(visibility_toggle const &) = default;
		visibility_toggle(visibility_toggle &&) = default;
		visibility_toggle &operator=(visibility_toggle const &) = default;
		visibility_toggle &operator=(visibility_toggle &&) = default;

		// getters
		std::string const &name() const { return m_name; }
		u32 mask() const { return m_mask; }

	private:
		std::string             m_name;             // display name for the toggle
		u32                     m_mask;             // toggle combination to show
	};
	using visibility_toggle_vector = std::vector<visibility_toggle>;

	/// \brief An edge of an item in a view
	class edge
	{
	public:
		// construction/destruction
		constexpr edge(unsigned index, float position, bool trailing)
			: m_index(index)
			, m_position(position)
			, m_trailing(trailing)
		{
		}

		// getters
		constexpr unsigned index() const { return m_index; }
		constexpr float position() const { return m_position; }
		constexpr bool trailing() const { return m_trailing; }

		// comparison
		constexpr bool operator<(edge const &that) const
		{
			return std::make_tuple(m_position, m_trailing, m_index) < std::make_tuple(that.m_position, that.m_trailing, that.m_index);
		}

	private:
		unsigned                m_index;            // index of item in some collection
		float                   m_position;         // position of edge on given axis
		bool                    m_trailing;         // false for edge at lower position on axis
	};
	using edge_vector = std::vector<edge>;

	// construction/destruction
	layout_view(
			layout_environment &env,
			util::xml::data_node const &viewnode,
			element_map &elemmap,
			group_map &groupmap);
	~layout_view();

	// getters
	item *get_item(std::string const &id);
	item_list &items() { return m_items; }
	bool has_screen(screen_device &screen);
	const std::string &name() const { return m_name; }
	const std::string &unqualified_name() const { return m_unqualified_name; }
	size_t visible_screen_count() const { return m_screens.size(); }
	float effective_aspect() const { return m_effaspect; }
	const render_bounds &bounds() const { return m_bounds; }
	bool has_visible_screen(screen_device &screen) const;
	const item_ref_vector &visible_items() const { return m_visible_items; }
	const item_ref_vector &visible_screen_items() const { return m_screen_items; }
	const item_ref_vector &interactive_items() const { return m_interactive_items; }
	const edge_vector &interactive_edges_x() const { return m_interactive_edges_x; }
	const edge_vector &interactive_edges_y() const { return m_interactive_edges_y; }
	const screen_ref_vector &visible_screens() const { return m_screens; }
	const visibility_toggle_vector &visibility_toggles() const { return m_vistoggles; }
	u32 default_visibility_mask() const { return m_defvismask; }
	bool has_art() const { return m_has_art; }

	// set handlers
	void set_prepare_items_callback(prepare_items_delegate &&handler);
	void set_preload_callback(preload_delegate &&handler);
	void set_recomputed_callback(recomputed_delegate &&handler);

	// operations
	void prepare_items() { if (!m_prepare_items.isnull()) m_prepare_items(); }
	void recompute(u32 visibility_mask, bool zoom_to_screens);
	void preload();

	// resolve tags, if any
	void resolve_tags();

private:
	struct layer_lists;

	using item_id_map = std::unordered_map<
			std::reference_wrapper<std::string const>,
			item &,
			std::hash<std::string>,
			std::equal_to<std::string> >;

	// add items, recursing for groups
	void add_items(
			layer_lists &layers,
			view_environment &env,
			util::xml::data_node const &parentnode,
			element_map &elemmap,
			group_map &groupmap,
			int orientation,
			layout_group::transform const &trans,
			render_color const &color,
			bool root,
			bool repeat,
			bool init);

	static std::string make_name(layout_environment &env, util::xml::data_node const &viewnode);

	// internal state
	float                       m_effaspect;        // X/Y of the layout in current configuration
	render_bounds               m_bounds;           // computed bounds of the view in current configuration
	item_list                   m_items;            // list of layout items
	item_ref_vector             m_visible_items;    // all visible items
	item_ref_vector             m_screen_items;     // visible items that represent screens to draw
	item_ref_vector             m_interactive_items;// visible items that can accept pointer input
	edge_vector                 m_interactive_edges_x;
	edge_vector                 m_interactive_edges_y;
	screen_ref_vector           m_screens;          // list screens visible in current configuration

	// handlers
	prepare_items_delegate      m_prepare_items;    // prepare items for adding to render container
	preload_delegate            m_preload;          // additional actions when visible items change
	recomputed_delegate         m_recomputed;       // additional actions on resizing/visibility change

	// cold items
	std::string                 m_name;             // display name for the view
	std::string                 m_unqualified_name; // the name exactly as specified in the layout file
	item_id_map                 m_items_by_id;      // items with non-empty ID indexed by ID
	visibility_toggle_vector    m_vistoggles;       // collections of items that can be shown/hidden
	render_bounds               m_expbounds;        // explicit bounds of the view
	u32                         m_defvismask;       // default visibility mask
	bool                        m_has_art;          // true if the layout contains non-screen elements
};


/// \brief Layout description file
///
/// Comprises a list of elements and a list of views.  The elements are
/// reusable items that the views reference.
class layout_file
{
public:
	using element_map = std::unordered_map<std::string, layout_element>;
	using group_map = std::unordered_map<std::string, layout_group>;
	using view_list = std::list<layout_view>;
	using resolve_tags_delegate = delegate<void ()>;

	// construction/destruction
	layout_file(device_t &device, util::xml::data_node const &rootnode, char const *searchpath, char const *dirname);
	~layout_file();

	// getters
	device_t &device() const { return m_device; }
	element_map const &elements() const { return m_elemmap; }
	view_list &views() { return m_viewlist; }
	view_list const &views() const { return m_viewlist; }

	// resolve tags, if any
	void resolve_tags();

	// set handlers
	void set_resolve_tags_callback(resolve_tags_delegate &&handler);

private:
	using environment = emu::render::detail::layout_environment;

	// add elements and parameters
	void add_elements(
			environment &env,
			util::xml::data_node const &parentnode,
			group_map &groupmap,
			bool repeat,
			bool init);

	// internal state
	device_t &              m_device;       // device that caused file to be loaded
	element_map             m_elemmap;      // list of shared layout elements
	view_list               m_viewlist;     // list of views
	resolve_tags_delegate   m_resolve_tags; // additional actions after resolving tags
};

// ======================> render_target

// a render_target describes a surface that is being rendered to
class render_target
{
	friend resource_pool_object<render_target>::~resource_pool_object();
	friend class simple_list<render_target>;
	friend class render_manager;

	// construction/destruction
	render_target(render_manager &manager, const internal_layout *layoutfile = nullptr, u32 flags = 0);
	render_target(render_manager &manager, util::xml::data_node const &layout, u32 flags = 0);
	~render_target();

public:
	// getters
	render_target *next() const { return m_next; }
	render_manager &manager() const { return m_manager; }
	u32 width() const { return m_width; }
	u32 height() const { return m_height; }
	float pixel_aspect() const { return m_pixel_aspect; }
	bool keepaspect() const { return m_keepaspect; }
	int scale_mode() const { return m_scale_mode; }
	float max_update_rate() const { return m_max_refresh; }
	int orientation() const { return m_orientation; }
	render_layer_config layer_config() const { return m_layerconfig; }
	layout_view &current_view() const { return m_views[m_curview].first.get(); }
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
	layout_view::visibility_toggle_vector const &visibility_toggles();

	// bounds computations
	void compute_visible_area(s32 target_width, s32 target_height, float target_pixel_aspect, int target_orientation, s32 &visible_width, s32 &visible_height);
	void compute_minimum_size(s32 &minwidth, s32 &minheight);

	// get a primitive list
	render_primitive_list &get_primitives();

	// hit testing
	bool map_point_container(s32 target_x, s32 target_y, render_container &container, float &container_x, float &container_y);
	bool map_point_input(s32 target_x, s32 target_y, ioport_port *&input_port, ioport_value &input_mask, float &input_x, float &input_y);

	// reference tracking
	void invalidate_all(void *refptr);

	// debug containers
	render_container *debug_alloc();
	void debug_free(render_container &container);
	void debug_append(render_container &container);

	// resolve tag lookups
	void resolve_tags();

private:
	using view_mask_pair = std::pair<std::reference_wrapper<layout_view>, u32>;
	using view_mask_vector = std::vector<view_mask_pair>;

	// private classes declared in render.cpp
	struct object_transform;

	// internal helpers
	enum constructor_impl_t { CONSTRUCTOR_IMPL };
	template <typename T> render_target(render_manager &manager, T&& layout, u32 flags, constructor_impl_t);
	void update_layer_config();
	void load_layout_files(const internal_layout *layoutfile, bool singlefile);
	void load_layout_files(util::xml::data_node const &rootnode, bool singlefile);
	void load_additional_layout_files(const char *basename, bool have_artwork);
	bool load_layout_file(const char *dirname, const char *filename);
	bool load_layout_file(const char *dirname, const internal_layout &layout_data, device_t *device = nullptr);
	bool load_layout_file(device_t &device, util::xml::data_node const &rootnode, const char *searchpath, const char *dirname);
	void add_container_primitives(render_primitive_list &list, const object_transform &root_xform, const object_transform &xform, render_container &container, int blendmode);
	void add_element_primitives(render_primitive_list &list, const object_transform &xform, layout_element &element, int state, int blendmode);
	std::pair<float, float> map_point_internal(s32 target_x, s32 target_y);

	// config callbacks
	void config_load(util::xml::data_node const &targetnode);
	bool config_save(util::xml::data_node &targetnode);

	// view lookups
	layout_view *view_by_index(unsigned index);
	int view_index(layout_view &view) const;

	// optimized clearing
	void init_clear_extents();
	bool remove_clear_extent(const render_bounds &bounds);
	void add_clear_extents(render_primitive_list &list);
	void add_clear_and_optimize_primitive_list(render_primitive_list &list);

	// constants
	static constexpr int NUM_PRIMLISTS = 3;
	static constexpr int MAX_CLEAR_EXTENTS = 1000;

	// internal state
	render_target *         m_next;                     // link to next target
	render_manager &        m_manager;                  // reference to our owning manager
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
	std::vector<bool>       m_hit_test;                 // used when mapping points to inputs
	layout_view *           m_base_view;                // the view at the time of first frame
	int                     m_base_orientation;         // the orientation at the time of first frame
	render_layer_config     m_base_layerconfig;         // the layer configuration at the time of first frame
	int                     m_maxtexwidth;              // maximum width of a texture
	int                     m_maxtexheight;             // maximum height of a texture
	simple_list<render_container> m_debug_containers;   // list of debug containers
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
	render_target *first_target() const { return m_targetlist.first(); }
	render_target *target_by_index(int index) const;

	// UI targets
	render_target &ui_target() const { assert(m_ui_target != nullptr); return *m_ui_target; }
	void set_ui_target(render_target &target) { m_ui_target = &target; }
	float ui_aspect(render_container *rc = nullptr);

	// UI containers
	render_container &ui_container() const { assert(m_ui_container != nullptr); return *m_ui_container; }

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
	// containers
	render_container *container_alloc(screen_device *screen = nullptr);
	void container_free(render_container *container);

	// config callbacks
	void config_load(config_type cfg_type, util::xml::data_node const *parentnode);
	void config_save(config_type cfg_type, util::xml::data_node *parentnode);

	// internal state
	running_machine &               m_machine;          // reference back to the machine

	// array of live targets
	simple_list<render_target>      m_targetlist;       // list of targets
	render_target *                 m_ui_target;        // current UI target

	// texture lists
	u32                             m_live_textures;    // number of live textures
	u64                             m_texture_id;       // rolling texture ID counter
	fixed_allocator<render_texture> m_texture_allocator;// texture allocator

	// containers for the UI and for screens
	render_container *              m_ui_container;     // UI container
	simple_list<render_container>   m_screen_container_list; // list of containers for the screen
};

#endif  // MAME_EMU_RENDER_H
