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

#ifndef MAME_EMU_EMUPAL_H
#define MAME_EMU_EMUPAL_H

#pragma once

#include "memarray.h"
#include <type_traits>
#include <utility>


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> raw_to_rgb_converter

class raw_to_rgb_converter
{
public:
	// helper function
	typedef rgb_t (*raw_to_rgb_func)(u32 raw);

	// constructor
	raw_to_rgb_converter() { }
	raw_to_rgb_converter(int bytes_per_entry, raw_to_rgb_func func)
		: m_bytes_per_entry(bytes_per_entry)
		, m_func(func)
	{ }

	// getters
	int bytes_per_entry() const { return m_bytes_per_entry; }

	// helpers
	rgb_t operator()(u32 raw) const { return (*m_func)(raw); }

	// generic raw-to-RGB conversion helpers
	template <int RedBits, int GreenBits, int BlueBits, int RedShift, int GreenShift, int BlueShift>
	static rgb_t standard_rgb_decoder(u32 raw)
	{
		u8 const r = palexpand<RedBits>(raw >> RedShift);
		u8 const g = palexpand<GreenBits>(raw >> GreenShift);
		u8 const b = palexpand<BlueBits>(raw >> BlueShift);
		return rgb_t(r, g, b);
	}

	// data-inverted generic raw-to-RGB conversion helpers
	template <int RedBits, int GreenBits, int BlueBits, int RedShift, int GreenShift, int BlueShift>
	static rgb_t inverted_rgb_decoder(u32 raw)
	{
		u8 const r = palexpand<RedBits>(~raw >> RedShift);
		u8 const g = palexpand<GreenBits>(~raw >> GreenShift);
		u8 const b = palexpand<BlueBits>(~raw >> BlueShift);
		return rgb_t(r, g, b);
	}

	template <int IntBits, int RedBits, int GreenBits, int BlueBits, int IntShift, int RedShift, int GreenShift, int BlueShift>
	static rgb_t standard_irgb_decoder(u32 raw)
	{
		u8 const i = palexpand<IntBits>(raw >> IntShift);
		u8 const r = (i * palexpand<RedBits>(raw >> RedShift)) >> 8;
		u8 const g = (i * palexpand<GreenBits>(raw >> GreenShift)) >> 8;
		u8 const b = (i * palexpand<BlueBits>(raw >> BlueShift)) >> 8;
		return rgb_t(r, g, b);
	}

	// other standard decoders
	static rgb_t IRRRRRGGGGGBBBBB_decoder(u32 raw);
	static rgb_t RRRRGGGGBBBBRGBx_decoder(u32 raw);  // bits 3/2/1 are LSb
	static rgb_t xRGBRRRRGGGGBBBB_bit0_decoder(u32 raw);  // bits 14/13/12 are LSb
	static rgb_t xRGBRRRRGGGGBBBB_bit4_decoder(u32 raw);  // bits 14/13/12 are MSb
	static rgb_t xBGRBBBBGGGGRRRR_bit0_decoder(u32 raw);  // bits 12/13/14 are LSb

private:
	// internal data
	int                 m_bytes_per_entry = 0;
	raw_to_rgb_func     m_func = nullptr;
};


// ======================> palette_device

// device type definition
DECLARE_DEVICE_TYPE(PALETTE, palette_device)

class palette_device : public device_t, public device_palette_interface
{
public:
	typedef device_delegate<void (palette_device &)> init_delegate;

	// black-fill on start
	enum black_t        { BLACK };

	// monochrome
	enum mono_t         { MONOCHROME };
	enum mono_inv_t     { MONOCHROME_INVERTED };
	enum mono_hi_t      { MONOCHROME_HIGHLIGHT };

	// 3-bit (8-colour) - components here are LSB to MSB
	enum rgb_3b_t       { RGB_3BIT };
	enum rbg_3b_t       { RBG_3BIT };
	enum grb_3b_t       { GRB_3BIT };
	enum gbr_3b_t       { GBR_3BIT };
	enum brg_3b_t       { BRG_3BIT };
	enum bgr_3b_t       { BGR_3BIT };

	// 8-bit
	enum rgb_332_t      { RGB_332, RRRGGGBB };
	enum bgr_233_t      { BGR_233, BBGGGRRR };
	enum rgb_332_inv_t  { RGB_332_inverted, RRRGGGBB_inverted };
	enum bgr_233_inv_t  { BGR_233_inverted, BBGGGRRR_inverted };

	// 15-bit
	enum rgb_555_t      { RGB_555, RRRRRGGGGGBBBBB };
	enum grb_555_t      { GRB_555, GGGGGRRRRRBBBBB };
	enum bgr_555_t      { BGR_555, BBBBBGGGGGRRRRR };

	// 16-bit
	enum xrgb_333_t     { xRGB_333, xxxxxxxRRRGGGBBB };
	enum xrbg_333_t     { xRBG_333, xxxxxxxRRRBBBGGG };
	enum xbgr_333_t     { xBGR_333, xxxxxxxBBBGGGRRR };
	enum xrgb_444_t     { xRGB_444, xxxxRRRRGGGGBBBB };
	enum xrbg_444_t     { xRBG_444, xxxxRRRRBBBBGGGG };
	enum xbrg_444_t     { xBRG_444, xxxxBBBBRRRRGGGG };
	enum xbgr_444_t     { xBGR_444, xxxxBBBBGGGGRRRR };
	enum rgbx_444_t     { RGBx_444, RRRRGGGGBBBBxxxx };
	enum grbx_444_t     { GRBx_444, GGGGRRRRBBBBxxxx };
	enum gbrx_444_t     { GBRx_444, GGGGBBBBRRRRxxxx };
	enum irgb_4444_t    { IRGB_4444, IIIIRRRRGGGGBBBB };
	enum rgbi_4444_t    { RGBI_4444, RRRRGGGGBBBBIIII };
	enum ibgr_4444_t    { IBGR_4444, IIIIBBBBGGGGRRRR };
	enum xrgb_555_t     { xRGB_555, xRRRRRGGGGGBBBBB };
	enum xgrb_555_t     { xGRB_555, xGGGGGRRRRRBBBBB };
	enum xgbr_555_t     { xGBR_555, xGGGGGBBBBBRRRRR };
	enum xbrg_555_t     { xBRG_555, xBBBBBRRRRRGGGGG };
	enum xbgr_555_t     { xBGR_555, xBBBBBGGGGGRRRRR };
	enum rgbx_555_t     { RGBx_555, RRRRRGGGGGBBBBBx };
	enum grbx_555_t     { GRBx_555, GGGGGRRRRRBBBBBx };
	enum brgx_555_t     { BRGx_555, BBBBBRRRRRGGGGGx };
	enum xrbg_inv_t     { xRBG_555_inverted, xRRRRRBBBBBGGGGG_inverted };
	enum irgb_1555_t    { IRGB_1555, IRRRRRGGGGGBBBBB };
	enum rgb_565_t      { RGB_565, RRRRRGGGGGGBBBBB };
	enum bgr_565_t      { BGR_565, BBBBBGGGGGGBBBBB };

	// 32-bit
	enum xrgb_888_t     { xRGB_888 };
	enum xgrb_888_t     { xGRB_888 };
	enum xbrg_888_t     { xBRG_888 };
	enum xbgr_888_t     { xBGR_888 };
	enum rgbx_888_t     { RGBx_888 };
	enum grbx_888_t     { GRBx_888 };
	enum bgrx_888_t     { BGRx_888 };

	// other standard formats
	enum rgb_444_prom_t { RGB_444_PROMS, RRRRGGGGBBBB_PROMS };

	// exotic formats
	enum rrrrggggbbbbrgbx_t      { RRRRGGGGBBBBRGBx };
	enum xrgbrrrrggggbbbb_bit0_t { xRGBRRRRGGGGBBBB_bit0 };
	enum xrgbrrrrggggbbbb_bit4_t { xRGBRRRRGGGGBBBB_bit4 };
	enum xbgrbbbbggggrrrr_bit0_t { xBGRBBBBGGGGRRRR_bit0 };

	// construction/destruction
	palette_device(const machine_config &mconfig, const char *tag, device_t *owner, init_delegate &&init, u32 entries = 0U, u32 indirect = 0U);
	palette_device(const machine_config &mconfig, const char *tag, device_t *owner, black_t, u32 entries = 0U);
	palette_device(const machine_config &mconfig, const char *tag, device_t *owner, mono_t);
	palette_device(const machine_config &mconfig, const char *tag, device_t *owner, mono_inv_t);
	palette_device(const machine_config &mconfig, const char *tag, device_t *owner, mono_hi_t);
	palette_device(const machine_config &mconfig, const char *tag, device_t *owner, rgb_3b_t);
	palette_device(const machine_config &mconfig, const char *tag, device_t *owner, rbg_3b_t);
	palette_device(const machine_config &mconfig, const char *tag, device_t *owner, grb_3b_t);
	palette_device(const machine_config &mconfig, const char *tag, device_t *owner, gbr_3b_t);
	palette_device(const machine_config &mconfig, const char *tag, device_t *owner, brg_3b_t);
	palette_device(const machine_config &mconfig, const char *tag, device_t *owner, bgr_3b_t);
	palette_device(const machine_config &mconfig, const char *tag, device_t *owner, rgb_555_t);
	palette_device(const machine_config &mconfig, const char *tag, device_t *owner, grb_555_t);
	palette_device(const machine_config &mconfig, const char *tag, device_t *owner, bgr_555_t);
	palette_device(const machine_config &mconfig, const char *tag, device_t *owner, rgb_565_t);
	palette_device(const machine_config &mconfig, const char *tag, device_t *owner, bgr_565_t);
	palette_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

	template <typename T>
	palette_device(const machine_config &mconfig, const char *tag, device_t *owner, rgb_444_prom_t, T &&region, u32 entries)
		: palette_device(mconfig, tag, owner, init_delegate(*this, FUNC(palette_device::palette_init_rgb_444_proms)), entries)
	{
		set_prom_region(std::forward<T>(region));
	}

	template <typename F>
	palette_device(const machine_config &mconfig, const char *tag, device_t *owner, F &&init, std::enable_if_t<init_delegate::supports_callback<F>::value, const char *> name, u32 entries = 0U, u32 indirect = 0U)
		: palette_device(mconfig, tag, owner, 0U)
	{ set_init(std::forward<F>(init), name).set_entries(entries, indirect); }
	template <typename T, typename F>
	palette_device(const machine_config &mconfig, const char *tag, device_t *owner, T &&devname, F &&init, std::enable_if_t<init_delegate::supports_callback<F>::value, const char *> name, u32 entries = 0U, u32 indirect = 0U)
		: palette_device(mconfig, tag, owner, 0U)
	{ set_init(std::forward<T>(devname), std::forward<F>(init), name).set_entries(entries, indirect); }

	// configuration
	template <typename... T> palette_device &set_init(T &&... args) { m_init.set(std::forward<T>(args)...); return *this; }
	palette_device &set_format(raw_to_rgb_converter raw_to_rgb) { m_raw_to_rgb = raw_to_rgb; return *this; }
	palette_device &set_format(int bytes_per_entry, raw_to_rgb_converter::raw_to_rgb_func func, u32 entries);
	palette_device &set_format(rgb_332_t, u32 entries);
	palette_device &set_format(bgr_233_t, u32 entries);
	palette_device &set_format(rgb_332_inv_t, u32 entries);
	palette_device &set_format(bgr_233_inv_t, u32 entries);
	palette_device &set_format(xrgb_333_t, u32 entries);
	palette_device &set_format(xrbg_333_t, u32 entries);
	palette_device &set_format(xbgr_333_t, u32 entries);
	palette_device &set_format(xrgb_444_t, u32 entries);
	palette_device &set_format(xrbg_444_t, u32 entries);
	palette_device &set_format(xbrg_444_t, u32 entries);
	palette_device &set_format(xbgr_444_t, u32 entries);
	palette_device &set_format(rgbx_444_t, u32 entries);
	palette_device &set_format(grbx_444_t, u32 entries);
	palette_device &set_format(gbrx_444_t, u32 entries);
	palette_device &set_format(irgb_4444_t, u32 entries);
	palette_device &set_format(rgbi_4444_t, u32 entries);
	palette_device &set_format(ibgr_4444_t, u32 entries);
	palette_device &set_format(xrgb_555_t, u32 entries);
	palette_device &set_format(xgrb_555_t, u32 entries);
	palette_device &set_format(xgbr_555_t, u32 entries);
	palette_device &set_format(xbrg_555_t, u32 entries);
	palette_device &set_format(xbgr_555_t, u32 entries);
	palette_device &set_format(rgbx_555_t, u32 entries);
	palette_device &set_format(grbx_555_t, u32 entries);
	palette_device &set_format(brgx_555_t, u32 entries);
	palette_device &set_format(xrbg_inv_t, u32 entries);
	palette_device &set_format(irgb_1555_t, u32 entries);
	palette_device &set_format(rgb_565_t, u32 entries);
	palette_device &set_format(bgr_565_t, u32 entries);
	palette_device &set_format(xrgb_888_t, u32 entries);
	palette_device &set_format(xgrb_888_t, u32 entries);
	palette_device &set_format(xbrg_888_t, u32 entries);
	palette_device &set_format(xbgr_888_t, u32 entries);
	palette_device &set_format(rgbx_888_t, u32 entries);
	palette_device &set_format(grbx_888_t, u32 entries);
	palette_device &set_format(bgrx_888_t, u32 entries);
	palette_device &set_format(rrrrggggbbbbrgbx_t, u32 entries);
	palette_device &set_format(xrgbrrrrggggbbbb_bit0_t, u32 entries);
	palette_device &set_format(xrgbrrrrggggbbbb_bit4_t, u32 entries);
	palette_device &set_format(xbgrbbbbggggrrrr_bit0_t, u32 entries);
	template <typename T> palette_device &set_format(T x, u32 entries, u32 indirect) { set_format(x, entries); set_indirect_entries(indirect); return *this; }
	palette_device &set_membits(int membits) { m_membits = membits; m_membits_supplied = true; return *this; }
	palette_device &set_endianness(endianness_t endianness) { m_endianness = endianness; m_endianness_supplied = true; return *this; }
	palette_device &set_entries(u32 entries) { m_entries = entries; return *this; }
	palette_device &set_entries(u32 entries, u32 indirect) { m_entries = entries; m_indirect_entries = indirect; return *this; }
	palette_device &set_indirect_entries(u32 entries) { m_indirect_entries = entries; return *this; }
	palette_device &enable_shadows() { m_enable_shadows = true; return *this; }
	palette_device &enable_hilights() { m_enable_hilights = true; return *this; }
	template <typename T> palette_device &set_prom_region(T &&region) { m_prom_region.set_tag(std::forward<T>(region)); return *this; }

	// palette RAM accessors
	memory_array &basemem() { return m_paletteram; }
	memory_array &extmem() { return m_paletteram_ext; }

	// raw entry reading
	u32 read_entry(pen_t pen) const
	{
		u32 data = m_paletteram.read(pen);
		if (m_paletteram_ext.base() != nullptr)
			data |= m_paletteram_ext.read(pen) << (8 * m_paletteram.bytes_per_entry());
		return data;
	}

	// generic read/write handlers
	u8 read8(offs_t offset);
	u8 read8_ext(offs_t offset);
	void write8(offs_t offset, u8 data);
	void write8_ext(offs_t offset, u8 data);
	void write_indirect(offs_t offset, u8 data);
	void write_indirect_ext(offs_t offset, u8 data);
	u16 read16(offs_t offset);
	u16 read16_ext(offs_t offset);
	void write16(offs_t offset, u16 data, u16 mem_mask = u16(~0));
	void write16_ext(offs_t offset, u16 data, u16 mem_mask = u16(~0));
	u32 read32(offs_t offset);
	void write32(offs_t offset, u32 data, u32 mem_mask = u32(~0));

	// helper to update palette when data changed
	void update() { if (!m_init.isnull()) m_init(*this); }

protected:
	// device-level overrides
	virtual void device_start() override;

	// device_palette_interface overrides
	virtual u32 palette_entries() const noexcept override { return m_entries; }
	virtual u32 palette_indirect_entries() const noexcept override { return m_indirect_entries; }
	virtual bool palette_shadows_enabled() const noexcept override { return m_enable_shadows; }
	virtual bool palette_hilights_enabled() const noexcept override { return m_enable_hilights; }

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
	void palette_init_rgb_444_proms(palette_device &palette);
	void palette_init_rgb_555(palette_device &palette);
	void palette_init_grb_555(palette_device &palette);
	void palette_init_bgr_555(palette_device &palette);
	void palette_init_rgb_565(palette_device &palette);
	void palette_init_bgr_565(palette_device &palette);

private:
	void update_for_write(offs_t byte_offset, int bytes_modified, bool indirect = false);

	// configuration state
	u32                 m_entries;              // number of entries in the palette
	u32                 m_indirect_entries;     // number of indirect colors in the palette
	bool                m_enable_shadows;       // are shadows enabled?
	bool                m_enable_hilights;      // are hilights enabled?
	int                 m_membits;              // width of palette RAM, if different from native
	bool                m_membits_supplied;     // true if membits forced in static config
	endianness_t        m_endianness;           // endianness of palette RAM, if different from native
	bool                m_endianness_supplied;  // true if endianness forced in static config
	optional_memory_region m_prom_region;       // region where the color PROMs are
	init_delegate       m_init;

	// palette RAM
	raw_to_rgb_converter m_raw_to_rgb;          // format of palette RAM
	memory_array        m_paletteram;           // base memory
	memory_array        m_paletteram_ext;       // extended memory
};


#endif  // MAME_EMU_EMUPAL_H
