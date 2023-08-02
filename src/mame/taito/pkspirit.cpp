// license:BSD-3-Clause
// copyright-holders:

/*
    Poker Spirit (c) 1992 Taito
    2 PCBs: Base PCB (stickered K11J0730A Poker Spirit) + video PCB (stickered K11X0731A Poker Spirit)

    Main components:

    Base PCB:
    1 x TMP68303F-16
    2 x 84256A-10L CMOS 256K-bit low power SRAM
    1 x TE7750 I/O expander
    1 x M66011 serial bus controller
    1 x 93C46 EEPROM
    1 x 32.000MHz Osc (near the TMP)
    2 x 8-dip banks

    Video PCB:
    1 x TC0470LIN video custom
    1 x TC0600OBT video custom
    1 x TC0650FDA video custom
    1 x Z8400BB1
    1 x OKI M6295
    1 x YM2203C
    1 x PC060HA CIU
    1 x 36.000MHz Osc (near the Z80)
*/

#include "emu.h"

#include "taitosnd.h"

#include "cpu/m68000/tmp68301.h"
#include "cpu/z80/z80.h"
#include "machine/te7750.h"
#include "sound/okim6295.h"
#include "sound/ymopn.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"


namespace {

class pkspirit_state : public driver_device
{
public:
	pkspirit_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_bg_videoram(*this, "bg_videoram"),
		m_fg_videoram(*this, "fg_videoram")
	{ }

	void pkspirit(machine_config &config);

protected:
	virtual void video_start() override;

private:
	required_device<tmp68301_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;

	required_shared_ptr<uint16_t> m_bg_videoram;
	required_shared_ptr<uint16_t> m_fg_videoram;

	tilemap_t *m_bg_tilemap = nullptr;
	tilemap_t *m_fg_tilemap = nullptr;

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void bg_videoram_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void fg_videoram_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	TILE_GET_INFO_MEMBER(get_fg_tile_info);

	void main_map(address_map &map);
	void sound_map(address_map &map);
};

void pkspirit_state::bg_videoram_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_bg_videoram[offset]);
	m_bg_tilemap->mark_tile_dirty(offset);
}

TILE_GET_INFO_MEMBER(pkspirit_state::get_bg_tile_info)
{
	int tileno = m_bg_videoram[tile_index];
	tileinfo.set(0, tileno, 0, 0);
}

void pkspirit_state::fg_videoram_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_fg_videoram[offset]);
	m_fg_tilemap->mark_tile_dirty(offset);
}

TILE_GET_INFO_MEMBER(pkspirit_state::get_fg_tile_info)
{
	int tileno = m_fg_videoram[tile_index];
	tileinfo.set(0, tileno, 0, 0);
}


void pkspirit_state::video_start()
{
	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(pkspirit_state::get_bg_tile_info)), TILEMAP_SCAN_ROWS, 16, 16, 64, 32);
	m_fg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(pkspirit_state::get_fg_tile_info)), TILEMAP_SCAN_ROWS, 16, 16, 64, 32);
	m_fg_tilemap->set_transparent_pen(0);
}

uint32_t pkspirit_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	m_fg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	return 0;
}


void pkspirit_state::main_map(address_map &map) // TODO: verify everything
{
	map(0x000000, 0x01ffff).rom().region("maincpu", 0);
	map(0x100000, 0x10001f).rw("te7750", FUNC(te7750_device::read), FUNC(te7750_device::write)).umask16(0x00ff); // maybe
	// map(0x200000, 0x200001).r //?
	map(0x300000, 0x30ffff).ram(); // main RAM?
	map(0x800001, 0x800001).w("ciu", FUNC(pc060ha_device::master_port_w));
	map(0x800003, 0x800003).rw("ciu", FUNC(pc060ha_device::master_comm_r), FUNC(pc060ha_device::master_comm_w));
	//map(0x900000, 0x900001).w // ?

	map(0xa00000, 0xa0003f).ram();
	map(0xa04000, 0xa057ff).ram();
	map(0xb00000, 0xb00fff).ram().w(FUNC(pkspirit_state::fg_videoram_w)).share(m_fg_videoram); // GFX chips RAM?
	map(0xb01000, 0xb01fff).ram(); // GFX chips RAM?
	map(0xb02000, 0xb02fff).ram().w(FUNC(pkspirit_state::bg_videoram_w)).share(m_bg_videoram); // GFX chips RAM?
	map(0xb03000, 0xb03fff).ram(); // GFX chips RAM ?
	map(0xb04000, 0xb04fff).ram(); // maybe sprite data, arranged as strips of 4 horizontal tiles
	map(0xb05000, 0xb0ffff).ram(); // GFX chips RAM ?

	map(0xb10800, 0xb1087f).ram(); // control registers for one of the custom GFX chips?
	map(0xb20000, 0xb2001f).ram(); // control registers for one of the custom GFX chips?
}

void pkspirit_state::sound_map(address_map &map) // TODO: verify everything
{
	map(0x0000, 0x3fff).rom().region("audiocpu", 0); // banked?
	map(0x8000, 0x8fff).ram();
	map(0xa000, 0xa000).w("ciu", FUNC(pc060ha_device::slave_port_w));
	map(0xa001, 0xa001).rw("ciu", FUNC(pc060ha_device::slave_comm_r), FUNC(pc060ha_device::slave_comm_w));
	map(0xc000, 0xc001).rw("opn", FUNC(ym2203_device::read), FUNC(ym2203_device::write));
	map(0xe000, 0xe000).rw("oki", FUNC(okim6295_device::read), FUNC(okim6295_device::write));
}


static INPUT_PORTS_START( pkspirit )
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


const gfx_layout gfx_16x16x5_planar =
{
	16,16,
	RGN_FRAC(1,5),
	5,
	{ RGN_FRAC(0,5), RGN_FRAC(1,5), RGN_FRAC(2,5), RGN_FRAC(3,5), RGN_FRAC(4,5) },
	{ STEP8(7,-1), STEP8(15,-1) },
	{ STEP16(0,16) },
	16*16
};


static GFXDECODE_START( gfx_pkspirit ) // TODO: wrong, needs adjustments
	GFXDECODE_ENTRY( "tiles", 0, gfx_16x16x5_planar, 0, 256)
GFXDECODE_END


void pkspirit_state::pkspirit(machine_config &config)
{
	// basic machine hardware
	TMP68301(config, m_maincpu, 32_MHz_XTAL / 2); // divider not verified, actually TMP68303F-16
	m_maincpu->set_addrmap(AS_PROGRAM, &pkspirit_state::main_map);
	m_maincpu->parallel_r_cb().set([this]() { logerror("%s par_r\n", machine().describe_context()); return 0xffff; });

	z80_device &audiocpu(Z80(config, "audiocpu", 36_MHz_XTAL / 9)); // divider not verified, but marked as 4MHz on PCB
	audiocpu.set_addrmap(AS_PROGRAM, &pkspirit_state::sound_map);

	TE7750(config, "te7750");

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER)); // TODO: wrong
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(64*8, 64*8);
	screen.set_visarea(0*8, 64*8-1, 0*8, 56*8-1);
	screen.set_palette("palette");
	screen.set_screen_update(FUNC(pkspirit_state::screen_update));
	screen.screen_vblank().set_inputline(m_maincpu, 1);

	GFXDECODE(config, "gfxdecode", "palette", gfx_pkspirit);
	PALETTE(config, "palette").set_format(palette_device::xRGB_888, 8192); // TODO: wrong

	// sound hardware
	SPEAKER(config, "mono").front_center();

	pc060ha_device &ciu(PC060HA(config, "ciu", 0));
	ciu.set_master_tag(m_maincpu);
	ciu.set_slave_tag("audiocpu");

	ym2203_device &opn(YM2203(config, "opn", 36_MHz_XTAL / 9)); // divider not verified
	opn.irq_handler().set_inputline("audiocpu", 0);
	//ymsnd.port_a_write_callback() TODO: writes continuously here.
	opn.add_route(ALL_OUTPUTS, "mono", 0.30);

	OKIM6295(config, "oki", 1.056_MHz_XTAL, okim6295_device::PIN7_HIGH).add_route(ALL_OUTPUTS, "mono", 0.5); // all verified
}


ROM_START( pkspirit )
	ROM_REGION( 0x40000, "maincpu", 0 ) // on base PCB
	ROM_LOAD16_BYTE( "d41_18.ic26", 0x00000, 0x20000, CRC(7bf56618) SHA1(e619325856bf39c2fbc359cf71bde73fbee54d79) ) // 1xxxxxxxxxxxxxxxx = 0xFF
	ROM_LOAD16_BYTE( "d41_17.ic25", 0x00001, 0x20000, CRC(913f9b21) SHA1(f0a29aa07d48f62d2b394874e78ca5c3d26e36a4) ) // 1xxxxxxxxxxxxxxxx = 0xFF

	ROM_REGION( 0x10000, "audiocpu", 0 ) // on video PCB
	ROM_LOAD( "d41_06.ic21", 0x00000, 0x10000, CRC(64103680) SHA1(81701348691562e296527fb8e1731de2f02d71d1) )

	ROM_REGION( 0xa0000, "tiles", 0 ) // on video PCB, TODO: correct ROM loading
	ROM_LOAD16_BYTE( "d41_07.ic1",  0x00000, 0x10000, CRC(c57b18f8) SHA1(e25f2ff8d0bf312d23b8ff09f07f63e61c3094c6) )
	ROM_LOAD16_BYTE( "d41_08.ic2",  0x00001, 0x10000, CRC(67b941dd) SHA1(cc9abb5d7f3cb90a91921097d23261d80f418287) )
	ROM_LOAD16_BYTE( "d41_10.ic6",  0x20000, 0x10000, CRC(0af0488c) SHA1(36a77b980038731b703f935a6a813bc9295b7889) )
	ROM_LOAD16_BYTE( "d41_09.ic5",  0x20001, 0x10000, CRC(199820a2) SHA1(ae3af3aa424b535fc03ef6bb99fca146bdcd88b1) )
	ROM_LOAD16_BYTE( "d41_12.ic9",  0x40000, 0x10000, CRC(7c521521) SHA1(10ac517921a08ce5387656f33bf3e45879fa614a) )
	ROM_LOAD16_BYTE( "d41_11.ic8",  0x40001, 0x10000, CRC(4913401d) SHA1(d7c92405b7c5505a6b4ac738b6d502cf99f995c6) )
	ROM_LOAD16_BYTE( "d41_15.ic15", 0x60000, 0x10000, CRC(df078e72) SHA1(b781b1ce3b87755513eefc295f00159f495359d8) )
	ROM_LOAD16_BYTE( "d41_13.ic11", 0x60001, 0x10000, CRC(78e1fc0b) SHA1(8475a97cc574971ec201840a71dbf823762f2970) )
	ROM_LOAD16_BYTE( "d41_14.ic12", 0x80000, 0x10000, CRC(3ec2fe4b) SHA1(74877d1623e2336770ff0606d6ca7d7c89a51004) )
	ROM_LOAD16_BYTE( "d41_16.ic16", 0x80001, 0x10000, CRC(6c9d169d) SHA1(e6cd2ddd6b6242e2fadbbcb4b3170dd54391b25e) )

	ROM_REGION( 0x40000, "oki", ROMREGION_ERASE00 ) // on video PCB
	// empty socket at IC38. Confirmed on 2 different PCBs

	ROM_REGION( 0x80, "eeprom", 0 ) // on base PCB
	ROM_LOAD( "93c46.ic37", 0x00, 0x80, NO_DUMP )

	ROM_REGION( 0x400, "plds", 0 )
	ROM_LOAD( "d41_05.ic13", 0x000, 0x144, NO_DUMP ) // PAL20L8B, on base PCB
	ROM_LOAD( "d41_04.ic19", 0x200, 0x144, NO_DUMP ) // PAL20L8B, on video PCB
ROM_END

} // anonymous namespace


GAME( 1992, pkspirit, 0, pkspirit, pkspirit, pkspirit_state, empty_init, ROT0, "Taito Corporation", "Poker Spirit", MACHINE_IS_SKELETON )
