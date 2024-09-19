// license:BSD-3-Clause
// copyright-holders: Mike Balfour, Aaron Giles

/***************************************************************************

    Atari Cloud 9 (prototype) hardware

    driver by Mike Balfour

    Games supported:
        * Cloud 9
        * Firebeast

    Known issues:
        * none at this time

****************************************************************************

    Horizontal sync chain:

        Appears to be the same as Crystal Castles. See ccastles.cpp for
        details.

        Pixel clock = 5MHz
        HBLANK ends at H = 0
        HBLANK begins at H = 256
        HSYNC begins at H = 260 (? unconfirmed)
        HSYNC ends at H = 288 (? unconfirmed)
        HTOTAL = 320

    Vertical sync chain:

        Appears to be similar to Crystal Castles. The PROM at 10E seems
        to have a similar layout to the SYNC PROM used by Crystal
        Castles. The standard PROM maps as follows:

        VBLANK ends at V = 23
        VBLANK begins at V = 255
        VSYNC begins at V = 3
        VSYNC ends at V = 6
        VTOTAL = 256

    Interrupts:

        IRQ is clocked by /32V, so IRQs are generated a V = 0,64,128,192.

****************************************************************************

    This hardware is very similar to Crystal Castles. The primary
    difference is the lack of banked ROM and the mapping of the bitmap
    layer into the lower 20k instead of the lower 32k. In order to do
    this, they split the bitmap into two banks. Bank 0 holds pixels
    0,1,4,5,... while bank 1 holds pixels 2,3,6,7,... This is all handled
    transparently by bitmode.

    The lower 24 lines of video RAM are used for working RAM. This amounts
    to $600 bytes at $0000. In order to provide more work RAM, the write
    protect logic selects bank 1 for accesses to VRAM at $4000, so the
    other $600 bytes can be accessed there. Only Firebeast seems to use
    this RAM.

    0000        R/W     Write to bit mode X latch (and through to RAM)
    0001        R/W     Write to bit mode Y latch (and through to RAM)
    0002        R/W     Access the bitmap via bit mode
    0000-3FFF   R/W     Video RAM bank 0 (or 1 or both, depending on video control)
    4000-4FFF   R/W     Video RAM bank 1
    5000-53FF   R/W     Motion Object RAM
    5400        W       Watchdog
    5480        W       IRQ Acknowledge
    5500-557F   W       Color RAM (9 bits, 4 banks, LSB of Blue is addr&$40)
    5580        W       Auto-increment X bitmap index (~D7)
    5581        W       Auto-increment Y bitmap index (~D7)
    5584        W       VRAM Both Banks - (D7) seems to allow writing to both banks
    5585        W       Invert screen?
    5586        W       VRAM Bank select?
    5587        W       Color bank select
    5600        W       Coin Counter 1 (D7)
    5601        W       Coin Counter 2 (D7)
    5602        W       Start1 LED (~D7)
    5603        W       Start2 LED (~D7)
    5700        W       EAROM recall
    5800        R       IN0 (D7=Vblank, D6=Right Coin, D5=Left Coin, D4=Aux, D3=Self Test)
    5801        R       IN1 (D7=Start1, D6=Start2, D5=Fire, D4=Zap)
    5900        R       Trackball Vert
    5901        R       Trackball Horiz
    5A00-5A0F   R/W     Pokey 1
    5B00-5B0F   R/W     Pokey 2
    5C00-5CFF   W       EAROM
    6000-FFFF   R       Program ROM

    If you have any questions about how this driver works, don't hesitate to
    ask.  - Mike Balfour (mab22@po.cwru.edu)

***************************************************************************/

#include "emu.h"

#include "cpu/m6502/m6502.h"
#include "machine/74259.h"
#include "machine/watchdog.h"
#include "machine/x2212.h"
#include "sound/pokey.h"
#include "video/resnet.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"


namespace {

class cloud9_state : public driver_device
{
public:
	cloud9_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_nvram(*this, "nvram"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette"),
		m_videolatch(*this, "videolatch"),
		m_videoram(*this, "videoram", 0x8000U, ENDIANNESS_LITTLE),
		m_spriteram(*this, "spriteram"),
		m_paletteram(*this, "paletteram"),
		m_syncprom(*this, "syncprom"),
		m_wpprom(*this, "wpprom"),
		m_track(*this, "TRACK%c", 'X')
	{ }

	int vblank_r();
	void cloud9(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

private:
	// devices
	required_device<m6502_device> m_maincpu;
	required_device<x2212_device> m_nvram;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	required_device<ls259_device> m_videolatch;

	// memory pointers
	memory_share_creator<uint8_t> m_videoram;
	required_shared_ptr<uint8_t> m_spriteram;
	required_shared_ptr<uint8_t> m_paletteram;
	required_region_ptr<uint8_t> m_syncprom;
	required_region_ptr<uint8_t> m_wpprom;

	required_ioport_array<2> m_track;

	// video-related
	bitmap_ind16 m_spritebitmap = 0;
	double m_rweights[3]{};
	double m_gweights[3]{};
	double m_bweights[3]{};
	uint8_t m_bitmode_addr[2]{};

	// misc
	int m_vblank_start = 0;
	int m_vblank_end = 0;
	emu_timer *m_irq_timer = nullptr;
	uint8_t m_irq_state = 0U;

	void irq_ack_w(uint8_t data);
	uint8_t leta_r(offs_t offset);
	void nvram_recall_w(uint8_t data);
	void nvram_store_w(uint8_t data);
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
	void program_map(address_map &map) ATTR_COLD;
};


/*************************************
 *
 *  Video startup
 *
 *************************************/

void cloud9_state::video_start()
{
	static const int resistances[3] = { 22000, 10000, 4700 };

	// allocate second bank of videoram
	membank("videoram_bank")->set_base(m_videoram.target());

	// compute the color output resistor weights at startup
	compute_resistor_weights(0, 255, -1.0,
			3,  resistances, m_rweights, 1000, 0,
			3,  resistances, m_gweights, 1000, 0,
			3,  resistances, m_bweights, 1000, 0);

	// allocate a bitmap for drawing sprites
	m_screen->register_screen_bitmap(m_spritebitmap);

	// register for savestates
	save_item(NAME(m_bitmode_addr));
}



/*************************************
 *
 *  Palette RAM accesses
 *
 *************************************/

void cloud9_state::paletteram_w(offs_t offset, uint8_t data)
{
	int bit0, bit1, bit2;

	// extract the raw RGB bits
	int r = (data & 0xe0) >> 5;
	int g = (data & 0x1c) >> 2;
	int b = ((data & 0x03) << 1) | ((offset & 0x40) >> 6);

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

	m_palette->set_pen_color(offset & 0x3f, rgb_t(r, g, b));
}



/*************************************
 *
 *  Video RAM access via the write
 *  protect PROM
 *
 *************************************/

inline void cloud9_state::write_vram(uint16_t addr, uint8_t data, uint8_t bitmd, uint8_t pixba)
{
	uint8_t *dest = &m_videoram[0x0000 | (addr & 0x3fff)];
	uint8_t *dest2 = &m_videoram[0x4000 | (addr & 0x3fff)];
	uint8_t promaddr = 0;

	/*
	    Inputs to the write-protect PROM:

	    Bit 7 = BITMD
	    Bit 6 = video_control[4]
	    Bit 5 = video_control[6]
	    Bit 4 = 1 if (A15-A12 != 4)
	    Bit 3 = !(A13 | A12 | A11)
	    Bit 2 = A9 & A10
	    Bit 1 = PIXB
	    Bit 0 = PIXA
	*/
	promaddr |= bitmd << 7;
	promaddr |= m_videolatch->q4_r() << 6;
	promaddr |= m_videolatch->q6_r() << 5;
	promaddr |= ((addr & 0xf000) != 0x4000) << 4;
	promaddr |= ((addr & 0x3800) == 0x0000) << 3;
	promaddr |= ((addr & 0x0600) == 0x0600) << 2;
	promaddr |= (pixba << 0);

	// look up the PROM result
	uint8_t const wpbits = m_wpprom[promaddr];

	// write to the appropriate parts of VRAM depending on the result
	if (!(wpbits & 1))
		dest2[0] = (dest2[0] & 0x0f) | (data & 0xf0);
	if (!(wpbits & 2))
		dest2[0] = (dest2[0] & 0xf0) | (data & 0x0f);
	if (!(wpbits & 4))
		dest[0] = (dest[0] & 0x0f) | (data & 0xf0);
	if (!(wpbits & 8))
		dest[0] = (dest[0] & 0xf0) | (data & 0x0f);
}



/*************************************
 *
 *  Autoincrement control for bit mode
 *
 *************************************/

inline void cloud9_state::bitmode_autoinc()
{
	// auto increment in the x-direction if it's enabled
	if (!m_videolatch->q0_r()) // /AX
		m_bitmode_addr[0]++;

	// auto increment in the y-direction if it's enabled
	if (!m_videolatch->q1_r()) // /AY
		m_bitmode_addr[1]++;
}



/*************************************
 *
 *  Standard video RAM access
 *
 *************************************/

void cloud9_state::videoram_w(offs_t offset, uint8_t data)
{
	// direct writes to VRAM go through the write protect PROM as well
	write_vram(offset, data, 0, 0);
}



/*************************************
 *
 *  Bit mode video RAM access
 *
 *************************************/

uint8_t cloud9_state::bitmode_r()
{
	// in bitmode, the address comes from the autoincrement latches
	uint16_t const addr = (m_bitmode_addr[1] << 6) | (m_bitmode_addr[0] >> 2);

	// the appropriate pixel is selected into the upper 4 bits
	uint8_t const result = m_videoram[((~m_bitmode_addr[0] & 2) << 13) | addr] << ((m_bitmode_addr[0] & 1) * 4);

	// autoincrement because /BITMD was selected
	bitmode_autoinc();

	// the upper 4 bits of the data lines are not driven so make them all 1's
	return (result >> 4) | 0xf0;
}


void cloud9_state::bitmode_w(uint8_t data)
{
	// in bitmode, the address comes from the autoincrement latches
	uint16_t const addr = (m_bitmode_addr[1] << 6) | (m_bitmode_addr[0] >> 2);

	// the lower 4 bits of data are replicated to the upper 4 bits
	data = (data & 0x0f) | (data << 4);

	// write through the generic VRAM routine, passing the low 2 X bits as PIXB/PIXA
	write_vram(addr, data, 1, m_bitmode_addr[0] & 3);

	// autoincrement because /BITMD was selected
	bitmode_autoinc();
}


void cloud9_state::bitmode_addr_w(offs_t offset, uint8_t data)
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

uint32_t cloud9_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	uint8_t const *const spriteaddr = m_spriteram;
	int const flip = m_videolatch->q5_r() ? 0xff : 0x00;    // PLAYER2
	pen_t const black = m_palette->black_pen();

	// draw the sprites
	m_spritebitmap.fill(0x00, cliprect);
	for (int offs = 0; offs < 0x20; offs++)
		if (spriteaddr[offs + 0x00] != 0)
		{
			int const x = spriteaddr[offs + 0x60];
			int const y = 256 - 15 - spriteaddr[offs + 0x00];
			int const xflip = spriteaddr[offs + 0x40] & 0x80;
			int const yflip = spriteaddr[offs + 0x40] & 0x40;
			int const which = spriteaddr[offs + 0x20];
			int const color = 0;

			m_gfxdecode->gfx(0)->transpen(m_spritebitmap, cliprect, which, color, xflip, yflip, x, y, 0);
			if (x >= 256 - 16)
				m_gfxdecode->gfx(0)->transpen(m_spritebitmap, cliprect, which, color, xflip, yflip, x - 256, y, 0);
		}

	// draw the bitmap to the screen, looping over Y
	for (int y = cliprect.top(); y <= cliprect.bottom(); y++)
	{
		uint16_t *const dst = &bitmap.pix(y);

		// if we're in the VBLANK region, just fill with black
		if (~m_syncprom[y] & 2)
		{
			for (int x = cliprect.left(); x <= cliprect.right(); x++)
				dst[x] = black;
		}

		// non-VBLANK region: merge the sprites and the bitmap
		else
		{
			uint16_t const *const mosrc = &m_spritebitmap.pix(y);
			int const effy = y ^ flip;

			// two videoram arrays
			uint8_t const *const src[2]{ &m_videoram[0x4000 | (effy * 64)], &m_videoram[0x0000 | (effy * 64)] };

			// loop over X
			for (int x = cliprect.left(); x <= cliprect.right(); x++)
			{
				// if we're in the HBLANK region, just store black
				if (x >= 256)
					dst[x] = black;

				// otherwise, process normally
				else
				{
					int const effx = x ^ flip;

					// low 4 bits = left pixel, high 4 bits = right pixel
					uint8_t pix = (src[(effx >> 1) & 1][effx / 4] >> ((~effx & 1) * 4)) & 0x0f;
					uint8_t const mopix = mosrc[x];

					// sprites have priority if sprite pixel != 0 or some other condition
					if (mopix != 0)
						pix = mopix | 0x10;

					// the high bit is the bank select
					pix |= m_videolatch->q7_r() << 5;

					// store the pixel value and also a priority value based on the topmost bit
					dst[x] = pix;
				}
			}
		}
	}
	return 0;
}


static constexpr double MASTER_CLOCK = 10'000'000;
static constexpr double PIXEL_CLOCK = MASTER_CLOCK / 2;
static constexpr uint16_t HTOTAL = 320;
static constexpr uint16_t VTOTAL = 256;

/*************************************
 *
 *  VBLANK and IRQ generation
 *
 *************************************/

inline void cloud9_state::schedule_next_irq(int curscanline)
{
	// IRQ is clocked by /32V, so every 64 scanlines
	curscanline = (curscanline + 64) & 255;

	// next one at the start of this scanline
	m_irq_timer->adjust(m_screen->time_until_pos(curscanline), curscanline);
}


TIMER_CALLBACK_MEMBER(cloud9_state::clock_irq)
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


int cloud9_state::vblank_r()
{
	int const scanline = m_screen->vpos();
	return (~m_syncprom[scanline & 0xff] >> 1) & 1;
}



/*************************************
 *
 *  Machine setup
 *
 *************************************/

void cloud9_state::machine_start()
{
	rectangle visarea;

	// find the start of VBLANK in the SYNC PROM
	for (m_vblank_start = 0; m_vblank_start < 256; m_vblank_start++)
		if ((m_syncprom[(m_vblank_start - 1) & 0xff] & 2) != 0 && (m_syncprom[m_vblank_start] & 2) == 0)
			break;
	if (m_vblank_start == 0)
		m_vblank_start = 256;

	// find the end of VBLANK in the SYNC PROM
	for (m_vblank_end = 0; m_vblank_end < 256; m_vblank_end++)
		if ((m_syncprom[(m_vblank_end - 1) & 0xff] & 2) == 0 && (m_syncprom[m_vblank_end] & 2) != 0)
			break;

	// can't handle the wrapping case
	assert(m_vblank_end < m_vblank_start);

	// reconfigure the visible area to match
	visarea.set(0, 255, m_vblank_end + 1, m_vblank_start);
	m_screen->configure(320, 256, visarea, HZ_TO_ATTOSECONDS(PIXEL_CLOCK) * VTOTAL * HTOTAL);

	// create a timer for IRQs and set up the first callback
	m_irq_timer = timer_alloc(FUNC(cloud9_state::clock_irq), this);
	m_irq_state = 0;
	schedule_next_irq(0 - 64);

	// setup for save states
	save_item(NAME(m_irq_state));
}


void cloud9_state::machine_reset()
{
	m_maincpu->set_input_line(0, CLEAR_LINE);
	m_irq_state = 0;
}



/*************************************
 *
 *  Output ports
 *
 *************************************/

void cloud9_state::irq_ack_w(uint8_t data)
{
	if (m_irq_state)
	{
		m_maincpu->set_input_line(0, CLEAR_LINE);
		m_irq_state = 0;
	}
}


uint8_t cloud9_state::leta_r(offs_t offset)
{
	return offset ? m_track[0]->read() : m_track[1]->read();
}



/*************************************
 *
 *  NVRAM handling
 *
 *************************************/

void cloud9_state::nvram_recall_w(uint8_t data)
{
	m_nvram->recall(0);
	m_nvram->recall(1);
	m_nvram->recall(0);
}


void cloud9_state::nvram_store_w(uint8_t data)
{
	m_nvram->store(0);
	m_nvram->store(1);
	m_nvram->store(0);
}



/*************************************
 *
 *  Main CPU memory handlers
 *
 *************************************/

void cloud9_state::program_map(address_map &map)
{
	map(0x0000, 0x4fff).bankr("videoram_bank").w(FUNC(cloud9_state::videoram_w));
	map(0x0000, 0x0001).w(FUNC(cloud9_state::bitmode_addr_w));
	map(0x0002, 0x0002).rw(FUNC(cloud9_state::bitmode_r), FUNC(cloud9_state::bitmode_w));
	map(0x5000, 0x53ff).ram().share(m_spriteram);
	map(0x5400, 0x547f).w("watchdog", FUNC(watchdog_timer_device::reset_w));
	map(0x5480, 0x54ff).w(FUNC(cloud9_state::irq_ack_w));
	map(0x5500, 0x557f).ram().w(FUNC(cloud9_state::paletteram_w)).share(m_paletteram);
	map(0x5580, 0x5587).mirror(0x0078).w(m_videolatch, FUNC(ls259_device::write_d7)); // video control registers
	map(0x5600, 0x5607).mirror(0x0078).w("outlatch", FUNC(ls259_device::write_d7));
	map(0x5680, 0x56ff).w(FUNC(cloud9_state::nvram_store_w));
	map(0x5700, 0x577f).w(FUNC(cloud9_state::nvram_recall_w));
	map(0x5800, 0x5800).mirror(0x007e).portr("IN0");
	map(0x5801, 0x5801).mirror(0x007e).portr("IN1");
	map(0x5900, 0x5903).mirror(0x007c).r(FUNC(cloud9_state::leta_r));
	map(0x5a00, 0x5a0f).mirror(0x00f0).rw("pokey1", FUNC(pokey_device::read), FUNC(pokey_device::write));
	map(0x5b00, 0x5b0f).mirror(0x00f0).rw("pokey2", FUNC(pokey_device::read), FUNC(pokey_device::write));
	map(0x5c00, 0x5cff).mirror(0x0300).rw(m_nvram, FUNC(x2212_device::read), FUNC(x2212_device::write));
	map(0x6000, 0xffff).rom();
}



/*************************************
 *
 *  Port definitions
 *
 *************************************/

static INPUT_PORTS_START( cloud9 )
	PORT_START("IN0")
	PORT_BIT( 0x07, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_SERVICE( 0x08, IP_ACTIVE_LOW )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(cloud9_state, vblank_r)

	PORT_START("IN1")
	PORT_BIT( 0x0f, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START1 )

	PORT_START("DSW")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0xfe, 0x04, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x22, DEF_STR( 9C_1C ) )
	PORT_DIPSETTING(    0x1e, DEF_STR( 8C_1C ) )
	PORT_DIPSETTING(    0x1a, DEF_STR( 7C_1C ) )
	PORT_DIPSETTING(    0x16, DEF_STR( 6C_1C ) )
	PORT_DIPSETTING(    0x12, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x0e, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x0a, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Free_Play ) )

	PORT_START("TRACKY")
	PORT_BIT( 0xff, 0x00, IPT_TRACKBALL_Y ) PORT_SENSITIVITY(30) PORT_KEYDELTA(30) PORT_REVERSE

	PORT_START("TRACKX")
	PORT_BIT( 0xff, 0x00, IPT_TRACKBALL_X ) PORT_SENSITIVITY(30) PORT_KEYDELTA(30)
INPUT_PORTS_END


static INPUT_PORTS_START( firebeas )
	PORT_START("IN0")
	PORT_BIT( 0x07, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_SERVICE( 0x08, IP_ACTIVE_LOW )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(cloud9_state, vblank_r)

	PORT_START("IN1")
	PORT_BIT( 0x07, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START1 )

	PORT_START("DSW")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("TRACKY")
	PORT_BIT( 0xff, 0x00, IPT_TRACKBALL_Y ) PORT_SENSITIVITY(30) PORT_KEYDELTA(30) PORT_REVERSE

	PORT_START("TRACKX")
	PORT_BIT( 0xff, 0x00, IPT_TRACKBALL_X ) PORT_SENSITIVITY(30) PORT_KEYDELTA(30) PORT_REVERSE
INPUT_PORTS_END



/*************************************
 *
 *  Graphics definitions
 *
 *************************************/

static GFXDECODE_START( gfx_cloud9 )
	GFXDECODE_ENTRY( "sprites", 0x0000, gfx_16x16x4_planar, 0, 4 )
GFXDECODE_END


/*************************************
 *
 *  Machine driver
 *
 *************************************/

void cloud9_state::cloud9(machine_config &config)
{
	// basic machine hardware
	M6502(config, m_maincpu, MASTER_CLOCK / 8);
	m_maincpu->set_addrmap(AS_PROGRAM, &cloud9_state::program_map);

	ls259_device &outlatch(LS259(config, "outlatch"));
	outlatch.q_out_cb<0>().set([this] (int state) { machine().bookkeeping().coin_counter_w(0, state); });
	outlatch.q_out_cb<1>().set([this] (int state) { machine().bookkeeping().coin_counter_w(1, state); });
	outlatch.q_out_cb<2>().set_output("led0").invert();
	outlatch.q_out_cb<3>().set_output("led1").invert();

	WATCHDOG_TIMER(config, "watchdog").set_vblank_count("screen", 8);

	X2212(config, "nvram").set_auto_save(true);

	// video hardware
	GFXDECODE(config, m_gfxdecode, m_palette, gfx_cloud9);
	PALETTE(config, m_palette).set_entries(64);

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz((double)PIXEL_CLOCK / (double)VTOTAL / (double)HTOTAL);
	m_screen->set_size(HTOTAL, VTOTAL);
	m_screen->set_vblank_time(0);          // VBLANK is handled manually
	m_screen->set_visarea(0, 255, 0, 231);
	m_screen->set_screen_update(FUNC(cloud9_state::screen_update));
	m_screen->set_palette(m_palette);

	LS259(config, m_videolatch);

	// sound hardware
	SPEAKER(config, "mono").front_center();

	pokey_device &pokey1(POKEY(config, "pokey1", MASTER_CLOCK / 8));
	pokey1.add_route(ALL_OUTPUTS, "mono", 0.50);

	pokey_device &pokey2(POKEY(config, "pokey2", MASTER_CLOCK / 8));
	pokey2.allpot_r().set_ioport("DSW");
	pokey2.add_route(ALL_OUTPUTS, "mono", 0.50);
}



/*************************************
 *
 *  ROM definitions
 *
 *************************************/

ROM_START( cloud9 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "c9_6000.bin", 0x6000, 0x2000, CRC(b5d95d98) SHA1(9a347e5fc6e9e753e5c6972341725b5f4412e451) )
	ROM_LOAD( "c9_8000.bin", 0x8000, 0x2000, CRC(49af8f22) SHA1(c118372bec0c428c2b60d29df95f358b302d5e66) )
	ROM_LOAD( "c9_a000.bin", 0xa000, 0x2000, CRC(7cf404a6) SHA1(d20b662102f8426af51b1ca4ed8e18b00d711365) )
	ROM_LOAD( "c9_c000.bin", 0xc000, 0x2000, CRC(26a4d7df) SHA1(8eef0a5f5d1ff13eec75d0c50f5a5dea28486ae5) )
	ROM_LOAD( "c9_e000.bin", 0xe000, 0x2000, CRC(6e663bce) SHA1(4f4a5dc57ba6bc38a17973a6644849f6f5a2dfd1) )

	ROM_REGION( 0x4000, "sprites", 0 )
	ROM_LOAD( "c9_gfx0.bin", 0x0000, 0x1000, CRC(d01a8019) SHA1(a77d6125b116ab4bf9446e3b99469dad2719f7e5) )
	ROM_LOAD( "c9_gfx1.bin", 0x1000, 0x1000, CRC(514ac009) SHA1(f05081d8da47e650b0bd12cd00460c98a4f745b1) )
	ROM_LOAD( "c9_gfx2.bin", 0x2000, 0x1000, CRC(930c1ade) SHA1(ba22cb7b105da2ab8c40574e70f18d594d833452) )
	ROM_LOAD( "c9_gfx3.bin", 0x3000, 0x1000, CRC(27e9b88d) SHA1(a1d27e62eea9cdff662a3c160f650bbdb32b7f47) )

	ROM_REGION( 0x100, "syncprom", 0 )
	ROM_LOAD( "63s141.e10", 0x0000, 0x0100, BAD_DUMP CRC(8e98083f) SHA1(ed29c7ed2226613ed5d09ecef4e645e3b53f7f8d) )  // Sync PROM

	ROM_REGION( 0x100, "wpprom", 0 )
	ROM_LOAD( "82s129.p3",  0x0000, 0x0100, BAD_DUMP CRC(615d784d) SHA1(e7e6397ae45d6ae8b3670b457ede79c42d18d71f) )  // VRAM Write Protect PROM

	ROM_REGION( 0x200, "unkproms", 0 )
	ROM_LOAD( "63s141.m10", 0x0000, 0x0100, BAD_DUMP CRC(b0b039c0) SHA1(724fa88f3f3c62b3c9345cdb13e114a10b7bbdb0) )  // ??? PROM
	ROM_LOAD( "63s141.m8",  0x0100, 0x0100, BAD_DUMP CRC(6d7479ec) SHA1(7a7c30f5846b98afaaca2af9aab82416ebafe4cc) )  // ??? PROM
ROM_END


ROM_START( firebeas )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "6000.j2",  0x6000, 0x2000, CRC(dbd04782) SHA1(1237511b00a4121fae01a07bca05a76202d74058) )
	ROM_LOAD( "8000.kl2", 0x8000, 0x2000, CRC(96231ca4) SHA1(e865d4396968f94a284fe9993f066d55f9c225a4) )
	ROM_LOAD( "a000.lm2", 0xa000, 0x2000, CRC(f7e0bf25) SHA1(d116cbc7643a3328f7a1fe4ff03d8a087b8c7648) )
	ROM_LOAD( "c000.n2",  0xc000, 0x2000, CRC(43a91b74) SHA1(6b38703e793ebcbee7b060053485072d9dac6b8b) )
	ROM_LOAD( "e000.p2",  0xe000, 0x1000, CRC(8e5045ab) SHA1(8e5e8dd7710dc5ec68602f069dfc30e1bcaf7411) )
	ROM_RELOAD(           0xf000, 0x1000 )

	ROM_REGION( 0x8000, "sprites", 0 )
	ROM_LOAD( "mo0000.r12", 0x0000, 0x2000, CRC(5246c979) SHA1(a975ede0a6c2c91f4373ecba1e2f61f1aedcee62) )
	ROM_LOAD( "mo2000.p12", 0x2000, 0x2000, CRC(1a3b6d57) SHA1(d9015140e69fdc3f73113f0afc3be2579197017a) )
	ROM_LOAD( "mo4000.n12", 0x4000, 0x2000, CRC(69e18319) SHA1(3bf3cbe4ea06ea71928eff8a57c2ef7dc6e6716a) )
	ROM_LOAD( "mo6000.m12", 0x6000, 0x2000, CRC(b722997f) SHA1(65a2618ecd8b4923f30f59c1fb95124cf0391964) )

	ROM_REGION( 0x100, "syncprom", 0 )
	ROM_LOAD( "63s141.e10", 0x0000, 0x0100, CRC(8e98083f) SHA1(ed29c7ed2226613ed5d09ecef4e645e3b53f7f8d) )  // Sync PROM

	ROM_REGION( 0x100, "wpprom", 0 )
	ROM_LOAD( "82s129.p3",  0x0000, 0x0100, CRC(615d784d) SHA1(e7e6397ae45d6ae8b3670b457ede79c42d18d71f) )  // VRAM Write Protect PROM

	ROM_REGION( 0x200, "unkproms", 0 )
	ROM_LOAD( "63s141.m10", 0x0000, 0x0100, CRC(b0b039c0) SHA1(724fa88f3f3c62b3c9345cdb13e114a10b7bbdb0) )  // ??? PROM
	ROM_LOAD( "63s141.m8",  0x0100, 0x0100, CRC(6d7479ec) SHA1(7a7c30f5846b98afaaca2af9aab82416ebafe4cc) )  // ??? PROM
ROM_END

} // anonymous namespace


/*************************************
 *
 *  Game drivers
 *
 *************************************/

GAME( 1983, cloud9,   0, cloud9, cloud9,   cloud9_state, empty_init, ROT0, "Atari", "Cloud 9 (prototype)",   MACHINE_SUPPORTS_SAVE )
GAME( 1983, firebeas, 0, cloud9, firebeas, cloud9_state, empty_init, ROT0, "Atari", "Firebeast (prototype)", MACHINE_SUPPORTS_SAVE )
