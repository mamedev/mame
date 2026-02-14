// license:BSD-3-Clause
// copyright-holders:Barry Rodewald
/*
 * mach32.c
 *
 *  Implementation of the ATi mach32 and mach64 video chips
 *  Based on ati_vga and mach8
 *
 *  Created on: 16/05/2014
 */

#include "emu.h"
#include "ati_mach32.h"

#include "screen.h"

#define VERBOSE (LOG_GENERAL)
//#define LOG_OUTPUT_FUNC osd_printf_info
#include "logmacro.h"

DEFINE_DEVICE_TYPE(ATIMACH32,       mach32_device,       "mach32",       "ATi mach32")
DEFINE_DEVICE_TYPE(ATIMACH32_8514A, mach32_8514a_device, "mach32_8514a", "ATi mach32 (2D acceleration module)")
DEFINE_DEVICE_TYPE(ATIMACH64,       mach64_device,       "mach64",       "ATi mach64")
DEFINE_DEVICE_TYPE(ATIMACH64_8514A, mach64_8514a_device, "mach64_8514a", "ATi mach64 (2D acceleration module)")


/*
 *  mach32
 */

// 8514/A device
mach32_8514a_device::mach32_8514a_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: mach32_8514a_device(mconfig, ATIMACH32_8514A, tag, owner, clock)
{
}

mach32_8514a_device::mach32_8514a_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: mach8_device(mconfig, type, tag, owner, clock)
	, m_chip_ID(0)
	, m_membounds(0)
{
}


// SVGA device
mach32_device::mach32_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: mach32_device(mconfig, ATIMACH32, tag, owner, clock)
{
}

mach32_device::mach32_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: ati_vga_device(mconfig, type, tag, owner, clock)
	, m_8514a(*this, "8514a")
	, m_cursor_enable(false)
{
}

void mach32_device::device_add_mconfig(machine_config &config)
{
	ATIMACH32_8514A(config, "8514a", 0).set_vga(DEVICE_SELF);
}

void mach32_8514a_device::device_start()
{
	mach8_device::device_start();
	//    017h  68800-AX
	//    177h  68800-LX
	//    2F7h  68800-6
	//  The 68800-3 appears to return 0 for this field (undocumented)
	m_chip_ID = 0x17;
	m_membounds = 0;
}

// Configuration Status Register 1 (read only)
// bit 0:     Disable VGA: 0=VGA+8514/A, 1=8514/A only
// bits 1-3:  Bus Type:  0=16-bit ISA. 1=EISA, 2=16-bit MCA, 3=32-bit MCA, 4=LBus 386SX
//                       5=LBus 386DX, 6=LBus 486. 7=PCI
// bits 4-6:  RAM Type:  3=256Kx16 DRAM
// bit 7:     Chip Disable
// bit 8:     TST_VCTR_ENA:  1=delay memory write by 1/2 MCLK to test vector generation
// bits 9-11: DAC Type:  0=ATI68830, 1=SC11483, 2=ATI68875, 3=Bt476, 4=Bt481, 5=ATI68860 (68800AX or higher)
//                       The Graphics Ultra Pro has an ATI68875
// bit 12:    Enable internal uC address decode
// bit 13-15: Card ID:  ID when using multiple controllers
uint16_t mach32_8514a_device::mach32_config1_r()
{
	return 0x0430;  // enable VGA, 16-bit ISA, 256Kx16 DRAM, ATI68875
}


/* 7AEE (W)   Mach 32
 * bits    0-2  Monitor Alias - Monitor ID
 * bit       3  Enable reporting of Monitor Alias
 * bits    4-5  Pixel Width (0=4bpp, 1=8bpp, 2=16bpp, 3=24bpp)
 * bits    6-7  16bpp colour mode (0=555, 1=565, 2=655, 3=664)
 * bit       8  Multiplex pixels - processes 4 pixels in parallel by the DAC
 * bit       9  24bpp config (0=3Bytes/pixel, 1=4Bytes/pixel)
 * bit      10  24bpp colour order (0=RGB, LSB reserved in 4Bpp, 1=BGR, MSB reserved in 4Bpp)
 * bit      11  Display pixel size to be written
 * bits  12-13  DAC extended address inputs RS2,RS3
 * bit      14  Enabled 8-bit DAC (0=6-bit)
 * bit      15  Draw pixel size to be written
 * If bits 11 and 15 are both 0, then for compatibility, both will be written
 */
void mach32_8514a_device::mach32_ge_ext_config_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	if(offset == 1)
	{
		COMBINE_DATA(&mach8.ge_ext_config);

		if ((data & 0x0030) != (mach8.ge_ext_config & 0x0030))
			display_mode_change = true;
		if(data & 0x0800)
			display_mode_change = true;
		if(!(data & 0x8000) && (!(data & 0x0800)))
			display_mode_change = true;
	}
}

void mach32_8514a_device::device_reset()
{
}

void mach32_8514a_device::mach32_mem_boundary_w(uint16_t data)
{
	m_membounds = data;
	if(data & 0x10)
		LOG("ATI: Unimplemented memory boundary activated.\n");
}


void mach32_device::device_start()
{
	ati_vga_device::device_start();
	ati.vga_chip_id = 0x00;  // correct?
}

void mach32_device::device_reset()
{
	ati_vga_device::device_reset();
}

// TODO: has issues in SDD 15bpp tests and onward, MACH64 doesn't work at all
void mach32_device::ati_define_video_mode()
{
	uint16_t config = m_8514a->get_ext_config();

	ati_vga_device::ati_define_video_mode();

	// SDD disagrees with this
	//if(ati.ext_reg[0x30] & 0x20)
	{
		// TODO: use devcb instead of this goofy handling
		if(m_8514a->has_display_mode_changed())
		{
			svga.rgb8_en = svga.rgb15_en = svga.rgb16_en = svga.rgb24_en = svga.rgb32_en = 0;

			switch(config & 0x0030)  // pixel depth
			{
				case 0x0000:
					break;
				case 0x0010:
					svga.rgb8_en = 1;
					break;
				case 0x0020:
				{
					switch(config & 0xc0)
					{
						case 0x00: svga.rgb15_en = 1; break;
						case 0x40: svga.rgb16_en = 1; break;
						default:
							// TODO: should be possible to tell depending how it's used
							popmessage("ati_mach32.cpp: unemulated pixel mode 664RGB/655RGB");
							break;
					}
					break;
				}
				case 0x0030:
					if (BIT(config, 9))
						svga.rgb32_en = 1;
					else
						svga.rgb24_en = 1;
					break;
			}
		}
	}
	//else
	//{
	//	ati_vga_device::ati_define_video_mode();
	//	return;
	//}

	set_dot_clock();
}

uint16_t mach32_device::offset()
{
	if(get_video_depth() >= 15)
		return ati_vga_device::offset() << 1;
	return ati_vga_device::offset();
}

uint32_t mach32_device::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	ati_vga_device::screen_update(screen, bitmap, cliprect);
	draw_hw_cursor(screen, bitmap, cliprect);

	return 0;
}

uint32_t mach32_device::draw_hw_cursor(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	uint8_t depth = get_video_depth();

	if(!m_cursor_enable)
		return 0;

	uint32_t src = (m_cursor_address & 0x000fffff) << 2;

	uint32_t colour0, colour1;
	if(depth == 8)
	{
		colour0 = pen(m_cursor_colour0_b);
		colour1 = pen(m_cursor_colour1_b);
	}
	else  // 16/24/32bpp
	{
		colour0 = (m_cursor_colour0_r << 16) | (m_cursor_colour0_g << 8) | (m_cursor_colour0_b);
		colour1 = (m_cursor_colour1_r << 16) | (m_cursor_colour1_g << 8) | (m_cursor_colour1_b);
	}

	// draw hardware pointer (64x64 max)
	for(uint8_t y=0;y<64;y++)
	{
		uint32_t *dst = &bitmap.pix(m_cursor_vertical + y, m_cursor_horizontal);
		for(uint8_t x=0;x<64;x+=8)
		{
			uint16_t const bits = (vga.memory[(src+0) % vga.svga_intf.vram_size] | ((vga.memory[(src+1) % vga.svga_intf.vram_size]) << 8));

			for(uint8_t z=0;z<8;z++)
			{
				if(((z + x) > (m_cursor_offset_horizontal-1)) && (y < (63 - m_cursor_offset_vertical)))
				{
					uint8_t const val = (bits >> (z*2)) & 0x03;
					switch(val)
					{
						case 0:  // cursor colour 0
							*dst = colour0;
							break;
						case 1:  // cursor colour 1
							*dst = colour1;
							break;
						case 2:  // transparent
							break;
						case 3:  // complement
							*dst = ~(*dst);
							break;
					}
					dst++;
				}
			}
			src+=2;
		}
	}

	return 0;
}

// mach32 Hardware Pointer
void mach32_device::mach32_cursor_l_w(offs_t offset, uint16_t data)
{
	if(offset == 1)
		m_cursor_address = (m_cursor_address & 0xf0000) | data;
	LOG("mach32 HW pointer data address: %05x",m_cursor_address);
}

void mach32_device::mach32_cursor_h_w(offs_t offset, uint16_t data)
{
	if(offset == 1)
	{
		m_cursor_address = (m_cursor_address & 0x0ffff) | ((data & 0x000f) << 16);
		m_cursor_enable = data & 0x8000;
		LOG("mach32 HW pointer data address: %05x",m_cursor_address);
	}
}

void mach32_device::mach32_cursor_pos_h(offs_t offset, uint16_t data)
{
	if(offset == 1)
		m_cursor_horizontal = data & 0x07ff;
}

void mach32_device::mach32_cursor_pos_v(offs_t offset, uint16_t data)
{
	if(offset == 1)
		m_cursor_vertical = data & 0x0fff;
}

void mach32_device::mach32_cursor_colour_b_w(offs_t offset, uint16_t data)
{
	if(offset == 1)
	{
		m_cursor_colour0_b = data & 0xff;
		m_cursor_colour1_b = data >> 8;
		LOG("Mach32: HW Cursor Colour Blue write RGB: 0: %02x %02x %02x  1: %02x %02x %02x\n"
			,m_cursor_colour0_r,m_cursor_colour0_g,m_cursor_colour0_b,m_cursor_colour1_r,m_cursor_colour1_g,m_cursor_colour1_b);
	}
}

void mach32_device::mach32_cursor_colour_0_w(offs_t offset, uint16_t data)
{
	if(offset == 1)
	{
		m_cursor_colour0_g = data & 0xff;
		m_cursor_colour0_r = data >> 8;
		LOG("Mach32: HW Cursor Colour 0 write RGB: %02x %02x %02x\n",m_cursor_colour0_r,m_cursor_colour0_g,m_cursor_colour0_b);
	}
}

void mach32_device::mach32_cursor_colour_1_w(offs_t offset, uint16_t data)
{
	if(offset == 1)
	{
		m_cursor_colour1_g = data & 0xff;
		m_cursor_colour1_r = data >> 8;
		LOG("Mach32: HW Cursor Colour 1 write RGB: %02x %02x %02x\n",m_cursor_colour1_r,m_cursor_colour1_g,m_cursor_colour1_b);
	}
}

void mach32_device::mach32_cursor_offset_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	if(offset == 1)
	{
		if(ACCESSING_BITS_0_7)
			m_cursor_offset_horizontal = data & 0x00ff;
		if(ACCESSING_BITS_8_15)
			m_cursor_offset_vertical = data >> 8;
		LOG("Mach32: HW Cursor Offset write H:%i V:%i\n",m_cursor_offset_horizontal,m_cursor_offset_vertical);
	}
}

void mach32_device::refresh_bank()
{
	const u8 ati32 = ati.ext_reg[0x32];
	const u8 ati3e = ati.ext_reg[0x3e];
	const u8 ati2e = ati.ext_reg[0x2e];
	if(ati3e & 0x08)
	{
		svga.bank_r = ((ati32 & 0x01) << 3) | ((ati32 & 0xe0) >> 5);
		svga.bank_r|= (ati2e & 0xc) << 2;
		svga.bank_w = ((ati32 & 0x1e) >> 1);
		svga.bank_w|= (ati2e & 0x3) << 4;
	}
	else
	{
		svga.bank_r = (ati32 & 0x1e) >> 1;
		svga.bank_r|= (ati2e & 3) << 4;
		svga.bank_w = svga.bank_r;
	}
}

/*
 *   mach64
 */

// 8514/A device
mach64_8514a_device::mach64_8514a_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: mach64_8514a_device(mconfig, ATIMACH64_8514A, tag, owner, clock)
{
}

mach64_8514a_device::mach64_8514a_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: mach32_8514a_device(mconfig, type, tag, owner, clock)
{
}


// SVGA device
mach64_device::mach64_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: mach64_device(mconfig, ATIMACH64, tag, owner, clock)
{
}

mach64_device::mach64_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: mach32_device(mconfig, type, tag, owner, clock), m_8514a(*this, "8514a")
{
}

void mach64_device::device_add_mconfig(machine_config &config)
{
	ATIMACH64_8514A(config, "8514a", 0).set_vga(DEVICE_SELF);
}

void mach64_8514a_device::device_start()
{
	mach32_8514a_device::device_start();
	//    017h  68800-AX
	//    177h  68800-LX
	//    2F7h  68800-6
	//  The 68800-3 appears to return 0 for this field (undocumented)
	m_chip_ID = 0x0000;  // value is unknown for mach64
	m_membounds = 0;
}

void mach64_8514a_device::device_reset()
{
}

void mach64_device::device_start()
{
	mach32_device::device_start();
}

void mach64_device::device_reset()
{
	mach32_device::device_reset();
}

void mach64_device::set_color(u8 index, u32 color)
{
	// update the palette device so we can see in the F4 viewer what the Rage is doing
	set_pen_color(index, (color>>16) & 0xff, (color>>8) & 0xff, color & 0xff);
}

u32 mach64_device::framebuffer_r(offs_t offset, u32 mem_mask)
{
	const u32 *target = (u32 *)&vga.memory[offset<<2];
	return *target;
}

void mach64_device::framebuffer_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA((u32*)&vga.memory[offset<<2]);
}

u32 mach64_device::framebuffer_be_r(offs_t offset, u32 mem_mask)
{
	const u32 *target = (u32 *)&vga.memory[offset<<2];
	return swapendian_int32(*target);
}

void mach64_device::framebuffer_be_w(offs_t offset, u32 data, u32 mem_mask)
{
	data = swapendian_int32(data);
	mem_mask = swapendian_int32(mem_mask);
	COMBINE_DATA((u32*)&vga.memory[offset<<2]);
}
