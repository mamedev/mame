// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/***************************************************************************

    Sun cgsix-series accelerated 8-bit color video controller

***************************************************************************/

#include "emu.h"
#include "cgsix.h"

DEFINE_DEVICE_TYPE(SBUS_TURBOGX, sbus_turbogx_device, "turbogx", "Sun TurboGX SBus Video")
DEFINE_DEVICE_TYPE(SBUS_TURBOGXP, sbus_turbogxp_device, "turbogxp", "Sun TurboGX+ SBus Video")

//-------------------------------------------------
//  base cgsix device
//-------------------------------------------------

void sbus_cgsix_device::base_map(address_map &map)
{
	map(0x00000000, 0x01ffffff).rw(FUNC(sbus_cgsix_device::unknown_r), FUNC(sbus_cgsix_device::unknown_w));
	map(0x00000000, 0x00007fff).r(FUNC(sbus_cgsix_device::rom_r));
	map(0x00200000, 0x0020000f).m(m_ramdac, FUNC(bt458_device::map)).umask32(0xff000000);
	map(0x00300000, 0x00300fff).rw(FUNC(sbus_cgsix_device::fbc_r), FUNC(sbus_cgsix_device::fbc_w));
	map(0x00301818, 0x0030181b).rw(FUNC(sbus_cgsix_device::thc_misc_r), FUNC(sbus_cgsix_device::thc_misc_w));
	map(0x003018fc, 0x003018ff).rw(FUNC(sbus_cgsix_device::cursor_address_r), FUNC(sbus_cgsix_device::cursor_address_w));
	map(0x00301900, 0x003019ff).rw(FUNC(sbus_cgsix_device::cursor_ram_r), FUNC(sbus_cgsix_device::cursor_ram_w));
	map(0x00700000, 0x00700fff).rw(FUNC(sbus_cgsix_device::fbc_r), FUNC(sbus_cgsix_device::fbc_w));
}

sbus_cgsix_device::sbus_cgsix_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, type, tag, owner, clock)
	, device_sbus_card_interface(mconfig, *this)
	, m_rom(*this, "prom")
	, m_screen(*this, "screen")
	, m_ramdac(*this, "ramdac")
{
}

void sbus_cgsix_device::device_start()
{
	m_vram = std::make_unique<uint32_t[]>(m_vram_size / 4);
	save_pointer(NAME(m_vram), m_vram_size / 4);
	save_item(NAME(m_vram_size));

	m_cursor_ram = std::make_unique<uint32_t[]>(32 * 2);
	save_pointer(NAME(m_cursor_ram), 32 * 2);

	save_item(NAME(m_fbc.m_config));
	save_item(NAME(m_fbc.m_misc));
	save_item(NAME(m_fbc.m_clip_check));
	save_item(NAME(m_fbc.m_status));
	save_item(NAME(m_fbc.m_draw_status));
	save_item(NAME(m_fbc.m_blit_status));
	save_item(NAME(m_fbc.m_font));

	save_item(NAME(m_fbc.m_x0));
	save_item(NAME(m_fbc.m_y0));
	save_item(NAME(m_fbc.m_z0));
	save_item(NAME(m_fbc.m_color0));
	save_item(NAME(m_fbc.m_x1));
	save_item(NAME(m_fbc.m_y1));
	save_item(NAME(m_fbc.m_z1));
	save_item(NAME(m_fbc.m_color1));
	save_item(NAME(m_fbc.m_x2));
	save_item(NAME(m_fbc.m_y2));
	save_item(NAME(m_fbc.m_z2));
	save_item(NAME(m_fbc.m_color2));
	save_item(NAME(m_fbc.m_x3));
	save_item(NAME(m_fbc.m_y3));
	save_item(NAME(m_fbc.m_z3));
	save_item(NAME(m_fbc.m_color3));

	save_item(NAME(m_fbc.m_raster_offx));
	save_item(NAME(m_fbc.m_raster_offy));

	save_item(NAME(m_fbc.m_autoincx));
	save_item(NAME(m_fbc.m_autoincy));

	save_item(NAME(m_fbc.m_clip_minx));
	save_item(NAME(m_fbc.m_clip_miny));

	save_item(NAME(m_fbc.m_clip_maxx));
	save_item(NAME(m_fbc.m_clip_maxy));

	save_item(NAME(m_fbc.m_fcolor));
	save_item(NAME(m_fbc.m_bcolor));

	save_item(NAME(m_fbc.m_rasterop));

	save_item(NAME(m_fbc.m_plane_mask));
	save_item(NAME(m_fbc.m_pixel_mask));

	save_item(NAME(m_fbc.m_patt_align));
	save_item(NAME(m_fbc.m_patt_align_x));
	save_item(NAME(m_fbc.m_patt_align_y));
	save_item(NAME(m_fbc.m_pattern));

	save_item(NAME(m_fbc.m_ipoint_absx));
	save_item(NAME(m_fbc.m_ipoint_absy));
	save_item(NAME(m_fbc.m_ipoint_absz));
	save_item(NAME(m_fbc.m_ipoint_relx));
	save_item(NAME(m_fbc.m_ipoint_rely));
	save_item(NAME(m_fbc.m_ipoint_relz));
	save_item(NAME(m_fbc.m_ipoint_r));
	save_item(NAME(m_fbc.m_ipoint_g));
	save_item(NAME(m_fbc.m_ipoint_b));
	save_item(NAME(m_fbc.m_ipoint_a));

	save_item(NAME(m_fbc.m_iline_absx));
	save_item(NAME(m_fbc.m_iline_absy));
	save_item(NAME(m_fbc.m_iline_absz));
	save_item(NAME(m_fbc.m_iline_relx));
	save_item(NAME(m_fbc.m_iline_rely));
	save_item(NAME(m_fbc.m_iline_relz));
	save_item(NAME(m_fbc.m_iline_r));
	save_item(NAME(m_fbc.m_iline_g));
	save_item(NAME(m_fbc.m_iline_b));
	save_item(NAME(m_fbc.m_iline_a));

	save_item(NAME(m_fbc.m_itri_absx));
	save_item(NAME(m_fbc.m_itri_absy));
	save_item(NAME(m_fbc.m_itri_absz));
	save_item(NAME(m_fbc.m_itri_relx));
	save_item(NAME(m_fbc.m_itri_rely));
	save_item(NAME(m_fbc.m_itri_relz));
	save_item(NAME(m_fbc.m_itri_r));
	save_item(NAME(m_fbc.m_itri_g));
	save_item(NAME(m_fbc.m_itri_b));
	save_item(NAME(m_fbc.m_itri_a));

	save_item(NAME(m_fbc.m_iquad_absx));
	save_item(NAME(m_fbc.m_iquad_absy));
	save_item(NAME(m_fbc.m_iquad_absz));
	save_item(NAME(m_fbc.m_iquad_relx));
	save_item(NAME(m_fbc.m_iquad_rely));
	save_item(NAME(m_fbc.m_iquad_relz));
	save_item(NAME(m_fbc.m_iquad_r));
	save_item(NAME(m_fbc.m_iquad_g));
	save_item(NAME(m_fbc.m_iquad_b));
	save_item(NAME(m_fbc.m_iquad_a));

	save_item(NAME(m_fbc.m_irect_absx));
	save_item(NAME(m_fbc.m_irect_absy));
	save_item(NAME(m_fbc.m_irect_absz));
	save_item(NAME(m_fbc.m_irect_relx));
	save_item(NAME(m_fbc.m_irect_rely));
	save_item(NAME(m_fbc.m_irect_relz));
	save_item(NAME(m_fbc.m_irect_r));
	save_item(NAME(m_fbc.m_irect_g));
	save_item(NAME(m_fbc.m_irect_b));
	save_item(NAME(m_fbc.m_irect_a));

	save_item(NAME(m_fbc.m_vertex_count));

	m_fbc.m_prim_buf = std::make_unique<vertex[]>(0x1000); // Unknown size on hardware
	save_pointer(reinterpret_cast<uint8_t*>(m_fbc.m_prim_buf.get()), "m_fbc.m_prim_buf", sizeof(vertex) * 0x1000);

	save_item(NAME(m_fbc.m_curr_prim_type));

	save_item(NAME(m_thc_misc));
	save_item(NAME(m_cursor_x));
	save_item(NAME(m_cursor_y));
}

void sbus_cgsix_device::device_reset()
{
	m_fbc.m_vertex_count = 0;
	m_thc_misc = 0;
}

uint32_t sbus_cgsix_device::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	pen_t const *const pens = m_ramdac->pens();
	auto const vram = util::big_endian_cast<uint8_t const>(&m_vram[0]);

	for (int16_t y = 0; y < 900; y++)
	{
		uint32_t *scanline = &bitmap.pix(y);
		const bool cursor_row_hit = (y >= m_cursor_y && y < (m_cursor_y + 32));
		for (int16_t x = 0; x < 1152; x++)
		{
			uint8_t cursor_pixel = 0;
			const bool cursor_column_hit = (x >= m_cursor_x && x < (m_cursor_x + 32));
			if (cursor_row_hit && cursor_column_hit)
			{
				const int16_t cursor_row = y - m_cursor_y;
				const uint32_t cursor_bit = 31 - (x - m_cursor_x);
				const uint32_t cursor_plane_a = m_cursor_ram[cursor_row];
				const uint32_t cursor_plane_b = m_cursor_ram[cursor_row + 32];
				cursor_pixel = (BIT(cursor_plane_b, cursor_bit) << 1) | BIT(cursor_plane_a, cursor_bit);
				if (cursor_pixel)
				{
					*scanline++ = pens[0x100 + cursor_pixel];
					continue;
				}
			}

			const uint8_t pixel = vram[y * 1152 + x];
			*scanline++ = pens[pixel];
		}
	}

	return 0;
}

uint32_t sbus_cgsix_device::rom_r(offs_t offset)
{
	return ((uint32_t*)m_rom->base())[offset];
}

uint32_t sbus_cgsix_device::unknown_r(offs_t offset, uint32_t mem_mask)
{
	logerror("%s: unknown_r: %08x & %08x\n", machine().describe_context(), offset << 2, mem_mask);
	return 0;
}

void sbus_cgsix_device::unknown_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	logerror("%s: unknown_w: %08x = %08x & %08x\n", machine().describe_context(), offset << 2, data, mem_mask);
}

uint32_t sbus_cgsix_device::vram_r(offs_t offset)
{
	return m_vram[offset];
}

void sbus_cgsix_device::vram_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&m_vram[offset]);
}

uint8_t sbus_cgsix_device::perform_rasterop(uint8_t src, uint8_t dst, uint8_t mask)
{
	const uint32_t rops[4] = { fbc_rasterop_rop00(), fbc_rasterop_rop01(), fbc_rasterop_rop10(), fbc_rasterop_rop11() };

	if (fbc_misc_data() == FBC_MISC_DATA_COLOR1)
	{
		src = BIT(src, 0) * 0xff;
		dst = BIT(dst, 0) * 0xff;
	}

	uint8_t result = 0;
	//logerror("src:%02x dst:%02x\n", src, dst);
	for (int bit = 0; bit < 8; bit++)
	{
		const uint8_t mask = (1 << bit);
		const uint8_t f = BIT(m_fbc.m_fcolor, bit) << 1;
		const uint8_t b = BIT(m_fbc.m_bcolor, bit);
		const uint8_t s = src & mask;
		const uint8_t d = dst & mask;
		const uint32_t rop = rops[f | b];
		//if (fbc_misc_data() == FBC_MISC_DATA_COLOR8)
			//logerror("f:%d b:%d s:%02x d:%02x rop:%d\n", f >> 1, b, s, d, rop);

		uint8_t value = 0;
		switch (rop)
		{
			case ROP_CLR:                               break;
			case ROP_SRC_NOR_DST:   value = ~(s | d);   break;
			case ROP_NSRC_AND_DST:  value = ~s & d;     break;
			case ROP_NOT_SRC:       value = ~s;         break;
			case ROP_SRC_AND_NDST:  value = s & ~d;     break;
			case ROP_NOT_DST:       value = ~d;         break;
			case ROP_SRC_XOR_DST:   value = s ^ d;      break;
			case ROP_SRC_NAND_DST:  value = ~(s & d);   break;
			case ROP_SRC_AND_DST:   value = s & d;      break;
			case ROP_SRC_XNOR_DST:  value = ~(s ^ d);   break;
			case ROP_DST:           value = d;          break;
			case ROP_NSRC_OR_DST:   value = ~s | d;     break;
			case ROP_SRC:           value = s;          break;
			case ROP_SRC_OR_NDST:   value = s | ~d;     break;
			case ROP_SRC_OR_DST:    value = s | d;      break;
			case ROP_SET:           value = 0xff;       break;
			default:                value = 0;          break;
		}
		result |= value & mask;
	}
	//if (fbc_misc_data() == FBC_MISC_DATA_COLOR8)
		//logerror("result: %02x\n", result);
	return result;
}

void sbus_cgsix_device::handle_font_poke()
{
	if (fbc_misc_draw() > FBC_MISC_DRAW_RENDER)
	{
		logerror("handle_font_poke: Unsupported font draw mode %d, abandoning draw\n", fbc_misc_draw());
		return;
	}

	const bool color8 = (fbc_misc_data() == FBC_MISC_DATA_COLOR8);

	uint32_t pixel_mask = fbc_get_pixel_mask();
	uint8_t plane_mask = fbc_get_plane_mask();

	const uint32_t daddr = m_fbc.m_y0 * 1152;
	auto const vram = util::big_endian_cast<uint8_t>(&m_vram[0]) + daddr;
	const int width = (int)m_fbc.m_x1 - (int)m_fbc.m_x0;
	const uint32_t font = m_fbc.m_font;
	uint32_t x = m_fbc.m_x0;
	//logerror("Width: %d, bits %d to %d\n", width, 31, 31 - width);
	int step_size = color8 ? 8 : 1;
	int start_bit = 32 - step_size;

	for (int bit = start_bit; bit >= (start_bit - width * step_size) && x < 1152; bit -= step_size, x++)
	{
		if (!BIT(pixel_mask, 31 - (x % 32)))
			continue;

		uint8_t src = 0;
		if (color8)
			src = (font >> bit) & 0xff;
		else
			src = BIT(font, bit) ? 0xff : 0x00;
		const uint8_t dst = vram[x];
		vram[x] = perform_rasterop(src, dst, plane_mask);
	}
	m_fbc.m_x0 += m_fbc.m_autoincx;
	m_fbc.m_x1 += m_fbc.m_autoincx;
	m_fbc.m_y0 += m_fbc.m_autoincy;
	m_fbc.m_y1 += m_fbc.m_autoincy;
}

uint32_t sbus_cgsix_device::fbc_get_pixel_mask()
{
	switch (fbc_rasterop_pixel())
	{
	default: // Ignore
		return 0xffffffff;
	case 1: // Zeros
		return 0x00000000;
	case 2: // Ones
		return 0xffffffff;
		break;
	case 3: // Mask
		return m_fbc.m_pixel_mask;
	}
}

uint8_t sbus_cgsix_device::fbc_get_plane_mask()
{
	switch (fbc_rasterop_plane())
	{
	default: // Ignore
		return 0xff;
	case 1: // Zeros
		return 0x00;
	case 2: // Ones
		return 0xff;
		break;
	case 3: // Mask
		return (uint8_t)m_fbc.m_plane_mask;
	}
}

// NOTE: This is basically untested, and probably full of bugs!
void sbus_cgsix_device::handle_draw_command()
{
	if (fbc_misc_draw() > FBC_MISC_DRAW_RENDER)
	{
		logerror("handle_draw_command: Unsupported draw mode %d, abandoning draw\n", fbc_misc_draw());
		return;
	}

	if (m_fbc.m_curr_prim_type != PRIM_RECT)
	{
		logerror("handle_draw_command: Unsupported prim type %d, abandoning draw\n", m_fbc.m_curr_prim_type);
		return;
	}

	if (m_fbc.m_vertex_count < 2)
	{
		logerror("handle_draw_command: Insufficient number of vertices queued, abandoning draw\n");
		return;
	}

	auto const vram = util::big_endian_cast<uint8_t>(&m_vram[0]);

	uint32_t pixel_mask = fbc_get_pixel_mask();
	uint8_t plane_mask = fbc_get_plane_mask();

	uint32_t vindex = 0;
	while (vindex < m_fbc.m_vertex_count)
	{
		vertex &v0 = m_fbc.m_prim_buf[vindex++];
		vertex &v1 = m_fbc.m_prim_buf[vindex++];

		for (uint32_t y = v0.m_absy; y <= v1.m_absy; y++)
		{
			auto const line = vram + (y * 1152);
			const uint16_t patt_y_index = (y - m_fbc.m_patt_align_y) % 16;
			for (uint32_t x = v0.m_absx; x <= v1.m_absx; x++)
			{
				if (!BIT(pixel_mask, 31 - (x % 32)))
					continue;

				uint8_t src = line[x];

				switch (fbc_rasterop_pattern())
				{
				default: // Ignore
					break;
				case 1: // Zeroes
					src = 0x00;
					break;
				case 2: // Ones
					src = 0xff;
					break;
				case 3: // Pattern register
				{
					const uint16_t patt_x_index = 15 - ((x - m_fbc.m_patt_align_x) % 16);
					src = BIT(m_fbc.m_patterns[patt_y_index], patt_x_index) ? 0xff : 0x00;
					break;
				}
				}

				const uint8_t dst = line[x];
				line[x] = perform_rasterop(src, dst, plane_mask);
			}
		}
	}
	m_fbc.m_vertex_count = 0;
}

// NOTE: This is basically untested, and probably full of bugs!
void sbus_cgsix_device::handle_blit_command()
{
	auto const vram = util::big_endian_cast<uint8_t>(&m_vram[0]);
	const uint32_t fbw = 1152;//(m_fbc.m_clip_maxx + 1);
	logerror("Copying from %d,%d-%d,%d to %d,%d-%d,%d, width %d, height %d\n"
		, m_fbc.m_x0, m_fbc.m_y0
		, m_fbc.m_x1, m_fbc.m_y1
		, m_fbc.m_x2, m_fbc.m_y2
		, m_fbc.m_x3, m_fbc.m_y3
		, m_fbc.m_clip_maxx, m_fbc.m_clip_maxy);
	uint32_t srcy = m_fbc.m_y0;
	uint32_t dsty = m_fbc.m_y2;
	for (; srcy < m_fbc.m_y1; srcy++, dsty++)
	{
		auto srcline = vram + (srcy * fbw);
		auto dstline = vram + (dsty * fbw);
		uint32_t srcx = m_fbc.m_x0;
		uint32_t dstx = m_fbc.m_x2;
		for (; srcx < m_fbc.m_x1; srcx++, dstx++)
		{
			const uint8_t src = srcline[srcx];
			const uint8_t dst = dstline[dstx];
			const uint8_t result = perform_rasterop(src, dst);
			//logerror("vram[%d] = %02x\n", result);
			dstline[dstx] = result;
		}
	}
}

uint32_t sbus_cgsix_device::fbc_r(offs_t offset, uint32_t mem_mask)
{
	uint32_t ret = 0;
	switch (offset)
	{
		case FBC_CONFIG:
		{
			const uint32_t data = (m_fbc.m_config & FBC_CONFIG_MASK) | FBC_CONFIG_FBID | FBC_CONFIG_VERSION;
			logerror("%s: fbc_r: CONFIG (%08x & %08x)\n", machine().describe_context(), data, mem_mask);
			return data;
		}
		case FBC_MISC:
			logerror("fbc_r: MISC (%08x & %08x)\n", m_fbc.m_misc, mem_mask);
			return m_fbc.m_misc;
		case FBC_CLIP_CHECK:
			logerror("fbc_r: CLIP_CHECK (%08x & %08x)\n", m_fbc.m_clip_check, mem_mask);
			return m_fbc.m_clip_check;

		case FBC_STATUS:
			logerror("fbc_r: STATUS (%08x & %08x)\n", m_fbc.m_status, mem_mask);
			return m_fbc.m_status;
		case FBC_DRAW_STATUS:
			logerror("fbc_r: DRAW_STATUS (%08x & %08x)\n", m_fbc.m_draw_status, mem_mask);
			handle_draw_command();
			return m_fbc.m_draw_status;
		case FBC_BLIT_STATUS:
			logerror("fbc_r: BLIT_STATUS (%08x & %08x)\n", m_fbc.m_blit_status, mem_mask);
			handle_blit_command();
			return m_fbc.m_blit_status;
		case FBC_FONT:
			logerror("fbc_r: FONT (%08x & %08x)\n", m_fbc.m_font, mem_mask);
			return m_fbc.m_font;

		case FBC_X0:
			logerror("fbc_r: X0 (%08x & %08x)\n", m_fbc.m_x0, mem_mask);
			return m_fbc.m_x0;
		case FBC_Y0:
			logerror("fbc_r: Y0 (%08x & %08x)\n", m_fbc.m_y0, mem_mask);
			return m_fbc.m_y0;
		case FBC_Z0:
			logerror("fbc_r: Z0 (%08x & %08x)\n", m_fbc.m_z0, mem_mask);
			return m_fbc.m_z0;
		case FBC_COLOR0:
			logerror("fbc_r: COLOR0 (%08x & %08x)\n", m_fbc.m_color0, mem_mask);
			return m_fbc.m_color0;
		case FBC_X1:
			logerror("fbc_r: X1 (%08x & %08x)\n", m_fbc.m_x1, mem_mask);
			return m_fbc.m_x1;
		case FBC_Y1:
			logerror("fbc_r: Y1 (%08x & %08x)\n", m_fbc.m_y1, mem_mask);
			return m_fbc.m_y1;
		case FBC_Z1:
			logerror("fbc_r: Z1 (%08x & %08x)\n", m_fbc.m_z1, mem_mask);
			return m_fbc.m_z1;
		case FBC_COLOR1:
			logerror("fbc_r: COLOR1 (%08x & %08x)\n", m_fbc.m_color1, mem_mask);
			return m_fbc.m_color1;
		case FBC_X2:
			logerror("fbc_r: X2 (%08x & %08x)\n", m_fbc.m_x2, mem_mask);
			return m_fbc.m_x2;
		case FBC_Y2:
			logerror("fbc_r: Y2 (%08x & %08x)\n", m_fbc.m_y2, mem_mask);
			return m_fbc.m_y2;
		case FBC_Z2:
			logerror("fbc_r: Z2 (%08x & %08x)\n", m_fbc.m_z2, mem_mask);
			return m_fbc.m_z2;
		case FBC_COLOR2:
			logerror("fbc_r: COLOR2 (%08x & %08x)\n", m_fbc.m_color2, mem_mask);
			return m_fbc.m_color2;
		case FBC_X3:
			logerror("fbc_r: X3 (%08x & %08x)\n", m_fbc.m_x3, mem_mask);
			return m_fbc.m_x3;
		case FBC_Y3:
			logerror("fbc_r: Y3 (%08x & %08x)\n", m_fbc.m_y3, mem_mask);
			return m_fbc.m_y3;
		case FBC_Z3:
			logerror("fbc_r: Z3 (%08x & %08x)\n", m_fbc.m_z3, mem_mask);
			return m_fbc.m_z3;
		case FBC_COLOR3:
			logerror("fbc_r: COLOR3 (%08x & %08x)\n", m_fbc.m_color3, mem_mask);
			return m_fbc.m_color3;

		case FBC_RASTER_OFFX:
			logerror("fbc_r: RASTER_OFFX (%08x & %08x)\n", m_fbc.m_raster_offx, mem_mask);
			return m_fbc.m_raster_offx;
		case FBC_RASTER_OFFY:
			logerror("fbc_r: RASTER_OFFY (%08x & %08x)\n", m_fbc.m_raster_offy, mem_mask);
			return m_fbc.m_raster_offy;
		case FBC_AUTOINCX:
			logerror("fbc_r: AUTOINCX (%08x & %08x)\n", m_fbc.m_autoincx, mem_mask);
			return m_fbc.m_autoincx;
		case FBC_AUTOINCY:
			logerror("fbc_r: AUTOINCY (%08x & %08x)\n", m_fbc.m_autoincy, mem_mask);
			return m_fbc.m_autoincy;
		case FBC_CLIP_MINX:
			logerror("fbc_r: CLIP_MINX (%08x & %08x)\n", m_fbc.m_clip_minx, mem_mask);
			return m_fbc.m_clip_minx;
		case FBC_CLIP_MINY:
			logerror("fbc_r: CLIP_MINY (%08x & %08x)\n", m_fbc.m_clip_miny, mem_mask);
			return m_fbc.m_clip_miny;
		case FBC_CLIP_MAXX:
			logerror("fbc_r: CLIP_MAXX (%08x & %08x)\n", m_fbc.m_clip_maxx, mem_mask);
			return m_fbc.m_clip_maxx;
		case FBC_CLIP_MAXY:
			logerror("fbc_r: CLIP_MAXY (%08x & %08x)\n", m_fbc.m_clip_maxy, mem_mask);
			return m_fbc.m_clip_maxy;

		case FBC_FCOLOR:
			logerror("fbc_r: FCOLOR (%08x & %08x)\n", m_fbc.m_fcolor, mem_mask);
			return m_fbc.m_fcolor;
		case FBC_BCOLOR:
			logerror("fbc_r: BCOLOR (%08x & %08x)\n", m_fbc.m_bcolor, mem_mask);
			return m_fbc.m_bcolor;
		case FBC_RASTEROP:
			logerror("fbc_r: RASTEROP (%08x & %08x)\n", m_fbc.m_rasterop, mem_mask);
			return m_fbc.m_rasterop;
		case FBC_PLANE_MASK:
			logerror("fbc_r: PLANE_MASK (%08x & %08x)\n", m_fbc.m_plane_mask, mem_mask);
			return m_fbc.m_plane_mask;
		case FBC_PIXEL_MASK:
			logerror("fbc_r: PIXEL_MASK (%08x & %08x)\n", m_fbc.m_pixel_mask, mem_mask);
			return m_fbc.m_pixel_mask;

		case FBC_PATT_ALIGN:
			logerror("fbc_r: PATT_ALIGN (%08x & %08x)\n", m_fbc.m_patt_align, mem_mask);
			return m_fbc.m_patt_align;
		case FBC_PATTERN0:
			logerror("fbc_r: PATTERN0 (%08x & %08x)\n", m_fbc.m_pattern[0], mem_mask);
			return m_fbc.m_pattern[0];
		case FBC_PATTERN1:
			logerror("fbc_r: PATTERN1 (%08x & %08x)\n", m_fbc.m_pattern[1], mem_mask);
			return m_fbc.m_pattern[1];
		case FBC_PATTERN2:
			logerror("fbc_r: PATTERN2 (%08x & %08x)\n", m_fbc.m_pattern[2], mem_mask);
			return m_fbc.m_pattern[2];
		case FBC_PATTERN3:
			logerror("fbc_r: PATTERN3 (%08x & %08x)\n", m_fbc.m_pattern[3], mem_mask);
			return m_fbc.m_pattern[3];
		case FBC_PATTERN4:
			logerror("fbc_r: PATTERN4 (%08x & %08x)\n", m_fbc.m_pattern[4], mem_mask);
			return m_fbc.m_pattern[4];
		case FBC_PATTERN5:
			logerror("fbc_r: PATTERN5 (%08x & %08x)\n", m_fbc.m_pattern[5], mem_mask);
			return m_fbc.m_pattern[5];
		case FBC_PATTERN6:
			logerror("fbc_r: PATTERN6 (%08x & %08x)\n", m_fbc.m_pattern[6], mem_mask);
			return m_fbc.m_pattern[6];
		case FBC_PATTERN7:
			logerror("fbc_r: PATTERN7 (%08x & %08x)\n", m_fbc.m_pattern[7], mem_mask);
			return m_fbc.m_pattern[7];

		case FBC_IPOINT_ABSX:
			logerror("fbc_r: IPOINT_ABSX (%08x & %08x)\n", m_fbc.m_ipoint_absx, mem_mask);
			return m_fbc.m_ipoint_absx;
		case FBC_IPOINT_ABSY:
			logerror("fbc_r: IPOINT_ABSY (%08x & %08x)\n", m_fbc.m_ipoint_absy, mem_mask);
			return m_fbc.m_ipoint_absy;
		case FBC_IPOINT_ABSZ:
			logerror("fbc_r: IPOINT_ABSZ (%08x & %08x)\n", m_fbc.m_ipoint_absz, mem_mask);
			return m_fbc.m_ipoint_absz;
		case FBC_IPOINT_RELX:
			logerror("fbc_r: IPOINT_RELX (%08x & %08x)\n", m_fbc.m_ipoint_relx, mem_mask);
			return m_fbc.m_ipoint_relx;
		case FBC_IPOINT_RELY:
			logerror("fbc_r: IPOINT_RELY (%08x & %08x)\n", m_fbc.m_ipoint_rely, mem_mask);
			return m_fbc.m_ipoint_rely;
		case FBC_IPOINT_RELZ:
			logerror("fbc_r: IPOINT_RELZ (%08x & %08x)\n", m_fbc.m_ipoint_relz, mem_mask);
			return m_fbc.m_ipoint_relz;
		case FBC_IPOINT_R:
			logerror("fbc_r: IPOINT_R (%08x & %08x)\n", m_fbc.m_ipoint_r, mem_mask);
			return m_fbc.m_ipoint_r;
		case FBC_IPOINT_G:
			logerror("fbc_r: IPOINT_G (%08x & %08x)\n", m_fbc.m_ipoint_g, mem_mask);
			return m_fbc.m_ipoint_g;
		case FBC_IPOINT_B:
			logerror("fbc_r: IPOINT_B (%08x & %08x)\n", m_fbc.m_ipoint_b, mem_mask);
			return m_fbc.m_ipoint_b;
		case FBC_IPOINT_A:
			logerror("fbc_r: IPOINT_A (%08x & %08x)\n", m_fbc.m_ipoint_a, mem_mask);
			return m_fbc.m_ipoint_a;

		case FBC_ILINE_ABSX:
			logerror("fbc_r: ILINE_ABSX (%08x & %08x)\n", m_fbc.m_iline_absx, mem_mask);
			return m_fbc.m_iline_absx;
		case FBC_ILINE_ABSY:
			logerror("fbc_r: ILINE_ABSY (%08x & %08x)\n", m_fbc.m_iline_absy, mem_mask);
			return m_fbc.m_iline_absy;
		case FBC_ILINE_ABSZ:
			logerror("fbc_r: ILINE_ABSZ (%08x & %08x)\n", m_fbc.m_iline_absz, mem_mask);
			return m_fbc.m_iline_absz;
		case FBC_ILINE_RELX:
			logerror("fbc_r: ILINE_RELX (%08x & %08x)\n", m_fbc.m_iline_relx, mem_mask);
			return m_fbc.m_iline_relx;
		case FBC_ILINE_RELY:
			logerror("fbc_r: ILINE_RELY (%08x & %08x)\n", m_fbc.m_iline_rely, mem_mask);
			return m_fbc.m_iline_rely;
		case FBC_ILINE_RELZ:
			logerror("fbc_r: ILINE_RELZ (%08x & %08x)\n", m_fbc.m_iline_relz, mem_mask);
			return m_fbc.m_iline_relz;
		case FBC_ILINE_R:
			logerror("fbc_r: ILINE_R (%08x & %08x)\n", m_fbc.m_iline_r, mem_mask);
			return m_fbc.m_iline_r;
		case FBC_ILINE_G:
			logerror("fbc_r: ILINE_G (%08x & %08x)\n", m_fbc.m_iline_g, mem_mask);
			return m_fbc.m_iline_g;
		case FBC_ILINE_B:
			logerror("fbc_r: ILINE_B (%08x & %08x)\n", m_fbc.m_iline_b, mem_mask);
			return m_fbc.m_iline_b;
		case FBC_ILINE_A:
			logerror("fbc_r: ILINE_A (%08x & %08x)\n", m_fbc.m_iline_a, mem_mask);
			return m_fbc.m_iline_a;

		case FBC_ITRI_ABSX:
			logerror("fbc_r: ITRI_ABSX (%08x & %08x)\n", m_fbc.m_itri_absx, mem_mask);
			return m_fbc.m_itri_absx;
		case FBC_ITRI_ABSY:
			logerror("fbc_r: ITRI_ABSY (%08x & %08x)\n", m_fbc.m_itri_absy, mem_mask);
			return m_fbc.m_itri_absy;
		case FBC_ITRI_ABSZ:
			logerror("fbc_r: ITRI_ABSZ (%08x & %08x)\n", m_fbc.m_itri_absz, mem_mask);
			return m_fbc.m_itri_absz;
		case FBC_ITRI_RELX:
			logerror("fbc_r: ITRI_RELX (%08x & %08x)\n", m_fbc.m_itri_relx, mem_mask);
			return m_fbc.m_itri_relx;
		case FBC_ITRI_RELY:
			logerror("fbc_r: ITRI_RELY (%08x & %08x)\n", m_fbc.m_itri_rely, mem_mask);
			return m_fbc.m_itri_rely;
		case FBC_ITRI_RELZ:
			logerror("fbc_r: ITRI_RELZ (%08x & %08x)\n", m_fbc.m_itri_relz, mem_mask);
			return m_fbc.m_itri_relz;
		case FBC_ITRI_R:
			logerror("fbc_r: ITRI_R (%08x & %08x)\n", m_fbc.m_itri_r, mem_mask);
			return m_fbc.m_itri_r;
		case FBC_ITRI_G:
			logerror("fbc_r: ITRI_G (%08x & %08x)\n", m_fbc.m_itri_g, mem_mask);
			return m_fbc.m_itri_g;
		case FBC_ITRI_B:
			logerror("fbc_r: ITRI_B (%08x & %08x)\n", m_fbc.m_itri_b, mem_mask);
			return m_fbc.m_itri_b;
		case FBC_ITRI_A:
			logerror("fbc_r: ITRI_A (%08x & %08x)\n", m_fbc.m_itri_a, mem_mask);
			return m_fbc.m_itri_a;

		case FBC_IQUAD_ABSX:
			logerror("fbc_r: IQUAD_ABSX (%08x & %08x)\n", m_fbc.m_iquad_absx, mem_mask);
			return m_fbc.m_iquad_absx;
		case FBC_IQUAD_ABSY:
			logerror("fbc_r: IQUAD_ABSY (%08x & %08x)\n", m_fbc.m_iquad_absy, mem_mask);
			return m_fbc.m_iquad_absy;
		case FBC_IQUAD_ABSZ:
			logerror("fbc_r: IQUAD_ABSZ (%08x & %08x)\n", m_fbc.m_iquad_absz, mem_mask);
			return m_fbc.m_iquad_absz;
		case FBC_IQUAD_RELX:
			logerror("fbc_r: IQUAD_RELX (%08x & %08x)\n", m_fbc.m_iquad_relx, mem_mask);
			return m_fbc.m_iquad_relx;
		case FBC_IQUAD_RELY:
			logerror("fbc_r: IQUAD_RELY (%08x & %08x)\n", m_fbc.m_iquad_rely, mem_mask);
			return m_fbc.m_iquad_rely;
		case FBC_IQUAD_RELZ:
			logerror("fbc_r: IQUAD_RELZ (%08x & %08x)\n", m_fbc.m_iquad_relz, mem_mask);
			return m_fbc.m_iquad_relz;
		case FBC_IQUAD_R:
			logerror("fbc_r: IQUAD_R (%08x & %08x)\n", m_fbc.m_iquad_r, mem_mask);
			return m_fbc.m_iquad_r;
		case FBC_IQUAD_G:
			logerror("fbc_r: IQUAD_G (%08x & %08x)\n", m_fbc.m_iquad_g, mem_mask);
			return m_fbc.m_iquad_g;
		case FBC_IQUAD_B:
			logerror("fbc_r: IQUAD_B (%08x & %08x)\n", m_fbc.m_iquad_b, mem_mask);
			return m_fbc.m_iquad_b;
		case FBC_IQUAD_A:
			logerror("fbc_r: IQUAD_A (%08x & %08x)\n", m_fbc.m_iquad_a, mem_mask);
			return m_fbc.m_iquad_a;

		case FBC_IRECT_ABSX:
			logerror("fbc_r: IRECT_ABSX (%08x & %08x)\n", m_fbc.m_irect_absx, mem_mask);
			return m_fbc.m_irect_absx;
		case FBC_IRECT_ABSY:
			logerror("fbc_r: IRECT_ABSY (%08x & %08x)\n", m_fbc.m_irect_absy, mem_mask);
			return m_fbc.m_irect_absy;
		case FBC_IRECT_ABSZ:
			logerror("fbc_r: IRECT_ABSZ (%08x & %08x)\n", m_fbc.m_irect_absz, mem_mask);
			return m_fbc.m_irect_absz;
		case FBC_IRECT_RELX:
			logerror("fbc_r: IRECT_RELX (%08x & %08x)\n", m_fbc.m_irect_relx, mem_mask);
			return m_fbc.m_irect_relx;
		case FBC_IRECT_RELY:
			logerror("fbc_r: IRECT_RELY (%08x & %08x)\n", m_fbc.m_irect_rely, mem_mask);
			return m_fbc.m_irect_rely;
		case FBC_IRECT_RELZ:
			logerror("fbc_r: IRECT_RELZ (%08x & %08x)\n", m_fbc.m_irect_relz, mem_mask);
			return m_fbc.m_irect_relz;
		case FBC_IRECT_R:
			logerror("fbc_r: IRECT_R (%08x & %08x)\n", m_fbc.m_irect_r, mem_mask);
			return m_fbc.m_irect_r;
		case FBC_IRECT_G:
			logerror("fbc_r: IRECT_G (%08x & %08x)\n", m_fbc.m_irect_g, mem_mask);
			return m_fbc.m_irect_g;
		case FBC_IRECT_B:
			logerror("fbc_r: IRECT_B (%08x & %08x)\n", m_fbc.m_irect_b, mem_mask);
			return m_fbc.m_irect_b;
		case FBC_IRECT_A:
			logerror("fbc_r: IRECT_A (%08x & %08x)\n", m_fbc.m_irect_a, mem_mask);
			return m_fbc.m_irect_a;

		default:
			logerror("fbc_r: Unknown register %08x & %08x\n", 0x00700000 | (offset << 2), mem_mask);
			break;
	}
	return ret;
}

void sbus_cgsix_device::fbc_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	static char const *const misc_bdisp_name[4] = { "IGNORE", "0", "1", "ILLEGAL" };
	static char const *const misc_bread_name[4] = { "IGNORE", "0", "1", "ILLEGAL" };
	static char const *const misc_bwrite1_name[4] = { "IGNORE", "ENABLE", "DISABLE", "ILLEGAL" };
	static char const *const misc_bwrite0_name[4] = { "IGNORE", "ENABLE", "DISABLE", "ILLEGAL" };
	static char const *const misc_draw_name[4] = { "IGNORE", "RENDER", "PICK", "ILLEGAL" };
	static char const *const misc_data_name[4] = { "IGNORE", "COLOR8", "COLOR1", "HRMONO" };
	static char const *const misc_blit_name[4] = { "IGNORE", "NOSRC", "SRC", "ILLEGAL" };
	static char const *const rasterop_rop_name[16] =
	{
		"CLR", "SRC_NOR_DST", "NSRC_AND_DST", "NOT_SRC", "SRC_AND_NDST", "NOT_DST", "SRC_XOR_DST", "SRC_NAND_DST",
		"SRC_AND_DST", "SRC_XNOR_DST", "DST", "NSRC_OR_DST", "SRC", "SRC_OR_NDST", "SRC_OR_DST", "SET"
	};
	static char const *const rasterop_plot_name[2] = { "PLOT", "UNPLOT" };
	static char const *const rasterop_rast_name[2] = { "BOOL", "LINEAR" };
	static char const *const rasterop_attr_name[4] = { "IGNORE", "UNSUPP", "SUPP", "ILLEGAL" };
	static char const *const rasterop_polyg_name[4] = { "IGNORE", "OVERLAP", "NONOVERLAP", "ILLEGAL" };
	static char const *const rasterop_pattern_name[4] = { "IGNORE", "ZEROES", "ONES", "MASK" };
	static char const *const rasterop_pixel_name[4] = { "IGNORE", "ZEROES", "ONES", "MASK" };
	static char const *const rasterop_plane_name[4] = { "IGNORE", "ZEROES", "ONES", "MASK" };

	switch (offset)
	{
		case FBC_CONFIG:
			COMBINE_DATA(&m_fbc.m_config);
			logerror("fbc_w: CONFIG = %08x & %08x)\n", data, mem_mask);
			break;
		case FBC_MISC:
			COMBINE_DATA(&m_fbc.m_misc);
			logerror("fbc_w: MISC = %08x & %08x\n", data, mem_mask);
			logerror("       MISC_INDEX     = %d\n", fbc_misc_index());
			logerror("       MISC_INDEX_MOD = %d\n", fbc_misc_index_mod());
			logerror("       MISC_BDISP     = %d (%s)\n", fbc_misc_bdisp(), misc_bdisp_name[fbc_misc_bdisp()]);
			logerror("       MISC_BREAD     = %d (%s)\n", fbc_misc_bread(), misc_bread_name[fbc_misc_bread()]);
			logerror("       MISC_BWRITE1   = %d (%s)\n", fbc_misc_bwrite1(), misc_bwrite1_name[fbc_misc_bwrite1()]);
			logerror("       MISC_BWRITE0   = %d (%s)\n", fbc_misc_bwrite0(), misc_bwrite0_name[fbc_misc_bwrite0()]);
			logerror("       MISC_DRAW      = %d (%s)\n", fbc_misc_draw(), misc_draw_name[fbc_misc_draw()]);
			logerror("       MISC_DATA      = %d (%s)\n", fbc_misc_data(), misc_data_name[fbc_misc_data()]);
			logerror("       MISC_BLIT      = %d (%s)\n", fbc_misc_blit(), misc_blit_name[fbc_misc_blit()]);
			break;
		case FBC_CLIP_CHECK:
			logerror("fbc_w: CLIP_CHECK = %08x & %08x\n", data, mem_mask);
			COMBINE_DATA(&m_fbc.m_clip_check);
			break;

		case FBC_STATUS:
			logerror("fbc_w: STATUS = %08x & %08x\n", data, mem_mask);
			//COMBINE_DATA(&m_fbc.m_status);
			break;
		case FBC_DRAW_STATUS:
			logerror("fbc_w: DRAW_STATUS = %08x & %08x\n", data, mem_mask);
			//COMBINE_DATA(&m_fbc.m_draw_status);
			break;
		case FBC_BLIT_STATUS:
			logerror("fbc_w: BLIT_STATUS = %08x & %08x\n", data, mem_mask);
			//COMBINE_DATA(&m_fbc.m_blit_status);
			break;
		case FBC_FONT:
		{
			logerror("fbc_w: FONT = %08x & %08x\n", data, mem_mask);
			COMBINE_DATA(&m_fbc.m_font);
			handle_font_poke();
			break;
		}

		case FBC_X0:
			logerror("fbc_w: X0 = %08x & %08x\n", data, mem_mask);
			COMBINE_DATA(&m_fbc.m_x0);
			break;
		case FBC_Y0:
			logerror("fbc_w: Y0 = %08x & %08x\n", data, mem_mask);
			COMBINE_DATA(&m_fbc.m_y0);
			break;
		case FBC_Z0:
			logerror("fbc_w: Z0 = %08x & %08x\n", data, mem_mask);
			COMBINE_DATA(&m_fbc.m_z0);
			break;
		case FBC_COLOR0:
			logerror("fbc_w: COLOR0 = %08x & %08x\n", data, mem_mask);
			COMBINE_DATA(&m_fbc.m_color0);
			break;
		case FBC_X1:
			logerror("fbc_w: X1 = %08x & %08x\n", data, mem_mask);
			COMBINE_DATA(&m_fbc.m_x1);
			break;
		case FBC_Y1:
			logerror("fbc_w: Y1 = %08x & %08x\n", data, mem_mask);
			COMBINE_DATA(&m_fbc.m_y1);
			break;
		case FBC_Z1:
			logerror("fbc_w: Z1 = %08x & %08x\n", data, mem_mask);
			COMBINE_DATA(&m_fbc.m_z1);
			break;
		case FBC_COLOR1:
			logerror("fbc_w: COLOR1 = %08x & %08x\n", data, mem_mask);
			COMBINE_DATA(&m_fbc.m_color1);
			break;
		case FBC_X2:
			logerror("fbc_w: X2 = %08x & %08x\n", data, mem_mask);
			COMBINE_DATA(&m_fbc.m_x2);
			break;
		case FBC_Y2:
			logerror("fbc_w: Y2 = %08x & %08x\n", data, mem_mask);
			COMBINE_DATA(&m_fbc.m_y2);
			break;
		case FBC_Z2:
			logerror("fbc_w: Z2 = %08x & %08x\n", data, mem_mask);
			COMBINE_DATA(&m_fbc.m_z2);
			break;
		case FBC_COLOR2:
			logerror("fbc_w: COLOR2 = %08x & %08x\n", data, mem_mask);
			COMBINE_DATA(&m_fbc.m_color2);
			break;
		case FBC_X3:
			logerror("fbc_w: X3 = %08x & %08x\n", data, mem_mask);
			COMBINE_DATA(&m_fbc.m_x3);
			break;
		case FBC_Y3:
			logerror("fbc_w: Y3 = %08x & %08x\n", data, mem_mask);
			COMBINE_DATA(&m_fbc.m_y3);
			break;
		case FBC_Z3:
			logerror("fbc_w: Z3 = %08x & %08x\n", data, mem_mask);
			COMBINE_DATA(&m_fbc.m_z3);
			break;
		case FBC_COLOR3:
			logerror("fbc_w: COLOR3 = %08x & %08x\n", data, mem_mask);
			COMBINE_DATA(&m_fbc.m_color3);
			break;

		case FBC_RASTER_OFFX:
			logerror("fbc_w: RASTER_OFFX = %08x & %08x\n", data, mem_mask);
			COMBINE_DATA(&m_fbc.m_raster_offx);
			break;
		case FBC_RASTER_OFFY:
			logerror("fbc_w: RASTER_OFFY = %08x & %08x\n", data, mem_mask);
			COMBINE_DATA(&m_fbc.m_raster_offy);
			break;
		case FBC_AUTOINCX:
			logerror("fbc_w: AUTOINCX = %08x & %08x\n", data, mem_mask);
			COMBINE_DATA(&m_fbc.m_autoincx);
			break;
		case FBC_AUTOINCY:
			logerror("fbc_w: AUTOINCY = %08x & %08x\n", data, mem_mask);
			COMBINE_DATA(&m_fbc.m_autoincy);
			break;
		case FBC_CLIP_MINX:
			logerror("fbc_w: CLIP_MINX = %08x & %08x\n", data, mem_mask);
			COMBINE_DATA(&m_fbc.m_clip_minx);
			break;
		case FBC_CLIP_MINY:
			logerror("fbc_w: CLIP_MINY = %08x & %08x\n", data, mem_mask);
			COMBINE_DATA(&m_fbc.m_clip_miny);
			break;
		case FBC_CLIP_MAXX:
			logerror("fbc_w: CLIP_MAXX = %08x & %08x\n", data, mem_mask);
			COMBINE_DATA(&m_fbc.m_clip_maxx);
			break;
		case FBC_CLIP_MAXY:
			logerror("fbc_w: CLIP_MAXY = %08x & %08x\n", data, mem_mask);
			COMBINE_DATA(&m_fbc.m_clip_maxy);
			break;

		case FBC_FCOLOR:
			logerror("fbc_w: FCOLOR = %08x & %08x\n", data, mem_mask);
			COMBINE_DATA(&m_fbc.m_fcolor);
			break;
		case FBC_BCOLOR:
			logerror("fbc_w: BCOLOR = %08x & %08x\n", data, mem_mask);
			COMBINE_DATA(&m_fbc.m_bcolor);
			break;
		case FBC_RASTEROP:
			COMBINE_DATA(&m_fbc.m_rasterop);
			logerror("fbc_w: RASTEROP = %08x & %08x\n", data, mem_mask);
			logerror("       RASTEROP_ROP00 = %d (%s)\n", fbc_rasterop_rop00(), rasterop_rop_name[fbc_rasterop_rop00()]);
			logerror("       RASTEROP_ROP01 = %d (%s)\n", fbc_rasterop_rop01(), rasterop_rop_name[fbc_rasterop_rop01()]);
			logerror("       RASTEROP_ROP10 = %d (%s)\n", fbc_rasterop_rop10(), rasterop_rop_name[fbc_rasterop_rop10()]);
			logerror("       RASTEROP_ROP11 = %d (%s)\n", fbc_rasterop_rop11(), rasterop_rop_name[fbc_rasterop_rop11()]);
			logerror("       RASTEROP_PLOT  = %d (%s)\n", fbc_rasterop_plot(), rasterop_plot_name[fbc_rasterop_plot()]);
			logerror("       RASTEROP_RAST  = %d (%s)\n", fbc_rasterop_rast(), rasterop_rast_name[fbc_rasterop_rast()]);
			logerror("       RASTEROP_ATTR  = %d (%s)\n", fbc_rasterop_attr(), rasterop_attr_name[fbc_rasterop_attr()]);
			logerror("       RASTEROP_POLYG = %d (%s)\n", fbc_rasterop_polyg(), rasterop_polyg_name[fbc_rasterop_polyg()]);
			logerror("       RASTEROP_PATT  = %d (%s)\n", fbc_rasterop_pattern(), rasterop_pattern_name[fbc_rasterop_pattern()]);
			logerror("       RASTEROP_PIXEL = %d (%s)\n", fbc_rasterop_pixel(), rasterop_pixel_name[fbc_rasterop_pixel()]);
			logerror("       RASTEROP_PLANE = %d (%s)\n", fbc_rasterop_plane(), rasterop_plane_name[fbc_rasterop_plane()]);
			break;
		case FBC_PLANE_MASK:
			logerror("fbc_w: PLANE_MASK = %08x & %08x\n", data, mem_mask);
			COMBINE_DATA(&m_fbc.m_plane_mask);
			break;
		case FBC_PIXEL_MASK:
			logerror("fbc_w: PIXEL_MASK = %08x & %08x\n", data, mem_mask);
			COMBINE_DATA(&m_fbc.m_pixel_mask);
			break;

		case FBC_PATT_ALIGN:
			logerror("fbc_w: PATT_ALIGN = %08x & %08x\n", data, mem_mask);
			COMBINE_DATA(&m_fbc.m_patt_align);
			m_fbc.m_patt_align_x = (m_fbc.m_patt_align >> 16) & 0x000f;
			m_fbc.m_patt_align_y = m_fbc.m_patt_align & 0x000f;
			break;
		case FBC_PATTERN0:
			logerror("fbc_w: PATTERN0 = %08x & %08x\n", data, mem_mask);
			COMBINE_DATA(&m_fbc.m_pattern[0]);
			m_fbc.m_patterns[0] = m_fbc.m_pattern[0] >> 16;
			m_fbc.m_patterns[1] = (uint16_t)m_fbc.m_pattern[0];
			break;
		case FBC_PATTERN1:
			logerror("fbc_w: PATTERN1 = %08x & %08x\n", data, mem_mask);
			COMBINE_DATA(&m_fbc.m_pattern[1]);
			m_fbc.m_patterns[2] = m_fbc.m_pattern[1] >> 16;
			m_fbc.m_patterns[3] = (uint16_t)m_fbc.m_pattern[1];
			break;
		case FBC_PATTERN2:
			logerror("fbc_w: PATTERN2 = %08x & %08x\n", data, mem_mask);
			COMBINE_DATA(&m_fbc.m_pattern[2]);
			m_fbc.m_patterns[4] = m_fbc.m_pattern[2] >> 16;
			m_fbc.m_patterns[5] = (uint16_t)m_fbc.m_pattern[2];
			break;
		case FBC_PATTERN3:
			logerror("fbc_w: PATTERN3 = %08x & %08x\n", data, mem_mask);
			COMBINE_DATA(&m_fbc.m_pattern[3]);
			m_fbc.m_patterns[6] = m_fbc.m_pattern[3] >> 16;
			m_fbc.m_patterns[7] = (uint16_t)m_fbc.m_pattern[3];
			break;
		case FBC_PATTERN4:
			logerror("fbc_w: PATTERN4 = %08x & %08x\n", data, mem_mask);
			COMBINE_DATA(&m_fbc.m_pattern[4]);
			m_fbc.m_patterns[8] = m_fbc.m_pattern[4] >> 16;
			m_fbc.m_patterns[9] = (uint16_t)m_fbc.m_pattern[4];
			break;
		case FBC_PATTERN5:
			logerror("fbc_w: PATTERN5 = %08x & %08x\n", data, mem_mask);
			COMBINE_DATA(&m_fbc.m_pattern[5]);
			m_fbc.m_patterns[10] = m_fbc.m_pattern[5] >> 16;
			m_fbc.m_patterns[11] = (uint16_t)m_fbc.m_pattern[5];
			break;
		case FBC_PATTERN6:
			logerror("fbc_w: PATTERN6 = %08x & %08x\n", data, mem_mask);
			COMBINE_DATA(&m_fbc.m_pattern[6]);
			m_fbc.m_patterns[12] = m_fbc.m_pattern[6] >> 16;
			m_fbc.m_patterns[13] = (uint16_t)m_fbc.m_pattern[6];
			break;
		case FBC_PATTERN7:
			logerror("fbc_w: PATTERN7 = %08x & %08x\n", data, mem_mask);
			COMBINE_DATA(&m_fbc.m_pattern[7]);
			m_fbc.m_patterns[14] = m_fbc.m_pattern[7] >> 16;
			m_fbc.m_patterns[15] = (uint16_t)m_fbc.m_pattern[7];
			break;

		case FBC_IPOINT_ABSX:
			logerror("fbc_w: IPOINT_ABSX = %08x & %08x\n", data, mem_mask);
			COMBINE_DATA(&m_fbc.m_ipoint_absx);
			break;
		case FBC_IPOINT_ABSY:
			logerror("fbc_w: IPOINT_ABSY = %08x & %08x\n", data, mem_mask);
			COMBINE_DATA(&m_fbc.m_ipoint_absy);
			break;
		case FBC_IPOINT_ABSZ:
			logerror("fbc_w: IPOINT_ABSZ = %08x & %08x\n", data, mem_mask);
			COMBINE_DATA(&m_fbc.m_ipoint_absz);
			break;
		case FBC_IPOINT_RELX:
			logerror("fbc_w: IPOINT_RELX = %08x & %08x\n", data, mem_mask);
			COMBINE_DATA(&m_fbc.m_ipoint_relx);
			break;
		case FBC_IPOINT_RELY:
			logerror("fbc_w: IPOINT_RELY = %08x & %08x\n", data, mem_mask);
			COMBINE_DATA(&m_fbc.m_ipoint_rely);
			break;
		case FBC_IPOINT_RELZ:
			logerror("fbc_w: IPOINT_RELZ = %08x & %08x\n", data, mem_mask);
			COMBINE_DATA(&m_fbc.m_ipoint_relz);
			break;
		case FBC_IPOINT_R:
			logerror("fbc_w: IPOINT_R = %08x & %08x\n", data, mem_mask);
			COMBINE_DATA(&m_fbc.m_ipoint_r);
			break;
		case FBC_IPOINT_G:
			logerror("fbc_w: IPOINT_G = %08x & %08x\n", data, mem_mask);
			COMBINE_DATA(&m_fbc.m_ipoint_g);
			break;
		case FBC_IPOINT_B:
			logerror("fbc_w: IPOINT_B = %08x & %08x\n", data, mem_mask);
			COMBINE_DATA(&m_fbc.m_ipoint_b);
			break;
		case FBC_IPOINT_A:
			logerror("fbc_w: IPOINT_A = %08x & %08x\n", data, mem_mask);
			COMBINE_DATA(&m_fbc.m_ipoint_a);
			break;

		case FBC_ILINE_ABSX:
			logerror("fbc_w: ILINE_ABSX = %08x & %08x\n", data, mem_mask);
			COMBINE_DATA(&m_fbc.m_iline_absx);
			break;
		case FBC_ILINE_ABSY:
			logerror("fbc_w: ILINE_ABSY = %08x & %08x\n", data, mem_mask);
			COMBINE_DATA(&m_fbc.m_iline_absy);
			break;
		case FBC_ILINE_ABSZ:
			logerror("fbc_w: ILINE_ABSZ = %08x & %08x\n", data, mem_mask);
			COMBINE_DATA(&m_fbc.m_iline_absz);
			break;
		case FBC_ILINE_RELX:
			logerror("fbc_w: ILINE_RELX = %08x & %08x\n", data, mem_mask);
			COMBINE_DATA(&m_fbc.m_iline_relx);
			break;
		case FBC_ILINE_RELY:
			logerror("fbc_w: ILINE_RELY = %08x & %08x\n", data, mem_mask);
			COMBINE_DATA(&m_fbc.m_iline_rely);
			break;
		case FBC_ILINE_RELZ:
			logerror("fbc_w: ILINE_RELZ = %08x & %08x\n", data, mem_mask);
			COMBINE_DATA(&m_fbc.m_iline_relz);
			break;
		case FBC_ILINE_R:
			logerror("fbc_w: ILINE_R = %08x & %08x\n", data, mem_mask);
			COMBINE_DATA(&m_fbc.m_iline_r);
			break;
		case FBC_ILINE_G:
			logerror("fbc_w: ILINE_G = %08x & %08x\n", data, mem_mask);
			COMBINE_DATA(&m_fbc.m_iline_g);
			break;
		case FBC_ILINE_B:
			logerror("fbc_w: ILINE_B = %08x & %08x\n", data, mem_mask);
			COMBINE_DATA(&m_fbc.m_iline_b);
			break;
		case FBC_ILINE_A:
			logerror("fbc_w: ILINE_A = %08x & %08x\n", data, mem_mask);
			COMBINE_DATA(&m_fbc.m_iline_a);
			break;

		case FBC_ITRI_ABSX:
			logerror("fbc_w: ITRI_ABSX = %08x & %08x\n", data, mem_mask);
			COMBINE_DATA(&m_fbc.m_itri_absx);
			break;
		case FBC_ITRI_ABSY:
			COMBINE_DATA(&m_fbc.m_itri_absy);
			logerror("fbc_w: ITRI_ABSY = %08x & %08x\n", data, mem_mask);
			break;
		case FBC_ITRI_ABSZ:
			logerror("fbc_w: ITRI_ABSZ = %08x & %08x\n", data, mem_mask);
			COMBINE_DATA(&m_fbc.m_itri_absz);
			break;
		case FBC_ITRI_RELX:
			logerror("fbc_w: ITRI_RELX = %08x & %08x\n", data, mem_mask);
			COMBINE_DATA(&m_fbc.m_itri_relx);
			break;
		case FBC_ITRI_RELY:
			logerror("fbc_w: ITRI_RELY = %08x & %08x\n", data, mem_mask);
			COMBINE_DATA(&m_fbc.m_itri_rely);
			break;
		case FBC_ITRI_RELZ:
			logerror("fbc_w: ITRI_RELZ = %08x & %08x\n", data, mem_mask);
			COMBINE_DATA(&m_fbc.m_itri_relz);
			break;
		case FBC_ITRI_R:
			logerror("fbc_w: ITRI_R = %08x & %08x\n", data, mem_mask);
			COMBINE_DATA(&m_fbc.m_itri_r);
			break;
		case FBC_ITRI_G:
			logerror("fbc_w: ITRI_G = %08x & %08x\n", data, mem_mask);
			COMBINE_DATA(&m_fbc.m_itri_g);
			break;
		case FBC_ITRI_B:
			logerror("fbc_w: ITRI_B = %08x & %08x\n", data, mem_mask);
			COMBINE_DATA(&m_fbc.m_itri_b);
			break;
		case FBC_ITRI_A:
			logerror("fbc_w: ITRI_A = %08x & %08x\n", data, mem_mask);
			COMBINE_DATA(&m_fbc.m_itri_a);
			break;

		case FBC_IQUAD_ABSX:
			logerror("fbc_w: IQUAD_ABSX = %08x & %08x\n", data, mem_mask);
			COMBINE_DATA(&m_fbc.m_iquad_absx);
			break;
		case FBC_IQUAD_ABSY:
			logerror("fbc_w: IQUAD_ABSY = %08x & %08x\n", data, mem_mask);
			COMBINE_DATA(&m_fbc.m_iquad_absy);
			break;
		case FBC_IQUAD_ABSZ:
			logerror("fbc_w: IQUAD_ABSZ = %08x & %08x\n", data, mem_mask);
			COMBINE_DATA(&m_fbc.m_iquad_absz);
			break;
		case FBC_IQUAD_RELX:
			logerror("fbc_w: IQUAD_RELX = %08x & %08x\n", data, mem_mask);
			COMBINE_DATA(&m_fbc.m_iquad_relx);
			break;
		case FBC_IQUAD_RELY:
			logerror("fbc_w: IQUAD_RELY = %08x & %08x\n", data, mem_mask);
			COMBINE_DATA(&m_fbc.m_iquad_rely);
			break;
		case FBC_IQUAD_RELZ:
			logerror("fbc_w: IQUAD_RELZ = %08x & %08x\n", data, mem_mask);
			COMBINE_DATA(&m_fbc.m_iquad_relz);
			break;
		case FBC_IQUAD_R:
			logerror("fbc_w: IQUAD_R = %08x & %08x\n", data, mem_mask);
			COMBINE_DATA(&m_fbc.m_iquad_r);
			break;
		case FBC_IQUAD_G:
			logerror("fbc_w: IQUAD_G = %08x & %08x\n", data, mem_mask);
			COMBINE_DATA(&m_fbc.m_iquad_g);
			break;
		case FBC_IQUAD_B:
			logerror("fbc_w: IQUAD_B = %08x & %08x\n", data, mem_mask);
			COMBINE_DATA(&m_fbc.m_iquad_b);
			break;
		case FBC_IQUAD_A:
			logerror("fbc_w: IQUAD_A = %08x & %08x\n", data, mem_mask);
			COMBINE_DATA(&m_fbc.m_iquad_a);
			break;

		case FBC_IRECT_ABSX:
			logerror("fbc_w: IRECT_ABSX = %08x & %08x\n", data, mem_mask);
			m_fbc.m_prim_buf[m_fbc.m_vertex_count].m_absx = data;
			m_fbc.m_vertex_count++;
			m_fbc.m_curr_prim_type = PRIM_RECT;
			break;
		case FBC_IRECT_ABSY:
			logerror("fbc_w: IRECT_ABSY = %08x & %08x\n", data, mem_mask);
			m_fbc.m_prim_buf[m_fbc.m_vertex_count].m_absy = data;
			break;
		case FBC_IRECT_ABSZ:
			logerror("fbc_w: IRECT_ABSZ = %08x & %08x\n", data, mem_mask);
			m_fbc.m_prim_buf[m_fbc.m_vertex_count].m_absz = data;
			break;
		case FBC_IRECT_RELX:
			logerror("fbc_w: IRECT_RELX = %08x & %08x\n", data, mem_mask);
			m_fbc.m_prim_buf[m_fbc.m_vertex_count].m_relx = data;
			break;
		case FBC_IRECT_RELY:
			logerror("fbc_w: IRECT_RELY = %08x & %08x\n", data, mem_mask);
			m_fbc.m_prim_buf[m_fbc.m_vertex_count].m_rely = data;
			break;
		case FBC_IRECT_RELZ:
			logerror("fbc_w: IRECT_RELZ = %08x & %08x\n", data, mem_mask);
			m_fbc.m_prim_buf[m_fbc.m_vertex_count].m_relz = data;
			break;
		case FBC_IRECT_R:
			logerror("fbc_w: IRECT_R = %08x & %08x\n", data, mem_mask);
			m_fbc.m_prim_buf[m_fbc.m_vertex_count].m_r = data;
			break;
		case FBC_IRECT_G:
			logerror("fbc_w: IRECT_G = %08x & %08x\n", data, mem_mask);
			m_fbc.m_prim_buf[m_fbc.m_vertex_count].m_g = data;
			break;
		case FBC_IRECT_B:
			logerror("fbc_w: IRECT_B = %08x & %08x\n", data, mem_mask);
			m_fbc.m_prim_buf[m_fbc.m_vertex_count].m_b = data;
			break;
		case FBC_IRECT_A:
			logerror("fbc_w: IRECT_A = %08x & %08x\n", data, mem_mask);
			m_fbc.m_prim_buf[m_fbc.m_vertex_count].m_a = data;
			break;

		default:
			logerror("fbc_w: Unknown register %08x = %08x & %08x\n", 0x00700000 | (offset << 2), data, mem_mask);
			break;
	}
}

uint32_t sbus_cgsix_device::cursor_address_r()
{
	return (m_cursor_x << 16) | (uint16_t)m_cursor_y;
}

void sbus_cgsix_device::cursor_address_w(uint32_t data)
{
	m_cursor_x = (int16_t)(data >> 16);
	m_cursor_y = (int16_t)data;
}

uint32_t sbus_cgsix_device::cursor_ram_r(offs_t offset)
{
	return m_cursor_ram[offset];
}

void sbus_cgsix_device::cursor_ram_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&m_cursor_ram[offset]);
}

uint32_t sbus_cgsix_device::thc_misc_r(offs_t offset, uint32_t mem_mask)
{
	logerror("thc_misc_r: %08x & %08x\n", m_thc_misc | THC_MISC_REV, mem_mask);
	return m_thc_misc | THC_MISC_REV;
}

void sbus_cgsix_device::thc_misc_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	logerror("thc_misc_w: %08x & %08x\n", data, mem_mask);
	if (BIT(data, THC_MISC_IRQ_BIT) && BIT(m_thc_misc, THC_MISC_IRQ_BIT))
	{
		data &= ~(1 << THC_MISC_IRQ_BIT);
		lower_irq(4);
	}
	if (BIT(data, THC_MISC_IRQEN_BIT))
	{
		data &= ~(1 << THC_MISC_IRQ_BIT);
	}

	COMBINE_DATA(&m_thc_misc);
	m_thc_misc &= THC_MISC_WRITE_MASK;
}

void sbus_cgsix_device::vblank_w(int state)
{
	int old_state = BIT(m_thc_misc, THC_MISC_VSYNC_BIT);
	if (old_state != state)
	{
		if (state)
		{
			m_thc_misc |= 1 << THC_MISC_VSYNC_BIT;
			if (BIT(m_thc_misc, THC_MISC_IRQEN_BIT))
			{
				m_thc_misc |= 1 << THC_MISC_IRQ_BIT;
				raise_irq(4);
			}
		}
		else
		{
			m_thc_misc &= ~(1 << THC_MISC_VSYNC_BIT);
		}
	}
}

//-------------------------------------------------
//  TurboGX implementation
//-------------------------------------------------

void sbus_turbogx_device::mem_map(address_map &map)
{
	base_map(map);
	map(0x00800000, 0x008fffff).rw(FUNC(sbus_turbogx_device::vram_r), FUNC(sbus_turbogx_device::vram_w));
}

ROM_START( sbus_turbogx )
	ROM_REGION32_BE(0x8000, "prom", ROMREGION_ERASEFF)
	ROM_LOAD( "sunw,501-2325.bin", 0x0000, 0x8000, CRC(bbdc45f8) SHA1(e4a51d78e199cd57f2fcb9d45b25dfae2bd537e4))
ROM_END

const tiny_rom_entry *sbus_turbogx_device::device_rom_region() const
{
	return ROM_NAME( sbus_turbogx );
}

void sbus_turbogx_device::device_add_mconfig(machine_config &config)
{
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_screen_update(FUNC(sbus_turbogx_device::screen_update));
	m_screen->set_raw(105.561_MHz_XTAL, 1472, 0, 1152, 943, 0, 900);
	m_screen->screen_vblank().set(FUNC(sbus_turbogx_device::vblank_w));

	BT458(config, m_ramdac, 0);
}

sbus_turbogx_device::sbus_turbogx_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: sbus_cgsix_device(mconfig, SBUS_TURBOGX, tag, owner, clock, 0x100000)
{
}

void sbus_turbogx_device::install_device()
{
	m_sbus->install_device(m_base, m_base + 0x1ffffff, *this, &sbus_turbogx_device::mem_map);
}

//-------------------------------------------------
//  TurboGX+ implementation
//-------------------------------------------------

void sbus_turbogxp_device::mem_map(address_map &map)
{
	base_map(map);
	map(0x00800000, 0x00bfffff).rw(FUNC(sbus_turbogxp_device::vram_r), FUNC(sbus_turbogxp_device::vram_w));
}

ROM_START( sbus_turbogxp )
	ROM_REGION32_BE(0x8000, "prom", ROMREGION_ERASEFF)
	ROM_LOAD( "sunw,501-2253.bin", 0x0000, 0x8000, CRC(525a58db) SHA1(721fc378d4b952b5cbb271e16bd67bc02439efdc))
ROM_END

const tiny_rom_entry *sbus_turbogxp_device::device_rom_region() const
{
	return ROM_NAME( sbus_turbogxp );
}

void sbus_turbogxp_device::device_add_mconfig(machine_config &config)
{
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_screen_update(FUNC(sbus_turbogxp_device::screen_update));
	m_screen->set_size(1152, 900);
	m_screen->set_visarea(0, 1152-1, 0, 900-1);
	m_screen->set_refresh_hz(72);
	m_screen->screen_vblank().set(FUNC(sbus_turbogxp_device::vblank_w));

	BT467(config, m_ramdac, 0);
}

sbus_turbogxp_device::sbus_turbogxp_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: sbus_cgsix_device(mconfig, SBUS_TURBOGXP, tag, owner, clock, 0x400000)
{
}

void sbus_turbogxp_device::install_device()
{
	m_sbus->install_device(m_base, m_base + 0x1ffffff, *this, &sbus_turbogxp_device::mem_map);
}

