// license:BSD-3-Clause
// copyright-holders:AJR

/*
Video slots by Logic Game Tech Int. (LGT) running on Z80-family CPUs.

Standard Z80 PCB components are:
Z80 (U45, covered by a "NO. 88002" sticker)
12.000 MHz oscillator (U26, Toyocom TCO-711A)
明星97 II ASIC (U25, 80-pin QFP, custom)
明星97 II ASIC 2 (U16, 44-pin PLCC, custom)
OKI M6295 or compatible (U10, 44-pin QFP, unreadable)
5x HM6116LK-70 (U18, U20, U23, U28, U31)
28-pin DIP chip under a "Q.C. Passed" sticker (U3)
27C512 program ROM (U43)
4x 2 Mbit GFX ROMs (U1, U5, U7, U14)
2 Mbit sample ROM (U4)
GAL16V8B (U8)
MC1413PD / ULN2003AD (U34)
DIP-8 chip (U17, unreadable, probably the audio amplifier)
Ni-Cd battery (BT1)
volume trimmer (VR1)
3x 3-pin jumpers (JP1, JP2, JP3)
no DIP switches

Unpopulated: U40, U41, Q2, Y2, PLCC footprint near JP3 (partially covered by a sticker)


KL5C80A12 (or similar) PCB components are:
scratched off rectangular 100-pin chip, stickered ASIC 1
scratched off square 100-pin chip, stickered ASIC 2
scratched off square 100-pin chip, stickered ASIC 3
scratched off square 44-pin chip, stickered ASIC 4
12 MHz XTAL (near ASIC 2)
7.3728 MHz XTAL (near ASIC 4)
U6295 sound chip
HM86171-80 RAMDAC (near CPU ROM)
6x M5M5256DVP (1 near CPU ROM, 5 near GFX ROMs)

Games use PCBs with different layout, however the components appear
to be the same or at least same from different manufacturers.

"ASIC 1" is probably a KL5C80A12 CPU, though its on-chip peripherals are used for little
besides rudimentary ROM banking and port I/O.

Pharaoh's Mystery has a 'Wing' string plus other strings typically associated to Wing games.

TODO:
- arthurkn uploads code to NVRAM if missing, fruitcat and pharmyst need NVRAM pre-populated
  pharmyst's one is dumped, while fruitcat's isn't, thus it currently runs off the rails
  when calling to NVRAM, seems to fortuitously recover, but never populates tile RAM.
  Note that according to the dumper, NVRAM is probably populated by one of the customs,
  since he removed battery from NVRAM many times and it hasn't stopped the PCB from
  working
- arthurkn runs correctly and needs the following:
  - outputs (lamps / meters)
  - hopper (off by default)
  - verify the bonus games which appear when hitting 3 "bars" (control bit 6 handling)
- pharmyst writes commands to 0x8d81 (C1 = restore settings, C2 = store settings) and waits
  for a custom to clear it; the custom is currently simulated by clearing the byte at once.
  It also needs inputs, outputs checking;
*/


#include "emu.h"

#include "cpu/z80/kl5c80a12.h"
#include "cpu/z80/z80.h"
#include "machine/nvram.h"
#include "machine/ticket.h"
#include "machine/watchdog.h"
#include "sound/okim6295.h"
#include "video/ramdac.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"


namespace {

class lgtz80_base_state : public driver_device
{
public:
	lgtz80_base_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_oki(*this, "oki"),
		m_tile_ram(*this, "tile_ram"),
		m_tile_attr_ram(*this, "tile_attr_ram"),
		m_reel_ram(*this, "reel_ram%u", 0U),
		m_reel_attr_ram(*this, "reel_attr_ram%u", 0U),
		m_reel_scroll_ram(*this, "reel_scroll_ram%u", 0U)
	{ }

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<okim6295_device> m_oki;

	required_shared_ptr<u8> m_tile_ram;
	required_shared_ptr<u8> m_tile_attr_ram;
	required_shared_ptr_array<u8, 4> m_reel_ram;
	required_shared_ptr_array<u8, 4> m_reel_attr_ram;
	required_shared_ptr_array<u8, 4> m_reel_scroll_ram;

	void base(machine_config &config) ATTR_COLD;

	u8 m_control = 0;

	// reel layers 0-2 are each shown in a fixed horizontal band (deduced from the games' screen layout)
	u16 m_reel_band_start = 0;
	u16 m_reel_band_height = 0;
	u8 m_reel_trans_pen = 0;

	tilemap_t *m_tilemap = nullptr;
	tilemap_t *m_reel_tilemap[4] {};

	u32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	void vblank_nmi_w(int state);

	u8 control_r();

	void tile_ram_w(offs_t offset, u8 data);
	void tile_attr_ram_w(offs_t offset, u8 data);
	template <u8 Which> void reel_ram_w(offs_t offset, u8 data);
	template <u8 Which> void reel_attr_ram_w(offs_t offset, u8 data);
	template <u8 Which> void reel_scroll_ram_w(offs_t offset, u8 data);
	TILE_GET_INFO_MEMBER(get_tile_info);
	template <u8 Which> TILE_GET_INFO_MEMBER(get_reel_tile_info);

	void ramdac_map(address_map &map) ATTR_COLD;
};

class lgtz80_state : public lgtz80_base_state
{
public:
	lgtz80_state(const machine_config &mconfig, device_type type, const char *tag) :
		lgtz80_base_state(mconfig, type, tag),
		m_rombank(*this, "rombank")
	{
		m_reel_band_start = 32;
		m_reel_band_height = 64;
		m_reel_trans_pen = 0x00;
	}

	void mx97iitb(machine_config &config) ATTR_COLD;

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	required_memory_bank m_rombank;

	void control_w(u8 data);

	void program_map(address_map &map) ATTR_COLD;
	void io_map(address_map &map) ATTR_COLD;
};

class lgtkl_state : public lgtz80_base_state
{
public:
	lgtkl_state(const machine_config &mconfig, device_type type, const char *tag) :
		lgtz80_base_state(mconfig, type, tag),
		m_nvram(*this, "nvram")
	{
		m_reel_band_start = 24;
		m_reel_band_height = 56;
		m_reel_trans_pen = 0x1f;
	}

	void fruitcat(machine_config &config) ATTR_COLD;
	void pharmyst(machine_config &config) ATTR_COLD;
	void arthurkn(machine_config &config) ATTR_COLD;
	void arthurkn100(machine_config &config) ATTR_COLD;

	void init_arthurkn() ATTR_COLD;
	void init_arthurkn100() ATTR_COLD;
	void init_circus2() ATTR_COLD;
	void init_fruitcat() ATTR_COLD;
	void init_pharmyst() ATTR_COLD;

protected:
	virtual void machine_reset() override ATTR_COLD;

private:
	required_shared_ptr<u8> m_nvram;

	void oki_bank_w(u8 data);
	void settings_cmd_w(u8 data);
	void control_w(u8 data);
	u8 e0_r();

	void program_map(address_map &map) ATTR_COLD;
	void pharmyst_program_map(address_map &map) ATTR_COLD;
	void fruitcat_io_map(address_map &map) ATTR_COLD;
	void arthurkn_io_map(address_map &map) ATTR_COLD;
	void arthurkn100_io_map(address_map &map) ATTR_COLD;
};


void lgtz80_base_state::machine_start()
{
	save_item(NAME(m_control));
}

void lgtz80_state::machine_start()
{
	lgtz80_base_state::machine_start();

	m_rombank->configure_entries(0, 4, memregion("maincpu")->base(), 0x4000);
}

void lgtz80_state::machine_reset()
{
	lgtz80_base_state::machine_reset();

	control_w(0);
}

void lgtkl_state::machine_reset()
{
	lgtz80_base_state::machine_reset();

	control_w(0);
}


u32 lgtz80_base_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	// one scroll value every 16 pixels, at even addresses only
	// layer 0: 0xe040-0xe07f, layer 1: 0xe080-0xe0bf, layer 2: 0xe100-0xe13f, layer 3: 0xe000-0xe03f
	for (int j = 0; j < 0x20; j++)
	{
		m_reel_tilemap[0]->set_scrolly(j, m_reel_scroll_ram[0][0x40 | (j << 1)]);
		m_reel_tilemap[1]->set_scrolly(j, m_reel_scroll_ram[1][j << 1]);
		m_reel_tilemap[2]->set_scrolly(j, m_reel_scroll_ram[2][j << 1]);
		m_reel_tilemap[3]->set_scrolly(j, m_reel_scroll_ram[0][j << 1]);
	}

	bitmap.fill(rgb_t::black(), cliprect);

	if (BIT(m_control, 6))
	{
		// reel layer 0 is shown full screen (title screens, bonus games)
		m_reel_tilemap[0]->draw(screen, bitmap, cliprect, 0, 0);
	}
	else
	{
		m_reel_tilemap[3]->draw(screen, bitmap, cliprect, 0, 0);

		for (int i = 0; i < 3; i++)
		{
			rectangle clip = cliprect;
			clip.sety(m_reel_band_start + i * m_reel_band_height, m_reel_band_start + (i + 1) * m_reel_band_height - 1);
			clip &= cliprect;
			if (!clip.empty())
				m_reel_tilemap[i]->draw(screen, bitmap, clip, 0, 0);
		}
	}

	m_tilemap->draw(screen, bitmap, cliprect, 0, 0);

	return 0;
}

void lgtz80_base_state::video_start()
{
	m_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(lgtz80_base_state::get_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 64, 32);

	m_reel_tilemap[0] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(lgtz80_base_state::get_reel_tile_info<0>)), TILEMAP_SCAN_ROWS, 8, 32, 64, 8);
	m_reel_tilemap[1] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(lgtz80_base_state::get_reel_tile_info<1>)), TILEMAP_SCAN_ROWS, 8, 32, 64, 8);
	m_reel_tilemap[2] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(lgtz80_base_state::get_reel_tile_info<2>)), TILEMAP_SCAN_ROWS, 8, 32, 64, 8);
	m_reel_tilemap[3] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(lgtz80_base_state::get_reel_tile_info<3>)), TILEMAP_SCAN_ROWS, 8, 32, 64, 8);

	m_tilemap->set_transparent_pen(0);

	for (int i = 0; i < 4; i++)
	{
		m_reel_tilemap[i]->set_scroll_cols(0x20); // one scroll value every 16 pixels
		m_reel_tilemap[i]->set_transparent_pen(m_reel_trans_pen);
	}
}

TILE_GET_INFO_MEMBER(lgtz80_base_state::get_tile_info)
{
	int const tile = m_tile_ram[tile_index] | (m_tile_attr_ram[tile_index] << 8);

	tileinfo.set(0, tile, 0, 0);
}

template <u8 Which>
TILE_GET_INFO_MEMBER(lgtz80_base_state::get_reel_tile_info)
{
	int const tile = m_reel_ram[Which][tile_index] | (m_reel_attr_ram[Which][tile_index] << 8);

	tileinfo.set(1, tile, 0, 0);
}

void lgtz80_base_state::tile_ram_w(offs_t offset, u8 data)
{
	m_tile_ram[offset] = data;
	m_tilemap->mark_tile_dirty(offset);
}

void lgtz80_base_state::tile_attr_ram_w(offs_t offset, u8 data)
{
	m_tile_attr_ram[offset] = data;
	m_tilemap->mark_tile_dirty(offset);
}

template <u8 Which>
void lgtz80_base_state::reel_ram_w(offs_t offset, u8 data)
{
	m_reel_ram[Which][offset] = data;
	m_reel_tilemap[Which]->mark_tile_dirty(offset);
}

template <u8 Which>
void lgtz80_base_state::reel_attr_ram_w(offs_t offset, u8 data)
{
	m_reel_attr_ram[Which][offset] = data;
	m_reel_tilemap[Which]->mark_tile_dirty(offset);
}

template <u8 Which>
void lgtz80_base_state::reel_scroll_ram_w(offs_t offset, u8 data)
{
	// A6, A7 and A8 select layers 0, 1 and 2 (0xe040, 0xe080, 0xe100), none of them selects layer 3 (0xe000).
	// mx97iitb writes to 0xe1c0-0xe1ff to initialize layers 0-2 at once.
	offset |= Which << 7;

	u8 const col = offset & 0x3f;

	if (!(offset & 0x1c0))
		m_reel_scroll_ram[0][col] = data;
	if (BIT(offset, 6))
		m_reel_scroll_ram[0][0x40 | col] = data;
	if (BIT(offset, 7))
		m_reel_scroll_ram[1][col] = data;
	if (BIT(offset, 8))
		m_reel_scroll_ram[2][col] = data;
}


void lgtkl_state::oki_bank_w(u8 data)
{
	// fruitcat only? arthurkn configures P00 as an output pin but never writes to the data register
	m_oki->set_rom_bank(BIT(data, 0));
}

u8 lgtz80_base_state::control_r()
{
	return m_control;
}

void lgtz80_state::control_w(u8 data)
{
	m_control = data;

	// bit 0, 1 are ROM bank
	m_rombank->set_entry(bitswap<2>(m_control, 1, 0));

	// Bit 7 = NMI mask
	if (!BIT(m_control, 7))
		m_maincpu->set_input_line(INPUT_LINE_NMI, CLEAR_LINE);


	if (data & 0x7c)
		logerror("%s: control_w bits 2-6 (%02X)\n", machine().describe_context(), data);
}

void lgtkl_state::control_w(u8 data)
{
	// Bit 7 = NMI mask
	m_control = data;
	if (!BIT(data, 7))
		m_maincpu->set_input_line(INPUT_LINE_NMI, CLEAR_LINE);

	// Bit 6 always set?
	if (!BIT(data, 6))
		logerror("%s: control_w bit 6 unset (%02X)\n", machine().describe_context(), data);

	if (data & 0x3f)
		logerror("%s: control_w bits 0-5 (%02X)\n", machine().describe_context(), data);
}

void lgtkl_state::settings_cmd_w(u8 data)
{
	// pharmyst: command to the custom that keeps the game settings (0x28d00-0x28d3f) without battery.
	// C1 = restore settings, C2 (with 5A in the following byte) = store settings.
	// The CPU waits for the custom to clear the command byte.
	// TODO: the custom isn't dumped / emulated, the settings are simply kept in NVRAM
	m_nvram[0xd81] = data;

	if (data == 0xc1 || data == 0xc2)
	{
		logerror("%s: settings command %02x\n", machine().describe_context(), data);
		m_nvram[0xd81] = 0;
	}
}

u8 lgtkl_state::e0_r()
{
	// boot handshake with an external device (ASIC 4?): the games wait for 0x6e, then write 0x38
	return 0x6e;
}

void lgtz80_base_state::vblank_nmi_w(int state)
{
	if (state && BIT(m_control, 7))
		m_maincpu->set_input_line(INPUT_LINE_NMI, ASSERT_LINE);
}

void lgtz80_state::program_map(address_map &map)
{
	map(0x0000, 0x3fff).rom();
	map(0x4000, 0x7fff).bankr(m_rombank);
	map(0x8000, 0x9fff).ram().share("nvram");
	map(0x9081, 0x9081).w("watchdog", FUNC(watchdog_timer_device::reset_w)); // only ever written with 0x10, never read; still written to RAM?
	map(0xc000, 0xc1ff).ram().w(FUNC(lgtz80_state::reel_attr_ram_w<0>)).share(m_reel_attr_ram[0]);
	map(0xc200, 0xc3ff).ram().w(FUNC(lgtz80_state::reel_attr_ram_w<1>)).share(m_reel_attr_ram[1]);
	map(0xc400, 0xc5ff).ram().w(FUNC(lgtz80_state::reel_attr_ram_w<2>)).share(m_reel_attr_ram[2]);
	map(0xc600, 0xc7ff).ram().w(FUNC(lgtz80_state::reel_attr_ram_w<3>)).share(m_reel_attr_ram[3]);
	map(0xc800, 0xc9ff).ram().w(FUNC(lgtz80_state::reel_ram_w<0>)).share(m_reel_ram[0]);
	map(0xca00, 0xcbff).ram().w(FUNC(lgtz80_state::reel_ram_w<1>)).share(m_reel_ram[1]);
	map(0xcc00, 0xcdff).ram().w(FUNC(lgtz80_state::reel_ram_w<2>)).share(m_reel_ram[2]);
	map(0xce00, 0xcfff).ram().w(FUNC(lgtz80_state::reel_ram_w<3>)).share(m_reel_ram[3]);
	map(0xd000, 0xd7ff).ram().w(FUNC(lgtz80_state::tile_attr_ram_w)).share(m_tile_attr_ram);
	map(0xd800, 0xdfff).ram().w(FUNC(lgtz80_state::tile_ram_w)).share(m_tile_ram);
	map(0xe000, 0xe07f).ram().w(FUNC(lgtz80_state::reel_scroll_ram_w<0>)).share(m_reel_scroll_ram[0]);
	map(0xe080, 0xe0ff).ram().w(FUNC(lgtz80_state::reel_scroll_ram_w<1>)).share(m_reel_scroll_ram[1]);
	map(0xe100, 0xe17f).ram().w(FUNC(lgtz80_state::reel_scroll_ram_w<2>)).share(m_reel_scroll_ram[2]);
	map(0xe180, 0xe1ff).ram().w(FUNC(lgtz80_state::reel_scroll_ram_w<3>)).share(m_reel_scroll_ram[3]);
}

void lgtkl_state::program_map(address_map &map)
{
	map(0x00000, 0x1ffff).rom();
	map(0x28000, 0x2bfff).ram().share("nvram");
	map(0x4c000, 0x4c1ff).ram().w(FUNC(lgtkl_state::reel_attr_ram_w<0>)).share(m_reel_attr_ram[0]);
	map(0x4c200, 0x4c3ff).ram().w(FUNC(lgtkl_state::reel_attr_ram_w<1>)).share(m_reel_attr_ram[1]);
	map(0x4c400, 0x4c5ff).ram().w(FUNC(lgtkl_state::reel_attr_ram_w<2>)).share(m_reel_attr_ram[2]);
	map(0x4c600, 0x4c7ff).ram().w(FUNC(lgtkl_state::reel_attr_ram_w<3>)).share(m_reel_attr_ram[3]);
	map(0x4c800, 0x4c9ff).ram().w(FUNC(lgtkl_state::reel_ram_w<0>)).share(m_reel_ram[0]);
	map(0x4ca00, 0x4cbff).ram().w(FUNC(lgtkl_state::reel_ram_w<1>)).share(m_reel_ram[1]);
	map(0x4cc00, 0x4cdff).ram().w(FUNC(lgtkl_state::reel_ram_w<2>)).share(m_reel_ram[2]);
	map(0x4ce00, 0x4cfff).ram().w(FUNC(lgtkl_state::reel_ram_w<3>)).share(m_reel_ram[3]);
	map(0x4d000, 0x4d7ff).ram().w(FUNC(lgtkl_state::tile_attr_ram_w)).share(m_tile_attr_ram);
	map(0x4d800, 0x4dfff).ram().w(FUNC(lgtkl_state::tile_ram_w)).share(m_tile_ram);
	map(0x4e000, 0x4e07f).ram().w(FUNC(lgtkl_state::reel_scroll_ram_w<0>)).share(m_reel_scroll_ram[0]);
	map(0x4e080, 0x4e0ff).ram().w(FUNC(lgtkl_state::reel_scroll_ram_w<1>)).share(m_reel_scroll_ram[1]);
	map(0x4e100, 0x4e17f).ram().w(FUNC(lgtkl_state::reel_scroll_ram_w<2>)).share(m_reel_scroll_ram[2]);
	map(0x4e180, 0x4e1ff).ram().w(FUNC(lgtkl_state::reel_scroll_ram_w<3>)).share(m_reel_scroll_ram[3]);
}

void lgtkl_state::pharmyst_program_map(address_map &map)
{
	program_map(map);

	map(0x28d81, 0x28d81).w(FUNC(lgtkl_state::settings_cmd_w));
}

void lgtz80_state::io_map(address_map &map)
{
	map.unmap_value_high();
	map.global_mask(0xff);

	map(0x80, 0x80).portr("IN1"); // .w ?
	map(0x88, 0x88).portr("IN2");
	map(0xa8, 0xa8).rw("oki", FUNC(okim6295_device::read), FUNC(okim6295_device::write));
	map(0xb8, 0xb8).w("ramdac", FUNC(ramdac_device::index_w));
	map(0xb9, 0xb9).w("ramdac", FUNC(ramdac_device::pal_w));
	map(0xba, 0xba).w("ramdac", FUNC(ramdac_device::mask_w));
	map(0xc0, 0xc0).r(FUNC(lgtz80_state::control_r)).w(FUNC(lgtz80_state::control_w));
}

void lgtkl_state::fruitcat_io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x80, 0x80).w("ramdac", FUNC(ramdac_device::index_w));
	map(0x81, 0x81).w("ramdac", FUNC(ramdac_device::pal_w));
	map(0x82, 0x82).w("ramdac", FUNC(ramdac_device::mask_w));
	map(0x88, 0x88).rw("oki", FUNC(okim6295_device::read), FUNC(okim6295_device::write));
	// map(0x98, 0x98).w(); TODO
	// map(0xb0, 0xb0).w(); TODO: probably lamps
	map(0xc0, 0xc0).rw(FUNC(lgtkl_state::control_r), FUNC(lgtkl_state::control_w));
}

void lgtkl_state::arthurkn_io_map(address_map &map)
{
	map.global_mask(0xff);
	// map(0x80, 0x80).w(); TODO: bits 1-4 are meters, tested in key test screen
	// map(0x88, 0x88).w(); TODO: probably lamps
	map(0xa0, 0xa0).rw("oki", FUNC(okim6295_device::read), FUNC(okim6295_device::write));
	map(0xb0, 0xb0).w("ramdac", FUNC(ramdac_device::index_w));
	map(0xb1, 0xb1).w("ramdac", FUNC(ramdac_device::pal_w));
	map(0xb2, 0xb2).w("ramdac", FUNC(ramdac_device::mask_w));
	map(0xc0, 0xc0).rw(FUNC(lgtkl_state::control_r), FUNC(lgtkl_state::control_w));
	map(0xe0, 0xe0).r(FUNC(lgtkl_state::e0_r));
}

void lgtkl_state::arthurkn100_io_map(address_map &map)
{
	arthurkn_io_map(map);

	map(0xa8, 0xa8).portr("DSW1");
	map(0xb8, 0xb8).portr("DSW2");
}

void lgtz80_base_state::ramdac_map(address_map &map)
{
	map(0x000, 0x2ff).rw("ramdac", FUNC(ramdac_device::ramdac_pal_r), FUNC(ramdac_device::ramdac_rgb666_w));
}


static INPUT_PORTS_START( arthurkn )
	PORT_START("IN1")
	PORT_SERVICE_NO_TOGGLE( 0x01, IP_ACTIVE_LOW )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_GAMBLE_BET ) //  also works as down in system configuration
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START1 ) // also works for exiting system configuration
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_GAMBLE_TAKE ) // TODO: "Get" in test mode, is it take? also works as up in system configuration
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_GAMBLE_LOW ) PORT_NAME("Low / Show Odds") // "Small" in test mode. Also works as change setting down in system configuration
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_GAMBLE_PAYOUT ) // "Coin out" in test mode
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_SLOT_STOP1 ) // "Stop A" in test mode

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SLOT_STOP2 ) // "Stop B" in test mode
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_SLOT_STOP3 ) // "Stop C" in test mode
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_GAMBLE_D_UP )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_GAMBLE_BOOK ) // "Check" in test mode
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_GAMBLE_HIGH ) // "Big" in test mode. Also works as change setting up in system configuration
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_GAMBLE_KEYIN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_GAMBLE_KEYOUT )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) // TODO: hopper

	PORT_START("IN3") // not shown in key test mode
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_SERVICE1 ) // ??

// no DSW on PCB
INPUT_PORTS_END

static INPUT_PORTS_START( arthurkn100 )
	PORT_INCLUDE( arthurkn )

	// 2 banks of 8 switches on this PCB
	PORT_START("DSW1")
	PORT_DIPNAME( 0x03, 0x03, "10 Times Feature" )  PORT_DIPLOCATION("DSW1:1,2")
	PORT_DIPSETTING(    0x03, "No (Win Points to Points)" )
	PORT_DIPSETTING(    0x02, "Yes (Free Spin Deduct Credit)" )
	PORT_DIPSETTING(    0x01, "No (Win Points to Score)" )
	PORT_DIPSETTING(    0x00, "Yes (Normal)" )
	PORT_DIPNAME( 0x04, 0x04, "Play Score" )  PORT_DIPLOCATION("DSW1:3")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "Display Odds" )  PORT_DIPLOCATION("DSW1:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "Pay-Out Mode" )  PORT_DIPLOCATION("DSW1:5")
	PORT_DIPSETTING(    0x10, "Credit Only" )
	PORT_DIPSETTING(    0x00, "Credit Only (duplicate)" ) // credit + black box according to notes, but seems hard-coded
	PORT_DIPNAME( 0x20, 0x20, "Key-Out Mode" )  PORT_DIPLOCATION("DSW1:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "Game Count Mode" )  PORT_DIPLOCATION("DSW1:7")
	PORT_DIPSETTING(    0x40, "Count" )
	PORT_DIPSETTING(    0x00, DEF_STR( Normal ) )
	PORT_DIPNAME( 0x80, 0x80, "L-Win Mode" )  PORT_DIPLOCATION("DSW1:8")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x07, 0x07, DEF_STR( Coin_A ) )  PORT_DIPLOCATION("DSW2:1,2,3")
	PORT_DIPSETTING(    0x07, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x04, "1 Coin/15 Credits" )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_20C ) )
	PORT_DIPSETTING(    0x02, "1 Coin/75 Credits" )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_100C ) )
	PORT_DIPSETTING(    0x00, "1 Coin/500 Credits" )
	PORT_DIPNAME( 0x38, 0x38, DEF_STR( Coin_B ) )  PORT_DIPLOCATION("DSW2:4,5,6") // Key
	PORT_DIPSETTING(    0x38, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x28, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x20, "1 Coin/15 Credits" )
	PORT_DIPSETTING(    0x18, DEF_STR( 1C_20C ) )
	PORT_DIPSETTING(    0x10, "1 Coin/75 Credits" )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_100C ) )
	PORT_DIPSETTING(    0x00, "1 Coin/500 Credits" )
	PORT_DIPNAME( 0xc0, 0xc0, "Pay-Out Setting" )  PORT_DIPLOCATION("DSW2:7,8")
	PORT_DIPSETTING(    0xc0, "Fixed 10 Points/1 Ticket" )
	PORT_DIPSETTING(    0x80, "Fixed 20 Points/1 Ticket" )
	PORT_DIPSETTING(    0x40, "Fixed 50 Points/1 Ticket" )
	PORT_DIPSETTING(    0x00, "1 (Same as Key-In)" )
INPUT_PORTS_END

static INPUT_PORTS_START( pharmyst )
	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN ) // "Ticket out" in test mode
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN ) // "Ticket notch" in test mode
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN ) // tested in test mode, but no definition given
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN ) // "
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN ) // "
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN ) // "
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN ) // "
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN2 )

	PORT_START("IN2")
	PORT_SERVICE_NO_TOGGLE( 0x01, IP_ACTIVE_LOW )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_GAMBLE_BET ) //  also works as down in system configuration
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START1 ) // also works for exiting system configuration
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_GAMBLE_TAKE ) // TODO: "Get" in test mode, is it take? also works as up in system configuration
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_GAMBLE_LOW ) PORT_NAME("Low / Show Odds") // "Small" in test mode. Also works as change setting down in system configuration
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_GAMBLE_PAYOUT ) // "Coin out" in test mode
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_SLOT_STOP1 ) // "Stop A" in test mode

	PORT_START("IN3") // not shown in key test mode
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SLOT_STOP2 ) // "Stop B" in test mode
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_SLOT_STOP3 ) // "Stop C" in test mode
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_GAMBLE_D_UP )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_GAMBLE_BOOK ) // "Check" in test mode
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_GAMBLE_HIGH ) // "Big" in test mode. Also works as change setting up in system configuration
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_GAMBLE_KEYIN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_GAMBLE_KEYOUT )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) // TODO: hopper

// no DSW on PCB
INPUT_PORTS_END


GFXLAYOUT_RAW(gfx_8x32x8_raw, 8, 32, 8 * 8, 8 * 32 * 8);

static GFXDECODE_START( gfx )
	GFXDECODE_ENTRY( "tiles", 0, gfx_8x8x8_raw, 0, 1 )
	GFXDECODE_ENTRY( "reels", 0, gfx_8x32x8_raw, 0, 1 )
GFXDECODE_END


void lgtz80_base_state::base(machine_config &config)
{
	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	screen_device &screen(SCREEN(config, "screen"));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(64*8, 32*8);
	screen.set_visarea(1, 64*8-1, 8, 31*8-1);
	screen.set_screen_update(FUNC(lgtz80_base_state::screen_update));
	screen.screen_vblank().set(FUNC(lgtz80_base_state::vblank_nmi_w));

	GFXDECODE(config, "gfxdecode", "palette", gfx);

	PALETTE(config, "palette").set_entries(0x100);

	RAMDAC(config, "ramdac", "palette").set_addrmap(0, &lgtz80_base_state::ramdac_map);

	SPEAKER(config, "mono").front_center();

	OKIM6295(config, "oki", 12_MHz_XTAL / 12, okim6295_device::PIN7_HIGH).add_route(ALL_OUTPUTS, "mono", 1.0); // pin 7 and clock not verified
}

void lgtz80_state::mx97iitb(machine_config &config)
{
	base(config);

	Z80(config, m_maincpu, 12_MHz_XTAL / 4); // divider not verified
	m_maincpu->set_addrmap(AS_PROGRAM, &lgtz80_state::program_map);
	m_maincpu->set_addrmap(AS_IO, &lgtz80_state::io_map);

	WATCHDOG_TIMER(config, "watchdog").set_time(attotime::from_seconds(1)); // TODO: actual timeout unknown
}

void lgtkl_state::fruitcat(machine_config &config)
{
	base(config);

	kl5c80a12_device &maincpu(KL5C80A12(config, m_maincpu, 12_MHz_XTAL)); // exact CPU model and divider not verified
	maincpu.set_addrmap(AS_PROGRAM, &lgtkl_state::program_map);
	maincpu.set_addrmap(AS_IO, &lgtkl_state::fruitcat_io_map);
	maincpu.out_p0_callback().set(FUNC(lgtkl_state::oki_bank_w));
	maincpu.in_p1_callback().set_ioport("IN1");
	maincpu.in_p2_callback().set_ioport("IN2");
	maincpu.in_p3_callback().set_ioport("IN3");
}

void lgtkl_state::pharmyst(machine_config &config)
{
	fruitcat(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &lgtkl_state::pharmyst_program_map);
}

void lgtkl_state::arthurkn(machine_config &config)
{
	fruitcat(config);
	m_maincpu->set_addrmap(AS_IO, &lgtkl_state::arthurkn_io_map);
}

void lgtkl_state::arthurkn100(machine_config &config)
{
	arthurkn(config);
	m_maincpu->set_addrmap(AS_IO, &lgtkl_state::arthurkn100_io_map);
}


ROM_START( fruitcat )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "fruit_cat_v2.00.u8", 0x00000, 0x20000, CRC(83d71147) SHA1(4253f5d3273bce22262d34a08f492fa72f776aa5) )

	ROM_REGION( 0x200000, "tiles", 0 )
	ROM_LOAD16_WORD_SWAP( "fruit_cat_rom2.u22", 0x000000, 0x200000, CRC(c78150e8) SHA1(eb276b9b2c4e45b8caf81f17831f6201a6d7392c) ) // actual label has ROM2 between brackets

	ROM_REGION( 0x200000, "reels", 0 )
	ROM_LOAD16_WORD_SWAP( "fruit_cat_rom3.u29", 0x000000, 0x200000, CRC(71afea49) SHA1(89c814302fb58705a479310edb433594d151dfb5) ) // actual label has ROM3 between brackets

	ROM_REGION( 0x80000, "oki", 0 )
	ROM_LOAD( "am29f040.u2", 0x00000, 0x80000, CRC(efd1209e) SHA1(5cd76c9d3073b2e689aa7903e2d65b8ce5b091ca) )

	ROM_REGION( 0x200, "plds", ROMREGION_ERASE00 )
	ROM_LOAD( "atf16v8b-15pc.u21", 0x000, 0x117, NO_DUMP )
ROM_END

ROM_START( arthurkn ) // no stickers on ROMs
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "w29ee011.u21", 0x00000, 0x20000, CRC(d8e2b9f4) SHA1(e8c55c42d7b57fde3168e07fa51f307b83803967) )

	ROM_REGION( 0x200000, "tiles", 0 )
	ROM_LOAD16_WORD_SWAP( "m27c160.u42", 0x000000, 0x200000, CRC(f03a9b0d) SHA1(1e8d9efe7d50871ffc6a0c4c7f08047dd5aac294) )

	ROM_REGION( 0x200000, "reels", 0 )
	ROM_LOAD16_WORD_SWAP( "m27c160.u43", 0x000000, 0x200000, CRC(31d2caab) SHA1(0ee7f35dadb1d5159a487701d059bfd2f54f8c02) )

	ROM_REGION( 0x80000, "oki", 0 )
	ROM_LOAD( "m29f040b.u18", 0x00000, 0x80000, CRC(2b9ab706) SHA1(92154126c7db227acaa4966f71d28475c622e1e6) ) // 1xxxxxxxxxxxxxxxxxx = 0xFF

	ROM_REGION( 0x200, "plds", ROMREGION_ERASE00 )
	ROM_LOAD( "atf16v8b-15pc.u3", 0x000, 0x117, NO_DUMP )
ROM_END

ROM_START( arthurkn100 )
	ROM_REGION( 0x20000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD( "sst39sf010.u21", 0x00000, 0x20000, CRC(4f4d416a) SHA1(7f4eba1e62b65e9acd31a0432ba14728ebeb0bb7) )

	ROM_REGION( 0x200000, "tiles", 0 )
	ROM_LOAD16_WORD_SWAP( "m27c160.u42", 0x000000, 0x200000, CRC(f03a9b0d) SHA1(1e8d9efe7d50871ffc6a0c4c7f08047dd5aac294) )

	ROM_REGION( 0x200000, "reels", 0 )
	ROM_LOAD16_WORD_SWAP( "m27c160.u43", 0x000000, 0x200000, CRC(31d2caab) SHA1(0ee7f35dadb1d5159a487701d059bfd2f54f8c02) )

	ROM_REGION( 0x80000, "oki", 0 )
	ROM_LOAD( "m29f040b.u18", 0x00000, 0x80000, CRC(2b9ab706) SHA1(92154126c7db227acaa4966f71d28475c622e1e6) ) // 1xxxxxxxxxxxxxxxxxx = 0xFF

	ROM_REGION( 0x200, "plds", ROMREGION_ERASE00 )
	ROM_LOAD( "atf16v8b-15pc.u3", 0x000, 0x117, NO_DUMP )
ROM_END

ROM_START( pharmyst ) // no stickers on ROMs
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "w29ee011.u8", 0x00000, 0x20000, CRC(7fcfe9a9) SHA1(e432bdc02d9efd17198e7cb88c4fbc018a3bfec9) )

	ROM_REGION( 0x200000, "tiles", 0 )
	ROM_LOAD16_WORD_SWAP( "m27c160.u22", 0x000000, 0x200000, CRC(b7714015) SHA1(96f4db072350021b561e113eedd82872b373bde7) )

	ROM_REGION( 0x200000, "reels", 0 )
	ROM_LOAD16_WORD_SWAP( "m27c160.u29", 0x000000, 0x200000, CRC(9fd81328) SHA1(10938a6ce8bdb27002a92bac80a5be35ee2b8f3a) )

	ROM_REGION( 0x80000, "oki", 0 )
	ROM_LOAD( "m29f040.u2", 0x00000, 0x80000, CRC(62cfddc4) SHA1(be9c0376d56e03b91d4802d285f30f18224968e8) )

	ROM_REGION( 0x4000, "nvram", 0 )
	ROM_LOAD( "nvram.u7", 0x0000, 0x4000, CRC(36b1569d) SHA1(1ecff32999ad81bf3bc2f792317d555adea26fa5) )

	ROM_REGION( 0x200, "plds", ROMREGION_ERASE00 )
	ROM_LOAD( "atf16v8b-15pc.u21", 0x000, 0x117, NO_DUMP )
ROM_END

ROM_START( circus2 ) // no stickers on ROMs
	ROM_REGION( 0x20000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD( "mx26c1000b.u21", 0x00000, 0x20000, CRC(d573d9d5) SHA1(43d72a53be06b1b4cdf5e0273ad338760b2fad62) )

	ROM_REGION( 0x200000, "tiles", 0 )
	ROM_LOAD16_WORD_SWAP( "m27c160.u42", 0x000000, 0x200000, CRC(048afd3d) SHA1(6f969c78c4d6242df4ec41d84d2b41d6705d19df) ) // 1xxxxxxxxxxxxxxxxxxxx = 0xFF

	ROM_REGION( 0x200000, "reels", 0 )
	ROM_LOAD16_WORD_SWAP( "m27c160.u43", 0x000000, 0x200000, CRC(7b6ef226) SHA1(d86bb0b54094971df34403f1cc0cc426b647cc6e) ) // 1xxxxxxxxxxxxxxxxxxxx = 0xFF

	ROM_REGION( 0x80000, "oki", 0 )
	ROM_LOAD( "am29f040.u18", 0x00000, 0x80000, CRC(71c42437) SHA1(70c63ea468aecf573edebb357f1b877490d65b24) )

	ROM_REGION( 0x200, "plds", ROMREGION_ERASE00 )
	ROM_LOAD( "atf16v8b-15pc.u3", 0x000, 0x117, NO_DUMP )
ROM_END

// 明星97 II 特別版
ROM_START( mx97iitb )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "slon_king.u43", 0x00000, 0x10000, CRC(0c424074) SHA1(c606ecc422226750221502b51e0c8a6bd8570eac) )

	ROM_REGION( 0x80000, "tiles", 0 )
	ROM_LOAD16_BYTE( "slon_king.u7",  0x00000, 0x40000, CRC(698c293c) SHA1(51244c298047d997107e7fbea0dc0e88aef3f084) )
	ROM_LOAD16_BYTE( "slon_king.u14", 0x00001, 0x40000, CRC(fd8e2c31) SHA1(46f44d4d1a608e8c17874c5a25f52f77e6e47cea) )

	ROM_REGION( 0x80000, "reels", 0 )
	ROM_LOAD16_BYTE( "slon_king.u1",  0x00000, 0x40000, CRC(c7fa777c) SHA1(070c3f81927caa6b97c0a809dc73de247f4fa101) )
	ROM_LOAD16_BYTE( "slon_king.u5",  0x00001, 0x40000, CRC(7b9bd11f) SHA1(d1c4c9f4cce612a63d937c77c0db99c71f5fd53d) )

	ROM_REGION( 0x40000, "oki", 0 )
	ROM_LOAD( "slon_king.u4", 0x00000, 0x40000, CRC(6bc004b7) SHA1(ab7588e2ab657d09e446a74450965f818d0a58e0) )

	ROM_REGION( 0x200, "plds", ROMREGION_ERASE00 )
	ROM_LOAD( "gal16v8b.u8", 0x000, 0x117, NO_DUMP )
ROM_END


void lgtkl_state::init_fruitcat()
{
	// Encryption involves a permutation of odd-numbered data lines, conditional on address lines
	u8 *rom = memregion("maincpu")->base();
	for (int i = 0; i < 0x20000; i++)
	{
		switch (i & 0x7c0)
		{
			case 0x000: rom[i] = bitswap<8>(rom[i], 5, 6, 7, 4, 1, 2, 3, 0); break;
			case 0x040:
			case 0x440: rom[i] = bitswap<8>(rom[i], 7, 6, 3, 4, 1, 2, 5, 0); break;
			case 0x080:
			case 0x480: rom[i] = bitswap<8>(rom[i], 1, 6, 7, 4, 5, 2, 3, 0); break;
			case 0x0c0: rom[i] = bitswap<8>(rom[i], 1, 6, 5, 4, 3, 2, 7, 0); break;
			case 0x100:
			case 0x400: rom[i] = bitswap<8>(rom[i], 3, 6, 7, 4, 5, 2, 1, 0); break;
			case 0x140: rom[i] = bitswap<8>(rom[i], 1, 6, 3, 4, 7, 2, 5, 0); break;
			case 0x180:
			case 0x2c0: rom[i] = bitswap<8>(rom[i], 3, 6, 5, 4, 1, 2, 7, 0); break;
			case 0x1c0: rom[i] = bitswap<8>(rom[i], 7, 6, 5, 4, 1, 2, 3, 0); break;
			case 0x200: rom[i] = bitswap<8>(rom[i], 5, 6, 3, 4, 1, 2, 7, 0); break;
			case 0x240:
			case 0x5c0: rom[i] = bitswap<8>(rom[i], 5, 6, 1, 4, 7, 2, 3, 0); break;
			case 0x280: rom[i] = bitswap<8>(rom[i], 3, 6, 1, 4, 7, 2, 5, 0); break;
			case 0x300: break;
			case 0x340: rom[i] = bitswap<8>(rom[i], 1, 6, 7, 4, 3, 2, 5, 0); break;
			case 0x380: rom[i] = bitswap<8>(rom[i], 5, 6, 3, 4, 7, 2, 1, 0); break;
			case 0x3c0:
			case 0x540: rom[i] = bitswap<8>(rom[i], 5, 6, 1, 4, 3, 2, 7, 0); break;
			case 0x4c0: rom[i] = bitswap<8>(rom[i], 7, 6, 3, 4, 5, 2, 1, 0); break;
			case 0x500:
			case 0x780: rom[i] = bitswap<8>(rom[i], 7, 6, 1, 4, 5, 2, 3, 0); break;
			case 0x580: rom[i] = bitswap<8>(rom[i], 1, 6, 5, 4, 7, 2, 3, 0); break;
			case 0x600:
			case 0x680: rom[i] = bitswap<8>(rom[i], 7, 6, 1, 4, 3, 2, 5, 0); break;
			case 0x640: rom[i] = bitswap<8>(rom[i], 3, 6, 5, 4, 7, 2, 1, 0); break;
			case 0x6c0: rom[i] = bitswap<8>(rom[i], 5, 6, 7, 4, 3, 2, 1, 0); break;
			case 0x700: rom[i] = bitswap<8>(rom[i], 3, 6, 7, 4, 1, 2, 5, 0); break;
			case 0x740: rom[i] = bitswap<8>(rom[i], 3, 6, 1, 4, 5, 2, 7, 0); break;
			case 0x7c0: rom[i] = bitswap<8>(rom[i], 1, 6, 3, 4, 5, 2, 7, 0); break;
		}
	}
}

void lgtkl_state::init_arthurkn()
{
	// Encryption involves a permutation of odd-numbered data lines, conditional on address lines
	u8 *rom = memregion("maincpu")->base();
	for (int i = 0; i < 0x20000; i++)
	{
		switch (i & 0x7c0)
		{
			case 0x000:
			case 0x1c0: rom[i] = bitswap<8>(rom[i], 3, 6, 5, 4, 1, 2, 7, 0); break;
			case 0x040:
			case 0x0c0: rom[i] = bitswap<8>(rom[i], 7, 6, 3, 4, 1, 2, 5, 0); break;
			case 0x080:
			case 0x2c0: rom[i] = bitswap<8>(rom[i], 5, 6, 1, 4, 7, 2, 3, 0); break;
			case 0x100: rom[i] = bitswap<8>(rom[i], 1, 6, 7, 4, 3, 2, 5, 0); break;
			case 0x140:
			case 0x340: rom[i] = bitswap<8>(rom[i], 3, 6, 7, 4, 5, 2, 1, 0); break;
			case 0x180: break;
			case 0x200:
			case 0x380: rom[i] = bitswap<8>(rom[i], 7, 6, 1, 4, 3, 2, 5, 0); break;
			case 0x240: rom[i] = bitswap<8>(rom[i], 5, 6, 3, 4, 1, 2, 7, 0); break;
			case 0x280:
			case 0x300: rom[i] = bitswap<8>(rom[i], 7, 6, 1, 4, 5, 2, 3, 0); break;
			case 0x3c0:
			case 0x440: rom[i] = bitswap<8>(rom[i], 5, 6, 1, 4, 3, 2, 7, 0); break;
			case 0x400:
			case 0x480: rom[i] = bitswap<8>(rom[i], 1, 6, 7, 4, 5, 2, 3, 0); break;
			case 0x4c0: rom[i] = bitswap<8>(rom[i], 5, 6, 7, 4, 3, 2, 1, 0); break;
			case 0x500: rom[i] = bitswap<8>(rom[i], 3, 6, 1, 4, 7, 2, 5, 0); break;
			case 0x540: rom[i] = bitswap<8>(rom[i], 1, 6, 5, 4, 7, 2, 3, 0); break;
			case 0x580: rom[i] = bitswap<8>(rom[i], 3, 6, 5, 4, 7, 2, 1, 0); break;
			case 0x5c0: rom[i] = bitswap<8>(rom[i], 3, 6, 1, 4, 5, 2, 7, 0); break;
			case 0x600: rom[i] = bitswap<8>(rom[i], 7, 6, 5, 4, 1, 2, 3, 0); break;
			case 0x640: rom[i] = bitswap<8>(rom[i], 5, 6, 7, 4, 1, 2, 3, 0); break;
			case 0x680: rom[i] = bitswap<8>(rom[i], 1, 6, 5, 4, 3, 2, 7, 0); break;
			case 0x6c0: rom[i] = bitswap<8>(rom[i], 7, 6, 3, 4, 5, 2, 1, 0); break;
			case 0x700: rom[i] = bitswap<8>(rom[i], 1, 6, 3, 4, 5, 2, 7, 0); break;
			case 0x740: rom[i] = bitswap<8>(rom[i], 3, 6, 7, 4, 1, 2, 5, 0); break;
			case 0x780: rom[i] = bitswap<8>(rom[i], 1, 6, 3, 4, 7, 2, 5, 0); break;
			case 0x7c0: rom[i] = bitswap<8>(rom[i], 5, 6, 3, 4, 7, 2, 1, 0); break;
		}
	}
}

void lgtkl_state::init_arthurkn100()
{
	// Encryption involves a permutation of odd-numbered data lines, conditional on address lines
	u8 *rom = memregion("maincpu")->base();
	for (int i = 0; i < 0x20000; i++)
	{
		switch (i & 0x7c0)
		{
			case 0x000: rom[i] = bitswap<8>(rom[i], 5, 6, 3, 4, 1, 2, 7, 0); break;
			case 0x040: rom[i] = bitswap<8>(rom[i], 1, 6, 7, 4, 3, 2, 5, 0); break;
			case 0x080:
			case 0x200: rom[i] = bitswap<8>(rom[i], 7, 6, 3, 4, 1, 2, 5, 0); break;
			case 0x0c0:
			case 0x700: rom[i] = bitswap<8>(rom[i], 7, 6, 1, 4, 5, 2, 3, 0); break;
			case 0x100: rom[i] = bitswap<8>(rom[i], 5, 6, 7, 4, 3, 2, 1, 0); break;
			case 0x140:
			case 0x4c0: rom[i] = bitswap<8>(rom[i], 3, 6, 7, 4, 5, 2, 1, 0); break;
			case 0x180:
			case 0x780: rom[i] = bitswap<8>(rom[i], 5, 6, 1, 4, 7, 2, 3, 0); break;
			case 0x1c0: rom[i] = bitswap<8>(rom[i], 5, 6, 7, 4, 1, 2, 3, 0); break;
			case 0x240:
			case 0x300: rom[i] = bitswap<8>(rom[i], 7, 6, 1, 4, 3, 2, 5, 0); break;
			case 0x280:
			case 0x400: rom[i] = bitswap<8>(rom[i], 5, 6, 1, 4, 3, 2, 7, 0); break;
			case 0x2c0:
			case 0x380: rom[i] = bitswap<8>(rom[i], 3, 6, 5, 4, 1, 2, 7, 0); break;
			case 0x340: rom[i] = bitswap<8>(rom[i], 7, 6, 3, 4, 5, 2, 1, 0); break;
			case 0x3c0: rom[i] = bitswap<8>(rom[i], 1, 6, 5, 4, 7, 2, 3, 0); break;
			case 0x440: rom[i] = bitswap<8>(rom[i], 3, 6, 1, 4, 5, 2, 7, 0); break;
			case 0x480:
			case 0x740: rom[i] = bitswap<8>(rom[i], 1, 6, 7, 4, 5, 2, 3, 0); break;
			case 0x500: rom[i] = bitswap<8>(rom[i], 1, 6, 3, 4, 7, 2, 5, 0); break;
			case 0x540: rom[i] = bitswap<8>(rom[i], 5, 6, 3, 4, 7, 2, 1, 0); break;
			case 0x580: rom[i] = bitswap<8>(rom[i], 3, 6, 5, 4, 7, 2, 1, 0); break;
			case 0x5c0: rom[i] = bitswap<8>(rom[i], 3, 6, 1, 4, 7, 2, 5, 0); break;
			case 0x600: rom[i] = bitswap<8>(rom[i], 1, 6, 3, 4, 5, 2, 7, 0); break;
			case 0x640: rom[i] = bitswap<8>(rom[i], 3, 6, 7, 4, 1, 2, 5, 0); break;
			case 0x680: rom[i] = bitswap<8>(rom[i], 1, 6, 5, 4, 3, 2, 7, 0); break;
			case 0x6c0: rom[i] = bitswap<8>(rom[i], 7, 6, 5, 4, 1, 2, 3, 0); break;
			case 0x7c0: break;
		}
	}
}

void lgtkl_state::init_pharmyst()
{
	// Encryption involves a permutation of odd-numbered data lines, conditional on address lines
	u8 *rom = memregion("maincpu")->base();

	for (int i = 0; i < 0x20000; i++)
	{
		switch (i & 0x7c0)
		{
			case 0x000: rom[i] = bitswap<8>(rom[i], 5, 6, 7, 4, 1, 2, 3, 0); break;
			case 0x040: rom[i] = bitswap<8>(rom[i], 7, 6, 3, 4, 1, 2, 5, 0); break;
			case 0x080: rom[i] = bitswap<8>(rom[i], 7, 6, 5, 4, 1, 2, 3, 0); break;
			case 0x0c0: rom[i] = bitswap<8>(rom[i], 5, 6, 3, 4, 1, 2, 7, 0); break;
			case 0x100: rom[i] = bitswap<8>(rom[i], 3, 6, 1, 4, 7, 2, 5, 0); break;
			case 0x140: rom[i] = bitswap<8>(rom[i], 3, 6, 5, 4, 1, 2, 7, 0); break;
			case 0x180: rom[i] = bitswap<8>(rom[i], 5, 6, 1, 4, 7, 2, 3, 0); break;
			case 0x1c0: rom[i] = bitswap<8>(rom[i], 1, 6, 3, 4, 7, 2, 5, 0); break;
			case 0x200: rom[i] = bitswap<8>(rom[i], 3, 6, 7, 4, 5, 2, 1, 0); break;
			case 0x240: rom[i] = bitswap<8>(rom[i], 1, 6, 7, 4, 5, 2, 3, 0); break;
			case 0x280: rom[i] = bitswap<8>(rom[i], 1, 6, 7, 4, 5, 2, 3, 0); break;
			case 0x2c0: rom[i] = bitswap<8>(rom[i], 1, 6, 5, 4, 3, 2, 7, 0); break;
			case 0x300: rom[i] = bitswap<8>(rom[i], 7, 6, 1, 4, 5, 2, 3, 0); break;
			case 0x340: rom[i] = bitswap<8>(rom[i], 7, 6, 1, 4, 3, 2, 5, 0); break;
			case 0x380: rom[i] = bitswap<8>(rom[i], 5, 6, 3, 4, 7, 2, 1, 0); break;
			case 0x3c0: rom[i] = bitswap<8>(rom[i], 5, 6, 1, 4, 3, 2, 7, 0); break;
			case 0x400: rom[i] = bitswap<8>(rom[i], 3, 6, 7, 4, 5, 2, 1, 0); break;
			case 0x440: rom[i] = bitswap<8>(rom[i], 7, 6, 3, 4, 1, 2, 5, 0); break;
			case 0x480: rom[i] = bitswap<8>(rom[i], 3, 6, 5, 4, 1, 2, 7, 0); break;
			case 0x4c0: rom[i] = bitswap<8>(rom[i], 7, 6, 5, 4, 3, 2, 1, 0); break;
			case 0x500: rom[i] = bitswap<8>(rom[i], 5, 6, 1, 4, 3, 2, 7, 0); break;
			case 0x540: rom[i] = bitswap<8>(rom[i], 1, 6, 5, 4, 7, 2, 3, 0); break;
			case 0x580: rom[i] = bitswap<8>(rom[i], 3, 6, 1, 4, 5, 2, 7, 0); break;
			case 0x5c0: rom[i] = bitswap<8>(rom[i], 7, 6, 1, 4, 5, 2, 3, 0); break;
			case 0x600: rom[i] = bitswap<8>(rom[i], 7, 6, 3, 4, 5, 2, 1, 0); break;
			case 0x640: rom[i] = bitswap<8>(rom[i], 3, 6, 7, 4, 1, 2, 5, 0); break;
			case 0x680: rom[i] = bitswap<8>(rom[i], 7, 6, 1, 4, 3, 2, 5, 0); break;
			case 0x6c0: rom[i] = bitswap<8>(rom[i], 5, 6, 7, 4, 3, 2, 1, 0); break;
			case 0x700: rom[i] = bitswap<8>(rom[i], 1, 6, 7, 4, 3, 2, 5, 0); break;
			case 0x740: rom[i] = bitswap<8>(rom[i], 5, 6, 1, 4, 7, 2, 3, 0); break;
			case 0x780: rom[i] = bitswap<8>(rom[i], 3, 6, 5, 4, 7, 2, 1, 0); break;
			case 0x7c0: rom[i] = bitswap<8>(rom[i], 1, 6, 3, 4, 5, 2, 7, 0); break;
		}
	}
}

void lgtkl_state::init_circus2()
{
	// Encryption involves a permutation of odd-numbered data lines, conditional on address lines
	u8 *rom = memregion("maincpu")->base();

	for (int i = 0; i < 0x20000; i++)
	{
		switch (i & 0x7c0)
		{
			case 0x000: rom[i] = bitswap<8>(rom[i], 7, 6, 3, 4, 1, 2, 5, 0); break;
			case 0x040: rom[i] = bitswap<8>(rom[i], 7, 6, 1, 4, 3, 2, 5, 0); break;
			case 0x080: rom[i] = bitswap<8>(rom[i], 7, 6, 1, 4, 3, 2, 5, 0); break;
			case 0x0c0: rom[i] = bitswap<8>(rom[i], 1, 6, 3, 4, 7, 2, 5, 0); break;
			case 0x100: rom[i] = bitswap<8>(rom[i], 3, 6, 7, 4, 1, 2, 5, 0); break;
			case 0x140: rom[i] = bitswap<8>(rom[i], 5, 6, 1, 4, 7, 2, 3, 0); break;
			case 0x180: rom[i] = bitswap<8>(rom[i], 3, 6, 1, 4, 7, 2, 5, 0); break;
			case 0x1c0: rom[i] = bitswap<8>(rom[i], 5, 6, 1, 4, 3, 2, 7, 0); break;
			case 0x200: rom[i] = bitswap<8>(rom[i], 1, 6, 5, 4, 3, 2, 7, 0); break;
			case 0x240: rom[i] = bitswap<8>(rom[i], 7, 6, 3, 4, 5, 2, 1, 0); break;
			case 0x280: rom[i] = bitswap<8>(rom[i], 5, 6, 1, 4, 3, 2, 7, 0); break;
			case 0x2c0: rom[i] = bitswap<8>(rom[i], 1, 6, 5, 4, 7, 2, 3, 0); break;
			case 0x300: rom[i] = bitswap<8>(rom[i], 7, 6, 3, 4, 1, 2, 5, 0); break;
			case 0x340: rom[i] = bitswap<8>(rom[i], 7, 6, 5, 4, 1, 2, 3, 0); break;
			case 0x380: rom[i] = bitswap<8>(rom[i], 1, 6, 7, 4, 5, 2, 3, 0); break;
			case 0x3c0: rom[i] = bitswap<8>(rom[i], 3, 6, 7, 4, 5, 2, 1, 0); break;
			case 0x400: rom[i] = bitswap<8>(rom[i], 3, 6, 1, 4, 5, 2, 7, 0); break;
			case 0x440: rom[i] = bitswap<8>(rom[i], 3, 6, 5, 4, 1, 2, 7, 0); break;
			case 0x480: rom[i] = bitswap<8>(rom[i], 7, 6, 1, 4, 5, 2, 3, 0); break;
			case 0x4c0: rom[i] = bitswap<8>(rom[i], 5, 6, 1, 4, 7, 2, 3, 0); break;
			case 0x500: rom[i] = bitswap<8>(rom[i], 5, 6, 3, 4, 7, 2, 1, 0); break;
			case 0x540: rom[i] = bitswap<8>(rom[i], 3, 6, 5, 4, 7, 2, 1, 0); break;
			case 0x580: rom[i] = bitswap<8>(rom[i], 3, 6, 7, 4, 5, 2, 1, 0); break;
			case 0x5c0: rom[i] = bitswap<8>(rom[i], 5, 6, 7, 4, 1, 2, 3, 0); break;
			case 0x600: rom[i] = bitswap<8>(rom[i], 7, 6, 3, 4, 1, 2, 5, 0); break;
			case 0x640: rom[i] = bitswap<8>(rom[i], 3, 6, 5, 4, 1, 2, 7, 0); break;
			case 0x680: rom[i] = bitswap<8>(rom[i], 7, 6, 1, 4, 5, 2, 3, 0); break;
			case 0x6c0: rom[i] = bitswap<8>(rom[i], 1, 6, 7, 4, 5, 2, 3, 0); break;
			case 0x700: rom[i] = bitswap<8>(rom[i], 5, 6, 3, 4, 1, 2, 7, 0); break;
			case 0x740: rom[i] = bitswap<8>(rom[i], 1, 6, 7, 4, 3, 2, 5, 0); break;
			case 0x780: rom[i] = bitswap<8>(rom[i], 7, 6, 5, 4, 3, 2, 1, 0); break;
			case 0x7c0: rom[i] = bitswap<8>(rom[i], 5, 6, 7, 4, 3, 2, 1, 0); break;
		}
	}
}

} // anonymous namespace


GAME( 200?,  mx97iitb,    0,        mx97iitb,    arthurkn,    lgtz80_state, empty_init,       ROT0, "LGT",               "Mingxing 97 II Tebie Ban",      MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE )

GAME( 2003?, fruitcat,    0,        fruitcat,    arthurkn,    lgtkl_state,  init_fruitcat,    ROT0, "LGT",               "Fruit Cat (v2.00)",             MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE )
GAME( 200?,  arthurkn,    0,        arthurkn,    arthurkn,    lgtkl_state,  init_arthurkn,    ROT0, "LGT",               "Arthur's Knights",              MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE )
GAME( 200?,  arthurkn100, arthurkn, arthurkn100, arthurkn100, lgtkl_state,  init_arthurkn100, ROT0, "LGT (LSE license)", "Arthur's Knights (v1.00)",      MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE )
GAME( 2007,  pharmyst,    0,        pharmyst,    pharmyst,    lgtkl_state,  init_pharmyst,    ROT0, "LGT",               "Pharaoh's Mystery (USA v3.00)", MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE )
GAME( 200?,  circus2,     0,        arthurkn,    arthurkn,    lgtkl_state,  init_circus2,     ROT0, "LGT",               "Circus 2",                      MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE )
