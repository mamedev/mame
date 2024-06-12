// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    emupal.cpp

    Palette device.

***************************************************************************/

#include "emu.h"
#include "emupal.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(PALETTE, palette_device, "palette", "palette")

palette_device::palette_device(const machine_config &mconfig, const char *tag, device_t *owner, init_delegate &&init, u32 entries, u32 indirect)
	: palette_device(mconfig, tag, owner, 0U)
{
	set_entries(entries, indirect);
	m_init = std::move(init);
}

palette_device::palette_device(const machine_config &mconfig, const char *tag, device_t *owner, black_t, u32 entries)
	: palette_device(mconfig, tag, owner, init_delegate(*this, FUNC(palette_device::palette_init_all_black)), entries)
{
}

palette_device::palette_device(const machine_config &mconfig, const char *tag, device_t *owner, mono_t)
	: palette_device(mconfig, tag, owner, init_delegate(*this, FUNC(palette_device::palette_init_monochrome)), 2)
{
}

palette_device::palette_device(const machine_config &mconfig, const char *tag, device_t *owner, mono_inv_t)
	: palette_device(mconfig, tag, owner, init_delegate(*this, FUNC(palette_device::palette_init_monochrome_inverted)), 2)
{
}

palette_device::palette_device(const machine_config &mconfig, const char *tag, device_t *owner, mono_hi_t)
	: palette_device(mconfig, tag, owner, init_delegate(*this, FUNC(palette_device::palette_init_monochrome_highlight)), 3)
{
}

palette_device::palette_device(const machine_config &mconfig, const char *tag, device_t *owner, rgb_3b_t)
	: palette_device(mconfig, tag, owner, init_delegate(*this, FUNC(palette_device::palette_init_3bit_rgb)), 8)
{
}

palette_device::palette_device(const machine_config &mconfig, const char *tag, device_t *owner, rbg_3b_t)
	: palette_device(mconfig, tag, owner, init_delegate(*this, FUNC(palette_device::palette_init_3bit_rbg)), 8)
{
}

palette_device::palette_device(const machine_config &mconfig, const char *tag, device_t *owner, grb_3b_t)
	: palette_device(mconfig, tag, owner, init_delegate(*this, FUNC(palette_device::palette_init_3bit_grb)), 8)
{
}

palette_device::palette_device(const machine_config &mconfig, const char *tag, device_t *owner, gbr_3b_t)
	: palette_device(mconfig, tag, owner, init_delegate(*this, FUNC(palette_device::palette_init_3bit_gbr)), 8)
{
}

palette_device::palette_device(const machine_config &mconfig, const char *tag, device_t *owner, brg_3b_t)
	: palette_device(mconfig, tag, owner, init_delegate(*this, FUNC(palette_device::palette_init_3bit_brg)), 8)
{
}

palette_device::palette_device(const machine_config &mconfig, const char *tag, device_t *owner, bgr_3b_t)
	: palette_device(mconfig, tag, owner, init_delegate(*this, FUNC(palette_device::palette_init_3bit_bgr)), 8)
{
}

palette_device::palette_device(const machine_config &mconfig, const char *tag, device_t *owner, rgb_555_t)
	: palette_device(mconfig, tag, owner, init_delegate(*this, FUNC(palette_device::palette_init_rgb_555)), 32768)
{
}

palette_device::palette_device(const machine_config &mconfig, const char *tag, device_t *owner, grb_555_t)
	: palette_device(mconfig, tag, owner, init_delegate(*this, FUNC(palette_device::palette_init_grb_555)), 32768)
{
}

palette_device::palette_device(const machine_config &mconfig, const char *tag, device_t *owner, bgr_555_t)
	: palette_device(mconfig, tag, owner, init_delegate(*this, FUNC(palette_device::palette_init_bgr_555)), 32768)
{
}

palette_device::palette_device(const machine_config &mconfig, const char *tag, device_t *owner, rgb_565_t)
	: palette_device(mconfig, tag, owner, init_delegate(*this, FUNC(palette_device::palette_init_rgb_565)), 65536)
{
}

palette_device::palette_device(const machine_config &mconfig, const char *tag, device_t *owner, bgr_565_t)
	: palette_device(mconfig, tag, owner, init_delegate(*this, FUNC(palette_device::palette_init_bgr_565)), 65536)
{
}


palette_device::palette_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, PALETTE, tag, owner, clock)
	, device_palette_interface(mconfig, *this)
	, m_entries(0)
	, m_indirect_entries(0)
	, m_enable_shadows(0)
	, m_enable_hilights(0)
	, m_membits(0)
	, m_membits_supplied(false)
	, m_endianness()
	, m_endianness_supplied(false)
	, m_prom_region(*this, finder_base::DUMMY_TAG)
	, m_init(*this)
	, m_raw_to_rgb()
{
}


palette_device &palette_device::set_format(int bytes_per_entry, raw_to_rgb_converter::raw_to_rgb_func func, u32 entries)
{
	set_format(raw_to_rgb_converter(bytes_per_entry, func));
	set_entries(entries);
	return *this;
}

palette_device &palette_device::set_format(rgb_332_t, u32 entries)
{
	set_format(1, &raw_to_rgb_converter::standard_rgb_decoder<3,3,2, 5,2,0>, entries);
	return *this;
}

palette_device &palette_device::set_format(bgr_233_t, u32 entries)
{
	set_format(1, &raw_to_rgb_converter::standard_rgb_decoder<3,3,2, 0,3,6>, entries);
	return *this;
}

palette_device &palette_device::set_format(rgb_332_inv_t, u32 entries)
{
	set_format(1, &raw_to_rgb_converter::inverted_rgb_decoder<3,3,2, 5,2,0>, entries);
	return *this;
}

palette_device &palette_device::set_format(bgr_233_inv_t, u32 entries)
{
	set_format(1, &raw_to_rgb_converter::inverted_rgb_decoder<3,3,2, 0,3,6>, entries);
	return *this;
}

palette_device &palette_device::set_format(xrgb_333_t, u32 entries)
{
	set_format(2, &raw_to_rgb_converter::standard_rgb_decoder<3,3,3, 6,3,0>, entries);
	return *this;
}

palette_device &palette_device::set_format(xrbg_333_t, u32 entries)
{
	set_format(2, &raw_to_rgb_converter::standard_rgb_decoder<3,3,3, 6,0,3>, entries);
	return *this;
}

palette_device &palette_device::set_format(xbgr_333_t, u32 entries)
{
	set_format(2, &raw_to_rgb_converter::standard_rgb_decoder<3,3,3, 0,3,6>, entries);
	return *this;
}

palette_device &palette_device::set_format(xrgb_444_t, u32 entries)
{
	set_format(2, &raw_to_rgb_converter::standard_rgb_decoder<4,4,4, 8,4,0>, entries);
	return *this;
}

palette_device &palette_device::set_format(xrbg_444_t, u32 entries)
{
	set_format(2, &raw_to_rgb_converter::standard_rgb_decoder<4,4,4, 8,0,4>, entries);
	return *this;
}

palette_device &palette_device::set_format(xbrg_444_t, u32 entries)
{
	set_format(2, &raw_to_rgb_converter::standard_rgb_decoder<4,4,4, 4,0,8>, entries);
	return *this;
}

palette_device &palette_device::set_format(xbgr_444_t, u32 entries)
{
	set_format(2, &raw_to_rgb_converter::standard_rgb_decoder<4,4,4, 0,4,8>, entries);
	return *this;
}

palette_device &palette_device::set_format(rgbx_444_t, u32 entries)
{
	set_format(2, &raw_to_rgb_converter::standard_rgb_decoder<4,4,4, 12,8,4>, entries);
	return *this;
}

palette_device &palette_device::set_format(grbx_444_t, u32 entries)
{
	set_format(2, &raw_to_rgb_converter::standard_rgb_decoder<4,4,4, 8,12,4>, entries);
	return *this;
}

palette_device &palette_device::set_format(gbrx_444_t, u32 entries)
{
	set_format(2, &raw_to_rgb_converter::standard_rgb_decoder<4,4,4, 4,12,8>, entries);
	return *this;
}

palette_device &palette_device::set_format(irgb_4444_t, u32 entries)
{
	set_format(2, &raw_to_rgb_converter::standard_irgb_decoder<4,4,4,4, 12,8,4,0>, entries);
	return *this;
}

palette_device &palette_device::set_format(rgbi_4444_t, u32 entries)
{
	set_format(2, &raw_to_rgb_converter::standard_irgb_decoder<4,4,4,4, 0,12,8,4>, entries);
	return *this;
}

palette_device &palette_device::set_format(ibgr_4444_t, u32 entries)
{
	set_format(2, &raw_to_rgb_converter::standard_irgb_decoder<4,4,4,4, 12,0,4,8>, entries);
	return *this;
}

palette_device &palette_device::set_format(xrgb_555_t, u32 entries)
{
	set_format(2, &raw_to_rgb_converter::standard_rgb_decoder<5,5,5, 10,5,0>, entries);
	return *this;
}

palette_device &palette_device::set_format(xgrb_555_t, u32 entries)
{
	set_format(2, &raw_to_rgb_converter::standard_rgb_decoder<5,5,5, 5,10,0>, entries);
	return *this;
}

palette_device &palette_device::set_format(xgbr_555_t, u32 entries)
{
	set_format(2, &raw_to_rgb_converter::standard_rgb_decoder<5,5,5, 0,10,5>, entries);
	return *this;
}

palette_device &palette_device::set_format(xbrg_555_t, u32 entries)
{
	set_format(2, &raw_to_rgb_converter::standard_rgb_decoder<5,5,5, 5,0,10>, entries);
	return *this;
}

palette_device &palette_device::set_format(xbgr_555_t, u32 entries)
{
	set_format(2, &raw_to_rgb_converter::standard_rgb_decoder<5,5,5, 0,5,10>, entries);
	return *this;
}

palette_device &palette_device::set_format(rgbx_555_t, u32 entries)
{
	set_format(2, &raw_to_rgb_converter::standard_rgb_decoder<5,5,5, 11,6,1>, entries);
	return *this;
}

palette_device &palette_device::set_format(grbx_555_t, u32 entries)
{
	set_format(2, &raw_to_rgb_converter::standard_rgb_decoder<5,5,5, 6,11,1>, entries);
	return *this;
}

palette_device &palette_device::set_format(brgx_555_t, u32 entries)
{
	set_format(2, &raw_to_rgb_converter::standard_rgb_decoder<5,5,5, 6,1,11>, entries);
	return *this;
}

palette_device &palette_device::set_format(xrbg_inv_t, u32 entries)
{
	set_format(2, &raw_to_rgb_converter::inverted_rgb_decoder<5,5,5, 10,0,5>, entries);
	return *this;
}

palette_device &palette_device::set_format(irgb_1555_t, u32 entries)
{
	set_format(2, &raw_to_rgb_converter::IRRRRRGGGGGBBBBB_decoder, entries);
	return *this;
}

palette_device &palette_device::set_format(rgb_565_t, u32 entries)
{
	set_format(2, &raw_to_rgb_converter::standard_rgb_decoder<5,6,5, 11,5,0>, entries);
	return *this;
}

palette_device &palette_device::set_format(bgr_565_t, u32 entries)
{
	set_format(2, &raw_to_rgb_converter::standard_rgb_decoder<5,6,5, 0,5,11>, entries);
	return *this;
}

palette_device &palette_device::set_format(xrgb_888_t, u32 entries)
{
	set_format(4, &raw_to_rgb_converter::standard_rgb_decoder<8,8,8, 16,8,0>, entries);
	return *this;
}

palette_device &palette_device::set_format(xgrb_888_t, u32 entries)
{
	set_format(4, &raw_to_rgb_converter::standard_rgb_decoder<8,8,8, 8,16,0>, entries);
	return *this;
}

palette_device &palette_device::set_format(xbrg_888_t, u32 entries)
{
	set_format(4, &raw_to_rgb_converter::standard_rgb_decoder<8,8,8, 8,0,16>, entries);
	return *this;
}

palette_device &palette_device::set_format(xbgr_888_t, u32 entries)
{
	set_format(4, &raw_to_rgb_converter::standard_rgb_decoder<8,8,8, 0,8,16>, entries);
	return *this;
}

palette_device &palette_device::set_format(rgbx_888_t, u32 entries)
{
	set_format(4, &raw_to_rgb_converter::standard_rgb_decoder<8,8,8, 24,16,8>, entries);
	return *this;
}

palette_device &palette_device::set_format(grbx_888_t, u32 entries)
{
	set_format(4, &raw_to_rgb_converter::standard_rgb_decoder<8,8,8, 16,24,8>, entries);
	return *this;
}

palette_device &palette_device::set_format(bgrx_888_t, u32 entries)
{
	set_format(4, &raw_to_rgb_converter::standard_rgb_decoder<8,8,8, 8,16,24>, entries);
	return *this;
}

palette_device &palette_device::set_format(rrrrggggbbbbrgbx_t, u32 entries)
{
	set_format(2, &raw_to_rgb_converter::RRRRGGGGBBBBRGBx_decoder, entries);
	return *this;
}

palette_device &palette_device::set_format(xrgbrrrrggggbbbb_bit0_t, u32 entries)
{
	set_format(2, &raw_to_rgb_converter::xRGBRRRRGGGGBBBB_bit0_decoder, entries);
	return *this;
}

palette_device &palette_device::set_format(xrgbrrrrggggbbbb_bit4_t, u32 entries)
{
	set_format(2, &raw_to_rgb_converter::xRGBRRRRGGGGBBBB_bit4_decoder, entries);
	return *this;
}

palette_device &palette_device::set_format(xbgrbbbbggggrrrr_bit0_t, u32 entries)
{
	set_format(2, &raw_to_rgb_converter::xBGRBBBBGGGGRRRR_bit0_decoder, entries);
	return *this;
}


//**************************************************************************
//  GENERIC WRITE HANDLERS
//**************************************************************************

//-------------------------------------------------
//  update_for_write - given a write of a given
//  length to a given byte offset, update all
//  potentially modified palette entries
//-------------------------------------------------

inline void palette_device::update_for_write(offs_t byte_offset, int bytes_modified, bool indirect)
{
	assert((m_indirect_entries != 0) == indirect);

	// determine how many entries were modified
	int bpe = m_paletteram.bytes_per_entry();
	assert(bpe != 0);
	int count = (bytes_modified + bpe - 1) / bpe;

	// for each entry modified, fetch the palette data and set the pen color or indirect color
	offs_t base = byte_offset / bpe;
	for (int index = 0; index < count; index++)
	{
		if (indirect)
			set_indirect_color(base + index, m_raw_to_rgb(read_entry(base + index)));
		else
			set_pen_color(base + index, m_raw_to_rgb(read_entry(base + index)));
	}
}


//-------------------------------------------------
//  write - write a byte to the base paletteram
//-------------------------------------------------

void palette_device::write8(offs_t offset, u8 data)
{
	m_paletteram.write8(offset, data);
	update_for_write(offset, 1);
}

void palette_device::write16(offs_t offset, u16 data, u16 mem_mask)
{
	m_paletteram.write16(offset, data, mem_mask);
	update_for_write(offset * 2, 2);
}

void palette_device::write32(offs_t offset, u32 data, u32 mem_mask)
{
	m_paletteram.write32(offset, data, mem_mask);
	update_for_write(offset * 4, 4);
}

u8 palette_device::read8(offs_t offset)
{
	return m_paletteram.read8(offset);
}

u16 palette_device::read16(offs_t offset)
{
	return m_paletteram.read16(offset);
}

u32 palette_device::read32(offs_t offset)
{
	return m_paletteram.read32(offset);
}


//-------------------------------------------------
//  write_ext - write a byte to the extended
//  paletteram
//-------------------------------------------------

void palette_device::write8_ext(offs_t offset, u8 data)
{
	m_paletteram_ext.write8(offset, data);
	update_for_write(offset, 1);
}

void palette_device::write16_ext(offs_t offset, u16 data, u16 mem_mask)
{
	m_paletteram_ext.write16(offset, data, mem_mask);
	update_for_write(offset * 2, 2);
}

u8 palette_device::read8_ext(offs_t offset)
{
	return m_paletteram_ext.read8(offset);
}

u16 palette_device::read16_ext(offs_t offset)
{
	return m_paletteram_ext.read16(offset);
}


//-------------------------------------------------
//  write_indirect - write a byte to the base
//  paletteram, updating indirect colors
//-------------------------------------------------

void palette_device::write_indirect(offs_t offset, u8 data)
{
	m_paletteram.write8(offset, data);
	update_for_write(offset, 1, true);
}


//-------------------------------------------------
//  write_ext - write a byte to the extended
//  paletteram, updating indirect colors
//-------------------------------------------------

void palette_device::write_indirect_ext(offs_t offset, u8 data)
{
	m_paletteram_ext.write8(offset, data);
	update_for_write(offset, 1, true);
}



//**************************************************************************
//  DEVICE MANAGEMENT
//**************************************************************************

//-------------------------------------------------
//  device_start - start up the device
//-------------------------------------------------

void palette_device::device_start()
{
	// bind the init function
	m_init.resolve();

	// find the memory, if present
	const memory_share *share = memshare(tag());
	if (share != nullptr)
	{
		// find the extended (split) memory, if present
		std::string tag_ext = std::string(tag()).append("_ext");
		const memory_share *share_ext = memshare(tag_ext);

		// make sure we have specified a format
		if (m_raw_to_rgb.bytes_per_entry() <= 0)
			throw emu_fatalerror("palette_device(%s): Palette has memory share but no format specified", tag());

		// determine bytes per entry and configure
		int bytes_per_entry = m_raw_to_rgb.bytes_per_entry();
		if (share_ext == nullptr)
			m_paletteram.set(*share, bytes_per_entry);
		else
		{
			m_paletteram.set(*share, bytes_per_entry / 2);
			m_paletteram_ext.set(*share_ext, bytes_per_entry / 2);
		}

		// override membits if provided
		if (m_membits_supplied)
		{
			// forcing width only makes sense when narrower than the native bus width
			if (m_membits >= share->bitwidth())
				throw emu_fatalerror("palette_device(%s): Improper use of MCFG_PALETTE_MEMBITS", tag());
			m_paletteram.set_membits(m_membits);
			if (share_ext != nullptr)
				m_paletteram_ext.set_membits(m_membits);
		}

		// override endianness if provided
		if (m_endianness_supplied)
		{
			// forcing endianness only makes sense when the RAM is narrower than the palette format and not split
			if (share_ext || (m_paletteram.membits() / 8) >= bytes_per_entry)
				throw emu_fatalerror("palette_device(%s): Improper use of MCFG_PALETTE_ENDIANNESS", tag());
			m_paletteram.set_endianness(m_endianness);
		}
	}

	// call the initialization helper if present
	if (!m_init.isnull())
		m_init(*this);
}



//**************************************************************************
//  COMMON PALETTE INITIALIZATION
//**************************************************************************

/*-------------------------------------------------
    black - completely black palette
-------------------------------------------------*/

void palette_device::palette_init_all_black(palette_device &palette)
{
	for (int i = 0; i < palette.entries(); i++)
	{
		palette.set_pen_color(i, rgb_t::black());
	}
}


/*-------------------------------------------------
    monochrome - 2-color black & white
-------------------------------------------------*/

void palette_device::palette_init_monochrome(palette_device &palette)
{
	palette.set_pen_color(0, rgb_t::black());
	palette.set_pen_color(1, rgb_t::white());
}


/*-------------------------------------------------
    monochrome_inverted - 2-color white & black
-------------------------------------------------*/

void palette_device::palette_init_monochrome_inverted(palette_device &palette)
{
	palette.set_pen_color(0, rgb_t::white());
	palette.set_pen_color(1, rgb_t::black());
}


/*-------------------------------------------------
    monochrome_highlight - 3-color
-------------------------------------------------*/

void palette_device::palette_init_monochrome_highlight(palette_device &palette)
{
	palette.set_pen_color(0, rgb_t::black());
	palette.set_pen_color(1, rgb_t(0xc0, 0xc0, 0xc0));
	palette.set_pen_color(2, rgb_t::white());
}


/*-------------------------------------------------
    3bit_rgb - 8-color rgb
-------------------------------------------------*/

void palette_device::palette_init_3bit_rgb(palette_device &palette)
{
	for (int i = 0; i < 8; i++)
		palette.set_pen_color(i, rgb_t(pal1bit(i >> 0), pal1bit(i >> 1), pal1bit(i >> 2)));
}


/*-------------------------------------------------
    3bit_rbg - 8-color rgb
-------------------------------------------------*/

void palette_device::palette_init_3bit_rbg(palette_device &palette)
{
	for (int i = 0; i < 8; i++)
		palette.set_pen_color(i, rgb_t(pal1bit(i >> 0), pal1bit(i >> 2), pal1bit(i >> 1)));
}


/*-------------------------------------------------
    3bit_brg - 8-color rgb
-------------------------------------------------*/

void palette_device::palette_init_3bit_brg(palette_device &palette)
{
	for (int i = 0; i < 8; i++)
		palette.set_pen_color(i, rgb_t(pal1bit(i >> 1), pal1bit(i >> 2), pal1bit(i >> 0)));
}


/*-------------------------------------------------
    3bit_grb - 8-color rgb
-------------------------------------------------*/

void palette_device::palette_init_3bit_grb(palette_device &palette)
{
	for (int i = 0; i < 8; i++)
		palette.set_pen_color(i, rgb_t(pal1bit(i >> 1), pal1bit(i >> 0), pal1bit(i >> 2)));
}


/*-------------------------------------------------
    3bit_gbr - 8-color rgb
-------------------------------------------------*/

void palette_device::palette_init_3bit_gbr(palette_device &palette)
{
	for (int i = 0; i < 8; i++)
		palette.set_pen_color(i, rgb_t(pal1bit(i >> 2), pal1bit(i >> 0), pal1bit(i >> 1)));
}


/*-------------------------------------------------
    3bit_bgr - 8-color rgb
-------------------------------------------------*/

void palette_device::palette_init_3bit_bgr(palette_device &palette)
{
	for (int i = 0; i < 8; i++)
		palette.set_pen_color(i, rgb_t(pal1bit(i >> 2), pal1bit(i >> 1), pal1bit(i >> 0)));
}


/*-------------------------------------------------
    RRRR_GGGG_BBBB - standard 4-4-4 palette,
    assuming the commonly used resistor values:

    bit 3 -- 220 ohm resistor  -- RED/GREEN/BLUE
          -- 470 ohm resistor  -- RED/GREEN/BLUE
          -- 1  kohm resistor  -- RED/GREEN/BLUE
    bit 0 -- 2.2kohm resistor  -- RED/GREEN/BLUE
-------------------------------------------------*/

void palette_device::palette_init_rgb_444_proms(palette_device &palette)
{
	if (!m_prom_region.found())
		throw emu_fatalerror("Unable to find color PROM region '%s'.", m_prom_region.finder_tag());

	const u8 *colors = m_prom_region->base();

	for (int i = 0; i < palette.entries(); i++)
	{
		int bit0, bit1, bit2, bit3;

		// red component
		bit0 = (colors[i] >> 0) & 0x01;
		bit1 = (colors[i] >> 1) & 0x01;
		bit2 = (colors[i] >> 2) & 0x01;
		bit3 = (colors[i] >> 3) & 0x01;
		int r = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;

		// green component
		bit0 = (colors[i + palette.entries()] >> 0) & 0x01;
		bit1 = (colors[i + palette.entries()] >> 1) & 0x01;
		bit2 = (colors[i + palette.entries()] >> 2) & 0x01;
		bit3 = (colors[i + palette.entries()] >> 3) & 0x01;
		int g = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;

		// blue component
		bit0 = (colors[i + 2*palette.entries()] >> 0) & 0x01;
		bit1 = (colors[i + 2*palette.entries()] >> 1) & 0x01;
		bit2 = (colors[i + 2*palette.entries()] >> 2) & 0x01;
		bit3 = (colors[i + 2*palette.entries()] >> 3) & 0x01;
		int b = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;

		palette.set_pen_color(i, rgb_t(r,g,b));
	}
}



/*-------------------------------------------------
    standard 5-5-5 palette for games using a
    15-bit color space
-------------------------------------------------*/

void palette_device::palette_init_rgb_555(palette_device &palette)
{
	for (int i = 0; i < 0x8000; i++)
		palette.set_pen_color(i, rgbexpand<5,5,5>(i, 10, 5, 0));
}


void palette_device::palette_init_grb_555(palette_device &palette)
{
	for (int i = 0; i < 0x8000; i++)
		palette.set_pen_color(i, rgbexpand<5,5,5>(i, 5, 10, 0));
}


void palette_device::palette_init_bgr_555(palette_device &palette)
{
	for (int i = 0; i < 0x8000; i++)
		palette.set_pen_color(i, rgbexpand<5,5,5>(i, 0, 5, 10));
}



/*-------------------------------------------------
    standard 5-6-5 palette for games using a
    16-bit color space
-------------------------------------------------*/

void palette_device::palette_init_rgb_565(palette_device &palette)
{
	for (int i = 0; i < 0x10000; i++)
		palette.set_pen_color(i, rgbexpand<5,6,5>(i, 11, 5, 0));
}

void palette_device::palette_init_bgr_565(palette_device &palette)
{
	for (int i = 0; i < 0x10000; i++)
		palette.set_pen_color(i, rgbexpand<5,6,5>(i, 0, 5, 11));
}

rgb_t raw_to_rgb_converter::IRRRRRGGGGGBBBBB_decoder(u32 raw)
{
	u8 const i = (raw >> 15) & 1;
	u8 const r = pal6bit(((raw >> 9) & 0x3e) | i);
	u8 const g = pal6bit(((raw >> 4) & 0x3e) | i);
	u8 const b = pal6bit(((raw << 1) & 0x3e) | i);
	return rgb_t(r, g, b);
}

rgb_t raw_to_rgb_converter::RRRRGGGGBBBBRGBx_decoder(u32 raw)
{
	u8 const r = pal5bit(((raw >> 11) & 0x1e) | ((raw >> 3) & 0x01));
	u8 const g = pal5bit(((raw >> 7) & 0x1e) | ((raw >> 2) & 0x01));
	u8 const b = pal5bit(((raw >> 3) & 0x1e) | ((raw >> 1) & 0x01));
	return rgb_t(r, g, b);
}

rgb_t raw_to_rgb_converter::xRGBRRRRGGGGBBBB_bit0_decoder(u32 raw)
{
	u8 const r = pal5bit(((raw >> 7) & 0x1e) | ((raw >> 14) & 0x01));
	u8 const g = pal5bit(((raw >> 3) & 0x1e) | ((raw >> 13) & 0x01));
	u8 const b = pal5bit(((raw << 1) & 0x1e) | ((raw >> 12) & 0x01));
	return rgb_t(r, g, b);
}

rgb_t raw_to_rgb_converter::xRGBRRRRGGGGBBBB_bit4_decoder(u32 raw)
{
	u8 const r = pal5bit(((raw >> 8) & 0x0f) | ((raw >> 10) & 0x10));
	u8 const g = pal5bit(((raw >> 4) & 0x0f) | ((raw >> 9)  & 0x10));
	u8 const b = pal5bit(((raw >> 0) & 0x0f) | ((raw >> 8)  & 0x10));
	return rgb_t(r, g, b);
}

rgb_t raw_to_rgb_converter::xBGRBBBBGGGGRRRR_bit0_decoder(u32 raw)
{
	u8 const r = pal5bit(((raw << 1) & 0x1e) | ((raw >> 12) & 0x01));
	u8 const g = pal5bit(((raw >> 3) & 0x1e) | ((raw >> 13) & 0x01));
	u8 const b = pal5bit(((raw >> 7) & 0x1e) | ((raw >> 14) & 0x01));
	return rgb_t(r, g, b);
}
