// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/******************************************************************************

    palette.h

    Core palette routines.

***************************************************************************/

#pragma once

#ifndef __PALETTE_H__
#define __PALETTE_H__

#include "osdcore.h"
#include "coretmpl.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// forward definitions
class palette_t;

// an rgb15_t is a single combined 15-bit R,G,B value
typedef UINT16 rgb15_t;


// ======================> rgb_t

// an rgb_t is a single combined R,G,B (and optionally alpha) value
class rgb_t
{
public:
	// construction/destruction
	rgb_t() { }
	rgb_t(UINT32 data) { m_data = data; }
	rgb_t(UINT8 r, UINT8 g, UINT8 b) { m_data = (255 << 24) | (r << 16) | (g << 8) | b; }
	rgb_t(UINT8 a, UINT8 r, UINT8 g, UINT8 b) { m_data = (a << 24) | (r << 16) | (g << 8) | b; }

	// getters
	UINT8 a() const { return m_data >> 24; }
	UINT8 r() const { return m_data >> 16; }
	UINT8 g() const { return m_data >> 8; }
	UINT8 b() const { return m_data >> 0; }
	rgb15_t as_rgb15() const { return ((r() >> 3) << 10) | ((g() >> 3) << 5) | ((b() >> 3) << 0); }
	UINT8 brightness() const { return (r() * 222 + g() * 707 + b() * 71) / 1000; }
	UINT32 const *ptr() const { return &m_data; }

	// setters
	rgb_t &set_a(UINT8 a) { m_data &= ~0xff000000; m_data |= a << 24; return *this; }
	rgb_t &set_r(UINT8 r) { m_data &= ~0x00ff0000; m_data |= r << 16; return *this; }
	rgb_t &set_g(UINT8 g) { m_data &= ~0x0000ff00; m_data |= g <<  8; return *this; }
	rgb_t &set_b(UINT8 b) { m_data &= ~0x000000ff; m_data |= b <<  0; return *this; }

	// implicit conversion operators
	operator UINT32() const { return m_data; }

	// operations
	rgb_t &scale8(UINT8 scale) { m_data = rgb_t(clamphi((a() * scale) >> 8), clamphi((r() * scale) >> 8), clamphi((g() * scale) >> 8), clamphi((b() * scale) >> 8)); return *this; }

	// assignment operators
	rgb_t &operator=(UINT32 rhs) { m_data = rhs; return *this; }
	rgb_t &operator+=(const rgb_t &rhs) { m_data = rgb_t(clamphi(a() + rhs.a()), clamphi(r() + rhs.r()), clamphi(g() + rhs.g()), clamphi(b() + rhs.b())); return *this; }
	rgb_t &operator-=(const rgb_t &rhs) { m_data = rgb_t(clamplo(a() - rhs.a()), clamplo(r() - rhs.r()), clamplo(g() - rhs.g()), clamplo(b() - rhs.b())); return *this; }

	// arithmetic operators
	const rgb_t operator+(const rgb_t &rhs) const { rgb_t result = *this; result += rhs; return result; }
	const rgb_t operator-(const rgb_t &rhs) const { rgb_t result = *this; result -= rhs; return result; }

	// static helpers
	static UINT8 clamp(INT32 value) { return (value < 0) ? 0 : (value > 255) ? 255 : value; }
	static UINT8 clamphi(INT32 value) { return (value > 255) ? 255 : value; }
	static UINT8 clamplo(INT32 value) { return (value < 0) ? 0 : value; }

	// constants
	static const rgb_t black;
	static const rgb_t white;
	static const rgb_t green;
	static const rgb_t amber;

private:
	UINT32  m_data;
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
	const UINT32 *dirty_list(UINT32 &mindirty, UINT32 &maxdirty);

	// dirty marking
	void mark_dirty(UINT32 index) { m_live->mark_dirty(index); }

private:
	// internal object to track dirty states
	class dirty_state
	{
	public:
		// construction
		dirty_state();

		// operations
		const UINT32 *dirty_list(UINT32 &mindirty, UINT32 &maxdirty);
		void resize(UINT32 colors);
		void mark_dirty(UINT32 index);
		void reset();

	private:
		// internal state
		std::vector<UINT32> m_dirty;          // bitmap of dirty entries
		UINT32          m_mindirty;             // minimum dirty entry
		UINT32          m_maxdirty;             // minimum dirty entry
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
	static palette_t *alloc(UINT32 numcolors, UINT32 numgroups = 1);

	// reference counting
	void ref() { m_refcount++; }
	void deref();

	// getters
	int num_colors() const { return m_numcolors; }
	int num_groups() const { return m_numgroups; }
	int max_index() const { return m_numcolors * m_numgroups + 2; }
	UINT32 black_entry() const { return m_numcolors * m_numgroups + 0; }
	UINT32 white_entry() const { return m_numcolors * m_numgroups + 1; }

	// overall adjustments
	void set_brightness(float brightness);
	void set_contrast(float contrast);
	void set_gamma(float gamma);

	// entry getters
	rgb_t entry_color(UINT32 index) const { return (index < m_numcolors) ? m_entry_color[index] : rgb_t::black; }
	rgb_t entry_adjusted_color(UINT32 index) const { return (index < m_numcolors * m_numgroups) ? m_adjusted_color[index] : rgb_t::black; }
	float entry_contrast(UINT32 index) const { return (index < m_numcolors) ? m_entry_contrast[index] : 1.0f; }

	// entry setters
	void entry_set_color(UINT32 index, rgb_t rgb);
	void entry_set_red_level(UINT32 index, UINT8 level);
	void entry_set_green_level(UINT32 index, UINT8 level);
	void entry_set_blue_level(UINT32 index, UINT8 level);
	void entry_set_contrast(UINT32 index, float contrast);

	// entry list getters
	const rgb_t *entry_list_raw() const { return &m_entry_color[0]; }
	const rgb_t *entry_list_adjusted() const { return &m_adjusted_color[0]; }
	const rgb_t *entry_list_adjusted_rgb15() const { return &m_adjusted_rgb15[0]; }

	// group adjustments
	void group_set_brightness(UINT32 group, float brightness);
	void group_set_contrast(UINT32 group, float contrast);

	// utilities
	void normalize_range(UINT32 start, UINT32 end, int lum_min = 0, int lum_max = 255);

private:
	// construction/destruction
	palette_t(UINT32 numcolors, UINT32 numgroups = 1);
	~palette_t();

	// internal helpers
	rgb_t adjust_palette_entry(rgb_t entry, float brightness, float contrast, const UINT8 *gamma_map);
	void update_adjusted_color(UINT32 group, UINT32 index);

	// internal state
	UINT32          m_refcount;                   // reference count on the palette
	UINT32          m_numcolors;                  // number of colors in the palette
	UINT32          m_numgroups;                  // number of groups in the palette

	float           m_brightness;                 // overall brightness value
	float           m_contrast;                   // overall contrast value
	float           m_gamma;                      // overall gamma value
	UINT8           m_gamma_map[256];             // gamma map

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
inline UINT8 palexpand(UINT8 bits)
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

inline UINT8 pal1bit(UINT8 bits) { return palexpand<1>(bits); }
inline UINT8 pal2bit(UINT8 bits) { return palexpand<2>(bits); }
inline UINT8 pal3bit(UINT8 bits) { return palexpand<3>(bits); }
inline UINT8 pal4bit(UINT8 bits) { return palexpand<4>(bits); }
inline UINT8 pal5bit(UINT8 bits) { return palexpand<5>(bits); }
inline UINT8 pal6bit(UINT8 bits) { return palexpand<6>(bits); }
inline UINT8 pal7bit(UINT8 bits) { return palexpand<7>(bits); }


//-------------------------------------------------
//  rgbexpand - expand a 32-bit raw data to 8-bit
//  RGB
//-------------------------------------------------

template<int _RBits, int _GBits, int _BBits>
inline rgb_t rgbexpand(UINT32 data, UINT8 rshift, UINT8 gshift, UINT8 bshift)
{
	return rgb_t(palexpand<_RBits>(data >> rshift), palexpand<_GBits>(data >> gshift), palexpand<_BBits>(data >> bshift));
}


//-------------------------------------------------
//  palxxx - create an x-x-x color by extracting
//  bits from a UINT32
//-------------------------------------------------

inline rgb_t pal332(UINT32 data, UINT8 rshift, UINT8 gshift, UINT8 bshift) { return rgbexpand<3,3,2>(data, rshift, gshift, bshift); }
inline rgb_t pal444(UINT32 data, UINT8 rshift, UINT8 gshift, UINT8 bshift) { return rgbexpand<4,4,4>(data, rshift, gshift, bshift); }
inline rgb_t pal555(UINT32 data, UINT8 rshift, UINT8 gshift, UINT8 bshift) { return rgbexpand<5,5,5>(data, rshift, gshift, bshift); }
inline rgb_t pal565(UINT32 data, UINT8 rshift, UINT8 gshift, UINT8 bshift) { return rgbexpand<5,6,5>(data, rshift, gshift, bshift); }
inline rgb_t pal888(UINT32 data, UINT8 rshift, UINT8 gshift, UINT8 bshift) { return rgbexpand<8,8,8>(data, rshift, gshift, bshift); }


#endif  // __PALETTE_H__
