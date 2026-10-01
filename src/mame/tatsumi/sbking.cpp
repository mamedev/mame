// license:BSD-3-Clause
// copyright-holders:

/*
Super Bonus King by Tazmi

2-PCB stack (TVS_05MB + TVS_05S)

TVS_05MB
almost all components are scratched, so the following is based on read / write patterns
Z80 CPU
6 MHz XTAL (confirmed)
NEC D445LC RAM (confirmed) + another scratched one
2x I8255 PPIs
MC6845 CRTC
bank of 8 switches
10-position rotary switch



TVS_05S
almost all components are scratched, so the following is based on read / write patterns
Z80 CPU
3.5795 MHz XTAL (confirmed)
AY8910 sound chip


TODO:
- colors
- inputs / outputs
- sound
- screen dimensions
*/


#include "emu.h"

#include "cpu/z80/z80.h"
#include "machine/i8255.h"
#include "sound/ay8910.h"
#include "video/mc6845.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"


namespace {

class sbking_state : public driver_device
{
public:
	sbking_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_tileram(*this, "tileram"),
		m_attrram(*this, "attrram")
	{ }

	void sbking(machine_config &config) ATTR_COLD;

protected:
	virtual void video_start() override ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;

	required_shared_ptr<uint8_t> m_tileram;
	required_shared_ptr<uint8_t> m_attrram;

	tilemap_t *m_bg_tilemap = nullptr;

	void palette_init_cb(palette_device &palette) const ATTR_COLD;

	TILE_GET_INFO_MEMBER(get_bg_tile_info);

	void tileram_w(offs_t offset, uint8_t data);
	void attrram_w(offs_t offset, uint8_t data);

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	void main_program_map(address_map &map) ATTR_COLD;
	void audio_program_map(address_map &map) ATTR_COLD;
	void audio_io_map(address_map &map) ATTR_COLD;
};


void sbking_state::video_start()
{
	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(sbking_state::get_bg_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);
}

void sbking_state::palette_init_cb(palette_device &palette) const
{
	// TODO
}

TILE_GET_INFO_MEMBER(sbking_state::get_bg_tile_info)
{
	int const tile = m_tileram[tile_index] | (BIT(m_attrram[tile_index], 0) << 8);
	int const color = (m_attrram[tile_index] & 0x0e) >> 1;

	tileinfo.set(0, tile, color, 0);
}

void sbking_state::tileram_w(offs_t offset, uint8_t data)
{
	m_tileram[offset] = data;

	m_bg_tilemap->mark_tile_dirty(offset);
}

void sbking_state::attrram_w(offs_t offset, uint8_t data)
{
	m_attrram[offset] = data;

	m_bg_tilemap->mark_tile_dirty(offset);
}

uint32_t sbking_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(rgb_t::black(), cliprect);

	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);

	return 0;
}


void sbking_state::main_program_map(address_map &map)
{
	map(0x0000, 0x1fff).rom();
	map(0x2000, 0x2000).w("crtc", FUNC(mc6845_device::address_w));
	map(0x2001, 0x2001).w("crtc", FUNC(mc6845_device::register_w));
	map(0x2800, 0x2bff).ram();
	map(0x3000, 0x33ff).ram().w(FUNC(sbking_state::tileram_w)).share(m_tileram);
	map(0x3400, 0x37ff).ram().w(FUNC(sbking_state::attrram_w)).share(m_attrram);
	map(0x3800, 0x3803).rw("ppi0", FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0x3a00, 0x3a03).rw("ppi1", FUNC(i8255_device::read), FUNC(i8255_device::write));
}

void sbking_state::audio_program_map(address_map &map)
{
	map(0x0000, 0x07ff).rom();
	map(0x1000, 0x10ff).ram();
}

void sbking_state::audio_io_map(address_map &map)
{
	map.global_mask(0xff);

	map(0x40, 0x40).rw("ay", FUNC(ay8910_device::data_r), FUNC(ay8910_device::data_w));
	map(0x41, 0x41).w("ay", FUNC(ay8910_device::address_w));
}


static INPUT_PORTS_START(sbking)
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW1")
	PORT_DIPUNKNOWN_DIPLOC(0x01, 0x01, "SW1:1")
	PORT_DIPUNKNOWN_DIPLOC(0x02, 0x02, "SW1:2")
	PORT_DIPUNKNOWN_DIPLOC(0x04, 0x04, "SW1:3")
	PORT_DIPUNKNOWN_DIPLOC(0x08, 0x08, "SW1:4")
	PORT_DIPUNKNOWN_DIPLOC(0x10, 0x10, "SW1:5")
	PORT_DIPUNKNOWN_DIPLOC(0x20, 0x20, "SW1:6")
	PORT_DIPUNKNOWN_DIPLOC(0x40, 0x40, "SW1:7")
	PORT_DIPUNKNOWN_DIPLOC(0x80, 0x80, "SW1:8")
INPUT_PORTS_END


const gfx_layout gfx_8x8x2 =
{
	8,8,
	RGN_FRAC(1,2),
	2,
	{ RGN_FRAC(1,2), RGN_FRAC(0,2) },
	{ STEP8(7,-1) },
	{ STEP8(0,8) },
	8*8
};

static GFXDECODE_START( gfx_sbking )
	GFXDECODE_ENTRY( "tiles", 0, gfx_8x8x2, 0, 8 )
GFXDECODE_END


void sbking_state::sbking(machine_config &config)
{
	Z80(config, m_maincpu, 6_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &sbking_state::main_program_map);
	m_maincpu->set_vblank_int("screen", FUNC(sbking_state::irq0_line_hold));

	z80_device &audiocpu(Z80(config, "audiocpu", 3.579545_MHz_XTAL));
	audiocpu.set_addrmap(AS_PROGRAM, &sbking_state::audio_program_map);
	audiocpu.set_addrmap(AS_IO, &sbking_state::audio_io_map);
	audiocpu.set_periodic_int(FUNC(sbking_state::irq0_line_hold), attotime::from_hz(60*4)); // guess. TODO: identify where this comes from

	i8255_device &ppi0(I8255(config, "ppi0")); // 0x9a, port A and B in, port C 0-3 out, 4-7 in
	ppi0.in_pa_callback().set([this] () { logerror("%s PPI0 port A read\n", machine().describe_context()); return 0xff; });
	ppi0.in_pb_callback().set([this] () { logerror("%s PPI0 port B read\n", machine().describe_context()); return 0xff; });
	ppi0.in_pc_callback().set([this] () { logerror("%s PPI0 port C read\n", machine().describe_context()); return 0x70 | (machine().rand() & 0x80); }); // TODO: remove hack
	ppi0.out_pc_callback().set([this] (uint8_t data) { logerror("%s PPI0 port C write: %02x\n", machine().describe_context(), data); });

	i8255_device &ppi1(I8255(config, "ppi1")); // 0x80, all outs
	ppi1.out_pa_callback().set([this] (uint8_t data) { logerror("%s PPI1 port A write: %02x\n", machine().describe_context(), data); });
	ppi1.out_pb_callback().set([this] (uint8_t data) { logerror("%s PPI1 port B write: %02x\n", machine().describe_context(), data); });
	ppi1.out_pc_callback().set([this] (uint8_t data) { logerror("%s PPI1 port C write: %02x\n", machine().describe_context(), data); });

	// TODO: everything
	screen_device &screen(SCREEN(config, "screen"));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(64*8, 32*8);
	screen.set_visarea(0*8, 32*8-1, 2*8, 30*8-1);
	screen.set_screen_update(FUNC(sbking_state::screen_update));

	GFXDECODE(config, m_gfxdecode, "palette", gfx_sbking);

	mc6845_device &crtc(MC6845(config, "crtc", 6_MHz_XTAL)); // TODO: exact chip model is unknown
	crtc.set_screen("screen");
	crtc.set_show_border_area(false);
	crtc.set_char_width(8);

	PALETTE(config, "palette", FUNC(sbking_state::palette_init_cb), 0x20);

	SPEAKER(config, "mono").front_center();

	ay8910_device &ay(AY8910(config, "ay", 3.579545_MHz_XTAL / 2)); // TODO: exact chip model is unknown, divider not verified
	ay.port_a_read_callback().set([this] () { logerror("%s AY port A read\n", machine().describe_context()); return 0x00; });
	ay.port_b_read_callback().set([this] () { logerror("%s AY port B read\n", machine().describe_context()); return 0x00; });
	ay.add_route(ALL_OUTPUTS, "mono", 1.0);
}


ROM_START( sbking )
	ROM_REGION( 0x2000, "maincpu", 0 )
	ROM_LOAD( "05bk_1.a1", 0x0000, 0x0800, CRC(731453d1) SHA1(d0e05dcbacba91a76ec179377d74b286a3806c6a) )
	ROM_LOAD( "05bk_2.b1", 0x0800, 0x0800, CRC(ddfedcb8) SHA1(ffc4edbce5bcb3aad7ce5b20fd71f70145786c46) )
	ROM_LOAD( "05bk_3.c1", 0x1000, 0x0800, CRC(75a01954) SHA1(1aa946930e5127c6196ad8d443005d97af80a2ab) )
	ROM_LOAD( "05bk_4.d1", 0x1800, 0x0800, CRC(01d5c23e) SHA1(f965ed21805b8263525b779d1bf53f42e6cae7ac) )

	ROM_REGION( 0x800, "audiocpu", 0 )
	ROM_LOAD( "5tk_10.c1", 0x000, 0x800, CRC(d7691364) SHA1(1a21de7af7c8eac442c3663d9e433b08e6c6460a) )

	ROM_REGION( 0x2000, "tiles", 0 )
	ROM_LOAD( "05bk_5.a6", 0x0000, 0x0800, CRC(195919c4) SHA1(37bca8d1adda5c4b8e0737b3d4990b94f01c0dc3) )
	ROM_LOAD( "05bk_6.b6", 0x0800, 0x0800, CRC(a9ae3d21) SHA1(92884c2c382d4e5f3aed052c0bc111403d251e4c) )
	ROM_LOAD( "05bk_7.c6", 0x1000, 0x0800, CRC(866b185c) SHA1(e14df087799f6e9224c697438e1021cfc3026956) )
	ROM_LOAD( "05bk_8.d6", 0x1800, 0x0800, CRC(58d7399f) SHA1(3cbf259a0fcf24f1b50ebd293610e1b7cd775233) )

	ROM_REGION( 0x800, "tiles2", 0 )
	ROM_LOAD( "05bk_9.e6", 0x000, 0x800, CRC(aaee2921) SHA1(0f6718f36040c0392d8d5924b14c17a3155ddf5d) ) // 01xxxx11xxx = 0xFF

	ROM_REGION( 0x20, "proms", 0 )
	ROM_LOAD( "prom.d7", 0x00, 0x20, CRC(282b6b75) SHA1(866db9b0f9bee327f57ce38336ca45ab013b8e27) ) // read as 82S123
ROM_END

} // anonymous namespace


GAME( 198?, sbking, 0, sbking, sbking, sbking_state, empty_init, ROT0, "Tazmi", "Super Bonus King", MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
