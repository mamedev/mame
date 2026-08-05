// license:BSD-3-Clause
// copyright-holders:Brad Oliver,Aaron Giles,Bernd Wiebelt,Allard van der Bas
/******************************************************************************
 *
 * vector.cpp
 *
 *        anti-alias code by Andrew Caldwell
 *        (still more to add)
 *
 * Vector Team
 *
 *        Brad Oliver
 *        Aaron Giles
 *        Bernd Wiebelt
 *        Allard van der Bas
 *        Al Kossow (VECSIM)
 *        Hedley Rainnie (VECSIM)
 *        Eric Smith (VECSIM)
 *        Neil Bradley (technical advice)
 *        Andrew Caldwell (anti-aliasing)
 *
 **************************************************************************** */

#include "emu.h"
#include "vector.h"

#include "emuopts.h"

#define VECTOR_WIDTH_DENOM 512

// 20000 is needed for mhavoc (see MT 06668) 10000 is enough for other games
#define MAX_POINTS 20000

float vector_options::s_flicker = 0.0f;
float vector_options::s_beam_width_min = 0.0f;
float vector_options::s_beam_width_max = 0.0f;
float vector_options::s_beam_dot_size = 0.0f;
float vector_options::s_beam_intensity_weight = 0.0f;

void vector_options::init(emu_options &options)
{
	s_beam_width_min = options.beam_width_min();
	s_beam_width_max = options.beam_width_max();
	s_beam_dot_size = options.beam_dot_size();
	s_beam_intensity_weight = options.beam_intensity_weight();
	s_flicker = options.flicker();
}

// device type definition
DEFINE_DEVICE_TYPE(VECTOR, vector_device, "vector_device", "VECTOR")

vector_device::vector_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, VECTOR, tag, owner, clock),
		device_video_output_interface(mconfig, *this),
		m_vector_list(nullptr),
		m_vector_index(0),
		m_min_intensity(255),
		m_max_intensity(0),
		m_visarea(rectangle()),
		m_frame_period(attotime::from_hz(DEFAULT_FRAME_RATE)),
		m_color(rgb_t::green()),
		m_vblank(*this),
		m_vector_update(*this),
		m_vblank_timer(nullptr)
{
}

void vector_device::device_start()
{
	m_vector_update.resolve();

	vector_options::init(machine().options());

	m_prevpoint.x = m_prevpoint.y = m_prevpoint.col = m_prevpoint.intensity = 0;
	m_vector_index = 0;

	/* allocate memory for tables */
	m_vector_list = std::make_unique<point[]>(MAX_POINTS);

	// register items for saving
	save_item(STRUCT_MEMBER(m_prevpoint, x));
	save_item(STRUCT_MEMBER(m_prevpoint, y));
	save_item(STRUCT_MEMBER(m_prevpoint, col));
	save_item(STRUCT_MEMBER(m_prevpoint, intensity));
	save_item(NAME(m_frame_period));

	/* allocate and start the vblank timer */
	m_vblank_timer = timer_alloc(FUNC(vector_device::vblank_timer_callback), this);
	m_vblank_timer->adjust(m_frame_period, 0, m_frame_period);
}


//-------------------------------------------------
//  vblank_timer_callback - periodically fires to
//  drive update_if_primary
//-------------------------------------------------

void vector_device::vblank_timer_callback(s32 param)
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

void vector_device::override_frame_period(attotime period)
{
	m_frame_period = period;
	m_vblank_timer->adjust(m_frame_period, 0, m_frame_period);
}

//-------------------------------------------------
//  subscribe for frame-begin notifications
//-------------------------------------------------

util::notifier_subscription vector_device::add_frame_begin_notifier(frame_begin_delegate &&n)
{
	return m_frame_begin_notifier.subscribe(std::move(n));
}


//-------------------------------------------------
//  subscribe for frame-end notifications
//-------------------------------------------------

util::notifier_subscription vector_device::add_frame_end_notifier(frame_end_delegate &&n)
{
	return m_frame_end_notifier.subscribe(std::move(n));
}


//-------------------------------------------------
//  subscribe for hidden-move notifications
//-------------------------------------------------

util::notifier_subscription vector_device::add_move_notifier(move_delegate &&n)
{
	return m_move_notifier.subscribe(std::move(n));
}


//-------------------------------------------------
//  subscribe for visible-line notifications
//-------------------------------------------------

util::notifier_subscription vector_device::add_line_notifier(line_delegate &&n)
{
	return m_line_notifier.subscribe(std::move(n));
}


//-------------------------------------------------
// www.dinodini.wordpress.com/2010/04/05/normalized-tunable-sigmoid-functions/
//-------------------------------------------------

float vector_device::normalized_sigmoid(float n, float k)
{
	// valid for n and k in range of -1.0 and 1.0
	return (n - n * k) / (k - fabs(n) * 2.0f * k + 1.0f);
}


//-------------------------------------------------
// Adds a line end point to the vertices list. The vector processor emulation
// needs to call this.
//-------------------------------------------------

void vector_device::add_point(int x, int y, rgb_t color, int intensity)
{
	point *newpoint;

	intensity = std::clamp(intensity, 0, 255);

	m_min_intensity = intensity > 0 ? std::min(m_min_intensity, intensity) : m_min_intensity;
	m_max_intensity = intensity > 0 ? std::max(m_max_intensity, intensity) : m_max_intensity;

	if (vector_options::s_flicker && (intensity > 0))
	{
		float random = float(machine().rand() & 255) / 255.0f; // random value between 0.0 and 1.0

		intensity -= int(intensity * random * vector_options::s_flicker);

		intensity = std::clamp(intensity, 0, 255);
	}

	newpoint = &m_vector_list[m_vector_index];
	newpoint->x = x;
	newpoint->y = y;
	newpoint->col = color;
	newpoint->intensity = intensity;

	m_vector_index++;
	if (m_vector_index >= MAX_POINTS)
	{
		m_vector_index--;
		logerror("*** Warning! Vector list overflow!\n");
	}
}


//-------------------------------------------------
// The vector CPU creates a new display list. We save the old display list,
// but only once per refresh.
//-------------------------------------------------

void vector_device::clear_list()
{
	m_vector_index = 0;
}

//-------------------------------------------------
// Update the container with queued vectors.
//-------------------------------------------------

bool vector_device::video_output_update()
{
	if (!m_vector_update.isnull())
		m_vector_update(*this);

	uint32_t flags = PRIMFLAG_ANTIALIAS(1) | PRIMFLAG_BLENDMODE(BLENDMODE_ADD) | PRIMFLAG_VECTOR(1);
	const rectangle &visarea = m_visarea;
	float xscale = 1.0f / (65536 * visarea.width());
	float yscale = 1.0f / (65536 * visarea.height());
	float xoffs = (float)visarea.min_x;
	float yoffs = (float)visarea.min_y;

	point *curpoint;

	curpoint = m_vector_list.get();

	container().empty();
	container().add_rect(0.0f, 0.0f, 1.0f, 1.0f, rgb_t(0xff,0x00,0x00,0x00), PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA) | PRIMFLAG_VECTORBUF(1));

	m_frame_begin_notifier();

	for (int i = 0; i < m_vector_index; i++)
	{
		render_bounds coords;

		float intensity = (float)curpoint->intensity / 255.0f;
		float intensity_weight = normalized_sigmoid(intensity, vector_options::s_beam_intensity_weight);

		// check for static intensity
		float beam_width = m_min_intensity == m_max_intensity
			? vector_options::s_beam_width_min
			: vector_options::s_beam_width_min + intensity_weight * (vector_options::s_beam_width_max - vector_options::s_beam_width_min);

		// normalize width
		beam_width *= 1.0f / (float)VECTOR_WIDTH_DENOM;

		// apply point scale for points
		if (m_prevpoint.x == curpoint->x && m_prevpoint.y == curpoint->y)
			beam_width *= vector_options::s_beam_dot_size;

		coords.x0 = (float(m_prevpoint.x) - xoffs) * xscale;
		coords.y0 = (float(m_prevpoint.y) - yoffs) * yscale;
		coords.x1 = (float(curpoint->x) - xoffs) * xscale;
		coords.y1 = (float(curpoint->y) - yoffs) * yscale;

		if (curpoint->intensity != 0)
		{
			container().add_line(
					coords.x0, coords.y0, coords.x1, coords.y1,
					beam_width,
					(curpoint->intensity << 24) | (curpoint->col & 0xffffff),
					flags);
			m_line_notifier(m_prevpoint.x, m_prevpoint.y, curpoint->x, curpoint->y, curpoint->col, curpoint->intensity, visarea.width(), visarea.height());
		}
		else
		{
			m_move_notifier(curpoint->x, curpoint->y, curpoint->col, visarea.width(), visarea.height());
		}

		m_prevpoint.x = curpoint->x;
		m_prevpoint.y = curpoint->y;
		m_prevpoint.col = curpoint->col;
		m_prevpoint.intensity = curpoint->intensity;

		curpoint++;
	}

	m_frame_end_notifier();

	return true;
}


//-------------------------------------------------
//  physical aspect ratio
//-------------------------------------------------

std::pair<unsigned, unsigned> vector_device::physical_aspect() const
{
	return { 4, 3 };
}
