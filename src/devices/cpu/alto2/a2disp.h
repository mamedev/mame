// license:BSD-3-Clause
// copyright-holders:Juergen Buchmueller
/*****************************************************************************
 *
 *   Xerox AltoII display emulation
 *
 *****************************************************************************/
#ifdef  ALTO2_DEFINE_CONSTANTS

/**
 * @brief Start value for the horizontal line counter.
 *
 * This value is loaded into the three 4 bit counters (type 9316)
 * with numbers 65, 67, and 75.
 * 65: A=0 B=1 C=1 D=0
 * 67: A=1 B=0 C=0 D=1
 * 75: A=0 B=0 C=0 D=0
 *
 * The value is 2+4+16+128 = 150
 */
#define A2_DISP_HLC_START (2+4+16+128)

/**
 * @brief End value for the horizontal line counter.
 *
 * This is decoded by H30, an 8 input NAND gate.
 * The value is 1899; horz. line count range 150...1899 = 1750.
 * So there are 1750 / 2 = 875 total scanlines.
 *
 * Note: The horizontal line counts 150 ... 1023 for the even field,
 * and 1024 ... 1899 for the odd field.
 */
#define A2_DISP_HLC_END (1+2+8+32+64+256+512+1024)

/**
 * @brief display total height, including overscan (vertical blanking and synch)
 *
 * The display is interleaved in two fields, alternatingly drawing the even and odd
 * scanlines to the monitor. The frame rate is 60Hz, which is actually the rate
 * of the half-frames. The rate for full frames is thus 30Hz.
 */
#define A2_DISP_TOTAL_HEIGHT (static_cast<double>(A2_DISP_HLC_END - A2_DISP_HLC_START) / 2)

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
#define A2_DISP_TOTAL_WIDTH 768

//! The display fifo has 16 words.
#define A2_DISP_FIFO 16

//! Words per scanline.
#define A2_DISP_SCANLINE_WORDS (A2_DISP_TOTAL_WIDTH/16)

//! Number of visible scanlines per frame
#define A2_DISP_HEIGHT 808

//! Visible width of the display; 38 x 16 bit words - 2 pixels.
#define A2_DISP_WIDTH 606

//! Visible words per scanline.
#define A2_DISP_VISIBLE_WORDS ((A2_DISP_WIDTH+15)/16)

//! Display bit clock in Hertz (20.16MHz).
#define A2_DISP_BITCLOCK 20160000ll

//! Display bit time in pico seconds (~= 49.6031ns).
#define A2_DISP_BITTIME(n) DOUBLE_TO_ATTOSECONDS(static_cast<double>(n)/A2_DISP_BITCLOCK)

//! Time for a scanline in pico seconds (768 * 49.6031ns ~= 38095.1808ns).
#define A2_DISP_SCANLINE_TIME A2_DISP_BITTIME(A2_DISP_TOTAL_WIDTH)

//!< Time of the visible part of a scanline in pico seconds (606 * 49.6031ns ~= 30059.4786ns).
#define A2_DISP_VISIBLE_TIME A2_DISP_BITTIME(A2_DISP_WIDTH)

//!< Time for a word in pico seconds (16 pixels * 49.6031ns ~= 793.6496ns).
#define A2_DISP_WORD_TIME A2_DISP_BITTIME(16)

#define A2_DISP_VBLANK_TIME (static_cast<double>(-A2_DISP_HEIGHT)*HZ_TO_ATTOSECONDS(26250))

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

#ifndef MAME_CPU_ALTO2_A2DISP_H
#define MAME_CPU_ALTO2_A2DISP_H
struct {
	uint32_t state = 0;                     //!< current state of the display_state_machine()
	uint32_t hlc = 0;                       //!< horizontal line counter
	uint32_t setmode = 0;                   //!< value written by last SETMODE<-
	uint32_t inverse = 0;                   //!< set to 0xffff if line is inverse, 0x0000 otherwise
	uint32_t scanline = 0;                  //!< current scanline
	bool halfclock = false;                 //!< false for normal pixel clock, true for half pixel clock
	bool vblank = false;                    //!< true during vblank, false otherwise
	uint16_t fifo[A2_DISP_FIFO] = { };      //!< display word fifo
	uint32_t wa = 0;                        //!< fifo input pointer (write address; 4-bit)
	uint32_t ra = 0;                        //!< fifo output pointer (read address; 4-bit)
	uint32_t a63 = 0;                       //!< most recent value read from the PROM a63
	uint32_t a66 = 0;                       //!< most recent value read from the PROM a66
	bool dht_blocks = false;                //!< set true, if the DHT executed BLOCK
	bool dwt_blocks = false;                //!< set true, if the DWT executed BLOCK
	bool curt_blocks = false;               //!< set true, if the CURT executed BLOCK
	bool curt_wakeup = false;               //!< set true, if CURT wakeups are generated
	uint32_t xpreg = 0;                     //!< cursor cursor x position register (10-bit)
	uint32_t csr = 0;                       //!< cursor shift register (16-bit)
	std::unique_ptr<uint16_t[]> framebuf;   //!< array of words of the raw bitmap that is displayed
	std::unique_ptr<uint8_t[]> patterns;    //!< array of 65536 patterns (16 bytes) with 1 byte per pixel
	std::unique_ptr<bitmap_ind16> bitmap;   //!< MAME bitmap with 16 bit indices
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
std::unique_ptr<uint8_t[]> m_disp_a38;

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
 * which happens to be very close to every 7th CPU microcycle.
 * </PRE>
 */
std::unique_ptr<uint8_t[]> m_disp_a63;

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
std::unique_ptr<uint8_t[]> m_disp_a66;

enum {
	disp_a66_VSYNC_ODD      = (1 << 0),     //!< Q1 (001) is VSYNC for the odd field (with H1024=1)
	disp_a66_VSYNC_EVEN     = (1 << 1),     //!< Q2 (002) is VSYNC for the even field (with H1024=0)
	disp_a66_VBLANK_ODD     = (1 << 2),     //!< Q3 (004) is VBLANK for the odd field (with H1024=1)
	disp_a66_VBLANK_EVEN    = (1 << 3)      //!< Q4 (010) is VBLANK for the even field (with H1024=0)
};

void update_framebuf_word(uint16_t* framebuf, int x, int y, uint16_t word);
void unload_word();                 //!< unload the next word from the display FIFO and shift it to the screen
void display_state_machine();       //!< function called by the CPU execution loop to enter the next display state

void f2_late_evenfield(void);       //!< branch on the evenfield flip-flop

void init_disp();                   //!< initialize the display context
void exit_disp();                   //!< deinitialize the display context
void reset_disp();                  //!< reset the display context

#endif  // MAME_CPU_ALTO2_A2DISP_H
#endif  // ALTO2_DEFINE_CONSTANTS
