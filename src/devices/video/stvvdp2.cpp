// license:LGPL-2.1+
// copyright-holders:David Haywood, Angelo Salese, Olivier Galibert, Mariusz Wojcieszek, R. Belmont
/* Sega Saturn VDP2 */

#define DEBUG_MODE 0
#define TEST_FUNCTIONS 0
#define POPMESSAGE_DEBUG 0

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

#include "emu.h"
#include "includes/stv.h"

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
| Plane Size       | 1 H x 1 V 1 Pages ; 2 H x 1 V 1 Pages ; 2 H x 2 V Pages                                 |
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

	#define STV_VDP2_TVMD   (m_vdp2_regs[0x000/2])

	#define STV_VDP2_DISP   ((STV_VDP2_TVMD & 0x8000) >> 15)
	#define STV_VDP2_BDCLMD ((STV_VDP2_TVMD & 0x0100) >> 8)
	#define STV_VDP2_LSMD   ((STV_VDP2_TVMD & 0x00c0) >> 6)
	#define STV_VDP2_VRES   ((STV_VDP2_TVMD & 0x0030) >> 4)
	#define STV_VDP2_HRES   ((STV_VDP2_TVMD & 0x0007) >> 0)

/* 180002 - r/w - EXTEN - External Signal Enable Register
 bit-> /----15----|----14----|----13----|----12----|----11----|----10----|----09----|----08----\
       |    --    |    --    |    --    |    --    |    --    |    --    | EXLTEN   | EXSYEN   |
       |----07----|----06----|----05----|----04----|----03----|----02----|----01----|----00----|
       |    --    |    --    |    --    |    --    |    --    |    --    | DASEL    | EXBGEN   |
       \----------|----------|----------|----------|----------|----------|----------|---------*/

	#define STV_VDP2_EXTEN  (m_vdp2_regs[0x002/2])

	#define STV_VDP2_EXLTEN ((STV_VDP2_EXTEN & 0x0200) >> 9)

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

	#define STV_VDP2_VRSIZE (m_vdp2_regs[0x006/2])

	#define STV_VDP2_VRAMSZ ((STV_VDP2_VRSIZE & 0x8000) >> 15)

/* 180008 - r/o - HCNT - H-Counter
 bit-> /----15----|----14----|----13----|----12----|----11----|----10----|----09----|----08----\
       |    --    |    --    |    --    |    --    |    --    |    --    | HCT9     | HCT8     |
       |----07----|----06----|----05----|----04----|----03----|----02----|----01----|----00----|
       | HCT7     | HCT6     | HCT5     | HCT4     | HCT3     | HCT2     | HCT1     | HCT0     |
       \----------|----------|----------|----------|----------|----------|----------|---------*/

	#define STV_VDP2_HCNT (m_vdp2_regs[0x008/2])

/* 18000A - r/o - VCNT - V-Counter
 bit-> /----15----|----14----|----13----|----12----|----11----|----10----|----09----|----08----\
       |    --    |    --    |    --    |    --    |    --    |    --    | VCT9     | VCT8     |
       |----07----|----06----|----05----|----04----|----03----|----02----|----01----|----00----|
       | VCT7     | VCT6     | VCT5     | VCT4     | VCT3     | VCT2     | VCT1     | VCT0     |
       \----------|----------|----------|----------|----------|----------|----------|---------*/

	#define STV_VDP2_VCNT (m_vdp2_regs[0x00a/2])

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

	#define STV_VDP2_RAMCTL (m_vdp2_regs[0x00e/2])

	#define STV_VDP2_CRKTE ((STV_VDP2_RAMCTL & 0x8000) >> 15)
	#define STV_VDP2_CRMD  ((STV_VDP2_RAMCTL & 0x3000) >> 12)
	#define STV_VDP2_RDBSB1 ((STV_VDP2_RAMCTL & 0x00c0) >> 6)
	#define STV_VDP2_RDBSB0 ((STV_VDP2_RAMCTL & 0x0030) >> 4)
	#define STV_VDP2_RDBSA1 ((STV_VDP2_RAMCTL & 0x000c) >> 2)
	#define STV_VDP2_RDBSA0 ((STV_VDP2_RAMCTL & 0x0003) >> 0)


/* 180010 - r/w - -CYCA0L - VRAM CYCLE PATTERN (BANK A0)
 bit-> /----15----|----14----|----13----|----12----|----11----|----10----|----09----|----08----\
       | VCP0A03  | VCP0A02  | VCP0A01  | VCP0A00  | VCP1A03  | VCP1A02  | VCP1A01  | VCP1A00  |
       |----07----|----06----|----05----|----04----|----03----|----02----|----01----|----00----|
       | VCP2A03  | VCP2A02  | VCP2A01  | VCP2A00  | VCP3A03  | VCP3A02  | VCP3A01  | VCP3A00  |
       \----------|----------|----------|----------|----------|----------|----------|---------*/

	#define STV_VDP2_CYCA0L (m_vdp2_regs[0x010/2])

/* 180012 - r/w - -CYCA0U - VRAM CYCLE PATTERN (BANK A0)
 bit-> /----15----|----14----|----13----|----12----|----11----|----10----|----09----|----08----\
       | VCP4A03  | VCP4A02  | VCP4A01  | VCP4A00  | VCP5A03  | VCP5A02  | VCP5A01  | VCP5A00  |
       |----07----|----06----|----05----|----04----|----03----|----02----|----01----|----00----|
       | VCP6A03  | VCP6A02  | VCP6A01  | VCP6A00  | VCP7A03  | VCP7A02  | VCP7A01  | VCP7A00  |
       \----------|----------|----------|----------|----------|----------|----------|---------*/

	#define STV_VDP2_CYCA0U (m_vdp2_regs[0x012/2])

/* 180014 - r/w - -CYCA1L - VRAM CYCLE PATTERN (BANK A1)
 bit-> /----15----|----14----|----13----|----12----|----11----|----10----|----09----|----08----\
       | VCP0A13  | VCP0A12  | VCP0A11  | VCP0A10  | VCP1A13  | VCP1A12  | VCP1A11  | VCP1A10  |
       |----07----|----06----|----05----|----04----|----03----|----02----|----01----|----00----|
       | VCP2A13  | VCP2A12  | VCP2A11  | VCP2A10  | VCP3A13  | VCP3A12  | VCP3A11  | VCP3A10  |
       \----------|----------|----------|----------|----------|----------|----------|---------*/

	#define STV_VDP2_CYCA1L (m_vdp2_regs[0x014/2])

/* 180016 - r/w - -CYCA1U - VRAM CYCLE PATTERN (BANK A1)
 bit-> /----15----|----14----|----13----|----12----|----11----|----10----|----09----|----08----\
       | VCP4A13  | VCP4A12  | VCP4A11  | VCP4A10  | VCP5A13  | VCP5A12  | VCP5A11  | VCP5A10  |
       |----07----|----06----|----05----|----04----|----03----|----02----|----01----|----00----|
       | VCP6A13  | VCP6A12  | VCP6A11  | VCP6A10  | VCP7A13  | VCP7A12  | VCP7A11  | VCP7A10  |
       \----------|----------|----------|----------|----------|----------|----------|---------*/

	#define STV_VDP2_CYCA1U (m_vdp2_regs[0x016/2])

/* 180018 - r/w - -CYCB0L - VRAM CYCLE PATTERN (BANK B0)
 bit-> /----15----|----14----|----13----|----12----|----11----|----10----|----09----|----08----\
       | VCP0B03  | VCP0B02  | VCP0B01  | VCP0B00  | VCP1B03  | VCP1B02  | VCP1B01  | VCP1B00  |
       |----07----|----06----|----05----|----04----|----03----|----02----|----01----|----00----|
       | VCP2B03  | VCP2B02  | VCP2B01  | VCP2B00  | VCP3B03  | VCP3B02  | VCP3B01  | VCP3B00  |
       \----------|----------|----------|----------|----------|----------|----------|---------*/

	#define STV_VDP2_CYCA2L (m_vdp2_regs[0x018/2])

/* 18001A - r/w - -CYCB0U - VRAM CYCLE PATTERN (BANK B0)
 bit-> /----15----|----14----|----13----|----12----|----11----|----10----|----09----|----08----\
       | VCP4B03  | VCP4B02  | VCP4B01  | VCP4B00  | VCP5B03  | VCP5B02  | VCP5B01  | VCP5B00  |
       |----07----|----06----|----05----|----04----|----03----|----02----|----01----|----00----|
       | VCP6B03  | VCP6B02  | VCP6B01  | VCP6B00  | VCP7B03  | VCP7B02  | VCP7B01  | VCP7B00  |
       \----------|----------|----------|----------|----------|----------|----------|---------*/

	#define STV_VDP2_CYCA2U (m_vdp2_regs[0x01a/2])

/* 18001C - r/w - -CYCB1L - VRAM CYCLE PATTERN (BANK B1)
 bit-> /----15----|----14----|----13----|----12----|----11----|----10----|----09----|----08----\
       | VCP0B13  | VCP0B12  | VCP0B11  | VCP0B10  | VCP1B13  | VCP1B12  | VCP1B11  | VCP1B10  |
       |----07----|----06----|----05----|----04----|----03----|----02----|----01----|----00----|
       | VCP2B13  | VCP2B12  | VCP2B11  | VCP2B10  | VCP3B13  | VCP3B12  | VCP3B11  | VCP3B10  |
       \----------|----------|----------|----------|----------|----------|----------|---------*/

	#define STV_VDP2_CYCA3L (m_vdp2_regs[0x01c/2])

/* 18001E - r/w - -CYCB1U - VRAM CYCLE PATTERN (BANK B1)
 bit-> /----15----|----14----|----13----|----12----|----11----|----10----|----09----|----08----\
       | VCP4B13  | VCP4B12  | VCP4B11  | VCP4B10  | VCP5B13  | VCP5B12  | VCP5B11  | VCP5B10  |
       |----07----|----06----|----05----|----04----|----03----|----02----|----01----|----00----|
       | VCP6B13  | VCP6B12  | VCP6B11  | VCP6B10  | VCP7B13  | VCP7B12  | VCP7B11  | VCP7B10  |
       \----------|----------|----------|----------|----------|----------|----------|---------*/

	#define STV_VDP2_CYCA3U (m_vdp2_regs[0x01e/2])

/* 180020 - r/w - BGON - SCREEN DISPLAY ENABLE

 this register allows each tilemap to be enabled or disabled and also which layers are solid

 bit-> /----15----|----14----|----13----|----12----|----11----|----10----|----09----|----08----\
       |    --    |    --    |    --    | R0TPON   | N3TPON   | N2TPON   | N1TPON   | N0TPON   |
       |----07----|----06----|----05----|----04----|----03----|----02----|----01----|----00----|
       |    --    |    --    | R1ON     | R0ON     | N3ON     | N2ON     | N1ON     | N0ON     |
       \----------|----------|----------|----------|----------|----------|----------|---------*/

	#define STV_VDP2_BGON (m_vdp2_regs[0x020/2])

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

	#define STV_VDP2_MZCTL (m_vdp2_regs[0x022/2])

	#define STV_VDP2_MZSZV ((STV_VDP2_MZCTL & 0xf000) >> 12)
	#define STV_VDP2_MZSZH ((STV_VDP2_MZCTL & 0x0f00) >> 8)
	#define STV_VDP2_R0MZE ((STV_VDP2_MZCTL & 0x0010) >> 4)
	#define STV_VDP2_N3MZE ((STV_VDP2_MZCTL & 0x0008) >> 3)
	#define STV_VDP2_N2MZE ((STV_VDP2_MZCTL & 0x0004) >> 2)
	#define STV_VDP2_N1MZE ((STV_VDP2_MZCTL & 0x0002) >> 1)
	#define STV_VDP2_N0MZE ((STV_VDP2_MZCTL & 0x0001) >> 0)

/*180024 - Special Function Code Select

*/

	#define STV_VDP2_SFSEL (m_vdp2_regs[0x024/2])

/*180026 - Special Function Code

*/

	#define STV_VDP2_SFCODE (m_vdp2_regs[0x026/2])


/*
180028 - CHCTLA - Character Control (NBG0, NBG1)
 bit-> /----15----|----14----|----13----|----12----|----11----|----10----|----09----|----08----\
       |    --    |    --    | N1CHCN1  | N1CHCN0  | N1BMSZ1  | N1BMSZ0  | N1BMEN   | N1CHSZ   |
       |----07----|----06----|----05----|----04----|----03----|----02----|----01----|----00----|
       |    --    | N0CHCN2  | N0CHCN1  | N0CHCN0  | N0BMSZ1  | N0BMSZ0  | N0BMEN   | N0CHSZ   |
       \----------|----------|----------|----------|----------|----------|----------|---------*/

	#define STV_VDP2_CHCTLA (m_vdp2_regs[0x028/2])

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

	#define STV_VDP2_CHCTLB (m_vdp2_regs[0x02a/2])

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

/*  R0CHSZ - RBG0 Character (Tile) Size
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

	#define STV_VDP2_BMPNA (m_vdp2_regs[0x02c/2])

	#define STV_VDP2_N1BMP ((STV_VDP2_BMPNA & 0x0700) >> 8)
	#define STV_VDP2_N0BMP ((STV_VDP2_BMPNA & 0x0007) >> 0)

/* 18002E - Bitmap Palette Number (RBG0)
 bit-> /----15----|----14----|----13----|----12----|----11----|----10----|----09----|----08----\
       |    --    |    --    |    --    |    --    |    --    |    --    |    --    |    --    |
       |----07----|----06----|----05----|----04----|----03----|----02----|----01----|----00----|
       |    --    |    --    |    --    |    --    |    --    |    --    |    --    |    --    |
       \----------|----------|----------|----------|----------|----------|----------|---------*/

	#define STV_VDP2_BMPNB (m_vdp2_regs[0x02e/2])

	#define STV_VDP2_R0BMP ((STV_VDP2_BMPNB & 0x0007) >> 0)

/* 180030 - PNCN0 - Pattern Name Control (NBG0)
 bit-> /----15----|----14----|----13----|----12----|----11----|----10----|----09----|----08----\
       | N0PNB    | N0CNSM   |    --    |    --    |    --    |    --    | N0SPR    | N0SCC    |
       |----07----|----06----|----05----|----04----|----03----|----02----|----01----|----00----|
       | N0SPLT6  | N0SPLT5  | N0SPLT4  | N0SPCN4  | N0SPCN3  | N0SPCN2  | N0SPCN1  | N0SPCN0  |
       \----------|----------|----------|----------|----------|----------|----------|---------*/

	#define STV_VDP2_PNCN0 (m_vdp2_regs[0x030/2])

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

	#define STV_VDP2_PNCN1 (m_vdp2_regs[0x032/2])

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

	#define STV_VDP2_PNCN2 (m_vdp2_regs[0x034/2])

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

	#define STV_VDP2_PNCN3 (m_vdp2_regs[0x036/2])

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

	#define STV_VDP2_PNCR (m_vdp2_regs[0x038/2])

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

/* 18003A - PLSZ - Plane Size
 bit-> /----15----|----14----|----13----|----12----|----11----|----10----|----09----|----08----\
       |    --    |    --    |    --    |    --    |    --    |    --    |    --    |    --    |
       |----07----|----06----|----05----|----04----|----03----|----02----|----01----|----00----|
       | N3PLSZ1  | N3PLSZ0  |    --    |    --    | N1PLSZ1  | N1PLSZ0  | N0PLSZ1  | N0PLSZ0  |
       \----------|----------|----------|----------|----------|----------|----------|---------*/

	#define STV_VDP2_PLSZ (m_vdp2_regs[0x03a/2])

	/* NBG0 Plane Size
	00 1H Page x 1V Page
	01 2H Pages x 1V Page
	10 invalid
	11 2H Pages x 2V Pages  */
	#define STV_VDP2_RBOVR  ((STV_VDP2_PLSZ & 0xc000) >> 14)
	#define STV_VDP2_RBPLSZ ((STV_VDP2_PLSZ & 0x3000) >> 12)
	#define STV_VDP2_RAOVR  ((STV_VDP2_PLSZ & 0x0c00) >> 10)
	#define STV_VDP2_RAPLSZ ((STV_VDP2_PLSZ & 0x0300) >> 8)
	#define STV_VDP2_N3PLSZ ((STV_VDP2_PLSZ & 0x00c0) >> 6)
	#define STV_VDP2_N2PLSZ ((STV_VDP2_PLSZ & 0x0030) >> 4)
	#define STV_VDP2_N1PLSZ ((STV_VDP2_PLSZ & 0x000c) >> 2)
	#define STV_VDP2_N0PLSZ ((STV_VDP2_PLSZ & 0x0003) >> 0)

/* 18003C - MPOFN - Map Offset (NBG0, NBG1, NBG2, NBG3)
 bit-> /----15----|----14----|----13----|----12----|----11----|----10----|----09----|----08----\
       |    --    | N3MP8    | N3MP7    | N3MP6    |    --    | N2MP8    | N2MP7    | N2MP6    |
       |----07----|----06----|----05----|----04----|----03----|----02----|----01----|----00----|
       |    --    | N1MP8    | N1MP7    | N1MP6    |    --    | N0MP8    | N0MP7    | N0MP6    |
       \----------|----------|----------|----------|----------|----------|----------|---------*/

	#define STV_VDP2_MPOFN_ (m_vdp2_regs[0x03c/2])

	/* Higher 3 bits of the map offset for each layer */
	#define STV_VDP2_N3MP_ ((STV_VDP2_MPOFN_ & 0x3000) >> 12)
	#define STV_VDP2_N2MP_ ((STV_VDP2_MPOFN_ & 0x0300) >> 8)
	#define STV_VDP2_N1MP_ ((STV_VDP2_MPOFN_ & 0x0030) >> 4)
	#define STV_VDP2_N0MP_ ((STV_VDP2_MPOFN_ & 0x0003) >> 0)




/* 18003E - Map Offset (Rotation Parameter A,B)
 bit-> /----15----|----14----|----13----|----12----|----11----|----10----|----09----|----08----\
       |    --    |    --    |    --    |    --    |    --    |    --    |    --    |    --    |
       |----07----|----06----|----05----|----04----|----03----|----02----|----01----|----00----|
       |    --    |    --    |    --    |    --    |    --    |    --    |    --    |    --    |
       \----------|----------|----------|----------|----------|----------|----------|---------*/

	#define STV_VDP2_MPOFR_ (m_vdp2_regs[0x03e/2])

	#define STV_VDP2_RBMP_ ((STV_VDP2_MPOFR_ & 0x0030) >> 4)
	#define STV_VDP2_RAMP_ ((STV_VDP2_MPOFR_ & 0x0003) >> 0)

/* 180040 - MPABN0 - Map (NBG0, Plane A,B)
 bit-> /----15----|----14----|----13----|----12----|----11----|----10----|----09----|----08----\
       |    --    |    --    | N0MPB5   | N0MPB4   | N0MPB3   | N0MPB2   | N0MPB1   | N0MPB0   |
       |----07----|----06----|----05----|----04----|----03----|----02----|----01----|----00----|
       |    --    |    --    | N0MPA5   | N0MPA4   | N0MPA3   | N0MPA2   | N0MPA1   | N0MPA0   |
       \----------|----------|----------|----------|----------|----------|----------|---------*/

	#define STV_VDP2_MPABN0 (m_vdp2_regs[0x040/2])

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

	#define STV_VDP2_MPCDN0 (m_vdp2_regs[0x042/2])

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

	#define STV_VDP2_MPABN1 (m_vdp2_regs[0x044/2])

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

	#define STV_VDP2_MPCDN1 (m_vdp2_regs[0x046/2])

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

	#define STV_VDP2_MPABN2 (m_vdp2_regs[0x048/2])

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

	#define STV_VDP2_MPCDN2 (m_vdp2_regs[0x04a/2])

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

	#define STV_VDP2_MPABN3 (m_vdp2_regs[0x04c/2])

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

	#define STV_VDP2_MPCDN3 (m_vdp2_regs[0x04e/2])

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

	#define STV_VDP2_MPABRA (m_vdp2_regs[0x050/2])

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
	#define STV_VDP2_MPCDRA (m_vdp2_regs[0x052/2])

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
	#define STV_VDP2_MPEFRA (m_vdp2_regs[0x054/2])

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
	#define STV_VDP2_MPGHRA (m_vdp2_regs[0x056/2])

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
	#define STV_VDP2_MPIJRA (m_vdp2_regs[0x058/2])

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
	#define STV_VDP2_MPKLRA (m_vdp2_regs[0x05a/2])

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
	#define STV_VDP2_MPMNRA (m_vdp2_regs[0x05c/2])

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
	#define STV_VDP2_MPOPRA (m_vdp2_regs[0x05e/2])

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

	#define STV_VDP2_MPABRB (m_vdp2_regs[0x060/2])

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

	#define STV_VDP2_MPCDRB (m_vdp2_regs[0x062/2])

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

	#define STV_VDP2_MPEFRB (m_vdp2_regs[0x064/2])

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

	#define STV_VDP2_MPGHRB (m_vdp2_regs[0x066/2])

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

	#define STV_VDP2_MPIJRB (m_vdp2_regs[0x068/2])

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

	#define STV_VDP2_MPKLRB (m_vdp2_regs[0x06a/2])

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

	#define STV_VDP2_MPMNRB (m_vdp2_regs[0x06c/2])

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

	#define STV_VDP2_MPOPRB (m_vdp2_regs[0x06e/2])

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

	#define STV_VDP2_SCXIN0 (m_vdp2_regs[0x070/2])


/* 180072 - Screen Scroll (NBG0, Horizontal Fractional Part)
 bit-> /----15----|----14----|----13----|----12----|----11----|----10----|----09----|----08----\
       |    --    |    --    |    --    |    --    |    --    |    --    |    --    |    --    |
       |----07----|----06----|----05----|----04----|----03----|----02----|----01----|----00----|
       |    --    |    --    |    --    |    --    |    --    |    --    |    --    |    --    |
       \----------|----------|----------|----------|----------|----------|----------|---------*/

	#define STV_VDP2_SCXDN0 (m_vdp2_regs[0x072/2])

/* 180074 - SCYIN0 - Screen Scroll (NBG0, Vertical Integer Part)
 bit-> /----15----|----14----|----13----|----12----|----11----|----10----|----09----|----08----\
       |    --    |    --    |    --    |    --    |    --    |    --    |    --    |    --    |
       |----07----|----06----|----05----|----04----|----03----|----02----|----01----|----00----|
       |    --    |    --    |    --    |    --    |    --    |    --    |    --    |    --    |
       \----------|----------|----------|----------|----------|----------|----------|---------*/
	#define STV_VDP2_SCYIN0 (m_vdp2_regs[0x074/2])


/* 180076 - Screen Scroll (NBG0, Vertical Fractional Part)
 bit-> /----15----|----14----|----13----|----12----|----11----|----10----|----09----|----08----\
       |    --    |    --    |    --    |    --    |    --    |    --    |    --    |    --    |
       |----07----|----06----|----05----|----04----|----03----|----02----|----01----|----00----|
       |    --    |    --    |    --    |    --    |    --    |    --    |    --    |    --    |
       \----------|----------|----------|----------|----------|----------|----------|---------*/

	#define STV_VDP2_SCYDN0 (m_vdp2_regs[0x076/2])

/* 180078 - Coordinate Inc (NBG0, Horizontal Integer Part)
 bit-> /----15----|----14----|----13----|----12----|----11----|----10----|----09----|----08----\
       |    --    |    --    |    --    |    --    |    --    |    --    |    --    |    --    |
       |----07----|----06----|----05----|----04----|----03----|----02----|----01----|----00----|
       |    --    |    --    |    --    |    --    |    --    |    --    |    --    |    --    |
       \----------|----------|----------|----------|----------|----------|----------|---------*/

	#define STV_VDP2_ZMXIN0 (m_vdp2_regs[0x078/2])

	#define STV_VDP2_N0ZMXI ((STV_VDP2_ZMXIN0 & 0x0007) >> 0)

/* 18007a - Coordinate Inc (NBG0, Horizontal Fractional Part)
 bit-> /----15----|----14----|----13----|----12----|----11----|----10----|----09----|----08----\
       |    --    |    --    |    --    |    --    |    --    |    --    |    --    |    --    |
       |----07----|----06----|----05----|----04----|----03----|----02----|----01----|----00----|
       |    --    |    --    |    --    |    --    |    --    |    --    |    --    |    --    |
       \----------|----------|----------|----------|----------|----------|----------|---------*/

	#define STV_VDP2_ZMXDN0 (m_vdp2_regs[0x07a/2])

	#define STV_VDP2_N0ZMXD ((STV_VDP2_ZMXDN0 >> 8)& 0xff)
	#define STV_VDP2_ZMXN0  (((STV_VDP2_N0ZMXI<<16) | (STV_VDP2_N0ZMXD<<8))  & 0x0007ff00)


/* 18007c - Coordinate Inc (NBG0, Vertical Integer Part)
 bit-> /----15----|----14----|----13----|----12----|----11----|----10----|----09----|----08----\
       |    --    |    --    |    --    |    --    |    --    |    --    |    --    |    --    |
       |----07----|----06----|----05----|----04----|----03----|----02----|----01----|----00----|
       |    --    |    --    |    --    |    --    |    --    |    --    |    --    |    --    |
       \----------|----------|----------|----------|----------|----------|----------|---------*/

	#define STV_VDP2_ZMYIN0 (m_vdp2_regs[0x07c/2])

	#define STV_VDP2_N0ZMYI ((STV_VDP2_ZMYIN0 & 0x0007) >> 0)

/* 18007e - Coordinate Inc (NBG0, Vertical Fractional Part)
 bit-> /----15----|----14----|----13----|----12----|----11----|----10----|----09----|----08----\
       |    --    |    --    |    --    |    --    |    --    |    --    |    --    |    --    |
       |----07----|----06----|----05----|----04----|----03----|----02----|----01----|----00----|
       |    --    |    --    |    --    |    --    |    --    |    --    |    --    |    --    |
       \----------|----------|----------|----------|----------|----------|----------|---------*/

	#define STV_VDP2_ZMYDN0 (m_vdp2_regs[0x07e/2])

	#define STV_VDP2_N0ZMYD ((STV_VDP2_ZMYDN0 >> 8)& 0xff)
	#define STV_VDP2_ZMYN0  (((STV_VDP2_N0ZMYI<<16) | (STV_VDP2_N0ZMYD<<8))  & 0x0007ff00)

/* 180080 - SCXIN1 - Screen Scroll (NBG1, Horizontal Integer Part)
 bit-> /----15----|----14----|----13----|----12----|----11----|----10----|----09----|----08----\
       |    --    |    --    |    --    |    --    |    --    |    --    |    --    |    --    |
       |----07----|----06----|----05----|----04----|----03----|----02----|----01----|----00----|
       |    --    |    --    |    --    |    --    |    --    |    --    |    --    |    --    |
       \----------|----------|----------|----------|----------|----------|----------|---------*/

	#define STV_VDP2_SCXIN1 (m_vdp2_regs[0x080/2])

/* 180082 - Screen Scroll (NBG1, Horizontal Fractional Part)
 bit-> /----15----|----14----|----13----|----12----|----11----|----10----|----09----|----08----\
       |    --    |    --    |    --    |    --    |    --    |    --    |    --    |    --    |
       |----07----|----06----|----05----|----04----|----03----|----02----|----01----|----00----|
       |    --    |    --    |    --    |    --    |    --    |    --    |    --    |    --    |
       \----------|----------|----------|----------|----------|----------|----------|---------*/

	#define STV_VDP2_SCXDN1 (m_vdp2_regs[0x082/2])

/* 180084 - SCYIN1 - Screen Scroll (NBG1, Vertical Integer Part)
 bit-> /----15----|----14----|----13----|----12----|----11----|----10----|----09----|----08----\
       |    --    |    --    |    --    |    --    |    --    |    --    |    --    |    --    |
       |----07----|----06----|----05----|----04----|----03----|----02----|----01----|----00----|
       |    --    |    --    |    --    |    --    |    --    |    --    |    --    |    --    |
       \----------|----------|----------|----------|----------|----------|----------|---------*/

	#define STV_VDP2_SCYIN1 (m_vdp2_regs[0x084/2])

/* 180086 - Screen Scroll (NBG1, Vertical Fractional Part)
 bit-> /----15----|----14----|----13----|----12----|----11----|----10----|----09----|----08----\
       |    --    |    --    |    --    |    --    |    --    |    --    |    --    |    --    |
       |----07----|----06----|----05----|----04----|----03----|----02----|----01----|----00----|
       |    --    |    --    |    --    |    --    |    --    |    --    |    --    |    --    |
       \----------|----------|----------|----------|----------|----------|----------|---------*/

	#define STV_VDP2_SCYDN1 (m_vdp2_regs[0x086/2])

/* 180088 - Coordinate Inc (NBG1, Horizontal Integer Part)
 bit-> /----15----|----14----|----13----|----12----|----11----|----10----|----09----|----08----\
       |    --    |    --    |    --    |    --    |    --    |    --    |    --    |    --    |
       |----07----|----06----|----05----|----04----|----03----|----02----|----01----|----00----|
       |    --    |    --    |    --    |    --    |    --    |    --    |    --    |    --    |
       \----------|----------|----------|----------|----------|----------|----------|---------*/

	#define STV_VDP2_ZMXIN1 (m_vdp2_regs[0x088/2])

	#define STV_VDP2_N1ZMXI ((STV_VDP2_ZMXIN1 & 0x0007) >> 0)

/* 18008a - Coordinate Inc (NBG1, Horizontal Fractional Part)
 bit-> /----15----|----14----|----13----|----12----|----11----|----10----|----09----|----08----\
       |    --    |    --    |    --    |    --    |    --    |    --    |    --    |    --    |
       |----07----|----06----|----05----|----04----|----03----|----02----|----01----|----00----|
       |    --    |    --    |    --    |    --    |    --    |    --    |    --    |    --    |
       \----------|----------|----------|----------|----------|----------|----------|---------*/

	#define STV_VDP2_ZMXDN1 (m_vdp2_regs[0x08a/2])

	#define STV_VDP2_N1ZMXD ((STV_VDP2_ZMXDN1 >> 8)& 0xff)
	#define STV_VDP2_ZMXN1  (((STV_VDP2_N1ZMXI<<16) | (STV_VDP2_N1ZMXD<<8)) & 0x0007ff00)

/* 18008c - Coordinate Inc (NBG1, Vertical Integer Part)
 bit-> /----15----|----14----|----13----|----12----|----11----|----10----|----09----|----08----\
       |    --    |    --    |    --    |    --    |    --    |    --    |    --    |    --    |
       |----07----|----06----|----05----|----04----|----03----|----02----|----01----|----00----|
       |    --    |    --    |    --    |    --    |    --    |    --    |    --    |    --    |
       \----------|----------|----------|----------|----------|----------|----------|---------*/

	#define STV_VDP2_ZMYIN1 (m_vdp2_regs[0x08c/2])

	#define STV_VDP2_N1ZMYI ((STV_VDP2_ZMYIN1 & 0x0007) >> 0)

/* 18008e - Coordinate Inc (NBG1, Vertical Fractional Part)
 bit-> /----15----|----14----|----13----|----12----|----11----|----10----|----09----|----08----\
       |    --    |    --    |    --    |    --    |    --    |    --    |    --    |    --    |
       |----07----|----06----|----05----|----04----|----03----|----02----|----01----|----00----|
       |    --    |    --    |    --    |    --    |    --    |    --    |    --    |    --    |
       \----------|----------|----------|----------|----------|----------|----------|---------*/

	#define STV_VDP2_ZMYDN1 (m_vdp2_regs[0x08e/2])

	#define STV_VDP2_N1ZMYD ((STV_VDP2_ZMYDN1 >> 8)& 0xff)
	#define STV_VDP2_ZMYN1  (((STV_VDP2_N1ZMYI<<16) | (STV_VDP2_N1ZMYD<<8)) & 0x007ff00)

/* 180090 - SCXN2 - Screen Scroll (NBG2, Horizontal)
 bit-> /----15----|----14----|----13----|----12----|----11----|----10----|----09----|----08----\
       |    --    |    --    |    --    |    --    |    --    |    --    |    --    |    --    |
       |----07----|----06----|----05----|----04----|----03----|----02----|----01----|----00----|
       |    --    |    --    |    --    |    --    |    --    |    --    |    --    |    --    |
       \----------|----------|----------|----------|----------|----------|----------|---------*/

	#define STV_VDP2_SCXN2 (m_vdp2_regs[0x090/2])

/* 180092 - SCYN2 - Screen Scroll (NBG2, Vertical)
 bit-> /----15----|----14----|----13----|----12----|----11----|----10----|----09----|----08----\
       |    --    |    --    |    --    |    --    |    --    |    --    |    --    |    --    |
       |----07----|----06----|----05----|----04----|----03----|----02----|----01----|----00----|
       |    --    |    --    |    --    |    --    |    --    |    --    |    --    |    --    |
       \----------|----------|----------|----------|----------|----------|----------|---------*/

	#define STV_VDP2_SCYN2 (m_vdp2_regs[0x092/2])

/* 180094 - SCXN3 - Screen Scroll (NBG3, Horizontal)
 bit-> /----15----|----14----|----13----|----12----|----11----|----10----|----09----|----08----\
       |    --    |    --    |    --    |    --    |    --    |    --    |    --    |    --    |
       |----07----|----06----|----05----|----04----|----03----|----02----|----01----|----00----|
       |    --    |    --    |    --    |    --    |    --    |    --    |    --    |    --    |
       \----------|----------|----------|----------|----------|----------|----------|---------*/

	#define STV_VDP2_SCXN3 (m_vdp2_regs[0x094/2])

/* 180096 - SCYN3 - Screen Scroll (NBG3, Vertical)
 bit-> /----15----|----14----|----13----|----12----|----11----|----10----|----09----|----08----\
       |    --    |    --    |    --    |    --    |    --    |    --    |    --    |    --    |
       |----07----|----06----|----05----|----04----|----03----|----02----|----01----|----00----|
       |    --    |    --    |    --    |    --    |    --    |    --    |    --    |    --    |
       \----------|----------|----------|----------|----------|----------|----------|---------*/

	#define STV_VDP2_SCYN3 (m_vdp2_regs[0x096/2])

/* 180098 - Reduction Enable
 bit-> /----15----|----14----|----13----|----12----|----11----|----10----|----09----|----08----\
       |    --    |    --    |    --    |    --    |    --    |    --    | N1ZMQT   | N1ZMHF   |
       |----07----|----06----|----05----|----04----|----03----|----02----|----01----|----00----|
       |    --    |    --    |    --    |    --    |    --    |    --    | N0ZMQT   | N0ZMHF   |
       \----------|----------|----------|----------|----------|----------|----------|---------*/

	#define STV_VDP2_ZMCTL (m_vdp2_regs[0x098/2])

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

	#define STV_VDP2_SCRCTL (m_vdp2_regs[0x09a/2])

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

	#define STV_VDP2_VCSTAU (m_vdp2_regs[0x09c/2] & 7)


/* 18009e - Vertical Cell Table Address (NBG0, NBG1)
 bit-> /----15----|----14----|----13----|----12----|----11----|----10----|----09----|----08----\
       |    --    |    --    |    --    |    --    |    --    |    --    |    --    |    --    |
       |----07----|----06----|----05----|----04----|----03----|----02----|----01----|----00----|
       |    --    |    --    |    --    |    --    |    --    |    --    |    --    |    --    |
       \----------|----------|----------|----------|----------|----------|----------|---------*/

	#define STV_VDP2_VCSTAL (m_vdp2_regs[0x09e/2])


/* 1800a0 - LSTA0U - Line Scroll Table Address (NBG0)
 bit-> /----15----|----14----|----13----|----12----|----11----|----10----|----09----|----08----\
       |    --    |    --    |    --    |    --    |    --    |    --    |    --    |    --    |
       |----07----|----06----|----05----|----04----|----03----|----02----|----01----|----00----|
       |    --    |    --    |    --    |    --    |    --    |    --    |    --    |    --    |
       \----------|----------|----------|----------|----------|----------|----------|---------*/

	/*bit 2 unused when VRAM = 4 Mbits*/
	#define STV_VDP2_LSTA0U (m_vdp2_regs[0x0a0/2] & 7)

/* 1800a2 - LSTA0L - Line Scroll Table Address (NBG0)
 bit-> /----15----|----14----|----13----|----12----|----11----|----10----|----09----|----08----\
       |    --    |    --    |    --    |    --    |    --    |    --    |    --    |    --    |
       |----07----|----06----|----05----|----04----|----03----|----02----|----01----|----00----|
       |    --    |    --    |    --    |    --    |    --    |    --    |    --    |    --    |
       \----------|----------|----------|----------|----------|----------|----------|---------*/

	#define STV_VDP2_LSTA0L (m_vdp2_regs[0x0a2/2])

/* 1800a4 - LSTA1U - Line Scroll Table Address (NBG1)
 bit-> /----15----|----14----|----13----|----12----|----11----|----10----|----09----|----08----\
       |    --    |    --    |    --    |    --    |    --    |    --    |    --    |    --    |
       |----07----|----06----|----05----|----04----|----03----|----02----|----01----|----00----|
       |    --    |    --    |    --    |    --    |    --    |    --    |    --    |    --    |
       \----------|----------|----------|----------|----------|----------|----------|---------*/

	/*bit 2 unused when VRAM = 4 Mbits*/
	#define STV_VDP2_LSTA1U (m_vdp2_regs[0x0a4/2] & 7)

/* 1800a6 - LSTA1L - Line Scroll Table Address (NBG1)
 bit-> /----15----|----14----|----13----|----12----|----11----|----10----|----09----|----08----\
       |    --    |    --    |    --    |    --    |    --    |    --    |    --    |    --    |
       |----07----|----06----|----05----|----04----|----03----|----02----|----01----|----00----|
       |    --    |    --    |    --    |    --    |    --    |    --    |    --    |    --    |
       \----------|----------|----------|----------|----------|----------|----------|---------*/

	#define STV_VDP2_LSTA1L (m_vdp2_regs[0x0a6/2])

/* 1800a8 - LCTAU - Line Colour Screen Table Address
 bit-> /----15----|----14----|----13----|----12----|----11----|----10----|----09----|----08----\
       |    --    |    --    |    --    |    --    |    --    |    --    |    --    |    --    |
       |----07----|----06----|----05----|----04----|----03----|----02----|----01----|----00----|
       |    --    |    --    |    --    |    --    |    --    |    --    |    --    |    --    |
       \----------|----------|----------|----------|----------|----------|----------|---------*/

	#define STV_VDP2_LCTAU  (m_vdp2_regs[0x0a8/2])
	#define STV_VDP2_LCCLMD ((STV_VDP2_LCTAU & 0x8000) >> 15)

/* 1800aa - LCTAL - Line Colour Screen Table Address
 bit-> /----15----|----14----|----13----|----12----|----11----|----10----|----09----|----08----\
       |    --    |    --    |    --    |    --    |    --    |    --    |    --    |    --    |
       |----07----|----06----|----05----|----04----|----03----|----02----|----01----|----00----|
       |    --    |    --    |    --    |    --    |    --    |    --    |    --    |    --    |
       \----------|----------|----------|----------|----------|----------|----------|---------*/
	#define STV_VDP2_LCTAL  (m_vdp2_regs[0x0aa/2])

	#define STV_VDP2_LCTA   (((STV_VDP2_LCTAU & 0x0007) << 16) | (STV_VDP2_LCTAL & 0xffff))

/* 1800ac - Back Screen Table Address
 bit-> /----15----|----14----|----13----|----12----|----11----|----10----|----09----|----08----\
       |  BKCLMD  |    --    |    --    |    --    |    --    |    --    |    --    |    --    |
       |----07----|----06----|----05----|----04----|----03----|----02----|----01----|----00----|
       |    --    |    --    |    --    |    --    |    --    |  BKTA18  |  BKTA17  |  BKTA16  |
       \----------|----------|----------|----------|----------|----------|----------|---------*/

	#define STV_VDP2_BKTAU  (m_vdp2_regs[0x0ac/2])

	#define STV_VDP2_BKCLMD ((STV_VDP2_BKTAU & 0x8000) >> 15)


/* 1800ae - Back Screen Table Address
 bit-> /----15----|----14----|----13----|----12----|----11----|----10----|----09----|----08----\
       |  BKTA15  |  BKTA14  |  BKTA13  |  BKTA12  |  BKTA11  |  BKTA10  |  BKTA9   |  BKTA8   |
       |----07----|----06----|----05----|----04----|----03----|----02----|----01----|----00----|
       |  BKTA7   |  BKTA7   |  BKTA6   |  BKTA5   |  BKTA4   |  BKTA3   |  BKTA2   |  BKTA0   |
       \----------|----------|----------|----------|----------|----------|----------|---------*/

	#define STV_VDP2_BKTAL  (m_vdp2_regs[0x0ae/2])

	#define STV_VDP2_BKTA   (((STV_VDP2_BKTAU & 0x0007) << 16) | (STV_VDP2_BKTAL & 0xffff))

/* 1800b0 - RPMD - Rotation Parameter Mode
 bit-> /----15----|----14----|----13----|----12----|----11----|----10----|----09----|----08----\
       |    --    |    --    |    --    |    --    |    --    |    --    |    --    |    --    |
       |----07----|----06----|----05----|----04----|----03----|----02----|----01----|----00----|
       |    --    |    --    |    --    |    --    |    --    |    --    |    --    |    --    |
       \----------|----------|----------|----------|----------|----------|----------|---------*/

	#define STV_VDP2_RPMD   ((m_vdp2_regs[0x0b0/2]) & 0x0003)

/* 1800b2 - RPRCTL - Rotation Parameter Read Control
 bit-> /----15----|----14----|----13----|----12----|----11----|----10----|----09----|----08----\
       |    --    |    --    |    --    |    --    |    --    | RBKASTRE | RBYSTRE  | RBXSTRE  |
       |----07----|----06----|----05----|----04----|----03----|----02----|----01----|----00----|
       |    --    |    --    |    --    |    --    |    --    | RAKASTRE | RAYSTRE  | RBXSTRE  |
       \----------|----------|----------|----------|----------|----------|----------|---------*/

	#define STV_VDP2_RPRCTL     (m_vdp2_regs[0x0b2/2])
	#define STV_VDP2_RBKASTRE   ((STV_VDP2_RPRCTL & 0x0400) >> 10)
	#define STV_VDP2_RBYSTRE    ((STV_VDP2_RPRCTL & 0x0200) >> 9)
	#define STV_VDP2_RBXSTRE    ((STV_VDP2_RPRCTL & 0x0100) >> 8)
	#define STV_VDP2_RAKASTRE   ((STV_VDP2_RPRCTL & 0x0004) >> 2)
	#define STV_VDP2_RAYSTRE    ((STV_VDP2_RPRCTL & 0x0002) >> 1)
	#define STV_VDP2_RAXSTRE    ((STV_VDP2_RPRCTL & 0x0001) >> 0)

/* 1800b4 - KTCTL - Coefficient Table Control
 bit-> /----15----|----14----|----13----|----12----|----11----|----10----|----09----|----08----\
       |    --    |    --    |    --    |  RBKLCE  |  RBKMD1  |  RBKMD0  |  RBKDBS  |   RBKTE  |
       |----07----|----06----|----05----|----04----|----03----|----02----|----01----|----00----|
       |    --    |    --    |    --    |  RAKLCE  |  RAKMD1  |  RAKMD0  |  RAKDBS  |   RAKTE  |
       \----------|----------|----------|----------|----------|----------|----------|---------*/

	#define STV_VDP2_KTCTL  (m_vdp2_regs[0x0b4/2])
	#define STV_VDP2_RBKLCE ((STV_VDP2_KTCTL & 0x1000) >> 12)
	#define STV_VDP2_RBKMD  ((STV_VDP2_KTCTL & 0x0c00) >> 10)
	#define STV_VDP2_RBKDBS ((STV_VDP2_KTCTL & 0x0200) >> 9)
	#define STV_VDP2_RBKTE  ((STV_VDP2_KTCTL & 0x0100) >> 8)
	#define STV_VDP2_RAKLCE ((STV_VDP2_KTCTL & 0x0010) >> 4)
	#define STV_VDP2_RAKMD  ((STV_VDP2_KTCTL & 0x000c) >> 2)
	#define STV_VDP2_RAKDBS ((STV_VDP2_KTCTL & 0x0002) >> 1)
	#define STV_VDP2_RAKTE  ((STV_VDP2_KTCTL & 0x0001) >> 0)

/* 1800b6 - KTAOF - Coefficient Table Address Offset (Rotation Parameter A,B)
 bit-> /----15----|----14----|----13----|----12----|----11----|----10----|----09----|----08----\
       |    --    |    --    |    --    |    --    |    --    | RBKTAOS2 | RBKTAOS1 | RBKTAOS0 |
       |----07----|----06----|----05----|----04----|----03----|----02----|----01----|----00----|
       |    --    |    --    |    --    |    --    |    --    | RAKTAOS2 | RAKTAOS1 | RAKTAOS0 |
       \----------|----------|----------|----------|----------|----------|----------|---------*/

	#define STV_VDP2_KTAOF  (m_vdp2_regs[0x0b6/2])
	#define STV_VDP2_RBKTAOS ((STV_VDP2_KTAOF & 0x0700) >> 8)
	#define STV_VDP2_RAKTAOS ((STV_VDP2_KTAOF & 0x0007) >> 0)

/* 1800b8 - OVPNRA - Screen Over Pattern Name (Rotation Parameter A)
 bit-> /----15----|----14----|----13----|----12----|----11----|----10----|----09----|----08----\
       |    --    |    --    |    --    |    --    |    --    |    --    |    --    |    --    |
       |----07----|----06----|----05----|----04----|----03----|----02----|----01----|----00----|
       |    --    |    --    |    --    |    --    |    --    |    --    |    --    |    --    |
       \----------|----------|----------|----------|----------|----------|----------|---------*/

	#define STV_VDP2_OVPNRA (m_vdp2_regs[0x0b8/2])

/* 1800ba - Screen Over Pattern Name (Rotation Parameter B)
 bit-> /----15----|----14----|----13----|----12----|----11----|----10----|----09----|----08----\
       |    --    |    --    |    --    |    --    |    --    |    --    |    --    |    --    |
       |----07----|----06----|----05----|----04----|----03----|----02----|----01----|----00----|
       |    --    |    --    |    --    |    --    |    --    |    --    |    --    |    --    |
       \----------|----------|----------|----------|----------|----------|----------|---------*/

	#define STV_VDP2_OVPNRB (m_vdp2_regs[0x0ba/2])

/* 1800bc - RPTAU - Rotation Parameter Table Address (Rotation Parameter A,B)
 bit-> /----15----|----14----|----13----|----12----|----11----|----10----|----09----|----08----\
       |    --    |    --    |    --    |    --    |    --    |    --    |    --    |    --    |
       |----07----|----06----|----05----|----04----|----03----|----02----|----01----|----00----|
       |    --    |    --    |    --    |    --    |    --    |  RPTA18  |  RPTA17  |  RPTA16  |
       \----------|----------|----------|----------|----------|----------|----------|---------*/
	#define STV_VDP2_RPTAU  (m_vdp2_regs[0x0bc/2] & 7)

/* 1800be - RPTAL - Rotation Parameter Table Address (Rotation Parameter A,B)
 bit-> /----15----|----14----|----13----|----12----|----11----|----10----|----09----|----08----\
       |  RPTA15  |  RPTA14  |  RPTA13  |  RPTA12  |  RPTA11  |  RPTA10  |   RPTA9  |   RPTA8  |
       |----07----|----06----|----05----|----04----|----03----|----02----|----01----|----00----|
       |   RPTA7  |   RPTA6  |   RPTA5  |   RPTA4  |   RPTA3  |   RPTA2  |   RPTA1  |    --    |
       \----------|----------|----------|----------|----------|----------|----------|---------*/

	#define STV_VDP2_RPTAL  (m_vdp2_regs[0x0be/2] & 0x0000ffff)

/* 1800c0 - Window Position (W0, Horizontal Start Point)
 bit-> /----15----|----14----|----13----|----12----|----11----|----10----|----09----|----08----\
       |    --    |    --    |    --    |    --    |    --    |    --    |    --    |    --    |
       |----07----|----06----|----05----|----04----|----03----|----02----|----01----|----00----|
       |    --    |    --    |    --    |    --    |    --    |    --    |    --    |    --    |
       \----------|----------|----------|----------|----------|----------|----------|---------*/

	#define STV_VDP2_WPSX0 (m_vdp2_regs[0x0c0/2])

	#define STV_VDP2_W0SX ((STV_VDP2_WPSX0 & 0x03ff) >> 0)

/* 1800c2 - Window Position (W0, Vertical Start Point)
 bit-> /----15----|----14----|----13----|----12----|----11----|----10----|----09----|----08----\
       |    --    |    --    |    --    |    --    |    --    |    --    |    --    |    --    |
       |----07----|----06----|----05----|----04----|----03----|----02----|----01----|----00----|
       |    --    |    --    |    --    |    --    |    --    |    --    |    --    |    --    |
       \----------|----------|----------|----------|----------|----------|----------|---------*/

	#define STV_VDP2_WPSY0 (m_vdp2_regs[0x0c2/2])

	#define STV_VDP2_W0SY ((STV_VDP2_WPSY0 & 0x03ff) >> 0)

/* 1800c4 - Window Position (W0, Horizontal End Point)
 bit-> /----15----|----14----|----13----|----12----|----11----|----10----|----09----|----08----\
       |    --    |    --    |    --    |    --    |    --    |    --    |    --    |    --    |
       |----07----|----06----|----05----|----04----|----03----|----02----|----01----|----00----|
       |    --    |    --    |    --    |    --    |    --    |    --    |    --    |    --    |
       \----------|----------|----------|----------|----------|----------|----------|---------*/

	#define STV_VDP2_WPEX0 (m_vdp2_regs[0x0c4/2])

	#define STV_VDP2_W0EX ((STV_VDP2_WPEX0 & 0x03ff) >> 0)

/* 1800c6 - Window Position (W0, Vertical End Point)
 bit-> /----15----|----14----|----13----|----12----|----11----|----10----|----09----|----08----\
       |    --    |    --    |    --    |    --    |    --    |    --    |    --    |    --    |
       |----07----|----06----|----05----|----04----|----03----|----02----|----01----|----00----|
       |    --    |    --    |    --    |    --    |    --    |    --    |    --    |    --    |
       \----------|----------|----------|----------|----------|----------|----------|---------*/

	#define STV_VDP2_WPEY0 (m_vdp2_regs[0x0c6/2])

	#define STV_VDP2_W0EY ((STV_VDP2_WPEY0 & 0x03ff) >> 0)

/* 1800c8 - Window Position (W1, Horizontal Start Point)
 bit-> /----15----|----14----|----13----|----12----|----11----|----10----|----09----|----08----\
       |    --    |    --    |    --    |    --    |    --    |    --    |    --    |    --    |
       |----07----|----06----|----05----|----04----|----03----|----02----|----01----|----00----|
       |    --    |    --    |    --    |    --    |    --    |    --    |    --    |    --    |
       \----------|----------|----------|----------|----------|----------|----------|---------*/

	#define STV_VDP2_WPSX1 (m_vdp2_regs[0x0c8/2])

	#define STV_VDP2_W1SX ((STV_VDP2_WPSX1 & 0x03ff) >> 0)

/* 1800ca - Window Position (W1, Vertical Start Point)
 bit-> /----15----|----14----|----13----|----12----|----11----|----10----|----09----|----08----\
       |    --    |    --    |    --    |    --    |    --    |    --    |    --    |    --    |
       |----07----|----06----|----05----|----04----|----03----|----02----|----01----|----00----|
       |    --    |    --    |    --    |    --    |    --    |    --    |    --    |    --    |
       \----------|----------|----------|----------|----------|----------|----------|---------*/

	#define STV_VDP2_WPSY1 (m_vdp2_regs[0x0ca/2])

	#define STV_VDP2_W1SY ((STV_VDP2_WPSY1 & 0x03ff) >> 0)

/* 1800cc - Window Position (W1, Horizontal End Point)
 bit-> /----15----|----14----|----13----|----12----|----11----|----10----|----09----|----08----\
       |    --    |    --    |    --    |    --    |    --    |    --    |    --    |    --    |
       |----07----|----06----|----05----|----04----|----03----|----02----|----01----|----00----|
       |    --    |    --    |    --    |    --    |    --    |    --    |    --    |    --    |
       \----------|----------|----------|----------|----------|----------|----------|---------*/

	#define STV_VDP2_WPEX1 (m_vdp2_regs[0x0cc/2])

	#define STV_VDP2_W1EX ((STV_VDP2_WPEX1 & 0x03ff) >> 0)

/* 1800ce - Window Position (W1, Vertical End Point)
 bit-> /----15----|----14----|----13----|----12----|----11----|----10----|----09----|----08----\
       |    --    |    --    |    --    |    --    |    --    |    --    |    --    |    --    |
       |----07----|----06----|----05----|----04----|----03----|----02----|----01----|----00----|
       |    --    |    --    |    --    |    --    |    --    |    --    |    --    |    --    |
       \----------|----------|----------|----------|----------|----------|----------|---------*/

	#define STV_VDP2_WPEY1 (m_vdp2_regs[0x0ce/2])

	#define STV_VDP2_W1EY ((STV_VDP2_WPEY1 & 0x03ff) >> 0)

/* 1800d0 - Window Control (NBG0, NBG1)
 bit-> /----15----|----14----|----13----|----12----|----11----|----10----|----09----|----08----\
       |    --    |    --    |    --    |    --    |    --    |    --    |    --    |    --    |
       |----07----|----06----|----05----|----04----|----03----|----02----|----01----|----00----|
       |    --    |    --    |    --    |    --    |    --    |    --    |    --    |    --    |
       \----------|----------|----------|----------|----------|----------|----------|---------*/

	#define STV_VDP2_WCTLA (m_vdp2_regs[0x0d0/2])
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

	#define STV_VDP2_WCTLB (m_vdp2_regs[0x0d2/2])
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

	#define STV_VDP2_WCTLC (m_vdp2_regs[0x0d4/2])
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

	#define STV_VDP2_WCTLD (m_vdp2_regs[0x0d6/2])
	#define STV_VDP2_CCLOG ((STV_VDP2_WCTLD & 0x8000) >> 15)
	#define STV_VDP2_CCSWE ((STV_VDP2_WCTLD & 0x2000) >> 13)
	#define STV_VDP2_CCSWA ((STV_VDP2_WCTLD & 0x1000) >> 12)
	#define STV_VDP2_CCW1E ((STV_VDP2_WCTLD & 0x0800) >> 11)
	#define STV_VDP2_CCW1A ((STV_VDP2_WCTLD & 0x0400) >> 10)
	#define STV_VDP2_CCW0E ((STV_VDP2_WCTLD & 0x0200) >> 9)
	#define STV_VDP2_CCW0A ((STV_VDP2_WCTLD & 0x0100) >> 8)
	#define STV_VDP2_RPLOG ((STV_VDP2_WCTLD & 0x0080) >> 7)
	#define STV_VDP2_RPW1E ((STV_VDP2_WCTLD & 0x0008) >> 3)
	#define STV_VDP2_RPW1A ((STV_VDP2_WCTLD & 0x0004) >> 2)
	#define STV_VDP2_RPW0E ((STV_VDP2_WCTLD & 0x0002) >> 1)
	#define STV_VDP2_RPW0A ((STV_VDP2_WCTLD & 0x0001) >> 0)

/* 1800d8 - Line Window Table Address (W0)
 bit-> /----15----|----14----|----13----|----12----|----11----|----10----|----09----|----08----\
       |    --    |    --    |    --    |    --    |    --    |    --    |    --    |    --    |
       |----07----|----06----|----05----|----04----|----03----|----02----|----01----|----00----|
       |    --    |    --    |    --    |    --    |    --    |    --    |    --    |    --    |
       \----------|----------|----------|----------|----------|----------|----------|---------*/

	#define STV_VDP2_LWTA0U (m_vdp2_regs[0x0d8/2])

	#define STV_VDP2_W0LWE  ((STV_VDP2_LWTA0U & 0x8000) >> 15)

/* 1800da - Line Window Table Address (W0)
 bit-> /----15----|----14----|----13----|----12----|----11----|----10----|----09----|----08----\
       |    --    |    --    |    --    |    --    |    --    |    --    |    --    |    --    |
       |----07----|----06----|----05----|----04----|----03----|----02----|----01----|----00----|
       |    --    |    --    |    --    |    --    |    --    |    --    |    --    |    --    |
       \----------|----------|----------|----------|----------|----------|----------|---------*/

	#define STV_VDP2_LWTA0L (m_vdp2_regs[0x0da/2])

	/* bit 19 isn't used when VRAM = 4 Mbit */
	#define STV_VDP2_W0LWTA (((STV_VDP2_LWTA0U & 0x0007) << 16) | (STV_VDP2_LWTA0L & 0xfffe))


/* 1800dc - Line Window Table Address (W1)
 bit-> /----15----|----14----|----13----|----12----|----11----|----10----|----09----|----08----\
       |    --    |    --    |    --    |    --    |    --    |    --    |    --    |    --    |
       |----07----|----06----|----05----|----04----|----03----|----02----|----01----|----00----|
       |    --    |    --    |    --    |    --    |    --    |    --    |    --    |    --    |
       \----------|----------|----------|----------|----------|----------|----------|---------*/

	#define STV_VDP2_LWTA1U (m_vdp2_regs[0x0dc/2])

	#define STV_VDP2_W1LWE  ((STV_VDP2_LWTA1U & 0x8000) >> 15)


/* 1800de - Line Window Table Address (W1)
 bit-> /----15----|----14----|----13----|----12----|----11----|----10----|----09----|----08----\
       |    --    |    --    |    --    |    --    |    --    |    --    |    --    |    --    |
       |----07----|----06----|----05----|----04----|----03----|----02----|----01----|----00----|
       |    --    |    --    |    --    |    --    |    --    |    --    |    --    |    --    |
       \----------|----------|----------|----------|----------|----------|----------|---------*/

	#define STV_VDP2_LWTA1L (m_vdp2_regs[0x0de/2])

	/* bit 19 isn't used when VRAM = 4 Mbit */
	#define STV_VDP2_W1LWTA (((STV_VDP2_LWTA1U & 0x0007) << 16) | (STV_VDP2_LWTA1L & 0xfffe))


/* 1800e0 - Sprite Control
 bit-> /----15----|----14----|----13----|----12----|----11----|----10----|----09----|----08----\
       |    --    |    --    | SPCCCS1  | SPCCCS0  |    --    |  SPCCN2  |  SPCCN1  |  SPCCN0  |
       |----07----|----06----|----05----|----04----|----03----|----02----|----01----|----00----|
       |    --    |    --    |  SPCLMD  | SPWINEN  |  SPTYPE3 |  SPTYPE2 |  SPTYPE1 |  SPTYPE0 |
       \----------|----------|----------|----------|----------|----------|----------|---------*/

	#define STV_VDP2_SPCTL  (m_vdp2_regs[0xe0/2])
	#define STV_VDP2_SPCCCS     ((STV_VDP2_SPCTL & 0x3000) >> 12)
	#define STV_VDP2_SPCCN      ((STV_VDP2_SPCTL & 0x700) >> 8)
	#define STV_VDP2_SPCLMD     ((STV_VDP2_SPCTL & 0x20) >> 5)
	#define STV_VDP2_SPWINEN    ((STV_VDP2_SPCTL & 0x10) >> 4)
	#define STV_VDP2_SPTYPE     (STV_VDP2_SPCTL & 0xf)

/* 1800e2 - Shadow Control
 bit-> /----15----|----14----|----13----|----12----|----11----|----10----|----09----|----08----\
       |    --    |    --    |    --    |    --    |    --    |    --    |    --    |    --    |
       |----07----|----06----|----05----|----04----|----03----|----02----|----01----|----00----|
       |    --    |    --    |    --    |    --    |    --    |    --    |    --    |    --    |
       \----------|----------|----------|----------|----------|----------|----------|---------*/

	#define STV_VDP2_SDCTL  (m_vdp2_regs[0x0e2/2])

/* 1800e4 - CRAOFA - Colour Ram Address Offset (NBG0 - NBG3)
 bit-> /----15----|----14----|----13----|----12----|----11----|----10----|----09----|----08----\
       |    --    | N0CAOS2  | N3CAOS1  | N3CAOS0  |    --    | N2CAOS2  | N2CAOS1  | N2CAOS0  |
       |----07----|----06----|----05----|----04----|----03----|----02----|----01----|----00----|
       |    --    | N1CAOS2  | N1CAOS1  | N1CAOS0  |    --    | N0CAOS2  | N0CAOS1  | N0CAOS0  |
       \----------|----------|----------|----------|----------|----------|----------|---------*/

	#define STV_VDP2_CRAOFA (m_vdp2_regs[0x0e4/2])

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
	#define STV_VDP2_CRAOFB (m_vdp2_regs[0x0e6/2])
	#define STV_VDP2_R0CAOS ((STV_VDP2_CRAOFB & 0x0007) >> 0)
	#define STV_VDP2_SPCAOS ((STV_VDP2_CRAOFB & 0x0070) >> 4)

/* 1800e8 - LNCLEN - Line Colour Screen Enable
 bit-> /----15----|----14----|----13----|----12----|----11----|----10----|----09----|----08----\
       |    --    |    --    |    --    |    --    |    --    |    --    |    --    |    --    |
       |----07----|----06----|----05----|----04----|----03----|----02----|----01----|----00----|
       |    --    |    --    |  SPLCEN  |  R0LCEN  |  N3LCEN  |  N2LCEN  |  N1LCEN  | N0LCEN   |
       \----------|----------|----------|----------|----------|----------|----------|---------*/

	#define STV_VDP2_LNCLEN (m_vdp2_regs[0x0e8/2])
	#define STV_VDP2_SPLCEN ((STV_VDP2_LNCLEN & 0x0020) >> 5)
	#define STV_VDP2_R0LCEN ((STV_VDP2_LNCLEN & 0x0010) >> 4)
	#define STV_VDP2_N3LCEN ((STV_VDP2_LNCLEN & 0x0008) >> 3)
	#define STV_VDP2_N2LCEN ((STV_VDP2_LNCLEN & 0x0004) >> 2)
	#define STV_VDP2_N1LCEN ((STV_VDP2_LNCLEN & 0x0002) >> 1)
	#define STV_VDP2_N0LCEN ((STV_VDP2_LNCLEN & 0x0001) >> 0)

/* 1800ea - Special Priority Mode
 bit-> /----15----|----14----|----13----|----12----|----11----|----10----|----09----|----08----\
       |    --    |    --    |    --    |    --    |    --    |    --    |    --    |    --    |
       |----07----|----06----|----05----|----04----|----03----|----02----|----01----|----00----|
       |    --    |    --    |    --    |    --    |    --    |    --    |    --    |    --    |
       \----------|----------|----------|----------|----------|----------|----------|---------*/

	#define STV_VDP2_SFPRMD (m_vdp2_regs[0x0ea/2])


/* 1800ec - Colour Calculation Control
 bit-> /----15----|----14----|----13----|----12----|----11----|----10----|----09----|----08----\
       |  BOKEN   |  BOKN2   |  BOKN1   |   BOKN0  |    --    |  EXCCEN  |  CCRTMD  |  CCMD    |
       |----07----|----06----|----05----|----04----|----03----|----02----|----01----|----00----|
       |    --    |  SPCCEN  |  LCCCEN  |  R0CCEN  |  N3CCEN  |  N2CCEN  |  N1CCEN  |  N0CCEN  |
       \----------|----------|----------|----------|----------|----------|----------|---------*/

	#define STV_VDP2_CCCR       (m_vdp2_regs[0x0ec/2])
	#define STV_VDP2_CCMD       ((STV_VDP2_CCCR & 0x100) >> 8)
	#define STV_VDP2_SPCCEN     ((STV_VDP2_CCCR & 0x40) >> 6)
	#define STV_VDP2_LCCCEN     ((STV_VDP2_CCCR & 0x20) >> 5)
	#define STV_VDP2_R0CCEN     ((STV_VDP2_CCCR & 0x10) >> 4)
	#define STV_VDP2_N3CCEN     ((STV_VDP2_CCCR & 0x8) >> 3)
	#define STV_VDP2_N2CCEN     ((STV_VDP2_CCCR & 0x4) >> 2)
	#define STV_VDP2_N1CCEN     ((STV_VDP2_CCCR & 0x2) >> 1)
	#define STV_VDP2_N0CCEN     ((STV_VDP2_CCCR & 0x1) >> 0)


/* 1800ee - Special Colour Calculation Mode
 bit-> /----15----|----14----|----13----|----12----|----11----|----10----|----09----|----08----\
       |    --    |    --    |    --    |    --    |    --    |    --    |    --    |    --    |
       |----07----|----06----|----05----|----04----|----03----|----02----|----01----|----00----|
       |    --    |    --    |    --    |    --    |    --    |    --    |    --    |    --    |
       \----------|----------|----------|----------|----------|----------|----------|---------*/

	#define STV_VDP2_SFCCMD     (m_vdp2_regs[0x0ee/2])

/* 1800f0 - Priority Number (Sprite 0,1)
 bit-> /----15----|----14----|----13----|----12----|----11----|----10----|----09----|----08----\
       |    --    |    --    |    --    |    --    |    --    |  S1PRIN2 |  S1PRIN1 |  S1PRIN0 |
       |----07----|----06----|----05----|----04----|----03----|----02----|----01----|----00----|
       |    --    |    --    |    --    |    --    |    --    |  S0PRIN2 |  S0PRIN1 |  S0PRIN0 |
       \----------|----------|----------|----------|----------|----------|----------|---------*/

	#define STV_VDP2_PRISA      (m_vdp2_regs[0x0f0/2])
	#define STV_VDP2_S1PRIN     ((STV_VDP2_PRISA & 0x0700) >> 8)
	#define STV_VDP2_S0PRIN     ((STV_VDP2_PRISA & 0x0007) >> 0)

/* 1800f2 - Priority Number (Sprite 2,3)
 bit-> /----15----|----14----|----13----|----12----|----11----|----10----|----09----|----08----\
       |    --    |    --    |    --    |    --    |    --    |  S3PRIN2 |  S3PRIN1 |  S3PRIN0 |
       |----07----|----06----|----05----|----04----|----03----|----02----|----01----|----00----|
       |    --    |    --    |    --    |    --    |    --    |  S2PRIN2 |  S2PRIN1 |  S2PRIN0 |
       \----------|----------|----------|----------|----------|----------|----------|---------*/

	#define STV_VDP2_PRISB      (m_vdp2_regs[0x0f2/2])
	#define STV_VDP2_S3PRIN     ((STV_VDP2_PRISB & 0x0700) >> 8)
	#define STV_VDP2_S2PRIN     ((STV_VDP2_PRISB & 0x0007) >> 0)

/* 1800f4 - Priority Number (Sprite 4,5)
 bit-> /----15----|----14----|----13----|----12----|----11----|----10----|----09----|----08----\
       |    --    |    --    |    --    |    --    |    --    |  S5PRIN2 |  S5PRIN1 |  S5PRIN0 |
       |----07----|----06----|----05----|----04----|----03----|----02----|----01----|----00----|
       |    --    |    --    |    --    |    --    |    --    |  S4PRIN2 |  S4PRIN1 |  S4PRIN0 |
       \----------|----------|----------|----------|----------|----------|----------|---------*/

	#define STV_VDP2_PRISC      (m_vdp2_regs[0x0f4/2])
	#define STV_VDP2_S5PRIN     ((STV_VDP2_PRISC & 0x0700) >> 8)
	#define STV_VDP2_S4PRIN     ((STV_VDP2_PRISC & 0x0007) >> 0)

/* 1800f6 - Priority Number (Sprite 6,7)
 bit-> /----15----|----14----|----13----|----12----|----11----|----10----|----09----|----08----\
       |    --    |    --    |    --    |    --    |    --    |  S7PRIN2 |  S7PRIN1 |  S7PRIN0 |
       |----07----|----06----|----05----|----04----|----03----|----02----|----01----|----00----|
       |    --    |    --    |    --    |    --    |    --    |  S6PRIN2 |  S6PRIN1 |  S6PRIN0 |
       \----------|----------|----------|----------|----------|----------|----------|---------*/

	#define STV_VDP2_PRISD      (m_vdp2_regs[0x0f6/2])
	#define STV_VDP2_S7PRIN     ((STV_VDP2_PRISD & 0x0700) >> 8)
	#define STV_VDP2_S6PRIN     ((STV_VDP2_PRISD & 0x0007) >> 0)


/* 1800f8 - PRINA - Priority Number (NBG 0,1)
 bit-> /----15----|----14----|----13----|----12----|----11----|----10----|----09----|----08----\
       |    --    |    --    |    --    |    --    |    --    |    --    |    --    |    --    |
       |----07----|----06----|----05----|----04----|----03----|----02----|----01----|----00----|
       |    --    |    --    |    --    |    --    |    --    |    --    |    --    |    --    |
       \----------|----------|----------|----------|----------|----------|----------|---------*/

	#define STV_VDP2_PRINA (m_vdp2_regs[0x0f8/2])

	#define STV_VDP2_N1PRIN ((STV_VDP2_PRINA & 0x0700) >> 8)
	#define STV_VDP2_N0PRIN ((STV_VDP2_PRINA & 0x0007) >> 0)

/* 1800fa - PRINB - Priority Number (NBG 2,3)
 bit-> /----15----|----14----|----13----|----12----|----11----|----10----|----09----|----08----\
       |    --    |    --    |    --    |    --    |    --    |    --    |    --    |    --    |
       |----07----|----06----|----05----|----04----|----03----|----02----|----01----|----00----|
       |    --    |    --    |    --    |    --    |    --    |    --    |    --    |    --    |
       \----------|----------|----------|----------|----------|----------|----------|---------*/

	#define STV_VDP2_PRINB (m_vdp2_regs[0x0fa/2])

	#define STV_VDP2_N3PRIN ((STV_VDP2_PRINB & 0x0700) >> 8)
	#define STV_VDP2_N2PRIN ((STV_VDP2_PRINB & 0x0007) >> 0)

/* 1800fc - Priority Number (RBG0)
 bit-> /----15----|----14----|----13----|----12----|----11----|----10----|----09----|----08----\
       |    --    |    --    |    --    |    --    |    --    |    --    |    --    |    --    |
       |----07----|----06----|----05----|----04----|----03----|----02----|----01----|----00----|
       |    --    |    --    |    --    |    --    |    --    |    --    |    --    |    --    |
       \----------|----------|----------|----------|----------|----------|----------|---------*/
	#define STV_VDP2_PRIR (m_vdp2_regs[0x0fc/2])

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

	#define STV_VDP2_CCRSA      (m_vdp2_regs[0x100/2])
	#define STV_VDP2_S1CCRT     ((STV_VDP2_CCRSA & 0x1f00) >> 8)
	#define STV_VDP2_S0CCRT     ((STV_VDP2_CCRSA & 0x001f) >> 0)

/* 180102 - Colour Calculation Ratio (Sprite 2,3)
 bit-> /----15----|----14----|----13----|----12----|----11----|----10----|----09----|----08----\
       |    --    |    --    |    --    |  S3CCRT4 |  S3CCRT3 |  S3CCRT2 |  S3CCRT1 |  S3CCRT0 |
       |----07----|----06----|----05----|----04----|----03----|----02----|----01----|----00----|
       |    --    |    --    |    --    |  S2CCRT4 |  S2CCRT3 |  S2CCRT2 |  S2CCRT1 |  S2CCRT0 |
       \----------|----------|----------|----------|----------|----------|----------|---------*/

	#define STV_VDP2_CCRSB      (m_vdp2_regs[0x102/2])
	#define STV_VDP2_S3CCRT     ((STV_VDP2_CCRSB & 0x1f00) >> 8)
	#define STV_VDP2_S2CCRT     ((STV_VDP2_CCRSB & 0x001f) >> 0)

/* 180104 - Colour Calculation Ratio (Sprite 4,5)
 bit-> /----15----|----14----|----13----|----12----|----11----|----10----|----09----|----08----\
       |    --    |    --    |    --    |  S5CCRT4 |  S5CCRT3 |  S5CCRT2 |  S5CCRT1 |  S5CCRT0 |
       |----07----|----06----|----05----|----04----|----03----|----02----|----01----|----00----|
       |    --    |    --    |    --    |  S4CCRT4 |  S4CCRT3 |  S4CCRT2 |  S4CCRT1 |  S4CCRT0 |
       \----------|----------|----------|----------|----------|----------|----------|---------*/

	#define STV_VDP2_CCRSC      (m_vdp2_regs[0x104/2])
	#define STV_VDP2_S5CCRT     ((STV_VDP2_CCRSC & 0x1f00) >> 8)
	#define STV_VDP2_S4CCRT     ((STV_VDP2_CCRSC & 0x001f) >> 0)

/* 180106 - Colour Calculation Ratio (Sprite 6,7)
 bit-> /----15----|----14----|----13----|----12----|----11----|----10----|----09----|----08----\
       |    --    |    --    |    --    |  S7CCRT4 |  S7CCRT3 |  S7CCRT2 |  S7CCRT1 |  S7CCRT0 |
       |----07----|----06----|----05----|----04----|----03----|----02----|----01----|----00----|
       |    --    |    --    |    --    |  S6CCRT4 |  S6CCRT3 |  S6CCRT2 |  S6CCRT1 |  S6CCRT0 |
       \----------|----------|----------|----------|----------|----------|----------|---------*/

	#define STV_VDP2_CCRSD      (m_vdp2_regs[0x106/2])
	#define STV_VDP2_S7CCRT     ((STV_VDP2_CCRSD & 0x1f00) >> 8)
	#define STV_VDP2_S6CCRT     ((STV_VDP2_CCRSD & 0x001f) >> 0)

/* 180108 - Colour Calculation Ratio (NBG 0,1)
 bit-> /----15----|----14----|----13----|----12----|----11----|----10----|----09----|----08----\
       |    --    |    --    |    --    | N1CCRT4  | N1CCRT3  | N1CCRT2  | N1CCRT1  | N1CCRT0  |
       |----07----|----06----|----05----|----04----|----03----|----02----|----01----|----00----|
       |    --    |    --    |    --    | N0CCRT4  | N0CCRT3  | N0CCRT2  | N0CCRT1  | N0CCRT0  |
       \----------|----------|----------|----------|----------|----------|----------|---------*/

	#define STV_VDP2_CCRNA  (m_vdp2_regs[0x108/2])
	#define STV_VDP2_N1CCRT ((STV_VDP2_CCRNA & 0x1f00) >> 8)
	#define STV_VDP2_N0CCRT (STV_VDP2_CCRNA & 0x1f)

/* 18010a - Colour Calculation Ratio (NBG 2,3)
 bit-> /----15----|----14----|----13----|----12----|----11----|----10----|----09----|----08----\
       |    --    |    --    |    --    | N3CCRT4  | N3CCRT3  | N3CCRT2  | N3CCRT1  | N3CCRT0  |
       |----07----|----06----|----05----|----04----|----03----|----02----|----01----|----00----|
       |    --    |    --    |    --    | N2CCRT4  | N2CCRT3  | N2CCRT2  | N2CCRT1  | N2CCRT0  |
       \----------|----------|----------|----------|----------|----------|----------|---------*/

	#define STV_VDP2_CCRNB  (m_vdp2_regs[0x10a/2])
	#define STV_VDP2_N3CCRT ((STV_VDP2_CCRNB & 0x1f00) >> 8)
	#define STV_VDP2_N2CCRT (STV_VDP2_CCRNB & 0x1f)

/* 18010c - Colour Calculation Ratio (RBG 0)
 bit-> /----15----|----14----|----13----|----12----|----11----|----10----|----09----|----08----\
       |    --    |    --    |    --    |    --    |    --    |    --    |    --    |    --    |
       |----07----|----06----|----05----|----04----|----03----|----02----|----01----|----00----|
       |    --    |    --    |    --    |    --    |    --    |    --    |    --    |    --    |
       \----------|----------|----------|----------|----------|----------|----------|---------*/

	#define STV_VDP2_CCRR   (m_vdp2_regs[0x10c/2])
	#define STV_VDP2_R0CCRT (STV_VDP2_CCRR & 0x1f)

/* 18010e - Colour Calculation Ratio (Line Colour Screen, Back Colour Screen)
 bit-> /----15----|----14----|----13----|----12----|----11----|----10----|----09----|----08----\
       |    --    |    --    |    --    |    --    |    --    |    --    |    --    |    --    |
       |----07----|----06----|----05----|----04----|----03----|----02----|----01----|----00----|
       |    --    |    --    |    --    |    --    |    --    |    --    |    --    |    --    |
       \----------|----------|----------|----------|----------|----------|----------|---------*/

	#define STV_VDP2_CCRLB   (m_vdp2_regs[0x10e/2])


/* 180110 - Colour Offset Enable
 bit-> /----15----|----14----|----13----|----12----|----11----|----10----|----09----|----08----\
       |    --    |    --    |    --    |    --    |    --    |    --    |    --    |    --    |
       |----07----|----06----|----05----|----04----|----03----|----02----|----01----|----00----|
       |    --    |    --    |    --    |    --    |    --    |    --    |    --    |    --    |
       \----------|----------|----------|----------|----------|----------|----------|---------*/

	#define STV_VDP2_CLOFEN (m_vdp2_regs[0x110/2])
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

	#define STV_VDP2_CLOFSL (m_vdp2_regs[0x112/2])
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

	#define STV_VDP2_COAR (m_vdp2_regs[0x114/2])

/* 180116 - Colour Offset A (Green)
 bit-> /----15----|----14----|----13----|----12----|----11----|----10----|----09----|----08----\
       |    --    |    --    |    --    |    --    |    --    |    --    |    --    |    --    |
       |----07----|----06----|----05----|----04----|----03----|----02----|----01----|----00----|
       |    --    |    --    |    --    |    --    |    --    |    --    |    --    |    --    |
       \----------|----------|----------|----------|----------|----------|----------|---------*/
	#define STV_VDP2_COAG (m_vdp2_regs[0x116/2])

/* 180118 - Colour Offset A (Blue)
 bit-> /----15----|----14----|----13----|----12----|----11----|----10----|----09----|----08----\
       |    --    |    --    |    --    |    --    |    --    |    --    |    --    |    --    |
       |----07----|----06----|----05----|----04----|----03----|----02----|----01----|----00----|
       |    --    |    --    |    --    |    --    |    --    |    --    |    --    |    --    |
       \----------|----------|----------|----------|----------|----------|----------|---------*/

	#define STV_VDP2_COAB (m_vdp2_regs[0x118/2])

/* 18011a - Colour Offset B (Red)
 bit-> /----15----|----14----|----13----|----12----|----11----|----10----|----09----|----08----\
       |    --    |    --    |    --    |    --    |    --    |    --    |    --    |    --    |
       |----07----|----06----|----05----|----04----|----03----|----02----|----01----|----00----|
       |    --    |    --    |    --    |    --    |    --    |    --    |    --    |    --    |
       \----------|----------|----------|----------|----------|----------|----------|---------*/
	#define STV_VDP2_COBR (m_vdp2_regs[0x11a/2])

/* 18011c - Colour Offset B (Green)
 bit-> /----15----|----14----|----13----|----12----|----11----|----10----|----09----|----08----\
       |    --    |    --    |    --    |    --    |    --    |    --    |    --    |    --    |
       |----07----|----06----|----05----|----04----|----03----|----02----|----01----|----00----|
       |    --    |    --    |    --    |    --    |    --    |    --    |    --    |    --    |
       \----------|----------|----------|----------|----------|----------|----------|---------*/
	#define STV_VDP2_COBG (m_vdp2_regs[0x11c/2])

/* 18011e - Colour Offset B (Blue)
 bit-> /----15----|----14----|----13----|----12----|----11----|----10----|----09----|----08----\
       |    --    |    --    |    --    |    --    |    --    |    --    |    --    |    --    |
       |----07----|----06----|----05----|----04----|----03----|----02----|----01----|----00----|
       |    --    |    --    |    --    |    --    |    --    |    --    |    --    |    --    |
       \----------|----------|----------|----------|----------|----------|----------|---------*/
	#define STV_VDP2_COBB (m_vdp2_regs[0x11e/2])


#define STV_VDP2_RBG_ROTATION_PARAMETER_A   1
#define STV_VDP2_RBG_ROTATION_PARAMETER_B   2


#define mul_fixed32( a, b ) mul_32x32_shift( a, b, 16 )

void saturn_state::stv_vdp2_fill_rotation_parameter_table( UINT8 rot_parameter )
{
	UINT32 address = 0;

	address = (((STV_VDP2_RPTAU << 16) | STV_VDP2_RPTAL) << 1);
	if ( rot_parameter == 1 )
	{
		address &= ~0x00000080;
	}
	else if ( rot_parameter == 2 )
	{
		address |= 0x00000080;
	}

	stv_current_rotation_parameter_table.xst  = (m_vdp2_vram[address/4] & 0x1fffffc0) | ((m_vdp2_vram[address/4] & 0x10000000) ? 0xe0000000 : 0x00000000 );
	stv_current_rotation_parameter_table.yst  = (m_vdp2_vram[address/4 + 1] & 0x1fffffc0) | ((m_vdp2_vram[address/4 + 1] & 0x10000000) ? 0xe0000000 : 0x00000000 );
	stv_current_rotation_parameter_table.zst  = (m_vdp2_vram[address/4 + 2] & 0x1fffffc0) | ((m_vdp2_vram[address/4 + 2] & 0x10000000) ? 0xe0000000 : 0x00000000 );
	stv_current_rotation_parameter_table.dxst = (m_vdp2_vram[address/4 + 3] & 0x0007ffc0) | ((m_vdp2_vram[address/4 + 3] & 0x00040000) ? 0xfff80000 : 0x00000000 );
	stv_current_rotation_parameter_table.dyst = (m_vdp2_vram[address/4 + 4] & 0x0007ffc0) | ((m_vdp2_vram[address/4 + 4] & 0x00040000) ? 0xfff80000 : 0x00000000 );
	stv_current_rotation_parameter_table.dx   = (m_vdp2_vram[address/4 + 5] & 0x0007ffc0) | ((m_vdp2_vram[address/4 + 5] & 0x00040000) ? 0xfff80000 : 0x00000000 );
	stv_current_rotation_parameter_table.dy   = (m_vdp2_vram[address/4 + 6] & 0x0007ffc0) | ((m_vdp2_vram[address/4 + 6] & 0x00040000) ? 0xfff80000 : 0x00000000 );
	stv_current_rotation_parameter_table.A    = (m_vdp2_vram[address/4 + 7] & 0x000fffc0) | ((m_vdp2_vram[address/4 + 7] & 0x00080000) ? 0xfff00000 : 0x00000000 );
	stv_current_rotation_parameter_table.B    = (m_vdp2_vram[address/4 + 8] & 0x000fffc0) | ((m_vdp2_vram[address/4 + 8] & 0x00080000) ? 0xfff00000 : 0x00000000 );
	stv_current_rotation_parameter_table.C    = (m_vdp2_vram[address/4 + 9] & 0x000fffc0) | ((m_vdp2_vram[address/4 + 9] & 0x00080000) ? 0xfff00000 : 0x00000000 );
	stv_current_rotation_parameter_table.D    = (m_vdp2_vram[address/4 + 10] & 0x000fffc0) | ((m_vdp2_vram[address/4 + 10] & 0x00080000) ? 0xfff00000 : 0x00000000 );
	stv_current_rotation_parameter_table.E    = (m_vdp2_vram[address/4 + 11] & 0x000fffc0) | ((m_vdp2_vram[address/4 + 11] & 0x00080000) ? 0xfff00000 : 0x00000000 );
	stv_current_rotation_parameter_table.F    = (m_vdp2_vram[address/4 + 12] & 0x000fffc0) | ((m_vdp2_vram[address/4 + 12] & 0x00080000) ? 0xfff00000 : 0x00000000 );
	stv_current_rotation_parameter_table.px   = (m_vdp2_vram[address/4 + 13] & 0x3fff0000) | ((m_vdp2_vram[address/4 + 13] & 0x30000000) ? 0xc0000000 : 0x00000000 );
	stv_current_rotation_parameter_table.py   = (m_vdp2_vram[address/4 + 13] & 0x00003fff) << 16;
	if ( stv_current_rotation_parameter_table.py & 0x20000000 ) stv_current_rotation_parameter_table.py |= 0xc0000000;
	stv_current_rotation_parameter_table.pz   = (m_vdp2_vram[address/4 + 14] & 0x3fff0000) | ((m_vdp2_vram[address/4 + 14] & 0x20000000) ? 0xc0000000 : 0x00000000 );
	stv_current_rotation_parameter_table.cx   = (m_vdp2_vram[address/4 + 15] & 0x3fff0000) | ((m_vdp2_vram[address/4 + 15] & 0x20000000) ? 0xc0000000 : 0x00000000 );
	stv_current_rotation_parameter_table.cy   = (m_vdp2_vram[address/4 + 15] & 0x00003fff) << 16;
	if ( stv_current_rotation_parameter_table.cy & 0x20000000 ) stv_current_rotation_parameter_table.cy |= 0xc0000000;
	stv_current_rotation_parameter_table.cz   = (m_vdp2_vram[address/4 + 16] & 0x3fff0000) | ((m_vdp2_vram[address/4 + 16] & 0x20000000) ? 0xc0000000 : 0x00000000 );
	stv_current_rotation_parameter_table.mx   = (m_vdp2_vram[address/4 + 17] & 0x3fffffc0) | ((m_vdp2_vram[address/4 + 17] & 0x20000000) ? 0xc0000000 : 0x00000000 );
	stv_current_rotation_parameter_table.my   = (m_vdp2_vram[address/4 + 18] & 0x3fffffc0) | ((m_vdp2_vram[address/4 + 18] & 0x20000000) ? 0xc0000000 : 0x00000000 );
	stv_current_rotation_parameter_table.kx   = (m_vdp2_vram[address/4 + 19] & 0x00ffffff) | ((m_vdp2_vram[address/4 + 19] & 0x00800000) ? 0xff000000 : 0x00000000 );
	stv_current_rotation_parameter_table.ky   = (m_vdp2_vram[address/4 + 20] & 0x00ffffff) | ((m_vdp2_vram[address/4 + 20] & 0x00800000) ? 0xff000000 : 0x00000000 );
	stv_current_rotation_parameter_table.kast = (m_vdp2_vram[address/4 + 21] & 0xffffffc0);
	stv_current_rotation_parameter_table.dkast= (m_vdp2_vram[address/4 + 22] & 0x03ffffc0) | ((m_vdp2_vram[address/4 + 22] & 0x02000000) ? 0xfc000000 : 0x00000000 );
	stv_current_rotation_parameter_table.dkax = (m_vdp2_vram[address/4 + 23] & 0x03ffffc0) | ((m_vdp2_vram[address/4 + 23] & 0x02000000) ? 0xfc000000 : 0x00000000 );

#define RP  stv_current_rotation_parameter_table

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
	#if 0
	if(LOG_ROZ == 2)
	{
		if(machine().input().code_pressed_once(JOYCODE_Y_UP_SWITCH))
			m_vdpdebug_roz++;

		if(machine().input().code_pressed_once(JOYCODE_Y_DOWN_SWITCH))
			m_vdpdebug_roz--;

		if(m_vdpdebug_roz > 10)
			m_vdpdebug_roz = 10;

		switch(m_vdpdebug_roz)
		{
			case 0: popmessage( "Rotation parameter Table (%d)", rot_parameter ); break;
			case 1: popmessage( "xst = %x, yst = %x, zst = %x", RP.xst, RP.yst, RP.zst ); break;
			case 2: popmessage( "dxst = %x, dyst = %x", RP.dxst, RP.dyst ); break;
			case 3: popmessage( "dx = %x, dy = %x", RP.dx, RP.dy ); break;
			case 4: popmessage( "A = %x, B = %x, C = %x, D = %x, E = %x, F = %x", RP.A, RP.B, RP.C, RP.D, RP.E, RP.F ); break;
			case 5: popmessage( "px = %x, py = %x, pz = %x", RP.px, RP.py, RP.pz ); break;
			case 6: popmessage( "cx = %x, cy = %x, cz = %x", RP.cx, RP.cy, RP.cz ); break;
			case 7: popmessage( "mx = %x, my = %x", RP.mx, RP.my ); break;
			case 8: popmessage( "kx = %x, ky = %x", RP.kx, RP.ky ); break;
			case 9: popmessage( "kast = %x, dkast = %x, dkax = %x", RP.kast, RP.dkast, RP.dkax ); break;
			case 10: break;
		}
	}
	#endif
}

/* check if RGB layer has rotation applied */
UINT8 saturn_state::stv_vdp2_is_rotation_applied(void)
{
#define _FIXED_1    (0x00010000)
#define _FIXED_0    (0x00000000)

	if ( RP.A == _FIXED_1 &&
			RP.B == _FIXED_0 &&
			RP.C == _FIXED_0 &&
			RP.D == _FIXED_0 &&
			RP.E == _FIXED_1 &&
			RP.F == _FIXED_0 &&
			RP.dxst == _FIXED_0 &&
			RP.dyst == _FIXED_1 &&
			RP.dx == _FIXED_1 &&
			RP.dy == _FIXED_0 &&
			RP.kx == _FIXED_1 &&
			RP.ky == _FIXED_1 )
	{
		return 0;
	}
	else
	{
		return 1;
	}
}

UINT8 saturn_state::stv_vdp2_are_map_registers_equal(void)
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

void saturn_state::stv_vdp2_check_fade_control_for_layer( void )
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

#define STV_VDP2_CP_NBG0_PNMDR      0x0
#define STV_VDP2_CP_NBG1_PNMDR      0x1
#define STV_VDP2_CP_NBG2_PNMDR      0x2
#define STV_VDP2_CP_NBG3_PNMDR      0x3
#define STV_VDP2_CP_NBG0_CPDR       0x4
#define STV_VDP2_CP_NBG1_CPDR       0x5
#define STV_VDP2_CP_NBG2_CPDR       0x6
#define STV_VDP2_CP_NBG3_CPDR       0x7

UINT8 saturn_state::stv_vdp2_check_vram_cycle_pattern_registers( UINT8 access_command_pnmdr, UINT8 access_command_cpdr, UINT8 bitmap_enable )
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

INLINE UINT32 stv_add_blend(UINT32 a, UINT32 b)
{
	rgb_t rb = (a & 0xff00ff) + (b & 0xff00ff);
	rgb_t g = (a & 0x00ff00) + (b & 0x00ff00);
	return rgb_t((rb & 0x1000000) ? 0xff : rb.r(),
		(g & 0x0010000) ? 0xff : g.g(),
		(rb & 0x0000100) ? 0xff : rb.b()
	);
}


void saturn_state::stv_vdp2_compute_color_offset( int *r, int *g, int *b, int cor )
{
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
	if(*r < 0)      { *r = 0; }
	if(*r > 0xff)   { *r = 0xff; }
	if(*g < 0)      { *g = 0; }
	if(*g > 0xff)   { *g = 0xff; }
	if(*b < 0)      { *b = 0; }
	if(*b > 0xff)   { *b = 0xff; }
}

void saturn_state::stv_vdp2_compute_color_offset_UINT32(rgb_t *rgb, int cor)
{
	int _r = rgb->r();
	int _g = rgb->g();
	int _b = rgb->b();
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
	if(_r < 0)      { _r = 0; }
	if(_r > 0xff)   { _r = 0xff; }
	if(_g < 0)      { _g = 0; }
	if(_g > 0xff)   { _g = 0xff; }
	if(_b < 0)      { _b = 0; }
	if(_b > 0xff)   { _b = 0xff; }

	*rgb = rgb_t(_r, _g, _b);
}

void saturn_state::stv_vdp2_drawgfxzoom(
		bitmap_rgb32 &dest_bmp,const rectangle &clip,gfx_element *gfx,
		UINT32 code,UINT32 color,int flipx,int flipy,int sx,int sy,
		int transparency,int transparent_color,int scalex, int scaley,
		int sprite_screen_width, int sprite_screen_height, int alpha)
{
	rectangle myclip;

	if (!scalex || !scaley) return;

	if (gfx->has_pen_usage() && transparency == STV_TRANSPARENCY_PEN)
	{
		int transmask = 0;

		transmask = 1 << (transparent_color & 0xff);

		if ((gfx->pen_usage(code) & ~transmask) == 0)
			/* character is totally transparent, no need to draw */
			return;
		else if ((gfx->pen_usage(code) & transmask) == 0)
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
	myclip = clip;
	myclip &= dest_bmp.cliprect();

	if( gfx )
	{
		const pen_t *pal = &m_palette->pen(gfx->colorbase() + gfx->granularity() * (color % gfx->colors()));
		const UINT8 *source_base = gfx->get_data(code % gfx->elements());

		//int sprite_screen_height = (scaley*gfx->height()+0x8000)>>16;
		//int sprite_screen_width = (scalex*gfx->width()+0x8000)>>16;

		if (sprite_screen_width && sprite_screen_height)
		{
			/* compute sprite increment per screen pixel */
			//int dx = (gfx->width()<<16)/sprite_screen_width;
			//int dy = (gfx->height()<<16)/sprite_screen_height;
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

			if( sx < myclip.min_x)
			{ /* clip left */
				int pixels = myclip.min_x-sx;
				sx += pixels;
				x_index_base += pixels*dx;
			}
			if( sy < myclip.min_y )
			{ /* clip top */
				int pixels = myclip.min_y-sy;
				sy += pixels;
				y_index += pixels*dy;
			}
			/* NS 980211 - fixed incorrect clipping */
			if( ex > myclip.max_x+1 )
			{ /* clip right */
				int pixels = ex-myclip.max_x-1;
				ex -= pixels;
			}
			if( ey > myclip.max_y+1 )
			{ /* clip bottom */
				int pixels = ey-myclip.max_y-1;
				ey -= pixels;
			}

			if( ex>sx )
			{ /* skip if inner loop doesn't draw anything */
				int y;

				/* case 0: STV_TRANSPARENCY_NONE */
				if (transparency == STV_TRANSPARENCY_NONE)
				{
					for( y=sy; y<ey; y++ )
					{
						const UINT8 *source = source_base + (y_index>>16) * gfx->rowbytes();
						UINT32 *dest = &dest_bmp.pix32(y);

						int x, x_index = x_index_base;
						for( x=sx; x<ex; x++ )
						{
							if(stv_vdp2_window_process(x,y))
								dest[x] = pal[source[x_index>>16]];
							x_index += dx;
						}

						y_index += dy;
					}
				} /* case 1: STV_TRANSPARENCY_PEN */
				else if (transparency == STV_TRANSPARENCY_PEN)
				{
					for( y=sy; y<ey; y++ )
					{
						const UINT8 *source = source_base + (y_index>>16) * gfx->rowbytes();
						UINT32 *dest = &dest_bmp.pix32(y);

						int x, x_index = x_index_base;
						for( x=sx; x<ex; x++ )
						{
							if(stv_vdp2_window_process(x,y))
							{
								int c = source[x_index>>16];
								if( c != transparent_color ) dest[x] = pal[c];
							}
							x_index += dx;
						}

						y_index += dy;
					}
				} /* case 6: STV_TRANSPARENCY_ALPHA */
				else if (transparency == STV_TRANSPARENCY_ALPHA)
				{
					for( y=sy; y<ey; y++ )
					{
						const UINT8 *source = source_base + (y_index>>16) * gfx->rowbytes();
						UINT32 *dest = &dest_bmp.pix32(y);

						int x, x_index = x_index_base;
						for( x=sx; x<ex; x++ )
						{
							if(stv_vdp2_window_process(x,y))
							{
								int c = source[x_index>>16];
								if( c != transparent_color ) dest[x] = alpha_blend_r32(dest[x], pal[c], alpha);
							}
							x_index += dx;
						}

						y_index += dy;
					}
				} /* case : STV_TRANSPARENCY_ADD_BLEND */
				else if (transparency == STV_TRANSPARENCY_ADD_BLEND )
				{
					for( y=sy; y<ey; y++ )
					{
						const UINT8 *source = source_base + (y_index>>16) * gfx->rowbytes();
						UINT32 *dest = &dest_bmp.pix32(y);

						int x, x_index = x_index_base;
						for( x=sx; x<ex; x++ )
						{
							if(stv_vdp2_window_process(x,y))
							{
								int c = source[x_index>>16];
								if( c != transparent_color ) dest[x] = stv_add_blend(dest[x],pal[c]);
							}
							x_index += dx;
						}

						y_index += dy;
					}
				}
			}
		}
	}
}

void saturn_state::stv_vdp2_drawgfxzoom_rgb555(
		bitmap_rgb32 &dest_bmp,const rectangle &clip,
		UINT32 code,UINT32 color,int flipx,int flipy,int sx,int sy,
		int transparency,int transparent_color,int scalex, int scaley,
		int sprite_screen_width, int sprite_screen_height, int alpha)
{
	rectangle myclip;
	UINT8* gfxdata;

	gfxdata = m_vdp2.gfx_decode + code * 0x20;

	if(stv2_current_tilemap.window_control.enabled[0] ||
		stv2_current_tilemap.window_control.enabled[1])
		popmessage("Window Enabled for RGB555 Zoom");

	if (!scalex || !scaley) return;

	#if 0
	if (gfx->has_pen_usage() && transparency == STV_TRANSPARENCY_PEN)
	{
		int transmask = 0;

		transmask = 1 << (transparent_color & 0xff);

		if ((gfx->pen_usage(code) & ~transmask) == 0)
			/* character is totally transparent, no need to draw */
			return;
		else if ((gfx->pen_usage(code) & transmask) == 0)
			/* character is totally opaque, can disable transparency */
			transparency = STV_TRANSPARENCY_NONE;
	}
	#endif

	/*
	scalex and scaley are 16.16 fixed point numbers
	1<<15 : shrink to 50%
	1<<16 : uniform scale
	1<<17 : double to 200%
	*/


	/* KW 991012 -- Added code to force clip to bitmap boundary */
	myclip = clip;
	myclip &= dest_bmp.cliprect();

//  if( gfx )
	{
//      const UINT8 *source_base = gfx->get_data(code % gfx->elements());

		//int sprite_screen_height = (scaley*gfx->height()+0x8000)>>16;
		//int sprite_screen_width = (scalex*gfx->width()+0x8000)>>16;

		if (sprite_screen_width && sprite_screen_height)
		{
			/* compute sprite increment per screen pixel */
			//int dx = (gfx->width()<<16)/sprite_screen_width;
			//int dy = (gfx->height()<<16)/sprite_screen_height;
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

			if( sx < myclip.min_x)
			{ /* clip left */
				int pixels = myclip.min_x-sx;
				sx += pixels;
				x_index_base += pixels*dx;
			}
			if( sy < myclip.min_y )
			{ /* clip top */
				int pixels = myclip.min_y-sy;
				sy += pixels;
				y_index += pixels*dy;
			}
			/* NS 980211 - fixed incorrect clipping */
			if( ex > myclip.max_x+1 )
			{ /* clip right */
				int pixels = ex-myclip.max_x-1;
				ex -= pixels;
			}
			if( ey > myclip.max_y+1 )
			{ /* clip bottom */
				int pixels = ey-myclip.max_y-1;
				ey -= pixels;
			}

			if( ex>sx )
			{ /* skip if inner loop doesn't draw anything */
				int y;

				/* case 0: STV_TRANSPARENCY_NONE */
				if (transparency == STV_TRANSPARENCY_NONE)
				{
					for( y=sy; y<ey; y++ )
					{
						const UINT8 *source = gfxdata + (y_index>>16)*16;
						UINT32 *dest = &dest_bmp.pix32(y);
						int r,g,b,data;

						int x, x_index = x_index_base;
						for( x=sx; x<ex; x++ )
						{
							data = (source[(x_index>>16)*2] << 8) | source[(x_index>>16)*2+1];
							b = pal5bit((data & 0x7c00) >> 10);
							g = pal5bit((data & 0x03e0) >> 5);
							r = pal5bit( data & 0x001f);
							if(stv2_current_tilemap.fade_control & 1)
								stv_vdp2_compute_color_offset(&r,&g,&b,stv2_current_tilemap.fade_control & 2);

							dest[x] = rgb_t(r, g, b);
							x_index += dx;
						}

						y_index += dy;
					}
				}

				/* case 1: STV_TRANSPARENCY_PEN */
				if (transparency == STV_TRANSPARENCY_PEN)
				{
					for( y=sy; y<ey; y++ )
					{
						const UINT8 *source = gfxdata + (y_index>>16)*16;
						UINT32 *dest = &dest_bmp.pix32(y);
						int r,g,b,data;

						int x, x_index = x_index_base;
						for( x=sx; x<ex; x++ )
						{
							data = (source[(x_index>>16)*2] << 8) | source[(x_index>>16)*2+1];
							b = pal5bit((data & 0x7c00) >> 10);
							g = pal5bit((data & 0x03e0) >> 5);
							r = pal5bit( data & 0x001f);
							if(stv2_current_tilemap.fade_control & 1)
								stv_vdp2_compute_color_offset(&r,&g,&b,stv2_current_tilemap.fade_control & 2);

							if( data ) dest[x] = rgb_t(r, g, b);
							x_index += dx;
						}

						y_index += dy;
					}
				}

				/* case 6: STV_TRANSPARENCY_ALPHA */
				if (transparency == STV_TRANSPARENCY_ALPHA)
				{
					for( y=sy; y<ey; y++ )
					{
						const UINT8 *source = gfxdata + (y_index>>16)*16;
						UINT32 *dest = &dest_bmp.pix32(y);
						int r,g,b,data;

						int x, x_index = x_index_base;
						for( x=sx; x<ex; x++ )
						{
							data = (source[(x_index>>16)*2] << 8) | source[(x_index>>16)*2+1];
							b = pal5bit((data & 0x7c00) >> 10);
							g = pal5bit((data & 0x03e0) >> 5);
							r = pal5bit( data & 0x001f);
							if(stv2_current_tilemap.fade_control & 1)
								stv_vdp2_compute_color_offset(&r,&g,&b,stv2_current_tilemap.fade_control & 2);

							if( data ) dest[x] = alpha_blend_r32(dest[x], rgb_t(r, g, b), alpha);
							x_index += dx;
						}

						y_index += dy;
					}
				}

				/* case : STV_TRANSPARENCY_ADD_BLEND */
				if (transparency == STV_TRANSPARENCY_ADD_BLEND )
				{
					for( y=sy; y<ey; y++ )
					{
						const UINT8 *source = gfxdata + (y_index>>16)*16;
						UINT32 *dest = &dest_bmp.pix32(y);
						int r,g,b,data;

						int x, x_index = x_index_base;
						for( x=sx; x<ex; x++ )
						{
							data = (source[(x_index*2+0)>>16]<<0)|(source[(x_index*2+1)>>16]<<8);
							b = pal5bit((data & 0x7c00) >> 10);
							g = pal5bit((data & 0x03e0) >> 5);
							r = pal5bit( data & 0x001f);
							if(stv2_current_tilemap.fade_control & 1)
								stv_vdp2_compute_color_offset(&r,&g,&b,stv2_current_tilemap.fade_control & 2);

							if( data ) dest[x] = stv_add_blend(dest[x], rgb_t(r, g, b));
							x_index += dx;
						}

						y_index += dy;
					}
				}

			}
		}
	}

}


void saturn_state::stv_vdp2_drawgfx_rgb555( bitmap_rgb32 &dest_bmp, const rectangle &clip, UINT32 code, int flipx, int flipy, int sx, int sy, int transparency, int alpha)
{
	rectangle myclip;
	UINT8* gfxdata;
	int sprite_screen_width, sprite_screen_height;

	gfxdata = m_vdp2.gfx_decode + code * 0x20;
	sprite_screen_width = sprite_screen_height = 8;

	if(stv2_current_tilemap.window_control.enabled[0] ||
		stv2_current_tilemap.window_control.enabled[1])
		popmessage("Window Enabled for RGB555 tiles");

	/* KW 991012 -- Added code to force clip to bitmap boundary */
	myclip = clip;
	myclip &= dest_bmp.cliprect();

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

		if( sx < myclip.min_x)
		{ /* clip left */
			int pixels = myclip.min_x-sx;
			sx += pixels;
			x_index_base += pixels*dx;
		}
		if( sy < myclip.min_y )
		{ /* clip top */
			int pixels = myclip.min_y-sy;
			sy += pixels;
			y_index += pixels*dy;
		}
		/* NS 980211 - fixed incorrect clipping */
		if( ex > myclip.max_x+1 )
		{ /* clip right */
			int pixels = ex-myclip.max_x-1;
			ex -= pixels;
		}
		if( ey > myclip.max_y+1 )
		{ /* clip bottom */
			int pixels = ey-myclip.max_y-1;
			ey -= pixels;
		}

		if( ex>sx )
		{ /* skip if inner loop doesn't draw anything */
			int y;

			for( y=sy; y<ey; y++ )
			{
				const UINT8 *source = gfxdata + (y_index>>16)*16;
				UINT32 *dest = &dest_bmp.pix32(y);
				UINT16 data;

				int x, x_index = x_index_base;
				for( x=sx; x<ex; x++ )
				{
					int r,g,b;

					data = (source[(x_index>>16)*2] << 8) | source[(x_index>>16)*2+1];
					if ((data & 0x8000) || (transparency == STV_TRANSPARENCY_NONE))
					{
						b = pal5bit((data & 0x7c00) >> 10);
						g = pal5bit((data & 0x03e0) >> 5);
						r = pal5bit( data & 0x001f);
						if(stv2_current_tilemap.fade_control & 1)
							stv_vdp2_compute_color_offset(&r,&g,&b,stv2_current_tilemap.fade_control & 2);

						if ( transparency == STV_TRANSPARENCY_ALPHA )
							dest[x] = alpha_blend_r32( dest[x], rgb_t(r, g, b), alpha );
						else
							dest[x] = rgb_t(r, g, b);
					}
					x_index += dx;
				}

				y_index += dy;
			}

		}

	}

}


void saturn_state::stv_vdp2_drawgfx_rgb888( bitmap_rgb32 &dest_bmp, const rectangle &clip, UINT32 code, int flipx, int flipy,
										int sx, int sy, int transparency, int alpha)
{
	rectangle myclip;
	UINT8* gfxdata;
	int sprite_screen_width, sprite_screen_height;

	gfxdata = m_vdp2.gfx_decode + code * 0x20;
	sprite_screen_width = sprite_screen_height = 8;

	if(stv2_current_tilemap.window_control.enabled[0] ||
		stv2_current_tilemap.window_control.enabled[1])
		popmessage("Window Enabled for RGB888 tiles");

	/* KW 991012 -- Added code to force clip to bitmap boundary */
	myclip = clip;
	myclip &= dest_bmp.cliprect();
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

		if( sx < myclip.min_x)
		{ /* clip left */
			int pixels = myclip.min_x-sx;
			sx += pixels;
			x_index_base += pixels*dx;
		}
		if( sy < myclip.min_y )
		{ /* clip top */
			int pixels = myclip.min_y-sy;
			sy += pixels;
			y_index += pixels*dy;
		}
		/* NS 980211 - fixed incorrect clipping */
		if( ex > myclip.max_x+1 )
		{ /* clip right */
			int pixels = ex-myclip.max_x-1;
			ex -= pixels;
		}
		if( ey > myclip.max_y+1 )
		{ /* clip bottom */
			int pixels = ey-myclip.max_y-1;
			ey -= pixels;
		}

		if( ex>sx )
		{ /* skip if inner loop doesn't draw anything */
			int y;

			for( y=sy; y<ey; y++ )
			{
				const UINT8 *source = gfxdata + (y_index>>16)*32;
				UINT32 *dest = &dest_bmp.pix32(y);
				UINT32 data;

				int x, x_index = x_index_base;

				for( x=sx; x<ex; x++ )
				{
					int r,g,b;

					data = (source[(x_index>>16)*4+0] << 24) | (source[(x_index>>16)*4+1] << 16) | (source[(x_index>>16)*4+2] << 8) | (source[(x_index>>16)*4+3] << 0);
					if ((data & 0x80000000) || (transparency == STV_TRANSPARENCY_NONE))
					{
						b = (data & 0xff0000) >> 16;
						g = (data & 0x00ff00) >> 8;
						r = (data & 0x0000ff);

						if(stv2_current_tilemap.fade_control & 1)
							stv_vdp2_compute_color_offset(&r,&g,&b,stv2_current_tilemap.fade_control & 2);

						if ( transparency == STV_TRANSPARENCY_ALPHA )
							dest[x] = alpha_blend_r32( dest[x], rgb_t(r, g, b), alpha );
						else
							dest[x] = rgb_t(r, g, b);
					}
					x_index += dx;
				}

				y_index += dy;
			}

		}

	}
}

void saturn_state::stv_vdp2_drawgfx_alpha(bitmap_rgb32 &dest_bmp,const rectangle &clip,gfx_element *gfx,
							UINT32 code,UINT32 color, int flipx,int flipy,int offsx,int offsy,
							int transparent_color, int alpha)
{
	const pen_t *pal = &m_palette->pen(gfx->colorbase() + gfx->granularity() * (color % gfx->colors()));
	const UINT8 *source_base = gfx->get_data(code % gfx->elements());
	int x_index_base, y_index, sx, sy, ex, ey;
	int xinc, yinc;

	xinc = flipx ? -1 : 1;
	yinc = flipy ? -1 : 1;

	x_index_base = flipx ? gfx->width()-1 : 0;
	y_index = flipy ? gfx->height()-1 : 0;

	/* start coordinates */
	sx = offsx;
	sy = offsy;

	/* end coordinates */
	ex = sx + gfx->width();
	ey = sy + gfx->height();

	/* clip left */
	if (sx < clip.min_x)
	{
		int pixels = clip.min_x-sx;
		sx += pixels;
		x_index_base += xinc*pixels;
	}

	/* clip top */
	if (sy < clip.min_y)
	{   int pixels = clip.min_y-sy;
		sy += pixels;
		y_index += yinc*pixels;
	}

	/* clip right */
	if (ex > clip.max_x+1)
	{
		ex = clip.max_x+1;
	}
	/* clip bottom */
	if (ey > clip.max_y+1)
	{
		ey = clip.max_y+1;
	}

	/* skip if inner loop doesn't draw anything */
	if (ex > sx)
	{
		int x, y;

		{
			for (y = sy; y < ey; y++)
			{
				const UINT8 *source = source_base + y_index*gfx->rowbytes();
				UINT32 *dest = &dest_bmp.pix32(y);
				int x_index = x_index_base;
				for (x = sx; x < ex; x++)
				{
					if(stv_vdp2_window_process(x,y))
					{
						int c = (source[x_index]);
						if (c != transparent_color)
							dest[x] = alpha_blend_r32( dest[x], pal[c], alpha );;
					}

					x_index += xinc;
				}
				y_index += yinc;
			}
		}
	}
}

void saturn_state::stv_vdp2_drawgfx_transpen(bitmap_rgb32 &dest_bmp,const rectangle &clip,gfx_element *gfx,
							UINT32 code,UINT32 color, int flipx,int flipy,int offsx,int offsy,
							int transparent_color)
{
	const pen_t *pal = &m_palette->pen(gfx->colorbase() + gfx->granularity() * (color % gfx->colors()));
	const UINT8 *source_base = gfx->get_data(code % gfx->elements());
	int x_index_base, y_index, sx, sy, ex, ey;
	int xinc, yinc;

	xinc = flipx ? -1 : 1;
	yinc = flipy ? -1 : 1;

	x_index_base = flipx ? gfx->width()-1 : 0;
	y_index = flipy ? gfx->height()-1 : 0;

	/* start coordinates */
	sx = offsx;
	sy = offsy;

	/* end coordinates */
	ex = sx + gfx->width();
	ey = sy + gfx->height();

	/* clip left */
	if (sx < clip.min_x)
	{
		int pixels = clip.min_x-sx;
		sx += pixels;
		x_index_base += xinc*pixels;
	}

	/* clip top */
	if (sy < clip.min_y)
	{   int pixels = clip.min_y-sy;
		sy += pixels;
		y_index += yinc*pixels;
	}

	/* clip right */
	if (ex > clip.max_x+1)
	{
		ex = clip.max_x+1;
	}
	/* clip bottom */
	if (ey > clip.max_y+1)
	{
		ey = clip.max_y+1;
	}

	/* skip if inner loop doesn't draw anything */
	if (ex > sx)
	{
		int x, y;

		{
			for (y = sy; y < ey; y++)
			{
				const UINT8 *source = source_base + y_index*gfx->rowbytes();
				UINT32 *dest = &dest_bmp.pix32(y);
				int x_index = x_index_base;
				for (x = sx; x < ex; x++)
				{
					if(stv_vdp2_window_process(x,y))
					{
						int c = (source[x_index]);
						if (c != transparent_color)
							dest[x] = pal[c];
					}

					x_index += xinc;
				}
				y_index += yinc;
			}
		}
	}
}

void saturn_state::draw_4bpp_bitmap(bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	int xsize, ysize, xsize_mask, ysize_mask;
	int xsrc,ysrc,xdst,ydst;
	int src_offs;
	UINT8* vram = m_vdp2.gfx_decode;
	UINT32 map_offset = stv2_current_tilemap.bitmap_map * 0x20000;
	int scrollx = stv2_current_tilemap.scrollx;
	int scrolly = stv2_current_tilemap.scrolly;
	UINT16 dot_data;
	UINT16 pal_bank;

	xsize = (stv2_current_tilemap.bitmap_size & 2) ? 1024 : 512;
	ysize = (stv2_current_tilemap.bitmap_size & 1) ? 512 : 256;

	xsize_mask = (stv2_current_tilemap.linescroll_enable) ? 1024 : xsize;
	ysize_mask = (stv2_current_tilemap.vertical_linescroll_enable) ? 512 : ysize;

	pal_bank = stv2_current_tilemap.bitmap_palette_number;
	pal_bank+= stv2_current_tilemap.colour_ram_address_offset;
	pal_bank&= 7;
	pal_bank<<=8;
	if(stv2_current_tilemap.fade_control & 1)
		pal_bank += ((stv2_current_tilemap.fade_control & 2) ? (2*2048) : (2048));

	for(ydst=cliprect.min_y;ydst<=cliprect.max_y;ydst++)
	{
		for(xdst=cliprect.min_x;xdst<=cliprect.max_x;xdst++)
		{
			if(!stv_vdp2_window_process(xdst,ydst))
				continue;

			xsrc = (xdst + scrollx) & (xsize_mask-1);
			ysrc = (ydst + scrolly) & (ysize_mask-1);
			src_offs = (xsrc + (ysrc*xsize));
			src_offs/= 2;
			src_offs += map_offset;
			src_offs &= 0x7ffff;

			dot_data = vram[src_offs] >> ((xsrc & 1) ? 0 : 4);
			dot_data&= 0xf;

			if ((dot_data != 0) || (stv2_current_tilemap.transparency == STV_TRANSPARENCY_NONE))
			{
				dot_data += pal_bank;

				if ( stv2_current_tilemap.colour_calculation_enabled == 0 )
					bitmap.pix32(ydst, xdst) = m_palette->pen(dot_data);
				else
					bitmap.pix32(ydst, xdst) = alpha_blend_r32(bitmap.pix32(ydst, xdst), m_palette->pen(dot_data), stv2_current_tilemap.alpha);
			}
		}
	}
}


void saturn_state::draw_8bpp_bitmap(bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	int xsize, ysize, xsize_mask, ysize_mask;
	int xsrc,ysrc,xdst,ydst;
	int src_offs;
	UINT8* vram = m_vdp2.gfx_decode;
	UINT32 map_offset = stv2_current_tilemap.bitmap_map * 0x20000;
	int scrollx = stv2_current_tilemap.scrollx;
	int scrolly = stv2_current_tilemap.scrolly;
	UINT16 dot_data;
	UINT16 pal_bank;
	int xf, yf;

	xsize = (stv2_current_tilemap.bitmap_size & 2) ? 1024 : 512;
	ysize = (stv2_current_tilemap.bitmap_size & 1) ? 512 : 256;

	xsize_mask = (stv2_current_tilemap.linescroll_enable) ? 1024 : xsize;
	ysize_mask = (stv2_current_tilemap.vertical_linescroll_enable) ? 512 : ysize;

	pal_bank = stv2_current_tilemap.bitmap_palette_number;
	pal_bank+= stv2_current_tilemap.colour_ram_address_offset;
	pal_bank&= 7;
	pal_bank<<=8;
	if(stv2_current_tilemap.fade_control & 1)
		pal_bank += ((stv2_current_tilemap.fade_control & 2) ? (2*2048) : (2048));

	for(ydst=cliprect.min_y;ydst<=cliprect.max_y;ydst++)
	{
		for(xdst=cliprect.min_x;xdst<=cliprect.max_x;xdst++)
		{
			if(!stv_vdp2_window_process(xdst,ydst))
				continue;

			xf = stv2_current_tilemap.incx * xdst;
			xf>>=16;
			yf = stv2_current_tilemap.incy * ydst;
			yf>>=16;

			xsrc = (xf + scrollx) & (xsize_mask-1);
			ysrc = (yf + scrolly) & (ysize_mask-1);
			src_offs = (xsrc + (ysrc*xsize));
			src_offs += map_offset;
			src_offs &= 0x7ffff;

			dot_data = vram[src_offs];

			if ((dot_data != 0) || (stv2_current_tilemap.transparency == STV_TRANSPARENCY_NONE))
			{
				dot_data += pal_bank;

				if ( stv2_current_tilemap.colour_calculation_enabled == 0 )
					bitmap.pix32(ydst, xdst) = m_palette->pen(dot_data);
				else
					bitmap.pix32(ydst, xdst) = alpha_blend_r32(bitmap.pix32(ydst, xdst), m_palette->pen(dot_data), stv2_current_tilemap.alpha);
			}
		}
	}
}

void saturn_state::draw_11bpp_bitmap(bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	int xsize, ysize, xsize_mask, ysize_mask;
	int xsrc,ysrc,xdst,ydst;
	int src_offs;
	UINT8* vram = m_vdp2.gfx_decode;
	UINT32 map_offset = stv2_current_tilemap.bitmap_map * 0x20000;
	int scrollx = stv2_current_tilemap.scrollx;
	int scrolly = stv2_current_tilemap.scrolly;
	UINT16 dot_data;
	UINT16 pal_bank;
	int xf, yf;

	xsize = (stv2_current_tilemap.bitmap_size & 2) ? 1024 : 512;
	ysize = (stv2_current_tilemap.bitmap_size & 1) ? 512 : 256;

	xsize_mask = (stv2_current_tilemap.linescroll_enable) ? 1024 : xsize;
	ysize_mask = (stv2_current_tilemap.vertical_linescroll_enable) ? 512 : ysize;

	pal_bank = 0;
	if(stv2_current_tilemap.fade_control & 1)
		pal_bank = ((stv2_current_tilemap.fade_control & 2) ? (2*2048) : (2048));

	for(ydst=cliprect.min_y;ydst<=cliprect.max_y;ydst++)
	{
		for(xdst=cliprect.min_x;xdst<=cliprect.max_x;xdst++)
		{
			if(!stv_vdp2_window_process(xdst,ydst))
				continue;

			xf = stv2_current_tilemap.incx * xdst;
			xf>>=16;
			yf = stv2_current_tilemap.incy * ydst;
			yf>>=16;

			xsrc = (xf + scrollx) & (xsize_mask-1);
			ysrc = (yf + scrolly) & (ysize_mask-1);
			src_offs = (xsrc + (ysrc*xsize));
			src_offs *= 2;
			src_offs += map_offset;
			src_offs &= 0x7ffff;

			dot_data = ((vram[src_offs]<<8)|(vram[src_offs+1]<<0)) & 0x7ff;

			if ((dot_data != 0) || (stv2_current_tilemap.transparency == STV_TRANSPARENCY_NONE))
			{
				dot_data += pal_bank;

				if ( stv2_current_tilemap.colour_calculation_enabled == 0 )
					bitmap.pix32(ydst, xdst) = m_palette->pen(dot_data);
				else
					bitmap.pix32(ydst, xdst) = alpha_blend_r32(bitmap.pix32(ydst, xdst), m_palette->pen(dot_data), stv2_current_tilemap.alpha);
			}
		}
	}
}


void saturn_state::draw_rgb15_bitmap(bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	int xsize, ysize, xsize_mask, ysize_mask;
	int xsrc,ysrc,xdst,ydst;
	int src_offs;
	UINT8* vram = m_vdp2.gfx_decode;
	UINT32 map_offset = stv2_current_tilemap.bitmap_map * 0x20000;
	int scrollx = stv2_current_tilemap.scrollx;
	int scrolly = stv2_current_tilemap.scrolly;
	int r,g,b;
	UINT16 dot_data;
	int xf, yf;

	xsize = (stv2_current_tilemap.bitmap_size & 2) ? 1024 : 512;
	ysize = (stv2_current_tilemap.bitmap_size & 1) ? 512 : 256;

	xsize_mask = (stv2_current_tilemap.linescroll_enable) ? 1024 : xsize;
	ysize_mask = (stv2_current_tilemap.vertical_linescroll_enable) ? 512 : ysize;

	for(ydst=cliprect.min_y;ydst<=cliprect.max_y;ydst++)
	{
		for(xdst=cliprect.min_x;xdst<=cliprect.max_x;xdst++)
		{
			if(!stv_vdp2_window_process(xdst,ydst))
				continue;

			xf = stv2_current_tilemap.incx * xdst;
			xf>>=16;
			yf = stv2_current_tilemap.incy * ydst;
			yf>>=16;

			xsrc = (xf + scrollx) & (xsize_mask-1);
			ysrc = (yf + scrolly) & (ysize_mask-1);
			src_offs = (xsrc + (ysrc*xsize));
			src_offs *= 2;
			src_offs += map_offset;
			src_offs &= 0x7ffff;

			dot_data =(vram[src_offs]<<8)|(vram[src_offs+1]<<0);

			if ((dot_data & 0x8000) || (stv2_current_tilemap.transparency == STV_TRANSPARENCY_NONE))
			{
				b = pal5bit((dot_data & 0x7c00) >> 10);
				g = pal5bit((dot_data & 0x03e0) >> 5);
				r = pal5bit((dot_data & 0x001f) >> 0);

				if(stv2_current_tilemap.fade_control & 1)
					stv_vdp2_compute_color_offset(&r,&g,&b,stv2_current_tilemap.fade_control & 2);

				if ( stv2_current_tilemap.colour_calculation_enabled == 0 )
					bitmap.pix32(ydst, xdst) = rgb_t(r, g, b);
				else
					bitmap.pix32(ydst, xdst) = alpha_blend_r32( bitmap.pix32(ydst, xdst), rgb_t(r, g, b), stv2_current_tilemap.alpha );
			}
		}
	}
}

void saturn_state::draw_rgb32_bitmap(bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	int xsize, ysize, xsize_mask, ysize_mask;
	int xsrc,ysrc,xdst,ydst;
	int src_offs;
	UINT8* vram = m_vdp2.gfx_decode;
	UINT32 map_offset = stv2_current_tilemap.bitmap_map * 0x20000;
	int scrollx = stv2_current_tilemap.scrollx;
	int scrolly = stv2_current_tilemap.scrolly;
	int r,g,b;
	UINT32 dot_data;
	int xf, yf;

	xsize = (stv2_current_tilemap.bitmap_size & 2) ? 1024 : 512;
	ysize = (stv2_current_tilemap.bitmap_size & 1) ? 512 : 256;

	xsize_mask = (stv2_current_tilemap.linescroll_enable) ? 1024 : xsize;
	ysize_mask = (stv2_current_tilemap.vertical_linescroll_enable) ? 512 : ysize;

	for(ydst=cliprect.min_y;ydst<=cliprect.max_y;ydst++)
	{
		for(xdst=cliprect.min_x;xdst<=cliprect.max_x;xdst++)
		{
			if(!stv_vdp2_window_process(xdst,ydst))
				continue;

			xf = stv2_current_tilemap.incx * xdst;
			xf>>=16;
			yf = stv2_current_tilemap.incy * ydst;
			yf>>=16;

			xsrc = (xf + scrollx) & (xsize_mask-1);
			ysrc = (yf + scrolly) & (ysize_mask-1);
			src_offs = (xsrc + (ysrc*xsize));
			src_offs *= 4;
			src_offs += map_offset;
			src_offs &= 0x7ffff;

			dot_data = (vram[src_offs+0]<<24)|(vram[src_offs+1]<<16)|(vram[src_offs+2]<<8)|(vram[src_offs+3]<<0);

			if ((dot_data & 0x80000000) || (stv2_current_tilemap.transparency == STV_TRANSPARENCY_NONE))
			{
				b = ((dot_data & 0x00ff0000) >> 16);
				g = ((dot_data & 0x0000ff00) >> 8);
				r = ((dot_data & 0x000000ff) >> 0);

				if(stv2_current_tilemap.fade_control & 1)
					stv_vdp2_compute_color_offset(&r,&g,&b,stv2_current_tilemap.fade_control & 2);

				if ( stv2_current_tilemap.colour_calculation_enabled == 0 )
					bitmap.pix32(ydst, xdst) = rgb_t(r, g, b);
				else
					bitmap.pix32(ydst, xdst) = alpha_blend_r32( bitmap.pix32(ydst, xdst), rgb_t(r, g, b), stv2_current_tilemap.alpha );
			}
		}
	}
}


void saturn_state::stv_vdp2_draw_basic_bitmap(bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	if (!stv2_current_tilemap.enabled) return;

	/* new bitmap code, supposed to rewrite the old one. Not supposed to be clean, but EFFICIENT! */
	if(stv2_current_tilemap.incx == 0x10000 && stv2_current_tilemap.incy == 0x10000)
	{
		switch(stv2_current_tilemap.colour_depth)
		{
			case 0: draw_4bpp_bitmap(bitmap,cliprect); return;
			case 1: draw_8bpp_bitmap(bitmap,cliprect); return;
			case 2: draw_11bpp_bitmap(bitmap, cliprect); return;
			case 3: draw_rgb15_bitmap(bitmap,cliprect); return;
			case 4: draw_rgb32_bitmap(bitmap,cliprect); return;
		}

		/* intentional fall-through*/
		popmessage("%d %s %s %s",stv2_current_tilemap.colour_depth,
									stv2_current_tilemap.transparency == STV_TRANSPARENCY_NONE ? "no trans" : "trans",
									stv2_current_tilemap.colour_calculation_enabled ? "cc" : "no cc",
									(stv2_current_tilemap.incx == 0x10000 && stv2_current_tilemap.incy == 0x10000) ? "no zoom" : "zoom");
	}
	else
	{
		switch(stv2_current_tilemap.colour_depth)
		{
		//  case 0: draw_4bpp_bitmap(bitmap,cliprect); return;
			case 1: draw_8bpp_bitmap(bitmap,cliprect); return;
		//  case 2: draw_11bpp_bitmap(bitmap, cliprect); return;
			case 3: draw_rgb15_bitmap(bitmap,cliprect); return;
			case 4: draw_rgb32_bitmap(bitmap,cliprect); return;
		}

		/* intentional fall-through*/
		popmessage("%d %s %s %s",stv2_current_tilemap.colour_depth,
									stv2_current_tilemap.transparency == STV_TRANSPARENCY_NONE ? "no trans" : "trans",
									stv2_current_tilemap.colour_calculation_enabled ? "cc" : "no cc",
									(stv2_current_tilemap.incx == 0x10000 && stv2_current_tilemap.incy == 0x10000) ? "no zoom" : "zoom");
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

void saturn_state::stv_vdp2_get_map_page( int x, int y, int *_map, int *_page )
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

void saturn_state::stv_vdp2_draw_basic_tilemap(bitmap_rgb32 &bitmap, const rectangle &cliprect)
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
	int plsize_bytes/*, plsize_dwords*/;

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
	//plsize_dwords = plsize_bytes /4;
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
			while( ((drawypos + drawyposinc) >> 16) < cliprect.min_y )
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
		if ((drawypos >> 16) > cliprect.max_y) break;

		ypageoffs = y & (pgtiles_y-1);

		for (x = 0; x<mptiles_x; x++) {
			int xpageoffs;
			int tilecodespacing = 1;

			if ( x == 0 )
			{
				int drawxposinc = tilesizex*(stv2_current_tilemap.tile_size ? 2 : 1);
				drawxpos = -(stv2_current_tilemap.scrollx*scalex);
				while( ((drawxpos + drawxposinc) >> 16) < cliprect.min_x )
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
			if ( (drawxpos >> 16) > cliprect.max_x ) break;

			xpageoffs = x & (pgtiles_x-1);

			stv_vdp2_get_map_page(x,y,&map,&page);

			newbase = base[map] + page * pgsize_dwords;
			offs = (ypageoffs * pgtiles_x) + xpageoffs;

/* GET THE TILE INFO ... */
			/* 1 word per tile mode with supplement bits */
			if (stv2_current_tilemap.pattern_data_size ==1)
			{
				data = m_vdp2_vram[newbase + offs/2];
				data = (offs&1) ? (data & 0x0000ffff) : ((data & 0xffff0000) >> 16);

				/* Supplement Mode 12 bits, no flip */
				if (stv2_current_tilemap.character_number_supplement == 1)
				{
/* no flip */       flipyx   = 0;
/* 8x8 */           if (stv2_current_tilemap.tile_size==0) tilecode = (data & 0x0fff) + ( (stv2_current_tilemap.supplementary_character_bits&0x1c) << 10);
/* 16x16 */         else tilecode = ((data & 0x0fff) << 2) + (stv2_current_tilemap.supplementary_character_bits&0x03) + ((stv2_current_tilemap.supplementary_character_bits&0x10) << 10);
				}
				/* Supplement Mode 10 bits, with flip */
				else
				{
/* flip bits */     flipyx   = (data & 0x0c00) >> 10;
/* 8x8 */           if (stv2_current_tilemap.tile_size==0) tilecode = (data & 0x03ff) +  ( (stv2_current_tilemap.supplementary_character_bits) << 10);
/* 16x16 */         else tilecode = ((data & 0x03ff) <<2) +  (stv2_current_tilemap.supplementary_character_bits&0x03) + ((stv2_current_tilemap.supplementary_character_bits&0x1c) << 10);
				}

/*>16cols*/     if (stv2_current_tilemap.colour_depth != 0) pal = ((data & 0x7000)>>8);
/*16 cols*/     else pal = ((data & 0xf000)>>12) +( (stv2_current_tilemap.supplementary_palette_bits) << 4);

			}
			/* 2 words per tile, no supplement bits */
			else
			{
				data = m_vdp2_vram[newbase + offs];
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
				if (tilecode == 0x7fff) tilecode--; /* prevents crash but unsure what should happen; wrapping? */
				tilecodespacing = 2;
			}
			else if (stv2_current_tilemap.colour_depth == 0)
			{
				gfx = 0;
				tilecode &=0x7fff;
				tilecodespacing = 1;
			}
/* TILES ARE NOW DECODED */

			if(!STV_VDP2_VRAMSZ)
				tilecode &= 0x3fff;

/* DRAW! */
			if(stv2_current_tilemap.incx != 0x10000 ||
				stv2_current_tilemap.incy != 0x10000 ||
				stv2_current_tilemap.transparency == STV_TRANSPARENCY_ADD_BLEND )
			{
#define SCR_TILESIZE_X          (((drawxpos + tilesizex) >> 16) - (drawxpos >> 16))
#define SCR_TILESIZE_X1(startx) (((drawxpos + (startx) + tilesizex) >> 16) - ((drawxpos + (startx))>>16))
#define SCR_TILESIZE_Y          (((drawypos + tilesizey) >> 16) - (drawypos >> 16))
#define SCR_TILESIZE_Y1(starty) (((drawypos + (starty) + tilesizey) >> 16) - ((drawypos + (starty))>>16))
				if (stv2_current_tilemap.tile_size==1)
				{
					if ( stv2_current_tilemap.colour_depth == 4 )
					{
						popmessage("Unsupported tilemap gfx zoom color depth = 4, tile size = 1, contact MAMEdev");
					}
					else if ( stv2_current_tilemap.colour_depth == 3 )
					{
						/* RGB555 */
						stv_vdp2_drawgfxzoom_rgb555(bitmap,cliprect,tilecode+(0+(flipyx&1)+(flipyx&2))*tilecodespacing,pal,flipyx&1,flipyx&2,drawxpos >> 16, drawypos >> 16,stv2_current_tilemap.transparency,0,scalex,scaley,SCR_TILESIZE_X, SCR_TILESIZE_Y,stv2_current_tilemap.alpha);
						stv_vdp2_drawgfxzoom_rgb555(bitmap,cliprect,tilecode+(1-(flipyx&1)+(flipyx&2))*tilecodespacing,pal,flipyx&1,flipyx&2,(drawxpos+tilesizex) >> 16,drawypos >> 16,stv2_current_tilemap.transparency,0,scalex,scaley,SCR_TILESIZE_X1(tilesizex), SCR_TILESIZE_Y,stv2_current_tilemap.alpha);
						stv_vdp2_drawgfxzoom_rgb555(bitmap,cliprect,tilecode+(2+(flipyx&1)-(flipyx&2))*tilecodespacing,pal,flipyx&1,flipyx&2,drawxpos >> 16,(drawypos+tilesizey) >> 16,stv2_current_tilemap.transparency,0,scalex,scaley,SCR_TILESIZE_X, SCR_TILESIZE_Y1(tilesizey),stv2_current_tilemap.alpha);
						stv_vdp2_drawgfxzoom_rgb555(bitmap,cliprect,tilecode+(3-(flipyx&1)-(flipyx&2))*tilecodespacing,pal,flipyx&1,flipyx&2,(drawxpos+tilesizex)>> 16,(drawypos+tilesizey) >> 16,stv2_current_tilemap.transparency,0,scalex,scaley,SCR_TILESIZE_X1(tilesizex), SCR_TILESIZE_Y1(tilesizey),stv2_current_tilemap.alpha);
					}
					else
					{
						/* normal */
						stv_vdp2_drawgfxzoom(bitmap,cliprect,m_gfxdecode->gfx(gfx),tilecode+(0+(flipyx&1)+(flipyx&2))*tilecodespacing,pal,flipyx&1,flipyx&2,drawxpos >> 16, drawypos >> 16,stv2_current_tilemap.transparency,0,scalex,scaley,SCR_TILESIZE_X, SCR_TILESIZE_Y,stv2_current_tilemap.alpha);
						stv_vdp2_drawgfxzoom(bitmap,cliprect,m_gfxdecode->gfx(gfx),tilecode+(1-(flipyx&1)+(flipyx&2))*tilecodespacing,pal,flipyx&1,flipyx&2,(drawxpos+tilesizex) >> 16,drawypos >> 16,stv2_current_tilemap.transparency,0,scalex,scaley,SCR_TILESIZE_X1(tilesizex), SCR_TILESIZE_Y,stv2_current_tilemap.alpha);
						stv_vdp2_drawgfxzoom(bitmap,cliprect,m_gfxdecode->gfx(gfx),tilecode+(2+(flipyx&1)-(flipyx&2))*tilecodespacing,pal,flipyx&1,flipyx&2,drawxpos >> 16,(drawypos+tilesizey) >> 16,stv2_current_tilemap.transparency,0,scalex,scaley,SCR_TILESIZE_X, SCR_TILESIZE_Y1(tilesizey),stv2_current_tilemap.alpha);
						stv_vdp2_drawgfxzoom(bitmap,cliprect,m_gfxdecode->gfx(gfx),tilecode+(3-(flipyx&1)-(flipyx&2))*tilecodespacing,pal,flipyx&1,flipyx&2,(drawxpos+tilesizex)>> 16,(drawypos+tilesizey) >> 16,stv2_current_tilemap.transparency,0,scalex,scaley,SCR_TILESIZE_X1(tilesizex), SCR_TILESIZE_Y1(tilesizey),stv2_current_tilemap.alpha);
					}
				}
				else
				{
					if ( stv2_current_tilemap.colour_depth == 4 )
						popmessage("Unsupported tilemap gfx zoom color depth = 4, tile size = 0, contact MAMEdev");
					else if ( stv2_current_tilemap.colour_depth == 3)
					{
						stv_vdp2_drawgfxzoom_rgb555(bitmap,cliprect,tilecode,pal,flipyx&1,flipyx&2, drawxpos >> 16, drawypos >> 16,stv2_current_tilemap.transparency,0,scalex,scaley,SCR_TILESIZE_X,SCR_TILESIZE_Y,stv2_current_tilemap.alpha);
					}
					else
						stv_vdp2_drawgfxzoom(bitmap,cliprect,m_gfxdecode->gfx(gfx),tilecode,pal,flipyx&1,flipyx&2, drawxpos >> 16, drawypos >> 16,stv2_current_tilemap.transparency,0,scalex,scaley,SCR_TILESIZE_X,SCR_TILESIZE_Y,stv2_current_tilemap.alpha);
				}
			}
			else
			{
				int olddrawxpos, olddrawypos;
				olddrawxpos = drawxpos; drawxpos >>= 16;
				olddrawypos = drawypos; drawypos >>= 16;
				if (stv2_current_tilemap.tile_size==1)
				{
					if ( stv2_current_tilemap.colour_depth == 4 )
					{
						/* normal */
						stv_vdp2_drawgfx_rgb888(bitmap,cliprect,tilecode+(0+(flipyx&1)+(flipyx&2))*4,flipyx&1,flipyx&2,drawxpos, drawypos,stv2_current_tilemap.transparency,stv2_current_tilemap.alpha);
						stv_vdp2_drawgfx_rgb888(bitmap,cliprect,tilecode+(1-(flipyx&1)+(flipyx&2))*4,flipyx&1,flipyx&2,drawxpos+8,drawypos,stv2_current_tilemap.transparency,stv2_current_tilemap.alpha);
						stv_vdp2_drawgfx_rgb888(bitmap,cliprect,tilecode+(2+(flipyx&1)-(flipyx&2))*4,flipyx&1,flipyx&2,drawxpos,drawypos+8,stv2_current_tilemap.transparency,stv2_current_tilemap.alpha);
						stv_vdp2_drawgfx_rgb888(bitmap,cliprect,tilecode+(3-(flipyx&1)-(flipyx&2))*4,flipyx&1,flipyx&2,drawxpos+8,drawypos+8,stv2_current_tilemap.transparency,stv2_current_tilemap.alpha);
					}
					else if ( stv2_current_tilemap.colour_depth == 3 )
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
						stv_vdp2_drawgfx_alpha(bitmap,cliprect,m_gfxdecode->gfx(gfx),tilecode+(0+(flipyx&1)+(flipyx&2))*tilecodespacing,pal,flipyx&1,flipyx&2,drawxpos, drawypos,0,stv2_current_tilemap.alpha);
						stv_vdp2_drawgfx_alpha(bitmap,cliprect,m_gfxdecode->gfx(gfx),tilecode+(1-(flipyx&1)+(flipyx&2))*tilecodespacing,pal,flipyx&1,flipyx&2,drawxpos+8,drawypos,0,stv2_current_tilemap.alpha);
						stv_vdp2_drawgfx_alpha(bitmap,cliprect,m_gfxdecode->gfx(gfx),tilecode+(2+(flipyx&1)-(flipyx&2))*tilecodespacing,pal,flipyx&1,flipyx&2,drawxpos,drawypos+8,0,stv2_current_tilemap.alpha);
						stv_vdp2_drawgfx_alpha(bitmap,cliprect,m_gfxdecode->gfx(gfx),tilecode+(3-(flipyx&1)-(flipyx&2))*tilecodespacing,pal,flipyx&1,flipyx&2,drawxpos+8,drawypos+8,0,stv2_current_tilemap.alpha);
					}
					else
					{
						/* normal */
						stv_vdp2_drawgfx_transpen(bitmap,cliprect,m_gfxdecode->gfx(gfx),tilecode+(0+(flipyx&1)+(flipyx&2))*tilecodespacing,pal,flipyx&1,flipyx&2,drawxpos, drawypos,(stv2_current_tilemap.transparency==STV_TRANSPARENCY_PEN)?0:-1);
						stv_vdp2_drawgfx_transpen(bitmap,cliprect,m_gfxdecode->gfx(gfx),tilecode+(1-(flipyx&1)+(flipyx&2))*tilecodespacing,pal,flipyx&1,flipyx&2,drawxpos+8,drawypos,(stv2_current_tilemap.transparency==STV_TRANSPARENCY_PEN)?0:-1);
						stv_vdp2_drawgfx_transpen(bitmap,cliprect,m_gfxdecode->gfx(gfx),tilecode+(2+(flipyx&1)-(flipyx&2))*tilecodespacing,pal,flipyx&1,flipyx&2,drawxpos,drawypos+8,(stv2_current_tilemap.transparency==STV_TRANSPARENCY_PEN)?0:-1);
						stv_vdp2_drawgfx_transpen(bitmap,cliprect,m_gfxdecode->gfx(gfx),tilecode+(3-(flipyx&1)-(flipyx&2))*tilecodespacing,pal,flipyx&1,flipyx&2,drawxpos+8,drawypos+8,(stv2_current_tilemap.transparency==STV_TRANSPARENCY_PEN)?0:-1);
					}
				}
				else
				{
					if ( stv2_current_tilemap.colour_depth == 4)
					{
						stv_vdp2_drawgfx_rgb888(bitmap,cliprect,tilecode,flipyx&1,flipyx&2,drawxpos,drawypos,stv2_current_tilemap.transparency,stv2_current_tilemap.alpha);
					}
					else if ( stv2_current_tilemap.colour_depth == 3)
					{
						stv_vdp2_drawgfx_rgb555(bitmap,cliprect,tilecode,flipyx&1,flipyx&2,drawxpos,drawypos,stv2_current_tilemap.transparency,stv2_current_tilemap.alpha);
					}
					else
					{
						if (stv2_current_tilemap.transparency == STV_TRANSPARENCY_ALPHA)
							stv_vdp2_drawgfx_alpha(bitmap,cliprect,m_gfxdecode->gfx(gfx),tilecode,pal,flipyx&1,flipyx&2, drawxpos, drawypos,0,stv2_current_tilemap.alpha);
						else
							stv_vdp2_drawgfx_transpen(bitmap,cliprect,m_gfxdecode->gfx(gfx),tilecode,pal,flipyx&1,flipyx&2, drawxpos, drawypos,(stv2_current_tilemap.transparency==STV_TRANSPARENCY_PEN)?0:-1);
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
			logerror( "Layer RBG%d, size %d x %d\n", stv2_current_tilemap.layer_name & 0x7f, cliprect.max_x + 1, cliprect.max_y + 1 );
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
		_val = m_vdp2_vram[ _address ]; \
		_val &= 0x07ffff00; \
		if ( _val & 0x04000000 ) _val |= 0xf8000000; \
	}


void saturn_state::stv_vdp2_check_tilemap_with_linescroll(bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	rectangle mycliprect;
	int cur_line = cliprect.min_y;
	int address;
	int active_functions = 0;
	INT32 scroll_values[3], prev_scroll_values[3];
	int i;
	int scroll_values_equal;
	int lines;
	INT16 main_scrollx, main_scrolly;
//  INT32 incx;
	int linescroll_enable, vertical_linescroll_enable, linezoom_enable;
	int vertical_linescroll_index = -1;

	// read original scroll values
	main_scrollx = stv2_current_tilemap.scrollx;
	main_scrolly = stv2_current_tilemap.scrolly;
//  incx = stv2_current_tilemap.incx;

	// prepare linescroll flags
	linescroll_enable = stv2_current_tilemap.linescroll_enable;
//  stv2_current_tilemap.linescroll_enable = 0;
	vertical_linescroll_enable = stv2_current_tilemap.vertical_linescroll_enable;
//  stv2_current_tilemap.vertical_linescroll_enable = 0;
	linezoom_enable = stv2_current_tilemap.linezoom_enable;
//  stv2_current_tilemap.linezoom_enable = 0;

	// prepare working clipping rectangle
	memcpy( &mycliprect, &cliprect, sizeof(rectangle) );

	// calculate the number of active functions
	if ( linescroll_enable ) active_functions++;
	if ( vertical_linescroll_enable )
	{
		vertical_linescroll_index = active_functions;
		active_functions++;
	}
	if ( linezoom_enable ) active_functions++;

	// address of data table
	address = stv2_current_tilemap.linescroll_table_address + active_functions*4*cliprect.min_y;

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
			prev_scroll_values[i] = m_vdp2_vram[ (address / 4) + i ];
		}
	}

	while( cur_line <= cliprect.max_y )
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
					scroll_values[i] = m_vdp2_vram[ (address / 4) + i ];
				}
			}

			// compare scroll values
			scroll_values_equal = 1;
			for ( i = 0; i < active_functions; i++ )
			{
				scroll_values_equal &= (scroll_values[i] == prev_scroll_values[i]);
			}
		} while( scroll_values_equal && ((cur_line + lines) <= cliprect.max_y) );

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
			stv2_current_tilemap.scrollx = main_scrollx + (prev_scroll_values[i] >> 16);
			i++;
		}
		// vertical line scroll
		if ( vertical_linescroll_enable )
		{
			stv2_current_tilemap.scrolly = main_scrolly + (prev_scroll_values[i] >> 16);
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

//      if ( LOG_VDP2 ) logerror( "Linescroll: y < %d, %d >, scrollx = %d, scrolly = %d, incx = %f\n", mycliprect.min_y, mycliprect.max_y, stv2_current_tilemap.scrollx, stv2_current_tilemap.scrolly, (float)stv2_current_tilemap.incx/65536.0 );
		// render current tilemap portion
		if (stv2_current_tilemap.bitmap_enable) // this layer is a bitmap
		{
			stv_vdp2_draw_basic_bitmap(bitmap, mycliprect);
		}
		else
		{
			//stv_vdp2_apply_window_on_layer(mycliprect);
			stv_vdp2_draw_basic_tilemap(bitmap, mycliprect);
		}

		// update parameters for next iteration
		memcpy( prev_scroll_values, scroll_values, sizeof(scroll_values));
		cur_line += lines;
	}
}

void saturn_state::stv_vdp2_draw_line(bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	int x,y;
	UINT8* gfxdata = m_vdp2.gfx_decode;
	UINT32 base_offs,base_mask;
	UINT32 pix;
	UINT8 interlace;

	interlace = (STV_VDP2_LSMD == 3)+1;

	{
		base_mask = STV_VDP2_VRAMSZ ? 0x7ffff : 0x3ffff;

		for(y=cliprect.min_y;y<=cliprect.max_y;y++)
		{
			base_offs = (STV_VDP2_LCTA & base_mask) << 1;

			if(STV_VDP2_LCCLMD)
				base_offs += (y / interlace) << 1;

			for(x=cliprect.min_x;x<=cliprect.max_x;x++)
			{
				UINT16 pen;

				pen = (gfxdata[base_offs+0]<<8)|gfxdata[base_offs+1];
				pix = bitmap.pix32(y, x);

				bitmap.pix32(y, x) = stv_add_blend(m_palette->pen(pen & 0x7ff),pix);
			}
		}
	}
}

void saturn_state::stv_vdp2_draw_mosaic(bitmap_rgb32 &bitmap, const rectangle &cliprect, UINT8 is_roz)
{
	int x,y,xi,yi;
	UINT8 h_size,v_size;
	UINT32 pix;

	h_size = STV_VDP2_MZSZH+1;
	v_size = STV_VDP2_MZSZV+1;

	if(is_roz)
		v_size = 1;

	if(h_size == 1 && v_size == 1)
		return; // don't bother

	if(STV_VDP2_LSMD == 3)
		v_size <<= 1;

	for(y=cliprect.min_y;y<=cliprect.max_y;y+=v_size)
	{
		for(x=cliprect.min_x;x<=cliprect.max_x;x+=h_size)
		{
			pix = bitmap.pix32(y, x);

			for(yi=0;yi<v_size;yi++)
				for(xi=0;xi<h_size;xi++)
					bitmap.pix32(y+yi, x+xi) = pix;
		}
	}
}

void saturn_state::stv_vdp2_check_tilemap(bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	/* the idea is here we check the tilemap capabilities / whats enabled and call an appropriate tilemap drawing routine, or
	  at the very list throw up a few errors if the tilemaps want to do something we don't support yet */
//  int window_applied = 0;
	rectangle mycliprect = cliprect;

	if ( stv2_current_tilemap.linescroll_enable ||
			stv2_current_tilemap.vertical_linescroll_enable ||
			stv2_current_tilemap.linezoom_enable )
	{
		stv_vdp2_check_tilemap_with_linescroll(bitmap, cliprect);
		return;
	}


	if (stv2_current_tilemap.bitmap_enable) // this layer is a bitmap
	{
		stv_vdp2_draw_basic_bitmap(bitmap, mycliprect);
	}
	else
	{
		//stv_vdp2_apply_window_on_layer(mycliprect);
		stv_vdp2_draw_basic_tilemap(bitmap, mycliprect);
	}

	/* post-processing functions (TODO: needs layer bitmaps to be individual planes to work correctly) */
	if(stv2_current_tilemap.line_screen_enabled && TEST_FUNCTIONS)
		stv_vdp2_draw_line(bitmap,cliprect);

	if(stv2_current_tilemap.mosaic_screen_enabled && TEST_FUNCTIONS)
		stv_vdp2_draw_mosaic(bitmap,cliprect,stv2_current_tilemap.layer_name & 0x80);


	{
		if(stv2_current_tilemap.colour_depth == 2 && !stv2_current_tilemap.bitmap_enable)
			popmessage("2048 color mode used on a non-bitmap plane");

//      if(STV_VDP2_SCXDN0 || STV_VDP2_SCXDN1 || STV_VDP2_SCYDN0 || STV_VDP2_SCYDN1)
//          popmessage("Fractional part scrolling write, contact MAMEdev");

		/* Pukunpa */
		//if(STV_VDP2_SPWINEN)
		//  popmessage("Sprite Window enabled");

		/* Capcom Collection Dai 2 - Choh Makaimura (Duh!) */
		if(STV_VDP2_MZCTL & 0x1f && POPMESSAGE_DEBUG)
			popmessage("Mosaic control enabled = %04x\n",STV_VDP2_MZCTL);

		/* Bio Hazard bit 1 */
		/* Airs Adventure 0x3e */
		/* Bakuretsu Hunter */
		if(STV_VDP2_LNCLEN & ~2 && POPMESSAGE_DEBUG)
			popmessage("Line Colour screen enabled %04x %08x, contact MAMEdev",STV_VDP2_LNCLEN,STV_VDP2_LCTAU<<16|STV_VDP2_LCTAL);

		/* Bio Hazard 0x400 = extended color calculation enabled */
		/* Advanced World War 0x200 = color calculation ratio mode */
		/* Whizz = 0x8100 */
		/* Dark Saviour = 0x9051 on save select screen (the one with a Saturn in the background) */
		if(STV_VDP2_CCCR & 0x6000)
			popmessage("Gradation enabled %04x, contact MAMEdev",STV_VDP2_CCCR);

		/* Advanced VG, Shining Force III */
		if(STV_VDP2_SFCCMD && POPMESSAGE_DEBUG)
			popmessage("Special Color Calculation enable %04x, contact MAMEdev",STV_VDP2_SFCCMD);

		/* Cleopatra Fortune Transparent Shadow */
		/* Pretty Fighter X Back & Transparent Shadow*/
		//if(STV_VDP2_SDCTL & 0x0120)
		//  popmessage("%s shadow select bit enabled, contact MAMEdev",STV_VDP2_SDCTL & 0x100 ? "Transparent" : "Back");

		/* Langrisser III bit 3 normal, bit 1 during battle field */
		/* Metal Slug bit 0 during gameplay */
		/* Bug! Sega Away Logo onward 0x470 */
		/* Command & Conquer 0x0004 0xc000 */
		if(STV_VDP2_SFSEL & ~0x47f)
			popmessage("Special Function Code Select enable %04x %04x, contact MAMEdev",STV_VDP2_SFSEL,STV_VDP2_SFCODE);

		/* Albert Odyssey Gaiden 0x0001 */
		/* Asuka 120% (doesn't make sense?) 0x0101 */
		/* Slam n Jam 96 0x0003 */
		if(STV_VDP2_ZMCTL & 0x0200)
			popmessage("Reduction enable %04x, contact MAMEdev",STV_VDP2_ZMCTL);

		/* Burning Rangers and friends FMV, J.League Pro Soccer Club Wo Tsukurou!! backgrounds */
		if(STV_VDP2_SCRCTL & 0x0101 && POPMESSAGE_DEBUG)
			popmessage("Vertical cell scroll enable %04x, contact MAMEdev",STV_VDP2_SCRCTL);

		/* Magical Drop III 0x200 -> color calculation window */
		/* Ide Yousuke Meijin No Shin Jissen Mahjong 0x0303 */
		/* Decathlete 0x088 */
		/* Sexy Parodius 0x2300 */
//      if(STV_VDP2_WCTLD & 0x2000)
//          popmessage("Special window enabled %04x, contact MAMEdev",STV_VDP2_WCTLD);

		/* Shining Force III, After Burner 2 (doesn't make a proper use tho?) */
		/* Layer Section */
		//if(STV_VDP2_W0LWE || STV_VDP2_W1LWE)
		//  popmessage("Line Window %s %08x enabled, contact MAMEdev",STV_VDP2_W0LWE ? "0" : "1",STV_VDP2_W0LWTA);

		/* Akumajou Dracula, bits 2-4 */
		/* Arcana Strikes bit 5 */
		/* Choh Makai Mura 0x0055 */
		/* Sega Rally 0x0155 */
		/* Find Love  0x4400 */
		/* Dragon Ball Z 0x3800 - 0x2c00 */
		/* Assault Suit Leynos 2 0x0200*/
		/* Bug! 0x8800 */
		/* Wonder 3 0x0018 */
		if(STV_VDP2_SFPRMD & ~0xff7f)
			popmessage("Special Priority Mode enabled %04x, contact MAMEdev",STV_VDP2_SFPRMD);
	}
}


void saturn_state::stv_vdp2_copy_roz_bitmap(bitmap_rgb32 &bitmap,
										bitmap_rgb32 &roz_bitmap,
										const rectangle &cliprect,
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
	UINT32 *line;
	rgb_t pix;
	//UINT32 coeff_line_color_screen_data;
	INT32 clipxmask = 0, clipymask = 0;


	vcnt_shift = ((STV_VDP2_LSMD & 3) == 3);
	hcnt_shift = ((STV_VDP2_HRES & 2) == 2);

	planesizex--;
	planesizey--;
	planerenderedsizex--;
	planerenderedsizey--;

	kx = RP.kx;
	ky = RP.ky;

	use_coeff_table = coeff_table_mode = coeff_table_size = coeff_table_shift = 0;
	coeff_table_offset = 0;
	coeff_table_val = 0;
	coeff_table_base = nullptr;

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
			coeff_table_base = m_vdp2_vram;
		}
		else
		{
			coeff_table_base = m_vdp2_cram;
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

	for (vcnt = cliprect.min_y; vcnt <= cliprect.max_y; vcnt++ )
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

		line = &bitmap.pix32(vcnt);

		if ( !use_coeff_table || RP.dkax == 0 )
		{
			if ( use_coeff_table )
			{
				switch( coeff_table_size )
				{
					case 0:
						address = coeff_table_offset + ((RP.kast + RP.dkast*(vcnt>>vcnt_shift)) >> 16) * 4;
						coeff_table_val = coeff_table_base[ address / 4 ];
						//coeff_line_color_screen_data = (coeff_table_val & 0x7f000000) >> 24;
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
						//coeff_line_color_screen_data = 0;
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

			for (hcnt = cliprect.min_x; hcnt <= cliprect.max_x; xs+=dxs, ys+=dys, hcnt++ )
			{
				x = xs >> 16;
				y = ys >> 16;

				if ( x & clipxmask || y & clipymask ) continue;
				pix = roz_bitmap.pix32(y & planerenderedsizey, x & planerenderedsizex);
				switch( stv2_current_tilemap.transparency )
				{
					case STV_TRANSPARENCY_PEN:
						if (pix & 0xffffff)
						{
							if(stv2_current_tilemap.fade_control & 1)
								stv_vdp2_compute_color_offset_UINT32(&pix,stv2_current_tilemap.fade_control & 2);

							line[hcnt] = pix;
						}
						break;
					case STV_TRANSPARENCY_NONE:
						if(stv2_current_tilemap.fade_control & 1)
							stv_vdp2_compute_color_offset_UINT32(&pix,stv2_current_tilemap.fade_control & 2);

						line[hcnt] = pix;
						break;
					case STV_TRANSPARENCY_ALPHA:
						if (pix & 0xffffff)
						{
							if(stv2_current_tilemap.fade_control & 1)
								stv_vdp2_compute_color_offset_UINT32(&pix,stv2_current_tilemap.fade_control & 2);

							line[hcnt] = alpha_blend_r32( line[hcnt], pix, stv2_current_tilemap.alpha );
						}
						break;
					case STV_TRANSPARENCY_ADD_BLEND:
						if (pix & 0xffffff)
						{
							if(stv2_current_tilemap.fade_control & 1)
								stv_vdp2_compute_color_offset_UINT32(&pix,stv2_current_tilemap.fade_control & 2);

							line[hcnt] = stv_add_blend( line[hcnt], pix );
						}
						break;
				}

			}
		}
		else
		{
			for (hcnt = cliprect.min_x; hcnt <= cliprect.max_x; hcnt++ )
			{
				switch( coeff_table_size )
				{
					case 0:
						address = coeff_table_offset + ((RP.kast + RP.dkast*(vcnt>>vcnt_shift) + RP.dkax*hcnt) >> 16) * 4;
						coeff_table_val = coeff_table_base[ address / 4 ];
						//coeff_line_color_screen_data = (coeff_table_val & 0x7f000000) >> 24;
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
						//coeff_line_color_screen_data = 0;
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

				pix = roz_bitmap.pix32(y & planerenderedsizey, x & planerenderedsizex);
				switch( stv2_current_tilemap.transparency )
				{
					case STV_TRANSPARENCY_PEN:
						if (pix & 0xffffff)
						{
							if(stv2_current_tilemap.fade_control & 1)
								stv_vdp2_compute_color_offset_UINT32(&pix,stv2_current_tilemap.fade_control & 2);

							line[hcnt] = pix;
						}
						break;
					case STV_TRANSPARENCY_NONE:
						if(stv2_current_tilemap.fade_control & 1)
							stv_vdp2_compute_color_offset_UINT32(&pix,stv2_current_tilemap.fade_control & 2);

						line[hcnt] = pix;
						break;
					case STV_TRANSPARENCY_ALPHA:
						if (pix & 0xffffff)
						{
							if(stv2_current_tilemap.fade_control & 1)
								stv_vdp2_compute_color_offset_UINT32(&pix,stv2_current_tilemap.fade_control & 2);

							line[hcnt] = alpha_blend_r32( line[hcnt], pix, stv2_current_tilemap.alpha );
						}
						break;
					case STV_TRANSPARENCY_ADD_BLEND:
						if (pix & 0xffffff)
						{
							if(stv2_current_tilemap.fade_control & 1)
								stv_vdp2_compute_color_offset_UINT32(&pix,stv2_current_tilemap.fade_control & 2);

							line[hcnt] = stv_add_blend( line[hcnt], pix );
						}
						break;
				}
			}
		}
	}
}

void saturn_state::stv_vdp2_draw_NBG0(bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	UINT32 base_mask;

	base_mask = STV_VDP2_VRAMSZ ? 0x7ffff : 0x3ffff;

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

	stv2_current_tilemap.enabled = STV_VDP2_N0ON | STV_VDP2_R1ON;

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
	stv2_current_tilemap.linescroll_table_address = (((STV_VDP2_LSTA0U << 16) | STV_VDP2_LSTA0L) & base_mask) * 2;
	stv2_current_tilemap.vertical_linescroll_enable = STV_VDP2_N0LSCY;
	stv2_current_tilemap.linezoom_enable = STV_VDP2_N0LZMX;

	stv2_current_tilemap.plane_size = (STV_VDP2_R1ON) ? STV_VDP2_RBPLSZ : STV_VDP2_N0PLSZ;
	stv2_current_tilemap.colour_ram_address_offset = STV_VDP2_N0CAOS;
	stv2_current_tilemap.fade_control = (STV_VDP2_N0COEN * 1) | (STV_VDP2_N0COSL * 2);
	stv_vdp2_check_fade_control_for_layer();
	stv2_current_tilemap.window_control.logic = STV_VDP2_N0LOG;
	stv2_current_tilemap.window_control.enabled[0] = STV_VDP2_N0W0E;
	stv2_current_tilemap.window_control.enabled[1] = STV_VDP2_N0W1E;
//  stv2_current_tilemap.window_control.? = STV_VDP2_N0SWE;
	stv2_current_tilemap.window_control.area[0] = STV_VDP2_N0W0A;
	stv2_current_tilemap.window_control.area[1] = STV_VDP2_N0W1A;
//  stv2_current_tilemap.window_control.? = STV_VDP2_N0SWA;

	stv2_current_tilemap.line_screen_enabled = STV_VDP2_N0LCEN;
	stv2_current_tilemap.mosaic_screen_enabled = STV_VDP2_N0MZE;

	stv2_current_tilemap.layer_name=(STV_VDP2_R1ON) ? 0x81 : 0;

	if ( stv2_current_tilemap.enabled && (!(STV_VDP2_R1ON))) /* TODO: check cycle pattern for RBG1 */
	{
		stv2_current_tilemap.enabled = stv_vdp2_check_vram_cycle_pattern_registers( STV_VDP2_CP_NBG0_PNMDR, STV_VDP2_CP_NBG0_CPDR, stv2_current_tilemap.bitmap_enable );
	}

	if(STV_VDP2_R1ON)
		stv_vdp2_draw_rotation_screen(bitmap, cliprect, 2 );
	else
		stv_vdp2_check_tilemap(bitmap, cliprect);
}

void saturn_state::stv_vdp2_draw_NBG1(bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	UINT32 base_mask;

	base_mask = STV_VDP2_VRAMSZ ? 0x7ffff : 0x3ffff;

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
	stv2_current_tilemap.linescroll_table_address = (((STV_VDP2_LSTA1U << 16) | STV_VDP2_LSTA1L) & base_mask) * 2;
	stv2_current_tilemap.vertical_linescroll_enable = STV_VDP2_N1LSCY;
	stv2_current_tilemap.linezoom_enable = STV_VDP2_N1LZMX;

	stv2_current_tilemap.plane_size = STV_VDP2_N1PLSZ;
	stv2_current_tilemap.colour_ram_address_offset = STV_VDP2_N1CAOS;
	stv2_current_tilemap.fade_control = (STV_VDP2_N1COEN * 1) | (STV_VDP2_N1COSL * 2);
	stv_vdp2_check_fade_control_for_layer();
	stv2_current_tilemap.window_control.logic = STV_VDP2_N1LOG;
	stv2_current_tilemap.window_control.enabled[0] = STV_VDP2_N1W0E;
	stv2_current_tilemap.window_control.enabled[1] = STV_VDP2_N1W1E;
//  stv2_current_tilemap.window_control.? = STV_VDP2_N1SWE;
	stv2_current_tilemap.window_control.area[0] = STV_VDP2_N1W0A;
	stv2_current_tilemap.window_control.area[1] = STV_VDP2_N1W1A;
//  stv2_current_tilemap.window_control.? = STV_VDP2_N1SWA;

	stv2_current_tilemap.line_screen_enabled = STV_VDP2_N1LCEN;
	stv2_current_tilemap.mosaic_screen_enabled = STV_VDP2_N1MZE;

	stv2_current_tilemap.layer_name=1;

	if ( stv2_current_tilemap.enabled )
	{
		stv2_current_tilemap.enabled = stv_vdp2_check_vram_cycle_pattern_registers( STV_VDP2_CP_NBG1_PNMDR, STV_VDP2_CP_NBG1_CPDR, stv2_current_tilemap.bitmap_enable );
	}

	stv_vdp2_check_tilemap(bitmap, cliprect);
}

void saturn_state::stv_vdp2_draw_NBG2(bitmap_rgb32 &bitmap, const rectangle &cliprect)
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
	stv2_current_tilemap.window_control.logic = STV_VDP2_N2LOG;
	stv2_current_tilemap.window_control.enabled[0] = STV_VDP2_N2W0E;
	stv2_current_tilemap.window_control.enabled[1] = STV_VDP2_N2W1E;
//  stv2_current_tilemap.window_control.? = STV_VDP2_N2SWE;
	stv2_current_tilemap.window_control.area[0] = STV_VDP2_N2W0A;
	stv2_current_tilemap.window_control.area[1] = STV_VDP2_N2W1A;
//  stv2_current_tilemap.window_control.? = STV_VDP2_N2SWA;

	stv2_current_tilemap.line_screen_enabled = STV_VDP2_N2LCEN;
	stv2_current_tilemap.mosaic_screen_enabled = STV_VDP2_N2MZE;

	stv2_current_tilemap.layer_name=2;

	stv2_current_tilemap.plane_size = STV_VDP2_N2PLSZ;

	if ( stv2_current_tilemap.enabled )
	{
		stv2_current_tilemap.enabled = stv_vdp2_check_vram_cycle_pattern_registers( STV_VDP2_CP_NBG2_PNMDR, STV_VDP2_CP_NBG2_CPDR, stv2_current_tilemap.bitmap_enable );
	}

	stv_vdp2_check_tilemap(bitmap, cliprect);
}

void saturn_state::stv_vdp2_draw_NBG3(bitmap_rgb32 &bitmap, const rectangle &cliprect)
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
	stv2_current_tilemap.window_control.logic = STV_VDP2_N3LOG;
	stv2_current_tilemap.window_control.enabled[0] = STV_VDP2_N3W0E;
	stv2_current_tilemap.window_control.enabled[1] = STV_VDP2_N3W1E;
//  stv2_current_tilemap.window_control.? = STV_VDP2_N3SWE;
	stv2_current_tilemap.window_control.area[0] = STV_VDP2_N3W0A;
	stv2_current_tilemap.window_control.area[1] = STV_VDP2_N3W1A;
//  stv2_current_tilemap.window_control.? = STV_VDP2_N3SWA;

	stv2_current_tilemap.line_screen_enabled = STV_VDP2_N3LCEN;
	stv2_current_tilemap.mosaic_screen_enabled = STV_VDP2_N3MZE;

	stv2_current_tilemap.layer_name=3;

	stv2_current_tilemap.plane_size = STV_VDP2_N3PLSZ;

	if ( stv2_current_tilemap.enabled )
	{
		stv2_current_tilemap.enabled = stv_vdp2_check_vram_cycle_pattern_registers( STV_VDP2_CP_NBG3_PNMDR, STV_VDP2_CP_NBG3_CPDR, stv2_current_tilemap.bitmap_enable );
	}

	stv_vdp2_check_tilemap(bitmap, cliprect);
}


void saturn_state::stv_vdp2_draw_rotation_screen(bitmap_rgb32 &bitmap, const rectangle &cliprect, int iRP)
{
	rectangle roz_clip_rect, mycliprect;
	int planesizex = 0, planesizey = 0;
	int planerenderedsizex, planerenderedsizey;
	UINT8 colour_calculation_enabled;
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

	stv_vdp2_fill_rotation_parameter_table(iRP);

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

		stv_vdp2_check_tilemap(bitmap,cliprect);
	}
	else
	{
		if ( !m_vdp2.roz_bitmap[iRP-1].valid() )
			m_vdp2.roz_bitmap[iRP-1].allocate(4096, 4096);

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
//      window_control = stv2_current_tilemap.window_control;
//      stv2_current_tilemap.window_control = 0;
		fade_control = stv2_current_tilemap.fade_control;
		stv2_current_tilemap.fade_control = 0;
		g_profiler.start(PROFILER_USER1);
		if ( LOG_VDP2 ) logerror( "Checking for cached RBG bitmap, cache_dirty = %d, memcmp() = %d\n", stv_rbg_cache_data.is_cache_dirty, memcmp(&stv_rbg_cache_data.layer_data[iRP-1],&stv2_current_tilemap,sizeof(stv2_current_tilemap)));
		if ( (stv_rbg_cache_data.is_cache_dirty & iRP) ||
			memcmp(&stv_rbg_cache_data.layer_data[iRP-1],&stv2_current_tilemap,sizeof(stv2_current_tilemap)) != 0 )
		{
			m_vdp2.roz_bitmap[iRP-1].fill(m_palette->black_pen(), roz_clip_rect );
			stv_vdp2_check_tilemap(m_vdp2.roz_bitmap[iRP-1], roz_clip_rect);
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

		g_profiler.stop();

		stv2_current_tilemap.colour_calculation_enabled = colour_calculation_enabled;
		if ( colour_calculation_enabled )
		{
			stv2_current_tilemap.transparency = STV_TRANSPARENCY_ALPHA;
		}

		mycliprect = cliprect;

		/* TODO: remove me. */
		if ( stv2_current_tilemap.window_control.enabled[0] || stv2_current_tilemap.window_control.enabled[1] )
		{
			//popmessage("Window control for RBG");
			stv_vdp2_apply_window_on_layer(mycliprect);
			stv2_current_tilemap.window_control.enabled[0] = 0;
			stv2_current_tilemap.window_control.enabled[1] = 0;
		}

		stv2_current_tilemap.fade_control = fade_control;

		g_profiler.start(PROFILER_USER2);
		stv_vdp2_copy_roz_bitmap(bitmap, m_vdp2.roz_bitmap[iRP-1], mycliprect, iRP, planesizex, planesizey, planerenderedsizex, planerenderedsizey );
		g_profiler.stop();
	}

}

void saturn_state::stv_vdp2_draw_RBG0(bitmap_rgb32 &bitmap, const rectangle &cliprect)
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
	stv2_current_tilemap.window_control.logic = STV_VDP2_R0LOG;
	stv2_current_tilemap.window_control.enabled[0] = STV_VDP2_R0W0E;
	stv2_current_tilemap.window_control.enabled[1] = STV_VDP2_R0W1E;
//  stv2_current_tilemap.window_control.? = STV_VDP2_R0SWE;
	stv2_current_tilemap.window_control.area[0] = STV_VDP2_R0W0A;
	stv2_current_tilemap.window_control.area[1] = STV_VDP2_R0W1A;
//  stv2_current_tilemap.window_control.? = STV_VDP2_R0SWA;

	stv2_current_tilemap.scrollx = 0;
	stv2_current_tilemap.scrolly = 0;
	stv2_current_tilemap.incx = 0x10000;
	stv2_current_tilemap.incy = 0x10000;

	stv2_current_tilemap.linescroll_enable = 0;
	stv2_current_tilemap.linescroll_interval = 0;
	stv2_current_tilemap.linescroll_table_address = 0;
	stv2_current_tilemap.vertical_linescroll_enable = 0;
	stv2_current_tilemap.linezoom_enable = 0;

	stv2_current_tilemap.line_screen_enabled = STV_VDP2_R0LCEN;
	stv2_current_tilemap.mosaic_screen_enabled = STV_VDP2_R0MZE;

	/*Use 0x80 as a normal/rotate switch*/
	stv2_current_tilemap.layer_name=0x80;

	if ( !stv2_current_tilemap.enabled ) return;

	switch(STV_VDP2_RPMD)
	{
		case 0://Rotation Parameter A
			stv_vdp2_draw_rotation_screen(bitmap, cliprect, 1 );
			break;
		case 1://Rotation Parameter B
		//case 2:
			stv_vdp2_draw_rotation_screen(bitmap, cliprect, 2 );
			break;
		case 2://Rotation Parameter A & B CKTE
			stv_vdp2_draw_rotation_screen(bitmap, cliprect, 2 );
			stv_vdp2_draw_rotation_screen(bitmap, cliprect, 1 );
			break;
		case 3://Rotation Parameter A & B Window (wrong)
			stv_vdp2_draw_rotation_screen(bitmap, cliprect, 1 );
			break;
	}

}

void saturn_state::stv_vdp2_draw_back(bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	int x,y;
	UINT8* gfxdata = m_vdp2.gfx_decode;
	UINT32 base_offs,base_mask;
	UINT8 interlace;

	interlace = (STV_VDP2_LSMD == 3)+1;

//  popmessage("Back screen %08x %08x %08x",STV_VDP2_BDCLMD,STV_VDP2_BKCLMD,STV_VDP2_BKTA);

	/* draw black if BDCLMD and DISP are cleared */
	if(!(STV_VDP2_BDCLMD) && !(STV_VDP2_DISP))
		bitmap.fill(m_palette->black_pen(), cliprect);
	else
	{
		base_mask = STV_VDP2_VRAMSZ ? 0x7ffff : 0x3ffff;

		for(y=cliprect.min_y;y<=cliprect.max_y;y++)
		{
			base_offs = ((STV_VDP2_BKTA ) & base_mask) << 1;
			if(STV_VDP2_BKCLMD)
				base_offs += ((y / interlace) << 1);

			for(x=cliprect.min_x;x<=cliprect.max_x;x++)
			{
				int r,g,b;
				UINT16 dot;

				dot = (gfxdata[base_offs+0]<<8)|gfxdata[base_offs+1];
				b = pal5bit((dot & 0x7c00) >> 10);
				g = pal5bit((dot & 0x03e0) >> 5);
				r = pal5bit( dot & 0x001f);
				if(STV_VDP2_BKCOEN)
					stv_vdp2_compute_color_offset( &r, &g, &b, STV_VDP2_BKCOSL );

				bitmap.pix32(y, x) = rgb_t(r, g, b);
			}
		}
	}
}

READ32_MEMBER ( saturn_state::saturn_vdp2_vram_r )
{
	return m_vdp2_vram[offset];
}

WRITE32_MEMBER ( saturn_state::saturn_vdp2_vram_w )
{
	UINT8* gfxdata = m_vdp2.gfx_decode;

	COMBINE_DATA(&m_vdp2_vram[offset]);

	data = m_vdp2_vram[offset];
	/* put in gfx region for easy decoding */
	gfxdata[offset*4+0] = (data & 0xff000000) >> 24;
	gfxdata[offset*4+1] = (data & 0x00ff0000) >> 16;
	gfxdata[offset*4+2] = (data & 0x0000ff00) >> 8;
	gfxdata[offset*4+3] = (data & 0x000000ff) >> 0;

	m_gfxdecode->gfx(0)->mark_dirty(offset/8);
	m_gfxdecode->gfx(1)->mark_dirty(offset/8);
	m_gfxdecode->gfx(2)->mark_dirty(offset/8);
	m_gfxdecode->gfx(3)->mark_dirty(offset/8);

	/* 8-bit tiles overlap, so this affects the previous one as well */
	if (offset/8 != 0)
	{
		m_gfxdecode->gfx(2)->mark_dirty(offset/8 - 1);
		m_gfxdecode->gfx(3)->mark_dirty(offset/8 - 1);
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

READ16_MEMBER ( saturn_state::saturn_vdp2_regs_r )
{
	switch(offset)
	{
		case 0x002/2:
		{
			/* latch h/v signals through HV latch*/
			if(!STV_VDP2_EXLTEN)
			{
				if(!space.debugger_access())
				{
					m_vdp2.h_count = get_hcounter();
					m_vdp2.v_count = get_vcounter();
					/* latch flag */
					m_vdp2.exltfg |= 1;
				}
			}

			break;
		}
		case 0x004/2:
		{
			/*Screen Status Register*/
										/*VBLANK              HBLANK            ODD               PAL    */
			m_vdp2_regs[offset] = (m_vdp2.exltfg<<9) |
											(m_vdp2.exsyfg<<8) |
											(get_vblank() << 3) |
											(get_hblank() << 2) |
											(get_odd_bit() << 1) |
											(m_vdp2.pal << 0);

			/* vblank bit is always 1 if DISP bit is disabled */
			if(!STV_VDP2_DISP)
				m_vdp2_regs[offset] |= 1 << 3;

			/* HV latches clears if this register is read */
			if(!space.debugger_access())
			{
				m_vdp2.exltfg &= ~1;
				m_vdp2.exsyfg &= ~1;
			}
			break;
		}
		case 0x006/2:
		{
			m_vdp2_regs[offset] = (STV_VDP2_VRAMSZ << 15) |
											((0 << 0) & 0xf); // VDP2 version

			/* Games basically r/w the entire VDP2 register area when this is tripped. (example: Silhouette Mirage)
			   Disable log for the time being. */
			//if(!space.debugger_access())
			//  printf("Warning: VDP2 version read\n");
			break;
		}

		/* HCNT */
		case 0x008/2:
		{
			m_vdp2_regs[offset] = (m_vdp2.h_count);
			break;
		}

		/* VCNT */
		case 0x00a/2:
		{
			m_vdp2_regs[offset] = (m_vdp2.v_count);
			break;
		}

		default:
			//if(!space.debugger_access())
			//  printf("VDP2: read from register %08x %08x\n",offset*4,mem_mask);
			break;
	}

	return m_vdp2_regs[offset];
}

READ32_MEMBER ( saturn_state::saturn_vdp2_cram_r )
{
	offset &= (0xfff) >> (2);
	return m_vdp2_cram[offset];
}




WRITE32_MEMBER ( saturn_state::saturn_vdp2_cram_w )
{
	int r,g,b;
	UINT8 cmode0;

	cmode0 = (STV_VDP2_CRMD & 3) == 0;

	offset &= (0xfff) >> (2);
	COMBINE_DATA(&m_vdp2_cram[offset]);

	switch( STV_VDP2_CRMD )
	{
		/*Mode 2/3*/
		case 2:
		case 3:
		{
			//offset &= (0xfff) >> 2;

			b = ((m_vdp2_cram[offset] & 0x00ff0000) >> 16);
			g = ((m_vdp2_cram[offset] & 0x0000ff00) >> 8);
			r = ((m_vdp2_cram[offset] & 0x000000ff) >> 0);
			m_palette->set_pen_color(offset,rgb_t(r,g,b));
			m_palette->set_pen_color(offset^0x400,rgb_t(r,g,b));
		}
		break;
		/*Mode 0*/
		case 0:
		case 1:
		{
			offset &= (0xfff) >> (cmode0+2);

			b = ((m_vdp2_cram[offset] & 0x00007c00) >> 10);
			g = ((m_vdp2_cram[offset] & 0x000003e0) >> 5);
			r = ((m_vdp2_cram[offset] & 0x0000001f) >> 0);
			m_palette->set_pen_color((offset*2)+1,pal5bit(r),pal5bit(g),pal5bit(b));
			if(cmode0)
				m_palette->set_pen_color(((offset*2)+1)^0x400,pal5bit(r),pal5bit(g),pal5bit(b));

			b = ((m_vdp2_cram[offset] & 0x7c000000) >> 26);
			g = ((m_vdp2_cram[offset] & 0x03e00000) >> 21);
			r = ((m_vdp2_cram[offset] & 0x001f0000) >> 16);
			m_palette->set_pen_color(offset*2,pal5bit(r),pal5bit(g),pal5bit(b));
			if(cmode0)
				m_palette->set_pen_color((offset*2)^0x400,pal5bit(r),pal5bit(g),pal5bit(b));
		}
		break;
	}
}

void saturn_state::refresh_palette_data( void )
{
	int r,g,b;
	int c_i;
	UINT8 bank;

	switch( STV_VDP2_CRMD )
	{
		case 2:
		case 3:
		{
			for(c_i=0;c_i<0x400;c_i++)
			{
				b = ((m_vdp2_cram[c_i] & 0x00ff0000) >> 16);
				g = ((m_vdp2_cram[c_i] & 0x0000ff00) >> 8);
				r = ((m_vdp2_cram[c_i] & 0x000000ff) >> 0);
				m_palette->set_pen_color(c_i,rgb_t(r,g,b));
				m_palette->set_pen_color(c_i+0x400,rgb_t(r,g,b));
			}
		}
		break;
		case 0:
		{
			for(bank=0;bank<2;bank++)
			{
				for(c_i=0;c_i<0x400;c_i++)
				{
					b = ((m_vdp2_cram[c_i] & 0x00007c00) >> 10);
					g = ((m_vdp2_cram[c_i] & 0x000003e0) >> 5);
					r = ((m_vdp2_cram[c_i] & 0x0000001f) >> 0);
					m_palette->set_pen_color((c_i*2)+1+bank*0x400,pal5bit(r),pal5bit(g),pal5bit(b));
					b = ((m_vdp2_cram[c_i] & 0x7c000000) >> 26);
					g = ((m_vdp2_cram[c_i] & 0x03e00000) >> 21);
					r = ((m_vdp2_cram[c_i] & 0x001f0000) >> 16);
					m_palette->set_pen_color(c_i*2+bank*0x400,pal5bit(r),pal5bit(g),pal5bit(b));
				}
			}
		}
		break;
		case 1:
		{
			for(c_i=0;c_i<0x800;c_i++)
			{
				b = ((m_vdp2_cram[c_i] & 0x00007c00) >> 10);
				g = ((m_vdp2_cram[c_i] & 0x000003e0) >> 5);
				r = ((m_vdp2_cram[c_i] & 0x0000001f) >> 0);
				m_palette->set_pen_color((c_i*2)+1,pal5bit(r),pal5bit(g),pal5bit(b));
				b = ((m_vdp2_cram[c_i] & 0x7c000000) >> 26);
				g = ((m_vdp2_cram[c_i] & 0x03e00000) >> 21);
				r = ((m_vdp2_cram[c_i] & 0x001f0000) >> 16);
				m_palette->set_pen_color(c_i*2,pal5bit(r),pal5bit(g),pal5bit(b));
			}
		}
		break;
	}
}

WRITE16_MEMBER ( saturn_state::saturn_vdp2_regs_w )
{
	COMBINE_DATA(&m_vdp2_regs[offset]);

	if(m_vdp2.old_crmd != STV_VDP2_CRMD)
	{
		m_vdp2.old_crmd = STV_VDP2_CRMD;
		refresh_palette_data();
	}
	if(m_vdp2.old_tvmd != STV_VDP2_TVMD)
	{
		m_vdp2.old_tvmd = STV_VDP2_TVMD;
		stv_vdp2_dynamic_res_change();
	}

	if(STV_VDP2_VRAMSZ)
		printf("VDP2 sets up 8 Mbit VRAM!\n");
}

int saturn_state::get_hblank_duration( void )
{
	int res;

	res = (STV_VDP2_HRES & 1) ? 455 : 427;

	/* double pump horizontal max res */
	if(STV_VDP2_HRES & 2)
		res<<=1;

	return res;
}

/*some vblank lines measurements (according to Charles MacDonald)*/
/* TODO: interlace mode "eats" one line, should be 262.5 */
int saturn_state::get_vblank_duration( void )
{
	int res;

	res = (m_vdp2.pal) ? 313 : 263;

	/* compensate for interlacing */
	if((STV_VDP2_LSMD & 3) == 3)
		res<<=1;

	if(STV_VDP2_HRES & 4)
		res = (STV_VDP2_HRES & 1) ? 561 : 525;  //Hi-Vision / 31kHz Monitor

	return res;
}

int saturn_state::get_pixel_clock( void )
{
	int res,divider;

	res = m_vdp2.dotsel ? MASTER_CLOCK_352 : MASTER_CLOCK_320;
	/* TODO: divider is ALWAYS 8, this thing is just to over-compensate for MAME framework faults ... */
	divider = 8;

	if(STV_VDP2_HRES & 2)
		divider>>=1;

	if((STV_VDP2_LSMD & 3) == 3)
		divider>>=1;

	if(STV_VDP2_HRES & 4) //TODO
		divider>>=1;

	return res/divider;
}

/* TODO: hblank position and hblank firing doesn't really match HW behaviour. */
UINT8 saturn_state::get_hblank( void )
{
	const rectangle &visarea = machine().first_screen()->visible_area();
	int cur_h = machine().first_screen()->hpos();

	if (cur_h > visarea.max_x) //TODO
		return 1;

	return 0;
}

UINT8 saturn_state::get_vblank( void )
{
	int cur_v,vblank;
	cur_v = machine().first_screen()->vpos();

	vblank = get_vblank_start_position() * get_ystep_count();

	if (cur_v >= vblank)
		return 1;

	return 0;
}

UINT8 saturn_state::get_odd_bit( void )
{
	if(STV_VDP2_HRES & 4) //exclusive monitor mode makes this bit to be always 1
		return 1;

	if(STV_VDP2_LSMD == 0) // same for non-interlace mode
		return 1;

	return machine().first_screen()->frame_number() & 1;
}

int saturn_state::get_vblank_start_position( void )
{
	/* TODO: test says that second setting happens at 241, might need further investigation ... */
	const int d_vres[4] = { 224, 240, 256, 256 };
	int vres_mask;
	int vblank_line;

	vres_mask = (m_vdp2.pal << 1)|1; //PAL uses mask 3, NTSC uses mask 1
	vblank_line = d_vres[STV_VDP2_VRES & vres_mask];

	return vblank_line;
}

int saturn_state::get_ystep_count( void )
{
	int max_y = machine().first_screen()->height();
	int y_step;

	y_step = 2;

	if((max_y == 263 && m_vdp2.pal == 0) || (max_y == 313 && m_vdp2.pal == 1))
		y_step = 1;

	return y_step;
}

/* TODO: these needs to be checked via HW tests! */
int saturn_state::get_hcounter( void )
{
	int hcount;

	hcount = machine().first_screen()->hpos();

	switch(STV_VDP2_HRES & 6)
	{
		/* Normal */
		case 0:
			hcount &= 0x1ff;
			hcount <<= 1;
			break;
		/* Hi-Res */
		case 2:
			hcount &= 0x3ff;
			break;
		/* Exclusive Normal*/
		case 4:
			hcount &= 0x1ff;
			break;
		/* Exclusive Hi-Res */
		case 6:
			hcount >>= 1;
			hcount &= 0x1ff;
			break;
	}

	return hcount;
}

int saturn_state::get_vcounter( void )
{
	int vcount;

	vcount = machine().first_screen()->vpos();

	/* Exclusive Monitor */
	if(STV_VDP2_HRES & 4)
		return vcount & 0x3ff;

	/* Double Density Interlace */
	if((STV_VDP2_LSMD & 3) == 3)
		return (vcount & ~1) | (machine().first_screen()->frame_number() & 1);

	/* docs says << 1, but according to HW tests it's a typo. */
	assert((vcount & 0x1ff) < ARRAY_LENGTH(true_vcount));
	return (true_vcount[vcount & 0x1ff][STV_VDP2_VRES]); // Non-interlace
}

void saturn_state::stv_vdp2_state_save_postload( void )
{
	UINT8 *gfxdata = m_vdp2.gfx_decode;
	int offset;
	UINT32 data;

	for ( offset = 0; offset < 0x100000/4; offset++ )
	{
		data = m_vdp2_vram[offset];
		/* put in gfx region for easy decoding */
		gfxdata[offset*4+0] = (data & 0xff000000) >> 24;
		gfxdata[offset*4+1] = (data & 0x00ff0000) >> 16;
		gfxdata[offset*4+2] = (data & 0x0000ff00) >> 8;
		gfxdata[offset*4+3] = (data & 0x000000ff) >> 0;

		m_gfxdecode->gfx(0)->mark_dirty(offset/8);
		m_gfxdecode->gfx(1)->mark_dirty(offset/8);
		m_gfxdecode->gfx(2)->mark_dirty(offset/8);
		m_gfxdecode->gfx(3)->mark_dirty(offset/8);

		/* 8-bit tiles overlap, so this affects the previous one as well */
		if (offset/8 != 0)
		{
			m_gfxdecode->gfx(2)->mark_dirty(offset/8 - 1);
			m_gfxdecode->gfx(3)->mark_dirty(offset/8 - 1);
		}

	}

	memset( &stv_rbg_cache_data, 0, sizeof(stv_rbg_cache_data));
	stv_rbg_cache_data.is_cache_dirty = 3;
	memset( &stv_vdp2_layer_data_placement, 0, sizeof(stv_vdp2_layer_data_placement));

	refresh_palette_data();
}

void saturn_state::stv_vdp2_exit ( void )
{
	m_vdp2.roz_bitmap[0].reset();
	m_vdp2.roz_bitmap[1].reset();
}

int saturn_state::stv_vdp2_start ( void )
{
	machine().add_notifier(MACHINE_NOTIFY_EXIT, machine_notify_delegate(FUNC(saturn_state::stv_vdp2_exit), this));

	m_vdp2_regs = auto_alloc_array_clear(machine(), UINT16, 0x040000/2 );
	m_vdp2_vram = auto_alloc_array_clear(machine(), UINT32, 0x100000/4 );
	m_vdp2_cram = auto_alloc_array_clear(machine(), UINT32, 0x080000/4 );
	m_vdp2.gfx_decode = auto_alloc_array(machine(), UINT8, 0x100000 );

//  m_gfxdecode->gfx(0)->granularity()=4;
//  m_gfxdecode->gfx(1)->granularity()=4;

	memset( &stv_rbg_cache_data, 0, sizeof(stv_rbg_cache_data));
	stv_rbg_cache_data.is_cache_dirty = 3;
	memset( &stv_vdp2_layer_data_placement, 0, sizeof(stv_vdp2_layer_data_placement));

	save_pointer(NAME(m_vdp2_regs), 0x040000/2);
	save_pointer(NAME(m_vdp2_vram), 0x100000/4);
	save_pointer(NAME(m_vdp2_cram), 0x080000/4);
	machine().save().register_postload(save_prepost_delegate(FUNC(saturn_state::stv_vdp2_state_save_postload), this));

	return 0;
}

/* maybe we should move this to video/stv.c */
VIDEO_START_MEMBER(saturn_state,stv_vdp2)
{
	int i;
	machine().first_screen()->register_screen_bitmap(m_tmpbitmap);
	stv_vdp2_start();
	stv_vdp1_start();
	m_vdpdebug_roz = 0;
	m_gfxdecode->gfx(0)->set_source(m_vdp2.gfx_decode);
	m_gfxdecode->gfx(1)->set_source(m_vdp2.gfx_decode);
	m_gfxdecode->gfx(2)->set_source(m_vdp2.gfx_decode);
	m_gfxdecode->gfx(3)->set_source(m_vdp2.gfx_decode);

	/* calc V counter offsets */
	/* 224 mode */
	for(i=0;i<263;i++)
	{
		true_vcount[i][0] = i;
		if(i>0xec)
			true_vcount[i][0]+=0xf9;
	}

	for(i=0;i<263;i++)
	{
		true_vcount[i][1] = i;
		if(i>0xf5)
			true_vcount[i][1]+=0xf9;
	}

	/* 256 mode, todo */
	for(i=0;i<263;i++)
	{
		true_vcount[i][2] = i;
		true_vcount[i][3] = i;
	}
}

void saturn_state::stv_vdp2_dynamic_res_change( void )
{
	const int d_vres[4] = { 224, 240, 256, 256 };
	const int d_hres[4] = { 320, 352, 640, 704 };
	int horz_res,vert_res;
	int vres_mask;

	vres_mask = (m_vdp2.pal << 1)|1; //PAL uses mask 3, NTSC uses mask 1
	vert_res = d_vres[STV_VDP2_VRES & vres_mask];

	if((STV_VDP2_VRES & 3) == 3)
		popmessage("Illegal VRES MODE, contact MAMEdev");

	/*Double-density interlace mode,doubles the vertical res*/
	if((STV_VDP2_LSMD & 3) == 3) { vert_res*=2;  }

	horz_res = d_hres[STV_VDP2_HRES & 3];
	/*Exclusive modes,they sets the Vertical Resolution without considering the
	  VRES register.*/
	if(STV_VDP2_HRES & 4)
		vert_res = 480;

	{
		int vblank_period,hblank_period;
		attoseconds_t refresh;
		rectangle visarea(0, horz_res-1, 0, vert_res-1);

		vblank_period = get_vblank_duration();
		hblank_period = get_hblank_duration();
		refresh  = HZ_TO_ATTOSECONDS(get_pixel_clock()) * (hblank_period) * vblank_period;
		//printf("%d %d %d %d\n",horz_res,vert_res,horz_res+hblank_period,vblank_period);

		machine().first_screen()->configure(hblank_period, vblank_period, visarea, refresh );
	}
//  machine().first_screen()->set_visible_area(0*8, horz_res-1,0*8, vert_res-1);
}

/*This is for calculating the rgb brightness*/
/*TODO: Optimize this...*/
void saturn_state::stv_vdp2_fade_effects( void )
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
		color = m_palette->pen_color(i);
		t_r = (STV_VDP2_COAR & 0x100) ? (color.r() - (0x100 - (STV_VDP2_COAR & 0xff))) : ((STV_VDP2_COAR & 0xff) + color.r());
		t_g = (STV_VDP2_COAG & 0x100) ? (color.g() - (0x100 - (STV_VDP2_COAG & 0xff))) : ((STV_VDP2_COAG & 0xff) + color.g());
		t_b = (STV_VDP2_COAB & 0x100) ? (color.b() - (0x100 - (STV_VDP2_COAB & 0xff))) : ((STV_VDP2_COAB & 0xff) + color.b());
		if(t_r < 0)     { t_r = 0; }
		if(t_r > 0xff)  { t_r = 0xff; }
		if(t_g < 0)     { t_g = 0; }
		if(t_g > 0xff)  { t_g = 0xff; }
		if(t_b < 0)     { t_b = 0; }
		if(t_b > 0xff)  { t_b = 0xff; }
		r = t_r;
		g = t_g;
		b = t_b;
		m_palette->set_pen_color(i+(2048*1),rgb_t(r,g,b));

		/*Fade B*/
		color = m_palette->pen_color(i);
		t_r = (STV_VDP2_COBR & 0x100) ? (color.r() - (0xff - (STV_VDP2_COBR & 0xff))) : ((STV_VDP2_COBR & 0xff) + color.r());
		t_g = (STV_VDP2_COBG & 0x100) ? (color.g() - (0xff - (STV_VDP2_COBG & 0xff))) : ((STV_VDP2_COBG & 0xff) + color.g());
		t_b = (STV_VDP2_COBB & 0x100) ? (color.b() - (0xff - (STV_VDP2_COBB & 0xff))) : ((STV_VDP2_COBB & 0xff) + color.b());
		if(t_r < 0)     { t_r = 0; }
		if(t_r > 0xff)  { t_r = 0xff; }
		if(t_g < 0)     { t_g = 0; }
		if(t_g > 0xff)  { t_g = 0xff; }
		if(t_b < 0)     { t_b = 0; }
		if(t_b > 0xff)  { t_b = 0xff; }
		r = t_r;
		g = t_g;
		b = t_b;
		m_palette->set_pen_color(i+(2048*2),rgb_t(r,g,b));
	}
	//popmessage("%04x %04x %04x %04x %04x %04x",STV_VDP2_COAR,STV_VDP2_COAG,STV_VDP2_COAB,STV_VDP2_COBR,STV_VDP2_COBG,STV_VDP2_COBB);
}

void saturn_state::stv_vdp2_get_window0_coordinates(int *s_x, int *e_x, int *s_y, int *e_y)
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

void saturn_state::stv_vdp2_get_window1_coordinates(int *s_x, int *e_x, int *s_y, int *e_y)
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
			*s_y = ((STV_VDP2_W1SY & 0x7ff) >> 0);
			*e_y = ((STV_VDP2_W1EY & 0x7ff) >> 0);
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

int saturn_state::get_window_pixel(int s_x,int e_x,int s_y,int e_y,int x, int y,UINT8 win_num)
{
	int res;

	res = 1;
	if(stv2_current_tilemap.window_control.enabled[win_num])
	{
		if(stv2_current_tilemap.window_control.area[win_num])
			res = (y >= s_y && y <= e_y && x >= s_x && x <= e_x);
		else
			res = (y >= s_y && y <= e_y && x >= s_x && x <= e_x) ^ 1;
	}

	return res;
}

inline int saturn_state::stv_vdp2_window_process(int x,int y)
{
	int s_x=0,e_x=0,s_y=0,e_y=0;
	int w0_pix, w1_pix;

	if (stv2_current_tilemap.window_control.enabled[0] == 0 &&
		stv2_current_tilemap.window_control.enabled[1] == 0)
		return 1;

	stv_vdp2_get_window0_coordinates(&s_x, &e_x, &s_y, &e_y);
	w0_pix = get_window_pixel(s_x,e_x,s_y,e_y,x,y,0);

	stv_vdp2_get_window1_coordinates(&s_x, &e_x, &s_y, &e_y);
	w1_pix = get_window_pixel(s_x,e_x,s_y,e_y,x,y,1);

	return stv2_current_tilemap.window_control.logic & 1 ? (w0_pix | w1_pix) : (w0_pix & w1_pix);
}

/* TODO: remove this crap. */
int saturn_state::stv_vdp2_apply_window_on_layer(rectangle &cliprect)
{
	int s_x=0,e_x=0,s_y=0,e_y=0;

	if ( stv2_current_tilemap.window_control.enabled[0] && (!stv2_current_tilemap.window_control.area[0]))
	{
		/* w0, transparent outside supported */
		stv_vdp2_get_window0_coordinates(&s_x, &e_x, &s_y, &e_y);

		if ( s_x > cliprect.min_x ) cliprect.min_x = s_x;
		if ( e_x < cliprect.max_x ) cliprect.max_x = e_x;
		if ( s_y > cliprect.min_y ) cliprect.min_y = s_y;
		if ( e_y < cliprect.max_y ) cliprect.max_y = e_y;

		return 1;
	}
	else if (  stv2_current_tilemap.window_control.enabled[1] && (!stv2_current_tilemap.window_control.area[1]) )
	{
		/* w1, transparent outside supported */
		stv_vdp2_get_window1_coordinates(&s_x, &e_x, &s_y, &e_y);

		if ( s_x > cliprect.min_x ) cliprect.min_x = s_x;
		if ( e_x < cliprect.max_x ) cliprect.max_x = e_x;
		if ( s_y > cliprect.min_y ) cliprect.min_y = s_y;
		if ( e_y < cliprect.max_y ) cliprect.max_y = e_y;

		return 1;
	}
	else
	{
		return 0;
	}
}

void saturn_state::draw_sprites(bitmap_rgb32 &bitmap, const rectangle &cliprect, UINT8 pri)
{
	int x,y,r,g,b;
	int i;
	UINT16 pix;
	UINT16 *framebuffer_line;
	UINT32 *bitmap_line, *bitmap_line2 = nullptr;
	UINT8  interlace_framebuffer;
	UINT8  double_x;
	static const UINT16 sprite_colormask_table[] = {
		0x07ff, 0x07ff, 0x07ff, 0x07ff, 0x03ff, 0x07ff, 0x03ff, 0x01ff,
		0x007f, 0x003f, 0x003f, 0x003f, 0x00ff, 0x00ff, 0x00ff, 0x00ff
	};
	static const UINT16 priority_shift_table[] = { 14, 13, 14, 13, 13, 12, 12, 12, 7, 7, 6, 0, 7, 7, 6, 0 };
	static const UINT16 priority_mask_table[]  = {  3,  7,  1,  3,  3,  7,  7,  7, 1, 1, 3, 0, 1, 1, 3, 0 };
	static const UINT16 ccrr_shift_table[] =     { 11, 11, 11, 11, 10, 11, 10,  9, 0, 6, 0, 6, 0, 6, 0, 6 };
	static const UINT16 ccrr_mask_table[] =      {  7,  3,  7,  3,  7,  1,  3,  7, 0, 1, 0, 3, 0, 1, 0, 3 };
	static const UINT16 shadow_mask_table[] = { 0, 0, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0, 0, 0, 0, 0, 0, 0, 0 };
	UINT16 alpha_enabled;

	int sprite_type;
	int sprite_colormask;
	int color_offset_pal;
	int sprite_shadow;
	UINT16 sprite_priority_shift, sprite_priority_mask, sprite_ccrr_shift, sprite_ccrr_mask;
	UINT8   priority;
	UINT8   ccr = 0;
	UINT8 sprite_priorities[8];
	UINT8 sprite_ccr[8];
	int sprite_color_mode = STV_VDP2_SPCLMD;

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
	sprite_shadow = shadow_mask_table[sprite_type];

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
			case 0x3: alpha_enabled = 2; sprite_shadow = 0; break;
		}
	}
	else
	{
		alpha_enabled = 0;
	}

	/* framebuffer interlace */
	if ( (STV_VDP2_LSMD == 3) && m_vdp1.framebuffer_double_interlace == 0 )
		interlace_framebuffer = 1;
	else
		interlace_framebuffer = 0;

	/*Guess:Some games needs that the horizontal sprite size to be doubled
	  (TODO: understand the proper settings,it might not work like this)*/
	if(STV_VDP1_TVM == 0 && STV_VDP2_HRES & 2) // astrass & findlove
		double_x = 1;
	else
		double_x = 0;

	/* window control */
	stv2_current_tilemap.window_control.logic = STV_VDP2_SPLOG;
	stv2_current_tilemap.window_control.enabled[0] = STV_VDP2_SPW0E;
	stv2_current_tilemap.window_control.enabled[1] = STV_VDP2_SPW1E;
//  stv2_current_tilemap.window_control.? = STV_VDP2_SPSWE;
	stv2_current_tilemap.window_control.area[0] = STV_VDP2_SPW0A;
	stv2_current_tilemap.window_control.area[1] = STV_VDP2_SPW1A;
//  stv2_current_tilemap.window_control.? = STV_VDP2_SPSWA;

//  stv_vdp2_apply_window_on_layer(mycliprect);

	if (interlace_framebuffer == 0 && double_x == 0 )
	{
		if ( alpha_enabled == 0 )
		{
			for ( y = cliprect.min_y; y <= cliprect.max_y; y++ )
			{
				if ( stv_sprite_priorities_usage_valid )
					if (stv_sprite_priorities_in_fb_line[y][pri] == 0)
						continue;

				framebuffer_line = m_vdp1.framebuffer_display_lines[y];
				bitmap_line = &bitmap.pix32(y);

				for ( x = cliprect.min_x; x <= cliprect.max_x; x++ )
				{
					if(!stv_vdp2_window_process(x,y))
						continue;

					pix = framebuffer_line[x];
					if ( (pix & 0x8000) && sprite_color_mode)
					{
						if ( sprite_priorities[0] != pri )
						{
							stv_sprite_priorities_used[sprite_priorities[0]] = 1;
							stv_sprite_priorities_in_fb_line[y][sprite_priorities[0]] = 1;
							continue;
						};

						if(STV_VDP2_SPWINEN && pix == 0x8000) /* Pukunpa */
							continue;

						b = pal5bit((pix & 0x7c00) >> 10);
						g = pal5bit((pix & 0x03e0) >> 5);
						r = pal5bit( pix & 0x001f);
						if ( color_offset_pal )
						{
							stv_vdp2_compute_color_offset( &r, &g, &b, STV_VDP2_SPCOSL );
						}

						bitmap_line[x] = rgb_t(r, g, b);
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

						{
							pix &= sprite_colormask;
							if ( pix == (sprite_colormask - 1) )
							{
								/*shadow - in reality, we should check from what layer pixel beneath comes...*/
								if ( STV_VDP2_SDCTL & 0x3f )
								{
									rgb_t p = bitmap_line[x];
									bitmap_line[x] = rgb_t(p.r() >> 1, p.g() >> 1, p.b() >> 1);
								}
								/* note that when shadows are disabled, "shadow" palette entries are not drawn */
							}
							else if ( pix )
							{
								pix += (STV_VDP2_SPCAOS << 8);
								pix &= 0x7ff;
								pix += color_offset_pal;
								bitmap_line[x] = m_palette->pen( pix );
							}
						}

						/* TODO: I don't think this one makes much logic ... (1) */
						if ( pix & sprite_shadow )
						{
							if ( pix & ~sprite_shadow )
							{
								rgb_t p = bitmap_line[x];
								bitmap_line[x] = rgb_t(p.r() >> 1, p.g() >> 1, p.b() >> 1);
							}
						}
					}
				}
			}
		}
		else //alpha_enabled == 1
		{
			for ( y = cliprect.min_y; y <= cliprect.max_y; y++ )
			{
				if ( stv_sprite_priorities_usage_valid )
					if (stv_sprite_priorities_in_fb_line[y][pri] == 0)
						continue;

				framebuffer_line = m_vdp1.framebuffer_display_lines[y];
				bitmap_line = &bitmap.pix32(y);

				for ( x = cliprect.min_x; x <= cliprect.max_x; x++ )
				{
					if(!stv_vdp2_window_process(x,y))
						continue;

					pix = framebuffer_line[x];
					if ( (pix & 0x8000) && sprite_color_mode)
					{
						if ( sprite_priorities[0] != pri )
						{
							stv_sprite_priorities_used[sprite_priorities[0]] = 1;
							stv_sprite_priorities_in_fb_line[y][sprite_priorities[0]] = 1;
							continue;
						};

						b = pal5bit((pix & 0x7c00) >> 10);
						g = pal5bit((pix & 0x03e0) >> 5);
						r = pal5bit( pix & 0x001f);
						if ( color_offset_pal )
						{
							stv_vdp2_compute_color_offset( &r, &g, &b, STV_VDP2_SPCOSL );
						}
						ccr = sprite_ccr[0];
						if ( STV_VDP2_CCMD )
						{
							bitmap_line[x] = stv_add_blend( bitmap_line[x], rgb_t(r, g, b));
						}
						else
						{
							bitmap_line[x] = alpha_blend_r32( bitmap_line[x], rgb_t(r, g ,b), ((UINT16)(0x1f-ccr)*0xff)/0x1f);
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
						if ( alpha_enabled == 2 )
						{
							if ( ( pix & 0x8000 ) == 0 )
							{
								ccr = 0;
							}
						}


						{
							pix &= sprite_colormask;
							if ( pix == (sprite_colormask - 1) )
							{
								/*shadow - in reality, we should check from what layer pixel beneath comes...*/
								if ( STV_VDP2_SDCTL & 0x3f )
								{
									rgb_t p = bitmap_line[x];
									bitmap_line[x] = rgb_t(p.r() >> 1, p.g() >> 1, p.b() >> 1);
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
										bitmap_line[x] = stv_add_blend( bitmap_line[x], m_palette->pen(pix) );
									}
									else
									{
										bitmap_line[x] = alpha_blend_r32( bitmap_line[x], m_palette->pen(pix), ((UINT16)(0x1f-ccr)*0xff)/0x1f );
									}
								}
								else
									bitmap_line[x] = m_palette->pen(pix);
							}
						}

						/* TODO: (1) */
						if ( pix & sprite_shadow )
						{
							if ( pix & ~sprite_shadow )
							{
								rgb_t p = bitmap_line[x];
								bitmap_line[x] = rgb_t(p.r() >> 1, p.g() >> 1, p.b() >> 1);
							}
						}
					}
				}
			}
		}
	}
	else
	{
		for ( y = cliprect.min_y; y <= cliprect.max_y / (interlace_framebuffer+1); y++ )
		{
			if ( stv_sprite_priorities_usage_valid )
				if (stv_sprite_priorities_in_fb_line[y][pri] == 0)
					continue;

			framebuffer_line = m_vdp1.framebuffer_display_lines[y];
			if ( interlace_framebuffer == 0 )
			{
				bitmap_line = &bitmap.pix32(y);
			}
			else
			{
				bitmap_line = &bitmap.pix32(2*y);
				bitmap_line2 = &bitmap.pix32(2*y + 1);
			}

			for ( x = cliprect.min_x; x <= cliprect.max_x /(double_x+1) ; x++ )
			{
				if(!stv_vdp2_window_process(x,y))
					continue;

				pix = framebuffer_line[x];
				if ( (pix & 0x8000) && sprite_color_mode)
				{
					if ( sprite_priorities[0] != pri )
					{
						stv_sprite_priorities_used[sprite_priorities[0]] = 1;
						stv_sprite_priorities_in_fb_line[y][sprite_priorities[0]] = 1;
						continue;
					};

					b = pal5bit((pix & 0x7c00) >> 10);
					g = pal5bit((pix & 0x03e0) >> 5);
					r = pal5bit( pix & 0x001f);
					if ( color_offset_pal )
					{
						stv_vdp2_compute_color_offset( &r, &g, &b, STV_VDP2_SPCOSL );
					}
					if ( alpha_enabled == 0 )
					{
						if(double_x)
						{
							bitmap_line[x*2] = rgb_t(r, g, b);
							if ( interlace_framebuffer == 1 ) bitmap_line2[x*2] = rgb_t(r, g, b);
							bitmap_line[x*2+1] = rgb_t(r, g, b);
							if ( interlace_framebuffer == 1 ) bitmap_line2[x*2+1] = rgb_t(r, g, b);
						}
						else
						{
							bitmap_line[x] = rgb_t(r, g, b);
							if ( interlace_framebuffer == 1 ) bitmap_line2[x] = rgb_t(r, g, b);
						}
					}
					else // alpha_blend == 1
					{
						ccr = sprite_ccr[0];

						if ( STV_VDP2_CCMD )
						{
							if(double_x)
							{
								bitmap_line[x*2] = stv_add_blend( bitmap_line[x*2], rgb_t(r, g, b) );
								if ( interlace_framebuffer == 1 ) bitmap_line2[x*2] = stv_add_blend( bitmap_line2[x*2], rgb_t(r, g, b) );
								bitmap_line[x*2+1] = stv_add_blend( bitmap_line[x*2+1], rgb_t(r, g, b) );
								if ( interlace_framebuffer == 1 ) bitmap_line2[x*2+1] = stv_add_blend( bitmap_line2[x*2+1], rgb_t(r, g, b) );
							}
							else
							{
								bitmap_line[x] = stv_add_blend( bitmap_line[x], rgb_t(r, g, b) );
								if ( interlace_framebuffer == 1 ) bitmap_line2[x] = stv_add_blend( bitmap_line2[x], rgb_t(r, g, b) );
							}
						}
						else
						{
							if(double_x)
							{
								bitmap_line[x*2] = alpha_blend_r32( bitmap_line[x*2], rgb_t(r, g, b), ((UINT16)(0x1f-ccr)*0xff)/0x1f );
								if ( interlace_framebuffer == 1 ) bitmap_line2[x*2] = alpha_blend_r32( bitmap_line2[x*2], rgb_t(r, g, b), ((UINT16)(0x1f-ccr)*0xff)/0x1f );
								bitmap_line[x*2+1] = alpha_blend_r32( bitmap_line[x*2+1], rgb_t(r, g, b), ((UINT16)(0x1f-ccr)*0xff)/0x1f );
								if ( interlace_framebuffer == 1 ) bitmap_line2[x*2+1] = alpha_blend_r32( bitmap_line2[x*2+1], rgb_t(r, g, b), ((UINT16)(0x1f-ccr)*0xff)/0x1f);
							}
							else
							{
								bitmap_line[x] = alpha_blend_r32( bitmap_line[x], rgb_t(r, g, b), ((UINT16)(0x1f-ccr)*0xff)/0x1f);
								if ( interlace_framebuffer == 1 ) bitmap_line2[x] = alpha_blend_r32( bitmap_line2[x], rgb_t(r, g, b), ((UINT16)(0x1f-ccr)*0xff)/0x1f);
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

					if ( alpha_enabled == 2 )
					{
						if ( ( pix & 0x8000 ) == 0 )
						{
							ccr = 0;
						}
					}

					{
						pix &= sprite_colormask;
						if ( pix == (sprite_colormask - 1) )
						{
							/*shadow - in reality, we should check from what layer pixel beneath comes...*/
							if ( STV_VDP2_SDCTL & 0x3f )
							{
								rgb_t p = bitmap_line[x];
								if(double_x)
								{
									p = bitmap_line[x*2];
									bitmap_line[x*2] = rgb_t(p.r() >> 1, p.g() >> 1, p.b() >> 1);
									p = bitmap_line[x*2+1];
									bitmap_line[x*2+1] = rgb_t(p.r() >> 1, p.g() >> 1, p.b() >> 1);
								}
								else
									bitmap_line[x] = rgb_t(p.r() >> 1, p.g() >> 1, p.b() >> 1);
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
									bitmap_line[x*2] = m_palette->pen( pix );
									if ( interlace_framebuffer == 1 ) bitmap_line2[x*2] = m_palette->pen( pix );
									bitmap_line[x*2+1] = m_palette->pen( pix );
									if ( interlace_framebuffer == 1 ) bitmap_line2[x*2+1] = m_palette->pen( pix );
								}
								else
								{
									bitmap_line[x] = m_palette->pen( pix );
									if ( interlace_framebuffer == 1 ) bitmap_line2[x] = m_palette->pen( pix );
								}
							}
							else // alpha_blend == 1
							{
								if ( STV_VDP2_CCMD )
								{
									if(double_x)
									{
										bitmap_line[x*2] = stv_add_blend( bitmap_line[x*2], m_palette->pen(pix) );
										if ( interlace_framebuffer == 1 ) bitmap_line2[x*2] = stv_add_blend( bitmap_line2[x], m_palette->pen(pix) );
										bitmap_line[x*2+1] = stv_add_blend( bitmap_line[x*2+1], m_palette->pen(pix) );
										if ( interlace_framebuffer == 1 ) bitmap_line2[x*2+1] = stv_add_blend( bitmap_line2[x], m_palette->pen(pix) );
									}
									else
									{
										bitmap_line[x] = stv_add_blend( bitmap_line[x], m_palette->pen(pix) );
										if ( interlace_framebuffer == 1 ) bitmap_line2[x] = stv_add_blend( bitmap_line2[x], m_palette->pen(pix) );
									}
								}
								else
								{
									if(double_x)
									{
										bitmap_line[x*2] = alpha_blend_r32( bitmap_line[x*2], m_palette->pen(pix), ((UINT16)(0x1f-ccr)*0xff)/0x1f );
										if ( interlace_framebuffer == 1 ) bitmap_line2[x*2] = alpha_blend_r32( bitmap_line2[x], m_palette->pen(pix), ((UINT16)(0x1f-ccr)*0xff)/0x1f );
										bitmap_line[x*2+1] = alpha_blend_r32( bitmap_line[x*2+1], m_palette->pen(pix), ((UINT16)(0x1f-ccr)*0xff)/0x1f );
										if ( interlace_framebuffer == 1 ) bitmap_line2[x*2+1] = alpha_blend_r32( bitmap_line2[x], m_palette->pen(pix), ((UINT16)(0x1f-ccr)*0xff)/0x1f );
									}
									else
									{
										bitmap_line[x] = alpha_blend_r32( bitmap_line[x], m_palette->pen(pix), ((UINT16)(0x1f-ccr)*0xff)/0x1f );
										if ( interlace_framebuffer == 1 ) bitmap_line2[x] = alpha_blend_r32( bitmap_line2[x], m_palette->pen(pix), ((UINT16)(0x1f-ccr)*0xff)/0x1f );
									}
								}
							}
						}
					}

					/* TODO: (1) */
					if ( pix & sprite_shadow )
					{
						if ( pix & ~sprite_shadow )
						{
							rgb_t p = bitmap_line[x];
							if(double_x)
							{
								p = bitmap_line[x*2];
								bitmap_line[x*2] = rgb_t(p.r() >> 1, p.g() >> 1, p.b() >> 1);
								p = bitmap_line[x*2+1];
								bitmap_line[x*2+1] = rgb_t(p.r() >> 1, p.g() >> 1, p.b() >> 1);
							}
							else
								bitmap_line[x] = rgb_t(p.r() >> 1, p.g() >> 1, p.b() >> 1);
						}
					}
				}
			}
		}
	}

	stv_sprite_priorities_usage_valid = 1;
}

UINT32 saturn_state::screen_update_stv_vdp2(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	stv_vdp2_fade_effects();

	stv_vdp2_draw_back(m_tmpbitmap,cliprect);

	if(STV_VDP2_DISP)
	{
		UINT8 pri;

		stv_sprite_priorities_usage_valid = 0;
		memset(stv_sprite_priorities_used, 0, sizeof(stv_sprite_priorities_used));
		memset(stv_sprite_priorities_in_fb_line, 0, sizeof(stv_sprite_priorities_in_fb_line));

		/*If a plane has a priority value of zero it isn't shown at all.*/
		for(pri=1;pri<8;pri++)
		{
			if(pri==STV_VDP2_N3PRIN) { stv_vdp2_draw_NBG3(m_tmpbitmap,cliprect); }
			if(pri==STV_VDP2_N2PRIN) { stv_vdp2_draw_NBG2(m_tmpbitmap,cliprect); }
			if(pri==STV_VDP2_N1PRIN) { stv_vdp2_draw_NBG1(m_tmpbitmap,cliprect); }
			if(pri==STV_VDP2_N0PRIN) { stv_vdp2_draw_NBG0(m_tmpbitmap,cliprect); }
			if(pri==STV_VDP2_R0PRIN) { stv_vdp2_draw_RBG0(m_tmpbitmap,cliprect); }
			{ draw_sprites(m_tmpbitmap,cliprect,pri); }
		}
	}

	copybitmap(bitmap, m_tmpbitmap, 0, 0, 0, 0, cliprect);

	#if 0
	/* Do NOT remove me, used to test video code performance. */
	if(machine().input().code_pressed(KEYCODE_Q))
	{
		popmessage("Halt CPUs");
		m_maincpu->set_input_line(INPUT_LINE_HALT, ASSERT_LINE);
		m_slave->set_input_line(INPUT_LINE_HALT, ASSERT_LINE);
		m_audiocpu->set_input_line(INPUT_LINE_HALT, ASSERT_LINE);
	}
	#endif
	return 0;
}
