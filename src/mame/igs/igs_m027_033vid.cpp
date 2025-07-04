// license:BSD-3-Clause
// copyright-holders: David Haywood

/*
IGS ARM7 (IGS027A) based mahjong / gambling platform(s),
with IGS 033 custom video chip.

Main components for the IGS PCB-0405-02-FZ are:
- IGS 027A (ARM7-based MCU)
- 24 MHz XTAL
- IGS 033 graphics chip
- 82C55 2K15 PPI
- K668 ADPCM chip (M6295 clone)
- 3 banks of 8 DIP switches

TODO:
 - IGS 033 appears to encapsulate the behavior of the video/interface chip found in igspoker.cpp
   so could be turned into a device, possibly shared
 - verify possibly incomplete inputs
*/

#include "emu.h"

#include "igs027a.h"
#include "pgmcrypt.h"

#include "machine/i8255.h"
#include "machine/nvram.h"
#include "machine/ticket.h"
#include "machine/timer.h"
#include "sound/okim6295.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"

#include "endianness.h"

#include <algorithm>


// configurable logging
#define LOG_PORTS     (1U << 1)

// #define VERBOSE (LOG_GENERAL | LOG_PORTS)

#include "logmacro.h"

#define LOGPORTS(...)     LOGMASKED(LOG_PORTS,     __VA_ARGS__)


namespace {

class igs_m027_033vid_state : public driver_device
{
public:
	igs_m027_033vid_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_external_rom(*this, "user1"),
		m_nvram(*this, "nvram"),
		m_bg_videoram(*this, "bg_videoram"),
		m_bg_attr_videoram(*this, "bg_attr_videoram"),
		m_maincpu(*this, "maincpu"),
		m_hopper(*this, "hopper"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette"),
		m_gfxdecode(*this, "gfxdecode"),
		m_oki(*this, "oki")
	{ }

	void m027_033vid(machine_config &config) ATTR_COLD;
	void huahuas5(machine_config &config) ATTR_COLD;

	void init_huahuas5() ATTR_COLD;
	void init_qiji6() ATTR_COLD;

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

private:
	required_region_ptr<u32> m_external_rom;
	required_shared_ptr<u32> m_nvram;
	required_shared_ptr<u32> m_bg_videoram;
	required_shared_ptr<u32> m_bg_attr_videoram;

	required_device<igs027a_cpu_device> m_maincpu;
	required_device<hopper_device> m_hopper;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<okim6295_device> m_oki;

	u32 m_xor_table[0x100];
	tilemap_t *m_bg_tilemap = nullptr;
	u8 m_tilebank = 0;
	u8 m_video_enable = 0;

	TIMER_DEVICE_CALLBACK_MEMBER(interrupt);

	TILE_GET_INFO_MEMBER(get_bg_tile_info);

	void bg_videoram_w(offs_t offset, u32 data, u32 mem_mask);
	void bg_attr_videoram_w(offs_t offset, u32 data, u32 mem_mask);

	u32 external_rom_r(offs_t offset);
	void xor_table_w(offs_t offset, u8 data);

	void counters_w(u8 data);
	void out_port_w(u8 data);
	u32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void m027_map(address_map &map) ATTR_COLD;
};


void igs_m027_033vid_state::machine_start()
{
	std::fill(std::begin(m_xor_table), std::end(m_xor_table), 0);

	save_item(NAME(m_xor_table));
	save_item(NAME(m_tilebank));
	save_item(NAME(m_video_enable));
}

void igs_m027_033vid_state::video_start()
{
	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(igs_m027_033vid_state::get_bg_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 64, 32);
}

u32 igs_m027_033vid_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	if (m_video_enable)
		m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);

	return 0;
}

TILE_GET_INFO_MEMBER(igs_m027_033vid_state::get_bg_tile_info)
{
	int tileno = m_bg_videoram[tile_index / 4];
	tileno = tileno >> (8 * (tile_index & 3)) & 0xff;
	int attr = m_bg_attr_videoram[tile_index / 4];
	attr = attr >> (8 * (tile_index & 3)) & 0xff;

	tileno |= ((attr & 0x1f) << 8);

	int col = (attr & 0xe0) >> 5;
	col <<= 1; // gfx are 4bpp, every other set of colours is unused

	tileno |= (m_tilebank << 13);

	tileinfo.set(0, tileno, col, 0);
}

// TODO: convert to 8-bit handlers and 8-bit RAM?
void igs_m027_033vid_state::bg_videoram_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_bg_videoram[offset]);
	m_bg_tilemap->mark_tile_dirty((offset * 4) + 0);
	m_bg_tilemap->mark_tile_dirty((offset * 4) + 1);
	m_bg_tilemap->mark_tile_dirty((offset * 4) + 2);
	m_bg_tilemap->mark_tile_dirty((offset * 4) + 3);
}

void igs_m027_033vid_state::bg_attr_videoram_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_bg_attr_videoram[offset]);
	m_bg_tilemap->mark_tile_dirty((offset * 4) + 0);
	m_bg_tilemap->mark_tile_dirty((offset * 4) + 1);
	m_bg_tilemap->mark_tile_dirty((offset * 4) + 2);
	m_bg_tilemap->mark_tile_dirty((offset * 4) + 3);
}

void igs_m027_033vid_state::counters_w(u8 data)
{
	if (data & 0x0f)
		logerror("%s PPI out port C w: %02X\n", machine().describe_context(), data);

	machine().bookkeeping().coin_counter_w(0, BIT(data, 7)); // COIN1
	machine().bookkeeping().coin_counter_w(1, BIT(data, 5)); // COIN2 (or KEYIN?)
	machine().bookkeeping().coin_counter_w(2, BIT(data, 6)); // PAYOUT
	machine().bookkeeping().coin_counter_w(3, BIT(data, 4)); // KEYOUT
}

void igs_m027_033vid_state::out_port_w(u8 data)
{
	if (data & 0xe8)
		logerror("%s IGS027A out port w: %02X\n", machine().describe_context(), data);

	m_video_enable = BIT(data, 0);

	m_oki->set_rom_bank(bitswap<2>(data, 1, 2)); // TODO: fishy, verify when more games are dumped

	m_tilebank = BIT(data, 4);

	m_bg_tilemap->mark_all_dirty();
}


/***************************************************************************

    Memory Maps

***************************************************************************/

void igs_m027_033vid_state::m027_map(address_map &map) // TODO: some unknown writes
{
	map(0x0800'0000, 0x0807'ffff).r(FUNC(igs_m027_033vid_state::external_rom_r)); // Game ROM

	map(0x1800'0000, 0x1800'7fff).ram().share(m_nvram);

	map(0x3800'2000, 0x3800'20ff).ram().w(m_palette, FUNC(palette_device::write32)).share("palette");
	map(0x3800'3000, 0x3800'30ff).ram().w(m_palette, FUNC(palette_device::write32_ext)).share("palette_ext");

	map(0x3800'4000, 0x3800'4003).portr("DSW");
	map(0x3800'5003, 0x3800'5003).lw8(NAME([this] (uint8_t data) { m_hopper->motor_w(BIT(data, 0)); }));
	map(0x3800'5010, 0x3800'5013).umask32(0x0000'00ff).rw("oki", FUNC(okim6295_device::read), FUNC(okim6295_device::write));
	map(0x3800'5030, 0x3800'5033).rw("ppi", FUNC(i8255_device::read), FUNC(i8255_device::write));

	map(0x3800'7000, 0x3800'77ff).ram().w(FUNC(igs_m027_033vid_state::bg_videoram_w)).share(m_bg_videoram);
	map(0x3800'7800, 0x3800'7fff).ram().w(FUNC(igs_m027_033vid_state::bg_attr_videoram_w)).share(m_bg_attr_videoram);

	map(0x4000'0008, 0x4000'000b).nopw();
	map(0x4800'0000, 0x4800'0003).nopw();

	map(0x5000'0000, 0x5000'03ff).umask32(0x0000'00ff).w(FUNC(igs_m027_033vid_state::xor_table_w)); // uploads XOR table to external ROM here

	map(0x7000'0000, 0x7000'01ff).nopw();
}


/***************************************************************************

    Input Ports

***************************************************************************/

INPUT_PORTS_START( qiji6 ) // TODO: complete
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_GAMBLE_PAYOUT )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_GAMBLE_D_UP )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_GAMBLE_TAKE )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_GAMBLE_HIGH )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_GAMBLE_LOW )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_GAMBLE_KEYOUT )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("hopper", FUNC(hopper_device::line_r))

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_POKER_HOLD2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_POKER_HOLD1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_GAMBLE_BOOK )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START1 )
	PORT_SERVICE_NO_TOGGLE( 0x20, IP_ACTIVE_LOW )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_GAMBLE_BET )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_POKER_HOLD4 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_POKER_HOLD3 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_POKER_HOLD5 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	// most settings are done on the software side. Password is all Start
	PORT_START("DSW")
	PORT_DIPNAME(          0x0000001f, 0x0000001f, "Satellite Machine Number" ) PORT_DIPLOCATION("SW1:1,2,3,4,5")
	PORT_DIPSETTING(                   0x0000001f, "1" )
	PORT_DIPSETTING(                   0x0000001e, "2" )
	PORT_DIPSETTING(                   0x0000001d, "3" )
	PORT_DIPSETTING(                   0x0000001c, "4" )
	PORT_DIPSETTING(                   0x0000001b, "5" )
	PORT_DIPSETTING(                   0x0000001a, "6" )
	PORT_DIPSETTING(                   0x00000019, "7" )
	PORT_DIPSETTING(                   0x00000018, "8" )
	PORT_DIPSETTING(                   0x00000017, "9" )
	PORT_DIPSETTING(                   0x00000016, "10" )
	PORT_DIPSETTING(                   0x00000015, "11" )
	PORT_DIPSETTING(                   0x00000014, "12" )
	PORT_DIPSETTING(                   0x00000013, "13" )
	PORT_DIPSETTING(                   0x00000012, "14" )
	PORT_DIPSETTING(                   0x00000011, "15" )
	PORT_DIPSETTING(                   0x00000010, "16" )
	PORT_DIPSETTING(                   0x0000000f, "17" )
	PORT_DIPSETTING(                   0x0000000e, "18" )
	PORT_DIPSETTING(                   0x0000000d, "19" )
	PORT_DIPSETTING(                   0x0000000c, "20" )
	PORT_DIPSETTING(                   0x0000000b, "20 (duplicate)" )
	PORT_DIPSETTING(                   0x0000000a, "20 (duplicate)" )
	PORT_DIPSETTING(                   0x00000009, "20 (duplicate)" )
	PORT_DIPSETTING(                   0x00000008, "20 (duplicate)" )
	PORT_DIPSETTING(                   0x00000007, "20 (duplicate)" )
	PORT_DIPSETTING(                   0x00000006, "20 (duplicate)" )
	PORT_DIPSETTING(                   0x00000005, "20 (duplicate)" )
	PORT_DIPSETTING(                   0x00000004, "20 (duplicate)" )
	PORT_DIPSETTING(                   0x00000003, "20 (duplicate)" )
	PORT_DIPSETTING(                   0x00000002, "20 (duplicate)" )
	PORT_DIPSETTING(                   0x00000001, "20 (duplicate)" )
	PORT_DIPSETTING(                   0x00000000, "20 (duplicate)" )
	PORT_DIPUNKNOWN_DIPLOC(0x00000020, 0x00000020, "SW1:6")
	PORT_DIPUNKNOWN_DIPLOC(0x00000040, 0x00000040, "SW1:7")
	PORT_DIPNAME(          0x00000080, 0x00000080, "Link Mode" ) PORT_DIPLOCATION("SW1:8") // Hard-coded?
	PORT_DIPSETTING(                   0x00000080, "Single Machine" )
	PORT_DIPSETTING(                   0x00000000, "Single Machine (duplicate)" )
	PORT_DIPUNKNOWN_DIPLOC(0x00000100, 0x00000100, "SW3:1")
	PORT_DIPUNKNOWN_DIPLOC(0x00000200, 0x00000200, "SW3:2")
	PORT_DIPUNKNOWN_DIPLOC(0x00000400, 0x00000400, "SW3:3")
	PORT_DIPUNKNOWN_DIPLOC(0x00000800, 0x00000800, "SW3:4")
	PORT_DIPUNKNOWN_DIPLOC(0x00001000, 0x00001000, "SW3:5")
	PORT_DIPUNKNOWN_DIPLOC(0x00002000, 0x00002000, "SW3:6")
	PORT_DIPUNKNOWN_DIPLOC(0x00004000, 0x00004000, "SW3:7")
	PORT_DIPUNKNOWN_DIPLOC(0x00008000, 0x00008000, "SW3:8")
	PORT_DIPNAME(          0x00010000, 0x00000000, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(                   0x00010000, DEF_STR( Off ) )
	PORT_DIPSETTING(                   0x00000000, DEF_STR( On ) )
	PORT_DIPUNKNOWN_DIPLOC(0x00020000, 0x00020000, "SW2:2")
	PORT_DIPUNKNOWN_DIPLOC(0x00040000, 0x00040000, "SW2:3")
	PORT_DIPUNKNOWN_DIPLOC(0x00080000, 0x00080000, "SW2:4")
	PORT_DIPUNKNOWN_DIPLOC(0x00100000, 0x00100000, "SW2:5")
	PORT_DIPUNKNOWN_DIPLOC(0x00200000, 0x00200000, "SW2:6")
	PORT_DIPUNKNOWN_DIPLOC(0x00400000, 0x00400000, "SW2:7")
	PORT_DIPUNKNOWN_DIPLOC(0x00800000, 0x00800000, "SW2:8")
	PORT_BIT( 0xff000000, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END

INPUT_PORTS_START( huahuas5 )
	PORT_INCLUDE( qiji6 )

	PORT_MODIFY("DSW")
	PORT_DIPNAME(          0x00010000, 0x00010000, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(                   0x00000000, DEF_STR( Off ) )
	PORT_DIPSETTING(                   0x00010000, DEF_STR( On ) )
INPUT_PORTS_END


static const gfx_layout tiles8x8x4_layout =
{
	8, 8,
	RGN_FRAC(1,1),
	4,
	{ 24, 8, 16, 0 },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32 },
	32*8
};

static GFXDECODE_START( igs033 )
	GFXDECODE_ENTRY( "igs033", 0, tiles8x8x4_layout, 0, 16 )
GFXDECODE_END

TIMER_DEVICE_CALLBACK_MEMBER(igs_m027_033vid_state::interrupt)
{
	// TODO: which of these are needed, are they always enabled, when do they fire?
	int scanline = param;

	if (scanline == 240)
		m_maincpu->pulse_input_line(arm7_cpu_device::ARM7_IRQ_LINE, m_maincpu->minimum_quantum_time());

	if (scanline == 0)
		m_maincpu->pulse_input_line(arm7_cpu_device::ARM7_FIRQ_LINE, m_maincpu->minimum_quantum_time());
}

/***************************************************************************

    Machine Drivers

***************************************************************************/

u32 igs_m027_033vid_state::external_rom_r(offs_t offset)
{
	return m_external_rom[offset] ^ m_xor_table[offset & 0x00ff];
}

void igs_m027_033vid_state::xor_table_w(offs_t offset, u8 data)
{
	m_xor_table[offset] = (u32(data) << 24) | (u32(data) << 8);
}

void igs_m027_033vid_state::m027_033vid(machine_config &config)
{
	IGS027A(config, m_maincpu, 24_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &igs_m027_033vid_state::m027_map);
	m_maincpu->in_port().set([this] () { LOGPORTS("%s IGS027A in port r\n", machine().describe_context()); return 0xffffffff; }); // TODO: read continuously, what's here?
	m_maincpu->out_port().set(FUNC(igs_m027_033vid_state::out_port_w));

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	i8255_device &ppi(I8255A(config, "ppi"));
	ppi.tri_pa_callback().set_constant(0x00);
	ppi.tri_pb_callback().set_constant(0x00);
	ppi.tri_pc_callback().set_constant(0x00);
	ppi.in_pa_callback().set_ioport("IN0");
	ppi.in_pb_callback().set_ioport("IN1");
	ppi.in_pc_callback().set_ioport("IN2");
	// A and B out ports seem unused
	ppi.out_pa_callback().set([this] (u8 data) { LOGPORTS("%s: PPI port A write %02x\n", machine().describe_context(), data); });
	ppi.out_pb_callback().set([this] (u8 data) { LOGPORTS("%s: PPI port B write %02x\n", machine().describe_context(), data); });
	ppi.out_pc_callback().set(FUNC(igs_m027_033vid_state::counters_w));

	HOPPER(config, m_hopper, attotime::from_msec(50));

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(1000));
	m_screen->set_size(512, 256);
	m_screen->set_visarea(0, 512-1, 0, 256-1);
	m_screen->set_screen_update(FUNC(igs_m027_033vid_state::screen_update));
	m_screen->set_palette(m_palette);

	PALETTE(config, m_palette).set_format(palette_device::xBGR_555, 0x100);
	m_palette->set_membits(8);

	GFXDECODE(config, m_gfxdecode, "palette", igs033);

	TIMER(config, "scantimer").configure_scanline(FUNC(igs_m027_033vid_state::interrupt), "screen", 0, 1);

	// sound hardware
	SPEAKER(config, "mono").front_center();

	OKIM6295(config, m_oki, 24_MHz_XTAL / 24, okim6295_device::PIN7_HIGH).add_route(ALL_OUTPUTS, "mono", 0.5); // divider and pin 7 not verified
}

void igs_m027_033vid_state::huahuas5(machine_config &config)
{
	m027_033vid(config);

	m_oki->set_clock(24_MHz_XTAL / 12); // divider and pin 7 not verified
}


/***************************************************************************

    ROMs Loading

***************************************************************************/


// IGS PCB-0405-02-FZ
// SP and IGS027A ROMs have original labels with '花花世界3', but it seems the external program ROM and GFX ROM got changed
ROM_START( qiji6 )
	ROM_REGION( 0x4000, "maincpu", 0 )
	// Internal rom of IGS027A ARM based MCU
	ROM_LOAD( "f8_027a.bin", 0x0000, 0x4000, CRC(4662f015) SHA1(c10889964b675f5c11ea1571332f3eec418c9a28) )

	ROM_REGION32_LE( 0x80000, "user1", 0 ) // external ARM data / prg
	ROM_LOAD( "v118.u12", 0x00000, 0x80000, CRC(c2729fbe) SHA1(2153675a1161bd6aea6367c55fcf801c7fb0dd3a) )

	ROM_REGION( 0x80000, "igs033", 0 )
	ROM_LOAD( "7e.u20", 0x000000, 0x080000, CRC(8362eeff) SHA1(1babebe872d253d9131131658e701fbf270d42e2) )

	ROM_REGION( 0x80000, "oki", 0 )
	ROM_LOAD( "sp.3", 0x00000, 0x80000, CRC(06b70fe9) SHA1(5df34f870d32893b5c3095fb9653954209712cdb) )
ROM_END

// 花花世界 5 (Huāhuā Shìjiè 5) / 飞行世界 (Fēixíng Shìjiè)
// Alternate title is shown if the card type is changed in the test menu
// IGS PCB-0405-02-FZ + IGS PCB-0492-00 riser board
ROM_START( huahuas5 )
	ROM_REGION( 0x4000, "maincpu", 0 )
	// Internal rom of IGS027A ARM based MCU
	ROM_LOAD( "f11_027a.bin", 0x0000, 0x4000, CRC(f4cacbcf) SHA1(e09f554c1539f37f56d235134754b2a371ea6ad5) )

	ROM_REGION32_LE( 0x80000, "user1", 0 ) // external ARM data / prg
	ROM_LOAD( "v107.u12", 0x00000, 0x80000, CRC(f7b2265a) SHA1(87a5987c39888b18b71675ddfc014a49dab60839) )

	ROM_REGION( 0x80000, "igs033", 0 )
	ROM_LOAD( "full.u20", 0x000000, 0x080000, CRC(26c30dd7) SHA1(ae53a5986262ce587e414a37e6b3bcb3acec83a5) )

	ROM_REGION( 0x100000, "oki", 0 ) // on the riser board with a PAL, probably for banking
	ROM_LOAD( "fullsp.u3", 0x000000, 0x100000, CRC(25c7a2a8) SHA1(01133bf0b7ef140e3d1608d49d041fd86c90ac94) ) //  1xxxxxxxxxxxxxxxxxxxx = 0x00
	ROM_IGNORE(                      0x100000 )
ROM_END


void igs_m027_033vid_state::init_qiji6()
{
	qiji6_decrypt(machine());
}

void igs_m027_033vid_state::init_huahuas5()
{
	cjddzlf_decrypt(machine());
}

} // anonymous namespace


/***************************************************************************

    Game Drivers

***************************************************************************/

// internal ROM date is 2002, external software revision could be later
GAME( 2002, qiji6,    0, m027_033vid, qiji6,    igs_m027_033vid_state, init_qiji6,    ROT0, "IGS", "Qiji 6 (V118CN)",                           MACHINE_SUPPORTS_SAVE )
// internal ROM date is 2004, external software revision could be later
GAME( 2004, huahuas5, 0, huahuas5,    huahuas5, igs_m027_033vid_state, init_huahuas5, ROT0, "IGS", "Huahua Shijie 5 / Feixing Shijie (V107CN)", MACHINE_SUPPORTS_SAVE )
