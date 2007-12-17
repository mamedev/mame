/*
** File: tms9928a.c -- software implementation of the Texas Instruments
**                     TMS9918(A), TMS9928(A) and TMS9929(A), used by the Coleco, MSX and
**                     TI99/4(A).
**
** All undocumented features as described in the following file
** should be emulated.
**
** http://www.msxnet.org/tech/tms9918a.txt
**
** By Sean Young 1999 (sean@msxnet.org).
** Based on code by Mike Balfour. Features added:
** - read-ahead
** - single read/write address
** - AND mask for mode 2
** - multicolor mode
** - undocumented screen modes
** - illegal sprites (max 4 on one line)
** - vertical coordinate corrected -- was one to high (255 => 0, 0 => 1)
** - errors in interrupt emulation
** - back drop correctly emulated.
**
** 19 feb 2000, Sean:
** - now uses plot _pixel (..), so -ror works properly
** - fixed bug in tms.patternmask
**
** 3 nov 2000, Raphael Nabet:
** - fixed a nasty bug in draw_sprites. A transparent sprite caused
**   sprites at lower levels not to be displayed, which is wrong.
**
** 3 jan 2001, Sean Young:
** - A few minor cleanups
** - Changed TMS9928A_vram_[rw] and  TMS9928A_register_[rw] to READ8_HANDLER
**   and WRITE8_HANDLER.
** - Got rid of the color table, unused. Also got rid of the old colors,
**   which were commented out anyway.
**
**
** Todo:
** - The screen image is rendered in `one go'. Modifications during
**   screen build up are not shown.
** - Correctly emulate 4,8,16 kb VRAM if needed.
** - uses plot _pixel (...) in TMS_sprites (...), which is rended in
**   in a back buffer created with malloc (). Hmm..
** - Colours are incorrect. [fixed by R Nabet ?]
*/

#include "driver.h"
#include "tms9928a.h"


/*
    New palette (R. Nabet).

    First 3 columns from TI datasheet (in volts).
    Next 3 columns based on formula :
        Y = .299*R + .587*G + .114*B (NTSC)
    (the coefficients are likely to be slightly different with PAL, but who cares ?)
    I assumed the "zero" for R-Y and B-Y was 0.47V.
    Last 3 coeffs are the 8-bit values.

    Color            Y      R-Y     B-Y     R       G       B       R   G   B
    0 Transparent
    1 Black         0.00    0.47    0.47    0.00    0.00    0.00      0   0   0
    2 Medium green  0.53    0.07    0.20    0.13    0.79    0.26     33 200  66
    3 Light green   0.67    0.17    0.27    0.37    0.86    0.47     94 220 120
    4 Dark blue     0.40    0.40    1.00    0.33    0.33    0.93     84  85 237
    5 Light blue    0.53    0.43    0.93    0.49    0.46    0.99    125 118 252
    6 Dark red      0.47    0.83    0.30    0.83    0.32    0.30    212  82  77
    7 Cyan          0.73    0.00    0.70    0.26    0.92    0.96     66 235 245
    8 Medium red    0.53    0.93    0.27    0.99    0.33    0.33    252  85  84
    9 Light red     0.67    0.93    0.27    1.13(!) 0.47    0.47    255 121 120
    A Dark yellow   0.73    0.57    0.07    0.83    0.76    0.33    212 193  84
    B Light yellow  0.80    0.57    0.17    0.90    0.81    0.50    230 206 128
    C Dark green    0.47    0.13    0.23    0.13    0.69    0.23     33 176  59
    D Magenta       0.53    0.73    0.67    0.79    0.36    0.73    201  91 186
    E Gray          0.80    0.47    0.47    0.80    0.80    0.80    204 204 204
    F White         1.00    0.47    0.47    1.00    1.00    1.00    255 255 255
*/
static const rgb_t TMS9928A_palette[16] =
{
	RGB_BLACK,
	RGB_BLACK,
	MAKE_RGB(33, 200, 66),
	MAKE_RGB(94, 220, 120),
	MAKE_RGB(84, 85, 237),
	MAKE_RGB(125, 118, 252),
	MAKE_RGB(212, 82, 77),
	MAKE_RGB(66, 235, 245),
	MAKE_RGB(252, 85, 84),
	MAKE_RGB(255, 121, 120),
	MAKE_RGB(212, 193, 84),
	MAKE_RGB(230, 206, 128),
	MAKE_RGB(33, 176, 59),
	MAKE_RGB(201, 91, 186),
	MAKE_RGB(204, 204, 204),
	RGB_WHITE
};

/*
** Forward declarations of internal functions.
*/
static void draw_mode0 (running_machine *machine, mame_bitmap *bitmap, const rectangle *cliprect);
static void draw_mode1 (running_machine *machine, mame_bitmap *bitmap, const rectangle *cliprect);
static void draw_mode2 (running_machine *machine, mame_bitmap *bitmap, const rectangle *cliprect);
static void draw_mode12 (running_machine *machine, mame_bitmap *bitmap, const rectangle *cliprect);
static void draw_mode3 (running_machine *machine, mame_bitmap *bitmap, const rectangle *cliprect);
static void draw_mode23 (running_machine *machine, mame_bitmap *bitmap, const rectangle *cliprect);
static void draw_modebogus (running_machine *machine, mame_bitmap *bitmap, const rectangle *cliprect);
static void draw_sprites (running_machine *machine, mame_bitmap *bitmap, const rectangle *cliprect);
static void change_register (int reg, UINT8 data);

static void (*ModeHandlers[])(running_machine *machine, mame_bitmap *bitmap, const rectangle *cliprect) = {
        draw_mode0, draw_mode1, draw_mode2,  draw_mode12,
        draw_mode3, draw_modebogus, draw_mode23,
        draw_modebogus };

#define IMAGE_SIZE (256*192)        /* size of rendered image        */

#define LEFT_BORDER			15		/* a bit less for 9918a??? */
#define RIGHT_BORDER		15		/* 13 for 9929a */
#define TOP_BORDER_60HZ		27
#define BOTTOM_BORDER_60HZ	24
#define TOP_BORDER_50HZ		51		/* unknown (102 for top+bottom?) */
#define BOTTOM_BORDER_50HZ	51		/* unknown (102 for top+bottom?) */
#define TOP_BORDER			tms.top_border
#define BOTTOM_BORDER		tms.bottom_border

#define TMS_SPRITES_ENABLED ((tms.Regs[1] & 0x50) == 0x40)
#define TMS_50HZ ((tms.model == TMS9929) || (tms.model == TMS9929A))
#define TMS_REVA ((tms.model == TMS99x8A) || (tms.model == TMS9929A))
#define TMS_MODE ( (TMS_REVA ? (tms.Regs[0] & 2) : 0) | \
	((tms.Regs[1] & 0x10)>>4) | ((tms.Regs[1] & 8)>>1))

typedef struct {
    /* TMS9928A internal settings */
    UINT8 ReadAhead,Regs[8],StatusReg,FirstByte,latch,INT;
    INT32 Addr;
    int colour,pattern,nametbl,spriteattribute,spritepattern;
    int colourmask,patternmask;
    void (*INTCallback)(int);
    /* memory */
    UINT8 *vMem, *dBackMem;
    mame_bitmap *tmpbmp;
    int vramsize, model;
    /* emulation settings */
    int LimitSprites; /* max 4 sprites on a row, like original TMS9918A */
    int top_border, bottom_border;
    rectangle visarea;
} TMS9928A;

static TMS9928A tms;

/*
** initialize the palette
*/
static PALETTE_INIT( tms9928a )
{
	palette_set_colors(machine, 0, TMS9928A_palette, TMS9928A_PALETTE_SIZE);
}


/*
** The init, reset and shutdown functions
*/
void TMS9928A_reset () {
    int  i;

    for (i=0;i<8;i++) tms.Regs[i] = 0;
    tms.StatusReg = 0;
    tms.nametbl = tms.pattern = tms.colour = 0;
    tms.spritepattern = tms.spriteattribute = 0;
    tms.colourmask = tms.patternmask = 0;
    tms.Addr = tms.ReadAhead = tms.INT = 0;
    tms.FirstByte = 0;
	tms.latch = 0;
}

static void TMS9928A_start (running_machine *machine, const TMS9928a_interface *intf)
{
    assert_always(((intf->vram == 0x1000) || (intf->vram == 0x2000) || (intf->vram == 0x4000)), "4, 8 or 16 kB vram please");

    tms.model = intf->model;

	tms.top_border = TMS_50HZ ? TOP_BORDER_50HZ : TOP_BORDER_60HZ;
	tms.bottom_border = TMS_50HZ ? BOTTOM_BORDER_50HZ : BOTTOM_BORDER_60HZ;

	tms.INTCallback = intf->int_callback;

	/* determine the visible area */
	tms.visarea.min_x = LEFT_BORDER - MIN(intf->borderx, LEFT_BORDER);
	tms.visarea.max_x = LEFT_BORDER + 32*8 - 1 + MIN(intf->borderx, RIGHT_BORDER);
	tms.visarea.min_y = tms.top_border - MIN(intf->bordery, tms.top_border);
	tms.visarea.max_y = tms.top_border + 24*8 - 1 + MIN(intf->bordery, tms.bottom_border);

	/* configure the screen if we weren't overridden */
	if (machine->screen[0].width == LEFT_BORDER+32*8+RIGHT_BORDER && machine->screen[0].height == TOP_BORDER_60HZ+24*8+BOTTOM_BORDER_60HZ)
		video_screen_configure(0, LEFT_BORDER + 32*8 + RIGHT_BORDER, tms.top_border + 24*8 + tms.bottom_border, &tms.visarea, machine->screen[0].refresh);

    /* Video RAM */
    tms.vramsize = intf->vram;
    tms.vMem = (UINT8*) auto_malloc (intf->vram);
    memset (tms.vMem, 0, intf->vram);

    /* Sprite back buffer */
    tms.dBackMem = (UINT8*)auto_malloc (IMAGE_SIZE);

    /* back bitmap */
    tms.tmpbmp = auto_bitmap_alloc (256, 192, machine->screen[0].format);

    TMS9928A_reset ();
    tms.LimitSprites = 1;

	state_save_register_item("tms9928a", 0, tms.Regs[0]);
	state_save_register_item("tms9928a", 0, tms.Regs[1]);
	state_save_register_item("tms9928a", 0, tms.Regs[2]);
	state_save_register_item("tms9928a", 0, tms.Regs[3]);
	state_save_register_item("tms9928a", 0, tms.Regs[4]);
	state_save_register_item("tms9928a", 0, tms.Regs[5]);
	state_save_register_item("tms9928a", 0, tms.Regs[6]);
	state_save_register_item("tms9928a", 0, tms.Regs[7]);
	state_save_register_item("tms9928a", 0, tms.StatusReg);
	state_save_register_item("tms9928a", 0, tms.ReadAhead);
	state_save_register_item("tms9928a", 0, tms.FirstByte);
	state_save_register_item("tms9928a", 0, tms.latch);
	state_save_register_item("tms9928a", 0, tms.Addr);
	state_save_register_item("tms9928a", 0, tms.INT);
	state_save_register_item_pointer("tms9928a", 0, tms.vMem, intf->vram);
}

const rectangle *TMS9928A_get_visarea (void)
{
	return &tms.visarea;
}


void TMS9928A_post_load (void) {
	int i;

	/* all registers need to be re-written, so tables are recalculated */
	for (i=0;i<8;i++)
		change_register (i, tms.Regs[i]);

	/* make sure the interrupt request is set properly */
	if (tms.INTCallback) tms.INTCallback (tms.INT);
}


/*
** The I/O functions.
*/
READ8_HANDLER (TMS9928A_vram_r) {
    UINT8 b;
    b = tms.ReadAhead;
    tms.ReadAhead = tms.vMem[tms.Addr];
    tms.Addr = (tms.Addr + 1) & (tms.vramsize - 1);
    tms.latch = 0;
    return b;
}

WRITE8_HANDLER (TMS9928A_vram_w) {

    tms.vMem[tms.Addr] = data;
    tms.Addr = (tms.Addr + 1) & (tms.vramsize - 1);
    tms.ReadAhead = data;
    tms.latch = 0;
}

READ8_HANDLER (TMS9928A_register_r) {
    UINT8 b;
    b = tms.StatusReg;
    tms.StatusReg = 0x1f;
    if (tms.INT) {
        tms.INT = 0;
        if (tms.INTCallback) tms.INTCallback (tms.INT);
    }
    tms.latch = 0;
    return b;
}

WRITE8_HANDLER (TMS9928A_register_w) {
	int reg;

    if (tms.latch) {
        if (data & 0x80) {
            /* register write */
			reg = data & 7;
			/*if (tms.FirstByte != tms.Regs[reg])*/ /* Removed to fix ColecoVision MESS Driver*/
	            change_register (reg, tms.FirstByte);
        } else {
            /* set read/write address */
            tms.Addr = ((UINT16)data << 8 | tms.FirstByte) & (tms.vramsize - 1);
            if ( !(data & 0x40) ) {
				/* read ahead */
				TMS9928A_vram_r	(0);
            }
        }
        tms.latch = 0;
    } else {
        tms.FirstByte = data;
		tms.latch = 1;
    }
}

static void change_register (int reg, UINT8 val) {
    static const UINT8 Mask[8] =
        { 0x03, 0xfb, 0x0f, 0xff, 0x07, 0x7f, 0x07, 0xff };
    static const char *modes[] = {
        "Mode 0 (GRAPHIC 1)", "Mode 1 (TEXT 1)", "Mode 2 (GRAPHIC 2)",
        "Mode 1+2 (TEXT 1 variation)", "Mode 3 (MULTICOLOR)",
        "Mode 1+3 (BOGUS)", "Mode 2+3 (MULTICOLOR variation)",
        "Mode 1+2+3 (BOGUS)" };
    UINT8 b;

    val &= Mask[reg];
    tms.Regs[reg] = val;

    logerror("TMS9928A: Reg %d = %02xh\n", reg, (int)val);
    switch (reg) {
    case 0:
		/* re-calculate masks and pattern generator & colour */
		if (val & 2) {
			tms.colour = ((tms.Regs[3] & 0x80) * 64) & (tms.vramsize - 1);
			tms.colourmask = (tms.Regs[3] & 0x7f) * 8 | 7;
			tms.pattern = ((tms.Regs[4] & 4) * 2048) & (tms.vramsize - 1);
			tms.patternmask = (tms.Regs[4] & 3) * 256 |
				(tms.colourmask & 255);
		} else {
			tms.colour = (tms.Regs[3] * 64) & (tms.vramsize - 1);
			tms.pattern = (tms.Regs[4] * 2048) & (tms.vramsize - 1);
		}
        logerror("TMS9928A: %s\n", modes[TMS_MODE]);
        break;
    case 1:
        /* check for changes in the INT line */
        b = (val & 0x20) && (tms.StatusReg & 0x80) ;
        if (b != tms.INT) {
            tms.INT = b;
            if (tms.INTCallback) tms.INTCallback (tms.INT);
        }
        logerror("TMS9928A: %s\n", modes[TMS_MODE]);
        break;
    case 2:
        tms.nametbl = (val * 1024) & (tms.vramsize - 1);
        break;
    case 3:
        if (tms.Regs[0] & 2) {
            tms.colour = ((val & 0x80) * 64) & (tms.vramsize - 1);
            tms.colourmask = (val & 0x7f) * 8 | 7;
         } else {
            tms.colour = (val * 64) & (tms.vramsize - 1);
        }
		tms.patternmask = (tms.Regs[4] & 3) * 256 | (tms.colourmask & 255);
        break;
    case 4:
        if (tms.Regs[0] & 2) {
            tms.pattern = ((val & 4) * 2048) & (tms.vramsize - 1);
            tms.patternmask = (val & 3) * 256 | 255;
        } else {
            tms.pattern = (val * 2048) & (tms.vramsize - 1);
        }
        break;
    case 5:
        tms.spriteattribute = (val * 128) & (tms.vramsize - 1);
        break;
    case 6:
        tms.spritepattern = (val * 2048) & (tms.vramsize - 1);
        break;
    case 7:
        /* The backdrop is updated at TMS9928A_refresh() */
        break;
    }
}

/*
** Interface functions
*/

/*void TMS9928A_int_callback (void (*callback)(int)) {
    tms.INTCallback = callback;
}*/

void TMS9928A_set_spriteslimit (int limit) {
    tms.LimitSprites = limit;
}

/*
** Updates the screen (the dMem memory area).
*/
VIDEO_UPDATE( tms9928a )
{
    INT32 BackColour = tms.Regs[7] & 15; if (!BackColour) BackColour=1;
    palette_set_color(machine, 0, TMS9928A_palette[BackColour]);

	if (! (tms.Regs[1] & 0x40))
		fillbitmap(bitmap, machine->pens[BackColour], cliprect);
	else
	{
		(*ModeHandlers[TMS_MODE])(machine, tms.tmpbmp, cliprect);

		copybitmap(bitmap, tms.tmpbmp, 0, 0, LEFT_BORDER, TOP_BORDER, cliprect, TRANSPARENCY_NONE, 0);
		{
			rectangle rt;

			/* set borders */
			rt.min_x = 0; rt.max_x = LEFT_BORDER+256+RIGHT_BORDER-1;
			rt.min_y = 0; rt.max_y = TOP_BORDER-1;
			fillbitmap (bitmap, BackColour, &rt);
			rt.min_y = TOP_BORDER+192; rt.max_y = TOP_BORDER+192+BOTTOM_BORDER-1;
			fillbitmap (bitmap, BackColour, &rt);

			rt.min_y = TOP_BORDER; rt.max_y = TOP_BORDER+192-1;
			rt.min_x = 0; rt.max_x = LEFT_BORDER-1;
			fillbitmap (bitmap, BackColour, &rt);
			rt.min_x = LEFT_BORDER+256; rt.max_x = LEFT_BORDER+256+RIGHT_BORDER-1;
			fillbitmap (bitmap, BackColour, &rt);
	    }
		if (TMS_SPRITES_ENABLED)
			draw_sprites(machine, bitmap, cliprect);
	}

	return 0;
}

int TMS9928A_interrupt () {
    int b;

    /* when skipping frames, calculate sprite collision */
    if (video_skip_this_frame() ) {
		if (TMS_SPRITES_ENABLED) {
			draw_sprites (NULL, NULL, NULL);
		}
    }

    tms.StatusReg |= 0x80;
    b = (tms.Regs[1] & 0x20) != 0;
    if (b != tms.INT) {
        tms.INT = b;
        if (tms.INTCallback) tms.INTCallback (tms.INT);
    }

    return b;
}

static void draw_mode1 (running_machine *machine, mame_bitmap *bitmap, const rectangle *cliprect) {
    int pattern,x,y,yy,xx,name,charcode;
    UINT8 fg,bg,*patternptr;
	rectangle rt;

    fg = machine->pens[tms.Regs[7] / 16];
    bg = machine->pens[tms.Regs[7] & 15];

	/* colours at sides must be reset */
	rt.min_y = 0; rt.max_y = 191;
	rt.min_x = 0; rt.max_x = 7;
	fillbitmap (bitmap, bg, &rt);
	rt.min_y = 0; rt.max_y = 191;
	rt.min_x = 248; rt.max_x = 255;
	fillbitmap (bitmap, bg, &rt);

    name = 0;
    for (y=0;y<24;y++) {
        for (x=0;x<40;x++) {
            charcode = tms.vMem[tms.nametbl+name];
            name++;
            patternptr = tms.vMem + tms.pattern + (charcode*8);
            for (yy=0;yy<8;yy++) {
                pattern = *patternptr++;
                for (xx=0;xx<6;xx++) {
					*BITMAP_ADDR16(bitmap, y*8+yy, 8+x*6+xx) = (pattern & 0x80) ? fg : bg;
                    pattern *= 2;
                }
            }
        }
    }
}

static void draw_mode12 (running_machine *machine, mame_bitmap *bitmap, const rectangle *cliprect) {
    int pattern,x,y,yy,xx,name,charcode;
    UINT8 fg,bg,*patternptr;
	rectangle rt;

    fg = machine->pens[tms.Regs[7] / 16];
    bg = machine->pens[tms.Regs[7] & 15];

	/* colours at sides must be reset */
	rt.min_y = 0; rt.max_y = 191;
	rt.min_x = 0; rt.max_x = 7;
	fillbitmap (bitmap, bg, &rt);
	rt.min_y = 0; rt.max_y = 191;
	rt.min_x = 248; rt.max_x = 255;
	fillbitmap (bitmap, bg, &rt);

    name = 0;
    for (y=0;y<24;y++) {
        for (x=0;x<40;x++) {
            charcode = (tms.vMem[tms.nametbl+name]+(y/8)*256)&tms.patternmask;
            name++;
            patternptr = tms.vMem + tms.pattern + (charcode*8);
            for (yy=0;yy<8;yy++) {
                pattern = *patternptr++;
                for (xx=0;xx<6;xx++) {
					*BITMAP_ADDR16(bitmap, y*8+yy, 8+x*6+xx) = (pattern & 0x80) ? fg : bg;
                    pattern *= 2;
                }
            }
        }
    }
}

static void draw_mode0 (running_machine *machine, mame_bitmap *bitmap, const rectangle *cliprect) {
    int pattern,x,y,yy,xx,name,charcode,colour;
    UINT8 fg,bg,*patternptr;

    name = 0;
    for (y=0;y<24;y++) {
        for (x=0;x<32;x++) {
            charcode = tms.vMem[tms.nametbl+name];
            name++;
            patternptr = tms.vMem + tms.pattern + charcode*8;
            colour = tms.vMem[tms.colour+charcode/8];
            fg = machine->pens[colour / 16];
            bg = machine->pens[colour & 15];
            for (yy=0;yy<8;yy++) {
                pattern=*patternptr++;
                for (xx=0;xx<8;xx++) {
					*BITMAP_ADDR16(bitmap, y*8+yy, x*8+xx) = (pattern & 0x80) ? fg : bg;
                    pattern *= 2;
                }
            }
        }
    }
}

static void draw_mode2 (running_machine *machine, mame_bitmap *bitmap, const rectangle *cliprect) {
    int colour,name,x,y,yy,pattern,xx,charcode;
    UINT8 fg,bg;
    UINT8 *colourptr,*patternptr;

    name = 0;
    for (y=0;y<24;y++) {
        for (x=0;x<32;x++) {
            charcode = tms.vMem[tms.nametbl+name]+(y/8)*256;
            name++;
            colour = (charcode&tms.colourmask);
            pattern = (charcode&tms.patternmask);
            patternptr = tms.vMem+tms.pattern+colour*8;
            colourptr = tms.vMem+tms.colour+pattern*8;
            for (yy=0;yy<8;yy++) {
                pattern = *patternptr++;
                colour = *colourptr++;
                fg = machine->pens[colour / 16];
                bg = machine->pens[colour & 15];
                for (xx=0;xx<8;xx++) {
					*BITMAP_ADDR16(bitmap, y*8+yy, x*8+xx) = (pattern & 0x80) ? fg : bg;
                    pattern *= 2;
                }
            }
        }
    }
}

static void draw_mode3 (running_machine *machine, mame_bitmap *bitmap, const rectangle *cliprect) {
    int x,y,yy,yyy,name,charcode;
    UINT8 fg,bg,*patternptr;

    name = 0;
    for (y=0;y<24;y++) {
        for (x=0;x<32;x++) {
            charcode = tms.vMem[tms.nametbl+name];
            name++;
            patternptr = tms.vMem+tms.pattern+charcode*8+(y&3)*2;
            for (yy=0;yy<2;yy++) {
                fg = machine->pens[(*patternptr / 16)];
                bg = machine->pens[((*patternptr++) & 15)];
                for (yyy=0;yyy<4;yyy++) {
			*BITMAP_ADDR16(bitmap, y*8+yy*4+yyy, x*8+0) = fg;
			*BITMAP_ADDR16(bitmap, y*8+yy*4+yyy, x*8+1) = fg;
			*BITMAP_ADDR16(bitmap, y*8+yy*4+yyy, x*8+2) = fg;
			*BITMAP_ADDR16(bitmap, y*8+yy*4+yyy, x*8+3) = fg;
			*BITMAP_ADDR16(bitmap, y*8+yy*4+yyy, x*8+4) = fg;
			*BITMAP_ADDR16(bitmap, y*8+yy*4+yyy, x*8+5) = fg;
			*BITMAP_ADDR16(bitmap, y*8+yy*4+yyy, x*8+6) = fg;
			*BITMAP_ADDR16(bitmap, y*8+yy*4+yyy, x*8+7) = fg;
                }
            }
        }
    }
}

static void draw_mode23 (running_machine *machine, mame_bitmap *bitmap, const rectangle *cliprect) {
    int x,y,yy,yyy,name,charcode;
    UINT8 fg,bg,*patternptr;

    name = 0;
    for (y=0;y<24;y++) {
        for (x=0;x<32;x++) {
            charcode = tms.vMem[tms.nametbl+name];
            name++;
            patternptr = tms.vMem + tms.pattern +
                ((charcode+(y&3)*2+(y/8)*256)&tms.patternmask)*8;
            for (yy=0;yy<2;yy++) {
                fg = machine->pens[(*patternptr / 16)];
                bg = machine->pens[((*patternptr++) & 15)];
                for (yyy=0;yyy<4;yyy++) {
			*BITMAP_ADDR16(bitmap, y*8+yy*4+yyy, x*8+0) = fg;
			*BITMAP_ADDR16(bitmap, y*8+yy*4+yyy, x*8+1) = fg;
			*BITMAP_ADDR16(bitmap, y*8+yy*4+yyy, x*8+2) = fg;
			*BITMAP_ADDR16(bitmap, y*8+yy*4+yyy, x*8+3) = fg;
			*BITMAP_ADDR16(bitmap, y*8+yy*4+yyy, x*8+4) = fg;
			*BITMAP_ADDR16(bitmap, y*8+yy*4+yyy, x*8+5) = fg;
			*BITMAP_ADDR16(bitmap, y*8+yy*4+yyy, x*8+6) = fg;
			*BITMAP_ADDR16(bitmap, y*8+yy*4+yyy, x*8+7) = fg;
                }
            }
        }
    }
}

static void draw_modebogus (running_machine *machine, mame_bitmap *bitmap, const rectangle *cliprect) {
    UINT8 fg,bg;
    int x,y,n,xx;

    fg = machine->pens[tms.Regs[7] / 16];
    bg = machine->pens[tms.Regs[7] & 15];

    for (y=0;y<192;y++) {
        xx=0;
        n=8; while (n--) *BITMAP_ADDR16(bitmap, y, xx++) = bg;
        for (x=0;x<40;x++) {
            n=4; while (n--) *BITMAP_ADDR16(bitmap, y, xx++) = fg;
            n=2; while (n--) *BITMAP_ADDR16(bitmap, y, xx++) = bg;
        }
        n=8; while (n--) *BITMAP_ADDR16(bitmap, y, xx++) = bg;
    }
}

/*
** This function renders the sprites. Sprite collision is calculated in
** in a back buffer (tms.dBackMem), because sprite collision detection
** is rather complicated (transparent sprites also cause the sprite
** collision bit to be set, and ``illegal'' sprites do not count
** (they're not displayed)).
**
** This code should be optimized. One day.
*/
static void draw_sprites (running_machine *machine, mame_bitmap *bitmap, const rectangle *cliprect) {
    UINT8 *attributeptr,*patternptr,c;
    int p,x,y,size,i,j,large,yy,xx,limit[192],
        illegalsprite,illegalspriteline;
    UINT16 line,line2;

    attributeptr = tms.vMem + tms.spriteattribute;
    size = (tms.Regs[1] & 2) ? 16 : 8;
    large = (int)(tms.Regs[1] & 1);

    for (x=0;x<192;x++) limit[x] = 4;
    tms.StatusReg = 0x80;
    illegalspriteline = 255;
    illegalsprite = 0;

    memset (tms.dBackMem, 0, IMAGE_SIZE);
    for (p=0;p<32;p++) {
        y = *attributeptr++;
        if (y == 208) break;
        if (y > 208) {
            y=-(~y&255);
        } else {
            y++;
        }
        x = *attributeptr++;
        patternptr = tms.vMem + tms.spritepattern +
            ((size == 16) ? *attributeptr & 0xfc : *attributeptr) * 8;
        attributeptr++;
        c = (*attributeptr & 0x0f);
        if (*attributeptr & 0x80) x -= 32;
        attributeptr++;

        if (!large) {
            /* draw sprite (not enlarged) */
            for (yy=y;yy<(y+size);yy++) {
                if ( (yy < 0) || (yy > 191) ) continue;
                if (limit[yy] == 0) {
                    /* illegal sprite line */
                    if (yy < illegalspriteline) {
                        illegalspriteline = yy;
                        illegalsprite = p;
                    } else if (illegalspriteline == yy) {
                        if (illegalsprite > p) {
                            illegalsprite = p;
                        }
                    }
                    if (tms.LimitSprites) continue;
                } else limit[yy]--;
                line = 256*patternptr[yy-y] + patternptr[yy-y+16];
                for (xx=x;xx<(x+size);xx++) {
                    if (line & 0x8000) {
                        if ((xx >= 0) && (xx < 256)) {
                            if (tms.dBackMem[yy*256+xx]) {
                                tms.StatusReg |= 0x20;
                            } else {
                                tms.dBackMem[yy*256+xx] = 0x01;
                            }
                            if (c && ! (tms.dBackMem[yy*256+xx] & 0x02))
                            {
                            	tms.dBackMem[yy*256+xx] |= 0x02;
                            	if (bitmap)
                            		*BITMAP_ADDR16(bitmap, TOP_BORDER+yy, LEFT_BORDER+xx) = machine->pens[c];
							}
                        }
                    }
                    line *= 2;
                }
            }
        } else {
            /* draw enlarged sprite */
            for (i=0;i<size;i++) {
                yy=y+i*2;
                line2 = 256*patternptr[i] + patternptr[i+16];
                for (j=0;j<2;j++) {
                    if ( (yy >= 0) && (yy <= 191) ) {
                        if (limit[yy] == 0) {
                            /* illegal sprite line */
                            if (yy < illegalspriteline) {
                                illegalspriteline = yy;
                                 illegalsprite = p;
                            } else if (illegalspriteline == yy) {
                                if (illegalsprite > p) {
                                    illegalsprite = p;
                                }
                            }
                            if (tms.LimitSprites) continue;
                        } else limit[yy]--;
                        line = line2;
                        for (xx=x;xx<(x+size*2);xx+=2) {
                            if (line & 0x8000) {
                                if ((xx >=0) && (xx < 256)) {
                                    if (tms.dBackMem[yy*256+xx]) {
                                        tms.StatusReg |= 0x20;
                                    } else {
                                        tms.dBackMem[yy*256+xx] = 0x01;
                                    }
		                            if (c && ! (tms.dBackMem[yy*256+xx] & 0x02))
        		                    {
                		            	tms.dBackMem[yy*256+xx] |= 0x02;
                                        if (bitmap)
		                            		*BITMAP_ADDR16(bitmap, TOP_BORDER+yy, LEFT_BORDER+xx) = machine->pens[c];
                		            }
                                }
                                if (((xx+1) >=0) && ((xx+1) < 256)) {
                                    if (tms.dBackMem[yy*256+xx+1]) {
                                        tms.StatusReg |= 0x20;
                                    } else {
                                        tms.dBackMem[yy*256+xx+1] = 0x01;
                                    }
		                            if (c && ! (tms.dBackMem[yy*256+xx+1] & 0x02))
        		                    {
                		            	tms.dBackMem[yy*256+xx+1] |= 0x02;
                                        if (bitmap)
		                            		*BITMAP_ADDR16(bitmap, TOP_BORDER+yy, LEFT_BORDER+xx+1) = machine->pens[c];
									}
                                }
                            }
                            line *= 2;
                        }
                    }
                    yy++;
                }
            }
        }
    }
    if (illegalspriteline == 255) {
        tms.StatusReg |= (p > 31) ? 31 : p;
    } else {
        tms.StatusReg |= 0x40 + illegalsprite;
    }
}

static TMS9928a_interface sIntf;


void TMS9928A_configure (const TMS9928a_interface *intf)
{
	sIntf = *intf;
}


VIDEO_START( tms9928a )
{
	assert(sIntf.model != TMS_INVALID_MODEL);
	TMS9928A_start(machine, &sIntf);
}


MACHINE_DRIVER_START( tms9928a )

	/* video hardware */
	MDRV_VIDEO_ATTRIBUTES(VIDEO_UPDATE_BEFORE_VBLANK | VIDEO_TYPE_RASTER)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(LEFT_BORDER+32*8+RIGHT_BORDER, TOP_BORDER_60HZ+24*8+BOTTOM_BORDER_60HZ)
	MDRV_SCREEN_VISIBLE_AREA(LEFT_BORDER-12, LEFT_BORDER+32*8+12-1, TOP_BORDER_60HZ-9, TOP_BORDER_60HZ+24*8+9-1)

	MDRV_PALETTE_LENGTH(TMS9928A_PALETTE_SIZE)
	MDRV_PALETTE_INIT(tms9928a)

	MDRV_VIDEO_START(tms9928a)
	MDRV_VIDEO_UPDATE(tms9928a)
MACHINE_DRIVER_END

