// license:BSD-3-Clause
// copyright-holders:Zsolt Vasvari, Curt Coder

/***************************************************************************
Lovely Cards/Poker/Pontoon driver, updated by El Condor from work by Uki and
Zsolt Vasvari respectively.
Later cleaned up by Curt Coder and Scott Yellig.
---

Lovely Poker by Uki/El Condor
Enter switch test mode by pressing the Deal key twice when the
crosshatch pattern comes up.

After you get into Check Mode (F2), press the Deal key to switch pages.

Memory Mapped:

0000-5fff   R   ROM
6000-67ff   RW  Battery Backed RAM
9000-93ff   RW  Video RAM
9400-97ff   RW  Color RAM
                Bits 0-3 - color
                Bits 4-5 - character bank
                Bit  6   - flip x
                Bit  7   - Is it used?
a000        R   Input Port 0
a001        R   Input Port 1
a002        R   Input Port 2
a001        W  Control Port 0
a002        W  Control Port 1

I/O Ports:
00         RW  YM2149 Data  Port
               Port A - DSW #1
               Port B - DSW #2
01          W  YM2149 Control Port

---

Pontoon Memory Map (preliminary)

Pontoon driver by Zsolt Vasvari
Enter switch test mode by pressing the Hit key twice when the
crosshatch pattern comes up
After you get into Check Mode (F2), press the Hit key to switch pages.

Memory Mapped:

0000-5fff   R   ROM
6000-67ff   RW  Battery Backed RAM
8000-83ff   RW  Video RAM
8400-87ff   RW  Color RAM
                Bits 0-3 - color
                Bits 4-5 - character bank
                Bit  6   - flip x
                Bit  7   - Is it used?
a000        R   Input Port 0
a001        R   Input Port 1
a002        R   Input Port 2
a001        W   Control Port 0
a002        W   Control Port 1

I/O Ports:
00         RW  YM2149 Data Port
               Port A - DSW #1
               Port B - DSW #2
01          W  YM2149 Control Port

TODO:

- What do the control ports do? Payout?
- Input ports need to be cleaned up
- NVRAM does not work for lvcards? In lvcardsa it works

***************************************************************************/

#include "emu.h"


#include "cpu/z80/z80.h"
#include "machine/nvram.h"
#include "sound/ay8910.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"


namespace {

class lvcards_state : public driver_device
{
public:
	lvcards_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_videoram(*this, "videoram"),
		m_colorram(*this, "colorram"),
		m_gfxdecode(*this, "gfxdecode"),
		m_decrypted_opcodes(*this, "decrypted_opcodes")
	{ }

	void lvcards(machine_config &config);
	void lvcardsa(machine_config &config);

	void init_lvcardsa();

protected:
	virtual void video_start() override ATTR_COLD;

	void videoram_w(offs_t offset, uint8_t data);
	void colorram_w(offs_t offset, uint8_t data);

	required_device<cpu_device> m_maincpu;

private:
	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr<uint8_t> m_colorram;
	required_device<gfxdecode_device> m_gfxdecode;
	optional_shared_ptr<uint8_t> m_decrypted_opcodes;
	tilemap_t *m_bg_tilemap = nullptr;
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	void palette(palette_device &palette) const;
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void io_map(address_map &map) ATTR_COLD;
	void lvcards_map(address_map &map) ATTR_COLD;
	void lvcardsa_decrypted_opcodes_map(address_map &map) ATTR_COLD;
};


class lvpoker_state : public lvcards_state
{
public:
	lvpoker_state(const machine_config &mconfig, device_type type, const char *tag) :
		lvcards_state(mconfig, type, tag),
		m_in2(*this, "IN2")
	{ }

	void lvpoker(machine_config &config);
	void ponttehk(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	void control_port_2_w(uint8_t data);
	void control_port_2a_w(uint8_t data);
	uint8_t payout_r();

	void lvpoker_map(address_map &map) ATTR_COLD;
	void ponttehk_map(address_map &map) ATTR_COLD;

	required_ioport m_in2;

	uint8_t m_payout = 0U;
	uint8_t m_pulse = 0U;
	uint8_t m_result = 0U;
};


void lvcards_state::palette(palette_device &palette) const //Ever so slightly different, but different enough.
{
	const uint8_t *color_prom = memregion("proms")->base();

	for (int i = 0; i < palette.entries(); i++)
	{
		int bit0, bit1, bit2, bit3;

		// red component
		bit0 = (color_prom[0] >> 0) & 0x11;
		bit1 = (color_prom[0] >> 1) & 0x11;
		bit2 = (color_prom[0] >> 2) & 0x11;
		bit3 = (color_prom[0] >> 3) & 0x11;
		int const r = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;

		// green component
		bit0 = (color_prom[palette.entries()] >> 0) & 0x11;
		bit1 = (color_prom[palette.entries()] >> 1) & 0x11;
		bit2 = (color_prom[palette.entries()] >> 2) & 0x11;
		bit3 = (color_prom[palette.entries()] >> 3) & 0x11;
		int const g = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;

		// blue component
		bit0 = (color_prom[2 * palette.entries()] >> 0) & 0x11;
		bit1 = (color_prom[2 * palette.entries()] >> 1) & 0x11;
		bit2 = (color_prom[2 * palette.entries()] >> 2) & 0x11;
		bit3 = (color_prom[2 * palette.entries()] >> 3) & 0x11;
		int const b = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;

		palette.set_pen_color(i, rgb_t(r, g, b));

		color_prom++;
	}
}

void lvcards_state::videoram_w(offs_t offset, uint8_t data)
{
	m_videoram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);
}

void lvcards_state::colorram_w(offs_t offset, uint8_t data)
{
	m_colorram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);
}

TILE_GET_INFO_MEMBER(lvcards_state::get_bg_tile_info)
{
	int const attr = m_colorram[tile_index];
	int const code = m_videoram[tile_index] + ((attr & 0x30) << 4) + ((attr & 0x80) << 3);
	int const color = attr & 0x0f;
	int const flags = (attr & 0x40) ? TILE_FLIPX : 0;

	tileinfo.set(0, code, color, flags);
}

void lvcards_state::video_start()
{
	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(lvcards_state::get_bg_tile_info)), TILEMAP_SCAN_ROWS,
			8, 8, 32, 32);
}

uint32_t lvcards_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	return 0;
}


void lvpoker_state::machine_start()
{
	lvcards_state::machine_start();

	save_item(NAME(m_payout));
	save_item(NAME(m_pulse));
	save_item(NAME(m_result));
}

void lvpoker_state::machine_reset()
{
	lvcards_state::machine_reset();

	m_payout = 0;
	m_pulse = 0;
	m_result = 0;
}

void lvpoker_state::control_port_2_w(uint8_t data)
{
	switch (data)
	{
	case 0x60:
		m_payout = 1;
		break;
	case 0xc0:
		m_payout = 1;
		break;
	default:
		m_payout = 0;
		break;
	}
}

void lvpoker_state::control_port_2a_w(uint8_t data)
{
	switch (data)
	{
	case 0x60:
		m_payout = 1;
		break;
	case 0x80:
		m_payout = 1;
		break;
	default:
		m_payout = 0;
		break;
	}
}

uint8_t lvpoker_state::payout_r()
{
	m_result = m_in2->read();

	if (m_payout)
	{
		if (m_pulse < 3)
		{
			m_result = m_result | 0x40;
			m_pulse++;
		}
		else
		{
			m_pulse = 0;
		}
	}
	return m_result;
}

void lvpoker_state::ponttehk_map(address_map &map)
{
	map(0x0000, 0x5fff).rom();
	map(0x6000, 0x67ff).ram().share("nvram");
	map(0x8000, 0x83ff).ram().w(FUNC(lvpoker_state::videoram_w)).share("videoram");
	map(0x8400, 0x87ff).ram().w(FUNC(lvpoker_state::colorram_w)).share("colorram");
	map(0xa000, 0xa000).portr("IN0");
	map(0xa001, 0xa001).portr("IN1").nopw(); // lamps
	map(0xa002, 0xa002).r(FUNC(lvpoker_state::payout_r)).w(FUNC(lvpoker_state::control_port_2a_w)); //.nopw(); // ???
}

void lvcards_state::lvcards_map(address_map &map)
{
	map(0x0000, 0x5fff).rom();
	map(0x6000, 0x67ff).ram().share("nvram");
	map(0x9000, 0x93ff).ram().w(FUNC(lvcards_state::videoram_w)).share(m_videoram);
	map(0x9400, 0x97ff).ram().w(FUNC(lvcards_state::colorram_w)).share(m_colorram);
	map(0xa000, 0xa000).portr("IN0");
	map(0xa001, 0xa001).portr("IN1").nopw();
	map(0xa002, 0xa002).portr("IN2").nopw();
	map(0xc000, 0xdfff).rom();
}

void lvcards_state::io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x00).r("aysnd", FUNC(ay8910_device::data_r));
	map(0x00, 0x01).w("aysnd", FUNC(ay8910_device::data_address_w));
}

void lvcards_state::lvcardsa_decrypted_opcodes_map(address_map &map)
{
	map(0x0000, 0x5fff).rom().share(m_decrypted_opcodes);
	map(0xc000, 0xdfff).rom().region("maincpu", 0xc000);
}

void lvpoker_state::lvpoker_map(address_map &map)
{
	map(0x0000, 0x5fff).rom();
	map(0x6000, 0x67ff).ram().share("nvram");
	map(0x9000, 0x93ff).ram().w(FUNC(lvpoker_state::videoram_w)).share("videoram");
	map(0x9400, 0x97ff).ram().w(FUNC(lvpoker_state::colorram_w)).share("colorram");
	map(0xa000, 0xa000).portr("IN0");
	map(0xa001, 0xa001).portr("IN1").nopw(); // lamps
	map(0xa002, 0xa002).r(FUNC(lvpoker_state::payout_r)).w(FUNC(lvpoker_state::control_port_2_w));
	map(0xc000, 0xdfff).rom();
}

static INPUT_PORTS_START( lvcards )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("Analyzer")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_GAMBLE_HIGH ) PORT_NAME("Red")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_GAMBLE_LOW ) PORT_NAME("Black")

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_POKER_HOLD1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_POKER_HOLD2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_POKER_HOLD3 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_POKER_HOLD4 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_POKER_HOLD5 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_GAMBLE_DEAL ) PORT_NAME("Deal / Double")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_GAMBLE_BET )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_POKER_CANCEL ) PORT_NAME("Cancel / Take Score")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(3)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW0")
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
	PORT_DIPNAME( 0x80, 0x80, "Reset?" )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x07, 0x07, "1 COIN =" )
	PORT_DIPSETTING(    0x06, "5$" )
	PORT_DIPSETTING(    0x05, "10$" )
	PORT_DIPSETTING(    0x04, "15$" )
	PORT_DIPSETTING(    0x03, "20$" )
	PORT_DIPSETTING(    0x07, "25$" )
	PORT_DIPSETTING(    0x02, "50$" )
	PORT_DIPSETTING(    0x01, "75$" )
	PORT_DIPSETTING(    0x00, "100$" )
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
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( lvpoker )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("Analyzer") PORT_TOGGLE
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MEMORY_RESET ) PORT_CODE(KEYCODE_MINUS)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE2 ) PORT_NAME("Clear Stats")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_EQUALS) PORT_NAME("Hopper Reset")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_GAMBLE_HIGH ) PORT_NAME("Red")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_GAMBLE_LOW ) PORT_NAME("Black")

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_POKER_HOLD1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_POKER_HOLD2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_POKER_HOLD3 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_POKER_HOLD4 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_POKER_HOLD5 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_GAMBLE_DEAL ) PORT_NAME("Deal / Double")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_GAMBLE_BET )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_SERVICE3 ) PORT_NAME("Remove Credit as coins")

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_CUSTOM ) // Token Drop
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_POKER_CANCEL ) PORT_NAME("Cancel / Take Score")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(3)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_IMPULSE(3)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN3 ) PORT_IMPULSE(3)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_CUSTOM ) // Overflow
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_CUSTOM ) // Token Out
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_CUSTOM)

	PORT_START("DSW0")
	PORT_DIPNAME( 0x07, 0x07, "Winning Percentage" )
	PORT_DIPSETTING(    0x06, "70%" )
	PORT_DIPSETTING(    0x05, "74%" )
	PORT_DIPSETTING(    0x04, "78%" )
	PORT_DIPSETTING(    0x03, "82%" )
	PORT_DIPSETTING(    0x02, "86%" )
	PORT_DIPSETTING(    0x07, "90%" )
	PORT_DIPSETTING(    0x01, "94%" )
	PORT_DIPSETTING(    0x00, "98%" )
	PORT_DIPNAME( 0x08, 0x08, "Max. Payout Adjustment")
	PORT_DIPSETTING(    0x08, "Free")
	PORT_DIPSETTING(    0x00, "2000")
	PORT_DIPNAME( 0x10, 0x10, "Bonus Game Difficulty")
	PORT_DIPSETTING(    0x10, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hard ) )
	PORT_DIPNAME( 0x60, 0x20, "Payment Method" )
	PORT_DIPSETTING(    0x00, "Credit In/Coin Out")
	PORT_DIPSETTING(    0x20, "Credit In/Credit Out")
	PORT_DIPSETTING(    0x40, "Coin In/Coin Out")
	//PORT_DIPSETTING(    0x60, "Credit In/Coin Out") Again, clearly no Coin in, Credit out
	PORT_DIPNAME( 0x80, 0x80, "Memory Reset Switch" )
	PORT_DIPSETTING(    0x80, "Disabled" )
	PORT_DIPSETTING(    0x00, "Enabled" )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x07, 0x06, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 1C_6C ) )
	PORT_DIPNAME( 0x38, 0x30, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(    0x28, DEF_STR( 2C_2C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x38, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x18, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(    0x00, "1 Coin/10 Credits" )
	PORT_DIPNAME( 0x40, 0x40, "Coin C (Service Switch)" )
	PORT_DIPSETTING(    0x40, "1 Push/1 Credit" )
	PORT_DIPSETTING(    0x00, "1 Push/10 Credits" )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( ponttehk )
	PORT_START("IN0")
	PORT_SERVICE( 0x01, IP_ACTIVE_LOW )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE2 ) PORT_NAME("Reset All")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE3 ) PORT_NAME("Clear Stats")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_U)  PORT_NAME("Call Attendant")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_EQUALS) PORT_NAME("Hopper Reset")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("Bonus Game")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("Stand")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("Hit")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_NAME("Use Credit") PORT_CODE(KEYCODE_L)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_NAME("Remove Credit as coins") PORT_CODE(KEYCODE_A)

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_CUSTOM )// Token Drop
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_IMPULSE(3)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(3)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN3 ) PORT_IMPULSE(3)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_CUSTOM )   // Overflow optometric switch - reversed logic
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_3) PORT_NAME("Coinout Sensor") //Token Out
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_CUSTOM )//Motor On?

	PORT_START("DSW0")
	PORT_DIPNAME( 0x07, 0x07, "Winning Percentage" )
	PORT_DIPSETTING(    0x06, "70%" )
	PORT_DIPSETTING(    0x05, "74%" )
	PORT_DIPSETTING(    0x04, "78%" )
	PORT_DIPSETTING(    0x03, "82%" )
	PORT_DIPSETTING(    0x02, "86%" )
	PORT_DIPSETTING(    0x07, "90%" )
	PORT_DIPSETTING(    0x01, "94%" )
	PORT_DIPSETTING(    0x00, "98%" )
	PORT_BIT( 0x18, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_DIPNAME( 0x60, 0x20, "Payment Method" )
	PORT_DIPSETTING(    0x00, "Credit In/Coin Out" )
	PORT_DIPSETTING(    0x20, "Coin In/Coin Out" )
	PORT_DIPSETTING(    0x40, "Credit In/Credit Out" )
	//PORT_DIPSETTING(    0x60, "Credit In/Coin Out" ) Again, clearly no Coin in, Credit out
	PORT_DIPNAME( 0x80, 0x80, "Reset All Switch" )
	PORT_DIPSETTING(    0x80, "Disabled" )
	PORT_DIPSETTING(    0x00, "Enabled" )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x07, 0x06, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 1C_6C ) )
	PORT_DIPNAME( 0x38, 0x30, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(    0x28, DEF_STR( 2C_2C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x38, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x18, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(    0x00, "1 Coin/10 Credits" )
	PORT_DIPNAME( 0x40, 0x40, "Coin C (Service Switch)" )
	PORT_DIPSETTING(    0x40, "1 Push/1 Credit" )
	PORT_DIPSETTING(    0x00, "1 Push/10 Credits" )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )
INPUT_PORTS_END


// Graphics Decode Information

static GFXDECODE_START( gfx_lvcards )
	GFXDECODE_ENTRY( "tiles", 0, gfx_8x8x4_packed_lsb, 0, 16 )
GFXDECODE_END


void lvcards_state::lvcards(machine_config &config)
{
	// basic machine hardware
	Z80(config, m_maincpu, 18'432'000 / 4); // unknown frequency, assume same as tehkanwc.cpp
	m_maincpu->set_addrmap(AS_PROGRAM, &lvcards_state::lvcards_map);
	m_maincpu->set_addrmap(AS_IO, &lvcards_state::io_map);
	m_maincpu->set_vblank_int("screen", FUNC(lvcards_state::irq0_line_hold));

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500)); // not accurate
	screen.set_size(32*8, 32*8);
	screen.set_visarea(8*0, 32*8-1, 2*8, 30*8-1);
	screen.set_screen_update(FUNC(lvcards_state::screen_update));
	screen.set_palette("palette");

	GFXDECODE(config, m_gfxdecode, "palette", gfx_lvcards);
	PALETTE(config, "palette", FUNC(lvcards_state::palette), 256);

	// sound hardware
	SPEAKER(config, "mono").front_center();

	ay8910_device &aysnd(AY8910(config, "aysnd", 18'432'000 / 12)); // unknown frequency, assume same as tehkanwc.cpp
	aysnd.port_a_read_callback().set_ioport("DSW0");
	aysnd.port_b_read_callback().set_ioport("DSW1");
	aysnd.add_route(ALL_OUTPUTS, "mono", 0.25);
}

void lvcards_state::lvcardsa(machine_config &config)
{
	lvcards(config);

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_1);
	m_maincpu->set_addrmap(AS_OPCODES, &lvpoker_state::lvcardsa_decrypted_opcodes_map);
}

void lvpoker_state::lvpoker(machine_config &config)
{
	lvcards(config);

	// basic machine hardware
	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_1);
	m_maincpu->set_addrmap(AS_PROGRAM, &lvpoker_state::lvpoker_map);
}

void lvpoker_state::ponttehk(machine_config &config)
{
	lvcards(config);

	// basic machine hardware
	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_1);
	m_maincpu->set_addrmap(AS_PROGRAM, &lvpoker_state::ponttehk_map);

	// video hardware
	PALETTE(config.replace(), "palette", palette_device::RGB_444_PROMS, "proms", 256);
}

ROM_START( lvpoker )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "lp1.bin",      0x0000, 0x4000, CRC(ecfefa91) SHA1(7f6e0f30a2c4299797a8066419bf247b2900e312) )
	ROM_LOAD( "lp2.bin",      0x4000, 0x2000, CRC(06d5484f) SHA1(326756a03eaeefc944428c7e011fcdc128aa415a) )
	ROM_LOAD( "lp3.bin",      0xc000, 0x2000, CRC(05e17de8) SHA1(76b38e414f225789de8af9ca0556008e17285ffe) )

	ROM_REGION( 0x10000, "tiles", 0 )
	ROM_LOAD( "lp4.bin",      0x0000, 0x2000, CRC(04fd2a6b) SHA1(33fb42f54646dc91f5aca1c55cfc932fa04f5d77) )
	ROM_CONTINUE(             0x8000, 0x2000 )
	ROM_LOAD( "lp5.bin",      0x2000, 0x2000, CRC(9b5b531c) SHA1(1ce700361ea39a15c9c62fc0fa61df0cda62a340) )
	ROM_CONTINUE(             0xa000, 0x2000 )
	ROM_LOAD( "lc6.bin",      0x4000, 0x2000, CRC(2991a6ec) SHA1(b2c32550884b7b708db48bb7f0854bbad504417d) )
	ROM_RELOAD(               0xc000, 0x2000 )
	ROM_LOAD( "lp7.bin",      0x6000, 0x2000, CRC(217e9cfb) SHA1(3e7d76db5195c599c2bf186ae6616b29bc0fd337) )
	ROM_RELOAD(               0xe000, 0x2000 )

	ROM_REGION( 0x0300, "proms", 0 )
	ROM_LOAD( "3.7c",         0x0000, 0x0100, CRC(0c2ead4a) SHA1(e8e29e21366622c9bf3ee5e807c83b5cd7da8e3e) )
	ROM_LOAD( "2.7b",         0x0100, 0x0100, CRC(f4bc51e2) SHA1(38293c1117d6f3076081b6f2da3f42819558b04f) )
	ROM_LOAD( "1.7a",         0x0200, 0x0100, CRC(e40f2363) SHA1(cea598b6ed037dd3b4306c2ca3b0b4d5197d42a4) )
ROM_END

ROM_START( lvcards )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "lc1.bin", 0x0000, 0x4000, CRC(0c5fbf05) SHA1(bf996bdccfc5748cee91d004f2b1da10bcd8e329) )
	ROM_LOAD( "lc2.bin", 0x4000, 0x2000, CRC(deb54548) SHA1(a245898635c5cd3c26989c2bba89bb71edacd906) )
	ROM_LOAD( "lc3.bin", 0xc000, 0x2000, CRC(45c2bea9) SHA1(3a33501824769656aa87649c3fd0a8b8a4d83f3c) )

	ROM_REGION( 0x10000, "tiles", 0 )
	ROM_LOAD( "lc4.bin", 0x0000, 0x2000, CRC(dd705389) SHA1(271c11c2bd9affd976d65e318fd9fb01dbdde040) )
	ROM_CONTINUE(        0x8000, 0x2000 )
	ROM_LOAD( "lc5.bin", 0x2000, 0x2000, CRC(ddd1e3e5) SHA1(b7e8ccaab318b61b91eae4eee9e04606f9717037) )
	ROM_CONTINUE(        0xa000, 0x2000 )
	ROM_LOAD( "lc6.bin", 0x4000, 0x2000, CRC(2991a6ec) SHA1(b2c32550884b7b708db48bb7f0854bbad504417d) )
	ROM_RELOAD(          0xc000, 0x2000 )
	ROM_LOAD( "lc7.bin", 0x6000, 0x2000, CRC(f1b84c56) SHA1(6834139400bf8aa8db17f65bfdcbdcb2433d5fdc) )
	ROM_RELOAD(          0xe000, 0x2000 )

	ROM_REGION( 0x0300, "proms", 0 )
	ROM_LOAD( "3.7c", 0x0000,  0x0100, CRC(0c2ead4a) SHA1(e8e29e21366622c9bf3ee5e807c83b5cd7da8e3e) )
	ROM_LOAD( "2.7b", 0x0100,  0x0100, CRC(f4bc51e2) SHA1(38293c1117d6f3076081b6f2da3f42819558b04f) )
	ROM_LOAD( "1.7a", 0x0200,  0x0100, CRC(e40f2363) SHA1(cea598b6ed037dd3b4306c2ca3b0b4d5197d42a4) )
ROM_END

/*
>romcmp -d lvcards.zip lvcardsa.zip
10 and 10 files
3.7c                                            FIXED BITS (0000xxxx)
2.7b                                            FIXED BITS (0000xxxx)
1.7a                                            FIXED BITS (0000xxxx)
                        7052.a7                 FIXED BITS (0000xxxx)
                        7052.b7                 FIXED BITS (0000xxxx)
                        7052.c7                 FIXED BITS (0000xxxx)
lc4.bin                 4.f4                    IDENTICAL
lc6.bin                 6.c4                    IDENTICAL
lc7.bin                 7.a4                    IDENTICAL
lc5.bin      [1/2]      5.d4         [1/2]      IDENTICAL
3.7c         [1/2]      7052.c7      [1/2]      IDENTICAL
2.7b         [1/2]      7052.b7      [1/2]      IDENTICAL
1.7a         [1/2]      7052.a7      [1/2]      IDENTICAL
lc3.bin      [2/2]      3.h4         [2/2]      IDENTICAL
lc3.bin      [1/2]      3.h4         [1/2]      99.975586%
lc5.bin      [2/2]      5.d4         [2/2]      99.963379%
3.7c         [2/2]      7052.c7      [2/2]      96.875000%
1.7a         [2/2]      7052.a7      [2/2]      94.531250%
2.7b         [2/2]      7052.b7      [2/2]      93.750000%
lc2.bin      [even 1/2] 2.k4         [even 1/2] 15.966797%
lc1.bin      [1/4]      1.l4         [1/4]      10.009766%
lc1.bin      [3/4]      1.l4         [3/4]      9.814453%
lc2.bin      [odd 1/2]  2.k4         [even 2/2] 0.781250%
lc2.bin      [odd 2/2]  2.k4         [odd 1/2]  0.488281%
lc1.bin      [2/4]      1.l4         [4/4]      0.317383%
lc1.bin      [4/4]      1.l4         [2/4]      0.317383%
lc2.bin      [even 2/2] 2.k4         [odd 2/2]  0.146484%
*/
ROM_START( lvcardsa ) // Tehkan 5910A-1 PCB
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "1.l4", 0x0000, 0x4000, CRC(81a25f70) SHA1(ce8917674b3374e5e138420f062d9b7aba3f6e11) )
	ROM_LOAD( "2.k4", 0x4000, 0x2000, CRC(d1c72fc2) SHA1(496606e129046d253d716254ef4e58a93ca10aa0) )
	ROM_LOAD( "3.h4", 0xc000, 0x2000, CRC(45cb4b4f) SHA1(6eb5725c048efe729246c730d6576b19ee24eab7) )

	ROM_REGION( 0x10000, "tiles", 0 )
	ROM_LOAD( "4.f4", 0x0000, 0x2000, CRC(dd705389) SHA1(271c11c2bd9affd976d65e318fd9fb01dbdde040) )  // identical
	ROM_CONTINUE(        0x8000, 0x2000 )
	ROM_LOAD( "5.d4", 0x2000, 0x2000, CRC(60841508) SHA1(1da57c57ae01b8c93c32e6ffe7efd0852296eaf0) )  // 1st half: identical. 2nd half: 99.963379%
	ROM_CONTINUE(        0xa000, 0x2000 )
	ROM_LOAD( "6.c4", 0x4000, 0x2000, CRC(2991a6ec) SHA1(b2c32550884b7b708db48bb7f0854bbad504417d) )  // identical
	ROM_RELOAD(          0xc000, 0x2000 )
	ROM_LOAD( "7.a4", 0x6000, 0x2000, CRC(f1b84c56) SHA1(6834139400bf8aa8db17f65bfdcbdcb2433d5fdc) )  // identical
	ROM_RELOAD(          0xe000, 0x2000 )

	ROM_REGION( 0x0300, "proms", 0 )
	ROM_LOAD( "7052.c7", 0x0000,  0x0100, CRC(c8ddd9d8) SHA1(511a575b9a0c66819754146f72eab075bbc0a06e) )
	ROM_LOAD( "7052.b7", 0x0100,  0x0100, CRC(a7f37cb0) SHA1(236145e93af244a50ed66b9875ed019b1dde9027) )
	ROM_LOAD( "7052.a7", 0x0200,  0x0100, CRC(dcb970fd) SHA1(ce88f5f49dbcf187ac9344235b035f742c820336) )
ROM_END

ROM_START( ponttehk )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ponttehk.001", 0x0000, 0x4000, CRC(1f8c1b38) SHA1(3776ddd695741223bd9ad41f74187bff31f2cd3b) )
	ROM_LOAD( "ponttehk.002", 0x4000, 0x2000, CRC(befb4f48) SHA1(8ca146c8b52afab5deb6f0ff52bdbb2b1ff3ded7) )

	ROM_REGION( 0x8000, "tiles", 0 )
	ROM_LOAD( "ponttehk.003", 0x0000, 0x2000, CRC(a6a91b3d) SHA1(d180eabe67efd3fd1205570b661a74acf7ed93b3) )
	ROM_LOAD( "ponttehk.004", 0x2000, 0x2000, CRC(976ed924) SHA1(4d305694b3e157411068baf3052e3aac7d0b32d5) )
	ROM_LOAD( "ponttehk.005", 0x4000, 0x2000, CRC(2b8e8ca7) SHA1(dd86d3b4fd1627bdaa0603ffd2f1bc2953bc51f8) )
	ROM_LOAD( "ponttehk.006", 0x6000, 0x2000, CRC(6bc23965) SHA1(b73a584fc5b2dd9436bbb8bc1620f5a51d351aa8) )

	ROM_REGION( 0x0300, "proms", 0 )
	ROM_LOAD( "pon24s10.003", 0x0000, 0x0100, CRC(4623b7f3) SHA1(55948753dec09d0a476b90ca75e7e092ce0f68ee) )  /* red component */
	ROM_LOAD( "pon24s10.002", 0x0100, 0x0100, CRC(117e1b67) SHA1(b753137878fe5cd650722cf526cd4929821240a8) )  /* green component */
	ROM_LOAD( "pon24s10.001", 0x0200, 0x0100, CRC(c64ecee8) SHA1(80c9ec21e135235f7f2d41ce7900cf3904123823) )  /* blue component */
ROM_END

void lvcards_state::init_lvcardsa()
{
	uint8_t *rom = memregion("maincpu")->base();

	for (int i = 0; i < 0x6000; i++)
	{
		uint8_t x = rom[i];

		switch (i & 0x1111)
		{
			case 0x0000: x = bitswap<8>(x ^ 0xa8, 7, 6, 3, 4, 5, 2, 1, 0); break;
			case 0x0001: x = bitswap<8>(x ^ 0xa8, 3, 6, 5, 4, 7, 2, 1, 0); break;
			case 0x0010: x = bitswap<8>(x ^ 0x28, 5, 6, 3, 4, 7, 2, 1, 0); break;
			case 0x0011: x = bitswap<8>(x ^ 0xa8, 7, 6, 5, 4, 3, 2, 1, 0); break; // xor ok, no swap?
			case 0x0100: x = bitswap<8>(x ^ 0xa8, 3, 6, 5, 4, 7, 2, 1, 0); break;
			case 0x0101: x = bitswap<8>(x ^ 0xa8, 3, 6, 5, 4, 7, 2, 1, 0); break;
			case 0x0110: x = bitswap<8>(x ^ 0x28, 5, 6, 3, 4, 7, 2, 1, 0); break;
			case 0x0111: x = bitswap<8>(x ^ 0xa8, 7, 6, 3, 4, 5, 2, 1, 0); break;
			case 0x1000: x = bitswap<8>(x ^ 0xa8, 7, 6, 3, 4, 5, 2, 1, 0); break;
			case 0x1001: x = bitswap<8>(x ^ 0x28, 5, 6, 7, 4, 3, 2, 1, 0); break;
			case 0x1010: x = bitswap<8>(x ^ 0xa8, 3, 6, 5, 4, 7, 2, 1, 0); break;
			case 0x1011: x = bitswap<8>(x ^ 0xa8, 3, 6, 5, 4, 7, 2, 1, 0); break;
			case 0x1100: x = bitswap<8>(x ^ 0xa8, 3, 6, 5, 4, 7, 2, 1, 0); break;
			case 0x1101: x = bitswap<8>(x ^ 0xa8, 7, 6, 3, 4, 5, 2, 1, 0); break;
			case 0x1110: x = bitswap<8>(x ^ 0x28, 5, 6, 7, 4, 3, 2, 1, 0); break;
			case 0x1111: x = bitswap<8>(x ^ 0xa8, 3, 6, 5, 4, 7, 2, 1, 0); break;
		}

		m_decrypted_opcodes[i] = x;
	}

	for (int a = 0; a < 0x6000; a++)
	{
		uint8_t x = rom[a];

		switch(a & 0x1111)
		{
			case 0x0000: x = bitswap<8>(x ^ 0xa8, 3, 6, 5, 4, 7, 2, 1, 0); break;
			case 0x0001: x = bitswap<8>(x ^ 0xa8, 3, 6, 5, 4, 7, 2, 1, 0); break;
			case 0x0010: x = bitswap<8>(x ^ 0x28, 5, 6, 3, 4, 7, 2, 1, 0); break;
			case 0x0011: x = bitswap<8>(x ^ 0xa8, 7, 6, 5, 4, 3, 2, 1, 0); break; // xor ok, no swap?
			case 0x0100: x = bitswap<8>(x ^ 0xa8, 3, 6, 7, 4, 5, 2, 1, 0); break;
			case 0x0101: x = bitswap<8>(x ^ 0xa8, 3, 6, 5, 4, 7, 2, 1, 0); break;
			case 0x0110: x = bitswap<8>(x ^ 0x28, 5, 6, 3, 4, 7, 2, 1, 0); break;
			case 0x0111: x = bitswap<8>(x ^ 0xa8, 7, 6, 3, 4, 5, 2, 1, 0); break;
			case 0x1000: x = bitswap<8>(x ^ 0xa8, 3, 6, 5, 4, 7, 2, 1, 0); break;
			case 0x1001: x = bitswap<8>(x ^ 0x28, 5, 6, 7, 4, 3, 2, 1, 0); break;
			case 0x1010: x = bitswap<8>(x ^ 0x28, 5, 6, 7, 4, 3, 2, 1, 0); break;
			case 0x1011: x = bitswap<8>(x ^ 0xa8, 3, 6, 5, 4, 7, 2, 1, 0); break;
			case 0x1100: x = bitswap<8>(x ^ 0xa8, 7, 6, 3, 4, 5, 2, 1, 0); break;
			case 0x1101: x = bitswap<8>(x ^ 0x28, 5, 6, 7, 4, 3, 2, 1, 0); break;
			case 0x1110: x = bitswap<8>(x ^ 0xa8, 3, 6, 5, 4, 7, 2, 1, 0); break;
			case 0x1111: x = bitswap<8>(x ^ 0xa8, 7, 6, 3, 4, 5, 2, 1, 0); break;
		}

		rom[a] = x;
	}
}

} // anonymous namespace


GAME( 1985, lvcards,  0,       lvcards,  lvcards,  lvcards_state, empty_init,    ROT0, "Tehkan", "Lovely Cards",             MACHINE_SUPPORTS_SAVE )
GAME( 1985, lvcardsa, lvcards, lvcardsa, lvcards,  lvcards_state, init_lvcardsa, ROT0, "Tehkan", "Lovely Cards (encrypted)", MACHINE_SUPPORTS_SAVE )
GAME( 1985, lvpoker,  lvcards, lvpoker,  lvpoker,  lvpoker_state, empty_init,    ROT0, "Tehkan", "Lovely Poker",             MACHINE_SUPPORTS_SAVE )
GAME( 1985, ponttehk, 0,       ponttehk, ponttehk, lvpoker_state, empty_init,    ROT0, "Tehkan", "Pontoon (Tehkan)",         MACHINE_SUPPORTS_SAVE )
