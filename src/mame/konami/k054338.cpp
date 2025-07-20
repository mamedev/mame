// license:BSD-3-Clause
// copyright-holders:David Haywood

#include "emu.h"
#include "k054338.h"

#define VERBOSE 0
#include "logmacro.h"


/***************************************************************************/
/*                                                                         */
/*                                 054338                                  */
/*                                                                         */
/***************************************************************************/

// k054338 alpha blend / final mixer (normally used with the 55555)
// because the implementation is video dependant, this is just a
// register-handling shell.

DEFINE_DEVICE_TYPE(K054338, k054338_device, "k054338", "Konami 054338 Mixer")

k054338_device::k054338_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, K054338, tag, owner, clock),
	device_video_interface(mconfig, *this),
	m_alpha_inv(0),
	m_k055555(*this, finder_base::DUMMY_TAG)
{
	memset(&m_regs, 0, sizeof(m_regs));
	memset(&m_shd_rgb, 0, sizeof(m_shd_rgb));
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void k054338_device::device_start()
{
	save_item(NAME(m_regs));
	save_item(NAME(m_shd_rgb));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void k054338_device::device_reset()
{
	memset(m_regs, 0, sizeof(uint16_t)*32);
	memset(m_shd_rgb, 0, sizeof(int)*9);
}

/*****************************************************************************
    DEVICE HANDLERS
*****************************************************************************/

void k054338_device::word_w(offs_t offset, u16 data, u16 mem_mask)
{
	COMBINE_DATA(m_regs + offset);
}

// returns a 16-bit '338 register
u16 k054338_device::register_r(offs_t offset)
{
	return m_regs[offset];
}

void k054338_device::update_all_shadows(int rushingheroes_hack, palette_device &palette)
{
	int i, d;
	int noclip = m_regs[K338_REG_CONTROL] & K338_CTL_CLIPSL;

	for (i = 0; i < 9; i++)
	{
		d = m_regs[K338_REG_SHAD1R + i] & 0x1ff;
		if (d >= 0x100) d -= 0x200;
		m_shd_rgb[i] = d;
	}

	if (!rushingheroes_hack)
	{
		palette.set_shadow_dRGB32(0, m_shd_rgb[0], m_shd_rgb[1], m_shd_rgb[2], noclip);
		palette.set_shadow_dRGB32(1, m_shd_rgb[3], m_shd_rgb[4], m_shd_rgb[5], noclip);
		palette.set_shadow_dRGB32(2, m_shd_rgb[6], m_shd_rgb[7], m_shd_rgb[8], noclip);
	}
	else // rushing heroes seems to specify shadows in another format, or it's not being interpreted properly.
	{
		palette.set_shadow_dRGB32(0, -80, -80, -80, 0);
		palette.set_shadow_dRGB32(1, -80, -80, -80, 0);
		palette.set_shadow_dRGB32(2, -80, -80, -80, 0);
	}
}

// k054338 BG color fill
void k054338_device::fill_solid_bg(bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	uint32_t bgcolor = (register_r(K338_REG_BGC_R) & 0xff) << 16;
	bgcolor |= register_r(K338_REG_BGC_GB);

	bitmap.fill(bgcolor, cliprect);
}

// Unified k054338/K055555 BG color fill (see p.67)
void k054338_device::fill_backcolor(bitmap_rgb32 &bitmap, const rectangle &cliprect, const pen_t *pal_ptr, int mode)
{
	if ((mode & 0x02) == 0) // solid fill
	{
		bitmap.fill(*pal_ptr, cliprect);
	}
	else
	{
		uint32_t *dst_ptr = &bitmap.pix(cliprect.min_y);
		int dst_pitch = bitmap.rowpixels();

		if ((mode & 0x01) == 0) // vertical gradient fill
		{
			pal_ptr += cliprect.min_y;
			for(int y = cliprect.min_y; y <= cliprect.max_y; y++)
			{
				for(int x = cliprect.min_x; x <= cliprect.max_x; x++)
				{
					dst_ptr[x] = *pal_ptr;
				}

				pal_ptr++;
				dst_ptr += dst_pitch;
			}
		}
		else    // horizontal gradient fill
		{
			int width = cliprect.width() * sizeof(uint32_t);
			pal_ptr += cliprect.min_x;
			dst_ptr += cliprect.min_x;
			for(int y = cliprect.min_y; y<= cliprect.max_y; y++)
			{
				memcpy(dst_ptr, pal_ptr, width);
				dst_ptr += dst_pitch;
			}
		}
	}
}

// addition blending unimplemented (requires major changes to drawgfx and tilemap.cpp)
int k054338_device::set_alpha_level(int pblend)
{
	if (pblend > 3) logerror("mixing mode index out of range: %d (valid range: 0-3)", pblend);

	if (!pblend) return 255;

	// sanitize input
	pblend &= 0b11;

	const u8 mixset = m_regs[K338_REG_PBLEND + (pblend >> 1 & 1)] >> (~pblend << 3 & 8);
	int mixlv  = mixset & 0x1f;

	if (m_alpha_inv) mixlv = 0x1f - mixlv;

	// expand mix level (5 bits -> 8)
	mixlv = (mixlv << 3) | (mixlv >> 2);

	const bool additive_mode = mixset & 0x20;
	const bool mixpri = m_regs[K338_REG_CONTROL] & K338_CTL_MIXPRI;

	mixlv |= additive_mode << 8;
	mixlv |= mixpri << 9;

	// returns 10 bits: pa llll llll
	// p: MI (mixpri)
	// a: additive mode
	// l: alpha level (expanded)
	return mixlv;
}

void k054338_device::invert_alpha(int invert)
{
	m_alpha_inv = invert;
}


void k054338_device::export_config(int **shd_rgb)
{
	*shd_rgb = m_shd_rgb;
}
