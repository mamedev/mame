// license:BSD-3-Clause
// copyright-holders:Brad Oliver,Aaron Giles,Bernd Wiebelt,Allard van der Bas
/******************************************************************************
 *
 * vector.c
 *
 *        anti-alias code by Andrew Caldwell
 *        (still more to add)
 *
 * 040227 Fixed miny clip scaling which was breaking in mhavoc. AREK
 * 010903 added support for direct RGB modes MLR
 * 980611 use translucent vectors. Thanks to Peter Hirschberg
 *        and Neil Bradley for the inspiration. BW
 * 980307 added cleverer dirty handling. BW, ASG
 *        fixed antialias table .ac
 * 980221 rewrote anti-alias line draw routine
 *        added inline assembly multiply fuction for 8086 based machines
 *        beam diameter added to draw routine
 *        beam diameter is accurate in anti-alias line draw (Tcosin)
 *        flicker added .ac
 * 980203 moved LBO's routines for drawing into a buffer of vertices
 *        from avgdvg.c to this location. Scaling is now initialized
 *        by calling vector_init(...). BW
 * 980202 moved out of msdos.c ASG
 * 980124 added anti-alias line draw routine
 *        modified avgdvg.c and sega.c to support new line draw routine
 *        added two new tables Tinten and Tmerge (for 256 color support)
 *        added find_color routine to build above tables .ac
 *
 **************************************************************************** */

#include "emu.h"
#include "emuopts.h"
#include "rendutil.h"
#include "vector.h"


#define FLT_EPSILON 1E-5

#define VECTOR_WIDTH_DENOM 512

#define MAX_POINTS 10000

#define VECTOR_TEAM \
	"-* Vector Heads *-\n" \
	"Brad Oliver\n" \
	"Aaron Giles\n" \
	"Bernd Wiebelt\n" \
	"Allard van der Bas\n" \
	"Al Kossow (VECSIM)\n" \
	"Hedley Rainnie (VECSIM)\n" \
	"Eric Smith (VECSIM)\n" \
	"Neil Bradley (technical advice)\n" \
	"Andrew Caldwell (anti-aliasing)\n" \
	"- *** -\n"

#if 0

#define TEXTURE_LENGTH_BUCKETS      32
#define TEXTURE_INTENSITY_BUCKETS   4
#define TEXTURE_WIDTH               16

#define MAX_INTENSITY               2
#define VECTOR_BLEED                (0.25f)
#define VECTOR_INT_SCALE            (255.0f * 1.5f)


struct vector_texture
{
	render_texture *    texture;
	bitmap_argb32 *     bitmap;
};

static vector_texture *vectortex[TEXTURE_INTENSITY_BUCKETS][TEXTURE_LENGTH_BUCKETS];


static render_texture *get_vector_texture(float dx, float dy, float intensity)
{
	float length = sqrt(dx * dx + dy * dy);
	int lbucket = length * (float)TEXTURE_LENGTH_BUCKETS;
	int ibucket = (intensity / (float)MAX_INTENSITY) * (float)TEXTURE_INTENSITY_BUCKETS;
	vector_texture *tex;
	int height, x, y;
	float totalint;

	if (lbucket > TEXTURE_LENGTH_BUCKETS)
		lbucket = TEXTURE_LENGTH_BUCKETS;
	if (ibucket > TEXTURE_INTENSITY_BUCKETS)
		ibucket = TEXTURE_INTENSITY_BUCKETS;

	tex = &vectortex[ibucket][lbucket];
	if (tex->texture != NULL)
		return tex->texture;

	height = lbucket * VECTOR_WIDTH_DENOM / TEXTURE_LENGTH_BUCKETS;
	tex->bitmap = global_alloc(bitmap_argb32(TEXTURE_WIDTH, height));
	tex->bitmap.fill(rgb_t(0xff,0xff,0xff,0xff));

	totalint = 1.0f;
	for (x = TEXTURE_WIDTH / 2 - 1; x >= 0; x--)
	{
		int intensity = (int)(totalint * (1.0f - VECTOR_BLEED) * VECTOR_INT_SCALE);
		intensity = MIN(255, intensity);
		totalint -= (float)intensity * (1.0f / VECTOR_INT_SCALE);

		for (y = 0; y < height; y++)
		{
			UINT32 *pix;

			pix = (UINT32 *)bitmap.base + y * bitmap.rowpixels + x;
			*pix = rgb_t((*pix.a() * intensity) >> 8,0xff,0xff,0xff);

			pix = (UINT32 *)bitmap.base + y * bitmap.rowpixels + (TEXTURE_WIDTH - 1 - x);
			*pix = rgb_t((*pix.a() * intensity) >> 8,0xff,0xff,0xff);
		}
	}

	tex->texture = render_texture_create();
	return tex->texture;
}

#endif

#define VCLEAN  0
#define VDIRTY  1
#define VCLIP   2

// device type definition
const device_type VECTOR = &device_creator<vector_device>;

vector_device::vector_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source)
	: device_t(mconfig, type, name, tag, owner, clock, shortname, source),
		device_video_interface(mconfig, *this), 
		m_vector_list(NULL)
{
}

vector_device::vector_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, VECTOR, "VECTOR", tag, owner, clock, "vector_device", __FILE__),
	  device_video_interface(mconfig, *this),
	  m_vector_list(NULL)
{
}

float vector_device::m_flicker = 0.0f;
float vector_device::m_beam_width_min = 0.0f;
float vector_device::m_beam_width_max = 0.0f;
float vector_device::m_beam_intensity_weight = 0.0f;
int vector_device::m_vector_index;

void vector_device::device_start()
{
	/* Grab the settings for this session */
	m_beam_width_min = machine().options().beam_width_min();
	m_beam_width_max = machine().options().beam_width_max();
	m_beam_intensity_weight = machine().options().beam_intensity_weight();
	m_flicker = machine().options().flicker();

	m_vector_index = 0;

	/* allocate memory for tables */
	m_vector_list = auto_alloc_array_clear(machine(), point, MAX_POINTS);
}

void vector_device::set_flicker(float newval)
{
	m_flicker = newval;
}

float vector_device::get_flicker()
{
	return m_flicker;
}

void vector_device::set_beam_width_min(float newval)
{
	m_beam_width_min = newval;
}

float vector_device::get_beam_width_min()
{
	return m_beam_width_min;
}

void vector_device::set_beam_width_max(float newval)
{
	m_beam_width_max = newval;
}

float vector_device::get_beam_width_max()
{
	return m_beam_width_max;
}

void vector_device::set_beam_intensity_weight(float newval)
{
	m_beam_intensity_weight = newval;
}

float vector_device::get_beam_intensity_weight()
{
	return m_beam_intensity_weight;
}


/*
 * www.dinodini.wordpress.com/2010/04/05/normalized-tunable-sigmoid-functions/
 */
float vector_device::normalized_sigmoid(float n, float k)
{
	// valid for n and k in range of -1.0 and 1.0
	return (n - n * k) / (k - fabs(n) * 2.0f * k + 1.0f);
}


/*
 * Adds a line end point to the vertices list. The vector processor emulation
 * needs to call this.
 */
void vector_device::add_point(int x, int y, rgb_t color, int intensity)
{
	point *newpoint;

	if (intensity > 255)
	{
		intensity = 255;
	}

	if (m_flicker && (intensity > 0))
	{
		float random = (float)(machine().rand() & 255) / 255.0f; // random value between 0.0 and 1.0

		intensity -= (int)(intensity * random * m_flicker);
		if (intensity < 0)
		{
			intensity = 0;
		}
		if (intensity > 255)
		{
			intensity = 255;
		}
	}

	newpoint = &m_vector_list[m_vector_index];
	newpoint->x = x;
	newpoint->y = y;
	newpoint->col = color;
	newpoint->intensity = intensity;
	newpoint->status = VDIRTY; /* mark identical lines as clean later */

	m_vector_index++;
	if (m_vector_index >= MAX_POINTS)
	{
		m_vector_index--;
		logerror("*** Warning! Vector list overflow!\n");
	}
}


/*
 * Add new clipping info to the list
 */
void vector_device::add_clip(int x1, int yy1, int x2, int y2)
{
	point *newpoint;

	newpoint = &m_vector_list[m_vector_index];
	newpoint->x = x1;
	newpoint->y = yy1;
	newpoint->arg1 = x2;
	newpoint->arg2 = y2;
	newpoint->status = VCLIP;

	m_vector_index++;
	if (m_vector_index >= MAX_POINTS)
	{
		m_vector_index--;
		logerror("*** Warning! Vector list overflow!\n");
	}
}


/*
 * The vector CPU creates a new display list. We save the old display list,
 * but only once per refresh.
 */
void vector_device::clear_list(void)
{
	m_vector_index = 0;
}


UINT32 vector_device::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	UINT32 flags = PRIMFLAG_ANTIALIAS(screen.machine().options().antialias() ? 1 : 0) | PRIMFLAG_BLENDMODE(BLENDMODE_ADD) | PRIMFLAG_VECTOR(1);
	const rectangle &visarea = screen.visible_area();
	float xscale = 1.0f / (65536 * visarea.width());
	float yscale = 1.0f / (65536 * visarea.height());
	float xoffs = (float)visarea.min_x;
	float yoffs = (float)visarea.min_y;
	float xratio = xscale / yscale;
	float yratio = yscale / xscale;
	xratio = (xratio < 1.0f) ? xratio : 1.0f;
	yratio = (yratio < 1.0f) ? yratio : 1.0f;

	point *curpoint;
	render_bounds clip;
	int lastx = 0;
	int lasty = 0;

	curpoint = m_vector_list;

	screen.container().empty();
	screen.container().add_rect(0.0f, 0.0f, 1.0f, 1.0f, rgb_t(0xff,0x00,0x00,0x00), PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA) | PRIMFLAG_VECTORBUF(1));

	clip.x0 = clip.y0 = 0.0f;
	clip.x1 = clip.y1 = 1.0f;

	for (int i = 0; i < m_vector_index; i++)
	{
		render_bounds coords;

		if (curpoint->status == VCLIP)
		{
			coords.x0 = ((float)curpoint->x - xoffs) * xscale;
			coords.y0 = ((float)curpoint->y - yoffs) * yscale;
			coords.x1 = ((float)curpoint->arg1 - xoffs) * xscale;
			coords.y1 = ((float)curpoint->arg2 - yoffs) * yscale;

			clip.x0 = (coords.x0 > 0.0f) ? coords.x0 : 0.0f;
			clip.y0 = (coords.y0 > 0.0f) ? coords.y0 : 0.0f;
			clip.x1 = (coords.x1 < 1.0f) ? coords.x1 : 1.0f;
			clip.y1 = (coords.y1 < 1.0f) ? coords.y1 : 1.0f;
		}
		else
		{
			float intensity = (float)curpoint->intensity / 255.0f;
			float intensity_weight = normalized_sigmoid(intensity, m_beam_intensity_weight);

			float beam_intensity_width = (m_beam_width_max - m_beam_width_min) * intensity_weight + m_beam_width_min;
			float beam_width = beam_intensity_width * (1.0f / (float)VECTOR_WIDTH_DENOM);

			coords.x0 = ((float)lastx - xoffs) * xscale;
			coords.y0 = ((float)lasty - yoffs) * yscale;
			coords.x1 = ((float)curpoint->x - xoffs) * xscale;
			coords.y1 = ((float)curpoint->y - yoffs) * yscale;

			// extend zero-length vector line (vector point) by quarter beam_width on both sides
			if (fabs(coords.x0 - coords.x1) < FLT_EPSILON &&
				fabs(coords.y0 - coords.y1) < FLT_EPSILON)
			{
				coords.x0 += xratio * beam_width * 0.25f;
				coords.y0 += yratio * beam_width * 0.25f;
				coords.x1 -= xratio * beam_width * 0.25f;
				coords.y1 -= yratio * beam_width * 0.25f;
			}

			if (curpoint->intensity != 0 && !render_clip_line(&coords, &clip))
			{
				screen.container().add_line(
					coords.x0, coords.y0, coords.x1, coords.y1,
					beam_width,
					(curpoint->intensity << 24) | (curpoint->col & 0xffffff),
					flags);
			}

			lastx = curpoint->x;
			lasty = curpoint->y;
		}

		curpoint++;
	}

	return 0;
}
