// license:BSD-3-Clause
// copyright-holders: Aaron Giles

/***************************************************************************

    Atari Crystal Castles hardware

    Games supported:
        * Crystal Castles (1983) [8 sets]

    Known issues:
        * none at this time

****************************************************************************

    Horizontal sync chain:

        A J/K flip flop @ 8L counts the 1H line, and cascades into a
        4-bit binary counter @ 7M, which counts the 2H,4H,8H,16H lines.
        This counter cascades into a 4-bit BCD decade counter @ 7N
        which counts the 32H,64H,128H,HBLANK lines. The counter system
        rolls over after counting to 320.

        Pixel clock = 5MHz
        HBLANK ends at H = 0
        HBLANK begins at H = 256
        HSYNC begins at H = 304
        HSYNC ends at H = 320
        HTOTAL = 320

    Vertical sync chain:

        The HBLANK signal clocks a 4-bit binary counter @ 7P, which counts
        the 1V,2V,4V,8V lines. This counter cascades into a second 4-bit
        binary counter @ 7R which counts the 16V,32V,64V,128V lines. The
        counter system rolls over after counting to 256.

        VBLANK and VSYNC signals are controlled by a PROM at 8J. The
        standard PROM maps as follows:

        VBLANK ends at V = 24
        VBLANK begins at V = 0
        VSYNC begins at V = 4
        VSYNC ends at V = 7
        VTOTAL = 256

    Interrupts:

        /IRQ clocked by IRQCK signal from the PROM at 8J. The standard
        PROM has a rising edge at V = 0,64,128,192.

****************************************************************************

    Crystal Castles memory map.

     Address  A A A A A A A A A A A A A A A A  R  D D D D D D D D  Function
              1 1 1 1 1 1 9 8 7 6 5 4 3 2 1 0  /  7 6 5 4 3 2 1 0
              5 4 3 2 1 0                      W
    -------------------------------------------------------------------------------
    0000      X X X X X X X X X X X X X X X X  W  X X X X X X X X  X Coordinate
    0001      0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1  W  D D D D D D D D  Y Coordinate
    0002      0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 0 R/W D D D D          Bit Mode
    0003-0BFF 0 0 0 0 A A A A A A A A A A A A R/W D D D D D D D D  RAM (DRAM)
    0C00-7FFF 0 A A A A A A A A A A A A A A A R/W D D D D D D D D  Screen RAM
    8000-8DFF 1 0 0 0 A A A A A A A A A A A A R/W D D D D D D D D  RAM (STATIC)
    8E00-8EFF 1 0 0 0 1 1 1 0 A A A A A A A A R/W D D D D D D D D  MOB BUF 2
    -------------------------------------------------------------------------------
    8F00-8FFF 1 0 0 0 1 1 1 1 A A A A A A A A R/W D D D D D D D D  MOB BUF 1
                                          0 0 R/W D D D D D D D D  MOB Picture
                                          0 1 R/W D D D D D D D D  MOB Vertical
                                          1 0 R/W D D D D D D D D  MOB Priority
                                          1 1 R/W D D D D D D D D  MOB Horizontal
    -------------------------------------------------------------------------------
    9000-90FF 1 0 0 1 0 0 X X A A A A A A A A R/W D D D D D D D D  NOVRAM
    9400-9401 1 0 0 1 0 1 0 X X X X X X X 0 A  R                   TRAK-BALL 1
    9402-9403 1 0 0 1 0 1 0 X X X X X X X 1 A  R                   TRAK-BALL 2
    9500-9501 1 0 0 1 0 1 0 X X X X X X X X A  R                   TRAK-BALL 1 mirror
    9600      1 0 0 1 0 1 1 X X X X X X X X X  R                   IN0
                                               R                D  COIN R
                                               R              D    COIN L
                                               R            D      COIN AUX
                                               R          D        SLAM
                                               R        D          SELF TEST
                                               R      D            VBLANK
                                               R    D              JMP1
                                               R  D                JMP2
    -------------------------------------------------------------------------------
    9800-980F 1 0 0 1 1 0 0 X X X X X A A A A R/W D D D D D D D D  CI/O 0
    9A00-9A0F 1 0 0 1 1 0 1 X X X X X A A A A R/W D D D D D D D D  CI/O 1
    9A08                                                    D D D  Option SW
                                                          D        SPARE
                                                        D          SPARE
                                                      D            SPARE
    9C00      1 0 0 1 1 1 0 0 0 X X X X X X X  W                   RECALL
    -------------------------------------------------------------------------------
    9C80      1 0 0 1 1 1 0 0 1 X X X X X X X  W  D D D D D D D D  H Scr Ctr Load
    9D00      1 0 0 1 1 1 0 1 0 X X X X X X X  W  D D D D D D D D  V Scr Ctr Load
    9D80      1 0 0 1 1 1 0 1 1 X X X X X X X  W                   Int. Acknowledge
    9E00      1 0 0 1 1 1 1 0 0 X X X X X X X  W                   WDOG
              1 0 0 1 1 1 1 0 1 X X X X A A A  W                D  OUT0
    9E80                                0 0 0  W                D  Trak Ball Light P1
    9E81                                0 0 1  W                D  Trak Ball Light P2
    9E82                                0 1 0  W                D  Store Low
    9E83                                0 1 1  W                D  Store High
    9E84                                1 0 0  W                D  Spare
    9E85                                1 0 1  W                D  Coin Counter R
    9E86                                1 1 0  W                D  Coin Counter L
    9E87                                1 1 1  W                D  BANK0-BANK1
              1 0 0 1 1 1 1 1 0 X X X X A A A  W          D        OUT1
    9F00                                0 0 0  W          D        ^AX
    9F01                                0 0 1  W          D        ^AY
    9F02                                0 1 0  W          D        ^XINC
    9F03                                0 1 1  W          D        ^YINC
    9F04                                1 0 0  W          D        PLAYER2 (flip screen)
    9F05                                1 0 1  W          D        ^SIRE
    9F06                                1 1 0  W          D        BOTHRAM
    9F07                                1 1 1  W          D        BUF1/^BUF2 (sprite bank)
    9F80-9FBF 1 0 0 1 1 1 1 1 1 X A A A A A A  W  D D D D D D D D  COLORAM
    A000-FFFF 1 A A A A A A A A A A A A A A A  R  D D D D D D D D  Program ROM

***************************************************************************/

#include "emu.h"

#include "cpu/m6502/m6502.h"
#include "machine/74259.h"
#include "machine/rescap.h"
#include "machine/watchdog.h"
#include "machine/x2212.h"
#include "sound/pokey.h"
#include "video/resnet.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"


namespace {

class ccastles_state : public driver_device
{
public:
	ccastles_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_nvram_4b(*this, "nvram_4b"),
		m_nvram_4a(*this, "nvram_4a"),
		m_outlatch(*this, "outlatch%u", 0U),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette"),
		m_videoram(*this, "videoram"),
		m_spriteram(*this, "spriteram"),
		m_syncprom(*this, "syncprom"),
		m_wpprom(*this, "wpprom"),
		m_priprom(*this, "priprom"),
		m_leta(*this, "LETA%u", 0U)
	{ }

	int vblank_r();
	void ccastles(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

private:
	// devices
	required_device<m6502_device> m_maincpu;
	required_device<x2212_device> m_nvram_4b;
	required_device<x2212_device> m_nvram_4a;
	required_device_array<ls259_device, 2> m_outlatch;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;

	// memory pointers
	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr<uint8_t> m_spriteram;
	required_region_ptr<uint8_t> m_syncprom;
	required_region_ptr<uint8_t> m_wpprom;
	required_region_ptr<uint8_t> m_priprom;

	required_ioport_array<4> m_leta;

	// video-related
	bitmap_ind16 m_spritebitmap = 0;
	double m_rweights[3]{};
	double m_gweights[3]{};
	double m_bweights[3]{};
	uint8_t m_bitmode_addr[2]{};
	uint8_t m_hscroll = 0U;
	uint8_t m_vscroll = 0U;

	// misc
	int m_vblank_start = 0;
	int m_vblank_end = 0;
	emu_timer *m_irq_timer = nullptr;
	uint8_t m_irq_state = 0U;

	static constexpr int MASTER_CLOCK = 10'000'000;
	static constexpr int PIXEL_CLOCK = MASTER_CLOCK / 2;
	static constexpr int HTOTAL = 320;
	static constexpr int VTOTAL = 256;

	void irq_ack_w(uint8_t data);
	uint8_t leta_r(offs_t offset);
	void nvram_recall_w(uint8_t data);
	void nvram_store_w(int state);
	uint8_t nvram_r(address_space &space, offs_t offset);
	void nvram_w(offs_t offset, uint8_t data);
	void hscroll_w(uint8_t data);
	void vscroll_w(uint8_t data);
	void video_control_w(offs_t offset, uint8_t data);
	void paletteram_w(offs_t offset, uint8_t data);
	void videoram_w(offs_t offset, uint8_t data);
	uint8_t bitmode_r();
	void bitmode_w(uint8_t data);
	void bitmode_addr_w(offs_t offset, uint8_t data);
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	TIMER_CALLBACK_MEMBER(clock_irq);
	inline void write_vram(uint16_t addr, uint8_t data, uint8_t bitmd, uint8_t pixba);
	inline void bitmode_autoinc();
	inline void schedule_next_irq(int curscanline);
	void main_map(address_map &map) ATTR_COLD;
};


/*************************************
 *
 *  Video startup
 *
 *************************************/

void ccastles_state::video_start()
{
	static const int resistances[3] = { 22000, 10000, 4700 };

	// compute the color output resistor weights at startup
	compute_resistor_weights(0, 255, -1.0,
			3,  resistances, m_rweights, 1000, 0,
			3,  resistances, m_gweights, 1000, 0,
			3,  resistances, m_bweights, 1000, 0);

	// allocate a bitmap for drawing sprites
	m_screen->register_screen_bitmap(m_spritebitmap);

	// register for savestates
	save_item(NAME(m_bitmode_addr));
	save_item(NAME(m_hscroll));
	save_item(NAME(m_vscroll));
}



/*************************************
 *
 *  Video control registers
 *
 *************************************/

void ccastles_state::hscroll_w(uint8_t data)
{
	m_screen->update_partial(m_screen->vpos());
	m_hscroll = data;
}


void ccastles_state::vscroll_w(uint8_t data)
{
	m_vscroll = data;
}


void ccastles_state::video_control_w(offs_t offset, uint8_t data)
{
	// only D3 matters
	m_outlatch[1]->write_bit(offset, BIT(data, 3));
}



/*************************************
 *
 *  Palette RAM accesses
 *
 *************************************/

void ccastles_state::paletteram_w(offs_t offset, uint8_t data)
{
	int bit0, bit1, bit2;
	int r, g, b;

	// extract the raw RGB bits
	r = ((data & 0xc0) >> 6) | ((offset & 0x20) >> 3);
	b = (data & 0x38) >> 3;
	g = (data & 0x07);

	// red component (inverted)
	bit0 = (~r >> 0) & 0x01;
	bit1 = (~r >> 1) & 0x01;
	bit2 = (~r >> 2) & 0x01;
	r = combine_weights(m_rweights, bit0, bit1, bit2);

	// green component (inverted)
	bit0 = (~g >> 0) & 0x01;
	bit1 = (~g >> 1) & 0x01;
	bit2 = (~g >> 2) & 0x01;
	g = combine_weights(m_gweights, bit0, bit1, bit2);

	// blue component (inverted)
	bit0 = (~b >> 0) & 0x01;
	bit1 = (~b >> 1) & 0x01;
	bit2 = (~b >> 2) & 0x01;
	b = combine_weights(m_bweights, bit0, bit1, bit2);

	m_palette->set_pen_color(offset & 0x1f, rgb_t(r, g, b));
}



/*************************************
 *
 *  Video RAM access via the write
 *  protect PROM
 *
 *************************************/

inline void ccastles_state::write_vram(uint16_t addr, uint8_t data, uint8_t bitmd, uint8_t pixba)
{
	uint8_t *dest = &m_videoram[addr & 0x7ffe];
	uint8_t promaddr = 0;

	/*
	    Inputs to the write-protect PROM:

	    Bit 7 = BA1520 = 0 if (BA15-BA12 != 0), or 1 otherwise
	    Bit 6 = DRBA11
	    Bit 5 = DRBA10
	    Bit 4 = /BITMD
	    Bit 3 = GND
	    Bit 2 = BA0
	    Bit 1 = PIXB
	    Bit 0 = PIXA
	*/
	promaddr |= ((addr & 0xf000) == 0) << 7;
	promaddr |= (addr & 0x0c00) >> 5;
	promaddr |= (!bitmd) << 4;
	promaddr |= (addr & 0x0001) << 2;
	promaddr |= (pixba << 0);

	// look up the PROM result
	uint8_t wpbits = m_wpprom[promaddr];

	// write to the appropriate parts of VRAM depending on the result
	if (!(wpbits & 1))
		dest[0] = (dest[0] & 0xf0) | (data & 0x0f);
	if (!(wpbits & 2))
		dest[0] = (dest[0] & 0x0f) | (data & 0xf0);
	if (!(wpbits & 4))
		dest[1] = (dest[1] & 0xf0) | (data & 0x0f);
	if (!(wpbits & 8))
		dest[1] = (dest[1] & 0x0f) | (data & 0xf0);
}



/*************************************
 *
 *  Autoincrement control for bit mode
 *
 *************************************/

inline void ccastles_state::bitmode_autoinc()
{
	// auto increment in the x-direction if it's enabled
	if (!m_outlatch[1]->q0_r()) // /AX
	{
		if (!m_outlatch[1]->q2_r()) // /XINC
			m_bitmode_addr[0]++;
		else
			m_bitmode_addr[0]--;
	}

	// auto increment in the y-direction if it's enabled
	if (!m_outlatch[1]->q1_r()) // /AY
	{
		if (!m_outlatch[1]->q3_r()) // /YINC
			m_bitmode_addr[1]++;
		else
			m_bitmode_addr[1]--;
	}
}



/*************************************
 *
 *  Standard video RAM access
 *
 *************************************/

void ccastles_state::videoram_w(offs_t offset, uint8_t data)
{
	// direct writes to VRAM go through the write protect PROM as well
	write_vram(offset, data, 0, 0);
}



/*************************************
 *
 *  Bit mode video RAM access
 *
 *************************************/

uint8_t ccastles_state::bitmode_r()
{
	// in bitmode, the address comes from the autoincrement latches
	uint16_t const addr = (m_bitmode_addr[1] << 7) | (m_bitmode_addr[0] >> 1);

	// the appropriate pixel is selected into the upper 4 bits
	uint8_t const result = m_videoram[addr] << ((~m_bitmode_addr[0] & 1) * 4);

	// autoincrement because /BITMD was selected
	bitmode_autoinc();

	// the low 4 bits of the data lines are not driven so make them all 1's
	return result | 0x0f;
}


void ccastles_state::bitmode_w(uint8_t data)
{
	// in bitmode, the address comes from the autoincrement latches
	uint16_t const addr = (m_bitmode_addr[1] << 7) | (m_bitmode_addr[0] >> 1);

	// the upper 4 bits of data are replicated to the lower 4 bits
	data = (data & 0xf0) | (data >> 4);

	// write through the generic VRAM routine, passing the low 2 X bits as PIXB/PIXA
	write_vram(addr, data, 1, m_bitmode_addr[0] & 3);

	// autoincrement because /BITMD was selected
	bitmode_autoinc();
}


void ccastles_state::bitmode_addr_w(offs_t offset, uint8_t data)
{
	// write through to video RAM and also to the addressing latches
	write_vram(offset, data, 0, 0);
	m_bitmode_addr[offset] = data;
}



/*************************************
 *
 *  Video updating
 *
 *************************************/

uint32_t ccastles_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	uint8_t const *const spriteaddr = &m_spriteram[m_outlatch[1]->q7_r() * 0x100];   // BUF1/BUF2
	int const flip = m_outlatch[1]->q4_r() ? 0xff : 0x00;    // PLAYER2
	pen_t const black = m_palette->black_pen();

	// draw the sprites
	m_spritebitmap.fill(0x0f, cliprect);
	for (int offs = 0; offs < 320 / 2; offs += 4)
	{
		int const x = spriteaddr[offs + 3];
		int const y = 256 - 16 - spriteaddr[offs + 1];
		int const which = spriteaddr[offs];
		int const color = spriteaddr[offs + 2] >> 7;

		m_gfxdecode->gfx(0)->transpen(m_spritebitmap, cliprect, which, color, flip, flip, x, y, 7);
	}

	// draw the bitmap to the screen, looping over Y
	for (int y = cliprect.top(); y <= cliprect.bottom(); y++)
	{
		uint16_t *const dst = &bitmap.pix(y);

		// if we're in the VBLANK region, just fill with black
		if (m_syncprom[y] & 1)
		{
			for (int x = cliprect.left(); x <= cliprect.right(); x++)
				dst[x] = black;
		}

		// non-VBLANK region: merge the sprites and the bitmap
		else
		{
			uint16_t const *const mosrc = &m_spritebitmap.pix(y);

			// the "POTATO" chip does some magic here; this is just a guess
			int effy = (((y - m_vblank_end) + (flip ? 0 : m_vscroll)) ^ flip) & 0xff;
			if (effy < 24)
				effy = 24;
			uint8_t const *const src = &m_videoram[effy * 128];

			// loop over X
			for (int x = cliprect.left(); x <= cliprect.right(); x++)
			{
				// if we're in the HBLANK region, just store black
				if (x >= 256)
					dst[x] = black;

				// otherwise, process normally
				else
				{
					int const effx = (m_hscroll + (x ^ flip)) & 255;

					// low 4 bits = left pixel, high 4 bits = right pixel
					uint8_t pix = (src[effx / 2] >> ((effx & 1) * 4)) & 0x0f;
					uint8_t const mopix = mosrc[x];

					/* Inputs to the priority PROM:

					    Bit 7 = GND
					    Bit 6 = /CRAM
					    Bit 5 = BA4
					    Bit 4 = MV2
					    Bit 3 = MV1
					    Bit 2 = MV0
					    Bit 1 = MPI
					    Bit 0 = BIT3
					*/
					uint8_t prindex = 0x40;
					prindex |= (mopix & 7) << 2;
					prindex |= (mopix & 8) >> 2;
					prindex |= (pix & 8) >> 3;
					uint8_t const prvalue = m_priprom[prindex];

					// Bit 1 of prvalue selects the low 4 bits of the final pixel
					if (prvalue & 2)
						pix = mopix;

					// Bit 0 of prvalue selects bit 4 of the final color
					pix |= (prvalue & 1) << 4;

					// store the pixel value and also a priority value based on the topmost bit
					dst[x] = pix;
				}
			}
		}
	}
	return 0;
}


/************************************* *
 *  VBLANK and IRQ generation
 *
 *************************************/

inline void ccastles_state::schedule_next_irq(int curscanline)
{
	// scan for a rising edge on the IRQCK signal
	for (curscanline++; ; curscanline = (curscanline + 1) & 0xff)
		if ((m_syncprom[(curscanline - 1) & 0xff] & 8) == 0 && (m_syncprom[curscanline] & 8) != 0)
			break;

	// next one at the start of this scanline
	m_irq_timer->adjust(m_screen->time_until_pos(curscanline), curscanline);
}


TIMER_CALLBACK_MEMBER(ccastles_state::clock_irq)
{
	// assert the IRQ if not already asserted
	if (!m_irq_state)
	{
		m_maincpu->set_input_line(0, ASSERT_LINE);
		m_irq_state = 1;
	}

	// force an update now
	m_screen->update_partial(m_screen->vpos());

	// find the next edge
	schedule_next_irq(param);
}


int ccastles_state::vblank_r()
{
	int const scanline = m_screen->vpos();
	return m_syncprom[scanline & 0xff] & 1;
}



/*************************************
 *
 *  Machine setup
 *
 *************************************/

void ccastles_state::machine_start()
{
	rectangle visarea;

	// find the start of VBLANK in the SYNC PROM
	for (m_vblank_start = 0; m_vblank_start < 256; m_vblank_start++)
		if ((m_syncprom[(m_vblank_start - 1) & 0xff] & 1) == 0 && (m_syncprom[m_vblank_start] & 1) != 0)
			break;
	if (m_vblank_start == 0)
		m_vblank_start = 256;

	// find the end of VBLANK in the SYNC PROM
	for (m_vblank_end = 0; m_vblank_end < 256; m_vblank_end++)
		if ((m_syncprom[(m_vblank_end - 1) & 0xff] & 1) != 0 && (m_syncprom[m_vblank_end] & 1) == 0)
			break;

	// can't handle the wrapping case
	assert(m_vblank_end < m_vblank_start);

	// reconfigure the visible area to match
	visarea.set(0, 255, m_vblank_end, m_vblank_start - 1);
	m_screen->configure(320, 256, visarea, HZ_TO_ATTOSECONDS(PIXEL_CLOCK) * VTOTAL * HTOTAL);

	// configure the ROM banking
	membank("rombank")->configure_entries(0, 2, memregion("maincpu")->base() + 0xa000, 0x6000);

	// create a timer for IRQs and set up the first callback
	m_irq_timer = timer_alloc(FUNC(ccastles_state::clock_irq), this);
	m_irq_state = 0;
	schedule_next_irq(0);

	// setup for save states
	save_item(NAME(m_irq_state));
}


void ccastles_state::machine_reset()
{
	m_maincpu->set_input_line(0, CLEAR_LINE);
	m_irq_state = 0;
}



/*************************************
 *
 *  Output ports
 *
 *************************************/

void ccastles_state::irq_ack_w(uint8_t data)
{
	if (m_irq_state)
	{
		m_maincpu->set_input_line(0, CLEAR_LINE);
		m_irq_state = 0;
	}
}


uint8_t ccastles_state::leta_r(offs_t offset)
{
	return m_leta[offset]->read();
}



/*************************************
 *
 *  NVRAM handling
 *
 *************************************/

void ccastles_state::nvram_recall_w(uint8_t data)
{
	m_nvram_4b->recall(0);
	m_nvram_4b->recall(1);
	m_nvram_4b->recall(0);
	m_nvram_4a->recall(0);
	m_nvram_4a->recall(1);
	m_nvram_4a->recall(0);
}


void ccastles_state::nvram_store_w(int state)
{
	m_nvram_4b->store(!m_outlatch[0]->q2_r() && m_outlatch[0]->q3_r());
	m_nvram_4a->store(!m_outlatch[0]->q2_r() && m_outlatch[0]->q3_r());
}


uint8_t ccastles_state::nvram_r(address_space &space, offs_t offset)
{
	return (m_nvram_4b->read(space, offset) & 0x0f) | (m_nvram_4a->read(space, offset) << 4);
}


void ccastles_state::nvram_w(offs_t offset, uint8_t data)
{
	m_nvram_4b->write(offset, data);
	m_nvram_4a->write(offset, data >> 4);
}



/*************************************
 *
 *  Main CPU memory handlers
 *
 *************************************/

// complete memory map derived from schematics
void ccastles_state::main_map(address_map &map)
{
	map(0x0000, 0x7fff).ram().w(FUNC(ccastles_state::videoram_w)).share(m_videoram);
	map(0x0000, 0x0001).w(FUNC(ccastles_state::bitmode_addr_w));
	map(0x0002, 0x0002).rw(FUNC(ccastles_state::bitmode_r), FUNC(ccastles_state::bitmode_w));
	map(0x8000, 0x8dff).ram();
	map(0x8e00, 0x8fff).ram().share(m_spriteram);
	map(0x9000, 0x90ff).mirror(0x0300).rw(FUNC(ccastles_state::nvram_r), FUNC(ccastles_state::nvram_w));
	map(0x9400, 0x9403).mirror(0x01fc).r(FUNC(ccastles_state::leta_r));
	map(0x9600, 0x97ff).portr("IN0");
	map(0x9800, 0x980f).mirror(0x01f0).rw("pokey1", FUNC(pokey_device::read), FUNC(pokey_device::write));
	map(0x9a00, 0x9a0f).mirror(0x01f0).rw("pokey2", FUNC(pokey_device::read), FUNC(pokey_device::write));
	map(0x9c00, 0x9c7f).w(FUNC(ccastles_state::nvram_recall_w));
	map(0x9c80, 0x9cff).w(FUNC(ccastles_state::hscroll_w));
	map(0x9d00, 0x9d7f).w(FUNC(ccastles_state::vscroll_w));
	map(0x9d80, 0x9dff).w(FUNC(ccastles_state::irq_ack_w));
	map(0x9e00, 0x9e7f).w("watchdog", FUNC(watchdog_timer_device::reset_w));
	map(0x9e80, 0x9e87).mirror(0x0078).w(m_outlatch[0], FUNC(ls259_device::write_d0));
	map(0x9f00, 0x9f07).mirror(0x0078).w(FUNC(ccastles_state::video_control_w));
	map(0x9f80, 0x9fbf).mirror(0x0040).w(FUNC(ccastles_state::paletteram_w));
	map(0xa000, 0xdfff).bankr("rombank");
	map(0xe000, 0xffff).rom();
}



/*************************************
 *
 *  Port definitions
 *
 *************************************/

static INPUT_PORTS_START( ccastles )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_TILT )
	PORT_SERVICE( 0x10, IP_ACTIVE_LOW )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(ccastles_state, vblank_r)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("Left Jump/1P Start Upright")    PORT_CONDITION("IN1", 0x20, EQUALS, 0x00)  // left Jump, non-cocktail start1
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("1P Jump")           PORT_CONDITION("IN1", 0x20, EQUALS, 0x20)  // 1p Jump, cocktail
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("Right Jump/2P Start Upright")   PORT_CONDITION("IN1", 0x20, EQUALS, 0x00)  // right Jump, non-cocktail start2
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL PORT_NAME("2P Jump") PORT_CONDITION("IN1", 0x20, EQUALS, 0x20)  // 2p Jump, cocktail

	PORT_START("IN1")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START1 ) PORT_NAME("1P Start Cocktail")  // cocktail only
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START2 ) PORT_NAME("2P Start Cocktail")  // cocktail only
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Cocktail ) )
	PORT_BIT( 0xc0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("LETA0")
	PORT_BIT( 0xff, 0x00, IPT_TRACKBALL_Y ) PORT_SENSITIVITY(10) PORT_KEYDELTA(30) PORT_REVERSE

	PORT_START("LETA1")
	PORT_BIT( 0xff, 0x00, IPT_TRACKBALL_X ) PORT_SENSITIVITY(10) PORT_KEYDELTA(30)

	PORT_START("LETA2")
	PORT_BIT( 0xff, 0x00, IPT_TRACKBALL_Y ) PORT_COCKTAIL PORT_SENSITIVITY(10) PORT_KEYDELTA(30) PORT_REVERSE   // cocktail only

	PORT_START("LETA3")
	PORT_BIT( 0xff, 0x00, IPT_TRACKBALL_X ) PORT_COCKTAIL PORT_SENSITIVITY(10) PORT_KEYDELTA(30)            // cocktail only
INPUT_PORTS_END


static INPUT_PORTS_START( ccastlesj )
	PORT_INCLUDE(ccastles)

	PORT_MODIFY("LETA0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_MODIFY("LETA1")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_MODIFY("LETA2")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_MODIFY("LETA3")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END



/*************************************
 *
 *  Graphics definitions
 *
 *************************************/

/* technically, this is 4bpp graphics, but the top bit is thrown away during
    processing to make room for the priority bit in the sprite buffers */
static const gfx_layout ccastles_spritelayout =
{
	8,16,
	RGN_FRAC(1,2),
	3,
	{ 4, RGN_FRAC(1,2)+0, RGN_FRAC(1,2)+4 },
	{ 0, 1, 2, 3, 8+0, 8+1, 8+2, 8+3 },
	{ 0*16, 1*16, 2*16, 3*16, 4*16, 5*16, 6*16, 7*16,
			8*16, 9*16, 10*16, 11*16, 12*16, 13*16, 14*16, 15*16 },
	32*8
};


static GFXDECODE_START( gfx_ccastles )
	GFXDECODE_ENTRY( "gfx", 0x0000, ccastles_spritelayout,  0, 2 )
GFXDECODE_END


/*************************************
 *
 *  Machine driver
 *
 *************************************/

void ccastles_state::ccastles(machine_config &config)
{
	// basic machine hardware
	M6502(config, m_maincpu, MASTER_CLOCK / 8);
	m_maincpu->set_addrmap(AS_PROGRAM, &ccastles_state::main_map);

	LS259(config, m_outlatch[0]); // 8N
	m_outlatch[0]->q_out_cb<0>().set_output("led0").invert();
	m_outlatch[0]->q_out_cb<1>().set_output("led1").invert();
	m_outlatch[0]->q_out_cb<2>().set(FUNC(ccastles_state::nvram_store_w));
	m_outlatch[0]->q_out_cb<3>().set(FUNC(ccastles_state::nvram_store_w));
	m_outlatch[0]->q_out_cb<5>().set([this] (int state) { machine().bookkeeping().coin_counter_w(0, state); });
	m_outlatch[0]->q_out_cb<6>().set([this] (int state) { machine().bookkeeping().coin_counter_w(1, state); });
	m_outlatch[0]->q_out_cb<7>().set_membank("rombank");

	LS259(config, m_outlatch[1]); // 6P

	WATCHDOG_TIMER(config, "watchdog").set_vblank_count("screen", 8);

	X2212(config, "nvram_4b").set_auto_save(true);
	X2212(config, "nvram_4a").set_auto_save(true);

	// video hardware
	GFXDECODE(config, m_gfxdecode, m_palette, gfx_ccastles);
	PALETTE(config, m_palette).set_entries(32);

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_raw(PIXEL_CLOCK, HTOTAL, 0, 256, VTOTAL, 24, 256); // potentially adjusted later
	m_screen->set_screen_update(FUNC(ccastles_state::screen_update));
	m_screen->set_palette(m_palette);

	// sound hardware
	SPEAKER(config, "mono").front_center();

	pokey_device &pokey1(POKEY(config, "pokey1", MASTER_CLOCK / 8));
	// NOTE: 1k + 0.2k is not 100% exact, but should not make an audible difference
	pokey1.set_output_opamp(RES_K(1) + RES_K(0.2), CAP_U(0.01), 5.0);
	pokey1.add_route(ALL_OUTPUTS, "mono", 1.0);

	pokey_device &pokey2(POKEY(config, "pokey2", MASTER_CLOCK / 8));
	// NOTE: 1k + 0.2k is not 100% exact, but should not make an audible difference
	pokey2.set_output_opamp(RES_K(1) + RES_K(0.2), CAP_U(0.01), 5.0);
	pokey2.allpot_r().set_ioport("IN1");
	pokey2.add_route(ALL_OUTPUTS, "mono", 1.0);
}



/*************************************
 *
 *  ROM definitions
 *
 *************************************/

ROM_START( ccastles )
	ROM_REGION( 0x14000, "maincpu", 0 )
	ROM_LOAD( "136022-403.1k", 0x0a000, 0x2000, CRC(81471ae5) SHA1(8ec13b48119ecf8fe85207403c0a0de5240cded4) )
	ROM_LOAD( "136022-404.1l", 0x0c000, 0x2000, CRC(820daf29) SHA1(a2cff00e9ddce201344692b75038431e4241fedd) )
	ROM_LOAD( "136022-405.1n", 0x0e000, 0x2000, CRC(4befc296) SHA1(2e789a32903808014e9d5f3021d7eff57c3e2212) )
	ROM_LOAD( "136022-102.1h", 0x10000, 0x2000, CRC(f6ccfbd4) SHA1(69c3da2cbefc5e03a77357e817e3015da5d8334a) )
	ROM_LOAD( "136022-101.1f", 0x12000, 0x2000, CRC(e2e17236) SHA1(81fa95b4d9beacb06d6b4afdf346d94117396557) )

	ROM_REGION( 0x4000, "gfx", 0 )
	ROM_LOAD( "136022-106.8d", 0x0000, 0x2000, CRC(9d1d89fc) SHA1(01c279edee322cc28f34506c312e4a9e3363b1be) )
	ROM_LOAD( "136022-107.8b", 0x2000, 0x2000, CRC(39960b7d) SHA1(82bdf764ac23e72598883283c5e957169387abd4) )

	ROM_REGION( 0x0100, "syncprom", 0 )
	ROM_LOAD( "82s129-136022-108.7k",  0x0000, 0x0100, CRC(6ed31e3b) SHA1(c3f3e4e7f313ecfd101cc52dfc44bd6b51a2ac88) ) // vertical sync generation

	ROM_REGION( 0x0100, "addressprom", 0 )
	ROM_LOAD( "82s129-136022-109.6l",  0x0000, 0x0100, CRC(b3515f1a) SHA1(c1bf077242481ef2f958580602b8113532b58612) ) // address decoding

	ROM_REGION( 0x0100, "wpprom", 0 )
	ROM_LOAD( "82s129-136022-110.11l", 0x0000, 0x0100, CRC(068bdc7e) SHA1(ae155918fdafd14299bc448b43eed8ad9c1ef5ef) ) // DRAM write protection

	ROM_REGION( 0x0100, "priprom", 0 )
	ROM_LOAD( "82s129-136022-111.10k", 0x0000, 0x0100, CRC(c29c18d9) SHA1(278bf61a290ae72ddaae2bafb4ab6739d3fb6238) ) // color selection
ROM_END


ROM_START( ccastlesg )
	ROM_REGION( 0x14000, "maincpu", 0 )
	ROM_LOAD( "136022-303.1k", 0x0a000, 0x2000, CRC(10e39fce) SHA1(5247f52e14ccf39f0ec699a39c8ebe35e61e07d2) )
	ROM_LOAD( "136022-304.1l", 0x0c000, 0x2000, CRC(74510f72) SHA1(d22550f308ff395d51869b52449bc0669a4e35e4) )
	ROM_LOAD( "136022-112.1n", 0x0e000, 0x2000, CRC(69b8d906) SHA1(b71251a4402eedf97b6ed5798403823739991d3e) )
	ROM_LOAD( "136022-102.1h", 0x10000, 0x2000, CRC(f6ccfbd4) SHA1(69c3da2cbefc5e03a77357e817e3015da5d8334a) )
	ROM_LOAD( "136022-101.1f", 0x12000, 0x2000, CRC(e2e17236) SHA1(81fa95b4d9beacb06d6b4afdf346d94117396557) )

	ROM_REGION( 0x4000, "gfx", 0 )
	ROM_LOAD( "136022-106.8d", 0x0000, 0x2000, CRC(9d1d89fc) SHA1(01c279edee322cc28f34506c312e4a9e3363b1be) )
	ROM_LOAD( "136022-107.8b", 0x2000, 0x2000, CRC(39960b7d) SHA1(82bdf764ac23e72598883283c5e957169387abd4) )

	ROM_REGION( 0x0100, "syncprom", 0 )
	ROM_LOAD( "82s129-136022-108.7k",  0x0000, 0x0100, CRC(6ed31e3b) SHA1(c3f3e4e7f313ecfd101cc52dfc44bd6b51a2ac88) ) // vertical sync generation

	ROM_REGION( 0x0100, "addressprom", 0 )
	ROM_LOAD( "82s129-136022-109.6l",  0x0000, 0x0100, CRC(b3515f1a) SHA1(c1bf077242481ef2f958580602b8113532b58612) ) // address decoding

	ROM_REGION( 0x0100, "wpprom", 0 )
	ROM_LOAD( "82s129-136022-110.11l", 0x0000, 0x0100, CRC(068bdc7e) SHA1(ae155918fdafd14299bc448b43eed8ad9c1ef5ef) ) // DRAM write protection

	ROM_REGION( 0x0100, "priprom", 0 )
	ROM_LOAD( "82s129-136022-111.10k", 0x0000, 0x0100, CRC(c29c18d9) SHA1(278bf61a290ae72ddaae2bafb4ab6739d3fb6238) ) // color selection
ROM_END


ROM_START( ccastlesp )
	ROM_REGION( 0x14000, "maincpu", 0 )
	ROM_LOAD( "136022-303.1k", 0x0a000, 0x2000, CRC(10e39fce) SHA1(5247f52e14ccf39f0ec699a39c8ebe35e61e07d2) )
	ROM_LOAD( "136022-304.1l", 0x0c000, 0x2000, CRC(74510f72) SHA1(d22550f308ff395d51869b52449bc0669a4e35e4) )
	ROM_LOAD( "136022-113.1n", 0x0e000, 0x2000, CRC(b833936e) SHA1(c063989107acb82ac963342d6328c7e459160d2a) )
	ROM_LOAD( "136022-102.1h", 0x10000, 0x2000, CRC(f6ccfbd4) SHA1(69c3da2cbefc5e03a77357e817e3015da5d8334a) )
	ROM_LOAD( "136022-101.1f", 0x12000, 0x2000, CRC(e2e17236) SHA1(81fa95b4d9beacb06d6b4afdf346d94117396557) )

	ROM_REGION( 0x4000, "gfx", 0 )
	ROM_LOAD( "136022-106.8d", 0x0000, 0x2000, CRC(9d1d89fc) SHA1(01c279edee322cc28f34506c312e4a9e3363b1be) )
	ROM_LOAD( "136022-107.8b", 0x2000, 0x2000, CRC(39960b7d) SHA1(82bdf764ac23e72598883283c5e957169387abd4) )

	ROM_REGION( 0x0100, "syncprom", 0 )
	ROM_LOAD( "82s129-136022-108.7k",  0x0000, 0x0100, CRC(6ed31e3b) SHA1(c3f3e4e7f313ecfd101cc52dfc44bd6b51a2ac88) ) // vertical sync generation

	ROM_REGION( 0x0100, "addressprom", 0 )
	ROM_LOAD( "82s129-136022-109.6l",  0x0000, 0x0100, CRC(b3515f1a) SHA1(c1bf077242481ef2f958580602b8113532b58612) ) // address decoding

	ROM_REGION( 0x0100, "wpprom", 0 )
	ROM_LOAD( "82s129-136022-110.11l", 0x0000, 0x0100, CRC(068bdc7e) SHA1(ae155918fdafd14299bc448b43eed8ad9c1ef5ef) ) // DRAM write protection

	ROM_REGION( 0x0100, "priprom", 0 )
	ROM_LOAD( "82s129-136022-111.10k", 0x0000, 0x0100, CRC(c29c18d9) SHA1(278bf61a290ae72ddaae2bafb4ab6739d3fb6238) ) // color selection
ROM_END


ROM_START( ccastlesf )
	ROM_REGION( 0x14000, "maincpu", 0 )
	ROM_LOAD( "136022-303.1k", 0x0a000, 0x2000, CRC(10e39fce) SHA1(5247f52e14ccf39f0ec699a39c8ebe35e61e07d2) )
	ROM_LOAD( "136022-304.1l", 0x0c000, 0x2000, CRC(74510f72) SHA1(d22550f308ff395d51869b52449bc0669a4e35e4) )
	ROM_LOAD( "136022-114.1n", 0x0e000, 0x2000, CRC(8585b4d1) SHA1(e2054dba64cc210a0790fe32a98d8c35c1389bf5) )
	ROM_LOAD( "136022-102.1h", 0x10000, 0x2000, CRC(f6ccfbd4) SHA1(69c3da2cbefc5e03a77357e817e3015da5d8334a) )
	ROM_LOAD( "136022-101.1f", 0x12000, 0x2000, CRC(e2e17236) SHA1(81fa95b4d9beacb06d6b4afdf346d94117396557) )

	ROM_REGION( 0x4000, "gfx", 0 )
	ROM_LOAD( "136022-106.8d", 0x0000, 0x2000, CRC(9d1d89fc) SHA1(01c279edee322cc28f34506c312e4a9e3363b1be) )
	ROM_LOAD( "136022-107.8b", 0x2000, 0x2000, CRC(39960b7d) SHA1(82bdf764ac23e72598883283c5e957169387abd4) )

	ROM_REGION( 0x0100, "syncprom", 0 )
	ROM_LOAD( "82s129-136022-108.7k",  0x0000, 0x0100, CRC(6ed31e3b) SHA1(c3f3e4e7f313ecfd101cc52dfc44bd6b51a2ac88) ) // vertical sync generation

	ROM_REGION( 0x0100, "addressprom", 0 )
	ROM_LOAD( "82s129-136022-109.6l",  0x0000, 0x0100, CRC(b3515f1a) SHA1(c1bf077242481ef2f958580602b8113532b58612) ) // address decoding

	ROM_REGION( 0x0100, "wpprom", 0 )
	ROM_LOAD( "82s129-136022-110.11l", 0x0000, 0x0100, CRC(068bdc7e) SHA1(ae155918fdafd14299bc448b43eed8ad9c1ef5ef) ) // DRAM write protection

	ROM_REGION( 0x0100, "priprom", 0 )
	ROM_LOAD( "82s129-136022-111.10k", 0x0000, 0x0100, CRC(c29c18d9) SHA1(278bf61a290ae72ddaae2bafb4ab6739d3fb6238) ) // color selection
ROM_END


ROM_START( ccastles3 )
	ROM_REGION( 0x14000, "maincpu", 0 )
	ROM_LOAD( "136022-303.1k", 0x0a000, 0x2000, CRC(10e39fce) SHA1(5247f52e14ccf39f0ec699a39c8ebe35e61e07d2) )
	ROM_LOAD( "136022-304.1l", 0x0c000, 0x2000, CRC(74510f72) SHA1(d22550f308ff395d51869b52449bc0669a4e35e4) )
	ROM_LOAD( "136022-305.1n", 0x0e000, 0x2000, CRC(9418cf8a) SHA1(1f835db94270e4a16e721b2ac355fb7e7c052285) )
	ROM_LOAD( "136022-102.1h", 0x10000, 0x2000, CRC(f6ccfbd4) SHA1(69c3da2cbefc5e03a77357e817e3015da5d8334a) )
	ROM_LOAD( "136022-101.1f", 0x12000, 0x2000, CRC(e2e17236) SHA1(81fa95b4d9beacb06d6b4afdf346d94117396557) )

	ROM_REGION( 0x4000, "gfx", 0 )
	ROM_LOAD( "136022-106.8d", 0x0000, 0x2000, CRC(9d1d89fc) SHA1(01c279edee322cc28f34506c312e4a9e3363b1be) )
	ROM_LOAD( "136022-107.8b", 0x2000, 0x2000, CRC(39960b7d) SHA1(82bdf764ac23e72598883283c5e957169387abd4) )

	ROM_REGION( 0x0100, "syncprom", 0 )
	ROM_LOAD( "82s129-136022-108.7k",  0x0000, 0x0100, CRC(6ed31e3b) SHA1(c3f3e4e7f313ecfd101cc52dfc44bd6b51a2ac88) ) // vertical sync generation

	ROM_REGION( 0x0100, "addressprom", 0 )
	ROM_LOAD( "82s129-136022-109.6l",  0x0000, 0x0100, CRC(b3515f1a) SHA1(c1bf077242481ef2f958580602b8113532b58612) ) // address decoding

	ROM_REGION( 0x0100, "wpprom", 0 )
	ROM_LOAD( "82s129-136022-110.11l", 0x0000, 0x0100, CRC(068bdc7e) SHA1(ae155918fdafd14299bc448b43eed8ad9c1ef5ef) ) // DRAM write protection

	ROM_REGION( 0x0100, "priprom", 0 )
	ROM_LOAD( "82s129-136022-111.10k", 0x0000, 0x0100, CRC(c29c18d9) SHA1(278bf61a290ae72ddaae2bafb4ab6739d3fb6238) ) // color selection
ROM_END


ROM_START( ccastles2 )
	ROM_REGION( 0x14000, "maincpu", 0 )
	ROM_LOAD( "136022-203.1k", 0x0a000, 0x2000, CRC(348a96f0) SHA1(76de7bf6a01ccb15a4fe7333c1209f623a2e0d1b) )
	ROM_LOAD( "136022-204.1l", 0x0c000, 0x2000, CRC(d48d8c1f) SHA1(8744182a3e2096419de63e341feb77dd8a8bcb34) )
	ROM_LOAD( "136022-205.1n", 0x0e000, 0x2000, CRC(0e4883cc) SHA1(a96abbf654e087409a90c1686d9dd553bd08c14e) )
	ROM_LOAD( "136022-102.1h", 0x10000, 0x2000, CRC(f6ccfbd4) SHA1(69c3da2cbefc5e03a77357e817e3015da5d8334a) )
	ROM_LOAD( "136022-101.1f", 0x12000, 0x2000, CRC(e2e17236) SHA1(81fa95b4d9beacb06d6b4afdf346d94117396557) )

	ROM_REGION( 0x4000, "gfx", 0 )
	ROM_LOAD( "136022-106.8d", 0x0000, 0x2000, CRC(9d1d89fc) SHA1(01c279edee322cc28f34506c312e4a9e3363b1be) )
	ROM_LOAD( "136022-107.8b", 0x2000, 0x2000, CRC(39960b7d) SHA1(82bdf764ac23e72598883283c5e957169387abd4) )

	ROM_REGION( 0x0100, "syncprom", 0 )
	ROM_LOAD( "82s129-136022-108.7k",  0x0000, 0x0100, CRC(6ed31e3b) SHA1(c3f3e4e7f313ecfd101cc52dfc44bd6b51a2ac88) ) // vertical sync generation

	ROM_REGION( 0x0100, "addressprom", 0 )
	ROM_LOAD( "82s129-136022-109.6l",  0x0000, 0x0100, CRC(b3515f1a) SHA1(c1bf077242481ef2f958580602b8113532b58612) ) // address decoding

	ROM_REGION( 0x0100, "wpprom", 0 )
	ROM_LOAD( "82s129-136022-110.11l", 0x0000, 0x0100, CRC(068bdc7e) SHA1(ae155918fdafd14299bc448b43eed8ad9c1ef5ef) ) // DRAM write protection

	ROM_REGION( 0x0100, "priprom", 0 )
	ROM_LOAD( "82s129-136022-111.10k", 0x0000, 0x0100, CRC(c29c18d9) SHA1(278bf61a290ae72ddaae2bafb4ab6739d3fb6238) ) // color selection
ROM_END


ROM_START( ccastles1 )
	ROM_REGION( 0x14000, "maincpu", 0 )
	ROM_LOAD( "136022-103.1k", 0x0a000, 0x2000, CRC(9d10e314) SHA1(3474ae0f0617c1dc9aaa02ca2a912a72d57eba73) )
	ROM_LOAD( "136022-104.1l", 0x0c000, 0x2000, CRC(fe2647a4) SHA1(532b236043449b35bd444fff63a7e083d0e2d8c8) )
	ROM_LOAD( "136022-105.1n", 0x0e000, 0x2000, CRC(5a13af07) SHA1(d4314a4344aac4a794d9014943591fee2e9bf13b) )
	ROM_LOAD( "136022-102.1h", 0x10000, 0x2000, CRC(f6ccfbd4) SHA1(69c3da2cbefc5e03a77357e817e3015da5d8334a) )
	ROM_LOAD( "136022-101.1f", 0x12000, 0x2000, CRC(e2e17236) SHA1(81fa95b4d9beacb06d6b4afdf346d94117396557) )

	ROM_REGION( 0x4000, "gfx", 0 )
	ROM_LOAD( "136022-106.8d", 0x0000, 0x2000, CRC(9d1d89fc) SHA1(01c279edee322cc28f34506c312e4a9e3363b1be) )
	ROM_LOAD( "136022-107.8b", 0x2000, 0x2000, CRC(39960b7d) SHA1(82bdf764ac23e72598883283c5e957169387abd4) )

	ROM_REGION( 0x0100, "syncprom", 0 )
	ROM_LOAD( "82s129-136022-108.7k",  0x0000, 0x0100, CRC(6ed31e3b) SHA1(c3f3e4e7f313ecfd101cc52dfc44bd6b51a2ac88) ) // vertical sync generation

	ROM_REGION( 0x0100, "addressprom", 0 )
	ROM_LOAD( "82s129-136022-109.6l",  0x0000, 0x0100, CRC(b3515f1a) SHA1(c1bf077242481ef2f958580602b8113532b58612) ) // address decoding

	ROM_REGION( 0x0100, "wpprom", 0 )
	ROM_LOAD( "82s129-136022-110.11l", 0x0000, 0x0100, CRC(068bdc7e) SHA1(ae155918fdafd14299bc448b43eed8ad9c1ef5ef) ) // DRAM write protection

	ROM_REGION( 0x0100, "priprom", 0 )
	ROM_LOAD( "82s129-136022-111.10k", 0x0000, 0x0100, CRC(c29c18d9) SHA1(278bf61a290ae72ddaae2bafb4ab6739d3fb6238) ) // color selection
ROM_END


ROM_START( ccastlesj )
	ROM_REGION( 0x14000, "maincpu", 0 )
	ROM_LOAD( "a000.12m",      0x0a000, 0x2000, CRC(0d911ef4) SHA1(fbd6a5a0e4e865421ed3720aa61221d03583f248) )
	ROM_LOAD( "c000.13m",      0x0c000, 0x2000, CRC(246079de) SHA1(ade2c63656339c3e7e634470a17bc30da1006979) )
	ROM_LOAD( "e000.14m",      0x0e000, 0x2000, CRC(3beec4f3) SHA1(076caffe67910bdcd1f51a41f2cc4ebdb930c7ca) )
	ROM_LOAD( "136022-102.1h", 0x10000, 0x2000, CRC(f6ccfbd4) SHA1(69c3da2cbefc5e03a77357e817e3015da5d8334a) )
	ROM_LOAD( "136022-101.1f", 0x12000, 0x2000, CRC(e2e17236) SHA1(81fa95b4d9beacb06d6b4afdf346d94117396557) )

	ROM_REGION( 0x4000, "gfx", 0 )
	ROM_LOAD( "136022-106.8d", 0x0000, 0x2000, CRC(9d1d89fc) SHA1(01c279edee322cc28f34506c312e4a9e3363b1be) )
	ROM_LOAD( "136022-107.8b", 0x2000, 0x2000, CRC(39960b7d) SHA1(82bdf764ac23e72598883283c5e957169387abd4) )

	ROM_REGION( 0x0100, "syncprom", 0 )
	ROM_LOAD( "82s129-136022-108.7k",  0x0000, 0x0100, CRC(6ed31e3b) SHA1(c3f3e4e7f313ecfd101cc52dfc44bd6b51a2ac88) ) // vertical sync generation

	ROM_REGION( 0x0100, "addressprom", 0 )
	ROM_LOAD( "82s129-136022-109.6l",  0x0000, 0x0100, CRC(b3515f1a) SHA1(c1bf077242481ef2f958580602b8113532b58612) ) // address decoding

	ROM_REGION( 0x0100, "wpprom", 0 )
	ROM_LOAD( "82s129-136022-110.11l", 0x0000, 0x0100, CRC(068bdc7e) SHA1(ae155918fdafd14299bc448b43eed8ad9c1ef5ef) ) // DRAM write protection

	ROM_REGION( 0x0100, "priprom", 0 )
	ROM_LOAD( "82s129-136022-111.10k", 0x0000, 0x0100, CRC(c29c18d9) SHA1(278bf61a290ae72ddaae2bafb4ab6739d3fb6238) ) // color selection
ROM_END

} // anonymous namespace


/*************************************
 *
 *  Game drivers
 *
 *************************************/

GAME( 1983, ccastles,  0,        ccastles, ccastles,  ccastles_state, empty_init, ROT0, "Atari", "Crystal Castles (version 4)",          MACHINE_SUPPORTS_SAVE )
GAME( 1983, ccastlesg, ccastles, ccastles, ccastles,  ccastles_state, empty_init, ROT0, "Atari", "Crystal Castles (version 3, German)",  MACHINE_SUPPORTS_SAVE )
GAME( 1983, ccastlesp, ccastles, ccastles, ccastles,  ccastles_state, empty_init, ROT0, "Atari", "Crystal Castles (version 3, Spanish)", MACHINE_SUPPORTS_SAVE )
GAME( 1983, ccastlesf, ccastles, ccastles, ccastles,  ccastles_state, empty_init, ROT0, "Atari", "Crystal Castles (version 3, French)",  MACHINE_SUPPORTS_SAVE )
GAME( 1983, ccastles3, ccastles, ccastles, ccastles,  ccastles_state, empty_init, ROT0, "Atari", "Crystal Castles (version 3)",          MACHINE_SUPPORTS_SAVE )
GAME( 1983, ccastles2, ccastles, ccastles, ccastles,  ccastles_state, empty_init, ROT0, "Atari", "Crystal Castles (version 2)",          MACHINE_SUPPORTS_SAVE )
GAME( 1983, ccastles1, ccastles, ccastles, ccastles,  ccastles_state, empty_init, ROT0, "Atari", "Crystal Castles (version 1)",          MACHINE_SUPPORTS_SAVE )
GAME( 1983, ccastlesj, ccastles, ccastles, ccastlesj, ccastles_state, empty_init, ROT0, "Atari", "Crystal Castles (joystick version)",   MACHINE_SUPPORTS_SAVE )
