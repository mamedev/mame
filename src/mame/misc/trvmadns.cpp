// license: BSD-3-Clause
// copyright-holders: Pierpaolo Prazzoli, Dirk Best
// thanks-to: David Haywood, Tomasz Slanina
/***************************************************************************

    Trivia Madness

    © 1985 Thunderhead Inc. / M. Kramer Manufacturing Co., Inc.

    TODO:
    - Verify palette
    - Remove IRQ hack
    - Writes to 0xe000 (watchdog?)

    Notes:
    - Copyright notice inside the ROM: "(C) COPYRIGHT 1985 DESIGN LABS, INC."
    - Hold the service mode key and reset to wipe NVRAM


PC-1031-D    (CPU Board)
+---------------------------------------------+
|H6845P       CNA                   10MHz     |
|      BAT          CNB                       C
|       9                                     N
C       1               6       8             2
N  Z    2               1       9             8
5  8    8  U   U   U    1       1             |
0  0       7   6   5    6       0          VOL|
+---------------------------------------------+

A-1041-B    (ROM Board)
+-------------------------|-CN50-|------------+
|                                     AM9128  |
|                       HM6264                |
|                                             |
C PAL10L8  U13   U17    U21     U29     U35   |
N          U12   U16    U20     U28     U34   |
5          U11   U15    U19     U27     U33   |
0          U10   U14    U18     U26     U32   |
+---------------------------------------------+

CPU board:
  CPU: Z84000AB1 Z80 cpu
Sound: AY-3-8910
  RAM: AMD AM9128-15PC (2048x8 Static RAM)
  OSC: 10.000MHz
Video: F6845P (or H6845P also labeled as HD46505RP) 40 pin dil (8 bit CRT Controller)
 Misc: RCA X (CDM6116E2) 24 pin dil (General-Purpose Static RAM - Multiplexed I/O)
 Roms: u7f lat green - type 2764
       u6f lat green - type 2764
       u5f lat la trivia - type 2764
  BAT: Battery to back AM9128 for game configuration?
 CN28: Edge connector 28 finger dual side for 56 connections, not JAMMA compatible
2 50-pin Ribon cable connectors (CNA + CNB make up 1 50-pin connection to the ROM board)

Rom board:
 Ram: HM6264P-15 (64 K SRAM ( 8-kword X 8-bit )
 Ram: 8437 (SY2128-3 2Kx8 SRAM 150NS)
Roms: row d-e sex a1       - type 27128
      row c-d sex a2       - type 27128
      row b-c sex a3       - type 27128
      row a sex a4         - type 27128
      row d-e soaps a1     - type 27128
      row c-d soaps a2     - type 27128
      row b-c soaps a3     - type 27128
      row d-e 2 str trk a1 - type 27256
      row c-d 2 str trk a2 - type 27256
      row d-e 2 rebel a1   - type 27256
      row c-d 2 rebel a2   - type 27256
      row triva madness    - type 27128
2 50-pin Ribon cable connectors

***************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "machine/bankdev.h"
#include "machine/nvram.h"
#include "sound/ay8910.h"
#include "video/mc6845.h"
#include "video/pwm.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"

#include "trvmadns.lh"


namespace {

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class trvmadns_state : public driver_device
{
public:
	trvmadns_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_bankdev(*this, "bankdev"),
		m_crtc(*this, "crtc"),
		m_lamps(*this, "lamps"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_gfxram(*this, "gfxram"),
		m_tileram(*this, "tileram"),
		m_paletteram(*this, "paletteram")
	{ }

	void trvmadns(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;
	required_device<address_map_bank_device> m_bankdev;
	required_device<mc6845_device> m_crtc;
	required_device<pwm_display_device> m_lamps;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_shared_ptr<uint8_t> m_gfxram;
	required_shared_ptr<uint8_t> m_tileram;
	required_shared_ptr<uint8_t> m_paletteram;

	void cpu_map(address_map &map) ATTR_COLD;
	void romboard_map(address_map &map) ATTR_COLD;
	void io_map(address_map &map) ATTR_COLD;

	void unknown_w(uint8_t data);
	void lamps_w(uint8_t data);

	void postload();

	void gfxram_w(offs_t offset, uint8_t data);
	void palette_w(offs_t offset, uint8_t data);
	void tileram_w(offs_t offset, uint8_t data);

	TILE_GET_INFO_MEMBER(tile_info);
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	tilemap_t *m_tilemap = nullptr;
};


//**************************************************************************
//  ADDRESS MAPS
//**************************************************************************

void trvmadns_state::cpu_map(address_map &map)
{
	map(0x0000, 0x5fff).rom();
	map(0x6000, 0x7fff).m(m_bankdev, FUNC(address_map_bank_device::amap8));
	map(0x8000, 0x87ff).ram().share("nvram"); // u3
	map(0xa000, 0xa7ff).ram().w(FUNC(trvmadns_state::tileram_w)).share("tileram"); // u17
	map(0xc000, 0xc000).mirror(0x1ffe).w(m_crtc, FUNC(mc6845_device::address_w));
	map(0xc001, 0xc001).mirror(0x1ffe).rw(m_crtc, FUNC(mc6845_device::register_r), FUNC(mc6845_device::register_w));
	map(0xe000, 0xe000).w(FUNC(trvmadns_state::unknown_w));
	map(0xe004, 0xe004).w(FUNC(trvmadns_state::lamps_w));
}

void trvmadns_state::romboard_map(address_map &map)
{
	map(0x000000, 0x00ffff).rom().region("set1", 0); // 00-07
	map(0x040000, 0x04ffff).rom().region("set2", 0); // 20-27
	map(0x080000, 0x08ffff).rom().region("set3", 0); // 40-47
	map(0x0c0000, 0x0cffff).rom().region("set4", 0); // 60-67
	map(0x100000, 0x13ffff).rom().region("extra", 0); // 80-9f
	map(0x140000, 0x141fff).ram().w(FUNC(trvmadns_state::gfxram_w)).share("gfxram"); // a0
	map(0x150000, 0x1500ff).ram().w(FUNC(trvmadns_state::palette_w)).share("paletteram"); // a8
}

void trvmadns_state::io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x00).w("ay", FUNC(ay8910_device::address_w));
	map(0x01, 0x01).w("ay", FUNC(ay8910_device::data_w));
	map(0x02, 0x02).r("ay", FUNC(ay8910_device::data_r));
	map(0x80, 0x80).lw8(NAME([this] (uint8_t data) { m_bankdev->set_bank(data); }));
}


//**************************************************************************
//  INPUT PORT DEFINITIONS
//**************************************************************************

static INPUT_PORTS_START( trvmadns )
	PORT_START("IN0")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_POKER_HOLD1)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_POKER_HOLD2)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_POKER_HOLD3)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_POKER_HOLD4)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_POKER_HOLD5)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_COIN1)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_START1)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_SERVICE)
INPUT_PORTS_END


//**************************************************************************
//  VIDEO EMULATION
//**************************************************************************

void trvmadns_state::tileram_w(offs_t offset, uint8_t data)
{
	if (offset == 0)
	{
		// Mystery device connected on bit 0 here that can cause an
		// interrupt. Gets initialized with 19 bits, then bits are
		// written that should cause an IRQ. This is only done once
		// at startup.

		if (m_maincpu->pcbase() == 0x29e9)
			m_maincpu->set_input_line(0, HOLD_LINE);
	}

	m_tileram[offset] = data;
	m_tilemap->mark_tile_dirty(offset >> 1);
}

void trvmadns_state::gfxram_w(offs_t offset, uint8_t data)
{
	m_gfxram[offset] = data;
	m_gfxdecode->gfx(0)->mark_dirty(offset / 16);
}

void trvmadns_state::palette_w(offs_t offset, uint8_t data)
{
	m_paletteram[offset] = data;

	int b = (data >> 6) & 0x03;
	int g = (data >> 3) & 0x07;
	int r = (data >> 0) & 0x07;

	m_palette->set_pen_color(offset, pal3bit(r), pal3bit(g), pal2bit(b));
}

uint32_t trvmadns_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_tilemap->draw(screen, bitmap, cliprect, 0, 0);

	return 0;
}

static const gfx_layout charlayout =
{
	8, 8,
	0x200,
	2,
	{ 1, 0 },
	{ 6, 4, 2, 0, 14, 12, 10, 8 },
	{ 0*16, 1*16, 2*16, 3*16, 4*16, 5*16, 6*16, 7*16 },
	8*16
};

static GFXDECODE_START( gfx_trvmadns )
	GFXDECODE_RAM("gfxram", 0, charlayout, 0, 16)
GFXDECODE_END

TILE_GET_INFO_MEMBER(trvmadns_state::tile_info)
{
	// 7-------  not used (or color?)
	// -6543---  color
	// -----2--  flip x
	// ------1-  flip y
	// -------0  tile index bit 8

	uint8_t attr = m_tileram[(tile_index << 1) + 0];
	uint16_t tile = (BIT(attr, 0) << 8) | m_tileram[(tile_index << 1) + 1];
	uint8_t color = (attr >> 3) & 0x0f;
	uint8_t flags = TILE_FLIPXY((attr >> 1) & 0x03);

	tileinfo.set(0, tile, color, flags);
}


//**************************************************************************
//  MACHINE EMULATION
//**************************************************************************

void trvmadns_state::unknown_w(uint8_t data)
{
	// 7654321-  not used
	// -------0  unknown
}

void trvmadns_state::lamps_w(uint8_t data)
{
	// 7-------  not used
	// -6------  start button led
	// --5-----  not used
	// ---43210  button lamps 5 to 1

	m_lamps->matrix(1, data);
}

void trvmadns_state::machine_start()
{
	machine().save().register_postload(save_prepost_delegate(FUNC(trvmadns_state::postload), this));
	m_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(trvmadns_state::tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);
}

void trvmadns_state::postload()
{
	m_gfxdecode->gfx(0)->mark_all_dirty();
}


//**************************************************************************
//  MACHINE DEFINTIONS
//**************************************************************************

void trvmadns_state::trvmadns(machine_config &config)
{
	Z80(config, m_maincpu, 10_MHz_XTAL / 4); // most likely 2.5 MHz (less likely 5 MHz)
	m_maincpu->set_addrmap(AS_PROGRAM, &trvmadns_state::cpu_map);
	m_maincpu->set_addrmap(AS_IO, &trvmadns_state::io_map);

	ADDRESS_MAP_BANK(config, m_bankdev, 0);
	m_bankdev->set_addrmap(AS_PROGRAM, &trvmadns_state::romboard_map);
	m_bankdev->set_data_width(8);
	m_bankdev->set_addr_width(21);
	m_bankdev->set_stride(0x2000);

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(10_MHz_XTAL / 2, 320, 0, 256, 261, 0, 240); // 5 MHz?
	screen.set_screen_update(FUNC(trvmadns_state::screen_update));
	screen.set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_trvmadns);

	PALETTE(config, m_palette).set_entries(256);

	MC6845(config, m_crtc, 10_MHz_XTAL / 16); // 0.625 MHz?
	m_crtc->set_screen("screen");
	m_crtc->set_show_border_area(false);
	m_crtc->set_char_width(8);
	m_crtc->out_vsync_callback().set_inputline(m_maincpu, INPUT_LINE_NMI);

	PWM_DISPLAY(config, m_lamps).set_size(1, 8);
	m_lamps->set_bri_levels(0.9);

	// sound hardware
	SPEAKER(config, "mono").front_center();

	ay8910_device &ay(AY8910(config, "ay", 10_MHz_XTAL / 2 / 4)); // 1.25 MHz?
	ay.port_a_read_callback().set_ioport("IN0");
	ay.port_b_read_callback().set_constant(0xff);
	ay.add_route(ALL_OUTPUTS, "mono", 1.0);

	config.set_default_layout(layout_trvmadns);
}


//**************************************************************************
//  ROM DEFINITIONS
//**************************************************************************

ROM_START( trvmadns )
	ROM_REGION(0x6000, "maincpu", 0)
	ROM_LOAD("u5f lat la trivia.u5", 0x0000, 0x2000, CRC(a8fb07ea) SHA1(dcf2cccd8b98087d30b3347e69b1bf8565f95ad6) )
	ROM_LOAD("u6f lat green.u6",     0x2000, 0x2000, CRC(40f816f1) SHA1(a1a6a9af99edb1860bc4c8eb51859bbfbf91cae2) )
	ROM_LOAD("u7f lat green.u7",     0x4000, 0x2000, CRC(3e45feb0) SHA1(5ffc18ab3f6ace844242d4be52b3946c1469944a) )

	ROM_REGION(0x10000, "set1", 0)
	ROM_LOAD("row c-d 2 rebel a2.bin",   0x0000, 0x8000, CRC(b45429be) SHA1(9285f12bc0bceb0a91fc1f2f2941825b73bdb02c) )
	ROM_LOAD("row d-e 2 rebel a1.bin",   0x8000, 0x8000, CRC(92e6dcf8) SHA1(e8429fe60fadfc841ed0d69b4a815765e82723db) )

	ROM_REGION(0x10000, "set2", 0)
	ROM_LOAD("row c-d 2 str trk a2.bin", 0x0000, 0x8000, CRC(0133c462) SHA1(e6928880cb7916408579fa9d67ad3adc558de133) )
	ROM_LOAD("row d-e 2 str trk a1.bin", 0x8000, 0x8000, CRC(dc6fc7e1) SHA1(86ba730123bbb06d8290d68c042dd215bddf7629) )

	ROM_REGION(0x10000, "set3", 0)
	ROM_LOAD("row b-c soaps a3.bin",     0x4000, 0x4000, CRC(b42f3294) SHA1(eac4e26ed48e7de80a60fbb2ee4e661619700d1d) )
	ROM_LOAD("row c-d soaps a2.bin",     0x8000, 0x4000, CRC(13685dac) SHA1(f5f6103404c846decf32eae5d504e00e17629b03) )
	ROM_LOAD("row d-e soaps a1.bin",     0xc000, 0x4000, CRC(df451b8a) SHA1(23a5f953018e5401fe26eb638e13caacf6fa628f) )

	ROM_REGION(0x10000, "set4", 0)
	ROM_LOAD("row a sex a4.bin",         0x0000, 0x4000, CRC(2d179c7b) SHA1(153240f1fcc4f53b6840eafdd9ce0fb3e52ec1aa) )
	ROM_LOAD("row b-c sex a3.bin",       0x4000, 0x4000, CRC(3fea2c2a) SHA1(fa403e14b057f0e6d607871adcaba85a6c77f1f9) )
	ROM_LOAD("row c-d sex a2.bin",       0x8000, 0x4000, CRC(8fba2e07) SHA1(cbe7f9b973bd2a127cae736df39112b050ec98d8) )
	ROM_LOAD("row d-e sex a1.bin",       0xc000, 0x4000, CRC(857c1332) SHA1(fdb08080143e170441b3db9e69b21ac9da10d499) )

	ROM_REGION(0x40000, "extra", 0)
	ROM_LOAD("trivia madness 81b9--6aa6.u35", 0xc000, 0x4000, CRC(5aec7cfa) SHA1(09e4eac78d975aef3af224b42b60499d759e7749) )

	ROM_REGION(0x2c, "pal", 0)
	ROM_LOAD("pal10l8_la1.u5", 0x00, 0x2c, CRC(66d6eee0) SHA1(f95d9bfc2a6038237014eec30d859d8b8c296725))
ROM_END

ROM_START( trvmadnsa )
	ROM_REGION(0x6000, "maincpu", 0)
	ROM_LOAD("u5f lat la trivia.u5", 0x0000, 0x2000, CRC(a8fb07ea) SHA1(dcf2cccd8b98087d30b3347e69b1bf8565f95ad6) )
	ROM_LOAD("u6f lat green.u6",     0x2000, 0x2000, CRC(40f816f1) SHA1(a1a6a9af99edb1860bc4c8eb51859bbfbf91cae2) )
	ROM_LOAD("u7f lat green.u7",     0x4000, 0x2000, CRC(3e45feb0) SHA1(5ffc18ab3f6ace844242d4be52b3946c1469944a) )

	ROM_REGION(0x10000, "set1", 0)
	ROM_LOAD("row a. travel b4 04bc--8208.u10",   0x0000, 0x4000, CRC(eaa9c4d3) SHA1(49518a5baba42459b0a777d25874e0ef979a3847) )
	ROM_LOAD("row b.c. travel b3 bcbd--f516.u11", 0x4000, 0x4000, CRC(389a0f0f) SHA1(ef9cbb8ce921aadfea9932074899e8c08eea5d4e) )
	ROM_LOAD("row c.d. travel b2 04b0--ed47.u12", 0x8000, 0x4000, CRC(62247db1) SHA1(3c24d0c77bd8560d3ec26603b5ba18ffb401f5d2) )
	ROM_LOAD("row d.e. travel b1 a257--b03b.u13", 0xc000, 0x4000, CRC(fc7711eb) SHA1(91e3ae7be16a498aef6f1594744043ad5efd4b26) )

	ROM_REGION(0x10000, "set2", 0)
	ROM_LOAD("row a. sports b4 6392--bb9b.u14",   0x0000, 0x4000, CRC(c333669a) SHA1(a2db88f716a529ab88ad01a165b3581b299f6283) )
	ROM_LOAD("row c.d. sports b2 f9c1--1f79.u16", 0x8000, 0x4000, CRC(b582bd2b) SHA1(cb8123e971d3618573591a4cbd13c40531b70140) )
	ROM_LOAD("row b.c. sports b3 0167--04c6.u15", 0x4000, 0x4000, CRC(c182b664) SHA1(2af3050cf375528bd27a09cce30832a678bb55db) )
	ROM_LOAD("row d.e. sports b1 4714--1f76.u17", 0xc000, 0x4000, CRC(70a33fbd) SHA1(e4e725a86b85827599b5ba2fec56352e55c0f33d) )

	ROM_REGION(0x10000, "set3", 0)
	ROM_LOAD("row a. t.v. b4 2555--9e1e.u18",     0x0000, 0x4000, CRC(f6837c47) SHA1(f3e49fe69ab84eba8ead04b7c2c1d0c9227517fd) )
	ROM_LOAD("row b.c. t.v. b3 22c9--de46.u19",   0x4000, 0x4000, CRC(eeebbfa0) SHA1(d679c29a7868aa2214857d8381cdff4f7a7c116f) )
	ROM_LOAD("row c.d. t.v. b2 e38b--f95a.u20",   0x8000, 0x4000, CRC(7ded2e40) SHA1(1766f12f82f4692b0f73e4a65456c4ed7dbb56ee) )
	ROM_LOAD("row d.e. t.v. b1 2313--edb8.u21",   0xc000, 0x4000, CRC(9841b455) SHA1(1281d9085a026617950d609cf3cb8c45d58b6aa3) )

	ROM_REGION(0x10000, "set4", 0)
	ROM_LOAD("row a. ent b4 6429--71fb.u26",      0x0000, 0x4000, CRC(eb72757e) SHA1(4614e845ec44c04a208afc9bff16067b25091ba3) )
	ROM_LOAD("row b.c. ent b3 bcf7--914c.u27",    0x4000, 0x4000, CRC(48d6f2f4) SHA1(59fe01a8474fb3c77a995cb7d55ea1dacbfb247a) )
	ROM_LOAD("row c.d. ent b2 8e46--6967.u28",    0x8000, 0x4000, CRC(fc6aa7f0) SHA1(e95a7bf6dc07d151abb46c0066208666d01b96a8) )
	ROM_LOAD("row d.e. ent b1 5fae--1ec8.u29",    0xc000, 0x4000, CRC(7ff56ea9) SHA1(c8e6e3b0ac4fc2ac566d041dee5422c6976d3b91) )

	ROM_REGION(0x40000, "extra", 0)
	ROM_LOAD("trivia madness 81b9--6aa6.u35", 0xc000, 0x4000, CRC(5aec7cfa) SHA1(09e4eac78d975aef3af224b42b60499d759e7749) )

	ROM_REGION(0x2c, "pal", 0)
	ROM_LOAD("pal10l8_la1.u5", 0x00, 0x2c, CRC(66d6eee0) SHA1(f95d9bfc2a6038237014eec30d859d8b8c296725))
ROM_END

} // anonymous namespace


//**************************************************************************
//  SYSTEM DRIVERS
//**************************************************************************

//    YEAR  NAME       PARENT    MACHINE   INPUT     CLASS           INIT        ROTATION  COMPANY             FULLNAME                                  FLAGS
GAME( 1985, trvmadns,         0, trvmadns, trvmadns, trvmadns_state, empty_init, ROT0,     "Thunderhead Inc.", "Trivia Madness - Series A Question set", MACHINE_IMPERFECT_COLORS | MACHINE_SUPPORTS_SAVE )
GAME( 1985, trvmadnsa, trvmadns, trvmadns, trvmadns, trvmadns_state, empty_init, ROT0,     "Thunderhead Inc.", "Trivia Madness - Series B Question set", MACHINE_IMPERFECT_COLORS | MACHINE_SUPPORTS_SAVE )
