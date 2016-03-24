// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    emupal.h

    Palette device.

****************************************************************************

    There are several levels of abstraction in the way MAME handles the palette,
    and several display modes which can be used by the drivers.

    Palette
    -------
    Note: in the following text, "color" refers to a color in the emulated
    game's virtual palette. For example, a game might be able to display 1024
    colors at the same time. If the game uses RAM to change the available
    colors, the term "palette" refers to the colors available at any given time,
    not to the whole range of colors which can be produced by the hardware. The
    latter is referred to as "color space".
    The term "pen" refers to one of the maximum MAX_PENS colors that can be
    used to generate the display.

    So, to summarize, the three layers of palette abstraction are:

    P1) The game virtual palette (the "colors")
    P2) MAME's MAX_PENS colors palette (the "pens")
    P3) The OS specific hardware color registers (the "OS specific pens")

    The array Machine->pens[] is a lookup table which maps game colors to OS
    specific pens (P1 to P3). When you are working on bitmaps at the pixel level,
    *always* use Machine->pens to map the color numbers. *Never* use constants.
    For example if you want to make pixel (x,y) of color 3, do:
    *BITMAP_ADDR(bitmap, <type>, y, x) = Machine->pens[3];


    Lookup table
    ------------
    Palettes are only half of the story. To map the gfx data to colors, the
    graphics routines use a lookup table. For example if we have 4bpp tiles,
    which can have 256 different color codes, the lookup table for them will have
    256 * 2^4 = 4096 elements. For games using a palette RAM, the lookup table is
    usually a 1:1 map. For games using PROMs, the lookup table is often larger
    than the palette itself so for example the game can display 256 colors out
    of a palette of 16.

    The palette and the lookup table are initialized to default values by the
    main core, but can be initialized by the driver using the function
    MachineDriver->vh_init_palette(). For games using palette RAM, that
    function is usually not needed, and the lookup table can be set up by
    properly initializing the color_codes_start and total_color_codes fields in
    the GfxDecodeInfo array.
    When vh_init_palette() initializes the lookup table, it maps gfx codes
    to game colors (P1 above). The lookup table will be converted by the core to
    map to OS specific pens (P3 above), and stored in Machine->remapped_colortable.


    Display modes
    -------------
    The available display modes can be summarized in three categories:
    1) Static palette. Use this for games which use PROMs for color generation.
        The palette is initialized by palette_init(), and never changed
        again.
    2) Dynamic palette. Use this for games which use RAM for color generation.
        The palette can be dynamically modified by the driver using the function
        palette_set_color().
    3) Direct mapped 16-bit or 32-bit color. This should only be used in special
        cases, e.g. to support alpha blending.
        MachineDriver->video_attributes must contain VIDEO_RGB_DIRECT.


    Shadows(Highlights) Quick Reference
    -----------------------------------

    1) declare MCFG_VIDEO_ATTRIBUTES( ... )

    2) make a pen table fill with DRAWMODE_NONE, DRAWMODE_SOURCE or DRAWMODE_SHADOW

    3) (optional) set shadow darkness or highlight brightness by
        set_shadow_factor(0.0-1.0) or
        _set_highlight_factor(1.0-n.n)

    4) before calling drawgfx use
        palette_set_shadow_mode(0) to arm shadows or
        palette_set_shadow_mode(1) to arm highlights

    5) call drawgfx_transtable
        drawgfx_transtable( ..., pentable )

******************************************************************************/

#pragma once

#ifndef __EMU_H__
#error Dont include this file directly; include emu.h instead.
#endif

#ifndef __EMUPAL_H__
#define __EMUPAL_H__


//**************************************************************************
//  CONSTANTS
//**************************************************************************

#define PALETTE_DEFAULT_SHADOW_FACTOR (0.6)
#define PALETTE_DEFAULT_HIGHLIGHT_FACTOR (1/PALETTE_DEFAULT_SHADOW_FACTOR)

#define PALETTE_INIT_NAME(_Name) palette_init_##_Name
#define DECLARE_PALETTE_INIT(_Name) void PALETTE_INIT_NAME(_Name)(palette_device &palette)
#define PALETTE_INIT_MEMBER(_Class, _Name) void _Class::PALETTE_INIT_NAME(_Name)(palette_device &palette)

#define PALETTE_DECODER_NAME(_Name) _Name##_decoder
#define DECLARE_PALETTE_DECODER(_Name) static rgb_t PALETTE_DECODER_NAME(_Name)(UINT32 raw)
#define PALETTE_DECODER_MEMBER(_Class, _Name) rgb_t _Class::PALETTE_DECODER_NAME(_Name)(UINT32 raw)

// standard 3-3-2 formats
#define PALETTE_FORMAT_BBGGGRRR raw_to_rgb_converter(1, &raw_to_rgb_converter::standard_rgb_decoder<3,3,2, 0,3,6>)
#define PALETTE_FORMAT_RRRGGGBB raw_to_rgb_converter(1, &raw_to_rgb_converter::standard_rgb_decoder<3,3,2, 5,2,0>)

// data-inverted 3-3-2 formats
#define PALETTE_FORMAT_BBGGGRRR_inverted raw_to_rgb_converter(1, &raw_to_rgb_converter::inverted_rgb_decoder<3,3,2, 0,3,6>)
#define PALETTE_FORMAT_RRRGGGBB_inverted raw_to_rgb_converter(1, &raw_to_rgb_converter::inverted_rgb_decoder<3,3,2, 5,2,0>)

// standard 3-3-3 formats
#define PALETTE_FORMAT_xxxxxxxBBBGGGRRR raw_to_rgb_converter(2, &raw_to_rgb_converter::standard_rgb_decoder<3,3,3, 0,3,6>)
#define PALETTE_FORMAT_xxxxxxxRRRBBBGGG raw_to_rgb_converter(2, &raw_to_rgb_converter::standard_rgb_decoder<3,3,3, 6,0,3>)
#define PALETTE_FORMAT_xxxxxxxRRRGGGBBB raw_to_rgb_converter(2, &raw_to_rgb_converter::standard_rgb_decoder<3,3,3, 6,3,0>)

// standard 4-4-4 formats
#define PALETTE_FORMAT_xxxxBBBBGGGGRRRR raw_to_rgb_converter(2, &raw_to_rgb_converter::standard_rgb_decoder<4,4,4, 0,4,8>)
#define PALETTE_FORMAT_xxxxBBBBRRRRGGGG raw_to_rgb_converter(2, &raw_to_rgb_converter::standard_rgb_decoder<4,4,4, 4,0,8>)
#define PALETTE_FORMAT_xxxxRRRRBBBBGGGG raw_to_rgb_converter(2, &raw_to_rgb_converter::standard_rgb_decoder<4,4,4, 8,0,4>)
#define PALETTE_FORMAT_xxxxRRRRGGGGBBBB raw_to_rgb_converter(2, &raw_to_rgb_converter::standard_rgb_decoder<4,4,4, 8,4,0>)
#define PALETTE_FORMAT_RRRRGGGGBBBBxxxx raw_to_rgb_converter(2, &raw_to_rgb_converter::standard_rgb_decoder<4,4,4, 12,8,4>)
#define PALETTE_FORMAT_GGGGBBBBRRRRxxxx raw_to_rgb_converter(2, &raw_to_rgb_converter::standard_rgb_decoder<4,4,4, 4,12,8>)

// standard 4-4-4-4 formats
#define PALETTE_FORMAT_IIIIRRRRGGGGBBBB raw_to_rgb_converter(2, &raw_to_rgb_converter::standard_irgb_decoder<4,4,4,4, 12,8,4,0>)
#define PALETTE_FORMAT_RRRRGGGGBBBBIIII raw_to_rgb_converter(2, &raw_to_rgb_converter::standard_irgb_decoder<4,4,4,4, 0,12,8,4>)

// standard 5-5-5 formats
#define PALETTE_FORMAT_xBBBBBGGGGGRRRRR raw_to_rgb_converter(2, &raw_to_rgb_converter::standard_rgb_decoder<5,5,5, 0,5,10>)
#define PALETTE_FORMAT_xBBBBBRRRRRGGGGG raw_to_rgb_converter(2, &raw_to_rgb_converter::standard_rgb_decoder<5,5,5, 5,0,10>)
#define PALETTE_FORMAT_xRRRRRGGGGGBBBBB raw_to_rgb_converter(2, &raw_to_rgb_converter::standard_rgb_decoder<5,5,5, 10,5,0>)
#define PALETTE_FORMAT_xGGGGGRRRRRBBBBB raw_to_rgb_converter(2, &raw_to_rgb_converter::standard_rgb_decoder<5,5,5, 5,10,0>)
#define PALETTE_FORMAT_xGGGGGBBBBBRRRRR raw_to_rgb_converter(2, &raw_to_rgb_converter::standard_rgb_decoder<5,5,5, 0,10,5>)
#define PALETTE_FORMAT_BBBBBRRRRRGGGGGx raw_to_rgb_converter(2, &raw_to_rgb_converter::standard_rgb_decoder<5,5,5, 6,1,11>)
#define PALETTE_FORMAT_GGGGGRRRRRBBBBBx raw_to_rgb_converter(2, &raw_to_rgb_converter::standard_rgb_decoder<5,5,5, 6,11,1>)
#define PALETTE_FORMAT_RRRRRGGGGGBBBBBx raw_to_rgb_converter(2, &raw_to_rgb_converter::standard_rgb_decoder<5,5,5, 11,6,1>)
#define PALETTE_FORMAT_RRRRGGGGBBBBRGBx raw_to_rgb_converter(2, &raw_to_rgb_converter::RRRRGGGGBBBBRGBx_decoder)
#define PALETTE_FORMAT_xRGBRRRRGGGGBBBB_bit0 raw_to_rgb_converter(2, &raw_to_rgb_converter::xRGBRRRRGGGGBBBB_bit0_decoder)
#define PALETTE_FORMAT_xRGBRRRRGGGGBBBB_bit4 raw_to_rgb_converter(2, &raw_to_rgb_converter::xRGBRRRRGGGGBBBB_bit4_decoder)

// data-inverted 5-5-5 formats
#define PALETTE_FORMAT_xRRRRRBBBBBGGGGG_inverted raw_to_rgb_converter(2, &raw_to_rgb_converter::inverted_rgb_decoder<5,5,5, 10,0,5>)

// standard 5-6-5 formats
#define PALETTE_FORMAT_RRRRRGGGGGGBBBBB raw_to_rgb_converter(2, &raw_to_rgb_converter::standard_rgb_decoder<5,6,5, 11,5,0>)
#define PALETTE_FORMAT_BBBBBGGGGGGRRRRR raw_to_rgb_converter(2, &raw_to_rgb_converter::standard_rgb_decoder<5,6,5, 0,5,11>)

// standard 5-5-5-1 formats
#define PALETTE_FORMAT_IRRRRRGGGGGBBBBB raw_to_rgb_converter(2, &raw_to_rgb_converter::IRRRRRGGGGGBBBBB_decoder)

// standard 8-8-8 formats
#define PALETTE_FORMAT_XRGB raw_to_rgb_converter(4, &raw_to_rgb_converter::standard_rgb_decoder<8,8,8, 16,8,0>)
#define PALETTE_FORMAT_XBGR raw_to_rgb_converter(4, &raw_to_rgb_converter::standard_rgb_decoder<8,8,8, 0,8,16>)
#define PALETTE_FORMAT_XBRG raw_to_rgb_converter(4, &raw_to_rgb_converter::standard_rgb_decoder<8,8,8, 8,0,16>)
#define PALETTE_FORMAT_XGRB raw_to_rgb_converter(4, &raw_to_rgb_converter::standard_rgb_decoder<8,8,8, 8,16,0>)
#define PALETTE_FORMAT_RGBX raw_to_rgb_converter(4, &raw_to_rgb_converter::standard_rgb_decoder<8,8,8, 24,16,8>)
#define PALETTE_FORMAT_GRBX raw_to_rgb_converter(4, &raw_to_rgb_converter::standard_rgb_decoder<8,8,8, 16,24,8>)



//**************************************************************************
//  DEVICE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_PALETTE_ADD(_tag, _entries) \
	MCFG_DEVICE_ADD(_tag, PALETTE, 0) \
	MCFG_PALETTE_ENTRIES(_entries)
#define MCFG_PALETTE_ADD_INIT_BLACK(_tag, _entries) \
	MCFG_PALETTE_ADD(_tag, _entries) \
	palette_device::static_set_init(*device, palette_init_delegate(FUNC(palette_device::palette_init_all_black), downcast<palette_device *>(device)));

#define MCFG_PALETTE_MODIFY MCFG_DEVICE_MODIFY


#define MCFG_PALETTE_INIT_OWNER(_class, _method) \
	palette_device::static_set_init(*device, palette_init_delegate(&_class::PALETTE_INIT_NAME(_method), #_class "::palette_init_" #_method, downcast<_class *>(owner)));
#define MCFG_PALETTE_INIT_DEVICE(_tag, _class, _method) \
	palette_device::static_set_init(*device, palette_init_delegate(&_class::PALETTE_INIT_NAME(_method), #_class "::palette_init_" #_method, _tag));

#define MCFG_PALETTE_FORMAT(_format) \
	palette_device::static_set_format(*device, PALETTE_FORMAT_##_format);

#define MCFG_PALETTE_FORMAT_CLASS(_bytes_per_entry, _class, _method) \
	palette_device::static_set_format(*device, raw_to_rgb_converter(_bytes_per_entry, &_class::PALETTE_DECODER_NAME(_method)));

#define MCFG_PALETTE_MEMBITS(_width) \
	palette_device::static_set_membits(*device, _width);

#define MCFG_PALETTE_ENDIANNESS(_endianness) \
	palette_device::static_set_endianness(*device, _endianness);

#define MCFG_PALETTE_ENTRIES(_entries) \
	palette_device::static_set_entries(*device, _entries);

#define MCFG_PALETTE_INDIRECT_ENTRIES(_entries) \
	palette_device::static_set_indirect_entries(*device, _entries);

#define MCFG_PALETTE_ENABLE_SHADOWS() \
	palette_device::static_enable_shadows(*device);

#define MCFG_PALETTE_ENABLE_HILIGHTS() \
	palette_device::static_enable_hilights(*device);


// monochrome palettes
#define MCFG_PALETTE_ADD_MONOCHROME(_tag) \
	MCFG_PALETTE_ADD(_tag, 2) \
	palette_device::static_set_init(*device, palette_init_delegate(FUNC(palette_device::palette_init_monochrome), downcast<palette_device *>(device)));

#define MCFG_PALETTE_ADD_MONOCHROME_INVERTED(_tag) \
	MCFG_PALETTE_ADD(_tag, 2) \
	palette_device::static_set_init(*device, palette_init_delegate(FUNC(palette_device::palette_init_monochrome_inverted), downcast<palette_device *>(device)));

#define MCFG_PALETTE_ADD_MONOCHROME_HIGHLIGHT(_tag) \
	MCFG_PALETTE_ADD(_tag, 3) \
	palette_device::static_set_init(*device, palette_init_delegate(FUNC(palette_device::palette_init_monochrome_highlight), downcast<palette_device *>(device)));

// 8-bit palettes
#define MCFG_PALETTE_ADD_3BIT_RGB(_tag) \
	MCFG_PALETTE_ADD(_tag, 8) \
	palette_device::static_set_init(*device, palette_init_delegate(FUNC(palette_device::palette_init_3bit_rgb), downcast<palette_device *>(device)));

#define MCFG_PALETTE_ADD_3BIT_RBG(_tag) \
	MCFG_PALETTE_ADD(_tag, 8) \
	palette_device::static_set_init(*device, palette_init_delegate(FUNC(palette_device::palette_init_3bit_rbg), downcast<palette_device *>(device)));

#define MCFG_PALETTE_ADD_3BIT_BRG(_tag) \
	MCFG_PALETTE_ADD(_tag, 8) \
	palette_device::static_set_init(*device, palette_init_delegate(FUNC(palette_device::palette_init_3bit_brg), downcast<palette_device *>(device)));

#define MCFG_PALETTE_ADD_3BIT_GRB(_tag) \
	MCFG_PALETTE_ADD(_tag, 8) \
	palette_device::static_set_init(*device, palette_init_delegate(FUNC(palette_device::palette_init_3bit_grb), downcast<palette_device *>(device)));

#define MCFG_PALETTE_ADD_3BIT_GBR(_tag) \
	MCFG_PALETTE_ADD(_tag, 8) \
	palette_device::static_set_init(*device, palette_init_delegate(FUNC(palette_device::palette_init_3bit_gbr), downcast<palette_device *>(device)));

#define MCFG_PALETTE_ADD_3BIT_BGR(_tag) \
	MCFG_PALETTE_ADD(_tag, 8) \
	palette_device::static_set_init(*device, palette_init_delegate(FUNC(palette_device::palette_init_3bit_bgr), downcast<palette_device *>(device)));

// 15-bit palettes
#define MCFG_PALETTE_ADD_RRRRRGGGGGBBBBB(_tag) \
	MCFG_PALETTE_ADD(_tag, 32768) \
	palette_device::static_set_init(*device, palette_init_delegate(FUNC(palette_device::palette_init_RRRRRGGGGGBBBBB), downcast<palette_device *>(device)));

// 16-bit palettes
#define MCFG_PALETTE_ADD_BBBBBGGGGGRRRRR(_tag) \
	MCFG_PALETTE_ADD(_tag, 32768) \
	palette_device::static_set_init(*device, palette_init_delegate(FUNC(palette_device::palette_init_BBBBBGGGGGRRRRR), downcast<palette_device *>(device)));

#define MCFG_PALETTE_ADD_RRRRRGGGGGGBBBBB(_tag) \
	MCFG_PALETTE_ADD(_tag, 65536) \
	palette_device::static_set_init(*device, palette_init_delegate(FUNC(palette_device::palette_init_RRRRRGGGGGGBBBBB), downcast<palette_device *>(device)));


// other standard palettes
#define MCFG_PALETTE_ADD_RRRRGGGGBBBB_PROMS(_tag, _entries) \
	MCFG_PALETTE_ADD(_tag, _entries) \
	palette_device::static_set_init(*device, palette_init_delegate(FUNC(palette_device::palette_init_RRRRGGGGBBBB_proms), downcast<palette_device *>(device)));

// not implemented yet
#if 0
#define MCFG_PALETTE_ADD_HARDCODED(_tag, _array) \
	MCFG_PALETTE_ADD(_tag, sizeof(_array) / 3) \
	palette_device::static_set_init(*device, palette_init_delegate(FUNC(palette_device::palette_init_RRRRGGGGBBBB_proms), downcast<palette_device *>(device)));
#endif



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

typedef device_delegate<void (palette_device &)> palette_init_delegate;


// ======================> raw_to_rgb_converter

class raw_to_rgb_converter
{
	// helper function
	typedef rgb_t (*raw_to_rgb_func)(UINT32 raw);

public:
	// constructor
	raw_to_rgb_converter(int bytes_per_entry = 0, raw_to_rgb_func func = nullptr)
		: m_bytes_per_entry(bytes_per_entry),
			m_func(func) { }

	// getters
	int bytes_per_entry() const { return m_bytes_per_entry; }

	// helpers
	rgb_t operator()(UINT32 raw) const { return (*m_func)(raw); }

	// generic raw-to-RGB conversion helpers
	template<int _RedBits, int _GreenBits, int _BlueBits, int _RedShift, int _GreenShift, int _BlueShift>
	static rgb_t standard_rgb_decoder(UINT32 raw)
	{
		UINT8 r = palexpand<_RedBits>(raw >> _RedShift);
		UINT8 g = palexpand<_GreenBits>(raw >> _GreenShift);
		UINT8 b = palexpand<_BlueBits>(raw >> _BlueShift);
		return rgb_t(r, g, b);
	}

	// data-inverted generic raw-to-RGB conversion helpers
	template<int _RedBits, int _GreenBits, int _BlueBits, int _RedShift, int _GreenShift, int _BlueShift>
	static rgb_t inverted_rgb_decoder(UINT32 raw)
	{
		UINT8 r = palexpand<_RedBits>(~raw >> _RedShift);
		UINT8 g = palexpand<_GreenBits>(~raw >> _GreenShift);
		UINT8 b = palexpand<_BlueBits>(~raw >> _BlueShift);
		return rgb_t(r, g, b);
	}

	template<int _IntBits, int _RedBits, int _GreenBits, int _BlueBits, int _IntShift, int _RedShift, int _GreenShift, int _BlueShift>
	static rgb_t standard_irgb_decoder(UINT32 raw)
	{
		UINT8 i = palexpand<_IntBits>(raw >> _IntShift);
		UINT8 r = (i * palexpand<_RedBits>(raw >> _RedShift)) >> 8;
		UINT8 g = (i * palexpand<_GreenBits>(raw >> _GreenShift)) >> 8;
		UINT8 b = (i * palexpand<_BlueBits>(raw >> _BlueShift)) >> 8;
		return rgb_t(r, g, b);
	}

	// other standard decoders
	static rgb_t IRRRRRGGGGGBBBBB_decoder(UINT32 raw);
	static rgb_t RRRRGGGGBBBBRGBx_decoder(UINT32 raw);  // bits 3/2/1 are LSb
	static rgb_t xRGBRRRRGGGGBBBB_bit0_decoder(UINT32 raw);  // bits 14/13/12 are LSb
	static rgb_t xRGBRRRRGGGGBBBB_bit4_decoder(UINT32 raw);  // bits 14/13/12 are MSb

private:
	// internal data
	int                 m_bytes_per_entry;
	raw_to_rgb_func     m_func;
};


// ======================> palette_device

// device type definition
extern const device_type PALETTE;

class palette_device :  public device_t
{
	static const int MAX_SHADOW_PRESETS = 4;

public:
	// construction/destruction
	palette_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// static configuration
	static void static_set_init(device_t &device, palette_init_delegate init);
	static void static_set_format(device_t &device, raw_to_rgb_converter raw_to_rgb);
	static void static_set_membits(device_t &device, int membits);
	static void static_set_endianness(device_t &device, endianness_t endianness);
	static void static_set_entries(device_t &device, int entries);
	static void static_set_indirect_entries(device_t &device, int entries);
	static void static_enable_shadows(device_t &device);
	static void static_enable_hilights(device_t &device);

	// getters
	int entries() const { return m_entries; }
	int indirect_entries() const { return m_indirect_entries; }
	palette_t *palette() const { return m_palette; }
	const pen_t &pen(int index) const { return m_pens[index]; }
	const pen_t *pens() const { return m_pens; }
	pen_t *shadow_table() const { return m_shadow_table; }
	rgb_t pen_color(pen_t pen) { return m_palette->entry_color(pen); }
	double pen_contrast(pen_t pen) { return m_palette->entry_contrast(pen); }
	pen_t black_pen() const { return m_black_pen; }
	pen_t white_pen() const { return m_white_pen; }
	memory_array &basemem() { return m_paletteram; }
	memory_array &extmem() { return m_paletteram_ext; }
	bool shadows_enabled() { return m_enable_shadows; }
	bool hilights_enabled() { return m_enable_hilights; }

	// setters
	void set_pen_color(pen_t pen, rgb_t rgb) { m_palette->entry_set_color(pen, rgb); }
	void set_pen_red_level(pen_t pen, UINT8 level) { m_palette->entry_set_red_level(pen, level); }
	void set_pen_green_level(pen_t pen, UINT8 level) { m_palette->entry_set_green_level(pen, level); }
	void set_pen_blue_level(pen_t pen, UINT8 level) { m_palette->entry_set_blue_level(pen, level); }
	void set_pen_color(pen_t pen, UINT8 r, UINT8 g, UINT8 b) { m_palette->entry_set_color(pen, rgb_t(r, g, b)); }
	void set_pen_colors(pen_t color_base, const rgb_t *colors, int color_count) { while (color_count--) set_pen_color(color_base++, *colors++); }
	void set_pen_colors(pen_t color_base, const std::vector<rgb_t> &colors) { for(unsigned int i=0; i != colors.size(); i++) set_pen_color(color_base+i, colors[i]); }
	void set_pen_contrast(pen_t pen, double bright) { m_palette->entry_set_contrast(pen, bright); }

	// indirection (aka colortables)
	UINT16 pen_indirect(int index) const { return m_indirect_pens[index]; }
	rgb_t indirect_color(int index) const { return m_indirect_colors[index]; }
	void set_indirect_color(int index, rgb_t rgb);
	void set_pen_indirect(pen_t pen, UINT16 index);
	UINT32 transpen_mask(gfx_element &gfx, int color, int transcolor);

	// shadow config
	void set_shadow_factor(double factor) { assert(m_shadow_group != 0); m_palette->group_set_contrast(m_shadow_group, factor); }
	void set_highlight_factor(double factor) { assert(m_hilight_group != 0); m_palette->group_set_contrast(m_hilight_group, factor); }
	void set_shadow_mode(int mode) { assert(mode >= 0 && mode < MAX_SHADOW_PRESETS); m_shadow_table = m_shadow_tables[mode].base; }

	// generic read/write handlers
	DECLARE_READ8_MEMBER(read);
	DECLARE_WRITE8_MEMBER(write);
	DECLARE_WRITE8_MEMBER(write_ext);
	DECLARE_WRITE8_MEMBER(write_indirect);
	DECLARE_WRITE8_MEMBER(write_indirect_ext);
	DECLARE_READ16_MEMBER(read);
	DECLARE_WRITE16_MEMBER(write);
	DECLARE_WRITE16_MEMBER(write_ext);
	DECLARE_READ32_MEMBER(read);
	DECLARE_WRITE32_MEMBER(write);

	// generic palette init routines
	void palette_init_all_black(palette_device &palette);
	void palette_init_monochrome(palette_device &palette);
	void palette_init_monochrome_inverted(palette_device &palette);
	void palette_init_monochrome_highlight(palette_device &palette);
	void palette_init_3bit_rgb(palette_device &palette);
	void palette_init_3bit_rbg(palette_device &palette);
	void palette_init_3bit_brg(palette_device &palette);
	void palette_init_3bit_grb(palette_device &palette);
	void palette_init_3bit_gbr(palette_device &palette);
	void palette_init_3bit_bgr(palette_device &palette);
	void palette_init_RRRRGGGGBBBB_proms(palette_device &palette);
	void palette_init_RRRRRGGGGGBBBBB(palette_device &palette);
	void palette_init_BBBBBGGGGGRRRRR(palette_device &palette);
	void palette_init_RRRRRGGGGGGBBBBB(palette_device &palette);

	// helper to update palette when data changed
	void update() { if (!m_init.isnull()) m_init(*this); }
protected:
	// device-level overrides
	virtual void device_validity_check(validity_checker &valid) const override;
	virtual void device_start() override;
	virtual void device_pre_save() override;
	virtual void device_post_load() override;
	virtual void device_stop() override;

	void allocate_palette();
	void allocate_color_tables();
	void allocate_shadow_tables();

	void update_for_write(offs_t byte_offset, int bytes_modified, bool indirect = false);
public: // needed by konamigx
	void set_shadow_dRGB32(int mode, int dr, int dg, int db, bool noclip);
protected:
	void configure_rgb_shadows(int mode, float factor);

private:
	// configuration state
	int                 m_entries;              // number of entries in the palette
	int                 m_indirect_entries;     // number of indirect colors in the palette
	bool                m_enable_shadows;       // are shadows enabled?
	bool                m_enable_hilights;      // are hilights enabled?
	int                 m_membits;              // width of palette RAM, if different from native
	bool                m_membits_supplied;     // true if membits forced in static config
	endianness_t        m_endianness;           // endianness of palette RAM, if different from native
	bool                m_endianness_supplied;  // true if endianness forced in static config

	// palette RAM
	raw_to_rgb_converter m_raw_to_rgb;          // format of palette RAM
	memory_array        m_paletteram;           // base memory
	memory_array        m_paletteram_ext;       // extended memory

	// internal state
	palette_t *         m_palette;              // the palette itself
	const pen_t *       m_pens;                 // remapped palette pen numbers
	bitmap_format       m_format;               // format assumed for palette data
	pen_t *             m_shadow_table;         // table for looking up a shadowed pen
	UINT32              m_shadow_group;         // index of the shadow group, or 0 if none
	UINT32              m_hilight_group;        // index of the hilight group, or 0 if none
	pen_t               m_white_pen;            // precomputed white pen value
	pen_t               m_black_pen;            // precomputed black pen value

	// indirection state
	std::vector<rgb_t> m_indirect_colors;     // actual colors set for indirection
	std::vector<UINT16> m_indirect_pens;      // indirection values

	struct shadow_table_data
	{
		pen_t *            base;               // pointer to the base of the table
		INT16              dr;                 // delta red value
		INT16              dg;                 // delta green value
		INT16              db;                 // delta blue value
		bool               noclip;             // clip?
	};
	shadow_table_data   m_shadow_tables[MAX_SHADOW_PRESETS]; // array of shadow table data

	std::vector<pen_t> m_save_pen;           // pens for save/restore
	std::vector<float> m_save_contrast;      // brightness for save/restore

	std::vector<pen_t> m_pen_array;
	std::vector<pen_t> m_shadow_array;
	std::vector<pen_t> m_hilight_array;
	palette_init_delegate m_init;
};

// device type iterator
typedef device_type_iterator<&device_creator<palette_device>, palette_device> palette_device_iterator;


#endif  // __EMUPAL_H__
