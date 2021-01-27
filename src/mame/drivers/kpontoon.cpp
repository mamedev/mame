// license:BSD-3-Clause
// copyright-holders: R. Belmont

/*
    Pontoon (GS270) (c) 1993 Konami

    PWB353261A

    2 Z84C0008PEC
    1 053246A
    1 053247A
    1 054539
    1 053252
    3 8-dip banks
    3 XTALs (1 21.05300MHz and 2 18.43200MHz)

    TTL char readback:
    C000-D000 window, control bit 7 enables ROM readback, bit 6 selects which ROM, F450 banks
*/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "machine/k053252.h"
#include "sound/k054539.h"
#include "video/k053246_k053247_k055673.h"
#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"

namespace {

class kpontoon_state : public driver_device
{
public:
	kpontoon_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_mainbank(*this, "mainbank"),
		m_ttl_vram(*this, "ttl_vram"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_charview(*this, "charview"),
		m_charrom(*this, "chars")
	{ }

	void kpontoon(machine_config &config);

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;

private:
	tilemap_t *m_ttl_tilemap;

	void main_map(address_map &map);
	void sound_map(address_map &map);

	required_device<cpu_device> m_maincpu;
	required_memory_bank m_mainbank;
	required_shared_ptr<u8> m_ttl_vram;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	memory_view m_charview;
	required_region_ptr<u8> m_charrom;

	void control_w(u8 data);
	void charrom_bank_w(u8 data);
	u8 charrom_r(offs_t offset);
	void ttl_ram_w(offs_t offset, u8 data);

	TILE_GET_INFO_MEMBER(ttl_get_tile_info);

	u32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	u8 m_control;
	u8 m_charrom_bank;
};

void kpontoon_state::control_w(u8 data)
{
/*
    bit 7 = 1 to enable char ROM readback at C000-D000, 0 for char VRAM
    bit 6 = ?
    bit 5 = char ROM select for readback
    bit 4 = ?
    bit 3 = ?
    bit 2 = ?
    bit 0,1 = program ROM bank
*/

	m_mainbank->set_entry(data & 0x03);
	m_charview.select(BIT(data, 7));
	m_control = data;
}

void kpontoon_state::charrom_bank_w(u8 data)
{
	m_charrom_bank = data;
}

u8 kpontoon_state::charrom_r(offs_t offset)
{
	offs_t loc = ((0x1000 * m_charrom_bank) + offset) << 1;

	// ROMs are byte interleaved, bit 6 selects which ROM we're reading
	if (!BIT(m_control, 5))
	{
		loc += 1;
	}

	return m_charrom[loc];
}

void kpontoon_state::main_map(address_map &map)
{
	map(0x0000, 0x7fff).rom().region("maincpu", 0);
	map(0x8000, 0x9fff).bankr(m_mainbank);
	map(0xa000, 0xafff).ram();
	map(0xb000, 0xbfff).ram(); // work RAM (stack is here)

	map(0x0000, 0xffff).view(m_charview);
	m_charview[0](0xc000, 0xcfff).ram().share("ttl_vram").w(FUNC(kpontoon_state::ttl_ram_w));
	m_charview[1](0xc000, 0xcfff).r(FUNC(kpontoon_state::charrom_r));

	map(0xe000, 0xefff).ram();
	map(0xf000, 0xf1ff).ram().w(m_palette, FUNC(palette_device::write8)).share("palette");
	map(0xf450, 0xf450).w(FUNC(kpontoon_state::charrom_bank_w));
	map(0xf470, 0xf470).nopw(); // watchdog
	map(0xf830, 0xf830).w(FUNC(kpontoon_state::control_w));
}

void kpontoon_state::sound_map(address_map &map)
{
	map(0x0000, 0x7fff).rom().region("audiocpu", 0);
}

static INPUT_PORTS_START( kpontoon )
	PORT_START("IN0")
	PORT_BIT( 0xffff, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN1")
	PORT_BIT( 0xffff, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW1")
	PORT_DIPUNKNOWN_DIPLOC(0x01, 0x01, "DSW1:1")
	PORT_DIPUNKNOWN_DIPLOC(0x02, 0x02, "DSW1:2")
	PORT_DIPUNKNOWN_DIPLOC(0x04, 0x04, "DSW1:3")
	PORT_DIPUNKNOWN_DIPLOC(0x08, 0x08, "DSW1:4")
	PORT_DIPUNKNOWN_DIPLOC(0x10, 0x10, "DSW1:5")
	PORT_DIPUNKNOWN_DIPLOC(0x20, 0x20, "DSW1:6")
	PORT_DIPUNKNOWN_DIPLOC(0x40, 0x40, "DSW1:7")
	PORT_DIPUNKNOWN_DIPLOC(0x80, 0x80, "DSW1:8")

	PORT_START("DSW2")
	PORT_DIPUNKNOWN_DIPLOC(0x01, 0x01, "DSW2:1")
	PORT_DIPUNKNOWN_DIPLOC(0x02, 0x02, "DSW2:2")
	PORT_DIPUNKNOWN_DIPLOC(0x04, 0x04, "DSW2:3")
	PORT_DIPUNKNOWN_DIPLOC(0x08, 0x08, "DSW2:4")
	PORT_DIPUNKNOWN_DIPLOC(0x10, 0x10, "DSW2:5")
	PORT_DIPUNKNOWN_DIPLOC(0x20, 0x20, "DSW2:6")
	PORT_DIPUNKNOWN_DIPLOC(0x40, 0x40, "DSW2:7")
	PORT_DIPUNKNOWN_DIPLOC(0x80, 0x80, "DSW2:8")

	PORT_START("DSW3")
	PORT_DIPUNKNOWN_DIPLOC(0x01, 0x01, "DSW3:1")
	PORT_DIPUNKNOWN_DIPLOC(0x02, 0x02, "DSW3:2")
	PORT_DIPUNKNOWN_DIPLOC(0x04, 0x04, "DSW3:3")
	PORT_DIPUNKNOWN_DIPLOC(0x08, 0x08, "DSW3:4")
	PORT_DIPUNKNOWN_DIPLOC(0x10, 0x10, "DSW3:5")
	PORT_DIPUNKNOWN_DIPLOC(0x20, 0x20, "DSW3:6")
	PORT_DIPUNKNOWN_DIPLOC(0x40, 0x40, "DSW3:7")
	PORT_DIPUNKNOWN_DIPLOC(0x80, 0x80, "DSW3:8")
INPUT_PORTS_END


void kpontoon_state::machine_start()
{
	m_mainbank->configure_entries(0, 4, memregion("maincpu")->base() + 0x8000, 0x2000);
	m_mainbank->set_entry(0);

	m_ttl_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(kpontoon_state::ttl_get_tile_info)), TILEMAP_SCAN_ROWS, 16, 16, 64, 32);
	m_ttl_tilemap->set_transparent_pen(0);
}

void kpontoon_state::machine_reset()
{
	m_charview.select(0);
}

void kpontoon_state::ttl_ram_w(offs_t offset, u8 data)
{
	u8 *vram = (u8 *)m_ttl_vram.target();
	vram[offset] = data;
	m_ttl_tilemap->mark_tile_dirty(offset / 2);
}

TILE_GET_INFO_MEMBER(kpontoon_state::ttl_get_tile_info)
{
	u8 *vram = (u8 *)m_ttl_vram.target();
	int attr, code;

	attr = vram[(tile_index << 1) + 1];
	code = vram[(tile_index << 1)];

	tileinfo.set(0, code, attr, 0);
}

uint32_t kpontoon_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(0, cliprect);
	m_ttl_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	return 0;
}

static GFXDECODE_START( gfx_pontoon )
	GFXDECODE_ENTRY( "chars", 0, gfx_16x16x4_packed_msb, 0, 1 ) // TODO: looks decent, but needs verifying
GFXDECODE_END

void kpontoon_state::kpontoon(machine_config &config)
{
	// basic machine hardware
	Z80(config, m_maincpu, 21'053'000 / 4); // clock unverified
	m_maincpu->set_addrmap(AS_PROGRAM, &kpontoon_state::main_map);

	z80_device &audiocpu(Z80(config, "audiocpu", XTAL(18.432_MHz_XTAL) / 4)); // clock unverified
	audiocpu.set_addrmap(AS_PROGRAM, &kpontoon_state::sound_map);
	audiocpu.set_disable();

	K053252(config, "k053252", 21'053'000 / 4); // clock unverified

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER)); // TODO: all wrong
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500));
	screen.set_size(128*8, 64*8);
	screen.set_visarea(26*8, 90*8-1, 24, 56*8-1);
	screen.set_screen_update(FUNC(kpontoon_state::screen_update));
	screen.set_palette("palette");

	PALETTE(config, m_palette).set_format(palette_device::xBGR_555, 256); // TODO: all wrong
	GFXDECODE(config, m_gfxdecode, "palette", gfx_pontoon);

	// sound hardware
	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();

	k053247_device &k053246(K053246(config, "k053246", 0));
	//k053246.set_sprite_callback(FUNC(kpontoon_state::sprite_callback));
	k053246.set_config(NORMAL_PLANE_ORDER, 0, 0); // TODO: verify
	k053246.set_palette(m_palette);

	k054539_device &k054539(K054539(config, "k054539", 18.432_MHz_XTAL));
	k054539.add_route(0, "rspeaker", 0.75);
	k054539.add_route(1, "lspeaker", 0.75);
}


ROM_START( kpontoon )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "270eac04.bin", 0x00000, 0x10000, CRC(614e35bd) SHA1(50a7740f0442949d9c49209b04589296d065af52) )

	ROM_REGION( 0x10000, "audiocpu", 0 ) // really?
	ROM_LOAD( "270b01.bin", 0x00000, 0x10000, CRC(012f542e) SHA1(b83f53e5297ec0c2f742701f694149d1e466648e) )

	ROM_REGION( 0x80000, "k053246", 0 ) // TODO: correct ROM loading
	ROM_LOAD64_BYTE( "270ea07.bin", 0x00007, 0x10000, CRC(27e791fa) SHA1(fcafe9fd74ab729a90e4a36b25dbc4ce050b68ba) )
	ROM_LOAD64_BYTE( "270ea08.bin", 0x00006, 0x10000, CRC(aa1474cc) SHA1(b935ab41daeaace62a90885c0d29f49df67ca3a3) )
	ROM_LOAD64_BYTE( "270ea09.bin", 0x00005, 0x10000, CRC(0462f9ea) SHA1(c16f6b8ef8b04fcc2b11783e3a5e553a45fafb84) )
	ROM_LOAD64_BYTE( "270ea10.bin", 0x00004, 0x10000, CRC(95dc83df) SHA1(0b27f7d801def2e3fde2fc0c4e130017cfa867bf) )
	ROM_LOAD64_BYTE( "270ea11.bin", 0x00003, 0x10000, CRC(5e0f83e0) SHA1(79c6119d488f8515419e6422a686dcc8363ce9c5) )
	ROM_LOAD64_BYTE( "270ea12.bin", 0x00002, 0x10000, CRC(553fcee0) SHA1(7330b223a71966aaa3216c9e58bdc090c8d402b0) )
	ROM_LOAD64_BYTE( "270ea13.bin", 0x00001, 0x10000, CRC(ead4a114) SHA1(ddce7c4c4606cedcc05c1d2e5fdeda5d1358c97e) )
	ROM_LOAD64_BYTE( "270ea14.bin", 0x00000, 0x10000, CRC(ac6b06c2) SHA1(5584b1d1047f0e447dd20252eb0ec3089bd228fe) )

	ROM_REGION( 0x40000, "chars", 0 )
	ROM_LOAD16_BYTE( "270ea05.bin", 0x00000, 0x20000, CRC(d08e9a8b) SHA1(d333fa4f89907dd223e7a7d6c57a7fc78e61ee2d) )
	ROM_LOAD16_BYTE( "270ea06.bin", 0x00001, 0x20000, CRC(11eccb4b) SHA1(b943aa492819aa0563fc5295a890cd59c4b8aa4c) )

	ROM_REGION( 0x80000, "k054539", 0 )
	ROM_LOAD( "270a02.bin", 0x00000, 0x80000, CRC(a1a75c87) SHA1(6ae459ab67b24ace06657b7e4be7803cd35f4bd3) )
ROM_END

} // Anonymous namespace


GAME( 1993, kpontoon,  0, kpontoon, kpontoon, kpontoon_state, empty_init, ROT0, "Konami", "Pontoon (Konami)", MACHINE_IS_SKELETON )
