// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    screen.c

    Core MAME screen device.

***************************************************************************/

#include "emu.h"
#include "emuopts.h"
#include "png.h"
#include "rendutil.h"

#include <nanosvg/src/nanosvg.h>
#include <nanosvg/src/nanosvgrast.h>
#include <set>

//**************************************************************************
//  DEBUGGING
//**************************************************************************

#define VERBOSE                     (0)
#define LOG_PARTIAL_UPDATES(x)      do { if (VERBOSE) logerror x; } while (0)



//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

// device type definition
const device_type SCREEN = &device_creator<screen_device>;

const attotime screen_device::DEFAULT_FRAME_PERIOD(attotime::from_hz(DEFAULT_FRAME_RATE));

UINT32 screen_device::m_id_counter = 0;

class screen_device_svg_renderer {
public:
	screen_device_svg_renderer(memory_region *region);
	~screen_device_svg_renderer();

	int width() const;
	int height() const;

	int render(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	static void output_notifier(const char *outname, INT32 value, void *param);

private:
	struct paired_entry {
		int key;
		int cache_entry;
		paired_entry(int k, int c) { key = k; cache_entry = c; }
	};

	struct cached_bitmap {
		int x, y, sx, sy;
		std::vector<UINT32> image;
		std::vector<paired_entry> pairs;
	};

	struct bbox {
		int x0, y0, x1, y1;
	};

	NSVGimage *m_image;
	NSVGrasterizer *m_rasterizer;
	std::vector<bool> m_key_state;
	std::vector<std::list<NSVGshape *>> m_keyed_shapes;
	std::unordered_map<std::string, int> m_key_ids;
	int m_key_count;

	int m_sx, m_sy;
	double m_scale;
	std::vector<UINT32> m_background;

	std::vector<cached_bitmap> m_cache;

	void output_change(const char *outname, INT32 value);
	void render_state(std::vector<UINT32> &dest, const std::vector<bool> &state);
	void compute_initial_bboxes(std::vector<bbox> &bboxes);
	bool compute_mask_intersection_bbox(int key1, int key2, bbox &bb) const;
	void compute_diff_image(const std::vector<UINT32> &rend, const bbox &bb, cached_bitmap &dest) const;
	void compute_dual_diff_image(const std::vector<UINT32> &rend, const bbox &bb, const cached_bitmap &src1, const cached_bitmap &src2, cached_bitmap &dest) const;
	void rebuild_cache();
	void blit(bitmap_rgb32 &bitmap, const cached_bitmap &src) const;
};

screen_device_svg_renderer::screen_device_svg_renderer(memory_region *region)
{
	char *s = new char[region->bytes()+1];
	memcpy(s, region->base(), region->bytes());
	s[region->bytes()] = 0;
	m_image = nsvgParse(s, "px", 72);
	delete[] s;
	m_rasterizer = nsvgCreateRasterizer();

	m_key_count = 0;

	for (NSVGshape *shape = m_image->shapes; shape != nullptr; shape = shape->next)
		if(shape->title[0]) {
			auto it = m_key_ids.find(shape->title);
			if(it != m_key_ids.end())
				m_keyed_shapes[it->second].push_back(shape);
			else {
				int id = m_key_count;
				m_key_count++;
				m_keyed_shapes.resize(m_key_count);
				m_keyed_shapes[id].push_back(shape);
				m_key_ids[shape->title] = id;
			}
		}
	m_key_state.resize(m_key_count);
	// Don't memset a vector<bool>, they're special, and not in a good way
	for(int i=0; i != m_key_count; i++)
		m_key_state[i] = false;

	m_sx = m_sy = 0;
	m_scale = 1.0;

#if 0
	double ar = m_image->width / m_image->height;
	int w,h;
	if (ar < (16.0/9.0))
	{
		h = 1080;
		w = (h * ar) + 0.5;
	}
	else
	{
		w = 1920;
		h = (w / ar) + 0.5;
	}

	printf("\n\nMCFG_SCREEN_SIZE(%d, %d)\nMCFG_SCREEN_VISIBLE_AREA(0, %d-1, 0, %d-1)\n", w, h, w, h);
#endif
}

screen_device_svg_renderer::~screen_device_svg_renderer()
{
	nsvgDeleteRasterizer(m_rasterizer);
	nsvgDelete(m_image);
}

int screen_device_svg_renderer::width() const
{
	return int(m_image->width + 0.5);
}

int screen_device_svg_renderer::height() const
{
	return int(m_image->height + 0.5);
}

void screen_device_svg_renderer::render_state(std::vector<UINT32> &dest, const std::vector<bool> &state)
{
	for(int key = 0; key != m_key_count; key++) {
		if (state[key])
			for(auto s : m_keyed_shapes[key])
				s->flags |= NSVG_FLAGS_VISIBLE;
		else
			for(auto s : m_keyed_shapes[key])
				s->flags &= ~NSVG_FLAGS_VISIBLE;
	}

	nsvgRasterize(m_rasterizer, m_image, 0, 0, m_scale, (unsigned char *)&dest[0], m_sx, m_sy, m_sx*4);

	// Nanosvg generates non-premultiplied alpha, so remultiply by
	// alpha to "blend" against a black background.  Plus align the
	// channel order to what we do.

	UINT8 *image = (UINT8 *)&dest[0];
	for(unsigned int pixel=0; pixel != m_sy*m_sx; pixel++) {
		UINT8 r = image[0];
		UINT8 g = image[1];
		UINT8 b = image[2];
		UINT8 a = image[3];
		if(a != 0xff) {
			r = r*a/255;
			g = g*a/255;
			b = b*a/255;
		}
		UINT32 color = 0xff000000 | (r << 16) | (g << 8) | (b << 0);
		*(UINT32 *)image = color;
		image += 4;
	}
}

void screen_device_svg_renderer::blit(bitmap_rgb32 &bitmap, const cached_bitmap &src) const
{
	const UINT32 *s = &src.image[0];
	for(int y=0; y<src.sy; y++) {
		UINT32 *d = &bitmap.pix(y + src.y, src.x);
		for(int x=0; x<src.sx; x++) {
			UINT32 c = *s++;
			if(c)
				*d = c;
			d++;
		}
	}
}

int screen_device_svg_renderer::render(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	int nsx = bitmap.width();
	int nsy = bitmap.height();

	if(nsx != m_sx || nsy != m_sy) {
		m_sx = nsx;
		m_sy = nsy;
		double sx = double(m_sx)/m_image->width;
		double sy = double(m_sy)/m_image->height;
		m_scale = sx > sy ? sy : sx;
		m_background.resize(m_sx * m_sy);
		rebuild_cache();
	}

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

	return 0;
}

void screen_device_svg_renderer::output_notifier(const char *outname, INT32 value, void *param)
{
	static_cast<screen_device_svg_renderer *>(param)->output_change(outname, value);
}

void screen_device_svg_renderer::output_change(const char *outname, INT32 value)
{
	auto l = m_key_ids.find(outname);
	if (l == m_key_ids.end())
		return;
	m_key_state[l->second] = value;
}

void screen_device_svg_renderer::compute_initial_bboxes(std::vector<bbox> &bboxes)
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

void screen_device_svg_renderer::compute_diff_image(const std::vector<UINT32> &rend, const bbox &bb, cached_bitmap &dest) const
{
	int x0, y0, x1, y1;
	x0 = y0 = x1 = y1 = -1;
	for(int y = bb.y0; y != bb.y1; y++) {
		const UINT32 *src1 = &m_background[bb.x0 + y * m_sx];
		const UINT32 *src2 = &rend[bb.x0 + y * m_sx];
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
	UINT32 *dst = &dest.image[0];
	for(int y = 0; y != dest.sy; y++) {
		const UINT32 *src1 = &m_background[dest.x + (y + dest.y) * m_sx];
		const UINT32 *src2 = &rend[dest.x + (y + dest.y) * m_sx];
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

bool screen_device_svg_renderer::compute_mask_intersection_bbox(int key1, int key2, bbox &bb) const
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
		const UINT32 *src1 = &c1.image[(cx0 - c1.x) + c1.sx * (y - c1.y)];
		const UINT32 *src2 = &c2.image[(cx0 - c2.x) + c2.sx * (y - c2.y)];
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

void screen_device_svg_renderer::compute_dual_diff_image(const std::vector<UINT32> &rend, const bbox &bb, const cached_bitmap &src1, const cached_bitmap &src2, cached_bitmap &dest) const
{
	dest.x = bb.x0;
	dest.y = bb.y0;
	dest.sx = bb.x1 - bb.x0 + 1;
	dest.sy = bb.y1 - bb.y0 + 1;
	dest.image.resize(dest.sx*dest.sy);
	for(int y = 0; y != dest.sy; y++) {
		const UINT32 *psrc1 = &src1.image[(dest.x - src1.x) + src1.sx * (y + dest.y - src1.y)];
		const UINT32 *psrc2 = &src2.image[(dest.x - src2.x) + src2.sx * (y + dest.y - src2.y)];
		const UINT32 *psrcr = &rend      [ dest.x           +    m_sx * (y + dest.y         )];
		UINT32       *pdest = &dest.image[                    dest.sx *  y                   ];
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

void screen_device_svg_renderer::rebuild_cache()
{
	m_cache.clear();
	std::vector<UINT32> rend(m_sx*m_sy);

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
//  screen_device - constructor
//-------------------------------------------------

screen_device::screen_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, SCREEN, "Video Screen", tag, owner, clock, "screen", __FILE__),
		m_type(SCREEN_TYPE_RASTER),
		m_oldstyle_vblank_supplied(false),
		m_refresh(0),
		m_vblank(0),
		m_xoffset(0.0f),
		m_yoffset(0.0f),
		m_xscale(1.0f),
		m_yscale(1.0f),
		m_palette(*this),
		m_video_attributes(0),
		m_svg_region(nullptr),
		m_container(nullptr),
		m_width(100),
		m_height(100),
		m_visarea(0, 99, 0, 99),
		m_texformat(),
		m_curbitmap(0),
		m_curtexture(0),
		m_changed(true),
		m_last_partial_scan(0),
		m_partial_scan_hpos(0),
		m_color(rgb_t(0xff, 0xff, 0xff, 0xff)),
		m_brightness(0xff),
		m_frame_period(DEFAULT_FRAME_PERIOD.as_attoseconds()),
		m_scantime(1),
		m_pixeltime(1),
		m_vblank_period(0),
		m_vblank_start_time(attotime::zero),
		m_vblank_end_time(attotime::zero),
		m_vblank_begin_timer(nullptr),
		m_vblank_end_timer(nullptr),
		m_scanline0_timer(nullptr),
		m_scanline_timer(nullptr),
		m_frame_number(0),
		m_partial_updates_this_frame(0)
{
	m_unique_id = m_id_counter;
	m_id_counter++;
	memset(m_texture, 0, sizeof(m_texture));
}


//-------------------------------------------------
//  ~screen_device - destructor
//-------------------------------------------------

screen_device::~screen_device()
{
}


//-------------------------------------------------
//  static_set_type - configuration helper
//  to set the screen type
//-------------------------------------------------

void screen_device::static_set_type(device_t &device, screen_type_enum type)
{
	downcast<screen_device &>(device).m_type = type;
}


void screen_device::static_set_svg_region(device_t &device, const char *region)
{
	downcast<screen_device &>(device).m_svg_region = region;
}


//-------------------------------------------------
//  static_set_raw - configuration helper
//  to set the raw screen parameters
//-------------------------------------------------

void screen_device::static_set_raw(device_t &device, UINT32 pixclock, UINT16 htotal, UINT16 hbend, UINT16 hbstart, UINT16 vtotal, UINT16 vbend, UINT16 vbstart)
{
	screen_device &screen = downcast<screen_device &>(device);
	screen.m_clock = pixclock;
	screen.m_refresh = HZ_TO_ATTOSECONDS(pixclock) * htotal * vtotal;
	screen.m_vblank = screen.m_refresh / vtotal * (vtotal - (vbstart - vbend));
	screen.m_width = htotal;
	screen.m_height = vtotal;
	screen.m_visarea.set(hbend, hbstart - 1, vbend, vbstart - 1);
}


//-------------------------------------------------
//  static_set_refresh - configuration helper
//  to set the refresh rate
//-------------------------------------------------

void screen_device::static_set_refresh(device_t &device, attoseconds_t rate)
{
	downcast<screen_device &>(device).m_refresh = rate;
}


//-------------------------------------------------
//  static_set_vblank_time - configuration helper
//  to set the VBLANK duration
//-------------------------------------------------

void screen_device::static_set_vblank_time(device_t &device, attoseconds_t time)
{
	screen_device &screen = downcast<screen_device &>(device);
	screen.m_vblank = time;
	screen.m_oldstyle_vblank_supplied = true;
}


//-------------------------------------------------
//  static_set_size - configuration helper to set
//  the width/height of the screen
//-------------------------------------------------

void screen_device::static_set_size(device_t &device, UINT16 width, UINT16 height)
{
	screen_device &screen = downcast<screen_device &>(device);
	screen.m_width = width;
	screen.m_height = height;
}


//-------------------------------------------------
//  static_set_visarea - configuration helper to
//  set the visible area of the screen
//-------------------------------------------------

void screen_device::static_set_visarea(device_t &device, INT16 minx, INT16 maxx, INT16 miny, INT16 maxy)
{
	downcast<screen_device &>(device).m_visarea.set(minx, maxx, miny, maxy);
}


//-------------------------------------------------
//  static_set_default_position - configuration
//  helper to set the default position and scale
//  factors for the screen
//-------------------------------------------------

void screen_device::static_set_default_position(device_t &device, double xscale, double xoffs, double yscale, double yoffs)
{
	screen_device &screen = downcast<screen_device &>(device);
	screen.m_xscale = xscale;
	screen.m_xoffset = xoffs;
	screen.m_yscale = yscale;
	screen.m_yoffset = yoffs;
}


//-------------------------------------------------
//  static_set_screen_update - set the legacy(?)
//  screen update callback in the device
//  configuration
//-------------------------------------------------

void screen_device::static_set_screen_update(device_t &device, screen_update_ind16_delegate callback)
{
	screen_device &screen = downcast<screen_device &>(device);
	screen.m_screen_update_ind16 = callback;
	screen.m_screen_update_rgb32 = screen_update_rgb32_delegate();
}

void screen_device::static_set_screen_update(device_t &device, screen_update_rgb32_delegate callback)
{
	screen_device &screen = downcast<screen_device &>(device);
	screen.m_screen_update_ind16 = screen_update_ind16_delegate();
	screen.m_screen_update_rgb32 = callback;
}


//-------------------------------------------------
//  static_set_screen_vblank - set the screen
//  VBLANK callback in the device configuration
//-------------------------------------------------

void screen_device::static_set_screen_vblank(device_t &device, screen_vblank_delegate callback)
{
	downcast<screen_device &>(device).m_screen_vblank = callback;
}


//-------------------------------------------------
//  static_set_palette - set the screen palette
//  configuration
//-------------------------------------------------

void screen_device::static_set_palette(device_t &device, const char *tag)
{
	downcast<screen_device &>(device).m_palette.set_tag(tag);
}


//-------------------------------------------------
//  static_set_video_attributes - set the screen
//  video attributes
//-------------------------------------------------

void screen_device::static_set_video_attributes(device_t &device, UINT32 flags)
{
	screen_device &screen = downcast<screen_device &>(device);
	screen.m_video_attributes = flags;
}


//-------------------------------------------------
//  static_set_color - set the screen global color
//-------------------------------------------------

void screen_device::static_set_color(device_t &device, rgb_t color)
{
	screen_device &screen = downcast<screen_device &>(device);
	screen.m_color = color;
}


//-------------------------------------------------
//  device_validity_check - verify device
//  configuration
//-------------------------------------------------

void screen_device::device_validity_check(validity_checker &valid) const
{
	// sanity check dimensions
	if (m_width <= 0 || m_height <= 0)
		osd_printf_error("Invalid display dimensions\n");

	// sanity check display area
	if (m_type != SCREEN_TYPE_VECTOR && m_type != SCREEN_TYPE_SVG)
	{
		if (m_visarea.empty() || m_visarea.max_x >= m_width || m_visarea.max_y >= m_height)
			osd_printf_error("Invalid display area\n");

		// sanity check screen formats
		if (m_screen_update_ind16.isnull() && m_screen_update_rgb32.isnull())
			osd_printf_error("Missing SCREEN_UPDATE function\n");
	}

	// check for svg region
	if (m_type == SCREEN_TYPE_SVG && !m_svg_region)
		osd_printf_error("Missing SVG region information\n");

	// check for zero frame rate
	if (m_refresh == 0)
		osd_printf_error("Invalid (zero) refresh rate\n");

	texture_format texformat = !m_screen_update_ind16.isnull() ? TEXFORMAT_PALETTE16 : TEXFORMAT_RGB32;
	if (m_palette == nullptr && texformat == TEXFORMAT_PALETTE16)
		osd_printf_error("Screen does not have palette defined\n");
	if (m_palette != nullptr && texformat == TEXFORMAT_RGB32)
		osd_printf_warning("Screen does not need palette defined\n");
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void screen_device::device_start()
{
	if (m_type == SCREEN_TYPE_SVG)
	{
		memory_region *reg = owner()->memregion(m_svg_region);
		if (!reg)
			fatalerror("SVG region \"%s\" does not exist\n", m_svg_region);
		m_svg = std::make_unique<screen_device_svg_renderer>(reg);
		machine().output().set_notifier(nullptr, screen_device_svg_renderer::output_notifier, m_svg.get());

		if (0)
		{
			// The osd picks up the size before start is called, so that's useless
			m_width = m_svg->width();
			m_height = m_svg->height();
			m_visarea.set(0, m_width-1, 0, m_height-1);
		}
	}

	// bind our handlers
	m_screen_update_ind16.bind_relative_to(*owner());
	m_screen_update_rgb32.bind_relative_to(*owner());
	m_screen_vblank.bind_relative_to(*owner());

	// if we have a palette and it's not started, wait for it
	if (m_palette != nullptr && !m_palette->started())
		throw device_missing_dependencies();

	// configure bitmap formats and allocate screen bitmaps
	// svg is RGB32 too, and doesn't have any update method
	texture_format texformat = !m_screen_update_ind16.isnull() ? TEXFORMAT_PALETTE16 : TEXFORMAT_RGB32;

	for (auto & elem : m_bitmap)
	{
		elem.set_format(format(), texformat);
		register_screen_bitmap(elem);
	}
	register_screen_bitmap(m_priority);

	// allocate raw textures
	m_texture[0] = machine().render().texture_alloc();
	m_texture[0]->set_osd_data((UINT64)((m_unique_id << 1) | 0));
	m_texture[1] = machine().render().texture_alloc();
	m_texture[1]->set_osd_data((UINT64)((m_unique_id << 1) | 1));

	// configure the default cliparea
	render_container::user_settings settings;
	m_container->get_user_settings(settings);
	settings.m_xoffset = m_xoffset;
	settings.m_yoffset = m_yoffset;
	settings.m_xscale = m_xscale;
	settings.m_yscale = m_yscale;
	m_container->set_user_settings(settings);

	// allocate the VBLANK timers
	m_vblank_begin_timer = timer_alloc(TID_VBLANK_START);
	m_vblank_end_timer = timer_alloc(TID_VBLANK_END);

	// allocate a timer to reset partial updates
	m_scanline0_timer = timer_alloc(TID_SCANLINE0);

	// allocate a timer to generate per-scanline updates
	if ((m_video_attributes & VIDEO_UPDATE_SCANLINE) != 0)
		m_scanline_timer = timer_alloc(TID_SCANLINE);

	// configure the screen with the default parameters
	configure(m_width, m_height, m_visarea, m_refresh);

	// reset VBLANK timing
	m_vblank_start_time = attotime::zero;
	m_vblank_end_time = attotime(0, m_vblank_period);

	// start the timer to generate per-scanline updates
	if ((m_video_attributes & VIDEO_UPDATE_SCANLINE) != 0)
		m_scanline_timer->adjust(time_until_pos(0));

	// create burn-in bitmap
	if (machine().options().burnin())
	{
		int width, height;
		if (sscanf(machine().options().snap_size(), "%dx%d", &width, &height) != 2 || width == 0 || height == 0)
			width = height = 300;
		m_burnin.allocate(width, height);
		m_burnin.fill(0);
	}

	// load the effect overlay
	const char *overname = machine().options().effect();
	if (overname != nullptr && strcmp(overname, "none") != 0)
		load_effect_overlay(overname);

	// register items for saving
	save_item(NAME(m_width));
	save_item(NAME(m_height));
	save_item(NAME(m_visarea.min_x));
	save_item(NAME(m_visarea.min_y));
	save_item(NAME(m_visarea.max_x));
	save_item(NAME(m_visarea.max_y));
	save_item(NAME(m_last_partial_scan));
	save_item(NAME(m_frame_period));
	save_item(NAME(m_brightness));
	save_item(NAME(m_scantime));
	save_item(NAME(m_pixeltime));
	save_item(NAME(m_vblank_period));
	save_item(NAME(m_vblank_start_time));
	save_item(NAME(m_vblank_end_time));
	save_item(NAME(m_frame_number));
	if (m_oldstyle_vblank_supplied)
		logerror("%s: Deprecated legacy Old Style screen configured (MCFG_SCREEN_VBLANK_TIME), please use MCFG_SCREEN_RAW_PARAMS instead.\n",this->tag());
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void screen_device::device_reset()
{
	// reset brightness to default
	m_brightness = 0xff;
}


//-------------------------------------------------
//  device_stop - clean up before the machine goes
//  away
//-------------------------------------------------

void screen_device::device_stop()
{
	machine().render().texture_free(m_texture[0]);
	machine().render().texture_free(m_texture[1]);
	if (m_burnin.valid())
		finalize_burnin();
}


//-------------------------------------------------
//  device_post_load - device-specific update
//  after a save state is loaded
//-------------------------------------------------

void screen_device::device_post_load()
{
	realloc_screen_bitmaps();
}


//-------------------------------------------------
//  device_timer - called whenever a device timer
//  fires
//-------------------------------------------------

void screen_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	switch (id)
	{
		// signal VBLANK start
		case TID_VBLANK_START:
			vblank_begin();
			break;

		// signal VBLANK end
		case TID_VBLANK_END:
			vblank_end();
			break;

		// first scanline
		case TID_SCANLINE0:
			reset_partial_updates();
			break;

		// subsequent scanlines when scanline updates are enabled
		case TID_SCANLINE:

			// force a partial update to the current scanline
			update_partial(param);

			// compute the next visible scanline
			param++;
			if (param > m_visarea.max_y)
				param = m_visarea.min_y;
			m_scanline_timer->adjust(time_until_pos(param), param);
			break;
	}
}


//-------------------------------------------------
//  configure - configure screen parameters
//-------------------------------------------------

void screen_device::configure(int width, int height, const rectangle &visarea, attoseconds_t frame_period)
{
	// validate arguments
	assert(width > 0);
	assert(height > 0);
	assert(visarea.min_x >= 0);
	assert(visarea.min_y >= 0);
//  assert(visarea.max_x < width);
//  assert(visarea.max_y < height);
	assert(m_type == SCREEN_TYPE_VECTOR || m_type == SCREEN_TYPE_SVG || visarea.min_x < width);
	assert(m_type == SCREEN_TYPE_VECTOR || m_type == SCREEN_TYPE_SVG || visarea.min_y < height);
	assert(frame_period > 0);

	// fill in the new parameters
	m_width = width;
	m_height = height;
	m_visarea = visarea;

	// reallocate bitmap if necessary
	realloc_screen_bitmaps();

	// compute timing parameters
	m_frame_period = frame_period;
	m_scantime = frame_period / height;
	m_pixeltime = frame_period / (height * width);

	// if an old style VBLANK_TIME was specified in the MACHINE_CONFIG,
	// use it; otherwise calculate the VBLANK period from the visible area
	if (m_oldstyle_vblank_supplied)
		m_vblank_period = m_vblank;
	else
		m_vblank_period = m_scantime * (height - visarea.height());

	// we are now fully configured with the new parameters
	// and can safely call time_until_pos(), etc.

	// if the frame period was reduced so that we are now past the end of the frame,
	// call the VBLANK start timer now; otherwise, adjust it for the future
	attoseconds_t delta = (machine().time() - m_vblank_start_time).as_attoseconds();
	if (delta >= m_frame_period)
		vblank_begin();
	else
		m_vblank_begin_timer->adjust(time_until_vblank_start());

	// if we are on scanline 0 already, call the scanline 0 timer
	// by hand now; otherwise, adjust it for the future
	if (vpos() == 0)
		reset_partial_updates();
	else
		m_scanline0_timer->adjust(time_until_pos(0));

	// adjust speed if necessary
	machine().video().update_refresh_speed();
}


//-------------------------------------------------
//  reset_origin - reset the timing such that the
//  given (x,y) occurs at the current time
//-------------------------------------------------

void screen_device::reset_origin(int beamy, int beamx)
{
	// compute the effective VBLANK start/end times
	attotime curtime = machine().time();
	m_vblank_end_time = curtime - attotime(0, beamy * m_scantime + beamx * m_pixeltime);
	m_vblank_start_time = m_vblank_end_time - attotime(0, m_vblank_period);

	// if we are resetting relative to (0,0) == VBLANK end, call the
	// scanline 0 timer by hand now; otherwise, adjust it for the future
	if (beamy == 0 && beamx == 0)
		reset_partial_updates();
	else
		m_scanline0_timer->adjust(time_until_pos(0));

	// if we are resetting relative to (visarea.max_y + 1, 0) == VBLANK start,
	// call the VBLANK start timer now; otherwise, adjust it for the future
	if (beamy == ((m_visarea.max_y + 1) % m_height) && beamx == 0)
		vblank_begin();
	else
		m_vblank_begin_timer->adjust(time_until_vblank_start());
}


//-------------------------------------------------
//  realloc_screen_bitmaps - reallocate bitmaps
//  and textures as necessary
//-------------------------------------------------

void screen_device::realloc_screen_bitmaps()
{
	// doesn't apply for vector games
	if (m_type == SCREEN_TYPE_VECTOR)
		return;

	// determine effective size to allocate
	INT32 effwidth = MAX(m_width, m_visarea.max_x + 1);
	INT32 effheight = MAX(m_height, m_visarea.max_y + 1);

	// reize all registered screen bitmaps
	for (auto_bitmap_item &item : m_auto_bitmap_list)
		item.m_bitmap.resize(effwidth, effheight);

	// re-set up textures
	if (m_palette != nullptr)
	{
		m_bitmap[0].set_palette(m_palette->palette());
		m_bitmap[1].set_palette(m_palette->palette());
	}
	m_texture[0]->set_bitmap(m_bitmap[0], m_visarea, m_bitmap[0].texformat());
	m_texture[1]->set_bitmap(m_bitmap[1], m_visarea, m_bitmap[1].texformat());
}


//-------------------------------------------------
//  set_visible_area - just set the visible area
//-------------------------------------------------

void screen_device::set_visible_area(int min_x, int max_x, int min_y, int max_y)
{
	rectangle visarea(min_x, max_x, min_y, max_y);
	assert(!visarea.empty());
	configure(m_width, m_height, visarea, m_frame_period);
}


//-------------------------------------------------
//  update_partial - perform a partial update from
//  the last scanline up to and including the
//  specified scanline
//-----------------------------------------------*/

bool screen_device::update_partial(int scanline)
{
	// validate arguments
	assert(scanline >= 0);

	LOG_PARTIAL_UPDATES(("Partial: update_partial(%s, %d): ", tag(), scanline));

	// these two checks only apply if we're allowed to skip frames
	if (!(m_video_attributes & VIDEO_ALWAYS_UPDATE))
	{
		// if skipping this frame, bail
		if (machine().video().skip_this_frame())
		{
			LOG_PARTIAL_UPDATES(("skipped due to frameskipping\n"));
			return FALSE;
		}

		// skip if this screen is not visible anywhere
		if (!machine().render().is_live(*this))
		{
			LOG_PARTIAL_UPDATES(("skipped because screen not live\n"));
			return FALSE;
		}
	}

	// skip if we already rendered this line
	if (scanline < m_last_partial_scan)
	{
		LOG_PARTIAL_UPDATES(("skipped because line was already rendered\n"));
		return false;
	}

	// set the range of scanlines to render
	rectangle clip = m_visarea;
	if (m_last_partial_scan > clip.min_y)
		clip.min_y = m_last_partial_scan;
	if (scanline < clip.max_y)
		clip.max_y = scanline;

	// skip if entirely outside of visible area
	if (clip.min_y > clip.max_y)
	{
		LOG_PARTIAL_UPDATES(("skipped because outside of visible area\n"));
		return false;
	}

	// otherwise, render
	LOG_PARTIAL_UPDATES(("updating %d-%d\n", clip.min_y, clip.max_y));
	g_profiler.start(PROFILER_VIDEO);

	UINT32 flags;
	if (m_type != SCREEN_TYPE_SVG)
	{
		screen_bitmap &curbitmap = m_bitmap[m_curbitmap];
		switch (curbitmap.format())
		{
			default:
			case BITMAP_FORMAT_IND16:   flags = m_screen_update_ind16(*this, curbitmap.as_ind16(), clip);   break;
			case BITMAP_FORMAT_RGB32:   flags = m_screen_update_rgb32(*this, curbitmap.as_rgb32(), clip);   break;
		}
	}
	else
	{
		flags = m_svg->render(*this, m_bitmap[m_curbitmap].as_rgb32(), clip);
	}

	m_partial_updates_this_frame++;
	g_profiler.stop();

	// if we modified the bitmap, we have to commit
	m_changed |= ~flags & UPDATE_HAS_NOT_CHANGED;

	// remember where we left off
	m_last_partial_scan = scanline + 1;
	return true;
}


//-------------------------------------------------
//  update_now - perform an update from the last
//  beam position up to the current beam position
//-------------------------------------------------

void screen_device::update_now()
{
	// these two checks only apply if we're allowed to skip frames
	if (!(m_video_attributes & VIDEO_ALWAYS_UPDATE))
	{
		// if skipping this frame, bail
		if (machine().video().skip_this_frame())
		{
			LOG_PARTIAL_UPDATES(("skipped due to frameskipping\n"));
			return;
		}

		// skip if this screen is not visible anywhere
		if (!machine().render().is_live(*this))
		{
			LOG_PARTIAL_UPDATES(("skipped because screen not live\n"));
			return;
		}
	}

	int current_vpos = vpos();
	int current_hpos = hpos();
	rectangle clip = m_visarea;

	LOG_PARTIAL_UPDATES(("update_now(): Y=%d, X=%d, last partial %d, partial hpos %d  (vis %d %d)\n", current_vpos, current_hpos, m_last_partial_scan, m_partial_scan_hpos, m_visarea.max_x, m_visarea.max_y));

	// start off by doing a partial update up to the line before us, in case that was necessary
	if (current_vpos > m_last_partial_scan)
	{
		// if the line before us was incomplete, we must do it in two pieces
		if (m_partial_scan_hpos > 0)
		{
			INT32 save_scan = m_partial_scan_hpos;
			update_partial(current_vpos - 2);
			m_partial_scan_hpos = save_scan;

			// now finish the previous partial scanline
			int scanline = current_vpos - 1;
			if (m_partial_scan_hpos > clip.min_x)
				clip.min_x = m_partial_scan_hpos;
			if (current_hpos < clip.max_x)
				clip.max_x = current_hpos;
			if (m_last_partial_scan > clip.min_y)
				clip.min_y = m_last_partial_scan;
			if (scanline < clip.max_y)
				clip.max_y = scanline;

			// if there's something to draw, do it
			if ((clip.min_x <= clip.max_x) && (clip.min_y <= clip.max_y))
			{
				g_profiler.start(PROFILER_VIDEO);

				screen_bitmap &curbitmap = m_bitmap[m_curbitmap];
				switch (curbitmap.format())
				{
					default:
					case BITMAP_FORMAT_IND16: m_screen_update_ind16(*this, curbitmap.as_ind16(), clip);   break;
					case BITMAP_FORMAT_RGB32: m_screen_update_rgb32(*this, curbitmap.as_rgb32(), clip);   break;
				}

				m_partial_updates_this_frame++;
				g_profiler.stop();
				m_partial_scan_hpos = 0;
				m_last_partial_scan = current_vpos + 1;
			}
		}
		else
		{
			update_partial(current_vpos - 1);
		}
	}

	// now draw this partial scanline
	clip = m_visarea;

	if (m_partial_scan_hpos > clip.min_x)
		clip.min_x = m_partial_scan_hpos;
	if (current_hpos < clip.max_x)
		clip.max_x = current_hpos;
	if (current_vpos > clip.min_y)
		clip.min_y = current_vpos;
	if (current_vpos < clip.max_y)
		clip.max_y = current_vpos;

	// and if there's something to draw, do it
	if ((clip.min_x <= clip.max_x) && (clip.min_y <= clip.max_y))
	{
		g_profiler.start(PROFILER_VIDEO);

		LOG_PARTIAL_UPDATES(("doing scanline partial draw: Y %d X %d-%d\n", clip.max_y, clip.min_x, clip.max_x));

		UINT32 flags;
		screen_bitmap &curbitmap = m_bitmap[m_curbitmap];
		switch (curbitmap.format())
		{
			default:
			case BITMAP_FORMAT_IND16:   flags = m_screen_update_ind16(*this, curbitmap.as_ind16(), clip);   break;
			case BITMAP_FORMAT_RGB32:   flags = m_screen_update_rgb32(*this, curbitmap.as_rgb32(), clip);   break;
		}

		m_partial_updates_this_frame++;
		g_profiler.stop();

		// if we modified the bitmap, we have to commit
		m_changed |= ~flags & UPDATE_HAS_NOT_CHANGED;

		// remember where we left off
		m_partial_scan_hpos = current_hpos;
		m_last_partial_scan = current_vpos;

		// if we completed the line, mark it so
		if (current_hpos >= m_visarea.max_x)
		{
			m_partial_scan_hpos = 0;
			m_last_partial_scan = current_vpos + 1;
		}
	}
}


//-------------------------------------------------
//  reset_partial_updates - reset the partial
//  updating state
//-------------------------------------------------

void screen_device::reset_partial_updates()
{
	m_last_partial_scan = 0;
	m_partial_scan_hpos = 0;
	m_partial_updates_this_frame = 0;
	m_scanline0_timer->adjust(time_until_pos(0));
}


//-------------------------------------------------
//  vpos - returns the current vertical position
//  of the beam
//-------------------------------------------------

int screen_device::vpos() const
{
	attoseconds_t delta = (machine().time() - m_vblank_start_time).as_attoseconds();
	int vpos;

	// round to the nearest pixel
	delta += m_pixeltime / 2;

	// compute the v position relative to the start of VBLANK
	vpos = delta / m_scantime;

	// adjust for the fact that VBLANK starts at the bottom of the visible area
	return (m_visarea.max_y + 1 + vpos) % m_height;
}


//-------------------------------------------------
//  hpos - returns the current horizontal position
//  of the beam
//-------------------------------------------------

int screen_device::hpos() const
{
	attoseconds_t delta = (machine().time() - m_vblank_start_time).as_attoseconds();

	// round to the nearest pixel
	delta += m_pixeltime / 2;

	// compute the v position relative to the start of VBLANK
	int vpos = delta / m_scantime;

	// subtract that from the total time
	delta -= vpos * m_scantime;

	// return the pixel offset from the start of this scanline
	return delta / m_pixeltime;
}


//-------------------------------------------------
//  time_until_pos - returns the amount of time
//  remaining until the beam is at the given
//  hpos,vpos
//-------------------------------------------------

attotime screen_device::time_until_pos(int vpos, int hpos) const
{
	// validate arguments
	assert(vpos >= 0);
	assert(hpos >= 0);

	// since we measure time relative to VBLANK, compute the scanline offset from VBLANK
	vpos += m_height - (m_visarea.max_y + 1);
	vpos %= m_height;

	// compute the delta for the given X,Y position
	attoseconds_t targetdelta = (attoseconds_t)vpos * m_scantime + (attoseconds_t)hpos * m_pixeltime;

	// if we're past that time (within 1/2 of a pixel), head to the next frame
	attoseconds_t curdelta = (machine().time() - m_vblank_start_time).as_attoseconds();
	if (targetdelta <= curdelta + m_pixeltime / 2)
		targetdelta += m_frame_period;
	while (targetdelta <= curdelta)
		targetdelta += m_frame_period;

	// return the difference
	return attotime(0, targetdelta - curdelta);
}


//-------------------------------------------------
//  time_until_vblank_end - returns the amount of
//  time remaining until the end of the current
//  VBLANK (if in progress) or the end of the next
//  VBLANK
//-------------------------------------------------

attotime screen_device::time_until_vblank_end() const
{
	// if we are in the VBLANK region, compute the time until the end of the current VBLANK period
	attotime target_time = m_vblank_end_time;
	if (!vblank())
		target_time += attotime(0, m_frame_period);
	return target_time - machine().time();
}


//-------------------------------------------------
//  register_vblank_callback - registers a VBLANK
//  callback
//-------------------------------------------------

void screen_device::register_vblank_callback(vblank_state_delegate vblank_callback)
{
	// validate arguments
	assert(!vblank_callback.isnull());

	// do nothing if we already have this callback registered
	for (callback_item &item : m_callback_list)
		if (item.m_callback == vblank_callback)
			return;

	// if not found, register
	m_callback_list.append(*global_alloc(callback_item(vblank_callback)));
}


//-------------------------------------------------
//  register_screen_bitmap - registers a bitmap
//  that should track the screen size
//-------------------------------------------------

void screen_device::register_screen_bitmap(bitmap_t &bitmap)
{
	// append to the list
	m_auto_bitmap_list.append(*global_alloc(auto_bitmap_item(bitmap)));

	// if allocating now, just do it
	bitmap.allocate(width(), height());
	if (m_palette != nullptr)
		bitmap.set_palette(m_palette->palette());
}


//-------------------------------------------------
//  vblank_begin - call any external callbacks to
//  signal the VBLANK period has begun
//-------------------------------------------------

void screen_device::vblank_begin()
{
	// reset the starting VBLANK time
	m_vblank_start_time = machine().time();
	m_vblank_end_time = m_vblank_start_time + attotime(0, m_vblank_period);

	// if this is the primary screen and we need to update now
	if (this == machine().first_screen() && !(m_video_attributes & VIDEO_UPDATE_AFTER_VBLANK))
		machine().video().frame_update();

	// call the screen specific callbacks
	for (callback_item &item : m_callback_list)
		item.m_callback(*this, true);
	if (!m_screen_vblank.isnull())
		m_screen_vblank(*this, true);

	// reset the VBLANK start timer for the next frame
	m_vblank_begin_timer->adjust(time_until_vblank_start());

	// if no VBLANK period, call the VBLANK end callback immediately, otherwise reset the timer
	if (m_vblank_period == 0)
		vblank_end();
	else
		m_vblank_end_timer->adjust(time_until_vblank_end());
}


//-------------------------------------------------
//  vblank_end - call any external callbacks to
//  signal the VBLANK period has ended
//-------------------------------------------------

void screen_device::vblank_end()
{
	// call the screen specific callbacks
	for (callback_item &item : m_callback_list)
		item.m_callback(*this, false);
	if (!m_screen_vblank.isnull())
		m_screen_vblank(*this, false);

	// if this is the primary screen and we need to update now
	if (this == machine().first_screen() && (m_video_attributes & VIDEO_UPDATE_AFTER_VBLANK))
		machine().video().frame_update();

	// increment the frame number counter
	m_frame_number++;
}


//-------------------------------------------------
//  update_quads - set up the quads for this
//  screen
//-------------------------------------------------

bool screen_device::update_quads()
{
	// only update if live
	if (machine().render().is_live(*this))
	{
		// only update if empty and not a vector game; otherwise assume the driver did it directly
		if (m_type != SCREEN_TYPE_VECTOR && (m_video_attributes & VIDEO_SELF_RENDER) == 0)
		{
			// if we're not skipping the frame and if the screen actually changed, then update the texture
			if (!machine().video().skip_this_frame() && m_changed)
			{
				m_texture[m_curbitmap]->set_bitmap(m_bitmap[m_curbitmap], m_visarea, m_bitmap[m_curbitmap].texformat());
				m_curtexture = m_curbitmap;
				m_curbitmap = 1 - m_curbitmap;
			}

			// brightness adjusted render color
			rgb_t color = m_color - rgb_t(0, 0xff - m_brightness, 0xff - m_brightness, 0xff - m_brightness);

			// create an empty container with a single quad
			m_container->empty();
			m_container->add_quad(0.0f, 0.0f, 1.0f, 1.0f, color, m_texture[m_curtexture], PRIMFLAG_BLENDMODE(BLENDMODE_NONE) | PRIMFLAG_SCREENTEX(1));
		}
	}

	// reset the screen changed flags
	bool result = m_changed;
	m_changed = false;
	return result;
}


//-------------------------------------------------
//  update_burnin - update the burnin bitmap
//-------------------------------------------------

void screen_device::update_burnin()
{
// TODO: other than being unnecessary (we should use our rand function first off), this is a simplification of how analog signals really works!
#undef rand
	if (!m_burnin.valid())
		return;

	screen_bitmap &curbitmap = m_bitmap[m_curtexture];
	if (!curbitmap.valid())
		return;

	int srcwidth = curbitmap.width();
	int srcheight = curbitmap.height();
	int dstwidth = m_burnin.width();
	int dstheight = m_burnin.height();
	int xstep = (srcwidth << 16) / dstwidth;
	int ystep = (srcheight << 16) / dstheight;
	int xstart = ((UINT32)rand() % 32767) * xstep / 32767;
	int ystart = ((UINT32)rand() % 32767) * ystep / 32767;
	int srcx, srcy;
	int x, y;

	switch (curbitmap.format())
	{
		default:
		case BITMAP_FORMAT_IND16:
		{
			// iterate over rows in the destination
			bitmap_ind16 &srcbitmap = curbitmap.as_ind16();
			for (y = 0, srcy = ystart; y < dstheight; y++, srcy += ystep)
			{
				UINT64 *dst = &m_burnin.pix64(y);
				const UINT16 *src = &srcbitmap.pix16(srcy >> 16);
				const rgb_t *palette = m_palette->palette()->entry_list_adjusted();
				for (x = 0, srcx = xstart; x < dstwidth; x++, srcx += xstep)
				{
					rgb_t pixel = palette[src[srcx >> 16]];
					dst[x] += pixel.g() + pixel.r() + pixel.b();
				}
			}
			break;
		}

		case BITMAP_FORMAT_RGB32:
		{
			// iterate over rows in the destination
			bitmap_rgb32 &srcbitmap = curbitmap.as_rgb32();
			for (y = 0, srcy = ystart; y < dstheight; y++, srcy += ystep)
			{
				UINT64 *dst = &m_burnin.pix64(y);
				const UINT32 *src = &srcbitmap.pix32(srcy >> 16);
				for (x = 0, srcx = xstart; x < dstwidth; x++, srcx += xstep)
				{
					rgb_t pixel = src[srcx >> 16];
					dst[x] += pixel.g() + pixel.r() + pixel.b();
				}
			}
			break;
		}
	}
}


//-------------------------------------------------
//  finalize_burnin - finalize the burnin bitmap
//-------------------------------------------------

void screen_device::finalize_burnin()
{
	if (!m_burnin.valid())
		return;

	// compute the scaled visible region
	rectangle scaledvis;
	scaledvis.min_x = m_visarea.min_x * m_burnin.width() / m_width;
	scaledvis.max_x = m_visarea.max_x * m_burnin.width() / m_width;
	scaledvis.min_y = m_visarea.min_y * m_burnin.height() / m_height;
	scaledvis.max_y = m_visarea.max_y * m_burnin.height() / m_height;

	// wrap a bitmap around the memregion we care about
	bitmap_argb32 finalmap(scaledvis.width(), scaledvis.height());
	int srcwidth = m_burnin.width();
	int srcheight = m_burnin.height();
	int dstwidth = finalmap.width();
	int dstheight = finalmap.height();
	int xstep = (srcwidth << 16) / dstwidth;
	int ystep = (srcheight << 16) / dstheight;

	// find the maximum value
	UINT64 minval = ~(UINT64)0;
	UINT64 maxval = 0;
	for (int y = 0; y < srcheight; y++)
	{
		UINT64 *src = &m_burnin.pix64(y);
		for (int x = 0; x < srcwidth; x++)
		{
			minval = MIN(minval, src[x]);
			maxval = MAX(maxval, src[x]);
		}
	}

	if (minval == maxval)
		return;

	// now normalize and convert to RGB
	for (int y = 0, srcy = 0; y < dstheight; y++, srcy += ystep)
	{
		UINT64 *src = &m_burnin.pix64(srcy >> 16);
		UINT32 *dst = &finalmap.pix32(y);
		for (int x = 0, srcx = 0; x < dstwidth; x++, srcx += xstep)
		{
			int brightness = (UINT64)(maxval - src[srcx >> 16]) * 255 / (maxval - minval);
			dst[x] = rgb_t(0xff, brightness, brightness, brightness);
		}
	}

	// write the final PNG

	// compute the name and create the file
	emu_file file(machine().options().snapshot_directory(), OPEN_FLAG_WRITE | OPEN_FLAG_CREATE | OPEN_FLAG_CREATE_PATHS);
	osd_file::error filerr = file.open(machine().basename(), PATH_SEPARATOR "burnin-", this->tag()+1, ".png") ;
	if (filerr == osd_file::error::NONE)
	{
		png_info pnginfo = { nullptr };
//      png_error pngerr;
		char text[256];

		// add two text entries describing the image
		sprintf(text,"%s %s", emulator_info::get_appname(), emulator_info::get_build_version());
		png_add_text(&pnginfo, "Software", text);
		sprintf(text, "%s %s", machine().system().manufacturer, machine().system().description);
		png_add_text(&pnginfo, "System", text);

		// now do the actual work
		png_write_bitmap(file, &pnginfo, finalmap, 0, nullptr);

		// free any data allocated
		png_free(&pnginfo);
	}
}


//-------------------------------------------------
//  finalize_burnin - finalize the burnin bitmap
//-------------------------------------------------

void screen_device::load_effect_overlay(const char *filename)
{
	// ensure that there is a .png extension
	std::string fullname(filename);
	int extension = fullname.find_last_of('.');
	if (extension != -1)
		fullname.erase(extension, -1);
	fullname.append(".png");

	// load the file
	emu_file file(machine().options().art_path(), OPEN_FLAG_READ);
	render_load_png(m_screen_overlay_bitmap, file, nullptr, fullname.c_str());
	if (m_screen_overlay_bitmap.valid())
		m_container->set_overlay(&m_screen_overlay_bitmap);
	else
		osd_printf_warning("Unable to load effect PNG file '%s'\n", fullname.c_str());
}
