// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/******************************************************************************

    palette.h

    Core palette routines.

***************************************************************************/

#ifndef MAME_UTIL_PALETTE_H
#define MAME_UTIL_PALETTE_H

#pragma once

#include "osdcore.h"
#include "coretmpl.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// forward definitions
class palette_t;

// an rgb15_t is a single combined 15-bit R,G,B value
typedef uint16_t rgb15_t;


// ======================> rgb_t

// an rgb_t is a single combined R,G,B (and optionally alpha) value
class rgb_t
{
public:
	// construction/destruction
	constexpr rgb_t() : m_data(0) { }
	constexpr rgb_t(uint32_t data) : m_data(data) { }
	constexpr rgb_t(uint8_t r, uint8_t g, uint8_t b) : m_data((255 << 24) | (r << 16) | (g << 8) | b) { }
	constexpr rgb_t(uint8_t a, uint8_t r, uint8_t g, uint8_t b) : m_data((a << 24) | (r << 16) | (g << 8) | b) { }

	// getters
	constexpr uint8_t a() const { return m_data >> 24; }
	constexpr uint8_t r() const { return m_data >> 16; }
	constexpr uint8_t g() const { return m_data >> 8; }
	constexpr uint8_t b() const { return m_data >> 0; }
	constexpr rgb15_t as_rgb15() const { return ((r() >> 3) << 10) | ((g() >> 3) << 5) | ((b() >> 3) << 0); }
	constexpr uint8_t brightness() const { return (r() * 222 + g() * 707 + b() * 71) / 1000; }
	constexpr uint32_t const *ptr() const { return &m_data; }
	void expand_rgb(uint8_t &r, uint8_t &g, uint8_t &b) const { r = m_data >> 16; g = m_data >> 8; b = m_data >> 0; }
	void expand_rgb(int &r, int &g, int &b) const { r = (m_data >> 16) & 0xff; g = (m_data >> 8) & 0xff; b = (m_data >> 0) & 0xff; }

	// setters
	rgb_t &set_a(uint8_t a) { m_data &= ~0xff000000; m_data |= a << 24; return *this; }
	rgb_t &set_r(uint8_t r) { m_data &= ~0x00ff0000; m_data |= r << 16; return *this; }
	rgb_t &set_g(uint8_t g) { m_data &= ~0x0000ff00; m_data |= g <<  8; return *this; }
	rgb_t &set_b(uint8_t b) { m_data &= ~0x000000ff; m_data |= b <<  0; return *this; }

	// implicit conversion operators
	constexpr operator uint32_t() const { return m_data; }

	// operations
	rgb_t &scale8(uint8_t scale) { m_data = rgb_t(clamphi((a() * scale) >> 8), clamphi((r() * scale) >> 8), clamphi((g() * scale) >> 8), clamphi((b() * scale) >> 8)); return *this; }

	// assignment operators
	rgb_t &operator=(uint32_t rhs) { m_data = rhs; return *this; }
	rgb_t &operator+=(const rgb_t &rhs) { m_data = uint32_t(*this + rhs); return *this; }
	rgb_t &operator-=(const rgb_t &rhs) { m_data = uint32_t(*this - rhs); return *this; }

	// arithmetic operators
	constexpr rgb_t operator+(const rgb_t &rhs) const { return rgb_t(clamphi(a() + rhs.a()), clamphi(r() + rhs.r()), clamphi(g() + rhs.g()), clamphi(b() + rhs.b())); }
	constexpr rgb_t operator-(const rgb_t &rhs) const { return rgb_t(clamplo(a() - rhs.a()), clamplo(r() - rhs.r()), clamplo(g() - rhs.g()), clamplo(b() - rhs.b())); }

	// static helpers
	static constexpr uint8_t clamp(int32_t value) { return (value < 0) ? 0 : (value > 255) ? 255 : value; }
	static constexpr uint8_t clamphi(int32_t value) { return (value > 255) ? 255 : value; }
	static constexpr uint8_t clamplo(int32_t value) { return (value < 0) ? 0 : value; }

	// constant factories
	static constexpr rgb_t black() { return rgb_t(0, 0, 0); }
	static constexpr rgb_t white() { return rgb_t(255, 255, 255); }
	static constexpr rgb_t green() { return rgb_t(0, 255, 0); }
	static constexpr rgb_t amber() { return rgb_t(247, 170, 0); }
	static constexpr rgb_t transparent() { return rgb_t(0, 0, 0, 0); }

private:
	uint32_t  m_data;
};


// ======================> palette_client

// a single palette client
class palette_client
{
public:
	// construction/destruction
	palette_client(palette_t &palette);
	~palette_client();

	// getters
	palette_client *next() const { return m_next; }
	palette_t &palette() const { return m_palette; }
	const uint32_t *dirty_list(uint32_t &mindirty, uint32_t &maxdirty);

	// dirty marking
	void mark_dirty(uint32_t index) { m_live->mark_dirty(index); }

private:
	// internal object to track dirty states
	class dirty_state
	{
	public:
		// construction
		dirty_state();

		// operations
		const uint32_t *dirty_list(uint32_t &mindirty, uint32_t &maxdirty);
		void resize(uint32_t colors);
		void mark_dirty(uint32_t index);
		void reset();

	private:
		// internal state
		std::vector<uint32_t> m_dirty;          // bitmap of dirty entries
		uint32_t          m_mindirty;             // minimum dirty entry
		uint32_t          m_maxdirty;             // minimum dirty entry
	};

	// internal state
	palette_t &     m_palette;                  // reference to the palette
	palette_client *m_next;                     // pointer to next client
	dirty_state *   m_live;                     // live dirty state
	dirty_state *   m_previous;                 // previous dirty state
	dirty_state     m_dirty[2];                 // two dirty states
};


// ======================> palette_t

// a palette object
class palette_t
{
	friend class palette_client;

public:
	// static constructor: used to ensure same new/delete is used
	static palette_t *alloc(uint32_t numcolors, uint32_t numgroups = 1);

	// reference counting
	void ref() { m_refcount++; }
	void deref();

	// getters
	int num_colors() const { return m_numcolors; }
	int num_groups() const { return m_numgroups; }
	int max_index() const { return m_numcolors * m_numgroups + 2; }
	uint32_t black_entry() const { return m_numcolors * m_numgroups + 0; }
	uint32_t white_entry() const { return m_numcolors * m_numgroups + 1; }

	// overall adjustments
	void set_brightness(float brightness);
	void set_contrast(float contrast);
	void set_gamma(float gamma);

	// entry getters
	rgb_t entry_color(uint32_t index) const { return (index < m_numcolors) ? m_entry_color[index] : rgb_t::black(); }
	rgb_t entry_adjusted_color(uint32_t index) const { return (index < m_numcolors * m_numgroups) ? m_adjusted_color[index] : rgb_t::black(); }
	float entry_contrast(uint32_t index) const { return (index < m_numcolors) ? m_entry_contrast[index] : 1.0f; }

	// entry setters
	void entry_set_color(uint32_t index, rgb_t rgb);
	void entry_set_red_level(uint32_t index, uint8_t level);
	void entry_set_green_level(uint32_t index, uint8_t level);
	void entry_set_blue_level(uint32_t index, uint8_t level);
	void entry_set_contrast(uint32_t index, float contrast);

	// entry list getters
	const rgb_t *entry_list_raw() const { return &m_entry_color[0]; }
	const rgb_t *entry_list_adjusted() const { return &m_adjusted_color[0]; }
	const rgb_t *entry_list_adjusted_rgb15() const { return &m_adjusted_rgb15[0]; }

	// group adjustments
	void group_set_brightness(uint32_t group, float brightness);
	void group_set_contrast(uint32_t group, float contrast);

	// utilities
	void normalize_range(uint32_t start, uint32_t end, int lum_min = 0, int lum_max = 255);

private:
	// construction/destruction
	palette_t(uint32_t numcolors, uint32_t numgroups = 1);
	~palette_t();

	// internal helpers
	rgb_t adjust_palette_entry(rgb_t entry, float brightness, float contrast, const uint8_t *gamma_map);
	void update_adjusted_color(uint32_t group, uint32_t index);

	// internal state
	uint32_t          m_refcount;                   // reference count on the palette
	uint32_t          m_numcolors;                  // number of colors in the palette
	uint32_t          m_numgroups;                  // number of groups in the palette

	float           m_brightness;                 // overall brightness value
	float           m_contrast;                   // overall contrast value
	float           m_gamma;                      // overall gamma value
	uint8_t           m_gamma_map[256];             // gamma map

	std::vector<rgb_t> m_entry_color;           // array of raw colors
	std::vector<float> m_entry_contrast;        // contrast value for each entry
	std::vector<rgb_t> m_adjusted_color;        // array of adjusted colors
	std::vector<rgb_t> m_adjusted_rgb15;        // array of adjusted colors as RGB15

	std::vector<float> m_group_bright;          // brightness value for each group
	std::vector<float> m_group_contrast;        // contrast value for each group

	palette_client *m_client_list;                // list of clients for this palette
};



//**************************************************************************
//  INLINE FUNCTIONS
//**************************************************************************

//-------------------------------------------------
//  palexpand - expand a palette value to 8 bits
//-------------------------------------------------

template<int _NumBits>
inline uint8_t palexpand(uint8_t bits)
{
	if (_NumBits == 1) { return (bits & 1) ? 0xff : 0x00; }
	if (_NumBits == 2) { bits &= 3; return (bits << 6) | (bits << 4) | (bits << 2) | bits; }
	if (_NumBits == 3) { bits &= 7; return (bits << 5) | (bits << 2) | (bits >> 1); }
	if (_NumBits == 4) { bits &= 0xf; return (bits << 4) | bits; }
	if (_NumBits == 5) { bits &= 0x1f; return (bits << 3) | (bits >> 2); }
	if (_NumBits == 6) { bits &= 0x3f; return (bits << 2) | (bits >> 4); }
	if (_NumBits == 7) { bits &= 0x7f; return (bits << 1) | (bits >> 6); }
	return bits;
}


//-------------------------------------------------
//  palxbit - convert an x-bit value to 8 bits
//-------------------------------------------------

inline uint8_t pal1bit(uint8_t bits) { return palexpand<1>(bits); }
inline uint8_t pal2bit(uint8_t bits) { return palexpand<2>(bits); }
inline uint8_t pal3bit(uint8_t bits) { return palexpand<3>(bits); }
inline uint8_t pal4bit(uint8_t bits) { return palexpand<4>(bits); }
inline uint8_t pal5bit(uint8_t bits) { return palexpand<5>(bits); }
inline uint8_t pal6bit(uint8_t bits) { return palexpand<6>(bits); }
inline uint8_t pal7bit(uint8_t bits) { return palexpand<7>(bits); }


//-------------------------------------------------
//  rgbexpand - expand a 32-bit raw data to 8-bit
//  RGB
//-------------------------------------------------

template<int _RBits, int _GBits, int _BBits>
inline rgb_t rgbexpand(uint32_t data, uint8_t rshift, uint8_t gshift, uint8_t bshift)
{
	return rgb_t(palexpand<_RBits>(data >> rshift), palexpand<_GBits>(data >> gshift), palexpand<_BBits>(data >> bshift));
}


//-------------------------------------------------
//  palxxx - create an x-x-x color by extracting
//  bits from a uint32_t
//-------------------------------------------------

inline rgb_t pal332(uint32_t data, uint8_t rshift, uint8_t gshift, uint8_t bshift) { return rgbexpand<3,3,2>(data, rshift, gshift, bshift); }
inline rgb_t pal444(uint32_t data, uint8_t rshift, uint8_t gshift, uint8_t bshift) { return rgbexpand<4,4,4>(data, rshift, gshift, bshift); }
inline rgb_t pal555(uint32_t data, uint8_t rshift, uint8_t gshift, uint8_t bshift) { return rgbexpand<5,5,5>(data, rshift, gshift, bshift); }
inline rgb_t pal565(uint32_t data, uint8_t rshift, uint8_t gshift, uint8_t bshift) { return rgbexpand<5,6,5>(data, rshift, gshift, bshift); }
inline rgb_t pal888(uint32_t data, uint8_t rshift, uint8_t gshift, uint8_t bshift) { return rgbexpand<8,8,8>(data, rshift, gshift, bshift); }

#endif // MAME_UTIL_PALETTE_H
