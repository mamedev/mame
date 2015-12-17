// license:BSD-3-Clause
// copyright-holders:Aaron Giles
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

#ifndef __RENDER_H__
#define __RENDER_H__

//#include "osdepend.h"

#include <math.h>


//**************************************************************************
//  CONSTANTS
//**************************************************************************

// blending modes
enum
{
	BLENDMODE_NONE = 0,                                 // no blending
	BLENDMODE_ALPHA,                                    // standard alpha blend
	BLENDMODE_RGB_MULTIPLY,                             // apply source alpha to source pix, then multiply RGB values
	BLENDMODE_ADD                                       // apply source alpha to source pix, then add to destination
};


// render creation flags
const UINT8 RENDER_CREATE_NO_ART        = 0x01;         // ignore any views that have art in them
const UINT8 RENDER_CREATE_SINGLE_FILE   = 0x02;         // only load views from the file specified
const UINT8 RENDER_CREATE_HIDDEN        = 0x04;         // don't make this target visible


// flags for primitives
const int PRIMFLAG_TEXORIENT_SHIFT = 0;
const UINT32 PRIMFLAG_TEXORIENT_MASK = 15 << PRIMFLAG_TEXORIENT_SHIFT;

const int PRIMFLAG_TEXFORMAT_SHIFT = 4;
const UINT32 PRIMFLAG_TEXFORMAT_MASK = 15 << PRIMFLAG_TEXFORMAT_SHIFT;

const int PRIMFLAG_BLENDMODE_SHIFT = 8;
const UINT32 PRIMFLAG_BLENDMODE_MASK = 15 << PRIMFLAG_BLENDMODE_SHIFT;

const int PRIMFLAG_ANTIALIAS_SHIFT = 12;
const UINT32 PRIMFLAG_ANTIALIAS_MASK = 1 << PRIMFLAG_ANTIALIAS_SHIFT;

const int PRIMFLAG_SCREENTEX_SHIFT = 13;
const UINT32 PRIMFLAG_SCREENTEX_MASK = 1 << PRIMFLAG_SCREENTEX_SHIFT;

const int PRIMFLAG_TEXWRAP_SHIFT = 14;
const UINT32 PRIMFLAG_TEXWRAP_MASK = 1 << PRIMFLAG_TEXWRAP_SHIFT;

const int PRIMFLAG_TEXSHADE_SHIFT = 15;
const UINT32 PRIMFLAG_TEXSHADE_MASK = 3 << PRIMFLAG_TEXSHADE_SHIFT;

const int PRIMFLAG_VECTOR_SHIFT = 17;
const UINT32 PRIMFLAG_VECTOR_MASK = 1 << PRIMFLAG_VECTOR_SHIFT;

const int PRIMFLAG_VECTORBUF_SHIFT = 18;
const UINT32 PRIMFLAG_VECTORBUF_MASK = 1 << PRIMFLAG_VECTORBUF_SHIFT;

const int PRIMFLAG_TYPE_SHIFT = 19;
const UINT32 PRIMFLAG_TYPE_MASK = 3 << PRIMFLAG_TYPE_SHIFT;
const UINT32 PRIMFLAG_TYPE_LINE = 0 << PRIMFLAG_TYPE_SHIFT;
const UINT32 PRIMFLAG_TYPE_QUAD = 1 << PRIMFLAG_TYPE_SHIFT;

//**************************************************************************
//  MACROS
//**************************************************************************

#define PRIMFLAG_TEXORIENT(x)       ((x) << PRIMFLAG_TEXORIENT_SHIFT)
#define PRIMFLAG_GET_TEXORIENT(x)   (((x) & PRIMFLAG_TEXORIENT_MASK) >> PRIMFLAG_TEXORIENT_SHIFT)

#define PRIMFLAG_TEXFORMAT(x)       ((x) << PRIMFLAG_TEXFORMAT_SHIFT)
#define PRIMFLAG_GET_TEXFORMAT(x)   (((x) & PRIMFLAG_TEXFORMAT_MASK) >> PRIMFLAG_TEXFORMAT_SHIFT)

#define PRIMFLAG_BLENDMODE(x)       ((x) << PRIMFLAG_BLENDMODE_SHIFT)
#define PRIMFLAG_GET_BLENDMODE(x)   (((x) & PRIMFLAG_BLENDMODE_MASK) >> PRIMFLAG_BLENDMODE_SHIFT)

#define PRIMFLAG_ANTIALIAS(x)       ((x) << PRIMFLAG_ANTIALIAS_SHIFT)
#define PRIMFLAG_GET_ANTIALIAS(x)   (((x) & PRIMFLAG_ANTIALIAS_MASK) >> PRIMFLAG_ANTIALIAS_SHIFT)

#define PRIMFLAG_SCREENTEX(x)       ((x) << PRIMFLAG_SCREENTEX_SHIFT)
#define PRIMFLAG_GET_SCREENTEX(x)   (((x) & PRIMFLAG_SCREENTEX_MASK) >> PRIMFLAG_SCREENTEX_SHIFT)

#define PRIMFLAG_TEXWRAP(x)         ((x) << PRIMFLAG_TEXWRAP_SHIFT)
#define PRIMFLAG_GET_TEXWRAP(x)     (((x) & PRIMFLAG_TEXWRAP_MASK) >> PRIMFLAG_TEXWRAP_SHIFT)

#define PRIMFLAG_TEXSHADE(x)        ((x) << PRIMFLAG_TEXSHADE_SHIFT)
#define PRIMFLAG_GET_TEXSHADE(x)    (((x) & PRIMFLAG_TEXSHADE_MASK) >> PRIMFLAG_TEXSHADE_SHIFT)

#define PRIMFLAG_VECTOR(x)          ((x) << PRIMFLAG_VECTOR_SHIFT)
#define PRIMFLAG_GET_VECTOR(x)      (((x) & PRIMFLAG_VECTOR_MASK) >> PRIMFLAG_VECTOR_SHIFT)

#define PRIMFLAG_VECTORBUF(x)       ((x) << PRIMFLAG_VECTORBUF_SHIFT)
#define PRIMFLAG_GET_VECTORBUF(x)   (((x) & PRIMFLAG_VECTORBUF_MASK) >> PRIMFLAG_VECTORBUF_SHIFT)


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// forward definitions
class device_t;
class screen_device;
class render_container;
class render_manager;
struct xml_data_node;
class render_font;
struct object_transform;
class layout_element;
class layout_view;


// texture scaling callback
typedef void (*texture_scaler_func)(bitmap_argb32 &dest, bitmap_argb32 &source, const rectangle &sbounds, void *param);

// render_bounds - floating point bounding rectangle
struct render_bounds
{
	float               x0;                 // leftmost X coordinate
	float               y0;                 // topmost Y coordinate
	float               x1;                 // rightmost X coordinate
	float               y1;                 // bottommost Y coordinate

	float width() const { return x1 - x0; }
	float height() const { return y1 - y0; }
};


// render_color - floating point set of ARGB values
struct render_color
{
	float               a;                  // alpha component (0.0 = transparent, 1.0 = opaque)
	float               r;                  // red component (0.0 = none, 1.0 = max)
	float               g;                  // green component (0.0 = none, 1.0 = max)
	float               b;                  // blue component (0.0 = none, 1.0 = max)
};


// render_texuv - floating point set of UV texture coordinates
struct render_texuv
{
	float               u;                  // U coodinate (0.0-1.0)
	float               v;                  // V coordinate (0.0-1.0)
};


// render_quad_texuv - floating point set of UV texture coordinates
struct render_quad_texuv
{
	render_texuv        tl;                 // top-left UV coordinate
	render_texuv        tr;                 // top-right UV coordinate
	render_texuv        bl;                 // bottom-left UV coordinate
	render_texuv        br;                 // bottom-right UV coordinate
};


// render_texinfo - texture information
struct render_texinfo
{
	void *              base;               // base of the data
	UINT32              rowpixels;          // pixels per row
	UINT32              width;              // width of the image
	UINT32              height;             // height of the image
	UINT32              seqid;              // sequence ID
	UINT64              osddata;            // aux data to pass to osd
	const rgb_t *       palette;            // palette for PALETTE16 textures, bcg lookup table for RGB32/YUY16
};


// ======================> render_screen_list

// a render_screen_list is a list of screen_devices
class render_screen_list
{
	// screen list item
	class item
	{
		friend class simple_list<item>;
		friend class render_screen_list;

	public:
		// construction/destruction
		item(screen_device &screen)
			: m_next(nullptr),
				m_screen(screen) { }

		// state
		item *              m_next;             // next screen in list
		screen_device &     m_screen;           // reference to screen device
	};

public:
	// getters
	int count() const { return m_list.count(); }

	// operations
	void add(screen_device &screen) { m_list.append(*global_alloc(item(screen))); }
	void reset() { m_list.reset(); }

	// query
	int contains(screen_device &screen) const
	{
		int count = 0;
		for (item *curitem = m_list.first(); curitem != nullptr; curitem = curitem->m_next)
			if (&curitem->m_screen == &screen) count++;
		return count;
	}

private:
	// internal state
	simple_list<item> m_list;
};


// ======================> render_layer_config

// render_layer_config - describes the state of layers
class render_layer_config
{
	static const UINT8 ENABLE_BACKDROP          = 0x01; // enable backdrop layers
	static const UINT8 ENABLE_OVERLAY           = 0x02; // enable overlay layers
	static const UINT8 ENABLE_BEZEL             = 0x04; // enable bezel layers
	static const UINT8 ENABLE_CPANEL            = 0x08; // enable cpanel layers
	static const UINT8 ENABLE_MARQUEE           = 0x10; // enable marquee layers
	static const UINT8 ZOOM_TO_SCREEN           = 0x20; // zoom to screen area by default
	static const UINT8 ENABLE_SCREEN_OVERLAY    = 0x40; // enable screen overlays
	static const UINT8 DEFAULT = ENABLE_BACKDROP | ENABLE_OVERLAY | ENABLE_BEZEL | ENABLE_CPANEL | ENABLE_MARQUEE | ENABLE_SCREEN_OVERLAY;

public:
	render_layer_config()
		: m_state(DEFAULT) { }

	bool operator==(const render_layer_config &rhs) const { return m_state == rhs.m_state; }
	bool operator!=(const render_layer_config &rhs) const { return m_state != rhs.m_state; }

	bool backdrops_enabled() const { return ((m_state & ENABLE_BACKDROP) != 0); }
	bool overlays_enabled() const { return ((m_state & ENABLE_OVERLAY) != 0); }
	bool bezels_enabled() const { return ((m_state & ENABLE_BEZEL) != 0); }
	bool cpanels_enabled() const { return ((m_state & ENABLE_CPANEL) != 0); }
	bool marquees_enabled() const { return ((m_state & ENABLE_MARQUEE) != 0); }
	bool screen_overlay_enabled() const { return ((m_state & ENABLE_SCREEN_OVERLAY) != 0); }
	bool zoom_to_screen() const { return ((m_state & ZOOM_TO_SCREEN) != 0); }

	render_layer_config &set_backdrops_enabled(bool enable) { if (enable) m_state |= ENABLE_BACKDROP; else m_state &= ~ENABLE_BACKDROP; return *this; }
	render_layer_config &set_overlays_enabled(bool enable) { if (enable) m_state |= ENABLE_OVERLAY; else m_state &= ~ENABLE_OVERLAY; return *this; }
	render_layer_config &set_bezels_enabled(bool enable) { if (enable) m_state |= ENABLE_BEZEL; else m_state &= ~ENABLE_BEZEL; return *this; }
	render_layer_config &set_cpanels_enabled(bool enable) { if (enable) m_state |= ENABLE_CPANEL; else m_state &= ~ENABLE_CPANEL; return *this; }
	render_layer_config &set_marquees_enabled(bool enable) { if (enable) m_state |= ENABLE_MARQUEE; else m_state &= ~ENABLE_MARQUEE; return *this; }
	render_layer_config &set_screen_overlay_enabled(bool enable) { if (enable) m_state |= ENABLE_SCREEN_OVERLAY; else m_state &= ~ENABLE_SCREEN_OVERLAY; return *this; }
	render_layer_config &set_zoom_to_screen(bool zoom) { if (zoom) m_state |= ZOOM_TO_SCREEN; else m_state &= ~ZOOM_TO_SCREEN; return *this; }

private:
	UINT8               m_state;
};


// ======================> render_primitive

// render_primitive - a single low-level primitive for the rendering engine
class render_primitive
{
	friend class simple_list<render_primitive>;

public:
	render_primitive():
		type(),
		flags(0),
		width(0),
		container(nullptr),
		m_next(nullptr)
	{}

	// render primitive types
	enum primitive_type
	{
		INVALID = 0,                        // invalid type
		LINE,                               // a single line
		QUAD                                // a rectilinear quad
	};

	// getters
	render_primitive *next() const { return m_next; }

	// reset to prepare for re-use
	void reset();

	// public state
	primitive_type      type;               // type of primitive
	render_bounds       bounds;             // bounds or positions
	render_color        color;              // RGBA values
	UINT32              flags;              // flags
	float               width;              // width (for line primitives)
	render_texinfo      texture;            // texture info (for quad primitives)
	render_quad_texuv   texcoords;          // texture coordinates (for quad primitives)
	render_container *  container;          // the render container we belong to

private:
	// internal state
	render_primitive *  m_next;             // pointer to next element
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

	// lock management
	void acquire_lock() { osd_lock_acquire(m_lock); }
	void release_lock() { osd_lock_release(m_lock); }

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

	osd_lock *          m_lock;                             // lock to protect list accesses
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

	// set any necessary aux data
	void set_osd_data(UINT64 data) { m_osddata = data; }

	// generic high-quality bitmap scaler
	static void hq_scale(bitmap_argb32 &dest, bitmap_argb32 &source, const rectangle &sbounds, void *param);

private:
	// internal helpers
	void get_scaled(UINT32 dwidth, UINT32 dheight, render_texinfo &texinfo, render_primitive_list &primlist);
	const rgb_t *get_adjusted_palette(render_container &container);

	static const int MAX_TEXTURE_SCALES = 16;

	// a scaled_texture contains a single scaled entry for a texture
	struct scaled_texture
	{
		bitmap_argb32 *     bitmap;                 // final bitmap
		UINT32              seqid;                  // sequence number
	};

	// internal state
	render_manager *    m_manager;                  // reference to our manager
	render_texture *    m_next;                     // next texture (for free list)
	bitmap_t *          m_bitmap;                   // pointer to the original bitmap
	rectangle           m_sbounds;                  // source bounds within the bitmap
	texture_format      m_format;                   // format of the texture data
	UINT64              m_osddata;                  // aux data to pass to osd

	// scaling state (ARGB32 only)
	texture_scaler_func m_scaler;                   // scaling callback
	void *              m_param;                    // scaling callback parameter
	UINT32              m_curseq;                   // current sequence number
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
	bool is_empty() const { return (m_itemlist.count() == 0); }
	void get_user_settings(user_settings &settings) const { settings = m_user; }

	// setters
	void set_overlay(bitmap_argb32 *bitmap);
	void set_user_settings(const user_settings &settings);

	// empty the item list
	void empty() { m_item_allocator.reclaim_all(m_itemlist); }

	// add items to the list
	void add_line(float x0, float y0, float x1, float y1, float width, rgb_t argb, UINT32 flags);
	void add_quad(float x0, float y0, float x1, float y1, rgb_t argb, render_texture *texture, UINT32 flags);
	void add_char(float x0, float y0, float height, float aspect, rgb_t argb, render_font &font, UINT16 ch);
	void add_point(float x0, float y0, float diameter, rgb_t argb, UINT32 flags) { add_line(x0, y0, x0, y0, diameter, argb, flags); }
	void add_rect(float x0, float y0, float x1, float y1, rgb_t argb, UINT32 flags) { add_quad(x0, y0, x1, y1, argb, nullptr, flags); }

	// brightness/contrast/gamma helpers
	bool has_brightness_contrast_gamma_changes() const { return (m_user.m_brightness != 1.0f || m_user.m_contrast != 1.0f || m_user.m_gamma != 1.0f); }
	UINT8 apply_brightness_contrast_gamma(UINT8 value);
	float apply_brightness_contrast_gamma_fp(float value);
	const rgb_t *bcg_lookup_table(int texformat, palette_t *palette = nullptr);

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
		UINT8 type() const { return m_type; }
		const render_bounds &bounds() const { return m_bounds; }
		const render_color &color() const { return m_color; }
		UINT32 flags() const { return m_flags; }
		UINT32 internal() const { return m_internal; }
		float width() const { return m_width; }
		render_texture *texture() const { return m_texture; }

	private:
		// internal state
		item *              m_next;             // pointer to the next element in the list
		UINT8               m_type;             // type of element
		render_bounds       m_bounds;           // bounds of the element
		render_color        m_color;            // RGBA factors
		UINT32              m_flags;            // option flags
		UINT32              m_internal;         // internal flags
		float               m_width;            // width of the line (lines only)
		render_texture *    m_texture;          // pointer to the source texture (quads only)
	};

	// generic screen overlay scaler
	static void overlay_scale(bitmap_argb32 &dest, bitmap_argb32 &source, const rectangle &sbounds, void *param);

	// internal helpers
	item *first_item() const { return m_itemlist.first(); }
	item &add_generic(UINT8 type, float x0, float y0, float x1, float y1, rgb_t argb);
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
	std::unique_ptr<palette_client> m_palclient;       // client to the screen palette
	std::vector<rgb_t>           m_bcglookup;            // copy of screen palette with bcg adjustment
	rgb_t                   m_bcglookup256[0x400];  // lookup table for brightness/contrast/gamma
};



//**************************************************************************
//  CONSTANTS
//**************************************************************************

enum item_layer
{
	ITEM_LAYER_FIRST = 0,
	ITEM_LAYER_BACKDROP = ITEM_LAYER_FIRST,
	ITEM_LAYER_SCREEN,
	ITEM_LAYER_OVERLAY,
	ITEM_LAYER_BEZEL,
	ITEM_LAYER_CPANEL,
	ITEM_LAYER_MARQUEE,
	ITEM_LAYER_MAX
};
DECLARE_ENUM_OPERATORS(item_layer)



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************


// ======================> layout_element

// a layout_element is a single named element, which may have multiple components
class layout_element
{
	friend class simple_list<layout_element>;

public:
	// construction/destruction
	layout_element(running_machine &machine, xml_data_node &elemnode, const char *dirname);
	virtual ~layout_element();

	// getters
	layout_element *next() const { return m_next; }
	const char *name() const { return m_name.c_str(); }
	running_machine &machine() const { return m_machine; }
	int default_state() const { return m_defstate; }
	int maxstate() const { return m_maxstate; }
	render_texture *state_texture(int state);

private:
	// a component represents an image, rectangle, or disk in an element
	class component
	{
		friend class layout_element;
		friend class simple_list<component>;

	public:
		// construction/destruction
		component(running_machine &machine, xml_data_node &compnode, const char *dirname);
		~component();

		// getters
		component *next() const { return m_next; }
		const render_bounds &bounds() const { return m_bounds; }

		// operations
		void draw(running_machine &machine, bitmap_argb32 &dest, const rectangle &bounds, int state);

	private:
		// component types
		enum component_type
		{
			CTYPE_INVALID = 0,
			CTYPE_IMAGE,
			CTYPE_RECT,
			CTYPE_DISK,
			CTYPE_TEXT,
			CTYPE_LED7SEG,
			CTYPE_LED8SEG_GTS1,
			CTYPE_LED14SEG,
			CTYPE_LED16SEG,
			CTYPE_LED14SEGSC,
			CTYPE_LED16SEGSC,
			CTYPE_DOTMATRIX,
			CTYPE_DOTMATRIX5DOT,
			CTYPE_DOTMATRIXDOT,
			CTYPE_SIMPLECOUNTER,
			CTYPE_REEL,
			CTYPE_MAX
		};

		// helpers
		void draw_rect(bitmap_argb32 &dest, const rectangle &bounds);
		void draw_disk(bitmap_argb32 &dest, const rectangle &bounds);
		void draw_text(running_machine &machine, bitmap_argb32 &dest, const rectangle &bounds);
		void draw_simplecounter(running_machine &machine, bitmap_argb32 &dest, const rectangle &bounds, int state);
		void draw_reel(running_machine &machine, bitmap_argb32 &dest, const rectangle &bounds, int state);
		void draw_beltreel(running_machine &machine, bitmap_argb32 &dest, const rectangle &bounds, int state);
		void load_bitmap();
		void load_reel_bitmap(int number);
		void draw_led7seg(bitmap_argb32 &dest, const rectangle &bounds, int pattern);
		void draw_led8seg_gts1(bitmap_argb32 &dest, const rectangle &bounds, int pattern);
		void draw_led14seg(bitmap_argb32 &dest, const rectangle &bounds, int pattern);
		void draw_led14segsc(bitmap_argb32 &dest, const rectangle &bounds, int pattern);
		void draw_led16seg(bitmap_argb32 &dest, const rectangle &bounds, int pattern);
		void draw_led16segsc(bitmap_argb32 &dest, const rectangle &bounds, int pattern);
		void draw_dotmatrix(int dots,bitmap_argb32 &dest, const rectangle &bounds, int pattern);
		void draw_segment_horizontal_caps(bitmap_argb32 &dest, int minx, int maxx, int midy, int width, int caps, rgb_t color);
		void draw_segment_horizontal(bitmap_argb32 &dest, int minx, int maxx, int midy, int width, rgb_t color);
		void draw_segment_vertical_caps(bitmap_argb32 &dest, int miny, int maxy, int midx, int width, int caps, rgb_t color);
		void draw_segment_vertical(bitmap_argb32 &dest, int miny, int maxy, int midx, int width, rgb_t color);
		void draw_segment_diagonal_1(bitmap_argb32 &dest, int minx, int maxx, int miny, int maxy, int width, rgb_t color);
		void draw_segment_diagonal_2(bitmap_argb32 &dest, int minx, int maxx, int miny, int maxy, int width, rgb_t color);
		void draw_segment_decimal(bitmap_argb32 &dest, int midx, int midy, int width, rgb_t color);
		void draw_segment_comma(bitmap_argb32 &dest, int minx, int maxx, int miny, int maxy, int width, rgb_t color);
		void apply_skew(bitmap_argb32 &dest, int skewwidth);

		#define MAX_BITMAPS 32

		// internal state
		component *         m_next;                     // link to next component
		component_type      m_type;                     // type of component
		int                 m_state;                    // state where this component is visible (-1 means all states)
		render_bounds       m_bounds;                   // bounds of the element
		render_color        m_color;                    // color of the element
		std::string         m_string;                   // string for text components
		int                 m_digits;                   // number of digits for simple counters
		int                 m_textalign;                // text alignment to box
		bitmap_argb32       m_bitmap[MAX_BITMAPS];      // source bitmap for images
		std::string         m_dirname;                  // directory name of image file (for lazy loading)
		std::unique_ptr<emu_file> m_file[MAX_BITMAPS];        // file object for reading image/alpha files
		std::string         m_imagefile[MAX_BITMAPS];   // name of the image file (for lazy loading)
		std::string         m_alphafile[MAX_BITMAPS];   // name of the alpha file (for lazy loading)
		bool                m_hasalpha[MAX_BITMAPS];    // is there any alpha component present?

		// stuff for fruit machine reels
		// basically made up of multiple text strings / gfx
		int                 m_numstops;
		std::string         m_stopnames[MAX_BITMAPS];
		int                 m_stateoffset;
		int                 m_reelreversed;
		int                 m_numsymbolsvisible;
		int                 m_beltreel;
	};

	// a texture encapsulates a texture for a given element in a given state
	class texture
	{
	public:
		texture();
		~texture();

		layout_element *    m_element;      // pointer back to the element
		render_texture *    m_texture;      // texture for this state
		int                 m_state;        // associated state number
	};

	// internal helpers
	static void element_scale(bitmap_argb32 &dest, bitmap_argb32 &source, const rectangle &sbounds, void *param);

	// internal state
	layout_element *    m_next;             // link to next element
	running_machine &   m_machine;          // reference to the owning machine
	std::string         m_name;             // name of this element
	simple_list<component> m_complist;      // list of components
	int                 m_defstate;         // default state of this element
	int                 m_maxstate;         // maximum state value for all components
	std::vector<texture>     m_elemtex;       // array of element textures used for managing the scaled bitmaps
};


// ======================> layout_view

// a layout_view encapsulates a named list of items
class layout_view
{
	friend class simple_list<layout_view>;

public:
	// an item is a single backdrop, screen, overlay, bezel, cpanel, or marquee item
	class item
	{
		friend class layout_view;
		friend class simple_list<item>;

	public:
		// construction/destruction
		item(running_machine &machine, xml_data_node &itemnode, simple_list<layout_element> &elemlist);
		virtual ~item();

		// getters
		item *next() const { return m_next; }
		layout_element *element() const { return m_element; }
		screen_device *screen() { return m_screen; }
		const render_bounds &bounds() const { return m_bounds; }
		const render_color &color() const { return m_color; }
		int orientation() const { return m_orientation; }
		render_container *screen_container(running_machine &machine) const;
		bool has_input() const { return !m_input_tag.empty(); }
		ioport_port *input_tag_and_mask(ioport_value &mask) const { mask = m_input_mask; return m_input_port; };

		// fetch state based on configured source
		int state() const;

		// resolve tags, if any
		void resolve_tags();

	private:
		// internal state
		item *              m_next;             // link to next item
		layout_element *    m_element;          // pointer to the associated element (non-screens only)
		std::string         m_output_name;      // name of this item
		std::string         m_input_tag;        // input tag of this item
		ioport_port *       m_input_port;       // input port of this item
		ioport_value        m_input_mask;       // input mask of this item
		screen_device *     m_screen;           // pointer to screen
		int                 m_orientation;      // orientation of this item
		render_bounds       m_bounds;           // bounds of the item
		render_bounds       m_rawbounds;        // raw (original) bounds of the item
		render_color        m_color;            // color of the item
	};

	// construction/destruction
	layout_view(running_machine &machine, xml_data_node &viewnode, simple_list<layout_element> &elemlist);
	virtual ~layout_view();

	// getters
	layout_view *next() const { return m_next; }
	item *first_item(item_layer layer) const;
	const char *name() const { return m_name.c_str(); }
	const render_bounds &bounds() const { return m_bounds; }
	const render_bounds &screen_bounds() const { return m_scrbounds; }
	const render_screen_list &screens() const { return m_screens; }
	bool layer_enabled(item_layer layer) const { return m_layenabled[layer]; }

	//
	bool has_art() const { return (m_backdrop_list.count() + m_overlay_list.count() + m_bezel_list.count() + m_cpanel_list.count() + m_marquee_list.count() != 0); }
	float effective_aspect(render_layer_config config) const { return (config.zoom_to_screen() && m_screens.count() != 0) ? m_scraspect : m_aspect; }

	// operations
	void recompute(render_layer_config layerconfig);

	// resolve tags, if any
	void resolve_tags();

private:
	// internal state
	layout_view *       m_next;             // pointer to next layout in the list
	std::string         m_name;             // name of the layout
	float               m_aspect;           // X/Y of the layout
	float               m_scraspect;        // X/Y of the screen areas
	render_screen_list  m_screens;          // list of active screens
	render_bounds       m_bounds;           // computed bounds of the view
	render_bounds       m_scrbounds;        // computed bounds of the screens within the view
	render_bounds       m_expbounds;        // explicit bounds of the view
	bool                m_layenabled[ITEM_LAYER_MAX]; // is this layer enabled?
	simple_list<item>   m_backdrop_list;    // list of backdrop items
	simple_list<item>   m_screen_list;      // list of screen items
	simple_list<item>   m_overlay_list;     // list of overlay items
	simple_list<item>   m_bezel_list;       // list of bezel items
	simple_list<item>   m_cpanel_list;      // list of marquee items
	simple_list<item>   m_marquee_list;     // list of marquee items
};


// ======================> layout_file

// a layout_file consists of a list of elements and a list of views
class layout_file
{
	friend class simple_list<layout_file>;

public:
	// construction/destruction
	layout_file(running_machine &machine, xml_data_node &rootnode, const char *dirname);
	virtual ~layout_file();

	// getters
	layout_file *next() const { return m_next; }
	layout_element *first_element() const { return m_elemlist.first(); }
	layout_view *first_view() const { return m_viewlist.first(); }

private:
	// internal state
	layout_file *       m_next;             // pointer to the next file in the list
	simple_list<layout_element> m_elemlist; // list of shared layout elements
	simple_list<layout_view> m_viewlist;    // list of views
};

// ======================> render_target

// a render_target describes a surface that is being rendered to
class render_target
{
	friend resource_pool_object<render_target>::~resource_pool_object();
	friend class simple_list<render_target>;
	friend class render_manager;

	// construction/destruction
	render_target(render_manager &manager, const char *layoutfile = nullptr, UINT32 flags = 0);
	~render_target();

public:
	// getters
	render_target *next() const { return m_next; }
	render_manager &manager() const { return m_manager; }
	UINT32 width() const { return m_width; }
	UINT32 height() const { return m_height; }
	float pixel_aspect() const { return m_pixel_aspect; }
	float max_update_rate() const { return m_max_refresh; }
	int orientation() const { return m_orientation; }
	render_layer_config layer_config() const { return m_layerconfig; }
	layout_view *current_view() const { return m_curview; }
	int view() const { return view_index(*m_curview); }
	bool hidden() const { return ((m_flags & RENDER_CREATE_HIDDEN) != 0); }
	bool is_ui_target() const;
	int index() const;

	// setters
	void set_bounds(INT32 width, INT32 height, float pixel_aspect = 0);
	void set_max_update_rate(float updates_per_second) { m_max_refresh = updates_per_second; }
	void set_orientation(int orientation) { m_orientation = orientation; }
	void set_view(int viewindex);
	void set_max_texture_size(int maxwidth, int maxheight);
	void set_transform_primitives(bool transform_primitives) { m_transform_primitives = transform_primitives; }

	// layer config getters
	bool backdrops_enabled() const { return m_layerconfig.backdrops_enabled(); }
	bool overlays_enabled() const { return m_layerconfig.overlays_enabled(); }
	bool bezels_enabled() const { return m_layerconfig.bezels_enabled(); }
	bool cpanels_enabled() const { return m_layerconfig.cpanels_enabled(); }
	bool marquees_enabled() const { return m_layerconfig.marquees_enabled(); }
	bool screen_overlay_enabled() const { return m_layerconfig.screen_overlay_enabled(); }
	bool zoom_to_screen() const { return m_layerconfig.zoom_to_screen(); }

	// layer config setters
	void set_backdrops_enabled(bool enable) { m_layerconfig.set_backdrops_enabled(enable); update_layer_config(); }
	void set_overlays_enabled(bool enable) { m_layerconfig.set_overlays_enabled(enable); update_layer_config(); }
	void set_bezels_enabled(bool enable) { m_layerconfig.set_bezels_enabled(enable); update_layer_config(); }
	void set_cpanels_enabled(bool enable) { m_layerconfig.set_cpanels_enabled(enable); update_layer_config(); }
	void set_marquees_enabled(bool enable) { m_layerconfig.set_marquees_enabled(enable); update_layer_config(); }
	void set_screen_overlay_enabled(bool enable) { m_layerconfig.set_screen_overlay_enabled(enable); update_layer_config(); }
	void set_zoom_to_screen(bool zoom) { m_layerconfig.set_zoom_to_screen(zoom); update_layer_config(); }

	// view configuration helper
	int configured_view(const char *viewname, int targetindex, int numtargets);

	// view information
	const char *view_name(int viewindex);
	const render_screen_list &view_screens(int viewindex);

	// bounds computations
	void compute_visible_area(INT32 target_width, INT32 target_height, float target_pixel_aspect, int target_orientation, INT32 &visible_width, INT32 &visible_height);
	void compute_minimum_size(INT32 &minwidth, INT32 &minheight);

	// get a primitive list
	render_primitive_list &get_primitives();

	// hit testing
	bool map_point_container(INT32 target_x, INT32 target_y, render_container &container, float &container_x, float &container_y);
	bool map_point_input(INT32 target_x, INT32 target_y, ioport_port *&input_port, ioport_value &input_mask, float &input_x, float &input_y);

	// reference tracking
	void invalidate_all(void *refptr);

	// debug containers
	render_container *debug_alloc();
	void debug_free(render_container &container);
	void debug_append(render_container &container);

	// resolve tag lookups
	void resolve_tags();

private:
	// internal helpers
	void update_layer_config();
	void load_layout_files(const char *layoutfile, bool singlefile);
	bool load_layout_file(const char *dirname, const char *filename);
	void add_container_primitives(render_primitive_list &list, const object_transform &xform, render_container &container, int blendmode);
	void add_element_primitives(render_primitive_list &list, const object_transform &xform, layout_element &element, int state, int blendmode);
	bool map_point_internal(INT32 target_x, INT32 target_y, render_container *container, float &mapped_x, float &mapped_y, ioport_port *&mapped_input_port, ioport_value &mapped_input_mask);

	// config callbacks
	void config_load(xml_data_node &targetnode);
	bool config_save(xml_data_node &targetnode);

	// view lookups
	layout_view *view_by_index(int index) const;
	int view_index(layout_view &view) const;

	// optimized clearing
	void init_clear_extents();
	bool remove_clear_extent(const render_bounds &bounds);
	void add_clear_extents(render_primitive_list &list);
	void add_clear_and_optimize_primitive_list(render_primitive_list &list);

	// constants
	static const int NUM_PRIMLISTS = 3;
	static const int MAX_CLEAR_EXTENTS = 1000;

	// internal state
	render_target *         m_next;                     // link to next target
	render_manager &        m_manager;                  // reference to our owning manager
	layout_view *           m_curview;                  // current view
	simple_list<layout_file> m_filelist;                // list of layout files
	UINT32                  m_flags;                    // creation flags
	render_primitive_list   m_primlist[NUM_PRIMLISTS];  // list of primitives
	int                     m_listindex;                // index of next primlist to use
	INT32                   m_width;                    // width in pixels
	INT32                   m_height;                   // height in pixels
	render_bounds           m_bounds;                   // bounds of the target
	float                   m_pixel_aspect;             // aspect ratio of individual pixels
	float                   m_max_refresh;              // maximum refresh rate, 0 or if none
	int                     m_orientation;              // orientation
	render_layer_config     m_layerconfig;              // layer configuration
	layout_view *           m_base_view;                // the view at the time of first frame
	int                     m_base_orientation;         // the orientation at the time of first frame
	render_layer_config     m_base_layerconfig;         // the layer configuration at the time of first frame
	int                     m_maxtexwidth;              // maximum width of a texture
	int                     m_maxtexheight;             // maximum height of a texture
	simple_list<render_container> m_debug_containers;   // list of debug containers
	INT32                   m_clear_extent_count;       // number of clear extents
	INT32                   m_clear_extents[MAX_CLEAR_EXTENTS]; // array of clear extents
	bool                    m_transform_primitives;     // determines if the primitives shall be scaled/offset by screen settings,
														// otherwise the respective render API will handle it (default is true)

	static render_screen_list s_empty_screen_list;
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
	render_target *target_alloc(const char *layoutfile = nullptr, UINT32 flags = 0);
	void target_free(render_target *target);
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
	render_font *font_alloc(const char *filename = nullptr);
	void font_free(render_font *font);

	// reference tracking
	void invalidate_all(void *refptr);

	// resolve tag lookups
	void resolve_tags();

private:
	// containers
	render_container *container_alloc(screen_device *screen = nullptr);
	void container_free(render_container *container);

	// config callbacks
	void config_load(int config_type, xml_data_node *parentnode);
	void config_save(int config_type, xml_data_node *parentnode);

	// internal state
	running_machine &               m_machine;          // reference back to the machine

	// array of live targets
	simple_list<render_target>      m_targetlist;       // list of targets
	render_target *                 m_ui_target;        // current UI target

	// texture lists
	UINT32                          m_live_textures;    // number of live textures
	fixed_allocator<render_texture> m_texture_allocator;// texture allocator

	// containers for the UI and for screens
	render_container *              m_ui_container;     // UI container
	simple_list<render_container>   m_screen_container_list; // list of containers for the screen
};

#endif  // __RENDER_H__
