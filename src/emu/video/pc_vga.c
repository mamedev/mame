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
#include "pc_video.h"

#include "memconv.h"

/***************************************************************************

    Static declarations

***************************************************************************/

#define LOG_ACCESSES	0
#define LOG_REGISTERS	0

static const char* vga_crtc_reg_names[] =
{
	"Horizontal Total Register",
	"End Horizontal Display Register",
	"Start Horizontal Blanking Register",
	"End Horizontal Blanking Register",
	"Start Horizontal Retrace Register",
	"End Horizontal Retrace Register",
	"Vertical Total Register",
	"Overflow Register",
	"Preset Row Scan Register",
	"Maximum Scan Line Register",
	"Cursor Start Register",
	"Cursor End Register",
	"Start Address High Register",
	"Start Address Low Register",
	"Cursor Location High Register",
	"Cursor Location Low Register",
	"Vertical Retrace Start Register",
	"Vertical Retrace End Register",
	"Vertical Display End Register",
	"Offset Register",
	"Underline Location Register",
	"Start Vertical Blanking Register",
	"End Vertical Blanking",
	"CRTC Mode Control Register",
	"Line Compare Register"
};


static PALETTE_INIT( ega );
static PALETTE_INIT( vga );
static VIDEO_START( ega );
static VIDEO_START( vga );
static VIDEO_RESET( ega );
static VIDEO_RESET( vga );

static pc_video_update_proc pc_vga_choosevideomode(running_machine *machine, int *width, int *height);
static pc_video_update_proc pc_ega_choosevideomode(running_machine *machine, int *width, int *height);

/***************************************************************************

    MachineDriver stuff

***************************************************************************/

/* grabbed from dac inited by et4000 bios */
static const unsigned char ega_palette[] =
{
	0x00, 0x00, 0x00,
	0x00, 0x00, 0xa8,
	0x00, 0xa8, 0x00,
	0x00, 0xa8, 0xa8,
	0xa8, 0x00, 0x00,
	0xa8, 0x00, 0xa8,
	0xa8, 0x54, 0x00,
	0xa8, 0xa8, 0xa8,
	0x00, 0x00, 0x00,
	0x00, 0x00, 0xa8,
	0x00, 0xa8, 0x00,
	0x00, 0xa8, 0xa8,
	0xa8, 0x00, 0x00,
	0xa8, 0x00, 0xa8,
	0xa8, 0x54, 0x00,
	0xa8, 0xa8, 0xa8,
	0x54, 0x54, 0x54,
	0x54, 0x54, 0xfc,
	0x54, 0xfc, 0x54,
	0x54, 0xfc, 0xfc,
	0xfc, 0x54, 0x54,
	0xfc, 0x54, 0xfc,
	0xfc, 0xfc, 0x54,
	0xfc, 0xfc, 0xfc,
	0x54, 0x54, 0x54,
	0x54, 0x54, 0xfc,
	0x54, 0xfc, 0x54,
	0x54, 0xfc, 0xfc,
	0xfc, 0x54, 0x54,
	0xfc, 0x54, 0xfc,
	0xfc, 0xfc, 0x54,
	0xfc, 0xfc, 0xfc,
	0x00, 0x00, 0x00,
	0x00, 0x00, 0xa8,
	0x00, 0xa8, 0x00,
	0x00, 0xa8, 0xa8,
	0xa8, 0x00, 0x00,
	0xa8, 0x00, 0xa8,
	0xa8, 0x54, 0x00,
	0xa8, 0xa8, 0xa8,
	0x00, 0x00, 0x00,
	0x00, 0x00, 0xa8,
	0x00, 0xa8, 0x00,
	0x00, 0xa8, 0xa8,
	0xa8, 0x00, 0x00,
	0xa8, 0x00, 0xa8,
	0xa8, 0x54, 0x00,
	0xa8, 0xa8, 0xa8,
	0x54, 0x54, 0x54,
	0x54, 0x54, 0xfc,
	0x54, 0xfc, 0x54,
	0x54, 0xfc, 0xfc,
	0xfc, 0x54, 0x54,
	0xfc, 0x54, 0xfc,
	0xfc, 0xfc, 0x54,
	0xfc, 0xfc, 0xfc,
	0x54, 0x54, 0x54,
	0x54, 0x54, 0xfc,
	0x54, 0xfc, 0x54,
	0x54, 0xfc, 0xfc,
	0xfc, 0x54, 0x54,
	0xfc, 0x54, 0xfc,
	0xfc, 0xfc, 0x54,
	0xfc, 0xfc, 0xfc
};

static const unsigned short vga_colortable[] =
{
     0, 0, 0, 1, 0, 2, 0, 3, 0, 4, 0, 5, 0, 6, 0, 7, 0, 8, 0, 9, 0,10, 0,11, 0,12, 0,13, 0,14, 0,15,
     1, 0, 1, 1, 1, 2, 1, 3, 1, 4, 1, 5, 1, 6, 1, 7, 1, 8, 1, 9, 1,10, 1,11, 1,12, 1,13, 1,14, 1,15,
     2, 0, 2, 1, 2, 2, 2, 3, 2, 4, 2, 5, 2, 6, 2, 7, 2, 8, 2, 9, 2,10, 2,11, 2,12, 2,13, 2,14, 2,15,
     3, 0, 3, 1, 3, 2, 3, 3, 3, 4, 3, 5, 3, 6, 3, 7, 3, 8, 3, 9, 3,10, 3,11, 3,12, 3,13, 3,14, 3,15,
     4, 0, 4, 1, 4, 2, 4, 3, 4, 4, 4, 5, 4, 6, 4, 7, 4, 8, 4, 9, 4,10, 4,11, 4,12, 4,13, 4,14, 4,15,
     5, 0, 5, 1, 5, 2, 5, 3, 5, 4, 5, 5, 5, 6, 5, 7, 5, 8, 5, 9, 5,10, 5,11, 5,12, 5,13, 5,14, 5,15,
     6, 0, 6, 1, 6, 2, 6, 3, 6, 4, 6, 5, 6, 6, 6, 7, 6, 8, 6, 9, 6,10, 6,11, 6,12, 6,13, 6,14, 6,15,
     7, 0, 7, 1, 7, 2, 7, 3, 7, 4, 7, 5, 7, 6, 7, 7, 7, 8, 7, 9, 7,10, 7,11, 7,12, 7,13, 7,14, 7,15,
/* flashing is done by dirtying the videoram buffer positions with attr bit #7 set */
     8, 0, 8, 1, 8, 2, 8, 3, 8, 4, 8, 5, 8, 6, 8, 7, 8, 8, 8, 9, 8,10, 8,11, 8,12, 8,13, 8,14, 8,15,
     9, 0, 9, 1, 9, 2, 9, 3, 9, 4, 9, 5, 9, 6, 9, 7, 9, 8, 9, 9, 9,10, 9,11, 9,12, 9,13, 9,14, 9,15,
    10, 0,10, 1,10, 2,10, 3,10, 4,10, 5,10, 6,10, 7,10, 8,10, 9,10,10,10,11,10,12,10,13,10,14,10,15,
    11, 0,11, 1,11, 2,11, 3,11, 4,11, 5,11, 6,11, 7,11, 8,11, 9,11,10,11,11,11,12,11,13,11,14,11,15,
    12, 0,12, 1,12, 2,12, 3,12, 4,12, 5,12, 6,12, 7,12, 8,12, 9,12,10,12,11,12,12,12,13,12,14,12,15,
    13, 0,13, 1,13, 2,13, 3,13, 4,13, 5,13, 6,13, 7,13, 8,13, 9,13,10,13,11,13,12,13,13,13,14,13,15,
    14, 0,14, 1,14, 2,14, 3,14, 4,14, 5,14, 6,14, 7,14, 8,14, 9,14,10,14,11,14,12,14,13,14,14,14,15,
    15, 0,15, 1,15, 2,15, 3,15, 4,15, 5,15, 6,15, 7,15, 8,15, 9,15,10,15,11,15,12,15,13,15,14,15,15
};

MACHINE_DRIVER_START( pcvideo_vga )
	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(720, 480)
	MDRV_SCREEN_VISIBLE_AREA(0,720-1, 0,480-1)
	MDRV_PALETTE_LENGTH(0x100)
	MDRV_PALETTE_INIT(vga)

	MDRV_VIDEO_START(vga)
	MDRV_VIDEO_RESET(vga)
	MDRV_VIDEO_UPDATE(pc_video)
MACHINE_DRIVER_END

MACHINE_DRIVER_START( pcvideo_pc1640 )
	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(720, 350)
	MDRV_SCREEN_VISIBLE_AREA(0,720-1, 0,350-1)
	MDRV_PALETTE_LENGTH(sizeof(ega_palette) / 3)
	MDRV_PALETTE_INIT(ega)

	MDRV_VIDEO_START(ega)
	MDRV_VIDEO_RESET(ega)
	MDRV_VIDEO_UPDATE(pc_video)
MACHINE_DRIVER_END

/***************************************************************************/

static PALETTE_INIT( ega )
{
	int i;

	for ( i = 0; i < sizeof(ega_palette) / 3; i++ ) {
		palette_set_color_rgb(machine, i, ega_palette[i*3], ega_palette[i*3+1], ega_palette[i*3+2]);
	}
}

static PALETTE_INIT( vga )
{
	int i;
	for (i = 0; i < 0x100; i++)
		palette_set_color_rgb(machine, i, 0, 0, 0);
}

static UINT8 color_bitplane_to_packed[4/*plane*/][8/*pixel*/][256];

static struct
{
	struct pc_vga_interface vga_intf;
	struct pc_svga_interface svga_intf;

	UINT8 *memory;
	UINT8 *fontdirty;
	UINT16 pens[16]; /* the current 16 pens */

	UINT8 miscellaneous_output;
	UINT8 feature_control;
	UINT16 line_compare;  // for split-screen use.

	struct
	{
		UINT8 index;
		UINT8 *data;
	} sequencer;
	struct
	{
		UINT8 index;
		UINT8 *data;
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

	struct {
		int (*get_clock)(void);

		int (*get_lines)(void);
		int (*get_sync_lines)(void);

		int (*get_columns)(void);
		int (*get_sync_columns)(void);

		attotime start_time;
		int retrace;
	} monitor;

	/* oak vga */
	struct { UINT8 reg; } oak;

	int log;
} vga;


// to use the mscrtc6845 macros
#define REG(x) vga.crtc.data[x]

#define DOUBLESCAN ((vga.crtc.data[9]&0x80)||((vga.crtc.data[9]&0x1f)!=0) )
#define CRTC_PORT_ADDR ((vga.miscellaneous_output&1)?0x3d0:0x3b0)

#define CRTC_ON (vga.crtc.data[0x17]&0x80)

#define LINES_HELPER ( (vga.crtc.data[0x12] \
				|((vga.crtc.data[7]&2)<<7) \
				|((vga.crtc.data[7]&0x40)<<3))+1 )
//#define TEXT_LINES (LINES_HELPER)
#define LINES (DOUBLESCAN?LINES_HELPER>>1:LINES_HELPER)
#define TEXT_LINES (LINES_HELPER >> ((vga.crtc.data[9]&0x80) ? 1 : 0))

#define GRAPHIC_MODE (vga.gc.data[6]&1) /* else textmodus */

#define EGA_COLUMNS (vga.crtc.data[1]+1)
#define EGA_START_ADDRESS ((vga.crtc.data[0xd]|(vga.crtc.data[0xc]<<8))<<2)
#define EGA_LINE_LENGTH (vga.crtc.data[0x13]<<3)

#define VGA_COLUMNS (EGA_COLUMNS>>1)
#define VGA_START_ADDRESS (EGA_START_ADDRESS)
#define VGA_LINE_LENGTH (EGA_LINE_LENGTH<<2)

#define CHAR_WIDTH ((vga.sequencer.data[1]&1)?8:9)
//#define CHAR_HEIGHT ((vga.crtc.data[9]&0x1f)+1)

#define TEXT_COLUMNS (vga.crtc.data[1]+1)
#define TEXT_START_ADDRESS (EGA_START_ADDRESS)
#define TEXT_LINE_LENGTH (EGA_LINE_LENGTH>>2)

#define TEXT_COPY_9COLUMN(ch) ( (ch>=192)&&(ch<=223)&&(vga.attribute.data[0x10]&4))

//#define CURSOR_ON (!(vga.crtc.data[0xa]&0x20))
//#define CURSOR_STARTLINE (vga.crtc.data[0xa]&0x1f)
//#define CURSOR_ENDLINE (vga.crtc.data[0xb]&0x1f)
//#define CURSOR_POS (vga.crtc.data[0xf]|(vga.crtc.data[0xe]<<8))

#define FONT1 ( ((vga.sequencer.data[3]&3)|((vga.sequencer.data[3]&0x10)<<2))*0x2000)
#define FONT2 ((((vga.sequencer.data[3]&c)>>2)|((vga.sequencer.data[3]&0x20)<<3))*0x2000)

#define CRTC6845_COLUMNS (REG(0)+1)
#define CRTC6845_CHAR_COLUMNS (REG(1))
#define CRTC6845_LINES (REG(4)*CRTC6845_CHAR_HEIGHT+REG(5))
#define CRTC6845_CHAR_LINES REG(6)
#define CRTC6845_CHAR_HEIGHT ((REG(9)&0x1f)+1)
#define CRTC6845_VIDEO_START ((REG(0xc)<<8)|REG(0xd))
#define CRTC6845_INTERLACE_MODE (REG(8)&3)
#define CRTC6845_INTERLACE_SIGNAL 1
#define CRTC6845_INTERLACE 3
#define CRTC6845_CURSOR_MODE (REG(0xa)&0x60)
#define CRTC6845_CURSOR_OFF 0x20
#define CRTC6845_CURSOR_16FRAMES 0x40
#define CRTC6845_CURSOR_32FRAMES 0x60
#define CRTC6845_SKEW	(REG(8)&15)
#define CRTC6845_CURSOR_POS ((REG(0xe)<<8)|REG(0xf))
#define CRTC6845_CURSOR_TOP	(REG(0xa)&0x1f)
#define CRTC6845_CURSOR_BOTTOM REG(0xb)

INLINE UINT8 rotate_right(UINT8 val, UINT8 rot)
{
	return (val >> rot) | (val << (8 - rot));
}



static int ega_get_clock(void)
{
	int clck=0;
	switch(vga.miscellaneous_output&0xc) {
	case 0: clck=14000000;break;
	case 4: clck=16000000;break;
	/* case 8: external */
	/* case 0xc: reserved */
	}
	if (vga.sequencer.data[1]&8) clck/=2;
	return clck;
}

static int vga_get_clock(void)
{
	int clck=0;
	switch(vga.miscellaneous_output&0xc) {
	case 0: clck=25000000;break;
	case 4: clck=28000000;break;
	/* case 8: external */
	/* case 0xc: reserved */
	}
	if (vga.sequencer.data[1]&8) clck/=2;
	return clck;
}

static int ega_get_crtc_columns(void) /* in clocks! */
{
	int columns=vga.crtc.data[0]+2;
	if (!GRAPHIC_MODE) {
		columns*=CHAR_WIDTH;
	} else {
		columns*=8;
	}
	return columns;
}

static int ega_get_crtc_lines(void)
{
	int lines=vga.crtc.data[6]|((vga.crtc.data[7]&1));

	return lines;
}

static int vga_get_crtc_columns(void) /* in clocks! */
{
	int columns=vga.crtc.data[0]+5;

	if (!GRAPHIC_MODE)
		columns *= CHAR_WIDTH;
	else if (vga.gc.data[5]&0x40)
		columns *= 4;
	else
		columns *= 8;

	return columns;
}

static int vga_get_crtc_lines(void)
{
	int lines=(vga.crtc.data[6]
			   |((vga.crtc.data[7]&1)<<8)
			   |((vga.crtc.data[7]&0x20)<<(8-4)))+2;

	return lines;
}

static int vga_get_crtc_sync_lines(void)
{
	return 10;
}

static int vga_get_crtc_sync_columns(void)
{
	return 40;
}

INLINE WRITE8_HANDLER(vga_dirty_w)
{
	vga.memory[offset] = data;
}

INLINE WRITE8_HANDLER(vga_dirty_font_w)
{
	if (vga.memory[offset] != data)
	{
		vga.memory[offset] = data;
		if ((offset&3)==2)
			vga.fontdirty[offset>>7]=1;
	}
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
		vga_dirty_font_w(space, (offset<<2)+2, vga_latch_write(2,data));
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
	vga_dirty_font_w(space, ((offset&~3)<<2)|(offset&3),data);
}

/* 16 bit */
static READ16_HANDLER( vga_text16_r ) { return read16le_with_read8_handler(vga_text_r, space, offset, mem_mask); }
static READ16_HANDLER( vga_vga16_r ) { return read16le_with_read8_handler(vga_vga_r, space, offset, mem_mask); }
static WRITE16_HANDLER( vga_text16_w ) { write16le_with_write8_handler(vga_text_w, space, offset, data, mem_mask); }
static WRITE16_HANDLER( vga_vga16_w ) { write16le_with_write8_handler(vga_vga_w, space, offset, data, mem_mask); }

static READ16_HANDLER( vga_ega16_r ) { return read16le_with_read8_handler(vga_ega_r, space, offset, mem_mask); }
static WRITE16_HANDLER( vga_ega16_w ) { write16le_with_write8_handler(vga_ega_w, space, offset, data, mem_mask); }

/* 32 bit */
static READ32_HANDLER( vga_text32_r ) { return read32le_with_read8_handler(vga_text_r, space, offset, mem_mask); }
static READ32_HANDLER( vga_vga32_r ) { return read32le_with_read8_handler(vga_vga_r, space, offset, mem_mask); }
static WRITE32_HANDLER( vga_text32_w ) { write32le_with_write8_handler(vga_text_w, space, offset, data, mem_mask); }
static WRITE32_HANDLER( vga_vga32_w ) { write32le_with_write8_handler(vga_vga_w, space, offset, data, mem_mask); }

static READ32_HANDLER( vga_ega32_r ) { return read32le_with_read8_handler(vga_ega_r, space, offset, mem_mask); }
static WRITE32_HANDLER( vga_ega32_w ) { write32le_with_write8_handler(vga_ega_w, space, offset, data, mem_mask); }

/* 64 bit */
static READ64_HANDLER( vga_text64_r ) { return read64be_with_read8_handler(vga_text_r, space, offset, mem_mask); }
static READ64_HANDLER( vga_vga64_r ) { return read64be_with_read8_handler(vga_vga_r, space, offset, mem_mask); }
static WRITE64_HANDLER( vga_text64_w ) { write64be_with_write8_handler(vga_text_w, space, offset, data, mem_mask); }
static WRITE64_HANDLER( vga_vga64_w ) { write64be_with_write8_handler(vga_vga_w, space, offset, data, mem_mask); }

static READ64_HANDLER( vga_ega64_r ) { return read64be_with_read8_handler(vga_ega_r, space, offset, mem_mask); }
static WRITE64_HANDLER( vga_ega64_w ) { write64be_with_write8_handler(vga_ega_w, space, offset, data, mem_mask); }

static void vga_cpu_interface(running_machine *machine)
{
	address_space *space = cpu_get_address_space(machine->firstcpu, ADDRESS_SPACE_PROGRAM);
	static int sequencer, gc;
	read8_space_func read_handler;
	write8_space_func write_handler;
	read16_space_func read_handler16;
	write16_space_func write_handler16;
	read32_space_func read_handler32;
	write32_space_func write_handler32;
	read64_space_func read_handler64;
	write64_space_func write_handler64;
	UINT8 sel;
	int buswidth;

	if ((gc==vga.gc.data[6])&&(sequencer==vga.sequencer.data[4])) return;

	gc=vga.gc.data[6];
	sequencer=vga.sequencer.data[4];

	if (vga.sequencer.data[4]&8)
	{
		read_handler = vga_vga_r;
		write_handler = vga_vga_w;
		read_handler16 = vga_vga16_r;
		write_handler16 = vga_vga16_w;
		read_handler32 = vga_vga32_r;
		write_handler32 = vga_vga32_w;
		read_handler64 = vga_vga64_r;
		write_handler64 = vga_vga64_w;
	}
	else if (vga.sequencer.data[4] & 4)
	{
		read_handler = vga_ega_r;
		write_handler = vga_ega_w;
		read_handler16 = vga_ega16_r;
		write_handler16 = vga_ega16_w;
		read_handler32 = vga_ega32_r;
		write_handler32 = vga_ega32_w;
		read_handler64 = vga_ega64_r;
		write_handler64 = vga_ega64_w;
	}
	else
	{
		read_handler = vga_text_r;
		write_handler = vga_text_w;
		read_handler16 = vga_text16_r;
		write_handler16 = vga_text16_w;
		read_handler32 = vga_text32_r;
		write_handler32 = vga_text32_w;
		read_handler64 = vga_text64_r;
		write_handler64 = vga_text64_w;
	}

	/* remap the VGA memory */

	if (vga.vga_intf.map_vga_memory)
	{
		sel = vga.gc.data[6] & 0x0c;
		switch(sel)
		{
			case 0x00:
				if (vga.vga_intf.vga_memory_bank != NULL)
				{
					vga.vga_intf.map_vga_memory(machine, 0xA0000, 0xBFFFF, read_handler, write_handler);
					memory_set_bankptr(machine, vga.vga_intf.vga_memory_bank, vga.memory);
				}
				break;
			case 0x04:
				vga.vga_intf.map_vga_memory(machine, 0xA0000, 0xAFFFF, read_handler, write_handler);
				break;
			case 0x08:
				vga.vga_intf.map_vga_memory(machine, 0xB0000, 0xB7FFF, read_handler, write_handler);
				break;
			case 0x0C:
				vga.vga_intf.map_vga_memory(machine, 0xB8000, 0xBFFFF, read_handler, write_handler);
				break;
		}
	}
	else
	{
		buswidth = downcast<cpu_device *>(machine->firstcpu)->space_config(AS_PROGRAM)->m_databus_width;
		switch(buswidth)
		{
			case 8:
				sel = vga.gc.data[6] & 0x0c;
				if (sel)
				{
					if (sel == 0x04) memory_install_read8_handler(space,  0xa0000, 0xaffff, 0, 0, read_handler ); else memory_nop_read(space,  0xa0000, 0xaffff, 0, 0);
					if (sel == 0x08) memory_install_read8_handler(space,  0xb0000, 0xb7fff, 0, 0, read_handler ); else memory_nop_read(space,  0xb0000, 0xb7fff, 0, 0);
					if (sel == 0x0C) memory_install_read8_handler(space,  0xb8000, 0xbffff, 0, 0, read_handler ); else memory_nop_read(space,  0xb8000, 0xbffff, 0, 0);
					if (sel == 0x04) memory_install_write8_handler(space, 0xa0000, 0xaffff, 0, 0, write_handler); else memory_nop_write(space, 0xa0000, 0xaffff, 0, 0);
					if (sel == 0x08) memory_install_write8_handler(space, 0xb0000, 0xb7fff, 0, 0, write_handler); else memory_nop_write(space, 0xb0000, 0xb7fff, 0, 0);
					if (sel == 0x0C) memory_install_write8_handler(space, 0xb8000, 0xbffff, 0, 0, write_handler); else memory_nop_write(space, 0xb8000, 0xbffff, 0, 0);
				}
				else
				{
					memory_install_read_bank(space,  0xa0000, 0xbffff, 0, 0, "vgabank" );
					memory_install_write_bank(space, 0xa0000, 0xbffff, 0, 0, "vgabank" );
					memory_set_bankptr(machine,"vgabank", vga.memory);
				}
				break;

			case 16:
				sel = vga.gc.data[6] & 0x0c;
				if (sel)
				{
					if (sel == 0x04) memory_install_read16_handler(space,  0xa0000, 0xaffff, 0, 0, read_handler16 ); else memory_nop_read(space,  0xa0000, 0xaffff, 0, 0);
					if (sel == 0x08) memory_install_read16_handler(space,  0xb0000, 0xb7fff, 0, 0, read_handler16 ); else memory_nop_read(space,  0xb0000, 0xb7fff, 0, 0);
					if (sel == 0x0C) memory_install_read16_handler(space,  0xb8000, 0xbffff, 0, 0, read_handler16 ); else memory_nop_read(space,  0xb8000, 0xbffff, 0, 0);
					if (sel == 0x04) memory_install_write16_handler(space, 0xa0000, 0xaffff, 0, 0, write_handler16); else memory_nop_write(space, 0xa0000, 0xaffff, 0, 0);
					if (sel == 0x08) memory_install_write16_handler(space, 0xb0000, 0xb7fff, 0, 0, write_handler16); else memory_nop_write(space, 0xb0000, 0xb7fff, 0, 0);
					if (sel == 0x0C) memory_install_write16_handler(space, 0xb8000, 0xbffff, 0, 0, write_handler16); else memory_nop_write(space, 0xb8000, 0xbffff, 0, 0);
				}
				else
				{
					memory_install_read_bank(space,  0xa0000, 0xbffff, 0, 0, "vgabank" );
					memory_install_write_bank(space, 0xa0000, 0xbffff, 0, 0, "vgabank" );
					memory_set_bankptr(machine,"vgabank", vga.memory);
				}
				break;

			case 32:
				sel = vga.gc.data[6] & 0x0c;
				if (sel)
				{
					if (sel == 0x04) memory_install_read32_handler(space,  0xa0000, 0xaffff, 0, 0, read_handler32 ); else memory_nop_read(space,  0xa0000, 0xaffff, 0, 0);
					if (sel == 0x08) memory_install_read32_handler(space,  0xb0000, 0xb7fff, 0, 0, read_handler32 ); else memory_nop_read(space,  0xb0000, 0xb7fff, 0, 0);
					if (sel == 0x0C) memory_install_read32_handler(space,  0xb8000, 0xbffff, 0, 0, read_handler32 ); else memory_nop_read(space,  0xb8000, 0xbffff, 0, 0);
					if (sel == 0x04) memory_install_write32_handler(space, 0xa0000, 0xaffff, 0, 0, write_handler32); else memory_nop_write(space, 0xa0000, 0xaffff, 0, 0);
					if (sel == 0x08) memory_install_write32_handler(space, 0xb0000, 0xb7fff, 0, 0, write_handler32); else memory_nop_write(space, 0xb0000, 0xb7fff, 0, 0);
					if (sel == 0x0C) memory_install_write32_handler(space, 0xb8000, 0xbffff, 0, 0, write_handler32); else memory_nop_write(space, 0xb8000, 0xbffff, 0, 0);
				}
				else
				{
					memory_install_read_bank(space,  0xa0000, 0xbffff, 0, 0, "vgabank" );
					memory_install_write_bank(space, 0xa0000, 0xbffff, 0, 0, "vgabank" );
					memory_set_bankptr(machine,"vgabank", vga.memory);
				}
				break;

			case 64:
				sel = vga.gc.data[6] & 0x0c;
				if (sel)
				{
					if (sel == 0x04) memory_install_read64_handler(space,  0xa0000, 0xaffff, 0, 0, read_handler64 ); else memory_nop_read(space,  0xa0000, 0xaffff, 0, 0);
					if (sel == 0x08) memory_install_read64_handler(space,  0xb0000, 0xb7fff, 0, 0, read_handler64 ); else memory_nop_read(space,  0xb0000, 0xb7fff, 0, 0);
					if (sel == 0x0C) memory_install_read64_handler(space,  0xb8000, 0xbffff, 0, 0, read_handler64 ); else memory_nop_read(space,  0xb8000, 0xbffff, 0, 0);
					if (sel == 0x04) memory_install_write64_handler(space, 0xa0000, 0xaffff, 0, 0, write_handler64); else memory_nop_write(space, 0xa0000, 0xaffff, 0, 0);
					if (sel == 0x08) memory_install_write64_handler(space, 0xb0000, 0xb7fff, 0, 0, write_handler64); else memory_nop_write(space, 0xb0000, 0xb7fff, 0, 0);
					if (sel == 0x0C) memory_install_write64_handler(space, 0xb8000, 0xbffff, 0, 0, write_handler64); else memory_nop_write(space, 0xb8000, 0xbffff, 0, 0);
				}
				else
				{
					memory_install_read_bank(space,  0xa0000, 0xbffff, 0, 0, "vgabank" );
					memory_install_write_bank(space, 0xa0000, 0xbffff, 0, 0, "vgabank" );
					memory_set_bankptr(machine,"vgabank", vga.memory);
				}
				break;

			default:
				fatalerror("VGA:  Bus width %d not supported\n", buswidth);
				break;
		}
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
		vga.attribute.state = 0;
		data = 0;/*4; */
#if 0 /* slow */
		{
			int clock=vga.monitor.get_clock();
			int lines=vga.monitor.get_lines();
			int columns=vga.monitor.get_columns();
			int diff = (attotime_mul(attotime_sub(timer_get_time(space->machine), vga.monitor.start_time), clock).seconds)
				%(lines*columns);
			if (diff<columns*vga.monitor.get_sync_lines()) data|=8;
			diff=diff/lines;
			if (diff%columns<vga.monitor.get_sync_columns()) data|=1;
		}
#elif 1
		if (vga.monitor.retrace)
		{
			data |= 1;
			if (attotime_compare(attotime_sub(timer_get_time(space->machine), vga.monitor.start_time), ATTOTIME_IN_USEC(300)) > 0)
			{
				data |= 8;
				vga.monitor.retrace=0;
			}
		}
		else
		{
			if (attotime_compare(attotime_sub(timer_get_time(space->machine), vga.monitor.start_time), ATTOTIME_IN_MSEC(15)) > 0)
				vga.monitor.retrace=1;
			vga.monitor.start_time=timer_get_time(space->machine);
		}
#else
		// not working with ps2m30
		if (vga.monitor.retrace) data|=9;
		vga.monitor.retrace=0;
#endif
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
		data=0;
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
				logerror("vga_crtc_w(): CRTC[0x%02X %s] = 0x%02X\n",
					vga.crtc.index,
					(vga.crtc.index < vga.svga_intf.crtc_regcount) ? vga_crtc_reg_names[vga.crtc.index] : "?",
					data);
			}
			if(vga.crtc.index == 0x18 || vga.crtc.index == 0x07 || vga.crtc.index == 0x19 ) // Line compare
				vga.line_compare = (((vga.crtc.data[0x09] & 0x40) << 3) | ((vga.crtc.data[0x07] & 0x10) << 4) | vga.crtc.data[0x18])/2;
			if (vga.crtc.index < vga.svga_intf.crtc_regcount)
				vga.crtc.data[vga.crtc.index] = data;
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



READ8_HANDLER( ega_port_03c0_r)
{
	UINT8 data = 0xff;
	switch (offset) {
	case 2: data=0xff;/*!*/break;
	}
	return data;
}

READ8_HANDLER( vga_port_03c0_r )
{
	UINT8 data = 0xff;

	switch (offset)
	{
		case 1:
			if (vga.attribute.state==0)
			{
				data = vga.attribute.index;
			}
			else
			{
				if ((vga.attribute.index&0x1f)<sizeof(vga.attribute.data))
					data=vga.attribute.data[vga.attribute.index&0x1f];
			}
			break;

		case 2:
			data = 0;
			switch ((vga.miscellaneous_output>>2)&3)
			{
				case 3:
					if (vga.vga_intf.read_dipswitch && vga.vga_intf.read_dipswitch(space, 0) & 0x01)
						data |= 0x10;
					break;
				case 2:
					if (vga.vga_intf.read_dipswitch && vga.vga_intf.read_dipswitch(space, 0) & 0x02)
						data |= 0x10;
					break;
				case 1:
					if (vga.vga_intf.read_dipswitch && vga.vga_intf.read_dipswitch(space, 0) & 0x04)
						data |= 0x10;
					break;
				case 0:
					if (vga.vga_intf.read_dipswitch && vga.vga_intf.read_dipswitch(space, 0) & 0x08)
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
			if ((vga.attribute.index&0x1f)<sizeof(vga.attribute.data))
				vga.attribute.data[vga.attribute.index&0x1f]=data;
		}
		vga.attribute.state=!vga.attribute.state;
		break;
	case 2:
		vga.miscellaneous_output=data;
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
			vga_cpu_interface(space->machine);

			if (vga.sequencer.index == 0)
				vga.monitor.start_time = timer_get_time(space->machine);
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
			vga_cpu_interface(space->machine);
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



READ8_HANDLER( paradise_ega_03c0_r )
{
	UINT8 data = vga_port_03c0_r(space, offset);

	if (offset == 2)
	{
		if ( (vga.feature_control&3)==2 ) {
			data=(data&~0x60)|((vga.vga_intf.read_dipswitch(space, 0)&0xc0)>>1);
		} else if ((vga.feature_control&3)==1 ) {
			data=(data&~0x60)|((vga.vga_intf.read_dipswitch(space, 0)&0x30)<<1);
		}
	}
	return data;
}



void pc_vga_reset(running_machine *machine)
{
	/* clear out the VGA structure */
	memset(vga.pens, 0, sizeof(vga.pens));
	vga.miscellaneous_output = 1;
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
	memset(&vga.monitor, 0, sizeof(vga.monitor));
	memset(&vga.oak, 0, sizeof(vga.oak));
	vga.log = 0;

	vga.gc.data[6] = 0xc; /* prevent xtbios excepting vga ram as system ram */
/* amstrad pc1640 bios relies on the position of
   the video memory area,
   so I introduced the reset to switch to b8000 area */
	vga.sequencer.data[4] = 0;
	vga_cpu_interface(machine);

	vga.line_compare = 0x3ff;
	// set CRTC register to match the line compare value
	vga.crtc.data[0x18] = vga.line_compare & 0xff;
	if(vga.line_compare & 0x100)
		vga.crtc.data[0x07] |= 0x10;
	if(vga.line_compare & 0x200)
		vga.crtc.data[0x09] |= 0x40;
}

/* 16 bit */
READ16_HANDLER( vga_port16le_03b0_r ) { return read16le_with_read8_handler(vga_port_03b0_r, space, offset, mem_mask); }
READ16_HANDLER( vga_port16le_03c0_r ) { return read16le_with_read8_handler(vga_port_03c0_r, space, offset, mem_mask); }
READ16_HANDLER( vga_port16le_03d0_r ) { return read16le_with_read8_handler(vga_port_03d0_r, space, offset, mem_mask); }

WRITE16_HANDLER( vga_port16le_03b0_w ) { write16le_with_write8_handler(vga_port_03b0_w, space, offset, data, mem_mask); }
WRITE16_HANDLER( vga_port16le_03c0_w ) { write16le_with_write8_handler(vga_port_03c0_w, space, offset, data, mem_mask); }
WRITE16_HANDLER( vga_port16le_03d0_w ) { write16le_with_write8_handler(vga_port_03d0_w, space, offset, data, mem_mask); }

/* 32 bit */
static READ32_HANDLER( vga_port32le_03b0_r ) { return read32le_with_read8_handler(vga_port_03b0_r, space, offset, mem_mask); }
static READ32_HANDLER( vga_port32le_03c0_r ) { return read32le_with_read8_handler(vga_port_03c0_r, space, offset, mem_mask); }
static READ32_HANDLER( vga_port32le_03d0_r ) { return read32le_with_read8_handler(vga_port_03d0_r, space, offset, mem_mask); }

static WRITE32_HANDLER( vga_port32le_03b0_w ) { write32le_with_write8_handler(vga_port_03b0_w, space, offset, data, mem_mask); }
static WRITE32_HANDLER( vga_port32le_03c0_w ) { write32le_with_write8_handler(vga_port_03c0_w, space, offset, data, mem_mask); }
static WRITE32_HANDLER( vga_port32le_03d0_w ) { write32le_with_write8_handler(vga_port_03d0_w, space, offset, data, mem_mask); }

/* 64 bit */
static READ64_HANDLER( vga_port64be_03b0_r ) { return read64be_with_read8_handler(vga_port_03b0_r, space, offset, mem_mask); }
static READ64_HANDLER( vga_port64be_03c0_r ) { return read64be_with_read8_handler(vga_port_03c0_r, space, offset, mem_mask); }
static READ64_HANDLER( vga_port64be_03d0_r ) { return read64be_with_read8_handler(vga_port_03d0_r, space, offset, mem_mask); }

static WRITE64_HANDLER( vga_port64be_03b0_w ) { write64be_with_write8_handler(vga_port_03b0_w, space, offset, data, mem_mask); }
static WRITE64_HANDLER( vga_port64be_03c0_w ) { write64be_with_write8_handler(vga_port_03c0_w, space, offset, data, mem_mask); }
static WRITE64_HANDLER( vga_port64be_03d0_w ) { write64be_with_write8_handler(vga_port_03d0_w, space, offset, data, mem_mask); }

void pc_vga_init(running_machine *machine, const struct pc_vga_interface *vga_intf, const struct pc_svga_interface *svga_intf)
{
	int i, j, k, mask, buswidth;
	address_space *spacevga;

	memset(&vga, 0, sizeof(vga));

	for (k=0;k<4;k++)
	{
		for (mask=0x80, j=0; j<8; j++, mask>>=1)
		{
			for  (i=0; i<256; i++)
				color_bitplane_to_packed[k][j][i]=(i&mask)?(1<<k):0;
		}
	}

	/* copy over interfaces */
	vga.vga_intf = *vga_intf;
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
	vga.fontdirty		= auto_alloc_array(machine, UINT8, 0x800);
	vga.sequencer.data	= auto_alloc_array(machine, UINT8, vga.svga_intf.seq_regcount);
	vga.crtc.data		= auto_alloc_array(machine, UINT8, vga.svga_intf.crtc_regcount);
	vga.gc.data			= auto_alloc_array(machine, UINT8, vga.svga_intf.gc_regcount);
	memset(vga.memory, '\0', vga.svga_intf.vram_size);
	memset(vga.fontdirty, '\0', 0x800);
	memset(vga.sequencer.data, '\0', vga.svga_intf.seq_regcount);
	memset(vga.crtc.data, '\0', vga.svga_intf.crtc_regcount);
	memset(vga.gc.data, '\0', vga.svga_intf.gc_regcount);

	buswidth = downcast<cpu_device *>(machine->firstcpu)->space_config(AS_PROGRAM)->m_databus_width;
	spacevga = cpu_get_address_space(machine->firstcpu,vga.vga_intf.port_addressspace);
	switch(buswidth)
	{
		case 8:
			memory_install_read8_handler(spacevga, vga.vga_intf.port_offset + 0x3b0, vga.vga_intf.port_offset + 0x3bf, 0, 0, vga_port_03b0_r );
			memory_install_read8_handler(spacevga, vga.vga_intf.port_offset + 0x3c0, vga.vga_intf.port_offset + 0x3cf, 0, 0, vga_port_03c0_r );
			memory_install_read8_handler(spacevga, vga.vga_intf.port_offset + 0x3d0, vga.vga_intf.port_offset + 0x3df, 0, 0, vga_port_03d0_r );

			memory_install_write8_handler(spacevga, vga.vga_intf.port_offset + 0x3b0, vga.vga_intf.port_offset + 0x3bf, 0, 0, vga_port_03b0_w );
			memory_install_write8_handler(spacevga, vga.vga_intf.port_offset + 0x3c0, vga.vga_intf.port_offset + 0x3cf, 0, 0, vga_port_03c0_w );
			memory_install_write8_handler(spacevga, vga.vga_intf.port_offset + 0x3d0, vga.vga_intf.port_offset + 0x3df, 0, 0, vga_port_03d0_w );
			break;

		case 16:
			memory_install_read16_handler(spacevga, vga.vga_intf.port_offset + 0x3b0, vga.vga_intf.port_offset + 0x3bf, 0, 0, vga_port16le_03b0_r );
			memory_install_read16_handler(spacevga, vga.vga_intf.port_offset + 0x3c0, vga.vga_intf.port_offset + 0x3cf, 0, 0, vga_port16le_03c0_r );
			memory_install_read16_handler(spacevga, vga.vga_intf.port_offset + 0x3d0, vga.vga_intf.port_offset + 0x3df, 0, 0, vga_port16le_03d0_r );

			memory_install_write16_handler(spacevga, vga.vga_intf.port_offset + 0x3b0, vga.vga_intf.port_offset + 0x3bf, 0, 0, vga_port16le_03b0_w );
			memory_install_write16_handler(spacevga, vga.vga_intf.port_offset + 0x3c0, vga.vga_intf.port_offset + 0x3cf, 0, 0, vga_port16le_03c0_w );
			memory_install_write16_handler(spacevga, vga.vga_intf.port_offset + 0x3d0, vga.vga_intf.port_offset + 0x3df, 0, 0, vga_port16le_03d0_w );
			break;

		case 32:
			memory_install_read32_handler(spacevga, vga.vga_intf.port_offset + 0x3b0, vga.vga_intf.port_offset + 0x3bf, 0, 0, vga_port32le_03b0_r );
			memory_install_read32_handler(spacevga, vga.vga_intf.port_offset + 0x3c0, vga.vga_intf.port_offset + 0x3cf, 0, 0, vga_port32le_03c0_r );
			memory_install_read32_handler(spacevga, vga.vga_intf.port_offset + 0x3d0, vga.vga_intf.port_offset + 0x3df, 0, 0, vga_port32le_03d0_r );

			memory_install_write32_handler(spacevga, vga.vga_intf.port_offset + 0x3b0, vga.vga_intf.port_offset + 0x3bf, 0, 0, vga_port32le_03b0_w );
			memory_install_write32_handler(spacevga, vga.vga_intf.port_offset + 0x3c0, vga.vga_intf.port_offset + 0x3cf, 0, 0, vga_port32le_03c0_w );
			memory_install_write32_handler(spacevga, vga.vga_intf.port_offset + 0x3d0, vga.vga_intf.port_offset + 0x3df, 0, 0, vga_port32le_03d0_w );
			break;

		case 64:
			memory_install_read64_handler(spacevga, vga.vga_intf.port_offset + 0x3b0, vga.vga_intf.port_offset + 0x3bf, 0, 0, vga_port64be_03b0_r );
			memory_install_read64_handler(spacevga, vga.vga_intf.port_offset + 0x3c0, vga.vga_intf.port_offset + 0x3cf, 0, 0, vga_port64be_03c0_r );
			memory_install_read64_handler(spacevga, vga.vga_intf.port_offset + 0x3d0, vga.vga_intf.port_offset + 0x3df, 0, 0, vga_port64be_03d0_r );

			memory_install_write64_handler(spacevga, vga.vga_intf.port_offset + 0x3b0, vga.vga_intf.port_offset + 0x3bf, 0, 0, vga_port64be_03b0_w );
			memory_install_write64_handler(spacevga, vga.vga_intf.port_offset + 0x3c0, vga.vga_intf.port_offset + 0x3cf, 0, 0, vga_port64be_03c0_w );
			memory_install_write64_handler(spacevga, vga.vga_intf.port_offset + 0x3d0, vga.vga_intf.port_offset + 0x3df, 0, 0, vga_port64be_03d0_w );
			break;
	}

	pc_vga_reset(machine);
}

static TIMER_CALLBACK(vga_timer)
{
	vga.monitor.retrace=1;
}

static VIDEO_START( ega )
{
	vga.monitor.get_clock = ega_get_clock;
	vga.monitor.get_lines = ega_get_crtc_lines;
	vga.monitor.get_columns = ega_get_crtc_columns;
	vga.monitor.get_sync_lines = vga_get_crtc_sync_lines;
	vga.monitor.get_sync_columns = vga_get_crtc_sync_columns;
	timer_pulse(machine, ATTOTIME_IN_HZ(60), NULL, 0, vga_timer);
	pc_video_start(machine, pc_ega_choosevideomode, 0);
}

static VIDEO_RESET( ega )
{
}

static VIDEO_START( vga )
{
	vga.monitor.get_clock=vga_get_clock;
	vga.monitor.get_lines=vga_get_crtc_lines;
	vga.monitor.get_columns=vga_get_crtc_columns;
	vga.monitor.get_sync_lines=vga_get_crtc_sync_lines;
	vga.monitor.get_sync_columns=vga_get_crtc_sync_columns;
	timer_pulse(machine, ATTOTIME_IN_HZ(60), NULL, 0, vga_timer);
	pc_video_start(machine, pc_vga_choosevideomode, 0);
}

static VIDEO_RESET( vga )
{
	pc_vga_reset(machine);
}

static void vga_vh_text(bitmap_t *bitmap)
{
	UINT8 ch, attr;
	UINT8 bits;
	UINT8 *font;
	UINT16 *bitmapline;
	int width=CHAR_WIDTH, height=CRTC6845_CHAR_HEIGHT;
	int pos, line, column, mask, w, h, addr;
	pen_t pen;

	if (CRTC6845_CURSOR_MODE!=CRTC6845_CURSOR_OFF)
	{
		if (++vga.cursor.time>=0x10)
		{
			vga.cursor.visible^=1;
			vga.cursor.time=0;
		}
	}

	for (addr = TEXT_START_ADDRESS, line = -CRTC6845_SKEW; line < TEXT_LINES;
		 line += height, addr += TEXT_LINE_LENGTH)
	{
		for (pos = addr, column=0; column<TEXT_COLUMNS; column++, pos++)
		{
			ch   = vga.memory[(pos<<2) + 0];
			attr = vga.memory[(pos<<2) + 1];
			font = vga.memory+2+(ch<<(5+2))+FONT1;

			for (h = MAX(-line, 0); (h < height) && (line+h < MIN(TEXT_LINES, bitmap->height)); h++)
			{
				bitmapline = BITMAP_ADDR16(bitmap, line+h, 0);
				bits = font[h<<2];

				assert(bitmapline);

				for (mask=0x80, w=0; (w<width)&&(w<8); w++, mask>>=1)
				{
					if (bits&mask)
						pen = vga.pens[attr & 0x0f];
					else
						pen = vga.pens[attr >> 4];
					bitmapline[column*width+w] = pen;
				}
				if (w<width)
				{
					/* 9 column */
					if (TEXT_COPY_9COLUMN(ch)&&(bits&1))
						pen = vga.pens[attr & 0x0f];
					else
						pen = vga.pens[attr >> 4];
					bitmapline[column*width+w] = pen;
				}
			}
			if ((CRTC6845_CURSOR_MODE!=CRTC6845_CURSOR_OFF)
				&&vga.cursor.visible&&(pos==CRTC6845_CURSOR_POS))
			{
				for (h=CRTC6845_CURSOR_TOP;
					 (h<=CRTC6845_CURSOR_BOTTOM)&&(h<height)&&(line+h<TEXT_LINES);
					 h++)
				{
					plot_box(bitmap, column*width, line+h, width, 1, vga.pens[attr&0xf]);
				}
			}
		}
	}
}

static void vga_vh_ega(bitmap_t *bitmap)
{
	int pos, line, column, c, addr, i;
	int height = CRTC6845_CHAR_HEIGHT;
	UINT16 *bitmapline;
	UINT16 *newbitmapline;
	pen_t pen;

	for (addr=EGA_START_ADDRESS, pos=0, line=0; line<LINES;
		 line += height, addr=(addr+EGA_LINE_LENGTH)&0x3ffff)
	{
		bitmapline = BITMAP_ADDR16(bitmap, line, 0);

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
				bitmapline[c+i] = pen;

				data[0]>>=1;
				data[1]>>=1;
				data[2]>>=1;
				data[3]>>=1;
			}
		}

		for (i = 1; i < height; i++)
		{
			if (line + i >= LINES)
				break;

			newbitmapline = BITMAP_ADDR16(bitmap, line+i, 0);
			memcpy(newbitmapline, bitmapline, EGA_COLUMNS * 8 * sizeof(UINT16));
		}
	}
}

static void vga_vh_vga(bitmap_t *bitmap)
{
	int pos, line, column, c, addr, curr_addr;
	UINT16 *bitmapline;

	curr_addr = 0;
	if(vga.sequencer.data[4] & 0x08)
	{
		for (addr = VGA_START_ADDRESS, line=0; line<LINES; line++, addr+=VGA_LINE_LENGTH, curr_addr+=VGA_LINE_LENGTH)
		{
			if(line < (vga.line_compare & 0xff))
				curr_addr = addr;
			if(line == (vga.line_compare & 0xff))
				curr_addr = 0;
			bitmapline = BITMAP_ADDR16(bitmap, line, 0);
			addr %= vga.svga_intf.vram_size;
			for (pos=curr_addr, c=0, column=0; column<VGA_COLUMNS; column++, c+=8, pos+=0x20)
			{
				if(pos + 0x20 > vga.svga_intf.vram_size)
					return;
				bitmapline[c+0] = vga.memory[pos+0];
				bitmapline[c+1] = vga.memory[pos+1];
				bitmapline[c+2] = vga.memory[pos+2];
				bitmapline[c+3] = vga.memory[pos+3];
				bitmapline[c+4] = vga.memory[pos+0x10];
				bitmapline[c+5] = vga.memory[pos+0x11];
				bitmapline[c+6] = vga.memory[pos+0x12];
				bitmapline[c+7] = vga.memory[pos+0x13];
			}
		}
	}
	else
	{
		for (addr = VGA_START_ADDRESS, line=0; line<LINES; line++, addr+=VGA_LINE_LENGTH/4, curr_addr+=VGA_LINE_LENGTH/4)
		{
			if(line < (vga.line_compare & 0xff))
				curr_addr = addr;
			if(line == (vga.line_compare & 0xff))
				curr_addr = 0;
			bitmapline = BITMAP_ADDR16(bitmap, line, 0);
			addr %= vga.svga_intf.vram_size;
			for (pos=curr_addr, c=0, column=0; column<VGA_COLUMNS; column++, c+=8, pos+=0x08)
			{
				if(pos + 0x08 > vga.svga_intf.vram_size)
					return;
				bitmapline[c+0] = vga.memory[pos+0];
				bitmapline[c+1] = vga.memory[pos+1];
				bitmapline[c+2] = vga.memory[pos+2];
				bitmapline[c+3] = vga.memory[pos+3];
				bitmapline[c+4] = vga.memory[pos+4];
				bitmapline[c+5] = vga.memory[pos+5];
				bitmapline[c+6] = vga.memory[pos+6];
				bitmapline[c+7] = vga.memory[pos+7];
			}
		}
	}
}

static pc_video_update_proc pc_ega_choosevideomode(running_machine *machine, int *width, int *height)
{
	pc_video_update_proc proc = NULL;
	int i;

	//if (CRTC_ON)
	{
		for (i = 0; i < 16; i++)
			vga.pens[i]=machine->pens[i/*vga.attribute.data[i]&0x3f*/];

		if (!GRAPHIC_MODE)
		{
			proc = vga_vh_text;
			*height = TEXT_LINES;
			*width = TEXT_COLUMNS*CHAR_WIDTH;
		}
		else
		{
			proc = vga_vh_ega;
			*height = LINES;
			*width = EGA_COLUMNS*8;
		}
	}
	return proc;
}

static pc_video_update_proc pc_vga_choosevideomode(running_machine *machine, int *width, int *height)
{
	pc_video_update_proc proc = NULL;
	int i;

	//if (CRTC_ON)
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
				vga.pens[i] = machine->pens[(vga.attribute.data[i]&0x0f)
										 |((vga.attribute.data[0x14]&0xf)<<4)];
			}
		}
		else
		{
			for (i=0; i<16;i++)
			{
				vga.pens[i]=machine->pens[(vga.attribute.data[i]&0x3f)
										 |((vga.attribute.data[0x14]&0xc)<<4)];
			}
		}

		if (vga.svga_intf.choosevideomode)
			proc = vga.svga_intf.choosevideomode(vga.sequencer.data, vga.crtc.data, vga.gc.data, width, height);

		if (!proc)
		{
			if (!GRAPHIC_MODE)
			{
				proc = vga_vh_text;
				*height = TEXT_LINES;
				*width = TEXT_COLUMNS * CHAR_WIDTH;
			}
			else if (vga.gc.data[5]&0x40)
			{
				proc = vga_vh_vga;
				*height = LINES;
				*width = VGA_COLUMNS * 8;
			}
			else
			{
				proc = vga_vh_ega;
				*height = LINES;
				*width = EGA_COLUMNS * 8;
			}
		}
	}
	return proc;
}



void *pc_vga_memory(void)
{
	return vga.memory;
}



size_t pc_vga_memory_size(void)
{
	return vga.svga_intf.vram_size;
}

READ16_HANDLER( paradise_ega16le_03c0_r ) { return read16le_with_read8_handler(paradise_ega_03c0_r, space, offset, mem_mask); }
