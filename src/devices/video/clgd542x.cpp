// license:BSD-3-Clause
// copyright-holders:Barry Rodewald
/*

    Cirrus Logic GD542x/3x video chipsets

*/

#include "emu.h"
#include "clgd542x.h"

#include "screen.h"


#define LOG_REG 0
#define LOG_BLIT 1

#define CRTC_PORT_ADDR ((vga.miscellaneous_output&1)?0x3d0:0x3b0)

//#define TEXT_LINES (LINES_HELPER)
#define LINES (vga.crtc.vert_disp_end+1)
#define TEXT_LINES (vga.crtc.vert_disp_end+1)

#define GRAPHIC_MODE (vga.gc.alpha_dis) /* else text mode */

#define EGA_COLUMNS (vga.crtc.horz_disp_end+1)
#define EGA_START_ADDRESS (vga.crtc.start_addr)
#define EGA_LINE_LENGTH (vga.crtc.offset<<1)

#define VGA_COLUMNS (vga.crtc.horz_disp_end+1)
#define VGA_START_ADDRESS (vga.crtc.start_addr)
#define VGA_LINE_LENGTH (vga.crtc.offset<<3)

#define IBM8514_LINE_LENGTH (m_vga->offset())

#define VGA_CH_WIDTH ((vga.sequencer.data[1]&1)?8:9)

#define TEXT_COLUMNS (vga.crtc.horz_disp_end+1)
#define TEXT_START_ADDRESS (vga.crtc.start_addr<<3)
#define TEXT_LINE_LENGTH (vga.crtc.offset<<1)

#define TEXT_COPY_9COLUMN(ch) (((ch & 0xe0) == 0xc0)&&(vga.attribute.data[0x10]&4))

DEFINE_DEVICE_TYPE(CIRRUS_GD5428, cirrus_gd5428_device, "clgd5428", "Cirrus Logic GD5428")
DEFINE_DEVICE_TYPE(CIRRUS_GD5430, cirrus_gd5430_device, "clgd5430", "Cirrus Logic GD5430")
DEFINE_DEVICE_TYPE(CIRRUS_GD5446, cirrus_gd5446_device, "clgd5446", "Cirrus Logic GD5446")


cirrus_gd5428_device::cirrus_gd5428_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: cirrus_gd5428_device(mconfig, CIRRUS_GD5428, tag, owner, clock)
{
}

cirrus_gd5428_device::cirrus_gd5428_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: svga_device(mconfig, type, tag, owner, clock)
{
}

cirrus_gd5430_device::cirrus_gd5430_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: cirrus_gd5428_device(mconfig, CIRRUS_GD5430, tag, owner, clock)
{
}

cirrus_gd5446_device::cirrus_gd5446_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: cirrus_gd5428_device(mconfig, CIRRUS_GD5446, tag, owner, clock)
{
}

void cirrus_gd5428_device::device_start()
{
	zero();

	for (int i = 0; i < 0x100; i++)
		set_pen_color(i, 0, 0, 0);

	// Avoid an infinite loop when displaying.  0 is not possible anyway.
	vga.crtc.maximum_scan_line = 1;

	// copy over interfaces
	vga.read_dipswitch.set(nullptr); //read_dipswitch;
	vga.svga_intf.seq_regcount = 0x1f;
	vga.svga_intf.crtc_regcount = 0x2d;
	vga.memory = std::make_unique<uint8_t []>(vga.svga_intf.vram_size);
	memset(&vga.memory[0], 0, vga.svga_intf.vram_size);

	save_pointer(NAME(vga.memory), vga.svga_intf.vram_size);
	save_pointer(vga.crtc.data,"CRTC Registers",0x100);
	save_pointer(vga.sequencer.data,"Sequencer Registers",0x100);
	save_pointer(vga.attribute.data,"Attribute Registers", 0x15);
	save_item(NAME(m_chip_id));

	m_vblank_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(vga_device::vblank_timer_cb),this));

	m_chip_id = 0x98;  // GD5428 - Rev 0
}

void cirrus_gd5430_device::device_start()
{
	cirrus_gd5428_device::device_start();
	m_chip_id = 0xa0;  // GD5430 - Rev 0
}

void cirrus_gd5446_device::device_start()
{
	cirrus_gd5428_device::device_start();
	m_chip_id = 0x80 | 0x39;  // GD5446
}


void cirrus_gd5428_device::device_reset()
{
	vga_device::device_reset();
	gc_locked = true;
	gc_mode_ext = 0;
	gc_bank_0 = gc_bank_1 = 0;
	m_lock_reg = 0;
	m_blt_status = 0;
	m_cursor_attr = 0x00;  // disable hardware cursor and extra palette
	m_cursor_x = m_cursor_y = 0;
	m_cursor_addr = 0;
	m_scratchpad1 = m_scratchpad2 = m_scratchpad3 = 0;
	m_cr19 = m_cr1a = m_cr1b = 0;
	m_vclk_num[0] = 0x4a;
	m_vclk_denom[0] = 0x2b;
	m_vclk_num[1] = 0x5b;
	m_vclk_denom[1] = 0x2f;
	m_blt_source = m_blt_dest = m_blt_source_current = m_blt_dest_current = 0;
	memset(m_ext_palette, 0, sizeof(m_ext_palette));
	m_ext_palette_enabled = false;
	m_blt_system_transfer = false;
}

uint32_t cirrus_gd5428_device::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	uint32_t ptr = (vga.svga_intf.vram_size - 0x4000);  // cursor patterns are stored in the last 16kB of VRAM
	svga_device::screen_update(screen, bitmap, cliprect);

	/*uint8_t cur_mode =*/ pc_vga_choosevideomode();
	if(m_cursor_attr & 0x01)  // hardware cursor enabled
	{
		// draw hardware graphics cursor
		if(m_cursor_attr & 0x04)  // 64x64
		{
			ptr += ((m_cursor_addr & 0x3c) * 256);
			for(int y=0;y<64;y++)
			{
				for(int x=0;x<64;x+=8)
				{
					for(int bit=0;bit<8;bit++)
					{
						uint8_t pixel1 = vga.memory[ptr % vga.svga_intf.vram_size] >> (7-bit);
						uint8_t pixel2 = vga.memory[(ptr+512) % vga.svga_intf.vram_size] >> (7-bit);
						uint8_t output = ((pixel1 & 0x01) << 1) | (pixel2 & 0x01);
						switch(output)
						{
						case 0:  // transparent - do nothing
							break;
						case 1:  // background
							bitmap.pix(m_cursor_y+y,m_cursor_x+x+bit) = (m_ext_palette[0].red << 16) | (m_ext_palette[0].green << 8) | (m_ext_palette[0].blue);
							break;
						case 2:  // XOR
							bitmap.pix(m_cursor_y+y,m_cursor_x+x+bit) = ~bitmap.pix(m_cursor_y+y,m_cursor_x+x+bit);
							break;
						case 3:  // foreground
							bitmap.pix(m_cursor_y+y,m_cursor_x+x+bit) = (m_ext_palette[15].red << 16) | (m_ext_palette[15].green << 8) | (m_ext_palette[15].blue);
							break;
						}
					}
				}
			}
		}
		else
		{
			ptr += ((m_cursor_addr & 0x3f) * 256);
			for(int y=0;y<32;y++)
			{
				for(int x=0;x<32;x+=8)
				{
					for(int bit=0;bit<8;bit++)
					{
						uint8_t pixel1 = vga.memory[ptr % vga.svga_intf.vram_size] >> (7-bit);
						uint8_t pixel2 = vga.memory[(ptr+128) % vga.svga_intf.vram_size] >> (7-bit);
						uint8_t output = ((pixel1 & 0x01) << 1) | (pixel2 & 0x01);
						switch(output)
						{
						case 0:  // transparent - do nothing
							break;
						case 1:  // background
							bitmap.pix(m_cursor_y+y,m_cursor_x+x+bit) = (m_ext_palette[0].red << 18) | (m_ext_palette[0].green << 10) | (m_ext_palette[0].blue << 2);
							break;
						case 2:  // XOR
							bitmap.pix(m_cursor_y+y,m_cursor_x+x+bit) = ~bitmap.pix(m_cursor_y+y,m_cursor_x+x+bit);
							break;
						case 3:  // foreground
							bitmap.pix(m_cursor_y+y,m_cursor_x+x+bit) = (m_ext_palette[15].red << 18) | (m_ext_palette[15].green << 10) | (m_ext_palette[15].blue << 2);
							break;
						}
					}
					ptr++;
				}
			}
		}
	}
	return 0;
}

void cirrus_gd5428_device::cirrus_define_video_mode()
{
	uint8_t divisor = 1;
	float clock;
	const XTAL xtal = XTAL(14'318'181);
	uint8_t clocksel = (vga.miscellaneous_output & 0xc) >> 2;

	svga.rgb8_en = 0;
	svga.rgb15_en = 0;
	svga.rgb16_en = 0;
	svga.rgb24_en = 0;
	svga.rgb32_en = 0;

	if(gc_locked || m_vclk_num[clocksel] == 0 || m_vclk_denom[clocksel] == 0)
		clock = ((vga.miscellaneous_output & 0xc) ? xtal*2: xtal*1.75).dvalue();
	else
	{
		int numerator = m_vclk_num[clocksel] & 0x7f;
		int denominator = (m_vclk_denom[clocksel] & 0x3e) >> 1;
		int mul = m_vclk_denom[clocksel] & 0x01 ? 2 : 1;
		clock = (xtal * numerator / denominator / mul).dvalue();
	}

	if (!gc_locked && (vga.sequencer.data[0x07] & 0x01))
	{
		switch(vga.sequencer.data[0x07] & 0x06)  // bit 3 is reserved on GD542x
		{
			case 0x00:  svga.rgb8_en = 1; break;
			case 0x02:  svga.rgb16_en = 1; clock /= 2; break;  // Clock / 2 for 16-bit data
			case 0x04:  svga.rgb24_en = 1; clock /= 3; break; // Clock / 3 for 24-bit data
			case 0x06:  svga.rgb16_en = 1; divisor = 2; break; // Clock rate for 16-bit data
		}
	}
	recompute_params_clock(divisor, (int)clock);
}

uint16_t cirrus_gd5428_device::offset()
{
	uint16_t off = vga_device::offset();

	if (svga.rgb8_en == 1) // guess
		off <<= 2;
	if (svga.rgb16_en == 1)
		off <<= 2;
	if (svga.rgb24_en == 1)
		off <<= 2;
	if (svga.rgb32_en == 1)
		off <<= 2;
//  popmessage("Offset: %04x  %s %s ** -- actual: %04x",vga.crtc.offset,vga.crtc.dw?"DW":"--",vga.crtc.word_mode?"BYTE":"WORD",off);
	return off;
}

void cirrus_gd5428_device::start_bitblt()
{
	uint32_t x,y;

	if(m_blt_mode & 0x01)
	{
		start_reverse_bitblt();
		return;
	}

	if(LOG_BLIT) logerror("CL: BitBLT started: Src: %06x Dst: %06x Width: %i Height %i ROP: %02x Mode: %02x\n",m_blt_source,m_blt_dest,m_blt_width,m_blt_height,m_blt_rop,m_blt_mode);

	m_blt_source_current = m_blt_source;
	m_blt_dest_current = m_blt_dest;

	for(y=0;y<=m_blt_height;y++)
	{
		for(x=0;x<=m_blt_width;x++)
		{
			if(m_blt_mode & 0x80)  // colour expand
			{
				if(m_blt_mode & 0x10)  // 16-bit colour expansion / transparency width
				{
					// use GR0/1/10/11 background/foreground regs
					uint16_t pixel = (vga.memory[m_blt_source_current % vga.svga_intf.vram_size] >> (7-((x/2) % 8)) & 0x01) ? ((m_gr11 << 8) | vga.gc.enable_set_reset) : ((m_gr10 << 8) | vga.gc.set_reset);

					if(m_blt_dest_current & 1)
						copy_pixel(pixel >> 8, vga.memory[m_blt_dest_current % vga.svga_intf.vram_size]);
					else
						copy_pixel(pixel & 0xff, vga.memory[m_blt_dest_current % vga.svga_intf.vram_size]);
					if((x % 8) == 7 && !(m_blt_mode & 0x40))  // don't increment if a pattern (it's only 8 bits)
						m_blt_source_current++;
				}
				else
				{
					uint8_t pixel = (vga.memory[m_blt_source_current % vga.svga_intf.vram_size] >> (7-(x % 8)) & 0x01) ? vga.gc.enable_set_reset : vga.gc.set_reset;  // use GR0/1/10/11 background/foreground regs

					copy_pixel(pixel, vga.memory[m_blt_dest_current % vga.svga_intf.vram_size]);
					if((x % 8) == 7 && !(m_blt_mode & 0x40))  // don't increment if a pattern (it's only 8 bits)
						m_blt_source_current++;
				}
			}
			else
			{
				copy_pixel(vga.memory[m_blt_source_current % vga.svga_intf.vram_size], vga.memory[m_blt_dest_current % vga.svga_intf.vram_size]);
				m_blt_source_current++;
			}

			m_blt_dest_current++;
			if(m_blt_mode & 0x40 && (x % 8) == 7)  // 8x8 pattern - reset pattern source location
			{
				if(m_blt_mode & 0x80) // colour expand
					m_blt_source_current = m_blt_source + (1*(y % 8)); // patterns are linear data
				else if(svga.rgb15_en || svga.rgb16_en)
				{
					if(m_blt_mode & 0x40 && (x % 16) == 15)
						m_blt_source_current = m_blt_source + (16*(y % 8));
				}
				else
					m_blt_source_current = m_blt_source + (8*(y % 8));
			}
		}
		if(m_blt_mode & 0x40)  // 8x8 pattern
		{
			if(m_blt_mode & 0x80) // colour expand
				m_blt_source_current = m_blt_source + (1*(y % 8)); // patterns are linear data
			else if(svga.rgb15_en || svga.rgb16_en)
			{
				if(m_blt_mode & 0x40 && (x % 16) == 15)
					m_blt_source_current = m_blt_source + (16*(y % 8));
			}
			else
				m_blt_source_current = m_blt_source + (8*(y % 8));
		}
		else
			m_blt_source_current = m_blt_source + (m_blt_source_pitch*(y+1));
		m_blt_dest_current = m_blt_dest + (m_blt_dest_pitch*(y+1));
	}
	m_blt_status &= ~0x02;
}

void cirrus_gd5428_device::start_reverse_bitblt()
{
	uint32_t x,y;

	if(LOG_BLIT) logerror("CL: Reverse BitBLT started: Src: %06x Dst: %06x Width: %i Height %i ROP: %02x Mode: %02x\n",m_blt_source,m_blt_dest,m_blt_width,m_blt_height,m_blt_rop,m_blt_mode);

	// Start at end of blit
	m_blt_source_current = m_blt_source;
	m_blt_dest_current = m_blt_dest;

	for(y=0;y<=m_blt_height;y++)
	{
		for(x=0;x<=m_blt_width;x++)
		{
			if(m_blt_mode & 0x80)  // colour expand
			{
				if(m_blt_mode & 0x10)  // 16-bit colour expansion / transparency width
				{
					// use GR0/1/10/11 background/foreground regs
					uint16_t pixel = (vga.memory[m_blt_source_current % vga.svga_intf.vram_size] >> (7-((x/2) % 8)) & 0x01) ? ((m_gr11 << 8) | vga.gc.enable_set_reset) : ((m_gr10 << 8) | vga.gc.set_reset);

					if(m_blt_dest_current & 1)
						copy_pixel(pixel >> 8, vga.memory[m_blt_dest_current % vga.svga_intf.vram_size]);
					else
						copy_pixel(pixel & 0xff, vga.memory[m_blt_dest_current % vga.svga_intf.vram_size]);
					if((x % 8) == 7 && !(m_blt_mode & 0x40))  // don't increment if a pattern (it's only 8 bits)
						m_blt_source_current--;
				}
				else
				{
				uint8_t pixel = (vga.memory[m_blt_source_current % vga.svga_intf.vram_size] >> (7-(x % 8)) & 0x01) ? vga.gc.enable_set_reset : vga.gc.set_reset;  // use GR0/1/10/11 background/foreground regs

				copy_pixel(pixel, vga.memory[m_blt_dest_current % vga.svga_intf.vram_size]);
				if((x % 8) == 7 && !(m_blt_mode & 0x40))  // don't decrement if a pattern (it's only 8 bits)
					m_blt_source_current--;
				}
			}
			else
			{
				copy_pixel(vga.memory[m_blt_source_current % vga.svga_intf.vram_size], vga.memory[m_blt_dest_current % vga.svga_intf.vram_size]);
				m_blt_source_current--;
			}
			m_blt_dest_current--;
			if(m_blt_mode & 0x40 && (x % 8) == 7)  // 8x8 pattern - reset pattern source location
			{
				if(m_blt_mode & 0x80) // colour expand
					m_blt_source_current = m_blt_source - (1*(y % 8)); // patterns are linear data
				else if(svga.rgb15_en || svga.rgb16_en)
				{
					if(m_blt_mode & 0x40 && (x % 16) == 15)
						m_blt_source_current = m_blt_source - (16*(y % 8));
				}
				else
					m_blt_source_current = m_blt_source - (8*(y % 8));
			}
		}
		if(m_blt_mode & 0x40)  // 8x8 pattern
		{
			if(m_blt_mode & 0x80) // colour expand
				m_blt_source_current = m_blt_source - (1*(y % 8)); // patterns are linear data
			else if(svga.rgb15_en || svga.rgb16_en)
				{
					if(m_blt_mode & 0x40 && (x % 16) == 15)
						m_blt_source_current = m_blt_source - (16*(y % 8));
				}
			else
				m_blt_source_current = m_blt_source - (8*(y % 8));
		}
		else
			m_blt_source_current = m_blt_source - (m_blt_source_pitch*(y+1));
		m_blt_dest_current = m_blt_dest - (m_blt_dest_pitch*(y+1));
	}
	m_blt_status &= ~0x02;
}

void cirrus_gd5428_device::start_system_bitblt()
{
	if(LOG_BLIT) logerror("CL: BitBLT from system memory started: Src: %06x Dst: %06x Width: %i Height %i ROP: %02x Mode: %02x\n",m_blt_source,m_blt_dest,m_blt_width,m_blt_height,m_blt_rop,m_blt_mode);
	m_blt_system_transfer = true;
	m_blt_system_count = 0;
	m_blt_system_buffer = 0;
	m_blt_pixel_count = m_blt_scan_count = 0;
	m_blt_source_current = m_blt_source;
	m_blt_dest_current = m_blt_dest;
	m_blt_status |= 0x09;
}

// non colour-expanded BitBLTs from system memory must be doubleword sized, extra bytes are ignored
void cirrus_gd5428_device::blit_dword()
{
	// TODO: add support for reverse direction
	uint8_t x,pixel;

	for(x=0;x<32;x+=8)
	{
		pixel = ((m_blt_system_buffer & (0x000000ff << x)) >> x);
		if(m_blt_pixel_count <= m_blt_width)
			copy_pixel(pixel,vga.memory[m_blt_dest_current % vga.svga_intf.vram_size]);
		m_blt_dest_current++;
		m_blt_pixel_count++;
	}
	if(m_blt_pixel_count > m_blt_width)
	{
		m_blt_pixel_count = 0;
		m_blt_scan_count++;
		m_blt_dest_current = m_blt_dest + (m_blt_dest_pitch*m_blt_scan_count);
	}
	if(m_blt_scan_count > m_blt_height)
	{
		m_blt_system_transfer = false;  //  BitBLT complete
		m_blt_status &= ~0x0b;
	}
}

// colour-expanded BitBLTs from system memory are on a byte boundary, unused bits are ignored
void cirrus_gd5428_device::blit_byte()
{
	// TODO: add support for reverse direction
	uint8_t x,pixel;

	for(x=0;x<8;x++)
	{
		// use GR0/1/10/11 background/foreground regs
		if(m_blt_dest_current & 1)
			pixel = ((m_blt_system_buffer & (0x00000001 << (7-x))) >> (7-x)) ? m_gr11 : m_gr10;
		else
			pixel = ((m_blt_system_buffer & (0x00000001 << (7-x))) >> (7-x)) ? vga.gc.enable_set_reset : vga.gc.set_reset;
		if(m_blt_pixel_count <= m_blt_width - 1)
			copy_pixel(pixel,vga.memory[m_blt_dest_current % vga.svga_intf.vram_size]);
		m_blt_dest_current++;
		m_blt_pixel_count++;
	}
	if(m_blt_pixel_count > m_blt_width)
	{
		m_blt_pixel_count = 0;
		m_blt_scan_count++;
		m_blt_dest_current = m_blt_dest + (m_blt_dest_pitch*m_blt_scan_count);
	}
	if(m_blt_scan_count > m_blt_height)
	{
		m_blt_system_transfer = false;  //  BitBLT complete
		m_blt_status &= ~0x0b;
	}
}

void cirrus_gd5428_device::copy_pixel(uint8_t src, uint8_t dst)
{
	uint8_t res = src;

	switch(m_blt_rop)
	{
	case 0x00:  // BLACK
		res = 0x00;
		break;
	case 0x0b:  // DSTINVERT
		res = ~dst;
		break;
	case 0x0d:  // SRC
		res = src;
		break;
	case 0x0e:  // WHITE
		res = 0xff;
		break;
	case 0x59:  // SRCINVERT
		res = src ^ dst;
		break;
	default:
		popmessage("CL: Unsupported BitBLT ROP mode %02x",m_blt_rop);
	}

	// handle transparency compare
	if(m_blt_mode & 0x08)  // TODO: 16-bit compare
	{
		// if ROP result matches the transparency colour, don't change the pixel
		if((res & (~m_blt_trans_colour_mask & 0xff)) == ((m_blt_trans_colour & 0xff) & (~m_blt_trans_colour_mask & 0xff)))
			return;
	}

	vga.memory[m_blt_dest_current % vga.svga_intf.vram_size] = res;
}

uint8_t cirrus_gd5428_device::cirrus_seq_reg_read(uint8_t index)
{
	uint8_t res;

	res = 0xff;

	switch(index)
	{
		case 0x02:
			if(gc_mode_ext & 0x08)
				res = vga.sequencer.map_mask & 0xff;
			else
				res = vga.sequencer.map_mask & 0x0f;
			break;
		case 0x06:
			if(gc_locked)
				return 0x0f;
			else
				return m_lock_reg;
			break;
		case 0x09:
			//printf("%02x\n",index);
			res = vga.sequencer.data[index];
			break;
		case 0x0a:
			res = m_scratchpad1;
			break;
		case 0x0b:
		case 0x0c:
		case 0x0d:
		case 0x0e:
			res = m_vclk_num[index-0x0b];
			break;
		case 0x0f:
			res = vga.sequencer.data[index] & 0xe7;
			res |= 0x18;  // 32-bit DRAM data bus width (1MB-2MB)
			break;
		case 0x12:
			res = m_cursor_attr;
			break;
		case 0x14:
			res = m_scratchpad2;
			break;
		case 0x15:
			res = m_scratchpad3;
			break;
		case 0x1b:
		case 0x1c:
		case 0x1d:
		case 0x1e:
			res = m_vclk_denom[index-0x1b];
			break;
		default:
			res = vga.sequencer.data[index];
	}

	return res;
}

void cirrus_gd5428_device::cirrus_seq_reg_write(uint8_t index, uint8_t data)
{
	if(LOG_REG) logerror("CL: SEQ write %02x to SR%02x\n",data,index);
	switch(index)
	{
		case 0x02:
			if(gc_mode_ext & 0x08)
				vga.sequencer.map_mask = data & 0xff;
			else
				vga.sequencer.map_mask = data & 0x0f;
			break;
		case 0x06:
			// Note: extensions are always enabled on the GD5429
			if((data & 0x17) == 0x12)  // bits 3,5,6,7 ignored
			{
				gc_locked = false;
				logerror("Cirrus register extensions unlocked\n");
			}
			else
			{
				gc_locked = true;
				logerror("Cirrus register extensions locked\n");
			}
			m_lock_reg = data & 0x17;
			break;
		case 0x07:
			if((data & 0xf0) != 0)
				popmessage("1MB framebuffer window enabled at %iMB (%02x)",data >> 4,data);
			vga.sequencer.data[vga.sequencer.index] = data;
			break;
		case 0x09:
			//printf("%02x %02x\n",index,data);
			vga.sequencer.data[vga.sequencer.index] = data;
			break;
		case 0x0a:
			m_scratchpad1 = data;  // GD5402/GD542x BIOS writes VRAM size here.
			break;
		case 0x0b:
		case 0x0c:
		case 0x0d:
		case 0x0e:
			m_vclk_num[index-0x0b] = data;
			break;
		case 0x10:
		case 0x30:
		case 0x50:
		case 0x70:
		case 0x90:
		case 0xb0:
		case 0xd0:
		case 0xf0:  // bits 5-7 of the register index are the low bits of the X co-ordinate
			m_cursor_x = (data << 3) | ((index & 0xe0) >> 5);
			break;
		case 0x11:
		case 0x31:
		case 0x51:
		case 0x71:
		case 0x91:
		case 0xb1:
		case 0xd1:
		case 0xf1:  // bits 5-7 of the register index are the low bits of the Y co-ordinate
			m_cursor_y = (data << 3) | ((index & 0xe0) >> 5);
			break;
		case 0x12:
			// bit 0 - enable cursor
			// bit 1 - enable extra palette (cursor colours are there)
			// bit 2 - 64x64 cursor (32x32 if clear, GD5422+)
			// bit 7 - overscan colour protect - if set, use colour 2 in the extra palette for the border (GD5424+)
			m_cursor_attr = data;
			m_ext_palette_enabled = data & 0x02;
			break;
		case 0x13:
			m_cursor_addr = data;  // bits 0 and 1 are ignored if using 64x64 cursor
			break;
		case 0x14:
			m_scratchpad2 = data;
			break;
		case 0x15:
			m_scratchpad3 = data;  // GD543x BIOS writes VRAM size here
			break;
		case 0x1b:
		case 0x1c:
		case 0x1d:
		case 0x1e:
			m_vclk_denom[index-0x1b] = data;
			break;
		default:
			vga.sequencer.data[vga.sequencer.index] = data;
			seq_reg_write(vga.sequencer.index,data);
	}
}

uint8_t cirrus_gd5428_device::cirrus_gc_reg_read(uint8_t index)
{
	uint8_t res = 0xff;

	switch(index)
	{
	case 0x00:
		if(gc_mode_ext & 0x04)
			res = vga.gc.set_reset & 0xff;
		else
			res = vga.gc.set_reset & 0x0f;
		break;
	case 0x01:
		if(gc_mode_ext & 0x04)
			res = vga.gc.enable_set_reset & 0xff;
		else
			res = vga.gc.enable_set_reset & 0x0f;
		break;
	case 0x05:
		res  = (vga.gc.shift256 & 1) << 6;
		res |= (vga.gc.shift_reg & 1) << 5;
		res |= (vga.gc.host_oe & 1) << 4;
		res |= (vga.gc.read_mode & 1) << 3;
		if(gc_mode_ext & 0x04)
			res |= (vga.gc.write_mode & 7);
		else
			res |= (vga.gc.write_mode & 3);
		break;
	case 0x09:  // Offset register 0
		res = gc_bank_0;
		break;
	case 0x0a:  // Offset register 1
		res = gc_bank_1;
		break;
	case 0x0b:  // Graphics controller mode extensions
		res = gc_mode_ext;
		break;
	case 0x0c:  // Colour Key
		break;
	case 0x0d:  // Colour Key Mask
		break;
	case 0x0e:  // Miscellaneous Control
		break;
	case 0x10:  // Background Colour Byte 1
		res = m_gr10;
		break;
	case 0x11:  // Foreground Colour Byte 1
		res = m_gr11;
		break;
	case 0x20:  // BLT Width 0
		res = m_blt_width & 0x00ff;
		break;
	case 0x21:  // BLT Width 1
		res = m_blt_width >> 8;
		break;
	case 0x22:  // BLT Height 0
		res = m_blt_height & 0x00ff;
		break;
	case 0x23:  // BLT Height 1
		res = m_blt_height >> 8;
		break;
	case 0x24:  // BLT Destination Pitch 0
		res = m_blt_dest_pitch & 0x00ff;
		break;
	case 0x25:  // BLT Destination Pitch 1
		res = m_blt_dest_pitch >> 8;
		break;
	case 0x26:  // BLT Source Pitch 0
		res = m_blt_source_pitch & 0x00ff;
		break;
	case 0x27:  // BLT Source Pitch 1
		res = m_blt_source_pitch >> 8;
		break;
	case 0x28:  // BLT Destination start 0
		res = m_blt_dest & 0x000000ff;
		break;
	case 0x29:  // BLT Destination start 1
		res = (m_blt_dest & 0x0000ff00) >> 8;
		break;
	case 0x2a:  // BLT Destination start 2
		res = (m_blt_dest & 0x00ff0000) >> 16;
		break;
	case 0x2c:  // BLT source start 0
		res = m_blt_source & 0x000000ff;
		break;
	case 0x2d:  // BLT source start 1
		res = (m_blt_source & 0x0000ff00) >> 8;
		break;
	case 0x2e:  // BLT source start 2
		res = (m_blt_source & 0x00ff0000) >> 16;
		break;
	case 0x2f:  // BLT destination write mask (GD5430/36/40)
		// TODO
		break;
	case 0x30:  // BLT Mode
		res = m_blt_mode;
		break;
	case 0x31:  // BitBLT Start / Status
		res = m_blt_status;
		break;
	case 0x32:  // BitBLT ROP mode
		res = m_blt_rop;
		break;
	case 0x34:  // BitBLT Transparent Colour
		res = m_blt_trans_colour & 0x00ff;
		break;
	case 0x35:
		res = m_blt_trans_colour >> 8;
		break;
	case 0x36:  // BitBLT Transparent Colour Mask
		res = m_blt_trans_colour_mask & 0x00ff;
		break;
	case 0x37:
		res = m_blt_trans_colour_mask >> 8;
		break;
	default:
		res = gc_reg_read(index);
	}

	return res;
}

void cirrus_gd5428_device::cirrus_gc_reg_write(uint8_t index, uint8_t data)
{
	if(LOG_REG) logerror("CL: GC write %02x to GR%02x\n",data,index);
	switch(index)
	{
	case 0x00:  // if extended writes are enabled (bit 2 of index 0bh), then index 0 and 1 are extended to 8 bits, however XFree86 does not appear to do this...
		vga.gc.set_reset = data & 0xff;
		break;
	case 0x01:
		vga.gc.enable_set_reset = data & 0xff;
		break;
	case 0x05:
		vga.gc.shift256 = (data & 0x40) >> 6;
		vga.gc.shift_reg = (data & 0x20) >> 5;
		vga.gc.host_oe = (data & 0x10) >> 4;
		vga.gc.read_mode = (data & 8) >> 3;
		if(gc_mode_ext & 0x04)
			vga.gc.write_mode = data & 7;
		else
			vga.gc.write_mode = data & 3;
		break;
	case 0x09:  // Offset register 0
		gc_bank_0 = data;
		logerror("CL: Offset register 0 set to %i\n",data);
		break;
	case 0x0a:  // Offset register 1
		gc_bank_1 = data;
		logerror("CL: Offset register 1 set to %i\n",data);
		break;
	case 0x0b:  // Graphics controller mode extensions
		gc_mode_ext = data;
		if(!(data & 0x04))
		{
			vga.gc.set_reset &= 0x0f;
			vga.gc.enable_set_reset &= 0x0f;
		}
		if(!(data & 0x08))
			vga.sequencer.map_mask &= 0x0f;
		break;
	case 0x0c:  // Colour Key
		break;
	case 0x0d:  // Colour Key Mask
		break;
	case 0x0e:  // Miscellaneous Control
		break;
	case 0x10:  // Background Colour Byte 1
		m_gr10 = data;
		break;
	case 0x11:  // Foreground Colour Byte 1
		m_gr11 = data;
		break;
	case 0x20:  // BLT Width 0
		m_blt_width = (m_blt_width & 0xff00) | data;
		break;
	case 0x21:  // BLT Width 1
		m_blt_width = (m_blt_width & 0x00ff) | (data << 8);
		break;
	case 0x22:  // BLT Height 0
		m_blt_height = (m_blt_height & 0xff00) | data;
		break;
	case 0x23:  // BLT Height 1
		m_blt_height = (m_blt_height & 0x00ff) | (data << 8);
		break;
	case 0x24:  // BLT Destination Pitch 0
		m_blt_dest_pitch = (m_blt_dest_pitch & 0xff00) | data;
		break;
	case 0x25:  // BLT Destination Pitch 1
		m_blt_dest_pitch = (m_blt_dest_pitch & 0x00ff) | (data << 8);
		break;
	case 0x26:  // BLT Source Pitch 0
		m_blt_source_pitch = (m_blt_source_pitch & 0xff00) | data;
		break;
	case 0x27:  // BLT Source Pitch 1
		m_blt_source_pitch = (m_blt_source_pitch & 0x00ff) | (data << 8);
		break;
	case 0x28:  // BLT Destination start 0
		m_blt_dest = (m_blt_dest & 0xffffff00) | data;
		break;
	case 0x29:  // BLT Destination start 1
		m_blt_dest = (m_blt_dest & 0xffff00ff) | (data << 8);
		break;
	case 0x2a:  // BLT Destination start 2
		m_blt_dest = (m_blt_dest & 0xff00ffff) | (data << 16);
		break;
	case 0x2c:  // BLT source start 0
		m_blt_source = (m_blt_source & 0xffffff00) | data;
		break;
	case 0x2d:  // BLT source start 1
		m_blt_source = (m_blt_source & 0xffff00ff) | (data << 8);
		break;
	case 0x2e:  // BLT source start 2
		m_blt_source = (m_blt_source & 0xff00ffff) | (data << 16);
		break;
	case 0x2f:  // BLT destination write mask (GD5430/36/40)
		// TODO
		break;
	case 0x30:  // BLT Mode
		m_blt_mode = data;
		break;
	case 0x31:  // BitBLT Start / Status
		m_blt_status = data & ~0xf2;
		if(data & 0x02)
		{
			if(m_blt_mode & 0x04)  // blit source is system memory
				start_system_bitblt();
			else
				start_bitblt();
		}
		break;
	case 0x32:  // BitBLT ROP mode
		m_blt_rop = data;
		break;
	case 0x34:  // BitBLT Transparent Colour
		m_blt_trans_colour = (m_blt_trans_colour & 0xff00) | data;
		break;
	case 0x35:
		m_blt_trans_colour = (m_blt_trans_colour & 0x00ff) | (data << 8);
		break;
	case 0x36:  // BitBLT Transparent Colour Mask
		m_blt_trans_colour_mask = (m_blt_trans_colour_mask & 0xff00) | data;
		break;
	case 0x37:
		m_blt_trans_colour_mask = (m_blt_trans_colour_mask & 0x00ff) | (data << 8);
		break;
	default:
		gc_reg_write(index,data);
	}
}

uint8_t cirrus_gd5428_device::port_03c0_r(offs_t offset)
{
	uint8_t res = 0xff;

	switch(offset)
	{
		case 0x05:
			res = cirrus_seq_reg_read(vga.sequencer.index);
			break;
		case 0x09:
			if(!m_ext_palette_enabled)
				res = vga_device::port_03c0_r(offset);
			else
			{
				if (vga.dac.read)
				{
					switch (vga.dac.state++)
					{
						case 0:
							res = m_ext_palette[vga.dac.read_index & 0x0f].red;
							break;
						case 1:
							res = m_ext_palette[vga.dac.read_index & 0x0f].green;
							break;
						case 2:
							res = m_ext_palette[vga.dac.read_index & 0x0f].blue;
							break;
					}

					if (vga.dac.state==3)
					{
						vga.dac.state = 0;
						vga.dac.read_index++;
					}
				}
			}
			break;
		case 0x0f:
			res = cirrus_gc_reg_read(vga.gc.index);
			break;
		default:
			res = vga_device::port_03c0_r(offset);
			break;
	}

	return res;
}

void cirrus_gd5428_device::port_03c0_w(offs_t offset, uint8_t data)
{
	switch(offset)
	{
		case 0x05:
			cirrus_seq_reg_write(vga.sequencer.index,data);
			break;
		case 0x09:
			if(!m_ext_palette_enabled)
				vga_device::port_03c0_w(offset,data);
			else
			{
				if (!vga.dac.read)
				{
					switch (vga.dac.state++) {
					case 0:
						m_ext_palette[vga.dac.write_index & 0x0f].red=data;
						break;
					case 1:
						m_ext_palette[vga.dac.write_index & 0x0f].green=data;
						break;
					case 2:
						m_ext_palette[vga.dac.write_index & 0x0f].blue=data;
						break;
					}
					vga.dac.dirty=1;
					if (vga.dac.state==3)
					{
						vga.dac.state=0;
						vga.dac.write_index++;
					}
				}
			}
			break;
		case 0x0f:
			cirrus_gc_reg_write(vga.gc.index,data);
			break;
		default:
			vga_device::port_03c0_w(offset,data);
			break;
	}
	cirrus_define_video_mode();
}

uint8_t cirrus_gd5428_device::port_03b0_r(offs_t offset)
{
	uint8_t res = 0xff;

	if (CRTC_PORT_ADDR == 0x3b0)
	{
		switch(offset)
		{
			case 5:
				res = cirrus_crtc_reg_read(vga.crtc.index);
				break;
			default:
				res = vga_device::port_03b0_r(offset);
				break;
		}
	}

	return res;
}

uint8_t cirrus_gd5428_device::port_03d0_r(offs_t offset)
{
	uint8_t res = 0xff;

	if (CRTC_PORT_ADDR == 0x3d0)
	{
		switch(offset)
		{
			case 5:
				res = cirrus_crtc_reg_read(vga.crtc.index);
				break;
			default:
				res = vga_device::port_03d0_r(offset);
				break;
		}
	}

	return res;
}

void cirrus_gd5428_device::port_03b0_w(offs_t offset, uint8_t data)
{
	if (CRTC_PORT_ADDR == 0x3b0)
	{
		switch(offset)
		{
			case 5:
				vga.crtc.data[vga.crtc.index] = data;
				cirrus_crtc_reg_write(vga.crtc.index,data);
				break;
			default:
				vga_device::port_03b0_w(offset,data);
				break;
		}
	}
	cirrus_define_video_mode();
}

void cirrus_gd5428_device::port_03d0_w(offs_t offset, uint8_t data)
{
	if (CRTC_PORT_ADDR == 0x3d0)
	{
		switch(offset)
		{
			case 5:
				vga.crtc.data[vga.crtc.index] = data;
				cirrus_crtc_reg_write(vga.crtc.index,data);
				break;
			default:
				vga_device::port_03d0_w(offset,data);
				break;
		}
	}
	cirrus_define_video_mode();
}

uint8_t cirrus_gd5428_device::cirrus_crtc_reg_read(uint8_t index)
{
	uint8_t res;

	switch(index)
	{
	case 0x16:  // VGA Vertical Blank end - some SVGA chipsets use all 8 bits, and this is one of them (according to MFGTST CRTC tests)
		res = vga.crtc.vert_blank_end & 0x00ff;
		break;
	case 0x19:
		res = m_cr19;
		break;
	case 0x1a:
		res = m_cr1a;
		break;
	case 0x1b:
		res = m_cr1b;
		break;
	case 0x27:
		res = m_chip_id;
		break;
	default:
		res = crtc_reg_read(index);
		break;
	}

	return res;
}

void cirrus_gd5428_device::cirrus_crtc_reg_write(uint8_t index, uint8_t data)
{
	if(LOG_REG) logerror("CL: CRTC write %02x to CR%02x\n",data,index);
	switch(index)
	{
	case 0x16:  // VGA Vertical Blank end - some SVGA chipsets use all 8 bits, and this is one of them (according to MFGTST CRTC tests)
		vga.crtc.vert_blank_end &= ~0x00ff;
		vga.crtc.vert_blank_end |= data;
		break;
	case 0x19:
		m_cr19 = data;
		break;
	case 0x1a:
		m_cr1a = data;
		vga.crtc.horz_blank_end = (vga.crtc.horz_blank_end & 0xff3f) | ((data & 0x30) << 2);
		vga.crtc.vert_blank_end = (vga.crtc.vert_blank_end & 0xfcff) | ((data & 0xc0) << 2);
		break;
	case 0x1b:
		m_cr1b = data;
		vga.crtc.start_addr_latch &= ~0x070000;
		vga.crtc.start_addr_latch |= ((data & 0x01) << 16);
		vga.crtc.start_addr_latch |= ((data & 0x0c) << 15);
		vga.crtc.offset = (vga.crtc.offset & 0x00ff) | ((data & 0x10) << 4);
		cirrus_define_video_mode();
		break;
	case 0x1d:
		//vga.crtc.start_addr_latch = (vga.crtc.start_addr_latch & 0xf7ffff) | ((data & 0x01) << 16);  // GD543x
		break;
	case 0x27:
		// Do nothing, read only
		break;
	default:
		crtc_reg_write(index,data);
		break;
	}

}

inline uint8_t cirrus_gd5428_device::cirrus_vga_latch_write(int offs, uint8_t data)
{
	uint8_t res = 0;
	uint8_t mode_mask = (gc_mode_ext & 0x04) ? 0x07 : 0x03;

	switch (vga.gc.write_mode & mode_mask) {
	case 0:
		data = rotate_right(data);
		if(vga.gc.enable_set_reset & 1<<offs)
			res = vga_logical_op((vga.gc.set_reset & 1<<offs) ? vga.gc.bit_mask : 0, offs,vga.gc.bit_mask);
		else
			res = vga_logical_op(data, offs, vga.gc.bit_mask);
		break;
	case 1:
		res = vga.gc.latch[offs];
		break;
	case 2:
		res = vga_logical_op((data & 1<<offs) ? 0xff : 0x00,offs,vga.gc.bit_mask);
		break;
	case 3:
		data = rotate_right(data);
		res = vga_logical_op((vga.gc.set_reset & 1<<offs) ? 0xff : 0x00,offs,data&vga.gc.bit_mask);
		break;
	case 4:
		res = vga.gc.latch[offs];
		popmessage("CL: Unimplemented VGA write mode 4 enabled");
		break;
	case 5:
		res = vga.gc.latch[offs];
		popmessage("CL: Unimplemented VGA write mode 5 enabled");
		break;
	}

	return res;
}

uint8_t cirrus_gd5428_device::mem_r(offs_t offset)
{
	uint32_t addr;
	uint8_t bank;
	uint8_t cur_mode = pc_vga_choosevideomode();

	if(gc_locked || offset >= 0x10000 || cur_mode == TEXT_MODE || cur_mode == SCREEN_OFF)
		return vga_device::mem_r(offset);

	if(offset >= 0x8000 && offset < 0x10000 && (gc_mode_ext & 0x01)) // if accessing bank 1 (if enabled)
		bank = gc_bank_1;
	else
		bank = gc_bank_0;

	if(gc_mode_ext & 0x20)  // 16kB bank granularity
		addr = bank * 0x4000;
	else  // 4kB bank granularity
		addr = bank * 0x1000;

	// Is the display address adjusted automatically when not using Chain-4 addressing?  The GD542x BIOS doesn't do it, but Virtual Pool expects it.
	if(!(vga.sequencer.data[4] & 0x8))
		addr <<= 2;

	if(svga.rgb8_en || svga.rgb15_en || svga.rgb16_en || svga.rgb24_en)
	{
		uint8_t data = 0;
		if(gc_mode_ext & 0x01)
		{
			if(offset & 0x10000)
				return 0;
			if(offset < 0x8000)
				offset &= 0x7fff;
			else
			{
				offset -= 0x8000;
				offset &= 0x7fff;
			}
		}
		else
			offset &= 0xffff;

		if(vga.sequencer.data[4] & 0x8)
			data = vga.memory[(offset+addr) % vga.svga_intf.vram_size];
		else
		{
			{
				int i;

				for(i=0;i<4;i++)
				{
					if(vga.sequencer.map_mask & 1 << i)
						data |= vga.memory[((offset*4+i)+addr) % vga.svga_intf.vram_size];
				}
			}
		return data;
		}
	}

	switch(vga.gc.memory_map_sel & 0x03)
	{
		case 0: break;
		case 1: if(gc_mode_ext & 0x01) offset &= 0x7fff; else offset &= 0x0ffff; break;
		case 2: offset -= 0x10000; offset &= 0x07fff; break;
		case 3: offset -= 0x18000; offset &= 0x07fff; break;
	}

	if(vga.sequencer.data[4] & 4)
	{
		int data;
		if (!machine().side_effects_disabled())
		{
			vga.gc.latch[0]=vga.memory[(offset+addr) % vga.svga_intf.vram_size];
			vga.gc.latch[1]=vga.memory[((offset+addr)+0x10000) % vga.svga_intf.vram_size];
			vga.gc.latch[2]=vga.memory[((offset+addr)+0x20000) % vga.svga_intf.vram_size];
			vga.gc.latch[3]=vga.memory[((offset+addr)+0x30000) % vga.svga_intf.vram_size];
		}

		if (vga.gc.read_mode)
		{
			uint8_t byte,layer;
			uint8_t fill_latch;
			data=0;

			for(byte=0;byte<8;byte++)
			{
				fill_latch = 0;
				for(layer=0;layer<4;layer++)
				{
					if(vga.gc.latch[layer] & 1 << byte)
						fill_latch |= 1 << layer;
				}
				fill_latch &= vga.gc.color_dont_care;
				if(fill_latch == vga.gc.color_compare)
					data |= 1 << byte;
			}
		}
		else
			data=vga.gc.latch[vga.gc.read_map_sel];

		return data;
	}
	else
	{
		// TODO: Lines up in 16-colour mode, likely different for 256-colour modes (docs say video addresses are shifted right 3 places)
		uint8_t i,data;
//      uint8_t bits = ((gc_mode_ext & 0x08) && (vga.gc.write_mode == 1)) ? 8 : 4;

		data = 0;
		//printf("%08x\n",offset);

		if(gc_mode_ext & 0x02)
		{
			for(i=0;i<8;i++)
			{
				if(vga.sequencer.map_mask & 1 << i)
					data |= vga.memory[(((offset+addr))+i*0x10000) % vga.svga_intf.vram_size];
			}
		}
		else
		{
			for(i=0;i<4;i++)
			{
				if(vga.sequencer.map_mask & 1 << i)
					data |= vga.memory[(((offset+addr))+i*0x10000) % vga.svga_intf.vram_size];
			}
		}

		return data;
	}
}

void cirrus_gd5428_device::mem_w(offs_t offset, uint8_t data)
{
	uint32_t addr;
	uint8_t bank;
	uint8_t cur_mode = pc_vga_choosevideomode();

	if(m_blt_system_transfer)
	{
		if(m_blt_mode & 0x80)  // colour expand
		{
			m_blt_system_buffer &= ~(0x000000ff);
			m_blt_system_buffer |= data;
			blit_byte();
			m_blt_system_count = 0;
		}
		else
		{
			m_blt_system_buffer &= ~(0x000000ff << (m_blt_system_count * 8));
			m_blt_system_buffer |= (data << (m_blt_system_count * 8));
			m_blt_system_count++;
			if(m_blt_system_count >= 4)
			{
				blit_dword();
				m_blt_system_count = 0;
			}
		}
		return;
	}

	if(gc_locked || offset >= 0x10000 || cur_mode == TEXT_MODE || cur_mode == SCREEN_OFF)
	{
		vga_device::mem_w(offset,data);
		return;
	}

	if(offset >= 0x8000 && offset < 0x10000 && (gc_mode_ext & 0x01)) // if accessing bank 1 (if enabled)
		bank = gc_bank_1;
	else
		bank = gc_bank_0;

	if(gc_mode_ext & 0x20)  // 16kB bank granularity
		addr = bank * 0x4000;
	else  // 4kB bank granularity
		addr = bank * 0x1000;

	// Is the display address adjusted automatically when using Chain-4 addressing?  The GD542x BIOS doesn't do it, but Virtual Pool expects it.
	if(!(vga.sequencer.data[4] & 0x8))
		addr <<= 2;

	if(svga.rgb8_en || svga.rgb15_en || svga.rgb16_en || svga.rgb24_en)
	{
		if(offset & 0x10000)
			return;
		if(gc_mode_ext & 0x01)
		{
			if(offset < 0x8000)
				offset &= 0x7fff;
			else
			{
				offset -= 0x8000;
				offset &= 0x7fff;
			}
		}
		else
			offset &= 0xffff;

		// GR0 (and GR10 in 15/16bpp modes) = background colour in write mode 5
		// GR1 (and GR11 in 15/16bpp modes) = foreground colour in write modes 4 or 5
		if(vga.gc.write_mode == 4)
		{
			int i;

			for(i=0;i<8;i++)
			{
				if(svga.rgb8_en)
				{
					if(data & (0x01 << (7-i)))
						vga.memory[((addr+offset)*8+i) % vga.svga_intf.vram_size] = vga.gc.enable_set_reset;
				}
				else if(svga.rgb15_en || svga.rgb16_en)
				{
					if(data & (0x01 << (7-i)))
					{
						vga.memory[((addr+offset)*16+(i*2)) % vga.svga_intf.vram_size] = vga.gc.enable_set_reset;
						vga.memory[((addr+offset)*16+(i*2)+1) % vga.svga_intf.vram_size] = m_gr11;
					}
				}
			}
			return;
		}

		if(vga.gc.write_mode == 5)
		{
			int i;

			for(i=0;i<8;i++)
			{
				if(svga.rgb8_en)
				{
					if(data & (0x01 << (7-i)))
						vga.memory[((addr+offset)*8+i) % vga.svga_intf.vram_size] = vga.gc.enable_set_reset;
					else
						vga.memory[((addr+offset)*8+i) % vga.svga_intf.vram_size] = vga.gc.set_reset;
				}
				else if(svga.rgb15_en || svga.rgb16_en)
				{
					if(data & (0x01 << (7-i)))
					{
						vga.memory[((addr+offset)*16+(i*2)) % vga.svga_intf.vram_size] = vga.gc.enable_set_reset;
						vga.memory[((addr+offset)*16+(i*2)+1) % vga.svga_intf.vram_size] = m_gr11;
					}
					else
					{
						vga.memory[((addr+offset)*16+(i*2)) % vga.svga_intf.vram_size] = vga.gc.set_reset;
						vga.memory[((addr+offset)*16+(i*2)+1) % vga.svga_intf.vram_size] = m_gr10;
					}
				}
			}
			return;
		}

		if(vga.sequencer.data[4] & 0x8)
			vga.memory[(offset+addr) % vga.svga_intf.vram_size] = data;
		else
		{
			int i;
			for(i=0;i<4;i++)
			{
				if(vga.sequencer.map_mask & 1 << i)
					vga.memory[((offset*4+i)+addr) % vga.svga_intf.vram_size] = data;
			}
		}
	}
	else
	{
		//Inside each case must prevent writes to non-mapped VGA memory regions, not only mask the offset.
		switch(vga.gc.memory_map_sel & 0x03)
		{
			case 0: break;
			case 1:
				if(offset & 0x10000)
					return;

				if(gc_mode_ext & 0x01)
					offset &= 0x7fff;
				else
					offset &= 0xffff;
				break;
			case 2:
				if((offset & 0x18000) != 0x10000)
					return;

				offset &= 0x07fff;
				break;
			case 3:
				if((offset & 0x18000) != 0x18000)
					return;

				offset &= 0x07fff;
				break;
		}

		{
		// TODO: Lines up in 16-colour mode, likely different for 256-colour modes (docs say video addresses are shifted right 3 places)
			uint8_t i;
//          uint8_t bits = ((gc_mode_ext & 0x08) && (vga.gc.write_mode == 1)) ? 8 : 4;

			for(i=0;i<4;i++)
			{
				if(vga.sequencer.map_mask & 1 << i)
				{
					if(gc_mode_ext & 0x02)
					{
						vga.memory[(((offset+addr) << 1)+i*0x10000) % vga.svga_intf.vram_size] = (vga.sequencer.data[4] & 4) ? cirrus_vga_latch_write(i,data) : data;
						vga.memory[(((offset+addr) << 1)+i*0x10000+1) % vga.svga_intf.vram_size] = (vga.sequencer.data[4] & 4) ? cirrus_vga_latch_write(i,data) : data;
					}
					else
						vga.memory[(((offset+addr))+i*0x10000) % vga.svga_intf.vram_size] = (vga.sequencer.data[4] & 4) ? cirrus_vga_latch_write(i,data) : data;
				}
			}
			return;
		}
	}
}
