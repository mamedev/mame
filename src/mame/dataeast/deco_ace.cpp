// license:BSD-3-Clause
// copyright-holders:Bryan McPhail
/*************************************************************************

    deco_ace.cpp

    Data East 99 "ACE" Chip Emulation

    Original source (from deco32.cpp) by Bryan McPhail, Splited by cam900.

**************************************************************************/
/*Some notes pieced together from Tattoo Assassins info(from deco32.cpp):

    Bytes 0 to 0x1f : Alpha Control
    Tattoo Assassins :
    Bytes 0x00 to 0x07(0x1c) - object alpha control per palette/priority / 8
    Bytes 0x08(0x20) to 0x16(0x58) - used, unknown condition/purpose (possibly object)
    Bytes 0x17(0x5c) to 0x1f(0x7c) - tilemap alpha control

    Night slashers :
    Bytes 0x00 to 0x05(0x14) - object alpha control per palette / 2 (if ((pix & 0x900) != 0x100))
    Bytes 0x06(0x18) to 0x16(0x58) - unused/unknown
    Bytes 0x17(0x5c) to 0x1f(0x7c) - tilemap alpha control

    Boogie Wings:
    Bytes 0x00 to 0x0f(0x1f) - object alpha control per palette bank (if ((pix & 0x900) == 0))
    Bytes 0x10(0x20) to 0x11(0x26) - object alpha control per palette bit 3 (if ((pix & 0x900) == 0x100))
    Bytes 0x12(0x20) to 0x13(0x26) - object alpha control per palette bit 3 (if ((pix & 0x900) == 0x800))
    Bytes 0x14(0x28) to 0x19(0x32) - object alpha control per pixel (if ((pix & 0x900) == 0x900))
    Bytes 0x1a(0x34) to 0x1e(0x3c) - unused/unknown
    Bytes 0x1f(0x3e) - 2nd tilemap chip, 'tilemap_1' alpha control, used at shadowing

    0 = opaque, 0x10 = 50% transparent,
    0x20 = fully transparent (doesn't make sense bitwise -AS),
    0x22 = stage 1 boss dark effect in Boogie Wings (incorrectly emulated, reverses source pixel + shifts down by 1? -AS)
    0x21/0x1000 = used,unknown (by what!? -AS)

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
    Byte 0x27(0x9c): unused/unknown

    The 'ST' value lerps between the 'PT' value and the palette entries.  So, if PT==0,
    then ST ranging from 0 to 255 will cause a fade to black (when ST==255 the palette
    becomes zero).

    'fadetype' - 1100 for multiplicative fade, 1000 for additive

    TODO:
        additive fade is correct? verify additive fading from real pcb.
*/


#include "emu.h"
#include "deco_ace.h"
#include <algorithm>


DEFINE_DEVICE_TYPE(DECO_ACE, deco_ace_device, "deco_ace", "Data East 99 'ACE' Chip")

deco_ace_device::deco_ace_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, DECO_ACE, tag, owner, clock),
	device_video_interface(mconfig, *this),
	device_palette_interface(mconfig, *this),
	m_paletteram(nullptr),
	m_paletteram_buffered(nullptr),
	m_ace_ram(nullptr)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void deco_ace_device::device_start()
{
	m_paletteram = make_unique_clear<uint32_t[]>(2048);
	m_paletteram_buffered = make_unique_clear<uint32_t[]>(2048);
	m_ace_ram = make_unique_clear<uint16_t[]>(0x28);

	save_pointer(NAME(m_paletteram), 2048);
	save_pointer(NAME(m_paletteram_buffered), 2048);
	save_pointer(NAME(m_ace_ram), 0x28);
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void deco_ace_device::device_reset()
{
	memset(m_ace_ram.get(),0,0x28);
}

//-------------------------------------------------
//  device_post_load - device-specific post-load
//-------------------------------------------------

void deco_ace_device::device_post_load()
{
	palette_update();
}

/*****************************************************************************
    DEVICE HANDLERS
*****************************************************************************/

/* Games have double buffered paletteram - the real palette ram is
only updated on a DMA call */

// nslasher
uint32_t deco_ace_device::buffered_palette_r(offs_t offset)
{
	return m_paletteram[offset];
}

void deco_ace_device::buffered_palette_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&m_paletteram[offset]);
}

// boogwing has 16 bit cpu data bus(M68000 Based)
uint16_t deco_ace_device::buffered_palette16_r(offs_t offset)
{
	if ((offset & 1) == 0)
		return (m_paletteram[offset >> 1] >> 16) & 0xffff;
	else
		return m_paletteram[offset >> 1] & 0xffff;
}

void deco_ace_device::buffered_palette16_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	if ((offset & 1) == 0)
		m_paletteram[offset >> 1] = (m_paletteram[offset >> 1] & ~(mem_mask<<16)) | ((data & mem_mask)<<16);
	else
		m_paletteram[offset >> 1] = (m_paletteram[offset >> 1] & ~mem_mask) | (data & mem_mask);
}

uint16_t deco_ace_device::ace_r(offs_t offset)
{
	return m_ace_ram[offset];
}

void deco_ace_device::ace_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_ace_ram[offset]);
	if ((offset >= 0x20) && (offset <= 0x26))
		palette_update();
}

void deco_ace_device::palette_update()
{
	int fadeptr=m_ace_ram[0x20] & 0xff;
	int fadeptg=m_ace_ram[0x21] & 0xff;
	int fadeptb=m_ace_ram[0x22] & 0xff;
	int fadepsr=m_ace_ram[0x23] & 0xff;
	int fadepsg=m_ace_ram[0x24] & 0xff;
	int fadepsb=m_ace_ram[0x25] & 0xff;
	uint16_t mode=m_ace_ram[0x26] & 0xffff;

	for (int i = 0; i < 2048; i++)
	{
		/* Lerp palette entry to 'fadept' according to 'fadeps' */
		int b = (m_paletteram_buffered[i] >>16) & 0xff;
		int g = (m_paletteram_buffered[i] >> 8) & 0xff;
		int r = (m_paletteram_buffered[i] >> 0) & 0xff;
		set_pen_color(i + 2048, rgb_t(r, g, b)); // raw palettes

		switch (mode)
		{
			default:
			case 0x1100: // multiplicative fade
				/* Yeah, this should really be fixed point, I know */
				b = std::max<int>(0, std::min<int>(255, u8((b + (((fadeptb - b) * fadepsb) / 255)))));
				g = std::max<int>(0, std::min<int>(255, u8((g + (((fadeptg - g) * fadepsg) / 255)))));
				r = std::max<int>(0, std::min<int>(255, u8((r + (((fadeptr - r) * fadepsr) / 255)))));
				break;
			case 0x1000: // additive fade, correct?
				b = std::min(b + fadepsb, 0xff);
				g = std::min(g + fadepsg, 0xff);
				r = std::min(r + fadepsr, 0xff);
				break;
		}
		set_pen_color(i, rgb_t(r, g, b)); // faded palettes
	}
}

/*************************************************************************

    get_alpha : Get alpha value (ACE RAM Area 0x00 - 0x1f)

*************************************************************************/

uint8_t deco_ace_device::get_alpha(uint8_t val)
{
	val &= 0x1f;
	int alpha = m_ace_ram[val] & 0xff;
	if (alpha > 0x20)
	{
		return 0x80; // TODO : Special blending command? 0x21, 0x22, 0x1000 confirmed
	}
	else
	{
		alpha = 255 - (alpha << 3);
		if (alpha < 0)
			alpha = 0;

		return (uint8_t)alpha;
	}
}

/*************************************************************************

    get_aceram : Get ACE RAM value

*************************************************************************/

uint16_t deco_ace_device::get_aceram(uint8_t val)
{
	if (val >= 0x28)
		return 0;

	return m_ace_ram[val];
}

void deco_ace_device::palette_dma_w(uint16_t data)
{
	std::copy(&m_paletteram[0], &m_paletteram[2048], &m_paletteram_buffered[0]);
	palette_update();
}

/*****************************************************************************************/
