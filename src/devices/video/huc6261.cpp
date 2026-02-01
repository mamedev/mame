// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
/**********************************************************************

    Hudson/NEC HuC6261 Video Colour Encoder

    The HuC6261 generates the tv control signals. A full line lasts
    1365 "master" cycles (typically at 21.47727Mhz).

    HSync is low for 237 and high for 1128 master cycles.
    VSync is low for 4095 master cycles (3 lines).
    VSync changes 30 master cycles after HSync would go low.

    TODO:
    - Allow fetch and mix pixels from other PC-FX video hardwares
    - Implement 'Cellophane' effect

**********************************************************************/

#include "emu.h"
#include "huc6261.h"

#include <algorithm>

//#define VERBOSE 1
#include "logmacro.h"


static constexpr unsigned HUC6261_HSYNC_LENGTH = 237;
static constexpr unsigned HUC6261_HSYNC_START  = (huc6261_device::WPF - HUC6261_HSYNC_LENGTH);

constexpr unsigned huc6261_device::WPF;
constexpr unsigned huc6261_device::LPF;

DEFINE_DEVICE_TYPE(HUC6261, huc6261_device, "huc6261", "Hudson HuC6261 VCE")


huc6261_device::huc6261_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, HUC6261, tag, owner, clock)
	, device_video_interface(mconfig, *this)
	, m_huc6270{*this, { finder_base::DUMMY_TAG, finder_base::DUMMY_TAG }}
	, m_huc6271(*this, finder_base::DUMMY_TAG)
	, m_huc6272(*this, finder_base::DUMMY_TAG)
	, m_last_h(0)
	, m_last_v(0)
	, m_height(0)
	, m_palette{0}
	, m_address(0)
	, m_palette_latch(0)
	, m_register(0)
	, m_control(0)
	, m_priority{0}
	, m_pixels_per_clock(0)
	, m_pixel_data{0}
	, m_palette_offset{0}
	, m_pixel_clock(0)
	, m_timer(nullptr)
{
	// Set up UV lookup table
	for (int ur = 0; ur < 256; ur++)
	{
		for (int vr = 0; vr < 256; vr++)
		{
			const s32 u = ur - 128;
			const s32 v = vr - 128;

			const s32 r =              + 1.13983 * v;
			const s32 g = -0.35465 * u - 0.58060 * v;
			const s32 b =  2.03211 * u;

			m_uv_lookup[(ur << 8) | vr][0] = r;
			m_uv_lookup[(ur << 8) | vr][1] = g;
			m_uv_lookup[(ur << 8) | vr][2] = b;
		}
	}
}


inline u32 huc6261_device::yuv2rgb(u32 yuv) const
{
	const u8 y = yuv >> 8;
	const u16 uv = ((yuv & 0xf0) << 8) | ((yuv & 0xf) << 4);

	const s32 r = std::clamp<s32>(y + m_uv_lookup[uv][0], 0, 255);
	const s32 g = std::clamp<s32>(y + m_uv_lookup[uv][1], 0, 255);
	const s32 b = std::clamp<s32>(y + m_uv_lookup[uv][2], 0, 255);

	return (r << 16) | (g << 8) | b;
}

inline void huc6261_device::apply_pal_offs(u16 &pix_data) const
{
	// sprite
	if (pix_data & huc6270_device::HUC6270_SPRITE)
	{
		pix_data &= 0xff;
		pix_data += ((m_palette_offset[0] & 0xff00) >> 8) << 1;
	}
	else // background
		pix_data += (m_palette_offset[0] & 0xff) << 1;

	pix_data &= 0x1ff;
}

TIMER_CALLBACK_MEMBER(huc6261_device::update_events)
{
	const int vpos = screen().vpos();
	const int hpos = screen().hpos();
	int h = m_last_h;
	int v = m_last_v;
	u32 *bitmap_line = &m_bmp.pix(v);

	while (h != hpos || v != vpos)
	{
		if (m_pixel_clock == 0)
		{
			auto profile = g_profiler.start(PROFILER_VIDEO);
			/* Get next pixel information */
			m_pixel_data[0] = m_huc6270[0]->next_pixel();
			m_pixel_data[1] = m_huc6270[1]->next_pixel();
			apply_pal_offs(m_pixel_data[0]);
			apply_pal_offs(m_pixel_data[1]);
		}

		bitmap_line[h] = yuv2rgb(m_palette[m_pixel_data[0]]);
		// TODO: is mixing correct?
		if ((m_pixel_data[1] & 0xff) != 0)
			bitmap_line[h] = yuv2rgb(m_palette[m_pixel_data[1]]);

		m_pixel_clock = (m_pixel_clock + 1) % m_pixels_per_clock;
		h = (h + 1) % WPF;

		switch (h)
		{
		case HUC6261_HSYNC_START:       /* Start of HSync */
			m_huc6270[0]->hsync_changed(0);
			m_huc6270[1]->hsync_changed(0);
//          if (v == 0)
//          {
//              /* Check if the screen should be resized */
//              m_height = LPF - (m_blur ? 1 : 0);
//              if (m_height != video_screen_get_height(m_screen))
//              {
//                  rectangle visible_area;
//
//                  /* TODO: Set proper visible area parameters */
//                  visible_area.min_x = 64;
//                  visible_area.min_y = 18;
//                  visible_area.max_x = 64 + 1024 + 64 - 1;
//                  visible_area.max_y = 18 + 242 - 1;
//
//                  video_screen_configure(m_screen, WPF, m_height, &visible_area, HZ_TO_ATTOSECONDS(device->clock / (WPF * m_height)));
//              }
//          }
			break;

		case 0:     /* End of HSync */
			m_huc6270[0]->hsync_changed(1);
			m_huc6270[1]->hsync_changed(1);
			m_pixel_clock = 0;
			v = (v + 1) % m_height;
			bitmap_line = &m_bmp.pix(v);
			break;

		case HUC6261_HSYNC_START + 30:      /* End/Start of VSync */
			if (v >= m_height - 4)
			{
				const int vsync = (v >= m_height - 4 && v < m_height - 1) ? 0 : 1;

				m_huc6270[0]->vsync_changed(vsync);
				m_huc6270[1]->vsync_changed(vsync);
			}
			break;
		}
	}

	m_last_h = h;
	m_last_v = v;

	/* Reschedule timer */
	if (m_last_h < HUC6261_HSYNC_START)
	{
		/* Next event is start of HSync signal */
		v = m_last_v;
		h = HUC6261_HSYNC_START;
	}
	else if ((m_last_v == m_height - 4 || m_last_v == m_height - 1) && m_last_h < HUC6261_HSYNC_START + 30)
	{
		/* Next event is start/end of VSync signal */
		v = m_last_v;
		h = HUC6261_HSYNC_START + 30;
	}
	else
	{
		/* Next event is end of HSync signal */
		v = (m_last_v + 1) % m_height;
		h = 0;
	}

	/* Ask our slave device for time until next possible event */
	{
		u16 next_event_clocks = WPF; //m_get_time_til_next_event(0, 0xffff);

		/* Adjust for pixel clocks per pixel */
		next_event_clocks *= m_pixels_per_clock;

		/* Adjust for clocks left to go for current pixel */
		next_event_clocks += (m_pixels_per_clock - (m_pixel_clock + 1));

		int event_hpos = hpos + next_event_clocks;
		int event_vpos = vpos;
		while (event_hpos > WPF)
		{
			event_vpos += 1;
			event_hpos -= WPF;
		}

		if (event_vpos < v || (event_vpos == v && event_hpos <= h))
		{
			if (event_vpos > vpos || (event_vpos == vpos && event_hpos > hpos))
			{
				v = event_vpos;
				h = event_hpos;
			}
		}
	}

	m_timer->adjust(screen().time_until_pos(v, h));
}


u32 huc6261_device::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	copybitmap(bitmap, m_bmp, 0, 0, 0, 0, cliprect);
	return 0;
}


u16 huc6261_device::read(offs_t offset)
{
	u16 data = 0xffff;

	switch (offset & 1)
	{
		/* Status info */
		case 0x00:
			{
				const u16 vpos = screen().vpos();
				const u16 hpos = screen().hpos();

				data = (vpos << 5) | (m_register & 0x1f);

				if (vpos >= 22 && vpos < 262 && hpos < HUC6261_HSYNC_START)
				{
					data |= 0x8000;
				}
			}
			break;

		/* Register contents(?) */
		case 0x01:
			switch (m_register)
			{
				case 0x00:
					data = m_control;
					break;

				case 0x01:
					data = m_address;
					break;

				case 0x02:
				case 0x03:
					data = m_palette_latch;
					if (!machine().side_effects_disabled())
					{
						m_address = (m_address + 1) & 0x1ff;
						m_palette_latch = m_palette[m_address];
					}
					break;

				case 0x08:
					data = m_priority[4] | (m_priority[5] << 4) | (m_priority[6] << 8);
					break;

				case 0x09:
					data = m_priority[0] | (m_priority[1] << 4) | (m_priority[2] << 8) | (m_priority[3] << 12);
					break;
			}
			break;
	}

	return data;
}


void huc6261_device::write(offs_t offset, u16 data)
{
	switch (offset & 1)
	{
		/* Register */
		case 0x00:
			m_register = data;
			break;

		case 0x01:
			//logerror("huc6261: writing 0x%04x to register 0x%02x\n", data, m_register);
			switch (m_register)
			{
				/* Control register */
				// -x-- ---- ---- ---- Enable HuC6271: 0 - disabled, 1 - enabled
				// --x- ---- ---- ---- Enable HuC6272 BG3: 0 - disabled, 1 - enabled
				// ---x ---- ---- ---- Enable HuC6272 BG2: 0 - disabled, 1 - enabled
				// ---- x--- ---- ---- Enable HuC6272 BG1: 0 - disabled, 1 - enabled
				// ---- -x-- ---- ---- Enable HuC6272 BG0: 0 - disabled, 1 - enabled
				// ---- --x- ---- ---- Enable HuC6270 SPR: 0 - disabled, 1 - enabled
				// ---- ---x ---- ---- Enable HuC6270 BG: 0 - disabled, 1 - enabled
				// ---- ---- x--- ---- Number of SPR colors?: 0 - 16, 1 - 256
				// ---- ---- -x-- ---- Number of BG colors?: 0 - 16, 1 - 256
				// ---- ---- ---- x--- Dot clock: 0 - 5MHz, 1 - 7MHz
				// ---- ---- ---- -x-- Synchronization: 0 - internal, 1 - external
				// ---- ---- ---- --xx Screen height: 00 - 262 lines, 01 - 263 lines, 10 - interlace, 11 - unknown/undefined
				case 0x00:
					m_control = data;
					m_pixels_per_clock = BIT(data, 3) ? 3 : 4;
					break;

				// Palette address
				case 0x01:
					m_address = data & 0x1ff;
					m_palette_latch = m_palette[m_address];
					break;

				// Palette data
				case 0x02:
					m_palette_latch = data;
					m_palette[m_address] = m_palette_latch;
					m_address = (m_address + 1) & 0x1ff;
					break;

				// Palette offset 0-3
				// despite what the documentation claims this is really full 8-bit (Audio CD player)
				// VCE Sprite & BG
				case 0x04:
				// King BG 0 & 1
				case 0x05:
				// King BG 2 & 3
				case 0x06:
				// <unused> & Rainbow
				case 0x07:
					m_palette_offset[m_register & 3] = data;
					break;

				// Priority 0
				// -----xxx-------- HuC6271 Rainbow priority
				// ---------xxx---- HuC6270 SPR priority
				// -------------xxx HuC6270 BG priority
				case 0x08:
					m_priority[4] = (data >> 0) & 0x07;
					m_priority[5] = (data >> 4) & 0x07;
					m_priority[6] = (data >> 8) & 0x07;
					break;

				// Priority 1
				// -xxx------------ HuC6272 BG3 priority
				// -----xxx-------- HuC6272 BG2 priority
				// ---------xxx---- HuC6272 BG1 priority
				// -------------xxx HuC6272 BG0 priority
				case 0x09:
					m_priority[0] = (data >>  0) & 0x07;
					m_priority[1] = (data >>  4) & 0x07;
					m_priority[2] = (data >>  8) & 0x07;
					m_priority[3] = (data >> 12) & 0x07;
					break;

				// Chroma key Y
				case 0x0a:
					break;

				// Chroma key U
				case 0x0b:
					break;

				// Chroma key V
				case 0x0c:
					break;

				//
				case 0x0d:
					break;

				//
				case 0x0e:
					break;

				//
				case 0x0f:
					break;

				//
				case 0x10:
					break;

				//
				case 0x11:
					break;

				//
				case 0x12:
					break;

				//
				case 0x13:
					break;

				//
				case 0x14:
					break;

				//
				case 0x15:
					break;
			}
			break;
	}
}


void huc6261_device::device_start()
{
	m_timer = timer_alloc(FUNC(huc6261_device::update_events), this);

	m_bmp.allocate(WPF, LPF);

	save_item(NAME(m_last_h));
	save_item(NAME(m_last_v));
	save_item(NAME(m_height));
	save_item(NAME(m_palette));
	save_item(NAME(m_palette_latch));
	save_item(NAME(m_address));
	save_item(NAME(m_register));
	save_item(NAME(m_control));
	save_item(NAME(m_priority));
	save_item(NAME(m_pixels_per_clock));
	save_item(NAME(m_pixel_data));
	save_item(NAME(m_pixel_clock));
	save_item(NAME(m_bmp));
}


void huc6261_device::device_reset()
{
	m_register = 0;
	m_pixels_per_clock = 4;
	m_height = 263;
	m_pixel_clock = 0;

	memset(m_palette, 0, sizeof(m_palette));

	m_last_v = screen().vpos();
	m_last_h = screen().hpos();
	m_timer->adjust(screen().time_until_pos((screen().vpos() + 1) % 263, 0));
}
