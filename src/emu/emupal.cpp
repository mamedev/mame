// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    emupal.c

    Palette device.

***************************************************************************/

#include "emu.h"

#define VERBOSE 0


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type PALETTE = &device_creator<palette_device>;

palette_device::palette_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, PALETTE, "palette", tag, owner, clock, "palette", __FILE__),
		m_entries(0),
		m_indirect_entries(0),
		m_enable_shadows(0),
		m_enable_hilights(0), 
		m_membits(0),
		m_membits_supplied(false), 
		m_endianness(),
		m_endianness_supplied(false),
		m_raw_to_rgb(raw_to_rgb_converter()),
		m_palette(NULL),
		m_pens(NULL), 
		m_format(),
		m_shadow_table(NULL),
		m_shadow_group(0),
		m_hilight_group(0), 
		m_white_pen(0), 
		m_black_pen(0),
		m_init(palette_init_delegate())
{
}


//**************************************************************************
//  INITIALIZATION AND CONFIGURATION
//**************************************************************************

void palette_device::static_set_init(device_t &device, palette_init_delegate init)
{
	downcast<palette_device &>(device).m_init = init;
}


void palette_device::static_set_format(device_t &device, raw_to_rgb_converter raw_to_rgb)
{
	downcast<palette_device &>(device).m_raw_to_rgb = raw_to_rgb;
}


void palette_device::static_set_membits(device_t &device, int membits)
{
	palette_device &palette = downcast<palette_device &>(device);
	palette.m_membits = membits;
	palette.m_membits_supplied = true;
}


void palette_device::static_set_endianness(device_t &device, endianness_t endianness)
{
	palette_device &palette = downcast<palette_device &>(device);
	palette.m_endianness = endianness;
	palette.m_endianness_supplied = true;
}


void palette_device::static_set_entries(device_t &device, int entries)
{
	downcast<palette_device &>(device).m_entries = entries;
}


void palette_device::static_set_indirect_entries(device_t &device, int entries)
{
	downcast<palette_device &>(device).m_indirect_entries = entries;
}


void palette_device::static_enable_shadows(device_t &device)
{
	downcast<palette_device &>(device).m_enable_shadows = true;
}


void palette_device::static_enable_hilights(device_t &device)
{
	downcast<palette_device &>(device).m_enable_hilights = true;
}



//**************************************************************************
//  INDIRECTION (AKA COLORTABLES)
//**************************************************************************

//-------------------------------------------------
//  set_indirect_color - set an indirect color
//-------------------------------------------------

void palette_device::set_indirect_color(int index, rgb_t rgb)
{
	// make sure we are in range
	assert(index < m_indirect_entries);

	// alpha doesn't matter
	rgb.set_a(255);

	// update if it has changed
	if (m_indirect_colors[index] != rgb)
	{
		m_indirect_colors[index] = rgb;

		// update the palette for any colortable entries that reference it
		for (UINT32 pen = 0; pen < m_indirect_pens.size(); pen++)
			if (m_indirect_pens[pen] == index)
				m_palette->entry_set_color(pen, rgb);
	}
}


//-------------------------------------------------
//  set_pen_indirect - set an indirect pen index
//-------------------------------------------------

void palette_device::set_pen_indirect(pen_t pen, UINT16 index)
{
	// make sure we are in range
	assert(pen < m_entries && index < m_indirect_entries);

	m_indirect_pens[pen] = index;

	m_palette->entry_set_color(pen, m_indirect_colors[index]);
}


//-------------------------------------------------
//  transpen_mask - return a mask of pens that
//  whose indirect values match the given
//  transcolor
//-------------------------------------------------

UINT32 palette_device::transpen_mask(gfx_element &gfx, int color, int transcolor)
{
	UINT32 entry = gfx.colorbase() + (color % gfx.colors()) * gfx.granularity();

	// make sure we are in range
	assert(entry < m_indirect_pens.size());
	assert(gfx.depth() <= 32);

	// either gfx->color_depth entries or as many as we can get up until the end
	int count = MIN(gfx.depth(), m_indirect_pens.size() - entry);

	// set a bit anywhere the transcolor matches
	UINT32 mask = 0;
	for (int bit = 0; bit < count; bit++)
		if (m_indirect_pens[entry++] == transcolor)
			mask |= 1 << bit;

	// return the final mask
	return mask;
}



//**************************************************************************
//  SHADOW TABLE CONFIGURATION
//**************************************************************************

//-------------------------------------------------
//  palette_set_shadow_mode(mode)
//
//      mode: 0 = use preset 0 (default shadow)
//            1 = use preset 1 (default highlight)
//            2 = use preset 2 *
//            3 = use preset 3 *
//
//  * Preset 2 & 3 work independently under 32bpp,
//    supporting up to four different types of
//    shadows at one time. They mirror preset 1 & 2
//    in lower depth settings to maintain
//    compatibility.
//
//
//  set_shadow_dRGB32(mode, dr, dg, db, noclip)
//
//      mode:    0 to   3 (which preset to configure)
//
//        dr: -255 to 255 ( red displacement )
//        dg: -255 to 255 ( green displacement )
//        db: -255 to 255 ( blue displacement )
//
//      noclip: 0 = resultant RGB clipped at 0x00/0xff
//              1 = resultant RGB wraparound 0x00/0xff
//
//
//  * Color shadows only work under 32bpp.
//    This function has no effect in lower color
//    depths where
//
//      set_shadow_factor() or
//      set_highlight_factor()
//
//    should be used instead.
//
//  * 32-bit shadows are lossy. Even with zero RGB
//    displacements the affected area will still look
//    slightly darkened.
//
//    Drivers should ensure all shadow pens in
//    gfx_drawmode_table[] are set to DRAWMODE_NONE
//    when RGB displacements are zero to avoid the
//    darkening effect.
//-------------------------------------------------

//-------------------------------------------------
//  set_shadow_dRGB32 - configure delta RGB values
//  for 1 of 4 shadow tables
//-------------------------------------------------

void palette_device::set_shadow_dRGB32(int mode, int dr, int dg, int db, bool noclip)
{
	shadow_table_data &stable = m_shadow_tables[mode];

	// only applies to RGB direct modes
	assert(m_format != BITMAP_FORMAT_IND16);
	assert(stable.base != NULL);

	// clamp the deltas (why?)
	if (dr < -0xff) dr = -0xff; else if (dr > 0xff) dr = 0xff;
	if (dg < -0xff) dg = -0xff; else if (dg > 0xff) dg = 0xff;
	if (db < -0xff) db = -0xff; else if (db > 0xff) db = 0xff;

	// early exit if nothing changed
	if (dr == stable.dr && dg == stable.dg && db == stable.db && noclip == stable.noclip)
		return;
	stable.dr = dr;
	stable.dg = dg;
	stable.db = db;
	stable.noclip = noclip;

	if (VERBOSE)
		popmessage("shadow %d recalc %d %d %d %02x", mode, dr, dg, db, noclip);

	// regenerate the table
	for (int i = 0; i < 32768; i++)
	{
		int r = pal5bit(i >> 10) + dr;
		int g = pal5bit(i >> 5) + dg;
		int b = pal5bit(i >> 0) + db;

		// apply clipping
		if (!noclip)
		{
			r = rgb_t::clamp(r);
			g = rgb_t::clamp(g);
			b = rgb_t::clamp(b);
		}
		rgb_t final = rgb_t(r, g, b);

		// store either 16 or 32 bit
		if (m_format == BITMAP_FORMAT_RGB32)
			stable.base[i] = final;
		else
			stable.base[i] = final.as_rgb15();
	}
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
		UINT32 data = m_paletteram.read(base + index);
		if (m_paletteram_ext.base() != NULL)
			data |= m_paletteram_ext.read(base + index) << (8 * bpe);

		if (indirect)
			set_indirect_color(base + index, m_raw_to_rgb(data));
		else
			m_palette->entry_set_color(base + index, m_raw_to_rgb(data));
	}
}


//-------------------------------------------------
//  write - write a byte to the base paletteram
//-------------------------------------------------

WRITE8_MEMBER(palette_device::write)
{
	m_paletteram.write8(offset, data);
	update_for_write(offset, 1);
}

WRITE16_MEMBER(palette_device::write)
{
	m_paletteram.write16(offset, data, mem_mask);
	update_for_write(offset * 2, 2);
}

WRITE32_MEMBER(palette_device::write)
{
	m_paletteram.write32(offset, data, mem_mask);
	update_for_write(offset * 4, 4);
}

READ8_MEMBER(palette_device::read)
{
	return m_paletteram.read8(offset);
}

READ16_MEMBER(palette_device::read)
{
	return m_paletteram.read16(offset);
}

READ32_MEMBER(palette_device::read)
{
	return m_paletteram.read32(offset);
}


//-------------------------------------------------
//  write_ext - write a byte to the extended
//  paletteram
//-------------------------------------------------

WRITE8_MEMBER(palette_device::write_ext)
{
	m_paletteram_ext.write8(offset, data);
	update_for_write(offset, 1);
}


WRITE16_MEMBER(palette_device::write_ext)
{
	m_paletteram_ext.write16(offset, data, mem_mask);
	update_for_write(offset * 2, 2);
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
	if (share != NULL)
	{
		// find the extended (split) memory, if present
		std::string tag_ext = std::string(tag()).append("_ext");
		const memory_share *share_ext = memshare(tag_ext.c_str());

		// make sure we have specified a format
		assert_always(m_raw_to_rgb.bytes_per_entry() > 0, "Palette has memory share but no format specified");

		// determine bytes per entry and configure
		int bytes_per_entry = m_raw_to_rgb.bytes_per_entry();
		if (share_ext == NULL)
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
			if (share_ext != NULL)
				m_paletteram_ext.set_membits(m_membits);
		}

		// override endianness if provided
		if (m_endianness_supplied)
		{
			// forcing endianness only makes sense when the RAM is narrower than the palette format and not split
			assert_always((share_ext == NULL && m_paletteram.membits() / 8 < bytes_per_entry), "Improper use of MCFG_PALETTE_ENDIANNESS");
			m_paletteram.set_endianness(m_endianness);
		}
	}

	// reset all our data
	screen_device *device = machine().first_screen();
	m_format = (device != NULL) ? device->format() : BITMAP_FORMAT_INVALID;

	// allocate the palette
	if (m_entries > 0)
	{
		allocate_palette();
		allocate_color_tables();
		allocate_shadow_tables();

		// allocate indirection tables
		if (m_indirect_entries > 0)
		{
			m_indirect_colors.resize(m_indirect_entries);
			for (int color = 0; color < m_indirect_entries; color++)
			{
				// alpha = 0 ensures change is detected the first time set_indirect_color() is called
				m_indirect_colors[color] = rgb_t(0, 0, 0, 0);
			}

			m_indirect_pens.resize(m_entries);
			for (int pen = 0; pen < m_entries; pen++)
				m_indirect_pens[pen] = pen % m_indirect_entries;
		}
	}

	// call the initialization helper if present
	if (!m_init.isnull())
		m_init(*this);

	// set up save/restore of the palette
	m_save_pen.resize(m_palette->num_colors());
	m_save_contrast.resize(m_palette->num_colors());
	save_item(NAME(m_save_pen));
	save_item(NAME(m_save_contrast));

	// save indirection tables if we have them
	if (m_indirect_entries > 0)
	{
		save_item(NAME(m_indirect_colors));
		save_item(NAME(m_indirect_pens));
	}
}



//**************************************************************************
//  INTERNAL FUNCTIONS
//**************************************************************************

//-------------------------------------------------
//  device_pre_save - prepare the save arrays
//  for saving
//-------------------------------------------------

void palette_device::device_pre_save()
{
	// fill the save arrays with updated pen and brightness information
	int numcolors = m_palette->num_colors();
	for (int index = 0; index < numcolors; index++)
	{
		m_save_pen[index] = pen_color(index);
		m_save_contrast[index] = pen_contrast(index);
	}
}


//-------------------------------------------------
//  device_post_load - called after restore to
//  actually update the palette
//-------------------------------------------------

void palette_device::device_post_load()
{
	// reset the pen and brightness for each entry
	int numcolors = m_palette->num_colors();
	for (int index = 0; index < numcolors; index++)
	{
		set_pen_color(index, m_save_pen[index]);
		set_pen_contrast(index, m_save_contrast[index]);
	}
}


//-------------------------------------------------
//  device_stop - final cleanup
//-------------------------------------------------

void palette_device::device_stop()
{
	// dereference the palette
	if (m_palette != NULL)
		m_palette->deref();
}


//-------------------------------------------------
//  device_validity_check - validate device
//  configuration
//-------------------------------------------------

void palette_device::device_validity_check(validity_checker &valid) const
{
}


//-------------------------------------------------
//  allocate_palette - allocate and configure the
//  palette object itself
//-------------------------------------------------

void palette_device::allocate_palette()
{
	// determine the number of groups we need
	int numgroups = 1;
	if (m_enable_shadows)
		m_shadow_group = numgroups++;
	if (m_enable_hilights)
		m_hilight_group = numgroups++;
	assert_always(m_entries * numgroups <= 65536, "Palette has more than 65536 colors.");

	// allocate a palette object containing all the colors and groups
	m_palette = palette_t::alloc(m_entries, numgroups);

	// configure the groups
	if (m_shadow_group != 0)
		set_shadow_factor(float(PALETTE_DEFAULT_SHADOW_FACTOR));
	if (m_hilight_group != 0)
		set_highlight_factor(float(PALETTE_DEFAULT_HIGHLIGHT_FACTOR));

	// set the initial colors to a standard rainbow
	for (int index = 0; index < m_entries; index++)
		set_pen_color(index, rgbexpand<1,1,1>(index, 0, 1, 2));

	// switch off the color mode
	switch (m_format)
	{
		// 16-bit paletteized case
		case BITMAP_FORMAT_IND16:
			m_black_pen = m_palette->black_entry();
			m_white_pen = m_palette->white_entry();
			if (m_black_pen >= 65536)
				m_black_pen = 0;
			if (m_white_pen >= 65536)
				m_white_pen = 65535;
			break;

		// 32-bit direct case
		case BITMAP_FORMAT_RGB32:
			m_black_pen = rgb_t::black;
			m_white_pen = rgb_t::white;
			break;

		// screenless case
		case BITMAP_FORMAT_INVALID:
		default:
			break;
	}
}


//-------------------------------------------------
//  allocate_color_tables - allocate memory for
//  pen and color tables
//-------------------------------------------------

void palette_device::allocate_color_tables()
{
	int total_colors = m_palette->num_colors() * m_palette->num_groups();

	// allocate memory for the pen table
	switch (m_format)
	{
		case BITMAP_FORMAT_IND16:
			// create a dummy 1:1 mapping
			{
				m_pen_array.resize(total_colors + 2);
				pen_t *pentable = &m_pen_array[0];
				m_pens = &m_pen_array[0];
				for (int i = 0; i < total_colors + 2; i++)
					pentable[i] = i;
			}
			break;

		case BITMAP_FORMAT_RGB32:
			m_pens = reinterpret_cast<const pen_t *>(m_palette->entry_list_adjusted());
			break;

		default:
			m_pens = NULL;
			break;
	}
}


//-------------------------------------------------
//  allocate_shadow_tables - allocate memory for
//  shadow tables
//-------------------------------------------------

void palette_device::allocate_shadow_tables()
{
	// if we have shadows, allocate shadow tables
	if (m_enable_shadows)
	{
		m_shadow_array.resize(65536);

		// palettized mode gets a single 64k table in slots 0 and 2
		if (m_format == BITMAP_FORMAT_IND16)
		{
			m_shadow_tables[0].base = m_shadow_tables[2].base = &m_shadow_array[0];
			for (int i = 0; i < 65536; i++)
				m_shadow_array[i] = (i < m_entries) ? (i + m_entries) : i;
		}

		// RGB mode gets two 32k tables in slots 0 and 2
		else
		{
			m_shadow_tables[0].base = &m_shadow_array[0];
			m_shadow_tables[2].base = &m_shadow_array[32768];
			configure_rgb_shadows(0, float(PALETTE_DEFAULT_SHADOW_FACTOR));
		}
	}

	// if we have hilights, allocate shadow tables
	if (m_enable_hilights)
	{
		m_hilight_array.resize(65536);

		// palettized mode gets a single 64k table in slots 1 and 3
		if (m_format == BITMAP_FORMAT_IND16)
		{
			m_shadow_tables[1].base = m_shadow_tables[3].base = &m_hilight_array[0];
			for (int i = 0; i < 65536; i++)
				m_hilight_array[i] = (i < m_entries) ? (i + 2 * m_entries) : i;
		}

		// RGB mode gets two 32k tables in slots 1 and 3
		else
		{
			m_shadow_tables[1].base = &m_hilight_array[0];
			m_shadow_tables[3].base = &m_hilight_array[32768];
			configure_rgb_shadows(1, float(PALETTE_DEFAULT_HIGHLIGHT_FACTOR));
		}
	}

	// set the default table
	m_shadow_table = m_shadow_tables[0].base;
}


//-------------------------------------------------
//  configure_rgb_shadows - configure shadows
//  for the RGB tables
//-------------------------------------------------

void palette_device::configure_rgb_shadows(int mode, float factor)
{
	// only applies to RGB direct modes
	assert(m_format != BITMAP_FORMAT_IND16);

	// verify the shadow table
	assert(mode >= 0 && mode < ARRAY_LENGTH(m_shadow_tables));
	shadow_table_data &stable = m_shadow_tables[mode];
	assert(stable.base != NULL);

	// regenerate the table
	int ifactor = int(factor * 256.0f);
	for (int rgb555 = 0; rgb555 < 32768; rgb555++)
	{
		UINT8 r = rgb_t::clamp((pal5bit(rgb555 >> 10) * ifactor) >> 8);
		UINT8 g = rgb_t::clamp((pal5bit(rgb555 >> 5) * ifactor) >> 8);
		UINT8 b = rgb_t::clamp((pal5bit(rgb555 >> 0) * ifactor) >> 8);

		// store either 16 or 32 bit
		rgb_t final = rgb_t(r, g, b);
		if (m_format == BITMAP_FORMAT_RGB32)
			stable.base[rgb555] = final;
		else
			stable.base[rgb555] = final.as_rgb15();
	}
}



//**************************************************************************
//  COMMON PALETTE INITIALIZATION
//**************************************************************************

/*-------------------------------------------------
    black - completely black palette
-------------------------------------------------*/

void palette_device::palette_init_all_black(palette_device &palette)
{
	int i;

	for (i = 0; i < palette.entries(); i++)
	{
		palette.set_pen_color(i,rgb_t::black); // black
	}
}


/*-------------------------------------------------
    black_and_white - basic 2-color black & white
-------------------------------------------------*/

void palette_device::palette_init_black_and_white(palette_device &palette)
{
	palette.set_pen_color(0,rgb_t::black); // black
	palette.set_pen_color(1,rgb_t::white); // white
}


/*-------------------------------------------------
    white_and_black - basic 2-color white & black
-------------------------------------------------*/

void palette_device::palette_init_white_and_black(palette_device &palette)
{
	palette.set_pen_color(0,rgb_t::white); // white
	palette.set_pen_color(1,rgb_t::black); // black
}


/*-------------------------------------------------
    monochrome_amber - 2-color black & amber
-------------------------------------------------*/

void palette_device::palette_init_monochrome_amber(palette_device &palette)
{
	palette.set_pen_color(0, rgb_t::black); // black
	palette.set_pen_color(1, rgb_t(0xf7, 0xaa, 0x00)); // amber
}


/*-------------------------------------------------
    monochrome_green - 2-color black & green
-------------------------------------------------*/

void palette_device::palette_init_monochrome_green(palette_device &palette)
{
	palette.set_pen_color(0, rgb_t::black); // black
	palette.set_pen_color(1, rgb_t(0x00, 0xff, 0x00)); // green
}


/*-------------------------------------------------
    monochrome_green_highlight - 3-color black & green
-------------------------------------------------*/

void palette_device::palette_init_monochrome_green_highlight(palette_device &palette)
{
	palette.set_pen_color(0, rgb_t::black); // black
	palette.set_pen_color(1, rgb_t(0x00, 0xc0, 0x00)); // green
	palette.set_pen_color(2, rgb_t(0x00, 0xff, 0x00)); // green
}


/*-------------------------------------------------
    monochrome_yellow - 2-color black & yellow
-------------------------------------------------*/

void palette_device::palette_init_monochrome_yellow(palette_device &palette)
{
	palette.set_pen_color(0, rgb_t::black); // black
	palette.set_pen_color(1, rgb_t(0xff, 0xff, 0x00)); // yellow
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
	const UINT8 *color_prom = machine().root_device().memregion("proms")->base();
	int i;

	for (i = 0; i < palette.entries(); i++)
	{
		int bit0,bit1,bit2,bit3,r,g,b;

		// red component
		bit0 = (color_prom[i] >> 0) & 0x01;
		bit1 = (color_prom[i] >> 1) & 0x01;
		bit2 = (color_prom[i] >> 2) & 0x01;
		bit3 = (color_prom[i] >> 3) & 0x01;
		r = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;

		// green component
		bit0 = (color_prom[i + palette.entries()] >> 0) & 0x01;
		bit1 = (color_prom[i + palette.entries()] >> 1) & 0x01;
		bit2 = (color_prom[i + palette.entries()] >> 2) & 0x01;
		bit3 = (color_prom[i + palette.entries()] >> 3) & 0x01;
		g = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;

		// blue component
		bit0 = (color_prom[i + 2*palette.entries()] >> 0) & 0x01;
		bit1 = (color_prom[i + 2*palette.entries()] >> 1) & 0x01;
		bit2 = (color_prom[i + 2*palette.entries()] >> 2) & 0x01;
		bit3 = (color_prom[i + 2*palette.entries()] >> 3) & 0x01;
		b = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;

		palette.set_pen_color(i,rgb_t(r,g,b));
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
    RRRRR_GGGGGG_BBBBB -
    standard 5-6-5 palette for games using a
    16-bit color space
-------------------------------------------------*/

void palette_device::palette_init_RRRRRGGGGGGBBBBB(palette_device &palette)
{
	int i;

	for (i = 0; i < 0x10000; i++)
		palette.set_pen_color(i, rgbexpand<5,6,5>(i, 11, 5, 0));
}

rgb_t raw_to_rgb_converter::IRRRRRGGGGGBBBBB_decoder(UINT32 raw)
{
	UINT8 i = (raw >> 15) & 1;
	UINT8 r = pal6bit(((raw >> 9) & 0x3e) | i);
	UINT8 g = pal6bit(((raw >> 4) & 0x3e) | i);
	UINT8 b = pal6bit(((raw << 1) & 0x3e) | i);
	return rgb_t(r, g, b);
}

rgb_t raw_to_rgb_converter::RRRRGGGGBBBBRGBx_decoder(UINT32 raw)
{
	UINT8 r = pal5bit(((raw >> 11) & 0x1e) | ((raw >> 3) & 0x01));
	UINT8 g = pal5bit(((raw >> 7) & 0x1e) | ((raw >> 2) & 0x01));
	UINT8 b = pal5bit(((raw >> 3) & 0x1e) | ((raw >> 1) & 0x01));
	return rgb_t(r, g, b);
}

rgb_t raw_to_rgb_converter::xRGBRRRRGGGGBBBB_bit0_decoder(UINT32 raw)
{
	UINT8 r = pal5bit(((raw >> 7) & 0x1e) | ((raw >> 14) & 0x01));
	UINT8 g = pal5bit(((raw >> 3) & 0x1e) | ((raw >> 13) & 0x01));
	UINT8 b = pal5bit(((raw << 1) & 0x1e) | ((raw >> 12) & 0x01));
	return rgb_t(r, g, b);
}

rgb_t raw_to_rgb_converter::xRGBRRRRGGGGBBBB_bit4_decoder(UINT32 raw)
{
	UINT8 r = pal5bit(((raw >> 8) & 0x0f) | ((raw >> 10) & 0x10));
	UINT8 g = pal5bit(((raw >> 4) & 0x0f) | ((raw >> 9)  & 0x10));
	UINT8 b = pal5bit(((raw >> 0) & 0x0f) | ((raw >> 8)  & 0x10));
	return rgb_t(r, g, b);
}
