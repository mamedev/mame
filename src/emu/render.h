/***************************************************************************

    render.h

    Core rendering routines for MAME.

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

#include "osdepend.h"

#include <math.h>



//**************************************************************************
//  CONSTANTS
//**************************************************************************

// texture formats
enum
{
	TEXFORMAT_UNDEFINED = 0,							// require a format to be specified
	TEXFORMAT_PALETTE16,								// 16bpp palettized, alpha ignored
	TEXFORMAT_PALETTEA16,								// 16bpp palettized, alpha respected
	TEXFORMAT_RGB15,									// 16bpp 5-5-5 RGB
	TEXFORMAT_RGB32,									// 32bpp 8-8-8 RGB
	TEXFORMAT_ARGB32,									// 32bpp 8-8-8-8 ARGB
	TEXFORMAT_YUY16										// 16bpp 8-8 Y/Cb, Y/Cr in sequence
};

// blending modes
enum
{
	BLENDMODE_NONE = 0,									// no blending
	BLENDMODE_ALPHA,									// standard alpha blend
	BLENDMODE_RGB_MULTIPLY,								// apply source alpha to source pix, then multiply RGB values
	BLENDMODE_ADD										// apply source alpha to source pix, then add to destination
};


// render creation flags
const UINT8 RENDER_CREATE_NO_ART		= 0x01;			// ignore any views that have art in them
const UINT8 RENDER_CREATE_SINGLE_FILE	= 0x02;			// only load views from the file specified
const UINT8 RENDER_CREATE_HIDDEN		= 0x04;			// don't make this target visible


// layer config masks
const UINT8 LAYER_CONFIG_ENABLE_BACKDROP		= 0x01;	// enable backdrop layers
const UINT8 LAYER_CONFIG_ENABLE_OVERLAY			= 0x02;	// enable overlay layers
const UINT8 LAYER_CONFIG_ENABLE_BEZEL			= 0x04;	// enable bezel layers
const UINT8 LAYER_CONFIG_ZOOM_TO_SCREEN			= 0x08;	// zoom to screen area by default
const UINT8 LAYER_CONFIG_ENABLE_SCREEN_OVERLAY	= 0x10;	// enable screen overlays

const UINT8 LAYER_CONFIG_DEFAULT =	(LAYER_CONFIG_ENABLE_BACKDROP |
									 LAYER_CONFIG_ENABLE_OVERLAY |
									 LAYER_CONFIG_ENABLE_BEZEL |
									 LAYER_CONFIG_ENABLE_SCREEN_OVERLAY);


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



//**************************************************************************
//  MACROS
//**************************************************************************

#define PRIMFLAG_TEXORIENT(x)		((x) << PRIMFLAG_TEXORIENT_SHIFT)
#define PRIMFLAG_GET_TEXORIENT(x)	(((x) & PRIMFLAG_TEXORIENT_MASK) >> PRIMFLAG_TEXORIENT_SHIFT)

#define PRIMFLAG_TEXFORMAT(x)		((x) << PRIMFLAG_TEXFORMAT_SHIFT)
#define PRIMFLAG_GET_TEXFORMAT(x)	(((x) & PRIMFLAG_TEXFORMAT_MASK) >> PRIMFLAG_TEXFORMAT_SHIFT)

#define PRIMFLAG_BLENDMODE(x)		((x) << PRIMFLAG_BLENDMODE_SHIFT)
#define PRIMFLAG_GET_BLENDMODE(x)	(((x) & PRIMFLAG_BLENDMODE_MASK) >> PRIMFLAG_BLENDMODE_SHIFT)

#define PRIMFLAG_ANTIALIAS(x)		((x) << PRIMFLAG_ANTIALIAS_SHIFT)
#define PRIMFLAG_GET_ANTIALIAS(x)	(((x) & PRIMFLAG_ANTIALIAS_MASK) >> PRIMFLAG_ANTIALIAS_SHIFT)

#define PRIMFLAG_SCREENTEX(x)		((x) << PRIMFLAG_SCREENTEX_SHIFT)
#define PRIMFLAG_GET_SCREENTEX(x)	(((x) & PRIMFLAG_SCREENTEX_MASK) >> PRIMFLAG_SCREENTEX_SHIFT)

#define PRIMFLAG_TEXWRAP(x)			((x) << PRIMFLAG_TEXWRAP_SHIFT)
#define PRIMFLAG_GET_TEXWRAP(x)		(((x) & PRIMFLAG_TEXWRAP_MASK) >> PRIMFLAG_TEXWRAP_SHIFT)



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// forward definitions
class device_t;
class screen_device;
class render_container;
class render_manager;
typedef struct _xml_data_node xml_data_node;
typedef struct _render_font render_font;
struct object_transform;
typedef struct _layout_element layout_element;
typedef struct _layout_view layout_view;
typedef struct _view_item view_item;
typedef struct _layout_file layout_file;


// texture scaling callback
typedef void (*texture_scaler_func)(bitmap_t &dest, const bitmap_t &source, const rectangle &sbounds, void *param);


// render_bounds - floating point bounding rectangle
struct render_bounds
{
	float				x0;					// leftmost X coordinate
	float				y0;					// topmost Y coordinate
	float				x1;					// rightmost X coordinate
	float				y1;					// bottommost Y coordinate
};


// render_color - floating point set of ARGB values
struct render_color
{
	float				a;					// alpha component (0.0 = transparent, 1.0 = opaque)
	float				r;					// red component (0.0 = none, 1.0 = max)
	float				g;					// green component (0.0 = none, 1.0 = max)
	float				b;					// blue component (0.0 = none, 1.0 = max)
};


// render_texuv - floating point set of UV texture coordinates
struct render_texuv
{
	float				u;					// U coodinate (0.0-1.0)
	float				v;					// V coordinate (0.0-1.0)
};


// render_quad_texuv - floating point set of UV texture coordinates
struct render_quad_texuv
{
	render_texuv		tl;					// top-left UV coordinate
	render_texuv		tr;					// top-right UV coordinate
	render_texuv		bl;					// bottom-left UV coordinate
	render_texuv		br;					// bottom-right UV coordinate
};


// render_texinfo - texture information
struct render_texinfo
{
	void *				base;				// base of the data
	UINT32				rowpixels;			// pixels per row
	UINT32				width;				// width of the image
	UINT32				height;				// height of the image
	const rgb_t *		palette;			// palette for PALETTE16 textures, LUTs for RGB15/RGB32
	UINT32				seqid;				// sequence ID
};


// ======================> render_primitive

// render_primitive - a single low-level primitive for the rendering engine
class render_primitive
{
	friend class simple_list<render_primitive>;

public:
	// render primitive types
	enum primitive_type
	{
		INVALID = 0,						// invalid type
		LINE,								// a single line
		QUAD								// a rectilinear quad
	};

	// getters
	render_primitive *next() const { return m_next; }

	// reset to prepare for re-use
	void reset();

	// public state
	primitive_type		type;				// type of primitive
	render_bounds		bounds;				// bounds or positions
	render_color		color;				// RGBA values
	UINT32				flags;				// flags
	float				width;				// width (for line primitives)
	render_texinfo		texture;			// texture info (for quad primitives)
	render_quad_texuv	texcoords;			// texture coordinates (for quad primitives)

private:
	// internal state
	render_primitive *	m_next;				// pointer to next element
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
		reference *			m_next;				// link to the next reference
		void *				m_refptr;			// reference pointer
	};

	// internal state
	simple_list<render_primitive> m_primlist;				// list of primitives
	simple_list<reference> m_reflist;						// list of references

	fixed_allocator<render_primitive> m_primitive_allocator;// allocator for primitives
	fixed_allocator<reference> m_reference_allocator;		// allocator for references

	osd_lock *			m_lock;								// lock to protect list accesses
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
	void reset(render_manager &manager, texture_scaler_func scaler = NULL, void *param = NULL);

public:
	// getters
	int format() const { return m_format; }

	// configure the texture bitmap
	void set_bitmap(bitmap_t *bitmap, const rectangle *sbounds, int format, palette_t *palette = NULL);

	// generic high-quality bitmap scaler
	static void hq_scale(bitmap_t &dest, const bitmap_t &source, const rectangle &sbounds, void *param);

private:
	// internal helpers
	bool get_scaled(UINT32 dwidth, UINT32 dheight, render_texinfo &texinfo, render_primitive_list &primlist);
	const rgb_t *get_adjusted_palette(render_container &container);

	static const int MAX_TEXTURE_SCALES = 8;

	// a scaled_texture contains a single scaled entry for a texture
	struct scaled_texture
	{
		bitmap_t *			bitmap;					// final bitmap
		UINT32				seqid;					// sequence number
	};

	// internal state
	render_manager *	m_manager;					// reference to our manager
	render_texture *	m_next;						// next texture (for free list)
	bitmap_t *			m_bitmap;					// pointer to the original bitmap
	rectangle			m_sbounds;					// source bounds within the bitmap
	palette_t *			m_palette;					// palette associated with the texture
	int					m_format;					// format of the texture data
	texture_scaler_func	m_scaler;					// scaling callback
	void *				m_param;					// scaling callback parameter
	UINT32				m_curseq;					// current sequence number
	scaled_texture		m_scaled[MAX_TEXTURE_SCALES];// array of scaled variants of this texture
	rgb_t *				m_bcglookup;				// dynamically allocated B/C/G lookup table
	UINT32				m_bcglookup_entries;		// number of B/C/G lookup entries allocated
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
	render_container(render_manager &manager, screen_device *screen = NULL);
	~render_container();

public:
	// user settings describes the collected user-controllable settings
	struct user_settings
	{
		// construction/destruction
		user_settings();

		// public state
		int					m_orientation;		// orientation
		float				m_brightness;		// brightness
		float				m_contrast;			// contrast
		float				m_gamma;			// gamma
		float				m_xscale;			// horizontal scale factor
		float				m_yscale;			// vertical scale factor
		float				m_xoffset;			// horizontal offset
		float				m_yoffset;			// vertical offset
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
	void set_overlay(bitmap_t *bitmap);
	void set_user_settings(const user_settings &settings);

	// empty the item list
	void empty() { m_item_allocator.reclaim_all(m_itemlist); }

	// add items to the list
	void add_line(float x0, float y0, float x1, float y1, float width, rgb_t argb, UINT32 flags);
	void add_quad(float x0, float y0, float x1, float y1, rgb_t argb, render_texture *texture, UINT32 flags);
	void add_char(float x0, float y0, float height, float aspect, rgb_t argb, render_font &font, UINT16 ch);
	void add_point(float x0, float y0, float diameter, rgb_t argb, UINT32 flags) { add_line(x0, y0, x0, y0, diameter, argb, flags); }
	void add_rect(float x0, float y0, float x1, float y1, rgb_t argb, UINT32 flags) { add_quad(x0, y0, x1, y1, argb, NULL, flags); }

	// brightness/contrast/gamma helpers
	bool has_brightness_contrast_gamma_changes() const { return (m_user.m_brightness != 1.0f || m_user.m_contrast != 1.0f || m_user.m_gamma != 1.0f); }
	UINT8 apply_brightness_contrast_gamma(UINT8 value);
	float apply_brightness_contrast_gamma_fp(float value);
	const rgb_t *bcg_lookup_table(int texformat, palette_t *palette = NULL);

private:
	// an item describes a high level primitive that is added to a container
	class item
	{
		friend class render_container;
		friend class simple_list<item>;

	public:
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
		item *				m_next;				// pointer to the next element in the list
		UINT8				m_type;				// type of element
		render_bounds		m_bounds;			// bounds of the element
		render_color		m_color;			// RGBA factors
		UINT32				m_flags;			// option flags
		UINT32				m_internal;			// internal flags
		float				m_width;			// width of the line (lines only)
		render_texture *	m_texture;			// pointer to the source texture (quads only)
	};

	// generic screen overlay scaler
	static void overlay_scale(bitmap_t &dest, const bitmap_t &source, const rectangle &sbounds, void *param);

	// internal helpers
	item *first_item() const { return m_itemlist.first(); }
	item &add_generic(UINT8 type, float x0, float y0, float x1, float y1, rgb_t argb);
	void recompute_lookups();
	void update_palette();

	// internal state
	render_container *		m_next;					// the next container in the list
	render_manager &		m_manager;				// reference back to the owning manager
	simple_list<item>		m_itemlist;				// head of the item list
	fixed_allocator<item>	m_item_allocator;		// free container items
	screen_device *			m_screen;				// the screen device
	user_settings			m_user;					// user settings
	bitmap_t *				m_overlaybitmap;		// overlay bitmap
	render_texture *		m_overlaytexture;		// overlay texture
	palette_client *		m_palclient;			// client to the system palette
	rgb_t					m_bcglookup256[0x400];	// lookup table for brightness/contrast/gamma
	rgb_t					m_bcglookup32[0x80];	// lookup table for brightness/contrast/gamma
	rgb_t					m_bcglookup[0x10000];	// full palette lookup with bcg adjustements
};


// ======================> render_target

// a render_target describes a surface that is being rendered to
class render_target
{
	friend resource_pool_object<render_target>::~resource_pool_object();
	friend class simple_list<render_target>;
	friend class render_manager;

	// construction/destruction
	render_target(render_manager &manager, const char *layoutfile = NULL, UINT32 flags = 0);
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
	int layer_config() const { return m_layerconfig; }
	int view() const { return view_index(*m_curview); }
	bool hidden() const { return ((m_flags & RENDER_CREATE_HIDDEN) != 0); }
	bool backdrops_enabled() const { return (m_layerconfig & LAYER_CONFIG_ENABLE_BACKDROP) != 0; }
	bool overlays_enabled() const { return (m_layerconfig & LAYER_CONFIG_ENABLE_OVERLAY) != 0; }
	bool bezels_enabled() const { return (m_layerconfig & LAYER_CONFIG_ENABLE_BEZEL) != 0; }
	bool screen_overlay_enabled() const { return (m_layerconfig & LAYER_CONFIG_ENABLE_SCREEN_OVERLAY) != 0; }
	bool zoom_to_screen() const { return (m_layerconfig & LAYER_CONFIG_ZOOM_TO_SCREEN) != 0; }
	bool is_ui_target() const;
	int index() const;

	// setters
	void set_bounds(INT32 width, INT32 height, float pixel_aspect = 0);
	void set_max_update_rate(float updates_per_second) { m_max_refresh = updates_per_second; }
	void set_orientation(int orientation) { m_orientation = orientation; }
	void set_layer_config(int layerconfig);
	void set_view(int viewindex);
	void set_max_texture_size(int maxwidth, int maxheight);
	void set_backdrops_enabled(bool enable) { if (enable) m_layerconfig |= LAYER_CONFIG_ENABLE_BACKDROP; else m_layerconfig &= ~LAYER_CONFIG_ENABLE_BACKDROP; }
	void set_overlays_enabled(bool enable) { if (enable) m_layerconfig |= LAYER_CONFIG_ENABLE_OVERLAY; else m_layerconfig &= ~LAYER_CONFIG_ENABLE_OVERLAY; }
	void set_bezels_enabled(bool enable) { if (enable) m_layerconfig |= LAYER_CONFIG_ENABLE_BEZEL; else m_layerconfig &= ~LAYER_CONFIG_ENABLE_BEZEL; }
	void set_screen_overlay_enabled(bool enable) { if (enable) m_layerconfig |= LAYER_CONFIG_ENABLE_SCREEN_OVERLAY; else m_layerconfig &= ~LAYER_CONFIG_ENABLE_SCREEN_OVERLAY; }
	void set_zoom_to_screen(bool zoom) { if (zoom) m_layerconfig |= LAYER_CONFIG_ZOOM_TO_SCREEN; else m_layerconfig &= ~LAYER_CONFIG_ZOOM_TO_SCREEN; }

	// view information
	const char *view_name(int viewindex);
	UINT32 view_screens(int viewindex);

	// bounds computations
	void compute_visible_area(INT32 target_width, INT32 target_height, float target_pixel_aspect, int target_orientation, INT32 &visible_width, INT32 &visible_height);
	void compute_minimum_size(INT32 &minwidth, INT32 &minheight);

	// get a primitive list
	render_primitive_list &get_primitives();

	// hit testing
	bool map_point_container(INT32 target_x, INT32 target_y, render_container &container, float &container_x, float &container_y);
	bool map_point_input(INT32 target_x, INT32 target_y, const char *&input_tag, UINT32 &input_mask, float &input_x, float &input_y);

	// reference tracking
	void invalidate_all(void *refptr);

	// debug containers
	render_container *debug_alloc();
	void debug_free(render_container &container);
	void debug_top(render_container &container);

private:
	// internal helpers
	void load_layout_files(const char *layoutfile, bool singlefile);
	void add_container_primitives(render_primitive_list &list, const object_transform &xform, render_container &container, int blendmode);
	void add_element_primitives(render_primitive_list &list, const object_transform &xform, const layout_element &element, int state, int blendmode);
	bool map_point_internal(INT32 target_x, INT32 target_y, render_container *container, float &mapped_x, float &mapped_y, view_item *&mapped_item);

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
	render_target *			m_next;						// link to next target
	render_manager &		m_manager;					// reference to our owning manager
	layout_view *			m_curview;					// current view
	layout_file *			m_filelist;					// list of layout files
	UINT32					m_flags;					// creation flags
	render_primitive_list	m_primlist[NUM_PRIMLISTS];	// list of primitives
	int						m_listindex;				// index of next primlist to use
	INT32					m_width;					// width in pixels
	INT32					m_height;					// height in pixels
	render_bounds			m_bounds;					// bounds of the target
	float					m_pixel_aspect;				// aspect ratio of individual pixels
	float					m_max_refresh;				// maximum refresh rate, 0 or if none
	int						m_orientation;				// orientation
	int						m_layerconfig;				// layer configuration
	layout_view *			m_base_view;				// the view at the time of first frame
	int						m_base_orientation;			// the orientation at the time of first frame
	int						m_base_layerconfig;			// the layer configuration at the time of first frame
	int						m_maxtexwidth;				// maximum width of a texture
	int						m_maxtexheight;				// maximum height of a texture
	simple_list<render_container> m_debug_containers;	// list of debug containers
	INT32					m_clear_extent_count;		// number of clear extents
	INT32					m_clear_extents[MAX_CLEAR_EXTENTS]; // array of clear extents
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
	render_target *target_alloc(const char *layoutfile = NULL, UINT32 flags = 0);
	void target_free(render_target *target);
	render_target *first_target() const { return m_targetlist.first(); }
	render_target *target_by_index(int index) const;

	// UI targets
	render_target &ui_target() const { assert(m_ui_target != NULL); return *m_ui_target; }
	void set_ui_target(render_target &target) { m_ui_target = &target; }
	float ui_aspect();

	// screen containers
	render_container *container_for_screen(screen_device *screen);

	// UI containers
	render_container &ui_container() const { assert(m_ui_container != NULL); return *m_ui_container; }

	// textures
	render_texture *texture_alloc(texture_scaler_func scaler = NULL, void *param = NULL);
	void texture_free(render_texture *texture);

	// reference tracking
	void invalidate_all(void *refptr);

private:
	// containers
	render_container *container_alloc(screen_device *screen = NULL);
	void container_free(render_container *container);

	// config callbacks
	static void config_load_static(running_machine *machine, int config_type, xml_data_node *parentnode);
	static void config_save_static(running_machine *machine, int config_type, xml_data_node *parentnode);
	void config_load(int config_type, xml_data_node *parentnode);
	void config_save(int config_type, xml_data_node *parentnode);

	// internal state
	running_machine &				m_machine;			// reference back to the machine

	// array of live targets
	simple_list<render_target>		m_targetlist;		// list of targets
	render_target *					m_ui_target;		// current UI target

	// texture lists
	UINT32							m_live_textures;	// number of live textures
	fixed_allocator<render_texture>	m_texture_allocator;// texture allocator

	// containers for the UI and for screens
	render_container *				m_ui_container;		// UI container
	simple_list<render_container>	m_screen_container_list; // list of containers for the screen
};


#endif	// __RENDER_H__
