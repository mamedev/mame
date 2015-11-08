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
    ROM_LOAD("oakvga.bin", 0xc0000, 0x8000, 0x318c5f43)
    (tseng labs famous et4000 isa vga card (oem))
    ROM_LOAD("et4000b.bin", 0xc0000, 0x8000, 0xa903540d)
    (tseng labs famous et4000 isa vga card)
    ROM_LOAD("et4000.bin", 0xc0000, 0x8000, 0xf01e4be0)

***************************************************************************/

#include "emu.h"
#include "pc_vga.h"
#include "bus/isa/trident.h"
#include "machine/eepromser.h"
#include "debugger.h"

/***************************************************************************

    Local variables

***************************************************************************/

enum
{
	IBM8514_IDLE = 0,
	IBM8514_DRAWING_RECT,
	IBM8514_DRAWING_LINE,
	IBM8514_DRAWING_BITBLT,
	IBM8514_DRAWING_PATTERN,
	IBM8514_DRAWING_SSV_1,
	IBM8514_DRAWING_SSV_2
};

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

#define CHAR_WIDTH ((vga.sequencer.data[1]&1)?8:9)

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

    Static declarations

***************************************************************************/

#define LOG_ACCESSES    0
#define LOG_REGISTERS   0

#define LOG_8514        1

/***************************************************************************

    Generic VGA

***************************************************************************/
// device type definition
const device_type VGA = &device_creator<vga_device>;
const device_type TSENG_VGA = &device_creator<tseng_vga_device>;
const device_type S3_VGA = &device_creator<s3_vga_device>;
const device_type GAMTOR_VGA = &device_creator<gamtor_vga_device>;
const device_type ATI_VGA = &device_creator<ati_vga_device>;
const device_type IBM8514A = &device_creator<ibm8514a_device>;
const device_type MACH8 = &device_creator<mach8_device>;

vga_device::vga_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source)
	: device_t(mconfig, type, name, tag, owner, clock, shortname, source),
		m_palette(*this, "^palette"),
		m_screen(*this,"^screen")
{
}

vga_device::vga_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, VGA, "VGA", tag, owner, clock, "vga", __FILE__),
		m_palette(*this, "^palette"),
		m_screen(*this,"^screen")
{
}

svga_device::svga_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source)
	: vga_device(mconfig, type, name, tag, owner, clock, shortname, source)
{
}

tseng_vga_device::tseng_vga_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: svga_device(mconfig, TSENG_VGA, "TSENG LABS VGA", tag, owner, clock, "tseng_vga", __FILE__)
{
}

s3_vga_device::s3_vga_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: ati_vga_device(mconfig, S3_VGA, "S3 Graphics VGA", tag, owner, clock, "s3_vga", __FILE__)
{
}

s3_vga_device::s3_vga_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source)
	: ati_vga_device(mconfig, type, name, tag, owner, clock, shortname, source)
{
}

gamtor_vga_device::gamtor_vga_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: svga_device(mconfig, GAMTOR_VGA, "GAMTOR VGA", tag, owner, clock, "gamtor_vga", __FILE__)
{
}

ati_vga_device::ati_vga_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: svga_device(mconfig, ATI_VGA, "ATI VGA", tag, owner, clock, "ati_vga", __FILE__)
{
}

ati_vga_device::ati_vga_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source)
	: svga_device(mconfig, type, name, tag, owner, clock, shortname, source)
{
}

ibm8514a_device::ibm8514a_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, IBM8514A, "IBM8514A Video", tag, owner, clock, "ibm8514a", __FILE__)
{
}

ibm8514a_device::ibm8514a_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source)
	: device_t(mconfig, type, name, tag, owner, clock, shortname, source)
{
}

mach8_device::mach8_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source)
	: ibm8514a_device(mconfig, type, name, tag, owner, clock, shortname, source)
{
}

mach8_device::mach8_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: ibm8514a_device(mconfig, MACH8, "MACH8", tag, owner, clock, "mach8", __FILE__)
{
}

// zero everything, keep vtbls
void vga_device::zero()
{
	memset(&vga.svga_intf, 0, sizeof(vga.svga_intf));
	vga.memory.resize(0);
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
	m_vblank_timer->adjust( machine().first_screen()->time_until_pos(vga.crtc.vert_blank_start + vga.crtc.vert_blank_end) );
}

void vga_device::device_start()
{
	zero();

	int i;
	for (i = 0; i < 0x100; i++)
		m_palette->set_pen_color(i, 0, 0, 0);

	// Avoid an infinite loop when displaying.  0 is not possible anyway.
	vga.crtc.maximum_scan_line = 1;


	// copy over interfaces
	vga.read_dipswitch = read8_delegate(); //read_dipswitch;
	vga.svga_intf.seq_regcount = 0x05;
	vga.svga_intf.crtc_regcount = 0x19;
	vga.svga_intf.vram_size = 0x100000;

	vga.memory.resize(vga.svga_intf.vram_size);
	memset(&vga.memory[0], 0, vga.svga_intf.vram_size);
	save_item(NAME(vga.memory));
	save_pointer(vga.crtc.data,"CRTC Registers",0x100);
	save_pointer(vga.sequencer.data,"Sequencer Registers",0x100);
	save_pointer(vga.attribute.data,"Attribute Registers", 0x15);

	m_vblank_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(vga_device::vblank_timer_cb),this));
}

void svga_device::device_start()
{
	vga_device::device_start();
	memset(&svga, 0, sizeof(svga));
}

void ati_vga_device::device_start()
{
	svga_device::device_start();
	memset(&ati, 0, sizeof(ati));
	save_pointer(ati.ext_reg,"ATi Extended Registers",64);
	m_8514 = subdevice<mach8_device>("8514a");
	ati.vga_chip_id = 0x06;  // 28800-6
}

void s3_vga_device::device_start()
{
	svga_device::device_start();
	memset(&s3, 0, sizeof(s3));
	int x;
	// Initialise hardware graphics cursor colours, Windows 95 doesn't touch the registers for some reason
	for(x=0;x<4;x++)
	{
		s3.cursor_fg[x] = 0xff;
		s3.cursor_bg[x] = 0x00;
	}
	m_8514 = subdevice<ibm8514a_device>("8514a");
	// set device ID
	s3.id_high = 0x88;  // CR2D
	s3.id_low = 0x11;   // CR2E
	s3.revision = 0x00; // CR2F
	s3.id_cr30 = 0xe1;  // CR30
}

void tseng_vga_device::device_start()
{
	svga_device::device_start();
	memset(&et4k, 0, sizeof(et4k));
}

void ibm8514a_device::device_start()
{
	memset(&ibm8514, 0, sizeof(ibm8514));
	ibm8514.read_mask = 0x00000000;
	ibm8514.write_mask = 0xffffffff;
}

void ibm8514a_device::device_config_complete()
{
	if(m_vga_tag.length() != 0)
	{
		m_vga = machine().device<vga_device>(m_vga_tag.c_str());
	}
}

void mach8_device::device_start()
{
	ibm8514a_device::device_start();
	memset(&mach8, 0, sizeof(mach8));
}

UINT16 vga_device::offset()
{
//  popmessage("Offset: %04x  %s %s **",vga.crtc.offset,vga.crtc.dw?"DW":"--",vga.crtc.word_mode?"BYTE":"WORD");
	if(vga.crtc.dw)
		return vga.crtc.offset << 3;
	if(vga.crtc.word_mode)
		return vga.crtc.offset << 1;
	else
		return vga.crtc.offset << 2;
}

void vga_device::vga_vh_text(bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	UINT8 ch, attr;
	UINT8 bits;
	UINT32 font_base;
	UINT32 *bitmapline;
	int width=CHAR_WIDTH, height = (vga.crtc.maximum_scan_line) * (vga.crtc.scan_doubling + 1);
	int pos, line, column, mask, w, h, addr;
	UINT8 blink_en,fore_col,back_col;
	pen_t pen;

	if(vga.crtc.cursor_enable)
		vga.cursor.visible = machine().first_screen()->frame_number() & 0x10;
	else
		vga.cursor.visible = 0;

	for (addr = vga.crtc.start_addr, line = -vga.crtc.preset_row_scan; line < TEXT_LINES;
			line += height, addr += (offset()>>1))
	{
		for (pos = addr, column=0; column<TEXT_COLUMNS; column++, pos++)
		{
			ch   = vga.memory[(pos<<1) + 0];
			attr = vga.memory[(pos<<1) + 1];
			font_base = 0x20000+(ch<<5);
			font_base += ((attr & 8) ? vga.sequencer.char_sel.A : vga.sequencer.char_sel.B)*0x2000;
			blink_en = (vga.attribute.data[0x10]&8&&machine().first_screen()->frame_number() & 0x20) ? attr & 0x80 : 0;

			fore_col = attr & 0xf;
			back_col = (attr & 0x70) >> 4;
			back_col |= (vga.attribute.data[0x10]&8) ? 0 : ((attr & 0x80) >> 4);

			for (h = MAX(-line, 0); (h < height) && (line+h < MIN(TEXT_LINES, bitmap.height())); h++)
			{
				bitmapline = &bitmap.pix32(line+h);
				bits = vga.memory[font_base+(h>>(vga.crtc.scan_doubling))];

				for (mask=0x80, w=0; (w<width)&&(w<8); w++, mask>>=1)
				{
					if (bits&mask)
						pen = vga.pens[blink_en ? back_col : fore_col];
					else
						pen = vga.pens[back_col];

					if(!machine().first_screen()->visible_area().contains(column*width+w, line+h))
						continue;
					bitmapline[column*width+w] = pen;

				}
				if (w<width)
				{
					/* 9 column */
					if (TEXT_COPY_9COLUMN(ch)&&(bits&1))
						pen = vga.pens[blink_en ? back_col : fore_col];
					else
						pen = vga.pens[back_col];

					if(!machine().first_screen()->visible_area().contains(column*width+w, line+h))
						continue;
					bitmapline[column*width+w] = pen;
				}
			}
			if (vga.cursor.visible&&(pos==vga.crtc.cursor_addr))
			{
				for (h=vga.crtc.cursor_scan_start;
						(h<=vga.crtc.cursor_scan_end)&&(h<height)&&(line+h<TEXT_LINES);
						h++)
				{
					if(!machine().first_screen()->visible_area().contains(column*width, line+h))
						continue;
					bitmap.plot_box(column*width, line+h, width, 1, vga.pens[attr&0xf]);
				}
			}
		}
	}
}

void vga_device::vga_vh_ega(bitmap_rgb32 &bitmap,  const rectangle &cliprect)
{
	int pos, line, column, c, addr, i, yi;
	int height = vga.crtc.maximum_scan_line * (vga.crtc.scan_doubling + 1);
	UINT32 *bitmapline;
	pen_t pen;
	int pel_shift = (vga.attribute.pel_shift & 7);

//  popmessage("%08x %02x",EGA_START_ADDRESS,pel_shift);

	/**/
	for (addr=EGA_START_ADDRESS, pos=0, line=0; line<LINES;
			line += height, addr += offset())
	{
		for(yi=0;yi<height;yi++)
		{
			bitmapline = &bitmap.pix32(line + yi);

			for (pos=addr, c=0, column=0; column<EGA_COLUMNS+1; column++, c+=8, pos=(pos+1)&0xffff)
			{
				int data[4];

				data[0]=vga.memory[(pos & 0xffff)];
				data[1]=vga.memory[(pos & 0xffff)+0x10000]<<1;
				data[2]=vga.memory[(pos & 0xffff)+0x20000]<<2;
				data[3]=vga.memory[(pos & 0xffff)+0x30000]<<3;

				for (i = 7; i >= 0; i--)
				{
					pen = vga.pens[(data[0]&1) | (data[1]&2) | (data[2]&4) | (data[3]&8)];

					data[0]>>=1;
					data[1]>>=1;
					data[2]>>=1;
					data[3]>>=1;

					if(!machine().first_screen()->visible_area().contains(c+i-pel_shift, line + yi))
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
	int pos, line, column, c, addr, curr_addr;
	UINT32 *bitmapline;
	UINT16 mask_comp;
	int height = vga.crtc.maximum_scan_line * (vga.crtc.scan_doubling + 1);
	int yi;
	int xi;
	int pel_shift = (vga.attribute.pel_shift & 6);

	/* line compare is screen sensitive */
	mask_comp = 0x3ff; //| (LINES & 0x300);

//  popmessage("%02x %02x",vga.attribute.pel_shift,vga.sequencer.data[4] & 0x08);

	curr_addr = 0;
	if(!(vga.sequencer.data[4] & 0x08))
	{
		for (addr = VGA_START_ADDRESS, line=0; line<LINES; line+=height, addr+=offset(), curr_addr+=offset())
		{
			for(yi = 0;yi < height; yi++)
			{
				if((line + yi) < (vga.crtc.line_compare & mask_comp))
					curr_addr = addr;
				if((line + yi) == (vga.crtc.line_compare & mask_comp))
				{
					curr_addr = 0;
					pel_shift = 0;
				}
				bitmapline = &bitmap.pix32(line + yi);
				for (pos=curr_addr, c=0, column=0; column<VGA_COLUMNS+1; column++, c+=8, pos++)
				{
					if(pos > 0x80000/4)
						return;

					for(xi=0;xi<8;xi++)
					{
						if(!machine().first_screen()->visible_area().contains(c+xi-(pel_shift), line + yi))
							continue;
						bitmapline[c+xi-(pel_shift)] = m_palette->pen(vga.memory[(pos & 0xffff)+((xi >> 1)*0x10000)]);
					}
				}
			}
		}
	}
	else
	{
		for (addr = VGA_START_ADDRESS, line=0; line<LINES; line+=height, addr+=offset(), curr_addr+=offset())
		{
			for(yi = 0;yi < height; yi++)
			{
				if((line + yi) < (vga.crtc.line_compare & mask_comp))
					curr_addr = addr;
				if((line + yi) == (vga.crtc.line_compare & mask_comp))
					curr_addr = 0;
				bitmapline = &bitmap.pix32(line + yi);
				//addr %= 0x80000;
				for (pos=curr_addr, c=0, column=0; column<VGA_COLUMNS+1; column++, c+=0x10, pos+=0x8)
				{
					if(pos + 0x08 > 0x80000)
						return;

					for(xi=0;xi<0x10;xi++)
					{
						if(!machine().first_screen()->visible_area().contains(c+xi-(pel_shift), line + yi))
							continue;
						bitmapline[c+xi-pel_shift] = m_palette->pen(vga.memory[(pos+(xi >> 1)) & 0xffff]);
					}
				}
			}
		}
	}
}

void vga_device::vga_vh_cga(bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	UINT32 *bitmapline;
	int height = (vga.crtc.scan_doubling + 1);
	int x,xi,y,yi;
	UINT32 addr;
	pen_t pen;
	int width;

	width = (vga.crtc.horz_disp_end + 1) * 8;

	for(y=0;y<LINES;y++)
	{
		addr = ((y & 1) * 0x2000) + (((y & ~1) >> 1) * width/4);

		for(x=0;x<width;x+=4)
		{
			for(yi=0;yi<height;yi++)
			{
				bitmapline = &bitmap.pix32(y * height + yi);

				for(xi=0;xi<4;xi++)
				{
					pen = vga.pens[(vga.memory[addr] >> (6-xi*2)) & 3];
					if(!machine().first_screen()->visible_area().contains(x+xi, y * height + yi))
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
	UINT32 *bitmapline;
	int height = (vga.crtc.scan_doubling + 1);
	int x,xi,y,yi;
	UINT32 addr;
	pen_t pen;
	int width;

	width = (vga.crtc.horz_disp_end + 1) * 8;

	for(y=0;y<LINES;y++)
	{
		addr = ((y & 1) * 0x2000) + (((y & ~1) >> 1) * width/8);

		for(x=0;x<width;x+=8)
		{
			for(yi=0;yi<height;yi++)
			{
				bitmapline = &bitmap.pix32(y * height + yi);

				for(xi=0;xi<8;xi++)
				{
					pen = vga.pens[(vga.memory[addr] >> (7-xi)) & 1];
					if(!machine().first_screen()->visible_area().contains(x+xi, y * height + yi))
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
	int pos, line, column, c, addr, curr_addr;
	UINT32 *bitmapline;
	UINT16 mask_comp;
	int height = vga.crtc.maximum_scan_line * (vga.crtc.scan_doubling + 1);
	int yi;
	int xi;
	UINT8 start_shift;
//  UINT16 line_length;

	/* line compare is screen sensitive */
	mask_comp = 0x3ff;
	curr_addr = 0;
//  if(vga.crtc.dw)
//      line_length = vga.crtc.offset << 3;  // doubleword mode
//  else
//  {
//      line_length = vga.crtc.offset << 4;
//  }

	start_shift = (!(vga.sequencer.data[4] & 0x08)) ? 2 : 0;
	{
		for (addr = VGA_START_ADDRESS << start_shift, line=0; line<LINES; line+=height, addr+=offset(), curr_addr+=offset())
		{
			for(yi = 0;yi < height; yi++)
			{
				if((line + yi) < (vga.crtc.line_compare & mask_comp))
					curr_addr = addr;
				if((line + yi) == (vga.crtc.line_compare & mask_comp))
					curr_addr = 0;
				bitmapline = &bitmap.pix32(line + yi);
				addr %= vga.svga_intf.vram_size;
				for (pos=curr_addr, c=0, column=0; column<VGA_COLUMNS; column++, c+=8, pos+=0x8)
				{
					if(pos + 0x08 >= vga.svga_intf.vram_size)
						return;

					for(xi=0;xi<8;xi++)
					{
						if(!machine().first_screen()->visible_area().contains(c+xi, line + yi))
							continue;
						bitmapline[c+xi] = m_palette->pen(vga.memory[(pos+(xi))]);
					}
				}
			}
		}
	}
}

void svga_device::svga_vh_rgb15(bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	#define MV(x) (vga.memory[x]+(vga.memory[x+1]<<8))
	#define IV 0xff000000
	int height = vga.crtc.maximum_scan_line * (vga.crtc.scan_doubling + 1);
	int xi;
	int yi;
	int xm;
	int pos, line, column, c, addr, curr_addr;

	UINT32 *bitmapline;
//  UINT16 mask_comp;

	/* line compare is screen sensitive */
//  mask_comp = 0xff | (TLINES & 0x300);
	curr_addr = 0;
	yi=0;
	for (addr = TGA_START_ADDRESS, line=0; line<TLINES; line+=height, addr+=offset(), curr_addr+=offset())
	{
		bitmapline = &bitmap.pix32(line);
		addr %= vga.svga_intf.vram_size;
		for (pos=addr, c=0, column=0; column<TGA_COLUMNS; column++, c+=8, pos+=0x10)
		{
			if(pos + 0x10 >= vga.svga_intf.vram_size)
				return;
			for(xi=0,xm=0;xi<8;xi++,xm+=2)
			{
				int r,g,b;

				if(!machine().first_screen()->visible_area().contains(c+xi, line + yi))
					continue;

				r = (MV(pos+xm)&0x7c00)>>10;
				g = (MV(pos+xm)&0x03e0)>>5;
				b = (MV(pos+xm)&0x001f)>>0;
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
	#define IV 0xff000000
	int height = vga.crtc.maximum_scan_line * (vga.crtc.scan_doubling + 1);
	int xi;
	int yi;
	int xm;
	int pos, line, column, c, addr, curr_addr;

	UINT32 *bitmapline;
//  UINT16 mask_comp;

	/* line compare is screen sensitive */
//  mask_comp = 0xff | (TLINES & 0x300);
	curr_addr = 0;
	yi=0;
	for (addr = TGA_START_ADDRESS, line=0; line<TLINES; line+=height, addr+=offset(), curr_addr+=offset())
	{
		bitmapline = &bitmap.pix32(line);
		addr %= vga.svga_intf.vram_size;
		for (pos=addr, c=0, column=0; column<TGA_COLUMNS; column++, c+=8, pos+=0x10)
		{
			if(pos + 0x10 >= vga.svga_intf.vram_size)
				return;
			for(xi=0,xm=0;xi<8;xi++,xm+=2)
			{
				int r,g,b;

				if(!machine().first_screen()->visible_area().contains(c+xi, line + yi))
					continue;

				r = (MV(pos+xm)&0xf800)>>11;
				g = (MV(pos+xm)&0x07e0)>>5;
				b = (MV(pos+xm)&0x001f)>>0;
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
	#define ID 0xff000000
	int height = vga.crtc.maximum_scan_line * (vga.crtc.scan_doubling + 1);
	int xi;
	int yi;
	int xm;
	int pos, line, column, c, addr, curr_addr;
	UINT32 *bitmapline;

//  UINT16 mask_comp;

	/* line compare is screen sensitive */
//  mask_comp = 0xff | (TLINES & 0x300);
	curr_addr = 0;
	yi=0;
	for (addr = TGA_START_ADDRESS<<1, line=0; line<TLINES; line+=height, addr+=offset(), curr_addr+=offset())
	{
		bitmapline = &bitmap.pix32(line);
		addr %= vga.svga_intf.vram_size;
		for (pos=addr, c=0, column=0; column<TGA_COLUMNS; column++, c+=8, pos+=24)
		{
			if(pos + 24 >= vga.svga_intf.vram_size)
				return;
			for(xi=0,xm=0;xi<8;xi++,xm+=3)
			{
				int r,g,b;

				if(!machine().first_screen()->visible_area().contains(c+xi, line + yi))
					continue;

				r = (MD(pos+xm)&0xff0000)>>16;
				g = (MD(pos+xm)&0x00ff00)>>8;
				b = (MD(pos+xm)&0x0000ff)>>0;
				bitmapline[c+xi] = IV|(r<<16)|(g<<8)|(b<<0);
			}
		}
	}
}

void svga_device::svga_vh_rgb32(bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	#define MD(x) (vga.memory[x]+(vga.memory[x+1]<<8)+(vga.memory[x+2]<<16))
	#define ID 0xff000000
	int height = vga.crtc.maximum_scan_line * (vga.crtc.scan_doubling + 1);
	int xi;
	int yi;
	int xm;
	int pos, line, column, c, addr, curr_addr;
	UINT32 *bitmapline;

//  UINT16 mask_comp;

	/* line compare is screen sensitive */
//  mask_comp = 0xff | (TLINES & 0x300);
	curr_addr = 0;
	yi=0;
	for (addr = TGA_START_ADDRESS, line=0; line<TLINES; line+=height, addr+=(offset()), curr_addr+=(offset()))
	{
		bitmapline = &bitmap.pix32(line);
		addr %= vga.svga_intf.vram_size;
		for (pos=addr, c=0, column=0; column<TGA_COLUMNS; column++, c+=8, pos+=0x20)
		{
			if(pos + 0x20 >= vga.svga_intf.vram_size)
				return;
			for(xi=0,xm=0;xi<8;xi++,xm+=4)
			{
				int r,g,b;

				if(!machine().first_screen()->visible_area().contains(c+xi, line + yi))
					continue;

				r = (MD(pos+xm)&0xff0000)>>16;
				g = (MD(pos+xm)&0x00ff00)>>8;
				b = (MD(pos+xm)&0x0000ff)>>0;
				bitmapline[c+xi] = IV|(r<<16)|(g<<8)|(b<<0);
			}
		}
	}
}

UINT8 vga_device::pc_vga_choosevideomode()
{
	int i;

	if (vga.crtc.sync_en)
	{
		if (vga.dac.dirty)
		{
			for (i=0; i<256;i++)
			{
				/* TODO: color shifters? */
				m_palette->set_pen_color(i, (vga.dac.color[i & vga.dac.mask].red & 0x3f) << 2,
										(vga.dac.color[i & vga.dac.mask].green & 0x3f) << 2,
										(vga.dac.color[i & vga.dac.mask].blue & 0x3f) << 2);
			}
			vga.dac.dirty = 0;
		}

		if (vga.attribute.data[0x10] & 0x80)
		{
			for (i=0; i<16;i++)
			{
				vga.pens[i] = m_palette->pen((vga.attribute.data[i]&0x0f)
											|((vga.attribute.data[0x14]&0xf)<<4));
			}
		}
		else
		{
			for (i=0; i<16;i++)
			{
				vga.pens[i]=m_palette->pen((vga.attribute.data[i]&0x3f)
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


UINT8 svga_device::pc_vga_choosevideomode()
{
	int i;

	if (vga.crtc.sync_en)
	{
		if (vga.dac.dirty)
		{
			for (i=0; i<256;i++)
			{
				/* TODO: color shifters? */
				m_palette->set_pen_color(i, (vga.dac.color[i & vga.dac.mask].red & 0x3f) << 2,
										(vga.dac.color[i & vga.dac.mask].green & 0x3f) << 2,
										(vga.dac.color[i & vga.dac.mask].blue & 0x3f) << 2);
			}
			vga.dac.dirty = 0;
		}

		if (vga.attribute.data[0x10] & 0x80)
		{
			for (i=0; i<16;i++)
			{
				vga.pens[i] = m_palette->pen((vga.attribute.data[i]&0x0f)
											|((vga.attribute.data[0x14]&0xf)<<4));
			}
		}
		else
		{
			for (i=0; i<16;i++)
			{
				vga.pens[i]=m_palette->pen((vga.attribute.data[i]&0x3f)
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


UINT32 vga_device::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	UINT8 cur_mode = pc_vga_choosevideomode();
	switch(cur_mode)
	{
		case SCREEN_OFF:   bitmap.fill  (m_palette->black_pen(), cliprect);break;
		case TEXT_MODE:    vga_vh_text  (bitmap, cliprect); break;
		case VGA_MODE:     vga_vh_vga   (bitmap, cliprect); break;
		case EGA_MODE:     vga_vh_ega   (bitmap, cliprect); break;
		case CGA_MODE:     vga_vh_cga   (bitmap, cliprect); break;
		case MONO_MODE:    vga_vh_mono  (bitmap, cliprect); break;
	}

	return 0;
}

UINT32 svga_device::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	UINT8 cur_mode = pc_vga_choosevideomode();

	switch(cur_mode)
	{
		case SCREEN_OFF:   bitmap.fill  (m_palette->black_pen(), cliprect);break;
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

UINT32 s3_vga_device::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	UINT8 cur_mode = 0;

	svga_device::screen_update(screen, bitmap, cliprect);

	cur_mode = pc_vga_choosevideomode();

	// draw hardware graphics cursor
	// TODO: support 16 bit and greater video modes
	if(s3.cursor_mode & 0x01)  // if cursor is enabled
	{
		UINT32 src;
		UINT32* dst;
		UINT8 val;
		int x,y;
		UINT16 cx = s3.cursor_x & 0x07ff;
		UINT16 cy = s3.cursor_y & 0x07ff;
		UINT32 bg_col;
		UINT32 fg_col;

		if(cur_mode == SCREEN_OFF || cur_mode == TEXT_MODE || cur_mode == MONO_MODE || cur_mode == CGA_MODE || cur_mode == EGA_MODE)
			return 0;  // cursor only works in VGA or SVGA modes

		src = s3.cursor_start_addr * 1024;  // start address is in units of 1024 bytes

		if(cur_mode == RGB16_MODE)
		{
			int r,g,b;
			UINT16 datax;

			datax = s3.cursor_bg[0]|s3.cursor_bg[1]<<8;
			r = (datax&0xf800)>>11;
			g = (datax&0x07e0)>>5;
			b = (datax&0x001f)>>0;
			r = (r << 3) | (r & 0x7);
			g = (g << 2) | (g & 0x3);
			b = (b << 3) | (b & 0x7);
			bg_col = (0xff<<24)|(r<<16)|(g<<8)|(b<<0);

			datax = s3.cursor_fg[0]|s3.cursor_fg[1]<<8;
			r = (datax&0xf800)>>11;
			g = (datax&0x07e0)>>5;
			b = (datax&0x001f)>>0;
			r = (r << 3) | (r & 0x7);
			g = (g << 2) | (g & 0x3);
			b = (b << 3) | (b & 0x7);
			fg_col = (0xff<<24)|(r<<16)|(g<<8)|(b<<0);
		}
		else /* TODO: other modes */
		{
			bg_col = m_palette->pen(s3.cursor_bg[0]);
			fg_col = m_palette->pen(s3.cursor_fg[0]);
		}

		//popmessage("%08x %08x",(s3.cursor_bg[0])|(s3.cursor_bg[1]<<8)|(s3.cursor_bg[2]<<16)|(s3.cursor_bg[3]<<24)
		//                    ,(s3.cursor_fg[0])|(s3.cursor_fg[1]<<8)|(s3.cursor_fg[2]<<16)|(s3.cursor_fg[3]<<24));
//      for(x=0;x<64;x++)
//          printf("%08x: %02x %02x %02x %02x\n",src+x*4,vga.memory[src+x*4],vga.memory[src+x*4+1],vga.memory[src+x*4+2],vga.memory[src+x*4+3]);
		for(y=0;y<64;y++)
		{
			dst = &bitmap.pix32(cy + y, cx);
			for(x=0;x<64;x++)
			{
				UINT16 bita = (vga.memory[(src+1) % vga.svga_intf.vram_size] | ((vga.memory[(src+0) % vga.svga_intf.vram_size]) << 8)) >> (15-(x % 16));
				UINT16 bitb = (vga.memory[(src+3) % vga.svga_intf.vram_size] | ((vga.memory[(src+2) % vga.svga_intf.vram_size]) << 8)) >> (15-(x % 16));
				val = ((bita & 0x01) << 1) | (bitb & 0x01);
				if(s3.extended_dac_ctrl & 0x10)
				{  // X11 mode
					switch(val)
					{
					case 0x00:
						// no change
						break;
					case 0x01:
						// no change
						break;
					case 0x02:
						dst[x] = bg_col;
						break;
					case 0x03:
						dst[x] = fg_col;
						break;
					}
				}
				else
				{  // Windows mode
					switch(val)
					{
					case 0x00:
						dst[x] = bg_col;
						break;
					case 0x01:
						dst[x] = fg_col;
						break;
					case 0x02:  // screen data
						// no change
						break;
					case 0x03:  // inverted screen data
						dst[x] = ~(dst[x]);
						break;
					}
				}
				if(x % 16 == 15)
					src+=4;
			}
		}
	}
	return 0;
}


/***************************************************************************/

inline UINT8 vga_device::vga_latch_write(int offs, UINT8 data)
{
	UINT8 res = 0;

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

UINT8 vga_device::crtc_reg_read(UINT8 index)
{
	UINT8 res;

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
	UINT8 hclock_m = (!GRAPHIC_MODE) ? CHAR_WIDTH : 8;
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
	machine().first_screen()->configure((hblank_period), (vblank_period), visarea, refresh );
	//popmessage("%d %d\n",vga.crtc.horz_total * 8,vga.crtc.vert_total);
	m_vblank_timer->adjust( machine().first_screen()->time_until_pos(vga.crtc.vert_blank_start + vga.crtc.vert_blank_end) );
}

void vga_device::recompute_params()
{
	if(vga.miscellaneous_output & 8)
		logerror("Warning: VGA external clock latch selected\n");
	else
		recompute_params_clock(1, (vga.miscellaneous_output & 0xc) ? XTAL_28_63636MHz : XTAL_25_1748MHz);
}

void vga_device::crtc_reg_write(UINT8 index, UINT8 data)
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
			logerror("Unhandled CRTC reg w %02x %02x\n",index,data);
			break;
	}
}

void vga_device::seq_reg_write(UINT8 index, UINT8 data)
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

UINT8 vga_device::vga_vblank()
{
	UINT8 res;
	UINT16 vblank_start,vblank_end,vpos;

	/* calculate vblank start / end positions */
	res = 0;
	vblank_start = vga.crtc.vert_blank_start;
	vblank_end = vga.crtc.vert_blank_start + vga.crtc.vert_blank_end;
	vpos = machine().first_screen()->vpos();

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

READ8_MEMBER(vga_device::vga_crtc_r)
{
	UINT8 data = 0xff;

	switch (offset) {
	case 4:
		data = vga.crtc.index;
		break;
	case 5:
		data = crtc_reg_read(vga.crtc.index);
		break;
	case 0xa:
		UINT8 hsync,vsync;
		vga.attribute.state = 0;
		data = 0;

		hsync = space.machine().first_screen()->hblank() & 1;
		vsync = vga_vblank(); //space.machine().first_screen()->vblank() & 1;

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

WRITE8_MEMBER(vga_device::vga_crtc_w)
{
	switch (offset)
	{
		case 4:
			vga.crtc.index = data;
			break;

		case 5:
			if (LOG_REGISTERS)
			{
				logerror("vga_crtc_w(): CRTC[0x%02X%s] = 0x%02X\n",
					vga.crtc.index,
					(vga.crtc.index < vga.svga_intf.crtc_regcount) ? "" : "?",
					data);
			}

			crtc_reg_write(vga.crtc.index,data);
			//space.machine().first_screen()->update_partial(space.machine().first_screen()->vpos());
			#if 0
			if((vga.crtc.index & 0xfe) != 0x0e)
				printf("%02x %02x %d\n",vga.crtc.index,data,space.machine().first_screen()->vpos());
			#endif
			break;

		case 0xa:
			vga.feature_control = data;
			break;
	}
}



READ8_MEMBER(vga_device::port_03b0_r)
{
	UINT8 data = 0xff;
	if (CRTC_PORT_ADDR==0x3b0)
		data=vga_crtc_r(space, offset, mem_mask);
	return data;
}

UINT8 vga_device::gc_reg_read(UINT8 index)
{
	UINT8 res;

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
			res |= (vga.gc.shift_reg & 1) << 5;;
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

READ8_MEMBER(vga_device::port_03c0_r)
{
	UINT8 data = 0xff;

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
			// TODO: in VGA bit 4 is actually always on?
			data = 0x60; // is VGA
			switch ((vga.miscellaneous_output>>2)&3)
			{
				case 3:
					if (!vga.read_dipswitch.isnull() && vga.read_dipswitch(space, 0, mem_mask) & 0x01)
						data |= 0x10;
					else
						data |= 0x10;
					break;
				case 2:
					if (!vga.read_dipswitch.isnull() && vga.read_dipswitch(space, 0, mem_mask) & 0x02)
						data |= 0x10;
					else
						data |= 0x10;
					break;
				case 1:
					if (!vga.read_dipswitch.isnull() && vga.read_dipswitch(space, 0, mem_mask) & 0x04)
						data |= 0x10;
					else
						data |= 0x10;
					break;
				case 0:
					if (!vga.read_dipswitch.isnull() && vga.read_dipswitch(space, 0, mem_mask) & 0x08)
						data |= 0x10;
					else
						data |= 0x10;
					break;
			}
			break;

		case 3:
			data = vga.oak.reg;
			break;

		case 4:
			data = vga.sequencer.index;
			break;

		case 5:
			if (vga.sequencer.index < vga.svga_intf.seq_regcount)
				data = vga.sequencer.data[vga.sequencer.index];
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
						data = vga.dac.color[vga.dac.read_index].red;
						break;
					case 1:
						data = vga.dac.color[vga.dac.read_index].green;
						break;
					case 2:
						data = vga.dac.color[vga.dac.read_index].blue;
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

READ8_MEMBER(vga_device::port_03d0_r)
{
	UINT8 data = 0xff;
	if (CRTC_PORT_ADDR == 0x3d0)
		data = vga_crtc_r(space, offset, mem_mask);
	if(offset == 8)
	{
		logerror("VGA: 0x3d8 read at %08x\n",space.device().safe_pc());
		data = 0; // TODO: PC-200 reads back CGA register here, everything else returns open bus OR CGA emulation of register 0x3d8
	}

	return data;
}

WRITE8_MEMBER(vga_device::port_03b0_w)
{
	if (LOG_ACCESSES)
		logerror("vga_port_03b0_w(): port=0x%04x data=0x%02x\n", offset + 0x3b0, data);

	if (CRTC_PORT_ADDR == 0x3b0)
		vga_crtc_w(space, offset, data, mem_mask);
}

void vga_device::attribute_reg_write(UINT8 index, UINT8 data)
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

void vga_device::gc_reg_write(UINT8 index,UINT8 data)
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

WRITE8_MEMBER(vga_device::port_03c0_w)
{
	if (LOG_ACCESSES)
		logerror("vga_port_03c0_w(): port=0x%04x data=0x%02x\n", offset + 0x3c0, data);

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
		if (LOG_REGISTERS)
		{
			logerror("vga_port_03c0_w(): SEQ[0x%02X%s] = 0x%02X\n",
				vga.sequencer.index,
				(vga.sequencer.index < vga.svga_intf.seq_regcount) ? "" : "?",
				data);
		}
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
				vga.dac.color[vga.dac.write_index].red=data;
				break;
			case 1:
				vga.dac.color[vga.dac.write_index].green=data;
				break;
			case 2:
				vga.dac.color[vga.dac.write_index].blue=data;
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



WRITE8_MEMBER(vga_device::port_03d0_w)
{
	if (LOG_ACCESSES)
		logerror("vga_port_03d0_w(): port=0x%04x data=0x%02x\n", offset + 0x3d0, data);

	if (CRTC_PORT_ADDR == 0x3d0)
		vga_crtc_w(space, offset, data, mem_mask);
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
	/* indiana.c boot PROM doesn't set this and assumes it's 0xff */
	vga.dac.mask = 0xff;
}

void s3_vga_device::device_reset()
{
	vga_device::device_reset();
	// Power-on strapping bits.  Sampled at reset, but can be modified later.
	// These are just assumed defaults.
	s3.strapping = 0x000f0b1e;
	s3.sr10 = 0x42;
	s3.sr11 = 0x41;
}

READ8_MEMBER(vga_device::mem_r)
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
		if (!space.debugger_access())
		{
			vga.gc.latch[0]=vga.memory[(offset)];
			vga.gc.latch[1]=vga.memory[(offset)+0x10000];
			vga.gc.latch[2]=vga.memory[(offset)+0x20000];
			vga.gc.latch[3]=vga.memory[(offset)+0x30000];
		}

		if (vga.gc.read_mode)
		{
			UINT8 byte,layer;
			UINT8 fill_latch;
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
		// TODO: Guesswork, probably not right
		UINT8 i,data;

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

WRITE8_MEMBER(vga_device::mem_w)
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
		UINT8 i;

		for(i=0;i<4;i++)
		{
			if(vga.sequencer.map_mask & 1 << i)
				vga.memory[offset+i*0x10000] = (vga.sequencer.data[4] & 4) ? vga_latch_write(i,data) : data;
		}
		return;
	}
}

READ8_MEMBER(vga_device::mem_linear_r)
{
	return vga.memory[offset];
}

WRITE8_MEMBER(vga_device::mem_linear_w)
{
	vga.memory[offset] = data;
}

MACHINE_CONFIG_FRAGMENT( pcvideo_vga )
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_RAW_PARAMS(XTAL_25_1748MHz,900,0,640,526,0,480)
	MCFG_SCREEN_UPDATE_DEVICE("vga", vga_device, screen_update)

	MCFG_PALETTE_ADD("palette", 0x100)
	MCFG_DEVICE_ADD("vga", VGA, 0)
MACHINE_CONFIG_END

MACHINE_CONFIG_FRAGMENT( pcvideo_trident_vga )
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_RAW_PARAMS(XTAL_25_1748MHz,900,0,640,526,0,480)
	MCFG_SCREEN_UPDATE_DEVICE("vga", trident_vga_device, screen_update)

	MCFG_PALETTE_ADD("palette", 0x100)
	MCFG_DEVICE_ADD("vga", TRIDENT_VGA, 0)
MACHINE_CONFIG_END

MACHINE_CONFIG_FRAGMENT( pcvideo_gamtor_vga )
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_RAW_PARAMS(XTAL_25_1748MHz,900,0,640,526,0,480)
	MCFG_SCREEN_UPDATE_DEVICE("vga", gamtor_vga_device, screen_update)

	MCFG_PALETTE_ADD("palette", 0x100)
	MCFG_DEVICE_ADD("vga", GAMTOR_VGA, 0)
MACHINE_CONFIG_END

MACHINE_CONFIG_FRAGMENT( pcvideo_s3_vga )
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_RAW_PARAMS(XTAL_25_1748MHz,900,0,640,526,0,480)
	MCFG_SCREEN_UPDATE_DEVICE("vga", s3_vga_device, screen_update)

	MCFG_PALETTE_ADD("palette", 0x100)
	MCFG_DEVICE_ADD("vga", S3_VGA, 0)
MACHINE_CONFIG_END


static MACHINE_CONFIG_FRAGMENT( ati_vga )
	MCFG_MACH8_ADD_OWNER("8514a")
	MCFG_EEPROM_SERIAL_93C46_ADD("ati_eeprom")
MACHINE_CONFIG_END

static MACHINE_CONFIG_FRAGMENT( s3_764 )
	MCFG_8514A_ADD_OWNER("8514a")
MACHINE_CONFIG_END

//-------------------------------------------------
//  machine_config_additions - device-specific
//  machine configurations
//-------------------------------------------------

machine_config_constructor ati_vga_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( ati_vga );
}

machine_config_constructor s3_vga_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( s3_764 );
}

/******************************************

Tseng ET4000k implementation

******************************************/

void tseng_vga_device::tseng_define_video_mode()
{
	int divisor;
	int xtal = 0;
	svga.rgb8_en = 0;
	svga.rgb15_en = 0;
	svga.rgb16_en = 0;
	svga.rgb24_en = 0;
	switch(((et4k.aux_ctrl << 1) & 4)|(vga.miscellaneous_output & 0xc)>>2)
	{
		case 0:
			xtal = XTAL_25_1748MHz;
			break;
		case 1:
			xtal = XTAL_28_63636MHz;
			break;
		case 2:
			xtal = 16257000*2; //2xEGA clock
			break;
		case 3:
			xtal = XTAL_40MHz;
			break;
		case 4:
			xtal = XTAL_36MHz;
			break;
		case 5:
			xtal = XTAL_45MHz;
			break;
		case 6:
			xtal = 31000000;
			break;
		case 7:
			xtal = 38000000;
			break;
	}
	switch(et4k.dac_ctrl & 0xe0)
	{
		case 0xa0:
			svga.rgb15_en = 1;
			divisor = 2;
			break;
		case 0xe0:
			svga.rgb16_en = 1;
			divisor = 2;
			break;
		case 0x60:
			svga.rgb24_en = 1;
			divisor = 3;
			xtal *= 2.0f/3.0f;
			break;
		default:
			svga.rgb8_en = (!(vga.sequencer.data[1] & 8) && (vga.sequencer.data[4] & 8) && vga.gc.shift256 && vga.crtc.div2 && GRAPHIC_MODE);
			divisor = 1;
			break;
	}
	recompute_params_clock(divisor, xtal);
}

UINT8 tseng_vga_device::tseng_crtc_reg_read(UINT8 index)
{
	UINT8 res;

	if(index <= 0x18)
		res = crtc_reg_read(index);
	else
	{
		switch(index)
		{
			case 0x34:
				res = et4k.aux_ctrl;
				break;
			case 0x3f:
				res = et4k.horz_overflow;
				break;
			default:
				res = vga.crtc.data[index];
				//printf("%02x\n",index);
				break;
		}
	}

	return res;
}

void tseng_vga_device::tseng_crtc_reg_write(UINT8 index, UINT8 data)
{
	if(index <= 0x18)
		crtc_reg_write(index,data);
	else
	{
		switch(index)
		{
			case 0x34:
				et4k.aux_ctrl = data;
				break;
			case 0x3f:
				et4k.horz_overflow = data;
				vga.crtc.horz_total = (vga.crtc.horz_total & 0xff) | ((data & 1) << 8);
				break;
			default:
				//printf("%02x %02x\n",index,data);
				break;
		}
	}
}

UINT8 tseng_vga_device::tseng_seq_reg_read(UINT8 index)
{
	UINT8 res;

	res = 0xff;

	if(index <= 0x04)
		res = vga.sequencer.data[index];
	else
	{
		switch(index)
		{
			case 0x06:
			case 0x07:
				//printf("%02x\n",index);
				break;
		}
	}

	return res;
}

void tseng_vga_device::tseng_seq_reg_write(UINT8 index, UINT8 data)
{
	if(index <= 0x04)
	{
		vga.sequencer.data[vga.sequencer.index] = data;
		seq_reg_write(vga.sequencer.index,data);
	}
	else
	{
		switch(index)
		{
			case 0x06:
			case 0x07:
				//printf("%02x %02x\n",index,data);
				break;
		}
	}
}

READ8_MEMBER(tseng_vga_device::port_03b0_r)
{
	UINT8 res = 0xff;

	if (CRTC_PORT_ADDR == 0x3b0)
	{
		switch(offset)
		{
			case 5:
				res = tseng_crtc_reg_read(vga.crtc.index);
				break;
			case 8:
				res = et4k.reg_3d8;
				break;
			default:
				res = vga_device::port_03b0_r(space,offset,mem_mask);
				break;
		}
	}

	return res;
}

WRITE8_MEMBER(tseng_vga_device::port_03b0_w)
{
	if (CRTC_PORT_ADDR == 0x3b0)
	{
		switch(offset)
		{
			case 5:
				vga.crtc.data[vga.crtc.index] = data;
				tseng_crtc_reg_write(vga.crtc.index,data);
				break;
			case 8:
				et4k.reg_3d8 = data;
				if(data == 0xa0)
					et4k.ext_reg_ena = true;
				else if(data == 0x29)
					et4k.ext_reg_ena = false;
				break;
			default:
				vga_device::port_03b0_w(space,offset,data,mem_mask);
				break;
		}
	}
	tseng_define_video_mode();
}

void tseng_vga_device::tseng_attribute_reg_write(UINT8 index, UINT8 data)
{
	switch(index)
	{
		case 0x16:
			et4k.misc1 = data;
			#if 0
			svga.rgb8_en = 0;
			svga.rgb15_en = 0;
			svga.rgb16_en = 0;
			svga.rgb32_en = 0;
			/* TODO: et4k and w32 are different here */
			switch(et4k.misc1 & 0x30)
			{
				case 0:
					// normal power-up mode
					break;
				case 0x10:
					svga.rgb8_en = 1;
					break;
				case 0x20:
				case 0x30:
					popmessage("Tseng 15/16 bit HiColor mode, contact MAMEdev");
					break;
			}
			#endif
			break;
		case 0x17: et4k.misc2 = data; break;
		default:
			attribute_reg_write(index,data);
	}

}

READ8_MEMBER(tseng_vga_device::port_03c0_r)
{
	UINT8 res;

	switch(offset)
	{
		case 0x01:
			switch(vga.attribute.index)
			{
				case 0x16: res = et4k.misc1; break;
				case 0x17: res = et4k.misc2; break;
				default:
					res = vga_device::port_03c0_r(space,offset,mem_mask);
					break;
			}

			break;

		case 0x05:
			res = tseng_seq_reg_read(vga.sequencer.index);
			break;
		case 0x0d:
			res = svga.bank_w & 0xf;
			res |= (svga.bank_r & 0xf) << 4;
			break;
		case 0x06:
			if(et4k.dac_state == 4)
			{
				if(!et4k.dac_ctrl)
					et4k.dac_ctrl = 0x80;
				res = et4k.dac_ctrl;
				break;
			}
			et4k.dac_state++;
			res = vga_device::port_03c0_r(space,offset,mem_mask);
			break;
		case 0x08:
			et4k.dac_state = 0;
		default:
			res = vga_device::port_03c0_r(space,offset,mem_mask);
			break;
	}

	return res;
}

WRITE8_MEMBER(tseng_vga_device::port_03c0_w)
{
	switch(offset)
	{
		case 0:
			if (vga.attribute.state==0)
			{
				vga.attribute.index=data;
			}
			else
			{
				tseng_attribute_reg_write(vga.attribute.index,data);
			}
			vga.attribute.state=!vga.attribute.state;
			break;

		case 0x05:
			tseng_seq_reg_write(vga.sequencer.index,data);
			break;
		case 0x0d:
			svga.bank_w = data & 0xf;
			svga.bank_r = (data & 0xf0) >> 4;
			break;
		case 0x06:
			if(et4k.dac_state == 4)
			{
				et4k.dac_ctrl = data;
				break;
			}
		default:
			vga_device::port_03c0_w(space,offset,data,mem_mask);
			break;
	}
	tseng_define_video_mode();
}

READ8_MEMBER(tseng_vga_device::port_03d0_r)
{
	UINT8 res = 0xff;

	if (CRTC_PORT_ADDR == 0x3d0)
	{
		switch(offset)
		{
			case 5:
				res = tseng_crtc_reg_read(vga.crtc.index);
				break;
			case 8:
				res = et4k.reg_3d8;
				break;
			default:
				res = vga_device::port_03d0_r(space,offset,mem_mask);
				break;
		}
	}

	return res;
}

WRITE8_MEMBER(tseng_vga_device::port_03d0_w)
{
	if (CRTC_PORT_ADDR == 0x3d0)
	{
		switch(offset)
		{
			case 5:
				vga.crtc.data[vga.crtc.index] = data;
				tseng_crtc_reg_write(vga.crtc.index,data);
				//if((vga.crtc.index & 0xfe) != 0x0e)
				//  printf("%02x %02x %d\n",vga.crtc.index,data,space.machine().first_screen()->vpos());
				break;
			case 8:
				et4k.reg_3d8 = data;
				if(data == 0xa0)
					et4k.ext_reg_ena = true;
				else if(data == 0x29)
					et4k.ext_reg_ena = false;
				break;
			default:
				vga_device::port_03d0_w(space,offset,data,mem_mask);
				break;
		}
	}
	tseng_define_video_mode();
}

READ8_MEMBER(tseng_vga_device::mem_r)
{
	if(svga.rgb8_en || svga.rgb15_en || svga.rgb16_en || svga.rgb24_en)
	{
		offset &= 0xffff;
		return vga.memory[(offset+svga.bank_r*0x10000)];
	}

	return vga_device::mem_r(space,offset,mem_mask);
}

WRITE8_MEMBER(tseng_vga_device::mem_w)
{
	if(svga.rgb8_en || svga.rgb15_en || svga.rgb16_en || svga.rgb24_en)
	{
		offset &= 0xffff;
		vga.memory[(offset+svga.bank_w*0x10000)] = data;
	}
	else
		vga_device::mem_w(space,offset,data,mem_mask);
}

/******************************************

S3 implementation

******************************************/

UINT16 s3_vga_device::offset()
{
	//popmessage("Offset: %04x  %s %s %s",vga.crtc.offset,vga.crtc.dw?"DW":"--",vga.crtc.word_mode?"BYTE":"WORD",(s3.memory_config & 0x08)?"31":"--");
	if(s3.memory_config & 0x08)
		return vga.crtc.offset << 3;
	return vga_device::offset();
}

UINT8 s3_vga_device::s3_crtc_reg_read(UINT8 index)
{
	UINT8 res;

	if(index <= 0x18)
		res = crtc_reg_read(index);
	else
	{
		switch(index)
		{
			case 0x2d:
				res = s3.id_high;
				break;
			case 0x2e:
				res = s3.id_low;
				break;
			case 0x2f:
				res = s3.revision;
				break;
			case 0x30: // CR30 Chip ID/REV register
				res = s3.id_cr30;
				break;
			case 0x31:
				res = s3.memory_config;
				break;
			case 0x35:
				res = s3.crt_reg_lock;
				break;
			case 0x36:  // Configuration register 1
				res = s3.strapping & 0x000000ff;  // PCI (not really), Fast Page Mode DRAM
				break;
			case 0x37:  // Configuration register 2
				res = (s3.strapping & 0x0000ff00) >> 8;  // enable chipset, 64k BIOS size, internal DCLK/MCLK
				break;
			case 0x38:
				res = s3.reg_lock1;
				break;
			case 0x39:
				res = s3.reg_lock2;
				break;
			case 0x42: // CR42 Mode Control
				res = s3.cr42 & 0x0f;  // bit 5 set if interlaced, leave it unset for now.
				break;
			case 0x43:
				res = s3.cr43;
				break;
			case 0x45:
				res = s3.cursor_mode;
				break;
			case 0x46:
				res = (s3.cursor_x & 0xff00) >> 8;
				break;
			case 0x47:
				res = s3.cursor_x & 0x00ff;
				break;
			case 0x48:
				res = (s3.cursor_y & 0xff00) >> 8;
				break;
			case 0x49:
				res = s3.cursor_y & 0x00ff;
				break;
			case 0x4a:
				res = s3.cursor_fg[s3.cursor_fg_ptr];
				s3.cursor_fg_ptr = 0;
				break;
			case 0x4b:
				res = s3.cursor_bg[s3.cursor_bg_ptr];
				s3.cursor_bg_ptr = 0;
				break;
			case 0x4c:
				res = (s3.cursor_start_addr & 0xff00) >> 8;
				break;
			case 0x4d:
				res = s3.cursor_start_addr & 0x00ff;
				break;
			case 0x4e:
				res = s3.cursor_pattern_x;
				break;
			case 0x4f:
				res = s3.cursor_pattern_y;
				break;
			case 0x51:
				res = (vga.crtc.start_addr_latch & 0x0c0000) >> 18;
				res |= ((svga.bank_w & 0x30) >> 2);
				res |= ((vga.crtc.offset & 0x0300) >> 4);
				break;
			case 0x55:
				res = s3.extended_dac_ctrl;
				break;
			case 0x5c:
				// if VGA dot clock is set to 3 (misc reg bits 2-3), then selected dot clock is read, otherwise read VGA clock select
				if((vga.miscellaneous_output & 0xc) == 0x0c)
					res = s3.cr42 & 0x0f;
				else
					res = (vga.miscellaneous_output & 0xc) >> 2;
				break;
			case 0x67:
				res = s3.ext_misc_ctrl_2;
				break;
			case 0x68:  // Configuration register 3
				res = (s3.strapping & 0x00ff0000) >> 16;  // no /CAS,/OE stretch time, 32-bit data bus size
				break;
			case 0x69:
				res = vga.crtc.start_addr_latch >> 16;
				break;
			case 0x6a:
				res = svga.bank_r & 0x7f;
				break;
			case 0x6f: // Configuration register 4 (Trio64V+)
				res = (s3.strapping & 0xff000000) >> 24;  // LPB(?) mode, Serial port I/O at port 0xe8, Serial port I/O disabled (MMIO only), no WE delay
				break;
			default:
				res = vga.crtc.data[index];
				//debugger_break(machine);
				//printf("%02x\n",index);
				break;
		}
	}

	return res;
}

void s3_vga_device::s3_define_video_mode()
{
	int divisor = 1;
	int xtal = (vga.miscellaneous_output & 0xc) ? XTAL_28_63636MHz : XTAL_25_1748MHz;
	double freq;

	if((vga.miscellaneous_output & 0xc) == 0x0c)
	{
		// DCLK calculation
		freq = ((double)(s3.clk_pll_m+2) / (double)((s3.clk_pll_n+2)*(pow(2.0,s3.clk_pll_r)))) * 14.318; // clock between XIN and XOUT
		xtal = freq * 1000000;
	}

	if((s3.ext_misc_ctrl_2) >> 4)
	{
		svga.rgb8_en = 0;
		svga.rgb15_en = 0;
		svga.rgb16_en = 0;
		svga.rgb32_en = 0;
		switch((s3.ext_misc_ctrl_2) >> 4)
		{
			case 0x01: svga.rgb8_en = 1; break;
			case 0x03: svga.rgb15_en = 1; divisor = 2; break;
			case 0x05: svga.rgb16_en = 1; divisor = 2; break;
			case 0x0d: svga.rgb32_en = 1; divisor = 1; break;
			default: fatalerror("TODO: S3 colour mode not implemented %02x\n",((s3.ext_misc_ctrl_2) >> 4));
		}
	}
	else
	{
		svga.rgb8_en = (s3.memory_config & 8) >> 3;
		svga.rgb15_en = 0;
		svga.rgb16_en = 0;
		svga.rgb32_en = 0;
	}
	recompute_params_clock(divisor, xtal);
}

void s3_vga_device::s3_crtc_reg_write(UINT8 index, UINT8 data)
{
	if(index <= 0x18)
		crtc_reg_write(index,data);
	else
	{
		switch(index)
		{
			case 0x31: // CR31 Memory Configuration Register
				s3.memory_config = data;
				vga.crtc.start_addr_latch &= ~0x30000;
				vga.crtc.start_addr_latch |= ((data & 0x30) << 12);
				s3_define_video_mode();
				break;
			case 0x35:
				if((s3.reg_lock1 & 0xc) != 8 || ((s3.reg_lock1 & 0xc0) == 0)) // lock register
					return;
				s3.crt_reg_lock = data;
				svga.bank_w = data & 0xf;
				svga.bank_r = svga.bank_w;
				break;
			case 0x36:
				if(s3.reg_lock2 == 0xa5)
				{
					s3.strapping = (s3.strapping & 0xffffff00) | data;
					logerror("CR36: Strapping data = %08x\n",s3.strapping);
				}
				break;
			case 0x37:
				if(s3.reg_lock2 == 0xa5)
				{
					s3.strapping = (s3.strapping & 0xffff00ff) | (data << 8);
					logerror("CR37: Strapping data = %08x\n",s3.strapping);
				}
				break;
			case 0x38:
				s3.reg_lock1 = data;
				break;
			case 0x39:
				/* TODO: reg lock mechanism */
				s3.reg_lock2 = data;
				break;
			case 0x40:
				s3.enable_8514 = data & 0x01;  // enable 8514/A registers (x2e8, x6e8, xae8, xee8)
				break;
			case 0x42:
				s3.cr42 = data;  // bit 5 = interlace, bits 0-3 = dot clock (seems to be undocumented)
				break;
			case 0x43:
				s3.cr43 = data;  // bit 2 = bit 8 of offset register, but only if bits 4-5 of CR51 are 00h.
				vga.crtc.offset = (vga.crtc.offset & 0x00ff) | ((data & 0x04) << 6);
				s3_define_video_mode();
				break;
/*
3d4h index 45h (R/W):  CR45 Hardware Graphics Cursor Mode
bit    0  HWGC ENB. Hardware Graphics Cursor Enable. Set to enable the
          HardWare Cursor in VGA and enhanced modes.
       1  (911/24) Delay Timing for Pattern Data Fetch
       2  (801/5,928) Hardware Cursor Horizontal Stretch 2. If set the cursor
           pixels are stretched horizontally to two bytes and items 0 and 1 of
           the fore/background stacks in 3d4h index 4Ah/4Bh are used.
       3  (801/5,928) Hardware Cursor Horizontal Stretch 3. If set the cursor
           pixels are stretched horizontally to three bytes and items 0,1 and
           2 of the fore/background stacks in 3d4h index 4Ah/4Bh are used.
     2-3  (805i,864/964) HWC-CSEL. Hardware Cursor Color Select.
            0: 4/8bit, 1: 15/16bt, 2: 24bit, 3: 32bit
          Note: So far I've had better luck with: 0: 8/15/16bit, 1: 32bit??
       4  (80x +) Hardware Cursor Right Storage. If set the cursor data is
           stored in the last 256 bytes of 4 1Kyte lines (4bits/pixel) or the
           last 512 bytes of 2 2Kbyte lines (8bits/pixel). Intended for
           1280x1024 modes where there are no free lines at the bottom.
       5  (928) Cursor Control Enable for Brooktree Bt485 DAC. If set and 3d4h
           index 55h bit 5 is set the HC1 output becomes the ODF and the HC0
           output becomes the CDE
          (964) BT485 ODF Selection for Bt485A RAMDAC. If set pin 185 (RS3
           /ODF) is the ODF output to a Bt485A compatible RamDAC (low for even
           fields and high for odd fields), if clear pin185 is the RS3 output.
 */
			case 0x45:
				s3.cursor_mode = data;
				break;
/*
3d4h index 46h M(R/W):  CR46/7 Hardware Graphics Cursor Origin-X
bit 0-10  The HardWare Cursor X position. For 64k modes this value should be
          twice the actual X co-ordinate.
 */
			case 0x46:
				s3.cursor_x = (s3.cursor_x & 0x00ff) | (data << 8);
				break;
			case 0x47:
				s3.cursor_x = (s3.cursor_x & 0xff00) | data;
				break;
/*
3d4h index 48h M(R/W):  CR48/9 Hardware Graphics Cursor Origin-Y
bit  0-9  (911/24) The HardWare Cursor Y position.
    0-10  (80x +) The HardWare Cursor Y position.
Note: The position is activated when the high byte of the Y coordinate (index
      48h) is written, so this byte should be written last (not 911/924 ?)
 */
			case 0x48:
				s3.cursor_y = (s3.cursor_y & 0x00ff) | (data << 8);
				break;
			case 0x49:
				s3.cursor_y = (s3.cursor_y & 0xff00) | data;
				break;

/*
3d4h index 4Ah (R/W):  Hardware Graphics Cursor Foreground Stack       (80x +)
bit  0-7  The Foreground Cursor color. Three bytes (4 for the 864/964) are
          stacked here. When the Cursor Mode register (3d4h index 45h) is read
          the stackpointer is reset. When a byte is written the byte is
          written into the current top of stack and the stackpointer is
          increased. The first byte written (item 0) is allways used, the
          other two(3) only when Hardware Cursor Horizontal Stretch (3d4h
          index 45h bit 2-3) is enabled.
 */
			case 0x4a:
				s3.cursor_fg[s3.cursor_fg_ptr++] = data;
				s3.cursor_fg_ptr %= 4;
				break;
/*
3d4h index 4Bh (R/W):  Hardware Graphics Cursor Background Stack       (80x +)
bit  0-7  The Background Cursor color. Three bytes (4 for the 864/964) are
          stacked here. When the Cursor Mode register (3d4h index 45h) is read
          the stackpointer is reset. When a byte is written the byte is
          written into the current top of stack and the stackpointer is
          increased. The first byte written (item 0) is allways used, the
          other two(3) only when Hardware Cursor Horizontal Stretch (3d4h
          index 45h bit 2-3) is enabled.
 */
			case 0x4b:
				s3.cursor_bg[s3.cursor_bg_ptr++] = data;
				s3.cursor_bg_ptr %= 4;
				break;
/*
3d4h index 4Ch M(R/W):  CR4C/D Hardware Graphics Cursor Storage Start Address
bit  0-9  (911,924) HCS_STADR. Hardware Graphics Cursor Storage Start Address
    0-11  (80x,928) HWGC_STA. Hardware Graphics Cursor Storage Start Address
    0-12  (864,964) HWGC_STA. Hardware Graphics Cursor Storage Start Address
          Address of the HardWare Cursor Map in units of 1024 bytes (256 bytes
          for planar modes). The cursor map is a 64x64 bitmap with 2 bits (A
          and B) per pixel. The map is stored as one word (16 bits) of bit A,
          followed by one word with the corresponding 16 B bits.
          The bits are interpreted as:
             A    B    MS-Windows:         X-11:
             0    0    Background          Screen data
             0    1    Foreground          Screen data
             1    0    Screen data         Background
             1    1    Inverted screen     Foreground
          The Windows/X11 switch is only available for the 80x +.
          (911/24) For 64k color modes the cursor is stored as one byte (8
            bits) of A bits, followed by the 8 B-bits, and each bit in the
            cursor should be doubled to provide a consistent cursor image.
          (801/5,928) For Hi/True color modes use the Horizontal Stretch bits
            (3d4h index 45h bits 2 and 3).
 */
			case 0x4c:
				s3.cursor_start_addr = (s3.cursor_start_addr & 0x00ff) | (data << 8);
				break;
			case 0x4d:
				s3.cursor_start_addr = (s3.cursor_start_addr & 0xff00) | data;
				break;
/*
3d4h index 4Eh (R/W):  CR4E HGC Pattern Disp Start X-Pixel Position
bit  0-5  Pattern Display Start X-Pixel Position.
 */
			case 0x4e:
				s3.cursor_pattern_x = data;
				break;
/*
3d4h index 4Fh (R/W):  CR4F HGC Pattern Disp Start Y-Pixel Position
bit  0-5  Pattern Display Start Y-Pixel Position.
 */
			case 0x4f:
				s3.cursor_pattern_y = data;
				break;
			case 0x51:
				vga.crtc.start_addr_latch &= ~0xc0000;
				vga.crtc.start_addr_latch |= ((data & 0x3) << 18);
				svga.bank_w = (svga.bank_w & 0xcf) | ((data & 0x0c) << 2);
				svga.bank_r = svga.bank_w;
				if((data & 0x30) != 0x00)
					vga.crtc.offset = (vga.crtc.offset & 0x00ff) | ((data & 0x30) << 4);
				else
					vga.crtc.offset = (vga.crtc.offset & 0x00ff) | ((s3.cr43 & 0x04) << 6);
				s3_define_video_mode();
				break;
			case 0x53:
				s3.cr53 = data;
				break;
/*
3d4h index 55h (R/W):  Extended Video DAC Control Register             (80x +)
bit 0-1  DAC Register Select Bits. Passed to the RS2 and RS3 pins on the
         RAMDAC, allowing access to all 8 or 16 registers on advanced RAMDACs.
         If this field is 0, 3d4h index 43h bit 1 is active.
      2  Enable General Input Port Read. If set DAC reads are disabled and the
         STRD strobe for reading the General Input Port is enabled for reading
         while DACRD is active, if clear DAC reads are enabled.
      3  (928) Enable External SID Operation if set. If set video data is
           passed directly from the VRAMs to the DAC rather than through the
           VGA chip
      4  Hardware Cursor MS/X11 Mode. If set the Hardware Cursor is in X11
         mode, if clear in MS-Windows mode
      5  (80x,928) Hardware Cursor External Operation Mode. If set the two
          bits of cursor data ,is output on the HC[0-1] pins for the video DAC
          The SENS pin becomes HC1 and the MID2 pin becomes HC0.
      6  ??
      7  (80x,928) Disable PA Output. If set PA[0-7] and VCLK are tristated.
         (864/964) TOFF VCLK. Tri-State Off VCLK Output. VCLK output tri
          -stated if set
 */
			case 0x55:
				s3.extended_dac_ctrl = data;
				break;
/*
3d4h index 5Dh (R/W):  Extended Horizontal Overflow Register           (80x +)
bit    0  Horizontal Total bit 8. Bit 8 of the Horizontal Total register (3d4h
          index 0)
       1  Horizontal Display End bit 8. Bit 8 of the Horizontal Display End
          register (3d4h index 1)
       2  Start Horizontal Blank bit 8. Bit 8 of the Horizontal Start Blanking
          register (3d4h index 2).
       3  (864,964) EHB+64. End Horizontal Blank +64. If set the /BLANK pulse
           is extended by 64 DCLKs. Note: Is this bit 6 of 3d4h index 3 or
           does it really extend by 64 ?
       4  Start Horizontal Sync Position bit 8. Bit 8 of the Horizontal Start
          Retrace register (3d4h index 4).
       5  (864,964) EHS+32. End Horizontal Sync +32. If set the HSYNC pulse
           is extended by 32 DCLKs. Note: Is this bit 5 of 3d4h index 5 or
           does it really extend by 32 ?
       6  (928,964) Data Transfer Position bit 8. Bit 8 of the Data Transfer
            Position register (3d4h index 3Bh)
       7  (928,964) Bus-Grant Terminate Position bit 8. Bit 8 of the Bus Grant
            Termination register (3d4h index 5Fh).
*/
			case 0x5d:
				vga.crtc.horz_total = (vga.crtc.horz_total & 0xfeff) | ((data & 0x01) << 8);
				vga.crtc.horz_disp_end = (vga.crtc.horz_disp_end & 0xfeff) | ((data & 0x02) << 7);
				vga.crtc.horz_blank_start = (vga.crtc.horz_blank_start & 0xfeff) | ((data & 0x04) << 6);
				vga.crtc.horz_blank_end = (vga.crtc.horz_blank_end & 0xffbf) | ((data & 0x08) << 3);
				vga.crtc.horz_retrace_start = (vga.crtc.horz_retrace_start & 0xfeff) | ((data & 0x10) << 4);
				vga.crtc.horz_retrace_end = (vga.crtc.horz_retrace_end & 0xffdf) | (data & 0x20);
				s3_define_video_mode();
				break;
/*
3d4h index 5Eh (R/W):  Extended Vertical Overflow Register             (80x +)
bit    0  Vertical Total bit 10. Bit 10 of the Vertical Total register (3d4h
          index 6). Bits 8 and 9 are in 3d4h index 7 bit 0 and 5.
       1  Vertical Display End bit 10. Bit 10 of the Vertical Display End
          register (3d4h index 12h). Bits 8 and 9 are in 3d4h index 7 bit 1
          and 6
       2  Start Vertical Blank bit 10. Bit 10 of the Vertical Start Blanking
          register (3d4h index 15h). Bit 8 is in 3d4h index 7 bit 3 and bit 9
          in 3d4h index 9 bit 5
       4  Vertical Retrace Start bit 10. Bit 10 of the Vertical Start Retrace
          register (3d4h index 10h). Bits 8 and 9 are in 3d4h index 7 bit 2
          and 7.
       6  Line Compare Position bit 10. Bit 10 of the Line Compare register
          (3d4h index 18h). Bit 8 is in 3d4h index 7 bit 4 and bit 9 in 3d4h
          index 9 bit 6.
 */
			case 0x5e:
				vga.crtc.vert_total = (vga.crtc.vert_total & 0xfbff) | ((data & 0x01) << 10);
				vga.crtc.vert_disp_end = (vga.crtc.vert_disp_end & 0xfbff) | ((data & 0x02) << 9);
				vga.crtc.vert_blank_start = (vga.crtc.vert_blank_start & 0xfbff) | ((data & 0x04) << 8);
				vga.crtc.vert_retrace_start = (vga.crtc.vert_retrace_start & 0xfbff) | ((data & 0x10) << 6);
				vga.crtc.line_compare = (vga.crtc.line_compare & 0xfbff) | ((data & 0x40) << 4);
				s3_define_video_mode();
				break;
			case 0x67:
				s3.ext_misc_ctrl_2 = data;
				s3_define_video_mode();
				break;
			case 0x68:
				if(s3.reg_lock2 == 0xa5)
				{
					s3.strapping = (s3.strapping & 0xff00ffff) | (data << 16);
					logerror("CR68: Strapping data = %08x\n",s3.strapping);
				}
				break;
			case 0x69:
				vga.crtc.start_addr_latch &= ~0x1f0000;
				vga.crtc.start_addr_latch |= ((data & 0x1f) << 16);
				s3_define_video_mode();
				break;
			case 0x6a:
				svga.bank_w = data & 0x3f;
				svga.bank_r = svga.bank_w;
				break;
			case 0x6f:
				if(s3.reg_lock2 == 0xa5)
				{
					s3.strapping = (s3.strapping & 0x00ffffff) | (data << 24);
					logerror("CR6F: Strapping data = %08x\n",s3.strapping);
				}
				break;
			default:
				if(LOG_8514) logerror("S3: 3D4 index %02x write %02x\n",index,data);
				break;
		}
	}
}

UINT8 s3_vga_device::s3_seq_reg_read(UINT8 index)
{
	UINT8 res = 0xff;

	if(index <= 0x0c)
		res = vga.sequencer.data[index];
	else
	{
		switch(index)
		{
		case 0x10:
			res = s3.sr10;
			break;
		case 0x11:
			res = s3.sr11;
			break;
		case 0x12:
			res = s3.sr12;
			break;
		case 0x13:
			res = s3.sr13;
			break;
		case 0x15:
			res = s3.sr15;
			break;
		case 0x17:
			res = s3.sr17;  // CLKSYN test register
			s3.sr17--;  // who knows what it should return, docs only say it defaults to 0, and is reserved for testing of the clock synthesiser
			break;
		}
	}

	return res;
}

void s3_vga_device::s3_seq_reg_write(UINT8 index, UINT8 data)
{
	if(index <= 0x0c)
	{
		vga.sequencer.data[vga.sequencer.index] = data;
		seq_reg_write(vga.sequencer.index,data);
	}
	else
	{
		switch(index)
		{
		// Memory CLK PLL
		case 0x10:
			s3.sr10 = data;
			break;
		case 0x11:
			s3.sr11 = data;
			break;
		// Video CLK PLL
		case 0x12:
			s3.sr12 = data;
			break;
		case 0x13:
			s3.sr13 = data;
			break;
		case 0x15:
			if(data & 0x02)  // load DCLK frequency (would normally have a small variable delay)
			{
				s3.clk_pll_n = s3.sr12 & 0x1f;
				s3.clk_pll_r = (s3.sr12 & 0x60) >> 5;
				s3.clk_pll_m = s3.sr13 & 0x7f;
				s3_define_video_mode();
			}
			if(data & 0x20)  // immediate DCLK/MCLK load
			{
				s3.clk_pll_n = s3.sr12 & 0x1f;
				s3.clk_pll_r = (s3.sr12 & 0x60) >> 5;
				s3.clk_pll_m = s3.sr13 & 0x7f;
				s3_define_video_mode();
			}
			s3.sr15 = data;
		}
	}
}



READ8_MEMBER(s3_vga_device::port_03b0_r)
{
	UINT8 res = 0xff;

	if (CRTC_PORT_ADDR == 0x3b0)
	{
		switch(offset)
		{
			case 5:
				res = s3_crtc_reg_read(vga.crtc.index);
				break;
			default:
				res = vga_device::port_03b0_r(space,offset,mem_mask);
				break;
		}
	}

	return res;
}

WRITE8_MEMBER(s3_vga_device::port_03b0_w)
{
	if (CRTC_PORT_ADDR == 0x3b0)
	{
		switch(offset)
		{
			case 5:
				vga.crtc.data[vga.crtc.index] = data;
				s3_crtc_reg_write(vga.crtc.index,data);
				break;
			default:
				vga_device::port_03b0_w(space,offset,data,mem_mask);
				break;
		}
	}
}

READ8_MEMBER(s3_vga_device::port_03c0_r)
{
	UINT8 res;

	switch(offset)
	{
		case 5:
			res = s3_seq_reg_read(vga.sequencer.index);
			break;
		default:
			res = vga_device::port_03c0_r(space,offset,mem_mask);
			break;
	}

	return res;
}

WRITE8_MEMBER(s3_vga_device::port_03c0_w)
{
	switch(offset)
	{
		case 5:
			s3_seq_reg_write(vga.sequencer.index,data);
			break;
		default:
			vga_device::port_03c0_w(space,offset,data,mem_mask);
			break;
	}
}

READ8_MEMBER(s3_vga_device::port_03d0_r)
{
	UINT8 res = 0xff;

	if (CRTC_PORT_ADDR == 0x3d0)
	{
		switch(offset)
		{
			case 5:
				res = s3_crtc_reg_read(vga.crtc.index);
				break;
			default:
				res = vga_device::port_03d0_r(space,offset,mem_mask);
				break;
		}
	}

	return res;
}

WRITE8_MEMBER(s3_vga_device::port_03d0_w)
{
	if (CRTC_PORT_ADDR == 0x3d0)
	{
		switch(offset)
		{
			case 5:
				vga.crtc.data[vga.crtc.index] = data;
				s3_crtc_reg_write(vga.crtc.index,data);
				break;
			default:
				vga_device::port_03d0_w(space,offset,data,mem_mask);
				break;
		}
	}
}

READ8_MEMBER(ati_vga_device::port_03c0_r)
{
	UINT8 data = 0xff;

	switch(offset)
	{
	case 1:
		if ((vga.attribute.index&0x1f) < sizeof(vga.attribute.data))
			data = vga.attribute.data[vga.attribute.index&0x1f];
		break;
	default:
		data = vga_device::port_03c0_r(space,offset,mem_mask);
		break;
	}
	return data;
}


/* accelerated ports, TBD ... */

void ibm8514a_device::ibm8514_write_fg(UINT32 offset)
{
	address_space& space = machine().device("maincpu")->memory().space(AS_PROGRAM);
	offset %= m_vga->vga.svga_intf.vram_size;
	UINT8 dst = m_vga->mem_linear_r(space,offset,0xff);
	UINT8 src = 0;

	// check clipping rectangle
	if((ibm8514.current_cmd & 0xe000) == 0xc000)  // BitBLT writes to the destination X/Y, so check that instead
	{
		if(ibm8514.dest_x < ibm8514.scissors_left || ibm8514.dest_x > ibm8514.scissors_right || ibm8514.dest_y < ibm8514.scissors_top || ibm8514.dest_y > ibm8514.scissors_bottom)
			return;  // do nothing
	}
	else
		if(ibm8514.curr_x < ibm8514.scissors_left || ibm8514.curr_x > ibm8514.scissors_right || ibm8514.curr_y < ibm8514.scissors_top || ibm8514.curr_y > ibm8514.scissors_bottom)
			return;  // do nothing

	// determine source
	switch(ibm8514.fgmix & 0x0060)
	{
	case 0x0000:
		src = ibm8514.bgcolour;
		break;
	case 0x0020:
		src = ibm8514.fgcolour;
		break;
	case 0x0040:
		src = ibm8514.pixel_xfer;
		break;
	case 0x0060:
		// video memory - presume the memory is sourced from the current X/Y co-ords
		src = m_vga->mem_linear_r(space,((ibm8514.curr_y * IBM8514_LINE_LENGTH) + ibm8514.curr_x),0xff);
		break;
	}

	// write the data
	switch(ibm8514.fgmix & 0x000f)
	{
	case 0x0000:
		m_vga->mem_linear_w(space,offset,~dst,0xff);
		break;
	case 0x0001:
		m_vga->mem_linear_w(space,offset,0x00,0xff);
		break;
	case 0x0002:
		m_vga->mem_linear_w(space,offset,0xff,0xff);
		break;
	case 0x0003:
		m_vga->mem_linear_w(space,offset,dst,0xff);
		break;
	case 0x0004:
		m_vga->mem_linear_w(space,offset,~src,0xff);
		break;
	case 0x0005:
		m_vga->mem_linear_w(space,offset,src ^ dst,0xff);
		break;
	case 0x0006:
		m_vga->mem_linear_w(space,offset,~(src ^ dst),0xff);
		break;
	case 0x0007:
		m_vga->mem_linear_w(space,offset,src,0xff);
		break;
	case 0x0008:
		m_vga->mem_linear_w(space,offset,~(src & dst),0xff);
		break;
	case 0x0009:
		m_vga->mem_linear_w(space,offset,(~src) | dst,0xff);
		break;
	case 0x000a:
		m_vga->mem_linear_w(space,offset,src | (~dst),0xff);
		break;
	case 0x000b:
		m_vga->mem_linear_w(space,offset,src | dst,0xff);
		break;
	case 0x000c:
		m_vga->mem_linear_w(space,offset,src & dst,0xff);
		break;
	case 0x000d:
		m_vga->mem_linear_w(space,offset,src & (~dst),0xff);
		break;
	case 0x000e:
		m_vga->mem_linear_w(space,offset,(~src) & dst,0xff);
		break;
	case 0x000f:
		m_vga->mem_linear_w(space,offset,~(src | dst),0xff);
		break;
	}
}

void ibm8514a_device::ibm8514_write_bg(UINT32 offset)
{
	address_space& space = machine().device("maincpu")->memory().space(AS_PROGRAM);
	offset %= m_vga->vga.svga_intf.vram_size;
	UINT8 dst = m_vga->mem_linear_r(space,offset,0xff);
	UINT8 src = 0;

	// check clipping rectangle
	if((ibm8514.current_cmd & 0xe000) == 0xc000)  // BitBLT writes to the destination X/Y, so check that instead
	{
		if(ibm8514.dest_x < ibm8514.scissors_left || ibm8514.dest_x > ibm8514.scissors_right || ibm8514.dest_y < ibm8514.scissors_top || ibm8514.dest_y > ibm8514.scissors_bottom)
			return;  // do nothing
	}
	else
		if(ibm8514.curr_x < ibm8514.scissors_left || ibm8514.curr_x > ibm8514.scissors_right || ibm8514.curr_y < ibm8514.scissors_top || ibm8514.curr_y > ibm8514.scissors_bottom)
			return;  // do nothing

	// determine source
	switch(ibm8514.bgmix & 0x0060)
	{
	case 0x0000:
		src = ibm8514.bgcolour;
		break;
	case 0x0020:
		src = ibm8514.fgcolour;
		break;
	case 0x0040:
		src = ibm8514.pixel_xfer;
		break;
	case 0x0060:
		// video memory - presume the memory is sourced from the current X/Y co-ords
		src = m_vga->mem_linear_r(space,((ibm8514.curr_y * IBM8514_LINE_LENGTH) + ibm8514.curr_x),0xff);
		break;
	}

	// write the data
	switch(ibm8514.bgmix & 0x000f)
	{
	case 0x0000:
		m_vga->mem_linear_w(space,offset,~dst,0xff);
		break;
	case 0x0001:
		m_vga->mem_linear_w(space,offset,0x00,0xff);
		break;
	case 0x0002:
		m_vga->mem_linear_w(space,offset,0xff,0xff);
		break;
	case 0x0003:
		m_vga->mem_linear_w(space,offset,dst,0xff);
		break;
	case 0x0004:
		m_vga->mem_linear_w(space,offset,~src,0xff);
		break;
	case 0x0005:
		m_vga->mem_linear_w(space,offset,src ^ dst,0xff);
		break;
	case 0x0006:
		m_vga->mem_linear_w(space,offset,~(src ^ dst),0xff);
		break;
	case 0x0007:
		m_vga->mem_linear_w(space,offset,src,0xff);
		break;
	case 0x0008:
		m_vga->mem_linear_w(space,offset,~(src & dst),0xff);
		break;
	case 0x0009:
		m_vga->mem_linear_w(space,offset,(~src) | dst,0xff);
		break;
	case 0x000a:
		m_vga->mem_linear_w(space,offset,src | (~dst),0xff);
		break;
	case 0x000b:
		m_vga->mem_linear_w(space,offset,src | dst,0xff);
		break;
	case 0x000c:
		m_vga->mem_linear_w(space,offset,src & dst,0xff);
		break;
	case 0x000d:
		m_vga->mem_linear_w(space,offset,src & (~dst),0xff);
		break;
	case 0x000e:
		m_vga->mem_linear_w(space,offset,(~src) & dst,0xff);
		break;
	case 0x000f:
		m_vga->mem_linear_w(space,offset,~(src | dst),0xff);
		break;
	}
}

void ibm8514a_device::ibm8514_write(UINT32 offset, UINT32 src)
{
	int data_size = 8;
	UINT32 xfer = 0;
	address_space& space = machine().device("maincpu")->memory().space(AS_PROGRAM);

	switch(ibm8514.pixel_control & 0x00c0)
	{
	case 0x0000:  // Foreground Mix only
		ibm8514_write_fg(offset);
		break;
	case 0x0040:  // fixed pattern (?)
		// TODO
		break;
	case 0x0080:  // use pixel transfer register
		if(ibm8514.bus_size == 0)  // 8-bit
			data_size = 8;
		if(ibm8514.bus_size == 1)  // 16-bit
			data_size = 16;
		if(ibm8514.bus_size == 2)  // 32-bit
			data_size = 32;
		if((ibm8514.current_cmd & 0x1000) && (data_size != 8))
		{
			xfer = ((ibm8514.pixel_xfer & 0x000000ff) << 8) | ((ibm8514.pixel_xfer & 0x0000ff00) >> 8)
					| ((ibm8514.pixel_xfer & 0x00ff0000) << 8) | ((ibm8514.pixel_xfer & 0xff000000) >> 8);
		}
		else
			xfer = ibm8514.pixel_xfer;
		if(ibm8514.current_cmd & 0x0002)
		{
			if((xfer & ((1<<(data_size-1))>>ibm8514.src_x)) != 0)
				ibm8514_write_fg(offset);
			else
				ibm8514_write_bg(offset);
		}
		else
		{
			ibm8514_write_fg(offset);
		}
		ibm8514.src_x++;
		if(ibm8514.src_x >= data_size)
			ibm8514.src_x = 0;
		break;
	case 0x00c0:  // use source plane
		if(m_vga->mem_linear_r(space,src,0xff) != 0x00)
			ibm8514_write_fg(offset);
		else
			ibm8514_write_bg(offset);
		break;
	}
}

/*
92E8h W(R/W):  Line Error Term Read/Write Register (ERR_TERM).
bit  0-12  (911/924) LINE PARAMETER/ERROR TERM. For Line Drawing this is the
            Bresenham Initial Error Term 2*dminor-dmajor (one less if the
            starting X is less than the ending X) in two's complement format.
            (dminor is the length of the line projected onto the minor or
            dependent axis, dmajor is the length of the line projected onto
            the major or independent axis).
     0-13  (80x +) LINE PARAMETER/ERROR TERM. See above.
 */
READ16_MEMBER(ibm8514a_device::ibm8514_line_error_r)
{
	return ibm8514.line_errorterm;
}

WRITE16_MEMBER(ibm8514a_device::ibm8514_line_error_w)
{
	ibm8514.line_errorterm = data;
	if(LOG_8514) logerror("8514/A: Line Parameter/Error Term write %04x\n",data);
}

/*
  9AE8h W(R):  Graphics Processor Status Register (GP_STAT)
bit   0-7  Queue State.
             00h = 8 words available - queue is empty
             01h = 7 words available
             03h = 6 words available
             07h = 5 words available
             0Fh = 4 words available
             1Fh = 3 words available
             3Fh = 2 words available
             7Fh = 1 word  available
             FFh = 0 words available - queue is full
        8  (911-928) DTA AVA. Read Data Available. If set data is ready to be
            read from the PIX_TRANS register (E2E8h).
        9  HDW BSY. Hardware Graphics Processor Busy
           If set the Graphics Processor is busy.
       10  (928 +) AE. All FIFO Slots Empty. If set all FIFO slots are empty.
    11-15  (864/964) (R) Queue State bits 8-12. 1Fh if 8 words or less
            available, Fh for 9 words, 7 for 10 words, 3 for 11 words, 1 for
            12 words and 0 for 13 words available.
 */
READ16_MEMBER(ibm8514a_device::ibm8514_gpstatus_r)
{
	UINT16 ret = 0x0000;

	//if(LOG_8514) logerror("S3: 9AE8 read\n");
	if(ibm8514.gpbusy == true)
		ret |= 0x0200;
	if(ibm8514.data_avail == true)
		ret |= 0x0100;
	return ret;
}

void ibm8514a_device::ibm8514_draw_vector(UINT8 len, UINT8 dir, bool draw)
{
	UINT32 offset;
	int x = 0;

	while(x <= len)
	{
		offset = (ibm8514.curr_y * IBM8514_LINE_LENGTH) + ibm8514.curr_x;
		if(draw)
			ibm8514_write(offset,offset);
		switch(dir)
		{
		case 0:  // 0 degrees
			ibm8514.curr_x++;
			break;
		case 1:  // 45 degrees
			ibm8514.curr_x++;
			ibm8514.curr_y--;
			break;
		case 2:  // 90 degrees
			ibm8514.curr_y--;
			break;
		case 3:  // 135 degrees
			ibm8514.curr_y--;
			ibm8514.curr_x--;
			break;
		case 4:  // 180 degrees
			ibm8514.curr_x--;
			break;
		case 5:  // 225 degrees
			ibm8514.curr_x--;
			ibm8514.curr_y++;
			break;
		case 6:  // 270 degrees
			ibm8514.curr_y++;
			break;
		case 7:  // 315 degrees
			ibm8514.curr_y++;
			ibm8514.curr_x++;
			break;
		}
		x++;
	}
}

/*
9AE8h W(W):  Drawing Command Register (CMD)
bit     0  (911-928) ~RD/WT. Read/Write Data. If set VRAM write operations are
            enabled. If clear operations execute normally but writes are
            disabled.
        1  PX MD. Pixel Mode. Defines the orientation of the display bitmap.
             0 = Through plane mode (Single pixel transferred at a time)
             1 = Across plane mode (Multiple pixels transferred at a time).
        2  LAST PXOF. Last Pixel Off. If set the last pixel of a line command
           (CMD_LINE, SSV or LINEAF) is not drawn. This is used for mixes such
           as XOR where drawing the same pixel twice would give the wrong
           color.
        3  DIR TYP. Direction Type.
             0: Bresenham line drawing (X-Y Axial)
                  CMD_LINE draws a line using the Bresenham algorithm as
                  specified in the DESTY_AXSTP (8AE8h), DESTX_DIASTP (8EE8h),
                  ERR_TERM (92E8h) and MAJ_AXIS_PCNT (96E8h) registers
                  INC_X, INC_Y and YMAJAXIS determines the direction.
             1: Vector line draws (Radial).
                  CMD_NOP allows drawing of Short Stroke Vectors (SSVs) by
                  writing to the Short Stroke register (9EE8h).
                  CMD_LINE draws a vector of length MAJ_AXIS_PCNT (96E8h)
                  in the direction specified by LINEDIR (bits 5-7).
                  DRWG-DIR determines the direction of the line.
        4  DRAW YES. If clear the current position is moved, but no pixels
           are modified. This bit should be set when attempting read or
           write of bitmap data.
      5-7  DRWG-DIR. Drawing Direction. When a line draw command (CMD_LINE)
           with DIR TYP=1 (Radial) is issued, these bits define the direction
           of the line counter clockwise relative to the positive X-axis.
             0 = 000 degrees
             1 = 045 degrees
             2 = 090 degrees
             3 = 135 degrees
             4 = 180 degrees
             5 = 225 degrees
             6 = 270 degrees
             7 = 315 degrees
        5  INC_X. This bit together with INC_Y determines which quadrant
           the slope of a line lies within. They also determine the
           orientation of rectangle draw commands.
           If set lines are drawn in the positive X direction (left to right).
        6  YMAJAXIS. For Bresenham line drawing commands this bit determines
           which axis is the independent or major axis. INC_X and INC_Y
           determines which quadrant the slope falls within. This bit further
           defines the slope to within an octant.
           If set Y is the major (independent) axis.
        7  INC_Y. This bit together with INC_X determines which quadrant
           the slope of a line lies within. They also determine the
           orientation of rectangle draw commands.
           If set lines are drawn in the positive Y direction (down).
        8  WAIT YES. If set the drawing engine waits for read/write of the
           PIX_TRANS register (E2E8h) for each pixel during a draw operation.
        9  (911-928) BUS SIZE. If set the PIX_TRANS register (E2E8h) is
            processed internally as two bytes in the order specified by BYTE
            SWAP. If clear all accesses to E2E8h are 8bit.
     9-10  (864,964) BUS SIZE. Select System Bus Size. Controls the width of
            the Pixel Data Transfer registers (E2E8h,E2EAh) and the memory
            mapped I/O. 0: 8bit, 1: 16bit, 2: 32bit
       12  BYTE SWAP. Affects both reads and writes of SHORT_STROKE (9EE8h)
           and PIX_TRANS (E2E8h) when 16bit=1.
           If set take low byte first, if clear take high byte first.
    13-15  Draw Command:
            0 = NOP. Used for Short Stroke Vectors.
            1 = Draw Line. If bit 3 is set the line is drawn to the angle in
                bits 5-7 and the length in the Major Axis Pixel Count register
                (96E8h), if clear the line is drawn from the Bresenham
                constants in the Axial Step Constant register(8AE8h), Diagonal
                Step Constant register (8EE8h), Line Error Term register
               (92E8h) and bits 5-7 of this register.
            2 = Rectangle Fill. The Current X (86E8h) and Y (82E8h)
                registers holds the coordinates of the rectangle to fill and
                the Major Axis Pixel Count register (96E8h) holds the
                horizontal width (in pixels) fill and the Minor Axis Pixel
                Count register (BEE8h index 0) holds the height of the
                rectangle.
            6 = BitBLT. Copies the source rectangle specified by the Current X
                (86E8h) and Y (8AE8h) registers to the destination rectangle,
                specified as for the Rectangle Fills.
            7 = (80x +) Pattern Fill. The source rectangle is an 8x8 pattern
                rectangle, which is copied repeatably to the destination
                rectangle.
 */
WRITE16_MEMBER(ibm8514a_device::ibm8514_cmd_w)
{
	int x,y;
	int pattern_x,pattern_y;
	UINT32 off,src;
	UINT8 readmask;

	ibm8514.current_cmd = data;
	ibm8514.src_x = 0;
	ibm8514.src_y = 0;
	ibm8514.bus_size = (data & 0x0600) >> 9;
	switch(data & 0xe000)
	{
	case 0x0000:  // NOP (for "Short Stroke Vectors")
		ibm8514.state = IBM8514_IDLE;
		ibm8514.gpbusy = false;
		if(LOG_8514) logerror("8514/A: Command (%04x) - NOP (Short Stroke Vector)\n",ibm8514.current_cmd);
		break;
	case 0x2000:  // Line
		ibm8514.state = IBM8514_IDLE;
		ibm8514.gpbusy = false;
		if(data & 0x0008)
		{
			if(data & 0x0100)
			{
				ibm8514.state = IBM8514_DRAWING_LINE;
				ibm8514.data_avail = true;
				if(LOG_8514) logerror("8514/A: Command (%04x) - Vector Line (WAIT) %i,%i \n",ibm8514.current_cmd,ibm8514.curr_x,ibm8514.curr_y);
			}
			else
			{
				ibm8514_draw_vector(ibm8514.rect_width,(data & 0x00e0) >> 5,(data & 0010) ? true : false);
				if(LOG_8514) logerror("8514/A: Command (%04x) - Vector Line - %i,%i \n",ibm8514.current_cmd,ibm8514.curr_x,ibm8514.curr_y);
			}
		}
		else
		{
			// Not perfect, but will do for now.
			INT16 dx = ibm8514.rect_width;
			INT16 dy = ibm8514.line_axial_step >> 1;
			INT16 err = ibm8514.line_errorterm;
			int sx = (data & 0x0020) ? 1 : -1;
			int sy = (data & 0x0080) ? 1 : -1;
			int count = 0;
			INT16 temp;

			if(LOG_8514) logerror("8514/A: Command (%04x) - Line (Bresenham) - %i,%i  Axial %i, Diagonal %i, Error %i, Major Axis %i, Minor Axis %i\n",ibm8514.current_cmd,
				ibm8514.curr_x,ibm8514.curr_y,ibm8514.line_axial_step,ibm8514.line_diagonal_step,ibm8514.line_errorterm,ibm8514.rect_width,ibm8514.rect_height);

			if((data & 0x0040))
			{
				temp = dx; dx = dy; dy = temp;
			}
			for(;;)
			{
				ibm8514_write(ibm8514.curr_x + (ibm8514.curr_y * IBM8514_LINE_LENGTH),ibm8514.curr_x + (ibm8514.curr_y * IBM8514_LINE_LENGTH));
				if (count > ibm8514.rect_width) break;
				count++;
				if((err*2) > -dy)
				{
					err -= dy;
					ibm8514.curr_x += sx;
				}
				if((err*2) < dx)
				{
					err += dx;
					ibm8514.curr_y += sy;
				}
			}
		}
		break;
	case 0x4000:  // Rectangle Fill
		if(data & 0x0100)  // WAIT (for read/write of PIXEL TRANSFER (E2E8))
		{
			ibm8514.state = IBM8514_DRAWING_RECT;
			//ibm8514.gpbusy = true;  // DirectX 5 keeps waiting for the busy bit to be clear...
			ibm8514.bus_size = (data & 0x0600) >> 9;
			ibm8514.data_avail = true;
			if(LOG_8514) logerror("8514/A: Command (%04x) - Rectangle Fill (WAIT) %i,%i Width: %i Height: %i Colour: %08x\n",ibm8514.current_cmd,ibm8514.curr_x,
					ibm8514.curr_y,ibm8514.rect_width,ibm8514.rect_height,ibm8514.fgcolour);
			break;
		}
		if(LOG_8514) logerror("8514/A: Command (%04x) - Rectangle Fill %i,%i Width: %i Height: %i Colour: %08x\n",ibm8514.current_cmd,ibm8514.curr_x,
				ibm8514.curr_y,ibm8514.rect_width,ibm8514.rect_height,ibm8514.fgcolour);
		off = 0;
		off += (IBM8514_LINE_LENGTH * ibm8514.curr_y);
		off += ibm8514.curr_x;
		for(y=0;y<=ibm8514.rect_height;y++)
		{
			for(x=0;x<=ibm8514.rect_width;x++)
			{
				if(data & 0x0020)  // source pattern is always based on current X/Y?
					ibm8514_write((off+x) % m_vga->vga.svga_intf.vram_size,(off+x) % m_vga->vga.svga_intf.vram_size);
				else
					ibm8514_write((off-x) % m_vga->vga.svga_intf.vram_size,(off-x) % m_vga->vga.svga_intf.vram_size);
				if(ibm8514.current_cmd & 0x0020)
				{
					ibm8514.curr_x++;
					if(ibm8514.curr_x > ibm8514.prev_x + ibm8514.rect_width)
					{
						ibm8514.curr_x = ibm8514.prev_x;
						ibm8514.src_x = 0;
						if(ibm8514.current_cmd & 0x0080)
							ibm8514.curr_y++;
						else
							ibm8514.curr_y--;
					}
				}
				else
				{
					ibm8514.curr_x--;
					if(ibm8514.curr_x < ibm8514.prev_x - ibm8514.rect_width)
					{
						ibm8514.curr_x = ibm8514.prev_x;
						ibm8514.src_x = 0;
						if(ibm8514.current_cmd & 0x0080)
							ibm8514.curr_y++;
						else
							ibm8514.curr_y--;
					}
				}
			}
			if(data & 0x0080)
				off += IBM8514_LINE_LENGTH;
			else
				off -= IBM8514_LINE_LENGTH;
		}
		ibm8514.state = IBM8514_IDLE;
		ibm8514.gpbusy = false;
		break;
	case 0xc000:  // BitBLT
		if(LOG_8514) logerror("8514/A: Command (%04x) - BitBLT from %i,%i to %i,%i  Width: %i  Height: %i\n",ibm8514.current_cmd,
				ibm8514.curr_x,ibm8514.curr_y,ibm8514.dest_x,ibm8514.dest_y,ibm8514.rect_width,ibm8514.rect_height);
		off = 0;
		off += (IBM8514_LINE_LENGTH * ibm8514.dest_y);
		off += ibm8514.dest_x;
		src = 0;
		src += (IBM8514_LINE_LENGTH * ibm8514.curr_y);
		src += ibm8514.curr_x;
		readmask = ((ibm8514.read_mask & 0x01) << 7) | ((ibm8514.read_mask & 0xfe) >> 1);
		for(y=0;y<=ibm8514.rect_height;y++)
		{
			for(x=0;x<=ibm8514.rect_width;x++)
			{
				if((ibm8514.pixel_control & 0xc0) == 0xc0)
				{
					// only check read mask if Mix Select is set to 11 (VRAM determines mix)
					if(m_vga->mem_linear_r(space,(src+x),0xff) & ~readmask)
					{
						// presumably every program is going to be smart enough to set the FG mix to use VRAM (0x6x)
						if(data & 0x0020)
							ibm8514_write(off+x,src+x);
						else
							ibm8514_write(off-x,src-x);
					}
				}
				else
				{
					// presumably every program is going to be smart enough to set the FG mix to use VRAM (0x6x)
					if(data & 0x0020)
						ibm8514_write(off+x,src+x);
					else
						ibm8514_write(off-x,src-x);
				}
				if(ibm8514.current_cmd & 0x0020)
				{
					ibm8514.curr_x++;
					if(ibm8514.curr_x > ibm8514.prev_x + ibm8514.rect_width)
					{
						ibm8514.curr_x = ibm8514.prev_x;
						ibm8514.src_x = 0;
						if(ibm8514.current_cmd & 0x0080)
							ibm8514.curr_y++;
						else
							ibm8514.curr_y--;
					}
				}
				else
				{
					ibm8514.curr_x--;
					if(ibm8514.curr_x < ibm8514.prev_x - ibm8514.rect_width)
					{
						ibm8514.curr_x = ibm8514.prev_x;
						ibm8514.src_x = 0;
						if(ibm8514.current_cmd & 0x0080)
							ibm8514.curr_y++;
						else
							ibm8514.curr_y--;
					}
				}
			}
			if(data & 0x0080)
			{
				src += IBM8514_LINE_LENGTH;
				off += IBM8514_LINE_LENGTH;
			}
			else
			{
				src -= IBM8514_LINE_LENGTH;
				off -= IBM8514_LINE_LENGTH;
			}
		}
		ibm8514.state = IBM8514_IDLE;
		ibm8514.gpbusy = false;
		ibm8514.curr_x = ibm8514.prev_x;
		ibm8514.curr_y = ibm8514.prev_y;
		break;
	case 0xe000:  // Pattern Fill
		if(LOG_8514) logerror("8514/A: Command (%04x) - Pattern Fill - source %i,%i  dest %i,%i  Width: %i Height: %i\n",ibm8514.current_cmd,
				ibm8514.curr_x,ibm8514.curr_y,ibm8514.dest_x,ibm8514.dest_y,ibm8514.rect_width,ibm8514.rect_height);
		off = 0;
		off += (IBM8514_LINE_LENGTH * ibm8514.dest_y);
		off += ibm8514.dest_x;
		src = 0;
		src += (IBM8514_LINE_LENGTH * ibm8514.curr_y);
		src += ibm8514.curr_x;
		if(data & 0x0020)
			pattern_x = 0;
		else
			pattern_x = 7;
		if(data & 0x0080)
			pattern_y = 0;
		else
			pattern_y = 7;

		for(y=0;y<=ibm8514.rect_height;y++)
		{
			for(x=0;x<=ibm8514.rect_width;x++)
			{
				if(data & 0x0020)
				{
					ibm8514_write(off+x,src+pattern_x);
					pattern_x++;
					if(pattern_x >= 8)
						pattern_x = 0;
				}
				else
				{
					ibm8514_write(off-x,src-pattern_x);
					pattern_x--;
					if(pattern_x < 0)
						pattern_x = 7;
				}
			}

			// for now, presume that INC_X and INC_Y affect both src and dest, at is would for a bitblt.
			if(data & 0x0020)
				pattern_x = 0;
			else
				pattern_x = 7;
			if(data & 0x0080)
			{
				pattern_y++;
				src += IBM8514_LINE_LENGTH;
				if(pattern_y >= 8)
				{
					pattern_y = 0;
					src -= (IBM8514_LINE_LENGTH * 8);  // move src pointer back to top of pattern
				}
				off += IBM8514_LINE_LENGTH;
			}
			else
			{
				pattern_y--;
				src -= IBM8514_LINE_LENGTH;
				if(pattern_y < 0)
				{
					pattern_y = 7;
					src += (IBM8514_LINE_LENGTH * 8);  // move src pointer back to bottom of pattern
				}
				off -= IBM8514_LINE_LENGTH;
			}
		}
		ibm8514.state = IBM8514_IDLE;
		ibm8514.gpbusy = false;
		break;
	default:
		ibm8514.state = IBM8514_IDLE;
		ibm8514.gpbusy = false;
		if(LOG_8514) logerror("8514/A: Unknown command: %04x\n",data);
	}
}

/*
8AE8h W(R/W):  Destination Y Position & Axial Step Constant Register
               (DESTY_AXSTP)
bit  0-11  DESTINATION Y-POSITION. During BITBLT operations this is the Y
           co-ordinate of the destination in pixels.
     0-12  (911/924) LINE PARAMETER AXIAL STEP CONSTANT. During Line Drawing,
            this is the Bresenham constant 2*dminor in two's complement
            format. (dminor is the length of the line projected onto the minor
            or dependent axis).
     0-13  (80 x+) LINE PARAMETER AXIAL STEP CONSTANT. Se above

 */
READ16_MEMBER(ibm8514a_device::ibm8514_desty_r)
{
	return ibm8514.line_axial_step;
}

WRITE16_MEMBER(ibm8514a_device::ibm8514_desty_w)
{
	ibm8514.line_axial_step = data;
	ibm8514.dest_y = data;
	if(LOG_8514) logerror("8514/A: Line Axial Step / Destination Y write %04x\n",data);
}

/*
8EE8h W(R/W):  Destination X Position & Diagonal Step Constant Register
               (DESTX_DISTP)
bit  0-11  DESTINATION X-POSITION. During BITBLT operations this is the X
           co-ordinate of the destination in pixels.
     0-12  (911/924) LINE PARAMETER DIAGONAL STEP CONSTANT. During Line
            Drawing this is the Bresenham constant 2*dminor-2*dmajor in two's
            complement format. (dminor is the length of the line projected
            onto the minor or dependent axis, dmajor is the length of the line
            projected onto the major or independent axis)
     0-13  (80x +) LINE PARAMETER DIAGONAL STEP CONSTANT. Se above

 */
READ16_MEMBER(ibm8514a_device::ibm8514_destx_r)
{
	return ibm8514.line_diagonal_step;
}

WRITE16_MEMBER(ibm8514a_device::ibm8514_destx_w)
{
	ibm8514.line_diagonal_step = data;
	ibm8514.dest_x = data;
	if(LOG_8514) logerror("8514/A: Line Diagonal Step / Destination X write %04x\n",data);
}

/*
9EE8h W(R/W):  Short Stroke Vector Transfer Register (SHORT_STROKE)
bit   0-3  Length of vector projected onto the major axis.
           This is also the number of pixels drawn.
        4  Must be set for pixels to be written.
      5-7  VECDIR. The angle measured counter-clockwise from horizontal
           right) at which the line is drawn,
             0 = 000 degrees
             1 = 045 degrees
             2 = 090 degrees
             3 = 135 degrees
             4 = 180 degrees
             5 = 225 degrees
             6 = 270 degrees
             7 = 315 degrees
     8-15  The lower 8 bits are duplicated in the upper 8 bits so two
           short stroke vectors can be drawn with one command.
Note: The upper byte must be written for the SSV command to be executed.
      Thus if a byte is written to 9EE8h another byte must be written to
      9EE9h before execution starts. A single 16bit write will do.
      If only one SSV is desired the other byte can be set to 0.
 */
void ibm8514a_device::ibm8514_wait_draw_ssv()
{
	UINT8 len = ibm8514.wait_vector_len;
	UINT8 dir = ibm8514.wait_vector_dir;
	bool draw = ibm8514.wait_vector_draw;
	UINT8 count = ibm8514.wait_vector_count;
	UINT32 offset;
	int x;
	int data_size;

	switch(ibm8514.bus_size)
	{
	case 0:
		data_size = 8;
		break;
	case 1:
		data_size = 16;
		break;
	case 2:
		data_size = 32;
		break;
	default:
		data_size = 8;
		break;
	}

	for(x=0;x<data_size;x++)
	{
		if(len > count)
		{
			if(ibm8514.state == IBM8514_DRAWING_SSV_1)
			{
				ibm8514.state = IBM8514_DRAWING_SSV_2;
				ibm8514.wait_vector_len = (ibm8514.ssv & 0x0f00) >> 8;
				ibm8514.wait_vector_dir = (ibm8514.ssv & 0xe000) >> 13;
				ibm8514.wait_vector_draw = (ibm8514.ssv & 0x1000) ? true : false;
				ibm8514.wait_vector_count = 0;
				return;
			}
			else if(ibm8514.state == IBM8514_DRAWING_SSV_2)
			{
				ibm8514.state = IBM8514_IDLE;
				ibm8514.gpbusy = false;
				ibm8514.data_avail = false;
				return;
			}
		}

		if(ibm8514.state == IBM8514_DRAWING_SSV_1 || ibm8514.state == IBM8514_DRAWING_SSV_2)
		{
			offset = (ibm8514.curr_y * IBM8514_LINE_LENGTH) + ibm8514.curr_x;
			if(draw)
				ibm8514_write(offset,offset);
			switch(dir)
			{
			case 0:  // 0 degrees
				ibm8514.curr_x++;
				break;
			case 1:  // 45 degrees
				ibm8514.curr_x++;
				ibm8514.curr_y--;
				break;
			case 2:  // 90 degrees
				ibm8514.curr_y--;
				break;
			case 3:  // 135 degrees
				ibm8514.curr_y--;
				ibm8514.curr_x--;
				break;
			case 4:  // 180 degrees
				ibm8514.curr_x--;
				break;
			case 5:  // 225 degrees
				ibm8514.curr_x--;
				ibm8514.curr_y++;
				break;
			case 6:  // 270 degrees
				ibm8514.curr_y++;
				break;
			case 7:  // 315 degrees
				ibm8514.curr_y++;
				ibm8514.curr_x++;
				break;
			}
		}
	}
}

void ibm8514a_device::ibm8514_draw_ssv(UINT8 data)
{
	UINT8 len = data & 0x0f;
	UINT8 dir = (data & 0xe0) >> 5;
	bool draw = (data & 0x10) ? true : false;

	ibm8514_draw_vector(len,dir,draw);
}

READ16_MEMBER(ibm8514a_device::ibm8514_ssv_r)
{
	return ibm8514.ssv;
}

WRITE16_MEMBER(ibm8514a_device::ibm8514_ssv_w)
{
	ibm8514.ssv = data;

	if(ibm8514.current_cmd & 0x0100)
	{
		ibm8514.state = IBM8514_DRAWING_SSV_1;
		ibm8514.data_avail = true;
		ibm8514.wait_vector_len = ibm8514.ssv & 0x0f;
		ibm8514.wait_vector_dir = (ibm8514.ssv & 0xe0) >> 5;
		ibm8514.wait_vector_draw = (ibm8514.ssv & 0x10) ? true : false;
		ibm8514.wait_vector_count = 0;
		return;
	}

	if(ibm8514.current_cmd & 0x1000)  // byte sequence
	{
		ibm8514_draw_ssv(data & 0xff);
		ibm8514_draw_ssv(data >> 8);
	}
	else
	{
		ibm8514_draw_ssv(data >> 8);
		ibm8514_draw_ssv(data & 0xff);
	}
	if(LOG_8514) logerror("8514/A: Short Stroke Vector write %04x\n",data);
}

void ibm8514a_device::ibm8514_wait_draw_vector()
{
	UINT8 len = ibm8514.wait_vector_len;
	UINT8 dir = ibm8514.wait_vector_dir;
	bool draw = ibm8514.wait_vector_draw;
	UINT8 count = ibm8514.wait_vector_count;
	UINT32 offset;
	UINT8 data_size = 0;
	int x;

	if(ibm8514.bus_size == 0)  // 8-bit
		data_size = 8;
	if(ibm8514.bus_size == 1)  // 16-bit
		data_size = 16;
	if(ibm8514.bus_size == 2)  // 32-bit
		data_size = 32;

	for(x=0;x<data_size;x++)
	{
		if(len > count)
		{
			if(ibm8514.state == IBM8514_DRAWING_LINE)
			{
				ibm8514.state = IBM8514_IDLE;
				ibm8514.gpbusy = false;
				ibm8514.data_avail = false;
				return;
			}
		}

		if(ibm8514.state == IBM8514_DRAWING_LINE)
		{
			offset = (ibm8514.curr_y * IBM8514_LINE_LENGTH) + ibm8514.curr_x;
			if(draw)
				ibm8514_write(offset,offset);
			switch(dir)
			{
			case 0:  // 0 degrees
				ibm8514.curr_x++;
				break;
			case 1:  // 45 degrees
				ibm8514.curr_x++;
				ibm8514.curr_y--;
				break;
			case 2:  // 90 degrees
				ibm8514.curr_y--;
				break;
			case 3:  // 135 degrees
				ibm8514.curr_y--;
				ibm8514.curr_x--;
				break;
			case 4:  // 180 degrees
				ibm8514.curr_x--;
				break;
			case 5:  // 225 degrees
				ibm8514.curr_x--;
				ibm8514.curr_y++;
				break;
			case 6:  // 270 degrees
				ibm8514.curr_y++;
				break;
			case 7:  // 315 degrees
				ibm8514.curr_y++;
				ibm8514.curr_x++;
				break;
			}
		}
	}
}

/*
96E8h W(R/W):  Major Axis Pixel Count/Rectangle Width Register (MAJ_AXIS_PCNT)
bit  0-10  (911/924)  RECTANGLE WIDTH/LINE PARAMETER MAX. For BITBLT and
            rectangle commands this is the width of the area. For Line Drawing
            this is the Bresenham constant dmajor in two's complement format.
            (dmajor is the length of the line projected onto the major or
            independent axis). Must be positive.
     0-11  (80x +) RECTANGLE WIDTH/LINE PARAMETER MAX. See above
 */
READ16_MEMBER(ibm8514a_device::ibm8514_width_r)
{
	return ibm8514.rect_width;
}

WRITE16_MEMBER(ibm8514a_device::ibm8514_width_w)
{
	ibm8514.rect_width = data & 0x1fff;
	if(LOG_8514) logerror("8514/A: Major Axis Pixel Count / Rectangle Width write %04x\n",data);
}

READ16_MEMBER(ibm8514a_device::ibm8514_currentx_r)
{
	return ibm8514.curr_x;
}

WRITE16_MEMBER(ibm8514a_device::ibm8514_currentx_w)
{
	ibm8514.curr_x = data;
	ibm8514.prev_x = data;
	if(LOG_8514) logerror("8514/A: Current X set to %04x (%i)\n",data,ibm8514.curr_x);
}

READ16_MEMBER(ibm8514a_device::ibm8514_currenty_r)
{
	return ibm8514.curr_y;
}

WRITE16_MEMBER(ibm8514a_device::ibm8514_currenty_w)
{
	ibm8514.curr_y = data;
	ibm8514.prev_y = data;
	if(LOG_8514) logerror("8514/A: Current Y set to %04x (%i)\n",data,ibm8514.curr_y);
}

READ16_MEMBER(ibm8514a_device::ibm8514_fgcolour_r)
{
	return ibm8514.fgcolour;
}

WRITE16_MEMBER(ibm8514a_device::ibm8514_fgcolour_w)
{
	ibm8514.fgcolour = data;
	if(LOG_8514) logerror("8514/A: Foreground Colour write %04x\n",data);
}

READ16_MEMBER(ibm8514a_device::ibm8514_bgcolour_r)
{
	return ibm8514.bgcolour;
}

WRITE16_MEMBER(ibm8514a_device::ibm8514_bgcolour_w)
{
	ibm8514.bgcolour = data;
	if(LOG_8514) logerror("8514/A: Background Colour write %04x\n",data);
}

/*
AEE8h W(R/W):  Read Mask Register (RD_MASK)
bit   0-7  (911/924) Read Mask affects the following commands: CMD_RECT,
            CMD_BITBLT and reading data in Across Plane Mode.
            Each bit set prevents the plane from being read.
     0-15  (801/5) Readmask. See above.
     0-31  (928 +) Readmask. See above. In 32 bits per pixel modes there are
            two 16bit registers at this address. BEE8h index 0Eh bit 4 selects
            which 16 bits are accessible and each access toggles to the other
            16 bits.
 */
READ16_MEMBER(ibm8514a_device::ibm8514_read_mask_r)
{
	return ibm8514.read_mask & 0xffff;
}

WRITE16_MEMBER(ibm8514a_device::ibm8514_read_mask_w)
{
	ibm8514.read_mask = (ibm8514.read_mask & 0xffff0000) | data;
	if(LOG_8514) logerror("8514/A: Read Mask (Low) write = %08x\n",ibm8514.read_mask);
}

/*
AAE8h W(R/W):  Write Mask Register (WRT_MASK)
bit   0-7  (911/924) Writemask. A plane can only be modified if the
            corresponding bit is set.
     0-15  (801/5) Writemask. See above.
     0-31  (928 +) Writemask. See above. In 32 bits per pixel modes there are
            two 16bit registers at this address. BEE8h index 0Eh bit 4 selects
            which 16 bits are accessible and each access toggles to the other
            16 bits.
 */
READ16_MEMBER(ibm8514a_device::ibm8514_write_mask_r)
{
	return ibm8514.write_mask & 0xffff;
}

WRITE16_MEMBER(ibm8514a_device::ibm8514_write_mask_w)
{
	ibm8514.write_mask = (ibm8514.write_mask & 0xffff0000) | data;
	if(LOG_8514) logerror("8514/A: Write Mask (Low) write = %08x\n",ibm8514.write_mask);
}

READ16_MEMBER(ibm8514a_device::ibm8514_multifunc_r )
{
	switch(ibm8514.multifunc_sel)
	{
	case 0:
		return ibm8514.rect_height;
	case 1:
		return ibm8514.scissors_top;
	case 2:
		return ibm8514.scissors_left;
	case 3:
		return ibm8514.scissors_bottom;
	case 4:
		return ibm8514.scissors_right;
		// TODO: remaining functions
	default:
		if(LOG_8514) logerror("8514/A: Unimplemented multifunction register %i selected\n",ibm8514.multifunc_sel);
		return 0xff;
	}
}

WRITE16_MEMBER(ibm8514a_device::ibm8514_multifunc_w )
{
	switch(data & 0xf000)
	{
/*
BEE8h index 00h W(R/W): Minor Axis Pixel Count Register (MIN_AXIS_PCNT).
bit  0-10  (911/924) Rectangle Height. Height of BITBLT or rectangle command.
            Actual height is one larger.
     0-11  (80x +) Rectangle Height. See above
*/
	case 0x0000:
		ibm8514.rect_height = data & 0x0fff;
		if(LOG_8514) logerror("8514/A: Minor Axis Pixel Count / Rectangle Height write %04x\n",data);
		break;
/*
BEE8h index 01h W(R/W):  Top Scissors Register (SCISSORS_T).
bit  0-10  (911/924) Clipping Top Limit. Defines the upper bound of the
            Clipping Rectangle (Lowest Y coordinate).
     0-11  (80x +) Clipping Top Limit. See above

BEE8h index 02h W(R/W):  Left Scissors Registers (SCISSORS_L).
bit  0-10  (911,924) Clipping Left Limit. Defines the left bound of the
            Clipping Rectangle (Lowest X coordinate).
     0-11  (80x +) Clipping Left Limit. See above.

BEE8h index 03h W(R/W):  Bottom Scissors Register (SCISSORS_B).
bit  0-10  (911,924) Clipping Bottom Limit. Defines the bottom bound of the
            Clipping Rectangle (Highest Y coordinate).
     0-11  (80x +) Clipping Bottom Limit. See above.

BEE8h index 04h W(R/W):  Right Scissors Register (SCISSORS_R).
bit  0-10  (911,924) Clipping Right Limit. Defines the right bound of the
            Clipping Rectangle (Highest X coordinate).
     0-11  (80x +) Clipping Bottom Limit. See above.
 */
	case 0x1000:
		ibm8514.scissors_top = data & 0x0fff;
		if(LOG_8514) logerror("S3: Scissors Top write %04x\n",data);
		break;
	case 0x2000:
		ibm8514.scissors_left = data & 0x0fff;
		if(LOG_8514) logerror("S3: Scissors Left write %04x\n",data);
		break;
	case 0x3000:
		ibm8514.scissors_bottom = data & 0x0fff;
		if(LOG_8514) logerror("S3: Scissors Bottom write %04x\n",data);
		break;
	case 0x4000:
		ibm8514.scissors_right = data & 0x0fff;
		if(LOG_8514) logerror("S3: Scissors Right write %04x\n",data);
		break;
/*
BEE8h index 0Ah W(R/W):  Pixel Control Register (PIX_CNTL).
BIT     2  (911-928) Pack Data. If set image read data is a monochrome bitmap,
            if clear it is a bitmap of the current pixel depth
      6-7  DT-EX-DRC. Select Mix Select.
             0  Foreground Mix is always used.
             1  use fixed pattern to decide which mix setting to use on a pixel
             2  CPU Data (Pixel Transfer register) determines the Mix register used.
             3  Video memory determines the Mix register used.
 */
	case 0xa000:
		ibm8514.pixel_control = data;
		if(LOG_8514) logerror("S3: Pixel control write %04x\n",data);
		break;
	case 0xe000:
		ibm8514.multifunc_misc = data;
		if(LOG_8514) logerror("S3: Multifunction Miscellaneous write %04x\n",data);
		break;
/*
BEE8h index 0Fh W(W):  Read Register Select Register (READ_SEL)    (801/5,928)
bit   0-2  (911-928) READ-REG-SEL. Read Register Select. Selects the register
            that is actually read when a read of BEE8h happens. Each read of
            BEE8h increments this register by one.
             0: Read will return contents of BEE8h index 0.
             1: Read will return contents of BEE8h index 1.
             2: Read will return contents of BEE8h index 2.
             3: Read will return contents of BEE8h index 3.
             4: Read will return contents of BEE8h index 4.
             5: Read will return contents of BEE8h index 0Ah.
             6: Read will return contents of BEE8h index 0Eh.
             7: Read will return contents of 9AE8h (Bits 13-15 will be 0).
      0-3  (864,964) READ-REG-SEL. See above plus:
             8: Read will return contents of 42E8h (Bits 12-15 will be 0)
             9: Read will return contents of 46E8h
            10: Read will return contents of BEE8h index 0Dh
 */
	case 0xf000:
		ibm8514.multifunc_sel = data & 0x000f;
		if(LOG_8514) logerror("S3: Multifunction select write %04x\n",data);
	default:
		if(LOG_8514) logerror("S3: Unimplemented multifunction register %i write %03x\n",data >> 12,data & 0x0fff);
	}
}

void ibm8514a_device::ibm8514_wait_draw()
{
	int x, data_size = 8;
	UINT32 off;

	// the data in the pixel transfer register or written to VRAM masks the rectangle output
	if(ibm8514.bus_size == 0)  // 8-bit
		data_size = 8;
	if(ibm8514.bus_size == 1)  // 16-bit
		data_size = 16;
	if(ibm8514.bus_size == 2)  // 32-bit
		data_size = 32;
	off = 0;
	off += (IBM8514_LINE_LENGTH * ibm8514.curr_y);
	off += ibm8514.curr_x;
	if(ibm8514.current_cmd & 0x02) // "across plane mode"
	{
		for(x=0;x<data_size;x++)
		{
			ibm8514_write(off,off);
			if(ibm8514.current_cmd & 0x0020)
			{
				off++;
				ibm8514.curr_x++;
				if(ibm8514.curr_x > ibm8514.prev_x + ibm8514.rect_width)
				{
					ibm8514.curr_x = ibm8514.prev_x;
					ibm8514.src_x = 0;
					if(ibm8514.current_cmd & 0x0080)
					{
						ibm8514.curr_y++;
						if(ibm8514.curr_y > ibm8514.prev_y + ibm8514.rect_height)
						{
							ibm8514.state = IBM8514_IDLE;
							ibm8514.data_avail = false;
							ibm8514.gpbusy = false;
						}
					}
					else
					{
						ibm8514.curr_y--;
						if(ibm8514.curr_y < ibm8514.prev_y - ibm8514.rect_height)
						{
							ibm8514.state = IBM8514_IDLE;
							ibm8514.data_avail = false;
							ibm8514.gpbusy = false;
						}
					}
					return;
				}
			}
			else
			{
				off--;
				ibm8514.curr_x--;
				if(ibm8514.curr_x < ibm8514.prev_x - ibm8514.rect_width)
				{
					ibm8514.curr_x = ibm8514.prev_x;
					ibm8514.src_x = 0;
					if(ibm8514.current_cmd & 0x0080)
					{
						ibm8514.curr_y++;
						if(ibm8514.curr_y > ibm8514.prev_y + ibm8514.rect_height)
						{
							ibm8514.state = IBM8514_IDLE;
							ibm8514.gpbusy = false;
							ibm8514.data_avail = false;
						}
					}
					else
					{
						ibm8514.curr_y--;
						if(ibm8514.curr_y < ibm8514.prev_y - ibm8514.rect_height)
						{
							ibm8514.state = IBM8514_IDLE;
							ibm8514.gpbusy = false;
							ibm8514.data_avail = false;
						}
					}
					return;
				}
			}
		}
	}
	else
	{
		// "through plane" mode (single pixel)
		for(x=0;x<data_size;x+=8)
		{
			ibm8514_write(off,off);

			if(ibm8514.current_cmd & 0x0020)
			{
				off++;
				ibm8514.curr_x++;
				if(ibm8514.curr_x > ibm8514.prev_x + ibm8514.rect_width)
				{
					ibm8514.curr_x = ibm8514.prev_x;
					ibm8514.src_x = 0;
					if(ibm8514.current_cmd & 0x0080)
					{
						ibm8514.curr_y++;
						if(ibm8514.curr_y > ibm8514.prev_y + ibm8514.rect_height)
						{
							ibm8514.state = IBM8514_IDLE;
							ibm8514.gpbusy = false;
							ibm8514.data_avail = false;
						}
					}
					else
					{
						ibm8514.curr_y--;
						if(ibm8514.curr_y < ibm8514.prev_y - ibm8514.rect_height)
						{
							ibm8514.state = IBM8514_IDLE;
							ibm8514.gpbusy = false;
							ibm8514.data_avail = false;
						}
					}
					return;
				}
			}
			else
			{
				off--;
				ibm8514.curr_x--;
				if(ibm8514.curr_x < ibm8514.prev_x - ibm8514.rect_width)
				{
					ibm8514.curr_x = ibm8514.prev_x;
					ibm8514.src_x = 0;
					if(ibm8514.current_cmd & 0x0080)
					{
						ibm8514.curr_y++;
						if(ibm8514.curr_y > ibm8514.prev_y + ibm8514.rect_height)
						{
							ibm8514.state = IBM8514_IDLE;
							ibm8514.gpbusy = false;
							ibm8514.data_avail = false;
						}
					}
					else
					{
						ibm8514.curr_y--;
						if(ibm8514.curr_y < ibm8514.prev_y - ibm8514.rect_height)
						{
							ibm8514.state = IBM8514_IDLE;
							ibm8514.gpbusy = false;
							ibm8514.data_avail = false;
						}
					}
					return;
				}
			}
		}
	}
}

/*
B6E8h W(R/W):  Background Mix Register (BKGD_MIX)
bit   0-3  Background MIX (BACKMIX).
            00  not DST
            01  0 (false)
            02  1 (true)
            03  2 DST
            04  not SRC
            05  SRC xor DST
            06  not (SRC xor DST)
            07  SRC
            08  not (SRC and DST)
            09  (not SRC) or DST
            0A  SRC or (not DST)
            0B  SRC or DST
            0C  SRC and DST
            0D  SRC and (not DST)
            0E  (not SRC) and DST
            0F  not (SRC or DST)
           DST is always the destination bitmap, bit SRC has four
           possible sources selected by the BSS bits.
      5-6  Background Source Select (BSS)
             0  BSS is Background Color
             1  BSS is Foreground Color
             2  BSS is Pixel Data from the PIX_TRANS register (E2E8h)
             3  BSS is Bitmap Data (Source data from display buffer).
 */
READ16_MEMBER(ibm8514a_device::ibm8514_backmix_r)
{
	return ibm8514.bgmix;
}

WRITE16_MEMBER(ibm8514a_device::ibm8514_backmix_w)
{
	ibm8514.bgmix = data;
	if(LOG_8514) logerror("8514/A: BG Mix write %04x\n",data);
}

READ16_MEMBER(ibm8514a_device::ibm8514_foremix_r)
{
	return ibm8514.fgmix;
}

WRITE16_MEMBER(ibm8514a_device::ibm8514_foremix_w)
{
	ibm8514.fgmix = data;
	if(LOG_8514) logerror("8514/A: FG Mix write %04x\n",data);
}

READ16_MEMBER(ibm8514a_device::ibm8514_pixel_xfer_r)
{
	if(offset == 1)
		return (ibm8514.pixel_xfer & 0xffff0000) >> 16;
	else
		return ibm8514.pixel_xfer & 0x0000ffff;
}

WRITE16_MEMBER(ibm8514a_device::ibm8514_pixel_xfer_w)
{
	if(offset == 1)
		ibm8514.pixel_xfer = (ibm8514.pixel_xfer & 0x0000ffff) | (data << 16);
	else
		ibm8514.pixel_xfer = (ibm8514.pixel_xfer & 0xffff0000) | data;

	if(ibm8514.state == IBM8514_DRAWING_RECT)
		ibm8514_wait_draw();

	if(ibm8514.state == IBM8514_DRAWING_SSV_1 || ibm8514.state == IBM8514_DRAWING_SSV_2)
		ibm8514_wait_draw_ssv();

	if(ibm8514.state == IBM8514_DRAWING_LINE)
		ibm8514_wait_draw_vector();

	if(LOG_8514) logerror("S3: Pixel Transfer = %08x\n",ibm8514.pixel_xfer);
}

READ8_MEMBER(s3_vga_device::mem_r)
{
	if (svga.rgb8_en || svga.rgb15_en || svga.rgb16_en || svga.rgb32_en)
	{
		int data;
		if(offset & 0x10000)
			return 0;
		data = 0;
		if(vga.sequencer.data[4] & 0x8)
		{
			if((offset + (svga.bank_r*0x10000)) < vga.svga_intf.vram_size)
				data = vga.memory[(offset + (svga.bank_r*0x10000))];
		}
		else
		{
			int i;

			for(i=0;i<4;i++)
			{
				if(vga.sequencer.map_mask & 1 << i)
				{
					if((offset*4+i+(svga.bank_r*0x10000)) < vga.svga_intf.vram_size)
						data |= vga.memory[(offset*4+i+(svga.bank_r*0x10000))];
				}
			}
		}
		return data;
	}
	if((offset + (svga.bank_r*0x10000)) < vga.svga_intf.vram_size)
		return vga_device::mem_r(space,offset,mem_mask);
	else
		return 0xff;
}

WRITE8_MEMBER(s3_vga_device::mem_w)
{
	ibm8514a_device* dev = get_8514();
	// bit 4 of CR53 enables memory-mapped I/O
	// 0xA0000-0xA7fff maps to port 0xE2E8 (pixel transfer)
	if(s3.cr53 & 0x10)
	{
		if(offset < 0x8000)
		{
			// pass through to the pixel transfer register (DirectX 5 wants this)
			if(dev->ibm8514.bus_size == 0)
			{
				dev->ibm8514.pixel_xfer = (dev->ibm8514.pixel_xfer & 0xffffff00) | data;
				dev->ibm8514_wait_draw();
			}
			if(dev->ibm8514.bus_size == 1)
			{
				switch(offset & 0x0001)
				{
				case 0:
				default:
					dev->ibm8514.pixel_xfer = (dev->ibm8514.pixel_xfer & 0xffffff00) | data;
					break;
				case 1:
					dev->ibm8514.pixel_xfer = (dev->ibm8514.pixel_xfer & 0xffff00ff) | (data << 8);
					dev->ibm8514_wait_draw();
					break;
				}
			}
			if(dev->ibm8514.bus_size == 2)
			{
				switch(offset & 0x0003)
				{
				case 0:
				default:
					dev->ibm8514.pixel_xfer = (dev->ibm8514.pixel_xfer & 0xffffff00) | data;
					break;
				case 1:
					dev->ibm8514.pixel_xfer = (dev->ibm8514.pixel_xfer & 0xffff00ff) | (data << 8);
					break;
				case 2:
					dev->ibm8514.pixel_xfer = (dev->ibm8514.pixel_xfer & 0xff00ffff) | (data << 16);
					break;
				case 3:
					dev->ibm8514.pixel_xfer = (dev->ibm8514.pixel_xfer & 0x00ffffff) | (data << 24);
					dev->ibm8514_wait_draw();
					break;
				}
			}
			return;
		}
		switch(offset)
		{
		case 0x8100:
		case 0x82e8:
			dev->ibm8514.curr_y = (dev->ibm8514.curr_y & 0xff00) | data;
			dev->ibm8514.prev_y = (dev->ibm8514.prev_y & 0xff00) | data;
			break;
		case 0x8101:
		case 0x82e9:
			dev->ibm8514.curr_y = (dev->ibm8514.curr_y & 0x00ff) | (data << 8);
			dev->ibm8514.prev_y = (dev->ibm8514.prev_y & 0x00ff) | (data << 8);
			break;
		case 0x8102:
		case 0x86e8:
			dev->ibm8514.curr_x = (dev->ibm8514.curr_x & 0xff00) | data;
			dev->ibm8514.prev_x = (dev->ibm8514.prev_x & 0xff00) | data;
			break;
		case 0x8103:
		case 0x86e9:
			dev->ibm8514.curr_x = (dev->ibm8514.curr_x & 0x00ff) | (data << 8);
			dev->ibm8514.prev_x = (dev->ibm8514.prev_x & 0x00ff) | (data << 8);
			break;
		case 0x8108:
		case 0x8ae8:
			dev->ibm8514.line_axial_step = (dev->ibm8514.line_axial_step & 0xff00) | data;
			dev->ibm8514.dest_y = (dev->ibm8514.dest_y & 0xff00) | data;
			break;
		case 0x8109:
		case 0x8ae9:
			dev->ibm8514.line_axial_step = (dev->ibm8514.line_axial_step & 0x00ff) | ((data & 0x3f) << 8);
			dev->ibm8514.dest_y = (dev->ibm8514.dest_y & 0x00ff) | (data << 8);
			break;
		case 0x810a:
		case 0x8ee8:
			dev->ibm8514.line_diagonal_step = (dev->ibm8514.line_diagonal_step & 0xff00) | data;
			dev->ibm8514.dest_x = (dev->ibm8514.dest_x & 0xff00) | data;
			break;
		case 0x810b:
		case 0x8ee9:
			dev->ibm8514.line_diagonal_step = (dev->ibm8514.line_diagonal_step & 0x00ff) | ((data & 0x3f) << 8);
			dev->ibm8514.dest_x = (dev->ibm8514.dest_x & 0x00ff) | (data << 8);
			break;
		case 0x8118:
		case 0x9ae8:
			s3.mmio_9ae8 = (s3.mmio_9ae8 & 0xff00) | data;
			break;
		case 0x8119:
		case 0x9ae9:
			s3.mmio_9ae8 = (s3.mmio_9ae8 & 0x00ff) | (data << 8);
			dev->ibm8514_cmd_w(space,0,s3.mmio_9ae8,0xffff);
			break;
		case 0x8120:
		case 0xa2e8:
			dev->ibm8514.bgcolour = (dev->ibm8514.bgcolour & 0xff00) | data;
			break;
		case 0x8121:
		case 0xa2e9:
			dev->ibm8514.bgcolour = (dev->ibm8514.bgcolour & 0x00ff) | (data << 8);
			break;
		case 0x8124:
		case 0xa6e8:
			dev->ibm8514.fgcolour = (dev->ibm8514.fgcolour & 0xff00) | data;
			break;
		case 0x8125:
		case 0xa6e9:
			dev->ibm8514.fgcolour = (dev->ibm8514.fgcolour & 0x00ff) | (data << 8);
			break;
		case 0x8128:
		case 0xaae8:
			dev->ibm8514.write_mask = (dev->ibm8514.write_mask & 0xff00) | data;
			break;
		case 0x8129:
		case 0xaae9:
			dev->ibm8514.write_mask = (dev->ibm8514.write_mask & 0x00ff) | (data << 8);
			break;
		case 0x812c:
		case 0xaee8:
			dev->ibm8514.read_mask = (dev->ibm8514.read_mask & 0xff00) | data;
			break;
		case 0x812d:
		case 0xaee9:
			dev->ibm8514.read_mask = (dev->ibm8514.read_mask & 0x00ff) | (data << 8);
			break;
		case 0xb6e8:
		case 0x8134:
			dev->ibm8514.bgmix = (dev->ibm8514.bgmix & 0xff00) | data;
			break;
		case 0x8135:
		case 0xb6e9:
			dev->ibm8514.bgmix = (dev->ibm8514.bgmix & 0x00ff) | (data << 8);
			break;
		case 0x8136:
		case 0xbae8:
			dev->ibm8514.fgmix = (dev->ibm8514.fgmix & 0xff00) | data;
			break;
		case 0x8137:
		case 0xbae9:
			dev->ibm8514.fgmix = (dev->ibm8514.fgmix & 0x00ff) | (data << 8);
			break;
		case 0x8138:
			dev->ibm8514.scissors_top = (dev->ibm8514.scissors_top & 0xff00) | data;
			break;
		case 0x8139:
			dev->ibm8514.scissors_top = (dev->ibm8514.scissors_top & 0x00ff) | (data << 8);
			break;
		case 0x813a:
			dev->ibm8514.scissors_left = (dev->ibm8514.scissors_left & 0xff00) | data;
			break;
		case 0x813b:
			dev->ibm8514.scissors_left = (dev->ibm8514.scissors_left & 0x00ff) | (data << 8);
			break;
		case 0x813c:
			dev->ibm8514.scissors_bottom = (dev->ibm8514.scissors_bottom & 0xff00) | data;
			break;
		case 0x813d:
			dev->ibm8514.scissors_bottom = (dev->ibm8514.scissors_bottom & 0x00ff) | (data << 8);
			break;
		case 0x813e:
			dev->ibm8514.scissors_right = (dev->ibm8514.scissors_right & 0xff00) | data;
			break;
		case 0x813f:
			dev->ibm8514.scissors_right = (dev->ibm8514.scissors_right & 0x00ff) | (data << 8);
			break;
		case 0x8140:
			dev->ibm8514.pixel_control = (dev->ibm8514.pixel_control & 0xff00) | data;
			break;
		case 0x8141:
			dev->ibm8514.pixel_control = (dev->ibm8514.pixel_control & 0x00ff) | (data << 8);
			break;
		case 0x8146:
			dev->ibm8514.multifunc_sel = (dev->ibm8514.multifunc_sel & 0xff00) | data;
			break;
		case 0x8148:
			dev->ibm8514.rect_height = (dev->ibm8514.rect_height & 0xff00) | data;
			break;
		case 0x8149:
			dev->ibm8514.rect_height = (dev->ibm8514.rect_height & 0x00ff) | (data << 8);
			break;
		case 0x814a:
			dev->ibm8514.rect_width = (dev->ibm8514.rect_width & 0xff00) | data;
			break;
		case 0x814b:
			dev->ibm8514.rect_width = (dev->ibm8514.rect_width & 0x00ff) | (data << 8);
			break;
		case 0x8150:
			dev->ibm8514.pixel_xfer = (dev->ibm8514.pixel_xfer & 0xffffff00) | data;
			if(dev->ibm8514.state == IBM8514_DRAWING_RECT)
				dev->ibm8514_wait_draw();
			break;
		case 0x8151:
			dev->ibm8514.pixel_xfer = (dev->ibm8514.pixel_xfer & 0xffff00ff) | (data << 8);
			if(dev->ibm8514.state == IBM8514_DRAWING_RECT)
				dev->ibm8514_wait_draw();
			break;
		case 0x8152:
			dev->ibm8514.pixel_xfer = (dev->ibm8514.pixel_xfer & 0xff00ffff) | (data << 16);
			if(dev->ibm8514.state == IBM8514_DRAWING_RECT)
				dev->ibm8514_wait_draw();
			break;
		case 0x8153:
			dev->ibm8514.pixel_xfer = (dev->ibm8514.pixel_xfer & 0x00ffffff) | (data << 24);
			if(dev->ibm8514.state == IBM8514_DRAWING_RECT)
				dev->ibm8514_wait_draw();
			break;
		case 0xbee8:
			s3.mmio_bee8 = (s3.mmio_bee8 & 0xff00) | data;
			break;
		case 0xbee9:
			s3.mmio_bee8 = (s3.mmio_bee8 & 0x00ff) | (data << 8);
			dev->ibm8514_multifunc_w(space,0,s3.mmio_bee8,0xffff);
			break;
		case 0x96e8:
			s3.mmio_96e8 = (s3.mmio_96e8 & 0xff00) | data;
			break;
		case 0x96e9:
			s3.mmio_96e8 = (s3.mmio_96e8 & 0x00ff) | (data << 8);
			dev->ibm8514_width_w(space,0,s3.mmio_96e8,0xffff);
			break;
		case 0xe2e8:
			dev->ibm8514.pixel_xfer = (dev->ibm8514.pixel_xfer & 0xffffff00) | data;
			dev->ibm8514_wait_draw();
			break;
		default:
			if(LOG_8514) logerror("S3: MMIO offset %05x write %02x\n",offset+0xa0000,data);
		}
		return;
	}

	if (svga.rgb8_en || svga.rgb15_en || svga.rgb16_en || svga.rgb32_en)
	{
		//printf("%08x %02x (%02x %02x) %02X\n",offset,data,vga.sequencer.map_mask,svga.bank_w,(vga.sequencer.data[4] & 0x08));
		if(offset & 0x10000)
			return;
		if(vga.sequencer.data[4] & 0x8)
		{
			if((offset + (svga.bank_w*0x10000)) < vga.svga_intf.vram_size)
				vga.memory[(offset + (svga.bank_w*0x10000))] = data;
		}
		else
		{
			int i;
			for(i=0;i<4;i++)
			{
				if(vga.sequencer.map_mask & 1 << i)
				{
					if((offset*4+i+(svga.bank_w*0x10000)) < vga.svga_intf.vram_size)
						vga.memory[(offset*4+i+(svga.bank_w*0x10000))] = data;
				}
			}
		}
		return;
	}

	if((offset + (svga.bank_w*0x10000)) < vga.svga_intf.vram_size)
		vga_device::mem_w(space,offset,data,mem_mask);
}

/******************************************

gamtor.c implementation (TODO: identify the video card)

******************************************/

READ8_MEMBER(gamtor_vga_device::mem_r)
{
	return vga.memory[offset];
}

WRITE8_MEMBER(gamtor_vga_device::mem_w)
{
	vga.memory[offset] = data;
}


READ8_MEMBER(gamtor_vga_device::port_03b0_r)
{
	UINT8 res;

	switch(offset)
	{
		default:
			res = vga_device::port_03b0_r(space,offset ^ 3,mem_mask);
			break;
	}

	return res;
}

WRITE8_MEMBER(gamtor_vga_device::port_03b0_w)
{
	switch(offset)
	{
		default:
			vga_device::port_03b0_w(space,offset ^ 3,data,mem_mask);
			break;
	}
}

READ8_MEMBER(gamtor_vga_device::port_03c0_r)
{
	UINT8 res;

	switch(offset)
	{
		default:
			res = vga_device::port_03c0_r(space,offset ^ 3,mem_mask);
			break;
	}

	return res;
}

WRITE8_MEMBER(gamtor_vga_device::port_03c0_w)
{
	switch(offset)
	{
		default:
			vga_device::port_03c0_w(space,offset ^ 3,data,mem_mask);
			break;
	}
}

READ8_MEMBER(gamtor_vga_device::port_03d0_r)
{
	UINT8 res;

	switch(offset)
	{
		default:
			res = vga_device::port_03d0_r(space,offset ^ 3,mem_mask);
			break;
	}

	return res;
}

WRITE8_MEMBER(gamtor_vga_device::port_03d0_w)
{
	switch(offset)
	{
		default:
			vga_device::port_03d0_w(space,offset ^ 3,data,mem_mask);
			break;
	}
}

UINT16 ati_vga_device::offset()
{
	//popmessage("Offset: %04x  %s %s %s %s",vga.crtc.offset,vga.crtc.dw?"DW":"--",vga.crtc.word_mode?"BYTE":"WORD",(ati.ext_reg[0x33] & 0x40) ? "PEL" : "---",(ati.ext_reg[0x30] & 0x20) ? "256" : "---");
	if(ati.ext_reg[0x30] & 0x20)  // likely wrong, gets 640x400/480 SVGA and tweaked 256 colour modes displaying correctly in Fractint.
		return vga_device::offset() << 3;
	if(ati.ext_reg[0x33] & 0x40)
		return vga_device::offset() << 2;
	return vga_device::offset();
}


void ati_vga_device::ati_define_video_mode()
{
	int clock;
	UINT8 clock_type;
	int div = ((ati.ext_reg[0x38] & 0xc0) >> 6) + 1;
	int divisor = 1;

	svga.rgb8_en = 0;
	svga.rgb15_en = 0;
	svga.rgb16_en = 0;
	svga.rgb32_en = 0;

	if(ati.ext_reg[0x30] & 0x20)
		svga.rgb8_en = 1;

	clock_type = ((ati.ext_reg[0x3e] & 0x10)>>1) | ((ati.ext_reg[0x39] & 0x02)<<1) | ((vga.miscellaneous_output & 0x0c)>>2);
	switch(clock_type)
	{
	case 0:
		clock = XTAL_42_9545MHz;
		break;
	case 1:
		clock = 48771000;
		break;
	case 2:
		clock = 16657000;
		break;
	case 3:
		clock = XTAL_36MHz;
		break;
	case 4:
		clock = 50350000;
		break;
	case 5:
		clock = 56640000;
		break;
	case 6:
		clock = 28322000;
		break;
	case 7:
		clock = 44900000;
		break;
	case 8:
		clock = 30240000;
		break;
	case 9:
		clock = XTAL_32MHz;
		break;
	case 10:
		clock = 37500000;
		break;
	case 11:
		clock = 39000000;
		break;
	case 12:
		clock = XTAL_40MHz;
		break;
	case 13:
		clock = 56644000;
		break;
	case 14:
		clock = 75000000;
		break;
	case 15:
		clock = 65000000;
		break;
	default:
		clock = XTAL_42_9545MHz;
		logerror("Invalid dot clock %i selected.\n",clock_type);
	}
//  logerror("ATI: Clock select type %i (%iHz / %i)\n",clock_type,clock,div);
	recompute_params_clock(divisor,clock / div);
}

READ8_MEMBER(ati_vga_device::mem_r)
{
	if(svga.rgb8_en || svga.rgb15_en || svga.rgb16_en || svga.rgb24_en)
	{
		if(ati.ext_reg[0x3d] & 0x04)
		{
			offset &= 0x1ffff;
			return vga.memory[(offset+svga.bank_r*0x20000) % vga.svga_intf.vram_size];
		}
		else
		{
			offset &= 0xffff;
			return vga.memory[(offset+svga.bank_r*0x10000) % vga.svga_intf.vram_size];
		}
	}

	return vga_device::mem_r(space,offset,mem_mask);
}

WRITE8_MEMBER(ati_vga_device::mem_w)
{
	if(svga.rgb8_en || svga.rgb15_en || svga.rgb16_en || svga.rgb24_en)
	{
		if(ati.ext_reg[0x3d] & 0x04)
		{
			offset &= 0x1ffff;
			vga.memory[(offset+svga.bank_w*0x20000) % vga.svga_intf.vram_size] = data;
		}
		else
		{
			offset &= 0xffff;
			vga.memory[(offset+svga.bank_w*0x10000) % vga.svga_intf.vram_size] = data;
		}
	}
	else
		vga_device::mem_w(space,offset,data,mem_mask);
}


READ8_MEMBER(ati_vga_device::ati_port_ext_r)
{
	UINT8 ret = 0xff;

	switch(offset)
	{
	case 0:
		break;
	case 1:
		switch(ati.ext_reg_select)
		{
		case 0x20:
			ret = 0x10;  // 512kB memory
			break;
		case 0x28:  // Vertical line counter (high)
			ret = (machine().first_screen()->vpos() >> 8) & 0x03;
			break;
		case 0x29:  // Vertical line counter (low)
			ret = machine().first_screen()->vpos() & 0xff;  // correct?
			break;
		case 0x2a:
			ret = ati.vga_chip_id;  // Chip revision (6 for the 28800-6, 5 for the 28800-5)
			break;
		case 0x37:
			{
				eeprom_serial_93cxx_device* eep = subdevice<eeprom_serial_93cxx_device>("ati_eeprom");
				ret = 0x00;
				ret |= eep->do_read() << 3;
			}
			break;
		case 0x3d:
			ret = ati.ext_reg[ati.ext_reg_select] & 0x0f;
			ret |= 0x10;  // EGA DIP switch emulation
			break;
		default:
			ret = ati.ext_reg[ati.ext_reg_select];
			logerror("ATI: Extended VGA register 0x01CE index %02x read\n",ati.ext_reg_select);
		}
		break;
	}
	return ret;
}

WRITE8_MEMBER(ati_vga_device::ati_port_ext_w)
{
	switch(offset)
	{
	case 0:
		ati.ext_reg_select = data & 0x3f;
		break;
	case 1:
		ati.ext_reg[ati.ext_reg_select] = data;
		switch(ati.ext_reg_select)
		{
		case 0x23:
			vga.crtc.start_addr_latch = (vga.crtc.start_addr_latch & 0xfffdffff) | ((data & 0x10) << 13);
			vga.crtc.cursor_addr = (vga.crtc.cursor_addr & 0xfffdffff) | ((data & 0x08) << 14);
			logerror("ATI: ATI23 write %02x\n",data);
			break;
		case 0x2d:
			if(data & 0x08)
			{
				vga.crtc.horz_total = (vga.crtc.horz_total & 0x00ff) | (data & 0x01) << 8;
				// bit 1 = bit 8 of horizontal blank start
				// bit 2 = bit 8 of horizontal retrace start
			}
			logerror("ATI: ATI2D (extensions) write %02x\n",data);
			break;
		case 0x30:
			vga.crtc.start_addr_latch = (vga.crtc.start_addr_latch & 0xfffeffff) | ((data & 0x40) << 10);
			vga.crtc.cursor_addr = (vga.crtc.cursor_addr & 0xfffeffff) | ((data & 0x04) << 14);
			logerror("ATI: ATI30 write %02x\n",data);
			break;
		case 0x32:  // memory page select
			if(ati.ext_reg[0x3e] & 0x08)
			{
				svga.bank_r = ((data & 0x01) << 3) | ((data & 0xe0) >> 5);
				svga.bank_w = ((data & 0x1e) >> 1);
			}
			else
			{
				svga.bank_r = ((data & 0x1e) >> 1);
				svga.bank_w = ((data & 0x1e) >> 1);
			}
			//logerror("ATI: Memory Page Select write %02x (read: %i write %i)\n",data,svga.bank_r,svga.bank_w);
			break;
		case 0x33:  // EEPROM
			if(data & 0x04)
			{
				eeprom_serial_93cxx_device* eep = subdevice<eeprom_serial_93cxx_device>("ati_eeprom");
				if(eep != NULL)
				{
					eep->di_write((data & 0x01) ? ASSERT_LINE : CLEAR_LINE);
					eep->clk_write((data & 0x02) ? ASSERT_LINE : CLEAR_LINE);
					eep->cs_write((data & 0x08) ? ASSERT_LINE : CLEAR_LINE);
				}
			}
			else
				logerror("ATI: ATI33 write %02x\n",data);
			break;
		default:
			logerror("ATI: Extended VGA register 0x01CE index %02x write %02x\n",ati.ext_reg_select,data);
		}
		break;
	}
	ati_define_video_mode();
}

/*
02E8h W(R):  Display Status Register
bit     0  SENSE is the result of a wired-OR of 3 comparators, one
           for each of the RGB video signal.
           By programming the RAMDAC for various values
           and patterns and then reading the SENSE, the monitor type
           (color, monochrome or none) can be determined.
        1  VBLANK. Vertical Blank State
           If Vertical Blank is active this bit is set.
        2  HORTOG. Horizontal Toggle
           This bit toggles every time a HSYNC pulse starts
     3-15  Reserved(0)
 */
READ16_MEMBER(ibm8514a_device::ibm8514_status_r)
{
	return m_vga->vga_vblank() << 1;
}

WRITE16_MEMBER(ibm8514a_device::ibm8514_htotal_w)
{
	ibm8514.htotal = data & 0x01ff;
	//vga.crtc.horz_total = data & 0x01ff;
	if(LOG_8514) logerror("8514/A: Horizontal total write %04x\n",data);
}

/*
42E8h W(R):  Subsystem Status Register (SUBSYS_STAT)
bit   0-3  Interrupt requests. These bits show the state of internal interrupt
           requests. An interrupt will only occur if the corresponding bit(s)
           in SUBSYS_CNTL is set. Interrupts can only be reset by writing a 1
           to the corresponding Interrupt Clear bit in SUBSYS_CNTL.
             Bit 0: VBLNKFLG
                 1: PICKFLAG
                 2: INVALIDIO
                 3: GPIDLE
      4-6  MONITORID.
              1: IBM 8507 (1024x768) Monochrome
              2: IBM 8514 (1024x768) Color
              5: IBM 8503 (640x480) Monochrome
              6: IBM 8512/13 (640x480) Color
        7  8PLANE.
           (CT82c480) This bit is latched on reset from pin P4D7.
     8-11  CHIP_REV. Chip revision number.
    12-15  (CT82c480) CHIP_ID. 0=CT 82c480.
 */
READ16_MEMBER(ibm8514a_device::ibm8514_substatus_r)
{
	// TODO:
	if(m_vga->vga_vblank() != 0)  // not correct, but will do for now
		ibm8514.substatus |= 0x01;
	return ibm8514.substatus;
}

/*
42E8h W(W):  Subsystem Control Register (SUBSYS_CNTL)
bit   0-3  Interrupt Reset. Write 1 to a bit to reset the interrupt.
           Bit 0  RVBLNKFLG   Write 1 to reset Vertical Blank interrupt.
               1  RPICKFLAG   Write 1 to reset PICK interrupt.
               2  RINVALIDIO  Write 1 to reset Queue Overflow/Data
                              Underflow interrupt.
               3  RGPIDLE     Write 1 to reset GPIDLE interrupt.
      4-7  Reserved(0)
        8  IBLNKFLG.   If set Vertical Blank Interrupts are enabled.
        9  IPICKFLAG.  If set PICK Interrupts are enabled.
       10  IINVALIDIO. If set Queue Overflow/Data Underflow Interrupts are
                       enabled.
       11  IGPIDLE.    If set Graphics Engine Idle Interrupts are enabled.
    12-13  CHPTEST. Used for chip testing.
    14-15  Graphics Processor Control (GPCTRL).
 */
WRITE16_MEMBER(ibm8514a_device::ibm8514_subcontrol_w)
{
	ibm8514.subctrl = data;
	ibm8514.substatus &= ~(data & 0x0f);  // reset interrupts
//  if(LOG_8514) logerror("8514/A: Subsystem control write %04x\n",data);
}

READ16_MEMBER(ibm8514a_device::ibm8514_subcontrol_r)
{
	return ibm8514.subctrl;
}

READ16_MEMBER(ibm8514a_device::ibm8514_htotal_r)
{
	return ibm8514.htotal;
}

READ16_MEMBER(ibm8514a_device::ibm8514_vtotal_r)
{
	return ibm8514.vtotal;
}

WRITE16_MEMBER(ibm8514a_device::ibm8514_vtotal_w)
{
	ibm8514.vtotal = data;
//  vga.crtc.vert_total = data;
	if(LOG_8514) logerror("8514/A: Vertical total write %04x\n",data);
}

READ16_MEMBER(ibm8514a_device::ibm8514_vdisp_r)
{
	return ibm8514.vdisp;
}

WRITE16_MEMBER(ibm8514a_device::ibm8514_vdisp_w)
{
	ibm8514.vdisp = data;
//  vga.crtc.vert_disp_end = data >> 3;
	if(LOG_8514) logerror("8514/A: Vertical Displayed write %04x\n",data);
}

READ16_MEMBER(ibm8514a_device::ibm8514_vsync_r)
{
	return ibm8514.vsync;
}

WRITE16_MEMBER(ibm8514a_device::ibm8514_vsync_w)
{
	ibm8514.vsync = data;
	if(LOG_8514) logerror("8514/A: Vertical Sync write %04x\n",data);
}

void ibm8514a_device::enabled()
{
	ibm8514.state = IBM8514_IDLE;
	ibm8514.gpbusy = false;
}

READ16_MEMBER(mach8_device::mach8_ec0_r)
{
	return ibm8514.ec0;
}

WRITE16_MEMBER(mach8_device::mach8_ec0_w)
{
	ibm8514.ec0 = data;
	if(LOG_8514) logerror("8514/A: Extended configuration 0 write %04x\n",data);
}

READ16_MEMBER(mach8_device::mach8_ec1_r)
{
	return ibm8514.ec1;
}

WRITE16_MEMBER(mach8_device::mach8_ec1_w)
{
	ibm8514.ec1 = data;
	if(LOG_8514) logerror("8514/A: Extended configuration 1 write %04x\n",data);
}

READ16_MEMBER(mach8_device::mach8_ec2_r)
{
	return ibm8514.ec2;
}

WRITE16_MEMBER(mach8_device::mach8_ec2_w)
{
	ibm8514.ec2 = data;
	if(LOG_8514) logerror("8514/A: Extended configuration 2 write %04x\n",data);
}

READ16_MEMBER(mach8_device::mach8_ec3_r)
{
	return ibm8514.ec3;
}

WRITE16_MEMBER(mach8_device::mach8_ec3_w)
{
	ibm8514.ec3 = data;
	if(LOG_8514) logerror("8514/A: Extended configuration 3 write %04x\n",data);
}

READ16_MEMBER(mach8_device::mach8_ext_fifo_r)
{
	return 0x00;  // for now, report all FIFO slots at free
}

WRITE16_MEMBER(mach8_device::mach8_linedraw_index_w)
{
	mach8.linedraw = data & 0x07;
	if(LOG_8514) logerror("Mach8: Line Draw Index write %04x\n",data);
}

READ16_MEMBER(mach8_device::mach8_bresenham_count_r)
{
	return ibm8514.rect_width & 0x1fff;
}

WRITE16_MEMBER(mach8_device::mach8_bresenham_count_w)
{
	ibm8514.rect_width = data & 0x1fff;
	if(LOG_8514) logerror("Mach8: Bresenham count write %04x\n",data);
}

READ16_MEMBER(mach8_device::mach8_linedraw_r)
{
	return 0xff;
}

WRITE16_MEMBER(mach8_device::mach8_linedraw_w)
{
	// TODO: actually draw the lines
	switch(mach8.linedraw)
	{
	case 0:  // Set current X
		ibm8514.curr_x = data;
		mach8.linedraw++;
		break;
	case 1:  // Set current Y
		ibm8514.curr_y = data;
		mach8.linedraw++;
		break;
	case 2:  // Line end X
		ibm8514.curr_x = data;
		mach8.linedraw++;
		break;
	case 3:  // Line end Y
		ibm8514.curr_y = data;
		mach8.linedraw = 2;
		break;
	case 4:  // Set current X
		ibm8514.curr_x = data;
		mach8.linedraw++;
		break;
	case 5:  // Set current Y
		ibm8514.curr_y = data;
		mach8.linedraw = 4;
		break;
	}
	logerror("ATI: Linedraw register write %04x, mode %i\n",data,mach8.linedraw);
}

READ16_MEMBER(mach8_device::mach8_sourcex_r)
{
	return ibm8514.dest_x;
}

READ16_MEMBER(mach8_device::mach8_sourcey_r)
{
	return ibm8514.dest_y;
}

WRITE16_MEMBER(mach8_device::mach8_ext_leftscissor_w)
{
	// TODO
}

WRITE16_MEMBER(mach8_device::mach8_ext_topscissor_w)
{
	// TODO
}

READ16_MEMBER(mach8_device::mach8_scratch0_r)
{
	return mach8.scratch0;
}

WRITE16_MEMBER(mach8_device::mach8_scratch0_w)
{
	mach8.scratch0 = data;
	if(LOG_8514) logerror("Mach8: Scratch Pad 0 write %04x\n",data);
}

READ16_MEMBER(mach8_device::mach8_scratch1_r)
{
	return mach8.scratch1;
}

WRITE16_MEMBER(mach8_device::mach8_scratch1_w)
{
	mach8.scratch1 = data;
	if(LOG_8514) logerror("Mach8: Scratch Pad 1 write %04x\n",data);
}

/*
12EEh W(R):  Configuration Status 1 Register                           (Mach8)
bit    0  CLK_MODE. Set to use clock chip, clear to use crystals.
       1  BUS_16. Set for 16bit bus, clear for 8bit bus
       2  MC_BUS. Set for MicroChannel bus, clear for ISA/EISA bus
       3  EEPROM_ENA. EEPROM enabled if set
       4  DRAM_ENA. Set for DRAM, clear for VRAM.
     5-6  MEM_INSTALLED. Video memory. 0: 512K, 1: 1024K
       7  ROM_ENA. Set is ROM is enabled
       8  ROM_PAGE_ENA. Set if ROM paging enabled
    9-15  ROM_LOCATION. If bit 2 and 3 are 0 the ROM will be at this location:
           0: C000h, 1: C080h, 2: C100h, .. 127: FF80h (unlikely)
 */
READ16_MEMBER(mach8_device::mach8_config1_r)
{
	return 0x0082;
}

/*
16EEh (R):  Configuration Status 2 Register                            (Mach8)
bit    0  SHARE_CLOCK. If set the Mach8 shares clock with the VGA
       1  HIRES_BOOT. Boot in hi-res mode if set
       2  EPROM_16_ENA. Adapter configured for 16bit ROM if set
       3  WRITE_PER_BIT. Write masked VRAM operations supported if set
       4  FLASH_ENA. Flash page writes supported if set
 */
READ16_MEMBER(mach8_device::mach8_config2_r)
{
	return 0x0002;
}
