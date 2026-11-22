// license:BSD-3-Clause
// copyright-holders:Ken Van Mersbergen (Dutchman2000)
/***************************************************************************

    Peter Pack Rat - T11 prototype
    Atari, Inc., 1984 (project 415)

    Project leader / engineer: Sam Ly     Programmer: Peter Thompson

    This is the original T11 version of Peter Pack Rat, before the game
    was moved to Atari System 1 (68010). Everything below was worked out
    from the 1984 source code (PK.MAC, PR.MAC, PACK.H, TALK.C, PIRQ.C,
    SETVCT.MAC, COLOR0.MAC, PIXCOL.C, DEPLOY.MAC) and from the 6502 sound
    program, not from a board, so treat it as a best guess.

    Main CPU: DEC T11, 16-bit bus. Addresses below are octal (as in the
    source) and hex.

    000000-017777  0000-1FFF  working RAM (4K words)
    020000         2000       R: switches (DWORD)          W: output latch
                                 D15 OUTFLAG (1 = 6502 took our byte)
                                 D14 cocktail switch (active low)
                                 D13-D8 player 2 jump/action/U/D/L/R
                                 D7  INFLAG (0 = byte waiting from 6502)
                                 D6  self test (active low)
                                 D5-D0 player 1 jump/action/U/D/L/R
                              W: D7-D5 ROM bank (3-7 = banks 0-4)
                                 D4 run LED, D3 video on, D1 cocktail flip
                                 D0 LISTEN (lets OUTFLAG interrupt the T11)
    024000         2800       W word: byte to 6502   R byte: byte from 6502
    024001         2801       W byte: acknowledge byte from 6502
    030000-033777  3000-37FF  colour RAM, 1K x 12 bits (IIIRRRGGGBBB)
                              address = alpha(2) : MO(4) : PF(4)
    034000         3800       playfield horizontal scroll
    035000         3A00       playfield vertical scroll
    036000-037777  3C00-3FFF  "pixie" pixel averaging RAM (4 bits)
    040000-043777  4000-47FF  playfield RAM, 32x32 stamps of 16x16
                              D11-D8 PF ROM number, D7-D0 stamp
    044000-044177  4800-487F  MO vertical position (48 used)
    044200-044377  4880-48FF  MO picture
                              D15 vflip, D14 bank+8, D13 hflip, D12 8x32,
                              D11-D9 bank, D8-D0 stamp (8x16)
    044400-044577  4900-497F  MO horizontal position
    050000-057777  5000-5FFF  alphanumerics, 64 x 32 words (rows 2-31 shown)
    100000-157777  8000-DFFF  program ROM
    160000-177777  E000-FFFF  banked ROM window (5 banks)

    Interrupts, wired like Atari System 2 (same vectors as SETVCT.MAC):
      CP3  vblank                 -> 100-114 / 140-154 (p7irq)
      CP2  OUTFLAG & LISTEN       -> 120-134 (xmit)
      CP1  byte from 6502 waiting -> 060-064 (xrec)

    Sound CPU: 6502, 4 POKEYs, NVRAM ("4 pokeys and NVRAM run by
    seperate 6502", PR.MAC)

    0000-0FFF  RAM
    1000-10FF  NVRAM
    1800-18FF  quad POKEY, chip selects are A4-A7 active low:
               18E0 POKEY 0, 18D0 POKEY 1, 18B0 POKEY 2, 1870 POKEY 3
    2000       R: D0-D2 coins (right/left/aux, active low)
                  D4 speech ready (code exists, no speech data in ROM)
                  D5 self test (active low)
                  D6 0 = byte waiting from T11
                  D7 1 = T11 has taken our last byte
    2800       R: byte from T11
    3000       W: watchdog
    3100       W: IRQ acknowledge (IRQ 4 times a frame, 240 Hz)
    3200       W: acknowledge byte from T11
    3300       W: byte to T11
    3400-3407  W: output latches (coin counters, LEDs, speech control)
    3500       W: speech data
    4000-FFFF  ROM

    Known gaps / guesses
    - screen size 288x240 is from the game's scroll limits and the alpha
      layout; pixel clock and blanking are typical Atari values
    - MO and playfield position offsets are guesses
    - pixie RAM (corner rounding of playfield pixels) is not emulated
    - cocktail flip is not emulated
    - playfield ROMs are rebuilt from Atari's PPS artwork, not dumps

***************************************************************************/

#include "emu.h"

#include "cpu/m6502/m6502.h"
#include "cpu/t11/t11.h"
#include "machine/nvram.h"
#include "machine/timer.h"
#include "sound/pokey.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"


namespace {

class peterpakt_state : public driver_device
{
public:
	peterpakt_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette"),
		m_gfxdecode(*this, "gfxdecode"),
		m_pokey(*this, "pokey%u", 0U),
		m_rombank(*this, "rombank"),
		m_cpram(*this, "cpram"),
		m_pfram(*this, "pfram"),
		m_moram(*this, "moram"),
		m_alpharam(*this, "alpharam"),
		m_in0(*this, "IN0"),
		m_in1(*this, "IN1"),
		m_led(*this, "led0")
	{ }

	void peterpakt(machine_config &config);

	int outflag_r() { return m_to6502_full ? 0 : 1; }
	int inflag_r() { return m_from6502_full ? 0 : 1; }

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	required_device<t11_device> m_maincpu;
	required_device<m6502_device> m_audiocpu;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device_array<pokey_device, 4> m_pokey;
	required_memory_bank m_rombank;
	required_shared_ptr<u16> m_cpram;
	required_shared_ptr<u16> m_pfram;
	required_shared_ptr<u16> m_moram;
	required_shared_ptr<u16> m_alpharam;
	required_ioport m_in0;
	required_ioport m_in1;
	output_finder<> m_led;

	emu_timer *m_vblank_off_timer = nullptr;

	u16 m_outlatch = 0;
	u16 m_hscroll = 0;
	u16 m_vscroll = 0;
	bool m_vblank_irq = false;

	u8 m_to6502_data = 0;
	bool m_to6502_full = false;
	u8 m_from6502_data = 0;
	bool m_from6502_full = false;

	void update_interrupts();
	TIMER_CALLBACK_MEMBER(vblank_off);
	void vblank_w(int state);
	TIMER_DEVICE_CALLBACK_MEMBER(sound_irq);

	// T11 side
	u16 switches_r();
	void outlatch_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	u16 port_r();
	void port_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	void cpram_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	void hscroll_w(offs_t offset, u16 data, u16 mem_mask = ~0) { COMBINE_DATA(&m_hscroll); }
	void vscroll_w(offs_t offset, u16 data, u16 mem_mask = ~0) { COMBINE_DATA(&m_vscroll); }

	// 6502 side
	u8 sound_status_r();
	u8 sound_data_r();
	void sound_ack_w(u8 data);
	void sound_data_w(u8 data);
	void sound_irq_ack_w(u8 data);
	void sound_latch_w(offs_t offset, u8 data);
	u8 quad_pokey_r(offs_t offset);
	void quad_pokey_w(offs_t offset, u8 data);

	u32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void main_map(address_map &map) ATTR_COLD;
	void sound_map(address_map &map) ATTR_COLD;
};


/*************************************
 *
 *  Interrupts and communications
 *
 *************************************/

void peterpakt_state::update_interrupts()
{
	// CP3: vblank
	m_maincpu->set_input_line(t11_device::CP3_LINE, m_vblank_irq ? ASSERT_LINE : CLEAR_LINE);

	// CP2: the 6502 has taken our byte, and LISTEN is set
	m_maincpu->set_input_line(t11_device::CP2_LINE, (!m_to6502_full && BIT(m_outlatch, 0)) ? ASSERT_LINE : CLEAR_LINE);

	// CP1: a byte from the 6502 is waiting
	m_maincpu->set_input_line(t11_device::CP1_LINE, m_from6502_full ? ASSERT_LINE : CLEAR_LINE);
}

void peterpakt_state::vblank_w(int state)
{
	if (state)
	{
		// one short pulse per frame
		m_vblank_irq = true;
		update_interrupts();
		m_vblank_off_timer->adjust(m_screen->time_until_pos(m_screen->vpos() + 1));
	}
}

TIMER_CALLBACK_MEMBER(peterpakt_state::vblank_off)
{
	m_vblank_irq = false;
	update_interrupts();
}

TIMER_DEVICE_CALLBACK_MEMBER(peterpakt_state::sound_irq)
{
	m_audiocpu->set_input_line(m6502_device::IRQ_LINE, ASSERT_LINE);
}


/*************************************
 *
 *  T11 I/O
 *
 *************************************/

u16 peterpakt_state::switches_r()
{
	return m_in0->read();
}

void peterpakt_state::outlatch_w(offs_t offset, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_outlatch);

	// D7-D5: ROM bank, 3-7 select banks 0-4 (PG0-PG4 in the source)
	const int bank = (m_outlatch >> 5) & 7;
	if (bank >= 3)
		m_rombank->set_entry(bank - 3);
	else
		logerror("%s: unknown ROM bank %d\n", machine().describe_context(), bank);

	m_led = BIT(m_outlatch, 4);     // run LED
	update_interrupts();            // D0 = LISTEN
}

u16 peterpakt_state::port_r()
{
	// byte from the 6502 (INPORT, read as a byte at 024000)
	return 0xff00 | m_from6502_data;
}

void peterpakt_state::port_w(offs_t offset, u16 data, u16 mem_mask)
{
	if (ACCESSING_BITS_0_7)
	{
		// OUTPORT: byte to the 6502
		m_to6502_data = data & 0xff;
		m_to6502_full = true;
	}
	else
	{
		// DATACK: byte write to 024001 acknowledges the byte from the 6502
		m_from6502_full = false;
	}
	update_interrupts();
}

void peterpakt_state::cpram_w(offs_t offset, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_cpram[offset]);

	// 12 bits: D11-D9 intensity, D8-D6 red, D5-D3 green, D2-D0 blue
	const u16 d = m_cpram[offset];
	const int i = (d >> 9) & 7;
	const int r = (d >> 6) & 7;
	const int g = (d >> 3) & 7;
	const int b = (d >> 0) & 7;
	const int scale = (i + 1) * 255;    // intensity 7 = full brightness
	m_palette->set_pen_color(offset, rgb_t(r * scale / 56, g * scale / 56, b * scale / 56));
}


/*************************************
 *
 *  6502 I/O
 *
 *************************************/

u8 peterpakt_state::sound_status_r()
{
	u8 result = m_in1->read() & 0x1f;
	if (BIT(m_in0->read(), 6))
		result |= 0x20;     // D5 = self test switch, shared with the T11
	if (!m_to6502_full)
		result |= 0x40;     // D6 = 0 when the T11 has sent a byte
	if (!m_from6502_full)
		result |= 0x80;     // D7 = 1 when the T11 has taken our byte
	return result;
}

u8 peterpakt_state::sound_data_r()
{
	return m_to6502_data;
}

void peterpakt_state::sound_ack_w(u8 data)
{
	// the sound program writes here right after reading 2800
	if (!machine().side_effects_disabled())
	{
		m_to6502_full = false;
		update_interrupts();
	}
}

void peterpakt_state::sound_data_w(u8 data)
{
	m_from6502_data = data;
	m_from6502_full = true;
	update_interrupts();
}

void peterpakt_state::sound_irq_ack_w(u8 data)
{
	m_audiocpu->set_input_line(m6502_device::IRQ_LINE, CLEAR_LINE);
}

void peterpakt_state::sound_latch_w(offs_t offset, u8 data)
{
	// 3400-3407: coin counters, LEDs and speech control (D7 of each)
	if (offset == 6 || offset == 7)
		machine().bookkeeping().coin_counter_w(offset - 6, BIT(data, 7));
}

u8 peterpakt_state::quad_pokey_r(offs_t offset)
{
	// A4-A7 are active-low chip selects for POKEYs 0-3
	const int sel = ~offset >> 4;
	for (int i = 0; i < 4; i++)
		if (BIT(sel, i))
			return m_pokey[i]->read(offset & 0x0f);
	return 0xff;
}

void peterpakt_state::quad_pokey_w(offs_t offset, u8 data)
{
	const int sel = ~offset >> 4;
	for (int i = 0; i < 4; i++)
		if (BIT(sel, i))
			m_pokey[i]->write(offset & 0x0f, data);
}


/*************************************
 *
 *  Video
 *
 *************************************/

u32 peterpakt_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	// D3 of the output latch: 0 blanks the screen
	if (!BIT(m_outlatch, 3))
	{
		bitmap.fill(1024, cliprect);
		return 0;
	}

	gfx_element *const angfx = m_gfxdecode->gfx(0);
	gfx_element *const pfgfx = m_gfxdecode->gfx(1);
	gfx_element *const mogfx = m_gfxdecode->gfx(2);

	for (int y = cliprect.min_y; y <= cliprect.max_y; y++)
	{
		// motion objects for this line; later entries are in front
		u8 mo[512];
		std::fill(std::begin(mo), std::end(mo), 0);

		for (int i = 0; i < 48; i++)
		{
			const u16 vpos = m_moram[0x00 + i];
			const u16 pict = m_moram[0x40 + i];
			const u16 hpos = m_moram[0x80 + i];

			// MO origin is the lower left corner, vertical counts up from the bottom
			const int height = BIT(pict, 12) ? 32 : 16;
			const int top = 239 - (vpos & 0x1ff) - (height - 1);
			int row = (y - top) & 0x1ff;
			if (row >= height)
				continue;
			if (BIT(pict, 15))
				row = height - 1 - row;

			int code = ((BIT(pict, 14) << 3 | ((pict >> 9) & 7)) << 9) | (pict & 0x1ff);
			code += row >> 4;
			const u8 *src = mogfx->get_data(code % mogfx->elements()) + (row & 15) * mogfx->rowbytes();
			const bool hflip = BIT(pict, 13);

			for (int px = 0; px < 8; px++)
			{
				const u8 pix = src[hflip ? 7 - px : px];
				if (pix)
					mo[(hpos + px) & 0x1ff] = pix;
			}
		}

		u16 *const dest = &bitmap.pix(y);
		const int pfy = (y + m_vscroll) & 0x1ff;
		const u16 *const pfrow = &m_pfram[(pfy >> 4) * 32];
		const u16 *const anrow = &m_alpharam[((y >> 3) + 2) * 64];

		for (int x = cliprect.min_x; x <= cliprect.max_x; x++)
		{
			const int pfx = (x + m_hscroll) & 0x1ff;
			const int pfcode = pfrow[pfx >> 4] & 0x0fff;
			const u8 pf = pfgfx->get_data(pfcode)[(pfy & 15) * pfgfx->rowbytes() + (pfx & 15)];

			const int ancode = anrow[x >> 3] & 0xff;
			const u8 an = angfx->get_data(ancode)[(y & 7) * angfx->rowbytes() + (x & 7)];

			dest[x] = (an << 8) | (mo[x] << 4) | pf;
		}
	}
	return 0;
}


/*************************************
 *
 *  Address maps
 *
 *************************************/

void peterpakt_state::main_map(address_map &map)
{
	map(0x0000, 0x1fff).ram();
	map(0x2000, 0x2001).mirror(0x07fe).rw(FUNC(peterpakt_state::switches_r), FUNC(peterpakt_state::outlatch_w));
	map(0x2800, 0x2801).mirror(0x07fe).rw(FUNC(peterpakt_state::port_r), FUNC(peterpakt_state::port_w));
	map(0x3000, 0x37ff).ram().w(FUNC(peterpakt_state::cpram_w)).share(m_cpram);
	map(0x3800, 0x3801).mirror(0x01fe).w(FUNC(peterpakt_state::hscroll_w));
	map(0x3a00, 0x3a01).mirror(0x01fe).w(FUNC(peterpakt_state::vscroll_w));
	map(0x3c00, 0x3fff).ram();     // pixie RAM, 4 bits
	map(0x4000, 0x47ff).ram().share(m_pfram);
	map(0x4800, 0x4fff).ram().share(m_moram);
	map(0x5000, 0x5fff).ram().share(m_alpharam);
	map(0x8000, 0xdfff).rom().region("maincpu", 0x0000);
	map(0xe000, 0xffff).bankr(m_rombank);
}

void peterpakt_state::sound_map(address_map &map)
{
	map(0x0000, 0x0fff).ram();
	map(0x1000, 0x10ff).mirror(0x0700).ram().share("nvram");
	map(0x1800, 0x18ff).mirror(0x0700).rw(FUNC(peterpakt_state::quad_pokey_r), FUNC(peterpakt_state::quad_pokey_w));
	map(0x2000, 0x2000).mirror(0x07ff).r(FUNC(peterpakt_state::sound_status_r));
	map(0x2800, 0x2800).mirror(0x07ff).r(FUNC(peterpakt_state::sound_data_r));
	map(0x3000, 0x3000).mirror(0x00ff).nopw();     // watchdog
	map(0x3100, 0x3100).mirror(0x00ff).w(FUNC(peterpakt_state::sound_irq_ack_w));
	map(0x3200, 0x3200).mirror(0x00ff).w(FUNC(peterpakt_state::sound_ack_w));
	map(0x3300, 0x3300).mirror(0x00ff).w(FUNC(peterpakt_state::sound_data_w));
	map(0x3400, 0x3407).mirror(0x00f8).w(FUNC(peterpakt_state::sound_latch_w));
	map(0x3500, 0x3500).mirror(0x00ff).nopw();     // speech data (no speech in this build)
	map(0x4000, 0xffff).rom();
}


/*************************************
 *
 *  Input ports
 *
 *************************************/

static INPUT_PORTS_START( peterpakt )
	PORT_START("IN0")   // T11 DWORD, 020000
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("P1 Throw / 1 Player Start") PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("P1 Jump / 2 Player Start") PORT_PLAYER(1)
	PORT_SERVICE( 0x0040, IP_ACTIVE_LOW )
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(FUNC(peterpakt_state::inflag_r))
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("P2 Throw") PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("P2 Jump") PORT_PLAYER(2)
	PORT_CONFNAME( 0x4000, 0x4000, DEF_STR( Cabinet ) )
	PORT_CONFSETTING(      0x4000, DEF_STR( Upright ) )
	PORT_CONFSETTING(      0x0000, DEF_STR( Cocktail ) )
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(FUNC(peterpakt_state::outflag_r))

	PORT_START("IN1")   // 6502 2000 (D6/D7 are added by the driver)
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )     // right
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )     // left
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN3 )     // aux
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )    // speech ready (no chip)
	PORT_BIT( 0xe0, IP_ACTIVE_HIGH, IPT_UNUSED )    // D5 self test (from IN0), D6/D7 talk flags
INPUT_PORTS_END


/*************************************
 *
 *  Graphics layouts
 *
 *************************************/

// 8x8 alphanumerics, 2 bits per pixel packed, 2 bytes per row
static const gfx_layout anlayout =
{
	8,8,
	RGN_FRAC(1,1),
	2,
	{ 0, 1 },
	{ STEP8(0,2) },
	{ STEP8(0,16) },
	8*16
};

// 16x16 playfield stamps, "packrat format" from PRPF.FOR:
// for every 8 pixels the LOW ROM holds (p1<<4|p0),(p5<<4|p4)
// and the HIG ROM holds (p3<<4|p2),(p7<<4|p6)
static const gfx_layout pflayout =
{
	16,16,
	RGN_FRAC(1,2),
	4,
	{ 0, 1, 2, 3 },
	{ 4, 0, RGN_FRAC(1,2)+4, RGN_FRAC(1,2)+0, 12, 8, RGN_FRAC(1,2)+12, RGN_FRAC(1,2)+8,
		20, 16, RGN_FRAC(1,2)+20, RGN_FRAC(1,2)+16, 28, 24, RGN_FRAC(1,2)+28, RGN_FRAC(1,2)+24 },
	{ STEP16(0,32) },
	64*8
};

// 8x16 motion object stamps, bit planes:
// LOW ROM = pixel bits 3 and 2, HIG ROM = pixel bits 1 and 0
// (checked against the PPS artwork in PAKSPI.PIF and PAKTUF.PIF)
// low byte holds pixels 0-3, high byte pixels 4-7
static const gfx_layout molayout =
{
	8,16,
	RGN_FRAC(1,2),
	4,
	{ 0, 4, RGN_FRAC(1,2)+0, RGN_FRAC(1,2)+4 },
	{ 0, 1, 2, 3, 8, 9, 10, 11 },
	{ STEP16(0,16) },
	32*8
};

static GFXDECODE_START( gfx_peterpakt )
	GFXDECODE_ENTRY( "alpha", 0, anlayout, 0x300, 1 )
	GFXDECODE_ENTRY( "pf",    0, pflayout, 0,     1 )
	GFXDECODE_ENTRY( "mo",    0, molayout, 0,     1 )
GFXDECODE_END


/*************************************
 *
 *  Machine setup
 *
 *************************************/

void peterpakt_state::machine_start()
{

	m_rombank->configure_entries(0, 5, memregion("maincpu")->base() + 0x6000, 0x2000);
	m_vblank_off_timer = timer_alloc(FUNC(peterpakt_state::vblank_off), this);

	m_palette->set_pen_color(1024, rgb_t::black());

	save_item(NAME(m_outlatch));
	save_item(NAME(m_hscroll));
	save_item(NAME(m_vscroll));
	save_item(NAME(m_vblank_irq));
	save_item(NAME(m_to6502_data));
	save_item(NAME(m_to6502_full));
	save_item(NAME(m_from6502_data));
	save_item(NAME(m_from6502_full));
}

void peterpakt_state::machine_reset()
{
	m_outlatch = 0;
	m_rombank->set_entry(0);
	m_vblank_irq = false;
	m_to6502_full = false;
	m_from6502_full = false;
	update_interrupts();
}

void peterpakt_state::peterpakt(machine_config &config)
{
	constexpr XTAL MASTER_CLOCK = 20_MHz_XTAL;
	constexpr XTAL VIDEO_CLOCK = 14.318181_MHz_XTAL;

	T11(config, m_maincpu, MASTER_CLOCK / 2);
	m_maincpu->set_initial_mode(0x36ff);    // start at 100000, as on System 2
	m_maincpu->set_addrmap(AS_PROGRAM, &peterpakt_state::main_map);

	M6502(config, m_audiocpu, VIDEO_CLOCK / 8);
	m_audiocpu->set_addrmap(AS_PROGRAM, &peterpakt_state::sound_map);
	TIMER(config, "soundirq").configure_periodic(FUNC(peterpakt_state::sound_irq), attotime::from_hz(240));

	config.set_perfect_quantum(m_maincpu);

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_peterpakt);
	PALETTE(config, m_palette).set_entries(1024 + 1);

	SCREEN(config, m_screen, 0);
	m_screen->set_raw(VIDEO_CLOCK / 2, 456, 0, 288, 262, 0, 240);
	m_screen->set_screen_update(FUNC(peterpakt_state::screen_update));
	m_screen->set_palette(m_palette);
	m_screen->screen_vblank().set(FUNC(peterpakt_state::vblank_w));

	SPEAKER(config, "mono").front_center();
	for (int i = 0; i < 4; i++)
	{
		POKEY(config, m_pokey[i], VIDEO_CLOCK / 8);
		m_pokey[i]->add_route(ALL_OUTPUTS, "mono", 0.50);
	}
}


/*************************************
 *
 *  ROM definitions
 *
 *************************************/

ROM_START( peterpakt )
	ROM_REGION( 0x10000, "maincpu", 0 )    // T11 code: ROM 0 = fixed area + bank 0, ROM 1 = banks 1-4
	ROM_LOAD16_BYTE( "pakrat_jun01_rom0_lo.bin", 0x00000, 0x4000, CRC(62dbcc8a) SHA1(c7ef27c438000cc54ab9bd3f9a1b254dc088f88c) )
	ROM_LOAD16_BYTE( "pakrat_jun01_rom0_hi.bin", 0x00001, 0x4000, CRC(1a8fe4f6) SHA1(17d62b5ae139607f42f8a2540e52ec0558398470) )
	ROM_LOAD16_BYTE( "pakrat_jun01_rom1_lo.bin", 0x08000, 0x4000, CRC(193190f2) SHA1(7dc08d41ac5315ddd5aaeee3f879541d25931c30) )
	ROM_LOAD16_BYTE( "pakrat_jun01_rom1_hi.bin", 0x08001, 0x4000, CRC(0e7942cc) SHA1(f5016830046fa8d8b5d0dccdf8f278ba2eba8799) )

	ROM_REGION( 0x10000, "audiocpu", 0 )   // 6502 code (HOST2 build)
	ROM_LOAD( "pakaudio-4k.bin", 0x04000, 0x4000, CRC(61b5ecaa) SHA1(22eedb837a09d8adb54310cc1479d38ccac0d36d) )
	ROM_LOAD( "pakaudio-8k.bin", 0x08000, 0x4000, CRC(aeabb55a) SHA1(3e3558ac3b580b9dff0c21c525797a963cf32eaa) )
	ROM_LOAD( "pakaudio-ck.bin", 0x0c000, 0x4000, CRC(34a0442b) SHA1(8e4a7e899209a2030b94986bb247942f516716a0) )

	ROM_REGION( 0x1000, "alpha", 0 )
	ROM_LOAD( "pakrat_alpha_numeric.bin", 0x00000, 0x1000, CRC(4efea8f4) SHA1(5296d28c6a6238ebd50a0d226c927492aef2292d) )

	ROM_REGION( 0x80000, "pf", ROMREGION_ERASE00 )  // rebuilt from Atari's PPS artwork (NSEWER, NTREE, NJUNK)
	ROM_LOAD( "pakrat_pf_rom02_sewer_lo.bin", 0x08000, 0x4000, CRC(64c2406e) SHA1(c01c69fbe60853c6c39c16dde832773b61cd78cd) )
	ROM_LOAD( "pakrat_pf_rom03_sewer_lo.bin", 0x0c000, 0x4000, CRC(b32af37f) SHA1(6f643cda5e59ee52bc547ec747fdd4e19b71505d) )
	ROM_LOAD( "pakrat_pf_rom04_sewer_lo.bin", 0x10000, 0x4000, CRC(58d3d974) SHA1(e7d67fee7c1121be545833dd475d02047bf1a8f0) )
	ROM_LOAD( "pakrat_pf_rom05_sewer_lo.bin", 0x14000, 0x4000, CRC(62509fef) SHA1(9de14e6f344be63fd91894773e9bb7017dd3c65c) )
	ROM_LOAD( "pakrat_pf_rom06_tree_lo.bin", 0x18000, 0x4000, CRC(bbcc02c8) SHA1(4d5824a87449374966674b37e427798816b5ab0e) )
	ROM_LOAD( "pakrat_pf_rom07_tree_lo.bin", 0x1c000, 0x4000, CRC(cc539314) SHA1(4d32df367d535e7d8ca39c30bcb61fc24484c47d) )
	ROM_LOAD( "pakrat_pf_rom08_tree_lo.bin", 0x20000, 0x4000, CRC(d94f76b2) SHA1(c2c1a026d46cb8d4a830fdaf50c3d56031053c22) )
	ROM_LOAD( "pakrat_pf_rom09_tree_lo.bin", 0x24000, 0x4000, CRC(1eeac93c) SHA1(cd9571fc1eda4f60d987dd90dcff779ec57c6f40) )
	ROM_LOAD( "pakrat_pf_rom10_junk_lo.bin", 0x28000, 0x4000, CRC(dd38ce64) SHA1(618702566015d065d7e2f58695151db511960bac) )
	ROM_LOAD( "pakrat_pf_rom11_junk_lo.bin", 0x2c000, 0x4000, CRC(339fcd0b) SHA1(7307a0ce06b392734c8253269f089439172d3f46) )
	ROM_LOAD( "pakrat_pf_rom02_sewer_hi.bin", 0x48000, 0x4000, CRC(8fb51b80) SHA1(e328520ddfd9048c0b435c2bd783928e70428507) )
	ROM_LOAD( "pakrat_pf_rom03_sewer_hi.bin", 0x4c000, 0x4000, CRC(cea90ba0) SHA1(f4814bc89ce0a9079ad296cfbb8e4aba0b276eef) )
	ROM_LOAD( "pakrat_pf_rom04_sewer_hi.bin", 0x50000, 0x4000, CRC(4878348c) SHA1(33bbfaba3edbf5a6c559a5d58ef54c46f390aa94) )
	ROM_LOAD( "pakrat_pf_rom05_sewer_hi.bin", 0x54000, 0x4000, CRC(5120acbf) SHA1(a6fafcdbe4ecf8e21c61341f286fa7376c40fd0e) )
	ROM_LOAD( "pakrat_pf_rom06_tree_hi.bin", 0x58000, 0x4000, CRC(ee1d37fe) SHA1(456b39fdc646be61cc986d96042b5e85615aec6d) )
	ROM_LOAD( "pakrat_pf_rom07_tree_hi.bin", 0x5c000, 0x4000, CRC(72069eec) SHA1(b6386d6f480d96f7ad82a11bc9ef81afbcafb0a7) )
	ROM_LOAD( "pakrat_pf_rom08_tree_hi.bin", 0x60000, 0x4000, CRC(919fb8c6) SHA1(035c637fcd8d8c73dbaccad08e2f857226983d8b) )
	ROM_LOAD( "pakrat_pf_rom09_tree_hi.bin", 0x64000, 0x4000, CRC(c0a3d459) SHA1(90fca92ad271ad0587817791dd462e6a002c3d20) )
	ROM_LOAD( "pakrat_pf_rom10_junk_hi.bin", 0x68000, 0x4000, CRC(6b76b890) SHA1(64985b9a825beb9242d6e928f515ddef564690a2) )
	ROM_LOAD( "pakrat_pf_rom11_junk_hi.bin", 0x6c000, 0x4000, CRC(a163e848) SHA1(7fa9915c8af1b04028677b078a534802dd8f05c7) )

	ROM_REGION( 0x80000, "mo", ROMREGION_ERASE00 )  // banks 0-9, LOW then HIG
	ROM_LOAD( "pakrat_gfx_bank0_walk_lo.bin", 0x00000, 0x4000, CRC(357863e4) SHA1(3f82a98a8c12c2a306455a0926de500a54b06b3d) )
	ROM_LOAD( "pakrat_gfx_bank1_actn0_lo.bin", 0x04000, 0x4000, CRC(4efe7e31) SHA1(ee5bf301df2b2cae08197d6636ceb5a0efe0ec78) )
	ROM_LOAD( "pakrat_gfx_bank2_actn1_lo.bin", 0x08000, 0x4000, CRC(f0265530) SHA1(807b6f026ccf28e142fc9d2d0dec4052474965d4) )
	ROM_LOAD( "pakrat_gfx_bank3_pakall_lo.bin", 0x0c000, 0x4000, CRC(8ad92683) SHA1(7c751122bf7d0c37d105dfbfb248484156fbda44) )
	ROM_LOAD( "pakrat_gfx_bank4_tough_lo.bin", 0x10000, 0x4000, CRC(bd4d4421) SHA1(8b1d24b0a4654ac95bd8c7aa34570d3cfcdbef42) )
	ROM_LOAD( "pakrat_gfx_bank5_buldog_lo.bin", 0x14000, 0x4000, CRC(660733fb) SHA1(2cbf521ec1c92a6c967681c081cd99c516ca74b9) )
	ROM_LOAD( "pakrat_gfx_bank6_pakspi_lo.bin", 0x18000, 0x4000, CRC(1ddc4441) SHA1(62ce32aaa6559992fd48761b319963f89d62971b) )
	ROM_LOAD( "pakrat_gfx_bank7_packrat_lo.bin", 0x1c000, 0x4000, CRC(2c9163e5) SHA1(5c921483ffb6c6597970a0569115e039b2cba662) )
	ROM_LOAD( "pakrat_gfx_bank8_stooj_lo.bin", 0x20000, 0x4000, CRC(262c0f46) SHA1(4715b283967923f00b275339db59d4cdc1ed1627) )
	ROM_LOAD( "pakrat_gfx_bank9_obj_lo.bin", 0x24000, 0x4000, CRC(2fc468e6) SHA1(a250228ee0e718148627abff22b39764e5e63cf2) )
	ROM_LOAD( "pakrat_gfx_bank0_walk_hi.bin", 0x40000, 0x4000, CRC(5ddac992) SHA1(29a51eefd5025ae302ea7b58956fdcbc6364febc) )
	ROM_LOAD( "pakrat_gfx_bank1_actn0_hi.bin", 0x44000, 0x4000, CRC(91dbf695) SHA1(e88aa92c19a720536a7e10a4217fafa6ad51a952) )
	ROM_LOAD( "pakrat_gfx_bank2_actn1_hi.bin", 0x48000, 0x4000, CRC(a7863c21) SHA1(cb04aaf7c70fa7f5e1cd015f7a16c544ec863358) )
	ROM_LOAD( "pakrat_gfx_bank3_pakall_hi.bin", 0x4c000, 0x4000, CRC(e97c800a) SHA1(b4f7c286a9a13841e60b5137650db72c6476fb09) )
	ROM_LOAD( "pakrat_gfx_bank4_tough_hi.bin", 0x50000, 0x4000, CRC(7252b9a2) SHA1(f83d8bf1eb164954fc3d0c9b46302c196b96e6a2) )
	ROM_LOAD( "pakrat_gfx_bank5_buldog_hi.bin", 0x54000, 0x4000, CRC(16546ef0) SHA1(2bc9e8d18c0794bf19f92327da2e820056aa5f4d) )
	ROM_LOAD( "pakrat_gfx_bank6_pakspi_hi.bin", 0x58000, 0x4000, CRC(2a48381b) SHA1(1f0fb3645917113d73b0f82020943ba8de17c2ff) )
	ROM_LOAD( "pakrat_gfx_bank7_packrat_hi.bin", 0x5c000, 0x4000, CRC(fb302123) SHA1(7a3991df73d94160a139c182d07be47652489dcc) )
	ROM_LOAD( "pakrat_gfx_bank8_stooj_hi.bin", 0x60000, 0x4000, CRC(c0296f76) SHA1(9acec6fe873352f17a53846796f8d7057aceac78) )
	ROM_LOAD( "pakrat_gfx_bank9_obj_hi.bin", 0x64000, 0x4000, CRC(83544432) SHA1(575c318357afffbf4bbfe64ecf6470d9bb88b47a) )
ROM_END

ROM_START( peterpakta )
	ROM_REGION( 0x10000, "maincpu", 0 )    // T11 code: ROM 0 = fixed area + bank 0, ROM 1 = banks 1-4
	ROM_LOAD16_BYTE( "pakrat_may11_rom0_lo.bin", 0x00000, 0x4000, CRC(882cc19f) SHA1(7631d4d7d0533d7e77aedcd844f5b84457e9dc20) )
	ROM_LOAD16_BYTE( "pakrat_may11_rom0_hi.bin", 0x00001, 0x4000, CRC(97ca50ec) SHA1(dc95f161c15fb4d826d85f9949c9577e7418d413) )
	ROM_LOAD16_BYTE( "pakrat_may11_rom1_lo.bin", 0x08000, 0x4000, CRC(193190f2) SHA1(7dc08d41ac5315ddd5aaeee3f879541d25931c30) )
	ROM_LOAD16_BYTE( "pakrat_may11_rom1_hi.bin", 0x08001, 0x4000, CRC(0e7942cc) SHA1(f5016830046fa8d8b5d0dccdf8f278ba2eba8799) )

	ROM_REGION( 0x10000, "audiocpu", 0 )   // 6502 code (HOST2 build)
	ROM_LOAD( "pakaudio-4k.bin", 0x04000, 0x4000, CRC(61b5ecaa) SHA1(22eedb837a09d8adb54310cc1479d38ccac0d36d) )
	ROM_LOAD( "pakaudio-8k.bin", 0x08000, 0x4000, CRC(aeabb55a) SHA1(3e3558ac3b580b9dff0c21c525797a963cf32eaa) )
	ROM_LOAD( "pakaudio-ck.bin", 0x0c000, 0x4000, CRC(34a0442b) SHA1(8e4a7e899209a2030b94986bb247942f516716a0) )

	ROM_REGION( 0x1000, "alpha", 0 )
	ROM_LOAD( "pakrat_alpha_numeric.bin", 0x00000, 0x1000, CRC(4efea8f4) SHA1(5296d28c6a6238ebd50a0d226c927492aef2292d) )

	ROM_REGION( 0x80000, "pf", ROMREGION_ERASE00 )  // rebuilt from Atari's PPS artwork (NSEWER, NTREE, NJUNK)
	ROM_LOAD( "pakrat_pf_rom02_sewer_lo.bin", 0x08000, 0x4000, CRC(64c2406e) SHA1(c01c69fbe60853c6c39c16dde832773b61cd78cd) )
	ROM_LOAD( "pakrat_pf_rom03_sewer_lo.bin", 0x0c000, 0x4000, CRC(b32af37f) SHA1(6f643cda5e59ee52bc547ec747fdd4e19b71505d) )
	ROM_LOAD( "pakrat_pf_rom04_sewer_lo.bin", 0x10000, 0x4000, CRC(58d3d974) SHA1(e7d67fee7c1121be545833dd475d02047bf1a8f0) )
	ROM_LOAD( "pakrat_pf_rom05_sewer_lo.bin", 0x14000, 0x4000, CRC(62509fef) SHA1(9de14e6f344be63fd91894773e9bb7017dd3c65c) )
	ROM_LOAD( "pakrat_pf_rom06_tree_lo.bin", 0x18000, 0x4000, CRC(bbcc02c8) SHA1(4d5824a87449374966674b37e427798816b5ab0e) )
	ROM_LOAD( "pakrat_pf_rom07_tree_lo.bin", 0x1c000, 0x4000, CRC(cc539314) SHA1(4d32df367d535e7d8ca39c30bcb61fc24484c47d) )
	ROM_LOAD( "pakrat_pf_rom08_tree_lo.bin", 0x20000, 0x4000, CRC(d94f76b2) SHA1(c2c1a026d46cb8d4a830fdaf50c3d56031053c22) )
	ROM_LOAD( "pakrat_pf_rom09_tree_lo.bin", 0x24000, 0x4000, CRC(1eeac93c) SHA1(cd9571fc1eda4f60d987dd90dcff779ec57c6f40) )
	ROM_LOAD( "pakrat_pf_rom10_junk_lo.bin", 0x28000, 0x4000, CRC(dd38ce64) SHA1(618702566015d065d7e2f58695151db511960bac) )
	ROM_LOAD( "pakrat_pf_rom11_junk_lo.bin", 0x2c000, 0x4000, CRC(339fcd0b) SHA1(7307a0ce06b392734c8253269f089439172d3f46) )
	ROM_LOAD( "pakrat_pf_rom02_sewer_hi.bin", 0x48000, 0x4000, CRC(8fb51b80) SHA1(e328520ddfd9048c0b435c2bd783928e70428507) )
	ROM_LOAD( "pakrat_pf_rom03_sewer_hi.bin", 0x4c000, 0x4000, CRC(cea90ba0) SHA1(f4814bc89ce0a9079ad296cfbb8e4aba0b276eef) )
	ROM_LOAD( "pakrat_pf_rom04_sewer_hi.bin", 0x50000, 0x4000, CRC(4878348c) SHA1(33bbfaba3edbf5a6c559a5d58ef54c46f390aa94) )
	ROM_LOAD( "pakrat_pf_rom05_sewer_hi.bin", 0x54000, 0x4000, CRC(5120acbf) SHA1(a6fafcdbe4ecf8e21c61341f286fa7376c40fd0e) )
	ROM_LOAD( "pakrat_pf_rom06_tree_hi.bin", 0x58000, 0x4000, CRC(ee1d37fe) SHA1(456b39fdc646be61cc986d96042b5e85615aec6d) )
	ROM_LOAD( "pakrat_pf_rom07_tree_hi.bin", 0x5c000, 0x4000, CRC(72069eec) SHA1(b6386d6f480d96f7ad82a11bc9ef81afbcafb0a7) )
	ROM_LOAD( "pakrat_pf_rom08_tree_hi.bin", 0x60000, 0x4000, CRC(919fb8c6) SHA1(035c637fcd8d8c73dbaccad08e2f857226983d8b) )
	ROM_LOAD( "pakrat_pf_rom09_tree_hi.bin", 0x64000, 0x4000, CRC(c0a3d459) SHA1(90fca92ad271ad0587817791dd462e6a002c3d20) )
	ROM_LOAD( "pakrat_pf_rom10_junk_hi.bin", 0x68000, 0x4000, CRC(6b76b890) SHA1(64985b9a825beb9242d6e928f515ddef564690a2) )
	ROM_LOAD( "pakrat_pf_rom11_junk_hi.bin", 0x6c000, 0x4000, CRC(a163e848) SHA1(7fa9915c8af1b04028677b078a534802dd8f05c7) )

	ROM_REGION( 0x80000, "mo", ROMREGION_ERASE00 )  // banks 0-9, LOW then HIG
	ROM_LOAD( "pakrat_gfx_bank0_walk_lo.bin", 0x00000, 0x4000, CRC(357863e4) SHA1(3f82a98a8c12c2a306455a0926de500a54b06b3d) )
	ROM_LOAD( "pakrat_gfx_bank1_actn0_lo.bin", 0x04000, 0x4000, CRC(4efe7e31) SHA1(ee5bf301df2b2cae08197d6636ceb5a0efe0ec78) )
	ROM_LOAD( "pakrat_gfx_bank2_actn1_lo.bin", 0x08000, 0x4000, CRC(f0265530) SHA1(807b6f026ccf28e142fc9d2d0dec4052474965d4) )
	ROM_LOAD( "pakrat_gfx_bank3_pakall_lo.bin", 0x0c000, 0x4000, CRC(8ad92683) SHA1(7c751122bf7d0c37d105dfbfb248484156fbda44) )
	ROM_LOAD( "pakrat_gfx_bank4_tough_lo.bin", 0x10000, 0x4000, CRC(bd4d4421) SHA1(8b1d24b0a4654ac95bd8c7aa34570d3cfcdbef42) )
	ROM_LOAD( "pakrat_gfx_bank5_buldog_lo.bin", 0x14000, 0x4000, CRC(660733fb) SHA1(2cbf521ec1c92a6c967681c081cd99c516ca74b9) )
	ROM_LOAD( "pakrat_gfx_bank6_pakspi_lo.bin", 0x18000, 0x4000, CRC(1ddc4441) SHA1(62ce32aaa6559992fd48761b319963f89d62971b) )
	ROM_LOAD( "pakrat_gfx_bank7_packrat_lo.bin", 0x1c000, 0x4000, CRC(2c9163e5) SHA1(5c921483ffb6c6597970a0569115e039b2cba662) )
	ROM_LOAD( "pakrat_gfx_bank8_stooj_lo.bin", 0x20000, 0x4000, CRC(262c0f46) SHA1(4715b283967923f00b275339db59d4cdc1ed1627) )
	ROM_LOAD( "pakrat_gfx_bank9_obj_lo.bin", 0x24000, 0x4000, CRC(2fc468e6) SHA1(a250228ee0e718148627abff22b39764e5e63cf2) )
	ROM_LOAD( "pakrat_gfx_bank0_walk_hi.bin", 0x40000, 0x4000, CRC(5ddac992) SHA1(29a51eefd5025ae302ea7b58956fdcbc6364febc) )
	ROM_LOAD( "pakrat_gfx_bank1_actn0_hi.bin", 0x44000, 0x4000, CRC(91dbf695) SHA1(e88aa92c19a720536a7e10a4217fafa6ad51a952) )
	ROM_LOAD( "pakrat_gfx_bank2_actn1_hi.bin", 0x48000, 0x4000, CRC(a7863c21) SHA1(cb04aaf7c70fa7f5e1cd015f7a16c544ec863358) )
	ROM_LOAD( "pakrat_gfx_bank3_pakall_hi.bin", 0x4c000, 0x4000, CRC(e97c800a) SHA1(b4f7c286a9a13841e60b5137650db72c6476fb09) )
	ROM_LOAD( "pakrat_gfx_bank4_tough_hi.bin", 0x50000, 0x4000, CRC(7252b9a2) SHA1(f83d8bf1eb164954fc3d0c9b46302c196b96e6a2) )
	ROM_LOAD( "pakrat_gfx_bank5_buldog_hi.bin", 0x54000, 0x4000, CRC(16546ef0) SHA1(2bc9e8d18c0794bf19f92327da2e820056aa5f4d) )
	ROM_LOAD( "pakrat_gfx_bank6_pakspi_hi.bin", 0x58000, 0x4000, CRC(2a48381b) SHA1(1f0fb3645917113d73b0f82020943ba8de17c2ff) )
	ROM_LOAD( "pakrat_gfx_bank7_packrat_hi.bin", 0x5c000, 0x4000, CRC(fb302123) SHA1(7a3991df73d94160a139c182d07be47652489dcc) )
	ROM_LOAD( "pakrat_gfx_bank8_stooj_hi.bin", 0x60000, 0x4000, CRC(c0296f76) SHA1(9acec6fe873352f17a53846796f8d7057aceac78) )
	ROM_LOAD( "pakrat_gfx_bank9_obj_hi.bin", 0x64000, 0x4000, CRC(83544432) SHA1(575c318357afffbf4bbfe64ecf6470d9bb88b47a) )
ROM_END

ROM_START( peterpakts )
	ROM_REGION( 0x10000, "maincpu", 0 )    // T11 code rebuilt from the 1984 source (MAC11 + DECUS C + LINKM)
	ROM_LOAD16_BYTE( "pakrat_src_rom0_lo.bin", 0x0000, 0x4000, CRC(4d690ada) SHA1(19f989517f7ebd32e6679f2ea201b346dfe62199) )
	ROM_LOAD16_BYTE( "pakrat_src_rom0_hi.bin", 0x0001, 0x4000, CRC(858db3c5) SHA1(469a72a4f2bc479e3155762126c46ab3e1d9a1a5) )
	ROM_LOAD16_BYTE( "pakrat_src_rom1_lo.bin", 0x8000, 0x4000, CRC(8e2f9b90) SHA1(09efb3f9d9bf14d4857e9caf6dee26c32ae3c13e) )
	ROM_LOAD16_BYTE( "pakrat_src_rom1_hi.bin", 0x8001, 0x4000, CRC(59efd482) SHA1(47a4d264951b2733a376f5093d8634fb7e9c9a80) )

	ROM_REGION( 0x10000, "audiocpu", 0 )   // 6502 code (HOST2 build)
	ROM_LOAD( "pakaudio-4k.bin", 0x04000, 0x4000, CRC(61b5ecaa) SHA1(22eedb837a09d8adb54310cc1479d38ccac0d36d) )
	ROM_LOAD( "pakaudio-8k.bin", 0x08000, 0x4000, CRC(aeabb55a) SHA1(3e3558ac3b580b9dff0c21c525797a963cf32eaa) )
	ROM_LOAD( "pakaudio-ck.bin", 0x0c000, 0x4000, CRC(34a0442b) SHA1(8e4a7e899209a2030b94986bb247942f516716a0) )

	ROM_REGION( 0x1000, "alpha", 0 )
	ROM_LOAD( "pakrat_alpha_numeric.bin", 0x00000, 0x1000, CRC(4efea8f4) SHA1(5296d28c6a6238ebd50a0d226c927492aef2292d) )

	ROM_REGION( 0x80000, "pf", ROMREGION_ERASE00 )  // rebuilt from Atari's PPS artwork (NSEWER, NTREE, NJUNK)
	ROM_LOAD( "pakrat_pf_rom02_sewer_lo.bin", 0x08000, 0x4000, CRC(64c2406e) SHA1(c01c69fbe60853c6c39c16dde832773b61cd78cd) )
	ROM_LOAD( "pakrat_pf_rom03_sewer_lo.bin", 0x0c000, 0x4000, CRC(b32af37f) SHA1(6f643cda5e59ee52bc547ec747fdd4e19b71505d) )
	ROM_LOAD( "pakrat_pf_rom04_sewer_lo.bin", 0x10000, 0x4000, CRC(58d3d974) SHA1(e7d67fee7c1121be545833dd475d02047bf1a8f0) )
	ROM_LOAD( "pakrat_pf_rom05_sewer_lo.bin", 0x14000, 0x4000, CRC(62509fef) SHA1(9de14e6f344be63fd91894773e9bb7017dd3c65c) )
	ROM_LOAD( "pakrat_pf_rom06_tree_lo.bin", 0x18000, 0x4000, CRC(bbcc02c8) SHA1(4d5824a87449374966674b37e427798816b5ab0e) )
	ROM_LOAD( "pakrat_pf_rom07_tree_lo.bin", 0x1c000, 0x4000, CRC(cc539314) SHA1(4d32df367d535e7d8ca39c30bcb61fc24484c47d) )
	ROM_LOAD( "pakrat_pf_rom08_tree_lo.bin", 0x20000, 0x4000, CRC(d94f76b2) SHA1(c2c1a026d46cb8d4a830fdaf50c3d56031053c22) )
	ROM_LOAD( "pakrat_pf_rom09_tree_lo.bin", 0x24000, 0x4000, CRC(1eeac93c) SHA1(cd9571fc1eda4f60d987dd90dcff779ec57c6f40) )
	ROM_LOAD( "pakrat_pf_rom10_junk_lo.bin", 0x28000, 0x4000, CRC(dd38ce64) SHA1(618702566015d065d7e2f58695151db511960bac) )
	ROM_LOAD( "pakrat_pf_rom11_junk_lo.bin", 0x2c000, 0x4000, CRC(339fcd0b) SHA1(7307a0ce06b392734c8253269f089439172d3f46) )
	ROM_LOAD( "pakrat_pf_rom02_sewer_hi.bin", 0x48000, 0x4000, CRC(8fb51b80) SHA1(e328520ddfd9048c0b435c2bd783928e70428507) )
	ROM_LOAD( "pakrat_pf_rom03_sewer_hi.bin", 0x4c000, 0x4000, CRC(cea90ba0) SHA1(f4814bc89ce0a9079ad296cfbb8e4aba0b276eef) )
	ROM_LOAD( "pakrat_pf_rom04_sewer_hi.bin", 0x50000, 0x4000, CRC(4878348c) SHA1(33bbfaba3edbf5a6c559a5d58ef54c46f390aa94) )
	ROM_LOAD( "pakrat_pf_rom05_sewer_hi.bin", 0x54000, 0x4000, CRC(5120acbf) SHA1(a6fafcdbe4ecf8e21c61341f286fa7376c40fd0e) )
	ROM_LOAD( "pakrat_pf_rom06_tree_hi.bin", 0x58000, 0x4000, CRC(ee1d37fe) SHA1(456b39fdc646be61cc986d96042b5e85615aec6d) )
	ROM_LOAD( "pakrat_pf_rom07_tree_hi.bin", 0x5c000, 0x4000, CRC(72069eec) SHA1(b6386d6f480d96f7ad82a11bc9ef81afbcafb0a7) )
	ROM_LOAD( "pakrat_pf_rom08_tree_hi.bin", 0x60000, 0x4000, CRC(919fb8c6) SHA1(035c637fcd8d8c73dbaccad08e2f857226983d8b) )
	ROM_LOAD( "pakrat_pf_rom09_tree_hi.bin", 0x64000, 0x4000, CRC(c0a3d459) SHA1(90fca92ad271ad0587817791dd462e6a002c3d20) )
	ROM_LOAD( "pakrat_pf_rom10_junk_hi.bin", 0x68000, 0x4000, CRC(6b76b890) SHA1(64985b9a825beb9242d6e928f515ddef564690a2) )
	ROM_LOAD( "pakrat_pf_rom11_junk_hi.bin", 0x6c000, 0x4000, CRC(a163e848) SHA1(7fa9915c8af1b04028677b078a534802dd8f05c7) )

	ROM_REGION( 0x80000, "mo", ROMREGION_ERASE00 )  // banks 0-9, LOW then HIG
	ROM_LOAD( "pakrat_gfx_bank0_walk_lo.bin", 0x00000, 0x4000, CRC(357863e4) SHA1(3f82a98a8c12c2a306455a0926de500a54b06b3d) )
	ROM_LOAD( "pakrat_gfx_bank1_actn0_lo.bin", 0x04000, 0x4000, CRC(4efe7e31) SHA1(ee5bf301df2b2cae08197d6636ceb5a0efe0ec78) )
	ROM_LOAD( "pakrat_gfx_bank2_actn1_lo.bin", 0x08000, 0x4000, CRC(f0265530) SHA1(807b6f026ccf28e142fc9d2d0dec4052474965d4) )
	ROM_LOAD( "pakrat_gfx_bank3_pakall_lo.bin", 0x0c000, 0x4000, CRC(8ad92683) SHA1(7c751122bf7d0c37d105dfbfb248484156fbda44) )
	ROM_LOAD( "pakrat_gfx_bank4_tough_lo.bin", 0x10000, 0x4000, CRC(bd4d4421) SHA1(8b1d24b0a4654ac95bd8c7aa34570d3cfcdbef42) )
	ROM_LOAD( "pakrat_gfx_bank5_buldog_lo.bin", 0x14000, 0x4000, CRC(660733fb) SHA1(2cbf521ec1c92a6c967681c081cd99c516ca74b9) )
	ROM_LOAD( "pakrat_gfx_bank6_pakspi_lo.bin", 0x18000, 0x4000, CRC(1ddc4441) SHA1(62ce32aaa6559992fd48761b319963f89d62971b) )
	ROM_LOAD( "pakrat_gfx_bank7_packrat_lo.bin", 0x1c000, 0x4000, CRC(2c9163e5) SHA1(5c921483ffb6c6597970a0569115e039b2cba662) )
	ROM_LOAD( "pakrat_gfx_bank8_stooj_lo.bin", 0x20000, 0x4000, CRC(262c0f46) SHA1(4715b283967923f00b275339db59d4cdc1ed1627) )
	ROM_LOAD( "pakrat_gfx_bank9_obj_lo.bin", 0x24000, 0x4000, CRC(2fc468e6) SHA1(a250228ee0e718148627abff22b39764e5e63cf2) )
	ROM_LOAD( "pakrat_gfx_bank0_walk_hi.bin", 0x40000, 0x4000, CRC(5ddac992) SHA1(29a51eefd5025ae302ea7b58956fdcbc6364febc) )
	ROM_LOAD( "pakrat_gfx_bank1_actn0_hi.bin", 0x44000, 0x4000, CRC(91dbf695) SHA1(e88aa92c19a720536a7e10a4217fafa6ad51a952) )
	ROM_LOAD( "pakrat_gfx_bank2_actn1_hi.bin", 0x48000, 0x4000, CRC(a7863c21) SHA1(cb04aaf7c70fa7f5e1cd015f7a16c544ec863358) )
	ROM_LOAD( "pakrat_gfx_bank3_pakall_hi.bin", 0x4c000, 0x4000, CRC(e97c800a) SHA1(b4f7c286a9a13841e60b5137650db72c6476fb09) )
	ROM_LOAD( "pakrat_gfx_bank4_tough_hi.bin", 0x50000, 0x4000, CRC(7252b9a2) SHA1(f83d8bf1eb164954fc3d0c9b46302c196b96e6a2) )
	ROM_LOAD( "pakrat_gfx_bank5_buldog_hi.bin", 0x54000, 0x4000, CRC(16546ef0) SHA1(2bc9e8d18c0794bf19f92327da2e820056aa5f4d) )
	ROM_LOAD( "pakrat_gfx_bank6_pakspi_hi.bin", 0x58000, 0x4000, CRC(2a48381b) SHA1(1f0fb3645917113d73b0f82020943ba8de17c2ff) )
	ROM_LOAD( "pakrat_gfx_bank7_packrat_hi.bin", 0x5c000, 0x4000, CRC(fb302123) SHA1(7a3991df73d94160a139c182d07be47652489dcc) )
	ROM_LOAD( "pakrat_gfx_bank8_stooj_hi.bin", 0x60000, 0x4000, CRC(c0296f76) SHA1(9acec6fe873352f17a53846796f8d7057aceac78) )
	ROM_LOAD( "pakrat_gfx_bank9_obj_hi.bin", 0x64000, 0x4000, CRC(83544432) SHA1(575c318357afffbf4bbfe64ecf6470d9bb88b47a) )
ROM_END

} // anonymous namespace


GAME( 1984, peterpakt,  0,         peterpakt, peterpakt, peterpakt_state, empty_init, ROT0, "Atari, Inc.", "Peter Pack Rat (T11 prototype, Jun 01 1984)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )
GAME( 1984, peterpakta, peterpakt, peterpakt, peterpakt, peterpakt_state, empty_init, ROT0, "Atari, Inc.", "Peter Pack Rat (T11 prototype, May 11 1984)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )
GAME( 1984, peterpakts, peterpakt, peterpakt, peterpakt, peterpakt_state, empty_init, ROT0, "Atari, Inc.", "Peter Pack Rat (T11 prototype, rebuilt from source)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )
