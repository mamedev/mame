// license:BSD-3-Clause
// copyright-holders: Angelo Salese, Roberto Fresca
/***************************************************************************

    Double Crown (c) 1997 Cadence Technology / Dyna

    Driver by Angelo Salese
    Additional work by Roberto Fresca.

    TODO:
    - Bogus "Hole" in main screen display;
    - Is the background pen really black?
    - Lots of unmapped I/Os (game doesn't make much use of the HW);
    - video / irq timings;

    Notes:
    - at POST the SW tries to write to the palette RAM in a banking fashion.
      I think it's just an HW left-over.
    - there are various bogus checks to ROM region throughout the whole SW
      (0x0030-0x0033? O.o), trying to change the values of these ones changes
      the functionality of the game, almost like that the DSWs are tied to
      these ...

============================================================================

    Excellent System
    boardlabel: ES-9411B

    28.6363 xtal
    ES-9409 QFP is 208 pins.. for graphics only?
    Z0840006PSC Zilog z80, is rated 6.17 MHz
    OKI M82C55A-2
    65764H-5 .. 64kbit ram CMOS
    2 * N341256P-25 - CMOS SRAM 256K-BIT(32KX8)
    4 * dipsw 8pos
    YMZ284-D (ay8910, but without i/o ports)
    MAXIM MAX693ACPE is a "Microprocessor Supervisory Circuit", for watchdog
    and for nvram functions.

***************************************************************************/


#define MAIN_CLOCK          XTAL(28'636'363)
#define CPU_CLOCK           MAIN_CLOCK / 6
#define SND_CLOCK           MAIN_CLOCK / 12

#include "emu.h"
#include "cpu/z80/z80.h"
#include "sound/ay8910.h"
#include "machine/i8255.h"
#include "machine/nvram.h"
#include "machine/timer.h"
#include "machine/watchdog.h"
#include "emupal.h"
#include "screen.h"
#include "speaker.h"

#include "dblcrown.lh"
#define DEBUG_VRAM

class dblcrown_state : public driver_device
{
public:
	dblcrown_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_watchdog(*this, "watchdog")
		, m_gfxdecode(*this, "gfxdecode")
		, m_palette(*this, "palette")
		, m_inputs(*this, "IN%u", 0U)
		, m_lamps(*this, "lamp%u", 0U)
	{ }

	void dblcrown(machine_config &config);

private:
	// driver_device overrides
	virtual void machine_start() override;
	virtual void machine_reset() override;

	virtual void video_start() override;

	// screen updates
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	DECLARE_WRITE8_MEMBER(bank_w);
	DECLARE_READ8_MEMBER(irq_source_r);
	DECLARE_WRITE8_MEMBER(irq_source_w);
	DECLARE_READ8_MEMBER(palette_r);
	DECLARE_WRITE8_MEMBER(palette_w);
	DECLARE_READ8_MEMBER(vram_r);
	DECLARE_WRITE8_MEMBER(vram_w);
	DECLARE_READ8_MEMBER(vram_bank_r);
	DECLARE_WRITE8_MEMBER(vram_bank_w);
	DECLARE_WRITE8_MEMBER(mux_w);
	DECLARE_READ8_MEMBER(in_mux_r);
	DECLARE_READ8_MEMBER(in_mux_type_r);
	DECLARE_WRITE8_MEMBER(output_w);
	DECLARE_WRITE8_MEMBER(lamps_w);
	DECLARE_WRITE8_MEMBER(watchdog_w);

	TIMER_DEVICE_CALLBACK_MEMBER(dblcrown_irq_scanline);
	void dblcrown_palette(palette_device &palette) const;

	void dblcrown_io(address_map &map);
	void dblcrown_map(address_map &map);

	// devices
	required_device<cpu_device> m_maincpu;
	required_device<watchdog_timer_device> m_watchdog;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_ioport_array<4> m_inputs;
	output_finder<8> m_lamps;

	uint8_t m_bank;
	uint8_t m_irq_src;
	std::unique_ptr<uint8_t[]> m_pal_ram;
	std::unique_ptr<uint8_t[]> m_vram;
	uint8_t m_vram_bank[2];
	uint8_t m_mux_data;
};

void dblcrown_state::video_start()
{
	m_pal_ram = std::make_unique<uint8_t[]>(0x200 * 2);
	m_vram = std::make_unique<uint8_t[]>(0x1000 * 0x10);

	save_pointer(NAME(m_vram), 0x1000 * 0x10);
}

uint32_t dblcrown_state::screen_update( screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect )
{
	gfx_element *gfx = m_gfxdecode->gfx(0);
	gfx_element *gfx_2 = m_gfxdecode->gfx(1);
	int x,y;
	int count;

	count = 0xa000;

	for (y = 0; y < 16; y++)
	{
		for (x = 0; x < 32; x++)
		{
			uint16_t tile = ((m_vram[count]) | (m_vram[count+1] << 8)) & 0xfff;
			uint8_t col = (m_vram[count+1] >> 4);

			gfx_2->opaque(bitmap, cliprect, tile, col, 0, 0, x * 16, y * 16);

			count += 2;
		}
	}

	count = 0xb000;

	for (y = 0; y < 32; y++)
	{
		for (x = 0; x < 64; x++)
		{
			uint16_t tile = ((m_vram[count]) | (m_vram[count + 1] << 8)) & 0xfff;
			uint8_t col = (m_vram[count + 1] >> 4); // ok?

			gfx->transpen(bitmap, cliprect, tile, col, 0, 0, x * 8, y * 8, 0);

			count += 2;
		}
	}

	return 0;
}

WRITE8_MEMBER( dblcrown_state::bank_w)
{
	m_bank = data;
	membank("rom_bank")->set_entry(m_bank & 0x1f);
}

READ8_MEMBER( dblcrown_state::irq_source_r)
{
	return m_irq_src;
}

WRITE8_MEMBER( dblcrown_state::irq_source_w)
{
	m_irq_src = data; // this effectively acks the irq, by writing 0
}

READ8_MEMBER( dblcrown_state::palette_r)
{
	//if(m_bank & 8) /* TODO: verify this */
	//  offset+=0x200;

	return m_pal_ram[offset];
}

WRITE8_MEMBER( dblcrown_state::palette_w)
{
	int r,g,b,datax;

	//if(m_bank & 8) /* TODO: verify this */
	//  offset+=0x200;

	m_pal_ram[offset] = data;
	offset >>= 1;
	datax = m_pal_ram[offset * 2] + 256 * m_pal_ram[offset * 2 + 1];

	r = ((datax) & 0x000f) >> 0;
	g = ((datax) & 0x00f0) >> 4;
	b = ((datax) & 0x0f00) >> 8;
	/* TODO: remaining bits */

	m_palette->set_pen_color(offset, pal4bit(r), pal4bit(g), pal4bit(b));
}


READ8_MEMBER( dblcrown_state::vram_r)
{
	uint32_t hi_offs;
	hi_offs = m_vram_bank[(offset & 0x1000) >> 12] << 12;

	return m_vram[(offset & 0xfff) | hi_offs];
}

WRITE8_MEMBER( dblcrown_state::vram_w)
{
	uint32_t hi_offs;
	hi_offs = m_vram_bank[(offset & 0x1000) >> 12] << 12;

	m_vram[(offset & 0xfff) | hi_offs] = data;

	#ifdef DEBUG_VRAM
	{
		uint8_t *VRAM = memregion("vram")->base();

		VRAM[(offset & 0xfff) | hi_offs] = data;
		m_gfxdecode->gfx(0)->mark_dirty(((offset & 0xfff) | hi_offs) / 32);
	}
	#endif
}

READ8_MEMBER( dblcrown_state::vram_bank_r)
{
	return m_vram_bank[offset];
}

WRITE8_MEMBER( dblcrown_state::vram_bank_w)
{
	m_vram_bank[offset] = data & 0xf;

	if(data & 0xf0)
		printf("vram bank = %02x\n",data);
}

WRITE8_MEMBER( dblcrown_state::mux_w)
{
	m_mux_data = data;
}

READ8_MEMBER( dblcrown_state::in_mux_r )
{
	int i;
	uint8_t res;

	res = 0;

	for(i = 0; i < 4; i++)
	{
		if(m_mux_data & 1 << i)
			res |= m_inputs[i]->read();
	}

	return res;
}

READ8_MEMBER( dblcrown_state::in_mux_type_r )
{
	int i;
	uint8_t res;

	res = 0xff;

	for(i = 0; i < 4; i++)
	{
		if (m_inputs[i]->read() != 0xff)
			res &= ~(1 << i);
	}

	return res;
}

WRITE8_MEMBER( dblcrown_state::output_w )
{
/*  bits
  7654 3210
  ---- -x--  unknown (active after deal)
  ---- x---  Payout counter pulse
  ---x ----  Coin In counter pulse
  -x-- ----  unknown (active after deal)
  x-x- --xx  unknown
*/

	machine().bookkeeping().coin_counter_w(0, data & 0x10);  /* Coin In counter pulse */
	machine().bookkeeping().coin_counter_w(1 ,data & 0x08);  /* Payout counter pulse */
//  popmessage("out: %02x", data);
}


WRITE8_MEMBER( dblcrown_state::lamps_w )
{
/*  bits
  7654 3210
  ---- ---x  Deal
  ---- --x-  Bet
  ---- -x--  Cancel
  ---- x---  Hold 5
  ---x ----  Hold 4
  --x- ----  Hold 3
  -x-- ----  Hold 2
  x--- ----  Hold 1
*/

	for (int n = 0; n < 8; n++)
		m_lamps[n] = BIT(data, n);
}

WRITE8_MEMBER(dblcrown_state::watchdog_w)
/*
  Always 0x01...
*/
{
	if (data & 0x01)      /* check for refresh value (0x01) */
	{
		m_watchdog->watchdog_reset();
	}
	else
	{
		popmessage("Watchdog: %02x", data);
	}
}


void dblcrown_state::dblcrown_map(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0x9fff).bankr("rom_bank");
	map(0xa000, 0xb7ff).ram(); // work ram
	map(0xb800, 0xbfff).ram().share("nvram");
	map(0xc000, 0xdfff).rw(FUNC(dblcrown_state::vram_r), FUNC(dblcrown_state::vram_w));
	map(0xf000, 0xf1ff).rw(FUNC(dblcrown_state::palette_r), FUNC(dblcrown_state::palette_w));
	map(0xfe00, 0xfeff).ram(); // ???
	map(0xff00, 0xffff).ram(); // ???, intentional fall-through
	map(0xff00, 0xff01).rw(FUNC(dblcrown_state::vram_bank_r), FUNC(dblcrown_state::vram_bank_w));
	map(0xff04, 0xff04).rw(FUNC(dblcrown_state::irq_source_r), FUNC(dblcrown_state::irq_source_w));

}

void dblcrown_state::dblcrown_io(address_map &map)
{
	map.global_mask(0xff);
	map.unmap_value_high();
	map(0x00, 0x00).portr("DSWA");
	map(0x01, 0x01).portr("DSWB");
	map(0x02, 0x02).portr("DSWC");
	map(0x03, 0x03).portr("DSWD");
	map(0x04, 0x04).r(FUNC(dblcrown_state::in_mux_r));
	map(0x05, 0x05).r(FUNC(dblcrown_state::in_mux_type_r));
	map(0x10, 0x13).rw("ppi", FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0x20, 0x21).w("ymz", FUNC(ymz284_device::address_data_w));
	map(0x30, 0x30).w(FUNC(dblcrown_state::watchdog_w));
	map(0x40, 0x40).w(FUNC(dblcrown_state::output_w));
}

static INPUT_PORTS_START( dblcrown )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MEMORY_RESET )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_SERVICE2 ) PORT_NAME("Credit Reset")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_GAMBLE_KEYIN ) PORT_NAME("Note")
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_GAMBLE_HIGH ) PORT_NAME("Big")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_GAMBLE_LOW ) PORT_NAME("Small")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_GAMBLE_TAKE )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_GAMBLE_KEYOUT ) PORT_NAME("Payout")
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_POKER_HOLD1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_POKER_HOLD2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_POKER_HOLD3 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_POKER_HOLD4 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_POKER_HOLD5 )
	PORT_BIT( 0xe0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_POKER_CANCEL ) PORT_NAME("Cancel / Repeat Bet")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_GAMBLE_DEAL ) PORT_NAME("Deal / Draw")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_POKER_BET )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_GAMBLE_D_UP )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SERVICE3 ) PORT_NAME("Analyzer")
	PORT_BIT( 0xe0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("DSWA")
	PORT_DIPNAME( 0x01, 0x01, "DSWA" )
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
	PORT_DIPNAME( 0x20, 0x20, "Hold Type" )
	PORT_DIPSETTING(    0x20, "Hold" )
	PORT_DIPSETTING(    0x00, "Discard" )
	PORT_DIPNAME( 0x40, 0x40, "Input Test" )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSWB")
	PORT_DIPNAME( 0x01, 0x01, "DSWB" )
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

	PORT_START("DSWC")
	PORT_DIPNAME( 0x01, 0x01, "DSWC" )
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

	PORT_START("DSWD")
	PORT_DIPNAME( 0x01, 0x01, "DSWD" )
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
INPUT_PORTS_END

static const gfx_layout char_8x8_layout =
{
	8,8,
	RGN_FRAC(1,1),
	4,
	{ 0,1,2,3 },
	{ 4,0, 12,8, 20,16, 28,24 },
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32 },
	32*8
};

static const gfx_layout char_16x16_layout =
{
	16,16,
	RGN_FRAC(1,1),
	4,
	{ 0,1,2,3 },
	{ 4,0, 12,8, 20,16, 28,24, 36,32, 44,40, 52,48, 60,56 },
	{ STEP16(0,8*8) },
	8*8*16
};


static GFXDECODE_START( gfx_dblcrown )
#ifdef DEBUG_VRAM
	GFXDECODE_ENTRY( "vram", 0, char_8x8_layout, 0, 0x10 )
#endif
	GFXDECODE_ENTRY( "gfx1", 0, char_16x16_layout, 0, 0x10 )
GFXDECODE_END



void dblcrown_state::machine_start()
{
	uint8_t *ROM = memregion("maincpu")->base();
	membank("rom_bank")->configure_entries(0, 0x20, &ROM[0], 0x2000);

	m_lamps.resolve();
}

void dblcrown_state::machine_reset()
{
}


void dblcrown_state::dblcrown_palette(palette_device &palette) const
{
}

TIMER_DEVICE_CALLBACK_MEMBER(dblcrown_state::dblcrown_irq_scanline)
{
	int scanline = param;

	if (scanline == 256)
	{
		m_maincpu->set_input_line(0, HOLD_LINE);
		m_irq_src = 2;
	}
	else if ((scanline % 4) == 0) /* TODO: proper timing of this ... */
	{
/*
This is the main loop of this irq source. They hooked a timer irq then polled inputs via this wacky routine.
It needs at least 64 instances because 0xa05b will be eventually nuked by the vblank irq sub-routine.

043B: pop  af
043C: push af
043D: ld   a,($A05B)
0440: cp   $00
0442: jr   z,$0463
0444: cp   $10
0446: jr   z,$046D
0448: cp   $20
044A: jr   z,$047F
044C: cp   $30
044E: jr   z,$0491
0450: cp   $40
0452: jr   z,$04AB
0454: ld   a,($A05B)
0457: inc  a
0458: ld   ($A05B),a
045B: xor  a
045C: ld   ($FF04),a
045F: pop  af
0460: ei
0461: reti
*/
		m_maincpu->set_input_line(0, HOLD_LINE);
		m_irq_src = 4;
	}
}


MACHINE_CONFIG_START(dblcrown_state::dblcrown)

	/* basic machine hardware */
	MCFG_DEVICE_ADD("maincpu", Z80, CPU_CLOCK)
	MCFG_DEVICE_PROGRAM_MAP(dblcrown_map)
	MCFG_DEVICE_IO_MAP(dblcrown_io)
	TIMER(config, "scantimer").configure_scanline(FUNC(dblcrown_state::dblcrown_irq_scanline), "screen", 0, 1);

	WATCHDOG_TIMER(config, m_watchdog).set_time(attotime::from_msec(1000));   /* 1000 ms. (minimal of MAX693A watchdog long timeout period with internal oscillator) */

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500))
	MCFG_SCREEN_UPDATE_DRIVER(dblcrown_state, screen_update)
	MCFG_SCREEN_SIZE(64*8, 64*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 40*8-1, 2*8, 30*8-1)
	MCFG_SCREEN_PALETTE(m_palette)

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_dblcrown);

	PALETTE(config, m_palette, FUNC(dblcrown_state::dblcrown_palette), 0x100);

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	i8255_device &ppi(I8255(config, "ppi"));
	ppi.out_pa_callback().set(FUNC(dblcrown_state::lamps_w));
	ppi.out_pb_callback().set(FUNC(dblcrown_state::bank_w));
	ppi.out_pc_callback().set(FUNC(dblcrown_state::mux_w));

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	YMZ284(config, "ymz", SND_CLOCK).add_route(ALL_OUTPUTS, "mono", 0.75);
MACHINE_CONFIG_END


/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( dblcrown )
	ROM_REGION( 0x40000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD("1.u33", 0x00000, 0x40000, CRC(5df95a9c) SHA1(799333206089989c25ff9f167363073d4cf64bd2) )
//  ROM_FILL( 0x0030, 4, 0xff )

	ROM_REGION( 0x80000, "gfx1", ROMREGION_ERASE00 )
	ROM_LOAD("2.u43", 0x00000, 0x80000, CRC(58200bd4) SHA1(2795cfc41056111f66bfb82916343d1c733baa83) )

#ifdef DEBUG_VRAM
	ROM_REGION( 0x1000*0x10, "vram", ROMREGION_ERASE00 )
#endif

	ROM_REGION( 0x0bf1, "plds", 0 )
	ROM_LOAD("palce16v8h.u39", 0x0000, 0x0117, CRC(c74231ee) SHA1(f1b9e98f1fde53eee64d5da38fb8a6c22b6333e2) )
ROM_END


/*     YEAR  NAME      PARENT  MACHINE   INPUT     CLASS           INIT        ROT    COMPANY                FULLNAME                FLAGS                    LAYOUT  */
GAMEL( 1997, dblcrown, 0,      dblcrown, dblcrown, dblcrown_state, empty_init, ROT0, "Cadence Technology",  "Double Crown (v1.0.3)", MACHINE_IMPERFECT_GRAPHICS, layout_dblcrown ) // 1997 DYNA copyright in tile GFX
