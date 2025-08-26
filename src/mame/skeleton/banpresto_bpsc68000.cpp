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
- everything
*/

#include "emu.h"

#include "cpu/m68000/m68000.h"
#include "machine/eepromser.h"
#include "machine/nvram.h"
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
		m_gfxdecode(*this, "gfxdecode")
	{ }

	void bpsc68000(machine_config &config) ATTR_COLD;

protected:
	virtual void video_start() override ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void prg_map(address_map &map) ATTR_COLD;
};


void bpsc68000_state::video_start()
{
}

uint32_t bpsc68000_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	return 0;
}


void bpsc68000_state::prg_map(address_map &map)
{
	map.unmap_value_high(); // TODO: remove once more about the hw is figured out

	map(0x000000, 0x01ffff).rom();
	map(0x200000, 0x20ffff).ram();
	// map(0xa00000, 0xa00001).r() // inputs?
	// map(0xa00002, 0xa00003).r() // inputs??
	// map(0xc00000, 0xc0002b).rw(); // video regs?
	map(0x800001, 0x800001).w("oki", FUNC(okim6295_device::write));
	map(0xe00000, 0xe07fff).ram();
}


static INPUT_PORTS_START( lnumbers )
	PORT_START("IN0")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(3)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(3)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(3)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(3)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_PLAYER(3)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_PLAYER(3)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON7 ) PORT_PLAYER(3)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_BUTTON8 ) PORT_PLAYER(3)
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(4)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(4)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(4)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(4)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_PLAYER(4)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_PLAYER(4)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_BUTTON7 ) PORT_PLAYER(4)
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_BUTTON8 ) PORT_PLAYER(4)

	PORT_START("DSW1")
	PORT_DIPNAME( 0x0001, 0x0001, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW1:1")
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0002, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW1:2")
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0004, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW1:3")
	PORT_DIPSETTING(      0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0008, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW1:4")
	PORT_DIPSETTING(      0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW1:5")
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW1:6")
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW1:7")
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW1:8")
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0100, 0x0100, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW2:1")
	PORT_DIPSETTING(      0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0200, 0x0200, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW2:2")
	PORT_DIPSETTING(      0x0200, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0400, 0x0400, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW2:3")
	PORT_DIPSETTING(      0x0400, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0800, 0x0800, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW2:4")
	PORT_DIPSETTING(      0x0800, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x1000, 0x1000, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW2:5")
	PORT_DIPSETTING(      0x1000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x2000, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW2:6")
	PORT_DIPSETTING(      0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000, 0x4000, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW2:7")
	PORT_DIPSETTING(      0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000, 0x8000, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW2:8")
	PORT_DIPSETTING(      0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
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

	// NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER)); // TODO
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(64*8, 32*8);
	screen.set_visarea(0, 320-1, 16, 240-1);
	screen.set_screen_update(FUNC(bpsc68000_state::screen_update));
	screen.set_palette("palette");

	GFXDECODE(config, m_gfxdecode, "palette", gfx);

	PALETTE(config, "palette").set_format(palette_device::xBGR_555, 0x400);

	// sound hardware
	SPEAKER(config, "mono").front_center();

	OKIM6295(config, "oki", 1'066'000, okim6295_device::PIN7_HIGH).add_route(ALL_OUTPUTS, "mono", 0.40); // TODO: check pin 7.
}


// ウルトラマン倶楽部 ラッキーナンバーズ
ROM_START( lnumbers )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "s197_a01.prog16b.u15", 0x00000, 0x20000, CRC(b824a0ed) SHA1(ea8ef81d17896205f89d330066a23c459ab5e668) )

	ROM_REGION( 0x100000, "sprites", 0 )
	ROM_LOAD( "s197_a04.obj1.u24", 0x00000, 0x80000, CRC(e70f4f14) SHA1(030bbbe2da07a9b8c3a8c7055799e89113bad16b) )
	ROM_LOAD( "s197_a03_obj2.u25", 0x80000, 0x80000, CRC(5d5955e8) SHA1(aea84b08ed639b2bacada1c4ade9760b2599cfc7) )
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


GAME( 1995, lnumbers, 0, bpsc68000, lnumbers, bpsc68000_state, empty_init, ROT0, "Banpresto", "Ultraman Club - Lucky Numbers", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
