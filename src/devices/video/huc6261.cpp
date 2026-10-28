// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
/**********************************************************************

    Hudson/NEC HuC6261 Video Colour Encoder

    The HuC6261 generates the tv control signals. A full line lasts
    1365 "master" cycles (typically at 21.47727Mhz).

    HSync is low for 237 and high for 1128 master cycles.
    VSync is low for 4095 master cycles (3 lines), lines 0-2.
    VSync changes 30 master cycles after HSync would go low.

    TODO:
    - 1/2 dot shift in interlace mode (DCC = 3)
    - 7MHz dot clock alignment with the KING/RAINBOW layers

**********************************************************************/

#include "emu.h"
#include "huc6261.h"

#include <algorithm>

//#define VERBOSE 1
#include "logmacro.h"


static constexpr unsigned HUC6261_HSYNC_LENGTH = 237;
static constexpr unsigned HUC6261_HSYNC_START  = (huc6261_device::WPF - HUC6261_HSYNC_LENGTH);

DEFINE_DEVICE_TYPE(HUC6261, huc6261_device, "huc6261", "Hudson HuC6261 VCE")


huc6261_device::huc6261_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, HUC6261, tag, owner, clock)
	, device_video_interface(mconfig, *this)
	, m_huc6270{*this, { finder_base::DUMMY_TAG, finder_base::DUMMY_TAG }}
	, m_huc6272(*this, finder_base::DUMMY_TAG)
	, m_last_h(0)
	, m_last_v(0)
	, m_height(LPF)
	, m_interlace(false)
	, m_odd_field(false)
	, m_palette{0}
	, m_address(0)
	, m_palette_latch(0)
	, m_register(0)
	, m_control(0)
	, m_control_reg(0)
	, m_priority{0}
	, m_pixels_per_clock(0)
	, m_pixel_data{0}
	, m_palette_offset{0}
	, m_pixel_clock(0)
	, m_timer(nullptr)
{
	// Set up UV lookup table: the inverse of the RGB to YUV matrix in the HuC6271 manual
	// (Y = 0.2990R + 0.5870G + 0.1140B, U = -0.1686R - 0.3311G + 0.4997B + 128,
	// V = 0.4998R - 0.4185G - 0.0813B + 128), which is JPEG YCbCr
	for (int ur = 0; ur < 256; ur++)
	{
		for (int vr = 0; vr < 256; vr++)
		{
			const s32 u = ur - 128;
			const s32 v = vr - 128;

			const s32 r =              + 1.40200 * v;
			const s32 g = -0.34414 * u - 0.71414 * v;
			const s32 b =  1.77200 * u;

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
inline u32 huc6261_device::palette_yuv888_entry(u16 entry)
{
	return ((entry & 0xff00) << 8) | ((entry & 0xf0) << 8) | ((entry & 0x0f) << 4);
}

inline u32 huc6261_device::palette_yuv888(u16 index) const
{
	return palette_yuv888_entry(m_palette[index & 0x1ff]);
}

// cellophane coefficient set (0 = none, 1-3) for a layer
inline u8 huc6261_device::cello_set(u8 layer) const
{
	switch (layer)
	{
		case LAYER_VDC_BG:  return (m_cello_enable >> 0) & 3;
		case LAYER_VDC_SPR: return (m_cello_enable >> 2) & 3;
		case LAYER_RAINBOW: return (m_cello_enable >> 12) & 3;
		default:            return (m_cello_enable >> (4 + (layer - LAYER_KING_BG0) * 2)) & 3;
	}
}

// front * front_coeff + back * back_coeff, per component, coefficients in 1/8 (Y, U, V nibbles)
u32 huc6261_device::cello_blend(u16 front_coeff, u32 front, u16 back_coeff, u32 back)
{
	auto scale_y = [] (u32 yuv, u16 coeff) { return int((yuv >> 16) & 0xff) * int((coeff >> 8) & 0xf) / 8; };
	auto scale_uv = [] (u32 yuv, int shift, u16 coeff, int cshift) { return (int((yuv >> shift) & 0xff) - 128) * int((coeff >> cshift) & 0xf) / 8; };

	const int y = std::clamp(scale_y(back, back_coeff) + scale_y(front, front_coeff), 0, 255);
	const int u = std::clamp(scale_uv(back, 8, back_coeff, 4) + scale_uv(front, 8, front_coeff, 4) + 128, 0, 255);
	const int v = std::clamp(scale_uv(back, 0, back_coeff, 0) + scale_uv(front, 0, front_coeff, 0) + 128, 0, 255);
	return (y << 16) | (u << 8) | v;
}

/*
 * Layer priority, rebuilt at the start of each line.
 * Each layer has a 3-bit priority (0 = backmost) and an enable bit in the control register.
 * Software is not meant to give two layers the same priority; the manual gives
 * HuC6270 > HuC6272 > HuC6271 for the cleared registers, so equal priorities are ordered
 * RAINBOW, KING BG0-3, VDC BG, VDC SPR from back to front.
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
	static constexpr u8 TIE_ORDER[LAYER_COUNT] = { LAYER_RAINBOW, LAYER_KING_BG0, LAYER_KING_BG0 + 1, LAYER_KING_BG0 + 2, LAYER_KING_BG0 + 3, LAYER_VDC_BG, LAYER_VDC_SPR };
	u8 rank = 1;
	for (int prio = 1; prio <= 8; prio++)
	{
		for (u8 layer : TIE_ORDER)
		{
			if (value[layer] == prio)
				m_layer_rank[layer] = rank++;
		}
	}
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

	struct { u8 rank; u8 id; u32 yuv; } layer[3]{ };

	if (vdc & 0xf)
	{
		const bool sprite = BIT(vdc, 8);
		const u16 offset = ((m_palette_offset[0] >> (sprite ? 8 : 0)) & 0xff) << 1;
		layer[0].id = sprite ? LAYER_VDC_SPR : LAYER_VDC_BG;
		layer[0].rank = m_layer_rank[layer[0].id];
		layer[0].yuv = palette_yuv888((vdc & 0xff) + offset);
	}

	if (king >> 28)
	{
		const int n = (king >> 28) - 1;
		layer[1].id = LAYER_KING_BG0 + n;
		layer[1].rank = m_layer_rank[layer[1].id];
		if (BIT(king, 27))
		{
			layer[1].yuv = king & 0xffffff;
		}
		else
		{
			const u16 offset = ((m_palette_offset[1 + (n >> 1)] >> (BIT(n, 0) ? 8 : 0)) & 0xff) << 1;
			layer[1].yuv = palette_yuv888((king & 0xff) + offset);
		}
	}

	if (rainbow)
	{
		layer[2].id = LAYER_RAINBOW;
		layer[2].rank = m_layer_rank[LAYER_RAINBOW];
		if (BIT(rainbow, 27))
			layer[2].yuv = rainbow & 0xffffff;
		else
			layer[2].yuv = palette_yuv888((rainbow & 0xff) + ((m_palette_offset[3] & 0xff) << 1));
	}

	// draw back to front
	if (layer[0].rank > layer[1].rank)
		std::swap(layer[0], layer[1]);
	if (layer[1].rank > layer[2].rank)
		std::swap(layer[1], layer[2]);
	if (layer[0].rank > layer[1].rank)
		std::swap(layer[0], layer[1]);

	// no cellophane with the 7MHz dot clock
	const bool cello = !BIT(m_control, 3);
	const u8 cello_mode = cello ? (m_cello_enable >> 14) : 0;

	// hindmost colour is palette entry 0, black if no layer is enabled, or the cellophane colour/black in back/front mode
	u32 out;
	if (cello_mode == 1)
		out = palette_yuv888_entry(m_cello_colour);
	else if (cello_mode == 3)
		out = 0x008080;
	else
		out = (m_control & 0x7f00) ? palette_yuv888(0) : 0x008080;

	bool out_is_layer = false;
	for (auto &l : layer)
	{
		if (!l.rank)
			continue;

		const u8 set = cello ? cello_set(l.id) : 0;
		const bool sprite_ok = l.id != LAYER_VDC_SPR || BIT(m_cello_sprite_mask, (vdc >> 4) & 0xf);
		// back cellophane also blends with the hindmost colour, otherwise only with another layer
		if (set && sprite_ok && (cello_mode == 1 || out_is_layer))
			out = cello_blend(m_cello_coeff[(set - 1) * 2], l.yuv, m_cello_coeff[(set - 1) * 2 + 1], out);
		else
			out = l.yuv;
		out_is_layer = true;
	}

	// front cellophane: the cellophane colour over everything, shown as is where every layer is transparent
	if (cello_mode == 3)
		out = out_is_layer ? cello_blend(m_cello_coeff[0], palette_yuv888_entry(m_cello_colour), m_cello_coeff[1], out) : palette_yuv888_entry(m_cello_colour);

	return yuv888_to_rgb(out);
}

// the screen counts at twice the master clock, so a screen line is half a display line
std::pair<int, int> huc6261_device::beam_position() const
{
	const u32 clocks = (u32(screen().vpos()) * WPF + screen().hpos()) / 2;
	return std::make_pair(int(clocks / WPF), int(clocks % WPF));
}

attotime huc6261_device::time_until_beam(int v, int h) const
{
	const u32 pos = (u32(v) * WPF + h) * 2;
	return screen().time_until_pos(pos / WPF, pos % WPF);
}

// interlace starts with the odd field, then the fields alternate
void huc6261_device::start_frame()
{
	const bool interlace = BIT(m_control, 1);
	m_odd_field = interlace && (!m_interlace || !m_odd_field);
	m_interlace = interlace;
}

TIMER_CALLBACK_MEMBER(huc6261_device::update_events)
{
	const auto [vpos, hpos] = beam_position();
	int h = m_last_h;
	int v = m_last_v;

	// each display line covers two bitmap rows: both when not interlaced, the odd field on the lower one
	u32 *bitmap_line = nullptr;
	u32 *bitmap_line2 = nullptr;
	auto set_rows = [this, &v, &bitmap_line, &bitmap_line2] ()
	{
		bitmap_line = &m_bmp.pix(v * 2 + (m_odd_field ? 1 : 0));
		bitmap_line2 = m_interlace ? nullptr : &m_bmp.pix(v * 2 + 1);
	};
	set_rows();

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
		if (bitmap_line2)
			bitmap_line2[h] = m_pixel_rgb;

		m_pixel_clock = (m_pixel_clock + 1) % m_pixels_per_clock;
		h = (h + 1) % WPF;

		switch (h)
		{
		case HUC6261_HSYNC_START:       /* Start of HSync */
			m_huc6270[0]->hsync_changed(0);
			m_huc6270[1]->hsync_changed(0);
			m_huc6272->rainbow_hsync();
			break;

		case 0:     /* End of HSync */
			m_huc6270[0]->hsync_changed(1);
			m_huc6270[1]->hsync_changed(1);
			m_pixel_clock = 0;
			v = (v + 1) % m_height;
			if (v == 0)
			{
				m_control = m_control_reg;
				m_pixels_per_clock = BIT(m_control, 3) ? 3 : 4;
				start_frame();
			}
			else
			{
				m_control = (m_control & 0x80ff) | (m_control_reg & 0x7f00);
			}

			// the frame length (DCC 01 = 262 lines, otherwise 263) is applied at the start of vblank,
			// where the screen can be reconfigured without moving the beam
			if (v == 260)
			{
				const int height = ((m_control & 3) == 1) ? 262 : 263;
				if (height != m_height)
				{
					m_height = height;
					screen().configure(WPF, m_height * 2, screen().visible_area(), attotime::from_ticks(u64(WPF) * m_height, clock()));
				}
			}
			set_rows();
			start_line(v);
			break;

		case HUC6261_HSYNC_START + 30:      /* End/Start of VSync */
		{
			// VSync is low for lines 0-2, the HuC6270s time their display from it
			if (v == 0 || v == 3)
			{
				const int vsync = (v == 0) ? 0 : 1;

				m_huc6270[0]->vsync_changed(vsync);
				m_huc6270[1]->vsync_changed(vsync);
			}
			break;
		}
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
	else if ((m_last_v == 0 || m_last_v == 3) && m_last_h < HUC6261_HSYNC_START + 30)
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

	m_timer->adjust(time_until_beam(v, h));
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
				const u8 py = pix >> 16, pu = pix >> 8, pv = pix;
				bool key = false;
				if (!BIT(m_control, 3))
				{
					if (all_zero)
						key = !(pix & 0xffffff);
					else
						key = py >= ymin && py <= ymax && pu >= umin && pu <= umax && pv >= vmin && pv <= vmax;
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
				const auto [vpos, hpos] = beam_position();

				data = (vpos << 5) | (m_register & 0x1f);
				if (m_odd_field)
					data |= 0x4000;

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
					data = m_control_reg;
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

				case 0x04:
				case 0x05:
				case 0x06:
				case 0x07:
					data = m_palette_offset[m_register & 3];
					break;

				case 0x08:
					data = m_priority[4] | (m_priority[5] << 4) | (m_priority[6] << 8);
					break;

				case 0x09:
					data = m_priority[0] | (m_priority[1] << 4) | (m_priority[2] << 8) | (m_priority[3] << 12);
					break;

				case 0x0a:
				case 0x0b:
				case 0x0c:
					data = m_chroma_key[m_register - 0x0a];
					break;

				case 0x0d:
					data = m_cello_colour;
					break;

				// games fade by reading the cellophane registers back and stepping them (dknight4)
				case 0x0e:
					data = m_cello_enable;
					break;

				case 0x0f:
					data = m_cello_sprite_mask;
					break;

				case 0x10:
				case 0x11:
				case 0x12:
				case 0x13:
				case 0x14:
				case 0x15:
					data = m_cello_coeff[m_register - 0x10];
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
			m_register = data & 0x1f;
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
				// ---- ---- ---- --xx Screen height: 00 - 263 lines, 01 - 262 lines, 10 - interlace, 11 - interlace with 1/2 dot shift
				// bits 8-14 take effect from the next line, the others from the next frame
				case 0x00:
					m_control_reg = data;
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
					m_palette_offset[m_register & 3] = data;
					break;

				// <unused> & Rainbow, only the low byte is used
				case 0x07:
					m_palette_offset[3] = data & 0xff;
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

				// Cellophane colour, Y8 U4 V4
				case 0x0d:
					m_cello_colour = data;
					break;

				// Cellophane enable
				// xx-- ---- ---- ---- 01 = back cellophane (blend over the cellophane colour), 11 = front (blend the cellophane colour over the result)
				// --xx ---- ---- ---- RAINBOW coefficient set
				// ---- xxxx xxxx ---- KING BG3-BG0 coefficient sets
				// ---- ---- ---- xx-- HuC6270 SPR coefficient set
				// ---- ---- ---- --xx HuC6270 BG coefficient set
				// coefficient set 0 = no blending, 1-3 = coefficient pair 0-2
				case 0x0e:
					m_cello_enable = data;
					break;

				// HuC6270 sprite palettes subject to cellophane, one bit per palette
				case 0x0f:
					m_cello_sprite_mask = data;
					break;

				// Cellophane coefficient pairs (front, back), Y U V 4 bits each, in 1/8
				case 0x10:
				case 0x11:
				case 0x12:
				case 0x13:
				case 0x14:
				case 0x15:
					m_cello_coeff[m_register - 0x10] = data & 0xfff;
					break;
			}
			break;
	}
}


void huc6261_device::device_start()
{
	m_timer = timer_alloc(FUNC(huc6261_device::update_events), this);

	m_bmp.allocate(WPF, SCREEN_LPF);

	save_item(NAME(m_last_h));
	save_item(NAME(m_last_v));
	save_item(NAME(m_height));
	save_item(NAME(m_interlace));
	save_item(NAME(m_odd_field));
	save_item(NAME(m_palette));
	save_item(NAME(m_palette_latch));
	save_item(NAME(m_palette_offset));
	save_item(NAME(m_address));
	save_item(NAME(m_register));
	save_item(NAME(m_control));
	save_item(NAME(m_control_reg));
	save_item(NAME(m_priority));
	save_item(NAME(m_pixels_per_clock));
	save_item(NAME(m_pixel_data));
	save_item(NAME(m_pixel_clock));
	save_item(NAME(m_pixel_rgb));
	save_item(NAME(m_king_line));
	save_item(NAME(m_rainbow_line));
	save_item(NAME(m_chroma_key));
	save_item(NAME(m_cello_colour));
	save_item(NAME(m_cello_enable));
	save_item(NAME(m_cello_sprite_mask));
	save_item(NAME(m_cello_coeff));
	save_item(NAME(m_layer_rank));
}


void huc6261_device::device_reset()
{
	// all registers are cleared except R0F, which is set to FFh
	m_register = 0;
	m_control = 0;
	m_control_reg = 0;
	m_address = 0;
	m_palette_latch = 0;
	std::fill(std::begin(m_palette_offset), std::end(m_palette_offset), 0);
	std::fill(std::begin(m_priority), std::end(m_priority), 0);
	m_pixels_per_clock = 4;
	m_interlace = false;
	m_odd_field = false;
	if (m_height != 263)
	{
		m_height = 263;
		screen().configure(WPF, SCREEN_LPF, screen().visible_area(), attotime::from_ticks(u64(WPF) * LPF, clock()));
	}
	m_pixel_clock = 0;

	memset(m_palette, 0, sizeof(m_palette));
	m_pixel_rgb = 0;
	std::fill(std::begin(m_king_line), std::end(m_king_line), 0);
	std::fill(std::begin(m_rainbow_line), std::end(m_rainbow_line), 0);
	std::fill(std::begin(m_chroma_key), std::end(m_chroma_key), 0);
	m_cello_colour = 0;
	m_cello_enable = 0;
	m_cello_sprite_mask = 0xff;
	std::fill(std::begin(m_cello_coeff), std::end(m_cello_coeff), 0);
	std::fill(std::begin(m_layer_rank), std::end(m_layer_rank), 0);

	const auto [v, h] = beam_position();
	m_last_v = v;
	m_last_h = h;
	m_timer->adjust(time_until_beam((v + 1) % m_height, 0));
}
