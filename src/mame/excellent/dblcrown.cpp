// license:BSD-3-Clause
// copyright-holders: Angelo Salese, Roberto Fresca
/**************************************************************************************************

Double Crown (c) 1997 Cadence Technology / Dyna

TODO:
- Is the background pen really black?
- Pinpoint optional hopper line_r hookup, via dip4:8.
- Lots of unmapped I/Os (game doesn't make much use of the HW);
- video / irq timings;

Notes:
- at POST the SW tries to write to the palette RAM in a banking fashion. HW left-over?
- there are various $0030-$0033 ROM checks across the SW, changing these values to non-zero
  effectively changes game functionality (cfr. matrix mode at POST), ROM overlay or just
  different ROM versions?
- Topmost 2 rows are intended to be seen, cfr. analyzer "5 of a kind" and "royal flush" drawn there.
  Whatever these rows are supposed to indicate in gameplay (scoring cards for a fever bonus?)
  is untested.

===================================================================================================

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

**************************************************************************************************/


#define MAIN_CLOCK          XTAL(28'636'363)
#define CPU_CLOCK           MAIN_CLOCK / 6
#define SND_CLOCK           MAIN_CLOCK / 12

#include "emu.h"
#include "cpu/z80/z80.h"
#include "machine/bankdev.h"
#include "machine/i8255.h"
#include "machine/nvram.h"
#include "machine/ticket.h"
#include "machine/timer.h"
#include "machine/watchdog.h"
#include "sound/ay8910.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"

#include "dblcrown.lh"


namespace {

class dblcrown_state : public driver_device
{
public:
	dblcrown_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_watchdog(*this, "watchdog")
		, m_hopper(*this, "hopper")
		, m_vram(*this, "vram")
		, m_vram_bank(*this, "vram_bank%u", 0U)
		, m_gfxdecode(*this, "gfxdecode")
		, m_palette(*this, "palette")
		, m_inputs(*this, "IN%u", 0U)
		, m_lamps(*this, "lamp%u", 0U)
	{ }

	void dblcrown(machine_config &config);

private:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	virtual void video_start() override ATTR_COLD;

	required_device<cpu_device> m_maincpu;
	required_device<watchdog_timer_device> m_watchdog;
	required_device<ticket_dispenser_device> m_hopper;
	required_shared_ptr<u8> m_vram;
	required_device_array<address_map_bank_device, 2> m_vram_bank;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_ioport_array<4> m_inputs;
	output_finder<8> m_lamps;

	void bank_w(uint8_t data);
	uint8_t irq_source_r();
	void irq_source_w(uint8_t data);
	uint8_t palette_r(offs_t offset);
	void palette_w(offs_t offset, uint8_t data);
	uint8_t vram_r(offs_t offset);
	void vram_w(offs_t offset, uint8_t data);
	uint8_t vram_bank_r(offs_t offset);
	void vram_bank_w(offs_t offset, uint8_t data);
	void key_select_w(uint8_t data);
	uint8_t key_matrix_r();
	uint8_t key_pending_r();
	void output_w(uint8_t data);
	void lamps_w(uint8_t data);
	void watchdog_w(uint8_t data);

	TIMER_DEVICE_CALLBACK_MEMBER(scanline_cb);

	void main_map(address_map &map) ATTR_COLD;
	void main_io(address_map &map) ATTR_COLD;
	void vram_map(address_map &map) ATTR_COLD;

	uint8_t m_bank = 0;
	uint8_t m_irq_src = 0;
	uint8_t m_key_select = 0;

	std::unique_ptr<uint8_t[]> m_pal_ram;
	uint8_t m_vram_bank_entry[2]{};
	tilemap_t *m_bg_tilemap = nullptr;
	tilemap_t *m_fg_tilemap = nullptr;
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	TILE_GET_INFO_MEMBER(get_fg_tile_info);

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
};

TILE_GET_INFO_MEMBER(dblcrown_state::get_bg_tile_info)
{
	const u8 *rambase = (const u8 *)tilemap.user_data();
	const u16 code = (rambase[tile_index * 2 + 0] | (rambase[tile_index * 2 + 1] << 8)) & 0xfff;
	const u8 color = (rambase[tile_index * 2 + 1] >> 4);

	tileinfo.set(0, code, color, 0);
}

TILE_GET_INFO_MEMBER(dblcrown_state::get_fg_tile_info)
{
	const u8 *rambase = (const u8 *)tilemap.user_data();
	const u16 code = (rambase[tile_index * 2 + 0] | (rambase[tile_index * 2 + 1] << 8)) & 0x7ff;
	const u8 color = (rambase[tile_index * 2 + 1] >> 4);

	tileinfo.set(1, code, color, 0);
}

void dblcrown_state::video_start()
{
	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(dblcrown_state::get_bg_tile_info)), TILEMAP_SCAN_ROWS, 16, 16, 32, 16);
	m_bg_tilemap->set_user_data(&m_vram[0xa000]);

	m_fg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(dblcrown_state::get_fg_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 64, 32);
	m_fg_tilemap->set_user_data(&m_vram[0xb000]);

	m_fg_tilemap->set_transparent_pen(0);

	m_pal_ram = std::make_unique<uint8_t[]>(0x200 * 2);
	// NOTE: set_source alone will crash with 0-length gfxdecoding
	// need to explicitly use a fn otherwise unused by the rest of
	// the MAME ecosystem at the time of this writing.
	m_gfxdecode->gfx(1)->set_source_and_total(m_vram, m_vram.length() / 32);
}

uint32_t dblcrown_state::screen_update( screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect )
{
	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	m_fg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	return 0;
}

void dblcrown_state::bank_w(uint8_t data)
{
	m_bank = data;
	membank("rom_bank")->set_entry(m_bank & 0x1f);
}

uint8_t dblcrown_state::irq_source_r()
{
	return m_irq_src;
}

void dblcrown_state::irq_source_w(uint8_t data)
{
	m_irq_src = data; // this effectively acks the irq, by writing 0
}

uint8_t dblcrown_state::palette_r(offs_t offset)
{
	//if(m_bank & 8) /* TODO: verify this */
	//  offset+=0x200;

	return m_pal_ram[offset];
}

void dblcrown_state::palette_w(offs_t offset, uint8_t data)
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

void dblcrown_state::vram_w(offs_t offset, uint8_t data)
{
	m_vram[offset] = data;

	if ((offset & 0xf000) == 0xa000)
		m_bg_tilemap->mark_tile_dirty((offset & 0xfff) >> 1);

	if ((offset & 0xf000) == 0xb000)
		m_fg_tilemap->mark_tile_dirty((offset & 0xfff) >> 1);

	m_gfxdecode->gfx(1)->mark_dirty(offset / 32);
}

uint8_t dblcrown_state::vram_bank_r(offs_t offset)
{
	return m_vram_bank_entry[offset];
}

void dblcrown_state::vram_bank_w(offs_t offset, uint8_t data)
{
	m_vram_bank_entry[offset] = data & 0xf;
	m_vram_bank[offset]->set_bank(m_vram_bank_entry[offset]);

	if(data & 0xf0)
		logerror("Upper vram bank write = %02x\n",data);
}

void dblcrown_state::key_select_w(uint8_t data)
{
	m_key_select = data;
}

uint8_t dblcrown_state::key_matrix_r()
{
	uint8_t res = 0;

	for(int i = 0; i < 4; i++)
	{
		if(m_key_select & 1 << i)
			res |= m_inputs[i]->read();
	}

	return res;
}

uint8_t dblcrown_state::key_pending_r()
{
	uint8_t res = 0xff;

	for(int i = 0; i < 4; i++)
	{
		if (m_inputs[i]->read() != 0xff)
			res &= ~(1 << i);
	}

	return res;
}

/*  bits
 * 7654 3210
 * ---- -x--  coin lockout
 * ---- x---  Payout counter pulse
 * ---x ----  Coin In counter pulse
 * -x-- ----  unknown (active after deal)
 * x-x- --xx  unknown
 */
void dblcrown_state::output_w(uint8_t data)
{
	machine().bookkeeping().coin_counter_w(0, BIT(data, 4));
	m_hopper->motor_w(BIT(data, 3));
	machine().bookkeeping().coin_lockout_global_w(!BIT(data, 2));
}

/*  bits
 * 7654 3210
 * ---- ---x  Deal
 * ---- --x-  Bet
 * ---- -x--  Cancel
 * ---- x---  Hold 5
 * ---x ----  Hold 4
 * --x- ----  Hold 3
 * -x-- ----  Hold 2
 * x--- ----  Hold 1
 */
void dblcrown_state::lamps_w(uint8_t data)
{
	for (int n = 0; n < 8; n++)
		m_lamps[n] = BIT(data, n);
}

// MAX693A
void dblcrown_state::watchdog_w(uint8_t data)
{
	// check for refresh value (0x01)
	if (data & 0x01)
	{
		m_watchdog->watchdog_reset();
	}
	else
	{
		popmessage("Watchdog: %02x", data);
	}
}


void dblcrown_state::main_map(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0x9fff).bankr("rom_bank");
	map(0xa000, 0xb7ff).ram(); // work ram
	map(0xb800, 0xbfff).ram().share("nvram");
	map(0xc000, 0xcfff).m(m_vram_bank[0], FUNC(address_map_bank_device::amap8));
	map(0xd000, 0xdfff).m(m_vram_bank[1], FUNC(address_map_bank_device::amap8));
	map(0xf000, 0xf1ff).rw(FUNC(dblcrown_state::palette_r), FUNC(dblcrown_state::palette_w));
	map(0xfe00, 0xfeff).ram(); // ???
	map(0xff00, 0xffff).ram(); // ???, intentional fall-through
	map(0xff00, 0xff01).rw(FUNC(dblcrown_state::vram_bank_r), FUNC(dblcrown_state::vram_bank_w));
	map(0xff04, 0xff04).rw(FUNC(dblcrown_state::irq_source_r), FUNC(dblcrown_state::irq_source_w));
}

void dblcrown_state::main_io(address_map &map)
{
	map.global_mask(0xff);
	map.unmap_value_high();
	map(0x00, 0x00).portr("DSWA");
	map(0x01, 0x01).portr("DSWB");
	map(0x02, 0x02).portr("DSWC");
	map(0x03, 0x03).portr("DSWD");
	map(0x04, 0x04).r(FUNC(dblcrown_state::key_matrix_r));
	map(0x05, 0x05).r(FUNC(dblcrown_state::key_pending_r));
	map(0x10, 0x13).rw("ppi", FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0x20, 0x21).w("ymz", FUNC(ymz284_device::address_data_w));
	map(0x30, 0x30).w(FUNC(dblcrown_state::watchdog_w));
	map(0x40, 0x40).w(FUNC(dblcrown_state::output_w));
}

void dblcrown_state::vram_map(address_map &map)
{
	map(0x0000, 0xffff).ram().w(FUNC(dblcrown_state::vram_w)).share("vram");
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
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_GAMBLE_PAYOUT )
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
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_GAMBLE_BET )
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
	PORT_DIPNAME( 0x40, 0x40, "Card Reveal Animation" )
	PORT_DIPSETTING(    0x40, "Normal" )
	PORT_DIPSETTING(    0x00, "Fast" )
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
	PORT_DIPNAME( 0xe0, 0xe0, "Credit Limit" )
	PORT_DIPSETTING(    0xe0, "1000" )
	PORT_DIPSETTING(    0xc0, "3000")
	PORT_DIPSETTING(    0xa0, "5000" )
	PORT_DIPSETTING(    0x80, "10000" )
	PORT_DIPSETTING(    0x60, "20000" )
	PORT_DIPSETTING(    0x40, "30000" )
	PORT_DIPSETTING(    0x20, "40000" )
	PORT_DIPSETTING(    0x00, "50000" )

	PORT_START("DSWD")
	PORT_DIPNAME( 0x0f, 0x0f, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(    0x00, "10 Coins/1 Credit" )
	PORT_DIPSETTING(    0x02, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x01, "1 Coin/1 Credit (again 1)" )
	PORT_DIPSETTING(    0x07, "1 Coin/1 Credit (again 2)" )
	PORT_DIPSETTING(    0x0f, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x0e, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x0d, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x0b, "1 Coin/10 Credits" )
	PORT_DIPSETTING(    0x0a, "1 Coin/20 Credits" )
	PORT_DIPSETTING(    0x09, "1 Coin/25 Credits" )
	PORT_DIPSETTING(    0x08, "1 Coin/50 Credits" )
	PORT_DIPNAME( 0x70, 0x70, DEF_STR( Coin_B ) ) // Coinage for note in
	PORT_DIPSETTING(    0x70, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x60, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x50, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x40, "1 Coin/10 Credits" )
	PORT_DIPSETTING(    0x30, "1 Coin/25 Credits" )
	PORT_DIPSETTING(    0x20, "1 Coin/50 Credits" )
	PORT_DIPSETTING(    0x10, "1 Coin/100 Credits" )
	PORT_DIPSETTING(    0x00, "1 Coin/500 Credits" )
	// TODO: game will error blink if On at payout time
	PORT_DIPNAME( 0x80, 0x80, "Hopper Status?" )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

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
	GFXDECODE_ENTRY( "gfx1", 0, char_16x16_layout, 0, 0x10 )
	GFXDECODE_ENTRY( nullptr, 0, gfx_8x8x4_packed_lsb, 0, 0x10 )
GFXDECODE_END



void dblcrown_state::machine_start()
{
	uint8_t *ROM = memregion("maincpu")->base();
	membank("rom_bank")->configure_entries(0, 0x20, &ROM[0], 0x2000);

	m_lamps.resolve();
}

void dblcrown_state::machine_reset()
{
	m_vram_bank[0]->set_bank(0);
	m_vram_bank[1]->set_bank(0);
}


TIMER_DEVICE_CALLBACK_MEMBER(dblcrown_state::scanline_cb)
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


void dblcrown_state::dblcrown(machine_config &config)
{
	Z80(config, m_maincpu, CPU_CLOCK);
	m_maincpu->set_addrmap(AS_PROGRAM, &dblcrown_state::main_map);
	m_maincpu->set_addrmap(AS_IO, &dblcrown_state::main_io);
	TIMER(config, "scantimer").configure_scanline(FUNC(dblcrown_state::scanline_cb), "screen", 0, 1);

	// 1000 ms. (minimal of MAX693A watchdog long timeout period with internal oscillator)
	WATCHDOG_TIMER(config, m_watchdog).set_time(attotime::from_msec(1000));

	HOPPER(config, m_hopper, attotime::from_msec(50));

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	i8255_device &ppi(I8255(config, "ppi"));
	ppi.out_pa_callback().set(FUNC(dblcrown_state::lamps_w));
	ppi.out_pb_callback().set(FUNC(dblcrown_state::bank_w));
	ppi.out_pc_callback().set(FUNC(dblcrown_state::key_select_w));

	for (auto bank : m_vram_bank)
		ADDRESS_MAP_BANK(config, bank).set_map(&dblcrown_state::vram_map).set_options(ENDIANNESS_LITTLE, 8, 16, 0x1000);

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500));
	screen.set_screen_update(FUNC(dblcrown_state::screen_update));
	screen.set_size(64*8, 64*8);
	screen.set_visarea(0*8, 40*8-1, 0*8, 30*8-1);
	screen.set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_dblcrown);

	PALETTE(config, m_palette).set_entries(0x100);

	SPEAKER(config, "mono").front_center();
	YMZ284(config, "ymz", SND_CLOCK).add_route(ALL_OUTPUTS, "mono", 0.75);
}


/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( dblcrown )
	ROM_REGION( 0x40000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD("1.u33", 0x00000, 0x40000, CRC(5df95a9c) SHA1(799333206089989c25ff9f167363073d4cf64bd2) )
//  ROM_FILL( 0x0030, 4, 0xff )

	ROM_REGION( 0x80000, "gfx1", ROMREGION_ERASE00 )
	ROM_LOAD("2.u43", 0x00000, 0x80000, CRC(58200bd4) SHA1(2795cfc41056111f66bfb82916343d1c733baa83) )

	ROM_REGION( 0x0bf1, "plds", 0 )
	ROM_LOAD("palce16v8h.u39", 0x0000, 0x0117, CRC(c74231ee) SHA1(f1b9e98f1fde53eee64d5da38fb8a6c22b6333e2) )
ROM_END

} // anonymous namespace


GAMEL( 1997, dblcrown, 0,      dblcrown, dblcrown, dblcrown_state, empty_init, ROT0, "Cadence Technology",  "Double Crown (v1.0.3)", MACHINE_IMPERFECT_GRAPHICS, layout_dblcrown ) // 1997 DYNA copyright in tile GFX
