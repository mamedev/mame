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
//  MACROS
//**************************************************************************

// macros to assemble rgb_t values */
#define MAKE_ARGB(a,r,g,b)  (((rgb_t(a) & 0xff) << 24) | ((rgb_t(r) & 0xff) << 16) | ((rgb_t(g) & 0xff) << 8) | (rgb_t(b) & 0xff))
#define MAKE_RGB(r,g,b)     (MAKE_ARGB(255,r,g,b))

// macros to extract components from rgb_t values */
#define RGB_ALPHA(rgb)      (((rgb) >> 24) & 0xff)
#define RGB_RED(rgb)        (((rgb) >> 16) & 0xff)
#define RGB_GREEN(rgb)      (((rgb) >> 8) & 0xff)
#define RGB_BLUE(rgb)       ((rgb) & 0xff)

// common colors */
#define RGB_BLACK           (MAKE_ARGB(255,0,0,0))
#define RGB_WHITE           (MAKE_ARGB(255,255,255,255))



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// an rgb_t is a single combined R,G,B (and optionally alpha) value */
typedef UINT32 rgb_t;

// an rgb15_t is a single combined 15-bit R,G,B value */
typedef UINT16 rgb15_t;

// forward definitions
class palette_t;

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
		dynamic_array<UINT32> m_dirty;          // bitmap of dirty entries
		UINT32          m_mindirty;             // minimum dirty entry
		UINT32          m_maxdirty;             // minimum dirty entry
	};

	// internal state
	palette_t &     m_palette;                  // reference to the palette
	palette_client *m_next;                     // pointer to next client
	dirty_state *   m_live;                     // live dirty state
	dirty_state *   m_previous;                 // previous dirty state
	dirty_state		m_dirty[2];					// two dirty states
};


// ======================> palette_t

// a palette object
class palette_t
{
	friend class palette_client;

public:
	// construction/destruction
	palette_t(UINT32 numcolors, UINT32 numgroups = 1);
	
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
	rgb_t entry_color(UINT32 index) const { return (index < m_numcolors) ? m_entry_color[index] : RGB_BLACK; }
	rgb_t entry_adjusted_color(UINT32 index) const { return (index < m_numcolors * m_numgroups) ? m_adjusted_color[index] : RGB_BLACK; }
	float entry_contrast(UINT32 index) const { return (index < m_numcolors) ? m_entry_contrast[index] : 1.0f; }

	// entry setters
	void entry_set_color(UINT32 index, rgb_t rgb);
	void entry_set_contrast(UINT32 index, float contrast);
	
	// entry list getters
	const rgb_t *entry_list_raw() const { return m_entry_color; }
	const rgb_t *entry_list_adjusted() const { return m_adjusted_color; }
	const rgb_t *entry_list_adjusted_rgb15() const { return m_adjusted_rgb15; }
	
	// group adjustments
	void group_set_brightness(UINT32 group, float brightness);
	void group_set_contrast(UINT32 group, float contrast);
	
	// utilities
	void normalize_range(UINT32 start, UINT32 end, int lum_min = 0, int lum_max = 255);
	
private:
	// destructor -- can only destruct via 
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

	dynamic_array<rgb_t> m_entry_color;                // array of raw colors
	dynamic_array<float> m_entry_contrast;             // contrast value for each entry
	dynamic_array<rgb_t> m_adjusted_color;             // array of adjusted colors
	dynamic_array<rgb_t> m_adjusted_rgb15;             // array of adjusted colors as RGB15

	dynamic_array<float> m_group_bright;               // brightness value for each group
	dynamic_array<float> m_group_contrast;             // contrast value for each group

	palette_client *m_client_list;                // list of clients for this palette
};



//**************************************************************************
//  INLINE FUNCTIONS
//**************************************************************************

//-------------------------------------------------
//  rgb_to_rgb15 - convert an RGB triplet to
//  a 15-bit OSD-specified RGB value
//-------------------------------------------------

inline rgb15_t rgb_to_rgb15(rgb_t rgb)
{
	return ((RGB_RED(rgb) >> 3) << 10) | ((RGB_GREEN(rgb) >> 3) << 5) | ((RGB_BLUE(rgb) >> 3) << 0);
}


//-------------------------------------------------
//  rgb_clamp - clamp an RGB component to 0-255
//-------------------------------------------------

inline UINT8 rgb_clamp(INT32 value)
{
	if (value < 0)
		return 0;
	if (value > 255)
		return 255;
	return value;
}


//-------------------------------------------------
//  pal1bit - convert a 1-bit value to 8 bits
//-------------------------------------------------

inline UINT8 pal1bit(UINT8 bits)
{
	return (bits & 1) ? 0xff : 0x00;
}


//-------------------------------------------------
//  pal2bit - convert a 2-bit value to 8 bits
//-------------------------------------------------

inline UINT8 pal2bit(UINT8 bits)
{
	bits &= 3;
	return (bits << 6) | (bits << 4) | (bits << 2) | bits;
}


//-------------------------------------------------
//  pal3bit - convert a 3-bit value to 8 bits
//-------------------------------------------------

inline UINT8 pal3bit(UINT8 bits)
{
	bits &= 7;
	return (bits << 5) | (bits << 2) | (bits >> 1);
}


//-------------------------------------------------
//  pal4bit - convert a 4-bit value to 8 bits
//-------------------------------------------------

inline UINT8 pal4bit(UINT8 bits)
{
	bits &= 0xf;
	return (bits << 4) | bits;
}


//-------------------------------------------------
//  pal5bit - convert a 5-bit value to 8 bits
//-------------------------------------------------

inline UINT8 pal5bit(UINT8 bits)
{
	bits &= 0x1f;
	return (bits << 3) | (bits >> 2);
}


//-------------------------------------------------
//  pal6bit - convert a 6-bit value to 8 bits
//-------------------------------------------------

inline UINT8 pal6bit(UINT8 bits)
{
	bits &= 0x3f;
	return (bits << 2) | (bits >> 4);
}


//-------------------------------------------------
//  pal7bit - convert a 7-bit value to 8 bits
//-------------------------------------------------

inline UINT8 pal7bit(UINT8 bits)
{
	bits &= 0x7f;
	return (bits << 1) | (bits >> 6);
}


//-------------------------------------------------
//  palexpand - expand a palette value to 8 bits
//-------------------------------------------------

template<int _NumBits>
inline UINT8 palexpand(UINT8 data)
{
	if (_NumBits == 1) return pal1bit(data);
	if (_NumBits == 2) return pal2bit(data);
	if (_NumBits == 3) return pal3bit(data);
	if (_NumBits == 4) return pal4bit(data);
	if (_NumBits == 5) return pal5bit(data);
	if (_NumBits == 6) return pal6bit(data);
	if (_NumBits == 7) return pal7bit(data);
	return data;
}


//-------------------------------------------------
//  pal332 - create a 3-3-2 color by extracting
//  bits from a UINT32
//-------------------------------------------------

inline rgb_t pal332(UINT32 data, UINT8 rshift, UINT8 gshift, UINT8 bshift)
{
	return MAKE_RGB(pal3bit(data >> rshift), pal3bit(data >> gshift), pal2bit(data >> bshift));
}


//-------------------------------------------------
//  pal444 - create a 4-4-4 color by extracting
//  bits from a UINT32
//-------------------------------------------------

inline rgb_t pal444(UINT32 data, UINT8 rshift, UINT8 gshift, UINT8 bshift)
{
	return MAKE_RGB(pal4bit(data >> rshift), pal4bit(data >> gshift), pal4bit(data >> bshift));
}


//-------------------------------------------------
//  pal555 - create a 5-5-5 color by extracting
//  bits from a UINT32
//-------------------------------------------------

inline rgb_t pal555(UINT32 data, UINT8 rshift, UINT8 gshift, UINT8 bshift)
{
	return MAKE_RGB(pal5bit(data >> rshift), pal5bit(data >> gshift), pal5bit(data >> bshift));
}


//-------------------------------------------------
//  pal565 - create a 5-6-5 color by extracting
//  bits from a UINT32
//-------------------------------------------------

inline rgb_t pal565(UINT32 data, UINT8 rshift, UINT8 gshift, UINT8 bshift)
{
	return MAKE_RGB(pal5bit(data >> rshift), pal6bit(data >> gshift), pal5bit(data >> bshift));
}


//-------------------------------------------------
//  pal888 - create a 8-8-8 color by extracting
//  bits from a UINT32
//-------------------------------------------------

inline rgb_t pal888(UINT32 data, UINT8 rshift, UINT8 gshift, UINT8 bshift)
{
	return MAKE_RGB(data >> rshift, data >> gshift, data >> bshift);
}


#endif  // __PALETTE_H__ */
