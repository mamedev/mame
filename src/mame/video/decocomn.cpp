// license:BSD-3-Clause
// copyright-holders:Bryan McPhail
/* Common DECO video functions (general, not sorted by IC) */
/* I think most of this stuff is driver specific and really shouldn't be in a device at all.
   It was only put here because I wanted to split deco_tilegen1 to just be the device for the
   tilemap chips, and not contain all this extra unrelated stuff */


#include "emu.h"
#include "video/decocomn.h"


const device_type DECOCOMN = &device_creator<decocomn_device>;

decocomn_device::decocomn_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, DECOCOMN, "DECO Common Video Functions", tag, owner, clock, "decocomn", __FILE__),
	device_video_interface(mconfig, *this),
	m_dirty_palette(nullptr),
	m_priority(0),
	m_palette(*this),
	m_generic_paletteram_16(*this, "^paletteram")
{
}

//-------------------------------------------------
//  static_set_palette_tag: Set the tag of the
//  palette device
//-------------------------------------------------

void decocomn_device::static_set_palette_tag(device_t &device, const char *tag)
{
	downcast<decocomn_device &>(device).m_palette.set_tag(tag);
}

//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void decocomn_device::device_config_complete()
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void decocomn_device::device_start()
{
//  int width, height;

//  width = m_screen->width();
//  height = m_screen->height();

	m_dirty_palette = make_unique_clear<UINT8[]>(4096);

	save_item(NAME(m_priority));
	save_pointer(NAME(m_dirty_palette.get()), 4096);
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void decocomn_device::device_reset()
{
	m_priority = 0;
}

/*****************************************************************************
    DEVICE HANDLERS
*****************************************************************************/

/* Later games have double buffered paletteram - the real palette ram is
only updated on a DMA call */

WRITE16_MEMBER( decocomn_device::nonbuffered_palette_w )
{
	int r,g,b;

	COMBINE_DATA(&m_generic_paletteram_16[offset]);
	if (offset&1) offset--;

	b = (m_generic_paletteram_16[offset] >> 0) & 0xff;
	g = (m_generic_paletteram_16[offset + 1] >> 8) & 0xff;
	r = (m_generic_paletteram_16[offset + 1] >> 0) & 0xff;

	m_palette->set_pen_color(offset / 2, rgb_t(r,g,b));
}

WRITE16_MEMBER( decocomn_device::buffered_palette_w )
{
	COMBINE_DATA(&m_generic_paletteram_16[offset]);

	m_dirty_palette[offset / 2] = 1;
}

WRITE16_MEMBER( decocomn_device::palette_dma_w )
{
	const int m = m_palette->entries();
	int r, g, b, i;

	for (i = 0; i < m; i++)
	{
		if (m_dirty_palette[i])
		{
			m_dirty_palette[i] = 0;

			b = (m_generic_paletteram_16[i * 2] >> 0) & 0xff;
			g = (m_generic_paletteram_16[i * 2 + 1] >> 8) & 0xff;
			r = (m_generic_paletteram_16[i * 2 + 1] >> 0) & 0xff;

			m_palette->set_pen_color(i, rgb_t(r,g,b));
		}
	}
}

/*****************************************************************************************/

/* */
READ16_MEMBER( decocomn_device::d_71_r )
{
	return 0xffff;
}

WRITE16_MEMBER( decocomn_device::priority_w )
{
	m_priority = data;
}

READ16_MEMBER( decocomn_device::priority_r )
{
	return m_priority;
}
