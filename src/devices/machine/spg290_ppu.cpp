// license:BSD-3-Clause
// copyright-holders:Sandro Ronco
/*****************************************************************************

        SPG290 PPU

        TODO:
        - Horizontal and vertical compression
        - Characters flip
        - Various graphics glitches

*****************************************************************************/

#include "emu.h"
#include "spg290_ppu.h"


DEFINE_DEVICE_TYPE(SPG290_PPU, spg290_ppu_device, "spg290_ppu", "SPG290 PPU")


spg290_ppu_device::spg290_ppu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, SPG290_PPU, tag, owner, clock)
	, m_screen(*this, finder_base::DUMMY_TAG)
	, m_sprite_palette_ram(*this, "sprite_palette_ram")
	, m_char_palette_ram(*this, "char_palette_ram")
	, m_hoffset_ram(*this, "hoffset_ram")
	, m_voffset_ram(*this, "voffset_ram")
	, m_sprite_ram(*this, "sprite_ram")
	, m_vblank_irq_cb(*this)
	, m_space_read_cb(*this)
{
}


void spg290_ppu_device::map(address_map &map)
{
	map(0x0000, 0x00ff).rw(FUNC(spg290_ppu_device::regs_r), FUNC(spg290_ppu_device::regs_w));
	map(0x1000, 0x17ff).ram().share(m_char_palette_ram);
	map(0x1800, 0x1fff).ram().share(m_sprite_palette_ram);
	map(0x2000, 0x27ff).ram().share(m_hoffset_ram);
	map(0x3000, 0x37ff).ram().share(m_voffset_ram);
	map(0x4000, 0x4fff).ram().share(m_sprite_ram);
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void spg290_ppu_device::device_start()
{
	m_vblank_irq_cb.resolve_safe();
	m_space_read_cb.resolve_safe(0);

	save_item(NAME(m_control));
	save_item(NAME(m_sprite_control));
	save_item(NAME(m_irq_control));
	save_item(NAME(m_irq_status));
	save_item(NAME(m_sprite_max));
	save_item(NAME(m_sprite_buf_start));
	save_item(NAME(m_blend_mode));
	save_item(NAME(m_frame_buff));
	save_item(NAME(m_transrgb));
	save_item(NAME(m_vcomp_value));
	save_item(NAME(m_vcomp_offset));
	save_item(NAME(m_vcomp_step));
	save_item(NAME(m_irq_timing_v));
	save_item(NAME(m_irq_timing_h));
	save_item(NAME(m_vblank_lines));

	save_item(STRUCT_MEMBER(m_txs, control));
	save_item(STRUCT_MEMBER(m_txs, attribute));
	save_item(STRUCT_MEMBER(m_txs, posx));
	save_item(STRUCT_MEMBER(m_txs, posy));
	save_item(STRUCT_MEMBER(m_txs, nptr));
	save_item(STRUCT_MEMBER(m_txs, blend));
	save_item(STRUCT_MEMBER(m_txs, buf_start));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void spg290_ppu_device::device_reset()
{
	m_control = 0;
	m_sprite_control = 0;
	m_irq_control = 0;
	m_irq_status = 0;
	m_sprite_max = 0;
	m_sprite_buf_start = 0;
	m_blend_mode = 0;
	m_frame_buff[0] = 0;
	m_frame_buff[1] = 0;
	m_frame_buff[2] = 0;
	m_transrgb = 0;
	m_vcomp_value = 0;
	m_vcomp_offset = 0;
	m_vcomp_step = 0;
	m_irq_timing_v = 0;
	m_irq_timing_h = 0;
	m_vblank_lines = 0;
	memset(m_txs, 0, sizeof(m_txs));

	m_vblank_irq_cb(CLEAR_LINE);
}


uint32_t spg290_ppu_device::regs_r(offs_t offset, uint32_t mem_mask)
{
	switch(offset << 2)
	{
	case 0x00:          // PPU Control
		return m_control;
	case 0x04:          // Sprite Control
		return m_sprite_control;
	case 0x08:          // PPU Max Sprites
		return m_sprite_max;
	case 0x0c:          // blend mode
		return m_blend_mode;
	case 0x10:          // PPU Max Sprites
		return m_transrgb;
	case 0x20:          // Text Layer 1 X pos
		return m_txs[0].posx;
	case 0x24:          // Text Layer 1 Y pos
		return m_txs[0].posy;
	case 0x28:          // Text Layer 1 attribute
		return m_txs[0].attribute;
	case 0x2c:          // Text Layer 1 control
		return m_txs[0].control;
	case 0x30:          // Text Layer 1 number ptr
		return m_txs[0].nptr;
	case 0x38:          // Text Layer 1 buffer blend
		return m_txs[0].blend;
	case 0x3c:          // Text Layer 2 X pos
		return m_txs[1].posx;
	case 0x40:          // Text Layer 2 Y pos
		return m_txs[1].posy;
	case 0x44:          // Text Layer 2 attribute
		return m_txs[1].attribute;
	case 0x48:          // Text Layer 2 control
		return m_txs[1].control;
	case 0x4c:          // Text Layer 2 number ptr
		return m_txs[1].nptr;
	case 0x54:          // Text Layer 2 buffer blend
		return m_txs[1].blend;
	case 0x58:          // Text Layer 3 X pos
		return m_txs[2].posx;
	case 0x5c:          // Text Layer 3 Y pos
		return m_txs[2].posy;
	case 0x60:          // Text Layer 3 attribute
		return m_txs[2].attribute;
	case 0x64:          // Text Layer 3 control
		return m_txs[2].control;
	case 0x68:          // Text Layer 3 number ptr
		return m_txs[2].nptr;
	case 0x70:          // Tx3 buffer blend
		return m_txs[2].blend;
	case 0x74:          // vertical comp value
		return m_vcomp_value;
	case 0x78:          // vertical comp offset
		return m_vcomp_offset;
	case 0x7c:          // vertical comp step
		return m_vcomp_step;
	case 0x80:          // PPU IRQ Control
		return m_irq_control;
	case 0x84:          // PPU IRQ ack
		return m_irq_status;
	case 0x88:          // IRQTimingV
		return m_irq_timing_v;
	case 0x8c:          // IRQTimingH
		return m_irq_timing_h;
	case 0x90:          // vblank lines
		return m_vblank_lines;
	case 0x94:          // lines count
		return m_vblank_lines + m_screen->vpos();
	case 0xa0: case 0xa4: case 0xa8:          // Tx1 buffer start
		return m_txs[0].buf_start[offset & 3];
	case 0xac: case 0xb0: case 0xb4:          // Tx2 buffer start
		return m_txs[1].buf_start[(offset+1) & 3];
	case 0xb8: case 0xbc: case 0xc0:          // Tx3 buffer starts
		return m_txs[2].buf_start[(offset+2) & 3];
	case 0xc4: case 0xc8: case 0xcc:          // frame buffer start
		return m_frame_buff[(offset-1) & 3];
	case 0xd0:          // Sprites buffer start
		return m_sprite_buf_start;
	default:
		logerror("[%s] %s: unknown read %x\n", tag(), machine().describe_context(), offset << 2);
	}

	return 0;
}


void spg290_ppu_device::regs_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	switch(offset << 2)
	{
	case 0x00:          // PPU Control
		COMBINE_DATA(&m_control);
		break;
	case 0x04:          // Sprite Control
		COMBINE_DATA(&m_sprite_control);
		break;
	case 0x08:          // PPU Max Sprites
		COMBINE_DATA(&m_sprite_max);
		break;
	case 0x0c:          // blend mode
		COMBINE_DATA(&m_blend_mode);
		break;
	case 0x10:          // PPU Max Sprites
		COMBINE_DATA(&m_transrgb);
		break;
	case 0x20:          // Text Layer 1 X pos
		COMBINE_DATA(&m_txs[0].posx);
		break;
	case 0x24:          // Text Layer 1 Y pos
		COMBINE_DATA(&m_txs[0].posy);
		break;
	case 0x28:          // Text Layer 1 attribute
		COMBINE_DATA(&m_txs[0].attribute);
		break;
	case 0x2c:          // Text Layer 1 control
		COMBINE_DATA(&m_txs[0].control);
		break;
	case 0x30:          // Text Layer 1 number ptr
		COMBINE_DATA(&m_txs[0].nptr);
		break;
	case 0x38:          // Text Layer 1 buffer blend
		COMBINE_DATA(&m_txs[0].blend);
		break;
	case 0x3c:          // Text Layer 2 X pos
		COMBINE_DATA(&m_txs[1].posx);
		break;
	case 0x40:          // Text Layer 2 Y pos
		COMBINE_DATA(&m_txs[1].posy);
		break;
	case 0x44:          // Text Layer 2 attribute
		COMBINE_DATA(&m_txs[1].attribute);
		break;
	case 0x48:          // Text Layer 2 control
		COMBINE_DATA(&m_txs[1].control);
		break;
	case 0x4c:          // Text Layer 2 number ptr
		COMBINE_DATA(&m_txs[1].nptr);
		break;
	case 0x54:          // Text Layer 2 buffer blend
		COMBINE_DATA(&m_txs[1].blend);
		break;
	case 0x58:          // Text Layer 3 X pos
		COMBINE_DATA(&m_txs[2].posx);
		break;
	case 0x5c:          // Text Layer 3 Y pos
		COMBINE_DATA(&m_txs[2].posy);
		break;
	case 0x60:          // Text Layer 3 attribute
		COMBINE_DATA(&m_txs[2].attribute);
		break;
	case 0x64:          // Text Layer 3 control
		COMBINE_DATA(&m_txs[2].control);
		break;
	case 0x68:          // Text Layer 3 number ptr
		COMBINE_DATA(&m_txs[2].nptr);
		break;
	case 0x70:          // Tx3 buffer blend
		COMBINE_DATA(&m_txs[2].blend);
		break;
	case 0x74:          // vertical comp value
		COMBINE_DATA(&m_vcomp_value);
		break;
	case 0x78:          // vertical comp offset
		COMBINE_DATA(&m_vcomp_offset);
		break;
	case 0x7c:          // vertical comp step
		COMBINE_DATA(&m_vcomp_step);
		break;
	case 0x80:          // PPU IRQ Control
		COMBINE_DATA(&m_irq_control);
		m_irq_status &= m_irq_control;
		break;
	case 0x84:          // PPU IRQ ack
		if (ACCESSING_BITS_0_7)
		{
			m_irq_status &= ~data;

			if (!(m_irq_status & m_irq_control & 0x07))
				m_vblank_irq_cb(CLEAR_LINE);
		}
		break;
	case 0x88:          // IRQTimingV
		COMBINE_DATA(&m_irq_timing_v);
		break;
	case 0x8c:          // IRQTimingH
		COMBINE_DATA(&m_irq_timing_h);
		break;
	case 0x90:          // vblank lines
		COMBINE_DATA(&m_vblank_lines);
		break;
	case 0xa0: case 0xa4: case 0xa8:          // Tx1 buffer start
		COMBINE_DATA(&m_txs[0].buf_start[offset & 3]);
		break;
	case 0xac: case 0xb0: case 0xb4:          // Tx2 buffer start
		COMBINE_DATA(&m_txs[1].buf_start[(offset+1) & 3]);
		break;
	case 0xb8: case 0xbc: case 0xc0:          // Tx3 buffer starts
		COMBINE_DATA(&m_txs[2].buf_start[(offset+2) & 3]);
		break;
	case 0xc4: case 0xc8: case 0xcc:          // frame buffer start
		COMBINE_DATA(&m_frame_buff[(offset-1) & 3]);
		break;
	case 0xd0:          // Sprites buffer start
		COMBINE_DATA(&m_sprite_buf_start);
		break;
	default:
		logerror("[%s] %s: unknown write %x = %x\n", tag(), machine().describe_context(), offset << 2, data);
	}
}

void spg290_ppu_device::screen_vblank(int state)
{
	if (state && (m_irq_control & 0x01))       // VBlanking Start IRQ
	{
		m_irq_status |= 0x01;
		m_vblank_irq_cb(ASSERT_LINE);
	}
	else if (!state && (m_irq_control & 0x02)) // VBlanking End IRQ
	{
		m_irq_status |= 0x02;
		m_vblank_irq_cb(ASSERT_LINE);
	}
}

inline rgb_t spg290_ppu_device::blend_colors(const rgb_t &c0, const rgb_t &c1, uint8_t level)
{
	if (m_blend_mode & 1)
	{
		int r = (c0.r() * level / 63) - (c1.r() * (63 - level) / 63);
		int g = (c0.g() * level / 63) - (c1.g() * (63 - level) / 63);
		int b = (c0.b() * level / 63) - (c1.b() * (63 - level) / 63);
		return rgb_t(r, g, b);
	}
	else
	{
		int r = (c0.r() * level / 63) + (c1.r() * (63 - level) / 63);
		int g = (c0.g() * level / 63) + (c1.g() * (63 - level) / 63);
		int b = (c0.b() * level / 63) + (c1.b() * (63 - level) / 63);
		return rgb_t(r, g, b);
	}
}

inline void spg290_ppu_device::argb1555(bitmap_rgb32 &bitmap, const rectangle &cliprect, uint16_t posy, uint16_t posx, uint16_t argb, uint8_t blend)
{
	if (!(argb & 0x8000) && cliprect.contains(posx, posy))
	{
		rgb_t color = rgb_t(pal5bit(argb >> 10), pal5bit(argb >> 5), pal5bit(argb >> 0));
		if (blend)
			color = blend_colors(bitmap.pix(posy, posx), color, blend);

		bitmap.pix(posy, posx) = color;
	}
}

inline void spg290_ppu_device::rgb565(bitmap_rgb32 &bitmap, const rectangle &cliprect, uint16_t posy, uint16_t posx, uint16_t rgb, uint8_t blend)
{
	if ((!(m_transrgb & 0x10000) || (m_transrgb & 0xffff) != rgb) && cliprect.contains(posx, posy))
	{
		rgb_t color = rgb_t(pal5bit(rgb >> 11), pal6bit(rgb >> 5), pal5bit(rgb >> 0));
		if (blend)
			color = blend_colors(bitmap.pix(posy, posx), color, blend);

		bitmap.pix(posy, posx) = color;
	}
}

void spg290_ppu_device::blit_sprite(bitmap_rgb32 &bitmap, const rectangle &cliprect, uint32_t control, uint32_t attribute, uint32_t buf_start)
{
	uint16_t sprite_num   = control & 0xffff;
	uint16_t sprite_x     = (control >> 16) & 0x3ff;
	uint16_t sprite_y     = (attribute >> 16) & 0x3ff;
	uint8_t  sprite_hsize = 8 << ((attribute >> 4) & 0x03);
	uint8_t  sprite_vsize = 8 << ((attribute >> 6) & 0x03);
	uint8_t  bit_pixel    = ((attribute & 3) + 1) << 1;
	uint8_t  sprite_flip  = (attribute >> 2) & 0x03;
	uint16_t pen_bank     = ((attribute >> 8) & 0x1f) * 0x10;
	uint8_t  blend        = (attribute & 0x8000) ? 0x3f - ((attribute >> 26) & 0x3f) : 0;
	uint8_t  pixel_word   = 32 / bit_pixel;
	uint8_t  pixel_byte   = 8 / bit_pixel;
	uint8_t  word_line    = sprite_hsize / pixel_word;
	uint16_t pen_mask     = (1 << bit_pixel) - 1;

	if (sprite_num == 0)
		return;

	uint32_t sprite_base = buf_start + (sprite_num * (sprite_hsize * sprite_vsize * bit_pixel / 8));

	if (sprite_x > cliprect.max_x)  sprite_x = (sprite_x & 0x3ff) - 0x400;
	if (sprite_y > cliprect.max_y)  sprite_y = (sprite_y & 0x3ff) - 0x400;

	for (int y=0; y < sprite_vsize; y++)
		for (int x=0; x < word_line; x++)
		{
			uint32_t data = m_space_read_cb(sprite_base + (y * word_line + x) * 4);

			for (int i=0; i < 4; i++)
			{
				for (int b=0; b < pixel_byte; b++)
				{
					int bitpos = i * pixel_byte + b;
					uint16_t pen = (data >> (8 - (b + 1) * bit_pixel)) & pen_mask;
					uint16_t posx = cliprect.min_x;
					if (sprite_flip & 0x01)
						posx += sprite_x + (sprite_hsize - (x * pixel_word + bitpos));
					else
						posx += sprite_x + x * pixel_word + bitpos;

					uint16_t posy = cliprect.min_y;
					if (sprite_flip & 0x02)
						posy += sprite_y + (sprite_vsize - y);
					else
						posy += sprite_y + y;

					argb1555(bitmap, cliprect, posy, posx, m_sprite_palette_ram[(pen_bank + pen) & 0x1ff], blend);
				}

				data >>= 8;
			}
		}
}

void spg290_ppu_device::blit_bitmap(bitmap_rgb32 &bitmap, const rectangle &cliprect, uint32_t control, uint32_t attribute, int posy, int posx, uint32_t nptr, uint32_t buf_start, uint8_t blend)
{
	int max_x = cliprect.max_x + posx;
	int max_y = cliprect.max_y + posy;
	for (int y=0; y < max_y; y++)
	{
		int line = (control & 0x04) ? 0 : y;
		uint32_t tx_start = m_space_read_cb(nptr + line * 4) * 2;

		for (int x=0; x < max_x; x+=2)
		{
			uint16_t px = cliprect.min_x + x;
			uint16_t py = cliprect.min_y + y;
			uint32_t data = m_space_read_cb(buf_start + tx_start + x * 2);

			for (int b=0; b < 2; b++)
			{
				if (control & 0x1000)
					rgb565(bitmap, cliprect, py - posy, px + b - posx, data & 0xffff, blend);
				else
					argb1555(bitmap, cliprect, py - posy, px + b - posx, data & 0xffff, blend);

				data >>= 16;
			}
		}
	}
}

void spg290_ppu_device::blit_character(bitmap_rgb32 &bitmap, const rectangle &cliprect, uint32_t control, uint32_t attribute, int posy, int posx, uint32_t nptr, uint32_t buf_start, uint8_t blend)
{
	uint8_t  hsize      = 8 << ((attribute >> 4) & 0x03);
	uint8_t  vsize      = 8 << ((attribute >> 6) & 0x03);
	uint8_t  bit_pixel  = ((attribute & 3) + 1) << 1;
	uint16_t pen_bank   = ((attribute >> 8) & 0x1f) * 0x10;
	//uint8_t  char_flip  = (attribute >> 2) & 0x03;
	uint8_t  pixel_word = 32 / bit_pixel;
	uint8_t  chars_line = 1024 / hsize;
	uint16_t pen_mask   = (1 << bit_pixel) - 1;

	int max_x = (cliprect.max_x + posx) / vsize + 1;
	int max_y = (cliprect.max_y + posy) / hsize + 1;

	for (int y=0; y <= max_y; y++)
	{
		for (int x=0; x <= max_x; x++)
		{
			int line = (control & 0x04) ? 0 : y;
			uint32_t addr = line * chars_line + x;
			uint32_t char_id = m_space_read_cb(nptr + addr * 2);

			if (addr & 1)
				char_id >>= 16;

			uint32_t char_addr = buf_start + (char_id & 0xffff) * hsize * vsize;

			for (int cy=0; cy < vsize; cy++)
			{
				uint32_t line_addr = char_addr + cy * hsize;
				for (int cx=0; cx < hsize; cx++)
				{
					uint32_t data = m_space_read_cb(line_addr + (cx * bit_pixel) / 8);
					uint16_t px = cliprect.min_x + x * hsize + cx;
					uint16_t py = cliprect.min_y + y * vsize + cy;
					uint16_t pen = (data >> ((cx % pixel_word) * bit_pixel)) & pen_mask;
					uint16_t pix = m_char_palette_ram[(pen_bank + pen) & 0x3ff];

					if (control & 0x1000)
						rgb565(bitmap, cliprect, py - posy, px - posx, pix, blend);
					else
						argb1555(bitmap, cliprect, py - posy, px - posx, pix, blend);
				}
			}
		}
	}
}


uint32_t spg290_ppu_device::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(rgb_t::black(), cliprect);

	if (!(m_control & 0x1000))
		return 0;

	for (int depth=0; depth<4; depth++)
	{
		//if (machine().input().code_pressed(KEYCODE_1_PAD) && depth == 0)  continue;
		//if (machine().input().code_pressed(KEYCODE_2_PAD) && depth == 1)  continue;
		//if (machine().input().code_pressed(KEYCODE_3_PAD) && depth == 2)  continue;
		//if (machine().input().code_pressed(KEYCODE_4_PAD) && depth == 3)  continue;

		// draw the bitmap/text layers
		for (int l=0; l<3; l++)
		{
			//if (machine().input().code_pressed(KEYCODE_7_PAD) && l == 0)      continue;
			//if (machine().input().code_pressed(KEYCODE_8_PAD) && l == 1)      continue;
			//if (machine().input().code_pressed(KEYCODE_9_PAD) && l == 2)      continue;

			if ((m_txs[l].control & 0x08) && ((m_txs[l].attribute >> 13) & 3) == depth)
			{
				uint8_t blend = (m_txs[l].control & 0x0100) ? m_txs[l].blend : 0;

				// posx is 10-bit two's complement
				int posx = m_txs[l].posx & 0x3ff;
				if (posx & 0x200)
					posx = (posx & 0x3ff) - 0x400;

				// posy is 9-bit two's complement
				int posy = m_txs[l].posy & 0x1ff;
				if (posy & 0x100)
					posy = (posy & 0x1ff) - 0x200;

				if (m_txs[l].control & 0x01)
					blit_bitmap(bitmap, cliprect, m_txs[l].control, m_txs[l].attribute, posy, posx, m_txs[l].nptr, m_txs[l].buf_start[0], blend);
				else
					blit_character(bitmap, cliprect, m_txs[l].control, m_txs[l].attribute, posy, posx, m_txs[l].nptr, m_txs[l].buf_start[0], blend);
			}
		}

		//if (machine().input().code_pressed(KEYCODE_0_PAD))    continue;

		// draw the sprites
		if (m_sprite_control & 1)
		{
			for (int i=0; i <= (m_sprite_max & 0x1ff); i++)
			{
				if (((m_sprite_ram[i * 2 + 1] >> 13) & 3) == depth)
					blit_sprite(bitmap, cliprect, m_sprite_ram[i * 2], m_sprite_ram[i * 2 + 1], m_sprite_buf_start);
			}
		}
	}

	return 0;
}
