// license:BSD-3-Clause
// copyright-holders:Juergen Buchmueller
/*****************************************************************************
 *
 *   Xerox AltoII display interface
 *
 *****************************************************************************/
#include "alto2cpu.h"
#include "a2roms.h"

/**
 * @brief PROM a38 contains the STOPWAKE' and MBEMBPTY' signals for the FIFO
 * The inputs to a38 are the UNLOAD counter RA[0-3] and the DDR<- counter
 * WA[0-3], and the designer decided to reverse the address lines :-)
 *
 * <PRE>
 *  a38  counter
 *  -------------
 *   A0  RA[0]
 *   A1  RA[1]
 *   A2  RA[2]
 *   A3  RA[3]
 *   A4  WA[0]
 *   A5  WA[1]
 *   A6  WA[2]
 *   A7  WA[3]
 *
 * Only two bits of a38 are used:
 *  O1 (002) = STOPWAKE'
 *  O3 (010) = MBEMPTY'
 * </PRE>
 */

//! P3601 256x4 BPROM; display FIFO control: STOPWAKE, MBEMPTY
static const prom_load_t pl_displ_a38 =
{
	"displ.a38",
	nullptr,
	"fd30beb7",
	"65e4a19ba4ff748d525122128c514abedd55d866",
	/* size */  0400,
	/* amap */  AMAP_REVERSE_0_7,           // reverse address lines A0-A7
	/* axor */  0,
	/* dxor */  0,
	/* width */ 4,
	/* shift */ 0,
	/* dmap */  DMAP_DEFAULT,
	/* dand */  ZERO,
	/* type */  sizeof(UINT8)
};

//! PROM a38 bit O1 is STOPWAKE' (stop DWT if bit is zero)
#define FIFO_STOPWAKE(a38) ((a38 & disp_a38_STOPWAKE) ? false : true)

//! PROM a38 bit O3 is MBEMPTY' (FIFO is empty if bit is zero)
#define FIFO_MBEMPTY(a38) ((a38 & disp_a38_MBEMPTY) ? false : true)

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
 *  PROM  174   A   others
 *  ------------------------
 *  B0    D5    -   HBLANK
 *  B1    D0    -   HSYNC
 *  B2    D4    A0  -
 *  B3    D1    A1  -
 *  B4    D3    A2  -
 *  B5    D2    A3  -
 *  B6    -     -   SCANEND
 *  B7    -     -   HLCGATE
 *  ------------------------
 *  H[1]' -     A4  -
 *
 * The display_state_machine() is called at a rate of pixelclock/24.
 *
 * Decoded states of this PROM:
 *
 *  STATE  PROM   binary   HBLANK  HSYNC NEXT SCANEND HLCGATE
 *  ----------------------------------------------------------
 *    000  0007  00000111     1      1    001    0       0
 *    001  0013  00001011     1      1    002    0       0
 *    002  0015  00001101     1      0    003    0       0
 *    003  0021  00010001     1      0    004    0       0
 *    004  0024  00010100     0      0    005    0       0
 *    005  0030  00011000     0      0    006    0       0
 *    006  0034  00011100     0      0    007    0       0
 *    007  0040  00100000     0      0    010    0       0
 *    010  0044  00100100     0      0    011    0       0
 *    011  0050  00101000     0      0    012    0       0
 *    012  0054  00101100     0      0    013    0       0
 *    013  0060  00110000     0      0    014    0       0
 *    014  0064  00110100     0      0    015    0       0
 *    015  0070  00111000     0      0    016    0       0
 *    016  0074  00111100     0      0    017    0       0
 *    017  0200  10000000     0      0    000    0       1
 *    020  0004  00000100     0      0    001    0       0
 *    021  0010  00001000     0      0    002    0       0
 *    022  0014  00001100     0      0    003    0       0
 *    023  0020  00010000     0      0    004    0       0
 *    024  0024  00010100     0      0    005    0       0
 *    025  0030  00011000     0      0    006    0       0
 *    026  0034  00011100     0      0    007    0       0
 *    027  0040  00100000     0      0    010    0       0
 *    030  0044  00100100     0      0    011    0       0
 *    031  0050  00101000     0      0    012    0       0
 *    032  0054  00101100     0      0    013    0       0
 *    033  0060  00110000     0      0    014    0       0
 *    034  0064  00110100     0      0    015    0       0
 *    035  0070  00111000     0      0    016    0       0
 *    036  0175  01111101     1      0    017    1       0
 *    037  0203  10000011     1      1    000    0       1
 * </PRE>
 */

//! 82S23 32x8 BPROM; display HBLANK, HSYNC, SCANEND, HLCGATE ...
static const prom_load_t pl_displ_a63 =
{
	"displ.a63",
	nullptr,
	"82a20d60",
	"39d90703568be5419ada950e112d99227873fdea",
	/* size */  0040,
	/* amap */  AMAP_DEFAULT,
	/* axor */  0,
	/* dxor */  0,
	/* width */ 8,
	/* shift */ 0,
	/* dmap */  DMAP_DEFAULT,
	/* dand */  ZERO,
	/* type */  sizeof(UINT8)
};

//!< test the HBLANK (horizontal blanking) signal in PROM a63 being high
#define A63_HBLANK(a) ((a & disp_a63_HBLANK) ? true : false)

//!< test the HSYNC (horizontal synchonisation) signal in PROM a63 being high
#define A63_HSYNC(a) ((a & disp_a63_HSYNC) ? true : false)

//!< test the SCANEND (scanline end) signal in PROM a63 being high
#define A63_SCANEND(a) ((a & disp_a63_SCANEND) ? true : false)

//!< test the HLCGATE (horz. line counter gate) signal in PROM a63 being high
#define A63_HLCGATE(a) ((a & disp_a63_HLCGATE) ? true : false)

/**
 * @brief PROM a66 is a 256x4 bit (type 3601)
 * <PRE>
 * Address lines are driven by H[1] to H[128] of the horz. line counters.
 * PROM is enabled when H[256] and H[512] are both 0.
 *
 * Q1 is VSYNC for the odd field (with H1024=1)
 * Q2 is VSYNC for the even field (with H1024=0)
 * Q3 is VBLANK for the odd field (with H1024=1)
 * Q4 is VBLANK for the even field (with H1024=0)
 * </PRE>
 */

//! P3601 256x4 BPROM; display VSYNC and VBLANK
static const prom_load_t pl_displ_a66 =
{
	"displ.a66",
	nullptr,
	"9f91aad9",
	"69b1d4c71f4e18103112e8601850c2654e9265cf",
	/* size */  0400,
	/* amap */  AMAP_DEFAULT,
	/* axor */  0,
	/* dxor */  0,
	/* width */ 4,
	/* shift */ 0,
	/* dmap */  DMAP_DEFAULT,
	/* dand */  ZERO,
	/* type */  sizeof(UINT8)
};

//! test the VSYNC (vertical synchronisation) signal in PROM a66 being high
#define A66_VSYNC(a) (a & (HLC1024 ? disp_a66_VSYNC_ODD : disp_a66_VSYNC_EVEN) ? false : true)

//! test the VBLANK (vertical blanking) signal in PROM a66 being high
#define A66_VBLANK(a) (a & (HLC1024 ? disp_a66_VBLANK_ODD : disp_a66_VBLANK_EVEN) ? false : true)

/**
 * @brief double the bits for a byte (left and right of display word) to a word
 */
static const UINT16 double_bits[256] = {
	0x0000,0x0003,0x000c,0x000f,0x0030,0x0033,0x003c,0x003f,
	0x00c0,0x00c3,0x00cc,0x00cf,0x00f0,0x00f3,0x00fc,0x00ff,
	0x0300,0x0303,0x030c,0x030f,0x0330,0x0333,0x033c,0x033f,
	0x03c0,0x03c3,0x03cc,0x03cf,0x03f0,0x03f3,0x03fc,0x03ff,
	0x0c00,0x0c03,0x0c0c,0x0c0f,0x0c30,0x0c33,0x0c3c,0x0c3f,
	0x0cc0,0x0cc3,0x0ccc,0x0ccf,0x0cf0,0x0cf3,0x0cfc,0x0cff,
	0x0f00,0x0f03,0x0f0c,0x0f0f,0x0f30,0x0f33,0x0f3c,0x0f3f,
	0x0fc0,0x0fc3,0x0fcc,0x0fcf,0x0ff0,0x0ff3,0x0ffc,0x0fff,
	0x3000,0x3003,0x300c,0x300f,0x3030,0x3033,0x303c,0x303f,
	0x30c0,0x30c3,0x30cc,0x30cf,0x30f0,0x30f3,0x30fc,0x30ff,
	0x3300,0x3303,0x330c,0x330f,0x3330,0x3333,0x333c,0x333f,
	0x33c0,0x33c3,0x33cc,0x33cf,0x33f0,0x33f3,0x33fc,0x33ff,
	0x3c00,0x3c03,0x3c0c,0x3c0f,0x3c30,0x3c33,0x3c3c,0x3c3f,
	0x3cc0,0x3cc3,0x3ccc,0x3ccf,0x3cf0,0x3cf3,0x3cfc,0x3cff,
	0x3f00,0x3f03,0x3f0c,0x3f0f,0x3f30,0x3f33,0x3f3c,0x3f3f,
	0x3fc0,0x3fc3,0x3fcc,0x3fcf,0x3ff0,0x3ff3,0x3ffc,0x3fff,
	0xc000,0xc003,0xc00c,0xc00f,0xc030,0xc033,0xc03c,0xc03f,
	0xc0c0,0xc0c3,0xc0cc,0xc0cf,0xc0f0,0xc0f3,0xc0fc,0xc0ff,
	0xc300,0xc303,0xc30c,0xc30f,0xc330,0xc333,0xc33c,0xc33f,
	0xc3c0,0xc3c3,0xc3cc,0xc3cf,0xc3f0,0xc3f3,0xc3fc,0xc3ff,
	0xcc00,0xcc03,0xcc0c,0xcc0f,0xcc30,0xcc33,0xcc3c,0xcc3f,
	0xccc0,0xccc3,0xcccc,0xcccf,0xccf0,0xccf3,0xccfc,0xccff,
	0xcf00,0xcf03,0xcf0c,0xcf0f,0xcf30,0xcf33,0xcf3c,0xcf3f,
	0xcfc0,0xcfc3,0xcfcc,0xcfcf,0xcff0,0xcff3,0xcffc,0xcfff,
	0xf000,0xf003,0xf00c,0xf00f,0xf030,0xf033,0xf03c,0xf03f,
	0xf0c0,0xf0c3,0xf0cc,0xf0cf,0xf0f0,0xf0f3,0xf0fc,0xf0ff,
	0xf300,0xf303,0xf30c,0xf30f,0xf330,0xf333,0xf33c,0xf33f,
	0xf3c0,0xf3c3,0xf3cc,0xf3cf,0xf3f0,0xf3f3,0xf3fc,0xf3ff,
	0xfc00,0xfc03,0xfc0c,0xfc0f,0xfc30,0xfc33,0xfc3c,0xfc3f,
	0xfcc0,0xfcc3,0xfccc,0xfccf,0xfcf0,0xfcf3,0xfcfc,0xfcff,
	0xff00,0xff03,0xff0c,0xff0f,0xff30,0xff33,0xff3c,0xff3f,
	0xffc0,0xffc3,0xffcc,0xffcf,0xfff0,0xfff3,0xfffc,0xffff
};

#define HLC1    X_BIT(m_dsp.hlc,16,15)    //!< horizontal line counter bit 0 (mid of the scanline)
#define HLC2    X_BIT(m_dsp.hlc,16,14)    //!< horizontal line counter bit 1
#define HLC4    X_BIT(m_dsp.hlc,16,13)    //!< horizontal line counter bit 2
#define HLC8    X_BIT(m_dsp.hlc,16,12)    //!< horizontal line counter bit 3
#define HLC16   X_BIT(m_dsp.hlc,16,11)    //!< horizontal line counter bit 4
#define HLC32   X_BIT(m_dsp.hlc,16,10)    //!< horizontal line counter bit 5
#define HLC64   X_BIT(m_dsp.hlc,16, 9)    //!< horizontal line counter bit 6
#define HLC128  X_BIT(m_dsp.hlc,16, 8)    //!< horizontal line counter bit 7
#define HLC256  X_BIT(m_dsp.hlc,16, 7)    //!< horizontal line counter bit 8
#define HLC512  X_BIT(m_dsp.hlc,16, 6)    //!< horizontal line counter bit 9
#define HLC1024 X_BIT(m_dsp.hlc,16, 5)    //!< horizontal line counter bit 10 (odd/even field

#define GET_SETMODE_SPEEDY(mode) X_RDBITS(mode,16,0,0)  //!< get the pixel clock speed from a SETMODE<- bus value
#define GET_SETMODE_INVERSE(mode) X_RDBITS(mode,16,1,1) //!< get the inverse video flag from a SETMODE<- bus value

//!< helper to extract A3-A0 from a PROM a63 value
#define A63_NEXT(n) ((n >> 2) & 017)

//! update the internal frame buffer and draw the scanline segment if changed
void alto2_cpu_device::update_framebuf_word(UINT16* framebuf, int x, int y, UINT16 word)
{
	// mixing with the cursor
	if (x == m_dsp.curxpos + 0)
		word ^= m_dsp.cursor0;
	if (x == m_dsp.curxpos + 1)
		word ^= m_dsp.cursor1;
	// no change?
	if (word == framebuf[x])
		return;
	framebuf[x] = word;
	draw_scanline8(*m_dsp.bitmap, x * 16, y, 16, m_dsp.patterns + 16 * word, nullptr);
}

/**
 * @brief unload the next word from the display FIFO and shift it to the screen
 */
void alto2_cpu_device::unload_word()
{
	int x = m_unload_word;
	int y = ((m_dsp.hlc - m_dsp.vblank) & ~02001) ^ HLC1024;

	if (y < 0 || y >= ALTO2_DISPLAY_HEIGHT || x >= ALTO2_DISPLAY_VISIBLE_WORDS)
	{
		m_unload_time = -1;
		return;
	}
	UINT16* framebuf = m_dsp.framebuf.get()  + y * ALTO2_DISPLAY_SCANLINE_WORDS;
	UINT16 word = m_dsp.inverse;
	UINT8 a38 = m_disp_a38[m_dsp.ra * 16 + m_dsp.wa];
	if (FIFO_MBEMPTY(a38))
	{
		LOG((this,LOG_DISPL,1, " DSP FIFO underrun y:%d x:%d\n", y, x));
	}
	else
	{
		word ^= m_dsp.fifo[m_dsp.ra];
		m_dsp.ra = (m_dsp.ra + 1) % ALTO2_DISPLAY_FIFO;
		LOG((this,LOG_DISPL,3, " DSP pull %04x from FIFO[%02o] y:%d x:%d\n",
				word, (m_dsp.ra - 1) & (ALTO2_DISPLAY_FIFO - 1), y, x));
	}

	if (m_dsp.halfclock)
	{
		const UINT16 word1 = double_bits[word / 256];
		update_framebuf_word(framebuf, x, y, word1);
		x++;
		if (x < ALTO2_DISPLAY_VISIBLE_WORDS)
		{
			const UINT16 word2 = double_bits[word % 256];
			update_framebuf_word(framebuf, x, y, word2);
			x++;
		}
		m_unload_time += ALTO2_DISPLAY_BITTIME(32);
	}
	else
	{
		update_framebuf_word(framebuf, x, y, word);
		x++;
		m_unload_time += ALTO2_DISPLAY_BITTIME(16);
	}
	if (x < ALTO2_DISPLAY_VISIBLE_WORDS)
		m_unload_word = x;
	else
		m_unload_time = -1;
}


/**
 * @brief function called by the CPU to enter the next display state
 *
 * There are 32 states per scanline and 875 scanlines per frame.
 */
void alto2_cpu_device::display_state_machine()
{
	LOG((this,LOG_DISPL,5,"DSP%03o:", m_dsp.state));
	if (020 == m_dsp.state)
	{
		LOG((this,LOG_DISPL,2," HLC=%d", m_dsp.hlc));
	}

	const UINT8 a63 = m_disp_a63[m_dsp.state];
	if (A63_HLCGATE(a63))
	{
		// count horizontal line counters and wrap
		m_dsp.hlc += 1;
		if (m_dsp.hlc > ALTO2_DISPLAY_HLC_END)
			m_dsp.hlc = ALTO2_DISPLAY_HLC_START;
		// wake up the memory refresh task _twice_ on each scanline
		m_task_wakeup |= 1 << task_mrt;
	}
	// PROM a66 is disabled, if any of HLC256 or HLC512 are high
	const UINT8 a66 = (HLC256 | HLC512) ? 017 : m_disp_a66[m_dsp.hlc & 0377];

	// next address from PROM a63, use A4 from HLC1
	const UINT8 next = ((HLC1 ^ 1) << 4) | A63_NEXT(a63);

	if (A66_VBLANK(a66))
	{
		// Rising edge of VBLANK: remember HLC[1-10] where the VBLANK starts
		m_dsp.vblank = m_dsp.hlc & ~(1 << 10);

		LOG((this,LOG_DISPL,1, " VBLANK"));

		// VSYNC is always within VBLANK, thus we handle it only here
		if (A66_VSYNC(a66) && !A66_VSYNC(m_dsp.a66))
		{
			LOG((this,LOG_DISPL,1, " VSYNC/ (wake DVT)"));
			/*
			 * The display vertical task DVT is woken once per field
			 * at the beginning of vertical retrace.
			 */
			m_task_wakeup |= 1 << task_dvt;
		}
	}
	else
	{
		// Falling edge of VBLANK?
		if (A66_VBLANK(m_dsp.a66))
		{
			/*
			 * VBLANKPULSE:
			 * The display horizontal task DHT is woken once at the
			 * beginning of each field, and thereafter whenever the
			 * display word task blocks.
			 *
			 * The DHT can block itself, in which case neither it nor
			 * the word task can be woken until the start of the
			 * next field.
			 */
			LOG((this,LOG_DISPL,1, " VBLANKPULSE (wake DHT)"));
			m_dsp.dht_blocks = false;
			m_dsp.dwt_blocks = false;
			m_task_wakeup |= 1 << task_dht;
			/*
			 * VBLANKPULSE also resets the cursor task block flip flop,
			 * which is built from two NAND gates a40c and a40d (74H01).
			 */
			m_dsp.curt_blocks = false;
		}
		if (!A63_HBLANK(a63) && A63_HBLANK(m_dsp.a63))
		{
			// Falling edge of a63 HBLANK starts unloading of FIFO words
			LOG((this,LOG_DISPL,1, " HBLANK\\ UNLOAD"));
			m_unload_time = ALTO2_DISPLAY_BITTIME(m_dsp.halfclock ? 32 : 16);
			m_unload_word = 0;
		}
	}

	/*
	 * The wakeup request for the display word task (DWT) is controlled by
	 * the state of the 16 word FIFO. If DWT has not executed a BLOCK,
	 * if DHT is not blocked, and if the buffer is not full, DWT wakeups
	 * are generated.
	 */
	UINT8 a38 = m_disp_a38[m_dsp.ra * 16 + m_dsp.wa];
	if (!m_dsp.dwt_blocks && !m_dsp.dht_blocks && !FIFO_STOPWAKE(a38))
	{
		m_task_wakeup |= 1 << task_dwt;
		LOG((this,LOG_DISPL,1, " (wake DWT)"));
	}

	// Stop waking up the DWT when SCANEND is active
	if (A63_SCANEND(a63))
	{
		m_task_wakeup &= ~(1 << task_dwt);
		LOG((this,LOG_DISPL,1, " SCANEND"));
	}

	LOG((this,LOG_DISPL,1, "%s", A63_HBLANK(a63) ? " HBLANK": ""));

	if (A63_HSYNC(a63))
	{
		// Active HSYNC
		if (!A63_HSYNC(m_dsp.a63))
		{
			// Rising edge of HSYNC => CLRBUF
			LOG((this,LOG_DISPL,1, " HSYNC/ (CLRBUF)"));
			/*
			 * The hardware sets the buffer empty and clears the DWT block
			 * flip-flop at the beginning of horizontal retrace for
			 * every scanline.
			 */
			m_dsp.wa = 0;
			m_dsp.ra = 0;
			m_dsp.dwt_blocks = false;
			// now take the new values from the last SETMODE<-
			m_dsp.inverse = GET_SETMODE_INVERSE(m_dsp.setmode) ? 0xffff : 0x0000;
			m_dsp.halfclock = GET_SETMODE_SPEEDY(m_dsp.setmode) ? true : false;
			// stop the CPU execution loop from calling unload_word()
			m_unload_time = -1;
		}
		else
		{
			LOG((this,LOG_DISPL,1, " HSYNC"));
		}
	}
	else
	// Falling edge of HSYNC?
	if (A63_HSYNC(m_dsp.a63))
	{
		/*
		 * CLRBUF' also resets the 2nd cursor task block flip flop,
		 * which is built from two NAND gates a30c and a30d (74H00).
		 * If both flip flops are reset, the NOR gate a20d (74S02)
		 * decodes this as WAKECURT signal.
		 */
		m_dsp.curt_wakeup = true;
		if (!m_dsp.curt_blocks)
			m_task_wakeup |= 1 << task_curt;
	}

	LOG((this,LOG_DISPL,1, " NEXT:%03o\n", next));

	m_dsp.a63 = a63;
	m_dsp.a66 = a66;
	m_dsp.state = next;
	m_dsp_time += ALTO2_DISPLAY_BITTIME(32);
}

/**
 * @brief branch on evenfield
 *
 * NEXT(09) = even field ? 1 : 0
 */
void alto2_cpu_device::f2_late_evenfield()
{
	UINT16 r = HLC1024 ^ 1;
	LOG((this,LOG_DISPL,2,"  EVENFIELD branch (%#o | %#o)\n", m_next2, r));
	m_next2 |= r;
}

/**
 * @brief initialize the display context to useful values
 *
 * Zap the display context.
 * Allocate a framebuf array to save updating the bitmap when
 * there is no change in the data word.
 */
void alto2_cpu_device::init_disp()
{
	memset(&m_dsp, 0, sizeof(m_dsp));
	save_item(NAME(m_dsp.state));
	save_item(NAME(m_dsp.hlc));
	save_item(NAME(m_dsp.setmode));
	save_item(NAME(m_dsp.inverse));
	save_item(NAME(m_dsp.halfclock));
	save_item(NAME(m_dsp.fifo));
	save_item(NAME(m_dsp.wa));
	save_item(NAME(m_dsp.ra));
	save_item(NAME(m_dsp.a63));
	save_item(NAME(m_dsp.a66));
	save_item(NAME(m_dsp.dht_blocks));
	save_item(NAME(m_dsp.dwt_blocks));
	save_item(NAME(m_dsp.curt_blocks));
	save_item(NAME(m_dsp.curt_wakeup));
	save_item(NAME(m_dsp.vblank));
	save_item(NAME(m_dsp.xpreg));
	save_item(NAME(m_dsp.csr));
	save_item(NAME(m_dsp.curxpos));
	save_item(NAME(m_dsp.cursor0));
	save_item(NAME(m_dsp.cursor1));

	m_disp_a38 = prom_load(machine(), &pl_displ_a38, memregion("displ_a38")->base());
	m_disp_a63 = prom_load(machine(), &pl_displ_a63, memregion("displ_a63")->base());
	m_disp_a66 = prom_load(machine(), &pl_displ_a66, memregion("displ_a66")->base());

	m_dsp.hlc = ALTO2_DISPLAY_HLC_START;

	m_dsp.framebuf = std::make_unique<UINT16[]>(ALTO2_DISPLAY_HEIGHT * ALTO2_DISPLAY_SCANLINE_WORDS);
	m_dsp.patterns = auto_alloc_array(machine(), UINT8, 65536 * 16);
	for (int y = 0; y < 65536; y++) {
		UINT8* dst = m_dsp.patterns + y * 16;
		for (int x = 0; x < 16; x++)
			*dst++ = (~y >> (15 - x)) & 1;
	}

	m_dsp.bitmap = std::make_unique<bitmap_ind16>(ALTO2_DISPLAY_WIDTH, ALTO2_DISPLAY_HEIGHT);
	m_dsp.state = 0;
}

void alto2_cpu_device::exit_disp()
{
	// nothing to do yet
}

void alto2_cpu_device::reset_disp()
{
	m_dsp.state = 0;
	m_dsp.hlc = ALTO2_DISPLAY_HLC_START;
	m_dsp.a63 = 0;
	m_dsp.a66 = 0;
	m_dsp.setmode = 0;
	m_dsp.inverse = 0;
	m_dsp.halfclock = false;
	m_dsp.wa = 0;
	m_dsp.ra = 0;
	m_dsp.dht_blocks = false;
	m_dsp.dwt_blocks = false;
	m_dsp.curt_blocks = false;
	m_dsp.curt_wakeup = false;
	m_dsp.vblank = 0;
	m_dsp.xpreg = 0;
	m_dsp.csr = 0;
	m_dsp.curxpos = 0;
	m_dsp.cursor0 = 0;
	m_dsp.cursor1 = 0;
	memset(m_dsp.framebuf.get(), 1, sizeof(UINT16) * ALTO2_DISPLAY_HEIGHT * ALTO2_DISPLAY_SCANLINE_WORDS);
}

/* Video update */
UINT32 alto2_cpu_device::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	copybitmap(bitmap, *m_dsp.bitmap, 0, 0, 0, 0, cliprect);
	return 0;
}

void alto2_cpu_device::screen_eof(screen_device &screen, bool state)
{
	// FIXME: do we need this in some way?
}
