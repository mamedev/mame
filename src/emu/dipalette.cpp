// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    dipalette.cpp

    Device palette interface.

***************************************************************************/

#include "emu.h"
#include "screen.h"

#define VERBOSE 0


//**************************************************************************
//  DEVICE INTERFACE MANAGEMENT
//**************************************************************************

//-------------------------------------------------
//  device_palette_interface - constructor
//-------------------------------------------------

device_palette_interface::device_palette_interface(const machine_config &mconfig, device_t &device)
	: device_interface(device, "palette"),
		m_palette(nullptr),
		m_pens(nullptr),
		m_format(BITMAP_FORMAT_RGB32),
		m_shadow_table(nullptr),
		m_shadow_group(0),
		m_hilight_group(0),
		m_white_pen(0),
		m_black_pen(0)
{
}


//-------------------------------------------------
//  interface_validity_check - validation for a
//  device after the configuration has been
//  constructed
//-------------------------------------------------

void device_palette_interface::interface_validity_check(validity_checker &valid) const
{
	// this info must be available before the device has started
	if (palette_entries() == 0)
		osd_printf_error("Palette has no entries specified\n");
}


//-------------------------------------------------
//  interface_pre_start - work to be done prior to
//  actually starting a device
//-------------------------------------------------

void device_palette_interface::interface_pre_start()
{
	// allocate the palette
	u32 numentries = palette_entries();
	allocate_palette(numentries);
	allocate_color_tables();
	allocate_shadow_tables();

	// allocate indirection tables
	int indirect_colors = palette_indirect_entries();
	if (indirect_colors > 0)
	{
		m_indirect_colors.resize(indirect_colors);
		for (int color = 0; color < indirect_colors; color++)
		{
			// alpha = 0 ensures change is detected the first time set_indirect_color() is called
			m_indirect_colors[color] = rgb_t::transparent();
		}

		m_indirect_pens.resize(numentries);
		for (int pen = 0; pen < numentries; pen++)
			m_indirect_pens[pen] = pen % indirect_colors;
	}
}


//-------------------------------------------------
//  interface_post_start - work to be done after
//  actually starting a device
//-------------------------------------------------

void device_palette_interface::interface_post_start()
{
	// set up save/restore of the palette
	m_save_pen.resize(m_palette->num_colors());
	m_save_contrast.resize(m_palette->num_colors());
	device().save_item(NAME(m_save_pen));
	device().save_item(NAME(m_save_contrast));

	// save indirection tables if we have them
	if (m_indirect_colors.size() > 0)
	{
		device().save_item(NAME(m_indirect_colors));
		device().save_item(NAME(m_indirect_pens));
	}
}


//-------------------------------------------------
//  interface_pre_save - prepare the save arrays
//  for saving
//-------------------------------------------------

void device_palette_interface::interface_pre_save()
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
//  interface_post_load - called after restore to
//  actually update the palette
//-------------------------------------------------

void device_palette_interface::interface_post_load()
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
//  interface_post_stop - final cleanup
//-------------------------------------------------

void device_palette_interface::interface_post_stop()
{
	// dereference the palette
	if (m_palette != nullptr)
		m_palette->deref();
}


//**************************************************************************
//  INDIRECTION (AKA COLORTABLES)
//**************************************************************************

//-------------------------------------------------
//  set_indirect_color - set an indirect color
//-------------------------------------------------

void device_palette_interface::set_indirect_color(int index, rgb_t rgb)
{
	// make sure we are in range
	assert(index < m_indirect_colors.size());

	// alpha doesn't matter
	rgb.set_a(255);

	// update if it has changed
	if (m_indirect_colors[index] != rgb)
	{
		m_indirect_colors[index] = rgb;

		// update the palette for any colortable entries that reference it
		for (u32 pen = 0; pen < m_indirect_pens.size(); pen++)
			if (m_indirect_pens[pen] == index)
				m_palette->entry_set_color(pen, rgb);
	}
}


//-------------------------------------------------
//  set_pen_indirect - set an indirect pen index
//-------------------------------------------------

void device_palette_interface::set_pen_indirect(pen_t pen, indirect_pen_t index)
{
	// make sure we are in range
	assert(pen < entries() && index < indirect_entries());

	m_indirect_pens[pen] = index;

	m_palette->entry_set_color(pen, m_indirect_colors[index]);
}


//-------------------------------------------------
//  transpen_mask - return a mask of pens that
//  whose indirect values match the given
//  transcolor
//-------------------------------------------------

u32 device_palette_interface::transpen_mask(gfx_element &gfx, u32 color, indirect_pen_t transcolor) const
{
	u32 entry = gfx.colorbase() + (color % gfx.colors()) * gfx.granularity();

	// make sure we are in range
	assert(entry < m_indirect_pens.size());
	assert(gfx.depth() <= 32);

	// either gfx->color_depth entries or as many as we can get up until the end
	int count = std::min(size_t(gfx.depth()), m_indirect_pens.size() - entry);

	// set a bit anywhere the transcolor matches
	u32 mask = 0;
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

void device_palette_interface::set_shadow_dRGB32(int mode, int dr, int dg, int db, bool noclip)
{
	shadow_table_data &stable = m_shadow_tables[mode];

	// only applies to RGB direct modes
	assert(m_format != BITMAP_FORMAT_IND16);
	assert(stable.base != nullptr);

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
		device().popmessage("shadow %d recalc %d %d %d %02x", mode, dr, dg, db, noclip);

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
//  INTERNAL FUNCTIONS
//**************************************************************************

//-------------------------------------------------
//  allocate_palette - allocate and configure the
//  palette object itself
//-------------------------------------------------

void device_palette_interface::allocate_palette(u32 numentries)
{
	assert(numentries > 0);

	// determine the number of groups we need
	int numgroups = 1;
	if (palette_shadows_enabled())
		m_shadow_group = numgroups++;
	if (palette_hilights_enabled())
		m_hilight_group = numgroups++;
	if (numentries * numgroups > 65536)
		throw emu_fatalerror("%s(%s): Palette has more than 65536 colors.", device().shortname(), device().tag());

	// allocate a palette object containing all the colors and groups
	m_palette = palette_t::alloc(numentries, numgroups);

	// configure the groups
	if (m_shadow_group != 0)
		set_shadow_factor(PALETTE_DEFAULT_SHADOW_FACTOR);
	if (m_hilight_group != 0)
		set_highlight_factor(PALETTE_DEFAULT_HIGHLIGHT_FACTOR);

	// set the initial colors to a standard rainbow
	for (int index = 0; index < numentries; index++)
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
			m_black_pen = rgb_t::black();
			m_white_pen = rgb_t::white();
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

void device_palette_interface::allocate_color_tables()
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
			m_pens = nullptr;
			break;
	}
}


//-------------------------------------------------
//  allocate_shadow_tables - allocate memory for
//  shadow tables
//-------------------------------------------------

void device_palette_interface::allocate_shadow_tables()
{
	int numentries = m_palette->num_colors();

	// if we have shadows, allocate shadow tables
	if (m_shadow_group != 0)
	{
		m_shadow_array.resize(65536);

		// palettized mode gets a single 64k table in slots 0 and 2
		if (m_format == BITMAP_FORMAT_IND16)
		{
			m_shadow_tables[0].base = m_shadow_tables[2].base = &m_shadow_array[0];
			for (int i = 0; i < 65536; i++)
				m_shadow_array[i] = (i < numentries) ? (i + numentries) : i;
		}

		// RGB mode gets two 32k tables in slots 0 and 2
		else
		{
			m_shadow_tables[0].base = &m_shadow_array[0];
			m_shadow_tables[2].base = &m_shadow_array[32768];
			configure_rgb_shadows(0, PALETTE_DEFAULT_SHADOW_FACTOR);
		}
	}

	// if we have hilights, allocate shadow tables
	if (m_hilight_group != 0)
	{
		m_hilight_array.resize(65536);

		// palettized mode gets a single 64k table in slots 1 and 3
		if (m_format == BITMAP_FORMAT_IND16)
		{
			m_shadow_tables[1].base = m_shadow_tables[3].base = &m_hilight_array[0];
			for (int i = 0; i < 65536; i++)
				m_hilight_array[i] = (i < numentries) ? (i + 2 * numentries) : i;
		}

		// RGB mode gets two 32k tables in slots 1 and 3
		else
		{
			m_shadow_tables[1].base = &m_hilight_array[0];
			m_shadow_tables[3].base = &m_hilight_array[32768];
			configure_rgb_shadows(1, PALETTE_DEFAULT_HIGHLIGHT_FACTOR);
		}
	}

	// set the default table
	m_shadow_table = m_shadow_tables[0].base;
}


//-------------------------------------------------
//  configure_rgb_shadows - configure shadows
//  for the RGB tables
//-------------------------------------------------

void device_palette_interface::configure_rgb_shadows(int mode, float factor)
{
	// only applies to RGB direct modes
	assert(m_format != BITMAP_FORMAT_IND16);

	// verify the shadow table
	assert(mode >= 0 && mode < ARRAY_LENGTH(m_shadow_tables));
	shadow_table_data &stable = m_shadow_tables[mode];
	assert(stable.base != nullptr);

	// regenerate the table
	int ifactor = int(factor * 256.0f);
	for (int rgb555 = 0; rgb555 < 32768; rgb555++)
	{
		u8 const r = rgb_t::clamp((pal5bit(rgb555 >> 10) * ifactor) >> 8);
		u8 const g = rgb_t::clamp((pal5bit(rgb555 >> 5) * ifactor) >> 8);
		u8 const b = rgb_t::clamp((pal5bit(rgb555 >> 0) * ifactor) >> 8);

		// store either 16 or 32 bit
		rgb_t final = rgb_t(r, g, b);
		if (m_format == BITMAP_FORMAT_RGB32)
			stable.base[rgb555] = final;
		else
			stable.base[rgb555] = final.as_rgb15();
	}
}
