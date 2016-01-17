// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria, Aaron Giles, Alex W. Jackson
/***************************************************************************

    digfx.h

    Device graphics interfaces.

***************************************************************************/

#pragma once

#ifndef __EMU_H__
#error Dont include this file directly; include emu.h instead.
#endif

#ifndef __DIGFX_H__
#define __DIGFX_H__



//**************************************************************************
//  CONSTANTS
//**************************************************************************

const int MAX_GFX_ELEMENTS = 32;
const int MAX_GFX_PLANES = 8;
const int MAX_GFX_SIZE = 32;



//**************************************************************************
//  GRAPHICS LAYOUT MACROS
//**************************************************************************

#define EXTENDED_XOFFS          { 0 }
#define EXTENDED_YOFFS          { 0 }

#define GFX_RAW                 0x12345678
#define GFXLAYOUT_RAW( name, width, height, linemod, charmod ) \
const gfx_layout name = { width, height, RGN_FRAC(1,1), 8, { GFX_RAW }, { 0 }, { linemod }, charmod };
// When planeoffset[0] is set to GFX_RAW, the gfx data is left as-is, with no conversion.
// No buffer is allocated for the decoded data, and gfxdata is set to point to the source
// data.
// yoffset[0] is the line modulo (*8) and charincrement the char modulo (*8). They are *8
// for consistency with the usual behaviour, but the bottom 3 bits are not used.
//
// This special mode can be used for graphics that are already in 8bpp linear format,
// or for unusual formats that don't fit our generic model and need to be decoded using
// custom code. See blend_gfx() in atarigen.c for an example of the latter usage.


// these macros describe gfx_layouts in terms of fractions of a region
// they can be used for total, planeoffset, xoffset, yoffset
#define RGN_FRAC(num,den)       (0x80000000 | (((num) & 0x0f) << 27) | (((den) & 0x0f) << 23))
#define IS_FRAC(offset)         ((offset) & 0x80000000)
#define FRAC_NUM(offset)        (((offset) >> 27) & 0x0f)
#define FRAC_DEN(offset)        (((offset) >> 23) & 0x0f)
#define FRAC_OFFSET(offset)     ((offset) & 0x007fffff)

// these macros are useful in gfx_layouts
#define STEP2(START,STEP)       (START),(START)+(STEP)
#define STEP4(START,STEP)       STEP2(START,STEP),STEP2((START)+2*(STEP),STEP)
#define STEP8(START,STEP)       STEP4(START,STEP),STEP4((START)+4*(STEP),STEP)
#define STEP16(START,STEP)      STEP8(START,STEP),STEP8((START)+8*(STEP),STEP)
#define STEP32(START,STEP)      STEP16(START,STEP),STEP16((START)+16*(STEP),STEP)
#define STEP64(START,STEP)      STEP32(START,STEP),STEP32((START)+32*(STEP),STEP)
#define STEP128(START,STEP)     STEP64(START,STEP),STEP64((START)+64*(STEP),STEP)
#define STEP256(START,STEP)     STEP128(START,STEP),STEP128((START)+128*(STEP),STEP)
#define STEP512(START,STEP)     STEP256(START,STEP),STEP256((START)+256*(STEP),STEP)
#define STEP1024(START,STEP)    STEP512(START,STEP),STEP512((START)+512*(STEP),STEP)
#define STEP2048(START,STEP)    STEP1024(START,STEP),STEP1024((START)+1024*(STEP),STEP)



//**************************************************************************
//  GRAPHICS INFO MACROS
//**************************************************************************

// optional horizontal and vertical scaling factors
#define GFXENTRY_XSCALEMASK   0x000000ff
#define GFXENTRY_YSCALEMASK   0x0000ff00
#define GFXENTRY_XSCALE(x)    ((((x)-1) << 0) & GFXENTRY_XSCALEMASK)
#define GFXENTRY_YSCALE(x)    ((((x)-1) << 8) & GFXENTRY_YSCALEMASK)
#define GFXENTRY_GETXSCALE(x) ((((x) & GFXENTRY_XSCALEMASK) >> 0) + 1)
#define GFXENTRY_GETYSCALE(x) ((((x) & GFXENTRY_YSCALEMASK) >> 8) + 1)

// GFXENTRY_RAM means region tag refers to a RAM share instead of a ROM region
#define GFXENTRY_ROM          0x00000000
#define GFXENTRY_RAM          0x00010000
#define GFXENTRY_ISROM(x)     (((x) & GFXENTRY_RAM) == 0)
#define GFXENTRY_ISRAM(x)     (((x) & GFXENTRY_RAM) != 0)

// GFXENTRY_DEVICE means region tag is relative to this device instead of its owner
#define GFXENTRY_DEVICE       0x00020000
#define GFXENTRY_ISDEVICE(x)  (((x) & GFXENTRY_DEVICE) != 0)

// GFXENTRY_REVERSE reverses the bit order in the layout (0-7 = LSB-MSB instead of MSB-LSB)
#define GFXENTRY_REVERSE      0x00040000
#define GFXENTRY_ISREVERSE(x) (((x) & GFXENTRY_REVERSE) != 0)


// these macros are used for declaring gfx_decode_entry info arrays
#define GFXDECODE_NAME( name ) gfxdecodeinfo_##name
#define GFXDECODE_EXTERN( name ) extern const gfx_decode_entry GFXDECODE_NAME(name)[]
#define GFXDECODE_START( name ) const gfx_decode_entry GFXDECODE_NAME(name)[] = {
#define GFXDECODE_END { 0 } };

// use these to declare a gfx_decode_entry array as a member of a device class
#define DECLARE_GFXDECODE_MEMBER( name ) static const gfx_decode_entry name[]
#define GFXDECODE_MEMBER( name ) const gfx_decode_entry name[] = {
// common gfx_decode_entry macros
#define GFXDECODE_ENTRYX(region,offset,layout,start,colors,flags) { region, offset, &layout, start, colors, flags },
#define GFXDECODE_ENTRY(region,offset,layout,start,colors) { region, offset, &layout, start, colors, 0 },

// specialized gfx_decode_entry macros
#define GFXDECODE_RAM(region,offset,layout,start,colors) { region, offset, &layout, start, colors, GFXENTRY_RAM },
#define GFXDECODE_DEVICE(region,offset,layout,start,colors) { region, offset, &layout, start, colors, GFXENTRY_DEVICE },
#define GFXDECODE_DEVICE_RAM(region,offset,layout,start,colors) { region, offset, &layout, start, colors, GFXENTRY_DEVICE | GFXENTRY_RAM },
#define GFXDECODE_SCALE(region,offset,layout,start,colors,x,y) { region, offset, &layout, start, colors, GFXENTRY_XSCALE(x) | GFXENTRY_YSCALE(y) },
#define GFXDECODE_REVERSEBITS(region,offset,layout,start,colors) { region, offset, &layout, start, colors, GFXENTRY_REVERSE },



//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_GFX_PALETTE(_palette_tag) \
	device_gfx_interface::static_set_palette(*device, _palette_tag);

#define MCFG_GFX_INFO(_info) \
	device_gfx_interface::static_set_info(*device, GFXDECODE_NAME(_info));



//**************************************************************************
//  DEVICE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_GFXDECODE_ADD(_tag, _palette_tag, _info) \
	MCFG_DEVICE_ADD(_tag, GFXDECODE, 0) \
	MCFG_GFX_PALETTE(_palette_tag) \
	MCFG_GFX_INFO(_info)

#define MCFG_GFXDECODE_MODIFY(_tag, _info) \
	MCFG_DEVICE_MODIFY(_tag) \
	MCFG_GFX_INFO(_info)



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// forward declarations
class gfx_element;
class palette_device;

struct gfx_layout
{
	UINT32 xoffs(int x) const { return (extxoffs != nullptr) ? extxoffs[x] : xoffset[x]; }
	UINT32 yoffs(int y) const { return (extyoffs != nullptr) ? extyoffs[y] : yoffset[y]; }

	UINT16          width;              // pixel width of each element
	UINT16          height;             // pixel height of each element
	UINT32          total;              // total number of elements, or RGN_FRAC()
	UINT16          planes;             // number of bitplanes
	UINT32          planeoffset[MAX_GFX_PLANES]; // bit offset of each bitplane
	UINT32          xoffset[MAX_GFX_SIZE]; // bit offset of each horizontal pixel
	UINT32          yoffset[MAX_GFX_SIZE]; // bit offset of each vertical pixel
	UINT32          charincrement;      // distance between two consecutive elements (in bits)
	const UINT32 *  extxoffs;           // extended X offset array for really big layouts
	const UINT32 *  extyoffs;           // extended Y offset array for really big layouts
};

struct gfx_decode_entry
{
	const char *    memory_region;      // memory region where the data resides
	UINT32          start;              // offset of beginning of data to decode
	const gfx_layout *gfxlayout;        // pointer to gfx_layout describing the layout; NULL marks the end of the array
	UINT16          color_codes_start;  // offset in the color lookup table where color codes start
	UINT16          total_color_codes;  // total number of color codes
	UINT32          flags;              // flags and optional scaling factors
};

// ======================> device_gfx_interface

class device_gfx_interface : public device_interface
{
public:
	// construction/destruction
	device_gfx_interface(const machine_config &mconfig, device_t &device,
						const gfx_decode_entry *gfxinfo = nullptr, const char *palette_tag = nullptr);
	virtual ~device_gfx_interface();

	// static configuration
	static void static_set_info(device_t &device, const gfx_decode_entry *gfxinfo);
	static void static_set_palette(device_t &device, std::string tag);

	// getters
	palette_device *palette() const { return m_palette; }
	gfx_element *gfx(int index) const { assert(index < MAX_GFX_ELEMENTS); return m_gfx[index].get(); }

	// decoding
	void decode_gfx(const gfx_decode_entry *gfxdecodeinfo);
	void decode_gfx() { decode_gfx(m_gfxdecodeinfo); }

	void set_gfx(int index, std::unique_ptr<gfx_element> &&element) { assert(index < MAX_GFX_ELEMENTS); m_gfx[index] = std::move(element); }

protected:
	// interface-level overrides
	virtual void interface_validity_check(validity_checker &valid) const override;
	virtual void interface_pre_start() override;
	virtual void interface_post_start() override;

	palette_device *            m_palette;                  // pointer to the palette device
	std::unique_ptr<gfx_element>  m_gfx[MAX_GFX_ELEMENTS];    // array of pointers to graphic sets

private:
	// configuration
	const gfx_decode_entry *    m_gfxdecodeinfo;        // pointer to array of gfx decode information
	std::string                 m_palette_tag;          // configured tag for palette device
	bool                        m_palette_is_sibling;   // is palette a sibling or a subdevice?

	// internal state
	bool                        m_decoded;                  // have we processed our decode info yet?
};

// iterator
typedef device_interface_iterator<device_gfx_interface> gfx_interface_iterator;


#endif  /* __DIGFX_H__ */
