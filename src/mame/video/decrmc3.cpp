// license:BSD-3-Clause
// copyright-holders:Bryan McPhail
/***************************************************************************

    decrmc3.cpp

    Data East custom palette device.

***************************************************************************/

#include "emu.h"
#include "decrmc3.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(DECO_RMC3, deco_rmc3_device, "deco_rmc3", "DECO RM-C3 PALETTE")

deco_rmc3_device::deco_rmc3_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, DECO_RMC3, tag, owner, clock),
		device_palette_interface(mconfig, *this),
		m_entries(0),
		m_indirect_entries(0),
		m_prom_region(*this, finder_base::DUMMY_TAG),
		m_init(deco_rmc3_palette_init_delegate())
{
}


//**************************************************************************
//  GENERIC WRITE HANDLERS
//**************************************************************************

//-------------------------------------------------
//  update_for_write - given a write of a given
//  length to a given byte offset, update all
//  potentially modified palette entries
//-------------------------------------------------

inline void deco_rmc3_device::update_for_write(offs_t byte_offset, int bytes_modified, bool indirect)
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
			set_indirect_color(base + index, deco_rgb_decoder(read_entry(base + index)));
		else
			set_pen_color(base + index, deco_rgb_decoder(read_entry(base + index)));
	}
}


//-------------------------------------------------
//  write - write a byte to the base paletteram
//-------------------------------------------------

WRITE8_MEMBER(deco_rmc3_device::write8)
{
	m_paletteram.write8(offset, data);
	update_for_write(offset, 1);
}

WRITE16_MEMBER(deco_rmc3_device::write16)
{
	m_paletteram.write16(offset, data, mem_mask);
	update_for_write(offset * 2, 2);
}

WRITE32_MEMBER(deco_rmc3_device::write32)
{
	m_paletteram.write32(offset, data, mem_mask);
	update_for_write(offset * 4, 4);
}

READ8_MEMBER(deco_rmc3_device::read8)
{
	return m_paletteram.read8(offset);
}

READ16_MEMBER(deco_rmc3_device::read16)
{
	return m_paletteram.read16(offset);
}

READ32_MEMBER(deco_rmc3_device::read32)
{
	return m_paletteram.read32(offset);
}


//-------------------------------------------------
//  write_ext - write a byte to the extended
//  paletteram
//-------------------------------------------------

WRITE8_MEMBER(deco_rmc3_device::write8_ext)
{
	m_paletteram_ext.write8(offset, data);
	update_for_write(offset, 1);
}


WRITE16_MEMBER(deco_rmc3_device::write16_ext)
{
	m_paletteram_ext.write16(offset, data, mem_mask);
	update_for_write(offset * 2, 2);
}


//-------------------------------------------------
//  write_indirect - write a byte to the base
//  paletteram, updating indirect colors
//-------------------------------------------------

WRITE8_MEMBER(deco_rmc3_device::write_indirect)
{
	m_paletteram.write8(offset, data);
	update_for_write(offset, 1, true);
}


//-------------------------------------------------
//  write_ext - write a byte to the extended
//  paletteram, updating indirect colors
//-------------------------------------------------

WRITE8_MEMBER(deco_rmc3_device::write_indirect_ext)
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

void deco_rmc3_device::device_start()
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

		// determine bytes per entry and configure
		int bytes_per_entry = 2;
		if (share_ext == nullptr)
			m_paletteram.set(*share, bytes_per_entry);
		else
		{
			m_paletteram.set(*share, bytes_per_entry / 2);
			m_paletteram_ext.set(*share_ext, bytes_per_entry / 2);
		}


	}

	// call the initialization helper if present
	if (!m_init.isnull())
		m_init(*this);
}



// This conversion mimics the specific weighting used by the Data East
// custom resistor pack marked DECO RM-C3 to convert the digital
// palette for analog output.  It is used on games such as The Real
// Ghostbusters, Gondomania, Cobra Command, Psychonics Oscar.
//
// Resistor values are 220 ohms (MSB), 470 ohms, 1 kohm, 2.2 kohm (LSB)

void deco_rmc3_device::palette_init_proms(deco_rmc3_device &palette)
{
	if (!m_prom_region.found())
		throw emu_fatalerror("Unable to find color PROM region '%s'.", m_prom_region.finder_tag());

	const u8 *colors = m_prom_region->base();

	for (int i = 0; i < palette.entries(); i++)
	{
		u8 r = colors[i]&0xf;
		u8 g = (colors[i]>>4)&0xf;
		u8 b = colors[i + palette.entries()]&0xf;

		r = 0x0e * BIT(r,0) + 0x1f * BIT(r,1) + 0x43 * BIT(r,2) + 0x8f * BIT(r,3);
		g = 0x0e * BIT(g,0) + 0x1f * BIT(g,1) + 0x43 * BIT(g,2) + 0x8f * BIT(g,3);
		b = 0x0e * BIT(b,0) + 0x1f * BIT(b,1) + 0x43 * BIT(b,2) + 0x8f * BIT(b,3);

		palette.set_pen_color(i, rgb_t(r,g,b));
	}
}

rgb_t deco_rmc3_device::deco_rgb_decoder(u32 raw)
{
	u8 r = raw&0xf;
	u8 g = (raw>>4)&0xf;
	u8 b = (raw>>8)&0xf;

	r = 0x0e * BIT(r,0) + 0x1f * BIT(r,1) + 0x43 * BIT(r,2) + 0x8f * BIT(r,3);
	g = 0x0e * BIT(g,0) + 0x1f * BIT(g,1) + 0x43 * BIT(g,2) + 0x8f * BIT(g,3);
	b = 0x0e * BIT(b,0) + 0x1f * BIT(b,1) + 0x43 * BIT(b,2) + 0x8f * BIT(b,3);

	return rgb_t(r, g, b);
}
