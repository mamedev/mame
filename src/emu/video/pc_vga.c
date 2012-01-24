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
    - convert to an ISA device.
    - add emulated mc6845 hook-up
    - fix video update.
	- rewrite video drawing functions (they are horrible)
	- fix RAM read/writes, CGA and Mono has video bugs due of corrupted vga.memory
	- fix emulated CGA and Mono modes
	- add VESA etc.
    - (and many more ...)

	per-game issues:
	- The Incredible Machine: fix partial updates
	- MAME 0.01: fix 92 Hz refresh rate bug (uses VESA register?).

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

static UINT8 color_bitplane_to_packed[4/*plane*/][8/*pixel*/][256];

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
		UINT8 *data;
		UINT8 latch[4];
	} gc;
	struct { UINT8 index, data[0x15]; int state; } attribute;


	struct {
		UINT8 read_index, write_index, mask;
		int read;
		int state;
		struct { UINT8 red, green, blue; } color[0x100];
		int dirty;
	} dac;

	struct {
		int time;
		int visible;
	} cursor;

	/* oak vga */
	struct { UINT8 reg; } oak;

	int log;
} vga;


#define REG(x) vga.crtc.data[x]

#define CRTC_CHAR_HEIGHT ((REG(9)&0x1f)+1)
#define CRTC_CURSOR_MODE (REG(0xa)&0x60)
#define CRTC_CURSOR_OFF 0x20
#define CRTC_SKEW	(REG(8)&15)
#define CRTC_CURSOR_POS ((REG(0xe)<<8)|REG(0xf))
#define CRTC_CURSOR_TOP	(REG(0xa)&0x1f)
#define CRTC_CURSOR_BOTTOM REG(0xb)

#define CRTC_PORT_ADDR ((vga.miscellaneous_output&1)?0x3d0:0x3b0)

#define LINES_HELPER ( (vga.crtc.data[0x12] \
				|((vga.crtc.data[7]&2)<<7) \
				|((vga.crtc.data[7]&0x40)<<3))+1 )
//#define TEXT_LINES (LINES_HELPER)
#define LINES (LINES_HELPER)
#define TEXT_LINES (LINES_HELPER >> ((vga.crtc.data[9]&0x80) ? 1 : 0))

#define GRAPHIC_MODE (vga.gc.data[6]&1) /* else textmodus */

#define EGA_COLUMNS (vga.crtc.data[1]+1)
#define EGA_START_ADDRESS ((vga.crtc.data[0xd]|(vga.crtc.data[0xc]<<8))<<2)
#define EGA_LINE_LENGTH (vga.crtc.data[0x13]<<3)

#define VGA_COLUMNS (EGA_COLUMNS>>1)
#define VGA_START_ADDRESS (EGA_START_ADDRESS)
#define VGA_LINE_LENGTH (EGA_LINE_LENGTH<<2)

#define CHAR_WIDTH ((vga.sequencer.data[1]&1)?8:9)

#define TEXT_COLUMNS (vga.crtc.data[1]+1)
#define TEXT_START_ADDRESS (EGA_START_ADDRESS)
#define TEXT_LINE_LENGTH (EGA_LINE_LENGTH>>2)

#define TEXT_COPY_9COLUMN(ch) ((ch>=192)&&(ch<=223)&&(vga.attribute.data[0x10]&4))

#define FONT1 (  ((vga.sequencer.data[3]&0x3)    |((vga.sequencer.data[3]&0x10)<<2))*0x2000 )
#define FONT2 ( (((vga.sequencer.data[3]&0xc)>>2)|((vga.sequencer.data[3]&0x20)<<3))*0x2000 )

static int pc_current_height;
static int pc_current_width;

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
//  pc_choosevideomode = choosevideomode;
	pc_current_height = -1;
	pc_current_width = -1;
}

static void vga_vh_text(running_machine &machine, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	UINT8 ch, attr;
	UINT8 bits;
	UINT8 *font;
	UINT32 *bitmapline;
	int width=CHAR_WIDTH, height=CRTC_CHAR_HEIGHT;
	int pos, line, column, mask, w, h, addr;
	pen_t pen;

	if (CRTC_CURSOR_MODE!=CRTC_CURSOR_OFF)
	{
		if (++vga.cursor.time>=0x10)
		{
			vga.cursor.visible^=1;
			vga.cursor.time=0;
		}
	}

	for (addr = TEXT_START_ADDRESS, line = -CRTC_SKEW; line < TEXT_LINES;
		 line += height, addr += TEXT_LINE_LENGTH)
	{
		for (pos = addr, column=0; column<TEXT_COLUMNS; column++, pos++)
		{
			ch   = vga.memory[(pos<<2) + 0];
			attr = vga.memory[(pos<<2) + 1];
			font = vga.memory+2+(ch<<(5+2))+FONT1;

			for (h = MAX(-line, 0); (h < height) && (line+h < MIN(TEXT_LINES, bitmap.height())); h++)
			{
				bitmapline = &bitmap.pix32(line+h);
				bits = font[h<<2];

				assert(bitmapline);

				for (mask=0x80, w=0; (w<width)&&(w<8); w++, mask>>=1)
				{
					if (bits&mask)
						pen = vga.pens[attr & 0x0f];
					else
						pen = vga.pens[attr >> 4];

					if(!machine.primary_screen->visible_area().contains(column*width+w, line+h))
						continue;
					bitmapline[column*width+w] = pen;

				}
				if (w<width)
				{
					/* 9 column */
					if (TEXT_COPY_9COLUMN(ch)&&(bits&1))
						pen = vga.pens[attr & 0x0f];
					else
						pen = vga.pens[attr >> 4];

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
		 line += height, addr=(addr+EGA_LINE_LENGTH)&0x3ffff)
	{
		for(yi=0;yi<height;yi++)
		{
			bitmapline = &bitmap.pix32(line + yi);

			for (pos=addr, c=0, column=0; column<EGA_COLUMNS; column++, c+=8, pos=(pos+4)&0x3ffff)
			{
				int data[4];

				data[0]=vga.memory[pos];
				data[1]=vga.memory[pos+1]<<1;
				data[2]=vga.memory[pos+2]<<2;
				data[3]=vga.memory[pos+3]<<3;

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
	int xi,xi_h;

	/* line compare is screen sensitive */
	mask_comp = 0x0ff | (LINES & 0x300);

	curr_addr = 0;
	if(vga.sequencer.data[4] & 0x08)
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
				for (pos=curr_addr, c=0, column=0; column<VGA_COLUMNS; column++, c+=0x10, pos+=0x20)
				{
					if(pos + 0x20 > vga.svga_intf.vram_size)
						return;

					for(xi=0;xi<0x10;xi++)
					{
						xi_h = ((xi & 6) >> 1) | ((xi & 8) << 1);
						if(!machine.primary_screen->visible_area().contains(c+xi, line + yi))
							continue;
						bitmapline[c+xi] = machine.pens[vga.memory[pos+xi_h]];
					}
				}
			}
		}
	}
	else
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
				for (pos=curr_addr, c=0, column=0; column<VGA_COLUMNS; column++, c+=0x10, pos+=0x08)
				{
					if(pos + 0x08 > vga.svga_intf.vram_size)
						return;

					for(xi=0;xi<0x10;xi++)
					{
						xi_h = (xi & 0xe) >> 1;
						if(!machine.primary_screen->visible_area().contains(c+xi, line + yi))
							continue;
						bitmapline[c+xi] = machine.pens[vga.memory[pos+xi_h]];
					}
				}
			}
		}
	}
}

static void vga_vh_cga(running_machine &machine, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	UINT32 *bitmapline;
	//int height = vga.crtc.maximum_scan_line * (vga.crtc.scan_doubling + 1);
	int x,y,yi;
	UINT32 addr;
	pen_t pen;

	addr = 0;

	for(y=0;y<200;y++)
	{
		//addr = (y) * test;

		for(x=0;x<640;x+=16)
		{
			for(yi=0;yi<1;yi++)
			{
				bitmapline = &bitmap.pix32(y + yi);

				/* TODO: debug dirty assignment */
				{
					pen = vga.pens[(vga.memory[addr] >> (6)) & 3];
					bitmapline[x+0] = pen;
					pen = vga.pens[(vga.memory[addr] >> (4)) & 3];
					bitmapline[x+1] = pen;
					pen = vga.pens[(vga.memory[addr] >> (2)) & 3];
					bitmapline[x+2] = pen;
					pen = vga.pens[(vga.memory[addr] >> (0)) & 3];
					bitmapline[x+3] = pen;
					pen = vga.pens[(vga.memory[addr+0x4000] >> (6)) & 3];
					bitmapline[x+4] = pen;
					pen = vga.pens[(vga.memory[addr+0x4000] >> (4)) & 3];
					bitmapline[x+5] = pen;
					pen = vga.pens[(vga.memory[addr+0x4000] >> (2)) & 3];
					bitmapline[x+6] = pen;
					pen = vga.pens[(vga.memory[addr+0x4000] >> (0)) & 3];
					bitmapline[x+7] = pen;
					pen = vga.pens[(vga.memory[addr+1] >> (6)) & 3];
					bitmapline[x+8] = pen;
					pen = vga.pens[(vga.memory[addr+1] >> (4)) & 3];
					bitmapline[x+9] = pen;
					pen = vga.pens[(vga.memory[addr+1] >> (2)) & 3];
					bitmapline[x+10] = pen;
					pen = vga.pens[(vga.memory[addr+1] >> (0)) & 3];
					bitmapline[x+11] = pen;
					pen = vga.pens[(vga.memory[addr+1+0x4000] >> (6)) & 3];
					bitmapline[x+12] = pen;
					pen = vga.pens[(vga.memory[addr+1+0x4000] >> (4)) & 3];
					bitmapline[x+13] = pen;
					pen = vga.pens[(vga.memory[addr+1+0x4000] >> (2)) & 3];
					bitmapline[x+14] = pen;
					pen = vga.pens[(vga.memory[addr+1+0x4000] >> (0)) & 3];
					bitmapline[x+15] = pen;
				}
			}

			//popmessage("%02x %02x %02x %02x %02x %02x %02x %02x",vga.memory[0],vga.memory[1],vga.memory[2],vga.memory[3],vga.memory[4],vga.memory[5],vga.memory[6],vga.memory[7]);

			addr+=2;
		}
	}
}

static void vga_vh_mono(running_machine &machine, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	UINT32 *bitmapline;
	//int height = vga.crtc.maximum_scan_line * (vga.crtc.scan_doubling + 1);
	int x,xi,y,yi;
	UINT32 addr;
	pen_t pen;

	addr = 0;

	for(y=0;y<200;y++)
	{
		//addr = (y) * test;

		for(x=0;x<640;x+=8)
		{
			for(yi=0;yi<1;yi++)
			{
				bitmapline = &bitmap.pix32(y + yi);

				for(xi=0;xi<8;xi++)
				{
					pen = vga.pens[(vga.memory[addr] >> (xi)) & 1];
					bitmapline[x+xi] = pen;
				}
			}

			//popmessage("%02x %02x %02x %02x %02x %02x %02x %02x",vga.memory[0],vga.memory[1],vga.memory[2],vga.memory[3],vga.memory[4],vga.memory[5],vga.memory[6],vga.memory[7]);

			addr++;
		}
	}
}



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

		if (vga.svga_intf.choosevideomode)
		{
			return 6;
		}
		else if (!GRAPHIC_MODE)
		{
			//proc = vga_vh_text;
			//*height = TEXT_LINES;
			//*width = TEXT_COLUMNS * CHAR_WIDTH;
			if(vga.crtc.scan_doubling)
				popmessage("Text mode with double scan enabled, contact MAMEdev");

			return 1;
		}
		else if (vga.gc.data[5]&0x40)
		{
			//proc = vga_vh_vga;
			//*height = LINES;
			//*width = VGA_COLUMNS * 8;
			return 2;
		}
		else if (vga.gc.data[5]&0x20)
		{
			return 3;
		}
		else if ((vga.gc.data[6]&0x0c) == 0x0c)
		{
			return 4;
		}
		else
		{
			//proc = vga_vh_ega;
			//*height = LINES;
			//*width = EGA_COLUMNS * 8;
			return 5;
		}
	}

	return 0;
}

SCREEN_UPDATE_RGB32( pc_video )
{
	UINT8 cur_mode = 0;
	int w = 0, h = 0;

	cur_mode = pc_vga_choosevideomode(screen.machine());

	//popmessage("%02x",cur_mode);

	switch(cur_mode)
	{
		case 0: bitmap.fill(get_black_pen(screen.machine()), cliprect);break;
		case 1: vga_vh_text(screen.machine(), bitmap, cliprect); break;
		case 2: vga_vh_vga (screen.machine(), bitmap, cliprect); break;
		case 3: vga_vh_cga (screen.machine(), bitmap, cliprect); break;
		case 4: vga_vh_mono(screen.machine(), bitmap, cliprect); break;
		case 5: vga_vh_ega (screen.machine(), bitmap, cliprect); break;
		case 6: vga.svga_intf.choosevideomode(screen.machine(), bitmap, cliprect, vga.sequencer.data, vga.crtc.data, vga.gc.data, &w, &h); break;
	}

	return 0;
}
/***************************************************************************/

INLINE UINT8 rotate_right(UINT8 val, UINT8 rot)
{
	return (val >> rot) | (val << (8 - rot));
}


INLINE WRITE8_HANDLER(vga_dirty_w)
{
	vga.memory[offset] = data;
}

static READ8_HANDLER(vga_text_r)
{
	int data;
	data=vga.memory[((offset&~1)<<1)|(offset&1)];

	return data;
}

static WRITE8_HANDLER(vga_text_w)
{
	vga_dirty_w(space, ((offset&~1)<<1)|(offset&1),data);
}

INLINE UINT8 ega_bitplane_to_packed(UINT8 *latch, int number)
{
	return color_bitplane_to_packed[0][number][latch[0]]
		|color_bitplane_to_packed[1][number][latch[1]]
		|color_bitplane_to_packed[2][number][latch[2]]
		|color_bitplane_to_packed[3][number][latch[3]];
}

static READ8_HANDLER(vga_ega_r)
{
	int data;
	vga.gc.latch[0]=vga.memory[(offset<<2)];
	vga.gc.latch[1]=vga.memory[(offset<<2)+1];
	vga.gc.latch[2]=vga.memory[(offset<<2)+2];
	vga.gc.latch[3]=vga.memory[(offset<<2)+3];
	if (vga.gc.data[5]&8) {
		data=0;
		if (!(ega_bitplane_to_packed(vga.gc.latch, 0)^(vga.gc.data[2]&0xf&~vga.gc.data[7]))) data|=1;
		if (!(ega_bitplane_to_packed(vga.gc.latch, 1)^(vga.gc.data[2]&0xf&~vga.gc.data[7]))) data|=2;
		if (!(ega_bitplane_to_packed(vga.gc.latch, 2)^(vga.gc.data[2]&0xf&~vga.gc.data[7]))) data|=4;
		if (!(ega_bitplane_to_packed(vga.gc.latch, 3)^(vga.gc.data[2]&0xf&~vga.gc.data[7]))) data|=8;
		if (!(ega_bitplane_to_packed(vga.gc.latch, 4)^(vga.gc.data[2]&0xf&~vga.gc.data[7]))) data|=0x10;
		if (!(ega_bitplane_to_packed(vga.gc.latch, 5)^(vga.gc.data[2]&0xf&~vga.gc.data[7]))) data|=0x20;
		if (!(ega_bitplane_to_packed(vga.gc.latch, 6)^(vga.gc.data[2]&0xf&~vga.gc.data[7]))) data|=0x40;
		if (!(ega_bitplane_to_packed(vga.gc.latch, 7)^(vga.gc.data[2]&0xf&~vga.gc.data[7]))) data|=0x80;
	} else {
		data=vga.gc.latch[vga.gc.data[4]&3];
	}

	return data;
}

INLINE UINT8 vga_latch_helper(UINT8 cpu, UINT8 latch, UINT8 mask)
{
	switch (vga.gc.data[3] & 0x18)
	{
		case 0x00:
			return rotate_right((cpu&mask)|(latch&~mask), vga.gc.data[3] & 0x07);
		case 0x08:
			return rotate_right(((cpu&latch)&mask)|(latch&~mask), vga.gc.data[3] & 0x07);
		case 0x10:
			return rotate_right(((cpu|latch)&mask)|(latch&~mask), vga.gc.data[3] & 0x07);
		case 0x18:
			return rotate_right(((cpu^latch)&mask)|(latch&~mask), vga.gc.data[3] & 0x07);
	}
	return 0; /* must not be reached, suppress compiler warning */
}

INLINE UINT8 vga_latch_write(int offs, UINT8 data)
{
	switch (vga.gc.data[5]&3) {
	case 0:
		if (vga.gc.data[1]&(1<<offs)) {
			return vga_latch_helper( (vga.gc.data[0]&(1<<offs))?vga.gc.data[8]:0,
									  vga.gc.latch[offs],vga.gc.data[8] );
		} else {
			return vga_latch_helper(data, vga.gc.latch[offs], vga.gc.data[8]);
		}
		break;
	case 1:
		return vga.gc.latch[offs];
	case 2:
		if (data&(1<<offs)) {
			return vga_latch_helper(0xff, vga.gc.latch[offs], vga.gc.data[8]);
		} else {
			return vga_latch_helper(0, vga.gc.latch[offs], vga.gc.data[8]);
		}
		break;
	case 3:
		if (vga.gc.data[0]&(1<<offs)) {
			return vga_latch_helper(0xff, vga.gc.latch[offs], data&vga.gc.data[8]);
		} else {
			return vga_latch_helper(0, vga.gc.latch[offs], data&vga.gc.data[8]);
		}
		break;
	}
	return 0; /* must not be reached, suppress compiler warning */
}

static WRITE8_HANDLER(vga_ega_w)
{
	if (vga.sequencer.data[2]&1)
		vga_dirty_w(space, offset<<2, vga_latch_write(0,data));
	if (vga.sequencer.data[2]&2)
		vga_dirty_w(space, (offset<<2)+1, vga_latch_write(1,data));
	if (vga.sequencer.data[2]&4)
		vga_dirty_w(space, (offset<<2)+2, vga_latch_write(2,data));
	if (vga.sequencer.data[2]&8)
		vga_dirty_w(space, (offset<<2)+3, vga_latch_write(3,data));
	if ((offset==0xffff)&&(data==0)) vga.log=1;
}

static  READ8_HANDLER(vga_vga_r)
{
	int data;
	data=vga.memory[((offset&~3)<<2)|(offset&3)];

	return data;
}

static WRITE8_HANDLER(vga_vga_w)
{
	vga_dirty_w(space, ((offset&~3)<<2)|(offset&3),data);
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
	pixel_clock /= 	(((vga.sequencer.data[1]&8) >> 3) + 1);

	refresh  = HZ_TO_ATTOSECONDS(pixel_clock) * (hblank_period) * vblank_period;

	machine.primary_screen->configure((hblank_period), (vblank_period), visarea, refresh );
//	popmessage("%d %d\n",vga.crtc.horz_total * 8,vga.crtc.vert_total);
}

static void crtc_reg_write(running_machine &machine, UINT8 index, UINT8 data)
{
	/* Doom does this */
//	if(vga.crtc.protect_enable && index <= 0x07)
//		printf("write to protected address %02x\n",index);

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
		case 0xa:
			vga.feature_control = data;
			break;

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
			//printf("%02x %02x %d\n",vga.crtc.index,data,space->machine().primary_screen->vpos());
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
				data = 0xff; // TODO: actually undefined
			else if ((vga.attribute.index&0x1f)<sizeof(vga.attribute.data))
				data=vga.attribute.data[vga.attribute.index&0x1f];
			break;

		case 2:
			data = 0;
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
			if (vga.dac.read)
				data = 0;
			else
				data = 3;
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
			if (vga.gc.index < vga.svga_intf.gc_regcount)
				data = vga.gc.data[vga.gc.index];
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
			if(!(vga.attribute.index & 0x20)) // protection bit
				if ((vga.attribute.index&0x1f)<sizeof(vga.attribute.data))
					vga.attribute.data[vga.attribute.index&0x1f]=data;
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
			recompute_params(space->machine());
		}
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
#if 0
				if (vga.dac.write_index==64) {
					int i;
					mame_printf_debug("start palette\n");
					for (i=0;i<64;i++) {
						mame_printf_debug(" 0x%.2x, 0x%.2x, 0x%.2x,\n",
							   vga.dac.color[i].red*4,
							   vga.dac.color[i].green*4,
							   vga.dac.color[i].blue*4);
					}
				}
#endif
			}
		}
		break;
	case 0xe:
		vga.gc.index=data;
		break;
	case 0xf:
		if (LOG_REGISTERS)
		{
			logerror("vga_port_03c0_w(): GC[0x%02X%s] = 0x%02X\n",
				vga.gc.index,
				(vga.gc.index < vga.svga_intf.gc_regcount) ? "" : "?",
				data);
		}
		if (vga.gc.index < vga.svga_intf.gc_regcount)
		{
			vga.gc.data[vga.gc.index] = data;
		}
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
	memset(vga.gc.data, 0, vga.svga_intf.gc_regcount * sizeof(*vga.gc.data));
	memset(vga.gc.latch, 0, sizeof(vga.gc.latch));
	memset(&vga.attribute, 0, sizeof(vga.attribute));
	memset(&vga.dac, 0, sizeof(vga.dac));
	memset(&vga.cursor, 0, sizeof(vga.cursor));
	memset(&vga.oak, 0, sizeof(vga.oak));
	vga.log = 0;

	vga.gc.data[6] = 0xc; /* prevent xtbios excepting vga ram as system ram */
/* amstrad pc1640 bios relies on the position of
   the video memory area,
   so I introduced the reset to switch to b8000 area */
	vga.sequencer.data[4] = 0;

	/* TODO: real defaults */
	vga.crtc.line_compare = 0x3ff;
}

READ8_HANDLER(vga_mem_r)
{
	read8_space_func read_handler;
	if (vga.sequencer.data[4]&8)
	{
		read_handler = vga_vga_r;
	}
	else if (vga.sequencer.data[4] & 4)
	{
		read_handler = vga_ega_r;
	}
	else
	{
		read_handler = vga_text_r;
	}
	switch((vga.gc.data[6] >> 2) & 0x03)
	{
		case 0: return vga.memory[offset & 0x1ffff];
		case 1: return read_handler(space,(offset - 0x00000) & 0x0ffff);
		case 2: return read_handler(space,(offset - 0x10000) & 0x0ffff);
		case 3: return read_handler(space,(offset - 0x18000) & 0x0ffff);
	}
	return 0;
}

WRITE8_HANDLER(vga_mem_w)
{
	write8_space_func write_handler;
	if (vga.sequencer.data[4]&8)
	{
		write_handler = vga_vga_w;
	}
	else if (vga.sequencer.data[4] & 4)
	{
		write_handler = vga_ega_w;
	}
	else
	{
		write_handler = vga_text_w;
	}
	switch((vga.gc.data[6] >> 2) & 0x03)
	{
		case 0: vga.memory[offset & 0x1ffff] = data; break;
		case 1: write_handler(space,(offset - 0x00000) & 0x0ffff,data); break;
		case 2: write_handler(space,(offset - 0x10000) & 0x0ffff,data); break;
		case 3: write_handler(space,(offset - 0x18000) & 0x0ffff,data); break;
	}

}

void pc_vga_init(running_machine &machine, read8_space_func read_dipswitch, const struct pc_svga_interface *svga_intf)
{
	int i, j, k, mask1;

	memset(&vga, 0, sizeof(vga));

	for (k=0;k<4;k++)
	{
		for (mask1=0x80, j=0; j<8; j++, mask1>>=1)
		{
			for  (i=0; i<256; i++)
				color_bitplane_to_packed[k][j][i]=(i&mask1)?(1<<k):0;
		}
	}
	/* copy over interfaces */
	vga.read_dipswitch = read_dipswitch;
	if (svga_intf)
	{
		vga.svga_intf = *svga_intf;

		if (vga.svga_intf.seq_regcount < 0x05)
			fatalerror("Invalid SVGA sequencer register count");
		if (vga.svga_intf.gc_regcount < 0x09)
			fatalerror("Invalid SVGA GC register count");
		if (vga.svga_intf.crtc_regcount < 0x19)
			fatalerror("Invalid SVGA CRTC register count");
	}
	else
	{
		vga.svga_intf.vram_size = 0x40000;
		vga.svga_intf.seq_regcount = 0x05;
		vga.svga_intf.gc_regcount = 0x09;
		vga.svga_intf.crtc_regcount = 0x19;
	}

	vga.memory			= auto_alloc_array(machine, UINT8, vga.svga_intf.vram_size);
	vga.sequencer.data	= auto_alloc_array(machine, UINT8, vga.svga_intf.seq_regcount);
	vga.crtc.data		= auto_alloc_array(machine, UINT8, vga.svga_intf.crtc_regcount);
	vga.gc.data			= auto_alloc_array(machine, UINT8, vga.svga_intf.gc_regcount);
	memset(vga.memory, '\0', vga.svga_intf.vram_size);
	memset(vga.sequencer.data, '\0', vga.svga_intf.seq_regcount);
	memset(vga.crtc.data, '\0', vga.svga_intf.crtc_regcount);
	memset(vga.gc.data, '\0', vga.svga_intf.gc_regcount);

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
