// license:BSD-3-Clause
// copyright-holders:

/*
Boram Z80-based poker games

The 2 dumped games come from 2 similar PCBs:
PK uses the ATPK-BORAM 0211 PCB, while Turbo PK uses the ATPK-BORAM 0300 III PCB.
Main components are:
Z80A CPU (different variants)
HD46505SP CRT
I8255 PPI
4 MHz XTAL
13 MHz XTAL
AY-8910 sound chip
on 0211 PCB: 2x 8-DIP banks
on 0300 III PCB: 4x 8-DIP banks
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

class boramz80_state : public driver_device
{
public:
	boramz80_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_char_ram(*this, "char_ram"),
		m_tile_ram(*this, "tile_ram")
	{ }

	void pk(machine_config &config) ATTR_COLD;

	void init_tpkborama();

protected:
	virtual void video_start() override ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;

	required_shared_ptr<uint8_t> m_char_ram;
	required_shared_ptr<uint8_t> m_tile_ram;

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	void program_map(address_map &map) ATTR_COLD;
	void io_map(address_map &map) ATTR_COLD;
};


void boramz80_state::video_start()
{
}

uint32_t boramz80_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(rgb_t::black(), cliprect);

	return 0;
}


void boramz80_state::program_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0x8fff).ram(); // TODO: only 0x800 for pkboram
	map(0xa000, 0xa7ff).ram().share(m_char_ram);
	map(0xc000, 0xc7ff).ram().share(m_tile_ram);
	map(0xe000, 0xe3ff).ram().w("palette", FUNC(palette_device::write8)).share("palette");
	map(0xf000, 0xf3ff).ram().w("palette", FUNC(palette_device::write8_ext)).share("palette_ext");
}

void boramz80_state::io_map(address_map &map)
{
	map.global_mask(0xff);

	map(0x00, 0x03).rw("ppi", FUNC(i8255_device::read), FUNC(i8255_device::write));
	//map(0x20, 0x20).r();
	//map(0x40, 0x40).r();
	//map(0x60, 0x60).r();
	map(0x80, 0x81).w("aysnd", FUNC(ay8910_device::data_address_w));
	map(0x81, 0x81).r("aysnd", FUNC(ay8910_device::data_r));
	//map(0xa0, 0xa0).r();
	map(0xc0, 0xc0).w("crtc", FUNC(mc6845_device::address_w));
	map(0xc1, 0xc1).w("crtc", FUNC(mc6845_device::register_w));
}


static INPUT_PORTS_START( pkboram )
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

	PORT_START("IN2")
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

	PORT_START("DSW2")
	PORT_DIPUNKNOWN_DIPLOC(0x01, 0x01, "SW2:1")
	PORT_DIPUNKNOWN_DIPLOC(0x02, 0x02, "SW2:2")
	PORT_DIPUNKNOWN_DIPLOC(0x04, 0x04, "SW2:3")
	PORT_DIPUNKNOWN_DIPLOC(0x08, 0x08, "SW2:4")
	PORT_DIPUNKNOWN_DIPLOC(0x10, 0x10, "SW2:5")
	PORT_DIPUNKNOWN_DIPLOC(0x20, 0x20, "SW2:6")
	PORT_DIPUNKNOWN_DIPLOC(0x40, 0x40, "SW2:7")
	PORT_DIPUNKNOWN_DIPLOC(0x80, 0x80, "SW2:8")
INPUT_PORTS_END

static INPUT_PORTS_START( tpkboram )
	PORT_INCLUDE( pkboram )

	PORT_START("DSW3")
	PORT_DIPUNKNOWN_DIPLOC(0x01, 0x01, "SW3:1")
	PORT_DIPUNKNOWN_DIPLOC(0x02, 0x02, "SW3:2")
	PORT_DIPUNKNOWN_DIPLOC(0x04, 0x04, "SW3:3")
	PORT_DIPUNKNOWN_DIPLOC(0x08, 0x08, "SW3:4")
	PORT_DIPUNKNOWN_DIPLOC(0x10, 0x10, "SW3:5")
	PORT_DIPUNKNOWN_DIPLOC(0x20, 0x20, "SW3:6")
	PORT_DIPUNKNOWN_DIPLOC(0x40, 0x40, "SW3:7")
	PORT_DIPUNKNOWN_DIPLOC(0x80, 0x80, "SW3:8")

	PORT_START("DSW4")
	PORT_DIPUNKNOWN_DIPLOC(0x01, 0x01, "SW4:1")
	PORT_DIPUNKNOWN_DIPLOC(0x02, 0x02, "SW4:2")
	PORT_DIPUNKNOWN_DIPLOC(0x04, 0x04, "SW4:3")
	PORT_DIPUNKNOWN_DIPLOC(0x08, 0x08, "SW4:4")
	PORT_DIPUNKNOWN_DIPLOC(0x10, 0x10, "SW4:5")
	PORT_DIPUNKNOWN_DIPLOC(0x20, 0x20, "SW4:6")
	PORT_DIPUNKNOWN_DIPLOC(0x40, 0x40, "SW4:7")
	PORT_DIPUNKNOWN_DIPLOC(0x80, 0x80, "SW4:8")
INPUT_PORTS_END


static GFXDECODE_START( gfx_boram )
	GFXDECODE_ENTRY( "chars", 0, gfx_8x8x2_planar,  0, 16 )
	GFXDECODE_ENTRY( "tiles", 0, gfx_8x8x4_planar,  0, 16 ) // probably wrong
GFXDECODE_END


void boramz80_state::pk(machine_config &config)
{
	// basic machine hardware
	Z80(config, m_maincpu, 4_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &boramz80_state::program_map);
	m_maincpu->set_addrmap(AS_IO, &boramz80_state::io_map);
	//m_maincpu->set_vblank_int("screen", FUNC(boramz80_state::irq0_line_hold));

	i8255_device &ppi(I8255A(config, "ppi"));
	ppi.in_pa_callback().set([this] () { logerror("%s: PPI port A read\n", machine().describe_context()); return ioport("IN0")->read(); });
	ppi.in_pb_callback().set([this] () { logerror("%s: PPI port B read\n", machine().describe_context()); return ioport("IN1")->read(); });
	ppi.in_pc_callback().set([this] () { logerror("%s: PPI port C read\n", machine().describe_context()); return ioport("IN2")->read(); });
	ppi.out_pc_callback().set([this] (uint8_t data) { logerror("%s: PPI port C write %02x\n", machine().describe_context(), data); });

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER)); // TODO: everything
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(32*8, 32*8);
	screen.set_visarea_full();
	screen.set_screen_update(FUNC(boramz80_state::screen_update));

	hd6845s_device &crtc(HD6845S(config, "crtc", 13_MHz_XTAL / 16));  // divisor guessed
	crtc.set_screen("screen");
	crtc.set_show_border_area(false);
	crtc.set_char_width(8);

	GFXDECODE(config, "gfxdecode", "palette", gfx_boram);
	PALETTE(config, "palette").set_format(palette_device::xRGB_555, 0x400); // TODO: verify once it works

	SPEAKER(config, "mono").front_center();

	ay8910_device &aysnd(AY8910(config, "aysnd", 4_MHz_XTAL / 4)); // not sure, could derive from 13 MHz XTAL
	aysnd.port_a_read_callback().set_ioport("DSW1"); // TODO: verify once it works
	aysnd.port_b_read_callback().set_ioport("DSW2"); // TODO: verify once it works
	aysnd.port_a_write_callback().set([this] (uint8_t data) { logerror("%s: AY port A write %02x\n", machine().describe_context(), data); });
	aysnd.add_route(ALL_OUTPUTS, "mono", 0.50);
}


ROM_START( pkboram )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "b.rom", 0x0000, 0x8000, CRC(5f38640d) SHA1(914cbd3c5e0406e2daa9bdad6bd46758498aabb5) )

	ROM_REGION( 0x4000, "chars", 0 )
	ROM_LOAD( "1.cg1", 0x0000, 0x2000, CRC(a5c43569) SHA1(17a5d529ee2ef18019dabf9aefcf595d1193c7d0) )
	ROM_LOAD( "2.cg2", 0x2000, 0x2000, CRC(68906bae) SHA1(4ccec70f4d6044a7e23e4e50c98916278fe7dfd0) )

	ROM_REGION( 0x40000, "tiles", 0 )
	ROM_LOAD( "3.pg1",  0x00000, 0x8000, CRC(5bd66a9b) SHA1(52fcda1e818c19910b88956ed28a479b6d5f3385) )
	ROM_LOAD( "4.pg2",  0x08000, 0x8000, CRC(a8832475) SHA1(f0ec6cd74992cd6f27c12e0c6da6aaabdb8b2e52) )
	ROM_LOAD( "5.pg3",  0x10000, 0x8000, CRC(bf00fe98) SHA1(218103db220d96c6f16e685f48df8a63443d24f7) )
	ROM_LOAD( "6.pg4",  0x18000, 0x8000, CRC(5fe7b018) SHA1(26a41f96a4b4722b73dbee08cfa1272aa6d83ca8) )
	ROM_LOAD( "7.pg5",  0x20000, 0x8000, CRC(c19d09bf) SHA1(124169b22e566b2a44ae1d0ae1259cdb188e8769) )
	ROM_LOAD( "8.pg6",  0x28000, 0x8000, CRC(191d2ab3) SHA1(ad8bfc3f28ccf503cf388791634f32f745559c3c) )
	ROM_LOAD( "9.pg7",  0x30000, 0x8000, CRC(fd182a3a) SHA1(0d7e9e905b33fd6925962d6992c595830a35ac26) )
	ROM_LOAD( "10.pg8", 0x38000, 0x8000, CRC(7c2e9f86) SHA1(b82efdd718fa49cb57330fdcf05df6a9e025a822) )
ROM_END

ROM_START( tpkboram )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "pkt-21spko.rom", 0x0000, 0x8000, CRC(a024d82b) SHA1(4d656261747930415807cd084536ef145fbf0f5b) )

	ROM_REGION( 0x4000, "chars", 0 )
	ROM_LOAD( "1.cg1", 0x0000, 0x2000, CRC(69f44d04) SHA1(2f98805e4b70ce3426078f35ff260a3bc97fab86) )
	ROM_LOAD( "2.cg2", 0x2000, 0x2000, CRC(c1adf009) SHA1(0d5d8b39d40c807b9b5ed7418ba871c4d683286a) )

	ROM_REGION( 0x40000, "tiles", 0 )
	ROM_LOAD( "3.pg1",  0x00000, 0x8000, CRC(5506285c) SHA1(017095d0c293b8a5ae73e40a4e5f662d8ba01a06) )
	ROM_LOAD( "4.pg2",  0x08000, 0x8000, CRC(6d62a734) SHA1(42716934bc93f3c815af961a6efbae120bec2793) )
	ROM_LOAD( "5.pg3",  0x10000, 0x8000, CRC(644c44a6) SHA1(a407735ccdefc4ff7a6f2f007b9e1c4846202dfe) )
	ROM_LOAD( "6.pg4",  0x18000, 0x8000, CRC(029cc0d1) SHA1(d41ec3fa38c1729fee3026f5c9365175738ecc99) )
	ROM_LOAD( "7.pg5",  0x20000, 0x8000, CRC(c07a3cac) SHA1(19c1f996494cf0b200c7d781ba3ffd4af9bfe73b) )
	ROM_LOAD( "8.pg6",  0x28000, 0x8000, CRC(8f2a8c3e) SHA1(5ec031dc1fa21a09c1a4ebc0b6bb5f899038801a) )
	ROM_LOAD( "9.pg7",  0x30000, 0x8000, CRC(7dbbdeb5) SHA1(4d379b9e0c825174bf151117e3550809948e1763) )
	ROM_LOAD( "10.pg8", 0x38000, 0x8000, CRC(4a293afa) SHA1(be532e6a476f78638e7f558bf8093e1914bc3688) )
ROM_END

// this runs on a newer ATPK-BORAM PK-0500 PCB. Given all GFX match tpkboram, it's probably a newer revision.
// code is encrypted
ROM_START( tpkborama )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "223.rom", 0x0000, 0x8000, CRC(1d776d37) SHA1(6918cddb0b47d28cf8145823f869dfd2296c0eed) )

	ROM_REGION( 0x4000, "chars", 0 ) // these are same as tpkboram
	ROM_LOAD( "1.cg1", 0x0000, 0x2000, CRC(69f44d04) SHA1(2f98805e4b70ce3426078f35ff260a3bc97fab86) )
	ROM_LOAD( "2.cg2", 0x2000, 0x2000, CRC(c1adf009) SHA1(0d5d8b39d40c807b9b5ed7418ba871c4d683286a) )

	ROM_REGION( 0x40000, "tiles", 0 ) // these are all 1st and 2nd half identical, but same as tpkboram if split
	ROM_LOAD( "3.pg1",  0x00000, 0x8000, CRC(612c5b39) SHA1(9682167b1fbbcd34b71c2628641b646a2993f61b) )
	ROM_IGNORE(                  0x8000 )
	ROM_LOAD( "4.pg2",  0x08000, 0x8000, CRC(14ee6437) SHA1(a046b3efb14a400d201f7ce1c3ee0e01badb46a6) )
	ROM_IGNORE(                  0x8000 )
	ROM_LOAD( "5.pg3",  0x10000, 0x8000, CRC(ce87f0c5) SHA1(96379856182bb0c81c805906551ec2e4aa2eb1d5) )
	ROM_IGNORE(                  0x8000 )
	ROM_LOAD( "6.pg4",  0x18000, 0x8000, CRC(0a8a6106) SHA1(ac88f1ef2eb39cd24a236b2f18e85367c0736ae8) )
	ROM_IGNORE(                  0x8000 )
	ROM_LOAD( "7.pg5",  0x20000, 0x8000, CRC(484a0eec) SHA1(6e32da2d4d78fb4c4bae2d2da945a71231051d5f) )
	ROM_IGNORE(                  0x8000 )
	ROM_LOAD( "8.pg6",  0x28000, 0x8000, CRC(772d8996) SHA1(bd0412d0656a26a80b0f00ff5d6bcff2c4adb6c7) )
	ROM_IGNORE(                  0x8000 )
	ROM_LOAD( "9.pg7",  0x30000, 0x8000, CRC(ff052a99) SHA1(7523ab2eeef1e44107710c8a68897daa7bf2ce12) )
	ROM_IGNORE(                  0x8000 )
	ROM_LOAD( "10.pg8", 0x38000, 0x8000, CRC(61a4e0f3) SHA1(8d9f0efd3b691eaf93c933c63ba6aa34ebad71b1) )
	ROM_IGNORE(                  0x8000 )
ROM_END


void boramz80_state::init_tpkborama()
{
	uint8_t *rom = memregion("maincpu")->base();

	for (int i = 0; i < 0x8000; i++)
	{
		// TODO
		rom[i] = rom[i];
	}
}

} // anonymous namespace


GAME( 1987, pkboram,   0,        pk, pkboram,  boramz80_state, empty_init,     ROT0, "Boram", "PK - New Exciting Poker!",        MACHINE_IS_SKELETON ) // PK-BORAM 0211 aug.04.1987. BORAM CORP
GAME( 1988, tpkboram,  0,        pk, tpkboram, boramz80_state, empty_init,     ROT0, "Boram", "Turbo PK",                        MACHINE_IS_SKELETON ) // PK-TURBO jan.29.1988. BORAM CORP.
GAME( 1998, tpkborama, tpkboram, pk, tpkboram, boramz80_state, init_tpkborama, ROT0, "Boram", "Turbo PK (Ver 2.3B2, encrypted)", MACHINE_IS_SKELETON ) // dep inctype-23B1998 0519Ver 2.3B2
