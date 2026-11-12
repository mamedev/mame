// license:BSD-3-Clause
// copyright-holders:Andrei I. Holub

#include "emu.h"
#include "specnext_vtest.h"


DEFINE_DEVICE_TYPE(SPECNEXT_VTEST, specnext_vtest_device, "vtest", "Spectrum Next Video Test Pattern")


specnext_vtest_device::specnext_vtest_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, SPECNEXT_VTEST, tag, owner, clock)
{
}

void specnext_vtest_device::draw(bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	static constexpr unsigned BORDER = 32;
	static constexpr unsigned FULL_W = 640;
	static constexpr unsigned FULL_H = 192 + (BORDER << 1); // 256

	const int ox = m_offset_h - (BORDER << 1); // left - 64
	const int oy = m_offset_v - BORDER;         // top  - 32

	// FPGA video_test_pattern.vhd vertical state machine (FPGA line numbers):
	//
	//   VS_BLANK_0 : lines 0-47    black   (48 lines)
	//   VS_RAINBOW : lines 48-111  rainbow (64 lines)
	//   VS_BLANK_1 : lines 112-127 black   (16 lines)
	//   VS_RG      : lines 128-191 RG ramp (64 lines)
	//   VS_BLANK_2 : lines 192-207 black   (16 lines)
	//   VS_BGR     : lines 208-271 BGR ramp(64 lines)
	//   remainder  : black

	// MAME doesn't scandouble, so FPGA line counts are halved (>> 1)
	static constexpr unsigned BLANK0_LINES   = 48 >> 1;
	static constexpr unsigned RAINBOW_LINES  = 64 >> 1;
	static constexpr unsigned BLANK1_LINES   = 16 >> 1;
	static constexpr unsigned RG_LINES       = 64 >> 1;
	static constexpr unsigned BLANK2_LINES   = 16 >> 1;
	static constexpr unsigned BGR_LINES      = 64 >> 1;

	static constexpr unsigned RAINBOW_START = BLANK0_LINES;
	static constexpr unsigned RAINBOW_END   = RAINBOW_START + RAINBOW_LINES;
	static constexpr unsigned RG_START      = RAINBOW_END + BLANK1_LINES;
	static constexpr unsigned RG_END        = RG_START + RG_LINES;
	static constexpr unsigned BGR_START     = RG_END + BLANK2_LINES;
	static constexpr unsigned BGR_END       = BGR_START + BGR_LINES;

	for (int y = cliprect.top(); y <= cliprect.bottom(); y++)
	{
		const int vrow = y - oy;
		if (vrow < 0 || vrow >= FULL_H)
		{
			u32 *pix = &bitmap.pix(y, cliprect.left());
			for (int x = cliprect.left(); x <= cliprect.right(); x++, pix++)
				*pix = rgb_t::black();
			continue;
		}

		// Determine vertical band
		unsigned band_row = 0;
		int band_id; // 0=rainbow, 1=RG, 2=BGR, -1=blank

		if (vrow >= BGR_START && vrow < BGR_END)
		{
			band_id = 2;
			band_row = vrow - BGR_START;
		}
		else if (vrow >= RG_START && vrow < RG_END)
		{
			band_id = 1;
			band_row = vrow - RG_START;
		}
		else if (vrow >= RAINBOW_START && vrow < RAINBOW_END)
		{
			band_id = 0;
			band_row = vrow - RAINBOW_START;
		}
		else
			band_id = -1;

		bool is_3bit = (band_id >= 0) && (band_row < 8);
		bool is_4bit = (band_id >= 0) && (band_row >= 8 && band_row < 16);

		u32 *pix = &bitmap.pix(y, cliprect.left());
		for (int x = cliprect.left(); x <= cliprect.right(); x++, pix++)
		{
			const int hcol = x - ox;

			if (band_id < 0 || hcol < 0 || hcol >= FULL_W)
			{
				*pix = rgb_t::black();
				continue;
			}

			u8 r8 = 0, g8 = 0, b8 = 0;

			if (band_id == 0) // Rainbow
			{
				const unsigned delta_top = hcol * 255 / FULL_W;
				const unsigned phase = hcol * 512 / FULL_W;

				r8 = u8(~delta_top & 0xff);
				b8 = u8(delta_top & 0xff);
				g8 = (phase < 256) ? u8(phase & 0xff) : u8(~phase & 0xff);
			}
			else if (band_id == 1) // Red/Green ramp
			{
				const unsigned half = FULL_W / 2;
				if (hcol < half)
				{
					r8 = hcol * 255 / half;
				}
				else
				{
					g8 = (hcol - half) * 255 / half;
				}
			}
			else // Blue/Grey ramp
			{
				const unsigned half = FULL_W / 2;
				if (hcol < half)
				{
					b8 = hcol * 255 / half;
				}
				else
				{
					const u8 grey = (hcol - half) * 255 / half;
					r8 = grey;
					g8 = grey;
					b8 = grey;
				}
			}

			if (is_4bit)
			{
				r8 &= 0xf0;
				g8 &= 0xf0;
				b8 &= 0xf0;
			}
			else if (is_3bit)
			{
				r8 &= 0xe0;
				g8 &= 0xe0;
				b8 &= 0xe0;
			}

			*pix = rgb_t(r8, g8, b8);
		}
	}
}

void specnext_vtest_device::device_add_mconfig(machine_config &config)
{
	m_offset_h = 0;
	m_offset_v = 0;
}

void specnext_vtest_device::device_start()
{
	save_item(NAME(m_offset_h));
	save_item(NAME(m_offset_v));
}
