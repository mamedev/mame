/* Sega Saturn VDP2 */

/*Debug features,remember to zero it if you publish this file.*/
#define DEBUG_MODE 0

/*

the dirty marking stuff and tile decoding will probably be removed in the end anyway as we'll need custom
rendering code since mame's drawgfx / tilesytem don't offer everything st-v needs

this system seems far too complex to use Mame's tilemap system

4 'scroll' planes (scroll screens)

the scroll planes have slightly different capabilities

NBG0
NBG1
NBG2
NBG3

2 'rotate' planes

RBG0
RBG1

-- other crap
EXBG (external)

-----------------------------------------------------------------------------------------------------------

Video emulation TODO:
-all games:
 \-priorities (check myfairld,thunt)
 \-complete windows effects
 \-mosaic effect
 \-ODD bit/H/V Counter not yet emulated properly
 \-Reduction enable bits
 \-Check if there are any remaining video registers that are yet to be macroized & added to the rumble.
-batmanfr:
 \-If you reset the game after the character selection screen,when you get again to it there's garbage
   floating behind Batman.
-elandore:
 \-(BTANB) priorities at the VS. screen apparently is wrong,but it's like this on the Saturn version too.
-hanagumi:
 \-ending screens have corrupt graphics. (*untested*)
-kiwames:
 \-incorrect color emulation for the alpha blended flames on the title screen,it's caused by a schizoid
   linescroll emulation quirk.
 \-the VDP1 sprites refresh is too slow,causing the "Draw by request" mode to
   flicker.Moved back to default ATM.
-pblbeach:
 \-Sprites are offset, because it doesn't clear vdp1 local coordinates set by bios,
   I guess that they are cleared when some vdp1 register is written (kludged for now)
-prikura:
 \-Attract mode presentation has corrupted graphics in various places,probably caused by incomplete
   framebuffer data delete.
-seabass:
 \-Player sprite is corrupt/missing during movements,caused by incomplete framebuffer switching.

Notes of Interest & Unclear features:

-the test mode / bios is drawn with layer NBG3;
-hanagumi puts a 'RED' dragon logo in tileram (base 0x64000, 4bpp, 8x8 tiles) but
its not displayed because its priority value is 0.Left-over?

-scrolling is screen display wise,meaning that a scrolling value is masked with the
screen resolution size values;

-H-Blank bit is INDIPENDENT of the V-Blank bit...trying to fix enable/disable it during V-Blank period
 causes wrong gameplay speed in Golden Axe:The Duel.

-Bitmaps uses transparency pens,examples are:
\-elandore's energy bars;
\-mausuke's foreground(the one used on the playfield)
\-shanhigw's tile-based sprites;
The transparency pen table is like this:

|------------------|---------------------|
| Character count  | Transparency code   |
|------------------|---------------------|
| 16 colors        |=0x0 (4 bits)        |
| 256 colors       |=0x00 (8 bits)       |
| 2048 colors      |=0x000 (11 bits)     |
| 32,768 colors    |MSB=0 (bit 15)       |
| 16,770,000 colors|MSB=0 (bit 31)       |
|------------------|---------------------|
In other words,the first three types uses the offset and not the color allocated.

-double density interlace setting (LSMD == 3) apparently does a lot of fancy stuff in the graphics sizes.

-Debug key list(only if you enable the debug mode on top of this file):
    \-T: NBG3 layer toggle
    \-Y: NBG2 layer toggle
    \-U: NBG1 layer toggle
    \-I: NBG0 layer toggle
    \-O: SPRITE toggle
    \-K: RBG0 layer toggle
    \-W Decodes the graphics for F4 menu.
    \-M Stores VDP1 ram contents from a file.
    \-N Stores VDP1 ram contents into a file.
*/

#include "driver.h"
#include "eminline.h"
#include "profiler.h"
#include "includes/stv.h"

UINT32* stv_vdp2_regs;
UINT32* stv_vdp2_vram;

static UINT8* stv_vdp2_gfx_decode;

static int stv_vdp2_render_rbg0;
int stv_hblank,stv_vblank;
static int stv_odd;
static int horz_res,vert_res;

UINT32* stv_vdp2_cram;

static void stv_vdp2_dynamic_res_change(running_machine *machine);
static UINT8 get_hblank(running_machine *machine);
static int get_vblank_duration(running_machine *machine);
static int get_hblank_duration(running_machine *machine);
static UINT8 get_odd_bit(running_machine *machine);

static void refresh_palette_data(running_machine *machine);
static int stv_vdp2_window_process(int x,int y);
static int stv_vdp2_apply_window_on_layer(rectangle *cliprect);
static void stv_vdp2_get_window0_coordinates(UINT16 *s_x, UINT16 *e_x, UINT16 *s_y, UINT16 *e_y);
static void stv_vdp2_check_tilemap(running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect);
static bitmap_t *stv_vdp2_roz_bitmap[2];

enum
{
	STV_TRANSPARENCY_NONE,
	STV_TRANSPARENCY_PEN,
	STV_TRANSPARENCY_ADD_BLEND,
	STV_TRANSPARENCY_ALPHA
};

#if DEBUG_MODE
#define LOG_VDP2 1
#define LOG_ROZ 0
#else
#define LOG_VDP2 0
#define LOG_ROZ 0
#endif

/*

-------------------------------------------------|-----------------------------|------------------------------
|  Function        |  Normal Scroll Screen                                     |  Rotation Scroll Screen     |
|                  |-----------------------------|-----------------------------|------------------------------
|                  | NBG0         | NBG1         | NBG2         | NBG3         | RBG0         | RBG1         |
-------------------------------------------------|-----------------------------|------------------------------
| Character Colour | 16 colours   | 16 colours   | 16 colours   | 16 colours   | 16 colours   | 16 colours   |
| Count            | 256 " "      | 256 " "      | 256 " "      | 256 " "      | 256 " "      | 256 " "      |
|                  | 2048 " "     | 2048 " "     |              |              | 2048 " "     | 2048 " "     |
|                  | 32768 " "    | 32768 " "    |              |              | 32768 " "    | 32768 " "    |
|                  | 16770000 " " |              |              |              | 16770000 " " | 16770000 " " |
-------------------------------------------------|-----------------------------|------------------------------
| Character Size   | 1x1 Cells , 2x2 Cells                                                                   |
-------------------------------------------------|-----------------------------|------------------------------
| Pattern Name     | 1 word , 2 words                                                                        |
| Data Size        |                                                                                         |
-------------------------------------------------|-----------------------------|------------------------------
| Plane Size       | 1 H x 1 V 1 Pages ; 2 H x 1 V 1 Pages ; 2 H x 2 V Pages (I don't understand ... )       |
-------------------------------------------------|-----------------------------|------------------------------
| Plane Count      | 4                                                         | 16                          |
-------------------------------------------------|-----------------------------|------------------------------
| Bitmap Possible  | Yes                         | No                          | Yes          | No           |
-------------------------------------------------|-----------------------------|------------------------------
| Bitmap Size      | 512 x 256                   | N/A                         | 512x256      | N/A          |
|                  | 512 x 512                   |                             | 512x512      |              |
|                  | 1024 x 256                  |                             |              |              |
|                  | 1024 x 512                  |                             |              |              |
-------------------------------------------------|-----------------------------|------------------------------
| Scale            | 0.25 x - 256 x              | None                        | Any ?                       |
-------------------------------------------------|-----------------------------|------------------------------
| Rotation         | No                                                        | Yes                         |
-------------------------------------------------|-----------------------------|-----------------------------|
| Linescroll       | Yes                         | No                                                        |
-------------------------------------------------|-----------------------------|------------------------------
| Column Scroll    | Yes                         | No                                                        |
-------------------------------------------------|-----------------------------|------------------------------
| Mosaic           | Yes                                                       | Horizontal Only             |
-------------------------------------------------|-----------------------------|------------------------------

*/

/* 180000 - r/w - TVMD - TV Screen Mode
 bit-> /----15----|----14----|----13----|----12----|----11----|----10----|----09----|----08----\
       | DISP     |    --    |    --    |    --    |    --    |    --    |    --    | BDCLMD   |
       |----07----|----06----|----05----|----04----|----03----|----02----|----01----|----00----|
       | LSMD1    | LSMD0    | VRESO1   | VRESO0   |    --    | HRESO2   | HRESO1   | HRESO0   |
       \----------|----------|----------|----------|----------|----------|----------|---------*/

	#define STV_VDP2_TVMD 	((stv_vdp2_regs[0x000/4] >> 16)&0x0000ffff)

	#define STV_VDP2_DISP   ((STV_VDP2_TVMD & 0x8000) >> 15)
	#define STV_VDP2_BDCLMD	((STV_VDP2_TVMD & 0x0100) >> 8)
	#define STV_VDP2_LSMD 	((STV_VDP2_TVMD & 0x00c0) >> 6)
	#define STV_VDP2_VRES 	((STV_VDP2_TVMD & 0x0030) >> 4)
	#define STV_VDP2_HRES 	((STV_VDP2_TVMD & 0x0007) >> 0)

/* 180002 - r/w - EXTEN - External Signal Enable Register
 bit-> /----15----|----14----|----13----|----12----|----11----|----10----|----09----|----08----\
       |    --    |    --    |    --    |    --    |    --    |    --    | EXLTEN   | EXSYEN   |
       |----07----|----06----|----05----|----04----|----03----|----02----|----01----|----00----|
       |    --    |    --    |    --    |    --    |    --    |    --    | DASEL    | EXBGEN   |
       \----------|----------|----------|----------|----------|----------|----------|---------*/

/* 180004 - r/o - TVSTAT - Screen Status
 bit-> /----15----|----14----|----13----|----12----|----11----|----10----|----09----|----08----\
       |    --    |    --    |    --    |    --    |    --    |    --    | EXLTFG   | EXSYFG   |
       |----07----|----06----|----05----|----04----|----03----|----02----|----01----|----00----|
       |    --    |    --    |    --    |    --    | VBLANK   | HBLANK   | ODD      | EVEN     |
       \----------|----------|----------|----------|----------|----------|----------|---------*/

/* 180006 - r/w - VRSIZE - VRAM Size
 bit-> /----15----|----14----|----13----|----12----|----11----|----10----|----09----|----08----\
       | VRAMSZ   |    --    |    --    |    --    |    --    |    --    |    --    |    --    |
       |----07----|----06----|----05----|----04----|----03----|----02----|----01----|----00----|
       |    --    |    --    |    --    |    --    | VER3     | VER2     | VER1     | VER0     |
       \----------|----------|----------|----------|----------|----------|----------|---------*/

	#define STV_VDP2_VRSIZE ((stv_vdp2_regs[0x004/4] >> 0)&0x0000ffff)

	#define STV_VDP2_VRAMSZ ((STV_VDP2_VRSIZE & 0x8000) >> 15)

/* 180008 - r/o - HCNT - H-Counter
 bit-> /----15----|----14----|----13----|----12----|----11----|----10----|----09----|----08----\
       |    --    |    --    |    --    |    --    |    --    |    --    | HCT9     | HCT8     |
       |----07----|----06----|----05----|----04----|----03----|----02----|----01----|----00----|
       | HCT7     | HCT6     | HCT5     | HCT4     | HCT3     | HCT2     | HCT1     | HCT0     |
       \----------|----------|----------|----------|----------|----------|----------|---------*/

	#define STV_VDP2_HCNT ((stv_vdp2_regs[0x008/4] >> 16)&0x000003ff)

/* 18000A - r/o - VCNT - V-Counter
 bit-> /----15----|----14----|----13----|----12----|----11----|----10----|----09----|----08----\
       |    --    |    --    |    --    |    --    |    --    |    --    | VCT9     | VCT8     |
       |----07----|----06----|----05----|----04----|----03----|----02----|----01----|----00----|
       | VCT7     | VCT6     | VCT5     | VCT4     | VCT3     | VCT2     | VCT1     | VCT0     |
       \----------|----------|----------|----------|----------|----------|----------|---------*/

	#define STV_VDP2_VCNT ((stv_vdp2_regs[0x008/4] >> 0)&0x000003ff)

/* 18000C - RESERVED
 bit-> /----15----|----14----|----13----|----12----|----11----|----10----|----09----|----08----\
       |    --    |    --    |    --    |    --    |    --    |    --    |    --    |    --    |
       |----07----|----06----|----05----|----04----|----03----|----02----|----01----|----00----|
       |    --    |    --    |    --    |    --    |    --    |    --    |    --    |    --    |
       \----------|----------|----------|----------|----------|----------|----------|---------*/

/* 18000E - r/w - RAMCTL - RAM Control
 bit-> /----15----|----14----|----13----|----12----|----11----|----10----|----09----|----08----\
       |  CRKTE   |    --    | CRMD1    | CRMD0    |    --    |    --    | VRBMD    | VRAMD    |
       |----07----|----06----|----05----|----04----|----03----|----02----|----01----|----00----|
       | RDBSB11  | RDBSB10  | RDBSB01  | RDBSB00  | RDBSA11  | RDBSA10  | RDBSA01  | RDBSA00  |
       \----------|----------|----------|----------|----------|----------|----------|---------*/

	#define STV_VDP2_RAMCTL ((stv_vdp2_regs[0x00c/4] >> 0)&0x0000ffff)

	#define STV_VDP2_CRKTE ((STV_VDP2_RAMCTL & 0x8000) >> 15)
	#define STV_VDP2_CRMD  ((STV_VDP2_RAMCTL & 0x3000) >> 12)
	#define STV_VDP2_RDBSB1	((STV_VDP2_RAMCTL & 0x00c0) >> 6)
	#define STV_VDP2_RDBSB0 ((STV_VDP2_RAMCTL & 0x0030) >> 4)
	#define STV_VDP2_RDBSA1	((STV_VDP2_RAMCTL & 0x000c) >> 2)
	#define STV_VDP2_RDBSA0	((STV_VDP2_RAMCTL & 0x0003) >> 0)


/* 180010 - r/w - -CYCA0L - VRAM CYCLE PATTERN (BANK A0)
 bit-> /----15----|----14----|----13----|----12----|----11----|----10----|----09----|----08----\
       | VCP0A03  | VCP0A02  | VCP0A01  | VCP0A00  | VCP1A03  | VCP1A02  | VCP1A01  | VCP1A00  |
       |----07----|----06----|----05----|----04----|----03----|----02----|----01----|----00----|
       | VCP2A03  | VCP2A02  | VCP2A01  | VCP2A00  | VCP3A03  | VCP3A02  | VCP3A01  | VCP3A00  |
       \----------|----------|----------|----------|----------|----------|----------|---------*/

	#define STV_VDP2_CYCA0L	((stv_vdp2_regs[0x010/4] >> 16)&0x0000ffff)

/* 180012 - r/w - -CYCA0U - VRAM CYCLE PATTERN (BANK A0)
 bit-> /----15----|----14----|----13----|----12----|----11----|----10----|----09----|----08----\
       | VCP4A03  | VCP4A02  | VCP4A01  | VCP4A00  | VCP5A03  | VCP5A02  | VCP5A01  | VCP5A00  |
       |----07----|----06----|----05----|----04----|----03----|----02----|----01----|----00----|
       | VCP6A03  | VCP6A02  | VCP6A01  | VCP6A00  | VCP7A03  | VCP7A02  | VCP7A01  | VCP7A00  |
       \----------|----------|----------|----------|----------|----------|----------|---------*/

	#define STV_VDP2_CYCA0U	((stv_vdp2_regs[0x10/4] >> 0)&0x0000ffff)

/* 180014 - r/w - -CYCA1L - VRAM CYCLE PATTERN (BANK A1)
 bit-> /----15----|----14----|----13----|----12----|----11----|----10----|----09----|----08----\
       | VCP0A13  | VCP0A12  | VCP0A11  | VCP0A10  | VCP1A13  | VCP1A12  | VCP1A11  | VCP1A10  |
       |----07----|----06----|----05----|----04----|----03----|----02----|----01----|----00----|
       | VCP2A13  | VCP2A12  | VCP2A11  | VCP2A10  | VCP3A13  | VCP3A12  | VCP3A11  | VCP3A10  |
       \----------|----------|----------|----------|----------|----------|----------|---------*/

	#define STV_VDP2_CYCA1L	((stv_vdp2_regs[0x014/4] >> 16)&0x0000ffff)

/* 180016 - r/w - -CYCA1U - VRAM CYCLE PATTERN (BANK A1)
 bit-> /----15----|----14----|----13----|----12----|----11----|----10----|----09----|----08----\
       | VCP4A13  | VCP4A12  | VCP4A11  | VCP4A10  | VCP5A13  | VCP5A12  | VCP5A11  | VCP5A10  |
       |----07----|----06----|----05----|----04----|----03----|----02----|----01----|----00----|
       | VCP6A13  | VCP6A12  | VCP6A11  | VCP6A10  | VCP7A13  | VCP7A12  | VCP7A11  | VCP7A10  |
       \----------|----------|----------|----------|----------|----------|----------|---------*/

	#define STV_VDP2_CYCA1U	((stv_vdp2_regs[0x14/4] >> 0)&0x0000ffff)

/* 180018 - r/w - -CYCB0L - VRAM CYCLE PATTERN (BANK B0)
 bit-> /----15----|----14----|----13----|----12----|----11----|----10----|----09----|----08----\
       | VCP0B03  | VCP0B02  | VCP0B01  | VCP0B00  | VCP1B03  | VCP1B02  | VCP1B01  | VCP1B00  |
       |----07----|----06----|----05----|----04----|----03----|----02----|----01----|----00----|
       | VCP2B03  | VCP2B02  | VCP2B01  | VCP2B00  | VCP3B03  | VCP3B02  | VCP3B01  | VCP3B00  |
       \----------|----------|----------|----------|----------|----------|----------|---------*/

	#define STV_VDP2_CYCA2L	((stv_vdp2_regs[0x018/4] >> 16)&0x0000ffff)

/* 18001A - r/w - -CYCB0U - VRAM CYCLE PATTERN (BANK B0)
 bit-> /----15----|----14----|----13----|----12----|----11----|----10----|----09----|----08----\
       | VCP4B03  | VCP4B02  | VCP4B01  | VCP4B00  | VCP5B03  | VCP5B02  | VCP5B01  | VCP5B00  |
       |----07----|----06----|----05----|----04----|----03----|----02----|----01----|----00----|
       | VCP6B03  | VCP6B02  | VCP6B01  | VCP6B00  | VCP7B03  | VCP7B02  | VCP7B01  | VCP7B00  |
       \----------|----------|----------|----------|----------|----------|----------|---------*/

	#define STV_VDP2_CYCA2U	((stv_vdp2_regs[0x18/4] >> 0)&0x0000ffff)

/* 18001C - r/w - -CYCB1L - VRAM CYCLE PATTERN (BANK B1)
 bit-> /----15----|----14----|----13----|----12----|----11----|----10----|----09----|----08----\
       | VCP0B13  | VCP0B12  | VCP0B11  | VCP0B10  | VCP1B13  | VCP1B12  | VCP1B11  | VCP1B10  |
       |----07----|----06----|----05----|----04----|----03----|----02----|----01----|----00----|
       | VCP2B13  | VCP2B12  | VCP2B11  | VCP2B10  | VCP3B13  | VCP3B12  | VCP3B11  | VCP3B10  |
       \----------|----------|----------|----------|----------|----------|----------|---------*/

	#define STV_VDP2_CYCA3L	((stv_vdp2_regs[0x01c/4] >> 16)&0x0000ffff)

/* 18001E - r/w - -CYCB1U - VRAM CYCLE PATTERN (BANK B1)
 bit-> /----15----|----14----|----13----|----12----|----11----|----10----|----09----|----08----\
       | VCP4B13  | VCP4B12  | VCP4B11  | VCP4B10  | VCP5B13  | VCP5B12  | VCP5B11  | VCP5B10  |
       |----07----|----06----|----05----|----04----|----03----|----02----|----01----|----00----|
       | VCP6B13  | VCP6B12  | VCP6B11  | VCP6B10  | VCP7B13  | VCP7B12  | VCP7B11  | VCP7B10  |
       \----------|----------|----------|----------|----------|----------|----------|---------*/

	#define STV_VDP2_CYCA3U	((stv_vdp2_regs[0x1c/4] >> 0)&0x0000ffff)

/* 180020 - r/w - BGON - SCREEN DISPLAY ENABLE

 this register allows each tilemap to be enabled or disabled and also which layers are solid

 bit-> /----15----|----14----|----13----|----12----|----11----|----10----|----09----|----08----\
       |    --    |    --    |    --    | R0TPON   | N3TPON   | N2TPON   | N1TPON   | N0TPON   |
       |----07----|----06----|----05----|----04----|----03----|----02----|----01----|----00----|
       |    --    |    --    | R1ON     | R0ON     | N3ON     | N2ON     | N1ON     | N0ON     |
       \----------|----------|----------|----------|----------|----------|----------|---------*/

	#define STV_VDP2_BGON ((stv_vdp2_regs[0x020/4] >> 16)&0x0000ffff)

	// NxOn - Layer Enable Register
	#define STV_VDP2_xxON ((STV_VDP2_BGON & 0x001f) >> 0) /* to see if anything is enabled */

	#define STV_VDP2_N0ON ((STV_VDP2_BGON & 0x0001) >> 0) /* N0On = NBG0 Enable */
	#define STV_VDP2_N1ON ((STV_VDP2_BGON & 0x0002) >> 1) /* N1On = NBG1 Enable */
	#define STV_VDP2_N2ON ((STV_VDP2_BGON & 0x0004) >> 2) /* N2On = NBG2 Enable */
	#define STV_VDP2_N3ON ((STV_VDP2_BGON & 0x0008) >> 3) /* N3On = NBG3 Enable */
	#define STV_VDP2_R0ON ((STV_VDP2_BGON & 0x0010) >> 4) /* R0On = RBG0 Enable */
	#define STV_VDP2_R1ON ((STV_VDP2_BGON & 0x0020) >> 5) /* R1On = RBG1 Enable */

	// NxTPON - Transparency Pen Enable Registers
	#define STV_VDP2_N0TPON ((STV_VDP2_BGON & 0x0100) >> 8) /*  N0TPON = NBG0 Draw Transparent Pen (as solid) /or/ RBG1 Draw Transparent Pen */
	#define STV_VDP2_N1TPON ((STV_VDP2_BGON & 0x0200) >> 9) /*  N1TPON = NBG1 Draw Transparent Pen (as solid) /or/ EXBG Draw Transparent Pen */
	#define STV_VDP2_N2TPON ((STV_VDP2_BGON & 0x0400) >> 10)/*  N2TPON = NBG2 Draw Transparent Pen (as solid) */
	#define STV_VDP2_N3TPON ((STV_VDP2_BGON & 0x0800) >> 11)/*  N3TPON = NBG3 Draw Transparent Pen (as solid) */
	#define STV_VDP2_R0TPON ((STV_VDP2_BGON & 0x1000) >> 12)/*  R0TPON = RBG0 Draw Transparent Pen (as solid) */

/*
180022 - MZCTL - Mosaic Control
bit->  /----15----|----14----|----13----|----12----|----11----|----10----|----09----|----08----\
       |    --    |    --    |    --    |    --    |    --    |    --    |    --    |    --    |
       |----07----|----06----|----05----|----04----|----03----|----02----|----01----|----00----|
       |    --    |    --    |    --    |    --    |    --    |    --    |    --    |    --    |
       \----------|----------|----------|----------|----------|----------|----------|---------*/

	#define STV_VDP2_MZCTL ((stv_vdp2_regs[0x020/4] >> 0)&0x0000ffff)

	#define STV_VDP2_R0MZE ((STV_VDP2_MZCTL & 0x0010) >> 4)
	#define STV_VDP2_N3MZE ((STV_VDP2_MZCTL & 0x0008) >> 3)
	#define STV_VDP2_N2MZE ((STV_VDP2_MZCTL & 0x0004) >> 2)
	#define STV_VDP2_N1MZE ((STV_VDP2_MZCTL & 0x0002) >> 1)
	#define STV_VDP2_N0MZE ((STV_VDP2_MZCTL & 0x0001) >> 0)

/*180024 - Special Function Code Select

180026 - Special Function Code

180028 - CHCTLA - Character Control (NBG0, NBG1)
 bit-> /----15----|----14----|----13----|----12----|----11----|----10----|----09----|----08----\
       |    --    |    --    | N1CHCN1  | N1CHCN0  | N1BMSZ1  | N1BMSZ0  | N1BMEN   | N1CHSZ   |
       |----07----|----06----|----05----|----04----|----03----|----02----|----01----|----00----|
       |    --    | N0CHCN2  | N0CHCN1  | N0CHCN0  | N0BMSZ1  | N0BMSZ0  | N0BMEN   | N0CHSZ   |
       \----------|----------|----------|----------|----------|----------|----------|---------*/

	#define STV_VDP2_CHCTLA ((stv_vdp2_regs[0x028/4] >> 16)&0x0000ffff)

/* -------------------------- NBG0 Character Control Registers -------------------------- */

/*  N0CHCNx  NBG0 (or RGB1) Colour Depth
    000 - 16 Colours
    001 - 256 Colours
    010 - 2048 Colours
    011 - 32768 Colours (RGB5)
    100 - 16770000 Colours (RGB8)
    101 - invalid
    110 - invalid
    111 - invalid   */
	#define STV_VDP2_N0CHCN ((STV_VDP2_CHCTLA & 0x0070) >> 4)

/*  N0BMSZx - NBG0 Bitmap Size *guessed*
    00 - 512 x 256
    01 - 512 x 512
    10 - 1024 x 256
    11 - 1024 x 512   */
	#define STV_VDP2_N0BMSZ ((STV_VDP2_CHCTLA & 0x000c) >> 2)

/*  N0BMEN - NBG0 Bitmap Enable
    0 - use cell mode
    1 - use bitmap mode   */
	#define STV_VDP2_N0BMEN ((STV_VDP2_CHCTLA & 0x0002) >> 1)

/*  N0CHSZ - NBG0 Character (Tile) Size
    0 - 1 cell  x 1 cell  (8x8)
    1 - 2 cells x 2 cells (16x16)  */
	#define STV_VDP2_N0CHSZ ((STV_VDP2_CHCTLA & 0x0001) >> 0)

/* -------------------------- NBG1 Character Control Registers -------------------------- */

/*  N1CHCNx - NBG1 (or EXB1) Colour Depth
    00 - 16 Colours
    01 - 256 Colours
    10 - 2048 Colours
    11 - 32768 Colours (RGB5)  */
	#define STV_VDP2_N1CHCN ((STV_VDP2_CHCTLA & 0x3000) >> 12)

/*  N1BMSZx - NBG1 Bitmap Size *guessed*
    00 - 512 x 256
    01 - 512 x 512
    10 - 1024 x 256
    11 - 1024 x 512   */
	#define STV_VDP2_N1BMSZ ((STV_VDP2_CHCTLA & 0x0c00) >> 10)

/*  N1BMEN - NBG1 Bitmap Enable
    0 - use cell mode
    1 - use bitmap mode   */
	#define STV_VDP2_N1BMEN ((STV_VDP2_CHCTLA & 0x0200) >> 9)

/*  N1CHSZ - NBG1 Character (Tile) Size
    0 - 1 cell  x 1 cell  (8x8)
    1 - 2 cells x 2 cells (16x16)  */
	#define STV_VDP2_N1CHSZ ((STV_VDP2_CHCTLA & 0x0100) >> 8)

/*
18002A - CHCTLB - Character Control (NBG2, NBG1, RBG0)
 bit-> /----15----|----14----|----13----|----12----|----11----|----10----|----09----|----08----\
       |    --    | R0CHCN2  | R0CHCN1  | R0CHCN0  |    --    | R0BMSZ   | R0BMEN   | R0CHSZ   |
       |----07----|----06----|----05----|----04----|----03----|----02----|----01----|----00----|
       |    --    |    --    | N3CHCN   | N3CHSZ   |    --    |    --    | N2CHCN   | N2CHSZ   |
       \----------|----------|----------|----------|----------|----------|----------|---------*/

	#define STV_VDP2_CHCTLB ((stv_vdp2_regs[0x028/4] >> 0)&0x0000ffff)

/* -------------------------- RBG0 Character Control Registers -------------------------- */


/*  R0CHCNx  RBG0  Colour Depth
    000 - 16 Colours
    001 - 256 Colours
    010 - 2048 Colours
    011 - 32768 Colours (RGB5)
    100 - 16770000 Colours (RGB8)
    101 - invalid
    110 - invalid
    111 - invalid   */
	#define STV_VDP2_R0CHCN ((STV_VDP2_CHCTLB & 0x7000) >> 12)

/*  R0BMSZx - RBG0 Bitmap Size *guessed*
    00 - 512 x 256
    01 - 512 x 512  */
	#define STV_VDP2_R0BMSZ ((STV_VDP2_CHCTLB & 0x0400) >> 10)

/*  R0BMEN - RBG0 Bitmap Enable
    0 - use cell mode
    1 - use bitmap mode   */
	#define STV_VDP2_R0BMEN ((STV_VDP2_CHCTLB & 0x0200) >> 9)

/*  R0CHSZ - NBG0 Character (Tile) Size
    0 - 1 cell  x 1 cell  (8x8)
    1 - 2 cells x 2 cells (16x16)  */
	#define STV_VDP2_R0CHSZ ((STV_VDP2_CHCTLB & 0x0100) >> 8)

	#define STV_VDP2_N3CHCN ((STV_VDP2_CHCTLB & 0x0020) >> 5)
	#define STV_VDP2_N3CHSZ ((STV_VDP2_CHCTLB & 0x0010) >> 4)
	#define STV_VDP2_N2CHCN ((STV_VDP2_CHCTLB & 0x0002) >> 1)
	#define STV_VDP2_N2CHSZ ((STV_VDP2_CHCTLB & 0x0001) >> 0)


/*
18002C - BMPNA - Bitmap Palette Number (NBG0, NBG1)
 bit-> /----15----|----14----|----13----|----12----|----11----|----10----|----09----|----08----\
       |    --    |    --    |    --    |    --    |    --    |    --    |    --    |    --    |
       |----07----|----06----|----05----|----04----|----03----|----02----|----01----|----00----|
       |    --    |    --    |    --    |    --    |    --    |    --    |    --    |    --    |
       \----------|----------|----------|----------|----------|----------|----------|---------*/

	#define STV_VDP2_BMPNA ((stv_vdp2_regs[0x02c/4] >> 16)&0x0000ffff)

	#define STV_VDP2_N1BMP ((STV_VDP2_BMPNA & 0x0700) >> 8)
	#define STV_VDP2_N0BMP ((STV_VDP2_BMPNA & 0x0007) >> 0)

/* 18002E - Bitmap Palette Number (RBG0)
 bit-> /----15----|----14----|----13----|----12----|----11----|----10----|----09----|----08----\
       |    --    |    --    |    --    |    --    |    --    |    --    |    --    |    --    |
       |----07----|----06----|----05----|----04----|----03----|----02----|----01----|----00----|
       |    --    |    --    |    --    |    --    |    --    |    --    |    --    |    --    |
       \----------|----------|----------|----------|----------|----------|----------|---------*/

	#define STV_VDP2_BMPNB ((stv_vdp2_regs[0x02c/4] >> 16)&0x0000ffff)

	#define STV_VDP2_R0BMP ((STV_VDP2_BMPNB & 0x0007) >> 0)

/* 180030 - PNCN0 - Pattern Name Control (NBG0)
 bit-> /----15----|----14----|----13----|----12----|----11----|----10----|----09----|----08----\
       | N0PNB    | N0CNSM   |    --    |    --    |    --    |    --    | N0SPR    | N0SCC    |
       |----07----|----06----|----05----|----04----|----03----|----02----|----01----|----00----|
       | N0SPLT6  | N0SPLT5  | N0SPLT4  | N0SPCN4  | N0SPCN3  | N0SPCN2  | N0SPCN1  | N0SPCN0  |
       \----------|----------|----------|----------|----------|----------|----------|---------*/

	#define STV_VDP2_PNCN0 ((stv_vdp2_regs[0x030/4] >> 16)&0x0000ffff)

/*  Pattern Data Size
    0 = 2 bytes
    1 = 1 byte */
	#define STV_VDP2_N0PNB  ((STV_VDP2_PNCN0 & 0x8000) >> 15)

/*  Character Number Supplement (in 1 byte mode)
    0 = Character Number = 10bits + 2bits for flip
    1 = Character Number = 12 bits, no flip  */
	#define STV_VDP2_N0CNSM ((STV_VDP2_PNCN0 & 0x4000) >> 14)

/*  NBG0 Special Priority Register (in 1 byte mode) */
	#define STV_VDP2_N0SPR ((STV_VDP2_PNCN0 & 0x0200) >> 9)

/*  NBG0 Special Colour Control Register (in 1 byte mode) */
	#define STV_VDP2_N0SCC ((STV_VDP2_PNCN0 & 0x0100) >> 8)

/*  Supplementary Palette Bits (in 1 byte mode) */
	#define STV_VDP2_N0SPLT ((STV_VDP2_PNCN0 & 0x00e0) >> 5)

/*  Supplementary Character Bits (in 1 byte mode) */
	#define STV_VDP2_N0SPCN ((STV_VDP2_PNCN0 & 0x001f) >> 0)

/* 180032 - Pattern Name Control (NBG1)
 bit-> /----15----|----14----|----13----|----12----|----11----|----10----|----09----|----08----\
       |    --    |    --    |    --    |    --    |    --    |    --    |    --    |    --    |
       |----07----|----06----|----05----|----04----|----03----|----02----|----01----|----00----|
       |    --    |    --    |    --    |    --    |    --    |    --    |    --    |    --    |
       \----------|----------|----------|----------|----------|----------|----------|---------*/

	#define STV_VDP2_PNCN1 ((stv_vdp2_regs[0x030/4] >> 0)&0x0000ffff)

/*  Pattern Data Size
    0 = 2 bytes
    1 = 1 byte */
	#define STV_VDP2_N1PNB  ((STV_VDP2_PNCN1 & 0x8000) >> 15)

/*  Character Number Supplement (in 1 byte mode)
    0 = Character Number = 10bits + 2bits for flip
    1 = Character Number = 12 bits, no flip  */
	#define STV_VDP2_N1CNSM ((STV_VDP2_PNCN1 & 0x4000) >> 14)

/*  NBG0 Special Priority Register (in 1 byte mode) */
	#define STV_VDP2_N1SPR ((STV_VDP2_PNCN1 & 0x0200) >> 9)

/*  NBG0 Special Colour Control Register (in 1 byte mode) */
	#define STV_VDP2_N1SCC ((STV_VDP2_PNCN1 & 0x0100) >> 8)

/*  Supplementary Palette Bits (in 1 byte mode) */
	#define STV_VDP2_N1SPLT ((STV_VDP2_PNCN1 & 0x00e0) >> 5)

/*  Supplementary Character Bits (in 1 byte mode) */
	#define STV_VDP2_N1SPCN ((STV_VDP2_PNCN1 & 0x001f) >> 0)


/* 180034 - Pattern Name Control (NBG2)
 bit-> /----15----|----14----|----13----|----12----|----11----|----10----|----09----|----08----\
       |    --    |    --    |    --    |    --    |    --    |    --    |    --    |    --    |
       |----07----|----06----|----05----|----04----|----03----|----02----|----01----|----00----|
       |    --    |    --    |    --    |    --    |    --    |    --    |    --    |    --    |
       \----------|----------|----------|----------|----------|----------|----------|---------*/

	#define STV_VDP2_PNCN2 ((stv_vdp2_regs[0x034/4] >> 16)&0x0000ffff)

/*  Pattern Data Size
    0 = 2 bytes
    1 = 1 byte */
	#define STV_VDP2_N2PNB  ((STV_VDP2_PNCN2 & 0x8000) >> 15)

/*  Character Number Supplement (in 1 byte mode)
    0 = Character Number = 10bits + 2bits for flip
    1 = Character Number = 12 bits, no flip  */
	#define STV_VDP2_N2CNSM ((STV_VDP2_PNCN2 & 0x4000) >> 14)

/*  NBG0 Special Priority Register (in 1 byte mode) */
	#define STV_VDP2_N2SPR ((STV_VDP2_PNCN2 & 0x0200) >> 9)

/*  NBG0 Special Colour Control Register (in 1 byte mode) */
	#define STV_VDP2_N2SCC ((STV_VDP2_PNCN2 & 0x0100) >> 8)

/*  Supplementary Palette Bits (in 1 byte mode) */
	#define STV_VDP2_N2SPLT ((STV_VDP2_PNCN2 & 0x00e0) >> 5)

/*  Supplementary Character Bits (in 1 byte mode) */
	#define STV_VDP2_N2SPCN ((STV_VDP2_PNCN2 & 0x001f) >> 0)


/* 180036 - Pattern Name Control (NBG3)
 bit-> /----15----|----14----|----13----|----12----|----11----|----10----|----09----|----08----\
       | N3PNB    | N3CNSM   |    --    |    --    |    --    |    --    | N3SPR    | N3SCC    |
       |----07----|----06----|----05----|----04----|----03----|----02----|----01----|----00----|
       | N3SPLT6  | N3SPLT5  | N3SPLT4  | N3SPCN4  | N3SPCN3  | N3SPCN2  | N3SPCN1  | N3SPCN0  |
       \----------|----------|----------|----------|----------|----------|----------|---------*/

	#define STV_VDP2_PNCN3 ((stv_vdp2_regs[0x034/4] >> 0)&0x0000ffff)

/*  Pattern Data Size
    0 = 2 bytes
    1 = 1 byte */
	#define STV_VDP2_N3PNB  ((STV_VDP2_PNCN3 & 0x8000) >> 15)

/*  Character Number Supplement (in 1 byte mode)
    0 = Character Number = 10bits + 2bits for flip
    1 = Character Number = 12 bits, no flip  */
	#define STV_VDP2_N3CNSM ((STV_VDP2_PNCN3 & 0x4000) >> 14)

/*  NBG0 Special Priority Register (in 1 byte mode) */
	#define STV_VDP2_N3SPR ((STV_VDP2_PNCN3 & 0x0200) >> 9)

/*  NBG0 Special Colour Control Register (in 1 byte mode) */
	#define STV_VDP2_N3SCC ((STV_VDP2_PNCN3 & 0x0100) >> 8)

/*  Supplementary Palette Bits (in 1 byte mode) */
	#define STV_VDP2_N3SPLT ((STV_VDP2_PNCN3 & 0x00e0) >> 5)

/*  Supplementary Character Bits (in 1 byte mode) */
	#define STV_VDP2_N3SPCN ((STV_VDP2_PNCN3 & 0x001f) >> 0)


/* 180038 - Pattern Name Control (RBG0)
 bit-> /----15----|----14----|----13----|----12----|----11----|----10----|----09----|----08----\
       |    --    |    --    |    --    |    --    |    --    |    --    |    --    |    --    |
       |----07----|----06----|----05----|----04----|----03----|----02----|----01----|----00----|
       |    --    |    --    |    --    |    --    |    --    |    --    |    --    |    --    |
       \----------|----------|----------|----------|----------|----------|----------|---------*/

	#define STV_VDP2_PNCR ((stv_vdp2_regs[0x038/4] >> 16)&0x0000ffff)

/*  Pattern Data Size
    0 = 2 bytes
    1 = 1 byte */
	#define STV_VDP2_R0PNB  ((STV_VDP2_PNCR & 0x8000) >> 15)

/*  Character Number Supplement (in 1 byte mode)
    0 = Character Number = 10bits + 2bits for flip
    1 = Character Number = 12 bits, no flip  */
	#define STV_VDP2_R0CNSM ((STV_VDP2_PNCR & 0x4000) >> 14)

/*  NBG0 Special Priority Register (in 1 byte mode) */
	#define STV_VDP2_R0SPR ((STV_VDP2_PNCR & 0x0200) >> 9)

/*  NBG0 Special Colour Control Register (in 1 byte mode) */
	#define STV_VDP2_R0SCC ((STV_VDP2_PNCR & 0x0100) >> 8)

/*  Supplementary Palette Bits (in 1 byte mode) */
	#define STV_VDP2_R0SPLT ((STV_VDP2_PNCR & 0x00e0) >> 5)

/*  Supplementary Character Bits (in 1 byte mode) */
	#define STV_VDP2_R0SPCN ((STV_VDP2_PNCR & 0x001f) >> 0)

/* 18003A - PLSZ - Plane Size (incomplete)
 bit-> /----15----|----14----|----13----|----12----|----11----|----10----|----09----|----08----\
       |    --    |    --    |    --    |    --    |    --    |    --    |    --    |    --    |
       |----07----|----06----|----05----|----04----|----03----|----02----|----01----|----00----|
       | N3PLSZ1  | N3PLSZ0  |    --    |    --    | N1PLSZ1  | N1PLSZ0  | N0PLSZ1  | N0PLSZ0  |
       \----------|----------|----------|----------|----------|----------|----------|---------*/

	#define STV_VDP2_PLSZ ((stv_vdp2_regs[0x038/4] >> 0)&0x0000ffff)

	/* NBG0 Plane Size
    00 1H Page x 1V Page
    01 2H Pages x 1V Page
    10 invalid
    11 2H Pages x 2V Pages  */
	#define STV_VDP2_RBOVR	((STV_VDP2_PLSZ & 0xc000) >> 14)
	#define STV_VDP2_RAOVR	((STV_VDP2_PLSZ & 0x0c00) >> 10)
	#define STV_VDP2_N0PLSZ ((STV_VDP2_PLSZ & 0x0003) >> 0)
	#define STV_VDP2_N1PLSZ ((STV_VDP2_PLSZ & 0x000c) >> 2)
	#define STV_VDP2_N2PLSZ ((STV_VDP2_PLSZ & 0x0030) >> 4)
	#define STV_VDP2_N3PLSZ ((STV_VDP2_PLSZ & 0x00c0) >> 6)
	#define STV_VDP2_RAPLSZ ((STV_VDP2_PLSZ & 0x0300) >> 8)
	#define STV_VDP2_RBPLSZ ((STV_VDP2_PLSZ & 0x3000) >> 12)

/* 18003C - MPOFN - Map Offset (NBG0, NBG1, NBG2, NBG3)
 bit-> /----15----|----14----|----13----|----12----|----11----|----10----|----09----|----08----\
       |    --    | N3MP8    | N3MP7    | N3MP6    |    --    | N2MP8    | N2MP7    | N2MP6    |
       |----07----|----06----|----05----|----04----|----03----|----02----|----01----|----00----|
       |    --    | N1MP8    | N1MP7    | N1MP6    |    --    | N0MP8    | N0MP7    | N0MP6    |
       \----------|----------|----------|----------|----------|----------|----------|---------*/

	#define STV_VDP2_MPOFN_ ((stv_vdp2_regs[0x03c/4] >> 16)&0x0000ffff)

	/* Higher 3 bits of the map offset for each layer */
	#define STV_VDP2_N0MP_ ((STV_VDP2_MPOFN_ & 0x0007) >> 0)
	#define STV_VDP2_N1MP_ ((STV_VDP2_MPOFN_ & 0x0070) >> 4)
	#define STV_VDP2_N2MP_ ((STV_VDP2_MPOFN_ & 0x0700) >> 8)
	#define STV_VDP2_N3MP_ ((STV_VDP2_MPOFN_ & 0x7000) >> 12)




/* 18003E - Map Offset (Rotation Parameter A,B)
 bit-> /----15----|----14----|----13----|----12----|----11----|----10----|----09----|----08----\
       |    --    |    --    |    --    |    --    |    --    |    --    |    --    |    --    |
       |----07----|----06----|----05----|----04----|----03----|----02----|----01----|----00----|
       |    --    |    --    |    --    |    --    |    --    |    --    |    --    |    --    |
       \----------|----------|----------|----------|----------|----------|----------|---------*/

	#define STV_VDP2_MPOFR_ ((stv_vdp2_regs[0x03c/4] >> 0)&0x0000ffff)

	#define STV_VDP2_RAMP_ ((STV_VDP2_MPOFR_ & 0x0007) >> 0)
	#define STV_VDP2_RBMP_ ((STV_VDP2_MPOFR_ & 0x0070) >> 4)

/* 180040 - MPABN0 - Map (NBG0, Plane A,B)
 bit-> /----15----|----14----|----13----|----12----|----11----|----10----|----09----|----08----\
       |    --    |    --    | N0MPB5   | N0MPB4   | N0MPB3   | N0MPB2   | N0MPB1   | N0MPB0   |
       |----07----|----06----|----05----|----04----|----03----|----02----|----01----|----00----|
       |    --    |    --    | N0MPA5   | N0MPA4   | N0MPA3   | N0MPA2   | N0MPA1   | N0MPA0   |
       \----------|----------|----------|----------|----------|----------|----------|---------*/

	#define STV_VDP2_MPABN0 ((stv_vdp2_regs[0x040/4] >> 16)&0x0000ffff)

	/* N0MPB5 = lower 6 bits of Map Address of Plane B of Tilemap NBG0 */
	#define STV_VDP2_N0MPB ((STV_VDP2_MPABN0 & 0x3f00) >> 8)

	/* N0MPA5 = lower 6 bits of Map Address of Plane A of Tilemap NBG0 */
	#define STV_VDP2_N0MPA ((STV_VDP2_MPABN0 & 0x003f) >> 0)


/* 180042 - MPCDN0 - (NBG0, Plane C,D)
 bit-> /----15----|----14----|----13----|----12----|----11----|----10----|----09----|----08----\
       |    --    |    --    | N0MPD5   | N0MPD4   | N0MPD3   | N0MPD2   | N0MPD1   | N0MPD0   |
       |----07----|----06----|----05----|----04----|----03----|----02----|----01----|----00----|
       |    --    |    --    | N0MPC5   | N0MPC4   | N0MPC3   | N0MPC2   | N0MPC1   | N0MPC0   |
       \----------|----------|----------|----------|----------|----------|----------|---------*/

	#define STV_VDP2_MPCDN0 ((stv_vdp2_regs[0x040/4] >> 0)&0x0000ffff)

	/* N0MPB5 = lower 6 bits of Map Address of Plane D of Tilemap NBG0 */
	#define STV_VDP2_N0MPD ((STV_VDP2_MPCDN0 & 0x3f00) >> 8)

	/* N0MPA5 = lower 6 bits of Map Address of Plane C of Tilemap NBG0 */
	#define STV_VDP2_N0MPC ((STV_VDP2_MPCDN0 & 0x003f) >> 0)


/* 180044 - Map (NBG1, Plane A,B)
 bit-> /----15----|----14----|----13----|----12----|----11----|----10----|----09----|----08----\
       |    --    |    --    |    --    |    --    |    --    |    --    |    --    |    --    |
       |----07----|----06----|----05----|----04----|----03----|----02----|----01----|----00----|
       |    --    |    --    |    --    |    --    |    --    |    --    |    --    |    --    |
       \----------|----------|----------|----------|----------|----------|----------|---------*/

	#define STV_VDP2_MPABN1 ((stv_vdp2_regs[0x044/4] >> 16)&0x0000ffff)

	/* N0MPB5 = lower 6 bits of Map Address of Plane B of Tilemap NBG1 */
	#define STV_VDP2_N1MPB ((STV_VDP2_MPABN1 & 0x3f00) >> 8)

	/* N0MPA5 = lower 6 bits of Map Address of Plane A of Tilemap NBG1 */
	#define STV_VDP2_N1MPA ((STV_VDP2_MPABN1 & 0x003f) >> 0)

/* 180046 - Map (NBG1, Plane C,D)
 bit-> /----15----|----14----|----13----|----12----|----11----|----10----|----09----|----08----\
       |    --    |    --    |    --    |    --    |    --    |    --    |    --    |    --    |
       |----07----|----06----|----05----|----04----|----03----|----02----|----01----|----00----|
       |    --    |    --    |    --    |    --    |    --    |    --    |    --    |    --    |
       \----------|----------|----------|----------|----------|----------|----------|---------*/

	#define STV_VDP2_MPCDN1 ((stv_vdp2_regs[0x044/4] >> 0)&0x0000ffff)

	/* N0MPB5 = lower 6 bits of Map Address of Plane D of Tilemap NBG0 */
	#define STV_VDP2_N1MPD ((STV_VDP2_MPCDN1 & 0x3f00) >> 8)

	/* N0MPA5 = lower 6 bits of Map Address of Plane C of Tilemap NBG0 */
	#define STV_VDP2_N1MPC ((STV_VDP2_MPCDN1 & 0x003f) >> 0)


/* 180048 - Map (NBG2, Plane A,B)
 bit-> /----15----|----14----|----13----|----12----|----11----|----10----|----09----|----08----\
       |    --    |    --    |    --    |    --    |    --    |    --    |    --    |    --    |
       |----07----|----06----|----05----|----04----|----03----|----02----|----01----|----00----|
       |    --    |    --    |    --    |    --    |    --    |    --    |    --    |    --    |
       \----------|----------|----------|----------|----------|----------|----------|---------*/

	#define STV_VDP2_MPABN2 ((stv_vdp2_regs[0x048/4] >> 16)&0x0000ffff)

	/* N0MPB5 = lower 6 bits of Map Address of Plane B of Tilemap NBG2 */
	#define STV_VDP2_N2MPB ((STV_VDP2_MPABN2 & 0x3f00) >> 8)

	/* N0MPA5 = lower 6 bits of Map Address of Plane A of Tilemap NBG2 */
	#define STV_VDP2_N2MPA ((STV_VDP2_MPABN2 & 0x003f) >> 0)

/* 18004a - Map (NBG2, Plane C,D)
 bit-> /----15----|----14----|----13----|----12----|----11----|----10----|----09----|----08----\
       |    --    |    --    |    --    |    --    |    --    |    --    |    --    |    --    |
       |----07----|----06----|----05----|----04----|----03----|----02----|----01----|----00----|
       |    --    |    --    |    --    |    --    |    --    |    --    |    --    |    --    |
       \----------|----------|----------|----------|----------|----------|----------|---------*/

	#define STV_VDP2_MPCDN2 ((stv_vdp2_regs[0x048/4] >> 0)&0x0000ffff)

	/* N0MPB5 = lower 6 bits of Map Address of Plane D of Tilemap NBG2 */
	#define STV_VDP2_N2MPD ((STV_VDP2_MPCDN2 & 0x3f00) >> 8)

	/* N0MPA5 = lower 6 bits of Map Address of Plane C of Tilemap NBG2 */
	#define STV_VDP2_N2MPC ((STV_VDP2_MPCDN2 & 0x003f) >> 0)

/* 18004c - Map (NBG3, Plane A,B)
 bit-> /----15----|----14----|----13----|----12----|----11----|----10----|----09----|----08----\
       |    --    |    --    |    --    |    --    |    --    |    --    |    --    |    --    |
       |----07----|----06----|----05----|----04----|----03----|----02----|----01----|----00----|
       |    --    |    --    |    --    |    --    |    --    |    --    |    --    |    --    |
       \----------|----------|----------|----------|----------|----------|----------|---------*/

	#define STV_VDP2_MPABN3 ((stv_vdp2_regs[0x04c/4] >> 16)&0x0000ffff)

	/* N0MPB5 = lower 6 bits of Map Address of Plane B of Tilemap NBG1 */
	#define STV_VDP2_N3MPB ((STV_VDP2_MPABN3 & 0x3f00) >> 8)

	/* N0MPA5 = lower 6 bits of Map Address of Plane A of Tilemap NBG1 */
	#define STV_VDP2_N3MPA ((STV_VDP2_MPABN3 & 0x003f) >> 0)


/* 18004e - Map (NBG3, Plane C,D)
 bit-> /----15----|----14----|----13----|----12----|----11----|----10----|----09----|----08----\
       |    --    |    --    |    --    |    --    |    --    |    --    |    --    |    --    |
       |----07----|----06----|----05----|----04----|----03----|----02----|----01----|----00----|
       |    --    |    --    |    --    |    --    |    --    |    --    |    --    |    --    |
       \----------|----------|----------|----------|----------|----------|----------|---------*/

	#define STV_VDP2_MPCDN3 ((stv_vdp2_regs[0x04c/4] >> 0)&0x0000ffff)

	/* N0MPB5 = lower 6 bits of Map Address of Plane B of Tilemap NBG0 */
	#define STV_VDP2_N3MPD ((STV_VDP2_MPCDN3 & 0x3f00) >> 8)

	/* N0MPA5 = lower 6 bits of Map Address of Plane A of Tilemap NBG0 */
	#define STV_VDP2_N3MPC ((STV_VDP2_MPCDN3 & 0x003f) >> 0)

/* 180050 - Map (Rotation Parameter A, Plane A,B)
 bit-> /----15----|----14----|----13----|----12----|----11----|----10----|----09----|----08----\
       |    --    |    --    |    --    |    --    |    --    |    --    |    --    |    --    |
       |----07----|----06----|----05----|----04----|----03----|----02----|----01----|----00----|
       |    --    |    --    |    --    |    --    |    --    |    --    |    --    |    --    |
       \----------|----------|----------|----------|----------|----------|----------|---------*/

	#define STV_VDP2_MPABRA ((stv_vdp2_regs[0x050/4] >> 16)&0x0000ffff)

	/* R0MPB5 = lower 6 bits of Map Address of Plane B of Tilemap RBG0 */
	#define STV_VDP2_RAMPB ((STV_VDP2_MPABRA & 0x3f00) >> 8)

	/* R0MPA5 = lower 6 bits of Map Address of Plane A of Tilemap RBG0 */
	#define STV_VDP2_RAMPA ((STV_VDP2_MPABRA & 0x003f) >> 0)



/* 180052 - Map (Rotation Parameter A, Plane C,D)
 bit-> /----15----|----14----|----13----|----12----|----11----|----10----|----09----|----08----\
       |    --    |    --    |    --    |    --    |    --    |    --    |    --    |    --    |
       |----07----|----06----|----05----|----04----|----03----|----02----|----01----|----00----|
       |    --    |    --    |    --    |    --    |    --    |    --    |    --    |    --    |
       \----------|----------|----------|----------|----------|----------|----------|---------*/
	#define STV_VDP2_MPCDRA ((stv_vdp2_regs[0x050/4] >> 0)&0x0000ffff)

	/* R0MPB5 = lower 6 bits of Map Address of Plane D of Tilemap RBG0 */
	#define STV_VDP2_RAMPD ((STV_VDP2_MPCDRA & 0x3f00) >> 8)

	/* R0MPA5 = lower 6 bits of Map Address of Plane C of Tilemap RBG0 */
	#define STV_VDP2_RAMPC ((STV_VDP2_MPCDRA & 0x003f) >> 0)

/* 180054 - Map (Rotation Parameter A, Plane E,F)
 bit-> /----15----|----14----|----13----|----12----|----11----|----10----|----09----|----08----\
       |    --    |    --    |    --    |    --    |    --    |    --    |    --    |    --    |
       |----07----|----06----|----05----|----04----|----03----|----02----|----01----|----00----|
       |    --    |    --    |    --    |    --    |    --    |    --    |    --    |    --    |
       \----------|----------|----------|----------|----------|----------|----------|---------*/
	#define STV_VDP2_MPEFRA ((stv_vdp2_regs[0x054/4] >> 16)&0x0000ffff)

	/* R0MPB5 = lower 6 bits of Map Address of Plane F of Tilemap RBG0 */
	#define STV_VDP2_RAMPF ((STV_VDP2_MPEFRA & 0x3f00) >> 8)

	/* R0MPA5 = lower 6 bits of Map Address of Plane E of Tilemap RBG0 */
	#define STV_VDP2_RAMPE ((STV_VDP2_MPEFRA & 0x003f) >> 0)

/* 180056 - Map (Rotation Parameter A, Plane G,H)
 bit-> /----15----|----14----|----13----|----12----|----11----|----10----|----09----|----08----\
       |    --    |    --    |    --    |    --    |    --    |    --    |    --    |    --    |
       |----07----|----06----|----05----|----04----|----03----|----02----|----01----|----00----|
       |    --    |    --    |    --    |    --    |    --    |    --    |    --    |    --    |
       \----------|----------|----------|----------|----------|----------|----------|---------*/
	#define STV_VDP2_MPGHRA ((stv_vdp2_regs[0x054/4] >> 0)&0x0000ffff)

	/* R0MPB5 = lower 6 bits of Map Address of Plane H of Tilemap RBG0 */
	#define STV_VDP2_RAMPH ((STV_VDP2_MPGHRA & 0x3f00) >> 8)

	/* R0MPA5 = lower 6 bits of Map Address of Plane G of Tilemap RBG0 */
	#define STV_VDP2_RAMPG ((STV_VDP2_MPGHRA & 0x003f) >> 0)

/* 180058 - Map (Rotation Parameter A, Plane I,J)
 bit-> /----15----|----14----|----13----|----12----|----11----|----10----|----09----|----08----\
       |    --    |    --    |    --    |    --    |    --    |    --    |    --    |    --    |
       |----07----|----06----|----05----|----04----|----03----|----02----|----01----|----00----|
       |    --    |    --    |    --    |    --    |    --    |    --    |    --    |    --    |
       \----------|----------|----------|----------|----------|----------|----------|---------*/
	#define STV_VDP2_MPIJRA ((stv_vdp2_regs[0x058/4] >> 16)&0x0000ffff)

	/* R0MPB5 = lower 6 bits of Map Address of Plane J of Tilemap RBG0 */
	#define STV_VDP2_RAMPJ ((STV_VDP2_MPIJRA & 0x3f00) >> 8)

	/* R0MPA5 = lower 6 bits of Map Address of Plane I of Tilemap RBG0 */
	#define STV_VDP2_RAMPI ((STV_VDP2_MPIJRA & 0x003f) >> 0)

/* 18005a - Map (Rotation Parameter A, Plane K,L)
 bit-> /----15----|----14----|----13----|----12----|----11----|----10----|----09----|----08----\
       |    --    |    --    |    --    |    --    |    --    |    --    |    --    |    --    |
       |----07----|----06----|----05----|----04----|----03----|----02----|----01----|----00----|
       |    --    |    --    |    --    |    --    |    --    |    --    |    --    |    --    |
       \----------|----------|----------|----------|----------|----------|----------|---------*/
	#define STV_VDP2_MPKLRA ((stv_vdp2_regs[0x058/4] >> 0)&0x0000ffff)

	/* R0MPB5 = lower 6 bits of Map Address of Plane L of Tilemap RBG0 */
	#define STV_VDP2_RAMPL ((STV_VDP2_MPKLRA & 0x3f00) >> 8)

	/* R0MPA5 = lower 6 bits of Map Address of Plane K of Tilemap RBG0 */
	#define STV_VDP2_RAMPK ((STV_VDP2_MPKLRA & 0x003f) >> 0)

/* 18005c - Map (Rotation Parameter A, Plane M,N)
 bit-> /----15----|----14----|----13----|----12----|----11----|----10----|----09----|----08----\
       |    --    |    --    |    --    |    --    |    --    |    --    |    --    |    --    |
       |----07----|----06----|----05----|----04----|----03----|----02----|----01----|----00----|
       |    --    |    --    |    --    |    --    |    --    |    --    |    --    |    --    |
       \----------|----------|----------|----------|----------|----------|----------|---------*/
	#define STV_VDP2_MPMNRA ((stv_vdp2_regs[0x05c/4] >> 16)&0x0000ffff)

	/* R0MPB5 = lower 6 bits of Map Address of Plane N of Tilemap RBG0 */
	#define STV_VDP2_RAMPN ((STV_VDP2_MPMNRA & 0x3f00) >> 8)

	/* R0MPA5 = lower 6 bits of Map Address of Plane M of Tilemap RBG0 */
	#define STV_VDP2_RAMPM ((STV_VDP2_MPMNRA & 0x003f) >> 0)

/* 18005e - Map (Rotation Parameter A, Plane O,P)
 bit-> /----15----|----14----|----13----|----12----|----11----|----10----|----09----|----08----\
       |    --    |    --    |    --    |    --    |    --    |    --    |    --    |    --    |
       |----07----|----06----|----05----|----04----|----03----|----02----|----01----|----00----|
       |    --    |    --    |    --    |    --    |    --    |    --    |    --    |    --    |
       \----------|----------|----------|----------|----------|----------|----------|---------*/
	#define STV_VDP2_MPOPRA ((stv_vdp2_regs[0x05c/4] >> 0)&0x0000ffff)

	/* R0MPB5 = lower 6 bits of Map Address of Plane P of Tilemap RBG0 */
	#define STV_VDP2_RAMPP ((STV_VDP2_MPOPRA & 0x3f00) >> 8)

	/* R0MPA5 = lower 6 bits of Map Address of Plane O of Tilemap RBG0 */
	#define STV_VDP2_RAMPO ((STV_VDP2_MPOPRA & 0x003f) >> 0)

/* 180060 - Map (Rotation Parameter B, Plane A,B)
 bit-> /----15----|----14----|----13----|----12----|----11----|----10----|----09----|----08----\
       |    --    |    --    |    --    |    --    |    --    |    --    |    --    |    --    |
       |----07----|----06----|----05----|----04----|----03----|----02----|----01----|----00----|
       |    --    |    --    |    --    |    --    |    --    |    --    |    --    |    --    |
       \----------|----------|----------|----------|----------|----------|----------|---------*/

	#define STV_VDP2_MPABRB ((stv_vdp2_regs[0x060/4] >> 16)&0x0000ffff)

	/* R0MPB5 = lower 6 bits of Map Address of Plane B of Tilemap RBG0 */
	#define STV_VDP2_RBMPB ((STV_VDP2_MPABRB & 0x3f00) >> 8)

	/* R0MPA5 = lower 6 bits of Map Address of Plane A of Tilemap RBG0 */
	#define STV_VDP2_RBMPA ((STV_VDP2_MPABRB & 0x003f) >> 0)


/* 180062 - Map (Rotation Parameter B, Plane C,D)
 bit-> /----15----|----14----|----13----|----12----|----11----|----10----|----09----|----08----\
       |    --    |    --    |    --    |    --    |    --    |    --    |    --    |    --    |
       |----07----|----06----|----05----|----04----|----03----|----02----|----01----|----00----|
       |    --    |    --    |    --    |    --    |    --    |    --    |    --    |    --    |
       \----------|----------|----------|----------|----------|----------|----------|---------*/

	#define STV_VDP2_MPCDRB ((stv_vdp2_regs[0x060/4] >> 0)&0x0000ffff)

	/* R0MPD5 = lower 6 bits of Map Address of Plane D of Tilemap RBG0 */
	#define STV_VDP2_RBMPD ((STV_VDP2_MPCDRB & 0x3f00) >> 8)

	/* R0MPc5 = lower 6 bits of Map Address of Plane C of Tilemap RBG0 */
	#define STV_VDP2_RBMPC ((STV_VDP2_MPCDRB & 0x003f) >> 0)

/* 180064 - Map (Rotation Parameter B, Plane E,F)
 bit-> /----15----|----14----|----13----|----12----|----11----|----10----|----09----|----08----\
       |    --    |    --    |    --    |    --    |    --    |    --    |    --    |    --    |
       |----07----|----06----|----05----|----04----|----03----|----02----|----01----|----00----|
       |    --    |    --    |    --    |    --    |    --    |    --    |    --    |    --    |
       \----------|----------|----------|----------|----------|----------|----------|---------*/

	#define STV_VDP2_MPEFRB ((stv_vdp2_regs[0x064/4] >> 16)&0x0000ffff)

	/* R0MPF5 = lower 6 bits of Map Address of Plane F of Tilemap RBG0 */
	#define STV_VDP2_RBMPF ((STV_VDP2_MPEFRB & 0x3f00) >> 8)

	/* R0MPE5 = lower 6 bits of Map Address of Plane E of Tilemap RBG0 */
	#define STV_VDP2_RBMPE ((STV_VDP2_MPEFRB & 0x003f) >> 0)

/* 180066 - Map (Rotation Parameter B, Plane G,H)
 bit-> /----15----|----14----|----13----|----12----|----11----|----10----|----09----|----08----\
       |    --    |    --    |    --    |    --    |    --    |    --    |    --    |    --    |
       |----07----|----06----|----05----|----04----|----03----|----02----|----01----|----00----|
       |    --    |    --    |    --    |    --    |    --    |    --    |    --    |    --    |
       \----------|----------|----------|----------|----------|----------|----------|---------*/

	#define STV_VDP2_MPGHRB ((stv_vdp2_regs[0x064/4] >> 0)&0x0000ffff)

	/* R0MPH5 = lower 6 bits of Map Address of Plane H of Tilemap RBG0 */
	#define STV_VDP2_RBMPH ((STV_VDP2_MPGHRB & 0x3f00) >> 8)

	/* R0MPG5 = lower 6 bits of Map Address of Plane G of Tilemap RBG0 */
	#define STV_VDP2_RBMPG ((STV_VDP2_MPGHRB & 0x003f) >> 0)

/* 180068 - Map (Rotation Parameter B, Plane I,J)
 bit-> /----15----|----14----|----13----|----12----|----11----|----10----|----09----|----08----\
       |    --    |    --    |    --    |    --    |    --    |    --    |    --    |    --    |
       |----07----|----06----|----05----|----04----|----03----|----02----|----01----|----00----|
       |    --    |    --    |    --    |    --    |    --    |    --    |    --    |    --    |
       \----------|----------|----------|----------|----------|----------|----------|---------*/

	#define STV_VDP2_MPIJRB ((stv_vdp2_regs[0x068/4] >> 16)&0x0000ffff)

	/* R0MPJ5 = lower 6 bits of Map Address of Plane J of Tilemap RBG0 */
	#define STV_VDP2_RBMPJ ((STV_VDP2_MPIJRB & 0x3f00) >> 8)

	/* R0MPI5 = lower 6 bits of Map Address of Plane E of Tilemap RBG0 */
	#define STV_VDP2_RBMPI ((STV_VDP2_MPIJRB & 0x003f) >> 0)

/* 18006a - Map (Rotation Parameter B, Plane K,L)
 bit-> /----15----|----14----|----13----|----12----|----11----|----10----|----09----|----08----\
       |    --    |    --    |    --    |    --    |    --    |    --    |    --    |    --    |
       |----07----|----06----|----05----|----04----|----03----|----02----|----01----|----00----|
       |    --    |    --    |    --    |    --    |    --    |    --    |    --    |    --    |
       \----------|----------|----------|----------|----------|----------|----------|---------*/

	#define STV_VDP2_MPKLRB ((stv_vdp2_regs[0x068/4] >> 0)&0x0000ffff)

	/* R0MPL5 = lower 6 bits of Map Address of Plane L of Tilemap RBG0 */
	#define STV_VDP2_RBMPL ((STV_VDP2_MPKLRB & 0x3f00) >> 8)

	/* R0MPK5 = lower 6 bits of Map Address of Plane K of Tilemap RBG0 */
	#define STV_VDP2_RBMPK ((STV_VDP2_MPKLRB & 0x003f) >> 0)

/* 18006c - Map (Rotation Parameter B, Plane M,N)
 bit-> /----15----|----14----|----13----|----12----|----11----|----10----|----09----|----08----\
       |    --    |    --    |    --    |    --    |    --    |    --    |    --    |    --    |
       |----07----|----06----|----05----|----04----|----03----|----02----|----01----|----00----|
       |    --    |    --    |    --    |    --    |    --    |    --    |    --    |    --    |
       \----------|----------|----------|----------|----------|----------|----------|---------*/

	#define STV_VDP2_MPMNRB ((stv_vdp2_regs[0x06c/4] >> 16)&0x0000ffff)

	/* R0MPN5 = lower 6 bits of Map Address of Plane N of Tilemap RBG0 */
	#define STV_VDP2_RBMPN ((STV_VDP2_MPMNRB & 0x3f00) >> 8)

	/* R0MPM5 = lower 6 bits of Map Address of Plane M of Tilemap RBG0 */
	#define STV_VDP2_RBMPM ((STV_VDP2_MPMNRB & 0x003f) >> 0)

/* 18006e - Map (Rotation Parameter B, Plane O,P)
 bit-> /----15----|----14----|----13----|----12----|----11----|----10----|----09----|----08----\
       |    --    |    --    |    --    |    --    |    --    |    --    |    --    |    --    |
       |----07----|----06----|----05----|----04----|----03----|----02----|----01----|----00----|
       |    --    |    --    |    --    |    --    |    --    |    --    |    --    |    --    |
       \----------|----------|----------|----------|----------|----------|----------|---------*/

	#define STV_VDP2_MPOPRB ((stv_vdp2_regs[0x06c/4] >> 0)&0x0000ffff)

	/* R0MPP5 = lower 6 bits of Map Address of Plane P of Tilemap RBG0 */
	#define STV_VDP2_RBMPP ((STV_VDP2_MPOPRB & 0x3f00) >> 8)

	/* R0MPO5 = lower 6 bits of Map Address of Plane O of Tilemap RBG0 */
	#define STV_VDP2_RBMPO ((STV_VDP2_MPOPRB & 0x003f) >> 0)

/* 180070 - SCXIN0 - Screen Scroll (NBG0, Horizontal Integer Part)
 bit-> /----15----|----14----|----13----|----12----|----11----|----10----|----09----|----08----\
       |    --    |    --    |    --    |    --    |    --    |    --    |    --    |    --    |
       |----07----|----06----|----05----|----04----|----03----|----02----|----01----|----00----|
       |    --    |    --    |    --    |    --    |    --    |    --    |    --    |    --    |
       \----------|----------|----------|----------|----------|----------|----------|---------*/

	#define STV_VDP2_SCXIN0 ((stv_vdp2_regs[0x070/4] >> 16)&0x0000ffff)


/* 180072 - Screen Scroll (NBG0, Horizontal Fractional Part)
 bit-> /----15----|----14----|----13----|----12----|----11----|----10----|----09----|----08----\
       |    --    |    --    |    --    |    --    |    --    |    --    |    --    |    --    |
       |----07----|----06----|----05----|----04----|----03----|----02----|----01----|----00----|
       |    --    |    --    |    --    |    --    |    --    |    --    |    --    |    --    |
       \----------|----------|----------|----------|----------|----------|----------|---------*/

/* 180074 - SCYIN0 - Screen Scroll (NBG0, Vertical Integer Part)
 bit-> /----15----|----14----|----13----|----12----|----11----|----10----|----09----|----08----\
       |    --    |    --    |    --    |    --    |    --    |    --    |    --    |    --    |
       |----07----|----06----|----05----|----04----|----03----|----02----|----01----|----00----|
       |    --    |    --    |    --    |    --    |    --    |    --    |    --    |    --    |
       \----------|----------|----------|----------|----------|----------|----------|---------*/
	#define STV_VDP2_SCYIN0 ((stv_vdp2_regs[0x074/4] >> 16)&0x0000ffff)


/* 180076 - Screen Scroll (NBG0, Vertical Fractional Part)
 bit-> /----15----|----14----|----13----|----12----|----11----|----10----|----09----|----08----\
       |    --    |    --    |    --    |    --    |    --    |    --    |    --    |    --    |
       |----07----|----06----|----05----|----04----|----03----|----02----|----01----|----00----|
       |    --    |    --    |    --    |    --    |    --    |    --    |    --    |    --    |
       \----------|----------|----------|----------|----------|----------|----------|---------*/

/* 180078 - Coordinate Inc (NBG0, Horizontal Integer Part)
 bit-> /----15----|----14----|----13----|----12----|----11----|----10----|----09----|----08----\
       |    --    |    --    |    --    |    --    |    --    |    --    |    --    |    --    |
       |----07----|----06----|----05----|----04----|----03----|----02----|----01----|----00----|
       |    --    |    --    |    --    |    --    |    --    |    --    |    --    |    --    |
       \----------|----------|----------|----------|----------|----------|----------|---------*/

	#define STV_VDP2_ZMXIN0 ((stv_vdp2_regs[0x078/4] >> 16)&0x0000ffff)
	#define STV_VDP2_ZMXN0	(stv_vdp2_regs[0x078/4] & 0x007ff00)

	#define STV_VDP2_N0ZMXI ((STV_VDP2_ZMXIN0 & 0x0007) >> 0)

/* 18007a - Coordinate Inc (NBG0, Horizontal Fractional Part)
 bit-> /----15----|----14----|----13----|----12----|----11----|----10----|----09----|----08----\
       |    --    |    --    |    --    |    --    |    --    |    --    |    --    |    --    |
       |----07----|----06----|----05----|----04----|----03----|----02----|----01----|----00----|
       |    --    |    --    |    --    |    --    |    --    |    --    |    --    |    --    |
       \----------|----------|----------|----------|----------|----------|----------|---------*/

	#define STV_VDP2_ZMXDN0 ((stv_vdp2_regs[0x078/4] >> 0)&0x0000ffff)

	#define STV_VDP2_N0ZMXD ((STV_VDP2_ZMXDN0 >> 8)& 0xff)

/* 18007c - Coordinate Inc (NBG0, Vertical Integer Part)
 bit-> /----15----|----14----|----13----|----12----|----11----|----10----|----09----|----08----\
       |    --    |    --    |    --    |    --    |    --    |    --    |    --    |    --    |
       |----07----|----06----|----05----|----04----|----03----|----02----|----01----|----00----|
       |    --    |    --    |    --    |    --    |    --    |    --    |    --    |    --    |
       \----------|----------|----------|----------|----------|----------|----------|---------*/

	#define STV_VDP2_ZMYIN0 ((stv_vdp2_regs[0x07c/4] >> 16)&0x0000ffff)
	#define STV_VDP2_ZMYN0	(stv_vdp2_regs[0x07c/4] & 0x007ff00)

	#define STV_VDP2_N0ZMYI ((STV_VDP2_ZMYIN0 & 0x0007) >> 0)

/* 18007e - Coordinate Inc (NBG0, Vertical Fractional Part)
 bit-> /----15----|----14----|----13----|----12----|----11----|----10----|----09----|----08----\
       |    --    |    --    |    --    |    --    |    --    |    --    |    --    |    --    |
       |----07----|----06----|----05----|----04----|----03----|----02----|----01----|----00----|
       |    --    |    --    |    --    |    --    |    --    |    --    |    --    |    --    |
       \----------|----------|----------|----------|----------|----------|----------|---------*/

	#define STV_VDP2_ZMYDN0 ((stv_vdp2_regs[0x07c/4] >> 0)&0x0000ffff)

	#define STV_VDP2_N0ZMYD ((STV_VDP2_ZMYDN0 >> 8)& 0xff)

/* 180080 - SCXIN1 - Screen Scroll (NBG1, Horizontal Integer Part)
 bit-> /----15----|----14----|----13----|----12----|----11----|----10----|----09----|----08----\
       |    --    |    --    |    --    |    --    |    --    |    --    |    --    |    --    |
       |----07----|----06----|----05----|----04----|----03----|----02----|----01----|----00----|
       |    --    |    --    |    --    |    --    |    --    |    --    |    --    |    --    |
       \----------|----------|----------|----------|----------|----------|----------|---------*/

	#define STV_VDP2_SCXIN1 ((stv_vdp2_regs[0x080/4] >> 16)&0x0000ffff)

/* 180082 - Screen Scroll (NBG1, Horizontal Fractional Part)
 bit-> /----15----|----14----|----13----|----12----|----11----|----10----|----09----|----08----\
       |    --    |    --    |    --    |    --    |    --    |    --    |    --    |    --    |
       |----07----|----06----|----05----|----04----|----03----|----02----|----01----|----00----|
       |    --    |    --    |    --    |    --    |    --    |    --    |    --    |    --    |
       \----------|----------|----------|----------|----------|----------|----------|---------*/

/* 180084 - SCYIN1 - Screen Scroll (NBG1, Vertical Integer Part)
 bit-> /----15----|----14----|----13----|----12----|----11----|----10----|----09----|----08----\
       |    --    |    --    |    --    |    --    |    --    |    --    |    --    |    --    |
       |----07----|----06----|----05----|----04----|----03----|----02----|----01----|----00----|
       |    --    |    --    |    --    |    --    |    --    |    --    |    --    |    --    |
       \----------|----------|----------|----------|----------|----------|----------|---------*/

	#define STV_VDP2_SCYIN1 ((stv_vdp2_regs[0x084/4] >> 16)&0x0000ffff)

/* 180086 - Screen Scroll (NBG1, Vertical Fractional Part)
 bit-> /----15----|----14----|----13----|----12----|----11----|----10----|----09----|----08----\
       |    --    |    --    |    --    |    --    |    --    |    --    |    --    |    --    |
       |----07----|----06----|----05----|----04----|----03----|----02----|----01----|----00----|
       |    --    |    --    |    --    |    --    |    --    |    --    |    --    |    --    |
       \----------|----------|----------|----------|----------|----------|----------|---------*/

/* 180088 - Coordinate Inc (NBG1, Horizontal Integer Part)
 bit-> /----15----|----14----|----13----|----12----|----11----|----10----|----09----|----08----\
       |    --    |    --    |    --    |    --    |    --    |    --    |    --    |    --    |
       |----07----|----06----|----05----|----04----|----03----|----02----|----01----|----00----|
       |    --    |    --    |    --    |    --    |    --    |    --    |    --    |    --    |
       \----------|----------|----------|----------|----------|----------|----------|---------*/

	#define STV_VDP2_ZMXIN1 ((stv_vdp2_regs[0x088/4] >> 16)&0x0000ffff)
	#define STV_VDP2_ZMXN1	(stv_vdp2_regs[0x088/4] & 0x007ff00)

	#define STV_VDP2_N1ZMXI ((STV_VDP2_ZMXIN1 & 0x0007) >> 0)

/* 18008a - Coordinate Inc (NBG1, Horizontal Fractional Part)
 bit-> /----15----|----14----|----13----|----12----|----11----|----10----|----09----|----08----\
       |    --    |    --    |    --    |    --    |    --    |    --    |    --    |    --    |
       |----07----|----06----|----05----|----04----|----03----|----02----|----01----|----00----|
       |    --    |    --    |    --    |    --    |    --    |    --    |    --    |    --    |
       \----------|----------|----------|----------|----------|----------|----------|---------*/

	#define STV_VDP2_ZMXDN1 ((stv_vdp2_regs[0x088/4] >> 0)&0x0000ffff)

	#define STV_VDP2_N1ZMXD ((STV_VDP2_ZMXDN1 >> 8)& 0xff)

/* 18008c - Coordinate Inc (NBG1, Vertical Integer Part)
 bit-> /----15----|----14----|----13----|----12----|----11----|----10----|----09----|----08----\
       |    --    |    --    |    --    |    --    |    --    |    --    |    --    |    --    |
       |----07----|----06----|----05----|----04----|----03----|----02----|----01----|----00----|
       |    --    |    --    |    --    |    --    |    --    |    --    |    --    |    --    |
       \----------|----------|----------|----------|----------|----------|----------|---------*/

	#define STV_VDP2_ZMYIN1 ((stv_vdp2_regs[0x08c/4] >> 16)&0x0000ffff)
	#define STV_VDP2_ZMYN1	(stv_vdp2_regs[0x08c/4] & 0x007ff00)

	#define STV_VDP2_N1ZMYI ((STV_VDP2_ZMYIN1 & 0x0007) >> 0)

/* 18008e - Coordinate Inc (NBG1, Vertical Fractional Part)
 bit-> /----15----|----14----|----13----|----12----|----11----|----10----|----09----|----08----\
       |    --    |    --    |    --    |    --    |    --    |    --    |    --    |    --    |
       |----07----|----06----|----05----|----04----|----03----|----02----|----01----|----00----|
       |    --    |    --    |    --    |    --    |    --    |    --    |    --    |    --    |
       \----------|----------|----------|----------|----------|----------|----------|---------*/

	#define STV_VDP2_ZMYDN1 ((stv_vdp2_regs[0x08c/4] >> 0)&0x0000ffff)

	#define STV_VDP2_N1ZMYD ((STV_VDP2_ZMYDN1 >> 8)& 0xff)

/* 180090 - SCXN2 - Screen Scroll (NBG2, Horizontal)
 bit-> /----15----|----14----|----13----|----12----|----11----|----10----|----09----|----08----\
       |    --    |    --    |    --    |    --    |    --    |    --    |    --    |    --    |
       |----07----|----06----|----05----|----04----|----03----|----02----|----01----|----00----|
       |    --    |    --    |    --    |    --    |    --    |    --    |    --    |    --    |
       \----------|----------|----------|----------|----------|----------|----------|---------*/

	#define STV_VDP2_SCXN2 ((stv_vdp2_regs[0x090/4] >> 16)&0x0000ffff)

/* 180092 - SCYN2 - Screen Scroll (NBG2, Vertical)
 bit-> /----15----|----14----|----13----|----12----|----11----|----10----|----09----|----08----\
       |    --    |    --    |    --    |    --    |    --    |    --    |    --    |    --    |
       |----07----|----06----|----05----|----04----|----03----|----02----|----01----|----00----|
       |    --    |    --    |    --    |    --    |    --    |    --    |    --    |    --    |
       \----------|----------|----------|----------|----------|----------|----------|---------*/

	#define STV_VDP2_SCYN2 ((stv_vdp2_regs[0x090/4] >> 0)&0x0000ffff)

/* 180094 - SCXN3 - Screen Scroll (NBG3, Horizontal)
 bit-> /----15----|----14----|----13----|----12----|----11----|----10----|----09----|----08----\
       |    --    |    --    |    --    |    --    |    --    |    --    |    --    |    --    |
       |----07----|----06----|----05----|----04----|----03----|----02----|----01----|----00----|
       |    --    |    --    |    --    |    --    |    --    |    --    |    --    |    --    |
       \----------|----------|----------|----------|----------|----------|----------|---------*/

	#define STV_VDP2_SCXN3 ((stv_vdp2_regs[0x094/4] >> 16)&0x0000ffff)

/* 180096 - SCYN3 - Screen Scroll (NBG3, Vertical)
 bit-> /----15----|----14----|----13----|----12----|----11----|----10----|----09----|----08----\
       |    --    |    --    |    --    |    --    |    --    |    --    |    --    |    --    |
       |----07----|----06----|----05----|----04----|----03----|----02----|----01----|----00----|
       |    --    |    --    |    --    |    --    |    --    |    --    |    --    |    --    |
       \----------|----------|----------|----------|----------|----------|----------|---------*/

	#define STV_VDP2_SCYN3 ((stv_vdp2_regs[0x094/4] >> 0)&0x0000ffff)

/* 180098 - Reduction Enable
 bit-> /----15----|----14----|----13----|----12----|----11----|----10----|----09----|----08----\
       |    --    |    --    |    --    |    --    |    --    |    --    | N1ZMQT   | N1ZMHF   |
       |----07----|----06----|----05----|----04----|----03----|----02----|----01----|----00----|
       |    --    |    --    |    --    |    --    |    --    |    --    | N0ZMQT   | N0ZMHF   |
       \----------|----------|----------|----------|----------|----------|----------|---------*/

	#define STV_VDP2_ZMCTL ((stv_vdp2_regs[0x098/4] >> 16)&0x0000ffff)

	#define STV_VDP2_N1ZMQT  ((STV_VDP2_ZMCTL & 0x0200) >> 9)
	#define STV_VDP2_N1ZMHF  ((STV_VDP2_ZMCTL & 0x0100) >> 8)
	#define STV_VDP2_N0ZMQT  ((STV_VDP2_ZMCTL & 0x0002) >> 1)
	#define STV_VDP2_N0ZMHF  ((STV_VDP2_ZMCTL & 0x0001) >> 0)

/* 18009a - Line and Vertical Cell Scroll Control (NBG0, NBG1)
 bit-> /----15----|----14----|----13----|----12----|----11----|----10----|----09----|----08----\
       |    --    |    --    |    --    |    --    |    --    |    --    |    --    |    --    |
       |----07----|----06----|----05----|----04----|----03----|----02----|----01----|----00----|
       |    --    |    --    |    --    |    --    |    --    |    --    |    --    |    --    |
       \----------|----------|----------|----------|----------|----------|----------|---------*/

	#define STV_VDP2_SCRCTL ((stv_vdp2_regs[0x098/4] >> 0)&0x0000ffff)

	#define STV_VDP2_N1LSS  ((STV_VDP2_SCRCTL & 0x3000) >> 12)
	#define STV_VDP2_N1LZMX ((STV_VDP2_SCRCTL & 0x0800) >> 11)
	#define STV_VDP2_N1LSCY ((STV_VDP2_SCRCTL & 0x0400) >> 10)
	#define STV_VDP2_N1LSCX ((STV_VDP2_SCRCTL & 0x0200) >> 9)
	#define STV_VDP2_N1VCSC ((STV_VDP2_SCRCTL & 0x0100) >> 8)
	#define STV_VDP2_N0LSS  ((STV_VDP2_SCRCTL & 0x0030) >> 4)
	#define STV_VDP2_N0LZMX ((STV_VDP2_SCRCTL & 0x0008) >> 3)
	#define STV_VDP2_N0LSCY ((STV_VDP2_SCRCTL & 0x0004) >> 2)
	#define STV_VDP2_N0LSCX ((STV_VDP2_SCRCTL & 0x0002) >> 1)
	#define STV_VDP2_N0VCSC ((STV_VDP2_SCRCTL & 0x0001) >> 0)

/* 18009c - Vertical Cell Table Address (NBG0, NBG1)
 bit-> /----15----|----14----|----13----|----12----|----11----|----10----|----09----|----08----\
       |    --    |    --    |    --    |    --    |    --    |    --    |    --    |    --    |
       |----07----|----06----|----05----|----04----|----03----|----02----|----01----|----00----|
       |    --    |    --    |    --    |    --    |    --    |    --    |    --    |    --    |
       \----------|----------|----------|----------|----------|----------|----------|---------*/

/* 18009e - Vertical Cell Table Address (NBG0, NBG1)
 bit-> /----15----|----14----|----13----|----12----|----11----|----10----|----09----|----08----\
       |    --    |    --    |    --    |    --    |    --    |    --    |    --    |    --    |
       |----07----|----06----|----05----|----04----|----03----|----02----|----01----|----00----|
       |    --    |    --    |    --    |    --    |    --    |    --    |    --    |    --    |
       \----------|----------|----------|----------|----------|----------|----------|---------*/

/* 1800a0 - LSTA0U - Line Scroll Table Address (NBG0)
 bit-> /----15----|----14----|----13----|----12----|----11----|----10----|----09----|----08----\
       |    --    |    --    |    --    |    --    |    --    |    --    |    --    |    --    |
       |----07----|----06----|----05----|----04----|----03----|----02----|----01----|----00----|
       |    --    |    --    |    --    |    --    |    --    |    --    |    --    |    --    |
       \----------|----------|----------|----------|----------|----------|----------|---------*/

	/*bit 2 unused when VRAM = 4 Mbits*/
	#define STV_VDP2_LSTA0U ((stv_vdp2_regs[0x0a0/4] >> 16)&0x00000003)

/* 1800a2 - LSTA0L - Line Scroll Table Address (NBG0)
 bit-> /----15----|----14----|----13----|----12----|----11----|----10----|----09----|----08----\
       |    --    |    --    |    --    |    --    |    --    |    --    |    --    |    --    |
       |----07----|----06----|----05----|----04----|----03----|----02----|----01----|----00----|
       |    --    |    --    |    --    |    --    |    --    |    --    |    --    |    --    |
       \----------|----------|----------|----------|----------|----------|----------|---------*/

	#define STV_VDP2_LSTA0L ((stv_vdp2_regs[0x0a0/4] >> 0)&0x0000fffe)

/* 1800a4 - LSTA1U - Line Scroll Table Address (NBG1)
 bit-> /----15----|----14----|----13----|----12----|----11----|----10----|----09----|----08----\
       |    --    |    --    |    --    |    --    |    --    |    --    |    --    |    --    |
       |----07----|----06----|----05----|----04----|----03----|----02----|----01----|----00----|
       |    --    |    --    |    --    |    --    |    --    |    --    |    --    |    --    |
       \----------|----------|----------|----------|----------|----------|----------|---------*/

	/*bit 2 unused when VRAM = 4 Mbits*/
	#define STV_VDP2_LSTA1U ((stv_vdp2_regs[0x0a4/4] >> 16)&0x00000003)

/* 1800a6 - LSTA1L - Line Scroll Table Address (NBG1)
 bit-> /----15----|----14----|----13----|----12----|----11----|----10----|----09----|----08----\
       |    --    |    --    |    --    |    --    |    --    |    --    |    --    |    --    |
       |----07----|----06----|----05----|----04----|----03----|----02----|----01----|----00----|
       |    --    |    --    |    --    |    --    |    --    |    --    |    --    |    --    |
       \----------|----------|----------|----------|----------|----------|----------|---------*/

	#define STV_VDP2_LSTA1L ((stv_vdp2_regs[0x0a4/4] >> 0)&0x0000fffe)

/* 1800a8 - LCTAU - Line Colour Screen Table Address
 bit-> /----15----|----14----|----13----|----12----|----11----|----10----|----09----|----08----\
       |    --    |    --    |    --    |    --    |    --    |    --    |    --    |    --    |
       |----07----|----06----|----05----|----04----|----03----|----02----|----01----|----00----|
       |    --    |    --    |    --    |    --    |    --    |    --    |    --    |    --    |
       \----------|----------|----------|----------|----------|----------|----------|---------*/

	#define STV_VDP2_LCTAU	((stv_vdp2_regs[0x0a8/4] >> 16) & 0xffff)
	#define STV_VDP2_LCCLMD	((STV_VDP2_LCTAU & 0x8000) >> 15)

/* 1800aa - LCTAL - Line Colour Screen Table Address
 bit-> /----15----|----14----|----13----|----12----|----11----|----10----|----09----|----08----\
       |    --    |    --    |    --    |    --    |    --    |    --    |    --    |    --    |
       |----07----|----06----|----05----|----04----|----03----|----02----|----01----|----00----|
       |    --    |    --    |    --    |    --    |    --    |    --    |    --    |    --    |
       \----------|----------|----------|----------|----------|----------|----------|---------*/
	#define STV_VDP2_LCTAL	((stv_vdp2_regs[0x0a8/4] >> 0) & 0xffff)

	#define STV_VDP2_LCTA	(stv_vdp2_regs[0x0a8/4] & 0x0003ffff)

/* 1800ac - Back Screen Table Address
 bit-> /----15----|----14----|----13----|----12----|----11----|----10----|----09----|----08----\
       |  BKCLMD  |    --    |    --    |    --    |    --    |    --    |    --    |    --    |
       |----07----|----06----|----05----|----04----|----03----|----02----|----01----|----00----|
       |    --    |    --    |    --    |    --    |    --    |  BKTA18  |  BKTA17  |  BKTA16  |
       \----------|----------|----------|----------|----------|----------|----------|---------*/

/* 1800ae - Back Screen Table Address
 bit-> /----15----|----14----|----13----|----12----|----11----|----10----|----09----|----08----\
       |  BKTA15  |  BKTA14  |  BKTA13  |  BKTA12  |  BKTA11  |  BKTA10  |  BKTA9   |  BKTA8   |
       |----07----|----06----|----05----|----04----|----03----|----02----|----01----|----00----|
       |  BKTA7   |  BKTA7   |  BKTA6   |  BKTA5   |  BKTA4   |  BKTA3   |  BKTA2   |  BKTA0   |
       \----------|----------|----------|----------|----------|----------|----------|---------*/

	#define STV_VDP2_BKTA_UL (stv_vdp2_regs[0x0ac/4])

	#define STV_VDP2_BKCLMD ((STV_VDP2_BKTA_UL & 0x80000000) >> 31)
	#define STV_VDP2_BKTA   ((STV_VDP2_BKTA_UL & 0x0003ffff) >> 0)
	/*MSB of this register is used when the extra RAM cart is used,ignore it for now.*/
	//  #define STV_VDP2_BKTA   ((STV_VDP2_BKTA_UL & 0x0007ffff) >> 0)

/* 1800b0 - RPMD - Rotation Parameter Mode
 bit-> /----15----|----14----|----13----|----12----|----11----|----10----|----09----|----08----\
       |    --    |    --    |    --    |    --    |    --    |    --    |    --    |    --    |
       |----07----|----06----|----05----|----04----|----03----|----02----|----01----|----00----|
       |    --    |    --    |    --    |    --    |    --    |    --    |    --    |    --    |
       \----------|----------|----------|----------|----------|----------|----------|---------*/

	#define STV_VDP2_RPMD	((stv_vdp2_regs[0x0b0/4] >> 16) & 0x00000003)

/* 1800b2 - RPRCTL - Rotation Parameter Read Control
 bit-> /----15----|----14----|----13----|----12----|----11----|----10----|----09----|----08----\
       |    --    |    --    |    --    |    --    |    --    | RBKASTRE | RBYSTRE  | RBXSTRE  |
       |----07----|----06----|----05----|----04----|----03----|----02----|----01----|----00----|
       |    --    |    --    |    --    |    --    |    --    | RAKASTRE | RAYSTRE  | RBXSTRE  |
       \----------|----------|----------|----------|----------|----------|----------|---------*/

	#define STV_VDP2_RPRCTL		((stv_vdp2_regs[0x0b0/4] >> 0)&0x0000ffff)
	#define STV_VDP2_RBKASTRE	((STV_VDP2_RPRCTL & 0x0400) >> 10)
	#define STV_VDP2_RBYSTRE	((STV_VDP2_RPRCTL & 0x0200) >> 9)
	#define STV_VDP2_RBXSTRE	((STV_VDP2_RPRCTL & 0x0100) >> 8)
	#define STV_VDP2_RAKASTRE	((STV_VDP2_RPRCTL & 0x0004) >> 2)
	#define STV_VDP2_RAYSTRE	((STV_VDP2_RPRCTL & 0x0002) >> 1)
	#define STV_VDP2_RAXSTRE	((STV_VDP2_RPRCTL & 0x0001) >> 0)

/* 1800b4 - KTCTL - Coefficient Table Control
 bit-> /----15----|----14----|----13----|----12----|----11----|----10----|----09----|----08----\
       |    --    |    --    |    --    |  RBKLCE  |  RBKMD1  |  RBKMD0  |  RBKDBS  |   RBKTE  |
       |----07----|----06----|----05----|----04----|----03----|----02----|----01----|----00----|
       |    --    |    --    |    --    |  RAKLCE  |  RAKMD1  |  RAKMD0  |  RAKDBS  |   RAKTE  |
       \----------|----------|----------|----------|----------|----------|----------|---------*/

	#define	STV_VDP2_KTCTL	((stv_vdp2_regs[0x0b4/4] >> 16) & 0x0000ffff)
	#define STV_VDP2_RBKLCE	((STV_VDP2_KTCTL & 0x1000) >> 12)
	#define STV_VDP2_RBKMD	((STV_VDP2_KTCTL & 0x0c00) >> 10)
	#define STV_VDP2_RBKDBS	((STV_VDP2_KTCTL & 0x0200) >> 9)
	#define STV_VDP2_RBKTE	((STV_VDP2_KTCTL & 0x0100) >> 8)
	#define STV_VDP2_RAKLCE	((STV_VDP2_KTCTL & 0x0010) >> 4)
	#define STV_VDP2_RAKMD	((STV_VDP2_KTCTL & 0x000c) >> 2)
	#define STV_VDP2_RAKDBS	((STV_VDP2_KTCTL & 0x0002) >> 1)
	#define STV_VDP2_RAKTE	((STV_VDP2_KTCTL & 0x0001) >> 0)

/* 1800b6 - KTAOF - Coefficient Table Address Offset (Rotation Parameter A,B)
 bit-> /----15----|----14----|----13----|----12----|----11----|----10----|----09----|----08----\
       |    --    |    --    |    --    |    --    |    --    | RBKTAOS2 | RBKTAOS1 | RBKTAOS0 |
       |----07----|----06----|----05----|----04----|----03----|----02----|----01----|----00----|
       |    --    |    --    |    --    |    --    |    --    | RAKTAOS2 | RAKTAOS1 | RAKTAOS0 |
       \----------|----------|----------|----------|----------|----------|----------|---------*/

	#define STV_VDP2_KTAOF	((stv_vdp2_regs[0x0b4/4] >> 0) & 0x0000ffff)
	#define STV_VDP2_RBKTAOS ((STV_VDP2_KTAOF & 0x0700) >> 8)
	#define STV_VDP2_RAKTAOS ((STV_VDP2_KTAOF & 0x0007) >> 0)

/* 1800b8 - OVPNRA - Screen Over Pattern Name (Rotation Parameter A)
 bit-> /----15----|----14----|----13----|----12----|----11----|----10----|----09----|----08----\
       |    --    |    --    |    --    |    --    |    --    |    --    |    --    |    --    |
       |----07----|----06----|----05----|----04----|----03----|----02----|----01----|----00----|
       |    --    |    --    |    --    |    --    |    --    |    --    |    --    |    --    |
       \----------|----------|----------|----------|----------|----------|----------|---------*/

	#define STV_VDP2_OVPNRA	((stv_vdp2_regs[0x0b8/4] >> 16) & 0x0000ffff)

/* 1800ba - Screen Over Pattern Name (Rotation Parameter B)
 bit-> /----15----|----14----|----13----|----12----|----11----|----10----|----09----|----08----\
       |    --    |    --    |    --    |    --    |    --    |    --    |    --    |    --    |
       |----07----|----06----|----05----|----04----|----03----|----02----|----01----|----00----|
       |    --    |    --    |    --    |    --    |    --    |    --    |    --    |    --    |
       \----------|----------|----------|----------|----------|----------|----------|---------*/

	#define STV_VDP2_OVPNRB ((stv_vdp2_regs[0x0b8/4] >> 0) & 0x0000ffff)

/* 1800bc - RPTAU - Rotation Parameter Table Address (Rotation Parameter A,B)
 bit-> /----15----|----14----|----13----|----12----|----11----|----10----|----09----|----08----\
       |    --    |    --    |    --    |    --    |    --    |    --    |    --    |    --    |
       |----07----|----06----|----05----|----04----|----03----|----02----|----01----|----00----|
       |    --    |    --    |    --    |    --    |    --    |  RPTA18  |  RPTA17  |  RPTA16  |
       \----------|----------|----------|----------|----------|----------|----------|---------*/
	#define STV_VDP2_RPTAU	((stv_vdp2_regs[0x0bc/4] >> 16) & 0x00000007)

/* 1800be - RPTAL - Rotation Parameter Table Address (Rotation Parameter A,B)
 bit-> /----15----|----14----|----13----|----12----|----11----|----10----|----09----|----08----\
       |  RPTA15  |  RPTA14  |  RPTA13  |  RPTA12  |  RPTA11  |  RPTA10  |   RPTA9  |   RPTA8  |
       |----07----|----06----|----05----|----04----|----03----|----02----|----01----|----00----|
       |   RPTA7  |   RPTA6  |   RPTA5  |   RPTA4  |   RPTA3  |   RPTA2  |   RPTA1  |    --    |
       \----------|----------|----------|----------|----------|----------|----------|---------*/

	#define STV_VDP2_RPTAL	((stv_vdp2_regs[0x0bc/4] >> 0) & 0x0000fffe)

/* 1800c0 - Window Position (W0, Horizontal Start Point)
 bit-> /----15----|----14----|----13----|----12----|----11----|----10----|----09----|----08----\
       |    --    |    --    |    --    |    --    |    --    |    --    |    --    |    --    |
       |----07----|----06----|----05----|----04----|----03----|----02----|----01----|----00----|
       |    --    |    --    |    --    |    --    |    --    |    --    |    --    |    --    |
       \----------|----------|----------|----------|----------|----------|----------|---------*/

	#define STV_VDP2_WPSX0 ((stv_vdp2_regs[0x0c0/4] >> 16)&0x0000ffff)

	#define STV_VDP2_W0SX ((STV_VDP2_WPSX0 & 0x03ff) >> 0)

/* 1800c2 - Window Position (W0, Vertical Start Point)
 bit-> /----15----|----14----|----13----|----12----|----11----|----10----|----09----|----08----\
       |    --    |    --    |    --    |    --    |    --    |    --    |    --    |    --    |
       |----07----|----06----|----05----|----04----|----03----|----02----|----01----|----00----|
       |    --    |    --    |    --    |    --    |    --    |    --    |    --    |    --    |
       \----------|----------|----------|----------|----------|----------|----------|---------*/

	#define STV_VDP2_WPSY0 ((stv_vdp2_regs[0x0c0/4] >> 0)&0x0000ffff)

	#define STV_VDP2_W0SY ((STV_VDP2_WPSY0 & 0x03ff) >> 0)

/* 1800c4 - Window Position (W0, Horizontal End Point)
 bit-> /----15----|----14----|----13----|----12----|----11----|----10----|----09----|----08----\
       |    --    |    --    |    --    |    --    |    --    |    --    |    --    |    --    |
       |----07----|----06----|----05----|----04----|----03----|----02----|----01----|----00----|
       |    --    |    --    |    --    |    --    |    --    |    --    |    --    |    --    |
       \----------|----------|----------|----------|----------|----------|----------|---------*/

	#define STV_VDP2_WPEX0 ((stv_vdp2_regs[0x0c4/4] >> 16)&0x0000ffff)

	#define STV_VDP2_W0EX ((STV_VDP2_WPEX0 & 0x03ff) >> 0)

/* 1800c6 - Window Position (W0, Vertical End Point)
 bit-> /----15----|----14----|----13----|----12----|----11----|----10----|----09----|----08----\
       |    --    |    --    |    --    |    --    |    --    |    --    |    --    |    --    |
       |----07----|----06----|----05----|----04----|----03----|----02----|----01----|----00----|
       |    --    |    --    |    --    |    --    |    --    |    --    |    --    |    --    |
       \----------|----------|----------|----------|----------|----------|----------|---------*/

	#define STV_VDP2_WPEY0 ((stv_vdp2_regs[0x0c4/4] >> 0)&0x0000ffff)

	#define STV_VDP2_W0EY ((STV_VDP2_WPEY0 & 0x03ff) >> 0)

/* 1800c8 - Window Position (W1, Horizontal Start Point)
 bit-> /----15----|----14----|----13----|----12----|----11----|----10----|----09----|----08----\
       |    --    |    --    |    --    |    --    |    --    |    --    |    --    |    --    |
       |----07----|----06----|----05----|----04----|----03----|----02----|----01----|----00----|
       |    --    |    --    |    --    |    --    |    --    |    --    |    --    |    --    |
       \----------|----------|----------|----------|----------|----------|----------|---------*/

	#define STV_VDP2_WPSX1 ((stv_vdp2_regs[0x0c8/4] >> 16)&0x0000ffff)

	#define STV_VDP2_W1SX ((STV_VDP2_WPSX1 & 0x03ff) >> 0)

/* 1800ca - Window Position (W1, Vertical Start Point)
 bit-> /----15----|----14----|----13----|----12----|----11----|----10----|----09----|----08----\
       |    --    |    --    |    --    |    --    |    --    |    --    |    --    |    --    |
       |----07----|----06----|----05----|----04----|----03----|----02----|----01----|----00----|
       |    --    |    --    |    --    |    --    |    --    |    --    |    --    |    --    |
       \----------|----------|----------|----------|----------|----------|----------|---------*/

	#define STV_VDP2_WPSY1 ((stv_vdp2_regs[0x0c8/4] >> 0)&0x0000ffff)

	#define STV_VDP2_W1SY ((STV_VDP2_WPSY1 & 0x03ff) >> 0)

/* 1800cc - Window Position (W1, Horizontal End Point)
 bit-> /----15----|----14----|----13----|----12----|----11----|----10----|----09----|----08----\
       |    --    |    --    |    --    |    --    |    --    |    --    |    --    |    --    |
       |----07----|----06----|----05----|----04----|----03----|----02----|----01----|----00----|
       |    --    |    --    |    --    |    --    |    --    |    --    |    --    |    --    |
       \----------|----------|----------|----------|----------|----------|----------|---------*/

	#define STV_VDP2_WPEX1 ((stv_vdp2_regs[0x0cc/4] >> 16)&0x0000ffff)

	#define STV_VDP2_W1EX ((STV_VDP2_WPEX1 & 0x03ff) >> 0)

/* 1800ce - Window Position (W1, Vertical End Point)
 bit-> /----15----|----14----|----13----|----12----|----11----|----10----|----09----|----08----\
       |    --    |    --    |    --    |    --    |    --    |    --    |    --    |    --    |
       |----07----|----06----|----05----|----04----|----03----|----02----|----01----|----00----|
       |    --    |    --    |    --    |    --    |    --    |    --    |    --    |    --    |
       \----------|----------|----------|----------|----------|----------|----------|---------*/

	#define STV_VDP2_WPEY1 ((stv_vdp2_regs[0x0cc/4] >> 0)&0x0000ffff)

	#define STV_VDP2_W1EY ((STV_VDP2_WPEY1 & 0x03ff) >> 0)

/* 1800d0 - Window Control (NBG0, NBG1)
 bit-> /----15----|----14----|----13----|----12----|----11----|----10----|----09----|----08----\
       |    --    |    --    |    --    |    --    |    --    |    --    |    --    |    --    |
       |----07----|----06----|----05----|----04----|----03----|----02----|----01----|----00----|
       |    --    |    --    |    --    |    --    |    --    |    --    |    --    |    --    |
       \----------|----------|----------|----------|----------|----------|----------|---------*/

	#define STV_VDP2_WCTLA ((stv_vdp2_regs[0x0d0/4] >> 16)&0x0000ffff)
	#define STV_VDP2_N1LOG ((STV_VDP2_WCTLA & 0x8000) >> 15)
	#define STV_VDP2_N1SWE ((STV_VDP2_WCTLA & 0x2000) >> 13)
	#define STV_VDP2_N1SWA ((STV_VDP2_WCTLA & 0x1000) >> 12)
	#define STV_VDP2_N1W1E ((STV_VDP2_WCTLA & 0x0800) >> 11)
	#define STV_VDP2_N1W1A ((STV_VDP2_WCTLA & 0x0400) >> 10)
	#define STV_VDP2_N1W0E ((STV_VDP2_WCTLA & 0x0200) >> 9)
	#define STV_VDP2_N1W0A ((STV_VDP2_WCTLA & 0x0100) >> 8)
	#define STV_VDP2_N0LOG ((STV_VDP2_WCTLA & 0x0080) >> 7)
	#define STV_VDP2_N0SWE ((STV_VDP2_WCTLA & 0x0020) >> 5)
	#define STV_VDP2_N0SWA ((STV_VDP2_WCTLA & 0x0010) >> 4)
	#define STV_VDP2_N0W1E ((STV_VDP2_WCTLA & 0x0008) >> 3)
	#define STV_VDP2_N0W1A ((STV_VDP2_WCTLA & 0x0004) >> 2)
	#define STV_VDP2_N0W0E ((STV_VDP2_WCTLA & 0x0002) >> 1)
	#define STV_VDP2_N0W0A ((STV_VDP2_WCTLA & 0x0001) >> 0)

/* 1800d2 - Window Control (NBG2, NBG3)
 bit-> /----15----|----14----|----13----|----12----|----11----|----10----|----09----|----08----\
       |    --    |    --    |    --    |    --    |    --    |    --    |    --    |    --    |
       |----07----|----06----|----05----|----04----|----03----|----02----|----01----|----00----|
       |    --    |    --    |    --    |    --    |    --    |    --    |    --    |    --    |
       \----------|----------|----------|----------|----------|----------|----------|---------*/

	#define STV_VDP2_WCTLB ((stv_vdp2_regs[0x0d0/4] >> 0)&0x0000ffff)
	#define STV_VDP2_N3LOG ((STV_VDP2_WCTLB & 0x8000) >> 15)
	#define STV_VDP2_N3SWE ((STV_VDP2_WCTLB & 0x2000) >> 13)
	#define STV_VDP2_N3SWA ((STV_VDP2_WCTLB & 0x1000) >> 12)
	#define STV_VDP2_N3W1E ((STV_VDP2_WCTLB & 0x0800) >> 11)
	#define STV_VDP2_N3W1A ((STV_VDP2_WCTLB & 0x0400) >> 10)
	#define STV_VDP2_N3W0E ((STV_VDP2_WCTLB & 0x0200) >> 9)
	#define STV_VDP2_N3W0A ((STV_VDP2_WCTLB & 0x0100) >> 8)
	#define STV_VDP2_N2LOG ((STV_VDP2_WCTLB & 0x0080) >> 7)
	#define STV_VDP2_N2SWE ((STV_VDP2_WCTLB & 0x0020) >> 5)
	#define STV_VDP2_N2SWA ((STV_VDP2_WCTLB & 0x0010) >> 4)
	#define STV_VDP2_N2W1E ((STV_VDP2_WCTLB & 0x0008) >> 3)
	#define STV_VDP2_N2W1A ((STV_VDP2_WCTLB & 0x0004) >> 2)
	#define STV_VDP2_N2W0E ((STV_VDP2_WCTLB & 0x0002) >> 1)
	#define STV_VDP2_N2W0A ((STV_VDP2_WCTLB & 0x0001) >> 0)

/* 1800d4 - Window Control (RBG0, Sprite)
 bit-> /----15----|----14----|----13----|----12----|----11----|----10----|----09----|----08----\
       |    --    |    --    |    --    |    --    |    --    |    --    |    --    |    --    |
       |----07----|----06----|----05----|----04----|----03----|----02----|----01----|----00----|
       |    --    |    --    |    --    |    --    |    --    |    --    |    --    |    --    |
       \----------|----------|----------|----------|----------|----------|----------|---------*/

	#define STV_VDP2_WCTLC ((stv_vdp2_regs[0x0d4/4] >> 16)&0x0000ffff)
	#define STV_VDP2_SPLOG ((STV_VDP2_WCTLC & 0x8000) >> 15)
	#define STV_VDP2_SPSWE ((STV_VDP2_WCTLC & 0x2000) >> 13)
	#define STV_VDP2_SPSWA ((STV_VDP2_WCTLC & 0x1000) >> 12)
	#define STV_VDP2_SPW1E ((STV_VDP2_WCTLC & 0x0800) >> 11)
	#define STV_VDP2_SPW1A ((STV_VDP2_WCTLC & 0x0400) >> 10)
	#define STV_VDP2_SPW0E ((STV_VDP2_WCTLC & 0x0200) >> 9)
	#define STV_VDP2_SPW0A ((STV_VDP2_WCTLC & 0x0100) >> 8)
	#define STV_VDP2_R0LOG ((STV_VDP2_WCTLC & 0x0080) >> 7)
	#define STV_VDP2_R0SWE ((STV_VDP2_WCTLC & 0x0020) >> 5)
	#define STV_VDP2_R0SWA ((STV_VDP2_WCTLC & 0x0010) >> 4)
	#define STV_VDP2_R0W1E ((STV_VDP2_WCTLC & 0x0008) >> 3)
	#define STV_VDP2_R0W1A ((STV_VDP2_WCTLC & 0x0004) >> 2)
	#define STV_VDP2_R0W0E ((STV_VDP2_WCTLC & 0x0002) >> 1)
	#define STV_VDP2_R0W0A ((STV_VDP2_WCTLC & 0x0001) >> 0)

/* 1800d6 - Window Control (Parameter Window, Colour Calc. Window)
 bit-> /----15----|----14----|----13----|----12----|----11----|----10----|----09----|----08----\
       |    --    |    --    |    --    |    --    |    --    |    --    |    --    |    --    |
       |----07----|----06----|----05----|----04----|----03----|----02----|----01----|----00----|
       |    --    |    --    |    --    |    --    |    --    |    --    |    --    |    --    |
       \----------|----------|----------|----------|----------|----------|----------|---------*/

/* 1800d8 - Line Window Table Address (W0)
 bit-> /----15----|----14----|----13----|----12----|----11----|----10----|----09----|----08----\
       |    --    |    --    |    --    |    --    |    --    |    --    |    --    |    --    |
       |----07----|----06----|----05----|----04----|----03----|----02----|----01----|----00----|
       |    --    |    --    |    --    |    --    |    --    |    --    |    --    |    --    |
       \----------|----------|----------|----------|----------|----------|----------|---------*/

/* 1800da - Line Window Table Address (W0)
 bit-> /----15----|----14----|----13----|----12----|----11----|----10----|----09----|----08----\
       |    --    |    --    |    --    |    --    |    --    |    --    |    --    |    --    |
       |----07----|----06----|----05----|----04----|----03----|----02----|----01----|----00----|
       |    --    |    --    |    --    |    --    |    --    |    --    |    --    |    --    |
       \----------|----------|----------|----------|----------|----------|----------|---------*/

/* 1800dc - Line Window Table Address (W1)
 bit-> /----15----|----14----|----13----|----12----|----11----|----10----|----09----|----08----\
       |    --    |    --    |    --    |    --    |    --    |    --    |    --    |    --    |
       |----07----|----06----|----05----|----04----|----03----|----02----|----01----|----00----|
       |    --    |    --    |    --    |    --    |    --    |    --    |    --    |    --    |
       \----------|----------|----------|----------|----------|----------|----------|---------*/

/* 1800de - Line Window Table Address (W1)
 bit-> /----15----|----14----|----13----|----12----|----11----|----10----|----09----|----08----\
       |    --    |    --    |    --    |    --    |    --    |    --    |    --    |    --    |
       |----07----|----06----|----05----|----04----|----03----|----02----|----01----|----00----|
       |    --    |    --    |    --    |    --    |    --    |    --    |    --    |    --    |
       \----------|----------|----------|----------|----------|----------|----------|---------*/

/* 1800e0 - Sprite Control
 bit-> /----15----|----14----|----13----|----12----|----11----|----10----|----09----|----08----\
       |    --    |    --    | SPCCCS1  | SPCCCS0  |    --    |  SPCCN2  |  SPCCN1  |  SPCCN0  |
       |----07----|----06----|----05----|----04----|----03----|----02----|----01----|----00----|
       |    --    |    --    |  SPCLMD  | SPWINEN  |  SPTYPE3 |  SPTYPE2 |  SPTYPE1 |  SPTYPE0 |
       \----------|----------|----------|----------|----------|----------|----------|---------*/

	#define STV_VDP2_SPCTL		((stv_vdp2_regs[0xe0/4] >> 16)&0x0000ffff)
	#define STV_VDP2_SPCCCS		((STV_VDP2_SPCTL & 0x3000) >> 12)
	#define STV_VDP2_SPCCN		((STV_VDP2_SPCTL & 0x700) >> 8)
	#define STV_VDP2_SPCLMD		((STV_VDP2_SPCTL & 0x20) >> 5)
	#define STV_VDP2_SPWINEN	((STV_VDP2_SPCTL & 0x10) >> 4)
	#define STV_VDP2_SPTYPE		(STV_VDP2_SPCTL & 0xf)

/* 1800e2 - Shadow Control
 bit-> /----15----|----14----|----13----|----12----|----11----|----10----|----09----|----08----\
       |    --    |    --    |    --    |    --    |    --    |    --    |    --    |    --    |
       |----07----|----06----|----05----|----04----|----03----|----02----|----01----|----00----|
       |    --    |    --    |    --    |    --    |    --    |    --    |    --    |    --    |
       \----------|----------|----------|----------|----------|----------|----------|---------*/

	#define STV_VDP2_SDCTL		((stv_vdp2_regs[0x0e0/4] >> 0) & 0x0000ffff)

/* 1800e4 - CRAOFA - Colour Ram Address Offset (NBG0 - NBG3)
 bit-> /----15----|----14----|----13----|----12----|----11----|----10----|----09----|----08----\
       |    --    | N0CAOS2  | N3CAOS1  | N3CAOS0  |    --    | N2CAOS2  | N2CAOS1  | N2CAOS0  |
       |----07----|----06----|----05----|----04----|----03----|----02----|----01----|----00----|
       |    --    | N1CAOS2  | N1CAOS1  | N1CAOS0  |    --    | N0CAOS2  | N0CAOS1  | N0CAOS0  |
       \----------|----------|----------|----------|----------|----------|----------|---------*/

	#define STV_VDP2_CRAOFA ((stv_vdp2_regs[0x0e4/4] >> 16)&0x0000ffff)

	/* NxCAOS =  */
	#define STV_VDP2_N0CAOS ((STV_VDP2_CRAOFA & 0x0007) >> 0)
	#define STV_VDP2_N1CAOS ((STV_VDP2_CRAOFA & 0x0070) >> 4)
	#define STV_VDP2_N2CAOS ((STV_VDP2_CRAOFA & 0x0700) >> 8)
	#define STV_VDP2_N3CAOS ((STV_VDP2_CRAOFA & 0x7000) >> 12)


/* 1800e6 - Colour Ram Address Offset (RBG0, SPRITE)
 bit-> /----15----|----14----|----13----|----12----|----11----|----10----|----09----|----08----\
       |    --    |    --    |    --    |    --    |    --    |    --    |    --    |    --    |
       |----07----|----06----|----05----|----04----|----03----|----02----|----01----|----00----|
       |    --    |    --    |    --    |    --    |    --    |    --    |    --    |    --    |
       \----------|----------|----------|----------|----------|----------|----------|---------*/
	#define STV_VDP2_CRAOFB ((stv_vdp2_regs[0x0e4/4] >> 0)&0x0000ffff)
	#define STV_VDP2_R0CAOS ((STV_VDP2_CRAOFB & 0x0007) >> 0)
	#define STV_VDP2_SPCAOS ((STV_VDP2_CRAOFB & 0x0070) >> 4)

/* 1800e8 - LNCLEN - Line Colour Screen Enable
 bit-> /----15----|----14----|----13----|----12----|----11----|----10----|----09----|----08----\
       |    --    |    --    |    --    |    --    |    --    |    --    |    --    |    --    |
       |----07----|----06----|----05----|----04----|----03----|----02----|----01----|----00----|
       |    --    |    --    |  SPLCEN  |  R0LCEN  |  N3LCEN  |  N2LCEN  |  N1LCEN  | N0LCEN   |
       \----------|----------|----------|----------|----------|----------|----------|---------*/

	#define STV_VDP2_LNCLEN	((stv_vdp2_regs[0x0e8/4] >> 16)&0x0000ffff)
	#define STV_VDP2_SPLCEN	((STV_VDP2_LNCLEN) & 0x0020) >> 5)
	#define STV_VDP2_R0LCEN	((STV_VDP2_LNCLEN) & 0x0010) >> 4)
	#define STV_VDP2_N3LCEN	((STV_VDP2_LNCLEN) & 0x0008) >> 3)
	#define STV_VDP2_N2LCEN	((STV_VDP2_LNCLEN) & 0x0004) >> 2)
	#define STV_VDP2_N1LCEN	((STV_VDP2_LNCLEN) & 0x0002) >> 1)
	#define STV_VDP2_N0LCEN	((STV_VDP2_LNCLEN) & 0x0001) >> 0)

/* 1800ea - Special Priority Mode
 bit-> /----15----|----14----|----13----|----12----|----11----|----10----|----09----|----08----\
       |    --    |    --    |    --    |    --    |    --    |    --    |    --    |    --    |
       |----07----|----06----|----05----|----04----|----03----|----02----|----01----|----00----|
       |    --    |    --    |    --    |    --    |    --    |    --    |    --    |    --    |
       \----------|----------|----------|----------|----------|----------|----------|---------*/

/* 1800ec - Colour Calculation Control
 bit-> /----15----|----14----|----13----|----12----|----11----|----10----|----09----|----08----\
       |  BOKEN   |  BOKN2   |  BOKN1   |   BOKN0  |    --    |  EXCCEN  |  CCRTMD  |  CCMD    |
       |----07----|----06----|----05----|----04----|----03----|----02----|----01----|----00----|
       |    --    |  SPCCEN  |  LCCCEN  |  R0CCEN  |  N3CCEN  |  N2CCEN  |  N1CCEN  |  N0CCEN  |
       \----------|----------|----------|----------|----------|----------|----------|---------*/

	#define STV_VDP2_CCCR		((stv_vdp2_regs[0xec/4]>>16)&0x0000ffff)
	#define STV_VDP2_CCMD		((STV_VDP2_CCCR & 0x100) >> 8)
	#define STV_VDP2_SPCCEN		((STV_VDP2_CCCR & 0x40) >> 6)
	#define STV_VDP2_LCCCEN		((STV_VDP2_CCCR & 0x20) >> 5)
	#define STV_VDP2_R0CCEN		((STV_VDP2_CCCR & 0x10) >> 4)
	#define STV_VDP2_N3CCEN		((STV_VDP2_CCCR & 0x8) >> 3)
	#define STV_VDP2_N2CCEN		((STV_VDP2_CCCR & 0x4) >> 2)
	#define STV_VDP2_N1CCEN		((STV_VDP2_CCCR & 0x2) >> 1)
	#define STV_VDP2_N0CCEN		((STV_VDP2_CCCR & 0x1) >> 0)


/* 1800ee - Special Colour Calculation Mode
 bit-> /----15----|----14----|----13----|----12----|----11----|----10----|----09----|----08----\
       |    --    |    --    |    --    |    --    |    --    |    --    |    --    |    --    |
       |----07----|----06----|----05----|----04----|----03----|----02----|----01----|----00----|
       |    --    |    --    |    --    |    --    |    --    |    --    |    --    |    --    |
       \----------|----------|----------|----------|----------|----------|----------|---------*/

/* 1800f0 - Priority Number (Sprite 0,1)
 bit-> /----15----|----14----|----13----|----12----|----11----|----10----|----09----|----08----\
       |    --    |    --    |    --    |    --    |    --    |  S1PRIN2 |  S1PRIN1 |  S1PRIN0 |
       |----07----|----06----|----05----|----04----|----03----|----02----|----01----|----00----|
       |    --    |    --    |    --    |    --    |    --    |  S0PRIN2 |  S0PRIN1 |  S0PRIN0 |
       \----------|----------|----------|----------|----------|----------|----------|---------*/

	#define STV_VDP2_PRISA		((stv_vdp2_regs[0xf0/4] >> 16) & 0x0000ffff)
	#define STV_VDP2_S1PRIN		((STV_VDP2_PRISA & 0x0700) >> 8)
	#define STV_VDP2_S0PRIN		((STV_VDP2_PRISA & 0x0007) >> 0)

/* 1800f2 - Priority Number (Sprite 2,3)
 bit-> /----15----|----14----|----13----|----12----|----11----|----10----|----09----|----08----\
       |    --    |    --    |    --    |    --    |    --    |  S3PRIN2 |  S3PRIN1 |  S3PRIN0 |
       |----07----|----06----|----05----|----04----|----03----|----02----|----01----|----00----|
       |    --    |    --    |    --    |    --    |    --    |  S2PRIN2 |  S2PRIN1 |  S2PRIN0 |
       \----------|----------|----------|----------|----------|----------|----------|---------*/

	#define STV_VDP2_PRISB		((stv_vdp2_regs[0xf0/4] >> 0) & 0x0000ffff)
	#define STV_VDP2_S3PRIN		((STV_VDP2_PRISB & 0x0700) >> 8)
	#define STV_VDP2_S2PRIN		((STV_VDP2_PRISB & 0x0007) >> 0)

/* 1800f4 - Priority Number (Sprite 4,5)
 bit-> /----15----|----14----|----13----|----12----|----11----|----10----|----09----|----08----\
       |    --    |    --    |    --    |    --    |    --    |  S5PRIN2 |  S5PRIN1 |  S5PRIN0 |
       |----07----|----06----|----05----|----04----|----03----|----02----|----01----|----00----|
       |    --    |    --    |    --    |    --    |    --    |  S4PRIN2 |  S4PRIN1 |  S4PRIN0 |
       \----------|----------|----------|----------|----------|----------|----------|---------*/

	#define STV_VDP2_PRISC		((stv_vdp2_regs[0xf4/4] >> 16) & 0x0000ffff)
	#define STV_VDP2_S5PRIN		((STV_VDP2_PRISC & 0x0700) >> 8)
	#define STV_VDP2_S4PRIN		((STV_VDP2_PRISC & 0x0007) >> 0)

/* 1800f6 - Priority Number (Sprite 6,7)
 bit-> /----15----|----14----|----13----|----12----|----11----|----10----|----09----|----08----\
       |    --    |    --    |    --    |    --    |    --    |  S7PRIN2 |  S7PRIN1 |  S7PRIN0 |
       |----07----|----06----|----05----|----04----|----03----|----02----|----01----|----00----|
       |    --    |    --    |    --    |    --    |    --    |  S6PRIN2 |  S6PRIN1 |  S6PRIN0 |
       \----------|----------|----------|----------|----------|----------|----------|---------*/

	#define STV_VDP2_PRISD		((stv_vdp2_regs[0xf4/4] >> 0) & 0x0000ffff)
	#define STV_VDP2_S7PRIN		((STV_VDP2_PRISD & 0x0700) >> 8)
	#define STV_VDP2_S6PRIN		((STV_VDP2_PRISD & 0x0007) >> 0)


/* 1800f8 - PRINA - Priority Number (NBG 0,1)
 bit-> /----15----|----14----|----13----|----12----|----11----|----10----|----09----|----08----\
       |    --    |    --    |    --    |    --    |    --    |    --    |    --    |    --    |
       |----07----|----06----|----05----|----04----|----03----|----02----|----01----|----00----|
       |    --    |    --    |    --    |    --    |    --    |    --    |    --    |    --    |
       \----------|----------|----------|----------|----------|----------|----------|---------*/

	#define STV_VDP2_PRINA ((stv_vdp2_regs[0x0f8/4] >> 16)&0x0000ffff)

	#define STV_VDP2_N1PRIN ((STV_VDP2_PRINA & 0x0700) >> 8)
	#define STV_VDP2_N0PRIN ((STV_VDP2_PRINA & 0x0007) >> 0)

/* 1800fa - PRINB - Priority Number (NBG 2,3)
 bit-> /----15----|----14----|----13----|----12----|----11----|----10----|----09----|----08----\
       |    --    |    --    |    --    |    --    |    --    |    --    |    --    |    --    |
       |----07----|----06----|----05----|----04----|----03----|----02----|----01----|----00----|
       |    --    |    --    |    --    |    --    |    --    |    --    |    --    |    --    |
       \----------|----------|----------|----------|----------|----------|----------|---------*/

	#define STV_VDP2_PRINB ((stv_vdp2_regs[0x0f8/4] >> 0)&0x0000ffff)

	#define STV_VDP2_N3PRIN ((STV_VDP2_PRINB & 0x0700) >> 8)
	#define STV_VDP2_N2PRIN ((STV_VDP2_PRINB & 0x0007) >> 0)

/* 1800fc - Priority Number (RBG0)
 bit-> /----15----|----14----|----13----|----12----|----11----|----10----|----09----|----08----\
       |    --    |    --    |    --    |    --    |    --    |    --    |    --    |    --    |
       |----07----|----06----|----05----|----04----|----03----|----02----|----01----|----00----|
       |    --    |    --    |    --    |    --    |    --    |    --    |    --    |    --    |
       \----------|----------|----------|----------|----------|----------|----------|---------*/
	#define STV_VDP2_PRIR ((stv_vdp2_regs[0x0fc/4] >> 16)&0x0000ffff)

	#define STV_VDP2_R0PRIN ((STV_VDP2_PRIR & 0x0007) >> 0)

/* 1800fe - Reserved
 bit-> /----15----|----14----|----13----|----12----|----11----|----10----|----09----|----08----\
       |    --    |    --    |    --    |    --    |    --    |    --    |    --    |    --    |
       |----07----|----06----|----05----|----04----|----03----|----02----|----01----|----00----|
       |    --    |    --    |    --    |    --    |    --    |    --    |    --    |    --    |
       \----------|----------|----------|----------|----------|----------|----------|---------*/

/* 180100 - Colour Calculation Ratio (Sprite 0,1)
 bit-> /----15----|----14----|----13----|----12----|----11----|----10----|----09----|----08----\
       |    --    |    --    |    --    |  S1CCRT4 |  S1CCRT3 |  S1CCRT2 |  S1CCRT1 |  S1CCRT0 |
       |----07----|----06----|----05----|----04----|----03----|----02----|----01----|----00----|
       |    --    |    --    |    --    |  S0CCRT4 |  S0CCRT3 |  S0CCRT2 |  S0CCRT1 |  S0CCRT0 |
       \----------|----------|----------|----------|----------|----------|----------|---------*/

	#define STV_VDP2_CCRSA		((stv_vdp2_regs[0x100/4] >> 16) & 0x0000ffff)
	#define STV_VDP2_S1CCRT		((STV_VDP2_CCRSA & 0x1f00) >> 8)
	#define STV_VDP2_S0CCRT		((STV_VDP2_CCRSA & 0x001f) >> 0)

/* 180102 - Colour Calculation Ratio (Sprite 2,3)
 bit-> /----15----|----14----|----13----|----12----|----11----|----10----|----09----|----08----\
       |    --    |    --    |    --    |  S3CCRT4 |  S3CCRT3 |  S3CCRT2 |  S3CCRT1 |  S3CCRT0 |
       |----07----|----06----|----05----|----04----|----03----|----02----|----01----|----00----|
       |    --    |    --    |    --    |  S2CCRT4 |  S2CCRT3 |  S2CCRT2 |  S2CCRT1 |  S2CCRT0 |
       \----------|----------|----------|----------|----------|----------|----------|---------*/

	#define STV_VDP2_CCRSB		((stv_vdp2_regs[0x100/4] >> 0) & 0x0000ffff)
	#define STV_VDP2_S3CCRT		((STV_VDP2_CCRSB & 0x1f00) >> 8)
	#define STV_VDP2_S2CCRT		((STV_VDP2_CCRSB & 0x001f) >> 0)

/* 180104 - Colour Calculation Ratio (Sprite 4,5)
 bit-> /----15----|----14----|----13----|----12----|----11----|----10----|----09----|----08----\
       |    --    |    --    |    --    |  S5CCRT4 |  S5CCRT3 |  S5CCRT2 |  S5CCRT1 |  S5CCRT0 |
       |----07----|----06----|----05----|----04----|----03----|----02----|----01----|----00----|
       |    --    |    --    |    --    |  S4CCRT4 |  S4CCRT3 |  S4CCRT2 |  S4CCRT1 |  S4CCRT0 |
       \----------|----------|----------|----------|----------|----------|----------|---------*/

	#define STV_VDP2_CCRSC		((stv_vdp2_regs[0x104/4 ]>> 16) & 0x0000ffff)
	#define STV_VDP2_S5CCRT		((STV_VDP2_CCRSC & 0x1f00) >> 8)
	#define STV_VDP2_S4CCRT		((STV_VDP2_CCRSC & 0x001f) >> 0)

/* 180106 - Colour Calculation Ratio (Sprite 6,7)
 bit-> /----15----|----14----|----13----|----12----|----11----|----10----|----09----|----08----\
       |    --    |    --    |    --    |  S7CCRT4 |  S7CCRT3 |  S7CCRT2 |  S7CCRT1 |  S7CCRT0 |
       |----07----|----06----|----05----|----04----|----03----|----02----|----01----|----00----|
       |    --    |    --    |    --    |  S6CCRT4 |  S6CCRT3 |  S6CCRT2 |  S6CCRT1 |  S6CCRT0 |
       \----------|----------|----------|----------|----------|----------|----------|---------*/

	#define STV_VDP2_CCRSD		((stv_vdp2_regs[0x104/4 ]>> 0) & 0x0000ffff)
	#define STV_VDP2_S7CCRT		((STV_VDP2_CCRSD & 0x1f00) >> 8)
	#define STV_VDP2_S6CCRT		((STV_VDP2_CCRSD & 0x001f) >> 0)

/* 180108 - Colour Calculation Ratio (NBG 0,1)
 bit-> /----15----|----14----|----13----|----12----|----11----|----10----|----09----|----08----\
       |    --    |    --    |    --    | N1CCRT4  | N1CCRT3  | N1CCRT2  | N1CCRT1  | N1CCRT0  |
       |----07----|----06----|----05----|----04----|----03----|----02----|----01----|----00----|
       |    --    |    --    |    --    | N0CCRT4  | N0CCRT3  | N0CCRT2  | N0CCRT1  | N0CCRT0  |
       \----------|----------|----------|----------|----------|----------|----------|---------*/

	#define STV_VDP2_CCRNA	((stv_vdp2_regs[0x108/4] >> 16)&0x0000ffff)
	#define STV_VDP2_N1CCRT	((STV_VDP2_CCRNA & 0x1f00) >> 8)
	#define STV_VDP2_N0CCRT (STV_VDP2_CCRNA & 0x1f)

/* 18010a - Colour Calculation Ratio (NBG 2,3)
 bit-> /----15----|----14----|----13----|----12----|----11----|----10----|----09----|----08----\
       |    --    |    --    |    --    | N3CCRT4  | N3CCRT3  | N3CCRT2  | N3CCRT1  | N3CCRT0  |
       |----07----|----06----|----05----|----04----|----03----|----02----|----01----|----00----|
       |    --    |    --    |    --    | N2CCRT4  | N2CCRT3  | N2CCRT2  | N2CCRT1  | N2CCRT0  |
       \----------|----------|----------|----------|----------|----------|----------|---------*/

	#define STV_VDP2_CCRNB	((stv_vdp2_regs[0x108/4] >> 0)&0x0000ffff)
	#define STV_VDP2_N3CCRT	((STV_VDP2_CCRNB & 0x1f00) >> 8)
	#define STV_VDP2_N2CCRT (STV_VDP2_CCRNB & 0x1f)

/* 18010c - Colour Calculation Ratio (RBG 0)
 bit-> /----15----|----14----|----13----|----12----|----11----|----10----|----09----|----08----\
       |    --    |    --    |    --    |    --    |    --    |    --    |    --    |    --    |
       |----07----|----06----|----05----|----04----|----03----|----02----|----01----|----00----|
       |    --    |    --    |    --    |    --    |    --    |    --    |    --    |    --    |
       \----------|----------|----------|----------|----------|----------|----------|---------*/

	#define STV_VDP2_CCRR	((stv_vdp2_regs[0x10c/4] >> 16)&0x0000ffff)
	#define STV_VDP2_R0CCRT (STV_VDP2_CCRR & 0x1f)

/* 18010e - Colour Calculation Ratio (Line Colour Screen, Back Colour Screen)
 bit-> /----15----|----14----|----13----|----12----|----11----|----10----|----09----|----08----\
       |    --    |    --    |    --    |    --    |    --    |    --    |    --    |    --    |
       |----07----|----06----|----05----|----04----|----03----|----02----|----01----|----00----|
       |    --    |    --    |    --    |    --    |    --    |    --    |    --    |    --    |
       \----------|----------|----------|----------|----------|----------|----------|---------*/

/* 180110 - Colour Offset Enable
 bit-> /----15----|----14----|----13----|----12----|----11----|----10----|----09----|----08----\
       |    --    |    --    |    --    |    --    |    --    |    --    |    --    |    --    |
       |----07----|----06----|----05----|----04----|----03----|----02----|----01----|----00----|
       |    --    |    --    |    --    |    --    |    --    |    --    |    --    |    --    |
       \----------|----------|----------|----------|----------|----------|----------|---------*/

	#define STV_VDP2_CLOFEN ((stv_vdp2_regs[0x110/4] >> 16)&0x0000ffff)
	#define STV_VDP2_N0COEN ((STV_VDP2_CLOFEN & 0x01) >> 0)
	#define STV_VDP2_N1COEN ((STV_VDP2_CLOFEN & 0x02) >> 1)
	#define STV_VDP2_N2COEN ((STV_VDP2_CLOFEN & 0x04) >> 2)
	#define STV_VDP2_N3COEN ((STV_VDP2_CLOFEN & 0x08) >> 3)
	#define STV_VDP2_R0COEN ((STV_VDP2_CLOFEN & 0x10) >> 4)
	#define STV_VDP2_BKCOEN ((STV_VDP2_CLOFEN & 0x20) >> 5)
	#define STV_VDP2_SPCOEN ((STV_VDP2_CLOFEN & 0x40) >> 6)

/* 180112 - Colour Offset Select
 bit-> /----15----|----14----|----13----|----12----|----11----|----10----|----09----|----08----\
       |    --    |    --    |    --    |    --    |    --    |    --    |    --    |    --    |
       |----07----|----06----|----05----|----04----|----03----|----02----|----01----|----00----|
       |    --    |    --    |    --    |    --    |    --    |    --    |    --    |    --    |
       \----------|----------|----------|----------|----------|----------|----------|---------*/

	#define STV_VDP2_CLOFSL ((stv_vdp2_regs[0x110/4] >> 0)&0x0000ffff)
	#define STV_VDP2_N0COSL ((STV_VDP2_CLOFSL & 0x01) >> 0)
	#define STV_VDP2_N1COSL ((STV_VDP2_CLOFSL & 0x02) >> 1)
	#define STV_VDP2_N2COSL ((STV_VDP2_CLOFSL & 0x04) >> 2)
	#define STV_VDP2_N3COSL ((STV_VDP2_CLOFSL & 0x08) >> 3)
	#define STV_VDP2_R0COSL ((STV_VDP2_CLOFSL & 0x10) >> 4)
	#define STV_VDP2_BKCOSL ((STV_VDP2_CLOFSL & 0x20) >> 5)
	#define STV_VDP2_SPCOSL ((STV_VDP2_CLOFSL & 0x40) >> 6)

/* 180114 - Colour Offset A (Red)
 bit-> /----15----|----14----|----13----|----12----|----11----|----10----|----09----|----08----\
       |    --    |    --    |    --    |    --    |    --    |    --    |    --    |    --    |
       |----07----|----06----|----05----|----04----|----03----|----02----|----01----|----00----|
       |    --    |    --    |    --    |    --    |    --    |    --    |    --    |    --    |
       \----------|----------|----------|----------|----------|----------|----------|---------*/

	#define STV_VDP2_COAR ((stv_vdp2_regs[0x114/4] >> 16)&0x0000ffff)

/* 180116 - Colour Offset A (Green)
 bit-> /----15----|----14----|----13----|----12----|----11----|----10----|----09----|----08----\
       |    --    |    --    |    --    |    --    |    --    |    --    |    --    |    --    |
       |----07----|----06----|----05----|----04----|----03----|----02----|----01----|----00----|
       |    --    |    --    |    --    |    --    |    --    |    --    |    --    |    --    |
       \----------|----------|----------|----------|----------|----------|----------|---------*/
	#define STV_VDP2_COAG ((stv_vdp2_regs[0x114/4] >> 0)&0x0000ffff)

/* 180118 - Colour Offset A (Blue)
 bit-> /----15----|----14----|----13----|----12----|----11----|----10----|----09----|----08----\
       |    --    |    --    |    --    |    --    |    --    |    --    |    --    |    --    |
       |----07----|----06----|----05----|----04----|----03----|----02----|----01----|----00----|
       |    --    |    --    |    --    |    --    |    --    |    --    |    --    |    --    |
       \----------|----------|----------|----------|----------|----------|----------|---------*/

	#define STV_VDP2_COAB ((stv_vdp2_regs[0x118/4] >> 16)&0x0000ffff)

/* 18011a - Colour Offset B (Red)
 bit-> /----15----|----14----|----13----|----12----|----11----|----10----|----09----|----08----\
       |    --    |    --    |    --    |    --    |    --    |    --    |    --    |    --    |
       |----07----|----06----|----05----|----04----|----03----|----02----|----01----|----00----|
       |    --    |    --    |    --    |    --    |    --    |    --    |    --    |    --    |
       \----------|----------|----------|----------|----------|----------|----------|---------*/
	#define STV_VDP2_COBR ((stv_vdp2_regs[0x118/4] >> 0)&0x0000ffff)

/* 18011b - Colour Offset B (Green)
 bit-> /----15----|----14----|----13----|----12----|----11----|----10----|----09----|----08----\
       |    --    |    --    |    --    |    --    |    --    |    --    |    --    |    --    |
       |----07----|----06----|----05----|----04----|----03----|----02----|----01----|----00----|
       |    --    |    --    |    --    |    --    |    --    |    --    |    --    |    --    |
       \----------|----------|----------|----------|----------|----------|----------|---------*/
	#define STV_VDP2_COBG ((stv_vdp2_regs[0x11c/4] >> 16)&0x0000ffff)

/* 18011c - Colour Offset B (Blue)
 bit-> /----15----|----14----|----13----|----12----|----11----|----10----|----09----|----08----\
       |    --    |    --    |    --    |    --    |    --    |    --    |    --    |    --    |
       |----07----|----06----|----05----|----04----|----03----|----02----|----01----|----00----|
       |    --    |    --    |    --    |    --    |    --    |    --    |    --    |    --    |
       \----------|----------|----------|----------|----------|----------|----------|---------*/
	#define STV_VDP2_COBB ((stv_vdp2_regs[0x11c/4] >> 0)&0x0000ffff)

/*For Debug purposes only*/
static struct stv_vdp2_debugging
{
	UINT8 l_en;	 /*For Layer enable/disable*/
	UINT8 win;	 /*Enters into Window effect debug menu*/
	UINT32 error; /*bits for VDP2 error logging*/
	UINT8 roz;   /*Debug roz on screen*/
} debug;

/*
Errors are currently mapped as follows:
x--- ---- ---- ---- ---- ---- ---- ---- VRAM Size = 8 Mbit
-x-- ---- ---- ---- ---- ---- ---- ---- CRKTE used
---- ---- ---- ---- ---- ---- ---- --x- Mosaic Control
---- ---- ---- ---- ---- ---- ---- ---x Window on tilemap
*/
#define VDP2_ERR(_bit_) (debug.error & _bit_)
#define VDP2_CHK(_bit_) (debug.error^=_bit_)

/* Not sure if to use this for the rotating tilemaps as well or just use different draw functions, might add too much bloat */
static struct stv_vdp2_tilemap_capabilities
{
	UINT8  enabled;
	UINT8  transparency;
	UINT8  colour_calculation_enabled;
	UINT8  colour_depth;
	UINT8  alpha;
	UINT8  tile_size;
	UINT8  bitmap_enable;
	UINT8  bitmap_size;
	UINT8  bitmap_palette_number;
	UINT8  bitmap_map;
	UINT16 map_offset[16];
	UINT8  map_count;

	UINT8  pattern_data_size;
	UINT8  character_number_supplement;
	UINT8  special_priority_register;
	UINT8  special_colour_control_register;
	UINT8  supplementary_palette_bits;
	UINT8  supplementary_character_bits;

	INT16 scrollx;
	INT16 scrolly;
	UINT32 incx, incy;

	UINT8	linescroll_enable;
	UINT8	linescroll_interval;
	UINT32	linescroll_table_address;
	UINT8	vertical_linescroll_enable;
	UINT8	linezoom_enable;

	UINT8  plane_size;
	UINT8  colour_ram_address_offset;
	UINT8  fade_control;
	UINT8  window_control;

//  UINT8  real_map_offset[16];

	int layer_name; /* just to keep track */
} stv2_current_tilemap;

#define STV_VDP2_RBG_ROTATION_PARAMETER_A	1
#define STV_VDP2_RBG_ROTATION_PARAMETER_B	2

static struct rotation_table
{
	INT32	xst;
	INT32	yst;
	INT32	zst;
	INT32	dxst;
	INT32	dyst;
	INT32	dx;
	INT32	dy;
	INT32	A;
	INT32	B;
	INT32	C;
	INT32	D;
	INT32	E;
	INT32	F;
	INT32	px;
	INT32	py;
	INT32	pz;
	INT32	cx;
	INT32	cy;
	INT32	cz;
	INT32	mx;
	INT32	my;
	INT32	kx;
	INT32	ky;
	UINT32	kast;
	INT32	dkast;
	INT32	dkax;

} stv_current_rotation_parameter_table;

static struct _stv_vdp2_layer_data_placement
{
	UINT32	map_offset_min;
	UINT32	map_offset_max;
	UINT32  tile_offset_min;
	UINT32	tile_offset_max;
} stv_vdp2_layer_data_placement;

static struct _stv_rbg_cache_data
{
	UINT8	watch_vdp2_vram_writes;
	UINT8	is_cache_dirty;

	UINT32	map_offset_min[2];
	UINT32	map_offset_max[2];
	UINT32	tile_offset_min[2];
	UINT32	tile_offset_max[2];

	struct stv_vdp2_tilemap_capabilities	layer_data[2];

} stv_rbg_cache_data;

#define mul_fixed32( a, b ) mul_32x32_shift( a, b, 16 )

static void stv_vdp2_fill_rotation_parameter_table( running_machine *machine, UINT8 rot_parameter )
{
	UINT32 address;

	address = (((STV_VDP2_RPTAU << 16) | STV_VDP2_RPTAL) << 1);
	if ( rot_parameter == 1 )
	{
		address &= ~0x00000080;
	}
	else if ( rot_parameter == 2 )
	{
		address |= 0x00000080;
	}

	stv_current_rotation_parameter_table.xst  = (stv_vdp2_vram[address/4] & 0x1fffffc0) | ((stv_vdp2_vram[address/4] & 0x10000000) ? 0xe0000000 : 0x00000000 );
	stv_current_rotation_parameter_table.yst  = (stv_vdp2_vram[address/4 + 1] & 0x1fffffc0) | ((stv_vdp2_vram[address/4 + 1] & 0x10000000) ? 0xe0000000 : 0x00000000 );
	stv_current_rotation_parameter_table.zst  = (stv_vdp2_vram[address/4 + 2] & 0x1fffffc0) | ((stv_vdp2_vram[address/4 + 2] & 0x10000000) ? 0xe0000000 : 0x00000000 );
	stv_current_rotation_parameter_table.dxst = (stv_vdp2_vram[address/4 + 3] & 0x0007ffc0) | ((stv_vdp2_vram[address/4 + 3] & 0x00040000) ? 0xfff80000 : 0x00000000 );
	stv_current_rotation_parameter_table.dyst = (stv_vdp2_vram[address/4 + 4] & 0x0007ffc0) | ((stv_vdp2_vram[address/4 + 4] & 0x00040000) ? 0xfff80000 : 0x00000000 );
	stv_current_rotation_parameter_table.dx   = (stv_vdp2_vram[address/4 + 5] & 0x0007ffc0) | ((stv_vdp2_vram[address/4 + 5] & 0x00040000) ? 0xfff80000 : 0x00000000 );
	stv_current_rotation_parameter_table.dy   = (stv_vdp2_vram[address/4 + 6] & 0x0007ffc0) | ((stv_vdp2_vram[address/4 + 6] & 0x00040000) ? 0xfff80000 : 0x00000000 );
	stv_current_rotation_parameter_table.A	  = (stv_vdp2_vram[address/4 + 7] & 0x000fffc0) | ((stv_vdp2_vram[address/4 + 7] & 0x00080000) ? 0xfff00000 : 0x00000000 );
	stv_current_rotation_parameter_table.B    = (stv_vdp2_vram[address/4 + 8] & 0x000fffc0) | ((stv_vdp2_vram[address/4 + 8] & 0x00080000) ? 0xfff00000 : 0x00000000 );
	stv_current_rotation_parameter_table.C    = (stv_vdp2_vram[address/4 + 9] & 0x000fffc0) | ((stv_vdp2_vram[address/4 + 9] & 0x00080000) ? 0xfff00000 : 0x00000000 );
	stv_current_rotation_parameter_table.D    = (stv_vdp2_vram[address/4 + 10] & 0x000fffc0) | ((stv_vdp2_vram[address/4 + 10] & 0x00080000) ? 0xfff00000 : 0x00000000 );
	stv_current_rotation_parameter_table.E    = (stv_vdp2_vram[address/4 + 11] & 0x000fffc0) | ((stv_vdp2_vram[address/4 + 11] & 0x00080000) ? 0xfff00000 : 0x00000000 );
	stv_current_rotation_parameter_table.F    = (stv_vdp2_vram[address/4 + 12] & 0x000fffc0) | ((stv_vdp2_vram[address/4 + 12] & 0x00080000) ? 0xfff00000 : 0x00000000 );
	stv_current_rotation_parameter_table.px	  = (stv_vdp2_vram[address/4 + 13] & 0x3fff0000) | ((stv_vdp2_vram[address/4 + 13] & 0x30000000) ? 0xc0000000 : 0x00000000 );
	stv_current_rotation_parameter_table.py	  = (stv_vdp2_vram[address/4 + 13] & 0x00003fff) << 16;
	if ( stv_current_rotation_parameter_table.py & 0x20000000 ) stv_current_rotation_parameter_table.py |= 0xc0000000;
	stv_current_rotation_parameter_table.pz   = (stv_vdp2_vram[address/4 + 14] & 0x3fff0000) | ((stv_vdp2_vram[address/4 + 14] & 0x20000000) ? 0xc0000000 : 0x00000000 );
	stv_current_rotation_parameter_table.cx   = (stv_vdp2_vram[address/4 + 15] & 0x3fff0000) | ((stv_vdp2_vram[address/4 + 15] & 0x20000000) ? 0xc0000000 : 0x00000000 );
	stv_current_rotation_parameter_table.cy   = (stv_vdp2_vram[address/4 + 15] & 0x00003fff) << 16;
	if ( stv_current_rotation_parameter_table.cy & 0x20000000 ) stv_current_rotation_parameter_table.cy |= 0xc0000000;
	stv_current_rotation_parameter_table.cz   = (stv_vdp2_vram[address/4 + 16] & 0x3fff0000) | ((stv_vdp2_vram[address/4 + 16] & 0x20000000) ? 0xc0000000 : 0x00000000 );
	stv_current_rotation_parameter_table.mx   = (stv_vdp2_vram[address/4 + 17] & 0x3fffffc0) | ((stv_vdp2_vram[address/4 + 17] & 0x20000000) ? 0xc0000000 : 0x00000000 );
	stv_current_rotation_parameter_table.my   = (stv_vdp2_vram[address/4 + 18] & 0x3fffffc0) | ((stv_vdp2_vram[address/4 + 18] & 0x20000000) ? 0xc0000000 : 0x00000000 );
	stv_current_rotation_parameter_table.kx   = (stv_vdp2_vram[address/4 + 19] & 0x00ffffff) | ((stv_vdp2_vram[address/4 + 19] & 0x00800000) ? 0xff000000 : 0x00000000 );
	stv_current_rotation_parameter_table.ky   = (stv_vdp2_vram[address/4 + 20] & 0x00ffffff) | ((stv_vdp2_vram[address/4 + 20] & 0x00800000) ? 0xff000000 : 0x00000000 );
	stv_current_rotation_parameter_table.kast = (stv_vdp2_vram[address/4 + 21] & 0xffffffc0);
	stv_current_rotation_parameter_table.dkast= (stv_vdp2_vram[address/4 + 22] & 0x03ffffc0) | ((stv_vdp2_vram[address/4 + 22] & 0x02000000) ? 0xfc000000 : 0x00000000 );
	stv_current_rotation_parameter_table.dkax = (stv_vdp2_vram[address/4 + 23] & 0x03ffffc0) | ((stv_vdp2_vram[address/4 + 23] & 0x02000000) ? 0xfc000000 : 0x00000000 );

#define RP	stv_current_rotation_parameter_table

	if(LOG_ROZ == 1) logerror( "Rotation parameter table (%d)\n", rot_parameter );
	if(LOG_ROZ == 1) logerror( "xst = %x, yst = %x, zst = %x\n", RP.xst, RP.yst, RP.zst );
	if(LOG_ROZ == 1) logerror( "dxst = %x, dyst = %x\n", RP.dxst, RP.dyst );
	if(LOG_ROZ == 1) logerror( "dx = %x, dy = %x\n", RP.dx, RP.dy );
	if(LOG_ROZ == 1) logerror( "A = %x, B = %x, C = %x, D = %x, E = %x, F = %x\n", RP.A, RP.B, RP.C, RP.D, RP.E, RP.F );
	if(LOG_ROZ == 1) logerror( "px = %x, py = %x, pz = %x\n", RP.px, RP.py, RP.pz );
	if(LOG_ROZ == 1) logerror( "cx = %x, cy = %x, cz = %x\n", RP.cx, RP.cy, RP.cz );
	if(LOG_ROZ == 1) logerror( "mx = %x, my = %x\n", RP.mx, RP.my );
	if(LOG_ROZ == 1) logerror( "kx = %x, ky = %x\n", RP.kx, RP.ky );
	if(LOG_ROZ == 1) logerror( "kast = %x, dkast = %x, dkax = %x\n", RP.kast, RP.dkast, RP.dkax );

	/*Attempt to show on screen the rotation table*/
	if(LOG_ROZ == 2)
	{
		if(input_code_pressed_once(machine, JOYCODE_Y_UP_SWITCH))
			debug.roz++;

		if(input_code_pressed_once(machine, JOYCODE_Y_DOWN_SWITCH))
			debug.roz--;

		if(debug.roz > 10)
			debug.roz = 10;

		switch(debug.roz)
		{
	    	case 0: popmessage( "Rotation parameter Table (%d)", rot_parameter ); break;
	        case 1: popmessage( "xst = %x, yst = %x, zst = %x", RP.xst, RP.yst, RP.zst ); break;
	        case 2: popmessage( "dxst = %x, dyst = %x", RP.dxst, RP.dyst ); break;
	        case 3: popmessage( "dx = %x, dy = %x", RP.dx, RP.dy ); break;
	        case 4: popmessage( "A = %x, B = %x, C = %x, D = %x, E = %x, F = %x", RP.A, RP.B, RP.C, RP.D, RP.E, RP.F ); break;
	        case 5: popmessage( "px = %x, py = %x, pz = %x", RP.px, RP.py, RP.pz ); break;
			case 6:	popmessage( "cx = %x, cy = %x, cz = %x", RP.cx, RP.cy, RP.cz ); break;
			case 7:	popmessage( "mx = %x, my = %x", RP.mx, RP.my ); break;
			case 8:	popmessage( "kx = %x, ky = %x", RP.kx, RP.ky ); break;
	 		case 9:	popmessage( "kast = %x, dkast = %x, dkax = %x", RP.kast, RP.dkast, RP.dkax ); break;
			case 10: break;
		}
	}
}

/* check if RGB layer has rotation applied */
static UINT8 stv_vdp2_is_rotation_applied(void)
{
#define _FIXED_1	(0x00010000)
#define _FIXED_0	(0x00000000)

	if ( RP.A == _FIXED_1 &&
		 RP.B == _FIXED_0 &&
		 RP.C == _FIXED_0 &&
		 RP.D == _FIXED_0 &&
		 RP.E == _FIXED_1 &&
		 RP.F == _FIXED_0 &&
		 RP.dxst == _FIXED_0 &&
		 RP.dyst == _FIXED_1 &&
		 RP.dx == _FIXED_1 &&
		 RP.dy == _FIXED_0 )
	{
		return 0;
	}
	else
	{
		return 1;
	}
}

static UINT8 stv_vdp2_are_map_registers_equal(void)
{
	int i;

	for ( i = 1; i < stv2_current_tilemap.map_count; i++ )
	{
		if ( stv2_current_tilemap.map_offset[i] != stv2_current_tilemap.map_offset[0] )
		{
			return 0;
		}
	}
	return 1;
}

static void stv_vdp2_check_fade_control_for_layer(void)
{
	if ( stv2_current_tilemap.fade_control & 1 )
	{
		if ( stv2_current_tilemap.fade_control & 2 )
		{
			if ((STV_VDP2_COBR & 0x1ff) == 0 &&
				(STV_VDP2_COBG & 0x1ff) == 0 &&
				(STV_VDP2_COBB & 0x1ff) == 0 )
			{
				stv2_current_tilemap.fade_control = 0;
			}
		}
		else
		{
			if ((STV_VDP2_COAR & 0x1ff) == 0 &&
				(STV_VDP2_COAG & 0x1ff) == 0 &&
				(STV_VDP2_COAB & 0x1ff) == 0 )
			{
				stv2_current_tilemap.fade_control = 0;
			}
		}
	}
}

#define STV_VDP2_CP_NBG0_PNMDR		0x0
#define STV_VDP2_CP_NBG1_PNMDR		0x1
#define STV_VDP2_CP_NBG2_PNMDR		0x2
#define STV_VDP2_CP_NBG3_PNMDR		0x3
#define STV_VDP2_CP_NBG0_CPDR		0x4
#define STV_VDP2_CP_NBG1_CPDR		0x5
#define STV_VDP2_CP_NBG2_CPDR		0x6
#define STV_VDP2_CP_NBG3_CPDR		0x7

static UINT8 stv_vdp2_check_vram_cycle_pattern_registers(
								UINT8 access_command_pnmdr,
								UINT8 access_command_cpdr,
								UINT8 bitmap_enable )
{
	int i;
	UINT8  access_command_ok = 0;
	UINT16 cp_regs[8];
	cp_regs[0] = STV_VDP2_CYCA0L;
	cp_regs[1] = STV_VDP2_CYCA0U;
	cp_regs[2] = STV_VDP2_CYCA1L;
	cp_regs[3] = STV_VDP2_CYCA1U;
	cp_regs[4] = STV_VDP2_CYCA2L;
	cp_regs[5] = STV_VDP2_CYCA2U;
	cp_regs[6] = STV_VDP2_CYCA3L;
	cp_regs[7] = STV_VDP2_CYCA3U;

	if ( bitmap_enable ) access_command_ok = 1;

	for ( i = 0; i < 8; i++ )
	{
		if ( ((cp_regs[i] >> 12) & 0xf) == access_command_pnmdr )
		{
			access_command_ok |= 1;
		}
		if ( ((cp_regs[i] >> 12) & 0xf) == access_command_cpdr )
		{
			access_command_ok |= 2;
		}
		if ( ((cp_regs[i] >> 8) & 0xf) == access_command_pnmdr )
		{
			access_command_ok |= 1;
		}
		if ( ((cp_regs[i] >> 8) & 0xf) == access_command_cpdr )
		{
			access_command_ok |= 2;
		}
		if ( ((cp_regs[i] >> 4) & 0xf) == access_command_pnmdr )
		{
			access_command_ok |= 1;
		}
		if ( ((cp_regs[i] >> 4) & 0xf) == access_command_cpdr )
		{
			access_command_ok |= 2;
		}
		if ( ((cp_regs[i] >> 0) & 0xf) == access_command_pnmdr )
		{
			access_command_ok |= 1;
		}
		if ( ((cp_regs[i] >> 0) & 0xf) == access_command_cpdr )
		{
			access_command_ok |= 2;
		}
	}
	return access_command_ok == 3 ? 1 : 0;
}

INLINE UINT16 stv_add_blend(UINT16 a, UINT16 b)
{
	UINT16 _r = (a & 0x7c00) + (b & 0x7c00);
	UINT16 _g = (a & 0x03e0) + (b & 0x03e0);
	UINT16 _b = (a & 0x001f) + (b & 0x001f);

	if ( _r > 0x7c00 ) _r = 0x7c00;
	if ( _g > 0x03e0 ) _g = 0x03e0;
	if ( _b > 0x001f ) _b = 0x001f;

	return _r | _g | _b;

}

static void stv_vdp2_drawgfxzoom(
		bitmap_t *dest_bmp,const rectangle *clip,const gfx_element *gfx,
		UINT32 code,UINT32 color,int flipx,int flipy,int sx,int sy,
		int transparency,int transparent_color,int scalex, int scaley,
		int sprite_screen_width, int sprite_screen_height, int alpha)
{
	rectangle myclip;

	if (!scalex || !scaley) return;

	if (gfx->pen_usage && transparency == STV_TRANSPARENCY_PEN)
	{
		int transmask = 0;

		transmask = 1 << (transparent_color & 0xff);

		if ((gfx->pen_usage[code] & ~transmask) == 0)
			/* character is totally transparent, no need to draw */
			return;
		else if ((gfx->pen_usage[code] & transmask) == 0)
			/* character is totally opaque, can disable transparency */
			transparency = STV_TRANSPARENCY_NONE;
	}

	/*
    scalex and scaley are 16.16 fixed point numbers
    1<<15 : shrink to 50%
    1<<16 : uniform scale
    1<<17 : double to 200%
    */


	/* KW 991012 -- Added code to force clip to bitmap boundary */
	if(clip)
	{
		myclip.min_x = clip->min_x;
		myclip.max_x = clip->max_x;
		myclip.min_y = clip->min_y;
		myclip.max_y = clip->max_y;

		if (myclip.min_x < 0) myclip.min_x = 0;
		if (myclip.max_x >= dest_bmp->width) myclip.max_x = dest_bmp->width-1;
		if (myclip.min_y < 0) myclip.min_y = 0;
		if (myclip.max_y >= dest_bmp->height) myclip.max_y = dest_bmp->height-1;

		clip=&myclip;
	}

	if( gfx )
	{
		const pen_t *pal = &gfx->machine->pens[gfx->color_base + gfx->color_granularity * (color % gfx->total_colors)];
		const UINT8 *source_base = gfx_element_get_data(gfx, code % gfx->total_elements);

		//int sprite_screen_height = (scaley*gfx->height+0x8000)>>16;
		//int sprite_screen_width = (scalex*gfx->width+0x8000)>>16;

		if (sprite_screen_width && sprite_screen_height)
		{
			/* compute sprite increment per screen pixel */
			//int dx = (gfx->width<<16)/sprite_screen_width;
			//int dy = (gfx->height<<16)/sprite_screen_height;
			int dx = stv2_current_tilemap.incx;
			int dy = stv2_current_tilemap.incy;

			int ex = sx+sprite_screen_width;
			int ey = sy+sprite_screen_height;

			int x_index_base;
			int y_index;

			if( flipx )
			{
				x_index_base = (sprite_screen_width-1)*dx;
				dx = -dx;
			}
			else
			{
				x_index_base = 0;
			}

			if( flipy )
			{
				y_index = (sprite_screen_height-1)*dy;
				dy = -dy;
			}
			else
			{
				y_index = 0;
			}

			if( clip )
			{
				if( sx < clip->min_x)
				{ /* clip left */
					int pixels = clip->min_x-sx;
					sx += pixels;
					x_index_base += pixels*dx;
				}
				if( sy < clip->min_y )
				{ /* clip top */
					int pixels = clip->min_y-sy;
					sy += pixels;
					y_index += pixels*dy;
				}
				/* NS 980211 - fixed incorrect clipping */
				if( ex > clip->max_x+1 )
				{ /* clip right */
					int pixels = ex-clip->max_x-1;
					ex -= pixels;
				}
				if( ey > clip->max_y+1 )
				{ /* clip bottom */
					int pixels = ey-clip->max_y-1;
					ey -= pixels;
				}
			}

			if( ex>sx )
			{ /* skip if inner loop doesn't draw anything */
				int y;

				/* case 0: STV_TRANSPARENCY_NONE */
				if (transparency == STV_TRANSPARENCY_NONE)
				{
					if (gfx->flags & GFX_ELEMENT_PACKED)
					{
						for( y=sy; y<ey; y++ )
						{
							const UINT8 *source = source_base + (y_index>>16) * gfx->line_modulo;
							UINT16 *dest = BITMAP_ADDR16(dest_bmp, y, 0);

							int x, x_index = x_index_base;
							for( x=sx; x<ex; x++ )
							{
								dest[x] = pal[(source[x_index>>17] >> ((x_index & 0x10000) >> 14)) & 0x0f];
								x_index += dx;
							}

							y_index += dy;
						}
					}
					else
					{
						for( y=sy; y<ey; y++ )
						{
							const UINT8 *source = source_base + (y_index>>16) * gfx->line_modulo;
							UINT16 *dest = BITMAP_ADDR16(dest_bmp, y, 0);

							int x, x_index = x_index_base;
							for( x=sx; x<ex; x++ )
							{
								dest[x] = pal[source[x_index>>16]];
								x_index += dx;
							}

							y_index += dy;
						}
					}
				}

				/* case 1: STV_TRANSPARENCY_PEN */
				if (transparency == STV_TRANSPARENCY_PEN)
				{
					if (gfx->flags & GFX_ELEMENT_PACKED)
					{
						for( y=sy; y<ey; y++ )
						{
							const UINT8 *source = source_base + (y_index>>16) * gfx->line_modulo;
							UINT16 *dest = BITMAP_ADDR16(dest_bmp, y, 0);

							int x, x_index = x_index_base;
							for( x=sx; x<ex; x++ )
							{
								int c = (source[x_index>>17] >> ((x_index & 0x10000) >> 14)) & 0x0f;
								if( c != transparent_color ) dest[x] = pal[c];
								x_index += dx;
							}

							y_index += dy;
						}
					}
					else
					{
						for( y=sy; y<ey; y++ )
						{
							const UINT8 *source = source_base + (y_index>>16) * gfx->line_modulo;
							UINT16 *dest = BITMAP_ADDR16(dest_bmp, y, 0);

							int x, x_index = x_index_base;
							for( x=sx; x<ex; x++ )
							{
								int c = source[x_index>>16];
								if( c != transparent_color ) dest[x] = pal[c];
								x_index += dx;
							}

							y_index += dy;
						}
					}
				}

				/* case 6: STV_TRANSPARENCY_ALPHA */
				if (transparency == STV_TRANSPARENCY_ALPHA)
				{
					for( y=sy; y<ey; y++ )
					{
						const UINT8 *source = source_base + (y_index>>16) * gfx->line_modulo;
						UINT16 *dest = BITMAP_ADDR16(dest_bmp, y, 0);

						int x, x_index = x_index_base;
						for( x=sx; x<ex; x++ )
						{
							int c = source[x_index>>16];
							if( c != transparent_color ) dest[x] = alpha_blend_r16(dest[x], pal[c], alpha);
							x_index += dx;
						}

						y_index += dy;
					}
				}

				/* case : STV_TRANSPARENCY_ADD_BLEND */
				if (transparency == STV_TRANSPARENCY_ADD_BLEND )
				{
					if (gfx->flags & GFX_ELEMENT_PACKED)
					{
						for( y=sy; y<ey; y++ )
						{
							const UINT8 *source = source_base + (y_index>>16) * gfx->line_modulo;
							UINT16 *dest = BITMAP_ADDR16(dest_bmp, y, 0);

							int x, x_index = x_index_base;
							for( x=sx; x<ex; x++ )
							{
								int c = (source[x_index>>17] >> ((x_index & 0x10000) >> 14)) & 0x0f;
								if( c != transparent_color ) dest[x] = stv_add_blend(dest[x],pal[c]);
								x_index += dx;
							}

							y_index += dy;
						}
					}
					else
					{
						for( y=sy; y<ey; y++ )
						{
							const UINT8 *source = source_base + (y_index>>16) * gfx->line_modulo;
							UINT16 *dest = BITMAP_ADDR16(dest_bmp, y, 0);

							int x, x_index = x_index_base;
							for( x=sx; x<ex; x++ )
							{
								int c = source[x_index>>16];
								if( c != transparent_color ) dest[x] = stv_add_blend(dest[x],pal[c]);
								x_index += dx;
							}

							y_index += dy;
						}
					}
				}

			}
		}
	}

}

static void stv_vdp2_compute_color_offset_RGB555( int *r, int *g, int *b, int cor )
{
	*r <<= 3;
	*g <<= 3;
	*b <<= 3;
	if ( cor == 0 )
	{
		*r = (STV_VDP2_COAR & 0x100) ? (*r - (0x100 - (STV_VDP2_COAR & 0xff))) : ((STV_VDP2_COAR & 0xff) + *r);
		*g = (STV_VDP2_COAG & 0x100) ? (*g - (0x100 - (STV_VDP2_COAG & 0xff))) : ((STV_VDP2_COAG & 0xff) + *g);
		*b = (STV_VDP2_COAB & 0x100) ? (*b - (0x100 - (STV_VDP2_COAB & 0xff))) : ((STV_VDP2_COAB & 0xff) + *b);
	}
	else
	{
		*r = (STV_VDP2_COBR & 0x100) ? (*r - (0xff - (STV_VDP2_COBR & 0xff))) : ((STV_VDP2_COBR & 0xff) + *r);
		*g = (STV_VDP2_COBG & 0x100) ? (*g - (0xff - (STV_VDP2_COBG & 0xff))) : ((STV_VDP2_COBG & 0xff) + *g);
		*b = (STV_VDP2_COBB & 0x100) ? (*b - (0xff - (STV_VDP2_COBB & 0xff))) : ((STV_VDP2_COBB & 0xff) + *b);
	}
	if(*r < 0) 		{ *r = 0; }
	if(*r > 0xff) 	{ *r = 0xff; }
	if(*g < 0) 		{ *g = 0; }
	if(*g > 0xff) 	{ *g = 0xff; }
	if(*b < 0) 		{ *b = 0; }
	if(*b > 0xff) 	{ *b = 0xff; }
	*r >>= 3;
	*g >>= 3;
	*b >>= 3;

}

static void stv_vdp2_compute_color_offset_RGB555_UINT16(UINT16 *rgb, int cor)
{
	int _r = (*rgb & 0x7c00) >> (10-3);
	int _g = (*rgb & 0x03e0) >> (5-3);
	int _b = (*rgb & 0x001f) << 3;
	if ( cor == 0 )
	{
		_r = (STV_VDP2_COAR & 0x100) ? (_r - (0x100 - (STV_VDP2_COAR & 0xff))) : ((STV_VDP2_COAR & 0xff) + _r);
		_g = (STV_VDP2_COAG & 0x100) ? (_g - (0x100 - (STV_VDP2_COAG & 0xff))) : ((STV_VDP2_COAG & 0xff) + _g);
		_b = (STV_VDP2_COAB & 0x100) ? (_b - (0x100 - (STV_VDP2_COAB & 0xff))) : ((STV_VDP2_COAB & 0xff) + _b);
	}
	else
	{
		_r = (STV_VDP2_COBR & 0x100) ? (_r - (0xff - (STV_VDP2_COBR & 0xff))) : ((STV_VDP2_COBR & 0xff) + _r);
		_g = (STV_VDP2_COBG & 0x100) ? (_g - (0xff - (STV_VDP2_COBG & 0xff))) : ((STV_VDP2_COBG & 0xff) + _g);
		_b = (STV_VDP2_COBB & 0x100) ? (_b - (0xff - (STV_VDP2_COBB & 0xff))) : ((STV_VDP2_COBB & 0xff) + _b);
	}
	if(_r < 0) 		{ _r = 0; }
	if(_r > 0xff) 	{ _r = 0xff; }
	if(_g < 0) 		{ _g = 0; }
	if(_g > 0xff) 	{ _g = 0xff; }
	if(_b < 0) 		{ _b = 0; }
	if(_b > 0xff) 	{ _b = 0xff; }
	_r >>= 3;
	_g >>= 3;
	_b >>= 3;

	*rgb = (_r << 10) |  (_g << 5) | _b;
}

static void stv_vdp2_drawgfx_rgb555( bitmap_t *dest_bmp, const rectangle *clip, UINT32 code, int flipx, int flipy,
									 int sx, int sy, int transparency, int alpha)
{
	rectangle myclip;
	UINT8* gfxdata;
	int t_pen;
	int sprite_screen_width, sprite_screen_height;

	gfxdata = stv_vdp2_gfx_decode + code * 0x20;
	sprite_screen_width = sprite_screen_height = 8;

	/* KW 991012 -- Added code to force clip to bitmap boundary */
	if(clip)
	{
		myclip.min_x = clip->min_x;
		myclip.max_x = clip->max_x;
		myclip.min_y = clip->min_y;
		myclip.max_y = clip->max_y;

		if (myclip.min_x < 0) myclip.min_x = 0;
		if (myclip.max_x >= dest_bmp->width) myclip.max_x = dest_bmp->width-1;
		if (myclip.min_y < 0) myclip.min_y = 0;
		if (myclip.max_y >= dest_bmp->height) myclip.max_y = dest_bmp->height-1;

		clip=&myclip;
	}
	{
		int dx = stv2_current_tilemap.incx;
		int dy = stv2_current_tilemap.incy;

		int ex = sx+sprite_screen_width;
		int ey = sy+sprite_screen_height;

		int x_index_base;
		int y_index;

		if( flipx )
		{
			x_index_base = (sprite_screen_width-1)*dx;
			dx = -dx;
		}
		else
		{
			x_index_base = 0;
		}

		if( flipy )
		{
			y_index = (sprite_screen_height-1)*dy;
			dy = -dy;
		}
		else
		{
			y_index = 0;
		}

		if( clip )
		{
			if( sx < clip->min_x)
			{ /* clip left */
				int pixels = clip->min_x-sx;
				sx += pixels;
				x_index_base += pixels*dx;
			}
			if( sy < clip->min_y )
			{ /* clip top */
				int pixels = clip->min_y-sy;
				sy += pixels;
				y_index += pixels*dy;
			}
			/* NS 980211 - fixed incorrect clipping */
			if( ex > clip->max_x+1 )
			{ /* clip right */
				int pixels = ex-clip->max_x-1;
				ex -= pixels;
			}
			if( ey > clip->max_y+1 )
			{ /* clip bottom */
				int pixels = ey-clip->max_y-1;
				ey -= pixels;
			}
		}

		if( ex>sx )
		{ /* skip if inner loop doesn't draw anything */
			int y;

			for( y=sy; y<ey; y++ )
			{
				const UINT8 *source = gfxdata + (y_index>>16)*16;
				UINT16 *dest = BITMAP_ADDR16(dest_bmp, y, 0);
				UINT16 data;

				int x, x_index = x_index_base;
				for( x=sx; x<ex; x++ )
				{
					int r,g,b;

					data = (source[(x_index>>16)*2] << 8) | source[(x_index>>16)*2+1];
					t_pen = (data & 0x8000) || ( transparency == STV_TRANSPARENCY_NONE );
					if (t_pen)
					{
						b = (data & 0x7c00) >> 10;
						g = (data & 0x03e0) >> 5;
						r = (data & 0x001f);
						if(stv2_current_tilemap.fade_control & 1)
							stv_vdp2_compute_color_offset_RGB555(&r,&g,&b,stv2_current_tilemap.fade_control & 2);

						if ( transparency == STV_TRANSPARENCY_ALPHA )
							dest[x] = alpha_blend_r16( dest[x], b | g << 5 | r << 10, alpha );
						else
							dest[x] = b | g << 5 | r << 10;
					}
					x_index += dx;
				}

				y_index += dy;
			}

		}

	}

}

static void stv_vdp2_draw_basic_bitmap(running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect)
{
//  if(LOG_VDP2) logerror ("bitmap enable %02x size %08x depth %08x\n", stv2_current_tilemap.layer_name, stv2_current_tilemap.bitmap_size, stv2_current_tilemap.colour_depth);
//  popmessage ("bitmap enable %02x size %08x depth %08x number %02x", stv2_current_tilemap.layer_name, stv2_current_tilemap.bitmap_size, stv2_current_tilemap.colour_depth,stv2_current_tilemap.bitmap_palette_number);
	//popmessage("%04x",STV_VDP2_SCRCTL);

	int xsize = 0, xsizemask = 0;
	int ysize = 0, ysizemask = 0;
	int xlinesize = 0, xpixelsize = 0;
	int xcnt,ycnt;
	UINT8* gfxdata = stv_vdp2_gfx_decode;
	static UINT16 *destline;
	UINT16 pal_color_offset = 0;
	UINT8* gfxdatalow, *gfxdatahigh;
	/*Window effect 1=no draw*/
	int tw = 0;
	/*Transparency code 1=opaque,0=transparent*/
	int t_pen;
	if (!stv2_current_tilemap.enabled) return;

	/* size for n0 / n1 */
	switch (stv2_current_tilemap.bitmap_size)
	{
		case 0: xsize=512; ysize=256; break;
		case 1: xsize=512; ysize=512; break;
		case 2: xsize=1024; ysize=256; break;
		case 3: xsize=1024; ysize=512; break;
	}
	xsizemask = xsize - 1;
	ysizemask = ysize - 1;

	switch( stv2_current_tilemap.colour_depth )
	{
		case 0: xlinesize = xsize / 2; xpixelsize = 0; break;
		case 1: xlinesize = xsize; xpixelsize = 1; break;
		case 2: case 3: xlinesize = xsize * 2; xpixelsize = 2; break;
		case 4: xlinesize = xsize * 4; xpixelsize = 4; break;
	}

	if(stv2_current_tilemap.colour_depth == 0)
		stv2_current_tilemap.scrollx /= 2;
	if(stv2_current_tilemap.colour_depth == 2 || stv2_current_tilemap.colour_depth == 3)
		stv2_current_tilemap.scrollx*=2;
	if(stv2_current_tilemap.colour_depth == 4)
		stv2_current_tilemap.scrollx*=4;

	gfxdatalow = gfxdata + stv2_current_tilemap.bitmap_map * 0x20000;
	gfxdata+=(
	(stv2_current_tilemap.scrollx & (xlinesize-1)) +
	((stv2_current_tilemap.scrolly & (ysize-1)) * (xlinesize)) +
	(stv2_current_tilemap.bitmap_map * 0x20000)
	);
	gfxdatahigh = gfxdatalow + xlinesize*ysize;

//  popmessage("%04x %04x",stv2_current_tilemap.scrollx,stv2_current_tilemap.scrolly);

	/*Enable fading bit*/
	if(stv2_current_tilemap.fade_control & 1)
	{
		/*Select fading bit*/
		pal_color_offset += ((stv2_current_tilemap.fade_control & 2) ? (2*2048) : (2048));
	}

	stv2_current_tilemap.bitmap_palette_number+=stv2_current_tilemap.colour_ram_address_offset;
	stv2_current_tilemap.bitmap_palette_number&=7;//safety check

	switch(stv2_current_tilemap.colour_depth)
	{
		/*Palette Format*/
		case 0:
			for (ycnt = 0; ycnt <ysize;ycnt++)
			{
				for (xcnt = 0; xcnt <xsize;xcnt+=2)
				{
					tw = stv_vdp2_window_process(xcnt+1,ycnt);
					if(tw == 0)
					{
						t_pen = (((gfxdata[0] & 0x0f) >> 0) != 0) ? (1) : (0);
						if(stv2_current_tilemap.transparency == STV_TRANSPARENCY_NONE) t_pen = 1;
						if(t_pen)
						{
							if ( stv2_current_tilemap.colour_calculation_enabled == 0 )
								*BITMAP_ADDR16(bitmap, ycnt, xcnt+1) = machine->pens[((gfxdata[0] & 0x0f) >> 0) | (stv2_current_tilemap.bitmap_palette_number * 0x100) | pal_color_offset];
							else
								*BITMAP_ADDR16(bitmap, ycnt, xcnt+1) = alpha_blend_r16(*BITMAP_ADDR16(bitmap, ycnt, xcnt+1), machine->pens[((gfxdata[0] & 0x0f) >> 0) | (stv2_current_tilemap.bitmap_palette_number * 0x100) | pal_color_offset], stv2_current_tilemap.alpha);
						}
					}
					tw = stv_vdp2_window_process(xcnt,ycnt);
					if(tw == 0)
					{
						t_pen = (((gfxdata[0] & 0xf0) >> 4) != 0) ? (1) : (0);
						if(stv2_current_tilemap.transparency == STV_TRANSPARENCY_NONE) t_pen = 1;
						if(t_pen)
						{
							if ( stv2_current_tilemap.colour_calculation_enabled == 0 )
								*BITMAP_ADDR16(bitmap, ycnt, xcnt) = machine->pens[((gfxdata[0] & 0xf0) >> 4) | (stv2_current_tilemap.bitmap_palette_number * 0x100) | pal_color_offset];
							else
								*BITMAP_ADDR16(bitmap, ycnt, xcnt) = alpha_blend_r16(*BITMAP_ADDR16(bitmap, ycnt, xcnt), machine->pens[((gfxdata[0] & 0xf0) >> 4) | (stv2_current_tilemap.bitmap_palette_number * 0x100) | pal_color_offset], stv2_current_tilemap.alpha);
						}
					}
					gfxdata++;
					if ( gfxdata >= gfxdatahigh ) gfxdata = gfxdatalow;
				}
			}
			break;
		case 1:
			if ( stv2_current_tilemap.incx == 0x10000 && stv2_current_tilemap.incy == 0x10000 )
			{
				//int gfx_wraparound = -1;

				gfxdata += xlinesize*cliprect->min_y;

				for (ycnt = cliprect->min_y; ycnt <= cliprect->max_y; ycnt++)
				{
					for (xcnt = cliprect->min_x; xcnt <= cliprect->max_x; xcnt++)
					{
						int xs = xcnt & xsizemask;

						tw = stv_vdp2_window_process(xcnt,ycnt);
						if(tw == 0)
						{
							//60aee2c = $0013 at @605d838
							t_pen = ((gfxdata[xs] & 0xff) != 0) ? (1) : (0);
							if(stv2_current_tilemap.transparency == STV_TRANSPARENCY_NONE) t_pen = 1;
							if(t_pen)
							{
								if ( stv2_current_tilemap.colour_calculation_enabled == 0 )
									*BITMAP_ADDR16(bitmap, ycnt, xcnt) = machine->pens[(gfxdata[xs] & 0xff) | (stv2_current_tilemap.bitmap_palette_number * 0x100) | pal_color_offset];
								else
									*BITMAP_ADDR16(bitmap, ycnt, xcnt) = alpha_blend_r16(*BITMAP_ADDR16(bitmap, ycnt, xcnt), machine->pens[(gfxdata[xs] & 0xff) | (stv2_current_tilemap.bitmap_palette_number * 0x100) | pal_color_offset], stv2_current_tilemap.alpha);
							}
						}
						if ( (gfxdata + xs) >= gfxdatahigh )
						{
							//gfx_wraparound = (ycnt << 16) | xcnt;
							gfxdata = gfxdatalow - xs;
						}
					}
					if ( (gfxdata + xlinesize) < gfxdatahigh )
					{
						gfxdata += xlinesize;
					}
					else
					{
						gfxdata = gfxdatalow + ((gfxdata + xlinesize) - gfxdatahigh);
					}
				}

			}
			else
			{
				int xx, xs, yy=0;
				for (ycnt = cliprect->min_y; ycnt <= cliprect->max_y; yy+=stv2_current_tilemap.incy, ycnt++ )
				{
					gfxdata += xlinesize*(yy>>16);
					yy &= 0xffff;

					destline = BITMAP_ADDR16(bitmap, ycnt, 0);
					xx = 0;
					for (xcnt = cliprect->min_x; xcnt <= cliprect->max_x; xx+=stv2_current_tilemap.incx, xcnt++)
					{
						xs = xx >> 16;
						tw = stv_vdp2_window_process(xcnt,ycnt);
						if(tw == 0)
						{
							t_pen = ((gfxdata[xs] & 0xff) != 0) ? 1 : 0;
							if(stv2_current_tilemap.transparency == STV_TRANSPARENCY_NONE) t_pen = 1;
							if(t_pen)
							{
								if ( stv2_current_tilemap.colour_calculation_enabled == 0 )
									*BITMAP_ADDR16(bitmap, ycnt, xcnt) = machine->pens[(gfxdata[xs] & 0xff) | (stv2_current_tilemap.bitmap_palette_number * 0x100) | pal_color_offset];
								else
									*BITMAP_ADDR16(bitmap, ycnt, xcnt) = alpha_blend_r16(*BITMAP_ADDR16(bitmap, ycnt, xcnt), machine->pens[(gfxdata[xs] & 0xff) | (stv2_current_tilemap.bitmap_palette_number * 0x100) | pal_color_offset], stv2_current_tilemap.alpha);
							}
						}

						if ( (gfxdata + xs) >= gfxdatahigh ) gfxdata = gfxdatalow;

					}
				}
			}
			break;
		case 2:
			for (ycnt = 0; ycnt <ysize;ycnt++)
			{
				for (xcnt = 0; xcnt <xsize;xcnt++)
				{
					tw = stv_vdp2_window_process(xcnt,ycnt);
					if(tw == 0)
					{
						t_pen = ((((gfxdata[0] & 0x07) * 0x100) | (gfxdata[1] & 0xff)) != 0) ? (1) : (0);
						if(stv2_current_tilemap.transparency == STV_TRANSPARENCY_NONE) t_pen = 1;
						if(t_pen)
						{
							if ( stv2_current_tilemap.colour_calculation_enabled == 0 )
								*BITMAP_ADDR16(bitmap, ycnt, xcnt) = machine->pens[((gfxdata[0] & 0x07) * 0x100) | (gfxdata[1] & 0xff) | pal_color_offset];
							else
								*BITMAP_ADDR16(bitmap, ycnt, xcnt) = alpha_blend_r16(*BITMAP_ADDR16(bitmap, ycnt, xcnt), machine->pens[((gfxdata[0] & 0x07) * 0x100) | (gfxdata[1] & 0xff) | pal_color_offset], stv2_current_tilemap.alpha);
						}
					}

					gfxdata+=2;
					if ( gfxdata >= gfxdatahigh ) gfxdata = gfxdatalow;
				}
			}
			break;
		/*RGB format*/
		/*
        M                     L
        S                     S
        B                     B
        --------BBBBBGGGGGRRRRR
        */
		case 3:
			if ( stv2_current_tilemap.incx == 0x10000 && stv2_current_tilemap.incy == 0x10000 )
			{
				/* adjust for cliprect */
				gfxdata += xlinesize*cliprect->min_y;

				for (ycnt = cliprect->min_y; ycnt <= cliprect->max_y; ycnt++)
				{
					destline = BITMAP_ADDR16(bitmap, ycnt, 0);

					for (xcnt = cliprect->min_x; xcnt <= cliprect->max_x; xcnt++)
					{
						int r,g,b;
						int xs = xcnt & xsizemask;

						t_pen = ((gfxdata[2*xs] & 0x80) >> 7) || (stv2_current_tilemap.transparency == STV_TRANSPARENCY_NONE);
						if (!t_pen) continue;
						b = ((gfxdata[2*xs] & 0x7c) >> 2);
						g = ((gfxdata[2*xs] & 0x03) << 3) | ((gfxdata[2*xs+1] & 0xe0) >> 5);
						r = ((gfxdata[2*xs+1] & 0x1f));
						if(stv2_current_tilemap.fade_control & 1)
							stv_vdp2_compute_color_offset_RGB555(&r,&g,&b,stv2_current_tilemap.fade_control & 2);
						tw = stv_vdp2_window_process(xcnt,ycnt);
						if(tw == 0)
						{
							if ( stv2_current_tilemap.colour_calculation_enabled == 0 )
								destline[xcnt] = b | g << 5 | r << 10;
							else
								destline[xcnt] = alpha_blend_r16( destline[xcnt], b | g << 5 | r << 10, stv2_current_tilemap.alpha );
						}

						if ( (gfxdata + 2*xs) >= gfxdatahigh ) gfxdata = gfxdatalow;
					}

					gfxdata += xlinesize;
					if ( gfxdata >= gfxdatahigh ) gfxdata = gfxdatalow + (gfxdata - gfxdatahigh);
				}

			}
			else
			{
				int xx, xs, yy=0;

				for (ycnt = cliprect->min_y; ycnt <= cliprect->max_y; yy+=stv2_current_tilemap.incy, ycnt++ )
				{
					gfxdata += xlinesize*(yy>>16);
					yy &= 0xffff;

					destline = BITMAP_ADDR16(bitmap, ycnt, 0);
					xx = 0;
					for (xcnt = cliprect->min_x; xcnt <= cliprect->max_x; xx+=stv2_current_tilemap.incx, xcnt++)
					{
						int r,g,b;

						xs = xx >> 16;
						t_pen = ((gfxdata[2*xs] & 0x80) >> 7);
						if(stv2_current_tilemap.transparency == STV_TRANSPARENCY_NONE) t_pen = 1;
						b = ((gfxdata[2*xs] & 0x7c) >> 2);
						g = ((gfxdata[2*xs] & 0x03) << 3) | ((gfxdata[2*xs+1] & 0xe0) >> 5);
						r = ((gfxdata[2*xs+1] & 0x1f));
						if(stv2_current_tilemap.fade_control & 1)
							stv_vdp2_compute_color_offset_RGB555(&r,&g,&b,stv2_current_tilemap.fade_control & 2);
						tw = stv_vdp2_window_process(xcnt,ycnt);
						if(tw == 0)
						{
							if(t_pen)
							{
								if ( stv2_current_tilemap.colour_calculation_enabled == 1 )
									destline[xcnt] = alpha_blend_r16( destline[xcnt], b | g << 5 | r << 10, stv2_current_tilemap.alpha );
								else
									destline[xcnt] = b | g << 5 | r << 10;
							}
						}

						if ( (gfxdata + 2*xs) >= gfxdatahigh ) gfxdata = gfxdatalow;
					}
					/*Guess: myfairlady needs that the vertical resolution is doubled because it's using the double density mode.*/
					if(STV_VDP2_LSMD == 3) { gfxdata += xlinesize*(yy>>16); }
					else 				   { gfxdata += xlinesize; }
					if ( gfxdata >= gfxdatahigh ) gfxdata = gfxdatalow + (gfxdata - gfxdatahigh);
				}
			}
			break;
		/*
        M                              L
        S                              S
        B                              B
        --------BBBBBBBBGGGGGGGGRRRRRRRR
        */
		case 4:
			//popmessage("BITMAP type 4 enabled");
			for (ycnt = 0; ycnt <ysize;ycnt++)
			{
				destline = BITMAP_ADDR16(bitmap, ycnt, 0);

				for (xcnt = 0; xcnt <xsize;xcnt++)
				{
					int r,g,b;

					t_pen = ((gfxdata[0] & 0x80) >> 7);
					if(stv2_current_tilemap.transparency == STV_TRANSPARENCY_NONE) t_pen = 1;

					/*TODO: 8bpp*/
					b = (gfxdata[1] & 0xf8) >> 3;
					g = (gfxdata[2] & 0xf8) >> 3;
					r = (gfxdata[3] & 0xf8) >> 3;

					tw = stv_vdp2_window_process(xcnt,ycnt);
					if(tw == 0)
					{
						if(t_pen)
							destline[xcnt] = b | g << 5 | r << 10;
					}
					gfxdata+=4;
					/*This is not used for this type,see shanhigw Sunsoft logo*/
					//if ( gfxdata >= gfxdatahigh ) gfxdata = gfxdatalow;
				}
			}
			break;
	}
}

  /*---------------------------------------------------------------------------
   | Plane Size | Pattern Name Data Size | Character Size | Map Bits / Address |
   ----------------------------------------------------------------------------|
   |            |                        | 1 H x 1 V      | bits 6-0 * 0x02000 |
   |            | 1 word                 |-------------------------------------|
   |            |                        | 2 H x 2 V      | bits 8-0 * 0x00800 |
   | 1 H x 1 V  ---------------------------------------------------------------|
   |            |                        | 1 H x 1 V      | bits 5-0 * 0x04000 |
   |            | 2 words                |-------------------------------------|
   |            |                        | 2 H x 2 V      | bits 7-0 * 0x01000 |
   -----------------------------------------------------------------------------
   |            |                        | 1 H x 1 V      | bits 6-1 * 0x04000 |
   |            | 1 word                 |-------------------------------------|
   |            |                        | 2 H x 2 V      | bits 8-1 * 0x01000 |
   | 2 H x 1 V  ---------------------------------------------------------------|
   |            |                        | 1 H x 1 V      | bits 5-1 * 0x08000 |
   |            | 2 words                |-------------------------------------|
   |            |                        | 2 H x 2 V      | bits 7-1 * 0x02000 |
   -----------------------------------------------------------------------------
   |            |                        | 1 H x 1 V      | bits 6-2 * 0x08000 |
   |            | 1 word                 |-------------------------------------|
   |            |                        | 2 H x 2 V      | bits 8-2 * 0x02000 |
   | 2 H x 2 V  ---------------------------------------------------------------|
   |            |                        | 1 H x 1 V      | bits 5-2 * 0x10000 |
   |            | 2 words                |-------------------------------------|
   |            |                        | 2 H x 2 V      | bits 7-2 * 0x04000 |
   --the-highest-bit-is-ignored-if-vram-is-only-4mbits------------------------*/


/*
4.2 Sega's Cell / Character Pattern / Page / Plane / Map system, aka a rather annoying thing that makes optimizations hard
 (this is only for the normal tilemaps at the moment, i haven't even thought about the ROZ ones)

Tiles:

Cells are 8x8 gfx stored in video ram, they can be of various colour depths

Character Patterns can be 8x8 or 16x16 (1 hcell x 1 vcell or 2 hcell x 2 vcell)
  (a 16x16 character pattern is 4 8x8 cells put together)

A page is made up of 64x64 cells, thats 64x64 character patterns in 8x8 mode or 32x32 character patterns in 16x16 mode.
  64 * 8  = 512 (0x200)
  32 * 16 = 512 (0x200)
A page is _always_ 512 (0x200) pixels in each direction

in 1 word mode a 32*16 x 32*16 page is 0x0800 bytes
in 1 word mode a 64*8  x 64*8  page is 0x2000 bytes
in 2 word mode a 32*16 x 32*16 page is 0x1000 bytes
in 2 word mode a 64*8  x 64*8  page is 0x4000 bytes

either 1, 2 or 4 pages make each plane depending on the plane size register (per tilemap)
  therefore each plane is either
  64 * 8 * 1 x 64 * 8 * 1 (512 x 512)
  64 * 8 * 2 x 64 * 8 * 1 (1024 x 512)
  64 * 8 * 2 x 64 * 8 * 2 (1024 x 1024)

  32 * 16 * 1 x 32 * 16 * 1 (512 x 512)
  32 * 16 * 2 x 32 * 16 * 1 (1024 x 512)
  32 * 16 * 2 x 32 * 16 * 2 (1024 x 1024)

map is always enabled?
  map is a 2x2 arrangement of planes, all 4 of the planes can be the same.

*/

static void stv_vdp2_get_map_page( int x, int y, int *_map, int *_page )
{
	int page = 0;
	int map = 0;

	if ( stv2_current_tilemap.map_count == 4 )
	{
		if ( stv2_current_tilemap.tile_size == 0 )
		{
			if ( stv2_current_tilemap.plane_size & 1 )
			{
				page = ((x >> 6) & 1);
				map = (x >> 7) & 1;
			}
			else
			{
				map = (x >> 6) & 1;
			}

			if ( stv2_current_tilemap.plane_size & 2 )
			{
				page |= ((y >> (6-1)) & 2);
				map |= ((y >> (7-1)) & 2);
			}
			else
			{
				map |= ((y >> (6-1)) & 2);
			}
		}
		else
		{
			if ( stv2_current_tilemap.plane_size & 1 )
			{
				page = ((x >> 5) & 1);
				map = (x >> 6) & 1;
			}
			else
			{
				map = (x >> 5) & 1;
			}

			if ( stv2_current_tilemap.plane_size & 2 )
			{
				page |= ((y >> (5 - 1)) & 2);
				map |= ((y >> (6-1)) & 2);
			}
			else
			{
				map |= ((y >> (5-1)) & 2);
			}
		}
	}
	else //16
	{
		if ( stv2_current_tilemap.tile_size == 0 )
		{
			if ( stv2_current_tilemap.plane_size & 1 )
			{
				page = ((x >> 6) & 1);
				map = (x >> 7) & 3;
			}
			else
			{
				map = (x >> 6) & 3;
			}

			if ( stv2_current_tilemap.plane_size & 2 )
			{
				page |= ((y >> (6-1)) & 2);
				map |= ((y >> (7-2)) & 12);
			}
			else
			{
				map |= ((y >> (6-2)) & 12);
			}
		}
		else
		{
			if ( stv2_current_tilemap.plane_size & 1 )
			{
				page = ((x >> 5) & 1);
				map = (x >> 6) & 3;
			}
			else
			{
				map = (x >> 5) & 3;
			}

			if ( stv2_current_tilemap.plane_size & 2 )
			{
				page |= ((y >> (5 - 1)) & 2);
				map |= ((y >> (6-2)) & 12);
			}
			else
			{
				map |= ((y >> (5-2)) & 12);
			}
		}
	}
	*_page = page;
	*_map = map;
}

static void stv_vdp2_draw_basic_tilemap(running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect)
{
	/* hopefully this is easier to follow than it is efficient .. */

	/* I call character patterns tiles .. even if they represent up to 4 tiles */

	/* Page variables */
	int pgtiles_x, pgpixels_x;
	int pgtiles_y, pgpixels_y;
	int pgsize_bytes, pgsize_dwords;

	/* Plane Variables */
	int pltiles_x, plpixels_x;
	int pltiles_y, plpixels_y;
	int plsize_bytes, plsize_dwords;

	/* Map Variables */
	int mptiles_x, mppixels_x;
	int mptiles_y, mppixels_y;
	int mpsize_bytes, mpsize_dwords;

	/* work Variables */
	int i, x, y;
	int base[16];

	int scalex,scaley;
	int tilesizex, tilesizey;
	int drawypos, drawxpos;

	int tilecodemin = 0x10000000, tilecodemax = 0;

	if ( stv2_current_tilemap.incx == 0 || stv2_current_tilemap.incy == 0 ) return;

	if ( stv2_current_tilemap.colour_calculation_enabled == 1 )
	{
		if ( STV_VDP2_CCMD )
		{
			stv2_current_tilemap.transparency = STV_TRANSPARENCY_ADD_BLEND;
		}
		else
		{
			stv2_current_tilemap.transparency = STV_TRANSPARENCY_ALPHA;
		}
	}

	scalex = (INT32)((INT64)S64(0x100000000) / (INT64)stv2_current_tilemap.incx);
	scaley = (INT32)((INT64)S64(0x100000000) / (INT64)stv2_current_tilemap.incy);
	tilesizex = scalex * 8;
	tilesizey = scaley * 8;
	drawypos = drawxpos = 0;

	/* Calculate the Number of tiles for x / y directions of each page (actually these will be the same */
	/* (2-stv2_current_tilemap.tile_size) << 5) */
	pgtiles_x = ((2-stv2_current_tilemap.tile_size) << 5); // 64 (8x8 mode) or 32 (16x16 mode)
	pgtiles_y = ((2-stv2_current_tilemap.tile_size) << 5); // 64 (8x8 mode) or 32 (16x16 mode)

	/* Calculate the Page Size in BYTES */
	/* 64 * 64 * (1 * 2) = 0x2000 bytes
       32 * 32 * (1 * 2) = 0x0800 bytes
       64 * 64 * (2 * 2) = 0x4000 bytes
       32 * 32 * (2 * 2) = 0x1000 bytes */

	pgsize_bytes = (pgtiles_x * pgtiles_y) * ((2-stv2_current_tilemap.pattern_data_size)*2);

   /*---------------------------------------------------------------------------
   | Plane Size | Pattern Name Data Size | Character Size | Map Bits / Address |
   ----------------------------------------------------------------------------|
   |            |                        | 1 H x 1 V      | bits 6-0 * 0x02000 |
   |            | 1 word                 |-------------------------------------|
   |            |                        | 2 H x 2 V      | bits 8-0 * 0x00800 |
   | 1 H x 1 V  ---------------------------------------------------------------|
   |            |                        | 1 H x 1 V      | bits 5-0 * 0x04000 |
   |            | 2 words                |-------------------------------------|
   |            |                        | 2 H x 2 V      | bits 7-0 * 0x01000 |
   ---------------------------------------------------------------------------*/


	/* Page Dimensions are always 0x200 pixes (512x512) */
	pgpixels_x = 0x200;
	pgpixels_y = 0x200;

	/* Work out the Plane Size in tiles and Plane Dimensions (pixels) */
	switch (stv2_current_tilemap.plane_size & 3)
	{
		case 0: // 1 page * 1 page
			pltiles_x  = pgtiles_x;
			plpixels_x = pgpixels_x;
			pltiles_y  = pgtiles_y;
			plpixels_y = pgpixels_y;
			break;

		case 1: // 2 pages * 1 page
			pltiles_x  = pgtiles_x * 2;
			plpixels_x = pgpixels_x * 2;
			pltiles_y  = pgtiles_y;
			plpixels_y = pgpixels_y;
			break;

		case 3: // 2 pages * 2 pages
			pltiles_x  = pgtiles_x * 2;
			plpixels_x = pgpixels_x * 2;
			pltiles_y  = pgtiles_y * 2;
			plpixels_y = pgpixels_y * 2;
			break;

		default:
			// illegal
			pltiles_x  = pgtiles_x;
			plpixels_x = pgpixels_x;
			pltiles_y  = pgtiles_y * 2;
			plpixels_y = pgpixels_y * 2;
		break;
	}

	/* Plane Size in BYTES */
	/* still the same as before
       (64 * 1) * (64 * 1) * (1 * 2) = 0x02000 bytes
       (32 * 1) * (32 * 1) * (1 * 2) = 0x00800 bytes
       (64 * 1) * (64 * 1) * (2 * 2) = 0x04000 bytes
       (32 * 1) * (32 * 1) * (2 * 2) = 0x01000 bytes
       changed
       (64 * 2) * (64 * 1) * (1 * 2) = 0x04000 bytes
       (32 * 2) * (32 * 1) * (1 * 2) = 0x01000 bytes
       (64 * 2) * (64 * 1) * (2 * 2) = 0x08000 bytes
       (32 * 2) * (32 * 1) * (2 * 2) = 0x02000 bytes
       changed
       (64 * 2) * (64 * 1) * (1 * 2) = 0x08000 bytes
       (32 * 2) * (32 * 1) * (1 * 2) = 0x02000 bytes
       (64 * 2) * (64 * 1) * (2 * 2) = 0x10000 bytes
       (32 * 2) * (32 * 1) * (2 * 2) = 0x04000 bytes
    */

	plsize_bytes = (pltiles_x * pltiles_y) * ((2-stv2_current_tilemap.pattern_data_size)*2);

   /*---------------------------------------------------------------------------
   | Plane Size | Pattern Name Data Size | Character Size | Map Bits / Address |
   -----------------------------------------------------------------------------
   | 1 H x 1 V   see above, nothing has changed                                |
   -----------------------------------------------------------------------------
   |            |                        | 1 H x 1 V      | bits 6-1 * 0x04000 |
   |            | 1 word                 |-------------------------------------|
   |            |                        | 2 H x 2 V      | bits 8-1 * 0x01000 |
   | 2 H x 1 V  ---------------------------------------------------------------|
   |            |                        | 1 H x 1 V      | bits 5-1 * 0x08000 |
   |            | 2 words                |-------------------------------------|
   |            |                        | 2 H x 2 V      | bits 7-1 * 0x02000 |
   -----------------------------------------------------------------------------
   |            |                        | 1 H x 1 V      | bits 6-2 * 0x08000 |
   |            | 1 word                 |-------------------------------------|
   |            |                        | 2 H x 2 V      | bits 8-2 * 0x02000 |
   | 2 H x 2 V  ---------------------------------------------------------------|
   |            |                        | 1 H x 1 V      | bits 5-2 * 0x10000 |
   |            | 2 words                |-------------------------------------|
   |            |                        | 2 H x 2 V      | bits 7-2 * 0x04000 |
   --the-highest-bit-is-ignored-if-vram-is-only-4mbits------------------------*/


	/* Work out the Map Sizes in tiles, Map Dimensions */
	/* maps are always enabled? */
	if ( stv2_current_tilemap.map_count == 4 )
	{
		mptiles_x = pltiles_x * 2;
		mptiles_y = pltiles_y * 2;
		mppixels_x = plpixels_x * 2;
		mppixels_y = plpixels_y * 2;
	}
	else
	{
		mptiles_x = pltiles_x * 4;
		mptiles_y = pltiles_y * 4;
		mppixels_x = plpixels_x * 4;
		mppixels_y = plpixels_y * 4;
	}

	/* Map Size in BYTES */
	mpsize_bytes = (mptiles_x * mptiles_y) * ((2-stv2_current_tilemap.pattern_data_size)*2);


   /*-----------------------------------------------------------------------------------------------------------
   |            |                        | 1 H x 1 V      | bits 6-1 (upper mask 0x07f) (0x1ff >> 2) * 0x04000 |
   |            | 1 word                 |---------------------------------------------------------------------|
   |            |                        | 2 H x 2 V      | bits 8-1 (upper mask 0x1ff) (0x1ff >> 0) * 0x01000 |
   | 2 H x 1 V  -----------------------------------------------------------------------------------------------|
   |            |                        | 1 H x 1 V      | bits 5-1 (upper mask 0x03f) (0x1ff >> 3) * 0x08000 |
   |            | 2 words                |---------------------------------------------------------------------|
   |            |                        | 2 H x 2 V      | bits 7-1 (upper mask 0x0ff) (0x1ff >> 1) * 0x02000 |
   -------------------------------------------------------------------------------------------------------------
    lower mask = ~stv2_current_tilemap.plane_size
   -----------------------------------------------------------------------------------------------------------*/

	/* Precalculate bases from MAP registers */
	for (i = 0; i < stv2_current_tilemap.map_count; i++)
	{
		static const int shifttable[4] = {0,1,2,2};

		int uppermask, uppermaskshift;

		uppermaskshift = (1-stv2_current_tilemap.pattern_data_size) | ((1-stv2_current_tilemap.tile_size)<<1);
		uppermask = 0x1ff >> uppermaskshift;

		base[i] = ((stv2_current_tilemap.map_offset[i] & uppermask) >> shifttable[stv2_current_tilemap.plane_size]) * plsize_bytes;

		base[i] &= 0x7ffff; /* shienryu needs this for the text layer, is there a problem elsewhere or is it just right without the ram cart */

		base[i] = base[i] / 4; // convert bytes to DWORDS
	}

	/* other bits */
	//stv2_current_tilemap.trans_enabled = stv2_current_tilemap.trans_enabled ? STV_TRANSPARENCY_NONE : STV_TRANSPARENCY_PEN;
	stv2_current_tilemap.scrollx &= mppixels_x-1;
	stv2_current_tilemap.scrolly &= mppixels_y-1;

	pgsize_dwords = pgsize_bytes /4;
	plsize_dwords = plsize_bytes /4;
	mpsize_dwords = mpsize_bytes /4;

//  if (stv2_current_tilemap.layer_name==3) popmessage ("well this is a bit  %08x", stv2_current_tilemap.map_offset[0]);
//  if (stv2_current_tilemap.layer_name==3) popmessage ("well this is a bit  %08x %08x %08x %08x", stv2_current_tilemap.plane_size, pgtiles_x, pltiles_x, mptiles_x);

	if (!stv2_current_tilemap.enabled) return; // stop right now if its disabled ...

	/* most things we need (or don't need) to work out are now worked out */

	for (y = 0; y<mptiles_y; y++) {
		int ypageoffs;
		int page, map, newbase, offs, data;
		int tilecode, flipyx, pal, gfx = 0;

		map = 0 ; page = 0 ;
		if ( y == 0 )
		{
			int drawyposinc = tilesizey*(stv2_current_tilemap.tile_size ? 2 : 1);
			drawypos = -(stv2_current_tilemap.scrolly*scaley);
			while( ((drawypos + drawyposinc) >> 16) < cliprect->min_y )
			{
				drawypos += drawyposinc;
				y++;
			}
			mptiles_y += y;
		}
		else
		{
			drawypos += tilesizey*(stv2_current_tilemap.tile_size ? 2 : 1);
		}
		if ((drawypos >> 16) > cliprect->max_y) break;

		ypageoffs = y & (pgtiles_y-1);

		for (x = 0; x<mptiles_x; x++) {
			int xpageoffs;
			int tilecodespacing = 1;

			if ( x == 0 )
			{
				int drawxposinc = tilesizex*(stv2_current_tilemap.tile_size ? 2 : 1);
				drawxpos = -(stv2_current_tilemap.scrollx*scalex);
				while( ((drawxpos + drawxposinc) >> 16) < cliprect->min_x )
				{
					drawxpos += drawxposinc;
					x++;
				}
				mptiles_x += x;
			}
			else
			{
				drawxpos+=tilesizex*(stv2_current_tilemap.tile_size ? 2 : 1);
			}
			if ( (drawxpos >> 16) > cliprect->max_x ) break;

			xpageoffs = x & (pgtiles_x-1);

			stv_vdp2_get_map_page(x,y,&map,&page);

			newbase = base[map] + page * pgsize_dwords;
			offs = (ypageoffs * pgtiles_x) + xpageoffs;

/* GET THE TILE INFO ... */
			/* 1 word per tile mode with supplement bits */
			if (stv2_current_tilemap.pattern_data_size ==1)
			{

				data = stv_vdp2_vram[newbase + offs/2];
				data = (offs&1) ? (data & 0x0000ffff) : ((data & 0xffff0000) >> 16);

				/* Supplement Mode 12 bits, no flip */
				if (stv2_current_tilemap.character_number_supplement == 1)
				{
/* no flip */		flipyx   = 0;
/* 8x8 */			if (stv2_current_tilemap.tile_size==0) tilecode = (data & 0x0fff) + ( (stv2_current_tilemap.supplementary_character_bits&0x1c) << 10);
/* 16x16 */ 		else tilecode = ((data & 0x0fff) << 2) + (stv2_current_tilemap.supplementary_character_bits&0x03) + ((stv2_current_tilemap.supplementary_character_bits&0x10) << 10);
				}
				/* Supplement Mode 10 bits, with flip */
				else
				{
/* flip bits */ 	flipyx   = (data & 0x0c00) >> 10;
/* 8x8 */			if (stv2_current_tilemap.tile_size==0) tilecode = (data & 0x03ff) +  ( (stv2_current_tilemap.supplementary_character_bits) << 10);
/* 16x16 */			else tilecode = ((data & 0x03ff) <<2) +  (stv2_current_tilemap.supplementary_character_bits&0x03) + ((stv2_current_tilemap.supplementary_character_bits&0x1c) << 10);
				}

/*>16cols*/		if (stv2_current_tilemap.colour_depth != 0) pal = ((data & 0x7000)>>8);
/*16 cols*/		else pal = ((data & 0xf000)>>12) +( (stv2_current_tilemap.supplementary_palette_bits) << 4);

			}
			/* 2 words per tile, no supplement bits */
			else
			{

				data = stv_vdp2_vram[newbase + offs];
				tilecode = (data & 0x00007fff);
				pal   = (data &    0x007f0000)>>16;
	//          specialc = (data & 0x10000000)>>28;
				flipyx   = (data & 0xc0000000)>>30;
			}
/* WE'VE GOT THE TILE INFO ... */

			if ( tilecode < tilecodemin ) tilecodemin = tilecode;
			if ( tilecode > tilecodemax ) tilecodemax = tilecode;

/* DECODE ANY TILES WE NEED TO DECODE */

			pal += stv2_current_tilemap.colour_ram_address_offset<< 4; // bios uses this ..

			/*Enable fading bit*/
			if(stv2_current_tilemap.fade_control & 1)
			{
				/*Select fading bit*/
				pal += ((stv2_current_tilemap.fade_control & 2) ? (0x100) : (0x80));
			}

			if (stv2_current_tilemap.colour_depth == 1)
			{
				gfx = 2;
				pal = pal >>4;
				tilecode &=0x7fff;
				if (tilecode == 0x7fff) tilecode--;	/* prevents crash but unsure what should happen; wrapping? */
				tilecodespacing = 2;
			}
			else if (stv2_current_tilemap.colour_depth == 0)
			{
				gfx = 0;
				tilecode &=0x7fff;
				tilecodespacing = 1;
			}
/* TILES ARE NOW DECODED */

/* DRAW! */
			if(stv2_current_tilemap.incx != 0x10000 ||
			   stv2_current_tilemap.incy != 0x10000 ||
			   stv2_current_tilemap.transparency == STV_TRANSPARENCY_ADD_BLEND )
			{
#define SCR_TILESIZE_X			(((drawxpos + tilesizex) >> 16) - (drawxpos >> 16))
#define SCR_TILESIZE_X1(startx)	(((drawxpos + (startx) + tilesizex) >> 16) - ((drawxpos + (startx))>>16))
#define SCR_TILESIZE_Y			(((drawypos + tilesizey) >> 16) - (drawypos >> 16))
#define SCR_TILESIZE_Y1(starty)	(((drawypos + (starty) + tilesizey) >> 16) - ((drawypos + (starty))>>16))
				if (stv2_current_tilemap.tile_size==1)
				{
					/* normal */
					stv_vdp2_drawgfxzoom(bitmap,cliprect,machine->gfx[gfx],tilecode+(0+(flipyx&1)+(flipyx&2))*tilecodespacing,pal,flipyx&1,flipyx&2,drawxpos >> 16, drawypos >> 16,stv2_current_tilemap.transparency,0,scalex,scaley,SCR_TILESIZE_X, SCR_TILESIZE_Y,stv2_current_tilemap.alpha);
					stv_vdp2_drawgfxzoom(bitmap,cliprect,machine->gfx[gfx],tilecode+(1-(flipyx&1)+(flipyx&2))*tilecodespacing,pal,flipyx&1,flipyx&2,(drawxpos+tilesizex) >> 16,drawypos >> 16,stv2_current_tilemap.transparency,0,scalex,scaley,SCR_TILESIZE_X1(tilesizex), SCR_TILESIZE_Y,stv2_current_tilemap.alpha);
					stv_vdp2_drawgfxzoom(bitmap,cliprect,machine->gfx[gfx],tilecode+(2+(flipyx&1)-(flipyx&2))*tilecodespacing,pal,flipyx&1,flipyx&2,drawxpos >> 16,(drawypos+tilesizey) >> 16,stv2_current_tilemap.transparency,0,scalex,scaley,SCR_TILESIZE_X, SCR_TILESIZE_Y1(tilesizey),stv2_current_tilemap.alpha);
					stv_vdp2_drawgfxzoom(bitmap,cliprect,machine->gfx[gfx],tilecode+(3-(flipyx&1)-(flipyx&2))*tilecodespacing,pal,flipyx&1,flipyx&2,(drawxpos+tilesizex)>> 16,(drawypos+tilesizey) >> 16,stv2_current_tilemap.transparency,0,scalex,scaley,SCR_TILESIZE_X1(tilesizex), SCR_TILESIZE_Y1(tilesizey),stv2_current_tilemap.alpha);

				}
				else
				{
					stv_vdp2_drawgfxzoom(bitmap,cliprect,machine->gfx[gfx],tilecode,pal,flipyx&1,flipyx&2, drawxpos >> 16, drawypos >> 16,stv2_current_tilemap.transparency,0,scalex,scaley,SCR_TILESIZE_X,SCR_TILESIZE_Y,stv2_current_tilemap.alpha);
				}
			}
			else
			{
				int olddrawxpos, olddrawypos;
				olddrawxpos = drawxpos; drawxpos >>= 16;
				olddrawypos = drawypos; drawypos >>= 16;
				if (stv2_current_tilemap.tile_size==1)
				{
					if ( stv2_current_tilemap.colour_depth == 3 )
					{
						/* normal */
						stv_vdp2_drawgfx_rgb555(bitmap,cliprect,tilecode+(0+(flipyx&1)+(flipyx&2))*4,flipyx&1,flipyx&2,drawxpos, drawypos,stv2_current_tilemap.transparency,stv2_current_tilemap.alpha);
						stv_vdp2_drawgfx_rgb555(bitmap,cliprect,tilecode+(1-(flipyx&1)+(flipyx&2))*4,flipyx&1,flipyx&2,drawxpos+8,drawypos,stv2_current_tilemap.transparency,stv2_current_tilemap.alpha);
						stv_vdp2_drawgfx_rgb555(bitmap,cliprect,tilecode+(2+(flipyx&1)-(flipyx&2))*4,flipyx&1,flipyx&2,drawxpos,drawypos+8,stv2_current_tilemap.transparency,stv2_current_tilemap.alpha);
						stv_vdp2_drawgfx_rgb555(bitmap,cliprect,tilecode+(3-(flipyx&1)-(flipyx&2))*4,flipyx&1,flipyx&2,drawxpos+8,drawypos+8,stv2_current_tilemap.transparency,stv2_current_tilemap.alpha);

					}
					else if (stv2_current_tilemap.transparency == STV_TRANSPARENCY_ALPHA)
					{
						/* alpha */
						drawgfx_alpha(bitmap,cliprect,machine->gfx[gfx],tilecode+(0+(flipyx&1)+(flipyx&2))*tilecodespacing,pal,flipyx&1,flipyx&2,drawxpos, drawypos,0,stv2_current_tilemap.alpha);
						drawgfx_alpha(bitmap,cliprect,machine->gfx[gfx],tilecode+(1-(flipyx&1)+(flipyx&2))*tilecodespacing,pal,flipyx&1,flipyx&2,drawxpos+8,drawypos,0,stv2_current_tilemap.alpha);
						drawgfx_alpha(bitmap,cliprect,machine->gfx[gfx],tilecode+(2+(flipyx&1)-(flipyx&2))*tilecodespacing,pal,flipyx&1,flipyx&2,drawxpos,drawypos+8,0,stv2_current_tilemap.alpha);
						drawgfx_alpha(bitmap,cliprect,machine->gfx[gfx],tilecode+(3-(flipyx&1)-(flipyx&2))*tilecodespacing,pal,flipyx&1,flipyx&2,drawxpos+8,drawypos+8,0,stv2_current_tilemap.alpha);

					}
					else
					{
						/* normal */
						drawgfx_transpen(bitmap,cliprect,machine->gfx[gfx],tilecode+(0+(flipyx&1)+(flipyx&2))*tilecodespacing,pal,flipyx&1,flipyx&2,drawxpos, drawypos,(stv2_current_tilemap.transparency==STV_TRANSPARENCY_PEN)?0:-1);
						drawgfx_transpen(bitmap,cliprect,machine->gfx[gfx],tilecode+(1-(flipyx&1)+(flipyx&2))*tilecodespacing,pal,flipyx&1,flipyx&2,drawxpos+8,drawypos,(stv2_current_tilemap.transparency==STV_TRANSPARENCY_PEN)?0:-1);
						drawgfx_transpen(bitmap,cliprect,machine->gfx[gfx],tilecode+(2+(flipyx&1)-(flipyx&2))*tilecodespacing,pal,flipyx&1,flipyx&2,drawxpos,drawypos+8,(stv2_current_tilemap.transparency==STV_TRANSPARENCY_PEN)?0:-1);
						drawgfx_transpen(bitmap,cliprect,machine->gfx[gfx],tilecode+(3-(flipyx&1)-(flipyx&2))*tilecodespacing,pal,flipyx&1,flipyx&2,drawxpos+8,drawypos+8,(stv2_current_tilemap.transparency==STV_TRANSPARENCY_PEN)?0:-1);

					}
				}
				else
				{
					if ( stv2_current_tilemap.colour_depth == 3 )
					{
						stv_vdp2_drawgfx_rgb555(bitmap,cliprect,tilecode,flipyx&1,flipyx&2,drawxpos,drawypos,stv2_current_tilemap.transparency,stv2_current_tilemap.alpha);
					}
					else
					{
						if (stv2_current_tilemap.transparency == STV_TRANSPARENCY_ALPHA)
							drawgfx_alpha(bitmap,cliprect,machine->gfx[gfx],tilecode,pal,flipyx&1,flipyx&2, drawxpos, drawypos,0,stv2_current_tilemap.alpha);
						else
							drawgfx_transpen(bitmap,cliprect,machine->gfx[gfx],tilecode,pal,flipyx&1,flipyx&2, drawxpos, drawypos,(stv2_current_tilemap.transparency==STV_TRANSPARENCY_PEN)?0:-1);
					}
				}
				drawxpos = olddrawxpos;
				drawypos = olddrawypos;
			}
/* DRAWN?! */

		}
	}
	if ( stv2_current_tilemap.layer_name & 0x80 )
	{
		static const int shifttable[4] = {0,1,2,2};
		int uppermask, uppermaskshift;
		int mapsize;
		uppermaskshift = (1-stv2_current_tilemap.pattern_data_size) | ((1-stv2_current_tilemap.tile_size)<<1);
		uppermask = 0x1ff >> uppermaskshift;

		if ( LOG_VDP2 )
		{
			logerror( "Layer RBG%d, size %d x %d\n", stv2_current_tilemap.layer_name & 0x7f, cliprect->max_x + 1, cliprect->max_y + 1 );
			logerror( "Tiles: min %08X, max %08X\n", tilecodemin, tilecodemax );
			logerror( "MAP size in dwords %08X\n", mpsize_dwords );
			for (i = 0; i < stv2_current_tilemap.map_count; i++)
			{
				logerror( "Map register %d: base %08X\n", stv2_current_tilemap.map_offset[i], base[i] );
			}
		}

		// store map information
		stv_vdp2_layer_data_placement.map_offset_min = 0x7fffffff;
		stv_vdp2_layer_data_placement.map_offset_max = 0x00000000;
		for (i = 0; i < stv2_current_tilemap.map_count; i++)
		{
			if ( base[i] < stv_vdp2_layer_data_placement.map_offset_min )
				stv_vdp2_layer_data_placement.map_offset_min = base[i];
			if ( base[i] > stv_vdp2_layer_data_placement.map_offset_max )
				stv_vdp2_layer_data_placement.map_offset_max = base[i];
		}


		mapsize = ((1 & uppermask) >> shifttable[stv2_current_tilemap.plane_size]) * plsize_bytes -
				  ((0 & uppermask) >> shifttable[stv2_current_tilemap.plane_size]) * plsize_bytes;
		mapsize /= 4;

		stv_vdp2_layer_data_placement.map_offset_max += mapsize;

		stv_vdp2_layer_data_placement.tile_offset_min = tilecodemin * 0x20 / 4;
		stv_vdp2_layer_data_placement.tile_offset_max = (tilecodemax + 1) * 0x20 / 4;
	}

}

#define STV_VDP2_READ_VERTICAL_LINESCROLL( _val, _address ) \
	{ \
		_val = stv_vdp2_vram[ _address ]; \
		_val &= 0x07ffff00; \
		if ( _val & 0x04000000 ) _val |= 0xf8000000; \
	}


static void stv_vdp2_check_tilemap_with_linescroll(running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect)
{
	rectangle mycliprect;
	int cur_line = cliprect->min_y;
	int address;
	int active_functions = 0;
	INT32 scroll_values[3], prev_scroll_values[3];
	int i;
	int scroll_values_equal;
	int lines;
	INT16 scrollx, scrolly;
	INT32 incx;
	int linescroll_enable, vertical_linescroll_enable, linezoom_enable;
	int vertical_linescroll_index = -1;

	// read original scroll values
	scrollx = stv2_current_tilemap.scrollx;
	scrolly = stv2_current_tilemap.scrolly;
	incx = stv2_current_tilemap.incx;

	// prepare linescroll flags
	linescroll_enable = stv2_current_tilemap.linescroll_enable;
	stv2_current_tilemap.linescroll_enable = 0;
	vertical_linescroll_enable = stv2_current_tilemap.vertical_linescroll_enable;
	stv2_current_tilemap.vertical_linescroll_enable = 0;
	linezoom_enable = stv2_current_tilemap.linezoom_enable;
	stv2_current_tilemap.linezoom_enable = 0;

	// prepare working clipping rectangle
	memcpy( &mycliprect, cliprect, sizeof(rectangle) );

	// calculate the number of active functions
	if ( linescroll_enable ) active_functions++;
	if ( vertical_linescroll_enable )
	{
		vertical_linescroll_index = active_functions;
		active_functions++;
	}
	if ( linezoom_enable ) active_functions++;

	// address of data table
	address = stv2_current_tilemap.linescroll_table_address + active_functions*4*cliprect->min_y;

	// get the first scroll values
	for ( i = 0; i < active_functions; i++ )
	{
		if ( i == vertical_linescroll_index )
		{
			STV_VDP2_READ_VERTICAL_LINESCROLL( prev_scroll_values[i], (address / 4) + i );
			prev_scroll_values[i] -= (cur_line * stv2_current_tilemap.incy);
		}
		else
		{
			prev_scroll_values[i] = stv_vdp2_vram[ (address / 4) + i ];
		}
	}

	while( cur_line <= cliprect->max_y )
	{
		lines = 0;
		do
		{
			// update address
			address += active_functions*4;

			// update lines count
			lines += stv2_current_tilemap.linescroll_interval;

			// get scroll values
			for ( i = 0; i < active_functions; i++ )
			{
				if ( i == vertical_linescroll_index )
				{
					STV_VDP2_READ_VERTICAL_LINESCROLL( scroll_values[i], (address/4) + i );
					scroll_values[i] -= (cur_line + lines) * stv2_current_tilemap.incy;
				}
				else
				{
					scroll_values[i] = stv_vdp2_vram[ (address / 4) + i ];
				}
			}

			// compare scroll values
			scroll_values_equal = 1;
			for ( i = 0; i < active_functions; i++ )
			{
				scroll_values_equal &= (scroll_values[i] == prev_scroll_values[i]);
			}
		} while( scroll_values_equal && ((cur_line + lines) <= cliprect->max_y) );

		// determined how many lines can be drawn
		// prepare clipping rectangle
		mycliprect.min_y = cur_line;
		mycliprect.max_y = cur_line + lines - 1;

		// prepare scroll values
		i = 0;
		// linescroll
		if ( linescroll_enable )
		{
			prev_scroll_values[i] &= 0x07ffff00;
			if ( prev_scroll_values[i] & 0x04000000 ) prev_scroll_values[i] |= 0xf8000000;
			stv2_current_tilemap.scrollx = scrollx + (prev_scroll_values[i] >> 16);
			i++;
		}
		// vertical line scroll
		if ( vertical_linescroll_enable )
		{
			stv2_current_tilemap.scrolly = scrolly + (prev_scroll_values[i] >> 16);
			i++;
		}

		// linezooom
		if ( linezoom_enable )
		{
			prev_scroll_values[i] &= 0x0007ff00;
			if ( prev_scroll_values[i] & 0x00040000 ) prev_scroll_values[i] |= 0xfff80000;
			stv2_current_tilemap.incx = prev_scroll_values[i];
			i++;
		}

		if ( LOG_VDP2 ) logerror( "Linescroll: y < %d, %d >, scrollx = %d, scrolly = %d, incx = %f\n", mycliprect.min_y, mycliprect.max_y, stv2_current_tilemap.scrollx, stv2_current_tilemap.scrolly, (float)stv2_current_tilemap.incx/65536.0 );
		// render current tilemap portion
		stv_vdp2_check_tilemap(machine, bitmap, &mycliprect );

		// update parameters for next iteration
		memcpy( prev_scroll_values, scroll_values, sizeof(scroll_values));
		cur_line += lines;
	}
}

static void stv_vdp2_check_tilemap(running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect)
{
	/* the idea is here we check the tilemap capabilities / whats enabled and call an appropriate tilemap drawing routine, or
      at the very list throw up a few errors if the tilemaps want to do something we don't support yet */

	int window_applied = 0;
	rectangle mycliprect;
	mycliprect.min_x = cliprect->min_x;
	mycliprect.max_x = cliprect->max_x;
	mycliprect.min_y = cliprect->min_y;
	mycliprect.max_y = cliprect->max_y;

	if ( stv2_current_tilemap.linescroll_enable ||
		 stv2_current_tilemap.vertical_linescroll_enable ||
		 stv2_current_tilemap.linezoom_enable )
	{
		stv_vdp2_check_tilemap_with_linescroll(machine, bitmap, cliprect);
		return;
	}

	window_applied = stv_vdp2_apply_window_on_layer(&mycliprect);

	if (stv2_current_tilemap.bitmap_enable) // this layer is a bitmap
	{
		/*elandore doesn't like current cliprect code,will be worked on...*/
		if ( window_applied && stv2_current_tilemap.colour_depth != 0)
			stv2_current_tilemap.window_control = 0;
		stv_vdp2_draw_basic_bitmap(machine, bitmap, &mycliprect);
	}
	else
	{

		stv_vdp2_draw_basic_tilemap(machine, bitmap, &mycliprect);

		if((stv2_current_tilemap.window_control & 6) != 0 && VDP2_ERR(1))
		{
			VDP2_CHK(1);
			mame_printf_debug("Window control enabled on a tilemap plane = %02x\n",stv2_current_tilemap.window_control);
		}
	}

	if((STV_VDP2_MZCTL & 0x1f) != 0 && VDP2_ERR(2))
	{
		VDP2_CHK(2);
		mame_printf_debug("Mosaic control enabled = %04x\n",STV_VDP2_MZCTL);
	}
}

static void stv_vdp2_copy_roz_bitmap(bitmap_t *bitmap,
									 bitmap_t *roz_bitmap,
									 const rectangle *cliprect,
									 int iRP,
									 int planesizex,
									 int planesizey,
									 int planerenderedsizex,
									 int planerenderedsizey)
{
	INT32 xsp, ysp, xp, yp, dx, dy, x, y, xs, ys, dxs, dys;
	INT32 vcnt, hcnt;
	INT32 kx, ky;
	INT8  use_coeff_table, coeff_table_mode, coeff_table_size, coeff_table_shift;
	INT8  screen_over_process;
	UINT8 vcnt_shift, hcnt_shift;
	UINT8 coeff_msb;
	UINT32 *coeff_table_base, coeff_table_offset;
	INT32 coeff_table_val;
	UINT32 address;
	UINT16 *line;
	UINT16 pix;
	UINT32 coeff_line_color_screen_data;
	INT32 clipxmask = 0, clipymask = 0;


	if((STV_VDP2_LSMD & 3) == 3)
	{
		vcnt_shift = 1;
	}
	else
	{
		vcnt_shift = 0;
	}

	switch( STV_VDP2_HRES & 7 )
	{
		case 2: /*640*/
		case 3: /*704*/
		case 6:
		case 7:
			hcnt_shift = 1;
			break;
		default:
			hcnt_shift = 0;
			break;
	}

	planesizex--;
	planesizey--;
	planerenderedsizex--;
	planerenderedsizey--;

	kx = RP.kx;
	ky = RP.ky;

	use_coeff_table = coeff_table_mode = coeff_table_size = coeff_table_shift = 0;
	coeff_table_offset = 0;
	coeff_table_val = 0;
	coeff_table_base = NULL;

	if ( LOG_ROZ == 1 ) logerror( "Rendering RBG with parameter %s\n", iRP == 1 ? "A" : "B" );
	if ( LOG_ROZ == 1 ) logerror( "RPMD (parameter mode) = %x\n", STV_VDP2_RPMD );
	if ( LOG_ROZ == 1 ) logerror( "RPRCTL (parameter read control) = %04x\n", STV_VDP2_RPRCTL );
	if ( LOG_ROZ == 1 ) logerror( "KTCTL (coefficient table control) = %04x\n", STV_VDP2_KTCTL );
	if ( LOG_ROZ == 1 ) logerror( "KTAOF (coefficient table address offset) = %04x\n", STV_VDP2_KTAOF );
	if ( LOG_ROZ == 1 ) logerror( "RAOVR (screen-over process) = %x\n", STV_VDP2_RAOVR );
	if ( iRP == 1 )
	{
		use_coeff_table = STV_VDP2_RAKTE;
		if ( use_coeff_table == 1 )
		{
			coeff_table_mode = STV_VDP2_RAKMD;
			coeff_table_size = STV_VDP2_RAKDBS;
			coeff_table_offset = STV_VDP2_RAKTAOS;
		}
		screen_over_process = STV_VDP2_RAOVR;
	}
	else
	{
		use_coeff_table = STV_VDP2_RBKTE;
		if ( use_coeff_table == 1 )
		{
			coeff_table_mode = STV_VDP2_RBKMD;
			coeff_table_size = STV_VDP2_RBKDBS;
			coeff_table_offset = STV_VDP2_RBKTAOS;
		}
		screen_over_process = STV_VDP2_RBOVR;
	}
	if ( use_coeff_table )
	{
		if ( STV_VDP2_CRKTE == 0 )
		{
			coeff_table_base = stv_vdp2_vram;
		}
		else
		{
			coeff_table_base = stv_vdp2_cram;
		}
		if ( coeff_table_size == 0 )
		{
			coeff_table_offset = (coeff_table_offset & 0x0003) * 0x40000;
			coeff_table_shift = 2;
		}
		else
		{
			coeff_table_offset = (coeff_table_offset & 0x0007) * 0x20000;
			coeff_table_shift = 1;
		}
	}

	if ( stv2_current_tilemap.colour_calculation_enabled == 1 )
	{
		if ( STV_VDP2_CCMD )
		{
			stv2_current_tilemap.transparency = STV_TRANSPARENCY_ADD_BLEND;
		}
		else
		{
			stv2_current_tilemap.transparency = STV_TRANSPARENCY_ALPHA;
		}
	}

	/* clipping */
	switch( screen_over_process )
	{
		case 0:
			/* repeated */
			clipxmask = clipymask = 0;
			break;
		case 1:
			/* screen over pattern, not supported */
			clipxmask = clipymask = 0;
			break;
		case 2:
			/* outside display area, scroll screen is transparent */
			clipxmask = ~planesizex;
			clipymask = ~planesizey;
			break;
		case 3:
			/* display area is 512x512, outside is transparent */
			clipxmask = ~511;
			clipymask = ~511;
			break;
	}

	//dx  = (RP.A * RP.dx) + (RP.B * RP.dy);
	//dy  = (RP.D * RP.dx) + (RP.E * RP.dy);
	dx = mul_fixed32( RP.A, RP.dx ) + mul_fixed32( RP.B, RP.dy );
	dy = mul_fixed32( RP.D, RP.dx ) + mul_fixed32( RP.E, RP.dy );

	//xp  = RP.A * ( RP.px - RP.cx ) + RP.B * ( RP.py - RP.cy ) + RP.C * ( RP.pz - RP.cz ) + RP.cx + RP.mx;
	//yp  = RP.D * ( RP.px - RP.cx ) + RP.E * ( RP.py - RP.cy ) + RP.F * ( RP.pz - RP.cz ) + RP.cy + RP.my;
	xp = mul_fixed32( RP.A, RP.px - RP.cx ) + mul_fixed32( RP.B, RP.py - RP.cy ) + mul_fixed32( RP.C, RP.pz - RP.cz ) + RP.cx + RP.mx;
	yp = mul_fixed32( RP.D, RP.px - RP.cx ) + mul_fixed32( RP.E, RP.py - RP.cy ) + mul_fixed32( RP.F, RP.pz - RP.cz ) + RP.cy + RP.my;

	for (vcnt = cliprect->min_y; vcnt <= cliprect->max_y; vcnt++ )
	{
		/*xsp = RP.A * ( ( RP.xst + RP.dxst * (vcnt << 16) ) - RP.px ) +
              RP.B * ( ( RP.yst + RP.dyst * (vcnt << 16) ) - RP.py ) +
              RP.C * ( RP.zst - RP.pz);
        ysp = RP.D * ( ( RP.xst + RP.dxst * (vcnt << 16) ) - RP.px ) +
              RP.E * ( ( RP.yst + RP.dyst * (vcnt << 16) ) - RP.py ) +
              RP.F * ( RP.zst - RP.pz );*/
		xsp = mul_fixed32( RP.A, RP.xst + mul_fixed32( RP.dxst, vcnt << (16 - vcnt_shift)) - RP.px ) +
			  mul_fixed32( RP.B, RP.yst + mul_fixed32( RP.dyst, vcnt << (16 - vcnt_shift)) - RP.py ) +
			  mul_fixed32( RP.C, RP.zst - RP.pz );
		ysp = mul_fixed32( RP.D, RP.xst + mul_fixed32( RP.dxst, vcnt << (16 - vcnt_shift)) - RP.px ) +
			  mul_fixed32( RP.E, RP.yst + mul_fixed32( RP.dyst, vcnt << (16 - vcnt_shift)) - RP.py ) +
			  mul_fixed32( RP.F, RP.zst - RP.pz );
		//xp  = RP.A * ( RP.px - RP.cx ) + RP.B * ( RP.py - RP.cy ) + RP.C * ( RP.pz - RP.cz ) + RP.cx + RP.mx;
		//yp  = RP.D * ( RP.px - RP.cx ) + RP.E * ( RP.py - RP.cy ) + RP.F * ( RP.pz - RP.cz ) + RP.cy + RP.my;
		//dx  = (RP.A * RP.dx) + (RP.B * RP.dy);
		//dy  = (RP.D * RP.dx) + (RP.E * RP.dy);

		line = BITMAP_ADDR16(bitmap, vcnt, 0);

		if ( !use_coeff_table || RP.dkax == 0 )
		{
			if ( use_coeff_table )
			{
				switch( coeff_table_size )
				{
					case 0:
						address = coeff_table_offset + ((RP.kast + RP.dkast*(vcnt>>vcnt_shift)) >> 16) * 4;
						coeff_table_val = coeff_table_base[ address / 4 ];
						coeff_line_color_screen_data = (coeff_table_val & 0x7f000000) >> 24;
						coeff_msb = (coeff_table_val & 0x80000000) > 0;
						if ( coeff_table_val & 0x00800000 )
						{
							coeff_table_val |= 0xff000000;
						}
						else
						{
							coeff_table_val &= 0x007fffff;
						}
						break;
					case 1:
						address = coeff_table_offset + ((RP.kast + RP.dkast*(vcnt>>vcnt_shift)) >> 16) * 2;
						coeff_table_val = coeff_table_base[ address / 4 ];
						if ( (address & 2) == 0 )
						{
							coeff_table_val >>= 16;
						}
						coeff_table_val &= 0xffff;
						coeff_line_color_screen_data = 0;
						coeff_msb = (coeff_table_val & 0x8000) > 0;
						if ( coeff_table_val & 0x4000 )
						{
							coeff_table_val |= 0xffff8000;
						}
						else
						{
							coeff_table_val &= 0x3fff;
						}
						coeff_table_val <<= 6; /* to form 16.16 fixed point val */
						break;
					default:
						coeff_msb = 1;
						break;
				}
				if ( coeff_msb ) continue;

				switch( coeff_table_mode )
				{
					case 0:
						kx = ky = coeff_table_val;
						break;
					case 1:
						kx = coeff_table_val;
						break;
					case 2:
						ky = coeff_table_val;
						break;
					case 3:
						xp = coeff_table_val;
						break;
				}
			}

			//x = RP.kx * ( xsp + dx * (hcnt << 16)) + xp;
			//y = RP.ky * ( ysp + dy * (hcnt << 16)) + yp;
			xs = mul_fixed32( kx, xsp ) + xp;
			ys = mul_fixed32( ky, ysp ) + yp;
			dxs = mul_fixed32( kx, mul_fixed32( dx, 1 << (16-hcnt_shift)));
			dys = mul_fixed32( ky, mul_fixed32( dy, 1 << (16-hcnt_shift)));

			for (hcnt = cliprect->min_x; hcnt <= cliprect->max_x; xs+=dxs, ys+=dys, hcnt++ )
			{
				x = xs >> 16;
				y = ys >> 16;

				if ( x & clipxmask || y & clipymask ) continue;
				pix = *BITMAP_ADDR16(roz_bitmap, y & planerenderedsizey, x & planerenderedsizex);
				if(stv2_current_tilemap.fade_control & 1)
					stv_vdp2_compute_color_offset_RGB555_UINT16(&pix,stv2_current_tilemap.fade_control & 2);
				switch( stv2_current_tilemap.transparency )
				{
					case STV_TRANSPARENCY_PEN:
						if ( pix != 0x0000 ) line[hcnt] = pix;
						break;
					case STV_TRANSPARENCY_NONE:
						line[hcnt] = pix;
						break;
					case STV_TRANSPARENCY_ALPHA:
						if ( pix != 0x000 ) line[hcnt] = alpha_blend_r16( line[hcnt], pix, stv2_current_tilemap.alpha );
						break;
					case STV_TRANSPARENCY_ADD_BLEND:
						if ( pix != 0x0000 ) line[hcnt] = stv_add_blend( line[hcnt], pix );
						break;
				}

			}
		}
		else
		{
			for (hcnt = cliprect->min_x; hcnt <= cliprect->max_x; hcnt++ )
			{
				switch( coeff_table_size )
				{
					case 0:
						address = coeff_table_offset + ((RP.kast + RP.dkast*(vcnt>>vcnt_shift) + RP.dkax*hcnt) >> 16) * 4;
						coeff_table_val = coeff_table_base[ address / 4 ];
						coeff_line_color_screen_data = (coeff_table_val & 0x7f000000) >> 24;
						coeff_msb = (coeff_table_val & 0x80000000) > 0;
						if ( coeff_table_val & 0x00800000 )
						{
							coeff_table_val |= 0xff000000;
						}
						else
						{
							coeff_table_val &= 0x007fffff;
						}
						break;
					case 1:
						address = coeff_table_offset + ((RP.kast + RP.dkast*(vcnt>>vcnt_shift) + RP.dkax*hcnt) >> 16) * 2;
						coeff_table_val = coeff_table_base[ address / 4 ];
						if ( (address & 2) == 0 )
						{
							coeff_table_val >>= 16;
						}
						coeff_table_val &= 0xffff;
						coeff_line_color_screen_data = 0;
						coeff_msb = (coeff_table_val & 0x8000) > 0;
						if ( coeff_table_val & 0x4000 )
						{
							coeff_table_val |= 0xffff8000;
						}
						else
						{
							coeff_table_val &= 0x3fff;
						}
						coeff_table_val <<= 6; /* to form 16.16 fixed point val */
						break;
					default:
						coeff_msb = 1;
						break;
				}
				if ( coeff_msb ) continue;
				switch( coeff_table_mode )
				{
					case 0:
						kx = ky = coeff_table_val;
						break;
					case 1:
						kx = coeff_table_val;
						break;
					case 2:
						ky = coeff_table_val;
						break;
					case 3:
						xp = coeff_table_val;
						break;
				}

				//x = RP.kx * ( xsp + dx * (hcnt << 16)) + xp;
				//y = RP.ky * ( ysp + dy * (hcnt << 16)) + yp;
				x = mul_fixed32( kx, xsp + mul_fixed32( dx, (hcnt>>hcnt_shift) << 16 ) ) + xp;
				y = mul_fixed32( ky, ysp + mul_fixed32( dy, (hcnt>>hcnt_shift) << 16 ) ) + yp;

				x >>= 16;
				y >>= 16;

				if ( x & clipxmask || y & clipymask ) continue;

				pix = *BITMAP_ADDR16(roz_bitmap, y & planerenderedsizey, x & planerenderedsizex);
				if(stv2_current_tilemap.fade_control & 1)
					stv_vdp2_compute_color_offset_RGB555_UINT16(&pix,stv2_current_tilemap.fade_control & 2);
				switch( stv2_current_tilemap.transparency )
				{
					case STV_TRANSPARENCY_PEN:
						if ( pix != 0x0000 ) line[hcnt] = pix;
						break;
					case STV_TRANSPARENCY_NONE:
						line[hcnt] = pix;
						break;
					case STV_TRANSPARENCY_ALPHA:
						if ( pix != 0x000 ) line[hcnt] = alpha_blend_r16( line[hcnt], pix, stv2_current_tilemap.alpha );
						break;
					case STV_TRANSPARENCY_ADD_BLEND:
						if ( pix != 0x0000 ) line[hcnt] = stv_add_blend( line[hcnt], pix );
						break;
				}
			}
		}
	}
}

static void stv_vdp2_draw_NBG0(running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect)
{
	/*
       Colours           : 16, 256, 2048, 32768, 16770000
       Char Size         : 1x1 cells, 2x2 cells
       Pattern Data Size : 1 word, 2 words
       Plane Layouts     : 1 x 1, 2 x 1, 2 x 2
       Planes            : 4
       Bitmap            : Possible
       Bitmap Sizes      : 512 x 256, 512 x 512, 1024 x 256, 1024 x 512
       Scale             : 0.25 x - 256 x
       Rotation          : No
       Linescroll        : Yes
       Column Scroll     : Yes
       Mosaic            : Yes
    */
	stv2_current_tilemap.enabled = STV_VDP2_N0ON;

//  if (!stv2_current_tilemap.enabled) return; // stop right now if its disabled ...

	//stv2_current_tilemap.trans_enabled = STV_VDP2_N0TPON;
	if ( STV_VDP2_N0CCEN )
	{
		stv2_current_tilemap.colour_calculation_enabled = 1;
		stv2_current_tilemap.alpha = ((UINT16)(0x1f-STV_VDP2_N0CCRT)*0xff)/0x1f;
	}
	else
	{
		stv2_current_tilemap.colour_calculation_enabled = 0;
	}
	if ( STV_VDP2_N0TPON == 0 )
	{
		stv2_current_tilemap.transparency = STV_TRANSPARENCY_PEN;
	}
	else
	{
		stv2_current_tilemap.transparency = STV_TRANSPARENCY_NONE;
	}
	stv2_current_tilemap.colour_depth = STV_VDP2_N0CHCN;
	stv2_current_tilemap.tile_size = STV_VDP2_N0CHSZ;
	stv2_current_tilemap.bitmap_enable = STV_VDP2_N0BMEN;
	stv2_current_tilemap.bitmap_size = STV_VDP2_N0BMSZ;
	stv2_current_tilemap.bitmap_palette_number = STV_VDP2_N0BMP;
	stv2_current_tilemap.bitmap_map = STV_VDP2_N0MP_;
	stv2_current_tilemap.map_offset[0] = STV_VDP2_N0MPA | (STV_VDP2_N0MP_ << 6);
	stv2_current_tilemap.map_offset[1] = STV_VDP2_N0MPB | (STV_VDP2_N0MP_ << 6);
	stv2_current_tilemap.map_offset[2] = STV_VDP2_N0MPC | (STV_VDP2_N0MP_ << 6);
	stv2_current_tilemap.map_offset[3] = STV_VDP2_N0MPD | (STV_VDP2_N0MP_ << 6);
	stv2_current_tilemap.map_count = 4;

	stv2_current_tilemap.pattern_data_size = STV_VDP2_N0PNB;
	stv2_current_tilemap.character_number_supplement = STV_VDP2_N0CNSM;
	stv2_current_tilemap.special_priority_register = STV_VDP2_N0SPR;
	stv2_current_tilemap.special_colour_control_register = STV_VDP2_PNCN0;
	stv2_current_tilemap.supplementary_palette_bits = STV_VDP2_N0SPLT;
	stv2_current_tilemap.supplementary_character_bits = STV_VDP2_N0SPCN;

	stv2_current_tilemap.scrollx = STV_VDP2_SCXIN0;
	stv2_current_tilemap.scrolly = STV_VDP2_SCYIN0;
	stv2_current_tilemap.incx = STV_VDP2_ZMXN0;
	stv2_current_tilemap.incy = STV_VDP2_ZMYN0;

	stv2_current_tilemap.linescroll_enable = STV_VDP2_N0LSCX;
	stv2_current_tilemap.linescroll_interval = (((STV_VDP2_LSMD & 3) == 2) ? (2) : (1)) << (STV_VDP2_N0LSS);
	stv2_current_tilemap.linescroll_table_address = ((STV_VDP2_LSTA0U << 16) | STV_VDP2_LSTA0L) * 2;
	stv2_current_tilemap.vertical_linescroll_enable = STV_VDP2_N0LSCY;
	stv2_current_tilemap.linezoom_enable = STV_VDP2_N0LZMX;

	stv2_current_tilemap.plane_size = STV_VDP2_N0PLSZ;
	stv2_current_tilemap.colour_ram_address_offset = STV_VDP2_N0CAOS;
	stv2_current_tilemap.fade_control = (STV_VDP2_N0COEN * 1) | (STV_VDP2_N0COSL * 2);
	stv_vdp2_check_fade_control_for_layer();
	stv2_current_tilemap.window_control = (STV_VDP2_N0LOG * 0x01) |
										  (STV_VDP2_N0W0E * 0x02) |
										  (STV_VDP2_N0W1E * 0x04) |
										  (STV_VDP2_N0SWE * 0x08) |
										  (STV_VDP2_N0W0A * 0x10) |
										  (STV_VDP2_N0W1A * 0x20) |
										  (STV_VDP2_N0SWA * 0x40);

	stv2_current_tilemap.layer_name=0;

	if ( stv2_current_tilemap.enabled )
	{
		stv2_current_tilemap.enabled = stv_vdp2_check_vram_cycle_pattern_registers( STV_VDP2_CP_NBG0_PNMDR, STV_VDP2_CP_NBG0_CPDR, stv2_current_tilemap.bitmap_enable );
	}

	stv_vdp2_check_tilemap(machine, bitmap, cliprect);
}

static void stv_vdp2_draw_NBG1(running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect)
{
	/*
       Colours           : 16, 256, 2048, 32768
       Char Size         : 1x1 cells, 2x2 cells
       Pattern Data Size : 1 word, 2 words
       Plane Layouts     : 1 x 1, 2 x 1, 2 x 2
       Planes            : 4
       Bitmap            : Possible
       Bitmap Sizes      : 512 x 256, 512 x 512, 1024 x 256, 1024 x 512
       Scale             : 0.25 x - 256 x
       Rotation          : No
       Linescroll        : Yes
       Column Scroll     : Yes
       Mosaic            : Yes
    */
	stv2_current_tilemap.enabled = STV_VDP2_N1ON;

//  if (!stv2_current_tilemap.enabled) return; // stop right now if its disabled ...

	//stv2_current_tilemap.trans_enabled = STV_VDP2_N1TPON;
	if ( STV_VDP2_N1CCEN )
	{
		stv2_current_tilemap.colour_calculation_enabled = 1;
		stv2_current_tilemap.alpha = ((UINT16)(0x1f-STV_VDP2_N1CCRT)*0xff)/0x1f;
	}
	else
	{
		stv2_current_tilemap.colour_calculation_enabled = 0;
	}
	if ( STV_VDP2_N1TPON == 0 )
	{
		stv2_current_tilemap.transparency = STV_TRANSPARENCY_PEN;
	}
	else
	{
		stv2_current_tilemap.transparency = STV_TRANSPARENCY_NONE;
	}
	stv2_current_tilemap.colour_depth = STV_VDP2_N1CHCN;
	stv2_current_tilemap.tile_size = STV_VDP2_N1CHSZ;
	stv2_current_tilemap.bitmap_enable = STV_VDP2_N1BMEN;
	stv2_current_tilemap.bitmap_size = STV_VDP2_N1BMSZ;
	stv2_current_tilemap.bitmap_palette_number = STV_VDP2_N1BMP;
	stv2_current_tilemap.bitmap_map = STV_VDP2_N1MP_;
	stv2_current_tilemap.map_offset[0] = STV_VDP2_N1MPA | (STV_VDP2_N1MP_ << 6);
	stv2_current_tilemap.map_offset[1] = STV_VDP2_N1MPB | (STV_VDP2_N1MP_ << 6);
	stv2_current_tilemap.map_offset[2] = STV_VDP2_N1MPC | (STV_VDP2_N1MP_ << 6);
	stv2_current_tilemap.map_offset[3] = STV_VDP2_N1MPD | (STV_VDP2_N1MP_ << 6);
	stv2_current_tilemap.map_count = 4;

	stv2_current_tilemap.pattern_data_size = STV_VDP2_N1PNB;
	stv2_current_tilemap.character_number_supplement = STV_VDP2_N1CNSM;
	stv2_current_tilemap.special_priority_register = STV_VDP2_N1SPR;
	stv2_current_tilemap.special_colour_control_register = STV_VDP2_PNCN1;
	stv2_current_tilemap.supplementary_palette_bits = STV_VDP2_N1SPLT;
	stv2_current_tilemap.supplementary_character_bits = STV_VDP2_N1SPCN;

	stv2_current_tilemap.scrollx = STV_VDP2_SCXIN1;
	stv2_current_tilemap.scrolly = STV_VDP2_SCYIN1;
	stv2_current_tilemap.incx = STV_VDP2_ZMXN1;
	stv2_current_tilemap.incy = STV_VDP2_ZMYN1;

	stv2_current_tilemap.linescroll_enable = STV_VDP2_N1LSCX;
	stv2_current_tilemap.linescroll_interval = (((STV_VDP2_LSMD & 3) == 2) ? (2) : (1)) << (STV_VDP2_N1LSS);
	stv2_current_tilemap.linescroll_table_address = ((STV_VDP2_LSTA1U << 16) | STV_VDP2_LSTA1L) * 2;
	stv2_current_tilemap.vertical_linescroll_enable = STV_VDP2_N1LSCY;
	stv2_current_tilemap.linezoom_enable = STV_VDP2_N1LZMX;

	stv2_current_tilemap.plane_size = STV_VDP2_N1PLSZ;
	stv2_current_tilemap.colour_ram_address_offset = STV_VDP2_N1CAOS;
	stv2_current_tilemap.fade_control = (STV_VDP2_N1COEN * 1) | (STV_VDP2_N1COSL * 2);
	stv_vdp2_check_fade_control_for_layer();
	stv2_current_tilemap.window_control = (STV_VDP2_N1LOG * 0x01) |
										  (STV_VDP2_N1W0E * 0x02) |
										  (STV_VDP2_N1W1E * 0x04) |
										  (STV_VDP2_N1SWE * 0x08) |
										  (STV_VDP2_N1W0A * 0x10) |
										  (STV_VDP2_N1W1A * 0x20) |
										  (STV_VDP2_N1SWA * 0x40);

	stv2_current_tilemap.layer_name=1;

	if ( stv2_current_tilemap.enabled )
	{
		stv2_current_tilemap.enabled = stv_vdp2_check_vram_cycle_pattern_registers( STV_VDP2_CP_NBG1_PNMDR, STV_VDP2_CP_NBG1_CPDR, stv2_current_tilemap.bitmap_enable );
	}

	stv_vdp2_check_tilemap(machine, bitmap, cliprect);
}

static void stv_vdp2_draw_NBG2(running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect)
{
	/*
       NBG2 is the first of the 2 more basic tilemaps, it has exactly the same capabilities as NBG3

       Colours           : 16, 256
       Char Size         : 1x1 cells, 2x2 cells
       Pattern Data Size : 1 word, 2 words
       Plane Layouts     : 1 x 1, 2 x 1, 2 x 2
       Planes            : 4
       Bitmap            : No
       Bitmap Sizes      : N/A
       Scale             : No
       Rotation          : No
       Linescroll        : No
       Column Scroll     : No
       Mosaic            : Yes
    */

	stv2_current_tilemap.enabled = STV_VDP2_N2ON;

	/* these modes for N0 disable this layer */
	if (STV_VDP2_N0CHCN == 0x03) stv2_current_tilemap.enabled = 0;
	if (STV_VDP2_N0CHCN == 0x04) stv2_current_tilemap.enabled = 0;

//  if (!stv2_current_tilemap.enabled) return; // stop right now if its disabled ...

	//stv2_current_tilemap.trans_enabled = STV_VDP2_N2TPON;
	if ( STV_VDP2_N2CCEN )
	{
		stv2_current_tilemap.colour_calculation_enabled = 1;
		stv2_current_tilemap.alpha = ((UINT16)(0x1f-STV_VDP2_N2CCRT)*0xff)/0x1f;
	}
	else
	{
		stv2_current_tilemap.colour_calculation_enabled = 0;
	}
	if ( STV_VDP2_N2TPON == 0 )
	{
		stv2_current_tilemap.transparency = STV_TRANSPARENCY_PEN;
	}
	else
	{
		stv2_current_tilemap.transparency = STV_TRANSPARENCY_NONE;
	}
	stv2_current_tilemap.colour_depth = STV_VDP2_N2CHCN;
	stv2_current_tilemap.tile_size = STV_VDP2_N2CHSZ;
	/* this layer can't be a bitmap,so ignore these registers*/
	stv2_current_tilemap.bitmap_enable = 0;
	stv2_current_tilemap.bitmap_size = 0;
	stv2_current_tilemap.bitmap_palette_number = 0;
	stv2_current_tilemap.bitmap_map = 0;
	stv2_current_tilemap.map_offset[0] = STV_VDP2_N2MPA | (STV_VDP2_N2MP_ << 6);
	stv2_current_tilemap.map_offset[1] = STV_VDP2_N2MPB | (STV_VDP2_N2MP_ << 6);
	stv2_current_tilemap.map_offset[2] = STV_VDP2_N2MPC | (STV_VDP2_N2MP_ << 6);
	stv2_current_tilemap.map_offset[3] = STV_VDP2_N2MPD | (STV_VDP2_N2MP_ << 6);
	stv2_current_tilemap.map_count = 4;

	stv2_current_tilemap.pattern_data_size = STV_VDP2_N2PNB;
	stv2_current_tilemap.character_number_supplement = STV_VDP2_N2CNSM;
	stv2_current_tilemap.special_priority_register = STV_VDP2_N2SPR;
	stv2_current_tilemap.special_colour_control_register = STV_VDP2_PNCN2;
	stv2_current_tilemap.supplementary_palette_bits = STV_VDP2_N2SPLT;
	stv2_current_tilemap.supplementary_character_bits = STV_VDP2_N2SPCN;

	stv2_current_tilemap.scrollx = STV_VDP2_SCXN2;
	stv2_current_tilemap.scrolly = STV_VDP2_SCYN2;
	/*This layer can't be scaled*/
	stv2_current_tilemap.incx = 0x10000;
	stv2_current_tilemap.incy = 0x10000;

	stv2_current_tilemap.linescroll_enable = 0;
	stv2_current_tilemap.linescroll_interval = 0;
	stv2_current_tilemap.linescroll_table_address = 0;
	stv2_current_tilemap.vertical_linescroll_enable = 0;
	stv2_current_tilemap.linezoom_enable = 0;

	stv2_current_tilemap.colour_ram_address_offset = STV_VDP2_N2CAOS;
	stv2_current_tilemap.fade_control = (STV_VDP2_N2COEN * 1) | (STV_VDP2_N2COSL * 2);
	stv_vdp2_check_fade_control_for_layer();
	stv2_current_tilemap.window_control = (STV_VDP2_N2LOG * 0x01) |
										  (STV_VDP2_N2W0E * 0x02) |
										  (STV_VDP2_N2W1E * 0x04) |
										  (STV_VDP2_N2SWE * 0x08) |
										  (STV_VDP2_N2W0A * 0x10) |
										  (STV_VDP2_N2W1A * 0x20) |
										  (STV_VDP2_N2SWA * 0x40);

	stv2_current_tilemap.layer_name=2;

	stv2_current_tilemap.plane_size = STV_VDP2_N2PLSZ;

	if ( stv2_current_tilemap.enabled )
	{
		stv2_current_tilemap.enabled = stv_vdp2_check_vram_cycle_pattern_registers( STV_VDP2_CP_NBG2_PNMDR, STV_VDP2_CP_NBG2_CPDR, stv2_current_tilemap.bitmap_enable );
	}

	stv_vdp2_check_tilemap(machine, bitmap, cliprect);
}

static void stv_vdp2_draw_NBG3(running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect)
{
	/*
       NBG3 is the second of the 2 more basic tilemaps, it has exactly the same capabilities as NBG2

       Colours           : 16, 256
       Char Size         : 1x1 cells, 2x2 cells
       Pattern Data Size : 1 word, 2 words
       Plane Layouts     : 1 x 1, 2 x 1, 2 x 2
       Planes            : 4
       Bitmap            : No
       Bitmap Sizes      : N/A
       Scale             : No
       Rotation          : No
       Linescroll        : No
       Column Scroll     : No
       Mosaic            : Yes
    */

	stv2_current_tilemap.enabled = STV_VDP2_N3ON;

//  if (!stv2_current_tilemap.enabled) return; // stop right now if its disabled ...

	/* these modes for N1 disable this layer */
	if (STV_VDP2_N1CHCN == 0x03) stv2_current_tilemap.enabled = 0;
	if (STV_VDP2_N1CHCN == 0x04) stv2_current_tilemap.enabled = 0;

	//stv2_current_tilemap.trans_enabled = STV_VDP2_N3TPON;
	if ( STV_VDP2_N3CCEN )
	{
		stv2_current_tilemap.colour_calculation_enabled = 1;
		stv2_current_tilemap.alpha = ((UINT16)(0x1f-STV_VDP2_N3CCRT)*0xff)/0x1f;
	}
	else
	{
		stv2_current_tilemap.colour_calculation_enabled = 0;
	}
	if ( STV_VDP2_N3TPON == 0 )
	{
		stv2_current_tilemap.transparency = STV_TRANSPARENCY_PEN;
	}
	else
	{
		stv2_current_tilemap.transparency = STV_TRANSPARENCY_NONE;
	}
	stv2_current_tilemap.colour_depth = STV_VDP2_N3CHCN;
	stv2_current_tilemap.tile_size = STV_VDP2_N3CHSZ;
	/* this layer can't be a bitmap,so ignore these registers*/
	stv2_current_tilemap.bitmap_enable = 0;
	stv2_current_tilemap.bitmap_size = 0;
	stv2_current_tilemap.bitmap_palette_number = 0;
	stv2_current_tilemap.bitmap_map = 0;
	stv2_current_tilemap.map_offset[0] = STV_VDP2_N3MPA | (STV_VDP2_N3MP_ << 6);
	stv2_current_tilemap.map_offset[1] = STV_VDP2_N3MPB | (STV_VDP2_N3MP_ << 6);
	stv2_current_tilemap.map_offset[2] = STV_VDP2_N3MPC | (STV_VDP2_N3MP_ << 6);
	stv2_current_tilemap.map_offset[3] = STV_VDP2_N3MPD | (STV_VDP2_N3MP_ << 6);
	stv2_current_tilemap.map_count = 4;

	stv2_current_tilemap.pattern_data_size = STV_VDP2_N3PNB;
	stv2_current_tilemap.character_number_supplement = STV_VDP2_N3CNSM;
	stv2_current_tilemap.special_priority_register = STV_VDP2_N3SPR;
	stv2_current_tilemap.special_colour_control_register = STV_VDP2_N3SCC;
	stv2_current_tilemap.supplementary_palette_bits = STV_VDP2_N3SPLT;
	stv2_current_tilemap.supplementary_character_bits = STV_VDP2_N3SPCN;

	stv2_current_tilemap.scrollx = STV_VDP2_SCXN3;
	stv2_current_tilemap.scrolly = STV_VDP2_SCYN3;
	/*This layer can't be scaled*/
	stv2_current_tilemap.incx = 0x10000;
	stv2_current_tilemap.incy = 0x10000;

	stv2_current_tilemap.linescroll_enable = 0;
	stv2_current_tilemap.linescroll_interval = 0;
	stv2_current_tilemap.linescroll_table_address = 0;
	stv2_current_tilemap.vertical_linescroll_enable = 0;
	stv2_current_tilemap.linezoom_enable = 0;

	stv2_current_tilemap.colour_ram_address_offset = STV_VDP2_N3CAOS;
	stv2_current_tilemap.fade_control = (STV_VDP2_N3COEN * 1) | (STV_VDP2_N3COSL * 2);
	stv_vdp2_check_fade_control_for_layer();
	stv2_current_tilemap.window_control = (STV_VDP2_N3LOG * 0x01) |
										  (STV_VDP2_N3W0E * 0x02) |
										  (STV_VDP2_N3W1E * 0x04) |
										  (STV_VDP2_N3SWE * 0x08) |
										  (STV_VDP2_N3W0A * 0x10) |
										  (STV_VDP2_N3W1A * 0x20) |
										  (STV_VDP2_N3SWA * 0x40);

	stv2_current_tilemap.layer_name=3;

	stv2_current_tilemap.plane_size = STV_VDP2_N3PLSZ;

	if ( stv2_current_tilemap.enabled )
	{
		stv2_current_tilemap.enabled = stv_vdp2_check_vram_cycle_pattern_registers( STV_VDP2_CP_NBG3_PNMDR, STV_VDP2_CP_NBG3_CPDR, stv2_current_tilemap.bitmap_enable );
	}

	stv_vdp2_check_tilemap(machine, bitmap, cliprect);
}


static void stv_vdp2_draw_rotation_screen(running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect, int iRP)
{
	rectangle roz_clip_rect, mycliprect;
	int planesizex = 0, planesizey = 0;
	int planerenderedsizex, planerenderedsizey;
	UINT8 colour_calculation_enabled;
	UINT8 window_control;
	UINT8 fade_control;

	if ( iRP == 1)
	{
		stv2_current_tilemap.bitmap_map = STV_VDP2_RAMP_;
		stv2_current_tilemap.map_offset[0] = STV_VDP2_RAMPA | (STV_VDP2_RAMP_ << 6);
		stv2_current_tilemap.map_offset[1] = STV_VDP2_RAMPB | (STV_VDP2_RAMP_ << 6);
		stv2_current_tilemap.map_offset[2] = STV_VDP2_RAMPC | (STV_VDP2_RAMP_ << 6);
		stv2_current_tilemap.map_offset[3] = STV_VDP2_RAMPD | (STV_VDP2_RAMP_ << 6);
		stv2_current_tilemap.map_offset[4] = STV_VDP2_RAMPE | (STV_VDP2_RAMP_ << 6);
		stv2_current_tilemap.map_offset[5] = STV_VDP2_RAMPF | (STV_VDP2_RAMP_ << 6);
		stv2_current_tilemap.map_offset[6] = STV_VDP2_RAMPG | (STV_VDP2_RAMP_ << 6);
		stv2_current_tilemap.map_offset[7] = STV_VDP2_RAMPH | (STV_VDP2_RAMP_ << 6);
		stv2_current_tilemap.map_offset[8] = STV_VDP2_RAMPI | (STV_VDP2_RAMP_ << 6);
		stv2_current_tilemap.map_offset[9] = STV_VDP2_RAMPJ | (STV_VDP2_RAMP_ << 6);
		stv2_current_tilemap.map_offset[10] = STV_VDP2_RAMPK | (STV_VDP2_RAMP_ << 6);
		stv2_current_tilemap.map_offset[11] = STV_VDP2_RAMPL | (STV_VDP2_RAMP_ << 6);
		stv2_current_tilemap.map_offset[12] = STV_VDP2_RAMPM | (STV_VDP2_RAMP_ << 6);
		stv2_current_tilemap.map_offset[13] = STV_VDP2_RAMPN | (STV_VDP2_RAMP_ << 6);
		stv2_current_tilemap.map_offset[14] = STV_VDP2_RAMPO | (STV_VDP2_RAMP_ << 6);
		stv2_current_tilemap.map_offset[15] = STV_VDP2_RAMPP | (STV_VDP2_RAMP_ << 6);
		stv2_current_tilemap.map_count = 16;
	}
	else
	{
		stv2_current_tilemap.bitmap_map = STV_VDP2_RBMP_;
		stv2_current_tilemap.map_offset[0] = STV_VDP2_RBMPA | (STV_VDP2_RBMP_ << 6);
		stv2_current_tilemap.map_offset[1] = STV_VDP2_RBMPB | (STV_VDP2_RBMP_ << 6);
		stv2_current_tilemap.map_offset[2] = STV_VDP2_RBMPC | (STV_VDP2_RBMP_ << 6);
		stv2_current_tilemap.map_offset[3] = STV_VDP2_RBMPD | (STV_VDP2_RBMP_ << 6);
		stv2_current_tilemap.map_offset[4] = STV_VDP2_RBMPE | (STV_VDP2_RBMP_ << 6);
		stv2_current_tilemap.map_offset[5] = STV_VDP2_RBMPF | (STV_VDP2_RBMP_ << 6);
		stv2_current_tilemap.map_offset[6] = STV_VDP2_RBMPG | (STV_VDP2_RBMP_ << 6);
		stv2_current_tilemap.map_offset[7] = STV_VDP2_RBMPH | (STV_VDP2_RBMP_ << 6);
		stv2_current_tilemap.map_offset[8] = STV_VDP2_RBMPI | (STV_VDP2_RBMP_ << 6);
		stv2_current_tilemap.map_offset[9] = STV_VDP2_RBMPJ | (STV_VDP2_RBMP_ << 6);
		stv2_current_tilemap.map_offset[10] = STV_VDP2_RBMPK | (STV_VDP2_RBMP_ << 6);
		stv2_current_tilemap.map_offset[11] = STV_VDP2_RBMPL | (STV_VDP2_RBMP_ << 6);
		stv2_current_tilemap.map_offset[12] = STV_VDP2_RBMPM | (STV_VDP2_RBMP_ << 6);
		stv2_current_tilemap.map_offset[13] = STV_VDP2_RBMPN | (STV_VDP2_RBMP_ << 6);
		stv2_current_tilemap.map_offset[14] = STV_VDP2_RBMPO | (STV_VDP2_RBMP_ << 6);
		stv2_current_tilemap.map_offset[15] = STV_VDP2_RBMPP | (STV_VDP2_RBMP_ << 6);
		stv2_current_tilemap.map_count = 16;
	}

	stv_vdp2_fill_rotation_parameter_table(machine, iRP);

	if ( iRP == 1 )
	{
		stv2_current_tilemap.plane_size = STV_VDP2_RAPLSZ;
	}
	else
	{
		stv2_current_tilemap.plane_size = STV_VDP2_RBPLSZ;
	}

	if (stv2_current_tilemap.bitmap_enable)
	{
		switch (stv2_current_tilemap.bitmap_size)
		{
			case 0: planesizex=512; planesizey=256; break;
			case 1: planesizex=512; planesizey=512; break;
			case 2: planesizex=1024; planesizey=256; break;
			case 3: planesizex=1024; planesizey=512; break;
		}
	}
	else
	{
		switch( stv2_current_tilemap.plane_size )
		{
		case 0:
			planesizex = planesizey = 2048;
			break;
		case 1:
			planesizex = 4096;
			planesizey = 2048;
			break;
		case 2:
			planesizex = 0;
			planesizey = 0;
			break;
		case 3:
			planesizex = planesizey = 4096;
			break;
		}
	}


	if ( stv_vdp2_is_rotation_applied() == 0 )
	{
		stv2_current_tilemap.scrollx = stv_current_rotation_parameter_table.mx >> 16;
		stv2_current_tilemap.scrolly = stv_current_rotation_parameter_table.my >> 16;

		stv_vdp2_check_tilemap(machine, bitmap,cliprect);
	}
	else
	{
		if ( stv_vdp2_roz_bitmap[iRP-1] == NULL )
			stv_vdp2_roz_bitmap[iRP-1] = auto_bitmap_alloc(machine, 4096, 4096, video_screen_get_format(machine->primary_screen));

		roz_clip_rect.min_x = roz_clip_rect.min_y = 0;
		if ( (iRP == 1 && STV_VDP2_RAOVR == 3) ||
			 (iRP == 2 && STV_VDP2_RBOVR == 3) )
		{
			roz_clip_rect.max_x = roz_clip_rect.max_y = 511;
			planerenderedsizex = planerenderedsizey = 512;
		}
		else if (stv_vdp2_are_map_registers_equal() &&
				 !stv2_current_tilemap.bitmap_enable)
		{
			roz_clip_rect.max_x = (planesizex / 4) - 1;
			roz_clip_rect.max_y = (planesizey / 4) - 1;
			planerenderedsizex = planesizex / 4;
			planerenderedsizey = planesizey / 4;
		}
		else
		{
			roz_clip_rect.max_x = planesizex - 1;
			roz_clip_rect.max_y = planesizey - 1;
			planerenderedsizex = planesizex;
			planerenderedsizey = planesizey;
		}


		colour_calculation_enabled = stv2_current_tilemap.colour_calculation_enabled;
		stv2_current_tilemap.colour_calculation_enabled = 0;
		window_control = stv2_current_tilemap.window_control;
		stv2_current_tilemap.window_control = 0;
		fade_control = stv2_current_tilemap.fade_control;
		stv2_current_tilemap.fade_control = 0;
		profiler_mark_start(PROFILER_USER1);
		if ( LOG_VDP2 ) logerror( "Checking for cached RBG bitmap, cache_dirty = %d, memcmp() = %d\n", stv_rbg_cache_data.is_cache_dirty, memcmp(&stv_rbg_cache_data.layer_data[iRP-1],&stv2_current_tilemap,sizeof(stv2_current_tilemap)));
		if ( (stv_rbg_cache_data.is_cache_dirty & iRP) ||
			memcmp(&stv_rbg_cache_data.layer_data[iRP-1],&stv2_current_tilemap,sizeof(stv2_current_tilemap)) != 0 )
		{
			bitmap_fill( stv_vdp2_roz_bitmap[iRP-1], &roz_clip_rect , get_black_pen(machine));
			stv_vdp2_check_tilemap(machine, stv_vdp2_roz_bitmap[iRP-1], &roz_clip_rect);
			// prepare cache data
			stv_rbg_cache_data.watch_vdp2_vram_writes |= iRP;
			stv_rbg_cache_data.is_cache_dirty &= ~iRP;
			memcpy(&stv_rbg_cache_data.layer_data[iRP-1], &stv2_current_tilemap, sizeof(stv2_current_tilemap));
			stv_rbg_cache_data.map_offset_min[iRP-1] = stv_vdp2_layer_data_placement.map_offset_min;
			stv_rbg_cache_data.map_offset_max[iRP-1] = stv_vdp2_layer_data_placement.map_offset_max;
			stv_rbg_cache_data.tile_offset_min[iRP-1] = stv_vdp2_layer_data_placement.tile_offset_min;
			stv_rbg_cache_data.tile_offset_max[iRP-1] = stv_vdp2_layer_data_placement.tile_offset_max;
			if ( LOG_VDP2 ) logerror( "Cache watch: map = %06X - %06X, tile = %06X - %06X\n", stv_rbg_cache_data.map_offset_min[iRP-1],
				stv_rbg_cache_data.map_offset_max[iRP-1], stv_rbg_cache_data.tile_offset_min[iRP-1], stv_rbg_cache_data.tile_offset_max[iRP-1] );
		}

		profiler_mark_end();

		stv2_current_tilemap.colour_calculation_enabled = colour_calculation_enabled;
		if ( colour_calculation_enabled )
		{
			stv2_current_tilemap.transparency = STV_TRANSPARENCY_ALPHA;
		}

		mycliprect.min_x = cliprect->min_x;
		mycliprect.max_x = cliprect->max_x;
		mycliprect.min_y = cliprect->min_y;
		mycliprect.max_y = cliprect->max_y;

		if ( window_control )
		{
			stv2_current_tilemap.window_control = window_control;
			stv_vdp2_apply_window_on_layer(&mycliprect);
		}

		stv2_current_tilemap.fade_control = fade_control;

		profiler_mark_start(PROFILER_USER2);
		stv_vdp2_copy_roz_bitmap(bitmap, stv_vdp2_roz_bitmap[iRP-1], &mycliprect, iRP, planesizex, planesizey, planerenderedsizex, planerenderedsizey );
		profiler_mark_end();
	}

}

static void stv_vdp2_draw_RBG0(running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect)
{
	/*
       Colours           : 16, 256, 2048, 32768, 16770000
       Char Size         : 1x1 cells, 2x2 cells
       Pattern Data Size : 1 word, 2 words
       Plane Layouts     : 1 x 1, 2 x 1, 2 x 2
       Planes            : 4
       Bitmap            : Possible
       Bitmap Sizes      : 512 x 256, 512 x 512, 1024 x 256, 1024 x 512
       Scale             : 0.25 x - 256 x
       Rotation          : Yes
       Linescroll        : Yes
       Column Scroll     : Yes
       Mosaic            : Yes
    */

	stv2_current_tilemap.enabled = STV_VDP2_R0ON;

//  if (!stv2_current_tilemap.enabled) return; // stop right now if its disabled ...

	//stv2_current_tilemap.trans_enabled = STV_VDP2_R0TPON;
	if ( STV_VDP2_R0CCEN )
	{
		stv2_current_tilemap.colour_calculation_enabled = 1;
		stv2_current_tilemap.alpha = ((UINT16)(0x1f-STV_VDP2_R0CCRT)*0xff)/0x1f;
	}
	else
	{
		stv2_current_tilemap.colour_calculation_enabled = 0;
	}
	if ( STV_VDP2_R0TPON == 0 )
	{
		stv2_current_tilemap.transparency = STV_TRANSPARENCY_PEN;
	}
	else
	{
		stv2_current_tilemap.transparency = STV_TRANSPARENCY_NONE;
	}
	stv2_current_tilemap.colour_depth = STV_VDP2_R0CHCN;
	stv2_current_tilemap.tile_size = STV_VDP2_R0CHSZ;
	stv2_current_tilemap.bitmap_enable = STV_VDP2_R0BMEN;
	stv2_current_tilemap.bitmap_size = STV_VDP2_R0BMSZ;
	stv2_current_tilemap.bitmap_palette_number = STV_VDP2_R0BMP;

	stv2_current_tilemap.pattern_data_size = STV_VDP2_R0PNB;
	stv2_current_tilemap.character_number_supplement = STV_VDP2_R0CNSM;
	stv2_current_tilemap.special_priority_register = STV_VDP2_R0SPR;
	stv2_current_tilemap.special_colour_control_register = STV_VDP2_R0SCC;
	stv2_current_tilemap.supplementary_palette_bits = STV_VDP2_R0SPLT;
	stv2_current_tilemap.supplementary_character_bits = STV_VDP2_R0SPCN;

	stv2_current_tilemap.colour_ram_address_offset = STV_VDP2_R0CAOS;
	stv2_current_tilemap.fade_control = (STV_VDP2_R0COEN * 1) | (STV_VDP2_R0COSL * 2);
	stv_vdp2_check_fade_control_for_layer();
	stv2_current_tilemap.window_control = (STV_VDP2_R0LOG * 0x01) |
										  (STV_VDP2_R0W0E * 0x02) |
										  (STV_VDP2_R0W1E * 0x04) |
										  (STV_VDP2_R0SWE * 0x08) |
										  (STV_VDP2_R0W0A * 0x10) |
										  (STV_VDP2_R0W1A * 0x20) |
										  (STV_VDP2_R0SWA * 0x40);

	stv2_current_tilemap.scrollx = 0;
	stv2_current_tilemap.scrolly = 0;
	stv2_current_tilemap.incx = 0x10000;
	stv2_current_tilemap.incy = 0x10000;

	stv2_current_tilemap.linescroll_enable = 0;
	stv2_current_tilemap.linescroll_interval = 0;
	stv2_current_tilemap.linescroll_table_address = 0;
	stv2_current_tilemap.vertical_linescroll_enable = 0;
	stv2_current_tilemap.linezoom_enable = 0;

	/*Use 0x80 as a normal/rotate switch*/
	stv2_current_tilemap.layer_name=0x80;

	if ( !stv_vdp2_render_rbg0 ) return;
	if ( !stv2_current_tilemap.enabled ) return;

	switch(STV_VDP2_RPMD)
	{
		case 0://Rotation Parameter A
			stv_vdp2_draw_rotation_screen(machine, bitmap, cliprect, 1 );
			break;
		case 1://Rotation Parameter B
		//case 2:
			stv_vdp2_draw_rotation_screen(machine, bitmap, cliprect, 2 );
			break;
		case 2://Rotation Parameter A & B CKTE
			stv_vdp2_draw_rotation_screen(machine, bitmap, cliprect, 2 );
			stv_vdp2_draw_rotation_screen(machine, bitmap, cliprect, 1 );
			break;
		case 3://Rotation Parameter A & B Window (wrong)
			stv_vdp2_draw_rotation_screen(machine, bitmap, cliprect, 1 );
			break;
	}

}

static void stv_vdp2_draw_back(running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect)
{
	int xcnt,ycnt;
	UINT8* gfxdata = stv_vdp2_gfx_decode;
	static UINT16 *destline;
	int r,b,g;
	UINT16 data;

	if(!(STV_VDP2_BDCLMD & 1))
		bitmap_fill(bitmap, cliprect, get_black_pen(machine));
	else
	{
		#if DEBUG_MODE
		//popmessage("Back screen enabled %08x",STV_VDP2_BKTA);
		#endif
		gfxdata+=((STV_VDP2_BKTA)<<1);

		b = ((gfxdata[0] & 0x7c) >> 2);
		g = ((gfxdata[0] & 0x03) << 3) | ((gfxdata[1] & 0xe0) >> 5);
		r = ((gfxdata[1] & 0x1f));
		data = b | g << 5 | r << 10;

		for (ycnt = cliprect->min_y; ycnt <= cliprect->max_y;ycnt++)
		{
			destline = BITMAP_ADDR16(bitmap, ycnt, 0);

			for (xcnt = cliprect->min_x; xcnt <=cliprect->max_x;xcnt++)
			{
				destline[xcnt] = data;
			}
			if(STV_VDP2_BKCLMD)
			{
				gfxdata+=2;
				b = ((gfxdata[0] & 0x7c) >> 2);
				g = ((gfxdata[0] & 0x03) << 3) | ((gfxdata[1] & 0xe0) >> 5);
				r = ((gfxdata[1] & 0x1f));
				data = b | g << 5 | r << 10;
			}
		}
	}
}


WRITE32_HANDLER ( stv_vdp2_vram_w )
{
	UINT8 *stv_vdp2_vram_decode = stv_vdp2_gfx_decode;

	COMBINE_DATA(&stv_vdp2_vram[offset]);

	data = stv_vdp2_vram[offset];
	/* put in gfx region for easy decoding */
	stv_vdp2_vram_decode[offset*4+0] = (data & 0xff000000) >> 24;
	stv_vdp2_vram_decode[offset*4+1] = (data & 0x00ff0000) >> 16;
	stv_vdp2_vram_decode[offset*4+2] = (data & 0x0000ff00) >> 8;
	stv_vdp2_vram_decode[offset*4+3] = (data & 0x000000ff) >> 0;

	gfx_element_mark_dirty(space->machine->gfx[0], offset/8);
	gfx_element_mark_dirty(space->machine->gfx[1], offset/8);
	gfx_element_mark_dirty(space->machine->gfx[2], offset/8);
	gfx_element_mark_dirty(space->machine->gfx[3], offset/8);

	/* 8-bit tiles overlap, so this affects the previous one as well */
	if (offset/8 != 0)
	{
		gfx_element_mark_dirty(space->machine->gfx[2], offset/8 - 1);
		gfx_element_mark_dirty(space->machine->gfx[3], offset/8 - 1);
	}

	if ( stv_rbg_cache_data.watch_vdp2_vram_writes )
	{
		if ( stv_rbg_cache_data.watch_vdp2_vram_writes & STV_VDP2_RBG_ROTATION_PARAMETER_A )
		{
			if ( (offset >= stv_rbg_cache_data.map_offset_min[0] &&
				 offset < stv_rbg_cache_data.map_offset_max[0]) ||
				 (offset >= stv_rbg_cache_data.tile_offset_min[0] &&
				 offset < stv_rbg_cache_data.tile_offset_max[0]) )
			{
				if ( LOG_VDP2 ) logerror( "RBG Cache: dirtying for RP = 1, write at offset = %06X\n", offset );
				stv_rbg_cache_data.is_cache_dirty |= STV_VDP2_RBG_ROTATION_PARAMETER_A;
				stv_rbg_cache_data.watch_vdp2_vram_writes &= ~STV_VDP2_RBG_ROTATION_PARAMETER_A;
			}
		}
		if ( stv_rbg_cache_data.watch_vdp2_vram_writes & STV_VDP2_RBG_ROTATION_PARAMETER_B )
		{
			if ( (offset >= stv_rbg_cache_data.map_offset_min[1] &&
				 offset < stv_rbg_cache_data.map_offset_max[1]) ||
				 (offset >= stv_rbg_cache_data.tile_offset_min[1] &&
				 offset < stv_rbg_cache_data.tile_offset_max[1]) )
			{
				if ( LOG_VDP2 ) logerror( "RBG Cache: dirtying for RP = 2, write at offset = %06X\n", offset );
				stv_rbg_cache_data.is_cache_dirty |= STV_VDP2_RBG_ROTATION_PARAMETER_B;
				stv_rbg_cache_data.watch_vdp2_vram_writes &= ~STV_VDP2_RBG_ROTATION_PARAMETER_B;
			}
		}

	}

}

READ32_HANDLER ( stv_vdp2_vram_r )
{
	return stv_vdp2_vram[offset];
}

WRITE32_HANDLER ( stv_vdp2_cram_w )
{
	int r,g,b;
	COMBINE_DATA(&stv_vdp2_cram[offset]);

//  popmessage("%01x\n",STV_VDP2_CRMD);

	switch( STV_VDP2_CRMD )
	{
		/*Mode 2/3*/
		case 2:
		case 3:
		{
			b = ((stv_vdp2_cram[offset] & 0x00ff0000) >> 16);
			g = ((stv_vdp2_cram[offset] & 0x0000ff00) >> 8);
			r = ((stv_vdp2_cram[offset] & 0x000000ff) >> 0);
			palette_set_color(space->machine,offset,MAKE_RGB(r,g,b));
		}
		break;
		/*Mode 0*/
		case 0:
		{
			offset &= 0x3ff;

			b = ((stv_vdp2_cram[offset] & 0x00007c00) >> 10);
			g = ((stv_vdp2_cram[offset] & 0x000003e0) >> 5);
			r = ((stv_vdp2_cram[offset] & 0x0000001f) >> 0);
			palette_set_color_rgb(space->machine,(offset*2)+1,pal5bit(r),pal5bit(g),pal5bit(b));
			b = ((stv_vdp2_cram[offset] & 0x7c000000) >> 26);
			g = ((stv_vdp2_cram[offset] & 0x03e00000) >> 21);
			r = ((stv_vdp2_cram[offset] & 0x001f0000) >> 16);
			palette_set_color_rgb(space->machine,offset*2,pal5bit(r),pal5bit(g),pal5bit(b));
		}
		break;
		/*Mode 1*/
		case 1:
		{
			offset &= 0x7ff;

			b = ((stv_vdp2_cram[offset] & 0x00007c00) >> 10);
			g = ((stv_vdp2_cram[offset] & 0x000003e0) >> 5);
			r = ((stv_vdp2_cram[offset] & 0x0000001f) >> 0);
			palette_set_color_rgb(space->machine,(offset*2)+1,pal5bit(r),pal5bit(g),pal5bit(b));
			b = ((stv_vdp2_cram[offset] & 0x7c000000) >> 26);
			g = ((stv_vdp2_cram[offset] & 0x03e00000) >> 21);
			r = ((stv_vdp2_cram[offset] & 0x001f0000) >> 16);
			palette_set_color_rgb(space->machine,offset*2,pal5bit(r),pal5bit(g),pal5bit(b));
		}
		break;
	}
}

static void refresh_palette_data(running_machine *machine)
{
	int r,g,b;
	int c_i;

	for(c_i=0;c_i<0x800;c_i++)
	{
		switch( STV_VDP2_CRMD )
		{
			/*Mode 2/3*/
			case 2:
			case 3:
			{
				b = ((stv_vdp2_cram[c_i] & 0x00ff0000) >> 16);
				g = ((stv_vdp2_cram[c_i] & 0x0000ff00) >> 8);
				r = ((stv_vdp2_cram[c_i] & 0x000000ff) >> 0);
				palette_set_color(machine,c_i,MAKE_RGB(r,g,b));
			}
			break;
			/*Mode 0*/
			case 0:
			{
				//c_i &= 0x3ff;

				b = ((stv_vdp2_cram[c_i] & 0x00007c00) >> 10);
				g = ((stv_vdp2_cram[c_i] & 0x000003e0) >> 5);
				r = ((stv_vdp2_cram[c_i] & 0x0000001f) >> 0);
				palette_set_color_rgb(machine,(c_i*2)+1,pal5bit(r),pal5bit(g),pal5bit(b));
				b = ((stv_vdp2_cram[c_i] & 0x7c000000) >> 26);
				g = ((stv_vdp2_cram[c_i] & 0x03e00000) >> 21);
				r = ((stv_vdp2_cram[c_i] & 0x001f0000) >> 16);
				palette_set_color_rgb(machine,c_i*2,pal5bit(r),pal5bit(g),pal5bit(b));
			}
			break;
			/*Mode 1*/
			case 1:
			{
				//c_i &= 0x7ff;

				b = ((stv_vdp2_cram[c_i] & 0x00007c00) >> 10);
				g = ((stv_vdp2_cram[c_i] & 0x000003e0) >> 5);
				r = ((stv_vdp2_cram[c_i] & 0x0000001f) >> 0);
				palette_set_color_rgb(machine,(c_i*2)+1,pal5bit(r),pal5bit(g),pal5bit(b));
				b = ((stv_vdp2_cram[c_i] & 0x7c000000) >> 26);
				g = ((stv_vdp2_cram[c_i] & 0x03e00000) >> 21);
				r = ((stv_vdp2_cram[c_i] & 0x001f0000) >> 16);
				palette_set_color_rgb(machine,c_i*2,pal5bit(r),pal5bit(g),pal5bit(b));
			}
			break;
		}
	}
}

READ32_HANDLER ( stv_vdp2_cram_r )
{
	return stv_vdp2_cram[offset];
}

WRITE32_HANDLER ( stv_vdp2_regs_w )
{
	static UINT8 old_crmd;
	static UINT16 old_tvmd;
	COMBINE_DATA(&stv_vdp2_regs[offset]);

	if(old_crmd != STV_VDP2_CRMD)
	{
		old_crmd = STV_VDP2_CRMD;
		refresh_palette_data(space->machine);
	}
	if(old_tvmd != STV_VDP2_TVMD)
	{
		old_tvmd = STV_VDP2_TVMD;
		stv_vdp2_dynamic_res_change(space->machine);
	}
}

static UINT8 get_hblank(running_machine *machine)
{
	static int cur_h;

	rectangle visarea = *video_screen_get_visible_area(machine->primary_screen);
	cur_h = video_screen_get_hpos(machine->primary_screen);

	if (cur_h > visarea.max_x)
		return 1;
	else
		return 0;
}

/* the following is a complete guess-work */
static int get_hblank_duration(running_machine *machine)
{
	switch( STV_VDP2_HRES & 3 )
	{
		case 0: return 80; //400-320
		case 1: return 104; break; //456-352
		case 2: return 160; break; //(400-320)*2
		case 3: return 208; break; //(456-352)*2
	}

	return 0;
}

UINT8 stv_get_vblank(running_machine *machine)
{
	static int cur_v;
	rectangle visarea = *video_screen_get_visible_area(machine->primary_screen);
	cur_v = video_screen_get_vpos(machine->primary_screen);

	if (cur_v > visarea.max_y)
		return 1;
	else
		return 0;
}

/*some vblank lines measurements (according to Charles MacDonald)*/
static int get_vblank_duration(running_machine *machine)
{
	if(STV_VDP2_HRES & 4)
	{
		switch(STV_VDP2_HRES & 1)
		{
			case 0: return 45; //31kHz Monitor
			case 1: return 82; //Hi-Vision Monitor
		}
	}

	switch(STV_VDP2_VRES & 3)
	{
		case 0: return 40; //264-224
		case 1: return 24; //264-240
		case 2: return 8; //264-256
		case 3: return 8; //264-256
	}

	return 0;
}

static UINT8 get_odd_bit(running_machine *machine)
{
	static int cur_v;
	cur_v = video_screen_get_vpos(machine->primary_screen);

	if(STV_VDP2_HRES & 4) //exclusive monitor mode makes this bit to be always 1
		return 1;

	if(cur_v % 2)
		return 1;
	else
		return 0;
}

READ32_HANDLER ( stv_vdp2_regs_r )
{
//  if (offset!=1) if(LOG_VDP2) logerror ("VDP2: Read from Registers, Offset %04x\n",offset);

	switch(offset)
	{
		case 0x4/4:
		{
			/*Screen Status Register*/
			stv_vblank = stv_get_vblank(space->machine);
			stv_hblank = get_hblank(space->machine);
			stv_odd = get_odd_bit(space->machine);

								   /*VBLANK              HBLANK            ODD               PAL    */
			stv_vdp2_regs[offset] = (stv_vblank<<19) | (stv_hblank<<18) | (stv_odd << 17) | (0 << 16);
			break;
		}
		case 0x8/4:
		/*H/V Counter Register*/
		{
			static UINT16 h_count,v_count;
			/* TODO: handle various h/v settings. */
			h_count = video_screen_get_hpos(space->machine->primary_screen) & 0x3ff;
			v_count = video_screen_get_vpos(space->machine->primary_screen) & (STV_VDP2_LSMD == 3 ? 0x7ff : 0x3ff);
			stv_vdp2_regs[offset] = (h_count<<16)|(v_count);
			if(LOG_VDP2) logerror("CPU %s PC(%08x) = VDP2: H/V counter read : %08x\n", space->cpu->tag, cpu_get_pc(space->cpu),stv_vdp2_regs[offset]);
			break;
		}
	}
	return stv_vdp2_regs[offset];
}

static STATE_POSTLOAD( stv_vdp2_state_save_postload )
{
	UINT8 *stv_vdp2_vram_decode = stv_vdp2_gfx_decode;
	int offset;
	UINT32 data;

	for ( offset = 0; offset < 0x100000/4; offset++ )
	{
		data = stv_vdp2_vram[offset];
		/* put in gfx region for easy decoding */
		stv_vdp2_vram_decode[offset*4+0] = (data & 0xff000000) >> 24;
		stv_vdp2_vram_decode[offset*4+1] = (data & 0x00ff0000) >> 16;
		stv_vdp2_vram_decode[offset*4+2] = (data & 0x0000ff00) >> 8;
		stv_vdp2_vram_decode[offset*4+3] = (data & 0x000000ff) >> 0;
	}

	memset( &stv_rbg_cache_data, 0, sizeof(stv_rbg_cache_data));
	stv_rbg_cache_data.is_cache_dirty = 3;
	memset( &stv_vdp2_layer_data_placement, 0, sizeof(stv_vdp2_layer_data_placement));

	refresh_palette_data(machine);
}

static int stv_vdp2_start (running_machine *machine)
{
	stv_vdp2_regs = auto_alloc_array_clear(machine, UINT32, 0x040000/4 );
	stv_vdp2_vram = auto_alloc_array_clear(machine, UINT32, 0x100000/4 ); // actually we only need half of it since we don't emulate extra 4mbit ram cart.
	stv_vdp2_cram = auto_alloc_array_clear(machine, UINT32, 0x080000/4 );
	stv_vdp2_gfx_decode = auto_alloc_array(machine, UINT8, 0x100000 );

	stv_vdp2_render_rbg0 = 1;
//  machine->gfx[0]->color_granularity=4;
//  machine->gfx[1]->color_granularity=4;

	memset( &stv_rbg_cache_data, 0, sizeof(stv_rbg_cache_data));
	stv_rbg_cache_data.is_cache_dirty = 3;
	memset( &stv_vdp2_layer_data_placement, 0, sizeof(stv_vdp2_layer_data_placement));

	state_save_register_global_pointer(machine, stv_vdp2_regs, 0x040000/4);
	state_save_register_global_pointer(machine, stv_vdp2_vram, 0x100000/4);
	state_save_register_global_pointer(machine, stv_vdp2_cram, 0x080000/4);
	state_save_register_postload(machine, stv_vdp2_state_save_postload, NULL);

	return 0;
}

/* maybe we should move this to video/stv.c */
VIDEO_START( stv_vdp2 )
{
	stv_vdp2_roz_bitmap[0] =  stv_vdp2_roz_bitmap[1] = NULL;
	stv_vdp2_start(machine);
	stv_vdp1_start(machine);
	debug.l_en = 0xff;
	debug.error = 0xffffffff;
	debug.roz = 0;
	gfx_element_set_source(machine->gfx[0], stv_vdp2_gfx_decode);
	gfx_element_set_source(machine->gfx[1], stv_vdp2_gfx_decode);
	gfx_element_set_source(machine->gfx[2], stv_vdp2_gfx_decode);
	gfx_element_set_source(machine->gfx[3], stv_vdp2_gfx_decode);
	gfx_element_set_source(machine->gfx[4], stv_vdp1_gfx_decode);
	gfx_element_set_source(machine->gfx[5], stv_vdp1_gfx_decode);
	gfx_element_set_source(machine->gfx[6], stv_vdp1_gfx_decode);
	gfx_element_set_source(machine->gfx[7], stv_vdp1_gfx_decode);
}

/*TODO: frame_period should be different for every kind of resolution (needs tests on actual boards)*/
/*    & height / width not yet understood (docs-wise MUST be bigger than normal visible area)*/
static TIMER_CALLBACK( dyn_res_change )
{
	int vblank_period,hblank_period;
	rectangle visarea = *video_screen_get_visible_area(machine->primary_screen);
	visarea.min_x = 0;
	visarea.max_x = horz_res-1;
	visarea.min_y = 0;
	visarea.max_y = vert_res-1;

	vblank_period = get_vblank_duration(machine);
	hblank_period = get_hblank_duration(machine);
//  popmessage("%d",vblank_period);
//  hblank_period = get_hblank_duration(machine->primary_screen);
	video_screen_configure(machine->primary_screen, (horz_res+hblank_period), (vert_res+vblank_period), &visarea, video_screen_get_frame_period(machine->primary_screen).attoseconds );
}

static void stv_vdp2_dynamic_res_change(running_machine *machine)
{
	static UINT8 old_vres = 0,old_hres = 0;

	switch( STV_VDP2_VRES & 3 )
	{
		case 0: vert_res = 224; break;
		case 1: vert_res = 240; break;
		case 2: vert_res = 256; break;
		case 3:
			if(LOG_VDP2) logerror("WARNING: V Res setting (3) not allowed!\n");
			vert_res = 256;
			break;
	}

	/*Double-density interlace mode,doubles the vertical res*/
	if((STV_VDP2_LSMD & 3) == 3) { vert_res*=2;  }

	switch( STV_VDP2_HRES & 7 )
	{
		case 0: horz_res = 320; break;
		case 1: horz_res = 352; break;
		case 2: horz_res = 640; break;
		case 3: horz_res = 704; break;
		/*Exclusive modes,they sets the Vertical Resolution without considering the
            VRES register.*/
		case 4: horz_res = 320; vert_res = 480; break;
		case 5: horz_res = 352; vert_res = 480; break;
		case 6: horz_res = 640; vert_res = 480; break;
		case 7: horz_res = 704; vert_res = 480; break;
	}
//  horz_res+=1;
//  vert_res*=2;
	if(old_vres != vert_res || old_hres != horz_res)
	{
		timer_set(machine, video_screen_get_time_until_pos(machine->primary_screen, 0, 0), NULL, 0, dyn_res_change);
		old_vres = vert_res;
		old_hres = horz_res;
	}
//  video_screen_set_visarea(machine->primary_screen, 0*8, horz_res-1,0*8, vert_res-1);
	//if(LOG_VDP2) popmessage("%04d %04d",horz_res-1,vert-1);
}

/*This is for calculating the rgb brightness*/
/*TODO: Optimize this...*/
static void	stv_vdp2_fade_effects(running_machine *machine)
{
	/*
    Note:We have to use temporary storages because palette_get_color must use
    variables setted with unsigned int8
    */
	INT16 t_r,t_g,t_b;
	UINT8 r,g,b;
	rgb_t color;
	int i;
	//popmessage("%04x %04x",STV_VDP2_CLOFEN,STV_VDP2_CLOFSL);
	for(i=0;i<2048;i++)
	{
		/*Fade A*/
		color = palette_get_color(machine, i);
		t_r = (STV_VDP2_COAR & 0x100) ? (RGB_RED(color) - (0x100 - (STV_VDP2_COAR & 0xff))) : ((STV_VDP2_COAR & 0xff) + RGB_RED(color));
		t_g = (STV_VDP2_COAG & 0x100) ? (RGB_GREEN(color) - (0x100 - (STV_VDP2_COAG & 0xff))) : ((STV_VDP2_COAG & 0xff) + RGB_GREEN(color));
		t_b = (STV_VDP2_COAB & 0x100) ? (RGB_BLUE(color) - (0x100 - (STV_VDP2_COAB & 0xff))) : ((STV_VDP2_COAB & 0xff) + RGB_BLUE(color));
		if(t_r < 0) 	{ t_r = 0; }
		if(t_r > 0xff) 	{ t_r = 0xff; }
		if(t_g < 0) 	{ t_g = 0; }
		if(t_g > 0xff) 	{ t_g = 0xff; }
		if(t_b < 0) 	{ t_b = 0; }
		if(t_b > 0xff) 	{ t_b = 0xff; }
		r = t_r;
		g = t_g;
		b = t_b;
		palette_set_color(machine,i+(2048*1),MAKE_RGB(r,g,b));

		/*Fade B*/
		color = palette_get_color(machine, i);
		t_r = (STV_VDP2_COBR & 0x100) ? (RGB_RED(color) - (0xff - (STV_VDP2_COBR & 0xff))) : ((STV_VDP2_COBR & 0xff) + RGB_RED(color));
		t_g = (STV_VDP2_COBG & 0x100) ? (RGB_GREEN(color) - (0xff - (STV_VDP2_COBG & 0xff))) : ((STV_VDP2_COBG & 0xff) + RGB_GREEN(color));
		t_b = (STV_VDP2_COBB & 0x100) ? (RGB_BLUE(color) - (0xff - (STV_VDP2_COBB & 0xff))) : ((STV_VDP2_COBB & 0xff) + RGB_BLUE(color));
		if(t_r < 0) 	{ t_r = 0; }
		if(t_r > 0xff) 	{ t_r = 0xff; }
		if(t_g < 0) 	{ t_g = 0; }
		if(t_g > 0xff) 	{ t_g = 0xff; }
		if(t_b < 0) 	{ t_b = 0; }
		if(t_b > 0xff) 	{ t_b = 0xff; }
		r = t_r;
		g = t_g;
		b = t_b;
		palette_set_color(machine,i+(2048*2),MAKE_RGB(r,g,b));
	}
	//popmessage("%04x %04x %04x %04x %04x %04x",STV_VDP2_COAR,STV_VDP2_COAG,STV_VDP2_COAB,STV_VDP2_COBR,STV_VDP2_COBG,STV_VDP2_COBB);
}

/******************************************************************************************

ST-V VDP2 window effect function version 0.04

How it works: returns 0 if the requested pixel is drawnable,1 if it isn't.
For tilemap and sprite layer, clipping rectangle is changed.

Done:
-Basic support(w0 or w1),bitmaps only.
-W0 (outside) for tilemaps and sprite layer.

Not Done:
-Complete Windows on cells.A split between cells and bitmaps is in progress...
-w0 & w1 at the same time.
-Window logic.
-Line window.
-Color Calculation.
-Rotation parameter Window (already done?).

Window Registers are hooked up like this ATM:
    x--- ---- UNUSED
    -x-- ---- Sprite Window Area
    --x- ---- Window 1 Area
    ---x ---- Window 0 Area
                  (0 = Inside,1 = Outside)
    ---- x--- Sprite Window Enable
    ---- -x-- Window 1 Enable
    ---- --x- Window 0 Enable
                  (0 = Disabled,1 = Enabled)
    ---- ---x Window Logic
                  (0 = OR,1 = AND)
******************************************************************************************/

static void stv_vdp2_get_window0_coordinates(UINT16 *s_x, UINT16 *e_x, UINT16 *s_y, UINT16 *e_y)
{
	/*W0*/
 	switch(STV_VDP2_LSMD & 3)
	{
		case 0:
		case 1:
		case 2:
			*s_y = ((STV_VDP2_W0SY & 0x3ff) >> 0);
			*e_y = ((STV_VDP2_W0EY & 0x3ff) >> 0);
			break;
		case 3:
			*s_y = ((STV_VDP2_W0SY & 0x7ff) >> 0);
			*e_y = ((STV_VDP2_W0EY & 0x7ff) >> 0);
			break;
	}
	switch(STV_VDP2_HRES & 6)
	{
		/*Normal*/
		case 0:
			*s_x = ((STV_VDP2_W0SX & 0x3fe) >> 1);
			*e_x = ((STV_VDP2_W0EX & 0x3fe) >> 1);
			break;
		/*Hi-Res*/
		case 2:
			*s_x = ((STV_VDP2_W0SX & 0x3ff) >> 0);
			*e_x = ((STV_VDP2_W0EX & 0x3ff) >> 0);
			break;
		/*Exclusive Normal*/
		case 4:
			*s_x = ((STV_VDP2_W0SX & 0x1ff) >> 0);
			*e_x = ((STV_VDP2_W0EX & 0x1ff) >> 0);
			*s_y = ((STV_VDP2_W0SY & 0x3ff) >> 0);
			*e_y = ((STV_VDP2_W0EY & 0x3ff) >> 0);
			break;
		/*Exclusive Hi-Res*/
		case 6:
			*s_x = ((STV_VDP2_W0SX & 0x1ff) << 1);
			*e_x = ((STV_VDP2_W0EX & 0x1ff) << 1);
			*s_y = ((STV_VDP2_W0SY & 0x3ff) >> 0);
			*e_y = ((STV_VDP2_W0EY & 0x3ff) >> 0);
			break;
	}
}

static void stv_vdp2_get_window1_coordinates(UINT16 *s_x, UINT16 *e_x, UINT16 *s_y, UINT16 *e_y)
{
	/*W1*/
	switch(STV_VDP2_LSMD & 3)
	{
		case 0:
		case 1:
		case 2:
			*s_y = ((STV_VDP2_W1SY & 0x3ff) >> 0);
			*e_y = ((STV_VDP2_W1EY & 0x3ff) >> 0);
			break;
		case 3:
			*s_y = ((STV_VDP2_W1SY & 0x3ff) >> 0);
			*e_y = ((STV_VDP2_W1EY & 0x3ff) >> 0);
			break;
	}
	switch(STV_VDP2_HRES & 6)
	{
		/*Normal*/
		case 0:
			*s_x = ((STV_VDP2_W1SX & 0x3fe) >> 1);
			*e_x = ((STV_VDP2_W1EX & 0x3fe) >> 1);
			break;
		/*Hi-Res*/
		case 2:
			*s_x = ((STV_VDP2_W1SX & 0x3ff) >> 0);
			*e_x = ((STV_VDP2_W1EX & 0x3ff) >> 0);
			break;
		/*Exclusive Normal*/
		case 4:
			*s_x = ((STV_VDP2_W1SX & 0x1ff) >> 0);
			*e_x = ((STV_VDP2_W1EX & 0x1ff) >> 0);
			*s_y = ((STV_VDP2_W1SY & 0x3ff) >> 0);
			*e_y = ((STV_VDP2_W1EY & 0x3ff) >> 0);
			break;
		/*Exclusive Hi-Res*/
		case 6:
			*s_x = ((STV_VDP2_W1SX & 0x1ff) << 1);
			*e_x = ((STV_VDP2_W1EX & 0x1ff) << 1);
			*s_y = ((STV_VDP2_W1SY & 0x3ff) >> 0);
			*e_y = ((STV_VDP2_W1EY & 0x3ff) >> 0);
			break;
	}

}

static int stv_vdp2_window_process(int x,int y)
{
	UINT16 s_x=0,e_x=0,s_y=0,e_y=0;

	if ((stv2_current_tilemap.window_control & 6) == 0)
		return 0;

	stv_vdp2_get_window0_coordinates(&s_x, &e_x, &s_y, &e_y);

	if(stv2_current_tilemap.window_control & 2)
	{
		/*Outside Area*/
		if(stv2_current_tilemap.window_control & 0x10)
		{
			if(y < s_y || y > e_y)
				return 1;
			else
			{
				if(x < s_x || x > e_x)
					return 1;
				//else
				//  return 0;
			}
		}
		/*Inside Area*/
		else
		{
			if(y > s_y && y < e_y)
			{
				if(x > s_x && x < e_x)
					return 1;
			}
			//else
			//  return 0;
		}
	}

	stv_vdp2_get_window1_coordinates(&s_x, &e_x, &s_y, &e_y);

	if(stv2_current_tilemap.window_control & 4)
	{
		/*Outside Area*/
		if(stv2_current_tilemap.window_control & 0x20)
		{
			if(y < s_y || y > e_y)
				return 1;
			else
			{
				if(x < s_x || x > e_x)
					return 1;
				//else
				//  return 0;
			}
		}
		/*Inside Area*/
		else
		{
			if(y > s_y && y < e_y)
			{
				if(x > s_x && x < e_x)
					return 1;
			}
			//else
			//  return 0;
		}
	}
	return 0;
//  return 1;
}

static int stv_vdp2_apply_window_on_layer(rectangle *cliprect)
{
	UINT16 s_x=0,e_x=0,s_y=0,e_y=0;

	if ( stv2_current_tilemap.window_control == 0x12 )
	{
		/* w0, transparent outside supported */
		stv_vdp2_get_window0_coordinates(&s_x, &e_x, &s_y, &e_y);

		if ( s_x > cliprect->min_x ) cliprect->min_x = s_x;
		if ( e_x < cliprect->max_x ) cliprect->max_x = e_x;
		if ( s_y > cliprect->min_y ) cliprect->min_y = s_y;
		if ( e_y < cliprect->max_y ) cliprect->max_y = e_y;

		return 1;
	}
	else if ( stv2_current_tilemap.window_control == 0x24 )
	{
		/* w1, transparent outside supported */
		stv_vdp2_get_window1_coordinates(&s_x, &e_x, &s_y, &e_y);

		if ( s_x > cliprect->min_x ) cliprect->min_x = s_x;
		if ( e_x < cliprect->max_x ) cliprect->max_x = e_x;
		if ( s_y > cliprect->min_y ) cliprect->min_y = s_y;
		if ( e_y < cliprect->max_y ) cliprect->max_y = e_y;

		return 1;
	}
	else
	{
		return 0;
	}
}

/* VDP1 Framebuffer handling */
static int		stv_sprite_priorities_used[8];
static int		stv_sprite_priorities_usage_valid;
static UINT8	stv_sprite_priorities_in_fb_line[512][8];


static void draw_sprites(running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect, UINT8 pri)
{
	int x,y,r,g,b;
	int i;
	UINT16 pix;
	UINT16 *framebuffer_line;
	UINT16 *bitmap_line, *bitmap_line2 = NULL;
	UINT8  interlace_framebuffer;
	UINT8  double_x;
	static const UINT16 sprite_colormask_table[] = { 0x07ff, 0x07ff, 0x07ff, 0x07ff, 0x03ff, 0x07ff, 0x03ff, 0x01ff,
										0x007f, 0x003f, 0x003f, 0x003f, 0x0ff, 0x0ff, 0x0ff, 0x0ff };
	static const UINT16 priority_shift_table[] = { 14, 13, 14, 13, 13, 12, 12, 12, 7, 7, 6, 0, 7, 7, 6, 0 };
	static const UINT16 priority_mask_table[]  = {  3,  7,  1,  3,  3,  7,  7,  7, 1, 1, 3, 0, 1, 1, 3, 0 };
	static const UINT16 ccrr_shift_table[] =	 { 11, 11, 11, 11, 10, 11, 10,  9, 0, 6, 0, 6, 0, 6, 0, 6 };
	static const UINT16 ccrr_mask_table[] =	     {  7,  3,  7,  3,  7,  1,  3,  7, 0, 1, 0, 3, 0, 1, 0, 3 };
	UINT16 alpha_enabled;

	int sprite_type;
	int sprite_colormask;
	int color_offset_pal;
	UINT16 sprite_priority_shift, sprite_priority_mask, sprite_ccrr_shift, sprite_ccrr_mask;
	UINT8	priority;
	UINT8	ccr = 0;
	UINT8 sprite_priorities[8];
	UINT8 sprite_ccr[8];
	int		sprite_color_mode = STV_VDP2_SPCLMD;
	rectangle mycliprect;

	if ( (stv_sprite_priorities_usage_valid == 1) && (stv_sprite_priorities_used[pri] == 0) )
		return;

	sprite_priorities[0] = STV_VDP2_S0PRIN;
	sprite_priorities[1] = STV_VDP2_S1PRIN;
	sprite_priorities[2] = STV_VDP2_S2PRIN;
	sprite_priorities[3] = STV_VDP2_S3PRIN;
	sprite_priorities[4] = STV_VDP2_S4PRIN;
	sprite_priorities[5] = STV_VDP2_S5PRIN;
	sprite_priorities[6] = STV_VDP2_S6PRIN;
	sprite_priorities[7] = STV_VDP2_S7PRIN;

	sprite_ccr[0] = STV_VDP2_S0CCRT;
	sprite_ccr[1] = STV_VDP2_S1CCRT;
	sprite_ccr[2] = STV_VDP2_S2CCRT;
	sprite_ccr[3] = STV_VDP2_S3CCRT;
	sprite_ccr[4] = STV_VDP2_S4CCRT;
	sprite_ccr[5] = STV_VDP2_S5CCRT;
	sprite_ccr[6] = STV_VDP2_S6CCRT;
	sprite_ccr[7] = STV_VDP2_S7CCRT;

	sprite_type = STV_VDP2_SPTYPE;
	sprite_colormask = sprite_colormask_table[sprite_type];
	sprite_priority_shift = priority_shift_table[sprite_type];
	sprite_priority_mask = priority_mask_table[sprite_type];
	sprite_ccrr_shift = ccrr_shift_table[sprite_type];
	sprite_ccrr_mask = ccrr_mask_table[sprite_type];

	for ( i = 0; i < (sprite_priority_mask+1); i++ ) if ( sprite_priorities[i] == pri ) break;
	if ( i == (sprite_priority_mask+1) ) return;

	/* color offset (RGB brightness) */
	color_offset_pal = 0;
	if ( STV_VDP2_SPCOEN )
	{
		if ( STV_VDP2_SPCOSL == 0 )
		{ color_offset_pal = 2048; }
		else
		{ color_offset_pal = 2048*2; }
	}

	/* color calculation (alpha blending)*/
	if ( STV_VDP2_SPCCEN )
	{
		alpha_enabled = 0;
		switch( STV_VDP2_SPCCCS )
		{
			case 0x0: if ( pri <= STV_VDP2_SPCCN ) alpha_enabled = 1; break;
			case 0x1: if ( pri == STV_VDP2_SPCCN ) alpha_enabled = 1; break;
			case 0x2: if ( pri >= STV_VDP2_SPCCN ) alpha_enabled = 1; break;
			case 0x3: /* MSBON */ break;
		}
	}
	else
	{
		alpha_enabled = 0;
	}

	/* framebuffer interlace */
	if ( (STV_VDP2_LSMD == 2 || STV_VDP2_LSMD == 3) && stv_framebuffer_double_interlace == 0 )
		interlace_framebuffer = 1;
	else
		interlace_framebuffer = 0;

	/*Guess:Some games needs that the horizontal sprite size to be doubled
      (TODO: understand the proper settings,it might not work like this)*/
	if(STV_VDP2_LSMD == 3 && /*((STV_VDP2_HRES & 3) != 3) &&*/ (!(stv_framebuffer_mode & 1)))
		double_x = 1;
	else
		double_x = 0;

	/* window control */
	stv2_current_tilemap.window_control = (STV_VDP2_SPLOG * 0x01) |
										  (STV_VDP2_SPW0E * 0x02) |
										  (STV_VDP2_SPW1E * 0x04) |
										  (STV_VDP2_SPSWE * 0x08) |
										  (STV_VDP2_SPW0A * 0x10) |
										  (STV_VDP2_SPW1A * 0x20) |
										  (STV_VDP2_SPSWA * 0x40);
	mycliprect.min_x = cliprect->min_x;
	mycliprect.max_x = cliprect->max_x;
	mycliprect.min_y = cliprect->min_y;
	mycliprect.max_y = cliprect->max_y;
	stv_vdp2_apply_window_on_layer(&mycliprect);

	if (interlace_framebuffer == 0 && double_x == 0 )
	{
		if ( alpha_enabled == 0 )
		{
			for ( y = mycliprect.min_y; y <= mycliprect.max_y; y++ )
			{
				if ( stv_sprite_priorities_usage_valid )
					if (stv_sprite_priorities_in_fb_line[y][pri] == 0)
						continue;

				framebuffer_line = stv_framebuffer_display_lines[y];
				bitmap_line = BITMAP_ADDR16(bitmap, y, 0);

				for ( x = mycliprect.min_x; x <= mycliprect.max_x; x++ )
				{
					pix = framebuffer_line[x];
					if ( (pix & 0x8000) && sprite_color_mode)
					{
						if ( sprite_priorities[0] != pri )
						{
							stv_sprite_priorities_used[sprite_priorities[0]] = 1;
							stv_sprite_priorities_in_fb_line[y][sprite_priorities[0]] = 1;
							continue;
						};
						b = (pix & 0x7c00) >> 10;
						g = (pix & 0x03e0) >> 5;
						r = (pix & 0x1f);
						if ( color_offset_pal )
						{
							stv_vdp2_compute_color_offset_RGB555( &r, &g, &b, STV_VDP2_SPCOSL );
						}
						bitmap_line[x] = b | g << 5 | r << 10;
					}
					else
					{
						priority = sprite_priorities[(pix >> sprite_priority_shift) & sprite_priority_mask];
						if ( priority != pri )
						{
							stv_sprite_priorities_used[priority] = 1;
							stv_sprite_priorities_in_fb_line[y][priority] = 1;
							continue;
						};

						pix &= sprite_colormask;
						if ( pix == (sprite_colormask - 1) )
						{
							/*shadow - in reality, we should check from what layer pixel beneath comes...*/
							if ( STV_VDP2_SDCTL & 0x3f )
							{
								bitmap_line[x] = (bitmap_line[x] & ~0x421) >> 1;
							}
							/* note that when shadows are disabled, "shadow" palette entries are not drawn */
						}
						else if ( pix )
						{
							pix += (STV_VDP2_SPCAOS << 8);
							pix &= 0x7ff;
							pix += color_offset_pal;
							bitmap_line[x] = machine->pens[ pix ];
						}
					}
				}
			}
		}
		else //alpha_enabled == 1
		{
			for ( y = mycliprect.min_y; y <= mycliprect.max_y; y++ )
			{
				if ( stv_sprite_priorities_usage_valid )
					if (stv_sprite_priorities_in_fb_line[y][pri] == 0)
						continue;

				framebuffer_line = stv_framebuffer_display_lines[y];
				bitmap_line = BITMAP_ADDR16(bitmap, y, 0);

				for ( x = mycliprect.min_x; x <= mycliprect.max_x; x++ )
				{
					pix = framebuffer_line[x];
					if ( (pix & 0x8000) && sprite_color_mode)
					{
						if ( sprite_priorities[0] != pri )
						{
							stv_sprite_priorities_used[sprite_priorities[0]] = 1;
							stv_sprite_priorities_in_fb_line[y][sprite_priorities[0]] = 1;
							continue;
						};

						b = (pix & 0x7c00) >> 10;
						g = (pix & 0x03e0) >> 5;
						r = (pix & 0x1f);
						if ( color_offset_pal )
						{
							stv_vdp2_compute_color_offset_RGB555( &r, &g, &b, STV_VDP2_SPCOSL );
						}
						ccr = sprite_ccr[0];
						if ( STV_VDP2_CCMD )
						{
							bitmap_line[x] = stv_add_blend( bitmap_line[x], b | g << 5 | r << 10 );
						}
						else
						{
							bitmap_line[x] = alpha_blend_r16( bitmap_line[x], b | g << 5 | r << 10, ((UINT16)(0x1f-ccr)*0xff)/0x1f);
						}
					}
					else
					{
						priority = sprite_priorities[(pix >> sprite_priority_shift) & sprite_priority_mask];
						if ( priority != pri )
						{
							stv_sprite_priorities_used[priority] = 1;
							stv_sprite_priorities_in_fb_line[y][priority] = 1;
							continue;
						};

						ccr = sprite_ccr[ (pix >> sprite_ccrr_shift) & sprite_ccrr_mask ];

						pix &= sprite_colormask;
						if ( pix == (sprite_colormask - 1) )
						{
							/*shadow - in reality, we should check from what layer pixel beneath comes...*/
							if ( STV_VDP2_SDCTL & 0x3f )
							{
								bitmap_line[x] = (bitmap_line[x] & ~0x421) >> 1;
							}
							/* note that when shadows are disabled, "shadow" palette entries are not drawn */
						} else if ( pix )
						{
							pix += (STV_VDP2_SPCAOS << 8);
							pix &= 0x7ff;
							pix += color_offset_pal;
							if ( ccr > 0 )
							{
								if ( STV_VDP2_CCMD )
								{
									bitmap_line[x] = stv_add_blend( bitmap_line[x], machine->pens[pix] );
								}
								else
								{
									bitmap_line[x] = alpha_blend_r16( bitmap_line[x], machine->pens[pix], ((UINT16)(0x1f-ccr)*0xff)/0x1f );
								}
							}
							else
								bitmap_line[x] = machine->pens[pix];
						}
					}
				}
			}
		}
	}
	else
	{
		for ( y = mycliprect.min_y; y <= mycliprect.max_y; y++ )
		{
			if ( stv_sprite_priorities_usage_valid )
				if (stv_sprite_priorities_in_fb_line[y][pri] == 0)
					continue;

			framebuffer_line = stv_framebuffer_display_lines[y];
			if ( interlace_framebuffer == 0 )
			{
				bitmap_line = BITMAP_ADDR16(bitmap, y, 0);
			}
			else
			{
				bitmap_line = BITMAP_ADDR16(bitmap, 2*y, 0);
				bitmap_line2 = BITMAP_ADDR16(bitmap, 2*y + 1, 0);
			}

			for ( x = mycliprect.min_x; double_x ? x <= ((mycliprect.max_x)/2) : (x <= mycliprect.max_x); x++ )
			{
				pix = framebuffer_line[x];
				if ( (pix & 0x8000) && sprite_color_mode)
				{
					if ( sprite_priorities[0] != pri )
					{
						stv_sprite_priorities_used[sprite_priorities[0]] = 1;
						stv_sprite_priorities_in_fb_line[y][sprite_priorities[0]] = 1;
						continue;
					};
					b = (pix & 0x7c00) >> 10;
					g = (pix & 0x03e0) >> 5;
					r = (pix & 0x1f);
					if ( color_offset_pal )
					{
						stv_vdp2_compute_color_offset_RGB555( &r, &g, &b, STV_VDP2_SPCOSL );
					}
					if ( alpha_enabled == 0 )
					{
						if(double_x)
						{
							bitmap_line[x*2] = b | g << 5 | r << 10;
							if ( interlace_framebuffer == 1 ) bitmap_line2[x*2] = b | g << 5 | r << 10;
							bitmap_line[x*2+1] = b | g << 5 | r << 10;
							if ( interlace_framebuffer == 1 ) bitmap_line2[x*2+1] = b | g << 5 | r << 10;
						}
						else
						{
							bitmap_line[x] = b | g << 5 | r << 10;
							if ( interlace_framebuffer == 1 ) bitmap_line2[x] = b | g << 5 | r << 10;
						}
					}
					else // alpha_blend == 1
					{
						ccr = sprite_ccr[0];

						if ( STV_VDP2_CCMD )
						{
							if(double_x)
							{
								bitmap_line[x*2] = stv_add_blend( bitmap_line[x*2], b | g << 5 | r << 10 );
								if ( interlace_framebuffer == 1 ) bitmap_line2[x*2] = stv_add_blend( bitmap_line2[x*2], b | g << 5 | r << 10 );
								bitmap_line[x*2+1] = stv_add_blend( bitmap_line[x*2+1], b | g << 5 | r << 10 );
								if ( interlace_framebuffer == 1 ) bitmap_line2[x*2+1] = stv_add_blend( bitmap_line2[x*2+1], b | g << 5 | r << 10 );
							}
							else
							{
								bitmap_line[x] = stv_add_blend( bitmap_line[x], b | g << 5 | r << 10 );
								if ( interlace_framebuffer == 1 ) bitmap_line2[x] = stv_add_blend( bitmap_line2[x], b | g << 5 | r << 10 );
							}
						}
						else
						{
							if(double_x)
							{
								bitmap_line[x*2] = alpha_blend_r16( bitmap_line[x*2], b | g << 5 | r << 10, ((UINT16)(0x1f-ccr)*0xff)/0x1f );
								if ( interlace_framebuffer == 1 ) bitmap_line2[x*2] = alpha_blend_r16( bitmap_line2[x*2], b | g << 5 | r << 10, ((UINT16)(0x1f-ccr)*0xff)/0x1f );
								bitmap_line[x*2+1] = alpha_blend_r16( bitmap_line[x*2+1], b | g << 5 | r << 10, ((UINT16)(0x1f-ccr)*0xff)/0x1f );
								if ( interlace_framebuffer == 1 ) bitmap_line2[x*2+1] = alpha_blend_r16( bitmap_line2[x*2+1], b | g << 5 | r << 10, ((UINT16)(0x1f-ccr)*0xff)/0x1f);
							}
							else
							{
								bitmap_line[x] = alpha_blend_r16( bitmap_line[x], b | g << 5 | r << 10, ((UINT16)(0x1f-ccr)*0xff)/0x1f);
								if ( interlace_framebuffer == 1 ) bitmap_line2[x] = alpha_blend_r16( bitmap_line2[x], b | g << 5 | r << 10, ((UINT16)(0x1f-ccr)*0xff)/0x1f);
							}
						}
					}
				}
				else
				{
					priority = sprite_priorities[(pix >> sprite_priority_shift) & sprite_priority_mask];
					if ( priority != pri )
					{
						stv_sprite_priorities_used[priority] = 1;
						stv_sprite_priorities_in_fb_line[y][priority] = 1;
						continue;
					};

					if ( alpha_enabled )
						ccr = sprite_ccr[ (pix >> sprite_ccrr_shift) & sprite_ccrr_mask ];

					pix &= sprite_colormask;
					if ( pix == (sprite_colormask - 1) )
					{
						/*shadow - in reality, we should check from what layer pixel beneath comes...*/
						if ( STV_VDP2_SDCTL & 0x3f )
						{
							bitmap_line[x] = (bitmap_line[x] & ~0x421) >> 1;
						}
						/* note that when shadows are disabled, "shadow" palette entries are not drawn */
					} else if ( pix )
					{
						pix += (STV_VDP2_SPCAOS << 8);
						pix &= 0x7ff;
						pix += color_offset_pal;
						if ( alpha_enabled == 0 )
						{
							if(double_x)
							{
								bitmap_line[x*2] = machine->pens[ pix ];
								if ( interlace_framebuffer == 1 ) bitmap_line2[x*2] = machine->pens[ pix ];
								bitmap_line[x*2+1] = machine->pens[ pix ];
								if ( interlace_framebuffer == 1 ) bitmap_line2[x*2+1] = machine->pens[ pix ];
							}
							else
							{
								bitmap_line[x] = machine->pens[ pix ];
								if ( interlace_framebuffer == 1 ) bitmap_line2[x] = machine->pens[ pix ];
							}
						}
						else // alpha_blend == 1
						{
							if ( STV_VDP2_CCMD )
							{
								if(double_x)
								{
									bitmap_line[x*2] = stv_add_blend( bitmap_line[x*2], machine->pens[pix] );
									if ( interlace_framebuffer == 1 ) bitmap_line2[x*2] = stv_add_blend( bitmap_line2[x], machine->pens[pix] );
									bitmap_line[x*2+1] = stv_add_blend( bitmap_line[x*2+1], machine->pens[pix] );
									if ( interlace_framebuffer == 1 ) bitmap_line2[x*2+1] = stv_add_blend( bitmap_line2[x], machine->pens[pix] );
								}
								else
								{
									bitmap_line[x] = stv_add_blend( bitmap_line[x], machine->pens[pix] );
									if ( interlace_framebuffer == 1 ) bitmap_line2[x] = stv_add_blend( bitmap_line2[x], machine->pens[pix] );
								}
							}
							else
							{
								if(double_x)
								{
									bitmap_line[x*2] = alpha_blend_r16( bitmap_line[x*2], machine->pens[pix], ((UINT16)(0x1f-ccr)*0xff)/0x1f );
									if ( interlace_framebuffer == 1 ) bitmap_line2[x*2] = alpha_blend_r16( bitmap_line2[x], machine->pens[pix], ((UINT16)(0x1f-ccr)*0xff)/0x1f );
									bitmap_line[x*2+1] = alpha_blend_r16( bitmap_line[x*2+1], machine->pens[pix], ((UINT16)(0x1f-ccr)*0xff)/0x1f );
									if ( interlace_framebuffer == 1 ) bitmap_line2[x*2+1] = alpha_blend_r16( bitmap_line2[x], machine->pens[pix], ((UINT16)(0x1f-ccr)*0xff)/0x1f );
								}
								else
								{
									bitmap_line[x] = alpha_blend_r16( bitmap_line[x], machine->pens[pix], ((UINT16)(0x1f-ccr)*0xff)/0x1f );
									if ( interlace_framebuffer == 1 ) bitmap_line2[x] = alpha_blend_r16( bitmap_line2[x], machine->pens[pix], ((UINT16)(0x1f-ccr)*0xff)/0x1f );
								}
							}
						}
					}
				}
			}
		}
	}

	stv_sprite_priorities_usage_valid = 1;
}

VIDEO_UPDATE( stv_vdp2 )
{
	static UINT8 pri;
	video_update_vdp1(screen->machine);

	stv_vdp2_fade_effects(screen->machine);

	stv_vdp2_draw_back(screen->machine, bitmap,cliprect);

	#if DEBUG_MODE
	if(input_code_pressed_once(screen->machine, KEYCODE_T))
	{
		debug.l_en^=1;
		popmessage("NBG3 %sabled",debug.l_en & 1 ? "en" : "dis");
	}
	if(input_code_pressed_once(screen->machine, KEYCODE_Y))
	{
		debug.l_en^=2;
		popmessage("NBG2 %sabled",debug.l_en & 2 ? "en" : "dis");
	}
	if(input_code_pressed_once(screen->machine, KEYCODE_U))
	{
		debug.l_en^=4;
		popmessage("NBG1 %sabled",debug.l_en & 4 ? "en" : "dis");
	}
	if(input_code_pressed_once(screen->machine, KEYCODE_I))
	{
		debug.l_en^=8;
		popmessage("NBG0 %sabled",debug.l_en & 8 ? "en" : "dis");
	}
	if(input_code_pressed_once(screen->machine, KEYCODE_K))
	{
		debug.l_en^=0x10;
		popmessage("RBG0 %sabled",debug.l_en & 0x10 ? "en" : "dis");
	}
	if(input_code_pressed_once(screen->machine, KEYCODE_O))
	{
		debug.l_en^=0x20;
		popmessage("SPRITE %sabled",debug.l_en & 0x20 ? "en" : "dis");
	}
	#endif

	if(STV_VDP2_DISP != 0)
	{
		stv_sprite_priorities_usage_valid = 0;
		memset(stv_sprite_priorities_used, 0, sizeof(stv_sprite_priorities_used));
		memset(stv_sprite_priorities_in_fb_line, 0, sizeof(stv_sprite_priorities_in_fb_line));

		/*If a plane has a priority value of zero it isn't shown at all.*/
		for(pri=1;pri<8;pri++)
		{
			if (debug.l_en & 1)    { if(pri==STV_VDP2_N3PRIN) stv_vdp2_draw_NBG3(screen->machine, bitmap,cliprect); }
			if (debug.l_en & 2)    { if(pri==STV_VDP2_N2PRIN) stv_vdp2_draw_NBG2(screen->machine, bitmap,cliprect); }
			if (debug.l_en & 4)    { if(pri==STV_VDP2_N1PRIN) stv_vdp2_draw_NBG1(screen->machine, bitmap,cliprect); }
			if (debug.l_en & 8)    { if(pri==STV_VDP2_N0PRIN) stv_vdp2_draw_NBG0(screen->machine, bitmap,cliprect); }
			if (debug.l_en & 0x10) { if(pri==STV_VDP2_R0PRIN) stv_vdp2_draw_RBG0(screen->machine, bitmap,cliprect); }
			if (debug.l_en & 0x20) { draw_sprites(screen->machine,bitmap,cliprect,pri); }
		}
	}

#if DEBUG_MODE
	if(STV_VDP2_VRAMSZ && VDP2_ERR(0x80000000))
	{
		VDP2_CHK(0x80000000);
		mame_printf_debug("Warning: VRAM Size = 8 MBit!\n");
	}
	if(STV_VDP2_CRKTE && VDP2_ERR(0x40000000))
	{
		VDP2_CHK(0x40000000);
		mame_printf_debug("Warning: Color RAM Coefficient Table Ctrl used\n");
	}

	/*popmessage("N0 %02x %04x %02x %04x N1 %02x %04x %02x %04x"
    ,STV_VDP2_N0ZMXI,STV_VDP2_N0ZMXD
    ,STV_VDP2_N0ZMYI,STV_VDP2_N0ZMYD
    ,STV_VDP2_N1ZMXI,STV_VDP2_N1ZMXD
    ,STV_VDP2_N1ZMYI,STV_VDP2_N1ZMYD);*/

	if ( input_code_pressed_once(screen->machine, KEYCODE_W) )
	{
		int tilecode;

		for (tilecode = 0;tilecode<0x8000;tilecode++)
		{
			gfx_element_mark_dirty(screen->machine->gfx[0], tilecode);
		}

		for (tilecode = 0;tilecode<0x2000;tilecode++)
		{
			gfx_element_mark_dirty(screen->machine->gfx[1], tilecode);
		}

		for (tilecode = 0;tilecode<0x4000;tilecode++)
		{
			gfx_element_mark_dirty(screen->machine->gfx[2], tilecode);
		}

		for (tilecode = 0;tilecode<0x1000;tilecode++)
		{
			gfx_element_mark_dirty(screen->machine->gfx[3], tilecode);
		}

		/* vdp 1 ... doesn't have to be tile based */

		for (tilecode = 0;tilecode<0x8000;tilecode++)
		{
			gfx_element_mark_dirty(screen->machine->gfx[4], tilecode);
		}
		for (tilecode = 0;tilecode<0x2000;tilecode++)
		{
			gfx_element_mark_dirty(screen->machine->gfx[5], tilecode);
		}
		for (tilecode = 0;tilecode<0x4000;tilecode++)
		{
			gfx_element_mark_dirty(screen->machine->gfx[6], tilecode);
		}
		for (tilecode = 0;tilecode<0x1000;tilecode++)
		{
			gfx_element_mark_dirty(screen->machine->gfx[7], tilecode);
		}
	}

	if ( input_code_pressed_once(screen->machine, KEYCODE_N) )
	{
		FILE *fp;

		fp=fopen("mamevdp1", "w+b");
		if (fp)
		{
			fwrite(stv_vdp1_vram, 0x80000, 1, fp);
			fclose(fp);
		}
	}

	if ( input_code_pressed_once(screen->machine, KEYCODE_M) )
	{
		FILE *fp;

		fp=fopen("vdp1_vram.bin", "r+b");
		if (fp)
		{
			fread(stv_vdp1_vram, 0x80000, 1, fp);
			fclose(fp);
		}
	}

#endif



	return 0;
}

/* below is some old code we might use .. */

#if 0

static void stv_dump_ram()
{
	FILE *fp;

	fp=fopen("workraml.dmp", "w+b");
	if (fp)
	{
		fwrite(stv_workram_l, 0x00100000, 1, fp);
		fclose(fp);
	}
	fp=fopen("workramh.dmp", "w+b");
	if (fp)
	{
		fwrite(stv_workram_h, 0x00100000, 1, fp);
		fclose(fp);
	}
	fp=fopen("scu.dmp", "w+b");
	if (fp)
	{
		fwrite(stv_scu, 0xd0, 1, fp);
		fclose(fp);
	}
	fp=fopen("stv_a0_vram.dmp", "w+b");
	if (fp)
	{
		fwrite(stv_a0_vram, 0x00020000, 1, fp);
		fclose(fp);
	}
	fp=fopen("stv_a1_vram.dmp", "w+b");
	if (fp)
	{
		fwrite(stv_a1_vram, 0x00020000, 1, fp);
		fclose(fp);
	}
	fp=fopen("stv_b0_vram.dmp", "w+b");
	if (fp)
	{
		fwrite(stv_b0_vram, 0x00020000, 1, fp);
		fclose(fp);
	}
	fp=fopen("stv_b1_vram.dmp", "w+b");
	if (fp)
	{
		fwrite(stv_b1_vram, 0x00020000, 1, fp);
		fclose(fp);
	}
	fp=fopen("cram.dmp", "w+b");
	if (fp)
	{
		fwrite(stv_cram, 0x00080000, 1, fp);
		fclose(fp);
	}
	fp=fopen("68k.dmp", "w+b");
	if (fp)
	{
		fwrite(memory_region(machine, REGION_CPU3), 0x100000, 1, fp);
		fclose(fp);
	}
}


#endif
