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
    - Implement 'Cellophane' effect (layer blending, used by the BIOS menu message box)
    - Interlace and 7MHz dot clock alignment with the KING/RAINBOW layers

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


inline u32 huc6261_device::yuv888_to_rgb(u32 yuv) const
{
	const u8 y = yuv >> 16;
	const u16 uv = yuv & 0xffff;

	const s32 r = std::clamp<s32>(y + m_uv_lookup[uv][0], 0, 255);
	const s32 g = std::clamp<s32>(y + m_uv_lookup[uv][1], 0, 255);
	const s32 b = std::clamp<s32>(y + m_uv_lookup[uv][2], 0, 255);

	return (r << 16) | (g << 8) | b;
}

// palette entries are Y8 U4 V4
inline u32 huc6261_device::palette_yuv888(u16 index) const
{
	const u16 entry = m_palette[index & 0x1ff];
	return ((entry & 0xff00) << 8) | ((entry & 0xf0) << 8) | ((entry & 0x0f) << 4);
}

/*
 * Layer priority, rebuilt at the start of each line.
 * Each layer has a 3-bit priority (0 = backmost) and an enable bit in the control register,
 * equal priorities are ordered KING BG0-3, VDC BG, VDC SPR, RAINBOW from back to front.
 * The resulting rank is 0 for a hidden layer, otherwise 1 (backmost) to 7.
 */
void huc6261_device::update_layer_ranks()
{
	u8 value[LAYER_COUNT];
	for (int n = 0; n < 4; n++)
		value[LAYER_KING_BG0 + n] = BIT(m_control, 10 + n) ? m_priority[n] + 1 : 0;
	value[LAYER_VDC_BG] = BIT(m_control, 8) ? m_priority[4] + 1 : 0;
	value[LAYER_VDC_SPR] = BIT(m_control, 9) ? m_priority[5] + 1 : 0;
	value[LAYER_RAINBOW] = BIT(m_control, 14) ? m_priority[6] + 1 : 0;

	std::fill(std::begin(m_layer_rank), std::end(m_layer_rank), 0);
	u8 rank = 1;
	for (int prio = 1; prio <= 8; prio++)
		for (int layer = 0; layer < LAYER_COUNT; layer++)
			if (value[layer] == prio)
				m_layer_rank[layer] = rank++;
}

/*
 * Combines the HuC6270 pair and the KING BG line into the output colour for one dot.
 * vdc0/vdc1 are raw HuC6270 pixels (bit 8 = sprite), king is a KING BG line entry
 * (bits 31-28 layer 1-4, bit 27 direct colour, low bits palette index or YUV888),
 * rainbow is a RAINBOW line entry (0 = transparent, bit 27 YUV888, else palette index).
 */
u32 huc6261_device::mix_pixel(u16 vdc0, u16 vdc1, u32 king, u32 rainbow) const
{
	// HuC6270 pair: the second chip covers the first, unless the 256 colour combination modes are on
	u16 vdc;
	if (BIT(m_control, 7) && (vdc1 & 0x18f) > 0x180)
		vdc = (vdc1 & 0xf) | ((vdc0 & 0xf) << 4) | 0x100;
	else if (BIT(m_control, 6) && ((vdc1 ^ 0x100) & 0x18f) > 0x180)
		vdc = (vdc1 & 0xf) | ((vdc0 & 0xf) << 4);
	else
		vdc = (vdc1 & 0xf) ? vdc1 : vdc0;

	struct { u8 rank; u32 yuv; } layer[3]{ };

	if (vdc & 0xf)
	{
		const bool sprite = BIT(vdc, 8);
		const u16 offset = ((m_palette_offset[0] >> (sprite ? 8 : 0)) & 0xff) << 1;
		layer[0].rank = m_layer_rank[sprite ? LAYER_VDC_SPR : LAYER_VDC_BG];
		layer[0].yuv = palette_yuv888((vdc & 0xff) + offset);
	}

	if (king >> 28)
	{
		const int n = (king >> 28) - 1;
		layer[1].rank = m_layer_rank[LAYER_KING_BG0 + n];
		if (BIT(king, 27))
			layer[1].yuv = king & 0xffffff;
		else
		{
			const u16 offset = ((m_palette_offset[1 + (n >> 1)] >> (BIT(n, 0) ? 8 : 0)) & 0xff) << 1;
			layer[1].yuv = palette_yuv888((king & 0xff) + offset);
		}
	}

	if (rainbow)
	{
		layer[2].rank = m_layer_rank[LAYER_RAINBOW];
		if (BIT(rainbow, 27))
			layer[2].yuv = rainbow & 0xffffff;
		else
			layer[2].yuv = palette_yuv888((rainbow & 0xff) + ((m_palette_offset[3] & 0xff) << 1));
	}

	// hindmost colour is palette entry 0, black if no layer is enabled; the frontmost visible layer wins
	u32 out = (m_control & 0x7f00) ? palette_yuv888(0) : 0x008080;
	u8 front = 0;
	for (auto &l : layer)
	{
		if (l.rank > front)
		{
			out = l.yuv;
			front = l.rank;
		}
	}

	return yuv888_to_rgb(out);
}

TIMER_CALLBACK_MEMBER(huc6261_device::update_events)
{
	const int vpos = screen().vpos();
	const int hpos = screen().hpos();
	int h = m_last_h;
	int v = m_last_v;
	u32 *bitmap_line = &m_bmp.pix(v);

	// KING dots are 4 master clocks wide in both dot clock modes
	const int king_start = BIT(m_control, 3) ? KING_START_7MHZ : KING_START_5MHZ;

	while (h != hpos || v != vpos)
	{
		const int king_x = h - king_start;
		const bool king_dot = king_x >= 0 && king_x < 256 * 4 && (king_x & 3) == 0;

		if (m_pixel_clock == 0 || king_dot || h == king_start + 256 * 4)
		{
			auto profile = g_profiler.start(PROFILER_VIDEO);
			if (m_pixel_clock == 0)
			{
				/* Get next pixel information */
				m_pixel_data[0] = m_huc6270[0]->next_pixel();
				m_pixel_data[1] = m_huc6270[1]->next_pixel();
			}
			const bool in_window = king_x >= 0 && king_x < 256 * 4;
			const u32 king = in_window ? m_king_line[king_x >> 2] : 0;
			const u32 rainbow = in_window ? m_rainbow_line[king_x >> 2] : 0;
			m_pixel_rgb = mix_pixel(m_pixel_data[0], m_pixel_data[1], king, rainbow);
		}

		bitmap_line[h] = m_pixel_rgb;

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
			start_line(v);
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


// latch per-line state and render the KING layers for display line v
void huc6261_device::start_line(int v)
{
	update_layer_ranks();

	// RAINBOW decodes on every line, including blanking
	const int format = m_huc6272->rainbow_line(v, m_rainbow_line);

	if (v >= 22 && v < 262)
	{
		m_huc6272->render_bg_line(v - 22, m_king_line);

		if (format == huc6271_device::FORMAT_YUV)
		{
			// chroma key, Y/U/V each in [min, max] (low/high byte) becomes transparent, not in 7MHz mode
			const u8 ymin = m_chroma_key[0] & 0xff, ymax = m_chroma_key[0] >> 8;
			const u8 umin = m_chroma_key[1] & 0xff, umax = m_chroma_key[1] >> 8;
			const u8 vmin = m_chroma_key[2] & 0xff, vmax = m_chroma_key[2] >> 8;
			const bool all_zero = !(m_chroma_key[0] | m_chroma_key[1] | m_chroma_key[2]);
			for (auto &pix : m_rainbow_line)
			{
				const u8 y = pix >> 16, u = pix >> 8, v = pix;
				bool key = false;
				if (!BIT(m_control, 3))
				{
					if (all_zero)
						key = !(pix & 0xffffff);
					else
						key = y >= ymin && y <= ymax && u >= umin && u <= umax && v >= vmin && v <= vmax;
				}
				pix = key ? 0 : (pix | (1 << 27));
			}
		}
	}
	else
	{
		std::fill(std::begin(m_king_line), std::end(m_king_line), 0);
		std::fill(std::begin(m_rainbow_line), std::end(m_rainbow_line), 0);
	}
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

				// Chroma key Y, U, V: max in the high byte, min in the low byte
				case 0x0a:
				case 0x0b:
				case 0x0c:
					m_chroma_key[m_register - 0x0a] = data;
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
	save_item(NAME(m_pixel_rgb));
	save_item(NAME(m_king_line));
	save_item(NAME(m_rainbow_line));
	save_item(NAME(m_chroma_key));
	save_item(NAME(m_layer_rank));
}


void huc6261_device::device_reset()
{
	m_register = 0;
	m_pixels_per_clock = 4;
	m_height = 263;
	m_pixel_clock = 0;

	memset(m_palette, 0, sizeof(m_palette));
	m_pixel_rgb = 0;
	std::fill(std::begin(m_king_line), std::end(m_king_line), 0);
	std::fill(std::begin(m_rainbow_line), std::end(m_rainbow_line), 0);
	std::fill(std::begin(m_chroma_key), std::end(m_chroma_key), 0);
	std::fill(std::begin(m_layer_rank), std::end(m_layer_rank), 0);

	m_last_v = screen().vpos();
	m_last_h = screen().hpos();
	m_timer->adjust(screen().time_until_pos((screen().vpos() + 1) % 263, 0));
}
