// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/***************************************************************************

    Sun TurboGX accelerated 8-bit color video controller

***************************************************************************/

#include "emu.h"
#include "turbogx.h"
#include "screen.h"

DEFINE_DEVICE_TYPE(SBUS_TURBOGX, sbus_turbogx_device, "turbogx", "Sun TurboGX SBus Video")

void sbus_turbogx_device::mem_map(address_map &map)
{
	map(0x00000000, 0x01ffffff).rw(FUNC(sbus_turbogx_device::unknown_r), FUNC(sbus_turbogx_device::unknown_w));
	map(0x00000000, 0x00007fff).r(FUNC(sbus_turbogx_device::rom_r));
	map(0x00200000, 0x00200007).w(FUNC(sbus_turbogx_device::palette_w));
	map(0x00700000, 0x00700fff).rw(FUNC(sbus_turbogx_device::fbc_r), FUNC(sbus_turbogx_device::fbc_w));
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
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_screen_update(FUNC(sbus_turbogx_device::screen_update));
	screen.set_size(1152, 900);
	screen.set_visarea(0, 1152-1, 0, 900-1);
	screen.set_refresh_hz(72);

	PALETTE(config, m_palette, 256).set_init(DEVICE_SELF, FUNC(sbus_turbogx_device::palette_init));
}


sbus_turbogx_device::sbus_turbogx_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, SBUS_TURBOGX, tag, owner, clock)
	, device_sbus_card_interface(mconfig, *this)
	, m_rom(*this, "prom")
	, m_screen(*this, "screen")
	, m_palette(*this, "palette")
{
}

void sbus_turbogx_device::device_start()
{
	m_vram = std::make_unique<uint32_t[]>(0x100000/4);

	save_item(NAME(m_palette_entry));
	save_item(NAME(m_palette_r));
	save_item(NAME(m_palette_g));
	save_item(NAME(m_palette_b));
	save_item(NAME(m_palette_step));
}

void sbus_turbogx_device::device_reset()
{
	m_palette_entry = 0;
	m_palette_r = 0;
	m_palette_g = 0;
	m_palette_b = 0;
	m_palette_step = 0;

	memset(&m_fbc, 0, sizeof(m_fbc));
}

void sbus_turbogx_device::install_device()
{
	m_sbus->install_device(m_base, m_base + 0x1ffffff, *this, &sbus_turbogx_device::mem_map);
}

void sbus_turbogx_device::palette_init(palette_device &palette)
{
	for (int i = 0; i < 256; i++)
	{
		const uint8_t reversed = 255 - i;
		palette.set_pen_color(i, rgb_t(reversed, reversed, reversed));
	}
}

uint32_t sbus_turbogx_device::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	const pen_t *pens = m_palette->pens();
	uint8_t *vram = (uint8_t *)&m_vram[0];

	for (int y = 0; y < 900; y++)
	{
		uint32_t *scanline = &bitmap.pix32(y);
		for (int x = 0; x < 1152; x++)
		{
			const uint8_t pixel = vram[y * 1152 + BYTE4_XOR_BE(x)];
			*scanline++ = pens[pixel];
		}
	}

	return 0;
}

READ32_MEMBER(sbus_turbogx_device::rom_r)
{
	return ((uint32_t*)m_rom->base())[offset];
}

READ32_MEMBER(sbus_turbogx_device::unknown_r)
{
	logerror("%s: unknown_r: %08x & %08x\n", machine().describe_context(), offset << 2, mem_mask);
	return 0;
}

WRITE32_MEMBER(sbus_turbogx_device::unknown_w)
{
	logerror("%s: unknown_w: %08x = %08x & %08x\n", machine().describe_context(), offset << 2, data, mem_mask);
}

READ32_MEMBER(sbus_turbogx_device::vram_r)
{
	return m_vram[offset];
}

WRITE32_MEMBER(sbus_turbogx_device::vram_w)
{
	COMBINE_DATA(&m_vram[offset]);
}

WRITE32_MEMBER(sbus_turbogx_device::palette_w)
{
	if (offset == 0)
	{
		m_palette_entry = data >> 24;
		logerror("selecting palette entry %d\n", (uint32_t)m_palette_entry);
		m_palette_step = 0;
	}
	else if (offset == 1)
	{
		switch (m_palette_step)
		{
			case 0:
				logerror("palette entry %d red: %02x\n", (uint32_t)m_palette_entry, data);
				m_palette_r = data >> 24;
				m_palette_step++;
				break;
			case 1:
				logerror("palette entry %d green: %02x\n", (uint32_t)m_palette_entry, data);
				m_palette_g = data >> 24;
				m_palette_step++;
				break;
			case 2:
				logerror("palette entry %d blue: %02x\n", (uint32_t)m_palette_entry, data);
				m_palette_b = data >> 24;
				m_palette->set_pen_color(m_palette_entry, rgb_t(m_palette_r, m_palette_g, m_palette_b));
				m_palette_step = 0;
				m_palette_entry++;
				break;
		}
	}
}

uint8_t sbus_turbogx_device::perform_rasterop(uint8_t src, uint8_t dst)
{
	const uint32_t rops[4] = { fbc_rasterop_rop00(), fbc_rasterop_rop01(), fbc_rasterop_rop10(), fbc_rasterop_rop11() };

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
		//logerror("f:%d b:%d s:%02x d:%02x rop:%d\n", f >> 1, b, s, d, rop);

		uint8_t value = 0;
		switch (rop)
		{
			case ROP_CLR:								break;
			case ROP_SRC_NOR_DST:	value = ~(s | d);	break;
			case ROP_NSRC_AND_DST:	value = ~s & d;		break;
			case ROP_NOT_SRC:		value = ~s;			break;
			case ROP_SRC_AND_NDST:	value = s & ~d;		break;
			case ROP_NOT_DST:		value = ~d;			break;
			case ROP_SRC_XOR_DST:	value = s ^ d;		break;
			case ROP_SRC_NAND_DST:	value = ~(s & d);	break;
			case ROP_SRC_AND_DST:	value = s & d;		break;
			case ROP_SRC_XNOR_DST:	value = ~(s ^ d);	break;
			case ROP_DST:			value = d;			break;
			case ROP_NSRC_OR_DST:	value = ~s | d;		break;
			case ROP_SRC:			value = s;			break;
			case ROP_SRC_OR_NDST:	value = s | ~d;		break;
			case ROP_SRC_OR_DST:	value = s | d;		break;
			case ROP_SET:			value = 0xff;		break;
			default:				value = 0;			break;
		}
		result |= value & mask;
	}
	return result;
}

// NOTE: This is basically untested, and probably full of bugs!
void sbus_turbogx_device::handle_draw_command()
{
	if (fbc_misc_draw() != FBC_MISC_DRAW_RENDER)
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

	uint8_t *vram = (uint8_t*)&m_vram[0];

	uint32_t vindex = 0;
	while (vindex < m_fbc.m_vertex_count)
	{
		vertex_t &v0 = m_fbc.m_prim_buf[vindex++];
		vertex_t &v1 = m_fbc.m_prim_buf[vindex++];

		for (uint32_t y = v0.m_absy; y < v1.m_absy; y++)
		{
			const uint32_t line = y * (m_fbc.m_clip_maxx + 1);
			for (uint32_t x = v0.m_absx; x < v1.m_absx; x++)
			{
				const uint8_t src = vram[line + x];
				const uint8_t dst = src;
				const uint8_t result = perform_rasterop(src, dst);
				vram[line + x] = result;
			}
		}
	}
	m_fbc.m_vertex_count = 0;
}

// NOTE: This is basically untested, and probably full of bugs!
void sbus_turbogx_device::handle_blit_command()
{
	uint8_t *vram = (uint8_t*)&m_vram[0];
	const uint32_t fbw = (m_fbc.m_clip_maxx + 1);
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
		uint32_t srcy_index = srcy * fbw;
		uint32_t dsty_index = dsty * fbw;
		uint32_t srcx = m_fbc.m_x0;
		uint32_t dstx = m_fbc.m_x2;
		for (; srcx < m_fbc.m_x1; srcx++, dstx++)
		{
			const uint8_t src = vram[srcy_index + srcx];
			const uint8_t dst = vram[dsty_index + dstx];
			const uint8_t result = perform_rasterop(src, dst);
			//logerror("vram[%d] = %02x\n", result);
			vram[dsty_index + dstx] = result;
		}
	}
}

READ32_MEMBER(sbus_turbogx_device::fbc_r)
{
	uint32_t ret = 0;
	switch (offset)
	{
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

WRITE32_MEMBER(sbus_turbogx_device::fbc_w)
{
	static const char* misc_bdisp_name[4] = { "IGNORE", "0", "1", "ILLEGAL" };
	static const char* misc_bread_name[4] = { "IGNORE", "0", "1", "ILLEGAL" };
	static const char* misc_bwrite1_name[4] = { "IGNORE", "ENABLE", "DISABLE", "ILLEGAL" };
	static const char* misc_bwrite0_name[4] = { "IGNORE", "ENABLE", "DISABLE", "ILLEGAL" };
	static const char* misc_draw_name[4] = { "IGNORE", "RENDER", "PICK", "ILLEGAL" };
	static const char* misc_data_name[4] = { "IGNORE", "COLOR8", "COLOR1", "HRMONO" };
	static const char* misc_blit_name[4] = { "IGNORE", "NOSRC", "SRC", "ILLEGAL" };
	static const char* rasterop_rop_name[16] =
	{
		"CLR", "SRC_NOR_DST", "NSRC_AND_DST", "NOT_SRC", "SRC_AND_NDST", "NOT_DST", "SRC_XOR_DST", "SRC_NAND_DST",
		"SRC_AND_DST", "SRC_XNOR_DST", "DST", "NSRC_OR_DST", "SRC", "SRC_OR_NDST", "SRC_OR_DST", "SET"
	};
	static const char* rasterop_plot_name[2] = { "PLOT", "UNPLOT" };
	static const char* rasterop_rast_name[2] = { "BOOL", "LINEAR" };
	static const char* rasterop_attr_name[4] = { "IGNORE", "UNSUPP", "SUPP", "ILLEGAL" };
	static const char* rasterop_polyg_name[4] = { "IGNORE", "OVERLAP", "NONOVERLAP", "ILLEGAL" };
	static const char* rasterop_pattern_name[4] = { "IGNORE", "ZEROES", "ONES", "MASK" };
	static const char* rasterop_pixel_name[4] = { "IGNORE", "ZEROES", "ONES", "MASK" };
	static const char* rasterop_plane_name[4] = { "IGNORE", "ZEROES", "ONES", "MASK" };

	switch (offset)
	{
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
			logerror("fbc_w: FONT = %08x & %08x\n", data, mem_mask);
			COMBINE_DATA(&m_fbc.m_font);
			break;

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
			logerror("fbc_w: CLIP_MAXY (%08x & %08x\n", data, mem_mask);
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
			break;
		case FBC_PATTERN0:
			logerror("fbc_w: PATTERN0 = %08x & %08x\n", data, mem_mask);
			COMBINE_DATA(&m_fbc.m_pattern[0]);
			break;
		case FBC_PATTERN1:
			logerror("fbc_w: PATTERN1 = %08x & %08x\n", data, mem_mask);
			COMBINE_DATA(&m_fbc.m_pattern[1]);
			break;
		case FBC_PATTERN2:
			logerror("fbc_w: PATTERN2 = %08x & %08x\n", data, mem_mask);
			COMBINE_DATA(&m_fbc.m_pattern[2]);
			break;
		case FBC_PATTERN3:
			logerror("fbc_w: PATTERN3 = %08x & %08x\n", data, mem_mask);
			COMBINE_DATA(&m_fbc.m_pattern[3]);
			break;
		case FBC_PATTERN4:
			logerror("fbc_w: PATTERN4 = %08x & %08x\n", data, mem_mask);
			COMBINE_DATA(&m_fbc.m_pattern[4]);
			break;
		case FBC_PATTERN5:
			logerror("fbc_w: PATTERN5 = %08x & %08x\n", data, mem_mask);
			COMBINE_DATA(&m_fbc.m_pattern[5]);
			break;
		case FBC_PATTERN6:
			logerror("fbc_w: PATTERN6 = %08x & %08x\n", data, mem_mask);
			COMBINE_DATA(&m_fbc.m_pattern[6]);
			break;
		case FBC_PATTERN7:
			logerror("fbc_w: PATTERN7 = %08x & %08x\n", data, mem_mask);
			COMBINE_DATA(&m_fbc.m_pattern[7]);
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
