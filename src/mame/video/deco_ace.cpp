// license:BSD-3-Clause
// copyright-holders:Bryan McPhail,cam900
/* Data East 99 "ACE" Chip Emulation */
/*Some notes pieced together from Tattoo Assassins info(from deco32.cpp):

    Bytes 0 to 0x1f : Alpha Control
	Tattoo Assassins : 
	Bytes 0x00 to 0x16(0x58) - object alpha control?
	Bytes 0x17(0x5c) to 0x1f(0x7c) - tilemap alpha control

    0 = opaque, 0x10 = 50% transparent, 0x20 = fully transparent

    Byte 0x00: ACEO000P0
                        P8
                        1P0
                        1P8
                        O010C1
                        o010C8
                        ??

    Hardware fade registers:

    Byte 0x20(0x80): fadeptred
    Byte 0x21(0x84): fadeptgreen
    Byte 0x22(0x88): fadeptblue
    Byte 0x23(0x8c): fadestred
    Byte 0x24(0x90): fadestgreen
    Byte 0x25(0x94): fadestblue
    Byte 0x26(0x98): fadetype

    The 'ST' value lerps between the 'PT' value and the palette entries.  So, if PT==0,
    then ST ranging from 0 to 255 will cause a fade to black (when ST==255 the palette
    becomes zero).

    'fadetype' - 1100 for multiplicative fade, 1000 for additive
*/


#include "emu.h"
#include "video/deco_ace.h"


DEFINE_DEVICE_TYPE(DECO_ACE, deco_ace_device, "deco_ace", "Data East 99 'ACE' Chip")

deco_ace_device::deco_ace_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, DECO_ACE, tag, owner, clock),
	device_video_interface(mconfig, *this),
	m_dirty_palette(0),
	m_palette_effect_min(0x100),
	m_palette_effect_max(0xfff),
	m_palette(*this, finder_base::DUMMY_TAG),
	m_generic_paletteram_32(nullptr),
	m_ace_ram(nullptr)
{
}

//-------------------------------------------------
//  static_set_palette_tag: Set the tag of the
//  palette device
//-------------------------------------------------

void deco_ace_device::static_set_palette_tag(device_t &device, const char *tag)
{
	downcast<deco_ace_device &>(device).m_palette.set_tag(tag);
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void deco_ace_device::device_start()
{
	m_generic_paletteram_32 = make_unique_clear<uint32_t[]>(4096);
	m_ace_ram = make_unique_clear<uint16_t[]>(0x28);

	save_item(NAME(m_dirty_palette));
	save_pointer(NAME(m_generic_paletteram_32.get()), 4096);
	save_pointer(NAME(m_ace_ram.get()), 0x28);
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void deco_ace_device::device_reset()
{
	m_palette_effect_min = 0x100;
	m_palette_effect_max = 0xfff;
	for (int i = 0; i < 0x28; i++)
		m_ace_ram[i] = 0;

	m_dirty_palette = 1;
}

/*****************************************************************************
    DEVICE HANDLERS
*****************************************************************************/

// nslasher
READ32_MEMBER( deco_ace_device::buffered_palette_r )
{
	return m_generic_paletteram_32[offset];
}

WRITE32_MEMBER( deco_ace_device::buffered_palette_w )
{
	COMBINE_DATA(&m_generic_paletteram_32[offset]);
}

// boogwing has 16 bit cpu data bus
READ16_MEMBER( deco_ace_device::buffered_palette16_r )
{
	if ((offset & 1) == 0)
		return (m_generic_paletteram_32[offset >> 1] >> 16) & 0xffff;
	else
		return m_generic_paletteram_32[offset >> 1] & 0xffff;
}

WRITE16_MEMBER( deco_ace_device::buffered_palette16_w )
{
	if ((offset & 1) == 0)
		m_generic_paletteram_32[offset >> 1] = (m_generic_paletteram_32[offset >> 1] & ~(mem_mask<<16)) | ((data & mem_mask)<<16);
	else
		m_generic_paletteram_32[offset >> 1] = (m_generic_paletteram_32[offset >> 1] & ~mem_mask) | (data & mem_mask);
}

READ16_MEMBER( deco_ace_device::ace_r )
{
	return m_ace_ram[offset];
}

WRITE16_MEMBER( deco_ace_device::ace_w )
{
	COMBINE_DATA(&m_ace_ram[offset]);
	if ((offset >= 0x20) && (offset <= 0x26))
		m_dirty_palette = 1;
}

void deco_ace_device::palette_update()
{
	if (m_dirty_palette != 0)
	{
		int r,g,b,i;
		uint8_t fadeptr=m_ace_ram[0x20] & 0xff;
		uint8_t fadeptg=m_ace_ram[0x21] & 0xff;
		uint8_t fadeptb=m_ace_ram[0x22] & 0xff;
		uint8_t fadepsr=m_ace_ram[0x23] & 0xff;
		uint8_t fadepsg=m_ace_ram[0x24] & 0xff;
		uint8_t fadepsb=m_ace_ram[0x25] & 0xff;
	//  uint16_t mode=m_ace_ram[0x26] & 0xffff;

		m_dirty_palette=0;

		for (i=0; i<2048; i++)
		{
			/* Lerp palette entry to 'fadept' according to 'fadeps' */
			b = (m_generic_paletteram_32[i] >>16) & 0xff;
			g = (m_generic_paletteram_32[i] >> 8) & 0xff;
			r = (m_generic_paletteram_32[i] >> 0) & 0xff;

			if ((i>=m_palette_effect_min) && (i<=m_palette_effect_max)) /* Screenshots seem to suggest ACE fades do not affect playfield 1 palette (0-255) */
			{
				/* Yeah, this should really be fixed point, I know */
				// if (mode == 0x1100)
				b = (uint8_t)((float)b + (((float)fadeptb - (float)b) * (float)fadepsb/255.0f));
				g = (uint8_t)((float)g + (((float)fadeptg - (float)g) * (float)fadepsg/255.0f));
				r = (uint8_t)((float)r + (((float)fadeptr - (float)r) * (float)fadepsr/255.0f));
			}

			m_palette->set_pen_color(i,rgb_t(r,g,b));
		}
	}
}

void deco_ace_device::set_palette_effect_max(uint32_t val)
{
	if (m_palette_effect_max != val)
	{
		m_palette_effect_max = val;
		m_dirty_palette = 1;
	}
}

uint8_t deco_ace_device::get_alpha(uint8_t val)
{
	val &= 0x1f;
	uint8_t alpha = m_ace_ram[val] & 0xff;
	if (alpha > 0x20)
	{
		return 0x80; // todo
	}
	else
	{
		alpha = 256 - (alpha << 3);
		if (alpha == 256)
		{
			alpha = 255;
		}
		return alpha;
	}
}

uint16_t deco_ace_device::get_aceram(uint8_t val)
{
	val &= 0x3f;
	return m_ace_ram[val];
}

WRITE16_MEMBER( deco_ace_device::palette_dma_w )
{
	m_dirty_palette = 1;
}

/*****************************************************************************************/
