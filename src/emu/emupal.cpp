// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    emupal.c

    Palette device.

***************************************************************************/

#include "emu.h"
#include "emupal.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(PALETTE, palette_device, "palette", "palette")

palette_device::palette_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 entries)
	: device_t(mconfig, PALETTE, tag, owner, (uint32_t)0),
		device_palette_interface(mconfig, *this),
		m_entries(0),
		m_indirect_entries(0),
		m_enable_shadows(0),
		m_enable_hilights(0),
		m_membits(0),
		m_membits_supplied(false),
		m_endianness(),
		m_endianness_supplied(false),
		m_prom_region(*this, finder_base::DUMMY_TAG),
		m_init(palette_init_delegate()),
		m_raw_to_rgb(raw_to_rgb_converter())
{
	set_entries(entries);
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

WRITE8_MEMBER(palette_device::write8)
{
	m_paletteram.write8(offset, data);
	update_for_write(offset, 1);
}

WRITE16_MEMBER(palette_device::write16)
{
	m_paletteram.write16(offset, data, mem_mask);
	update_for_write(offset * 2, 2);
}

WRITE32_MEMBER(palette_device::write32)
{
	m_paletteram.write32(offset, data, mem_mask);
	update_for_write(offset * 4, 4);
}

READ8_MEMBER(palette_device::read8)
{
	return m_paletteram.read8(offset);
}

READ16_MEMBER(palette_device::read16)
{
	return m_paletteram.read16(offset);
}

READ32_MEMBER(palette_device::read32)
{
	return m_paletteram.read32(offset);
}


//-------------------------------------------------
//  write_ext - write a byte to the extended
//  paletteram
//-------------------------------------------------

WRITE8_MEMBER(palette_device::write8_ext)
{
	m_paletteram_ext.write8(offset, data);
	update_for_write(offset, 1);
}

WRITE16_MEMBER(palette_device::write16_ext)
{
	m_paletteram_ext.write16(offset, data, mem_mask);
	update_for_write(offset * 2, 2);
}

READ8_MEMBER(palette_device::read8_ext)
{
	return m_paletteram_ext.read8(offset);
}

READ16_MEMBER(palette_device::read16_ext)
{
	return m_paletteram_ext.read16(offset);
}


//-------------------------------------------------
//  write_indirect - write a byte to the base
//  paletteram, updating indirect colors
//-------------------------------------------------

WRITE8_MEMBER(palette_device::write_indirect)
{
	m_paletteram.write8(offset, data);
	update_for_write(offset, 1, true);
}


//-------------------------------------------------
//  write_ext - write a byte to the extended
//  paletteram, updating indirect colors
//-------------------------------------------------

WRITE8_MEMBER(palette_device::write_indirect_ext)
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
	m_init.bind_relative_to(*owner());

	// find the memory, if present
	const memory_share *share = memshare(tag());
	if (share != nullptr)
	{
		// find the extended (split) memory, if present
		std::string tag_ext = std::string(tag()).append("_ext");
		const memory_share *share_ext = memshare(tag_ext.c_str());

		// make sure we have specified a format
		assert_always(m_raw_to_rgb.bytes_per_entry() > 0, "Palette has memory share but no format specified");

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
			assert_always(m_membits < share->bitwidth(), "Improper use of MCFG_PALETTE_MEMBITS");
			m_paletteram.set_membits(m_membits);
			if (share_ext != nullptr)
				m_paletteram_ext.set_membits(m_membits);
		}

		// override endianness if provided
		if (m_endianness_supplied)
		{
			// forcing endianness only makes sense when the RAM is narrower than the palette format and not split
			assert_always((share_ext == nullptr && m_paletteram.membits() / 8 < bytes_per_entry), "Improper use of MCFG_PALETTE_ENDIANNESS");
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

void palette_device::palette_init_RRRRGGGGBBBB_proms(palette_device &palette)
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
    RRRRR_GGGGG_BBBBB/BBBBB_GGGGG_RRRRR -
    standard 5-5-5 palette for games using a
    15-bit color space
-------------------------------------------------*/

void palette_device::palette_init_RRRRRGGGGGBBBBB(palette_device &palette)
{
	int i;

	for (i = 0; i < 0x8000; i++)
		palette.set_pen_color(i, rgbexpand<5,5,5>(i, 10, 5, 0));
}


void palette_device::palette_init_BBBBBGGGGGRRRRR(palette_device &palette)
{
	int i;

	for (i = 0; i < 0x8000; i++)
		palette.set_pen_color(i, rgbexpand<5,5,5>(i, 0, 5, 10));
}



/*-------------------------------------------------
    RRRRR_GGGGGG_BBBBB/BBBBB_GGGGGG_RRRRR -
    standard 5-6-5 palette for games using a
    16-bit color space
-------------------------------------------------*/

void palette_device::palette_init_RRRRRGGGGGGBBBBB(palette_device &palette)
{
	int i;

	for (i = 0; i < 0x10000; i++)
		palette.set_pen_color(i, rgbexpand<5,6,5>(i, 11, 5, 0));
}

void palette_device::palette_init_BBBBBGGGGGGRRRRR(palette_device &palette)
{
	int i;

	for (i = 0; i < 0x10000; i++)
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
