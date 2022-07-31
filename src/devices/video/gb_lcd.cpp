// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
/***************************************************************************

  gb_lcd.c

  Video file to handle emulation of the Nintendo Game Boy.

  Original code                               Carsten Sorensen   1998
  Mess modifications, bug fixes and speedups  Hans de Goede      1998
  Bug fixes, SGB and GBC code                 Anthony Kruize     2002
  Improvements to match real hardware         Wilbert Pol        2006-2008

  Timing is not accurate enough:
  - Mode 3 takes 172 cycles (measured with logic analyzer by costis)

The following timing of the first frame when the LCD is turned on, is with
no sprites being displayed. If sprites are displayed then the timing of mode
3 and mode 0 will change.

* LCD turn on for a DMG (on a CGB the timing is a little bit different)
our state                                     LY   Mode  #Cycles
(GB_LCD_STATE_LY00_M2)                        0    0     80 (?)   The unit is actually in mode 0 here and the OAM area is unlocked
GB_LCD_STATE_LYXX_M3                          0    3     169-172  Draw line 0 during this period
  - Lock OAM area
  - Lock VRAM area
GB_LCD_STATE_LYXX_M0                          0    0     1
  - Unlock OAM area
  - Unlock VRAM area
GB_LCD_STATE_LYXX_M0_2                        0    0     199-202
  - Check for mode 0 interrupt
GB_LCD_STATE_LYXX_M0_INC                      1    0     4
  - clear mode 0 interrupt
  - check mode 2 interrupt
  - clear LY=LYC incidence flag
  - check LY=LYC interrupt
GB_LCD_STATE_LYXX_M2                          1    2     8
  - clear mode 0 interrupt
  - set LY=LYC incidence flag
  - Lock OAM area
  - latch WNDPOSY
GB_LCD_STATE_LYXX_M2_WND                      1    2     72
  - check whether window could trigger this line
GB_LCD_STATE_LYXX_M3                          1    3     169-172
etc, until                                    *    *     *
GB_LCD_STATE_LYXX_M0_2                        143  0     199-202
GB_LCD_STATE_LYXX_M0_INC                      144  0     4
  - check mode 2 interrupt
  - clear LY=LYC incidence flag
  - check LY=LYC interrupt
GB_LCD_STATE_LY9X_M1                          144  1     452
  - clear mode 0 interrupt
  - clear mode 2 interrupt
  - trigger VBlank interrupt
  - check mode 1 interrupt
  - set LY=LYC incidence flag
  - check LY=LYC interrupt
GB_LCD_STATE_LY9X_M1_INC                      145  1     4
  - clear LY=LYC incidence flag
GB_LCD_STATE_LY9X_M1                          145  1     452
  - set LY=LYC incidence flag
  - check LY=LYC interrupt
etc, until                                    *    1     *
GB_LCD_STATE_LY9X_M1_INC                      153  1     4
  - clear LY=LYC incidence flag
GB_LCD_STATE_LY00_M1                          0    1     4
  - set LY=LYC incidence flag (for line 153)
  - check LY=LYC interrupt (for line 153)
GB_LCD_STATE_LY00_M1_1                        0    1     4
  - clear LY=LYC incidence flag
GB_LCD_STATE_LY00_M1_2                        0    1     444
  - check LY=LYC interrupt (for line 0)
  - set LY=LYC incidence flag (for line 0)
GB_LCD_STATE_LY00_M0                          0    0     4
GB_LCD_STATE_LY00_M2                          0    2     8
  - clear mode 1 interrupt
  - check mode 2 interrupt
  - lock OAM area
  - latch WNDPOSY
GB_LCD_STATE_LY00_M2_WND                      0    2     72
  - check whether window could trigger this line
GB_LCD_STATE_LYXX_M3                          0    3     169-172




From kevtris' gameboy documentation:

Accessing VRAM while drawing a regular line (window is off):
- B01  - 6 cycles     ($9800, thrown away)
- B01s - 20x8 cycles  ($9800, $9801, $9802, $9803, etc)
- B01s - 7.5 cycles
total: 6 + 167.5 + xscroll & 7


Window is on; 0 < xwindow < $a6
- B01  - 6 cycles                                   ($9800, thrown away)
- B01s - 1-172 cycles (xscroll & 7 + xwindow + 1)   ($9800, $9801, $9802, etc)
- W01  - 6 cycles                                   ($9c00)
- W01s - 1.5-166.5 cycles (166.5 - xwindow)         ($9c01, $9c02, $9c03, etc)
Total: 6  + 6 + 167.5 + xscroll & 7


Window is on; xwindow = 0, xscroll & 7 = 0 - 6
- B01B - 7 cycles             ($9800, thrown away)
- W01  - 6 cycles             ($9c00)
- W01s - 167.5 + xscroll & 7  ($9c01, $9c02, $9c03, $9c04, etc)
Total: 7 + 6 + 167.5 + xscroll & 7


Window is on; xwindow = 0; xscroll & 7 = 7
- B01B - 7 cycles                                                         ($9800, thrown away)
- W01  - 6 cycles                                                         ($9c00)
- W01s - 167.5 + xscroll & 7 + 1 cycle delay during first sprite window   ($9c01, $9c02, $9c03, etc)
Total: 7 + 6 + 167.5 + xscroll & 7 + 1


Window is on; xwindow = $a6
- First scanline displays first scanline of the window
- ywindow values:
  00 : window line 0, window line 1, window line 2, window line 3
  01 : window line 0, background line 1, window line 2, window line 3
  02 : window line 0, background line 1, background line 2, window line 2, window line 3
  03 : window line 0, background line 1, background line 2, background line 3, window line 2, window line 3
- W01  - 6 cycles     ($9c00, thrown away)
- W01s - 20x8 cycles  ($9c01, $9c02, $9c03, $9c04, etc)
- W01s - 7.5 cycles
total: 6 + 167.5 + xscroll & 7


From gambatte scx_m3_extend test a precisely triggered write can extend the m3
period with the xscroll & 7 delay getting applied twice!
The write to the xscroll register occurs during the first B01s sequence for
this to happen or when it is almost/just done applying the xscroll & 7 cycles;
it is still unclear exactly when the xscroll delay cycles are applied:
B01 apply-scx B01sB01sB01s...
or
B01 B01 apply-scx sB01sB01s...

The first option seems the most logical setup since the xscroll & 7 delay is
applied in all cases including when xwindow is low, like 0, 1, or 2. In those
cases the B01 from the first B01s sequence is not completed yet. Unless it is
applied but only in the form of wait cycles before the window is started.


TODO:
- Add sprite support to new rendering engine.
- Replace memory map during OAM operation.
- Fix more test cases.
- Convert CGB code to new rendering engine.
- Simplify code. The code was built up adding more and more support for
  several cases, so it has become a bit more complex than is necessary. Once
  we have a baseline for passing testcases code simplifications can be
  more easily tested.

***************************************************************************/

#include "emu.h"
#include "video/gb_lcd.h"

#include "screen.h"

//#define VERBOSE 1
#include "logmacro.h"


#define LCDCONT     m_vid_regs[0x00]  /* LCD control register                       */
#define LCDSTAT     m_vid_regs[0x01]  /* LCD status register                        */
#define SCROLLY     m_vid_regs[0x02]  /* Starting Y position of the background      */
#define SCROLLX     m_vid_regs[0x03]  /* Starting X position of the background      */
#define CURLINE     m_vid_regs[0x04]  /* Current screen line being scanned          */
#define CMPLINE     m_vid_regs[0x05]  /* Gen. int. when scan reaches this line      */
#define BGRDPAL     m_vid_regs[0x07]  /* Background palette                         */
#define SPR0PAL     m_vid_regs[0x08]  /* Sprite palette #0                          */
#define SPR1PAL     m_vid_regs[0x09]  /* Sprite palette #1                          */
#define WNDPOSY     m_vid_regs[0x0A]  /* Window Y position                          */
#define WNDPOSX     m_vid_regs[0x0B]  /* Window X position                          */
#define KEY1        m_vid_regs[0x0D]  /* Prepare speed switch                       */
#define HDMA1       m_vid_regs[0x11]  /* HDMA source high byte                      */
#define HDMA2       m_vid_regs[0x12]  /* HDMA source low byte                       */
#define HDMA3       m_vid_regs[0x13]  /* HDMA destination high byte                 */
#define HDMA4       m_vid_regs[0x14]  /* HDMA destination low byte                  */
#define HDMA5       m_vid_regs[0x15]  /* HDMA length/mode/start                     */
#define GBCBCPS     m_vid_regs[0x28]  /* Backgound palette spec                     */
#define GBCBCPD     m_vid_regs[0x29]  /* Backgound palette data                     */
#define GBCOCPS     m_vid_regs[0x2A]  /* Object palette spec                        */
#define GBCOCPD     m_vid_regs[0x2B]  /* Object palette data                        */

/* -- Super Game Boy specific -- */
#define SGB_BORDER_PAL_OFFSET   64  /* Border colours stored from pal 4-7   */
#define SGB_XOFFSET             48  /* GB screen starts at column 48        */
#define SGB_YOFFSET             40  /* GB screen starts at row 40           */


enum {
	UNLOCKED=0,
	LOCKED
};


enum {
	GB_LCD_STATE_LYXX_M3=1,
	GB_LCD_STATE_LYXX_M3_2,
	GB_LCD_STATE_LYXX_PRE_M0,
	GB_LCD_STATE_LYXX_M0,
	GB_LCD_STATE_LYXX_M0_2,
	GB_LCD_STATE_LYXX_M0_GBC_PAL,
	GB_LCD_STATE_LYXX_M0_PRE_INC,
	GB_LCD_STATE_LYXX_M0_INC,
	GB_LCD_STATE_LY00_M2,
	GB_LCD_STATE_LY00_M2_WND,
	GB_LCD_STATE_LYXX_M2,
	GB_LCD_STATE_LYXX_M2_WND,
	GB_LCD_STATE_LY9X_M1,
	GB_LCD_STATE_LY9X_M1_INC,
	GB_LCD_STATE_LY00_M1,
	GB_LCD_STATE_LY00_M1_1,
	GB_LCD_STATE_LY00_M1_2,
	GB_LCD_STATE_LY00_M0
};


/* OAM contents on power up.

 The OAM area seems contain some kind of unit fingerprint. On each boot
 the data is almost always the same. Some random bits are flipped between
 different boots. It is currently unknown how much these fingerprints
 differ between different units.

 OAM fingerprints taken from Wilbert Pol's own unit.
 */

static const uint8_t dmg_oam_fingerprint[0x100] = {
	0xD8, 0xE6, 0xB3, 0x89, 0xEC, 0xDE, 0x11, 0x62, 0x0B, 0x7E, 0x48, 0x9E, 0xB9, 0x6E, 0x26, 0xC9,
	0x36, 0xF4, 0x7D, 0xE4, 0xD9, 0xCE, 0xFA, 0x5E, 0xA3, 0x77, 0x60, 0xFC, 0x1C, 0x64, 0x8B, 0xAC,
	0xB6, 0x74, 0x3F, 0x9A, 0x0E, 0xFE, 0xEA, 0xA9, 0x40, 0x3A, 0x7A, 0xB6, 0xF2, 0xED, 0xA8, 0x3E,
	0xAF, 0x2C, 0xD2, 0xF2, 0x01, 0xE0, 0x5B, 0x3A, 0x53, 0x6A, 0x1C, 0x6C, 0x20, 0xD9, 0x22, 0xB4,
	0x8C, 0x38, 0x71, 0x69, 0x3E, 0x93, 0xA3, 0x22, 0xCE, 0x76, 0x24, 0xE7, 0x1A, 0x14, 0x6B, 0xB1,
	0xF9, 0x3D, 0xBF, 0x3D, 0x74, 0x64, 0xCB, 0xF5, 0xDC, 0x9A, 0x53, 0xC6, 0x0E, 0x78, 0x34, 0xCB,
	0x42, 0xB3, 0xFF, 0x07, 0x73, 0xAE, 0x6C, 0xA2, 0x6F, 0x6A, 0xA4, 0x66, 0x0A, 0x8C, 0x40, 0xB3,
	0x9A, 0x3D, 0x39, 0x78, 0xAB, 0x29, 0xE7, 0xC5, 0x7A, 0xDD, 0x51, 0x95, 0x2B, 0xE4, 0x1B, 0xF6,
	0x31, 0x16, 0x34, 0xFE, 0x11, 0xF2, 0x5E, 0x11, 0xF3, 0x95, 0x66, 0xB9, 0x37, 0xC2, 0xAD, 0x6D,
	0x1D, 0xA7, 0x79, 0x06, 0xD7, 0xE5, 0x8F, 0xFA, 0x9C, 0x02, 0x0C, 0x31, 0x8B, 0x17, 0x2E, 0x31,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

static const uint8_t mgb_oam_fingerprint[0x100] = {
	0xB9, 0xE9, 0x0D, 0x69, 0xBB, 0x7F, 0x00, 0x80, 0xE9, 0x7B, 0x79, 0xA2, 0xFD, 0xCF, 0xD8, 0x0A,
	0x87, 0xEF, 0x44, 0x11, 0xFE, 0x37, 0x10, 0x21, 0xFA, 0xFF, 0x00, 0x17, 0xF6, 0x4F, 0x83, 0x03,
	0x3A, 0xF4, 0x00, 0x24, 0xBB, 0xAE, 0x05, 0x01, 0xFF, 0xF7, 0x12, 0x48, 0xA7, 0x5E, 0xF6, 0x28,
	0x5B, 0xFF, 0x2E, 0x10, 0xFF, 0xB9, 0x50, 0xC8, 0xAF, 0x77, 0x2C, 0x1A, 0x62, 0xD7, 0x81, 0xC2,
	0xFD, 0x5F, 0xA0, 0x94, 0xAF, 0xFF, 0x51, 0x20, 0x36, 0x76, 0x50, 0x0A, 0xFD, 0xF6, 0x20, 0x00,
	0xFE, 0xF7, 0xA0, 0x68, 0xFF, 0xFC, 0x29, 0x51, 0xA3, 0xFA, 0x06, 0xC4, 0x94, 0xFF, 0x39, 0x0A,
	0xFF, 0x6C, 0x20, 0x20, 0xF1, 0xAD, 0x0C, 0x81, 0x56, 0xFB, 0x03, 0x82, 0xFF, 0xFF, 0x08, 0x58,
	0x96, 0x7E, 0x01, 0x4D, 0xFF, 0xE4, 0x82, 0xE3, 0x3D, 0xBB, 0x54, 0x00, 0x3D, 0xF3, 0x04, 0x21,
	0xB7, 0x39, 0xCC, 0x10, 0xF9, 0x5B, 0x80, 0x50, 0x3F, 0x6A, 0x1C, 0x21, 0x1F, 0xFA, 0xA8, 0x52,
	0x5F, 0xB3, 0x44, 0xA1, 0x96, 0x1E, 0x00, 0x27, 0x63, 0x77, 0x30, 0x54, 0x37, 0x6F, 0x60, 0x22,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

static const uint8_t cgb_oam_fingerprint[0x100] = {
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x74, 0xFF, 0x09, 0x00, 0x9D, 0x61, 0xA8, 0x28, 0x36, 0x1E, 0x58, 0xAA, 0x75, 0x74, 0xA1, 0x42,
	0x05, 0x96, 0x40, 0x09, 0x41, 0x02, 0x60, 0x00, 0x1F, 0x11, 0x22, 0xBC, 0x31, 0x52, 0x22, 0x54,
	0x22, 0xA9, 0xC4, 0x00, 0x1D, 0xAD, 0x80, 0x0C, 0x5D, 0xFA, 0x51, 0x92, 0x93, 0x98, 0xA4, 0x04,
	0x22, 0xA9, 0xC4, 0x00, 0x1D, 0xAD, 0x80, 0x0C, 0x5D, 0xFA, 0x51, 0x92, 0x93, 0x98, 0xA4, 0x04,
	0x22, 0xA9, 0xC4, 0x00, 0x1D, 0xAD, 0x80, 0x0C, 0x5D, 0xFA, 0x51, 0x92, 0x93, 0x98, 0xA4, 0x04,
	0x22, 0xA9, 0xC4, 0x00, 0x1D, 0xAD, 0x80, 0x0C, 0x5D, 0xFA, 0x51, 0x92, 0x93, 0x98, 0xA4, 0x04
};

/*
 For an AGS in CGB mode this data is: */
#if 0
static const uint8_t ags_oam_fingerprint[0x100] = {
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA,
	0xBB, 0xBB, 0xBB, 0xBB, 0xBB, 0xBB, 0xBB, 0xBB, 0xBB, 0xBB, 0xBB, 0xBB, 0xBB, 0xBB, 0xBB, 0xBB,
	0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC,
	0xDD, 0xDD, 0xDD, 0xDD, 0xDD, 0xDD, 0xDD, 0xDD, 0xDD, 0xDD, 0xDD, 0xDD, 0xDD, 0xDD, 0xDD, 0xDD,
	0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF
};
#endif


DEFINE_DEVICE_TYPE(DMG_PPU, dmg_ppu_device, "dmg_ppu", "DMG PPU")
DEFINE_DEVICE_TYPE(MGB_PPU, mgb_ppu_device, "mgb_ppu", "MGB PPU")
DEFINE_DEVICE_TYPE(SGB_PPU, sgb_ppu_device, "sgb_ppu", "SGB PPU")
DEFINE_DEVICE_TYPE(CGB_PPU, cgb_ppu_device, "cgb_ppu", "CGB PPU")



dmg_ppu_device::dmg_ppu_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, uint32_t vram_size)
	: device_t(mconfig, type, tag, owner, clock)
	, device_video_interface(mconfig, *this)
	, m_lr35902(*this, finder_base::DUMMY_TAG)
	, m_enable_experimental_engine(false)
	, m_oam_size(0x100)
	, m_vram_size(vram_size)
{
}

dmg_ppu_device::dmg_ppu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: dmg_ppu_device(mconfig, DMG_PPU, tag, owner, clock, 0x2000)
{
	m_enable_experimental_engine = true;
}

mgb_ppu_device::mgb_ppu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: dmg_ppu_device(mconfig, MGB_PPU, tag, owner, clock, 0x2000)
{
	m_enable_experimental_engine = true;
}

sgb_ppu_device::sgb_ppu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: dmg_ppu_device(mconfig, SGB_PPU, tag, owner, clock, 0x2000)
{
	m_enable_experimental_engine = false;
}

cgb_ppu_device::cgb_ppu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: dmg_ppu_device(mconfig, CGB_PPU, tag, owner, clock, 0x4000)
{
	m_enable_experimental_engine = false;
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void dmg_ppu_device::common_start()
{
	screen().register_screen_bitmap(m_bitmap);
	save_item(NAME(m_bitmap));
	m_oam = make_unique_clear<uint8_t[]>(m_oam_size);
	m_vram = make_unique_clear<uint8_t[]>(m_vram_size);

	machine().save().register_postload(save_prepost_delegate(FUNC(dmg_ppu_device::videoptr_restore), this));
	m_lcd_timer = timer_alloc(FUNC(dmg_ppu_device::update_tick), this);

	m_program_space = &m_lr35902->space(AS_PROGRAM);

	save_pointer(NAME(m_oam), m_oam_size);
	save_pointer(NAME(m_vram), m_vram_size);
	save_item(NAME(m_window_lines_drawn));
	save_item(NAME(m_vid_regs));
	save_item(NAME(m_bg_zbuf));

	save_item(NAME(m_cgb_bpal));
	save_item(NAME(m_cgb_spal));

	save_item(NAME(m_gb_bpal));
	save_item(NAME(m_gb_spal0));
	save_item(NAME(m_gb_spal1));

	save_item(NAME(m_current_line));
	save_item(NAME(m_cmp_line));
	save_item(NAME(m_sprCount));
	save_item(NAME(m_sprite));
	save_item(NAME(m_previous_line));
	save_item(NAME(m_start_x));
	save_item(NAME(m_end_x));
	save_item(NAME(m_mode));
	save_item(NAME(m_state));
	save_item(NAME(m_sprite_cycles));
	save_item(NAME(m_window_cycles));
	save_item(NAME(m_scrollx_adjust));
	save_item(NAME(m_oam_locked));
	save_item(NAME(m_oam_locked_reading));
	save_item(NAME(m_vram_locked));
	save_item(NAME(m_pal_locked));
	save_item(NAME(m_hdma_enabled));
	save_item(NAME(m_hdma_possible));
	save_item(NAME(m_hdma_cycles_to_start));
	save_item(NAME(m_hdma_length));
	save_item(NAME(m_oam_dma_start_cycles));
	save_item(NAME(m_oam_dma_cycles_left));
	save_item(NAME(m_oam_dma_source_address));
	save_item(NAME(m_gbc_mode));
	save_item(NAME(m_window_x));
	save_item(NAME(m_window_y));
	save_item(NAME(m_stat_mode0_int));
	save_item(NAME(m_stat_mode1_int));
	save_item(NAME(m_stat_mode2_int));
	save_item(NAME(m_stat_lyc_int));
	save_item(NAME(m_stat_lyc_int_prev));
	save_item(NAME(m_stat_write_int));
	save_item(NAME(m_stat_int));
	save_item(NAME(m_gb_tile_no_mod));
	save_item(NAME(m_oam_dma_processing));
	save_item(NAME(m_gb_chrgen_offs));
	save_item(NAME(m_gb_bgdtab_offs));
	save_item(NAME(m_gb_wndtab_offs));
	save_item(NAME(m_gbc_chrgen_offs));
	save_item(NAME(m_gbc_bgdtab_offs));
	save_item(NAME(m_gbc_wndtab_offs));
	save_item(NAME(m_vram_bank));
	save_item(NAME(m_last_updated));
	save_item(NAME(m_cycles_left));
	save_item(NAME(m_next_state));
	save_item(NAME(m_old_curline));

	save_item(STRUCT_MEMBER(m_layer, enabled));
	save_item(STRUCT_MEMBER(m_layer, xindex));
	save_item(STRUCT_MEMBER(m_layer, xshift));
	save_item(STRUCT_MEMBER(m_layer, xstart));
	save_item(STRUCT_MEMBER(m_layer, xend));
	save_item(STRUCT_MEMBER(m_layer, bgline));

	save_item(NAME(m_line.tile_cycle));
	save_item(NAME(m_line.tile_count));
	save_item(NAME(m_line.y));
	save_item(NAME(m_line.pattern_address));
	save_item(NAME(m_line.pattern));
	save_item(NAME(m_line.tile_address));
	save_item(NAME(m_line.plane0));
	save_item(NAME(m_line.plane1));
	save_item(NAME(m_line.shift_register));
	save_item(NAME(m_line.sprite_delay_cycles));
	save_item(NAME(m_line.starting));
	save_item(NAME(m_line.sequence_counter));
	save_item(NAME(m_line.drawing));
	save_item(NAME(m_line.start_drawing));
	save_item(NAME(m_line.scrollx_delay));
	save_item(NAME(m_line.scrollx_to_apply));
	save_item(NAME(m_line.pixels_drawn));
	save_item(NAME(m_line.window_compare_position));
	save_item(NAME(m_line.window_active));
	save_item(NAME(m_line.scrollx));
	save_item(NAME(m_line.window_start_y));
	save_item(NAME(m_line.window_start_x));
	save_item(NAME(m_line.window_start_y_index));
	save_item(NAME(m_line.window_enable));
	save_item(NAME(m_line.window_enable_index));
	save_item(NAME(m_line.window_should_trigger));
	save_item(STRUCT_MEMBER(m_line.sprite, enabled));
	save_item(STRUCT_MEMBER(m_line.sprite, x));
	save_item(STRUCT_MEMBER(m_line.sprite, y));
	save_item(STRUCT_MEMBER(m_line.sprite, pattern));
	save_item(STRUCT_MEMBER(m_line.sprite, flags));
	save_item(STRUCT_MEMBER(m_line.sprite, tile_plane_0));
	save_item(STRUCT_MEMBER(m_line.sprite, tile_plane_1));
	save_item(NAME(m_frame_window_active));
}


void dmg_ppu_device::videoptr_restore()
{
	m_layer[0].bg_map = m_vram.get() + m_gb_bgdtab_offs;
	m_layer[0].bg_tiles = m_vram.get() + m_gb_chrgen_offs;
	m_layer[1].bg_map = m_vram.get() + m_gb_wndtab_offs;
	m_layer[1].bg_tiles = m_vram.get() + m_gb_chrgen_offs;
}

void cgb_ppu_device::videoptr_restore()
{
	m_layer[0].bg_map = m_vram.get() + m_gb_bgdtab_offs;
	m_layer[0].gbc_map = m_vram.get() + m_gbc_bgdtab_offs;
	m_layer[1].bg_map = m_vram.get() + m_gb_wndtab_offs;
	m_layer[1].gbc_map = m_vram.get() + m_gbc_wndtab_offs;
}


void dmg_ppu_device::device_start()
{
	common_start();

	memcpy(m_oam.get(), dmg_oam_fingerprint, 0x100);
}

void mgb_ppu_device::device_start()
{
	common_start();

	memcpy(m_oam.get(), mgb_oam_fingerprint, 0x100);
}

void sgb_ppu_device::device_start()
{
	common_start();

	m_sgb_tile_data = make_unique_clear<uint8_t[]>(0x2000);
	save_pointer(NAME(m_sgb_tile_data), 0x2000);

	memset(m_sgb_tile_map, 0, sizeof(m_sgb_tile_map));

	/* Some default colours for non-SGB games */
	m_sgb_pal[0] = 32767;
	m_sgb_pal[1] = 21140;
	m_sgb_pal[2] = 10570;
	m_sgb_pal[3] = 0;
	/* The rest of the colortable can be black */
	for (int i = 4; i < 8 * 16; i++)
		m_sgb_pal[i] = 0;

	save_item(NAME(m_sgb_atf_data));
	save_item(NAME(m_sgb_atf));
	save_item(NAME(m_sgb_pal_data));
	save_item(NAME(m_sgb_pal_map));
	save_item(NAME(m_sgb_pal));
	save_item(NAME(m_sgb_tile_map));
	save_item(NAME(m_sgb_window_mask));
}

void cgb_ppu_device::device_start()
{
	common_start();

	memcpy(m_oam.get(), cgb_oam_fingerprint, 0x100);

	/* Background is initialised as white */
	for (int i = 0; i < 32; i++)
	{
		m_cgb_bpal[i] = 32767;
	}
	/* Sprites are supposed to be uninitialized, but we'll make them black */
	for (int i = 0; i < 32; i++)
	{
		m_cgb_spal[i] = 0;
	}
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void dmg_ppu_device::common_reset()
{
	m_window_lines_drawn = 0;

	m_current_line = 0;
	m_cmp_line = 0;
	m_sprCount = 0;
	m_previous_line = 0;
	m_start_x = 0;
	m_end_x = 0;
	m_mode = 0;
	m_state = 0;
	m_sprite_cycles = 0;
	m_window_cycles = 0;
	m_scrollx_adjust = 0;
	m_oam_locked = 0;
	m_oam_locked_reading = 0;
	m_vram_locked = 0;
	m_pal_locked = 0;
	m_gbc_mode = 0;
	m_gb_tile_no_mod = 0;
	m_vram_bank = 0;
	m_oam_dma_processing = false;
	m_oam_dma_start_cycles = 0;
	m_oam_dma_cycles_left = 0;
	m_updating_state = false;
	m_stat_mode0_int = false;
	m_stat_mode1_int = false;
	m_stat_mode2_int = false;
	m_stat_lyc_int = false;
	m_stat_lyc_int_prev = false;
	m_stat_write_int = false;
	m_stat_int = false;
	m_hdma_cycles_to_start = 0;

	m_gb_chrgen_offs = 0;
	m_gb_bgdtab_offs = 0x1c00;
	m_gb_wndtab_offs = 0x1c00;

	memset(&m_vid_regs, 0, sizeof(m_vid_regs));
	memset(&m_bg_zbuf, 0, sizeof(m_bg_zbuf));
	memset(&m_cgb_bpal, 0, sizeof(m_cgb_bpal));
	memset(&m_cgb_spal, 0, sizeof(m_cgb_spal));
	memset(&m_sprite, 0, sizeof(m_sprite));
	memset(&m_layer[0], 0, sizeof(m_layer[0]));
	memset(&m_layer[1], 0, sizeof(m_layer[1]));

	// specific reg initialization
	m_vid_regs[0x06] = 0xff;

	for (int i = 0x0c; i < NR_GB_VID_REGS; i++)
	{
		m_vid_regs[i] = 0xff;
	}

	LCDSTAT = 0x80;
	LCDCONT = 0x00;     /* Video hardware is turned off at boot time */
	m_current_line = CURLINE = CMPLINE = 0x00;
	SCROLLX = SCROLLY = 0x00;
	SPR0PAL = SPR1PAL = 0xFF;
	WNDPOSX = WNDPOSY = 0x00;

	// Initialize palette arrays
	for (int i = 0; i < 4; i++)
	{
		m_gb_bpal[i] = m_gb_spal0[i] = m_gb_spal1[i] = i;
	}

	m_last_updated = machine().time();
}


void dmg_ppu_device::device_reset()
{
	common_reset();

	m_cycles_left = 456;
	m_lcd_timer->adjust(m_lr35902->cycles_to_attotime(456));
}

void sgb_ppu_device::device_reset()
{
	common_reset();

	memset(m_sgb_tile_data.get(), 0, 0x2000);

	m_sgb_window_mask = 0;

	memset(m_sgb_pal_map, 0, sizeof(m_sgb_pal_map));
	memset(m_sgb_atf_data, 0, sizeof(m_sgb_atf_data));
}

void cgb_ppu_device::device_reset()
{
	common_reset();

	m_gbc_chrgen_offs = 0x2000;
	m_gbc_bgdtab_offs = 0x3c00;
	m_gbc_wndtab_offs = 0x3c00;

	/* HDMA disabled */
	m_hdma_enabled = 0;
	m_hdma_possible = 0;

	m_gbc_mode = 1;
}



inline void dmg_ppu_device::plot_pixel(int x, int y, uint16_t color)
{
	m_bitmap.pix(y, x) = color;
}


void dmg_ppu_device::calculate_window_cycles()
{
	m_window_cycles = 0;

	LOG("m_window_x = %d, m_window_y = %d\n", m_window_x, m_window_y);

	if ((LCDCONT & WINDOW_ENABLED) && m_window_x < 167 && m_window_y < 144)
	{
		// This is not good enough yet
		m_window_cycles = 4;
		if (m_window_x == 0x0f)
		{
			m_window_cycles = 12;
		}
	}
}


void dmg_ppu_device::clear_line_state()
{
	for (int i = 0; i < 10; i++)
	{
		m_line.sprite[i].enabled = false;
	}
	m_line.sprite_delay_cycles = 0;
	m_line.starting = true;
	m_line.sequence_counter = 0;
	m_line.start_drawing = false;
	m_line.drawing = false;
	m_line.scrollx_delay = 0;
	m_line.scrollx_to_apply = 0;
	m_line.pixels_drawn = 0;
	m_line.tile_count = SCROLLX >> 3;
	m_line.tile_cycle = 0;
	m_line.window_compare_position = 0x100;
	m_line.window_active = false;
	m_line.window_should_trigger = false;

	if (m_enable_experimental_engine)
	{
		m_scrollx_adjust = 0;
	}
}


/*
  Select which sprites should be drawn for the current scanline.
 */
void dmg_ppu_device::select_sprites()
{
	m_sprCount = 0;
	m_sprite_cycles = 0;

	/* If video hardware is enabled and sprites are enabled */
	if ((LCDCONT & ENABLED) && (LCDCONT & SPRITES_ENABLED))
	{
		uint8_t sprite_occurs[32];

		memset(sprite_occurs, 0, sizeof(sprite_occurs));

		/* Check for stretched sprites */
		int height = (LCDCONT & 0x04) ? 16 : 8;
		int line = m_current_line + 16;

		for (int i = 0; i < 160; i+= 4)
		{
			if (line >= m_oam[i] && line < (m_oam[i] + height))
			{
				if (m_sprCount < 10)
				{
					m_sprite[m_sprCount] = i / 4;

					if (m_oam[i + 1] < 168)
					{
						m_line.sprite[m_sprCount].enabled = true;
						m_line.sprite[m_sprCount].y = m_oam[i];
						m_line.sprite[m_sprCount].x = m_oam[i + 1];
						m_line.sprite[m_sprCount].pattern = m_oam[i + 2];
						m_line.sprite[m_sprCount].flags = m_oam[i + 3];

						// X=0 is special
						int spr_x = m_oam[i + 1] ? m_oam[i + 1] + (SCROLLX & 0x07) : 0;

						if (sprite_occurs[spr_x >> 3])
						{
							m_sprite_cycles += 3;
						}
						m_sprite_cycles += 3;

						sprite_occurs[spr_x >> 3] |= (1 << (spr_x & 0x07));
					}

					m_sprCount++;
				}
			}
		}

		if (m_sprCount > 0)
		{
			for (int i = 0; i < 22; i++)
			{
				if (sprite_occurs[i])
				{
					LOG("sprite_occurs[%d] = %02x\n", i, sprite_occurs[i]);
				}

				if (sprite_occurs[i])
				{
					static int cycles[32] =
					{
						3, 8, 7, 8, 6, 8, 7, 8, 5, 8, 7, 8, 6, 8, 7, 8,
						4, 8, 7, 8, 6, 8, 7, 8, 5, 8, 7, 8, 6, 8, 7, 8
					};
					m_sprite_cycles += cycles[sprite_occurs[i] & 0x1f];
				}
			}

			LOG("m_sprite_cycles = %d\n", m_sprite_cycles);
		}
	}
}


void dmg_ppu_device::update_line_state(uint64_t cycles)
{
	// Can the bg tilemap select bit be changed while drawing the screen? => yes
	// Can scroll-y be changed while drawing the screen? => yes
	// Can bits 3-8 of scroll-x be changed while drawing the screen? => yes

	while (cycles > 0 && m_line.pixels_drawn < 160)
	{
		// Not sure if delaying during the first B01s sequence is the right time
		if (m_line.scrollx_delay > 0)
		{
			m_line.scrollx_delay -= 1;
			m_line.scrollx_to_apply++;
		}

		//LOG("tile_cycle = %u, starting = %s, drawing = %s\n", m_line.tile_cycle, m_line.starting ? "true" : "false", m_line.drawing ? "true" : "false");
		// output next pixel
		if (m_line.drawing)
		{
			if (m_line.scrollx_to_apply > 0)
			{
				// TODO: Determine when the scrollx shifts are applied when window-x is <= 0x07
				LOG("scrollx_to_apply: %u\n", m_line.scrollx_to_apply);
				if (!m_line.window_active)
				{
					m_line.shift_register <<= 2;
				}
				m_line.window_compare_position--;
				m_line.scrollx_to_apply--;
				m_cycles_left++;
				m_scrollx_adjust++;
			}
			else
			{
				if (!m_line.starting && m_line.tile_cycle < 8)
				{
					if (m_line.pixels_drawn < 8)
					{
						LOG("draw pixel %u\n", m_line.pixels_drawn);
					}
					plot_pixel(m_line.pixels_drawn, m_current_line, m_gb_bpal[m_line.shift_register >> 14]);
					m_bg_zbuf[m_line.pixels_drawn] = m_line.shift_register >> 14;
					m_line.shift_register <<= 2;
					m_line.pixels_drawn++;

					if (m_line.pixels_drawn == 160 && (LCDCONT & SPRITES_ENABLED))
					{
						update_sprites();
					}
				}
			}
		}

		uint8_t next_tile_cycle = m_line.tile_cycle + 1;

		switch (m_line.tile_cycle)
		{
		case 0:  // Set pattern address, latch data into shift register(s)
			if (!m_line.window_active && !(LCDCONT & BACKGROUND_ENABLED))
			{
				m_line.shift_register = 0;
			}
			else
			{
				// Interleave bits from plane0 and plane1
				m_line.shift_register = (((((m_line.plane0 * 0x0101010101010101U) & 0x8040201008040201U) * 0x0102040810204081U) >> 49) & 0x5555)
									  | (((((m_line.plane1 * 0x0101010101010101U) & 0x8040201008040201U) * 0x0102040810204081U) >> 48) & 0xAAAA);
			}
			if (m_line.pixels_drawn < 8)
			{
				LOG("m_current_line: %u, tile_count: %02x, plane0 = %02x, plane1 = %02x, shift_register = %04x\n", m_current_line, m_line.tile_count, m_line.plane0, m_line.plane1, m_line.shift_register);
			}
			if (m_line.sequence_counter >= 2)
			{
				if (!m_line.starting)
				{
					m_line.drawing = true;
				}
			}
			else if (m_line.sequence_counter == 1)
			{
				// start counting for start of window
				m_line.window_compare_position = 0;
			}
			m_line.sequence_counter++;
			if (m_line.window_active)
			{
				m_line.y = m_window_lines_drawn;
				m_line.pattern_address = m_gb_wndtab_offs | ((m_line.y & 0xF8) << 2) | (m_line.tile_count & 0x1f);
			}
			else
			{
				m_line.y = SCROLLY + m_current_line;
				m_line.pattern_address = m_gb_bgdtab_offs | ((m_line.y & 0xF8) << 2) | (((SCROLLX >> 3) + m_line.tile_count) & 0x1f);
			}

			m_line.tile_count++;
			break;

		case 1:  // Read pattern data
			m_line.pattern = m_vram.get()[m_line.pattern_address] ^ m_gb_tile_no_mod;
			if (m_line.tile_count < 8)
			{
				LOG("tile_count = %u, y = %u, pattern = %02x, pattern_address = %04x\n", m_line.tile_count, m_current_line, m_line.pattern, m_line.pattern_address);
			}
			break;

		case 2:  // Set plane 0 address
			m_line.tile_address = m_gb_chrgen_offs + ((m_line.pattern << 4) | ((m_line.y & 0x07) << 1));
			if (m_line.tile_count < 8)
			{
				LOG("tile_count = %u, tile_address = %04x, pattern = %02x, y = %u, m_gb_chrgen_offs = %04x\n", m_line.tile_count, m_line.tile_address, m_line.pattern, m_line.y & 7, m_gb_chrgen_offs);
			}
			break;

		case 3:  // Read plane 0 data
			m_line.plane0 = m_vram.get()[m_line.tile_address];
			if (m_line.starting && !m_line.window_active)
			{
				m_line.scrollx = SCROLLX;
			}
			break;

		case 4:  // Set plane 1 address
			m_line.tile_address = m_line.tile_address + 1;
			break;

		case 5:  // Read plane 1 data
			m_line.plane1 = m_vram.get()[m_line.tile_address];
			if (m_line.starting)
			{
				// TODO: (review) Do not reset tile_count and scroll when restarting for window
				// TODO: Check for window at pos 0
				if (m_line.window_active)
				{
					// Force line_drawing to true
					m_line.sequence_counter = 2;
					if (m_line.scrollx_delay > 0)
					{
					}
				}
				else
				{
					m_line.tile_count = 0;
					m_line.scrollx_delay = m_line.scrollx & 0x07;
				}

				m_line.starting = false;
				next_tile_cycle = 0;
			}
			break;

		case 6:  // sprite stuff
			break;

		case 7:  // more sprite stuff
			if (m_line.sprite_delay_cycles == 0)
			{
				next_tile_cycle &= 7;
			}
			break;

		case 8:   // even more sprite stuff/delay
			m_line.sprite_delay_cycles--;
			m_cycles_left++;
			m_sprite_cycles++;
			next_tile_cycle = m_line.sprite_delay_cycles == 0 ? 0 : 8;
			break;

		case 9:   // eat scrollx delay cycles before starting window
			LOG("eating scrollx_to_apply: %u\n", m_line.scrollx_to_apply);
			m_line.window_compare_position--;
			m_line.scrollx_to_apply--;
			m_cycles_left++;
			m_scrollx_adjust++;
			next_tile_cycle = m_line.scrollx_to_apply == 0 ? 0 : 9;
			break;

		default:
			next_tile_cycle &= 7;
			break;
		}
		m_line.tile_cycle = next_tile_cycle;
		cycles--;

		check_start_of_window();
	}

	if (m_line.pixels_drawn == 160 && m_line.window_active)
	{
		m_window_lines_drawn++;
		m_line.pixels_drawn++;
		m_line.window_active = false;
	}
}


void dmg_ppu_device::check_start_of_window()
{
//
// WY=1
// ly = 2
// late_enable_afterVblank_3_dmg08_cgb04c_out3.gbc - 4 cycles into M2 -> gb+cgb: triggering in both line 2 and 3 still returns STAT mode 0
// late_enable_afterVblank_4_dmg08_out3_cgb04c_out0.gbc - start of M2 -> gb: should not trigger, cgb: should trigger
// late_enable_afterVblank_5_dmg08_cgb04c_out0.gbc - enable 4 cycles before M2 ->  gb+cgb: should trigger on line 2 and 3
//
// Mid frame enable:
// - at start of frame window disabled
// - enabling at line of WY triggers when window is enabled before M2 of WY + 1

	// Check for start of window
	if (m_line.window_compare_position < 16)
	{
		LOG("check window this line, m_current_line = %u, WNDPOSY = %u, WNDPOSX = %u, m_line.window_compare_position = %u, tile_cycle = %u, window_start_y = %u, pixels_drawn = %u\n", m_current_line, WNDPOSY, WNDPOSX, m_line.window_compare_position, m_line.tile_cycle, m_line.window_start_y[m_line.window_start_y_index], m_line.pixels_drawn);
	}

	if (/*LCDCONT*/(m_line.window_enable[m_line.window_enable_index] & WINDOW_ENABLED) && !m_line.window_active && (m_frame_window_active || /*m_current_line >= m_window_y*/ m_line.window_should_trigger || m_current_line == m_line.window_start_y[m_line.window_start_y_index]) && m_line.window_compare_position == /*WNDPOSX*/ m_line.window_start_x[m_line.window_start_y_index] && m_line.window_compare_position < 0xA6)
	{
		LOG("enable window, m_current_line = %u, WNDPOSY = %u, WNDPOSX = %u, m_line.window_compare_position = %u, pixels_drawn = %u\n", m_current_line, WNDPOSY, WNDPOSX, m_line.window_compare_position, m_line.pixels_drawn);
		m_line.starting = true;
		m_line.window_active = true;
		m_frame_window_active = true;
		m_line.tile_cycle = !m_line.drawing && m_line.scrollx_to_apply > 0 ? 9 : 0;
		m_line.tile_count = 0;
		m_window_cycles = 6;
		m_cycles_left += 6;

		if (m_line.window_start_x[m_line.window_start_y_index] == 0)
		{
			// TODO: WX=00 should trigger a 1 cycle delay during the first sprite frame
			// Does not fix failing late_scx_late_wy_FFto4_ly4_wx00_2_dmg08_out3_cgb04c_out0.gbc yet
			m_line.sprite_delay_cycles += 1;
		}
	}

	// 4 makes most tests pass
	m_line.window_start_y[(m_line.window_start_y_index + 4) % std::size(m_line.window_start_y)] = WNDPOSY;
	// 2-4 makes most tests pass
	m_line.window_start_x[(m_line.window_start_y_index + 4) % std::size(m_line.window_start_x)] = WNDPOSX;
	m_line.window_start_y_index = (m_line.window_start_y_index + 1) % std::size(m_line.window_start_y);

	// 3 makes most tests pass
	m_line.window_enable[(m_line.window_enable_index + 3) % std::size(m_line.window_enable)] = LCDCONT;
	m_line.window_enable_index = (m_line.window_enable_index + 1) % std::size(m_line.window_enable);

	if (!m_line.starting && m_line.tile_cycle < 8)
	{
		m_line.window_compare_position++;
	}
}


void dmg_ppu_device::update_sprites()
{
	uint8_t height, tilemask, line, *vram;
	int yindex;

	if (LCDCONT & LARGE_SPRITES)
	{
		height = 16;
		tilemask = 0xFE;
	}
	else
	{
		height = 8;
		tilemask = 0xFF;
	}

	yindex = m_current_line;
	line = m_current_line + 16;

	vram = m_vram.get();
	for (int i = m_sprCount - 1; i >= 0; i--)
	{
		int oam_address = m_sprite[i] * 4;
		uint16_t *spal = (m_oam[oam_address + 3] & 0x10) ? m_gb_spal1 : m_gb_spal0;
		int xindex = m_oam[oam_address + 1] - 8;
		int adr = (m_oam[oam_address + 2] & tilemask) * 16;

		if (xindex < -7 || xindex > 160)
		{
			continue;
		}

		if (m_oam[oam_address + 3] & 0x40)         /* flip y ? */
		{
			adr += (height - 1 - line + m_oam[oam_address]) * 2;
		}
		else
		{
			adr += (line - m_oam[oam_address]) * 2;
		}
		uint16_t data = (vram[adr + 1] << 8) | vram[adr];

		switch (m_oam[oam_address + 3] & 0xA0)
		{
		case 0xA0:                 /* priority is set (behind bgnd & wnd, flip x) */
			for (int bit = 0; bit < 8; bit++, xindex++)
			{
				int colour = ((data & 0x0100) ? 2 : 0) | ((data & 0x0001) ? 1 : 0);
				if (colour && !m_bg_zbuf[xindex] && xindex >= 0 && xindex < 160)
					plot_pixel(xindex, yindex, spal[colour]);
				data >>= 1;
			}
			break;
		case 0x20:                 /* priority is not set (overlaps bgnd & wnd, flip x) */
			for (int bit = 0; bit < 8; bit++, xindex++)
			{
				int colour = ((data & 0x0100) ? 2 : 0) | ((data & 0x0001) ? 1 : 0);
				if (colour && xindex >= 0 && xindex < 160)
					plot_pixel(xindex, yindex, spal[colour]);
				data >>= 1;
			}
			break;
		case 0x80:                 /* priority is set (behind bgnd & wnd, don't flip x) */
			for (int bit = 0; bit < 8 && xindex < 160; bit++, xindex++)
			{
				int colour = ((data & 0x8000) ? 2 : 0) | ((data & 0x0080) ? 1 : 0);
				if (colour && !m_bg_zbuf[xindex] && xindex >= 0 && xindex < 160)
					plot_pixel(xindex, yindex, spal[colour]);
				data <<= 1;
			}
			break;
		case 0x00:                 /* priority is not set (overlaps bgnd & wnd, don't flip x) */
			for (int bit = 0; bit < 8 && xindex < 160; bit++, xindex++)
			{
				int colour = ((data & 0x8000) ? 2 : 0) | ((data & 0x0080) ? 1 : 0);
				if (colour && xindex >= 0 && xindex < 160)
					plot_pixel(xindex, yindex, spal[colour]);
				data <<= 1;
			}
			break;
		}
	}
}


void dmg_ppu_device::update_scanline(uint32_t cycles_to_go)
{
	if (m_enable_experimental_engine)
	{
		return;
	}

	g_profiler.start(PROFILER_VIDEO);

	/* Make sure we're in mode 3 */
	if ((LCDSTAT & 0x03) == 0x03)
	{
		int l = 0;

		if (m_start_x < 0)
		{
			/* Window is enabled if the hardware says so AND the current scanline is
			 * within the window AND the window X coordinate is <=166 */
			m_layer[1].enabled = ((LCDCONT & WINDOW_ENABLED) && (m_current_line >= m_window_y) && (m_window_x <= 166)) ? 1 : 0;

			/* BG is enabled if the hardware says so AND (window_off OR (window_on
			* AND window's X position is >=7)) */
			m_layer[0].enabled = ((LCDCONT & BACKGROUND_ENABLED) && ((!m_layer[1].enabled) || (m_layer[1].enabled && (m_window_x >= 7)))) ? 1 : 0;

			if (m_layer[0].enabled)
			{
				m_layer[0].bgline = (SCROLLY + m_current_line) & 0xFF;
				m_layer[0].bg_map = m_vram.get() + m_gb_bgdtab_offs;
				m_layer[0].bg_tiles = m_vram.get() + m_gb_chrgen_offs;
				m_layer[0].xindex = SCROLLX >> 3;
				m_layer[0].xshift = SCROLLX & 7;
				m_layer[0].xstart = 0;
				m_layer[0].xend = 160;
			}

			if (m_layer[1].enabled)
			{
				int xpos = m_window_x - 7;             /* Window is offset by 7 pixels */
				if (xpos < 0)
					xpos = 0;

				m_layer[1].bgline = m_window_lines_drawn;
				m_layer[1].bg_map = m_vram.get() + m_gb_wndtab_offs;
				m_layer[1].bg_tiles = m_vram.get() + m_gb_chrgen_offs;
				m_layer[1].xindex = 0;
				m_layer[1].xshift = 0;
				m_layer[1].xstart = xpos;
				m_layer[1].xend = 160;
				m_layer[0].xend = xpos;
			}
			m_start_x = 0;
		}

		if (cycles_to_go < 160)
		{
			m_end_x = std::min(int(160 - cycles_to_go), 160);
			/* Draw empty pixels when the background is disabled */
			if (!(LCDCONT & BACKGROUND_ENABLED))
			{
				rectangle r(m_start_x, m_end_x - 1, m_current_line, m_current_line);
				m_bitmap.fill(m_gb_bpal[0], r);
			}
			while (l < 2)
			{
				uint8_t xindex, *map, *tiles;
				uint16_t data;
				int i, tile_index;

				if (!m_layer[l].enabled)
				{
					l++;
					continue;
				}
				map = m_layer[l].bg_map + ((m_layer[l].bgline << 2) & 0x3E0);
				tiles = m_layer[l].bg_tiles + ((m_layer[l].bgline & 7) << 1);
				xindex = m_start_x;
				if (xindex < m_layer[l].xstart)
					xindex = m_layer[l].xstart;
				i = m_end_x;
				if (i > m_layer[l].xend)
					i = m_layer[l].xend;
				i = i - xindex;

				tile_index = (map[m_layer[l].xindex] ^ m_gb_tile_no_mod) * 16;
				data = tiles[tile_index] | (tiles[tile_index+1] << 8);
				data <<= m_layer[l].xshift;

				while (i > 0)
				{
					while ((m_layer[l].xshift < 8) && i)
					{
						int colour = ((data & 0x8000) ? 2 : 0) | ((data & 0x0080) ? 1 : 0);
						plot_pixel(xindex, m_current_line, m_gb_bpal[colour]);
						m_bg_zbuf[xindex] = colour;
						xindex++;
						data <<= 1;
						m_layer[l].xshift++;
						i--;
					}
					if (m_layer[l].xshift == 8)
					{
						/* Take possible changes to SCROLLY into account */
						if (l == 0)
						{
							m_layer[0].bgline = (SCROLLY + m_current_line) & 0xFF;
							map = m_layer[l].bg_map + ((m_layer[l].bgline << 2) & 0x3E0);
							tiles = m_layer[l].bg_tiles + ((m_layer[l].bgline & 7) << 1);
						}

						m_layer[l].xindex = (m_layer[l].xindex + 1) & 31;
						m_layer[l].xshift = 0;
						tile_index = (map[m_layer[l].xindex] ^ m_gb_tile_no_mod) * 16;
						data = tiles[tile_index] | (tiles[tile_index + 1] << 8);
					}
				}
				l++;
			}
			if (m_end_x == 160 && LCDCONT & SPRITES_ENABLED)
			{
				update_sprites();
			}
			m_start_x = m_end_x;
		}
	}
	else
	{
		if (!(LCDCONT & ENABLED))
		{
			/* Draw an empty line when LCD is disabled */
			if (m_previous_line != m_current_line)
			{
				if (m_current_line < 144)
				{
					const rectangle &r = screen().visible_area();
					rectangle r1(r.min_x, r.max_x, m_current_line, m_current_line);
					m_bitmap.fill(0, r1);
				}
				m_previous_line = m_current_line;
			}
		}
	}

	g_profiler.stop();
}

/* --- Super Game Boy Specific --- */

void sgb_ppu_device::update_sprites()
{
	uint8_t height, tilemask, line, *vram;
	int16_t yindex;

	if (LCDCONT & LARGE_SPRITES)
	{
		height = 16;
		tilemask = 0xFE;
	}
	else
	{
		height = 8;
		tilemask = 0xFF;
	}

	/* Offset to center of screen */
	yindex = m_current_line + SGB_YOFFSET;
	line = m_current_line + 16;

	vram = m_vram.get();

	for (int i = m_sprCount - 1; i >= 0; i--)
	{
		int oam_address = m_sprite[i] * 4;
		int adr = (m_oam[oam_address + 2] & tilemask) * 16;
		uint16_t *spal = (m_oam[oam_address + 3] & 0x10) ? m_gb_spal1 : m_gb_spal0;
		uint16_t xindex = m_oam[oam_address + 1] - 8;

		if (m_oam[oam_address + 3] & 0x40)         /* flip y ? */
		{
			adr += (height - 1 - line + m_oam[oam_address]) * 2;
		}
		else
		{
			adr += (line - m_oam[oam_address]) * 2;
		}
		uint16_t data = (vram[adr + 1] << 8) | vram[adr];

		/* Find the palette to use */
		// If sprite started before the start of the line we may need to pick a different pal_map entry?
		uint8_t pal = m_sgb_pal_map[(xindex < 0) ? 0 : (xindex >> 3)][((yindex - SGB_YOFFSET) >> 3)] << 2;

		/* Offset to center of screen */
		xindex += SGB_XOFFSET;

		switch (m_oam[oam_address + 3] & 0xA0)
		{
		case 0xA0:                 /* priority is set (behind bgnd & wnd, flip x) */
			for (int bit = 0; bit < 8; bit++, xindex++)
			{
				int colour = ((data & 0x0100) ? 2 : 0) | ((data & 0x0001) ? 1 : 0);
				if ((xindex >= SGB_XOFFSET && xindex < SGB_XOFFSET + 160) && colour && !m_bg_zbuf[xindex - SGB_XOFFSET])
					plot_pixel(xindex, yindex, m_sgb_pal[pal + spal[colour]]);
				data >>= 1;
			}
			break;
		case 0x20:                 /* priority is not set (overlaps bgnd & wnd, flip x) */
			for (int bit = 0; bit < 8; bit++, xindex++)
			{
				int colour = ((data & 0x0100) ? 2 : 0) | ((data & 0x0001) ? 1 : 0);
				if ((xindex >= SGB_XOFFSET && xindex < SGB_XOFFSET + 160) && colour)
					plot_pixel(xindex, yindex, m_sgb_pal[pal + spal[colour]]);
				data >>= 1;
			}
			break;
		case 0x80:                 /* priority is set (behind bgnd & wnd, don't flip x) */
			for (int bit = 0; bit < 8; bit++, xindex++)
			{
				int colour = ((data & 0x8000) ? 2 : 0) | ((data & 0x0080) ? 1 : 0);
				if ((xindex >= SGB_XOFFSET && xindex < SGB_XOFFSET + 160) && colour && !m_bg_zbuf[xindex - SGB_XOFFSET])
					plot_pixel(xindex, yindex, m_sgb_pal[pal + spal[colour]]);
				data <<= 1;
			}
			break;
		case 0x00:                 /* priority is not set (overlaps bgnd & wnd, don't flip x) */
			for (int bit = 0; bit < 8; bit++, xindex++)
			{
				int colour = ((data & 0x8000) ? 2 : 0) | ((data & 0x0080) ? 1 : 0);
				if ((xindex >= SGB_XOFFSET && xindex < SGB_XOFFSET + 160) && colour)
					plot_pixel(xindex, yindex, m_sgb_pal[pal + spal[colour]]);
				data <<= 1;
			}
			break;
		}
	}
}


void sgb_ppu_device::refresh_border()
{
	uint16_t data, data2;
	uint8_t *tiles, *tiles2;

	for (uint16_t yidx = 0; yidx < 224; yidx++)
	{
		uint8_t *map = m_sgb_tile_map + ((yidx >> 3) * 64);
		uint16_t xindex = 0;

		for (uint16_t xidx = 0; xidx < 64; xidx += 2)
		{
			if (map[xidx + 1] & 0x80) /* Vertical flip */
				tiles = m_sgb_tile_data.get() + ((7 - (yidx % 8)) << 1);
			else /* No vertical flip */
				tiles = m_sgb_tile_data.get() + ((yidx % 8) << 1);
			tiles2 = tiles + 16;

			uint8_t pal = (map[xidx + 1] & 0x1C) >> 2;
			if (pal == 0)
				pal = 1;
			pal <<= 4;

			data = tiles[map[xidx] * 32] | (tiles[(map[xidx] * 32) + 1] << 8);
			data2 = tiles2[map[xidx] * 32] | (tiles2[(map[xidx] * 32) + 1] << 8);

			for (int i = 0; i < 8; i++)
			{
				uint8_t colour;
				if ((map[xidx + 1] & 0x40))  /* Horizontal flip */
				{
					colour = ((data  & 0x0001) ? 1 : 0) | ((data  & 0x0100) ? 2 : 0) |
							((data2 & 0x0001) ? 4 : 0) | ((data2 & 0x0100) ? 8 : 0);
					data >>= 1;
					data2 >>= 1;
				}
				else    /* No horizontal flip */
				{
					colour = ((data  & 0x0080) ? 1 : 0) | ((data  & 0x8000) ? 2 : 0) |
							((data2 & 0x0080) ? 4 : 0) | ((data2 & 0x8000) ? 8 : 0);
					data <<= 1;
					data2 <<= 1;
				}
				/* A slight hack below so we don't draw over the GB screen.
				 * Drawing there is allowed, but due to the way we draw the
				 * scanline, it can obscure the screen even when it shouldn't.
				 */
				if (!((yidx >= SGB_YOFFSET && yidx < SGB_YOFFSET + 144) &&
					(xindex >= SGB_XOFFSET && xindex < SGB_XOFFSET + 160)))
				{
					plot_pixel(xindex, yidx, m_sgb_pal[pal + colour]);
				}
				xindex++;
			}
		}
	}
}

void sgb_ppu_device::update_scanline(uint32_t cycles_to_go)
{
	g_profiler.start(PROFILER_VIDEO);

	if ((LCDSTAT & 0x03) == 0x03)
	{
		int l = 0;

		if (m_start_x < 0)
		{
			/* Window is enabled if the hardware says so AND the current scanline is
			 * within the window AND the window X coordinate is <=166 */
			m_layer[1].enabled = ((LCDCONT & WINDOW_ENABLED) && m_current_line >= m_window_y && m_window_x <= 166) ? 1 : 0;

			/* BG is enabled if the hardware says so AND (window_off OR (window_on
			 * AND window's X position is >=7 )) */
			m_layer[0].enabled = ((LCDCONT & BACKGROUND_ENABLED) && ((!m_layer[1].enabled) || (m_layer[1].enabled && m_window_x >= 7))) ? 1 : 0;

			if (m_layer[0].enabled)
			{
				m_layer[0].bgline = (SCROLLY + m_current_line) & 0xFF;
				m_layer[0].bg_map = m_vram.get() + m_gb_bgdtab_offs;
				m_layer[0].bg_tiles = m_vram.get() + m_gb_chrgen_offs;
				m_layer[0].xindex = SCROLLX >> 3;
				m_layer[0].xshift = SCROLLX & 7;
				m_layer[0].xstart = 0;
				m_layer[0].xend = 160;
			}

			if (m_layer[1].enabled)
			{
				int xpos;

				/* Window X position is offset by 7 so we'll need to adjust */
				xpos = m_window_x - 7;
				if (xpos < 0)
					xpos = 0;

				m_layer[1].bgline = m_window_lines_drawn;
				m_layer[1].bg_map = m_vram.get() + m_gb_wndtab_offs;
				m_layer[1].bg_tiles = m_vram.get() + m_gb_chrgen_offs;
				m_layer[1].xindex = 0;
				m_layer[1].xshift = 0;
				m_layer[1].xstart = xpos;
				m_layer[1].xend = 160;
				m_layer[0].xend = xpos;
			}
			m_start_x = 0;
		}

		if (cycles_to_go == 0)
		{
			/* Does this belong here? or should it be moved to the else block */
			/* Handle SGB mask */
			switch (m_sgb_window_mask)
			{
			case 1: /* Freeze screen */
				return;
			case 2: /* Blank screen (black) */
				{
					rectangle r(SGB_XOFFSET, SGB_XOFFSET + 160-1, SGB_YOFFSET, SGB_YOFFSET + 144 - 1);
					m_bitmap.fill(0, r);
				}
				return;
			case 3: /* Blank screen (white - or should it be color 0?) */
				{
					rectangle r(SGB_XOFFSET, SGB_XOFFSET + 160 - 1, SGB_YOFFSET, SGB_YOFFSET + 144 - 1);
					m_bitmap.fill(32767, r);
				}
				return;
			}

			/* Draw the "border" if we're on the first line */
			if (m_current_line == 0)
			{
				refresh_border();
			}
		}
		if (cycles_to_go < 160)
		{
			m_end_x = std::min(int(160 - cycles_to_go),160);

			/* if background or screen disabled clear line */
			if (!(LCDCONT & BACKGROUND_ENABLED))
			{
				rectangle r(SGB_XOFFSET, SGB_XOFFSET + 160 - 1, m_current_line + SGB_YOFFSET, m_current_line + SGB_YOFFSET);
				m_bitmap.fill(0, r);
			}
			while (l < 2)
			{
				uint8_t   xindex, sgb_palette, *map, *tiles;
				uint16_t  data;
				int i, tile_index;

				if (!m_layer[l].enabled)
				{
					l++;
					continue;
				}
				map = m_layer[l].bg_map + ((m_layer[l].bgline << 2) & 0x3E0);
				tiles = m_layer[l].bg_tiles + ((m_layer[l].bgline & 7) << 1);
				xindex = m_start_x;
				if (xindex < m_layer[l].xstart)
					xindex = m_layer[l].xstart;
				i = m_end_x;
				if (i > m_layer[l].xend)
					i = m_layer[l].xend;
				i = i - xindex;

				tile_index = (map[m_layer[l].xindex] ^ m_gb_tile_no_mod) * 16;
				data = tiles[tile_index] | (tiles[tile_index + 1] << 8);
				data <<= m_layer[l].xshift;

				while (i > 0)
				{
					/* Figure out which palette we're using */
					assert(((m_end_x - i) >> 3) >= 0 && ((m_end_x - i) >> 3) < std::size(m_sgb_pal_map));
					sgb_palette = m_sgb_pal_map[(m_end_x - i) >> 3][m_current_line >> 3] << 2;

					while ((m_layer[l].xshift < 8) && i)
					{
						int colour = ((data & 0x8000) ? 2 : 0) | ((data & 0x0080) ? 1 : 0);
						plot_pixel(xindex + SGB_XOFFSET, m_current_line + SGB_YOFFSET, m_sgb_pal[sgb_palette + m_gb_bpal[colour]]);
						m_bg_zbuf[xindex] = colour;
						xindex++;
						data <<= 1;
						m_layer[l].xshift++;
						i--;
					}
					if (m_layer[l].xshift == 8)
					{
						/* Take possible changes to SCROLLY into account */
						if (l == 0)
						{
							m_layer[0].bgline = (SCROLLY + m_current_line) & 0xFF;
							map = m_layer[l].bg_map + ((m_layer[l].bgline << 2) & 0x3E0);
							tiles = m_layer[l].bg_tiles + ((m_layer[l].bgline & 7) << 1);
						}

						m_layer[l].xindex = (m_layer[l].xindex + 1) & 31;
						m_layer[l].xshift = 0;
						tile_index = (map[m_layer[l].xindex] ^ m_gb_tile_no_mod) * 16;
						data = tiles[tile_index] | (tiles[tile_index + 1] << 8);
					}
				}
				l++;
			}
			if ((m_end_x == 160) && (LCDCONT & SPRITES_ENABLED))
			{
				update_sprites();
			}
			m_start_x = m_end_x;
		}
	}
	else
	{
		if (!(LCDCONT & ENABLED))
		{
			/* if screen disabled clear line */
			if (m_previous_line != m_current_line)
			{
				/* Also refresh border here??? */
				if (m_current_line < 144)
				{
					rectangle r(SGB_XOFFSET, SGB_XOFFSET + 160 - 1, m_current_line + SGB_YOFFSET, m_current_line + SGB_YOFFSET);
					m_bitmap.fill(0, r);
				}
				m_previous_line = m_current_line;
			}
		}
	}

	g_profiler.stop();
}

/* --- Game Boy Color Specific --- */

void cgb_ppu_device::update_sprites()
{
	uint8_t height, tilemask, line;
	int yindex;

	if (LCDCONT & LARGE_SPRITES)
	{
		height = 16;
		tilemask = 0xFE;
	}
	else
	{
		height = 8;
		tilemask = 0xFF;
	}

	yindex = m_current_line;
	line = m_current_line + 16;

	for (int i = m_sprCount - 1; i >= 0; i--)
	{
		const uint16_t oam_address = m_sprite[i] * 4;
		uint8_t pal;
		int xindex = m_oam[oam_address + 1] - 8;
		uint16_t adr = ((m_oam[oam_address + 3] & 0x08) << 10) + (m_oam[oam_address + 2] & tilemask) * 16;

		if (xindex < -7 || xindex > 160)
		{
			continue;
		}

		/* Handle mono mode for GB games */
		if (!m_gbc_mode)
			pal = (m_oam[oam_address + 3] & 0x10) ? 4 : 0;
		else
			pal = ((m_oam[oam_address + 3] & 0x7) * 4);

		if (m_oam[oam_address + 3] & 0x40)         /* flip y ? */
		{
			adr += (height - 1 - line + m_oam[oam_address]) * 2;
		}
		else
		{
			adr += (line - m_oam[oam_address]) * 2;
		}

		uint16_t data = (m_vram[adr + 1] << 8) | m_vram[adr];

		switch (m_oam[oam_address + 3] & 0xA0)
		{
		case 0xA0:                 /* priority is set (behind bgnd & wnd, flip x) */
			for (int bit = 0; bit < 8; bit++, xindex++)
			{
				int colour = ((data & 0x0100) ? 2 : 0) | ((data & 0x0001) ? 1 : 0);
				if (colour && !m_bg_zbuf[xindex] && xindex >= 0 && xindex < 160)
				{
					if (!m_gbc_mode)
						colour = pal ? m_gb_spal1[colour] : m_gb_spal0[colour];
					plot_pixel(xindex, yindex, m_cgb_spal[pal + colour]);
				}
				data >>= 1;
			}
			break;
		case 0x20:                 /* priority is not set (overlaps bgnd & wnd, flip x) */
			for (int bit = 0; bit < 8; bit++, xindex++)
			{
				int colour = ((data & 0x0100) ? 2 : 0) | ((data & 0x0001) ? 1 : 0);
				if ((m_bg_zbuf[xindex] & 0x80) && (m_bg_zbuf[xindex] & 0x7f) && (LCDCONT & BACKGROUND_ENABLED))
					colour = 0;
				if (colour && xindex >= 0 && xindex < 160)
				{
					if (!m_gbc_mode)
						colour = pal ? m_gb_spal1[colour] : m_gb_spal0[colour];
					plot_pixel(xindex, yindex, m_cgb_spal[pal + colour]);
				}
				data >>= 1;
			}
			break;
		case 0x80:                 /* priority is set (behind bgnd & wnd, don't flip x) */
			for (int bit = 0; bit < 8; bit++, xindex++)
			{
				int colour = ((data & 0x8000) ? 2 : 0) | ((data & 0x0080) ? 1 : 0);
				if (colour && !m_bg_zbuf[xindex] && xindex >= 0 && xindex < 160)
				{
					if (!m_gbc_mode)
						colour = pal ? m_gb_spal1[colour] : m_gb_spal0[colour];
					plot_pixel(xindex, yindex, m_cgb_spal[pal + colour]);
				}
				data <<= 1;
			}
			break;
		case 0x00:                 /* priority is not set (overlaps bgnd & wnd, don't flip x) */
			for (int bit = 0; bit < 8; bit++, xindex++)
			{
				if (xindex >= 0 && xindex < 160)
				{
					int colour = ((data & 0x8000) ? 2 : 0) | ((data & 0x0080) ? 1 : 0);
					if ((m_bg_zbuf[xindex] & 0x80) && (m_bg_zbuf[xindex] & 0x7f) && (LCDCONT & BACKGROUND_ENABLED))
						colour = 0;
					if (colour)
					{
						if (!m_gbc_mode)
							colour = pal ? m_gb_spal1[colour] : m_gb_spal0[colour];
						plot_pixel(xindex, yindex, m_cgb_spal[pal + colour]);
					}
				}
				data <<= 1;
			}
			break;
		}
	}
}

void cgb_ppu_device::update_scanline(uint32_t cycles_to_go)
{
	g_profiler.start(PROFILER_VIDEO);

	if ((LCDSTAT & 0x03) == 0x03)
	{
		int l = 0;

		if (m_start_x < 0)
		{
			/* Window is enabled if the hardware says so AND the current scanline is
			 * within the window AND the window X coordinate is <=166 */
			m_layer[1].enabled = ((LCDCONT & WINDOW_ENABLED) && (m_current_line >= m_window_y) && (m_window_x <= 166)) ? 1 : 0;

			/* BG is enabled if the hardware says so AND (window_off OR (window_on
			 * AND window's X position is >=7 )) */
			m_layer[0].enabled = ((LCDCONT & BACKGROUND_ENABLED) && ((!m_layer[1].enabled) || (m_layer[1].enabled && (m_window_x >= 7)))) ? 1 : 0;

			if (m_layer[0].enabled)
			{
				m_layer[0].bgline = (SCROLLY + m_current_line) & 0xFF;
				m_layer[0].bg_map = m_vram.get() + m_gb_bgdtab_offs;
				m_layer[0].gbc_map = m_vram.get() + m_gbc_bgdtab_offs;
				m_layer[0].xindex = SCROLLX >> 3;
				m_layer[0].xshift = SCROLLX & 7;
				m_layer[0].xstart = 0;
				m_layer[0].xend = 160;
			}

			if (m_layer[1].enabled)
			{
				int xpos;

				/* Window X position is offset by 7 so we'll need to adust */
				xpos = m_window_x - 7;
				if (xpos < 0)
					xpos = 0;

				m_layer[1].bgline = m_window_lines_drawn;
				m_layer[1].bg_map = m_vram.get() + m_gb_wndtab_offs;
				m_layer[1].gbc_map = m_vram.get() + m_gbc_wndtab_offs;
				m_layer[1].xindex = 0;
				m_layer[1].xshift = 0;
				m_layer[1].xstart = xpos;
				m_layer[1].xend = 160;
				m_layer[0].xend = xpos;
			}
			m_start_x = 0;
		}

		if (cycles_to_go < 160)
		{
			m_end_x = std::min(int(160 - cycles_to_go), 160);
			/* Draw empty line when the background is disabled */
			if (!(LCDCONT & BACKGROUND_ENABLED))
			{
				rectangle r(m_start_x, m_end_x - 1, m_current_line, m_current_line);
				m_bitmap.fill((!m_gbc_mode) ? 0 : 32767, r);
			}
			while (l < 2)
			{
				uint8_t   xindex, *map, *tiles, *gbcmap;
				uint16_t  data;
				int i, tile_index;

				if (!m_layer[l].enabled)
				{
					l++;
					continue;
				}
				map = m_layer[l].bg_map + ((m_layer[l].bgline << 2) & 0x3E0);
				gbcmap = m_layer[l].gbc_map + ((m_layer[l].bgline << 2) & 0x3E0);
				tiles = (gbcmap[m_layer[l].xindex] & 0x08) ? (m_vram.get() + m_gbc_chrgen_offs) : (m_vram.get() + m_gb_chrgen_offs);

				/* Check for vertical flip */
				if (gbcmap[m_layer[l].xindex] & 0x40)
				{
					tiles += ((7 - (m_layer[l].bgline & 0x07)) << 1);
				}
				else
				{
					tiles += ((m_layer[l].bgline & 0x07) << 1);
				}
				xindex = m_start_x;
				if (xindex < m_layer[l].xstart)
					xindex = m_layer[l].xstart;
				i = m_end_x;
				if (i > m_layer[l].xend)
					i = m_layer[l].xend;
				i = i - xindex;

				tile_index = (map[m_layer[l].xindex] ^ m_gb_tile_no_mod) * 16;
				data = tiles[tile_index] | (tiles[tile_index + 1] << 8);
				/* Check for horinzontal flip */
				if (gbcmap[m_layer[l].xindex] & 0x20)
				{
					data >>= m_layer[l].xshift;
				}
				else
				{
					data <<= m_layer[l].xshift;
				}

				while (i > 0)
				{
					while ((m_layer[l].xshift < 8) && i)
					{
						int colour;
						/* Check for horinzontal flip */
						if (gbcmap[m_layer[l].xindex] & 0x20)
						{
							colour = ((data & 0x0100) ? 2 : 0) | ((data & 0x0001) ? 1 : 0);
							data >>= 1;
						}
						else
						{
							colour = ((data & 0x8000) ? 2 : 0) | ((data & 0x0080) ? 1 : 0);
							data <<= 1;
						}
						plot_pixel(xindex, m_current_line, m_cgb_bpal[(!m_gbc_mode) ? m_gb_bpal[colour] : (((gbcmap[m_layer[l].xindex] & 0x07) * 4) + colour)]);
						m_bg_zbuf[xindex] = colour + (gbcmap[m_layer[l].xindex] & 0x80);
						xindex++;
						m_layer[l].xshift++;
						i--;
					}
					if (m_layer[l].xshift == 8)
					{
						/* Take possible changes to SCROLLY into account */
						if (l == 0)
						{
							m_layer[0].bgline = (SCROLLY + m_current_line) & 0xFF;
							map = m_layer[l].bg_map + ((m_layer[l].bgline << 2) & 0x3E0);
							gbcmap = m_layer[l].gbc_map + ((m_layer[l].bgline << 2) & 0x3E0);
						}

						m_layer[l].xindex = (m_layer[l].xindex + 1) & 31;
						m_layer[l].xshift = 0;
						tiles = (gbcmap[m_layer[l].xindex] & 0x08) ? (m_vram.get() + m_gbc_chrgen_offs) : (m_vram.get() + m_gb_chrgen_offs);

						/* Check for vertical flip */
						if (gbcmap[m_layer[l].xindex] & 0x40)
						{
							tiles += ((7 - (m_layer[l].bgline & 0x07)) << 1);
						}
						else
						{
							tiles += ((m_layer[l].bgline & 0x07) << 1);
						}
						tile_index = (map[m_layer[l].xindex] ^ m_gb_tile_no_mod) * 16;
						data = tiles[tile_index] | (tiles[tile_index + 1] << 8);
					}
				}
				l++;
			}
			if (m_end_x == 160 && (LCDCONT & SPRITES_ENABLED))
			{
				update_sprites();
			}
			m_start_x = m_end_x;
		}
	}
	else
	{
		if (!(LCDCONT & ENABLED))
		{
			/* Draw an empty line when LCD is disabled */
			if (m_previous_line != m_current_line)
			{
				if (m_current_line < 144)
				{
					const rectangle &r1 = screen().visible_area();
					rectangle r(r1.min_x, r1.max_x, m_current_line, m_current_line);
					m_bitmap.fill((!m_gbc_mode) ? 0 : 32767 , r);
				}
				m_previous_line = m_current_line;
			}
		}
	}

	g_profiler.stop();
}


uint32_t dmg_ppu_device::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	copybitmap(bitmap, m_bitmap, 0, 0, 0, 0, cliprect);
	return 0;
}


void dmg_ppu_device::increment_scanline()
{
	m_current_line = (m_current_line + 1) % 154;
	if (LCDCONT & ENABLED)
	{
		CURLINE = m_current_line;
	}
	if (m_current_line == 0)
	{
		m_window_lines_drawn = 0;
	}
}


TIMER_CALLBACK_MEMBER(dmg_ppu_device::update_tick)
{
	update_state();
}


void dmg_ppu_device::update_oam_dma_state(uint64_t cycles)
{
	if (m_oam_dma_cycles_left > 0)
	{
		if (cycles >= m_oam_dma_cycles_left)
		{
			m_oam_dma_cycles_left = 0;
			m_oam_dma_processing = false;
			// TODO: reenable real program map...
		}
		else
		{
			m_oam_dma_cycles_left -= cycles;
		}
	}

	if (m_oam_dma_start_cycles > 0)
	{
		if (cycles >= m_oam_dma_start_cycles)
		{
			for (int i = 0; i < 0xA0; i++)
			{
				m_oam[i] = m_program_space->read_byte(m_oam_dma_source_address + i);
			}

			m_oam_dma_start_cycles = 0;
			m_oam_dma_cycles_left = 160 * 4;
			m_oam_dma_processing = true;
			// TODO: all reads should start to return 0xFF? from here
		}
		else
		{
			m_oam_dma_start_cycles -= cycles;
		}
	}
}


static const char* state_to_string(int state)
{
	switch (state)
	{
	case GB_LCD_STATE_LYXX_M3:
		return "GB_LCD_STATE_LYXX_M3";
	case GB_LCD_STATE_LYXX_M3_2:
		return "GB_LCD_STATE_LYXX_M3_2";
	case GB_LCD_STATE_LYXX_PRE_M0:
		return "GB_LCD_STATE_LYXX_PRE_M0";
	case GB_LCD_STATE_LYXX_M0:
		return "GB_LCD_STATE_LYXX_M0";
	case GB_LCD_STATE_LYXX_M0_2:
		return "GB_LCD_STATE_LYXX_M0_2";
	case GB_LCD_STATE_LYXX_M0_GBC_PAL:
		return "GB_LCD_STATE_LYXX_M0_GBC_PAL";
	case GB_LCD_STATE_LYXX_M0_PRE_INC:
		return "GB_LCD_STATE_LYXX_M0_PRE_INC";
	case GB_LCD_STATE_LYXX_M0_INC:
		return "GB_LCD_STATE_LYXX_M0_INC";
	case GB_LCD_STATE_LY00_M2:
		return "GB_LCD_STATE_LY00_M2";
	case GB_LCD_STATE_LY00_M2_WND:
		return "GB_LCD_STATE_LY00_M2_WND";
	case GB_LCD_STATE_LYXX_M2_WND:
		return "GB_LCD_STATE_LYXX_M2_WND";
	case GB_LCD_STATE_LYXX_M2:
		return "GB_LCD_STATE_LYXX_M2";
	case GB_LCD_STATE_LY9X_M1:
		return "GB_LCD_STATE_LY9X_M1";
	case GB_LCD_STATE_LY9X_M1_INC:
		return "GB_LCD_STATE_LY9X_M1_INC";
	case GB_LCD_STATE_LY00_M1:
		return "GB_LCD_STATE_LY00_M1";
	case GB_LCD_STATE_LY00_M1_1:
		return "GB_LCD_STATE_LY00_M1_1";
	case GB_LCD_STATE_LY00_M1_2:
		return "GB_LCD_STATE_LY00_M1_2";
	case GB_LCD_STATE_LY00_M0:
		return "GB_LCD_STATE_LY00_M0";
	default:
		return "unknown state";
	}
}


void dmg_ppu_device::update_state()
{
	if (m_updating_state)
	{
		return;
	}

	m_updating_state = true;

	attotime now = machine().time();

	assert(now >= m_last_updated);

	uint64_t cycles = m_lr35902->attotime_to_cycles(now - m_last_updated);

	update_oam_dma_state(cycles);

	if (LCDCONT & ENABLED)
	{
		LOG("m_cycles_left = %u, cycles = %u, CURLINE = %u, m_next_state = %s\n", m_cycles_left, cycles, CURLINE, state_to_string(m_next_state));

		if (m_cycles_left > 0)
		{
			if (m_state == GB_LCD_STATE_LYXX_M3 || m_state == GB_LCD_STATE_LYXX_M3_2 || m_state == GB_LCD_STATE_LYXX_M0 || m_state == GB_LCD_STATE_LYXX_M0_2)
			{
				// Execute <cycles> M3 cycles
				if (m_enable_experimental_engine)
				{
					update_line_state(cycles);
				}
			}

			if (cycles >= m_cycles_left) {
				cycles -= m_cycles_left;
				m_cycles_left = 0;
			}
			else
			{
				m_cycles_left -= cycles;
				cycles = 0;
			}
		}

		while (m_cycles_left == 0)
		{
			uint16_t state_cycles = 0;

			m_state = m_next_state;

			switch (m_state)
			{
			case GB_LCD_STATE_LYXX_PRE_M0:  /* Just before switching to mode 0 */
				m_next_state = GB_LCD_STATE_LYXX_M0;
				state_cycles = 4;
				break;

			case GB_LCD_STATE_LYXX_M0:      /* Switch to mode 0 */
				/* update current scanline */
				update_scanline(m_lr35902->attotime_to_cycles(m_lcd_timer->remaining()));
				/* Increment the number of window lines drawn if enabled */
				if (m_layer[1].enabled)
				{
					if (!m_enable_experimental_engine)
					{
						m_window_lines_drawn++;
					}
				}
				m_previous_line = m_current_line;
				/* Set Mode 0 lcdstate */
				LCDSTAT &= 0xFC;
				m_oam_locked = UNLOCKED;
				m_oam_locked_reading = UNLOCKED;
				m_vram_locked = UNLOCKED;
				m_next_state = GB_LCD_STATE_LYXX_M0_2;
				state_cycles = 1;
				break;

			case GB_LCD_STATE_LYXX_M0_2:
				m_stat_mode0_int = (LCDSTAT & MODE_0_INT_ENABLED) ? true : false;
				check_stat_irq();
				m_mode = 0;
				m_next_state = GB_LCD_STATE_LYXX_M0_INC;
				state_cycles = 200 - 1 + 3 - m_scrollx_adjust - m_sprite_cycles - m_window_cycles;
				break;

			case GB_LCD_STATE_LYXX_M0_INC:  /* Increment LY, stay in M0 for 4 more cycles */
				increment_scanline();
				m_window_y = WNDPOSY;
				m_stat_lyc_int_prev = m_stat_lyc_int;
				m_stat_lyc_int = false;
				if (CURLINE < 144)
				{
					m_stat_mode0_int = false;
				}
				m_stat_mode2_int = (LCDSTAT & MODE_2_INT_ENABLED) ? true : false;
				check_stat_irq();

				m_stat_lyc_int = ((CMPLINE == CURLINE) && (LCDSTAT & LY_LYC_INT_ENABLED)) ? true : false;
				LOG("GB_LCD_STATE_LYXX_M0_INC: CMPLINE = %02x, CURLINE = %02x, LCDSTAT = %02x, m_stat_lyc_int = %s\n", CMPLINE, CURLINE, LCDSTAT, m_stat_lyc_int ? "true":"false");
				/* Reset LY==LYC STAT bit */
				LCDSTAT &= ~LY_LYC_FLAG;
				/* Check if we're going into VBlank next */
				if (CURLINE == 144)
				{
					m_next_state = GB_LCD_STATE_LY9X_M1;
					state_cycles = 4;
				}
				else
				{
					m_next_state = GB_LCD_STATE_LYXX_M2;
					m_oam_locked_reading = LOCKED;
					state_cycles = 4;
				}
				break;

			case GB_LCD_STATE_LY00_M2:      /* Switch to mode 2 on line #0 */
				/* Set Mode 2 lcdstate */
				m_mode = 2;
				LCDSTAT = (LCDSTAT & 0xFC) | 0x02;
				m_oam_locked = LOCKED;
				/* Generate lcd interrupt if requested */
				m_stat_mode1_int = false;
				m_stat_mode2_int = (LCDSTAT & MODE_2_INT_ENABLED) ? true : false;
				check_stat_irq();
				/* Check for regular compensation of x-scroll register */
				if (!m_enable_experimental_engine)
				{
					m_scrollx_adjust = SCROLLX & 0x07;
				}
				/* Mode 2 lasts approximately 80 clock cycles */
				m_next_state = GB_LCD_STATE_LYXX_M3;
				clear_line_state();
				select_sprites();
//              if (!m_enable_experimental_engine)
				{
					m_window_y = WNDPOSY;
				}
				state_cycles = 80;
				state_cycles = 8;
				m_next_state = GB_LCD_STATE_LY00_M2_WND;
				break;

			case GB_LCD_STATE_LY00_M2_WND:
				// Check window active for current/previous line
				if ((LCDCONT & WINDOW_ENABLED) && m_current_line == m_window_y)
				{
					m_line.window_should_trigger = true;
				}
				LOG("window should trigger = %s, m_current_line = %u, m_window_y = %u\n", m_line.window_should_trigger ? "true" : "false", m_current_line, m_window_y);

				state_cycles = 80 - 8;
				m_next_state = GB_LCD_STATE_LYXX_M3;
				break;

			case GB_LCD_STATE_LYXX_M2:      /* Switch to mode 2 */
				m_stat_mode0_int = false;
				check_stat_irq();
				/* Update STAT register to the correct state */
				m_mode = 2;
				LCDSTAT = (LCDSTAT & 0xFC) | 0x02;
				m_oam_locked = LOCKED;
				/* Check if LY==LYC STAT bit should be set */
				if (CURLINE == CMPLINE)
				{
					LCDSTAT |= LY_LYC_FLAG;
				}
				/* Check for regular compensation of x-scroll register */
				if (!m_enable_experimental_engine)
				{
					m_scrollx_adjust = SCROLLX & 0x07;
				}
				/* Mode 2 last for approximately 80 clock cycles */
				m_next_state = GB_LCD_STATE_LYXX_M3;
				clear_line_state();
				select_sprites();
				if (!m_enable_experimental_engine)
				{
					m_window_y = WNDPOSY;
				}
				state_cycles = 80;
				m_next_state = GB_LCD_STATE_LYXX_M2_WND;
				state_cycles = 8;
				break;

			case GB_LCD_STATE_LYXX_M2_WND:
				// Check window active for current/previous line
				if ((LCDCONT & WINDOW_ENABLED) && m_current_line == m_window_y + 1)
				{
					m_line.window_should_trigger = true;
				}
				LOG("window should trigger = %s, m_current_line = %u, m_window_y = %u\n", m_line.window_should_trigger ? "true" : "false", m_current_line, m_window_y);
				m_next_state = GB_LCD_STATE_LYXX_M3;
				state_cycles = 80 - 8;
				break;

			case GB_LCD_STATE_LYXX_M3:      /* Switch to mode 3 */
				for (int i = 0; i < std::size(m_line.window_start_y); i++)
				{
					m_line.window_start_y[i] = WNDPOSY;
					m_line.window_start_x[i] = WNDPOSX;
				}
				m_line.window_start_y_index = 0;
				for (int i = 0; i < std::size(m_line.window_enable); i++)
				{
					m_line.window_enable[i] = LCDCONT;
				}
				m_line.window_enable_index = 0;
				m_stat_mode2_int = false;
				check_stat_irq();
				/* Set Mode 3 lcdstate */
				m_mode = 3;
				LCDSTAT = (LCDSTAT & 0xFC) | 0x03;
				// Normally OAM is already locked by mode 2, except when the lcd unit is just switched on.
				m_oam_locked = LOCKED;
				m_vram_locked = LOCKED;
				/* Check for compensations of x-scroll register */
				/* Mode 3 lasts for approximately 172+cycles needed to handle sprites */
				if (m_enable_experimental_engine)
				{
					m_next_state = GB_LCD_STATE_LYXX_M0;
					state_cycles = 4 - 3 + 168 + m_sprite_cycles;
				}
				else
				{
					// old
					m_next_state = GB_LCD_STATE_LYXX_PRE_M0;
					state_cycles = 168 + m_scrollx_adjust + m_sprite_cycles;
					// new
// WX write at 164 cycles left still taken into account
// WX write at 160 cycles left not taken into account
					m_next_state = GB_LCD_STATE_LYXX_M3_2;
					state_cycles = 12;
					m_start_x = -1;
				}
				break;

			case GB_LCD_STATE_LYXX_M3_2:
				m_window_x = WNDPOSX;
				if (!m_enable_experimental_engine)
				{
					calculate_window_cycles();
				}
				m_next_state = GB_LCD_STATE_LYXX_M0;
				state_cycles = 4 - 3 + 168 - 12 + m_scrollx_adjust + m_sprite_cycles + m_window_cycles;
				break;

			case GB_LCD_STATE_LY9X_M1:      /* Switch to or stay in mode 1 */
				m_stat_lyc_int = ((CMPLINE == CURLINE) && (LCDSTAT & LY_LYC_INT_ENABLED)) ? true : false;
				m_stat_mode2_int = false;
				m_stat_mode0_int = false;
				if (CURLINE == 144)
				{
					/* Trigger VBlank interrupt */
					m_lr35902->set_input_line(lr35902_cpu_device::VBL_INT, ASSERT_LINE);
					// Make sure the state is updated during the current timeslice in case it is read.
					m_lr35902->execute_set_input(lr35902_cpu_device::VBL_INT, ASSERT_LINE);
					/* Set VBlank lcdstate */
					m_mode = 1;
					LCDSTAT = (LCDSTAT & 0xFC) | 0x01;
					/* Trigger LCD interrupt if requested */
					m_stat_mode1_int = (LCDSTAT & MODE_1_INT_ENABLED) ? true : false;
				}
				check_stat_irq();
				/* Check if LY==LYC STAT bit should be set */
				if (CURLINE == CMPLINE)
				{
					LCDSTAT |= LY_LYC_FLAG;
				}
				m_next_state = GB_LCD_STATE_LY9X_M1_INC;
				state_cycles = 452;
				break;

			case GB_LCD_STATE_LY9X_M1_INC:      /* Increment scanline counter */
				increment_scanline();
				LOG("GB_LCD_STATE_LY9X_M1_INC: m_stat_lyc_int = %s\n", m_stat_lyc_int ? "true" : "false");
				/* Reset LY==LYC STAT bit */
				LCDSTAT &= ~LY_LYC_FLAG;
				if (m_current_line == 153)
				{
					m_next_state = GB_LCD_STATE_LY00_M1;
					state_cycles = 4;
				}
				else
				{
					m_next_state = GB_LCD_STATE_LY9X_M1;
					state_cycles = 4;
				}
				break;

			case GB_LCD_STATE_LY00_M1:      /* we stay in VBlank but current line counter should already be incremented */
				/* Check LY=LYC for line #153 */
				if (CURLINE == CMPLINE)
				{
					LCDSTAT |= LY_LYC_FLAG;
				}
				else
				{
					LCDSTAT &= ~LY_LYC_FLAG;
				}
				m_stat_lyc_int = ((CMPLINE == CURLINE) && (LCDSTAT & LY_LYC_INT_ENABLED)) ? true : false;
				check_stat_irq();

				increment_scanline();
				m_next_state = GB_LCD_STATE_LY00_M1_1;
				state_cycles = 4;
				break;

			case GB_LCD_STATE_LY00_M1_1:
				LCDSTAT &= ~LY_LYC_FLAG;
				m_next_state = GB_LCD_STATE_LY00_M1_2;
				state_cycles = 4;
				break;

			case GB_LCD_STATE_LY00_M1_2:    /* Rest of line #0 during VBlank */
				m_frame_window_active = false;
				m_stat_lyc_int = ((CMPLINE == CURLINE) && (LCDSTAT & LY_LYC_INT_ENABLED)) ? true : false;
				check_stat_irq();
				if (CURLINE == CMPLINE)
				{
					LCDSTAT |= LY_LYC_FLAG;
				}
				m_next_state = GB_LCD_STATE_LY00_M0;
				state_cycles = 444;
				break;

			case GB_LCD_STATE_LY00_M0:      /* The STAT register seems to go to 0 for about 4 cycles */
				m_window_y = WNDPOSY;
				/* Set Mode 0 lcdstat */
				LCDSTAT = (LCDSTAT & 0xFC);
				m_next_state = GB_LCD_STATE_LY00_M2;
				state_cycles = 4;
				break;
			}
			assert(state_cycles > 0);

			if (m_state == GB_LCD_STATE_LYXX_M3 || m_state == GB_LCD_STATE_LYXX_M3_2 || m_state == GB_LCD_STATE_LYXX_M0 || m_state == GB_LCD_STATE_LYXX_M0)
			{
				// Execute <cycles> M3 cycles
				if (m_enable_experimental_engine)
				{
					update_line_state(cycles);
				}
			}

			if (cycles >= state_cycles)
			{
				cycles -= state_cycles;
				m_cycles_left = 0;
			}
			else
			{
				m_cycles_left = state_cycles - cycles;
			}
		}
	}
	else
	{
		increment_scanline();
		if (m_current_line < 144)
		{
			// Force draw of an empty line
			update_scanline(0);
		}
		m_cycles_left = 456;
	}
	assert(m_cycles_left > 0);

	m_last_updated = machine().time();

	int next_cycles = m_cycles_left;

	if (m_oam_dma_start_cycles > 0 && m_oam_dma_start_cycles < next_cycles)
	{
		next_cycles = m_oam_dma_start_cycles;
	}

	if (m_oam_dma_cycles_left > 0 && m_oam_dma_cycles_left < next_cycles)
	{
		next_cycles = m_oam_dma_cycles_left;
	}

	m_lcd_timer->adjust(m_lr35902->cycles_to_attotime(next_cycles));

	m_updating_state = false;
}


// CGB specific code

void cgb_ppu_device::update_hdma_state(uint64_t cycles)
{
	if (m_hdma_cycles_to_start > 0)
	{
		if (cycles >= m_hdma_cycles_to_start)
		{
			m_hdma_cycles_to_start = 0;
			hdma_trans_execute();
		}
		else
		{
			m_hdma_cycles_to_start -= cycles;
		}
	}
}


void cgb_ppu_device::hdma_trans(uint16_t length)
{
	LOG("hdma_trans\n");
	m_hdma_length = length;
	m_hdma_cycles_to_start = 4;
	update_state();
}


void cgb_ppu_device::hdma_trans_execute()
{
	LOG("hdma_trans_execute\n");
	uint16_t length = m_hdma_length;
	uint16_t src, dst;

	src = (HDMA1 << 8) | (HDMA2 & 0xF0);
	// 102 Dalmatians uses destination 0000 and expects data to be DMAed.
	dst = 0x8000 | (HDMA3 << 8) | (HDMA4 & 0xF0);

	//LOG("length = %04x, src = %04x, dst = %04x\n", length, src, dst);
	while (length > 0)
	{
		if (dst & 0x8000)
		{
			uint16_t src_high = src & 0xF000;
			uint8_t source = 0xFF;
			if (src_high < 0x8000 || (src_high >= 0xA000 && src_high < 0xE000))
			{
				source = m_program_space->read_byte(src);
			}
			m_program_space->write_byte(dst & 0x9FFF, source);
		}
		src++;
		dst++;

		length--;
	}
	HDMA1 = src >> 8;
	HDMA2 = src & 0xFF;
	HDMA3 = dst >> 8;
	HDMA4 = dst & 0xFF;
	HDMA5--;
	if ((HDMA5 & 0x7f) == 0x7f)
	{
		HDMA5 = 0xff;
		m_hdma_enabled = 0;
	}

	m_lr35902->dma_cycles_to_burn(4 + m_hdma_length * 2);
}


void cgb_ppu_device::update_state()
{
	if (m_updating_state)
	{
		return;
	}

	m_updating_state = true;

	attotime now = machine().time();

	uint64_t cycles = m_lr35902->attotime_to_cycles(now - m_last_updated);

	update_oam_dma_state(cycles);
	update_hdma_state(cycles);

	if (LCDCONT & ENABLED)
	{
		LOG("m_cycles_left = %d, cycles = %d, m_next_state = %s\n", m_cycles_left, cycles, state_to_string(m_next_state));

		if (m_cycles_left > 0)
		{
			if (cycles >= m_cycles_left) {
				cycles -= m_cycles_left;
				m_cycles_left = 0;
			}
			else
			{
				m_cycles_left -= cycles;
				cycles = 0;
			}
		}

		while (m_cycles_left == 0)
		{
			uint16_t state_cycles = 0;

			m_state = m_next_state;

			switch (m_state)
			{
			case GB_LCD_STATE_LYXX_PRE_M0:  /* Just before switching to mode 0 */
				m_next_state = GB_LCD_STATE_LYXX_M0;
				state_cycles = 4;
				break;

			case GB_LCD_STATE_LYXX_M0:      /* Switch to mode 0 */
				m_stat_mode0_int = (LCDSTAT & MODE_0_INT_ENABLED) ? true : false;
				/* update current scanline */
				update_scanline(m_lr35902->attotime_to_cycles(m_lcd_timer->remaining()));
				/* Increment the number of window lines drawn if enabled */
				if (m_layer[1].enabled)
				{
					m_window_lines_drawn++;
				}
				m_previous_line = m_current_line;
				/* Set Mode 0 lcdstate */
				m_mode = 0;
				LCDSTAT &= 0xFC;
				m_oam_locked = UNLOCKED;
				m_oam_locked_reading = UNLOCKED;
				m_vram_locked = UNLOCKED;
				//m_pal_locked = UNLOCKED;
				m_next_state = GB_LCD_STATE_LYXX_M0_GBC_PAL;
				state_cycles = 4;
				m_next_state = GB_LCD_STATE_LYXX_M0_2;
				state_cycles = 1;
				break;

			case GB_LCD_STATE_LYXX_M0_2:
				check_stat_irq();
				m_next_state = GB_LCD_STATE_LYXX_M0_GBC_PAL;
				state_cycles = 3;
				break;

			case GB_LCD_STATE_LYXX_M0_GBC_PAL:
				m_pal_locked = UNLOCKED;
				/* Check for HBLANK DMA */
				if (m_hdma_enabled)
				{
					hdma_trans(0x10);
				}
				else
				{
					m_hdma_possible = 1;
				}
				m_next_state = GB_LCD_STATE_LYXX_M0_PRE_INC;
				state_cycles = 192 + 3 - m_scrollx_adjust - m_sprite_cycles - m_window_cycles;
				break;

			case GB_LCD_STATE_LYXX_M0_PRE_INC:  /* Just before incrementing the line counter */
				m_cmp_line = CMPLINE;
				m_next_state = GB_LCD_STATE_LYXX_M0_INC;
				state_cycles = 4;
				break;

			case GB_LCD_STATE_LYXX_M0_INC:  /* Increment LY, stay in M0 for 4 more cycles */
				increment_scanline();
				m_stat_lyc_int = false;
				if (CURLINE < 144)
				{
					m_stat_mode0_int = false;
				}
				m_stat_mode2_int = (LCDSTAT & MODE_2_INT_ENABLED) ? true : false;
				check_stat_irq();
				m_stat_lyc_int = ((m_cmp_line == CURLINE) && (LCDSTAT & LY_LYC_INT_ENABLED)) ? true : false;
				LOG("GB_LCD_STATE_LYXX_M0_INC: m_cmp_line = %u, CURLINE = %u, LCDSTAT = %02x, m_stat_lyc_int = %s\n", m_cmp_line, CURLINE, LCDSTAT, m_stat_lyc_int ? "true" : "false");
				m_hdma_possible = 0;
				/* Check if we're going into VBlank next */
				if (CURLINE == 144)
				{
					m_next_state = GB_LCD_STATE_LY9X_M1;
					state_cycles = 4;
				}
				else
				{
					/* Internally switch to mode 2 */
					m_next_state = GB_LCD_STATE_LYXX_M2;
					m_oam_locked_reading = LOCKED;
					state_cycles = 4;
				}
				break;

			case GB_LCD_STATE_LY00_M2:      /* Switch to mode 2 on line #0 */
				/* Set Mode 2 lcdstate */
				m_mode = 2;
				LCDSTAT = (LCDSTAT & 0xFC) | 0x02;
				m_oam_locked = LOCKED;
				/* Generate lcd interrupt if requested */
				m_stat_mode1_int = false;
				m_stat_mode2_int = (LCDSTAT & MODE_2_INT_ENABLED) ? true : false;
				check_stat_irq();
				/* Mode 2 lasts approximately 80 clock cycles */
				m_next_state = GB_LCD_STATE_LYXX_M3;
				clear_line_state();
				/* Check for regular compensation of x-scroll register */
				m_scrollx_adjust = SCROLLX & 0x07;
				select_sprites();
				m_window_y = WNDPOSY;
				state_cycles = 80;
				break;

			case GB_LCD_STATE_LYXX_M2:      /* Switch to mode 2 */
				m_stat_mode0_int = false;
				check_stat_irq();
				/* Update STAT register to the correct state */
				m_mode = 2;
				LCDSTAT = (LCDSTAT & 0xFC) | 0x02;
				m_oam_locked = LOCKED;
				/* Check if LY==LYC STAT bit should be set */
				if (CURLINE == CMPLINE)
				{
					LCDSTAT |= LY_LYC_FLAG;
				}
				else
				{
					LCDSTAT &= ~LY_LYC_FLAG;
				}
				/* Mode 2 last for approximately 80 clock cycles */
				m_next_state = GB_LCD_STATE_LYXX_M3;
				clear_line_state();
				/* Check for regular compensation of x-scroll register */
				m_scrollx_adjust = SCROLLX & 0x07;
				select_sprites();
				m_window_y = WNDPOSY;
				state_cycles = 80;
				break;

			case GB_LCD_STATE_LYXX_M3:      /* Switch to mode 3 */
				for (int i = 0; i < std::size(m_line.window_start_y); i++)
				{
					m_line.window_start_y[i] = WNDPOSY;
					m_line.window_start_x[i] = WNDPOSX;
				}
				m_line.window_start_y_index = 0;
				for (int i = 0; i < std::size(m_line.window_enable); i++)
				{
					m_line.window_enable[i] = LCDCONT;
				}
				m_line.window_enable_index = 0;
				m_stat_mode2_int = false;
				check_stat_irq();
				/* Set Mode 3 lcdstate */
				m_mode = 3;
				LCDSTAT = (LCDSTAT & 0xFC) | 0x03;
				m_oam_locked = LOCKED;
				m_vram_locked = LOCKED;
				m_pal_locked = LOCKED;
				/* Check for compensations of x-scroll register */
				/* Mode 3 lasts for approximately 172+cycles needed to handle sprites clock cycles */
				m_next_state = GB_LCD_STATE_LYXX_PRE_M0;
				state_cycles = 168 + m_scrollx_adjust + m_sprite_cycles;

				// This magic 12 needs to be improved as it fixes just one test. We have
				// to also support mid-scanline WX write at other times.
				m_next_state = GB_LCD_STATE_LYXX_M3_2;
				state_cycles = 12;
				m_start_x = -1;
				break;

			case GB_LCD_STATE_LYXX_M3_2:
				m_window_x = WNDPOSX;
				calculate_window_cycles();
				m_next_state = GB_LCD_STATE_LYXX_PRE_M0;
				state_cycles = 168 - 12 + m_scrollx_adjust + m_sprite_cycles + m_window_cycles;
				m_next_state = GB_LCD_STATE_LYXX_M0;
				state_cycles = 4 - /*2*/3 + 168 - 12 + m_scrollx_adjust + m_sprite_cycles + m_window_cycles;
				break;

			case GB_LCD_STATE_LY9X_M1:      /* Switch to or stay in mode 1 */
				m_stat_mode0_int = false;
				m_stat_mode2_int = false;
				if (CURLINE == 144)
				{
					/* Trigger VBlank interrupt */
					m_lr35902->set_input_line(lr35902_cpu_device::VBL_INT, ASSERT_LINE);
					m_lr35902->execute_set_input(lr35902_cpu_device::VBL_INT, ASSERT_LINE);
					/* Set VBlank lcdstate */
					m_mode = 1;
					LCDSTAT = (LCDSTAT & 0xFC) | 0x01;
					/* Trigger LCD interrupt if requested */
					m_stat_mode1_int = (LCDSTAT & MODE_1_INT_ENABLED) ? true : false;
				}
				check_stat_irq();
				/* Check if LY==LYC STAT bit should be set */
				if (CURLINE == CMPLINE)
				{
					LCDSTAT |= LY_LYC_FLAG;
				}
				else
				{
					LCDSTAT &= ~LY_LYC_FLAG;
				}
				m_next_state = GB_LCD_STATE_LY9X_M1_INC;
				state_cycles = 452;
				break;

			case GB_LCD_STATE_LY9X_M1_INC:      /* Increment scanline counter */
				increment_scanline();
				m_stat_lyc_int = ((CMPLINE == CURLINE) && (LCDSTAT & LY_LYC_INT_ENABLED)) ? true : false;
				if (m_current_line == 153)
				{
					m_next_state = GB_LCD_STATE_LY00_M1;
					state_cycles = 4;
				}
				else
				{
					m_next_state = GB_LCD_STATE_LY9X_M1;
					state_cycles = 4;
				}
				break;

			case GB_LCD_STATE_LY00_M1:      /* we stay in VBlank but current line counter should already be incremented */
				LOG("GB_LCD_STATE_LY00_M1, CURLINE=%u, CMPLINE=%u, m_stat_lyc_int=%s\n", CURLINE, CMPLINE, m_stat_lyc_int ? "true" : "false");
				/* Check LY=LYC for line #153 */
				if (CURLINE == CMPLINE)
				{
					LCDSTAT |= LY_LYC_FLAG;
				}
				else
				{
					LCDSTAT &= ~LY_LYC_FLAG;
				}
				check_stat_irq();
				m_next_state = GB_LCD_STATE_LY00_M1_1;
				state_cycles = 4;
				break;

			case GB_LCD_STATE_LY00_M1_1:
				increment_scanline();
				m_stat_lyc_int = ((CMPLINE == CURLINE) && (LCDSTAT & LY_LYC_INT_ENABLED)) ? true : false;
				LOG("GB_LCD_STATE_LY00_M1_1, m_stat_lyc_int = %s\n", m_stat_lyc_int ? "true" : "false");
				m_next_state = GB_LCD_STATE_LY00_M1_2;
				state_cycles = 4;
				break;

			case GB_LCD_STATE_LY00_M1_2:    /* Rest of line #0 during VBlank */
				check_stat_irq();
				if (CURLINE == CMPLINE)
				{
					LCDSTAT |= LY_LYC_FLAG;
				}
				else
				{
					LCDSTAT &= ~LY_LYC_FLAG;
				}
				m_next_state = GB_LCD_STATE_LY00_M0;
				state_cycles = 444;
				break;

			case GB_LCD_STATE_LY00_M0:      /* Just before going to mode 2 for LY 0 */
				m_mode = 2;
				m_next_state = GB_LCD_STATE_LY00_M2;
				state_cycles = 4;
				break;
			}
			assert(state_cycles > 0);

			if (cycles >= state_cycles)
			{
				cycles -= state_cycles;
				m_cycles_left = 0;
			}
			else
			{
				m_cycles_left = state_cycles - cycles;
			}
		}
	}
	else
	{
		increment_scanline();
		if (m_current_line < 144)
		{
			// Force draw of empty line
			update_scanline(0);
		}
		m_cycles_left = 456;
	}
	assert(m_cycles_left > 0);

	m_last_updated = now;

	int next_cycles = m_cycles_left;

	if (m_oam_dma_start_cycles > 0 && m_oam_dma_start_cycles < next_cycles)
	{
		next_cycles = m_oam_dma_start_cycles;
	}

	if (m_oam_dma_cycles_left > 0 && m_oam_dma_cycles_left < next_cycles)
	{
		next_cycles = m_oam_dma_cycles_left;
	}

	if (m_hdma_cycles_to_start > 0 && m_hdma_cycles_to_start < next_cycles)
	{
		next_cycles = m_hdma_cycles_to_start;
	}

	m_lcd_timer->adjust(m_lr35902->cycles_to_attotime(next_cycles));

	m_updating_state = false;
}


void dmg_ppu_device::lcd_switch_on(uint8_t new_data)
{
	m_current_line = 0;
	m_previous_line = 153;
	m_window_lines_drawn = 0;
	m_window_cycles = 0;
	m_mode = 4;   // Starting up
	m_sprCount = 0;
	m_sprite_cycles = 0;
	m_oam_locked = UNLOCKED;
	m_oam_locked_reading = UNLOCKED;
	m_window_y = 0xFF;
	m_stat_mode0_int = false;
	m_stat_mode1_int = false;
	m_stat_mode2_int = false;
	m_stat_lyc_int = false;
	m_stat_lyc_int_prev = false;
	m_stat_write_int = false;
	m_stat_int = false;
	m_hdma_cycles_to_start = 0;
	m_frame_window_active = false;
	// Check for LY=LYC coincidence
	if (CURLINE == CMPLINE && CURLINE != m_old_curline)
	{
		LCDSTAT |= LY_LYC_FLAG;
		// Generate lcd interrupt if requested
		if (LCDSTAT & LY_LYC_INT_ENABLED)
		{
			m_stat_lyc_int = true;
			check_stat_irq();
		}
	}
	else
	{
		LCDSTAT &= ~LY_LYC_FLAG;
	}
	clear_line_state();
	m_window_y = WNDPOSY;
	if ((new_data & WINDOW_ENABLED) && m_current_line == m_window_y)
	{
		m_line.window_should_trigger = true;
	}
	m_state = GB_LCD_STATE_LY00_M2;
	m_next_state = GB_LCD_STATE_LYXX_M3;
	m_cycles_left = 80;
	m_lcd_timer->adjust(m_lr35902->cycles_to_attotime(m_cycles_left));
}


uint8_t dmg_ppu_device::vram_r(offs_t offset)
{
	if (!machine().side_effects_disabled())
	{
		update_state();
		LOG("vram_r: offset=0x%04x\n", offset);
	}

	return (m_vram_locked == LOCKED) ? 0xff : m_vram[offset + (m_vram_bank * 0x2000)];
}


void dmg_ppu_device::vram_w(offs_t offset, uint8_t data)
{
	update_state();
	if (m_vram_locked == LOCKED)
		return;

	m_vram[offset + (m_vram_bank * 0x2000)] = data;
}


uint8_t dmg_ppu_device::oam_r(offs_t offset)
{
	if (!machine().side_effects_disabled())
	{
		update_state();
		LOG("oam_r: offset=0x%02x\n", offset);
	}

	return (m_oam_locked == LOCKED || m_oam_locked_reading == LOCKED || m_oam_dma_processing) ? 0xff : m_oam[offset];
}


void dmg_ppu_device::oam_w(offs_t offset, uint8_t data)
{
	update_state();
	if (m_oam_locked == LOCKED || offset >= 0xa0 || m_oam_dma_processing)
		return;

	m_oam[offset] = data;
}



uint8_t dmg_ppu_device::video_r(offs_t offset)
{
	if (!machine().side_effects_disabled())
	{
		update_state();
		if (offset == 1) LOG("STAT read\n");
		if (offset == 0x28) LOG("BCPS read, palette is %s\n", m_pal_locked == LOCKED ? "LOCKED" : "UNLOCKED");
		if (offset == 0x29) LOG("BCPD read, palette is %s\n", m_pal_locked == LOCKED ? "LOCKED" : "UNLOCKED");
	}

	return m_vid_regs[offset];
}


bool dmg_ppu_device::stat_write(uint8_t new_data)
{
	LOG("stat_write: old_data = %02x, new_data = %02x\n", LCDSTAT & 0x78, new_data & 0x78);

	bool new_lyc_int = m_stat_lyc_int;

	/* Check if line irqs are being enabled */
	if (new_data & 0x40)
	{
		if ((LCDSTAT & (LY_LYC_INT_ENABLED | LY_LYC_FLAG)) == LY_LYC_FLAG)
		{
			new_lyc_int = true;
		}
	}
	else
	{
		new_lyc_int = false;
	}

	switch (m_mode)
	{
	case 0:
		m_stat_mode0_int = (new_data & MODE_0_INT_ENABLED) ? true : false;
		if (!m_stat_int)
		{
			if (!(LCDSTAT & MODE_0_INT_ENABLED))
			{
				m_stat_write_int = true;
			}
		}
		else
		{
			if (!(LCDSTAT & MODE_0_INT_ENABLED))
			{
				if (!m_stat_lyc_int && !new_lyc_int)
				{
					// Force an irq
					m_stat_int = false;
					m_stat_write_int = true;
				}
			}
		}
		break;
	case 1:
		m_stat_mode1_int = (new_data & MODE_1_INT_ENABLED) ? true : false;
		if (!m_stat_int)
		{
			m_stat_write_int = true;
		}
		else
		{
			if (!(LCDSTAT & MODE_1_INT_ENABLED))
			{
				if (!m_stat_lyc_int && !new_lyc_int)
				{
					// Force an irq
					m_stat_int = false;
					m_stat_write_int = true;
				}
			}
		}
		break;
	case 2:
		// 0x20 -> 0x40  with LYC -> trigger
		// 0x20 -> 0x60  with LYC -> trigger
		//m_stat_mode2_int = (new_data & MODE_2_INT_ENABLED) ? true : false;
		if (LCDSTAT & MODE_2_INT_ENABLED)
		{
			if (!m_stat_lyc_int && new_lyc_int)
			{
				// Force an irq
				m_stat_int = false;
			}
		}
		// Weird trigger for stat irqs
		if ((LCDSTAT & (LY_LYC_INT_ENABLED | LY_LYC_FLAG)) == LY_LYC_FLAG)
		{
			m_stat_write_int = true;
			// Force an irq
			m_stat_int = false;
		}
		break;
	default:
		break;
	}

	m_stat_lyc_int = new_lyc_int;
	check_stat_irq();

	return false;
}


void dmg_ppu_device::check_stat_irq()
{
	bool new_stat_int = m_stat_mode0_int || m_stat_mode1_int || m_stat_mode2_int || m_stat_lyc_int || m_stat_write_int;

	LOG("m_mode = %d, m_stat_mode0_int = %s, m_stat_mode1_int = %s, m_stat_mode2_int = %s, m_stat_lyc_int = %s\n",
		m_mode,
		m_stat_mode0_int ? "true" : "false",
		m_stat_mode1_int ? "true" : "false",
		m_stat_mode2_int ? "true" : "false",
		m_stat_lyc_int ? "true" : "false"
	);

	if (new_stat_int && !m_stat_int)
	{
		LOG("--m_stat_mode0_int = %s, m_stat_mode1_int = %s, m_stat_mode2_int = %s, m_stat_lyc_int = %s\n",
			m_stat_mode0_int ? "true" : "false",
			m_stat_mode1_int ? "true" : "false",
			m_stat_mode2_int ? "true" : "false",
			m_stat_lyc_int ? "true" : "false"
		);

		m_lr35902->set_input_line(lr35902_cpu_device::LCD_INT, ASSERT_LINE);
		m_lr35902->execute_set_input(lr35902_cpu_device::LCD_INT, ASSERT_LINE);
	}

	m_stat_int = new_stat_int;
	m_stat_write_int = false;
}


void dmg_ppu_device::video_w(offs_t offset, uint8_t data)
{
	update_state();
	LOG("video_w: offset = %02x, data = %02x\n", offset, data);

	switch (offset)
	{
	case 0x00:                      /* LCDC - LCD Control */
		m_gb_chrgen_offs = (data & 0x10) ? 0x0000 : 0x0800;
		m_gb_tile_no_mod = (data & 0x10) ? 0x00 : 0x80;
		m_gb_bgdtab_offs = (data & 0x08) ? 0x1c00 : 0x1800;
		m_gb_wndtab_offs = (data & 0x40) ? 0x1c00 : 0x1800;
		/* if LCD controller is switched off, set STAT and LY to 00 */
		if (!(data & ENABLED))
		{
			LCDSTAT &= ~0x03;
			m_old_curline = CURLINE;
			CURLINE = 0;
			m_oam_locked = UNLOCKED;
			m_vram_locked = UNLOCKED;
		}
		else
		{
			/* If LCD is being switched on */
			if (!(LCDCONT & ENABLED))
			{
				lcd_switch_on(data);
			}
			else
			{
				if (m_line.window_active && !(data & WINDOW_ENABLED))
				{
					m_window_lines_drawn++;
					m_line.window_active = false;
				}
			}
		}
		break;
	case 0x01:                      /* STAT - LCD Status */
		data = 0x80 | (data & 0x78) | (LCDSTAT & 0x07);
		/*
		   Check for the STAT bug:
		   Writing to STAT when the LCD controller is active causes a STAT
		   interrupt to be triggered.
		 */
		if (LCDCONT & ENABLED)
		{
			stat_write(data);
		}
		break;
	case 0x04:                      /* LY - LCD Y-coordinate */
		return;
	case 0x05:                      /* LYC */
		if (CMPLINE != data)
		{
			if (CURLINE == data || (m_state == GB_LCD_STATE_LY00_M1 && CURLINE == 0 && data == 153))
			{
				LOG("write LYC, if\n");
				LCDSTAT |= LY_LYC_FLAG;
				/* Generate lcd interrupt if requested */
				if (LCDSTAT & LY_LYC_INT_ENABLED)
				{
					if (m_state != GB_LCD_STATE_LYXX_M0_INC || !m_stat_lyc_int_prev)
					{
						m_stat_lyc_int = true;
						// Force an irq?
						if (m_stat_mode2_int)
						{
							m_stat_int = false;
						}
						check_stat_irq();
					}
				}
			}
			else
			{
				LOG("write LYC, else\n");
				LCDSTAT &= ~LY_LYC_FLAG;
				m_stat_lyc_int = false;
				check_stat_irq();
			}
		}
		break;
	case 0x06:                      /* DMA - DMA Transfer and Start Address */
		m_oam_dma_source_address = data << 8;
		m_oam_dma_start_cycles = 8;
		update_state();
		return;
	case 0x07:                      /* BGP - Background Palette */
		update_scanline(m_lr35902->attotime_to_cycles(m_lcd_timer->remaining()));
		m_gb_bpal[0] = data & 0x3;
		m_gb_bpal[1] = (data & 0xC) >> 2;
		m_gb_bpal[2] = (data & 0x30) >> 4;
		m_gb_bpal[3] = (data & 0xC0) >> 6;
		break;
	case 0x08:                      /* OBP0 - Object Palette 0 */
//      update_scanline(m_lr35902->attotime_to_cycles(m_lcd_timer->remaining()));
		m_gb_spal0[0] = data & 0x3;
		m_gb_spal0[1] = (data & 0xC) >> 2;
		m_gb_spal0[2] = (data & 0x30) >> 4;
		m_gb_spal0[3] = (data & 0xC0) >> 6;
		break;
	case 0x09:                      /* OBP1 - Object Palette 1 */
//      update_scanline(m_lr35902->attotime_to_cycles(m_lcd_timer->remaining()));
		m_gb_spal1[0] = data & 0x3;
		m_gb_spal1[1] = (data & 0xC) >> 2;
		m_gb_spal1[2] = (data & 0x30) >> 4;
		m_gb_spal1[3] = (data & 0xC0) >> 6;
		break;
	case 0x02:                      /* SCY - Scroll Y */
		update_scanline(m_lr35902->attotime_to_cycles(m_lcd_timer->remaining()));
		break;
	case 0x03:                      /* SCX - Scroll X */
		update_scanline(m_lr35902->attotime_to_cycles(m_lcd_timer->remaining()));
		LOG("SCX: scrollx_delay = %d, m_cycles_left = %d\n", m_line.scrollx_delay, m_cycles_left);
		if (m_line.scrollx_delay > 0)
		{
			// Additional delay cycles; not sure if this is correct.
			int adjust = (data & 0x07);

			m_line.scrollx_delay += adjust;
		}
		break;
	case 0x0A:                      /* WY - Window Y position */
		LOG("WY write, m_cycles_left = %d\n", m_cycles_left);
		break;
	case 0x0B:                      /* WX - Window X position */
		LOG("WX write, m_cycles_left = %d\n", m_cycles_left);
		break;
	default:                        /* Unknown register, no change */
		return;
	}
	m_vid_regs[offset] = data;
}

uint8_t cgb_ppu_device::video_r(offs_t offset)
{
	if (!machine().side_effects_disabled())
	{
		update_state();
		if (offset == 1) LOG("STAT read\n");
		if (offset == 0x28) LOG("BCPS read, palette is %s\n", m_pal_locked == LOCKED ? "LOCKED" : "UNLOCKED");
		if (offset == 0x29) LOG("BCPD read, palette is %s\n", m_pal_locked == LOCKED ? "LOCKED" : "UNLOCKED");
	}

	switch (offset)
	{
	case 0x11:  /* FF51 */
	case 0x12:  /* FF52 */
	case 0x13:  /* FF53 */
	case 0x14:  /* FF54 */
		return 0xFF;
	case 0x29:  /* FF69 */
	case 0x2B:  /* FF6B */
		if (m_pal_locked == LOCKED)
		{
			return 0xFF;
		}
		break;
	}
	return m_vid_regs[offset];
}


bool cgb_ppu_device::stat_write(uint8_t new_data)
{
	LOG("stat_write: old_data = %02x, new_data = %02x\n", LCDSTAT & 0x78, new_data & 0x78);

	bool new_lyc_int = m_stat_lyc_int;

	/* Check if line irqs are being enabled */
	if (m_state != GB_LCD_STATE_LYXX_M0_INC && m_state != GB_LCD_STATE_LY00_M1_1)
	{
		if (new_data & LY_LYC_INT_ENABLED)
		{
			if ((LCDSTAT & (LY_LYC_INT_ENABLED | LY_LYC_FLAG)) == LY_LYC_FLAG)
			{
				new_lyc_int = true;
			}
		}
		else
		{
			new_lyc_int = false;
		}
	}

	switch (m_mode)
	{
	case 0:
		m_stat_mode0_int = (new_data & MODE_0_INT_ENABLED) ? true : false;
		break;
	case 1:
		// 0x40 -> 0x50, during incrementing line counter and LY=LYC check getting de-asserted
		if (m_state == GB_LCD_STATE_LY9X_M1_INC)
		{
			check_stat_irq();
		}
		m_stat_mode1_int = (new_data & MODE_1_INT_ENABLED) ? true : false;
		break;
	case 2:
		// 0x20 -> 0x40  with LYC -> trigger
		//m_stat_mode2_int = (new_data & MODE_2_INT_ENABLED) ? true : false;
		if (LCDSTAT & MODE_2_INT_ENABLED)
		{
			if (!m_stat_lyc_int && new_lyc_int)
			{
				// Force an irq
				m_stat_int = false;
			}
		}
		break;

	default:
		break;
	}

	m_stat_lyc_int = new_lyc_int;
	check_stat_irq();

	return false;
}


void cgb_ppu_device::video_w(offs_t offset, uint8_t data)
{
	update_state();
	LOG("video_w\n");

	switch (offset)
	{
	case 0x00:      /* LCDC - LCD Control */
		m_gb_chrgen_offs = (data & 0x10) ? 0x0000 : 0x0800;
		m_gbc_chrgen_offs = (data & 0x10) ? 0x2000 : 0x2800;
		m_gb_tile_no_mod = (data & 0x10) ? 0x00 : 0x80;
		m_gb_bgdtab_offs = (data & 0x08) ? 0x1c00 : 0x1800;
		m_gbc_bgdtab_offs = (data & 0x08) ? 0x3c00 : 0x3800;
		m_gb_wndtab_offs = (data & 0x40) ? 0x1c00 : 0x1800;
		m_gbc_wndtab_offs = (data & 0x40) ? 0x3c00 : 0x3800;
		/* if LCD controller is switched off, set STAT to 00 */
		if (!(data & 0x80))
		{
			LCDSTAT &= ~0x03;
			m_old_curline = CURLINE;
			CURLINE = 0;
			m_oam_locked = UNLOCKED;
			m_vram_locked = UNLOCKED;
			m_pal_locked = UNLOCKED;
		}
		else
		{
			/* If LCD is being switched on */
			if (!(LCDCONT & ENABLED))
			{
				lcd_switch_on(data);
			}
			else
			{
				if (m_line.window_active && !(data & WINDOW_ENABLED))
				{
					m_window_lines_drawn++;
					m_line.window_active = false;
				}
			}
		}
		break;
	case 0x01:      /* STAT - LCD Status */
		data = 0x80 | (data & 0x78) | (LCDSTAT & 0x07);
		if (LCDCONT & ENABLED)
		{
			stat_write(data);
		}
		break;
	case 0x05:                      /* LYC */
		if (CMPLINE != data)
		{
			if (CURLINE == data && m_state != GB_LCD_STATE_LY00_M1 && m_state != GB_LCD_STATE_LYXX_M0_PRE_INC)
			{
				LOG("write LYC, if, CURLINE=%u\n", CURLINE);

				LCDSTAT |= LY_LYC_FLAG;
				/* Generate lcd interrupt if requested */
				if (LCDSTAT & LY_LYC_INT_ENABLED)
				{
					m_stat_lyc_int = true;
					check_stat_irq();
				}
			}
			else
			{
				LOG("write LYC, else, CURLINE=%u\n", CURLINE);

				LCDSTAT &= ~LY_LYC_FLAG;
				check_stat_irq();
				m_cmp_line = data;
				m_stat_lyc_int = false;
				check_stat_irq();
			}
		}
		break;
	case 0x07:      /* BGP - GB background palette */
		update_scanline(m_lr35902->attotime_to_cycles(m_lcd_timer->remaining()));
		m_gb_bpal[0] = data & 0x3;
		m_gb_bpal[1] = (data & 0xC) >> 2;
		m_gb_bpal[2] = (data & 0x30) >> 4;
		m_gb_bpal[3] = (data & 0xC0) >> 6;
		break;
	case 0x08:      /* OBP0 - GB Object 0 palette */
		m_gb_spal0[0] = data & 0x3;
		m_gb_spal0[1] = (data & 0xC) >> 2;
		m_gb_spal0[2] = (data & 0x30) >> 4;
		m_gb_spal0[3] = (data & 0xC0) >> 6;
		break;
	case 0x09:      /* OBP1 - GB Object 1 palette */
		m_gb_spal1[0] = data & 0x3;
		m_gb_spal1[1] = (data & 0xC) >> 2;
		m_gb_spal1[2] = (data & 0x30) >> 4;
		m_gb_spal1[3] = (data & 0xC0) >> 6;
		break;
	case 0x0c:      /* Undocumented register involved in selecting gb/gbc mode */
		logerror("Write to undocumented register: %X = %X\n", offset, data);
		break;
	case 0x0F:      /* VBK - VRAM bank select */
		m_vram_bank = data & 0x01;
		data |= 0xFE;
		break;
	case 0x11:      /* HDMA1 - HBL General DMA - Source High */
		break;
	case 0x12:      /* HDMA2 - HBL General DMA - Source Low */
		break;
	case 0x13:      /* HDMA3 - HBL General DMA - Destination High */
		break;
	case 0x14:      /* HDMA4 - HBL General DMA - Destination Low */
		break;
	case 0x15:      /* HDMA5 - HBL General DMA - Mode, Length */
		if (!(data & 0x80))
		{
			if (m_hdma_enabled)
			{
				m_hdma_enabled = 0;
				data = HDMA5 & 0x80;
			}
			else
			{
				/* General DMA */
				hdma_trans(((data & 0x7F) + 1) * 0x10);
				data = 0xff;
			}
		}
		else
		{
			/* H-Blank DMA */
			m_hdma_enabled = 1;
			data &= 0x7f;
			m_vid_regs[offset] = data;
			/* Check if HDMA should be immediately performed */
			if (m_hdma_possible)
			{
				hdma_trans(0x10);
				m_hdma_possible = 0;
			}
		}
		break;
	case 0x28:      /* BCPS - Background palette specification */
		LOG("BCPS write %02x\n", data);

		GBCBCPS = data;
		if (data & 0x01)
			GBCBCPD = m_cgb_bpal[(data >> 1) & 0x1F] >> 8;
		else
			GBCBCPD = m_cgb_bpal[(data >> 1) & 0x1F] & 0xFF;
		break;
	case 0x29:      /* BCPD - background palette data */
		LOG("BCPD write %02x, palette is %s\n", data, m_pal_locked == LOCKED ? "LOCKED" : "UNLOCKED");

		if (m_pal_locked == LOCKED)
		{
			return;
		}
		GBCBCPD = data;
		if (GBCBCPS & 0x01)
			m_cgb_bpal[(GBCBCPS >> 1) & 0x1F] = ((data << 8) | (m_cgb_bpal[(GBCBCPS >> 1) & 0x1F] & 0xFF)) & 0x7FFF;
		else
			m_cgb_bpal[(GBCBCPS >> 1) & 0x1F] = ((m_cgb_bpal[(GBCBCPS >> 1) & 0x1F] & 0xFF00) | data) & 0x7FFF;
		if (GBCBCPS & 0x80)
		{
			GBCBCPS++;
			GBCBCPS &= 0xBF;
		}
		break;
	case 0x2A:      /* OCPS - Object palette specification */
		GBCOCPS = data;
		if (data & 0x01)
			GBCOCPD = m_cgb_spal[(data >> 1) & 0x1F] >> 8;
		else
			GBCOCPD = m_cgb_spal[(data >> 1) & 0x1F] & 0xFF;
		break;
	case 0x2B:      /* OCPD - Object palette data */
		if (m_pal_locked == LOCKED)
		{
			return;
		}
		GBCOCPD = data;
		if (GBCOCPS & 0x01)
			m_cgb_spal[(GBCOCPS >> 1) & 0x1F] = ((data << 8) | (m_cgb_spal[(GBCOCPS >> 1) & 0x1F] & 0xFF)) & 0x7FFF;
		else
			m_cgb_spal[(GBCOCPS >> 1) & 0x1F] = ((m_cgb_spal[(GBCOCPS >> 1) & 0x1F] & 0xFF00) | data) & 0x7FFF;
		if (GBCOCPS & 0x80)
		{
			GBCOCPS++;
			GBCOCPS &= 0xBF;
		}
		break;
	/* Undocumented registers */
	case 0x2C:
		/* bit 0 can be read/written */
		logerror("Write to undocumented register: %X = %X\n", offset, data);
		data = 0xFE | (data & 0x01);
		if (data & 0x01)
		{
			m_gbc_mode = 0;
		}
		break;
	case 0x32:
	case 0x33:
	case 0x34:
		/* whole byte can be read/written */
		logerror("Write to undocumented register: %X = %X\n", offset, data);
		break;
	case 0x35:
		/* bit 4-6 can be read/written */
		logerror("Write to undocumented register: %X = %X\n", offset, data);
		data = 0x8F | (data & 0x70);
		break;
	case 0x36:
	case 0x37:
		logerror("Write to undocumented register: %X = %X\n", offset, data);
		return;
	default:
		/* we didn't handle the write, so pass it to the GB handler */
		dmg_ppu_device::video_w(offset, data);
		return;
	}

	m_vid_regs[offset] = data;
}

// Super Game Boy

/* Super Game Boys transfer VRAM data using the display signals.
 * That means not DMG VRAM is copied from 0x8800 to SNES VRAM
 * but displayed BG contents.
 * Things to consider: LCDC (0xFF40) and SCY/SCX (0xFF42/3)
 *  - LCD must be on
 *  - TODO: Window might or might not influence result
 *  - CHR and BG likely influence result
 *  - BG must be on
 *  - BG should not be scrolled, but likely does influence transfer
 *  - TODO: are BG attributes (hflip, vflip) ignored?
 */
/**
 * Copy DMG VRAM data to SGM VRAM
 * @param dst       Destination Pointer
 * @param start     Logical Start Tile index inside display area.
 * @param num_tiles Number of DMG tiles (0x10U bytes) to copy.
 */
void sgb_ppu_device::sgb_vram_memcpy(uint8_t *dst, uint8_t start, size_t num_tiles)
{
	uint16_t bg_ix = (start / 0x14U) * 0x20U + (start % 0x14U);
	const uint8_t *const map = m_layer[0].bg_map;
	const uint8_t *const tiles = m_layer[0].bg_tiles;
	const uint8_t mod = m_gb_tile_no_mod;

	for (size_t i = 0x00U; i < num_tiles && i < 0x100U; ++i)
	{
		const uint8_t tile_ix = map[bg_ix] ^ mod;
		std::copy_n(&tiles[tile_ix << 4], 0x10U, dst);
		dst += 0x10U;

		++bg_ix;
		if ((bg_ix & 0x1fU) == 0x14U)
			bg_ix += 0x20U - 0x14U; /* advance to next start of line */
	}
}

void sgb_ppu_device::sgb_io_write_pal(int offs, uint8_t *data)
{
	switch (offs)
	{
		case 0x00:  /* PAL01 */
			m_sgb_pal[0 * 4 + 0] = data[1] | (data[2] << 8);
			m_sgb_pal[0 * 4 + 1] = data[3] | (data[4] << 8);
			m_sgb_pal[0 * 4 + 2] = data[5] | (data[6] << 8);
			m_sgb_pal[0 * 4 + 3] = data[7] | (data[8] << 8);
			m_sgb_pal[1 * 4 + 0] = data[1] | (data[2] << 8);
			m_sgb_pal[1 * 4 + 1] = data[9] | (data[10] << 8);
			m_sgb_pal[1 * 4 + 2] = data[11] | (data[12] << 8);
			m_sgb_pal[1 * 4 + 3] = data[13] | (data[14] << 8);
			break;
		case 0x01:  /* PAL23 */
			m_sgb_pal[2 * 4 + 0] = data[1] | (data[2] << 8);
			m_sgb_pal[2 * 4 + 1] = data[3] | (data[4] << 8);
			m_sgb_pal[2 * 4 + 2] = data[5] | (data[6] << 8);
			m_sgb_pal[2 * 4 + 3] = data[7] | (data[8] << 8);
			m_sgb_pal[3 * 4 + 0] = data[1] | (data[2] << 8);
			m_sgb_pal[3 * 4 + 1] = data[9] | (data[10] << 8);
			m_sgb_pal[3 * 4 + 2] = data[11] | (data[12] << 8);
			m_sgb_pal[3 * 4 + 3] = data[13] | (data[14] << 8);
			break;
		case 0x02:  /* PAL03 */
			m_sgb_pal[0 * 4 + 0] = data[1] | (data[2] << 8);
			m_sgb_pal[0 * 4 + 1] = data[3] | (data[4] << 8);
			m_sgb_pal[0 * 4 + 2] = data[5] | (data[6] << 8);
			m_sgb_pal[0 * 4 + 3] = data[7] | (data[8] << 8);
			m_sgb_pal[3 * 4 + 0] = data[1] | (data[2] << 8);
			m_sgb_pal[3 * 4 + 1] = data[9] | (data[10] << 8);
			m_sgb_pal[3 * 4 + 2] = data[11] | (data[12] << 8);
			m_sgb_pal[3 * 4 + 3] = data[13] | (data[14] << 8);
			break;
		case 0x03:  /* PAL12 */
			m_sgb_pal[1 * 4 + 0] = data[1] | (data[2] << 8);
			m_sgb_pal[1 * 4 + 1] = data[3] | (data[4] << 8);
			m_sgb_pal[1 * 4 + 2] = data[5] | (data[6] << 8);
			m_sgb_pal[1 * 4 + 3] = data[7] | (data[8] << 8);
			m_sgb_pal[2 * 4 + 0] = data[1] | (data[2] << 8);
			m_sgb_pal[2 * 4 + 1] = data[9] | (data[10] << 8);
			m_sgb_pal[2 * 4 + 2] = data[11] | (data[12] << 8);
			m_sgb_pal[2 * 4 + 3] = data[13] | (data[14] << 8);
			break;
		case 0x04:  /* ATTR_BLK */
		{
			uint8_t I, J, K, o;
			for( K = 0; K < data[1]; K++ )
			{
				o = K * 6;
				if( data[o + 2] & 0x1 )
				{
					for( I = data[ o + 4]; I <= data[o + 6]; I++ )
					{
						for( J = data[o + 5]; J <= data[o + 7]; J++ )
						{
							m_sgb_pal_map[I][J] = data[o + 3] & 0x3;
						}
					}
				}
			}
		}
			break;
		case 0x05:  /* ATTR_LIN */
		{
			uint8_t J, K;
			if( data[1] > 15 )
				data[1] = 15;
			for( K = 0; K < data[1]; K++ )
			{
				if( data[K + 1] & 0x80 )
				{
					for( J = 0; J < 20; J++ )
					{
						m_sgb_pal_map[J][data[K + 1] & 0x1f] = (data[K + 1] & 0x60) >> 5;
					}
				}
				else
				{
					for( J = 0; J < 18; J++ )
					{
						m_sgb_pal_map[data[K + 1] & 0x1f][J] = (data[K + 1] & 0x60) >> 5;
					}
				}
			}
		}
			break;
		case 0x06:  /* ATTR_DIV */
		{
			uint8_t I, J;
			if( data[1] & 0x40 ) /* Vertical */
			{
				for( I = 0; I < data[2]; I++ )
				{
					for( J = 0; J < 20; J++ )
					{
						m_sgb_pal_map[J][I] = (data[1] & 0xC) >> 2;
					}
				}
				for( J = 0; J < 20; J++ )
				{
					m_sgb_pal_map[J][data[2]] = (data[1] & 0x30) >> 4;
				}
				for( I = data[2] + 1; I < 18; I++ )
				{
					for( J = 0; J < 20; J++ )
					{
						m_sgb_pal_map[J][I] = data[1] & 0x3;
					}
				}
			}
			else /* Horizontal */
			{
				for( I = 0; I < data[2]; I++ )
				{
					for( J = 0; J < 18; J++ )
					{
						m_sgb_pal_map[I][J] = (data[1] & 0xC) >> 2;
					}
				}
				for( J = 0; J < 18; J++ )
				{
					m_sgb_pal_map[data[2]][J] = (data[1] & 0x30) >> 4;
				}
				for( I = data[2] + 1; I < 20; I++ )
				{
					for( J = 0; J < 18; J++ )
					{
						m_sgb_pal_map[I][J] = data[1] & 0x3;
					}
				}
			}
		}
			break;
		case 0x07:  /* ATTR_CHR */
		{
			uint16_t I, sets;
			uint8_t x, y;
			sets = (data[3] | (data[4] << 8) );
			if( sets > 360 )
				sets = 360;
			sets >>= 2;
			sets += 6;
			x = data[1];
			y = data[2];
			if( data[5] ) /* Vertical */
			{
				for( I = 6; I < sets; I++ )
				{
					m_sgb_pal_map[x][y++] = (data[I] & 0xC0) >> 6;
					if( y > 17 )
					{
						y = 0;
						x++;
						if( x > 19 )
							x = 0;
					}

					m_sgb_pal_map[x][y++] = (data[I] & 0x30) >> 4;
					if( y > 17 )
					{
						y = 0;
						x++;
						if( x > 19 )
							x = 0;
					}

					m_sgb_pal_map[x][y++] = (data[I] & 0xC) >> 2;
					if( y > 17 )
					{
						y = 0;
						x++;
						if( x > 19 )
							x = 0;
					}

					m_sgb_pal_map[x][y++] = data[I] & 0x3;
					if( y > 17 )
					{
						y = 0;
						x++;
						if( x > 19 )
							x = 0;
					}
				}
			}
			else /* horizontal */
			{
				for( I = 6; I < sets; I++ )
				{
					m_sgb_pal_map[x++][y] = (data[I] & 0xC0) >> 6;
					if( x > 19 )
					{
						x = 0;
						y++;
						if( y > 17 )
							y = 0;
					}

					m_sgb_pal_map[x++][y] = (data[I] & 0x30) >> 4;
					if( x > 19 )
					{
						x = 0;
						y++;
						if( y > 17 )
							y = 0;
					}

					m_sgb_pal_map[x++][y] = (data[I] & 0xC) >> 2;
					if( x > 19 )
					{
						x = 0;
						y++;
						if( y > 17 )
							y = 0;
					}

					m_sgb_pal_map[x++][y] = data[I] & 0x3;
					if( x > 19 )
					{
						x = 0;
						y++;
						if( y > 17 )
							y = 0;
					}
				}
			}
		}
			break;
		case 0x08:  /* SOUND */
			/* This command enables internal sound effects */
			/* Not Implemented */
			break;
		case 0x09:  /* SOU_TRN */
			/* This command sends data to the SNES sound processor.
			 We'll need to emulate that for this to be used */
			/* Not Implemented */
			break;
		case 0x0A:  /* PAL_SET */
		{
			uint16_t index_;

			/* Palette 0 */
			index_ = (uint16_t)(data[1] | (data[2] << 8)) * 4;
			m_sgb_pal[0] = m_sgb_pal_data[index_];
			m_sgb_pal[1] = m_sgb_pal_data[index_ + 1];
			m_sgb_pal[2] = m_sgb_pal_data[index_ + 2];
			m_sgb_pal[3] = m_sgb_pal_data[index_ + 3];
			/* Palette 1 */
			index_ = (uint16_t)(data[3] | (data[4] << 8)) * 4;
			m_sgb_pal[4] = m_sgb_pal_data[index_];
			m_sgb_pal[5] = m_sgb_pal_data[index_ + 1];
			m_sgb_pal[6] = m_sgb_pal_data[index_ + 2];
			m_sgb_pal[7] = m_sgb_pal_data[index_ + 3];
			/* Palette 2 */
			index_ = (uint16_t)(data[5] | (data[6] << 8)) * 4;
			m_sgb_pal[8] = m_sgb_pal_data[index_];
			m_sgb_pal[9] = m_sgb_pal_data[index_ + 1];
			m_sgb_pal[10] = m_sgb_pal_data[index_ + 2];
			m_sgb_pal[11] = m_sgb_pal_data[index_ + 3];
			/* Palette 3 */
			index_ = (uint16_t)(data[7] | (data[8] << 8)) * 4;
			m_sgb_pal[12] = m_sgb_pal_data[index_];
			m_sgb_pal[13] = m_sgb_pal_data[index_ + 1];
			m_sgb_pal[14] = m_sgb_pal_data[index_ + 2];
			m_sgb_pal[15] = m_sgb_pal_data[index_ + 3];
			/* Attribute File */
			if (data[9] & 0x40)
				m_sgb_window_mask = 0;
			m_sgb_atf = (data[9] & 0x3f) * (18 * 5);
			if (data[9] & 0x80)
			{
				for (int j = 0; j < 18; j++ )
				{
					for (int i = 0; i < 5; i++ )
					{
						m_sgb_pal_map[i * 4][j] = (m_sgb_atf_data[(j * 5) + m_sgb_atf + i] & 0xC0) >> 6;
						m_sgb_pal_map[(i * 4) + 1][j] = (m_sgb_atf_data[(j * 5) + m_sgb_atf + i] & 0x30) >> 4;
						m_sgb_pal_map[(i * 4) + 2][j] = (m_sgb_atf_data[(j * 5) + m_sgb_atf + i] & 0xC) >> 2;
						m_sgb_pal_map[(i * 4) + 3][j] = m_sgb_atf_data[(j * 5) + m_sgb_atf + i] & 0x3;
					}
				}
			}
		}
			break;
		case 0x0B:  /* PAL_TRN */
		{
			uint16_t col;

			for (int i = 0; i < 2048; i++ )
			{
				col = (m_vram[0x0800 + (i * 2) + 1] << 8) | m_vram[0x0800 + (i * 2)];
				m_sgb_pal_data[i] = col;
			}
		}
			break;
		case 0x0C:  /* ATRC_EN */
			/* Not Implemented */
			break;
		case 0x0D:  /* TEST_EN */
			/* Not Implemented */
			break;
		case 0x0E:  /* ICON_EN */
			/* Not Implemented */
			break;
		case 0x0F:  /* DATA_SND */
			/* Not Implemented */
			break;
		case 0x10:  /* DATA_TRN */
			/* Not Implemented */
			break;
		case 0x11:  /* MLT_REQ */
			/* MLT_REQ currently handled inside gb.cpp logic */
			break;
		case 0x12:  /* JUMP */
			/* Not Implemented */
			break;
		case 0x13:  /* CHR_TRN */
			if (data[1] & 0x01U)
				sgb_vram_memcpy(m_sgb_tile_data.get() + 0x1000U, 0x00U, 0x100U);
			else
				sgb_vram_memcpy(m_sgb_tile_data.get(), 0x00U, 0x100U);
			break;
		case 0x14:  /* PCT_TRN */
		{
			uint16_t col;
			uint8_t sgb_pal[0x80U];

			sgb_vram_memcpy(m_sgb_tile_map, 0x00U, 0x80U);
			sgb_vram_memcpy(sgb_pal, 0x80U, 0x08U);
			for (int i = 0; i < 4 * 16 /* 4 pals at 16 colors each */; i++)
			{
				col = (sgb_pal[(i * 2) + 1] << 8) | sgb_pal[i * 2];
				m_sgb_pal[SGB_BORDER_PAL_OFFSET + i] = col;
			}
		}
			break;
		case 0x15:  /* ATTR_TRN */
			sgb_vram_memcpy(m_sgb_atf_data, 0x00U, 0x100U);
			break;
		case 0x16:  /* ATTR_SET */
		{
			/* Attribute File */
			if (data[1] & 0x40)
				m_sgb_window_mask = 0;
			m_sgb_atf = (data[1] & 0x3f) * (18 * 5);
			for (int j = 0; j < 18; j++)
			{
				for (int i = 0; i < 5; i++)
				{
					m_sgb_pal_map[i * 4][j] = (m_sgb_atf_data[(j * 5) + m_sgb_atf + i] & 0xC0) >> 6;
					m_sgb_pal_map[(i * 4) + 1][j] = (m_sgb_atf_data[(j * 5) + m_sgb_atf + i] & 0x30) >> 4;
					m_sgb_pal_map[(i * 4) + 2][j] = (m_sgb_atf_data[(j * 5) + m_sgb_atf + i] & 0xC) >> 2;
					m_sgb_pal_map[(i * 4) + 3][j] = m_sgb_atf_data[(j * 5) + m_sgb_atf + i] & 0x3;
				}
			}
		}
			break;
		case 0x17:  /* MASK_EN */
			m_sgb_window_mask = data[1];
			break;
		case 0x18:  /* OBJ_TRN */
			/* Not Implemented */
			break;
		case 0x19:  /* PAL_PRI */
			/* Called by: dkl,dkl2,dkl3,zeldadx */
			/* Not Implemented */
			break;
		case 0x1E:  /* Used by bootrom to transfer the gb cart header */
			break;
		case 0x1F:  /* Used by bootrom to transfer the gb cart header */
			break;
		default:
			logerror( "SGB: Unknown Command 0x%02x!\n", data[0] >> 3 );
	}

}
