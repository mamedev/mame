// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
/***************************************************************************

Hexion (GX122) (c) 1992 Konami

driver by Nicola Salmoria

Notes:
- The board has a 052591, which is used for protection in Thunder Cross and
  S.P.Y. In this game, however, the only thing it seems to do is clear the
  screen.

This is the 052591 PMC code loaded at startup, it contains a RAM/VRAM filling program.
See https://github.com/furrtek/SiliconRE/tree/master/Konami/052591 for details

00: 5f 80 01 e0 08  Entry point, set OUT0 high
01: df 80 00 e0 0c  r0 = 0
02: df 90 02 e0 0c  r1 = 2
03: df a0 03 e0 0c  r2 = 3
04: df b0 0f e0 0c  r3 = f
05: df c0 ff bf 0c  ExtAddr = 1fff, r4 = ffff
06: 5c 02 00 33 0c
07: 5f 80 04 80 0c  Write 2 to RAM (1fff) m_bankctrl, select pmcram
08: 5c 0e 00 2b 0c
09: df 70 00 cb 08  r7 = RAM(4)
0a: 5f 80 00 80 0c  ExtAddr = 0
0b: 5c 04 00 2b 0c
0c: df 60 00 cb 08  r6 = RAM(0) (commands 0, 1 and 30 are used)
0d: 5c 0c 1f e9 0c  JP 1F if r6 == 0
0e: 4c 0c 2d e9 08  JP 2D if r6 == 1

Command anything other than 00 or 01: Set bank to r7, then clear 16 bytes starting from r5.w
0f: 5f 80 03 80 0c  ExtAddr = 3
10: 5c 04 00 2b 0c
11: 5f 00 00 cb 00  Read MSB from RAM[3]
12: 5f 80 02 a0 0c  ExtAddr = 2
13: df d0 00 c0 04  r5.w = RAM[3], RAM[2]
14: 01 3a 00 f3 0a  acc = r5 + r3 = r5 + f
15: 5c 08 00 b3 0c
16: 5c 0e 00 13 0c  Write 3 to RAM[1fff] m_bankctrl
17: 5f 80 00 a0 0c
18: 5c 00 00 13 0c  Write r7 to RAM[0]
19: 5c 08 00 b3 0c
1a: 5c 00 00 13 0c  Write 0 to RAM[1fff] m_bankctrl, select vram
1b: 84 5a 00 b3 0c
1c: 48 0a 5b d1 0c  Write 0 to RAM[r5++] until r5 > acc (16 times)
1d: 5f 80 00 e0 08  Set OUT0 low
1e: 5f 00 1e fd 0c  JP 1E, infinite loop

Command is 00: Set bank to 0 and fill from 0 to 0x1fff with r2.b
1f: 5f 80 01 a0 0c
20: df 20 00 cb 08  r2 = RAM[1]
21: 5c 08 00 b3 0c
22: 5f 80 03 00 0c  Write 3 to RAM[1fff] m_bankctrl
23: 5c 08 00 b3 0c
24: 5f 80 00 80 0c  Write 3 to RAM[1fff] m_bankctrl
25: 5c 00 00 33 0c
26: 5c 08 00 93 0c  Write 0 to RAM[0]
27: 9f 91 ff cf 0e  Write 0 to RAM[1fff] m_bankctrl, select vram, r1 = fff << 1 = 1ffe
28: 5c 84 00 20 0c
29: 84 00 00 b3 0c  ExtAddr = r0
2a: 49 10 69 d1 0c  Write r2 to RAM[r0++] while r0 < r1
2b: 5f 80 00 e0 08  Set OUT0 low
2c: 5f 00 2c fd 0c  JP 2C, infinite loop

Command is 01: Set banks to 1 and fill from 0 to 0x1fff with r2.b
2d: 5f 80 01 a0 0c
2e: df 20 00 cb 08  r2 = RAM(1)
2f: 5c 08 00 b3 0c
30: 5f 80 03 00 0c  Write 3 to RAM[1fff] m_bankctrl
31: 5c 00 00 b3 0c
32: 5f 80 01 00 0c  Write 3 to RAM[0]
33: 5c 08 00 b3 0c
34: 5f 80 00 80 0c  Write 1 to RAM[1fff] m_bankctrl
35: 5c 00 00 33 0c
36: 5c 08 00 93 0c  Write 0 to RAM[0]
37: 9f 91 ff cf 0e  Write 0 to RAM[1fff] m_bankctrl, select vram, r1 = fff << 1 = 1ffe
38: 5c 84 00 20 0c
39: 84 00 00 b3 0c
3a: 49 10 79 d1 0c  Write r2 to RAM[r0++] while r0 < r1
3b: 5f 80 00 e0 08  Set OUT0 low
3c: 5f 00 3c fd 0c  JP 3C, infinite loop

3d: ff ff ff ff ff  Garbage
3e: ff ff ff ff ff  Garbage
3f: ff ff ff ff ff  Garbage

***************************************************************************/

#include "emu.h"

#include "konamipt.h"

#include "cpu/z80/z80.h"
#include "machine/k053252.h"
#include "machine/timer.h"
#include "machine/watchdog.h"
#include "sound/k051649.h"
#include "sound/okim6295.h"

#include "emupal.h"
#include "speaker.h"
#include "tilemap.h"

#include "multibyte.h"


// configurable logging
#define LOG_UNKWRITE     (1U << 1)
#define LOG_BANKEDRAM    (1U << 2)
#define LOG_CCU          (1U << 3)

//#define VERBOSE (LOG_GENERAL | LOG_UNKWRITE | LOG_BANKEDRAM | LOG_CCU)

#include "logmacro.h"

#define LOGUNKWRITE(...)     LOGMASKED(LOG_UNKWRITE,     __VA_ARGS__)
#define LOGBANKEDRAM(...)    LOGMASKED(LOG_BANKEDRAM,    __VA_ARGS__)
#define LOGCCU(...)          LOGMASKED(LOG_CCU,          __VA_ARGS__)


namespace {

class hexion_state : public driver_device
{
public:
	hexion_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_k053252(*this, "k053252"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_vram(*this, "vram%u", 0U, 0x2000U, ENDIANNESS_LITTLE),
		m_pmcram(*this, "pmcram", 0x800, ENDIANNESS_LITTLE),    // Might be an unused area of VRAM
		m_rombank(*this, "rombank"),
		m_tilesrom(*this, "tiles")
	{ }

	void hexion(machine_config &config);
	void hexionb(machine_config &config);

protected:
	virtual void video_start() override ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;
	required_device<k053252_device> m_k053252;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	memory_share_array_creator<uint8_t, 2> m_vram;
	memory_share_creator<uint8_t> m_pmcram;
	required_memory_bank m_rombank;
	required_region_ptr<uint8_t> m_tilesrom;

	uint8_t m_bankctrl = 0;
	uint8_t m_rambank = 0;
	uint8_t m_pmcbank = 0;
	uint8_t m_gfxrom_select = 0;
	uint8_t m_ccu_int_time = 0;
	int16_t m_ccu_int_time_count = 0;
	tilemap_t *m_bg_tilemap[2]{};

	void coincntr_w(uint8_t data);
	void bankswitch_w(uint8_t data);
	uint8_t bankedram_r(offs_t offset);
	void bankedram_w(offs_t offset, uint8_t data);
	void bankctrl_w(uint8_t data);
	void gfxrom_select_w(uint8_t data);
	void irq_ack_w(int state);
	void nmi_ack_w(int state);
	void ccu_int_time_w(uint8_t data);

	template <uint8_t Which> TILE_GET_INFO_MEMBER(get_tile_info);

	TIMER_DEVICE_CALLBACK_MEMBER(scanline);

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void base_map(address_map &map) ATTR_COLD;
	void hexion_map(address_map &map) ATTR_COLD;
	void hexionb_map(address_map &map) ATTR_COLD;
};



/***************************************************************************

  Callbacks for the TileMap code

***************************************************************************/

template <uint8_t Which>
TILE_GET_INFO_MEMBER(hexion_state::get_tile_info)
{
	tile_index *= 4;
	tileinfo.set(0,
			m_vram[Which][tile_index] + ((m_vram[Which][tile_index + 1] & 0x3f) << 8),
			m_vram[Which][tile_index + 2] & 0x0f,
			0);
}


/***************************************************************************

  Start the video hardware emulation.

***************************************************************************/

void hexion_state::video_start()
{
	m_bg_tilemap[0] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(hexion_state::get_tile_info<0>)), TILEMAP_SCAN_ROWS, 8, 8, 64, 32);
	m_bg_tilemap[1] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(hexion_state::get_tile_info<1>)), TILEMAP_SCAN_ROWS, 8, 8, 64, 32);

	m_bg_tilemap[0]->set_transparent_pen(0);
	m_bg_tilemap[1]->set_scrollx(0, -4);
	m_bg_tilemap[1]->set_scrolly(0, 4);

	m_rombank->configure_entries(0, 16, memregion("maincpu")->base(), 0x2000);

	m_gfxrom_select = 0;

	save_item(NAME(m_bankctrl));
	save_item(NAME(m_rambank));
	save_item(NAME(m_pmcbank));
	save_item(NAME(m_gfxrom_select));
	save_item(NAME(m_ccu_int_time));
	save_item(NAME(m_ccu_int_time_count));
}



/***************************************************************************

  Memory handlers

***************************************************************************/

void hexion_state::bankswitch_w(uint8_t data)
{
	// bits 0-3 select ROM bank
	m_rombank->set_entry(data & 0x0f);

	// bit 6 triggers the 052591
	if (BIT(data, 6))
	{
		const uint8_t command = m_pmcram[0];
		if (command <= 1)
		{
			memset(m_vram[command], m_pmcram[1], 0x2000);
			m_bg_tilemap[command]->mark_all_dirty();
		}
		else
		{
			const uint8_t bank = m_pmcram[4] & 1;
			const unsigned offset = get_u16le(&m_pmcram[2]);
			for (int i = 0; 16 > i; ++i)
				m_vram[bank][(offset + i) & 0x1fff] = 0;
			for (unsigned i = offset & ~3U; (offset + 16) > i; i += 4)
				m_bg_tilemap[bank]->mark_tile_dirty((i >> 2) & 0x07ff);
		}
	}

	// bit 7 = PMC-BK
	m_pmcbank = BIT(data, 7);

	// other bits unknown
	if (data & 0x30)
		LOGUNKWRITE("bankswitch %02x", data & 0xf0);
}

uint8_t hexion_state::bankedram_r(offs_t offset)
{
	if (m_gfxrom_select && offset < 0x1000)
	{
		return m_tilesrom[((m_gfxrom_select & 0x7f) << 12) + offset];
	}
	else if (m_bankctrl == 0)
	{
		return m_vram[m_rambank][offset];
	}
	else if (m_bankctrl == 2 && offset < 0x800)
	{
		return m_pmcram[offset];
	}
	else
	{
		LOGBANKEDRAM("%s: bankedram_r offset %04x, bankctrl = %02x\n", m_maincpu->pc(), offset, m_bankctrl);
		return 0;
	}
}

void hexion_state::bankedram_w(offs_t offset, uint8_t data)
{
	if (m_bankctrl == 3 && offset == 0 && (data & 0xfe) == 0)
	{
		LOGBANKEDRAM("%s: bankedram_w offset %04x, data %02x, bankctrl = %02x\n", m_maincpu->pc(), offset, data, m_bankctrl);
		m_rambank = data & 1;
	}
	else if (m_bankctrl == 0)
	{
		if (m_pmcbank)
		{
			LOGBANKEDRAM("%s: bankedram_w offset %04x, data %02x, bankctrl = %02x\n", m_maincpu->pc(), offset, data, m_bankctrl);
			m_vram[m_rambank][offset] = data;
			m_bg_tilemap[m_rambank]->mark_tile_dirty(offset >> 2);
		}
		else
			LOGBANKEDRAM("%04x pmc internal ram %04x = %02x\n", m_maincpu->pc(), offset, data);
	}
	else if (m_bankctrl == 2 && offset < 0x800)
	{
		if (m_pmcbank)
		{
			LOGBANKEDRAM("%s: pmcram_w offset %04x, data %02x, bankctrl = %02x\n", m_maincpu->pc(), offset, data, m_bankctrl);
			m_pmcram[offset] = data;
		}
		else
			LOGBANKEDRAM("%04x pmc internal ram %04x = %02x\n", m_maincpu->pc(), offset, data);
	}
	else
		LOGBANKEDRAM("%s: bankedram_w offset %04x, data %02x, bankctrl = %02x\n", m_maincpu->pc(), offset, data, m_bankctrl);
}

void hexion_state::bankctrl_w(uint8_t data)
{
	LOGBANKEDRAM("%s: bankctrl_w %02x\n", m_maincpu->pc(), data);
	m_bankctrl = data;
}

void hexion_state::gfxrom_select_w(uint8_t data)
{
	LOGBANKEDRAM("%s: gfxrom_select_w %02x\n", m_maincpu->pc(), data);
	m_gfxrom_select = data;
}



/***************************************************************************

  Display refresh

***************************************************************************/

uint32_t hexion_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_bg_tilemap[1]->draw(screen, bitmap, cliprect, 0, 0);
	m_bg_tilemap[0]->draw(screen, bitmap, cliprect, 0, 0);
	return 0;
}


void hexion_state::coincntr_w(uint8_t data)
{
	// bits 0/1 = coin counters
	machine().bookkeeping().coin_counter_w(0, data & 0x01);
	machine().bookkeeping().coin_counter_w(1, data & 0x02);

	// bit 5 = flip screen
	flip_screen_set(data & 0x20);

	// other bits unknown
	if ((data & 0xdc) != 0x10) LOGUNKWRITE("coincntr %02x", data);
}

void hexion_state::irq_ack_w(int state)
{
	m_maincpu->set_input_line(0, CLEAR_LINE);
}

void hexion_state::nmi_ack_w(int state)
{
	m_maincpu->set_input_line(INPUT_LINE_NMI, CLEAR_LINE);
}

void hexion_state::ccu_int_time_w(uint8_t data)
{
	LOGCCU("ccu_int_time rewritten with value of %02x\n", data);
	m_ccu_int_time = data;
}

void hexion_state::base_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0x9fff).bankr(m_rombank);
	map(0xa000, 0xbfff).ram();
	map(0xc000, 0xdffe).rw(FUNC(hexion_state::bankedram_r), FUNC(hexion_state::bankedram_w));
	map(0xdfff, 0xdfff).w(FUNC(hexion_state::bankctrl_w));
	map(0xe000, 0xe000).noprw();
	map(0xf000, 0xf00f).rw(m_k053252, FUNC(k053252_device::read), FUNC(k053252_device::write));
	map(0xf200, 0xf200).w("oki", FUNC(okim6295_device::write));
	map(0xf400, 0xf400).portr("DSW1");
	map(0xf401, 0xf401).portr("DSW2");
	map(0xf402, 0xf402).portr("P1");
	map(0xf403, 0xf403).portr("P2");
	map(0xf440, 0xf440).portr("DSW3");
	map(0xf441, 0xf441).portr("SYSTEM");
	map(0xf480, 0xf480).w(FUNC(hexion_state::bankswitch_w));
	map(0xf4c0, 0xf4c0).w(FUNC(hexion_state::coincntr_w));
	map(0xf500, 0xf500).w(FUNC(hexion_state::gfxrom_select_w));
	map(0xf540, 0xf540).r("watchdog", FUNC(watchdog_timer_device::reset_r));
}

void hexion_state::hexion_map(address_map &map)
{
	base_map(map);

	map(0xe800, 0xe8ff).m("k051649", FUNC(k051649_device::scc_map));
}

void hexion_state::hexionb_map(address_map &map)
{
	base_map(map);

	map(0xe800, 0xe87f).noprw(); // all the code to use the k051649 is still present
	map(0xe880, 0xe889).noprw(); // but the bootleg has an additional M6295 @ 0xf5c0 instead
	map(0xe88a, 0xe88e).noprw();
	map(0xe88f, 0xe88f).noprw();
	map(0xe8e0, 0xe8ff).noprw();
	map(0xf5c0, 0xf5c0).w("oki2", FUNC(okim6295_device::write));
}

static INPUT_PORTS_START( hexion )
	PORT_START("DSW1")
	KONAMI_COINAGE_LOC(DEF_STR( Free_Play ), DEF_STR( Free_Play ), SW1)

	PORT_START("DSW2")
	PORT_BIT( 0x0f, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_DIPNAME( 0x70, 0x40, DEF_STR( Difficulty ) )       PORT_DIPLOCATION("SW2:5,6,7")
	PORT_DIPSETTING(    0x70, DEF_STR( Easiest ) )          // "1"
	PORT_DIPSETTING(    0x60, DEF_STR( Very_Easy) )         // "2"
	PORT_DIPSETTING(    0x50, DEF_STR( Easy ) )             // "3"
	PORT_DIPSETTING(    0x40, DEF_STR( Medium ) )           // "4"
	PORT_DIPSETTING(    0x30, DEF_STR( Medium_Hard ) )      // "5"
	PORT_DIPSETTING(    0x20, DEF_STR( Hard ) )             // "6"
	PORT_DIPSETTING(    0x10, DEF_STR( Very_Hard ) )        // "7"
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )          // "8"
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Demo_Sounds ) )      PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW3")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Flip_Screen ) )      PORT_DIPLOCATION("SW3:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0xfa, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_SERVICE_DIPLOC( 0x04, IP_ACTIVE_LOW, "SW3:3" )

	PORT_START("P1")
	KONAMI8_B12_START(1)

	PORT_START("P2")
	KONAMI8_B12_START(2)

	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_CUSTOM )   // 052591? game waits for it to be 0
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END



static const gfx_layout charlayout =
{
	8,8,
	RGN_FRAC(1,2),
	4,
	{ 0, 1, 2, 3 },
	{ RGN_FRAC(1,2)+0*4, RGN_FRAC(1,2)+1*4, 0*4, 1*4, RGN_FRAC(1,2)+2*4, RGN_FRAC(1,2)+3*4, 2*4, 3*4 },
	{ 0*16, 1*16, 2*16, 3*16, 4*16, 5*16, 6*16, 7*16 },
	16*8
};

static GFXDECODE_START( gfx_hexion )
	GFXDECODE_ENTRY( "tiles", 0, charlayout, 0, 16 )
GFXDECODE_END

TIMER_DEVICE_CALLBACK_MEMBER(hexion_state::scanline)
{
	int scanline = param;

	// z80 /IRQ is connected to the IRQ1(vblank) pin of k053252 CCU
	if(scanline == 256)
		m_maincpu->set_input_line(0, ASSERT_LINE);

	// z80 /NMI is connected to the IRQ2 pin of k053252 CCU
	// the following code is emulating INT_TIME of the k053252, this code will go away
	// when the new konami branch is merged.
	m_ccu_int_time_count--;
	if (m_ccu_int_time_count <= 0)
	{
		m_maincpu->set_input_line(INPUT_LINE_NMI, ASSERT_LINE);
		m_ccu_int_time_count = m_ccu_int_time;
	}
}

void hexion_state::hexion(machine_config &config)
{
	// basic machine hardware
	Z80(config, m_maincpu, XTAL(24'000'000) / 4); // Z80B 6 MHz @ 17F, xtal verified, divider not verified
	m_maincpu->set_addrmap(AS_PROGRAM, &hexion_state::hexion_map);
	TIMER(config, "scantimer").configure_scanline(FUNC(hexion_state::scanline), "screen", 0, 1);
	WATCHDOG_TIMER(config, "watchdog");

	K053252(config, m_k053252, XTAL(24'000'000 )/ 2); // K053252, X0-010(?) @8D, xtal verified, divider not verified
	m_k053252->int1_ack().set(FUNC(hexion_state::irq_ack_w));
	m_k053252->int2_ack().set(FUNC(hexion_state::nmi_ack_w));
	m_k053252->int_time().set(FUNC(hexion_state::ccu_int_time_w));

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(64*8, 36*8);
	screen.set_visarea(0*8, 64*8-1, 0*8, 32*8-1);
	screen.set_screen_update(FUNC(hexion_state::screen_update));
	screen.set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_hexion);
	PALETTE(config, "palette", palette_device::RGB_444_PROMS, "proms", 256);

	// sound hardware
	SPEAKER(config, "mono").front_center();

	// MSM6295GS @ 5E, clock frequency & pin 7 not verified
	OKIM6295(config, "oki", 1056000, okim6295_device::PIN7_HIGH).add_route(ALL_OUTPUTS, "mono", 0.5);

	// KONAMI 051649 // 2212P003 // JAPAN 8910EAJ @ 1D, xtal verified, divider not verified
	K051649(config, "k051649", XTAL(24'000'000) / 8).add_route(ALL_OUTPUTS, "mono", 0.5);
}

void hexion_state::hexionb(machine_config &config)
{
	hexion(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &hexion_state::hexionb_map);

	config.device_remove("k051649");

	// clock frequency & pin 7 not verified; this clock and pin 7 being low makes the pitch match the non-bootleg version, so is probably correct
	OKIM6295(config, "oki2", 1056000, okim6295_device::PIN7_LOW).add_route(ALL_OUTPUTS, "mono", 0.5);
}


/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( hexion )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "122__j_a__b01.16f", 0x00000, 0x20000, CRC(eabc6dd1) SHA1(e74c1f1f2fcf8973f0741a2d544f25c8639448bf) ) // "122 // J A // B01" @16F (27c010?)

	ROM_REGION( 0x80000, "tiles", 0 )    // addressable by the main CPU
	ROM_LOAD( "122a07.1h",   0x00000, 0x40000, CRC(22ae55e3) SHA1(41bdc990f69416b639542e2186a3610c16389063) ) // Later PCBs have mask ROMs labeled: "KONAMI // 055066 // 122A07 // 233505" @1H (maybe mismarked 2H on PCB?)
	ROM_LOAD( "122a06.1g",   0x40000, 0x40000, CRC(438f4388) SHA1(9e23805c9642a237daeaf106187d1e1e0692434d) ) // Later PCBs have mask ROMs labeled: "KONAMI // 055065 // 122A06 // 233506" @1G

	ROM_REGION( 0x40000, "oki", 0 )
	ROM_LOAD( "122a05.2f",   0x0000, 0x40000, CRC(bcc831bf) SHA1(c3382065dd0069a4dc0bde2d9931ec85b0bffc73) ) // Later PCBs have mask ROMs labeled: "KONAMI // 055064 // 122A05 // 233507" @2F (maybe 2G? marking isn't visible in the picture I have)

	ROM_REGION( 0x0300, "proms", 0 )
	ROM_LOAD( "122a04.10b",   0x0000, 0x0100, CRC(506eb8c6) SHA1(3bff7cf286942d8bdbc3998245c3de20981fbecb) ) // AMD27S21 == 82S129
	ROM_LOAD( "122a03.11b",   0x0100, 0x0100, CRC(590c4f64) SHA1(db4b34f8c5fdfea034a94d65873f6fb842f123e9) ) // AMD27S21 == 82S129
	ROM_LOAD( "122a02.13b",   0x0200, 0x0100, CRC(5734305c) SHA1(c72e59acf79a4db1a5a9d827eef899c0675336f2) ) // AMD27S21 == 82S129

	// there are also two PALs of unknown type on the PCB:
	//054843 @12F
	//054844 @12H(12I?)
ROM_END

ROM_START( hexionb )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "hexionb.u2", 0x00000, 0x20000, CRC(93edc5d4) SHA1(d14c5be85a67eebddda9103bdf19de8c3c05d3af) )

	ROM_REGION( 0x80000, "tiles", 0 )    // addressable by the main CPU
	ROM_LOAD( "hexionb.u30",   0x00000, 0x40000, CRC(22ae55e3) SHA1(41bdc990f69416b639542e2186a3610c16389063) ) // == 122a07.1h
	ROM_LOAD( "hexionb.u29",   0x40000, 0x40000, CRC(438f4388) SHA1(9e23805c9642a237daeaf106187d1e1e0692434d) ) // == 122a06.1g

	ROM_REGION( 0x40000, "oki", 0 )
	ROM_LOAD( "hexionb.u16",   0x0000, 0x40000, CRC(bcc831bf) SHA1(c3382065dd0069a4dc0bde2d9931ec85b0bffc73) ) // == 122a05.2f

	ROM_REGION( 0x40000, "oki2", 0 )
	ROM_LOAD( "hexionb.u18",   0x0000, 0x40000, CRC(c179d315) SHA1(b39d5ec8a90b7ae06763191b8324f32fe1d0ca9b) )

	ROM_REGION( 0x0300, "proms", 0 )
	ROM_LOAD( "82s129.u36",   0x0000, 0x0100, CRC(506eb8c6) SHA1(3bff7cf286942d8bdbc3998245c3de20981fbecb) )
	ROM_LOAD( "82s129.u37",   0x0100, 0x0100, CRC(590c4f64) SHA1(db4b34f8c5fdfea034a94d65873f6fb842f123e9) )
	ROM_LOAD( "82s129.u38",   0x0200, 0x0100, CRC(5734305c) SHA1(c72e59acf79a4db1a5a9d827eef899c0675336f2) )

	// there are also two PALs on the bootleg:
	//PAL20L10 @U12
	//PAL20L10 @U31
ROM_END

} // anonymous namespace


GAME( 1992, hexion,  0,      hexion,  hexion, hexion_state, empty_init, ROT0, "Konami",                     "Hexion (Japan ver JAB)",         MACHINE_SUPPORTS_SAVE )
GAME( 1992, hexionb, hexion, hexionb, hexion, hexion_state, empty_init, ROT0, "bootleg (Impeuropex Corp.)", "Hexion (Asia ver AAA, bootleg)", MACHINE_SUPPORTS_SAVE ) // we're missing an original Asia AAA
