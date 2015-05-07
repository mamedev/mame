// license:BSD-3-Clause
// copyright-holders:Juergen Buchmueller
/*****************************************************************************
 *
 *   Xerox AltoII display block
 *
 *****************************************************************************/
#ifdef  ALTO2_DEFINE_CONSTANTS

/**
 * @brief start value for the horizontal line counter
 *
 * This value is loaded into the three 4 bit counters (type 9316)
 * with numbers 65, 67, and 75.
 * 65: A=0 B=1 C=1 D=0
 * 67: A=1 B=0 C=0 D=1
 * 75: A=0 B=0 C=0 D=0
 *
 * The value is 150
 */
#define ALTO2_DISPLAY_HLC_START (2+4+16+128)

/**
 * @brief end value for the horizontal line counter
 *
 * This is decoded by H30, an 8 input NAND gate.
 * The value is 1899; horz. line count range 150...1899 = 1750.
 *
 * There are 1750 / 2 = 875 total scanlines.
 */
#define ALTO2_DISPLAY_HLC_END (1+2+8+32+64+256+512+1024)

/**
 * @brief display total height, including overscan (vertical blanking and synch)
 *
 * The display is interleaved in two fields, alternatingly drawing the even and odd
 * scanlines to the monitor. The frame rate is 60Hz, which is actually the rate
 * of the half-frames. The rate for full frames is thus 30Hz.
 */
#define ALTO2_DISPLAY_TOTAL_HEIGHT ((ALTO2_DISPLAY_HLC_END + 1 - ALTO2_DISPLAY_HLC_START) / 2)

/**
 * @brief display total width, including horizontal blanking
 *
 * Known facts:
 *
 * We have 606x808 visible pixels, and the pixel clock is said to be 50ns
 * (20MHz), while the crystal in the schematics is labeled 20.16 MHz,
 * so the pixel clock would actually be 49.6031ns.
 *
 * The total number of scanlines is, according to the docs, 875.
 *
 * 875 scanlines at 30 frames per second, thus the scanline rate is 26.250 kHz.
 *
 * If I divide 20.16 MHz by 26.250 kHz, I get 768 pixels for the total width
 * of a scanline in pixels.
 *
 * The horizontal blanking period would then be 768 - 606 = 162 pixels, and
 * thus 162 * 49.6031ns ~= 8036ns = 8.036us for the HBLANK time.
 *
 * In the display schematics there is a divide by 24 logic, and when
 * dividing the 768 pixels per scanline by 24, we have 32 phases of a scanline.
 *
 * A S8223 PROM (a63) with 32x8 bits contains the status of the HBLANK and
 * HSYNC signals for these phases, the SCANEND and HLCGATE signals, as well
 * as its own next address A0-A3!
 *
 */
#define ALTO2_DISPLAY_TOTAL_WIDTH 768


#define ALTO2_DISPLAY_FIFO 16                                                       //!< the display fifo has 16 words
#define ALTO2_DISPLAY_SCANLINE_WORDS (ALTO2_DISPLAY_TOTAL_WIDTH/16)                 //!< words per scanline
#define ALTO2_DISPLAY_HEIGHT 808                                                    //!< number of visible scanlines per frame; 808 really, but there are some empty lines?
#define ALTO2_DISPLAY_WIDTH 606                                                     //!< visible width of the display; 38 x 16 bit words - 2 pixels
#define ALTO2_DISPLAY_VISIBLE_WORDS ((ALTO2_DISPLAY_WIDTH+15)/16)                   //!< visible words per scanline
#define ALTO2_DISPLAY_BITCLOCK 20160000ll                                           //!< display bit clock in Hertz (20.16MHz)
#define ALTO2_DISPLAY_BITTIME(n) (U64(1000000000000)*(n)/ALTO2_DISPLAY_BITCLOCK)    //!< display bit time in pico seconds (~= 49.6031ns)
#define ALTO2_DISPLAY_SCANLINE_TIME ALTO2_DISPLAY_BITTIME(ALTO2_DISPLAY_TOTAL_WIDTH)//!< time for a scanline in pico seconds (768 * 49.6031ns ~= 38095.1808ns)
#define ALTO2_DISPLAY_VISIBLE_TIME ALTO2_DISPLAY_BITTIME(ALTO2_DISPLAY_WIDTH)       //!< time of the visible part of a scanline in pico seconds (606 * 49.6031ns ~= 30059.4786ns)
#define ALTO2_DISPLAY_WORD_TIME ALTO2_DISPLAY_BITTIME(16)                           //!< time for a word in pico seconds (16 pixels * 49.6031ns ~= 793.6496ns)
#define ALTO2_DISPLAY_VBLANK_TIME ((ALTO2_DISPLAY_TOTAL_HEIGHT-ALTO2_DISPLAY_HEIGHT)*HZ_TO_ATTOSECONDS(26250)/2)

#else   // ALTO2_DEFINE_CONSTANTS
/**
 * @brief structure of the display context
 *
 * Schematics of the task clear and wakeup signal generators
 * <PRE>
 * A quote (') appended to a signal name means inverted signal.
 *
 *  AND |     NAND|      NOR |       FF | Q    N174
 * -----+--- -----+---  -----+---  -----+---   -----
 *  0 0 | 0   0 0 | 1    0 0 | 1    S'\0| 1    delay
 *  0 1 | 0   0 1 | 1    0 1 | 0    R'\0| 0
 *  1 0 | 0   1 0 | 1    1 0 | 0
 *  1 1 | 1   1 1 | 0    1 1 | 0
 *
 *
 *                                                       DVTAC'
 *                                                      >-------+  +-----+
 *                                                              |  |  FF |
 * VBLANK'+----+ DELVBLANK' +---+  DELVBLANK   +----+           +--|S'   |
 * >------|N174|------+-----|inv|--------------|NAND| VBLANKPULSE  |     |              WAKEDVT'
 *        +----+      |     +---+              |    o--+-----------|R'  Q|---------------------->
 *                    |                      +-|    |  |           |     |
 *        +----+      |     DDELVBLANK'      | +----+  |           +-----+
 *      +-|N174|-----------------------------+         |      +---+
 *      | +----+      |                                +------oAND|
 *      |             |                      DSP07.01  |      |   o----------+
 *      +-------------+                      >---------|------o   |          |
 *                                                     |      +---+          |
 *                                                     |                     |
 *                                                     | +-----+             |
 *                                                     | |  FF |             |  +-----+
 *        DHTAC'       +---+                           | |     |             |  |  FF |
 *      >--------------oNOR|  *07.25       +----+      +-|S'   |   DHTBLK'   |  |     |
 *        BLOCK'       |   |---------------|NAND|        |    Q|--+----------|--|S1'  | WAKEDHT'
 *      >--------------o   |     DCSYSCLK  |    o--------|R'   |  | >--------|--|S2' Q|--------->
 *                     +---+     >---------|    |        +-----+  |  DHTAC'  +--|R'   |
 *                                         +----+                 |             +-----+
 *                                                   +------------+
 *                                                   |
 *        DWTAC'       +---+                         |   +-----+
 *      >--------------oNOR|  *07.26 +----+          |   |  FF |
 *        BLOCK'       |   |---------|NAND| DSP07.01 |   |     |
 *      >--------------o   | DCSYSCLK|    o----------|---|S1'  | DWTCN' +---+        DWTCN
 *                     +---+ >-------|    |          +---|S2' Q|--------|inv|-----------+----
 *                                   +----+          +---|R'   |        +---+           |
 *                                                   |   +-----+                        |
 *                 SCANEND     +----+                |                                  |
 *               >-------------|NAND|  CLRBUF'       |           .----------------------+
 *                 DCLK        |    o----------------+           |
 *               >-------------|    |                            |  +-----+
 *                             +----+                            +--| NAND|
 *                                                       STOPWAKE'  |     |preWake +----+ WAKEDWT'
 *                                                      >-----------|     o--------|N174|--------->
 *                                                        VBLANK'   |     |        +----+
 *                                                      >-----------|     |
 *                                                                  +-----+
 *                                                     a40c
 *                                        VBLANKPULSE +----+
 *                                       -------------|NAND|
 *                                                    |    o--+
 *                                                 +--|    |  |
 *                                                 |  +----+  |
 *                                                 +----------|-+
 *                                                 +----------+ |
 *        CURTAC'      +---+                       |  +----+    |     a20d
 *      >--------------oNOR|  *07.27 +----+        +--|NAND|    |    +----+
 *        BLOCK'       |   |---------|NAND| DSP07.07  |    o----+----o NOR| preWK  +----+ WAKECURT'
 *      >--------------o   | DCSYSCLK|    o-----------|    |         |    |--------|N174|--------->
 *                     +---+ >-------|    |           +----+    +----o    |        +----+
 *                                   +----+            a40d     |    +----+
 *                                          a30c                |
 *                              CURTAC'    +----+               |
 *                            >------------|NAND|    DSP07.03   |
 *                                         |    o--+------------+
 *                                      +--|    |  |
 *                                      |  +----+  |
 *                                      +----------|-+
 *                                      +----------+ |
 *                                      |  +----+    |
 *                                      +--|NAND|    |
 *                              CLRBUF'    |    o----+
 *                            >------------|    |
 *                                         +----+
 *                                          a30d
 * </PRE>
 */

#ifndef _A2DISP_H_
#define _A2DISP_H_
struct {
	UINT16 state;                       //!< current state of the display_state_machine()
	UINT16 hlc;                         //!< horizontal line counter
	UINT16 setmode;                     //!< value written by last SETMODE<-
	UINT16 inverse;                     //!< set to 0xffff if line is inverse, 0x0000 otherwise
	bool halfclock;                     //!< set 0 for normal pixel clock, 1 for half pixel clock
	UINT16 fifo[ALTO2_DISPLAY_FIFO];    //!< display word fifo
	UINT8 wa;                           //!< fifo input pointer (write address; 4-bit)
	UINT8 ra;                           //!< fifo output pointer (read address; 4-bit)
	UINT8 a63;                          //!< most recent value read from the PROM a63
	UINT8 a66;                          //!< most recent value read from the PROM a66
	bool dht_blocks;                    //!< set non-zero, if the DHT executed BLOCK
	bool dwt_blocks;                    //!< set non-zero, if the DWT executed BLOCK
	bool curt_blocks;                   //!< set non-zero, if the CURT executed BLOCK
	bool curt_wakeup;                   //!< set non-zero, if CURT wakeups are generated
	UINT16 vblank;                      //!< most recent HLC with VBLANK still high (11-bit)
	UINT16 xpreg;                       //!< cursor cursor x position register (10-bit)
	UINT16 csr;                         //!< cursor shift register (16-bit)
	UINT32 curxpos;                     //!< helper: first cursor word in scanline
	UINT16 cursor0;                     //!< helper: shifted cursor data for left word
	UINT16 cursor1;                     //!< helper: shifted cursor data for right word
	UINT16 *raw_bitmap;                 //!< array of words of the raw bitmap that is displayed
	UINT8 **scanline;                   //!< array of scanlines with 1 byte per pixel
	bitmap_ind16 *bitmap;               //!< MAME bitmap with 16 bit indices
	bool odd_frame;                     //!< true, if odd frame is drawn
}   m_dsp;

/**
 * @brief PROM a38 contains the STOPWAKE' and MBEMBPTY' signals for the FIFO
 * <PRE>
 * The inputs to a38 are the UNLOAD counter RA[0-3] and the DDR<- counter
 * WA[0-3], and the designer decided to reverse the address lines :-)
 *
 *  a38  counter FIFO counter
 *  --------------------------
 *   A0  RA[0]   fifo_rd
 *   A1  RA[1]
 *   A2  RA[2]
 *   A3  RA[3]
 *   A4  WA[0]   fifo_wr
 *   A5  WA[1]
 *   A6  WA[2]
 *   A7  WA[3]
 *
 * Only two bits of a38 are used:
 *  O1 (002) = STOPWAKE'
 *  O3 (010) = MBEMPTY'
 * </PRE>
 */
UINT8* m_disp_a38;

//! output bits of PROM A38
enum {
	disp_a38_STOPWAKE   = (1 << 1),
	disp_a38_MBEMPTY    = (1 << 3)
};

/**
 * @brief emulation of PROM a63 in the display schematics page 8
 * <PRE>
 * The PROM's address lines are driven by a clock CLK, which is
 * pixel clock / 24, and an inverted half-scanline signal H[1]'.
 *
 * It is 32x8 bits and its output bits (B) are connected to the
 * signals, as well as its own address lines (A) through a latch
 * of the type SN74774 like this:
 *
 * B    174     A   others
 * ------------------------
 * 0     5      -   HBLANK
 * 1     0      -   HSYNC
 * 2     4      0
 * 3     1      1
 * 4     3      2
 * 5     2      3
 * 6     -      -   SCANEND
 * 7     -      -   HLCGATE
 * ------------------------
 * H[1]' -      4
 *
 * The display_state_machine() is called by the CPU at a rate of pixelclock/24,
 * which happens to be very close to every 7th CPU micrcocycle.
 * </PRE>
 */
UINT8* m_disp_a63;

enum {
	disp_a63_HBLANK     = (1 << 0),         //!< PROM a63 B0 is latched as HBLANK signal
	disp_a63_HSYNC      = (1 << 1),         //!< PROM a63 B1 is latched as HSYNC signal
	disp_a63_A0         = (1 << 2),         //!< PROM a63 B2 is the latched next address bit A0
	disp_a63_A1         = (1 << 3),         //!< PROM a63 B3 is the latched next address bit A1
	disp_a63_A2         = (1 << 4),         //!< PROM a63 B4 is the latched next address bit A2
	disp_a63_A3         = (1 << 5),         //!< PROM a63 B5 is the latched next address bit A3
	disp_a63_SCANEND    = (1 << 6),         //!< PROM a63 B6 SCANEND signal, which resets the FIFO counters
	disp_a63_HLCGATE    = (1 << 7)          //!< PROM a63 B7 HLCGATE signal, which enables counting the HLC
};

/**
 * @brief vertical blank and synch PROM
 *
 * PROM a66 is a 256x4 bit (type 3601), containing the vertical blank + synch.
 * Address lines are driven by H[1] to H[128] of the horz. line counters.
 * The PROM is enabled whenever H[256] and H[512] are both 0.
 */
UINT8* m_disp_a66;

enum {
	disp_a66_VSYNC_ODD      = (1 << 0),     //!< Q1 (001) is VSYNC for the odd field (with H1024=1)
	disp_a66_VSYNC_EVEN     = (1 << 1),     //!< Q2 (002) is VSYNC for the even field (with H1024=0)
	disp_a66_VBLANK_ODD     = (1 << 2),     //!< Q3 (004) is VBLANK for the odd field (with H1024=1)
	disp_a66_VBLANK_EVEN    = (1 << 3)      //!< Q4 (010) is VBLANK for the even field (with H1024=0)
};

void update_bitmap_word(UINT16* bitmap, int x, int y, UINT16 word); //!< update a word in the screen bitmap
void unload_word();                 //!< unload the next word from the display FIFO and shift it to the screen
void display_state_machine();       //!< function called by the CPU to enter the next display state

void f2_late_evenfield(void);       //!< branch on the evenfield flip-flop

void init_disp();                   //!< initialize the display context
void exit_disp();                   //!< deinitialize the display context
void reset_disp();                  //!< reset the display context

void fake_status_putch(int x, UINT8 ch);
void fake_status_printf(int x, const char* format, ...);

#endif  // _A2DISP_H_
#endif  // ALTO2_DEFINE_CONSTANTS
