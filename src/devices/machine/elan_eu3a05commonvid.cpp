// license:BSD-3-Clause
// copyright-holders:David Haywood

#include "emu.h"
#include "elan_eu3a05commonvid.h"

/*
    Common video functions shared by Elan EU3A05 and EU3A14 CPU types
*/

elan_eu3a05commonvid_device::elan_eu3a05commonvid_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, type, tag, owner, clock)
	, m_palette(*this, finder_base::DUMMY_TAG)
{
}


void elan_eu3a05commonvid_device::device_start()
{
}

void elan_eu3a05commonvid_device::device_reset()
{
}

double elan_eu3a05commonvid_device::hue2rgb(double p, double q, double t)
{
	if (t < 0) t += 1;
	if (t > 1) t -= 1;
	if (t < 1 / 6.0f) return p + (q - p) * 6 * t;
	if (t < 1 / 2.0f) return q;
	if (t < 2 / 3.0f) return p + (q - p) * (2 / 3.0f - t) * 6;
	return p;
}


void elan_eu3a05commonvid_device::update_pen(int pen)
{
	uint16_t dat = m_palram[(pen*2)] << 8;
	dat |= m_palram[(pen*2)+1];

	// llll lsss ---h hhhh
	int l_raw = (dat & 0xf800) >> 11;
	int sl_raw = (dat & 0x0700) >> 8;
	int h_raw = (dat & 0x001f) >> 0;

	double l = (double)l_raw / 31.0f;
	double s = (double)sl_raw / 7.0f;
	double h = (double)h_raw / 24.0f;

	double r, g, b;

	if (s == 0) {
		r = g = b = l; // greyscale
	} else {
		double q = l < 0.5f ? l * (1 + s) : l + s - l * s;
		double p = 2 * l - q;
		r = hue2rgb(p, q, h + 1/3.0f);
		g = hue2rgb(p, q, h);
		b = hue2rgb(p, q, h - 1/3.0f);
	}

	int r_real = r * 255.0f;
	int g_real = g * 255.0f;
	int b_real = b * 255.0f;

	m_palette->set_pen_color(pen, r_real, g_real, b_real);
}


uint8_t elan_eu3a05commonvid_device::palette_r(offs_t offset)
{
	return m_palram[offset];
}

void elan_eu3a05commonvid_device::palette_w(offs_t offset, uint8_t data)
{
	m_palram[offset] = data;
	update_pen(offset/2);
}
