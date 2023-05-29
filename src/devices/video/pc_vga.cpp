// license:BSD-3-Clause
// copyright-holders:Nathan Woods, Peter Trauner, Angelo Salese
/***************************************************************************

    Video Graphics Adapter (VGA) section

    Nathan Woods    npwoods@mess.org
    Peter Trauner   PeT mess@utanet.at

    This code takes care of installing the various VGA memory and port
    handlers

    The VGA standard is compatible with MDA, CGA, Hercules, EGA
    (mda, cga, hercules not real register compatible)
    several vga cards drive also mda, cga, ega monitors
    some vga cards have register compatible mda, cga, hercules modes

    ega/vga
    64k (early ega 16k) words of 32 bit memory

    TODO:
    - modernize
    - fix video update, still need to get that choosevideomode() out of it.
    - rewrite video drawing functions (they are horrible)
    - add per-gfx card VESA functions;
    - (and many more ...)

    per-game issues:
    - The Incredible Machine: fix partial updates
    - MAME 0.01: fix 92 Hz refresh rate bug (uses VESA register?).
    - Virtual Pool: ET4k unrecognized;
    - California Chase (calchase): various gfx bugs, CPU related?
    - Jazz Jackrabbit: status bar is very jerky, but main screen scrolling is fine?
    - Catacombs: weird resolution (untested)

    ROM declarations:

    (oti 037 chip)
    ROM_LOAD("oakvga.bin", 0xc0000, 0x8000, CRC(318c5f43) SHA1(2aeb6cf737fd87dfd08c9f5b5bc421fcdbab4ce9) )

***************************************************************************/

#include "emu.h"
#include "pc_vga.h"

#include "screen.h"

#define LOG_WARN      (1U << 1)
#define LOG_REGS      (1U << 2) // deprecated
#define LOG_DSW       (1U << 3) // Input sense at $3c2

#define VERBOSE (LOG_GENERAL | LOG_WARN | LOG_DSW)
//#define LOG_OUTPUT_FUNC osd_printf_info
#include "logmacro.h"

#define LOGWARN(...)           LOGMASKED(LOG_WARN, __VA_ARGS__)
#define LOGREGS(...)           LOGMASKED(LOG_REGS, __VA_ARGS__)
#define LOGDSW(...)            LOGMASKED(LOG_DSW, __VA_ARGS__)


/***************************************************************************

    Local variables

***************************************************************************/

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

#define VGA_CH_WIDTH ((vga.sequencer.data[1]&1)?8:9)

#define TEXT_COLUMNS (vga.crtc.horz_disp_end+1)
#define TEXT_START_ADDRESS (vga.crtc.start_addr<<3)
#define TEXT_LINE_LENGTH (vga.crtc.offset<<1)

#define TEXT_COPY_9COLUMN(ch) (((ch & 0xe0) == 0xc0)&&(vga.attribute.data[0x10]&4))

// Special values for SVGA Trident - Mode Vesa 110h
#define TLINES (LINES)
#define TGA_COLUMNS (EGA_COLUMNS)
#define TGA_START_ADDRESS (vga.crtc.start_addr<<2)
#define TGA_LINE_LENGTH (vga.crtc.offset<<3)


/***************************************************************************

    Generic VGA

***************************************************************************/
// device type definition
DEFINE_DEVICE_TYPE(VGA,        vga_device,        "vga",        "VGA")

vga_device::vga_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, type, tag, owner, clock)
	, device_video_interface(mconfig, *this)
	, device_palette_interface(mconfig, *this)
	, vga(*this)
	, m_input_sense(*this, "VGA_SENSE")
{
}

vga_device::vga_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: vga_device(mconfig, VGA, tag, owner, clock)
{
}

svga_device::svga_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: vga_device(mconfig, type, tag, owner, clock)
{
}

// zero everything, keep vtbls
void vga_device::zero()
{
	vga.svga_intf.seq_regcount = 0;
	vga.svga_intf.crtc_regcount = 0;
	memset(vga.pens, 0, sizeof(vga.pens));
	vga.miscellaneous_output = 0;
	vga.feature_control = 0;
	memset(&vga.sequencer, 0, sizeof(vga.sequencer));
	memset(&vga.crtc, 0, sizeof(vga.crtc));
	memset(&vga.gc, 0, sizeof(vga.gc));
	memset(&vga.attribute, 0, sizeof(vga.attribute));
	memset(&vga.dac, 0, sizeof(vga.dac));
	memset(&vga.oak, 0, sizeof(vga.oak));
}

void svga_device::zero()
{
	vga_device::zero();
	memset(&svga, 0, sizeof(svga));
}

/* VBLANK callback, start address definitely updates AT vblank, not before. */
TIMER_CALLBACK_MEMBER(vga_device::vblank_timer_cb)
{
	vga.crtc.start_addr = vga.crtc.start_addr_latch;
	vga.attribute.pel_shift = vga.attribute.pel_shift_latch;
	m_vblank_timer->adjust( screen().time_until_pos(vga.crtc.vert_blank_start + vga.crtc.vert_blank_end) );
}

void vga_device::device_start()
{
	zero();

	for (int i = 0; i < 0x100; i++)
		set_pen_color(i, 0, 0, 0);

	// Avoid an infinite loop when displaying.  0 is not possible anyway.
	vga.crtc.maximum_scan_line = 1;


	// copy over interfaces
	vga.svga_intf.seq_regcount = 0x05;
	vga.svga_intf.crtc_regcount = 0x19;

	vga.memory = std::make_unique<uint8_t []>(vga.svga_intf.vram_size);
	memset(&vga.memory[0], 0, vga.svga_intf.vram_size);
	save_pointer(NAME(vga.memory), vga.svga_intf.vram_size);
	save_item(NAME(vga.pens));

	save_item(NAME(vga.miscellaneous_output));
	save_item(NAME(vga.feature_control));

	save_item(NAME(vga.sequencer.index));
	save_item(NAME(vga.sequencer.data));
	save_item(NAME(vga.sequencer.map_mask));
	save_item(NAME(vga.sequencer.char_sel.A));
	save_item(NAME(vga.sequencer.char_sel.B));

	save_item(NAME(vga.crtc.index));
	save_item(NAME(vga.crtc.data));
	save_item(NAME(vga.crtc.horz_total));
	save_item(NAME(vga.crtc.horz_disp_end));
	save_item(NAME(vga.crtc.horz_blank_start));
	save_item(NAME(vga.crtc.horz_retrace_start));
	save_item(NAME(vga.crtc.horz_retrace_skew));
	save_item(NAME(vga.crtc.horz_retrace_end));
	save_item(NAME(vga.crtc.disp_enable_skew));
	save_item(NAME(vga.crtc.evra));
	save_item(NAME(vga.crtc.vert_total));
	save_item(NAME(vga.crtc.vert_disp_end));
	save_item(NAME(vga.crtc.vert_retrace_start));
	save_item(NAME(vga.crtc.vert_retrace_end));
	save_item(NAME(vga.crtc.vert_blank_start));
	save_item(NAME(vga.crtc.line_compare));
	save_item(NAME(vga.crtc.cursor_addr));
	save_item(NAME(vga.crtc.byte_panning));
	save_item(NAME(vga.crtc.preset_row_scan));
	save_item(NAME(vga.crtc.scan_doubling));
	save_item(NAME(vga.crtc.maximum_scan_line));
	save_item(NAME(vga.crtc.cursor_enable));
	save_item(NAME(vga.crtc.cursor_scan_start));
	save_item(NAME(vga.crtc.cursor_skew));
	save_item(NAME(vga.crtc.cursor_scan_end));
	save_item(NAME(vga.crtc.start_addr));
	save_item(NAME(vga.crtc.start_addr_latch));
	save_item(NAME(vga.crtc.protect_enable));
	save_item(NAME(vga.crtc.bandwidth));
	save_item(NAME(vga.crtc.offset));
	save_item(NAME(vga.crtc.word_mode));
	save_item(NAME(vga.crtc.dw));
	save_item(NAME(vga.crtc.div4));
	save_item(NAME(vga.crtc.underline_loc));
	save_item(NAME(vga.crtc.vert_blank_end));
	save_item(NAME(vga.crtc.sync_en));
	save_item(NAME(vga.crtc.aw));
	save_item(NAME(vga.crtc.div2));
	save_item(NAME(vga.crtc.sldiv));
	save_item(NAME(vga.crtc.map14));
	save_item(NAME(vga.crtc.map13));
	save_item(NAME(vga.crtc.irq_clear));
	save_item(NAME(vga.crtc.irq_disable));
	save_item(NAME(vga.crtc.no_wrap));

	save_item(NAME(vga.gc.index));
	save_item(NAME(vga.gc.latch));
	save_item(NAME(vga.gc.set_reset));
	save_item(NAME(vga.gc.enable_set_reset));
	save_item(NAME(vga.gc.color_compare));
	save_item(NAME(vga.gc.logical_op));
	save_item(NAME(vga.gc.rotate_count));
	save_item(NAME(vga.gc.shift256));
	save_item(NAME(vga.gc.shift_reg));
	save_item(NAME(vga.gc.read_map_sel));
	save_item(NAME(vga.gc.read_mode));
	save_item(NAME(vga.gc.write_mode));
	save_item(NAME(vga.gc.color_dont_care));
	save_item(NAME(vga.gc.bit_mask));
	save_item(NAME(vga.gc.alpha_dis));
	save_item(NAME(vga.gc.memory_map_sel));
	save_item(NAME(vga.gc.host_oe));
	save_item(NAME(vga.gc.chain_oe));

	save_item(NAME(vga.attribute.index));
	save_item(NAME(vga.attribute.data));
	save_item(NAME(vga.attribute.state));
	save_item(NAME(vga.attribute.prot_bit));
	save_item(NAME(vga.attribute.pel_shift));
	save_item(NAME(vga.attribute.pel_shift_latch));

	save_item(NAME(vga.dac.read_index));
	save_item(NAME(vga.dac.write_index));
	save_item(NAME(vga.dac.mask));
	save_item(NAME(vga.dac.read));
	save_item(NAME(vga.dac.state));
	save_item(NAME(vga.dac.color));
	save_item(NAME(vga.dac.dirty));

	m_vblank_timer = timer_alloc(FUNC(vga_device::vblank_timer_cb), this);
}

void svga_device::device_start()
{
	vga_device::device_start();
	memset(&svga, 0, sizeof(svga));

	save_item(NAME(svga.bank_r));
	save_item(NAME(svga.bank_w));
	save_item(NAME(svga.rgb8_en));
	save_item(NAME(svga.rgb15_en));
	save_item(NAME(svga.rgb16_en));
	save_item(NAME(svga.rgb24_en));
	save_item(NAME(svga.rgb32_en));
	save_item(NAME(svga.id));
}

uint16_t vga_device::offset()
{
//  popmessage("Offset: %04x  %s %s **",vga.crtc.offset,vga.crtc.dw?"DW":"--",vga.crtc.word_mode?"BYTE":"WORD");
	if(vga.crtc.dw)
		return vga.crtc.offset << 3;
	if(vga.crtc.word_mode)
		return vga.crtc.offset << 1;
	else
		return vga.crtc.offset << 2;
}

uint32_t vga_device::start_addr()
{
//  popmessage("Offset: %04x  %s %s **",vga.crtc.offset,vga.crtc.dw?"DW":"--",vga.crtc.word_mode?"BYTE":"WORD");
	if(vga.crtc.dw)
		return vga.crtc.start_addr << 2;
	if(vga.crtc.word_mode)
		return vga.crtc.start_addr << 0;
	else
		return vga.crtc.start_addr << 1;
}

void vga_device::vga_vh_text(bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	int width=VGA_CH_WIDTH, height = (vga.crtc.maximum_scan_line) * (vga.crtc.scan_doubling + 1);

	if(vga.crtc.cursor_enable)
		vga.cursor.visible = screen().frame_number() & 0x10;
	else
		vga.cursor.visible = 0;

	for (int addr = vga.crtc.start_addr, line = -vga.crtc.preset_row_scan; line < TEXT_LINES;
			line += height, addr += (offset()>>1))
	{
		for (int pos = addr, column=0; column<TEXT_COLUMNS; column++, pos++)
		{
			uint8_t ch   = vga.memory[(pos<<1) + 0];
			uint8_t attr = vga.memory[(pos<<1) + 1];
			uint32_t font_base = 0x20000+(ch<<5);
			font_base += ((attr & 8) ? vga.sequencer.char_sel.A : vga.sequencer.char_sel.B)*0x2000;
			uint8_t blink_en = (vga.attribute.data[0x10]&8&&screen().frame_number() & 0x20) ? attr & 0x80 : 0;

			uint8_t fore_col = attr & 0xf;
			uint8_t back_col = (attr & 0x70) >> 4;
			back_col |= (vga.attribute.data[0x10]&8) ? 0 : ((attr & 0x80) >> 4);

			for (int h = std::max(-line, 0); (h < height) && (line+h < std::min(TEXT_LINES, bitmap.height())); h++)
			{
				uint32_t *const bitmapline = &bitmap.pix(line+h);
				uint8_t bits = vga.memory[font_base+(h>>(vga.crtc.scan_doubling))];

				int mask, w;
				for (mask=0x80, w=0; (w<width)&&(w<8); w++, mask>>=1)
				{
					pen_t pen;
					if (bits&mask)
						pen = vga.pens[blink_en ? back_col : fore_col];
					else
						pen = vga.pens[back_col];

					if(!screen().visible_area().contains(column*width+w, line+h))
						continue;
					bitmapline[column*width+w] = pen;

				}
				if (w<width)
				{
					/* 9 column */
					pen_t pen;
					if (TEXT_COPY_9COLUMN(ch)&&(bits&1))
						pen = vga.pens[blink_en ? back_col : fore_col];
					else
						pen = vga.pens[back_col];

					if(!screen().visible_area().contains(column*width+w, line+h))
						continue;
					bitmapline[column*width+w] = pen;
				}
			}
			if (vga.cursor.visible&&(pos==vga.crtc.cursor_addr))
			{
				for (int h=vga.crtc.cursor_scan_start;
						(h<=vga.crtc.cursor_scan_end)&&(h<height)&&(line+h<TEXT_LINES);
						h++)
				{
					if(!screen().visible_area().contains(column*width, line+h))
						continue;
					bitmap.plot_box(column*width, line+h, width, 1, vga.pens[attr&0xf]);
				}
			}
		}
	}
}

void vga_device::vga_vh_ega(bitmap_rgb32 &bitmap,  const rectangle &cliprect)
{
	int height = vga.crtc.maximum_scan_line * (vga.crtc.scan_doubling + 1);
	int pel_shift = (vga.attribute.pel_shift & 7);

//  popmessage("%08x %02x",EGA_START_ADDRESS,pel_shift);

	/**/
	for (int addr=EGA_START_ADDRESS, line=0; line<LINES; line += height, addr += offset())
	{
		for (int yi=0;yi<height;yi++)
		{
			uint32_t *const bitmapline = &bitmap.pix(line + yi);
			// ibm_5150:batmanmv uses this on gameplay for both EGA and "VGA" modes
			// NB: EGA mode in that game sets 663, should be 303 like the other mode
			// causing no status bar to appear. This is a known btanb in how VGA
			// handles EGA mode, cfr. https://www.os2museum.com/wp/fantasyland-on-vga/
			if((line + yi) == (vga.crtc.line_compare & 0x3ff))
				addr = 0;

			for (int pos=addr, c=0, column=0; column<EGA_COLUMNS+1; column++, c+=8, pos=(pos+1)&0xffff)
			{
				int data[4] = {
						vga.memory[(pos & 0xffff)],
						vga.memory[(pos & 0xffff)+0x10000]<<1,
						vga.memory[(pos & 0xffff)+0x20000]<<2,
						vga.memory[(pos & 0xffff)+0x30000]<<3 };

				for (int i = 7; i >= 0; i--)
				{
					pen_t pen = vga.pens[(data[0]&1) | (data[1]&2) | (data[2]&4) | (data[3]&8)];

					data[0]>>=1;
					data[1]>>=1;
					data[2]>>=1;
					data[3]>>=1;

					if(!screen().visible_area().contains(c+i-pel_shift, line + yi))
						continue;
					bitmapline[c+i-pel_shift] = pen;
				}
			}
		}
	}
}

/* TODO: I'm guessing that in 256 colors mode every pixel actually outputs two pixels. Is it right? */
void vga_device::vga_vh_vga(bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	int height = vga.crtc.maximum_scan_line * (vga.crtc.scan_doubling + 1);
	int pel_shift = (vga.attribute.pel_shift & 6);
	int addrmask = vga.crtc.no_wrap ? -1 : 0xffff;

	/* line compare is screen sensitive */
	uint16_t mask_comp = 0x3ff; //| (LINES & 0x300);

//  popmessage("%02x %02x",vga.attribute.pel_shift,vga.sequencer.data[4] & 0x08);

	int curr_addr = 0;
	if(!(vga.sequencer.data[4] & 0x08))
	{
		for (int addr = start_addr(), line=0; line<LINES; line+=height, addr+=offset(), curr_addr+=offset())
		{
			for(int yi = 0;yi < height; yi++)
			{
				if((line + yi) < (vga.crtc.line_compare & mask_comp))
					curr_addr = addr;
				if((line + yi) == (vga.crtc.line_compare & mask_comp))
				{
					curr_addr = 0;
					pel_shift = 0;
				}
				uint32_t *const bitmapline = &bitmap.pix(line + yi);
				for (int pos=curr_addr, c=0, column=0; column<VGA_COLUMNS+1; column++, c+=8, pos++)
				{
					if(pos > 0x80000/4)
						return;

					for(int xi=0;xi<8;xi++)
					{
						if (!screen().visible_area().contains(c+xi-(pel_shift), line + yi))
							continue;
						bitmapline[c+xi-(pel_shift)] = pen(vga.memory[(pos & addrmask)+((xi >> 1)*0x10000)]);
					}
				}
			}
		}
	}
	else
	{
		for (int addr = start_addr(), line=0; line<LINES; line+=height, addr+=offset(), curr_addr+=offset())
		{
			for(int yi = 0;yi < height; yi++)
			{
				if((line + yi) < (vga.crtc.line_compare & mask_comp))
					curr_addr = addr;
				if((line + yi) == (vga.crtc.line_compare & mask_comp))
					curr_addr = 0;
				uint32_t *const bitmapline = &bitmap.pix(line + yi);
				//addr %= 0x80000;
				for (int pos=curr_addr, c=0, column=0; column<VGA_COLUMNS+1; column++, c+=0x10, pos+=0x8)
				{
					if(pos + 0x08 > 0x80000)
						return;

					for (int xi=0;xi<0x10;xi++)
					{
						if(!screen().visible_area().contains(c+xi-(pel_shift), line + yi))
							continue;
						bitmapline[c+xi-pel_shift] = pen(vga.memory[(pos+(xi >> 1)) & addrmask]);
					}
				}
			}
		}
	}
}

void vga_device::vga_vh_cga(bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	int height = (vga.crtc.scan_doubling + 1);

	int width = (vga.crtc.horz_disp_end + 1) * 8;

	for(int y=0;y<LINES;y++)
	{
		uint32_t addr = ((y & 1) * 0x2000) + (((y & ~1) >> 1) * width/4);

		for(int x=0;x<width;x+=4)
		{
			for(int yi=0;yi<height;yi++)
			{
				uint32_t *const bitmapline = &bitmap.pix(y * height + yi);

				for(int xi=0;xi<4;xi++)
				{
					pen_t pen = vga.pens[(vga.memory[addr] >> (6-xi*2)) & 3];
					if(!screen().visible_area().contains(x+xi, y * height + yi))
						continue;
					bitmapline[x+xi] = pen;
				}
			}

			addr++;
		}
	}
}

void vga_device::vga_vh_mono(bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	int height = (vga.crtc.scan_doubling + 1);

	int width = (vga.crtc.horz_disp_end + 1) * 8;

	for(int y=0;y<LINES;y++)
	{
		uint32_t addr = ((y & 1) * 0x2000) + (((y & ~1) >> 1) * width/8);

		for(int x=0;x<width;x+=8)
		{
			for(int yi=0;yi<height;yi++)
			{
				uint32_t *const bitmapline = &bitmap.pix(y * height + yi);

				for(int xi=0;xi<8;xi++)
				{
					pen_t pen = vga.pens[(vga.memory[addr] >> (7-xi)) & 1];
					if(!screen().visible_area().contains(x+xi, y * height + yi))
						continue;
					bitmapline[x+xi] = pen;
				}
			}

			addr++;
		}
	}
}

void svga_device::svga_vh_rgb8(bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	int height = vga.crtc.maximum_scan_line * (vga.crtc.scan_doubling + 1);

	/* line compare is screen sensitive */
	uint16_t  mask_comp = 0x3ff;
	int curr_addr = 0;
//  uint16_t line_length;
//  if(vga.crtc.dw)
//      line_length = vga.crtc.offset << 3;  // doubleword mode
//  else
//  {
//      line_length = vga.crtc.offset << 4;
//  }

	uint8_t start_shift = (!(vga.sequencer.data[4] & 0x08) || svga.ignore_chain4) ? 2 : 0;
	for (int addr = VGA_START_ADDRESS << start_shift, line=0; line<LINES; line+=height, addr+=offset(), curr_addr+=offset())
	{
		for (int yi = 0;yi < height; yi++)
		{
			if((line + yi) < (vga.crtc.line_compare & mask_comp))
				curr_addr = addr;
			if((line + yi) == (vga.crtc.line_compare & mask_comp))
				curr_addr = 0;
			uint32_t *const bitmapline = &bitmap.pix(line + yi);
			addr %= vga.svga_intf.vram_size;
			for (int pos=curr_addr, c=0, column=0; column<VGA_COLUMNS; column++, c+=8, pos+=0x8)
			{
				if(pos + 0x08 >= vga.svga_intf.vram_size)
					return;

				for (int xi=0;xi<8;xi++)
				{
					if(!screen().visible_area().contains(c+xi, line + yi))
						continue;
					bitmapline[c+xi] = pen(vga.memory[(pos+(xi))]);
				}
			}
		}
	}
}

void svga_device::svga_vh_rgb15(bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	#define MV(x) (vga.memory[x]+(vga.memory[x+1]<<8))
	constexpr uint32_t IV = 0xff000000;
	int height = vga.crtc.maximum_scan_line * (vga.crtc.scan_doubling + 1);

	/* line compare is screen sensitive */
//  uint16_t mask_comp = 0xff | (TLINES & 0x300);
	int curr_addr = 0;
	int yi=0;
	for (int addr = TGA_START_ADDRESS, line=0; line<TLINES; line+=height, addr+=offset(), curr_addr+=offset())
	{
		uint32_t *const bitmapline = &bitmap.pix(line);
		addr %= vga.svga_intf.vram_size;
		for (int pos=addr, c=0, column=0; column<TGA_COLUMNS; column++, c+=8, pos+=0x10)
		{
			if(pos + 0x10 >= vga.svga_intf.vram_size)
				return;
			for(int xi=0,xm=0;xi<8;xi++,xm+=2)
			{
				if(!screen().visible_area().contains(c+xi, line + yi))
					continue;

				int r = (MV(pos+xm)&0x7c00)>>10;
				int g = (MV(pos+xm)&0x03e0)>>5;
				int b = (MV(pos+xm)&0x001f)>>0;
				r = (r << 3) | (r & 0x7);
				g = (g << 3) | (g & 0x7);
				b = (b << 3) | (b & 0x7);
				bitmapline[c+xi] = IV|(r<<16)|(g<<8)|(b<<0);
			}
		}
	}
}

void svga_device::svga_vh_rgb16(bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	#define MV(x) (vga.memory[x]+(vga.memory[x+1]<<8))
	constexpr uint32_t IV = 0xff000000;
	int height = vga.crtc.maximum_scan_line * (vga.crtc.scan_doubling + 1);

	/* line compare is screen sensitive */
//  uint16_t mask_comp = 0xff | (TLINES & 0x300);
	int curr_addr = 0;
	int yi=0;
	for (int addr = TGA_START_ADDRESS, line=0; line<TLINES; line+=height, addr+=offset(), curr_addr+=offset())
	{
		uint32_t *const bitmapline = &bitmap.pix(line);
		addr %= vga.svga_intf.vram_size;
		for (int pos=addr, c=0, column=0; column<TGA_COLUMNS; column++, c+=8, pos+=0x10)
		{
			if(pos + 0x10 >= vga.svga_intf.vram_size)
				return;
			for (int xi=0,xm=0;xi<8;xi++,xm+=2)
			{
				if(!screen().visible_area().contains(c+xi, line + yi))
					continue;

				int r = (MV(pos+xm)&0xf800)>>11;
				int g = (MV(pos+xm)&0x07e0)>>5;
				int b = (MV(pos+xm)&0x001f)>>0;
				r = (r << 3) | (r & 0x7);
				g = (g << 2) | (g & 0x3);
				b = (b << 3) | (b & 0x7);
				bitmapline[c+xi] = IV|(r<<16)|(g<<8)|(b<<0);
			}
		}
	}
}

void svga_device::svga_vh_rgb24(bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	#define MD(x) (vga.memory[x]+(vga.memory[x+1]<<8)+(vga.memory[x+2]<<16))
	constexpr uint32_t ID = 0xff000000;
	int height = vga.crtc.maximum_scan_line * (vga.crtc.scan_doubling + 1);

	/* line compare is screen sensitive */
//  uint16_t mask_comp = 0xff | (TLINES & 0x300);
	int curr_addr = 0;
	int yi=0;
	for (int addr = TGA_START_ADDRESS<<1, line=0; line<TLINES; line+=height, addr+=offset(), curr_addr+=offset())
	{
		uint32_t *const bitmapline = &bitmap.pix(line);
		addr %= vga.svga_intf.vram_size;
		for (int pos=addr, c=0, column=0; column<TGA_COLUMNS; column++, c+=8, pos+=24)
		{
			if(pos + 24 >= vga.svga_intf.vram_size)
				return;
			for (int xi=0,xm=0;xi<8;xi++,xm+=3)
			{
				if(!screen().visible_area().contains(c+xi, line + yi))
					continue;

				int r = (MD(pos+xm)&0xff0000)>>16;
				int g = (MD(pos+xm)&0x00ff00)>>8;
				int b = (MD(pos+xm)&0x0000ff)>>0;
				bitmapline[c+xi] = ID|(r<<16)|(g<<8)|(b<<0);
			}
		}
	}
}

void svga_device::svga_vh_rgb32(bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	#define MD(x) (vga.memory[x]+(vga.memory[x+1]<<8)+(vga.memory[x+2]<<16))
	constexpr uint32_t ID = 0xff000000;
	int height = vga.crtc.maximum_scan_line * (vga.crtc.scan_doubling + 1);

//  uint16_t mask_comp;

	/* line compare is screen sensitive */
//  mask_comp = 0xff | (TLINES & 0x300);
	int curr_addr = 0;
	int yi=0;
	for (int addr = TGA_START_ADDRESS, line=0; line<TLINES; line+=height, addr+=(offset()), curr_addr+=(offset()))
	{
		uint32_t *const bitmapline = &bitmap.pix(line);
		addr %= vga.svga_intf.vram_size;
		for (int pos=addr, c=0, column=0; column<TGA_COLUMNS; column++, c+=8, pos+=0x20)
		{
			if(pos + 0x20 >= vga.svga_intf.vram_size)
				return;
			for (int xi=0,xm=0;xi<8;xi++,xm+=4)
			{
				if(!screen().visible_area().contains(c+xi, line + yi))
					continue;

				int r = (MD(pos+xm)&0xff0000)>>16;
				int g = (MD(pos+xm)&0x00ff00)>>8;
				int b = (MD(pos+xm)&0x0000ff)>>0;
				bitmapline[c+xi] = ID|(r<<16)|(g<<8)|(b<<0);
			}
		}
	}
}

uint8_t vga_device::pc_vga_choosevideomode()
{
	if (vga.crtc.sync_en)
	{
		if (vga.dac.dirty)
		{
			for (int i=0; i<256;i++)
			{
				/* TODO: color shifters? */
				set_pen_color(i, (vga.dac.color[3*(i & vga.dac.mask)] & 0x3f) << 2,
										(vga.dac.color[3*(i & vga.dac.mask) + 1] & 0x3f) << 2,
										(vga.dac.color[3*(i & vga.dac.mask) + 2] & 0x3f) << 2);
			}
			vga.dac.dirty = 0;
		}

		if (vga.attribute.data[0x10] & 0x80)
		{
			for (int i=0; i<16;i++)
			{
				vga.pens[i] = pen((vga.attribute.data[i]&0x0f)
											|((vga.attribute.data[0x14]&0xf)<<4));
			}
		}
		else
		{
			for (int i=0; i<16;i++)
			{
				vga.pens[i] = pen((vga.attribute.data[i]&0x3f)
											|((vga.attribute.data[0x14]&0xc)<<4));
			}
		}

		if (!GRAPHIC_MODE)
		{
			return TEXT_MODE;
		}
		else if (vga.gc.shift256)
		{
			return VGA_MODE;
		}
		else if (vga.gc.shift_reg)
		{
			return CGA_MODE;
		}
		else if (vga.gc.memory_map_sel == 0x03)
		{
			return MONO_MODE;
		}
		else
		{
			return EGA_MODE;
		}
	}

	return SCREEN_OFF;
}


uint8_t svga_device::pc_vga_choosevideomode()
{
	if (vga.crtc.sync_en)
	{
		if (vga.dac.dirty)
		{
			for (int i=0; i<256;i++)
			{
				/* TODO: color shifters? */
				set_pen_color(i, (vga.dac.color[3*(i & vga.dac.mask)] & 0x3f) << 2,
										(vga.dac.color[3*(i & vga.dac.mask) + 1] & 0x3f) << 2,
										(vga.dac.color[3*(i & vga.dac.mask) + 2] & 0x3f) << 2);
			}
			vga.dac.dirty = 0;
		}

		if (vga.attribute.data[0x10] & 0x80)
		{
			for (int i=0; i<16;i++)
			{
				vga.pens[i] = pen((vga.attribute.data[i]&0x0f)
											|((vga.attribute.data[0x14]&0xf)<<4));
			}
		}
		else
		{
			for (int i=0; i<16;i++)
			{
				vga.pens[i] = pen((vga.attribute.data[i]&0x3f)
											|((vga.attribute.data[0x14]&0xc)<<4));
			}
		}

		if (svga.rgb32_en)
		{
			return RGB32_MODE;
		}
		else if (svga.rgb24_en)
		{
			return RGB24_MODE;
		}
		else if (svga.rgb16_en)
		{
			return RGB16_MODE;
		}
		else if (svga.rgb15_en)
		{
			return RGB15_MODE;
		}
		else if (svga.rgb8_en)
		{
			return RGB8_MODE;
		}
		else if (!GRAPHIC_MODE)
		{
			return TEXT_MODE;
		}
		else if (vga.gc.shift256)
		{
			return VGA_MODE;
		}
		else if (vga.gc.shift_reg)
		{
			return CGA_MODE;
		}
		else if (vga.gc.memory_map_sel == 0x03)
		{
			return MONO_MODE;
		}
		else
		{
			return EGA_MODE;
		}
	}

	return SCREEN_OFF;
}

uint8_t svga_device::get_video_depth()
{
	switch(pc_vga_choosevideomode())
	{
		case VGA_MODE:
		case RGB8_MODE:
			return 8;
		case RGB15_MODE:
		case RGB16_MODE:
			return 16;
		case RGB24_MODE:
		case RGB32_MODE:
			return 32;
	}
	return 0;
}

uint32_t vga_device::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	uint8_t cur_mode = pc_vga_choosevideomode();

	switch(cur_mode)
	{
		case SCREEN_OFF:   bitmap.fill  (black_pen(), cliprect);break;
		case TEXT_MODE:    vga_vh_text  (bitmap, cliprect); break;
		case VGA_MODE:     vga_vh_vga   (bitmap, cliprect); break;
		case EGA_MODE:     vga_vh_ega   (bitmap, cliprect); break;
		case CGA_MODE:     vga_vh_cga   (bitmap, cliprect); break;
		case MONO_MODE:    vga_vh_mono  (bitmap, cliprect); break;
	}

	return 0;
}

uint32_t svga_device::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	uint8_t cur_mode = pc_vga_choosevideomode();

	switch(cur_mode)
	{
		case SCREEN_OFF:   bitmap.fill  (black_pen(), cliprect);break;
		case TEXT_MODE:    vga_vh_text  (bitmap, cliprect); break;
		case VGA_MODE:     vga_vh_vga   (bitmap, cliprect); break;
		case EGA_MODE:     vga_vh_ega   (bitmap, cliprect); break;
		case CGA_MODE:     vga_vh_cga   (bitmap, cliprect); break;
		case MONO_MODE:    vga_vh_mono  (bitmap, cliprect); break;
		case RGB8_MODE:    svga_vh_rgb8 (bitmap, cliprect); break;
		case RGB15_MODE:   svga_vh_rgb15(bitmap, cliprect); break;
		case RGB16_MODE:   svga_vh_rgb16(bitmap, cliprect); break;
		case RGB24_MODE:   svga_vh_rgb24(bitmap, cliprect); break;
		case RGB32_MODE:   svga_vh_rgb32(bitmap, cliprect); break;
	}

	return 0;
}

/***************************************************************************/

uint8_t vga_device::vga_latch_write(int offs, uint8_t data)
{
	uint8_t res = 0;

	switch (vga.gc.write_mode & 3) {
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
	}

	return res;
}

uint8_t vga_device::crtc_reg_read(uint8_t index)
{
	uint8_t res;

	res = 0xff;

	switch(index)
	{
		case 0x00:
			res  = vga.crtc.horz_total & 0xff;
			break;
		case 0x01:
			res  = vga.crtc.horz_disp_end & 0xff;
			break;
		case 0x02:
			res  = vga.crtc.horz_blank_start & 0xff;
			break;
		case 0x03:
			res  = vga.crtc.horz_blank_end & 0x1f;
			res |= (vga.crtc.disp_enable_skew & 3) << 5;
			res |= (vga.crtc.evra & 1) << 7;
			break;
		case 0x04:
			res  = vga.crtc.horz_retrace_start & 0xff;
			break;
		case 0x05:
			res  = (vga.crtc.horz_blank_end & 0x20) << 2;
			res |= (vga.crtc.horz_retrace_skew & 3) << 5;
			res |= (vga.crtc.horz_retrace_end & 0x1f);
			break;
		case 0x06:
			res  = vga.crtc.vert_total & 0xff;
			break;
		case 0x07: // Overflow Register
			res  = (vga.crtc.line_compare & 0x100) >> 4;
			res |= (vga.crtc.vert_retrace_start & 0x200) >> 2;
			res |= (vga.crtc.vert_disp_end & 0x200) >> 3;
			res |= (vga.crtc.vert_total & 0x200) >> 4;
			res |= (vga.crtc.vert_blank_start & 0x100) >> 5;
			res |= (vga.crtc.vert_retrace_start & 0x100) >> 6;
			res |= (vga.crtc.vert_disp_end & 0x100) >> 7;
			res |= (vga.crtc.vert_total & 0x100) >> 8;
			break;
		case 0x08: // Preset Row Scan Register
			res  = (vga.crtc.byte_panning & 3) << 5;
			res |= (vga.crtc.preset_row_scan & 0x1f);
			break;
		case 0x09: // Maximum Scan Line Register
			res  = (vga.crtc.maximum_scan_line - 1) & 0x1f;
			res |= (vga.crtc.scan_doubling & 1) << 7;
			res |= (vga.crtc.line_compare & 0x200) >> 3;
			res |= (vga.crtc.vert_blank_start & 0x200) >> 4;
			break;
		case 0x0a:
			res  = (vga.crtc.cursor_scan_start & 0x1f);
			res |= ((vga.crtc.cursor_enable & 1) ^ 1) << 5;
			break;
		case 0x0b:
			res  = (vga.crtc.cursor_skew & 3) << 5;
			res |= (vga.crtc.cursor_scan_end & 0x1f);
			break;
		case 0x0c:
		case 0x0d:
			res  = (vga.crtc.start_addr_latch >> ((index & 1) ^ 1)*8) & 0xff;
			break;
		case 0x0e:
		case 0x0f:
			res  = (vga.crtc.cursor_addr >> ((index & 1) ^ 1)*8) & 0xff;
			break;
		case 0x10:
			res  = vga.crtc.vert_retrace_start & 0xff;
			break;
		case 0x11:
			res  = (vga.crtc.protect_enable & 1) << 7;
			res |= (vga.crtc.bandwidth & 1) << 6;
			res |= (vga.crtc.vert_retrace_end & 0xf);
			res |= (vga.crtc.irq_clear & 1)  << 4;
			res |= (vga.crtc.irq_disable & 1) << 5;
			break;
		case 0x12:
			res  = vga.crtc.vert_disp_end & 0xff;
			break;
		case 0x13:
			res  = vga.crtc.offset & 0xff;
			break;
		case 0x14:
			res  = (vga.crtc.dw & 1) << 6;
			res |= (vga.crtc.div4 & 1) << 5;
			res |= (vga.crtc.underline_loc & 0x1f);
			break;
		case 0x15:
			res  = vga.crtc.vert_blank_start & 0xff;
			break;
		case 0x16:
			res  = vga.crtc.vert_blank_end & 0x7f;
			break;
		case 0x17:
			res  = (vga.crtc.sync_en & 1) << 7;
			res |= (vga.crtc.word_mode & 1) << 6;
			res |= (vga.crtc.aw & 1) << 5;
			res |= (vga.crtc.div2 & 1) << 3;
			res |= (vga.crtc.sldiv & 1) << 2;
			res |= (vga.crtc.map14 & 1) << 1;
			res |= (vga.crtc.map13 & 1) << 0;
			break;
		case 0x18:
			res = vga.crtc.line_compare & 0xff;
			break;
		default:
			printf("Unhandled CRTC reg r %02x\n",index);
			break;
	}
	return res;
}

void vga_device::recompute_params_clock(int divisor, int xtal)
{
	int vblank_period,hblank_period;
	attoseconds_t refresh;
	uint8_t hclock_m = (!GRAPHIC_MODE) ? VGA_CH_WIDTH : 8;
	int pixel_clock;

	/* safety check */
	if(!vga.crtc.horz_disp_end || !vga.crtc.vert_disp_end || !vga.crtc.horz_total || !vga.crtc.vert_total)
		return;

	rectangle visarea(0, ((vga.crtc.horz_disp_end + 1) * ((float)(hclock_m)/divisor))-1, 0, vga.crtc.vert_disp_end);

	vblank_period = (vga.crtc.vert_total + 2);
	hblank_period = ((vga.crtc.horz_total + 5) * ((float)(hclock_m)/divisor));

	/* TODO: 10b and 11b settings aren't known */
	pixel_clock = xtal / (((vga.sequencer.data[1]&8) >> 3) + 1);

	refresh  = HZ_TO_ATTOSECONDS(pixel_clock) * (hblank_period) * vblank_period;
	screen().configure((hblank_period), (vblank_period), visarea, refresh );
	//popmessage("%d %d\n",vga.crtc.horz_total * 8,vga.crtc.vert_total);
	m_vblank_timer->adjust( screen().time_until_pos(vga.crtc.vert_blank_start + vga.crtc.vert_blank_end) );
}

void vga_device::recompute_params()
{
	if(vga.miscellaneous_output & 8)
		LOGWARN("Warning: VGA external clock latch selected\n");
	else
		recompute_params_clock(1, ((vga.miscellaneous_output & 0xc) ? XTAL(28'636'363) : XTAL(25'174'800)).value());
}

void vga_device::crtc_reg_write(uint8_t index, uint8_t data)
{
	/* Doom does this */
//  if(vga.crtc.protect_enable && index <= 0x07)
//      printf("write to protected address %02x\n",index);
	switch(index)
	{
		case 0x00:
			if(vga.crtc.protect_enable)
				break;
			vga.crtc.horz_total = (vga.crtc.horz_total & ~0xff) | (data & 0xff);
			recompute_params();
			break;
		case 0x01:
			if(vga.crtc.protect_enable)
				break;
			vga.crtc.horz_disp_end = (data & 0xff);
			recompute_params();
			break;
		case 0x02:
			if(vga.crtc.protect_enable)
				break;
			vga.crtc.horz_blank_start = (data & 0xff);
			break;
		case 0x03:
			if(vga.crtc.protect_enable)
				break;
			vga.crtc.horz_blank_end &= ~0x1f;
			vga.crtc.horz_blank_end |= data & 0x1f;
			vga.crtc.disp_enable_skew = (data & 0x60) >> 5;
			vga.crtc.evra = (data & 0x80) >> 7;
			break;
		case 0x04:
			if(vga.crtc.protect_enable)
				break;
			vga.crtc.horz_retrace_start = data & 0xff;
			break;
		case 0x05:
			if(vga.crtc.protect_enable)
				break;
			vga.crtc.horz_blank_end &= ~0x20;
			vga.crtc.horz_blank_end |= ((data & 0x80) >> 2);
			vga.crtc.horz_retrace_skew = ((data & 0x60) >> 5);
			vga.crtc.horz_retrace_end = data & 0x1f;
			break;
		case 0x06:
			if(vga.crtc.protect_enable)
				break;
			vga.crtc.vert_total &= ~0xff;
			vga.crtc.vert_total |= data & 0xff;
			recompute_params();
			break;
		case 0x07: // Overflow Register
			vga.crtc.line_compare       &= ~0x100;
			vga.crtc.line_compare       |= ((data & 0x10) << (8-4));
			if(vga.crtc.protect_enable)
				break;
			vga.crtc.vert_total         &= ~0x300;
			vga.crtc.vert_retrace_start &= ~0x300;
			vga.crtc.vert_disp_end      &= ~0x300;
			vga.crtc.vert_blank_start   &= ~0x100;
			vga.crtc.vert_retrace_start |= ((data & 0x80) << (9-7));
			vga.crtc.vert_disp_end      |= ((data & 0x40) << (9-6));
			vga.crtc.vert_total         |= ((data & 0x20) << (9-5));
			vga.crtc.vert_blank_start   |= ((data & 0x08) << (8-3));
			vga.crtc.vert_retrace_start |= ((data & 0x04) << (8-2));
			vga.crtc.vert_disp_end      |= ((data & 0x02) << (8-1));
			vga.crtc.vert_total         |= ((data & 0x01) << (8-0));
			recompute_params();
			break;
		case 0x08: // Preset Row Scan Register
			vga.crtc.byte_panning = (data & 0x60) >> 5;
			vga.crtc.preset_row_scan = (data & 0x1f);
			break;
		case 0x09: // Maximum Scan Line Register
			vga.crtc.line_compare      &= ~0x200;
			vga.crtc.vert_blank_start  &= ~0x200;
			vga.crtc.scan_doubling      = ((data & 0x80) >> 7);
			vga.crtc.line_compare      |= ((data & 0x40) << (9-6));
			vga.crtc.vert_blank_start  |= ((data & 0x20) << (9-5));
			vga.crtc.maximum_scan_line  = (data & 0x1f) + 1;
			break;
		case 0x0a:
			vga.crtc.cursor_enable = ((data & 0x20) ^ 0x20) >> 5;
			vga.crtc.cursor_scan_start = data & 0x1f;
			break;
		case 0x0b:
			vga.crtc.cursor_skew = (data & 0x60) >> 5;
			vga.crtc.cursor_scan_end = data & 0x1f;
			break;
		case 0x0c:
		case 0x0d:
			vga.crtc.start_addr_latch &= ~(0xff << (((index & 1)^1) * 8));
			vga.crtc.start_addr_latch |= (data << (((index & 1)^1) * 8));
			break;
		case 0x0e:
		case 0x0f:
			vga.crtc.cursor_addr &= ~(0xff << (((index & 1)^1) * 8));
			vga.crtc.cursor_addr |= (data << (((index & 1)^1) * 8));
			break;
		case 0x10:
			vga.crtc.vert_retrace_start &= ~0xff;
			vga.crtc.vert_retrace_start |= data & 0xff;
			break;
		case 0x11:
			vga.crtc.protect_enable = (data & 0x80) >> 7;
			vga.crtc.bandwidth = (data & 0x40) >> 6;
			vga.crtc.vert_retrace_end = data & 0x0f;
			vga.crtc.irq_clear = (data & 0x10) >> 4;
			vga.crtc.irq_disable = (data & 0x20) >> 5;
			break;
		case 0x12:
			vga.crtc.vert_disp_end &= ~0xff;
			vga.crtc.vert_disp_end |= data & 0xff;
			recompute_params();
			break;
		case 0x13:
			vga.crtc.offset &= ~0xff;
			vga.crtc.offset |= data & 0xff;
			break;
		case 0x14:
			vga.crtc.dw = (data & 0x40) >> 6;
			vga.crtc.div4 = (data & 0x20) >> 5;
			vga.crtc.underline_loc = (data & 0x1f);
			break;
		case 0x15:
			vga.crtc.vert_blank_start &= ~0xff;
			vga.crtc.vert_blank_start |= data & 0xff;
			break;
		case 0x16:
			vga.crtc.vert_blank_end = data & 0x7f;
			break;
		case 0x17:
			vga.crtc.sync_en = (data & 0x80) >> 7;
			vga.crtc.word_mode = (data & 0x40) >> 6;
			vga.crtc.aw = (data & 0x20) >> 5;
			vga.crtc.div2 = (data & 0x08) >> 3;
			vga.crtc.sldiv = (data & 0x04) >> 2;
			vga.crtc.map14 = (data & 0x02) >> 1;
			vga.crtc.map13 = (data & 0x01) >> 0;
			break;
		case 0x18:
			vga.crtc.line_compare &= ~0xff;
			vga.crtc.line_compare |= data & 0xff;
			break;
		default:
			LOG( "Unhandled CRTC reg w %02x %02x\n",index,data);
			break;
	}
}

void vga_device::seq_reg_write(uint8_t index, uint8_t data)
{
	switch(index)
	{
		case 0x02:
			vga.sequencer.map_mask = data & 0xf;
			break;
		case 0x03:
			/* --2- 84-- character select A
			   ---2 --84 character select B */
			vga.sequencer.char_sel.A = (((data & 0xc) >> 2)<<1) | ((data & 0x20) >> 5);
			vga.sequencer.char_sel.B = (((data & 0x3) >> 0)<<1) | ((data & 0x10) >> 4);
			if(data)
				popmessage("Char SEL checker, contact MAMEdev (%02x %02x)\n",vga.sequencer.char_sel.A,vga.sequencer.char_sel.B);
			break;
	}
}

uint8_t vga_device::vga_vblank()
{
	uint8_t res;
	uint16_t vblank_start,vblank_end,vpos;

	/* calculate vblank start / end positions */
	res = 0;
	vblank_start = vga.crtc.vert_blank_start;
	vblank_end = vga.crtc.vert_blank_start + vga.crtc.vert_blank_end;
	vpos = screen().vpos();

	/* check if we are under vblank period */
	if(vblank_end > vga.crtc.vert_total)
	{
		vblank_end -= vga.crtc.vert_total;
		if(vpos >= vblank_start || vpos <= vblank_end)
			res = 1;
	}
	else
	{
		if(vpos >= vblank_start && vpos <= vblank_end)
			res = 1;
	}

	//popmessage("%d %d %d - SR1=%02x",vblank_start,vblank_end,vga.crtc.vert_total,vga.sequencer.data[1]);

	return res;
}

uint8_t vga_device::vga_crtc_r(offs_t offset)
{
	uint8_t data = 0xff;

	switch (offset) {
	case 4:
		data = vga.crtc.index;
		break;
	case 5:
		data = crtc_reg_read(vga.crtc.index);
		break;
	case 0xa:
		uint8_t hsync,vsync;
		vga.attribute.state = 0;
		data = 0;

		hsync = screen().hblank() & 1;
		vsync = vga_vblank(); //screen().vblank() & 1;

		data |= (hsync | vsync) & 1; // DD - display disable register
		data |= (vsync & 1) << 3; // VRetrace register

		/* ega diagnostic readback enough for oak bios */
		switch (vga.attribute.data[0x12]&0x30) {
		case 0:
			if (vga.attribute.data[0x11]&1) data|=0x10;
			if (vga.attribute.data[0x11]&4) data|=0x20;
			break;
		case 0x10:
			data|=(vga.attribute.data[0x11]&0x30);
			break;
		case 0x20:
			if (vga.attribute.data[0x11]&2) data|=0x10;
			if (vga.attribute.data[0x11]&8) data|=0x20;
			break;
		case 0x30:
			data|=(vga.attribute.data[0x11]&0xc0)>>2;
			break;
		}
		break;
	case 0xf:
		/* oak test */
		//data=0;
		/* pega bios on/off */
		data=0x80;
		break;
	}
	return data;
}

void vga_device::vga_crtc_w(offs_t offset, uint8_t data)
{
	switch (offset)
	{
		case 4:
			vga.crtc.index = data;
			break;

		case 5:
			LOGREGS("vga_crtc_w(): CRTC[0x%02X%s] = 0x%02X\n",
				vga.crtc.index,
				(vga.crtc.index < vga.svga_intf.crtc_regcount) ? "" : "?",
				data);

			crtc_reg_write(vga.crtc.index,data);
			//screen().update_partial(screen().vpos());
			#if 0
			if((vga.crtc.index & 0xfe) != 0x0e)
				printf("%02x %02x %d\n",vga.crtc.index,data,screen().vpos());
			#endif
			break;

		case 0xa:
			vga.feature_control = data;
			break;
	}
}



uint8_t vga_device::port_03b0_r(offs_t offset)
{
	uint8_t data = 0xff;
	if (get_crtc_port() == 0x3b0)
		data = vga_crtc_r(offset);
	return data;
}

uint8_t vga_device::gc_reg_read(uint8_t index)
{
	uint8_t res;

	switch(index)
	{
		case 0x00:
			res = vga.gc.set_reset & 0xf;
			break;
		case 0x01:
			res = vga.gc.enable_set_reset & 0xf;
			break;
		case 0x02:
			res = vga.gc.color_compare & 0xf;
			break;
		case 0x03:
			res  = (vga.gc.logical_op & 3) << 3;
			res |= (vga.gc.rotate_count & 7);
			break;
		case 0x04:
			res = vga.gc.read_map_sel & 3;
			break;
		case 0x05:
			res  = (vga.gc.shift256 & 1) << 6;
			res |= (vga.gc.shift_reg & 1) << 5;
			res |= (vga.gc.host_oe & 1) << 4;
			res |= (vga.gc.read_mode & 1) << 3;
			res |= (vga.gc.write_mode & 3);
			break;
		case 0x06:
			res  = (vga.gc.memory_map_sel & 3) << 2;
			res |= (vga.gc.chain_oe & 1) << 1;
			res |= (vga.gc.alpha_dis & 1);
			break;
		case 0x07:
			res = vga.gc.color_dont_care & 0xf;
			break;
		case 0x08:
			res = vga.gc.bit_mask & 0xff;
			break;
		default:
			res = 0xff;
			break;
	}

	return res;
}

uint8_t vga_device::seq_reg_read(uint8_t index)
{
	LOG( "Reading unmapped sequencer read register %02x (SVGA?)\n", index);
	return 0;
}

/*
  4 additional dipswitches
  seems to have emulation modes at register level
  (mda/hgc lines bit 8 not identical to ega/vga)

  standard ega/vga dipswitches
  00000000  320x200
  00000001  640x200 hanging
  00000010  640x200 hanging
  00000011  640x200 hanging

  00000100  640x350 hanging
  00000101  640x350 hanging EGA mono
  00000110  320x200
  00000111  640x200

  00001000  640x200
  00001001  640x200
  00001010  720x350 partial visible
  00001011  720x350 partial visible

  00001100  320x200
  00001101  320x200
  00001110  320x200
  00001111  320x200
*/
static INPUT_PORTS_START(vga_sense)
	PORT_START("VGA_SENSE")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END

ioport_constructor vga_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(vga_sense);
}

uint8_t vga_device::port_03c0_r(offs_t offset)
{
	uint8_t data = 0xff;

	switch (offset)
	{
		case 0:
			data = vga.attribute.index;
			break;
		case 1:
			if((vga.attribute.index&0x20)
			&& ((vga.attribute.index&0x1f)<0x10))
				data = 0; // palette access is disabled in this mode
			else if ((vga.attribute.index&0x1f)<sizeof(vga.attribute.data))
				data=vga.attribute.data[vga.attribute.index&0x1f];
			break;

		case 2:
		{
			data = 0x60; // is VGA
			const u8 sense_bit = (3 - (vga.miscellaneous_output >> 2)) & 3;
			LOGDSW("Reading sense bit %d\n", sense_bit);
			if(BIT(m_input_sense->read(), sense_bit))
				data |= 0x10;
			break;
		}

		case 3:
			data = vga.oak.reg;
			break;

		case 4:
			data = vga.sequencer.index;
			break;

		case 5:
			if (vga.sequencer.index < vga.svga_intf.seq_regcount)
				data = vga.sequencer.data[vga.sequencer.index];
			else
				data = seq_reg_read(vga.sequencer.index);
			break;

		case 6:
			data = vga.dac.mask;
			break;

		case 7:
			data = (vga.dac.read) ? 3 : 0;
			break;

		case 8:
			data = vga.dac.write_index;
			break;

		case 9:
			if (vga.dac.read)
			{
				switch (vga.dac.state++)
				{
					case 0:
						data = vga.dac.color[3*vga.dac.read_index];
						break;
					case 1:
						data = vga.dac.color[3*vga.dac.read_index + 1];
						break;
					case 2:
						data = vga.dac.color[3*vga.dac.read_index + 2];
						break;
				}

				if (vga.dac.state==3)
				{
					vga.dac.state = 0;
					vga.dac.read_index++;
				}
			}
			break;

		case 0xa:
			data = vga.feature_control;
			break;

		case 0xc:
			data = vga.miscellaneous_output;
			break;

		case 0xe:
			data = vga.gc.index;
			break;

		case 0xf:
			data = gc_reg_read(vga.gc.index);
			break;
	}
	return data;
}

uint8_t vga_device::port_03d0_r(offs_t offset)
{
	uint8_t data = 0xff;
	if (get_crtc_port() == 0x3d0)
		data = vga_crtc_r(offset);
	if(offset == 8)
	{
		LOG("VGA: 0x3d8 read %s\n", machine().describe_context());
		data = 0; // TODO: PC-200 reads back CGA register here, everything else returns open bus OR CGA emulation of register 0x3d8
	}

	return data;
}

void vga_device::port_03b0_w(offs_t offset, uint8_t data)
{
	if (get_crtc_port() == 0x3b0)
		vga_crtc_w(offset, data);
}

void vga_device::attribute_reg_write(uint8_t index, uint8_t data)
{
	if((index & 0x30) == 0)
	{
		//if(vga.sequencer.data[1]&0x20) // ok?
		vga.attribute.data[index & 0x1f] = data & 0x3f;
	}
	else
	{
		switch(index & 0x1f)
		{
			/* TODO: intentional dirtiness, variable names to be properly changed */
			case 0x10: vga.attribute.data[0x10] = data; break;
			case 0x11: vga.attribute.data[0x11] = data; break;
			case 0x12: vga.attribute.data[0x12] = data; break;
			case 0x13: vga.attribute.pel_shift_latch = vga.attribute.data[0x13] = data; break;
			case 0x14: vga.attribute.data[0x14] = data; break;
		}
	}
}

void vga_device::gc_reg_write(uint8_t index,uint8_t data)
{
	switch(index)
	{
		case 0x00:
			vga.gc.set_reset = data & 0xf;
			break;
		case 0x01:
			vga.gc.enable_set_reset = data & 0xf;
			break;
		case 0x02:
			vga.gc.color_compare = data & 0xf;
			break;
		case 0x03:
			vga.gc.logical_op = (data & 0x18) >> 3;
			vga.gc.rotate_count = data & 7;
			break;
		case 0x04:
			vga.gc.read_map_sel = data & 3;
			break;
		case 0x05:
			vga.gc.shift256 = (data & 0x40) >> 6;
			vga.gc.shift_reg = (data & 0x20) >> 5;
			vga.gc.host_oe = (data & 0x10) >> 4;
			vga.gc.read_mode = (data & 8) >> 3;
			vga.gc.write_mode = data & 3;
			//if(data & 0x10 && vga.gc.alpha_dis)
			//  popmessage("Host O/E enabled, contact MAMEdev");
			break;
		case 0x06:
			vga.gc.memory_map_sel = (data & 0xc) >> 2;
			vga.gc.chain_oe = (data & 2) >> 1;
			vga.gc.alpha_dis = (data & 1);
			//if(data & 2 && vga.gc.alpha_dis)
			//  popmessage("Chain O/E enabled, contact MAMEdev");
			break;
		case 0x07:
			vga.gc.color_dont_care = data & 0xf;
			break;
		case 0x08:
			vga.gc.bit_mask = data & 0xff;
			break;
	}
}

void vga_device::port_03c0_w(offs_t offset, uint8_t data)
{
	switch (offset) {
	case 0:
		if (vga.attribute.state==0)
		{
			vga.attribute.index=data;
		}
		else
		{
			attribute_reg_write(vga.attribute.index,data);
		}
		vga.attribute.state=!vga.attribute.state;
		break;
	case 2:
		vga.miscellaneous_output=data;
		recompute_params();
		break;
	case 3:
		vga.oak.reg = data;
		break;
	case 4:
		vga.sequencer.index = data;
		break;
	case 5:
		LOGREGS("vga_port_03c0_w(): SEQ[0x%02X%s] = 0x%02X\n",
			vga.sequencer.index,
			(vga.sequencer.index < vga.svga_intf.seq_regcount) ? "" : "?",
			data);
		if (vga.sequencer.index < vga.svga_intf.seq_regcount)
		{
			vga.sequencer.data[vga.sequencer.index] = data;
		}

		seq_reg_write(vga.sequencer.index,data);
		recompute_params();
		break;
	case 6:
		vga.dac.mask=data;
		vga.dac.dirty=1;
		break;
	case 7:
		vga.dac.read_index=data;
		vga.dac.state=0;
		vga.dac.read=1;
		break;
	case 8:
		vga.dac.write_index=data;
		vga.dac.state=0;
		vga.dac.read=0;
		break;
	case 9:
		if (!vga.dac.read)
		{
			switch (vga.dac.state++) {
			case 0:
				vga.dac.color[3*vga.dac.write_index]=data;
				break;
			case 1:
				vga.dac.color[3*vga.dac.write_index + 1]=data;
				break;
			case 2:
				vga.dac.color[3*vga.dac.write_index + 2]=data;
				break;
			}
			vga.dac.dirty=1;
			if (vga.dac.state==3) {
				vga.dac.state=0; vga.dac.write_index++;
			}
		}
		break;
	case 0xe:
		vga.gc.index=data;
		break;
	case 0xf:
		gc_reg_write(vga.gc.index,data);
		break;
	}
}



void vga_device::port_03d0_w(offs_t offset, uint8_t data)
{
	if (get_crtc_port() == 0x3d0)
		vga_crtc_w(offset, data);
}

void vga_device::device_reset()
{
	/* clear out the VGA structure */
	memset(vga.pens, 0, sizeof(vga.pens));
	vga.miscellaneous_output = 0;
	vga.feature_control = 0;
	vga.sequencer.index = 0;
	memset(vga.sequencer.data, 0, sizeof(vga.sequencer.data));
	vga.crtc.index = 0;
	memset(vga.crtc.data, 0, sizeof(vga.crtc.data));
	vga.gc.index = 0;
	memset(vga.gc.latch, 0, sizeof(vga.gc.latch));
	memset(&vga.attribute, 0, sizeof(vga.attribute));
	memset(&vga.dac, 0, sizeof(vga.dac));
	memset(&vga.cursor, 0, sizeof(vga.cursor));
	memset(&vga.oak, 0, sizeof(vga.oak));

	vga.gc.memory_map_sel = 0x3; /* prevent xtbios excepting vga ram as system ram */
/* amstrad pc1640 bios relies on the position of
   the video memory area,
   so I introduced the reset to switch to b8000 area */
	vga.sequencer.data[4] = 0;

	/* TODO: real defaults */
	vga.crtc.line_compare = 0x3ff;
	/* skeleton/indiana.cpp boot PROM doesn't set this and assumes it's 0xff */
	vga.dac.mask = 0xff;
}

uint8_t vga_device::mem_r(offs_t offset)
{
	/* TODO: check me */
	switch(vga.gc.memory_map_sel & 0x03)
	{
		case 0: break;
		case 1: offset &= 0x0ffff; break;
		case 2: offset -= 0x10000; offset &= 0x07fff; break;
		case 3: offset -= 0x18000; offset &= 0x07fff; break;
	}

	if(vga.sequencer.data[4] & 4)
	{
		int data;
		if (!machine().side_effects_disabled())
		{
			vga.gc.latch[0]=vga.memory[(offset)];
			vga.gc.latch[1]=vga.memory[(offset)+0x10000];
			vga.gc.latch[2]=vga.memory[(offset)+0x20000];
			vga.gc.latch[3]=vga.memory[(offset)+0x30000];
		}

		if (vga.gc.read_mode)
		{
			// In Read Mode 1 latch is checked against this
			// cfr. lombrall & intsocch where they RMW sprite-like objects
			// and anything outside this formula goes transparent.
			const u8 target_color = (vga.gc.color_compare & vga.gc.color_dont_care);
			data = 0;

			for(u8 byte = 0; byte < 8; byte++)
			{
				u8 fill_latch = 0;
				for(u8 layer = 0; layer < 4; layer++)
				{
					if(vga.gc.latch[layer] & 1 << byte)
						fill_latch |= 1 << layer;
				}
				fill_latch &= vga.gc.color_dont_care;
				if(fill_latch == target_color)
					data |= 1 << byte;
			}
		}
		else
			data=vga.gc.latch[vga.gc.read_map_sel];

		return data;
	}
	else
	{
		// TODO: Guesswork, probably not right
		uint8_t i,data;

		data = 0;
		//printf("%08x\n",offset);

		for(i=0;i<4;i++)
		{
			if(vga.sequencer.map_mask & 1 << i)
				data |= vga.memory[offset+i*0x10000];
		}

		return data;
	}

	// never executed
	//return 0;
}

void vga_device::mem_w(offs_t offset, uint8_t data)
{
	//Inside each case must prevent writes to non-mapped VGA memory regions, not only mask the offset.
	switch(vga.gc.memory_map_sel & 0x03)
	{
		case 0: break;
		case 1:
			if(offset & 0x10000)
				return;

			offset &= 0x0ffff;
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
		uint8_t i;

		for(i=0;i<4;i++)
		{
			if(vga.sequencer.map_mask & 1 << i)
				vga.memory[offset+i*0x10000] = (vga.sequencer.data[4] & 4) ? vga_latch_write(i,data) : data;
		}
		return;
	}
}

uint8_t vga_device::mem_linear_r(offs_t offset)
{
	return vga.memory[offset % vga.svga_intf.vram_size];
}

void vga_device::mem_linear_w(offs_t offset, uint8_t data)
{
	vga.memory[offset % vga.svga_intf.vram_size] = data;
}
