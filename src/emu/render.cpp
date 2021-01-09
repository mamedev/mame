// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    render.cpp

    Core rendering system.

****************************************************************************

    Windows-specific to-do:
        * no fallback if we run out of video memory

    Longer-term to do: (once old renderer is gone)
        * make vector updates asynchronous

****************************************************************************

    Overview of objects:

        render_target -- This represents a final rendering target. It
            is specified using integer width/height values, can have
            non-square pixels, and you can specify its rotation. It is
            what really determines the final rendering details. The OSD
            layer creates one or more of these to encapsulate the
            rendering process. Each render_target holds a list of
            layout_files that it can use for drawing. When rendering, it
            makes use of both layout_files and render_containers.

        render_container -- Containers are the top of a hierarchy that is
            not directly related to the objects above. Containers hold
            high level primitives that are generated at runtime by the
            video system. They are used currently for each screen and
            the user interface. These high-level primitives are broken down
            into low-level primitives at render time.

***************************************************************************/

#include "emu.h"
#include "render.h"

#include "emuopts.h"
#include "rendfont.h"
#include "rendlay.h"
#include "rendutil.h"
#include "config.h"
#include "drivenum.h"
#include "layout/generic.h"

#include "ui/uimain.h"

#include "xmlfile.h"

#include <zlib.h>

#include <algorithm>



//**************************************************************************
//  CONSTANTS
//**************************************************************************

#define INTERNAL_FLAG_CHAR      0x00000001

enum
{
	COMPONENT_TYPE_IMAGE = 0,
	COMPONENT_TYPE_RECT,
	COMPONENT_TYPE_DISK,
	COMPONENT_TYPE_MAX
};


enum
{
	CONTAINER_ITEM_LINE = 0,
	CONTAINER_ITEM_QUAD,
	CONTAINER_ITEM_MAX
};



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// an object_transform is used to track transformations when building an object list
struct render_target::object_transform
{
	float               xoffs, yoffs;       // offset transforms
	float               xscale, yscale;     // scale transforms
	render_color        color;              // color transform
	int                 orientation;        // orientation transform
	bool                no_center;          // center the container?
};



//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

// precomputed UV coordinates for various orientations
static const render_quad_texuv oriented_texcoords[8] =
{
	{ { 0,0 }, { 1,0 }, { 0,1 }, { 1,1 } },     // 0
	{ { 1,0 }, { 0,0 }, { 1,1 }, { 0,1 } },     // ORIENTATION_FLIP_X
	{ { 0,1 }, { 1,1 }, { 0,0 }, { 1,0 } },     // ORIENTATION_FLIP_Y
	{ { 1,1 }, { 0,1 }, { 1,0 }, { 0,0 } },     // ORIENTATION_FLIP_X | ORIENTATION_FLIP_Y
	{ { 0,0 }, { 0,1 }, { 1,0 }, { 1,1 } },     // ORIENTATION_SWAP_XY
	{ { 0,1 }, { 0,0 }, { 1,1 }, { 1,0 } },     // ORIENTATION_SWAP_XY | ORIENTATION_FLIP_X
	{ { 1,0 }, { 1,1 }, { 0,0 }, { 0,1 } },     // ORIENTATION_SWAP_XY | ORIENTATION_FLIP_Y
	{ { 1,1 }, { 1,0 }, { 0,1 }, { 0,0 } }      // ORIENTATION_SWAP_XY | ORIENTATION_FLIP_X | ORIENTATION_FLIP_Y
};



//**************************************************************************
//  INLINE FUNCTIONS
//**************************************************************************

//-------------------------------------------------
//  apply_orientation - apply orientation to a
//  set of bounds
//-------------------------------------------------

inline void apply_orientation(render_bounds &bounds, int orientation)
{
	// swap first
	if (orientation & ORIENTATION_SWAP_XY)
	{
		std::swap(bounds.x0, bounds.y0);
		std::swap(bounds.x1, bounds.y1);
	}

	// apply X flip
	if (orientation & ORIENTATION_FLIP_X)
	{
		bounds.x0 = 1.0f - bounds.x0;
		bounds.x1 = 1.0f - bounds.x1;
	}

	// apply Y flip
	if (orientation & ORIENTATION_FLIP_Y)
	{
		bounds.y0 = 1.0f - bounds.y0;
		bounds.y1 = 1.0f - bounds.y1;
	}
}


//-------------------------------------------------
//  normalize_bounds - normalize bounds so that
//  x0/y0 are less than x1/y1
//-------------------------------------------------

inline void normalize_bounds(render_bounds &bounds)
{
	if (bounds.x0 > bounds.x1)
		std::swap(bounds.x0, bounds.x1);
	if (bounds.y0 > bounds.y1)
		std::swap(bounds.y0, bounds.y1);
}


//**************************************************************************
//  RENDER PRIMITIVE
//**************************************************************************

//-------------------------------------------------
//  reset - reset the state of a primitive after
//  it is re-allocated
//-------------------------------------------------

void render_primitive::reset()
{
	// do not clear m_next!
	memset(&type, 0, uintptr_t(&texcoords + 1) - uintptr_t(&type));
}



//**************************************************************************
//  RENDER PRIMITIVE LIST
//**************************************************************************

//-------------------------------------------------
//  render_primitive_list - constructor
//-------------------------------------------------

render_primitive_list::render_primitive_list()
{
}


//-------------------------------------------------
//  ~render_primitive_list - destructor
//-------------------------------------------------

render_primitive_list::~render_primitive_list()
{
	release_all();
}


//-------------------------------------------------
//  add_reference - add a new reference
//-------------------------------------------------

inline void render_primitive_list::add_reference(void *refptr)
{
	// skip if we already have one
	if (has_reference(refptr))
		return;

	// set the refptr and link us into the list
	reference *ref = m_reference_allocator.alloc();
	ref->m_refptr = refptr;
	m_reflist.append(*ref);
}


//-------------------------------------------------
//  has_reference - find a refptr in a reference
//  list
//-------------------------------------------------

inline bool render_primitive_list::has_reference(void *refptr) const
{
	// skip if we already have one
	for (reference &ref : m_reflist)
		if (ref.m_refptr == refptr)
			return true;
	return false;
}


//-------------------------------------------------
//  alloc - allocate a new empty primitive
//-------------------------------------------------

inline render_primitive *render_primitive_list::alloc(render_primitive::primitive_type type)
{
	render_primitive *result = m_primitive_allocator.alloc();
	result->reset();
	result->type = type;
	return result;
}


//-------------------------------------------------
//  release_all - release the contents of
//  a render list
//-------------------------------------------------

void render_primitive_list::release_all()
{
	// release all the live items while under the lock
	m_primitive_allocator.reclaim_all(m_primlist);
	m_reference_allocator.reclaim_all(m_reflist);
}


//-------------------------------------------------
//  append_or_return - append a primitive to the
//  end of the list, or return it to the free
//  list, based on a flag
//-------------------------------------------------

void render_primitive_list::append_or_return(render_primitive &prim, bool clipped)
{
	if (!clipped)
		m_primlist.append(prim);
	else
		m_primitive_allocator.reclaim(prim);
}



//**************************************************************************
//  RENDER TEXTURE
//**************************************************************************

//-------------------------------------------------
//  render_texture - constructor
//-------------------------------------------------

render_texture::render_texture()
	: m_manager(nullptr),
		m_next(nullptr),
		m_bitmap(nullptr),
		m_format(TEXFORMAT_ARGB32),
		m_id(~0ULL),
		m_old_id(~0ULL),
		m_scaler(nullptr),
		m_param(nullptr),
		m_curseq(0)
{
	m_sbounds.set(0, -1, 0, -1);
	memset(m_scaled, 0, sizeof(m_scaled));
}


//-------------------------------------------------
//  ~render_texture - destructor
//-------------------------------------------------

render_texture::~render_texture()
{
	release();
}


//-------------------------------------------------
//  reset - reset the state of a texture after
//  it has been re-allocated
//-------------------------------------------------

void render_texture::reset(render_manager &manager, texture_scaler_func scaler, void *param)
{
	m_manager = &manager;
	if (scaler != nullptr)
	{
		assert(m_format == TEXFORMAT_ARGB32);
		m_scaler = scaler;
		m_param = param;
	}
	m_old_id = m_id;
	m_id = ~0L;
}


//-------------------------------------------------
//  release - release resources when we are freed
//-------------------------------------------------

void render_texture::release()
{
	// free all scaled versions
	for (auto &elem : m_scaled)
	{
		m_manager->invalidate_all(elem.bitmap.get());
		elem.bitmap.reset();
		elem.seqid = 0;
	}

	// invalidate references to the original bitmap as well
	m_manager->invalidate_all(m_bitmap);
	m_bitmap = nullptr;
	m_sbounds.set(0, -1, 0, -1);
	m_format = TEXFORMAT_ARGB32;
	m_curseq = 0;
}


//-------------------------------------------------
//  set_bitmap - set a new source bitmap
//-------------------------------------------------

void render_texture::set_bitmap(bitmap_t &bitmap, const rectangle &sbounds, texture_format format)
{
	assert(bitmap.cliprect().contains(sbounds));

	// ensure we have a valid palette for palettized modes
	if (format == TEXFORMAT_PALETTE16)
		assert(bitmap.palette() != nullptr);

	// invalidate references to the old bitmap
	if (&bitmap != m_bitmap && m_bitmap != nullptr)
		m_manager->invalidate_all(m_bitmap);

	// set the new bitmap/palette
	m_bitmap = &bitmap;
	m_sbounds = sbounds;
	m_format = format;

	// invalidate all scaled versions
	for (auto & elem : m_scaled)
	{
		if (elem.bitmap)
			m_manager->invalidate_all(elem.bitmap.get());
		elem.bitmap.reset();
		elem.seqid = 0;
	}
}


//-------------------------------------------------
//  hq_scale - generic high quality resampling
//  scaler
//-------------------------------------------------

void render_texture::hq_scale(bitmap_argb32 &dest, bitmap_argb32 &source, const rectangle &sbounds, void *param)
{
	render_color color = { 1.0f, 1.0f, 1.0f, 1.0f };
	bitmap_argb32 sourcesub(source, sbounds);
	render_resample_argb_bitmap_hq(dest, sourcesub, color);
}


//-------------------------------------------------
//  get_scaled - get a scaled bitmap (if we can)
//-------------------------------------------------

void render_texture::get_scaled(u32 dwidth, u32 dheight, render_texinfo &texinfo, render_primitive_list &primlist, u32 flags)
{
	// source width/height come from the source bounds
	int swidth = m_sbounds.width();
	int sheight = m_sbounds.height();

	// ensure height/width are non-zero
	if (dwidth == 0) dwidth = 1;
	if (dheight == 0) dheight = 1;

	texinfo.unique_id = m_id;
	texinfo.old_id = m_old_id;
	if (m_old_id != ~0ULL)
		m_old_id = ~0ULL;

	// are we scaler-free? if so, just return the source bitmap
	if (m_scaler == nullptr || (m_bitmap != nullptr && swidth == dwidth && sheight == dheight))
	{
		if (m_bitmap == nullptr) return;

		// add a reference and set up the source bitmap
		primlist.add_reference(m_bitmap);
		texinfo.base = m_bitmap->raw_pixptr(m_sbounds.top(), m_sbounds.left());
		texinfo.rowpixels = m_bitmap->rowpixels();
		texinfo.width = swidth;
		texinfo.height = sheight;
		// palette will be set later
		texinfo.seqid = ++m_curseq;
	}
	else
	{
		// make sure we can recover the original argb32 bitmap
		bitmap_argb32 dummy;
		bitmap_argb32 &srcbitmap = (m_bitmap != nullptr) ? downcast<bitmap_argb32 &>(*m_bitmap) : dummy;

		// is it a size we already have?
		scaled_texture *scaled = nullptr;
		int scalenum;
		for (scalenum = 0; scalenum < ARRAY_LENGTH(m_scaled); scalenum++)
		{
			scaled = &m_scaled[scalenum];

			// we need a non-NULL bitmap with matching dest size
			if (scaled->bitmap != nullptr && dwidth == scaled->bitmap->width() && dheight == scaled->bitmap->height())
				break;
		}

		// did we get one?
		if (scalenum == ARRAY_LENGTH(m_scaled))
		{
			int lowest = -1;

			// didn't find one -- take the entry with the lowest seqnum
			for (scalenum = 0; scalenum < ARRAY_LENGTH(m_scaled); scalenum++)
				if ((lowest == -1 || m_scaled[scalenum].seqid < m_scaled[lowest].seqid) && !primlist.has_reference(m_scaled[scalenum].bitmap.get()))
					lowest = scalenum;
			if (-1 == lowest)
				throw emu_fatalerror("render_texture::get_scaled: Too many live texture instances!");

			// throw out any existing entries
			scaled = &m_scaled[lowest];
			if (scaled->bitmap)
			{
				m_manager->invalidate_all(scaled->bitmap.get());
				scaled->bitmap.reset();
			}

			// allocate a new bitmap
			scaled->bitmap = std::make_unique<bitmap_argb32>(dwidth, dheight);
			scaled->seqid = ++m_curseq;

			// let the scaler do the work
			(*m_scaler)(*scaled->bitmap, srcbitmap, m_sbounds, m_param);
		}

		// finally fill out the new info
		primlist.add_reference(scaled->bitmap.get());
		texinfo.base = &scaled->bitmap->pix(0);
		texinfo.rowpixels = scaled->bitmap->rowpixels();
		texinfo.width = dwidth;
		texinfo.height = dheight;
		// palette will be set later
		texinfo.seqid = scaled->seqid;
	}
}


//-------------------------------------------------
//  get_adjusted_palette - return the adjusted
//  palette for a texture
//-------------------------------------------------

const rgb_t *render_texture::get_adjusted_palette(render_container &container, u32 &out_length)
{
	// override the palette with our adjusted palette
	switch (m_format)
	{
		case TEXFORMAT_PALETTE16:

			assert(m_bitmap->palette() != nullptr);

			// return our adjusted palette
			return container.bcg_lookup_table(m_format, out_length, m_bitmap->palette());

		case TEXFORMAT_RGB32:
		case TEXFORMAT_ARGB32:
		case TEXFORMAT_YUY16:

			// if no adjustment necessary, return nullptr
			if (!container.has_brightness_contrast_gamma_changes())
				return nullptr;
			return container.bcg_lookup_table(m_format, out_length);

		default:
			assert(false);
	}

	return nullptr;
}



//**************************************************************************
//  RENDER CONTAINER
//**************************************************************************

//-------------------------------------------------
//  render_container - constructor
//-------------------------------------------------

render_container::render_container(render_manager &manager, screen_device *screen)
	: m_next(nullptr)
	, m_manager(manager)
	, m_screen(screen)
	, m_overlaybitmap(nullptr)
	, m_overlaytexture(nullptr)
{
	// make sure it is empty
	empty();

	// if we have a screen, read and apply the options
	if (m_screen)
	{
		// set the initial orientation and brightness/contrast/gamma
		m_user.m_orientation = m_screen->orientation();
		m_user.m_brightness = manager.machine().options().brightness();
		m_user.m_contrast = manager.machine().options().contrast();
		m_user.m_gamma = manager.machine().options().gamma();
		// palette client will be allocated later
	}

	recompute_lookups();
}


//-------------------------------------------------
//  ~render_container - destructor
//-------------------------------------------------

render_container::~render_container()
{
	// free all the container items
	empty();

	// free the overlay texture
	m_manager.texture_free(m_overlaytexture);
}


//-------------------------------------------------
//  set_overlay - set the overlay bitmap for the
//  container
//-------------------------------------------------

void render_container::set_overlay(bitmap_argb32 *bitmap)
{
	// free any existing texture
	m_manager.texture_free(m_overlaytexture);

	// set the new data and allocate the texture
	m_overlaybitmap = bitmap;
	if (m_overlaybitmap != nullptr)
	{
		m_overlaytexture = m_manager.texture_alloc(render_container::overlay_scale);
		m_overlaytexture->set_bitmap(*bitmap, bitmap->cliprect(), TEXFORMAT_ARGB32);
	}
}


//-------------------------------------------------
//  set_user_settings - set the current user
//  settings for a container
//-------------------------------------------------

void render_container::set_user_settings(const user_settings &settings)
{
	m_user = settings;
	recompute_lookups();
}


//-------------------------------------------------
//  add_line - add a line item to this container
//-------------------------------------------------

void render_container::add_line(float x0, float y0, float x1, float y1, float width, rgb_t argb, u32 flags)
{
	item &newitem = add_generic(CONTAINER_ITEM_LINE, x0, y0, x1, y1, argb);
	newitem.m_width = width;
	newitem.m_flags = flags;
}


//-------------------------------------------------
//  add_quad - add a quad item to this container
//-------------------------------------------------

void render_container::add_quad(float x0, float y0, float x1, float y1, rgb_t argb, render_texture *texture, u32 flags)
{
	item &newitem = add_generic(CONTAINER_ITEM_QUAD, x0, y0, x1, y1, argb);
	newitem.m_texture = texture;
	newitem.m_flags = flags;
}


//-------------------------------------------------
//  add_char - add a char item to this container
//-------------------------------------------------

void render_container::add_char(float x0, float y0, float height, float aspect, rgb_t argb, render_font &font, u16 ch)
{
	// compute the bounds of the character cell and get the texture
	render_bounds bounds;
	bounds.x0 = x0;
	bounds.y0 = y0;
	render_texture *texture = font.get_char_texture_and_bounds(height, aspect, ch, bounds);

	// add it like a quad
	item &newitem = add_generic(CONTAINER_ITEM_QUAD, bounds.x0, bounds.y0, bounds.x1, bounds.y1, argb);
	newitem.m_texture = texture;
	newitem.m_flags = PRIMFLAG_TEXORIENT(ROT0) | PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA) | PRIMFLAG_PACKABLE;
	newitem.m_internal = INTERNAL_FLAG_CHAR;
}


//-------------------------------------------------
//  apply_brightness_contrast_gamma - apply the
//  container's brightess, contrast, and gamma to
//  an 8-bit value
//-------------------------------------------------

u8 render_container::apply_brightness_contrast_gamma(u8 value)
{
	return ::apply_brightness_contrast_gamma(value, m_user.m_brightness, m_user.m_contrast, m_user.m_gamma);
}


//-------------------------------------------------
//  apply_brightness_contrast_gamma_fp - apply the
//  container's brightess, contrast, and gamma to
//  a floating-point value
//-------------------------------------------------

float render_container::apply_brightness_contrast_gamma_fp(float value)
{
	return ::apply_brightness_contrast_gamma_fp(value, m_user.m_brightness, m_user.m_contrast, m_user.m_gamma);
}


//-------------------------------------------------
//  bcg_lookup_table - return the appropriate
//  brightness/contrast/gamma lookup table for a
//  given texture mode
//-------------------------------------------------

const rgb_t *render_container::bcg_lookup_table(int texformat, u32 &out_length, palette_t *palette)
{
	switch (texformat)
	{
		case TEXFORMAT_PALETTE16:
			if (m_palclient == nullptr) // if adjusted palette hasn't been created yet, create it
			{
				m_palclient = std::make_unique<palette_client>(*palette);
				m_bcglookup.resize(palette->max_index());
				recompute_lookups();
			}
			assert (palette == &m_palclient->palette());
			out_length = palette->max_index();
			return &m_bcglookup[0];

		case TEXFORMAT_RGB32:
		case TEXFORMAT_ARGB32:
		case TEXFORMAT_YUY16:
			out_length = ARRAY_LENGTH(m_bcglookup256);
			return m_bcglookup256;

		default:
			out_length = 0;
			return nullptr;
	}
}


//-------------------------------------------------
//  overlay_scale - scaler for an overlay
//-------------------------------------------------

void render_container::overlay_scale(bitmap_argb32 &dest, bitmap_argb32 &source, const rectangle &sbounds, void *param)
{
	// simply replicate the source bitmap over the target
	for (int y = 0; y < dest.height(); y++)
	{
		u32 const *const src = &source.pix(y % source.height());
		u32 *dst = &dest.pix(y);
		int sx = 0;

		// loop over columns
		for (int x = 0; x < dest.width(); x++)
		{
			*dst++ = src[sx++];
			if (sx >= source.width())
				sx = 0;
		}
	}
}


//-------------------------------------------------
//  add_generic - add a generic item to a
//  container
//-------------------------------------------------

render_container::item &render_container::add_generic(u8 type, float x0, float y0, float x1, float y1, rgb_t argb)
{
	item *newitem = m_item_allocator.alloc();

	assert(x0 == x0);
	assert(x1 == x1);
	assert(y0 == y0);
	assert(y1 == y1);

	// copy the data into the new item
	newitem->m_type = type;
	newitem->m_bounds.x0 = x0;
	newitem->m_bounds.y0 = y0;
	newitem->m_bounds.x1 = x1;
	newitem->m_bounds.y1 = y1;
	newitem->m_color.r = (float)argb.r() * (1.0f / 255.0f);
	newitem->m_color.g = (float)argb.g() * (1.0f / 255.0f);
	newitem->m_color.b = (float)argb.b() * (1.0f / 255.0f);
	newitem->m_color.a = (float)argb.a() * (1.0f / 255.0f);
	newitem->m_flags = 0;
	newitem->m_internal = 0;
	newitem->m_width = 0;
	newitem->m_texture = nullptr;

	// add the item to the container
	return m_itemlist.append(*newitem);
}


//-------------------------------------------------
//  recompute_lookups - recompute the lookup table
//  for the render container
//-------------------------------------------------

void render_container::recompute_lookups()
{
	// recompute the 256 entry lookup table
	for (int i = 0; i < 0x100; i++)
	{
		u8 adjustedval = apply_brightness_contrast_gamma(i);
		m_bcglookup256[i + 0x000] = adjustedval << 0;
		m_bcglookup256[i + 0x100] = adjustedval << 8;
		m_bcglookup256[i + 0x200] = adjustedval << 16;
		m_bcglookup256[i + 0x300] = adjustedval << 24;
	}

	// recompute the palette entries
	if (m_palclient != nullptr)
	{
		palette_t &palette = m_palclient->palette();
		const rgb_t *adjusted_palette = palette.entry_list_adjusted();
		int colors = palette.max_index();

		if (has_brightness_contrast_gamma_changes())
		{
			for (int i = 0; i < colors; i++)
			{
				rgb_t newval = adjusted_palette[i];
				m_bcglookup[i] = (newval & 0xff000000) |
										m_bcglookup256[0x200 + newval.r()] |
										m_bcglookup256[0x100 + newval.g()] |
										m_bcglookup256[0x000 + newval.b()];
			}
		}
		else
			memcpy(&m_bcglookup[0], adjusted_palette, colors * sizeof(rgb_t));
	}
}


//-------------------------------------------------
//  update_palette - update any dirty palette
//  entries
//-------------------------------------------------

void render_container::update_palette()
{
	// skip if no client
	if (m_palclient == nullptr)
		return;

	// get the dirty list
	u32 mindirty, maxdirty;
	const u32 *dirty = m_palclient->dirty_list(mindirty, maxdirty);

	// iterate over dirty items and update them
	if (dirty != nullptr)
	{
		palette_t &palette = m_palclient->palette();
		const rgb_t *adjusted_palette = palette.entry_list_adjusted();

		if (has_brightness_contrast_gamma_changes())
		{
			// loop over chunks of 32 entries, since we can quickly examine 32 at a time
			for (u32 entry32 = mindirty / 32; entry32 <= maxdirty / 32; entry32++)
			{
				u32 dirtybits = dirty[entry32];
				if (dirtybits != 0)

					// this chunk of 32 has dirty entries; fix them up
					for (u32 entry = 0; entry < 32; entry++)
						if (dirtybits & (1 << entry))
						{
							u32 finalentry = entry32 * 32 + entry;
							rgb_t newval = adjusted_palette[finalentry];
							m_bcglookup[finalentry] = (newval & 0xff000000) |
														m_bcglookup256[0x200 + newval.r()] |
														m_bcglookup256[0x100 + newval.g()] |
														m_bcglookup256[0x000 + newval.b()];
						}
			}
		}
		else
			memcpy(&m_bcglookup[mindirty], &adjusted_palette[mindirty], (maxdirty - mindirty + 1) * sizeof(rgb_t));
	}
}


//-------------------------------------------------
//  user_settings - constructor
//-------------------------------------------------

render_container::user_settings::user_settings()
	: m_orientation(0)
	, m_brightness(1.0f)
	, m_contrast(1.0f)
	, m_gamma(1.0f)
	, m_xscale(1.0f)
	, m_yscale(1.0f)
	, m_xoffset(0.0f)
	, m_yoffset(0.0f)
{
}



//**************************************************************************
//  RENDER TARGET
//**************************************************************************

//-------------------------------------------------
//  render_target - constructor
//-------------------------------------------------

render_target::render_target(render_manager &manager, const internal_layout *layoutfile, u32 flags)
	: render_target(manager, layoutfile, flags, CONSTRUCTOR_IMPL)
{
}

render_target::render_target(render_manager &manager, util::xml::data_node const &layout, u32 flags)
	: render_target(manager, layout, flags, CONSTRUCTOR_IMPL)
{
}

template <typename T> render_target::render_target(render_manager &manager, T &&layout, u32 flags, constructor_impl_t)
	: m_next(nullptr)
	, m_manager(manager)
	, m_filelist(std::make_unique<std::list<layout_file>>())
	, m_curview(0U)
	, m_flags(flags)
	, m_listindex(0)
	, m_width(640)
	, m_height(480)
	, m_pixel_aspect(0.0f)
	, m_max_refresh(0)
	, m_orientation(0)
	, m_base_view(nullptr)
	, m_base_orientation(ROT0)
	, m_maxtexwidth(65536)
	, m_maxtexheight(65536)
	, m_transform_container(true)
	, m_external_artwork(false)
{
	// determine the base layer configuration based on options
	m_base_layerconfig.set_zoom_to_screen(manager.machine().options().artwork_crop());

	// aspect and scale options
	m_keepaspect = (manager.machine().options().keep_aspect() && !(flags & RENDER_CREATE_HIDDEN));
	m_int_overscan = manager.machine().options().int_overscan();
	m_int_scale_x = manager.machine().options().int_scale_x();
	m_int_scale_y = manager.machine().options().int_scale_y();
	if (m_manager.machine().options().auto_stretch_xy())
		m_scale_mode = SCALE_FRACTIONAL_AUTO;
	else if (manager.machine().options().uneven_stretch_x())
		m_scale_mode = SCALE_FRACTIONAL_X;
	else if (manager.machine().options().uneven_stretch_y())
		m_scale_mode = SCALE_FRACTIONAL_Y;
	else if (manager.machine().options().uneven_stretch())
		m_scale_mode = SCALE_FRACTIONAL;
	else
		m_scale_mode = SCALE_INTEGER;

	// determine the base orientation based on options
	if (!manager.machine().options().rotate())
		m_base_orientation = orientation_reverse(manager.machine().system().flags & machine_flags::MASK_ORIENTATION);

	// rotate left/right
	if (manager.machine().options().ror() || (manager.machine().options().auto_ror() && (manager.machine().system().flags & ORIENTATION_SWAP_XY)))
		m_base_orientation = orientation_add(ROT90, m_base_orientation);
	if (manager.machine().options().rol() || (manager.machine().options().auto_rol() && (manager.machine().system().flags & ORIENTATION_SWAP_XY)))
		m_base_orientation = orientation_add(ROT270, m_base_orientation);

	// flip X/Y
	if (manager.machine().options().flipx())
		m_base_orientation ^= ORIENTATION_FLIP_X;
	if (manager.machine().options().flipy())
		m_base_orientation ^= ORIENTATION_FLIP_Y;

	// set the orientation and layerconfig equal to the base
	m_orientation = m_base_orientation;
	m_layerconfig = m_base_layerconfig;

	// load the layout files
	load_layout_files(std::forward<T>(layout), flags & RENDER_CREATE_SINGLE_FILE);
	for (layout_file &file : *m_filelist)
		for (layout_view &view : file.views())
			if (!(m_flags & RENDER_CREATE_NO_ART) || !view.has_art())
				m_views.emplace_back(view, view.default_visibility_mask());

	// set the current view to the first one
	set_view(0);

	// make us the UI target if there is none
	if (!hidden() && manager.m_ui_target == nullptr)
		manager.set_ui_target(*this);
}


//-------------------------------------------------
//  ~render_target - destructor
//-------------------------------------------------

render_target::~render_target()
{
}


//-------------------------------------------------
//  is_ui_target - return true if this is the
//  UI target
//-------------------------------------------------

bool render_target::is_ui_target() const
{
	return (this == &m_manager.ui_target());
}


//-------------------------------------------------
//  index - return the index of this target
//-------------------------------------------------

int render_target::index() const
{
	return m_manager.m_targetlist.indexof(*this);
}


//-------------------------------------------------
//  set_bounds - set the bounds and pixel aspect
//  of a target
//-------------------------------------------------

void render_target::set_bounds(s32 width, s32 height, float pixel_aspect)
{
	m_width = width;
	m_height = height;
	m_bounds.x0 = m_bounds.y0 = 0;
	m_bounds.x1 = (float)width;
	m_bounds.y1 = (float)height;
	m_pixel_aspect = pixel_aspect != 0.0? pixel_aspect : 1.0;
}


//-------------------------------------------------
//  set_view - dynamically change the view for
//  a target
//-------------------------------------------------

void render_target::set_view(unsigned viewindex)
{
	if (m_views.size() > viewindex)
	{
		m_curview = viewindex;
		current_view().recompute(visibility_mask(), m_layerconfig.zoom_to_screen());
		current_view().preload();
	}
}


//-------------------------------------------------
//  set_max_texture_size - set the upper bound on
//  the texture size
//-------------------------------------------------

void render_target::set_max_texture_size(int maxwidth, int maxheight)
{
	m_maxtexwidth = maxwidth;
	m_maxtexheight = maxheight;
}


//-------------------------------------------------
//  set_visibility_toggle - show or hide selected
//  parts of a view
//-------------------------------------------------

void render_target::set_visibility_toggle(unsigned index, bool enable)
{
	assert(current_view().visibility_toggles().size() > index);
	if (enable)
		m_views[m_curview].second |= u32(1) << index;
	else
		m_views[m_curview].second &= ~(u32(1) << index);
	current_view().recompute(visibility_mask(), m_layerconfig.zoom_to_screen());
	current_view().preload();
}


//-------------------------------------------------
//  configured_view - select a view for this
//  target based on the configuration parameters
//-------------------------------------------------

unsigned render_target::configured_view(const char *viewname, int targetindex, int numtargets)
{
	layout_view *view = nullptr;

	// if it isn't "auto" or an empty string, try to match it as a view name prefix
	if (viewname && *viewname && strcmp(viewname, "auto"))
	{
		// scan for a matching view name
		size_t const viewlen = strlen(viewname);
		for (unsigned i = 0; !view && (m_views.size() > i); ++i)
			if (!core_strnicmp(m_views[i].first.name().c_str(), viewname, viewlen))
				view = &m_views[i].first;
	}

	// if we don't have a match, default to the nth view
	std::vector<std::reference_wrapper<screen_device> > screens;
	for (screen_device &screen : screen_device_enumerator(m_manager.machine().root_device()))
		screens.push_back(screen);
	if (!view && !screens.empty())
	{
		// if we have enough targets to be one per screen, assign in order
		if (numtargets >= screens.size())
		{
			// find the first view with this screen and this screen only
			screen_device const &screen = screens[index() % screens.size()];
			for (unsigned i = 0; !view && (m_views.size() > i); ++i)
			{
				for (layout_view::item &viewitem : m_views[i].first.items())
				{
					screen_device const *const viewscreen(viewitem.screen());
					if (viewscreen == &screen)
					{
						view = &m_views[i].first;
					}
					else if (viewscreen)
					{
						view = nullptr;
						break;
					}
				}
			}
		}

		// otherwise, find the first view that has all the screens
		if (!view)
		{
			for (unsigned i = 0; !view && (m_views.size() > i); ++i)
			{
				layout_view &curview = m_views[i].first;
				if (std::find_if(screens.begin(), screens.end(), [&curview] (screen_device &screen) { return !curview.has_screen(screen); }) == screens.end())
					view = &curview;
			}
		}
	}

	// make sure it's a valid view
	return view ? view_index(*view) : 0;
}


//-------------------------------------------------
//  view_name - return the name of the given view
//-------------------------------------------------

const char *render_target::view_name(unsigned viewindex)
{
	return (m_views.size() > viewindex) ? m_views[viewindex].first.name().c_str() : nullptr;
}


//-------------------------------------------------
//  compute_visible_area - compute the visible
//  area for the given target with the current
//  layout and proposed new parameters
//-------------------------------------------------

void render_target::compute_visible_area(s32 target_width, s32 target_height, float target_pixel_aspect, int target_orientation, s32 &visible_width, s32 &visible_height)
{
	switch (m_scale_mode)
	{
	case SCALE_FRACTIONAL:
		{
			float width, height;
			float scale;

			// constrained case
			if (m_keepaspect)
			{
				// start with the aspect ratio of the square pixel layout
				width = current_view().effective_aspect();
				height = 1.0f;

				// first apply target orientation
				if (target_orientation & ORIENTATION_SWAP_XY)
					std::swap(width, height);

				// apply the target pixel aspect ratio
				height *= target_pixel_aspect;

				// based on the height/width ratio of the source and target, compute the scale factor
				if (width / height > (float)target_width / (float)target_height)
					scale = (float)target_width / width;
				else
					scale = (float)target_height / height;
			}

			// stretch-to-fit case
			else
			{
				width = (float)target_width;
				height = (float)target_height;
				scale = 1.0f;
			}

			// set the final width/height
			visible_width = render_round_nearest(width * scale);
			visible_height = render_round_nearest(height * scale);
			break;
		}

	default:
		{
			// get source size and aspect
			s32 src_width, src_height;
			compute_minimum_size(src_width, src_height);
			float src_aspect = current_view().effective_aspect();

			// apply orientation if required
			if (target_orientation & ORIENTATION_SWAP_XY)
				src_aspect = 1.0 / src_aspect;

			// get target aspect
			float target_aspect = (float)target_width / (float)target_height * target_pixel_aspect;
			bool target_is_portrait = (target_aspect < 1.0f);

			// apply automatic axial stretching if required
			int scale_mode = m_scale_mode;
			if (m_scale_mode == SCALE_FRACTIONAL_AUTO)
			{
				bool is_rotated = (m_manager.machine().system().flags & ORIENTATION_SWAP_XY) ^ (target_orientation & ORIENTATION_SWAP_XY);
				scale_mode = is_rotated ^ target_is_portrait ? SCALE_FRACTIONAL_Y : SCALE_FRACTIONAL_X;
			}

			// determine the scale mode for each axis
			bool x_is_integer = !((!target_is_portrait && scale_mode == SCALE_FRACTIONAL_X) || (target_is_portrait && scale_mode == SCALE_FRACTIONAL_Y));
			bool y_is_integer = !((target_is_portrait && scale_mode == SCALE_FRACTIONAL_X) || (!target_is_portrait && scale_mode == SCALE_FRACTIONAL_Y));

			// first compute scale factors to fit the screen
			float xscale = (float)target_width / src_width;
			float yscale = (float)target_height / src_height;
			float maxxscale = std::max(1.0f, float(m_int_overscan ? render_round_nearest(xscale) : floor(xscale)));
			float maxyscale = std::max(1.0f, float(m_int_overscan ? render_round_nearest(yscale) : floor(yscale)));

			// now apply desired scale mode and aspect correction
			if (m_keepaspect && target_aspect > src_aspect) xscale *= src_aspect / target_aspect * (maxyscale / yscale);
			if (m_keepaspect && target_aspect < src_aspect) yscale *= target_aspect / src_aspect * (maxxscale / xscale);
			if (x_is_integer) xscale = std::min(maxxscale, std::max(1.0f, render_round_nearest(xscale)));
			if (y_is_integer) yscale = std::min(maxyscale, std::max(1.0f, render_round_nearest(yscale)));

			// check if we have user defined scale factors, if so use them instead
			int user_scale_x = target_is_portrait? m_int_scale_y : m_int_scale_x;
			int user_scale_y = target_is_portrait? m_int_scale_x : m_int_scale_y;
			xscale = user_scale_x > 0 ? user_scale_x : xscale;
			yscale = user_scale_y > 0 ? user_scale_y : yscale;

			// set the final width/height
			visible_width = render_round_nearest(src_width * xscale);
			visible_height = render_round_nearest(src_height * yscale);
			break;
		}
	}
}


//-------------------------------------------------
//  compute_minimum_size - compute the "minimum"
//  size of a target, which is the smallest bounds
//  that will ensure at least 1 target pixel per
//  source pixel for all included screens
//-------------------------------------------------

void render_target::compute_minimum_size(s32 &minwidth, s32 &minheight)
{
	float maxxscale = 1.0f, maxyscale = 1.0f;
	int screens_considered = 0;

	// early exit in case we are called between device teardown and render teardown
	if (m_manager.machine().phase() == machine_phase::EXIT)
	{
		minwidth = 640;
		minheight = 480;
		return;
	}

	if (m_views.empty())
		throw emu_fatalerror("Mandatory artwork is missing");

	// scan the current view for all screens
	for (layout_view::item &curitem : current_view().items())
	{
		screen_device const *const screen = curitem.screen();
		if (screen)
		{
			// use a hard-coded default visible area for vector screens
			const rectangle vectorvis(0, 639, 0, 479);
			const rectangle &visarea = (screen->screen_type() == SCREEN_TYPE_VECTOR) ? vectorvis : screen->visible_area();

			// apply target orientation to the bounds
			render_bounds bounds = curitem.bounds();
			apply_orientation(bounds, m_orientation);
			normalize_bounds(bounds);

			// based on the orientation of the screen container, check the bitmap
			float xscale, yscale;
			if (!(orientation_add(m_orientation, screen->container().orientation()) & ORIENTATION_SWAP_XY))
			{
				xscale = float(visarea.width()) / bounds.width();
				yscale = float(visarea.height()) / bounds.height();
			}
			else
			{
				xscale = float(visarea.height()) / bounds.width();
				yscale = float(visarea.width()) / bounds.height();
			}

			// pick the greater
			maxxscale = std::max(xscale, maxxscale);
			maxyscale = std::max(yscale, maxyscale);
			screens_considered++;
		}
	}

	// if there were no screens considered, pick a nominal default
	if (screens_considered == 0)
	{
		maxxscale = 640.0f;
		maxyscale = 480.0f;
	}

	// round up
	minwidth = render_round_nearest(maxxscale);
	minheight = render_round_nearest(maxyscale);
}


//-------------------------------------------------
//  get_primitives - return a list of primitives
//  for a given render target
//-------------------------------------------------

render_primitive_list &render_target::get_primitives()
{
	// remember the base values if this is the first frame
	if (!m_base_view)
		m_base_view = &current_view();

	// switch to the next primitive list
	render_primitive_list &list = m_primlist[m_listindex];
	m_listindex = (m_listindex + 1) % ARRAY_LENGTH(m_primlist);
	list.acquire_lock();

	// free any previous primitives
	list.release_all();

	// compute the visible width/height
	s32 viswidth, visheight;
	compute_visible_area(m_width, m_height, m_pixel_aspect, m_orientation, viswidth, visheight);

	// create a root transform for the target
	object_transform root_xform;
	root_xform.xoffs = (float)(m_width - viswidth) / 2;
	root_xform.yoffs = (float)(m_height - visheight) / 2;
	root_xform.xscale = (float)viswidth;
	root_xform.yscale = (float)visheight;
	root_xform.color.r = root_xform.color.g = root_xform.color.b = root_xform.color.a = 1.0f;
	root_xform.orientation = m_orientation;
	root_xform.no_center = false;

	if (m_manager.machine().phase() >= machine_phase::RESET)
	{
		// we're running - iterate over items in the view
		current_view().prepare_items();
		for (layout_view::item &curitem : current_view().visible_items())
		{
			// first apply orientation to the bounds
			render_bounds bounds = curitem.bounds();
			apply_orientation(bounds, root_xform.orientation);
			normalize_bounds(bounds);

			// apply the transform to the item
			object_transform item_xform;
			item_xform.xoffs = root_xform.xoffs + bounds.x0 * root_xform.xscale;
			item_xform.yoffs = root_xform.yoffs + bounds.y0 * root_xform.yscale;
			item_xform.xscale = (bounds.x1 - bounds.x0) * root_xform.xscale;
			item_xform.yscale = (bounds.y1 - bounds.y0) * root_xform.yscale;
			item_xform.color = curitem.color() * root_xform.color;
			item_xform.orientation = orientation_add(curitem.orientation(), root_xform.orientation);
			item_xform.no_center = false;

			// if there is no associated element, it must be a screen element
			if (curitem.screen())
				add_container_primitives(list, root_xform, item_xform, curitem.screen()->container(), curitem.blend_mode());
			else
				add_element_primitives(list, item_xform, *curitem.element(), curitem.element_state(), curitem.blend_mode());
		}
	}
	else
	{
		// if we are not in the running stage, draw an outer box
		render_primitive *prim = list.alloc(render_primitive::QUAD);
		prim->bounds.set_xy(0.0f, 0.0f, (float)m_width, (float)m_height);
		prim->full_bounds = prim->bounds;
		prim->color.set(1.0f, 0.1f, 0.1f, 0.1f);
		prim->texture.base = nullptr;
		prim->flags = PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA);
		list.append(*prim);

		if (m_width > 1 && m_height > 1)
		{
			prim = list.alloc(render_primitive::QUAD);
			prim->bounds.set_xy(1.0f, 1.0f, float(m_width - 1), float(m_height - 1));
			prim->full_bounds = prim->bounds;
			prim->color.set(1.0f, 0.0f, 0.0f, 0.0f);
			prim->texture.base = nullptr;
			prim->flags = PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA);
			list.append(*prim);
		}
	}

	// process the debug containers
	for (render_container &debug : m_debug_containers)
	{
		object_transform ui_xform;
		ui_xform.xoffs = 0;
		ui_xform.yoffs = 0;
		ui_xform.xscale = (float)m_width;
		ui_xform.yscale = (float)m_height;
		ui_xform.color.r = ui_xform.color.g = ui_xform.color.b = 1.0f;
		ui_xform.color.a = 0.9f;
		ui_xform.orientation = m_orientation;
		ui_xform.no_center = true;

		// add UI elements
		add_container_primitives(list, root_xform, ui_xform, debug, BLENDMODE_ALPHA);
	}

	// process the UI if we are the UI target
	if (is_ui_target())
	{
		// compute the transform for the UI
		object_transform ui_xform;
		ui_xform.xoffs = 0;
		ui_xform.yoffs = 0;
		ui_xform.xscale = (float) m_width;
		ui_xform.yscale = (float) m_height;
		ui_xform.color.r = ui_xform.color.g = ui_xform.color.b = ui_xform.color.a = 1.0f;
		ui_xform.orientation = m_orientation;
		ui_xform.no_center = false;

		// add UI elements
		add_container_primitives(list, root_xform, ui_xform, m_manager.ui_container(), BLENDMODE_ALPHA);
	}

	// optimize the list before handing it off
	add_clear_and_optimize_primitive_list(list);
	list.release_lock();
	return list;
}


//-------------------------------------------------
//  map_point_container - attempts to map a point
//  on the specified render_target to the
//  specified container, if possible
//-------------------------------------------------

bool render_target::map_point_container(s32 target_x, s32 target_y, render_container &container, float &container_x, float &container_y)
{
	std::pair<float, float> target_f(map_point_internal(target_x, target_y));

	// explicitly check for the UI container
	if (&container == &m_manager.ui_container())
	{
		// this hit test went against the UI container
		if ((target_f.first >= 0.0f) && (target_f.first < 1.0f) && (target_f.second >= 0.0f) && (target_f.second < 1.0f))
		{
			// this point was successfully mapped
			container_x = float(target_x) / m_width;
			container_y = float(target_y) / m_height;
			return true;
		}
	}
	else
	{
		if (m_orientation & ORIENTATION_FLIP_X)
			target_f.first = 1.0f - target_f.first;
		if (m_orientation & ORIENTATION_FLIP_Y)
			target_f.second = 1.0f - target_f.second;
		if (m_orientation & ORIENTATION_SWAP_XY)
			std::swap(target_f.first, target_f.second);

		// try to find the right container
		auto const &items(current_view().visible_screen_items());
		auto const found(std::find_if(
					items.begin(),
					items.end(),
					[&container] (layout_view::item &item) { return &item.screen()->container() == &container; }));
		if (items.end() != found)
		{
			layout_view::item &item(*found);
			render_bounds const bounds(item.bounds());
			if (bounds.includes(target_f.first, target_f.second))
			{
				// point successfully mapped
				container_x = (target_f.first - bounds.x0) / bounds.width();
				container_y = (target_f.second - bounds.y0) / bounds.height();
				return true;
			}
		}
	}

	// default to point not mapped
	container_x = container_y = -1.0f;
	return false;
}


//-------------------------------------------------
//  map_point_input - attempts to map a point on
//  the specified render_target to an input port
//  field, if possible
//-------------------------------------------------

bool render_target::map_point_input(s32 target_x, s32 target_y, ioport_port *&input_port, ioport_value &input_mask, float &input_x, float &input_y)
{
	std::pair<float, float> target_f(map_point_internal(target_x, target_y));
	if (m_orientation & ORIENTATION_FLIP_X)
		target_f.first = 1.0f - target_f.first;
	if (m_orientation & ORIENTATION_FLIP_Y)
		target_f.second = 1.0f - target_f.second;
	if (m_orientation & ORIENTATION_SWAP_XY)
		std::swap(target_f.first, target_f.second);

	auto const &items(current_view().interactive_items());
	m_hit_test.resize(items.size() * 2);
	std::fill(m_hit_test.begin(), m_hit_test.end(), false);

	for (auto const &edge : current_view().interactive_edges_x())
	{
		if ((edge.position() > target_f.first) || ((edge.position() == target_f.first) && edge.trailing()))
			break;
		else
			m_hit_test[edge.index()] = !edge.trailing();
	}

	for (auto const &edge : current_view().interactive_edges_y())
	{
		if ((edge.position() > target_f.second) || ((edge.position() == target_f.second) && edge.trailing()))
			break;
		else
			m_hit_test[items.size() + edge.index()] = !edge.trailing();
	}

	for (unsigned i = 0; items.size() > i; ++i)
	{
		if (m_hit_test[i] && m_hit_test[items.size() + i])
		{
			layout_view::item &item(items[i]);
			render_bounds const bounds(item.bounds());
			if (bounds.includes(target_f.first, target_f.second))
			{
				if (item.has_input())
				{
					// point successfully mapped
					std::tie(input_port, input_mask) = item.input_tag_and_mask();
					input_x = (target_f.first - bounds.x0) / bounds.width();
					input_y = (target_f.second - bounds.y0) / bounds.height();
					return true;
				}
				else
				{
					break;
				}
			}
		}
	}

	// default to point not mapped
	input_port = nullptr;
	input_mask = 0;
	input_x = input_y = -1.0f;
	return false;
}


//-------------------------------------------------
//  invalidate_all - if any of our primitive lists
//  contain a reference to the given pointer,
//  clear them
//-------------------------------------------------

void render_target::invalidate_all(void *refptr)
{
	// iterate through all our primitive lists
	for (auto & list : m_primlist)
	{
		// if we have a reference to this object, release our list
		list.acquire_lock();
		if (list.has_reference(refptr))
			list.release_all();
		list.release_lock();
	}
}


//-------------------------------------------------
//  debug_alloc - allocate a container for a debug
//  view
//-------------------------------------------------

render_container *render_target::debug_alloc()
{
	return &m_debug_containers.append(*m_manager.container_alloc());
}


//-------------------------------------------------
//  debug_free - free a container for a debug view
//-------------------------------------------------

void render_target::debug_free(render_container &container)
{
	m_debug_containers.remove(container);
}


//-------------------------------------------------
//  debug_append - move a debug view container to
//  the end of the list
//-------------------------------------------------

void render_target::debug_append(render_container &container)
{
	m_debug_containers.append(m_debug_containers.detach(container));
}


//-------------------------------------------------
//  resolve_tags - resolve tag lookups
//-------------------------------------------------

void render_target::resolve_tags()
{
	for (layout_file &file : *m_filelist)
		file.resolve_tags();

	current_view().recompute(visibility_mask(), m_layerconfig.zoom_to_screen());
	current_view().preload();
}


//-------------------------------------------------
//  update_layer_config - recompute after a layer
//  config change
//-------------------------------------------------

void render_target::update_layer_config()
{
	current_view().recompute(visibility_mask(), m_layerconfig.zoom_to_screen());
}


//-------------------------------------------------
//  load_layout_files - load layout files for a
//  given render target
//-------------------------------------------------

void render_target::load_layout_files(const internal_layout *layoutfile, bool singlefile)
{
	bool have_artwork = false;

	// if there's an explicit file, load that first
	const std::string &basename = m_manager.machine().basename();
	if (layoutfile)
		have_artwork |= load_layout_file(basename.c_str(), *layoutfile);

	// if we're only loading this file, we know our final result
	if (!singlefile)
		load_additional_layout_files(basename.c_str(), have_artwork);
}

void render_target::load_layout_files(util::xml::data_node const &rootnode, bool singlefile)
{
	bool have_artwork = false;

	// if there's an explicit file, load that first
	const std::string &basename = m_manager.machine().basename();
	have_artwork |= load_layout_file(m_manager.machine().root_device(), rootnode, m_manager.machine().options().art_path(), basename.c_str());

	// if we're only loading this file, we know our final result
	if (!singlefile)
		load_additional_layout_files(basename.c_str(), have_artwork);
}

void render_target::load_additional_layout_files(const char *basename, bool have_artwork)
{
	m_external_artwork = false;

	// if override_artwork defined, load that and skip artwork other than default
	const char *const override_art = m_manager.machine().options().override_artwork();
	if (override_art && *override_art)
	{
		if (load_layout_file(override_art, override_art))
			m_external_artwork = true;
		else if (load_layout_file(override_art, "default"))
			m_external_artwork = true;
	}

	const game_driver &system = m_manager.machine().system();

	// Skip if override_artwork has found artwork
	if (!m_external_artwork)
	{
		// try to load a file based on the driver name
		if (!load_layout_file(basename, system.name))
			m_external_artwork |= load_layout_file(basename, "default");
		else
			m_external_artwork = true;

		// if a default view has been specified, use that as a fallback
		bool have_default = false;
		if (system.default_layout)
			have_default |= load_layout_file(nullptr, *system.default_layout);
		m_manager.machine().config().apply_default_layouts(
				[this, &have_default] (device_t &dev, internal_layout const &layout)
				{ have_default |= load_layout_file(nullptr, layout, &dev); });

		// try to load another file based on the parent driver name
		int cloneof = driver_list::clone(system);
		while (0 <= cloneof)
		{
			if (!m_external_artwork || driver_list::driver(cloneof).flags & MACHINE_IS_BIOS_ROOT)
			{
				if (!load_layout_file(driver_list::driver(cloneof).name, driver_list::driver(cloneof).name))
					m_external_artwork |= load_layout_file(driver_list::driver(cloneof).name, "default");
				else
					m_external_artwork = true;
			}

			// Check the parent of the parent to cover bios based artwork
			const game_driver &parent(driver_list::driver(cloneof));
			cloneof = driver_list::clone(parent);
		}

		have_artwork |= m_external_artwork;

		// Use fallback artwork if defined and no artwork has been found yet
		if (!have_artwork)
		{
			const char *const fallback_art = m_manager.machine().options().fallback_artwork();
			if (fallback_art && *fallback_art)
			{
				if (!load_layout_file(fallback_art, fallback_art))
					have_artwork |= load_layout_file(fallback_art, "default");
				else
					have_artwork = true;
			}
		}
	}

	// local screen info to avoid repeated code
	class screen_info
	{
	public:
		screen_info(screen_device const &screen)
			: m_device(screen)
			, m_rotated(screen.orientation() & ORIENTATION_SWAP_XY)
			, m_physical(screen.physical_aspect())
			, m_native(screen.visible_area().width(), screen.visible_area().height())
		{
			util::reduce_fraction(m_native.first, m_native.second);
			if (m_rotated)
			{
				std::swap(m_physical.first, m_physical.second);
				std::swap(m_native.first, m_native.second);
			}
		}

		screen_device const &device() const { return m_device.get(); }
		bool rotated() const { return m_rotated; }
		bool square() const { return m_physical == m_native; }
		unsigned physical_x() const { return m_physical.first; }
		unsigned physical_y() const { return m_physical.second; }
		unsigned native_x() const { return m_native.first; }
		unsigned native_y() const { return m_native.second; }

		std::pair<float, float> tiled_size() const
		{
			if (physical_x() == physical_y())
				return std::make_pair(1.0F, 1.0F);
			else if (physical_x() > physical_y())
				return std::make_pair(1.0F, float(physical_y()) / physical_x());
			else
				return std::make_pair(float(physical_x()) / physical_y(), 1.0F);
		}

	private:
		std::reference_wrapper<screen_device const> m_device;
		bool m_rotated;
		std::pair<unsigned, unsigned> m_physical, m_native;
	};
	screen_device_enumerator iter(m_manager.machine().root_device());
	std::vector<screen_info> const screens(std::begin(iter), std::end(iter));

	// need this because views aren't fully set up yet
	auto const nth_view =
		[this] (unsigned n) -> layout_view *
		{
			for (layout_file &file : *m_filelist)
				for (layout_view &view : file.views())
					if (!(m_flags & RENDER_CREATE_NO_ART) || !view.has_art())
						if (n-- == 0)
							return &view;
			return nullptr;
		};

	if (screens.empty()) // ensure the fallback view for systems with no screens is loaded if necessary
	{
		if (!nth_view(0))
		{
			load_layout_file(nullptr, layout_noscreens);
			if (m_filelist->empty())
				throw emu_fatalerror("Couldn't parse default layout??");
		}
	}
	else // generate default layouts for larger numbers of screens
	{
		util::xml::file::ptr const root(util::xml::file::create());
		if (!root)
			throw emu_fatalerror("Couldn't create XML document??");
		util::xml::data_node *const layoutnode(root->add_child("mamelayout", nullptr));
		if (!layoutnode)
			throw emu_fatalerror("Couldn't create XML node??");
		layoutnode->set_attribute_int("version", 2);

		// generate individual physical aspect views
		for (unsigned i = 0; screens.size() > i; ++i)
		{
			util::xml::data_node *const viewnode(layoutnode->add_child("view", nullptr));
			if (!viewnode)
				throw emu_fatalerror("Couldn't create XML node??");
			viewnode->set_attribute(
					"name",
					util::string_format(
						"Screen %1$u Standard (%2$u:%3$u)",
						i, screens[i].physical_x(), screens[i].physical_y()).c_str());
			util::xml::data_node *const screennode(viewnode->add_child("screen", nullptr));
			if (!screennode)
				throw emu_fatalerror("Couldn't create XML node??");
			screennode->set_attribute_int("index", i);
			util::xml::data_node *const boundsnode(screennode->add_child("bounds", nullptr));
			if (!boundsnode)
				throw emu_fatalerror("Couldn't create XML node??");
			boundsnode->set_attribute_int("x", 0);
			boundsnode->set_attribute_int("y", 0);
			boundsnode->set_attribute_int("width", screens[i].physical_x());
			boundsnode->set_attribute_int("height", screens[i].physical_y());
		}

		// generate individual pixel aspect views
		for (unsigned i = 0; screens.size() > i; ++i)
		{
			if (!screens[i].square())
			{
				util::xml::data_node *const viewnode(layoutnode->add_child("view", nullptr));
				if (!viewnode)
					throw emu_fatalerror("Couldn't create XML node??");
				viewnode->set_attribute(
						"name",
						util::string_format(
							"Screen %1$u Pixel Aspect (%2$u:%3$u)",
							i, screens[i].native_x(), screens[i].native_y()).c_str());
				util::xml::data_node *const screennode(viewnode->add_child("screen", nullptr));
				if (!screennode)
					throw emu_fatalerror("Couldn't create XML node??");
				screennode->set_attribute_int("index", i);
				util::xml::data_node *const boundsnode(screennode->add_child("bounds", nullptr));
				if (!boundsnode)
					throw emu_fatalerror("Couldn't create XML node??");
				boundsnode->set_attribute_int("x", 0);
				boundsnode->set_attribute_int("y", 0);
				boundsnode->set_attribute_int("width", screens[i].native_x());
				boundsnode->set_attribute_int("height", screens[i].native_y());
			}
		}

		// generate the fake cocktail view for single-screen systems
		if (screens.size() == 1U)
		{
			util::xml::data_node *const viewnode(layoutnode->add_child("view", nullptr));
			if (!viewnode)
				throw emu_fatalerror("Couldn't create XML node??");
			viewnode->set_attribute("name", "Cocktail");

			util::xml::data_node *const mirrornode(viewnode->add_child("screen", nullptr));
			if (!mirrornode)
				throw emu_fatalerror("Couldn't create XML node??");
			mirrornode->set_attribute_int("index", 0);
			util::xml::data_node *const mirrorbounds(mirrornode->add_child("bounds", nullptr));
			if (!mirrorbounds)
				throw emu_fatalerror("Couldn't create XML node??");
			mirrorbounds->set_attribute_int("x", 0);
			mirrorbounds->set_attribute_float("y", (-0.01 * (std::min)(screens[0].physical_x(), screens[0].physical_y())) - screens[0].physical_y());
			mirrorbounds->set_attribute_int("width", screens[0].physical_x());
			mirrorbounds->set_attribute_int("height", screens[0].physical_y());
			util::xml::data_node *const flipper(mirrornode->add_child("orientation", nullptr));
			if (!flipper)
				throw emu_fatalerror("Couldn't create XML node??");
			flipper->set_attribute_int("rotate", 180);

			util::xml::data_node *const screennode(viewnode->add_child("screen", nullptr));
			if (!screennode)
				throw emu_fatalerror("Couldn't create XML node??");
			screennode->set_attribute_int("index", 0);
			util::xml::data_node *const screenbounds(screennode->add_child("bounds", nullptr));
			if (!screenbounds)
				throw emu_fatalerror("Couldn't create XML node??");
			screenbounds->set_attribute_int("x", 0);
			screenbounds->set_attribute_int("y", 0);
			screenbounds->set_attribute_int("width", screens[0].physical_x());
			screenbounds->set_attribute_int("height", screens[0].physical_y());
		}

		// generate tiled views if the supplied artwork doesn't provide a view of all screens
		bool need_tiles(screens.size() >= 3);
		if (!need_tiles && (screens.size() >= 2))
		{
			need_tiles = true;
			int viewindex(0);
			for (layout_view *view = nth_view(viewindex); need_tiles && view; view = nth_view(++viewindex))
			{
				bool screen_missing(false);
				for (screen_device &screen : iter)
				{
					if (!view->has_screen(screen))
					{
						screen_missing = true;
						break;
					}
				}
				if (!screen_missing)
					need_tiles = false;
			}
		}
		if (need_tiles)
		{
			// helpers for generating a view since we do this a lot
			std::vector<float> widths(screens.size()), heights(screens.size());
			std::vector<std::pair<float, float> > sizes(screens.size());
			std::transform(screens.begin(), screens.end(), sizes.begin(), [] (screen_info const &s) { return s.tiled_size(); });
			auto const generate_view =
					[&layoutnode, &screens, &widths, &heights, &sizes] (char const *title, unsigned columns, bool gapless, auto &&mapper)
					{
						// calculate necessary widths/heights of rows/columns restricting screens to unit square
						assert(0U < columns);
						assert(screens.size() >= columns);
						unsigned const rows((screens.size() + columns - 1) / columns);
						std::fill_n(widths.begin(), columns, 0.0F);
						std::fill_n(heights.begin(), rows, 0.0F);
						for (unsigned y = 0U; rows > y; ++y)
						{
							for (unsigned x = 0U; columns > x; ++x)
							{
								int const i(mapper(x, y));
								if (0 <= i)
								{
									widths[x] = (std::max)(widths[x], sizes[i].first);
									heights[y] = (std::max)(heights[y], sizes[i].second);
								}
							}
						}

						// spacing is 1% of minor dimension
						float spacing(0.0F);
						if (!gapless)
						{
							spacing = 0.01F * (std::min)(
									*std::max_element(widths.begin(), widths.begin() + columns),
									*std::max_element(heights.begin(), heights.begin() + rows));
						}

						// actually generate elements
						util::xml::data_node *viewnode = layoutnode->add_child("view", nullptr);
						if (!viewnode)
							throw emu_fatalerror("Couldn't create XML node??");
						viewnode->set_attribute("name", title);
						float ypos(0.0F);
						for (unsigned y = 0U; rows > y; ypos += heights[y] + spacing, ++y)
						{
							float xpos(0.0F);
							for (unsigned x = 0U; columns > x; xpos += widths[x] + spacing, ++x)
							{
								int const i(mapper(x, y));
								if (0 <= i)
								{
									util::xml::data_node *const screennode(viewnode->add_child("screen", nullptr));
									if (!screennode)
										throw emu_fatalerror("Couldn't create XML node??");
									screennode->set_attribute_int("index", i);
									util::xml::data_node *const boundsnode(screennode->add_child("bounds", nullptr));
									if (!boundsnode)
										throw emu_fatalerror("Couldn't create XML node??");
									boundsnode->set_attribute_float("x", xpos + ((widths[x] - sizes[i].first) / 2));
									boundsnode->set_attribute_float("y", ypos + ((heights[y] - sizes[i].second) / 2));
									boundsnode->set_attribute_float("width", sizes[i].first);
									boundsnode->set_attribute_float("height", sizes[i].second);
								}
							}
						}
					};

			// generate linear views
			generate_view("Left-to-Right", screens.size(), false, [] (unsigned x, unsigned y) { return x; });
			generate_view("Left-to-Right (Gapless)", screens.size(), true, [] (unsigned x, unsigned y) { return x; });
			generate_view("Top-to-Bottom", 1U, false, [] (unsigned x, unsigned y) { return y; });
			generate_view("Top-to-Bottom (Gapless)", 1U, true, [] (unsigned x, unsigned y) { return y; });

			// generate fake cocktail view for systems with two screens
			if (screens.size() == 2U)
			{
				float const height0(float(screens[0].physical_y()) / screens[0].physical_x());
				float const height1(float(screens[1].physical_y()) / screens[1].physical_x());
				float const minor_dim((std::max)((std::min)(height0, 1.0F), (std::min)(height1, 1.0F)));

				util::xml::data_node *const viewnode(layoutnode->add_child("view", nullptr));
				if (!viewnode)
					throw emu_fatalerror("Couldn't create XML node??");
				viewnode->set_attribute("name", "Cocktail");

				util::xml::data_node *const mirrornode(viewnode->add_child("screen", nullptr));
				if (!mirrornode)
					throw emu_fatalerror("Couldn't create XML node??");
				mirrornode->set_attribute_int("index", 1);
				util::xml::data_node *const mirrorbounds(mirrornode->add_child("bounds", nullptr));
				if (!mirrorbounds)
					throw emu_fatalerror("Couldn't create XML node??");
				mirrorbounds->set_attribute_int("x", 0);
				mirrorbounds->set_attribute_float("y", (-0.01 * minor_dim) - height1);
				mirrorbounds->set_attribute_int("width", 1);
				mirrorbounds->set_attribute_float("height", height1);
				util::xml::data_node *const flipper(mirrornode->add_child("orientation", nullptr));
				if (!flipper)
					throw emu_fatalerror("Couldn't create XML node??");
				flipper->set_attribute_int("rotate", 180);

				util::xml::data_node *const screennode(viewnode->add_child("screen", nullptr));
				if (!screennode)
					throw emu_fatalerror("Couldn't create XML node??");
				screennode->set_attribute_int("index", 0);
				util::xml::data_node *const screenbounds(screennode->add_child("bounds", nullptr));
				if (!screenbounds)
					throw emu_fatalerror("Couldn't create XML node??");
				screenbounds->set_attribute_int("x", 0);
				screenbounds->set_attribute_int("y", 0);
				screenbounds->set_attribute_int("width", 1);
				screenbounds->set_attribute_float("height", height0);
			}

			// generate tiled views
			for (unsigned mindim = 2; ((screens.size() + mindim - 1) / mindim) >= mindim; ++mindim)
			{
				unsigned const majdim((screens.size() + mindim - 1) / mindim);
				unsigned const remainder(screens.size() % majdim);
				if (!remainder || (((majdim + 1) / 2) <= remainder))
				{
					generate_view(
							util::string_format("%1$u\xC3\x97%2$u Left-to-Right, Top-to-Bottom", majdim, mindim).c_str(),
							majdim,
							false,
							[&screens, majdim] (unsigned x, unsigned y)
							{
								unsigned const i(x + (y * majdim));
								return (screens.size() > i) ? int(i) : -1;
							});
					generate_view(
							util::string_format("%1$u\xC3\x97%2$u Left-to-Right, Top-to-Bottom (Gapless)", majdim, mindim).c_str(),
							majdim,
							true,
							[&screens, majdim] (unsigned x, unsigned y)
							{
								unsigned const i(x + (y * majdim));
								return (screens.size() > i) ? int(i) : -1;
							});
					generate_view(
							util::string_format("%1$u\xC3\x97%2$u Top-to-Bottom, Left-to-Right", mindim, majdim).c_str(),
							mindim,
							false,
							[&screens, majdim] (unsigned x, unsigned y)
							{
								unsigned const i((x * majdim) + y);
								return (screens.size() > i) ? int(i) : -1;
							});
					generate_view(
							util::string_format("%1$u\xC3\x97%2$u Top-to-Bottom, Left-to-Right (Gapless)", mindim, majdim).c_str(),
							mindim,
							true,
							[&screens, majdim] (unsigned x, unsigned y)
							{
								unsigned const i((x * majdim) + y);
								return (screens.size() > i) ? int(i) : -1;
							});
				}
			}
		}

		// try to parse it
		if (!load_layout_file(m_manager.machine().root_device(), *root, m_manager.machine().options().art_path(), nullptr))
			throw emu_fatalerror("Couldn't parse generated layout??");
	}
}


//-------------------------------------------------
//  load_layout_file - load a single layout file
//  and append it to our list
//-------------------------------------------------

bool render_target::load_layout_file(const char *dirname, const internal_layout &layout_data, device_t *device)
{
	// +1 to ensure data is terminated for XML parser
	auto tempout = make_unique_clear<u8 []>(layout_data.decompressed_size + 1);

	z_stream stream;
	int zerr;

	// initialize the stream
	memset(&stream, 0, sizeof(stream));
	stream.next_out = tempout.get();
	stream.avail_out = layout_data.decompressed_size;

	zerr = inflateInit(&stream);
	if (zerr != Z_OK)
	{
		osd_printf_error("render_target::load_layout_file: zlib initialization error\n");
		return false;
	}

	// decompress this chunk
	stream.next_in = (unsigned char *)layout_data.data;
	stream.avail_in = layout_data.compressed_size;
	zerr = inflate(&stream, Z_NO_FLUSH);

	// stop at the end of the stream
	if (zerr == Z_STREAM_END)
	{
		// OK
	}
	else if (zerr != Z_OK)
	{
		osd_printf_error("render_target::load_layout_file: zlib decompression error\n");
		inflateEnd(&stream);
		return false;
	}

	// clean up
	zerr = inflateEnd(&stream);
	if (zerr != Z_OK)
		osd_printf_error("render_target::load_layout_file: zlib cleanup error\n");

	util::xml::file::ptr rootnode(util::xml::file::string_read(reinterpret_cast<char const *>(tempout.get()), nullptr));
	tempout.reset();

	// if we didn't get a properly-formatted XML file, record a warning and exit
	if (!load_layout_file(device ? *device : m_manager.machine().root_device(), *rootnode, m_manager.machine().options().art_path(), dirname))
	{
		osd_printf_warning("Improperly formatted XML string, ignoring\n");
		return false;
	}
	else
	{
		return true;
	}
}

bool render_target::load_layout_file(const char *dirname, const char *filename)
{
	// build the path and optionally prepend the directory
	std::string fname;
	if (dirname)
		fname.append(dirname).append(PATH_SEPARATOR);
	fname.append(filename).append(".lay");

	// attempt to open matching files
	util::xml::parse_options parseopt;
	util::xml::parse_error parseerr;
	parseopt.error = &parseerr;
	emu_file layoutfile(m_manager.machine().options().art_path(), OPEN_FLAG_READ);
	layoutfile.set_restrict_to_mediapath(1);
	bool result(false);
	for (osd_file::error filerr = layoutfile.open(fname); osd_file::error::NONE == filerr; filerr = layoutfile.open_next())
	{
		// read the file and parse as XML
		util::xml::file::ptr const rootnode(util::xml::file::read(layoutfile, &parseopt));
		if (rootnode)
		{
			// extract directory name from location of layout file
			std::string artdir(layoutfile.fullpath());
			auto const dirsep(std::find_if(artdir.rbegin(), artdir.rend(), &util::is_directory_separator));
			artdir.erase(dirsep.base(), artdir.end());

			// record a warning if we didn't get a properly-formatted XML file
			if (!load_layout_file(m_manager.machine().root_device(), *rootnode, nullptr, artdir.c_str()))
				osd_printf_warning("Improperly formatted XML layout file '%s', ignoring\n", filename);
			else
				result = true;
		}
		else if (parseerr.error_message)
		{
			osd_printf_warning(
					"Error parsing XML layout file '%s' at line %d column %d: %s, ignoring\n",
					filename,
					parseerr.error_line,
					parseerr.error_column,
					parseerr.error_message);
		}
		else
		{
			osd_printf_warning("Error parsing XML layout file '%s', ignorning\n", filename);
		}
	}
	return result;
}

bool render_target::load_layout_file(device_t &device, util::xml::data_node const &rootnode, const char *searchpath, const char *dirname)
{
	// parse and catch any errors
	try
	{
		m_filelist->emplace_back(device, rootnode, searchpath, dirname);
	}
	catch (emu_fatalerror &err)
	{
		osd_printf_warning("%s\n", err.what());
		return false;
	}

	return true;
}


//-------------------------------------------------
//  add_container_primitives - add primitives
//  based on the container
//-------------------------------------------------

void render_target::add_container_primitives(render_primitive_list &list, const object_transform &root_xform, const object_transform &xform, render_container &container, int blendmode)
{
	// first update the palette for the container, if it is dirty
	container.update_palette();

	// compute the clip rect
	render_bounds cliprect;
	cliprect.x0 = xform.xoffs;
	cliprect.y0 = xform.yoffs;
	cliprect.x1 = xform.xoffs + xform.xscale;
	cliprect.y1 = xform.yoffs + xform.yscale;
	cliprect &= m_bounds;

	float root_xoffs = root_xform.xoffs + fabsf(root_xform.xscale - xform.xscale) * 0.5f;
	float root_yoffs = root_xform.yoffs + fabsf(root_xform.yscale - xform.yscale) * 0.5f;

	render_bounds root_cliprect;
	root_cliprect.x0 = root_xoffs;
	root_cliprect.y0 = root_yoffs;
	root_cliprect.x1 = root_xoffs + root_xform.xscale;
	root_cliprect.y1 = root_yoffs + root_xform.yscale;
	root_cliprect &= m_bounds;

	// compute the container transform
	object_transform container_xform;
	container_xform.orientation = orientation_add(container.orientation(), xform.orientation);
	{
		float xscale = (container_xform.orientation & ORIENTATION_SWAP_XY) ? container.yscale() : container.xscale();
		float yscale = (container_xform.orientation & ORIENTATION_SWAP_XY) ? container.xscale() : container.yscale();
		float xoffs = (container_xform.orientation & ORIENTATION_SWAP_XY) ? container.yoffset() : container.xoffset();
		float yoffs = (container_xform.orientation & ORIENTATION_SWAP_XY) ? container.xoffset() : container.yoffset();
		if (container_xform.orientation & ORIENTATION_FLIP_X) xoffs = -xoffs;
		if (container_xform.orientation & ORIENTATION_FLIP_Y) yoffs = -yoffs;
		if (!m_transform_container)
		{
			xscale = 1.0f;
			yscale = 1.0f;
			xoffs = 0.0f;
			yoffs = 0.0f;
		}
		container_xform.xscale = xform.xscale * xscale;
		container_xform.yscale = xform.yscale * yscale;
		if (xform.no_center)
		{
			container_xform.xoffs = xform.xscale * (xoffs) + xform.xoffs;
			container_xform.yoffs = xform.yscale * (yoffs) + xform.yoffs;
		}
		else
		{
			container_xform.xoffs = xform.xscale * (0.5f - 0.5f * xscale + xoffs) + xform.xoffs;
			container_xform.yoffs = xform.yscale * (0.5f - 0.5f * yscale + yoffs) + xform.yoffs;
		}
		container_xform.color = xform.color;
	}

	// iterate over elements
	for (render_container::item &curitem : container.items())
	{
		// compute the oriented bounds
		render_bounds bounds = curitem.bounds();
		apply_orientation(bounds, container_xform.orientation);

		float xscale = container_xform.xscale;
		float yscale = container_xform.yscale;
		float xoffs = container_xform.xoffs;
		float yoffs = container_xform.yoffs;
		if (!m_transform_container && PRIMFLAG_GET_VECTOR(curitem.flags()))
		{
			xoffs = root_xoffs;
			yoffs = root_yoffs;
		}

		// allocate the primitive and set the transformed bounds/color data
		render_primitive *prim = list.alloc(render_primitive::INVALID);

		prim->container = &container; /* pass the container along for access to user_settings */

		prim->bounds.x0 = render_round_nearest(xoffs + bounds.x0 * xscale);
		prim->bounds.y0 = render_round_nearest(yoffs + bounds.y0 * yscale);
		if (curitem.internal() & INTERNAL_FLAG_CHAR)
		{
			prim->bounds.x1 = prim->bounds.x0 + render_round_nearest((bounds.x1 - bounds.x0) * xscale);
			prim->bounds.y1 = prim->bounds.y0 + render_round_nearest((bounds.y1 - bounds.y0) * yscale);
		}
		else
		{
			prim->bounds.x1 = render_round_nearest(xoffs + bounds.x1 * xscale);
			prim->bounds.y1 = render_round_nearest(yoffs + bounds.y1 * yscale);
		}

		// compute the color of the primitive
		prim->color.r = container_xform.color.r * curitem.color().r;
		prim->color.g = container_xform.color.g * curitem.color().g;
		prim->color.b = container_xform.color.b * curitem.color().b;
		prim->color.a = container_xform.color.a * curitem.color().a;

		// copy unclipped bounds
		prim->full_bounds = prim->bounds;

		// now switch off the type
		bool clipped = true;
		switch (curitem.type())
		{
			case CONTAINER_ITEM_LINE:
				// adjust the color for brightness/contrast/gamma
				prim->color.a = container.apply_brightness_contrast_gamma_fp(prim->color.a);
				prim->color.r = container.apply_brightness_contrast_gamma_fp(prim->color.r);
				prim->color.g = container.apply_brightness_contrast_gamma_fp(prim->color.g);
				prim->color.b = container.apply_brightness_contrast_gamma_fp(prim->color.b);

				// set the line type
				prim->type = render_primitive::LINE;
				prim->flags |= PRIMFLAG_TYPE_LINE;

				// scale the width by the minimum of X/Y scale factors
				prim->width = curitem.width() * std::min(container_xform.xscale, container_xform.yscale);
				prim->flags |= curitem.flags();

				// clip the primitive
				if (!m_transform_container && PRIMFLAG_GET_VECTOR(curitem.flags()))
				{
					clipped = render_clip_line(&prim->bounds, &root_cliprect);
				}
				else
				{
					clipped = render_clip_line(&prim->bounds, &cliprect);
				}
				break;

			case CONTAINER_ITEM_QUAD:
				// set the quad type
				prim->type = render_primitive::QUAD;
				prim->flags |= PRIMFLAG_TYPE_QUAD;

				// normalize the bounds
				normalize_bounds(prim->bounds);

				// get the scaled bitmap and set the resulting palette
				if (curitem.texture() != nullptr)
				{
					// determine the final orientation
					int finalorient = orientation_add(PRIMFLAG_GET_TEXORIENT(curitem.flags()), container_xform.orientation);

					// based on the swap values, get the scaled final texture
					int width = (finalorient & ORIENTATION_SWAP_XY) ? (prim->bounds.y1 - prim->bounds.y0) : (prim->bounds.x1 - prim->bounds.x0);
					int height = (finalorient & ORIENTATION_SWAP_XY) ? (prim->bounds.x1 - prim->bounds.x0) : (prim->bounds.y1 - prim->bounds.y0);
					width = std::min(width, m_maxtexwidth);
					height = std::min(height, m_maxtexheight);

					curitem.texture()->get_scaled(width, height, prim->texture, list, curitem.flags());

					// set the palette
					prim->texture.palette = curitem.texture()->get_adjusted_palette(container, prim->texture.palette_length);

					// determine UV coordinates
					prim->texcoords = oriented_texcoords[finalorient];

					// apply clipping
					clipped = render_clip_quad(&prim->bounds, &cliprect, &prim->texcoords);

					// apply the final orientation from the quad flags and then build up the final flags
					prim->flags |= (curitem.flags() & ~(PRIMFLAG_TEXORIENT_MASK | PRIMFLAG_BLENDMODE_MASK | PRIMFLAG_TEXFORMAT_MASK))
						| PRIMFLAG_TEXORIENT(finalorient)
						| PRIMFLAG_TEXFORMAT(curitem.texture()->format());
					prim->flags |= blendmode != -1
						? PRIMFLAG_BLENDMODE(blendmode)
						: PRIMFLAG_BLENDMODE(PRIMFLAG_GET_BLENDMODE(curitem.flags()));
				}
				else
				{
					// adjust the color for brightness/contrast/gamma
					prim->color.r = container.apply_brightness_contrast_gamma_fp(prim->color.r);
					prim->color.g = container.apply_brightness_contrast_gamma_fp(prim->color.g);
					prim->color.b = container.apply_brightness_contrast_gamma_fp(prim->color.b);

					// no texture
					prim->texture.base = nullptr;

					if (PRIMFLAG_GET_VECTORBUF(curitem.flags()))
					{
						// flags X(1) flip-x, Y(2) flip-y, S(4) swap-xy
						//
						// X  Y  S   e.g.       flips
						// 0  0  0   asteroid   !X !Y
						// 0  0  1   -           X  Y
						// 0  1  0   speedfrk   !X  Y
						// 0  1  1   tempest    !X  Y
						// 1  0  0   -           X !Y
						// 1  0  1   -           x !Y
						// 1  1  0   solarq      X  Y
						// 1  1  1   barrier    !X !Y

						bool flip_x = (m_manager.machine().system().flags & ORIENTATION_FLIP_X) == ORIENTATION_FLIP_X;
						bool flip_y = (m_manager.machine().system().flags & ORIENTATION_FLIP_Y) == ORIENTATION_FLIP_Y;
						bool swap_xy = (m_manager.machine().system().flags & ORIENTATION_SWAP_XY) == ORIENTATION_SWAP_XY;

						int vectororient = 0;
						if (flip_x)
						{
							vectororient |= ORIENTATION_FLIP_X;
						}
						if (flip_y)
						{
							vectororient |= ORIENTATION_FLIP_Y;
						}
						if ((flip_x && flip_y && swap_xy) || (!flip_x && !flip_y && swap_xy))
						{
							vectororient ^= ORIENTATION_FLIP_X;
							vectororient ^= ORIENTATION_FLIP_Y;
						}

						// determine the final orientation (textures are up-side down, so flip axis for vectors to immitate that behavior)
						int finalorient = orientation_add(vectororient, container_xform.orientation);

						// determine UV coordinates
						prim->texcoords = oriented_texcoords[finalorient];

						// apply clipping
						clipped = render_clip_quad(&prim->bounds, &cliprect, &prim->texcoords);

						// apply the final orientation from the quad flags and then build up the final flags
						prim->flags |= (curitem.flags() & ~(PRIMFLAG_TEXORIENT_MASK | PRIMFLAG_BLENDMODE_MASK | PRIMFLAG_TEXFORMAT_MASK))
							| PRIMFLAG_TEXORIENT(finalorient);
						prim->flags |= blendmode != -1
							? PRIMFLAG_BLENDMODE(blendmode)
							: PRIMFLAG_BLENDMODE(PRIMFLAG_GET_BLENDMODE(curitem.flags()));
					}
					else
					{
						// set the basic flags
						prim->flags |= (curitem.flags() & ~PRIMFLAG_BLENDMODE_MASK)
							| PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA);

						// apply clipping
						clipped = render_clip_quad(&prim->bounds, &cliprect, nullptr);
					}
				}
				break;
		}

		// add to the list or free if we're clipped out
		list.append_or_return(*prim, clipped);
	}

	// add the overlay if it exists
	if (container.overlay() != nullptr && m_layerconfig.screen_overlay_enabled())
	{
		s32 width, height;

		// allocate a primitive
		render_primitive *prim = list.alloc(render_primitive::QUAD);
		prim->bounds.set_wh(xform.xoffs, xform.yoffs, xform.xscale, xform.yscale);
		prim->full_bounds = prim->bounds;
		prim->color = container_xform.color;
		width = render_round_nearest(prim->bounds.x1) - render_round_nearest(prim->bounds.x0);
		height = render_round_nearest(prim->bounds.y1) - render_round_nearest(prim->bounds.y0);

		container.overlay()->get_scaled(
			(container_xform.orientation & ORIENTATION_SWAP_XY) ? height : width,
			(container_xform.orientation & ORIENTATION_SWAP_XY) ? width : height, prim->texture, list);

		// determine UV coordinates
		prim->texcoords = oriented_texcoords[container_xform.orientation];

		// set the flags and add it to the list
		prim->flags = PRIMFLAG_TEXORIENT(container_xform.orientation)
			| PRIMFLAG_BLENDMODE(BLENDMODE_RGB_MULTIPLY)
			| PRIMFLAG_TEXFORMAT(container.overlay()->format())
			| PRIMFLAG_TEXSHADE(1);

		list.append_or_return(*prim, false);
	}
}


//-------------------------------------------------
//  add_element_primitives - add the primitive
//  for an element in the current state
//-------------------------------------------------

void render_target::add_element_primitives(render_primitive_list &list, const object_transform &xform, layout_element &element, int state, int blendmode)
{
	// limit state range to non-negative values
	if (state < 0)
		state = 0;

	// get a pointer to the relevant texture
	render_texture *texture = element.state_texture(state);
	if (texture)
	{
		render_primitive *prim = list.alloc(render_primitive::QUAD);

		// configure the basics
		prim->color = xform.color;
		prim->flags = PRIMFLAG_TEXORIENT(xform.orientation) | PRIMFLAG_BLENDMODE(blendmode) | PRIMFLAG_TEXFORMAT(texture->format());

		// compute the bounds
		s32 width = render_round_nearest(xform.xscale);
		s32 height = render_round_nearest(xform.yscale);
		prim->bounds.set_wh(render_round_nearest(xform.xoffs), render_round_nearest(xform.yoffs), float(width), float(height));
		prim->full_bounds = prim->bounds;
		if (xform.orientation & ORIENTATION_SWAP_XY)
			std::swap(width, height);
		width = (std::min)(width, m_maxtexwidth);
		height = (std::min)(height, m_maxtexheight);

		// get the scaled texture and append it
		texture->get_scaled(width, height, prim->texture, list, prim->flags);

		// compute the clip rect
		render_bounds cliprect = prim->bounds & m_bounds;

		// determine UV coordinates and apply clipping
		prim->texcoords = oriented_texcoords[xform.orientation];
		bool clipped = render_clip_quad(&prim->bounds, &cliprect, &prim->texcoords);

		// add to the list or free if we're clipped out
		list.append_or_return(*prim, clipped);
	}
}


//-------------------------------------------------
//  map_point_internal - internal logic for
//  mapping points
//-------------------------------------------------

std::pair<float, float> render_target::map_point_internal(s32 target_x, s32 target_y)
{
	// compute the visible width/height
	s32 viswidth, visheight;
	compute_visible_area(m_width, m_height, m_pixel_aspect, m_orientation, viswidth, visheight);

	// create a root transform for the target
	object_transform root_xform;
	root_xform.xoffs = float(m_width - viswidth) / 2;
	root_xform.yoffs = float(m_height - visheight) / 2;

	// convert target coordinates to float
	if (!m_manager.machine().ui().is_menu_active())
		return std::make_pair(float(target_x - root_xform.xoffs) / viswidth, float(target_y - root_xform.yoffs) / visheight);
	else
		return std::make_pair(float(target_x) / m_width, float(target_y) / m_height);
}


//-------------------------------------------------
//  view_name - return the name of the indexed
//  view, or nullptr if it doesn't exist
//-------------------------------------------------

layout_view *render_target::view_by_index(unsigned index)
{
	return (m_views.size() > index) ? &m_views[index].first : nullptr;
}


//-------------------------------------------------
//  view_index - return the index of the given
//  view
//-------------------------------------------------

int render_target::view_index(layout_view &targetview) const
{
	// return index of view, or zero if not found
	for (int index = 0; m_views.size() > index; ++index)
	{
		if (&m_views[index].first == &targetview)
			return index;
	}
	return 0;
}


//-------------------------------------------------
//  config_load - process config information
//-------------------------------------------------

void render_target::config_load(util::xml::data_node const &targetnode)
{
	// find the view
	const char *viewname = targetnode.get_attribute_string("view", nullptr);
	if (viewname != nullptr)
		for (int viewnum = 0; viewnum < 1000; viewnum++)
		{
			const char *testname = view_name(viewnum);
			if (testname == nullptr)
				break;
			if (!strcmp(viewname, testname))
			{
				set_view(viewnum);
				break;
			}
		}

	// modify the artwork config
	int const zoom = targetnode.get_attribute_int("zoom", -1);
	if (zoom == 0 || zoom == 1)
		set_zoom_to_screen(zoom);

	// apply orientation
	int rotate = targetnode.get_attribute_int("rotate", -1);
	if (rotate != -1)
	{
		if (rotate == 90)
			rotate = ROT90;
		else if (rotate == 180)
			rotate = ROT180;
		else if (rotate == 270)
			rotate = ROT270;
		else
			rotate = ROT0;
		set_orientation(orientation_add(rotate, orientation()));

		// apply the opposite orientation to the UI
		if (is_ui_target())
		{
			render_container &ui_container = m_manager.ui_container();
			render_container::user_settings settings = ui_container.get_user_settings();
			settings.m_orientation = orientation_add(orientation_reverse(rotate), settings.m_orientation);
			ui_container.set_user_settings(settings);
		}
	}

	// apply per-view settings
	for (util::xml::data_node const *viewnode = targetnode.get_child("view"); viewnode; viewnode = viewnode->get_next_sibling("view"))
	{
		char const *const viewname = viewnode->get_attribute_string("name", nullptr);
		if (!viewname)
			continue;

		auto const view = std::find_if(m_views.begin(), m_views.end(), [viewname] (auto const &x) { return x.first.name() == viewname; });
		if (m_views.end() == view)
			continue;

		for (util::xml::data_node const *vistogglenode = viewnode->get_child("collection"); vistogglenode; vistogglenode = vistogglenode->get_next_sibling("collection"))
		{
			char const *const vistogglename = vistogglenode->get_attribute_string("name", nullptr);
			if (!vistogglename)
				continue;

			auto const &vistoggles = view->first.visibility_toggles();
			auto const vistoggle = std::find_if(vistoggles.begin(), vistoggles.end(), [vistogglename] (auto const &x) { return x.name() == vistogglename; });
			if (vistoggles.end() == vistoggle)
				continue;

			int const enable = vistogglenode->get_attribute_int("visible", -1);
			if (0 <= enable)
			{
				if (enable)
					view->second |= u32(1) << std::distance(vistoggles.begin(), vistoggle);
				else
					view->second &= ~(u32(1) << std::distance(vistoggles.begin(), vistoggle));
			}
		}

		if (&current_view() == &view->first)
		{
			current_view().recompute(visibility_mask(), m_layerconfig.zoom_to_screen());
			current_view().preload();
		}
	}
}


//-------------------------------------------------
//  config_save - save our configuration, or
//  return false if we are the same as the default
//-------------------------------------------------

bool render_target::config_save(util::xml::data_node &targetnode)
{
	bool changed = false;

	// output the basics
	targetnode.set_attribute_int("index", index());

	// output the view
	if (&current_view() != m_base_view)
	{
		targetnode.set_attribute("view", current_view().name().c_str());
		changed = true;
	}

	// output the layer config
	if (m_layerconfig != m_base_layerconfig)
	{
		targetnode.set_attribute_int("zoom", m_layerconfig.zoom_to_screen());
		changed = true;
	}

	// output rotation
	if (m_orientation != m_base_orientation)
	{
		int rotate = 0;
		if (orientation_add(ROT90, m_base_orientation) == m_orientation)
			rotate = 90;
		else if (orientation_add(ROT180, m_base_orientation) == m_orientation)
			rotate = 180;
		else if (orientation_add(ROT270, m_base_orientation) == m_orientation)
			rotate = 270;
		assert(rotate != 0);
		targetnode.set_attribute_int("rotate", rotate);
		changed = true;
	}

	// output layer configuration
	for (auto const &view : m_views)
	{
		u32 const defvismask = view.first.default_visibility_mask();
		if (defvismask != view.second)
		{
			util::xml::data_node *viewnode = nullptr;
			unsigned i = 0;
			for (layout_view::visibility_toggle const &toggle : view.first.visibility_toggles())
			{
				if (BIT(defvismask, i) != BIT(view.second, i))
				{
					if (!viewnode)
					{
						viewnode = targetnode.add_child("view", nullptr);
						viewnode->set_attribute("name", view.first.name().c_str());
					}
					util::xml::data_node *const vistogglenode = viewnode->add_child("collection", nullptr);
					vistogglenode->set_attribute("name", toggle.name().c_str());
					vistogglenode->set_attribute_int("visible", BIT(view.second, i));
					changed = true;
				}
				++i;
			}
		}
	}

	return changed;
}


//-------------------------------------------------
//  init_clear_extents - reset the extents list
//-------------------------------------------------

void render_target::init_clear_extents()
{
	m_clear_extents[0] = -m_height;
	m_clear_extents[1] = 1;
	m_clear_extents[2] = m_width;
	m_clear_extent_count = 3;
}


//-------------------------------------------------
//  remove_clear_extent - remove a quad from the
//  list of stuff to clear, unless it overlaps
//  a previous quad
//-------------------------------------------------

bool render_target::remove_clear_extent(const render_bounds &bounds)
{
	s32 *max = &m_clear_extents[MAX_CLEAR_EXTENTS];
	s32 *last = &m_clear_extents[m_clear_extent_count];
	s32 *ext = &m_clear_extents[0];
	s32 boundsx0 = ceil(bounds.x0);
	s32 boundsx1 = floor(bounds.x1);
	s32 boundsy0 = ceil(bounds.y0);
	s32 boundsy1 = floor(bounds.y1);
	s32 y0, y1 = 0;

	// loop over Y extents
	while (ext < last)
	{
		s32 *linelast;

		// first entry of each line should always be negative
		assert(ext[0] < 0.0f);
		y0 = y1;
		y1 = y0 - ext[0];

		// do we intersect this extent?
		if (boundsy0 < y1 && boundsy1 > y0)
		{
			s32 *xext;
			s32 x0, x1 = 0;

			// split the top
			if (y0 < boundsy0)
			{
				int diff = boundsy0 - y0;

				// make a copy of this extent
				memmove(&ext[ext[1] + 2], &ext[0], (last - ext) * sizeof(*ext));
				last += ext[1] + 2;
				if (last >= max)
					throw emu_fatalerror("render_target::remove_clear_extent: Ran out of clear extents!");

				// split the extent between pieces
				ext[ext[1] + 2] = -(-ext[0] - diff);
				ext[0] = -diff;

				// advance to the new extent
				y0 -= ext[0];
				ext += ext[1] + 2;
				y1 = y0 - ext[0];
			}

			// split the bottom
			if (y1 > boundsy1)
			{
				int diff = y1 - boundsy1;

				// make a copy of this extent
				memmove(&ext[ext[1] + 2], &ext[0], (last - ext) * sizeof(*ext));
				last += ext[1] + 2;
				if (last >= max)
					throw emu_fatalerror("render_target::remove_clear_extent: Ran out of clear extents!");

				// split the extent between pieces
				ext[ext[1] + 2] = -diff;
				ext[0] = -(-ext[0] - diff);

				// recompute y1
				y1 = y0 - ext[0];
			}

			// now remove the X extent
			linelast = &ext[ext[1] + 2];
			xext = &ext[2];
			while (xext < linelast)
			{
				x0 = x1;
				x1 = x0 + xext[0];

				// do we fully intersect this extent?
				if (boundsx0 >= x0 && boundsx1 <= x1)
				{
					// yes; split it
					memmove(&xext[2], &xext[0], (last - xext) * sizeof(*xext));
					last += 2;
					linelast += 2;
					if (last >= max)
						throw emu_fatalerror("render_target::remove_clear_extent: Ran out of clear extents!");

					// split this extent into three parts
					xext[0] = boundsx0 - x0;
					xext[1] = boundsx1 - boundsx0;
					xext[2] = x1 - boundsx1;

					// recompute x1
					x1 = boundsx1;
					xext += 2;
				}

				// do we partially intersect this extent?
				else if (boundsx0 < x1 && boundsx1 > x0)
					goto abort;

				// advance
				xext++;

				// do we partially intersect the next extent (which is a non-clear extent)?
				if (xext < linelast)
				{
					x0 = x1;
					x1 = x0 + xext[0];
					if (boundsx0 < x1 && boundsx1 > x0)
						goto abort;
					xext++;
				}
			}

			// update the count
			ext[1] = linelast - &ext[2];
		}

		// advance to the next row
		ext += 2 + ext[1];
	}

	// update the total count
	m_clear_extent_count = last - &m_clear_extents[0];
	return true;

abort:
	// update the total count even on a failure as we may have split extents
	m_clear_extent_count = last - &m_clear_extents[0];
	return false;
}


//-------------------------------------------------
//  add_clear_extents - add the accumulated
//  extents as a series of quads to clear
//-------------------------------------------------

void render_target::add_clear_extents(render_primitive_list &list)
{
	simple_list<render_primitive> clearlist;
	s32 *last = &m_clear_extents[m_clear_extent_count];
	s32 *ext = &m_clear_extents[0];
	s32 y0, y1 = 0;

	// loop over all extents
	while (ext < last)
	{
		s32 *linelast = &ext[ext[1] + 2];
		s32 *xext = &ext[2];
		s32 x0, x1 = 0;

		// first entry should always be negative
		assert(ext[0] < 0);
		y0 = y1;
		y1 = y0 - ext[0];

		// now remove the X extent
		while (xext < linelast)
		{
			x0 = x1;
			x1 = x0 + *xext++;

			// only add entries for non-zero widths
			if (x1 - x0 > 0)
			{
				render_primitive *prim = list.alloc(render_primitive::QUAD);
				prim->bounds.set_xy(float(x0), float(y0), float(x1), float(y1));
				prim->full_bounds = prim->bounds;
				prim->color.set(1.0f, 0.0f, 0.0f, 0.0f);
				prim->texture.base = nullptr;
				prim->flags = PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA);
				clearlist.append(*prim);
			}

			// skip the non-clearing extent
			x0 = x1;
			x1 = x0 + *xext++;
		}

		// advance to the next part
		ext += 2 + ext[1];
	}

	// we know that the first primitive in the list will be the global clip
	// so we insert the clears immediately after
	list.m_primlist.prepend_list(clearlist);
}


//-------------------------------------------------
//  add_clear_and_optimize_primitive_list -
//  optimize the primitive list
//-------------------------------------------------

void render_target::add_clear_and_optimize_primitive_list(render_primitive_list &list)
{
	// start with the assumption that we need to clear the whole screen
	init_clear_extents();

	// scan the list until we hit an intersection quad or a line
	for (render_primitive &prim : list)
	{
		// switch off the type
		switch (prim.type)
		{
			case render_primitive::LINE:
				goto done;

			case render_primitive::QUAD:
			{
				// stop when we hit an alpha texture
				if (PRIMFLAG_GET_TEXFORMAT(prim.flags) == TEXFORMAT_ARGB32)
					goto done;

				// if this quad can't be cleanly removed from the extents list, we're done
				if (!remove_clear_extent(prim.bounds))
					goto done;

				// change the blendmode on the first primitive to be NONE
				if (PRIMFLAG_GET_BLENDMODE(prim.flags) == BLENDMODE_RGB_MULTIPLY)
				{
					// RGB multiply will multiply against 0, leaving nothing
					prim.color.set(1.0f, 0.0f, 0.0f, 0.0f);
					prim.texture.base = nullptr;
					prim.flags = (prim.flags & ~PRIMFLAG_BLENDMODE_MASK) | PRIMFLAG_BLENDMODE(BLENDMODE_NONE);
				}
				else
				{
					// for alpha or add modes, we will blend against 0 or add to 0; treat it like none
					prim.flags = (prim.flags & ~PRIMFLAG_BLENDMODE_MASK) | PRIMFLAG_BLENDMODE(BLENDMODE_NONE);
				}

				// since alpha is disabled, premultiply the RGB values and reset the alpha to 1.0
				prim.color.r *= prim.color.a;
				prim.color.g *= prim.color.a;
				prim.color.b *= prim.color.a;
				prim.color.a = 1.0f;
				break;
			}

			default:
				throw emu_fatalerror("Unexpected primitive type");
		}
	}

done:
	// now add the extents to the clear list
	add_clear_extents(list);
}



//**************************************************************************
//  CORE IMPLEMENTATION
//**************************************************************************

//-------------------------------------------------
//  render_manager - constructor
//-------------------------------------------------

render_manager::render_manager(running_machine &machine)
	: m_machine(machine)
	, m_ui_target(nullptr)
	, m_live_textures(0)
	, m_texture_id(0)
	, m_ui_container(new render_container(*this))
{
	// register callbacks
	machine.configuration().config_register("video", config_load_delegate(&render_manager::config_load, this), config_save_delegate(&render_manager::config_save, this));

	// create one container per screen
	for (screen_device &screen : screen_device_enumerator(machine.root_device()))
		screen.set_container(*container_alloc(&screen));
}


//-------------------------------------------------
//  ~render_manager - destructor
//-------------------------------------------------

render_manager::~render_manager()
{
	// free all the containers since they may own textures
	container_free(m_ui_container);
	m_screen_container_list.reset();

	// better not be any outstanding textures when we die
	assert(m_live_textures == 0);
}


//-------------------------------------------------
//  is_live - return if the screen is 'live'
//-------------------------------------------------

bool render_manager::is_live(screen_device &screen) const
{
	// iterate over all live targets and or together their screen masks
	for (render_target const &target : m_targetlist)
	{
		if (!target.hidden())
		{
			layout_view const *view = &target.current_view();
			if (view->has_visible_screen(screen))
				return true;
		}
	}
	return false;
}


//-------------------------------------------------
//  max_update_rate - return the smallest maximum
//  update rate across all targets
//-------------------------------------------------

float render_manager::max_update_rate() const
{
	// iterate over all live targets and or together their screen masks
	float minimum = 0;
	for (render_target &target : m_targetlist)
		if (target.max_update_rate() != 0)
		{
			if (minimum == 0)
				minimum = target.max_update_rate();
			else
				minimum = std::min(target.max_update_rate(), minimum);
		}

	return minimum;
}


//-------------------------------------------------
//  target_alloc - allocate a new target
//-------------------------------------------------

render_target *render_manager::target_alloc(const internal_layout *layoutfile, u32 flags)
{
	return &m_targetlist.append(*new render_target(*this, layoutfile, flags));
}

render_target *render_manager::target_alloc(util::xml::data_node const &layout, u32 flags)
{
	return &m_targetlist.append(*new render_target(*this, layout, flags));
}


//-------------------------------------------------
//  target_free - free a target
//-------------------------------------------------

void render_manager::target_free(render_target *target)
{
	if (target != nullptr)
		m_targetlist.remove(*target);
}


//-------------------------------------------------
//  target_by_index - get a render_target by index
//-------------------------------------------------

render_target *render_manager::target_by_index(int index) const
{
	// count up the targets until we hit the requested index
	for (render_target &target : m_targetlist)
		if (!target.hidden())
			if (index-- == 0)
				return &target;
	return nullptr;
}


//-------------------------------------------------
//  ui_aspect - return the aspect ratio for UI
//  fonts
//-------------------------------------------------

float render_manager::ui_aspect(render_container *rc)
{
	int orient;
	float aspect;

	if (rc == m_ui_container || rc == nullptr) {
		// ui container, aggregated multi-screen target

		orient = orientation_add(m_ui_target->orientation(), m_ui_container->orientation());
		// based on the orientation of the target, compute height/width or width/height
		if (!(orient & ORIENTATION_SWAP_XY))
				aspect = (float)m_ui_target->height() / (float)m_ui_target->width();
		else
				aspect = (float)m_ui_target->width() / (float)m_ui_target->height();

		// if we have a valid pixel aspect, apply that and return
		if (m_ui_target->pixel_aspect() != 0.0f)
		{
			float pixel_aspect = m_ui_target->pixel_aspect();

			if (orient & ORIENTATION_SWAP_XY)
				pixel_aspect = 1.0f / pixel_aspect;

			return aspect /= pixel_aspect;
		}

	} else {
		// single screen container

		orient = rc->orientation();
		// based on the orientation of the target, compute height/width or width/height
		if (!(orient & ORIENTATION_SWAP_XY))
			aspect = (float)rc->screen()->visible_area().height() / (float)rc->screen()->visible_area().width();
		else
			aspect = (float)rc->screen()->visible_area().width() / (float)rc->screen()->visible_area().height();
	}

	// clamp for extreme proportions
	if (aspect < 0.66f)
		aspect = 0.66f;
	if (aspect > 1.5f)
		aspect = 1.5f;

	return aspect;
}


//-------------------------------------------------
//  texture_alloc - allocate a new texture
//-------------------------------------------------

render_texture *render_manager::texture_alloc(texture_scaler_func scaler, void *param)
{
	// allocate a new texture and reset it
	render_texture *tex = m_texture_allocator.alloc();
	tex->reset(*this, scaler, param);
	tex->set_id(m_texture_id);
	m_texture_id++;
	m_live_textures++;
	return tex;
}


//-------------------------------------------------
//  texture_free - release a texture
//-------------------------------------------------

void render_manager::texture_free(render_texture *texture)
{
	if (texture != nullptr)
	{
		m_live_textures--;
		texture->release();
	}
	m_texture_allocator.reclaim(texture);
}


//-------------------------------------------------
//  font_alloc - allocate a new font instance
//-------------------------------------------------

std::unique_ptr<render_font> render_manager::font_alloc(const char *filename)
{
	return std::unique_ptr<render_font>(new render_font(*this, filename));
}


//-------------------------------------------------
//  invalidate_all - remove all refs to a
//  particular reference pointer
//-------------------------------------------------

void render_manager::invalidate_all(void *refptr)
{
	// permit nullptr
	if (refptr == nullptr)
		return;

	// loop over targets
	for (render_target &target : m_targetlist)
		target.invalidate_all(refptr);
}


//-------------------------------------------------
//  resolve_tags - resolve tag lookups
//-------------------------------------------------

void render_manager::resolve_tags()
{
	for (render_target &target : m_targetlist)
		target.resolve_tags();
}


//-------------------------------------------------
//  container_alloc - allocate a new container
//-------------------------------------------------

render_container *render_manager::container_alloc(screen_device *screen)
{
	auto container = new render_container(*this, screen);
	if (screen != nullptr)
		m_screen_container_list.append(*container);
	return container;
}


//-------------------------------------------------
//  container_free - release a container
//-------------------------------------------------

void render_manager::container_free(render_container *container)
{
	m_screen_container_list.remove(*container);
}


//-------------------------------------------------
//  config_load - read and apply data from the
//  configuration file
//-------------------------------------------------

void render_manager::config_load(config_type cfg_type, util::xml::data_node const *parentnode)
{
	// we only care about game files with matching nodes
	if ((cfg_type != config_type::GAME) || !parentnode)
		return;

	// check the UI target
	util::xml::data_node const *const uinode = parentnode->get_child("interface");
	if (uinode)
	{
		render_target *const target = target_by_index(uinode->get_attribute_int("target", 0));
		if (target)
			set_ui_target(*target);
	}

	// iterate over target nodes
	for (util::xml::data_node const *targetnode = parentnode->get_child("target"); targetnode; targetnode = targetnode->get_next_sibling("target"))
	{
		render_target *const target = target_by_index(targetnode->get_attribute_int("index", -1));
		if (target && !target->hidden())
			target->config_load(*targetnode);
	}

	// iterate over screen nodes
	for (util::xml::data_node const *screennode = parentnode->get_child("screen"); screennode; screennode = screennode->get_next_sibling("screen"))
	{
		int const index = screennode->get_attribute_int("index", -1);
		render_container *container = m_screen_container_list.find(index);

		// fetch current settings
		render_container::user_settings settings = container->get_user_settings();

		// fetch color controls
		settings.m_brightness = screennode->get_attribute_float("brightness", settings.m_brightness);
		settings.m_contrast = screennode->get_attribute_float("contrast", settings.m_contrast);
		settings.m_gamma = screennode->get_attribute_float("gamma", settings.m_gamma);

		// fetch positioning controls
		settings.m_xoffset = screennode->get_attribute_float("hoffset", settings.m_xoffset);
		settings.m_xscale = screennode->get_attribute_float("hstretch", settings.m_xscale);
		settings.m_yoffset = screennode->get_attribute_float("voffset", settings.m_yoffset);
		settings.m_yscale = screennode->get_attribute_float("vstretch", settings.m_yscale);

		// set the new values
		container->set_user_settings(settings);
	}
}


//-------------------------------------------------
//  config_save - save data to the configuration
//  file
//-------------------------------------------------

void render_manager::config_save(config_type cfg_type, util::xml::data_node *parentnode)
{
	// we only care about game files
	if (cfg_type != config_type::GAME)
		return;

	// write out the interface target
	if (m_ui_target->index() != 0)
	{
		// create a node for it
		util::xml::data_node *const uinode = parentnode->add_child("interface", nullptr);
		if (uinode != nullptr)
			uinode->set_attribute_int("target", m_ui_target->index());
	}

	// iterate over targets
	for (int targetnum = 0; ; ++targetnum)
	{
		// get this target and break when we fail
		render_target *target = target_by_index(targetnum);
		if (!target)
		{
			break;
		}
		else if (!target->hidden())
		{
			// create a node
			util::xml::data_node *const targetnode = parentnode->add_child("target", nullptr);
			if (targetnode && !target->config_save(*targetnode))
				targetnode->delete_node();
		}
	}

	// iterate over screen containers
	int scrnum = 0;
	for (render_container *container = m_screen_container_list.first(); container != nullptr; container = container->next(), scrnum++)
	{
		// create a node
		util::xml::data_node *const screennode = parentnode->add_child("screen", nullptr);
		if (screennode != nullptr)
		{
			bool changed = false;

			// output the basics
			screennode->set_attribute_int("index", scrnum);

			render_container::user_settings settings = container->get_user_settings();

			// output the color controls
			if (settings.m_brightness != machine().options().brightness())
			{
				screennode->set_attribute_float("brightness", settings.m_brightness);
				changed = true;
			}

			if (settings.m_contrast != machine().options().contrast())
			{
				screennode->set_attribute_float("contrast", settings.m_contrast);
				changed = true;
			}

			if (settings.m_gamma != machine().options().gamma())
			{
				screennode->set_attribute_float("gamma", settings.m_gamma);
				changed = true;
			}

			// output the positioning controls
			if (settings.m_xoffset != 0.0f)
			{
				screennode->set_attribute_float("hoffset", settings.m_xoffset);
				changed = true;
			}

			if (settings.m_xscale != 1.0f)
			{
				screennode->set_attribute_float("hstretch", settings.m_xscale);
				changed = true;
			}

			if (settings.m_yoffset != 0.0f)
			{
				screennode->set_attribute_float("voffset", settings.m_yoffset);
				changed = true;
			}

			if (settings.m_yscale != 1.0f)
			{
				screennode->set_attribute_float("vstretch", settings.m_yscale);
				changed = true;
			}

			// if nothing changed, kill the node
			if (!changed)
				screennode->delete_node();
		}
	}
}
