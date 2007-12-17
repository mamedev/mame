/***************************************************************************

  video.c

  Functions to emulate the video hardware of the machine.

---

The Video system used in Space Tactics is unusual.
Here are my notes on how the video system works:

There are 4, 4K pages of Video RAM. (B,D,E & F)

The first 1K of each VRAM page contains the following:
0 0 V V V V V H H H H H   offset value for each 8x8 bitmap
     (v-tile)  (h-tile)

The offset values are used to generate an access into the
last 2K of the VRAM page:
1 D D D D D D D D V V V   here we find 8x8 character data
     (offset)    (line)

In addition, in Page B, the upper nibble of the offset is
also used to select the color palette for the tile.

Page B, D, E, and F all work similarly, except that pages
D, E, and F can be offset from Page B by a given
number of scanlines, based on the contents of the offset
RAM

The offset RAM is addressed in this format:
1 0 0 0 P P P V V V V V V V V V
        (Page)   (scanline)
Page 4=D, 5=E, 6=F

Page D, E, and F are drawn offset from the top of the screen,
starting on the first scanline which contains a 1 in the
appropriate memory location

---

The composited monitor image is seen in a mirror.  It appears
to move when the player moves the handle, due to motors which
tilt the mirror up and down, and the monitor left and right.

---

As if this wasn't enough, there are also "3D" fire beams made
of 120 green LED's which are on a mechanism in front of the mirror.
Along with a single red "sight" LED.  I am reading in the sequence
ROMS and building up a character set to simulate the LEDS with
conventional character graphics.

Finally, there is a score display made of 7-segment LEDS, along
with Credits, Barriers, and Rounds displays made of some other
type of LED bar graphs.  I'm displaying them the best I can on the
bottom line of the screen

***************************************************************************/

#include "driver.h"

/* These are defined in machine/stactics.c */
extern int stactics_vert_pos;
extern int stactics_horiz_pos;
extern UINT8 *stactics_motor_on;

/* These are needed by machine/stactics.c  */
int stactics_vblank_count;
int stactics_shot_standby;
int stactics_shot_arrive;

/* These are needed by driver/stactics.c   */
UINT8 *stactics_scroll_ram;
UINT8 *stactics_videoram_b;
UINT8 *stactics_chardata_b;
UINT8 *stactics_videoram_d;
UINT8 *stactics_chardata_d;
UINT8 *stactics_videoram_e;
UINT8 *stactics_chardata_e;
UINT8 *stactics_videoram_f;
UINT8 *stactics_chardata_f;
UINT8 *stactics_display_buffer;

static UINT8 *dirty_videoram_b;
static UINT8 *dirty_chardata_b;
static UINT8 *dirty_videoram_d;
static UINT8 *dirty_chardata_d;
static UINT8 *dirty_videoram_e;
static UINT8 *dirty_chardata_e;
static UINT8 *dirty_videoram_f;
static UINT8 *dirty_chardata_f;

static int d_offset;
static int e_offset;
static int f_offset;

static int palette_select;

static mame_bitmap *tmpbitmap2;
static mame_bitmap *bitmap_B;
static mame_bitmap *bitmap_D;
static mame_bitmap *bitmap_E;
static mame_bitmap *bitmap_F;

static UINT8 *beamdata;
static int states_per_frame;

#define DIRTY_CHARDATA_SIZE  0x100
#define BEAMDATA_SIZE        0x800

/* The first 16 came from the 7448 BCD to 7-segment decoder data sheet */
/* The rest are made up */

static UINT8 stactics_special_chars[32*8] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,   /* Space */
    0x80, 0x80, 0x80, 0xf0, 0x80, 0x80, 0xf0, 0x00,   /* extras... */
    0xf0, 0x80, 0x80, 0xf0, 0x00, 0x00, 0xf0, 0x00,   /* extras... */
    0x90, 0x90, 0x90, 0xf0, 0x00, 0x00, 0x00, 0x00,   /* extras... */
    0x00, 0x00, 0x00, 0xf0, 0x10, 0x10, 0xf0, 0x00,   /* extras... */
    0x00, 0x00, 0x00, 0xf0, 0x80, 0x80, 0xf0, 0x00,   /* extras... */
    0xf0, 0x90, 0x90, 0xf0, 0x10, 0x10, 0xf0, 0x00,   /* 9 */
    0xf0, 0x90, 0x90, 0xf0, 0x90, 0x90, 0xf0, 0x00,   /* 8 */
    0xf0, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x00,   /* 7 */
    0xf0, 0x80, 0x80, 0xf0, 0x90, 0x90, 0xf0, 0x00,   /* 6 */
    0xf0, 0x80, 0x80, 0xf0, 0x10, 0x10, 0xf0, 0x00,   /* 5 */
    0x90, 0x90, 0x90, 0xf0, 0x10, 0x10, 0x10, 0x00,   /* 4 */
    0xf0, 0x10, 0x10, 0xf0, 0x10, 0x10, 0xf0, 0x00,   /* 3 */
    0xf0, 0x10, 0x10, 0xf0, 0x80, 0x80, 0xf0, 0x00,   /* 2 */
    0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x00,   /* 1 */
    0xf0, 0x90, 0x90, 0x90, 0x90, 0x90, 0xf0, 0x00,   /* 0 */

    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,   /* Space */
    0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,   /* 1 pip */
    0x60, 0x90, 0x80, 0x60, 0x10, 0x90, 0x60, 0x00,   /* S for Score */
    0x80, 0x00, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00,   /* 2 pips */
    0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 0x00, 0x00,   /* 3 pips */
    0x60, 0x90, 0x80, 0x80, 0x80, 0x90, 0x60, 0x00,   /* C for Credits */
    0xe0, 0x90, 0x90, 0xe0, 0x90, 0x90, 0xe0, 0x00,   /* B for Barriers */
    0xe0, 0x90, 0x90, 0xe0, 0xc0, 0xa0, 0x90, 0x00,   /* R for Rounds */
    0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 0x80, 0x00,   /* 4 pips */
    0x00, 0x60, 0x60, 0x00, 0x60, 0x60, 0x00, 0x00,   /* Colon */
    0x40, 0xe0, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00,   /* Sight */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,   /* Space (Unused) */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,   /* Space (Unused) */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,   /* Space (Unused) */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,   /* Space (Unused) */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00    /* Space */
};


static int firebeam_state;
static int old_firebeam_state;

PALETTE_INIT( stactics )
{
    int i,j;

    for (i=0;i<16;i++)
    {
        int bit0,bit1,bit2, bit3,r,g,b;

        bit0 = i & 1;
        bit1 = (i >> 1) & 1;
        bit2 = (i >> 2) & 1;
        bit3 = (i >> 3) & 1;

        /* red component */
        r = 0xff * bit0;

        /* green component */
        g = 0xff * bit1 - 0xcc * bit3;

        /* blue component */
        b = 0xff * bit2;

		palette_set_color(machine,i,MAKE_RGB(r,g,b));
    }

    /* The color prom in Space Tactics is used for both   */
    /* color codes, and priority layering of the 4 layers */

    /* Since we are taking care of the layering by our    */
    /* drawing order, we don't need all of the color prom */
    /* entries */

    /* For each of 4 color schemes */
    for(i=0;i<4;i++)
    {
        /* For page B - Alphanumerics and alien shots */
        for(j=0;j<16;j++)
        {
            *(colortable++) = 0;
            *(colortable++) = color_prom[i*0x100+0x01*0x10+j];
        }
        /* For page F - Close Aliens (these are all the same color) */
        for(j=0;j<16;j++)
        {
            *(colortable++) = 0;
            *(colortable++) = color_prom[i*0x100+0x02*0x10];
        }
        /* For page E - Medium Aliens (these are all the same color) */
        for(j=0;j<16;j++)
        {
            *(colortable++) = 0;
            *(colortable++) = color_prom[i*0x100+0x04*0x10+j];
        }
        /* For page D - Far Aliens (these are all the same color) */
        for(j=0;j<16;j++)
        {
            *(colortable++) = 0;
            *(colortable++) = color_prom[i*0x100+0x08*0x10+j];
        }
    }
}

/***************************************************************************

  Start the video hardware emulation.

***************************************************************************/

VIDEO_START( stactics )
{
    int i,j;
    const UINT8 *firebeam_data;
    UINT8 firechar[256*8*9];

    tmpbitmap  = auto_bitmap_alloc(machine->screen[0].width,machine->screen[0].height,machine->screen[0].format);
    tmpbitmap2 = auto_bitmap_alloc(machine->screen[0].width,machine->screen[0].height,machine->screen[0].format);
    bitmap_B = auto_bitmap_alloc(machine->screen[0].width,machine->screen[0].height,machine->screen[0].format);
    bitmap_D = auto_bitmap_alloc(machine->screen[0].width,machine->screen[0].height,machine->screen[0].format);
    bitmap_E = auto_bitmap_alloc(machine->screen[0].width,machine->screen[0].height,machine->screen[0].format);
    bitmap_F = auto_bitmap_alloc(machine->screen[0].width,machine->screen[0].height,machine->screen[0].format);

	/* Allocate dirty buffers */
	dirty_videoram_b = auto_malloc(videoram_size);
	dirty_videoram_d = auto_malloc(videoram_size);
	dirty_videoram_e = auto_malloc(videoram_size);
	dirty_videoram_f = auto_malloc(videoram_size);
	dirty_chardata_b = auto_malloc(DIRTY_CHARDATA_SIZE);
	dirty_chardata_d = auto_malloc(DIRTY_CHARDATA_SIZE);
	dirty_chardata_e = auto_malloc(DIRTY_CHARDATA_SIZE);
	dirty_chardata_f = auto_malloc(DIRTY_CHARDATA_SIZE);

    memset(dirty_videoram_b,1,videoram_size);
    memset(dirty_videoram_d,1,videoram_size);
    memset(dirty_videoram_e,1,videoram_size);
    memset(dirty_videoram_f,1,videoram_size);
    memset(dirty_chardata_b,1,DIRTY_CHARDATA_SIZE);
    memset(dirty_chardata_d,1,DIRTY_CHARDATA_SIZE);
    memset(dirty_chardata_e,1,DIRTY_CHARDATA_SIZE);
    memset(dirty_chardata_f,1,DIRTY_CHARDATA_SIZE);

    d_offset = 0;
    e_offset = 0;
    f_offset = 0;

    palette_select = 0;
    stactics_vblank_count = 0;
    stactics_shot_standby = 1;
    stactics_shot_arrive = 0;
    firebeam_state = 0;
    old_firebeam_state = 0;

    /* Create a fake character set for LED fire beam */

    memset(firechar,0,sizeof(firechar));
    for(i=0;i<256;i++)
    {
        for(j=0;j<8;j++)
        {
            if ((i>>j)&0x01)
            {
                firechar[i*9+(7-j)]   |= (0x01<<(7-j));
                firechar[i*9+(7-j)+1] |= (0x01<<(7-j));
            }
        }
    }

    for(i=0;i<256;i++)
    {
        decodechar(machine->gfx[4],
                   i,
                   firechar,
                   machine->drv->gfxdecodeinfo[4].gfxlayout);
    }

    /* Decode the Fire Beam ROM for later      */
    /* (I am basically just juggling the bytes */
    /* and storing it again to make it easier) */

	beamdata = auto_malloc(BEAMDATA_SIZE);

    firebeam_data = memory_region(REGION_GFX1);

    for(i=0;i<256;i++)
    {
        beamdata[i*8]   = firebeam_data[i                   ];
        beamdata[i*8+1] = firebeam_data[i + 1024            ];
        beamdata[i*8+2] = firebeam_data[i              + 256];
        beamdata[i*8+3] = firebeam_data[i + 1024       + 256];
        beamdata[i*8+4] = firebeam_data[i        + 512      ];
        beamdata[i*8+5] = firebeam_data[i + 1024 + 512      ];
        beamdata[i*8+6] = firebeam_data[i        + 512 + 256];
        beamdata[i*8+7] = firebeam_data[i + 1024 + 512 + 256];
    }

    /* Build some characters for simulating the LED displays */

    for(i=0;i<32;i++)
    {
        decodechar(machine->gfx[5],
                   i,
                   stactics_special_chars,
                   machine->drv->gfxdecodeinfo[5].gfxlayout);
    }

    stactics_vblank_count = 0;
    stactics_vert_pos = 0;
    stactics_horiz_pos = 0;
    *stactics_motor_on = 0;
}


WRITE8_HANDLER( stactics_palette_w )
{
    int old_palette_select = palette_select;

    switch (offset)
    {
        case 0:
            palette_select = (palette_select & 0x02) | (data&0x01);
            break;
        case 1:
            palette_select = (palette_select & 0x01) | ((data&0x01)<<1);
            break;
        default:
            return;
    }

    if (old_palette_select != palette_select)
    {
        memset(dirty_videoram_b,1,videoram_size);
        memset(dirty_videoram_d,1,videoram_size);
        memset(dirty_videoram_e,1,videoram_size);
        memset(dirty_videoram_f,1,videoram_size);
    }
    return;
}


WRITE8_HANDLER( stactics_scroll_ram_w )
{
    int temp;

    if (stactics_scroll_ram[offset] != data)
    {
        stactics_scroll_ram[offset] = data;
        temp = (offset&0x700)>>8;
        switch(temp)
        {
            case 4:  // Page D
            {
                if (data&0x01)
                    d_offset = offset&0xff;
                break;
            }
            case 5:  // Page E
            {
                if (data&0x01)
                    e_offset = offset&0xff;
                break;
            }
            case 6:  // Page F
            {
                if (data&0x01)
                    f_offset = offset&0xff;
                break;
            }
        }
    }
}

WRITE8_HANDLER( stactics_speed_latch_w )
{
    /* This writes to a shift register which is clocked by   */
    /* a 555 oscillator.  This value determines the speed of */
    /* the LED fire beams as follows:                        */

    /*   555_freq / bits_in_SR * edges_in_SR / states_in_PR67 / frame_rate */
    /*      = num_led_states_per_frame  */
    /*   36439 / 8 * x / 32 / 60 ~= 19/8*x */

    /* Here, we will count the number of rising edges in the shift register */

    int i;
    int num_rising_edges = 0;

    for(i=0;i<8;i++)
    {
        if ( (((data>>i)&0x01) == 1) && (((data>>((i+1)%8))&0x01) == 0))
            num_rising_edges++;
    }

    states_per_frame = num_rising_edges*19/8;
}

WRITE8_HANDLER( stactics_shot_trigger_w )
{
    stactics_shot_standby = 0;
}

WRITE8_HANDLER( stactics_shot_flag_clear_w )
{
    stactics_shot_arrive = 0;
}

WRITE8_HANDLER( stactics_videoram_b_w )
{
    if (stactics_videoram_b[offset] != data)
    {
        stactics_videoram_b[offset] = data;
        dirty_videoram_b[offset] = 1;
    }
}

WRITE8_HANDLER( stactics_chardata_b_w )
{
    if (stactics_chardata_b[offset] != data)
    {
        stactics_chardata_b[offset] = data;
        dirty_chardata_b[offset>>3] = 1;
    }
}

WRITE8_HANDLER( stactics_videoram_d_w )
{
    if (stactics_videoram_d[offset] != data)
    {
        stactics_videoram_d[offset] = data;
        dirty_videoram_d[offset] = 1;
    }
}

WRITE8_HANDLER( stactics_chardata_d_w )
{
    if (stactics_chardata_d[offset] != data)
    {
        stactics_chardata_d[offset] = data;
        dirty_chardata_d[offset>>3] = 1;
    }
}

WRITE8_HANDLER( stactics_videoram_e_w )
{
    if (stactics_videoram_e[offset] != data)
    {
        stactics_videoram_e[offset] = data;
        dirty_videoram_e[offset] = 1;
    }
}

WRITE8_HANDLER( stactics_chardata_e_w )
{
    if (stactics_chardata_e[offset] != data)
    {
        stactics_chardata_e[offset] = data;
        dirty_chardata_e[offset>>3] = 1;
    }
}

WRITE8_HANDLER( stactics_videoram_f_w )
{
    if (stactics_videoram_f[offset] != data)
    {
        stactics_videoram_f[offset] = data;
        dirty_videoram_f[offset] = 1;
    }
}

WRITE8_HANDLER( stactics_chardata_f_w )
{
    if (stactics_chardata_f[offset] != data)
    {
        stactics_chardata_f[offset] = data;
        dirty_chardata_f[offset>>3] = 1;
    }
}

/* Actual area for visible monitor stuff is only 30*8 lines */
/* The rest is used for the score, etc. */

static const rectangle visible_screen_area = {0*8, 32*8, 0*8, 30*8};

VIDEO_UPDATE( stactics )
{
    int offs, sx, sy, i;
    int char_number;
    int color_code;
    int pixel_x, pixel_y;

    int palette_offset = palette_select * 64;

    for(offs=0x400-1; offs>=0; offs--)
    {
        sx = offs%32;
        sy = offs/32;

        color_code = palette_offset + (stactics_videoram_b[offs]>>4);

        /* Draw aliens in Page D */

        char_number = stactics_videoram_d[offs];

        if (dirty_chardata_d[char_number] == 1)
        {
            decodechar(machine->gfx[3],
                       char_number,
                       stactics_chardata_d,
                       machine->drv->gfxdecodeinfo[3].gfxlayout);
            dirty_chardata_d[char_number] = 2;
            dirty_videoram_d[offs] = 1;
        }
        else if (dirty_chardata_d[char_number] == 2)
        {
            dirty_videoram_d[offs] = 1;
        }

        if (dirty_videoram_d[offs])
        {
            drawgfx(bitmap_D,machine->gfx[3],
                    char_number,
                    color_code,
                    0,0,
                    sx*8,sy*8,
                    &machine->screen[0].visarea,TRANSPARENCY_NONE,0);
            dirty_videoram_d[offs] = 0;
        }

        /* Draw aliens in Page E */

        char_number = stactics_videoram_e[offs];

        if (dirty_chardata_e[char_number] == 1)
        {
            decodechar(machine->gfx[2],
                       char_number,
                       stactics_chardata_e,
                       machine->drv->gfxdecodeinfo[2].gfxlayout);
            dirty_chardata_e[char_number] = 2;
            dirty_videoram_e[offs] = 1;
        }
        else if (dirty_chardata_e[char_number] == 2)
        {
            dirty_videoram_e[offs] = 1;
        }

        if (dirty_videoram_e[offs])
        {
            drawgfx(bitmap_E,machine->gfx[2],
                    char_number,
                    color_code,
                    0,0,
                    sx*8,sy*8,
                    &machine->screen[0].visarea,TRANSPARENCY_NONE,0);
            dirty_videoram_e[offs] = 0;
        }

        /* Draw aliens in Page F */

        char_number = stactics_videoram_f[offs];

        if (dirty_chardata_f[char_number] == 1)
        {
            decodechar(machine->gfx[1],
                       char_number,
                       stactics_chardata_f,
                       machine->drv->gfxdecodeinfo[1].gfxlayout);
            dirty_chardata_f[char_number] = 2;
            dirty_videoram_f[offs] = 1;
        }
        else if (dirty_chardata_f[char_number] == 2)
        {
            dirty_videoram_f[offs] = 1;
        }

        if (dirty_videoram_f[offs])
        {
            drawgfx(bitmap_F,machine->gfx[1],
                    char_number,
                    color_code,
                    0,0,
                    sx*8,sy*8,
                    &machine->screen[0].visarea,TRANSPARENCY_NONE,0);
            dirty_videoram_f[offs] = 0;
        }

        /* Draw the page B stuff */

        char_number = stactics_videoram_b[offs];

        if (dirty_chardata_b[char_number] == 1)
        {
            decodechar(machine->gfx[0],
                       char_number,
                       stactics_chardata_b,
                       machine->drv->gfxdecodeinfo[0].gfxlayout);
            dirty_chardata_b[char_number] = 2;
            dirty_videoram_b[offs] = 1;
        }
        else if (dirty_chardata_b[char_number] == 2)
        {
            dirty_videoram_b[offs] = 1;
        }

        if (dirty_videoram_b[offs])
        {
            drawgfx(bitmap_B,machine->gfx[0],
                    char_number,
                    color_code,
                    0,0,
                    sx*8,sy*8,
                    &machine->screen[0].visarea,TRANSPARENCY_NONE,0);
            dirty_videoram_b[offs] = 0;
        }

    }

    /* Now, composite the four layers together */

    copyscrollbitmap(tmpbitmap2,bitmap_D,0,0,1,&d_offset,
                     &machine->screen[0].visarea,TRANSPARENCY_NONE,0);
    copyscrollbitmap(tmpbitmap2,bitmap_E,0,0,1,&e_offset,
                     &machine->screen[0].visarea,TRANSPARENCY_COLOR,0);
    copyscrollbitmap(tmpbitmap2,bitmap_F,0,0,1,&f_offset,
                     &machine->screen[0].visarea,TRANSPARENCY_COLOR,0);
    copybitmap(tmpbitmap2,bitmap_B,0,0,0,0,
                     &machine->screen[0].visarea,TRANSPARENCY_COLOR,0);

    /* Now flip X & simulate the monitor motion */
    fillbitmap(bitmap, machine->pens[0], &machine->screen[0].visarea);
    copybitmap(bitmap,tmpbitmap2,1,0,stactics_horiz_pos,stactics_vert_pos,
                &visible_screen_area,TRANSPARENCY_NONE,0);

    /* Finally, draw stuff that is on the console or on top of the monitor (LED's) */

    /***** Draw Score Display *****/

    pixel_x = 16;
    pixel_y = 248;

    /* Draw an S */
    drawgfx(bitmap,machine->gfx[5],
            18,
            0,
            0,0,
            pixel_x,pixel_y,
            &machine->screen[0].visarea,TRANSPARENCY_NONE,0);
    pixel_x+=6;
    /* Draw a colon */
    drawgfx(bitmap,machine->gfx[5],
            25,
            0,
            0,0,
            pixel_x,pixel_y,
            &machine->screen[0].visarea,TRANSPARENCY_NONE,0);
    pixel_x+=6;
    /* Draw the digits */
    for(i=1;i<7;i++)
    {
        drawgfx(bitmap,machine->gfx[5],
                stactics_display_buffer[i]&0x0f,
                16,
                0,0,
                pixel_x,pixel_y,
                &machine->screen[0].visarea,TRANSPARENCY_NONE,0);
        pixel_x+=6;
    }

    /***** Draw Credits Indicator *****/

    pixel_x = 64+16;

    /* Draw a C */
    drawgfx(bitmap,machine->gfx[5],
            21,
            0,
            0,0,
            pixel_x,pixel_y,
            &machine->screen[0].visarea,TRANSPARENCY_NONE,0);
    pixel_x+=6;
    /* Draw a colon */
    drawgfx(bitmap,machine->gfx[5],
            25,
            0,
            0,0,
            pixel_x,pixel_y,
            &machine->screen[0].visarea,TRANSPARENCY_NONE,0);
    pixel_x+=6;
    /* Draw the pips */
    for(i=7;i<9;i++)
    {
        drawgfx(bitmap,machine->gfx[5],
                16 + (~stactics_display_buffer[i]&0x0f),
                16,
                0,0,
                pixel_x,pixel_y,
                &machine->screen[0].visarea,TRANSPARENCY_NONE,0);
        pixel_x+=2;
    }

    /***** Draw Rounds Indicator *****/

    pixel_x = 128+16;

    /* Draw an R */
    drawgfx(bitmap,machine->gfx[5],
            22,
            0,
            0,0,
            pixel_x,pixel_y,
            &machine->screen[0].visarea,TRANSPARENCY_NONE,0);
    pixel_x+=6;
    /* Draw a colon */
    drawgfx(bitmap,machine->gfx[5],
            25,
            0,
            0,0,
            pixel_x,pixel_y,
            &machine->screen[0].visarea,TRANSPARENCY_NONE,0);
    pixel_x+=6;
    /* Draw the pips */
    for(i=9;i<12;i++)
    {
        drawgfx(bitmap,machine->gfx[5],
                16 + (~stactics_display_buffer[i]&0x0f),
                16,
                0,0,
                pixel_x,pixel_y,
                &machine->screen[0].visarea,TRANSPARENCY_NONE,0);
        pixel_x+=2;
    }

    /***** Draw Barriers Indicator *****/

    pixel_x = 192+16;
    /* Draw a B */
    drawgfx(bitmap,machine->gfx[5],
            23,
            0,
            0,0,
            pixel_x,pixel_y,
            &machine->screen[0].visarea,TRANSPARENCY_NONE,0);
    pixel_x+=6;
    /* Draw a colon */
    drawgfx(bitmap,machine->gfx[5],
            25,
            0,
            0,0,
            pixel_x,pixel_y,
            &machine->screen[0].visarea,TRANSPARENCY_NONE,0);
    pixel_x+=6;
    /* Draw the pips */
    for(i=12;i<16;i++)
    {
        drawgfx(bitmap,machine->gfx[5],
                16 + (~stactics_display_buffer[i]&0x0f),
                16,
                0,0,
                pixel_x,pixel_y,
                &machine->screen[0].visarea,TRANSPARENCY_NONE,0);
        pixel_x+=2;
    }

    /* An LED fire beam! */
    /* (There were 120 green LEDS mounted in the cabinet in the game, */
    /*  and one red one, for the sight)                               */

    /* First, update the firebeam state */

    old_firebeam_state = firebeam_state;
    if (stactics_shot_standby == 0)
    {
        firebeam_state = (firebeam_state + states_per_frame)%512;
    }

    /* These are thresholds for the two shots from the LED fire ROM */
    /* (Note: There are two more for sound triggers, */
    /*        whenever that gets implemented)        */

    if ((old_firebeam_state < 0x8b) & (firebeam_state >= 0x8b))
        stactics_shot_arrive = 1;

    if ((old_firebeam_state < 0xca) & (firebeam_state >= 0xca))
        stactics_shot_arrive = 1;

    if (firebeam_state > 255)
    {
        firebeam_state = 0;
        stactics_shot_standby = 1;
    }

    /* Now, draw the beam */

    pixel_x = 15;
    pixel_y = 166;

    for(i=0;i<8;i++)
    {
        if ((i%2)==1)
        {
            /* Draw 7 LEDS on each side */
            drawgfx(bitmap,machine->gfx[4],
                    beamdata[firebeam_state*8+i]&0x7f,
                    16*2,  /* Make it green */
                    0,0,
                    pixel_x,pixel_y,
                    &machine->screen[0].visarea,TRANSPARENCY_COLOR,0);
            drawgfx(bitmap,machine->gfx[4],
                    beamdata[firebeam_state*8+i]&0x7f,
                    16*2,  /* Make it green */
                    1,0,
                    255-pixel_x,pixel_y,
                    &machine->screen[0].visarea,TRANSPARENCY_COLOR,0);
            pixel_x+=14;
            pixel_y-=7;
        }
        else
        {
            /* Draw 8 LEDS on each side */
            drawgfx(bitmap,machine->gfx[4],
                    beamdata[firebeam_state*8+i],
                    16*2,  /* Make it green */
                    0,0,
                    pixel_x,pixel_y,
                    &machine->screen[0].visarea,TRANSPARENCY_COLOR,0);
            drawgfx(bitmap,machine->gfx[4],
                    beamdata[firebeam_state*8+i],
                    16*2,  /* Make it green */
                    1,0,
                    255-pixel_x,pixel_y,
                    &machine->screen[0].visarea,TRANSPARENCY_COLOR,0);
            pixel_x+=16;
            pixel_y-=8;
        }

    }

    /* Red Sight LED */

    pixel_x = 134;
    pixel_y = 112;

    if (*stactics_motor_on & 0x01)
    {
        drawgfx(bitmap,machine->gfx[5],
                26,
                16, /* red */
                0,0,
                pixel_x,pixel_y,
                &machine->screen[0].visarea,TRANSPARENCY_COLOR,0);
    }

    /* Update vblank counter */
    stactics_vblank_count++;

    /* reset dirty flags */
    for(i=0;i<0xff;i++)
    {
        dirty_chardata_b[i] &= 0x01;
        dirty_chardata_d[i] &= 0x01;
        dirty_chardata_e[i] &= 0x01;
        dirty_chardata_f[i] &= 0x01;
    }

	return 0;
}
