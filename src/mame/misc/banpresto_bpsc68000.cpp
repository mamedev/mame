// license:BSD-3-Clause
// copyright-holders:

/*
Banpresto M68K-based medal games with Banpresto customs

BPSC68000 PCB (rev B)

Main components:
- MC68EC000FN12 main CPU
- 2x LH52258AD-25 SRAM
- 24.000 MHz XTAL
- BPSPC 87F 9529 HAD3 Banpresto custom chip (208-pin, has the Banpresto logo)
- MFC68K 88F 9443 ABF3 Banpresto custom chip (100-pin, no Banpresto logo)
- SPMUX 89F 9531 ADG3 Banpresto custom chip (128-pin, no Banpresto logo)
- 2x HM514270CJ7 DRAM (between BPSPC and SPMUX customs)
- Sanyo LC89080Q high-speed current-output D/A converter
- 2x banks of 8 switches
- 2x LH528256D-70LL SRAM (near YMZ280 space)
- OKI M6295
- 1066J resonator (near M6295)
- M 535 1026B 8-pin chip (probably an EEPROM like Microchip 24LC1026 or similar)

The video section has an unpopulated space marked BPBG for a 160-pin (probably) custom chip.
The audio section also has unpopulated space marked for a YMZ280.


TODO:
- verify ticket dispenser hook-up (seems to work)
- unknown read / writes as noted in memory map
- spams "requested to play sample on non-stopped voice" from the Oki. Why?
- layout (cab picture is available)
- EEPROM? (don't see any obvious writes)
- sprites flip y bit (once a game which uses it is dumped)
- tilemaps (once a game which uses them is dumped)
- priorities (once a game which uses them is dumped)
- probably lots of other stuff (the dumped game makes little use of the hardware)
*/

#include "emu.h"

#include "cpu/m68000/m68000.h"
#include "machine/eepromser.h"
#include "machine/nvram.h"
#include "machine/ticket.h"
#include "sound/okim6295.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"


namespace {

class bpsc68000_state : public driver_device
{
public:
	bpsc68000_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_ticket_dispenser(*this, "ticket_dispenser"),
		m_spriteram(*this, "spriteram"),
		m_lamps(*this, "lamp%u", 0U)
	{ }

	void bpsc68000(machine_config &config) ATTR_COLD;

protected:
	virtual void machine_start() override ATTR_COLD { m_lamps.resolve(); }
	virtual void video_start() override ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<ticket_dispenser_device> m_ticket_dispenser;

	required_shared_ptr<uint16_t> m_spriteram;

	output_finder<4> m_lamps;

	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void counters_w(uint16_t data);
	void lamps_1_w(uint16_t data);
	void lamps_2_w(uint16_t data);

	void prg_map(address_map &map) ATTR_COLD;
};


void bpsc68000_state::video_start()
{
}

void bpsc68000_state::draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	for (int offs = m_spriteram.bytes() / 2 - 4; offs >= 0; offs -= 4)
	{
		if (!BIT(m_spriteram[offs], 2))
			continue;

		int sprite = m_spriteram[offs + 1];
		int const x = m_spriteram[offs + 2] & 0x3ff;
		int const y = m_spriteram[offs + 3] & 0x3ff;
		int const flipx = BIT(m_spriteram[offs], 4);
		int const flipy = 0; // TODO
		int const color = m_spriteram[offs] >> 8;
		int const xchain = m_spriteram[offs + 2] >> 11;
		int const ychain = m_spriteram[offs + 3] >> 11;

		for (int cy = 0; cy < (ychain + 1); cy++)
			for (int cx = 0; cx < (xchain + 1); cx++)
				m_gfxdecode->gfx(0)->transpen(bitmap, cliprect, sprite++, color, flipx, flipy, x + 16 * cx, y + 16 * cy, 0);
	}
}

uint32_t bpsc68000_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	draw_sprites(bitmap, cliprect);

	return 0;
}


void bpsc68000_state::counters_w(uint16_t data)
{
	// at start the game writes here with a 0x00ff mem_mask. Only value observed is 0xc6c6.
	// after that, the game writes with a 0xffff mem_mask

	for (int i = 0x00; i < 0x04; i++)
		if (BIT(data, i))
			logerror("%s counters_w unknown bit %1x written: %04x\n", machine().describe_context(), i, data);

	// bit 1 and 2 seem coin lockout related

	machine().bookkeeping().coin_counter_w(0, BIT(data, 4)); // 100 Yen

	machine().bookkeeping().coin_counter_w(1, BIT(data, 5)); // medal

	// TODO: strange hook-up, check if correct
	if (BIT(data, 6))
		m_ticket_dispenser->motor_w(1);

	if (BIT(data, 7))
		m_ticket_dispenser->motor_w(0);

	for (int i = 0x08; i < 0x10; i++)
		if (BIT(data, i))
			logerror("%s counters_w unknown bit %1x written: %04x\n", machine().describe_context(), i, data);

}

void bpsc68000_state::lamps_1_w(uint16_t data)
{
	m_lamps[1] = BIT(data, 4); // stop 1 lamp, shown in test mode
	m_lamps[2] = BIT(data, 5); // stop 2 lamp, shown in test mode
	m_lamps[3] = BIT(data, 6); // stop 3 lamp, shown in test mode

	if (data & 0xff8f)
		logerror("%s lamps_1_w unknown bits written: %04x\n", machine().describe_context(), data);
}

void bpsc68000_state::lamps_2_w(uint16_t data)
{
	m_lamps[0] = BIT(data, 0); // start lamp, shown in test mode

	if (data & 0xfffe)
		logerror("%s lamps_2_w unknown bits written: %04x\n", machine().describe_context(), data);
}


void bpsc68000_state::prg_map(address_map &map)
{
	map(0x000000, 0x01ffff).rom();
	map(0x200000, 0x2001ff).ram().w("palette", FUNC(palette_device::write16)).share("palette"); // TODO: surely bigger, adjust when a game using tilemaps is dumped
	map(0x200200, 0x20dfff).ram();
	map(0x20e000, 0x20ffff).ram().share(m_spriteram);
	// map(0x210000, 0x21000f).w() // only written shortly after start-up (CRTC programming?). All writes & 00ff
	map(0xa00000, 0xa00001).portr("DSW1");
	map(0xa00002, 0xa00003).portr("DSW2");
	// map(0xc00000, 0xc00001).rw() // read from c00000 & 00ff (rare), write to c00000 & 00ff (rare, only seen 1111 value)
	// map(0xc00002, 0xc00003).r() // read from c00002 & 00ff (often), write to c00000 & 00ff (often, only seen 0000 value)
	// map(0xc00008, 0xc00009).w() // write to c00008 & 00ff (rare, only seen 0808 value)
	// map(0xc0000a, 0xc0000b).w() // write to c0000a & 00ff (rare, only seen 0000 value)
	// map(0xc0000c, 0xc0000d).w() // write to c0000c & 00ff (rare, only seen 8c8c value)
	// map(0xc0000e, 0xc0000f).w() // write to c0000e & 00ff (rare, only seen 9999 value)
	map(0xc00020, 0xc00021).w(FUNC(bpsc68000_state::counters_w)); // .r() read from c00020 & ffff
	map(0xc00022, 0xc00023).w(FUNC(bpsc68000_state::lamps_1_w)); // .r() read from c00022 & ffff
	map(0xc00024, 0xc00025).w(FUNC(bpsc68000_state::lamps_2_w)); // .r() read from c00024 & ffff
	// map(0xc0002e, 0xc0002f).w() // write to c0002e & 00ff (rare, only seen 0707 value)
	map(0xc00026, 0xc00027).portr("IN0");
	map(0xc00028, 0xc00029).portr("IN1");
	map(0xc0002a, 0xc0002b).portr("IN2");
	map(0x800001, 0x800001).rw("oki", FUNC(okim6295_device::read), FUNC(okim6295_device::write));
	map(0xe00000, 0xe07fff).ram().share("nvram");
}


static INPUT_PORTS_START( lnumbers )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN ) // no effect in test mode
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN1 ) // 100 Yen
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN2 ) // Medal
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_SERVICE_NO_TOGGLE( 0x10, IP_ACTIVE_LOW )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN ) // no effect in test mode
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("ticket_dispenser", FUNC(ticket_dispenser_device::line_r)) // Medal sensor
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN ) // no effect in test mode

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN ) // no effect in test mode
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN ) // no effect in test mode
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_GAMBLE_PAYOUT )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN ) // no effect in test mode
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SLOT_STOP1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SLOT_STOP2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SLOT_STOP3 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN ) // no effect in test mode

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN ) // no effect in test mode
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN ) // no effect in test mode
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN ) // no effect in test mode
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN ) // no effect in test mode
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN ) // no effect in test mode
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN ) // no effect in test mode
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN ) // no effect in test mode

	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("DSW1:1")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x0e, 0x0e, "Payout %" ) PORT_DIPLOCATION("DSW1:2,3,4")
	PORT_DIPSETTING(    0x0e, "55" )
	PORT_DIPSETTING(    0x0c, "60" )
	PORT_DIPSETTING(    0x0a, "65" )
	PORT_DIPSETTING(    0x08, "70" )
	PORT_DIPSETTING(    0x06, "75" )
	PORT_DIPSETTING(    0x04, "80" )
	PORT_DIPSETTING(    0x02, "85" )
	PORT_DIPSETTING(    0x00, "90" )
	PORT_DIPNAME( 0x10, 0x10, "Winwave" ) PORT_DIPLOCATION("DSW1:5")
	PORT_DIPSETTING(    0x10, "Small" )
	PORT_DIPSETTING(    0x00, "Big" )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW1:6") // no effect in test mode
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW1:7") // no effect in test mode
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW1:8") // no effect in test mode
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x0f, 0x0f, DEF_STR( Coinage ) ) PORT_DIPLOCATION("DSW2:1,2,3,4") // Medals for 100 Yen
	PORT_DIPSETTING(    0x0f, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x0e, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x0d, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x0b, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(    0x0a, DEF_STR( 1C_7C ) )
	PORT_DIPSETTING(    0x09, DEF_STR( 1C_8C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_9C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 1C_10C ) )
	PORT_DIPSETTING(    0x06, "1 Coin/11 Credits" )
	PORT_DIPSETTING(    0x05, "1 Coin/12 Credits" )
	PORT_DIPSETTING(    0x04, "1 Coin/13 Credits" )
	PORT_DIPSETTING(    0x03, "1 Coin/14 Credits" )
	PORT_DIPSETTING(    0x02, "1 Coin/15 Credits" )
	PORT_DIPSETTING(    0x01, "1 Coin/16 Credits" )
	PORT_DIPSETTING(    0x00, "1 Coin/17 Credits" )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW2:5") // no effect in test mode
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW2:6") // no effect in test mode
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW2:7") // no effect in test mode
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW2:8") // no effect in test mode
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END


static GFXDECODE_START( gfx )
	GFXDECODE_ENTRY( "sprites", 0, gfx_16x16x4_packed_lsb, 0, 16 )
GFXDECODE_END


void bpsc68000_state::bpsc68000(machine_config &config)
{
	// basic machine hardware
	M68000(config, m_maincpu, 24_MHz_XTAL / 2);
	m_maincpu->set_addrmap(AS_PROGRAM, &bpsc68000_state::prg_map);
	m_maincpu->set_vblank_int("screen", FUNC(bpsc68000_state::irq4_line_hold));

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	TICKET_DISPENSER(config, m_ticket_dispenser, attotime::from_msec(100)); // guessed period

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(64*8, 32*8); // TODO
	screen.set_visarea(0, 256-1, 0, 224-1);
	screen.set_screen_update(FUNC(bpsc68000_state::screen_update));
	screen.set_palette("palette");

	GFXDECODE(config, m_gfxdecode, "palette", gfx);

	PALETTE(config, "palette").set_format(palette_device::xRGB_555, 0x100);

	// sound hardware
	SPEAKER(config, "mono").front_center();

	OKIM6295(config, "oki", 1'066'000, okim6295_device::PIN7_HIGH).add_route(ALL_OUTPUTS, "mono", 0.40); // TODO: check pin 7.
}


// ウルトラマン倶楽部 ラッキーナンバーズ
ROM_START( lnumbers )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "s197_a01.prog16b.u15", 0x00000, 0x20000, CRC(b824a0ed) SHA1(ea8ef81d17896205f89d330066a23c459ab5e668) )

	ROM_REGION( 0x100000, "sprites", 0 )
	ROM_LOAD( "s197_a03_obj2.u25", 0x00000, 0x80000, CRC(5d5955e8) SHA1(aea84b08ed639b2bacada1c4ade9760b2599cfc7) )
	ROM_LOAD( "s197_a04.obj1.u24", 0x80000, 0x80000, CRC(e70f4f14) SHA1(030bbbe2da07a9b8c3a8c7055799e89113bad16b) )
	// unpopulated OBJ3 / OBJ4 space at u26 / u27

	// unpopulated BG1 space at u31

	ROM_REGION( 0x40000, "oki", 0 )
	ROM_LOAD( "s197_a02.sound2.u41", 0x00000, 0x40000, CRC(a1a67ead) SHA1(d4f0979c534a6d131025da474944e0d32f583693) )
	// PCB can accommodate bigger ROMs, too

	ROM_REGION( 0x200, "plds", ROMREGION_ERASE00 )
	ROM_LOAD( "peel18cv8p-10_u2.u2", 0x000, 0x155, NO_DUMP )
	// unpopulated spaces at u28 and u44 marked 18CV8
ROM_END

} // anonymous namespace


GAME( 1995, lnumbers, 0, bpsc68000, lnumbers, bpsc68000_state, empty_init, ROT0, "Banpresto", "Ultraman Club - Lucky Numbers", MACHINE_SUPPORTS_SAVE )
