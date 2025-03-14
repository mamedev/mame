// license:BSD-3-Clause
// copyright-holders:

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


namespace {

class igs_m027_033vid_state : public driver_device
{
public:
	igs_m027_033vid_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_external_rom(*this, "user1"),
		m_nvram(*this, "nvram"),
		m_maincpu(*this, "maincpu"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette"),
		m_gfxdecode(*this, "gfxdecode"),
		m_bg_videoram(*this, "bg_videoram"),
		m_bg_attr_videoram(*this, "bg_attr_videoram")
	{ }

	void m027_033vid(machine_config &config) ATTR_COLD;

	void init_flowerw3() ATTR_COLD;

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

private:
	required_region_ptr<u32> m_external_rom;
	required_shared_ptr<u32> m_nvram;

	required_device<igs027a_cpu_device> m_maincpu;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	required_device<gfxdecode_device> m_gfxdecode;
	required_shared_ptr<uint32_t> m_bg_videoram;
	required_shared_ptr<uint32_t> m_bg_attr_videoram;

	u32 m_xor_table[0x100];
	tilemap_t *m_bg_tilemap = nullptr;

	TIMER_DEVICE_CALLBACK_MEMBER(interrupt);

	TILE_GET_INFO_MEMBER(get_bg_tile_info);

	void bg_videoram_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	void bg_attr_videoram_w(offs_t offset, uint32_t data, uint32_t mem_mask);

	u32 gpio_r() { return 0xffffffff; };
	u32 unknown_4000_r() { return 0xffffffff; };
	u32 unknown_5030_r() { return 0xffffffff; };

	u32 external_rom_r(offs_t offset);
	void xor_table_w(offs_t offset, u8 data);

	u32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void screen_vblank(int state);

	void m027_map(address_map &map) ATTR_COLD;
};


void igs_m027_033vid_state::machine_start()
{
	std::fill(std::begin(m_xor_table), std::end(m_xor_table), 0);

	save_item(NAME(m_xor_table));
}

void igs_m027_033vid_state::machine_reset()
{
}

void igs_m027_033vid_state::video_start()
{
	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(igs_m027_033vid_state::get_bg_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 64, 32);
}

u32 igs_m027_033vid_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	return 0;
}

void igs_m027_033vid_state::screen_vblank(int state)
{
}

TILE_GET_INFO_MEMBER(igs_m027_033vid_state::get_bg_tile_info)
{
	int tileno = m_bg_videoram[tile_index/4];
	tileno = tileno >> (8 * (tile_index & 3)) & 0xff;
	int attr = m_bg_attr_videoram[tile_index/4];
	attr = attr >> (8 * (tile_index & 3)) & 0xff;

	tileno |= ((attr & 0x1f) << 8);

	int col = (attr & 0xe0) >> 5;
	col <<= 1; // gfx are 4bpp, every other set of colours is unused

	tileno |= 0x2000; // bank not hooked up (where is it, when is it used, not in attract, probably D-UP game)

	tileinfo.set(0, tileno, col, 0);
}

// TODO: convert to 8-bit handlers and 8-bit RAM?
void igs_m027_033vid_state::bg_videoram_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&m_bg_videoram[offset]);
	m_bg_tilemap->mark_tile_dirty((offset * 4) + 0);
	m_bg_tilemap->mark_tile_dirty((offset * 4) + 1);
	m_bg_tilemap->mark_tile_dirty((offset * 4) + 2);
	m_bg_tilemap->mark_tile_dirty((offset * 4) + 3);
}

void igs_m027_033vid_state::bg_attr_videoram_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&m_bg_attr_videoram[offset]);
	m_bg_tilemap->mark_tile_dirty((offset * 4) + 0);
	m_bg_tilemap->mark_tile_dirty((offset * 4) + 1);
	m_bg_tilemap->mark_tile_dirty((offset * 4) + 2);
	m_bg_tilemap->mark_tile_dirty((offset * 4) + 3);
}

/***************************************************************************

    Memory Maps

***************************************************************************/

void igs_m027_033vid_state::m027_map(address_map &map) // TODO: everything to be verified
{
	map(0x0800'0000, 0x0807'ffff).r(FUNC(igs_m027_033vid_state::external_rom_r)); // Game ROM

	map(0x1800'0000, 0x1800'7fff).ram().share(m_nvram);

	map(0x3800'2000, 0x3800'20ff).ram().w(m_palette, FUNC(palette_device::write32)).share("palette");
	map(0x3800'3000, 0x3800'30ff).ram().w(m_palette, FUNC(palette_device::write32_ext)).share("palette_ext");

	map(0x3800'4000, 0x3800'4003).r(FUNC(igs_m027_033vid_state::unknown_4000_r));
	map(0x3800'5030, 0x3800'5033).r(FUNC(igs_m027_033vid_state::unknown_5030_r));

	map(0x3800'7000, 0x3800'77ff).ram().w(FUNC(igs_m027_033vid_state::bg_videoram_w)).share(m_bg_videoram);
	map(0x3800'7800, 0x3800'7fff).ram().w(FUNC(igs_m027_033vid_state::bg_attr_videoram_w)).share(m_bg_attr_videoram);
	
	map(0x5000'0000, 0x5000'03ff).umask32(0x0000'00ff).w(FUNC(igs_m027_033vid_state::xor_table_w)); // uploads XOR table to external ROM here
}


/***************************************************************************

    Input Ports

***************************************************************************/

INPUT_PORTS_START( flowerw3 )
	PORT_START("IN0")
	PORT_BIT(0xffff, IP_ACTIVE_LOW, IPT_UNKNOWN)

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

	PORT_START("DSW3")
	PORT_DIPUNKNOWN_DIPLOC(0x01, 0x01, "SW3:1")
	PORT_DIPUNKNOWN_DIPLOC(0x02, 0x02, "SW3:2")
	PORT_DIPUNKNOWN_DIPLOC(0x04, 0x04, "SW3:3")
	PORT_DIPUNKNOWN_DIPLOC(0x08, 0x08, "SW3:4")
	PORT_DIPUNKNOWN_DIPLOC(0x10, 0x10, "SW3:5")
	PORT_DIPUNKNOWN_DIPLOC(0x20, 0x20, "SW3:6")
	PORT_DIPUNKNOWN_DIPLOC(0x40, 0x40, "SW3:7")
	PORT_DIPUNKNOWN_DIPLOC(0x80, 0x80, "SW3:8")
INPUT_PORTS_END

static const gfx_layout tiles8x8x4_layout =
{
	8, 8,
	RGN_FRAC(1,1),
	4,
	{ 24,16,8,0 },
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
	m_maincpu->in_port().set(FUNC(igs_m027_033vid_state::gpio_r));

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(1000));
	m_screen->set_size(512, 256);
	m_screen->set_visarea(0, 512-1, 0, 256-1);
	m_screen->set_screen_update(FUNC(igs_m027_033vid_state::screen_update));
	m_screen->set_palette(m_palette);
	m_screen->screen_vblank().set(FUNC(igs_m027_033vid_state::screen_vblank));

	PALETTE(config, m_palette).set_format(palette_device::xBGR_555, 0x100);
	m_palette->set_membits(8);

	GFXDECODE(config, m_gfxdecode, "palette", igs033);

	TIMER(config, "scantimer").configure_scanline(FUNC(igs_m027_033vid_state::interrupt), "screen", 0, 1);

	// sound hardware
	SPEAKER(config, "mono").front_center();

	OKIM6295(config, "oki", 24_MHz_XTAL / 24, okim6295_device::PIN7_HIGH).add_route(ALL_OUTPUTS, "mono", 0.5); // divider and pin 7 not verified
}


/***************************************************************************

    ROMs Loading

***************************************************************************/


// IGS PCB-0405-02-FZ
// SP and IGS027A ROMs have original labels with '花花世界3'
ROM_START( flowerw3 )
	ROM_REGION( 0x4000, "maincpu", 0 )
	// Internal rom of IGS027A ARM based MCU
	ROM_LOAD( "f8_027a.bin", 0x0000, 0x4000, CRC(4662f015) SHA1(c10889964b675f5c11ea1571332f3eec418c9a28) )

	ROM_REGION32_LE( 0x80000, "user1", ROMREGION_ERASEFF ) // external ARM data / prg
	ROM_LOAD( "v118.u12", 0x00000, 0x80000, CRC(c2729fbe) SHA1(2153675a1161bd6aea6367c55fcf801c7fb0dd3a) )

	ROM_REGION( 0x80000, "igs033", 0 )
	// could be a slight encryption, but looks more like a bad read
	ROM_LOAD( "7e.u20",  0x000000, 0x080000, BAD_DUMP CRC(a7b65af6) SHA1(bef13d38eb793b2860c2922f0cfb4b011fd9991b) )

	ROM_REGION( 0x80000, "oki", 0 )
	ROM_LOAD( "sp.3", 0x00000, 0x80000, CRC(06b70fe9) SHA1(5df34f870d32893b5c3095fb9653954209712cdb) )
ROM_END


void igs_m027_033vid_state::init_flowerw3()
{
	flowerw3_decrypt(machine());
}

} // anonymous namespace


/***************************************************************************

    Game Drivers

***************************************************************************/

// internal ROM date is 2002, external software revision could be later
GAME( 2002, flowerw3, 0, m027_033vid, flowerw3, igs_m027_033vid_state, init_flowerw3, ROT0, "IGS", "Flower World 3 (V118CN)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // title to be adjusted once it works
