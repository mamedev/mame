// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/***************************************************************************

    screen_svg.cpp

    Core MAME SVG video output device.

***************************************************************************/

#include "emu.h"
#include "screen_svg.h"

#include "emuopts.h"
#include "nanosvg.h"
#include "render.h"
#include "video.h"

#include <cstdio>
#include <set>

// device type definition
DEFINE_DEVICE_TYPE(SCREEN_SVG, screen_svg_device, "screen_svg", "SVG Video Screen")

void screen_svg_device::svg_init()
{
	const std::unique_ptr<char []> s(new char[m_svg_region->bytes() + 1]);
	memcpy(s.get(), m_svg_region->base(), m_svg_region->bytes());
	s[m_svg_region->bytes()] = 0;
	m_image.reset(nsvgParse(s.get(), "px", 72));
	m_rasterizer.reset(nsvgCreateRasterizer());

	m_key_count = 0;

	for (NSVGshape *shape = m_image->shapes; shape; shape = shape->next)
		if(shape->title[0]) {
			const auto it = m_key_ids.find(shape->title);
			if(it != m_key_ids.end())
				m_keyed_shapes[it->second].push_back(shape);
			else {
				const int id = m_key_count++;
				m_keyed_shapes.resize(m_key_count);
				m_keyed_shapes[id].push_back(shape);
				m_key_ids[shape->title] = id;
			}
		}
	m_key_state.resize(m_key_count);
	std::fill(m_key_state.begin(),m_key_state.end(),false);
}

void screen_svg_device::render_state(std::vector<u32> &dest, const std::vector<bool> &state)
{
	for(int key = 0; key != m_key_count; key++) {
		if (state[key])
			for(auto s : m_keyed_shapes[key])
				s->flags |= NSVG_FLAGS_VISIBLE;
		else
			for(auto s : m_keyed_shapes[key])
				s->flags &= ~NSVG_FLAGS_VISIBLE;
	}

	nsvgRasterize(m_rasterizer.get(), m_image.get(), 0, 0, m_scale, (unsigned char *)&dest[0], m_sx, m_sy, m_sx*4);

	// Nanosvg generates non-premultiplied alpha, so remultiply by
	// alpha to "blend" against a black background.  Plus align the
	// channel order to what we do.

	u8 *image = (u8 *)&dest[0];
	for(unsigned int pixel=0; pixel != m_sy*m_sx; pixel++) {
		u8 r = image[0];
		u8 g = image[1];
		u8 b = image[2];
		u8 a = image[3];
		if(a != 0xff) {
			r = r*a/255;
			g = g*a/255;
			b = b*a/255;
		}
		u32 color = 0xff000000 | (r << 16) | (g << 8) | (b << 0);
		*(u32 *)image = color;
		image += 4;
	}
}

void screen_svg_device::blit(bitmap_rgb32 &bitmap, const cached_bitmap &src) const
{
	if(src.sy) {
		const u32 *s = &src.image[0];
		for(int y=0; y<src.sy; y++) {
			u32 *d = &bitmap.pix(y + src.y, src.x);
			for(int x=0; x<src.sx; x++, d++) {
				const u32 c = *s++;
				if(c)
					*d = c;
			}
		}
	}
}

void screen_svg_device::output_notifier(void *param, osd::output_item const &output, s32 seconds, s64 attoseconds)
{
	reinterpret_cast<screen_svg_device *>(param)->output_change(output);
}

void screen_svg_device::output_change(const osd::output_item &output)
{
	// for now, use the unqualified name for backwards compatibility
	// TODO: migrate to using qualified output names
	const auto l = m_key_ids.find(output.name());
	if (l == m_key_ids.end())
		return;
	m_key_state[l->second] = output.value();
}

void screen_svg_device::compute_initial_bboxes(std::vector<bbox> &bboxes)
{
	bboxes.resize(m_key_count);
	for(int key = 0; key != m_key_count; key++) {
		int x0, y0, x1, y1;
		x0 = y0 = x1 = y1 = -1;
		for(auto s : m_keyed_shapes[key]) {
			int xx0 = int(floor(s->bounds[0]*m_scale));
			int yy0 = int(floor(s->bounds[1]*m_scale));
			int xx1 = int(ceil (s->bounds[2]*m_scale)) + 1;
			int yy1 = int(ceil (s->bounds[3]*m_scale)) + 1;
			if(xx0 < 0)
				xx0 = 0;
			if(xx0 >= m_sx)
				xx0 = m_sx - 1;
			if(xx1 < 0)
				xx1 = 0;
			if(xx1 >= m_sx)
				xx1 = m_sx - 1;
			if(yy0 < 0)
				yy0 = 0;
			if(yy0 >= m_sy)
				yy0 = m_sy - 1;
			if(yy1 < 0)
				yy1 = 0;
			if(yy1 >= m_sy)
				yy1 = m_sy - 1;

			if(x0 == -1) {
				x0 = xx0;
				y0 = yy0;
				x1 = xx1;
				y1 = yy1;
			} else {
				if(xx0 < x0)
					x0 = xx0;
				if(yy0 < y0)
					y0 = yy0;
				if(xx1 > x1)
					x1 = xx1;
				if(yy1 > y1)
					y1 = yy1;
			}
		}
		bboxes[key].x0 = x0;
		bboxes[key].y0 = y0;
		bboxes[key].x1 = x1;
		bboxes[key].y1 = y1;
	}
}

void screen_svg_device::compute_diff_image(const std::vector<u32> &rend, const bbox &bb, cached_bitmap &dest) const
{
	int x0, y0, x1, y1;
	x0 = y0 = x1 = y1 = -1;
	for(int y = bb.y0; y != bb.y1; y++) {
		const u32 *src1 = &m_background[bb.x0 + y * m_sx];
		const u32 *src2 = &rend[bb.x0 + y * m_sx];
		for(int x = bb.x0; x != bb.x1; x++) {
			if(*src1 != *src2) {
				if(x0 == -1) {
					x0 = x1 = x;
					y0 = y1 = y;
				} else {
					if(x < x0)
						x0 = x;
					if(y < y0)
						y0 = y;
					if(x > x1)
						x1 = x;
					if(y > y1)
						y1 = y;
				}
			}
			src1++;
			src2++;
		}
	}
	if(x0 == -1) {
		dest.x = dest.y = dest.sx = dest.sy = 0;
		return;
	}

	dest.x = x0;
	dest.y = y0;
	dest.sx = x1+1-x0;
	dest.sy = y1+1-y0;
	dest.image.resize(dest.sx * dest.sy);
	u32 *dst = &dest.image[0];
	for(int y = 0; y != dest.sy; y++) {
		const u32 *src1 = &m_background[dest.x + (y + dest.y) * m_sx];
		const u32 *src2 = &rend[dest.x + (y + dest.y) * m_sx];
		for(int x = 0; x != dest.sx; x++) {
			if(*src1 != *src2)
				*dst = *src2;
			else
				*dst = 0x00000000;
			src1++;
			src2++;
			dst++;
		}
	}
}

bool screen_svg_device::compute_mask_intersection_bbox(int key1, int key2, bbox &bb) const
{
	const cached_bitmap &c1 = m_cache[key1];
	const cached_bitmap &c2 = m_cache[key2];
	if(c1.x >= c2.x + c2.sx ||
		c1.x + c1.sx <= c2.x ||
		c1.y >= c2.y + c2.sy ||
		c1.y + c1.sy <= c2.y)
		return false;
	int cx0 = c1.x > c2.x ? c1.x : c2.x;
	int cy0 = c1.y > c2.y ? c1.y : c2.y;
	int cx1 = c1.x + c1.sx < c2.x + c2.sx ? c1.x + c1.sx : c2.x + c2.sx;
	int cy1 = c1.y + c1.sy < c2.y + c2.sy ? c1.y + c1.sy : c2.y + c2.sy;

	int x0, y0, x1, y1;
	x0 = y0 = x1 = y1 = -1;

	for(int y = cy0; y < cy1; y++) {
		const u32 *src1 = &c1.image[(cx0 - c1.x) + c1.sx * (y - c1.y)];
		const u32 *src2 = &c2.image[(cx0 - c2.x) + c2.sx * (y - c2.y)];
		for(int x = cx0; x < cx1; x++) {
			if(*src1 && *src2 && *src1 != *src2) {
				if(x0 == -1) {
					x0 = x1 = x;
					y0 = y1 = y;
				} else {
					if(x < x0)
						x0 = x;
					if(y < y0)
						y0 = y;
					if(x > x1)
						x1 = x;
					if(y > y1)
						y1 = y;
				}
			}
			src1++;
			src2++;
		}
	}
	if(x0 == -1)
		return false;
	bb.x0 = x0;
	bb.x1 = x1;
	bb.y0 = y0;
	bb.y1 = y1;
	return true;
}

void screen_svg_device::compute_dual_diff_image(const std::vector<u32> &rend, const bbox &bb, const cached_bitmap &src1, const cached_bitmap &src2, cached_bitmap &dest) const
{
	dest.x = bb.x0;
	dest.y = bb.y0;
	dest.sx = bb.x1 - bb.x0 + 1;
	dest.sy = bb.y1 - bb.y0 + 1;
	dest.image.resize(dest.sx*dest.sy);
	for(int y = 0; y != dest.sy; y++) {
		const u32 *psrc1 = &src1.image[(dest.x - src1.x) + src1.sx * (y + dest.y - src1.y)];
		const u32 *psrc2 = &src2.image[(dest.x - src2.x) + src2.sx * (y + dest.y - src2.y)];
		const u32 *psrcr = &rend      [ dest.x           +    m_sx * (y + dest.y         )];
		u32       *pdest = &dest.image[                    dest.sx *  y                   ];
		for(int x = 0; x != dest.sx; x++) {
			if(*psrc1 && *psrc2 && *psrc1 != *psrc2)
				*pdest = *psrcr;
			psrc1++;
			psrc2++;
			psrcr++;
			pdest++;
		}
	}
}

void screen_svg_device::rebuild_cache()
{
	m_cache.clear();
	m_background.resize(m_sx * m_sy);
	std::vector<u32> rend(m_sx * m_sy);

	// Render the background, e.g. with everything off
	std::vector<bool> state(m_key_count);
	for(int key=0; key != m_key_count; key++)
		state[key] = false;

	render_state(m_background, state);

	// Render each individual element independently.  Try to reduce
	// the actual number of render passes with a greedy algorithm
	// using the element bounding boxes.
	std::vector<bbox> bboxes;
	compute_initial_bboxes(bboxes);

	m_cache.resize(m_key_count);
	std::set<int> to_do;
	for(int key=0; key != m_key_count; key++)
		to_do.insert(key);

	while(!to_do.empty()) {
		std::list<int> doing;
		for(int key : to_do) {
			for(int okey : doing) {
				// The bounding boxes include x1/y1, so the comparisons must be strict
				if(!(bboxes[key].x0 > bboxes[okey].x1 ||
						bboxes[key].x1 < bboxes[okey].x0 ||
						bboxes[key].y0 > bboxes[okey].y1 ||
						bboxes[key].y1 < bboxes[okey].y0))
					goto conflict;
			}
			doing.push_back(key);
		conflict:
			;
		}
		for(int key : doing)
			state[key] = true;
		render_state(rend, state);
		for(int key : doing) {
			state[key] = false;
			to_do.erase(key);
		}
		for(int key : doing)
			compute_diff_image(rend, bboxes[key], m_cache[key]);
	}

	// Then it's time to pick up the interactions.
	int spos = 0;
	int epos = m_key_count;
	std::vector<std::list<int> > keys(m_key_count);
	std::vector<int> previous;
	for(int key = 0; key != m_key_count; key++)
		keys[key].push_back(key);
	int ckey = m_key_count;
	while(spos != epos) {
		for(int key = spos; key < epos-1; key++) {
			for(int key2 = keys[key].back()+1; key2 < m_key_count; key2++) {
				bbox bb;
				if(compute_mask_intersection_bbox(key, key2, bb)) {
					previous.resize(ckey+1);
					previous[ckey] = key;
					m_cache[key].pairs.push_back(paired_entry(key2, ckey));
					keys.push_back(keys[key]);
					keys.back().push_back(key2);
					bboxes.push_back(bb);
					ckey++;
				}
			}
		}
		m_cache.resize(ckey);
		std::set<int> to_do;
		for(int key = epos; key != ckey; key++)
			to_do.insert(key);

		while(!to_do.empty()) {
			std::list<int> doing;
			for(int key : to_do) {
				for(int okey : doing) {
					// The bounding boxes include x1/y1, so the comparisons must be strict
					if(!(bboxes[key].x0 > bboxes[okey].x1 ||
							bboxes[key].x1 < bboxes[okey].x0 ||
							bboxes[key].y0 > bboxes[okey].y1 ||
							bboxes[key].y1 < bboxes[okey].y0))
						goto conflict2;
				}
				doing.push_back(key);
			conflict2:
				;
			}
			for(int key : doing)
				for(int akey : keys[key])
					state[akey] = true;

			render_state(rend, state);
			for(int key : doing) {
				for(int akey : keys[key])
					state[akey] = false;
				to_do.erase(key);
			}
			for(int key : doing)
				compute_dual_diff_image(rend, bboxes[key], m_cache[previous[key]], m_cache[keys[key].back()], m_cache[key]);
		}
		spos = epos;
		epos = ckey;
	}
}

//**************************************************************************
//  SCREEN DEVICE
//**************************************************************************

//-------------------------------------------------
//  screen_svg_device - constructor
//-------------------------------------------------

screen_svg_device::screen_svg_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, SCREEN_SVG, tag, owner, clock)
	, device_video_output_interface(mconfig, *this)
	, m_svg_region(*this, DEVICE_SELF)
	, m_screen_svg_update(*this)
	, m_curbitmap(0)
	, m_curtexture(0)
	, m_render_width(1000)
	, m_frame_period(attotime::from_hz(DEFAULT_FRAME_RATE))
	, m_vblank(*this)
{
	memset(m_texture, 0, sizeof(m_texture));
}


//-------------------------------------------------
//  ~screen_svg_device - destructor
//-------------------------------------------------

screen_svg_device::~screen_svg_device()
{
}

//-------------------------------------------------
//  device_validity_check - verify device
//  configuration
//-------------------------------------------------

void screen_svg_device::device_validity_check(validity_checker &valid) const
{
	// sanity check dimensions
	if (m_render_width <= 0)
		osd_printf_error("Invalid rendering width\n");

	// check for invalid frame rate
	if (m_frame_period.is_never() || m_frame_period >= attotime::from_hz(1))
		osd_printf_error("Invalid (under 1Hz) refresh rate\n");
}


//-------------------------------------------------
//  physical_aspect - determine the physical
//  aspect ratio to be used for rendering
//-------------------------------------------------

std::pair<unsigned, unsigned> screen_svg_device::physical_aspect() const
{
	std::pair<unsigned, unsigned> phys_aspect;
	phys_aspect.first = m_render_width;
	phys_aspect.second = m_render_height;

	// always keep this in reduced form
	util::reduce_fraction(phys_aspect.first, phys_aspect.second);

	return phys_aspect;
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void screen_svg_device::device_start()
{
	m_screen_svg_update.resolve();

	svg_init();

	machine().output().add_global_notifier(screen_svg_device::output_notifier, this);

	// We don't yet have a way for the rendering to tell us the optimal bitmap size...
	m_sx = m_render_width;
	m_sy = m_render_height;
	m_scale = m_render_width / m_image->width;

	for (auto & elem : m_bitmap)
		elem.resize(m_sx, m_sy);

	rebuild_cache();

	// allocate raw textures
	m_texture[0] = machine().render().texture_alloc();
	m_texture[0]->set_id(u64(m_unique_id) << 57);
	m_texture[1] = machine().render().texture_alloc();
	m_texture[1]->set_id((u64(m_unique_id) << 57) | 1);

	// register items for saving
	save_item(NAME(m_frame_period));

	// allocate and start the vblank timer
	m_vblank_timer = timer_alloc(FUNC(screen_svg_device::vblank_timer_callback), this);
	m_vblank_timer->adjust(m_frame_period, 0, m_frame_period);

	osd_printf_verbose("Parsed SVG '%s', aspect ratio %f\n", tag(), (m_image->height == 0.0f) ? 0 : m_image->width / m_image->height);
}


//-------------------------------------------------
//  vblank_timer_callback - periodically fires to
//  drive update_if_primary
//-------------------------------------------------

void screen_svg_device::vblank_timer_callback(s32 param)
{
	update_if_primary();

	// call the vblank callback
	m_vblank(1);
	m_vblank(0);
}

//-------------------------------------------------
//  override_frame_period - forcibly change the
//  refresh rate (from the sliders)
//-------------------------------------------------

void screen_svg_device::override_frame_period(attotime period)
{
	m_frame_period = period;
	m_vblank_timer->adjust(m_frame_period, 0, m_frame_period);
}


//-------------------------------------------------
//  device_stop - clean up before the machine goes
//  away
//-------------------------------------------------

void screen_svg_device::device_stop()
{
	machine().render().texture_free(m_texture[0]);
	machine().render().texture_free(m_texture[1]);
}

//-------------------------------------------------
//  video_output_update - set up the quad for this
//  screen
//-------------------------------------------------

bool screen_svg_device::video_output_update()
{
	if (!machine().video().skip_this_frame())
	{
		if (!m_screen_svg_update.isnull())
			m_screen_svg_update(*this);

		auto &bitmap = m_bitmap[m_curbitmap];
		for(unsigned int y = 0; y < m_sy; y++)
			memcpy(bitmap.raw_pixptr(y, 0), &m_background[y * m_sx], m_sx * 4);

		std::list<int> to_draw;
		for(int key = 0; key != m_key_count; key++)
			if(m_key_state[key])
				to_draw.push_back(key);
		while(!to_draw.empty()) {
			int key = to_draw.front();
			to_draw.pop_front();
			blit(bitmap, m_cache[key]);
			for(auto p : m_cache[key].pairs) {
				if(m_key_state[p.key])
					to_draw.push_back(p.cache_entry);
			}
		}

		m_texture[m_curbitmap]->set_bitmap(m_bitmap[m_curbitmap], rectangle(0, m_sx-1, 0, m_sy-1), TEXFORMAT_RGB32);
		m_curtexture = m_curbitmap;
		m_curbitmap = 1 - m_curbitmap;
	}

	// create an empty container with a single quad
	m_container->empty();
	m_container->add_quad(0.0f, 0.0f, 1.0f, 1.0f, rgb_t::white(), m_texture[m_curtexture], PRIMFLAG_BLENDMODE(BLENDMODE_NONE) | PRIMFLAG_SCREENTEX(1));

	return true;
}

rectangle screen_svg_device::visible_area() const
{
	return rectangle(0, m_render_width-1, 0, m_render_height-1);
}
