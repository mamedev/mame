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
    - add emulated mc6845 hook-up
    - fix video update.
    - rewrite video drawing functions (they are horrible)
    - add VESA etc.
    - "System Information" UI currently crashes the emulation
    - (and many more ...)

    per-game issues:
    - The Incredible Machine: fix partial updates
    - MAME 0.01: fix 92 Hz refresh rate bug (uses VESA register?).
    - Bio Menace: jerky H scrolling (uses EGA mode)
    - Virtual Pool: ET4k unrecognized;
    - California Chase (calchase): init bug causes messed up chars at POST
      (gfxs works if you soft reset).

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

/***************************************************************************

    Local variables

***************************************************************************/

static struct
{
	read8_space_func read_dipswitch;
	struct pc_svga_interface svga_intf;

	UINT8 *memory;
	UINT32 pens[16]; /* the current 16 pens */

	UINT8 miscellaneous_output;
	UINT8 feature_control;

	struct
	{
		UINT8 index;
		UINT8 *data;
		UINT8 map_mask;
		struct
		{
			UINT8 A, B;
		}char_sel;
	} sequencer;

	/* An empty comment at the start of the line indicates that register is currently unused */
	struct
	{
		UINT8 index;
		UINT8 *data;
		UINT16 horz_total;
		UINT16 horz_disp_end;
/**/	UINT8 horz_blank_start;
/**/	UINT8 horz_blank_end;
/**/	UINT8 horz_retrace_start;
/**/	UINT8 horz_retrace_skew;
/**/	UINT8 horz_retrace_end;
/**/	UINT8 disp_enable_skew;
/**/	UINT8 evra;
		UINT16 vert_total;
		UINT16 vert_disp_end;
/**/	UINT16 vert_retrace_start;
/**/	UINT8 vert_retrace_end;
/**/	UINT16 vert_blank_start;
		UINT16 line_compare;
/**/	UINT16 cursor_addr;
/**/	UINT8 byte_panning;
/**/	UINT8 preset_row_scan;
		UINT8 scan_doubling;
/**/	UINT8 maximum_scan_line;
/**/	UINT8 cursor_enable;
/**/	UINT8 cursor_scan_start;
/**/	UINT8 cursor_skew;
/**/	UINT8 cursor_scan_end;
/**/	UINT16 start_addr;
/**/	UINT8 protect_enable;
/**/	UINT8 bandwidth;
/**/	UINT8 offset;
/**/	UINT8 word_mode;
/**/	UINT8 dw;
/**/	UINT8 div4;
/**/	UINT8 underline_loc;
/**/	UINT8 vert_blank_end;
		UINT8 sync_en;
/**/	UINT8 aw;
/**/	UINT8 div2;
/**/	UINT8 sldiv;
/**/	UINT8 map14;
/**/	UINT8 map13;
	} crtc;

	struct
	{
		UINT8 index;
		UINT8 latch[4];
		UINT8 set_reset;
		UINT8 enable_set_reset;
		UINT8 color_compare;
		UINT8 logical_op;
		UINT8 rotate_count;
		UINT8 shift256;
		UINT8 shift_reg;
		UINT8 read_map_sel;
		UINT8 read_mode;
		UINT8 write_mode;
		UINT8 color_dont_care;
		UINT8 bit_mask;
		UINT8 alpha_dis;
		UINT8 memory_map_sel;
		UINT8 host_oe;
		UINT8 chain_oe;
	} gc;

	struct
	{
		UINT8 index, data[0x15]; int state;
		UINT8 prot_bit;
	} attribute;


	struct {
		UINT8 read_index, write_index, mask;
		int read;
		int state;
		struct { UINT8 red, green, blue; } color[0x100];
		int dirty;
	} dac;

	struct {
		UINT8 visible;
	} cursor;

	/* oak vga */
	struct { UINT8 reg; } oak;
} vga;

static struct
{
	UINT8 bank_r,bank_w;
	UINT8 rgb8_en;
	UINT8 rgb15_en;
	UINT8 id;
}svga;

#define REG(x) vga.crtc.data[x]

#define CRTC_CHAR_HEIGHT ((REG(9)&0x1f)+1)
#define CRTC_CURSOR_MODE (REG(0xa)&0x60)
#define CRTC_CURSOR_OFF 0x20
#define CRTC_SKEW	(REG(8)&15)
#define CRTC_CURSOR_POS ((REG(0xe)<<8)|REG(0xf))
#define CRTC_CURSOR_TOP	(REG(0xa)&0x1f)
#define CRTC_CURSOR_BOTTOM REG(0xb)

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

#define LOG_ACCESSES	0
#define LOG_REGISTERS	0

static VIDEO_RESET( vga );

/***************************************************************************

    MachineDriver stuff

***************************************************************************/

void pc_video_start(running_machine &machine)
{
	// ...

	// Avoid an infinite loop when displaying.  0 is not possible anyway.
	vga.crtc.maximum_scan_line = 1;
}

static void vga_vh_text(running_machine &machine, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	UINT8 ch, attr;
	UINT8 bits;
	UINT32 font_base;
	UINT32 *bitmapline;
	int width=CHAR_WIDTH, height=CRTC_CHAR_HEIGHT * (vga.crtc.scan_doubling + 1);
	int pos, line, column, mask, w, h, addr;
	UINT8 blink_en,fore_col,back_col;
	pen_t pen;

	if(vga.crtc.cursor_enable)
		vga.cursor.visible = machine.primary_screen->frame_number() & 0x10;
	else
		vga.cursor.visible = 0;

	for (addr = vga.crtc.start_addr, line = -CRTC_SKEW; line < TEXT_LINES;
		 line += height, addr += TEXT_LINE_LENGTH)
	{
		for (pos = addr, column=0; column<TEXT_COLUMNS; column++, pos++)
		{
			ch   = vga.memory[(pos<<1) + 0];
			attr = vga.memory[(pos<<1) + 1];
			font_base = 0x40000+(ch<<5);
			font_base += ((attr & 8) ? vga.sequencer.char_sel.B : vga.sequencer.char_sel.A)*0x2000;
			blink_en = (vga.attribute.data[0x10]&8&&machine.primary_screen->frame_number() & 0x20) ? attr & 0x80 : 0;

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

					if(!machine.primary_screen->visible_area().contains(column*width+w, line+h))
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

					if(!machine.primary_screen->visible_area().contains(column*width+w, line+h))
						continue;
					bitmapline[column*width+w] = pen;
				}
			}
			if ((CRTC_CURSOR_MODE!=CRTC_CURSOR_OFF)
				&&vga.cursor.visible&&(pos==CRTC_CURSOR_POS))
			{
				for (h=CRTC_CURSOR_TOP;
					 (h<=CRTC_CURSOR_BOTTOM)&&(h<height)&&(line+h<TEXT_LINES);
					 h++)
				{
					if(!machine.primary_screen->visible_area().contains(column*width, line+h))
						continue;
					bitmap.plot_box(column*width, line+h, width, 1, vga.pens[attr&0xf]);
				}
			}
		}
	}
}

static void vga_vh_ega(running_machine &machine, bitmap_rgb32 &bitmap,  const rectangle &cliprect)
{
	int pos, line, column, c, addr, i, yi;
	int height = vga.crtc.maximum_scan_line * (vga.crtc.scan_doubling + 1);
	UINT32 *bitmapline;
	pen_t pen;

	for (addr=EGA_START_ADDRESS, pos=0, line=0; line<LINES;
		 line += height, addr += EGA_LINE_LENGTH)
	{
		for(yi=0;yi<height;yi++)
		{
			bitmapline = &bitmap.pix32(line + yi);

			for (pos=addr, c=0, column=0; column<EGA_COLUMNS; column++, c+=8, pos=(pos+1)&0xffff)
			{
				int data[4];

				data[0]=vga.memory[(pos & 0xffff)];
				data[1]=vga.memory[(pos & 0xffff)+0x20000]<<1;
				data[2]=vga.memory[(pos & 0xffff)+0x40000]<<2;
				data[3]=vga.memory[(pos & 0xffff)+0x60000]<<3;

				for (i = 7; i >= 0; i--)
				{
					pen = vga.pens[(data[0]&1) | (data[1]&2) | (data[2]&4) | (data[3]&8)];
					if(!machine.primary_screen->visible_area().contains(c+i, line + yi))
						continue;
					bitmapline[c+i] = pen;

					data[0]>>=1;
					data[1]>>=1;
					data[2]>>=1;
					data[3]>>=1;
				}
			}
		}
	}
}

/* TODO: I'm guessing that in 256 colors mode every pixel actually outputs two pixels. Is it right? */
static void vga_vh_vga(running_machine &machine, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	int pos, line, column, c, addr, curr_addr;
	UINT32 *bitmapline;
	UINT16 mask_comp;
	int height = vga.crtc.maximum_scan_line * (vga.crtc.scan_doubling + 1);
	int yi;
	int xi;

	/* line compare is screen sensitive */
	mask_comp = 0x0ff | (LINES & 0x300);

	curr_addr = 0;
	if(!(vga.sequencer.data[4] & 0x08))
	{
		for (addr = VGA_START_ADDRESS, line=0; line<LINES; line+=height, addr+=VGA_LINE_LENGTH/4, curr_addr+=VGA_LINE_LENGTH/4)
		{
			for(yi = 0;yi < height; yi++)
			{
				if((line + yi) < (vga.crtc.line_compare & mask_comp))
					curr_addr = addr;
				if((line + yi) == (vga.crtc.line_compare & mask_comp))
					curr_addr = 0;
				bitmapline = &bitmap.pix32(line + yi);
				addr %= vga.svga_intf.vram_size;
				for (pos=curr_addr, c=0, column=0; column<VGA_COLUMNS; column++, c+=8, pos++)
				{
					if(pos > 0x80000/4)
						return;

					for(xi=0;xi<8;xi++)
					{
						if(!machine.primary_screen->visible_area().contains(c+xi, line + yi))
							continue;
						bitmapline[c+xi] = machine.pens[vga.memory[pos+((xi >> 1)*0x20000)]];
					}
				}
			}
		}
	}
	else
	{
		for (addr = VGA_START_ADDRESS, line=0; line<LINES; line+=height, addr+=VGA_LINE_LENGTH, curr_addr+=VGA_LINE_LENGTH)
		{
			for(yi = 0;yi < height; yi++)
			{
				if((line + yi) < (vga.crtc.line_compare & mask_comp))
					curr_addr = addr;
				if((line + yi) == (vga.crtc.line_compare & mask_comp))
					curr_addr = 0;
				bitmapline = &bitmap.pix32(line + yi);
				addr %= vga.svga_intf.vram_size;
				for (pos=curr_addr, c=0, column=0; column<VGA_COLUMNS; column++, c+=0x10, pos+=0x8)
				{
					if(pos + 0x08 > 0x80000)
						return;

					for(xi=0;xi<0x10;xi++)
					{
						if(!machine.primary_screen->visible_area().contains(c+xi, line + yi))
							continue;
						bitmapline[c+xi] = machine.pens[vga.memory[pos+(xi >> 1)]];
					}
				}
			}
		}
	}
}

static void vga_vh_cga(running_machine &machine, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	UINT32 *bitmapline;
	int height = (vga.crtc.scan_doubling + 1);
	int x,xi,y,yi;
	UINT32 addr;
	pen_t pen;
	int width;

	addr = 0;
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
					if(!machine.primary_screen->visible_area().contains(x+xi, y * height + yi))
						continue;
					bitmapline[x+xi] = pen;
				}
			}

			addr++;
		}
	}
}

static void vga_vh_mono(running_machine &machine, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	UINT32 *bitmapline;
	int height = (vga.crtc.scan_doubling + 1);
	int x,xi,y,yi;
	UINT32 addr;
	pen_t pen;
	int width;

	addr = 0;
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
					if(!machine.primary_screen->visible_area().contains(x+xi, y * height + yi))
						continue;
					bitmapline[x+xi] = pen;
				}
			}

			addr++;
		}
	}
}

static void svga_vh_rgb8(running_machine &machine, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	int pos, line, column, c, addr, curr_addr;
	UINT32 *bitmapline;
	UINT16 mask_comp;
	int height = vga.crtc.maximum_scan_line * (vga.crtc.scan_doubling + 1);
	int yi;
	int xi;

	/* line compare is screen sensitive */
	mask_comp = 0x3ff;

	curr_addr = 0;
	for (addr = VGA_START_ADDRESS, line=0; line<LINES; line+=height, addr+=VGA_LINE_LENGTH, curr_addr+=VGA_LINE_LENGTH)
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
				if(pos + 0x08 > 0x100000)
					return;

				for(xi=0;xi<8;xi++)
				{
					if(!machine.primary_screen->visible_area().contains(c+xi, line + yi))
						continue;
					bitmapline[c+xi] = machine.pens[vga.memory[(pos+(xi))]];
				}
			}
		}
	}
}

static void svga_vh_rgb15(running_machine &machine, bitmap_rgb32 &bitmap, const rectangle &cliprect)
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
	for (addr = TGA_START_ADDRESS, line=0; line<TLINES; line+=height, addr+=TGA_LINE_LENGTH, curr_addr+=TGA_LINE_LENGTH)
	{
		if(line>500) return;
		bitmapline = &bitmap.pix32(line);
		addr %= vga.svga_intf.vram_size;
		for (pos=addr, c=0, column=0; column<TGA_COLUMNS; column++, c+=8, pos+=0x10)
		{
			if(pos + 0x10 > vga.svga_intf.vram_size)
				return;
			for(xi=0,xm=0;xi<8;xi++,xm+=2)
			{
				if(!machine.primary_screen->visible_area().contains(c+xi, line + yi))
					continue;
				bitmapline[c+xi] = IV|(MV(pos+xm)&0x7c00)<<9 |(MV(pos+xm)&0x3e0)<<6|(MV(pos+xm)&0x1f)<<3;
			}
		}
	}
}

enum
{
	SCREEN_OFF = 0,
	TEXT_MODE,
	VGA_MODE,
	EGA_MODE,
	CGA_MODE,
	MONO_MODE,
	RGB8_MODE,
	RGB15_MODE,
	SVGA_HACK
};


static UINT8 pc_vga_choosevideomode(running_machine &machine)
{
	int i;

	if (vga.crtc.sync_en)
	{
		if (vga.dac.dirty)
		{
			for (i=0; i<256;i++)
			{
				palette_set_color_rgb(machine, i,(vga.dac.color[i].red & 0x3f) << 2,
									 (vga.dac.color[i].green & 0x3f) << 2,
									 (vga.dac.color[i].blue & 0x3f) << 2);
			}
			vga.dac.dirty = 0;
		}

		if (vga.attribute.data[0x10] & 0x80)
		{
			for (i=0; i<16;i++)
			{
				vga.pens[i] = machine.pens[(vga.attribute.data[i]&0x0f)
										 |((vga.attribute.data[0x14]&0xf)<<4)];
			}
		}
		else
		{
			for (i=0; i<16;i++)
			{
				vga.pens[i]=machine.pens[(vga.attribute.data[i]&0x3f)
										 |((vga.attribute.data[0x14]&0xc)<<4)];
			}
		}

		if (vga.svga_intf.choosevideomode) // TODO: remove this hack
		{
			return SVGA_HACK;
		}
		else if (svga.rgb8_en)
		{
			return RGB8_MODE;
		}
		else if (svga.rgb15_en&0x30)
		{
			return RGB15_MODE;
		}
		else if (!GRAPHIC_MODE)
		{
			//proc = vga_vh_text;
			//*height = TEXT_LINES;
			//*width = TEXT_COLUMNS * CHAR_WIDTH;

			return TEXT_MODE;
		}
		else if (vga.gc.shift256)
		{
			//proc = vga_vh_vga;
			//*height = LINES;
			//*width = VGA_COLUMNS * 8;
			return VGA_MODE;
		}
		else if (vga.gc.shift_reg)
		{
			// cga
			return CGA_MODE;
		}
		else if (vga.gc.memory_map_sel == 0x03)
		{
			// mono
			return MONO_MODE;
		}
		else
		{
			//proc = vga_vh_ega;
			//*height = LINES;
			//*width = EGA_COLUMNS * 8;
			return EGA_MODE;
		}
	}

	return SCREEN_OFF;
}

SCREEN_UPDATE_RGB32( pc_video )
{
	UINT8 cur_mode = 0;
	int w = 0, h = 0;

	cur_mode = pc_vga_choosevideomode(screen.machine());

	//popmessage("%02x",cur_mode);

	switch(cur_mode)
	{
		case SCREEN_OFF:   bitmap.fill(get_black_pen(screen.machine()), cliprect);break;
		case TEXT_MODE:    vga_vh_text(screen.machine(), bitmap, cliprect); break;
		case VGA_MODE:     vga_vh_vga (screen.machine(), bitmap, cliprect); break;
		case EGA_MODE:     vga_vh_ega (screen.machine(), bitmap, cliprect); break;
		case CGA_MODE:     vga_vh_cga (screen.machine(), bitmap, cliprect); break;
		case MONO_MODE:    vga_vh_mono(screen.machine(), bitmap, cliprect); break;
		case RGB8_MODE:    svga_vh_rgb8(screen.machine(), bitmap, cliprect); break;
		case RGB15_MODE:   svga_vh_rgb15(screen.machine(), bitmap, cliprect); break;
		case SVGA_HACK:    vga.svga_intf.choosevideomode(screen.machine(), bitmap, cliprect, vga.sequencer.data, vga.crtc.data, &w, &h); break;
	}

	return 0;
}
/***************************************************************************/

INLINE UINT8 rotate_right(UINT8 val)
{
	return (val >> vga.gc.rotate_count) | (val << (8 - vga.gc.rotate_count));
}

INLINE UINT8 vga_logical_op(UINT8 data, UINT8 plane, UINT8 mask)
{
	UINT8 res;

	switch(vga.gc.logical_op & 3)
	{
		case 0: /* NONE */
			res = (data & mask) | (vga.gc.latch[plane] & ~mask);
			break;
		case 1: /* AND */
			res = (data | ~mask) & (vga.gc.latch[plane]);
			break;
		case 2: /* OR */
			res = (data & mask) | (vga.gc.latch[plane]);
			break;
		case 3: /* XOR */
			res = (data & mask) ^ (vga.gc.latch[plane]);
			break;
	}

	return res;
}

INLINE UINT8 vga_latch_write(int offs, UINT8 data)
{
	UINT8 res;

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

#if 0
static UINT8 crtc_reg_read(UINT8 index)
{
	UINT8 res;

	res = 0xff;

	switch(index)
	{
		default:
			printf("Unhandled CRTC reg r %02x\n",index);
	}

	return res;
}
#endif

static void recompute_params(running_machine &machine)
{
	int vblank_period,hblank_period;
	attoseconds_t refresh;
	UINT8 hclock_m;
	int pixel_clock;

	hclock_m = (!GRAPHIC_MODE) ? CHAR_WIDTH : 8;

	/* safety check */
	if(!vga.crtc.horz_disp_end || !vga.crtc.vert_disp_end || !vga.crtc.horz_total || !vga.crtc.vert_total)
		return;

	rectangle visarea(0, ((vga.crtc.horz_disp_end + 1) * hclock_m)-1, 0, vga.crtc.vert_disp_end);

	vblank_period = (vga.crtc.vert_total + 2);
	hblank_period = ((vga.crtc.horz_total + 5) * hclock_m);

	/* TODO: 10b and 11b settings aren't known */
	pixel_clock  = (vga.miscellaneous_output & 0xc) ? XTAL_28_63636MHz : XTAL_25_1748MHz;
	pixel_clock /=	(((vga.sequencer.data[1]&8) >> 3) + 1);

	refresh  = HZ_TO_ATTOSECONDS(pixel_clock) * (hblank_period) * vblank_period;

	machine.primary_screen->configure((hblank_period), (vblank_period), visarea, refresh );
//  popmessage("%d %d\n",vga.crtc.horz_total * 8,vga.crtc.vert_total);
}

static void crtc_reg_write(running_machine &machine, UINT8 index, UINT8 data)
{
	/* Doom does this */
//  if(vga.crtc.protect_enable && index <= 0x07)
//      printf("write to protected address %02x\n",index);

	switch(index)
	{
		case 0x00:
			vga.crtc.horz_total = (data & 0xff);
			recompute_params(machine);
			break;
		case 0x01:
			vga.crtc.horz_disp_end = (data & 0xff);
			recompute_params(machine);
			break;
		case 0x02: vga.crtc.horz_blank_start = (data & 0xff); break;
		case 0x03:
			vga.crtc.horz_blank_end &= ~0x1f;
			vga.crtc.horz_blank_end |= data & 0x1f;
			vga.crtc.disp_enable_skew = (data & 0x60) >> 5;
			vga.crtc.evra = (data & 0x80) >> 7;
			break;
		case 0x04:
			vga.crtc.horz_retrace_start = data & 0xff;
			break;
		case 0x05:
			vga.crtc.horz_blank_end &= ~0x20;
			vga.crtc.horz_blank_end |= ((data & 0x80) >> 2);
			vga.crtc.horz_retrace_skew = ((data & 0x60) >> 5);
			vga.crtc.horz_retrace_end = data & 0x1f;
			break;
		case 0x06:
			vga.crtc.vert_total &= ~0xff;
			vga.crtc.vert_total |= data & 0xff;
			recompute_params(machine);
			break;
		case 0x07: // Overflow Register
			vga.crtc.vert_total         &= ~0x300;
			vga.crtc.vert_retrace_start &= ~0x300;
			vga.crtc.vert_disp_end      &= ~0x300;
			vga.crtc.line_compare       &= ~0x100;
			vga.crtc.vert_blank_start   &= ~0x100;
			vga.crtc.vert_retrace_start |= ((data & 0x80) << (9-7));
			vga.crtc.vert_disp_end      |= ((data & 0x40) << (9-6));
			vga.crtc.vert_total         |= ((data & 0x20) << (9-5));
			vga.crtc.line_compare       |= ((data & 0x10) << (8-4));
			vga.crtc.vert_blank_start   |= ((data & 0x08) << (8-3));
			vga.crtc.vert_retrace_start |= ((data & 0x04) << (8-2));
			vga.crtc.vert_disp_end      |= ((data & 0x02) << (8-1));
			vga.crtc.vert_total         |= ((data & 0x01) << (8-0));
			recompute_params(machine);
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
			vga.crtc.start_addr &= ~(0xff << (((index & 1)^1) * 8));
			vga.crtc.start_addr |= (data << (((index & 1)^1) * 8));
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
			break;
		case 0x12:
			vga.crtc.vert_disp_end &= ~0xff;
			vga.crtc.vert_disp_end |= data & 0xff;
			recompute_params(machine);
			break;
		case 0x13:
			vga.crtc.offset = data & 0xff;
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
			//printf("Unhandled CRTC reg w %02x %02x\n",index,data);
			break;
	}
}

static void seq_reg_write(running_machine &machine, UINT8 index, UINT8 data)
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

static READ8_HANDLER(vga_crtc_r)
{
	UINT8 data = 0xff;

	switch (offset) {
	case 4:
		data = vga.crtc.index;
		break;
	case 5:
		if (vga.crtc.index < vga.svga_intf.crtc_regcount)
			data = vga.crtc.data[vga.crtc.index];
		break;
	case 0xa:
		UINT8 hsync,vsync;
		vga.attribute.state = 0;
		data = 0;

		hsync = space->machine().primary_screen->hblank() & 1;
		vsync = space->machine().primary_screen->vblank() & 1;

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

static WRITE8_HANDLER(vga_crtc_w)
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

			if (vga.crtc.index < vga.svga_intf.crtc_regcount)
				vga.crtc.data[vga.crtc.index] = data;

			crtc_reg_write(space->machine(),vga.crtc.index,data);
			//space->machine().primary_screen->update_partial(space->machine().primary_screen->vpos());
			#if 0
			if((vga.crtc.index & 0xfe) != 0x0e)
				printf("%02x %02x %d\n",vga.crtc.index,data,space->machine().primary_screen->vpos());
			#endif
			break;

		case 0xa:
			vga.feature_control = data;
			break;
	}
}



READ8_HANDLER( vga_port_03b0_r )
{
	UINT8 data = 0xff;
	if (CRTC_PORT_ADDR==0x3b0)
		data=vga_crtc_r(space, offset);
	return data;
}

static UINT8 gc_reg_read(running_machine &machine,UINT8 index)
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

READ8_HANDLER( vga_port_03c0_r )
{
	UINT8 data = 0xff;

	switch (offset)
	{
		case 0:
			data = vga.attribute.index;
			break;
		case 1:
			if( vga.attribute.index & 0x20) // protection bit
				data = vga.attribute.index;
			else if ((vga.attribute.index&0x1f)<sizeof(vga.attribute.data))
				data=vga.attribute.data[vga.attribute.index&0x1f];
			break;

		case 2:
			// TODO: in VGA bit 4 is actually always on?
			data = 0x60; // is VGA
			switch ((vga.miscellaneous_output>>2)&3)
			{
				case 3:
					if (vga.read_dipswitch && vga.read_dipswitch(space, 0) & 0x01)
						data |= 0x10;
					break;
				case 2:
					if (vga.read_dipswitch && vga.read_dipswitch(space, 0) & 0x02)
						data |= 0x10;
					break;
				case 1:
					if (vga.read_dipswitch && vga.read_dipswitch(space, 0) & 0x04)
						data |= 0x10;
					break;
				case 0:
					if (vga.read_dipswitch && vga.read_dipswitch(space, 0) & 0x08)
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
			data = gc_reg_read(space->machine(),vga.gc.index);
			break;
	}
	return data;
}

READ8_HANDLER(vga_port_03d0_r)
{
	UINT8 data = 0xff;
	if (CRTC_PORT_ADDR == 0x3d0)
		data = vga_crtc_r(space, offset);
	if(offset == 8)
	{
		logerror("VGA: 0x3d8 read at %08x\n",cpu_get_pc(&space->device()));
		data = 0; // TODO: PC-200 reads back CGA register here, everything else returns open bus OR CGA emulation of register 0x3d8
	}

	return data;
}

WRITE8_HANDLER( vga_port_03b0_w )
{
	if (LOG_ACCESSES)
		logerror("vga_port_03b0_w(): port=0x%04x data=0x%02x\n", offset + 0x3b0, data);

	if (CRTC_PORT_ADDR == 0x3b0)
		vga_crtc_w(space, offset, data);
}

static void attribute_reg_write(UINT8 index, UINT8 data)
{
	if((index & 0x30) == 0)
	{
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
			case 0x13: vga.attribute.data[0x13] = data; break;
			case 0x14: vga.attribute.data[0x14] = data; break;
		}
	}
}

static void gc_reg_write(running_machine &machine,UINT8 index,UINT8 data)
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

WRITE8_HANDLER(vga_port_03c0_w)
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
		recompute_params(space->machine());
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

		seq_reg_write(space->machine(),vga.sequencer.index,data);
		recompute_params(space->machine());
		break;
	case 6:
		vga.dac.mask=data;
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
		gc_reg_write(space->machine(),vga.gc.index,data);
		break;
	}
}



WRITE8_HANDLER(vga_port_03d0_w)
{
	if (LOG_ACCESSES)
		logerror("vga_port_03d0_w(): port=0x%04x data=0x%02x\n", offset + 0x3d0, data);

	if (CRTC_PORT_ADDR == 0x3d0)
		vga_crtc_w(space, offset, data);
}

void pc_vga_reset(running_machine &machine)
{
	/* clear out the VGA structure */
	memset(vga.pens, 0, sizeof(vga.pens));
	vga.miscellaneous_output = 0;
	vga.feature_control = 0;
	vga.sequencer.index = 0;
	memset(vga.sequencer.data, 0, vga.svga_intf.seq_regcount * sizeof(*vga.sequencer.data));
	vga.crtc.index = 0;
	memset(vga.crtc.data, 0, vga.svga_intf.crtc_regcount * sizeof(*vga.crtc.data));
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
}

READ8_HANDLER(vga_mem_r)
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
		if (!space->debugger_access())
		{
			vga.gc.latch[0]=vga.memory[(offset)];
			vga.gc.latch[1]=vga.memory[(offset)+0x20000];
			vga.gc.latch[2]=vga.memory[(offset)+0x40000];
			vga.gc.latch[3]=vga.memory[(offset)+0x60000];
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
				data |= vga.memory[offset+i*0x20000];
		}

		return data;
	}

	return 0;
}

WRITE8_HANDLER(vga_mem_w)
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
				vga.memory[offset+i*0x20000] = (vga.sequencer.data[4] & 4) ? vga_latch_write(i,data) : data;
		}
		return;
	}
}

void pc_vga_init(running_machine &machine, read8_space_func read_dipswitch, const struct pc_svga_interface *svga_intf)
{
	memset(&vga, 0, sizeof(vga));

	/* copy over interfaces */
	vga.read_dipswitch = read_dipswitch;
	if (svga_intf)
	{
		vga.svga_intf = *svga_intf;

		if (vga.svga_intf.seq_regcount < 0x05)
			fatalerror("Invalid SVGA sequencer register count");
		if (vga.svga_intf.crtc_regcount < 0x19)
			fatalerror("Invalid SVGA CRTC register count");
	}
	else
	{
		vga.svga_intf.vram_size = 0x100000;
		vga.svga_intf.seq_regcount = 0x05;
		vga.svga_intf.crtc_regcount = 0x19;
	}

	vga.memory			= auto_alloc_array(machine, UINT8, vga.svga_intf.vram_size);
	vga.sequencer.data	= auto_alloc_array(machine, UINT8, vga.svga_intf.seq_regcount);
	vga.crtc.data		= auto_alloc_array(machine, UINT8, 0x100);
	memset(vga.memory, '\0', vga.svga_intf.vram_size);
	memset(vga.sequencer.data, '\0', vga.svga_intf.seq_regcount);
	memset(vga.crtc.data, '\0', 0x100);

	pc_vga_reset(machine);

}

void pc_vga_io_init(running_machine &machine, address_space *mem_space, offs_t mem_offset, address_space *io_space, offs_t port_offset)
{
	int buswidth;
	UINT64 mask = 0;

	buswidth = machine.firstcpu->memory().space_config(AS_PROGRAM)->m_databus_width;
	switch(buswidth)
	{
		case 8:
			mask = 0;
			break;

		case 16:
			mask = 0xffff;
			break;

		case 32:
			mask = 0xffffffff;
			break;

		case 64:
			mask = -1;
			break;

		default:
			fatalerror("VGA: Bus width %d not supported", buswidth);
			break;
	}
	io_space->install_legacy_readwrite_handler(port_offset + 0x3b0, port_offset + 0x3bf, FUNC(vga_port_03b0_r), FUNC(vga_port_03b0_w), mask);
	io_space->install_legacy_readwrite_handler(port_offset + 0x3c0, port_offset + 0x3cf, FUNC(vga_port_03c0_r), FUNC(vga_port_03c0_w), mask);
	io_space->install_legacy_readwrite_handler(port_offset + 0x3d0, port_offset + 0x3df, FUNC(vga_port_03d0_r), FUNC(vga_port_03d0_w), mask);

	mem_space->install_legacy_readwrite_handler(mem_offset + 0x00000, mem_offset + 0x1ffff, FUNC(vga_mem_r), FUNC(vga_mem_w), mask);
}

VIDEO_START( vga )
{
	int i;
	for (i = 0; i < 0x100; i++)
		palette_set_color_rgb(machine, i, 0, 0, 0);
	pc_video_start(machine);
}

static VIDEO_RESET( vga )
{
	pc_vga_reset(machine);
}



void *pc_vga_memory(void)
{
	return vga.memory;
}

size_t pc_vga_memory_size(void)
{
	return vga.svga_intf.vram_size;
}

MACHINE_CONFIG_FRAGMENT( pcvideo_vga )
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_RAW_PARAMS(XTAL_25_1748MHz,900,0,640,526,0,480)
	MCFG_SCREEN_UPDATE_STATIC(pc_video)

	MCFG_PALETTE_LENGTH(0x100)

	MCFG_VIDEO_START(vga)
	MCFG_VIDEO_RESET(vga)
MACHINE_CONFIG_END

MACHINE_CONFIG_FRAGMENT( pcvideo_vga_isa )
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_RAW_PARAMS(XTAL_25_1748MHz,900,0,640,526,0,480)
	MCFG_SCREEN_UPDATE_STATIC(pc_video)

	MCFG_PALETTE_LENGTH(0x100)
MACHINE_CONFIG_END

/******************************************

Tseng ET4000k implementation

******************************************/

static UINT8 tseng_crtc_reg_read(running_machine &machine, UINT8 index)
{
	UINT8 res;

	res = 0;
	if(index <= 0x18)
		res = vga.crtc.data[index];
	else
	{
		switch(index)
		{
			default:
				res = vga.crtc.data[index];
				//printf("%02x\n",index);
				break;
		}
	}

	return res;
}


static void tseng_crtc_reg_write(running_machine &machine, UINT8 index, UINT8 data)
{
	if(index <= 0x18)
		crtc_reg_write(machine,index,data);
	else
	{
		switch(index)
		{
			default:
				//printf("%02x %02x\n",index,data);
				break;
		}
	}
}

static UINT8 tseng_seq_reg_read(running_machine &machine, UINT8 index)
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

static void tseng_seq_reg_write(running_machine &machine, UINT8 index, UINT8 data)
{
	if(index <= 0x04)
	{
		vga.sequencer.data[vga.sequencer.index] = data;
		seq_reg_write(machine,vga.sequencer.index,data);
		recompute_params(machine);
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

READ8_HANDLER(tseng_et4k_03c0_r)
{
	UINT8 res;

	switch(offset)
	{
		case 0x05:
			res = tseng_seq_reg_read(space->machine(),vga.sequencer.index);
			break;
		case 0x0d:
			res = svga.bank_w & 0xf;
			res |= (svga.bank_r & 0xf) << 4;
			break;
		default:
			res = vga_port_03c0_r(space,offset);
			break;
	}

	return res;
}

WRITE8_HANDLER(tseng_et4k_03c0_w)
{
	switch(offset)
	{
		case 0x05:
			tseng_seq_reg_write(space->machine(),vga.sequencer.index,data);
			break;
		case 0x0d:
			svga.bank_w = data & 0xf;
			svga.bank_r = (data & 0xf0) >> 4;
			break;
		default:
			vga_port_03c0_w(space,offset,data);
			break;
	}
}

READ8_HANDLER(tseng_et4k_03d0_r)
{
	UINT8 res = 0xff;

	if (CRTC_PORT_ADDR == 0x3d0)
	{
		switch(offset)
		{
			case 5:
				res = tseng_crtc_reg_read(space->machine(),vga.crtc.index);
				break;
			default:
				res = vga_port_03d0_r(space,offset);
				break;
		}
	}

	return res;
}

WRITE8_HANDLER(tseng_et4k_03d0_w)
{
	if (CRTC_PORT_ADDR == 0x3d0)
	{
		switch(offset)
		{
			case 5:
				vga.crtc.data[vga.crtc.index] = data;
				tseng_crtc_reg_write(space->machine(),vga.crtc.index,data);
				break;
			/* TODO: investigate about this */
			case 8:
				svga.rgb8_en = data & 0x80;
				break;
			default:
				vga_port_03d0_w(space,offset,data);
				break;
		}
	}
}

READ8_HANDLER( tseng_mem_r )
{
	if(svga.rgb8_en)
	{
		offset &= 0xffff;
		return vga.memory[(offset+svga.bank_r*0x10000)];
	}

	return vga_mem_r(space,offset);
}

WRITE8_HANDLER( tseng_mem_w )
{
	if(svga.rgb8_en)
	{
		offset &= 0xffff;
		vga.memory[(offset+svga.bank_w*0x10000)] = data;
	}
	else
		vga_mem_w(space,offset,data);
}

/******************************************

Trident implementation

******************************************/

void pc_svga_trident_io_init(running_machine &machine, address_space *mem_space, offs_t mem_offset, address_space *io_space, offs_t port_offset)
{
	int buswidth;
	UINT64 mask = 0;

	buswidth = machine.firstcpu->memory().space_config(AS_PROGRAM)->m_databus_width;
	switch(buswidth)
	{
		case 8:
			mask = 0;
			break;

		case 16:
			mask = 0xffff;
			break;

		case 32:
			mask = 0xffffffff;
			break;

		case 64:
			mask = -1;
			break;

		default:
			fatalerror("VGA: Bus width %d not supported", buswidth);
			break;
	}
	io_space->install_legacy_readwrite_handler(port_offset + 0x3b0, port_offset + 0x3bf, FUNC(vga_port_03b0_r), FUNC(vga_port_03b0_w), mask);
	io_space->install_legacy_readwrite_handler(port_offset + 0x3c0, port_offset + 0x3cf, FUNC(trident_03c0_r), FUNC(trident_03c0_w), mask);
	io_space->install_legacy_readwrite_handler(port_offset + 0x3d0, port_offset + 0x3df, FUNC(trident_03d0_r), FUNC(trident_03d0_w), mask);

	mem_space->install_legacy_readwrite_handler(mem_offset + 0x00000, mem_offset + 0x1ffff, FUNC(trident_mem_r), FUNC(trident_mem_w), mask);

	// D3h = TGUI9660XGi
	svga.id = 0xd3; // TODO: hardcoded for California Chase
}

static UINT8 trident_seq_reg_read(running_machine &machine, UINT8 index)
{
	UINT8 res;

	res = 0xff;

	if(index <= 0x04)
		res = vga.sequencer.data[index];
	else
	{
		switch(index)
		{
			case 0x0b:
				res = svga.id;
				// TODO: new mode registers selected
				break;
			case 0x0d:
				res = svga.rgb15_en;
				break;
		}
	}

	return res;
}

static void trident_seq_reg_write(running_machine &machine, UINT8 index, UINT8 data)
{
	if(index <= 0x04)
	{
		vga.sequencer.data[vga.sequencer.index] = data;
		seq_reg_write(machine,vga.sequencer.index,data);
		recompute_params(machine);
	}
	else
	{
		switch(index)
		{
			case 0x0b:
				// TODO: old mode registers selected
				break;
			case 0x0d:
				svga.rgb15_en = data & 0x30; // TODO: doesn't match documentation
				break;
		}
	}
}


READ8_HANDLER(trident_03c0_r)
{
	UINT8 res;

	switch(offset)
	{
		case 0x05:
			res = trident_seq_reg_read(space->machine(),vga.sequencer.index);
			break;
		default:
			res = vga_port_03c0_r(space,offset);
			break;
	}

	return res;
}

WRITE8_HANDLER(trident_03c0_w)
{
	switch(offset)
	{
		case 0x05:
			trident_seq_reg_write(space->machine(),vga.sequencer.index,data);
			break;
		default:
			vga_port_03c0_w(space,offset,data);
			break;
	}
}


READ8_HANDLER(trident_03d0_r)
{
	UINT8 res = 0xff;

	if (CRTC_PORT_ADDR == 0x3d0)
	{
		switch(offset)
		{
			case 8:
				res = svga.bank_w & 0x1f; // TODO: a lot more complex than this
				break;
			default:
				res = vga_port_03d0_r(space,offset);
				break;
		}
	}

	return res;
}

WRITE8_HANDLER(trident_03d0_w)
{
	if (CRTC_PORT_ADDR == 0x3d0)
	{
		switch(offset)
		{
			case 8:
				svga.bank_w = data & 0x1f; // TODO: a lot more complex than this
				break;
			default:
				vga_port_03d0_w(space,offset,data);
				break;
		}
	}
}

READ8_HANDLER( trident_mem_r )
{
	if (svga.rgb15_en & 0x30)
	{
		int data;
		if(offset & 0x10000)  // TODO: old reg mode actually CAN read to the upper bank
			return 0;
		data=vga.memory[offset + (svga.bank_w*0x10000)];
		return data;
	}

	return vga_mem_r(space,offset);
}

WRITE8_HANDLER( trident_mem_w )
{
	if (svga.rgb15_en & 0x30)
	{
		if(offset & 0x10000) // TODO: old reg mode actually CAN write to the upper bank
			return;
		vga.memory[offset + (svga.bank_w*0x10000)]= data;
		return;
	}

	vga_mem_w(space,offset,data);
}
