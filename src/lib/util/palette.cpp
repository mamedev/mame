// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/******************************************************************************

    palette.c

    Palette handling functions.

******************************************************************************/

#include "palette.h"

#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstdlib>


//**************************************************************************
//  INLINE FUNCTIONS
//**************************************************************************

//-------------------------------------------------
//  adjust_palette_entry - adjust a palette
//  entry for brightness
//-------------------------------------------------

inline rgb_t palette_t::adjust_palette_entry(rgb_t entry, float brightness, float contrast, const uint8_t *gamma_map)
{
	int r = rgb_t::clamp(float(gamma_map[entry.r()]) * contrast + brightness);
	int g = rgb_t::clamp(float(gamma_map[entry.g()]) * contrast + brightness);
	int b = rgb_t::clamp(float(gamma_map[entry.b()]) * contrast + brightness);
	int a = entry.a();
	return rgb_t(a,r,g,b);
}



//**************************************************************************
//  CLIENT DIRTY LIST MANAGEMENT
//**************************************************************************

//-------------------------------------------------
//  dirty_state - constructor
//-------------------------------------------------

palette_client::dirty_state::dirty_state() : m_mindirty(0), m_maxdirty(0)
{
}


//-------------------------------------------------
//  dirty_list - return the current list and
//  min/max values
//-------------------------------------------------

const uint32_t *palette_client::dirty_state::dirty_list(uint32_t &mindirty, uint32_t &maxdirty) noexcept
{
	// fill in the mindirty/maxdirty
	mindirty = m_mindirty;
	maxdirty = m_maxdirty;

	// if nothing to report, report nothing
	return (m_mindirty > m_maxdirty) ? nullptr : &m_dirty[0];
}


//-------------------------------------------------
//  resize - resize the dirty array and mark all
//  dirty
//-------------------------------------------------

void palette_client::dirty_state::resize(uint32_t colors)
{
	// resize to the correct number of dwords and mark all entries dirty
	uint32_t dirty_dwords = (colors + 31) / 32;
	m_dirty.resize(dirty_dwords);
	std::fill(m_dirty.begin(), m_dirty.end(), ~uint32_t(0));

	// mark all entries dirty
	m_dirty[dirty_dwords - 1] &= (1 << (colors % 32)) - 1;

	// set min/max
	m_mindirty = 0;
	m_maxdirty = colors - 1;
}


//-------------------------------------------------
//  mark_dirty - mark a single entry dirty
//-------------------------------------------------

void palette_client::dirty_state::mark_dirty(uint32_t index) noexcept
{
	m_dirty[index / 32] |= 1 << (index % 32);
	m_mindirty = std::min(m_mindirty, index);
	m_maxdirty = std::max(m_maxdirty, index);
}


//-------------------------------------------------
//  reset - clear the dirty array to mark all
//  entries as clean
//-------------------------------------------------

void palette_client::dirty_state::reset() noexcept
{
	// erase relevant entries in the new live one
	if (m_mindirty <= m_maxdirty)
		std::fill(&m_dirty[m_mindirty / 32], &m_dirty[m_maxdirty / 32] + 1, 0);
	m_mindirty = m_dirty.size() * 32 - 1;
	m_maxdirty = 0;
}



//**************************************************************************
//  PALETTE CLIENT
//**************************************************************************

//-------------------------------------------------
//  palette_client - constructor
//-------------------------------------------------

palette_client::palette_client(palette_t &palette) :
	m_palette(palette),
	m_next(nullptr),
	m_live(&m_dirty[0]),
	m_previous(&m_dirty[1])
{
	// add a reference to the palette
	palette.ref();

	// resize the dirty lists
	uint32_t total_colors = palette.num_colors() * palette.num_groups();
	m_dirty[0].resize(total_colors);
	m_dirty[1].resize(total_colors);

	// now add us to the list of clients
	m_next = palette.m_client_list;
	palette.m_client_list = this;
}


//-------------------------------------------------
//  ~palette_client - destructor
//-------------------------------------------------

palette_client::~palette_client()
{
	// first locate and remove ourself from our palette's list
	for (palette_client **curptr = &m_palette.m_client_list; *curptr != nullptr; curptr = &(*curptr)->m_next)
		if (*curptr == this)
		{
			*curptr = m_next;
			break;
		}

	// now deref the palette
	m_palette.deref();
}


//-------------------------------------------------
//  dirty_list - atomically get the current dirty
//  list for a client
//-------------------------------------------------

const uint32_t *palette_client::dirty_list(uint32_t &mindirty, uint32_t &maxdirty) noexcept
{
	// if nothing to report, report nothing and don't swap
	uint32_t const *const result = m_live->dirty_list(mindirty, maxdirty);
	if (result)
	{
		// swap the live and previous lists
		std::swap(m_live, m_previous);

		// reset new live one and return the pointer to the previous
		m_live->reset();
	}
	return result;
}



//**************************************************************************
//  PALETTE_T
//**************************************************************************

//-------------------------------------------------
//  alloc - static allocator
//-------------------------------------------------

palette_t *palette_t::alloc(uint32_t numcolors, uint32_t numgroups)
{
	return new palette_t(numcolors, numgroups);
}


//-------------------------------------------------
//  palette_t - constructor
//-------------------------------------------------

palette_t::palette_t(uint32_t numcolors, uint32_t numgroups) :
	m_refcount(1),
	m_numcolors(numcolors),
	m_numgroups(numgroups),
	m_brightness(0.0f),
	m_contrast(1.0f),
	m_gamma(1.0f),
	m_entry_color(numcolors),
	m_entry_contrast(numcolors),
	m_adjusted_color(numcolors * numgroups + 2),
	m_adjusted_rgb15(numcolors * numgroups + 2),
	m_group_bright(numgroups),
	m_group_contrast(numgroups),
	m_client_list(nullptr)
{
	// initialize gamma map
	for (uint32_t index = 0; index < 256; index++)
		m_gamma_map[index] = index;

	// initialize the per-entry data
	for (uint32_t index = 0; index < numcolors; index++)
	{
		m_entry_color[index] = rgb_t::black();
		m_entry_contrast[index] = 1.0f;
	}

	// initialize the per-group data
	for (uint32_t index = 0; index < numgroups; index++)
	{
		m_group_bright[index] = 0.0f;
		m_group_contrast[index] = 1.0f;
	}

	// initialize the expanded data
	for (uint32_t index = 0; index < numcolors * numgroups; index++)
	{
		m_adjusted_color[index] = rgb_t::black();
		m_adjusted_rgb15[index] = rgb_t::black().as_rgb15();
	}

	// add black and white as the last two colors
	m_adjusted_color[numcolors * numgroups + 0] = rgb_t::black();
	m_adjusted_rgb15[numcolors * numgroups + 0] = rgb_t::black().as_rgb15();
	m_adjusted_color[numcolors * numgroups + 1] = rgb_t::white();
	m_adjusted_rgb15[numcolors * numgroups + 1] = rgb_t::white().as_rgb15();
}


//-------------------------------------------------
//  palette_t - destructor
//-------------------------------------------------

palette_t::~palette_t()
{
}


//-------------------------------------------------
//  palette_t - destructor
//-------------------------------------------------

void palette_t::deref() noexcept
{
	if (--m_refcount == 0)
		delete this;
}


//-------------------------------------------------
//  set_brightness - set the overall brightness
//  for the palette
//-------------------------------------------------

void palette_t::set_brightness(float brightness)
{
	// convert incoming value to normalized result
	brightness = (brightness - 1.0f) * 256.0f;

	// set the global brightness if changed
	if (m_brightness == brightness)
		return;
	m_brightness = brightness;

	// update across all indices in all groups
	for (int groupnum = 0; groupnum < m_numgroups; groupnum++)
		for (int index = 0; index < m_numcolors; index++)
			update_adjusted_color(groupnum, index);
}


//-------------------------------------------------
//  set_contrast - set the overall contrast for
//  the palette
//-------------------------------------------------

void palette_t::set_contrast(float contrast)
{
	// set the global contrast if changed
	if (m_contrast == contrast)
		return;
	m_contrast = contrast;

	// update across all indices in all groups
	for (int groupnum = 0; groupnum < m_numgroups; groupnum++)
		for (int index = 0; index < m_numcolors; index++)
			update_adjusted_color(groupnum, index);
}


//-------------------------------------------------
//  set_gamma - set the overall gamma for the
//  palette
//-------------------------------------------------

void palette_t::set_gamma(float gamma)
{
	// set the global gamma if changed
	if (m_gamma == gamma)
		return;
	m_gamma = gamma;

	// recompute the gamma map
	gamma = 1.0f / gamma;
	for (int index = 0; index < 256; index++)
	{
		float fval = float(index) * (1.0f / 255.0f);
		float fresult = pow(fval, gamma);
		m_gamma_map[index] = rgb_t::clamp(255.0f * fresult);
	}

	// update across all indices in all groups
	for (int groupnum = 0; groupnum < m_numgroups; groupnum++)
		for (int index = 0; index < m_numcolors; index++)
			update_adjusted_color(groupnum, index);
}


//-------------------------------------------------
//  entry_set_color - set the raw RGB color for a
//  given palette index
//-------------------------------------------------

void palette_t::entry_set_color(uint32_t index, rgb_t rgb)
{
	assert(index < m_numcolors);

	// if unchanged, ignore
	if (m_entry_color[index] == rgb)
		return;

	// set the color
	m_entry_color[index] = rgb;

	// update across all groups
	for (int groupnum = 0; groupnum < m_numgroups; groupnum++)
		update_adjusted_color(groupnum, index);
}


//-------------------------------------------------
//  entry_set_red_level - set the red level for a
//  given palette index
//-------------------------------------------------

void palette_t::entry_set_red_level(uint32_t index, uint8_t level)
{
	assert(index < m_numcolors);

	// if unchanged, ignore
	if (m_entry_color[index].r() == level)
		return;

	// set the level
	m_entry_color[index].set_r(level);

	// update across all groups
	for (int groupnum = 0; groupnum < m_numgroups; groupnum++)
		update_adjusted_color(groupnum, index);
}


//-------------------------------------------------
//  entry_set_green_level - set the green level for a
//  given palette index
//-------------------------------------------------

void palette_t::entry_set_green_level(uint32_t index, uint8_t level)
{
	assert(index < m_numcolors);

	// if unchanged, ignore
	if (m_entry_color[index].g() == level)
		return;

	// set the level
	m_entry_color[index].set_g(level);

	// update across all groups
	for (int groupnum = 0; groupnum < m_numgroups; groupnum++)
		update_adjusted_color(groupnum, index);
}


//-------------------------------------------------
//  entry_set_blue_level - set the blue level for a
//  given palette index
//-------------------------------------------------

void palette_t::entry_set_blue_level(uint32_t index, uint8_t level)
{
	assert(index < m_numcolors);

	// if unchanged, ignore
	if (m_entry_color[index].b() == level)
		return;

	// set the level
	m_entry_color[index].set_b(level);

	// update across all groups
	for (int groupnum = 0; groupnum < m_numgroups; groupnum++)
		update_adjusted_color(groupnum, index);
}


//-------------------------------------------------
//  entry_set_contrast - set the contrast
//  adjustment for a single palette index
//-------------------------------------------------

void palette_t::entry_set_contrast(uint32_t index, float contrast)
{
	assert(index < m_numcolors);

	// if unchanged, ignore
	if (m_entry_contrast[index] == contrast)
		return;

	// set the contrast
	m_entry_contrast[index] = contrast;

	// update across all groups
	for (int groupnum = 0; groupnum < m_numgroups; groupnum++)
		update_adjusted_color(groupnum, index);
}


//-------------------------------------------------
//  group_set_brightness - configure overall
//  brightness for a palette group
//-------------------------------------------------

void palette_t::group_set_brightness(uint32_t group, float brightness)
{
	assert(group < m_numgroups);

	// convert incoming value to normalized result
	brightness = (brightness - 1.0f) * 256.0f;

	// if unchanged, ignore
	if (m_group_bright[group] == brightness)
		return;

	// set the contrast
	m_group_bright[group] = brightness;

	// update across all colors
	for (int index = 0; index < m_numcolors; index++)
		update_adjusted_color(group, index);
}


//-------------------------------------------------
//  group_set_contrast - configure overall
//  contrast for a palette group
//-------------------------------------------------

void palette_t::group_set_contrast(uint32_t group, float contrast)
{
	assert(group < m_numgroups);

	// if unchanged, ignore
	if (m_group_contrast[group] == contrast)
		return;

	// set the contrast
	m_group_contrast[group] = contrast;

	// update across all colors
	for (int index = 0; index < m_numcolors; index++)
		update_adjusted_color(group, index);
}


//-------------------------------------------------
//  normalize_range - normalize a range of palette
//  entries
//-------------------------------------------------

void palette_t::normalize_range(uint32_t start, uint32_t end, int lum_min, int lum_max)
{
	// clamp within range
	// start = std::max(start, 0U); ==> reduces to start = start
	end = std::min(end, m_numcolors - 1);

	// find the minimum and maximum brightness of all the colors in the range
	int32_t ymin = 1000 * 255, ymax = 0;
	for (uint32_t index = start; index <= end; index++)
	{
		rgb_t rgb = m_entry_color[index];
		uint32_t y = 299 * rgb.r() + 587 * rgb.g() + 114 * rgb.b();
		ymin = (std::min<uint32_t>)(ymin, y);
		ymax = (std::max<uint32_t>)(ymax, y);
	}

	// determine target minimum/maximum
	int32_t tmin = (lum_min < 0) ? ((ymin + 500) / 1000) : lum_min;
	int32_t tmax = (lum_max < 0) ? ((ymax + 500) / 1000) : lum_max;

	// now normalize the palette
	for (uint32_t index = start; index <= end; index++)
	{
		rgb_t rgb = m_entry_color[index];
		int32_t y = 299 * rgb.r() + 587 * rgb.g() + 114 * rgb.b();
		int32_t u = ((int32_t)rgb.b()-y /1000)*492 / 1000;
		int32_t v = ((int32_t)rgb.r()-y / 1000)*877 / 1000;
		int32_t target = tmin + ((y - ymin) * (tmax - tmin + 1)) / (ymax - ymin);
		uint8_t r = rgb_t::clamp(target + 1140 * v / 1000);
		uint8_t g = rgb_t::clamp(target -  395 * u / 1000 - 581 * v / 1000);
		uint8_t b = rgb_t::clamp(target + 2032 * u / 1000);
		entry_set_color(index, rgb_t(r, g, b));
	}
}

/**
 * @fn  void palette_t::update_adjusted_color(uint32_t group, uint32_t index)
 *
 * @brief   -------------------------------------------------
 *            update_adjusted_color - update a color index by group and index pair
 *          -------------------------------------------------
 *
 * @param   group   The group.
 * @param   index   Zero-based index of the group.
 */

void palette_t::update_adjusted_color(uint32_t group, uint32_t index)
{
	// compute the adjusted value
	rgb_t adjusted = adjust_palette_entry(m_entry_color[index],
			m_group_bright[group] + m_brightness,
			m_group_contrast[group] * m_entry_contrast[index] * m_contrast,
			m_gamma_map);

	// if not different, ignore
	uint32_t finalindex = group * m_numcolors + index;
	if (m_adjusted_color[finalindex] == adjusted)
		return;

	// otherwise, modify the adjusted color array
	m_adjusted_color[finalindex] = adjusted;
	m_adjusted_rgb15[finalindex] = adjusted.as_rgb15();

	// mark dirty in all clients
	for (palette_client *client = m_client_list; client != nullptr; client = client->next())
		client->mark_dirty(finalindex);
}
